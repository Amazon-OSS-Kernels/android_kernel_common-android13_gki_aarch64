// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (c) 2023 MediaTek Inc.
 */


#ifndef _MTK_DTV_PWM_DAC_H_
#define _MTK_DTV_PWM_DAC_H_

////////////////////////////////////////////////////////////////////////////////
// Header Files
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Define & and data type
////////////////////////////////////////////////////////////////////////////////

//############################
//
//IP bank address : for pad mux in chiptop
//
//############################
#define GPIO_FUNC_MUX_REG_BASE          (0x1F000000 + (0x322900 << 1))

//for pwm dac0
#define CHIP_REG_PWM_DAC0                       (GPIO_FUNC_MUX_REG_BASE + 0x49 * 4)
#define CHIP_PWM_DAC0_PAD_0                     0
#define CHIP_PWM_DAC0_PAD_1                     BIT(0)	//PAD_DDCR_CK
#define CHIP_PWM_DAC0_PAD_MSK                   BIT(0)

//for pwm dac1
#define CHIP_REG_PWM_DAC1                       (GPIO_FUNC_MUX_REG_BASE + 0x49 * 4)
#define CHIP_PWM_DAC1_PAD_0                     (0)
#define CHIP_PWM_DAC1_PAD_1                     BIT(4)	//PAD_DDCR_DA
#define CHIP_PWM_DAC1_PAD_MSK                   BIT(4)

// for all pwms oen
#define CHIP_REG_PWM_DAC_OEN                    (GPIO_FUNC_MUX_REG_BASE + 0x68 * 4 + 1)
#define CHIP_PWMS_DAC_OEN_MSK                   (BIT(0) | BIT(1))

//############################
//
//PWM dac control registers
//
//############################

#define REG_PWM0_DAC_PERIOD                     (0x00)	//bit0~15
#define REG_PWM0_DAC_DUTY                       (0x01)	//bit0~15
#define REG_PWM0_DAC_DIV                        (0x02)	//bit0~7
#define REG_PWM0_DAC_DBEN                       (0x02)	//bit8
#define REG_PWM0_DAC_POLARITY                   (0x02)	//bit9
#define REG_PWM0_DAC_DUTY_OFFSET                (0x03)	//bit0~7
#define REG_PWM0_DAC_DUTY_AUTO_CORRECT          (0x04)	//bit0

#define REG_PWM1_DAC_PERIOD                     (0x10)	//bit0~15
#define REG_PWM1_DAC_DUTY                       (0x11)	//bit0~15
#define REG_PWM1_DAC_DIV                        (0x12)	//bit0~7
#define REG_PWM1_DAC_DBEN                       (0x12)	//bit8
#define REG_PWM1_DAC_POLARITY                   (0x12)	//bit9
#define REG_PWM1_DAC_DUTY_OFFSET                (0x13)	//bit0~7
#define REG_PWM1_DAC_DUTY_AUTO_CORRECT          (0x14)	//bit0

#define REG_PWM_DAC_DBEN_MASK                   BIT(8)
#define REG_PWM_DAC_POLARITY_MASK               BIT(9)
#define REG_PWM_DAC_DUTY_AUTO_CORRECT_MASK      BIT(0)

#define REG_CKG_PWM_DAC                         (0x70)
#define PWM_DAC_CLK_EN_MASK                     BIT(0)
#define PWM_DAC_CLK_INV_MASK                    BIT(1)
#define PWM_DAC_CLK_SEL_MASK                    (BIT(2) | BIT(3))

#endif // _MTK_DTV_PWM_DAC_H_
