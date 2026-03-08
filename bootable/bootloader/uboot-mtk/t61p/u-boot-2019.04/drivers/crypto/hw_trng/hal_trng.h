// SPDX-License-Identifier: GPL-2.0+
/*
 * MediaTek TRNG Interface driver
 *
 * Copyright (C) 2022 MediaTek Inc.
 * Author: WinniePooh <winniepooh.wu@mediatek.com>
 */

#ifndef _HAL_TRNG_H_
#define _HAL_TRNG_H_

#include "mtk_mmu.h"

#define CHAR_MASK                           (0xFF)
#define TRNG_XIU                            ((unsigned int *)RIU_PM_BASE)

#define TRNG_DELAY                          (1)
#define TRNG_TIMEOUT_VALUE                  (100)

#define TRNG_BASE_ADDR                      (0x29C400 >> 1)
#define REG_RNG_OUT                         (0x2)
#define REG_RNG2RIU_RDY                     (0x3)

#define RNG_TRIGGER_BIT                     (0x1)

#endif // #ifndef _HAL_TRNG_H_
