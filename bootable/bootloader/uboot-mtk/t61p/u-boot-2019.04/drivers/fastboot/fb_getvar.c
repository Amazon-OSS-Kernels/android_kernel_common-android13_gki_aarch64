// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (C) 2016 The Android Open Source Project
 */

#include <common.h>
//#include <env.h>
#include <fastboot.h>
#include <fastboot-internal.h>
#include <fb_mmc.h>
#include <fb_nand.h>
#include <fs.h>
#include <part.h>
#include <version.h>
#include <asm/io.h>
#include <android_ab.h>
#include <boot_impl.h>
#include <utility.h>
#ifdef UFBL_FEATURE_UNLOCK
#include <amzn_unlock.h>
#endif
#ifdef UFBL_FEATURE_REPLAY_PROTECTED_UNLOCK
#include <amzn_replay_protected_unlock.h>
#endif
#include <amzn_tv_common.h>
#include <amzn_tv_secure_boot.h>
#include <amzn_secure_boot.h>
#include <debug_impl.h>

static void getvar_version(char *var_parameter, char *response);
static void getvar_version_bootloader(char *var_parameter, char *response);
static void getvar_downloadsize(char *var_parameter, char *response);
static void getvar_serialno(char *var_parameter, char *response);
static void getvar_version_baseband(char *var_parameter, char *response);
static void getvar_product(char *var_parameter, char *response);
static void getvar_platform(char *var_parameter, char *response);
static void getvar_current_slot(char *var_parameter, char *response);
#if CONFIG_IS_ENABLED(FASTBOOT_FLASH)
static void getvar_has_slot(char *var_parameter, char *response);
#endif
#if CONFIG_IS_ENABLED(FASTBOOT_FLASH_MMC)
static void getvar_partition_type(char *part_name, char *response);
#endif
#if CONFIG_IS_ENABLED(FASTBOOT_FLASH)
static void getvar_partition_size(char *part_name, char *response);
#endif
static void getvar_is_userspace(char *var_parameter, char *response);
static void getvar_secure(char *var_parameter, char *response);
static void getvar_unlocked(char *var_parameter, char *response);
static void getvar_slot_count(char *var_parameter, char *response);
static void getvar_off_mode_charge(char *var_parameter, char *response);
static void getvar_variant(char *var_parameter, char *response);
static void getvar_battery_voltage(char *var_parameter, char *response);
static void getvar_battery_soc_ok(char *var_parameter, char *response);
static void getvar_all(char *var_parameter, char *response);
static void getvar_slot_suffixes(char *slot, char *response);
static void getvar_slot_successful(char *slot, char *response);
static void getvar_slot_unbootable(char *slot, char *response);
static void getvar_slot_retry_count(char *slot, char *response);
#if defined(TARGET_AMMO_SUPPORT)
static void getvar_product_variant(char *var_parameter, char *response);
#endif
#if defined(UFBL_FEATURE_UNLOCK)
static void getvar_unlock_code(char *var_parameter, char *response);
static void getvar_unlock_status(char *var_parameter, char *response);
static void getvar_unlock_version(char *var_parameter, char *response);
#endif
#ifdef UFBL_FEATURE_REPLAY_PROTECTED_UNLOCK
static void getvar_replay_protected_unlock_code(char *var_parameter, char *response);
#endif

#if defined(CONFIG_ANDROID_AB)
static void getvar_snapshot_update_status(char *var_parameter, char *response);
#endif

static void getvar_production_device(char *var_parameter, char *response);
static void getvar_antirollback_version_info(char *var_parameter, char *response);
unsigned int ReadRollbackIndexFromEfuse(unsigned char* addr);

static const struct {
	const char *variable;
	void (*dispatch)(char *var_parameter, char *response);
} getvar_dispatch[] = {
	{
		.variable = "version",
		.dispatch = getvar_version
	}, {
		.variable = "version-bootloader",
		.dispatch = getvar_version_bootloader
	}, {
		.variable = "downloadsize",
		.dispatch = getvar_downloadsize
	}, {
		.variable = "max-download-size",
		.dispatch = getvar_downloadsize
	}, {
		.variable = "serialno",
		.dispatch = getvar_serialno
	}, {
		.variable = "version-baseband",
		.dispatch = getvar_version_baseband
	}, {
		.variable = "product",
		.dispatch = getvar_product
	}, {
		.variable = "platform",
		.dispatch = getvar_platform
	}, {
		.variable = "current-slot",
		.dispatch = getvar_current_slot
#if CONFIG_IS_ENABLED(FASTBOOT_FLASH)
	}, {
		.variable = "has-slot",
		.dispatch = getvar_has_slot
#endif
#if CONFIG_IS_ENABLED(FASTBOOT_FLASH_MMC)
	}, {
		.variable = "partition-type",
		.dispatch = getvar_partition_type
#endif
#if CONFIG_IS_ENABLED(FASTBOOT_FLASH)
	}, {
		.variable = "partition-size",
		.dispatch = getvar_partition_size
#endif
	}, {
		.variable = "is-userspace",
		.dispatch = getvar_is_userspace
	}, {
		.variable = "secure",
		.dispatch = getvar_secure
	}, {
		.variable = "unlocked",
		.dispatch = getvar_unlocked
	}, {
		.variable = "off-mode-charge",
		.dispatch = getvar_off_mode_charge
	}, {
		.variable = "variant",
		.dispatch = getvar_variant
	}, {
		.variable = "battery-voltage",
		.dispatch = getvar_battery_voltage
	}, {
		.variable = "battery-soc-ok",
		.dispatch = getvar_battery_soc_ok
	}, {
		.variable = "all",
		.dispatch = getvar_all
    }, {
        .variable = "slot-count",
        .dispatch = getvar_slot_count
	}, {
		.variable = "slot-suffixes",
		.dispatch = getvar_slot_suffixes
	}, {
		.variable = "slot-successful",
		.dispatch = getvar_slot_successful
	}, {
		.variable = "slot-unbootable",
		.dispatch = getvar_slot_unbootable
	}, {
		.variable = "slot-retry-count",
		.dispatch = getvar_slot_retry_count
#if defined(TARGET_AMMO_SUPPORT)
	}, {
		.variable = "product-variant",
		.dispatch = getvar_product_variant
#endif
#if defined(UFBL_FEATURE_UNLOCK)
	}, {
		.variable = "unlock_code",
		.dispatch = getvar_unlock_code
	}, {
		.variable = "unlock_status",
		.dispatch = getvar_unlock_status
	}, {
		.variable = "unlock_version",
		.dispatch = getvar_unlock_version
#endif
#if defined(CONFIG_ANDROID_AB)
	}, {
		.variable = "snapshot-update-status",
		.dispatch = getvar_snapshot_update_status
#endif
#ifdef UFBL_FEATURE_REPLAY_PROTECTED_UNLOCK
	}, {
		.variable = "rpu_code",
		.dispatch = getvar_replay_protected_unlock_code
#endif
	}, {
		.variable = "antirollback_version_info",
		.dispatch = getvar_antirollback_version_info
	}, {
		.variable = "production_device",
		.dispatch = getvar_production_device
	}
};

#if CONFIG_IS_ENABLED(FASTBOOT_FLASH)
/**
 * Get partition number and size for any storage type.
 *
 * Can be used to check if partition with specified name exists.
 *
 * If error occurs, this function guarantees to fill @p response with fail
 * string. @p response can be rewritten in caller, if needed.
 *
 * @param[in] part_name Info for which partition name to look for
 * @param[in,out] response Pointer to fastboot response buffer
 * @param[out] size If not NULL, will contain partition size
 * @return Partition number or negative value on error
 */
static int getvar_get_part_info(const char *part_name, char *response,
				size_t *size)
{
	int r;
# if CONFIG_IS_ENABLED(FASTBOOT_FLASH_MMC)
	struct blk_desc *dev_desc;
	struct disk_partition part_info;

	r = fastboot_mmc_get_part_info(part_name, &dev_desc, &part_info,
				       response);
	if (r >= 0 && size)
		*size = part_info.size * part_info.blksz;
# elif CONFIG_IS_ENABLED(FASTBOOT_FLASH_NAND)
	struct part_info *part_info;

	r = fastboot_nand_get_part_info(part_name, &part_info, response);
	if (r >= 0 && size)
		*size = part_info->size;
# else
	fastboot_fail("this storage is not supported in bootloader", response);
	r = -ENODEV;
# endif

	return r;
}
#endif

static int getvar_get_slot_successful(const char *slot, char *response)
{
	int a_slot_status, b_slot_status;
	a_slot_status = readb(ROMCODE_AB_RETRY_COUNT) & (1 << ROMCODE_AB_SLOT_A_SHIFT);
	b_slot_status = readb(ROMCODE_AB_RETRY_COUNT) & (1 << ROMCODE_AB_SLOT_B_SHIFT);
	if((slot[0] == 'A' || slot[0] == 'a')){
		if(a_slot_status == 0)
			return 0;
		else
			return -1;
	} else {
		if(b_slot_status == 0)
			return 0;
		else
			return -1;
	}
}

static int getvar_get_slot_retry_count(const char *slot, char *response, int *size)
{
	int remain, current_slot;
	remain = AB_RETRY_COUNT_MAXIMUM - (readb(ROMCODE_AB_RETRY_COUNT) & 0x3);
	current_slot = (readb(ROMCODE_AB_SLOT_NUM) & 0x1);
	if (current_slot == 0){
		if((slot[0] == 'A' || slot[0] == 'a'))
			*size = remain;
		else
			*size = AB_RETRY_COUNT_MAXIMUM;
	} else {
		if((slot[0] == 'A' || slot[0] == 'a'))
			*size = 0;
		else
			*size = remain;
	}
	return 0;
}

static void getvar_version(char *var_parameter, char *response)
{
	fastboot_okay(FASTBOOT_VERSION, response);
}

static void getvar_version_bootloader(char *var_parameter, char *response)
{
	fastboot_okay(BOOTLOADER_VERSION, response);
}

static void getvar_downloadsize(char *var_parameter, char *response)
{
	fastboot_response("OKAY", response, "0x%08x", fastboot_buf_size);
}

static void getvar_serialno(char *var_parameter, char *response)
{
	const char *tmp = env_get("serial#");

	if (tmp) {
		fastboot_okay(tmp, response);
		return;
	}

	tmp = env_get("serialno");
	if (tmp)
		fastboot_okay(tmp, response);
	else
		fastboot_fail("Value not set", response);
}

static void getvar_version_baseband(char *var_parameter, char *response)
{
	fastboot_okay("N/A", response);
}

static void getvar_product(char *var_parameter, char *response)
{
	const char *board = env_get("board");

	if (board)
		fastboot_okay(board, response);
	else
		fastboot_fail("Board not set", response);
}

static void getvar_platform(char *var_parameter, char *response)
{
	const char *p = env_get("platform");

	if (p)
		fastboot_okay(p, response);
	else
		fastboot_fail("platform not set", response);
}

static void getvar_current_slot(char *var_parameter, char *response)
{
	int current_slot;
	current_slot = (readb(ROMCODE_AB_SLOT_NUM) & 0x1);
	if (current_slot == 0){
		fastboot_okay("a", response);
	} else if (current_slot == 1) {
		fastboot_okay("b", response);
	} else {
		fastboot_fail("unknown", response);
	}
}

#if CONFIG_IS_ENABLED(FASTBOOT_FLASH)
#define PART_SUFFIX_LEN 2
static void getvar_has_slot(char *part_name, char *response)
{
	char part_name_wslot[PART_NAME_LEN];
	size_t len;
	int r;

	if (!part_name || part_name[0] == '\0')
		goto fail;
    /*  part modified by David Li starts for fastboot supporting packed image  */
    if(strncmp(part_name, "bootloader", strlen("bootloader")) == 0)
    {
        printf("bootloader is packed image\n");
        fastboot_okay("no", response); /* part not exists*/
		return;
    }
	/* part_name_wslot = part_name + "_a" */
	len = strlcpy(part_name_wslot, part_name, PART_NAME_LEN - 3);
	if (len > PART_NAME_LEN - 3)
		goto fail;
	strncat(part_name_wslot, "_a", PART_SUFFIX_LEN);

	r = getvar_get_part_info(part_name_wslot, response, NULL);
	if (r >= 0) {
		fastboot_okay("yes", response); /* part exists and slotted */
		return;
	}

	r = getvar_get_part_info(part_name, response, NULL);
	if (r >= 0)
		fastboot_okay("no", response); /* part exists but not slotted */

	/* At this point response is filled with okay or fail string */
	return;

fail:
	fastboot_fail("invalid partition name", response);
}
#endif

#if CONFIG_IS_ENABLED(FASTBOOT_FLASH_MMC)
static void getvar_partition_type(char *part_name, char *response)
{
	int r;
	struct blk_desc *dev_desc;
	struct disk_partition part_info;

	r = fastboot_mmc_get_part_info(part_name, &dev_desc, &part_info,
				       response);
	if (r >= 0) {
		r = fs_set_blk_dev_with_part(dev_desc, r);
		if (r < 0)
			fastboot_fail("failed to set partition", response);
		else
			fastboot_okay(fs_get_type_name(), response);
	}
}
#endif

#if CONFIG_IS_ENABLED(FASTBOOT_FLASH)
static void getvar_partition_size(char *part_name, char *response)
{
	int r;
	size_t size;

	r = getvar_get_part_info(part_name, response, &size);
	if (r >= 0)
		fastboot_response("OKAY", response, "0x%016zx", size);
}
#endif

static void getvar_is_userspace(char *var_parameter, char *response)
{
    fastboot_okay("no", response);
}

/* This getvar_secure is regarding AVB device state */
static void getvar_secure(char *var_parameter, char *response)
{
    const char *p = env_get("devicestate");
    if (!strcmp(p,"lock"))
        fastboot_okay("yes", response);
    else
        fastboot_okay("no", response);
}

/* This getvar_unlocked is regarding AVB device state */
static void getvar_unlocked(char *var_parameter, char *response)
{
    const char *p = env_get("devicestate");
    if (!strcmp(p,"lock"))
        fastboot_okay("no", response);
    else
        fastboot_okay("yes", response);
}

static void getvar_off_mode_charge(char *var_parameter, char *response)
{
    fastboot_okay("0", response);
}

static void getvar_variant(char *var_parameter, char *response)
{
    fastboot_okay("", response);
}

static void getvar_battery_voltage(char *var_parameter, char *response)
{
    fastboot_okay("2700mV", response);
}

static void getvar_battery_soc_ok(char *var_parameter, char *response)
{
    fastboot_okay("yes", response);
}
extern int fastboot_tx_infor(const char *buffer, unsigned int buffer_size);
static void getvar_all(char *var_parameter, char *response)
{
#define FASTBOOT_NAME_LEN 64

    int ret;
    int p;
    struct blk_desc *blk_dev_desc = NULL;
    disk_partition_t info;
    char PartitionName[FASTBOOT_NAME_LEN] = {0};
    char PartitionSize[FASTBOOT_NAME_LEN] = {0};
    char DownloadSize[FASTBOOT_NAME_LEN] = {0};
    char name[PART_NAME_LEN];
    char Psize[FASTBOOT_NAME_LEN] = {0};
    // parameter of blk_get_dev should input mmc device number
    blk_dev_desc = blk_get_dev("mmc", 0);
    if (!blk_dev_desc) {
        return ;
    }

    for (p = 1; p <= MAX_SEARCH_PARTITIONS; p++)
    {
        ret = part_get_info(blk_dev_desc, p, &info);
        if (ret)
            continue;
        memset(name, 0, PART_NAME_LEN);
        if(strlen((char *) info.name)<PART_NAME_LEN)
        {
            memcpy(name,info.name ,strlen((char *) info.name));
            name[PART_NAME_LEN-1]='\0';
            if(snprintf(Psize,FASTBOOT_NAME_LEN -1, "0x%08lx",info.size * info.blksz)<0)
                printf(" %d snprint error !!! \n",__LINE__);
            if(snprintf(PartitionName,FASTBOOT_NAME_LEN-1,"INFOpartition-type:%s:ext4",name)<0)
                printf(" %d snprint error !!! \n",__LINE__);
            if(snprintf(PartitionSize,FASTBOOT_NAME_LEN-1, "INFOpartition-size:%s:%s",name,Psize)<0)
                printf(" %d snprint error !!! \n",__LINE__);
            fastboot_tx_infor(PartitionName,strlen(PartitionName));
            fastboot_tx_infor(PartitionSize,strlen(PartitionSize));
        }
        //printf("PartitionName %s \n",PartitionName);
        //printf("PartitionSize %s \n",PartitionSize);
        //printf("Psize %s \n",Psize);
    }
    fastboot_tx_infor("INFObattery-soc-ok:yes", strlen("INFObattery-soc-ok:yes"));
    fastboot_tx_infor("INFObattery-voltage:", strlen("INFObattery-voltage:"));
    fastboot_tx_infor("INFOvariant:", strlen("INFOvariant:"));
    fastboot_tx_infor("INFOoff-mode-charge:", strlen("INFOoff-mode-charge:0"));
    fastboot_tx_infor("INFOslot-count:2", strlen("INFOslot-count:2"));
    fastboot_tx_infor("INFOis-userspace:no", strlen("INFOis-userspace:no"));

	if (snprintf(DownloadSize, (FASTBOOT_NAME_LEN - 1), "INFOmax-download-size:0x%08x", fastboot_buf_size) < 0)
		printf("%d snprint error !!! \n", __LINE__);

    fastboot_tx_infor(DownloadSize, strlen(DownloadSize));

    fastboot_okay("yes", response);
}

static void getvar_slot_count(char *var_parameter, char *response)
{
    /* A/B not implemented, for now always return "2" */
    fastboot_okay("2", response);
}

static void getvar_slot_suffixes(char *var_parameter, char *response)
{
#define BOARD_SUPPORT_SUFFIX_NAME	"_a"
	fastboot_response("OKAY", response, "%s", BOARD_SUPPORT_SUFFIX_NAME);
}

static void getvar_slot_successful(char *slot, char *response)
{
	int r;
	if(slot == NULL){
		fastboot_fail("failed to pass slot argument", response);
		return;
	}

	r = getvar_get_slot_successful(slot, response);
	if (r == 0)
		fastboot_okay("yes", response);
	else
		fastboot_okay("no",response);
}

static void getvar_slot_unbootable(char *slot, char *response)
{
	int r;

	if(slot == NULL){
		fastboot_fail("failed to pass slot argument", response);
		return;
        }

	r = getvar_get_slot_successful(slot, response);
	if (r < 0)
		fastboot_okay("yes", response);
	else
		fastboot_okay("no",response);
}

static void getvar_slot_retry_count(char *slot, char *response)
{
	int r;
	int remain;

	if(slot == NULL){
		fastboot_fail("failed to pass slot argument", response);
		return;
	}

	r = getvar_get_slot_retry_count(slot, response, &remain);
	if (r == 0)
		fastboot_response("OKAY", response, "%d", remain);
}

#ifdef TARGET_AMMO_SUPPORT
static void getvar_product_variant(char *var_parameter, char *response)
{
#ifdef UFBL_FEATURE_IDME
	#define PROD_VAR_SIZE 32
	unsigned product_variant_is_valid = 1;
	char ammo_var[PROD_VAR_SIZE+1] = {0,};

	idme_get_oem_data_field("ammo_var=", ammo_var, PROD_VAR_SIZE);
	/* following if condition is only for abc123, abc123, abc123 */
	/* and abc123 as AMMO is enabled in the middle of development    */
	if (!(strcmp(ammo_var, ""))) {
		const char *board_name = env_get("board");

		if (!(strcmp(board_name, "abc123")))
			sprintf(ammo_var, "abc123-wp");
		else if (!(strcmp(board_name, "abc123")))
			sprintf(ammo_var, "abc123-gp");
		else if (!(strcmp(board_name, "abc123")))
			sprintf(ammo_var, "abc123-lp");
		else if (!(strcmp(board_name, "abc123")))
			sprintf(ammo_var, "abc123-ca");
		else if (!(strcmp(board_name, "abc123eu")))
                        sprintf(ammo_var, "abc123eu-pv");
		else if (!(strcmp(board_name, "groveeu")))
			sprintf(ammo_var, "groveeu-gm");
		else if (!(strcmp(board_name, "grove")))
			sprintf(ammo_var, "grove-gm");
		else {
			product_variant_is_valid = 0;
			debug("Invalid board name(%s) to produce product-variant\n", board_name);
		}
	}

	if (product_variant_is_valid)
		fastboot_okay(ammo_var, response);
	else
		fastboot_fail("Value not set", response);
#else
	fastboot_fail("Value not set", response);
#endif
}
#endif

#if defined(UFBL_FEATURE_UNLOCK)
static void getvar_unlock_code(char *var_parameter, char *response)
{
	unsigned char unlock_code[UNLOCK_CODE_LEN+1] = {0};
	unsigned int code_len = UNLOCK_CODE_LEN;

	if (amzn_get_unlock_code(unlock_code, &code_len)) {
		fastboot_fail("cannot get unlock code", response);
	} else {
		unlock_code[UNLOCK_CODE_LEN] = 0x00;
		fastboot_okay((const char *)unlock_code, response);
	}
}
static void getvar_unlock_status(char *var_parameter, char *response)
{
	unsigned char unlock_status[2];

	sprintf((char *)unlock_status, "%d", amzn_device_is_unlocked() ? 1 : 0);
	fastboot_okay((const char *)unlock_status, response);
}
static void getvar_unlock_version(char *var_parameter, char *response)
{
	unsigned char unlock_version[2];
	sprintf((char *)unlock_version, "%d", 1);
	fastboot_okay((const char *)unlock_version, response);
}
#endif

#if defined(CONFIG_ANDROID_AB)
static void getvar_snapshot_update_status(char *var_parameter, char *response)
{
	struct misc_virtual_ab_message message = {0};

	int ret = get_snapshot_merge_status(&message);
	if (ret != 0) {
		fastboot_fail("Get snapshot merge status failed", response);
	} else {
		/* got vab message content */
		if ((message.version == 0) && (message.magic ==  0)) {
			/* vab message is empty*/
			fastboot_okay("NONE", response);
		} else if ((message.version == MISC_VIRTUAL_AB_MESSAGE_VERSION) &&
					(message.magic == MISC_VIRTUAL_AB_MAGIC_HEADER)) {
			/* parse valid vab message */
			switch (message.merge_status) {
				case SNAPSHOTTED:
					fastboot_okay("Snapshotted", response);
					break;
				case MERGING:
					fastboot_okay("Merging", response);
					break;
				default:
					fastboot_okay("NONE", response);
					break;
			}
		} else {
			fastboot_fail("Invalid snapshot merge status", response);
		}
	}
}
#endif

#ifdef UFBL_FEATURE_REPLAY_PROTECTED_UNLOCK
static void getvar_replay_protected_unlock_code(char *var_parameter, char *response)
{
	uint8_t rpu_code[BASE64_LEN(AMZN_RPU_CODE_LEN) + 1] = {0};
	size_t code_len = sizeof(rpu_code);

	if (amzn_rpu_get_unlock_code(rpu_code, &code_len)) {
		fastboot_fail("cannot get replay protected unlock code", response);
	} else {
		fastboot_okay((const char *)rpu_code, response);
	}
}
#endif

#include <efuse_rbi.h>

static void getvar_antirollback_version_info(char *var_parameter, char *response)
{
	#define AR_VER_STRING_SIZE 80
	char ar_version_str[AR_VER_STRING_SIZE] = {0,};
#if (CONFIG_ROLLBACK_INDEX_IN_EFUSE == 1)
	struct ar_efuse_version_type ar_efuse_vers;
#endif
#if (CONFIG_ROLLBACK_INDEX_IN_RPMB == 1)
	struct ar_rpmb_version_type ar_rpmb_vers;
#endif
	unsigned ar_status = (unsigned)anti_rollback_enabled();

	switch (ar_status) {
#if (CONFIG_ROLLBACK_INDEX_IN_EFUSE == 1)
	case AR_ENABLED_EFUSE:
		if (amzn_antirollback_efuse_version((unsigned char *)&ar_efuse_vers)) {
			snprintf(ar_version_str, sizeof(ar_version_str), "%x:%x:%x:%x:%x:%x:%x:%x:%x:%x",
			ar_efuse_vers.hash1_version,
			ar_efuse_vers.teeloader_version,
			ar_efuse_vers.armfw_version,
			ar_efuse_vers.optee_version,
			ar_efuse_vers.reeloader_version,
			ar_efuse_vers.uboot_version,
			ar_efuse_vers.pmufw_version,
			ar_efuse_vers.vbmeta_version,
			ar_efuse_vers.boot_version,
			ar_efuse_vers.recovery_version);
			fastboot_okay(ar_version_str, response);
			UBOOT_INFO("[%s] efuse ar_version_str:%s\n", __func__, ar_version_str);
		} else
			fastboot_fail("anti-rollback version is not available", response);
		break;
#endif /* CONFIG_ROLLBACK_INDEX_IN_EFUSE */
#if (CONFIG_ROLLBACK_INDEX_IN_RPMB == 1)
	case AR_ENABLED_RPMB:
	case (AR_ENABLED_EFUSE|AR_ENABLED_RPMB):
		if (amzn_antirollback_rpmb_version((unsigned char *)&ar_rpmb_vers)) {
			snprintf(ar_version_str, sizeof(ar_version_str), "%llx:%llx:%llx:%llx:%llx:%llx:%llx:%llx:%llx:%llx",
			ar_rpmb_vers.hash1_version,
			ar_rpmb_vers.teeloader_version,
			ar_rpmb_vers.armfw_version,
			ar_rpmb_vers.optee_version,
			ar_rpmb_vers.reeloader_version,
			ar_rpmb_vers.uboot_version,
			ar_rpmb_vers.pmufw_version,
			ar_rpmb_vers.vbmeta_version,
			ar_rpmb_vers.boot_version,
			ar_rpmb_vers.recovery_version);
			fastboot_okay(ar_version_str, response);
			UBOOT_INFO("[%s] RPMB ar_version_str:%s\n", __func__, ar_version_str);
		} else
			fastboot_fail("anti-rollback version is not available", response);
		break;
#endif /* CONFIG_ROLLBACK_INDEX_IN_RPMB */
	case AR_DISABLED:
		fastboot_okay("anti-rollback is not enabled", response);
		break;

	case AR_NOT_INITED:
	default:
		fastboot_fail("anti-rollback version is not available", response);
		break;
	}
	return;
}

static void getvar_production_device(char *var_parameter, char *response)
{
	unsigned char production_device[5];

	snprintf((char *)production_device, sizeof(production_device), "%d", \
		amzn_target_device_type() == AMZN_PRODUCTION_DEVICE	 ? 1 : 0);
	fastboot_okay((const char *)production_device, response);
}
/**
 * fastboot_getvar() - Writes variable indicated by cmd_parameter to response.
 *
 * @cmd_parameter: Pointer to command parameter
 * @response: Pointer to fastboot response buffer
 *
 * Look up cmd_parameter first as an environment variable of the form
 * fastboot.<cmd_parameter>, if that exists return use its value to set
 * response.
 *
 * Otherwise lookup the name of variable and execute the appropriate
 * function to return the requested value.
 */

void fastboot_getvar(char *cmd_parameter, char *response)
{
	if (!cmd_parameter) {
		fastboot_fail("missing var", response);
	} else {
#define FASTBOOT_ENV_PREFIX	"fastboot."
		int i;
		char *var_parameter = cmd_parameter;
		char envstr[FASTBOOT_RESPONSE_LEN];
		const char *s;
		printf("getvar cmd_parameter=%s \n",cmd_parameter);
		snprintf(envstr, sizeof(envstr) - 1,
			 FASTBOOT_ENV_PREFIX "%s", cmd_parameter);
		s = env_get(envstr);
		if (s) {
			fastboot_response("OKAY", response, "%s", s);
			return;
		}
		strsep(&var_parameter, ":");
		for (i = 0; i < ARRAY_SIZE(getvar_dispatch); ++i) {
			if (!strcmp(getvar_dispatch[i].variable,
				    cmd_parameter)) {
				getvar_dispatch[i].dispatch(var_parameter,
							    response);
				return;
			}
		}
		pr_warn("WARNING: unknown variable: %s\n", cmd_parameter);
		fastboot_fail("Variable not implemented", response);
	}
}
