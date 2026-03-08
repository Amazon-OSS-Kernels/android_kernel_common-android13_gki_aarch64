// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

#include <common.h>
#include <MsTypes.h>
#include <fs.h>
#include <apiPNL.h>
#include <MsOS.h>
#include <debug_impl.h>
#include <linux/delay.h>
#include "demura_config.h"
#include "apiDemura.h"
#include "utility.h"
#include "dts_parser.h"
#include "demura_impl.h"
#include <linux/io.h>

#ifdef MSOS_TYPE_LINUX_KERNEL
#define mst_atoi(str) simple_strtoul(((str != NULL) ? str : ""), NULL, 0);
#else
#define mst_atoi(str) strtoul(((str != NULL) ? str : ""), NULL, 0);
#endif

//-------------------------------------------------------------------------------------------------
//  Global Variables
//-------------------------------------------------------------------------------------------------
static Demura_Panel_Data g_stpnlinfo = {0};
static MS_U8 g_u8vendor = 0;


static void mtk_demura_version(void);
//--------------------------------------------------------------------------------------------------
// Local Variables
//--------------------------------------------------------------------------------------------------
static void mtk_demura_version(void)
{
    u32 pnl_lib_version = 0xFFFFFFFF;
    char *ptr = env_get(DEMURA_ENV_VERSION);

    if (parse_dt(DEMURA_VIDEO_OUT_NODE, integer_dt_parser,
        (void *)&pnl_lib_version, DEMURA_PNL_KEYWORD) < 0)
    {
        pnl_lib_version = 0xFFFFFFFF;
    }
    else
    {
        if (ptr)
        {
            pnl_lib_version = (u32)mst_atoi(ptr);
        }
        MApi_MsDemura_SetVersion(pnl_lib_version);
    }

    UBOOT_TRACE("pnl_lib_version: %u\n", pnl_lib_version);
}

static MS_U16 _mtk_demura_check_pnlh(unsigned short h)
{
    MS_U16 tmp = 0;
    char *ptr = env_get(DEMURA_ENV_PNL_H);
    if (ptr == NULL)
    {
        return h;
    }
    UBOOT_TRACE(" force h %s \n", ptr);
    tmp = (MS_U16)mst_atoi(ptr);
    return tmp;
}

static MS_U16 _mtk_demura_check_pnlv(unsigned short v)
{
    MS_U16 tmp = 0;
    char *ptr = env_get(DEMURA_ENV_PNL_V);
    if (ptr == NULL)
    {
        return v;
    }
    tmp = (MS_U16)mst_atoi(ptr);
    UBOOT_TRACE(" force v %s \n", ptr);
    return tmp;

}

static u8 _mtk_demura_get_envar(void)
{
    u8 u8En = DEMURA_ENV_ENABLE_INVALID;
    char *ptr = env_get(DEMURA_ENV_ENABLE);
    if (ptr)
    {
        u8En = (u8)mst_atoi(ptr);
    }
    return u8En;
}

static MS_BOOL _mtk_demura_check_envar(bool bon)
{
    u8 u8En = _mtk_demura_get_envar();
    if (u8En == DEMURA_ENV_ENABLE_INVALID)
    {
        return (bon == true)? TRUE: FALSE;
    }
    UBOOT_TRACE(" force %d \n", u8En);
    return (u8En > 0)? TRUE: FALSE;
}

static u8 _mtk_demura_get_env_vendor(u8 panel_vendor)
{
    u8 u8vendor = E_DEMURA_MULTI_NOT;
    char *ptr = env_get(DEMURA_ENV_VENDORID);

    if (ptr)
    {
        u8vendor = (u8)mst_atoi(ptr);
        UBOOT_INFO("env vendor = %d\n", u8vendor);
    }

    if (u8vendor)
    {
        return u8vendor;
    }
    else
    {
        return panel_vendor;
    }
}

bool mtk_demura_process(void)
{
	UBOOT_TRACE("IN\n");
#if defined(CONFIG_DEMURA_VENDOR_MULTI)
    if(!MApi_Demura_Init(g_stpnlinfo, (EN_DEMURA_MULTI_VENDOR)g_u8vendor))
    {
        UBOOT_ERROR("MApi_Demura_Init fail\n");
        return false;
    }
#else
    if(!MApi_Demura_Init(g_stpnlinfo))
    {
        UBOOT_ERROR("MApi_Demura_Init fail\n");
        return false;
    }
#endif
    UBOOT_TRACE("OK\n");
    return true;
}

// convert EN_DEMURA_BIN_ORDER (demura_impl.h)
// to EN_DEMURA_API_BIN_ACT (apiDemura.h)
MS_U8 mtk_demura_get_order(MS_U8 order)
{
    if (order == E_DEMURA_BIN_ORDER_FIRST)
    {
        return E_DEMURA_API_BIN_ACT_FIRST;
    }
    else if (order == E_DEMURA_BIN_ORDER_LAST)
    {
        return E_DEMURA_API_BIN_ACT_LAST;
    }
    else if (order == E_DEMURA_BIN_ORDER_NO_DLG)
    {
        return E_DEMURA_API_BIN_ACT_NO_DLG;
    }

    return E_DEMURA_API_BIN_ACT_NO_DLG;
}

void mtk_demura_init(DemuraImpl_Panel_Data panel_data, Demura_Panel_Vendor panel_vendor, MS_U8 type)
{
	UBOOT_TRACE("IN\n");
	UBOOT_TRACE("wxh:%dx%d,vendor:%d,tcon:%d,tconless:%d,on:%d\n",
	    panel_data.u16PanelWidth, panel_data.u16PanelHeight, panel_vendor,
        panel_data.btcon, panel_data.bpanelless, panel_data.bon);

    g_stpnlinfo.u16PanelWidth = _mtk_demura_check_pnlh(panel_data.u16PanelWidth);
    g_stpnlinfo.u16PanelHeight = _mtk_demura_check_pnlv(panel_data.u16PanelHeight);
    g_stpnlinfo.bOn = _mtk_demura_check_envar(panel_data.bon);

    g_u8vendor = _mtk_demura_get_env_vendor(panel_vendor);

    mtk_demura_version();

    MApi_MsDemura_SetPnlInfo(g_stpnlinfo.u16PanelWidth, g_stpnlinfo.u16PanelHeight);

    if ((g_u8vendor == E_DEMURA_MULTI_NOT) || (g_u8vendor == E_DEMURA_MULTI_MAX))
    {
        UBOOT_TRACE("demura disable due to invalid vendor: %u\n", g_u8vendor);
        return;
    }

    MApi_MsDemura_SetBinType(type);

    MApi_MsDemura_SetAct(mtk_demura_get_order(panel_data.u8DemuraBinOrder));

    if (_mtk_demura_get_envar() == DEMURA_ENV_ENABLE_TRUE)
    {
        mtk_demura_process();
        return;
    }

    if (panel_data.bpanelless)
    {
        UBOOT_TRACE("demura disable due to paneless: %d\n", panel_data.bpanelless);
        return;
    }

    if (!panel_data.btcon)
    {
        UBOOT_TRACE("demura disable due to tcon:%d\n", panel_data.btcon);
        return;
    }

    mtk_demura_process();
	UBOOT_TRACE("OUT\n");
}

void mtk_demura_on(void)
{
#ifdef CONFIG_DEMURA_MSTAR
	UBOOT_DEBUG("Enable Mstar-Demura IP\n");
	MApi_MsDemura_Enable(TRUE);
#elif defined(CONFIG_DEMURA_FCIC)
	UBOOT_DEBUG("Enable FCIC-Demura IP\n");
	MApi_FCIC_Enable(TRUE);
#endif
}

void mtk_demura_off(void)
{
#ifdef CONFIG_DEMURA_MSTAR
    MApi_MsDemura_Enable(FALSE);
#elif defined(CONFIG_DEMURA_FCIC)
	MApi_FCIC_Enable(FALSE);
#endif

}



