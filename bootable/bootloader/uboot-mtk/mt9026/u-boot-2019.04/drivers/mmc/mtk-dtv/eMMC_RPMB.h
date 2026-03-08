/* SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause) */
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

#ifndef eMMC_RPMB
#define eMMC_RPMB

#include "eMMC.h"

#define EMMC_RPMB_REQ_AUTH_KEY         0x0001
#define EMMC_RPMB_REQ_WRITE_CNT_VAL    0x0002
#define EMMC_RPMB_REQ_AUTH_DATA_W      0x0003
#define EMMC_RPMB_REQ_AUTH_DATA_R      0x0004
#define EMMC_RPMB_REQ_RESULT_R         0x0005
#define EMMC_RPMB_DEVICE_CONFIG_R_REQ  0x0007
#define EMMC_RPMB_DEVICE_RESULT_MASK   0x007F


#define EMMC_RPMB_RSP_AUTH_KEY         0x0100
#define EMMC_RPMB_RSP_WRITE_CNT_VAL    0x0200
#define EMMC_RPMB_RSP_AUTH_DATA_W      0x0300
#define EMMC_RPMB_RSP_AUTH_DATA_R      0x0400

#define EMMC_RPMB_NONCE_FIELD_SIZE  16
#define EMMC_RPMB_STUFF_BYTE        196

#define EMMC_RPMB_KEY_SIZE      32
#define EMMC_RPMB_WCNT_MAX_FLAG 0x80

#define EMMC_RPMB_DATA_FIELD_SIZE    256
#define EMMC_RPMB_DATA_FIELD_MASK    0xFF
#define EMMC_RPMB_DATA_FIELD_SHIFT   8
#define EMMC_RPMB_MAC_CAL_FIELD_SIZE 284

struct EMMC_RPMB_DATA
{
   U16 u16_req_rsp;
   U16 u16_result;
   U16 u16_blk_cnt;
   U16 u16_addr;
   U32 u32_writecnt;
   U8  u8_nonce[16];
   U8  u8_data[256];
   U8  u8_auth_key[32];
   U8  u8_stuff[196];
} EMMC_PACK1;
int get_hmac_key(unsigned char *p_hmac_key);
U32 emmc_rpmb_program_auth_key(void);
U32 emmc_rpmb_get_counter_for_cmd(U32 *pu32_wcnt);
U32 emmc_rpmb_get_counter(U32 *pu32_wcnt);
//U32 emmc_rpmb_read_result(U16 *pu16_result);
U32 emmc_rpmb_no_data_req(void);
U32 emmc_rpmb_if_key_written(void);
U32 emmc_rpmb_verify_hmac(U8 *pu8_data_buf, U16 u16_blk_cnt);
U32 emmc_rpmb_write_data(U8 *pu8_data_buf, U32 u32_data_byte_cnt, U16 u16_blk_addr);
U32 emmc_rpmb_read_data(U8 *pu8_data_buf, U32 u32_data_byte_cnt, U16 u16_blk_addr);
U32 emmc_rpmb_read_blk(U8 *pu8_data_buf, U8 *pu8_nonce, U16 u16_blk_addr);
U32 emmc_rpmb_write_blk(U8 *pu8_data_buf);
U32 emmc_rpmb_test(void);
#endif // eMMC_RPMB
