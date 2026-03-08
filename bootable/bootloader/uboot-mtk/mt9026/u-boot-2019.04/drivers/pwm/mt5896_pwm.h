/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * MediaTek Inc. (C) 2020. All rights reserved.
 */

#ifndef _MT5896_PWM_H_
#define _MT5896_PWM_H_

////////////////////////////////////////////////////////////////////////////////
// Header Files
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Define & and data type
////////////////////////////////////////////////////////////////////////////////

//
//PWM control registers
//
#define REG_PWM_CLKALLEN			(0x1EC)
#define REG_PWM_CLKEN_BITMAP        (0x2C)
#define PWM0_CLK_EN_MASK            BIT(0)
#define PWMFULL_IP_ENGINE_EN_MASK   BIT(8)
#define PWM_OFFSET_BASE				0x40

#define REG_PWM0_PERIOD             (0x00)	//bit0~15
#define REG_PWM0_PERIOD_EXT         (0x04)	//bit0~7

#define REG_PWM0_DUTY               (0x08)	//bit0~15
#define REG_PWM0_DUTY_EXT           (0x0C)	//bit0~7

#define REG_PWM0_DIV                (0x18)	//bit0~15

#define REG_PWM0_PORARITY           (0x1C)	//bit8
#define REG_PWM_POLARITY_MASK       BIT(8)

#define REG_PWM0_VDBEN              (0x1C)	//bit9
#define REG_PWM_VDBEN_MASK          BIT(9)

#define REG_PWM0_RESET_EN           (0x1C)	//bit10
#define REG_PWM_RESET_EN_MASK       BIT(10)

#define REG_PWM0_DBEN               (0x1C)	//bit11
#define REG_PWM_DBEN_MASK           BIT(11)

#define REG_PWM0_IMPULSE_EN         (0x1C)	//bit12
#define REG_PWM_IMPULSE_EN_MASK     BIT(12)

#define REG_PWM0_ODDEVEN_SYNC       (0x1C)	//bit13
#define REG_PWM_ODDVEN_SYNC_MASK    BIT(13)

#define REG_PWM0_VDBEN_SW           (0x1C)	//bit14
#define REG_PWM_VDBEN_SW_MASK       BIT(14)

#define REG_RST_MUX0                (0x28)	//bit12
#define REG_HS_RST_CNT0             (0x28)	//bit8~11
#define REG_PWM_RST_CNT_MASK_L      GENMASK(11, 8)
#define REG_PWM_RST_MUX_EN_MASK_L	BIT(12)

#define REG_PWM0_SHIFT_L            (0x20)	//bit0~15
#define REG_PWM0_SHIFT_H            (0x24)	//bit0~7

#define REG_PWM0_NVS                (0x28)	//bit13
#define REG_PWM0_Align              (0x28)	//bit14

#define REG_CLK_PWM_SW_EN			(0x1844)
#define REG_CLK_XTAL_SW_EN			(0x1894)
#define XTAL_SW_EN_BIT				BIT(1)
#define PWM_SW_EN_BIT				BIT(0)

#define REG_PWM_FORCE_SHIFT_SET_EN	(0x1C)	//bit13
#define PWM_FORCE_SHIFT_SET    		BIT(0)
#define XTAL_HZ						12000000

#endif // _MT5896_PWM_H_
