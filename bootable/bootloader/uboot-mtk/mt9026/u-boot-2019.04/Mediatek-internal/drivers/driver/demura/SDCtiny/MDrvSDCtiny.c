// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

#include <stdio.h>
#include <common.h>
#include <MsTypes.h>
#include <drvBDMA.h>
#include <MsSystem.h>
#include <drvMIU.h>
#include <debug_impl.h>
#include <halSDCtiny.h>
#include <drvMMIO.h>
#include <halRegOp.h>
#include "MDrvSDCtiny.h"

#define SRAM_Length 0x17f

void MDrv_SDCtiny_Lut_sram(MS_U8 *lut_buf)
{
    MS_U16 index = 0; // for select table address

    Hal_SDCtiny_Enable_Sram();
    //for channel 0
    Hal_SDCtiny_Select_Channel0();
    for (index =0; index<=SRAM_Length; index++)
    {
        Hal_SDCtiny_Lut_sram(index,lut_buf);
    }
    //for channel 1
    Hal_SDCtiny_Select_Channel1();
    for (index =0; index<=SRAM_Length; index++)
    {
        Hal_SDCtiny_Lut_sram(index,lut_buf);
    }
}
void MDrv_SDCtiny_Lut_para(void)
{
    Hal_SDCtiny_Lut_para();
}
void MDrv_SDCtiny_IPbypassEn(MS_BOOL bOnOff)
{

    Hal_SDCtiny_IPbypassEn(bOnOff);
}

