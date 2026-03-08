// SPDX-License-Identifier: GPL-2.0
/*
 * MediaTek DTV eMMC Interface driver
 *
 * Copyright (C) 2019 MediaTek Inc.
 * Author: Tse-Wei Wang <tse-wei.wang@mediatek.com>
 */
 #ifndef __mtk_fcie_mmc__
#define __mtk_fcie_mmc__



 /* FCIE Register offset */
 struct mtk_fcie_regs {
	 u32 fcie_mie_event;
	 u32 fcie_mie_event_int;
	 u32 fcie_mma_pri;
	 u32 fcie_miu_dma_addr_15_0;
	 u32 fcie_miu_dma_addr_31_16;
	 u32 fcie_miu_dma_len_15_0;
	 u32 fcie_miu_dma_len_31_16;
	 u32 fcie_mie_func_ctrl;
	 u32 fcie_job_bl_cnt;		/*0x8*/
	 u32 fcie_blk_size;
	 u32 fcie_cmd_rsp_size;
	 u32 fcie_sd_mode;
	 u32 fcie_sd_ctrl;
	 u32 fcie_sd_status;
	 u32 fcie_boot_cfg;
	 u32 fcie_ddr_mode;			/*0xF*/
	 u32 fcie_sw_rsrv;			/*0x10*/
	 u32 fcie_sdio_mod;
	 u32 fcie_rsp_shift_cnt;
	 u32 fcie_rx_shift_cnt;
	 u32 fcie_zdec_ctl0;
	 u32 fcie_test_mode;
	 u32 fcie_mma_bank_size;
	 u32 fcie_wr_sbit_timer;
	 u32 fcie_rd_sbit_timer;	/*0x18*/
	 u32 reserved0[7];
	 u32 fcie_cmd_fifo[9];		/*0x20~0x28*/
	 u32 fcie_miu_dma_addr_47_32;/*0x29*/
	 u32 reserved1[6];
	 u32 fcie_cifd_event;		/*0x30*/
	 u32 fcie_cifd_int_en;
	 u32 reserved2[2];
	 u32 fcie_pwr_rd_mask;
	 u32 fcie_pwr_save_ctl;
	 u32 fcie_bist;
	 u32 fcie_boot;
	 u32 fcie_emmc_debug_bus0;
	 u32 fcie_emmc_debug_bus1;
	 u32 reserved3[5];
	 u32 fcie_rst;

 };

 struct mtk_emmcpll_regs {
	 u32 emmcpll_rx[0x80];
 };

struct mtk_generalriu_regs {
	u32 rx[0x80];
};

struct mtk_fcie_compatible {
	bool analog_skew4;
	bool digital_skew4;
	bool skip_identify;
	bool power_saving_mode;
	u16 analog_skew4_reg_offset;
	u8 analog_skew4_reg_bit;
	u16 digital_skew4_reg_offset;
	u8 digital_skew4_reg_bit;
	bool emmcpllbase_t_tablebase;
	bool mt5896_clk_layout;
};


struct mtk_fcie_plat {
	struct mmc_config cfg;
	struct mmc mmc;
};

struct mtk_fcie_cid_driving {
	u8 cid[10];
	u8 device_driving;
};

struct mtk_fcie_host {
	struct mtk_fcie_regs *base;
	struct mtk_generalriu_regs *psmbase;
	struct mtk_emmcpll_regs *emmcpllbase;
	struct mtk_generalriu_regs *funcmuxbase;
	struct mtk_generalriu_regs *clkgen;
	struct mtk_generalriu_regs *pm_sleep;
	struct mtk_generalriu_regs *clkgen2;
	struct mtk_generalriu_regs *fciebase;
	struct mtk_generalriu_regs *fde;
	void __iomem *riubase;
	void __iomem *fcieclkreg;
	struct mmc *mmc;
	struct clk fcie_syn_clk;
	struct clk fcie_top_clk;

	struct mtk_fcie_compatible *dev_comp;

	u8 no_clk_framework;
	u8 no_fde;
	/*fcie register variable*/
	u16 host_sd_mode;
	/* operation timeout clocks */
	u32 timeout_ns;
	u32 timeout_clks;

	uint last_resp_type;
	uint last_data_write;
	u8 if_sector_mode;

	u8 ecsd_196;
	u32 sec_count;
	/* for set ios*/
	enum bus_mode timing;
	uint host_clk;
	enum mmc_voltage voltage;

	u64	miu0_base;
	/*DTS defines*/
	u32	clk_shift;
	u32	clk_mask;
	u32	clk_1xp;
	u32	clk_2xp;
	/*DTS defined cids & driving strength*/
	u32 host_driving;
	u32 dts_cids_count;
	struct mtk_fcie_cid_driving *cid_drivings;

	u32 id_matched_driving;

	/*fde enable*/
	u8 fde_enc;
	struct list_head crypto_white_list;
	#ifdef ENABLE_FCIE_MIU_CHECKSUM
	u32 start_blk;
	u32 blks;
	char  *dst_buf;
	#endif
};

/**
 * struct emmc_crypto_disable_info - eMMC crypto disable info
 * @list: list headed by host->crypto_disable_list_head
 * @lba: logical block address
 * @length: transfer lenth in block
 */
struct emmc_crypto_disable_info {
    struct list_head list;
    u64 lba;
    u64 length;
};

#define FDE_CACHE_LINE                 0x10    // [FIXME]
#define EMMC_MAX_RETRY_CNT             0x5
#define DRV_PROBE_FLAG                 1
//#define EN_POWER_CUT_AUTO_TEST         1


#define CMD_R1B_RESPONSE  0x900
#define CMD_R1_RESPONSE   0x900
#define HW_TIMER_DELAY_1S 1000000
#define READ_TIMEOUT      5000000

#define MAX_TIME_WAIT_N_BLK_END_MASK (BIT(32) - 1)

#endif

