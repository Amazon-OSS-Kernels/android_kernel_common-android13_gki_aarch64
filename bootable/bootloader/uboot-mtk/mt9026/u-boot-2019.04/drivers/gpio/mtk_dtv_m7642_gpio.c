// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <linux/io.h>
#include <linux/bitmap.h>
#include <asm/io.h>
#include <asm/gpio.h>
#include <dt-bindings/gpio/gpio.h>
#include <dt-bindings/gpio/mt5888-gpio.h>

// set all pads (except SPI) as input
#define REG_ALL_PAD_IN      (0x3229e0UL)

#define _CONCAT(a, b)     a##b
#define CONCAT(a, b)      _CONCAT(a, b)

// Dummy
#define GPIO999_OEN     0, 0
#define GPIO999_OUT     0, 0
#define GPIO999_IN      0, 0

#define GPIO1_PAD PAD_DDCA_CK
#define GPIO1_OEN 0x0494, BIT(1)
#define GPIO1_OUT 0x0494, BIT(2)
#define GPIO1_IN  0x0494, BIT(0)

#define GPIO2_PAD PAD_DDCA_DA
#define GPIO2_OEN 0x0494, BIT(5)
#define GPIO2_OUT 0x0494, BIT(6)
#define GPIO2_IN  0x0494, BIT(4)

#define GPIO3_PAD PAD_IRIN
#define GPIO3_OEN 0x0f26, BIT(2)
#define GPIO3_OUT 0x0f26, BIT(2)
#define GPIO3_IN  0x0f26, BIT(2)

#define GPIO4_PAD PAD_PWM_PM
#define GPIO4_OEN 0x0f28, BIT(0)
#define GPIO4_OUT 0x0f28, BIT(1)
#define GPIO4_IN  0x0f28, BIT(2)

#define GPIO5_PAD PAD_CEC0
#define GPIO5_OEN 0x0f2a, BIT(0)
#define GPIO5_OUT 0x0f2a, BIT(1)
#define GPIO5_IN  0x0f2a, BIT(2)

#define GPIO6_PAD PAD_GPIO0_PM
#define GPIO6_OEN 0x0f00, BIT(0)
#define GPIO6_OUT 0x0f00, BIT(1)
#define GPIO6_IN  0x0f00, BIT(2)

#define GPIO7_PAD PAD_GPIO1_PM
#define GPIO7_OEN 0x0f02, BIT(0)
#define GPIO7_OUT 0x0f02, BIT(1)
#define GPIO7_IN  0x0f02, BIT(2)

#define GPIO8_PAD PAD_GPIO2_PM
#define GPIO8_OEN 0x0f04, BIT(0)
#define GPIO8_OUT 0x0f04, BIT(1)
#define GPIO8_IN  0x0f04, BIT(2)

#define GPIO9_PAD PAD_USB_CTRL
#define GPIO9_OEN 0x0f06, BIT(0)
#define GPIO9_OUT 0x0f06, BIT(1)
#define GPIO9_IN  0x0f06, BIT(2)

#define GPIO10_PAD PAD_GPIO5_PM
#define GPIO10_OEN 0x0f0a, BIT(0)
#define GPIO10_OUT 0x0f0a, BIT(1)
#define GPIO10_IN  0x0f0a, BIT(2)

#define GPIO11_PAD PAD_GPIO6_PM
#define GPIO11_OEN 0x0f0c, BIT(0)
#define GPIO11_OUT 0x0f0c, BIT(1)
#define GPIO11_IN  0x0f0c, BIT(2)

#define GPIO12_PAD PAD_GPIO7_PM
#define GPIO12_OEN 0x0f0e, BIT(0)
#define GPIO12_OUT 0x0f0e, BIT(1)
#define GPIO12_IN  0x0f0e, BIT(2)

#define GPIO13_PAD PAD_GPIO8_PM
#define GPIO13_OEN 0x0f10, BIT(0)
#define GPIO13_OUT 0x0f10, BIT(1)
#define GPIO13_IN  0x0f10, BIT(2)

#define GPIO14_PAD PAD_GPIO9_PM
#define GPIO14_OEN 0x0f12, BIT(0)
#define GPIO14_OUT 0x0f12, BIT(1)
#define GPIO14_IN  0x0f12, BIT(2)

#define GPIO15_PAD PAD_GPIO10_PM
#define GPIO15_OEN 0x0f14, BIT(0)
#define GPIO15_OUT 0x0f14, BIT(1)
#define GPIO15_IN  0x0f14, BIT(2)

#define GPIO16_PAD PAD_GPIO11_PM
#define GPIO16_OEN 0x0f16, BIT(0)
#define GPIO16_OUT 0x0f16, BIT(1)
#define GPIO16_IN  0x0f16, BIT(2)

#define GPIO17_PAD PAD_GPIO12_PM
#define GPIO17_OEN 0x0f18, BIT(0)
#define GPIO17_OUT 0x0f18, BIT(1)
#define GPIO17_IN  0x0f18, BIT(2)

#define GPIO18_PAD PAD_GPIO13_PM
#define GPIO18_OEN 0x0f1a, BIT(0)
#define GPIO18_OUT 0x0f1a, BIT(1)
#define GPIO18_IN  0x0f1a, BIT(2)

#define GPIO19_PAD PAD_GPIO14_PM
#define GPIO19_OEN 0x0f1c, BIT(0)
#define GPIO19_OUT 0x0f1c, BIT(1)
#define GPIO19_IN  0x0f1c, BIT(2)

#define GPIO20_PAD PAD_GPIO15_PM
#define GPIO20_OEN 0x0f1e, BIT(0)
#define GPIO20_OUT 0x0f1e, BIT(1)
#define GPIO20_IN  0x0f1e, BIT(2)

#define GPIO21_PAD PAD_GPIO16_PM
#define GPIO21_OEN 0x0f20, BIT(0)
#define GPIO21_OUT 0x0f20, BIT(1)
#define GPIO21_IN  0x0f20, BIT(2)

#define GPIO22_PAD PAD_GPIO17_PM
#define GPIO22_OEN 0x0f48, BIT(0)
#define GPIO22_OUT 0x0f48, BIT(1)
#define GPIO22_IN  0x0f48, BIT(2)

#define GPIO23_PAD PAD_GPIO18_PM
#define GPIO23_OEN 0x0f4a, BIT(0)
#define GPIO23_OUT 0x0f4a, BIT(1)
#define GPIO23_IN  0x0f4a, BIT(2)

#define GPIO24_PAD PAD_GPIO19_PM
#define GPIO24_OEN 0x0f4c, BIT(0)
#define GPIO24_OUT 0x0f4c, BIT(1)
#define GPIO24_IN  0x0f4c, BIT(2)

#define GPIO25_PAD PAD_GPIO20_PM
#define GPIO25_OEN 0x0f4e, BIT(0)
#define GPIO25_OUT 0x0f4e, BIT(1)
#define GPIO25_IN  0x0f4e, BIT(2)

#define GPIO26_PAD PAD_GPIO21_PM
#define GPIO26_OEN 0x0f50, BIT(0)
#define GPIO26_OUT 0x0f50, BIT(1)
#define GPIO26_IN  0x0f50, BIT(2)

#define GPIO27_PAD PAD_GPIO22_PM
#define GPIO27_OEN 0x0f52, BIT(0)
#define GPIO27_OUT 0x0f52, BIT(1)
#define GPIO27_IN  0x0f52, BIT(2)

#define GPIO28_PAD PAD_GPIO23_PM
#define GPIO28_OEN 0x0f54, BIT(0)
#define GPIO28_OUT 0x0f54, BIT(1)
#define GPIO28_IN  0x0f54, BIT(2)

#define GPIO29_PAD PAD_GPIO24_PM
#define GPIO29_OEN 0x0f56, BIT(0)
#define GPIO29_OUT 0x0f56, BIT(1)
#define GPIO29_IN  0x0f56, BIT(2)

#define GPIO30_PAD PAD_GPIO25_PM
#define GPIO30_OEN 0x0f58, BIT(0)
#define GPIO30_OUT 0x0f58, BIT(1)
#define GPIO30_IN  0x0f58, BIT(2)

#define GPIO31_PAD PAD_GPIO26_PM
#define GPIO31_OEN 0x0f5a, BIT(0)
#define GPIO31_OUT 0x0f5a, BIT(1)
#define GPIO31_IN  0x0f5a, BIT(2)

#define GPIO32_PAD PAD_GPIO27_PM
#define GPIO32_OEN 0x0f5c, BIT(0)
#define GPIO32_OUT 0x0f5c, BIT(1)
#define GPIO32_IN  0x0f5c, BIT(2)

#define GPIO33_PAD PAD_GPIO28_PM
#define GPIO33_OEN 0x0f5e, BIT(0)
#define GPIO33_OUT 0x0f5e, BIT(1)
#define GPIO33_IN  0x0f5e, BIT(2)

#define GPIO34_PAD PAD_DDCDA_CK
#define GPIO34_OEN 0x0496, BIT(1)
#define GPIO34_OUT 0x0496, BIT(2)
#define GPIO34_IN  0x0496, BIT(0)

#define GPIO35_PAD PAD_DDCDA_DA
#define GPIO35_OEN 0x0496, BIT(5)
#define GPIO35_OUT 0x0496, BIT(6)
#define GPIO35_IN  0x0496, BIT(4)

#define GPIO36_PAD PAD_DDCDB_CK
#define GPIO36_OEN 0x0497, BIT(1)
#define GPIO36_OUT 0x0497, BIT(2)
#define GPIO36_IN  0x0497, BIT(0)

#define GPIO37_PAD PAD_DDCDB_DA
#define GPIO37_OEN 0x0497, BIT(5)
#define GPIO37_OUT 0x0497, BIT(6)
#define GPIO37_IN  0x0497, BIT(4)

#define GPIO38_PAD PAD_DDCDC_CK
#define GPIO38_OEN 0x0498, BIT(1)
#define GPIO38_OUT 0x0498, BIT(2)
#define GPIO38_IN  0x0498, BIT(0)

#define GPIO39_PAD PAD_DDCDC_DA
#define GPIO39_OEN 0x0498, BIT(5)
#define GPIO39_OUT 0x0498, BIT(6)
#define GPIO39_IN  0x0498, BIT(4)

#define GPIO40_PAD PAD_DDCDD_CK
#define GPIO40_OEN 0x0499, BIT(1)
#define GPIO40_OUT 0x0499, BIT(2)
#define GPIO40_IN  0x0499, BIT(0)

#define GPIO41_PAD PAD_DDCDD_DA
#define GPIO41_OEN 0x0499, BIT(5)
#define GPIO41_OUT 0x0499, BIT(6)
#define GPIO41_IN  0x0499, BIT(4)

#define GPIO42_PAD PAD_SAR0
#define GPIO42_OEN 0x1423, BIT(0)
#define GPIO42_OUT 0x1424, BIT(0)
#define GPIO42_IN  0x1425, BIT(0)

#define GPIO43_PAD PAD_SAR1
#define GPIO43_OEN 0x1423, BIT(1)
#define GPIO43_OUT 0x1424, BIT(1)
#define GPIO43_IN  0x1425, BIT(1)

#define GPIO44_PAD PAD_SAR2
#define GPIO44_OEN 0x1423, BIT(2)
#define GPIO44_OUT 0x1424, BIT(2)
#define GPIO44_IN  0x1425, BIT(2)

#define GPIO45_PAD PAD_SAR3
#define GPIO45_OEN 0x1423, BIT(3)
#define GPIO45_OUT 0x1424, BIT(3)
#define GPIO45_IN  0x1425, BIT(3)

#define GPIO46_PAD PAD_SAR4
#define GPIO46_OEN 0x1423, BIT(4)
#define GPIO46_OUT 0x1424, BIT(4)
#define GPIO46_IN  0x1425, BIT(4)

#define GPIO47_PAD PAD_VPLUGIN
#define GPIO47_OEN 0x1423, BIT(5)
#define GPIO47_OUT 0x1424, BIT(5)
#define GPIO47_IN  0x1425, BIT(5)

#define GPIO48_PAD PAD_SAR6
#define GPIO48_OEN 0x1423, BIT(6)
#define GPIO48_OUT 0x1424, BIT(6)
#define GPIO48_IN  0x1425, BIT(6)

#define GPIO49_PAD PAD_SAR7
#define GPIO49_OEN 0x1423, BIT(7)
#define GPIO49_OUT 0x1424, BIT(7)
#define GPIO49_IN  0x1425, BIT(7)

#define GPIO50_PAD PAD_VID0
#define GPIO50_OEN 0x2e84, BIT(1)
#define GPIO50_OUT 0x2e84, BIT(0)
#define GPIO50_IN  0x2e84, BIT(2)

#define GPIO51_PAD PAD_VID1
#define GPIO51_OEN 0x2e85, BIT(1)
#define GPIO51_OUT 0x2e85, BIT(0)
#define GPIO51_IN  0x2e85, BIT(2)

#define GPIO52_PAD PAD_VID2
#define GPIO52_OEN 0x0f22, BIT(0)
#define GPIO52_OUT 0x0f22, BIT(1)
#define GPIO52_IN  0x0f22, BIT(2)

#define GPIO53_PAD PAD_WOL_INT_OUT
#define GPIO53_OEN 0x2e82, BIT(1)
#define GPIO53_OUT 0x2e82, BIT(0)
#define GPIO53_IN  0x2e82, BIT(2)

#define GPIO54_PAD PAD_DDCR_CK
#define GPIO54_OEN 0x322ee2, BIT(1)
#define GPIO54_OUT 0x322ee2, BIT(0)
#define GPIO54_IN  0x322ee2, BIT(2)

#define GPIO55_PAD PAD_DDCR_DA
#define GPIO55_OEN 0x322ee0, BIT(1)
#define GPIO55_OUT 0x322ee0, BIT(0)
#define GPIO55_IN  0x322ee0, BIT(2)

#define GPIO56_PAD PAD_DIM0
#define GPIO56_OEN 0x322f40, BIT(1)
#define GPIO56_OUT 0x322f40, BIT(0)
#define GPIO56_IN  0x322f40, BIT(2)

#define GPIO57_PAD PAD_DIM1
#define GPIO57_OEN 0x322f42, BIT(1)
#define GPIO57_OUT 0x322f42, BIT(0)
#define GPIO57_IN  0x322f42, BIT(2)

#define GPIO58_PAD PAD_DIM2
#define GPIO58_OEN 0x322f44, BIT(1)
#define GPIO58_OUT 0x322f44, BIT(0)
#define GPIO58_IN  0x322f44, BIT(2)

#define GPIO59_PAD PAD_DIM3
#define GPIO59_OEN 0x322f46, BIT(1)
#define GPIO59_OUT 0x322f46, BIT(0)
#define GPIO59_IN  0x322f46, BIT(2)

#define GPIO60_PAD PAD_GPIO0
#define GPIO60_OEN 0x322b00, BIT(1)
#define GPIO60_OUT 0x322b00, BIT(0)
#define GPIO60_IN  0x322b00, BIT(2)

#define GPIO61_PAD PAD_GPIO1
#define GPIO61_OEN 0x322b02, BIT(1)
#define GPIO61_OUT 0x322b02, BIT(0)
#define GPIO61_IN  0x322b02, BIT(2)

#define GPIO62_PAD PAD_GPIO8
#define GPIO62_OEN 0x322b10, BIT(1)
#define GPIO62_OUT 0x322b10, BIT(0)
#define GPIO62_IN  0x322b10, BIT(2)

#define GPIO63_PAD PAD_GPIO9
#define GPIO63_OEN 0x322b12, BIT(1)
#define GPIO63_OUT 0x322b12, BIT(0)
#define GPIO63_IN  0x322b12, BIT(2)

#define GPIO64_PAD PAD_GPIO10
#define GPIO64_OEN 0x322b14, BIT(1)
#define GPIO64_OUT 0x322b14, BIT(0)
#define GPIO64_IN  0x322b14, BIT(2)

#define GPIO65_PAD PAD_GPIO11
#define GPIO65_OEN 0x322b16, BIT(1)
#define GPIO65_OUT 0x322b16, BIT(0)
#define GPIO65_IN  0x322b16, BIT(2)

#define GPIO66_PAD PAD_GPIO12
#define GPIO66_OEN 0x322b18, BIT(1)
#define GPIO66_OUT 0x322b18, BIT(0)
#define GPIO66_IN  0x322b18, BIT(2)

#define GPIO67_PAD PAD_GPIO13
#define GPIO67_OEN 0x322b1a, BIT(1)
#define GPIO67_OUT 0x322b1a, BIT(0)
#define GPIO67_IN  0x322b1a, BIT(2)

#define GPIO68_PAD PAD_GPIO14
#define GPIO68_OEN 0x322b1c, BIT(1)
#define GPIO68_OUT 0x322b1c, BIT(0)
#define GPIO68_IN  0x322b1c, BIT(2)

#define GPIO69_PAD PAD_GPIO15
#define GPIO69_OEN 0x322b1e, BIT(1)
#define GPIO69_OUT 0x322b1e, BIT(0)
#define GPIO69_IN  0x322b1e, BIT(2)

#define GPIO70_PAD PAD_GPIO16
#define GPIO70_OEN 0x322b20, BIT(1)
#define GPIO70_OUT 0x322b20, BIT(0)
#define GPIO70_IN  0x322b20, BIT(2)

#define GPIO71_PAD PAD_GPIO17
#define GPIO71_OEN 0x322b22, BIT(1)
#define GPIO71_OUT 0x322b22, BIT(0)
#define GPIO71_IN  0x322b22, BIT(2)

#define GPIO72_PAD PAD_GPIO18
#define GPIO72_OEN 0x322b24, BIT(1)
#define GPIO72_OUT 0x322b24, BIT(0)
#define GPIO72_IN  0x322b24, BIT(2)

#define GPIO73_PAD PAD_I2S_IN_BCK
#define GPIO73_OEN 0x322e00, BIT(1)
#define GPIO73_OUT 0x322e00, BIT(0)
#define GPIO73_IN  0x322e00, BIT(2)

#define GPIO74_PAD PAD_I2S_IN_DIN0
#define GPIO74_OEN 0x322e04, BIT(1)
#define GPIO74_OUT 0x322e04, BIT(0)
#define GPIO74_IN  0x322e04, BIT(2)

#define GPIO75_PAD PAD_I2S_IN_DIN1
#define GPIO75_OEN 0x322e06, BIT(1)
#define GPIO75_OUT 0x322e06, BIT(0)
#define GPIO75_IN  0x322e06, BIT(2)

#define GPIO76_PAD PAD_I2S_IN_DIN2
#define GPIO76_OEN 0x322e08, BIT(1)
#define GPIO76_OUT 0x322e08, BIT(0)
#define GPIO76_IN  0x322e08, BIT(2)

#define GPIO77_PAD PAD_I2S_IN_WCK
#define GPIO77_OEN 0x322e02, BIT(1)
#define GPIO77_OUT 0x322e02, BIT(0)
#define GPIO77_IN  0x322e02, BIT(2)

#define GPIO78_PAD PAD_I2S_OUT_BCK
#define GPIO78_OEN 0x322e20, BIT(1)
#define GPIO78_OUT 0x322e20, BIT(0)
#define GPIO78_IN  0x322e20, BIT(2)

#define GPIO79_PAD PAD_I2S_OUT_MCK
#define GPIO79_OEN 0x322e22, BIT(1)
#define GPIO79_OUT 0x322e22, BIT(0)
#define GPIO79_IN  0x322e22, BIT(2)

#define GPIO80_PAD PAD_I2S_OUT_SD0
#define GPIO80_OEN 0x322e26, BIT(1)
#define GPIO80_OUT 0x322e26, BIT(0)
#define GPIO80_IN  0x322e26, BIT(2)

#define GPIO81_PAD PAD_I2S_OUT_SD1
#define GPIO81_OEN 0x322e28, BIT(1)
#define GPIO81_OUT 0x322e28, BIT(0)
#define GPIO81_IN  0x322e28, BIT(2)

#define GPIO82_PAD PAD_I2S_OUT_SD2
#define GPIO82_OEN 0x322e2a, BIT(1)
#define GPIO82_OUT 0x322e2a, BIT(0)
#define GPIO82_IN  0x322e2a, BIT(2)

#define GPIO83_PAD PAD_I2S_OUT_WCK
#define GPIO83_OEN 0x322e24, BIT(1)
#define GPIO83_OUT 0x322e24, BIT(0)
#define GPIO83_IN  0x322e24, BIT(2)

#define GPIO84_PAD PAD_LD_SPI_CK
#define GPIO84_OEN 0x322f26, BIT(1)
#define GPIO84_OUT 0x322f26, BIT(0)
#define GPIO84_IN  0x322f26, BIT(2)

#define GPIO85_PAD PAD_LD_SPI_CS
#define GPIO85_OEN 0x322f22, BIT(1)
#define GPIO85_OUT 0x322f22, BIT(0)
#define GPIO85_IN  0x322f22, BIT(2)

#define GPIO86_PAD PAD_LD_SPI_MISO
#define GPIO86_OEN 0x322f20, BIT(1)
#define GPIO86_OUT 0x322f20, BIT(0)
#define GPIO86_IN  0x322f20, BIT(2)

#define GPIO87_PAD PAD_LD_SPI_MOSI
#define GPIO87_OEN 0x322f24, BIT(1)
#define GPIO87_OUT 0x322f24, BIT(0)
#define GPIO87_IN  0x322f24, BIT(2)

#define GPIO88_PAD PAD_MIC_BCK
#define GPIO88_OEN 0x0f66, BIT(0)
#define GPIO88_OUT 0x0f66, BIT(1)
#define GPIO88_IN  0x0f66, BIT(2)

#define GPIO89_PAD PAD_MIC_SD0
#define GPIO89_OEN 0x0f68, BIT(0)
#define GPIO89_OUT 0x0f68, BIT(1)
#define GPIO89_IN  0x0f68, BIT(2)

#define GPIO90_PAD PAD_MIC_SD1
#define GPIO90_OEN 0x0f6a, BIT(0)
#define GPIO90_OUT 0x0f6a, BIT(1)
#define GPIO90_IN  0x0f6a, BIT(2)

#define GPIO91_PAD PAD_MIC_SD2
#define GPIO91_OEN 0x0f6c, BIT(0)
#define GPIO91_OUT 0x0f6c, BIT(1)
#define GPIO91_IN  0x0f6c, BIT(2)

#define GPIO92_PAD PAD_PCM2_CD_N
#define GPIO92_OEN 0x322d66, BIT(1)
#define GPIO92_OUT 0x322d66, BIT(0)
#define GPIO92_IN  0x322d66, BIT(2)

#define GPIO93_PAD PAD_PCM2_CE_N
#define GPIO93_OEN 0x322d68, BIT(1)
#define GPIO93_OUT 0x322d68, BIT(0)
#define GPIO93_IN  0x322d68, BIT(2)

#define GPIO94_PAD PAD_PCM2_IRQA_N
#define GPIO94_OEN 0x322d62, BIT(1)
#define GPIO94_OUT 0x322d62, BIT(0)
#define GPIO94_IN  0x322d62, BIT(2)

#define GPIO95_PAD PAD_PCM2_RESET
#define GPIO95_OEN 0x322d60, BIT(1)
#define GPIO95_OUT 0x322d60, BIT(0)
#define GPIO95_IN  0x322d60, BIT(2)

#define GPIO96_PAD PAD_PCM2_WAIT_N
#define GPIO96_OEN 0x322d64, BIT(1)
#define GPIO96_OUT 0x322d64, BIT(0)
#define GPIO96_IN  0x322d64, BIT(2)

#define GPIO97_PAD PAD_PCM_A0
#define GPIO97_OEN 0x322d00, BIT(1)
#define GPIO97_OUT 0x322d00, BIT(0)
#define GPIO97_IN  0x322d00, BIT(2)

#define GPIO98_PAD PAD_PCM_A1
#define GPIO98_OEN 0x322d02, BIT(1)
#define GPIO98_OUT 0x322d02, BIT(0)
#define GPIO98_IN  0x322d02, BIT(2)

#define GPIO99_PAD PAD_PCM_A2
#define GPIO99_OEN 0x322d04, BIT(1)
#define GPIO99_OUT 0x322d04, BIT(0)
#define GPIO99_IN  0x322d04, BIT(2)

#define GPIO100_PAD PAD_PCM_A3
#define GPIO100_OEN 0x322d06, BIT(1)
#define GPIO100_OUT 0x322d06, BIT(0)
#define GPIO100_IN  0x322d06, BIT(2)

#define GPIO101_PAD PAD_PCM_A4
#define GPIO101_OEN 0x322d08, BIT(1)
#define GPIO101_OUT 0x322d08, BIT(0)
#define GPIO101_IN  0x322d08, BIT(2)

#define GPIO102_PAD PAD_PCM_A5
#define GPIO102_OEN 0x322d0a, BIT(1)
#define GPIO102_OUT 0x322d0a, BIT(0)
#define GPIO102_IN  0x322d0a, BIT(2)

#define GPIO103_PAD PAD_PCM_A6
#define GPIO103_OEN 0x322d0c, BIT(1)
#define GPIO103_OUT 0x322d0c, BIT(0)
#define GPIO103_IN  0x322d0c, BIT(2)

#define GPIO104_PAD PAD_PCM_A7
#define GPIO104_OEN 0x322d0e, BIT(1)
#define GPIO104_OUT 0x322d0e, BIT(0)
#define GPIO104_IN  0x322d0e, BIT(2)

#define GPIO105_PAD PAD_PCM_A8
#define GPIO105_OEN 0x322d10, BIT(1)
#define GPIO105_OUT 0x322d10, BIT(0)
#define GPIO105_IN  0x322d10, BIT(2)

#define GPIO106_PAD PAD_PCM_A9
#define GPIO106_OEN 0x322d12, BIT(1)
#define GPIO106_OUT 0x322d12, BIT(0)
#define GPIO106_IN  0x322d12, BIT(2)

#define GPIO107_PAD PAD_PCM_A10
#define GPIO107_OEN 0x322d14, BIT(1)
#define GPIO107_OUT 0x322d14, BIT(0)
#define GPIO107_IN  0x322d14, BIT(2)

#define GPIO108_PAD PAD_PCM_A11
#define GPIO108_OEN 0x322d16, BIT(1)
#define GPIO108_OUT 0x322d16, BIT(0)
#define GPIO108_IN  0x322d16, BIT(2)

#define GPIO109_PAD PAD_PCM_A12
#define GPIO109_OEN 0x322d18, BIT(1)
#define GPIO109_OUT 0x322d18, BIT(0)
#define GPIO109_IN  0x322d18, BIT(2)

#define GPIO110_PAD PAD_PCM_A13
#define GPIO110_OEN 0x322d1a, BIT(1)
#define GPIO110_OUT 0x322d1a, BIT(0)
#define GPIO110_IN  0x322d1a, BIT(2)

#define GPIO111_PAD PAD_PCM_A14
#define GPIO111_OEN 0x322d1c, BIT(1)
#define GPIO111_OUT 0x322d1c, BIT(0)
#define GPIO111_IN  0x322d1c, BIT(2)

#define GPIO112_PAD PAD_PCM_CD_N
#define GPIO112_OEN 0x322d50, BIT(1)
#define GPIO112_OUT 0x322d50, BIT(0)
#define GPIO112_IN  0x322d50, BIT(2)

#define GPIO113_PAD PAD_PCM_CE_N
#define GPIO113_OEN 0x322d52, BIT(1)
#define GPIO113_OUT 0x322d52, BIT(0)
#define GPIO113_IN  0x322d52, BIT(2)

#define GPIO114_PAD PAD_PCM_D0
#define GPIO114_OEN 0x322d20, BIT(1)
#define GPIO114_OUT 0x322d20, BIT(0)
#define GPIO114_IN  0x322d20, BIT(2)

#define GPIO115_PAD PAD_PCM_D1
#define GPIO115_OEN 0x322d22, BIT(1)
#define GPIO115_OUT 0x322d22, BIT(0)
#define GPIO115_IN  0x322d22, BIT(2)

#define GPIO116_PAD PAD_PCM_D2
#define GPIO116_OEN 0x322d24, BIT(1)
#define GPIO116_OUT 0x322d24, BIT(0)
#define GPIO116_IN  0x322d24, BIT(2)

#define GPIO117_PAD PAD_PCM_D3
#define GPIO117_OEN 0x322d26, BIT(1)
#define GPIO117_OUT 0x322d26, BIT(0)
#define GPIO117_IN  0x322d26, BIT(2)

#define GPIO118_PAD PAD_PCM_D4
#define GPIO118_OEN 0x322d28, BIT(1)
#define GPIO118_OUT 0x322d28, BIT(0)
#define GPIO118_IN  0x322d28, BIT(2)

#define GPIO119_PAD PAD_PCM_D5
#define GPIO119_OEN 0x322d2a, BIT(1)
#define GPIO119_OUT 0x322d2a, BIT(0)
#define GPIO119_IN  0x322d2a, BIT(2)

#define GPIO120_PAD PAD_PCM_D6
#define GPIO120_OEN 0x322d2c, BIT(1)
#define GPIO120_OUT 0x322d2c, BIT(0)
#define GPIO120_IN  0x322d2c, BIT(2)

#define GPIO121_PAD PAD_PCM_D7
#define GPIO121_OEN 0x322d2e, BIT(1)
#define GPIO121_OUT 0x322d2e, BIT(0)
#define GPIO121_IN  0x322d2e, BIT(2)

#define GPIO122_PAD PAD_PCM_IORD_N
#define GPIO122_OEN 0x322d44, BIT(1)
#define GPIO122_OUT 0x322d44, BIT(0)
#define GPIO122_IN  0x322d44, BIT(2)

#define GPIO123_PAD PAD_PCM_IOWR_N
#define GPIO123_OEN 0x322d46, BIT(1)
#define GPIO123_OUT 0x322d46, BIT(0)
#define GPIO123_IN  0x322d46, BIT(2)

#define GPIO124_PAD PAD_PCM_IRQA_N
#define GPIO124_OEN 0x322d4a, BIT(1)
#define GPIO124_OUT 0x322d4a, BIT(0)
#define GPIO124_IN  0x322d4a, BIT(2)

#define GPIO125_PAD PAD_PCM_OE_N
#define GPIO125_OEN 0x322d42, BIT(1)
#define GPIO125_OUT 0x322d42, BIT(0)
#define GPIO125_IN  0x322d42, BIT(2)

#define GPIO126_PAD PAD_PCM_REG_N
#define GPIO126_OEN 0x322d4e, BIT(1)
#define GPIO126_OUT 0x322d4e, BIT(0)
#define GPIO126_IN  0x322d4e, BIT(2)

#define GPIO127_PAD PAD_PCM_RESET
#define GPIO127_OEN 0x322d40, BIT(1)
#define GPIO127_OUT 0x322d40, BIT(0)
#define GPIO127_IN  0x322d40, BIT(2)

#define GPIO128_PAD PAD_PCM_WAIT_N
#define GPIO128_OEN 0x322d4c, BIT(1)
#define GPIO128_OUT 0x322d4c, BIT(0)
#define GPIO128_IN  0x322d4c, BIT(2)

#define GPIO129_PAD PAD_PCM_WE_N
#define GPIO129_OEN 0x322d48, BIT(1)
#define GPIO129_OUT 0x322d48, BIT(0)
#define GPIO129_IN  0x322d48, BIT(2)

#define GPIO130_PAD PAD_PWM0
#define GPIO130_OEN 0x322f60, BIT(1)
#define GPIO130_OUT 0x322f60, BIT(0)
#define GPIO130_IN  0x322f60, BIT(2)

#define GPIO131_PAD PAD_PWM1
#define GPIO131_OEN 0x322f62, BIT(1)
#define GPIO131_OUT 0x322f62, BIT(0)
#define GPIO131_IN  0x322f62, BIT(2)

#define GPIO132_PAD PAD_PWM2
#define GPIO132_OEN 0x322f64, BIT(1)
#define GPIO132_OUT 0x322f64, BIT(0)
#define GPIO132_IN  0x322f64, BIT(2)

#define GPIO133_PAD PAD_SPDIF_IN
#define GPIO133_OEN 0x322e62, BIT(1)
#define GPIO133_OUT 0x322e62, BIT(0)
#define GPIO133_IN  0x322e62, BIT(2)

#define GPIO134_PAD PAD_SPDIF_OUT
#define GPIO134_OEN 0x322e60, BIT(1)
#define GPIO134_OUT 0x322e60, BIT(0)
#define GPIO134_IN  0x322e60, BIT(2)

#define GPIO135_PAD PAD_TCON0
#define GPIO135_OEN 0x322f00, BIT(1)
#define GPIO135_OUT 0x322f00, BIT(0)
#define GPIO135_IN  0x322f00, BIT(2)

#define GPIO136_PAD PAD_TCON1
#define GPIO136_OEN 0x322f02, BIT(1)
#define GPIO136_OUT 0x322f02, BIT(0)
#define GPIO136_IN  0x322f02, BIT(2)

#define GPIO137_PAD PAD_TCON2
#define GPIO137_OEN 0x322f04, BIT(1)
#define GPIO137_OUT 0x322f04, BIT(0)
#define GPIO137_IN  0x322f04, BIT(2)

#define GPIO138_PAD PAD_TCON3
#define GPIO138_OEN 0x322f06, BIT(1)
#define GPIO138_OUT 0x322f06, BIT(0)
#define GPIO138_IN  0x322f06, BIT(2)

#define GPIO139_PAD PAD_TCON4
#define GPIO139_OEN 0x322f08, BIT(1)
#define GPIO139_OUT 0x322f08, BIT(0)
#define GPIO139_IN  0x322f08, BIT(2)

#define GPIO140_PAD PAD_TGPIO0
#define GPIO140_OEN 0x322b80, BIT(1)
#define GPIO140_OUT 0x322b80, BIT(0)
#define GPIO140_IN  0x322b80, BIT(2)

#define GPIO141_PAD PAD_TGPIO1
#define GPIO141_OEN 0x322b82, BIT(1)
#define GPIO141_OUT 0x322b82, BIT(0)
#define GPIO141_IN  0x322b82, BIT(2)

#define GPIO142_PAD PAD_TGPIO2
#define GPIO142_OEN 0x322b84, BIT(1)
#define GPIO142_OUT 0x322b84, BIT(0)
#define GPIO142_IN  0x322b84, BIT(2)

#define GPIO143_PAD PAD_TGPIO3
#define GPIO143_OEN 0x322b86, BIT(1)
#define GPIO143_OUT 0x322b86, BIT(0)
#define GPIO143_IN  0x322b86, BIT(2)

#define GPIO144_PAD PAD_TS0_CLK
#define GPIO144_OEN 0x322c10, BIT(1)
#define GPIO144_OUT 0x322c10, BIT(0)
#define GPIO144_IN  0x322c10, BIT(2)

#define GPIO145_PAD PAD_TS0_D0
#define GPIO145_OEN 0x322c00, BIT(1)
#define GPIO145_OUT 0x322c00, BIT(0)
#define GPIO145_IN  0x322c00, BIT(2)

#define GPIO146_PAD PAD_TS0_D1
#define GPIO146_OEN 0x322c02, BIT(1)
#define GPIO146_OUT 0x322c02, BIT(0)
#define GPIO146_IN  0x322c02, BIT(2)

#define GPIO147_PAD PAD_TS0_D2
#define GPIO147_OEN 0x322c04, BIT(1)
#define GPIO147_OUT 0x322c04, BIT(0)
#define GPIO147_IN  0x322c04, BIT(2)

#define GPIO148_PAD PAD_TS0_D3
#define GPIO148_OEN 0x322c06, BIT(1)
#define GPIO148_OUT 0x322c06, BIT(0)
#define GPIO148_IN  0x322c06, BIT(2)

#define GPIO149_PAD PAD_TS0_D4
#define GPIO149_OEN 0x322c08, BIT(1)
#define GPIO149_OUT 0x322c08, BIT(0)
#define GPIO149_IN  0x322c08, BIT(2)

#define GPIO150_PAD PAD_TS0_D5
#define GPIO150_OEN 0x322c0a, BIT(1)
#define GPIO150_OUT 0x322c0a, BIT(0)
#define GPIO150_IN  0x322c0a, BIT(2)

#define GPIO151_PAD PAD_TS0_D6
#define GPIO151_OEN 0x322c0c, BIT(1)
#define GPIO151_OUT 0x322c0c, BIT(0)
#define GPIO151_IN  0x322c0c, BIT(2)

#define GPIO152_PAD PAD_TS0_D7
#define GPIO152_OEN 0x322c0e, BIT(1)
#define GPIO152_OUT 0x322c0e, BIT(0)
#define GPIO152_IN  0x322c0e, BIT(2)

#define GPIO153_PAD PAD_TS0_SYNC
#define GPIO153_OEN 0x322c12, BIT(1)
#define GPIO153_OUT 0x322c12, BIT(0)
#define GPIO153_IN  0x322c12, BIT(2)

#define GPIO154_PAD PAD_TS0_VLD
#define GPIO154_OEN 0x322c14, BIT(1)
#define GPIO154_OUT 0x322c14, BIT(0)
#define GPIO154_IN  0x322c14, BIT(2)

#define GPIO155_PAD PAD_TS1_CLK
#define GPIO155_OEN 0x322c30, BIT(1)
#define GPIO155_OUT 0x322c30, BIT(0)
#define GPIO155_IN  0x322c30, BIT(2)

#define GPIO156_PAD PAD_TS1_D0
#define GPIO156_OEN 0x322c20, BIT(1)
#define GPIO156_OUT 0x322c20, BIT(0)
#define GPIO156_IN  0x322c20, BIT(2)

#define GPIO157_PAD PAD_TS1_D1
#define GPIO157_OEN 0x322c22, BIT(1)
#define GPIO157_OUT 0x322c22, BIT(0)
#define GPIO157_IN  0x322c22, BIT(2)

#define GPIO158_PAD PAD_TS1_D2
#define GPIO158_OEN 0x322c24, BIT(1)
#define GPIO158_OUT 0x322c24, BIT(0)
#define GPIO158_IN  0x322c24, BIT(2)

#define GPIO159_PAD PAD_TS1_D3
#define GPIO159_OEN 0x322c26, BIT(1)
#define GPIO159_OUT 0x322c26, BIT(0)
#define GPIO159_IN  0x322c26, BIT(2)

#define GPIO160_PAD PAD_TS1_D4
#define GPIO160_OEN 0x322c28, BIT(1)
#define GPIO160_OUT 0x322c28, BIT(0)
#define GPIO160_IN  0x322c28, BIT(2)

#define GPIO161_PAD PAD_TS1_D5
#define GPIO161_OEN 0x322c2a, BIT(1)
#define GPIO161_OUT 0x322c2a, BIT(0)
#define GPIO161_IN  0x322c2a, BIT(2)

#define GPIO162_PAD PAD_TS1_D6
#define GPIO162_OEN 0x322c2c, BIT(1)
#define GPIO162_OUT 0x322c2c, BIT(0)
#define GPIO162_IN  0x322c2c, BIT(2)

#define GPIO163_PAD PAD_TS1_D7
#define GPIO163_OEN 0x322c2e, BIT(1)
#define GPIO163_OUT 0x322c2e, BIT(0)
#define GPIO163_IN  0x322c2e, BIT(2)

#define GPIO164_PAD PAD_TS1_SYNC
#define GPIO164_OEN 0x322c32, BIT(1)
#define GPIO164_OUT 0x322c32, BIT(0)
#define GPIO164_IN  0x322c32, BIT(2)

#define GPIO165_PAD PAD_TS1_VLD
#define GPIO165_OEN 0x322c34, BIT(1)
#define GPIO165_OUT 0x322c34, BIT(0)
#define GPIO165_IN  0x322c34, BIT(2)

#define GPIO166_PAD PAD_TS2_CLK
#define GPIO166_OEN 0x322c50, BIT(1)
#define GPIO166_OUT 0x322c50, BIT(0)
#define GPIO166_IN  0x322c50, BIT(2)

#define GPIO167_PAD PAD_TS2_D0
#define GPIO167_OEN 0x322c40, BIT(1)
#define GPIO167_OUT 0x322c40, BIT(0)
#define GPIO167_IN  0x322c40, BIT(2)

#define GPIO168_PAD PAD_TS2_D1
#define GPIO168_OEN 0x322c42, BIT(1)
#define GPIO168_OUT 0x322c42, BIT(0)
#define GPIO168_IN  0x322c42, BIT(2)

#define GPIO169_PAD PAD_TS2_D2
#define GPIO169_OEN 0x322c44, BIT(1)
#define GPIO169_OUT 0x322c44, BIT(0)
#define GPIO169_IN  0x322c44, BIT(2)

#define GPIO170_PAD PAD_TS2_D3
#define GPIO170_OEN 0x322c46, BIT(1)
#define GPIO170_OUT 0x322c46, BIT(0)
#define GPIO170_IN  0x322c46, BIT(2)

#define GPIO171_PAD PAD_TS2_D4
#define GPIO171_OEN 0x322c48, BIT(1)
#define GPIO171_OUT 0x322c48, BIT(0)
#define GPIO171_IN  0x322c48, BIT(2)

#define GPIO172_PAD PAD_TS2_D5
#define GPIO172_OEN 0x322c4a, BIT(1)
#define GPIO172_OUT 0x322c4a, BIT(0)
#define GPIO172_IN  0x322c4a, BIT(2)

#define GPIO173_PAD PAD_TS2_D6
#define GPIO173_OEN 0x322c4c, BIT(1)
#define GPIO173_OUT 0x322c4c, BIT(0)
#define GPIO173_IN  0x322c4c, BIT(2)

#define GPIO174_PAD PAD_TS2_D7
#define GPIO174_OEN 0x322c4e, BIT(1)
#define GPIO174_OUT 0x322c4e, BIT(0)
#define GPIO174_IN  0x322c4e, BIT(2)

#define GPIO175_PAD PAD_TS2_SYNC
#define GPIO175_OEN 0x322c52, BIT(1)
#define GPIO175_OUT 0x322c52, BIT(0)
#define GPIO175_IN  0x322c52, BIT(2)

#define GPIO176_PAD PAD_TS2_VLD
#define GPIO176_OEN 0x322c54, BIT(1)
#define GPIO176_OUT 0x322c54, BIT(0)
#define GPIO176_IN  0x322c54, BIT(2)

#define GPIO177_PAD PAD_TS3_CLK
#define GPIO177_OEN 0x322c70, BIT(1)
#define GPIO177_OUT 0x322c70, BIT(0)
#define GPIO177_IN  0x322c70, BIT(2)

#define GPIO178_PAD PAD_TS3_D0
#define GPIO178_OEN 0x322c60, BIT(1)
#define GPIO178_OUT 0x322c60, BIT(0)
#define GPIO178_IN  0x322c60, BIT(2)

#define GPIO179_PAD PAD_TS3_SYNC
#define GPIO179_OEN 0x322c72, BIT(1)
#define GPIO179_OUT 0x322c72, BIT(0)
#define GPIO179_IN  0x322c72, BIT(2)

#define GPIO180_PAD PAD_TS3_VLD
#define GPIO180_OEN 0x322c74, BIT(1)
#define GPIO180_OUT 0x322c74, BIT(0)
#define GPIO180_IN  0x322c74, BIT(2)

#define GPIO292_PAD PAD_TS3_D1
#define GPIO292_OEN 0x322c62, BIT(1)
#define GPIO292_OUT 0x322c62, BIT(0)
#define GPIO292_IN  0x322c62, BIT(2)

#define GPIO293_PAD PAD_TS3_D2
#define GPIO293_OEN 0x322c64, BIT(1)
#define GPIO293_OUT 0x322c64, BIT(0)
#define GPIO293_IN  0x322c64, BIT(2)

#define GPIO294_PAD PAD_TS3_D3
#define GPIO294_OEN 0x322c66, BIT(1)
#define GPIO294_OUT 0x322c66, BIT(0)
#define GPIO294_IN  0x322c66, BIT(2)

#define GPIO295_PAD PAD_TS3_D4
#define GPIO295_OEN 0x322c68, BIT(1)
#define GPIO295_OUT 0x322c68, BIT(0)
#define GPIO295_IN  0x322c68, BIT(2)

#define GPIO296_PAD PAD_TS3_D5
#define GPIO296_OEN 0x322c6a, BIT(1)
#define GPIO296_OUT 0x322c6a, BIT(0)
#define GPIO296_IN  0x322c6a, BIT(2)

#define GPIO297_PAD PAD_TS3_D6
#define GPIO297_OEN 0x322c6c, BIT(1)
#define GPIO297_OUT 0x322c6c, BIT(0)
#define GPIO297_IN  0x322c6c, BIT(2)

#define GPIO298_PAD PAD_TS3_D7
#define GPIO298_OEN 0x322c6e, BIT(1)
#define GPIO298_OUT 0x322c6e, BIT(0)
#define GPIO298_IN  0x322c6e, BIT(2)

#define GPIO299_PAD PAD_TCON5
#define GPIO299_OEN 0x322f0a, BIT(1)
#define GPIO299_OUT 0x322f0a, BIT(0)
#define GPIO299_IN  0x322f0a, BIT(2)

#define GPIO300_PAD PAD_TCON6
#define GPIO300_OEN 0x322f0c, BIT(1)
#define GPIO300_OUT 0x322f0c, BIT(0)
#define GPIO300_IN  0x322f0c, BIT(2)

#define GPIO301_PAD PAD_TCON7
#define GPIO301_OEN 0x322f0e, BIT(1)
#define GPIO301_OUT 0x322f0e, BIT(0)
#define GPIO301_IN  0x322f0e, BIT(2)

#define GPIO302_PAD PAD_TCON8
#define GPIO302_OEN 0x322f10, BIT(1)
#define GPIO302_OUT 0x322f10, BIT(0)
#define GPIO302_IN  0x322f10, BIT(2)

#define GPIO303_PAD PAD_GPIO29_PM
#define GPIO303_OEN 0x0f60, BIT(0)
#define GPIO303_OUT 0x0f60, BIT(1)
#define GPIO303_IN  0x0f60, BIT(2)

#define GPIO304_PAD PAD_GPIO30_PM
#define GPIO304_OEN 0x0f62, BIT(0)
#define GPIO304_OUT 0x0f62, BIT(1)
#define GPIO304_IN  0x0f62, BIT(2)

#define GPIO305_PAD PAD_GPIO31_PM
#define GPIO305_OEN 0x0f64, BIT(0)
#define GPIO305_OUT 0x0f64, BIT(1)
#define GPIO305_IN  0x0f64, BIT(2)

static const struct gpio_setting {
	u32 r_oen;
	u8 m_oen;
	u32 r_out;
	u8 m_out;
	u32 r_in;
	u8 m_in;
} gpio_table[] = {
#define __GPIO__(_x_) { CONCAT(CONCAT(GPIO, _x_), _OEN),   \
			   CONCAT(CONCAT(GPIO, _x_), _OUT),   \
			   CONCAT(CONCAT(GPIO, _x_), _IN) }
#define __GPIO(_x_)   __GPIO__(_x_)

//
// !! WARNING !! DO NOT MODIFIY !!!!
//
// These defines order must match following
// 1. the PAD name in GPIO excel
// 2. the perl script to generate the package header file
//
	//__GPIO(999), // 0 is not used

	__GPIO(999), __GPIO(1), __GPIO(2), __GPIO(3), __GPIO(4),
	__GPIO(5), __GPIO(6), __GPIO(7), __GPIO(8), __GPIO(9),
	__GPIO(10), __GPIO(11), __GPIO(12), __GPIO(13), __GPIO(14),
	__GPIO(15), __GPIO(16), __GPIO(17), __GPIO(18), __GPIO(19),
	__GPIO(20), __GPIO(21), __GPIO(22), __GPIO(23), __GPIO(24),
	__GPIO(25), __GPIO(26), __GPIO(27), __GPIO(28), __GPIO(29),
	__GPIO(30), __GPIO(31), __GPIO(32), __GPIO(33), __GPIO(34),
	__GPIO(35), __GPIO(36), __GPIO(37), __GPIO(38), __GPIO(39),
	__GPIO(40), __GPIO(41), __GPIO(42), __GPIO(43), __GPIO(44),
	__GPIO(45), __GPIO(46), __GPIO(47), __GPIO(48), __GPIO(49),
	__GPIO(50), __GPIO(51), __GPIO(52), __GPIO(53), __GPIO(54),
	__GPIO(55), __GPIO(56), __GPIO(57), __GPIO(58), __GPIO(59),
	__GPIO(60), __GPIO(61), __GPIO(62), __GPIO(63), __GPIO(64),
	__GPIO(65), __GPIO(66), __GPIO(67), __GPIO(68), __GPIO(69),
	__GPIO(70), __GPIO(71), __GPIO(72), __GPIO(73), __GPIO(74),
	__GPIO(75), __GPIO(76), __GPIO(77), __GPIO(78), __GPIO(79),
	__GPIO(80), __GPIO(81), __GPIO(82), __GPIO(83), __GPIO(84),
	__GPIO(85), __GPIO(86), __GPIO(87), __GPIO(88), __GPIO(89),
	__GPIO(90), __GPIO(91), __GPIO(92), __GPIO(93), __GPIO(94),
	__GPIO(95), __GPIO(96), __GPIO(97), __GPIO(98), __GPIO(99),
	__GPIO(100), __GPIO(101), __GPIO(102), __GPIO(103), __GPIO(104),
	__GPIO(105), __GPIO(106), __GPIO(107), __GPIO(108), __GPIO(109),
	__GPIO(110), __GPIO(111), __GPIO(112), __GPIO(113), __GPIO(114),
	__GPIO(115), __GPIO(116), __GPIO(117), __GPIO(118), __GPIO(119),
	__GPIO(120), __GPIO(121), __GPIO(122), __GPIO(123), __GPIO(124),
	__GPIO(125), __GPIO(126), __GPIO(127), __GPIO(128), __GPIO(129),
	__GPIO(130), __GPIO(131), __GPIO(132), __GPIO(133), __GPIO(134),
	__GPIO(135), __GPIO(136), __GPIO(137), __GPIO(138), __GPIO(139),
	__GPIO(140), __GPIO(141), __GPIO(142), __GPIO(143), __GPIO(144),
	__GPIO(145), __GPIO(146), __GPIO(147), __GPIO(148), __GPIO(149),
	__GPIO(150), __GPIO(151), __GPIO(152), __GPIO(153), __GPIO(154),
	__GPIO(155), __GPIO(156), __GPIO(157), __GPIO(158), __GPIO(159),
	__GPIO(160), __GPIO(161), __GPIO(162), __GPIO(163), __GPIO(164),
	__GPIO(165), __GPIO(166), __GPIO(167), __GPIO(168), __GPIO(169),
	__GPIO(170), __GPIO(171), __GPIO(172), __GPIO(173), __GPIO(174),
	__GPIO(175), __GPIO(176), __GPIO(177), __GPIO(178), __GPIO(179),
	__GPIO(180), __GPIO(999), __GPIO(999), __GPIO(999), __GPIO(999),
	__GPIO(999), __GPIO(999), __GPIO(999), __GPIO(999), __GPIO(999),
	__GPIO(999), __GPIO(999), __GPIO(999), __GPIO(999), __GPIO(999),
	__GPIO(999), __GPIO(999), __GPIO(999), __GPIO(999), __GPIO(999),
	__GPIO(999), __GPIO(999), __GPIO(999), __GPIO(999), __GPIO(999),
	__GPIO(999), __GPIO(999), __GPIO(999), __GPIO(999), __GPIO(999),
	__GPIO(999), __GPIO(999), __GPIO(999), __GPIO(999), __GPIO(999),
	__GPIO(999), __GPIO(999), __GPIO(999), __GPIO(999), __GPIO(999),
	__GPIO(999), __GPIO(999), __GPIO(999), __GPIO(999), __GPIO(999),
	__GPIO(999), __GPIO(999), __GPIO(999), __GPIO(999), __GPIO(999),
	__GPIO(999), __GPIO(999), __GPIO(999), __GPIO(999), __GPIO(999),
	__GPIO(999), __GPIO(999), __GPIO(999), __GPIO(999), __GPIO(999),
	__GPIO(999), __GPIO(999), __GPIO(999), __GPIO(999), __GPIO(999),
	__GPIO(999), __GPIO(999), __GPIO(999), __GPIO(999), __GPIO(999),
	__GPIO(999), __GPIO(999), __GPIO(999), __GPIO(999), __GPIO(999),
	__GPIO(999), __GPIO(999), __GPIO(999), __GPIO(999), __GPIO(999),
	__GPIO(999), __GPIO(999), __GPIO(999), __GPIO(999), __GPIO(999),
	__GPIO(999), __GPIO(999), __GPIO(999), __GPIO(999), __GPIO(999),
	__GPIO(999), __GPIO(999), __GPIO(999), __GPIO(999), __GPIO(999),
	__GPIO(999), __GPIO(999), __GPIO(999), __GPIO(999), __GPIO(999),
	__GPIO(999), __GPIO(999), __GPIO(999), __GPIO(999), __GPIO(999),
	__GPIO(999), __GPIO(999), __GPIO(999), __GPIO(999), __GPIO(999),
	__GPIO(999), __GPIO(999), __GPIO(292), __GPIO(293), __GPIO(294),
	__GPIO(295), __GPIO(296), __GPIO(297), __GPIO(298), __GPIO(299),
	__GPIO(300), __GPIO(301), __GPIO(302), __GPIO(303), __GPIO(304),
	__GPIO(305)
};

/**
 * struct mediatek_gpio_priv - mtk_dtv_gpio private data
 *
 * @bank_name: Name of bank, e.g. "PA", "PB" etc
 * @gpio_count: Number of GPIOs
 * @requested: Bit mapped of gpio is requested
 */
struct mediatek_gpio_priv {
	char *bank_name;
	int gpio_count;
	DECLARE_BITMAP(requested, ARRAY_SIZE(gpio_table));
};

#define MHAL_RIU_REG(addr) \
	((void __iomem *)0x1F000000 + (((addr) & ~1) << 1) + ((addr) & 1))

//------------------------------------------------------------------------------
/// exclude pad mode, make pad mode not the same as inquiry mode
/// @param  reg_addr              \b IN:  pad top register address
/// @param  test_mode             \b IN:
/// @param  test_mask             \b IN:
//------------------------------------------------------------------------------
static void _mediatek_gpio_exclude_pad_mode(u32 reg_addr, u16 test_mode,
					    u16 test_mask)
{
	u16 reg_mask_value;

	if ((reg_addr & 0x01) == 0) {
		// even register address: access register 16 bits
		debug
		    ("[gpio] PAD MUX reg 0x%08X, value 0x%04X, checking mask 0x%04X\n",
		     reg_addr, readw(MHAL_RIU_REG(reg_addr)), test_mask);

		reg_mask_value = readw(MHAL_RIU_REG(reg_addr)) & test_mask;
		if (reg_mask_value == test_mode) {
			// current pad mode is the same as inquiry mode,
			// set pad mode to 0
			pr_warn("[gpio] PAD MUX reset to zero\n");
			writew(readw(MHAL_RIU_REG(reg_addr)) & ~test_mask,
			       MHAL_RIU_REG(reg_addr));
		}
	} else {
		// odd register address: access register 8 bits
		debug
		    ("[gpio] PAD MUX reg 0x%08X, value 0x%02X, checking mask 0x%02X\n",
		     reg_addr, readb(MHAL_RIU_REG(reg_addr)), test_mask);

		reg_mask_value = readb(MHAL_RIU_REG(reg_addr)) & test_mask;
		if (reg_mask_value == test_mode) {
			// current pad mode is the same as inquiry mode,
			// set pad mode to 0
			pr_warn("[gpio] PAD MUX reset to zero\n");
			writeb(readb(MHAL_RIU_REG(reg_addr)) & ~test_mask,
			       MHAL_RIU_REG(reg_addr));
		}
	}
}

static int mediatek_gpio_request(struct udevice *dev, unsigned int offset,
				 const char *label)
{
	struct mediatek_gpio_priv *priv = dev_get_priv(dev);

	debug("%s, pin %d, label %s\n", __func__, offset, label);

	if (!gpio_table[offset].r_in && !gpio_table[offset].r_oen && !gpio_table[offset].r_out) {
		debug("pin %d does not have gpio registers setting\n", offset);
		return -EBUSY;
	}

	generic_set_bit(offset, priv->requested);

	switch (offset) {
	case PAD_DDCA_CK:
	case PAD_DDCA_DA:
		// reg_gpio2a0
		writeb(readb(MHAL_RIU_REG(0x0494)) | BIT(7),
		       MHAL_RIU_REG(0x0494));
		break;
	case PAD_IRIN:
		// reg_ir_is_gpio
		writeb(readb(MHAL_RIU_REG(0x0e38)) | BIT(4),
		       MHAL_RIU_REG(0x0e38));
		break;
	case PAD_PWM_PM:
		// reg_pwm_is_gpio
		writeb(readb(MHAL_RIU_REG(0x0e38)) | BIT(5),
		       MHAL_RIU_REG(0x0e38));
		break;
	case PAD_CEC0:
		// reg_cec_is_gpio
		writeb(readb(MHAL_RIU_REG(0x0e38)) | BIT(6),
		       MHAL_RIU_REG(0x0e38));
		break;
	case PAD_GPIO0_PM:
		// reg_sd_cdz_mode
		writeb(readb(MHAL_RIU_REG(0x0e4f)) & ~BIT(6),
		       MHAL_RIU_REG(0x0e4f));
		// reg_spi_pad_sel
		writeb(readb(MHAL_RIU_REG(0x0eed)) & ~BIT(2),
		       MHAL_RIU_REG(0x0eed));
		// reg_ld_spi1_config, reg_ld_spi3_config
		writeb(readb(MHAL_RIU_REG(0x0ee5)) & ~(BIT(5) | BIT(7)),
		       MHAL_RIU_REG(0x0ee5));
		// reg_gpio_is_pwm1 mode 1
		_mediatek_gpio_exclude_pad_mode(0x0eed, 0x01 << 3,
						GENMASK(4, 3));
		break;
	case PAD_GPIO1_PM:
		// reg_spi_pad_sel
		writeb(readb(MHAL_RIU_REG(0x0eed)) & ~BIT(2),
		       MHAL_RIU_REG(0x0eed));
		// reg_ld_spi2_config, reg_ld_spi3_config
		writeb(readb(MHAL_RIU_REG(0x0ee5)) & ~(BIT(6) | BIT(7)),
		       MHAL_RIU_REG(0x0ee5));
		// reg_uart_is_gpio_4 mode 1, 9
		_mediatek_gpio_exclude_pad_mode(0x0eec, 0x01 << 6,
						GENMASK(9, 6));
		_mediatek_gpio_exclude_pad_mode(0x0eec, 0x09 << 6,
						GENMASK(9, 6));
		// reg_uart_is_gpio_3 mode 1, 9
		_mediatek_gpio_exclude_pad_mode(0x0eec, 0x01 << 2,
						GENMASK(5, 2));
		_mediatek_gpio_exclude_pad_mode(0x0eec, 0x09 << 2,
						GENMASK(5, 2));
		// reg_uart_is_gpio mode 2, 6, 8, 12
		_mediatek_gpio_exclude_pad_mode(0x0e6b, 0x02 << 0,
						GENMASK(3, 0));
		_mediatek_gpio_exclude_pad_mode(0x0e6b, 0x06 << 0,
						GENMASK(3, 0));
		_mediatek_gpio_exclude_pad_mode(0x0e6b, 0x08 << 0,
						GENMASK(3, 0));
		_mediatek_gpio_exclude_pad_mode(0x0e6b, 0x0C << 0,
						GENMASK(3, 0));
		break;
	case PAD_GPIO2_PM:
		// reg_gpio_is_clk_rtc mode 1
		_mediatek_gpio_exclude_pad_mode(0x0ef4, 0x01 << 0,
						GENMASK(1, 0));
		// reg_gpio_is_clk_xtal mode 1
		_mediatek_gpio_exclude_pad_mode(0x0ef4, 0x01 << 4,
						GENMASK(5, 4));
		// reg_irtx_mode mode 1
		_mediatek_gpio_exclude_pad_mode(0x2e86, 0x01 << 3,
						GENMASK(4, 3));
		break;
	case PAD_USB_CTRL:
		// reg_uart_is_gpio_4 mode 2, 10
		_mediatek_gpio_exclude_pad_mode(0x0eec, 0x02 << 6,
						GENMASK(9, 6));
		_mediatek_gpio_exclude_pad_mode(0x0eec, 0x0A << 6,
						GENMASK(9, 6));
		// reg_uart_is_gpio_3 mode 2, 10
		_mediatek_gpio_exclude_pad_mode(0x0eec, 0x02 << 2,
						GENMASK(5, 2));
		_mediatek_gpio_exclude_pad_mode(0x0eec, 0x0A << 2,
						GENMASK(5, 2));
		// reg_uart_is_gpio_2 mode 1, 2
		_mediatek_gpio_exclude_pad_mode(0x0eec, 0x01 << 0,
						GENMASK(1, 0));
		_mediatek_gpio_exclude_pad_mode(0x0eec, 0x02 << 0,
						GENMASK(1, 0));
		break;
	case PAD_GPIO5_PM:
		// reg_spi_pad_sel
		writeb(readb(MHAL_RIU_REG(0x0eed)) & ~BIT(2),
		       MHAL_RIU_REG(0x0eed));
		// reg_ld_spi1_config, reg_ld_spi3_config
		writeb(readb(MHAL_RIU_REG(0x0ee5)) & ~(BIT(5) | BIT(7)),
		       MHAL_RIU_REG(0x0ee5));
		// reg_uart_is_gpio_4 mode 1, 3, 9, 11
		_mediatek_gpio_exclude_pad_mode(0x0eec, 0x01 << 6,
						GENMASK(9, 6));
		_mediatek_gpio_exclude_pad_mode(0x0eec, 0x03 << 6,
						GENMASK(9, 6));
		_mediatek_gpio_exclude_pad_mode(0x0eec, 0x09 << 6,
						GENMASK(9, 6));
		_mediatek_gpio_exclude_pad_mode(0x0eec, 0x0B << 6,
						GENMASK(9, 6));
		// reg_uart_is_gpio_3 mode 1, 9
		_mediatek_gpio_exclude_pad_mode(0x0eec, 0x01 << 2,
						GENMASK(5, 2));
		_mediatek_gpio_exclude_pad_mode(0x0eec, 0x09 << 2,
						GENMASK(5, 2));
		// reg_uart_is_gpio mode 1, 2, 5, 6, 8, 12
		_mediatek_gpio_exclude_pad_mode(0x0e6b, 0x01 << 0,
						GENMASK(3, 0));
		_mediatek_gpio_exclude_pad_mode(0x0e6b, 0x02 << 0,
						GENMASK(3, 0));
		_mediatek_gpio_exclude_pad_mode(0x0e6b, 0x05 << 0,
						GENMASK(3, 0));
		_mediatek_gpio_exclude_pad_mode(0x0e6b, 0x06 << 0,
						GENMASK(3, 0));
		_mediatek_gpio_exclude_pad_mode(0x0e6b, 0x08 << 0,
						GENMASK(3, 0));
		_mediatek_gpio_exclude_pad_mode(0x0e6b, 0x0C << 0,
						GENMASK(3, 0));
		break;
	case PAD_GPIO6_PM:
		// reg_spicsz1_gpio
		writeb(readb(MHAL_RIU_REG(0x0e6a)) | BIT(2),
		       MHAL_RIU_REG(0x0e6a));
		// reg_ld_spi2_config, reg_ld_spi3_config
		writeb(readb(MHAL_RIU_REG(0x0ee5)) & ~(BIT(6) | BIT(7)),
		       MHAL_RIU_REG(0x0ee5));
		// reg_gpio_is_pwm1 mode 2
		_mediatek_gpio_exclude_pad_mode(0x0eed, 0x02 << 3,
						GENMASK(4, 3));
		break;
	case PAD_GPIO7_PM:
		// reg_miic_mode mode 2
		_mediatek_gpio_exclude_pad_mode(0x0ec9, 0x02 << 6,
						GENMASK(7, 6));
		break;
	case PAD_GPIO8_PM:
		// reg_uart_is_gpio_4 mode 2, 3, 10, 11
		_mediatek_gpio_exclude_pad_mode(0x0eec, 0x02 << 6,
						GENMASK(9, 6));
		_mediatek_gpio_exclude_pad_mode(0x0eec, 0x03 << 6,
						GENMASK(9, 6));
		_mediatek_gpio_exclude_pad_mode(0x0eec, 0x0A << 6,
						GENMASK(9, 6));
		_mediatek_gpio_exclude_pad_mode(0x0eec, 0x0B << 6,
						GENMASK(9, 6));
		// reg_miic_mode mode 2
		_mediatek_gpio_exclude_pad_mode(0x0ec9, 0x02 << 6,
						GENMASK(7, 6));
		// reg_uart_is_gpio_3 mode 2, 10
		_mediatek_gpio_exclude_pad_mode(0x0eec, 0x02 << 2,
						GENMASK(5, 2));
		_mediatek_gpio_exclude_pad_mode(0x0eec, 0x0A << 2,
						GENMASK(5, 2));
		// reg_uart_is_gpio_2 mode 1, 2
		_mediatek_gpio_exclude_pad_mode(0x0eec, 0x01 << 0,
						GENMASK(1, 0));
		_mediatek_gpio_exclude_pad_mode(0x0eec, 0x02 << 0,
						GENMASK(1, 0));
		// reg_uart_is_gpio mode 1, 5
		_mediatek_gpio_exclude_pad_mode(0x0e6b, 0x01 << 0,
						GENMASK(3, 0));
		_mediatek_gpio_exclude_pad_mode(0x0e6b, 0x05 << 0,
						GENMASK(3, 0));
		break;
	case PAD_GPIO9_PM:
		// reg_gpio_is_clk_rtc mode 2
		_mediatek_gpio_exclude_pad_mode(0x0ef4, 0x02 << 0,
						GENMASK(1, 0));
		// reg_gpio_is_clk_xtal mode 2
		_mediatek_gpio_exclude_pad_mode(0x0ef4, 0x02 << 4,
						GENMASK(5, 4));
		break;
	case PAD_GPIO10_PM:
		// reg_gpio_is_clk_rtc mode 3
		_mediatek_gpio_exclude_pad_mode(0x0ef4, 0x03 << 0,
						GENMASK(1, 0));
		// reg_gpio_is_clk_xtal mode 3
		_mediatek_gpio_exclude_pad_mode(0x0ef4, 0x03 << 4,
						GENMASK(5, 4));
		break;
	case PAD_GPIO11_PM:
	case PAD_GPIO12_PM:
		// reg_uart_is_gpio_4 mode 4, 12
		_mediatek_gpio_exclude_pad_mode(0x0eec, 0x04 << 6,
						GENMASK(9, 6));
		_mediatek_gpio_exclude_pad_mode(0x0eec, 0x0C << 6,
						GENMASK(9, 6));
		// reg_uart_is_gpio_3 mode 4, 12
		_mediatek_gpio_exclude_pad_mode(0x0eec, 0x04 << 2,
						GENMASK(5, 2));
		_mediatek_gpio_exclude_pad_mode(0x0eec, 0x0C << 2,
						GENMASK(5, 2));
		// reg_uart_is_gpio_1 mode 1, 2
		_mediatek_gpio_exclude_pad_mode(0x0e6b, 0x01 << 6,
						GENMASK(7, 6));
		_mediatek_gpio_exclude_pad_mode(0x0e6b, 0x02 << 6,
						GENMASK(7, 6));
		// reg_pdm_test_mode
		writeb(readb(MHAL_RIU_REG(0x2e86)) & ~BIT(5),
		       MHAL_RIU_REG(0x2e86));
		break;
	case PAD_GPIO13_PM:
		// reg_mic_md mode 6
		_mediatek_gpio_exclude_pad_mode(0x2e86, 0x06 << 0,
						GENMASK(2, 0));
	case PAD_GPIO14_PM:
		// reg_mic_md mode 5
		_mediatek_gpio_exclude_pad_mode(0x2e86, 0x05 << 0,
						GENMASK(2, 0));
		break;
	case PAD_GPIO15_PM:
	case PAD_GPIO16_PM:
	case PAD_GPIO17_PM:
	case PAD_GPIO18_PM:
	case PAD_GPIO19_PM:
	case PAD_GPIO20_PM:
	case PAD_GPIO21_PM:
	case PAD_GPIO22_PM:
	case PAD_GPIO23_PM:
	case PAD_GPIO24_PM:
	case PAD_GPIO25_PM:
	case PAD_GPIO26_PM:
	case PAD_GPIO27_PM:
	case PAD_GPIO28_PM:
		break;
	case PAD_DDCDA_CK:
	case PAD_DDCDA_DA:
		// reg_ej_mode mode 1, 3, 5
		_mediatek_gpio_exclude_pad_mode(0x2ec4, 0x01 << 0,
						GENMASK(2, 0));
		_mediatek_gpio_exclude_pad_mode(0x2ec4, 0x03 << 0,
						GENMASK(2, 0));
		_mediatek_gpio_exclude_pad_mode(0x2ec4, 0x05 << 0,
						GENMASK(2, 0));
		// reg_gpio2d0_en
		writeb(readb(MHAL_RIU_REG(0x0496)) | BIT(7),
		       MHAL_RIU_REG(0x0496));
		// reg_cm4_ej_mode mode 1, 2
		_mediatek_gpio_exclude_pad_mode(0x2e87, 0x01 << 6,
						GENMASK(7, 6));
		_mediatek_gpio_exclude_pad_mode(0x2e87, 0x02 << 6,
						GENMASK(7, 6));
		break;
	case PAD_DDCDB_CK:
	case PAD_DDCDB_DA:
		// reg_ej_mode mode 1, 2, 6
		_mediatek_gpio_exclude_pad_mode(0x2ec4, 0x01 << 0,
						GENMASK(2, 0));
		_mediatek_gpio_exclude_pad_mode(0x2ec4, 0x02 << 0,
						GENMASK(2, 0));
		_mediatek_gpio_exclude_pad_mode(0x2ec4, 0x06 << 0,
						GENMASK(2, 0));
		// reg_gpio2d1_en
		writeb(readb(MHAL_RIU_REG(0x0497)) | BIT(7),
		       MHAL_RIU_REG(0x0497));
		// reg_cm4_ej_mode mode 1
		_mediatek_gpio_exclude_pad_mode(0x2e87, 0x01 << 6,
						GENMASK(7, 6));
		break;
	case PAD_DDCDC_CK:
	case PAD_DDCDC_DA:
		// reg_ej_mode mode 2, 3, 7
		_mediatek_gpio_exclude_pad_mode(0x2ec4, 0x02 << 0,
						GENMASK(2, 0));
		_mediatek_gpio_exclude_pad_mode(0x2ec4, 0x03 << 0,
						GENMASK(2, 0));
		_mediatek_gpio_exclude_pad_mode(0x2ec4, 0x07 << 0,
						GENMASK(2, 0));
		// reg_gpio2d2_en
		writeb(readb(MHAL_RIU_REG(0x0498)) | BIT(7),
		       MHAL_RIU_REG(0x0498));
		// reg_cm4_ej_mode mode 2, 3
		_mediatek_gpio_exclude_pad_mode(0x2e87, 0x02 << 6,
						GENMASK(7, 6));
		_mediatek_gpio_exclude_pad_mode(0x2e87, 0x03 << 6,
						GENMASK(7, 6));
		break;
	case PAD_DDCDD_CK:
	case PAD_DDCDD_DA:
		// reg_ej_mode mode 5, 6, 7
		_mediatek_gpio_exclude_pad_mode(0x2ec4, 0x05 << 0,
						GENMASK(2, 0));
		_mediatek_gpio_exclude_pad_mode(0x2ec4, 0x06 << 0,
						GENMASK(2, 0));
		_mediatek_gpio_exclude_pad_mode(0x2ec4, 0x07 << 0,
						GENMASK(2, 0));
		// reg_gpio2d3_en
		writeb(readb(MHAL_RIU_REG(0x0499)) | BIT(7),
		       MHAL_RIU_REG(0x0499));
		// reg_cm4_ej_mode mode 3
		_mediatek_gpio_exclude_pad_mode(0x2e87, 0x03 << 6,
						GENMASK(7, 6));
		break;
	case PAD_SAR0:
	case PAD_SAR1:
	case PAD_SAR2:
	case PAD_SAR3:
	case PAD_SAR4:
	case PAD_VPLUGIN:
	case PAD_SAR6:
	case PAD_SAR7:
		// reg_sar_aisel bit (offset-PAD_SAR0)
		writeb(readb(MHAL_RIU_REG(0x1422)) &
		       ~(1 << (offset - PAD_SAR0)), MHAL_RIU_REG(0x1422));
		break;
	case PAD_VID0:
	case PAD_VID1:
	case PAD_VID2:
		break;
	case PAD_WOL_INT_OUT:
		// reg_wol_is_gpio
		writeb(readb(MHAL_RIU_REG(0x0e39)) | BIT(1),
		       MHAL_RIU_REG(0x0e39));
		break;
	case PAD_DDCR_CK:
		// reg_test_in_mode mode 1
		_mediatek_gpio_exclude_pad_mode(0x3229f0, 0x01 << 0,
						GENMASK(1, 0));
		// reg_test_out_mode mode 1
		_mediatek_gpio_exclude_pad_mode(0x3229f0, 0x01 << 4,
						GENMASK(5, 4));
		// reg_pwm_dac0_mode
		writeb(readb(MHAL_RIU_REG(0x322992)) & ~BIT(0),
		       MHAL_RIU_REG(0x322992));
		// reg_ddcrmode mode 1, 2
		_mediatek_gpio_exclude_pad_mode(0x322934, 0x01 << 0,
						GENMASK(1, 0));
		_mediatek_gpio_exclude_pad_mode(0x322934, 0x02 << 0,
						GENMASK(1, 0));
		// reg_allpad_in
		break;
	case PAD_DDCR_DA:
		// reg_test_in_mode mode 1
		_mediatek_gpio_exclude_pad_mode(0x3229f0, 0x01 << 0,
						GENMASK(1, 0));
		// reg_test_out_mode mode 1
		_mediatek_gpio_exclude_pad_mode(0x3229f0, 0x01 << 4,
						GENMASK(5, 4));
		// reg_pwm_dac1_mode
		writeb(readb(MHAL_RIU_REG(0x322992)) & ~BIT(4),
		       MHAL_RIU_REG(0x322992));
		// reg_ddcrmode mode 1, 2
		_mediatek_gpio_exclude_pad_mode(0x322934, 0x01 << 0,
						GENMASK(1, 0));
		_mediatek_gpio_exclude_pad_mode(0x322934, 0x02 << 0,
						GENMASK(1, 0));
		// reg_allpad_in
		break;
	case PAD_DIM0:
		// reg_gpio_dim_pe_00
		writeb(readb(MHAL_RIU_REG(0x322f41)) | BIT(0),
		       MHAL_RIU_REG(0x322f41));
		//reg_ld_qspi_config mode 2
		_mediatek_gpio_exclude_pad_mode(0x32295e, 0x02 << 0,
						GENMASK(1, 0));
		//reg_allpad_in
		break;
	case PAD_DIM1:
		// reg_gpio_dim_pe_01
		writeb(readb(MHAL_RIU_REG(0x322f43)) | BIT(0),
		       MHAL_RIU_REG(0x322f43));
		//reg_ld_qspi_config mode 2
		_mediatek_gpio_exclude_pad_mode(0x32295e, 0x02 << 0,
						GENMASK(1, 0));
		//reg_allpad_in
		break;
	case PAD_DIM2:
		// reg_gpio_dim_pe_02
		writeb(readb(MHAL_RIU_REG(0x322f45)) | BIT(0),
		       MHAL_RIU_REG(0x322f45));
		//reg_ld_qspi_config mode 2
		_mediatek_gpio_exclude_pad_mode(0x32295e, 0x02 << 0,
						GENMASK(1, 0));
		//reg_allpad_in
		break;
	case PAD_DIM3:
		// reg_gpio_dim_pe_03
		writeb(readb(MHAL_RIU_REG(0x322f47)) | BIT(0),
		       MHAL_RIU_REG(0x322f47));
		//reg_ld_qspi_config mode 2
		_mediatek_gpio_exclude_pad_mode(0x32295e, 0x02 << 0,
						GENMASK(1, 0));
		//reg_allpad_in
		break;
	case PAD_GPIO0:
		// reg_vsync_like_config mode 5, 6
		_mediatek_gpio_exclude_pad_mode(0x3229a4, 0x05 << 0,
						GENMASK(2, 0));
		_mediatek_gpio_exclude_pad_mode(0x3229a4, 0x06 << 0,
						GENMASK(2, 0));
		// reg_ld_spi3_config mode 2, 3
		_mediatek_gpio_exclude_pad_mode(0x3229a1, 0x02 << 0,
						GENMASK(1, 0));
		_mediatek_gpio_exclude_pad_mode(0x3229a1, 0x03 << 0,
						GENMASK(1, 0));
		// reg_pwm3_mode mode 2
		_mediatek_gpio_exclude_pad_mode(0x322991, 0x02 << 4,
						GENMASK(5, 4));
		// reg_p1_enable_b0
		writeb(readb(MHAL_RIU_REG(0x3229c8)) & ~BIT(0),
		       MHAL_RIU_REG(0x3229c8));
		// reg_lg_earc_mode
		writeb(readb(MHAL_RIU_REG(0x32297c)) & ~BIT(4),
		       MHAL_RIU_REG(0x32297c));
		// reg_allpad_in
		break;
	case PAD_GPIO1:
		// reg_vsync_like_config mode 1, 2
		_mediatek_gpio_exclude_pad_mode(0x3229a4, 0x01 << 0,
						GENMASK(2, 0));
		_mediatek_gpio_exclude_pad_mode(0x3229a4, 0x02 << 0,
						GENMASK(2, 0));
		// reg_ld_spi3_config (not in tmux table ??)
		// reg_pwm3_mode mode 1
		_mediatek_gpio_exclude_pad_mode(0x322991, 0x01 << 4,
						GENMASK(5, 4));
		// reg_allpad_in
		break;
	case PAD_GPIO8:
		// reg_p1_enable_b7
		writeb(readb(MHAL_RIU_REG(0x3229cb)) & ~BIT(4),
		       MHAL_RIU_REG(0x3229cb));
		// reg_allpad_in
		break;
	case PAD_GPIO9:
		// reg_miic_mode2 mode 3
		_mediatek_gpio_exclude_pad_mode(0x322951, 0x03 << 0,
						GENMASK(1, 0));
		// reg_extint4
		writeb(readb(MHAL_RIU_REG(0x3229c2)) & ~BIT(0),
		       MHAL_RIU_REG(0x3229c2));
		// reg_p1_enable_b6
		writeb(readb(MHAL_RIU_REG(0x3229cb)) & ~BIT(0),
		       MHAL_RIU_REG(0x3229cb));
		// reg_allpad_in
		break;
	case PAD_GPIO10:
		// reg_miic_mode2 mode 3
		_mediatek_gpio_exclude_pad_mode(0x322951, 0x03 << 0,
						GENMASK(1, 0));
		// reg_extint5
		writeb(readb(MHAL_RIU_REG(0x3229c2)) & ~BIT(4),
		       MHAL_RIU_REG(0x3229c2));
		// reg_allpad_in
		break;
	case PAD_GPIO11:
		// reg_fifthuartmode
		// reg_od5thuart
		writeb(readb(MHAL_RIU_REG(0x322945)) & ~(BIT(4) | BIT(0)),
		       MHAL_RIU_REG(0x322945));
		//reg_tconconfig6
		writeb(readb(MHAL_RIU_REG(0x322983)) & ~BIT(0),
		       MHAL_RIU_REG(0x322983));
		// reg_extint6
		writeb(readb(MHAL_RIU_REG(0x3229c3)) & ~BIT(0),
		       MHAL_RIU_REG(0x3229c3));
		// reg_allpad_in
		break;
	case PAD_GPIO12:
		// reg_fifthuartmode
		// reg_od5thuart
		writeb(readb(MHAL_RIU_REG(0x322945)) & ~(BIT(4) | BIT(0)),
		       MHAL_RIU_REG(0x322945));
		// reg_tconconfig7
		writeb(readb(MHAL_RIU_REG(0x322983)) & ~BIT(4),
		       MHAL_RIU_REG(0x322983));
		// reg_extint7
		writeb(readb(MHAL_RIU_REG(0x3229c3)) & ~BIT(4),
		       MHAL_RIU_REG(0x3229c3));
		// reg_allpad_in
		break;
	case PAD_GPIO13:
		// reg_p1_enable_b1
		writeb(readb(MHAL_RIU_REG(0x3229c8)) & ~BIT(4),
		       MHAL_RIU_REG(0x3229c8));
		// reg_allpad_in
		break;
	case PAD_GPIO14:
		// reg_i2s_out_md mode 4, in tmux but not in gpio mapping ??
		_mediatek_gpio_exclude_pad_mode(0x322928, 0x04 << 0,
						GENMASK(2, 0));
		// reg_allpad_in
		break;
	case PAD_GPIO15:
		// reg_i2smutemode mode 1
		_mediatek_gpio_exclude_pad_mode(0x32292b, 0x01 << 0,
						GENMASK(1, 0));
		// reg_i2s_out_md mode 4, in tmux but not in gpio mapping ??
		_mediatek_gpio_exclude_pad_mode(0x322928, 0x04 << 0,
						GENMASK(2, 0));
		// reg_allpad_in
		break;
	case PAD_GPIO16:
		// reg_allpad_in
		break;
	case PAD_GPIO18:
		// reg_freeze_tuner mode 3
		_mediatek_gpio_exclude_pad_mode(0x322979, 0x03 << 0,
						GENMASK(1, 0));
	case PAD_GPIO17:
		// reg_miic_mode2 mode 2
		_mediatek_gpio_exclude_pad_mode(0x322951, 0x02 << 0,
						GENMASK(1, 0));
		// reg_allpad_in
		break;
	case PAD_I2S_IN_BCK:
		// reg_gpio_i2s_in_pe_00
		writeb(readb(MHAL_RIU_REG(0x322e01)) | BIT(0),
		       MHAL_RIU_REG(0x322e01));
	case PAD_I2S_IN_WCK:
		// reg_i2s_in_md mode 1, 2, 3, 5
		_mediatek_gpio_exclude_pad_mode(0x322924, 0x01 << 0,
						GENMASK(2, 0));
		_mediatek_gpio_exclude_pad_mode(0x322924, 0x02 << 0,
						GENMASK(2, 0));
		_mediatek_gpio_exclude_pad_mode(0x322924, 0x03 << 0,
						GENMASK(2, 0));
		_mediatek_gpio_exclude_pad_mode(0x322924, 0x05 << 0,
						GENMASK(2, 0));
		// reg_thirduartmode mode 1
		_mediatek_gpio_exclude_pad_mode(0x322943, 0x01 << 0,
						GENMASK(1, 0));
		// reg_od3rduart mode 1
		_mediatek_gpio_exclude_pad_mode(0x322943, 0x01 << 0,
						GENMASK(5, 4));
		// reg_allpad_in
		break;
	case PAD_I2S_IN_DIN0:
		// reg_gpio_i2s_in_pe_02
		writeb(readb(MHAL_RIU_REG(0x322e05)) | BIT(0),
		       MHAL_RIU_REG(0x322e05));
		// reg_i2s_in_md mode 1, 2, 3, 5
		_mediatek_gpio_exclude_pad_mode(0x322924, 0x01 << 0,
						GENMASK(2, 0));
		_mediatek_gpio_exclude_pad_mode(0x322924, 0x02 << 0,
						GENMASK(2, 0));
		_mediatek_gpio_exclude_pad_mode(0x322924, 0x03 << 0,
						GENMASK(2, 0));
		_mediatek_gpio_exclude_pad_mode(0x322924, 0x05 << 0,
						GENMASK(2, 0));
		//reg_miic_mode2 mode 1
		_mediatek_gpio_exclude_pad_mode(0x322951, 0x01 << 0,
						GENMASK(1, 0));
		//reg_3dflagconfig mode 3
		_mediatek_gpio_exclude_pad_mode(0x322970, 0x01 << 0,
						GENMASK(1, 0));
		//reg_osd3dflag_config mode 3
		_mediatek_gpio_exclude_pad_mode(0x322970, 0x03 << 4,
						GENMASK(5, 4));
		// reg_allpad_in
		break;
	case PAD_I2S_IN_DIN1:
		// reg_i2s_in_md mode 1, 2
		_mediatek_gpio_exclude_pad_mode(0x322924, 0x01 << 0,
						GENMASK(2, 0));
		_mediatek_gpio_exclude_pad_mode(0x322924, 0x02 << 0,
						GENMASK(2, 0));
		// reg_i2s_in_sd2_md mode 1
		_mediatek_gpio_exclude_pad_mode(0x322924, 0x01 << 4,
						GENMASK(5, 4));
		//reg_miic_mode2 mode 1
		_mediatek_gpio_exclude_pad_mode(0x322951, 0x01 << 0,
						GENMASK(1, 0));
		// reg_allpad_in
		break;
	case PAD_I2S_IN_DIN2:
		// reg_gpio_i2s_in_pe_04
		writeb(readb(MHAL_RIU_REG(0x322e09)) | BIT(0),
		       MHAL_RIU_REG(0x322e09));
		//reg_mic_md mode 7, 8
		_mediatek_gpio_exclude_pad_mode(0x322920, 0x07 << 0,
						GENMASK(3, 0));
		_mediatek_gpio_exclude_pad_mode(0x322920, 0x08 << 0,
						GENMASK(3, 0));
		// reg_i2s_in_md mode 1, 5
		_mediatek_gpio_exclude_pad_mode(0x322924, 0x01 << 0,
						GENMASK(2, 0));
		_mediatek_gpio_exclude_pad_mode(0x322924, 0x05 << 0,
						GENMASK(2, 0));
		// reg_i2s_in_sd2_md mode 2
		_mediatek_gpio_exclude_pad_mode(0x322924, 0x02 << 4,
						GENMASK(5, 4));
		// reg_allpad_in
		break;
	case PAD_I2S_OUT_BCK:
		// reg_ej_config mode 2
		_mediatek_gpio_exclude_pad_mode(0x101e27, 0x02 << 0,
						GENMASK(1, 0));
		// reg_ej_diagnosis mode 2
		_mediatek_gpio_exclude_pad_mode(0x3229e4, 0x02 << 4,
						GENMASK(5, 4));
		// reg_test_in_mode mode 1
		_mediatek_gpio_exclude_pad_mode(0x3229f0, 0x01 << 0,
						GENMASK(1, 0));
		// reg_test_out_mode mode 1
		_mediatek_gpio_exclude_pad_mode(0x3229f0, 0x01 << 4,
						GENMASK(5, 4));
		// reg_agc_dbg
		writeb(readb(MHAL_RIU_REG(0x3229e8)) & ~BIT(0),
		       MHAL_RIU_REG(0x3229e8));
		// reg_tserrout mode 1
		_mediatek_gpio_exclude_pad_mode(0x3229e8, 0x01 << 4,
						GENMASK(5, 4));
		// reg_i2s_out_md mode 1, 2, 3, 4
		//note: reg have 3 bits, gpio mapping have 2 bits
		_mediatek_gpio_exclude_pad_mode(0x322928, 0x01 << 0,
						GENMASK(2, 0));
		_mediatek_gpio_exclude_pad_mode(0x322928, 0x02 << 0,
						GENMASK(2, 0));
		_mediatek_gpio_exclude_pad_mode(0x322928, 0x03 << 0,
						GENMASK(2, 0));
		_mediatek_gpio_exclude_pad_mode(0x322928, 0x04 << 0,
						GENMASK(2, 0));
		// reg_allpad_in
		break;
	case PAD_I2S_OUT_MCK:
		// reg_fram_delay_flag
		writeb(readb(MHAL_RIU_REG(0x322916)) & ~BIT(0),
		       MHAL_RIU_REG(0x322916));
		// reg_ej_config mode 2
		_mediatek_gpio_exclude_pad_mode(0x101e27, 0x02 << 0,
						GENMASK(1, 0));
		// reg_ej_diagnosis mode 2
		_mediatek_gpio_exclude_pad_mode(0x3229e4, 0x02 << 4,
						GENMASK(5, 4));
		// reg_test_in_mode mode 1
		_mediatek_gpio_exclude_pad_mode(0x3229f0, 0x01 << 0,
						GENMASK(1, 0));
		// reg_test_out_mode mode 1
		_mediatek_gpio_exclude_pad_mode(0x3229f0, 0x01 << 4,
						GENMASK(5, 4));
		// reg_agc_dbg
		writeb(readb(MHAL_RIU_REG(0x3229e8)) & ~BIT(0),
		       MHAL_RIU_REG(0x3229e8));
		// reg_tserrout mode 2
		_mediatek_gpio_exclude_pad_mode(0x3229e8, 0x02 << 4,
						GENMASK(5, 4));
		// reg_i2s_out_mck_md
		writeb(readb(MHAL_RIU_REG(0x32292c)) & ~BIT(4),
		       MHAL_RIU_REG(0x32292c));
		// reg_extint2
		writeb(readb(MHAL_RIU_REG(0x3229c1)) & ~BIT(0),
		       MHAL_RIU_REG(0x3229c1));
		// reg_allpad_in
		// reg_irtx_mode mode 3
		_mediatek_gpio_exclude_pad_mode(0x2e86, 0x03 << 3,
						GENMASK(4, 3));
		break;
	case PAD_I2S_OUT_SD0:
		// reg_ej_config mode 2
		_mediatek_gpio_exclude_pad_mode(0x101e27, 0x02 << 0,
						GENMASK(1, 0));
		// reg_ej_diagnosis mode 2
		_mediatek_gpio_exclude_pad_mode(0x3229e4, 0x02 << 4,
						GENMASK(5, 4));
		// reg_test_in_mode mode 1
		_mediatek_gpio_exclude_pad_mode(0x3229f0, 0x01 << 0,
						GENMASK(1, 0));
		// reg_test_out_mode mode 1
		_mediatek_gpio_exclude_pad_mode(0x3229f0, 0x01 << 4,
						GENMASK(5, 4));
		// reg_i2s_out_md mode 1, 2, 3, 4
		//note: reg have 3 bits, gpio mapping have 2 bits
		_mediatek_gpio_exclude_pad_mode(0x322928, 0x01 << 0,
						GENMASK(2, 0));
		_mediatek_gpio_exclude_pad_mode(0x322928, 0x02 << 0,
						GENMASK(2, 0));
		_mediatek_gpio_exclude_pad_mode(0x322928, 0x03 << 0,
						GENMASK(2, 0));
		_mediatek_gpio_exclude_pad_mode(0x322928, 0x04 << 0,
						GENMASK(2, 0));
		// reg_allpad_in
		break;
	case PAD_I2S_OUT_SD1:
		// reg_ej_diagnosis mode 2
		_mediatek_gpio_exclude_pad_mode(0x3229e4, 0x02 << 4,
						GENMASK(5, 4));
		//reg_spdifoutconfig2
		writeb(readb(MHAL_RIU_REG(0x322931)) & ~BIT(0),
		       MHAL_RIU_REG(0x322931));
		// reg_i2s_out_md mode 1, 2
		//note: reg have 3 bits, gpio mapping have 2 bits
		_mediatek_gpio_exclude_pad_mode(0x322928, 0x01 << 0,
						GENMASK(2, 0));
		_mediatek_gpio_exclude_pad_mode(0x322928, 0x02 << 0,
						GENMASK(2, 0));
		// reg_fourthuartmode mode 2
		_mediatek_gpio_exclude_pad_mode(0x322944, 0x02 << 0,
						GENMASK(1, 0));
		// reg_od4thuart mode 2
		_mediatek_gpio_exclude_pad_mode(0x322944, 0x02 << 4,
						GENMASK(5, 4));
		// reg_fastuartmode mode 2
		_mediatek_gpio_exclude_pad_mode(0x32294e, 0x02 << 0,
						GENMASK(1, 0));
		// reg_odfastuart mode 2
		_mediatek_gpio_exclude_pad_mode(0x32294e, 0x02 << 4,
						GENMASK(5, 4));
		// reg_miic_mode0 mode 1
		_mediatek_gpio_exclude_pad_mode(0x322950, 0x01 << 0,
						GENMASK(1, 0));
		// reg_allpad_in
		break;
	case PAD_I2S_OUT_SD2:
		// reg_mic_md mode 8
		_mediatek_gpio_exclude_pad_mode(0x322920, 0x08 << 0,
						GENMASK(3, 0));
		// reg_i2s_out_md mode 1
		//note: reg have 3 bits, gpio mapping have 2 bits
		_mediatek_gpio_exclude_pad_mode(0x322928, 0x01 << 0,
						GENMASK(2, 0));
		// reg_i2s_out_sd2_md
		writeb(readb(MHAL_RIU_REG(0x322928)) & ~BIT(4),
		       MHAL_RIU_REG(0x322928));
		// reg_fourthuartmode mode 2
		_mediatek_gpio_exclude_pad_mode(0x322944, 0x02 << 0,
						GENMASK(1, 0));
		// reg_od4thuart mode 2
		_mediatek_gpio_exclude_pad_mode(0x322944, 0x02 << 4,
						GENMASK(5, 4));
		// reg_fastuartmode mode 2
		_mediatek_gpio_exclude_pad_mode(0x32294e, 0x02 << 0,
						GENMASK(1, 0));
		// reg_odfastuart mode 2
		_mediatek_gpio_exclude_pad_mode(0x32294e, 0x02 << 4,
						GENMASK(5, 4));
		// reg_miic_mode0 mode 1
		_mediatek_gpio_exclude_pad_mode(0x322950, 0x01 << 0,
						GENMASK(1, 0));
		// reg_allpad_in
		break;
	case PAD_I2S_OUT_WCK:
		// reg_ej_config mode 2
		_mediatek_gpio_exclude_pad_mode(0x101e27, 0x02 << 0,
						GENMASK(1, 0));
		// reg_ej_diagnosis mode 2
		_mediatek_gpio_exclude_pad_mode(0x3229e4, 0x02 << 4,
						GENMASK(5, 4));
		// reg_i2s_out_md mode 1, 2, 3, 4
		//note: reg have 3 bits, gpio mapping have 2 bits
		_mediatek_gpio_exclude_pad_mode(0x322928, 0x01 << 0,
						GENMASK(2, 0));
		_mediatek_gpio_exclude_pad_mode(0x322928, 0x02 << 0,
						GENMASK(2, 0));
		_mediatek_gpio_exclude_pad_mode(0x322928, 0x03 << 0,
						GENMASK(2, 0));
		_mediatek_gpio_exclude_pad_mode(0x322928, 0x04 << 0,
						GENMASK(2, 0));
		// reg_allpad_in
		break;
	case PAD_LD_SPI_CK:
		// reg_gpio_ld_pe_03
		writeb(readb(MHAL_RIU_REG(0x322f27)) | BIT(0),
		       MHAL_RIU_REG(0x322f27));
		// reg_ej_config mode 1
		_mediatek_gpio_exclude_pad_mode(0x101e27, 0x01 << 0,
						GENMASK(1, 0));
		// reg_ld_spi1_config mode 1
		_mediatek_gpio_exclude_pad_mode(0x3229a0, 0x01 << 0,
						GENMASK(1, 0));
		// reg_ld_spi3_config mode 1, 2
		_mediatek_gpio_exclude_pad_mode(0x3229a1, 0x01 << 0,
						GENMASK(1, 0));
		_mediatek_gpio_exclude_pad_mode(0x3229a1, 0x02 << 0,
						GENMASK(1, 0));
		// reg_ld_qspi_config mode 1
		_mediatek_gpio_exclude_pad_mode(0x32295e, 0x01 << 0,
						GENMASK(1, 0));
		// reg_p1_enable_b5
		writeb(readb(MHAL_RIU_REG(0x3229ca)) & ~BIT(4),
		       MHAL_RIU_REG(0x3229ca));
		// reg_allpad_in
		break;
	case PAD_LD_SPI_CS:
		// reg_gpio_ld_pe_01
		writeb(readb(MHAL_RIU_REG(0x322f23)) | BIT(0),
		       MHAL_RIU_REG(0x322f23));
		// reg_ej_config mode 1
		_mediatek_gpio_exclude_pad_mode(0x101e27, 0x01 << 0,
						GENMASK(1, 0));
		// reg_ld_spi2_config mode 1
		_mediatek_gpio_exclude_pad_mode(0x3229a0, 0x01 << 4,
						GENMASK(5, 4));
		// reg_ld_spi3_config mode 1
		_mediatek_gpio_exclude_pad_mode(0x3229a1, 0x01 << 0,
						GENMASK(1, 0));
		// reg_ld_qspi_config mode 1
		_mediatek_gpio_exclude_pad_mode(0x32295e, 0x01 << 0,
						GENMASK(1, 0));
		// reg_seconduartmodemode 1
		_mediatek_gpio_exclude_pad_mode(0x322942, 0x01 << 0,
						GENMASK(1, 0));
		// reg_od2nduart mode 1
		_mediatek_gpio_exclude_pad_mode(0x322942, 0x01 << 4,
						GENMASK(5, 4));
		// reg_fourthuartmode mode 1
		_mediatek_gpio_exclude_pad_mode(0x322944, 0x01 << 0,
						GENMASK(1, 0));
		// reg_od4thuart mode 1
		_mediatek_gpio_exclude_pad_mode(0x322944, 0x01 << 4,
						GENMASK(5, 4));
		// reg_fastuartmode mode 1
		_mediatek_gpio_exclude_pad_mode(0x32294e, 0x01 << 0,
						GENMASK(1, 0));
		// reg_odfastuart mode 1
		_mediatek_gpio_exclude_pad_mode(0x32294e, 0x01 << 4,
						GENMASK(5, 4));
		// reg_p1_enable_b3
		writeb(readb(MHAL_RIU_REG(0x3229c9)) & ~BIT(4),
		       MHAL_RIU_REG(0x3229c9));
		// reg_allpad_in
		break;
	case PAD_LD_SPI_MISO:
		// reg_gpio_ld_pe_00
		writeb(readb(MHAL_RIU_REG(0x322f21)) | BIT(0),
		       MHAL_RIU_REG(0x322f21));
		// reg_ej_config mode 1
		_mediatek_gpio_exclude_pad_mode(0x101e27, 0x01 << 0,
						GENMASK(1, 0));
		// reg_ld_spi2_config mode 1
		_mediatek_gpio_exclude_pad_mode(0x3229a0, 0x01 << 4,
						GENMASK(5, 4));
		// reg_ld_spi3_config mode 1, 2
		_mediatek_gpio_exclude_pad_mode(0x3229a1, 0x01 << 0,
						GENMASK(1, 0));
		_mediatek_gpio_exclude_pad_mode(0x3229a1, 0x02 << 0,
						GENMASK(1, 0));
		// reg_ld_qspi_config mode 1
		_mediatek_gpio_exclude_pad_mode(0x32295e, 0x01 << 0,
						GENMASK(1, 0));
		// reg_fourthuartmode mode 1
		_mediatek_gpio_exclude_pad_mode(0x322944, 0x01 << 0,
						GENMASK(1, 0));
		// reg_od4thuart mode 1
		_mediatek_gpio_exclude_pad_mode(0x322944, 0x01 << 4,
						GENMASK(5, 4));
		// reg_fastuartmode mode 1
		_mediatek_gpio_exclude_pad_mode(0x32294e, 0x01 << 0,
						GENMASK(1, 0));
		// reg_odfastuart mode 1
		_mediatek_gpio_exclude_pad_mode(0x32294e, 0x01 << 4,
						GENMASK(5, 4));
		// reg_p1_enable_b2
		writeb(readb(MHAL_RIU_REG(0x3229c9)) & ~BIT(0),
		       MHAL_RIU_REG(0x3229c9));
		// reg_3dflagconfig mode 2
		_mediatek_gpio_exclude_pad_mode(0x322970, 0x02 << 0,
						GENMASK(1, 0));
		// reg_osd3dflag_config mode 2
		_mediatek_gpio_exclude_pad_mode(0x322970, 0x02 << 4,
						GENMASK(5, 4));
		// reg_allpad_in
		break;
	case PAD_LD_SPI_MOSI:
		// reg_gpio_ld_pe_02
		writeb(readb(MHAL_RIU_REG(0x322f25)) | BIT(0),
		       MHAL_RIU_REG(0x322f25));
		// reg_ej_config mode 1
		_mediatek_gpio_exclude_pad_mode(0x101e27, 0x01 << 0,
						GENMASK(1, 0));
		// reg_ld_spi1_config mode 1
		_mediatek_gpio_exclude_pad_mode(0x3229a0, 0x01 << 0,
						GENMASK(1, 0));
		// reg_ld_spi3_config mode 1, 2
		_mediatek_gpio_exclude_pad_mode(0x3229a1, 0x01 << 0,
						GENMASK(1, 0));
		_mediatek_gpio_exclude_pad_mode(0x3229a1, 0x02 << 0,
						GENMASK(1, 0));
		// reg_ld_qspi_config mode 1
		_mediatek_gpio_exclude_pad_mode(0x32295e, 0x01 << 0,
						GENMASK(1, 0));
		// reg_seconduartmodemode 1
		_mediatek_gpio_exclude_pad_mode(0x322942, 0x01 << 0,
						GENMASK(1, 0));
		// reg_od2nduart mode 1
		_mediatek_gpio_exclude_pad_mode(0x322942, 0x01 << 4,
						GENMASK(5, 4));
		// reg_p1_enable_b4
		writeb(readb(MHAL_RIU_REG(0x3229ca)) & ~BIT(0),
		       MHAL_RIU_REG(0x3229ca));
		// reg_allpad_in
		break;
	case PAD_MIC_BCK:
		// reg_gpio_dmic_pe[0]
		writeb(readb(MHAL_RIU_REG(0x2e88)) | BIT(6),
		       MHAL_RIU_REG(0x2e88));
		// reg_mic_md mode 1~7
		_mediatek_gpio_exclude_pad_mode(0x2e86, 0x01 << 0,
						GENMASK(2, 0));
		_mediatek_gpio_exclude_pad_mode(0x2e86, 0x02 << 0,
						GENMASK(2, 0));
		_mediatek_gpio_exclude_pad_mode(0x2e86, 0x03 << 0,
						GENMASK(2, 0));
		_mediatek_gpio_exclude_pad_mode(0x2e86, 0x04 << 0,
						GENMASK(2, 0));
		_mediatek_gpio_exclude_pad_mode(0x2e86, 0x05 << 0,
						GENMASK(2, 0));
		_mediatek_gpio_exclude_pad_mode(0x2e86, 0x06 << 0,
						GENMASK(2, 0));
		_mediatek_gpio_exclude_pad_mode(0x2e86, 0x07 << 0,
						GENMASK(2, 0));
		break;
	case PAD_MIC_SD0:
		// reg_gpio_dmic_pe[1]
		writeb(readb(MHAL_RIU_REG(0x2e88)) | BIT(7),
		       MHAL_RIU_REG(0x2e88));
		// reg_mic_md mode 1~7
		_mediatek_gpio_exclude_pad_mode(0x2e86, 0x01 << 0,
						GENMASK(2, 0));
		_mediatek_gpio_exclude_pad_mode(0x2e86, 0x02 << 0,
						GENMASK(2, 0));
		_mediatek_gpio_exclude_pad_mode(0x2e86, 0x03 << 0,
						GENMASK(2, 0));
		_mediatek_gpio_exclude_pad_mode(0x2e86, 0x04 << 0,
						GENMASK(2, 0));
		_mediatek_gpio_exclude_pad_mode(0x2e86, 0x05 << 0,
						GENMASK(2, 0));
		_mediatek_gpio_exclude_pad_mode(0x2e86, 0x06 << 0,
						GENMASK(2, 0));
		_mediatek_gpio_exclude_pad_mode(0x2e86, 0x07 << 0,
						GENMASK(2, 0));
		break;
	case PAD_MIC_SD1:
		// reg_gpio_dmic_pe[2]
		writeb(readb(MHAL_RIU_REG(0x2e89)) | BIT(0),
		       MHAL_RIU_REG(0x2e89));
		// reg_mic_md mode 1, 3~7
		_mediatek_gpio_exclude_pad_mode(0x2e86, 0x01 << 0,
						GENMASK(2, 0));
		_mediatek_gpio_exclude_pad_mode(0x2e86, 0x03 << 0,
						GENMASK(2, 0));
		_mediatek_gpio_exclude_pad_mode(0x2e86, 0x04 << 0,
						GENMASK(2, 0));
		_mediatek_gpio_exclude_pad_mode(0x2e86, 0x05 << 0,
						GENMASK(2, 0));
		_mediatek_gpio_exclude_pad_mode(0x2e86, 0x06 << 0,
						GENMASK(2, 0));
		_mediatek_gpio_exclude_pad_mode(0x2e86, 0x07 << 0,
						GENMASK(2, 0));
		break;
	case PAD_MIC_SD2:
		// reg_mic_md mode 4~7
		_mediatek_gpio_exclude_pad_mode(0x2e86, 0x04 << 0,
						GENMASK(2, 0));
		_mediatek_gpio_exclude_pad_mode(0x2e86, 0x05 << 0,
						GENMASK(2, 0));
		_mediatek_gpio_exclude_pad_mode(0x2e86, 0x06 << 0,
						GENMASK(2, 0));
		_mediatek_gpio_exclude_pad_mode(0x2e86, 0x07 << 0,
						GENMASK(2, 0));
		break;
	case PAD_PCM2_CD_N:
		// reg_pcm2_cdn_config
		writeb(readb(MHAL_RIU_REG(0x322911)) & ~BIT(4),
		       MHAL_RIU_REG(0x322911));
	case PAD_PCM2_CE_N:
		// reg_pcm2ctrlconfig
		writeb(readb(MHAL_RIU_REG(0x322911)) & ~BIT(0),
		       MHAL_RIU_REG(0x322911));
		// reg_mspi3_config mode 3, 4
		_mediatek_gpio_exclude_pad_mode(0x322958, 0x03 << 0,
						GENMASK(2, 0));
		_mediatek_gpio_exclude_pad_mode(0x322958, 0x04 << 0,
						GENMASK(2, 0));
		// reg_sdio_config
		writeb(readb(MHAL_RIU_REG(0x322968)) & ~BIT(0),
		       MHAL_RIU_REG(0x322968));
		// reg_allpad_in
		break;
	case PAD_PCM2_IRQA_N:
		// reg_diseqc_out_config
		writeb(readb(MHAL_RIU_REG(0x322978)) & ~BIT(4),
		       MHAL_RIU_REG(0x322978));
		// reg_pcm2ctrlconfig
		writeb(readb(MHAL_RIU_REG(0x322911)) & ~BIT(0),
		       MHAL_RIU_REG(0x322911));
		// reg_mspi3_config, not in tmux but in gpio mapping ??
		// reg_allpad_in
		break;
	case PAD_PCM2_RESET:
		// reg_diseqc_in_config
		writeb(readb(MHAL_RIU_REG(0x322978)) & ~BIT(0),
		       MHAL_RIU_REG(0x322978));
		// reg_pcm2ctrlconfig
		writeb(readb(MHAL_RIU_REG(0x322911)) & ~BIT(0),
		       MHAL_RIU_REG(0x322911));
		// reg_mspi3_config mode 3
		// note: in tmux but not in gpio mapping ??
		_mediatek_gpio_exclude_pad_mode(0x322958, 0x03 << 0,
						GENMASK(2, 0));
		// reg_extint3
		writeb(readb(MHAL_RIU_REG(0x3229c1)) & ~BIT(4),
		       MHAL_RIU_REG(0x3229c1));
		// reg_allpad_in
		break;
	case PAD_PCM2_WAIT_N:
		// reg_pcm2ctrlconfig
		writeb(readb(MHAL_RIU_REG(0x322911)) & ~BIT(0),
		       MHAL_RIU_REG(0x322911));
		// reg_mspi3_config mode 3
		_mediatek_gpio_exclude_pad_mode(0x322958, 0x03 << 0,
						GENMASK(2, 0));
		// reg_allpad_in
		break;
	case PAD_PCM_A0:
	case PAD_PCM_A1:
	case PAD_PCM_A2:
	case PAD_PCM_A3:
	case PAD_PCM_A4:
	case PAD_PCM_A5:
	case PAD_PCM_A6:
	case PAD_PCM_A7:
		// reg_nand_mode mode 2
		_mediatek_gpio_exclude_pad_mode(0x322964, 0x02 << 4,
						GENMASK(1, 0));
	case PAD_PCM_A8:
	case PAD_PCM_A9:
	case PAD_PCM_A10:
	case PAD_PCM_A11:
	case PAD_PCM_A12:
	case PAD_PCM_A13:
	case PAD_PCM_A14:
	case PAD_PCM_D0:
	case PAD_PCM_D1:
	case PAD_PCM_D2:
	case PAD_PCM_D3:
	case PAD_PCM_D4:
	case PAD_PCM_D5:
	case PAD_PCM_D6:
	case PAD_PCM_D7:
		//reg_test_in_mode mode 3
		_mediatek_gpio_exclude_pad_mode(0x3229f0, 0x03 << 0,
						GENMASK(1, 0));
		// reg_test_out_mode mode 3
		_mediatek_gpio_exclude_pad_mode(0x3229f0, 0x03 << 4,
						GENMASK(5, 4));
		// reg_pcmadconfig
		writeb(readb(MHAL_RIU_REG(0x322910)) & ~BIT(0),
		       MHAL_RIU_REG(0x322910));
		// reg_allpad_in
		break;
	case PAD_PCM_CD_N:
		//reg_test_in_mode mode 3
		_mediatek_gpio_exclude_pad_mode(0x3229f0, 0x03 << 0,
						GENMASK(1, 0));
		// reg_test_out_mode mode 3
		_mediatek_gpio_exclude_pad_mode(0x3229f0, 0x03 << 4,
						GENMASK(5, 4));
		// reg_pcmctrlconfig
		writeb(readb(MHAL_RIU_REG(0x322910)) & ~BIT(4),
		       MHAL_RIU_REG(0x322910));
		// reg_allpad_in
		break;
	case PAD_PCM_CE_N:
	case PAD_PCM_IORD_N:
	case PAD_PCM_IOWR_N:
	case PAD_PCM_IRQA_N:
	case PAD_PCM_OE_N:
	case PAD_PCM_WE_N:
		// reg_sm_config mode 1
		_mediatek_gpio_exclude_pad_mode(0x322914, 0x01 << 0,
						GENMASK(1, 0));
	case PAD_PCM_REG_N:
	case PAD_PCM_RESET:
	case PAD_PCM_WAIT_N:
		// reg_pcmctrlconfig
		writeb(readb(MHAL_RIU_REG(0x322910)) & ~BIT(4),
		       MHAL_RIU_REG(0x322910));
		// reg_allpad_in
		break;
	case PAD_PWM0:
		// reg_vsense_en
		writeb(readb(MHAL_RIU_REG(0x3229f8)) & ~BIT(0),
		       MHAL_RIU_REG(0x3229f8));
		// reg_pwm0_mode
		writeb(readb(MHAL_RIU_REG(0x322990)) & ~BIT(0),
		       MHAL_RIU_REG(0x322990));
		// reg_allpad_in
		break;
	case PAD_PWM1:
		// reg_vsync_like_config mode 3, 4
		_mediatek_gpio_exclude_pad_mode(0x3229a4, 0x03 << 0,
						GENMASK(2, 0));
		_mediatek_gpio_exclude_pad_mode(0x3229a4, 0x04 << 0,
						GENMASK(2, 0));
		// reg_pwm1_mode
		writeb(readb(MHAL_RIU_REG(0x322990)) & ~BIT(4),
		       MHAL_RIU_REG(0x322990));
		// reg_allpad_in
		break;
	case PAD_PWM2:
		// reg_i2smutemode mode 2
		_mediatek_gpio_exclude_pad_mode(0x32292b, 0x02 << 0,
						GENMASK(1, 0));
		// reg_pwm2_mode mode 1
		_mediatek_gpio_exclude_pad_mode(0x322991, 0x01 << 0,
						GENMASK(1, 0));
		// reg_extint0
		writeb(readb(MHAL_RIU_REG(0x3229c0)) & ~BIT(0),
		       MHAL_RIU_REG(0x3229c0));
		// reg_lg_earc_mode
		writeb(readb(MHAL_RIU_REG(0x32297c)) & ~BIT(4),
		       MHAL_RIU_REG(0x32297c));
		//reg_allpad_in
		//reg_irtx_mode mode 2
		_mediatek_gpio_exclude_pad_mode(0x2e86, 0x02 << 3,
						GENMASK(4, 3));
		break;
	case PAD_SPDIF_IN:
		// reg_spdifinconfig
		writeb(readb(MHAL_RIU_REG(0x322930)) & ~BIT(0),
		       MHAL_RIU_REG(0x322930));
		// reg_tconconfig5 mode 2
		_mediatek_gpio_exclude_pad_mode(0x322982, 0x02 << 4,
						GENMASK(5, 4));
		// reg_extint1
		writeb(readb(MHAL_RIU_REG(0x3229c0)) & ~BIT(4),
		       MHAL_RIU_REG(0x3229c0));
		// reg_3dflagconfig mode 1
		_mediatek_gpio_exclude_pad_mode(0x322970, 0x01 << 0,
						GENMASK(1, 0));
		// reg_osd3dflag_config mode 1
		_mediatek_gpio_exclude_pad_mode(0x322974, 0x01 << 4,
						GENMASK(5, 4));
		// reg_allpad_in
		break;
	case PAD_SPDIF_OUT:
		// reg_spdifoutconfig
		writeb(readb(MHAL_RIU_REG(0x322930)) & ~BIT(4),
		       MHAL_RIU_REG(0x322930));
		// reg_allpad_in
		break;
	case PAD_TCON0:
		// reg_gpio_tcon_pe_00
		writeb(readb(MHAL_RIU_REG(0x322f01)) | BIT(0),
		       MHAL_RIU_REG(0x322f01));
		// reg_seconduartmode mode 3
		_mediatek_gpio_exclude_pad_mode(0x322942, 0x03 << 0,
						GENMASK(1, 0));
		// reg_od2nduart mode 3
		_mediatek_gpio_exclude_pad_mode(0x322942, 0x03 << 4,
						GENMASK(5, 4));
		// reg_miic_mode0 mode 2
		_mediatek_gpio_exclude_pad_mode(0x322950, 0x02 << 0,
						GENMASK(1, 0));
		// reg_tconconfig0
		writeb(readb(MHAL_RIU_REG(0x322980)) & ~BIT(0),
		       MHAL_RIU_REG(0x322980));
		// reg_allpad_in
		break;
	case PAD_TCON1:
		// reg_gpio_tcon_pe_01
		writeb(readb(MHAL_RIU_REG(0x322f03)) | BIT(0),
		       MHAL_RIU_REG(0x322f03));
		// reg_seconduartmode mode 3
		_mediatek_gpio_exclude_pad_mode(0x322942, 0x03 << 0,
						GENMASK(1, 0));
		// reg_od2nduart mode 3
		_mediatek_gpio_exclude_pad_mode(0x322942, 0x03 << 4,
						GENMASK(5, 4));
		// reg_miic_mode0 mode 2
		_mediatek_gpio_exclude_pad_mode(0x322950, 0x02 << 0,
						GENMASK(1, 0));
		// reg_tconconfig1
		writeb(readb(MHAL_RIU_REG(0x322980)) & ~BIT(4),
		       MHAL_RIU_REG(0x322980));
		// reg_allpad_in
		break;
	case PAD_TCON2:
		// reg_gpio_tcon_pe_02
		writeb(readb(MHAL_RIU_REG(0x322f05)) | BIT(0),
		       MHAL_RIU_REG(0x322f05));
		// reg_thirduartmode mode 3
		_mediatek_gpio_exclude_pad_mode(0x322943, 0x03 << 0,
						GENMASK(1, 0));
		// reg_od3rduart mode 3
		_mediatek_gpio_exclude_pad_mode(0x322943, 0x03 << 4,
						GENMASK(5, 4));
		// reg_tconconfig2
		writeb(readb(MHAL_RIU_REG(0x322981)) & ~BIT(0),
		       MHAL_RIU_REG(0x322981));
		// reg_allpad_in
		break;
	case PAD_TCON3:
		// reg_thirduartmode mode 3
		_mediatek_gpio_exclude_pad_mode(0x322943, 0x03 << 0,
						GENMASK(1, 0));
		// reg_od3rduart mode 3
		_mediatek_gpio_exclude_pad_mode(0x322943, 0x03 << 4,
						GENMASK(5, 4));
		// reg_tconconfig3
		writeb(readb(MHAL_RIU_REG(0x322981)) & ~BIT(4),
		       MHAL_RIU_REG(0x322981));
		// reg_allpad_in
		break;
	case PAD_TCON4:
		// reg_pwm2_mode mode 2
		_mediatek_gpio_exclude_pad_mode(0x322991, 0x02 << 0,
						GENMASK(1, 0));
		// reg_tconconfig4
		writeb(readb(MHAL_RIU_REG(0x322982)) & ~BIT(0),
		       MHAL_RIU_REG(0x322982));
		// reg_allpad_in
		break;
	case PAD_TGPIO0:
		// reg_vsync_vif_out_en
		writeb(readb(MHAL_RIU_REG(0x322979)) & ~BIT(4),
		       MHAL_RIU_REG(0x322979));
		//reg_freeze_tuner mode 1
		_mediatek_gpio_exclude_pad_mode(0x322979, 0x01 << 0,
						GENMASK(1, 0));
		// reg_allpad_in
		break;
	case PAD_TGPIO1:
		//reg_freeze_tuner mode 2
		_mediatek_gpio_exclude_pad_mode(0x322979, 0x02 << 0,
						GENMASK(1, 0));
		// reg_allpad_in
		break;
	case PAD_TGPIO2:
	case PAD_TGPIO3:
		// reg_miic_mode1
		writeb(readb(MHAL_RIU_REG(0x322950)) & ~BIT(4),
		       MHAL_RIU_REG(0x322950));
		// reg_allpad_in
		break;
	case PAD_TS0_CLK:
	case PAD_TS0_SYNC:
		// reg_ts0config mode 4
		_mediatek_gpio_exclude_pad_mode(0x322900, 0x04 << 0,
						GENMASK(2, 0));
	case PAD_TS0_VLD:
		// reg_ts0config mode 1, 2, 3
		_mediatek_gpio_exclude_pad_mode(0x322900, 0x01 << 0,
						GENMASK(2, 0));
		_mediatek_gpio_exclude_pad_mode(0x322900, 0x02 << 0,
						GENMASK(2, 0));
		_mediatek_gpio_exclude_pad_mode(0x322900, 0x03 << 0,
						GENMASK(2, 0));
		// reg_allpad_in
		break;
	case PAD_TS0_D0:
		// reg_ts0config mode 2, 3, 4
		_mediatek_gpio_exclude_pad_mode(0x322900, 0x02 << 0,
						GENMASK(2, 0));
		_mediatek_gpio_exclude_pad_mode(0x322900, 0x03 << 0,
						GENMASK(2, 0));
		_mediatek_gpio_exclude_pad_mode(0x322900, 0x04 << 0,
						GENMASK(2, 0));
	case PAD_TS0_D1:
	case PAD_TS0_D2:
	case PAD_TS0_D3:
	case PAD_TS0_D4:
	case PAD_TS0_D5:
	case PAD_TS0_D6:
	case PAD_TS0_D7:
		// reg_test_in_mode mode 1
		_mediatek_gpio_exclude_pad_mode(0x3229f0, 0x01 << 0,
						GENMASK(1, 0));
		// reg_test_out_mode mode 1
		_mediatek_gpio_exclude_pad_mode(0x3229f0, 0x01 << 4,
						GENMASK(5, 4));
		// reg_ts0config mode 1
		_mediatek_gpio_exclude_pad_mode(0x322900, 0x01 << 0,
						GENMASK(2, 0));
		// reg_allpad_in
		break;
	case PAD_TS1_CLK:
	case PAD_TS1_SYNC:
	case PAD_TS1_D0:
		// reg_ts1config mode 4
		_mediatek_gpio_exclude_pad_mode(0x322900, 0x04 << 4,
						GENMASK(6, 4));
	case PAD_TS1_VLD:
		// reg_ej_diagnosis mode 1
		_mediatek_gpio_exclude_pad_mode(0x3229e4, 0x01 << 4,
						GENMASK(5, 4));
		// reg_test_in_mode mode 1
		_mediatek_gpio_exclude_pad_mode(0x3229f0, 0x01 << 0,
						GENMASK(1, 0));
		// reg_test_out_mode mode 1
		_mediatek_gpio_exclude_pad_mode(0x3229f0, 0x01 << 4,
						GENMASK(5, 4));
		// reg_ts1config mode 1, 2, 3, 5, 6
		_mediatek_gpio_exclude_pad_mode(0x322900, 0x01 << 4,
						GENMASK(6, 4));
		_mediatek_gpio_exclude_pad_mode(0x322900, 0x02 << 4,
						GENMASK(6, 4));
		_mediatek_gpio_exclude_pad_mode(0x322900, 0x03 << 4,
						GENMASK(6, 4));
		_mediatek_gpio_exclude_pad_mode(0x322900, 0x05 << 4,
						GENMASK(6, 4));
		_mediatek_gpio_exclude_pad_mode(0x322900, 0x06 << 4,
						GENMASK(6, 4));
		// reg_ts_out_modemode 1, 2, 3, 4
		_mediatek_gpio_exclude_pad_mode(0x322906, 0x01 << 0,
						GENMASK(2, 0));
		_mediatek_gpio_exclude_pad_mode(0x322906, 0x02 << 0,
						GENMASK(2, 0));
		_mediatek_gpio_exclude_pad_mode(0x322906, 0x03 << 0,
						GENMASK(2, 0));
		_mediatek_gpio_exclude_pad_mode(0x322906, 0x04 << 0,
						GENMASK(2, 0));
		// reg_allpad_in
		break;
	case PAD_TS1_D1:
		// reg_ej_diagnosis mode 1
		_mediatek_gpio_exclude_pad_mode(0x3229e4, 0x01 << 4,
						GENMASK(5, 4));
	case PAD_TS1_D2:
	case PAD_TS1_D3:
	case PAD_TS1_D4:
	case PAD_TS1_D5:
	case PAD_TS1_D6:
		// reg_sm_config mode 2
		_mediatek_gpio_exclude_pad_mode(0x322914, 0x02 << 0,
						GENMASK(1, 0));
	case PAD_TS1_D7:
		// reg_test_in_mode mode 1
		_mediatek_gpio_exclude_pad_mode(0x3229f0, 0x01 << 0,
						GENMASK(1, 0));
		// reg_test_out_mode mode 1
		_mediatek_gpio_exclude_pad_mode(0x3229f0, 0x01 << 4,
						GENMASK(5, 4));
		// reg_ts1config mode 1, 2
		_mediatek_gpio_exclude_pad_mode(0x322900, 0x01 << 4,
						GENMASK(6, 4));
		_mediatek_gpio_exclude_pad_mode(0x322900, 0x02 << 4,
						GENMASK(6, 4));
		// reg_ts_out_modemode 1, 2, 3, 4
		_mediatek_gpio_exclude_pad_mode(0x322906, 0x01 << 0,
						GENMASK(2, 0));
		_mediatek_gpio_exclude_pad_mode(0x322906, 0x02 << 0,
						GENMASK(2, 0));
		_mediatek_gpio_exclude_pad_mode(0x322906, 0x03 << 0,
						GENMASK(2, 0));
		_mediatek_gpio_exclude_pad_mode(0x322906, 0x04 << 0,
						GENMASK(2, 0));
		// reg_allpad_in
		break;
	case PAD_TS2_CLK:
		// reg_ts2config mode 1, 2, 3, 4
		_mediatek_gpio_exclude_pad_mode(0x322901, 0x01 << 0,
						GENMASK(2, 0));
		_mediatek_gpio_exclude_pad_mode(0x322901, 0x02 << 0,
						GENMASK(2, 0));
		_mediatek_gpio_exclude_pad_mode(0x322901, 0x03 << 0,
						GENMASK(2, 0));
		_mediatek_gpio_exclude_pad_mode(0x322901, 0x04 << 0,
						GENMASK(2, 0));
		// reg_i2s_bt_md mode 1
		_mediatek_gpio_exclude_pad_mode(0x32292c, 0x01 << 0,
						GENMASK(1, 0));
		// reg_mspi3_config mode 1
		_mediatek_gpio_exclude_pad_mode(0x322958, 0x01 << 0,
						GENMASK(2, 0));
		// reg_seconduartmode mode 2
		_mediatek_gpio_exclude_pad_mode(0x322942, 0x02 << 0,
						GENMASK(1, 0));
		// reg_od2nduart mode 2
		_mediatek_gpio_exclude_pad_mode(0x322942, 0x02 << 4,
						GENMASK(5, 4));
		// reg_miic_mode0 mode 3
		_mediatek_gpio_exclude_pad_mode(0x322950, 0x03 << 0,
						GENMASK(1, 0));
		// reg_tconconfig10
		writeb(readb(MHAL_RIU_REG(0x322985)) & ~BIT(0),
		       MHAL_RIU_REG(0x322985));
		// reg_sdio_config
		writeb(readb(MHAL_RIU_REG(0x322968)) & ~BIT(0),
		       MHAL_RIU_REG(0x322968));
		// reg_allpad_in
		break;
	case PAD_TS2_SYNC:
		// reg_ts2config mode 1, 2, 3, 4
		_mediatek_gpio_exclude_pad_mode(0x322901, 0x01 << 0,
						GENMASK(2, 0));
		_mediatek_gpio_exclude_pad_mode(0x322901, 0x02 << 0,
						GENMASK(2, 0));
		_mediatek_gpio_exclude_pad_mode(0x322901, 0x03 << 0,
						GENMASK(2, 0));
		_mediatek_gpio_exclude_pad_mode(0x322901, 0x04 << 0,
						GENMASK(2, 0));
		// reg_i2s_bt_md mode 1
		_mediatek_gpio_exclude_pad_mode(0x32292c, 0x01 << 0,
						GENMASK(1, 0));
		// reg_mspi3_config mode 1, 2
		_mediatek_gpio_exclude_pad_mode(0x322958, 0x01 << 0,
						GENMASK(2, 0));
		_mediatek_gpio_exclude_pad_mode(0x322958, 0x02 << 0,
						GENMASK(2, 0));
		// reg_seconduartmode mode 2
		_mediatek_gpio_exclude_pad_mode(0x322942, 0x02 << 0,
						GENMASK(1, 0));
		// reg_od2nduart mode 2
		_mediatek_gpio_exclude_pad_mode(0x322942, 0x02 << 4,
						GENMASK(5, 4));
		// reg_miic_mode0 mode 3
		_mediatek_gpio_exclude_pad_mode(0x322950, 0x03 << 0,
						GENMASK(1, 0));
		// reg_tconconfig9
		writeb(readb(MHAL_RIU_REG(0x322984)) & ~BIT(4),
		       MHAL_RIU_REG(0x322984));
		// reg_sdio_config
		writeb(readb(MHAL_RIU_REG(0x322968)) & ~BIT(0),
		       MHAL_RIU_REG(0x322968));
		// reg_allpad_in
		break;
	case PAD_TS2_D0:
		// reg_ts2config mode 1, 2, 3, 4
		_mediatek_gpio_exclude_pad_mode(0x322901, 0x01 << 0,
						GENMASK(2, 0));
		_mediatek_gpio_exclude_pad_mode(0x322901, 0x02 << 0,
						GENMASK(2, 0));
		_mediatek_gpio_exclude_pad_mode(0x322901, 0x03 << 0,
						GENMASK(2, 0));
		_mediatek_gpio_exclude_pad_mode(0x322901, 0x04 << 0,
						GENMASK(2, 0));
		// reg_i2s_bt_md mode 1, 2
		_mediatek_gpio_exclude_pad_mode(0x32292c, 0x01 << 0,
						GENMASK(1, 0));
		_mediatek_gpio_exclude_pad_mode(0x32292c, 0x02 << 0,
						GENMASK(1, 0));
		// reg_mspi3_config mode 1, 2
		_mediatek_gpio_exclude_pad_mode(0x322958, 0x01 << 0,
						GENMASK(2, 0));
		_mediatek_gpio_exclude_pad_mode(0x322958, 0x02 << 0,
						GENMASK(2, 0));
		// reg_thirduartmode mode 2
		_mediatek_gpio_exclude_pad_mode(0x322943, 0x02 << 0,
						GENMASK(1, 0));
		// reg_od3rduart mode 2
		_mediatek_gpio_exclude_pad_mode(0x322943, 0x02 << 4,
						GENMASK(5, 4));
		// reg_tconconfig5 mode 1
		_mediatek_gpio_exclude_pad_mode(0x322982, 0x01 << 4,
						GENMASK(5, 4));
		// reg_sdio_config
		writeb(readb(MHAL_RIU_REG(0x322968)) & ~BIT(0),
		       MHAL_RIU_REG(0x322968));
		// reg_allpad_in
		break;
	case PAD_TS2_VLD:
		// reg_ts2config mode 1, 2, 4
		_mediatek_gpio_exclude_pad_mode(0x322901, 0x01 << 0,
						GENMASK(2, 0));
		_mediatek_gpio_exclude_pad_mode(0x322901, 0x02 << 0,
						GENMASK(2, 0));
		_mediatek_gpio_exclude_pad_mode(0x322901, 0x04 << 0,
						GENMASK(2, 0));
		// reg_i2s_bt_md mode 1, 2
		_mediatek_gpio_exclude_pad_mode(0x32292c, 0x01 << 0,
						GENMASK(1, 0));
		_mediatek_gpio_exclude_pad_mode(0x32292c, 0x02 << 0,
						GENMASK(1, 0));
		// reg_mspi3_config mode 1
		_mediatek_gpio_exclude_pad_mode(0x322958, 0x01 << 0,
						GENMASK(2, 0));
		// reg_thirduartmode mode 2
		_mediatek_gpio_exclude_pad_mode(0x322943, 0x02 << 0,
						GENMASK(1, 0));
		// reg_od3rduart mode 2
		_mediatek_gpio_exclude_pad_mode(0x322943, 0x02 << 4,
						GENMASK(5, 4));
		// reg_tconconfig8
		writeb(readb(MHAL_RIU_REG(0x322984)) & ~BIT(0),
		       MHAL_RIU_REG(0x322984));
		// reg_sdio_config
		writeb(readb(MHAL_RIU_REG(0x322968)) & ~BIT(0),
		       MHAL_RIU_REG(0x322968));
		// reg_allpad_in
		break;
	case PAD_TS2_D1:
		// reg_ts2config mode 1, 2
		_mediatek_gpio_exclude_pad_mode(0x322901, 0x01 << 0,
						GENMASK(2, 0));
		_mediatek_gpio_exclude_pad_mode(0x322901, 0x02 << 0,
						GENMASK(2, 0));
		// reg_allpad_in
		break;
	case PAD_TS2_D4:
	case PAD_TS2_D5:
	case PAD_TS2_D6:
	case PAD_TS2_D7:
		// reg_ts4config mode 2
		_mediatek_gpio_exclude_pad_mode(0x322902, 0x02 << 0,
						GENMASK(1, 0));
	case PAD_TS2_D3:
		// reg_ts4config mode 1
		_mediatek_gpio_exclude_pad_mode(0x322902, 0x01 << 0,
						GENMASK(1, 0));
	case PAD_TS2_D2:
		// reg_ts2config mode 1
		_mediatek_gpio_exclude_pad_mode(0x322901, 0x01 << 0,
						GENMASK(2, 0));
		// reg_allpad_in
		break;
	case PAD_TS3_CLK:
	case PAD_TS3_D0:
	case PAD_TS3_SYNC:
	case PAD_TS3_VLD:
		// reg_ts3config mode 2
		_mediatek_gpio_exclude_pad_mode(0x322901, 0x02 << 4,
						GENMASK(5, 4));
	case PAD_TS3_D1:
	case PAD_TS3_D2:
	case PAD_TS3_D3:
	case PAD_TS3_D4:
	case PAD_TS3_D5:
	case PAD_TS3_D6:
	case PAD_TS3_D7:
		// reg_ts3config mode 1
		_mediatek_gpio_exclude_pad_mode(0x322901, 0x01 << 4,
						GENMASK(5, 4));
		// reg_allpad_in
		break;
	case PAD_TCON5:
		// reg_tconconfig5 mode 3
		_mediatek_gpio_exclude_pad_mode(0x322982, 0x03 << 4,
						GENMASK(5, 4));
		// reg_vby1_osd
		writeb(readb(MHAL_RIU_REG(0x3229e9)) & ~BIT(0),
		       MHAL_RIU_REG(0x3229e9));
		// reg_allpad_in
		break;
	case PAD_TCON6:
		// reg_tconconfig6 mode 2
		_mediatek_gpio_exclude_pad_mode(0x322983, 0x02 << 0,
						GENMASK(1, 0));
		// reg_vby1_osd
		writeb(readb(MHAL_RIU_REG(0x3229e9)) & ~BIT(0),
		       MHAL_RIU_REG(0x3229e9));
		// reg_allpad_in
		break;
	case PAD_TCON7:
		// reg_tconconfig7 mode 2
		_mediatek_gpio_exclude_pad_mode(0x322983, 0x02 << 4,
						GENMASK(5, 4));
		// reg_vby1_vid mode 2
		_mediatek_gpio_exclude_pad_mode(0x3229e9, 0x02 << 4,
						GENMASK(5, 4));
		// reg_allpad_in
		break;
	case PAD_TCON8:
		// reg_tconconfig8 mode 2, 3
		_mediatek_gpio_exclude_pad_mode(0x322984, 0x02 << 0,
						GENMASK(1, 0));
		_mediatek_gpio_exclude_pad_mode(0x322984, 0x03 << 0,
						GENMASK(1, 0));
		// reg_vby1_vid mode 2
		_mediatek_gpio_exclude_pad_mode(0x3229e9, 0x02 << 4,
						GENMASK(5, 4));
		// reg_allpad_in
		break;
	case PAD_GPIO29_PM:
	case PAD_GPIO30_PM:
	case PAD_GPIO31_PM:
		break;
	default:
		debug("pin %d does not have gpio registers setting\n", offset);
		return -EINVAL;
	}

	return 0;
}

static int mediatek_gpio_get_value(struct udevice *dev, unsigned int offset)
{
	struct mediatek_gpio_priv *priv = dev_get_priv(dev);
	u8 reg;

	if (!test_bit(offset, priv->requested))
		return -EPERM;

	reg = readb(MHAL_RIU_REG(gpio_table[offset].r_in));

	debug("%s, offset %d\n", __func__, offset);
	debug("reg %p value 0x%02X, check mask 0x%02X\n",
	      MHAL_RIU_REG(gpio_table[offset].r_in),
	      readb(MHAL_RIU_REG(gpio_table[offset].r_in)),
	      gpio_table[offset].m_in);

	return (reg & gpio_table[offset].m_in) ? 1 : 0;
}

static int mediatek_gpio_set_value(struct udevice *dev, unsigned int offset,
				   int value)
{
	struct mediatek_gpio_priv *priv = dev_get_priv(dev);
	u8 reg;

	if (!test_bit(offset, priv->requested))
		return -EPERM;

	if (offset == PAD_IRIN) {
		dev_err(dev, "GPIO %d have no output function\n", offset);
		return -EPERM;
	}

	debug("%s, offset %d, set value %d\n", __func__, offset, value);
	debug("reg %p value 0x%02X, set mask 0x%02X\n",
	      MHAL_RIU_REG(gpio_table[offset].r_out),
	      readb(MHAL_RIU_REG(gpio_table[offset].r_out)),
	      gpio_table[offset].m_out);

	reg = readb(MHAL_RIU_REG(gpio_table[offset].r_out));
	if (value)
		reg |= gpio_table[offset].m_out;
	else
		reg &= ~gpio_table[offset].m_out;

	writeb(reg, MHAL_RIU_REG(gpio_table[offset].r_out));

	return 0;
}

static int mediatek_gpio_direction_input(struct udevice *dev,
					 unsigned int offset)
{
	struct mediatek_gpio_priv *priv = dev_get_priv(dev);

	if (!test_bit(offset, priv->requested))
		return -EPERM;

	debug("%s, pin %d reg %p value 0x%02X, set mask 0x%02X\n", __func__,
	      offset, MHAL_RIU_REG(gpio_table[offset].r_oen),
	      readb(MHAL_RIU_REG(gpio_table[offset].r_oen)),
	      gpio_table[offset].m_oen);
	writeb(readb(MHAL_RIU_REG(gpio_table[offset].r_oen)) |
	       gpio_table[offset].m_oen,
	       MHAL_RIU_REG(gpio_table[offset].r_oen));
	return 0;
}

static int mediatek_gpio_direction_output(struct udevice *dev,
					  unsigned int offset, int value)
{
	struct mediatek_gpio_priv *priv = dev_get_priv(dev);

	if (!test_bit(offset, priv->requested))
		return -EPERM;

	if (offset == PAD_IRIN) {
		dev_err(dev, "GPIO %d have no output function\n", offset);
		return -EPERM;
	}

	// set direction output
	debug("%s, pin %d reg %p value 0x%02X, clear mask 0x%02X\n", __func__,
	      offset, MHAL_RIU_REG(gpio_table[offset].r_oen),
	      readb(MHAL_RIU_REG(gpio_table[offset].r_oen)),
	      gpio_table[offset].m_oen);
	writeb(readb(MHAL_RIU_REG(gpio_table[offset].r_oen)) &
	       ~gpio_table[offset].m_oen,
	       MHAL_RIU_REG(gpio_table[offset].r_oen));

	// set output value
	debug("reg %p value 0x%02X, set value %d, mask 0x%02X\n",
	      MHAL_RIU_REG(gpio_table[offset].r_out),
	      readb(MHAL_RIU_REG(gpio_table[offset].r_out)), value,
	      gpio_table[offset].m_out);
	if (value)
		writeb(readb(MHAL_RIU_REG(gpio_table[offset].r_out)) |
		       gpio_table[offset].m_out,
		       MHAL_RIU_REG(gpio_table[offset].r_out));
	else
		writeb(readb(MHAL_RIU_REG(gpio_table[offset].r_out)) &
		       ~gpio_table[offset].m_out,
		       MHAL_RIU_REG(gpio_table[offset].r_out));

	return 0;
}

static int mediatek_gpio_get_function(struct udevice *dev, unsigned int offset)
{
	struct mediatek_gpio_priv *priv = dev_get_priv(dev);

	debug("%s, pin %d reg %p value 0x%02X, check mask 0x%02X\n", __func__,
	      offset, MHAL_RIU_REG(gpio_table[offset].r_oen),
	      readb(MHAL_RIU_REG(gpio_table[offset].r_oen)),
	      gpio_table[offset].m_oen);

	if (!test_bit(offset, priv->requested))
		return GPIOF_FUNC;

	if (offset == PAD_IRIN)
		return GPIOF_INPUT;

	if (readb(MHAL_RIU_REG(gpio_table[offset].r_oen)) & gpio_table[offset].m_oen)
		return GPIOF_INPUT;
	else
		return GPIOF_OUTPUT;
}

static const struct dm_gpio_ops gpio_mediatek_ops = {
	.direction_input = mediatek_gpio_direction_input,
	.direction_output = mediatek_gpio_direction_output,
	.get_value = mediatek_gpio_get_value,
	.set_value = mediatek_gpio_set_value,
	.get_function = mediatek_gpio_get_function,
	.request = mediatek_gpio_request,
};

static int gpio_mediatek_probe(struct udevice *dev)
{
	struct mediatek_gpio_priv *priv = dev_get_priv(dev);
	struct gpio_dev_priv *uc_priv = dev_get_uclass_priv(dev);

	debug("%s\n", __func__);
	/* Tell the uclass how many GPIOs we have */
	priv->gpio_count = ARRAY_SIZE(gpio_table);

	priv->bank_name = strdup(dev->name);
	if (!priv->bank_name)
		return -ENOMEM;
	bitmap_zero(priv->requested, priv->gpio_count);

	uc_priv->gpio_count = priv->gpio_count;
	uc_priv->bank_name = priv->bank_name;

	/* disable ALL_PAD_IN */
	writeb(readb(MHAL_RIU_REG(REG_ALL_PAD_IN)) & ~BIT(0),
	       MHAL_RIU_REG(REG_ALL_PAD_IN));

	return 0;
}

static const struct udevice_id mediatek_gpio_ids[] = {
	{.compatible = "mediatek,mtk-dtv-m7642-gpio"},
	{}
};

U_BOOT_DRIVER(gpio_mediatek_dtv_m7642) = {
	.name = "gpio_mediatek",
	.id = UCLASS_GPIO,
	.ops = &gpio_mediatek_ops,
	.of_match = mediatek_gpio_ids,
	.probe = gpio_mediatek_probe,
	.priv_auto_alloc_size = sizeof(struct mediatek_gpio_priv),
};
