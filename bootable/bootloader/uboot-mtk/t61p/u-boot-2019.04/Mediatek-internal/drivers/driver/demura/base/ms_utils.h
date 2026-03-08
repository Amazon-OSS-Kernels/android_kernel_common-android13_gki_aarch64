// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

#ifndef _MS_UTILS_H_
#define _MS_UTILS_H_
#include "MsTypes.h"

typedef enum {
    E_MS_UTIL_BIN_TYPE_MAIN,
    E_MS_UTIL_BIN_TYPE_DLG,
    E_MS_UTIL_BIN_TYPE_MAX
}EN_MS_UTIL_BIN_TYPE;

typedef enum
{
    E_MS_UTIL_BIN_ACT_OFF,
    E_MS_UTIL_BIN_ACT_FIRST = E_MS_UTIL_BIN_ACT_OFF,
    E_MS_UTIL_BIN_ACT_ON,
    E_MS_UTIL_BIN_ACT_LAST = E_MS_UTIL_BIN_ACT_ON ,
    E_MS_UTIL_BIN_ACT_NO_DLG,
    E_MS_UTIL_BIN_ACT_MAX
}EN_MS_UTIL_BIN_ACT;

typedef enum
{
    E_MS_UTIL_BIN_FILE_NORMAL,
    E_MS_UTIL_BIN_FILE_BACKLIGHT,
    E_MS_UTIL_BIN_FILE_BACKLIGHT_ONLY,
    E_MS_UTIL_BIN_FILE_MAX
}EN_MS_UTIL_BIN_File;

typedef enum
{
    E_MS_UTIL_BLOCK_SIZE_H4V4,
    E_MS_UTIL_BLOCK_SIZE_H4V8,
    E_MS_UTIL_BLOCK_SIZE_H4V16,

    E_MS_UTIL_BLOCK_SIZE_H8V4,
    E_MS_UTIL_BLOCK_SIZE_H8V8,
    E_MS_UTIL_BLOCK_SIZE_H8V16,

    E_MS_UTIL_BLOCK_SIZE_H16V4,
    E_MS_UTIL_BLOCK_SIZE_H16V8,
    E_MS_UTIL_BLOCK_SIZE_H16V16,
    E_MS_UTIL_BLOCK_SIZE_MAX
}EN_MS_UTIL_BLOCK_SIZE;

MS_BOOL init_spi_flash(void);
MS_BOOL read_spi_flash(MS_U8 *pBuf, MS_U32 pos, MS_U32 length);
//MS_BOOL mark_sf_by_mstar(MS_U32 sf_dat_oft, MS_U8 *pdat, MS_U32 dat_len);
//MS_BOOL sf_match_by_mstar(void);
MS_BOOL write_raw_data(MS_U8 *pdat, MS_U32 partition_oft, MS_U32 dat_len);
MS_BOOL read_raw_data(MS_U8 *pdat, MS_U32 partition_oft, MS_U32 dat_len);
MS_BOOL get_large_memory(MS_PHY *pmem_addr, MS_U32 req_len);

MS_BOOL get_demura_env_bypass_write_fs(void);

void set_demura_bin_type(MS_U8 u8type); // EN_MS_UTIL_BIN_TYPE
MS_U8 get_demura_bin_type(void);

void set_demura_act(MS_U8 act);
MS_U8 get_demura_act(void);

void set_demura_file(MS_U8 file);
MS_U8 get_demura_file(void);

MS_BOOL get_demura_dlg_is_enable(void);
void set_demura_block_size(MS_U8 file);
MS_U8 get_demura_block_size(void);
#endif
