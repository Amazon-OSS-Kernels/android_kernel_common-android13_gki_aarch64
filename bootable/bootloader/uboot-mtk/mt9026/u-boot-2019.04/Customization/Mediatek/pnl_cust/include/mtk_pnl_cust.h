/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2017 Google, Inc
 * Written by MediaTek
 */
#ifndef _MTK_TV_PNL_H
#define _MTK_TV_PNL_H

#include <panel_impl.h>

typedef enum
{
    EN_AUTOPGAMMA_TYPE_NULL = 0,
    EN_AUTOPGAMMA_H_K_C_TYPE,     /* for H_K_C panel    spi flash  offset 0x00000  size ?  bytes*/
    EN_AUTOPGAMMA_CSOT_TYPE1,   /* for CSOT panel   spi flash  offset 0xFF000  size 42 bytes */
    EN_AUTOPGAMMA_CSOT_TYPE2,   /* for CSOT panel   spi flash  offset 0xFE000  size 42 bytes */
    EN_AUTOPGAMMA_CSOT_TYPE3,   /* for CSOT panel   EEPROM     offset 0x00F00  size 48 bytes */
    EN_AUTOPGAMMA_CSOT_TYPE4,   /* for CSOT 120Hz   spi flash  offset 0xFE000  size 42 bytes update VCOM1*/
    EN_AUTOPGAMMA_CSOT_TYPE5,   /* for CSOT  60Hz   spi flash  offset 0xFE000  size 42 bytes update GAMMA/VCOM1/VCOM2*/
    EN_AUTOPGAMMA_CSOT_TYPE6,   /* for CSOT 120Hz   spi flash  offset 0xFE000  size 42 bytes update GAMMA/VCOM1/VCOM2*/
    EN_AUTOPGAMMA_CSOT_TYPE7,   /* for CSOT 120Hz   spi flash  offset 0xFE000  size 42 bytes update GAMMA/VCOM1/VCOM2*/
    EN_AUTOPGAMMA_CHOT_TYPE,   /* for CHOT panel   spi flash  offset 0xFE000  size 25 bytes update GAMMA/VCOM1*/
    EN_AUTOPGAMMA_H_K_C_TYPE2,     /* for H_K_C panel    spi flash  offset 0x00000  size 32  bytes update GAMMA/VCOM*/
    EN_AUTOPGAMMA_H_K_C_TYPE3,     /* for H_K_C panel    spi flash  offset 0x00000  size 31  bytes update GAMMA/VCOM*/
    EN_AUTOPGAMMA_TYPE_MAX,
} EN_AUTOPGAMMA_TYPE;

typedef enum
{
    EN_AUTO_P2P_CMD_TYPE_NULL = 0,
    EN_AUTO_P2P_CMD_TYPE_CSOT_GAMMA,
    EN_AUTO_P2P_CMD_TYPE_H_K_C_GAMMA,
    EN_AUTO_P2P_CMD_TYPE_USIT_GAMMA,
    EN_AUTO_P2P_CMD_TYPE_MAX,
} EN_AUTO_P2P_CMD_TYPE;

typedef struct
{
    MS_U16 u16SlaveIDAddr;
    MS_U8  u8AddrCnt;
    MS_U8  pu8Offset[2];
    MS_U16 u8PageSize; // max 2 bytes for many years !!
} ST_EEPROM_PARA;


int mtk_pnl_cust_settings_befor_vcc(st_multi_cust_ic_info *multi_cust_ic);
int mtk_pnl_cust_settings_vcc_ontiming1(st_multi_cust_ic_info *multi_cust_ic);
int mtk_pnl_cust_settings_ontiming1_data(st_multi_cust_ic_info *multi_cust_ic);
int mtk_pnl_cust_settings_data_ontiming2(st_multi_cust_ic_info *multi_cust_ic);
int mtk_pnl_cust_settings_ontiming2_backlight(st_multi_cust_ic_info *multi_cust_ic);
int mtk_pnl_cust_settings_after_backlight(st_multi_cust_ic_info *multi_cust_ic);
int mtk_pnl_cust_set_panel_mode(st_cust_tcon_info *tcon_info);
int mtk_pnl_is_dlg_mode(void);
#endif

