// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (c) 2023 MediaTek Inc.
 */


#ifndef _MTK_DTV_PWM_PM_H_
#define _MTK_DTV_PWM_PM_H_

////////////////////////////////////////////////////////////////////////////////
// Header Files
////////////////////////////////////////////////////////////////////////////////

//############################
//
//PWM pm control registers
//
//############################
#define REG_PM_PWM0_IS_GPIO         (0x1C)	//bit5, 0: PWM_PM0, 1: GPIO
#define REG_PM_PWM0_IS_GPIO_MASK    BIT(5)

#define REG_PWM0_PM_SW_RST          (0x31)	//bit10
#define REG_PWM0_PM_SW_RST_MASK     BIT(10)

#define REG_PWM0_PM_DIV             (0x68)	//bit0~7
#define REG_PWM0_PM_DUTY            (0x69)	//bit0~15
#define REG_PWM0_PM_PERIOD          (0x6A)	//bit0~15
#define REG_PWM0_PM_POLARITY        (0x6B)	//bit0
#define REG_PWM0_PM_DBEN            (0x6B)	//bit1

#define REG_GPIO_IS_PWM1            (0x76)	//bit11~12

#define REG_PWM1_PM_DIV             (0x78)	//bit0~7
#define REG_PWM1_PM_DUTY            (0x79)	//bit0~15
#define REG_PWM1_PM_PERIOD          (0x7A)	//bit0~15
#define REG_PWM1_PM_POLARITY        (0x7B)	//bit0
#define REG_PWM1_PM_DBEN            (0x7B)	//bit1

#define REG_PWM_PM_POLARITY_MASK    BIT(0)
#define REG_PWM_PM_DBEN_MASK        BIT(1)

#define REG_PWM1_PM_SW_RST          (0x7B)	//bit8
#define REG_PWM1_PM_SW_RST_MASK     BIT(8)

#endif // _MTK_DTV_PWM_PM_H_
