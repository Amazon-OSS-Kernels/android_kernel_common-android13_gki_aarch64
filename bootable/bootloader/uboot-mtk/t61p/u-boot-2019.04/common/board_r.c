// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2011 The Chromium OS Authors.
 * (C) Copyright 2002-2006
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 */

#include <common.h>
#include <api.h>
#include <image.h>
/* TODO: can we just include all these headers whether needed or not? */
#if defined(CONFIG_CMD_BEDBUG)
#include <bedbug/type.h>
#endif
#include <command.h>
#include <console.h>
#include <dm.h>
#include <environment.h>
#include <fdtdec.h>
#include <ide.h>
#include <initcall.h>
#if defined(CONFIG_CMD_KGDB)
#include <kgdb.h>
#endif
#include <malloc.h>
#include <mapmem.h>
#ifdef CONFIG_BITBANGMII
#include <miiphy.h>
#endif
#include <mmc.h>
#include <mtd.h>
#include <linux/soc/mediatek/mtk-miup.h>
#include <nand.h>
#include <of_live.h>
#include <onenand_uboot.h>
#include <scsi.h>
#include <serial.h>
#include <stdio_dev.h>
#include <timer.h>
#include <trace.h>
#include <watchdog.h>
#ifdef CONFIG_ADDR_MAP
#include <asm/mmu.h>
#endif
#include <asm/sections.h>
#include <dm/root.h>
#include <linux/compiler.h>
#include <linux/err.h>
#include <linux/ctype.h>
#include <efi_loader.h>
#include <cmdtable.h>
#include <flow.h>
#include <bootflow.h>
#ifdef CONFIG_AB_SIDELOAD
#include <mtk_ab.h>
#endif
#ifdef CONFIG_ROM_OVERLAY
#include <mtk_romtblo.h>
#endif
#include <debug_impl.h>
#include <utility.h>
#if defined(CONFIG_GPIO_HOG)
#include <asm/gpio.h>
#endif
#ifdef CONFIG_RELOAD_DEFAULT_ENV
#include <upgrade_utility.h>
#include <fs.h>
#define BASE_DECIMAL 10
#endif

#ifdef UFBL_FEATURE_IDME
#include <idme.h>
#endif
#include "one_package_impl.h"
#if defined(CONFIG_MULTICORES_PLATFORM)
#include <asm/gicv3.h>
#else
#include <asm/gic.h>
#endif
#include <asm/psci.h>
#include <linux/arm-smccc.h>
#include <thread_info.h>
#include <thread.h>
#include <amzn_tv_common.h>
#include <amzn_console.h>

#include <boot_impl.h>
#ifdef UFBL_FEATURE_REPLAY_PROTECTED_UNLOCK
#include <amzn_replay_protected_unlock.h>
#endif

DECLARE_GLOBAL_DATA_PTR;

ulong monitor_flash_len;

extern int uboot_boot_device;
extern void c_runtime_cpu_setup(void);
extern void cpu_interrupt_setting(void);

__weak int board_flash_wp_on(void)
{
	/*
	 * Most flashes can't be detected when write protection is enabled,
	 * so provide a way to let U-Boot gracefully ignore write protected
	 * devices.
	 */
	return 0;
}

__weak void cpu_secondary_init_r(void)
{
}

static int initr_secondary_cpu(void)
{
	/*
	 * after non-volatile devices & environment is setup and cpu code have
	 * another round to deal with any initialization that might require
	 * full access to the environment or loading of some image (firmware)
	 * from a non-volatile device
	 */
	/* TODO: maybe define this for all archs? */
	cpu_secondary_init_r();

	return 0;
}

static int initr_trace(void)
{
#ifdef CONFIG_TRACE
	trace_init(gd->trace_buff, CONFIG_TRACE_BUFFER_SIZE);
#endif

	return 0;
}

static int initr_reloc(void)
{
	/* tell others: relocation done */
	gd->flags |= GD_FLG_RELOC | GD_FLG_FULL_MALLOC_INIT;

	return 0;
}

#ifdef CONFIG_ARM
/*
 * Some of these functions are needed purely because the functions they
 * call return void. If we change them to return 0, these stubs can go away.
 */
static int initr_caches(void)
{
	/* Enable caches */
	enable_caches();
	return 0;
}
#endif

__weak int fixup_cpu(void)
{
	return 0;
}

static int initr_reloc_global_data(void)
{
#ifdef __ARM__
	monitor_flash_len = _end - __image_copy_start;
#elif defined(CONFIG_NDS32) || defined(CONFIG_RISCV)
	monitor_flash_len = (ulong)&_end - (ulong)&_start;
#elif !defined(CONFIG_SANDBOX) && !defined(CONFIG_NIOS2)
	monitor_flash_len = (ulong)&__init_end - gd->relocaddr;
#endif
#if defined(CONFIG_MPC85xx) || defined(CONFIG_MPC86xx)
	/*
	 * The gd->cpu pointer is set to an address in flash before relocation.
	 * We need to update it to point to the same CPU entry in RAM.
	 * TODO: why not just add gd->reloc_ofs?
	 */
	gd->arch.cpu += gd->relocaddr - CONFIG_SYS_MONITOR_BASE;

	/*
	 * If we didn't know the cpu mask & # cores, we can save them of
	 * now rather than 'computing' them constantly
	 */
	fixup_cpu();
#endif
#if !defined(CONFIG_ENV_ADDR) || defined(ENV_IS_EMBEDDED)
	/*
	 * Relocate the early env_addr pointer unless we know it is not inside
	 * the binary. Some systems need this and for the rest, it doesn't hurt.
	 */
	gd->env_addr += gd->reloc_off;
#endif
#ifdef CONFIG_OF_EMBED
	/*
	 * The fdt_blob needs to be moved to new relocation address
	 * incase of FDT blob is embedded with in image
	 */
	gd->fdt_blob += gd->reloc_off;
#endif
#ifdef CONFIG_EFI_LOADER
	efi_runtime_relocate(gd->relocaddr, NULL);
#endif

	return 0;
}

static int initr_serial(void)
{
	u64 start = 0, end = 0;

	start = get_timer(0);
	serial_initialize();
	end = get_timer(0);
	UBOOT_BOOTTIME("[%s][start:%llu][end:%llu][total time:%llu]\n", __func__, start, end, end - start);

	return 0;
}

#if defined(CONFIG_PPC) || defined(CONFIG_M68K) || defined(CONFIG_MIPS)
static int initr_trap(void)
{
	/*
	 * Setup trap handlers
	 */
#if defined(CONFIG_PPC)
	trap_init(gd->relocaddr);
#else
	trap_init(CONFIG_SYS_SDRAM_BASE);
#endif
	return 0;
}
#endif

#ifdef CONFIG_ADDR_MAP
static int initr_addr_map(void)
{
	init_addr_map();

	return 0;
}
#endif

#ifdef CONFIG_POST
static int initr_post_backlog(void)
{
	post_output_backlog();
	return 0;
}
#endif

#if defined(CONFIG_SYS_INIT_RAM_LOCK) && defined(CONFIG_E500)
static int initr_unlock_ram_in_cache(void)
{
	unlock_ram_in_cache();	/* it's time to unlock D-cache in e500 */
	return 0;
}
#endif

#ifdef CONFIG_PCI
static int initr_pci(void)
{
#ifndef CONFIG_DM_PCI
	pci_init();
#endif

	return 0;
}
#endif

static int initr_barrier(void)
{
#ifdef CONFIG_PPC
	/* TODO: Can we not use dmb() macros for this? */
	asm("sync ; isync");
#endif
	return 0;
}

#ifdef CONFIG_ROM_OVERLAY_UBOOT
static int initr_romtablo(void)
{
	unsigned long *romtbl;
	int ret;

	romtbl = (unsigned long *)CONFIG_ROM_TABLE_ADDRESS;
	init_rom_table((char *)romtbl);
	ret = check_rom_table_header();
	if( ret == ROMTBL_OK || ret == ROMTBL_NO_NEEDED)
	{
		if(ret == ROMTBL_OK)
		{
			ret = romtbl_overlay_to_uboot_dtb();
			if(ret != 0)
				UBOOT_ERROR("Do rom table overlay failure.\n");
		} else {
			UBOOT_DEBUG("No need to do rom table overlay.\n");
		}
	}
	else
	{
		UBOOT_ERROR("ROM table header is not match. Check memory address 0x%X\n",CONFIG_ROM_TABLE_ADDRESS);
	}

	return 0;
}
#endif

static int initr_dtb_dump(void)
{
	char option[]={"UDTB"};

	debug_dtb((unsigned long)gd->fdt_blob,option);
	return 0;
}

static int initr_malloc(void)
{
	ulong malloc_start;

#if CONFIG_VAL(SYS_MALLOC_F_LEN)
	debug("Pre-reloc malloc() used %#lx bytes (%ld KB)\n", gd->malloc_ptr,
	      gd->malloc_ptr / 1024);
#endif
	/* The malloc area is immediately below the monitor copy in DRAM */
	malloc_start = gd->relocaddr - TOTAL_MALLOC_LEN;
	mem_malloc_init((ulong)map_sysmem(malloc_start, TOTAL_MALLOC_LEN),
			TOTAL_MALLOC_LEN);
	return 0;
}

static int initr_console_record(void)
{
#if defined(CONFIG_CONSOLE_RECORD)
	return console_record_init();
#else
	return 0;
#endif
}

#ifdef CONFIG_SYS_NONCACHED_MEMORY
static int initr_noncached(void)
{
	noncached_init();
	return 0;
}
#endif

#ifdef CONFIG_OF_LIVE
static int initr_of_live(void)
{
	int ret;

	bootstage_start(BOOTSTAGE_ID_ACCUM_OF_LIVE, "of_live");
	ret = of_live_build(gd->fdt_blob, (struct device_node **)&gd->of_root);
	bootstage_accum(BOOTSTAGE_ID_ACCUM_OF_LIVE);
	if (ret)
		return ret;

	return 0;
}
#endif

#ifdef CONFIG_DM
static int initr_dm(void)
{
	int ret;
	u64 start = 0, end = 0;

	start = get_timer(0);
	/* Save the pre-reloc driver model and start a new one */
	gd->dm_root_f = gd->dm_root;
	gd->dm_root = NULL;
#ifdef CONFIG_TIMER
	gd->timer = NULL;
#endif
	bootstage_start(BOOTSTATE_ID_ACCUM_DM_R, "dm_r");
	ret = dm_init_and_scan(false);
	bootstage_accum(BOOTSTATE_ID_ACCUM_DM_R);
	if (ret)
		return ret;
#ifdef CONFIG_TIMER_EARLY
	ret = dm_timer_init();
	if (ret)
		return ret;
#endif

	end = get_timer(0);
	UBOOT_BOOTTIME("[%s][start:%llu][end:%llu][total time:%llu]\n", __func__, start, end, end - start);

	return 0;
}
#endif

#if IS_ENABLED(CONFIG_MTD)
static int initr_mtd(void)
{
	mtd_probe_devices();
	return 0;
}
#endif

#if IS_ENABLED(CONFIG_MTK_MIUP)
static int initr_miup(void)
{
	mtk_miup_probe_devices();
	return 0;
}
#endif

static int initr_bootstage(void)
{
	bootstage_mark_name(BOOTSTAGE_ID_START_UBOOT_R, "board_init_r");

	return 0;
}

__weak int power_init_board(void)
{
	return 0;
}

static int initr_announce(void)
{
	debug("Now running in RAM - U-Boot at: %08lx\n", gd->relocaddr);
	return 0;
}

#ifdef CONFIG_NEEDS_MANUAL_RELOC
static int initr_manual_reloc_cmdtable(void)
{
	fixup_cmdtable(ll_entry_start(cmd_tbl_t, cmd),
		       ll_entry_count(cmd_tbl_t, cmd));
	return 0;
}
#endif

#if defined(CONFIG_MTD_NOR_FLASH)
static int initr_flash(void)
{
	ulong flash_size = 0;
	bd_t *bd = gd->bd;

	puts("Flash: ");

	if (board_flash_wp_on())
		printf("Uninitialized - Write Protect On\n");
	else
		flash_size = flash_init();

	print_size(flash_size, "");
#ifdef CONFIG_SYS_FLASH_CHECKSUM
	/*
	 * Compute and print flash CRC if flashchecksum is set to 'y'
	 *
	 * NOTE: Maybe we should add some WATCHDOG_RESET()? XXX
	 */
	if (env_get_yesno("flashchecksum") == 1) {
		const uchar *flash_base = (const uchar *)CONFIG_SYS_FLASH_BASE;

		printf("  CRC: %08X", crc32(0,
					    flash_base,
					    flash_size));
	}
#endif /* CONFIG_SYS_FLASH_CHECKSUM */
	putc('\n');

	/* update start of FLASH memory    */
#ifdef CONFIG_SYS_FLASH_BASE
	bd->bi_flashstart = CONFIG_SYS_FLASH_BASE;
#endif
	/* size of FLASH memory (final value) */
	bd->bi_flashsize = flash_size;

#if defined(CONFIG_SYS_UPDATE_FLASH_SIZE)
	/* Make a update of the Memctrl. */
	update_flash_size(flash_size);
#endif

#if defined(CONFIG_OXC) || defined(CONFIG_RMU)
	/* flash mapped at end of memory map */
	bd->bi_flashoffset = CONFIG_SYS_TEXT_BASE + flash_size;
#elif CONFIG_SYS_MONITOR_BASE == CONFIG_SYS_FLASH_BASE
	bd->bi_flashoffset = monitor_flash_len;	/* reserved area for monitor */
#endif
	return 0;
}
#endif

#ifdef CONFIG_CMD_NAND
/* go init the NAND */
static int initr_nand(void)
{
	puts("NAND:  ");
	nand_init();
	printf("%lu MiB\n", nand_size() / 1024);
	return 0;
}
#endif

#if defined(CONFIG_CMD_ONENAND)
/* go init the NAND */
static int initr_onenand(void)
{
	puts("NAND:  ");
	onenand_init();
	return 0;
}
#endif

#ifdef CONFIG_MMC
static int initr_mmc(void)
{
	static int init = 0;
	u64 start = 0, end = 0;

	start = get_timer(0);
	if(init == 0)
		puts("MMC:   ");
	mmc_initialize(gd->bd);
	init++;

	end = get_timer(0);
	UBOOT_BOOTTIME("[%s][start:%llu][end:%llu][total time:%llu]\n", __func__, start, end, end - start);
	return 0;
}
#endif

#ifdef CONFIG_UFS
static int initr_ufs(void)
{
    static int i = 0;
	u64 start = 0, end = 0;

	start = get_timer(0);
    puts("UFS:   ");
    if (i == 0) {
        run_command("ufs scan",0);
        i = 1;
    }
    else
        run_command("ufs scan 0",0);
	end = get_timer(0);
	UBOOT_BOOTTIME("[%s][start:%llu][end:%llu][total time:%llu]\n", __func__, start, end, end - start);
    return 0;
}
#endif
#ifdef CONFIG_USB
unsigned int isUsbInit =0;
static int initr_usb(void)
{
	u64 start = 0, end = 0;

	start = get_timer(0);
    debug("USB:   ");
    if(get_boot_device()==EN_BOOT_DEVICE_USB)
    {
        if(isUsbInit==0)
        {
            run_command("usb start",0);
            isUsbInit=1;
        }
        else if(isUsbInit==1)
        {
            run_command("usb reset",0);
            run_command("mmc list",0);
            run_command("mmc rescan",0);
        }
    }
	end = get_timer(0);
	UBOOT_BOOTTIME("[%s][start:%llu][end:%llu][total time:%llu]\n", __func__, start, end, end - start);

    return 0;
}
#endif

#ifdef CONFIG_DTB_OVERLAY_FOR_UBOOT_DTB
static int initr_dtbo(void)
{
	int ret;
	ret = dtb_overlay_entry_point(NULL,"dtbo","u");
	if(ret != 0)
		printf("Do u-boot dtb overlay failure\n");

	return 0;
}
#endif

/*
 * Tell if it's OK to load the environment early in boot.
 *
 * If CONFIG_OF_CONTROL is defined, we'll check with the FDT to see
 * if this is OK (defaulting to saying it's OK).
 *
 * NOTE: Loading the environment early can be a bad idea if security is
 *       important, since no verification is done on the environment.
 *
 * @return 0 if environment should not be loaded, !=0 if it is ok to load
 */
static int should_load_env(void)
{
#ifdef CONFIG_OF_CONTROL
	return fdtdec_get_config_int(gd->fdt_blob, "load-environment", 1);
#elif defined CONFIG_DELAY_ENVIRONMENT
	return 0;
#else
	return 1;
#endif
}

static int check_default_env(char *name)
{
    int ret;
    char *current_value,*default_value;
    current_value = env_get(name);
    if(current_value == NULL)
    {
        default_value = env_get_default(name);
        ret = env_set(name, default_value);
        if(ret != 0)
            debug("%s env set default failure\n",name);
    }
    else
    {
        debug("%s env has value:%s\n",name,current_value);
    }
    return 0;
}

#ifdef CONFIG_NAND_BOOT
static int check_partition(void)
{
	int snprintf_len;
	char buffer[COMMAND_BUF_SIZE];
	char device_name[STORAGE_DEVICE_BUF_SIZE] = {0};
	u64 start  = 0, end = 0;

	start = get_timer(0);

	/* check UBILD exist & default partition exist or not */
	memset(buffer, 0, COMMAND_BUF_SIZE);
	snprintf_len = snprintf(buffer, COMMAND_BUF_SIZE, "ubi part %s", CONFIG_ENV_UBI_PART);
	if (snprintf_len >= COMMAND_BUF_SIZE) {
		printf("[ERROR][ubi Part] The array size is too small(%d), snprintf fail '%s'\n", COMMAND_BUF_SIZE, buffer);
		return 0;
	}
	printf("run command %s \n", buffer);
	if (run_command(buffer, 0) != 0) {
		printf("[ERROR][ubi Part Eroor] command '%s'\n", buffer);
		return 0;
	}

	/* check env volume exist*/
	memset(buffer, 0, COMMAND_BUF_SIZE);
	snprintf_len = snprintf(buffer, COMMAND_BUF_SIZE, "ubi check %s", CONFIG_ENV_UBI_VOLUME);
	if (snprintf_len >= COMMAND_BUF_SIZE) {
		printf("[ERROR][ubi check] The array size is too small(%d), snprintf fail '%s'\n", COMMAND_BUF_SIZE, buffer);
		return 0;
	}
	UBOOT_DEBUG("run command %s \n", buffer);
	if (run_command(buffer, 0) == 1) {
		printf("[ubi check] volume %s not exite,Create it\n", CONFIG_ENV_UBI_VOLUME);
		memset(buffer, 0, COMMAND_BUF_SIZE);
		snprintf_len = snprintf(buffer, COMMAND_BUF_SIZE, "ubi create %s 0x%x", CONFIG_ENV_UBI_VOLUME, CONFIG_ENV_SIZE);
		if (snprintf_len >= COMMAND_BUF_SIZE) {
		    printf("[ERROR][ubi create] The array size is too small(%d), snprintf fail '%s'\n"
				, COMMAND_BUF_SIZE, buffer);
		    return 0;
		}
	UBOOT_DEBUG("run command %s \n", buffer);
	run_command(buffer, 0);
	}

	/* check env_r volume exist*/
	memset(buffer, 0, COMMAND_BUF_SIZE);
	snprintf_len = snprintf(buffer, COMMAND_BUF_SIZE, "ubi check %s", CONFIG_ENV_UBI_VOLUME_REDUND);
	if (snprintf_len >= COMMAND_BUF_SIZE) {
		printf("[ERROR][ubi check] The array size is too small(%d), snprintf fail '%s'\n", COMMAND_BUF_SIZE, buffer);
		return 0;
	}
	UBOOT_DEBUG("run command %s \n", buffer);
	if (run_command(buffer, 0) == 1) {
		printf("[ubi check] volume %s not exite,Create it\n", CONFIG_ENV_UBI_VOLUME_REDUND);
		memset(buffer, 0, COMMAND_BUF_SIZE);
		snprintf_len = snprintf(buffer, COMMAND_BUF_SIZE, "ubi create %s 0x%x"
					, CONFIG_ENV_UBI_VOLUME_REDUND, CONFIG_ENV_SIZE);
		if (snprintf_len >= COMMAND_BUF_SIZE) {
		    printf("[ERROR][ubi create] The array size is too small(%d), snprintf fail '%s'\n"
				, COMMAND_BUF_SIZE, buffer);
		    return 0;
		}
		UBOOT_DEBUG("run command %s \n", buffer);
		run_command(buffer, 0);
	}

	end = get_timer(0);
	UBOOT_BOOTTIME("[%s][start:%llu][end:%llu][total time:%llu]\n", __func__, start, end, end - start);

	return 0;
}

#else
static int check_partition(void)
{
	int snprintf_len;
	char buffer[COMMAND_BUF_SIZE];
	char device_name[STORAGE_DEVICE_BUF_SIZE] = {0};
	u64 start  = 0, end = 0;

	start = get_timer(0);
	/* get boot device */
	if(sys_get_boot_device(device_name, sizeof(device_name))==-1)
	{
		printf("[ERROR][check_partition] Get boot device fail !! \n");
		return 0;
	}

	/* check GPT exist & default partition exist or not */
	memset(buffer, 0, COMMAND_BUF_SIZE);
	snprintf_len = snprintf(buffer, COMMAND_BUF_SIZE, "gpt check %s 0", device_name);
	if (snprintf_len >= COMMAND_BUF_SIZE)
	{
		printf("[ERROR][check_partition] The array size is too small(%d), snprintf fail '%s'\n", COMMAND_BUF_SIZE, buffer);
		return 0;
	}
	run_command(buffer, 0);
	end = get_timer(0);
	UBOOT_BOOTTIME("[%s][start:%llu][end:%llu][total time:%llu]\n", __func__, start, end, end - start);

	return 0;
}

#endif

#if defined(CONFIG_MMC) || defined(CONFIG_UFS)
static int initr_part_init(char *device_name)
{
	int snprintf_len;
	char buffer[COMMAND_BUF_SIZE];

	/* Scan partition info */
	memset(buffer, 0, COMMAND_BUF_SIZE);
	snprintf_len = snprintf(buffer, COMMAND_BUF_SIZE, "part init %s",
				device_name);
	if (snprintf_len >= COMMAND_BUF_SIZE) {
		debug("[ERROR][%s] Array is too small(%d), '%s'\n",
		      __func__, COMMAND_BUF_SIZE, buffer);
		return -1;
	}
	run_command(buffer, 0);

	return 0;
}

static int initr_part_scan(char *device_name)
{
	int snprintf_len;
	char buffer[COMMAND_BUF_SIZE];

	/* Scan partition info */
	memset(buffer, 0, COMMAND_BUF_SIZE);
	snprintf_len = snprintf(buffer, COMMAND_BUF_SIZE, "part scan %s",
				device_name);
	if (snprintf_len >= COMMAND_BUF_SIZE) {
		debug("[ERROR][%s] Array is too small(%d), '%s'\n",
		      __func__, COMMAND_BUF_SIZE, buffer);
		return -1;
	}
	run_command(buffer, 0);

	return 0;
}
#endif

#ifdef CONFIG_MMC
static int initr_mmc_part_init(void)
{
	int ret;
	u64 start = 0, end = 0;

	start = get_timer(0);
	ret = initr_part_init("mmc");
	if (ret == -1)
		debug("init mmc partition init failure.\n");

	end = get_timer(0);
	UBOOT_BOOTTIME("[%s][start:%llu][end:%llu][total time:%llu]\n", __func__, start, end, end - start);

	return 0;
}

static int initr_mmc_part_scan(void)
{
	int ret;
	u64 start = 0, end = 0;

	start = get_timer(0);
	ret = initr_part_scan("mmc");
	if (ret == -1)
		debug("init mmc partition scan failure.\n");

	end = get_timer(0);
	UBOOT_BOOTTIME("[%s][start:%llu][end:%llu][total time:%llu]\n", __func__, start, end, end - start);

	return 0;
}
#endif

#ifdef CONFIG_UFS
static int initr_ufs_part_init(void)
{
	int ret;
	u64 start = 0, end = 0;

	start = get_timer(0);
	ret = initr_part_init("ufs");
	if (ret == -1)
		debug("init ufs partition init failure.\n");

	end = get_timer(0);
	UBOOT_BOOTTIME("[%s][start:%llu][end:%llu][total time:%llu]\n", __func__, start, end, end - start);

	return 0;
}

static int initr_ufs_part_scan(void)
{
	int ret;
	u64 start  = 0, end = 0;

	start = get_timer(0);
	ret = initr_part_scan("ufs");
	if (ret == -1)
		debug("init ufs partition scan failure.\n");

	end = get_timer(0);
	UBOOT_BOOTTIME("[%s][start:%llu][end:%llu][total time:%llu]\n", __func__, start, end, end - start);

	return 0;
}
#endif

static int initr_env(void)
{
	/* initialize environment */
	u64 start  = 0, end  = 0;

	start = get_timer(0);
	if (should_load_env())
		env_relocate();
	else
		set_default_env(NULL, 0);
#ifdef CONFIG_OF_CONTROL
	env_set_hex("fdtcontroladdr",
		    (unsigned long)map_to_sysmem(gd->fdt_blob));
#endif

    check_default_env("fdt_high");
    check_default_env("initrd_high");
    check_default_env("dtboaddr");
    check_default_env("loadaddr");
	/* Initialize from environment */
	image_load_addr = env_get_ulong("loadaddr", 16, image_load_addr);
	end = get_timer(0);
	UBOOT_BOOTTIME("[%s][start:%llu][end:%llu][total time:%llu]\n", __func__, start, end, end - start);

	return 0;
}

#ifdef CONFIG_RELOAD_DEFAULT_ENV
#define ENV_PARTITION_NAME    "uenv"
static int initr_envload(void)
{
	char device_name[STORAGE_DEVICE_BUF_SIZE] = {0};
	char storage_info[STORAGE_INDEX_BUF_SIZE] = {0};
	char command[COMMAND_BUF_SIZE] = {0};
	char *env_buf = NULL;
	char *script_buf = NULL;
	unsigned int device = 0;
	loff_t file_size = 0;
	int bReloadEnv = 0;
	int env_exist = 0;
	int snprintf_len = 0;
	int ret = 0;
	u64 start = 0, end = 0;

	start = get_timer(0);
#ifdef CONFIG_ENV_IS_IN_EXT4
	ret = sys_get_boot_device(device_name, sizeof(device_name));
	if (ret < 0) {
		UBOOT_ERROR("Get boot device name failure\n");
		goto exit;
	}

	ret = sys_get_storage_info(device, ENV_PARTITION_NAME, storage_info);
	if (ret < 0) {
		UBOOT_ERROR("Get storage info failure\n");
		goto exit;
	}

	env_exist = file_exists(device_name, storage_info, CONFIG_ENV_EXT4_FILE, FS_TYPE_EXT);
	if (env_exist == 0) {
		UBOOT_ERROR("Partition 'uenv' is damaged or env file is not existed\n");
		snprintf_len = snprintf(command, COMMAND_BUF_SIZE - 1, "formatenv %s %d", device_name, device);
		if (snprintf_len < 0 || snprintf_len >= COMMAND_BUF_SIZE - 1) {
			UBOOT_ERROR("The array size is too small(%u), snprintf fail '%s'\n", COMMAND_BUF_SIZE, command);
			goto exit;
		}

		ret = run_command(command, 0);
		if (ret != CMD_RET_SUCCESS) {
			UBOOT_ERROR("Format 'uenv' partition fail\n");
			goto exit;
		} else {
			UBOOT_INFO("Reload env for 'uenv' partition is formatted\n");
			bReloadEnv = 1;
			goto reload;
		}
	}
#endif

	env_buf = env_get("uboot_default_env");
	if (env_buf) {
		ret = simple_strtoul(env_buf, NULL, BASE_DECIMAL);
		if (ret == 1) {
			UBOOT_INFO("Reload env for uboot_default_env=1\n");
			bReloadEnv = 1;
			goto reload;
		}
	}
	env_buf = env_get("bootargs");
	if (!env_buf) {
		UBOOT_INFO("Reload env for bootargs=NULL\n");
		bReloadEnv = 1;
		goto reload;
	}
	env_buf = env_get("bootcmd");
	if (!env_buf) {
		UBOOT_INFO("Reload env for bootcmd=NULL\n");
		bReloadEnv = 1;
		goto reload;
	}
	env_buf = env_get("recoverycmd");
	if (!env_buf) {
		UBOOT_INFO("Reload env for recoverycmd=NULL\n");
		bReloadEnv = 1;
		goto reload;
	}

reload:
	if (bReloadEnv == 1) {
		UBOOT_INFO("Load env from partition -> '%s' path -> '%s'\n", SET_ENV_PARTITOIN, SET_ENV_FILE);
		script_buf = (char *)read_storage_file_to_memory(SET_ENV_PARTITOIN, SET_ENV_FILE, &file_size);
		if (!script_buf) {
			UBOOT_ERROR("Read script file fail, goto exit\n");
			goto exit;
		}
		ret = runscript_linebyline(script_buf);
		if (ret != 0) {
			UBOOT_ERROR("Run script file fail, goto exit\n");
			goto exit;
		}
		env_save();
	}

exit:
	if (script_buf)
		free(script_buf);
	if (env_get("uboot_default_env")) {
		env_set("uboot_default_env", NULL);
		env_save();
	}
	end = get_timer(0);
	UBOOT_BOOTTIME("[%s][start:%llu][end:%llu][total time:%llu]\n", __func__, start, end, end - start);

	return 0;
}
#endif

#ifdef CONFIG_SYS_BOOTPARAMS_LEN
static int initr_malloc_bootparams(void)
{
	gd->bd->bi_boot_params = (ulong)malloc(CONFIG_SYS_BOOTPARAMS_LEN);
	if (!gd->bd->bi_boot_params) {
		puts("WARNING: Cannot allocate space for boot parameters\n");
		return -ENOMEM;
	}
	return 0;
}
#endif

static int initr_jumptable(void)
{
	jumptable_init();
	return 0;
}

#if defined(CONFIG_API)
static int initr_api(void)
{
	/* Initialize API */
	api_init();
	return 0;
}
#endif

/* enable exceptions */
#ifdef CONFIG_ARM
static int initr_enable_interrupts(void)
{
	enable_interrupts();
	return 0;
}
#endif

#ifdef CONFIG_CMD_KGDB
static int initr_kgdb(void)
{
	puts("KGDB:  ");
	kgdb_init();
	return 0;
}
#endif

#if defined(CONFIG_LED_STATUS)
static int initr_status_led(void)
{
#if defined(CONFIG_LED_STATUS_BOOT)
	status_led_set(CONFIG_LED_STATUS_BOOT, CONFIG_LED_STATUS_BLINKING);
#else
	status_led_init();
#endif
	return 0;
}
#endif

#if defined(CONFIG_SCSI) && !defined(CONFIG_DM_SCSI)
static int initr_scsi(void)
{
	puts("SCSI:  ");
	scsi_init();
	puts("\n");

	return 0;
}
#endif

#ifdef CONFIG_BITBANGMII
static int initr_bbmii(void)
{
	bb_miiphy_init();
	return 0;
}
#endif

#ifdef CONFIG_CMD_NET
static int initr_net(void)
{
	u64 start = 0, end = 0;

	start = get_timer(0);
	puts("Net:   ");
	eth_initialize();
#if defined(CONFIG_RESET_PHY_R)
	debug("Reset Ethernet PHY\n");
	reset_phy();
#endif
	end = get_timer(0);
	UBOOT_BOOTTIME("[%s][start:%llu][end:%llu][total time:%llu]\n", __func__, start, end, end - start);

	return 0;
}
#endif

#ifdef CONFIG_POST
static int initr_post(void)
{
	post_run(NULL, POST_RAM | post_bootmode_get(0));
	return 0;
}
#endif

#if defined(CONFIG_CMD_PCMCIA) && !defined(CONFIG_IDE)
static int initr_pcmcia(void)
{
	puts("PCMCIA:");
	pcmcia_init();
	return 0;
}
#endif

#if defined(CONFIG_IDE) && !defined(CONFIG_BLK)
static int initr_ide(void)
{
	puts("IDE:   ");
#if defined(CONFIG_START_IDE)
	if (board_start_ide())
		ide_init();
#else
	ide_init();
#endif
	return 0;
}
#endif

#if defined(CONFIG_PRAM)
/*
 * Export available size of memory for Linux, taking into account the
 * protected RAM at top of memory
 */
int initr_mem(void)
{
	ulong pram = 0;
	char memsz[32];

	pram = env_get_ulong("pram", 10, CONFIG_PRAM);
	sprintf(memsz, "%ldk", (long int)((gd->ram_size / 1024) - pram));
	env_set("mem", memsz);

	return 0;
}
#endif

#ifdef CONFIG_CMD_BEDBUG
static int initr_bedbug(void)
{
	bedbug_init();

	return 0;
}
#endif

static int valid_part(char* s)
{
	int n = strlen(s);
	int i;

	/* length of string should not be more than 3 */
	if (n > 3)
		return 0;

	/* check if the string only contains digits */
	for (i = 0; i < n; i++)
		if ((s[i] >= '0' && s[i] <= '9') == false)
			return 0;

	/* check if the number is between 0 to 255 */
	int v = simple_strtoul(s, NULL, 10);

	return (v >= 0 && v <= 255);
}

static int is_valid_ipaddr(char* pAddr)
{
	int counter = 0;
	char pStrIP[32] = {0};

	strncpy(pStrIP, pAddr, strlen(pAddr));

	char* p = strtok(pStrIP, ".");
	while (p) {
		if (valid_part(p)) {
			p = strtok(NULL, ".");
			if (p != NULL)
				++counter;
		} else {
			return 0;
		}
	}

	/* valid IP string must contain 3 dots */
	if (counter != 3)
		return 0;

	return 1;
}

static int is_valid_macaddr(char* str)
{
	int i;

	if (str == NULL)
		return 0;

	// check input string is hexdecimal digits
	for(i=0; i<12; i++)
		if (isxdigit(str[i]) == 0)
			return 0;

	return 1;
}

#ifdef UFBL_FEATURE_IDME
static int init_idme(void)
{
	int i, n;
	unsigned long bootmode = 1;
	int bNeedToSave = 0;


	char *key_pairs[][2] = {
		{"serial",   "serialno"},
		{"dev_flags","dev_flags"},
		{"usr_flags","usr_flags"},
		{"bootmode", "bootmode"},
	};

	/*
	 * Sync and update IDME values into env
	 */
	n = sizeof(key_pairs) / sizeof(key_pairs[0]);
	for (i = 0; i < n; i++) {
		char *env = NULL;
		char value[COMMAND_BUF_SIZE] = {0};

		if (!idme_get_var_external(key_pairs[i][0], value, sizeof(value)-1)) {
			UBOOT_INFO("idme %s read [%s]\n", key_pairs[i][0], value);
		} else {
			UBOOT_ERROR("idme %s read failed\n", key_pairs[i][0]);
		}
		env = env_get(key_pairs[i][1]);
		if (!env || strncmp(env, value, sizeof(value))) {
			if (value[0] != '\0') {
				env_set(key_pairs[i][1], value);
				bNeedToSave = 1;
			}
		}
	}

	/*
	 * Get eth_mac_addr from idme and set ethaddr env variable
	 */
	char *macInEnv = NULL;
	char idmemacaddr[32] = {0}, addrwithsep[48] = {0};
	if (!idme_get_var_external("eth_mac_addr", idmemacaddr, sizeof(idmemacaddr)-1) &&
	    strlen((const char*)idmemacaddr) == 12 && is_valid_macaddr(idmemacaddr)) {
		snprintf(addrwithsep, sizeof(addrwithsep), "%c%c:%c%c:%c%c:%c%c:%c%c:%c%c",
		idmemacaddr[0], idmemacaddr[1], idmemacaddr[2], idmemacaddr[3],
		idmemacaddr[4], idmemacaddr[5], idmemacaddr[6], idmemacaddr[7],
		idmemacaddr[8], idmemacaddr[9], idmemacaddr[10], idmemacaddr[11]);
		macInEnv = env_get("ethaddr");
		/* Need to update env variable ethaddr? */
		if ((!macInEnv) || (strncmp(addrwithsep, macInEnv, strlen(addrwithsep)))) {
			env_set("ethaddr", addrwithsep);
			bNeedToSave = 1;
		}
	}

	/*
	 * Get eth_ip_addr from idme and set ipaddr env variable
	 */
	#define NET_IP_ADDR_DEFAULT "192.168.1.101"
	char idmeipaddr[32] = {0};
	char ipaddrcmd[64] = {0};
	char *ip = NULL;

	if (!idme_get_var_external("eth_ip_addr", idmeipaddr, sizeof(idmeipaddr)-1) &&
	    strchr((const char*)idmeipaddr, '.') && is_valid_ipaddr(idmeipaddr)) {
		char *env_ip = env_get("ipaddr");
		if (!env_ip || strncmp(idmeipaddr, env_ip, strlen(idmeipaddr))) {
			ip = idmeipaddr;
		}
	} else {
		ip = NET_IP_ADDR_DEFAULT;
	}

	/* Need to run 'setenv' instead of calling env_set. The path through env_set
	 * prevents the global net_ip structure from being updated.
	 */
	if (ip) {
		snprintf(ipaddrcmd, sizeof(ipaddrcmd), "setenv ipaddr %s", ip);
		run_command(ipaddrcmd, 0);
		bNeedToSave = 1;
	}

	/*
	 * Bootmode
	 */
	bootmode = simple_strtoul(env_get("bootmode"), NULL, 10);

	if (bootmode > IDME_BOOTMODE_MAX) {
		env_set("bootmode", "1");
		bNeedToSave = 1;
	} else if (bootmode == IDME_BOOTMODE_TRANSITION) {
		run_command("amzn_boot transition", 0);
	} else if (bootmode == IDME_BOOTMODE_STANDBY_LOGO_POST_SHIPPING_SW_SWITCH) {
		run_command("amzn_boot reset", 0);
	}

	if (bNeedToSave)
		env_save();

	return 0;
}

static int idme_bootcount(void)
{
	idme_boot_info();
	return 0;
}
#endif

#if !defined(CONFIG_MULTICORES_PLATFORM)
static int run_main_loop(void)
{
#ifdef CONFIG_SANDBOX
	sandbox_main_loop_init();
#endif
	/* main_loop() can return to retry autoboot, if so just run it again */
	for (;;)
		main_loop();
	return 0;
}
#endif

#if defined(CONFIG_MULTICORES_PLATFORM)
smp_spin_lock_t g_print_lock;
smp_spin_lock_t cpu_init_lock;
volatile u8 cpu_init_cnt;
volatile u32 SMP_init_done;
volatile unsigned int GlobalActive_Area[NR_CPUS] = {0,};
volatile CoreWakeupFunc ActiveFuncPointer[NR_CPUS] = {0,};
#define MPIDR_MT_MASK		(ULL(1) << 24)
#define MPIDR_AFFINITY_BITS	(8)
#define DELAY_1US 1
#define TIMEOUT_US 10000000

extern void cli_loop(void);
extern void cli_init(void);
extern int secure_is_tee_fail(void);

void Core_Wakeup(CoreWakeupFunc __Addr, u32 CoreID)
{
	if (CoreID >= NR_CPUS) {
		printf("Error!! in Core_Wakeup, Not correct CPUID %d\n", CoreID);
		return;
	}

	if (GlobalActive_Area[CoreID] == 1) {
		printf("Core %d is active\n", CoreID);
		return;
	}

	//dbg_print("Wake UP Core %d\n", CoreID);
	ActiveFuncPointer[CoreID] =  __Addr;
	GlobalActive_Area[CoreID] = 1;
	flush_dcache_all();
}

void secondary_init_r(void)
{
	unsigned char cpuid = get_cpu_id();
	unsigned long irq_flag = 0;
	unsigned long timeout = 0;

	c_runtime_cpu_setup();
	initr_caches();
	cpu_interrupt_setting();

	smp_spin_lock_save(&cpu_init_lock, irq_flag);
	cpu_init_cnt++;
	smp_spin_unlock_restore(&cpu_init_lock, irq_flag);

	//dbg_print("cpu:%d ready to work!\n", cpuid);

	while (timeout < TIMEOUT_US) {
		if (GlobalActive_Area[cpuid] == 1 && ActiveFuncPointer[cpuid] != 0)
			ActiveFuncPointer[cpuid]();

		udelay(DELAY_1US);
		timeout++;
	}

	printf("%s timeout!\n", __func__);
}

#define IPI_TARGET_OFFSET_MT (16)
void ipi_send_target(u8 target_core)
{
#ifdef THREAD_DEBUG
    //unsigned char cpuid = get_cpu_id();
#endif
	u64 ipi_target = 0;

	if (read_mpidr() & MPIDR_MT_MASK)
		ipi_target =  (target_core << IPI_TARGET_OFFSET_MT) + 1;
    else
		ipi_target = ((u64)1 << target_core);
    //dbg_print("ipi_send_target core:%d to core:%d ipi_target:%llx\n", cpuid, target_core, ipi_target);
	asm volatile("msr S3_0_C12_C11_5, %0\n" : : "r" (ipi_target) : );
	asm volatile("isb");
}

void ipi_broadcast(void)
{
	u64 ipi_target = (1ULL << SGI_IRM_BIT);
#ifdef THREAD_DEBUG
	unsigned char cpuid = get_cpu_id();
#endif

	dbg_print("cpu:%d %s! ipi_target:%llx\n", __func__, cpuid, ipi_target);
	asm volatile("msr S3_0_C12_C11_5, %0\n" : : "r" (ipi_target) : );
	asm volatile("isb");
}

int secondary_start_uboot(void)
{
	unsigned long irq_flag = 0;

	if (secure_is_tee_fail() != 1) {
#ifdef CONFIG_AB_SIDELOAD
		// Need to go to reset and add retry_count for OTA case
		if (!is_fastboot_bootloader_mode())
		{
			run_command("retrycount",0);
			run_command("reset",0);
		}
#endif
		run_command("fastboot usb 0",0); // Force enter fastboot mode before jump to console to avoid fastboot reboot-bootloader failed
		cli_init();
		cli_loop();
	}

	struct arm_smccc_res res;
	u64 relocated_start = 0;
	u64 mpidr_value = 0;
	u64 secondary_core_num = 0;

	smp_spin_lock_init(&cpu_init_lock);
	smp_spin_lock_save(&cpu_init_lock, irq_flag);
	cpu_init_cnt++;
	smp_spin_unlock_restore(&cpu_init_lock, irq_flag);

	asm volatile("adr %0, _start\n\t":"=r" (relocated_start)::);

	mpidr_value = read_mpidr();
#ifdef THREAD_DEBUG
	if (mpidr_value & MPIDR_MT_MASK) {
		dbg_print("secondary_start_uboot, MPIDR MT bit is 1!\n");
	} else {
		dbg_print("secondary_start_uboot, MPIDR MT bit is 0!\n");
	}
#endif
	/* Make secondary cores (start from 1) waked from ATF and jump to uboot*/
	for (u64 i = 1; i < NR_CPUS; i++) {
		if (mpidr_value & MPIDR_MT_MASK)
			secondary_core_num = i << MPIDR_AFFINITY_BITS;
		else
			secondary_core_num = i;

		arm_smccc_smc(ARM_PSCI_0_2_FN64_CPU_ON, secondary_core_num, relocated_start, 0, 0, 0, 0, 0, &(res));
	}

	cpu_interrupt_setting();
	smp_spin_lock_init(&g_print_lock);

	while (cpu_init_cnt != NR_CPUS) {
		// Wait for secondary cores init
	}
	SMP_init_done = SMP_INIT_MAGIC;
	printf("all cores:%d are ready!\n", NR_CPUS);

	thread_start();
	return 0;
}

#endif

/*
 * We hope to remove most of the driver-related init and do it if/when
 * the driver is later used.
 *
 * TODO: perhaps reset the watchdog in the initcall function after each call?
 */
static init_fnc_t init_sequence_r[] = {
	initr_trace,
	initr_reloc,
	/* TODO: could x86/PPC have this also perhaps? */
#ifdef CONFIG_ARM
	initr_caches,
	/* Note: For Freescale LS2 SoCs, new MMU table is created in DDR.
	 *	 A temporary mapping of IFC high region is since removed,
	 *	 so environmental variables in NOR flash is not available
	 *	 until board_init() is called below to remap IFC to high
	 *	 region.
	 */
#endif
	initr_reloc_global_data,
#if defined(CONFIG_SYS_INIT_RAM_LOCK) && defined(CONFIG_E500)
	initr_unlock_ram_in_cache,
#endif
	initr_barrier,
	initr_malloc,
#ifdef CONFIG_ROM_OVERLAY_UBOOT
	initr_romtablo,
#endif
	log_init,
	amzn_init_log_buf,
	initr_bootstage,	/* Needs malloc() but has its own timer */
	initr_console_record,
#ifdef CONFIG_SYS_NONCACHED_MEMORY
	initr_noncached,
#endif
	bootstage_relocate,
#ifdef CONFIG_OF_LIVE
	initr_of_live,
#endif
#ifdef CONFIG_DM
	initr_dm,
#endif
#if defined(CONFIG_ARM) || defined(CONFIG_NDS32) || defined(CONFIG_RISCV) || \
	defined(CONFIG_SANDBOX)
	board_init,	/* Setup chipselects */
#endif
	/*
	 * TODO: printing of the clock inforamtion of the board is now
	 * implemented as part of bdinfo command. Currently only support for
	 * davinci SOC's is added. Remove this check once all the board
	 * implement this.
	 */
#ifdef CONFIG_CLOCKS
	set_cpu_clk_info, /* Setup clock information */
#endif
#ifdef CONFIG_EFI_LOADER
	efi_memory_init,
#endif
	stdio_init_tables,
	initr_serial,
	initr_announce,
	INIT_FUNC_WATCHDOG_RESET
#ifdef CONFIG_NEEDS_MANUAL_RELOC
	initr_manual_reloc_cmdtable,
#endif
#if defined(CONFIG_PPC) || defined(CONFIG_M68K) || defined(CONFIG_MIPS)
	initr_trap,
#endif
#ifdef CONFIG_ADDR_MAP
	initr_addr_map,
#endif
#if defined(CONFIG_BOARD_EARLY_INIT_R)
	board_early_init_r,
#endif
	INIT_FUNC_WATCHDOG_RESET
#ifdef CONFIG_POST
	initr_post_backlog,
#endif
	INIT_FUNC_WATCHDOG_RESET
#if defined(CONFIG_PCI) && defined(CONFIG_SYS_EARLY_PCI_INIT)
	/*
	 * Do early PCI configuration _before_ the flash gets initialised,
	 * because PCU resources are crucial for flash access on some boards.
	 */
	initr_pci,
#endif
#ifdef CONFIG_ARCH_EARLY_INIT_R
	arch_early_init_r,
#endif
	power_init_board,
#ifdef CONFIG_MTD_NOR_FLASH
	initr_flash,
#endif
	INIT_FUNC_WATCHDOG_RESET
#if defined(CONFIG_PPC) || defined(CONFIG_M68K) || defined(CONFIG_X86)
	/* initialize higher level parts of CPU like time base and timers */
	cpu_init_r,
#endif
#ifdef CONFIG_CMD_NAND
	initr_nand,
#endif
#if IS_ENABLED(CONFIG_MTD)
	initr_mtd,
#endif
#ifdef CONFIG_CMD_ONENAND
	initr_onenand,
#endif
#ifdef CONFIG_MMC
	initr_mmc,
#endif
#ifdef CONFIG_UFS
    initr_ufs,
#endif
#ifdef CONFIG_USB
    initr_usb,
#endif
#ifdef CONFIG_MMC
	initr_mmc_part_init,
#endif
#ifdef CONFIG_UFS
	initr_ufs_part_init,
#endif
	check_partition,
#ifdef CONFIG_MMC
	initr_mmc_part_scan,
#endif
#ifdef CONFIG_UFS
	initr_ufs_part_scan,
#endif
#ifdef UFBL_FEATURE_IDME
	idme_initialize,
	idme_bootcount,
#endif
	initr_env,
	_init_debug_level,
#ifdef CONFIG_RELOAD_DEFAULT_ENV
	initr_envload,
#endif
#ifdef UFBL_FEATURE_IDME
	init_idme,
#endif
#ifdef UFBL_FEATURE_REPLAY_PROTECTED_UNLOCK
	amzn_rpu_auto_relock_in_ship_mode,
#endif
#ifdef CONFIG_AB_SIDELOAD
	ab_pre_selection,
#endif
#ifdef CONFIG_DTB_OVERLAY_FOR_UBOOT_DTB
	initr_dtbo,
#ifdef CONFIG_DM
	initr_dm,
#endif
	initr_serial,
#ifdef CONFIG_MMC
	initr_mmc,
#endif
#ifdef CONFIG_UFS
	initr_ufs,
#endif
#ifdef CONFIG_USB
    initr_usb,
#endif
#endif
	initr_dtb_dump,
#if IS_ENABLED(CONFIG_MTK_MIUP)
	initr_miup,
#endif
#ifdef CONFIG_SYS_BOOTPARAMS_LEN
	initr_malloc_bootparams,
#endif
	INIT_FUNC_WATCHDOG_RESET
	initr_secondary_cpu,
#if defined(CONFIG_ID_EEPROM) || defined(CONFIG_SYS_I2C_MAC_OFFSET)
	mac_read_from_eeprom,
#endif
	INIT_FUNC_WATCHDOG_RESET
#if defined(CONFIG_PCI) && !defined(CONFIG_SYS_EARLY_PCI_INIT)
	/*
	 * Do pci configuration
	 */
	initr_pci,
#endif
	stdio_add_devices,
	initr_jumptable,
#ifdef CONFIG_API
	initr_api,
#endif
	console_init_r,		/* fully init console as a device */
#ifdef CONFIG_DISPLAY_BOARDINFO_LATE
	console_announce_r,
	show_board_info,
#endif
#ifdef CONFIG_ARCH_MISC_INIT
	arch_misc_init,		/* miscellaneous arch-dependent init */
#endif
#ifdef CONFIG_MISC_INIT_R
	misc_init_r,		/* miscellaneous platform-dependent init */
#endif
	INIT_FUNC_WATCHDOG_RESET
#ifdef CONFIG_CMD_KGDB
	initr_kgdb,
#endif
	interrupt_init,
#ifdef CONFIG_ARM
	initr_enable_interrupts,
#endif
#if defined(CONFIG_MICROBLAZE) || defined(CONFIG_M68K)
	timer_init,		/* initialize timer */
#endif
#if defined(CONFIG_LED_STATUS)
	initr_status_led,
#endif
	/* PPC has a udelay(20) here dating from 2002. Why? */
#if defined(CONFIG_GPIO_HOG)
	gpio_hog_probe_all,
#endif
#ifdef CONFIG_BOARD_LATE_INIT
	board_late_init,
#endif
#if defined(CONFIG_SCSI) && !defined(CONFIG_DM_SCSI)
	INIT_FUNC_WATCHDOG_RESET
	initr_scsi,
#endif
#ifdef CONFIG_BITBANGMII
	initr_bbmii,
#endif
#ifdef CONFIG_CMD_NET
	INIT_FUNC_WATCHDOG_RESET
	initr_net,
#endif
#ifdef CONFIG_POST
	initr_post,
#endif
#if defined(CONFIG_CMD_PCMCIA) && !defined(CONFIG_IDE)
	initr_pcmcia,
#endif
#if defined(CONFIG_IDE) && !defined(CONFIG_BLK)
	initr_ide,
#endif
#ifdef CONFIG_LAST_STAGE_INIT
	INIT_FUNC_WATCHDOG_RESET
	/*
	 * Some parts can be only initialized if all others (like
	 * Interrupts) are up and running (i.e. the PC-style ISA
	 * keyboard).
	 */
	last_stage_init,
#endif
#ifdef CONFIG_CMD_BEDBUG
	INIT_FUNC_WATCHDOG_RESET
	initr_bedbug,
#endif
#if defined(CONFIG_PRAM)
	initr_mem,
#endif

	mtk_init_command_table,
	do_basic_command_register,
#if defined(CONFIG_DIAG_TRANSITION_DIALOG)
	initr_diag_fos_trans_screen,
#endif
	do_customized_command_register,
#ifdef CONFIG_MULTICORES_PLATFORM
	secondary_start_uboot,
#else
	do_after_uboot_init,
	run_main_loop,
#endif
};

void board_init_r(gd_t *new_gd, ulong dest_addr)
{
	/*
	 * Set up the new global data pointer. So far only x86 does this
	 * here.
	 * TODO(sjg@chromium.org): Consider doing this for all archs, or
	 * dropping the new_gd parameter.
	 */
#if CONFIG_IS_ENABLED(X86_64)
	arch_setup_gd(new_gd);
#endif

#ifdef CONFIG_NEEDS_MANUAL_RELOC
	int i;
#endif

#if !defined(CONFIG_X86) && !defined(CONFIG_ARM) && !defined(CONFIG_ARM64)
	gd = new_gd;
#endif
	gd->flags &= ~GD_FLG_LOG_READY;

#ifdef CONFIG_NEEDS_MANUAL_RELOC
	for (i = 0; i < ARRAY_SIZE(init_sequence_r); i++)
		init_sequence_r[i] += gd->reloc_off;
#endif

	if (initcall_run_list(init_sequence_r))
		hang();

	/* NOTREACHED - run_main_loop() does not return */
	hang();
}
