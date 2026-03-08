// SPDX-License-Identifier: (GPL-2.0 OR BSD-3-Clause)
/*
 * Copyright (c) 2021 MediaTek Inc.
 */


#include <linux/delay.h>
//#include <linux/mutex.h>
//#include <linux/semaphore.h>
//#include <linux/sched.h>
#include <time.h>

#include "mtk_sti_msos.h"



void reg16_write(REG16 __iomem *offset, __u16 value)
{
	writew(value, offset);
}

__u16 reg16_read(REG16 __iomem *offset)
{
	return readw(offset);
}

void reg8_write(REG16 __iomem *offset, __u8 value)
{
	writeb(value, offset);
}

__u8 reg8_read(REG16 __iomem *offset)
{
	return readb(offset);
}

void reg8_mask_write(REG16 __iomem *offset, __u8 mask, __u8 value)
{
	__u8 temp;

	temp = readb(offset);
	temp &= ~mask;
	temp |= (mask & value);
	writeb(temp, offset);
}


void STI_NJPD_PowerOn(void)
{
    __u16 u16RegValue, u16RegSet;
    REG16 __iomem *base = (void*)CLK_REG_BASE;
    REG16 sw_en_smi2jpd_reg_offset = CLK_SMI2JPD_OFFSET;
    const __u16 sw_en_smi2jpd_reg_bit = 0x8000;
    REG16 clk_njpd2jpd_reg_offset = CLK_NJPD2JPD_OFFSET;
    const __u16 clk_njpd2jpd_reg_bit = 0x0001;

    u16RegValue = reg16_read(base + sw_en_smi2jpd_reg_offset);
    u16RegSet = sw_en_smi2jpd_reg_bit;
    u16RegValue |= u16RegSet;
    reg16_write(base + sw_en_smi2jpd_reg_offset, u16RegValue);

    u16RegValue = reg16_read(base + clk_njpd2jpd_reg_offset);
    u16RegSet = clk_njpd2jpd_reg_bit;
    u16RegValue |= u16RegSet;
    reg16_write(base + clk_njpd2jpd_reg_offset, u16RegValue);

}

/******************************************************************************/
///
///@param value \b IN
///@param value \b OUT
///@return status
/******************************************************************************/
void STI_NJPD_PowerOff(void)
{
    __u16 u16RegValue, u16RegSet;
    REG16 __iomem *base = (void*)CLK_REG_BASE;
    REG16 sw_en_smi2jpd_reg_offset = CLK_SMI2JPD_OFFSET;
    const __u16 sw_en_smi2jpd_reg_bit = 0x8000;
    REG16 clk_njpd2jpd_reg_offset = CLK_NJPD2JPD_OFFSET;
    const __u16 clk_njpd2jpd_reg_bit = 0x0001;

    u16RegValue = reg16_read(base + sw_en_smi2jpd_reg_offset);
    u16RegSet = sw_en_smi2jpd_reg_bit;
    u16RegValue &= ~u16RegSet;
    reg16_write(base + sw_en_smi2jpd_reg_offset, u16RegValue);

    u16RegValue = reg16_read(base + clk_njpd2jpd_reg_offset);
    u16RegSet = clk_njpd2jpd_reg_bit;
    u16RegValue &= ~u16RegSet;
    reg16_write(base + clk_njpd2jpd_reg_offset, u16RegValue);
}

void *STI_MALLOC(__u32 size)
{
	return malloc(size);
}

void STI_FREE(void *x, __u32 size)
{
	free(x);
}

void MsOS_DelayTask(__u32 u32Ms)
{
#if MSOS_TYPE_LINUX_KERNEL
	msleep((u32Ms));
#else
	udelay(u32Ms * 1000UL);
#endif
}

void MsOS_DelayTaskUs(__u32 u32Us)
{
#if MSOS_TYPE_LINUX_KERNEL
	if (u32Us < 10UL)
		udelay(u32Us);
	else if (u32Us < 20UL * 1000UL)
		usleep_range(u32Us, u32Us);
	else
		msleep_interruptible((unsigned int)(u32Us / 1000UL));
#else
	udelay(u32Us);
#endif
}


__u32 MsOS_GetSystemTime(void)
{
/*
	struct timespec ts;

	getrawmonotonic(&ts);
	return ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
*/
	return 1;
}

void MsOS_FlushMemory(void)
{

}
