// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

#ifndef _HAL_FCIC_H_
#define _HAL_FCIC_H_

#define REG_FCIC_BASE                0x230900  // --> Bank_0x3307 (Main 0x302F, Sub 0x07)

#define REG_FCIC_FLASH_BASE_ADDR     0x50        // 16 bit_mode  ==> offset * 4Byte
#define REG_FCIC_FLASH_DATA_NUM      0x52        // 16 bit_mode  ==> num  * 4Byte
#define FCIC_DATA_UNIT               4
#define FCIC_SRAM_START_ADDR         0x00

#define REG_FCIC_BYPASS_ON            0x54        // 16 bit mode


#define BK_REG_L( x, y )            ((x) | (((y) << 1)))
#define BK_REG_H( x, y )            (((x) | (((y) << 1))) + 1)

/* FCIC */
#define L_BK_FCIC(x)                 BK_REG_L(REG_FCIC_BASE, x)
#define H_BK_FCIC(x)                 BK_REG_H(REG_FCIC_BASE, x)


MS_U16 HAL_FCIC_Read_Reg(MS_U16 reg);
void HAL_FCIC_TurnOn(MS_BOOL bOnOff);

#endif