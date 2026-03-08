/* SPDX-License-Identifier: (GPL-2.0 OR BSD-3-Clause) */
/*
 * MediaTek Inc. (C) 2020. All rights reserved.
 */

#ifndef _MT5896_I2C_H_
#define _MT5896_I2C_H_

//------------------------------------------------------------------------------
//  Hardware Capability
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//  Macro and Define
//------------------------------------------------------------------------------

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

#define REG_HWI2C_SCL_SDA_IO            (0x06 * 4)
#define _SCLI                           (BIT(0))
#define _SDAI                           (BIT(1))
#define _SCLO                           (BIT(4))

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

#define REG_HWI2C_BYTE2BYTE_CNT         (0x13 * 4)
#define _BYTE2BYTE_DLY                  (BIT(3))

//DMA mode
#define REG_HWI2C_DMA_CFG               (0x20 * 4)
#define _DMA_CFG_RESET                  (BIT(1))
#define _DMA_CFG_INTEN                  (BIT(2))
#define _DMA_CFG_MIURST                 (BIT(3))
#define _DMA_CFG_MIUPRI                 (BIT(4))
#define _DMA_CFG_DMA_USE_REG            (BIT(6))

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

#define REG_HWI2C_DMA_WBUF00            (0x40 * 4)
#define REG_HWI2C_DMA_WBUF01            (0x40 * 4 + 1)
#define REG_HWI2C_DMA_WBUF02            (0x41 * 4)
#define REG_HWI2C_DMA_WBUF03            (0x41 * 4 + 1)
#define REG_HWI2C_DMA_WBUF04            (0x42 * 4)
#define REG_HWI2C_DMA_WBUF05            (0x42 * 4 + 1)
#define REG_HWI2C_DMA_WBUF06            (0x43 * 4)
#define REG_HWI2C_DMA_WBUF07            (0x43 * 4 + 1)
#define REG_HWI2C_DMA_WBUF08            (0x44 * 4)
#define REG_HWI2C_DMA_WBUF09            (0x44 * 4 + 1)
#define REG_HWI2C_DMA_WBUF10            (0x45 * 4)
#define REG_HWI2C_DMA_WBUF11            (0x45 * 4 + 1)
#define REG_HWI2C_DMA_WBUF12            (0x46 * 4)
#define REG_HWI2C_DMA_WBUF13            (0x46 * 4 + 1)
#define REG_HWI2C_DMA_WBUF14            (0x47 * 4)
#define REG_HWI2C_DMA_WBUF15            (0x47 * 4 + 1)
#define REG_HWI2C_DMA_WBUF16            (0x48 * 4)
#define REG_HWI2C_DMA_WBUF17            (0x48 * 4 + 1)
#define REG_HWI2C_DMA_WBUF18            (0x49 * 4)
#define REG_HWI2C_DMA_WBUF19            (0x49 * 4 + 1)
#define REG_HWI2C_DMA_WBUF20            (0x4A * 4)
#define REG_HWI2C_DMA_WBUF21            (0x4A * 4 + 1)
#define REG_HWI2C_DMA_WBUF22            (0x4B * 4)
#define REG_HWI2C_DMA_WBUF23            (0x4B * 4 + 1)
#define REG_HWI2C_DMA_WBUF24            (0x4C * 4)
#define REG_HWI2C_DMA_WBUF25            (0x4C * 4 + 1)
#define REG_HWI2C_DMA_WBUF26            (0x4D * 4)
#define REG_HWI2C_DMA_WBUF27            (0x4D * 4 + 1)
#define REG_HWI2C_DMA_WBUF28            (0x4E * 4)
#define REG_HWI2C_DMA_WBUF29            (0x4E * 4 + 1)
#define REG_HWI2C_DMA_WBUF30            (0x4F * 4)
#define REG_HWI2C_DMA_WBUF31            (0x4F * 4 + 1)
#define REG_HWI2C_DMA_WBUF32            (0x50 * 4)
#define REG_HWI2C_DMA_WBUF33            (0x50 * 4 + 1)
#define REG_HWI2C_DMA_WBUF34            (0x51 * 4)
#define REG_HWI2C_DMA_WBUF35            (0x51 * 4 + 1)
#define REG_HWI2C_DMA_WBUF36            (0x52 * 4)
#define REG_HWI2C_DMA_WBUF37            (0x52 * 4 + 1)
#define REG_HWI2C_DMA_WBUF38            (0x53 * 4)
#define REG_HWI2C_DMA_WBUF39            (0x53 * 4 + 1)
#define REG_HWI2C_DMA_WBUF40            (0x54 * 4)
#define REG_HWI2C_DMA_WBUF41            (0x54 * 4 + 1)
#define REG_HWI2C_DMA_WBUF42            (0x55 * 4)
#define REG_HWI2C_DMA_WBUF43            (0x55 * 4 + 1)
#define REG_HWI2C_DMA_WBUF44            (0x56 * 4)
#define REG_HWI2C_DMA_WBUF45            (0x56 * 4 + 1)
#define REG_HWI2C_DMA_WBUF46            (0x57 * 4)
#define REG_HWI2C_DMA_WBUF47            (0x57 * 4 + 1)

#define REG_HWI2C_DMA_RBUF00            (0x60 * 4)
#define REG_HWI2C_DMA_RBUF01            (0x60 * 4 + 1)
#define REG_HWI2C_DMA_RBUF02            (0x61 * 4)
#define REG_HWI2C_DMA_RBUF03            (0x61 * 4 + 1)
#define REG_HWI2C_DMA_RBUF04            (0x62 * 4)
#define REG_HWI2C_DMA_RBUF05            (0x62 * 4 + 1)
#define REG_HWI2C_DMA_RBUF06            (0x63 * 4)
#define REG_HWI2C_DMA_RBUF07            (0x63 * 4 + 1)
#define REG_HWI2C_DMA_RBUF08            (0x64 * 4)
#define REG_HWI2C_DMA_RBUF09            (0x64 * 4 + 1)
#define REG_HWI2C_DMA_RBUF10            (0x65 * 4)
#define REG_HWI2C_DMA_RBUF11            (0x65 * 4 + 1)
#define REG_HWI2C_DMA_RBUF12            (0x66 * 4)
#define REG_HWI2C_DMA_RBUF13            (0x66 * 4 + 1)
#define REG_HWI2C_DMA_RBUF14            (0x67 * 4)
#define REG_HWI2C_DMA_RBUF15            (0x67 * 4 + 1)
#define REG_HWI2C_DMA_RBUF16            (0x68 * 4)
#define REG_HWI2C_DMA_RBUF17            (0x68 * 4 + 1)
#define REG_HWI2C_DMA_RBUF18            (0x69 * 4)
#define REG_HWI2C_DMA_RBUF19            (0x69 * 4 + 1)
#define REG_HWI2C_DMA_RBUF20            (0x6A * 4)
#define REG_HWI2C_DMA_RBUF21            (0x6A * 4 + 1)
#define REG_HWI2C_DMA_RBUF22            (0x6B * 4)
#define REG_HWI2C_DMA_RBUF23            (0x6B * 4 + 1)
#define REG_HWI2C_DMA_RBUF24            (0x6C * 4)
#define REG_HWI2C_DMA_RBUF25            (0x6C * 4 + 1)
#define REG_HWI2C_DMA_RBUF26            (0x6D * 4)
#define REG_HWI2C_DMA_RBUF27            (0x6D * 4 + 1)
#define REG_HWI2C_DMA_RBUF28            (0x6E * 4)
#define REG_HWI2C_DMA_RBUF29            (0x6E * 4 + 1)
#define REG_HWI2C_DMA_RBUF30            (0x6F * 4)
#define REG_HWI2C_DMA_RBUF31            (0x6F * 4 + 1)
#define REG_HWI2C_DMA_RBUF32            (0x70 * 4)
#define REG_HWI2C_DMA_RBUF33            (0x70 * 4 + 1)
#define REG_HWI2C_DMA_RBUF34            (0x71 * 4)
#define REG_HWI2C_DMA_RBUF35            (0x71 * 4 + 1)
#define REG_HWI2C_DMA_RBUF36            (0x72 * 4)
#define REG_HWI2C_DMA_RBUF37            (0x72 * 4 + 1)
#define REG_HWI2C_DMA_RBUF38            (0x73 * 4)
#define REG_HWI2C_DMA_RBUF39            (0x73 * 4 + 1)
#define REG_HWI2C_DMA_RBUF40            (0x74 * 4)
#define REG_HWI2C_DMA_RBUF41            (0x74 * 4 + 1)
#define REG_HWI2C_DMA_RBUF42            (0x75 * 4)
#define REG_HWI2C_DMA_RBUF43            (0x75 * 4 + 1)
#define REG_HWI2C_DMA_RBUF44            (0x76 * 4)
#define REG_HWI2C_DMA_RBUF45            (0x76 * 4 + 1)
#define REG_HWI2C_DMA_RBUF46            (0x77 * 4)
#define REG_HWI2C_DMA_RBUF47            (0x77 * 4 + 1)

#define HWI2C_BUF_SIZE_MAX              (48)

#endif // _MT5896_I2C_H_
