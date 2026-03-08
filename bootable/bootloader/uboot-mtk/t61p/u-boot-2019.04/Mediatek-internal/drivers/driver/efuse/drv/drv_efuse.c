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

//=====================================================================================================

// For debug...
#if( EFUSE_TEST_IN_PC )
unsigned int u32_EFUSE_debug_flag = 0x00;

#else
unsigned int u32_EFUSE_debug_flag = 0x00;
#endif

//=====================================================================================================

int mdrv_efuse_read_subbank_reg_U32( unsigned char u8SubBank, unsigned int * pu32RegVal)
{
    return hal_efuse_read_subbank_reg_U32(u8SubBank, pu32RegVal);
}

