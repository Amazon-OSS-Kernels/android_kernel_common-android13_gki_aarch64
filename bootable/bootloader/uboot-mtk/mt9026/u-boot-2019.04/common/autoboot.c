// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#include <common.h>
#include <autoboot.h>
#include <bootretry.h>
#include <cli.h>
#include <console.h>
#include <fdtdec.h>
#include <menu.h>
#include <post.h>
#include <u-boot/sha256.h>
#include <bootcount.h>
#include <flow.h>
#ifdef CONFIG_MULTICORES_PLATFORM
#include <bootflow_thread.h>
#endif
#include <utility.h>
#include <version.h>
#include <wdt.h>
#include <dm.h>
#include <romtblo_impl.h>
#include <system_impl.h>
#include "amzn_secure_boot.h"
#include "idme.h"
#include "stdlib.h"
#include "exports.h"
#include <amzn_tv_common.h>
#include <amzn_tv_secure_boot.h>
#include <boot_impl.h>

DECLARE_GLOBAL_DATA_PTR;

#define MAX_DELAY_STOP_STR 32
#define BOOTARGS_VALUE_LEN 64
#define BOOTARGS_UBOOT_VERSION_KEY_LEN 24
#define DEV_FLAGS_LOGLEVEL_3 (0x00000200ull)

#ifndef DEBUG_BOOTKEYS
#define DEBUG_BOOTKEYS 0
#endif
#define debug_bootkeys(fmt, args...)		\
	debug_cond(DEBUG_BOOTKEYS, fmt, ##args)

/* Stored value of bootdelay, used by autoboot_command() */
static int stored_bootdelay;

#if defined(CONFIG_ANDROID_AB)
struct SUB_COMMAND{
	bool exist;
	char buf[64];
};
#endif

#if defined(CONFIG_AUTOBOOT_KEYED)
#if defined(CONFIG_AUTOBOOT_STOP_STR_SHA256)

/*
 * Use a "constant-length" time compare function for this
 * hash compare:
 *
 * https://crackstation.net/hashing-security.htm
 */
static int slow_equals(u8 *a, u8 *b, int len)
{
	int diff = 0;
	int i;

	for (i = 0; i < len; i++)
		diff |= a[i] ^ b[i];

	return diff == 0;
}

static int passwd_abort(uint64_t etime)
{
	const char *sha_env_str = env_get("bootstopkeysha256");
	u8 sha_env[SHA256_SUM_LEN];
	u8 sha[SHA256_SUM_LEN];
	char presskey[MAX_DELAY_STOP_STR];
	const char *algo_name = "sha256";
	u_int presskey_len = 0;
	int abort = 0;
	int size = sizeof(sha);
	int ret;

	if (sha_env_str == NULL)
		sha_env_str = CONFIG_AUTOBOOT_STOP_STR_SHA256;

	/*
	 * Generate the binary value from the environment hash value
	 * so that we can compare this value with the computed hash
	 * from the user input
	 */
	ret = hash_parse_string(algo_name, sha_env_str, sha_env);
	if (ret) {
		printf("Hash %s not supported!\n", algo_name);
		return 0;
	}

	/*
	 * We don't know how long the stop-string is, so we need to
	 * generate the sha256 hash upon each input character and
	 * compare the value with the one saved in the environment
	 */
	do {
		if (tstc()) {
			/* Check for input string overflow */
			if (presskey_len >= MAX_DELAY_STOP_STR)
				return 0;

			presskey[presskey_len++] = getc();

			/* Calculate sha256 upon each new char */
			hash_block(algo_name, (const void *)presskey,
				   presskey_len, sha, &size);

			/* And check if sha matches saved value in env */
			if (slow_equals(sha, sha_env, SHA256_SUM_LEN))
				abort = 1;
		}
	} while (!abort && get_ticks() <= etime);

	return abort;
}
#else
static int passwd_abort(uint64_t etime)
{
	int abort = 0;
	struct {
		char *str;
		u_int len;
		int retry;
	}
	delaykey[] = {
		{ .str = env_get("bootdelaykey"),  .retry = 1 },
		{ .str = env_get("bootstopkey"),   .retry = 0 },
	};

	char presskey[MAX_DELAY_STOP_STR];
	u_int presskey_len = 0;
	u_int presskey_max = 0;
	u_int i;

#  ifdef CONFIG_AUTOBOOT_DELAY_STR
	if (delaykey[0].str == NULL)
		delaykey[0].str = CONFIG_AUTOBOOT_DELAY_STR;
#  endif
#  ifdef CONFIG_AUTOBOOT_STOP_STR
	if (delaykey[1].str == NULL)
		delaykey[1].str = CONFIG_AUTOBOOT_STOP_STR;
#  endif

	for (i = 0; i < sizeof(delaykey) / sizeof(delaykey[0]); i++) {
		delaykey[i].len = delaykey[i].str == NULL ?
				    0 : strlen(delaykey[i].str);
		delaykey[i].len = delaykey[i].len > MAX_DELAY_STOP_STR ?
				    MAX_DELAY_STOP_STR : delaykey[i].len;

		presskey_max = presskey_max > delaykey[i].len ?
				    presskey_max : delaykey[i].len;

		debug_bootkeys("%s key:<%s>\n",
			       delaykey[i].retry ? "delay" : "stop",
			       delaykey[i].str ? delaykey[i].str : "NULL");
	}

	/* In order to keep up with incoming data, check timeout only
	 * when catch up.
	 */
	do {
		if (tstc()) {
			if (presskey_len < presskey_max) {
				presskey[presskey_len++] = getc();
			} else {
				for (i = 0; i < presskey_max - 1; i++)
					presskey[i] = presskey[i + 1];

				presskey[i] = getc();
			}
		}

		for (i = 0; i < sizeof(delaykey) / sizeof(delaykey[0]); i++) {
			if (delaykey[i].len > 0 &&
			    presskey_len >= delaykey[i].len &&
				memcmp(presskey + presskey_len -
					delaykey[i].len, delaykey[i].str,
					delaykey[i].len) == 0) {
					debug_bootkeys("got %skey\n",
						delaykey[i].retry ? "delay" :
						"stop");

				/* don't retry auto boot */
				if (!delaykey[i].retry)
					bootretry_dont_retry();
				abort = 1;
			}
		}
	} while (!abort && get_ticks() <= etime);

	return abort;
}
#endif

/***************************************************************************
 * Watch for 'delay' seconds for autoboot stop or autoboot delay string.
 * returns: 0 -  no key string, allow autoboot 1 - got key string, abort
 */
static int __abortboot(int bootdelay)
{
	int abort;
	uint64_t etime = endtick(bootdelay);

#  ifdef CONFIG_AUTOBOOT_PROMPT
	/*
	 * CONFIG_AUTOBOOT_PROMPT includes the %d for all boards.
	 * To print the bootdelay value upon bootup.
	 */
	printf(CONFIG_AUTOBOOT_PROMPT, bootdelay);
#  endif

	abort = passwd_abort(etime);
	if (!abort)
		debug_bootkeys("key timeout\n");

	return abort;
}

# else	/* !defined(CONFIG_AUTOBOOT_KEYED) */

#ifdef CONFIG_MENUKEY
static int menukey;
#endif

static int __abortboot(int bootdelay)
{
	int abort = 0;
	int input = 0;
	unsigned long ts;

#ifdef CONFIG_MENUPROMPT
	printf(CONFIG_MENUPROMPT);
#else
	printf("Hit any key to stop autoboot: %2d ", bootdelay);
#endif

	/*
	 * Check if key already pressed
	 */
	if (tstc()) {	/* we got a key press	*/
#if 1	/* block all key except enter key	*/
		input = getc();  /* consume input	*/

		if ((input == '\r') || (input == '\n'))
		{
			puts("\b\b\b 0");
			abort = 1;	/* don't auto boot	*/
		}
#else
		(void) getc();  /* consume input	*/
		puts("\b\b\b 0");
		abort = 1;	/* don't auto boot	*/
#endif
	}

	while ((bootdelay > 0) && (!abort)) {
		--bootdelay;
		/* delay 1000 ms */
		ts = get_timer(0);
		do {
			if (tstc()) {	/* we got a key press	*/
				abort  = 1;	/* don't auto boot	*/
				bootdelay = 0;	/* no more delay	*/
# ifdef CONFIG_MENUKEY
				menukey = getc();
# else
				(void) getc();  /* consume input	*/
# endif
				break;
			}
			udelay(10000);
		} while (!abort && get_timer(ts) < 1000);

		printf("\b\b\b%2d ", bootdelay);
	}

	putc('\n');

	return abort;
}
# endif	/* CONFIG_AUTOBOOT_KEYED */

static void amzn_add_selinux_bootargs(void)
{
	char* dev_flags_str = NULL;
	unsigned long dev_flags = 0;

	if ((dev_flags_str = env_get("dev_flags")) != NULL)
		dev_flags = simple_strtoul(dev_flags_str, NULL, 16);

	if (((dev_flags & DEV_FLAGS_SELINUX_FORCE_PERMISSIVE) == DEV_FLAGS_SELINUX_FORCE_PERMISSIVE)
		&& !is_lockdown()) {
		printf("force selinux permissive mode\n");
		add_bootargs("androidboot.selinux", "androidboot.selinux=permissive", 0);
	} else {
		printf("force selinux enforcing mode\n");
		del_bootargs("androidboot.selinux", 1);
	}
}

/*TODO: Set env by feature, clarify env usage, remove no use env at next iteraction*/
static void bootargs_setting(void)
{
	int ret = 0;
	char newstr[BOOTARGS_VALUE_LEN] = {0};
#ifndef CONFIG_TARGET_MT5896
	add_bootargs("EMAC_MEM", "EMAC_MEM=0x100000", 0);
	add_bootargs("PM51_ADDR", "PM51_ADDR=0x3FB90000", 0);
	add_bootargs("PM51_LEN", "PM51_LEN=0x10000", 0);
	add_bootargs("rootfstype", "rootfstype=ext4", 0);
	add_bootargs("rootflags", "rootflags=noblock_validity", 0);
	add_bootargs("panic", "panic=0", 0);
	add_bootargs("vmpressure_level_critical", "vmpressure_level_critical=80", 0);
	add_bootargs("cgroup.memory", "cgroup.memory=nokmem", 0);
	add_bootargs("utpa_str_fr", "utpa_str_fr=1", 0);
	add_bootargs("vmalloc", "vmalloc=550M", 0);
	add_bootargs("MIRROR", "MIRROR=0", 0);
	add_bootargs("ENV_VAR_OFFSET", "ENV_VAR_OFFSET=0x0", 0);
	add_bootargs("ENV_VAR_SIZE", "ENV_VAR_SIZE=0x10000", 0);
	add_bootargs("ENV", "ENV=EMMC", 0);
	add_bootargs("SECURITY", "SECURITY=ON", 0);
	add_bootargs("USB_OTG_SOFTWARE_ID", "USB_OTG_SOFTWARE_ID=host", 0);
	add_bootargs("recovery_fbaddr", "recovery_fbaddr=0x22500000", 0);
	add_bootargs("androidboot.force_normal_boot", "androidboot.force_normal_boot=1", 0);
#endif
	ret = snprintf(newstr, sizeof(newstr), "androidboot.bootloader="BOOTLOADER_VERSION);
	if(ret < BOOTARGS_UBOOT_VERSION_KEY_LEN)
		debug("bootloader version is not complete.\n");
	add_bootargs("androidboot.bootloader", newstr, 0);

	amzn_add_selinux_bootargs();

	if(is_secure_cpu())
		add_bootargs("androidboot.secure_cpu", "androidboot.secure_cpu=1", 0);
	else
		add_bootargs("androidboot.secure_cpu", "androidboot.secure_cpu=0", 0);

	if(amzn_target_device_type() == AMZN_PRODUCTION_DEVICE) {
		anti_rollback_status_type ar_status = anti_rollback_enabled();

		add_bootargs("androidboot.prod", "androidboot.prod=1", 0);
		if (ar_status & AR_ENABLED_EFUSE)
			add_bootargs("androidboot.arb_efuse_state", "androidboot.arb_efuse_state=1", 0);
		if (ar_status & AR_ENABLED_RPMB)
			add_bootargs("androidboot.rpmb_state", "androidboot.rpmb_state=1", 0);
	} else {
		add_bootargs("androidboot.prod", "androidboot.prod=0", 0);
		add_bootargs("androidboot.arb_efuse_state", "androidboot.arb_efuse_state=0", 0);
		add_bootargs("androidboot.rpmb_state", "androidboot.rpmb_state=0", 0);
	}

#if defined(UFBL_FEATURE_UNLOCK)
	if ( !target_is_production() || amzn_device_is_unlocked() || simple_strtoul(env_get("bootmode"), NULL, 10) == IDME_BOOTMODE_DIAG )
		add_bootargs("androidboot.unlocked_kernel", "androidboot.unlocked_kernel=true", 0);
	else
		add_bootargs("androidboot.unlocked_kernel", "androidboot.unlocked_kernel=false", 0);
#endif

	char *bootargs = env_get("bootargs");
	if ((strstr(bootargs,"androidboot.unlocked_kernel=true") != NULL) && (env_get("dev_flags") && (simple_strtoul(env_get("dev_flags"), NULL, 16) & DEV_FLAGS_LOGLEVEL_3))){
		add_bootargs("loglevel", "loglevel=3", 0);
	} else {
		add_bootargs("loglevel", "loglevel=6", 0);
	}

	if (env_get("fos_flags") && (simple_strtoul(env_get("fos_flags"), NULL, 16) & FOS_FLAGS_CONSOLE_ON)) {
		add_bootargs("8250.fos_console_on", "8250.fos_console_on=1", 0);
	} else {
		add_bootargs("8250.fos_console_on", "8250.fos_console_on=0", 0);
	}

#if (defined(TARGET_AMMO_SUPPORT) && defined(UFBL_FEATURE_IDME))
	{
		#define PROD_VAR_SIZE 32
		#define MAX_OEM_DATA 1024
		char ammo_var[PROD_VAR_SIZE+1] = {0,};
		char ammo_prop[PROD_VAR_SIZE+sizeof("androidboot.ammo.prod.var=")+2] = {0,};
		char remote_type_value[PROD_VAR_SIZE+1] = {0,};
		char remote_type_prop[PROD_VAR_SIZE+sizeof("androidboot.selected_remote=")+2] = {0,};
		char oem_data[MAX_OEM_DATA] = { 0x00, };

		idme_get_oem_data_field("ammo_var=",ammo_var, PROD_VAR_SIZE);
		sprintf(ammo_prop, "androidboot.ammo.prod.var=%s ", ammo_var);
		add_bootargs("androidboot.ammo.prod.var", ammo_prop, 0);

		idme_get_var_external("oem_data", oem_data, (sizeof(oem_data) - 1));
		if (strstr(oem_data, "selected_remote")) {
			idme_get_oem_data_field("selected_remote=",remote_type_value, PROD_VAR_SIZE);
			sprintf(remote_type_prop, "androidboot.selected_remote=%s ", remote_type_value);
			add_bootargs("androidboot.selected_remote", remote_type_prop, 0);
		}
	}
#endif
	if (is_lockdown()) {
		del_bootargs("CORE_DUMP_PATH", 1);
		del_bootargs("KDebug", 1);

	}else {
		add_bootargs("CORE_DUMP_PATH", "CORE_DUMP_PATH=/data/core_dump.%%p.gz ", 1);
		add_bootargs("KDebug", "KDebug=1 ", 1);
	}
}
/*TODO: end*/

static int abortboot(int bootdelay)
{
	int abort = 0;

	if (bootdelay >= 0)
		abort = __abortboot(bootdelay);

#ifdef CONFIG_SILENT_CONSOLE
	if (abort)
		gd->flags &= ~GD_FLG_SILENT;
#endif

	return abort;
}

static void process_fdt_options(const void *blob)
{
#if defined(CONFIG_OF_CONTROL) && defined(CONFIG_SYS_TEXT_BASE)
	ulong addr;

	/* Add an env variable to point to a kernel payload, if available */
	addr = fdtdec_get_config_int(gd->fdt_blob, "kernel-offset", 0);
	if (addr)
		env_set_addr("kernaddr", (void *)(CONFIG_SYS_TEXT_BASE + addr));

	/* Add an env variable to point to a root disk, if available */
	addr = fdtdec_get_config_int(gd->fdt_blob, "rootdisk-offset", 0);
	if (addr)
		env_set_addr("rootaddr", (void *)(CONFIG_SYS_TEXT_BASE + addr));
#endif /* CONFIG_OF_CONTROL && CONFIG_SYS_TEXT_BASE */
}

const char *bootdelay_process(void)
{
	char *s;
	int bootdelay;

	bootcount_inc();

	s = env_get("bootdelay");
	bootdelay = s ? (int)simple_strtol(s, NULL, 10) : CONFIG_BOOTDELAY;

#ifdef CONFIG_OF_CONTROL
	bootdelay = fdtdec_get_config_int(gd->fdt_blob, "bootdelay",
			bootdelay);
#endif

	debug("### main_loop entered: bootdelay=%d\n\n", bootdelay);

#if defined(CONFIG_MENU_SHOW)
	bootdelay = menu_show(bootdelay);
#endif
	bootretry_init_cmd_timeout();

#ifdef CONFIG_POST
	if (gd->flags & GD_FLG_POSTFAIL) {
		s = env_get("failbootcmd");
	} else
#endif /* CONFIG_POST */
	if (bootcount_error())
		s = env_get("altbootcmd");
	else if(is_recovery_mode())
		s = env_get("recoverycmd");
	else
		s = env_get("bootcmd");

	process_fdt_options(gd->fdt_blob);
	stored_bootdelay = bootdelay;

	return s;
}

#if defined(CONFIG_GICV3)
extern void cpu_interrupt_setting(void);
#endif

#define REG_DRAMC_ADDR                      (0x100)
#define REG_DRAMC_IDX                       (0x0A)
#define DRAMC_WDT_SET_BIT0   (0x0001)
#define DRAMC_WDT_SET_BIT8   (0x0100)
#define REG_WDT_RST_SEL         (0x0100)
#define REG_WDT_DRAMC_SREF_MODE  (0xC000)


void autoboot_command(const char *s)
{
	int chipid = romtbl_get_chip_id_info();
	int ldm;
	char bootarg[BOOTARGS_VALUE_LEN] = {0};

	debug("### main_loop: bootcmd=\"%s\"\n", s ? s : "<UNDEFINED>");

	if (stored_bootdelay != -1 && s && !abortboot(stored_bootdelay)) {


#ifdef CONFIG_AMAZON_UBOOT_SMP_OPTIMIZATION
		extern void amazon_init_optimize(void);
		printf("amazon: in smp!!!\n");
		amazon_init_optimize();
#else
		printf("amazon: not smp!!!\n");
#endif

#if !defined(CONFIG_MULTICORES_PLATFORM) && defined(CONFIG_GICV3)
		cpu_interrupt_setting();
#endif
#if defined(CONFIG_AUTOBOOT_KEYED) && !defined(CONFIG_AUTOBOOT_KEYED_CTRLC)
		int prev = disable_ctrlc(1);	/* disable Control C checking */
#endif
#ifdef CONFIG_MULTICORES_PLATFORM
		do_before_console_input();
		do_before_console_input_customer();
#endif
		do_after_console_input();
#ifdef CONFIG_MULTICORES_PLATFORM
		do_after_console_input_customer();
#endif
		do_before_boot_kernel();
#ifdef CONFIG_MULTICORES_PLATFORM
		do_before_boot_kernel_customer();
#endif
		if(is_qhb_boot_mode() == 1)
			add_bootargs("androidboot.quiescent", "androidboot.quiescent=1", 0);

		ldm = is_ldm_support();
		if (ldm > 0 && ldm <= 5) {
			printf("support_ldm=%d \n", ldm);
			snprintf(bootarg, BOOTARGS_VALUE_LEN, "androidboot.support_ldm=%d", ldm);
			add_bootargs("androidboot.support_ldm", bootarg, 0);
		}

		bootargs_setting();
		do_jump_to_kernel();
        	/*TODO: current do_jump_to_kernel just do verify, not jump to kernel.
                  After do_jump_to_kernel function implement complete,
                  the following source code will remove in the future.*/
		run_command_list(s, -1, 0);

#if defined(CONFIG_AUTOBOOT_KEYED) && !defined(CONFIG_AUTOBOOT_KEYED_CTRLC)
		disable_ctrlc(prev);	/* restore Control C checking */
#endif
	}
    else
    {
        struct udevice *dev;
        if(uclass_get_device(UCLASS_WDT, 0, &dev)==0)
        {
            if(dev!=NULL)
            {
                wdt_stop(dev);
                printf(" jump to console \n");
				if(chipid == MT5897)
				{
					CPU_REGISTER_ACCESS(REG_DRAMC_ADDR, REG_DRAMC_IDX) = (CPU_REGISTER_ACCESS(REG_DRAMC_ADDR, REG_DRAMC_IDX) & (~DRAMC_WDT_SET_BIT0));
					CPU_REGISTER_ACCESS(REG_DRAMC_ADDR, REG_DRAMC_IDX) = (CPU_REGISTER_ACCESS(REG_DRAMC_ADDR, REG_DRAMC_IDX) & (~DRAMC_WDT_SET_BIT8));
				}
				else if(chipid == MT5879)
				{
					CPU_REGISTER_ACCESS(REG_DRAMC_ADDR, REG_DRAMC_IDX) = (CPU_REGISTER_ACCESS(REG_DRAMC_ADDR, REG_DRAMC_IDX) & (~REG_WDT_RST_SEL));
					CPU_REGISTER_ACCESS(REG_DRAMC_ADDR, REG_DRAMC_IDX) = (CPU_REGISTER_ACCESS(REG_DRAMC_ADDR, REG_DRAMC_IDX) & (~REG_WDT_DRAMC_SREF_MODE));
				}

                printf(" disable DRM \n");
            }
            else
                printf(" get wdt device error !!!\n");

        }
        else
            printf(" WDT get device Error !!!!\n");
    }
#ifdef CONFIG_MENUKEY
	if (menukey == CONFIG_MENUKEY) {
		s = env_get("menucmd");
		if (s)
			run_command_list(s, -1, 0);
	}
#endif /* CONFIG_MENUKEY */
}
