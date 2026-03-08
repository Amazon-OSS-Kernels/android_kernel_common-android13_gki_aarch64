// SPDX-License-Identifier: (GPL-2.0 OR BSD-3-Clause)
/*
 * Copyright (c) 2021 MediaTek Inc.
 */
#ifndef __MTK_NJPD_H
#define __MTK_NJPD_H

//#define BOOTLOGO_FILE_NAME_LENGTH 0x10

typedef struct {
    //char name[BOOTLOGO_FILE_NAME_LENGTH];
    void __iomem *jpd_reg_base;
    void __iomem *jpd_ext_reg_base;
    struct clk clk_njpd;
    struct clk clk_smi2jpd;
    struct clk clk_njpd2jpd;
    unsigned int read_buffer;
    unsigned int read_size;
    unsigned int inter_buffer;
    unsigned int inter_size;
    unsigned int output_buffer;
    unsigned int output_size;
} MTK_NJPD_PRIV;

typedef enum {
    E_NJPD_BUF_NONE = 0,
    E_NJPD_BUF_HIGH = 1,
    E_NJPD_BUF_LOW  = 2,
    E_NJPD_BUF_0    = 3,
    E_NJPD_BUF_1    = 4,
} EN_NJPD_BUF_PART;

#endif /* __MTK_NJPD_H */
