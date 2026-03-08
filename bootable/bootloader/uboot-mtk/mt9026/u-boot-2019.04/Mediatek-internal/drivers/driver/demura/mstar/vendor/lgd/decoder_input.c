// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

#include <command.h>
#include <common.h>
#include <stdio.h>
#include <linux/string.h>
#include <MsTypes.h>
#include <debug_impl.h>
#include <dmalloc.h>
#include <ms_utils.h>
#include <demura.h>
#include <crc_libs.h>
#include <demura_config.h>
#include <CommonDataType.h>
#include <convert_entry.h>

#include <apiPNL.h>
#include <MDrvDemura.h>
#include <halDemura.h>
#include "vendor.h"
#include "parse_fcic.h"

MS_BOOL If_Need_Decode(void)
{
    UBOOT_TRACE("IN\n");
    if (init_spi_flash() != TRUE)
    {
        UBOOT_DEBUG("init_spi_flash error!\n");
        return TRUE;
    }

    char *sig_str = env_get(ENV_DEMURA_SIG);
    if (sig_str == NULL)
    {
        UBOOT_DEBUG("Can not find env(%s)\n", ENV_DEMURA_SIG);
        return TRUE;
    }
    else
    {
        MS_U8  checksum_sf;
        MS_U32 checksum_bd;
        MS_U32 sample_crc32, cal_crc32;
        MS_U8  buf[SAMPLE_DAT_LEN];
        MS_U32 lgd_checksum_adr, lgd_mura_start_adr;

        #if defined (CONFIG_DEMURA_VENDOR_LGD_V19)
            lgd_checksum_adr = 0xA1A3C;
            lgd_mura_start_adr = 0x80000;
        #elif defined (CONFIG_DEMURA_VENDOR_LGD_SQ18_19)
            lgd_checksum_adr = 0x5c310;
            lgd_mura_start_adr = 0x00000;
        #elif defined (CONFIG_DEMURA_VENDOR_LGD_V18)
            lgd_checksum_adr = 0x685f0;
            lgd_mura_start_adr = 0x60000;
        #endif

        if (read_spi_flash(&checksum_sf, lgd_checksum_adr, sizeof(checksum_sf)) == FALSE)
        {
            UBOOT_ERROR("Read LGD Demura Checksum Failed !\n");
            return TRUE;
        }
        UBOOT_DEBUG("checksum_sf  = 0x%x\n", (uint)checksum_sf);

        UBOOT_DEBUG("sig_str = %s\n", sig_str);
        if(2 == sscanf(sig_str, "0x%x:0x%x", &checksum_bd, &sample_crc32))
        {
            UBOOT_DEBUG("checksum_bd  = 0x%x\n", (uint)checksum_bd);
            UBOOT_DEBUG("sample_crc32 = 0x%x\n", (uint)sample_crc32);

            if (read_spi_flash(buf, (lgd_mura_start_adr + SAMPLE_DAT_START), SAMPLE_DAT_LEN) == TRUE)
            {
                cal_crc32 = MDrv_CRC32_Cal_DeMura(buf, SAMPLE_DAT_LEN);
                UBOOT_DEBUG("cal_crc32 = 0x%x\n", (uint)cal_crc32);
                if ((sample_crc32 == cal_crc32) && (checksum_bd == checksum_sf))
                {
                    UBOOT_DEBUG("Data Match, No Need to decode again\n");
                    UBOOT_TRACE("OK\n");
                    return FALSE;
                }
            }
        }
        UBOOT_TRACE("OK\n");
        return TRUE;
    }

    UBOOT_TRACE("OK\n");
    return FALSE;
}

MS_BOOL Decode_To_Mstar_Format(void *pDataInfo, BinOutputInfo *pbin_info)
{
    MS_BOOL bRet;

#ifndef CONFIG_DEMURA_LGD_DEMO
    bRet = parse_LG_bin(pDataInfo, pbin_info);
    if (bRet == FALSE)
    {
        UBOOT_ERROR("parse_LG_bin error\n");
        return FALSE;
    }

    // Decode Vendor data
    bRet = set_u13_interface(pDataInfo, pbin_info);
    if (bRet == FALSE)
    {
        UBOOT_ERROR("set_u13_interface error\n");
        return FALSE;
    }
#else
    bRet = transfer_LGD_Data_1tab(pDataInfo, pbin_info);
    if (bRet == FALSE)
    {
        UBOOT_ERROR("transfer_LGD_Data_1tab error\n");
        return FALSE;
    }
#endif

    #if (defined (CONFIG_DEMURA_M7622) || defined (CONFIG_DEMURA_M7632))
        MS_BOOL demura_enable[4]  = {TRUE, TRUE, TRUE, TRUE};
    #else
    MS_BOOL demura_enable[3]  = {TRUE, TRUE, TRUE};
    #endif

    pbin_info->Build_Date     =  0x18032315;  // 2018.03.23
    pbin_info->reg_base_addr  =  0x7700;
    pbin_info->project_id     =  ((MSTAR_CHIP_ID << 16) + (1ULL << 15) + ID_CUS_LGD);
    pbin_info->data_type      =  IC_DRAM;
    pbin_info->Gain_type      =  10;
    pbin_info->Vendor_ID      =  EN_DEMURA_ID_LGD;

    memcpy(pbin_info->channel_enable, demura_enable, sizeof(demura_enable));
    return TRUE;
}

