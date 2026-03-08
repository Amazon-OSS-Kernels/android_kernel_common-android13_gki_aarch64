// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (c) 2023 MediaTek Inc.
*/

#ifndef _DEMURA_IMPL_HEADER_
#define _DEMURA_IMPL_HEADER_
#include <apiDemura.h>

typedef enum
{
    E_DEMURA_BIN_ORDER_FIRST,
    E_DEMURA_BIN_ORDER_LAST,
    E_DEMURA_BIN_ORDER_NO_DLG,
    E_DEMURA_BIN_ORDER_MAX
}EN_DEMURA_BIN_ORDER;

typedef struct
{
    unsigned short u16PanelWidth;
    unsigned short u16PanelHeight;
    bool btcon; /* true:with mtk tcon false:without mtk tcon */
    bool bpanelless; /* true: paneless */
    bool bon; /* true: demura hw enable*/
    MS_U8 u8DemuraBinOrder; /* the real work demura bin  refer to EN_DEMURA_BIN_ORDER */
}DemuraImpl_Panel_Data;

typedef enum
{
    E_DEMURA_BIN_MAIN,
    E_DEMURA_BIN_DLG,
    E_DEMURA_BIN_MAX,
} EN_DEMURA_BIN_TYPE;

void mtk_demura_init(DemuraImpl_Panel_Data panel_data, Demura_Panel_Vendor panel_vendor, MS_U8 type);
void mtk_demura_on(void);
void mtk_demura_off(void);
bool mtk_demura_process(void);
#endif
