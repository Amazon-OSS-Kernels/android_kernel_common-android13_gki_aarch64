// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

#include <MsTypes.h>
#include <halRegOp.h>
#include "halFCIC.h"


MS_U16 HAL_FCIC_Read_Reg(MS_U16 reg)
{
    return HAL_DEMURA_Read2Byte(L_BK_FCIC(reg));
}


void HAL_FCIC_TurnOn(MS_BOOL bOnOff)
{
    MS_U16 regVal = HAL_DEMURA_Read2Byte(L_BK_FCIC(REG_FCIC_BYPASS_ON));

    if (bOnOff == TRUE)
    {
        regVal &= (~BIT15);
    }
    else
    {
        regVal |= (BIT15);
    }
    HAL_DEMURA_Write2Byte(L_BK_FCIC(REG_FCIC_BYPASS_ON), regVal);

    #if(CONFIG_DEMURA_FCIC_SRAM_SHARE)
        regVal = HAL_DEMURA_Read2Byte(0x0133A8);
        regVal &= (~BIT2);
        HAL_DEMURA_Write2Byte(0x0133A8, regVal);
    #endif
}

