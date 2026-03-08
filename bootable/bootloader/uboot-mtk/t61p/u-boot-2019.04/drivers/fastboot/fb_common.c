// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2008 - 2009
 * Windriver, <www.windriver.com>
 * Tom Rix <Tom.Rix@windriver.com>
 *
 * Copyright 2011 Sebastian Andrzej Siewior <bigeasy@linutronix.de>
 *
 * Copyright 2014 Linaro, Ltd.
 * Rob Herring <robh@kernel.org>
 */

#include <bcb.h>
#include <common.h>
#include <command.h>
//#include <env.h>
#include <fastboot.h>
#include <net/fastboot.h>
#include <memalign.h>

/**
 * fastboot_buf_addr - base address of the fastboot download buffer
 */
void *fastboot_buf_addr;

/**
 * fastboot_buf_size - size of the fastboot download buffer
 */
u32 fastboot_buf_size;

/**
 * fastboot_progress_callback - callback executed during long operations
 */
void (*fastboot_progress_callback)(const char *msg);

/**
 * fastboot_response() - Writes a response of the form "$tag$reason".
 *
 * @tag: The first part of the response
 * @response: Pointer to fastboot response buffer
 * @format: printf style format string
 */
void fastboot_response(const char *tag, char *response,
		       const char *format, ...)
{
	va_list args;

	strlcpy(response, tag, FASTBOOT_RESPONSE_LEN);
	if (format) {
		va_start(args, format);
		vsnprintf(response + strlen(response),
			  FASTBOOT_RESPONSE_LEN - strlen(response) - 1,
			  format, args);
		va_end(args);
	}
}

/**
 * fastboot_fail() - Write a FAIL response of the form "FAIL$reason".
 *
 * @reason: Pointer to returned reason string
 * @response: Pointer to fastboot response buffer
 */
void fastboot_fail(const char *reason, char *response)
{
	fastboot_response("FAIL", response, "%s", reason);
}

/**
 * fastboot_okay() - Write an OKAY response of the form "OKAY$reason".
 *
 * @reason: Pointer to returned reason string, or NULL to send a bare "OKAY"
 * @response: Pointer to fastboot response buffer
 */
void fastboot_okay(const char *reason, char *response)
{
	if (reason)
		fastboot_response("OKAY", response, "%s", reason);
	else
		fastboot_response("OKAY", response, NULL);
}

/**
 * fastboot_set_reboot_flag() - Set flag to indicate reboot-bootloader
 *
 * Set flag which indicates that we should reboot into the bootloader
 * following the reboot that fastboot executes after this function.
 *
 * This function should be overridden in your board file with one
 * which sets whatever flag your board specific Android bootloader flow
 * requires in order to re-enter the bootloader.
 */
int __weak fastboot_set_reboot_flag(enum fastboot_reboot_reason reason)
{
#if CONFIG_IS_ENABLED(FASTBOOT_FLASH)
	static const char * const boot_cmds[] = {
		[FASTBOOT_REBOOT_REASON_BOOTLOADER] = "bootonce-bootloader",
		[FASTBOOT_REBOOT_REASON_FASTBOOTD] = "boot-fastboot",
		[FASTBOOT_REBOOT_REASON_RECOVERY] = "boot-recovery"
	};
	static const char * const recovery_cmds="recovery\n--fastboot";

	if (reason >= FASTBOOT_REBOOT_REASONS_COUNT || reason < 0)
		return -EINVAL;

	if( bcb_write_reboot_reason(CONFIG_FASTBOOT_FLASH_MMC_DEV, "misc", (char *)boot_cmds[reason])!=CMD_RET_SUCCESS)
		return -EINVAL;
	if( bcb_write_recovery_reason(CONFIG_FASTBOOT_FLASH_MMC_DEV, "misc", (char *)recovery_cmds)!=CMD_RET_SUCCESS)
		return -EINVAL;
	return 0;
#else
	return -EINVAL;
#endif
}

/**
 * fastboot_get_progress_callback() - Return progress callback
 *
 * Return: Pointer to function called during long operations
 */
void (*fastboot_get_progress_callback(void))(const char *)
{
	return fastboot_progress_callback;
}

/**
 * fastboot_boot() - Execute fastboot boot command
 *
 * If ${fastboot_bootcmd} is set, run that command to execute the boot
 * process, if that returns, then exit the fastboot server and return
 * control to the caller.
 *
 * Otherwise execute "bootm <fastboot_buf_addr>", if that fails, reset
 * the board.
 */
void fastboot_boot(void)
{
	char *s;

	s = env_get("fastboot_bootcmd");
	if (s) {
		run_command(s, CMD_FLAG_ENV);
	} else {
		static char boot_addr_start[20];
		static char *const bootm_args[] = {
			"bootm", boot_addr_start, NULL
		};

		snprintf(boot_addr_start, sizeof(boot_addr_start) - 1,
			 "0x%p", fastboot_buf_addr);
		printf("Booting kernel at %s...\n\n\n", boot_addr_start);

		do_bootm(NULL, 0, 2, bootm_args);

		/*
		 * This only happens if image is somehow faulty so we start
		 * over. We deliberately leave this policy to the invocation
		 * of fastbootcmd if that's what's being run
		 */
		do_reset(NULL, 0, 0, NULL);
	}
}

/**
 * fastboot_set_progress_callback() - set progress callback
 *
 * @progress: Pointer to progress callback
 *
 * Set a callback which is invoked periodically during long running operations
 * (flash and erase). This can be used (for example) by the UDP transport to
 * send INFO responses to keep the client alive whilst those commands are
 * executing.
 */
void fastboot_set_progress_callback(void (*progress)(const char *msg))
{
	fastboot_progress_callback = progress;
}

/*
 * fastboot_init() - initialise new fastboot protocol session
 *
 * @buf_addr: Pointer to download buffer, or NULL for default
 * @buf_size: Size of download buffer, or zero for default
 */
void fastboot_init(void *buf_addr, u32 buf_size)
{
	fastboot_buf_addr = buf_addr ? buf_addr :
				       (void *)CONFIG_FASTBOOT_BUF_ADDR;
	fastboot_buf_size = buf_size ? buf_size : CONFIG_FASTBOOT_BUF_SIZE;
	fastboot_set_progress_callback(NULL);
}

#if defined(CONFIG_ANDROID_AB)
/*
 * get_snapshot_merge_status() - get vab snapshot merge status
 *
 * @misc_vab_message: Pointer to store vab merge status message
 */
int get_snapshot_merge_status(struct misc_virtual_ab_message *misc_vab_message)
{
	struct blk_desc *dev_desc;
	disk_partition_t part_info;
	char *dev_iface = "mmc";
	char *dev_num_part = "0#misc";
	ulong cnt, offset;

	if (!misc_vab_message) {
		return -EINVAL;
	}

	/* Lookup the "misc" partition */
	if (part_get_info_by_dev_and_name_or_num(
		dev_iface, dev_num_part, &dev_desc, &part_info) < 0) {
		printf("Error: Could not get misc partition info\n");
		return -ENODEV;
	}

	cnt = DIV_ROUND_UP(sizeof(struct misc_virtual_ab_message), part_info.blksz);
	offset = SYSTEM_SPACE_OFFSET_IN_MISC / part_info.blksz;
	if ((cnt + offset) > part_info.size) {
		printf("Error: %s:%s too small! (cnt %ld, offset %ld, blksz %ld, size %ld)",
			dev_iface, dev_num_part, cnt, offset, part_info.blksz, part_info.size);
		return -EINVAL;
	}

	ALLOC_CACHE_ALIGN_BUFFER(u8, vab_buf, cnt * part_info.blksz);
	if (blk_dread(dev_desc, part_info.start + offset, cnt, vab_buf) != cnt) {
		printf("Error: Could not read blk from misc partition\n");
		return -EIO;
	}

	memcpy(misc_vab_message, vab_buf, sizeof(struct misc_virtual_ab_message));

	printf("misc_vab_message.version: %d\n", misc_vab_message->version);
	printf("misc_vab_message.magic: 0x%x\n", misc_vab_message->magic);
	printf("misc_vab_message.merge_status: %d\n", misc_vab_message->merge_status);
	printf("misc_vab_message.source_slot: %d\n", misc_vab_message->source_slot);

	return 0;
}
#endif
