// SPDX-License-Identifier: (GPL-2.0 OR BSD-3-Clause)
/*
 * MediaTek Ldm driver
 *
 * Copyright (c) 2020 MediaTek Inc.
 * Author Sniper.Huang <sniper.huang@mediatek.com>
 */
#ifndef _MTK_TV_LDM_H
#define _MTK_TV_LDM_H
#include <asm/gpio.h>

#define FULL_DUTY_12BIT		0xFFF
#define SPI_READ_BUF_NUM	(0x02)
#define HIGH_BYTE_16BIT_BASE	0x100
#define MASK_2BYTES		(0xFFUL)


#define SHIFT_2BYTES	(8UL)

#define MSPI_MAX_NUM (0x08UL)

#define Reg_0x01_to_0x0B_NUM       0x0B
#define Reg_0x0E_to_0x15_NUM       0x08
#define Reg_0x1D_to_0x1F_NUM       0x03
#define Reg_0x01_to_0x15_NUM       0x15
#define Reg_0x00_to_0x1F_NUM       0x20

/* AMS 382X Define START*/
#define LDM_AS382X_CUR_ON_1_ADDR         0x01
#define LDM_AS382X_DELAY1_ADDR           0x16    // Defines the delay time of the PWM, 0x16 ~ 0x35,
#define LDM_AS382X_PWM_HTIME1_ADDR       0x37    // Defines PWM high time, 0x37 ~ 0x56,
#define LDM_AS382X_BDAC1_ADDR        0x70
#define LDM_AS382X_VDAC_LEDH_ADDR        0x0C
#define LDM_AS382X_FB_ON1_ADDR        0x0E


#define LDM_AS382X_STATUS_ADDR        0x60

#define LDM_AS382X_PLLmulti1_ADDR        0x61
#define LDM_AS382X_HDR_ADDR        0x67

#define LDM_AS382X_UNLOCK_ADDR           0x36   // Defines unlock address,
#define LDM_AS382X_UNLOCK_CMD       0xCA
#define LDM_AS382X_LOCK_CMD         0xAC

#define LDM_AS382X_HEADER_NUM	0x03

#define LDM_AS382X_DELAY_NUM	0x20
#define LDM_AS382X_PWM_HTIME_NUM	0x20
#define LDM_AS382X_BDAC_NUM	0x10

#define LDM_AS382X_SINGLE_BYTE_SAME_DATA   0x40
#define LDM_AS382X_BCAST_SINGLE_BYTE_SAME_DATA   0xC0
#define LDM_AS382X_BCAST_MULTI_BYTE_SAME_DATA   0x80
#define LDM_AS382X_BCAST_MULTI_BYTE_DIFF_DATA   0xBF

#define LDM_AS382X_SINGLE_READ_CMD_NUM   (0x03UL)
#define LDM_AS382X_BCAST_SINGLE_WRITE_CMD_NUM   0x03
#define LDM_AS382X_WRITE_SAME_DATA_REG_0x01_0x15_CMD_NUM   0x18
#define LDM_AS382X_WRITE_SAME_DATA_PLL_MULTI_CMD_NUM   0x05
#define LDM_AS382X_WRITE_SAME_DATA_PWM_CMD_NUM   (0x23UL)
#define LDM_AS382X_WRITE_SAME_DATA_BDAC_CMD_NUM	0x13

#define LDM_AS3824_BDAC_Vref_800 80000
#define LDM_AS3824_BDAC_Vref_500 50000

#define LDM_AS3824_TOTAL_CH 0x10

/* AMS 382X Define END*/
#define NT50516_POWER_ON	0xFF

/* IW7039 Define START*/
#define IW7039_SIMPLE_CHKSUM   0
#define IW7039_DEVICE_NUM   16
#define MAX_LDM_SW_SPI_CMD_SIZE 514
//.Set number of device
#define COMMAND_IW7039_SET_NUM_OF_DEVICE 0xFE00
//.Read single device single register
#define COMMAND_IW7039_READ_SINGLE_DEVICE 0x8001
#define ADDRESS_IW7039_READ_SINGLE_DEVICE 0x8000
#define DATA_DUMMY_IW7039_READ_SINGLE_DEVICE 0x0000
//.Write all device same one address
#define COMMAND_IW7039_WRITE_SAME_ONE_WORD_OF_ALL_DEVICE 0x8001
#define ADDRESS_IW7039_WRITE_SAME_ONE_WORD_OF_ALL_DEVICE 0x0000
//.Write all device diff 32 word data
#define COMMAND_IW7039_WRITE_DIFFERNET_DATA_OF_ALL_DEVICE 0xFF20
#define ADDRESS_IW7039_WRITE_DIFFERNET_DATA_OF_ALL_DEVICE 0x0000
//.Write all device same 32 word data
#define COMMAND_IW7039_WRITE_SAME_32_WORD_OF_ALL_DEVICE 0x8020
#define ADDRESS_IW7039_WRITE_SAME_32_WORD_OF_ALL_DEVICE 0x0000
//.Write one device 32 word data
#define COMMAND_IW7039_WRITE_32_WORD_OF_ONE_DEVICE 0x8020
#define ADDRESS_IW7039_WRITE_32_WORD_OF_ONE_DEVICE 0x0000
//.Write one device one address
#define COMMAND_IW7039_WRITE_ONE_WORD_OF_ONE_DEVICE 0x8001
#define ADDRESS_IW7039_WRITE_ONE_WORD_OF_ONE_DEVICE 0x0000
#define IW7039_ISET_ADDR             0x020    // Defines ISET 0x20 ~ 0x3F
#define IW7039_DT_ADDR             0x040
#define IW7039_PWM_HTIME1_ADDR       0x060    // Defines PWM high time, 0x60~ 0x56,

#define IW7039_0x344_ADDR		0x344

#define IW7039_PWM_HTIME_NUM		0x20
#define IW7039_PWM_ISET_NUM		0x20

#define IW7039_DAISY_CHAIN_CHIP_NUM_MAX		0x7F
#define IW7039_SINGLE_DATA_READ_DEVICE_NUM_MAX		0x7D

#define IW7039_INIT_REG0_VALUE		0x802
#define IW7039_FINISH_INIT_REG0_WO_CHK		0x803
#define IW7039_FINISH_INIT_REG0_W_CHK		0x813

#define IW7039_DUMMY_CAL_NUM	(16)//one device one bit
#define IW7039_POWER_ON	0x08

#define IW7039_READ_BUF_NUM	(20)
#define IW7039_CHK_SUM_NUM	(2)

#define IW7039_32WORD		(32)
/* IW7039 Define END*/

/* MBI6353 Define START*/

#define MBI6353_HEADER_NUM		0x03
#define MBI6353_CHECKSUM_NUM		0x01

#define MBI6353_WRITE_SAME_ONE_WORD_OF_ALL_DEVICE_H		0x80
#define MBI6353_WRITE_SAME_ONE_WORD_OF_ALL_DEVICE_L		0x00
#define MBI6353_MASK_ADDR_H		0x04
#define MBI6353_MASK_ADDR_L		0x01
#define MBI6353_CFG_ADDR_H		0x00
#define MBI6353_CFG_ADDR_L		0x00
#define MBI6353_SCAN1_DUTY_H		0x00
#define MBI6353_SCAN1_DUTY_L		0x20

#define MBI6353_MASK_NUM		0x03
#define MBI6353_CFG_NUM		0x10
#define MBI6353_DUTY_NUM		0xC0

#define SLEEP_TIME_1			1
#define SLEEP_TIME_10			10
#define SLEEP_TIME_30			30
#define SLEEP_TIME_100			100
#define SLEEP_TIME_200			200
/* MBI6353 Define END*/

/*MCU Define START*/
#define MCU_READ_BUF_NUM	(20)
#define MCU_READ_BUF_NUM1	(10)

#define MCU_1BYTE	(9)
#define MCU_2BYTE	(16)

/*MCU Define END*/

#define BYTE1 	1
#define BYTE2 	2

#define LD_MAX_HEIGHT (128)
#define LD_MAX_WIDTH (128)
#define LDF_MAX_WIDTH 128
#define LDF_MAX_HEIGHT (128)
#define LDB_MAX_WIDTH (128)
#define LDB_MAX_HEIGHT (128)

#define MAX_LANES	(64)
#define MPLL_CLK	(864)
#define LPLL_SPAN_FACTOR	(131072)
#define LPLL_CAL_PARAM	(16)
#define MAX_LDM_SPI_BYTES	(500UL)
#define IS_OUT_8K4K(width,height) (((width <= 8000)&&(width >= 7500))&&((height <= 4500)&&(height >= 4000)))
#define IS_OUT_4K2K(width,height) (((width <= 4000)&&(width >= 3750))&&((height <= 2250)&&(height >= 2000)))
#define IS_OUT_2K1K(width,height) (((width <= 2000)&&(width >= 1875))&&((height <= 1125)&&(height >= 1000)))
#define IS_VFREQ_60HZ_GROUP(vfreq) ((vfreq < 65) && (vfreq > 45))
#define IS_VFREQ_120HZ_GROUP(vfreq) ((vfreq < 125) && (vfreq > 90))
#define IS_VB1_OUT(x) ((x == E_LINK_VB1) || (x == E_LINK_VB1_TO_HDMITX))
#define LDM_TAG "LDM_SUPPORT"
#define LDM_VERSION "MX_LDM_VERSION"
#define LDM_MAKER "LDM_CUS_IP_PATH_uBoot"
#define LED_MSPI_EN "LED_MSPI_EN"

#define LDM_DMA_INFO "ldm-dma-info"
#define LDM_VSYNC_WIDTH "VsyncWidth"
#define LDM_VSYNC_WIDTH_SIZE (4)
#define LDM_PWM0_PERIOD "PWM0_Period"
#define LDM_PWM0_PERIOD_SIZE (2)
#define LDM_PWM0_DUTY "PWM0_Duty"
#define LDM_PWM0_DUTY_SIZE (2)
#define LDM_PWM0_SHIFT "PWM0_Shift"
#define LDM_PWM0_SHIFT_SIZE (2)
#define LDM_PWM1_PERIOD "PWM1_Period"
#define LDM_PWM1_PERIOD_SIZE (2)
#define LDM_PWM1_DUTY "PWM1_Duty"
#define LDM_PWM1_DUTY_SIZE (2)
#define LDM_PWM1_SHIFT "PWM1_Shift"
#define LDM_PWM1_SHIFT_SIZE (2)

#define LDM_LED_MSPI_INFO "ldm-mspi-info"
#define LDM_MSPIMODE "MspiMode"
#define LDM_MSPICLK "MspiClk"

#define LDM_LED_DEVICE_INFO "ldm-led-device-info"
#define LDM_LEDDEVICE_TYPE "LedDevice_Type"
#define LDM_DEVICE_NUM "Device_Num"

#define LDM_DEVICE_AS3824_INFO "ldm-led-device-as3824-info"
#define LDM_DEVICE_AS3824_RSENSE "Rsense"
#define LDM_DEVICE_AS3824_REG_01_TO_0B "AS3824_Reg_0x01_to_0x0B"
#define LDM_DEVICE_AS3824_REG_0E_TO_15 "AS3824_Reg_0x0E_to_0x15"
#define LDM_DEVICE_AS3824_REG_67 "AS3824_Reg_0x67"
#define LDM_DEVICE_AS3824_PWM_DUTY_INIT "AS3824_PWM_Duty_Init"
#define LDM_DEVICE_AS3824_PWM_PHASE_MBR_OFF "AS3824_PWM_Phase_MBR_OFF"
#define LDM_DEVICE_AS3824_PWM_PHASE_MBR_ON "AS3824_PWM_Phase_MBR_ON"
#define LDM_DEVICE_AS3824_REG_0C_0D_VDAC_MBR_OFF "AS3824_Reg_0x0C_0x0D_VDAC_MBR_OFF"
#define LDM_DEVICE_AS3824_REG_0C_0D_VDAC_MBR_ON "AS3824_Reg_0x0C_0x0D_VDAC_MBR_ON"
#define LDM_DEVICE_AS3824_REG_61_62_PLL_MULTI "AS3824_Reg_0x61_0x62_PLL_multi"
#define LDM_DEVICE_AS3824_BDAC_HIGH_LIMIT "AS3824_BDAC_High_Limit"
#define LDM_DEVICE_AS3824_BDAC_LOW_LIMIT "AS3824_BDAC_Low_Limit"

#define LDM_DEVICE_NT50585_INFO "ldm-led-device-nt50585-info"
#define LDM_DEVICE_NT50585_REG_01_TO_0B "NT50585_Reg_0x01_to_0x0B"
#define LDM_DEVICE_NT50585_REG_1D_TO_1F "NT50585_Reg_0x1D_to_0x1F"
#define LDM_DEVICE_NT50585_REG_60 "NT50585_Reg_0x60"
#define LDM_DEVICE_NT50585_REG_68 "NT50585_Reg_0x68"
#define LDM_DEVICE_NT50585_PWM_DUTY_INIT "NT50585_PWM_Duty_Init"
#define LDM_DEVICE_NT50585_PWM_PHASE_MBR_OFF "NT50585_PWM_Phase_MBR_OFF"
#define LDM_DEVICE_NT50585_PWM_PHASE_MBR_ON "NT50585_PWM_Phase_MBR_ON"
#define LDM_DEVICE_NT50585_REG_14_15_IDAC_MBR_OFF "NT50585_Reg_0x14_0x15_IDAC_MBR_OFF"
#define LDM_DEVICE_NT50585_REG_14_15_IDAC_MBR_ON "NT50585_Reg_0x14_0x15_IDAC_MBR_ON"
#define LDM_DEVICE_NT50585_REG_66_67_PLL_MULTI "NT50585_Reg_0x66_0x67_PLL_multi"
#define LDM_DEVICE_NT50585_BDAC_HIGH_LIMIT "NT50585_BDAC_High_Limit"
#define LDM_DEVICE_NT50585_BDAC_LOW_LIMIT "NT50585_BDAC_Low_Limit"

#define LDM_DEVICE_IW7039_INFO "ldm-led-device-IW7039-info"
#define LDM_DEVICE_IW7039_REG_00_TO_1F "IW7039_Reg_0x000_to_0x01F"
#define LDM_DEVICE_IW7039_PWM_DUTY_INIT "IW7039_PWM_Duty_Init"
#define LDM_DEVICE_IW7039_ISET_MBR_OFF "IW7039_ISET_MBR_OFF"
#define LDM_DEVICE_IW7039_ISET_MBR_ON "IW7039_ISET_MBR_ON"
#define LDM_DEVICE_IW7039_PWM_PHASE_MBR_OFF "IW7039_PWM_Phase_MBR_OFF"
#define LDM_DEVICE_IW7039_PWM_PHASE_MBR_ON "IW7039_PWM_Phase_MBR_ON"
#define LDM_DEVICE_NT50585_ISET_HIGH_LIMIT "IW7039_ISET_High_Limit"
#define LDM_DEVICE_NT50585_ISET_LOW_LIMIT "IW7039_ISET_Low_Limit"

#define LDM_DEVICE_MBI6353_INFO "ldm-led-device-MBI6353-info"
#define LDM_DEVICE_MBI6353_MASK "MBI6353_MASK"
#define LDM_DEVICE_MBI6353_CFG "MBI6353_CFG"
#define LDM_DEVICE_MBI6353_CFG_1 "MBI6353_CFG_1"
#define LDM_DEVICE_MBI6353_PWM_Duty_Init "MBI6353_PWM_Duty_Init"

#define Reg_0x01_to_0x0B_NUM       0x0B
#define Reg_0x1D_to_0x1F_NUM       0x03
#define Reg_0x0E_to_0x15_NUM       0x08
#define Reg_0x00_to_0x1F_NUM       0x20
#define VDAC_BYTE_NUM        2
#define PLL_MULTI_BYTE_NUM       2

#define LDM_NUM_1	(1UL)
#define LDM_NUM_2	(2UL)
#define LDM_NUM_4	(4UL)
#define LDM_NUM_6	(6UL)
#define LDM_NUM_10	(10UL)
#define LDM_NUM_16	(16UL)
#define LDM_NUM_256	(256UL)

enum EN_LDM_SUPPORT {
	E_LDM_UNSUPPORT = 0,		//no ldm
	E_LDM_SUPPORT_TRUNK_PQU,	//trunk ld in PQU
	E_LDM_SUPPORT_CUS,			//cus ld
	E_LDM_SUPPORT_TRUNK_FRC,	//trunk ld in FRC
	E_LDM_SUPPORT_BE,			// use BE
	E_LDM_SUPPORT_TRUNK_R2		//trunk ld in R2
};

enum EN_M_LDM_VERSION {
	E_M_LDM_VERSION_1 = 1,
	E_M_LDM_VERSION_2,
	E_M_LDM_VERSION_3,
	E_M_LDM_VERSION_4,
	E_M_LDM_VERSION_5
};

typedef struct
{
	uint16_t u16Rsense;
	uint32_t u32AS3824_Reg_0x01_to_0x0B[Reg_0x01_to_0x0B_NUM];
	uint32_t u32AS3824_Reg_0x0E_to_0x15[Reg_0x0E_to_0x15_NUM];
	uint8_t u8AS3824_Reg_0x67;
	uint16_t u16AS3824_PWM_Duty_Init;
	uint32_t u32AS3824_PWM_Phase_MBR_OFF[LD_MAX_HEIGHT];
	uint32_t u32AS3824_PWM_Phase_MBR_ON[LD_MAX_HEIGHT];
	uint32_t u32AS3824_Reg_0x0C_0x0D_VDAC_MBR_OFF[VDAC_BYTE_NUM];
	uint32_t u32AS3824_Reg_0x0C_0x0D_VDAC_MBR_ON[VDAC_BYTE_NUM];
	uint32_t u32AS3824_Reg_0x61_0x62_PLL_multi[PLL_MULTI_BYTE_NUM];
	uint8_t u8AS3824_BDAC_High_Limit;
	uint8_t u8AS3824_BDAC_Low_Limit;
}mtk_ldm_Device_AS3824;

typedef struct
{
	uint32_t u32NT50585_Reg_0x01_to_0x0B[Reg_0x01_to_0x0B_NUM];
	uint32_t u32NT50585_Reg_0x1D_to_0x1F[Reg_0x1D_to_0x1F_NUM];
	uint8_t u8NT50585_Reg_0x60;
	uint8_t u8NT50585_Reg_0x68;
	uint16_t u16NT50585_PWM_Duty_Init;
	uint32_t u32NT50585_PWM_Phase_MBR_OFF[LD_MAX_HEIGHT];
	uint32_t u32NT50585_PWM_Phase_MBR_ON[LD_MAX_HEIGHT];
	uint32_t u32NT50585_Reg_0x14_0x15_IDAC_MBR_OFF[VDAC_BYTE_NUM];
	uint32_t u32NT50585_Reg_0x14_0x15_IDAC_MBR_ON[VDAC_BYTE_NUM];
	uint32_t u32NT50585_Reg_0x66_0x67_PLL_multi[PLL_MULTI_BYTE_NUM];
	uint16_t u16NT50585_BDAC_High_Limit;
	uint16_t u16NT50585_BDAC_Low_Limit;
}mtk_ldm_Device_NT50585;

typedef struct
{
	uint32_t u32IW7039_Reg_0x000_to_0x01F[Reg_0x00_to_0x1F_NUM];
	uint16_t u16IW7039_PWM_Duty_Init;
	uint16_t u16IW7039_ISET_MBR_OFF;
	uint16_t u16IW7039_ISET_MBR_ON;
	uint32_t u32IW7039_PWM_Phase_MBR_OFF[LD_MAX_HEIGHT];
	uint32_t u32IW7039_PWM_Phase_MBR_ON[LD_MAX_HEIGHT];
	uint16_t u16IW7039_ISET_High_Limit;
	uint16_t u16IW7039_ISET_Low_Limit;
}mtk_ldm_Device_IW7039;

typedef struct
{
	uint32_t u32MBI6353_Mask[MBI6353_MASK_NUM * 2];
	uint32_t u32MBI6353_Config[MBI6353_CFG_NUM * 2];
	uint32_t u32MBI6353_Config_1[MBI6353_CFG_NUM * 2];
	uint16_t u16MBI6353_PWM_Duty_Init;
}mtk_ldm_Device_mbi6353;

typedef struct
{
	u8 u8mspi_mode;
	u32 u32mspi_clk;
}mtk_ldm_mspi_info;

typedef struct
{
	u32  au32ld_vsync_width[4];
	u32  au32ld_pwm0_period[2];
	u32  au32ld_pwm0_duty[2];
	u32  au32ld_pwm0_shift[2];
	u32  au32ld_pwm1_period[2];
	u32  au32ld_pwm1_duty[2];
	u32  au32ld_pwm1_shift[2];
}mtk_ldm_dma_info;

typedef struct
{
	uint8_t  u8LedDevice_Type;
	u32 u32device_num[MSPI_MAX_NUM];
}mtk_ldm_device_info;

struct mtk_ldm_priv {
	uint32_t u32LDMSupport;
	u8 u8ldm_version;
	bool bLDMCusPathuBoot;
	bool bled_mspi_en;

	mtk_ldm_mspi_info st_mtk_ldm_mspi_info;
	mtk_ldm_dma_info st_mtk_ldm_dma_info;
	mtk_ldm_device_info st_mtk_ldm_device_info;
	
	mtk_ldm_Device_AS3824 st_ldm_Device_AS3824;
	mtk_ldm_Device_NT50585 st_ldm_Device_NT50585;
	mtk_ldm_Device_IW7039 st_ldm_Device_IW7039;
	mtk_ldm_Device_mbi6353 st_ldm_Device_MBI6353;
};

struct mtk_ldm_spi_data {
	u16	u16len;
	uint8_t		au8Data[MAX_LDM_SPI_BYTES];
};

struct mtk_ldm_spi {
	u32	u32ch;
	u32	u32mode;
	u32	u32speed;
	u32	u32out_bit_len;
	u8		u8out[MAX_LDM_SPI_BYTES];
	u32	u32in_bit_len;
	u8		u8in[MAX_LDM_SPI_BYTES];
};

enum EN_LD_DEVICE_TYPE {
	E_LD_DEVICE_UNSUPPORT = 0,
	E_LD_DEVICE_AS3824 = 1,
	E_LD_DEVICE_NT50585,
	E_LD_DEVICE_IW7039,
	E_LD_DEVICE_MCU,
	E_LD_DEVICE_CUS,
	E_LD_DEVICE_MBI6353,
};

#endif
