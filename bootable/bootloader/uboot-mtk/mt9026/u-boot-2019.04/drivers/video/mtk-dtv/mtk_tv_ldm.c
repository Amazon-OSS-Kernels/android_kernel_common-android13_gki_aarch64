// SPDX-License-Identifier: (GPL-2.0 OR BSD-3-Clause)
/*
 * MediaTek Ldm driver
 *
 * Copyright (c) 2020 MediaTek Inc.
 * Author Sniper.Huang <sniper.huang@mediatek.com>
 */
//#define DEBUG //for open this file debug("xxx") log
#include <linux/delay.h>
#include "mtk_tv_ldm.h"
//#include "mtk_pnl_dts_st.h"
//#include "mtk_pnl_out_if.h"
//#include "mtk_pnl_clk_ctrl.h"
#include "mtk_pnl_utility.h"
#include <common.h>
#include <backlight.h>
#include <dm.h>
#include <panel.h>
#include <asm/gpio.h>
#include <power/regulator.h>
#include <display.h>
#include <dm/pinctrl.h>
#include <debug_impl.h>
#include "coda/LDC_BKA3EF.h"
#include "coda/LD1_BKA417.h"
#include <asm/io.h>
#include <spi.h>
#define SPI_XFER_BEGIN		BIT(0)	/* Assert CS before transfer */
#define SPI_XFER_END		BIT(1)	/* Deassert CS after transfer */
#define LDM_CHAR_NAME		(30)
#define BIT0			(0x0001)
#define LDM_3BITS		(3UL)
#define LDM_6BITS		(6UL)
#define LDM_7BITS		(7UL)
#define LDM_8BITS		(8UL)
#define LDM_9BITS		(9UL)
#define LDM_10BITS		(10UL)
#define LDM_CH_0		(0UL)
#define LDM_CH_1		(1UL)
#define LDM_CH_2		(2UL)
#define LDM_CH_3		(3UL)
#define LDM_CH_4		(4UL)
#define LDM_CH_5		(5UL)
#define LDM_CH_6		(6UL)
#define LDM_CH_7		(7UL)
#define LDM_CH_MAX		(8UL)
#define LDM_MASK		(0xFF)
#define LDM_BYTES_0		(0)
#define LDM_BYTES_1		(1)
#define LDM_BYTES_2		(2)
#define LDM_BYTES_3		(3)
#define LDM_BYTES_4		(4)
#define LDM_BYTES_5		(5)
#define LDM_BYTES_6		(6)
#define LDM_BYTES_7		(7)
#define LDM_BYTES_8		(8)
#define LDM_SPI_CLK		(1000000)
#define LDM_SPI_MODE	(0)
#define LDM_SPI_ADDR	(0x640800)//(0x1C640800)
#define LDM_SPI_OFFSET	(0x200)
#define LDM_SPI_REG_080	(0x080)
#define LDM_SPI_REG_084	(0x084)
#define LDM_SPI_REG_088	(0x088)
#define LDM_SPI_REG_08C	(0x08C)
#define LDM_SPI_REG_090	(0x090)
#define LDM_SPI_REG_094	(0x094)
#define LDM_SPI_REG_0A0	(0x0A0)
#define LDM_SPI_REG_0A4	(0x0A4)
#define LDM_SPI_REG_0A8	(0x0A8)
#define LDM_SPI_REG_0AC	(0x0AC)
#define LDM_SPI_REG_0B0	(0x0B0)
#define LDM_SPI_REG_0B4	(0x0B4)
#define LDM_SPI_REG_0E0	(0x0E0)
#define LDM_SPI_REG_130	(0x130)
#define LDM_SPI_REG_134	(0x134)
#define LDM_SPI_REG_138	(0x138)
#define LDM_SPI_REG_13C	(0x13C)

#define LDM_PWM_ADDR	(0x01483A00)
#define LDM_PWM_REG_004	(LDM_PWM_ADDR + 0x004)
#define LDM_PWM_REG_004_CLKEN_BITMAP 			(((uint32_t)7<<16)|(0<<8)|9)

#define LDM_PWM_REG_008	(LDM_PWM_ADDR + 0x008)
#define LDM_PWM_REG_0008_PWM0_VDBEN 	(((uint32_t)1<<16)|(9<<8)|10)
#define LDM_PWM_REG_0008_PWM0_RESET_EN 	(((uint32_t)1<<16)|(10<<8)|10)
#define LDM_PWM_REG_0008_PWM0_VDBEN_SW 	(((uint32_t)1<<16)|(14<<8)|10)

#define LDM_PWM_REG_00C	(LDM_PWM_ADDR + 0x00C)
#define LDM_PWM_REG_000C_PWM1_VDBEN 	(((uint32_t)1<<16)|(9<<8)|10)
#define LDM_PWM_REG_000C_PWM1_RESET_EN 	(((uint32_t)1<<16)|(10<<8)|10)
#define LDM_PWM_REG_000C_PWM1_VDBEN_SW 	(((uint32_t)1<<16)|(14<<8)|10)

#define LDM_PWM_REG_0A0 (LDM_PWM_ADDR + 0x0A0)
#define LDM_PWM_REG_0A0_VSYNC_LIKE_ST_0	(((uint32_t)16<<16)|(0<<8)|5)
#define LDM_PWM_REG_0A4 (LDM_PWM_ADDR + 0x0A4)
#define LDM_PWM_REG_0A4_VSYNC_LIKE_ST_1	(((uint32_t)16<<16)|(0<<8)|5)
#define LDM_PWM_REG_0A8 (LDM_PWM_ADDR + 0x0A8)
#define LDM_PWM_REG_0A8_VSYNC_LIKE_END_0	(((uint32_t)16<<16)|(0<<8)|5)
#define LDM_PWM_REG_0AC (LDM_PWM_ADDR + 0x0AC)
#define LDM_PWM_REG_0AC_VSYNC_LIKE_END_1	(((uint32_t)16<<16)|(0<<8)|5)
#define LDM_PWM_REG_0B0	(LDM_PWM_ADDR + 0x0B0)
#define LDM_PWM_REG_0B0_VSYNC_LIKE_SRC_SEL		(((uint32_t)1<<16)|(0<<8)|9)
#define LDM_PWM_REG_0B0_VSYNC_LIKE_SRC_SEL_V2	(((uint32_t)2<<16)|(0<<8)|9)

#define LDM_CLKEN_BITMAP		(0x07)

#define LDM_PWM_REG_1C8 (LDM_PWM_ADDR + 0x1C8)
#define LDM_PWM_REG_1C8_PWM0_PERIOD_0 	(((uint32_t)16<<16)|(0<<8)|5)
#define LDM_PWM_REG_1CC (LDM_PWM_ADDR + 0x1CC)
#define LDM_PWM_REG_1CC_PWM0_PERIOD_1 	(((uint32_t)8<<16)|(0<<8)|1)
#define LDM_PWM_REG_1D0 (LDM_PWM_ADDR + 0x1D0)
#define LDM_PWM_REG_1D0_PWM0_SHIFT_0 	(((uint32_t)16<<16)|(0<<8)|5)
#define LDM_PWM_REG_1D4 (LDM_PWM_ADDR + 0x1D4)
#define LDM_PWM_REG_1D4_PWM0_SHIFT_1 	(((uint32_t)8<<16)|(0<<8)|1)
#define LDM_PWM_REG_1D8 (LDM_PWM_ADDR + 0x1D8)
#define LDM_PWM_REG_1D8_PWM0_DUTY_0 	(((uint32_t)16<<16)|(0<<8)|5)
#define LDM_PWM_REG_1DC (LDM_PWM_ADDR + 0x1DC)
#define LDM_PWM_REG_1DC_PWM0_DUTY_1 	(((uint32_t)8<<16)|(0<<8)|1)

#define LDM_PWM_REG_1E8 (LDM_PWM_ADDR + 0x1E8)
#define LDM_PWM_REG_1E8_PWM1_PERIOD_0 	(((uint32_t)16<<16)|(0<<8)|5)
#define LDM_PWM_REG_1EC (LDM_PWM_ADDR + 0x1EC)
#define LDM_PWM_REG_1EC_PWM1_PERIOD_1 	(((uint32_t)8<<16)|(0<<8)|1)
#define LDM_PWM_REG_1F0 (LDM_PWM_ADDR + 0x1F0)
#define LDM_PWM_REG_1F0_PWM1_SHIFT_0 	(((uint32_t)16<<16)|(0<<8)|5)
#define LDM_PWM_REG_1F4 (LDM_PWM_ADDR + 0x1F4)
#define LDM_PWM_REG_1F4_PWM1_SHIFT_1 	(((uint32_t)8<<16)|(0<<8)|1)
#define LDM_PWM_REG_1F8 (LDM_PWM_ADDR + 0x1F8)
#define LDM_PWM_REG_1F8_PWM1_DUTY_0 	(((uint32_t)16<<16)|(0<<8)|5)
#define LDM_PWM_REG_1FC (LDM_PWM_ADDR + 0x1FC)
#define LDM_PWM_REG_1FC_PWM1_DUTY_1 	(((uint32_t)8<<16)|(0<<8)|1)

#define LDM_SPI_MASK    (((uint32_t)16<<16)|(0<<8)|5)
#define LDM_SPI_MASK_0  (((uint32_t)1<<16)|(0<<8)|9)
#define msleep(a) udelay(a * 1000) //macro

#define LDM_VER_1	(1)
#define LDM_VER_2	(2)
#define LDM_VER_3	(3)
#define LDM_VER_4	(4)
#define LDM_VER_5	(5)

#define RETRY_COUNTER		(200UL)

//static uint32_t au32LDMLedMSP8Bit[LDM_BYTES_8] =  {LDM_BYTES_7, LDM_BYTES_7, LDM_BYTES_7, LDM_BYTES_7, LDM_BYTES_7, LDM_BYTES_7, LDM_BYTES_7, LDM_BYTES_7};
//static uint32_t au32LDMLedMSP10Bit[LDM_BYTES_8] = {LDM_BYTES_1, LDM_BYTES_7, LDM_BYTES_1, LDM_BYTES_7, LDM_BYTES_1, LDM_BYTES_7, LDM_BYTES_1, LDM_BYTES_7};

static uint8_t LED_Device_AS3824_lock[]=	//Lock
{
	LDM_AS382X_BCAST_SINGLE_BYTE_SAME_DATA, LDM_AS382X_UNLOCK_ADDR, LDM_AS382X_LOCK_CMD,

	0x00,0x00,0x00,0x00,0x00, //Dummy*25
	0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,
};

static uint8_t LED_Device_AS3824_unlock[]=
{
	LDM_AS382X_BCAST_SINGLE_BYTE_SAME_DATA, LDM_AS382X_UNLOCK_ADDR, LDM_AS382X_UNLOCK_CMD,

	0x00,0x00,0x00,0x00,0x00, //Dummy*25
	0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,
};

/*
 *static int _vsync_like_gpio_set_state(struct udevice *dev, enum VSYNC_LIKE_GPIO state)
 *{
 *	struct mtk_ldm_priv *priv = dev_get_priv(dev);
 *	int ret;
 *
 *	if (!dm_gpio_is_valid(&priv->vsync_like_gpio))
 *		return -EREMOTEIO;
 *
 *	switch (state) {
 *		case E_LOW:
 *		case E_HIGH:
 *			break;
 *		case E_TOGGLE:
 *			ret = dm_gpio_get_value(&priv->vsync_like_gpio);
 *			if (ret < 0)
 *				return ret;
 *			state = !ret;
 *			break;
 *		default:
 *			return -ENOSYS;
 *			}
 *	return dm_gpio_set_value(&priv->vsync_like_gpio, state);
 *}
 */
static int _mtk_ldm_spi(struct mtk_ldm_spi *st_spi)
{
	struct spi_slave *slave;
	int ret = 0;
	unsigned long flags = SPI_XFER_BEGIN;
#ifdef CONFIG_DM_SPI
	char name[LDM_CHAR_NAME], *str;
	struct udevice *dev;
	ret = snprintf(name, sizeof(name), "generic_%d:%d", st_spi->u32ch, 0);
	if (ret < 0) {
		return ret;
	}
	str = strdup(name);
	if (!str) {
		return -ENOMEM;
	}
	ret = spi_get_bus_and_cs(st_spi->u32ch, 0, st_spi->u32speed, st_spi->u32mode, "spi_generic_drv", str, &dev, &slave);
	if (ret) {
		return ret;
	}
#else
	slave = spi_setup_slave(st_spi->u32ch, 0, st_spi->u32speed, st_spi->u32mode);
	if (!slave) {
		UBOOT_DEBUG("Invalid device %d:%d\n", st_spi->u32ch, 0);
		return -EINVAL;
	}
#endif
	ret = spi_claim_bus(slave);
	if (ret) {
		goto done;
	}
	if (st_spi->u32in_bit_len == 0)
		flags |= SPI_XFER_END;
	ret = spi_xfer(slave, st_spi->u32out_bit_len, &st_spi->u8out, NULL, flags);
	if (ret) {
		UBOOT_DEBUG("spi_xfer tx fail\n");
	} else if (st_spi->u32in_bit_len != 0) {
		ret = spi_xfer(slave, st_spi->u32in_bit_len, NULL, st_spi->u8in, SPI_XFER_END);
		if (ret) {
			UBOOT_DEBUG("spi_xfer rx fail\n");
		}
	}
#ifndef CONFIG_DM_SPI
	/* We don't get an error code in this case */
	if (ret) {
		ret = -EIO;
	}
#endif
	if (ret) {
		UBOOT_DEBUG("Error %d during SPI transaction\n", ret);
	} else {
		//int j;
		//for (j = 0; j < ((stSPI->u32in_bit_len + LDM_7BITS) / LDM_8BITS); j++) {
			//printf("%02X", stSPI->u8in[j]);
		//}
		//printf("\n");
	}
done:
	spi_release_bus(slave);
#ifndef CONFIG_DM_SPI
	spi_free_slave(slave);
#endif
	return ret;
}
/*
static void _mtk_ldm_set_spi_wd(uint8_t u8CH, uint32_t *pu32Data)
{
	uint16_t u16Data_1,u16Data_2;
	uint32_t u32Addr0 = LDM_SPI_ADDR + (LDM_SPI_OFFSET * u8CH) + LDM_SPI_REG_130;
	uint32_t u32Addr1 = LDM_SPI_ADDR + (LDM_SPI_OFFSET * u8CH) + LDM_SPI_REG_134;
	uint32_t u32Addr2 = LDM_SPI_ADDR + (LDM_SPI_OFFSET * u8CH) + LDM_SPI_REG_080;
	uint32_t u32Addr3 = LDM_SPI_ADDR + (LDM_SPI_OFFSET * u8CH) + LDM_SPI_REG_084;
	uint32_t u32Addr4 = LDM_SPI_ADDR + (LDM_SPI_OFFSET * u8CH) + LDM_SPI_REG_088;
	uint32_t u32Addr5 = LDM_SPI_ADDR + (LDM_SPI_OFFSET * u8CH) + LDM_SPI_REG_08C;
	uint32_t u32Addr6 = LDM_SPI_ADDR + (LDM_SPI_OFFSET * u8CH) + LDM_SPI_REG_090;
	uint32_t u32Addr7 = LDM_SPI_ADDR + (LDM_SPI_OFFSET * u8CH) + LDM_SPI_REG_094;

	u16Data_1 = (pu32Data[LDM_BYTES_3] << LDM_9BITS) | (pu32Data[LDM_BYTES_2] << LDM_6BITS) | (pu32Data[LDM_BYTES_1] << LDM_3BITS) | (pu32Data[LDM_BYTES_0]);
	u16Data_2 = (pu32Data[LDM_BYTES_7] << LDM_9BITS) | (pu32Data[LDM_BYTES_6] << LDM_6BITS) | (pu32Data[LDM_BYTES_5] << LDM_3BITS) | (pu32Data[LDM_BYTES_4]);

	W2BYTEMSK(u32Addr0, u16Data_1, LDM_SPI_MASK);
	W2BYTEMSK(u32Addr1, u16Data_2, LDM_SPI_MASK);
	W2BYTEMSK(u32Addr2, u16Data_1, LDM_SPI_MASK);
	W2BYTEMSK(u32Addr3, u16Data_2, LDM_SPI_MASK);
	W2BYTEMSK(u32Addr4, u16Data_1, LDM_SPI_MASK);
	W2BYTEMSK(u32Addr5, u16Data_2, LDM_SPI_MASK);
	W2BYTEMSK(u32Addr6, u16Data_1, LDM_SPI_MASK);
	W2BYTEMSK(u32Addr7, u16Data_2, LDM_SPI_MASK);
}

static void _mtk_ldm_set_spi_rd(uint8_t u8CH, uint32_t *pu32Data)
{
	uint16_t u16Data_1,u16Data_2;
	uint32_t u32Addr0 = LDM_SPI_ADDR + (LDM_SPI_OFFSET * u8CH) + LDM_SPI_REG_138;
	uint32_t u32Addr1 = LDM_SPI_ADDR + (LDM_SPI_OFFSET * u8CH) + LDM_SPI_REG_13C;
	uint32_t u32Addr2 = LDM_SPI_ADDR + (LDM_SPI_OFFSET * u8CH) + LDM_SPI_REG_0A0;
	uint32_t u32Addr3 = LDM_SPI_ADDR + (LDM_SPI_OFFSET * u8CH) + LDM_SPI_REG_0A4;
	uint32_t u32Addr4 = LDM_SPI_ADDR + (LDM_SPI_OFFSET * u8CH) + LDM_SPI_REG_0A8;
	uint32_t u32Addr5 = LDM_SPI_ADDR + (LDM_SPI_OFFSET * u8CH) + LDM_SPI_REG_0AC;
	uint32_t u32Addr6 = LDM_SPI_ADDR + (LDM_SPI_OFFSET * u8CH) + LDM_SPI_REG_0B0;
	uint32_t u32Addr7 = LDM_SPI_ADDR + (LDM_SPI_OFFSET * u8CH) + LDM_SPI_REG_0B4;

	u16Data_1 = (pu32Data[LDM_BYTES_3] << LDM_9BITS) | (pu32Data[LDM_BYTES_2] << LDM_6BITS) | (pu32Data[LDM_BYTES_1] << LDM_3BITS) | (pu32Data[LDM_BYTES_0]);
	u16Data_2 = (pu32Data[LDM_BYTES_7] << LDM_9BITS) | (pu32Data[LDM_BYTES_6] << LDM_6BITS) | (pu32Data[LDM_BYTES_5] << LDM_3BITS) | (pu32Data[LDM_BYTES_4]);

	W2BYTEMSK(u32Addr0, u16Data_1, LDM_SPI_MASK);
	W2BYTEMSK(u32Addr1, u16Data_2, LDM_SPI_MASK);
	W2BYTEMSK(u32Addr2, u16Data_1, LDM_SPI_MASK);
	W2BYTEMSK(u32Addr3, u16Data_2, LDM_SPI_MASK);
	W2BYTEMSK(u32Addr4, u16Data_1, LDM_SPI_MASK);
	W2BYTEMSK(u32Addr5, u16Data_2, LDM_SPI_MASK);
	W2BYTEMSK(u32Addr6, u16Data_1, LDM_SPI_MASK);
	W2BYTEMSK(u32Addr7, u16Data_2, LDM_SPI_MASK);
}
*/
static void _mtk_ldm_set_spi_Init(struct mtk_ldm_priv *priv, uint8_t u8CH)
{
	struct mtk_ldm_spi stMSPI;
	stMSPI.u32ch = u8CH;
	stMSPI.u32mode = LDM_SPI_MODE;
	stMSPI.u32speed = LDM_SPI_CLK;
	stMSPI.u32out_bit_len = 0;
	stMSPI.u32in_bit_len = 0;
	_mtk_ldm_spi(&stMSPI);
	//_mtk_ldm_set_spi_wd(u8CH, au32LDMLedMSP8Bit);
	//_mtk_ldm_set_spi_rd(u8CH, au32LDMLedMSP8Bit);
}

static int _mtk_ldm_spi_data(struct mtk_ldm_priv *priv,
	u8 u8CH, struct mtk_ldm_spi_data *pwdata, struct mtk_ldm_spi_data *prdata)
{
	struct mtk_ldm_spi stMSPI;
	int32_t ret = 0;
	uint16_t i;
	memset((void *)&stMSPI, 0, sizeof(struct mtk_ldm_spi));
	stMSPI.u32ch = u8CH;
	stMSPI.u32mode = priv->st_mtk_ldm_mspi_info.u8mspi_mode;
	stMSPI.u32speed = priv->st_mtk_ldm_mspi_info.u32mspi_clk;
	stMSPI.u32out_bit_len = pwdata->u16len;
	stMSPI.u32in_bit_len = prdata->u16len;

	for (i = 0; i < pwdata->u16len; i++)
		stMSPI.u8out[i] = pwdata->au8Data[i];

	stMSPI.u32out_bit_len = (pwdata->u16len * LDM_8BITS);
	stMSPI.u32in_bit_len = (prdata->u16len * LDM_8BITS);
	ret = _mtk_ldm_spi(&stMSPI);

	for (i = 0; i < prdata->u16len; i++)
		prdata->au8Data[i] = stMSPI.u8in[i];

	return ret;
}

static void _mtk_ldm_set_pwm_sel(struct mtk_ldm_priv *priv)
{
	if (priv->u8ldm_version == LDM_VER_1)
		W2BYTEMSK(LDM_PWM_REG_0B0, 0x01, LDM_PWM_REG_0B0_VSYNC_LIKE_SRC_SEL);
	else
		W2BYTEMSK(LDM_PWM_REG_0B0, 0x01, LDM_PWM_REG_0B0_VSYNC_LIKE_SRC_SEL_V2);
}

static void _mtk_ldm_set_pwm_clken_bitmap(void)
{
	W2BYTEMSK(LDM_PWM_REG_004, LDM_CLKEN_BITMAP, LDM_PWM_REG_004_CLKEN_BITMAP);
}

static void _mtk_ldm_set_pwm_vsync_like(struct mtk_ldm_priv *priv)
{
	W2BYTEMSK(LDM_PWM_REG_0A4,
		  (uint16_t)priv->st_mtk_ldm_dma_info.au32ld_vsync_width[LDM_BYTES_1],
		  LDM_PWM_REG_0A4_VSYNC_LIKE_ST_1);
	W2BYTEMSK(LDM_PWM_REG_0A0,
		  (uint16_t)priv->st_mtk_ldm_dma_info.au32ld_vsync_width[LDM_BYTES_0],
		  LDM_PWM_REG_0A0_VSYNC_LIKE_ST_0);
	W2BYTEMSK(LDM_PWM_REG_0AC,
		  (uint16_t)priv->st_mtk_ldm_dma_info.au32ld_vsync_width[LDM_BYTES_3],
		  LDM_PWM_REG_0AC_VSYNC_LIKE_END_1);
	W2BYTEMSK(LDM_PWM_REG_0A8,
		  (uint16_t)priv->st_mtk_ldm_dma_info.au32ld_vsync_width[LDM_BYTES_2],
		  LDM_PWM_REG_0A8_VSYNC_LIKE_END_0);
}

static void _mtk_ldm_set_pwm0_period(struct mtk_ldm_priv *priv)
{
	W2BYTEMSK(LDM_PWM_REG_1CC,
		  (uint16_t)priv->st_mtk_ldm_dma_info.au32ld_pwm0_period[LDM_BYTES_1],
		  LDM_PWM_REG_1CC_PWM0_PERIOD_1);
	W2BYTEMSK(LDM_PWM_REG_1C8,
		  (uint16_t)priv->st_mtk_ldm_dma_info.au32ld_pwm0_period[LDM_BYTES_0],
		  LDM_PWM_REG_1C8_PWM0_PERIOD_0);
}

static void _mtk_ldm_set_pwm0_shift(struct mtk_ldm_priv *priv)
{
	W2BYTEMSK(LDM_PWM_REG_1D4,
		  (uint16_t)priv->st_mtk_ldm_dma_info.au32ld_pwm0_shift[LDM_BYTES_1],
		  LDM_PWM_REG_1D4_PWM0_SHIFT_1);
	W2BYTEMSK(LDM_PWM_REG_1D0,
		  (uint16_t)priv->st_mtk_ldm_dma_info.au32ld_pwm0_shift[LDM_BYTES_0],
		  LDM_PWM_REG_1D0_PWM0_SHIFT_0);
}

static void _mtk_ldm_set_pwm0_duty(struct mtk_ldm_priv *priv)
{
	W2BYTEMSK(LDM_PWM_REG_1DC,
		  (uint16_t)priv->st_mtk_ldm_dma_info.au32ld_pwm0_duty[LDM_BYTES_1],
		  LDM_PWM_REG_1DC_PWM0_DUTY_1);
	W2BYTEMSK(LDM_PWM_REG_1D8,
		  (uint16_t)priv->st_mtk_ldm_dma_info.au32ld_pwm0_duty[LDM_BYTES_0],
		  LDM_PWM_REG_1D8_PWM0_DUTY_0);
}

static void _mtk_ldm_set_pwm1_period(struct mtk_ldm_priv *priv)
{
	W2BYTEMSK(LDM_PWM_REG_1EC,
		  (uint16_t)priv->st_mtk_ldm_dma_info.au32ld_pwm1_period[LDM_BYTES_1],
		  LDM_PWM_REG_1EC_PWM1_PERIOD_1);
	W2BYTEMSK(LDM_PWM_REG_1E8,
		  (uint16_t)priv->st_mtk_ldm_dma_info.au32ld_pwm1_period[LDM_BYTES_0],
		  LDM_PWM_REG_1E8_PWM1_PERIOD_0);
}

static void _mtk_ldm_set_pwm1_shift(struct mtk_ldm_priv *priv)
{
	W2BYTEMSK(LDM_PWM_REG_1F4,
		  (uint16_t)priv->st_mtk_ldm_dma_info.au32ld_pwm1_shift[LDM_BYTES_1],
		  LDM_PWM_REG_1F4_PWM1_SHIFT_1);
	W2BYTEMSK(LDM_PWM_REG_1F0,
		  (uint16_t)priv->st_mtk_ldm_dma_info.au32ld_pwm1_shift[LDM_BYTES_0],
		  LDM_PWM_REG_1F0_PWM1_SHIFT_0);
}

static void _mtk_ldm_set_pwm1_duty(struct mtk_ldm_priv *priv)
{
	W2BYTEMSK(LDM_PWM_REG_1FC,
		  (uint16_t)priv->st_mtk_ldm_dma_info.au32ld_pwm1_duty[LDM_BYTES_1],
		  LDM_PWM_REG_1FC_PWM1_DUTY_1);
	W2BYTEMSK(LDM_PWM_REG_1F8,
		  (uint16_t)priv->st_mtk_ldm_dma_info.au32ld_pwm1_duty[LDM_BYTES_0],
		  LDM_PWM_REG_1F8_PWM1_DUTY_0);
}

static void _mtk_ldm_set_pwm0_vdben(bool bEn)
{
	W2BYTEMSK(LDM_PWM_REG_008, (uint16_t)bEn, LDM_PWM_REG_0008_PWM0_VDBEN);
}

static void _mtk_ldm_set_pwm0_reset_en(bool bEn)
{
	W2BYTEMSK(LDM_PWM_REG_008, (uint16_t)bEn, LDM_PWM_REG_0008_PWM0_RESET_EN);
}

static void _mtk_ldm_set_pwm0_vdben_sw(bool bEn)
{
	W2BYTEMSK(LDM_PWM_REG_008, (uint16_t)bEn, LDM_PWM_REG_0008_PWM0_VDBEN_SW);
}

static void _mtk_ldm_set_pwm1_vdben(bool bEn)
{
	W2BYTEMSK(LDM_PWM_REG_00C, (uint16_t)bEn, LDM_PWM_REG_000C_PWM1_VDBEN);
}

static void _mtk_ldm_set_pwm1_reset_en(bool bEn)
{
	W2BYTEMSK(LDM_PWM_REG_00C, (uint16_t)bEn, LDM_PWM_REG_000C_PWM1_RESET_EN);
}

static void _mtk_ldm_set_pwm1_vdben_sw(bool bEn)
{
	W2BYTEMSK(LDM_PWM_REG_00C, (uint16_t)bEn, LDM_PWM_REG_000C_PWM1_VDBEN_SW);
}

static void _mtk_ldm_set_pwm_init(struct mtk_ldm_priv *priv)
{
	_mtk_ldm_set_pwm_clken_bitmap();

	_mtk_ldm_set_pwm0_vdben(1);
	_mtk_ldm_set_pwm0_reset_en(1);
	_mtk_ldm_set_pwm0_vdben_sw(1);
	_mtk_ldm_set_pwm0_period(priv);
	_mtk_ldm_set_pwm0_duty(priv);
	_mtk_ldm_set_pwm0_shift(priv);

	_mtk_ldm_set_pwm1_vdben(1);
	_mtk_ldm_set_pwm1_reset_en(1);
	_mtk_ldm_set_pwm1_vdben_sw(1);
	_mtk_ldm_set_pwm1_period(priv);
	_mtk_ldm_set_pwm1_duty(priv);
	_mtk_ldm_set_pwm1_shift(priv);
	_mtk_ldm_set_pwm_vsync_like(priv);
	_mtk_ldm_set_pwm_sel(priv);
}

int32_t _mtk_ldm_as3824_init(struct mtk_ldm_priv *priv, uint8_t u8CH)
{
	int32_t ret = 0;
	int i = 0;
	uint8_t u8temp = 0;
	uint32_t u32SPI_Devicenum;
	struct mtk_ldm_spi_data WData;
	struct mtk_ldm_spi_data RData;
	memset((void *)&WData, 0, sizeof(struct mtk_ldm_spi_data));
	memset((void *)&RData, 0, sizeof(struct mtk_ldm_spi_data));
	uint8_t LED_Device_AS3824_initial_Reg0x01to0x15[]=
	{
		LDM_AS382X_BCAST_MULTI_BYTE_SAME_DATA, Reg_0x01_to_0x15_NUM, LDM_AS382X_CUR_ON_1_ADDR,
		0x00,0x00,0x00,0x00,0x00, //addr 0x1~0x5 CUR_ON_1, CUR_ON_2 , FAULT_1, GPIO_CTIL, FB_SEL1
		0x00,0x00,0x00,0x00, //addr 0x6~0x9 , FB_SEL2, CURRctrl, SHORTLED1, SHORTLED2
		0x00,0x00,0x00,0x00, //addr 0xA~0xD, OPENLED1, OPENLED2, VDAC_H, VDAC_L
		0x00,0x00,0x00,0x00, //addr 0xE~0x11, FB_ON_1, FB_ON_2, IDAC_FB1_COUNTER, IDAC_FB2_COUNTER
		0x00,0x00,0x00,0x00, //addr 0x12~0x15, FBLOOP_CTRL, PWMCTRL, PWMperiodLSB, PWMperiodMSB

		0x00,0x00,0x00,0x00,0x00, //Dummy*25
		0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,
	};

	uint8_t LED_Device_AS3824_initial_PLLmulti[]=
	{
		LDM_AS382X_BCAST_MULTI_BYTE_SAME_DATA, PLL_MULTI_BYTE_NUM, LDM_AS382X_PLLmulti1_ADDR,
		0x00,0x00,

		0x00,0x00,0x00,0x00,0x00, //Dummy*25
		0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,
	};

	uint8_t LED_Device_AS3824_initial_HDR[]=
	{
		LDM_AS382X_BCAST_SINGLE_BYTE_SAME_DATA, LDM_AS382X_HDR_ADDR, LDM_NUM_16,

		0x00,0x00,0x00,0x00,0x00, //Dummy*25
		0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,
	};

	uint8_t LED_Device_AS3824_initial_PWM_Delay[]= //Delay
	{
		LDM_AS382X_BCAST_MULTI_BYTE_SAME_DATA, LDM_AS382X_DELAY_NUM, LDM_AS382X_DELAY1_ADDR, //addr 0x16~0x35
		0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,

		0x00,0x00,0x00,0x00,0x00, //Dummy*25
		0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,
	};

	uint8_t LED_Device_AS3824_initial_PWM_Duty[]= //Duty
	{
		LDM_AS382X_BCAST_MULTI_BYTE_SAME_DATA, LDM_AS382X_PWM_HTIME_NUM, LDM_AS382X_PWM_HTIME1_ADDR, //addr 0x37~0x56
		0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,

		0x00,0x00,0x00,0x00,0x00, //Dummy*25
		0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,
	};

	uint8_t LED_Device_AS3824_initial_BDAC[]= //BDAC
	{
		LDM_AS382X_BCAST_MULTI_BYTE_SAME_DATA, LDM_AS382X_BDAC_NUM, LDM_AS382X_BDAC1_ADDR, //addr 0x37~0x56
		0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,

		0x00,0x00,0x00,0x00,0x00, //Dummy*25
		0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,
	};

	uint8_t LED_Device_AS3824_read_single_data[]=
	{
		LDM_AS382X_SINGLE_BYTE_SAME_DATA, 0x00, 0x00,

		0x00,0x00,0x00,0x00,0x00, //Dummy*25
		0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,
	};

	LED_Device_AS3824_read_single_data[LDM_BYTES_0] = (LDM_AS382X_SINGLE_BYTE_SAME_DATA | LDM_BYTES_1);
	LED_Device_AS3824_read_single_data[LDM_BYTES_1] = LDM_AS382X_STATUS_ADDR;
	LED_Device_AS3824_read_single_data[LDM_BYTES_2] = 0x00;

	// Wait 3824 power on
	// check 0xff for Nova case
	UBOOT_DEBUG("Wait AS3824 Power");
	if ((priv->st_mtk_ldm_device_info.u32device_num[u8CH] + LDM_AS382X_WRITE_SAME_DATA_PWM_CMD_NUM)
		>= MAX_LDM_SPI_BYTES)
	{
		debug("[%s,%5d] Over Size!!!\n", __func__, __LINE__);
		return 0;
	}
	u32SPI_Devicenum = priv->st_mtk_ldm_device_info.u32device_num[u8CH];
	do
	{
		memset((void *)&WData,0,sizeof(struct mtk_ldm_spi_data));
		memset((void *)&RData,0,sizeof(struct mtk_ldm_spi_data));
		WData.u16len = LDM_AS382X_SINGLE_READ_CMD_NUM + u32SPI_Devicenum;
		memcpy(&WData.au8Data, LED_Device_AS3824_read_single_data, sizeof(LED_Device_AS3824_read_single_data));

		RData.u16len = 1;
		_mtk_ldm_spi_data(priv, u8CH, &WData, &RData);
		u8temp = RData.au8Data[0];

		UBOOT_DEBUG("check as3824 power %x:", u8temp);
	}while((u8temp == NT50516_POWER_ON)||(u8temp&BIT0)== 0);
	UBOOT_DEBUG("AS3824 power on");

	/*unlock device*/
	WData.u16len = LDM_AS382X_BCAST_SINGLE_WRITE_CMD_NUM + u32SPI_Devicenum;
	RData.u16len = 0;

	memcpy(&WData.au8Data, LED_Device_AS3824_unlock, sizeof(LED_Device_AS3824_unlock));
	ret = _mtk_ldm_spi_data(priv, u8CH, &WData, &RData);
	if(ret)
	{
		debug("[%s,%5d] SPI transfer error = %d!!!\n", __func__, __LINE__,  ret);
	}

	/*inital reg0x01~0x15*/
	for(i = 0; i < Reg_0x01_to_0x0B_NUM ; i++)
	{
		LED_Device_AS3824_initial_Reg0x01to0x15[LDM_AS382X_HEADER_NUM + i] = (uint8_t)priv->st_ldm_Device_AS3824.u32AS3824_Reg_0x01_to_0x0B[i];
	}

	for(i = 0; i < VDAC_BYTE_NUM ; i++)
	{
		LED_Device_AS3824_initial_Reg0x01to0x15[LDM_AS382X_HEADER_NUM +  LDM_AS382X_VDAC_LEDH_ADDR - 1 + i] =  (uint8_t)priv->st_ldm_Device_AS3824.u32AS3824_Reg_0x0C_0x0D_VDAC_MBR_OFF[i];
	}

	for(i = 0; i < Reg_0x0E_to_0x15_NUM ; i++)
	{
		LED_Device_AS3824_initial_Reg0x01to0x15[LDM_AS382X_HEADER_NUM + LDM_AS382X_FB_ON1_ADDR -1 + i] = (uint8_t)priv->st_ldm_Device_AS3824.u32AS3824_Reg_0x0E_to_0x15[i];
	}

	for(i = 0; i < LDM_AS382X_PWM_HTIME_NUM ; i++)
	{
		LED_Device_AS3824_initial_PWM_Duty[LDM_AS382X_HEADER_NUM + i] = (uint8_t)(priv->st_ldm_Device_AS3824.u16AS3824_PWM_Duty_Init & MASK_2BYTES);
		LED_Device_AS3824_initial_PWM_Duty[LDM_AS382X_HEADER_NUM + i + 1] = (uint8_t)(priv->st_ldm_Device_AS3824.u16AS3824_PWM_Duty_Init >> SHIFT_2BYTES);
	}

	WData.u16len = LDM_AS382X_WRITE_SAME_DATA_REG_0x01_0x15_CMD_NUM + u32SPI_Devicenum;
	RData.u16len = 0;
	memcpy(&WData.au8Data, LED_Device_AS3824_initial_Reg0x01to0x15, sizeof(LED_Device_AS3824_initial_Reg0x01to0x15));
	ret = _mtk_ldm_spi_data(priv, u8CH, &WData, &RData);
	if(ret)
	{
		debug("[%s,%5d] SPI transfer error = %d!!!\n", __func__, __LINE__,  ret);
	}

	/*inital PLLmulti*/
	WData.u16len = LDM_AS382X_WRITE_SAME_DATA_PLL_MULTI_CMD_NUM + u32SPI_Devicenum;
	RData.u16len = 0;
	memcpy(&WData.au8Data, LED_Device_AS3824_initial_PLLmulti, sizeof(LED_Device_AS3824_initial_PLLmulti));
	ret = _mtk_ldm_spi_data(priv, u8CH, &WData, &RData);
	if(ret)
	{
		debug("[%s,%5d] SPI transfer error = %d!!!\n", __func__, __LINE__,  ret);
	}

	/*inital HDR*/
	WData.u16len = LDM_AS382X_BCAST_SINGLE_WRITE_CMD_NUM + u32SPI_Devicenum;
	RData.u16len = 0;
	memcpy(&WData.au8Data, LED_Device_AS3824_initial_HDR, sizeof(LED_Device_AS3824_initial_HDR));
	ret = _mtk_ldm_spi_data(priv, u8CH, &WData, &RData);
	if(ret)
	{
		debug("[%s,%5d] SPI transfer error = %d!!!\n", __func__, __LINE__,  ret);
	}

	/*inital PWM_delay*/
	WData.u16len = LDM_AS382X_WRITE_SAME_DATA_PWM_CMD_NUM + u32SPI_Devicenum;
	RData.u16len = 0;
	memcpy(&WData.au8Data, LED_Device_AS3824_initial_PWM_Delay, sizeof(LED_Device_AS3824_initial_PWM_Delay));
	ret = _mtk_ldm_spi_data(priv, u8CH, &WData, &RData);
	if(ret)
	{
		debug("[%s,%5d] SPI transfer error = %d!!!\n", __func__, __LINE__,  ret);
	}

	/*inital PWM_Duty*/
	WData.u16len = LDM_AS382X_WRITE_SAME_DATA_PWM_CMD_NUM + u32SPI_Devicenum;
	RData.u16len = 0;
	memcpy(&WData.au8Data, LED_Device_AS3824_initial_PWM_Duty, sizeof(LED_Device_AS3824_initial_PWM_Duty));
	ret = _mtk_ldm_spi_data(priv, u8CH, &WData, &RData);
	if(ret)
	{
		debug("[%s,%5d] SPI transfer error = %d!!!\n", __func__, __LINE__,  ret);
	}

	/*inital BDAC*/
	WData.u16len = LDM_AS382X_WRITE_SAME_DATA_BDAC_CMD_NUM + u32SPI_Devicenum;
	RData.u16len = 0;
	memcpy(&WData.au8Data, LED_Device_AS3824_initial_BDAC, sizeof(LED_Device_AS3824_initial_BDAC));
	ret = _mtk_ldm_spi_data(priv, u8CH, &WData, &RData);
	if(ret)
	{
		debug("[%s,%5d] SPI transfer error = %d!!!\n", __func__, __LINE__,  ret);
	}

	/*lock device*/
	WData.u16len = LDM_AS382X_BCAST_SINGLE_WRITE_CMD_NUM + u32SPI_Devicenum;
	RData.u16len = 0;
	memcpy(&WData.au8Data, LED_Device_AS3824_lock, sizeof(LED_Device_AS3824_lock));
	ret = _mtk_ldm_spi_data(priv, u8CH, &WData, &RData);
	if(ret)
	{
		debug("[%s,%5d] SPI transfer error = %d!!!\n", __func__, __LINE__,  ret);
	}

	return ret;

}

uint16_t _mtk_ldm_mspi_read_iw7039_single_data_v2(
	struct mtk_ldm_priv *priv, uint8_t u8CH, u8 u8chip_num, u8 u8device_idx, u16 u16register_addr)
{
	u8 u8dummy_no = (u8chip_num / IW7039_DUMMY_CAL_NUM) + 1;
	u16 u16temp = 0;
	uint8_t au8Data[LDM_NUM_256];
	uint16_t u16ReadVal = 0;
	uint8_t i = 0;
	struct mtk_ldm_spi_data WData;
	struct mtk_ldm_spi_data RData;
	memset((void *)&WData, 0, sizeof(struct mtk_ldm_spi_data));
	memset((void *)&RData, 0, sizeof(struct mtk_ldm_spi_data));
	if (u8chip_num > IW7039_DAISY_CHAIN_CHIP_NUM_MAX)
	{
		UBOOT_DEBUG("[MSPI_INIT]MSPI_Read_Iw7039_SingleData number of device wrong!\n");
		return 0;
	}

	if (u8device_idx > IW7039_SINGLE_DATA_READ_DEVICE_NUM_MAX)
	{
		UBOOT_DEBUG("[MSPI_INIT]MSPI_Read_Iw7039_SingleData read device no. wrong!\n");
		return 0;
	}
	// command
	au8Data[LDM_BYTES_0] = (uint8_t)((COMMAND_IW7039_READ_SINGLE_DEVICE >> SHIFT_2BYTES) + u8device_idx);
	au8Data[LDM_BYTES_1] = (uint8_t)(COMMAND_IW7039_READ_SINGLE_DEVICE & MASK_2BYTES);

	// address
	u16temp = (ADDRESS_IW7039_READ_SINGLE_DEVICE + u16register_addr);
	au8Data[LDM_BYTES_2] = (uint8_t)(u16temp >> SHIFT_2BYTES);
	au8Data[LDM_BYTES_3] = (uint8_t)((u16temp & MASK_2BYTES));

	// device dummy
	for (i = 0; i < u8dummy_no; i++)
	{
		au8Data[LDM_NUM_6+BYTE2*i] = 0;
		au8Data[LDM_NUM_6+BYTE2*i+1] = 0;
	}

	WData.u16len = LDM_NUM_4 + (u8dummy_no * BYTE2);
	memset((void *)&RData,0,sizeof(struct mtk_ldm_spi_data));
	memcpy(&WData.au8Data, au8Data, WData.u16len);
	RData.u16len = LDM_NUM_2;
	_mtk_ldm_spi_data(priv, u8CH, &WData, &RData);
	u16ReadVal = RData.au8Data[1] | ((RData.au8Data[0]&MASK_2BYTES)<<SHIFT_2BYTES);

	return u16ReadVal;

}

void _mtk_ldm_mspi_write_all_iw7039_same_single_data(
	struct mtk_ldm_priv *priv, uint8_t u8CH, uint8_t u8chip_num, uint16_t u16register_addr, uint16_t u16inputd_data)
{
	u8 u8dummy_no = (u8chip_num / IW7039_DUMMY_CAL_NUM) + 1;
	u16 u16temp = 0;
	uint8_t au8Data[LDM_NUM_256];
	uint8_t i = 0;
	struct mtk_ldm_spi_data WData;
	struct mtk_ldm_spi_data RData;
	memset((void *)&WData, 0, sizeof(struct mtk_ldm_spi_data));
	memset((void *)&RData, 0, sizeof(struct mtk_ldm_spi_data));
	if (u8chip_num > IW7039_DAISY_CHAIN_CHIP_NUM_MAX)
	{
		UBOOT_DEBUG("[MSPI_INIT]MSPI_Write_Iw7039_SetDaisyChainChipNum number of device wrong!\n");
		return;
	}

	// SPI SW mode, pull high VSync GPIO7
	//GPIO_SetOut(7, 1);

	// command
	au8Data[LDM_BYTES_0] = (uint8_t)(COMMAND_IW7039_WRITE_SAME_ONE_WORD_OF_ALL_DEVICE >> SHIFT_2BYTES);
	au8Data[LDM_BYTES_1] = (uint8_t)(COMMAND_IW7039_WRITE_SAME_ONE_WORD_OF_ALL_DEVICE & MASK_2BYTES);

	// address
	u16temp = (ADDRESS_IW7039_WRITE_SAME_ONE_WORD_OF_ALL_DEVICE + u16register_addr);
	au8Data[LDM_BYTES_2] = (uint8_t)(u16temp >> SHIFT_2BYTES);
	au8Data[LDM_BYTES_3] = (uint8_t)((u16temp & MASK_2BYTES));

	// data
	au8Data[LDM_BYTES_4] = (uint8_t)(u16inputd_data >> SHIFT_2BYTES);
	au8Data[LDM_BYTES_5] = (uint8_t)((u16inputd_data & MASK_2BYTES));

	// dummy
	for (i = 0; i < u8dummy_no; i++)
	{
		au8Data[LDM_NUM_6+BYTE2*i] = 0;
		au8Data[LDM_NUM_6+BYTE2*i+1] = 0;
	}

	WData.u16len = (LDM_NUM_6 + (BYTE2 * u8dummy_no));
	RData.u16len = 0;
	memcpy(&WData.au8Data, au8Data, WData.u16len);
	_mtk_ldm_spi_data(priv, u8CH, &WData, &RData);

	// SPI SW mode, pull low VSync GPIO7
	//GPIO_SetOut(7, 0);
}

void _mtk_ldm_mspi_write_all_iw7039_same_32word_array_data(
	struct mtk_ldm_priv *priv, uint8_t u8CH, uint8_t u8chip_num, uint16_t u16register_addr, uint16_t *p16data)
{
	u8 u8dummy_no = (u8chip_num / IW7039_DUMMY_CAL_NUM) + 1;
	u16 u16temp = 0;
	uint16_t i = 0;
	uint8_t au8Data[LDM_NUM_256];
	struct mtk_ldm_spi_data WData;
	struct mtk_ldm_spi_data RData;
	memset((void *)&WData, 0, sizeof(struct mtk_ldm_spi_data));
	memset((void *)&RData, 0, sizeof(struct mtk_ldm_spi_data));
	if (u8chip_num > IW7039_DAISY_CHAIN_CHIP_NUM_MAX)
	{
		UBOOT_DEBUG("[MSPI_INIT]MSPI_Write_Iw7039_SetDaisyChainChipNum number of device wrong!\n");
		return;
	}

	// SPI SW mode, pull high VSync GPIO7
	//GPIO_SetOut(7, 1);

	// command
	au8Data[LDM_BYTES_0] = (uint8_t)(COMMAND_IW7039_WRITE_SAME_32_WORD_OF_ALL_DEVICE >> SHIFT_2BYTES);
	au8Data[LDM_BYTES_1] = (uint8_t)(COMMAND_IW7039_WRITE_SAME_32_WORD_OF_ALL_DEVICE & MASK_2BYTES);

	// address
	u16temp = (ADDRESS_IW7039_WRITE_SAME_32_WORD_OF_ALL_DEVICE + u16register_addr);
	au8Data[LDM_BYTES_2] = (uint8_t)(u16temp >> SHIFT_2BYTES);
	au8Data[LDM_BYTES_3] = (uint8_t)((u16temp & MASK_2BYTES));

	// data
	for (i = 0; i < IW7039_32WORD; i++)
	{
		au8Data[LDM_NUM_4 + (BYTE2 * i)] = (uint8_t)(p16data[i] >> SHIFT_2BYTES);
		au8Data[LDM_NUM_4 + (BYTE2 * i) + 1] = (uint8_t)((p16data[i] & MASK_2BYTES));
	}

	// dummy
	for (i = 0; i < u8dummy_no; i++)
	{
		au8Data[LDM_NUM_4 + (IW7039_32WORD * BYTE2) + (BYTE2 * i)] = 0;
		au8Data[LDM_NUM_4 + (IW7039_32WORD * BYTE2) + (BYTE2 * i) + 1] = 0;
	}

	WData.u16len = (LDM_NUM_4 + (IW7039_32WORD * BYTE2) + (BYTE2 * u8dummy_no));
	RData.u16len = 0;
	memcpy(&WData.au8Data, au8Data, WData.u16len);
	_mtk_ldm_spi_data(priv, u8CH, &WData, &RData);

	// SPI SW mode, pull low VSync GPIO7
	//GPIO_SetOut(7, 0);
}

void _mtk_ldm_mspi_write_iw7039_set_daisy_chain_chip_num(struct mtk_ldm_priv *priv, uint8_t u8CH, uint8_t u8chip_num)
{
	u8 u8dummy_no = (u8chip_num / IW7039_DUMMY_CAL_NUM) + 1;
	u8 i = 0;
	u8 au8Data[LDM_NUM_256];
	struct mtk_ldm_spi_data WData;
	struct mtk_ldm_spi_data RData;

	memset((void *)&WData, 0, sizeof(struct mtk_ldm_spi_data));
	memset((void *)&RData, 0, sizeof(struct mtk_ldm_spi_data));

	if (u8chip_num > IW7039_DAISY_CHAIN_CHIP_NUM_MAX) {
		UBOOT_ERROR("[MSPI_INIT]MSPI_Write_Iw7039_SetDaisyChainChipNum number of device wrong!\n");
		return;
	}

	// command
	au8Data[LDM_BYTES_0] = (u8)(COMMAND_IW7039_SET_NUM_OF_DEVICE >> SHIFT_2BYTES);
	au8Data[LDM_BYTES_1] = (u8)(u8chip_num);

	// device dummy
	for (i = 0; i < u8dummy_no; i++) {
		au8Data[BYTE2 + (BYTE2 * i)] = 0;
		au8Data[BYTE2 + (BYTE2 * i) + 1] = 0;
	}

	// Send SPI command
	WData.u16len = (LDM_NUM_2 + (BYTE2 * u8dummy_no));
	RData.u16len = 0;
	memcpy(&WData.au8Data, au8Data, WData.u16len);
	_mtk_ldm_spi_data(priv, u8CH, &WData, &RData);
}

void _mtk_ldm_iw7039_init_turn_on_backlight(struct mtk_ldm_priv *priv, uint8_t u8CH)
{
	u8 u8SPI_Devicenum;

	if (priv->st_mtk_ldm_device_info.u32device_num[u8CH] > MAX_LDM_SPI_BYTES)
		priv->st_mtk_ldm_device_info.u32device_num[u8CH] = MAX_LDM_SPI_BYTES;
	u8SPI_Devicenum = priv->st_mtk_ldm_device_info.u32device_num[u8CH];
#if IW7039_SIMPLE_CHKSUM
	//12.Soft start IW7039 calibration process
	_mtk_ldm_mspi_write_all_iw7039_same_single_data(priv, u8CH, u8SPI_Devicenum, 0x000, IW7039_FINISH_INIT_REG0_W_CHK);
#else
	//12.Soft start IW7039 calibration process
	_mtk_ldm_mspi_write_all_iw7039_same_single_data(priv, u8CH, u8SPI_Devicenum, 0x000, IW7039_FINISH_INIT_REG0_WO_CHK);
	msleep(SLEEP_TIME_1);
#endif

	UBOOT_DEBUG("pqu_ldm_iw7039_init end\n");
}

int32_t _mtk_ldm_iw7039_init(struct mtk_ldm_priv *priv, uint8_t u8CH)
{
	int32_t ret = 0;
	u16 u16read_val = 0;
	uint8_t u8SPI_Devicenum;
	uint8_t i = 0;
	u8 u8retry = 0;
	struct mtk_ldm_spi_data WData;
	struct mtk_ldm_spi_data RData;
	memset((void *)&WData, 0, sizeof(struct mtk_ldm_spi_data));
	memset((void *)&RData, 0, sizeof(struct mtk_ldm_spi_data));
	uint16_t intReg_Conf[Reg_0x00_to_0x1F_NUM] = {0x00, 0x00,0x00, 0x00, 0x00, 0x00, 0x00, 0x00,//0X00 ~ 0X07 , reg 0x001 config to use global ISET with 30mA, reg 0x05 config to 4x Vsync
											0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,// 0X08 ~ 0X0F ( 0x20 set ILED range as 60mA)
											0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,// 0X10 ~ 0X17 , Reg 0x10 , 0x11 enable 32 CHs, Reg 0x14 turn-off all fault , ADJ-short doesn¡¯t turn off
											0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,//0X18 ~0X1F , reg 0x18 and 0x19 enable ADP for 32 CHs
											}; //Initializing

	// Setting individual ILED
	uint16_t intReg_ISET[IW7039_PWM_ISET_NUM] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,// 0x20 ~ 0x27
											0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,// 0x28 ~ 0x2F
											0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,// 0x30 ~ 0x37
											0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,// 0x38 ~ 0x3F
											};//0x20 to 0x3F

	uint16_t intReg_HT[IW7039_PWM_HTIME_NUM] = {0,0, 0,0, 0,0, 0,0,// 0x60 ~ 0x67
											0,0, 0,0, 0,0, 0,0,// 0x68 ~ 0x6F
											0,0, 0,0, 0,0, 0,0,// 0x70 ~ 0x77
											0,0, 0,0, 0,0, 0,0,// 0x78 ~ 0x7F
											} ;//0x60 to 0x7F
	if (priv->st_mtk_ldm_device_info.u32device_num[u8CH] > MAX_LDM_SPI_BYTES)
		priv->st_mtk_ldm_device_info.u32device_num[u8CH] = MAX_LDM_SPI_BYTES;
	u8SPI_Devicenum = priv->st_mtk_ldm_device_info.u32device_num[u8CH];
	msleep(SLEEP_TIME_100);

	UBOOT_DEBUG("pqu_ldm_iw7039_init start");
	msleep(SLEEP_TIME_10);

	_mtk_ldm_mspi_write_iw7039_set_daisy_chain_chip_num(priv, u8CH, u8SPI_Devicenum);

	msleep(SLEEP_TIME_10);

	UBOOT_DEBUG("check IW7039 1 power\n");

	do
	{
		u16read_val = _mtk_ldm_mspi_read_iw7039_single_data_v2(priv, u8CH, u8SPI_Devicenum, 1, IW7039_0x344_ADDR);
		u16read_val = (u16read_val >> SHIFT_2BYTES) & MASK_2BYTES;
		u8retry++;
	} while ((u16read_val != IW7039_POWER_ON) && (u8retry <= RETRY_COUNTER));

	if (u16read_val == IW7039_POWER_ON)
		UBOOT_DEBUG("IW7039 CH:%d power 1 on\n", u8CH);
	else
		UBOOT_ERROR("IW7039 CH:%d power Off!!!!!!!!!!!!!!!!!!\n", u8CH);

	_mtk_ldm_mspi_write_all_iw7039_same_single_data(priv, u8CH, u8SPI_Devicenum, 0x000, IW7039_INIT_REG0_VALUE);
	msleep(SLEEP_TIME_10);

	UBOOT_DEBUG("IW7039 init table\n");
	for(i = 0; i < Reg_0x00_to_0x1F_NUM ; i++)
	{
		intReg_Conf[i] = priv->st_ldm_Device_IW7039.u32IW7039_Reg_0x000_to_0x01F[i];
	}

	_mtk_ldm_mspi_write_all_iw7039_same_32word_array_data(priv, u8CH, u8SPI_Devicenum, 0x000, intReg_Conf);
	msleep(SLEEP_TIME_10);

	UBOOT_DEBUG("IW7039 init ISET\n");
	for(i = 0; i < IW7039_PWM_ISET_NUM ; i++)
	{
		intReg_ISET[i] = priv->st_ldm_Device_IW7039.u16IW7039_ISET_MBR_OFF;
	}

	//Init ISET 0x20 ~ 0x3F
	_mtk_ldm_mspi_write_all_iw7039_same_32word_array_data(priv, u8CH, u8SPI_Devicenum, IW7039_ISET_ADDR, intReg_ISET);
	msleep(SLEEP_TIME_10);

	//Init HT 0x60 ~ 0x7F to 0 before read calibration state
	UBOOT_DEBUG("IW7039 init DUTY\n");
	if(1) //quiet hot boot
	{
		for(i = 0; i < IW7039_PWM_HTIME_NUM ; i++)
		{
			intReg_HT[i] = priv->st_ldm_Device_IW7039.u16IW7039_PWM_Duty_Init;
		}

		_mtk_ldm_mspi_write_all_iw7039_same_32word_array_data(priv, u8CH, u8SPI_Devicenum, IW7039_PWM_HTIME1_ADDR, intReg_HT);
		msleep(SLEEP_TIME_10);
	}
	else
	{
		_mtk_ldm_mspi_write_all_iw7039_same_32word_array_data(priv, u8CH, u8SPI_Devicenum, IW7039_PWM_HTIME1_ADDR, intReg_HT);
	}

	msleep(SLEEP_TIME_200);

	return ret;

}

int32_t _mtk_ldm_mbi6353_init(struct mtk_ldm_priv *priv, uint8_t u8CH)
{
	int32_t ret = 0;
	int i = 0;
	uint32_t u32SPI_Devicenum;
	struct mtk_ldm_spi_data WData;
	struct mtk_ldm_spi_data RData;
	uint8_t LED_Device_MBI6353_initial_Mask[]=
	{
		MBI6353_WRITE_SAME_ONE_WORD_OF_ALL_DEVICE_H, MBI6353_WRITE_SAME_ONE_WORD_OF_ALL_DEVICE_L, 0x00, MBI6353_MASK_NUM, MBI6353_MASK_ADDR_H, MBI6353_MASK_ADDR_L, //header
		0x00,0x00,0x00,0x00,0x00,0x00, //data
		0x00,0x00, //check sum
		0x00,0x00,0x00,0x00, //dummy
		0x00,0x00,0x00,0x00, 
		0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00, 
		0x00,0x00,0x00,0x00,0x00, //Dummy*25
		0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,
	};

	uint8_t LED_Device_MBI6353_initial_Config[]=
	{
		MBI6353_WRITE_SAME_ONE_WORD_OF_ALL_DEVICE_H, MBI6353_WRITE_SAME_ONE_WORD_OF_ALL_DEVICE_L, 0x00, MBI6353_CFG_NUM, MBI6353_CFG_ADDR_H, MBI6353_CFG_ADDR_L, //header
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00,0x00, //check sum
		0x00,0x00,0x00,0x00,0x00, //Dummy*25
		0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,
	};

	uint8_t LED_Device_MBI6353_initial_Duty[]=
	{
		MBI6353_WRITE_SAME_ONE_WORD_OF_ALL_DEVICE_H, MBI6353_WRITE_SAME_ONE_WORD_OF_ALL_DEVICE_L, 0x00, MBI6353_DUTY_NUM, MBI6353_SCAN1_DUTY_H, MBI6353_SCAN1_DUTY_L, //header
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 192 = 48 X 4 SCAN
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 

		0x00,0x00, //check sum
		0x00,0x00,0x00,0x00,0x00, //Dummy*25
		0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,
	};
	if (priv->st_mtk_ldm_device_info.u32device_num[u8CH] >= MAX_LDM_SPI_BYTES)
	{
		debug("[%s,%5d] Over Size!!!\n", __func__, __LINE__);
		return 0;
	}
	u32SPI_Devicenum = priv->st_mtk_ldm_device_info.u32device_num[u8CH];
/*inital Mask*/
	for(i = 0; i < MBI6353_MASK_NUM * BYTE2; i++)
	{
		LED_Device_MBI6353_initial_Mask[(MBI6353_HEADER_NUM * BYTE2) + i] = (uint8_t)priv->st_ldm_Device_MBI6353.u32MBI6353_Mask[i];
	}

	for(i = 0; i < MBI6353_CFG_NUM * BYTE2 ; i++)
	{
		LED_Device_MBI6353_initial_Config[(MBI6353_HEADER_NUM * BYTE2) + i] = (uint8_t)priv->st_ldm_Device_MBI6353.u32MBI6353_Config[i];
	}

	for(i = 0; i < MBI6353_DUTY_NUM; i++)
	{
		LED_Device_MBI6353_initial_Duty[(MBI6353_HEADER_NUM * BYTE2) + (BYTE2 * i)] = (uint8_t)(priv->st_ldm_Device_MBI6353.u16MBI6353_PWM_Duty_Init>> SHIFT_2BYTES);
		LED_Device_MBI6353_initial_Duty[(MBI6353_HEADER_NUM * BYTE2) + (BYTE2 * i) + 1] = (uint8_t)(priv->st_ldm_Device_MBI6353.u16MBI6353_PWM_Duty_Init & MASK_2BYTES);
	}

	/*SEND MASK*/
	WData.u16len = (MBI6353_HEADER_NUM + MBI6353_MASK_NUM + MBI6353_CHECKSUM_NUM + u32SPI_Devicenum - 1) * BYTE2;
	RData.u16len = 0;
	memcpy(&WData.au8Data, LED_Device_MBI6353_initial_Mask, sizeof(LED_Device_MBI6353_initial_Mask));
	_mtk_ldm_spi_data(priv, u8CH, &WData, &RData);
	if(ret)
	{
		UBOOT_DEBUG("[%s,%5d] SPI transfer error = %d!!!\n", __func__, __LINE__,  ret);
	}

	/*SEND CONFIG*/
	WData.u16len = (MBI6353_HEADER_NUM + MBI6353_CFG_NUM + MBI6353_CHECKSUM_NUM + u32SPI_Devicenum - 1) * BYTE2;
	RData.u16len = 0;
	memcpy(&WData.au8Data, LED_Device_MBI6353_initial_Config, sizeof(LED_Device_MBI6353_initial_Config));
	_mtk_ldm_spi_data(priv, u8CH, &WData, &RData);
	if(ret)
	{
		UBOOT_DEBUG("[%s,%5d] SPI transfer error = %d!!!\n", __func__, __LINE__,  ret);
	}

	/*SEND DUTY INIT*/
	WData.u16len = (MBI6353_HEADER_NUM + MBI6353_DUTY_NUM + MBI6353_CHECKSUM_NUM + u32SPI_Devicenum - 1) * BYTE2;
	RData.u16len = 0;
	memcpy(&WData.au8Data, LED_Device_MBI6353_initial_Duty, sizeof(LED_Device_MBI6353_initial_Duty));
	_mtk_ldm_spi_data(priv, u8CH, &WData, &RData);
	if(ret)
	{
		UBOOT_DEBUG("[%s,%5d] SPI transfer error = %d!!!\n", __func__, __LINE__,  ret);
	}
	

	return ret;

}

int32_t _mtk_ldm_cus_led_device_init(
struct mtk_ldm_priv *priv, uint8_t u8CH)
{
	int32_t ret = 0;
	uint8_t au8WData[LDM_NUM_256] = {0};
	struct mtk_ldm_spi_data WData;
	struct mtk_ldm_spi_data RData;
	memset((void *)&WData, 0, sizeof(struct mtk_ldm_spi_data));
	memset((void *)&RData, 0, sizeof(struct mtk_ldm_spi_data));
	if ((priv->st_mtk_ldm_device_info.u32device_num[u8CH] + LDM_NUM_10) >= MAX_LDM_SPI_BYTES)
	{
		debug("[%s,%5d] Over Size!!!\n", __func__, __LINE__);
		return 0;
	}
	WData.u16len = LDM_NUM_10 + priv->st_mtk_ldm_device_info.u32device_num[u8CH];
	RData.u16len = 0;

	memcpy(&WData.au8Data, au8WData, sizeof(au8WData));
	ret = _mtk_ldm_spi_data(priv, u8CH, &WData, &RData);
	if(ret)
	{
		UBOOT_DEBUG("SPI transfer error = %d!!!\n", ret);
	}

	return ret;

}

int32_t _mtk_ldm_leddevice_init(struct mtk_ldm_priv *priv)
{
	int32_t ret = 0;
	u8 i = 0;
	u8 u8ch = 0;
	if (priv == NULL) {
		debug("[%s] get device private fail\n", __func__);
		return ret;
	}

	if (priv->u8ldm_version == LDM_VER_1 || priv->u8ldm_version == LDM_VER_2 || priv->u8ldm_version == LDM_VER_3)
		u8ch = MSPI_MAX_NUM;
	else
		u8ch = BYTE2;

	_mtk_ldm_set_pwm_init(priv);
	switch (priv->st_mtk_ldm_device_info.u8LedDevice_Type)
	{
		case E_LD_DEVICE_UNSUPPORT:
			UBOOT_DEBUG("LD Device is not support\n");
			break;
		case E_LD_DEVICE_AS3824:
			UBOOT_DEBUG("LD Device AS3824 init start\n");
			for (i = 0; i < u8ch; i++)
			{
				if (priv->st_mtk_ldm_device_info.u32device_num[i] > 0)
				{
					_mtk_ldm_set_spi_Init(priv, i);
					ret = _mtk_ldm_as3824_init(priv, i);
					if (!ret)
						UBOOT_DEBUG("AS3824 init spi ch = %d done\n", i);
				}
			}
			break;
		case E_LD_DEVICE_NT50585:
			UBOOT_DEBUG("LD Device NT50585 init start\n");
			break;
		case E_LD_DEVICE_IW7039:
			UBOOT_DEBUG("LD Device IW7039 init start\n");
			for (i = 0; i < u8ch; i++)
			{
				if (priv->st_mtk_ldm_device_info.u32device_num[i] > 0)
				{
					_mtk_ldm_set_spi_Init(priv, i);
					ret = _mtk_ldm_iw7039_init(priv, i);
					if(!ret)
					{
						UBOOT_DEBUG("IW7039 init spi ch = %d done\n", i);
					}
				}
			}
			for (i = 0; i < u8ch; i++) {
				if (priv->st_mtk_ldm_device_info.u32device_num[i] > 0)
					_mtk_ldm_iw7039_init_turn_on_backlight(priv, i);
			}
			break;
		case E_LD_DEVICE_MCU:
			UBOOT_DEBUG("LD Device MCU init start\n");
			break;
		case E_LD_DEVICE_CUS:
			UBOOT_DEBUG("LD Device CUS init start\n");

			for (i = 0; i < u8ch; i++)
			{
				if (priv->st_mtk_ldm_device_info.u32device_num[i] > 0)
				{
					ret = _mtk_ldm_cus_led_device_init(priv, i);
					if (!ret)
						UBOOT_DEBUG("CUS_LED_Device_Init spi ch = %d done\n", i);
				}
			}
			break;
		case E_LD_DEVICE_MBI6353:
			UBOOT_DEBUG("[LDM]LD Device MBI6353 init start\n");
			for (i = 0; i < u8ch; i++)
			{
				if (priv->st_mtk_ldm_device_info.u32device_num[i] > 0)
				{
					ret = _mtk_ldm_mbi6353_init(priv, i);
					if (!ret)
						UBOOT_DEBUG("MBI6353nit spi ch = %d done\n", i);
				}
			}
			break;
		default:
			UBOOT_DEBUG("LD Device is not support\n");
			break;
	}

	return ret;
}

static int mtk_ldm_ofdata_to_platdata(struct udevice *dev)
{
	//dump device tree.
	struct mtk_ldm_priv *priv = dev_get_priv(dev);
	ofnode info_node;
	uint32_t u32Data;
	int ret = 0;
	if (!priv) {
		dev_err(dev, "Get device priv FAIL\n");
		return -EINVAL;
	}

	dev_read_u32(dev, LDM_TAG, &u32Data);
	priv->u32LDMSupport = u32Data;
	dev_read_u32(dev, LDM_VERSION, &u32Data);
	priv->u8ldm_version = u32Data;
	dev_read_u32(dev, LDM_MAKER, &u32Data);
	priv->bLDMCusPathuBoot = (bool)u32Data;
	UBOOT_DEBUG("[%s] LDMinfo = %d %d %d\n",
		__func__,
		priv->u32LDMSupport,
		priv->bLDMCusPathuBoot,
		priv->u8ldm_version);

	dev_read_u32(dev, LED_MSPI_EN, &u32Data);
	priv->bled_mspi_en = (bool)u32Data;

	info_node = dev_read_subnode(dev, LDM_DMA_INFO);
	if(!ofnode_valid(info_node))
	{
		debug("Read subnode (%s) FAIL\n", LDM_DMA_INFO);
	}
	else
	{
		ofnode_read_u32_array(info_node, LDM_VSYNC_WIDTH,
				      priv->st_mtk_ldm_dma_info.au32ld_vsync_width, LDM_VSYNC_WIDTH_SIZE);
		ofnode_read_u32_array(info_node, LDM_PWM0_PERIOD,
				      priv->st_mtk_ldm_dma_info.au32ld_pwm0_period, LDM_PWM0_PERIOD_SIZE);
		ofnode_read_u32_array(info_node, LDM_PWM0_DUTY,
				      priv->st_mtk_ldm_dma_info.au32ld_pwm0_duty, LDM_PWM0_DUTY_SIZE);
		ofnode_read_u32_array(info_node, LDM_PWM0_SHIFT,
				      priv->st_mtk_ldm_dma_info.au32ld_pwm0_shift, LDM_PWM0_SHIFT_SIZE);
		ofnode_read_u32_array(info_node, LDM_PWM1_PERIOD,
				      priv->st_mtk_ldm_dma_info.au32ld_pwm1_period, LDM_PWM1_PERIOD_SIZE);
		ofnode_read_u32_array(info_node, LDM_PWM1_DUTY,
				      priv->st_mtk_ldm_dma_info.au32ld_pwm1_duty, LDM_PWM1_DUTY_SIZE);
		ofnode_read_u32_array(info_node, LDM_PWM1_SHIFT,
				      priv->st_mtk_ldm_dma_info.au32ld_pwm1_shift, LDM_PWM1_SHIFT_SIZE);
	}
  
	info_node = dev_read_subnode(dev, LDM_LED_MSPI_INFO);
	if(!ofnode_valid(info_node))
	{
		debug("Read subnode (%s) FAIL\n", LDM_LED_MSPI_INFO);
	}
	else
	{
		ofnode_read_u32(info_node, LDM_MSPICLK, &u32Data);
		priv->st_mtk_ldm_mspi_info.u32mspi_clk = u32Data;
		ofnode_read_u32(info_node, LDM_MSPIMODE, &u32Data);
		priv->st_mtk_ldm_mspi_info.u8mspi_mode = (uint8_t)u32Data;
	}

	info_node = dev_read_subnode(dev, LDM_LED_DEVICE_INFO);
	if(!ofnode_valid(info_node))
	{
		debug("Read subnode (%s) FAIL\n", LDM_LED_DEVICE_INFO);
	}
	else
	{
		ofnode_read_u32(info_node, LDM_LEDDEVICE_TYPE, &u32Data);
		priv->st_mtk_ldm_device_info.u8LedDevice_Type = (uint8_t)u32Data;
		ofnode_read_u32_array(info_node, LDM_DEVICE_NUM, priv->st_mtk_ldm_device_info.u32device_num, MSPI_MAX_NUM);
	}

	info_node = dev_read_subnode(dev, LDM_DEVICE_AS3824_INFO);
	if(!ofnode_valid(info_node))
	{
		debug("Read subnode (%s) FAIL\n", LDM_DEVICE_AS3824_INFO);
	}
	else
	{
		ofnode_read_u32(info_node, LDM_DEVICE_AS3824_RSENSE, &u32Data);
		priv->st_ldm_Device_AS3824.u16Rsense = (uint16_t)u32Data;
		ofnode_read_u32_array(info_node, LDM_DEVICE_AS3824_REG_01_TO_0B,
			priv->st_ldm_Device_AS3824.u32AS3824_Reg_0x01_to_0x0B, Reg_0x01_to_0x0B_NUM);
		ofnode_read_u32_array(info_node, LDM_DEVICE_AS3824_REG_0E_TO_15,
			priv->st_ldm_Device_AS3824.u32AS3824_Reg_0x0E_to_0x15, Reg_0x0E_to_0x15_NUM);
		ofnode_read_u32(info_node, LDM_DEVICE_AS3824_REG_67, &u32Data);
		priv->st_ldm_Device_AS3824.u8AS3824_Reg_0x67 = (uint8_t)u32Data;
		ofnode_read_u32(info_node, LDM_DEVICE_AS3824_PWM_DUTY_INIT, &u32Data);
		priv->st_ldm_Device_AS3824.u16AS3824_PWM_Duty_Init = (uint16_t)u32Data;
		ofnode_read_u32_array(info_node, LDM_DEVICE_AS3824_PWM_PHASE_MBR_OFF,
			priv->st_ldm_Device_AS3824.u32AS3824_PWM_Phase_MBR_OFF, LD_MAX_HEIGHT);
		ofnode_read_u32_array(info_node, LDM_DEVICE_AS3824_PWM_PHASE_MBR_ON,
			priv->st_ldm_Device_AS3824.u32AS3824_PWM_Phase_MBR_ON, LD_MAX_HEIGHT);
		ofnode_read_u32_array(info_node, LDM_DEVICE_AS3824_REG_0C_0D_VDAC_MBR_OFF,
			priv->st_ldm_Device_AS3824.u32AS3824_Reg_0x0C_0x0D_VDAC_MBR_OFF, VDAC_BYTE_NUM);
		ofnode_read_u32_array(info_node, LDM_DEVICE_AS3824_REG_0C_0D_VDAC_MBR_ON,
			priv->st_ldm_Device_AS3824.u32AS3824_Reg_0x0C_0x0D_VDAC_MBR_ON, VDAC_BYTE_NUM);
		ofnode_read_u32_array(info_node, LDM_DEVICE_AS3824_REG_61_62_PLL_MULTI,
			priv->st_ldm_Device_AS3824.u32AS3824_Reg_0x61_0x62_PLL_multi, PLL_MULTI_BYTE_NUM);
		ofnode_read_u32(info_node, LDM_DEVICE_AS3824_BDAC_HIGH_LIMIT, &u32Data);
		priv->st_ldm_Device_AS3824.u8AS3824_BDAC_High_Limit = (uint8_t)u32Data;
		ofnode_read_u32(info_node, LDM_DEVICE_AS3824_BDAC_LOW_LIMIT, &u32Data);
		priv->st_ldm_Device_AS3824.u8AS3824_BDAC_Low_Limit = (uint8_t)u32Data;
	}

	info_node = dev_read_subnode(dev, LDM_DEVICE_NT50585_INFO);
	if(!ofnode_valid(info_node))
	{
		debug("Read subnode (%s) FAIL\n", LDM_DEVICE_NT50585_INFO);
	}
	else
	{
		ofnode_read_u32_array(info_node, LDM_DEVICE_NT50585_REG_01_TO_0B,
			priv->st_ldm_Device_NT50585.u32NT50585_Reg_0x01_to_0x0B, Reg_0x01_to_0x0B_NUM);
		ofnode_read_u32_array(info_node, LDM_DEVICE_NT50585_REG_1D_TO_1F,
			priv->st_ldm_Device_NT50585.u32NT50585_Reg_0x1D_to_0x1F, Reg_0x1D_to_0x1F_NUM);
		ofnode_read_u32(info_node, LDM_DEVICE_NT50585_REG_60, &u32Data);
		priv->st_ldm_Device_NT50585.u8NT50585_Reg_0x60 = (uint8_t)u32Data;
		ofnode_read_u32(info_node, LDM_DEVICE_NT50585_REG_68, &u32Data);
		priv->st_ldm_Device_NT50585.u8NT50585_Reg_0x68 = (uint8_t)u32Data;
		ofnode_read_u32(info_node, LDM_DEVICE_NT50585_PWM_DUTY_INIT, &u32Data);
		priv->st_ldm_Device_NT50585.u16NT50585_PWM_Duty_Init = (uint16_t)u32Data;
		ofnode_read_u32_array(info_node, LDM_DEVICE_NT50585_PWM_PHASE_MBR_OFF,
			priv->st_ldm_Device_NT50585.u32NT50585_PWM_Phase_MBR_OFF, LD_MAX_HEIGHT);
		ofnode_read_u32_array(info_node, LDM_DEVICE_NT50585_PWM_PHASE_MBR_ON,
			priv->st_ldm_Device_NT50585.u32NT50585_PWM_Phase_MBR_ON, LD_MAX_HEIGHT);
		ofnode_read_u32_array(info_node, LDM_DEVICE_NT50585_REG_14_15_IDAC_MBR_OFF,
			priv->st_ldm_Device_NT50585.u32NT50585_Reg_0x14_0x15_IDAC_MBR_OFF, VDAC_BYTE_NUM);
		ofnode_read_u32_array(info_node, LDM_DEVICE_NT50585_REG_14_15_IDAC_MBR_ON,
			priv->st_ldm_Device_NT50585.u32NT50585_Reg_0x14_0x15_IDAC_MBR_ON, VDAC_BYTE_NUM);
		ofnode_read_u32_array(info_node, LDM_DEVICE_NT50585_REG_66_67_PLL_MULTI,
			priv->st_ldm_Device_NT50585.u32NT50585_Reg_0x66_0x67_PLL_multi, PLL_MULTI_BYTE_NUM);
		ofnode_read_u32(info_node, LDM_DEVICE_NT50585_BDAC_HIGH_LIMIT, &u32Data);
		priv->st_ldm_Device_NT50585.u16NT50585_BDAC_High_Limit = (uint16_t)u32Data;
		ofnode_read_u32(info_node, LDM_DEVICE_NT50585_BDAC_LOW_LIMIT, &u32Data);
		priv->st_ldm_Device_NT50585.u16NT50585_BDAC_Low_Limit = (uint16_t)u32Data;
	}

	info_node = dev_read_subnode(dev, LDM_DEVICE_IW7039_INFO);
	if(!ofnode_valid(info_node))
	{
		debug("Read subnode (%s) FAIL\n", LDM_DEVICE_IW7039_INFO);
	}
	else
	{
		ofnode_read_u32_array(info_node, LDM_DEVICE_IW7039_REG_00_TO_1F,
			priv->st_ldm_Device_IW7039.u32IW7039_Reg_0x000_to_0x01F, Reg_0x00_to_0x1F_NUM);
		ofnode_read_u32(info_node, LDM_DEVICE_IW7039_PWM_DUTY_INIT, &u32Data);
		priv->st_ldm_Device_IW7039.u16IW7039_PWM_Duty_Init = (uint16_t)u32Data;
		ofnode_read_u32(info_node, LDM_DEVICE_IW7039_ISET_MBR_OFF, &u32Data);
		priv->st_ldm_Device_IW7039.u16IW7039_ISET_MBR_OFF = (uint16_t)u32Data;
		ofnode_read_u32(info_node, LDM_DEVICE_IW7039_ISET_MBR_ON, &u32Data);
		priv->st_ldm_Device_IW7039.u16IW7039_ISET_MBR_ON = (uint16_t)u32Data;
		ofnode_read_u32_array(info_node, LDM_DEVICE_IW7039_PWM_PHASE_MBR_OFF,
			priv->st_ldm_Device_IW7039.u32IW7039_PWM_Phase_MBR_OFF, LD_MAX_HEIGHT);
		ofnode_read_u32_array(info_node, LDM_DEVICE_IW7039_PWM_PHASE_MBR_ON,
			priv->st_ldm_Device_IW7039.u32IW7039_PWM_Phase_MBR_ON, LD_MAX_HEIGHT);
		ofnode_read_u32(info_node, LDM_DEVICE_NT50585_ISET_HIGH_LIMIT, &u32Data);
		priv->st_ldm_Device_IW7039.u16IW7039_ISET_High_Limit = (uint16_t)u32Data;
		ofnode_read_u32(info_node, LDM_DEVICE_NT50585_ISET_LOW_LIMIT, &u32Data);
		priv->st_ldm_Device_IW7039.u16IW7039_ISET_Low_Limit = (uint16_t)u32Data;
	}

	info_node = dev_read_subnode(dev, LDM_DEVICE_MBI6353_INFO);
	if(!ofnode_valid(info_node))
	{
		debug("Read subnode (%s) FAIL\n", LDM_DEVICE_MBI6353_INFO);
	}
	else
	{
		ofnode_read_u32_array(info_node, LDM_DEVICE_MBI6353_MASK,
			priv->st_ldm_Device_MBI6353.u32MBI6353_Mask, MBI6353_MASK_NUM * LDM_NUM_2);
		ofnode_read_u32_array(info_node, LDM_DEVICE_MBI6353_CFG,
			priv->st_ldm_Device_MBI6353.u32MBI6353_Config, MBI6353_CFG_NUM * LDM_NUM_2);
		ofnode_read_u32_array(info_node, LDM_DEVICE_MBI6353_CFG_1,
			priv->st_ldm_Device_MBI6353.u32MBI6353_Config_1, MBI6353_CFG_NUM * LDM_NUM_2);
		ofnode_read_u32(info_node, LDM_DEVICE_MBI6353_PWM_Duty_Init, &u32Data);
		priv->st_ldm_Device_MBI6353.u16MBI6353_PWM_Duty_Init = (uint16_t)u32Data;
	}
	return ret;
}

static void _mtk_ldm_LDC_Ctrl(struct udevice *dev)
{
	//int i;
	struct mtk_ldm_priv *priv = dev_get_priv(dev);
	if (priv == NULL) {
		debug("[%s] get device private fail\n", __func__);
		return;
	}
	switch (priv->u8ldm_version) {
		case E_M_LDM_VERSION_1:
		case E_M_LDM_VERSION_2:
		case E_M_LDM_VERSION_3:
		{
			if (priv->u32LDMSupport == E_LDM_SUPPORT_TRUNK_PQU
				|| priv->u32LDMSupport == E_LDM_SUPPORT_TRUNK_FRC) {
				W2BYTEMSK(REG_0004_LDC_BKA3EF, 1, REG_0004_LDC_BKA3EF_REG_LDC_BYPASS);
				W2BYTEMSK(REG_0004_LDC_BKA3EF, 0, REG_0004_LDC_BKA3EF_REG_LDC_PATH_SEL);
				W2BYTEMSK(REG_00DC_LD1_BKA417, 0, REG_00DC_LD1_BKA417_REG_LD_EMPTY_EN);
			} else if (priv->u32LDMSupport == E_LDM_SUPPORT_CUS) {
				if (priv->bLDMCusPathuBoot == true)  {
					W2BYTEMSK(REG_0004_LDC_BKA3EF, 0, REG_0004_LDC_BKA3EF_REG_LDC_BYPASS);
					W2BYTEMSK(REG_0004_LDC_BKA3EF, 1, REG_0004_LDC_BKA3EF_REG_LDC_PATH_SEL);
					W2BYTEMSK(REG_0020_LDC_BKA3EF, 0, REG_0020_LDC_BKA3EF_REG_LDC_XIU2AHB_SEL0);
					W2BYTEMSK(REG_0020_LDC_BKA3EF, 0, REG_0020_LDC_BKA3EF_REG_LDC_XIU2AHB_SEL1);
					W2BYTEMSK(REG_00DC_LD1_BKA417, 1, REG_00DC_LD1_BKA417_REG_LD_EMPTY_EN);
				} else {
					W2BYTEMSK(REG_0004_LDC_BKA3EF, 1, REG_0004_LDC_BKA3EF_REG_LDC_BYPASS);
					W2BYTEMSK(REG_0004_LDC_BKA3EF, 0, REG_0004_LDC_BKA3EF_REG_LDC_PATH_SEL);
					W2BYTEMSK(REG_00DC_LD1_BKA417, 0, REG_00DC_LD1_BKA417_REG_LD_EMPTY_EN);
				}
			} else {
				W2BYTEMSK(REG_0004_LDC_BKA3EF, 1, REG_0004_LDC_BKA3EF_REG_LDC_BYPASS);
				W2BYTEMSK(REG_0004_LDC_BKA3EF, 0, REG_0004_LDC_BKA3EF_REG_LDC_PATH_SEL);
				W2BYTEMSK(REG_00DC_LD1_BKA417, 1, REG_00DC_LD1_BKA417_REG_LD_EMPTY_EN);
			}
			break;
		}
		case E_M_LDM_VERSION_4:
		case E_M_LDM_VERSION_5:
		default:
		{
			if (priv->u32LDMSupport == E_LDM_SUPPORT_TRUNK_PQU
				|| priv->u32LDMSupport == E_LDM_SUPPORT_TRUNK_R2) {
				W2BYTEMSK(REG_00DC_LD1_BKA417, 0, REG_00DC_LD1_BKA417_REG_LD_EMPTY_EN);
			} else {
				W2BYTEMSK(REG_00DC_LD1_BKA417, 1, REG_00DC_LD1_BKA417_REG_LD_EMPTY_EN);
			}
			break;
		}
	}
}

static int mtk_ldm_enable_output(struct udevice *dev, int panel_bpp,
				 const struct display_timing *timing)
{
	struct mtk_ldm_priv *priv = dev_get_priv(dev);

	if (!timing || !dev) {
		UBOOT_ERROR("[%s] param FAIL\n", __func__);
		return -EPERM;
	}
	if (!priv) {
			UBOOT_ERROR("[%s] get device private fail\n", __func__);
			return -EPERM;
	}
	UBOOT_DEBUG("[%s] timing->flags = 0x%x\n", __func__, timing->flags);
	if ((timing->flags & DISPLAY_FLAGS_DE_HIGH) ||
	    (timing->flags & DISPLAY_FLAGS_VSYNC_HIGH)) {
		if (priv->bled_mspi_en)
			_mtk_ldm_leddevice_init(priv);
	}

	if ((timing->flags & DISPLAY_FLAGS_DE_LOW) ||
	    (timing->flags & DISPLAY_FLAGS_VSYNC_LOW)) {
		// todo
	}

	return 0;
}

static void _mtk_ldm_select_pins_to_func(struct udevice *dev)
{
	struct mtk_ldm_priv *priv = dev_get_priv(dev);
	if (priv == NULL) {
		debug("[%s] get device private fail\n", __func__);
		return;
	}
#if CONFIG_PINCTRL
	if (priv->u32LDMSupport == E_LDM_SUPPORT_CUS) {
	pinctrl_select_state(dev, "fun_ldc_in_out_pmux");
	}
#endif
	UBOOT_DEBUG("[%s] done\n", __func__);
}

static void _vsync_like_gpio_probe(struct udevice *dev)
{
	struct mtk_ldm_priv *priv = dev_get_priv(dev);
	int ret;

	ret = gpio_request_by_name(dev, "gpios", 0, &priv->vsync_like_gpio, GPIOD_IS_OUT);

	if (ret)
		UBOOT_DEBUG("[%s] no gpios\n", __func__);
}

static int mtk_ldm_probe(struct udevice *dev)
{
	//read dts table and init clk
	struct mtk_ldm_priv *priv = dev_get_priv(dev);
  	if (priv == NULL) {
		debug("[%s] get device private fail\n", __func__);
		return -ENXIO;
	}
	_mtk_ldm_LDC_Ctrl(dev);
	_mtk_ldm_select_pins_to_func(dev);
	_vsync_like_gpio_probe(dev);
	UBOOT_DEBUG("[%s] done\n", __func__);
	return 0;
}

/*
 *static const struct panel_ops mtk_panel_ops = {
 *	.enable_backlight	= mtk_panel_enable_backlight,
 *	.set_backlight		= mtk_panel_set_backlight,
 *};
 */
static const struct udevice_id ldm_ids[] = {
	{ .compatible = "mediatek,mediatek-ldm" },
	{ },
};

static const struct dm_display_ops mtk_ldm_ops = {
	.enable			= mtk_ldm_enable_output,
};

U_BOOT_DRIVER(mediatek_ldm) = {
	.name = "mtk_ldm",
	.id = UCLASS_DISPLAY,
	.of_match = ldm_ids,
	.ops = &mtk_ldm_ops,
	.ofdata_to_platdata = mtk_ldm_ofdata_to_platdata,
	.probe = mtk_ldm_probe,
	.priv_auto_alloc_size = sizeof(struct mtk_ldm_priv),
};



