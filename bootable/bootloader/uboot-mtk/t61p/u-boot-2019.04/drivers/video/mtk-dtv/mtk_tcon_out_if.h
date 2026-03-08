// SPDX-License-Identifier: (GPL-2.0 OR BSD-3-Clause)
/*
 * MediaTek tcon output driver
 *
 * Copyright (c) 2022 MediaTek Inc.
 */

#ifndef _MTK_TCON_OUT_IF_H_
#define _MTK_TCON_OUT_IF_H_

typedef enum
{
    E_APIPNL_TCON_TAB_TYPE_GENERAL,
    E_APIPNL_TCON_TAB_TYPE_GPIO,
    E_APIPNL_TCON_TAB_TYPE_SCALER,
    E_APIPNL_TCON_TAB_TYPE_MOD,
    E_APIPNL_TCON_TAB_TYPE_GAMMA,
    E_APIPNL_TCON_TAB_TYPE_POWER_SEQUENCE_ON,
    E_APIPNL_TCON_TAB_TYPE_POWER_SEQUENCE_OFF,
    E_APIPNL_TCON_TAB_TYPE_PANEL_INFO,
    E_APIPNL_TCON_TAB_TYPE_OVERDRIVER,
    E_APIPNL_TCON_TAB_TYPE_PCID,
    E_APIPNL_TCON_TAB_TYPE_PATCH,
    E_APIPNL_TCON_TAB_TYPE_LINE_OD_TABLE,
    E_APIPNL_TCON_TAB_TYPE_LINE_OD_REG,
    E_APIPNL_TCON_TAB_TYPE_EVA_REG,
    E_APIPNL_TCON_TAB_TYPE_EVA_TABLE,
    E_APIPNL_TCON_TAB_TYPE_COUNT,
    E_APIPNL_TCON_TAB_TYPE_NULL,
}APIPNL_TCON_TAB_TYPE;

bool mtk_tcon_preinit(struct udevice *dev);
bool mtk_tcon_pq_init(struct udevice *dev, struct st_tcon_pq_force_en force_en);
bool mtk_tcon_init(struct udevice *dev);
bool mtk_tcon_enable(struct udevice *dev, bool bEn);

#endif
