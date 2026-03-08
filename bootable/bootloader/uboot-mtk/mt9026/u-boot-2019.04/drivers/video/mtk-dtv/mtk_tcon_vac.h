// SPDX-License-Identifier: (GPL-2.0 OR BSD-3-Clause)
/*
 * MediaTek Panel driver
 *
 * Copyright (c) 2022 MediaTek Inc.
 */

#ifndef _MTK_TCON_VAC_H_
#define _MTK_TCON_VAC_H_

#define SUPPORT_VAC_256                     1
#define ENABLE_VAC_AUTODOWNLOAD            (TRUE)

bool mtk_tcon_vac_setting(struct udevice *dev);
bool mtk_tcon_vac_reg_setting(uint8_t* pu8TconTab, uint16_t u16RegisterCount);

#endif
