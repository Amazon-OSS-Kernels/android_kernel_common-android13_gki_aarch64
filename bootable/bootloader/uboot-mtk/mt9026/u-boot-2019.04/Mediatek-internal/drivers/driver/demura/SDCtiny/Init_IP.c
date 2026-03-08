// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

#include <stdio.h>
#include <common.h>
#include <MsTypes.h>
#include <apiPNL.h>
#include <drvBDMA.h>
#include <MsSystem.h>
//#include <MsStr.h>
#include <debug_impl.h>
#include <MsSysUtility.h>
#include <drvMIU.h>
#include <halSDCtiny.h>
#include "MDrvSDCtiny.h"
#include <ms_utils.h>
#include "apiDemura.h"
#include <MsApiMspi.h>


//#define DBG_WITHOUT_TCON    1

#undef ALIGN
#define ALIGN(x,a)        __ALIGN_MASK((x),(typeof(x))(a)-1)
#define __ALIGN_MASK(x,mask)    (((x)+(mask))&~(mask))


MS_BOOL MApi_SDCtiny_Enable(void)
{


    return TRUE;
}



MS_BOOL MApi_SDCtiny_Init(MS_PHYADDR phy_addr,MS_U32 lut_virtaddr)
{
    //
    //need copy lut data to dram by spi or bdma (?)
    //MS_U16 lut_size = 0x1842; //(0x17f+1)*2 + 0x42
    //MDrv_BDMA_CopyHnd (0x00, phy_addr, lut_size, E_BDMA_SPI12DRAM, 0);
    //
    //
    MDrv_SDCtiny_IPbypassEn(FALSE);
    MDrv_SDCtiny_Lut_sram((MS_U8 *)lut_virtaddr);
    MDrv_SDCtiny_Lut_para();
    return TRUE;
}


