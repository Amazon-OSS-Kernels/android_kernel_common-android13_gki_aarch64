/* SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause) */
/*
 * Copyright (c) 2023 MediaTek Inc.
*/

#ifndef _DRV_EFUSE_H_
#define _DRV_EFUSE_H_

#define K_EFUSE_R_OK                        0
#define K_EFUSE_R_NG                        -1
#define K_EFUSE_R_ERR_INVALID_PARA          -2
#define K_EFUSE_R_ERR_INIT_NOT_DONE         -3
#define K_EFUSE_R_ERR_EFUSE_ID_NOT_SUPPORT  -4
#define K_EFUSE_R_ERR_WAIT_HW_TIMEOUT       -5
#define K_EFUSE_R_ERR_READ_SUBBANK_FAIL     -6

//=====================================================================================================
//PM chip ID
#define UBOOT_BASE_ADDR 0x1C000000
//=====================================================================================================

#define MAX_SPPORT_EFUSE 2
typedef enum
{
    E_EFUSE_IDX_MIN = 1,
    E_EFUSE_IDX_45 = 0x42C,
    E_EFUSE_IDX_126 = 0x47D, // Dynamic OSD PQ
    E_EFUSE_IDX_MAX
} E_EFUSE_IDX;


#define REG_CPU_BASE    (0x1C000000)
// pm_top
#define REG_PM_TOP_BASE    (0x21000) // bank_108
#define REG_PM_TOP_CHIP_ID  (0x00)

#define EFUSE_REG_ACCESS(addr, idx)  *(volatile unsigned short*)(PMU_BASE_ADDR + ((addr << 9) + (idx << 2)))

// Chip ID ...
#define CHIP_ID_MACHL_I  0x0101
#define CHIP_ID_MANK_S   0x0109
#define CHIP_ID_MOKON_A  0x010C
#define CHIP_ID_MIFF_Y   0x010D
#define CHIP_ID_MOK_A    0x0111

int mtk_efuse_check_IP(unsigned long int id, unsigned int * p32Value);

int mdrv_efuse_read_subbank_reg_U32( unsigned char u8SubBank, unsigned int * pu32RegVal);

#endif

