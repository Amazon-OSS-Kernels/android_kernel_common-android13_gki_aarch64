// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (c) 2023 MediaTek Inc.
*/

#ifndef EFUSE_TEST_IN_PC
#define EFUSE_TEST_IN_PC    0
#endif

#include "efuse_common.h"

#include "drv_efuse.h"

//=====================================================================================================
#include "hal_efuse.h"
#include <common.h>
#include <dm.h>
#include <wdt.h>
#include <dm/uclass-internal.h>
#include <asm/io.h>

//=====================================================================================================

// For debug...
#if( EFUSE_TEST_IN_PC )
unsigned int u32_EFUSE_debug_flag = 0x00;

#else
unsigned int u32_EFUSE_debug_flag = 0x00;
#endif

//=====================================================================================================
#define EFUSE_REG_SIZE 3
#define E_EFUSE_26 0x26
#define E_EFUSE_27 0x27
#define E_EFUSE_REVERSE_EN 1//0: disable  1: enable
#define E_EFUSE_REVERSE_NO 0//0: enable  1: disable
#define E_EFUSE_ID_INDEX 1
#define E_EFUSE_REVERSE_INDEX 2

int efuse_RegInfo_by_ID[MAX_SPPORT_EFUSE][EFUSE_REG_SIZE]=
{
    //ID bank
    {E_EFUSE_IDX_126, E_EFUSE_26, E_EFUSE_REVERSE_NO},
    {E_EFUSE_IDX_45, E_EFUSE_27, E_EFUSE_REVERSE_NO}, // E_DRV_EFUSE_ID_HDMI1_4_OR_2_1 --> use to check 2K or 4K
};

//===============================================
int _efuse_hal_get_RegInfo_by_ID(unsigned long int id, unsigned int * p32Bank, unsigned int *reverse)
{
    for (unsigned int i=0; i< MAX_SPPORT_EFUSE; i++)
    {
        if (efuse_RegInfo_by_ID[i][0] == id)
        {
            *p32Bank = efuse_RegInfo_by_ID[i][E_EFUSE_ID_INDEX];
            *reverse = efuse_RegInfo_by_ID[i][E_EFUSE_REVERSE_INDEX];
            return K_EFUSE_R_OK;
        }
    }
    return K_EFUSE_R_NG;
}

//===========================================
int mtk_efuse_check_IP(unsigned long int id, unsigned int * p32Value)
{
    unsigned int u32Bank;
    unsigned int reverse;
    if (_efuse_hal_get_RegInfo_by_ID(id, &u32Bank, &reverse) == K_EFUSE_R_OK)
    {
        mdrv_efuse_read_subbank_reg_U32(u32Bank, p32Value);
        if (reverse == 0)
        {
            *p32Value = ((*p32Value == 0) ? true : false); // 0: enable. 1: disable
        }
        else
        {
            *p32Value = ((*p32Value == 0) ? false : true); // 0: disable. 1: enable
        }

        return K_EFUSE_R_OK;
    }
    return K_EFUSE_R_NG;
}

int mdrv_efuse_read_subbank_reg_U32( unsigned char u8SubBank, unsigned int * pu32RegVal)
{
    return hal_efuse_read_subbank_reg_U32(u8SubBank, pu32RegVal);
}

