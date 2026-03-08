// SPDX-License-Identifier: (GPL-2.0 OR BSD-3-Clause)
/*
 * MediaTek tcon overdrive driver
 *
 * Copyright (c) 2022 MediaTek Inc.
 */

#ifndef _MTK_TCON_OD_H_
#define _MTK_TCON_OD_H_

#define SUPPORT_OVERDRIVE                   1
#define ENABLE_OD_AUTODOWNLOAD             (TRUE)

bool mtk_tcon_od_setting(struct udevice *dev);

#endif
