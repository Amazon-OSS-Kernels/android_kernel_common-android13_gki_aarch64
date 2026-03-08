// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (C) 2016 The Android Open Source Project
 */

#include <common.h>
#include <command.h>
#include <environment.h>
#include <fastboot.h>
#include <fastboot-internal.h>
#include <fb_mmc.h>
#include <fb_nand.h>
#include <flash.h>
#include <part.h>
#include <stdlib.h>
#include <debug_impl.h>
#include <image-sparse.h>
#include <partition.h>

#include <upgrade_utility.h>

#ifdef UFBL_FEATURE_IDME
#include <idme.h>

#ifdef UFBL_FEATURE_FOS_FLAGS
#include <fos_flags.h>
#endif
#endif

#ifdef UFBL_FEATURE_UNLOCK
#include <amzn_unlock.h>
#endif

#ifdef UFBL_FEATURE_REPLAY_PROTECTED_UNLOCK
#include <amzn_replay_protected_unlock.h>
#endif

#include <amzn_tv_secure_boot.h>

/**
 * image_size - final fastboot image size
 */
static u32 image_size;

/**
 * fastboot_bytes_received - number of bytes received in the current download
 */
static u32 fastboot_bytes_received;

/**
 * fastboot_bytes_expected - number of bytes expected in the current download
 */
static u32 fastboot_bytes_expected;

#define CMD_BUF_SIZE 32
#define FASTBOOT_GPT_NAME "gpt"
#define FASTBOOT_GPT_NAME_LEN 3

static void okay(char *, char *);
static void getvar(char *, char *);
static void download(char *, char *);
#if CONFIG_IS_ENABLED(FASTBOOT_FLASH)
static void flash(char *, char *);
static void erase(char *, char *);
static void flashing(char *, char *);
#endif
static void reboot_bootloader(char *, char *);
static void reboot_fastbootd(char *, char *);
static void reboot_recovery(char *, char *);
static void set_active(char *, char *);
static void oem_set_serialno(char *, char *);
#if CONFIG_IS_ENABLED(FASTBOOT_CMD_OEM_FORMAT)
static void oem_format(char *, char *);
#endif

#ifdef UFBL_FEATURE_IDME
static void fbcmd_oem_idme(char *, char *);
#ifdef UFBL_FEATURE_FOS_FLAGS
static void fbcmd_oem_flags(char *, char *);
#endif
#ifdef UFBL_FEATURE_UNLOCK
static void fbcmd_oem_relock(char *, char *);
#endif
#endif

static const struct {
	const char *command;
	void (*dispatch)(char *cmd_parameter, char *response);
} commands[FASTBOOT_COMMAND_COUNT] = {
	[FASTBOOT_COMMAND_GETVAR] = {
		.command = "getvar",
		.dispatch = getvar
	},
	[FASTBOOT_COMMAND_DOWNLOAD] = {
		.command = "download",
		.dispatch = download
	},
#if CONFIG_IS_ENABLED(FASTBOOT_FLASH)
	[FASTBOOT_COMMAND_FLASH] =  {
		.command = "flash",
		.dispatch = flash
	},
	[FASTBOOT_COMMAND_ERASE] =  {
		.command = "erase",
		.dispatch = erase
	},
	[FASTBOOT_COMMAND_FLASHING] = {
		.command = "flashing",
		.dispatch = flashing,
	},
#endif
	[FASTBOOT_COMMAND_BOOT] =  {
		.command = "boot",
		.dispatch = okay
	},
	[FASTBOOT_COMMAND_CONTINUE] =  {
		.command = "continue",
		.dispatch = okay
	},
	[FASTBOOT_COMMAND_REBOOT] =  {
		.command = "reboot",
		.dispatch = okay
	},
	[FASTBOOT_COMMAND_REBOOT_BOOTLOADER] =  {
		.command = "reboot-bootloader",
		.dispatch = reboot_bootloader
	},
	[FASTBOOT_COMMAND_REBOOT_FASTBOOTD] =  {
		.command = "reboot-fastboot",
		.dispatch = reboot_fastbootd
	},
	[FASTBOOT_COMMAND_REBOOT_RECOVERY] =  {
		.command = "reboot-recovery",
		.dispatch = reboot_recovery
	},
	[FASTBOOT_COMMAND_SET_ACTIVE] =  {
		.command = "set_active",
		.dispatch = set_active
	},
	[FASTBOOT_COMMAND_OEM_SET_SERIALNO] =  {
		.command = "oem set_serialno",
		.dispatch = oem_set_serialno
	},
#if CONFIG_IS_ENABLED(FASTBOOT_CMD_OEM_FORMAT)
	[FASTBOOT_COMMAND_OEM_FORMAT] = {
		.command = "oem format",
		.dispatch = oem_format,
	},
#endif
#if CONFIG_IS_ENABLED(FASTBOOT_CMD_OEM_PARTCONF)
	[FASTBOOT_COMMAND_OEM_PARTCONF] = {
		.command = "oem partconf",
		.dispatch = oem_partconf,
	},
#endif
#if CONFIG_IS_ENABLED(FASTBOOT_CMD_OEM_BOOTBUS)
	[FASTBOOT_COMMAND_OEM_BOOTBUS] = {
		.command = "oem bootbus",
		.dispatch = oem_bootbus,
	},
#endif
#if CONFIG_IS_ENABLED(FASTBOOT_UUU_SUPPORT)
	[FASTBOOT_COMMAND_UCMD] = {
		.command = "UCmd",
		.dispatch = run_ucmd,
	},
	[FASTBOOT_COMMAND_ACMD] = {
		.command = "ACmd",
		.dispatch = run_acmd,
	},
#endif
#ifdef UFBL_FEATURE_IDME
	[FASTBOOT_COMMAND_OEM_IDME] = {
		.command = "oem idme",
		.dispatch = fbcmd_oem_idme,
	},
#ifdef UFBL_FEATURE_FOS_FLAGS
	[FASTBOOT_COMMAND_OEM_FLAGS] = {
		.command = "oem flags",
		.dispatch = fbcmd_oem_flags,
	},
#endif
#if defined(UFBL_FEATURE_UNLOCK)
	[FASTBOOT_COMMAND_OEM_RELOCK] = {
		.command = "oem relock",
		.dispatch = fbcmd_oem_relock,
	},
#endif
#endif

};

/**
 * fastboot_handle_command - Handle fastboot command
 *
 * @cmd_string: Pointer to command string
 * @response: Pointer to fastboot response buffer
 *
 * Return: Executed command, or -1 if not recognized
 */
int fastboot_handle_command(char *cmd_string, char *response)
{
	int i;
	char *cmd_parameter;

	cmd_parameter = cmd_string;

	if (strncmp(cmd_parameter, "flashing", strlen("flashing")) == 0) {
		strsep(&cmd_parameter, " ");
#ifdef UFBL_FEATURE_IDME
	} else if (!strncmp(cmd_parameter, commands[FASTBOOT_COMMAND_OEM_IDME].command,
	                    strlen(commands[FASTBOOT_COMMAND_OEM_IDME].command))) {
		cmd_parameter += strlen(commands[FASTBOOT_COMMAND_OEM_IDME].command);
		cmd_parameter[0] = '\0';
		cmd_parameter += 1;
#ifdef UFBL_FEATURE_FOS_FLAGS
	} else if (!strncmp(cmd_parameter, commands[FASTBOOT_COMMAND_OEM_FLAGS].command,
	                    strlen(commands[FASTBOOT_COMMAND_OEM_FLAGS].command))) {
		cmd_parameter += strlen(commands[FASTBOOT_COMMAND_OEM_FLAGS].command);
		cmd_parameter[0] = '\0';
		cmd_parameter += 1;
#endif
#endif
	}
	else {
		strsep(&cmd_parameter, ":");
	}

	for (i = 0; i < FASTBOOT_COMMAND_COUNT; i++)
	{
		if (!strcmp(commands[i].command, cmd_string)) {
			if (commands[i].dispatch) {
				printf("command ID =%d\n",i);
				printf("cmd_string =%s\n",cmd_string);
				printf("cmd_parameter =%s\n",cmd_parameter);
#if defined(UFBL_FEATURE_UNLOCK)
				if ((is_lockdown()) &&
					 (i != FASTBOOT_COMMAND_FLASH) &&
					 (i != FASTBOOT_COMMAND_GETVAR) &&
					 (i != FASTBOOT_COMMAND_REBOOT) &&
					 (i != FASTBOOT_COMMAND_DOWNLOAD) &&
					 (i != FASTBOOT_COMMAND_REBOOT_BOOTLOADER))
				{
					fastboot_fail("command not supported in lockdown", response);
					return -1;
				}
#endif
				commands[i].dispatch(cmd_parameter, response);
				return i;
			} else {
				break;
			}
		}
	}

	pr_err("command %s not recognized.\n", cmd_string);
	fastboot_fail("unrecognized command", response);
	return -1;
}

/**
 * okay() - Send bare OKAY response
 *
 * @cmd_parameter: Pointer to command parameter
 * @response: Pointer to fastboot response buffer
 *
 * Send a bare OKAY fastboot response. This is used where the command is
 * valid, but all the work is done after the response has been sent (e.g.
 * boot, reboot etc.)
 */
static void okay(char *cmd_parameter, char *response)
{
	fastboot_okay(NULL, response);
}

/**
 * getvar() - Read a config/version variable
 *
 * @cmd_parameter: Pointer to command parameter
 * @response: Pointer to fastboot response buffer
 */
static void getvar(char *cmd_parameter, char *response)
{
	fastboot_getvar(cmd_parameter, response);
}

/**
 * fastboot_download() - Start a download transfer from the client
 *
 * @cmd_parameter: Pointer to command parameter
 * @response: Pointer to fastboot response buffer
 */
static void download(char *cmd_parameter, char *response)
{
	char *tmp;

	if (!cmd_parameter) {
		fastboot_fail("Expected command parameter", response);
		return;
	}
	fastboot_bytes_received = 0;
	fastboot_bytes_expected = simple_strtoul(cmd_parameter, &tmp, 16);
	if (fastboot_bytes_expected == 0) {
		fastboot_fail("Expected nonzero image size", response);
		return;
	}
	/*
	 * Nothing to download yet. Response is of the form:
	 * [DATA|FAIL]$cmd_parameter
	 *
	 * where cmd_parameter is an 8 digit hexadecimal number
	 */
	if (fastboot_bytes_expected > fastboot_buf_size) {
		fastboot_fail(cmd_parameter, response);
	} else {
		printf("Starting download of %d bytes\n",
		       fastboot_bytes_expected);
		fastboot_response("DATA", response, "%s", cmd_parameter);
	}
}

/**
 * fastboot_data_remaining() - return bytes remaining in current transfer
 *
 * Return: Number of bytes left in the current download
 */
u32 fastboot_data_remaining(void)
{
	return fastboot_bytes_expected - fastboot_bytes_received;
}

/**
 * fastboot_data_download() - Copy image data to fastboot_buf_addr.
 *
 * @fastboot_data: Pointer to received fastboot data
 * @fastboot_data_len: Length of received fastboot data
 * @response: Pointer to fastboot response buffer
 *
 * Copies image data from fastboot_data to fastboot_buf_addr. Writes to
 * response. fastboot_bytes_received is updated to indicate the number
 * of bytes that have been transferred.
 *
 * On completion sets image_size and ${filesize} to the total size of the
 * downloaded image.
 */
void fastboot_data_download(const void *fastboot_data,
			    unsigned int fastboot_data_len,
			    char *response)
{
#define BYTES_PER_DOT	0x20000
	u32 pre_dot_num, now_dot_num;

	if (fastboot_data_len == 0 ||
	    (fastboot_bytes_received + fastboot_data_len) >
	    fastboot_bytes_expected) {
		fastboot_fail("Received invalid data length",
			      response);
		return;
	}
	/* Download data to fastboot_buf_addr */
	memcpy(fastboot_buf_addr + fastboot_bytes_received,
	       fastboot_data, fastboot_data_len);

	pre_dot_num = fastboot_bytes_received / BYTES_PER_DOT;
	fastboot_bytes_received += fastboot_data_len;
	now_dot_num = fastboot_bytes_received / BYTES_PER_DOT;

	if (pre_dot_num != now_dot_num) {
		putc('.');
		if (!(now_dot_num % 74))
			putc('\n');
	}
	*response = '\0';
}

/**
 * fastboot_data_complete() - Mark current transfer complete
 *
 * @response: Pointer to fastboot response buffer
 *
 * Set image_size and ${filesize} to the total size of the downloaded image.
 */
void fastboot_data_complete(char *response)
{
	/* Download complete. Respond with "OKAY" */
	fastboot_okay(NULL, response);
	printf("\ndownloading of %d bytes finished\n", fastboot_bytes_received);
	image_size = fastboot_bytes_received;
	env_set_hex("filesize", image_size);
	fastboot_bytes_expected = 0;
	fastboot_bytes_received = 0;
}

#if CONFIG_IS_ENABLED(FASTBOOT_FLASH)
/* Ported by Zhendong Wang starts for fastboot supporting packed image */

/* Note: This function must accordate with the the u32_to_u8 function in
 * image_pack.c. Otherwise, the parsed u32 is incorrect
 */
static void u8_array_to_u32( const u8 * array, u32 * num ) {
    u32 i = 0;
    *num = 0;
    for( i = 0; i < 4; i++ ) {
        *num <<= 8;
        *num += array[i];
    }
}
#define COMD_BUFFER_LEN 128
#define BOOT_PAR_1 1
#define BOOT_PAR_2 2
static int do_update_packed_images(char *response)
{
    int i;
    u8 *pTmp;
    const char *name;
    img_hdr *images_header;
    sparse_header_t *sparse_header;
    struct disk_partition info= {0};;
    u32 size=0;
    u32 num_images=0;
    u32 images_size=0;
    u32 header_size=0;
#ifdef CONFIG_FASTBOOT_MMC_BOOT_SUPPORT
    char cmd[COMD_BUFFER_LEN];
    int snprintf_len=0;
    int bootpart =0;
#endif
    img_info_tmp *img_info;
    images_header = fastboot_buf_addr;
    pTmp = fastboot_buf_addr;

    u8_array_to_u32((const u8 *)&images_header->num_images, &num_images);
    u8_array_to_u32((const u8 *)&images_header->images_size, &images_size);
    u8_array_to_u32((const u8 *)&images_header->header_size, &header_size);
    if (num_images < 1) {
        printf("This packed images contain no image.\n");
        fastboot_fail("The packed images contain no image",response);
        return MULTI_IMAGE_ERROR;
    }

    pTmp += header_size;
	img_info = (img_info_tmp *)images_header->img_info;

    for (i = 0; i < num_images; i++)
    {
        name = (const char *)img_info[i].name;
        u8_array_to_u32((const u8 *)&img_info[i].size, &size);
        printf("flashing %s\n", name);
#ifdef CONFIG_FASTBOOT_MMC_BOOT_SUPPORT
        if (strcmp(name, CONFIG_FASTBOOT_MMC_BOOT1_NAME) == 0)
        {
            bootpart=BOOT_PAR_1;
        }
        else if (strcmp(name, CONFIG_FASTBOOT_MMC_BOOT2_NAME) == 0)
        {
            bootpart=BOOT_PAR_2;
        }
        else
#endif
        {
            if ((get_boot_partition_info(name, &info, NULL) != 0) &&
                (strncmp(name, FASTBOOT_GPT_NAME, FASTBOOT_GPT_NAME_LEN) != 0))
            {
                printf("The partition %s is not defined.\n", name);
                fastboot_fail("The partition is not defined",response);
                return MULTI_IMAGE_ERROR;
            }
            if ((strncmp(name, FASTBOOT_GPT_NAME, FASTBOOT_GPT_NAME_LEN) != 0) &&
                size > (info.size * info.blksz)) {
                printf("Image size %d is larger than the partition size %lu.\n", size, (info.size * info.blksz));
                fastboot_fail("Image size is larger than the partition size",response);
                return MULTI_IMAGE_ERROR;
            }
        }

        sparse_header = (sparse_header_t *)pTmp;
        printf("Write partition %s.\n", name);
#ifdef CONFIG_FASTBOOT_MMC_BOOT_SUPPORT
        if(strcmp(name, CONFIG_FASTBOOT_MMC_BOOT1_NAME) == 0
            || strcmp(name, CONFIG_FASTBOOT_MMC_BOOT2_NAME) == 0)
        {
            memset(cmd, 0, COMD_BUFFER_LEN);
            snprintf_len = snprintf(cmd, sizeof(cmd)-1, "partition write.boot mmc 0 %d %p 0x%lx",bootpart, pTmp, (unsigned long)size);
            if (snprintf_len <0)
            {
                UBOOT_ERROR("The array size is too small(%d), snprintf fail '%s'\n", (int)(sizeof(cmd)-1), cmd);
            }
            UBOOT_DEBUG("cmd=%s\n",cmd);
            if(run_command(cmd, 0) != 0)
            {
                UBOOT_ERROR("[ERROR] cmd run error\n");
                    return MULTI_IMAGE_ERROR;
            }
        }
        else
#endif
        if (sparse_header->magic != SPARSE_HEADER_MAGIC) {
                fastboot_mmc_flash_write(name, pTmp, size,response);
                fastboot_okay("finished",response);
        } else {
            printf("[%s][%06d]Unsupport sparse image .\n",__FUNCTION__, __LINE__);
            fastboot_okay("finished",response);
        }

        pTmp += size;
    }
    return MULTI_IMAGE_WRITE_SUCCESS;
}


/**
 * flash() - write the downloaded image to the indicated partition.
 *
 * @cmd_parameter: Pointer to partition name
 * @response: Pointer to fastboot response buffer
 *
 * Writes the previously downloaded image to the partition indicated by
 * cmd_parameter. Writes to response.
 */
#define PARTITION_CMD_BUFFER_LEN 128
#define INVALID_BOOT_PARTITION_NUMBER 0xFF
static void flash(char *cmd_parameter, char *response)
{
	int snprintf_len;
	char cmd[PARTITION_CMD_BUFFER_LEN] = {0};
	u8 boot_partition_number = INVALID_BOOT_PARTITION_NUMBER;

#if defined(UFBL_FEATURE_UNLOCK)
	if (strncmp(cmd_parameter, "unlock", 6) == 0) {
		if (amzn_verify_unlock((void*)fastboot_buf_addr, image_size))
			fastboot_fail("[ERROR] unlock code error", response);
		else if (idme_update_var_ex("unlock_code", (const char *)fastboot_buf_addr, SIGNED_UNLOCK_CODE_LEN))
			fastboot_fail("[ERROR] unlock failed", response);
		else
			fastboot_okay(NULL, response);
		return;
#if defined(UFBL_FEATURE_REPLAY_PROTECTED_UNLOCK)
	} else if (strncmp(cmd_parameter, "rpucode", 7) == 0) {
		if (amzn_rpu_set_idme_unlock_code(fastboot_buf_addr, image_size)) {
			fastboot_fail("[ERROR] flash replay protected unlock code failed", response);
		} else {
			fastboot_okay(NULL, response);
		}
		amzn_target_is_replay_protected_unlocked();
		return;
	} else if (strncmp(cmd_parameter, "rpucert", 7) == 0) {
		if (amzn_rpu_set_idme_unlock_cert(fastboot_buf_addr, image_size)) {
			fastboot_fail("[ERROR] flash replay protected unlock certificate failed", response);
		} else {
			fastboot_okay(NULL, response);
		}
		amzn_target_is_replay_protected_unlocked();
		return;
#endif
	}
#endif

	if (is_lockdown()) {
		fastboot_fail("[ERROR] command not supported in lockdown", response);
		return;
	}

#if CONFIG_IS_ENABLED(FASTBOOT_FLASH_MMC)
    /* part modified by David Li starts for fastboot supporting packed image */

//    printf("cmd_parameter=%s, fastboot_buf_addr=%s", cmd_parameter, (char *)fastboot_buf_addr);
#ifdef CONFIG_FASTBOOT_MMC_BOOT_SUPPORT
	if (strncmp(cmd_parameter, "mmc0boot0", 9) == 0)
		boot_partition_number = 1;
#if defined(CONFIG_ANDROID_AB)
	else if (strncmp(cmd_parameter, "mmc0boot1", 9) == 0)
		boot_partition_number = 2;
#endif
	if (boot_partition_number != INVALID_BOOT_PARTITION_NUMBER) {
		memset(cmd, 0, COMD_BUFFER_LEN);
		snprintf_len = snprintf(cmd, sizeof(cmd), "partition write.boot mmc 0 %d %p 0x%x", \
						boot_partition_number, fastboot_buf_addr, image_size);
		if (snprintf_len >= sizeof(cmd))
		{
			printf("The array size is too small(%d), snprintf fail '%s'\n", (int)(sizeof(cmd)), cmd);
			fastboot_fail("writing partition command failed", response);
			return;
		}
		printf("cmd=%s\n",cmd);
		if(run_command(cmd, 0) != 0)
			fastboot_fail("[ERROR] cmd run error", response);
		else
			fastboot_okay(NULL, response);
		return;
	}
#endif
    if (strncmp((const char *)fastboot_buf_addr, "BOOTLDR!", strlen("BOOTLDR!")) == 0) {
         do_update_packed_images(response);
    }
    else if (strncmp((const char *)cmd_parameter, "mscript", strlen("mscript")) == 0) {
        memset(fastboot_buf_addr + image_size, 0, 1);
        int ret = runscript_linebyline((char *)fastboot_buf_addr);
        if (ret != 0)
            fastboot_fail("Failed to call mscript", response);
        else
            fastboot_okay(NULL, response);
    }
    else
    /* part modified by David Li ends */
	fastboot_mmc_flash_write(cmd_parameter, fastboot_buf_addr, image_size,
				 response);
#endif
#if CONFIG_IS_ENABLED(FASTBOOT_FLASH_NAND)
	fastboot_nand_flash_write(cmd_parameter, fastboot_buf_addr, image_size,
				  response);
#endif
}

/**
 * flashing() - Sets the device state.
 *
 * @cmd_parameter: Pointer to devic state
 * @response: Pointer to fastboot response buffer
 *
 * Set the device state with lock or unlock.
 */
static void flashing(char *cmd_parameter, char *response)
{
	char cmdbuf[CMD_BUF_SIZE] = {0};
	int snprintf_len;

	if (!cmd_parameter) {
		pr_err("missing command parameter\n");
		fastboot_fail("Expected command parameter", response);
		return;
	}

	if (run_command("avb init 0", 0))
		fastboot_fail("Failed to initialize avb", response);
	else
		fastboot_okay(NULL, response);

	if (strncmp(cmd_parameter, "lock", strlen("lock")) == 0) {
		snprintf_len = snprintf(cmdbuf, sizeof(cmdbuf), "avb set-devicestate 1");
		if (snprintf_len >= sizeof(cmdbuf)) {
			printf("The array size is too small(%d), snprintf fail '%s'\n", (int)sizeof(cmdbuf), cmdbuf);
			return ;
		}
	}
	else if (strncmp(cmd_parameter, "unlock", strlen("unlock")) == 0) {
		snprintf_len = snprintf(cmdbuf, sizeof(cmdbuf), "avb set-devicestate 0");
		if (snprintf_len >= sizeof(cmdbuf)) {
			printf("The array size is too small(%d), snprintf fail '%s'\n", (int)sizeof(cmdbuf), cmdbuf);
			return ;
		}
	}
	else {
		fastboot_fail("invalid command", response);
		return;
	}

	if (run_command(cmdbuf, 0))
		fastboot_fail("Can't write device state", response);
	else
		fastboot_okay(NULL, response);
}

/**
 * erase() - erase the indicated partition.
 *
 * @cmd_parameter: Pointer to partition name
 * @response: Pointer to fastboot response buffer
 *
 * Erases the partition indicated by cmd_parameter (clear to 0x00s). Writes
 * to response.
 */
static void erase(char *cmd_parameter, char *response)
{
#if CONFIG_IS_ENABLED(FASTBOOT_FLASH_MMC)
	fastboot_mmc_erase(cmd_parameter, response);
#endif
#if CONFIG_IS_ENABLED(FASTBOOT_FLASH_NAND)
	fastboot_nand_erase(cmd_parameter, response);
#endif
}
#endif

/**
 * reboot_bootloader() - Sets reboot bootloader flag.
 *
 * @cmd_parameter: Pointer to command parameter
 * @response: Pointer to fastboot response buffer
 */
static void reboot_bootloader(char *cmd_parameter, char *response)
{
	if (fastboot_set_reboot_flag(FASTBOOT_REBOOT_REASON_BOOTLOADER))
		fastboot_fail("Cannot set reboot flag", response);
	else
		fastboot_okay(NULL, response);
}

/**
 * reboot_fastbootd() - Sets reboot fastboot flag.
 *
 * @cmd_parameter: Pointer to command parameter
 * @response: Pointer to fastboot response buffer
 */
static void reboot_fastbootd(char *cmd_parameter, char *response)
{
	if (fastboot_set_reboot_flag(FASTBOOT_REBOOT_REASON_FASTBOOTD))
		fastboot_fail("Cannot set fastboot flag", response);
	else
		fastboot_okay(NULL, response);
}

/**
 * reboot_recovery() - Sets reboot recovery flag.
 *
 * @cmd_parameter: Pointer to command parameter
 * @response: Pointer to fastboot response buffer
 */
static void reboot_recovery(char *cmd_parameter, char *response)
{
	if (fastboot_set_reboot_flag(FASTBOOT_REBOOT_REASON_RECOVERY))
		fastboot_fail("Cannot set recovery flag", response);
	else
		fastboot_okay(NULL, response);
}

/**
 * set_active() - Sets active slot.
 *
 * @cmd_parameter: Pointer to command parameter
 * @response: Pointer to fastboot response buffer
 */
static void set_active(char *cmd_parameter, char *response)
{
	u8 slot = 0;
	char cmdbuf[32];
	int snprintf_len = 0;

#if defined(CONFIG_ANDROID_AB)
	struct misc_virtual_ab_message message = {0};
	int ret;

	/* Check snapshot merge status:
	   If the status is MERGING, the device should abort the operation.
	   The slot can safely be changed in the SNAPSHOTTED state */
	ret = get_snapshot_merge_status(&message);
	if (ret == 0) {
		if ((message.version == MISC_VIRTUAL_AB_MESSAGE_VERSION) &&
			(message.magic == MISC_VIRTUAL_AB_MAGIC_HEADER) &&
			(message.merge_status == MERGING)) {

			fastboot_fail("Snapshot in merging state, cannot set active slot", response);
			return;
		}
	}
#endif

	if (!cmd_parameter) {
		fastboot_fail("Expected command parameter", response);
		return;
	}
	if (strncmp(cmd_parameter, "a", 2) == 0)
		slot = 1;
	else if (strncmp(cmd_parameter, "b", 2) == 0)
		slot = 2;
	else {
		fastboot_fail("Cannot set active slot", response);
		return;
	}

	/* execute 'mmc partconfg' command with cmd_parameter arguments*/
	snprintf_len = snprintf(cmdbuf, sizeof(cmdbuf), "mmc partconf %x 0 %d 0",
				CONFIG_FASTBOOT_FLASH_MMC_DEV, slot);

	if (snprintf_len < 0)
		UBOOT_ERROR("The array size is too small(%d), snprintf fail '%s'\n", (int)(sizeof(cmdbuf) - 1), cmdbuf);

	printf("Execute: %s\n", cmdbuf);
	if (run_command(cmdbuf, 0))
		fastboot_fail("Cannot set oem partconf", response);
	else
		fastboot_okay(NULL, response);
}

/**
 * oem_set_serialno() - Execute the OEM set_serialno command
 *
 * @cmd_parameter: Pointer to command parameter
 * @response: Pointer to fastboot response buffer
 */
static void oem_set_serialno(char *cmd_parameter, char *response)
{
#define MAX_STRING_SERIAL   24
	char serialno_string[MAX_STRING_SERIAL] = { 0 };

	if (!cmd_parameter) {
		fastboot_fail("Expected command parameter", response);
		return;
	}
	memset(serialno_string, 0, MAX_STRING_SERIAL);
	strlcpy(serialno_string, cmd_parameter, sizeof(serialno_string));

	env_set("serialno", serialno_string);
	env_save();

	UBOOT_INFO("serialno=%s\n", serialno_string);
	fastboot_okay(NULL, response);
}

#if CONFIG_IS_ENABLED(FASTBOOT_CMD_OEM_FORMAT)
/**
 * oem_format() - Execute the OEM format command
 *
 * @cmd_parameter: Pointer to command parameter
 * @response: Pointer to fastboot response buffer
 */
static void oem_format(char *cmd_parameter, char *response)
{
	char cmdbuf[32];

	if (!env_get("partitions")) {
		fastboot_fail("partitions not set", response);
	} else {
		sprintf(cmdbuf, "gpt write mmc %x $partitions",
			CONFIG_FASTBOOT_FLASH_MMC_DEV);
		if (run_command(cmdbuf, 0))
			fastboot_fail("", response);
		else
			fastboot_okay(NULL, response);
	}
}
#endif



#ifdef UFBL_FEATURE_IDME
static void fbcmd_oem_idme(char *cmd_parameter, char *response)
{
	if (0 == fastboot_idme(response, cmd_parameter))
		fastboot_okay("idme done", response);
	else
		fastboot_fail("idme fail", response);
}

#ifdef UFBL_FEATURE_FOS_FLAGS
static void fbcmd_oem_flags(char *cmd_parameter, char *response)
{
	if (oem_flags(cmd_parameter, response, FASTBOOT_RESPONSE_LEN))
		fastboot_okay(NULL, response);
	else
		fastboot_fail("oem flags fail", response);
}
#endif
#if defined(UFBL_FEATURE_UNLOCK)
static void fbcmd_oem_relock(char *buffer, char *response)
{
	if (!amzn_device_relock())
		fastboot_okay("relock successful", response);
	else
		fastboot_fail("relock failed", response);
}
#endif
#endif
