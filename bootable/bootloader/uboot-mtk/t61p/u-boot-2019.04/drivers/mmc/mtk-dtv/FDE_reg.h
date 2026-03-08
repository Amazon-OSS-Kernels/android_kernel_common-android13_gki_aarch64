/* SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause) */
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

#ifndef __FDE_REG_H__
#define __FDE_REG_H__

#define FDE_AES_KEY                     GET_REG_ADDR(FDE_REG_BASE_ADDR, 0x00)	//0x0~0xF
#define FDE_AES_IV                      GET_REG_ADDR(FDE_REG_BASE_ADDR, 0x10)	//0x10~0x17
#define FDE_AES_CTR                     GET_REG_ADDR(FDE_REG_BASE_ADDR, 0x18)	//0x18~0x1F
#define FDE_AES_BUSY                    GET_REG_ADDR(FDE_REG_BASE_ADDR, 0x20)	
#define FDE_AES_CONTROL0                GET_REG_ADDR(FDE_REG_BASE_ADDR, 0x30)	
#define FDE_AES_CONTROL1                GET_REG_ADDR(FDE_REG_BASE_ADDR, 0x31)	

#define FDE_AES_XEX_HW_T_CALC_KICK      GET_REG_ADDR(FDE_REG_BASE_ADDR, 0x34)
#define FDE_AES_SW_RESET                GET_REG_ADDR(FDE_REG_BASE_ADDR, 0x38)

#define FDE_AES_XEX_HW_T_CALC_KEY       GET_REG_ADDR(FDE_REG_BASE_ADDR, 0x40)	//0x40~0x4F

#define FDE_AES_DATA_UNIT0              GET_REG_ADDR(FDE_REG_BASE_ADDR, 0x50)
#define FDE_AES_DATA_UNIT1              GET_REG_ADDR(FDE_REG_BASE_ADDR, 0x51)

#define FDE_AES_FUN                     GET_REG_ADDR(FDE_REG_BASE_ADDR, 0x60)
#define FDE_AES_DBG_BUS                 GET_REG_ADDR(FDE_REG_BASE_ADDR, 0x61)


//FDE_AES_BUSY 0x20
#define BIT_AES_BUSY                    BIT0
//FDE_AES_CONTROL0 0x30
#define BIT_ENC_OR_DEC              BIT0
#define ENABLE_ENC					0
#define ENABLE_DEC					1
#define BIT_MASK_AES_MODE			(BIT2|BIT3|BIT4|BIT5|BIT6)
#define BIT_AES_MODE_SHIFT			2
#define ECB_MODE				    0	//set 0
#define CBC_MODE				    1
#define CTR_MODE				    2
#define OFB_MODE				    5
#define CFB_MODE				    6
#define XTS_MODE				    8



#define BIT_MASK_KEY_SIZE			(BIT12|BIT13)
#define BIT_KEY_128BIT_SIZE			BIT_MASK_KEY_SIZE
#define BIT_KEY_192BIT_SIZE			BIT12
#define BIT_KEY_256BIT_SIZE			BIT13

//FDE_AES_XEX_HW_T_CALC_KICK  0x34
#define BIT_AES_XEX_HW_T_CALC_KICK  BIT0

//FDE_AES_SW_RESET  0x38
#define BIT_AES_SW_RESET            BIT0

//FDE_AES_FUN  0x60
#define BIT_AES_FUNC_EN             BIT0
#define BIT_AES_SWAP                BIT1
#define BIT_AES_AUTO_CONFIG         BIT2
#define BIT_AES_AUTO_CONFIG_CMDQ    BIT3
#define BIT_AES_SWITCH_ON			BIT4


#endif
