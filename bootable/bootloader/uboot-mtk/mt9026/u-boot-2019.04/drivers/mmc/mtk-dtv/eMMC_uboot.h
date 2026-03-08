/* SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause) */
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

#ifndef __EMMC_UBOOT__
#define __EMMC_UBOOT__

#include <common.h>
#include <malloc.h>
#include <linux/string.h>

#include <config.h>
#include <command.h>
#include <mmc.h>
#include <part.h>
#include <malloc.h>
#include <errno.h>
#include <watchdog.h>
#include <stdbool.h>
#include <asm/gpio.h>
#include <dm/pinctrl.h>
#include <linux/bitops.h>
#include <linux/io.h>
#include <linux/iopoll.h>
#include "../mmc_private.h"
#include <memalign.h>
#include <clk.h>
#include <common.h>
#include <dm.h>
#include <mmc.h>
#include <part.h>



//=====================================================

//=====================================================
//[FIXME] -->
#define EMMC_CACHE_LINE                 0x40    // [FIXME]

#define EMMC_PACK1                      __attribute__((__packed__))
#define EMMC_ALIGN1                     __aligned(EMMC_CACHE_LINE)
// <-- [FIXME]

//=====================================================
// HW registers
//=====================================================
#define REG_OFFSET_SHIFT_BITS           2

#define REG_FCIE_U16(Reg_Addr)          (*(volatile U16*)(Reg_Addr))
#define GET_REG_ADDR(x, y)              ((x)+((y) << REG_OFFSET_SHIFT_BITS))

#define REG_FCIE(reg_addr)              readw(reg_addr)
#define REG_FCIE_W(reg_addr, val)       writew((val), reg_addr)
#define REG_FCIE_R(reg_addr, val)       (val) = readw(reg_addr)
#define REG_FCIE_SETBIT(reg_addr, val)  setbits_le16(reg_addr, val)
#define REG_FCIE_CLRBIT(reg_addr, val)  clrbits_le16(reg_addr, val)
#define REG_FCIE_W1C(reg_addr, val)     REG_FCIE_W(reg_addr, REG_FCIE(reg_addr)&(val))

//------------------------------
#define RIU_PM_BASE                     0x1F000000
#define RIU_BASE                        ((unsigned long)emmc_drv.host->riubase)//0x1F200000
#define GPIO_RIU_BASE_ADDRESS           0x1F600000

#define REG_BANK_FCIE0                  0x8980
#define REG_BANK_FCIE1                  0x8A00
#define REG_BANK_FCIE2                  0x8A80

#define FCIE0_BASE                      ((unsigned long)emmc_drv.host->base)
#define FCIE1_BASE                      (((unsigned long)emmc_drv.host->base) + 0x200)
#define FCIE2_BASE                      ((unsigned long)emmc_drv.host->psmbase)

#define FCIE_REG_BASE_ADDR              GET_REG_ADDR(FCIE0_BASE, 0x00)
#define FCIE_CMDFIFO_BASE_ADDR          GET_REG_ADDR(FCIE0_BASE, 0x20)
#define FCIE_CIFD_BASE_ADDR             GET_REG_ADDR(FCIE1_BASE, 0x00)

#define FCIE_NC_WBUF_CIFD_BASE          GET_REG_ADDR(FCIE1_BASE, 0x00)
#define FCIE_NC_RBUF_CIFD_BASE          GET_REG_ADDR(FCIE1_BASE, 0x20)

#define FCIE_POWEER_SAVE_MODE_BASE      GET_REG_ADDR(FCIE2_BASE, 0x00)

#define FDE_REG_BASE_ADDR               ((unsigned long)emmc_drv.host->fde)

#include "eMMC_reg_v5.h"

//--------------------------------bdma address------------------------------------
/* BDMA */
#define BDMA_BASE                     0x1c603000


#define REG_BDMA_CFG                  GET_REG_ADDR(BDMA_BASE, 0x00)
#define reg_vl_triger                 (BIT(0))
#define reg_val_src_a_h            (BIT(10) | BIT(11))
#define reg_val_dst_a_h            (BIT(12) | BIT(13))
#define reg_val_src_a_h_shift       BIT(10)
#define reg_val_dst_a_h_shift       BIT(12)


#define REG_BDMA_OFS_DIRECTION     GET_REG_ADDR(BDMA_BASE, 0x02)
#define REG_VAL_DAM_TO_DRAM        (0x4040)
#define REG_BDMA_OFS_SRC_L                      GET_REG_ADDR(BDMA_BASE, 0x04)
#define REG_BDMA_OFS_SRC_H                      GET_REG_ADDR(BDMA_BASE, 0x05)
#define REG_BDMA_OFS_DST_L                      GET_REG_ADDR(BDMA_BASE, 0x06)
#define REG_BDMA_OFS_DST_H                      GET_REG_ADDR(BDMA_BASE, 0x07)
#define REG_BDMA_OFS_SZ_L                       GET_REG_ADDR(BDMA_BASE, 0x08)
#define REG_BDMA_OFS_SZ_H                       GET_REG_ADDR(BDMA_BASE, 0x09)

#define BDMA_DELAY                      (20)
#define BDMA_TIMEOUT                    (10000)

#define BUFF_BOUNDARY                   (BIT(32))
#define bdma_pa_32_shift               32
#define bdma_pa_16_shift               16

//--------------------------------FCIE1 address------------------------------------

#define REG_FCIE1_0x50          GET_REG_ADDR(FCIE1_BASE, 0x50)
#define REG_FCIE1_0x51          GET_REG_ADDR(FCIE1_BASE, 0x51)

#define BIT_FCIE_EN_CHKSUM              9
#define BIT_FCIE_EN_CHKSUM_SHIFT        8
#define BIT_FCIE_CHKSUM_DATA_SHIFT      15
#define FCIE_CHKSUM_DATA_START          0x10
#define FCIE_CHKSUM_DATA_END            0x1F
#define BIT_FCIE_CHKSUM_DATA_MASK       0xFFFF
#define FCIE_CHKSUM_SIZE                8

//--------------------------------clock gen------------------------------------
#define CLKGEN0_BASE                    ((unsigned long)emmc_drv.host->clkgen)

#define REG_CKG_FCIE                    GET_REG_ADDR(CLKGEN0_BASE, 0x64)
#define BIT_FCIE_CLK_GATING             BIT0
#define BIT_FCIE_CLK_INVERSE            BIT1
#define BIT_CLKGEN_FCIE_MASK            (BIT5|BIT4|BIT3|BIT2)
#define BIT_FCIE_CLK_SRC_SEL            BIT6 // 0: clk_xtal 12M, 1: clk_nfie_p1

//--------------------------------clock gen2------------------------------------
#define CLKGEN2_BASE                    ((unsigned long)emmc_drv.host->clkgen2)
#define reg_ckg_fcie_syn                GET_REG_ADDR(CLKGEN2_BASE, 0x0C)

//--------------------------------gpio function mux--------------------------------------
#define REG_BANK_GPIO_MUX               0x11480  // (0x3229 - 0x3000) x 80h
#define PAD_GPIO_MUX_BASE               ((unsigned long)emmc_drv.host->funcmuxbase)
#define PAD_CHIPTOP_BASE                GET_REG_ADDR(GPIO_RIU_BASE_ADDRESS, REG_BANK_GPIO_MUX)


#define reg_gpio_mux_0x30               GET_REG_ADDR(PAD_GPIO_MUX_BASE, 0x30)
#define BIT_EMMC_CONFIG_EMMC_MODE_1     BIT0
#define BIT_reg_emmc_rstz_en            BIT4


#define reg_nand_mode                   GET_REG_ADDR(PAD_GPIO_MUX_BASE, 0x32)
#define BIT_NAND_MODE                   (BIT1|BIT0)

#define reg_gpio_mux_0x70               GET_REG_ADDR(PAD_GPIO_MUX_BASE, 0x70)
#define BIT_ALL_PAD_IN                  BIT0

//--------------------------------emmc pll--------------------------------------
#define EMMC_PLL_BASE                   ((unsigned long)emmc_drv.host->emmcpllbase)

#define REG_EMMC_PLL_RX01               GET_REG_ADDR(EMMC_PLL_BASE, 0x01)
#define reg_emmcpll_0x02                GET_REG_ADDR(EMMC_PLL_BASE, 0x02)

#define reg_emmcpll_0x03                GET_REG_ADDR(EMMC_PLL_BASE, 0x03)
#define BIT_SKEW1_MASK                  (BIT3|BIT2|BIT1|BIT0)
#define BIT_CLK_PH_MASK                 BIT_SKEW1_MASK
#define BIT_SKEW2_MASK                  (BIT7|BIT6|BIT5|BIT4)
#define BIT_DEFAULT_SKEW2               (BIT6|BIT4)         //5
#define BIT_SKEW2_SHIFT                 4
#define BIT_SKEW3_MASK                  (BIT11|BIT10|BIT9|BIT8)
#define BIT_SKEW4_MASK                  (BIT15|BIT14|BIT13|BIT12)
#define BIT_SKEW4_SHIFT                 12


#define reg_emmcpll_fbdiv               GET_REG_ADDR(EMMC_PLL_BASE, 0x04)
#define reg_emmcpll_pdiv                GET_REG_ADDR(EMMC_PLL_BASE, 0x05)
#define reg_emmc_pll_reset              GET_REG_ADDR(EMMC_PLL_BASE, 0x06)
#define reg_emmc_pll_test               GET_REG_ADDR(EMMC_PLL_BASE, 0x07)

#define reg_emmcpll_0x04                GET_REG_ADDR(EMMC_PLL_BASE, 0x04)
#define reg_emmcpll_0x05                GET_REG_ADDR(EMMC_PLL_BASE, 0x05)
#define reg_emmcpll_0x06                GET_REG_ADDR(EMMC_PLL_BASE, 0x06)
#define reg_emmcpll_0x07                GET_REG_ADDR(EMMC_PLL_BASE, 0x07)
#define reg_emmcpll_0x08                GET_REG_ADDR(EMMC_PLL_BASE, 0x08)
#define reg_emmcpll_0x09                GET_REG_ADDR(EMMC_PLL_BASE, 0x09)
#define BIT_RXDLL_EN                    BIT0
#define BIT_RXDLL_MASK                  (BIT16-1)


#define reg_ddfset_15_00                GET_REG_ADDR(EMMC_PLL_BASE, 0x18)
#define reg_ddfset_23_16                GET_REG_ADDR(EMMC_PLL_BASE, 0x19)
#define reg_emmc_test                   GET_REG_ADDR(EMMC_PLL_BASE, 0x1A)

#define reg_atop_patch                  GET_REG_ADDR(EMMC_PLL_BASE, 0x1C)
#define BIT_HS200_PATCH                 BIT0
#define BIT_HS_RSP_META_PATCH_HW        BIT2
#define BIT_HS_D0_META_PATCH_HW         BIT4
#define BIT_HS_DIN0_PATCH               BIT5
#define BIT_HS_EMMC_DQS_PATCH           BIT6
#define BIT_HS_RSP_MASK_PATCH           BIT7
#define BIT_DDR_RSP_PATCH               BIT8
#define BIT_ATOP_PATCH_MASK             (BIT0|BIT1|BIT2|BIT4|BIT5|BIT6|BIT7|BIT8)

#define reg_emmcpll_0x1a                GET_REG_ADDR(EMMC_PLL_BASE, 0x1a)
#define reg_emmcpll_0x1c                GET_REG_ADDR(EMMC_PLL_BASE, 0x1c)
#define reg_emmcpll_0x1d                GET_REG_ADDR(EMMC_PLL_BASE, 0x1d)



#define reg_emmcpll_0x1e                GET_REG_ADDR(EMMC_PLL_BASE, 0x1e)
#define reg_emmcpll_0x1f                GET_REG_ADDR(EMMC_PLL_BASE, 0x1f)
#define reg_emmcpll_0x20                GET_REG_ADDR(EMMC_PLL_BASE, 0x20)
#define BIT_SEL_INTERNAL_MASK          (BIT10|BIT9)
#define BIT_SEL_SKEW4_FOR_RXDLL        BIT9 
#define BIT_SEL_SKEW4_FOR_CMD          BIT10

#define REG_EMMC_PLL_RX30               GET_REG_ADDR(EMMC_PLL_BASE, 0x30)
#define REG_EMMC_PLL_RX32               GET_REG_ADDR(EMMC_PLL_BASE, 0x32)
#define REG_EMMC_PLL_RX33               GET_REG_ADDR(EMMC_PLL_BASE, 0x33)
#define REG_EMMC_PLL_RX34               GET_REG_ADDR(EMMC_PLL_BASE, 0x34)

#define reg_emmcpll_0x45                GET_REG_ADDR(EMMC_PLL_BASE, 0x45)
#define reg_emmcpll_0x47                GET_REG_ADDR(EMMC_PLL_BASE, 0x47)
#define reg_emmcpll_0x48                GET_REG_ADDR(EMMC_PLL_BASE, 0x48)
#define reg_emmcpll_0x4a                GET_REG_ADDR(EMMC_PLL_BASE, 0x4a)
#define reg_emmcpll_0x5f                GET_REG_ADDR(EMMC_PLL_BASE, 0x5f)
#define BIT_FLASH_MACRO_TO_FICE         BIT0

#define reg_emmcpll_0x63                GET_REG_ADDR(EMMC_PLL_BASE, 0x63)
#define BIT_USE_RXDLL                   BIT0


#define reg_emmcpll_0x68                GET_REG_ADDR(EMMC_PLL_BASE, 0x68)
#define reg_emmcpll_0x69                GET_REG_ADDR(EMMC_PLL_BASE, 0x69)
#define BIT_SKEW4_CLK_INV_MASK          (BIT11|BIT10|BIT9|BIT8)
#define BIT_SKEW4_DATA_INV              BIT9
#define BIT_SKEW4_CMD_RSP_INV           BIT10
#define BIT_SKEW4_ALL_INV               (BIT10|BIT9)



#define reg_emmcpll_0x6a                GET_REG_ADDR(EMMC_PLL_BASE, 0x6a)
#define reg_emmcpll_0x6b                GET_REG_ADDR(EMMC_PLL_BASE, 0x6b)

#define reg_emmcpll_0x6c                GET_REG_ADDR(EMMC_PLL_BASE, 0x6c)
#define BIT_DQS_DELAY_CELL_MASK         (BIT4|BIT5|BIT6|BIT7)
#define BIT_DQS_DELAY_CELL_SHIFT        4
#define BIT_DQS_MODE_MASK               (BIT0|BIT1|BIT2)
#define BIT_DQS_MDOE_SHIFT              0
#define BIT_DQS_MODE_2T                 (0 << BIT_DQS_MDOE_SHIFT)
#define BIT_DQS_MODE_1_5T               (1 << BIT_DQS_MDOE_SHIFT)
#define BIT_DQS_MODE_2_5T               (2 << BIT_DQS_MDOE_SHIFT)
#define BIT_DQS_MODE_1T                 (3 << BIT_DQS_MDOE_SHIFT)

#define reg_emmcpll_0x6d                GET_REG_ADDR(EMMC_PLL_BASE, 0x6d)
#define reg_emmcpll_0x6e                GET_REG_ADDR(EMMC_PLL_BASE, 0x6e)
#define reg_emmcpll_0x6f                GET_REG_ADDR(EMMC_PLL_BASE, 0x6f)
#define reg_emmcpll_0x70                GET_REG_ADDR(EMMC_PLL_BASE, 0x70)
#define reg_emmcpll_0x71                GET_REG_ADDR(EMMC_PLL_BASE, 0x71)
#define reg_emmcpll_0x73                GET_REG_ADDR(EMMC_PLL_BASE, 0x73)
#define reg_emmcpll_0x74                GET_REG_ADDR(EMMC_PLL_BASE, 0x74)
#define reg_emmcpll_0x7f                GET_REG_ADDR(EMMC_PLL_BASE, 0x7f)

#define BIT_TUNE_SHOT_OFFSET_MASK       (BIT4|BIT5|BIT6|BIT7)
#define BIT_TUNE_SHOT_OFFSET_SHIFT      4

//--------------------------------clock gen------------------------------------
#define BIT_CLK_XTAL_12M                0x0
#define BIT_FCIE_CLK_20M                0x1
#define BIT_FCIE_CLK_32M                0x2
#define BIT_FCIE_CLK_36M                0x3
#define BIT_FCIE_CLK_40M                0x4
#define BIT_FCIE_CLK_43_2M              0x5
#define BIT_FCIE_CLK_54M                0x6
#define BIT_FCIE_CLK_62M                0x7
#define BIT_FCIE_CLK_72M                0x8
#define BIT_FCIE_CLK_86M                0x9
#define BIT_FCIE_CLK_EMMC_PLL_1X        0xB // 8 bits macro & 32 bit macro HS200
#define BIT_FCIE_CLK_EMMC_PLL_2X        0xC // 32 bit macroDDR & HS400
#define BIT_FCIE_CLK_300K               0xD
#define BIT_CLK_XTAL_24M                0xE
#define BIT_FCIE_CLK_48M                0xF


//--------------------------------mt5896------------------------------------

#define BIT_MT5896_CLK_XTAL_12M         0x0
#define BIT_MT5896_CLK_300K             0x5
#define BIT_MT5896_CLK_24M              0x6
#define BIT_MT5896_CLK_48M              0x7


#define MT5896_CLKGEN0_BASE                        ((unsigned long)emmc_drv.host->clkgen)
#define REG_MT5896_SW_EN_SMI_FCIE2FCIE             GET_REG_ADDR(MT5896_CLKGEN0_BASE, 0x53C)
#define BIT_MT5896_SW_ENABLE                       BIT5

#define MT5896_CLKGEN1_BASE                        ((unsigned long)emmc_drv.host->clkgen2)

#define REG_MT5896_CKG_FCIE                        GET_REG_ADDR(MT5896_CLKGEN1_BASE, 0x3de)
#define BIT_MT5896_FCIE_CLK_GATING                 BIT0
#define BIT_MT5896_FCIE_CLK_INVERSE                BIT1
#define BIT_MT5896_CLKGEN_FCIE_MASK                (BIT4 | BIT3 | BIT2)

#define REG_MT5896_CLG_FCIE_TSP                    GET_REG_ADDR(MT5896_CLKGEN1_BASE, 0x3de)
#define BIT_MT5896_FCIE_CLK_TSP_GATING             BIT8
#define BIT_MT5896_FCIE_CLK_TSP_INVERSE            BIT9
#define BIT_MT5896_CLKGEN_FCIE_TSP_MASK            (BIT11 | BIT10)
#define BIT_MT5896_FCIE_CLK_EMMC_PLL_2X            BIT10
#define BIT_MT5896_FCIE_CLK_EMMC_PLL_1X            BIT11

#define REG_MT5896_CKG_SMI_FCIE_SEL                GET_REG_ADDR(MT5896_CLKGEN1_BASE, 0x3da)
#define BIT_MT5896_CLKGEN_SMI_FCIE_SEL_MASK        (BIT3 | BIT2)
#define BIT_MT5896_CKG_SMI_FCIE_SEL_IMI            BIT2
#define BIT_MT5896_CKG_SMI_FCIE_SEL_SMI            BIT3

#define REG_MT5896_CKG_FCIE_SYNC                    GET_REG_ADDR(MT5896_CLKGEN1_BASE, 0x3e0)
#define BIT_MT5896_CKG_FCIE_SYN_MASK               (BIT1 | BIT0)
#define BIT_MT5896_CKG_FCIE_SYN                    BIT0


#define EMMC_PLL_FLAG                   0x80
#define EMMC_PLL_CLK__20M               (0x01 | EMMC_PLL_FLAG)
#define EMMC_PLL_CLK__27M               (0x02 | EMMC_PLL_FLAG)
#define EMMC_PLL_CLK__32M               (0x03 | EMMC_PLL_FLAG)
#define EMMC_PLL_CLK__36M               (0x04 | EMMC_PLL_FLAG)
#define EMMC_PLL_CLK__40M               (0x05 | EMMC_PLL_FLAG)
#define EMMC_PLL_CLK__48M               (0x06 | EMMC_PLL_FLAG)
#define EMMC_PLL_CLK__52M               (0x07 | EMMC_PLL_FLAG)
#define EMMC_PLL_CLK__62M               (0x08 | EMMC_PLL_FLAG)
#define EMMC_PLL_CLK__72M               (0x09 | EMMC_PLL_FLAG)
#define EMMC_PLL_CLK__80M               (0x0A | EMMC_PLL_FLAG)
#define EMMC_PLL_CLK__86M               (0x0B | EMMC_PLL_FLAG)
#define EMMC_PLL_CLK_100M               (0x0C | EMMC_PLL_FLAG)
#define EMMC_PLL_CLK_120M               (0x0D | EMMC_PLL_FLAG)
#define EMMC_PLL_CLK_140M               (0x0E | EMMC_PLL_FLAG)
#define EMMC_PLL_CLK_160M               (0x0F | EMMC_PLL_FLAG)
#define EMMC_PLL_CLK_200M               (0x10 | EMMC_PLL_FLAG)

/*
 * EMMC Clock Parameters
 */

#define PLL_CLK_20M  20000
#define PLL_CLK_27M  27000
#define PLL_CLK_32M  32000
#define PLL_CLK_36M  36000
#define PLL_CLK_40M  40000
#define PLL_CLK_48M  48000
#define PLL_CLK_52M  52000
#define PLL_CLK_62M  62000
#define PLL_CLK_72M  72000
#define PLL_CLK_80M  80000
#define PLL_CLK_86M  86000
#define PLL_CLK_100M  100000
#define PLL_CLK_120M  120000
#define PLL_CLK_140M  140000
#define PLL_CLK_160M  160000
#define PLL_CLK_200M  200000

#define FCIE_CLK_48M  48000
#define FCIE_CLK_43M  43000
#define FCIE_CLK_40M  40000
#define FCIE_CLK_36M  36000
#define FCIE_CLK_32M  32000
#define FCIE_CLK_24M  24000
#define FCIE_CLK_20M  20000

#define FCIE_CLK_12M  12000
#define FCIE_CLK_300k 300
#define EMMC_KHZ    1000
#define EMMC_FCIE_VALID_CLK_CNT         3//FIXME

extern  U8 gau8_fcie_clk_sel[];
extern  U8 gau8_emmc_pll_sel_52[];
extern  U8 gau8_emmc_pll_sel_200[];//for DDR52 or HS200

struct emmc_fcie_atop_set {
	U32 u32_scan_result;
	U8  u8_clk;
	U8  u8_reg2ch, u8_skew4;
	U8  u8_cell;
	U8  u8_skew2, u8_cell_cnt;
} EMMC_PACK1;

struct emmc_fcie_atop_set_skew4 {
	U32 u32_skew4_result_cmd;
	U32 u32_skew4_result_data;
	U16 u16_clk;
	U8 u8_skew4_rsp, u8_skew4_data;
	U8 u8_skew2;
	U8 u8_reg_2ch_rsp, u8_reg_2ch_data;
} EMMC_PACK1;

#define GT_OFFSET_SKEW4_INV_BIT       0
#define GT_OFFSET_SKEW4_VALUE         1
#define GT_OFFSET_RXDLL_VALUE         2
#define GT_OFFSET_SKEW2_VALUE         3
#define GT_OFFSET_SKEW4_INV_BIT_DIG   4

#define EMMC_RST_L()                    {REG_FCIE_SETBIT(FCIE_BOOT_CONFIG, BIT_EMMC_RSTZ_EN);\
					REG_FCIE_CLRBIT(FCIE_BOOT_CONFIG, BIT_EMMC_RSTZ);\
					}
#define EMMC_RST_H()                    {REG_FCIE_SETBIT(FCIE_BOOT_CONFIG, BIT_EMMC_RSTZ_EN);\
					REG_FCIE_SETBIT(FCIE_BOOT_CONFIG, BIT_EMMC_RSTZ);\
					}


#define REG_BANK_TIMER1                 0x1800
#define TIMER1_BASE                     GET_REG_ADDR(RIU_PM_BASE, REG_BANK_TIMER1)

#define TIMER1_ENABLE                   GET_REG_ADDR(TIMER1_BASE, 0x20)
#define TIMER1_HIT                      GET_REG_ADDR(TIMER1_BASE, 0x21)
#define TIMER1_MAX_LOW                  GET_REG_ADDR(TIMER1_BASE, 0x22)
#define TIMER1_MAX_HIGH                 GET_REG_ADDR(TIMER1_BASE, 0x23)
#define TIMER1_CAP_LOW                  GET_REG_ADDR(TIMER1_BASE, 0x24)
#define TIMER1_CAP_HIGH                 GET_REG_ADDR(TIMER1_BASE, 0x25)

//--------------------------------INV----------------------------
#define REG_ANL_SKEW4_INV                reg_emmcpll_0x6c

#define BIT_ANL_SKEW4_INV                BIT7


//--------------------------------power saving mode----------------------------
#define REG_BANK_PM_SLEEP               (0x700)
#define PM_SLEEP_REG_BASE_ADDR          ((unsigned long)emmc_drv.host->pm_sleep)
#define reg_pwrgd_int_glirm             GET_REG_ADDR(PM_SLEEP_REG_BASE_ADDR, 0x61)
#define BIT_PWRGD_INT_GLIRM_EN          BIT9
#define BIT_PWEGD_INT_GLIRM_MASK        (BIT15|BIT14|BIT13|BIT12|BIT11|BIT10)

//=====================================================
// API declarations
//=====================================================
U32 emmc_hw_timer_delay(U32 u32us);
U32 emmc_hw_timer_sleep(U32 u32ms);
#define EMMC_HW_TIMER_HZ                12000000//12MHz  [FIXME]

////////////////////////////////////////////////////////////////////////////////////////////////////

// define what latch method (mode) fcie has
// implement switch pad function with below cases

#define FCIE_MODE_GPIO_PAD_DEFO_SPEED       0
#define FCIE_EMMC_BYPASS                    FCIE_MODE_GPIO_PAD_DEFO_SPEED

#define FCIE_MODE_8BITS_MACRO_HIGH_SPEED    2
#define FCIE_EMMC_SDR                       FCIE_MODE_8BITS_MACRO_HIGH_SPEED

#define FCIE_MODE_8BITS_MACRO_DDR52         3
#define FCIE_EMMC_DDR                       FCIE_MODE_8BITS_MACRO_DDR52
#define FCIE_eMMC_DDR_8BIT_MACRO            FCIE_MODE_8BITS_MACRO_DDR52

#define FCIE_MODE_32BITS_MACRO_HS200        5
#define FCIE_EMMC_HS200                     FCIE_MODE_32BITS_MACRO_HS200

#define FCIE_MODE_32BITS_MACRO_HS400_DS     6 // data strobe
#define FCIE_EMMC_HS400                     FCIE_MODE_32BITS_MACRO_HS400_DS
#define FCIE_EMMC_HS400_DS                  FCIE_EMMC_HS400

#define FCIE_MODE_32BITS_MACRO_HS400_AIFO_5_1    7 // eMMC 5.1
#define FCIE_EMMC_HS400_5_1                 FCIE_MODE_32BITS_MACRO_HS400_AIFO_5_1
#define FCIE_EMMC_HS400_AIFO_5_1            FCIE_EMMC_HS400_5_1


// define what latch method (mode) use for latch eMMC data
// switch FCIE mode when driver (kernel) change eMMC speed

#define EMMC_DEFO_SPEED_MODE            FCIE_MODE_GPIO_PAD_DEFO_SPEED
#define EMMC_HIGH_SPEED_MODE            FCIE_MODE_8BITS_MACRO_HIGH_SPEED
#define EMMC_DDR52_MODE                 FCIE_MODE_8BITS_MACRO_DDR52
#define EMMC_HS200_MODE                 FCIE_MODE_32BITS_MACRO_HS200
#define EMMC_HS400_MODE                 FCIE_MODE_32BITS_MACRO_HS400_DS//FCIE_MODE_32BITS_MACRO_HS400_SKEW4
#define EMMC_HS400_5_1_MODE             FCIE_MODE_32BITS_MACRO_HS400_AIFO_5_1//FCIE_MODE_32BITS_MACRO_HS400_SKEW4

// define what speed we want this chip/project run
//------------------------------
// DDR48, DDR52, HS200, HS400
#define ENABLE_EMMC_ATOP                1
#define ENABLE_EMMC_DDR52               0
#define ENABLE_EMMC_HS200               1
#define ENABLE_EMMC_HS400               1
#define ENABLE_EMMC_HS400_5_1           1

#define EMMC_IF_TUNING_TTABLE()         (emmc_drv.u32_drvflag & DRV_FLAG_TUNING_TTABLE)

// mboot use this config
#if CONFIG_EMMC_FORCE_HS200
#undef  ENABLE_EMMC_DDR52
#undef  ENABLE_EMMC_HS200
#undef  ENABLE_EMMC_HS400
#undef  ENABLE_EMMC_HS400_5_1
#define ENABLE_EMMC_DDR52               0
#define ENABLE_EMMC_HS200               1
#define ENABLE_EMMC_HS400               0
#define ENABLE_EMMC_HS400_5_1           0
#endif

#if CONFIG_EMMC_FORCE_DDR52
#undef  ENABLE_EMMC_DDR52
#undef  ENABLE_EMMC_HS200
#undef  ENABLE_EMMC_HS400
#undef  ENABLE_EMMC_HS400_5_1
#define ENABLE_EMMC_DDR52               1
#define ENABLE_EMMC_HS200               0
#define ENABLE_EMMC_HS400               0
#define ENABLE_EMMC_HS400_5_1           0
#endif

#if defined(CONFIG_MSTAR_TITANIA_BD_FPGA) && CONFIG_MSTAR_TITANIA_BD_FPGA
#undef  ENABLE_EMMC_DDR52
#undef  ENABLE_EMMC_HS200
#undef  ENABLE_EMMC_HS400
#undef  ENABLE_EMMC_HS400_5_1
#define ENABLE_EMMC_DDR52               0
#define ENABLE_EMMC_HS200               0
#define ENABLE_EMMC_HS400               0
#define ENABLE_EMMC_HS400_5_1           0
#endif

#if (defined(ENABLE_EMMC_HS200) && ENABLE_EMMC_HS200) || \
	(defined(ENABLE_EMMC_HS400) && ENABLE_EMMC_HS400) || \
	(defined(ENABLE_EMMC_HS400_5_1) && ENABLE_EMMC_HS400_5_1)
#define ENABLE_EMMC_AFIFO               1
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////
U32 emmc_pads_switch(U32 u32_mode);
U32 emmc_clock_setting(U16 u16_clk_param);
void emmc_set_watch_dog(U8 u8_if_enable);
void emmc_reset_watch_dog(void);
unsigned long emmc_translate_dma_address_ex(unsigned long ulong_dma_addr, U32 u32_byte_cnt);
void emmc_flush_data_cache_buffer(unsigned long ulong_dma_addr, U32 u32_byte_cnt);
void emmc_invalidate_data_cache_buffer(unsigned long ulong_dma_addr, U32 u32_byte_cnt);
void emmc_flush_miu_pipe(void);
U32 emmc_platform_reset_pre(void);
U32 emmc_platform_reset_post(void);
U32 emmc_platform_init(void);
U32 emmc_check_if_mem_corrupt(void);
void emmc_dump_pad_clk(void);

#define eMMC_BOOT_PART_W                BIT0
#define eMMC_BOOT_PART_R                BIT1

U32 emmc_hw_timer_start(void);
U64 emmc_hw_timer_tick(void);
U32 emmc_fcie_reset_to_hs400(U8 u8_clk_param);
U32 emmc_fcie_reset_to_hs200(U8 u8_clk_param);

//=====================================================
// partitions config
//=====================================================
#define EMMC_SKIP_IDENTIFY              1 //1: enable, 0: disable
#define CONTEXT_SIZE_BLKCNT             2 //2 blocks (1KB) for driver context

// every blk is 512 bytes (reserve 2MB-64KB for internal use)
#define EMMC_DRV_RESERVED_BLK_CNT       ((0x200000 - 0x10000) / 0x200)

#define eMMC_CIS_NNI_BLK_CNT            2
#define eMMC_CIS_PNI_BLK_CNT            2
#define EMMC_TEST_BLK_CNT               (0x100000 / 0x200) //1MB

#define EMMC_CIS_BLK_0                  (64 * 1024 / 512) //from 64KB
#define EMMC_NNI_BLK_0                  (EMMC_CIS_BLK_0 + 0)
#define EMMC_NNI_BLK_1                  (EMMC_CIS_BLK_0 + 1)
#define EMMC_PNI_BLK_0                  (EMMC_CIS_BLK_0 + 2)
#define EMMC_PNI_BLK_1                  (EMMC_CIS_BLK_0 + 3)
#define EMMC_DDRTABLE_BLK_0             (EMMC_CIS_BLK_0 + 4)
#define EMMC_DDRTABLE_BLK_1             (EMMC_CIS_BLK_0 + 5)
#define EMMC_HS200TABLE_BLK_0           (EMMC_CIS_BLK_0 + 6)
#define EMMC_HS200TABLE_BLK_1           (EMMC_CIS_BLK_0 + 7)
#define EMMC_HS400TABLE_BLK_0           (EMMC_CIS_BLK_0 + 8)
#define EMMC_HS400TABLE_BLK_1           (EMMC_CIS_BLK_0 + 9)
#define EMMC_HS400EXTTABLE_BLK_0        (EMMC_CIS_BLK_0 + 10)
#define EMMC_HS400EXTTABLE_BLK_1        (EMMC_CIS_BLK_0 + 11)
#define EMMC_CRAZY_PATTERN_BLK          (EMMC_CIS_BLK_0 + 12)
#define EMMC_HS400SKEW4TABLE_BLK_0      (EMMC_CIS_BLK_0 + 13)
#define EMMC_HS400SKEW4TABLE_BLK_1      (EMMC_CIS_BLK_0 + 14)

#if defined(EMMC_SKIP_IDENTIFY) && EMMC_SKIP_IDENTIFY

#define EMMC_UB_DRV_CONTX               (EMMC_CIS_BLK_0 + 15)//1KB
#define EMMC_EXTCSD_BLK                 (EMMC_CIS_BLK_0 + 16)
#define EMMC_ALLRSP_BLK                 (EMMC_CIS_BLK_0 + 17)
#define EMMC_KL_DRV_CONTX               (EMMC_CIS_BLK_0 + 18)//1KB

#define EMMC_BURST_LEN_BLK_0            (EMMC_CIS_BLK_0 + 20)
#define EMMC_LIFE_TEST_BYTE_CNT_BLK     (EMMC_CIS_BLK_0 + 21)
#else
#define EMMC_BURST_LEN_BLK_0            (EMMC_CIS_BLK_0 + 14)
#define EMMC_LIFE_TEST_BYTE_CNT_BLK     (EMMC_CIS_BLK_0 + 15)
#endif

#define EMMC_CIS_BLK_END                EMMC_LIFE_TEST_BYTE_CNT_BLK
// last 1MB in reserved area, use for eMMC test
#define EMMC_TEST_BLK_0                 (EMMC_CIS_BLK_END + 1)
#define EMMC_TOTAL_RESERVED_BLK_CNT     (EMMC_DRV_RESERVED_BLK_CNT + EMMC_CIS_BLK_0)

//=====================================================
// Driver configs
//=====================================================
#define EMMC_UPDATE_FIRMWARE            0

#define EMMC_ST_PLAT                    0x80000000
// [CAUTION]: to verify IP and HAL code, defaut 0
#define IF_IP_VERIFY                    1//[FIXME] -->

// need to emmc_pads_switch
// need to eMMC_clock_setting
#define IF_FCIE_SHARE_IP                1

//------------------------------
#define FICE_BYTE_MODE_ENABLE           1//always 1
#define ENABLE_EMMC_RIU_MODE            0//for debug cache issue

#define ENABLE_EMMC_POWER_SAVING_MODE   1

//------------------------------
#define EMMC_FEATURE_RELIABLE_WRITE     1
#if EMMC_UPDATE_FIRMWARE
#undef  EMMC_FEATURE_RELIABLE_WRITE
#define EMMC_FEATURE_RELIABLE_WRITE     0
#endif

//------------------------------
#define eMMC_RSP_FROM_RAM               0
//------------------------------
#define EMMC_SECTOR_BUF_BYTECTN         EMMC_SECTOR_BUF_16KB
extern U8 GLOBAL_EMMC_SECTORBUF[];


#define FCIE_NO_MIU_SELECT             1



//=====================================================
// debug option
//=====================================================
#define EMMC_TEST_IN_DESIGN             0 // [FIXME]: set 1 to verify HW timer

#ifndef EMMC_DEBUG_MSG
#define EMMC_DEBUG_MSG                  1
#endif

/* Define trace levels. */
#define EMMC_DEBUG_LEVEL_ERROR          (1)    /* Error condition debug messages. */
#define EMMC_DEBUG_LEVEL_WARNING        (2)    /* Warning condition debug messages. */
#define EMMC_DEBUG_LEVEL_HIGH           (3)    /* Debug messages (high debugging). */
#define EMMC_DEBUG_LEVEL_MEDIUM         (4)    /* Debug messages. */
#define EMMC_DEBUG_LEVEL_LOW            (5)    /* Debug messages (low debugging). */

/* Higer debug level means more verbose */
#ifndef EMMC_DEBUG_LEVEL
#define EMMC_DEBUG_LEVEL                EMMC_DEBUG_LEVEL_WARNING
#endif

#if defined(EMMC_DEBUG_MSG) && EMMC_DEBUG_MSG
#define emmc_printf    printf
#define emmc_debug(dbg_lv, tag, str, ...) \
	do {\
		if (dbg_lv > EMMC_DEBUG_LEVEL)\
			break;\
		else if (EMMC_IF_DISABLE_LOG())\
			break;\
		else {\
			if (tag)\
				emmc_printf("[ %s() ] ", __func__);\
								\
			emmc_printf(str, ##__VA_ARGS__);\
		} \
	} while (0)
#else /* EMMC_DEBUG_MSG */
#define emmc_printf(...)
#define emmc_debug(enable, tag, str, ...) do {} while (0)
#endif /* EMMC_DEBUG_MSG */

#define emmc_die(msg) do {\
		printf("eMMC die %s() Ln: %d", __func__, __LINE__);\
		while (1)\
		;\
	} while (0)

#define emmc_pause() do {\
	printf("delay ");\
	emmc_hw_timer_delay(HW_TIMER_DELAY_1s);\
	printf("1 ");\
	emmc_hw_timer_delay(HW_TIMER_DELAY_1s);\
	printf("2 ");\
	emmc_hw_timer_delay(HW_TIMER_DELAY_1s);\
	printf("3 ");\
	emmc_hw_timer_delay(HW_TIMER_DELAY_1s);\
	printf("4 ");\
	emmc_hw_timer_delay(HW_TIMER_DELAY_1s);\
	printf("5 ");\
	emmc_hw_timer_delay(HW_TIMER_DELAY_1s);\
	printf("6\n");\
	emmc_hw_timer_delay(HW_TIMER_DELAY_1s);\
} while (0)

//=====================================================
// unit for HW Timer delay (unit of us)
//=====================================================
#define HW_TIMER_DELAY_1us              1
#define HW_TIMER_DELAY_5us              5
#define HW_TIMER_DELAY_10us             10
#define HW_TIMER_DELAY_100us            100
#define HW_TIMER_DELAY_500us            500
#define HW_TIMER_DELAY_1ms              (1000 * HW_TIMER_DELAY_1us)
#define HW_TIMER_DELAY_5ms              (5    * HW_TIMER_DELAY_1ms)
#define HW_TIMER_DELAY_10ms             (10   * HW_TIMER_DELAY_1ms)
#define HW_TIMER_DELAY_100ms            (100  * HW_TIMER_DELAY_1ms)
#define HW_TIMER_DELAY_500ms            (500  * HW_TIMER_DELAY_1ms)
#define HW_TIMER_DELAY_1s               (1000 * HW_TIMER_DELAY_1ms)

//=====================================================
// set FCIE clock
//=====================================================
#define FCIE_SLOWEST_CLK                BIT_FCIE_CLK_300K
#define FCIE_DEFO_SPEED_CLK             BIT_CLK_XTAL_12M
//#define FCIE_HIGH_SPEED_CLK             BIT_CLK_XTAL_12M
#define FCIE_HIGH_SPEED_CLK             EMMC_PLL_CLK_FAST

// for backward compatible
#define FCIE_SLOW_CLK                   FCIE_DEFO_SPEED_CLK
#define FCIE_DEFAULT_CLK                FCIE_HIGH_SPEED_CLK
#define EMMC_PLL_CLK_SLOW               EMMC_PLL_CLK__20M
#define EMMC_PLL_CLK_FAST               EMMC_PLL_CLK_200M


//=====================================================
// transfer DMA Address
//=====================================================
#define MIU_BUS_WIDTH_BITS              4 // Need to confirm

/*
 * Important:
 * The following buffers should be large enough for a whole eMMC block
 */
// FIXME, this is only for verifing IP
#define DMA_W_ADDR                      0x20400000
#define DMA_R_ADDR                      0x22500000
#define DMA_W_DDR_ADDR                  0x20600000
#define DMA_R_DDR_ADDR                  0x20700000
//#define DMA_MBOOT_ADDR                  0x23000000


//=====================================================
// misc
//=====================================================
//#define BIG_ENDIAN
#define LITTLE_ENDIAN
#define ADMA_TABLE_LENGTH 16
//=====================================================
// MT5896 E3 PATCH
//=====================================================
#define MT5896_CHIP_ID 5896
#define MT5896_CHIP_VER 3

//=====================================================
// MT5879 PATCH
//=====================================================
#define MT5879_CHIP_ID 5879

//=====================================================
// MT5873 PATCH
//=====================================================
#define MT5873_CHIP_ID 5873

#if defined(ENABLE_EMMC_HS400) && ENABLE_EMMC_HS400
U32 emmc_fcie_detect_hs400_timing(void);
void emmc_fcie_set_delay_line(U32 u32_value);
U32 emmc_fcie_detect_hs400_timing_skew4_rx(U8 u8_enable_error_log);
void emmc_fcie_set_skew1_skew4(U32 u32_skew1, U32 u32_skew4);
U32 emmc_fcie_detect_hs400_timing_skew4_skew1(U8 u8_enable_error_log);
U32 emmc_fcie_detect_hs400_timing_skew1_skew2(U8 u8_enable_error_log);
U32 emmc_fcie_detect_hs400_timing_trigger_level_rx(U8 u8_enable_error_log);
U32 emmc_fcie_detect_hs400_skew4_timing(void);
U32 emmc_fcie_switch_to_hs400_mode(void);
#endif
#if defined(ENABLE_EMMC_HS400_5_1) && ENABLE_EMMC_HS400_5_1
U32 emmc_fcie_reset_to_hs400_5_1(U8 u8_clk_param);
U32 emmc_fcie_detect_hs400_5_1_timing(void);
U32 emmc_fcie_detect_hs400_5_1_timing_rx(U8 u8_enable_error_log);
U32 emmc_fcie_detect_hs400_5_1_timing_skew1_skew2(U8 u8_enable_error_log);
U32 emmc_fcie_detect_hs400_5_1_timing_trigerlevel_rx(U8 u8_enable_error_log);
U32 emmc_fcie_detect_hs400_5_1_skew4(void);
U32 emmc_fcie_detect_hs400_5_1_skew4_timing(void);
#endif
U32 emmc_m6e3_rescan_timing(void);
U32 emmc_fcie_m6e3_fix_skew4_result(U32 u32_skew4_result);
void emmc_set_triger_level(U8 u8_level);
U32 emmc_fcie_detect_hs200_timing_skew1_skew4(U8 u8_enable_error_log);
U32 emmc_fcie_detect_hs200_timing_triger_level_skew4(U8 u8_enable_error_log);
U32 emmc_fcie_detect_hs400_timing_skew2_ex(U32 u32_skew2_result);
U32 emmc_fcie_m6e3_detect_hs400_timing_skew1_skew2(U8 u8_enable_error_log);
U32 emmc_fcie_m6e3_detect_hs400_timing_skew4_skew1(U8 u8_enable_error_log);
U32 emmc_fcie_m6e3_detect_hs400_5_1_timing_skew1_skew2(U8 u8_enable_error_log);
U32 emmc_fcie_m6e3_detect_hs400_5_1_timing_skew4_skew1(U8 u8_enable_error_log);
U32 emmc_fcie_detect_timing_skew4_ex(U32 u32_skew4_result, U8 *u8_skew4_idx, U8 *u8_reg2ch);
U32 emmc_fcie_m6e3_detect_hs400_timing_skew2_ex(U32 u32_skew2_result);
#endif /* __EMMC_UBOOT__ */
