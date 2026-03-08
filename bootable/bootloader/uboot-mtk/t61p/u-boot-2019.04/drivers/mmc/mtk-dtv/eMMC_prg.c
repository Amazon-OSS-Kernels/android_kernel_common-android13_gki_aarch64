// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (c) 2023 MediaTek Inc.
*/

#include "eMMC.h"


#if defined(UNIFIED_eMMC_DRIVER) && UNIFIED_eMMC_DRIVER

/* frequency bases */
/* divided by 10 to be nice to platforms without floating point */
int fbase[] = {
	10000,
	100000,
	1000000,
	10000000,
};

/* Multiplier values for TRAN_SPEED.  Multiplied by 10 to be nice
* to platforms without floating point.
*/
int multipliers[] = {
	0,
	10,
	12,
	13,
	15,
	20,
	25,
	30,
	35,
	40,
	45,
	50,
	55,
	60,
	70,
	80,
};

//========================================================
U32 emmc_load_images(U32 *pu32_addr, U32 u32_byte_cnt, U32 u32_hash_stage)
{
	U32 u32_err = EMMC_ST_SUCCESS;
	unsigned long dma_addr;
	U16 u16_reg;

	emmc_fcie_clear_events();
	if (u32_hash_stage == 0) {
		emmc_clock_setting(BIT_CLK_XTAL_12M);
		emmc_pads_switch(FCIE_EMMC_BYPASS);
		REG_FCIE_CLRBIT(FCIE_DDR_MODE, BIT_PAD_IN_SEL_SD | BIT_FALL_LATCH);
		REG_FCIE_SETBIT(FCIE_BOOT, BIT_NAND_BOOT_EN);
		REG_FCIE_CLRBIT(FCIE_BOOT, BIT_BOOTSRAM_ACCESS_SEL);
		REG_FCIE_CLRBIT(FCIE_BOOT_CONFIG, BIT_BOOT_MODE_EN);//DMA to DRAM

	u32_err = emmc_fcie_init();
	if (u32_err != EMMC_ST_SUCCESS)
		return u32_err;

	EMMC_RST_L();
	emmc_hw_timer_delay(HW_TIMER_DELAY_5ms);
	EMMC_RST_H();
	emmc_hw_timer_delay(HW_TIMER_DELAY_5ms);
	u32_err = emmc_rom_boot_cmd0(0xFFFFFFFA, (unsigned long)pu32_addr, (U16)(u32_byte_cnt >> 9));
	if (u32_err != EMMC_ST_SUCCESS)
		goto  LABEL_LOAD_IMAGE_END;
	} else {
		REG_FCIE_W(FCIE_JOB_BL_CNT, u32_byte_cnt >> EMMC_SECTOR_512BYTE_BITS);
		dma_addr = emmc_translate_dma_address_ex((unsigned long)pu32_addr, u32_byte_cnt);
		REG_FCIE_W(FCIE_MIU_DMA_ADDR_15_0, dma_addr & 0xFFFF);
		REG_FCIE_W(FCIE_MIU_DMA_ADDR_31_16, dma_addr >> 16);
		if (sizeof(unsigned long) == 8)
			REG_FCIE_W(FCIE_MIU_DMA_ADDR_47_32, dma_addr >> 32);

		REG_FCIE_W(FCIE_MIU_DMA_LEN_15_0, u32_byte_cnt & 0xFFFF);
		REG_FCIE_W(FCIE_MIU_DMA_LEN_31_16, u32_byte_cnt >> 16);
		REG_FCIE_CLRBIT(FCIE_BOOT, BIT_BOOTSRAM_ACCESS_SEL);
		REG_FCIE_CLRBIT(FCIE_BOOT_CONFIG, BIT_BOOT_MODE_EN);//DMA to DRAM
		REG_FCIE_W(FCIE_SD_CTRL, BIT_SD_DTRX_EN);
		REG_FCIE_SETBIT(FCIE_SD_CTRL, BIT_JOB_START);

		REG_FCIE_R(TIMER1_CAP_LOW, u16_reg);
		REG_FCIE_W(FCIE_0X5E, u16_reg);
		REG_FCIE_R(TIMER1_CAP_HIGH, u16_reg);
		REG_FCIE_W(FCIE_0X5F, u16_reg);

		//wait event
		while (1) {
			if (((REG_FCIE(FCIE_MIE_EVENT) & BIT_DMA_END) == BIT_DMA_END) ||
			    (((REG_FCIE(TIMER1_CAP_HIGH) << 16) | REG_FCIE(TIMER1_CAP_LOW)) -
			    ((REG_FCIE(FCIE_0X5F) << 16) | REG_FCIE(FCIE_0X5E)) >= 12000000))
				break;
		}

		if ((REG_FCIE(FCIE_MIE_EVENT) & BIT_DMA_END) != BIT_DMA_END) {
			u32_err = EMMC_ST_ERR_TIMEOUT_WAIT_REG0;
			goto LABEL_LOAD_IMAGE_END;
		}
	}
	//check status
	REG_FCIE_R(FCIE_SD_STATUS, u16_reg);
	if (u16_reg & BIT_SD_FCIE_ERR_FLAGS) {
		u32_err = EMMC_ST_ERR_FCIE_STS_ERR;
		if (u32_err != EMMC_ST_SUCCESS)
			goto LABEL_LOAD_IMAGE_END;
	}

	if (u32_hash_stage == 2) {
		u32_err = emmc_cmd0(0);
		if (u32_err != EMMC_ST_SUCCESS)
			goto LABEL_LOAD_IMAGE_END;

		REG_FCIE_CLRBIT(FCIE_BOOT, BIT_NAND_BOOT_EN);
	}
	return EMMC_ST_SUCCESS;
LABEL_LOAD_IMAGE_END:

	REG_FCIE_CLRBIT(FCIE_BOOT, BIT_NAND_BOOT_EN);
	return u32_err;
}

//========================================================
U32 emmc_write_boot_part(U8* pu8_DataBuf, U32 u32_DataByteCnt, U32 u32_BlkAddr, U8 u8_PartNo)
{
    U32 u32_err;
    U16 u16_SecCnt, u16_i ,u16_j;

    u32_err = emmc_check_if_ready();
    if(EMMC_ST_SUCCESS != u32_err)
    {
        emmc_debug(EMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: eMMC_Init_Ex fail: %Xh\n", u32_err);
        return u32_err;
    }

    // set Access Boot Partition 1
    if(u8_PartNo == 1)
    {
        #ifdef IP_FCIE_VERSION_5
        u32_err = emmc_modify_ext_csd(MMC_SWITCH_MODE_WRITE_BYTE, 179, BIT3|BIT0);
        #else
        u32_err = emmc_modify_ext_csd(MMC_SWITCH_MODE_WRITE_BYTE, 179, BIT6|BIT3|BIT0);
        #endif
    }
    else if(u8_PartNo == 2)
    {
        #ifdef IP_FCIE_VERSION_5
        u32_err = emmc_modify_ext_csd(MMC_SWITCH_MODE_WRITE_BYTE, 179, BIT3|BIT1);  // still boot from BP1
        #else
        u32_err = emmc_modify_ext_csd(MMC_SWITCH_MODE_WRITE_BYTE, 179, BIT6|BIT3|BIT1); // still boot from BP1
        #endif
    }
    if(EMMC_ST_SUCCESS != u32_err)
    {
        #ifdef IP_FCIE_VERSION_5
        emmc_debug(EMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: %Xh, eMMC, set Ext_CSD[179]: %Xh fail\n",
            u32_err, BIT3|BIT0);
        #else
        emmc_debug(EMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: %Xh, eMMC, set Ext_CSD[179]: %Xh fail\n",
            u32_err, BIT6|BIT3|BIT0);
        #endif
        return u32_err;
    }

    // write Boot Code
    u16_SecCnt = (u32_DataByteCnt>>9) + ((u32_DataByteCnt&0x1FF)?1:0);
    //eMMC_debug(eMMC_DEBUG_LEVEL,1,"eMMC Boot Data keeps %Xh sectors, ChkSum: %Xh \n",
    //    u16_SecCnt, eMMC_ChkSum(pu8_DataBuf, u16_SecCnt<<9));
    //    u16_SecCnt, eMMC_ChkSum(pu8_DataBuf, BOOT_PART_TOTAL_CNT<<9));

    u32_err = emmc_write_data_ex(pu8_DataBuf,
                  u16_SecCnt<<EMMC_SECTOR_512BYTE_BITS,
                  u32_BlkAddr);
    if(EMMC_ST_SUCCESS != u32_err)
    {
        emmc_debug(EMMC_DEBUG_LEVEL_ERROR,1,
            "eMMC Err: write Boot Partition fail, %Xh\n", u32_err);
        return u32_err;
    }
    #if 0
    // patch for U01 ROM code (clk not stop)
    for(u16_i=0; u16_i<eMMC_SECTOR_512BYTE; u16_i++)
        gau8_eMMC_SectorBuf[u16_i] = 0xFF;
    u32_err = eMMC_CMD24(BOOT_PART_TOTAL_CNT, gau8_eMMC_SectorBuf);
    if(eMMC_ST_SUCCESS != u32_err)
    {
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: U01 patch fail, %Xh\n", u32_err);
        return u32_err;
    }
    #endif
    // verify Boot Code
    u16_j =0;
    for(u16_i=(U16)u32_BlkAddr; u16_i<u16_SecCnt; u16_i++)
    {
        //eMMC_debug(eMMC_DEBUG_LEVEL,0,"\r checking: %03u%% ", (u16_i+1)*100/u16_SecCnt);
        //u32_err = eMMC_CMD17_CIFD(u16_i<<9, gau8_eMMC_SectorBuf);
        u32_err = emmc_cmd17(u16_i, GLOBAL_EMMC_SECTORBUF);
        if(EMMC_ST_SUCCESS != u32_err)
        {
            emmc_debug(EMMC_DEBUG_LEVEL_ERROR,1,
                "eMMC Err: read Boot Partition Sector %u fail, %Xh\n",
                u16_i, u32_err);
            return u32_err;
        }

        u32_err = emmc_compare_data(pu8_DataBuf+(u16_j<<9), GLOBAL_EMMC_SECTORBUF, EMMC_SECTOR_512BYTE);
        if(EMMC_ST_SUCCESS != u32_err)
        {
            emmc_debug(EMMC_DEBUG_LEVEL_ERROR,1,
                "eMMC Err: check Boot Partition Sector %u fail, %Xh\n",
                u16_i, u32_err);
            emmc_dump_mem(GLOBAL_EMMC_SECTORBUF, 0x200);
            return u32_err;
        }
        u16_j++;
    }

    // set Boot Bus
    u32_err = emmc_modify_ext_csd(MMC_SWITCH_MODE_WRITE_BYTE, 177, BIT1);
    if(EMMC_ST_SUCCESS != u32_err)
    {
        emmc_debug(EMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: %Xh, eMMC, set Ext_CSD[177]: %Xh fail\n",
            u32_err, BIT3|BIT1);
        return u32_err;
    }

    // clear Access Boot Partition
    #ifdef IP_FCIE_VERSION_5
    u32_err = emmc_modify_ext_csd(MMC_SWITCH_MODE_WRITE_BYTE, 179, BIT3);
    #else
    u32_err = emmc_modify_ext_csd(MMC_SWITCH_MODE_WRITE_BYTE, 179, BIT6|BIT3);
    #endif
    if(EMMC_ST_SUCCESS != u32_err)
    {
        #ifdef IP_FCIE_VERSION_5
        emmc_debug(EMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: %Xh, eMMC, set Ext_CSD[179]: %Xh fail\n",
            u32_err, BIT3);
        #else
        emmc_debug(EMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: %Xh, eMMC, set Ext_CSD[179]: %Xh fail\n",
            u32_err, BIT6|BIT3);
        #endif
        return u32_err;
    }

    //eMMC_debug(eMMC_DEBUG_LEVEL,1,"ok\n");
    return u32_err;
}

U32 emmc_read_boot_part(U8 *pu8_data_buf, U32 u32_data_byte_cnt, U32 u32_blk_addr, U8 u8_part_no)
{
	U32 u32_err;
	U16 u16_sec_cnt;

	u32_err = emmc_check_if_ready();
	if (u32_err != EMMC_ST_SUCCESS) {
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: emmc_init_ex fail: %Xh\n", u32_err);
		return u32_err;
	}

	//set Access Boot Partition 1
	if (u8_part_no == 1)
		u32_err = emmc_modify_ext_csd(MMC_SWITCH_MODE_WRITE_BYTE, 179, BIT3 | BIT0);
	else if (u8_part_no == 2)
		u32_err = emmc_modify_ext_csd(MMC_SWITCH_MODE_WRITE_BYTE, 179, BIT3 | BIT1);

	if (u32_err != EMMC_ST_SUCCESS) {
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: %Xh, eMMC, set Ext_CSD[179]: %Xh fail\n",
			   u32_err, BIT3 | BIT0);
		return u32_err;
	}

	//read Boot Code
	u16_sec_cnt = (u32_data_byte_cnt >> 9) + ((u32_data_byte_cnt & 0x1FF) ? 1 : 0);
	u32_err = emmc_read_data_ex(pu8_data_buf, u16_sec_cnt << EMMC_SECTOR_512BYTE_BITS, u32_blk_addr);
	if (u32_err != EMMC_ST_SUCCESS) {
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: read Boot Partition fail, %Xh\n", u32_err);
		return u32_err;
	}

	//clear Access Boot Partition
	u32_err = emmc_modify_ext_csd(MMC_SWITCH_MODE_WRITE_BYTE, EXT_CSD_PART_CONF, BIT3);

	if (u32_err != EMMC_ST_SUCCESS) {
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: %Xh, eMMC, set Ext_CSD[179]: %Xh fail\n",
			   u32_err, BIT3);
		return u32_err;
	}

	return u32_err;
}

void emmc_print_gp_partition(void)
{
	U8 u8_i;

	emmc_debug(EMMC_DEBUG_LEVEL, 0, "eMMC Gerneral Purpose Partition\n");
	emmc_debug(EMMC_DEBUG_LEVEL, 0, "No.          Size\n");

	for (u8_i = 0; u8_i < 4; u8_i++)
		emmc_debug(EMMC_DEBUG_LEVEL, 0, "%d          0x%X\n", u8_i, emmc_drv.gp_part[u8_i].u32_part_size);
}

//==============================================================
void emmc_dump_speed_status(void)
{
	if (emmc_drv.u32_drvflag & DRV_FLAG_DDR_MODE) {
		emmc_debug(EMMC_DEBUG_LEVEL, 0, "  DDR Mode\n");
		emmc_dump_timing_table();
		emmc_debug(EMMC_DEBUG_LEVEL, 0, "\n");
	} else {
		emmc_debug(EMMC_DEBUG_LEVEL, 0, "  SDR Mode\n");
	}
	emmc_debug(EMMC_DEBUG_LEVEL, 0, "  FCIE Clk: %u.%u MHz, %Xh\n",
		   emmc_drv.u32_clk_khz / EMMC_KHZ,
		   (emmc_drv.u32_clk_khz / 100) % 10, emmc_drv.u16_clk_reg_val);

	switch (emmc_drv.u8_bus_width) {
	case BIT_SD_DATA_WIDTH_1:
		emmc_debug(EMMC_DEBUG_LEVEL, 0, "  1-bit ");
	break;
	case BIT_SD_DATA_WIDTH_4:
		emmc_debug(EMMC_DEBUG_LEVEL, 0, "  4-bits ");
	break;
	case BIT_SD_DATA_WIDTH_8:
		emmc_debug(EMMC_DEBUG_LEVEL, 0, "  8-bits ");
	break;
	}
	emmc_debug(EMMC_DEBUG_LEVEL, 0, "width\n");
	emmc_debug(EMMC_DEBUG_LEVEL, 0, "  Bus Speed:");

	switch (emmc_drv.u32_drvflag & DRV_FLAG_SPEED_MASK) {
	case DRV_FLAG_SPEED_HIGH:
		emmc_debug(EMMC_DEBUG_LEVEL, 0, " HIGH\n");
	break;
	case DRV_FLAG_SPEED_HS200:
		emmc_debug(EMMC_DEBUG_LEVEL, 0, " HS200\n");
		emmc_dump_timing_table();
		emmc_debug(EMMC_DEBUG_LEVEL, 0, "\n");
	break;
	case DRV_FLAG_SPEED_HS400:
		emmc_debug(EMMC_DEBUG_LEVEL, 0, " HS400\n");
		emmc_dump_timing_table();
		emmc_debug(EMMC_DEBUG_LEVEL, 0, "\n");
	break;
	default:
		emmc_debug(EMMC_DEBUG_LEVEL, 0, " LOW\n");
	}
}

void emmc_dump_driver_status(void)
{
	emmc_debug(0, 1, "\n  eMMCDrvExtFlag: %Xh\n\n", gu32_emmc_drv_ext_flag);
	emmc_debug(EMMC_DEBUG_LEVEL, 0, "eMMC Status: 2014/03/25\n");

	//------------------------------------------------------
	//helpful debug info
	//------------------------------------------------------
	#if defined(ENABLE_EMMC_RIU_MODE) && ENABLE_EMMC_RIU_MODE
	emmc_debug(EMMC_DEBUG_LEVEL, 0, "  RIU Mode\n");
	#else
	emmc_debug(EMMC_DEBUG_LEVEL, 0, "  MIU Mode\n");
	#endif

	emmc_debug(EMMC_DEBUG_LEVEL, 0, "  Polling Mode\n");

	#if defined(FICE_BYTE_MODE_ENABLE) && FICE_BYTE_MODE_ENABLE
	emmc_debug(EMMC_DEBUG_LEVEL, 0, "  FCIE Byte Mode\n");
	#else
	emmc_debug(EMMC_DEBUG_LEVEL, 0, "  FCIE Block Mode\n");
	#endif

	emmc_dump_speed_status();

	emmc_debug(EMMC_DEBUG_LEVEL, 0, "  PwrOff Notification: ");
	switch (emmc_drv.u32_drvflag & DRV_FLAG_PWR_OFF_NOTIF_LONG_MASK) {
	case DRV_FLAG_PWR_OFF_NOTIF_OFF:
		emmc_debug(EMMC_DEBUG_LEVEL, 0, "OFF\n");
	break;
	case DRV_FLAG_PWR_OFF_NOTIF_ON:
		emmc_debug(EMMC_DEBUG_LEVEL, 0, "ON\n");
	break;
	case DRV_FLAG_PWR_OFF_NOTIF_SHORT:
		emmc_debug(EMMC_DEBUG_LEVEL, 0, "SHORT\n");
	break;
	case DRV_FLAG_PWR_OFF_NOTIF_LONG:
		emmc_debug(EMMC_DEBUG_LEVEL, 0, "LONG\n");
	break;
	default:
		emmc_debug(EMMC_DEBUG_LEVEL, 0, " eMMC Err: unknown: %Xh\n",
			   emmc_drv.u32_drvflag & DRV_FLAG_PWR_OFF_NOTIF_LONG_MASK);
	}
}

U32 emmc_init(void)
{
	U32 u32_err;

	u32_err = emmc_check_if_ready();
	if (u32_err != EMMC_ST_SUCCESS)
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: emmc_init_ex fail: %Xh\n", u32_err);

	return u32_err;
}

#define EMMC_NOT_READY_MARK    ~(('e' << 24) | ('M' << 16) | ('M' << 8) | 'C')
static U32 sgu32_if_ready_guard = EMMC_NOT_READY_MARK;

static U32 emmc_init_ex(void)
{
	U32 u32_err;
	U8 u8_cid[EMMC_MAX_RSP_BYTE_CNT];
	U8 u8_i;

	#if defined(EMMC_SKIP_IDENTIFY) && EMMC_SKIP_IDENTIFY
	//emmc_debug(0, 1, "0\n");

	if (emmc_fcie_check_fast_mode() == 0 || sizeof(emmc_drv) > 512 * CONTEXT_SIZE_BLKCNT)
		goto LABEL_NOT_SKIP_INIT;

	//emmc_debug(0, 1, "1\n");
	EMMC_SKIP_IDENTIFY_SAVE(1);
	emmc_drv.u16_mmc_mode = BIT_SD_DEFAULT_MODE_REG;
	emmc_drv.u8_if_sector_mode = 1;
	emmc_drv.u8_bus_width = BIT_SD_DATA_WIDTH_8;
	emmc_drv.u16_rca = 1;

	u32_err = emmc_load_context_rsp();
	if (u32_err != EMMC_ST_SUCCESS)
		goto LABEL_NOT_SKIP_INIT;

	//emmc_debug(0, 1, "2\n");
	EMMC_SKIP_IDENTIFY_SAVE(0);
	emmc_ext_csd_init();
	goto LABEL_INIT_END;

LABEL_NOT_SKIP_INIT:
	#endif

	//---------------------------------
	memset((void *)&emmc_drv, '\0', sizeof(struct emmc_driver) -
	       sizeof(struct mtk_fcie_host *));
	emmc_drv.u8_partition_config = 8;
	emmc_drv.u16_mmc_mode = BIT_SD_DEFAULT_MODE_REG;
	#ifdef CONFIG_ROM_OVERLAY
	emmc_drv.chip_id = romtbl_get_chip_id_info_dec();
	emmc_drv.revision = romtbl_get_chip_revision_info();

	if (emmc_drv.chip_id == MT5896_CHIP_ID && emmc_drv.revision == MT5896_CHIP_VER)
		emmc_drv.u8_emmc_pll_skew4 = 1;

	#endif
	//---------------------------------
	//init platform & FCIE
	emmc_platform_init();
	u32_err = emmc_fcie_init();
	if (u32_err)
		goto  LABEL_INIT_END;

	//---------------------------------
	u32_err = emmc_init_device();
	emmc_debug(EMMC_DEBUG_LEVEL, 0, "eMMC ");

	if (u32_err)
		goto LABEL_INIT_END;

	#if defined(EMMC_SKIP_IDENTIFY) && EMMC_SKIP_IDENTIFY
	if (EMMC_IF_SKIP_IDENTIFY_SAVE())
		emmc_save_context_rsp();
	#endif

	//---------------------------------
LABEL_INIT_END:
	emmc_drv.u32_last_err_code = u32_err;
	emmc_drv.u32_drvflag |= DRV_FLAG_INIT_DONE;
	sgu32_if_ready_guard = ~EMMC_NOT_READY_MARK;
	//setup ID
	//use first 10 bytes of CID
	emmc_debug(EMMC_DEBUG_LEVEL, 0, "eMMC %u.%u GB\n",
		   emmc_drv.u32_sec_count >> 11 >> 10,
		   (emmc_drv.u32_sec_count >> 11) * 100 / 1024 % 100);
	//emmc_dump_mem(emmc_drv.au8_ID, 0x10);
	memset(u8_cid, 0, EMMC_MAX_RSP_BYTE_CNT);

	for (u8_i = 0; u8_i < EMMC_MAX_RSP_BYTE_CNT - 1; u8_i++)
		u8_cid[u8_i + 1] =
			emmc_drv.au8_cid[EMMC_MAX_RSP_BYTE_CNT - u8_i - 1];

	emmc_debug(EMMC_DEBUG_LEVEL, 0, "CID:");//Customer ask log must be same with mtk's project

	for (u8_i = 0; u8_i < EMMC_MAX_RSP_BYTE_CNT; u8_i++) {
		if ((u8_i % 4) == 0)
			emmc_debug(EMMC_DEBUG_LEVEL, 0, "0x");

		emmc_debug(EMMC_DEBUG_LEVEL, 0, "%02X", u8_cid[u8_i]);
		if ((u8_i % 4) == 3)
			emmc_debug(EMMC_DEBUG_LEVEL, 0, "\n");
	}

	return u32_err;
}

U32 emmc_init_device_ex(void)
{
	U32 u32_err;

	emmc_drv.u32_drvflag &= ~DRV_FLAG_INIT_DONE;

	u32_err = emmc_identify();
	if (u32_err)
		return u32_err;

	emmc_clock_setting(FCIE_SLOW_CLK);

	u32_err = emmc_csd_config();
	if (u32_err != EMMC_ST_SUCCESS)
		return u32_err;

	u32_err = emmc_cmd3_cmd7(emmc_drv.u16_rca, MMC_CMD_SELECT_CARD);
	if (u32_err != EMMC_ST_SUCCESS)
		return u32_err;

	u32_err = emmc_ext_csd_config();
	if (u32_err != EMMC_ST_SUCCESS)
		return u32_err;

	u32_err = emmc_set_bus_width(8, 0);
	if (u32_err != EMMC_ST_SUCCESS)
		return u32_err;

	u32_err = emmc_set_bus_speed(EMMC_SPEED_HIGH);
	if (u32_err != EMMC_ST_SUCCESS)
		return u32_err;

	emmc_clock_setting(FCIE_DEFAULT_CLK);
	#ifndef CONFIG_MSTAR_TITANIA_BD_FPGA
	#ifdef IP_FCIE_VERSION_5
	emmc_pads_switch(FCIE_EMMC_SDR);
	#endif
	#endif

	return u32_err;
}


U32 emmc_init_device(void)
{
	U32 u32_err;

	u32_err = emmc_init_device_ex();
	if (u32_err != EMMC_ST_SUCCESS)
		goto  LABEL_INIT_END;

	//----------------------------------------
	#ifndef CONFIG_MSTAR_TITANIA_BD_FPGA
	u32_err = emmc_fcie_choose_speed_mode();
	if (u32_err != EMMC_ST_SUCCESS)
		goto  LABEL_INIT_END;
		//while(1);
	#endif
	//----------------------------------------

	//emmc_dump_mem(emmc_drv.au8_AllRsp, 0x100);
LABEL_INIT_END:

	return u32_err;
}

U32 emmc_check_if_ready(void)
{
	if (sgu32_if_ready_guard != EMMC_NOT_READY_MARK)
		return EMMC_ST_SUCCESS;

	return emmc_init_ex();
}

void emmc_reset_ready_flag(void)
{
	sgu32_if_ready_guard = EMMC_NOT_READY_MARK;
}


// =======================================================
// u32_data_byte_cnt: has to be 512B-boundary !
// =======================================================
static U32 emmc_erase_block_ex(U32 u32_emmc_blk_addr_start, U32 u32_emmc_blk_addr_end)
{
	U32 u32_i, u32_sector_cnt, u32_erase_sector_cnt, u32_one_time_erase_sector_cnt;
	U32 u32_err = 0, u32_erase_start, u32_erase_end;

	u32_sector_cnt = u32_emmc_blk_addr_end - u32_emmc_blk_addr_start;
	u32_one_time_erase_sector_cnt = (1024 * 1024 * 1024) >> 9;

	if (u32_sector_cnt <= u32_one_time_erase_sector_cnt) {
		u32_err =  emmc_erase_cmd_seq(u32_emmc_blk_addr_start, u32_emmc_blk_addr_end);
		if (u32_err != EMMC_ST_SUCCESS) {
			emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: EraseCMDSeq fail 0: %Xh\n", u32_err);
			return u32_err;
		}
	} else {
		u32_erase_start = u32_emmc_blk_addr_start;
		u32_erase_end = u32_emmc_blk_addr_start +
			(u32_sector_cnt / u32_one_time_erase_sector_cnt) * u32_one_time_erase_sector_cnt;

		for (u32_i = 0; u32_i < (u32_sector_cnt / u32_one_time_erase_sector_cnt); u32_i++) {
			u32_erase_start = u32_emmc_blk_addr_start + u32_i * u32_one_time_erase_sector_cnt;
			u32_err =  emmc_erase_cmd_seq(u32_erase_start, u32_erase_start + u32_one_time_erase_sector_cnt);
			if (u32_err != EMMC_ST_SUCCESS) {
				emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: EraseCMDSeq fail 1: %Xh\n", u32_err);
				return u32_err;
			}
		}

		u32_erase_sector_cnt = u32_sector_cnt - (u32_erase_end - u32_emmc_blk_addr_start);
		if (u32_erase_sector_cnt) {
			u32_err =  emmc_erase_cmd_seq(u32_erase_end, u32_emmc_blk_addr_end);
			if (u32_err != EMMC_ST_SUCCESS) {
				emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: EraseCMDSeq fail 2: %Xh\n", u32_err);
				return u32_err;
			}
		}
	}

	return u32_err;
}

U32 emmc_erase_block(U32 u32_emmc_blk_addr_start, U32 u32_emmc_blk_addr_end)
{
	U32 u32_err = 0, u32_sector_cnt, u32_i, u32_j;

	if (emmc_drv.u32_emmc_flag & EMMC_FLAG_TRIM) {
		u32_err =  emmc_erase_block_ex(u32_emmc_blk_addr_start, u32_emmc_blk_addr_end);
		if (u32_err != EMMC_ST_SUCCESS) {
			emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: EraseCMDSeq fail 0: %Xh\n", u32_err);
			return u32_err;
		}
	} else {
			for (u32_i = 0; u32_i < EMMC_SECTOR_BUF_BYTECTN; u32_i++)
				GLOBAL_EMMC_SECTORBUF[u32_i] = emmc_drv.u8_erased_mem_content;

			//erase blocks before EraseUnitSize
			u32_sector_cnt = u32_emmc_blk_addr_start / emmc_drv.u32_erase_unit_size;
			u32_sector_cnt = (u32_sector_cnt + 1) * emmc_drv.u32_erase_unit_size;
			u32_sector_cnt -= u32_emmc_blk_addr_start;
			u32_sector_cnt =
				u32_sector_cnt > (u32_emmc_blk_addr_end - u32_emmc_blk_addr_start) ?
				(u32_emmc_blk_addr_end - u32_emmc_blk_addr_start) : u32_sector_cnt;

			for (u32_i = 0; u32_i < u32_sector_cnt; u32_i++) {
				u32_j = ((u32_sector_cnt - u32_i) << 9) > EMMC_SECTOR_BUF_BYTECTN ?
					EMMC_SECTOR_BUF_BYTECTN : ((u32_sector_cnt - u32_i) << 9);

				u32_err = emmc_write_data_ex(GLOBAL_EMMC_SECTORBUF, u32_j, u32_emmc_blk_addr_start + u32_i);
				if (u32_err != EMMC_ST_SUCCESS) {
					emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
						   "eMMC Err: WriteData fail 0, %Xh\n", u32_err);
					return u32_err;
				}
				u32_i += u32_j >> EMMC_SECTOR_512BYTE_BITS;
		}
		if ((u32_emmc_blk_addr_end - u32_emmc_blk_addr_start) == u32_sector_cnt)
			goto LABEL_END_OF_ERASE;

		//erase blocks
		u32_i = (u32_emmc_blk_addr_end - (u32_emmc_blk_addr_start + u32_sector_cnt)) / emmc_drv.u32_erase_unit_size;
		if (u32_i) {
			u32_err = emmc_erase_block_ex((u32_emmc_blk_addr_start + u32_sector_cnt),
						      (u32_emmc_blk_addr_start + u32_sector_cnt) +
						      u32_i * emmc_drv.u32_erase_unit_size);
				if (u32_err != EMMC_ST_SUCCESS) {
					emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
						   "eMMC Err: EraseCMDSeq fail 1, %Xh\n", u32_err);
					return u32_err;
				}
		}

		//erase blocks after EraseUnitSize
		u32_emmc_blk_addr_start =
			(u32_emmc_blk_addr_start + u32_sector_cnt) + u32_i * emmc_drv.u32_erase_unit_size;

		while (u32_emmc_blk_addr_start < u32_emmc_blk_addr_end) {
			u32_j = ((u32_emmc_blk_addr_end - u32_emmc_blk_addr_start) << 9) > EMMC_SECTOR_BUF_BYTECTN ?
				EMMC_SECTOR_BUF_BYTECTN : ((u32_emmc_blk_addr_end - u32_emmc_blk_addr_start) << 9);

			u32_err = emmc_write_data_ex(GLOBAL_EMMC_SECTORBUF, u32_j, u32_emmc_blk_addr_start);
			if (u32_err != EMMC_ST_SUCCESS) {
				emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: WriteData fail 1, %Xh\n", u32_err);
				return u32_err;
			}
			u32_emmc_blk_addr_start += u32_j >> 9;
		}
	}

LABEL_END_OF_ERASE:
	return u32_err;
}

// ok: return 0
U32 emmc_write_data(U8 *pu8_data_buf, U32 u32_data_byte_cnt, U32 u32_blk_addr)
{
	U32 u32_err;

	emmc_lock_fcie(NULL);

	u32_err = emmc_write_data_ex(pu8_data_buf, u32_data_byte_cnt, u32_blk_addr);
	emmc_unlock_fcie(NULL);
	return u32_err;
}

U32 emmc_write_data_ex(U8 *pu8_data_buf, U32 u32_data_byte_cnt, U32 u32_blk_addr)
{
	U32 u32_err = EMMC_ST_SUCCESS;
	U16 u16_blk_cnt;

	//check if eMMC Init
	if (sgu32_if_ready_guard == EMMC_NOT_READY_MARK) {
		u32_err = emmc_init();
		if (u32_err != EMMC_ST_SUCCESS) {
			emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: eMMC not ready (init)\n");
			return EMMC_ST_ERR_NOT_INIT;
		}
	}

	if (!pu8_data_buf) {
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: w data buf is NULL: %lXh\n", (unsigned long)pu8_data_buf);
		return EMMC_ST_ERR_INVALID_PARAM;
	}
	//check if u32_data_byte_cnt is 512B boundary
	if (u32_data_byte_cnt & (EMMC_SECTOR_512BYTE - 1)) {
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: data not 512B boundary\n");
		return EMMC_ST_ERR_INVALID_PARAM;
	}
	if (u32_blk_addr + (u32_data_byte_cnt >> 9) > emmc_drv.u32_sec_count) {
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: invalid data range, %Xh > %Xh\n",
			   u32_blk_addr + (u32_data_byte_cnt >> 9), emmc_drv.u32_sec_count);
		return EMMC_ST_ERR_INVALID_PARAM;
	}

	//write data
	while (u32_data_byte_cnt) {
		if (u32_data_byte_cnt > EMMC_SECTOR_512BYTE) {
			if ((u32_data_byte_cnt >> EMMC_SECTOR_512BYTE_BITS) < BIT_SD_JOB_BLK_CNT_MASK)
				u16_blk_cnt = (u32_data_byte_cnt >> EMMC_SECTOR_512BYTE_BITS);
			else
				u16_blk_cnt = BIT_SD_JOB_BLK_CNT_MASK;

			u32_err = emmc_cmd25(u32_blk_addr, pu8_data_buf, u16_blk_cnt);
		} else {
			u16_blk_cnt = 1;
			u32_err = emmc_cmd24(u32_blk_addr, pu8_data_buf);
			if (u32_err == EMMC_ST_SUCCESS)
				u32_err = emmc_cmd13(emmc_drv.u16_rca);
		}

		if (u32_err != EMMC_ST_SUCCESS) {
			emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: W fail: %Xh\n", u32_err);
			emmc_drv.u32_last_err_code = u32_err;
			break;
		}

		u32_blk_addr += u16_blk_cnt;
		pu8_data_buf += u16_blk_cnt << EMMC_SECTOR_512BYTE_BITS;
		u32_data_byte_cnt -= u16_blk_cnt << EMMC_SECTOR_512BYTE_BITS;
	}

	return u32_err;
}

// =======================================================
// u32_data_byte_cnt: has to be 512B-boundary !
// =======================================================
// ok: return 0
U32 emmc_read_data(U8 *pu8_data_buf, U32 u32_data_byte_cnt, U32 u32_blk_addr)
{
	U32 u32_err;

	emmc_lock_fcie(NULL);

	u32_err = emmc_read_data_ex(pu8_data_buf, u32_data_byte_cnt, u32_blk_addr);
	emmc_unlock_fcie(NULL);

	return u32_err;
}

U32 emmc_read_data_ex(U8 *pu8_data_buf, U32 u32_data_byte_cnt, U32 u32_blk_addr)
{
	U32 u32_err = 0;
	U16 u16_blk_cnt;
	U8 u8_if_not_cache_line_aligned = 0;

	//emmc_debug(EMMC_DEBUG_LEVEL,1,"\n");
	//check if eMMC Init
	if (sgu32_if_ready_guard == EMMC_NOT_READY_MARK) {
		u32_err = emmc_init();
		if (u32_err != EMMC_ST_SUCCESS) {
			emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: eMMC not ready (init)\n");
			return EMMC_ST_ERR_NOT_INIT;
		}
	}
	//check if u32_data_byte_cnt is 512B boundary
	if (u32_data_byte_cnt & (EMMC_SECTOR_512BYTE - 1)) {
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: data not 512B boundary\n");
		return EMMC_ST_ERR_INVALID_PARAM;
	}
	if (u32_blk_addr + (u32_data_byte_cnt >> 9) > emmc_drv.u32_sec_count) {
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: invalid data range, %Xh > %Xh\n",
			   u32_blk_addr + (u32_data_byte_cnt >> 9), emmc_drv.u32_sec_count);
		return EMMC_ST_ERR_INVALID_PARAM;
	}

	//read data
	//first 512 bytes, special handle if not cache line aligned
	if ((unsigned long)pu8_data_buf & (EMMC_CACHE_LINE - 1)) {
		u32_err = emmc_cmd17(u32_blk_addr, GLOBAL_EMMC_SECTORBUF);
		if (u32_err != EMMC_ST_SUCCESS) {
			emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: R fail.0: %Xh\n", u32_err);
			emmc_drv.u32_last_err_code = u32_err;
			return u32_err;
		}
		memcpy(pu8_data_buf, GLOBAL_EMMC_SECTORBUF, EMMC_SECTOR_512BYTE);
		u32_blk_addr += 1;
		pu8_data_buf += EMMC_SECTOR_512BYTE;
		u32_data_byte_cnt -= EMMC_SECTOR_512BYTE;
		//last 512B must be not cache line aligned,
		//reserved for last emmc_cmd17_cifd
		if (u32_data_byte_cnt) {
			u8_if_not_cache_line_aligned = 1;
			u32_data_byte_cnt -= EMMC_SECTOR_512BYTE;
		}
	}

	while (u32_data_byte_cnt) {
		if (u32_data_byte_cnt > EMMC_SECTOR_512BYTE) {
			if ((u32_data_byte_cnt >> 9) < BIT_SD_JOB_BLK_CNT_MASK)
				u16_blk_cnt = (u32_data_byte_cnt >> 9);
			else
				u16_blk_cnt = BIT_SD_JOB_BLK_CNT_MASK;

			u32_err = emmc_cmd18(u32_blk_addr, pu8_data_buf, u16_blk_cnt);
		} else {
			u16_blk_cnt = 1;
			u32_err = emmc_cmd17(u32_blk_addr, pu8_data_buf);
		}

		if (u32_err != EMMC_ST_SUCCESS) {
			emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: R fail.1: %Xh\n", u32_err);
			emmc_drv.u32_last_err_code = u32_err;
			break;
		}

		u32_blk_addr += u16_blk_cnt;
		pu8_data_buf += u16_blk_cnt << EMMC_SECTOR_512BYTE_BITS;
		u32_data_byte_cnt -= u16_blk_cnt << EMMC_SECTOR_512BYTE_BITS;
	}

	//last 512 bytes, special handle if not cache line aligned
	if (u8_if_not_cache_line_aligned) {
		u32_err = emmc_cmd17(u32_blk_addr, GLOBAL_EMMC_SECTORBUF);
		if (u32_err != EMMC_ST_SUCCESS) {
			emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: R fail.2: %Xh\n", u32_err);
			emmc_drv.u32_last_err_code = u32_err;
		}
		memcpy(pu8_data_buf, GLOBAL_EMMC_SECTORBUF, EMMC_SECTOR_512BYTE);
	}

	return u32_err;
}

U32  emmc_get_capacity(U32 *pu32_total_sector_cnt)//1 sector = 512B
{
	U32 u32_err;

	emmc_lock_fcie(NULL);

	if (sgu32_if_ready_guard == EMMC_NOT_READY_MARK) {
		u32_err = emmc_init();
		if (u32_err != EMMC_ST_SUCCESS) {
			emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: eMMC not ready (init)\n");
			emmc_unlock_fcie(NULL);

			return EMMC_ST_ERR_NOT_INIT;
		}
	}

	*pu32_total_sector_cnt = emmc_drv.u32_sec_count;
	emmc_unlock_fcie(NULL);
	return EMMC_ST_SUCCESS;
}

U32 emmc_get_cid(U8 *pu8_cid)
{
	if (!pu8_cid)
		return EMMC_ST_ERR_INVALID_PARAM;

	memcpy(pu8_cid, &emmc_drv.au8_cid[1], 15);
	pu8_cid[15] = 0; //no CRC

	return EMMC_ST_SUCCESS;
}
#endif
