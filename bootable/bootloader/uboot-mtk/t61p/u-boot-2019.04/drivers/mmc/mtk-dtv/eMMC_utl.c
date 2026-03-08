// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (C) 2023 MediaTek Inc.
 */


#include "eMMC.h"
#if defined(UNIFIED_eMMC_DRIVER) && UNIFIED_eMMC_DRIVER

struct emmc_test_align_pack {
	U8 u8_0;
	U16 u16_0;
	U32 u32_0, u32_1;

} EMMC_PACK1;

//--------------------------------------------------------------------------

static struct emmc_test_align_pack g_emmc_test_align_pack_t EMMC_ALIGN1;

U32 emmc_check_align_pack(U8 u8_align_byte_cnt)
{
	if ((unsigned long)&g_emmc_test_align_pack_t.u8_0 & (u8_align_byte_cnt - 1)) {
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: not aliged. expect %Xh but %lXh\n",
			   u8_align_byte_cnt, (unsigned long)&g_emmc_test_align_pack_t.u8_0);
		emmc_die("\n");
		return EMMC_ST_ERR_NOT_ALIGN;
	}

	if ((unsigned long)&g_emmc_test_align_pack_t.u16_0 -
	    (unsigned long)&g_emmc_test_align_pack_t.u8_0 != 1 ||
	    (unsigned long)&g_emmc_test_align_pack_t.u32_0 -
	    (unsigned long)&g_emmc_test_align_pack_t.u8_0 != 3 ||
	    (unsigned long)&g_emmc_test_align_pack_t.u32_1 -
	    (unsigned long)&g_emmc_test_align_pack_t.u8_0 != 7) {
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: not packed. check err.0\n");
		emmc_die("\n");
		return EMMC_ST_ERR_NOT_PACKED;
	}

	if ((unsigned long)&g_emmc_test_align_pack_t.u16_0 -
	    (unsigned long)&g_emmc_test_align_pack_t.u8_0 != 1 ||
	    (unsigned long)&g_emmc_test_align_pack_t.u32_0 -
	    (unsigned long)&g_emmc_test_align_pack_t.u16_0 != 2 ||
	    (unsigned long)&g_emmc_test_align_pack_t.u32_1 -
	    (unsigned long)&g_emmc_test_align_pack_t.u32_0 != 4) {
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: not packed. check err.1\n");
		emmc_die("\n");
		return EMMC_ST_ERR_NOT_PACKED;
	}

	return EMMC_ST_SUCCESS;
}


static __inline void dump_mem_line(unsigned char *buf, U32 cnt)
{
	U32 i;

	emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 0, " 0x%08lX: ", (unsigned long)buf);
	for (i =  0; i < cnt; i++)
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 0, "%02Xh ", buf[i]);

	emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 0, " | ");

	for (i = 0; i < cnt; i++)
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 0, "%c", (buf[i] >= 32 && buf[i] < 128) ? buf[i] : '.');

		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 0, "\n");
	}

void emmc_dump_mem(unsigned char *buf, U32 cnt)
{
	U32 i;

	for (i = 0; i < cnt; i += 16)
		dump_mem_line(buf + i, 16);
}

static __inline void dump_mem_line_32(U32 *buf, int cnt)
{
	U32 i;

	emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 0, " 0x%08lX: ", (unsigned long)buf);
	for (i = 0; i < cnt; i++)
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 0, "%08Xh ", buf[i]);
}

void emmc_dump_mem_32(U32 *buf, U32 cnt)
{
	U32 i;

	for (i = 0; i < cnt; i += 8) {
		dump_mem_line_32(buf + i, 8);

		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 0, " | 0x%08X\n", i + 7);
	}
}

U32 emmc_compare_data(U8 *pu8_buf0, U8 *pu8_buf1, U32 u32_byte_cnt)
{
	U32 u32_i, u32_offset;

	for (u32_i = 0; u32_i < u32_byte_cnt; u32_i++) {
		if (pu8_buf0[u32_i] != pu8_buf1[u32_i]) {
			emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
				   "eMMC Err: data mismatch: i:[%Xh], Buf0: %02Xh, Buf1: %02Xh\n",
				   u32_i, pu8_buf0[u32_i], pu8_buf1[u32_i]);
			break;
		}
	}

	if (u32_i != u32_byte_cnt && (emmc_drv.u32_drvflag & DRV_FLAG_TUNING_TTABLE) == 0) {
		u32_offset = (u32_i >> EMMC_SECTOR_512BYTE_BITS) << 9;

		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 0, "\ndump Buf0, from %Xh-th bytes\n", u32_offset);
		emmc_dump_mem(pu8_buf0 + u32_offset, EMMC_SECTOR_512BYTE);
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 0, "dump Buf1, from %Xh-th bytes\n", u32_offset);
		emmc_dump_mem(pu8_buf1 + u32_offset, EMMC_SECTOR_512BYTE);
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 0, "\n");
		return EMMC_ST_ERR_DATA_MISMATCH;
	}

	if (u32_i != u32_byte_cnt)
		return EMMC_ST_ERR_DATA_MISMATCH;

	return EMMC_ST_SUCCESS;
}

U32 emmc_chk_sum(U8 *pu8_data, U32 u32_byte_cnt)
{
	U32 u32_sum = 0, u32_tmp;

	for (u32_tmp = 0; u32_tmp < u32_byte_cnt; u32_tmp++)
		u32_sum += pu8_data[u32_tmp];

	return u32_sum;
}

/* return 0: same, 1: different */
U32 emmc_compare_cis_tag(U8 *tag)
{
	const char *str = "MSTARSEMIUNFDCIS";
	int i = 0;

	for (i = 0; i < 16; i++) {
		if (tag[i] != str[i])
			return 1;
	}

	return 0;
}

U32 emmc_print_device_info(void)
{
	U32 u32_err;
	U8 au8_buf[EMMC_SECTOR_512BYTE];


	u32_err = emmc_check_if_ready();
	if (u32_err != EMMC_ST_SUCCESS) {
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: emmc_Init_ex fail: %Xh\n", u32_err);
		return u32_err;
	}

	//------------------------------
	u32_err = emmc_cmd8(au8_buf);
	if (u32_err != EMMC_ST_SUCCESS)
		return u32_err;

	emmc_debug(EMMC_DEBUG_LEVEL, 0, "\nExtCSD:\n");
	emmc_debug(EMMC_DEBUG_LEVEL, 0, "-------------------------------\n");
	//emmc_dump_mem(au8_buf, EMMC_SECTOR_512BYTE);

	switch (emmc_drv.u8_ecsd192_ver) {
	case 7:
		emmc_debug(EMMC_DEBUG_LEVEL, 0, "eMMC V.5.0\n");
	break;
	case 6:
		emmc_debug(EMMC_DEBUG_LEVEL, 0, "eMMC V.4.5\n");
	break;
	case 5:
		emmc_debug(EMMC_DEBUG_LEVEL, 0, "eMMC V.4.41\n");
	break;
	default:
		emmc_debug(EMMC_DEBUG_LEVEL, 0, "eMMC V.x, ECSD[192]:%u\n", emmc_drv.u8_ecsd192_ver);
	}
	emmc_debug(EMMC_DEBUG_LEVEL, 0, "Support: %s %s %s %s %s %s\n",
		   (emmc_drv.u8_ecsd196_dev_type & BIT7) ? "HS400 1.2V" : "",
		   (emmc_drv.u8_ecsd196_dev_type & BIT6) ? "HS400 1.8V" : "",
		   (emmc_drv.u8_ecsd196_dev_type & BIT5) ? "HS200 1.2V" : "",
		   (emmc_drv.u8_ecsd196_dev_type & BIT4) ? "HS200 1.8V" : "",
		   (emmc_drv.u8_ecsd196_dev_type & BIT3) ? "DDR52 1.2V" : "",
		   (emmc_drv.u8_ecsd196_dev_type & BIT2) ? "DDR52 1.8/3V" : "");

	emmc_debug(EMMC_DEBUG_LEVEL, 0, "SEC_COUNT: %Xh\n", emmc_drv.u32_sec_count);
	emmc_debug(EMMC_DEBUG_LEVEL, 0, "Capacity: %u MB\n", emmc_drv.u32_sec_count >> 11);
	emmc_debug(EMMC_DEBUG_LEVEL, 0, "eMMC BUS_WIDTH: %Xh\n", emmc_drv.u8_bus_width);

	emmc_debug(EMMC_DEBUG_LEVEL, 0, "TRIM: %Xh\n", (emmc_drv.u32_emmc_flag & EMMC_FLAG_TRIM) ? 1 : 0);

	if (emmc_drv.u32_emmc_flag & (EMMC_FLAG_HPI_CMD12 | EMMC_FLAG_HPI_CMD13))
		emmc_debug(EMMC_DEBUG_LEVEL, 0, "HPI: CMD%u\n", (emmc_drv.u32_emmc_flag & EMMC_FLAG_HPI_CMD12) ? 12 : 13);
	else
		emmc_debug(EMMC_DEBUG_LEVEL, 0, "HPI: 0\n");

	emmc_debug(EMMC_DEBUG_LEVEL, 0, "Reliable Write BlkCnt: %Xh\n", emmc_drv.u16_reliable_w_blk_cnt);
	emmc_debug(EMMC_DEBUG_LEVEL, 0, "Power Off Notification: ECSD[34]:%Xh, Short: %u ms, Long: %u ms\n",
		   emmc_drv.u8_ecsd34_pwr_off_ctrl,
		   emmc_drv.u8_ecsd248_cmd6_to * 10,
		   emmc_drv.u8_ecsd247_pwr_off_long_to * 10);

	emmc_debug(EMMC_DEBUG_LEVEL, 0, "-------------------------------\n\n");
	//------------------------------

	emmc_debug(EMMC_DEBUG_LEVEL, 0, "CSD:\n");
	emmc_debug(EMMC_DEBUG_LEVEL, 0, "-------------------------------\n");
	emmc_dump_mem(&emmc_drv.au8_csd[0], EMMC_MAX_RSP_BYTE_CNT);
	emmc_debug(EMMC_DEBUG_LEVEL, 0, "SPEC_VERS: %Xh, R_BL_LEN: %Xh, W_BL_LEN: %Xh\n",
		   emmc_drv.u8_spec_vers, emmc_drv.u8_r_bl_len, emmc_drv.u8_w_bl_len);
	emmc_debug(EMMC_DEBUG_LEVEL, 0, "Access Mode: %s\n", emmc_drv.u8_if_sector_mode ? "Sector" : "Byte");
	emmc_debug(EMMC_DEBUG_LEVEL, 0, "C_SIZE: %Xh\n", emmc_drv.u16_c_size);
	emmc_debug(EMMC_DEBUG_LEVEL, 0, "ERASE_GRP_SIZE: %Xh, ERASE_GRP_MULT: %Xh\n",
		   emmc_drv.u8_erase_grp_size, emmc_drv.u8_erase_grp_mult);
	emmc_debug(EMMC_DEBUG_LEVEL, 0, "EraseUnitSize: %u\n", emmc_drv.u32_erase_unit_size);

	switch (emmc_drv.u8_tran_speed) {
	case 0x2A:
		emmc_debug(EMMC_DEBUG_LEVEL, 0, "default speed 20MHz\n");
	break;
	case 0x32:
		emmc_debug(EMMC_DEBUG_LEVEL, 0, "default speed 26MHz\n");
	break;
	default:
		emmc_debug(EMMC_DEBUG_LEVEL, 0, "normal TRAN_SPEED: %Xh\n", emmc_drv.u8_tran_speed);
	}
	emmc_debug(EMMC_DEBUG_LEVEL, 0, "TAAC: %Xh, NSAC: %Xh, R2W_FACTOR: %Xh\n",
		   emmc_drv.u8_taac, emmc_drv.u8_nsac, emmc_drv.u8_r2w_factor);
	emmc_debug(EMMC_DEBUG_LEVEL, 0, "-------------------------------\n\n");

	emmc_debug(EMMC_DEBUG_LEVEL, 0, "RCA: %Xh\n", emmc_drv.u16_rca);
	emmc_debug(EMMC_DEBUG_LEVEL, 0, "CID:\n");
	emmc_dump_mem(&emmc_drv.au8_cid[1], EMMC_MAX_RSP_BYTE_CNT - 1);
	emmc_debug(EMMC_DEBUG_LEVEL, 0, "-------------------------------\n\n");

	return u32_err;
}

#endif
