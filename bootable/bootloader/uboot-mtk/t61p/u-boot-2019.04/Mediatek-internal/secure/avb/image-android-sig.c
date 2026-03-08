/* SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause) */
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

#include <common.h>
#include <malloc.h>
#include <utility.h>
#include <linux/arm-smccc.h>
#include <../lib/libavb/libavb.h>
#include <image.h>
#include <android_image.h>
#include <debug_impl.h>
#include <../include/tee.h>

#define DEVICE_UNLOCK   0
#define DEVICE_LOCK     1
#define BOOT_GREEN      0
#define BOOT_YELLOW     1
#define BOOT_ORANGE     2
#define BOOT_RED        3

#define RSA2048_BYTES   (2048 / 8)
#define RSA4096_BYTES   (4096 / 8)

/* Block size in bytes of a SHA-256 digest. */
#define SHA256_BYTES 64

/* Block size in bytes of a SHA-512 digest. */
#define SHA512_BYTES 128

#define KEYMASTER_OPTEE_ACCESS_ADDR 0xb200585E

typedef enum {
    KM_VERIFIED_BOOT_VERIFIED = 0,    /* Full chain of trust extending from the bootloader to
                                       * verified partitions, including the bootloader, boot
                                       * partition, and all verified partitions*/
    KM_VERIFIED_BOOT_SELF_SIGNED = 1, /* The boot partition has been verified using the embedded
                                       * certificate, and the signature is valid. The bootloader
                                       * displays a warning and the fingerprint of the public
                                       * key before allowing the boot process to continue.*/
    KM_VERIFIED_BOOT_UNVERIFIED = 2,  /* The device may be freely modified. Device integrity is left
                                       * to the user to verify out-of-band. The bootloader
                                       * displays a warning to the user before allowing the boot
                                       * process to continue */
    KM_VERIFIED_BOOT_FAILED = 3,      /* The device failed verification. The bootloader displays a
                                       * warning and stops the boot process, so no keymaster
                                       * implementation should ever actually return this value,
                                       * since it should not run.  Included here only for
                                       * completeness. */
} keymaster_verified_boot_t;


// only for tee_binding to reference used.
typedef struct {
    uint32_t os_version;
    uint32_t os_patchlevel;
    uint32_t verified_boot_key_len;
    char* verified_boot_key;
    uint32_t verified_boot_state;
    uint32_t device_locked; // true=1, false=0
    uint32_t verified_boot_hash_len;
    char *verified_boot_hash;
} Keymaster_verified_boot_parameters;

typedef struct {
    uint32_t buffer_size;
    uint8_t id[8]; //(i.e. mboot\0, null terminated)
    uint8_t name[28]; //(i.e. verified-boot\0, null terminated)
    uint8_t data[1];
} TEE_data;

/* ============================================================================
    Internal Functions
============================================================================ */
#if defined(CONFIG_HAPS)
static void optee_smccc_smc(unsigned long a0, unsigned long a1,
                unsigned long a2, unsigned long a3,
                unsigned long a4, unsigned long a5,
                unsigned long a6, unsigned long a7,
                struct arm_smccc_res *res)
{
    arm_smccc_smc(a0, a1, a2, a3, a4, a5, a6, a7, res);
}
#endif

/**
 * tee_binding()
 */
static int tee_binding(
    uint32_t os_version,
    uint32_t os_patchlevel,
    uint32_t verified_boot_key_len,
    uint8_t* verified_boot_key,
    uint32_t verified_boot_state,
    uint32_t device_locked,
    uint32_t verified_boot_hash_len,
    char* verified_boot_hash,
    struct AvbOps *avb_ops)
{
    uint32_t keymaster_params_size = 0;
    keymaster_params_size = sizeof(uint32_t)*6 + verified_boot_key_len + verified_boot_hash_len;
    uint8_t *keymaster_params_raw_data = malloc(keymaster_params_size);
    if(keymaster_params_raw_data == NULL)
    {
        UBOOT_ERROR("malloc for keymaster_params_raw_data fail!!\n");
        return -1;
    }
    memset(keymaster_params_raw_data, 0, keymaster_params_size);
    uint32_t raw_offset = 0;
    struct arm_smccc_res res;
    memset(&res, 0, sizeof(res));

    UBOOT_DEBUG("os_version=%d\n", os_version);
    memcpy(keymaster_params_raw_data, &os_version, sizeof(uint32_t));
    raw_offset += sizeof(uint32_t);

    UBOOT_DEBUG("os_patchlevel=%d\n", os_patchlevel);
    memcpy(keymaster_params_raw_data + raw_offset, &os_patchlevel, sizeof(uint32_t));
    raw_offset += sizeof(uint32_t);

    UBOOT_DEBUG("verified_boot_key_len=%d\n", verified_boot_key_len);
    memcpy(keymaster_params_raw_data + raw_offset, &verified_boot_key_len, sizeof(uint32_t));
    raw_offset += sizeof(uint32_t);

    // add verified_boot_key
    memcpy(keymaster_params_raw_data + raw_offset, verified_boot_key, verified_boot_key_len);
    raw_offset += verified_boot_key_len;

    uint32_t temp_verified_boot_state = verified_boot_state;
    UBOOT_DEBUG("state=%d\n", temp_verified_boot_state);
    memcpy(keymaster_params_raw_data + raw_offset, &temp_verified_boot_state, sizeof(uint32_t));
    raw_offset += sizeof(uint32_t);

    UBOOT_DEBUG("device_locked=%d\n", device_locked);
    memcpy(keymaster_params_raw_data + raw_offset, &device_locked, sizeof(uint32_t));
    raw_offset += sizeof(uint32_t);

    UBOOT_DEBUG("verified_boot_hash_len=%d\n\n", verified_boot_hash_len);
    memcpy(keymaster_params_raw_data + raw_offset, &verified_boot_hash_len, sizeof(uint32_t));
    raw_offset += sizeof(uint32_t);

    // add verified_boot_hash
    memcpy(keymaster_params_raw_data + raw_offset, verified_boot_hash, verified_boot_hash_len);

#if defined(CONFIG_HAPS)
    TEE_data *TEEdata = NULL;
    uint32_t tee_data_size = sizeof(*TEEdata) + keymaster_params_size;
    TEEdata = malloc(tee_data_size);
    if(TEEdata == NULL)
    {
        UBOOT_ERROR("malloc for TEEdata fail!!\n");
        free(keymaster_params_raw_data);
        return -1;
    }
    memset(TEEdata, 0, tee_data_size);
    TEEdata->buffer_size = 4 + 36 + keymaster_params_size;
    strncpy((char*)TEEdata->id, "mboot\0", 6);
    strncpy((char*)TEEdata->name, "verified-boot\0", 14);
    memcpy(TEEdata->data, keymaster_params_raw_data, keymaster_params_size);

    flush_cache((unsigned long)TEEdata, tee_data_size);
    optee_smccc_smc(KEYMASTER_OPTEE_ACCESS_ADDR, (unsigned long)TEEdata, 0, 0, 0, 0, 0, 0, &res);
    free(TEEdata);
    rc = TEE_SUCCESS;
#else

    //Start of storing keymaster_params_raw_data to RPMB via AVBTA
    uint8_t *keymaster_params_raw_data_ta_outbuff = NULL;
    int rc = TEE_ERROR_GENERIC;
    if (!avb_ops)
    {
        UBOOT_ERROR("Failed to initialize avb_ops\n");
        goto out;
    }
    keymaster_params_raw_data_ta_outbuff = malloc(keymaster_params_size);
    if(!keymaster_params_raw_data_ta_outbuff)
    {
        UBOOT_ERROR("Malloc fails\n");
        goto out;
    }
    memset(keymaster_params_raw_data_ta_outbuff, 0, keymaster_params_size);
    rc = avb_ops->read_vbmeta_data(avb_ops, keymaster_params_raw_data_ta_outbuff, keymaster_params_size);
    if (rc != AVB_IO_RESULT_OK && rc != TEE_ERROR_ITEM_NOT_FOUND)
    {
        UBOOT_ERROR("rc=0x%x\n", rc);
        goto out;
    }
    UBOOT_DUMP(keymaster_params_raw_data_ta_outbuff, keymaster_params_size);
    if( 0 != memcmp(keymaster_params_raw_data_ta_outbuff, keymaster_params_raw_data, keymaster_params_size))
    {
        UBOOT_INFO("Updating kmdata to storage\n");
        rc = avb_ops->write_vbmeta_data(avb_ops, keymaster_params_raw_data, keymaster_params_size);
        if (rc != AVB_IO_RESULT_OK)
        {
            UBOOT_ERROR("rc=0x%x\n", rc);
            goto out;
        }
    }
    rc = TEE_SUCCESS;
    //End of storing keymaster_params_raw_data to RPMB via AVBTA
out:
    if(keymaster_params_raw_data_ta_outbuff)
    {
        free(keymaster_params_raw_data_ta_outbuff);
    }
#endif
    free(keymaster_params_raw_data);
    return rc;
}

extern int sscanf(const char * buf, const char * fmt, ...);
static int get_data_from_bootargs(const char* bootargs, const char *prefix, int prefix_len, char* out_buf, int out_buf_len)
{
    char temp_prefix[128] = {0};
    char string_value[128] = {0};
    char* ptr = NULL;
    ptr = strstr(bootargs, prefix);
    if( ptr == NULL )
    {
        UBOOT_ERROR("Can't find prefix:%s\n", prefix);
        return -1;
    }
    strncpy(temp_prefix, prefix, prefix_len);
    strncat(temp_prefix, "%s", (128-prefix_len-1));
    if(sscanf(ptr, temp_prefix, string_value)<0)
        return -1;
    strncpy(out_buf, string_value, out_buf_len);
    return 0;
}

static int get_vbmeta_digest(AvbSlotVerifyData *avb_data, char* vbmeta_hash_value, uint32_t* verified_boot_hash_len)
{
    char vbmeta_hash_alg[16] = {0};
    int ret = 0;

    ret = get_data_from_bootargs(avb_data->cmdline, "androidboot.vbmeta.hash_alg=", strlen("androidboot.vbmeta.hash_alg="), vbmeta_hash_alg, sizeof(vbmeta_hash_alg));
    if(ret != 0)
    {
        UBOOT_ERROR("Get SHA algorithm fail\n");
        return -1;
    }
    else
    {
        if(strncmp(vbmeta_hash_alg, "sha256", 6)==0)
            *verified_boot_hash_len = SHA256_BYTES;
        else if(strncmp(vbmeta_hash_alg, "sha512", 6)==0)
            *verified_boot_hash_len = SHA512_BYTES;
        else
        {
            UBOOT_INFO("%s:%d Unsupport SHA algorithm\n", __FUNCTION__, __LINE__);
            return -1;
        }
        ret = get_data_from_bootargs(avb_data->cmdline, "androidboot.vbmeta.digest=", strlen("androidboot.vbmeta.digest="), vbmeta_hash_value, *verified_boot_hash_len);
        if(ret != 0)
        {
            UBOOT_ERROR("Get digest fail\n");
            return -1;
        }
    }
    return 0;
}

static void AVBPanelLogo(int index)
{
#if defined(CONFIG_MULTICORES_PLATFORM)
    UBOOT_DEBUG("Displaying AVB Panel Logo is disabled.\n");
#else
    int ret = -1;
    switch (index)
    {
        case BOOT_GREEN:
            ret = 0;
            break;
        case BOOT_YELLOW:
            ret = 0;
            break;
        case BOOT_ORANGE:
            ret = run_command("showlogo orange", 0);
            break;
        case BOOT_RED:
            ret = run_command("showlogo red", 0);
            break;
        default:
            UBOOT_ERROR("Unknown error occurred\n");
    }

    if (ret != 0)
        UBOOT_ERROR("show Bootlogo error\n");
#endif
}

int check_android_boot_state(bool is_device_state_unlocked, AvbSlotVerifyData *avb_data, AvbSlotVerifyResult avb_slot_verify_result, unsigned int os_version, struct AvbOps *avb_ops)
{
    uint32_t verified_boot_hash_len = SHA256_BYTES;
    char vbmeta_hash_value[SHA512_BYTES] = {0};
    int ret = 0;

    if( is_device_state_unlocked == true
        && (avb_slot_verify_result == AVB_SLOT_VERIFY_RESULT_OK
            || avb_slot_verify_result == AVB_SLOT_VERIFY_RESULT_ERROR_VERIFICATION
            || avb_slot_verify_result == AVB_SLOT_VERIFY_RESULT_ERROR_ROLLBACK_INDEX
            || avb_slot_verify_result == AVB_SLOT_VERIFY_RESULT_ERROR_PUBLIC_KEY_REJECTED)) //orange state.
    {
        AVBPanelLogo(BOOT_ORANGE);
        ret = get_vbmeta_digest(avb_data, vbmeta_hash_value, &verified_boot_hash_len);
        if(ret == 0)
        {
            if(tee_binding(os_version, 0, 0, NULL, BOOT_ORANGE,
                        DEVICE_UNLOCK, verified_boot_hash_len, vbmeta_hash_value, avb_ops) != TEE_SUCCESS)
            {
                goto fail_handler;
            }
            printf("Boot state is: Orange !\n");
            return 0;
        }
        else
        {
            UBOOT_ERROR("Get vbmeta digest fail on Boot state is: Orange\n");
            goto fail_handler;
        }
    }
    else if(is_device_state_unlocked == false
        && avb_slot_verify_result == AVB_SLOT_VERIFY_RESULT_OK) //green state.
    {
        AVBPanelLogo(BOOT_GREEN);
        ret = get_vbmeta_digest(avb_data, vbmeta_hash_value, &verified_boot_hash_len);
        if(ret == 0)
        {
            if(tee_binding(os_version, 0, avb_data->vbmeta_images->public_key_len, avb_data->vbmeta_images->public_key_data, BOOT_GREEN,
                        DEVICE_LOCK, verified_boot_hash_len, vbmeta_hash_value, avb_ops) != TEE_SUCCESS)
            {
                goto fail_handler;
            }
            printf("Boot state is: Green !\n");
            return 0;
        }
        else
        {
            UBOOT_ERROR("Get vbmeta digest fail on Boot state is: Green\n");
            goto fail_handler;
        }
    }
    else //halted for red state.
    {
        UBOOT_ERROR("## Android Verify Boot 2.0 fail, AvbSlotVerifyResult: %d\n", avb_slot_verify_result);
        AVBPanelLogo(BOOT_RED);
        UBOOT_ERROR("Boot state is: Red, System halted !\n");
    }

fail_handler:
    system_fail_handler();
    return -1;
}
