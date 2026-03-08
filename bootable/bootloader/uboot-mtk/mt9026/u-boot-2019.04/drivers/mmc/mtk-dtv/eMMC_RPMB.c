// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (C) 2023 MediaTek Inc.
 */

#include "eMMC.h"
#ifdef CONFIG_MULTICORES_PLATFORM
#include <smp/thread_info.h>
extern smp_spin_lock_t emmc_spin_lock;
#endif



struct EMMC_RPMB_DATA GLOBAL_EMMCRPMB;

void emmc_rpmb_swap_memcpy(U8 *u8_dest, U8 *u8_src, U32 u32_cnt)
{
	U32 u32_i;
	for (u32_i = 0; u32_i < u32_cnt; u32_i++)
		u8_dest[u32_cnt - u32_i - 1] = u8_src[u32_i];
}

void emmc_rpmb_copy_result(U8 *u8_dest, U8 *u8_src, U32 u32_cnt)
{
	U32 u32_i;
	for (u32_i = 0; u32_i < u32_cnt; u32_i++)
		u8_dest[u32_i] = u8_src[u32_cnt - u32_i - 1];
}

U32 emmc_cmd23_rpmb(U16 u16_blk_cnt, U8 u8_is_reliable)
{
	U32 u32_err, u32_arg;
	U16 u16_ctrl, u16_reg;
	U8 u8_retry_fcie = 0, u8_retry_cmd = 0;

	u32_arg = u16_blk_cnt & 0xFFFF;// don't set BIT24
	if (u8_is_reliable)
		u32_arg |= BIT31; // don't set BIT24
	u16_ctrl = BIT_SD_CMD_EN | BIT_SD_RSP_EN;

	EMMC_DISABLE_RETRY(1);
	emmc_skew4_rsp_reg();

LABEL_SEND_CMD:
	emmc_fcie_clear_events();
	u32_err = emmc_fcie_send_cmd(emmc_drv.u16_mmc_mode,
				     u16_ctrl, u32_arg, 23, EMMC_R1_BYTE_CNT);
	if (u32_err != EMMC_ST_SUCCESS) {
		if (u8_retry_cmd < EMMC_FCIE_CMD_RSP_ERR_RETRY_CNT && 0 ==
		    EMMC_IF_DISABLE_RETRY()) {
			u8_retry_cmd++;
			emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
				   "eMMC WARN: CMD23 retry:%u, %Xh\n",
				   u8_retry_cmd, u32_err);
				emmc_fcie_err_handler_retry();
				goto LABEL_SEND_CMD;
		}
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: CMD23 retry:%u, %Xh\n",
			   u8_retry_cmd, u32_err);
		emmc_fcie_err_handler_stop();
	} else {
		REG_FCIE_R(FCIE_SD_STATUS, u16_reg);

		if (u16_reg & (BIT_SD_RSP_TIMEOUT | BIT_SD_RSP_CRC_ERR)) {
			if (u8_retry_fcie < EMMC_CMD_API_ERR_RETRY_CNT && 0 ==
			    EMMC_IF_DISABLE_RETRY()) {
				u8_retry_fcie++;
				emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
					   "eMMC WARN: CMD23 SD_STS: %04Xh, Retry: %u\n",
					   u16_reg, u8_retry_fcie);
				emmc_fcie_err_handler_retry();
				goto LABEL_SEND_CMD;
			}

			u32_err = EMMC_ST_ERR_CMD23;
			emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
				   "eMMC Err: CMD23 SD_STS: %04Xh, Retry: %u\n",
				   u16_reg, u8_retry_fcie);
			emmc_fcie_err_handler_stop();
		} else {
			u32_err = emmc_check_r1_error();
			if (u32_err != EMMC_ST_SUCCESS) {
				emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
					   "eMMC Err: CMD23 check R1 error: %Xh\n", u32_err);
				emmc_fcie_err_handler_stop();
			}
		}
	}

	EMMC_DISABLE_RETRY(0);
	return u32_err;
}

U32 emmc_cmd18_rpmb_miu(U32 u32_emmc_blk_addr, U8 *pu8_data_buf, U16 u16_blk_cnt)
{
	U32 u32_err, u32_arg;
	U16 u16_mode, u16_ctrl, u16_reg;
	U8  u8_retry_fcie = 0, u8_retry_cmd = 0;
	unsigned long dma_addr;
	U16 u16_status_err_flag = BIT_SD_FCIE_ERR_FLAGS;

	// -------------------------------
	if (EMMC_IF_DISABLE_RETRY() == 0)
		emmc_fcie_err_handler_restore_clk();

	EMMC_DISABLE_RETRY(1);

LABEL_SEND_CMD:
	u32_err = emmc_cmd23_rpmb(u16_blk_cnt, 0);
	if (u32_err != EMMC_ST_SUCCESS)
		return u32_err;

	emmc_skew4_read_data_reg(&u16_status_err_flag);
	// -------------------------------
	// send cmd
	u16_ctrl = BIT_SD_CMD_EN | BIT_SD_RSP_EN | BIT_SD_DTRX_EN;
	u32_arg =  u32_emmc_blk_addr << (emmc_drv.u8_if_sector_mode ? 0 : EMMC_SECTOR_512BYTE_BITS);
	u16_mode = emmc_drv.u16_mmc_mode | emmc_drv.u8_bus_width;
	emmc_fcie_clear_events();
	REG_FCIE_W(FCIE_JOB_BL_CNT, u16_blk_cnt);
	REG_FCIE_W(FCIE_BLK_SIZE, 0x200);//64 bytes tuning pattern

	dma_addr = emmc_translate_dma_address_ex((unsigned long)pu8_data_buf,
						 EMMC_SECTOR_512BYTE * u16_blk_cnt);

	REG_FCIE_W(FCIE_MIU_DMA_ADDR_15_0, dma_addr & 0xFFFF);
	REG_FCIE_W(FCIE_MIU_DMA_ADDR_31_16, dma_addr >> 16);
	if (sizeof(unsigned long) == 8)
		REG_FCIE_W(FCIE_MIU_DMA_ADDR_47_32, dma_addr >> 32);

	REG_FCIE_W(FCIE_MIU_DMA_LEN_15_0, (EMMC_SECTOR_512BYTE * u16_blk_cnt) & 0xFFFF);
	REG_FCIE_W(FCIE_MIU_DMA_LEN_31_16, (EMMC_SECTOR_512BYTE * u16_blk_cnt) >> 16);

	u32_err = emmc_fcie_send_cmd(u16_mode, u16_ctrl, u32_arg, 18, EMMC_R1_BYTE_CNT);
	if (u32_err != EMMC_ST_SUCCESS) {
		if (u8_retry_cmd < EMMC_FCIE_CMD_RSP_ERR_RETRY_CNT && 0 ==
		    EMMC_IF_DISABLE_RETRY()) {
			u8_retry_cmd++;
			emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
				   "eMMC WARN: CMD18 retry:%u, %Xh\n", u8_retry_cmd, u32_err);
			emmc_fcie_err_handler_reinit();
			emmc_fcie_err_handler_retry();
			goto LABEL_SEND_CMD;
		}
		u32_err = EMMC_ST_ERR_CMD18_RSP;
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: CMD18 retry:%u, %Xh\n",
			   u8_retry_cmd, u32_err);
		emmc_fcie_err_handler_stop();
		goto LABEL_END;
	}

	// -------------------------------
	// check FCIE
	u32_err = emmc_fcie_wait_events(FCIE_MIE_EVENT, BIT_DMA_END, TIME_WAIT_n_BLK_END *
					(1 + (u16_blk_cnt >> 11)));

	REG_FCIE_R(FCIE_SD_STATUS, u16_reg);
	if (u32_err != EMMC_ST_SUCCESS || (u16_reg & BIT_SD_FCIE_ERR_FLAGS)) {
		if (u8_retry_fcie < EMMC_CMD_API_ERR_RETRY_CNT && 0 == EMMC_IF_DISABLE_RETRY()) {
			u8_retry_fcie++;
			emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
				   "eMMC WARN: CMD18 Reg.12: %04Xh, Err: %Xh, Retry: %u\n",
				   u16_reg, u32_err, u8_retry_fcie);

			emmc_fcie_err_handler_reinit();
			emmc_fcie_err_handler_retry();

			goto LABEL_SEND_CMD;
		}
		u32_err = EMMC_ST_ERR_CMD18;
		if (u16_reg & BIT_SD_RSP_CRC_ERR)
			u32_err = EMMC_ST_ERR_CMD18_RSP;
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
			   "eMMC Err: CMD18 Reg.12: %04Xh, Err: %Xh, Retry: %u\n",
			   u16_reg, u32_err, u8_retry_fcie);
		emmc_fcie_err_handler_stop();
		goto LABEL_END;
	}

	// -------------------------------
	// check device
	u32_err = emmc_check_r1_error();
	if (u32_err != EMMC_ST_SUCCESS) {
		u32_err = EMMC_ST_ERR_CMD18;
		emmc_debug(1, 1, "eMMC Err: CMD18 check R1 error: %Xh, Arg: %Xh\n",
			   u32_err, u32_arg);
		emmc_fcie_err_handler_stop();
		goto LABEL_END;
	}

LABEL_END:

	EMMC_DISABLE_RETRY(0);

	if (u32_err == EMMC_ST_SUCCESS)
		u32_err = emmc_cmd13(emmc_drv.u16_rca);

	return u32_err;
}

U32 emmc_cmd25_rpmb_miu(U32 u32_emmc_blk_addr, U8 *pu8_data_buf, U16 u16_blk_cnt, U8 u8_is_reliable)
{
	U32 u32_err, u32_arg;
	U16 u16_mode, u16_ctrl, u16_reg;
	U8 u8_retry_fcie = 0, u8_retry_cmd = 0;
	unsigned long dma_addr;

    /*
    // -------------------------------
    // restore clock to max
	if (0 == EMMC_IF_DISABLE_RETRY())
		emmc_fcie_err_handler_restore_clk();
    */
	EMMC_DISABLE_RETRY(1);

LABEL_SEND_CMD:
	u32_err = emmc_cmd23_rpmb(u16_blk_cnt, u8_is_reliable);
	if (u32_err != EMMC_ST_SUCCESS)
		return u32_err;

    // -------------------------------
    // send cmd
	u16_ctrl = BIT_SD_CMD_EN | BIT_SD_RSP_EN;
	u16_mode = emmc_drv.u16_mmc_mode | emmc_drv.u8_bus_width;
	u32_arg =  u32_emmc_blk_addr << (emmc_drv.u8_if_sector_mode ? 0 : EMMC_SECTOR_512BYTE_BITS);
	emmc_fcie_clear_events();
	REG_FCIE_W(FCIE_JOB_BL_CNT, u16_blk_cnt);

	dma_addr = emmc_translate_dma_address_ex((unsigned long)pu8_data_buf,
						 EMMC_SECTOR_512BYTE * u16_blk_cnt);
	REG_FCIE_W(FCIE_MIU_DMA_ADDR_15_0, dma_addr & 0xFFFF);
	REG_FCIE_W(FCIE_MIU_DMA_ADDR_31_16, dma_addr >> 16);
	if (sizeof(unsigned long) == 8)
		REG_FCIE_W(FCIE_MIU_DMA_ADDR_47_32, dma_addr >> 32);

	REG_FCIE_W(FCIE_MIU_DMA_LEN_15_0, (EMMC_SECTOR_512BYTE * u16_blk_cnt) & 0xFFFF);
	REG_FCIE_W(FCIE_MIU_DMA_LEN_31_16, (EMMC_SECTOR_512BYTE * u16_blk_cnt) >> 16);

	u32_err = emmc_fcie_send_cmd(u16_mode, u16_ctrl, u32_arg, 25, EMMC_R1_BYTE_CNT);
	if (u32_err != EMMC_ST_SUCCESS) {
		if (u8_retry_cmd < EMMC_FCIE_CMD_RSP_ERR_RETRY_CNT && 0 ==
		    EMMC_IF_DISABLE_RETRY()) {
			u8_retry_cmd++;
			emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
				   "eMMC WARN: CMD25 retry:%u, %Xh, Arg: %Xh\n",
				   u8_retry_cmd, u32_err, u32_arg);
			emmc_fcie_err_handler_reinit();
			emmc_fcie_err_handler_retry();
			goto LABEL_SEND_CMD;
		}
		u32_err = EMMC_ST_ERR_CMD25_RSP;
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
			   "eMMC Err: CMD25 retry:%u, %Xh, Arg: %Xh\n",
			   u8_retry_cmd, u32_err, u32_arg);
		emmc_fcie_err_handler_stop();
		goto LABEL_END;
	}

    // -------------------------------
    // check device
	REG_FCIE_R(FCIE_SD_STATUS, u16_reg);
	if (u16_reg & (BIT_SD_RSP_TIMEOUT | BIT_SD_RSP_CRC_ERR)) {
		if (u8_retry_fcie < EMMC_CMD_API_ERR_RETRY_CNT && 0 == EMMC_IF_DISABLE_RETRY()) {
			u8_retry_fcie++;
			emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
				   "eMMC WARN: CMD25 Reg.12: %04Xh, Err: %Xh, Retry: %u, Arg: %Xh\n",
				   u16_reg, u32_err, u8_retry_fcie, u32_arg);
			emmc_fcie_err_handler_reinit();
			emmc_fcie_err_handler_retry();
			goto LABEL_SEND_CMD;
	}
	u32_err = EMMC_ST_ERR_CMD25;
	if (u16_reg & BIT_SD_RSP_CRC_ERR)
		u32_err = EMMC_ST_ERR_CMD25_RSP;
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
			   "eMMC Err: CMD25 Reg.12: %04Xh, Err: %Xh, Retry: %u, Arg: %Xh\n",
			   u16_reg, u32_err, u8_retry_fcie, u32_arg);
		emmc_fcie_err_handler_stop();
		goto LABEL_END;
	}

	u32_err = emmc_check_r1_error();
	if (u32_err != EMMC_ST_SUCCESS) {
		u32_err = EMMC_ST_ERR_CMD25_CHK_R1;
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
			   "eMMC Err: CMD25 check R1 error: %Xh, Arg: %Xh\n", u32_err, u32_arg);
		emmc_fcie_err_handler_stop();
		goto LABEL_END;
	}

    // -------------------------------
    // send data
	u32_err = emmc_fcie_wait_d0_high(TIME_WAIT_DAT0_HIGH);
	if (u32_err != EMMC_ST_SUCCESS) {
		u32_err = EMMC_ST_ERR_CMD25_WAIT_D0H;
		goto LABEL_END;
	}

	emmc_check_power_cut();
	emmc_skew4_write_data_reg();
	REG_FCIE_W(FCIE_SD_CTRL, BIT_SD_DTRX_EN | BIT_SD_DAT_DIR_W);
	REG_FCIE_SETBIT(FCIE_SD_CTRL, BIT_JOB_START);

	u32_err = emmc_fcie_wait_events(FCIE_MIE_EVENT,
					BIT_DMA_END, TIME_WAIT_n_BLK_END * (1 + (u16_blk_cnt >> 9)));

	REG_FCIE_R(FCIE_SD_STATUS, u16_reg);
	if (u32_err != EMMC_ST_SUCCESS || (u16_reg & (BIT_SD_W_FAIL | BIT_SD_W_CRC_ERR))) {
		if (u8_retry_fcie < EMMC_CMD_API_ERR_RETRY_CNT && 0 == EMMC_IF_DISABLE_RETRY()) {
			u8_retry_fcie++;
			emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
				   "eMMC WARN: CMD25 Reg.12: %04Xh, Err: %Xh, Retry: %u, Arg: %Xh\n",
				   u16_reg, u32_err, u8_retry_fcie, u32_arg);
			emmc_fcie_err_handler_reinit();
			emmc_fcie_err_handler_retry();
			goto LABEL_SEND_CMD;
		}
		u32_err = EMMC_ST_ERR_CMD25;
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
			   "eMMC Err: CMD25 Reg.12: %04Xh, Err: %Xh, Retry: %u, Arg: %Xh\n",
			   u16_reg, u32_err, u8_retry_fcie, u32_arg);
		emmc_fcie_err_handler_stop();
		goto LABEL_END;
	}

LABEL_END:
	EMMC_DISABLE_RETRY(0);
	if (u32_err == EMMC_ST_SUCCESS)
		u32_err = emmc_cmd13(emmc_drv.u16_rca);


	return u32_err;
}

U32 emmc_rpmb_chk_result_err(void)
{
	if (GLOBAL_EMMCRPMB.u16_result & 0x7F) {
		if (GLOBAL_EMMCRPMB.u16_result != 0x0007) {
			emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
				   "eMMC Result Error: %Xh\n", GLOBAL_EMMCRPMB.u16_result);
			return ((U32)GLOBAL_EMMCRPMB.u16_result);
		}
	}

	return EMMC_ST_SUCCESS;
}

#if 0
U32 emmc_rpmb_read_result(U16 *pu16_result)
{
	U32 u32_err;
	U8 partconfig, origpartconfig;

	partconfig = emmc_drv.u8_partition_config;
	origpartconfig = emmc_drv.u8_partition_config;
	partconfig &= ~(BIT0 | BIT1 | BIT2);
	partconfig |= BIT0 | BIT1;

	u32_err = emmc_modify_ext_csd(MMC_SWITCH_MODE_WRITE_BYTE, 179, partconfig);
	if (u32_err != EMMC_ST_SUCCESS)
		return u32_err;

	memset((void *)(&GLOBAL_EMMCRPMB), 0, sizeof(struct EMMC_RPMB_DATA));
	GLOBAL_EMMCRPMB.u16_req_rsp = EMMC_RPMB_REQ_RESULT_R;

	u32_err = emmc_rpmb_no_data_req();
	if (u32_err == EMMC_ST_SUCCESS)
		*pu16_result = GLOBAL_EMMCRPMB.u16_result;

	return emmc_modify_ext_csd(MMC_SWITCH_MODE_WRITE_BYTE, 179, origpartconfig);
}
#endif
U32 emmc_rpmb_no_data_req(void)
{
	U32 u32_err;
	U8 retry_cnt = 0;

LABEL_SEND_CMD:

	emmc_rpmb_swap_memcpy(GLOBAL_EMMC_SECTORBUF,
			      (U8 *)(&GLOBAL_EMMCRPMB), sizeof(struct EMMC_RPMB_DATA));

	u32_err = emmc_cmd25_rpmb_miu(0, GLOBAL_EMMC_SECTORBUF, 1, 0);
	if (u32_err != EMMC_ST_SUCCESS) {
		if (emmc_drv.u8_emmc_pll_skew4 &&
		    (emmc_drv.pad_type == FCIE_EMMC_HS400 ||
		    emmc_drv.pad_type == FCIE_EMMC_HS400_5_1)) {
			if (retry_cnt < EMMC_MAX_RETRY_CNT) {
				emmc_m6e3_rescan_timing();
				retry_cnt++;
				goto LABEL_SEND_CMD;
			}
		}
		return u32_err;
	}
	u32_err = emmc_cmd18_rpmb_miu(0, GLOBAL_EMMC_SECTORBUF, 1);
	if (u32_err != EMMC_ST_SUCCESS) {
		if (emmc_drv.u8_emmc_pll_skew4 &&
		    (emmc_drv.pad_type == FCIE_EMMC_HS400 ||
		    emmc_drv.pad_type == FCIE_EMMC_HS400_5_1)) {
			if (retry_cnt < EMMC_MAX_RETRY_CNT) {
			    emmc_m6e3_rescan_timing();
				retry_cnt++;
				goto LABEL_SEND_CMD;
			}
		}
		return u32_err;
	}

	emmc_rpmb_copy_result((U8 *)(&GLOBAL_EMMCRPMB), GLOBAL_EMMC_SECTORBUF,
			      sizeof(struct EMMC_RPMB_DATA));

	u32_err = emmc_rpmb_chk_result_err();

	return u32_err;
}

U32 emmc_rpmb_read_blk(U8 *pu8_data_buf, U8 *pu8_nonce, U16 u16_blk_addr)
{
	U32 u32_err = 0;
	U8 partconfig, origpartconfig;

	partconfig = emmc_drv.u8_partition_config;
	origpartconfig = emmc_drv.u8_partition_config;
	partconfig &= ~(BIT0 | BIT1 | BIT2);
	partconfig |= BIT0 | BIT1;

	if (!pu8_data_buf) {
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
			   "eMMC Err: data buf is NULL: %lXh\n", (unsigned long)pu8_data_buf);
		return EMMC_ST_ERR_INVALID_PARAM;
	}

	u32_err = emmc_modify_ext_csd(MMC_SWITCH_MODE_WRITE_BYTE, EXT_CSD_PART_CONF, partconfig);
	if (u32_err != EMMC_ST_SUCCESS) {
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
			   "eMMC Err: %Xh, eMMC, set Ext_CSD[179]: %Xh fail\n",
			   u32_err, partconfig);
		goto LABEL_END;
	}
    // read RPMB raw data
	memset(&GLOBAL_EMMCRPMB, '\0', sizeof(struct EMMC_RPMB_DATA));
	GLOBAL_EMMCRPMB.u16_addr     = u16_blk_addr;
	GLOBAL_EMMCRPMB.u16_req_rsp  = EMMC_RPMB_REQ_AUTH_DATA_R;
	memcpy(GLOBAL_EMMCRPMB.u8_nonce, pu8_nonce, EMMC_RPMB_NONCE_FIELD_SIZE);

	emmc_rpmb_swap_memcpy(GLOBAL_EMMC_SECTORBUF, (U8 *)(&GLOBAL_EMMCRPMB),
			      sizeof(struct EMMC_RPMB_DATA));
	u32_err = emmc_cmd25_rpmb_miu(0, GLOBAL_EMMC_SECTORBUF, 1, 0);
	if (u32_err != EMMC_ST_SUCCESS)
		goto LABEL_END;

	u32_err = emmc_cmd18_rpmb_miu(0, pu8_data_buf, 1);
	if (u32_err != EMMC_ST_SUCCESS)
		goto LABEL_END;

	emmc_rpmb_copy_result((U8 *)(&GLOBAL_EMMCRPMB), pu8_data_buf, sizeof(struct EMMC_RPMB_DATA));
	u32_err = emmc_rpmb_chk_result_err();

LABEL_END:
	if (emmc_modify_ext_csd(MMC_SWITCH_MODE_WRITE_BYTE, EXT_CSD_PART_CONF, origpartconfig) !=
		EMMC_ST_SUCCESS) {
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
			   "eMMC Err: eMMC, set Ext_CSD[179]: %Xh fail\n",
			   origpartconfig);
		emmc_fcie_err_handler_stop();
	}

	return u32_err;
}

U32 emmc_rpmb_write_blk(U8 *pu8_data_buf)
{
	U32 u32_err = 0;
	U8 partconfig, origpartconfig;

	partconfig = emmc_drv.u8_partition_config;
	origpartconfig = emmc_drv.u8_partition_config;
	partconfig &= ~(BIT0 | BIT1 | BIT2);
	partconfig |= BIT0 | BIT1;

	if (!pu8_data_buf) {
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
			   "eMMC Err: Write buf is NULL: %lXh\n", (unsigned long)pu8_data_buf);
		return EMMC_ST_ERR_INVALID_PARAM;
	}

	u32_err = emmc_modify_ext_csd(MMC_SWITCH_MODE_WRITE_BYTE, EXT_CSD_PART_CONF, partconfig);
	if (u32_err != EMMC_ST_SUCCESS) {
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
			   "eMMC Err: %Xh, eMMC, set Ext_CSD[179]: %Xh fail\n",
			   u32_err, partconfig);
		goto LABEL_END;
	}
    // copy nonce
	memcpy(&GLOBAL_EMMCRPMB, pu8_data_buf, sizeof(struct EMMC_RPMB_DATA));


    // write data
	emmc_rpmb_swap_memcpy(GLOBAL_EMMC_SECTORBUF, pu8_data_buf, EMMC_SECTOR_512BYTE);
	u32_err = emmc_cmd25_rpmb_miu(0, GLOBAL_EMMC_SECTORBUF, 1, 1);
	if (u32_err != EMMC_ST_SUCCESS)
		goto LABEL_END;

    //=============read result==========================
	memset(&GLOBAL_EMMCRPMB, '\0', sizeof(struct EMMC_RPMB_DATA));
	GLOBAL_EMMCRPMB.u16_req_rsp = EMMC_RPMB_REQ_RESULT_R;
	u32_err = emmc_rpmb_no_data_req();

LABEL_END:
	if (EMMC_ST_SUCCESS !=
	    emmc_modify_ext_csd(MMC_SWITCH_MODE_WRITE_BYTE, EXT_CSD_PART_CONF, origpartconfig)) {
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
			   "eMMC Err: eMMC, set Ext_CSD[179]: %Xh fail\n", origpartconfig);
		emmc_fcie_err_handler_stop();
	}
	return u32_err;
}

//1: re-scend command, 0: Not re-scend command
static U32 emmc_rpmb_rescan_skew(U8 *retry_cnt)
{
	if (emmc_drv.u8_emmc_pll_skew4 &&
	    (emmc_drv.pad_type == FCIE_EMMC_HS400 ||
	    emmc_drv.pad_type == FCIE_EMMC_HS400_5_1)) {
		if ((*retry_cnt) < EMMC_MAX_RETRY_CNT) {
			(*retry_cnt) += 1;
			return  1;
		}
	}
	return 0;
}

U32 emmc_rpmb_if_key_written(void)
{
	U32 u32_err = 0, u32_ret = 0, key_written = 0;
	U8 partconfig, origpartconfig, retry_cnt = 0;

	#ifdef CONFIG_MULTICORES_PLATFORM
	unsigned long irq_flag = 0;

	smp_spin_lock_save(&emmc_spin_lock, irq_flag);
	#endif

	origpartconfig = emmc_drv.u8_partition_config;
	partconfig = emmc_drv.u8_partition_config;
	partconfig &= ~(BIT0 | BIT1 | BIT2);
	partconfig |= BIT0 | BIT1;

LABEL_SEND_CMD:
	u32_err = emmc_modify_ext_csd(MMC_SWITCH_MODE_WRITE_BYTE, EXT_CSD_PART_CONF, partconfig);
	if (u32_err != EMMC_ST_SUCCESS) {
		if (emmc_rpmb_rescan_skew(&retry_cnt))
			goto LABEL_SEND_CMD;

		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
			   "eMMC Err: %Xh, eMMC, set Ext_CSD[179]: %Xh fail\n",
			   u32_err, partconfig);
		goto LABEL_END;
	}

LABEL_SEND_RPMB_CMD:
    
    // read data to check key is written or not
	memset(&GLOBAL_EMMCRPMB, '\0', sizeof(struct EMMC_RPMB_DATA));
	GLOBAL_EMMCRPMB.u16_addr     = 0;
	GLOBAL_EMMCRPMB.u16_req_rsp  = EMMC_RPMB_REQ_AUTH_DATA_R;

	emmc_rpmb_swap_memcpy(GLOBAL_EMMC_SECTORBUF,
			      (U8 *)(&GLOBAL_EMMCRPMB), sizeof(struct EMMC_RPMB_DATA));
	u32_err = emmc_cmd25_rpmb_miu(0, GLOBAL_EMMC_SECTORBUF, 1, 0);
	if (u32_err != EMMC_ST_SUCCESS) {
		if (emmc_rpmb_rescan_skew(&retry_cnt))
			goto LABEL_SEND_RPMB_CMD;

		goto LABEL_END;
	}

	u32_err = emmc_cmd18_rpmb_miu(0, GLOBAL_EMMC_SECTORBUF, 1);

	if (u32_err != EMMC_ST_SUCCESS) {
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: R fail: %Xh\n", u32_err);
		if (emmc_rpmb_rescan_skew(&retry_cnt))
			goto LABEL_SEND_RPMB_CMD;

		goto LABEL_END;
	}

	emmc_rpmb_copy_result((U8 *)(&GLOBAL_EMMCRPMB), GLOBAL_EMMC_SECTORBUF,
			      sizeof(struct EMMC_RPMB_DATA));
	u32_err = emmc_rpmb_chk_result_err();

LABEL_END:
	u32_ret = emmc_modify_ext_csd(MMC_SWITCH_MODE_WRITE_BYTE, EXT_CSD_PART_CONF, origpartconfig);
	if (u32_ret != EMMC_ST_SUCCESS) {
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
			   "eMMC Err: eMMC, set Ext_CSD[179]: %Xh fail\n", origpartconfig);
		if (emmc_rpmb_rescan_skew(&retry_cnt))
			goto LABEL_END;

		emmc_fcie_err_handler_stop();
	}

	if (u32_err != EMMC_ST_SUCCESS)
		emmc_fcie_err_handler_stop();

	if (GLOBAL_EMMCRPMB.u16_result == EMMC_RPMB_DEVICE_CONFIG_R_REQ) {
		#ifdef CONFIG_MULTICORES_PLATFORM
		smp_spin_unlock_restore(&emmc_spin_lock, irq_flag);
		#endif
		key_written = 0;
	} else {
		#ifdef CONFIG_MULTICORES_PLATFORM
		smp_spin_unlock_restore(&emmc_spin_lock, irq_flag);
		#endif
		key_written = 1;
	}

	return key_written;
}

U32 emmc_rpmb_get_counter_for_cmd(U32 *pu32_wcnt)
{
	U32 u32_err;
	U8 partconfig, origpartconfig;

	partconfig = emmc_drv.u8_partition_config;
	origpartconfig = emmc_drv.u8_partition_config;
	partconfig &= ~(BIT0 | BIT1 | BIT2);
	partconfig |= BIT0 | BIT1;

	u32_err = emmc_modify_ext_csd(MMC_SWITCH_MODE_WRITE_BYTE, EXT_CSD_PART_CONF, partconfig);
	if (u32_err != EMMC_ST_SUCCESS) {
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
			   "eMMC Err: %Xh, eMMC, set Ext_CSD[179]: %Xh fail\n",
			   u32_err, BIT3 | BIT1 | BIT0);
		goto LABEL_END;
	}

	memset((void *)(&GLOBAL_EMMCRPMB), '\0', sizeof(struct EMMC_RPMB_DATA));
	GLOBAL_EMMCRPMB.u16_req_rsp = EMMC_RPMB_REQ_WRITE_CNT_VAL;
	u32_err = emmc_rpmb_no_data_req();
	if (u32_err == EMMC_ST_SUCCESS)
		*pu32_wcnt = GLOBAL_EMMCRPMB.u32_writecnt;

LABEL_END:
	if (emmc_modify_ext_csd(MMC_SWITCH_MODE_WRITE_BYTE, EXT_CSD_PART_CONF, origpartconfig) !=
	    EMMC_ST_SUCCESS) {
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
			   "eMMC Err: eMMC, set Ext_CSD[179]: %Xh fail\n", origpartconfig);
		emmc_fcie_err_handler_stop();
	}

	return u32_err;
}


#if defined(CONFIG_SECURITY_STORE_IN_EMMC_FLASH) && CONFIG_SECURITY_STORE_IN_EMMC_FLASH

#include <secure/crypto_hmac.h>
#include <secure/crypto_sha.h>

U32 emmc_rpmb_program_auth_key(void)
{
	U32 u32_err = 0;
	U8  u8_auto_key[32];
	U8 partconfig, origpartconfig;

	partconfig = emmc_drv.u8_partition_config;
	origpartconfig = emmc_drv.u8_partition_config;
	partconfig &= ~(BIT0 | BIT1 | BIT2);
	partconfig |= BIT0 | BIT1;

	get_hmac_key(u8_auto_key);

	u32_err = emmc_modify_ext_csd(MMC_SWITCH_MODE_WRITE_BYTE, EXT_CSD_PART_CONF, partconfig);
	if (u32_err != EMMC_ST_SUCCESS) {
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
			   "eMMC Err: %Xh, eMMC, set Ext_CSD[179]: %Xh fail\n",
			   u32_err, partconfig);
		goto LABEL_END;
	}

    //==============write auth key========================
	memset((void *)(&GLOBAL_EMMCRPMB), '\0', sizeof(struct EMMC_RPMB_DATA));
	memcpy(GLOBAL_EMMCRPMB. u8_auth_key, u8_auto_key, EMMC_RPMB_KEY_SIZE);
	GLOBAL_EMMCRPMB.u16_req_rsp = EMMC_RPMB_REQ_AUTH_KEY;
	emmc_rpmb_swap_memcpy(GLOBAL_EMMC_SECTORBUF,
			      (U8 *)(&GLOBAL_EMMCRPMB), sizeof(struct EMMC_RPMB_DATA));

	u32_err = emmc_cmd25_rpmb_miu(0, GLOBAL_EMMC_SECTORBUF, 1, 1);
	if (u32_err != EMMC_ST_SUCCESS)
		goto LABEL_END;

    //=============read result==========================
	memset((void *)(&GLOBAL_EMMCRPMB), '\0', sizeof(struct EMMC_RPMB_DATA));
	GLOBAL_EMMCRPMB.u16_req_rsp = EMMC_RPMB_REQ_RESULT_R;
	u32_err = emmc_rpmb_no_data_req();

LABEL_END:
	if (EMMC_ST_SUCCESS !=
		emmc_modify_ext_csd(MMC_SWITCH_MODE_WRITE_BYTE,
				    EXT_CSD_PART_CONF, origpartconfig)) {
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
			   "eMMC Err: set Ext_CSD[179]: %Xh fail\n", origpartconfig);
		emmc_fcie_err_handler_stop();
	}

	return u32_err;
}

U32 emmc_rpmb_get_counter(U32 *pu32_wcnt)
{
	U32 u32_err;

	memset((void *)(&GLOBAL_EMMCRPMB), '\0', sizeof(struct EMMC_RPMB_DATA));
	GLOBAL_EMMCRPMB.u16_req_rsp = EMMC_RPMB_REQ_WRITE_CNT_VAL;

	u32_err = emmc_rpmb_no_data_req();
	if (u32_err == EMMC_ST_SUCCESS)
		*pu32_wcnt = GLOBAL_EMMCRPMB.u32_writecnt;

	if (u32_err != EMMC_ST_SUCCESS)
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: Get Counter fail: %Xh\n", u32_err);

	return u32_err;
}


U32 emmc_rpmb_write_data(U8 *pu8_data_buf, U32 u32_data_byte_cnt, U16 u16_blk_addr)
{
	U32 u32_err = 0, u32_ret = 0;
	U32 u32_wcnt;

	U16 u16_i, u16_blk_cnt;
	U8  u8_EMMC_MAC_CALBUF[EMMC_RPMB_MAC_CAL_FIELD_SIZE << 1];
	U8  u8_auto_key[EMMC_RPMB_KEY_SIZE];
	U8  u8_input_key[EMMC_RPMB_KEY_SIZE];
	U8 partconfig, origpartconfig, retry_cnt = 0;

	if (!pu8_data_buf) {
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
			   "eMMC Err: Write buf is NULL: %Xh\n", (U32)pu8_data_buf);
		return EMMC_ST_ERR_INVALID_PARAM;
	}

	if (u32_data_byte_cnt & EMMC_RPMB_DATA_FIELD_MASK) {
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
			   "eMMC Err: data not 256B boundary: %Xh\n", u32_data_byte_cnt);
		return EMMC_ST_ERR_INVALID_PARAM;
	}
	if (u16_blk_addr + (u32_data_byte_cnt >> EMMC_RPMB_DATA_FIELD_SHIFT) >
		emmc_drv.u32_rpmb_size_sec_cnt) {
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
			   "eMMC Err: invalid data range, %Xh > %Xh\n",
			    u16_blk_addr + (u32_data_byte_cnt >> EMMC_RPMB_DATA_FIELD_SHIFT),
			   emmc_drv.u32_rpmb_size_sec_cnt);

		return EMMC_ST_ERR_INVALID_PARAM;
	}

	partconfig = emmc_drv.u8_partition_config;
	origpartconfig = emmc_drv.u8_partition_config;
	partconfig &= ~(BIT0 | BIT1 | BIT2);
	partconfig |= BIT0 | BIT1;

	get_hmac_key(u8_auto_key);

LABEL_SEND_CMD:
	u32_err = emmc_modify_ext_csd(MMC_SWITCH_MODE_WRITE_BYTE, EXT_CSD_PART_CONF, partconfig);
	if (u32_err != EMMC_ST_SUCCESS) {
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
			   "eMMC Err: %Xh, eMMC, set Ext_CSD[179]: %Xh fail\n",
			   u32_err, partconfig);
		if (emmc_rpmb_rescan_skew(&retry_cnt))
			goto LABEL_SEND_CMD;

		goto LABEL_END;
	}
    // write data
	while (u32_data_byte_cnt) {
LABEL_SEND_RPMB_CMD:

		u32_wcnt = 0;
		emmc_rpmb_get_counter(&u32_wcnt);
		if (GLOBAL_EMMCRPMB.u16_result & EMMC_RPMB_WCNT_MAX_FLAG) {
			u32_err = EMMC_ST_ERR_RPMB_WCNT_MAX;
			break;
		}

		if (u16_blk_addr & 1) {
			u16_blk_cnt = 1;
		} else {
			if (u32_data_byte_cnt >= EMMC_SECTOR_512BYTE)
				u16_blk_cnt = 1 << 1;
			else
				u16_blk_cnt = 1;
		}

		memset(u8_EMMC_MAC_CALBUF, '\0', EMMC_RPMB_MAC_CAL_FIELD_SIZE << 1);
		memset(GLOBAL_EMMC_SECTORBUF, '\0', EMMC_SECTOR_512BYTE << 1);

		for (u16_i = 0; u16_i < u16_blk_cnt; u16_i++) {
			memset(&GLOBAL_EMMCRPMB, '\0', sizeof(struct EMMC_RPMB_DATA));
			GLOBAL_EMMCRPMB.u32_writecnt = u32_wcnt;
			GLOBAL_EMMCRPMB.u16_addr     = u16_blk_addr;
			GLOBAL_EMMCRPMB.u16_blk_cnt  = u16_blk_cnt;
			GLOBAL_EMMCRPMB.u16_req_rsp  = EMMC_RPMB_REQ_AUTH_DATA_W;
			memcpy(GLOBAL_EMMCRPMB.u8_data, pu8_data_buf +
			       (u16_i << EMMC_RPMB_DATA_FIELD_SHIFT),
			       EMMC_RPMB_DATA_FIELD_SIZE);
			emmc_rpmb_swap_memcpy(u8_EMMC_MAC_CALBUF + u16_i *
					      EMMC_RPMB_MAC_CAL_FIELD_SIZE,
					      (U8 *)(&GLOBAL_EMMCRPMB), EMMC_RPMB_MAC_CAL_FIELD_SIZE);
			emmc_rpmb_swap_memcpy(GLOBAL_EMMC_SECTORBUF +
					      (u16_i << EMMC_SECTOR_512BYTE_BITS),
					      (U8 *)(&GLOBAL_EMMCRPMB),
					      sizeof(struct EMMC_RPMB_DATA));
		}

		emmc_rpmb_swap_memcpy(u8_input_key, u8_auto_key, EMMC_RPMB_KEY_SIZE);
		hmac_sha256(u8_EMMC_MAC_CALBUF, u16_i * EMMC_RPMB_MAC_CAL_FIELD_SIZE,
			    u8_input_key, EMMC_RPMB_KEY_SIZE, u8_digest);
		memcpy(GLOBAL_EMMC_SECTORBUF +
			(((u16_i - 1) << EMMC_SECTOR_512BYTE_BITS) + EMMC_RPMB_STUFF_BYTE),
			u8_digest, EMMC_RPMB_KEY_SIZE);

		u32_err = emmc_cmd25_rpmb_miu(0, GLOBAL_EMMC_SECTORBUF, u16_blk_cnt, 1);
		if (u32_err != EMMC_ST_SUCCESS) {
			if (emmc_rpmb_rescan_skew(&retry_cnt))
				goto LABEL_SEND_RPMB_CMD;

			break;
		}

        //=============read result==========================
		memset(&GLOBAL_EMMCRPMB, '\0', sizeof(struct EMMC_RPMB_DATA));
		GLOBAL_EMMCRPMB.u16_req_rsp = EMMC_RPMB_REQ_RESULT_R;
		u32_err = emmc_rpmb_no_data_req();
		if (u32_err != EMMC_ST_SUCCESS)
			break;

		u16_blk_addr += u16_blk_cnt;
		pu8_data_buf  += u16_blk_cnt << EMMC_RPMB_DATA_FIELD_SHIFT;
		u32_data_byte_cnt -= u16_blk_cnt << EMMC_RPMB_DATA_FIELD_SHIFT;
	}

LABEL_END:

	u32_ret = emmc_modify_ext_csd(MMC_SWITCH_MODE_WRITE_BYTE, EXT_CSD_PART_CONF, origpartconfig);

	if (u32_ret != EMMC_ST_SUCCESS) {
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
			   "eMMC Err: eMMC, set Ext_CSD[179]: %Xh fail\n", origpartconfig);
		if (emmc_rpmb_rescan_skew(&retry_cnt))
			goto LABEL_END;

		emmc_fcie_err_handler_stop();
	}
	return u32_err;
}

U32 emmc_rpmb_verify_hmac(U8 *pu8_data_buf, U16 u16_blk_cnt)
{
	U32 u32_err = 0;
	U16 u16_i;
	U8  u8_emmc_mac_cal_buf[568];
	U8  u8_digest_golden[32];
	U8  u8_digest_from_emmc[32];
	U8  u8_auto_key[32];
	U8  u8_input_key[32];

	get_hmac_key(u8_auto_key);
	emmc_rpmb_swap_memcpy(u8_input_key, u8_auto_key, 32);

	for (u16_i = 0; u16_i < u16_blk_cnt; u16_i++)
		memcpy(u8_emmc_mac_cal_buf + (u16_i * 284), pu8_data_buf + (u16_i << 9) + 228, 284);

	memcpy(u8_digest_from_emmc, pu8_data_buf + ((u16_blk_cnt - 1) << 9) + 196, 32);
	hmac_sha256(u8_emmc_mac_cal_buf, u16_blk_cnt * 284, u8_input_key, 32, u8_digest_golden);

	if (memcmp(u8_digest_golden, u8_digest_from_emmc, 32)) {
	    emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: HMAC Mismatch!\n");
	    emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "HMAC real:\n");
	    emmc_dump_mem(u8_digest_golden, 32);
	    emmc_debug(0, 0, "\n");
	    emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "HMAC from eMMC:\n");
	    emmc_dump_mem(u8_digest_from_emmc, 32);
	    emmc_debug(0, 0, "\n");
	    u32_err = EMMC_ST_ERR_RPMB_HMAC_ERR;
	}

	return u32_err;
}

U32 emmc_rpmb_read_data_ex(U8 *pu8_data_buf, U32 u32_data_byte_cnt, U16 u16_blk_addr, U8 verify)
{
	U32 u32_err = 0, u32_ret = 0;
	U16 u16_i, u16_blk_cnt;
	U8 partconfig, origpartconfig, retry_cnt = 0;

	if (!pu8_data_buf) {
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
			   "eMMC Err: r data buf is NULL: %Xh\n", (U32)pu8_data_buf);
		return EMMC_ST_ERR_INVALID_PARAM;
	}

	if (u32_data_byte_cnt & EMMC_RPMB_DATA_FIELD_MASK) {
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
			   "eMMC Err: data not 256B boundary: %Xh\n", u32_data_byte_cnt);
		return EMMC_ST_ERR_INVALID_PARAM;
	}
	if (u16_blk_addr + (u32_data_byte_cnt >> EMMC_RPMB_DATA_FIELD_SHIFT) >
	    emmc_drv.u32_rpmb_size_sec_cnt) {
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: invalid data range, %Xh > %Xh\n",
			   u16_blk_addr + (u32_data_byte_cnt >> EMMC_RPMB_DATA_FIELD_SHIFT),
			   emmc_drv.u32_rpmb_size_sec_cnt);
		return EMMC_ST_ERR_INVALID_PARAM;
	}

	origpartconfig = emmc_drv.u8_partition_config;
	partconfig = emmc_drv.u8_partition_config;
	partconfig &= ~(BIT0 | BIT1 | BIT2);
	partconfig |= BIT0 | BIT1;

LABEL_SEND_CMD:
	u32_err = emmc_modify_ext_csd(MMC_SWITCH_MODE_WRITE_BYTE, EXT_CSD_PART_CONF, partconfig);
	if (u32_err != EMMC_ST_SUCCESS) {
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
			   "eMMC Err: %Xh, eMMC, set Ext_CSD[179]: %Xh fail\n",
			   u32_err, partconfig);
		if (emmc_rpmb_rescan_skew(&retry_cnt))
			goto LABEL_SEND_CMD;

		goto LABEL_END;
	}
    // read data
	while (u32_data_byte_cnt) {
LABEL_SEND_RPMB_CMD:
		memset(&GLOBAL_EMMCRPMB, '\0', sizeof(struct EMMC_RPMB_DATA));
		GLOBAL_EMMCRPMB.u16_addr     = u16_blk_addr;
		GLOBAL_EMMCRPMB.u16_req_rsp  = EMMC_RPMB_REQ_AUTH_DATA_R;
		emmc_rpmb_swap_memcpy(GLOBAL_EMMC_SECTORBUF,
				      (U8 *)(&GLOBAL_EMMCRPMB), sizeof(struct EMMC_RPMB_DATA));
		u32_err = emmc_cmd25_rpmb_miu(0, GLOBAL_EMMC_SECTORBUF, 1, 0);
		if (u32_err != EMMC_ST_SUCCESS) {
			if (emmc_rpmb_rescan_skew(&retry_cnt))
				goto LABEL_SEND_RPMB_CMD;

			break;
		}

		if (u16_blk_addr & 1) {
			u16_blk_cnt = 1;
		} else {
			if (u32_data_byte_cnt >= EMMC_SECTOR_512BYTE)
				u16_blk_cnt = 1 << 1;
			else
				u16_blk_cnt = 1;
		}
		u32_err = emmc_cmd18_rpmb_miu(0, GLOBAL_EMMC_SECTORBUF, u16_blk_cnt);

		if (u32_err != EMMC_ST_SUCCESS) {
			emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: R fail: %Xh\n", u32_err);
			if (emmc_rpmb_rescan_skew(&retry_cnt))
				goto LABEL_SEND_RPMB_CMD;

			break;
		}

		if (verify != 0) {
			u32_err = emmc_rpmb_verify_hmac(GLOBAL_EMMC_SECTORBUF, u16_blk_cnt);
			if (u32_err != EMMC_ST_SUCCESS) {
				emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
					   "eMMC Err: R fail: %Xh\n", u32_err);
				break;
			}
		}

		for (u16_i = 0; u16_i < u16_blk_cnt; u16_i++) {
			emmc_rpmb_swap_memcpy(pu8_data_buf + (u16_i << EMMC_RPMB_DATA_FIELD_SHIFT),
					      GLOBAL_EMMC_SECTORBUF +
					      (u16_i << EMMC_SECTOR_512BYTE_BITS) +
					      EMMC_RPMB_STUFF_BYTE + EMMC_RPMB_KEY_SIZE,
					      EMMC_RPMB_DATA_FIELD_SIZE);
			emmc_rpmb_copy_result((U8 *)(&GLOBAL_EMMCRPMB),
					      GLOBAL_EMMC_SECTORBUF +
					      (u16_i << EMMC_SECTOR_512BYTE_BITS),
					      sizeof(struct EMMC_RPMB_DATA));
			u32_err = emmc_rpmb_chk_result_err();
			if (u32_err)
				break;
		}

		if (u16_i != u16_blk_cnt) {
			mmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: R fail: %Xh\n", u32_err);
			break;
		}

		u16_blk_addr += u16_blk_cnt;
		pu8_data_buf  += u16_blk_cnt << EMMC_RPMB_DATA_FIELD_SHIFT;
		u32_data_byte_cnt -= u16_blk_cnt << EMMC_RPMB_DATA_FIELD_SHIFT;
	}

LABEL_END:
	u32_ret = emmc_modify_ext_csd(MMC_SWITCH_MODE_WRITE_BYTE, EXT_CSD_PART_CONF, origpartconfig);
	if (u32_ret != EMMC_ST_SUCCESS) {
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
			   "eMMC Err: eMMC, set Ext_CSD[179]: %Xh fail\n", origpartconfig);
		if (emmc_rpmb_rescan_skew(&retry_cnt))
			goto LABEL_END;

		emmc_fcie_err_handler_stop();
	}

	return u32_err;
}

U32 emmc_rpmb_read_data(U8 *pu8_data_buf, U32 u32_data_byte_cnt, U16 u16_blk_addr)
{
	return emmc_rpmb_read_data_ex(pu8_data_buf, u32_data_byte_cnt, u16_blk_addr, 1);
}

U32 emmc_rpmb_read_data_no_verify(U8 *pu8_data_buf, U32 u32_data_byte_cnt, U16 u16_blk_addr)
{
	return emmc_rpmb_read_data_ex(pu8_data_buf, u32_data_byte_cnt, u16_blk_addr, 0);
}

U32 emmc_rpmb_test(void)
{
	U32 u32_err = 0;
	#if defined(IF_IP_VERIFY) && IF_IP_VERIFY
	U32 u32_i, u32_data_byte_cnt = 128 * 1024;
	U16 u16_blk_addr = 0;

	if (emmc_drv.u32_rpmb_size_sec_cnt == 0) {
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC has no RPMB partition size\n");
		goto LABEL_END;
	}

	if (!gau8_wbuf)
		gau8_wbuf = (U8 *)memalign(EMMC_CACHE_LINE, u32_data_byte_cnt);

	if (!gau8_rbuf)
		gau8_rbuf = (U8 *)memalign(EMMC_CACHE_LINE, u32_data_byte_cnt);

	for (u32_i = 0; u32_i < u32_data_byte_cnt; u32_i++) {
		gau8_wbuf[u32_i] = (U8)(u32_i & 0xFF);
		gau8_rbuf[u32_i] = ~gau8_wbuf[u32_i];
	}

	u32_err = emmc_rpmb_write_data(gau8_wbuf, u32_data_byte_cnt, u16_blk_addr);
	if (u32_err != EMMC_ST_SUCCESS)
		goto LABEL_END;
	u32_err = emmc_rpmb_read_data(gau8_rbuf, u32_data_byte_cnt, u16_blk_addr);
	if (u32_err != EMMC_ST_SUCCESS)
		goto LABEL_END;

	u32_err = emmc_compare_data(gau8_wbuf, gau8_rbuf, u32_data_byte_cnt);
	if (u32_err != EMMC_ST_SUCCESS)
		goto LABEL_END;

LABEL_END:

	if (gau8_wbuf) {
		free(gau8_wbuf);
		gau8_wbuf = 0;
	}

	if (gau8_rbuf) {
		free(gau8_rbuf);
		gau8_rbuf = 0;
	}

	#endif
	return u32_err;
}
#endif

