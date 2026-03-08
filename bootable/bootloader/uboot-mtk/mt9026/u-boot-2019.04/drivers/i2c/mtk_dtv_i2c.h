/* SPDX-License-Identifier: (GPL-2.0 OR BSD-3-Clause) */
/*
 * MediaTek Inc. (C) 2018-2020. All rights reserved.
 */

#ifndef _MTK_DTV_I2C_H_
#define _MTK_DTV_I2C_H_

//------------------------------------------------------------------------------
//  Hardware Capability
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//  Macro and Define
//------------------------------------------------------------------------------
//#define MHal_IIC_DELAY()            { udelay(1000); udelay(1000); udelay(1000); udelay(1000); udelay(1000); }//delay 5ms
//
//extern ptrdiff_t mstar_pm_base;
//
//#if defined(CONFIG_ARM) || defined(CONFIG_MIPS)
//#define RIU_MAP                    0xfd000000
//#elif defined(CONFIG_ARM64)
//extern ptrdiff_t mstar_pm_base;
//#define RIU_MAP                    mstar_pm_base
//#endif
//#define RIU8                            ((unsigned char volatile *) RIU_MAP)

//############################
//
//IP bank address : for pad mux in chiptop
//
//############################
#define GPIO_FUNC_MUX_REG_BASE          (0x1F000000 + (0x322900 << 1))
#define PMSLEEP_REG_BASE                (0x1F000000 + (0x000E00 << 1))

//for port 0
#define CHIP_REG_HWI2C_MIIC0            (GPIO_FUNC_MUX_REG_BASE + 0x28 * 4)
#define CHIP_MIIC0_PAD_0                0
#define CHIP_MIIC0_PAD_1                (BIT(0))	//PAD_I2S_OUT_SD1/PAD_I2S_OUT_SD2
#define CHIP_MIIC0_PAD_2                (BIT(1))	//PAD_TCON0/PAD_TCON1
#define CHIP_MIIC0_PAD_3                (BIT(0) | BIT(1))	//PAD_TS2_CLK/PAD_TS2_SYNC
#define CHIP_MIIC0_PAD_MSK              (BIT(0) | BIT(1))

//for port 1
#define CHIP_REG_HWI2C_MIIC1            (GPIO_FUNC_MUX_REG_BASE + 0x28 * 4)
#define CHIP_MIIC1_PAD_0                (0)
#define CHIP_MIIC1_PAD_1                (BIT(4))	//PAD_TGPIO2/PAD_TGPIO3
#define CHIP_MIIC1_PAD_MSK              (BIT(4))

//for port 2
#define CHIP_REG_HWI2C_MIIC2            (GPIO_FUNC_MUX_REG_BASE + 0x28 * 4 + 1)
#define CHIP_MIIC2_PAD_0                (0)
#define CHIP_MIIC2_PAD_1                (BIT0)	//PAD_I2S_IN_DIN0/PAD_I2S_IN_DIN1
#define CHIP_MIIC2_PAD_2                (BIT(0) | BIT(1))	//PAD_GPIO9/PAD_GPIO10
#define CHIP_MIIC2_PAD_MSK              (BIT(0) | BIT(1))

//for port 3
#define CHIP_REG_HWI2C_DDCR             (GPIO_FUNC_MUX_REG_BASE + 0x1A * 4)
#define CHIP_DDCR_PAD_0                 (0)
#define CHIP_DDCR_PAD_1                 (BIT(1))	//PAD_DDCR_CK/PAD_DDCR_DA
#define CHIP_DDCR_PAD_MSK               (BIT(1) | BIT(0))

//for port 4
#define CHIP_REG_HWI2C_MIIC4            (PMSLEEP_REG_BASE + 0x64 * 4 + 1)
#define CHIP_MIIC4_PAD_0                (0)
#define CHIP_MIIC4_PAD_1                (BIT(7))	//PAD_GPIO7_PM/PAD_GPIO8_PM
#define CHIP_MIIC4_PAD_MSK              (BIT(6) | BIT(7))

//#################
//#
//#  For Non-PM HWI2C
//#
//STD mode
#define REG_HWI2C_MIIC_CFG              (0x00 * 4)
#define _MIIC_CFG_RESET                 (BIT(0))
#define _MIIC_CFG_EN_DMA                (BIT(1))
#define _MIIC_CFG_EN_INT                (BIT(2))
#define _MIIC_CFG_EN_CLKSTR             (BIT(3))
#define _MIIC_CFG_EN_TMTINT             (BIT(4))
#define _MIIC_CFG_EN_FILTER             (BIT(5))
#define _MIIC_CFG_EN_PUSH1T             (BIT(6))
#define _MIIC_CFG_RESERVED              (BIT(7))

#define REG_HWI2C_CMD_START             (0x01 * 4)
#define _CMD_START                      (BIT(0))

#define REG_HWI2C_CMD_STOP              (0x01 * 4 + 1)
#define _CMD_STOP                       (BIT(0))

#define REG_HWI2C_WDATA                 (0x02 * 4)

#define REG_HWI2C_WDATA_GET             (0x02 * 4 + 1)
#define _WDATA_GET_ACKBIT               (BIT(0))

#define REG_HWI2C_RDATA                 (0x03 * 4)

#define REG_HWI2C_RDATA_CFG             (0x03 * 4 + 1)
#define _RDATA_CFG_TRIG                 (BIT(0))
#define _RDATA_CFG_ACKBIT               (BIT(1))

#define REG_HWI2C_INT_CTL               (0x04 * 4)
#define _INT_CTL                        (BIT(0))        //write this register to clear int

#define REG_HWI2C_CUR_STATE             (0x05 * 4)      //For Debug
#define _CUR_STATE_MSK                  (BIT(4) | BIT(3) | BIT(2) | BIT(1) | BIT(0))

#define REG_HWI2C_INT_STATUS            (0x05 * 4 + 1)  //For Debug
#define _INT_STARTDET                   (BIT(0))
#define _INT_STOPDET                    (BIT(1))
#define _INT_RXDONE                     (BIT(2))
#define _INT_TXDONE                     (BIT(3))
#define _INT_CLKSTR                     (BIT(4))
#define _INT_SCLERR                     (BIT(5))

#define REG_HWI2C_STP_CNT               (0x08 * 4)
#define REG_HWI2C_CKH_CNT               (0x09 * 4)
#define REG_HWI2C_CKL_CNT               (0x0A * 4)
#define REG_HWI2C_SDA_CNT               (0x0B * 4)
#define REG_HWI2C_STT_CNT               (0x0C * 4)
#define REG_HWI2C_LTH_CNT               (0x0D * 4)
#define REG_HWI2C_TMT_CNT               (0x0E * 4)
#define REG_HWI2C_SCLI_DELAY            (0x0F * 4)
#define _SCLI_DELAY                     (BIT(2) | BIT(1) | BIT(0))

#define REG_HWI2C_RESERVE0              (0x10 * 4)
#define REG_HWI2C_RESERVE1              (0x10 * 4 + 1)
#define _MIIC_RESET                     (BIT(0))

//DMA mode
#define REG_HWI2C_DMA_CFG               (0x20 * 4)
#define _DMA_CFG_RESET                  (BIT(1))
#define _DMA_CFG_INTEN                  (BIT(2))
#define _DMA_CFG_MIURST                 (BIT(3))
#define _DMA_CFG_MIUPRI                 (BIT(4))

#define REG_HWI2C_DMA_MIU_ADR_B0_B7     (0x21 * 4)
#define REG_HWI2C_DMA_MIU_ADR_B8_B15    (0x21 * 4 + 1)
#define REG_HWI2C_DMA_MIU_ADR_B16_B23   (0x22 * 4)
#define REG_HWI2C_DMA_MIU_ADR_B24_B31   (0x22 * 4 + 1)
#define REG_HWI2C_DMA_CTL               (0x23 * 4)
//#define _DMA_CTL_TRIG                   (BIT(0))
//#define _DMA_CTL_RETRIG                 (BIT(1))
#define _DMA_CTL_TXNOSTOP               (BIT(5))    //miic transfer format, 1: S+data..., 0: S+data...+P
#define _DMA_CTL_RDWTCMD                (BIT(6))    //miic transfer format, 1:read, 0:write
#define _DMA_CTL_MIUCHSEL               (BIT(7))    //0: miu0, 1:miu1

#define REG_HWI2C_DMA_TXR               (0x24 * 4)
#define _DMA_TXR_DONE                   (BIT(0))

#define REG_HWI2C_DMA_CMDDAT0           (0x25 * 4)  // 8 bytes
#define REG_HWI2C_DMA_CMDDAT1           (0x25 * 4 + 1)
#define REG_HWI2C_DMA_CMDDAT2           (0x26 * 4)
#define REG_HWI2C_DMA_CMDDAT3           (0x26 * 4 + 1)
#define REG_HWI2C_DMA_CMDDAT4           (0x27 * 4)
#define REG_HWI2C_DMA_CMDDAT5           (0x27 * 4 + 1)
#define REG_HWI2C_DMA_CMDDAT6           (0x28 * 4)
#define REG_HWI2C_DMA_CMDDAT7           (0x28 * 4 + 1)
#define REG_HWI2C_DMA_CMDLEN            (0x29 * 4)
#define _DMA_CMDLEN_MSK                 (BIT(2) | BIT(1) | BIT(0))

#define REG_HWI2C_DMA_DATLEN_L          (0x2A * 4)
#define REG_HWI2C_DMA_DATLEN_H          (0x2B * 4)
#define REG_HWI2C_DMA_TXFRCNT_L         (0x2C * 4)
#define REG_HWI2C_DMA_TXFRCNT_H         (0x2D * 4)
#define REG_HWI2C_DMA_SLVADR            (0x2E * 4)
#define _DMA_SLVADR_10BIT_MSK           (0x3FF)     //10 bits
#define _DMA_SLVADR_NORML_MSK           (0x7F)      //7 bits

#define REG_HWI2C_DMA_SLVCFG            (0x2E * 4 + 1)
#define _DMA_10BIT_MODE                 (BIT(2))

#define REG_HWI2C_DMA_CTL_TRIG          (0x2F * 4)
#define _DMA_CTL_TRIG                   (BIT(0))

#define REG_HWI2C_DMA_CTL_RETRIG        (0x2F * 4 + 1)
#define _DMA_CTL_RETRIG                 (BIT(0))

#define REG_HWI2C_RESERVE_0             (0x30 * 4)
#define REG_HWI2C_RESERVE_1             (0x30 * 4 + 1)

#define REG_HWI2C_DMA_MIU_ADR_B32_B39   (0x32 * 4)

#endif // _MTK_DTV_I2C_H_
