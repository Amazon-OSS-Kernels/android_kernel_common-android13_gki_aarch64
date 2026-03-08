/* SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause) */
/*
 * Copyright (c) 2023 MediaTek Inc.
 */


#include <debug_impl.h>
#include <program_efuse_rollback_index.h>
#include <utility.h>

#define TA_RAMLOG_UUID                                                                             \
    {                                                                                              \
        0xa9aa0a93, 0xe9f5, 0x4234, { 0x8f, 0xec, 0x21, 0x09, 0xcb, 0xa2, 0xf6, 0x70 }             \
    }
#define TA_CMD_WRITE_EFUSE_VERSION 106
#define PARAM_NUM 2

unsigned int CountBitsFromU32(unsigned int val)
{
    unsigned int count = 0;
    while (val)
    {
        val &= (val - 1);
        count++;
    }
    return count;
}

unsigned int EFuseBitsToVersion(unsigned int val, unsigned int *Is_Inconsistent)
{
    // Version        EFuseBits       96bit EFuseData (3 subbanks)
    // 0x0            0x0             0x00000000 0x00000000 0x00000000
    // 0x1            0x2             0x00000000 0x00000000 0x00000003
    // 0x2            0x4             0x00000000 0x00000000 0x0000000F
    // 0x3            0x6             0x00000000 0x00000000 0x0000003F
    // 0x4            0x8             0x00000000 0x00000000 0x000000FF
    // 0x5            0xA             0x00000000 0x00000000 0x000003FF
    // 0x6            0xC             0x00000000 0x00000000 0x00000FFF
    // 0x7            0xE             0x00000000 0x00000000 0x00003FFF
    // 0x8            0x10            0x00000000 0x00000000 0x0000FFFF
    // 0x9            0x12            0x00000000 0x00000000 0x0003FFFF
    // 0xA            0x14            0x00000000 0x00000000 0x000FFFFF
    // 0xB            0x16            0x00000000 0x00000000 0x003FFFFF
    // 0xC            0x18            0x00000000 0x00000000 0x00FFFFFF
    // 0xD            0x1A            0x00000000 0x00000000 0x03FFFFFF
    // 0xE            0x1C            0x00000000 0x00000000 0x0FFFFFFF
    // 0xF            0x1E            0x00000000 0x00000000 0x3FFFFFFF
    // 0x10           0x20            0x00000000 0x00000000 0xFFFFFFFF
    // 0x11           0x22            0x00000000 0x00000003 0xFFFFFFFF
    // 0x12           0x24            0x00000000 0x0000000F 0xFFFFFFFF
    //...
    // 0x20           0x40            0x00000003 0xFFFFFFFF 0xFFFFFFFF
    // 0x21           0x42            0x00000003 0xFFFFFFFF 0xFFFFFFFF
    //...
    // 0x2F           0x5E            0x3FFFFFFF 0xFFFFFFFF 0xFFFFFFFF
    // 0x30           0x60            0xFFFFFFFF 0xFFFFFFFF 0xFFFFFFFF
    // Each increase takes 2 bits of efuse, so number of bits doubles its actual version number.
    // Roundup odd values to even values
    if (val & 1)
    {
        val++;
        if (Is_Inconsistent)
        {
            *Is_Inconsistent = 1;
        }
    }
    return val >> 1;
}

static AvbIOResult program_efuse_rollback_index(const struct VERSION_INFO *rollback_index)
{
    struct udevice *tee = NULL;
    const struct tee_optee_ta_uuid uuid = TA_RAMLOG_UUID;
    struct tee_open_session_arg sess_arg;
    struct tee_invoke_arg invoke_arg;
    struct tee_param param[PARAM_NUM];
    void *Invokebuffer = NULL;
    int rc = AVB_IO_RESULT_ERROR_IO;
    // efuse_set_attr == 0: use real efuse
    // efuse_set_attr == 1: use virtual efuse
    unsigned int efuse_set_attr = 0;

    if (!rollback_index)
        return AVB_IO_RESULT_ERROR_INVALID_VALUE_SIZE;

    Invokebuffer = malloc_TEE(sizeof(struct VERSION_INFO));
    if (!Invokebuffer)
    {
        return AVB_IO_RESULT_ERROR_INSUFFICIENT_SPACE;
    }
    memcpy(Invokebuffer, rollback_index, sizeof(struct VERSION_INFO));

    tee = tee_find_device(tee, NULL, NULL, NULL);
    if (!tee)
        return AVB_IO_RESULT_ERROR_IO;

    memset(&sess_arg, 0, sizeof(struct tee_open_session_arg));
    tee_optee_ta_uuid_to_octets(sess_arg.uuid, &uuid);
    if (tee_open_session(tee, &sess_arg, 0, NULL))
    {
        UBOOT_ERROR("Failed to open session\n");
        goto free_mem;
    }

    memset(&invoke_arg, 0, sizeof(struct tee_invoke_arg));
    invoke_arg.func = TA_CMD_WRITE_EFUSE_VERSION;
    invoke_arg.session = sess_arg.session;
    memset(param, 0, sizeof(param));
    param[0].attr = TEE_PARAM_ATTR_TYPE_MEMREF_INPUT;
    param[0].u.memref.shm_offs = (ulong)Invokebuffer;
    param[0].u.memref.size = sizeof(struct VERSION_INFO);
    param[1].attr = TEE_PARAM_ATTR_TYPE_VALUE_INPUT;
    param[1].u.value.a = efuse_set_attr;
    if (tee_invoke_func(tee, &invoke_arg, PARAM_NUM, param))
    {
        UBOOT_ERROR("Failed to invoke\n");
        goto close;
    }

    if (invoke_arg.ret != TEE_SUCCESS)
    {
        UBOOT_ERROR("tee_invoke result code=0x%x\n", invoke_arg.ret);
        goto close;
    }
    rc = AVB_IO_RESULT_OK;
close:
    tee_close_session(tee, sess_arg.session);
free_mem:
    free_TEE(Invokebuffer);
    return rc;
}

AvbIOResult read_rollback_index_by_efuse(size_t rollback_index_slot, u64 *out_rollback_index)
{
    unsigned int i = 0, val = 0, subbank_start = 0, subbank_end = 0;
    unsigned int Is_Inconsistent = 0;
    switch (rollback_index_slot)
    {
    case VBMETA_ROLLBACK_INDEX_LOCATION:
    {
        subbank_start = EFUSE_VBMETA_VER;
        subbank_end = EFUSE_BOOT_VER;
        break;
    }
    case BOOT_ROLLBACK_INDEX_LOCATION:
    {
        subbank_start = EFUSE_BOOT_VER;
        subbank_end = EFUSE_RECOVERY_VER;
        break;
    }
    case RECOVERY_ROLLBACK_INDEX_LOCATION:
    {
        subbank_start = EFUSE_RECOVERY_VER;
        subbank_end = EFUSE_RESERVED;
        break;
    }
    default:
    {
        return AVB_IO_RESULT_ERROR_NO_SUCH_VALUE;
    }
    }
    *out_rollback_index = 0;
    for (i = subbank_start; i < subbank_end; i++)
    {
        if (mdrv_efuse_read_subbank_reg_U32(i, &val))
        {
            return AVB_IO_RESULT_ERROR_IO;
        }
        *out_rollback_index += CountBitsFromU32(val);
    }
    *out_rollback_index =
        (u64)EFuseBitsToVersion((unsigned int)*out_rollback_index, &Is_Inconsistent);
    if (Is_Inconsistent)
    {
        // In case of Is_Inconsistent, we need to notify rollbackindexes_to_efuse() to pack version
        // info to ramlog TA despite the need of version update. It creates another chance to make
        // versions in efuse consistent.
        if (set_pass_rollback_indexes_needed() != AVB_IO_RESULT_OK)
        {
            return AVB_IO_RESULT_ERROR_IO;
        }
        UBOOT_ERROR("EFuse Inconsistency! rollback index needs to update\n");
    }
    return AVB_IO_RESULT_OK;
}

AvbIOResult read_pmu_rollback_index_by_efuse(u64 *out_rollback_index)
{
    unsigned int i = 0, val = 0;
    unsigned int Is_Inconsistent = 0;
    u64 ver_pmu = 0;
    for (i = EFUSE_PMU_VER; i < EFUSE_VBMETA_VER; i++)
    {
        if (mdrv_efuse_read_subbank_reg_U32(i, &val))
        {
            return AVB_IO_RESULT_ERROR_IO;
        }
        ver_pmu += CountBitsFromU32(val);
    }
    ver_pmu = (u64)EFuseBitsToVersion((unsigned int)ver_pmu, &Is_Inconsistent);
    if (Is_Inconsistent)
    {
        // In case of Is_Inconsistent, we need to notify rollbackindexes_to_efuse() to pack version
        // info to ramlog TA despite the need of version update. It creates another chance to make
        // versions in efuse consistent.
        if (set_pass_rollback_indexes_needed() != AVB_IO_RESULT_OK)
        {
            return AVB_IO_RESULT_ERROR_IO;
        }
        UBOOT_ERROR("EFuse Inconsistency! pmu's rollback index needs to update\n");
    }
    *out_rollback_index = ver_pmu;
    return AVB_IO_RESULT_OK;
}

bool is_slot_in_efuse(size_t rollback_index_slot)
{
    return (rollback_index_slot == VBMETA_ROLLBACK_INDEX_LOCATION ||
            rollback_index_slot == RECOVERY_ROLLBACK_INDEX_LOCATION ||
            rollback_index_slot == BOOT_ROLLBACK_INDEX_LOCATION);
}

int rollbackindexes_to_efuse(AvbSlotVerifyData *AvbData)
{
    struct VERSION_INFO efuse_rollback_index;
    memset(&efuse_rollback_index, 0, sizeof(struct VERSION_INFO));
    // We only pass versions to ramlog TA if version update is needed.
    // If it's not needed, Uboot sends all-zero versions to efuse programmer via ramlog TA, and it will skip processing them.
    if (is_pass_rollback_index_info_needed())
    {
        // Assigning efuse_rollback_index.ver_pmu.
        efuse_rollback_index.ver_pmu = get_pmu_rollback_index();
        if (AvbData->rollback_indexes[VBMETA_ROLLBACK_INDEX_LOCATION] != 0)
        {
            UBOOT_INFO("Update avb vbmeta index = %lld\n",
                       AvbData->rollback_indexes[VBMETA_ROLLBACK_INDEX_LOCATION]);
            efuse_rollback_index.ver_vbmeta =
                AvbData->rollback_indexes[VBMETA_ROLLBACK_INDEX_LOCATION];
        }
        if (AvbData->rollback_indexes[BOOT_ROLLBACK_INDEX_LOCATION] != 0)
        {
            UBOOT_INFO("Update avb boot index = %lld\n",
                       AvbData->rollback_indexes[BOOT_ROLLBACK_INDEX_LOCATION]);
            efuse_rollback_index.ver_boot = AvbData->rollback_indexes[BOOT_ROLLBACK_INDEX_LOCATION];
        }
        if (AvbData->rollback_indexes[RECOVERY_ROLLBACK_INDEX_LOCATION] != 0)
        {
            UBOOT_INFO("Update avb recovery index = %lld\n",
                       AvbData->rollback_indexes[RECOVERY_ROLLBACK_INDEX_LOCATION]);
            efuse_rollback_index.ver_recovery =
                AvbData->rollback_indexes[RECOVERY_ROLLBACK_INDEX_LOCATION];
        }
    }
    UBOOT_DUMP(&efuse_rollback_index, sizeof(struct VERSION_INFO));
    return (program_efuse_rollback_index(&efuse_rollback_index) != AVB_IO_RESULT_OK);
}

AvbIOResult get_efuse_rollback_enabling_bit(unsigned int* val)
{
    if (mdrv_efuse_read_subbank_reg_U32(EFUSE_RESERVED_FOR_USER_1, val))
    {
        return AVB_IO_RESULT_ERROR_IO;
    }
    *val = *val & EFUSR_ROLLBACK_ENABLING_BIT_MASK;
    return AVB_IO_RESULT_OK;
}
