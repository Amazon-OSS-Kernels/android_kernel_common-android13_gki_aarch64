// SPDX-License-Identifier: GPL-2.0+
/*
 * MediaTek TRNG Interface driver
 *
 * Copyright (C) 2022 MediaTek Inc.
 * Author: WinniePooh <winniepooh.wu@mediatek.com>
 */

#ifndef _DRV_TRNG_H_
#define _DRV_TRNG_H_

#ifdef __cplusplus
extern "C"
{
#endif

//--------------------------------------------------------------------------------------------------
//  Function
//--------------------------------------------------------------------------------------------------
int HW_TRNG_GetRandomNumer(unsigned char *poutbuf, unsigned int size);

#ifdef __cplusplus
}
#endif

#endif // #ifndef _DRV_TRNG_H_
