// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

#include <stdio.h>
#include <common.h>
#include <MsTypes.h>
#include <apiPNL.h>

#include <debug_impl.h>
#include <drvMIU.h>
#include "halSIW.h"
#include <ms_utils.h>
#include "MDrvSIW.h"
#include "apiDemura.h"


MS_BOOL MApi_SIW_Enable(MS_PHYADDR phy_addr,MS_BOOL bEnable)
{
    if (bEnable == TRUE)
    {
        MDrv_SIW_Demura_AutoDownload_Setup(phy_addr, TRUE);
    }
    else
    {
        MDrv_SIW_Demura_AutoDownload_Setup(phy_addr, FALSE);
    }

    DEBUG_SIW_DEMURA("phy_addr : 0x%x\n", (uint)phy_addr);
    return TRUE;
}


MS_BOOL MApi_SIW_Init(MS_PHYADDR pBuf)
{
    MS_U64 virt_addr;
    MS_U64 data_addr;

    virt_addr = PA2BA(pBuf);

    DEBUG_SIW_DEMURA("MApi_SIW_Init Go! phy_addr 0x%lx, virt_addr 0x%lx\n", pBuf, virt_addr);

    if (init_spi_flash() != TRUE)
    {
        UBOOT_ERROR("init_spi_flash error!\n");
        MDrv_SIW_Demura_RD_Done(FALSE);
        return FALSE;
    }
    // Read flash to demura buffer
    if (read_spi_flash((MS_U8 *)virt_addr, 0, SIW_DATA_SIZE) != TRUE)
    {
        UBOOT_ERROR("read_spi_flash error!\n");
        MDrv_SIW_Demura_RD_Done(FALSE);
        return FALSE;
    }

    // lut checksum
    data_addr = virt_addr + SIW_LUT_OFFSET;
    DEBUG_SIW_DEMURA("lut checksum data_addr 0x%lX\n", data_addr);
    if (TRUE != MDrv_SIW_Demura_Checksum_CRC16((MS_U8 *)data_addr, SIW_LUT_SIZE))
    {
        UBOOT_ERROR("MDrv_SIW_Checksum_CRC16 LUT error\n");
        MDrv_SIW_Demura_RD_Done(FALSE);
        return FALSE;
    }

    // reg setting checksum
    data_addr += SIW_LUT_SIZE + SIW_LUT_CRC_SIZE;
    DEBUG_SIW_DEMURA("reg checksum data_addr 0x%lX\n", data_addr);
    if (TRUE != MDrv_SIW_Demura_Checksum_CRC16((MS_U8 *)data_addr, SIW_SPOC_REG_SIZE))
    {
        UBOOT_ERROR("MDrv_SIW_Checksum_CRC16 REG error\n");
        MDrv_SIW_Demura_RD_Done(FALSE);
        return FALSE;
    }

    MDrv_SIW_Demura_RD_Done(TRUE);

    MDrv_SIW_Demura_SPOC_Reg((MS_U8 *)data_addr);

    return TRUE;
}


