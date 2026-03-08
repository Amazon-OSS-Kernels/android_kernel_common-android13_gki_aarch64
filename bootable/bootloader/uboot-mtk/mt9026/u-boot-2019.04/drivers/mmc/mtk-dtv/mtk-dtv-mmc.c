// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (C) 2023 MediaTek Inc.
 */
//#include "mtk-dtv-mmc.h"
#include "eMMC.h"
#ifdef CONFIG_MULTICORES_PLATFORM
#include <smp/thread_info.h>
smp_spin_lock_t emmc_spin_lock = INIT_SPIN_LOCK;
#endif


static void mtk_fcie_reset_hw(struct mtk_fcie_host *host)
{
	emmc_fcie_reset();
}
static u32 mtk_fcie_cmd_prepare_raw_cmd(struct mtk_fcie_host *host,
				    struct mmc_cmd *cmd,
				    struct mmc_data *data)
{
	u32 sd_ctl = BIT_SD_CMD_EN;
	u32 sd_mode = host->host_sd_mode;
	u32 wait_event = BIT_CMD_END;
	u16 reg_val = 0;

	//clear all event
	writew(BIT_ALL_CARD_INT_EVENTS, &host->base->fcie_mie_event);
	reg_val = readw(&host->base->fcie_mie_event);
	if (reg_val & BIT_ALL_CARD_INT_EVENTS)
		mtk_fcie_reset_hw(host);

	setbits_le16(&host->base->fcie_sd_status, BIT_SD_FCIE_ERR_FLAGS);

	//fill ctrl and rsp register
	writew((((cmd->cmdarg>> 24)<<8) | (0x40|cmd->cmdidx))     , &host->base->fcie_cmd_fifo[0]);
	writew(((cmd->cmdarg & 0xFF00) | ((cmd->cmdarg>>16)&0xFF)), &host->base->fcie_cmd_fifo[1]);
	writew((cmd->cmdarg & 0xFF)                                 , &host->base->fcie_cmd_fifo[2]);

	if(data) {
		if(data->flags == MMC_DATA_READ) {
			sd_ctl |= BIT_SD_DTRX_EN;
			wait_event |= BIT_DMA_END;
		}
	}

    writew((5)<< BIT_CMD_SIZE_SHIFT, &host->base->fcie_cmd_rsp_size);	//eMMC_CMD_BYTE_CNT

	if(cmd->resp_type != MMC_RSP_NONE) {
		sd_ctl |= BIT_SD_RSP_EN;
		if(cmd->resp_type == MMC_RSP_R2) {
			sd_ctl |= BIT_SD_RSPR2_EN;
            setbits_le16(&host->base->fcie_cmd_rsp_size, 16);
		}
		else
			setbits_le16(&host->base->fcie_cmd_rsp_size, 5);
	}

	writew(sd_mode, &host->base->fcie_sd_mode);
    writew(sd_ctl, &host->base->fcie_sd_ctrl);

	return wait_event;
}

//#if CONFIG_IS_ENABLED(MMC_UHS_SUPPORT)
static int mtk_fcie_ops_wait_dat0(struct udevice *dev, int state,
					  int timeout_us)
{
	int ret = -ETIMEDOUT;
	bool dat0_high;
	bool target_dat0_high = !!state;
	struct mtk_fcie_host *host = dev_get_priv(dev);

	timeout_us = DIV_ROUND_UP(timeout_us, 10); /* check every 10 us. */
	while (timeout_us--) {
		dat0_high = !(readw(&host->base->fcie_sd_status) & BIT_SD_CARD_BUSY);
		if (dat0_high == target_dat0_high) {
			ret = 0;
			break;
		}
		udelay(10);
	}

	if (ret != 0)
		emmc_debug(0, 1, "eMMC Err: eMMC is busy\n");

	return ret;
}
//#endif

static int mtk_fcie_skip_response_err(struct mmc_cmd *cmd)
{
	int ret = 0;

	//Ignore response error of cmd8/cmd17/cmd18
	if (cmd->cmdidx == MMC_CMD_SEND_EXT_CSD ||
	    cmd->cmdidx == MMC_CMD_SET_BLOCK_COUNT ||
	    cmd->cmdidx == MMC_CMD_READ_SINGLE_BLOCK ||
	    cmd->cmdidx == MMC_CMD_READ_MULTIPLE_BLOCK ||
	    cmd->cmdidx == MMC_CMD_WRITE_SINGLE_BLOCK ||
	    cmd->cmdidx == MMC_CMD_WRITE_MULTIPLE_BLOCK ||
	    cmd->cmdidx == MMC_CMD_ERASE_GROUP_START ||
	    cmd->cmdidx == MMC_CMD_ERASE_GROUP_END ||
	    cmd->cmdidx == MMC_CMD_ERASE) {
		cmd->response[0] = CMD_R1_RESPONSE;
		ret = 1;
	}

	return ret;
}

static void mtk_fcie_if_switch_hs200_to_high(struct mmc_cmd *cmd)
{
	if (emmc_drv.pad_type == FCIE_EMMC_HS200) {
		if (cmd->cmdidx == MMC_CMD_SWITCH && ((cmd->cmdarg & 0xff0000) >> 16) == EXT_CSD_HS_TIMING) {
			if (((cmd->cmdarg & 0xff00) >> 8) == EXT_CSD_TIMING_HS)
				emmc_drv.u8_if_switch_hs200_to_high = 1;
		}
	}
}

static void mtk_fcie_cmd_rsp_crc(struct mtk_fcie_host *host, struct mmc_cmd *cmd, int *ret, u16 status)
{
	if (emmc_drv.u8_emmc_pll_skew4 && ((emmc_drv.pad_type == FCIE_EMMC_HS400 && emmc_drv.t_hs400_table.u8_set_cnt) ||
					   (emmc_drv.pad_type == FCIE_EMMC_HS400_5_1 && emmc_drv.t_table_g.u8_set_cnt))) {
		if (mtk_fcie_skip_response_err(cmd))
			*ret = 0;
		else
			emmc_m6e3_rescan_timing();
	} else {
		mtk_fcie_reset_hw(host);
		emmc_debug(0, 1, "eMMC Err: cmd crc error, FCIE status = %Xh\n", status);
	}
}

static void mtk_fcie_cmd_data_crc(struct mtk_fcie_host *host, u16 status)
{
	if (emmc_drv.u8_emmc_pll_skew4 && ((emmc_drv.pad_type == FCIE_EMMC_HS400 && emmc_drv.t_hs400_table.u8_set_cnt) ||
					   (emmc_drv.pad_type == FCIE_EMMC_HS400_5_1 && emmc_drv.t_table_g.u8_set_cnt)))
		emmc_m6e3_rescan_timing();
	else
		mtk_fcie_reset_hw(host);

	emmc_debug(0, 1, "eMMC Err: data crc error, FCIE status = %Xh\n", status);
}

static int mtk_fcie_cmd_done(struct mtk_fcie_host *host, struct mmc_cmd *cmd)
{
	u8 *u8_rsp = (u8*)cmd->response;
	u16 status;
	int ret = 0;
	int i;

	if (cmd->resp_type & MMC_RSP_PRESENT) {
		for(i=0; i < 15; i++) {
			u8_rsp[(3 - (i % 4)) + (4 * (i / 4))] =
				(u8)( readw(&host->base->fcie_cmd_fifo[((i+1)/2)]) >> (8*((i+1)%2)));
		}
	}

	if (cmd->resp_type & MMC_RSP_CRC) { //not cmd1
		status = readw(&host->base->fcie_sd_status) & BIT_SD_FCIE_ERR_FLAGS;

		if (status > 0) {
			if (status & BIT_SD_RSP_TIMEOUT)
				ret = -ETIMEDOUT;
			else
				ret = -EIO;

			if (status == BIT_SD_RSP_TIMEOUT) {
				mtk_fcie_reset_hw(host);
				emmc_debug(0, 1, "eMMC Err: cmd Rsp timeout error\n");
			} else if (status == BIT_SD_RSP_CRC_ERR) {
				mtk_fcie_cmd_rsp_crc(host, cmd, &ret, status);
			} else {
				mtk_fcie_cmd_data_crc(host, status);
			}
		} else {
			mtk_fcie_if_switch_hs200_to_high(cmd);
		}
	} else {
		if (cmd->cmdidx == 1 && ((cmd->response[0] >> 31) & BIT(0))) {
			emmc_drv.u8_if_sector_mode = ((cmd->response[0] >> 30) & BIT(0));
			host->if_sector_mode = ((cmd->response[0] >> 30) & BIT(0));
		}
	}

	if (cmd->cmdidx == MMC_CMD_ALL_SEND_CID) {
		emmc_drv.u16_rca = 1;
		emmc_fcie_get_cmd_fifo(0, EMMC_R2_BYTE_CNT >> 1, (U16 *)emmc_drv.au8_cid);
		//check customize device driving table
		host->id_matched_driving = 0xff;
		for (i = 0 ; i < host->dts_cids_count; i++) {
			if (!memcmp(&emmc_drv.au8_cid[1], host->cid_drivings[i].cid, 10))
				if (host->cid_drivings[i].device_driving >= 0 &&
				    host->cid_drivings[i].device_driving <= 4)
					host->id_matched_driving = host->cid_drivings[i].device_driving;
		}
	}

	if(cmd->resp_type & MMC_RSP_BUSY)
		host->last_resp_type = 1;

	return ret;
}

static bool mtk_fcie_mmc_is_ready(struct mtk_fcie_host *host)
{
	int ret;

	setbits_le16(&host->base->fcie_sd_mode, BIT_SD_CLK_EN);

	/* The max busy time we can endure is 10s */
	ret = emmc_fcie_wait_d0_high(10 * 1000 * 1000);

	if (ret) {
		pr_err("eMMC Err: CMD bus busy detected\n");
		mtk_fcie_reset_hw(host);
		return false;
	}

	return true;
}
static dmaaddr_t mtk_fcie_dma_address_trans(dmaaddr_t address, u32 bytecnt)
{
	//should configure by dts?
	return emmc_translate_dma_address_ex(address, bytecnt);
}
static void mtk_fcie_dma_address_trans_post(dmaaddr_t address, u32 bytecnt)
{
	//should configure by dts?
	emmc_invalidate_data_cache_buffer(address, bytecnt);
}
static int mtk_fcie_get_fde(struct mtk_fcie_host *host, u32 lba)
{
	struct emmc_crypto_disable_info *info;
	struct list_head *head = &host->crypto_white_list;

	if(!host->mmc->has_init)
		return 0;

	if (!list_empty(head)) {
		list_for_each_entry(info, head, list) {
			if((lba >= info->lba) && (lba < (info->lba+info->length))) {
				//printf("%s: lba %llx length %llx\n", "crypto-disable-list",
				//	info->lba, info->length);
				return 0;
			}
		}
	}
	return 1;
}

static void mtk_fcie_part_config(struct mtk_fcie_host *mtk_fcie_host, struct mmc_cmd *cmd)
{
	u8 patition_config = emmc_drv.u8_partition_config;

	if (cmd->cmdidx == 0x6) {
		if (((cmd->cmdarg & 0xff0000) >> 16) == 0xb3) {
			if (((cmd->cmdarg >> 24) & 3) == MMC_SWITCH_MODE_WRITE_BYTE) {
				patition_config = (cmd->cmdarg & 0xff00) >> 8;
				if ((patition_config >> 3) != 0x1 && (patition_config >> 3) != 0x2) {
					emmc_debug(0, 1,
						   "emmc warn: please check user set boot config %Xh\n",
						   patition_config);
				}
				emmc_drv.u8_partition_config = (cmd->cmdarg & 0xff00) >> 8;
			}
		}
	}
}

#ifdef ENABLE_FCIE_MIU_CHECKSUM

void mtk_fcie_dump_dst(void)
{
	emmc_debug(0, 0, "start blk=%d, blkcnt=%d, cache=%lXh\n", emmc_drv.host->start_blk, emmc_drv.host->blks,
		   (ulong)emmc_drv.host->dst_buf);

	emmc_debug(0, 0, "\ncache\n");

	emmc_dump_mem((u8 *)emmc_drv.host->dst_buf, (u32)emmc_drv.host->blks << EMMC_SECTOR_512BYTE_BITS);

	emmc_debug(0, 0, "\n");
	asm volatile ("b ." );
}

static void mtk_fcie_enable_miu_chksum(struct mtk_fcie_host *host)
{
	clrbits_le16(REG_FCIE1_0x50,  BIT2);

	clrbits_le16(&host->base->fcie_emmc_debug_bus1, BIT11 | BIT10 | BIT9 | BIT8);
	setbits_le16(&host->base->fcie_emmc_debug_bus1, BIT_FCIE_EN_CHKSUM << BIT_FCIE_EN_CHKSUM_SHIFT);

	clrbits_le16(REG_FCIE1_0x50, BIT3);
	clrbits_le16(REG_FCIE1_0x51, BIT15 | BIT14 | BIT13);
}

static void mtk_fcie_clear_miu_chksum(struct mtk_fcie_host *host)
{
	int i;

	// Clear checksum
	setbits_le16(REG_FCIE1_0x50, BIT2);
	//udelay(1);

	// Make sure checksum is cleared
	for (i = FCIE_CHKSUM_DATA_START; i <= FCIE_CHKSUM_DATA_END; i++) {
		clrbits_le16(REG_FCIE1_0x51, BIT12 | BIT11 | BIT10 | BIT9 | BIT8);
		setbits_le16(REG_FCIE1_0x51, i << BIT_FCIE_EN_CHKSUM_SHIFT);

		if (readw(&host->base->fcie_emmc_debug_bus0) != 0) {
			emmc_debug(EMMC_DEBUG_LEVEL, 0, "eMMC Err: Debug bus is not cleared\n");
			while (1)
				;
		}
	}

	clrbits_le16(REG_FCIE1_0x50, BIT2);
}

static u32 mtk_fcie_miu_chksum(struct mtk_fcie_host *host, u8 *dst_buf, u32 bufsize)
{
	int i;

	u16 checksum[FCIE_CHKSUM_SIZE];
	u16 old_checksum[FCIE_CHKSUM_SIZE];
	u16 data_in[FCIE_CHKSUM_SIZE];
	u16 *u16_dst_buf = (u16*)dst_buf;
	u32 u32_err =0;


	memset(checksum, 0, FCIE_CHKSUM_SIZE*sizeof(u16));
	memset(old_checksum, 0, FCIE_CHKSUM_SIZE*sizeof(u16));
	memset(data_in, 0, FCIE_CHKSUM_SIZE*sizeof(u16));

	for (i = 0; i < bufsize / sizeof(u16); i++) {
		data_in[i%FCIE_CHKSUM_SIZE] = u16_dst_buf[i];
		old_checksum[i%FCIE_CHKSUM_SIZE] = checksum[i%FCIE_CHKSUM_SIZE];

		if (i % FCIE_CHKSUM_SIZE != 0)
			checksum[i%FCIE_CHKSUM_SIZE] =
				data_in[i%FCIE_CHKSUM_SIZE]
				^ (((old_checksum[i%FCIE_CHKSUM_SIZE] << BIT0) & BIT_FCIE_CHKSUM_DATA_MASK)
				| (old_checksum[(i-1)%FCIE_CHKSUM_SIZE] >> BIT_FCIE_CHKSUM_DATA_SHIFT));
		else
			checksum[i%FCIE_CHKSUM_SIZE] =
				data_in[i%FCIE_CHKSUM_SIZE]
				^ (((old_checksum[i%FCIE_CHKSUM_SIZE] << BIT0) & BIT_FCIE_CHKSUM_DATA_MASK)
				| (checksum[FCIE_CHKSUM_SIZE-1] >> BIT_FCIE_CHKSUM_DATA_SHIFT));
	}

	for (i = 0; i < FCIE_CHKSUM_SIZE; i++) {
		clrbits_le16(REG_FCIE1_0x51,  BIT12|BIT11|BIT10|BIT9|BIT8);
		setbits_le16(REG_FCIE1_0x51,  (FCIE_CHKSUM_DATA_START+i)<<BIT_FCIE_EN_CHKSUM_SHIFT);

		if (checksum[i] != readw(&host->base->fcie_emmc_debug_bus0)) {
			emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 0, "eMMC Err: FCIE's MIU checksum is not the same with cache!\n\n");
			emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 0, "eMMC Err: FCIE's MIU checksum:0x%x, checksum:0x%x, I:%d\n\n",
				   readw(&host->base->fcie_emmc_debug_bus0), checksum[i], i);

			u32_err = EMMC_ST_ERR_MEM_CORRUPT;
		}
	}

	return u32_err;
}

void mtk_bdma_load(u8 *dst_addr, u8 *src_addr, u32 bufsize)
{
	u16 data = 0;
	int cnt = 0;
	ulong src_pa, dst_pa;

	clrbits_le16(FCIE_MMA_PRI_REG, BIT_MIU_SELECT_MASK);
	src_pa = virt_to_phys((void *)src_addr) - CONFIG_BUSADDR_START;
	//emmc_debug(0, 0, "src_pa=%lX\n", (ulong)src_pa);
	//non-cache
	clrbits_le16(FCIE_MMA_PRI_REG, BIT_MIU_SELECT_MASK);
	dst_pa = virt_to_phys((void *)dst_addr) - CONFIG_BUSADDR_START;
	//emmc_debug(0, 0, "dst_pa=%lX\n", (ulong)dst_pa);
	//direction
	writew(REG_VAL_DAM_TO_DRAM, REG_BDMA_OFS_DIRECTION);

	clrbits_le16(REG_BDMA_CFG, reg_val_src_a_h | reg_val_dst_a_h);
	if (src_pa >= BUFF_BOUNDARY) {
		data = ((src_pa >> bdma_pa_32_shift) << reg_val_src_a_h_shift) & reg_val_src_a_h;
		setbits_le16(REG_BDMA_CFG, data);
		src_pa -= BUFF_BOUNDARY;
	}

	if (dst_pa >= BUFF_BOUNDARY) {
		data = ((dst_pa >> bdma_pa_32_shift) << reg_val_dst_a_h_shift) & reg_val_dst_a_h;
		setbits_le16(REG_BDMA_CFG, data);
		dst_pa -= BUFF_BOUNDARY;
	}

	//src_addr
	writew((u16)(src_pa >>  0), REG_BDMA_OFS_SRC_L);
	writew((u16)(src_pa >> bdma_pa_16_shift), REG_BDMA_OFS_SRC_H);

	//dst_addr
	writew((u16)(dst_pa >>  0), REG_BDMA_OFS_DST_L);
	writew((u16)(dst_pa >> bdma_pa_16_shift), REG_BDMA_OFS_DST_H);

	//data_size
	writew((u16)(bufsize >>  0), REG_BDMA_OFS_SZ_L);
	writew((u16)(bufsize >> bdma_pa_16_shift), REG_BDMA_OFS_SZ_H);

	//tirgger dma
	writeb(reg_vl_triger, REG_BDMA_CFG);
	//check done
	do {
		udelay(BDMA_DELAY);
		cnt++;

		if ((BDMA_DELAY * cnt) >= BDMA_TIMEOUT) {
			emmc_debug(0, 0, "eMMC Err: wait dma %d timeout.\n", (BDMA_TIMEOUT));
			break;
		}
	} while (readb(REG_BDMA_CFG) & reg_vl_triger);
}

#endif

static void mtk_fcie_prepare_data_dma_registers(struct mtk_fcie_host *host,
						struct mmc_cmd *cmd,
						struct mmc_data *data,
						u32 *waited_time)
{
	u32 blocks = 0;
	dmaaddr_t dmaaddr;

	blocks = data->blocks;
	writew(blocks, &host->base->fcie_job_bl_cnt);
	writew(0x200, &host->base->fcie_blk_size);

	writew((blocks << 9) & 0xFFFF, &host->base->fcie_miu_dma_len_15_0);
	writew((blocks << 9) >> 16, &host->base->fcie_miu_dma_len_31_16);

	if (data->flags == MMC_DATA_READ) {
		dmaaddr = mtk_fcie_dma_address_trans((dmaaddr_t)data->dest, blocks << 9);
		#ifdef ENABLE_FCIE_MIU_CHECKSUM
		if ((emmc_drv.u8_partition_config & PART_ACCESS_MASK) == 0)
			emmc_drv.host->start_blk = cmd->cmdarg;
		#endif
		if (host->no_fde == 0 && host->fde_enc) {
			fde_aes_func_open(ENABLE_DEC, cmd->cmdarg);
			if ((unsigned long)data->dest & (FDE_CACHE_LINE - 1)) {
				emmc_debug(0, 1, "eMMC Err: R, buffer misaligned 16-bytes: %lXh\n",
					   (unsigned long)data->dest);
			}
		}
	} else {
		dmaaddr = mtk_fcie_dma_address_trans((dmaaddr_t)data->src, blocks << 9);
		if (host->no_fde == 0 && host->fde_enc) {
			fde_aes_func_open(ENABLE_ENC, cmd->cmdarg);
			if ((unsigned long)data->src & (FDE_CACHE_LINE - 1)) {
				emmc_debug(0, 1, "eMMC Err: W, buffer misaligned 16-bytes: %lXh\n",
					   (unsigned long)data->src);
			}
		}
	}
	writew(dmaaddr & 0xFFFF, &host->base->fcie_miu_dma_addr_15_0);
	writew((dmaaddr >> 16) & 0xFFFF, &host->base->fcie_miu_dma_addr_31_16);
	if (sizeof(dmaaddr_t) == 8)
		writew(dmaaddr >> 32, &host->base->fcie_miu_dma_addr_47_32);
	*waited_time = READ_TIMEOUT;

	#ifdef ENABLE_FCIE_MIU_CHECKSUM
	mtk_fcie_clear_miu_chksum(host);
	#endif
}

static void mtk_fcie_prepare_m6e3_skew_register(struct mmc_data *data)
{
	if (emmc_drv.u8_emmc_pll_skew4 && emmc_drv.t_hs400_table.u8_set_cnt &&
	    emmc_drv.pad_type == FCIE_EMMC_HS400) {
		if (data) {
			if (data->flags == MMC_DATA_READ)
				emmc_fcie_set_skew4_data_reg();
			else
				emmc_fcie_set_skew4_rsp_reg();
		} else {
			emmc_fcie_set_skew4_rsp_reg();
		}
	}
}

static int mtk_fcie_start_command(struct mtk_fcie_host *host, struct mmc_cmd *cmd,
			      struct mmc_data *data)
{
	u32 wanted_event = 0, waited_time;
	int ret;
	u8 u8_cmd_log_en = emmc_drv.u8_cmd_log_en;


	if (u8_cmd_log_en)
		emmc_debug(0, 0, "\ncmd %d, arg %Xh\n", cmd->cmdidx, cmd->cmdarg);

	if (emmc_drv.u8_if_switch_hs200_to_high) {
		if (emmc_drv.pad_type != FCIE_EMMC_BYPASS) {
			emmc_pads_switch(FCIE_EMMC_BYPASS);
			emmc_clock_setting(BIT_FCIE_CLK_48M);
		}
		emmc_drv.u8_if_switch_hs200_to_high = 0;
	}

	mtk_fcie_part_config(host, cmd);

	if (data) {
		if ((emmc_drv.u8_partition_config & PART_ACCESS_MASK) == 0 &&
		    (cmd->cmdidx == MMC_CMD_WRITE_SINGLE_BLOCK ||
		    cmd->cmdidx == MMC_CMD_WRITE_MULTIPLE_BLOCK ||
		    cmd->cmdidx == MMC_CMD_READ_SINGLE_BLOCK ||
		    cmd->cmdidx == MMC_CMD_READ_MULTIPLE_BLOCK))
			host->fde_enc = mtk_fcie_get_fde(host, cmd->cmdarg);
		else
			host->fde_enc = 0;
		//emmc_debug(0, 1, "%s\n", (host->fde_enc == 1) ? "enc":"noenc");
	}

	if (host->last_data_write || host->last_resp_type) {
		if (!mtk_fcie_mmc_is_ready(host))
			return -EIO;
		host->last_data_write = 0;
		host->last_resp_type = 0;
	}

	mtk_fcie_prepare_m6e3_skew_register(data);

	wanted_event = mtk_fcie_cmd_prepare_raw_cmd(host, cmd, data);

	//prepare data dma address register
	waited_time = 1000000;
	if (data)
		mtk_fcie_prepare_data_dma_registers(host, cmd, data, &waited_time);

	//emmc_debug(0, 0, "\n");
	setbits_le16(&host->base->fcie_sd_ctrl, BIT_JOB_START);

	ret = emmc_fcie_wait_events((unsigned long)(&host->base->fcie_mie_event), wanted_event, waited_time);

	if (ret) {
		emmc_debug(0, 1, "eMMC Err: cmd %d, arg %Xh ", cmd->cmdidx, cmd->cmdarg);
		emmc_debug(0, 1, "TO event %Xh, wanted %Xh\n", readw(&host->base->fcie_mie_event), wanted_event);
		ret = -ETIMEDOUT;
		if (data)
			emmc_debug(0, 0, "eMMC Err: blkcnt %d, %s %lXh",
				   data->blocks, (data->flags == MMC_DATA_WRITE) ? "src" : "dest",
				   (data->flags == MMC_DATA_WRITE) ? (unsigned long)data->src :
				   (unsigned long)data->dest);

		goto error;
	}

	return mtk_fcie_cmd_done(host, cmd);
error:
	emmc_fcie_dump_registers();
	if (host->no_fde == 0 && host->fde_enc)
		fde_aes_func_close();
	mtk_fcie_reset_hw(host);
	return ret;
}

static int mtk_fcie_need_to_rescan(void)
{
	if ((emmc_drv.pad_type == FCIE_EMMC_HS400 &&  emmc_drv.t_hs400_table.u8_set_cnt) ||
	    (emmc_drv.pad_type == FCIE_EMMC_HS400_5_1 && emmc_drv.t_table_g.u8_set_cnt))
		return 1;

	return 0;
}

static int mtk_fcie_start_data(struct mtk_fcie_host *host, struct mmc_data *data)
{
	u16 status;
	int ret;
	unsigned long long time_wait_n_block_end_temp = 0;
	u32 time_wait_n_block_end = 0;
	#ifdef ENABLE_FCIE_MIU_CHECKSUM
	u32 err;
	#endif

	if (data->flags == MMC_DATA_WRITE) {
		if (!mtk_fcie_mmc_is_ready(host)) {
			ret = -EIO;
			goto error;
		}

		emmc_skew4_write_data_reg();
		writew(BIT_SD_DTRX_EN|BIT_SD_DAT_DIR_W, &host->base->fcie_sd_ctrl);
		setbits_le16(&host->base->fcie_sd_ctrl, BIT_JOB_START);
		time_wait_n_block_end_temp = (unsigned long long)data->blocks * HW_TIMER_DELAY_1S;
		if (time_wait_n_block_end_temp > MAX_TIME_WAIT_N_BLK_END_MASK)
			time_wait_n_block_end = MAX_TIME_WAIT_N_BLK_END_MASK;
		else
			time_wait_n_block_end = data->blocks * HW_TIMER_DELAY_1S;

		if (data->blocks == 1)
			time_wait_n_block_end = WRITE_SINGLE_BLK_TIMEOUT;

		ret = emmc_fcie_wait_events((unsigned long)(&host->base->fcie_mie_event),
					    BIT_DMA_END, time_wait_n_block_end);
		if(ret) {
			emmc_debug(0, 1, "eMMC Err: W, TO event %Xh, wanted %Xh\n",
				   readw(&host->base->fcie_mie_event), BIT_DMA_END);
			ret = -ETIMEDOUT;
			goto error;
		}
		status = readw(&host->base->fcie_sd_status);
		if (status & BIT_DAT_WR_CERR) {
			emmc_debug(0, 1, "eMMC Err: Write crc error %Xh\n", status);
			ret = -EIO;
			if (emmc_drv.u8_emmc_pll_skew4) {
				if (mtk_fcie_need_to_rescan()) {
					emmc_m6e3_rescan_timing();
					if (host->no_fde == 0 && host->fde_enc)
						fde_aes_func_close();

					return ret;
				}
			}
			goto error;
		}

		host->last_data_write = 1;
	}
	else { //read invalidate cache
		#ifdef ENABLE_FCIE_MIU_CHECKSUM
		emmc_drv.host->blks = data->blocks;
		emmc_drv.host->dst_buf = (char *)data->dest;
		#endif
		mtk_fcie_dma_address_trans_post((dmaaddr_t)data->dest, data->blocks << 9);
		#ifdef ENABLE_FCIE_MIU_CHECKSUM
		if ((emmc_drv.u8_partition_config & PART_ACCESS_MASK) == 0) {
			err = mtk_fcie_miu_chksum(host, (u8 *)data->dest, data->blocks << EMMC_SECTOR_512BYTE_BITS);
			if (err)
				mtk_fcie_dump_dst();
		}
		#endif
	}
	if (host->no_fde == 0 && host->fde_enc)
		fde_aes_func_close();

	//clr all event
	writew(readl(&host->base->fcie_mie_event), &host->base->fcie_mie_event);

	return 0;
error:
	emmc_fcie_dump_registers();
	if (host->no_fde == 0 && host->fde_enc)
		fde_aes_func_close();
	mtk_fcie_reset_hw(host);
	return ret;
}

static void mtk_fcie_get_device_capacity(struct mtk_fcie_host *host, struct mmc_data *data, U8 *pbuf)
{
	host->sec_count = ((data->dest[EXT_CSD_SEC_CNT_24_31] << 24) |
		(data->dest[EXT_CSD_SEC_CNT_16_23] << 16) |
		(data->dest[EXT_CSD_SEC_CNT_7_15] << 8) |
		(data->dest[EXT_CSD_SEC_CNT]));

	if (pbuf[EXT_CSD_BOOT_BUS_WIDTH] != 0x2 && (pbuf[EXT_CSD_PART_CONF] >> 3) != 0x1 &&
	    (pbuf[EXT_CSD_PART_CONF] >> 3) != 0x2) {
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
			   "eMMC Err: eMMC boot bus condition = %xh, eMMC boot config = %xh\n",
			   pbuf[EXT_CSD_BOOT_BUS_WIDTH],
			   pbuf[EXT_CSD_PART_CONF]);
	}

	//--------------------------------
	if (emmc_drv.u32_sec_count == 0)
	    emmc_drv.u32_sec_count = ((pbuf[EXT_CSD_SEC_CNT_24_31] << 24) | (pbuf[EXT_CSD_SEC_CNT_16_23] << 16) |
		(pbuf[EXT_CSD_SEC_CNT_7_15] << 8) | (pbuf[EXT_CSD_SEC_CNT]));

	//-------------------------------
	if (emmc_drv.u32_boot_sec_count == 0)
		emmc_drv.u32_boot_sec_count = (pbuf[EXT_CSD_BOOT_MULT] * 128) * 2;
}

static void mtk_fcie_get_device_gpt(struct mtk_fcie_host *host, u8 *pbuf)
{
	//for GP Partition
	emmc_drv.u8_ecsd160_part_supfield = pbuf[EXT_CSD_PARTITIONING_SUPPORT];
	emmc_drv.u8_ecsd224_hc_erase_grp_size = pbuf[EXT_CSD_HC_ERASE_GRP_SIZE];
	emmc_drv.u8_ecsd221_hc_wp_grp_size = pbuf[EXT_CSD_HC_WP_GRP_SIZE];

	emmc_drv.gp_part[0].u32_part_size = ((pbuf[EXT_CSD_GP_SIZE_MULT_1_2] << 16) |
		(pbuf[EXT_CSD_GP_SIZE_MULT_1_1] << 8) |
		(pbuf[EXT_CSD_GP_SIZE_MULT_1_0])) *
		(emmc_drv.u8_ecsd224_hc_erase_grp_size  *
		emmc_drv.u8_ecsd221_hc_wp_grp_size *
		0x80000);

	emmc_drv.gp_part[1].u32_part_size = ((pbuf[EXT_CSD_GP_SIZE_MULT_2_2] << 16) |
		(pbuf[EXT_CSD_GP_SIZE_MULT_2_1] << 8) |
		(pbuf[EXT_CSD_GP_SIZE_MULT_2_0])) *
		(emmc_drv.u8_ecsd224_hc_erase_grp_size  *
		emmc_drv.u8_ecsd221_hc_wp_grp_size *
		0x80000);

	emmc_drv.gp_part[2].u32_part_size = ((pbuf[EXT_CSD_GP_SIZE_MULT_3_2] << 16) |
		(pbuf[EXT_CSD_GP_SIZE_MULT_3_1] << 8) |
		(pbuf[EXT_CSD_GP_SIZE_MULT_3_0])) *
		(emmc_drv.u8_ecsd224_hc_erase_grp_size  *
		emmc_drv.u8_ecsd221_hc_wp_grp_size *
		0x80000);

	emmc_drv.gp_part[3].u32_part_size = ((pbuf[EXT_CSD_GP_SIZE_MULT_4_2] << 16) |
		(pbuf[EXT_CSD_GP_SIZE_MULT_4_1] << 8) |
		(pbuf[EXT_CSD_GP_SIZE_MULT_4_0])) *
		(emmc_drv.u8_ecsd224_hc_erase_grp_size  *
		emmc_drv.u8_ecsd221_hc_wp_grp_size *
		0x80000);
}

static void mtk_fcie_get_device_inf(struct mtk_fcie_host *host, U8 *pbuf)
{
	if (!emmc_drv.u8_bus_width) {
	    emmc_drv.u8_bus_width = pbuf[EXT_CSD_BUS_WIDTH];

		switch (emmc_drv.u8_bus_width) {
		case 0:
			emmc_drv.u8_bus_width = BIT_SD_DATA_WIDTH_1;
		break;
		case 1:
			emmc_drv.u8_bus_width = BIT_SD_DATA_WIDTH_4;
		break;
		case 2:
			emmc_drv.u8_bus_width = BIT_SD_DATA_WIDTH_8;
		break;
		default:
			emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: eMMC BUS_WIDTH not support\n");
		}
	}

	//--------------------------------
	if (pbuf[EXT_SECURE_FEATURE_SUPPORT] & BIT4)//TRIM
		emmc_drv.u32_emmc_flag |= EMMC_FLAG_TRIM;
	else
		emmc_drv.u32_emmc_flag &= ~EMMC_FLAG_TRIM;

	//--------------------------------
	if (pbuf[EXT_CSD_HPI_FEATURE] & BIT0) {
		if (pbuf[EXT_CSD_HPI_FEATURE] & BIT1)
			emmc_drv.u32_emmc_flag |= EMMC_FLAG_HPI_CMD12;
		else
			emmc_drv.u32_emmc_flag |= EMMC_FLAG_HPI_CMD13;
	} else {
		emmc_drv.u32_emmc_flag &= ~(EMMC_FLAG_HPI_CMD12 | EMMC_FLAG_HPI_CMD13);
	}
	//--------------------------------
	if (pbuf[EXT_CSD_WR_REL_PARAM] & BIT2) {
		emmc_drv.u16_reliable_w_blk_cnt = BIT_SD_JOB_BLK_CNT_MASK;
	} else {
		if ((pbuf[EXT_CSD_HPI_FEATURE] & BIT0) && pbuf[EXT_CSD_REL_WR_SEC_C] == 1)
			emmc_drv.u16_reliable_w_blk_cnt = 1;
		else if ((pbuf[EXT_CSD_HPI_FEATURE] & BIT0) == 0)
			emmc_drv.u16_reliable_w_blk_cnt = pbuf[EXT_CSD_REL_WR_SEC_C];
		else
			emmc_drv.u16_reliable_w_blk_cnt = 0;//can not support Reliable Write
	}

	//--------------------------------
	emmc_drv.u8_erased_mem_content = pbuf[EXT_CSD_ERASEED_MEM_CONTENT];
	//--------------------------------

	emmc_drv.u8_ecsd184_stroe_support = pbuf[EXT_CSD_STROBE_SUPPORT];
	emmc_drv.u8_ecsd185_hs_timing = pbuf[EXT_CSD_HS_TIMING];
	emmc_drv.u8_ecsd192_ver = pbuf[EXT_CSD_REV];
	emmc_drv.u8_ecsd196_dev_type = pbuf[EXT_CSD_CARD_TYPE];
	emmc_drv.u8_ecsd197_driver_strength = pbuf[EXT_CSD_DRV_STRENGTH];
	emmc_drv.u8_ecsd248_cmd6_to = pbuf[EXT_CSD_GENERIC_CMD6_TIME];
	emmc_drv.u8_ecsd247_pwr_off_long_to = pbuf[EXT_CSD_POWER_OFF_LONG_TIME];
	emmc_drv.u8_ecsd34_pwr_off_ctrl = pbuf[EXT_CSD_POWER_OFF_NOTIFICATION];
}

static void mtk_fcie_get_max_en_size(struct mtk_fcie_host *host, U8 *pbuf)
{
	emmc_drv.u8_partition_config = pbuf[EXT_CSD_PART_CONF];

	//for Max Enhance Size
	emmc_drv.u8_ecsd157_max_enh_size_0 = pbuf[EXT_CSD_MAX_ENH_SIZE_MULT];
	emmc_drv.u8_ecsd158_max_enh_size_1 = pbuf[EXT_CSD_MAX_ENH_SIZE_MULT_1];
	emmc_drv.u8_ecsd159_max_enh_size_2 = pbuf[EXT_CSD_MAX_ENH_SIZE_MULT_2];
	emmc_drv.u8_u8_ecsd155_part_set_complete = pbuf[EXT_CSD_PARTITIONS_ATTRIBUTE];
	emmc_drv.u8_ecsd166_wr_rel_param = pbuf[EXT_CSD_PARTITIONS_ATTRIBUTE];
}

static u32 mtk_fcie_config_ecsd_hs400_5_1(struct mtk_fcie_host *host)
{
	u32 err = 0;

	if (emmc_drv.u8_ecsd196_dev_type & EMMC_DEVTYPE_HS400_1_8V &&
	    (host->mmc->cfg->host_caps & MMC_CAP(MMC_HS_400_ES)) &&
	    emmc_drv.u8_ecsd184_stroe_support > 0 &&
	    emmc_drv.t_table_g.u8_set_cnt == 0) {
		err = emmc_load_timing_table(FCIE_EMMC_HS400_5_1);
		if (err != EMMC_ST_SUCCESS)
			emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Warn: HS400 5.1 no Timing Table, %Xh\n", err);
	}

	return  err;
}

static u32 mtk_fcie_config_ecsd_hs400(struct mtk_fcie_host *host)
{
	u32 err = 0;

	if (emmc_drv.u8_ecsd196_dev_type & EMMC_DEVTYPE_HS400_1_8V &&
	    (host->mmc->cfg->host_caps & MMC_CAP(MMC_HS_400)) &&
	    emmc_drv.t_table_g.u8_set_cnt == 0 &&
	    emmc_drv.t_hs400_table.u8_set_cnt == 0) {
		if (emmc_drv.u8_emmc_pll_skew4) {
			err = emmc_load_timing_table(FCIE_EMMC_HS200);
			if (err != EMMC_ST_SUCCESS)
				emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
					   "eMMC Warn: HS200 no Timing Table, %Xh\n", err);
		}
		err = emmc_load_timing_table(FCIE_EMMC_HS400);

		if (err != EMMC_ST_SUCCESS)
			emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Warn: HS400 no Timing Table, %Xh\n", err);
	}

	return  err;
}

static void mtk_fcie_config_ecsd(struct mtk_fcie_host *host, struct mmc_data *data)
{
	u32 err;

	if (!host->sec_count) {
		host->ecsd_196 = data->dest[EXT_CSD_CARD_TYPE];

		mtk_fcie_get_device_capacity(host, data, (u8 *)data->dest);
		//--------------------------------
		mtk_fcie_get_device_inf(host, (u8 *)data->dest);
		mtk_fcie_get_device_gpt(host, (u8 *)data->dest);
		mtk_fcie_get_max_en_size(host, (u8 *)data->dest);

		emmc_drv.u32_drvflag |= DRV_FLAG_INIT_DONE;

		#if (defined(ENABLE_EMMC_HS400_5_1) && ENABLE_EMMC_HS400_5_1)
		err = mtk_fcie_config_ecsd_hs400_5_1(host);
		if (err != EMMC_ST_SUCCESS)
			return;
		#endif

		#if (defined(ENABLE_EMMC_HS400) && ENABLE_EMMC_HS400)
		err = mtk_fcie_config_ecsd_hs400(host);
		if (err != EMMC_ST_SUCCESS)
			return;
		#endif

		#if defined(ENABLE_EMMC_HS200) && ENABLE_EMMC_HS200
		if (emmc_drv.u8_ecsd196_dev_type & EMMC_DEVTYPE_HS200_1_8V &&
		    (host->mmc->cfg->host_caps & MMC_CAP(MMC_HS_200)) &&
		    !emmc_drv.t_table.u8_set_cnt) {
			err = emmc_load_timing_table(FCIE_EMMC_HS200);
			if (err != EMMC_ST_SUCCESS)
				emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Warn: HS200 no Timing Table, %Xh\n", err);
			return ;
		}
		#endif
		
		#if defined(ENABLE_EMMC_ATOP) && ENABLE_EMMC_ATOP
		if (emmc_drv.u8_ecsd196_dev_type & EMMC_DEVTYPE_DDR &&
		    (host->mmc->cfg->host_caps & MMC_CAP(MMC_DDR_52)) &&
		    !emmc_drv.t_table.u8_set_cnt) {
			err = emmc_load_timing_table(FCIE_EMMC_DDR);
			if (err != EMMC_ST_SUCCESS)
				emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Warn: DDR no Timing Table, %Xh\n", err);
			return ;
		}
		#endif
	} else {
		emmc_drv.u8_partition_config = data->dest[179];
	}
}

//0: Do not retry,  1: LABEL_RW_MULTI_BLK_START, 2: LABEL_CMD_START
static int mtk_fcie_ops_cmd_retry_m6e3(struct mmc_cmd *cmd, int *retry_cnt)
{
	if (emmc_drv.u8_emmc_pll_skew4 && (emmc_drv.u8_partition_config & PART_ACCESS_MASK) != MMC_PART_RPMB) {
		if ((emmc_drv.pad_type == FCIE_EMMC_HS400_5_1 && emmc_drv.t_hs400_table.u8_set_cnt) ||
		    (emmc_drv.pad_type == FCIE_EMMC_HS400 && emmc_drv.t_table_g.u8_set_cnt)) {
			if (cmd->cmdidx == MMC_CMD_READ_MULTIPLE_BLOCK ||
			    cmd->cmdidx == MMC_CMD_WRITE_MULTIPLE_BLOCK) {
				if ((*retry_cnt) < EMMC_MAX_RETRY_CNT) {
					(*retry_cnt)++;
					return 1;
				}
			} else {
				if ((*retry_cnt) < EMMC_MAX_RETRY_CNT) {
					(*retry_cnt)++;
					return 2;
				}
			}
		}
	}

	return 0;
}

//0: Do not retry,  1: LABEL_RW_MULTI_BLK_START, 2: LABEL_CMD_START
static int mtk_fcie_ops_data_retry_m6e3(struct mmc_cmd *cmd, int *retry_cnt)
{
	if (emmc_drv.u8_emmc_pll_skew4 && (emmc_drv.u8_partition_config & PART_ACCESS_MASK) != MMC_PART_RPMB) {
		if ((emmc_drv.pad_type == FCIE_EMMC_HS400_5_1 && emmc_drv.t_hs400_table.u8_set_cnt) ||
		    (emmc_drv.pad_type == FCIE_EMMC_HS400 && emmc_drv.t_table_g.u8_set_cnt)) {
			if (cmd->cmdidx == MMC_CMD_READ_MULTIPLE_BLOCK || cmd->cmdidx == MMC_CMD_WRITE_MULTIPLE_BLOCK) {
				if ((*retry_cnt) < EMMC_MAX_RETRY_CNT) {
					(*retry_cnt) += 1;
					return 1;
				}
			} else {
				if ((*retry_cnt) < EMMC_MAX_RETRY_CNT) {
					(*retry_cnt) += 1;
					return 2;
				}
			}
		}
	}

	return 0;
}

static int mtk_fcie_ops_send_cmd(struct udevice *dev, struct mmc_cmd *cmd, struct mmc_data *data)
{
	struct mtk_fcie_host *host = dev_get_priv(dev);
	int ret, err;
	int retry_cnt = 0;
	struct mmc_cmd cmd23;


    
	if ((cmd->cmdidx == MMC_CMD_SEND_EXT_CSD && cmd->cmdarg > 0) || cmd->cmdidx == MMC_CMD_APP_CMD)
		return -ETIMEDOUT;

LABEL_RW_MULTI_BLK_START:
	if (retry_cnt) {
		cmd23.cmdidx = MMC_CMD_SET_BLOCK_COUNT;
		cmd23.cmdarg = data->blocks;
		cmd23.cmdarg |= MMC_RELIABLE_WRITE_ARG;
		cmd23.resp_type = MMC_RSP_R1;

		ret = mtk_fcie_start_command(host, &cmd23, NULL);
		if (ret)
		    return ret;
	}
LABEL_CMD_START:

	ret = mtk_fcie_start_command(host, cmd, data);
	if (ret) {
		err = mtk_fcie_ops_cmd_retry_m6e3(cmd, &retry_cnt);
		if (err == 1)
			goto LABEL_RW_MULTI_BLK_START;
		else if (err == 2)
			goto LABEL_CMD_START;
		return ret;
	}
	if (data) {
		ret = mtk_fcie_start_data(host, data);
		//Only write command error
		if (ret) { 
			err = mtk_fcie_ops_data_retry_m6e3(cmd, &retry_cnt);
			if (err == 1)
				goto LABEL_RW_MULTI_BLK_START;
			else if (err == 2)
				goto LABEL_CMD_START;
			else
				return ret;
		}

		if (!ret && cmd->cmdidx == MMC_CMD_SEND_EXT_CSD)
			mtk_fcie_config_ecsd(host, data);
	}
	return ret;
}

static void mtk_fcie_set_buswidth(struct mtk_fcie_host *host, u32 width)
{
	host->host_sd_mode &= ~BIT_SD_DATA_WIDTH_MASK;
	switch (width) {
	default:
	case 1:
		emmc_drv.u8_bus_width = BIT_SD_DATA_WIDTH_1;
	break;
	case 4:
		emmc_drv.u8_bus_width = BIT_SD_DATA_WIDTH_4;
		host->host_sd_mode |= BIT_SD_DATA_WIDTH_4;
	break;
	case 8:
		emmc_drv.u8_bus_width = BIT_SD_DATA_WIDTH_8;
		host->host_sd_mode |= BIT_SD_DATA_WIDTH_8;
	break;
	}
	emmc_drv.u16_mmc_mode = host->host_sd_mode;
}

static int mtk_fcie_set_voltage(struct mtk_fcie_host *host, enum mmc_voltage voltage)
{
	//only set to 1.8V
	if (voltage == MMC_SIGNAL_VOLTAGE_180) {
		if (!(readw(&host->emmcpllbase->emmcpll_rx[0x1A]) & BIT(0))) {
			//emmc_debug(0, 0, "eMMC Err: not 1.8V IO setting\n");
			setbits_le16(&host->emmcpllbase->emmcpll_rx[0x1A], BIT(0) | BIT(2));//1.8V must set this bit & atop patch
			clrbits_le16(&host->emmcpllbase->emmcpll_rx[0x45], 0xfff);//host driving
			setbits_le16(&host->emmcpllbase->emmcpll_rx[0x45], host->host_driving);//host driving
		}
	} else if (voltage == MMC_SIGNAL_VOLTAGE_330) {
		return -EINVAL;
	} else {
		return -EINVAL;
	}

	host->voltage = voltage;
	return 0;
}

static void mtk_fcie_ops_set_ios_hs400(struct mmc *mmc)
{
	u8 cur_set_idx = 0;

	if (mmc->selected_mode == MMC_HS_400)
		emmc_pads_switch(FCIE_EMMC_HS400);
	#if CONFIG_IS_ENABLED(MMC_HS400_ES_SUPPORT)
	else
		emmc_pads_switch(FCIE_EMMC_HS400_5_1);
	#endif

	if (emmc_drv.t_table_g.u8_set_cnt || emmc_drv.t_hs400_table.u8_set_cnt) {
		if (emmc_drv.u8_emmc_pll_skew4 && emmc_drv.pad_type == FCIE_EMMC_HS400)
			cur_set_idx = EMMC_TIMING_SET_MAX;
		else
			cur_set_idx = emmc_drv.t_table_g.u8_cur_set_idx;

		emmc_fcie_apply_timing_set(cur_set_idx);
	}
}

static int mtk_fcie_ops_set_ios(struct udevice *dev)
{
	struct mtk_fcie_plat *plat = dev_get_platdata(dev);
	struct mtk_fcie_host *host = dev_get_priv(dev);
	struct mmc *mmc = &plat->mmc;
	uint clock = mmc->clock;
	int ret;

	//emmc_debug(0, 1, "voltage %d, bus-width %d, timing %d, clock %d\n",
		   //mmc->signal_voltage, mmc->bus_width, mmc->selected_mode, mmc->clock);
	if (host->voltage != mmc->signal_voltage) {
		ret = mtk_fcie_set_voltage(host, mmc->signal_voltage);
		if (ret)
			return ret;
	}

	mtk_fcie_set_buswidth(host, mmc->bus_width);

	//handle clock and pads
	if (mmc->clk_disable)// clock gating
		clock = 0;
	else if (clock < mmc->cfg->f_min)
		clock = mmc->cfg->f_min;

	if (host->timing != mmc->selected_mode || host->host_clk != clock) {
		if (mmc->selected_mode == MMC_LEGACY) {
			emmc_pads_switch(FCIE_EMMC_BYPASS);
			if(clock != mmc->cfg->f_min) {
				clock = 12000000;
				emmc_clock_setting(FCIE_SLOW_CLK);
			}
			else
				emmc_clock_setting(FCIE_SLOWEST_CLK);
		} else if (mmc->selected_mode == MMC_HS || mmc->selected_mode == MMC_HS_52) {
			emmc_pads_switch(FCIE_EMMC_BYPASS);
			emmc_clock_setting(BIT_FCIE_CLK_48M);
		} else if (mmc->selected_mode == MMC_DDR_52) {
			emmc_pads_switch(FCIE_EMMC_DDR);
			if (emmc_drv.t_table.u8_set_cnt)
				emmc_fcie_apply_timing_set(EMMC_TIMING_SET_MAX);
			else//no ddr table
				emmc_fcie_build_ddr_timing_table();
		} else if (mmc->selected_mode == MMC_HS_200) {
			emmc_pads_switch(FCIE_EMMC_HS200);
			if (emmc_drv.t_table.u8_set_cnt)
				emmc_fcie_apply_timing_set(EMMC_TIMING_SET_MAX);
		#if CONFIG_IS_ENABLED(MMC_HS400_ES_SUPPORT)
		} else if (mmc->selected_mode == MMC_HS_400 || mmc->selected_mode == MMC_HS_400_ES) {
		#else
		} else if (mmc->selected_mode == MMC_HS_400) {
		#endif
			mtk_fcie_ops_set_ios_hs400(mmc);
		}
		host->timing = mmc->selected_mode;
		host->host_clk = clock;
	}

	return 0;
}

#ifdef MMC_SUPPORTS_TUNING
static int mtk_fcie_execute_tuning(struct udevice *dev, uint opcode)
{
	//struct mtk_fcie_plat *plat = dev_get_platdata(dev);
	struct mtk_fcie_host *host = dev_get_priv(dev);
	//struct mmc *mmc = &plat->mmc;
	int ret = 0;

	if (host->ecsd_196 & EMMC_DEVTYPE_HS200_1_8V && emmc_drv.t_table.u8_set_cnt == 0) {
		if (host->ecsd_196 & EMMC_DEVTYPE_HS400_1_8V && host->mmc->cfg->host_caps & MMC_CAP(MMC_HS_400) &&
		    emmc_drv.u8_emmc_pll_skew4 == 0)
			emmc_drv.u32_drvflag |= DRV_FLAG_SPEED_HS400_DETECT;

		ret = emmc_fcie_build_hS200_timing_table();
		if (ret != EMMC_ST_SUCCESS) {
			emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
				   "eMMC Err: eMMC_FCIE_BuildHS200Timing fail: %Xh\n", ret);
			return ret;
		}
		if (host->ecsd_196 & EMMC_DEVTYPE_HS400_1_8V && host->mmc->cfg->host_caps & MMC_CAP(MMC_HS_400)) {
			//go to HS400 tuning
			if (emmc_drv.u8_emmc_pll_skew4 > 0)
				ret = emmc_fcie_detect_hs400_skew4_timing();
			else
				ret = emmc_fcie_detect_hs400_timing();

			if (ret != EMMC_ST_SUCCESS)
				emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
					   "eMMC Err: emmc_fcie_detect_hs400_timing fail: %Xh\n", ret);

			emmc_fcie_reset_to_hs200(FCIE_DEFAULT_CLK);
		}
	}

	#if defined(EN_POWER_CUT_AUTO_TEST) && EN_POWER_CUT_AUTO_TEST
	emmc_test_pwr_cut_test((U8 *)UPGRADE_BUFFER_ADDR, (U32)PWR_CUT_TEST_START_BLK);
	#endif

	return ret;
}
#endif
#if CONFIG_IS_ENABLED(MMC_HS400_ES_SUPPORT)
static int mtk_fcie_set_enhanced_strobe(struct udevice *dev)
{
	int ret = 0;

	if (!emmc_drv.t_table_g.u8_set_cnt) {
		if (emmc_drv.u8_emmc_pll_skew4)
			ret = emmc_fcie_detect_hs400_5_1_skew4_timing();
		else
			ret = emmc_fcie_detect_hs400_5_1_timing();
	} else {
		emmc_fcie_apply_timing_set(emmc_drv.t_table_g.u8_cur_set_idx);
	}

	#if defined(EN_POWER_CUT_AUTO_TEST) && EN_POWER_CUT_AUTO_TEST
	emmc_test_pwr_cut_test((U8 *)UPGRADE_BUFFER_ADDR, (U32)PWR_CUT_TEST_START_BLK);
	#endif

	return ret;
}
#endif

static int mtk_fcie_select_drive_strength(struct udevice *dev, int mode)
{
	struct mtk_fcie_host *host = dev_get_priv(dev);
	if (host->id_matched_driving >= 0 &&
		host->id_matched_driving <= 4)
		return host->id_matched_driving;
	return 0;
}

static void mtk_fcie_init_hw(struct mtk_fcie_host *host)
{
	int err;

	emmc_drv.u16_mmc_mode = BIT_SD_DEFAULT_MODE_REG;
	host->host_sd_mode = BIT_SD_DEFAULT_MODE_REG;
	host->if_sector_mode = 0;

	err = emmc_fcie_init();
	if (err)
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: eMMC_FCIE init fail: %Xh\n", err);
}

static int mtk_fcie_drv_probe(struct udevice *dev)
{
	struct mmc_uclass_priv *upriv = dev_get_uclass_priv(dev);
	struct mtk_fcie_plat *plat = dev_get_platdata(dev);
	struct mtk_fcie_host *host = dev_get_priv(dev);
	struct mmc_config *cfg = &plat->cfg;

	cfg->name = dev->name;
	//emmc_debug(0, 1, "%s\n", cfg->name);

	host->dev_comp = (struct mtk_fcie_compatible *)dev_get_driver_data(dev);

	cfg->f_min = 300*1000;
	cfg->f_max = 200*1000*1000;

	cfg->b_max = BIT_SD_JOB_BLK_CNT_MASK;
	cfg->voltages = MMC_VDD_27_28 | MMC_VDD_28_29 | MMC_VDD_29_30 | MMC_VDD_30_31 | \
                    MMC_VDD_31_32 | MMC_VDD_32_33 | MMC_VDD_33_34 | MMC_VDD_165_195;

	host->mmc = &plat->mmc;
	host->timeout_ns = 100000000;
	host->timeout_clks = 3 * 1048576;

	host->mmc->sectorbuf = (u8 *)memalign(ARCH_DMA_MINALIGN, EMMC_SECTOR_512BYTE);
	mtk_fcie_init_hw(host);

	upriv->mmc = &plat->mmc;

	return 0;
}

static void mtk_fcie_ofdata_to_platdata_cid(struct udevice *dev, struct mtk_fcie_host *host)
{
	int idx;
	const u8 *p;

	if (host->dts_cids_count) {
		host->cid_drivings = malloc(sizeof(struct mtk_fcie_cid_driving) * host->dts_cids_count);
		if (!host->cid_drivings)
			return;
		p = dev_read_u8_array_ptr(dev, "emmc-cids-driving", host->dts_cids_count * 11);

		if (p) {
			for (idx = 0; idx < host->dts_cids_count; idx++) {
				memcpy(host->cid_drivings[idx].cid, p, 10);
				p += 10;
				host->cid_drivings[idx].device_driving = *p++;
				//emmc_dump_mem(host->cid_drivings[idx].cid, 10);
				//printf("device driving %d\n", host->cid_drivings[idx].device_driving);
			}
		}
	}
}

static int mtk_fcie_ofdata_to_platdata_fde_list(struct udevice *dev, struct mtk_fcie_host *host, u32 *val, size_t sz)
{
	int idx;
	struct emmc_crypto_disable_info *info;

	for (idx = 0; idx < sz; idx += 4) {
		info = devm_kzalloc(dev, sizeof(*info), GFP_KERNEL);
		if (!info) {
			kfree(val);
			return -ENOMEM;
		}

		info->lba = (((u64)val[idx] << 32) | (u64)val[idx + 1]) >> 9;
		info->length = (((u64)val[idx + 2] << 32) | (u64)val[idx + 3]) >> 9;
		list_add_tail(&info->list, &host->crypto_white_list);
	}
	return 0;
}

static void mtk_fcie_ofdata_to_platdata_end(struct mtk_fcie_host *host, u8 *platdata_flag)
{
	u32 emmc_drv_offset = 0;

	if ((*platdata_flag) > 0) {
		emmc_drv_offset = (U32)((unsigned long)&emmc_drv.u32_chksum - (unsigned long)&emmc_drv);
		memset((void *)&emmc_drv.u32_chksum, 0, sizeof(struct emmc_driver) - emmc_drv_offset);
	} else {
		memset((void *)&emmc_drv, 0, sizeof(struct emmc_driver));
	}
	emmc_drv.host = host;
	#ifdef ENABLE_FCIE_MIU_CHECKSUM
	mtk_fcie_enable_miu_chksum(host);
	#endif

	#ifdef CONFIG_ROM_OVERLAY
	emmc_drv.chip_id = romtbl_get_chip_id_info_dec();
	emmc_drv.revision = romtbl_get_chip_revision_info();

	if (emmc_drv.chip_id == MT5896_CHIP_ID && emmc_drv.revision == MT5896_CHIP_VER)
		emmc_drv.u8_emmc_pll_skew4 = 1;
	#endif

	*platdata_flag = DRV_PROBE_FLAG;
}

static int mtk_fcie_ofdata_to_platdata(struct udevice *dev)
{
	struct mtk_fcie_plat *plat = dev_get_platdata(dev);
	struct mtk_fcie_host *host = dev_get_priv(dev);
	struct mmc_config *cfg = &plat->cfg;
	size_t sz = 0;
	int len = 0, ret;
	u32 *val;
	static u8 platdata_flag = 0;

	host->base = (void *)dev_read_addr_name(dev, "fcie");
	if (!host->base)
		return -EINVAL;
	host->psmbase= (void *)dev_read_addr_name(dev, "psm");
	if (!host->psmbase)
		return -EINVAL;
	host->emmcpllbase = (void *)dev_read_addr_name(dev, "emmcpll");
	if (!host->emmcpllbase)
		return -EINVAL;

	host->no_clk_framework = 1;

	if (host->no_clk_framework) {
		host->clkgen = (void *)dev_read_addr_name(dev, "clkgen0");
		if (!host->clkgen)
			return -EINVAL;
		host->clkgen2 = (void *)dev_read_addr_name(dev, "clkgen1");
		if (!host->clkgen2)
			return -EINVAL;
	} else { //clk framework
		//read 2xp /1xp
		host->fcieclkreg = (void *)dev_read_addr_name(dev, "clkreg");
		if (!host->fcieclkreg)
			return -EINVAL;

		if(dev_read_u32(dev, "clk-shift", &host->clk_shift))
			host->clk_shift = 0;

		if(dev_read_u32(dev, "clk-mask", &host->clk_mask))
			host->clk_mask = 0;

		if(dev_read_u32(dev, "clk-1xp", &host->clk_1xp))
			host->clk_1xp = 0;

		if(dev_read_u32(dev, "clk-2xp", &host->clk_2xp))
			host->clk_2xp = 0;
	}
	host->riubase = (void *)dev_read_addr_name(dev, "riubase");
	host->no_fde = 1;

	if (dev_read_stringlist_search(dev, "reg-names", "fde") >= 0) {
		host->fde = (void *)dev_read_addr_name(dev, "fde");
		host->no_fde = 0;

		INIT_LIST_HEAD(&host->crypto_white_list);
		if (!dev_read_prop(dev, "crypto-disable-list", &len)) {
			dev_err(dev, "crypto-disable-list not specified\n");
		}
		if (len > 0) {
			sz = len / sizeof(*val);
			val = devm_kcalloc(dev, sz, sizeof(*val), GFP_KERNEL);
			if (!val)
				return -ENOMEM;
			ret = dev_read_u32_array(dev, "crypto-disable-list", val, sz);
			if (ret && (ret != -EINVAL))
				dev_err(dev, "%s: error reading array %d\n", "crypto-disable-list", ret);

			ret = mtk_fcie_ofdata_to_platdata_fde_list(dev, host, val, sz);
			if (ret != 0)
				return ret;
			kfree(val);
		}
	}

	host->fciebase = (void*)(host->base);

	ret = mmc_of_parse(dev, cfg);
	if (ret)
		return ret;

	if (dev_read_u32(dev, "emmc-cids-count", &host->dts_cids_count))
		host->dts_cids_count = 0;

	if (dev_read_u32(dev, "host-driving", &host->host_driving))
		host->host_driving = 0;

	if (romtbl_get_chip_id_info() == MT5897)
		host->host_driving = 0xF00;

	if (host->dts_cids_count)
		mtk_fcie_ofdata_to_platdata_cid(dev, host);

	mtk_fcie_ofdata_to_platdata_end(host, &platdata_flag);
	return 0;
}

static int mtk_fcie_drv_bind(struct udevice *dev)
{
	struct mtk_fcie_plat *plat = dev_get_platdata(dev);

	return mmc_bind(dev, &plat->mmc, &plat->cfg);
}

static const struct dm_mmc_ops mtk_fcie_ops = {
	.send_cmd = mtk_fcie_ops_send_cmd,
	.set_ios = mtk_fcie_ops_set_ios,
#ifdef MMC_SUPPORTS_TUNING
	.execute_tuning = mtk_fcie_execute_tuning,
#endif
	.wait_dat0 = mtk_fcie_ops_wait_dat0,
#if CONFIG_IS_ENABLED(MMC_HS400_ES_SUPPORT)	
	.set_enhanced_strobe = mtk_fcie_set_enhanced_strobe,
#endif
	.select_drive_strength = mtk_fcie_select_drive_strength,
};

static const struct mtk_fcie_compatible mtk_fcie_compat = {
	.analog_skew4 = true,
	.digital_skew4 = false,
	.skip_identify = false,
	.power_saving_mode = false,
	.analog_skew4_reg_offset = 0x6C,
	.analog_skew4_reg_bit = BIT(7),
	.digital_skew4_reg_offset = 0x0,
	.digital_skew4_reg_bit = 0,
	.emmcpllbase_t_tablebase = false,
	.mt5896_clk_layout = false,
};

static const struct mtk_fcie_compatible mtk_mt5888_fcie_compat = {
	.analog_skew4 = true,
	.digital_skew4 = false,
	.skip_identify = false,
	.power_saving_mode = false,
	.analog_skew4_reg_offset = 0x6C,
	.analog_skew4_reg_bit = BIT(7),
	.digital_skew4_reg_offset = 0x0,
	.digital_skew4_reg_bit = 0,
	.emmcpllbase_t_tablebase = false,
	.mt5896_clk_layout = false,
};

static const struct mtk_fcie_compatible mtk_mt5896_fcie_compat = {
	.analog_skew4 = true,
	.digital_skew4 = false,
	.skip_identify = false,
	.power_saving_mode = false,
	.analog_skew4_reg_offset = 0x6C,
	.analog_skew4_reg_bit = BIT(7),
	.digital_skew4_reg_offset = 0x0,
	.digital_skew4_reg_bit = 0,
	.emmcpllbase_t_tablebase = true,
	.mt5896_clk_layout = true,
};

static const struct udevice_id mtk_fcie_ids[] = {
	{ .compatible = "mediatek,mmc-fcie", .data = (ulong)&mtk_fcie_compat },
	{ .compatible = "mediatek,mt5888-mmc-fcie", .data = (ulong)&mtk_mt5888_fcie_compat },
	{ .compatible = "mediatek,mt5896-mmc-fcie", .data = (ulong)&mtk_mt5896_fcie_compat },
	{ .compatible = "mediatek,mt5888-fcie-mmc", .data = (ulong)&mtk_mt5888_fcie_compat },
	{ .compatible = "mediatek,mt5896-fcie-mmc", .data = (ulong)&mtk_mt5896_fcie_compat },
	{}
};

U_BOOT_DRIVER(mtk_fcie_drv) = {
	.name = "mtk_fcie_mmc",
	.id = UCLASS_MMC,
	.of_match = mtk_fcie_ids,
	.ofdata_to_platdata = mtk_fcie_ofdata_to_platdata,
	.bind = mtk_fcie_drv_bind,
	.probe = mtk_fcie_drv_probe,
	.ops = &mtk_fcie_ops,
	.platdata_auto_alloc_size = sizeof(struct mtk_fcie_plat),
	.priv_auto_alloc_size = sizeof(struct mtk_fcie_host),
};
