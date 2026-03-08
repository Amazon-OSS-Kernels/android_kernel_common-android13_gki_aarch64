/*
 * Copyright (c) 2023 Amazon.com, Inc. or its affiliates.  All rights reserved.
 *
 */

#ifndef __AMZN_TV_SECURE_BOOT_H
#define __AMZN_TV_SECURE_BOOT_H

typedef enum {
	SBVC_MATCH,
	SBVC_INVALID_ARG,
	SBVC_MARK_NOT_FOUND,
	SBVC_NO_MEM,
	SBVC_READ_ERR,
	SBVC_DEV_MARK_NOT_FOUND,
	SBVC_SAME_VER,
	SBVC_ROLLBACK,
	SBVC_NON_SECURE,
} sbvc_result;

/* anti-rollback efuse version structure */
struct ar_efuse_version_type {
	uint32_t hash1_version;
	uint32_t teeloader_version;
	uint32_t armfw_version;
	uint32_t optee_version;
	uint32_t reeloader_version;
	uint32_t uboot_version;
	uint32_t pmufw_version;
	uint32_t vbmeta_version;
	uint32_t boot_version;
	uint32_t recovery_version;
};

/* anti-rollback rpmb version structure */
struct ar_rpmb_version_type {
	uint64_t hash1_version;
	uint64_t teeloader_version;
	uint64_t armfw_version;
	uint64_t optee_version;
	uint64_t reeloader_version;
	uint64_t uboot_version;
	uint64_t pmufw_version;
	uint64_t vbmeta_version;
	uint64_t boot_version;
	uint64_t recovery_version;
};

typedef enum {
	AR_NOT_INITED = 0x00,
	AR_DISABLED = 0x01,
	AR_ENABLED_ASSUMED = 0x02,
	AR_ENABLED_EFUSE = 0x04,
	AR_ENABLED_RPMB = 0x08
} anti_rollback_status_type;

sbvc_result sboot_version_check(char *sboot_buf, int sboot_len, int partition_num);

unsigned char is_secure_cpu(void);
int is_lockdown(void);
int target_is_production(void);
int amzn_device_is_unlocked(void);
int amzn_device_relock(void);
int chk_cmd_lockdown(const char* command);
anti_rollback_status_type anti_rollback_enabled(void);
#if (CONFIG_ROLLBACK_INDEX_IN_EFUSE == 1)
unsigned int amzn_antirollback_efuse_version(unsigned char *ar_vers);
#endif
#if (CONFIG_ROLLBACK_INDEX_IN_RPMB == 1)
unsigned int amzn_antirollback_rpmb_version(unsigned char *ar_vers);
#endif

#endif
