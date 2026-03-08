// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

#ifndef _MT5896_PM_PWM_H_
#define _MT5896_PM_PWM_H_

////////////////////////////////////////////////////////////////////////////////
// Header Files
////////////////////////////////////////////////////////////////////////////////

#define PWM_PM_MAX_CHANNEL          (6)

#define REG_PWM0_PM_PERIOD_L        (0x00)  //bit0~15
#define REG_PWM1_PM_PERIOD_L        (0x08)  //bit0~15
#define REG_PWM2_PM_PERIOD_L        (0x10)  //bit0~15
#define REG_PWM3_PM_PERIOD_L        (0x18)  //bit0~15
#define REG_PWM4_PM_PERIOD_L        (0x20)  //bit0~15
#define REG_PWM5_PM_PERIOD_L        (0x28)  //bit0~15

#define REG_PWM0_PM_PERIOD_H        (0x01)  //bit0~1
#define REG_PWM1_PM_PERIOD_H        (0x09)  //bit0~1
#define REG_PWM2_PM_PERIOD_H        (0x11)  //bit0~1
#define REG_PWM3_PM_PERIOD_H        (0x19)  //bit0~1
#define REG_PWM4_PM_PERIOD_H        (0x21)  //bit0~1
#define REG_PWM5_PM_PERIOD_H        (0x29)  //bit0~1

#define REG_PWM0_PM_DUTY_L          (0x02)  //bit0~15
#define REG_PWM1_PM_DUTY_L          (0x0A)  //bit0~15
#define REG_PWM2_PM_DUTY_L          (0x12)  //bit0~15
#define REG_PWM3_PM_DUTY_L          (0x1A)  //bit0~15
#define REG_PWM4_PM_DUTY_L          (0x22)  //bit0~15
#define REG_PWM5_PM_DUTY_L          (0x2A)  //bit0~15

#define REG_PWM0_PM_DUTY_H          (0x03)  //bit0~1
#define REG_PWM1_PM_DUTY_H          (0x0B)  //bit0~1
#define REG_PWM2_PM_DUTY_H          (0x13)  //bit0~1
#define REG_PWM3_PM_DUTY_H          (0x1B)  //bit0~1
#define REG_PWM4_PM_DUTY_H          (0x23)  //bit0~1
#define REG_PWM5_PM_DUTY_H          (0x2B)  //bit0~1

#define REG_PWM0_PM_DIV             (0x04)  //bit0~15
#define REG_PWM1_PM_DIV             (0x0C)  //bit0~15
#define REG_PWM2_PM_DIV             (0x14)  //bit0~15
#define REG_PWM3_PM_DIV             (0x1C)  //bit0~15
#define REG_PWM4_PM_DIV             (0x24)  //bit0~15
#define REG_PWM5_PM_DIV             (0x2C)  //bit0~15

#define REG_PWM0_PM_EN              (0x05)  //bit0
#define REG_PWM1_PM_EN              (0x0D)  //bit0
#define REG_PWM2_PM_EN              (0x15)  //bit0
#define REG_PWM3_PM_EN              (0x1D)  //bit0
#define REG_PWM4_PM_EN              (0x25)  //bit0
#define REG_PWM5_PM_EN              (0x2D)  //bit0
#define REG_PWM_PM_EN_MASK          BIT(0)

#define REG_PWM0_PM_PD_VALUE        (0x05)  //bit1
#define REG_PWM1_PM_PD_VALUE        (0x0D)  //bit1
#define REG_PWM2_PM_PD_VALUE        (0x15)  //bit1
#define REG_PWM3_PM_PD_VALUE        (0x1D)  //bit1
#define REG_PWM4_PM_PD_VALUE        (0x25)  //bit1
#define REG_PWM5_PM_PD_VALUE        (0x2D)  //bit1
#define REG_PWM_PM_PD_VALUE_MASK    BIT(1)

#define REG_PWM0_PM_POLARITY        (0x05)  //bit8
#define REG_PWM1_PM_POLARITY        (0x0D)  //bit8
#define REG_PWM2_PM_POLARITY        (0x15)  //bit8
#define REG_PWM3_PM_POLARITY        (0x1D)  //bit8
#define REG_PWM4_PM_POLARITY        (0x25)  //bit8
#define REG_PWM5_PM_POLARITY        (0x2D)  //bit8
#define REG_PWM_PM_POLARITY_MASK    BIT(8)

#define REG_PWM0_PM_DBEN            (0x05)  //bit9
#define REG_PWM1_PM_DBEN            (0x0D)  //bit9
#define REG_PWM2_PM_DBEN            (0x15)  //bit9
#define REG_PWM3_PM_DBEN            (0x1D)  //bit9
#define REG_PWM4_PM_DBEN            (0x25)  //bit9
#define REG_PWM5_PM_DBEN            (0x2D)  //bit9
#define REG_PWM_PM_DBEN_MASK        BIT(9)

#define REG_PWM0_PM_SW_RST          (0x05)  //bit12
#define REG_PWM1_PM_SW_RST          (0x0D)  //bit12
#define REG_PWM2_PM_SW_RST          (0x15)  //bit12
#define REG_PWM3_PM_SW_RST          (0x1D)  //bit12
#define REG_PWM4_PM_SW_RST          (0x25)  //bit12
#define REG_PWM5_PM_SW_RST          (0x2D)  //bit12
#define REG_PWM_PM_SW_RST_MASK      BIT(12)

#endif // _MT5896_PM_PWM_H_
