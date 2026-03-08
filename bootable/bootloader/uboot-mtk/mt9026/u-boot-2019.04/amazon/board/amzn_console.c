/*
 * Copyright (c) 2023 Amazon.com, Inc. or its affiliates.  All rights reserved.
 */

#include <malloc.h>
#include <stdio_dev.h>
#include <string.h>
#include <common.h>
#include <idme.h>
#if defined(UFBL_FEATURE_IDME)
#include <utility.h>
#endif
#include <idme_utilities.h>
#include <amzn_console.h>
#include <amzn_tv_secure_boot.h>
#include <amzn_tv_common.h>

/* save uboot log and pass it kernel for access */
static char *amzn_log_buf = NULL;
static int  amzn_log_begin = 0;
static int  amzn_log_end   = 0;
static int  amzn_log_init  = 0;

int amzn_init_log_buf()
{
	amzn_log_buf   = malloc(AMZN_LOG_SIZE);
	amzn_log_begin = 0;
	amzn_log_end   = -1;
	amzn_log_init  = AMZN_LOG_MAGIC;
	return 0;
}

void amzn_save_log(const char *s)
{
	if ( (amzn_log_init != AMZN_LOG_MAGIC) || (amzn_log_buf == NULL) )
		return;

	int trail_len, first_len, second_len;
	int msglen = (int)strlen(s);
	if (msglen > 0) {
		trail_len = AMZN_LOG_SIZE - amzn_log_end + 1;
		first_len = trail_len > msglen ?  msglen : trail_len;
		second_len = msglen - first_len;
		strncpy(amzn_log_buf+amzn_log_end+1, s, first_len);
		if (second_len > 0) {
			strncpy(amzn_log_buf, s+first_len, second_len);
			amzn_log_begin = second_len;
			amzn_log_end   = second_len - 1;
		} else {
			amzn_log_end   = amzn_log_end + first_len;
			if (amzn_log_begin) {
				amzn_log_begin = (amzn_log_end == AMZN_LOG_SIZE - 1) ?
						0 : amzn_log_end + 1;
			}
		}
	}
}

void amzn_get_log(char *log_buf)
{
	if (!log_buf)
		return;

	if (amzn_log_end == -1) /* log buffer empty */
		return;

	unsigned int logsize = (amzn_log_begin > amzn_log_end) ?
				AMZN_LOG_SIZE : amzn_log_end - amzn_log_begin + 1;
	if (amzn_log_begin > amzn_log_end) {
		strncpy(log_buf, (const char*)(amzn_log_buf+amzn_log_begin), AMZN_LOG_SIZE-amzn_log_begin);
		strncpy(log_buf+AMZN_LOG_SIZE-amzn_log_begin, (const char*)amzn_log_buf, amzn_log_end+1);
	} else {
		strncpy(log_buf, (const char*)(amzn_log_buf+amzn_log_begin), amzn_log_end-amzn_log_begin+1);
	}
	log_buf[logsize] = 0;

	return;
}

int amzn_uart_disable(void)
{
	bool fos_flags_console_on = false;
	bool is_eng = true;

#ifdef UFBL_FEATURE_IDME
	fos_flags_console_on = idme_is_fos_flag_set(FOS_FLAGS_CONSOLE_ON);
#endif

	is_eng = !target_is_production() ||
		 amzn_device_is_unlocked() ||
		 simple_strtoul(env_get("bootmode"), NULL, 10) == IDME_BOOTMODE_DIAG;

	if (is_eng && fos_flags_console_on) {
		printf("unlocked %d, console_on %d\n", is_eng, fos_flags_console_on);
	} else {
		printf("Disabling UART: unlocked %d, console_on %d\n",
		       is_eng, fos_flags_console_on);
		serial_disbale();
	}
	return 0;
}

#if defined(UFBL_FEATURE_IDME)
int amzn_diag_uartport_change(void)
{
	#define DIAG_UART_VAR_SIZE 32
	#define DIAG_BOOT_CONSOLE_VAR_SIZE 32
	unsigned int bootmode;
	unsigned int tty_port_num;
	char diag_console_tty_port[DIAG_UART_VAR_SIZE] = "\0";
	char androidboot_console[DIAG_BOOT_CONSOLE_VAR_SIZE] = "\0";

	bootmode = simple_strtoul(env_get("bootmode"), NULL, 10);
	idme_get_oem_data_field("diag_c_tty=", diag_console_tty_port, DIAG_UART_VAR_SIZE);

	if (diag_console_tty_port[0] != 0 && bootmode == IDME_BOOTMODE_DIAG) {
		tty_port_num = simple_strtoul(diag_console_tty_port, NULL, 0);
		snprintf(androidboot_console, sizeof (androidboot_console),
				"androidboot.console=ttyS%d", tty_port_num);
		do {
			printf("[Diag]Delete androidboot.console\n");
			del_bootargs("androidboot.console", 0);
		} while(find_bootargs("androidboot.console"));
		printf("[Diag]Set %s\n", androidboot_console);
		add_bootargs("androidboot.console", androidboot_console, 0);
	}
	return 0;
}
#endif
