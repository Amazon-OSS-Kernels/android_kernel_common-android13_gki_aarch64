/* SPDX-License-Identifier: GPL-2.0-only OR BSD-3-Clause */
/******************************************************************************
 *
 * This file is provided under a dual license.  When you use or
 * distribute this software, you may choose to be licensed under
 * version 2 of the GNU General Public License ("GPLv2 License")
 * or BSD License.
 *
 * GPLv2 License
 *
 * Copyright(C) 2019 MediaTek Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See http://www.gnu.org/licenses/gpl-2.0.html for more details.
 *
 * BSD LICENSE
 *
 * Copyright(C) 2019 MediaTek Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *  * Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *****************************************************************************/
#include "eMMC.h"
#include <malloc.h>

#if defined (UNIFIED_eMMC_DRIVER) && UNIFIED_eMMC_DRIVER

#define DDR_TEST_BLK_CNT        8
#define DDR_TEST_BUFFER_SIZE    (EMMC_SECTOR_512BYTE * DDR_TEST_BLK_CNT)

#define EMMC_PATTERN_00000000   0x00000000 // smooth
#define EMMC_PATTERN_FFFFFFFF   0xFFFFFFFF // smooth
#define EMMC_PATTERN_000000FF   0x000000FF // board
#define EMMC_PATTERN_0000FFFF   0x0000FFFF // board
#define EMMC_PATTERN_00FF00FF   0x00FF00FF // board
#define EMMC_PATTERN_AA55AA55   0xAA55AA55 // board
#define EMMC_PATTERN_F00FF00F   0xF00FF00F
#define EMMC_PATTERN_FF00FF00   0xFF00FF00
#define EMMC_PATTERN_5AA55AA5   0x5AA55AA5
#define EMMC_PATTERN_55AA55AA   0x55AA55AA
#define EMMC_PATTERN_5A5A5A5A   0x5A5A5A5A
#define EMMC_PATTERN_A55AA55A   0xA55AA55A
#define EMMC_PATTERN_5AA55AA5   0x5AA55AA5
#define EMMC_PATTERN_00112233   0x00112233
#define EMMC_PATTERN_00332211   0x00332211
#define EMMC_PATTERN_44444444   0x44444444
#define EMMC_PATTERN_55555555   0x55555555
#define EMMC_PATTERN_AAAAAAAA   0xAAAAAAAA

#define FCIE_BYPASS_VERIFY              1
#define FCIE_BYPASS_RIU_VERIFY          1
#define FCIE_SDR_VERIFY                 1
#define FCIE_DDR_VERIFY                 1
#define FCIE_HS200_VERIFY               1
#define FCIE_HS400_VERIFY               1
#define FCIE_BIST_TEST                  0
#define FCIE_POWER_SAVING_MODE_TEST     0
#define FCIE_RSP_SHIFT_TUNING_TEST      0
#define FCIE_WCRC_SHIFT_TUNING_TEST     0
#define FCIE_RSTOP_SHIFT_TUNING_TEST    0
#define FCIE_READ_CRC_ERROR_INT_TEST    0
#define FCIE_WRITE_CRC_ERROR_INT_TEST   0
#define FCIE_WRITE_TIMEOUT_INT_TEST     0
#define FCIE_READ_TIMEOUT_INT_TEST      0
#define FCIE_CMD_NO_RESPONSE_INT_TEST   0
#define FCIE_CMD_RSP_CRC_ERROR_INT_TEST 0
#define FCIE_R3_R4_RESPONSE_TEST        0
#define FCIE_CHECK_RESET_PIN            0
#define FCIE_LAST_BLOCK_CRC_TEST        0
#define DMA_AES_TEST                    0

static U8 GLOBAL_WBUF_DDR[DDR_TEST_BUFFER_SIZE] EMMC_ALIGN1;
static U8 GLOBAL_RBUF_DDR[DDR_TEST_BUFFER_SIZE] EMMC_ALIGN1;

U8 u8_crazy_pattern[] = {
    0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF,
    0x01, 0xFE, 0x01, 0xFE, 0x01, 0xFE, 0x01, 0xFE,
    0x02, 0xFD, 0x02, 0xFD, 0x02, 0xFD, 0x02, 0xFD,
    0x04, 0xFB, 0x04, 0xFB, 0x04, 0xFB, 0x04, 0xFB,
    0x08, 0xF7, 0x08, 0xF7, 0x08, 0xF7, 0x08, 0xF7,
    0x10, 0xEF, 0x10, 0xEF, 0x10, 0xEF, 0x10, 0xEF,
    0x20, 0xDF, 0x20, 0xDF, 0x20, 0xDF, 0x20, 0xDF,
    0x40, 0xBF, 0x40, 0xBF, 0x40, 0xBF, 0x40, 0xBF,
    0x80, 0x7F, 0x80, 0x7F, 0x80, 0x7F, 0x80, 0x7F,
    0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00,
    0xFE, 0x01, 0xFE, 0x01, 0xFE, 0x01, 0xFE, 0x01,
    0xFD, 0x02, 0xFD, 0x02, 0xFD, 0x02, 0xFD, 0x02,
    0xFB, 0x04, 0xFB, 0x04, 0xFB, 0x04, 0xFB, 0x04,
    0xF7, 0x08, 0xF7, 0x08, 0xF7, 0x08, 0xF7, 0x08,
    0xEF, 0x10, 0xEF, 0x10, 0xEF, 0x10, 0xEF, 0x10,
    0xDF, 0x20, 0xDF, 0x20, 0xDF, 0x20, 0xDF, 0x20,
    0xBF, 0x40, 0xBF, 0x40, 0xBF, 0x40, 0xBF, 0x40,
    0x7F, 0x80, 0x7F, 0x80, 0x7F, 0x80, 0x7F, 0x80
};

U32 emmc_test_blk_wrc_ex(U32 u32_emmc_addr, U32 u32_blk_cnt)
{
	U32 u32_err = EMMC_ST_SUCCESS;
	U32 u32_buf_byte_cnt;

	u32_buf_byte_cnt = u32_blk_cnt << EMMC_SECTOR_512BYTE_BITS;

	if (emmc_drv.u8_emmc_pll_skew4 && emmc_drv.pad_type == FCIE_EMMC_HS400)
		u32_err = emmc_cmd24_skew4_miu(u32_emmc_addr, GLOBAL_WBUF_DDR);
	else
		u32_err = emmc_cmd24_miu(u32_emmc_addr, GLOBAL_WBUF_DDR);

	if (u32_err != EMMC_ST_SUCCESS)
		goto LABEL_END;

	if (emmc_drv.u8_emmc_pll_skew4 && emmc_drv.pad_type == FCIE_EMMC_HS400)
		u32_err = emmc_cmd17_skew4_miu(u32_emmc_addr, GLOBAL_RBUF_DDR);
	else
		u32_err = emmc_cmd17_miu(u32_emmc_addr, GLOBAL_RBUF_DDR);

	if (u32_err != EMMC_ST_SUCCESS)
		goto LABEL_END;

	u32_err = emmc_compare_data(GLOBAL_WBUF_DDR, GLOBAL_RBUF_DDR, EMMC_SECTOR_512BYTE);
	if (u32_err != EMMC_ST_SUCCESS) {
		emmc_debug(0, 1, "Err, compare fail.single, %Xh\n", u32_err);
		goto LABEL_END;
	}

	if (emmc_drv.u8_emmc_pll_skew4 && emmc_drv.pad_type == FCIE_EMMC_HS400)
		u32_err = emmc_cmd25_skew4_miu(u32_emmc_addr, GLOBAL_WBUF_DDR, u32_blk_cnt);
	else
		u32_err = emmc_cmd25_miu(u32_emmc_addr, GLOBAL_WBUF_DDR, u32_blk_cnt);

	if (u32_err != EMMC_ST_SUCCESS)
		goto LABEL_END;

	if (emmc_drv.u8_emmc_pll_skew4 && emmc_drv.pad_type == FCIE_EMMC_HS400)
		u32_err = emmc_cmd18_skew4_miu(u32_emmc_addr, GLOBAL_RBUF_DDR, u32_blk_cnt);
	else
		u32_err = emmc_cmd18_miu(u32_emmc_addr, GLOBAL_RBUF_DDR, u32_blk_cnt);

	if (u32_err != EMMC_ST_SUCCESS)
		goto LABEL_END;

	u32_err = emmc_compare_data(GLOBAL_WBUF_DDR, GLOBAL_RBUF_DDR, u32_buf_byte_cnt);

	if (u32_err != EMMC_ST_SUCCESS)
		emmc_debug(0, 1, "Err, compare fail.multi %Xh\n", u32_err);

LABEL_END:

	return u32_err;
}

U32 emmc_test_blk_wrc_probe_timing(U32 u32_emmc_addr)
{
	U32 u32_err = EMMC_ST_SUCCESS;
	U32 u32_i, u32_j, *pu32_w = (U32 *)GLOBAL_WBUF_DDR, *pu32_r = (U32 *)GLOBAL_RBUF_DDR;
	U32 u32_blk_cnt = EMMC_TEST_BLK_CNT, u32_buf_byte_cnt;

	emmc_reset_watch_dog();

	u32_blk_cnt = u32_blk_cnt > DDR_TEST_BLK_CNT ? DDR_TEST_BLK_CNT : u32_blk_cnt;
	u32_buf_byte_cnt = u32_blk_cnt << EMMC_SECTOR_512BYTE_BITS;

	for (u32_i = 0; u32_i < u32_buf_byte_cnt >> 2; u32_i++)
		pu32_r[u32_i] = 0;

	for (u32_j = 0; u32_j < 10; u32_j++) {
		//init data pattern
		switch (u32_j) {
		case 0://increase
			for (u32_i = 0; u32_i < u32_buf_byte_cnt; u32_i++)
				GLOBAL_WBUF_DDR[u32_i] = (U8)(u32_i & 0xFF);
		break;
		case 1://decrease
			for (u32_i = 0; u32_i < u32_buf_byte_cnt; u32_i++)
				GLOBAL_WBUF_DDR[u32_i] = 0xFF - ((U8)(u32_i & 0xFF));
		break;
		case 2://0xF00F
			for (u32_i = 0; u32_i < u32_buf_byte_cnt >> 2; u32_i++)
				pu32_w[u32_i] = EMMC_PATTERN_F00FF00F;
		break;
		case 3://0xFF00
			for (u32_i = 0; u32_i < u32_buf_byte_cnt >> 2; u32_i++)
				pu32_w[u32_i] = EMMC_PATTERN_FF00FF00;
		break;
		case 4://0x5AA5
			for (u32_i = 0; u32_i < u32_buf_byte_cnt >> 2; u32_i++)
				pu32_w[u32_i] = EMMC_PATTERN_5AA55AA5;
		break;
		case 5://0x55AA
		continue;
		case 6://0x5A5A
		continue;
		case 7://0x0000
			for (u32_i = 0; u32_i < u32_buf_byte_cnt >> 2; u32_i++)
				pu32_w[u32_i] = EMMC_PATTERN_00000000;
		break;
		case 8://0xFFFF
		continue;
		case 9:
		{
			U16 u8_pat_size = sizeof(u8_crazy_pattern);
			U16 u8_rep = u32_buf_byte_cnt / u8_pat_size;
			U16 u8_tail = u32_buf_byte_cnt % u8_pat_size;

			for (u32_i = 0; u32_i < u8_rep; u32_i++)
				memcpy(GLOBAL_WBUF_DDR + (u32_i * u8_pat_size), u8_crazy_pattern, u8_pat_size);

			if (u8_tail != 0)
				memcpy(GLOBAL_WBUF_DDR + (u32_i * u8_pat_size), u8_crazy_pattern, u8_tail);
		break;
		}
		}
		u32_err = emmc_test_blk_wrc_ex(u32_emmc_addr, u32_blk_cnt);
		if (u32_err != EMMC_ST_SUCCESS)
			break;
	}

	if (u32_err != EMMC_ST_SUCCESS) {
		emmc_debug(1, 1, "data pattern %u: %02X%02X%02X%02Xh\n\n",
			   u32_j,
			   GLOBAL_WBUF_DDR[3],
			   GLOBAL_WBUF_DDR[2],
			   GLOBAL_WBUF_DDR[1],
			   GLOBAL_WBUF_DDR[0]);
	}

	return u32_err;
}


U32 emmc_stored_crazy_pattern(U32 u32_emmc_addr)
{
	U32 u32_err = EMMC_ST_SUCCESS;
	U32 u32_i;
	U16 u8_pat_size = sizeof(u8_crazy_pattern);
	U16 u8_rep = EMMC_SECTOR_512BYTE / u8_pat_size;
	U16 u8_tail = EMMC_SECTOR_512BYTE % u8_pat_size;


	for (u32_i = 0; u32_i < u8_rep; u32_i++)
		memcpy(GLOBAL_WBUF_DDR + (u32_i * u8_pat_size), u8_crazy_pattern, u8_pat_size);

	if (u8_tail != 0)
		memcpy(GLOBAL_WBUF_DDR + (u32_i * u8_pat_size), u8_crazy_pattern, u8_tail);

	u32_err = emmc_cmd24_miu(u32_emmc_addr, GLOBAL_WBUF_DDR);

	if (u32_err != EMMC_ST_SUCCESS)
		emmc_debug(0, 1, "eMMC Err: fail %Xh\n", u32_err);

	return u32_err;
}

U32 emmc_read_crazy_pattern(U32 u32_emmc_addr)
{
	U32 u32_err = EMMC_ST_SUCCESS;

	if (emmc_drv.pad_type == FCIE_EMMC_HS400)
		u32_err = emmc_cmd17_skew4_miu(u32_emmc_addr, GLOBAL_RBUF_DDR);
	else
		u32_err = emmc_cmd17_miu(u32_emmc_addr, GLOBAL_RBUF_DDR);

	if (u32_err != EMMC_ST_SUCCESS)
		emmc_debug(0, 1, "eMMC Err: fail %Xh\n", u32_err);

	return u32_err;
}

//=============================================================

//=============================================================
#if defined(IF_IP_VERIFY) && IF_IP_VERIFY

#define TEST_BUFFER_SIZE        0x100000 // 1MB
#define TEST_START_ADDR         0xA00000 // 10MB

U8 *gau8_wbuf = 0, *gau8_rbuf = 0;

#define MAX_SCATTERLIST_COUNT 0x10
#define EMMC_GENERIC_WAIT_TIME  (HW_TIMER_DELAY_1s * 2)//2 sec

static struct scatterlist
{
	U32 u32_length;
	unsigned long u32_dma_address;
} p_sg_st[MAX_SCATTERLIST_COUNT];

struct adma_descriptor p_adma_desc_st[MAX_SCATTERLIST_COUNT] EMMC_ALIGN1;

U32 emmc_test_single_blk_wrc_riu(U32 u32_emmc_addr, U32 u32_data_pattern)
{
	U32 u32_err;
	U32 u32_i, *pu32_w = (U32 *)gau8_wbuf, *pu32_r = (U32 *)gau8_rbuf;

	if (u32_data_pattern == EMMC_PATTERN_00112233 || u32_data_pattern == EMMC_PATTERN_00332211) {
		for (u32_i = 0; u32_i < TEST_BUFFER_SIZE; u32_i++) {
			if (u32_data_pattern == EMMC_PATTERN_00112233)
				gau8_wbuf[u32_i] = u32_i & 0xFF;
			else
				gau8_wbuf[u32_i] = 0xFF - (u32_i & 0xFF);

			gau8_rbuf[u32_i] = ~gau8_wbuf[u32_i];
		}
	} else {
		for (u32_i = 0; u32_i < TEST_BUFFER_SIZE >> 2; u32_i++) {
			pu32_w[u32_i] = u32_emmc_addr;
			pu32_r[u32_i] = ~pu32_w[u32_i];
		}
	}

	u32_err = emmc_cmd24_cifd(u32_emmc_addr, gau8_wbuf);
	if (u32_err != EMMC_ST_SUCCESS) {
		emmc_debug(0, 1, "Err, SingleBlkWRC W, %Xh\n", u32_err);
		return u32_err;
	}

	u32_err = emmc_cmd17_cifd(u32_emmc_addr, gau8_rbuf);
	if (u32_err != EMMC_ST_SUCCESS) {
		emmc_debug(0, 1, "Err, SingleBlkWRC R, %Xh\n", u32_err);
		return u32_err;
	}

	u32_err = emmc_compare_data(gau8_wbuf, gau8_rbuf, FCIE_CIFD_BYTE_CNT);
	if (u32_err != EMMC_ST_SUCCESS) {
		emmc_debug(0, 1, "Err, SingleBlkWRC C, %Xh\n", u32_err);
		return u32_err;
	}

	return u32_err;
}

U32 emmc_test_single_blk_wrc_miu(U32 u32_emmc_addr, U32 u32_data_pattern)
{
	U32 u32_i, *pu32_w = (U32 *)gau8_wbuf, *pu32_r = (U32 *)gau8_rbuf;

	if (u32_data_pattern == EMMC_PATTERN_00112233 || u32_data_pattern == EMMC_PATTERN_00332211) {
		for (u32_i = 0; u32_i < TEST_BUFFER_SIZE; u32_i++) {
			if (u32_data_pattern == EMMC_PATTERN_00112233)
				gau8_wbuf[u32_i] = u32_i & 0xFF;
			else
				gau8_wbuf[u32_i] = 0xFF - (u32_i & 0xFF);

			gau8_rbuf[u32_i] = ~gau8_wbuf[u32_i];
		}
	} else {
		for (u32_i = 0; u32_i < TEST_BUFFER_SIZE >> 2; u32_i++) {
			pu32_w[u32_i] = u32_data_pattern;
			pu32_r[u32_i] = ~pu32_w[u32_i];
		}
	}

	return emmc_test_single_blk_wrc_miu_ex(u32_emmc_addr, (U8 *)pu32_w, (U8 *)pu32_r);
}

U32 emmc_test_single_blk_wrc_miu_ex(U32 u32_emmc_addr, U8 *pu8_w, U8 *pu8_r)
{
	U32 u32_err = EMMC_ST_SUCCESS;

	u32_err = emmc_cmd24_miu(u32_emmc_addr, pu8_w);
	if (u32_err != EMMC_ST_SUCCESS) {
		emmc_debug(0, 1, "Err, SingleBlkWRC W, %Xh\n", u32_err);
		return u32_err;
	}

	u32_err = emmc_cmd17_miu(u32_emmc_addr, pu8_r);
	if (u32_err != EMMC_ST_SUCCESS) {
		emmc_debug(0, 1, "Err, SingleBlkWRC R, %Xh\n", u32_err);
		return u32_err;
	}

	u32_err = emmc_compare_data(pu8_w, pu8_r, EMMC_SECTOR_BYTECNT);
	if (u32_err != EMMC_ST_SUCCESS) {
		emmc_debug(0, 1, "Err, SingleBlkWRC C, %Xh\n", u32_err);
		return u32_err;
	}

	return u32_err;
}

U32 emmc_test_single_blk_wrc_miu_aes_ex(U32 u32_emmc_addr, U8 *pu8_w, U8 *pu8_r)
{
	U32 u32_err = EMMC_ST_SUCCESS;

	//AES config
	fde_aes_sw_reset();
	fde_aes_set_xts_lba(u32_emmc_addr);
	fde_aes_set_codec_dir(ENABLE_ENC);
	fde_aes_set_xex_hw_t_calc_kick(1);
	fde_aes_open();

	u32_err = emmc_cmd24_miu(u32_emmc_addr, pu8_w);

	fde_aes_set_xex_hw_t_calc_kick(0);
	fde_aes_close();

	if (u32_err != EMMC_ST_SUCCESS) {
		emmc_debug(0, 1, "Err, SingleBlkWRC W, %Xh\n", u32_err);
		return u32_err;
	}

	#if 1
	u32_err = emmc_cmd17_miu(u32_emmc_addr, pu8_r);
	if (u32_err != EMMC_ST_SUCCESS) {
		emmc_debug(0, 1, "Err, SingleBlkWRC R, %Xh\n", u32_err);
		return u32_err;
	}

	u32_err = emmc_compare_data(pu8_w, pu8_r, EMMC_SECTOR_BYTECNT);
	if (u32_err != EMMC_ST_SUCCESS)
		emmc_debug(0, 1, "Expect, SingleBlkWRC C, %Xh\n", u32_err);
	#endif

	fde_aes_sw_reset();
	fde_aes_set_xts_lba(u32_emmc_addr);
	fde_aes_set_codec_dir(ENABLE_DEC);
	fde_aes_set_xex_hw_t_calc_kick(1);
	fde_aes_open();

	u32_err = emmc_cmd17_miu(u32_emmc_addr, pu8_r);

	fde_aes_set_xex_hw_t_calc_kick(0);
	fde_aes_close();
	if (u32_err != EMMC_ST_SUCCESS) {
		emmc_debug(0, 1, "Err, SingleBlkWRC R, %Xh\n", u32_err);
		return u32_err;
	}

	u32_err = emmc_compare_data(pu8_w, pu8_r, EMMC_SECTOR_BYTECNT);
	if (u32_err != EMMC_ST_SUCCESS) {
		emmc_debug(0, 1, "Err, SingleBlkWRC C, %Xh\n", u32_err);
		return u32_err;
	}

	return u32_err;
}

U32 emmc_test_single_blk_wrc_miu_aes(U32 u32_emmc_addr, U32 u32_data_pattern)
{
	U32 u32_i, *pu32_w = (U32 *)gau8_wbuf, *pu32_r = (U32 *)gau8_rbuf;

	//init data pattern
	if (u32_data_pattern == EMMC_PATTERN_00112233) {
		for (u32_i = 0; u32_i < EMMC_SECTOR_BYTECNT >> 2; u32_i++) {
			pu32_w[u32_i] = u32_i % 0x80;
			pu32_r[u32_i] = ~pu32_w[u32_i];
		}
	} else if (u32_data_pattern == EMMC_PATTERN_00332211) {
		for (u32_i = 0; u32_i < EMMC_SECTOR_BYTECNT >> 2; u32_i++) {
			pu32_w[u32_i] = EMMC_PATTERN_00332211 + EMMC_PATTERN_44444444 * u32_i;
			pu32_r[u32_i] = ~pu32_w[u32_i];
		}
	} else {
		for (u32_i = 0; u32_i < EMMC_SECTOR_BYTECNT >> 2; u32_i++) {
			pu32_w[u32_i] = u32_data_pattern;
			pu32_r[u32_i] = ~pu32_w[u32_i];
		}
	}
	//emmc_debug(0, 1, "W pattern\n");
	//emmc_dump_mem(pu32_w, 16);

	return emmc_test_single_blk_wrc_miu_aes_ex(u32_emmc_addr, (U8 *)pu32_w, (U8 *)pu32_r);
}

U32 emmc_test_multi_blk_wrc_miu_aes(U32 u32_emmc_addr, U16 u16_blk_cnt, U32 u32_data_pattern)
{
	U32 u32_err;
	U32 u32_i, *pu32_w = (U32 *)gau8_wbuf, *pu32_r = (U32 *)gau8_rbuf;
	U16 u16_blk_cnt_tmp;

	//init data pattern
	if (u32_data_pattern == EMMC_PATTERN_00112233) {
		for (u32_i = 0; u32_i < TEST_BUFFER_SIZE >> 2; u32_i++) {
			pu32_w[u32_i] = u32_i % 0x80;
			pu32_r[u32_i] = ~pu32_w[u32_i];
		}
	} else if (u32_data_pattern == EMMC_PATTERN_00332211) {
		for (u32_i = 0; u32_i < TEST_BUFFER_SIZE >> 2; u32_i++) {
			pu32_w[u32_i] = u32_i;
			pu32_r[u32_i] = ~pu32_w[u32_i];
		}
	} else {
		for (u32_i = 0; u32_i < TEST_BUFFER_SIZE >> 2; u32_i++) {
			pu32_w[u32_i] = u32_data_pattern;
			pu32_r[u32_i] = ~pu32_w[u32_i];
		}
	}

	u16_blk_cnt_tmp = TEST_BUFFER_SIZE >> EMMC_SECTOR_512BYTE_BITS;
	u16_blk_cnt_tmp = u16_blk_cnt_tmp > u16_blk_cnt ? u16_blk_cnt : u16_blk_cnt_tmp;

	for (u32_i = 0; u32_i < u16_blk_cnt_tmp; u32_i++) {
		fde_aes_open();
		fde_aes_set_xts_lba(u32_emmc_addr + u32_i);
		fde_aes_set_codec_dir(ENABLE_ENC);
		fde_aes_set_xex_hw_t_calc_kick(1);
		u32_err = emmc_cmd25_miu(u32_emmc_addr + u32_i,
					 gau8_wbuf + (u32_i << EMMC_SECTOR_512BYTE_BITS), u16_blk_cnt_tmp - u32_i);

		fde_aes_set_xex_hw_t_calc_kick(0);
		fde_aes_close();
		if (u32_err != EMMC_ST_SUCCESS) {
			emmc_debug(0, 1, "Err, MultiBlkWRC W, %u, %Xh\n", u32_i, u32_err);
			goto LABEL_OF_ERROR;
		}
		fde_aes_open();
		fde_aes_set_xts_lba(u32_emmc_addr + u32_i);
		fde_aes_set_codec_dir(ENABLE_DEC);
		fde_aes_set_xex_hw_t_calc_kick(1);

		u32_err = emmc_cmd18_miu(u32_emmc_addr + u32_i,
					 gau8_rbuf + (u32_i << EMMC_SECTOR_512BYTE_BITS), u16_blk_cnt_tmp - u32_i);
		fde_aes_set_xex_hw_t_calc_kick(0);
		fde_aes_close();
		if (u32_err != EMMC_ST_SUCCESS) {
			emmc_debug(0, 1, "Err, MultiBlkWRC R, %u, %Xh\n", u32_i, u32_err);
			goto LABEL_OF_ERROR;
		}

		u32_err = emmc_compare_data(gau8_wbuf + (u32_i << EMMC_SECTOR_512BYTE_BITS),
					    gau8_rbuf + (u32_i << EMMC_SECTOR_512BYTE_BITS), (u16_blk_cnt_tmp - u32_i) << 9);

		//#if 0
		//u32_err = emmc_compare_data(gau8_wbuf + 512 + (u32_i << EMMC_SECTOR_512BYTE_BITS),
					    //gau8_rbuf + 512 + (u32_i << EMMC_SECTOR_512BYTE_BITS),
					    //(u16_blk_cnt_tmp - u32_i) << 9);
		//#endif
		if (u32_err != EMMC_ST_SUCCESS) {
			emmc_debug(0, 1, "Err, MultiBlkWRC C, %u, %Xh\n", u32_i, u32_err);
			goto LABEL_OF_ERROR;
		}
		u32_i += u16_blk_cnt_tmp - u32_i;
	}

	return u32_err;

LABEL_OF_ERROR:
	return u32_err;
}

U32 emmc_test_single_w_multi_r_blk_wrc_miu_aes(U32 u32_emmc_addr, U16 u16_blk_cnt, U32 u32_data_pattern)
{
	U32 u32_err;
	U32 u32_i, u32_j, *pu32_w = (U32 *)gau8_wbuf, *pu32_r = (U32 *)gau8_rbuf;
	U16 u16_blk_cnt_tmp;

	//init data pattern
	if (u32_data_pattern == EMMC_PATTERN_00112233) {
		for (u32_i = 0; u32_i < TEST_BUFFER_SIZE >> 2; u32_i++) {
			pu32_w[u32_i] = u32_i % 0x80;
			pu32_r[u32_i] = ~pu32_w[u32_i];
		}
	} else if (u32_data_pattern == EMMC_PATTERN_00332211) {
		for (u32_i = 0; u32_i < TEST_BUFFER_SIZE >> 2; u32_i++) {
			pu32_w[u32_i] = u32_i;
			pu32_r[u32_i] = ~pu32_w[u32_i];
		}
	} else {
		for (u32_i = 0; u32_i < TEST_BUFFER_SIZE >> 2; u32_i++) {
			pu32_w[u32_i] = u32_data_pattern;
			pu32_r[u32_i] = ~pu32_w[u32_i];
		}
	}

	u16_blk_cnt_tmp = TEST_BUFFER_SIZE >> EMMC_SECTOR_512BYTE_BITS;
	u16_blk_cnt_tmp = u16_blk_cnt_tmp > u16_blk_cnt ? u16_blk_cnt : u16_blk_cnt_tmp;

	for (u32_i = 0; u32_i < u16_blk_cnt_tmp; u32_i++) {
		for (u32_j = 0; u32_j < u16_blk_cnt_tmp - u32_i; u32_j++) {
			fde_aes_open();
			fde_aes_set_xts_lba(u32_emmc_addr + u32_i + u32_j);
			fde_aes_set_codec_dir(ENABLE_ENC);
			fde_aes_set_xex_hw_t_calc_kick(1);
			u32_err = emmc_cmd24_miu(u32_emmc_addr + u32_i + u32_j,
						 gau8_wbuf + ((u32_i + u32_j) << 9));

			fde_aes_set_xex_hw_t_calc_kick(0);
			fde_aes_close();
			if (u32_err != EMMC_ST_SUCCESS) {
				emmc_debug(0, 1, "Err, Single W, %u, %Xh\n", u32_i, u32_err);
				goto LABEL_OF_ERROR;
			}
		}
		fde_aes_open();
		fde_aes_set_xts_lba(u32_emmc_addr + u32_i);
		fde_aes_set_codec_dir(ENABLE_DEC);
		fde_aes_set_xex_hw_t_calc_kick(1);

		u32_err = emmc_cmd18_miu(u32_emmc_addr + u32_i,
					 gau8_rbuf + (u32_i << 9), u16_blk_cnt_tmp - u32_i);
		fde_aes_set_xex_hw_t_calc_kick(0);
		fde_aes_close();
		if (u32_err != EMMC_ST_SUCCESS) {
			emmc_debug(0, 1, "Err, MultiBlkWRC R, %u, %Xh\n", u32_i, u32_err);
			goto LABEL_OF_ERROR;
		}

		u32_err = emmc_compare_data(gau8_wbuf + (u32_i << EMMC_SECTOR_512BYTE_BITS),
					    gau8_rbuf + (u32_i << EMMC_SECTOR_512BYTE_BITS),
					    (u16_blk_cnt_tmp - u32_i) << EMMC_SECTOR_512BYTE_BITS);

		if (u32_err != EMMC_ST_SUCCESS) {
			emmc_debug(0, 1, "Err, MultiBlkWRC C, %u, %Xh\n", u32_i, u32_err);
			goto LABEL_OF_ERROR;
		}
		u32_i += u16_blk_cnt_tmp - u32_i;
	}

	return u32_err;

LABEL_OF_ERROR:
	return u32_err;
}

U32 emmc_test_multi_w_single_r_blk_wrc_miu_aes(U32 u32_emmc_addr, U16 u16_blk_cnt, U32 u32_data_pattern)
{
	U32 u32_err;
	U32 u32_i, u32_j, *pu32_w = (U32 *)gau8_wbuf, *pu32_r = (U32 *)gau8_rbuf;
	U16 u16_blk_cnt_tmp;

	//init data pattern
	if (u32_data_pattern == EMMC_PATTERN_00112233) {
		for (u32_i = 0; u32_i < TEST_BUFFER_SIZE >> 2; u32_i++) {
			pu32_w[u32_i] = u32_i % 0x80;
			pu32_r[u32_i] = ~pu32_w[u32_i];
		}
	} else if (u32_data_pattern == EMMC_PATTERN_00332211) {
		for (u32_i = 0; u32_i < TEST_BUFFER_SIZE >> 2; u32_i++) {
			pu32_w[u32_i] = u32_i;
			pu32_r[u32_i] = ~pu32_w[u32_i];
		}
	} else {
		for (u32_i = 0; u32_i < TEST_BUFFER_SIZE >> 2; u32_i++) {
			pu32_w[u32_i] = u32_data_pattern;
			pu32_r[u32_i] = ~pu32_w[u32_i];
		}
	}

	u16_blk_cnt_tmp = TEST_BUFFER_SIZE >> EMMC_SECTOR_512BYTE_BITS;
	u16_blk_cnt_tmp = u16_blk_cnt_tmp > u16_blk_cnt ? u16_blk_cnt : u16_blk_cnt_tmp;

	for (u32_i = 0; u32_i < u16_blk_cnt_tmp; u32_i++) {
		fde_aes_open();
		fde_aes_set_xts_lba(u32_emmc_addr + u32_i);
		fde_aes_set_codec_dir(ENABLE_ENC);
		fde_aes_set_xex_hw_t_calc_kick(1);
		u32_err = emmc_cmd25_miu(u32_emmc_addr + u32_i,
					 gau8_wbuf + (u32_i << EMMC_SECTOR_512BYTE_BITS), u16_blk_cnt_tmp - u32_i);

		fde_aes_set_xex_hw_t_calc_kick(0);
		fde_aes_close();
		if (u32_err != EMMC_ST_SUCCESS) {
			emmc_debug(0, 1, "Err, MultiBlkWRC W, %u, %Xh\n", u32_i, u32_err);
			goto LABEL_OF_ERROR;
		}

		for (u32_j = 0; u32_j < u16_blk_cnt_tmp - u32_i; u32_j++) {
			fde_aes_open();
			fde_aes_set_xts_lba(u32_emmc_addr + u32_i + u32_j);
			fde_aes_set_codec_dir(ENABLE_DEC);
			fde_aes_set_xex_hw_t_calc_kick(1);

			u32_err = emmc_cmd17_miu(u32_emmc_addr + u32_i + u32_j,
						 gau8_rbuf + ((u32_i + u32_j) << EMMC_SECTOR_512BYTE_BITS));
			fde_aes_set_xex_hw_t_calc_kick(0);
			fde_aes_close();
			if (u32_err != EMMC_ST_SUCCESS) {
				emmc_debug(0, 1, "Err, Single R, %u, %Xh\n", u32_i, u32_err);
				goto LABEL_OF_ERROR;
			}
		}
		u32_err = emmc_compare_data(gau8_wbuf + (u32_i << EMMC_SECTOR_512BYTE_BITS),
					    gau8_rbuf + (u32_i << EMMC_SECTOR_512BYTE_BITS),
					    (u16_blk_cnt_tmp - u32_i) << EMMC_SECTOR_512BYTE_BITS);

		//#if 0
		//u32_err = emmc_compare_data(gau8_wbuf + 512 + (u32_i << EMMC_SECTOR_512BYTE_BITS),
					    //gau8_rbuf + 512 + (u32_i << EMMC_SECTOR_512BYTE_BITS),
					    //(u16_blk_cnt_tmp - u32_i) << EMMC_SECTOR_512BYTE_BITS);
		//#endif
		if (u32_err != EMMC_ST_SUCCESS) {
			emmc_debug(0, 1, "Err, MultiBlkWRC C, %u, %Xh\n", u32_i, u32_err);
			goto LABEL_OF_ERROR;
		}

		u32_i += u16_blk_cnt_tmp - u32_i;
	}

	return u32_err;

LABEL_OF_ERROR:
	return u32_err;
}

static U32 emmc_test_sg_wrc_miu_ase_adma_write(U32 u32_emmc_addr)
{
	U32 u32_i, u32_err = EMMC_ST_SUCCESS;
	U32 u32_dmalen;
	unsigned long dma_addr, dmaaddr;

	memset(p_adma_desc_st, 0, MAX_SCATTERLIST_COUNT *  sizeof(struct adma_descriptor));

	for (u32_i = 0; u32_i < MAX_SCATTERLIST_COUNT; u32_i++) {
		if (p_sg_st[u32_i].u32_dma_address && p_sg_st[u32_i].u32_length) {
			dmaaddr = p_sg_st[u32_i].u32_dma_address;
			u32_dmalen = p_sg_st[u32_i].u32_length;
			dma_addr = emmc_translate_dma_address_ex((unsigned long)dmaaddr, u32_dmalen);

			//parsing dma address, length to descriptor
			p_adma_desc_st[u32_i].u32_address = (dma_addr);
			if (sizeof(unsigned long) == 8)
				p_adma_desc_st[u32_i].u32_address2 = (dma_addr >> BIT_MIU_DMA_ADDR_47_32_SHIFT);

			p_adma_desc_st[u32_i].u32_dma_len = (u32_dmalen);
			p_adma_desc_st[u32_i].u32_job_cnt = (u32_dmalen >> EMMC_SECTOR_512BYTE_BITS);
			p_adma_desc_st[u32_i].u32_miu_sel = 0;//FPGA Mode always miu0
		} else {
			break;
		}
	}

	if (u32_i > 0)
		p_adma_desc_st[u32_i - 1].u32_end = 1;
	else
		p_adma_desc_st[0].u32_end = 1;

	//emmc_dump_mem((U8 *)p_adma_desc_st, u32_i * sizeof(struct adma_descriptor));

	fde_aes_open();
	fde_aes_set_xts_lba(u32_emmc_addr);
	fde_aes_set_codec_dir(ENABLE_ENC);
	fde_aes_set_xex_hw_t_calc_kick(1);
	u32_err = emmc_cmd25_adma(u32_emmc_addr, (U8 *)p_adma_desc_st, sizeof(struct adma_descriptor) * u32_i);
	fde_aes_set_xex_hw_t_calc_kick(0);
	fde_aes_close();

	if (u32_err != EMMC_ST_SUCCESS)
		emmc_debug(0, 1, "Err, SgWRC_MIU W, %Xh\n", u32_err);

	return u32_err;
}

static U32 emmc_test_sg_wrc_miu_ase_adma_read(U32 u32_emmc_addr)
{
	U32 u32_i, u32_err = EMMC_ST_SUCCESS;
	U32 u32_dmalen;
	unsigned long dma_addr;

	memset(p_adma_desc_st, 0, MAX_SCATTERLIST_COUNT *  sizeof(struct adma_descriptor));

	u32_i = 0;
	for (u32_i = 0; u32_i < MAX_SCATTERLIST_COUNT; u32_i++) {
		if (p_sg_st[u32_i].u32_dma_address && p_sg_st[u32_i].u32_length) {
			dma_addr = p_sg_st[u32_i].u32_dma_address;
			u32_dmalen = p_sg_st[u32_i].u32_length;
			dma_addr = emmc_translate_dma_address_ex((unsigned long)dma_addr, u32_dmalen);

			//parsing dma address, length to descriptor
			p_adma_desc_st[u32_i].u32_address = (dma_addr);
			if (sizeof(unsigned long) == 8)
				p_adma_desc_st[u32_i].u32_address2 = (dma_addr >> BIT_MIU_DMA_ADDR_47_32_SHIFT);

			p_adma_desc_st[u32_i].u32_dma_len = (u32_dmalen);
			p_adma_desc_st[u32_i].u32_job_cnt = (u32_dmalen >> EMMC_SECTOR_512BYTE_BITS);
			p_adma_desc_st[u32_i].u32_miu_sel  = 0;
		} else {
			break;
		}
	}

	if (u32_i > 0)
		p_adma_desc_st[u32_i - 1].u32_end = 1;
	else
		p_adma_desc_st[0].u32_end = 1;

	//emmc_dump_mem((U8 *)p_adma_desc_st, u32_i *  sizeof(struct adma_descriptor));

	fde_aes_open();
	fde_aes_set_xts_lba(u32_emmc_addr);
	fde_aes_set_codec_dir(ENABLE_DEC);
	fde_aes_set_xex_hw_t_calc_kick(1);
	u32_err = emmc_cmd18_adma(u32_emmc_addr, (U8 *)p_adma_desc_st, sizeof(struct adma_descriptor) * u32_i);
	fde_aes_set_xex_hw_t_calc_kick(0);
	fde_aes_close();
	#ifndef NEED_POST_INVALIDATE_CACHE
	flush_cache((unsigned long)gau8_rbuf, TEST_BUFFER_SIZE);
	#endif

	if (u32_err != EMMC_ST_SUCCESS)
		emmc_debug(0, 1, "Err, SgWRC_MIU R, %Xh\n", u32_err);

	#if defined(NEED_POST_INVALIDATE_CACHE) && NEED_POST_INVALIDATE_CACHE
	for (u32_i = 0; u32_i < MAX_SCATTERLIST_COUNT; u32_i++) {
		if (p_sg_st[u32_i].u32_dma_address && p_sg_st[u32_i].u32_length) {
			dma_addr = p_sg_st[u32_i].u32_dma_address;
			u32_dmalen	= p_sg_st[u32_i].u32_length;
			emmc_invalidate_data_cache_buffer((unsigned long)dma_addr, u32_dmalen);
		}	else {
			break;
		}
	}
	#endif

	return u32_err;
}

U32 emmc_test_sg_wrc_miu_ase(U32 u32_emmc_addr, U16 u16_blk_cnt, U32 u32_data_pattern)
{
	U32 u32_err = EMMC_ST_SUCCESS;
	U32 u32_i, u32_j, *pu32_w = (U32 *)gau8_wbuf, *pu32_r = (U32 *)gau8_rbuf;
	U32 u32_temp = 0, u32_sg_blocks = 0;

	if (u32_data_pattern == EMMC_PATTERN_00112233 || u32_data_pattern == EMMC_PATTERN_00332211) {
		for (u32_i = 0; u32_i < TEST_BUFFER_SIZE; u32_i++) {
			if (u32_data_pattern == EMMC_PATTERN_00112233)
				gau8_wbuf[u32_i] = u32_i & 0xFF;
			else
				gau8_wbuf[u32_i] = 0xFF - (u32_i & 0xFF);
			gau8_rbuf[u32_i] = ~gau8_wbuf[u32_i];
		}
	} else {
		for (u32_i = 0; u32_i < TEST_BUFFER_SIZE >> 2; u32_i++) {
			pu32_w[u32_i] = EMMC_PATTERN_00112233;
			pu32_r[u32_i] = ~pu32_w[u32_i];
		}
	}

	u32_sg_blocks = u16_blk_cnt / MAX_SCATTERLIST_COUNT;
	u32_temp = 0;
	memset(p_sg_st, 0, MAX_SCATTERLIST_COUNT *  sizeof(struct adma_descriptor));

	//setting scatterlist dma address and dma length for write multiple block
	for (u32_i = 0; (u32_i < MAX_SCATTERLIST_COUNT / 4); u32_i++) {
		for (u32_j = 0; u32_j < 4; u32_j++) {
			p_sg_st[u32_i * 4 + u32_j].u32_dma_address = (unsigned long)(gau8_wbuf + u32_temp);
			if (u32_j < 3) {
				p_sg_st[u32_i * 4 + u32_j].u32_length = EMMC_SECTOR_512BYTE;
				u32_temp += EMMC_SECTOR_512BYTE;
			} else {
				p_sg_st[u32_i * 4 + u32_j].u32_length =
					(4 * u32_sg_blocks - 3) << EMMC_SECTOR_512BYTE_BITS;
				u32_temp += (4 * u32_sg_blocks - 3) << EMMC_SECTOR_512BYTE_BITS;
			}
		}
	}

	u32_err = emmc_test_sg_wrc_miu_ase_adma_write(u32_emmc_addr);
	if (u32_err != EMMC_ST_SUCCESS)
		return u32_err;

	u32_temp = 0;
	memset(p_sg_st, 0, MAX_SCATTERLIST_COUNT *  sizeof(struct adma_descriptor));

	//setting scatterlist dma address and dma length for read multiple block
	for (u32_i = 0; u32_i < MAX_SCATTERLIST_COUNT / 4; u32_i++) {
		for (u32_j = 0; u32_j < 4; u32_j++) {
			p_sg_st[u32_i * 4 + u32_j].u32_dma_address = (unsigned long)(gau8_rbuf + u32_temp);
			if (u32_j < 3) {
				p_sg_st[u32_i * 4 + u32_j].u32_length = EMMC_SECTOR_512BYTE;
				u32_temp += EMMC_SECTOR_512BYTE;
			} else {
				p_sg_st[u32_i * 4 + u32_j].u32_length =
					(4 * u32_sg_blocks - 3) << EMMC_SECTOR_512BYTE_BITS;
				u32_temp += (4 * u32_sg_blocks - 3) << EMMC_SECTOR_512BYTE_BITS;
			}
		}
	}

	u32_err = emmc_test_sg_wrc_miu_ase_adma_read(u32_emmc_addr);
	if (u32_err != EMMC_ST_SUCCESS)
		return u32_err;

	u32_err = emmc_compare_data(gau8_wbuf, gau8_rbuf, u16_blk_cnt << 9);

	if (u32_err != EMMC_ST_SUCCESS)
		emmc_debug(0, 1, "Err, SgWRC_MIU C, %u, %Xh\n", u32_i, u32_err);

	return u32_err;
}

U32 emmc_test_aes_simple_test(U32 u32_data_pattern)
{
	U32 u32_start_sector, u32_sector_cnt;
	U32 u32_err = 0;

	u32_start_sector = EMMC_TEST_BLK_0;
	u32_sector_cnt = EMMC_TEST_BLK_CNT;
	if (!gau8_wbuf)
		gau8_wbuf = (U8 *)memalign(EMMC_CACHE_LINE, TEST_BUFFER_SIZE);

	if (!gau8_rbuf)
		gau8_rbuf = (U8 *)memalign(EMMC_CACHE_LINE, TEST_BUFFER_SIZE);

	emmc_debug(EMMC_DEBUG_LEVEL, 1, "Data Pattern: %08Xh\n", u32_data_pattern);
	emmc_debug(0, 1, "AES single test\n");

	u32_err = emmc_test_single_blk_wrc_miu_aes(u32_start_sector, u32_data_pattern);
	if (u32_err != EMMC_ST_SUCCESS) {
		emmc_debug(0, 1, "AES single test fails %Xh\n", u32_err);
		return u32_err;
	}

	emmc_debug(0, 1, "AES multi test\n");
	u32_err = emmc_test_multi_blk_wrc_miu_aes(u32_start_sector, u32_sector_cnt, u32_data_pattern);
	if (u32_err != EMMC_ST_SUCCESS) {
		emmc_debug(0, 1, "AES multi test fails %Xh\n", u32_err);
		return u32_err;
	}

	#if 1
	emmc_debug(0, 1, "AES multi-single test\n");

	u32_err = emmc_test_multi_w_single_r_blk_wrc_miu_aes(u32_start_sector, u32_sector_cnt, u32_data_pattern);
	if (u32_err != EMMC_ST_SUCCESS) {
		emmc_debug(0, 1, "AES multi write-single read test fails %Xh\n", u32_err);
		return u32_err;
	}

	emmc_debug(0, 1, "AES single-multi test\n");
	u32_err = emmc_test_single_w_multi_r_blk_wrc_miu_aes(u32_start_sector, u32_sector_cnt, u32_data_pattern);
	if (u32_err != EMMC_ST_SUCCESS) {
		emmc_debug(0, 1, "AES singe write-multi read test fails %Xh\n", u32_err);
		return u32_err;
	}
	#endif

	emmc_debug(0, 1, "AES adma test\n");
	u32_err = emmc_test_sg_wrc_miu_ase(u32_start_sector, u32_sector_cnt, u32_data_pattern);
	if (u32_err != EMMC_ST_SUCCESS)
		emmc_debug(0, 1, "AES ADMA test fails %Xh\n", u32_err);

	return u32_err;
}

extern void fde_aes_set_key(U8 *pu8_key, U32 u32_key_size);

static U32 emmc_test_aes_all_mode_test_ex(int k,
					  U32 *test_pattern,
					  U32 *u32_data_unit_size,
					  U32 u32_start_sector,
					  U32 u32_sector_cnt)
{
	int l;
	U32 u32_err = 0;

	for (l = 0; l < 12; l++) {
		emmc_debug(EMMC_DEBUG_LEVEL, 1, "Data Pattern: %08Xh\n", test_pattern[l]);

		u32_err = emmc_test_single_blk_wrc_miu_aes(u32_start_sector, test_pattern[l]);
		if (u32_err) {
			emmc_debug(0, 1, "AES single test fails %Xh\n", u32_err);
			return u32_err;
		}
		u32_err = emmc_test_multi_blk_wrc_miu_aes(u32_start_sector,
							  u32_sector_cnt, test_pattern[l]);
		if (u32_err) {
			emmc_debug(0, 1, "AES multi test fails %Xh\n", u32_err);
			return u32_err;
		}

		#if 1
		if (u32_data_unit_size[k] <= 512) {
			u32_err = emmc_test_multi_w_single_r_blk_wrc_miu_aes(u32_start_sector, u32_sector_cnt,
									     test_pattern[l]);
			if (u32_err) {
				emmc_debug(0, 1, "AES multi write-single read test fails %Xh\n", u32_err);
				return u32_err;
			}

			u32_err = emmc_test_single_w_multi_r_blk_wrc_miu_aes(u32_start_sector, u32_sector_cnt,
									     test_pattern[l]);
			if (u32_err) {
				emmc_debug(0, 1, "AES singe write-multi read test fails %Xh\n", u32_err);
				return u32_err;
			}
		}
		#endif
		u32_err = emmc_test_sg_wrc_miu_ase(u32_start_sector, u32_sector_cnt, test_pattern[l]);
		if (u32_err) {
			emmc_debug(0, 1, "AES ADMA test fails %Xh\n", u32_err);
			return u32_err;
		}
	}

	return u32_err;
}

U32 emmc_test_aes_all_mode_test(void)
{
	int i, j, k;
	U8 u8_mode[6] = {ECB_MODE, CBC_MODE, CTR_MODE, OFB_MODE, CFB_MODE, XTS_MODE};
	U8 u8_keysize[3] = {16, 24, 32};
	U8 u8_key[32], u8_iv[16], u8_ctr[16], u8_t_key[32];
	U32 u32_data_unit_size[4] = {512, 1024, 2048, 4096};
	U32 u32_start_sector, u32_sector_cnt;
	U32 u32_err = 0;
	U32 test_pattern[] = {
		EMMC_PATTERN_00112233,
		EMMC_PATTERN_00332211,
		EMMC_PATTERN_00000000,
		EMMC_PATTERN_FFFFFFFF,
		EMMC_PATTERN_FF00FF00,
		EMMC_PATTERN_00FF00FF,
		EMMC_PATTERN_55555555,
		EMMC_PATTERN_AAAAAAAA,
		EMMC_PATTERN_AA55AA55,
		EMMC_PATTERN_55AA55AA,
		EMMC_PATTERN_A55AA55A,
		EMMC_PATTERN_5AA55AA5
	};
	if (!gau8_wbuf)
		gau8_wbuf = (U8 *)memalign(EMMC_CACHE_LINE, TEST_BUFFER_SIZE);

	if (!gau8_rbuf)
		gau8_rbuf = (U8 *)memalign(EMMC_CACHE_LINE, TEST_BUFFER_SIZE);

	u32_start_sector = EMMC_TEST_BLK_0;
	u32_sector_cnt = EMMC_TEST_BLK_CNT;

	for (i = 0; i < 6; i++) {
		for (j = 0 ; j < 32; j++) {
			u8_key[j] = j;
			u8_t_key[j] = 0xFF - j;
			if (j < 16) {
				u8_iv[j] = j;
				u8_ctr[j] = 0xFF - j;
			}
		}

		switch (u8_mode[i]) {
		case ECB_MODE:
			//ECB mode: need key
			memset(u8_t_key, 0, sizeof(u8_t_key));
			memset(u8_ctr, 0, sizeof(u8_ctr));
			memset(u8_iv, 0, sizeof(u8_iv));
		break;
		case CBC_MODE:
		case OFB_MODE:
		case CFB_MODE:
			memset(u8_t_key, 0, sizeof(u8_t_key));
			memset(u8_ctr, 0, sizeof(u8_ctr));
		break;
		case CTR_MODE:
			memset(u8_t_key, 0, sizeof(u8_t_key));
			memset(u8_iv, 0, sizeof(u8_iv));
		break;
		case XTS_MODE:
		break;
		}
		for (j = 0; j < 3; j++) {
			if (u8_keysize[j] == 24 && u8_mode[i] == XTS_MODE)
				continue;

			fde_aes_set_key(u8_key, (U32)u8_keysize[j]);
			fde_set_set_iv(u8_iv);
			fde_aes_set_ctr(u8_ctr);
			fde_aes_set_tcalc_key(u8_t_key);
			fde_aes_set_mode(u8_mode[i]);
			if (u8_mode[i] != XTS_MODE)
				REG_FCIE_SETBIT(FDE_AES_FUN, BIT2);
			else
				REG_FCIE_CLRBIT(FDE_AES_FUN, BIT2);
			for (k = 0; k < 4; k++) {
				REG_FCIE_W(FDE_AES_DATA_UNIT0, u32_data_unit_size[k] & 0xFFFF);
				REG_FCIE_W(FDE_AES_DATA_UNIT1, u32_data_unit_size[k] >> 16);

				emmc_debug(EMMC_DEBUG_LEVEL, 1, "mode %Xh, key size %d, data unit %d\n",
					   u8_mode[i], u8_keysize[j], u32_data_unit_size[k]);

				u32_err = emmc_test_aes_all_mode_test_ex(k, test_pattern, u32_data_unit_size,
									 u32_start_sector, u32_sector_cnt);
				if (u32_err)
					return u32_err;
			}
		}
	}

	return 0;
}

U32 emmc_pesudo_random(void)
{
	static U64 u64_next = 1;

	u64_next = (u64_next + get_timer(0)) * 1103515245 + 12345;
	return (U32)(u64_next / 65536) % 32768;
}

#if defined(DMA_AES_TEST) && DMA_AES_TEST
static U32 emmc_test_random_sg_wrc_miu_aes_adma_write(U32 u32_emmc_addr)
{
	U32 u32_err;
	U32 u32_i;
	U32 u32_dmaaddr, u32_dmalen;
	unsigned long dma_addr;

	memset(p_adma_desc_st, 0, 4 *  sizeof(struct adma_descriptor));

	for (u32_i = 0; u32_i < 4; u32_i++) {
		if (p_sg_st[u32_i].u32_dma_address && p_sg_st[u32_i].u32_length) {
			u32_dmaaddr = p_sg_st[u32_i].u32_dma_address;
			u32_dmalen  = p_sg_st[u32_i].u32_length;

			dma_addr = emmc_translate_dma_address_ex((unsigned long)u32_dmaaddr, u32_dmalen);
			//parsing dma address, length to descriptor
			p_adma_desc_st[u32_i].u32_address = (dma_addr);
			if (sizeof(unsigned long) == 8)
				p_adma_desc_st[u32_i].u32_address2 = (dma_addr >> BIT_MIU_DMA_ADDR_47_32_SHIFT);

			p_adma_desc_st[u32_i].u32_dma_len = (u32_dmalen);
			p_adma_desc_st[u32_i].u32_job_cnt = (u32_dmalen >> EMMC_SECTOR_512BYTE_BITS);
			p_adma_desc_st[u32_i].u32_miu_sel = 0;
		} else {
			break;
		}
	}

	if (u32_i > 0)
		p_adma_desc_st[u32_i - 1].u32_end = 1;
	else
		p_adma_desc_st[0].u32_end = 1;

	//emmc_dump_mem((U8*)p_adma_desc_st, u32_i * sizeof(struct adma_descriptor));

	fde_aes_open();
	fde_aes_set_xts_lba(u32_emmc_addr);
	fde_aes_set_codec_dir(ENABLE_ENC);
	fde_aes_set_xex_hw_t_calc_kick(1);
	u32_err = emmc_cmd25_adma(u32_emmc_addr, (U8 *)p_adma_desc_st, sizeof(struct adma_descriptor) * u32_i);
	fde_aes_set_xex_hw_t_calc_kick(0);
	fde_aes_close();

	if (u32_err != EMMC_ST_SUCCESS)
		emmc_debug(0, 1, "Err, SgWRC_MIU W, %Xh\n", u32_err);

	return u32_err;
}

static U32 emmc_test_random_sg_wrc_miu_aes_adma_read(U32 u32_emmc_addr)
{
	U32 u32_err = EMMC_ST_SUCCESS;
	U32 u32_i;
	U32 u32_dmaaddr, u32_dmalen;
	unsigned long dma_addr;

	memset(p_adma_desc_st, 0, MAX_SCATTERLIST_COUNT *  sizeof(struct adma_descriptor));

	u32_i = 0;
	for (u32_i = 0; u32_i < MAX_SCATTERLIST_COUNT; u32_i++) {
		if (p_sg_st[u32_i].u32_dma_address && p_sg_st[u32_i].u32_length) {
			u32_dmaaddr = p_sg_st[u32_i].u32_dma_address;
			u32_dmalen	= p_sg_st[u32_i].u32_length;

			dma_addr = emmc_translate_dma_address_ex((unsigned long)u32_dmaaddr, u32_dmalen);

			//parsing dma address, length to descriptor
			p_adma_desc_st[u32_i].u32_address = (dma_addr);
			if (sizeof(unsigned long) == 8)
				p_adma_desc_st[u32_i].u32_address2 = (dma_addr >> BIT_MIU_DMA_ADDR_47_32_SHIFT);
			p_adma_desc_st[u32_i].u32_dma_len = (u32_dmalen);
			p_adma_desc_st[u32_i].u32_job_cnt = (u32_dmalen >> EMMC_SECTOR_512BYTE_BITS);
			p_adma_desc_st[u32_i].u32_miu_sel  = 0;
		} else {
			break;
		}
	}

	if (u32_i > 0)
		p_adma_desc_st[u32_i - 1].u32_end = 1;
	else
		p_adma_desc_st[0].u32_end = 1;

	//emmc_dump_mem((U8 *)p_adma_desc_st, u32_i *  sizeof(struct adma_descriptor));

	fde_aes_open();
	fde_aes_set_xts_lba(u32_emmc_addr);
	fde_aes_set_codec_dir(ENABLE_DEC);
	fde_aes_set_xex_hw_t_calc_kick(1);
	u32_err = emmc_cmd18_adma(u32_emmc_addr, (U8 *)p_adma_desc_st, sizeof(struct adma_descriptor) * u32_i);
	fde_aes_set_xex_hw_t_calc_kick(0);
	fde_aes_close();
	#ifndef NEED_POST_INVALIDATE_CACHE
	flush_cache((unsigned long)gau8_rbuf, TEST_BUFFER_SIZE);
	#endif
	if (u32_err != EMMC_ST_SUCCESS)
		emmc_debug(0, 1, "Err, SgWRC_MIU R, %Xh\n", u32_err);

	#if defined(NEED_POST_INVALIDATE_CACHE) && NEED_POST_INVALIDATE_CACHE
	for (u32_i = 0; u32_i < MAX_SCATTERLIST_COUNT; u32_i++) {
		if (p_sg_st[u32_i].u32_dma_address && p_sg_st[u32_i].u32_length) {
			u32_dmaaddr = p_sg_st[u32_i].u32_dma_address;
			u32_dmalen = p_sg_st[u32_i].u32_length;
			emmc_invalidate_data_cache_buffer((unsigned long)u32_dmaaddr, u32_dmalen);
		} else {
			break;
		}
	}
	#endif

	return u32_err;
}

U32 emmc_test_random_sg_wrc_miu_aes(U32 u32_emmc_addr, U16 u16_blk_cnt, U8 is_write)
{
	U32 u32_err = EMMC_ST_SUCCESS;
	U32 u32_i, u32_j, *pu32_w = (U32 *)gau8_wbuf;
	U32 u32_temp;

	for (u32_i = 0; u32_i < EMMC_TEST_BLK_CNT; u32_i++) {
		for (u32_j = 0; u32_j < (EMMC_SECTOR_512BYTE >> 2); u32_j++)
			pu32_w[u32_i * (EMMC_SECTOR_512BYTE >> 2) + u32_j] = u32_emmc_addr + u32_i;
	}

	if (is_write) {
		u32_temp = 0;
		memset(p_sg_st, 0, 4 *  sizeof(struct adma_descriptor));
		//setting scatterlist dma address and dma length for write multiple block
		for (u32_i = 0; u32_i < 4; u32_i++) {
			p_sg_st[u32_i].u32_dma_address = (unsigned long)(gau8_wbuf + u32_temp);
			p_sg_st[u32_i].u32_length = EMMC_SECTOR_512BYTE << 1;
			u32_temp += EMMC_SECTOR_512BYTE << 1;
		}
		u32_err = emmc_test_random_sg_wrc_miu_aes_adma_write(u32_emmc_addr);
		if (u32_err != EMMC_ST_SUCCESS)
			return u32_err;
	}
	//emmc_debug(1, 0, "===================================================\n");

	u32_temp = 0;
	memset(p_sg_st, 0, 4 *  sizeof(struct adma_descriptor));

	//setting scatterlist dma address and dma length for read multiple block
	for (u32_i = 0; u32_i < 4; u32_i++) {
		p_sg_st[u32_i].u32_dma_address = (unsigned long)(gau8_rbuf + u32_temp);
		p_sg_st[u32_i].u32_length = EMMC_SECTOR_512BYTE << 1;
		u32_temp += EMMC_SECTOR_512BYTE << 1;
	}

	u32_err = emmc_test_random_sg_wrc_miu_aes_adma_read(u32_emmc_addr);
	if (u32_err != EMMC_ST_SUCCESS)
		return u32_err;

	u32_err = emmc_compare_data(gau8_wbuf, gau8_rbuf, u16_blk_cnt << EMMC_SECTOR_512BYTE_BITS);

	if (u32_err != EMMC_ST_SUCCESS) {
		emmc_debug(0, 1, "Err, SgWRC_MIU C, %u, %Xh\n", u32_i, u32_err);
		return u32_err;
	}

	return EMMC_ST_SUCCESS;
}

U32 emmc_test_aes_random_pos_test(void)
{
	U32 u32_start_sector, u32_sector_cnt;
	U32 u32_testblkaddr, u32_testblkcnt;
	U32 u32_i, u32_j, *pu32_w = (U32 *)gau8_wbuf;
	U32 u32_err = 0;
	U8 u8_is_write;

	fde_aes_init();
	u32_start_sector = 0x1A3E000;
	u32_sector_cnt = EMMC_TEST_BLK_CNT;

	emmc_debug(EMMC_DEBUG_LEVEL, 1, "AES RandomPosTest +\n");

	fde_aes_init();

	//initial data
	for (u32_i = 0; u32_i < EMMC_TEST_BLK_CNT; u32_i++) {
		for (u32_j = 0; u32_j < EMMC_SECTOR_512BYTE / 4; u32_j++)
			pu32_w[u32_i * (EMMC_SECTOR_512BYTE / 4) + u32_j] = u32_start_sector + u32_i;
	}
	fde_aes_open();
	fde_aes_set_xts_lba(u32_start_sector);
	fde_aes_set_codec_dir(ENABLE_ENC);
	fde_aes_set_xex_hw_t_calc_kick(1);
	u32_err = emmc_cmd25_miu(u32_start_sector, gau8_wbuf, u32_sector_cnt);

	fde_aes_set_xex_hw_t_calc_kick(0);
	fde_aes_close();
	if (u32_err != EMMC_ST_SUCCESS) {
		emmc_debug(0, 1, "Err, MultiBlkWRC W, %Xh\n", u32_err);
		goto LABEL_OF_ERROR;
	}
	fde_aes_open();
	fde_aes_set_xts_lba(u32_start_sector);
	fde_aes_set_codec_dir(ENABLE_DEC);
	fde_aes_set_xex_hw_t_calc_kick(1);

	u32_err = emmc_cmd18_miu(u32_start_sector, gau8_rbuf, u32_sector_cnt);
	fde_aes_set_xex_hw_t_calc_kick(0);
	fde_aes_close();
	if (u32_err != EMMC_ST_SUCCESS) {
		emmc_debug(0, 1, "Err, MultiBlkWRC R, %Xh\n", u32_err);
		goto LABEL_OF_ERROR;
	}

	u32_err = emmc_compare_data(gau8_wbuf, gau8_rbuf, (EMMC_TEST_BLK_CNT) << EMMC_SECTOR_512BYTE_BITS);

	if (u32_err != EMMC_ST_SUCCESS) {
		emmc_debug(0, 1, "Err, MultiBlkWRC C, %Xh\n", u32_err);
		goto LABEL_OF_ERROR;
	}

	//get start address and length
	while (1) {
		u8_is_write = (emmc_pesudo_random() % 2);
		u32_testblkaddr = (emmc_pesudo_random() %  EMMC_TEST_BLK_CNT);
		u32_testblkcnt = (EMMC_TEST_BLK_CNT -  u32_testblkaddr) >
			DDR_TEST_BLK_CNT ?
			DDR_TEST_BLK_CNT :
			(EMMC_TEST_BLK_CNT -  u32_testblkaddr);
		if (u32_testblkcnt < DDR_TEST_BLK_CNT)
			continue;

		u32_testblkaddr += u32_start_sector;
		emmc_debug(0, 1, "Random test blkaddr %Xh, blkcnt %Xh, write? %Xh\n",
			   u32_testblkaddr, u32_testblkcnt, u8_is_write);
		u32_err = emmc_test_random_sg_wrc_miu_aes(u32_testblkaddr, u32_testblkcnt, u8_is_write);
		if (u32_err != EMMC_ST_SUCCESS) {
			emmc_debug(0, 1, "Err, Random test fail %Xh\n", u32_err);
			emmc_debug(0, 1, "Err, Random test blkaddr %Xh, blkcnt %Xh, write? %Xh\n",
				   u32_testblkaddr, u32_testblkcnt, u8_is_write);
			goto LABEL_OF_ERROR;
		}
	}
LABEL_OF_ERROR:

	return u32_err;
}
#endif

static U32 emmc_test_sg_wrc_miu_adma_write(U32 u32_emmc_addr)
{
	U32 u32_err;
	U32 u32_i;
	U32 u32_dmalen;
	unsigned long dma_addr, dmaaddr;

	memset(p_adma_desc_st, 0, MAX_SCATTERLIST_COUNT *  sizeof(struct adma_descriptor));

	for (u32_i = 0; u32_i < MAX_SCATTERLIST_COUNT; u32_i++) {
		if (p_sg_st[u32_i].u32_dma_address && p_sg_st[u32_i].u32_length) {
			dmaaddr = p_sg_st[u32_i].u32_dma_address;
			u32_dmalen  = p_sg_st[u32_i].u32_length;

			dma_addr = emmc_translate_dma_address_ex((unsigned long)dmaaddr, u32_dmalen);

			//parsing dma address, length to descriptor
			p_adma_desc_st[u32_i].u32_address = (dma_addr);
			if (sizeof(unsigned long) == 8)
				p_adma_desc_st[u32_i].u32_address2 = (dma_addr >> BIT_MIU_DMA_ADDR_47_32_SHIFT);

			p_adma_desc_st[u32_i].u32_dma_len = (u32_dmalen);
			p_adma_desc_st[u32_i].u32_job_cnt = (u32_dmalen >> EMMC_SECTOR_512BYTE_BITS);
			p_adma_desc_st[u32_i].u32_miu_sel = 0;//FPGA Mode always miu0
		} else {
			break;
		}
	}

	if (u32_i > 0)
		p_adma_desc_st[u32_i - 1].u32_end = 1;
	else
		p_adma_desc_st[0].u32_end = 1;

	//emmc_dump_mem((U8 *)p_adma_desc_st, u32_i * sizeof(struct adma_descriptor));

	u32_err = emmc_cmd25_adma(u32_emmc_addr, (U8 *)p_adma_desc_st, sizeof(struct adma_descriptor) * u32_i);

	if (u32_err != EMMC_ST_SUCCESS)
		emmc_debug(0, 1, "Err, SgWRC_MIU W, %Xh\n", u32_err);

	return u32_err;
}

static U32 emmc_test_sg_wrc_miu_adma_read(U32 u32_emmc_addr)
{
	U32 u32_err = EMMC_ST_SUCCESS;
	U32 u32_i;
	U32 u32_dmalen;
	unsigned long dma_addr, dmaaddr;

	memset(p_adma_desc_st, 0, MAX_SCATTERLIST_COUNT *  sizeof(struct adma_descriptor));

	u32_i = 0;
	for (u32_i = 0; u32_i < MAX_SCATTERLIST_COUNT; u32_i++) {
		if (p_sg_st[u32_i].u32_dma_address && p_sg_st[u32_i].u32_length) {
			dmaaddr = p_sg_st[u32_i].u32_dma_address;
			u32_dmalen  = p_sg_st[u32_i].u32_length;

			dma_addr = emmc_translate_dma_address_ex((unsigned long)dmaaddr, u32_dmalen);

			//parsing dma address, length to descriptor
			p_adma_desc_st[u32_i].u32_address = (dma_addr);
			if (sizeof(unsigned long) == 8)
				p_adma_desc_st[u32_i].u32_address2 = (dma_addr >> BIT_MIU_DMA_ADDR_47_32_SHIFT);

			p_adma_desc_st[u32_i].u32_dma_len = (u32_dmalen);
			p_adma_desc_st[u32_i].u32_job_cnt = (u32_dmalen >> EMMC_SECTOR_512BYTE_BITS);
			p_adma_desc_st[u32_i].u32_miu_sel  = 0;
		} else {
			break;
		}
	}

	if (u32_i > 0)
		p_adma_desc_st[u32_i - 1].u32_end = 1;
	else
		p_adma_desc_st[0].u32_end = 1;

	//emmc_dump_mem((U8 *)p_adma_desc_st, u32_i *  sizeof(struct adma_descriptor));

	u32_err = emmc_cmd18_adma(u32_emmc_addr, (U8 *)p_adma_desc_st, sizeof(struct adma_descriptor) * u32_i);
	if (u32_err != EMMC_ST_SUCCESS) {
		emmc_debug(0, 1, "Err, SgWRC_MIU R, %Xh\n", u32_err);
		return u32_err;
	}

	#ifndef NEED_POST_INVALIDATE_CACHE
	flush_cache((unsigned long)gau8_rbuf, TEST_BUFFER_SIZE);
	#endif

	#if defined(NEED_POST_INVALIDATE_CACHE) && NEED_POST_INVALIDATE_CACHE
	for (u32_i = 0; u32_i < MAX_SCATTERLIST_COUNT; u32_i++) {
		if (p_sg_st[u32_i].u32_dma_address && p_sg_st[u32_i].u32_length) {
			dmaaddr = p_sg_st[u32_i].u32_dma_address;
			u32_dmalen  = p_sg_st[u32_i].u32_length;
			emmc_invalidate_data_cache_buffer((unsigned long)dmaaddr, u32_dmalen);
		} else {
			break;
		}
	}
	#endif

	return u32_err;
}

U32 emmc_test_sg_wrc_miu(U32 u32_emmc_addr, U16 u16_blk_cnt, U32 u32_data_pattern)
{
	U32 u32_err = EMMC_ST_SUCCESS;
	U32 u32_i, u32_j, *pu32_w = (U32 *)gau8_wbuf, *pu32_r = (U32 *)gau8_rbuf;
	U32 u32_temp, u32_sg_blocks;

	if (u32_data_pattern == EMMC_PATTERN_00112233 || u32_data_pattern == EMMC_PATTERN_00332211) {
		for (u32_i = 0; u32_i < TEST_BUFFER_SIZE; u32_i++) {
			if (u32_data_pattern == EMMC_PATTERN_00112233)
				gau8_wbuf[u32_i] = u32_i & 0xFF;
			else
				gau8_wbuf[u32_i] = 0xFF - (u32_i & 0xFF);

			gau8_rbuf[u32_i] = ~gau8_wbuf[u32_i];
		}
	} else {
		for (u32_i = 0; u32_i < TEST_BUFFER_SIZE >> 2; u32_i++) {
			pu32_w[u32_i] = u32_data_pattern;
			pu32_r[u32_i] = ~pu32_w[u32_i];
		}
	}

	u32_sg_blocks = u16_blk_cnt / MAX_SCATTERLIST_COUNT;
	u32_temp = 0;
	memset(p_sg_st, 0, MAX_SCATTERLIST_COUNT *  sizeof(struct adma_descriptor));

	//setting scatterlist dma address and dma length for write multiple block
	for (u32_i = 0; u32_i < MAX_SCATTERLIST_COUNT / 4; u32_i++) {
		for (u32_j = 0; u32_j < 4; u32_j++) {
			p_sg_st[u32_i * 4 + u32_j].u32_dma_address = (unsigned long)(gau8_wbuf + u32_temp);
			if (u32_j < 3) {
				p_sg_st[u32_i * 4 + u32_j].u32_length = EMMC_SECTOR_512BYTE;
				u32_temp += EMMC_SECTOR_512BYTE;
			} else {
				p_sg_st[u32_i * 4 + u32_j].u32_length = (4 * u32_sg_blocks - 3) << EMMC_SECTOR_512BYTE_BITS;
				u32_temp += (4 * u32_sg_blocks - 3) << EMMC_SECTOR_512BYTE_BITS;
			}
		}
	}

	u32_err = emmc_test_sg_wrc_miu_adma_write(u32_emmc_addr);
	if (u32_err != EMMC_ST_SUCCESS)
		return u32_err;

	//emmc_debug(1, 0, "===================================================\n");

	u32_temp = 0;
	memset(p_sg_st, 0, MAX_SCATTERLIST_COUNT *  sizeof(struct adma_descriptor));

	//setting scatterlist dma address and dma length for read multiple block
	for (u32_i = 0; u32_i < MAX_SCATTERLIST_COUNT / 4; u32_i++) {
		for (u32_j = 0; u32_j < 4; u32_j++) {
			p_sg_st[u32_i * 4 + u32_j].u32_dma_address = (unsigned long)(gau8_rbuf + u32_temp);
			if (u32_j < 3) {
				p_sg_st[u32_i * 4 + u32_j].u32_length = EMMC_SECTOR_512BYTE;
				u32_temp += EMMC_SECTOR_512BYTE;
			} else {
				p_sg_st[u32_i * 4 + u32_j].u32_length = (4 * u32_sg_blocks - 3) << EMMC_SECTOR_512BYTE_BITS;
				u32_temp += (4 * u32_sg_blocks - 3) << EMMC_SECTOR_512BYTE_BITS;
			}
		}
	}

	u32_err = emmc_test_sg_wrc_miu_adma_read(u32_emmc_addr);
	if (u32_err != EMMC_ST_SUCCESS)
		return u32_err;

	u32_err = emmc_compare_data(gau8_wbuf, gau8_rbuf, u16_blk_cnt << EMMC_SECTOR_512BYTE_BITS);

	if (u32_err != EMMC_ST_SUCCESS)
		emmc_debug(0, 1, "Err, SgWRC_MIU C, %u, %Xh\n", u32_i, u32_err);

	return u32_err;
}

U32 emmc_test_multi_blk_wrc_miu(U32 u32_emmc_addr, U16 u16_blk_cnt, U32 u32_data_pattern)
{
	U32 u32_err = 0;
	U32 u32_i, *pu32_w = (U32 *)gau8_wbuf, *pu32_r = (U32 *)gau8_rbuf;
	U16 u16_blk_cnt_tmp;

	//init data pattern
	for (u32_i = 0; u32_i < TEST_BUFFER_SIZE >> 2; u32_i++) {
		pu32_w[u32_i] = u32_data_pattern;
		pu32_r[u32_i] = ~pu32_w[u32_i];
	}

	u16_blk_cnt_tmp = TEST_BUFFER_SIZE >> EMMC_SECTOR_512BYTE_BITS;
	u16_blk_cnt_tmp = u16_blk_cnt_tmp > u16_blk_cnt ? u16_blk_cnt : u16_blk_cnt_tmp;

	for (u32_i = 0; u32_i < u16_blk_cnt_tmp;) {
		u32_err = emmc_cmd25_miu(u32_emmc_addr + u32_i,
					 gau8_wbuf + (u32_i << EMMC_SECTOR_512BYTE_BITS), u16_blk_cnt_tmp - u32_i);
		if (u32_err != EMMC_ST_SUCCESS) {
			emmc_debug(0, 1, "Err, MultiBlkWRC W, %u, %Xh\n", u32_i, u32_err);
			goto LABEL_OF_ERROR;
		}

		u32_err = emmc_cmd18_miu(u32_emmc_addr + u32_i,
					 gau8_rbuf + (u32_i << EMMC_SECTOR_512BYTE_BITS), u16_blk_cnt_tmp - u32_i);
		if (u32_err != EMMC_ST_SUCCESS) {
			emmc_debug(0, 1, "Err, MultiBlkWRC R, %u, %Xh\n", u32_i, u32_err);
			goto LABEL_OF_ERROR;
		}

		u32_err = emmc_compare_data(gau8_wbuf + (u32_i << EMMC_SECTOR_512BYTE_BITS),
					    gau8_rbuf + (u32_i << EMMC_SECTOR_512BYTE_BITS), (u16_blk_cnt_tmp - u32_i) << 9);
		if (u32_err != EMMC_ST_SUCCESS) {
			emmc_debug(0, 1, "Err, MultiBlkWRC C, %u, %Xh\n", u32_i, u32_err);
			goto LABEL_OF_ERROR;
		}
		u32_i += u16_blk_cnt_tmp - u32_i;
	}

	return u32_err;

LABEL_OF_ERROR:
	return u32_err;
}

U32 emmc_test_multi_blk_wrc_miu_large(U32 u32_emmc_addr, U32 u32_data_pattern)
{
	U32 u32_start_sector = 0, u32_sector_cnt = 0, u32_err = 0;
	U32 u32_i, *pu32_w = (U32 *)DMA_W_ADDR, *pu32_r = (U32 *)DMA_R_ADDR, u32_byte_cnt;
	U8 *pu8_w = (U8 *)DMA_W_ADDR, *pu8_r = (U8 *)DMA_R_ADDR;

	u32_start_sector = u32_emmc_addr;
	u32_sector_cnt = 0xFFFF;
	u32_byte_cnt = (u32_sector_cnt << EMMC_SECTOR_512BYTE_BITS);

	if ((unsigned long)pu8_r > (unsigned long)pu8_w) {
		if (((unsigned long)pu8_r - (unsigned long)pu8_w) < (u32_sector_cnt << 9)) {
			emmc_debug(0, 1, "Err, Please Reconfigure your Write / Read buffer address W: %lX, R: %lXh\n",
				   (unsigned long)pu8_w, (unsigned long)pu8_r);
			goto LABEL_IP_VERIFY_ERROR;
		}
	} else {
		if (((unsigned long)pu8_w - (unsigned long)pu8_r) < (u32_sector_cnt << 9)) {
			emmc_debug(0, 1, "Err, Please Reconfigure your Write / Read buffer address W: %lX, R: %lXh\n",
				   (unsigned long)pu8_w, (unsigned long)pu8_r);
			goto LABEL_IP_VERIFY_ERROR;
		}
	}

	//init data pattern
	for (u32_i = 0; u32_i < u32_byte_cnt >> 2; u32_i++) {
		pu32_w[u32_i] = u32_data_pattern;
		pu32_r[u32_i] = ~pu32_w[u32_i];
	}

	for (u32_i = 0; u32_i < u32_sector_cnt;) {
		u32_err = emmc_cmd25_miu(u32_start_sector + u32_i,
					 pu8_w + (u32_i << EMMC_SECTOR_512BYTE_BITS), u32_sector_cnt - u32_i);
		if (u32_err != EMMC_ST_SUCCESS) {
			emmc_debug(0, 1, "Err, MultiBlkWRC W, %u, %Xh\n", u32_i, u32_err);
			goto LABEL_IP_VERIFY_ERROR;
		}

		u32_err = emmc_cmd18_miu(u32_start_sector + u32_i,
					 pu8_r + (u32_i << EMMC_SECTOR_512BYTE_BITS), u32_sector_cnt - u32_i);
		if (u32_err != EMMC_ST_SUCCESS) {
			emmc_debug(0, 1, "Err, MultiBlkWRC R, %u, %Xh\n", u32_i, u32_err);
			goto LABEL_IP_VERIFY_ERROR;
		}

		u32_err = emmc_compare_data(pu8_w + (u32_i << EMMC_SECTOR_512BYTE_BITS),
					    pu8_r + (u32_i << EMMC_SECTOR_512BYTE_BITS), (u32_sector_cnt - u32_i) << 9);
		if (u32_err != EMMC_ST_SUCCESS) {
			emmc_debug(0, 1, "Err, MultiBlkWRC C, %u, %Xh\n", u32_i, u32_err);
			goto LABEL_IP_VERIFY_ERROR;
		}

		u32_i += u32_sector_cnt - u32_i;
	}

	//===============================================
	//emmc_debug(EMMC_DEBUG_LEVEL, 1, "\n");
	//emmc_debug(EMMC_DEBUG_LEVEL, 1, "[OK]\n\n");
	return EMMC_ST_SUCCESS;

LABEL_IP_VERIFY_ERROR:
	emmc_debug(EMMC_DEBUG_LEVEL, 1, "\n\n");
	emmc_debug(EMMC_DEBUG_LEVEL, 1, "Total Sec: %Xh, Test: StartSec: %Xh, SecCnt: %Xh\n",
		   emmc_drv.u32_sec_count, u32_start_sector, u32_sector_cnt);
	emmc_debug(EMMC_DEBUG_LEVEL, 1, "fail: %Xh\n\n", u32_err);
	emmc_dump_driver_status();

	return u32_err;
}

// =========================================================
U32 emmc_boot_mode(U32 *pu32_addr, U32 u32_byte_cnt)
{
	U32 u32_err = EMMC_ST_SUCCESS;
	U32 u32_boot_partition_size, u32_byte_cnt_hash0, u32_byte_cnt_hash1, u32_byte_cnt_hash2;
	U32 u32_emmc_chksum_rom = 0, u32_emmc_chksum_tmp = 0;

	u32_byte_cnt = (u32_byte_cnt & EMMC_SECTOR_512BYTE_MASK) ? (u32_byte_cnt + EMMC_SECTOR_512BYTE) : u32_byte_cnt;
	u32_boot_partition_size = emmc_drv.u8_boot_size_mult * 128 * 1024;
	if (u32_byte_cnt > u32_boot_partition_size) {
		emmc_debug(0, 1, "ByteCnt:%xh overflow eMMC boot partition size:%xh !!\n",
			   u32_byte_cnt, u32_boot_partition_size);
		goto LABEL_BOOT_MODE_END;
	} else if (u32_byte_cnt < (3 * EMMC_SECTOR_512BYTE)) {
		emmc_debug(0, 1, "ByteCnt:%xh less than 3 block!!\n", u32_byte_cnt);
		goto LABEL_BOOT_MODE_END;
	}

	u32_byte_cnt_hash0 = ((u32_byte_cnt / EMMC_SECTOR_512BYTE) / 3) * EMMC_SECTOR_512BYTE;
	u32_byte_cnt_hash1 = ((u32_byte_cnt / EMMC_SECTOR_512BYTE) / 3) * EMMC_SECTOR_512BYTE;
	u32_byte_cnt_hash2 = u32_byte_cnt - u32_byte_cnt_hash0 - u32_byte_cnt_hash1;
	emmc_hw_timer_start();

	u32_err = emmc_load_images(pu32_addr, u32_byte_cnt_hash0, 0);
	if (u32_err != EMMC_ST_SUCCESS)
		goto LABEL_BOOT_MODE_END;

	u32_err = emmc_load_images(pu32_addr + (u32_byte_cnt_hash0 >> 2), u32_byte_cnt_hash1, 1);
	if (u32_err != EMMC_ST_SUCCESS)
		goto LABEL_BOOT_MODE_END;

	u32_err = emmc_load_images(pu32_addr +
				   ((u32_byte_cnt_hash0 + u32_byte_cnt_hash1) >> 2), u32_byte_cnt_hash2, 2);
	if (u32_err != EMMC_ST_SUCCESS)
		goto LABEL_BOOT_MODE_END;

	u32_emmc_chksum_rom = emmc_chk_sum((U8 *)pu32_addr, u32_byte_cnt);
	//----------------------------------------
	emmc_platform_init();
	u32_err = emmc_fcie_init();
	if (u32_err != EMMC_ST_SUCCESS)
		goto  LABEL_BOOT_MODE_END;

	u32_err = emmc_init_device_ex();
	if (u32_err != EMMC_ST_SUCCESS)
		goto  LABEL_BOOT_MODE_END;

	u32_err = emmc_fcie_choose_speed_mode();
	if (u32_err != EMMC_ST_SUCCESS)
		goto  LABEL_BOOT_MODE_END;

	u32_err = emmc_read_boot_part((U8 *)pu32_addr, u32_byte_cnt, 0, 1);
	if (u32_err != EMMC_ST_SUCCESS)
		goto LABEL_BOOT_MODE_END;

	u32_emmc_chksum_tmp = emmc_chk_sum((U8 *)pu32_addr, u32_byte_cnt);

	if (u32_emmc_chksum_rom != u32_emmc_chksum_tmp) {
		emmc_debug(0, 0, "eMMC Rom boot checksum:%xh misatch read boot partition checksum:%xh !!\n",
			   u32_emmc_chksum_rom,
			   u32_emmc_chksum_tmp);
		goto LABEL_BOOT_MODE_END;
	}
	emmc_debug(0, 0, "eMMC Rom boot verify pass!!\n");
	return EMMC_ST_SUCCESS;

LABEL_BOOT_MODE_END:

	emmc_fcie_err_handler_stop();
	return u32_err;
}

void emmc_test_disable_uart(void)
{
	u32 *ptr;

	printf("Disable UART\n");

	ptr = (u32 *)(RIU_BASE + (0x0E12 << 1));
	*ptr &= ~0x1800;
}

//========================================================
//exposed function
//========================================================
#define EMMC_SPEED_TEST_COUNTDOWN          3

#define EMMC_SPEED_TEST_SINGLE_BLKCNT_W    50
#define EMMC_SPEED_TEST_MULTIPLE_BLKCNT_W  ((U32)5 * (1024 * 1024 / 512))

#define EMMC_SPEED_TEST_R_MULTI            20
#define EMMC_SPEED_TEST_SINGLE_BLKCNT_R    ((U32)EMMC_SPEED_TEST_SINGLE_BLKCNT_W * EMMC_SPEED_TEST_R_MULTI)
#define EMMC_SPEED_TEST_MULTIPLE_BLKCNT_R  ((U32)EMMC_SPEED_TEST_MULTIPLE_BLKCNT_W * EMMC_SPEED_TEST_R_MULTI)

void emmc_test_down_count(U32 u32_sec)
{
	U32 u32_i;
	U32 u32_t0;

	for (u32_i = 0; u32_i < u32_sec; u32_i++) {
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "%u\n", u32_sec - u32_i);
		u32_t0 = get_timer(0);
		while (get_timer(u32_t0) < 1000)
			;
	}
	emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, ".\n");
}

static U32 emmc_ipverify_performance_init(void)
{
	U32 u32_err = EMMC_ST_SUCCESS, u32_i;

	if (!gau8_wbuf)
		gau8_wbuf = (U8 *)memalign(EMMC_CACHE_LINE, TEST_BUFFER_SIZE);

	if (!gau8_rbuf)
		gau8_rbuf = (U8 *)memalign(EMMC_CACHE_LINE, TEST_BUFFER_SIZE);

	emmc_debug(EMMC_DEBUG_LEVEL_HIGH, 1, "\n");
	emmc_debug(EMMC_DEBUG_LEVEL_HIGH, 1, "SectorBuf:%lXh, RBuf:%lXh, WBuf:%lXh\n",
		   (unsigned long)GLOBAL_EMMC_SECTORBUF, (unsigned long)gau8_rbuf, (unsigned long)gau8_wbuf);

	//===============================================
	emmc_platform_init();
	u32_err = emmc_fcie_init();
	if (u32_err != EMMC_ST_SUCCESS) {
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "Err, emmc_fcie_init fail: %Xh\n", u32_err);
		goto LABEL_END;
	}
	u32_err = emmc_init_device_ex();
	if (u32_err != EMMC_ST_SUCCESS) {
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "Err, emmc_init_device_ex fail: %Xh\n", u32_err);
		goto LABEL_END;
	}

	emmc_test_down_count(EMMC_SPEED_TEST_COUNTDOWN);
	for (u32_i = 0; u32_i < TEST_BUFFER_SIZE; u32_i++)
		gau8_wbuf[u32_i] = (U8)u32_i;

	//===============================================
	u32_err = emmc_fcie_choose_speed_mode();
	if (u32_err != EMMC_ST_SUCCESS)
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: ChooseSpeedMode fail: %Xh\n", u32_err);

LABEL_END:

	return u32_err;
}

static U32 emmc_ipverify_performance_write_single_blk(U32 *u32_sector_addr)
{
	U32 u32_err = EMMC_ST_SUCCESS, u32_i, sector_addr;

	sector_addr = (*u32_sector_addr);

	for (u32_i = 0; u32_i < EMMC_SPEED_TEST_SINGLE_BLKCNT_W; u32_i++) {
		sector_addr += (emmc_drv.u32_sec_count >> 2) + 333 * u32_i;
		sector_addr %= emmc_drv.u32_sec_count;
		if (sector_addr < emmc_drv.u32_sec_count >> 1)
			sector_addr += emmc_drv.u32_sec_count >> 1;

		sector_addr -= EMMC_SPEED_TEST_SINGLE_BLKCNT_W;
		sector_addr = EMMC_TEST_BLK_0 + (sector_addr % EMMC_TEST_BLK_CNT);

		//emmc_debug(0, 0, "%08Xh\n", u32_sector_addr + u32_i);
		#if EMMC_FEATURE_RELIABLE_WRITE
		u32_err = emmc_cmd24(sector_addr, gau8_wbuf);
		#else
		u32_err = emmc_cmd24_miu(sector_addr, gau8_wbuf);
		#endif
		if (u32_err != EMMC_ST_SUCCESS) {
			emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: %Xh\n", u32_err);
			goto LABEL_END;
		}
	}
	emmc_fcie_wait_d0_high(TIME_WAIT_DAT0_HIGH);

LABEL_END:

	*u32_sector_addr = sector_addr;
	return u32_err;
}

static U32 emmc_ipverify_performance_read_single_blk(U32 *u32_sector_addr)
{
	U32 u32_err = 0, u32_i, sector_addr;

	sector_addr = (*u32_sector_addr);

	for (u32_i = 0; u32_i < EMMC_SPEED_TEST_SINGLE_BLKCNT_R; u32_i++) {
		sector_addr += (emmc_drv.u32_sec_count >> 2) + 333 * u32_i;
		sector_addr %= emmc_drv.u32_sec_count;
		if (sector_addr < (emmc_drv.u32_sec_count >> 1))
			sector_addr += emmc_drv.u32_sec_count >> 1;
		sector_addr -= EMMC_SPEED_TEST_SINGLE_BLKCNT_R;
		sector_addr = EMMC_TEST_BLK_0 + (sector_addr % EMMC_TEST_BLK_CNT);

		u32_err = emmc_cmd17_miu(sector_addr, gau8_rbuf);
		if (u32_err != EMMC_ST_SUCCESS) {
			emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: %Xh\n", u32_err);
			goto LABEL_END;
		}
	}
	emmc_fcie_wait_d0_high(TIME_WAIT_DAT0_HIGH);

LABEL_END:

	*u32_sector_addr = sector_addr;
	return u32_err;
}

static void emmc_ipverify_performance_sector_cnt(U32 *u32_sector_cnt)
{
	U32 sector_cnt = 0;

	sector_cnt = (*u32_sector_cnt);
	emmc_debug(0, 0, "------------------------------\n");
	switch (sector_cnt) {
	case 0:
		sector_cnt = 0x400 * 4 >> EMMC_SECTOR_512BYTE_BITS;//4KB
	break;
	case 0x400 * 4 >> EMMC_SECTOR_512BYTE_BITS:
		sector_cnt = 0x400 * 16 >> EMMC_SECTOR_512BYTE_BITS;//16KB
	break;
	case 0x400 * 16 >> EMMC_SECTOR_512BYTE_BITS:
		sector_cnt = 0x400 * 64 >> EMMC_SECTOR_512BYTE_BITS;//64KB
	break;
	case 0x400 * 64 >> EMMC_SECTOR_512BYTE_BITS:
		sector_cnt = 0x400 * 128 >> EMMC_SECTOR_512BYTE_BITS;//128KB
	break;
	case 0x400 * 128 >> EMMC_SECTOR_512BYTE_BITS:
		sector_cnt = 0x400 * 256 >> EMMC_SECTOR_512BYTE_BITS;//256KB
	break;
	case 0x400 * 256 >> EMMC_SECTOR_512BYTE_BITS:
		sector_cnt = 0x400 * 1024 >> EMMC_SECTOR_512BYTE_BITS;//1MB
	break;
	}
	emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 0, "Multi Write, total: %u MB, burst size: %u KB\n",
		   (EMMC_SPEED_TEST_MULTIPLE_BLKCNT_W << EMMC_SECTOR_512BYTE_BITS) >> 20, sector_cnt >> 1);

	*u32_sector_cnt = sector_cnt;
}

U32 emmc_ipverify_performance(void)
{
	U32 u32_err = EMMC_ST_SUCCESS, u32_i, u32_sector_addr = 0, u32_sector_cnt = 0;
	ulong u32_t, u32_t0;
	U8 u8_loop_cnt = 0;

	u32_err = emmc_ipverify_performance_init();
	if (u32_err != EMMC_ST_SUCCESS)
		goto LABEL_PERF_END;
//===============================================
LABEL_TEST_START:

//----------------------------------------
	emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 0, "Single Write, total: %u KB, %u blocks, burst size: %Xh\n",
		   (EMMC_SPEED_TEST_SINGLE_BLKCNT_W << EMMC_SECTOR_512BYTE_BITS) >> 10,
		   EMMC_SPEED_TEST_SINGLE_BLKCNT_W, 1 << EMMC_SECTOR_512BYTE_BITS);
	u32_t0 = get_timer(0);
	u32_err = emmc_ipverify_performance_write_single_blk(&u32_sector_addr);
	if (u32_err != EMMC_ST_SUCCESS)
		goto LABEL_PERF_END;

	u32_t = get_timer(u32_t0);//ms
	if (u32_t != 0)
		emmc_debug(0, 0, " %u KB/s\n",
			   (U32)(((EMMC_SPEED_TEST_SINGLE_BLKCNT_W << EMMC_SECTOR_512BYTE_BITS) * 1000 / u32_t) >> 10));
	else
		emmc_debug(0, 0, "t = 0\n");

// -----------------------------------------
	emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 0, "Single Read, total: %u KB, %u blocks, burst size: %Xh\n",
		   ((EMMC_SPEED_TEST_SINGLE_BLKCNT_R) << EMMC_SECTOR_512BYTE_BITS) >> 10,
	EMMC_SPEED_TEST_SINGLE_BLKCNT_R, 1 << EMMC_SECTOR_512BYTE_BITS);
	u32_t0 = get_timer(0);
	u32_err = emmc_ipverify_performance_read_single_blk(&u32_sector_addr);
	if (u32_err != EMMC_ST_SUCCESS)
		goto LABEL_PERF_END;

	u32_t = get_timer(u32_t0);//ms
	if (u32_t != 0) {
		emmc_debug(0, 0, " %u KB/s\n",
			   (U32)(((EMMC_SPEED_TEST_SINGLE_BLKCNT_R << 9) * 1000 / u32_t) >> 10));
	} else {
		emmc_debug(0, 0, "t=0\n");
	}
	//==================================
	u32_sector_cnt = 0;
LABEL_MULTI:
	emmc_ipverify_performance_sector_cnt(&u32_sector_cnt);

	u32_sector_addr = EMMC_TEST_BLK_0;
	u32_t0 = get_timer(0);
	for (u32_i = 0; u32_i < EMMC_SPEED_TEST_MULTIPLE_BLKCNT_W;) {
		u32_err = emmc_cmd25_miu(u32_sector_addr, gau8_wbuf, u32_sector_cnt);
		if (u32_err != EMMC_ST_SUCCESS) {
			emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: %Xh\n", u32_err);
			goto LABEL_PERF_END;
		}
		u32_i += u32_sector_cnt;
	}
	emmc_fcie_wait_d0_high(TIME_WAIT_DAT0_HIGH);
	u32_t = get_timer(u32_t0) / 10;//10ms
	if (u32_t != 0)
		emmc_debug(0, 0, " %u.%u MB/s\n",
			   (U32)(((EMMC_SPEED_TEST_MULTIPLE_BLKCNT_W << 9) * 100 / u32_t) >> 20),
			   (U32)((((EMMC_SPEED_TEST_MULTIPLE_BLKCNT_W << 9) * 100 / u32_t) >> 10) * 10 / 1024 % 10));
	else
		emmc_debug(0, 0, "t=0\n");

	//-----------------------------------------
	emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 0, "Multi Read, total: %uMB, burst size: %u KB\n",
		   (EMMC_SPEED_TEST_MULTIPLE_BLKCNT_R << 9) >> 20, u32_sector_cnt >> 1);

	u32_sector_addr = EMMC_TEST_BLK_0;
	u32_t0 = get_timer(0);
	for (u32_i = 0; u32_i < EMMC_SPEED_TEST_MULTIPLE_BLKCNT_R;) {
		u32_err = emmc_cmd18_miu(u32_sector_addr, gau8_rbuf, u32_sector_cnt);
		if (u32_err != EMMC_ST_SUCCESS) {
			emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: %Xh\n", u32_err);
			goto LABEL_PERF_END;
		}
		u32_i += u32_sector_cnt;
	}
	emmc_fcie_wait_d0_high(TIME_WAIT_DAT0_HIGH);
	u32_t = get_timer(u32_t0);//100ms
	if (u32_t != 0)
		emmc_debug(0, 0, " %u.%u MB/s\n",
			   (U32)(((EMMC_SPEED_TEST_MULTIPLE_BLKCNT_R << 9) >> 20) * 1000 / u32_t),
			   (U32)((((EMMC_SPEED_TEST_MULTIPLE_BLKCNT_R << 9) >> 10) * 1000 / u32_t) *10 / 1024 % 10));
	else
		emmc_debug(0, 0, "t=0\n");

	if (u32_sector_cnt != 0x400 * 1024 >> 9)
		goto LABEL_MULTI;

	//===============================================
	if (u8_loop_cnt == 0) {
		u8_loop_cnt++;

		emmc_debug(0, 0, "\n\n");
		//test for next mode
		u32_err = emmc_fcie_choose_speed_mode();
		if (u32_err != EMMC_ST_SUCCESS) {
			emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: ChooseSpeedMode fail: %Xh\n", u32_err);
			goto LABEL_PERF_END;
		}

		goto LABEL_TEST_START;
	}

LABEL_PERF_END:

	if (gau8_wbuf) {
		free(gau8_wbuf);
		gau8_wbuf = 0;
	}

	if (gau8_rbuf) {
		free(gau8_rbuf);
		gau8_rbuf = 0;
	}

	return u32_err;
}

static U32 emmc_ipverify_performance_fde_write_single_blk(U32 *u32_sector_addr)
{
	U32 u32_err = EMMC_ST_SUCCESS, u32_i, sector_addr = 0;

	sector_addr = (*u32_sector_addr);
	for (u32_i = 0; u32_i < EMMC_SPEED_TEST_SINGLE_BLKCNT_W; u32_i++) {
		sector_addr += (emmc_drv.u32_sec_count >> 2) + 333 * u32_i;
		sector_addr %= emmc_drv.u32_sec_count;
		if (sector_addr < emmc_drv.u32_sec_count >> 1)
			sector_addr += emmc_drv.u32_sec_count >> 1;

		sector_addr -= EMMC_SPEED_TEST_SINGLE_BLKCNT_W;
		sector_addr = EMMC_TEST_BLK_0 + (sector_addr % EMMC_TEST_BLK_CNT);

		//emmc_debug(0, 0, "%08Xh\n", u32_sector_addr + u32_i);
		fde_aes_open();
		fde_aes_set_xts_lba(sector_addr);
		fde_aes_set_codec_dir(ENABLE_ENC);
		fde_aes_set_xex_hw_t_calc_kick(1);
		#if EMMC_FEATURE_RELIABLE_WRITE
		u32_err = emmc_cmd24(sector_addr, gau8_wbuf);
		#else
		u32_err = emmc_cmd24_miu(sector_addr, gau8_wbuf);
		#endif
		fde_aes_set_xex_hw_t_calc_kick(0);
		fde_aes_close();

		if (u32_err != EMMC_ST_SUCCESS) {
			emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: %Xh\n", u32_err);
			goto LABEL_END;
		}
	}
	emmc_fcie_wait_d0_high(TIME_WAIT_DAT0_HIGH);

LABEL_END:
	*u32_sector_addr = sector_addr;

	return u32_err;
}

static U32 emmc_ipverify_performance_fde_read_single_blk(U32 *u32_sector_addr)
{
	U32 u32_err = EMMC_ST_SUCCESS, u32_i, sector_addr = 0;

	sector_addr = (*u32_sector_addr);

	for (u32_i = 0; u32_i < EMMC_SPEED_TEST_SINGLE_BLKCNT_R; u32_i++) {
		sector_addr += (emmc_drv.u32_sec_count >> 2) + 333 * u32_i;
		sector_addr %= emmc_drv.u32_sec_count;
		if (sector_addr < emmc_drv.u32_sec_count >> 1)
			sector_addr += emmc_drv.u32_sec_count >> 1;

		sector_addr -= EMMC_SPEED_TEST_SINGLE_BLKCNT_R;
		sector_addr = EMMC_TEST_BLK_0 + (sector_addr % EMMC_TEST_BLK_CNT);
		fde_aes_open();
		fde_aes_set_xts_lba(sector_addr);
		fde_aes_set_codec_dir(ENABLE_DEC);
		fde_aes_set_xex_hw_t_calc_kick(1);

		u32_err = emmc_cmd17_miu(sector_addr, gau8_rbuf);
		fde_aes_set_xex_hw_t_calc_kick(0);
		fde_aes_close();
		if (u32_err != EMMC_ST_SUCCESS) {
			emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: %Xh\n", u32_err);
			goto LABEL_END;
		}
	}
	emmc_fcie_wait_d0_high(TIME_WAIT_DAT0_HIGH);

LABEL_END:
	*u32_sector_addr = sector_addr;

	return u32_err;
}

static U32 emmc_ipverify_performance_fde_write_multi_blk(U32 u32_sector_addr, U32 u32_sector_cnt)
{
	U32 u32_err = EMMC_ST_SUCCESS;

	fde_aes_open();
	fde_aes_set_xts_lba(u32_sector_addr);
	fde_aes_set_codec_dir(ENABLE_ENC);
	fde_aes_set_xex_hw_t_calc_kick(1);
	u32_err = emmc_cmd25_miu(u32_sector_addr, gau8_wbuf, u32_sector_cnt);
	fde_aes_set_xex_hw_t_calc_kick(0);
	fde_aes_close();

	return u32_err;
}

static U32 emmc_ipverify_performance_fde_read_multi_blk(U32 u32_sector_addr, U32 u32_sector_cnt)
{
	U32 u32_err = EMMC_ST_SUCCESS;

	fde_aes_open();
	fde_aes_set_xts_lba(u32_sector_addr);
	fde_aes_set_codec_dir(ENABLE_DEC);
	fde_aes_set_xex_hw_t_calc_kick(1);

	u32_err = emmc_cmd18_miu(u32_sector_addr, gau8_rbuf, u32_sector_cnt);
	fde_aes_set_xex_hw_t_calc_kick(0);
	fde_aes_close();

	return u32_err;
}

U32 emmc_ipverify_performance_fde(void)
{
	U32 u32_err = EMMC_ST_SUCCESS, u32_i, u32_sector_addr = 0, u32_sector_cnt = 0;
	ulong u32_t, u32_t0;

	u32_err = emmc_ipverify_performance_init();
	if (u32_err != EMMC_ST_SUCCESS)
		goto LABEL_PERF_END;

	//===============================================
	//----------------------------------------
	emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 0, "Single Write, total: %u KB, %u blocks, burst size: %Xh\n",
		   (EMMC_SPEED_TEST_SINGLE_BLKCNT_W << EMMC_SECTOR_512BYTE_BITS) >> 10,
		   EMMC_SPEED_TEST_SINGLE_BLKCNT_W, 1 << EMMC_SECTOR_512BYTE_BITS);
	u32_t0 = timer_get_boot_us();
	u32_err = emmc_ipverify_performance_fde_write_single_blk(&u32_sector_addr);
	if (u32_err != EMMC_ST_SUCCESS)
		goto LABEL_PERF_END;

	u32_t = (timer_get_boot_us() - u32_t0);//ms
	if (u32_t != 0)
		emmc_debug(0, 0, " %lu KB/s\n",
			   ((EMMC_SPEED_TEST_SINGLE_BLKCNT_W << 9) >> 10) * 1000 * 1000 / u32_t);
	else
		emmc_debug(0, 0, "t=0\n");

	//-----------------------------------------
	emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 0, "Single Read, total: %u KB, %u blocks, burst size: %Xh\n",
		   ((EMMC_SPEED_TEST_SINGLE_BLKCNT_R) << 9) >> 10,
		   EMMC_SPEED_TEST_SINGLE_BLKCNT_R, 1 << 9);
	u32_t0 = timer_get_boot_us();
	u32_err = emmc_ipverify_performance_fde_read_single_blk(&u32_sector_addr);
	if (u32_err != EMMC_ST_SUCCESS)
		goto LABEL_PERF_END;

	u32_t = timer_get_boot_us() - u32_t0;//ms
	if (u32_t != 0)
		emmc_debug(0, 0, " %lu KB/s\n",
			   ((EMMC_SPEED_TEST_SINGLE_BLKCNT_R << 9) >> 10) * 1000 * 1000 / u32_t);
	else
		emmc_debug(0, 0, "t=0\n");

	//==================================
	u32_sector_cnt = 0;
LABEL_MULTI:
	emmc_ipverify_performance_sector_cnt(&u32_sector_cnt);

	u32_sector_addr = EMMC_TEST_BLK_0;
	u32_t0 = timer_get_boot_us();
	for (u32_i = 0; u32_i < EMMC_SPEED_TEST_MULTIPLE_BLKCNT_W;) {
		u32_err = emmc_ipverify_performance_fde_write_multi_blk(u32_sector_addr, u32_sector_cnt);
		if (u32_err != EMMC_ST_SUCCESS) {
				emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: %Xh\n", u32_err);
				goto LABEL_PERF_END;
		}
		u32_i += u32_sector_cnt;
	}
	emmc_fcie_wait_d0_high(TIME_WAIT_DAT0_HIGH);
	u32_t = timer_get_boot_us() - u32_t0;//10ms
	if (u32_t != 0)
		emmc_debug(0, 0, " %lu.%lu MB/s\n",
			   ((EMMC_SPEED_TEST_MULTIPLE_BLKCNT_W << 9) >> 20) * 1000 * 1000 / u32_t,
			   (((EMMC_SPEED_TEST_MULTIPLE_BLKCNT_W << 9) >> 10) * 1000 * 1000 / u32_t) *10 / 1024 % 10);
	else
		emmc_debug(0, 0, "t=0\n");

//-----------------------------------------
	emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 0, "Multi Read, total: %uMB, burst size: %u KB\n",
		   (EMMC_SPEED_TEST_MULTIPLE_BLKCNT_R << 9) >> 20, u32_sector_cnt >> 1);

	u32_sector_addr = EMMC_TEST_BLK_0;
	u32_t0 = timer_get_boot_us();//get_timer(0);
	for (u32_i = 0; u32_i < EMMC_SPEED_TEST_MULTIPLE_BLKCNT_R;) {
		u32_err = emmc_ipverify_performance_fde_read_multi_blk(u32_sector_addr, u32_sector_cnt);
		if (u32_err != EMMC_ST_SUCCESS) {
			emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: %Xh\n", u32_err);
			goto LABEL_PERF_END;
		}
		u32_i += u32_sector_cnt;
	}
	emmc_fcie_wait_d0_high(TIME_WAIT_DAT0_HIGH);
	u32_t = timer_get_boot_us() - u32_t0;//100ms
	if (u32_t != 0)
		emmc_debug(0, 0, " %lu.%lu MB/s\n",
			   ((EMMC_SPEED_TEST_MULTIPLE_BLKCNT_R << 9) >> 20) * 1000 * 1000 / u32_t,
			   (((EMMC_SPEED_TEST_MULTIPLE_BLKCNT_R << 9) >> 10) * 1000 * 1000 / u32_t) *10 / 1024 % 10);
	else
		emmc_debug(0, 0, "t=0\n");

	if (u32_sector_cnt != 0x400 * 1024 >> 9)
		goto LABEL_MULTI;

LABEL_PERF_END:

	if (gau8_wbuf) {
		free(gau8_wbuf);
		gau8_wbuf = 0;
	}

	if (gau8_rbuf) {
		free(gau8_rbuf);
		gau8_rbuf = 0;
	}

	return u32_err;
}

U8 u8_bus_width[3] = {1, 4, 8};

U32 u32_test_pattern[] = {
	EMMC_PATTERN_00112233,
	EMMC_PATTERN_00332211,
	EMMC_PATTERN_00000000,
	EMMC_PATTERN_FFFFFFFF,
	EMMC_PATTERN_FF00FF00,
	EMMC_PATTERN_00FF00FF,
	EMMC_PATTERN_55555555,
	EMMC_PATTERN_AAAAAAAA,
	EMMC_PATTERN_AA55AA55,
	EMMC_PATTERN_55AA55AA,
	EMMC_PATTERN_A55AA55A,
	EMMC_PATTERN_5AA55AA5
};

static U32 emmc_init_for_test(void)
{
	U32 u32_err;

	emmc_drv.u16_mmc_mode = BIT_SD_DEFAULT_MODE_REG;


	emmc_platform_init();
	u32_err = emmc_fcie_init();
	if (u32_err)
		goto  LABEL_INIT_END;

	emmc_drv.u32_drvflag &= ~DRV_FLAG_INIT_DONE;

	u32_err = emmc_identify();
	if (u32_err)
		return u32_err;

	emmc_clock_setting(FCIE_SLOW_CLK);

	u32_err = emmc_csd_config();
	if (u32_err != EMMC_ST_SUCCESS)
		goto  LABEL_INIT_END;

	u32_err = emmc_cmd3_cmd7(emmc_drv.u16_rca, MMC_CMD_SELECT_CARD);
	if (u32_err != EMMC_ST_SUCCESS)
		goto  LABEL_INIT_END;

	u32_err = emmc_ext_csd_config();
	if (u32_err != EMMC_ST_SUCCESS)
		goto  LABEL_INIT_END;

LABEL_INIT_END:
	emmc_drv.u32_drvflag |= DRV_FLAG_INIT_DONE;

	return u32_err;
}

#if FCIE_BIST_TEST
U32 emmc_test_bist(U8 u8_pat_sel)
{
	U32 u32_err = EMMC_ST_SUCCESS;

	//emmc_debug(0, 1, " mode %d\n", u8_PatSel);

	//Select pattern
	REG_FCIE_SETBIT(GET_REG_ADDR(FCIE_REG_BASE_ADDR, 0x36), (u8_pat_sel & 0xF) << 12);

	//Set BIST Mode
	REG_FCIE_SETBIT(FCIE_TEST_MODE, BIT_BIST_MODE);

	//Do Block Write (CMD24)
	emmc_cmd24(0, gau8_wbuf);

	//Do Block Read (CMD17)
	emmc_cmd17(0, gau8_rbuf);

	//Wait BIST Error Flag
	if ((REG_FCIE(GET_REG_ADDR(FCIE_REG_BASE_ADDR, 0x36)) & BIT0) == BIT0) {
		printf("\033[7;31m: BIST pattern type %d fail!!\033[m\n", u8_pat_sel);
		u32_err = EMMC_ST_ERR_BIST_FAIL;
		return u32_err;
	}

	printf("BIST pattern type %d ok!!\n", u8_pat_sel);

	REG_FCIE_CLRBIT(FCIE_TEST_MODE, BIT_BIST_MODE);

	return u32_err;
}
#endif
U32 emmc_test_power_saving_mode(void)
{
	U32 u32_err = EMMC_ST_SUCCESS;

	//Enable power save test by RIU mode
	REG_FCIE_W(FCIE_PWR_SAVE_CTL, 0x000D);

	/* (1) Clear HW Enable */
	REG_FCIE_W(GET_REG_ADDR(FCIE_POWEER_SAVE_MODE_BASE, 0x00), 0x0000);
	REG_FCIE_W(GET_REG_ADDR(FCIE_POWEER_SAVE_MODE_BASE, 0x01),
		   PWR_BAT_CLASS | PWR_RST_CLASS | PWR_CMD_WREG | PWR_CMD_BK0 | 0x07);

	/* (2) Clear All Interrupt */
	REG_FCIE_W(GET_REG_ADDR(FCIE_POWEER_SAVE_MODE_BASE, 0x02), 0xffff);
	REG_FCIE_W(GET_REG_ADDR(FCIE_POWEER_SAVE_MODE_BASE, 0x03),
		   PWR_BAT_CLASS | PWR_RST_CLASS | PWR_CMD_WREG | PWR_CMD_BK0 | 0x00);

	/* (3) Clear SD MODE Enable */
	REG_FCIE_W(GET_REG_ADDR(FCIE_POWEER_SAVE_MODE_BASE, 0x04), 0x0000);
	REG_FCIE_W(GET_REG_ADDR(FCIE_POWEER_SAVE_MODE_BASE, 0x05),
		   PWR_BAT_CLASS | PWR_RST_CLASS | PWR_CMD_WREG | PWR_CMD_BK0 | 0x0B);

	/* (4) Clear SD CTL Enable */
	REG_FCIE_W(GET_REG_ADDR(FCIE_POWEER_SAVE_MODE_BASE, 0x06), 0x0000);
	REG_FCIE_W(GET_REG_ADDR(FCIE_POWEER_SAVE_MODE_BASE, 0x07),
		   PWR_BAT_CLASS | PWR_RST_CLASS | PWR_CMD_WREG | PWR_CMD_BK0 | 0x0C);

	/* (5) Reset Start */
	REG_FCIE_W(GET_REG_ADDR(FCIE_POWEER_SAVE_MODE_BASE, 0x08), 0x0000);
	REG_FCIE_W(GET_REG_ADDR(FCIE_POWEER_SAVE_MODE_BASE, 0x09),
		   PWR_BAT_CLASS | PWR_RST_CLASS | PWR_CMD_WREG | PWR_CMD_BK0 | 0x3F);

	/* (6) Reset End */
	REG_FCIE_W(GET_REG_ADDR(FCIE_POWEER_SAVE_MODE_BASE, 0x0A), 0x0001);
	REG_FCIE_W(GET_REG_ADDR(FCIE_POWEER_SAVE_MODE_BASE, 0x0B),
		   PWR_BAT_CLASS | PWR_RST_CLASS | PWR_CMD_WREG | PWR_CMD_BK0 | 0x3F);

	/* (7) Set "SD_MOD" */
	REG_FCIE_W(GET_REG_ADDR(FCIE_POWEER_SAVE_MODE_BASE, 0x0C), 0x0021);
	REG_FCIE_W(GET_REG_ADDR(FCIE_POWEER_SAVE_MODE_BASE, 0x0D),
		   PWR_BAT_CLASS | PWR_RST_CLASS | PWR_CMD_WREG | PWR_CMD_BK0 | 0x0B);

	/* (8) Enable "reg_sd_en" */
	REG_FCIE_W(GET_REG_ADDR(FCIE_POWEER_SAVE_MODE_BASE, 0x0E), 0x1001);
	REG_FCIE_W(GET_REG_ADDR(FCIE_POWEER_SAVE_MODE_BASE, 0x0F),
		   PWR_BAT_CLASS | PWR_RST_CLASS | PWR_CMD_WREG | PWR_CMD_BK0 | 0x07);

	/* (9) Command Content, IDLE */
	REG_FCIE_W(GET_REG_ADDR(FCIE_POWEER_SAVE_MODE_BASE, 0x10), 0x0040);
	REG_FCIE_W(GET_REG_ADDR(FCIE_POWEER_SAVE_MODE_BASE, 0x11),
		   PWR_BAT_CLASS | PWR_RST_CLASS | PWR_CMD_WREG | PWR_CMD_BK0 | 0x20);

	/* (10) Command Content, STOP */
	REG_FCIE_W(GET_REG_ADDR(FCIE_POWEER_SAVE_MODE_BASE, 0x12), 0x0000);
	REG_FCIE_W(GET_REG_ADDR(FCIE_POWEER_SAVE_MODE_BASE, 0x13),
		   PWR_BAT_CLASS | PWR_RST_CLASS | PWR_CMD_WREG | PWR_CMD_BK0 | 0x21);

	/* (11) Command Content, STOP */
	REG_FCIE_W(GET_REG_ADDR(FCIE_POWEER_SAVE_MODE_BASE, 0x14), 0x0000);
	REG_FCIE_W(GET_REG_ADDR(FCIE_POWEER_SAVE_MODE_BASE, 0x15),
		   PWR_BAT_CLASS | PWR_RST_CLASS | PWR_CMD_WREG | PWR_CMD_BK0 | 0x22);

	/* (12) Command & Response Size */
	REG_FCIE_W(GET_REG_ADDR(FCIE_POWEER_SAVE_MODE_BASE, 0x16), 0x0500);
	REG_FCIE_W(GET_REG_ADDR(FCIE_POWEER_SAVE_MODE_BASE, 0x17),
		   PWR_BAT_CLASS | PWR_RST_CLASS | PWR_CMD_WREG | PWR_CMD_BK0 | 0x0A);

	/* (13) Enable Interrupt, SD_CMD_END */
	REG_FCIE_W(GET_REG_ADDR(FCIE_POWEER_SAVE_MODE_BASE, 0x18), 0x0002);
	REG_FCIE_W(GET_REG_ADDR(FCIE_POWEER_SAVE_MODE_BASE, 0x19),
		   PWR_BAT_CLASS | PWR_RST_CLASS | PWR_CMD_WREG | PWR_CMD_BK0 | 0x01);

	/* (14) Command Enable + job Start */
	REG_FCIE_W(GET_REG_ADDR(FCIE_POWEER_SAVE_MODE_BASE, 0x1A), 0x0044);
	REG_FCIE_W(GET_REG_ADDR(FCIE_POWEER_SAVE_MODE_BASE, 0x1B),
		   PWR_BAT_CLASS | PWR_RST_CLASS | PWR_CMD_WREG | PWR_CMD_BK0 | 0x0C);

	/* (15) Wait Interrupt */
	REG_FCIE_W(GET_REG_ADDR(FCIE_POWEER_SAVE_MODE_BASE, 0x1C), 0x0000);
	REG_FCIE_W(GET_REG_ADDR(FCIE_POWEER_SAVE_MODE_BASE, 0x1D),
		   PWR_BAT_CLASS | PWR_RST_CLASS | PWR_CMD_WINT);

	/* (16) STOP */
	REG_FCIE_W(GET_REG_ADDR(FCIE_POWEER_SAVE_MODE_BASE, 0x1E), 0x0000);
	REG_FCIE_W(GET_REG_ADDR(FCIE_POWEER_SAVE_MODE_BASE, 0x1F),
		   PWR_BAT_CLASS | PWR_RST_CLASS | PWR_CMD_STOP);

	emmc_debug(0, 1, "1s\n");
	mdelay(1000);
	emmc_debug(0, 1, "1s\n");
	mdelay(1000);
	emmc_debug(0, 1, "1s\n");
	mdelay(1000);
	emmc_debug(0, 1, "1s\n");
	mdelay(1000);
	emmc_debug(0, 1, "1s\n");
	mdelay(1000);

	//Trigger power save mode
	REG_FCIE_W(FCIE_PWR_SAVE_CTL, 0x000F);

	u32_err = emmc_fcie_wait_events(FCIE_PWR_SAVE_CTL,
					BIT_POWER_SAVE_MODE_INT | BIT_RIU_SAVE_EVENT, TIME_WAIT_CMDRSP_END);
	if (u32_err != EMMC_ST_SUCCESS) {
		emmc_debug(0, 1, "Power saving mode fail\n");
		return u32_err;
	}

	emmc_debug(0, 1, "Power saving mode ok\n");

	return 0;
}

#if FCIE_RSP_SHIFT_TUNING_TEST

U32 emmc_test_rsp_shift_tuning_ex(U32 u32_emmc_addr, U8 *pu8_w, U8 *pu8_r)
{
	U32 u32_err = EMMC_ST_SUCCESS;
	U8  u8_i;
	U8  u8_RSP_test_result[16] = {0};

	u32_err = emmc_cmd24_miu(u32_emmc_addr, pu8_w);
	if (u32_err != EMMC_ST_SUCCESS) {
		emmc_debug(0, 1, "Err, SingleBlkWRC W, %Xh\n", u32_err);
		return EMMC_ST_ERR_CMD24_MIU;
	}

	EMMC_DISABLE_RETRY(1);

	for (u8_i = 0; u8_i < 16; u8_i++) {
		REG_FCIE_CLRBIT(FCIE_RSP_SHIFT_CNT, BIT_RSP_SHIFT_TUNE_MASK);
		REG_FCIE_SETBIT(FCIE_RSP_SHIFT_CNT, BIT_RSP_SHIFT_SEL | (u8_i & 0xF));

		memset(pu8_r, 0, 512);

		EMMC_DISABLE_LOG(1);
		u32_err = emmc_cmd17_miu(u32_emmc_addr, pu8_r);
		EMMC_DISABLE_LOG(0);
		if (u32_err != EMMC_ST_SUCCESS) {
			emmc_debug(0, 1, "RSP shift tuning \033[7;35m%d is not ok\033[m\n", u8_i);
			u8_RSP_test_result[u8_i] = 0;
			continue;
		}

		u32_err = emmc_compare_data(pu8_w, pu8_r, EMMC_SECTOR_BYTECNT);
		if (u32_err != EMMC_ST_SUCCESS) {
			emmc_debug(0, 1, "Err, SingleBlkWRC C, %Xh\n", u32_err);
			return EMMC_ST_ERR_DATA_MISMATCH;
		}

		emmc_debug(0, 1, "RSP shift tuning \033[7;34m%d is ok\033[m\n", u8_i);
		u8_RSP_test_result[u8_i] = 1;
	}

	REG_FCIE_CLRBIT(FCIE_RSP_SHIFT_CNT, BIT_RSP_SHIFT_SEL);

	EMMC_DISABLE_RETRY(0);

	for (u8_i = 0; u8_i < 16; u8_i++) {
		if (u8_RSP_test_result[u8_i])
			return EMMC_ST_SUCCESS;
	}

	return u32_err;
}

U32 emmc_test_rsp_shift_tuning(U32 u32_emmc_addr, U32 u32_data_pattern)
{
	U32 u32_i, *pu32_w = (U32 *)gau8_wbuf, *pu32_r = (U32 *)gau8_rbuf;

	emmc_debug(0, 1, "Data pattern %08X\n", u32_data_pattern);

	//init data pattern
	if (u32_data_pattern == EMMC_PATTERN_00112233 || u32_data_pattern == EMMC_PATTERN_00332211) {
		for (u32_i = 0; u32_i < TEST_BUFFER_SIZE; u32_i++) {
			if (u32_data_pattern == EMMC_PATTERN_00112233)
				gau8_wbuf[u32_i] = u32_i & 0xFF;
			else
				gau8_wbuf[u32_i] = 0xFF - (u32_i & 0xFF);

			gau8_rbuf[u32_i] = ~gau8_wbuf[u32_i];
		}
	} else {
		for (u32_i = 0; u32_i < (TEST_BUFFER_SIZE >> 2); u32_i++) {
			pu32_w[u32_i] = u32_data_pattern;
			pu32_r[u32_i] = ~pu32_w[u32_i];
		}
	}

	return emmc_test_rsp_shift_tuning_ex(u32_emmc_addr, (U8 *)pu32_w, (U8 *)pu32_r);
}
#endif

U32 emmc_test_wcrc_shift_tuning_ex(U32 u32_emmc_addr, U8 *pu8_w, U8 *pu8_r)
{
	U32 u32_err;
	U8  u8_i;
	U8  u8_WCRC_test_result[16] = {0};

	for (u8_i = 0; u8_i < 16; u8_i++) {
		EMMC_DISABLE_RETRY(1);

		//emmc_debug(0, 0, "\033[7;35m1 FCIE_RX_SHIFT_CNT=%04X\033[m\n", REG_FCIE(FCIE_RX_SHIFT_CNT));
		REG_FCIE_CLRBIT(FCIE_RX_SHIFT_CNT, BIT_WRSTS_SHIFT_TUNE_MASK);
		REG_FCIE_SETBIT(FCIE_RX_SHIFT_CNT, BIT_WRSTS_SHIFT_SEL | ((u8_i & 0xF) << 8));
		//emmc_debug(0, 0, "\033[7;36m2 FCIE_RX_SHIFT_CNT=%04X\033[m\n", REG_FCIE(FCIE_RX_SHIFT_CNT));

		EMMC_DISABLE_LOG(1);
		u32_err = emmc_cmd24_miu(u32_emmc_addr, pu8_w);
		EMMC_DISABLE_LOG(0);
		if (u32_err != EMMC_ST_SUCCESS) {
			emmc_debug(0, 1, "WCRC shift tuning \033[7;35m%d is not ok\033[m\n", u8_i);
			continue;
		}

		EMMC_DISABLE_RETRY(0);

		memset(pu8_r, 0, 512);

		u32_err = emmc_cmd17_miu(u32_emmc_addr, pu8_r);
		if (u32_err != EMMC_ST_SUCCESS) {
			emmc_debug(0, 1, "Err, SingleBlkWRC R, %Xh\n", u32_err);
			return u32_err;
		}

		u32_err = emmc_compare_data(pu8_w, pu8_r, EMMC_SECTOR_BYTECNT);
		if (u32_err != EMMC_ST_SUCCESS) {
			emmc_debug(0, 1, "Err, SingleBlkWRC C, %Xh\n", u32_err);
			return u32_err;
		}

		emmc_debug(0, 1, "WCRC shift tuning \033[7;34m%d is ok\033[m\n", u8_i);

		u8_WCRC_test_result[u8_i] = 1;
	}

	REG_FCIE_CLRBIT(FCIE_RX_SHIFT_CNT, BIT_WRSTS_SHIFT_SEL);

	EMMC_DISABLE_RETRY(0);

	for (u8_i = 0; u8_i < 16; u8_i++) {
		if (u8_WCRC_test_result[u8_i])
			return EMMC_ST_SUCCESS;
	}

	return u32_err;
}

#if FCIE_WCRC_SHIFT_TUNING_TEST
U32 emmc_test_wcrc_shift_tuning(U32 u32_emmc_addr, U32 u32_data_pattern)
{
	U32 u32_i, *pu32_w = (U32 *)gau8_wbuf, *pu32_r = (U32 *)gau8_rbuf;

	emmc_debug(0, 1, "Data pattern %08X\n", u32_data_pattern);

	//init data pattern
	if (u32_data_pattern == EMMC_PATTERN_00112233 || u32_data_pattern == EMMC_PATTERN_00332211) {
		for (u32_i = 0; u32_i < TEST_BUFFER_SIZE; u32_i++) {
			if (u32_data_pattern == EMMC_PATTERN_00112233)
				gau8_wbuf[u32_i] = u32_i & 0xFF;
			else
				gau8_wbuf[u32_i] = 0xFF - (u32_i & 0xFF);

			gau8_rbuf[u32_i] = ~gau8_wbuf[u32_i];
		}
	} else {
		for (u32_i = 0; u32_i < TEST_BUFFER_SIZE >> 2; u32_i++) {
			pu32_w[u32_i] = u32_data_pattern;
			pu32_r[u32_i] = ~pu32_w[u32_i];
		}
	}

	return emmc_test_wcrc_shift_tuning_ex(u32_emmc_addr, (U8 *)pu32_w, (U8 *)pu32_r);
}
#endif
#if FCIE_RSTOP_SHIFT_TUNING_TEST
U32 emmc_test_rstop_shift_tuning_ex(U32 u32_emmc_addr, U8 *pu8_w, U8 *pu8_r)
{
	U32 u32_err;
	U8  u8_i = 15;
	U8  u8_RSTOP_test_result[16] = {0};

	u32_err = emmc_cmd24_miu(u32_emmc_addr, pu8_w);
	if (u32_err != EMMC_ST_SUCCESS) {
		emmc_debug(0, 1, "Err, SingleBlkWRC W, %Xh\n", u32_err);
		return u32_err;
	}

	EMMC_DISABLE_RETRY(1);

	for (u8_i = 0; u8_i < 16; u8_i++) {
		REG_FCIE_CLRBIT(FCIE_RX_SHIFT_CNT, BIT_RSTOP_SHIFT_TUNE_MASK);
		REG_FCIE_SETBIT(FCIE_RX_SHIFT_CNT, BIT_RSTOP_SHIFT_SEL | u8_i);

		memset(pu8_r, 0, 512);

		EMMC_DISABLE_LOG(1);
		u32_err = emmc_cmd17_miu(u32_emmc_addr, pu8_r);
		EMMC_DISABLE_LOG(0);
		if (u32_err != EMMC_ST_SUCCESS) {
			emmc_debug(0, 1, "RStop shift tuning \033[7;35m%d is not ok\033[m\n", u8_i);
			continue;
		}

		u32_err = emmc_compare_data(pu8_w, pu8_r, EMMC_SECTOR_BYTECNT);
		if (u32_err != EMMC_ST_SUCCESS) {
			emmc_debug(0, 1, "Err, SingleBlkWRC C, %Xh\n", u32_err);
			return u32_err;
		}

		emmc_debug(0, 1, "RStop shift tuning \033[7;34m%d is ok!!!\033[m\n", u8_i);

		u8_RSTOP_test_result[u8_i] = 1;
	}

	REG_FCIE_CLRBIT(FCIE_RX_SHIFT_CNT, BIT_RSTOP_SHIFT_SEL);
	EMMC_DISABLE_RETRY(0);

	for (u8_i = 0; u8_i < 16; u8_i++) {
		if (u8_RSTOP_test_result[u8_i])
			return EMMC_ST_SUCCESS;
	}

	return u32_err;
}

U32 emmc_test_rstop_shift_tuning(U32 u32_emmc_addr, U32 u32_data_pattern)
{
	U32 u32_i, *pu32_w = (U32 *)gau8_wbuf, *pu32_r = (U32 *)gau8_rbuf;

	emmc_debug(0, 1, "Data pattern %08X\n", u32_data_pattern);

	//init data pattern
	if (u32_data_pattern == EMMC_PATTERN_00112233 || u32_data_pattern == EMMC_PATTERN_00332211) {
		for (u32_i = 0; u32_i < TEST_BUFFER_SIZE; u32_i++) {
			if (u32_data_pattern == EMMC_PATTERN_00112233)
				gau8_wbuf[u32_i] = u32_i & 0xFF;
			else
				gau8_wbuf[u32_i] = 0xFF - (u32_i & 0xFF);

			gau8_rbuf[u32_i] = ~gau8_wbuf[u32_i];
		}
	} else {
		for (u32_i = 0; u32_i < TEST_BUFFER_SIZE >> 2; u32_i++) {
			pu32_w[u32_i] = u32_data_pattern;
			pu32_r[u32_i] = ~pu32_w[u32_i];
		}
	}

	return emmc_test_rstop_shift_tuning_ex(u32_emmc_addr, (U8 *)pu32_w, (U8 *)pu32_r);
}
#endif

#if FCIE_READ_CRC_ERROR_INT_TEST
U32 emmc_test_read_crc_err(U32 u32_emmc_blk_addr, U8 *pu8_data_buf)
{
	U32 u32_err, u32_arg;
	U16 u16_mode, u16_ctrl, u16_reg;
	unsigned long dma_addr;

	u16_ctrl = BIT_SD_CMD_EN | BIT_SD_RSP_EN | BIT_SD_DTRX_EN | BIT_ERR_DET_ON;
	u32_arg =  u32_emmc_blk_addr << (emmc_drv.u8_if_sector_mode ? 0 : 9);
	u16_mode = emmc_drv.u16_mmc_mode | emmc_drv.u8_bus_width;

	emmc_fcie_clear_events();
	REG_FCIE_W(FCIE_JOB_BL_CNT, 1);
	dma_addr = emmc_translate_dma_address_ex((unsigned long)pu8_data_buf, EMMC_SECTOR_512BYTE);

	REG_FCIE_W(FCIE_MIU_DMA_ADDR_15_0, dma_addr & 0xFFFF);
	REG_FCIE_W(FCIE_MIU_DMA_ADDR_31_16, dma_addr >> 16);
	if (sizeof(unsigned long) == 8)
		REG_FCIE_W(FCIE_MIU_DMA_ADDR_47_32, dma_addr >> 32);

	REG_FCIE_W(FCIE_MIU_DMA_LEN_15_0, (EMMC_SECTOR_512BYTE + 1) & 0xFFFF);
	REG_FCIE_W(FCIE_MIU_DMA_LEN_31_16, (EMMC_SECTOR_512BYTE + 1) >> 16);
	REG_FCIE_W(FCIE_BLK_SIZE, EMMC_SECTOR_512BYTE + 1);

	//#if 0
	//emmc_debug(0, 1, "1s\n");
	//mdelay(1000);
	//emmc_debug(0, 1, "1s\n");
	//mdelay(1000);
	//emmc_debug(0, 1, "1s\n");
	//mdelay(1000);
	//emmc_debug(0, 1, "1s\n");
	//mdelay(1000);
	//emmc_debug(0, 1, "1s\n");
	//mdelay(1000);
	//#endif

	u32_err = emmc_fcie_send_cmd(u16_mode, u16_ctrl, u32_arg, 17, EMMC_R1_BYTE_CNT);
	if (u32_err != EMMC_ST_SUCCESS) {
		emmc_fcie_err_handler_reinit();
		goto LABEL_END;
	}

	//-------------------------------
	//check FCIE
	u32_err = emmc_fcie_wait_events(FCIE_MIE_EVENT, BIT_DMA_END, TIME_WAIT_1_BLK_END);
	REG_FCIE_R(FCIE_SD_STATUS, u16_reg);
	if (u32_err != EMMC_ST_SUCCESS || (u16_reg & BIT_SD_FCIE_ERR_FLAGS)) {
		u32_err = EMMC_ST_ERR_CMD17_MIU;
		emmc_fcie_err_handler_reinit();
		goto LABEL_END;
	}

	//-------------------------------
	//check device
	u32_err = emmc_check_r1_error();
	if (u32_err != EMMC_ST_SUCCESS) {
		u32_err = EMMC_ST_ERR_CMD17_MIU;
		emmc_fcie_err_handler_reinit();
		goto LABEL_END;
	}

LABEL_END:

	EMMC_FCIE_CLK_DIS();
	return u32_err;
}
#endif
#if FCIE_WRITE_CRC_ERROR_INT_TEST
U32 emmc_test_write_crc_err(U32 u32_emmc_blk_addr, U8 *pu8_data_buf)
{
	U32 u32_err, u32_arg;
	U16 u16_mode, u16_ctrl, u16_reg;
	unsigned long dma_addr;

	//-------------------------------
	//send cmd
	u16_ctrl = BIT_SD_CMD_EN | BIT_SD_RSP_EN;
	u32_arg =  u32_emmc_blk_addr << (emmc_drv.u8_if_sector_mode ? 0 : 9);
	u16_mode = emmc_drv.u16_mmc_mode | emmc_drv.u8_bus_width;

	emmc_fcie_clear_events();
	REG_FCIE_W(FCIE_JOB_BL_CNT, 1);
	dma_addr = emmc_translate_dma_address_ex((unsigned long)pu8_data_buf, EMMC_SECTOR_512BYTE);

	REG_FCIE_W(FCIE_MIU_DMA_ADDR_15_0, dma_addr & 0xFFFF);
	REG_FCIE_W(FCIE_MIU_DMA_ADDR_31_16, dma_addr >> 16);
	if (sizeof(unsigned long) == 8)
		REG_FCIE_W(FCIE_MIU_DMA_ADDR_47_32, dma_addr >> 32);

	REG_FCIE_W(FCIE_MIU_DMA_LEN_15_0, (EMMC_SECTOR_512BYTE - 1) & 0xFFFF);
	REG_FCIE_W(FCIE_MIU_DMA_LEN_31_16, (EMMC_SECTOR_512BYTE - 1) >> 16);
	REG_FCIE_W(FCIE_BLK_SIZE, EMMC_SECTOR_512BYTE - 1);

	//#if 0
	//emmc_debug(0, 1, "1s\n");
	//mdelay(1000);
	//emmc_debug(0, 1, "1s\n");
	//mdelay(1000);
	//emmc_debug(0, 1, "1s\n");
	//mdelay(1000);
	//emmc_debug(0, 1, "1s\n");
	//mdelay(1000);
	//emmc_debug(0, 1, "1s\n");
	//mdelay(1000);
	//#endif

	u32_err = emmc_fcie_send_cmd(u16_mode, u16_ctrl, u32_arg, 24, EMMC_R1_BYTE_CNT);
	if (u32_err != EMMC_ST_SUCCESS) {
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: CMD24 %Xh, Arg: %Xh\n", u32_err, u32_arg);
		emmc_fcie_err_handler_reinit();
		goto LABEL_END;
	}

	//-------------------------------
	//check device
	u32_err = emmc_check_r1_error();
	if (u32_err != EMMC_ST_SUCCESS) {
		u32_err = EMMC_ST_ERR_CMD24_MIU_CHK_R1;
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
			   "eMMC Err: CMD24 check R1 error: %Xh, Arg: %Xh\n", u32_err, u32_arg);
		emmc_fcie_err_handler_reinit();
		goto LABEL_END;
	}

	//-------------------------------
	//send data
	u32_err = emmc_fcie_wait_d0_high(TIME_WAIT_DAT0_HIGH);
	if (u32_err != EMMC_ST_SUCCESS) {
		u32_err = EMMC_ST_ERR_CMD24_MIU_WAIT_D0H;
		goto LABEL_END;
	}

	REG_FCIE_W(FCIE_SD_CTRL, BIT_SD_DTRX_EN | BIT_SD_DAT_DIR_W | BIT_ERR_DET_ON);
	REG_FCIE_SETBIT(FCIE_SD_CTRL, BIT_JOB_START);

	u32_err = emmc_fcie_wait_events(FCIE_MIE_EVENT, BIT_DMA_END, TIME_WAIT_1_BLK_END);

	REG_FCIE_R(FCIE_SD_STATUS, u16_reg);
	if (u32_err != EMMC_ST_SUCCESS || (u16_reg & (BIT_SD_W_FAIL | BIT_SD_W_CRC_ERR))) {
		u32_err = EMMC_ST_ERR_CMD24_MIU;
		emmc_fcie_err_handler_reinit();
		goto LABEL_END;
	}

LABEL_END:
	EMMC_FCIE_CLK_DIS();
	return u32_err;
}
#endif

#if FCIE_WRITE_TIMEOUT_INT_TEST
U32 emmc_test_write_timeout_err(U32 u32_emmc_blk_addr, U8 *pu8_data_buf, U16 u16_blk_cnt)
{
	U32 u32_err, u32_arg;
	U16 u16_mode, u16_ctrl, u16_reg;
	unsigned long dma_addr;

	//-------------------------------
	//send cmd
	u16_ctrl = BIT_SD_CMD_EN | BIT_SD_RSP_EN;
	u16_mode = emmc_drv.u16_mmc_mode | emmc_drv.u8_bus_width;
	u32_arg =  u32_emmc_blk_addr << (emmc_drv.u8_if_sector_mode ? 0 : 9);

	emmc_fcie_clear_events();

	REG_FCIE_W(FCIE_JOB_BL_CNT, u16_blk_cnt);
	dma_addr = emmc_translate_dma_address_ex((unsigned long)pu8_data_buf, 512 * u16_blk_cnt);

	REG_FCIE_W(FCIE_MIU_DMA_ADDR_15_0, dma_addr & 0xFFFF);
	REG_FCIE_W(FCIE_MIU_DMA_ADDR_31_16, dma_addr >> 16);
	if (sizeof(unsigned long) == 8)
		REG_FCIE_W(FCIE_MIU_DMA_ADDR_47_32, dma_addr >> 32);

	REG_FCIE_W(FCIE_MIU_DMA_LEN_15_0, (EMMC_SECTOR_512BYTE) & 0xFFFF);
	REG_FCIE_W(FCIE_MIU_DMA_LEN_31_16, (EMMC_SECTOR_512BYTE) >> 16);

	//#if 0
	//emmc_debug(0, 1, "1s\n");
	//mdelay(1000);
	//emmc_debug(0, 1, "1s\n");
	//mdelay(1000);
	//emmc_debug(0, 1, "1s\n");
	//mdelay(1000);
	//emmc_debug(0, 1, "1s\n");
	//mdelay(1000);
	//emmc_debug(0, 1, "1s\n");
	//mdelay(1000);
	//#endif

	u32_err = emmc_fcie_send_cmd(u16_mode, u16_ctrl, u32_arg, 25, EMMC_R1_BYTE_CNT);
	if (u32_err != EMMC_ST_SUCCESS) {
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: CMD25 %Xh, Arg: %Xh\n", u32_err, u32_arg);
		emmc_fcie_err_handler_reinit();
		goto LABEL_END;
	}

	//-------------------------------
	//check device
	u32_err = emmc_check_r1_error();
	if (u32_err != EMMC_ST_SUCCESS) {
		u32_err = EMMC_ST_ERR_CMD25_CHK_R1;
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
			   "eMMC Err: CMD25 check R1 error: %Xh, Arg: %Xh\n", u32_err, u32_arg);
		emmc_fcie_err_handler_reinit();
		goto LABEL_END;
	}

	//-------------------------------
	//send data
	u32_err = emmc_fcie_wait_d0_high(TIME_WAIT_DAT0_HIGH);
	if (u32_err != EMMC_ST_SUCCESS) {
		u32_err = EMMC_ST_ERR_CMD25_WAIT_D0H;
		goto LABEL_END;
	}

	//#if 0
	//emmc_debug(0, 1, "1s\n");
	//mdelay(1000);
	//emmc_debug(0, 1, "1s\n");
	//mdelay(1000);
	//emmc_debug(0, 1, "1s\n");
	//mdelay(1000);
	//emmc_debug(0, 1, "1s\n");
	//mdelay(1000);
	//emmc_debug(0, 1, "1s\n");
	//mdelay(1000);
	//#endif

	REG_FCIE_W(FCIE_WR_SBIT_TIMER, BIT_WR_SBIT_TIMER_EN | 0x01);
	REG_FCIE_W(FCIE_SD_CTRL, BIT_ERR_DET_ON);
	REG_FCIE_SETBIT(FCIE_SD_CTRL, BIT_SD_DTRX_EN | BIT_SD_DAT_DIR_W);
	REG_FCIE_SETBIT(FCIE_SD_CTRL, BIT_JOB_START);

	u32_err = emmc_fcie_wait_events(FCIE_MIE_EVENT, BIT_DMA_END, HW_TIMER_DELAY_1s * 10);

	REG_FCIE_R(FCIE_SD_STATUS, u16_reg);
	if (u32_err != EMMC_ST_SUCCESS || (u16_reg & (BIT_SD_W_FAIL | BIT_SD_W_CRC_ERR))) {
		u32_err = EMMC_ST_ERR_CMD25;
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
			   "eMMC Err: CMD25 SD_STS: %04Xh, Err: %Xh, Arg: %Xh\n", u16_reg, u32_err, u32_arg);
		emmc_fcie_err_handler_reinit();
		return u32_err;
	}

LABEL_END:
	if (u32_err == EMMC_ST_SUCCESS) {
		if (emmc_cmd12(emmc_drv.u16_rca) != EMMC_ST_SUCCESS)
			emmc_cmd12_no_check(emmc_drv.u16_rca);
	}

	EMMC_FCIE_CLK_DIS();
	return u32_err;
}
#endif

#if FCIE_READ_TIMEOUT_INT_TEST
U32 emmc_test_read_timeout_err(U32 u32_arg)
{
	U32 u32_err;
	U16 u16_ctrl;
	unsigned long dma_addr;

	emmc_fcie_clear_events();

	u16_ctrl = BIT_SD_CMD_EN | BIT_SD_DTRX_EN | BIT_ERR_DET_ON;
	REG_FCIE_W(FCIE_JOB_BL_CNT, 1);
	dma_addr = emmc_translate_dma_address_ex((unsigned long)DMA_R_ADDR, EMMC_SECTOR_512BYTE);

	REG_FCIE_W(FCIE_MIU_DMA_ADDR_15_0, dma_addr & 0xFFFF);
	REG_FCIE_W(FCIE_MIU_DMA_ADDR_31_16, dma_addr >> 16);
	if (sizeof(unsigned long) == 8)
		REG_FCIE_W(FCIE_MIU_DMA_ADDR_47_32, dma_addr >> 32);

	REG_FCIE_W(FCIE_MIU_DMA_LEN_15_0, EMMC_SECTOR_512BYTE & 0xFFFF);
	REG_FCIE_W(FCIE_MIU_DMA_LEN_31_16, EMMC_SECTOR_512BYTE >> 16);
	REG_FCIE_W(FCIE_RD_SBIT_TIMER, BIT_RD_SBIT_TIMER_EN | 0x1);

	//#if 0
	//emmc_debug(0, 1, "1s\n");
	//mdelay(1000);
	//emmc_debug(0, 1, "1s\n");
	//mdelay(1000);
	//emmc_debug(0, 1, "1s\n");
	//mdelay(1000);
	//emmc_debug(0, 1, "1s\n");
	//mdelay(1000);
	//emmc_debug(0, 1, "1s\n");
	//mdelay(1000);
	//#endif

	REG_FCIE_CLRBIT(FCIE_CMD_RSP_SIZE, BIT_RSP_SIZE_MASK);
	REG_FCIE_W(FCIE_SD_MODE, emmc_drv.u16_mmc_mode);

	REG_FCIE_W(FCIE_CMDFIFO_ADDR(0), 0x40);
	REG_FCIE_W(FCIE_CMDFIFO_ADDR(1), 0);
	REG_FCIE_W(FCIE_CMDFIFO_ADDR(2), 0);

	u32_err = emmc_fcie_wait_d0_high(TIME_WAIT_DAT0_HIGH);
	if (u32_err != EMMC_ST_SUCCESS)
		goto LABEL_END;

	REG_FCIE_W(FCIE_SD_CTRL, u16_ctrl);
	REG_FCIE_SETBIT(FCIE_SD_CTRL, BIT_JOB_START);

	//wait event
	u32_err = emmc_fcie_wait_events(FCIE_MIE_EVENT, BIT_CMD_END, TIME_WAIT_CMDRSP_END * 2);
	if (u32_err != EMMC_ST_SUCCESS) {
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: CMD0 %Xh\n", u32_err);
		emmc_fcie_err_handler_reinit();
	}

LABEL_END:
	EMMC_FCIE_CLK_DIS();

	return u32_err;
}
#endif
#if FCIE_CMD_NO_RESPONSE_INT_TEST
U32 emmc_test_cmd_no_rsp_err(U32 u32_arg)
{
	U32 u32_err;
	U16 u16_ctrl;

	emmc_fcie_clear_events();

	u16_ctrl = BIT_SD_CMD_EN | BIT_SD_RSP_EN | BIT_ERR_DET_ON;

	//#if 0
	//emmc_debug(0, 1, "1s\n");
	//mdelay(1000);
	//emmc_debug(0, 1, "1s\n");
	//mdelay(1000);
	//emmc_debug(0, 1, "1s\n");
	//mdelay(1000);
	//emmc_debug(0, 1, "1s\n");
	//mdelay(1000);
	//emmc_debug(0, 1, "1s\n");
	//mdelay(1000);
	//#endif

	u32_err = emmc_fcie_send_cmd(emmc_drv.u16_mmc_mode, u16_ctrl, u32_arg, 0, 0);
	if (u32_err != EMMC_ST_SUCCESS)
		emmc_fcie_err_handler_reinit();

	EMMC_FCIE_CLK_DIS();
	return u32_err;
}
#endif
#if FCIE_CMD_RSP_CRC_ERROR_INT_TEST
U32 emmc_test_cmd_rsp_crc_err(U32 u32_emmc_blk_addr, U8 *pu8_data_buf)
{
	U32 u32_err, u32_arg;
	U16 u16_mode, u16_ctrl, u16_reg;
	unsigned long dma_addr;

	u16_ctrl = BIT_SD_CMD_EN | BIT_SD_RSP_EN | BIT_SD_DTRX_EN | BIT_ERR_DET_ON;
	u32_arg =  u32_emmc_blk_addr << (emmc_drv.u8_if_sector_mode ? 0 : 9);
	u16_mode = emmc_drv.u16_mmc_mode | emmc_drv.u8_bus_width;

	emmc_fcie_clear_events();
	REG_FCIE_W(FCIE_JOB_BL_CNT, 1);
	dma_addr = emmc_translate_dma_address_ex((unsigned long)pu8_data_buf, EMMC_SECTOR_512BYTE);

	REG_FCIE_W(FCIE_MIU_DMA_ADDR_15_0, dma_addr & 0xFFFF);
	REG_FCIE_W(FCIE_MIU_DMA_ADDR_31_16, dma_addr >> 16);
	if (sizeof(unsigned long) == 8)
		REG_FCIE_W(FCIE_MIU_DMA_ADDR_47_32, dma_addr >> 32);

	REG_FCIE_W(FCIE_MIU_DMA_LEN_15_0, EMMC_SECTOR_512BYTE & 0xFFFF);
	REG_FCIE_W(FCIE_MIU_DMA_LEN_31_16, EMMC_SECTOR_512BYTE >> 16);
	REG_FCIE_W(FCIE_BLK_SIZE,  EMMC_SECTOR_512BYTE);

	//#if 0
	//emmc_debug(0, 1, "1s\n");
	//mdelay(1000);
	//emmc_debug(0, 1, "1s\n");
	//mdelay(1000);
	//emmc_debug(0, 1, "1s\n");
	//mdelay(1000);
	//emmc_debug(0, 1, "1s\n");
	//mdelay(1000);
	//emmc_debug(0, 1, "1s\n");
	//mdelay(1000);
	//#endif

	REG_FCIE_CLRBIT(FCIE_CMD_RSP_SIZE, BIT_RSP_SIZE_MASK);
	REG_FCIE_SETBIT(FCIE_CMD_RSP_SIZE, (EMMC_R1_BYTE_CNT + 1) & BIT_RSP_SIZE_MASK);
	REG_FCIE_W(FCIE_SD_MODE, u16_mode);

	REG_FCIE_W(FCIE_CMDFIFO_ADDR(0), ((u32_arg >> 24) << 8) | (0x40 | 17));
	REG_FCIE_W(FCIE_CMDFIFO_ADDR(1), (u32_arg & 0xFF00) | ((u32_arg >> 16) & 0xFF));
	REG_FCIE_W(FCIE_CMDFIFO_ADDR(2), u32_arg & 0xFF);

	u32_err = emmc_fcie_wait_d0_high(TIME_WAIT_DAT0_HIGH);
	if (u32_err != EMMC_ST_SUCCESS)
		goto LABEL_END;

	REG_FCIE_W(FCIE_SD_CTRL, u16_ctrl);
	REG_FCIE_SETBIT(FCIE_SD_CTRL, BIT_JOB_START);

	//wait event
	u32_err = emmc_fcie_wait_events(FCIE_MIE_EVENT, BIT_CMD_END, TIME_WAIT_CMDRSP_END);

	if (u32_err != EMMC_ST_SUCCESS) {
		emmc_fcie_err_handler_reinit();
		goto LABEL_END;
	}

	//-------------------------------
	//check FCIE
	u32_err = emmc_fcie_wait_events(FCIE_MIE_EVENT, BIT_DMA_END, TIME_WAIT_1_BLK_END);

	REG_FCIE_R(FCIE_SD_STATUS, u16_reg);
	if (u32_err != EMMC_ST_SUCCESS || (u16_reg & BIT_SD_FCIE_ERR_FLAGS)) {
		u32_err = EMMC_ST_ERR_CMD17_MIU;
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
			   "eMMC Err: CMD17 SD_STS: %04Xh, Err: %Xh, Arg: %Xh\n", u16_reg, u32_err, u32_arg);
		emmc_fcie_err_handler_reinit();
		goto LABEL_END;
	}

	//-------------------------------
	//check device
	u32_err = emmc_check_r1_error();
	if (u32_err != EMMC_ST_SUCCESS) {
		u32_err = EMMC_ST_ERR_CMD17_MIU;
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
			   "eMMC Err: CMD17 check R1 error: %Xh, Arg: %Xh\n", u32_err, u32_arg);
		emmc_fcie_err_handler_reinit();
		goto LABEL_END;
	}

LABEL_END:

	EMMC_FCIE_CLK_DIS();
	return u32_err;
}
#endif
#if FCIE_R3_R4_RESPONSE_TEST
U32 emmc_test_r3_r4_response(void)
{
	U32 u32_err, u32_arg;
	U16 u16_ctrl, u16_reg;

	u32_arg = BIT30 | (BIT23 | BIT22 | BIT21 | BIT20 | BIT19 | BIT18 | BIT17 | BIT16 | BIT15 | BIT7);
	u16_ctrl = BIT_SD_CMD_EN | BIT_SD_RSP_EN | BIT_R3R4_RSP_EN;

	emmc_fcie_clear_events();
	u32_err = emmc_fcie_send_cmd(emmc_drv.u16_mmc_mode, u16_ctrl, u32_arg, 1, EMMC_R3_BYTE_CNT);

	if (u32_err != EMMC_ST_SUCCESS) {
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
			   "eMMC Warn: CMD1 send CMD fail: %08Xh\n", u32_err);
		return u32_err;
	}

	//check status
	REG_FCIE_R(FCIE_SD_STATUS, u16_reg);

	//R3 has no CRC, so does not check BIT_SD_RSP_CRC_ERR
	if (u16_reg & BIT_SD_RSP_TIMEOUT) {
		u32_err = EMMC_ST_ERR_CMD1;
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
			   "eMMC Warn: CMD1 no Rsp, Reg.12: %04Xh\n", u16_reg);

		return u32_err;
	} else if (u16_reg & BIT_CMD_RSP_CERR) {
		u32_err = EMMC_ST_ERR_CMD1;
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
			   "eMMC Warn: CMD1 CMD Rsp CRC error, Reg.12: %04Xh\n", u16_reg);
		return u32_err;
	}

	return u32_err;
}
#endif

#if FCIE_CHECK_RESET_PIN
U32 emmc_test_check_rst_pin(void)
{
	U32 u32_err = 0;

	u32_err = emmc_cmd17_miu(EMMC_TEST_BLK_0, gau8_rbuf);
	if (u32_err != EMMC_ST_SUCCESS) {
		emmc_debug(0, 1, "emmc_cmd17_miu fail %Xh\n", u32_err);
		return u32_err;
	}

	u32_err = emmc_cmd13(emmc_drv.u16_rca);
	if (u32_err != EMMC_ST_SUCCESS) {
		emmc_debug(0, 1, "emmc_cmd13 fail %Xh\n", u32_err);
		return u32_err;
	}

	emmc_debug(0, 0, "Toggle RST pin now\n");
	EMMC_RST_L(); udelay(1);
	EMMC_RST_H(); udelay(1);

	EMMC_DISABLE_RETRY(1);

	EMMC_DISABLE_LOG(1);
	u32_err = emmc_cmd13(emmc_drv.u16_rca);
	EMMC_DISABLE_LOG(0);

	EMMC_DISABLE_RETRY(0);

	return u32_err;
}

#endif

static U32 emmc_ipverify_main_bypass_verify(void)
{
	U32 u32_err = EMMC_ST_SUCCESS;
	U8 u8_i, u8_j;
	int i;

	for (u8_i = 0; u8_i < 3; u8_i++) {
		for (u8_j = 0; u8_j < 2 ; u8_j++) {
			u32_err = emmc_init_for_test();
			if (u32_err != EMMC_ST_SUCCESS)
				return u32_err;

			if (u8_j) {
				u32_err = emmc_set_bus_speed(EMMC_SPEED_HIGH);
				if (u32_err != EMMC_ST_SUCCESS)
					return u32_err;
			}

			u32_err = emmc_set_bus_width(u8_bus_width[u8_i], 0);
			if (u32_err != EMMC_ST_SUCCESS)
				return u32_err;

			emmc_pads_switch(FCIE_EMMC_BYPASS);
			emmc_clock_setting(FCIE_SLOW_CLK);

			emmc_debug(EMMC_DEBUG_LEVEL, 1, "\033[7;33meMMC BYPASS mode speed %s bus width %d\033[m\n\n",
				   (u8_j) ? "HIGH" : "LOW", u8_bus_width[u8_i]);

			for (i = 0; i < 12; i++) {
				u32_err = emmc_ipverify_main_ex(u32_test_pattern[i]);
				if (u32_err != EMMC_ST_SUCCESS)
					return u32_err;
			}
			emmc_debug(EMMC_DEBUG_LEVEL, 1, "normal pattern test ok\n\n");

			for (i = 0; i < 12; i++) {
				u32_err = emmc_ipverify_main_sg_ex(u32_test_pattern[i]);
				if (u32_err != EMMC_ST_SUCCESS)
					return u32_err;
			}
			emmc_debug(EMMC_DEBUG_LEVEL, 1, "sglist pattern test ok\n\n");

			#if defined(DMA_AES_TEST) && DMA_AES_TEST
			u32_err = emmc_test_aes_all_mode_test();
			if (u32_err != EMMC_ST_SUCCESS)
				emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "Err, emmc_test_aes_all_mode_test fail: %Xh\n", u32_err);

			emmc_debug(EMMC_DEBUG_LEVEL_HIGH, 1, "[emmc_test_aes_all_mode_test finish]\n");
			#endif

			emmc_debug(EMMC_DEBUG_LEVEL, 1, "\033[7;32meMMC BYPASS mode speed %s bus width %d verify ok\033[m\n\n",
				   (u8_j) ? "HIGH" : "LOW", u8_bus_width[u8_i]);
		}
	}

	return u32_err;
}

static U32 emmc_ipverify_main_riu_verify(void)
{
	U32 u32_err = EMMC_ST_SUCCESS;
	U8 u8_i;
	int i;

	for (u8_i = 0; u8_i < 3; u8_i++) {
		u32_err = emmc_init_for_test();
		if (u32_err != EMMC_ST_SUCCESS)
			return u32_err;

		u32_err = emmc_set_bus_speed(EMMC_SPEED_HIGH);
		if (u32_err != EMMC_ST_SUCCESS)
			return u32_err;

		u32_err = emmc_set_bus_width(u8_bus_width[u8_i], 0);
		if (u32_err != EMMC_ST_SUCCESS)
			return u32_err;

		emmc_pads_switch(FCIE_EMMC_BYPASS);
		emmc_clock_setting(FCIE_SLOW_CLK);
		emmc_debug(EMMC_DEBUG_LEVEL, 1, "\033[7;33meMMC RIU BYPASS mode speed HIGH bus width %d\033[m\n\n",
			   u8_bus_width[u8_i]);

		for (i = 0; i < 12; i++) {
			u32_err = emmc_ipverify_riu_main_ex(u32_test_pattern[i]);
			if (u32_err != EMMC_ST_SUCCESS)
				return u32_err;
		}
		emmc_debug(EMMC_DEBUG_LEVEL, 1, "normal pattern test ok\n\n");
		emmc_debug(EMMC_DEBUG_LEVEL, 1,
			   "\033[7;32meMMC RIU BYPASS mode speed HIGH bus width %d verify ok\033[m\n\n",
			   u8_bus_width[u8_i]);
	}

	return u32_err;
}

static U32 emmc_ipverify_main_sdr_verify(void)
{
	U32 u32_err = EMMC_ST_SUCCESS;
	U8 u8_i;
	int i;

	for (u8_i = 1; u8_i < 3; u8_i++) {
		u32_err = emmc_init_for_test();
		if (u32_err != EMMC_ST_SUCCESS)
			return u32_err;

		u32_err = emmc_set_bus_speed(EMMC_SPEED_HIGH);
		if (u32_err != EMMC_ST_SUCCESS)
			return u32_err;

		u32_err = emmc_set_bus_width(u8_bus_width[u8_i], 0);
		if (u32_err != EMMC_ST_SUCCESS)
			return u32_err;

		emmc_pads_switch(FCIE_EMMC_SDR);
		emmc_clock_setting(FCIE_DEFAULT_CLK);
		emmc_debug(EMMC_DEBUG_LEVEL, 1, "\033[7;33meMMC SDR mode, bus width %d\033[m\n\n",
			   u8_bus_width[u8_i]);

		for (i = 0; i < 12; i++) {
			u32_err = emmc_ipverify_main_ex(u32_test_pattern[i]);
			if (u32_err != EMMC_ST_SUCCESS)
				return u32_err;
		}
		emmc_debug(EMMC_DEBUG_LEVEL, 1, "normal pattern test ok\n\n");

		for (i = 0; i < 12; i++) {
			u32_err = emmc_ipverify_main_sg_ex(u32_test_pattern[i]);
			if (u32_err != EMMC_ST_SUCCESS)
				return u32_err;
		}
		emmc_debug(EMMC_DEBUG_LEVEL, 1, "sglist pattern test ok\n\n");

		#if defined(DMA_AES_TEST) && DMA_AES_TEST
		u32_err = emmc_test_aes_all_mode_test();
		if (u32_err != EMMC_ST_SUCCESS)
			emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "Err, emmc_test_aes_all_mode_test fail: %Xh\n", u32_err);

		emmc_debug(EMMC_DEBUG_LEVEL_HIGH, 1, "[emmc_test_aes_all_mode_test finish]\n");
		#endif

		emmc_debug(EMMC_DEBUG_LEVEL, 1, "\033[7;32meMMC SDR mode, bus width %d verify ok\033[m\n\n",
			   u8_bus_width[u8_i]);
	}

	return u32_err;
}

static U32 emmc_ipverify_main_ddr_verify(void)
{
	U32 u32_err = EMMC_ST_SUCCESS;
	U8 u8_i;
	int i;

	for (u8_i = 2; u8_i < 3; u8_i++) {
		u32_err = emmc_init_for_test();
		if (u32_err != EMMC_ST_SUCCESS)
			return u32_err;

		u32_err = emmc_fcie_enable_fastmode_ex(FCIE_EMMC_DDR);
		if (u32_err != EMMC_ST_SUCCESS)
			return u32_err;

		u32_err = emmc_fcie_build_ddr_timing_table();
		if (u32_err != EMMC_ST_SUCCESS)
			return u32_err;

		emmc_debug(EMMC_DEBUG_LEVEL, 1, "\033[7;33meMMC DDR mode, bus width %d\033[m\n\n",
			   u8_bus_width[u8_i]);

		for (i = 0; i < 12; i++) {
			u32_err = emmc_ipverify_main_ex(u32_test_pattern[i]);
			if (u32_err != EMMC_ST_SUCCESS)
				return u32_err;
		}
		emmc_debug(EMMC_DEBUG_LEVEL, 1, "normal pattern test ok\n\n");

		for (i = 0; i < 12; i++) {
			u32_err = emmc_ipverify_main_sg_ex(u32_test_pattern[i]);
			if (u32_err != EMMC_ST_SUCCESS)
				return u32_err;
		}
		emmc_debug(EMMC_DEBUG_LEVEL, 1, "sglist pattern test ok\n\n");

		#if defined(DMA_AES_TEST) && DMA_AES_TEST
		u32_err = emmc_test_aes_all_mode_test();
		if (u32_err != EMMC_ST_SUCCESS)
			emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "Err, emmc_test_aes_all_mode_test fail: %Xh\n", u32_err);

		emmc_debug(EMMC_DEBUG_LEVEL_HIGH, 1, "[emmc_test_aes_all_mode_test finish]\n");
		#endif

		emmc_debug(EMMC_DEBUG_LEVEL, 1, "\033[7;32meMMC DDR mode, bus width %d verify ok\033[m\n\n",
			   u8_bus_width[u8_i]);
	}

	return u32_err;
}

static U32 emmc_ipverify_main_hs200_verify(void)
{
	U32 u32_err = EMMC_ST_SUCCESS;
	U8 u8_i;
	int i;

	for (u8_i = 2; u8_i < 3; u8_i++) {
		u32_err = emmc_init_for_test();
		if (u32_err != EMMC_ST_SUCCESS)
			return u32_err;

		u32_err = emmc_fcie_enable_fastmode_ex(FCIE_EMMC_HS200);
		if (u32_err != EMMC_ST_SUCCESS)
			return u32_err;

		u32_err = emmc_fcie_build_hS200_timing_table();
			if (u32_err != EMMC_ST_SUCCESS)
				return u32_err;

		#if defined(ENABLE_EMMC_AFIFO) && ENABLE_EMMC_AFIFO
		emmc_debug(EMMC_DEBUG_LEVEL, 1, "\033[7;33meMMC HS200 AFIFO mode, bus width %d\033[m\n\n",
			   u8_bus_width[u8_i]);
		#else
		emmc_debug(EMMC_DEBUG_LEVEL, 1, "\033[7;33meMMC HS200 mode, bus width %d\033[m\n\n",
			   u8_bus_width[u8_i]);
		#endif

		u32_err = emmc_cmd21();
		if (u32_err != EMMC_ST_SUCCESS)
			return u32_err;

		emmc_debug(EMMC_DEBUG_LEVEL, 1, "CMD21 tuning pattern test ok\n\n");

		for (i = 0; i < 12; i++) {
			u32_err = emmc_ipverify_main_ex(u32_test_pattern[i]);
			if (u32_err != EMMC_ST_SUCCESS)
				return u32_err;
		}
		emmc_debug(EMMC_DEBUG_LEVEL, 1, "normal pattern test ok\n\n");

		for (i = 0; i < 12; i++) {
			u32_err = emmc_ipverify_main_sg_ex(u32_test_pattern[i]);
			if (u32_err != EMMC_ST_SUCCESS)
				return u32_err;
		}
		emmc_debug(EMMC_DEBUG_LEVEL, 1, "sglist pattern test ok\n\n");

		#if defined(DMA_AES_TEST) && DMA_AES_TEST
		u32_err = emmc_test_aes_all_mode_test();
		if (u32_err != EMMC_ST_SUCCESS)
			emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "Err, emmc_test_aes_all_mode_test fail: %Xh\n", u32_err);

		emmc_debug(EMMC_DEBUG_LEVEL_HIGH, 1, "[emmc_test_aes_all_mode_test finish]\n");
		#endif

		#if defined(ENABLE_EMMC_AFIFO) && ENABLE_EMMC_AFIFO
		emmc_debug(EMMC_DEBUG_LEVEL, 1, "\033[7;32meMMC HS200 AFIFO mode, bus width %d verify ok\033[m\n\n",
			   u8_bus_width[u8_i]);
		#else
		emmc_debug(EMMC_DEBUG_LEVEL, 1, "\033[7;32meMMC HS200 mode, bus width %d verify ok\033[m\n\n",
			   u8_bus_width[u8_i]);
		#endif
	}

	return u32_err;
}

static U32 emmc_ipverify_main_hs400_verify(void)
{
	U32 u32_err = EMMC_ST_SUCCESS;
	int i;

	u32_err = emmc_fcie_switch_to_hs400_mode();
	if (u32_err != EMMC_ST_SUCCESS) {
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "Err, emmc_fcie_switch_to_hs400_mode fail: %Xh\n", u32_err);
		return u32_err;
	}

	#if defined(ENABLE_EMMC_AFIFO) && ENABLE_EMMC_AFIFO
	emmc_debug(EMMC_DEBUG_LEVEL, 1, "\033[7;33meMMC HS400 AFIFO mode, bus width 8\033[m\n\n");
	#else
	emmc_debug(EMMC_DEBUG_LEVEL, 1, "\033[7;33meMMC HS400 mode, bus width 8\033[m\n\n");
	#endif

	for (i = 0; i < 12; i++) {
		u32_err = emmc_ipverify_main_ex(u32_test_pattern[i]);
		if (u32_err != EMMC_ST_SUCCESS)
			return u32_err;
	}
	emmc_debug(EMMC_DEBUG_LEVEL, 1, "normal pattern test ok\n\n");

	for (i = 0; i < 12; i++) {
		u32_err = emmc_ipverify_main_sg_ex(u32_test_pattern[i]);
		if (u32_err != EMMC_ST_SUCCESS)
			return u32_err;
	}
	emmc_debug(EMMC_DEBUG_LEVEL, 1, "sglist pattern test ok\n\n");

	#if defined(DMA_AES_TEST) && DMA_AES_TEST
	u32_err = emmc_test_aes_all_mode_test();
	if (u32_err != EMMC_ST_SUCCESS) {
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "Err, emmc_test_aes_all_mode_test fail: %Xh\n", u32_err);
		return u32_err;
	}
	emmc_debug(EMMC_DEBUG_LEVEL_HIGH, 1, "[emmc_test_aes_all_mode_test finish]\n");
	#endif

	#if defined(ENABLE_EMMC_AFIFO) && ENABLE_EMMC_AFIFO
	emmc_debug(EMMC_DEBUG_LEVEL, 1, "\033[7;32meMMC HS400 AFIFO mode, bus width 8 verify ok\033[m\n\n");
	#else
	emmc_debug(EMMC_DEBUG_LEVEL, 1, "\033[7;32meMMC HS400 mode, bus width 8 verify ok\033[m\n\n");
	#endif

	return u32_err;
}

static U32 emmc_ipverify_main_hs400_5_1_verify(void)
{
	U32 u32_err = EMMC_ST_SUCCESS;
	int i;

	if ((emmc_drv.u8_ecsd196_dev_type & EMMC_DEVTYPE_HS400_1_8V) && emmc_drv.u8_ecsd184_stroe_support) {
		emmc_drv.u16_mmc_mode = BIT_SD_DEFAULT_MODE_REG;

		//init platform & FCIE
		emmc_platform_init();
		u32_err = emmc_fcie_init();
		if (u32_err)
			return u32_err;

		u32_err = emmc_init_device_ex();
		if (u32_err != EMMC_ST_SUCCESS)
			return u32_err;

		u32_err = emmc_fcie_enable_fastmode_ex(FCIE_EMMC_HS400_5_1);
		if (u32_err != EMMC_ST_SUCCESS)
			return u32_err;

		if (emmc_drv.u8_emmc_pll_skew4)
			u32_err = emmc_fcie_detect_hs400_5_1_skew4_timing();
		else
			u32_err = emmc_fcie_detect_hs400_5_1_timing();

		if (u32_err != EMMC_ST_SUCCESS)
			return u32_err;

		emmc_debug(EMMC_DEBUG_LEVEL, 1, "\033[7;33meMMC HS400 enhence mode, bus width 8\033[m\n\n");

		for (i = 0; i < 12; i++) {
			u32_err = emmc_ipverify_main_ex(u32_test_pattern[i]);
			if (u32_err != EMMC_ST_SUCCESS)
				return u32_err;
		}
		emmc_debug(EMMC_DEBUG_LEVEL, 1, "normal pattern test ok\n\n");

		for (i = 0; i < 12; i++) {
			u32_err = emmc_ipverify_main_sg_ex(u32_test_pattern[i]);
			if (u32_err != EMMC_ST_SUCCESS)
				return u32_err;
		}
		emmc_debug(EMMC_DEBUG_LEVEL, 1, "sglist pattern test ok\n\n");

		#if defined(DMA_AES_TEST) && DMA_AES_TEST
		u32_err = emmc_test_aes_all_mode_test();
		if (u32_err != EMMC_ST_SUCCESS)
			emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "Err, emmc_test_aes_all_mode_test fail: %Xh\n", u32_err);

		emmc_debug(EMMC_DEBUG_LEVEL_HIGH, 1, "[emmc_test_aes_all_mode_test finish]\n");
		#endif

		emmc_debug(EMMC_DEBUG_LEVEL, 1, "\033[7;32meMMC HS400 enhence mode, bus width 8 verify ok\033[m\n\n");
	}

	return u32_err;
}

#if FCIE_BIST_TEST
static U32 emmc_ipverify_main_bist_test(void)
{
	U32 u32_err = EMMC_ST_SUCCESS;
	U8 u8_i;

	emmc_fcie_err_handler_reinit();
	emmc_debug(0, 1, "\033[7;33meMMC BIST Verify\033[m\n\n");

	for (u8_i = 0; u8_i < 12; u8_i++) {
		u32_err = emmc_test_bist(u8_i);
		if (u32_err != EMMC_ST_SUCCESS)
			break;
	}

	if (u32_err != EMMC_ST_SUCCESS)
		emmc_debug(0, 1, "\033[7;31meMMC BIST Verify Fail\033[m\n\n");
	else
		emmc_debug(0, 1, "\033[7;32meMMC BIST Verify OK\033[m\n\n");

	return u32_err;
}
#endif

#if FCIE_RSP_SHIFT_TUNING_TEST
static U32 emmc_ipverify_main_rsp_shift_tuning_test(void)
{
	U32 u32_err = EMMC_ST_SUCCESS;
	U8 u8_i;

	emmc_fcie_err_handler_reinit();
	emmc_debug(0, 1, "\033[7;33meMMC RSP Shift Tuning\033[m\n\n");

	for (u8_i = 0; u8_i < 12; u8_i++) {
		u32_err = emmc_test_rsp_shift_tuning(EMMC_TEST_BLK_0, u32_test_pattern[u8_i]);
		if (u32_err != EMMC_ST_SUCCESS)
			break;
	}

	if (u32_err != EMMC_ST_SUCCESS)
		emmc_debug(0, 1, "\033[7;31meMMC RSP Shift Tuning Fail\033[m\n\n");
	else
		emmc_debug(0, 1, "\033[7;32meMMC RSP Shift Tuning OK\033[m\n\n");

	return u32_err;
}
#endif

#if FCIE_WCRC_SHIFT_TUNING_TEST
static U32 emmc_ipverify_main_wcrc_shift_tuning_test(void)
{
	U32 u32_err = EMMC_ST_SUCCESS;
	U8 u8_i;

	emmc_fcie_err_handler_reinit();
	emmc_debug(0, 1, "\033[7;33meMMC WCRC Shift Tuning\033[m\n\n");

	for (u8_i = 0; u8_i < 12; u8_i++) {
		u32_err = emmc_test_wcrc_shift_tuning(EMMC_TEST_BLK_0, u32_test_pattern[u8_i]);
		if (u32_err != EMMC_ST_SUCCESS)
			break;
	}

	if (u32_err != EMMC_ST_SUCCESS)
		emmc_debug(0, 1, "\033[7;31meMMC WCRC Shift Tuning fail\033[m\n\n");
	else
		emmc_debug(0, 1, "\033[7;32meMMC WCRC Shift Tuning OK\033[m\n\n");

	return u32_err;
}
#endif

#if FCIE_RSTOP_SHIFT_TUNING_TEST
static U32 emmc_ipverify_main_rstop_shift_tuning_test(void)
{
	U32 u32_err = EMMC_ST_SUCCESS;
	U8 u8_i;

	emmc_fcie_err_handler_reinit();
	emmc_debug(0, 1, "\033[7;33meMMC RStop Shift Tuning\033[m\n\n");

	for (u8_i = 0; u8_i < 12; u8_i++) {
		u32_err = emmc_test_rstop_shift_tuning(EMMC_TEST_BLK_0, u32_test_pattern[u8_i]);
		if (u32_err != EMMC_ST_SUCCESS)
			break;
	}

	if (u32_err != EMMC_ST_SUCCESS)
		emmc_debug(0, 1, "\033[7;31meMMC RStop Shift Tuning fail\033[m\n\n");
	else
		emmc_debug(0, 1, "\033[7;32meMMC RStop Shift Tuning OK\033[m\n\n");

	return u32_err;
}
#endif

#if FCIE_READ_CRC_ERROR_INT_TEST
static U32 emmc_ipverify_main_read_crc_err_int_test(void)
{
	U32 u32_err = EMMC_ST_SUCCESS;

	emmc_fcie_err_handler_reinit();
	emmc_debug(0, 1, "\033[7;33mTest read CRC error int\033[m\n");

	u32_err = emmc_test_read_crc_err(EMMC_TEST_BLK_0, gau8_rbuf);

	if (u32_err != EMMC_ST_SUCCESS)
		emmc_debug(0, 1, "\033[7;32meMMC Read CRC error int OK\033[m\n\n");
	else
		emmc_debug(0, 1, "\033[7;31meMMC Read CRC error int Fail\033[m\n\n");

	return u32_err;
}
#endif

#if FCIE_WRITE_CRC_ERROR_INT_TEST
static U32 emmc_ipverify_main_write_crc_err_int_test(void)
{
	U32 u32_err = EMMC_ST_SUCCESS;

	emmc_fcie_err_handler_reinit();
	emmc_debug(0, 1, "\033[7;33mTest write CRC error int\033[m\n");

	u32_err = emmc_test_write_crc_err(EMMC_TEST_BLK_0, gau8_wbuf);

	if (u32_err != EMMC_ST_SUCCESS)
		emmc_debug(0, 1, "\033[7;32meMMC Write CRC error int OK\033[m\n\n");
	else
		emmc_debug(0, 1, "\033[7;31meMMC Write CRC error int Fail\033[m\n\n");

	return u32_err;
}
#endif

#if FCIE_WRITE_TIMEOUT_INT_TEST
static U32 emmc_ipverify_main_write_timeout_test(void)
{
	U32 u32_err = EMMC_ST_SUCCESS;


	emmc_fcie_err_handler_reinit();
	emmc_debug(0, 1, "\033[7;33mTest write timeout int\033[m\n");
	u32_err = emmc_test_write_timeout_err(EMMC_TEST_BLK_0, gau8_wbuf, 2);
	if (u32_err != EMMC_ST_SUCCESS)
		emmc_debug(0, 1, "\033[7;32meMMC Write timeout error int OK\033[m\n\n");
	else
		emmc_debug(0, 1, "\033[7;31meMMC Write timeout error int Fail\033[m\n\n");

	return u32_err;
}
#endif

#if FCIE_READ_TIMEOUT_INT_TEST
static U32 emmc_ipverify_main_read_timeout_test(void)
{
	U32 u32_err = EMMC_ST_SUCCESS;

	emmc_fcie_err_handler_reinit();
	emmc_debug(0, 1, "\033[7;33mTest read timeout int\033[m\n");

	u32_err = emmc_test_read_timeout_err(0);

	if (u32_err != EMMC_ST_SUCCESS)
		emmc_debug(0, 1, "\033[7;32meMMC Read timeout error int OK\033[m\n\n");
	else
		emmc_debug(0, 1, "\033[7;31meMMC Read timeout error int Fail\033[m\n\n");

	return u32_err;
}
#endif

#if FCIE_CMD_NO_RESPONSE_INT_TEST
static U32 emmc_ipverify_main_cmd_no_rsp_test(void)
{
	U32 u32_err = EMMC_ST_SUCCESS;

	emmc_fcie_err_handler_reinit();
	emmc_debug(0, 1, "\033[7;33mTest commnad no response int\033[m\n");
	u32_err = emmc_test_cmd_no_rsp_err(0);
	if (u32_err != EMMC_ST_SUCCESS)
		emmc_debug(0, 1, "\033[7;32meMMC command no response int OK\033[m\n\n");
	else
		emmc_debug(0, 1, "\033[7;31meMMC command no response int Fail\033[m\n\n");

	return u32_err;
}
#endif

#if FCIE_CMD_RSP_CRC_ERROR_INT_TEST
static U32 emmc_ipverify_main_rsp_crc_test(void)
{
	U32 u32_err = EMMC_ST_SUCCESS;

	emmc_fcie_err_handler_reinit();
	emmc_debug(0, 1, "\033[7;33mTest commnad response CRC error int\033[m\n");

	u32_err = emmc_test_cmd_rsp_crc_err(EMMC_TEST_BLK_0, gau8_rbuf);
	if (u32_err != EMMC_ST_SUCCESS)
		emmc_debug(0, 1, "\033[7;32meMMC command response CRC error int OK\033[m\n\n");
	else
		emmc_debug(0, 1, "\033[7;31meMMC command response CRC error int Fail\033[m\n\n");

	return u32_err;
}
#endif

#if FCIE_R3_R4_RESPONSE_TEST

static U32 emmc_ipverify_main_r3_r4_rsp_test(void)
{
	U32 u32_err = EMMC_ST_SUCCESS;

	emmc_drv.u16_mmc_mode = BIT_SD_DEFAULT_MODE_REG;

	emmc_platform_init();
	u32_err = emmc_fcie_init();
	if (u32_err != EMMC_ST_SUCCESS)
		return u32_err;

	emmc_drv.u16_rca = 1;
	emmc_drv.u8_bus_width = BIT_SD_DATA_WIDTH_1;
	emmc_drv.u16_mmc_mode &= ~BIT_SD_DATA_WIDTH_MASK;
	emmc_drv.u32_drvflag &= ~DRV_FLAG_SPEED_MASK;

	if (emmc_fcie_wait_d0_high(TIME_WAIT_DAT0_HIGH) != EMMC_ST_SUCCESS) {
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: WaitD0High TO\n");
		emmc_fcie_err_handler_stop();
	}
	EMMC_RST_L();  emmc_hw_timer_sleep(1);
	EMMC_RST_H();  emmc_hw_timer_sleep(1);

	//CMD0
	u32_err = emmc_cmd0(0);//reset to idle state
	if (u32_err != EMMC_ST_SUCCESS)
		return u32_err;

	emmc_debug(0, 1, "\033[7;33mTest R3/R4 response CRC error patch\033[m\n");

	u32_err = emmc_test_r3_r4_response();
	if (u32_err == EMMC_ST_SUCCESS)
		emmc_debug(0, 1, "\033[7;32mTest R3/R4 response CRC error patch OK\033[m\n\n");
	else
		emmc_debug(0, 1, "\033[7;31mTest R3/R4 response CRC error patch Fail\033[m\n\n");

	emmc_fcie_err_handler_reinit();
	return u32_err;
}
#endif

#if FCIE_CHECK_RESET_PIN
static U32 emmc_ipverify_main_chk_reset_pin(void)
{
	U32 u32_err = EMMC_ST_SUCCESS;

	emmc_fcie_err_handler_reinit();

	emmc_debug(0, 1, "\033[7;33mTest reset pin\033[m\n");
	u32_err = emmc_test_check_rst_pin();

	if (u32_err != EMMC_ST_SUCCESS)
		emmc_debug(0, 1, "\033[7;32mTest reset pin ok\033[m\n");
	else
		emmc_debug(0, 1, "\033[7;31mTest reset pin fail\033[m\n");

	emmc_fcie_err_handler_reinit();
	return u32_err;
}
#endif

#if FCIE_LAST_BLOCK_CRC_TEST

U8 u8_sdr_crc[12][16] = {
	{0x53, 0x39, 0x96, 0x41, 0x31, 0x63, 0x0B, 0x06, 0x15, 0x70, 0x34, 0xEB, 0x5D, 0xB4, 0xCF, 0x7E},
	{0x53, 0x39, 0x96, 0xBE, 0xCE, 0x63, 0xF4, 0xF9, 0x15, 0x8F, 0x34, 0xEB, 0xA2, 0x4B, 0xCF, 0x81},
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0x00, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0xFF},
	{0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0xFF, 0x00},
	{0xFF, 0xFF, 0x00, 0x00, 0x00, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0xFF, 0xFF},
	{0x00, 0x00, 0x00, 0x55, 0x55, 0x00, 0x55, 0x55, 0x00, 0x55, 0x00, 0x00, 0x55, 0x55, 0x00, 0x55},
	{0x00, 0x00, 0x00, 0xAA, 0xAA, 0x00, 0xAA, 0xAA, 0x00, 0xAA, 0x00, 0x00, 0xAA, 0xAA, 0x00, 0xAA},
	{0xFF, 0xFF, 0x00, 0xAA, 0xAA, 0xFF, 0xAA, 0x55, 0xFF, 0x55, 0x00, 0xFF, 0xAA, 0x55, 0xFF, 0x55},
	{0xFF, 0xFF, 0x00, 0x55, 0x55, 0xFF, 0x55, 0xAA, 0xFF, 0xAA, 0x00, 0xFF, 0x55, 0xAA, 0xFF, 0xAA},
	{0xFF, 0xFF, 0x00, 0xA5, 0xA5, 0xFF, 0xA5, 0x5A, 0xFF, 0x5A, 0x00, 0xFF, 0xA5, 0x5A, 0xFF, 0x5A},
	{0xFF, 0xFF, 0x00, 0x5A, 0x5A, 0xFF, 0x5A, 0xA5, 0xFF, 0xA5, 0x00, 0xFF, 0x5A, 0xA5, 0xFF, 0xA5}
};

static U32 emmc_ipverify_main_switch_sdr(void)
{
	U32 u32_err = EMMC_ST_SUCCESS;

	u32_err = emmc_init_for_test();
	if (u32_err != EMMC_ST_SUCCESS)
		return u32_err;

	u32_err = emmc_set_bus_speed(EMMC_SPEED_HIGH);
	if (u32_err != EMMC_ST_SUCCESS)
		return u32_err;

	u32_err = emmc_set_bus_width(8, 0);
	if (u32_err != EMMC_ST_SUCCESS)
		return u32_err;

	emmc_pads_switch(FCIE_EMMC_SDR);
	emmc_clock_setting(FCIE_DEFAULT_CLK);

	return u32_err;
}

static U32 emmc_ipverify_main_last_blk_crc_test(void)
{
	U32 u32_err = EMMC_ST_SUCCESS;
	U8 u8_i;
	int i;
	U16 tmp[16];
	U32 *pu32_w = (U32 *)gau8_wbuf, *pu32_r = (U32 *)gau8_rbuf;

	memset(tmp, 0, 16);
	emmc_ipverify_main_switch_sdr();
	emmc_debug(0, 1, "\033[7;33mTest last CRC bank\033[m\n");

	for (u8_i = 0; u8_i < 12; u8_i++) {
		emmc_debug(0, 1, "Data Pattern: %08X\n", u32_test_pattern[u8_i]);

		if (u32_test_pattern[u8_i] == EMMC_PATTERN_00112233 || u32_test_pattern[u8_i] == EMMC_PATTERN_00332211) {
			for (i = 0; i < 512; i++) {
				if (u32_test_pattern[u8_i] == EMMC_PATTERN_00112233)
					gau8_wbuf[i] = i & 0xFF;
				else if (u32_test_pattern[u8_i] == EMMC_PATTERN_00332211)
					gau8_wbuf[i] = 0xFF - (i & 0xFF);

				gau8_rbuf[i] = ~gau8_wbuf[i];
			}
		} else {
			for (i = 0; i < 128; i++) {
				pu32_w[i] = u32_test_pattern[u8_i];
				pu32_r[i] = ~pu32_w[i];
			}
		}
		EMMC_DISABLE_RETRY(1);

		u32_err = emmc_cmd25_miu(EMMC_TEST_BLK_0, gau8_wbuf, 1);
		if (u32_err != EMMC_ST_SUCCESS) {
			emmc_debug(0, 1, "emmc_cmd24_miu fail %Xh\n", u32_err);
			return u32_err;
		}

		u32_err = emmc_cmd17_miu(EMMC_TEST_BLK_0, gau8_rbuf);
		if (u32_err != EMMC_ST_SUCCESS) {
			emmc_debug(0, 1, "emmc_cmd17_miu fail %Xh\n", u32_err);
			return u32_err;
		}

		EMMC_DISABLE_RETRY(0);

		for (i = 0; i < 16; i++)
			REG_FCIE_R(GET_REG_ADDR(FCIE1_BASE, 0x40 + i), tmp[i]);

		if (memcmp(tmp, &u8_sdr_crc[u8_i], 16) != 0) {
			emmc_debug(0, 1, "Dump CRC Bank:\n");
			emmc_dump_mem((U8 *)tmp, 16);
			emmc_debug(0, 1, "Correct CRC Bank:\n");
			emmc_dump_mem((U8 *)&u8_sdr_crc[u8_i], 16);
			emmc_debug(0, 1, "\033[7;31mTest last CRC bank fail\033[m\n");
			return u32_err;
		}
		emmc_debug(0, 1, "[OK]\n");
	}
	emmc_debug(0, 1, "\033[7;32mTest last CRC bank ok\033[m\n");
	return u32_err;
}
#endif

static void emmc_ipverify_main_end(void)
{
	if (gau8_wbuf) {
		free(gau8_wbuf);
		gau8_wbuf = 0;
	}

	if (gau8_rbuf) {
		free(gau8_rbuf);
		gau8_rbuf = 0;
	}
}

static void emmc_ipverify_main_all_speed_mode(void)
{
	#if (0 || FCIE_BYPASS_VERIFY)//BYPASS
	emmc_ipverify_main_bypass_verify();
	#endif

	#if (0 || FCIE_BYPASS_RIU_VERIFY)//BYPASS + RIU mode
	emmc_ipverify_main_riu_verify();
	#endif

	#if (0 || FCIE_SDR_VERIFY)//SDR
	emmc_ipverify_main_sdr_verify();
	#endif

	#if (0 || FCIE_DDR_VERIFY) && (defined(ENABLE_EMMC_ATOP) && ENABLE_EMMC_ATOP)//DDR
	emmc_ipverify_main_ddr_verify();
	#endif

	#if (0 || FCIE_HS200_VERIFY) && (defined(ENABLE_EMMC_HS200) && ENABLE_EMMC_HS200)//HS200
	emmc_ipverify_main_hs200_verify();
	#endif

	#if (0 || FCIE_HS400_VERIFY) && (defined(ENABLE_EMMC_HS400) && ENABLE_EMMC_HS400)//HS400
	emmc_ipverify_main_hs400_verify();
	#endif

	#if (0 || FCIE_HS400_VERIFY) && (defined(ENABLE_EMMC_HS400_5_1) && ENABLE_EMMC_HS400_5_1)//HS400
	emmc_ipverify_main_hs400_5_1_verify();
	#endif
}

U32 emmc_ipverify_main(void)
{
	if (!gau8_wbuf)
		gau8_wbuf = (U8 *)memalign(EMMC_CACHE_LINE, TEST_BUFFER_SIZE);

	if (!gau8_rbuf)
		gau8_rbuf = (U8 *)memalign(EMMC_CACHE_LINE, TEST_BUFFER_SIZE);

	emmc_debug(EMMC_DEBUG_LEVEL, 1, "\n");
	emmc_debug(EMMC_DEBUG_LEVEL, 1, "%lXh %lXh %lXh\n",
		   (unsigned long)GLOBAL_EMMC_SECTORBUF, (unsigned long)gau8_rbuf, (unsigned long)gau8_wbuf);

	//===============================================
	emmc_ipverify_main_all_speed_mode();

	#if (0 || FCIE_BIST_TEST)
	emmc_ipverify_main_bist_test();
	#endif

	#if (0 || FCIE_POWER_SAVING_MODE_TEST)

	emmc_fcie_err_handler_reinit();
	emmc_debug(0, 0, "\033[7;31mTest power saving mode\033[m\n");
	emmc_test_power_saving_mode();

	//Check with LA
	#endif

	#if (0 || FCIE_RSP_SHIFT_TUNING_TEST)
	emmc_ipverify_main_rsp_shift_tuning_test();
	#endif

	#if (0 || FCIE_WCRC_SHIFT_TUNING_TEST)
	emmc_ipverify_main_wcrc_shift_tuning_test();
	#endif

	#if (0 || FCIE_RSTOP_SHIFT_TUNING_TEST)
	emmc_ipverify_main_rstop_shift_tuning_test();
	#endif

	#if (0 || FCIE_READ_CRC_ERROR_INT_TEST)
	emmc_ipverify_main_read_crc_err_int_test();
	#endif

	#if (0 || FCIE_WRITE_CRC_ERROR_INT_TEST)
	emmc_ipverify_main_write_crc_err_int_test();
	#endif

	#if (0 || FCIE_WRITE_TIMEOUT_INT_TEST)
	emmc_ipverify_main_write_timeout_test();
	#endif

	#if (0 || FCIE_READ_TIMEOUT_INT_TEST)
	emmc_ipverify_main_read_timeout_test();
	#endif

	#if (0 || FCIE_CMD_NO_RESPONSE_INT_TEST)
	emmc_ipverify_main_cmd_no_rsp_test();
	#endif

	#if (0 || FCIE_CMD_RSP_CRC_ERROR_INT_TEST)
	emmc_ipverify_main_rsp_crc_test();
	#endif

	#if (0 || FCIE_R3_R4_RESPONSE_TEST)
	emmc_ipverify_main_r3_r4_rsp_test();
	#endif

	#if (0 || FCIE_CHECK_RESET_PIN)
	emmc_ipverify_main_chk_reset_pin();
	#endif

	#if (0 || FCIE_LAST_BLOCK_CRC_TEST)
	emmc_ipverify_main_last_blk_crc_test();
	#endif

	#endif

	emmc_ipverify_main_end();

	return EMMC_ST_SUCCESS;
}

U32  emmc_ipverify_main_api_ex(U32 u32_data_pattern)
{
	U32 u32_i, *pu32_w = (U32 *)gau8_wbuf, *pu32_r = (U32 *)gau8_rbuf, u32_err;

	//init data pattern
	for (u32_i = 0; u32_i < TEST_BUFFER_SIZE >> 2; u32_i++) {
		pu32_w[u32_i] = u32_data_pattern;
		pu32_r[u32_i] = ~pu32_w[u32_i];
	}

	emmc_debug(EMMC_DEBUG_LEVEL, 1, "Data Pattern: %08Xh\n", u32_data_pattern);

	u32_err = emmc_write_data((U8 *)pu32_w, TEST_BUFFER_SIZE, EMMC_TEST_BLK_0);
	if (u32_err != EMMC_ST_SUCCESS) {
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "Err, emmc_write_data fail: %Xh\n", u32_err);
		goto LABEL_IP_VERIFY_ERROR;
	}
	emmc_debug(EMMC_DEBUG_LEVEL_HIGH, 1, "[emmc_write_data ok]\n");

	u32_err = emmc_read_data((U8 *)pu32_r, TEST_BUFFER_SIZE, EMMC_TEST_BLK_0);
	if (u32_err != EMMC_ST_SUCCESS) {
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "Err, emmc_read_data fail: %Xh\n", u32_err);
		goto LABEL_IP_VERIFY_ERROR;
	}
	emmc_debug(EMMC_DEBUG_LEVEL_HIGH, 1, "[emmc_read_data ok]\n");

	u32_err = emmc_compare_data((U8 *)pu32_w, (U8 *)pu32_r, TEST_BUFFER_SIZE);
	if (u32_err != EMMC_ST_SUCCESS) {
		emmc_debug(0, 1, "Err, emmc_compare_data fail: %u, %Xh\n", u32_i, u32_err);
		goto LABEL_IP_VERIFY_ERROR;
	}

	//===============================================
	emmc_debug(EMMC_DEBUG_LEVEL, 1, "\n");
	emmc_debug(EMMC_DEBUG_LEVEL, 1, "[OK]\n\n");
	return EMMC_ST_SUCCESS;

LABEL_IP_VERIFY_ERROR:
	emmc_debug(EMMC_DEBUG_LEVEL, 1, "\n\n");
	emmc_debug(EMMC_DEBUG_LEVEL, 1, "[eMMC IPVerify Fail: %Xh]\n\n", u32_err);
	emmc_dump_driver_status();

	return u32_err;
}

U32 emmc_ipverify_main_sg_ex(U32 u32_data_pattern)
{
	static U32 u32_start_sector = 0, u32_sector_cnt = 0, u32_err;

	//make StartSector SectorCnt random
	u32_start_sector = EMMC_TEST_BLK_0;
	u32_sector_cnt = EMMC_TEST_BLK_CNT;

	emmc_debug(EMMC_DEBUG_LEVEL, 1, "Data Pattern: %08Xh\n", u32_data_pattern);

	u32_err = emmc_test_sg_wrc_miu(u32_start_sector, u32_sector_cnt, u32_data_pattern);
	if (u32_err != EMMC_ST_SUCCESS) {
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "Err, emmc_test_sg_wrc_miu fail: %Xh\n", u32_err);
		goto LABEL_IP_VERIFY_ERROR;
	}
	emmc_debug(EMMC_DEBUG_LEVEL, 1, "[emmc_test_sg_wrc_miu ok]\n");

	//===============================================
	emmc_debug(EMMC_DEBUG_LEVEL, 1, "\n");
	emmc_debug(EMMC_DEBUG_LEVEL, 1, "[OK]\n\n");
	return EMMC_ST_SUCCESS;

LABEL_IP_VERIFY_ERROR:
	emmc_debug(EMMC_DEBUG_LEVEL, 1, "\n\n");
	emmc_debug(EMMC_DEBUG_LEVEL, 1, "Total Sec: %Xh, Test: StartSec: %Xh, SecCnt: %Xh\n",
		   emmc_drv.u32_sec_count, u32_start_sector, u32_sector_cnt);
	emmc_debug(EMMC_DEBUG_LEVEL, 1, "[eMMC IPVerify Fail: %Xh]\n\n", u32_err);
	emmc_dump_driver_status();

	return u32_err;
}

U32 emmc_ipverify_main_big_ex(U32 u32_data_pattern)
{
	U32 u32_start_sector = 0, u32_err;

	//make StartSector SectorCnt random
	u32_start_sector = EMMC_TEST_BLK_0;

	#if 1
	u32_err = emmc_test_multi_blk_wrc_miu_large(u32_start_sector, u32_data_pattern);
	if (u32_err != EMMC_ST_SUCCESS) {
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "Err, emmc_test_multi_blk_wrc_miu_large fail: %Xh\n", u32_err);
		goto LABEL_IP_VERIFY_ERROR;
	}
	emmc_debug(EMMC_DEBUG_LEVEL, 1, "[emmc_test_multi_blk_wrc_miu_large ok]\n");
	#endif

	//===============================================
	emmc_debug(EMMC_DEBUG_LEVEL, 1, "\n");
	emmc_debug(EMMC_DEBUG_LEVEL, 1, "[OK]\n\n");
	return EMMC_ST_SUCCESS;

LABEL_IP_VERIFY_ERROR:
	emmc_debug(EMMC_DEBUG_LEVEL, 1, "\n\n");
	emmc_debug(EMMC_DEBUG_LEVEL, 1, "Total Sec: %Xh, Test: StartSec: %Xh\n",
		   emmc_drv.u32_sec_count, u32_start_sector);
	emmc_debug(EMMC_DEBUG_LEVEL, 1, "[eMMC IPVerify Fail: %Xh]\n\n", u32_err);
	emmc_dump_driver_status();

	return u32_err;
}

U32 emmc_ipverify_main_ex(U32 u32_data_pattern)
{
	static U32 u32_start_sector = 0, u32_sector_cnt = 0, u32_err;

	u32_start_sector = EMMC_TEST_BLK_0;
	u32_sector_cnt = EMMC_TEST_BLK_CNT;

	emmc_debug(EMMC_DEBUG_LEVEL, 1, "Data Pattern: %08Xh\n", u32_data_pattern);

	//#if 0//because setting BIT_PAD_IN_SEL_SD and BIT_FALL_LATCH bit cause cifd crc error
	//if (emmc_drv.pad_type==FCIE_EMMC_BYPASS) {
		//u32_err = emmc_test_single_blk_wrc_riu(u32_start_sector, u32_data_pattern);
		//if (u32_err != EMMC_ST_SUCCESS)
			//emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "Err, emmc_test_single_blk_wrc_riu fail: %Xh\n", u32_err);

		//emmc_debug(EMMC_DEBUG_LEVEL, 1, "[emmc_test_single_blk_wrc_riu ok]\n");
	//}
	//#endif

	#if 1
	u32_err = emmc_test_single_blk_wrc_miu(u32_start_sector, u32_data_pattern);
	if (u32_err != EMMC_ST_SUCCESS) {
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "Err, emmc_test_single_blk_wrc_miu fail: %Xh\n", u32_err);
		goto LABEL_IP_VERIFY_ERROR;
	}
	emmc_debug(EMMC_DEBUG_LEVEL, 1, "[emmc_test_single_blk_wrc_miu ok]\n");
	#endif

	#if 1
	u32_err = emmc_test_multi_blk_wrc_miu(u32_start_sector, u32_sector_cnt, u32_data_pattern);
	if (u32_err != EMMC_ST_SUCCESS) {
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "Err, emmc_test_multi_blk_wrc_miu fail: %Xh\n", u32_err);
		goto LABEL_IP_VERIFY_ERROR;
	}
	emmc_debug(EMMC_DEBUG_LEVEL, 1, "[emmc_test_multi_blk_wrc_miu ok]\n");
	#endif

	//===============================================
	emmc_debug(EMMC_DEBUG_LEVEL, 1, "\n");
	emmc_debug(EMMC_DEBUG_LEVEL, 1, "[OK]\n\n");
	return EMMC_ST_SUCCESS;

LABEL_IP_VERIFY_ERROR:
	emmc_debug(EMMC_DEBUG_LEVEL, 1, "\n\n");
	emmc_debug(EMMC_DEBUG_LEVEL, 1, "Total Sec: %Xh, Test: StartSec: %Xh, SecCnt: %Xh\n",
		   emmc_drv.u32_sec_count, u32_start_sector, u32_sector_cnt);
	emmc_debug(EMMC_DEBUG_LEVEL, 1, "[eMMC IPVerify Fail: %Xh]\n\n", u32_err);
	emmc_dump_driver_status();

	return u32_err;
}

U32 emmc_ipverify_riu_main_ex(U32 u32_data_pattern)
{
	static U32 u32_start_sector = 0, u32_sector_cnt = 0, u32_err;

	//make StartSector SectorCnt random
	u32_start_sector = EMMC_TEST_BLK_0;
	u32_sector_cnt++;
	while (u32_sector_cnt > EMMC_TEST_BLK_CNT)
		u32_sector_cnt = 1;

	emmc_debug(EMMC_DEBUG_LEVEL, 1, "Data Pattern: %08Xh\n", u32_data_pattern);

	if (emmc_drv.pad_type == FCIE_EMMC_BYPASS) {
		u32_err = emmc_test_single_blk_wrc_riu(u32_start_sector, u32_data_pattern);
		if (u32_err != EMMC_ST_SUCCESS) {
			emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "Err, emmc_test_single_blk_wrc_riu fail: %Xh\n", u32_err);
			goto LABEL_IP_VERIFY_ERROR;
		}
		emmc_debug(EMMC_DEBUG_LEVEL, 1, "[emmc_test_single_blk_wrc_riu ok]\n");
	}

	//===============================================
	emmc_debug(EMMC_DEBUG_LEVEL, 1, "\n");
	emmc_debug(EMMC_DEBUG_LEVEL, 1, "[OK]\n\n");
	return EMMC_ST_SUCCESS;

LABEL_IP_VERIFY_ERROR:
	emmc_debug(EMMC_DEBUG_LEVEL, 1, "\n\n");
	emmc_debug(EMMC_DEBUG_LEVEL, 1, "Total Sec: %Xh, Test: StartSec: %Xh, SecCnt: %Xh\n",
		   emmc_drv.u32_sec_count, u32_start_sector, u32_sector_cnt);
	emmc_debug(EMMC_DEBUG_LEVEL, 1, "[eMMC IPVerify Fail: %Xh]\n\n", u32_err);
	emmc_dump_driver_status();

	return u32_err;
}

U32 emmc_ipverify_read_only(void)
{
	static U32 u32_start_sector = 0, u32_sector_cnt = 0, u32_err;

	//make StartSector SectorCnt random
	u32_start_sector = EMMC_TEST_BLK_0;
	u32_sector_cnt = EMMC_TEST_BLK_CNT;

	if (!gau8_rbuf)
		gau8_rbuf = (U8 *)memalign(EMMC_CACHE_LINE, TEST_BUFFER_SIZE);

	//#if 0
	//if (EMMC_IF_NORMAL_SDR()) {
		//u32_err = emmc_cmd17_cifd(u32_start_sector, gau8_rbuf);
		//if (u32_err != EMMC_ST_SUCCESS) {
			//emmc_debug(0, 1, "Err, CMD17_CIFD fail: %Xh\n", u32_err);
			//goto LABEL_IP_VERIFY_ERROR;
		//}
		//emmc_debug(EMMC_DEBUG_LEVEL_HIGH, 1, "[CMD17_CIFD ok]\n");
	//}
	//#endif

	//#if 0
	//u32_err = emmc_cmd17_miu(u32_start_sector, gau8_rbuf);
	//if (u32_err != EMMC_ST_SUCCESS) {
		//emmc_debug(0, 1, "Err, CMD17_MIU fail: %Xh\n", u32_err);
		//goto LABEL_IP_VERIFY_ERROR;
	//}
	//emmc_debug(EMMC_DEBUG_LEVEL_HIGH, 1, "[CMD17_MIU ok]\n");
	//#endif

	#if 1
	u32_err = emmc_cmd18_miu(u32_start_sector, gau8_rbuf, u32_sector_cnt);
	if (u32_err != EMMC_ST_SUCCESS) {
		emmc_debug(0, 1, "Err, CMD18_MIU fail: %Xh\n", u32_err);
		goto LABEL_IP_VERIFY_ERROR;
	}
	emmc_debug(EMMC_DEBUG_LEVEL_HIGH, 1, "[CMD18_MIU ok]\n");
	#endif
	if (gau8_rbuf) {
		free(gau8_rbuf);
		gau8_rbuf = 0;
	}
	//===============================================
	//emmc_debug(EMMC_DEBUG_LEVEL, 1, "\n");
	//emmc_debug(EMMC_DEBUG_LEVEL, 1, "[OK]\n\n");
	return EMMC_ST_SUCCESS;

LABEL_IP_VERIFY_ERROR:
	emmc_debug(EMMC_DEBUG_LEVEL, 1, "\n\n");
	emmc_debug(EMMC_DEBUG_LEVEL, 1, "Total Sec: %Xh, Test: StartSec: %Xh, SecCnt: %Xh\n",
		   emmc_drv.u32_sec_count, u32_start_sector, u32_sector_cnt);
	emmc_debug(EMMC_DEBUG_LEVEL, 1, "fail: %Xh\n\n", u32_err);
	emmc_dump_driver_status();
	if (gau8_rbuf) {
		free(gau8_rbuf);
		gau8_rbuf = 0;
	}
	return u32_err;
}


U32 emmc_ipverify_write_only(U16 u16_test_pattern)
{
	static U32 u32_start_sector = 0, u32_sector_cnt = 0, u32_err;
	U32 u32_i, *pu32_w, u32_byte_cnt;

	if (!gau8_wbuf)
		gau8_wbuf = (U8 *)memalign(EMMC_CACHE_LINE, TEST_BUFFER_SIZE);

	pu32_w = (U32 *)gau8_wbuf;

	//make StartSector SectorCnt random
	u32_start_sector = EMMC_TEST_BLK_0;
	u32_sector_cnt = EMMC_TEST_BLK_CNT;

	u32_byte_cnt = u32_sector_cnt << 9;

	//init data pattern
	for (u32_i = 0; u32_i < u32_byte_cnt >> 2; u32_i++)
		pu32_w[u32_i] = (u16_test_pattern << 16) + u16_test_pattern;

	//#if 0
	//if (EMMC_IF_NORMAL_SDR()) {
		//u32_err = emmc_cmd24_cifd(u32_start_sector, gau8_wbuf);
		//if (u32_err != EMMC_ST_SUCCESS) {
			//emmc_debug(0, 1, "Err, CMD24_CIFD fail: %Xh\n", u32_err);
			//goto LABEL_IP_VERIFY_ERROR;
		//}
		//emmc_debug(EMMC_DEBUG_LEVEL_HIGH, 1, "[CMD24_CIFD ok]\n");
	//}
	//#endif

	//#if 0
	//u32_err = emmc_cmd24_miu(u32_start_sector, gau8_wbuf);
	//if (u32_err != EMMC_ST_SUCCESS) {
		//emmc_debug(0, 1, "Err, CMD24_MIU fail: %Xh\n", u32_err);
		//goto LABEL_IP_VERIFY_ERROR;
	//}
	//emmc_debug(EMMC_DEBUG_LEVEL_HIGH, 1, "[CMD24_MIU ok]\n");
	//#endif

	#if 1
	u32_err = emmc_cmd25_miu(u32_start_sector, gau8_wbuf, u32_sector_cnt);
	if (u32_err != EMMC_ST_SUCCESS) {
		emmc_debug(0, 1, "Err, CMD25_MIU fail: %Xh\n", u32_err);
		goto LABEL_IP_VERIFY_ERROR;
	}
	emmc_debug(EMMC_DEBUG_LEVEL_HIGH, 1, "[CMD25_MIU ok]\n");
	#endif

	//===============================================
	//emmc_debug(EMMC_DEBUG_LEVEL, 1, "\n");
	//emmc_debug(EMMC_DEBUG_LEVEL, 1, "[OK]\n\n");
	if (gau8_wbuf) {
		free(gau8_wbuf);
		gau8_wbuf = 0;
	}

	return EMMC_ST_SUCCESS;

LABEL_IP_VERIFY_ERROR:
	emmc_debug(EMMC_DEBUG_LEVEL, 1, "\n\n");
	emmc_debug(EMMC_DEBUG_LEVEL, 1, "Total Sec: %Xh, Test: StartSec: %Xh, SecCnt: %Xh\n",
		   emmc_drv.u32_sec_count, u32_start_sector, u32_sector_cnt);
	emmc_debug(EMMC_DEBUG_LEVEL, 1, "fail: %Xh\n\n", u32_err);
	emmc_dump_driver_status();
	if (gau8_wbuf) {
		free(gau8_wbuf);
		gau8_wbuf = 0;
	}

	return u32_err;
}

U32 emmc_crazy_write_only(U16 *u16_pattern_test, U16 u16_pattern_cnt)
{
	U32 u32_i, u32_j, u32_err = EMMC_ST_SUCCESS;
	U32 u32_start_sector = EMMC_TEST_BLK_0, u32_sector_cnt = EMMC_TEST_BLK_CNT;
	U32 *pu32_w;

	if (!gau8_wbuf)
		gau8_wbuf = (U8 *)memalign(EMMC_CACHE_LINE, TEST_BUFFER_SIZE);

	pu32_w = (U32 *)gau8_wbuf;

	if (u16_pattern_cnt == 0)
		goto LABEL_IP_VERIFY_ERROR;

	//init data pattern
	for (u32_i = 0; u32_i < (TEST_BUFFER_SIZE / u16_pattern_cnt) >> 2; u32_i++) {
		for (u32_j = 0; u32_j < u16_pattern_cnt; u32_j++)
			pu32_w[u32_i * u16_pattern_cnt + u32_j] =
				(u16_pattern_test[u32_j] << 16) + u16_pattern_test[u32_j];
	}

	u32_err = emmc_cmd25_miu(u32_start_sector, (U8 *)pu32_w, u32_sector_cnt);
	if (u32_err != EMMC_ST_SUCCESS) {
		emmc_debug(0, 1, "Err, CMD25_MIU fail: %Xh\n", u32_err);
		goto LABEL_IP_VERIFY_ERROR;
	}

	if (gau8_wbuf) {
		free(gau8_wbuf);
		gau8_wbuf = 0;
	}

	return EMMC_ST_SUCCESS;
LABEL_IP_VERIFY_ERROR:
	emmc_debug(EMMC_DEBUG_LEVEL, 1, "fail: %Xh\n\n", u32_err);
	emmc_dump_driver_status();
	if (gau8_wbuf) {
		free(gau8_wbuf);
		gau8_wbuf = 0;
	}


	return u32_err;
}

U32 emmc_crazy_single_write_only(U16 *u16_pattern_test, U16 u16_pattern_cnt)
{
	U32 u32_i, u32_j, u32_err = EMMC_ST_SUCCESS;
	U32 u32_start_sector = EMMC_TEST_BLK_0;
	U32 *pu32_w;
	U8  *pu8_w;

	if (!gau8_wbuf)
		gau8_wbuf = (U8 *)memalign(EMMC_CACHE_LINE, TEST_BUFFER_SIZE);

	pu32_w = (U32 *)gau8_wbuf;
	pu8_w = gau8_wbuf;

	if (u16_pattern_cnt == 0)
		goto LABEL_IP_VERIFY_ERROR;

	//init data pattern
	for (u32_i = 0; u32_i < (TEST_BUFFER_SIZE / u16_pattern_cnt) >> 2; u32_i++) {
		for (u32_j = 0; u32_j < u16_pattern_cnt; u32_j++)
			pu32_w[u32_i * u16_pattern_cnt + u32_j] =
				(u16_pattern_test[u32_j] << 16) + u16_pattern_test[u32_j];
	}

	for (u32_i = 0; u32_i < (TEST_BUFFER_SIZE / 512) ; u32_i++) {
		u32_err = emmc_cmd24_miu(u32_start_sector + u32_i, (U8 *)(pu8_w + u32_i * 512));

		if (u32_err != EMMC_ST_SUCCESS) {
			emmc_debug(0, 1, "Err, CMD24_MIU fail: %Xh\n", u32_err);
			goto LABEL_IP_VERIFY_ERROR;
		}
		emmc_debug(0, 1, "[CMD24_MIU ok]\n");
	}

	if (gau8_wbuf) {
		free(gau8_wbuf);
		gau8_wbuf = 0;
	}

	return EMMC_ST_SUCCESS;

LABEL_IP_VERIFY_ERROR:
	emmc_debug(EMMC_DEBUG_LEVEL, 1, "fail: %Xh\n\n", u32_err);
	emmc_dump_driver_status();
	if (gau8_wbuf) {
		free(gau8_wbuf);
		gau8_wbuf = 0;
	}

	return u32_err;
}

U32 emmc_ipverify_main_big(void)
{

	emmc_ipverify_main_big_ex(EMMC_PATTERN_FFFFFFFF);
	emmc_ipverify_main_big_ex(EMMC_PATTERN_00000000);
	emmc_debug(EMMC_DEBUG_LEVEL, 0, "normal pattern test ok\n\n");

	emmc_ipverify_main_big_ex(EMMC_PATTERN_000000FF);
	emmc_ipverify_main_big_ex(EMMC_PATTERN_0000FFFF);
	emmc_ipverify_main_big_ex(EMMC_PATTERN_AA55AA55);
	emmc_ipverify_main_big_ex(EMMC_PATTERN_AA55AA55);
	emmc_debug(EMMC_DEBUG_LEVEL, 0, "specific pattern test ok\n\n");

	//===============================================
	//emmc_debug(EMMC_DEBUG_LEVEL, 1, "\n");
	//emmc_debug(EMMC_DEBUG_LEVEL, 1, "[OK]\n\n");
	return EMMC_ST_SUCCESS;
}

static void emmc_test_life_time_fixed(void)
{
	U32 u32_blk_cnt = 0, u32_write_gb = 0;
	U32 u32_i, u32_err = EMMC_ST_SUCCESS;

	while (1) {
		emmc_reset_watch_dog();

		for (u32_i = 0; u32_i < EMMC_SECTOR_BYTECNT; u32_i++)
			gau8_wbuf[u32_i] = u32_blk_cnt + u32_i;

		u32_err = emmc_cmd24_miu(TEST_START_ADDR >> EMMC_SECTOR_BYTECNT_BITS, gau8_wbuf);
		if (u32_err != EMMC_ST_SUCCESS) {
			emmc_debug(0, 1, "Err, CMD24_MIU, %Xh\n", u32_err);
			break;
		}

		u32_err = emmc_cmd18_miu(0, gau8_rbuf, TEST_BUFFER_SIZE >> 9);
		if (u32_err != EMMC_ST_SUCCESS) {
			emmc_debug(0, 1, "Err, CMD18_MIU, %Xh\n", u32_err);
			break;
		}
		u32_err = emmc_cmd18_miu(100, gau8_rbuf, TEST_BUFFER_SIZE >> 9);
		if (u32_err != EMMC_ST_SUCCESS) {
			emmc_debug(0, 1, "Err, CMD18_MIU, %Xh\n", u32_err);
			break;
		}

		u32_err = emmc_cmd17_miu(TEST_START_ADDR >> EMMC_SECTOR_BYTECNT_BITS, gau8_rbuf);
		if (u32_err != EMMC_ST_SUCCESS) {
			emmc_debug(0, 1, "Err, CMD17_MIU, %Xh\n", u32_err);
			break;
		}

		u32_err = emmc_compare_data(gau8_wbuf, gau8_rbuf, EMMC_SECTOR_BYTECNT);
		if (u32_err != EMMC_ST_SUCCESS) {
			emmc_debug(0, 1, "Err, ComapreData, %Xh\n", u32_err);
			break;
		}

		u32_blk_cnt += 1;
		if (((u32_blk_cnt << EMMC_SECTOR_BYTECNT_BITS) & 0x3FFFFFFF) == 0) {
			u32_write_gb++;
			emmc_debug(0, 1, "Write %u GBytes, Life: [267]:%Xh [268]:%Xh [269]:%Xh\n",
				   u32_write_gb, au8_ext_csd[EXT_CSD_PRE_EOL_INFO],
				   au8_ext_csd[EXT_CSD_LIFE_TIME_TYPA],
				   au8_ext_csd[EXT_CSD_LIFE_TIME_TYPB]);
			emmc_drv.life_bytes_cnt_t.u32_life_time_test_fixed_gbytes = u32_write_gb;
			emmc_drv.life_bytes_cnt_t.u32_chksum =
				emmc_chk_sum((U8 *)&emmc_drv.life_bytes_cnt_t, sizeof(emmc_drv.life_bytes_cnt_t) - 4);
			memset(gau8_wbuf, 0, EMMC_SECTOR_BYTECNT);
			memcpy(gau8_wbuf, (U8 *)&emmc_drv.life_bytes_cnt_t, sizeof(emmc_drv.life_bytes_cnt_t));
			u32_err = emmc_cmd24_miu(EMMC_LIFE_TEST_BYTE_CNT_BLK, gau8_wbuf);
			if (u32_err != EMMC_ST_SUCCESS) {
				emmc_debug(0, 1, "Err, CMD24_MIU, %Xh\n", u32_err);
				break;
			}
		}
	}

	emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 0, "\neMMC Err: fail: %Xh\n", u32_err);
	emmc_fcie_err_handler_stop();
}

static void emmc_test_life_time_filled(void)
{
	U32 u32_write_gb = 0, u32_loop_cnt = 0;
	U32 u32_i, u32_err = EMMC_ST_SUCCESS;
	U32 tmp_mb, tmp_gb = 0;

	while (1) {
		emmc_reset_watch_dog();

		for (u32_i = 0; u32_i < TEST_BUFFER_SIZE; u32_i++)
			gau8_wbuf[u32_i] = u32_loop_cnt + u32_i;

		tmp_mb = 0;
		for (u32_i = TEST_START_ADDR >> 9; u32_i < emmc_drv.u32_sec_count;) {
			u32_err = emmc_cmd25_miu(u32_i, gau8_wbuf, TEST_BUFFER_SIZE >> 9);
			if (u32_err != EMMC_ST_SUCCESS) {
				emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 0, "\neMMC Err: w fail: %Xh\n", u32_err);
				emmc_debug(EMMC_DEBUG_LEVEL, 0, "eMMC: (%Xh x %Xh) blocks tested, stop\n",
					   u32_loop_cnt, emmc_drv.u32_sec_count);
				emmc_fcie_err_handler_stop();
			}

			u32_i += TEST_BUFFER_SIZE >> EMMC_SECTOR_BYTECNT_BITS;
			if (emmc_drv.u32_sec_count - u32_i < (TEST_BUFFER_SIZE >> 9))
				break;

			tmp_mb += TEST_BUFFER_SIZE >> 20;
			if (tmp_mb % 0x400 == 0) {
				tmp_gb++;
				tmp_mb = 0;
			}
			if ((tmp_mb % 100) == 0)
				emmc_debug(EMMC_DEBUG_LEVEL, 0, "\r write %u GB, %u MB ......", tmp_gb, tmp_mb);
		}

		tmp_mb = 0;
		for (u32_i = TEST_START_ADDR >> 9; u32_i < emmc_drv.u32_sec_count;) {
			u32_err = emmc_cmd18_miu(u32_i, gau8_rbuf, TEST_BUFFER_SIZE >> 9);
			if (u32_err != EMMC_ST_SUCCESS) {
				emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 0, "\neMMC Err: r fail: %Xh\n", u32_err);
				emmc_debug(EMMC_DEBUG_LEVEL, 0, "eMMC: (%Xh x %Xh) blocks tested, stop\n",
					   u32_loop_cnt, emmc_drv.u32_sec_count);
				emmc_fcie_err_handler_stop();
			}

			u32_err = emmc_compare_data(gau8_wbuf, gau8_rbuf, TEST_BUFFER_SIZE);
			if (u32_err != EMMC_ST_SUCCESS) {
				emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 0, "\neMMC Err: c fail: %Xh\n", u32_err);
				emmc_debug(EMMC_DEBUG_LEVEL, 0, "eMMC: (%Xh x %Xh) blocks tested, stop\n",
					   u32_loop_cnt, emmc_drv.u32_sec_count);
				emmc_fcie_err_handler_stop();
			}

			u32_i += TEST_BUFFER_SIZE >> EMMC_SECTOR_BYTECNT_BITS;
			if (emmc_drv.u32_sec_count - u32_i < (TEST_BUFFER_SIZE >> 9))
				break;

			tmp_mb += TEST_BUFFER_SIZE >> 20;
			if ((tmp_mb % 100) == 0)
				emmc_debug(EMMC_DEBUG_LEVEL, 0, "\r read & check %u%% ... ",
					   u32_i * 100 / emmc_drv.u32_sec_count + 1);
		}

		u32_loop_cnt += 1;
		u32_write_gb += ((emmc_drv.u32_sec_count - (TEST_START_ADDR >> 9)) >> (30 - EMMC_SECTOR_BYTECNT_BITS));
		emmc_debug(0, 1, "Write %u GBytes, Life: [267]:%Xh [268]:%Xh [269]:%Xh\n",
			   u32_write_gb,
			   au8_ext_csd[EXT_CSD_PRE_EOL_INFO],
			   au8_ext_csd[EXT_CSD_LIFE_TIME_TYPA],
			   au8_ext_csd[EXT_CSD_LIFE_TIME_TYPB]);
		emmc_drv.life_bytes_cnt_t.u32_life_time_test_filled_gbytes = u32_write_gb;
		emmc_drv.life_bytes_cnt_t.u32_chksum =
			emmc_chk_sum((U8 *)&emmc_drv.life_bytes_cnt_t, sizeof(emmc_drv.life_bytes_cnt_t) - 4);
		memset(gau8_wbuf, 0, EMMC_SECTOR_BYTECNT);
		memcpy(gau8_wbuf, (U8 *)&emmc_drv.life_bytes_cnt_t, sizeof(emmc_drv.life_bytes_cnt_t));
		u32_err = emmc_cmd24_miu(EMMC_LIFE_TEST_BYTE_CNT_BLK, gau8_wbuf);
		if (u32_err != EMMC_ST_SUCCESS) {
			emmc_debug(0, 1, "Err, CMD24_MIU, %Xh\n", u32_err);
			emmc_fcie_err_handler_stop();
		}
	}
}

static void emmc_test_life_time_random(U8 u8_test_mode)
{
	U32 u32_i, u32_err = EMMC_ST_SUCCESS, u32_blk_addr;
	U16 u16_blk_cnt;
	U32 u32_loop_cnt = 0, u32_write_gb = 0, u32_wbyte = 0;

	while (1) {
		emmc_reset_watch_dog();

		u32_blk_addr = emmc_hw_timer_tick() * 1000;
		u32_blk_addr %= emmc_drv.u32_sec_count;
		if (emmc_drv.u32_sec_count - u32_blk_addr < (TEST_BUFFER_SIZE >> 9))
			u32_blk_addr = emmc_drv.u32_sec_count - (TEST_BUFFER_SIZE >> 9) - 1;

		if (u32_blk_addr < TEST_START_ADDR >> 9)
			u32_blk_addr += TEST_START_ADDR >> 9;

		u32_blk_addr &= ~0xFFF;

		if (u8_test_mode == EMMC_LIFETIME_TEST_RANDOM) {
			if ((u32_blk_addr / 0x1000) % 10 < 5)
				u16_blk_cnt = 0x1000 >> EMMC_SECTOR_BYTECNT_BITS;
			else if ((u32_blk_addr / 0x1000) % 10 < 8)
				u16_blk_cnt = 0x2000 >> EMMC_SECTOR_BYTECNT_BITS;
			else
				u16_blk_cnt = 0x4000 >> EMMC_SECTOR_BYTECNT_BITS;
		} else {
			if ((u32_blk_addr / 0x1000) % 10 < 5)
				u16_blk_cnt = TEST_BUFFER_SIZE >> EMMC_SECTOR_BYTECNT_BITS;
			else if ((u32_blk_addr / 0x1000) % 10 < 8)
				u16_blk_cnt = (TEST_BUFFER_SIZE / 2) >> EMMC_SECTOR_BYTECNT_BITS;
			else
				u16_blk_cnt = (TEST_BUFFER_SIZE / 4) >> EMMC_SECTOR_BYTECNT_BITS;
		}

		for (u32_i = 0; u32_i < u16_blk_cnt << 9; u32_i++)
			gau8_wbuf[u32_i] = u32_loop_cnt + u32_i;

		//emmc_debug(0,1,"Random Addr: %8lXh  Cnt: %3Xh\n", u32_blk_addr, u16_blk_cnt);

		u32_err = emmc_cmd25_miu(u32_blk_addr, gau8_wbuf, u16_blk_cnt);
		if (u32_err != EMMC_ST_SUCCESS) {
			emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 0, "\neMMC Err: w fail: %Xh, BlkAddr: %Xh\n", u32_err, u32_blk_addr);
			emmc_debug(EMMC_DEBUG_LEVEL, 0, "eMMC: (%Xh x %Xh) blocks tested, stop\n",
				   u32_loop_cnt, emmc_drv.u32_sec_count);
			emmc_fcie_err_handler_stop();
		}

		u32_err = emmc_cmd18_miu(u32_blk_addr, gau8_rbuf, u16_blk_cnt);
		if (u32_err != EMMC_ST_SUCCESS) {
			emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 0, "\neMMC Err: r fail: %Xh, BlkAddr: %Xh\n", u32_err, u32_blk_addr);
			emmc_debug(EMMC_DEBUG_LEVEL, 0, "eMMC: (%Xh x %Xh) blocks tested, stop\n",
				   u32_loop_cnt, emmc_drv.u32_sec_count);
			emmc_fcie_err_handler_stop();
		}
		u32_err = emmc_compare_data(gau8_wbuf, gau8_rbuf, u16_blk_cnt << 9);
		if (u32_err != EMMC_ST_SUCCESS) {
			emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 0, "\neMMC Err: c fail: %Xh, BlkAddr: %Xh\n", u32_err, u32_blk_addr);
			emmc_debug(EMMC_DEBUG_LEVEL, 0, "eMMC: (%Xh x %Xh) blocks tested, stop\n",
				   u32_loop_cnt, emmc_drv.u32_sec_count);
			emmc_fcie_err_handler_stop();
		}

		u32_loop_cnt += 1;
		u32_wbyte += u16_blk_cnt << EMMC_SECTOR_BYTECNT_BITS;
		//1GB
		if (u32_wbyte >= 0x40000000) {
			u32_wbyte -= 0x40000000;
			u32_write_gb++;
			emmc_ext_csd_config();
			emmc_debug(0, 1, "Write %u GBytes, Life: [267]:%Xh [268]:%Xh [269]:%Xh\n",
				   u32_write_gb,
				   au8_ext_csd[EXT_CSD_PRE_EOL_INFO],
				   au8_ext_csd[EXT_CSD_LIFE_TIME_TYPA],
				   au8_ext_csd[EXT_CSD_LIFE_TIME_TYPB]);
			emmc_drv.life_bytes_cnt_t.u32_life_time_test_random_gbytes = u32_write_gb;
			emmc_drv.life_bytes_cnt_t.u32_chksum = emmc_chk_sum((U8 *)&emmc_drv.life_bytes_cnt_t,
									    sizeof(emmc_drv.life_bytes_cnt_t) - sizeof(U32));
			memset(gau8_wbuf, 0, EMMC_SECTOR_BYTECNT);
			memcpy(gau8_wbuf, (U8 *)&emmc_drv.life_bytes_cnt_t, sizeof(emmc_drv.life_bytes_cnt_t));
			u32_err = emmc_cmd24_miu(EMMC_LIFE_TEST_BYTE_CNT_BLK, gau8_wbuf);
			if (u32_err != EMMC_ST_SUCCESS) {
				emmc_debug(0, 1, "Err, CMD24_MIU, %Xh\n", u32_err);
				emmc_fcie_err_handler_stop();
			}
			break;
		}
	}
}

U32 emmc_test_life_time(U8 u8_test_mode)
{
	U32 u32_err = EMMC_ST_SUCCESS;
	U32 u32_write_gb = 0;
	U32 u32_chksum = 0;

	emmc_debug(EMMC_DEBUG_LEVEL, 0, "===========================================\n");
	emmc_debug(EMMC_DEBUG_LEVEL, 0, "CAUTION: will damage images in eMMC\n");
	emmc_debug(EMMC_DEBUG_LEVEL, 0, "===========================================\n");

	if (!gau8_wbuf)
		gau8_wbuf = (U8 *)memalign(EMMC_CACHE_LINE, TEST_BUFFER_SIZE);

	if (!gau8_rbuf)
		gau8_rbuf = (U8 *)memalign(EMMC_CACHE_LINE, TEST_BUFFER_SIZE);

	//-------------------------------------------------
	u32_err = emmc_cmd17_miu(EMMC_LIFE_TEST_BYTE_CNT_BLK, gau8_rbuf);
	if (u32_err != EMMC_ST_SUCCESS) {
		emmc_debug(0, 1, "Err, CMD17_MIU, %Xh\n", u32_err);
		emmc_fcie_err_handler_stop();
	}
	memcpy((U8 *)&emmc_drv.life_bytes_cnt_t, gau8_rbuf, sizeof(emmc_drv.life_bytes_cnt_t));
	u32_chksum = emmc_chk_sum((U8 *)&emmc_drv.life_bytes_cnt_t, sizeof(emmc_drv.life_bytes_cnt_t) - 4);

	if (u8_test_mode == EMMC_LIFETIME_TEST_FIXED) {
		emmc_debug(EMMC_DEBUG_LEVEL, 1, "Fixed Addr Mode, fixed addr: %u th block\n", TEST_START_ADDR >> 9);
		if (u32_chksum != emmc_drv.life_bytes_cnt_t.u32_chksum) {
			u32_write_gb = 0;
			emmc_debug(0, 1, "No Life Bytes Count Store in eMMC\n");
		} else {
			u32_write_gb = emmc_drv.life_bytes_cnt_t.u32_life_time_test_fixed_gbytes;
			emmc_debug(0, 1, "Life Bytes Count Store in eMMC %u GBytes\n", u32_write_gb);
		}
		emmc_test_life_time_fixed();
	} else if (u8_test_mode == EMMC_LIFETIME_TEST_FILLED) {
		emmc_debug(EMMC_DEBUG_LEVEL, 1, "Filled Mode\n");

		if (u32_chksum != emmc_drv.life_bytes_cnt_t.u32_chksum) {
			u32_write_gb = 0;
			emmc_debug(0, 1, "No Life Bytes Count Store in eMMC\n");
		} else {
			u32_write_gb = emmc_drv.life_bytes_cnt_t.u32_life_time_test_filled_gbytes;
			emmc_debug(0, 1, "Life Bytes Count Store in eMMC %u GBytes\n", u32_write_gb);
		}
		emmc_test_life_time_filled();
	} else if (u8_test_mode == EMMC_LIFETIME_TEST_RANDOM || u8_test_mode == EMMC_LIFETIME_TEST_RANDOM_BIG) {
		emmc_hw_timer_start();//use to get tick as a random seed

		if (u8_test_mode == EMMC_LIFETIME_TEST_RANDOM)
			emmc_debug(EMMC_DEBUG_LEVEL, 1, "Random Mode - Small Data\n");
		else
			emmc_debug(EMMC_DEBUG_LEVEL, 1, "Random Mode - Large Data\n");

		if (u32_chksum != emmc_drv.life_bytes_cnt_t.u32_chksum) {
			u32_write_gb = 0;
			emmc_debug(0, 1, "No Life Bytes Count Store in eMMC\n");
		} else {
			u32_write_gb = emmc_drv.life_bytes_cnt_t.u32_life_time_test_random_gbytes;
			emmc_debug(0, 1, "Life Bytes Count Store in eMMC %u GBytes\n", u32_write_gb);
		}
		emmc_test_life_time_random(u8_test_mode);
	}

	return u32_err;
}

U32 emmc_ipverify_sdr_ddr_all_clk_temp(void)
{
	U32 u32_err;

	u32_err = emmc_fcie_choose_speed_mode();
	if (u32_err != EMMC_ST_SUCCESS) {
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: ChooseSpeedMode fail: %Xh\n", u32_err);
		return u32_err;
	}
	emmc_dump_timing_table();

	while (1) {
		#if 1
		//emmc_hw_timer_delay(HW_TIMER_DELAY_1s);
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "\n");
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "======================================\n");
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "SDR\n");
		u32_err = emmc_fcie_enable_sdr_mode();
		if (u32_err != EMMC_ST_SUCCESS) {
			emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: EnableSDRMode fail: %Xh\n", u32_err);
			emmc_dump_driver_status();
			return u32_err;
		}

		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "======================================\n");
		u32_err = emmc_fcie_choose_speed_mode();
		if (u32_err != EMMC_ST_SUCCESS) {
			emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: ChooseSpeedMode fail: %Xh\n", u32_err);
			emmc_dump_driver_status();
			return u32_err;
		}
		emmc_ipverify_main_ex(EMMC_PATTERN_FFFFFFFF);
		emmc_ipverify_main_ex(EMMC_PATTERN_00000000);
		emmc_ipverify_main_ex(EMMC_PATTERN_000000FF);
		emmc_ipverify_main_ex(EMMC_PATTERN_0000FFFF);
		emmc_ipverify_main_ex(EMMC_PATTERN_AA55AA55);
		emmc_ipverify_main_ex(EMMC_PATTERN_AA55AA55);

		emmc_ipverify_main_sg_ex(EMMC_PATTERN_FFFFFFFF);
		emmc_ipverify_main_sg_ex(EMMC_PATTERN_00000000);
		emmc_ipverify_main_sg_ex(EMMC_PATTERN_000000FF);
		emmc_ipverify_main_sg_ex(EMMC_PATTERN_0000FFFF);
		emmc_ipverify_main_sg_ex(EMMC_PATTERN_AA55AA55);
		emmc_ipverify_main_sg_ex(EMMC_PATTERN_AA55AA55);
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "\n");
	}

	return EMMC_ST_SUCCESS;
}

#define EMMC_PWRCUT_DATA0      0x55
#define EMMC_PWRCUT_DATA1      0xAA
#define EMMC_PWRCUT_DATA_CNT   2

#define EMMC_PWRCUT_TEST_UNIT_BYTECNT    0x1000000
#define EMMC_PWRCUT_TEST_SPACE_BYTECNT   (16 * EMMC_PWRCUT_TEST_UNIT_BYTECNT)//256MB
#define EMMC_PWRCUT_TEST_UNIT_CNT        (EMMC_PWRCUT_TEST_SPACE_BYTECNT / EMMC_PWRCUT_TEST_UNIT_BYTECNT)

struct emmc_pwr_cut_ctrl_blk {

	U32 u32_check_sum;
	U8  u8_target_unit;
	U8  au8_target_data[EMMC_PWRCUT_TEST_UNIT_CNT];

} EMMC_PACK1;

void emmc_dump_pwr_cut_ctrl_blk(struct emmc_pwr_cut_ctrl_blk *p_ctrl_blk_t)
{
	U32 u32_i;

	emmc_debug(EMMC_DEBUG_LEVEL, 0, "eMMC: CheckSum: %Xh\n", p_ctrl_blk_t->u32_check_sum);
	emmc_debug(EMMC_DEBUG_LEVEL, 0, "eMMC: TargetUnit: %Xh\n", p_ctrl_blk_t->u8_target_unit);

	emmc_debug(EMMC_DEBUG_LEVEL, 0, "  Target Data: ");
	for (u32_i = 0; u32_i < EMMC_PWRCUT_TEST_UNIT_CNT; u32_i++) {
		if ((u32_i & 0xF) == 0)
			emmc_debug(EMMC_DEBUG_LEVEL, 0, "\n");

		emmc_debug(EMMC_DEBUG_LEVEL, 0, " %Xh", p_ctrl_blk_t->au8_target_data[u32_i]);
	}
	emmc_debug(EMMC_DEBUG_LEVEL, 0, "\n");
}

U32 emmc_check_pwr_cut_ctrl_blk(U8 *u8_data_buf, U32 u32_blk_addr)
{
	U32 u32_err;
	struct emmc_pwr_cut_ctrl_blk *p_ctrl_blk_t;

	u32_err = emmc_read_data(u8_data_buf, EMMC_SECTOR_512BYTE, u32_blk_addr);
	if (u32_err != EMMC_ST_SUCCESS) {
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: read CtrlBlk, Err:%x\n", u32_err);
		return u32_err;
	}

	p_ctrl_blk_t = (struct emmc_pwr_cut_ctrl_blk *)u8_data_buf;

	if (p_ctrl_blk_t->u32_check_sum != emmc_chk_sum(u8_data_buf + 4, sizeof(struct emmc_pwr_cut_ctrl_blk) - 4)) {
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: read CtrlBlk, CheckSum fail\n");
		return EMMC_ST_ERR_CHKSUM;
	}

	return EMMC_ST_SUCCESS;
}

U32 emmc_test_pwr_cut_init_data(U8 *u8_data_buf, U32 u32_blk_start_addr)
{
	U32 u32_i, u32_j;
	U32 u32_err = EMMC_ST_SUCCESS;
	struct emmc_pwr_cut_ctrl_blk ctrl_blk_t;
	struct mmc *mmc = find_mmc_device(0);

	if (!mmc)
		return u32_err;

	//mmc_slc_mode(mmc, 0, 1);//enable reliable
	//---------------------------------------
	ctrl_blk_t.u8_target_unit = 0xFF;

	for (u32_i = 0; u32_i < EMMC_PWRCUT_TEST_UNIT_CNT; u32_i++)
		ctrl_blk_t.au8_target_data[u32_i] = EMMC_PWRCUT_DATA0;

	ctrl_blk_t.u32_check_sum = emmc_chk_sum(&ctrl_blk_t.au8_target_data[0],
						sizeof(ctrl_blk_t.au8_target_data));

	ctrl_blk_t.u32_check_sum += ctrl_blk_t.u8_target_unit;

	u32_err = emmc_write_data((U8 *)&ctrl_blk_t, EMMC_SECTOR_512BYTE, u32_blk_start_addr);
	if (u32_err != EMMC_ST_SUCCESS) {
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: write CtrlBlk, Err:%x\n", u32_err);
		return u32_err;
	}

	//---------------------------------------
	u32_blk_start_addr += 1;//skip Ctrl Blk

	memset(u8_data_buf, EMMC_PWRCUT_DATA0, EMMC_PWRCUT_TEST_UNIT_BYTECNT);
	emmc_debug(EMMC_DEBUG_LEVEL, 1, "Reliable write ...\n");

	for (u32_i = 0; u32_i < EMMC_PWRCUT_TEST_UNIT_CNT; u32_i++) {
		u32_err = emmc_write_data(u8_data_buf, EMMC_PWRCUT_TEST_UNIT_BYTECNT,
					  u32_blk_start_addr + u32_i * (EMMC_PWRCUT_TEST_UNIT_BYTECNT >> 9));
		if (u32_err != EMMC_ST_SUCCESS) {
			emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
				   "eMMC Err: Reliable write Fail @ %u unit, Err:%x\n", u32_i, u32_err);
			return u32_err;
		}
		emmc_debug(EMMC_DEBUG_LEVEL, 0, "%03u%% \r", (u32_i + 1) * 100 / EMMC_PWRCUT_TEST_UNIT_CNT);
	}

	//---------------------------------------
	emmc_debug(EMMC_DEBUG_LEVEL, 0, "\ncheck ...\n");

	if (emmc_check_pwr_cut_ctrl_blk(u8_data_buf, u32_blk_start_addr - 1) != EMMC_ST_SUCCESS) {
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: read CtrlBlk fail\n");
		return EMMC_ST_ERR_CHKSUM;
	}

	for (u32_i = 0; u32_i < EMMC_PWRCUT_TEST_UNIT_CNT; u32_i++) {
		u32_err = emmc_read_data(u8_data_buf, EMMC_PWRCUT_TEST_UNIT_BYTECNT,
					 u32_blk_start_addr + u32_i * (EMMC_PWRCUT_TEST_UNIT_BYTECNT >> 9));
		if (u32_err != EMMC_ST_SUCCESS) {
			emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
				   "eMMC Err: Reliable write Fail @ %u unit, Err:%x\n", u32_i, u32_err);
			return u32_err;
		}

		for (u32_j = 0; u32_j < EMMC_PWRCUT_TEST_UNIT_BYTECNT; u32_j++) {
			if (u8_data_buf[u32_j] != EMMC_PWRCUT_DATA0) {
				emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
					   "eMMC Err: Data Mismatch:  Blk:%Xh  ByteIdx:%Xh  ByteVal:%Xh\n",
					   u32_blk_start_addr + u32_i * (EMMC_PWRCUT_TEST_UNIT_BYTECNT >> 9) + (u32_j >> 9),
					   u32_j & 0x1FF, u8_data_buf[u32_j]);
				return EMMC_ST_ERR_DATA_MISMATCH;
			}
		}
		emmc_debug(EMMC_DEBUG_LEVEL, 0, "%03u%%\r", (u32_i + 1) * 100 / EMMC_PWRCUT_TEST_UNIT_CNT);
	}

	emmc_debug(EMMC_DEBUG_LEVEL, 0, "\ninit success\n");
	return EMMC_ST_SUCCESS;
}

U32 emmc_test_pwr_cut_test(U8 *u8_data_buf, U32 u32_blk_start_addr)
{
	U32 u32_i, u32_j;
	U32 u32_err = EMMC_ST_SUCCESS;
	U32 u32_T0, u32_blk_addr;
	struct emmc_pwr_cut_ctrl_blk ctrl_blk_t;

	emmc_hw_timer_start();
	//---------------------------------------
	emmc_debug(EMMC_DEBUG_LEVEL, 0, "\neMMC PwrCut Test, checking ...\n");

	//get Ctrl Blk
	u32_err = emmc_check_pwr_cut_ctrl_blk(u8_data_buf, u32_blk_start_addr);
	if (u32_err != EMMC_ST_SUCCESS) {
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: read CtrlBlk fail\n");
		return u32_err;
	}
	memcpy(&ctrl_blk_t, u8_data_buf, sizeof(struct emmc_pwr_cut_ctrl_blk));

	//check data
	u32_blk_start_addr++;

	for (u32_i = 0; u32_i < EMMC_PWRCUT_TEST_UNIT_CNT; u32_i++) {
		u32_err = emmc_read_data(u8_data_buf, EMMC_PWRCUT_TEST_UNIT_BYTECNT,
					 u32_blk_start_addr + u32_i * (EMMC_PWRCUT_TEST_UNIT_BYTECNT >> 9));
		if (u32_err != EMMC_ST_SUCCESS) {
			emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 0,
				   "\neMMC Err: Reliable write Fail @ %u unit, Err:%x\n", u32_i, u32_err);
			return u32_err;
		}

		if (u32_i == ctrl_blk_t.u8_target_unit) {
			for (u32_j = 0; u32_j < EMMC_PWRCUT_TEST_UNIT_BYTECNT; u32_j++) {
				if (u8_data_buf[u32_j] != EMMC_PWRCUT_DATA0 && u8_data_buf[u32_j] != EMMC_PWRCUT_DATA1) {
					emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 0, "\neMMC Err: Target Data Mismatch:  Unit:%Xh  ",
						   u32_i);
					emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 0,
						   "Blk: %Xh + %Xh x %Xh + %Xh = %Xh, ByteIdx:%Xh ByteVal:%Xh\n",
						   u32_blk_start_addr, u32_i, (EMMC_PWRCUT_TEST_UNIT_BYTECNT >> 9),
						   (u32_j >> 9),
						   u32_blk_start_addr + u32_i * (EMMC_PWRCUT_TEST_UNIT_BYTECNT >> 9) +
						   (u32_j >> 9),
						   u32_j & 0x1FF, u8_data_buf[u32_j]);
					emmc_dump_pwr_cut_ctrl_blk(&ctrl_blk_t);
					emmc_dump_mem(&u8_data_buf[u32_j] - 0x10, 0x30);
					u32_err = EMMC_ST_ERR_DATA_MISMATCH;
					return u32_err;
				}
			}
			//recover TargetUnit
			u32_blk_addr = u32_blk_start_addr + ctrl_blk_t.u8_target_unit *
				(EMMC_PWRCUT_TEST_UNIT_BYTECNT >> EMMC_SECTOR_512BYTE_BITS);
			memset(u8_data_buf, ctrl_blk_t.au8_target_data[ctrl_blk_t.u8_target_unit], EMMC_PWRCUT_TEST_UNIT_BYTECNT);
			u32_err = emmc_write_data(u8_data_buf, EMMC_PWRCUT_TEST_UNIT_BYTECNT, u32_blk_addr);
			if (u32_err != EMMC_ST_SUCCESS) {
				emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: write TargetUnit fail, Err:%x\n", u32_err);
				return u32_err;
			}
		} else {
			for (u32_j = 0; u32_j < EMMC_PWRCUT_TEST_UNIT_BYTECNT; u32_j++) {
				if (u8_data_buf[u32_j] != ctrl_blk_t.au8_target_data[u32_i]) {
					emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 0, "\neMMC Err: Target Data Mismatch:  Unit:%Xh  ",
						   u32_i);
					emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 0,
						   "Blk: %Xh + %Xh x %Xh + %Xh = %Xh, ByteIdx:%Xh  ByteVal:%Xh\n",
						   u32_blk_start_addr, u32_i, (EMMC_PWRCUT_TEST_UNIT_BYTECNT >> 9), (u32_j >> 9),
						   u32_blk_start_addr + u32_i * (EMMC_PWRCUT_TEST_UNIT_BYTECNT >> 9) + (u32_j >> 9),
						   u32_j & 0x1FF, u8_data_buf[u32_j]);
					emmc_dump_pwr_cut_ctrl_blk(&ctrl_blk_t);
					emmc_dump_mem(&u8_data_buf[u32_j] - 0x10, 0x30);
					u32_err = EMMC_ST_ERR_DATA_MISMATCH;
					return u32_err;
				}
			}
		}
		emmc_debug(EMMC_DEBUG_LEVEL, 0, "%03u%%\r", (u32_i + 1) * 100 / EMMC_PWRCUT_TEST_UNIT_CNT);
	}

	emmc_debug(EMMC_DEBUG_LEVEL, 0, "   ok\n");

	//---------------------------------------
	while (1) {
		u32_T0 = emmc_hw_timer_tick();
		ctrl_blk_t.u8_target_unit = u32_T0 % EMMC_PWRCUT_TEST_UNIT_CNT;
		ctrl_blk_t.au8_target_data[ctrl_blk_t.u8_target_unit] ^= 0xFF;

		ctrl_blk_t.u32_check_sum = emmc_chk_sum(&ctrl_blk_t.au8_target_data[0], sizeof(ctrl_blk_t.au8_target_data));
		ctrl_blk_t.u32_check_sum += ctrl_blk_t.u8_target_unit;

		u32_err = emmc_write_data((U8 *)&ctrl_blk_t, EMMC_SECTOR_512BYTE, u32_blk_start_addr - 1);
		if (u32_err != EMMC_ST_SUCCESS) {
			emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
				   "eMMC Err: write CtrlBlk, Err:%x\n", u32_err);
			return u32_err;
		}

		u32_blk_addr = u32_blk_start_addr + ctrl_blk_t.u8_target_unit *
			(EMMC_PWRCUT_TEST_UNIT_BYTECNT >> 9);
		memset(u8_data_buf, ctrl_blk_t.au8_target_data[ctrl_blk_t.u8_target_unit], EMMC_PWRCUT_TEST_UNIT_BYTECNT);

		emmc_debug(EMMC_DEBUG_LEVEL, 0, "writing... %02Xh %08Xh %02Xh\n",
			   ctrl_blk_t.u8_target_unit, u32_blk_addr, ctrl_blk_t.au8_target_data[ctrl_blk_t.u8_target_unit]);

		u32_err = emmc_write_data(u8_data_buf, EMMC_PWRCUT_TEST_UNIT_BYTECNT, u32_blk_addr);
		if (u32_err != EMMC_ST_SUCCESS) {
			emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: write Fail, Err:%x\n", u32_err);
			return u32_err;
		}
	}

	return EMMC_ST_SUCCESS;
}

U32 emmc_test_pwr_cut_test2(U8 *u8_data_buf, U32 u32_blk_start_addr)
{
	U32 u32_blk_cnt, u32_i, u32_test_loop;
	U32 u32_err = EMMC_ST_SUCCESS;
	U8 data;

	struct mmc *mmc = find_mmc_device(0);

	if (!mmc) {
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: no mmc device at slot\n");
		return 1;
	}

	if (!mmc->has_init) {
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: Need mmc init first!\n");
		return 1;
	}

	emmc_debug(EMMC_DEBUG_LEVEL, 1, "eMMC Info: Reliable write ext_csd[167] 0x%02x, ext_csd[166] 0x%02x\n",
		   mmc->ext_csd[167], mmc->ext_csd[166]);

	u32_blk_cnt = (48 * 1024 * 1024) >> EMMC_SECTOR_512BYTE_BITS;

	memset(u8_data_buf, 0, (48 * 1024 * 1024));

	u32_err = emmc_read_data(u8_data_buf, u32_blk_cnt << EMMC_SECTOR_512BYTE_BITS, u32_blk_start_addr);
	if (u32_err != EMMC_ST_SUCCESS) {
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: Reliable read Fail,Err:%x\n", u32_err);
		return 1;
	}

	data = u8_data_buf[0];
	for (u32_i = 0; u32_i < (48 * 1024 * 1024); u32_i++) {
		if (u8_data_buf[u32_i] != data) {
			emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: Data Mismatch: Blk:%X  Byte:%X  buf:%X data:%x\n",
				   u32_i >> 9, u32_i & 0x1FF, u8_data_buf[u32_i], data);
			return EMMC_ST_ERR_DATA_MISMATCH;
		}
	}
	emmc_debug(EMMC_DEBUG_LEVEL, 1, "Reliable read compare success\n");

	data = (U8)emmc_hw_timer_tick();
	memset(u8_data_buf, data, (48 * 1024 * 1024));
	u32_test_loop = 0;

	u32_err = emmc_write_data(u8_data_buf, (48 * 1024 * 1024), u32_blk_start_addr);
	if (u32_err != EMMC_ST_SUCCESS) {
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: Reliable write Fail,Err:%x\n", u32_err);
		return u32_err;
	}
	emmc_debug(EMMC_DEBUG_LEVEL, 1, "Reliable write Success\n");
	emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "power cut test loop star\n");

	u32_blk_start_addr += ((48 * 1024 * 1024) >> EMMC_SECTOR_512BYTE_BITS);
	u32_blk_cnt = (4 * 1024 * 1024) >> EMMC_SECTOR_512BYTE_BITS;
	while (1) {
		for (u32_i = 0; u32_i < u32_blk_cnt; u32_i++) {
			u32_err = emmc_write_data(u8_data_buf + (u32_i << 9), 512, u32_blk_start_addr + u32_i);
			if (u32_err != EMMC_ST_SUCCESS) {
				emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: Reliable write Fail,Err:%x\n", u32_err);
				return u32_err;
			}
		}
		u32_test_loop++;
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "Test_loop:%X\n", u32_test_loop);
	}

	return EMMC_ST_SUCCESS;
}

#if defined(DMA_MBOOT_ADDR)
#include "../disk/part_emmc.h"

U32 emmc_test_read_disturbance(void)
{
	emmc_partition_t mpart;
	U32 u32_err = 0;
	U32 u32_read_allcnt = 0;
	U32 u32_i;
	U32 *pu32_r = (U32 *)DMA_R_ADDR;
	U32 *u32_mboot_addr = (U32 *)DMA_MBOOT_ADDR;
	U32 u32_mboot_start = 0, u32_mboot_size = 0;

	for (u32_i = 1; u32_i <= EMMC_RESERVED_FOR_MAP_V2; u32_i++) {
		if (emmc_read_data((U8 *)&mpart, EMMC_SECTOR_512BYTE, u32_i) == EMMC_ST_SUCCESS) {
			if (strcmp("MBOOT", (const char *)mpart.name) == 0) {
				u32_mboot_start = mpart.start_block;
				u32_mboot_size = mpart.block_count << 9;
				u32_err = emmc_read_data((U8 *)u32_mboot_addr, u32_mboot_size, u32_mboot_start);
				if (u32_err != EMMC_ST_SUCCESS) {
					emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: read Fail,Err:%x\n", u32_err);
					return u32_err;
				}
				printf("** Find MBOOT **\n");
			}
		} else {
			emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: read Fail,Err:%x\n", u32_err);
			return u32_err;
		}
		if (u32_mboot_start != 0 && u32_mboot_size != 0)
			break;
	}
	if (u32_mboot_start == 0 && u32_mboot_size == 0) {
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "No mboot partition\n");
		return EMMC_ST_SUCCESS;
	}

	while (1) {
		u32_err = emmc_read_data((U8 *)pu32_r, u32_mboot_size, u32_mboot_start);
		if (u32_err != EMMC_ST_SUCCESS) {
			emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: read Fail,Err:%x\n", u32_err);
			return u32_err;
		}

		u32_err = emmc_compare_data((U8 *)pu32_r, (U8 *)u32_mboot_addr, u32_mboot_size);
		if (u32_err != EMMC_ST_SUCCESS) {
			emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: mboot mismatch,Err:%x\n", u32_err);
			return u32_err;
		}
		u32_read_allcnt++;
		if ((u32_read_allcnt % 2000) == 0)
			emmc_debug(0, 0, "eMMC read cnt:%8Xh\n", u32_read_allcnt);
	}

	return EMMC_ST_SUCCESS;
}
#endif
#endif
#endif
