// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

#include <stdio.h>
#include <common.h>
#include <MsTypes.h>
//#include <MsSystem.h>
#include <drvMIU.h>
#include <debug_impl.h>
#include <drvMMIO.h>
#include <halRegOp.h>
#include "MDrvSIW.h"
#include "halSIW.h"
#include <common.h>

void MDrv_SIW_Demura_AutoDownload_Setup(MS_PHYADDR phy_addr, MS_BOOL bEnable)
{
    DEBUG_SIW_DEMURA("MDrv_SIW_Demura_AutoDownload_Setup IN\n");
#if(TEST_SIW_DEMURA == 1)
    if (bEnable == TRUE)
    {
        HAL_SIW_Demura_Bypass(FALSE);
    }
    else
    {
        HAL_SIW_Demura_Bypass(TRUE);
    }
#endif

    if (bEnable == TRUE)
    {
        HAL_SIW_Demura_SetDL_BaseAddr(phy_addr + SIW_LUT_OFFSET);
        /* SIW Demura HW will set depth/length dynamically */
        //HAL_SIW_DEMURA_SetDL_Depth();
        //HAL_SIW_DEMURA_SetDL_ReqLenth();
        HAL_SIW_Demura_Enable_DL(TRUE);
        HAL_SIW_Demura_Big_Endian(TRUE);
    }
    else
    {
        HAL_SIW_Demura_Enable_DL(FALSE);
    }
    DEBUG_SIW_DEMURA("MDrv_SIW_Demura_AutoDownload_Setup OK\n");
}

void MDrv_SIW_Demura_SPOC_Reg(MS_U8 *pReg)
{
    HAL_SIW_Demura_SPOC_Reg(pReg);
}

MS_BOOL MDrv_SIW_Demura_Checksum_CRC16(MS_U8 *pHdr, MS_U32 lut_len)
{
    MS_U32 i, j;
    MS_U16 crc = 0xFFFF;
    MS_U16 checksum;
    MS_U8 u8_data;

    DEBUG_SIW_DEMURA("IN len %d\n", lut_len);
    // golden
    checksum = *(pHdr + lut_len); // high byte
    checksum = (checksum << 8) | (*(pHdr + lut_len + 1)); // high | low byte

    for(j = 0; j < lut_len; j++)
    {
        u8_data = pHdr[j];
        crc ^= (MS_U16)(u8_data) << 8;

        for(i = 0; i < 8; i++)
        {
            if(crc & 0x8000)
            {
                crc = (crc << 1) ^ 0x8005;
            }
            else
            {
                crc <<= 1;
            }
        }
    }
    DEBUG_SIW_DEMURA("crc = 0x%04X checksum = 0x%04X checksum addr 0x%X\n", crc, checksum, (pHdr + lut_len));
    if (crc != checksum)
    {
        UBOOT_ERROR("CRC failed!\n");
        return FALSE;
    }
    DEBUG_SIW_DEMURA("OK\n");
    return TRUE;

}


void MDrv_SIW_Demura_RD_Done(MS_BOOL bDone)
{
    HAL_SIW_Demura_RD_Done(bDone);
}

