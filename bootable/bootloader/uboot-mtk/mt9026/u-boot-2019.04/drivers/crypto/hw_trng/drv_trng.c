// SPDX-License-Identifier: GPL-2.0+
/*
 * MediaTek TRNG Interface driver
 *
 * Copyright (C) 2022 MediaTek Inc.
 * Author: WInniePooh <winniepooh.wu@mediatek.com>
 */

#include <common.h>
#include "u-boot/drv_trng.h"
#include "hal_trng.h"

static unsigned short hal_trng_getstatus(void)
{
	return (TRNG_XIU[TRNG_BASE_ADDR + REG_RNG2RIU_RDY] & RNG_TRIGGER_BIT);
}

static int hal_trng_getrandombyte(unsigned char *poutbuf)
{
	int timeout_count = 0;

	TRNG_XIU[TRNG_BASE_ADDR + REG_RNG2RIU_RDY] = RNG_TRIGGER_BIT;

	while (hal_trng_getstatus() != RNG_TRIGGER_BIT) {
		udelay(TRNG_DELAY);
		timeout_count++;
		if (timeout_count > TRNG_TIMEOUT_VALUE)
			return -1;
	}

	*poutbuf = (TRNG_XIU[TRNG_BASE_ADDR + REG_RNG_OUT] & CHAR_MASK);
	return 0;
}

int HW_TRNG_GetRandomNumer(unsigned char *poutbuf, unsigned int size)
{
	unsigned int remain_size = size;
	unsigned char *ptr;

	if (!poutbuf)
		return -1;
	ptr = poutbuf;

	while (remain_size) {
		if (hal_trng_getrandombyte(ptr) < 0)
			return -1;
		ptr++;
		remain_size--;
	}
	return 0;
}
