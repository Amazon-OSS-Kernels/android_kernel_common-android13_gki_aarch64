/* SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause) */
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

#define ckgen00 0x00204000

#define REG_14E4_CKGEN00 (ckgen00 + 0x14E4)
    #define REG_SW_EN_SMI2GE Fld(1,11,AC_MSKB1)//[11:11]

#define ckgen01 0x00206000

#define REG_02A0_CKGEN01 (ckgen01 + 0x2A0)
    #define REG_02A0_CKGEN01_REG_CKG_GE Fld(5,0,AC_MSKB0)//[4:0]
#define REG_15CC_CKGEN01 (ckgen01 + 0x15CC)
    #define REG_15CC_CKGEN01_REG_SW_EN_GE_FIFO_R2GE Fld(1,0,AC_MSKB0)//[0:0]
#define REG_15D0_CKGEN01 (ckgen01 + 0x15D0)
    #define REG_15D0_CKGEN01_REG_SW_EN_GE_FIFO_W2GE Fld(1,0,AC_MSKB0)//[0:0]
#define REG_15D4_CKGEN01 (ckgen01 + 0x15D4)
    #define REG_15D4_CKGEN01_REG_SW_EN_GE_PSRAM2GE Fld(1,0,AC_MSKB0)//[0:0]
#define REG_15D8_CKGEN01 (ckgen01 + 0x15D8)
    #define REG_15D8_CKGEN01_REG_SW_EN_GE2GE Fld(1,0,AC_MSKB0)//[0:0]
#define REG_1B74_CKGEN01 (ckgen01 + 0x1B74)
    #define REG_1B74_CKGEN01_REG_SW_EN_MCU_NONPM2GE Fld(1,8,AC_MSKB1)//[8:8]