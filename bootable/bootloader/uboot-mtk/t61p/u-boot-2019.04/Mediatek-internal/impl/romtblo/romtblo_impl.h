/* SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause) */
/*
 * Copyright (c) 2023 MediaTek Inc.
*/

#ifndef _ROM_TABLE_IMPL_HEADER_
#define _ROM_TABLE_IMPL_HEADER_
#include <linux/list.h>
#include <mtk_romtblo.h>

#define ROMTBL_HEADER_SIZE      (32)
#define ROMTBL_SIZE             (16*1024)
#define ROMTBL_DEVENTRY_SIZE    (15*1024)
#define ROMTBL_REGISTER_SIZE    (16)
#define ROMTBL_REGISTER_PAIR    (4)
#define ROMTBL_CAPABILITY_SIZE  (4)
#define ROMTBL_64BIT_HIGH_BYTE_VALUE  (0x0)
#define GIC_INTERRUPT_BASE      (0x20)
#define MEDIATEK_TV_CHIP_ID_MACHLI	(5896)
#define MEDIATEK_TV_CHIP_ID_MANKS	(5897)
#define MEDIATEK_TV_CHIP_ID_MOKONA	(5876)
#define MEDIATEK_TV_CHIP_ID_MIFFY	(5879)
#define MEDIATEK_TV_CHIP_ID_MOKA	(5873)
#define CHIP_HARDWARE_KEY   "androidboot.vendor.cf.hardware"
#define CHIP_HARDWAREREV_KEY   "androidboot.vendor.cf.hardwarerev"
#define CHIP_ID_CHAR   "MT"
#define CHIP_REVISION_CHAR   "E"
#define BUFFER_SIZE     64
#define CHIP_ID_BYTE_COUNT     (4)
#define HEX_DIG_SIZE    (0x10)
#define BINARY_PRE_BYTE     (4)
#define BIT_PRE_BYTE     (8)
#define DECIMAL_BASE     (10)
#define CHIP_ID_BASE     (22)
#define REVISION_BASE     (18)


enum CHIP_FAMILY_ID{
    MT5896 = 0,
    MT5897,
    MT5876,
    MT5879,
    MT5873,
    NOT_SUPPORT
};

typedef struct{
    int interrupt_base;
    int size;
    struct list_head list;
}ROM_GIC_LIST_T;

typedef struct
{
    unsigned int major_ver;
    unsigned int minor_ver;
    unsigned int chip;
    unsigned int revision;
}romtbl_header;

typedef struct
{
    unsigned short unique_id;                     // 2 Bytes
    unsigned char hw_ip_major_ver;               // 1 Byte
    unsigned char hw_ip_minor_ver;               // 1 Byte
    unsigned short gic_interrupt_num;           // 2 Bytes
    unsigned short register_size;                 // 2 Bytes
    unsigned int register_base_addr;          // 4 Bytes
    unsigned int register_size_array[ROMTBL_REGISTER_SIZE];             // 4 Bytes * 12
    unsigned int register_base_add_array[ROMTBL_REGISTER_SIZE];      // 4 Bytes * 12
    unsigned int capability[ROMTBL_CAPABILITY_SIZE];                // 4 Bytes or 4*4 Bytes Extension
} __attribute__((packed)) romtbl_info;

int romtbl_get_chip_id_info(void);
#endif /* _ROM_TABLE_IMPL_HEADER_ */
