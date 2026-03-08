/* SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause) */
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

#ifndef __DRV_EMMC_H__
#define __DRV_EMMC_H__

#ifndef U32
#define U32  unsigned long
#endif
#ifndef U16
#define U16  unsigned short
#endif
#ifndef U8
#define U8   unsigned char
#endif
#ifndef S32
#define S32  signed long
#endif
#ifndef S16
#define S16  signed short
#endif
#ifndef S8
#define S8   signed char
#endif

/*=============================================================*/
// Include files
/*=============================================================*/
#include "eMMC_config.h"

/*=============================================================*/
// Extern definition
/*=============================================================*/

/*=============================================================*/
// Macro definition
/*=============================================================*/

/*=============================================================*/
// Data type definition
/*=============================================================*/

/*=============================================================*/
// Variable definition
/*=============================================================*/

/*=============================================================*/
// Global function definition
/*=============================================================*/
U32  emmc_init(void);
U32  emmc_init_device(void);
U32  emmc_init_device_ex(void);
U32 emmc_load_images(U32 *pu32_addr, U32 u32_byte_cnt, U32 u32_hash_stage);
//--------------------------------------------
// CAUTION: u32_data_byte_cnt has to be 512B x n
//--------------------------------------------
U32  emmc_write_data_ex(U8 *pu8_data_buf, U32 u32_data_byte_cnt, U32 u32_blk_addr);
U32  emmc_read_data_ex(U8 *pu8_data_buf, U32 u32_data_byte_cnt, U32 u32_blk_addr);
// skip driver-reserved area
U32  emmc_write_data(U8 *pu8_data_buf, U32 u32_data_byte_cnt, U32 u32_blk_addr);
U32  emmc_read_data(U8 *pu8_data_buf, U32 u32_data_byte_cnt, U32 u32_blk_addr);
U32  emmc_get_capacity(U32 *pu32_total_sector_cnt); // 1 sector = 512B
U32  emmc_erase_block(U32 u32_emmc_blk_addr_start, U32 u32_emmc_blk_addr_end);
U32  emmc_get_cid(U8 *pu8_cid); // pu8_CID needs to hold 16 bytes
//--------------------------------------------


/*=============================================================*/
// internal function definition
/*=============================================================*/
U32  emmc_write_boot_part(U8 *pu8_data_buf, U32 u32_data_byte_cnt, U32 u32_blk_addr, U8 u8_part_no);
U32  emmc_read_boot_part(U8 *pu8_data_buf, U32 u32_data_byte_cnt, U32 u32_blk_addr, U8 u8_part_no);
U32  emmc_check_if_ready(void);
void emmc_reset_ready_flag(void);
void emmc_dump_driver_status(void);
void emmc_dump_speed_status(void);
U32  emmc_fcie_build_ddr_timing_table(void);
U32  emmc_fcie_build_hS200_timing_table(void);
void emmc_print_gp_partition(void);

#endif //__DRV_EMMC_H__

