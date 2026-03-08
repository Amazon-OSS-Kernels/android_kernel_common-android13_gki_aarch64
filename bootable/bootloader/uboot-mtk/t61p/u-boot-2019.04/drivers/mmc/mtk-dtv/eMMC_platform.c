// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (c) 2023 MediaTek Inc.
*/

#include "eMMC.h"
#if defined(UNIFIED_eMMC_DRIVER) && UNIFIED_eMMC_DRIVER

#if defined(ENABLE_EMMC_POWER_SAVING_MODE) && ENABLE_EMMC_POWER_SAVING_MODE

void emmc_prepare_power_saving_mode_queue(void)
{
	//REG_FCIE_SETBIT(reg_pwrgd_int_glirm, BIT_PWRGD_INT_GLIRM_EN);
	//REG_FCIE_CLRBIT(reg_pwrgd_int_glirm, BIT_PWEGD_INT_GLIRM_MASK);
	//REG_FCIE_SETBIT(reg_pwrgd_int_glirm, (0x3E << 10));

	REG_FCIE_CLRBIT(FCIE_PWR_SAVE_CTL, BIT_BAT_SD_POWER_SAVE_MASK);

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

	/* (7) Set "FCIE_DDR_MODE" */
	#if defined(ENABLE_EMMC_HS400) && ENABLE_EMMC_HS400
	REG_FCIE_W(GET_REG_ADDR(FCIE_POWEER_SAVE_MODE_BASE, 0x0C), 0x1100);
	#elif defined(ENABLE_EMMC_HS200) && ENABLE_EMMC_HS200
	REG_FCIE_W(GET_REG_ADDR(FCIE_POWEER_SAVE_MODE_BASE, 0x0C), 0x1000);
	#elif defined(ENABLE_EMMC_ATOP) && ENABLE_EMMC_ATOP
	REG_FCIE_W(GET_REG_ADDR(FCIE_POWEER_SAVE_MODE_BASE, 0x0C), 0x0180);
	#endif
	REG_FCIE_W(GET_REG_ADDR(FCIE_POWEER_SAVE_MODE_BASE, 0x0D),
		   PWR_BAT_CLASS | PWR_RST_CLASS | PWR_CMD_WREG | PWR_CMD_BK0 | 0x0F);

	/* (8) Set "SD_MOD" */
	REG_FCIE_W(GET_REG_ADDR(FCIE_POWEER_SAVE_MODE_BASE, 0x0E), 0x0021);
	REG_FCIE_W(GET_REG_ADDR(FCIE_POWEER_SAVE_MODE_BASE, 0x0F),
		   PWR_BAT_CLASS | PWR_RST_CLASS | PWR_CMD_WREG | PWR_CMD_BK0 | 0x0B);

	/* (9) Enable "reg_sd_en" */
	REG_FCIE_W(GET_REG_ADDR(FCIE_POWEER_SAVE_MODE_BASE, 0x10), 0x0001);
	REG_FCIE_W(GET_REG_ADDR(FCIE_POWEER_SAVE_MODE_BASE, 0x11),
		   PWR_BAT_CLASS | PWR_RST_CLASS | PWR_CMD_WREG | PWR_CMD_BK0 | 0x07);

	/* (10) Command Content, IDLE */
	REG_FCIE_W(GET_REG_ADDR(FCIE_POWEER_SAVE_MODE_BASE, 0x12), 0x0040);
	REG_FCIE_W(GET_REG_ADDR(FCIE_POWEER_SAVE_MODE_BASE, 0x13),
		   PWR_BAT_CLASS | PWR_RST_CLASS | PWR_CMD_WREG | PWR_CMD_BK0 | 0x20);

	/* (11) Command Content, STOP */
	REG_FCIE_W(GET_REG_ADDR(FCIE_POWEER_SAVE_MODE_BASE, 0x14), 0x0000);
	REG_FCIE_W(GET_REG_ADDR(FCIE_POWEER_SAVE_MODE_BASE, 0x15),
		   PWR_BAT_CLASS | PWR_RST_CLASS | PWR_CMD_WREG | PWR_CMD_BK0 | 0x21);

	/* (12) Command Content, STOP */
	REG_FCIE_W(GET_REG_ADDR(FCIE_POWEER_SAVE_MODE_BASE, 0x16), 0x0000);
	REG_FCIE_W(GET_REG_ADDR(FCIE_POWEER_SAVE_MODE_BASE, 0x17),
		   PWR_BAT_CLASS | PWR_RST_CLASS | PWR_CMD_WREG | PWR_CMD_BK0 | 0x22);

	/* (13) Command & Response Size */
	REG_FCIE_W(GET_REG_ADDR(FCIE_POWEER_SAVE_MODE_BASE, 0x18), 0x0500);
	REG_FCIE_W(GET_REG_ADDR(FCIE_POWEER_SAVE_MODE_BASE, 0x19),
		   PWR_BAT_CLASS | PWR_RST_CLASS | PWR_CMD_WREG | PWR_CMD_BK0 | 0x0A);

	/* (14) Enable Interrupt, SD_CMD_END */
	REG_FCIE_W(GET_REG_ADDR(FCIE_POWEER_SAVE_MODE_BASE, 0x1A), 0x0002);
	REG_FCIE_W(GET_REG_ADDR(FCIE_POWEER_SAVE_MODE_BASE, 0x1B),
		   PWR_BAT_CLASS | PWR_RST_CLASS | PWR_CMD_WREG | PWR_CMD_BK0 | 0x01);

	/* (15) Command Enable + job Start */
	REG_FCIE_W(GET_REG_ADDR(FCIE_POWEER_SAVE_MODE_BASE, 0x1C), 0x0044);
	REG_FCIE_W(GET_REG_ADDR(FCIE_POWEER_SAVE_MODE_BASE, 0x1D),
		   PWR_BAT_CLASS | PWR_RST_CLASS | PWR_CMD_WREG | PWR_CMD_BK0 | 0x0C);

	/* (16) Wait Interrupt */
	REG_FCIE_W(GET_REG_ADDR(FCIE_POWEER_SAVE_MODE_BASE, 0x1E), 0x0000);
	REG_FCIE_W(GET_REG_ADDR(FCIE_POWEER_SAVE_MODE_BASE, 0x1F),
		   PWR_BAT_CLASS | PWR_RST_CLASS | PWR_CMD_WINT);

	/* (17) STOP */
	REG_FCIE_W(GET_REG_ADDR(FCIE_POWEER_SAVE_MODE_BASE, 0x20), 0x0000);
	REG_FCIE_W(GET_REG_ADDR(FCIE_POWEER_SAVE_MODE_BASE, 0x21),
		   PWR_BAT_CLASS | PWR_RST_CLASS | PWR_CMD_STOP);

	REG_FCIE_CLRBIT(FCIE_PWR_SAVE_CTL, BIT_SD_POWER_SAVE_RST);
	REG_FCIE_SETBIT(FCIE_PWR_SAVE_CTL, BIT_SD_POWER_SAVE_RST);

	REG_FCIE_SETBIT(FCIE_PWR_SAVE_CTL, BIT_POWER_SAVE_MODE | BIT_POWER_SAVE_MODE_INT_EN);
}

#endif

void emmc_dump_pad_clk(void)
{

	//---------------------------------------------------------------------
	emmc_debug(0, 0, "[pad setting]: ");
	switch (emmc_drv.pad_type) {
	case FCIE_MODE_GPIO_PAD_DEFO_SPEED:
		emmc_debug(0, 0, "FCIE_MODE_GPIO_PAD_DEFO_SPEED\n");
	break;
	case FCIE_MODE_8BITS_MACRO_HIGH_SPEED:
		emmc_debug(0, 0, "FCIE_MODE_8BITS_MACRO_HIGH_SPEED\n");
	break;
	case FCIE_MODE_8BITS_MACRO_DDR52:
		emmc_debug(0, 0, "FCIE_MODE_8BITS_MACRO_DDR52\n");
	break;
	case FCIE_MODE_32BITS_MACRO_HS200:
		emmc_debug(0, 0, "FCIE_MODE_32BITS_MACRO_HS200\n");
	break;
	case FCIE_MODE_32BITS_MACRO_HS400_DS:
		emmc_debug(0, 0, "FCIE_MODE_32BITS_MACRO_HS400_DS\n");
	break;
	case FCIE_MODE_32BITS_MACRO_HS400_AIFO_5_1:
		emmc_debug(0, 0, "FCIE_MODE_32BITS_MACRO_HS400_AIFO_5_1\n");
	break;
	default:
		emmc_debug(0, 0, "eMMC Err: Pad unknown, %d\n", emmc_drv.pad_type);
		emmc_die("\n");
	break;
	}

	emmc_debug(0, 0, "FCIE_BOOT_CONFIG = %04Xh\n", REG_FCIE(FCIE_BOOT_CONFIG));
	emmc_debug(0, 0, "FCIE_SD_MODE = %04Xh (check data sync)\n", REG_FCIE(FCIE_SD_MODE));

	if (emmc_drv.host->no_clk_framework) {
		if (emmc_drv.host->dev_comp->mt5896_clk_layout) {
			emmc_debug(0, 0, "reg_MT5896_ckg_fcie = %04Xh\n", REG_FCIE(REG_MT5896_CKG_FCIE));
			emmc_debug(0, 0, "REG_MT5896_CLG_FCIE_TSP = %04Xh\n", REG_FCIE(REG_MT5896_CLG_FCIE_TSP));
			emmc_debug(0, 0, "REG_MT5896_CKG_FCIE_SYNC = %04Xh\n", REG_FCIE(REG_MT5896_CKG_FCIE_SYNC));
			emmc_debug(0, 0, "REG_MT5896_CKG_SMI_FCIE_SEL = %04Xh\n", REG_FCIE(REG_MT5896_CKG_SMI_FCIE_SEL));
		} else {
			emmc_debug(0, 0, "REG_CKG_FCIE = %04Xh\n", REG_FCIE(REG_CKG_FCIE));
		}
	}
}

static void emmc_pads_switch_default(void)
{
	//fcie
	REG_FCIE_CLRBIT(FCIE_DDR_MODE,
			BIT_FALL_LATCH | BIT_PAD_IN_SEL_SD | BIT_CLK2_SEL | BIT_32BIT_MACRO_EN |
			BIT_DDR_EN | BIT_8BIT_MACRO_EN | BIT3 | BIT2 | BIT1);
	//emmc_pll

	REG_FCIE_CLRBIT(reg_emmcpll_0x09, BIT0);
	REG_FCIE_CLRBIT(reg_emmcpll_0x1a, BIT10 | BIT5 | BIT4);
	REG_FCIE_CLRBIT(reg_emmcpll_0x1c, BIT8 | BIT9);
	REG_FCIE_CLRBIT(reg_emmcpll_0x1f, BIT2);
	REG_FCIE_CLRBIT(reg_emmcpll_0x20, BIT9 | BIT10);

	REG_FCIE_CLRBIT(reg_emmcpll_0x63, BIT0);
	REG_FCIE_CLRBIT(reg_emmcpll_0x68, BIT0 | BIT1);
	REG_FCIE_CLRBIT(reg_emmcpll_0x69, BIT7 | BIT6 | BIT5 | BIT4 | BIT3);

	if (emmc_drv.u8_emmc_pll_skew4)
		REG_FCIE_CLRBIT(reg_emmcpll_0x69, BIT_SKEW4_CLK_INV_MASK);

	REG_FCIE_CLRBIT(reg_emmcpll_0x6a, BIT0 | BIT1);
	REG_FCIE_W(reg_emmcpll_0x6b, 0x0000);
	REG_FCIE_CLRBIT(reg_emmcpll_0x6d, BIT0);
	REG_FCIE_CLRBIT(reg_emmcpll_0x70, BIT11 | BIT10 | BIT8);

	REG_FCIE_CLRBIT(reg_emmcpll_0x7f, BIT11 | BIT10 | BIT9 | BIT8 | BIT3 | BIT2 | BIT1);
	if (emmc_drv.host->dev_comp->mt5896_clk_layout)
		REG_FCIE_SETBIT(reg_emmcpll_0x74, BIT14);

	if (emmc_drv.chip_id == MT5879_CHIP_ID)
		REG_FCIE_CLRBIT(reg_emmcpll_0x5f, BIT_FLASH_MACRO_TO_FICE);
}

static U32 emmc_pads_switch_ddr(void)
{
	emmc_debug(EMMC_DEBUG_LEVEL_LOW, 0, "DDR\n");
	//emmc_pll
	REG_FCIE_SETBIT(reg_emmcpll_0x1c, BIT9);
	//8 bits macro reset + 32 bits macro reset
	REG_FCIE_CLRBIT(reg_emmcpll_0x6f, BIT0 | BIT1);
	REG_FCIE_SETBIT(reg_emmcpll_0x6f, BIT0 | BIT1);
	REG_FCIE_SETBIT(reg_emmcpll_0x68, BIT0);
	REG_FCIE_SETBIT(FCIE_DDR_MODE, BIT_DDR_EN | BIT_8BIT_MACRO_EN);

	if (emmc_drv.u8_bus_width == BIT_SD_DATA_WIDTH_4) {
		REG_FCIE_SETBIT(reg_emmcpll_0x6a, 1 << 0);
		REG_FCIE_W(reg_emmcpll_0x6b, 0x0213);
	} else if (emmc_drv.u8_bus_width == BIT_SD_DATA_WIDTH_8) {
		REG_FCIE_SETBIT(reg_emmcpll_0x6a, 2 << 0);
		REG_FCIE_W(reg_emmcpll_0x6b, 0x0113);
	} else {
		return EMMC_ST_ERR_INVALID_PARAM;
	}
	REG_FCIE_SETBIT(reg_emmcpll_0x6d, BIT0);
	REG_FCIE_W(reg_emmcpll_0x71, 0xFFFF);
	REG_FCIE_W(reg_emmcpll_0x73, 0xFFFF);

	return EMMC_ST_SUCCESS;
}

static U32 emmc_pads_switch_hs200(void)
{
	emmc_debug(EMMC_DEBUG_LEVEL_LOW, 0, "HS200\n");
	REG_FCIE_SETBIT(reg_emmcpll_0x1a, BIT5 | BIT4);
	REG_FCIE_SETBIT(reg_emmcpll_0x1c, BIT8);

	REG_FCIE_SETBIT(reg_emmcpll_0x20, BIT9 | BIT10);
	//8 bits macro reset + 32 bits macro reset
	REG_FCIE_CLRBIT(reg_emmcpll_0x6f, BIT0 | BIT1);
	REG_FCIE_SETBIT(reg_emmcpll_0x6f, BIT0 | BIT1);
	REG_FCIE_SETBIT(reg_emmcpll_0x68, BIT0);

	REG_FCIE_SETBIT(FCIE_DDR_MODE, BIT_32BIT_MACRO_EN);
	REG_FCIE_SETBIT(reg_emmcpll_0x69, 4 << 4);
	if (emmc_drv.u8_emmc_pll_skew4)
		REG_FCIE_SETBIT(reg_emmcpll_0x69, BIT_SKEW4_ALL_INV);

	if (emmc_drv.u8_bus_width == BIT_SD_DATA_WIDTH_4) {
		REG_FCIE_SETBIT(reg_emmcpll_0x6a, 1 << 0);
		REG_FCIE_W(reg_emmcpll_0x6b, 0x0413);
	} else if (emmc_drv.u8_bus_width == BIT_SD_DATA_WIDTH_8) {
		REG_FCIE_SETBIT(reg_emmcpll_0x6a, 2 << 0);
		REG_FCIE_W(reg_emmcpll_0x6b, 0x0213);
	} else {
		return EMMC_ST_ERR_INVALID_PARAM;
	}
	REG_FCIE_SETBIT(reg_emmcpll_0x70, BIT8);
	REG_FCIE_CLRBIT(reg_emmcpll_0x03, 0x0FFF);
	REG_FCIE_W(reg_emmcpll_0x71, 0xF800);

	REG_FCIE_W(reg_emmcpll_0x73, 0xFD00);
	if (emmc_drv.host->dev_comp->mt5896_clk_layout)
		REG_FCIE_CLRBIT(reg_emmcpll_0x74, BIT14);

	#if defined(ENABLE_EMMC_AFIFO) && ENABLE_EMMC_AFIFO
	REG_FCIE_SETBIT(reg_emmcpll_0x70, BIT10 | BIT11);
	REG_FCIE_SETBIT(reg_emmcpll_0x7f, BIT11);
	#endif

	if (emmc_drv.chip_id == MT5879_CHIP_ID)
		REG_FCIE_SETBIT(reg_emmcpll_0x5f, BIT_FLASH_MACRO_TO_FICE);

	return EMMC_ST_SUCCESS;
}

static U32 emmc_pads_switch_hs400(void)
{
	emmc_debug(EMMC_DEBUG_LEVEL_LOW, 0, "HS400\n");

	if (!emmc_drv.u8_emmc_pll_skew4)
		REG_FCIE_SETBIT(reg_emmcpll_0x09, BIT_RXDLL_EN);

	REG_FCIE_SETBIT(reg_emmcpll_0x1a, BIT5 | BIT4);
	REG_FCIE_SETBIT(reg_emmcpll_0x1c, BIT8);

	if (emmc_drv.u8_emmc_pll_skew4) {
		REG_FCIE_SETBIT(reg_emmcpll_0x20, BIT_SEL_INTERNAL_MASK);
	} else {
		REG_FCIE_SETBIT(reg_emmcpll_0x20, BIT_SEL_SKEW4_FOR_CMD);
		REG_FCIE_SETBIT(reg_emmcpll_0x63, BIT_USE_RXDLL);
	}

	//8 bits macro reset + 32 bits macro reset
	REG_FCIE_CLRBIT(reg_emmcpll_0x6f, BIT0 | BIT1);
	REG_FCIE_SETBIT(reg_emmcpll_0x6f, BIT0 | BIT1);//8 bits macro reset + 32 bits macro reset
	REG_FCIE_SETBIT(reg_emmcpll_0x68, BIT0 | BIT1);
	REG_FCIE_SETBIT(FCIE_DDR_MODE, BIT_32BIT_MACRO_EN | BIT_DDR_EN);
	REG_FCIE_SETBIT(reg_emmcpll_0x69, 6 << 4);

	if (emmc_drv.u8_emmc_pll_skew4)
		REG_FCIE_SETBIT(reg_emmcpll_0x69, BIT10 | BIT9);

	if (emmc_drv.u8_bus_width == BIT_SD_DATA_WIDTH_4) {
		REG_FCIE_SETBIT(reg_emmcpll_0x6a, 1 << 0);
		REG_FCIE_W(reg_emmcpll_0x6b, 0x0213);
	} else if (emmc_drv.u8_bus_width == BIT_SD_DATA_WIDTH_8) {
		REG_FCIE_SETBIT(reg_emmcpll_0x6a, 2 << 0);
		REG_FCIE_W(reg_emmcpll_0x6b, 0x0113);
	} else {
		return EMMC_ST_ERR_INVALID_PARAM;
	}
	REG_FCIE_SETBIT(reg_emmcpll_0x70, BIT8);
	REG_FCIE_CLRBIT(reg_emmcpll_0x03, 0x0F0F);
	REG_FCIE_W(reg_emmcpll_0x71, 0xF800);
	REG_FCIE_W(reg_emmcpll_0x73, 0xFD00);
	if (emmc_drv.host->dev_comp->mt5896_clk_layout)
		REG_FCIE_CLRBIT(reg_emmcpll_0x74, BIT14);

	#if defined(ENABLE_EMMC_AFIFO) && ENABLE_EMMC_AFIFO
	REG_FCIE_SETBIT(reg_emmcpll_0x70, BIT10 | BIT11);
	REG_FCIE_SETBIT(reg_emmcpll_0x7f, BIT2 | BIT11);
	#endif

	if (emmc_drv.chip_id == MT5879_CHIP_ID)
		REG_FCIE_SETBIT(reg_emmcpll_0x5f, BIT_FLASH_MACRO_TO_FICE);

	return EMMC_ST_SUCCESS;
}

static U32 emmc_pads_switch_hs400_5_1(void)
{
	emmc_debug(EMMC_DEBUG_LEVEL_LOW, 0, "HS400 5.1\n");

	if (!emmc_drv.u8_emmc_pll_skew4)
		REG_FCIE_SETBIT(reg_emmcpll_0x09, BIT0);

	REG_FCIE_SETBIT(reg_emmcpll_0x1a, BIT5 | BIT4);
	REG_FCIE_SETBIT(reg_emmcpll_0x1c, BIT8);

	if (emmc_drv.u8_emmc_pll_skew4)
		REG_FCIE_SETBIT(reg_emmcpll_0x20, BIT10 | BIT9);
	else
		REG_FCIE_SETBIT(reg_emmcpll_0x63, BIT0);

	//8 bits macro reset + 32 bits macro reset
	REG_FCIE_CLRBIT(reg_emmcpll_0x6f, BIT0 | BIT1);
	REG_FCIE_SETBIT(reg_emmcpll_0x6f, BIT0 | BIT1);
	REG_FCIE_SETBIT(reg_emmcpll_0x68, BIT0 | BIT1);
	REG_FCIE_SETBIT(FCIE_DDR_MODE, BIT_32BIT_MACRO_EN | BIT_DDR_EN);
	REG_FCIE_SETBIT(reg_emmcpll_0x69, 6 << 4);

	if (emmc_drv.u8_emmc_pll_skew4)
		REG_FCIE_SETBIT(reg_emmcpll_0x69, BIT10 | BIT9);

	if (emmc_drv.u8_bus_width == BIT_SD_DATA_WIDTH_4) {
		REG_FCIE_SETBIT(reg_emmcpll_0x6a, 1 << 0);
		REG_FCIE_W(reg_emmcpll_0x6b, 0x0213);
	} else if (emmc_drv.u8_bus_width == BIT_SD_DATA_WIDTH_8) {
		REG_FCIE_SETBIT(reg_emmcpll_0x6a, 2 << 0);
		REG_FCIE_W(reg_emmcpll_0x6b, 0x0113);
	} else {
		return EMMC_ST_ERR_INVALID_PARAM;
	}
	REG_FCIE_SETBIT(reg_emmcpll_0x70, BIT8);
	REG_FCIE_CLRBIT(reg_emmcpll_0x03, 0x0F0F);
	REG_FCIE_W(reg_emmcpll_0x71, 0xF800);
	REG_FCIE_W(reg_emmcpll_0x73, 0xFD00);
	if (emmc_drv.host->dev_comp->mt5896_clk_layout)
		REG_FCIE_CLRBIT(reg_emmcpll_0x74, BIT14);

	REG_FCIE_SETBIT(reg_emmcpll_0x70, BIT10 | BIT11);
	REG_FCIE_SETBIT(reg_emmcpll_0x7f, BIT2 | BIT8 | BIT11);

	if (emmc_drv.chip_id == MT5879_CHIP_ID)
		REG_FCIE_SETBIT(reg_emmcpll_0x5f, BIT_FLASH_MACRO_TO_FICE);

	return EMMC_ST_SUCCESS;
}

U32 emmc_pads_switch(U32 u32_mode)
{
	U32 u32_err = 0;

	emmc_drv.pad_type = u32_mode;
	emmc_pads_switch_default();

	switch (u32_mode) {
	case FCIE_EMMC_BYPASS:
		emmc_debug(EMMC_DEBUG_LEVEL_LOW, 0, "Bypass\n");
		//fcie
		REG_FCIE_SETBIT(FCIE_DDR_MODE, BIT_PAD_IN_SEL_SD | BIT_FALL_LATCH | BIT_CLK2_SEL);
		//emmc_pll
		REG_FCIE_SETBIT(reg_emmcpll_0x1a, BIT10);
		REG_FCIE_W(reg_emmcpll_0x71, 0xFFFF);
		REG_FCIE_W(reg_emmcpll_0x73, 0xFFFF);
	break;

	case FCIE_EMMC_SDR:
		emmc_debug(EMMC_DEBUG_LEVEL_LOW, 0, "SDR\n");
		//emmc_pll
		REG_FCIE_SETBIT(reg_emmcpll_0x1c, BIT9);
		//8 bits macro reset + 32 bits macro reset
		REG_FCIE_CLRBIT(reg_emmcpll_0x6f, BIT0 | BIT1);
		REG_FCIE_SETBIT(reg_emmcpll_0x6f, BIT0 | BIT1);
		REG_FCIE_SETBIT(reg_emmcpll_0x68, BIT0);
		REG_FCIE_SETBIT(FCIE_DDR_MODE, BIT_8BIT_MACRO_EN);
		REG_FCIE_W(reg_emmcpll_0x71, 0xFFFF);
		REG_FCIE_W(reg_emmcpll_0x73, 0xFFFF);
	break;

	case FCIE_EMMC_DDR:
		u32_err = emmc_pads_switch_ddr();
		if (u32_err != EMMC_ST_SUCCESS)
			goto ERROR_HANDLE;
	break;

	case FCIE_EMMC_HS200:
		u32_err = emmc_pads_switch_hs200();
		if (u32_err != EMMC_ST_SUCCESS)
			goto ERROR_HANDLE;
	break;

	case FCIE_EMMC_HS400:
		u32_err = emmc_pads_switch_hs400();
		if (u32_err != EMMC_ST_SUCCESS)
			goto ERROR_HANDLE;
	break;

	case FCIE_EMMC_HS400_AIFO_5_1:
		u32_err = emmc_pads_switch_hs400_5_1();
		if (u32_err != EMMC_ST_SUCCESS)
			goto ERROR_HANDLE;
	break;

	default:
		emmc_debug(1, 1, "eMMC Err: wrong parameter for switch pad func\n");
		return EMMC_ST_ERR_PARAMETER;
	break;
	}

	emmc_drv.u32_drvflag &= (~(DRV_FLAG_SPEED_HIGH | DRV_FLAG_DDR_MODE | DRV_FLAG_SPEED_HS200 | DRV_FLAG_SPEED_HS400));

	if (emmc_drv.pad_type == FCIE_EMMC_SDR)
		emmc_drv.u32_drvflag |= DRV_FLAG_SPEED_HIGH;
	else if (emmc_drv.pad_type == FCIE_EMMC_DDR)
		emmc_drv.u32_drvflag |= DRV_FLAG_DDR_MODE | DRV_FLAG_SPEED_HIGH;
	else if (emmc_drv.pad_type == FCIE_EMMC_HS200)
		emmc_drv.u32_drvflag |= DRV_FLAG_SPEED_HS200;
	else if (emmc_drv.pad_type == FCIE_EMMC_HS400 || emmc_drv.pad_type == FCIE_EMMC_HS400_5_1)
		emmc_drv.u32_drvflag |= DRV_FLAG_DDR_MODE | DRV_FLAG_SPEED_HS400;

	return EMMC_ST_SUCCESS;

ERROR_HANDLE:

	emmc_debug(1, 1, "eMMC Err: set bus width before pad switch\n");
	return EMMC_ST_ERR_INVALID_PARAM;
}

static U32 emmc_pll_setting_m6(U16 u16_clk_param)
{
	U32 u32_value_reg_emmc_pll_pdiv;

	//reset PDIV & LPDIV
	REG_FCIE_W(reg_emmcpll_0x07, 0x0010);
	REG_FCIE_W(reg_emmcpll_0x08, 0x0020);

	//set emmc pll test
	REG_FCIE_W(reg_emmcpll_0x07, 0x0d40);
	REG_FCIE_W(reg_emmcpll_0x08, 0x0090);

	//power on reset
	REG_FCIE_CLRBIT(reg_emmcpll_0x04, BIT6);

	//reset emmc pll
	REG_FCIE_SETBIT(reg_emmcpll_0x06, BIT0);
	REG_FCIE_CLRBIT(reg_emmcpll_0x04, BIT5);

	emmc_hw_timer_delay(HW_TIMER_DELAY_10us);
	switch (u16_clk_param) {
	case EMMC_PLL_CLK_200M://200M
		REG_FCIE_W(reg_ddfset_15_00, 0);
		REG_FCIE_W(reg_ddfset_23_16, 0);
		REG_FCIE_W(reg_ddfset_15_00, 0xffff);
		REG_FCIE_SETBIT(reg_ddfset_23_16, 0x1a);//195MHz
		u32_value_reg_emmc_pll_pdiv = 0;//PostDIV:2
	break;

	case EMMC_PLL_CLK_160M:
		REG_FCIE_W(reg_ddfset_15_00, 0);
		REG_FCIE_W(reg_ddfset_23_16, 0);
		REG_FCIE_SETBIT(reg_ddfset_15_00, 0x6666);
		REG_FCIE_SETBIT(reg_ddfset_23_16, 0x20);
		u32_value_reg_emmc_pll_pdiv = 0;//PostDIV: 2
	break;

	case EMMC_PLL_CLK_140M:
		REG_FCIE_W(reg_ddfset_15_00, 0);
		REG_FCIE_W(reg_ddfset_23_16, 0);
		REG_FCIE_SETBIT(reg_ddfset_15_00, 0x0750);
		REG_FCIE_SETBIT(reg_ddfset_23_16, 0x25);
		u32_value_reg_emmc_pll_pdiv = 0;//PostDIV: 2
	break;

	case EMMC_PLL_CLK_120M:
		REG_FCIE_W(reg_ddfset_15_00, 0);
		REG_FCIE_W(reg_ddfset_23_16, 0);
		REG_FCIE_SETBIT(reg_ddfset_15_00, 0x3333);
		REG_FCIE_SETBIT(reg_ddfset_23_16, 0x2b);
		u32_value_reg_emmc_pll_pdiv = 0;//PostDIV: 2
	break;

	default:
		emmc_debug(0, 0, "eMMC Err: emmc PLL not configed %Xh\n", u16_clk_param);
		emmc_die(" ");

	return EMMC_ST_ERR_UNKNOWN_CLK;
	}
	REG_FCIE_CLRBIT(reg_emmcpll_0x06, BIT0);
	REG_FCIE_W(reg_emmcpll_0x04, 0x0609);
	REG_FCIE_CLRBIT(reg_emmcpll_0x05, BIT2 | BIT1 | BIT0);
	REG_FCIE_SETBIT(reg_emmcpll_0x05, u32_value_reg_emmc_pll_pdiv);

	emmc_hw_timer_delay(5 * HW_TIMER_DELAY_100us);

	REG_FCIE_SETBIT(reg_emmcpll_0x06, BIT0);
	emmc_hw_timer_delay(HW_TIMER_DELAY_10us);
	REG_FCIE_CLRBIT(reg_emmcpll_0x06, BIT0);

	return EMMC_ST_SUCCESS;
}

static U32 emmc_pll_setting_m6l(U16 u16_clk_param)
{
	U32 u32_value_reg_emmc_pll_pdiv;

	//reset PDIV & LPDIV
	REG_FCIE_W(reg_emmcpll_0x07, 0x0010);
	REG_FCIE_W(reg_emmcpll_0x08, 0x0020);

	//set emmc pll test
	REG_FCIE_W(reg_emmcpll_0x07, 0x0a40);
	REG_FCIE_W(reg_emmcpll_0x08, 0);

	//power on reset
	REG_FCIE_CLRBIT(reg_emmcpll_0x04, BIT6);

	//reset emmc pll
	REG_FCIE_SETBIT(reg_emmcpll_0x06, BIT0);
	REG_FCIE_CLRBIT(reg_emmcpll_0x04, BIT5);

	emmc_hw_timer_delay(HW_TIMER_DELAY_10us);
	switch (u16_clk_param) {
	case EMMC_PLL_CLK_200M://200M
		REG_FCIE_W(reg_ddfset_15_00, 0);
		REG_FCIE_W(reg_ddfset_23_16, 0);
		REG_FCIE_W(reg_ddfset_15_00, 0xffff);
		REG_FCIE_SETBIT(reg_ddfset_23_16, 0x1a);//195MHz
		u32_value_reg_emmc_pll_pdiv = 0;//PostDIV: 2
	break;

	case EMMC_PLL_CLK_160M:
		REG_FCIE_W(reg_ddfset_15_00, 0);
		REG_FCIE_W(reg_ddfset_23_16, 0);
		REG_FCIE_SETBIT(reg_ddfset_15_00, 0x6666);
		REG_FCIE_SETBIT(reg_ddfset_23_16, 0x20);
		u32_value_reg_emmc_pll_pdiv = 0;//PostDIV: 2
	break;

	case EMMC_PLL_CLK_140M:
		REG_FCIE_W(reg_ddfset_15_00, 0);
		REG_FCIE_W(reg_ddfset_23_16, 0);
		REG_FCIE_SETBIT(reg_ddfset_15_00, 0x0750);
		REG_FCIE_SETBIT(reg_ddfset_23_16, 0x25);
		u32_value_reg_emmc_pll_pdiv = 0;//PostDIV: 2
	break;

	case EMMC_PLL_CLK_120M:
		REG_FCIE_W(reg_ddfset_15_00, 0);
		REG_FCIE_W(reg_ddfset_23_16, 0);
		REG_FCIE_SETBIT(reg_ddfset_15_00, 0x3333);
		REG_FCIE_SETBIT(reg_ddfset_23_16, 0x2b);
		u32_value_reg_emmc_pll_pdiv = 0;//PostDIV: 2
	break;

	default:
		emmc_debug(0, 0, "eMMC Err: emmc PLL not configed %Xh\n", u16_clk_param);
		emmc_die(" ");
		return EMMC_ST_ERR_UNKNOWN_CLK;
	}
	REG_FCIE_CLRBIT(reg_emmcpll_0x06, BIT0);
	REG_FCIE_W(reg_emmcpll_0x04, 0x0609);
	REG_FCIE_CLRBIT(reg_emmcpll_0x05, BIT2 | BIT1 | BIT0);
	REG_FCIE_SETBIT(reg_emmcpll_0x05, u32_value_reg_emmc_pll_pdiv);
	emmc_hw_timer_delay(5 * HW_TIMER_DELAY_100us);

	REG_FCIE_SETBIT(reg_emmcpll_0x06, BIT0);
	emmc_hw_timer_delay(HW_TIMER_DELAY_10us);
	REG_FCIE_CLRBIT(reg_emmcpll_0x06, BIT0);

	return EMMC_ST_SUCCESS;
}

static U32 emmc_pll_setting_miffy(U16 u16_clk_param)
{
	U32 u32_value_reg_emmc_pll_pdiv;

	//reset PDIV & LPDIV
	REG_FCIE_W(reg_emmcpll_0x07, 0x0100);
	REG_FCIE_W(reg_emmcpll_0x08, 0x0004);

	//set emmc pll test
	REG_FCIE_W(reg_emmcpll_0x07, 0);
	REG_FCIE_W(reg_emmcpll_0x08, 0);

	//power on reset
	REG_FCIE_CLRBIT(reg_emmcpll_0x04, BIT6);

	//reset emmc pll
	REG_FCIE_SETBIT(reg_emmcpll_0x06, BIT0);
	REG_FCIE_CLRBIT(reg_emmcpll_0x04, BIT5);

	emmc_hw_timer_delay(HW_TIMER_DELAY_10us);
	switch (u16_clk_param) {
	case EMMC_PLL_CLK_200M: //200M
		REG_FCIE_W(reg_ddfset_15_00, 0);
		REG_FCIE_W(reg_ddfset_23_16, 0);
		REG_FCIE_SETBIT(reg_ddfset_23_16, 0x24);//195MHz
		u32_value_reg_emmc_pll_pdiv = 0;//PostDIV: 2
	break;

	case EMMC_PLL_CLK_160M:
		REG_FCIE_W(reg_ddfset_15_00, 0);
		REG_FCIE_W(reg_ddfset_23_16, 0);
		REG_FCIE_SETBIT(reg_ddfset_15_00, 0x3333);
		REG_FCIE_SETBIT(reg_ddfset_23_16, 0x2b);
		u32_value_reg_emmc_pll_pdiv = 0;//PostDIV:2
	break;

	case EMMC_PLL_CLK_140M:
		REG_FCIE_W(reg_ddfset_15_00, 0);
		REG_FCIE_W(reg_ddfset_23_16, 0);
		REG_FCIE_SETBIT(reg_ddfset_15_00, 0x5f16);
		REG_FCIE_SETBIT(reg_ddfset_23_16, 0x31);
		u32_value_reg_emmc_pll_pdiv = 0;//PostDIV: 2
	break;

	case EMMC_PLL_CLK_120M:
		REG_FCIE_W(reg_ddfset_15_00, 0);
		REG_FCIE_W(reg_ddfset_23_16, 0);
		REG_FCIE_SETBIT(reg_ddfset_15_00, 0x999a);
		REG_FCIE_SETBIT(reg_ddfset_23_16, 0x39);
		u32_value_reg_emmc_pll_pdiv = 0;//PostDIV: 2
	break;

	default:
		emmc_debug(0, 0, "eMMC Err: emmc PLL not configed %Xh\n", u16_clk_param);
		emmc_die(" ");
		return EMMC_ST_ERR_UNKNOWN_CLK;
	}
	REG_FCIE_CLRBIT(reg_emmcpll_0x06, BIT0);
	REG_FCIE_W(reg_emmcpll_0x04, 0x0404);
	REG_FCIE_CLRBIT(reg_emmcpll_0x05, BIT2 | BIT1 | BIT0);
	REG_FCIE_SETBIT(reg_emmcpll_0x05, u32_value_reg_emmc_pll_pdiv);

	emmc_hw_timer_delay(5 * HW_TIMER_DELAY_100us);

	REG_FCIE_SETBIT(reg_emmcpll_0x06, BIT0);
	emmc_hw_timer_delay(HW_TIMER_DELAY_10us);
	REG_FCIE_CLRBIT(reg_emmcpll_0x06, BIT0);

	return EMMC_ST_SUCCESS;
}

static U32 emmc_pll_setting_m7332(U16 u16_clk_param)
{
	U32 u32_value_reg_emmc_pll_pdiv;

	//1. reset emmc pll
	REG_FCIE_SETBIT(reg_emmc_pll_reset, BIT0);
	REG_FCIE_CLRBIT(reg_emmcpll_fbdiv, BIT5);
	//Wait 10us
	emmc_hw_timer_delay(10);
	REG_FCIE_CLRBIT(reg_emmc_pll_reset, BIT0);

	//2. synth clock
	switch (u16_clk_param) {
	case EMMC_PLL_CLK_200M://200M
		REG_FCIE_W(reg_ddfset_23_16, 0);
		REG_FCIE_W(reg_ddfset_15_00, 0);
		REG_FCIE_SETBIT(reg_ddfset_23_16, 0x24);//195MHz
		REG_FCIE_SETBIT(reg_ddfset_15_00, 0x03D8);
		u32_value_reg_emmc_pll_pdiv = 1;//PostDIV: 2
	break;

	case EMMC_PLL_CLK_160M:
		REG_FCIE_W(reg_ddfset_23_16, 0);
		REG_FCIE_W(reg_ddfset_15_00, 0);
		REG_FCIE_SETBIT(reg_ddfset_23_16, 0x2B);
		REG_FCIE_SETBIT(reg_ddfset_15_00, 0x3333);
		u32_value_reg_emmc_pll_pdiv = 1;//PostDIV: 2
	break;

	case EMMC_PLL_CLK_140M:
		REG_FCIE_W(reg_ddfset_23_16, 0);
		REG_FCIE_W(reg_ddfset_15_00, 0);
		REG_FCIE_SETBIT(reg_ddfset_23_16, 0x31);
		REG_FCIE_SETBIT(reg_ddfset_15_00, 0x5F15);
		u32_value_reg_emmc_pll_pdiv = 1;//PostDIV: 2
	break;

	case EMMC_PLL_CLK_120M:
		REG_FCIE_W(reg_ddfset_23_16, 0);
		REG_FCIE_W(reg_ddfset_15_00, 0);
		REG_FCIE_SETBIT(reg_ddfset_23_16, 0x39);
		REG_FCIE_SETBIT(reg_ddfset_15_00, 0x9999);
		u32_value_reg_emmc_pll_pdiv = 1;//PostDIV: 2
	break;

	case EMMC_PLL_CLK_100M:
		REG_FCIE_W(reg_ddfset_23_16, 0);
		REG_FCIE_W(reg_ddfset_15_00, 0);
		REG_FCIE_SETBIT(reg_ddfset_23_16, 0x45);
		REG_FCIE_SETBIT(reg_ddfset_15_00, 0x1EB8);
		u32_value_reg_emmc_pll_pdiv = 1;//PostDIV: 2
	break;

	case EMMC_PLL_CLK__86M:
		REG_FCIE_W(reg_ddfset_23_16, 0);
		REG_FCIE_W(reg_ddfset_15_00, 0);
		REG_FCIE_SETBIT(reg_ddfset_23_16, 0x28);
		REG_FCIE_SETBIT(reg_ddfset_15_00, 0x2FA0);
		u32_value_reg_emmc_pll_pdiv = 2;//PostDIV: 4
	break;

	case EMMC_PLL_CLK__80M:
		REG_FCIE_W(reg_ddfset_23_16, 0);
		REG_FCIE_W(reg_ddfset_15_00, 0);
		REG_FCIE_SETBIT(reg_ddfset_23_16, 0x2B);
		REG_FCIE_SETBIT(reg_ddfset_15_00, 0x3333);
		u32_value_reg_emmc_pll_pdiv = 2;//PostDIV: 4
	break;

	case EMMC_PLL_CLK__72M:
		REG_FCIE_W(reg_ddfset_23_16, 0);
		REG_FCIE_W(reg_ddfset_15_00, 0);
		REG_FCIE_SETBIT(reg_ddfset_23_16, 0x30);
		REG_FCIE_SETBIT(reg_ddfset_15_00, 0x0000);
		u32_value_reg_emmc_pll_pdiv = 2;//PostDIV: 4
	break;

	case EMMC_PLL_CLK__62M:
		REG_FCIE_W(reg_ddfset_23_16, 0);
		REG_FCIE_W(reg_ddfset_15_00, 0);
		REG_FCIE_SETBIT(reg_ddfset_23_16, 0x37);
		REG_FCIE_SETBIT(reg_ddfset_15_00, 0xBDEF);
		u32_value_reg_emmc_pll_pdiv = 2;//PostDIV: 4
	break;

	case EMMC_PLL_CLK__52M:
		REG_FCIE_W(reg_ddfset_23_16, 0);
		REG_FCIE_W(reg_ddfset_15_00, 0);
		REG_FCIE_SETBIT(reg_ddfset_23_16, 0x42);
		REG_FCIE_SETBIT(reg_ddfset_15_00, 0x7627);
		u32_value_reg_emmc_pll_pdiv = 2;//PostDIV: 4
	break;

	case EMMC_PLL_CLK__48M:
		REG_FCIE_W(reg_ddfset_23_16, 0);
		REG_FCIE_W(reg_ddfset_15_00, 0);
		REG_FCIE_SETBIT(reg_ddfset_23_16, 0x48);
		REG_FCIE_SETBIT(reg_ddfset_15_00, 0x0000);
		u32_value_reg_emmc_pll_pdiv = 2;//PostDIV: 4
	break;

	case EMMC_PLL_CLK__40M:
		REG_FCIE_W(reg_ddfset_23_16, 0);
		REG_FCIE_W(reg_ddfset_15_00, 0);
		REG_FCIE_SETBIT(reg_ddfset_23_16, 0x2B);
		REG_FCIE_SETBIT(reg_ddfset_15_00, 0x3333);
		u32_value_reg_emmc_pll_pdiv = 4;//PostDIV: 8
	break;

	case EMMC_PLL_CLK__36M:
		REG_FCIE_W(reg_ddfset_23_16, 0);
		REG_FCIE_W(reg_ddfset_15_00, 0);
		REG_FCIE_SETBIT(reg_ddfset_23_16, 0x30);
		REG_FCIE_SETBIT(reg_ddfset_15_00, 0x0000);
		u32_value_reg_emmc_pll_pdiv = 4;//PostDIV: 8
	break;

	case EMMC_PLL_CLK__32M:
		REG_FCIE_W(reg_ddfset_23_16, 0);
		REG_FCIE_W(reg_ddfset_15_00, 0);
		REG_FCIE_SETBIT(reg_ddfset_23_16, 0x36);
		REG_FCIE_SETBIT(reg_ddfset_15_00, 0x0000);
		u32_value_reg_emmc_pll_pdiv = 4;//PostDIV: 8
	break;

	case EMMC_PLL_CLK__27M:
		REG_FCIE_W(reg_ddfset_23_16, 0);
		REG_FCIE_W(reg_ddfset_15_00, 0);
		REG_FCIE_SETBIT(reg_ddfset_23_16, 0x40);
		REG_FCIE_SETBIT(reg_ddfset_15_00, 0x0000);
		u32_value_reg_emmc_pll_pdiv = 4;//PostDIV: 8
	break;

	case EMMC_PLL_CLK__20M:
		REG_FCIE_W(reg_ddfset_23_16, 0);
		REG_FCIE_W(reg_ddfset_15_00, 0);
		REG_FCIE_SETBIT(reg_ddfset_23_16, 0x2B);
		REG_FCIE_SETBIT(reg_ddfset_15_00, 0x3333);
		u32_value_reg_emmc_pll_pdiv = 7;//PostDIV: 16
	break;

	default:
		emmc_debug(0, 0, "eMMC Err: emmc PLL not configed %Xh\n", u16_clk_param);
		emmc_die(" ");
		return EMMC_ST_ERR_UNKNOWN_CLK;
	}

	//3. VCO clock ( loop N = 4 )
	REG_FCIE_W(reg_emmcpll_fbdiv, 0);
	REG_FCIE_SETBIT(reg_emmcpll_fbdiv, 0x6);//PostDIV: 8

	//4. 1X clock
	REG_FCIE_CLRBIT(reg_emmcpll_pdiv, BIT2 | BIT1 | BIT0);
	REG_FCIE_SETBIT(reg_emmcpll_pdiv, u32_value_reg_emmc_pll_pdiv);//PostDIV: 8

	if (u16_clk_param == EMMC_PLL_CLK__20M)
		REG_FCIE_SETBIT(reg_emmc_pll_test, BIT10);
	else
		REG_FCIE_CLRBIT(reg_emmc_pll_test, BIT10);

	emmc_hw_timer_delay(HW_TIMER_DELAY_100us); //asked by Irwin

	return EMMC_ST_SUCCESS;
}

U32 emmc_pll_setting(U16 u16_clk_param)
{
	static U16 u16_old_clkparam = 0xFFFF;
	U32 u32_err = EMMC_ST_SUCCESS;
	int chip_id = 0;

	if (u16_clk_param == u16_old_clkparam)
		return EMMC_ST_SUCCESS;

	u16_old_clkparam = u16_clk_param;

	if (emmc_drv.host->dev_comp->mt5896_clk_layout) {
		chip_id = romtbl_get_chip_id_info();
		if (chip_id == MT5896)
			u32_err = emmc_pll_setting_m6(u16_clk_param);
		else if (chip_id == MT5897)
			u32_err = emmc_pll_setting_m6l(u16_clk_param);
		else if (chip_id == MT5876 || chip_id == MT5879)
			u32_err = emmc_pll_setting_miffy(u16_clk_param);
	} else {
		u32_err = emmc_pll_setting_m7332(u16_clk_param);
	}

	return u32_err;
}

void hal_emmc_pll_dll_setting(void)
{
	U16 u16_reg;

	REG_FCIE_CLRBIT(reg_emmcpll_0x09, BIT0);
	REG_FCIE_CLRBIT(REG_EMMC_PLL_RX30, BIT1);
	//Reset eMMC_DLL
	REG_FCIE_SETBIT(REG_EMMC_PLL_RX30, BIT2);
	REG_FCIE_CLRBIT(REG_EMMC_PLL_RX30, BIT2);

	//DLL pulse width and phase
	REG_FCIE_W(REG_EMMC_PLL_RX01, 0x7F72);

	//DLL code
	REG_FCIE_W(REG_EMMC_PLL_RX32, 0xF200);

	//DLL calibration
	REG_FCIE_W(REG_EMMC_PLL_RX30, 0x3378);

	REG_FCIE_SETBIT(REG_EMMC_PLL_RX33, BIT15);

	//Wait 100us
	udelay(250);

	//Get hw dll0 code
	REG_FCIE_R(REG_EMMC_PLL_RX33, u16_reg);

	REG_FCIE_CLRBIT(REG_EMMC_PLL_RX34, (BIT10 - 1));
	//Set dw dll0 code
	REG_FCIE_SETBIT(REG_EMMC_PLL_RX34, u16_reg & 0x03FF);

	//Disable reg_hw_upcode_en
	REG_FCIE_CLRBIT(REG_EMMC_PLL_RX30, BIT8 | BIT9);

	//Clear reg_emmc_dll_test[7]
	REG_FCIE_CLRBIT(reg_emmcpll_0x02, BIT15);

	if (!emmc_drv.u8_emmc_pll_skew4)
		REG_FCIE_SETBIT(reg_emmcpll_0x09, BIT0);
}

static U32 emmc_prepare_clock_setting_m6(U16 u16_clk_param, U16 *u16_clk_param_tmp)
{
	U16 u16_tmp = 0;

	switch (u16_clk_param) {
	case EMMC_PLL_CLK__20M:
		emmc_drv.u32_clk_khz =  PLL_CLK_20M;
	break;
	case EMMC_PLL_CLK__27M:
		emmc_drv.u32_clk_khz =  PLL_CLK_27M;
	break;
	case EMMC_PLL_CLK__32M:
		emmc_drv.u32_clk_khz =  PLL_CLK_32M;
	break;
	case EMMC_PLL_CLK__36M:
		emmc_drv.u32_clk_khz =  PLL_CLK_36M;
	break;
	case EMMC_PLL_CLK__40M:
		emmc_drv.u32_clk_khz =  PLL_CLK_40M;
	break;
	case EMMC_PLL_CLK__48M:
		emmc_drv.u32_clk_khz =  PLL_CLK_48M;
	break;
	case EMMC_PLL_CLK__52M:
		emmc_drv.u32_clk_khz =  PLL_CLK_52M;
	break;
	case EMMC_PLL_CLK__62M:
		emmc_drv.u32_clk_khz =  PLL_CLK_62M;
	break;
	case EMMC_PLL_CLK__72M:
		emmc_drv.u32_clk_khz =  PLL_CLK_72M;
	break;
	case EMMC_PLL_CLK__80M:
		emmc_drv.u32_clk_khz =  PLL_CLK_80M;
	break;
	case EMMC_PLL_CLK__86M:
		emmc_drv.u32_clk_khz =  PLL_CLK_86M;
	break;
	case EMMC_PLL_CLK_100M:
		emmc_drv.u32_clk_khz = PLL_CLK_100M;
	break;
	case EMMC_PLL_CLK_120M:
		emmc_drv.u32_clk_khz = PLL_CLK_120M;
	break;
	case EMMC_PLL_CLK_140M:
		emmc_drv.u32_clk_khz = PLL_CLK_140M;
	break;
	case EMMC_PLL_CLK_160M:
		emmc_drv.u32_clk_khz = PLL_CLK_160M;
	break;
	case EMMC_PLL_CLK_200M:
		emmc_drv.u32_clk_khz = PLL_CLK_200M;
	break;

	//clock_gen fcie clock
	case BIT_CLK_XTAL_12M:
	case BIT_FCIE_CLK_20M:
		*u16_clk_param_tmp = BIT_MT5896_CLK_XTAL_12M;
		emmc_drv.u32_clk_khz =  FCIE_CLK_12M;
	break;
	case BIT_FCIE_CLK_300K:
		*u16_clk_param_tmp = BIT_MT5896_CLK_300K;
		emmc_drv.u32_clk_khz =    FCIE_CLK_300k;
	break;
	case BIT_CLK_XTAL_24M:
	case BIT_FCIE_CLK_32M:
	case BIT_FCIE_CLK_36M:
	case BIT_FCIE_CLK_40M:
	case BIT_FCIE_CLK_43_2M:
		*u16_clk_param_tmp = BIT_MT5896_CLK_24M;
		emmc_drv.u32_clk_khz =  FCIE_CLK_24M;
	break;
	case BIT_FCIE_CLK_48M:
		*u16_clk_param_tmp = BIT_MT5896_CLK_48M;
		emmc_drv.u32_clk_khz =  FCIE_CLK_48M;
	break;
	default:
		emmc_debug(1, 1, "eMMC Err: clkgen %X %Xh\n", u16_clk_param, EMMC_ST_ERR_INVALID_PARAM);
		emmc_die(" ");
		return EMMC_ST_ERR_INVALID_PARAM;
	break;
	}

	//set smi fcie clk
	REG_FCIE_CLRBIT(REG_MT5896_CKG_SMI_FCIE_SEL, BIT_MT5896_CLKGEN_SMI_FCIE_SEL_MASK);
	REG_FCIE_SETBIT(REG_MT5896_CKG_SMI_FCIE_SEL, BIT_MT5896_CKG_SMI_FCIE_SEL_SMI);

	if ((REG_FCIE(REG_MT5896_CKG_FCIE_SYNC) & BIT_MT5896_CKG_FCIE_SYN_MASK) != BIT_MT5896_CKG_FCIE_SYN) {
		REG_FCIE_CLRBIT(REG_MT5896_CKG_FCIE_SYNC, BIT_MT5896_CKG_FCIE_SYN_MASK);
		REG_FCIE_SETBIT(REG_MT5896_CKG_FCIE_SYNC, BIT_MT5896_CKG_FCIE_SYN);
	}
	//set fcie 2 fcie clock sw enable
	REG_FCIE_SETBIT(REG_MT5896_SW_EN_SMI_FCIE2FCIE, BIT_MT5896_SW_ENABLE);

	REG_FCIE_R(REG_MT5896_CKG_FCIE, u16_tmp);
	udelay(7);
	u16_tmp &= ~(BIT_MT5896_FCIE_CLK_GATING | BIT_MT5896_FCIE_CLK_INVERSE | BIT_MT5896_CLKGEN_FCIE_MASK);
	REG_FCIE_W(REG_MT5896_CKG_FCIE, u16_tmp);
	REG_FCIE_CLRBIT(REG_MT5896_CLG_FCIE_TSP,
			BIT_MT5896_FCIE_CLK_TSP_GATING | BIT_MT5896_FCIE_CLK_TSP_INVERSE | BIT_MT5896_CLKGEN_FCIE_TSP_MASK);

	return EMMC_ST_SUCCESS;
}

static U32 emmc_prepare_clock_setting_m7332(U16 u16_clk_param)
{
	U16 u16_tmp = 0;

	switch (u16_clk_param) {
	case EMMC_PLL_CLK__20M:
		emmc_drv.u32_clk_khz =  PLL_CLK_20M;
	break;
	case EMMC_PLL_CLK__27M:
		emmc_drv.u32_clk_khz =  PLL_CLK_27M;
	break;
	case EMMC_PLL_CLK__32M:
		emmc_drv.u32_clk_khz =  PLL_CLK_32M;
	break;
	case EMMC_PLL_CLK__36M:
		emmc_drv.u32_clk_khz =  PLL_CLK_36M;
	break;
	case EMMC_PLL_CLK__40M:
		emmc_drv.u32_clk_khz =  PLL_CLK_40M;
	break;
	case EMMC_PLL_CLK__48M:
		emmc_drv.u32_clk_khz =  PLL_CLK_48M;
	break;
	case EMMC_PLL_CLK__52M:
		emmc_drv.u32_clk_khz =  PLL_CLK_52M;
	break;
	case EMMC_PLL_CLK__62M:
		emmc_drv.u32_clk_khz =  PLL_CLK_62M;
	break;
	case EMMC_PLL_CLK__72M:
		emmc_drv.u32_clk_khz =  PLL_CLK_72M;
	break;
	case EMMC_PLL_CLK__80M:
		emmc_drv.u32_clk_khz =  PLL_CLK_80M;
	break;
	case EMMC_PLL_CLK__86M:
		emmc_drv.u32_clk_khz =  PLL_CLK_86M;
	break;
	case EMMC_PLL_CLK_100M:
		emmc_drv.u32_clk_khz = PLL_CLK_100M;
	break;
	case EMMC_PLL_CLK_120M:
		emmc_drv.u32_clk_khz = PLL_CLK_120M;
	break;
	case EMMC_PLL_CLK_140M:
		emmc_drv.u32_clk_khz = PLL_CLK_140M;
	break;
	case EMMC_PLL_CLK_160M:
		emmc_drv.u32_clk_khz = PLL_CLK_160M;
	break;
	case EMMC_PLL_CLK_200M:
		emmc_drv.u32_clk_khz = PLL_CLK_200M;
	break;
	case BIT_CLK_XTAL_12M:
		emmc_drv.u32_clk_khz =  FCIE_CLK_12M;
	break;
	case BIT_FCIE_CLK_20M:
		emmc_drv.u32_clk_khz =  FCIE_CLK_20M;
	break;
	case BIT_FCIE_CLK_32M:
		emmc_drv.u32_clk_khz =  FCIE_CLK_32M;
	break;
	case BIT_FCIE_CLK_36M:
		emmc_drv.u32_clk_khz =  FCIE_CLK_36M;
	break;
	case BIT_FCIE_CLK_40M:
		emmc_drv.u32_clk_khz =  FCIE_CLK_40M;
	break;
	case BIT_FCIE_CLK_43_2M:
		emmc_drv.u32_clk_khz =  FCIE_CLK_43M;
	break;
	case BIT_FCIE_CLK_300K:
		emmc_drv.u32_clk_khz =  FCIE_CLK_300k;
	break;
	case BIT_CLK_XTAL_24M:
		emmc_drv.u32_clk_khz =  FCIE_CLK_24M;
	break;
	case BIT_FCIE_CLK_48M:
		emmc_drv.u32_clk_khz =  FCIE_CLK_48M;
	break;

	default:
		emmc_debug(1, 1, "eMMC Err: clkgen %Xh\n", EMMC_ST_ERR_INVALID_PARAM);
		emmc_die(" ");
		return EMMC_ST_ERR_INVALID_PARAM;
	break;
	}

	REG_FCIE_SETBIT(REG_CKG_FCIE, BIT7);//sel clk miu
	REG_FCIE_R(reg_ckg_fcie_syn, u16_tmp);
	if ((u16_tmp & (BIT0 | BIT1)) != BIT0) {
		REG_FCIE_CLRBIT(reg_ckg_fcie_syn, BIT0 | BIT1);
		REG_FCIE_SETBIT(reg_ckg_fcie_syn, BIT0);
	}

	return EMMC_ST_SUCCESS;
}

static void emmc_set_pll_clock_setting_m7332(U16 u16_clk_param)
{
	U16 u16_tmp = 0;

	//emmc_debug(0,0,"eMMC PLL: %Xh\n", u16_ClkParam);
	REG_FCIE_CLRBIT(REG_CKG_FCIE, BIT_FCIE_CLK_SRC_SEL);
	REG_FCIE_R(REG_CKG_FCIE, u16_tmp);
	udelay(7);
	u16_tmp &= ~(BIT_FCIE_CLK_GATING | BIT_FCIE_CLK_INVERSE | BIT_CLKGEN_FCIE_MASK);
	REG_FCIE_W(REG_CKG_FCIE, u16_tmp);
	emmc_pll_setting(u16_clk_param);

	if (emmc_drv.u32_drvflag & DRV_FLAG_SPEED_HS400) {
		REG_FCIE_SETBIT(REG_CKG_FCIE, BIT_FCIE_CLK_EMMC_PLL_2X << 2);
		REG_FCIE_SETBIT(REG_CKG_FCIE, BIT_FCIE_CLK_SRC_SEL);
		hal_emmc_pll_dll_setting();//tuning DLL setting
	} else if (emmc_drv.u32_drvflag & DRV_FLAG_SPEED_HS200) {
		REG_FCIE_SETBIT(REG_CKG_FCIE, BIT_FCIE_CLK_EMMC_PLL_1X << 2);
		REG_FCIE_SETBIT(REG_CKG_FCIE, BIT_FCIE_CLK_SRC_SEL);
	} else if (emmc_drv.u32_drvflag & DRV_FLAG_DDR_MODE) {
		REG_FCIE_SETBIT(REG_CKG_FCIE, BIT_FCIE_CLK_EMMC_PLL_1X << 2);
		REG_FCIE_SETBIT(REG_CKG_FCIE, BIT_FCIE_CLK_SRC_SEL);
		emmc_drv.u32_clk_khz >>= 2;
	} else if (emmc_drv.u32_drvflag & DRV_FLAG_SPEED_HIGH) {
		REG_FCIE_SETBIT(REG_CKG_FCIE, BIT_FCIE_CLK_EMMC_PLL_1X << 2);
		REG_FCIE_SETBIT(REG_CKG_FCIE, BIT_FCIE_CLK_SRC_SEL);
		emmc_drv.u32_clk_khz >>= 2;
	}
}

static U32 emmc_prepare_clock_setting_framework(U16 u16_clk_param)
{
	switch (u16_clk_param) {
	case EMMC_PLL_CLK__20M:
		emmc_drv.u32_clk_khz =  PLL_CLK_20M;
	break;
	case EMMC_PLL_CLK__27M:
		emmc_drv.u32_clk_khz =  PLL_CLK_27M;
	break;
	case EMMC_PLL_CLK__32M:
		emmc_drv.u32_clk_khz =  PLL_CLK_32M;
	break;
	case EMMC_PLL_CLK__36M:
		emmc_drv.u32_clk_khz =  PLL_CLK_36M;
	break;
	case EMMC_PLL_CLK__40M:
		emmc_drv.u32_clk_khz =  PLL_CLK_40M;
	break;
	case EMMC_PLL_CLK__48M:
		emmc_drv.u32_clk_khz =  PLL_CLK_48M;
	break;
	case EMMC_PLL_CLK__52M:
		emmc_drv.u32_clk_khz =  PLL_CLK_52M;
	break;
	case EMMC_PLL_CLK__62M:
		emmc_drv.u32_clk_khz =  PLL_CLK_62M;
	break;
	case EMMC_PLL_CLK__72M:
		emmc_drv.u32_clk_khz =  PLL_CLK_72M;
	break;
	case EMMC_PLL_CLK__80M:
		emmc_drv.u32_clk_khz =  PLL_CLK_80M;
	break;
	case EMMC_PLL_CLK__86M:
		emmc_drv.u32_clk_khz =  PLL_CLK_86M;
	break;
	case EMMC_PLL_CLK_100M:
		emmc_drv.u32_clk_khz = PLL_CLK_100M;
	break;
	case EMMC_PLL_CLK_120M:
		emmc_drv.u32_clk_khz = PLL_CLK_120M;
	break;
	case EMMC_PLL_CLK_140M:
		emmc_drv.u32_clk_khz = PLL_CLK_140M;
	break;
	case EMMC_PLL_CLK_160M:
		emmc_drv.u32_clk_khz = PLL_CLK_160M;
	break;
	case EMMC_PLL_CLK_200M:
		emmc_drv.u32_clk_khz = PLL_CLK_200M;
	break;
	case BIT_CLK_XTAL_12M:
		emmc_drv.u32_clk_khz =  FCIE_CLK_12M;
	break;
	case BIT_FCIE_CLK_20M:
		emmc_drv.u32_clk_khz =  FCIE_CLK_20M;
	break;
	case BIT_FCIE_CLK_32M:
		emmc_drv.u32_clk_khz =  FCIE_CLK_32M;
	break;
	case BIT_FCIE_CLK_36M:
		emmc_drv.u32_clk_khz =  FCIE_CLK_36M;
	break;
	case BIT_FCIE_CLK_40M:
		emmc_drv.u32_clk_khz =  FCIE_CLK_40M;
	break;
	case BIT_FCIE_CLK_43_2M:
		emmc_drv.u32_clk_khz =  FCIE_CLK_43M;
	break;
	case BIT_FCIE_CLK_300K:
		emmc_drv.u32_clk_khz = FCIE_CLK_300k;
	break;
	case BIT_CLK_XTAL_24M:
		emmc_drv.u32_clk_khz =  FCIE_CLK_24M;
	break;
	case BIT_FCIE_CLK_48M:
		emmc_drv.u32_clk_khz =  FCIE_CLK_48M;
	break;

	default:
		emmc_debug(1, 1, "eMMC Err: clkgen %Xh\n", EMMC_ST_ERR_INVALID_PARAM);
		emmc_die(" ");
		return EMMC_ST_ERR_INVALID_PARAM;
	break;
	}

	return EMMC_ST_SUCCESS;
}

static void emmc_set_pll_clock_setting_framework(U16 u16_clk_param)
{
	emmc_pll_setting(u16_clk_param);
	if (emmc_drv.u32_drvflag & DRV_FLAG_SPEED_HS400) {
		hal_emmc_pll_dll_setting();//tuning DLL setting
		REG_FCIE_CLRBIT(emmc_drv.host->fcieclkreg, emmc_drv.host->clk_mask);
		REG_FCIE_SETBIT(emmc_drv.host->fcieclkreg, emmc_drv.host->clk_2xp <<
				emmc_drv.host->clk_shift);
	} else if (emmc_drv.u32_drvflag & DRV_FLAG_SPEED_HS200) {
		REG_FCIE_CLRBIT(emmc_drv.host->fcieclkreg, emmc_drv.host->clk_mask);
		REG_FCIE_SETBIT(emmc_drv.host->fcieclkreg, emmc_drv.host->clk_1xp <<
			emmc_drv.host->clk_shift);
	} else if (emmc_drv.u32_drvflag & DRV_FLAG_DDR_MODE) {
		REG_FCIE_SETBIT(emmc_drv.host->fcieclkreg, emmc_drv.host->clk_1xp <<
				emmc_drv.host->clk_shift);
		emmc_drv.u32_clk_khz >>= 2;
	} else if (emmc_drv.u32_drvflag & DRV_FLAG_SPEED_HIGH) {
		REG_FCIE_CLRBIT(emmc_drv.host->fcieclkreg, emmc_drv.host->clk_mask);
		REG_FCIE_SETBIT(emmc_drv.host->fcieclkreg, emmc_drv.host->clk_1xp <<
				emmc_drv.host->clk_shift);
		emmc_drv.u32_clk_khz >>= 2;
	}
}

U32 emmc_clock_setting(U16 u16_clk_param)
{
	U16 u16_tmp = 0;
	U16 u16_clk_param_tmp = 0;
	U32 u32_err = EMMC_ST_SUCCESS;

	if (emmc_drv.host->no_clk_framework) {
		emmc_platform_reset_pre();

		REG_FCIE_CLRBIT(FCIE_SD_MODE, BIT_SD_CLK_EN);
		if (emmc_drv.host->dev_comp->mt5896_clk_layout) {
			//emmc_debug(0, 1, "eMMC : clkparam %X\n", u16_ClkParam);
			u32_err = emmc_prepare_clock_setting_m6(u16_clk_param, &u16_clk_param_tmp);
			if (u32_err != EMMC_ST_SUCCESS)
				return u32_err;

			if (u16_clk_param & EMMC_PLL_FLAG) {
				//emmc_debug(0,0,"eMMC PLL: %Xh\n", u16_ClkParam);
				emmc_pll_setting(u16_clk_param);

				if (emmc_drv.u32_drvflag & DRV_FLAG_SPEED_HS400) {
					hal_emmc_pll_dll_setting();//tuning DLL setting
					REG_FCIE_SETBIT(REG_MT5896_CLG_FCIE_TSP, BIT_MT5896_FCIE_CLK_EMMC_PLL_2X);
				} else if (emmc_drv.u32_drvflag & DRV_FLAG_SPEED_HS200) {
					REG_FCIE_SETBIT(REG_MT5896_CLG_FCIE_TSP, BIT_MT5896_FCIE_CLK_EMMC_PLL_1X);
				} else {
					REG_FCIE_SETBIT(REG_MT5896_CLG_FCIE_TSP, BIT_MT5896_FCIE_CLK_EMMC_PLL_1X);
					emmc_drv.u32_clk_khz >>= 2;
				}
			} else {
				//emmc_debug(0,0,"eMMC CLKGEN: %Xh\n", u16_ClkParam);
				REG_FCIE_SETBIT(REG_MT5896_CKG_FCIE, u16_clk_param_tmp << 2);
			}
		} else {
			u32_err = emmc_prepare_clock_setting_m7332(u16_clk_param);
			if (u32_err != EMMC_ST_SUCCESS)
				return u32_err;

			if (u16_clk_param & EMMC_PLL_FLAG) {
				emmc_set_pll_clock_setting_m7332(u16_clk_param);
			} else {
				//emmc_debug(0,0,"eMMC CLKGEN: %Xh\n", u16_ClkParam);
				REG_FCIE_CLRBIT(REG_CKG_FCIE, BIT_FCIE_CLK_SRC_SEL);
				REG_FCIE_R(REG_CKG_FCIE, u16_tmp);
				udelay(7);
				u16_tmp &= ~(BIT_FCIE_CLK_GATING | BIT_FCIE_CLK_INVERSE | BIT_CLKGEN_FCIE_MASK);
				REG_FCIE_W(REG_CKG_FCIE, u16_tmp);
				REG_FCIE_SETBIT(REG_CKG_FCIE, u16_clk_param << 2);
				REG_FCIE_SETBIT(REG_CKG_FCIE, BIT_FCIE_CLK_SRC_SEL);
			}
		}
	} else {
		//call clock apis for
		REG_FCIE_CLRBIT(FCIE_SD_MODE, BIT_SD_CLK_EN);
		u32_err = emmc_prepare_clock_setting_framework(u16_clk_param);
		if (u32_err != EMMC_ST_SUCCESS)
			return u32_err;

		if (u16_clk_param & EMMC_PLL_FLAG)
			emmc_set_pll_clock_setting_framework(u16_clk_param);
		else
			REG_FCIE_CLRBIT(emmc_drv.host->fcieclkreg, emmc_drv.host->clk_mask);
	}
	emmc_drv.u16_clk_reg_val = (U16)u16_clk_param;
	return EMMC_ST_SUCCESS;
}

U32 emmc_clock_gating(void)
{
	emmc_platform_reset_pre();
	emmc_drv.u32_clk_khz = 0;
	REG_FCIE_CLRBIT(FCIE_SD_MODE, BIT_SD_CLK_EN);
	emmc_platform_reset_post();

	return EMMC_ST_SUCCESS;
}

U8 gau8_fcie_clk_sel[EMMC_FCIE_VALID_CLK_CNT] = {
	BIT_FCIE_CLK_48M,
	BIT_FCIE_CLK_36M,
	BIT_FCIE_CLK_20M,
};

U32 emmc_platform_reset_pre(void)
{
	return EMMC_ST_SUCCESS;
}

U32 emmc_platform_reset_post(void)
{
	return EMMC_ST_SUCCESS;
}

U32 emmc_platform_init(void)
{
	#if 1
	//for eMMC 4.5 HS200 need 1.8V, unify all eMMC IO power to 1.8V
	//works both for eMMC 4.4 & 4.5
	//emmc_debug(0,0,"1.8V IO power for eMMC\n");
	//Irwin Tyan: set this bit to boost IO performance at low power supply.
	if ((REG_FCIE(reg_emmc_test) & BIT0) == 0) {
		//emmc_debug(0, 0, "eMMC Err: not 1.8V IO setting\n");
		REG_FCIE_SETBIT(reg_emmc_test, BIT0);//1.8V must set this bit
		REG_FCIE_SETBIT(reg_emmc_test, BIT2); //atop patch
	}
	REG_FCIE_W(reg_emmcpll_0x45, 0);//1.8V must set this bit
	REG_FCIE_SETBIT(reg_emmcpll_0x45, emmc_drv.host->host_driving);//1.8V must set this bit
	#else
	emmc_debug(0, 0, "3.3V IO power for eMMC\n");
	REG_FCIE_CLRBIT(reg_emmc_test, BIT0);//3.3V must clear this bit
	#endif

	emmc_pads_switch(EMMC_DEFO_SPEED_MODE);
	emmc_clock_setting(FCIE_SLOWEST_CLK);

	return EMMC_ST_SUCCESS;
}
#endif
