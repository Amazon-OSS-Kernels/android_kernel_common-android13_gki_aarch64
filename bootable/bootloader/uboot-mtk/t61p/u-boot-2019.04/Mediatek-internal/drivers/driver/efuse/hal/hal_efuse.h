/* SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause) */
/*
 * Copyright (c) 2023 MediaTek Inc.
*/

#ifndef _HAL_EFUSE_H_
#define _HAL_EFUSE_H_


#define K_EFUSE_R_OK                        0
#define K_EFUSE_R_NG                        -1
#define K_EFUSE_R_ERR_INVALID_PARA          -2
#define K_EFUSE_R_ERR_INIT_NOT_DONE         -3
#define K_EFUSE_R_ERR_EFUSE_ID_NOT_SUPPORT  -4
#define K_EFUSE_R_ERR_WAIT_HW_TIMEOUT       -5
#define K_EFUSE_R_ERR_READ_SUBBANK_FAIL     -6

int hal_efuse_read_subbank_reg_U32( unsigned char u8SubBank, unsigned int * pu32RegVal);

#endif
