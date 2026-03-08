// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (c) 2023 MediaTek Inc.
 */
//-------------------------------------------------------------------------------------------------
//  Include Files
//-------------------------------------------------------------------------------------------------
#if !defined(MSOS_TYPE_LINUX_KERNEL)
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#else
#include <linux/string.h>
#include <linux/slab.h>
#endif

#include <malloc.h>
#include "utopia.h"
#include "MsTypes.h"
#include "MsCommon.h"
#include "MsVersion.h"
#include "apiGOP.h"
#include "apiGOP_priv.h"
#include "util_symbol.h"
#include "drvGOP.h"
#include "MsOS.h"

#include "drvXC_IOPort.h"
#include "apiXC.h"
#include "apiXC_v2.h"

//-------------------------------------------------------------------------------------------------
//  Local Compiler Options
//-------------------------------------------------------------------------------------------------
#ifndef UTOPIAXP_REMOVE_WRAPPER
#if defined(MSOS_TYPE_LINUX) || defined(MSOS_TYPE_LINUX_KERNEL)
#ifndef GOP_UTOPIA2K
#define GOP_UTOPIA2K
#endif
#endif

#define DUMP_INFO   0UL

#define FPGA_TEST   0UL

//-------------------------------------------------------------------------------------------------
//  Local Defines
//-------------------------------------------------------------------------------------------------
#ifdef MSOS_TYPE_LINUX_KERNEL
#define atoi(str) simple_strtoul(((str != NULL) ? str : ""), NULL, 0)
#endif

#define GetMaxActiveGwinFalse 4UL
#define GetMaxActiveGwinFalse_op 5UL
#define GetMaxActiveGwinFalse_opened 6UL
#define PALETTE_ENTRY_NUM   32UL
#define GWIN_SDRAM_NULL 0x30UL
#define msWarning(c)    do {} while (0)
#define XC_MAIN_WINDOW  0UL

#ifndef UNUSED
#define UNUSED( var ) (void)(var)
#endif

#ifndef GOP_UNUSED
#define GOP_UNUSED(x)    UNUSED_ ## x __attribute__((__unused__))
#endif

//=============================================================
#include "ULog.h"
#include <debug_impl.h>
MS_U32 u32GOPDbgLevel_api = 0;

#ifdef CONFIG_GOP_DEBUG_LEVEL
// Debug Logs, level form low(INFO) to high(FATAL, always show)
// Function information, ex function entry
#define GOP_INFO(x, args...) { UBOOT_INFO(x, ##args); }
// Warning, illegal paramter but can be self fixed in functions
#define GOP_WARN(x, args...) { UBOOT_INFO(x, ##args); }
//  Need debug, illegal paramter.
#define GOP_DBUG(x, args...) { UBOOT_DEBUG(x, ##args); }
// Error, function will be terminated but system not crash
#define GOP_ERR(x, args...) { UBOOT_ERROR(x, ##args); }
// Critical, system crash. (ex. assert)
#define GOP_FATAL(x, args...) { UBOOT_ERROR(x, ##args); }
#else
#define GOP_INFO(x, args...)
// Warning, illegal paramter but can be self fixed in functions
#define GOP_WARN(x, args...)
//  Need debug, illegal paramter.
#define GOP_DBUG(x, args...)
// Error, function will be terminated but system not crash
#define GOP_ERR(x, args...)
// Critical, system crash. (ex. assert)
#define GOP_FATAL(x, args...)
#endif
//=============================================================

#ifdef  MSOS_TYPE_LINUX
#include <assert.h>
#include <unistd.h>
#define GOP_ASSERT(_bool, pri)  if (!(_bool)) {GOP_FATAL("\nAssert in %s,%d\n", __FUNCTION__, __LINE__); (pri); MsOS_DelayTask(100); assert(0);}
#else
#define GOP_ASSERT(_bool, pri)  if (!(_bool)) {GOP_FATAL("\nAssert in %s,%d\n", __FUNCTION__, __LINE__); (pri);};
#endif

#ifdef CONFIG_GOP_UTOPIA10
#define CheckGOPInstanceOpen()
#else
#define CheckGOPInstanceOpen() do{\
    if(pInstantGOP == NULL)\
    {\
        if(UtopiaOpen(MODULE_GOP | GOPDRIVER_BASE, &pInstantGOP, 0, pAttributeGOP) !=  UTOPIA_STATUS_SUCCESS)\
        {\
            GOP_ERR("Open GOP fail\n");\
            return GOP_API_FAIL;\
        }\
    }\
}while(0)
#endif

#endif

#ifndef UTOPIAXP_REMOVE_WRAPPER

#define GOP_ENTRY()                 do{ \
                                        APIGOP_ASSERT(bInit, GOP_FATAL("\n [Function   %s] [%d] Need GOP Driver Init First !!!!!!!\n",__FUNCTION__,__LINE__));\
                                    }while(0);
//{if(TRUE != MDrv_GOP_GWIN_BeginDraw()){GOP_ASSERT(FALSE, printf("\nError: Obtain GOP mutex fail\n"));}}
#define GOP_RETURN(_ret)             do{\
                                        return _ret;\
                                     }while(0);//{if(TRUE == MDrv_GOP_GWIN_EndDraw()){return _ret;}else{GOP_ASSERT(FALSE, printf("\nError: Release GOP mutex fail\n"));return _ret;}}

#if defined(MSOS_TYPE_LINUX_KERNEL)
#include <linux/slab.h>
#define free kfree
#define malloc(size) kmalloc((size), GFP_KERNEL)
#endif

#define GOP_CHK_VERSION(u32InputVer,u32InputLength,u32LibVer,u32LibLength,u32CopiedLength)     do{  \
    if(u32InputVer < 1)  \
    {  \
        GOP_ERR("[Err][%s][%d]: please check your u32Version, it should not set to 0!!\n",__func__,__LINE__);  \
        u32CopiedLength = 0;  \
    }  \
    else  \
    {  \
        if(u32InputLength > u32LibLength)  \
        {  \
            u32CopiedLength = u32LibLength;  \
        }  \
        else  \
        {  \
            u32CopiedLength = u32InputLength;  \
        }  \
    }  \
    if(u32InputVer != u32LibVer)  \
    {  \
        GOP_WARN("[Warning][%s][%d]: Your ver = %tu length = %tu,GOP lib ver = %tu lengtg = %td\n",__func__,__LINE__,(ptrdiff_t)u32InputVer,(ptrdiff_t)u32InputLength,(ptrdiff_t)u32LibVer,(ptrdiff_t)u32LibLength);  \
    }  \
    }while(0);

//-------------------------------------------------------------------------------------------------
//  Local Structures
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Global Variables
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Local Variables
//-------------------------------------------------------------------------------------------------
#ifndef CONFIG_GOP_REDUCE_CODESIZE_FOR_MBOOT
//GOP Lib Version Control
static MSIF_Version _api_gop_version =
{
    .DDI = { GOP_API_VERSION },
};
#endif

static MS_U32 (*fpSetFBFmt)(MS_U16 ,MS_PHY , MS_U16 ) = NULL;
static MS_BOOL (*fpXCIsInterlace)(void) = NULL;
static MS_U16 (*fpXCGetCapHStart)(void) = NULL;
static void (*fpXCReduceBWForOSD)(MS_U8 , MS_BOOL) = NULL;
static void (*fpEventNotify)(MS_U32 , void* ) = NULL;
static MS_BOOL (*fpXCSetDwinInfo)(MS_U16 cmd,MS_U32 *arg,MS_U16 size) = NULL;
static MS_BOOL bInit = FALSE;
MS_U32 u32TransColor = 0x00000000;

//-------------------------------------------------------------------------------------------------
//  Utopia 2.0
//-------------------------------------------------------------------------------------------------

void* pInstantGOP = NULL;
void* pAttributeGOP = NULL;

//-------------------------------------------------------------------------------------------------
//  Debug Functions
//-------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//  Local Functions
//------------------------------------------------------------------------------
static MS_U32 GOP_CalcPitch(MS_U8 fbFmt, MS_U16 width)
{
    MS_U16 pitch=0;

    switch ( fbFmt )
    {
    case E_MS_FMT_I1 :
        pitch = (width) >> 3;
        break;
    case E_MS_FMT_I2 :
        pitch = (width<<1) >> 3;
        break;
    case E_MS_FMT_I4 :
        pitch = (width<<2) >> 3;
        break;
    case E_MS_FMT_I8 :
        pitch = width;
        break;
    case E_MS_FMT_RGB565 :
    case E_MS_FMT_BGR565 :
    case E_MS_FMT_ARGB1555 :
    case E_MS_FMT_ABGR1555 :
    case E_MS_FMT_BGRA5551 :
    case E_MS_FMT_RGBA5551 :
    case E_MS_FMT_ARGB4444 :
    case E_MS_FMT_RGBA4444 :
    case E_MS_FMT_ABGR4444 :
    case E_MS_FMT_BGRA4444 :
    case E_MS_FMT_1ABFgBg12355:
    case E_MS_FMT_FaBaFgBg2266:
    case E_MS_FMT_YUV422:
    case E_MS_FMT_ARGB1555_DST :
        pitch = width << 1;
        break;
    case E_MS_FMT_AYUV8888 :
    case E_MS_FMT_ARGB8888 :
    case E_MS_FMT_RGBA8888 :
    case E_MS_FMT_BGRA8888:
    case E_MS_FMT_ABGR8888 :
        pitch = width << 2;
        break;
    default :
        //print err
        pitch = 0;
        break;
    }
    return pitch;
}
#ifndef CONFIG_GOP_REDUCE_CODESIZE_FOR_MBOOT
static MS_U16 GOP_GetBPP(EN_GOP_COLOR_TYPE fbFmt)
{
    MS_U16 bpp=0;

    switch ( fbFmt )
    {
    case E_GOP_COLOR_RGB555_BLINK :
    case E_GOP_COLOR_RGB565 :
    case E_GOP_COLOR_BGR565 :
    case E_GOP_COLOR_ARGB1555:
    case E_GOP_COLOR_ABGR1555:
    case E_GOP_COLOR_ARGB4444 :
    case E_GOP_COLOR_RGBA4444 :
    case E_GOP_COLOR_ABGR4444 :
    case E_GOP_COLOR_BGRA4444 :
    case E_GOP_COLOR_RGB555YUV422:
    case E_GOP_COLOR_YUV422:
    case E_GOP_COLOR_2266:
    case E_GOP_COLOR_RGBA5551:
    case E_GOP_COLOR_BGRA5551:
        bpp = 16;
        break;
    case E_GOP_COLOR_AYUV8888 :
    case E_GOP_COLOR_ARGB8888 :
    case E_GOP_COLOR_ABGR8888 :
    case E_GOP_COLOR_RGBA8888 :
    case E_GOP_COLOR_BGRA8888 :
        bpp = 32;
        break;

    case E_GOP_COLOR_I8 :
        bpp = 8;
        break;
    default :
        //print err
        //__ASSERT(0);
        bpp = FB_FMT_AS_DEFAULT;
        break;
    }
    return bpp;

}
#endif

#ifndef CONFIG_GOP_REDUCE_CODESIZE_FOR_MBOOT

E_GOP_API_Result MApi_GOP_GWIN_ResetPool(void)
{
    MS_U32 u32Index;
    static MS_BOOL bPoolReseted = FALSE;

    if (bPoolReseted)
    {
        return GOP_API_SUCCESS;
    }
    else
    {
        bPoolReseted = TRUE;
    }
#ifdef CONFIG_GOP_UTOPIA10
UNUSED(u32Index);
#else
    CheckGOPInstanceOpen();
    if (UtopiaIoctl(pInstantGOP,MAPI_CMD_GOP_RESETPOOL,&u32Index) != UTOPIA_STATUS_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        return GOP_API_FAIL;
    }
#endif
    return GOP_API_SUCCESS;

}

E_GOP_API_Result Mapi_GOP_GWIN_ResetGOP(MS_U32 u32Gop)
{

    MS_U32 i;
#ifdef CONFIG_GOP_UTOPIA10
    if(Ioctl_GOP_SetProperty(NULL,E_GOP_RESOURCE,u32Gop,(MS_U32*)&i,sizeof(int)) != GOP_API_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN(GOP_API_FAIL);
    }
#else
    GOP_SET_PROPERTY_PARAM ioctl_info;
    memset(&ioctl_info, 0x0, sizeof(GOP_SET_PROPERTY_PARAM));

    CheckGOPInstanceOpen();
    ioctl_info.en_pro   = E_GOP_RESOURCE;
    ioctl_info.gop_idx  = u32Gop;
    ioctl_info.pSetting = (void*)&i;
    ioctl_info.u32Size  = sizeof(MS_U32);

    if(UtopiaIoctl(pInstantGOP,MAPI_CMD_GOP_SET_PROPERTY,&ioctl_info)!= UTOPIA_STATUS_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        return GOP_API_FAIL;
    }
#endif
    return GOP_API_SUCCESS;
}

/********************************************************************************/
/// Open API function:
///     Read GWIN information from GOP registers
/// @param u8win \b IN  GWINID
/// @param pinfo \b OUT buffer to store GWIN information
/// @return GOP_API_SUCCESS - Success
/// @return GOP_API_FAIL - Failure
/********************************************************************************/
E_GOP_API_Result MApi_GOP_GWIN_GetWinInfo(MS_U8 u8win, GOP_GwinInfo* pinfo)
{
    GOP_ENTRY();
    GOP_BUFFER_INFO win_info;
    memset(&win_info, 0x0, sizeof(GOP_BUFFER_INFO));

#ifdef CONFIG_GOP_UTOPIA10
    if(Ioctl_GOP_GetWinInfo(NULL,u8win,&win_info) != GOP_API_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN(GOP_API_FAIL);
    }
#else
    GOP_GWIN_SETWININFO_PARAM ioctl_info;
    memset(&ioctl_info, 0x0, sizeof(GOP_GWIN_SETWININFO_PARAM));

    ioctl_info.GwinId = u8win;
    ioctl_info.pinfo = (MS_U32*)&win_info;
    ioctl_info.u32Size = sizeof(GOP_BUFFER_INFO);

    if(UtopiaIoctl(pInstantGOP,MAPI_CMD_GOP_GWIN_GET_WININFO,&ioctl_info)!= UTOPIA_STATUS_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN( GOP_API_FAIL);
    }
#endif

    pinfo->u32DRAMRBlkStart   =  win_info.addr;
    pinfo->u16DispHPixelStart =  win_info.disp_rect.x ;
    pinfo->u16DispVPixelStart =  win_info.disp_rect.y;
    pinfo->u16DispHPixelEnd   =  win_info.disp_rect.x + win_info.disp_rect.w;
    pinfo->u16DispVPixelEnd   =  win_info.disp_rect.y + win_info.disp_rect.h;
    pinfo->u16RBlkHPixSize    =  win_info.width;
    pinfo->u16RBlkVPixSize    =  win_info.height;
    pinfo->clrType            =  (EN_GOP_COLOR_TYPE)win_info.fbFmt;
    pinfo->u16RBlkHRblkSize   =  win_info.pitch;


    GOP_RETURN(GOP_API_SUCCESS);

}
#endif
MS_U32 _GOP_RegisterAllCBFunc(void);
#ifndef CONFIG_GOP_REDUCE_CODESIZE_FOR_MBOOT

/********************************************************************************/
/// Set transparent color (ARGB domain). Note that this funcion just set transparent color to gop hareware but
/// does not enable gop transparent function yet.
/// @param clr \b IN: transparent color
/// @param mask \b IN: alpha value
/// @return GOP_API_SUCCESS - Success
/********************************************************************************/
E_GOP_API_Result MApi_GOP_GWIN_SetTransClr_8888_EX(MS_U8 u8GOP,MS_U32 clr, MS_U32 mask)
{
    GOP_TRANSCOLOR trans_set;

    GOP_ENTRY();

    trans_set.bEn = TRUE;
    trans_set.color = clr;
    trans_set.fmt = GOPTRANSCLR_FMT2;
    trans_set.transclr_property = EN_TRANSCLR_SET_COLOR;

    u32TransColor = clr;
#ifdef CONFIG_GOP_UTOPIA10
    if(Ioctl_GOP_SetProperty(NULL,E_GOP_TRANSCOLOR,u8GOP,(MS_U32*)&trans_set,sizeof(GOP_TRANSCOLOR)) != GOP_API_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN(GOP_API_FAIL);
    }
#else
    GOP_SET_PROPERTY_PARAM ioctl_info;

    ioctl_info.en_pro = E_GOP_TRANSCOLOR;
    ioctl_info.gop_idx = u8GOP;
    ioctl_info.pSetting = (void*)&trans_set;
    ioctl_info.u32Size = sizeof(GOP_TRANSCOLOR);

    if (UtopiaIoctl(pInstantGOP,MAPI_CMD_GOP_SET_PROPERTY,(void*)&ioctl_info) != UTOPIA_STATUS_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN(GOP_API_FAIL);
    }
#endif
    GOP_RETURN(GOP_API_SUCCESS);
}
E_GOP_API_Result MApi_GOP_GWIN_SetTransClr_8888(MS_U32 clr, MS_U32 mask)
{
    MS_U8 u8GOP;
    E_GOP_API_Result ret;

    u8GOP = MApi_GOP_GWIN_GetCurrentGOP();
    ret = MApi_GOP_GWIN_SetTransClr_8888_EX(u8GOP,clr,mask);
    return ret;
}

#endif
E_GOP_API_Result MApi_GOP_GWIN_EnableT3DMode(MS_BOOL bEnable)
{
	MS_U32  u32Ena;

    GOP_ENTRY();

	u32Ena = (MS_U32)bEnable;
#ifdef CONFIG_GOP_UTOPIA10
    if(Ioctl_GOP_MISC(NULL,E_GOP_MISC_ENABLE_T3D, &u32Ena,sizeof(MS_BOOL)) != GOP_API_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN(GOP_API_FAIL);
    }
#else
    GOP_MISC_PARAM ioctl_info;

    ioctl_info.misc_type = E_GOP_MISC_ENABLE_T3D;
    ioctl_info.pMISC = (void*)&u32Ena;
    ioctl_info.u32Size  = sizeof(MS_BOOL);

    if(UtopiaIoctl(pInstantGOP,MAPI_CMD_GOP_MISC,&ioctl_info)!= UTOPIA_STATUS_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN( GOP_API_FAIL);
    }
#endif
    GOP_RETURN(GOP_API_SUCCESS);
}
#ifndef CONFIG_GOP_REDUCE_CODESIZE_FOR_MBOOT

/********************************************************************************/
/// Set clock for gop dwin (for dwin source is OP)
/// @return GOP_API_SUCCESS - Success
/********************************************************************************/
E_GOP_API_Result MApi_GOP_SetClkForCapture(void)
{
    MS_U32  value = 0;
    GOP_ENTRY();

#ifdef CONFIG_GOP_UTOPIA10
    if(Ioctl_GOP_MISC(NULL,E_GOP_MISC_SET_CAPTURE_CLK,(MS_U32*)&value,sizeof(int)) != GOP_API_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN(GOP_API_FAIL);
    }
#else
    GOP_MISC_PARAM ioctl_info;

    ioctl_info.misc_type = E_GOP_MISC_SET_CAPTURE_CLK;
    ioctl_info.pMISC = (MS_U32*)&value;
    ioctl_info.u32Size  = sizeof(MS_U32);

    if(UtopiaIoctl(pInstantGOP,MAPI_CMD_GOP_MISC,&ioctl_info)!= UTOPIA_STATUS_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN( GOP_API_FAIL);
    }
#endif
    GOP_RETURN(GOP_API_SUCCESS);
}
#endif

/********************************************************************************/
/// Set gop output color type
/// @param type \b IN: gop output color type
///   - # GOPOUT_RGB => RGB mode
///   - # GOPOUT_YUV => YUV mode
/// @return GOP_API_SUCCESS - Success
/********************************************************************************/
E_GOP_API_Result MApi_GOP_GWIN_OutputColor_EX(MS_U8 u8GOP,EN_GOP_OUTPUT_COLOR type)
{
    EN_GOP_OUTPUT_COLOR  output;

    GOP_ENTRY();

    output = type;
#ifdef CONFIG_GOP_UTOPIA10
    if(Ioctl_GOP_SetProperty(NULL,E_GOP_OUTPUT_COLOR,u8GOP,(MS_U32*)&output,sizeof(EN_GOP_OUTPUT_COLOR)) != GOP_API_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN(GOP_API_FAIL);
    }

#else
    GOP_SET_PROPERTY_PARAM ioctl_info;

    ioctl_info.en_pro   = E_GOP_OUTPUT_COLOR;
    ioctl_info.gop_idx  = u8GOP;
    ioctl_info.pSetting = (void*)&output;
    ioctl_info.u32Size  = sizeof(EN_GOP_OUTPUT_COLOR);

    if(UtopiaIoctl(pInstantGOP,MAPI_CMD_GOP_SET_PROPERTY,&ioctl_info)!= UTOPIA_STATUS_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN( GOP_API_FAIL);
    }
#endif
    GOP_RETURN(GOP_API_SUCCESS);
}
E_GOP_API_Result MApi_GOP_GWIN_OutputColor(EN_GOP_OUTPUT_COLOR type)
{
    MS_U8 u8GOP;
    E_GOP_API_Result ret;

    u8GOP = MApi_GOP_GWIN_GetCurrentGOP();
    ret = MApi_GOP_GWIN_OutputColor_EX(u8GOP,type);
    return ret;
}
#ifndef CONFIG_GOP_REDUCE_CODESIZE_FOR_MBOOT

/******************************************************************************/
/// Enable/Disable gop alpha inverse
/// @param bEnable \b IN: TRUE or FALSE
/// @return GOP_API_SUCCESS - Success
/******************************************************************************/
E_GOP_API_Result MApi_GOP_GWIN_SetAlphaInverse_EX(MS_U8 u8GOP,MS_BOOL bEnable)
{
	MS_U32 u32AlphaInv;

    GOP_ENTRY();

    u32AlphaInv = (MS_U32)bEnable;
#ifdef CONFIG_GOP_UTOPIA10
    if(Ioctl_GOP_SetProperty(NULL,E_GOP_ALPHAINVERSE,u8GOP,&u32AlphaInv,sizeof(MS_BOOL)) != GOP_API_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN(GOP_API_FAIL);
    }
#else
    GOP_SET_PROPERTY_PARAM ioctl_info;

    memset(&ioctl_info, 0x0, sizeof(GOP_SET_PROPERTY_PARAM));

    ioctl_info.en_pro   = E_GOP_ALPHAINVERSE;
    ioctl_info.gop_idx  = u8GOP;
    ioctl_info.pSetting = (void*)&u32AlphaInv;
    ioctl_info.u32Size  = sizeof(MS_BOOL);

    if(UtopiaIoctl(pInstantGOP,MAPI_CMD_GOP_SET_PROPERTY,&ioctl_info)!= UTOPIA_STATUS_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN( GOP_API_FAIL);
    }
#endif
    GOP_RETURN(GOP_API_SUCCESS);


}

E_GOP_API_Result MApi_GOP_GWIN_SetAlphaInverse(MS_BOOL bEnable)
{
    MS_U8 u8GOP;
    E_GOP_API_Result ret;

    u8GOP = MApi_GOP_GWIN_GetCurrentGOP();
    ret = MApi_GOP_GWIN_SetAlphaInverse_EX(u8GOP,bEnable);
    return ret;
}

/********************************************************************************/
/// Set GOP bank force write mode for update register. When enable bank force write mode, update gop num registers action will directly
/// take effect (do not wait next v-sync to update gop register!).
/// @param bEnable \b IN: TRUE/FALSE
/// @return GOP_API_SUCCESS - Success
/********************************************************************************/
E_GOP_API_Result MApi_GOP_GWIN_SetBnkForceWrite(MS_U8 u8GOP, MS_BOOL bEnable)
{
	MS_U32 u32Ena;

	u32Ena = (MS_U32)bEnable;

#ifdef CONFIG_GOP_UTOPIA10
    if(Ioctl_GOP_SetProperty(NULL,E_GOP_BANK_FORCE_WRITE,u8GOP,&u32Ena,sizeof(MS_BOOL)) != GOP_API_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN(GOP_API_FAIL);
    }
#else
    GOP_SET_PROPERTY_PARAM ioctl_info;

    memset(&ioctl_info, 0x0, sizeof(GOP_SET_PROPERTY_PARAM));

    CheckGOPInstanceOpen();

    ioctl_info.en_pro   = E_GOP_BANK_FORCE_WRITE;
    ioctl_info.gop_idx  = u8GOP;
    ioctl_info.pSetting = (void*)&u32Ena;
    ioctl_info.u32Size  = sizeof(MS_BOOL);

    if(UtopiaIoctl(pInstantGOP,MAPI_CMD_GOP_SET_PROPERTY,&ioctl_info)!= UTOPIA_STATUS_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        return GOP_API_FAIL;
    }
#endif

    return GOP_API_SUCCESS;
}

#endif
/********************************************************************************/
/// Set GOP force write mode for update register. When enable force write mode, all update gop registers action will directly
/// take effect (do not wait next v-sync to update gop register!).
/// @param bEnable \b IN: TRUE/FALSE
/// @return GOP_API_SUCCESS - Success
/********************************************************************************/
E_GOP_API_Result MApi_GOP_GWIN_SetForceWrite(MS_BOOL bEnable)
{
	MS_U32 u32Ena;

	u32Ena = (MS_U32)bEnable;
#ifdef CONFIG_GOP_UTOPIA10
    if(Ioctl_GOP_SetProperty(NULL,E_GOP_FORCE_WRITE,0x0,&u32Ena,sizeof(MS_BOOL)) != GOP_API_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN(GOP_API_FAIL);
    }
#else
    GOP_SET_PROPERTY_PARAM ioctl_info;

    CheckGOPInstanceOpen();

    memset(&ioctl_info, 0x0, sizeof(GOP_SET_PROPERTY_PARAM));

    ioctl_info.en_pro   = E_GOP_FORCE_WRITE;
    ioctl_info.gop_idx  = 0x0;
    ioctl_info.pSetting = (void*)&u32Ena;
    ioctl_info.u32Size  = sizeof(MS_BOOL);

    if(UtopiaIoctl(pInstantGOP,MAPI_CMD_GOP_SET_PROPERTY,&ioctl_info)!= UTOPIA_STATUS_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        return GOP_API_FAIL;
    }
#endif
    return GOP_API_SUCCESS;
}

/********************************************************************************/
/// Set stretch window H-Stretch ratio.
/// Example: gwin size:960*540  target gwin size: 1920*1080
///     step1: MApi_GOP_GWIN_Set_HSCALE(TRUE, 960, 1920);
///     step2: MApi_GOP_GWIN_Set_VSCALE(TRUE, 540, 1080);
///     step3: MApi_GOP_GWIN_Set_STRETCHWIN(u8GOPNum, E_GOP_DST_OP0, 0, 0, 960, 540);
/// @param bEnable \b IN:
///   - # TRUE enable
///   - # FALSE disable
/// @param src \b IN: original size
/// @param dst \b IN: target size
/// @return GOP_API_SUCCESS - Success
/********************************************************************************/
E_GOP_API_Result MApi_GOP_GWIN_Set_HSCALE_EX(MS_U8 u8GOP,MS_BOOL bEnable, MS_U16 src, MS_U16 dst)
{
    GOP_STRETCH_INFO stretch_info;
    GOP_ENTRY();

    memset(&stretch_info, 0x0, sizeof(GOP_STRETCH_INFO));

    stretch_info.SrcRect.w = src;
    if (bEnable)
    {
        stretch_info.DstRect.w = dst;
    }
    else
    {
        stretch_info.DstRect.w = src;
    }
#ifdef CONFIG_GOP_UTOPIA10
    if(Ioctl_GOP_Set_Stretch(NULL,E_GOP_STRETCH_HSCALE,u8GOP,&stretch_info) != GOP_API_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN(GOP_API_FAIL);
    }
#else
    GOP_STRETCH_SET_PARAM ioctl_info;
    memset(&ioctl_info, 0x0, sizeof(GOP_STRETCH_SET_PARAM));

    ioctl_info.gop_idx = u8GOP;
    ioctl_info.enStrtchType = E_GOP_STRETCH_HSCALE;
    ioctl_info.pStretch = (MS_U32*)&stretch_info;
    ioctl_info.u32Size = sizeof(GOP_STRETCH_INFO);

    if (UtopiaIoctl(pInstantGOP,MAPI_CMD_GOP_GWIN_SET_STRETCH,(void*)&ioctl_info) != UTOPIA_STATUS_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN(GOP_API_FAIL);
    }
#endif
    GOP_RETURN(GOP_API_SUCCESS);
}

E_GOP_API_Result MApi_GOP_GWIN_Set_HSCALE(MS_BOOL bEnable, MS_U16 src, MS_U16 dst)
{
    MS_U8 u8GOP;
    E_GOP_API_Result ret;

    u8GOP = MApi_GOP_GWIN_GetCurrentGOP();
    ret = MApi_GOP_GWIN_Set_HSCALE_EX(u8GOP,bEnable,src,dst);
    return ret;
}
/********************************************************************************/
/// Set stretch window V-Stretch ratio.
/// Example: gwin size:960*540  target gwin size: 1920*1080
///     step1: MApi_GOP_GWIN_Set_HSCALE(TRUE, 960, 1920);
///     step2: MApi_GOP_GWIN_Set_VSCALE(TRUE, 540, 1080);
///     step3: MApi_GOP_GWIN_Set_STRETCHWIN(u8GOPNum, E_GOP_DST_OP0, 0, 0, 960, 540);
/// @param bEnable \b IN:
///   - # TRUE enable
///   - # FALSE disable
/// @param src \b IN: original size
/// @param dst \b IN: target size
/// @return GOP_API_SUCCESS - Success
/********************************************************************************/
E_GOP_API_Result MApi_GOP_GWIN_Set_VSCALE_EX(MS_U8 u8GOP,MS_BOOL bEnable, MS_U16 src, MS_U16 dst)
{
    GOP_STRETCH_INFO stretch_info;
    GOP_ENTRY();

    memset(&stretch_info, 0x0, sizeof(GOP_STRETCH_INFO));

    stretch_info.SrcRect.h = src;
    if (bEnable)
    {
        stretch_info.DstRect.h = dst;
    }
    else
    {
        stretch_info.DstRect.h = src;
    }
#ifdef CONFIG_GOP_UTOPIA10
    if(Ioctl_GOP_Set_Stretch(NULL,E_GOP_STRETCH_VSCALE,u8GOP,&stretch_info) != GOP_API_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN(GOP_API_FAIL);
    }
#else
    GOP_STRETCH_SET_PARAM ioctl_info;
    memset(&ioctl_info, 0x0, sizeof(GOP_STRETCH_SET_PARAM));

    ioctl_info.gop_idx = u8GOP;
    ioctl_info.enStrtchType = E_GOP_STRETCH_VSCALE;
    ioctl_info.pStretch = (MS_U32*)&stretch_info;
    ioctl_info.u32Size = sizeof(GOP_STRETCH_INFO);

    if (UtopiaIoctl(pInstantGOP,MAPI_CMD_GOP_GWIN_SET_STRETCH,(void*)&ioctl_info) != UTOPIA_STATUS_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN(GOP_API_FAIL);
    }
#endif
    GOP_RETURN(GOP_API_SUCCESS);
}
E_GOP_API_Result MApi_GOP_GWIN_Set_VSCALE(MS_BOOL bEnable, MS_U16 src, MS_U16 dst)
{
    MS_U8 u8GOP;
    E_GOP_API_Result ret;

    u8GOP = MApi_GOP_GWIN_GetCurrentGOP();
    ret = MApi_GOP_GWIN_Set_VSCALE_EX(u8GOP,bEnable,src,dst);
    return ret;
}
#ifndef CONFIG_GOP_REDUCE_CODESIZE_FOR_MBOOT
/********************************************************************************/
/// Set GOP H stretch mode
/// @param HStrchMode \b IN:
///   - # E_GOP_HSTRCH_6TAPE
///   - # E_GOP_HSTRCH_DUPLICATE
/// @return GOP_API_SUCCESS - Success
/********************************************************************************/
E_GOP_API_Result MApi_GOP_GWIN_Set_HStretchMode_EX(MS_U8 u8GOP,EN_GOP_STRETCH_HMODE HStrchMode)
{
    GOP_STRETCH_INFO stretch_info;
    GOP_ENTRY();
    MS_U32 u32Ret = 0;

    memset(&stretch_info, 0x0, sizeof(GOP_STRETCH_INFO));

    stretch_info.enHMode = HStrchMode;
#ifdef CONFIG_GOP_UTOPIA10
    u32Ret = Ioctl_GOP_Set_Stretch(NULL,E_GOP_STRETCH_HSTRETCH_MODE,u8GOP,&stretch_info);
    if(u32Ret != GOP_API_SUCCESS)
    {
        if((E_GOP_API_Result)u32Ret == GOP_API_FUN_NOT_SUPPORTED)
        {
            GOP_ERR("[%s]Function not support\n",__FUNCTION__);
            GOP_RETURN(GOP_API_FUN_NOT_SUPPORTED);
        }
        else
        {
            GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
            GOP_RETURN(GOP_API_FAIL);
        }
    }
#else
    GOP_STRETCH_SET_PARAM ioctl_info;
    memset(&ioctl_info, 0x0, sizeof(GOP_STRETCH_SET_PARAM));

    ioctl_info.gop_idx = u8GOP;
    ioctl_info.enStrtchType = E_GOP_STRETCH_HSTRETCH_MODE;
    ioctl_info.pStretch = (MS_U32*)&stretch_info;
    ioctl_info.u32Size = sizeof(GOP_STRETCH_INFO);

    u32Ret = UtopiaIoctl(pInstantGOP,MAPI_CMD_GOP_GWIN_SET_STRETCH,(void*)&ioctl_info);

    if(u32Ret != UTOPIA_STATUS_SUCCESS)
    {
        if(u32Ret == UTOPIA_STATUS_NOT_SUPPORTED)
        {
            GOP_ERR("[%s]Function not support\n",__FUNCTION__);
            GOP_RETURN(GOP_API_FUN_NOT_SUPPORTED);
        }
        else
        {
            GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
            GOP_RETURN( GOP_API_FAIL);
        }
    }
#endif
    GOP_RETURN(GOP_API_SUCCESS);
}
E_GOP_API_Result MApi_GOP_GWIN_Set_HStretchMode(EN_GOP_STRETCH_HMODE HStrchMode)
{
    MS_U8 u8GOP;
    E_GOP_API_Result ret;

    u8GOP = MApi_GOP_GWIN_GetCurrentGOP();
    ret = MApi_GOP_GWIN_Set_HStretchMode_EX(u8GOP,HStrchMode);
    return ret;
}

/********************************************************************************/
/// Set GOP V stretch mode
/// @param VStrchMode \b IN:
///   - # E_GOP_VSTRCH_LINEAR
///   - # E_GOP_VSTRCH_DUPLICATE
///   - # E_GOP_VSTRCH_NEAREST
/// @return GOP_API_SUCCESS - Success
/********************************************************************************/
E_GOP_API_Result MApi_GOP_GWIN_Set_VStretchMode_EX(MS_U8 u8GOP,EN_GOP_STRETCH_VMODE VStrchMode)
{
    GOP_STRETCH_INFO stretch_info;
    GOP_ENTRY();
    MS_U32 u32Ret = 0;

    memset(&stretch_info, 0x0, sizeof(GOP_STRETCH_INFO));

    stretch_info.enVMode = VStrchMode;
#ifdef CONFIG_GOP_UTOPIA10
    u32Ret = Ioctl_GOP_Set_Stretch(NULL,E_GOP_STRETCH_VSTRETCH_MODE,u8GOP,&stretch_info);
    if(u32Ret != GOP_API_SUCCESS)
    {
        if((E_GOP_API_Result)u32Ret == GOP_API_FUN_NOT_SUPPORTED)
        {
            GOP_ERR("[%s]Function not support\n",__FUNCTION__);
            GOP_RETURN(GOP_API_FUN_NOT_SUPPORTED);
        }
        else
        {
            GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
            GOP_RETURN(GOP_API_FAIL);
        }
    }
#else
    GOP_STRETCH_SET_PARAM ioctl_info;
    memset(&ioctl_info, 0x0, sizeof(GOP_STRETCH_SET_PARAM));

    ioctl_info.gop_idx = u8GOP;
    ioctl_info.enStrtchType = E_GOP_STRETCH_VSTRETCH_MODE;
    ioctl_info.pStretch = (MS_U32*)&stretch_info;
    ioctl_info.u32Size = sizeof(GOP_STRETCH_INFO);

    u32Ret = UtopiaIoctl(pInstantGOP,MAPI_CMD_GOP_GWIN_SET_STRETCH,(void*)&ioctl_info);
    if(u32Ret != UTOPIA_STATUS_SUCCESS)
    {
        if((E_GOP_API_Result)u32Ret == UTOPIA_STATUS_NOT_SUPPORTED)
        {
            GOP_ERR("[%s]Function not support\n",__FUNCTION__);
            GOP_RETURN(GOP_API_FUN_NOT_SUPPORTED);
        }
        else
        {
            GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
            GOP_RETURN(GOP_API_FAIL);
        }
    }
#endif
    GOP_RETURN(GOP_API_SUCCESS);
}
E_GOP_API_Result MApi_GOP_GWIN_Set_VStretchMode(EN_GOP_STRETCH_VMODE VStrchMode)
{
    MS_U8 u8GOP;
    E_GOP_API_Result ret;

    u8GOP = MApi_GOP_GWIN_GetCurrentGOP();
    ret = MApi_GOP_GWIN_Set_VStretchMode_EX(u8GOP,VStrchMode);
    return ret;
}

//-------------------------------------------------------------------------------------------------
/// set gop destination path clock
/// @param gopNum \b IN: Number of GOP
/// @param eDstType \b IN: gop destination type
/// @return GOP_API_SUCCESS - Success
//-------------------------------------------------------------------------------------------------
E_GOP_API_Result MApi_GOP_SetGOPClk(MS_U8 gopNum, EN_GOP_DST_TYPE eDstType)
{
    EN_GOP_DST_TYPE eType;

    GOP_ENTRY();

    eType = eDstType;

#ifdef CONFIG_GOP_UTOPIA10
    if(Ioctl_GOP_SetProperty(NULL,E_GOP_CLK,gopNum,(MS_U32*)&eType,sizeof(EN_GOP_DST_TYPE)) != GOP_API_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN(GOP_API_FAIL);
    }
#else
    GOP_SET_PROPERTY_PARAM ioctl_info;
    memset(&ioctl_info, 0x0, sizeof(GOP_SET_PROPERTY_PARAM));

    ioctl_info.en_pro   = E_GOP_CLK;
    ioctl_info.gop_idx  = gopNum;
    ioctl_info.pSetting = (void*)&eType;
    ioctl_info.u32Size  = sizeof(EN_GOP_DST_TYPE);

    if(UtopiaIoctl(pInstantGOP,MAPI_CMD_GOP_SET_PROPERTY,&ioctl_info)!= UTOPIA_STATUS_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN( GOP_API_FAIL);
    }
#endif
    GOP_RETURN(GOP_API_SUCCESS);
}


//******************************************************************************
/// Enable gop progressive mode
/// @param bEnable \b IN:
///   - # TRUE Enable gop progressive mode
///   - # FALSE Disable gop progressive mode
/// @return GOP_API_SUCCESS - Success
//******************************************************************************
E_GOP_API_Result MApi_GOP_GWIN_EnableProgressive_EX(MS_U8 u8GOP,MS_BOOL bEnable)
{
    MS_U32  Progressive;

    GOP_ENTRY();

    Progressive = bEnable;
#ifdef CONFIG_GOP_UTOPIA10
    if(Ioctl_GOP_SetProperty(NULL,E_GOP_PROGRESSIVE ,u8GOP, (void*)&Progressive,sizeof(MS_BOOL)) != GOP_API_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN(GOP_API_FAIL);
    }
#else
    GOP_SET_PROPERTY_PARAM ioctl_info;
    memset(&ioctl_info, 0x0, sizeof(GOP_SET_PROPERTY_PARAM));
    ioctl_info.en_pro   = E_GOP_PROGRESSIVE;
    ioctl_info.gop_idx  = u8GOP;
    ioctl_info.pSetting = (void*)&Progressive;
    ioctl_info.u32Size  = sizeof(MS_BOOL);

    if(UtopiaIoctl(pInstantGOP,MAPI_CMD_GOP_SET_PROPERTY,&ioctl_info)!= UTOPIA_STATUS_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN( GOP_API_FAIL);
    }
#endif
    GOP_RETURN(GOP_API_SUCCESS);
}

E_GOP_API_Result MApi_GOP_GWIN_EnableProgressive(MS_BOOL bEnable)
{
    MS_U8 u8GOP;
    E_GOP_API_Result ret;

    u8GOP = MApi_GOP_GWIN_GetCurrentGOP();
    ret = MApi_GOP_GWIN_EnableProgressive_EX(u8GOP,bEnable);
    return ret;

}

//******************************************************************************
/// Set gop update register method by only once.
/// Example: if you want to update GOP function A, B, C in the same V sync, please write down your code like below
/// MApi_GOP_GWIN_UpdateRegOnce(TRUE);
/// GOP_FUN_A;
/// GOP_FUN_B;
/// GOP_FUN_C;
/// MApi_GOP_GWIN_UpdateRegOnce(FALSE);
/// @param bWriteRegOnce \b IN: TRUE/FALSE
/// @return GOP_API_SUCCESS - Success
//******************************************************************************
E_GOP_API_Result MApi_GOP_GWIN_UpdateRegOnce(MS_BOOL bWriteRegOnce)
{
    return MApi_GOP_GWIN_UpdateRegOnceEx(bWriteRegOnce, TRUE);
}

//******************************************************************************
/// Extend MApi_GOP_GWIN_UpdateRegOnceEx, update special gop.
/// Set gop update register method by only once.
/// Example: if you want to update GOP function A, B, C in the same V sync, please write down your code like below
/// MApi_GOP_GWIN_UpdateRegOnceByMask(u16GopMask,TRUE, TRUE);
/// GOP_FUN_A;
/// GOP_FUN_B;
/// GOP_FUN_C;
/// MApi_GOP_GWIN_UpdateRegOnceByMask(u16GopMask,FALSE, TRUE);
/// @param u16GopMask    \b IN:bit0-gop0, bit1-gop1...
/// @param bWriteRegOnce    \b IN: TRUE/FALSE
/// @param bSync            \b IN: TRUE/FALSE
/// @return GOP_API_SUCCESS - Success
//******************************************************************************
E_GOP_API_Result MApi_GOP_GWIN_UpdateRegOnceByMask(MS_U16 u16GopMask,MS_BOOL bWriteRegOnce, MS_BOOL bSync)
{
    GOP_UPDATE_INFO update;

    GOP_ENTRY();
    memset(&update, 0x0, sizeof(GOP_UPDATE_INFO));

    update.gop_idx = u16GopMask;
    update.update_type = E_GOP_UPDATE_CURRENT_ONCE;
    update.bEn = bWriteRegOnce;
    update.bSync = bSync;
#ifdef CONFIG_GOP_UTOPIA10
    if(Ioctl_GOP_TriggerRegWriteIn(NULL,update.gop_idx,update.update_type,update.bEn,update.bSync) != GOP_API_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN(GOP_API_FAIL);
    }
#else
    GOP_UPDATE_PARAM ioctl_info;
    memset(&ioctl_info, 0x0, sizeof(GOP_UPDATE_PARAM));

    ioctl_info.pUpdateInfo = (MS_U32*)&update;
    ioctl_info.u32Size = sizeof(GOP_UPDATE_INFO);

    if(UtopiaIoctl(pInstantGOP,MAPI_CMD_GOP_UPDATE,&ioctl_info)!= UTOPIA_STATUS_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN( GOP_API_FAIL);
    }
#endif
    GOP_RETURN(GOP_API_SUCCESS);

}

//******************************************************************************
/// Extend MApi_GOP_GWIN_UpdateRegOnce, add parameter bSync.
/// Set gop update register method by only once.
/// Example: if you want to update GOP function A, B, C in the same V sync, please write down your code like below
/// MApi_GOP_GWIN_UpdateRegOnceEx(TRUE, TRUE);
/// GOP_FUN_A;
/// GOP_FUN_B;
/// GOP_FUN_C;
/// MApi_GOP_GWIN_UpdateRegOnceEx(FALSE, TRUE);
/// @param bWriteRegOnce    \b IN: TRUE/FALSE
/// @param bSync            \b IN: TRUE/FALSE
/// @return GOP_API_SUCCESS - Success
//******************************************************************************
E_GOP_API_Result MApi_GOP_GWIN_UpdateRegOnceEx2(MS_U8 u8GOP,MS_BOOL bWriteRegOnce, MS_BOOL bSync)
{
    GOP_UPDATE_INFO update;

    GOP_ENTRY();
    memset(&update, 0x0, sizeof(GOP_UPDATE_INFO));

    update.gop_idx = u8GOP;
    update.update_type = E_GOP_UPDATE_ONCE;
    update.bEn = bWriteRegOnce;
    update.bSync = bSync;
#ifdef CONFIG_GOP_UTOPIA10
    if(Ioctl_GOP_TriggerRegWriteIn(NULL,update.gop_idx,update.update_type,update.bEn,update.bSync) != GOP_API_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN(GOP_API_FAIL);
    }
#else

    GOP_UPDATE_PARAM ioctl_info;
    memset(&ioctl_info, 0x0, sizeof(GOP_UPDATE_PARAM));

    ioctl_info.pUpdateInfo = (MS_U32*)&update;
    ioctl_info.u32Size = sizeof(GOP_UPDATE_INFO);

    if(UtopiaIoctl(pInstantGOP,MAPI_CMD_GOP_UPDATE,&ioctl_info)!= UTOPIA_STATUS_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN( GOP_API_FAIL);
    }
#endif
    GOP_RETURN(GOP_API_SUCCESS);

}

E_GOP_API_Result MApi_GOP_GWIN_UpdateRegOnceEx(MS_BOOL bWriteRegOnce, MS_BOOL bSync)
{
    MS_U8 u8GOP;
    E_GOP_API_Result ret;

    u8GOP = MApi_GOP_GWIN_GetCurrentGOP();
    ret = MApi_GOP_GWIN_UpdateRegOnceEx2(u8GOP,bWriteRegOnce,bSync);
    return ret;

}
#endif
//-------------------------------------------------------------------------------------------------
/// Get current active gop
/// @return current gop number
//-------------------------------------------------------------------------------------------------
MS_U8 MApi_GOP_GWIN_GetCurrentGOP(void)
{
    MS_U32 u32Gop = INVALID_GOP_NUM;
    GOP_ENTRY();

#ifdef CONFIG_GOP_UTOPIA10
    if(Ioctl_GOP_GetStatus(NULL,E_GOP_STATUS_CURRENT_GOP,(MS_U32*) &u32Gop,sizeof(MS_U32)) != GOP_API_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN(GOP_API_FAIL);
    }
#else
    GOP_GET_STATUS_PARAM ioctl_info;
    memset(&ioctl_info, 0x0, sizeof(GOP_UPDATE_PARAM));

    ioctl_info.type = 0;
    ioctl_info.en_status = E_GOP_STATUS_CURRENT_GOP;
    ioctl_info.pStatus = (MS_U32*) &u32Gop;
    ioctl_info.u32Size = sizeof(MS_U32);

    if (UtopiaIoctl(pInstantGOP,MAPI_CMD_GOP_GET_STATUS,(void*)&ioctl_info) != UTOPIA_STATUS_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN(GOP_API_FAIL);
    }
#endif
    GOP_RETURN((MS_U8)(u32Gop&0xFF));
}
#ifndef CONFIG_GOP_REDUCE_CODESIZE_FOR_MBOOT

/********************************************************************************/
/// Set GWIN information to GOP registers
/// @param u8win \b IN: GWINID
/// @param pinfo \b IN: GWIN Information
/// @return GOP_API_SUCCESS - Success
/// @return GOP_API_FAIL - Failure
/********************************************************************************/
E_GOP_API_Result MApi_GOP_GWIN_SetWinInfo(MS_U8 u8win, GOP_GwinInfo* pinfo)
{
    GOP_BUFFER_INFO win_info;

    GOP_ENTRY();

    memset(&win_info, 0x0, sizeof(GOP_BUFFER_INFO));

    win_info.addr = pinfo->u32DRAMRBlkStart;
    win_info.disp_rect.x = pinfo->u16DispHPixelStart;
    win_info.disp_rect.y = pinfo->u16DispVPixelStart;
    win_info.disp_rect.w = pinfo->u16DispHPixelEnd - pinfo->u16DispHPixelStart;
    win_info.disp_rect.h = pinfo->u16DispVPixelEnd - pinfo->u16DispVPixelStart;
    win_info.width  = pinfo->u16RBlkHPixSize;
    win_info.height = pinfo->u16RBlkVPixSize;
    win_info.fbFmt  = pinfo->clrType;
    win_info.pitch  = pinfo->u16RBlkHRblkSize;
#ifdef CONFIG_GOP_UTOPIA10
    if(Ioctl_GOP_SetWinInfo(NULL,u8win,&win_info) != GOP_API_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN(GOP_API_FAIL);
    }
#else
    GOP_GWIN_SETWININFO_PARAM ioctl_info;
    memset(&ioctl_info, 0x0, sizeof(GOP_GWIN_SETWININFO_PARAM));

    ioctl_info.GwinId = u8win;
    ioctl_info.pinfo = (MS_U32*)&win_info;
    ioctl_info.u32Size = sizeof(GOP_BUFFER_INFO);

    if(UtopiaIoctl(pInstantGOP,MAPI_CMD_GOP_GWIN_SET_WININFO,&ioctl_info)!= UTOPIA_STATUS_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN( GOP_API_FAIL);
    }
#endif
    GOP_RETURN(GOP_API_SUCCESS);
}

/******************************************************************************/
/// Get current frame buffer pool id
/// @return current frame buffer pool id. 0: first frame buffer pool, 1: second frame buffer pool
/******************************************************************************/
MS_U8 MApi_GOP_FB_Get(void)
{
	MS_U32 u32PoolId = 0x0;
    GOP_ENTRY();

#ifdef CONFIG_GOP_UTOPIA10
    if(Ioctl_GOP_FB_GetProperty(NULL,E_GOP_FB_POOLID,0x0, &u32PoolId,sizeof(MS_U8)) != GOP_API_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN(GOP_API_FAIL);
    }
#else
    GOP_FB_PROPERTY_PARAM ioctl_info;
    memset(&ioctl_info, 0x0, sizeof(GOP_FB_PROPERTY_PARAM));

    ioctl_info.en_property = E_GOP_FB_POOLID;
    ioctl_info.FBId = 0x0;
    ioctl_info.pSet = (void*)&u32PoolId;
    ioctl_info.u32Size = sizeof(MS_U8);

    if (UtopiaIoctl(pInstantGOP,MAPI_CMD_GOP_FB_GET_PROPERTY,(void*)&ioctl_info) != UTOPIA_STATUS_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN((MS_U8)u32PoolId);
    }
#endif
    GOP_RETURN((MS_U8)u32PoolId);
}

//******************************************************************************
/// Set GWin Attribute to Shared. If shared GWin, More than one process could
/// access this GWin.
/// @param winId \b IN: GWIN ID for shared
/// @param bIsShared \b IN: shared or not
/// @return GOP_API_SUCCESS - Success
//******************************************************************************
E_GOP_API_Result MApi_GOP_GWIN_SetGWinShared(MS_U8 winId, MS_BOOL bIsShared)
{
	MS_U32 u32Shared;

    GOP_ENTRY();

	u32Shared = (MS_U32)bIsShared;

#ifdef CONFIG_GOP_UTOPIA10
    if(Ioctl_GOP_GWin_SetProperty(NULL,E_GOP_GWIN_SHARE,winId, &u32Shared,sizeof(MS_BOOL)) != GOP_API_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN(GOP_API_FAIL);
    }
#else

    GOP_GWIN_PROPERTY_PARAM ioctl_info;
    memset(&ioctl_info, 0x0, sizeof(GOP_GWIN_PROPERTY_PARAM));

    ioctl_info.en_property = E_GOP_GWIN_SHARE;
    ioctl_info.GwinId = winId;
    ioctl_info.pSet = (void*)&u32Shared;
    ioctl_info.u32Size = sizeof(MS_BOOL);

    if (UtopiaIoctl(pInstantGOP,MAPI_CMD_GOP_GWIN_SET_PROPERTY,(void*)&ioctl_info) != UTOPIA_STATUS_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN(GOP_API_FAIL);
    }
#endif
    GOP_RETURN(GOP_API_SUCCESS);
}

//******************************************************************************
/// Set Reference cnt of shared GWin.
/// @param winId \b IN: GWIN ID for shared
/// @param u16SharedCnt \b IN: shared reference cnt.
/// @return GOP_API_SUCCESS - Success
//******************************************************************************
E_GOP_API_Result MApi_GOP_GWIN_SetGWinSharedCnt(MS_U8 winId, MS_U16 u16SharedCnt)
{
	MS_U32 u32Cnt;

    GOP_ENTRY();

	u32Cnt = (MS_U32)u16SharedCnt;

#ifdef CONFIG_GOP_UTOPIA10
    if(Ioctl_GOP_GWin_SetProperty(NULL,E_GOP_GWIN_SHARE_CNT,winId,&u32Cnt,sizeof(MS_U16)) != GOP_API_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN(GOP_API_FAIL);
    }
#else
    GOP_GWIN_PROPERTY_PARAM ioctl_info;
    memset(&ioctl_info, 0x0, sizeof(GOP_GWIN_PROPERTY_PARAM));

    ioctl_info.en_property = E_GOP_GWIN_SHARE_CNT;
    ioctl_info.GwinId = winId;
    ioctl_info.pSet = (void*)&u32Cnt;
    ioctl_info.u32Size = sizeof(MS_U16);

    if (UtopiaIoctl(pInstantGOP,MAPI_CMD_GOP_GWIN_SET_PROPERTY,(void*)&ioctl_info) != UTOPIA_STATUS_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN(GOP_API_FAIL);
    }
#endif
    GOP_RETURN(GOP_API_SUCCESS);
}

//******************************************************************************
/// Create GWIN and assign frame buffer to this GWIN
/// @param u8GOP \b IN: GOP number
/// @param FBId \b IN: frame buffer ID
/// @param dispX \b IN: frame buffer starting x position
/// @param dispY \b IN: frame buffer starting y position
/// @return The id for the created GWIN, if return 0xff, it represents create gwin fail
//******************************************************************************
MS_U8 MApi_GOP_GWIN_CreateWin_Assign_FB(MS_U8 u8GOP, MS_U8 FBId, MS_U16 dispX, MS_U16 dispY)
{
    return MApi_GOP_GWIN_CreateWin_Assign_32FB(u8GOP, (MS_U32)FBId, dispX, dispY);
}

MS_U8 MApi_GOP_GWIN_CreateWin_Assign_32FB(MS_U8 u8GOP, MS_U32 u32FBId, MS_U16 dispX, MS_U16 dispY)
{
    MS_U8 winId;
    GOP_ENTRY();

    MApi_GOP_GWIN_SwitchGOP(u8GOP);

    winId = MApi_GOP_GWIN_GetFreeWinID();

    MApi_GOP_GWIN_Map32FB2Win(u32FBId,winId);

    GOP_RETURN(winId);

}

//******************************************************************************
/// Create GWIN by given GWIN id and frame buffer id
/// @param u8WinId \b IN: GWIN id
/// @param u8GOP \b IN: GOP number
/// @param FBId \b IN: frame buffer id
/// @param dispX \b IN: gwin's frame buffer starting x
/// @param dispY \b IN: gwin's frame buffer starting y
/// @return The id for the created GWIN, if return 0xff, it represents create gwin fail
//******************************************************************************
MS_U8 MApi_GOP_GWIN_CreateStaticWin_Assign_FB(MS_U8 u8WinId, MS_U8 u8GOP, MS_U8 FBId, MS_U16 dispX, MS_U16 dispY)
{
    return MApi_GOP_GWIN_CreateStaticWin_Assign_32FB(u8WinId, u8GOP, (MS_U32)FBId, dispX, dispY);
}

MS_U8 MApi_GOP_GWIN_CreateStaticWin_Assign_32FB(MS_U8 u8WinId, MS_U8 u8GOP, MS_U32 u32FBId, MS_U16 dispX, MS_U16 dispY)
{
    GOP_ENTRY();

    MApi_GOP_GWIN_Map32FB2Win(u32FBId,u8WinId);

    GOP_RETURN(u8WinId);
}


/******************************************************************************/
/// Check if GWIN is created
/// @param gId \b IN: GWINID
/// @return TRUE or FALSE
/******************************************************************************/
MS_U8 MApi_GOP_GWIN_IsGwinExist(MS_U8 gId)
{
	MS_U32 u32Exist = FALSE;

    GOP_ENTRY();

#ifdef CONFIG_GOP_UTOPIA10
    if(Ioctl_GOP_GWin_GetProperty(NULL,E_GOP_GWIN_EXIST,gId, &u32Exist,sizeof(MS_U8)) != GOP_API_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN(GOP_API_FAIL);
    }
#else
    GOP_GWIN_PROPERTY_PARAM ioctl_info;
    memset(&ioctl_info, 0x0, sizeof(GOP_GWIN_PROPERTY_PARAM));

    ioctl_info.en_property = E_GOP_GWIN_EXIST;
    ioctl_info.GwinId = gId;
    ioctl_info.pSet = (void*)&u32Exist;
    ioctl_info.u32Size = sizeof(MS_U8);

    if (UtopiaIoctl(pInstantGOP,MAPI_CMD_GOP_GWIN_GET_PROPERTY,(void*)&ioctl_info) != UTOPIA_STATUS_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN((MS_U8)u32Exist);
    }
#endif
    GOP_RETURN((MS_U8)u32Exist);
}

/******************************************************************************/
/// Get total active gwin number
/// @return total active gwin number
/******************************************************************************/
MS_U16   MApi_GOP_GWIN_GetActiveGWIN(void)
{
    MS_U8 gwinNum = 0;
    MS_U8 i;
    GOP_ENTRY();

    for(i = 0; i < MAX_GWIN_SUPPORT ; i++)
    {
        if(MApi_GOP_GWIN_IsGWINEnabled(i) == TRUE)
        {
            gwinNum |= 1<<i;
        }
    }
    GOP_RETURN(gwinNum);

}
#endif
//******************************************************************************
/// Enable GWIN for display
/// @param winId \b IN: GWIN id
/// @param bEnable \b IN:
///   - # TRUE Show GWIN
///   - # FALSE Hide GWIN
/// @return GOP_API_SUCCESS - Success
/// @return GOP_API_FAIL - Failure
//******************************************************************************
E_GOP_API_Result MApi_GOP_GWIN_Enable(MS_U8 winId, MS_BOOL bEnable)
{
	MS_U32 u32En;
    GOP_ENTRY();

	u32En = (MS_U32)bEnable;
#ifdef CONFIG_GOP_UTOPIA10
    if(Ioctl_GOP_GWin_SetProperty(NULL,E_GOP_GWIN_ENABLE,winId, &u32En,sizeof(MS_BOOL)) != GOP_API_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN(GOP_API_FAIL);
    }
#else

    GOP_GWIN_PROPERTY_PARAM ioctl_info;
    memset(&ioctl_info, 0x0, sizeof(GOP_GWIN_PROPERTY_PARAM));

    ioctl_info.en_property = E_GOP_GWIN_ENABLE;
    ioctl_info.GwinId = winId;
    ioctl_info.pSet = (void*) &u32En;
    ioctl_info.u32Size = sizeof(MS_BOOL);

    if(UtopiaIoctl(pInstantGOP,MAPI_CMD_GOP_GWIN_SET_PROPERTY,&ioctl_info)!= UTOPIA_STATUS_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN(GOP_API_FAIL);
    }
#endif
    GOP_RETURN(GOP_API_SUCCESS);

}
#ifndef CONFIG_GOP_REDUCE_CODESIZE_FOR_MBOOT

/********************************************************************************/
/// Check if GWIN is enabled
/********************************************************************************/
MS_BOOL MApi_GOP_GWIN_IsEnabled(void)
{
    MS_U8 i = 0;
    MS_BOOL bEnable =FALSE;
    GOP_ENTRY();

    while(i<MAX_GWIN_SUPPORT)
    {
        bEnable = MApi_GOP_GWIN_IsGWINEnabled(i);
        if (bEnable==TRUE)
        {
            GOP_RETURN(TRUE);
        }
        i++;
    }
    GOP_RETURN(FALSE);
}

//******************************************************************************
/// Check if all GWIN is currently enabled
/// @return  - # TRUE GWins are enabled
/// @return  - # FALSE not all GWins are enabled
//******************************************************************************
MS_BOOL MApi_GOP_GWIN_IsAllGWINDisabled(void)
{
    MS_BOOL ret;
    GOP_ENTRY();

    ret =  MApi_GOP_GWIN_IsEnabled();
    GOP_RETURN(ret);

}

//******************************************************************************
/// Check if all some GWIN is currently enabled
/// @param  winId \b IN: gwin id
/// @return  - the according GWin is enabled or not
//******************************************************************************
MS_BOOL MApi_GOP_GWIN_IsGWINEnabled(MS_U8 winId)
{
	MS_U32 u32En;

    GOP_ENTRY();
#ifdef CONFIG_GOP_UTOPIA10
    if(Ioctl_GOP_GWin_GetProperty(NULL,E_GOP_GWIN_ENABLE,winId, &u32En,sizeof(MS_BOOL)) != GOP_API_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN(GOP_API_FAIL);
    }
#else
    GOP_GWIN_PROPERTY_PARAM ioctl_info;
    memset(&ioctl_info, 0x0, sizeof(GOP_GWIN_PROPERTY_PARAM));

    ioctl_info.en_property = E_GOP_GWIN_ENABLE;
    ioctl_info.GwinId = winId;
    ioctl_info.pSet = (void*) &u32En;
    ioctl_info.u32Size = sizeof(MS_BOOL);

    if(UtopiaIoctl(pInstantGOP,MAPI_CMD_GOP_GWIN_GET_PROPERTY,&ioctl_info)!= UTOPIA_STATUS_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN( GOP_API_FAIL);
    }
#endif
    GOP_RETURN((MS_BOOL)u32En);


}

//******************************************************************************
/// [Obsolete Function]
/// @param hext \b IN
///   - 0 disable horizontal pixel duplication
///   - 1 enable  horizontal pixel duplication
/// @param vext \b IN
///   - 0 disable vertical line duplication
///   - 1 enable  vertical line duplication
/// @return GOP_API_SUCCESS - Success
//******************************************************************************
E_GOP_API_Result MApi_GOP_GWIN_SetDuplication(MS_U8 GOP_UNUSED(hext), MS_U8 GOP_UNUSED(vext))
{
//NOT USE FOR COVERITY HAPPY -_-~
// THE CODE BELOW is useless

    GOP_ENTRY();
    GOP_RETURN(GOP_API_SUCCESS);

}

#endif
/******************************************************************************/
/// Delete the GWIN, free corresponding frame buffer
/// @param winId \b IN GWIN id
/// @return GOP_API_SUCCESS - Success
/// @return GOP_API_FAIL - Failure
/******************************************************************************/
E_GOP_API_Result MApi_GOP_GWIN_DeleteWin(MS_U8 winId)
{
    MS_U32 u32FBID;

    GOP_ENTRY();


    u32FBID = MApi_GOP_GWIN_Get32FBfromGWIN(winId);

#ifdef CONFIG_GOP_UTOPIA10
    if(Ioctl_GOP_Win_Destroy(NULL,winId) != GOP_API_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN(GOP_API_FAIL);
    }
#else
    GOP_GWIN_DESTROY_PARAM ioctl_info;
    memset(&ioctl_info, 0x0, sizeof(GOP_GWIN_DESTROY_PARAM));
    ioctl_info.GwinId = winId;

    if (UtopiaIoctl(pInstantGOP,MAPI_CMD_GOP_GWIN_DESTROY,(void*)&ioctl_info) != UTOPIA_STATUS_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN(GOP_API_FAIL);
    }
#endif
    if(MApi_GOP_GWIN_Is32FBExist(u32FBID)==TRUE)
    {
        MApi_GOP_GWIN_Destroy32FB(u32FBID);
    }
    GOP_RETURN(GOP_API_SUCCESS);

}
#ifndef CONFIG_GOP_REDUCE_CODESIZE_FOR_MBOOT

/******************************************************************************/
/// Set the GWIN's position
/// @param winId  \b IN: GWIN id
/// @param dispX \b IN: x coordinate
/// @param dispY \b IN: y coordinate
/// @return GOP_API_SUCCESS - Success
/// @return GOP_API_FAIL - Failure
/******************************************************************************/
E_GOP_API_Result MApi_GOP_GWIN_SetWinPosition(MS_U8 winId, MS_U16 dispX, MS_U16 dispY)
{

    GOP_BUFFER_INFO win_info;
    GOP_ENTRY();
    memset(&win_info, 0x0, sizeof(GOP_BUFFER_INFO));
#ifdef CONFIG_GOP_UTOPIA10
    if(Ioctl_GOP_GetWinInfo(NULL,winId,&win_info) != GOP_API_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN(GOP_API_FAIL);
    }
#else
    GOP_GWIN_SETWININFO_PARAM ioctl_info;
    memset(&ioctl_info, 0x0, sizeof(GOP_GWIN_SETWININFO_PARAM));

    ioctl_info.GwinId = winId;
    ioctl_info.pinfo = (MS_U32*)&win_info;
    ioctl_info.u32Size = sizeof(GOP_BUFFER_INFO);

    if(UtopiaIoctl(pInstantGOP,MAPI_CMD_GOP_GWIN_GET_WININFO,&ioctl_info)!= UTOPIA_STATUS_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN( GOP_API_FAIL);
    }
#endif

    win_info.disp_rect.x = dispX;
    win_info.disp_rect.y = dispY;

#ifdef CONFIG_GOP_UTOPIA10
    if(Ioctl_GOP_SetWinInfo(NULL,winId,&win_info) != GOP_API_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN(GOP_API_FAIL);
    }
#else
    if(UtopiaIoctl(pInstantGOP,MAPI_CMD_GOP_GWIN_SET_WININFO,&ioctl_info)!= UTOPIA_STATUS_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN( GOP_API_FAIL);
    }
#endif

    GOP_RETURN(GOP_API_SUCCESS);

}

/******************************************************************************/
/// [Obstacle Function]
/// @return 0
/******************************************************************************/
MS_U16 MApi_GOP_GWIN_Get_HSTART(void)
{
    return 0;
}


/******************************************************************************/
/// Set the GWIN's frame buffer offset
/// @param winId  \b IN: GWIN id
/// @param x \b IN: x coordinate
/// @param y \b IN: y coordinate
/// @return GOP_API_SUCCESS - Success
/// @return GOP_API_FAIL - Failure
/******************************************************************************/
E_GOP_API_Result MApi_GOP_GWIN_SetWinPositionOffset(MS_U8 winId, MS_U16 x, MS_U16 y)
{
    GOP_GwinInfo gwin;
    GOP_ENTRY();
    memset(&gwin, 0, sizeof(GOP_GwinInfo));
    MApi_GOP_GWIN_GetWinInfo(winId, &gwin);
    gwin.u16WinX = x;
    gwin.u16WinY = y;
    MApi_GOP_GWIN_SetWinInfo(winId, &gwin);
    GOP_RETURN(GOP_API_SUCCESS);

}


/******************************************************************************/
/// Set gwin property
/// @param WinProperty  \b IN: gwin property info
/// @return GOP_API_SUCCESS - Success
/// @return GOP_API_FAIL - Failure
/******************************************************************************/
E_GOP_API_Result MApi_GOP_GWIN_SetWinProperty(GOP_GwinDispProperty WinProperty)
{
    GOP_GwinInfo gWin;
    MS_U32 u32fbId;
    GOP_GwinFBAttr  fbInfo;

    GOP_ENTRY();

    u32fbId = MApi_GOP_GWIN_GetFBfromGWIN(WinProperty.gId);

    memset(&gWin, 0, sizeof(GOP_GwinInfo));
    memset(&fbInfo, 0x0, sizeof(GOP_GwinFBAttr));

    if(MApi_GOP_GWIN_GetWinInfo(WinProperty.gId,&gWin)!=GOP_API_SUCCESS)
    {
        GOP_WARN("[%s][%d] Fail!!!!\n",__FUNCTION__,__LINE__);
    }
    if(MApi_GOP_GWIN_Get32FBInfo(u32fbId,  &fbInfo)!=GOP_API_SUCCESS)
    {
        GOP_WARN("[%s][%d] Fail!!!!\n",__FUNCTION__,__LINE__);
    }

    gWin.u16DispHPixelStart = WinProperty.StartX;
    gWin.u16DispVPixelStart = WinProperty.StartY;
    gWin.u16DispHPixelEnd = WinProperty.StartX + WinProperty.Win_Width;
    gWin.u16DispVPixelEnd = WinProperty.StartY + WinProperty.Win_Height;
    gWin.u16WinX = WinProperty.FBOffset_X;
    gWin.u16WinY = WinProperty.FBOffset_Y;
    gWin.u16RBlkVPixSize = fbInfo.height;
    gWin.u16RBlkHPixSize = fbInfo.width;
    gWin.u16RBlkHRblkSize = GOP_CalcPitch(fbInfo.fbFmt, fbInfo.width);
    gWin.u32DRAMRBlkStart = fbInfo.addr;
    gWin.clrType = (EN_GOP_COLOR_TYPE)fbInfo.fbFmt;

    MApi_GOP_GWIN_SetWinInfo(WinProperty.gId, &gWin);
    GOP_RETURN(GOP_API_SUCCESS);

}

/******************************************************************************/
/// Switch GE to render on a GWIN's frame buffer
/// @param winId \b IN: GWIN id
/// @return GOP_API_SUCCESS - Success
/// @return GOP_API_FAIL - Failure
/******************************************************************************/
E_GOP_API_Result MApi_GOP_GWIN_Switch2Gwin(MS_U8 winId)
{
    GOP_CBFmtInfo* pCBFmtInfo=NULL;

    GOP_ENTRY();
    pCBFmtInfo = (PGOP_CBFmtInfo)malloc(sizeof(GOP_CBFmtInfo));

	if (pCBFmtInfo == NULL) {
		GOP_ERR("malloc GOP_CBFmtInfo fail\n");
		GOP_RETURN(GOP_API_FAIL);
	}
	memset(pCBFmtInfo, 0, sizeof(GOP_CBFmtInfo));

#ifdef CONFIG_GOP_UTOPIA10
    if(Ioctl_GOP_GWin_SetProperty(NULL,E_GOP_GWIN_SWITCH_2_GWIN,winId,(MS_U32*)pCBFmtInfo,sizeof(int)) != GOP_API_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        free(pCBFmtInfo);
        GOP_RETURN(GOP_API_FAIL);
    }
#else
    GOP_GWIN_PROPERTY_PARAM ioctl_info;
    memset(&ioctl_info, 0x0, sizeof(GOP_GWIN_PROPERTY_PARAM));

    ioctl_info.en_property = E_GOP_GWIN_SWITCH_2_GWIN;
    ioctl_info.GwinId = winId;
    ioctl_info.pSet = (MS_U32*)pCBFmtInfo;
    ioctl_info.u32Size = sizeof(MS_U32);

    if (UtopiaIoctl(pInstantGOP,MAPI_CMD_GOP_GWIN_SET_PROPERTY,(void*)&ioctl_info) != UTOPIA_STATUS_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        free(pCBFmtInfo);
        GOP_RETURN(GOP_API_FAIL);
    }
#endif
    free(pCBFmtInfo);

    GOP_RETURN(GOP_API_SUCCESS);
}


/*****************************************************************************/
/// Get current active GWIN id
/// @return current active GWIN id
/*****************************************************************************/
MS_U8 MApi_GOP_GWIN_GetCurrentWinId()
{
    MS_U32 u32GWin = INVALID_WIN_ID;
    GOP_ENTRY();

#ifdef CONFIG_GOP_UTOPIA10
    if(Ioctl_GOP_GetStatus(NULL,E_GOP_STATUS_CURRENT_GWIN,(MS_U32*) &u32GWin,sizeof(MS_U32)) != GOP_API_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN(GOP_API_FAIL);
    }
#else
    GOP_GET_STATUS_PARAM ioctl_info;
    memset(&ioctl_info, 0x0, sizeof(GOP_GET_STATUS_PARAM));

    ioctl_info.type = 0;
    ioctl_info.en_status = E_GOP_STATUS_CURRENT_GWIN;
    ioctl_info.pStatus = (MS_U32*) &u32GWin;
    ioctl_info.u32Size = sizeof(MS_U32);

    if (UtopiaIoctl(pInstantGOP,MAPI_CMD_GOP_GET_STATUS,(void*)&ioctl_info) != UTOPIA_STATUS_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN(GOP_API_FAIL);
    }
#endif
    GOP_RETURN((MS_U8)(u32GWin&0xFF));
}

/********************************************************************************/
/// Set which one gop show in the top when different gop do alpha blending.
/// @param u8GOP \b IN: GOP number
///   - # u8GOP is 0 => gop0's gwins will be shown in the top
///   - # u8GOP is 1 => gop1's gwins will be shown in the top
/// @return GOP_API_SUCCESS - Success
/********************************************************************************/
E_GOP_API_Result MApi_GOP_GWIN_OutputLayerSwitch(MS_U8 u8GOP)
{
    MS_U32 value = 0;

    GOP_ENTRY();
#ifdef CONFIG_GOP_UTOPIA10
    if(Ioctl_GOP_SetProperty(NULL,E_GOP_OUTPUT_LAYER_SWITCH ,u8GOP, (void*)&value,sizeof(MS_U32)) != GOP_API_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN(GOP_API_FAIL);
    }
#else
    GOP_SET_PROPERTY_PARAM ioctl_info;
    memset(&ioctl_info, 0x0, sizeof(GOP_SET_PROPERTY_PARAM));

    ioctl_info.en_pro   = E_GOP_OUTPUT_LAYER_SWITCH;
    ioctl_info.gop_idx  = u8GOP;
    ioctl_info.pSetting = (void*)&value;
    ioctl_info.u32Size  = sizeof(MS_U32);

    if(UtopiaIoctl(pInstantGOP,MAPI_CMD_GOP_SET_PROPERTY,&ioctl_info)!= UTOPIA_STATUS_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN( GOP_API_FAIL);
    }
#endif
    GOP_RETURN(GOP_API_SUCCESS);
}
#endif

/*******************************************************************************/
//Set which MUX select which GOP ,when different gop do the alpha blending
//@param pGopMuxConfig \b IN:information about GOP and corresponding level
//   #u8GopNum: the total GOP counts who need to select change Mux
//   #GopMux[i].u8GopIndex :the GOP which need to change Mux
//   #GopMux[i].u8MuxIndex :the GOP corresponding Mux
//@return GOP_API_SUCCESS - Success
/*******************************************************************************/

E_GOP_API_Result MApi_GOP_GWIN_SetMux(GOP_MuxConfig * pGopMuxConfig, MS_U32 u32SizeOfMuxInfo)
{
    GOP_SETMUX  MuxSet;
    MS_U32 i;

    GOP_ENTRY();

    memset(&MuxSet, 0x0, sizeof(GOP_SETMUX));

    MuxSet.MuxCount = pGopMuxConfig->u8MuxCounts;

    for(i=0; i< pGopMuxConfig->u8MuxCounts; i++)
    {
        MuxSet.gop[i] = pGopMuxConfig->GopMux[i].u8GopIndex;
        MuxSet.mux[i] = pGopMuxConfig->GopMux[i].u8MuxIndex;
    }
#ifdef CONFIG_GOP_UTOPIA10
    if(Ioctl_GOP_SetMux(NULL,&MuxSet,sizeof(GOP_SETMUX)) != GOP_API_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN(GOP_API_FAIL);
    }
#else
    GOP_SETMUX_PARAM  ioctl_info;

    memset(&ioctl_info, 0x0, sizeof(GOP_SETMUX_PARAM));

    ioctl_info.pMuxInfo = (MS_U32*)&MuxSet;
    ioctl_info.u32Size  = sizeof(GOP_SETMUX);

    if(UtopiaIoctl(pInstantGOP,MAPI_CMD_GOP_SET_MUX,&ioctl_info)!= UTOPIA_STATUS_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN( GOP_API_FAIL);
    }
#endif
    GOP_RETURN(GOP_API_SUCCESS);
}

/*******************************************************************************/
//Set which OSD Layer select which GOP
//@param pGopLayer \b IN:information about GOP and corresponding Layer
//   #u32LayerCounts: the total GOP/Layer counts to set
//   #stGopLayer[i].u32GopIndex :the GOP which need to change Layer
//   #stGopLayer[i].u32LayerIndex :the GOP corresponding Layer
//@return GOP_API_SUCCESS - Success
/*******************************************************************************/
E_GOP_API_Result MApi_GOP_GWIN_SetLayer(GOP_LayerConfig *pGopLayer, MS_U32 u32SizeOfLayerInfo)
{
    GOP_SETLayer  stLayerSetting;
    MS_U32 i;

    GOP_ENTRY();

    memset(&stLayerSetting, 0x0, sizeof(GOP_SETLayer));

    stLayerSetting.u32LayerCount = pGopLayer->u32LayerCounts;

    for(i=0; i< stLayerSetting.u32LayerCount; i++)
    {
        stLayerSetting.u32Gop[i] = pGopLayer->stGopLayer[i].u32GopIndex;
        stLayerSetting.u32Layer[i] = pGopLayer->stGopLayer[i].u32LayerIndex;
    }
#ifdef CONFIG_GOP_UTOPIA10
    if(Ioctl_GOP_SetLayer(NULL,&stLayerSetting,sizeof(GOP_SETLayer)) != GOP_API_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN(GOP_API_FAIL);
    }
#else
    GOP_SETLAYER_PARAM  ioctl_info;

    memset(&ioctl_info, 0x0, sizeof(GOP_SETLAYER_PARAM));

    ioctl_info.pLayerInfo= (MS_U32*)&stLayerSetting;
    ioctl_info.u32Size  = sizeof(GOP_SETLayer);

    if(UtopiaIoctl(pInstantGOP,MAPI_CMD_GOP_SET_LAYER,&ioctl_info)!= UTOPIA_STATUS_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN( GOP_API_FAIL);
    }
#endif
    GOP_RETURN(GOP_API_SUCCESS);
}
#ifndef CONFIG_GOP_REDUCE_CODESIZE_FOR_MBOOT

/*******************************************************************************/
//Get which OSD Layer select which GOP
//@param pGopLayer \b OUT:information about GOP and corresponding Layer
//   #u32LayerCounts: the total GOP/Layer inited
//   #stGopLayer[i].u32GopIndex :the GOP which need to change Layer
//   #stGopLayer[i].u32LayerIndex :the GOP corresponding Layer
//@return GOP_API_SUCCESS - Success
/*******************************************************************************/
E_GOP_API_Result MApi_GOP_GWIN_GetLayer(GOP_LayerConfig *pGopLayer, MS_U32 u32SizeOfLayerInfo)
{
    GOP_SETLayer  stLayerSetting;

    GOP_ENTRY();
    memset(&stLayerSetting, 0x0, sizeof(GOP_SETLayer));
#ifdef CONFIG_GOP_UTOPIA10
    if(Ioctl_GOP_GetLayer(NULL,&stLayerSetting,sizeof(GOP_SETLayer)) != GOP_API_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN(GOP_API_FAIL);
    }
#else
    GOP_SETLAYER_PARAM  ioctl_info;

    memset(&ioctl_info, 0x0, sizeof(GOP_SETLAYER_PARAM));

    ioctl_info.pLayerInfo= (MS_U32*)&stLayerSetting;
    ioctl_info.u32Size  = sizeof(GOP_SETLayer);

    if(UtopiaIoctl(pInstantGOP,MAPI_CMD_GOP_GET_LAYER,&ioctl_info)!= UTOPIA_STATUS_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN( GOP_API_FAIL);
    }
#endif
    if (pGopLayer != NULL)
    {
        MS_U32 u32GOP;
        pGopLayer->u32LayerCounts = stLayerSetting.u32LayerCount;
        for (u32GOP = 0; u32GOP < pGopLayer->u32LayerCounts; u32GOP++)
        {
            pGopLayer->stGopLayer[u32GOP].u32GopIndex = stLayerSetting.u32Gop[u32GOP];
            pGopLayer->stGopLayer[u32GOP].u32LayerIndex = stLayerSetting.u32Layer[u32GOP];
        }
    }

    GOP_RETURN(GOP_API_SUCCESS);
}

/*******************************************************************************/
//Get Layer From GOP
//@param u32GOP \b IN: gop id
//@return MS_U32 - the layer id which the gop is on, LAYER_ID_INVALID for invalid
/*******************************************************************************/
MS_U32 MApi_GOP_GWIN_GetLayerFromGOP(MS_U32 u32GOP)
{
    GOP_LayerConfig stGOPLayerConfig;
    MS_U32 u32Layer = LAYER_ID_INVALID;
    memset(&stGOPLayerConfig, 0, sizeof(GOP_LayerConfig));
    if (GOP_API_SUCCESS == MApi_GOP_GWIN_GetLayer(&stGOPLayerConfig, sizeof(GOP_LayerConfig)))
    {
        MS_U32 u32Index;
        for (u32Index = 0; u32Index < stGOPLayerConfig.u32LayerCounts; u32Index++)
        {
            if (stGOPLayerConfig.stGopLayer[u32Index].u32GopIndex == u32GOP)
            {
                u32Layer = stGOPLayerConfig.stGopLayer[u32Index].u32LayerIndex;
                break;
            }
        }
    }
    return u32Layer;
}

/*******************************************************************************/
//Get GOP From layer
//@param u32Layer \b IN: layer id
//@return MS_U32 - the gop id which the layer have,  INVALID_GOP_NUM for invalid
/*******************************************************************************/
MS_U32 MApi_GOP_GWIN_GetGOPFromLayer(MS_U32 u32Layer)
{
    MS_U32 u32GOP = INVALID_GOP_NUM;
    GOP_LayerConfig stGOPLayerConfig;
    memset(&stGOPLayerConfig, 0, sizeof(GOP_LayerConfig));
    if (GOP_API_SUCCESS == MApi_GOP_GWIN_GetLayer(&stGOPLayerConfig, sizeof(GOP_LayerConfig)))
    {
        MS_U32 u32Index;
        for (u32Index = 0; u32Index < stGOPLayerConfig.u32LayerCounts; u32Index++)
        {
            if (stGOPLayerConfig.stGopLayer[u32Index].u32LayerIndex == u32Layer)
            {
                u32GOP = stGOPLayerConfig.stGopLayer[u32Index].u32GopIndex;
                break;
            }
        }
    }

    return u32GOP;
}
#endif

/******************************************************************************/
/// Switch current active GOP
/// @param u8GOP \b IN : Number of GOP
/// @return GOP_API_SUCCESS - Success
/******************************************************************************/
E_GOP_API_Result MApi_GOP_GWIN_SwitchGOP(MS_U8 u8GOP)
{
    GOP_CBFmtInfo* pCBFmtInfo=NULL;

    GOP_ENTRY();
    pCBFmtInfo = (PGOP_CBFmtInfo)malloc(sizeof(GOP_CBFmtInfo));

#ifdef CONFIG_GOP_UTOPIA10
    if(Ioctl_GOP_Select(NULL,EN_GOP_SEL_GOP,u8GOP,(MS_U32*)pCBFmtInfo) != GOP_API_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        free(pCBFmtInfo);
        GOP_RETURN(GOP_API_FAIL);
    }
#else
    GOP_SELECTION_PROPERTY_PARAM ioctl_info;
    memset(&ioctl_info, 0x0, sizeof(GOP_SELECTION_PROPERTY_PARAM));

    ioctl_info.sel_type = EN_GOP_SEL_GOP;
    ioctl_info.id = u8GOP;
    ioctl_info.pinfo = (MS_U32*)pCBFmtInfo;

    if(UtopiaIoctl(pInstantGOP,MAPI_CMD_GOP_SELECTION,&ioctl_info)!= UTOPIA_STATUS_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        free(pCBFmtInfo);
        GOP_RETURN(GOP_API_FAIL);
    }
#endif
    free(pCBFmtInfo);

    GOP_RETURN(GOP_API_SUCCESS);
}

/******************************************************************************/
/// Configure the destination of a specific GOP
/// @param u8GOP \b IN : Number of GOP
/// @param dsttype \b IN : GOP destination
/// @return GOP_API_SUCCESS - Success
/// @return GOP_API_ENUM_NOT_SUPPORTED - GOP destination not support
/******************************************************************************/
E_GOP_API_Result MApi_GOP_GWIN_SetGOPDst(MS_U8 u8GOP, EN_GOP_DST_TYPE dsttype)
{
    GOP_ENTRY();
#ifdef CONFIG_GOP_UTOPIA10
    if(Ioctl_GOP_SetDst(NULL,u8GOP,dsttype) != GOP_API_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN(GOP_API_FAIL);
    }
#else
    GOP_SETDST_PARAM ioctl_info;

    memset(&ioctl_info, 0x0, sizeof(GOP_SETDST_PARAM));

    ioctl_info.en_dst  = dsttype;
    ioctl_info.gop_idx = u8GOP;

    if(UtopiaIoctl(pInstantGOP,MAPI_CMD_GOP_SET_DST,&ioctl_info)!= UTOPIA_STATUS_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN (GOP_API_FAIL);
    }
#endif
    GOP_RETURN( GOP_API_SUCCESS);
}


/*****************************************************************************/
/// Get the free GWIN id
/// @return the GWIN id
/*****************************************************************************/
MS_U8 MApi_GOP_GWIN_GetFreeWinID(void)
{
	MS_U32 u32FreeId = (MS_U32)INVALID_POOL_NEXT_FBID;

    GOP_ENTRY();

#ifdef CONFIG_GOP_UTOPIA10
    if(Ioctl_GOP_GWin_GetProperty(NULL,E_GOP_GWIN_FREE_ID,0x0, &u32FreeId,sizeof(MS_U8)) != GOP_API_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN(GOP_API_FAIL);
    }
#else
    GOP_GWIN_PROPERTY_PARAM ioctl_info;
    memset(&ioctl_info, 0x0, sizeof(GOP_GWIN_PROPERTY_PARAM));

    ioctl_info.en_property = E_GOP_GWIN_FREE_ID;
    ioctl_info.GwinId = 0x0;
    ioctl_info.pSet = (void*)&u32FreeId;
    ioctl_info.u32Size = sizeof(MS_U8);

    if (UtopiaIoctl(pInstantGOP,MAPI_CMD_GOP_GWIN_GET_PROPERTY,(void*)&ioctl_info) != UTOPIA_STATUS_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN((MS_U8)u32FreeId);
    }
#endif
    GOP_RETURN((MS_U8)u32FreeId);

}

/******************************************************************************/
/// Check if a frame buffer is allocated
/// @param fbId \b IN frame buffer id
/// @return
///   - # TRUE fbId is allocated
///   - # FALSE fbId is invalid or not allocated
/******************************************************************************/
MS_U8 MApi_GOP_GWIN_IsFBExist(MS_U8 fbId)
{
    return MApi_GOP_GWIN_Is32FBExist((MS_U32)fbId);
}

MS_U8 MApi_GOP_GWIN_Is32FBExist(MS_U32 u32fbId)
{
	MS_U32 u32Exist = FALSE;
    GOP_ENTRY();

#ifdef CONFIG_GOP_UTOPIA10
    if(Ioctl_GOP_FB_GetProperty(NULL,E_GOP_FB_EXIST,u32fbId, &u32Exist,sizeof(MS_U8)) != GOP_API_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN(GOP_API_FAIL);
    }
#else
    GOP_FB_PROPERTY_PARAM ioctl_info;
    memset(&ioctl_info, 0x0, sizeof(GOP_FB_PROPERTY_PARAM));

    ioctl_info.en_property = E_GOP_FB_EXIST;
    ioctl_info.FBId = u32fbId;
    ioctl_info.pSet = (void*)&u32Exist;
    ioctl_info.u32Size = sizeof(MS_U8);

    if (UtopiaIoctl(pInstantGOP,MAPI_CMD_GOP_FB_GET_PROPERTY,(void*)&ioctl_info) != UTOPIA_STATUS_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN((MS_U8)u32Exist);
    }
#endif
    GOP_RETURN((MS_U8)u32Exist);
}



/******************************************************************************/
/// Get free frame buffer id
/// @return frame buffer id. If return oxFF, it represents no free frame buffer id for use.
/******************************************************************************/
MS_U8 MApi_GOP_GWIN_GetFreeFBID(void)
{
    MS_U32 u32FreeFBId;

    u32FreeFBId = MApi_GOP_GWIN_GetFree32FBID();
    if( u32FreeFBId > 256) //Out of MS_U8
    {
        GOP_WARN( "[Warning] %s %d( u32FreeFBId:%td....)out of MS_U8 bound, please to use MApi_GOP_GWIN_GetFree32FBID()\n",__FUNCTION__,__LINE__,(ptrdiff_t)u32FreeFBId );
    }

    return u32FreeFBId;
}

MS_U32 MApi_GOP_GWIN_GetFree32FBID(void)
{
    MS_U32 u32FBId = 0xFFFFFFFF;
    GOP_ENTRY();
#ifdef CONFIG_GOP_UTOPIA10
    if(Ioctl_GOP_FB_GetProperty(NULL,E_GOP_FB_OBTAIN,0x0,(MS_U32*)&u32FBId,sizeof(MS_U32)) != GOP_API_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN(GOP_API_FAIL);
    }
#else
    GOP_FB_PROPERTY_PARAM ioctl_info;
    memset(&ioctl_info, 0x0, sizeof(GOP_FB_PROPERTY_PARAM));

    ioctl_info.en_property = E_GOP_FB_OBTAIN;
    ioctl_info.FBId = 0x0;
    ioctl_info.pSet = (MS_U32*)&u32FBId;
    ioctl_info.u32Size = sizeof(MS_U32);

    if (UtopiaIoctl(pInstantGOP,MAPI_CMD_GOP_FB_GET_PROPERTY,(void*)&ioctl_info) != UTOPIA_STATUS_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN(u32FBId);
    }
#endif
    GOP_RETURN(u32FBId);
}

/******************************************************************************/
/// Destroy the frame buffer and return the memory to mmgr
/// @param fbId  \b IN frame buffer id
/// @return GOP_API_SUCCESS - Success
/// @return GOP_API_CRT_GWIN_NOAVAIL - destory frame buffer fail
/******************************************************************************/
MS_U8 MApi_GOP_GWIN_DestroyFB(MS_U8 fbId)
{
    return MApi_GOP_GWIN_Destroy32FB((MS_U32)fbId);
}

MS_U8 MApi_GOP_GWIN_Destroy32FB(MS_U32 u32fbId)
{
    GOP_ENTRY();
#ifdef CONFIG_GOP_UTOPIA10
    if(Ioctl_GOP_FBDestroy(NULL,(MS_U32*)&u32fbId) != GOP_API_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN(GOP_API_FAIL);
    }
#else
    GOP_DELETE_BUFFER_PARAM ioctl_info;

    memset(&ioctl_info, 0x0, sizeof(GOP_DELETE_BUFFER_PARAM));

    ioctl_info.pBufId = (MS_U32*)&u32fbId;
    ioctl_info.u32Size = sizeof(MS_U32);

    if(UtopiaIoctl(pInstantGOP,MAPI_CMD_GOP_FB_DESTROY,&ioctl_info)!= UTOPIA_STATUS_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN( GOP_API_FAIL);
    }
#endif
    GOP_RETURN(GWIN_OK);
}
#ifndef CONFIG_GOP_REDUCE_CODESIZE_FOR_MBOOT


/******************************************************************************/
/// Destroy a frame buffer
/// @param fbId \b IN frame buffer id
/// @return GOP_API_SUCCESS - Success
/******************************************************************************/
E_GOP_API_Result MApi_GOP_GWIN_DeleteFB(MS_U8 fbId)
{
    return MApi_GOP_GWIN_Delete32FB((MS_U32)fbId);
}

E_GOP_API_Result MApi_GOP_GWIN_Delete32FB(MS_U32 u32fbId)
{
    GOP_ENTRY();
    GOP_INFO("\33[0;36m   %s:%d   FBId = %td \33[m \n",__FUNCTION__,__LINE__, (ptrdiff_t)u32fbId);
    if ((MApi_GOP_GWIN_Destroy32FB(u32fbId)) != GWIN_OK)
    {
        GOP_RETURN(GOP_API_FAIL);
    }
    GOP_RETURN(GOP_API_SUCCESS);

}
#endif
/********************************************************************************/
/// Set GWIN alpha blending
/// @param u8win \b IN GWIN id
/// @param bEnable \b IN
///   - # TRUE enable pixel alpha
///   - # FALSE disable pixel alpha
/// @param u8coef \b IN alpha blending coefficient (0-7)
/// @return GOP_API_SUCCESS - Success
/********************************************************************************/
E_GOP_API_Result MApi_GOP_GWIN_SetBlending(MS_U8 u8win, MS_BOOL bEnable, MS_U8 u8coef)
{
    GOP_GWIN_BLENDING blendInfo;
    GOP_ENTRY();

    blendInfo.Coef = u8coef;
    blendInfo.bEn = bEnable;
#ifdef CONFIG_GOP_UTOPIA10
    if(Ioctl_GOP_GWin_SetProperty(NULL,E_GOP_GWIN_BLENDING,u8win,(MS_U32*)&blendInfo,sizeof(GOP_GWIN_BLENDING)) != GOP_API_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN(GOP_API_FAIL);
    }
#else
    GOP_GWIN_PROPERTY_PARAM ioctl_info;
    memset(&ioctl_info, 0x0, sizeof(GOP_GWIN_PROPERTY_PARAM));

    ioctl_info.en_property = E_GOP_GWIN_BLENDING;
    ioctl_info.GwinId = u8win;
    ioctl_info.pSet = (MS_U32*)&blendInfo;
    ioctl_info.u32Size = sizeof(GOP_GWIN_BLENDING);

    if (UtopiaIoctl(pInstantGOP,MAPI_CMD_GOP_GWIN_SET_PROPERTY,(void*)&ioctl_info) != UTOPIA_STATUS_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN(GOP_API_FAIL);
    }
#endif
    GOP_RETURN(GOP_API_SUCCESS);

}

#ifndef CONFIG_GOP_REDUCE_CODESIZE_FOR_MBOOT

/********************************************************************************/
/// Set transparent color for the GWIN
/// @param fmt \b IN:
///  # GOPTRANSCLR_FMT0: RGB mode
///  # GOPTRANSCLR_FMT1: index mode
/// @param clr \b IN Transparent color for in specific color format. Note that this is only for
/// GOPTRANSCLR_FMT0(RGB mode) use. If you use GOPTRANSCLR_FMT1 (index mode), do not care this
/// @return GOP_API_SUCCESS - Success
/********************************************************************************/
E_GOP_API_Result MApi_GOP_GWIN_SetTransClr_EX(MS_U8 u8GOP,EN_GOP_TRANSCLR_FMT fmt, MS_U16 clr)
{
    GOP_TRANSCOLOR trans_set;

    GOP_ENTRY();

    trans_set.bEn = TRUE;
    trans_set.color = clr;
    trans_set.fmt = fmt;
    trans_set.transclr_property = EN_TRANSCLR_SET_COLOR;

    u32TransColor = clr;
#ifdef CONFIG_GOP_UTOPIA10
    if(Ioctl_GOP_SetProperty(NULL,E_GOP_TRANSCOLOR,u8GOP,(MS_U32*)&trans_set,sizeof(GOP_TRANSCOLOR)) != GOP_API_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN(GOP_API_FAIL);
    }
#else

    GOP_SET_PROPERTY_PARAM ioctl_info;
    memset(&ioctl_info, 0x0, sizeof(GOP_SET_PROPERTY_PARAM));

    ioctl_info.en_pro = E_GOP_TRANSCOLOR;
    ioctl_info.gop_idx = u8GOP;
    ioctl_info.pSetting = (void*)&trans_set;
    ioctl_info.u32Size = sizeof(GOP_TRANSCOLOR);

    if (UtopiaIoctl(pInstantGOP,MAPI_CMD_GOP_SET_PROPERTY,(void*)&ioctl_info) != UTOPIA_STATUS_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN(GOP_API_FAIL);
    }
#endif
    GOP_RETURN(GOP_API_SUCCESS);

}

E_GOP_API_Result MApi_GOP_GWIN_SetTransClr(EN_GOP_TRANSCLR_FMT fmt, MS_U16 clr)
{
    MS_U8 u8GOP;
    E_GOP_API_Result ret;

    u8GOP = MApi_GOP_GWIN_GetCurrentGOP();
    ret = MApi_GOP_GWIN_SetTransClr_EX(u8GOP,fmt,clr);
    return ret;

}
/********************************************************************************/
/// Set transparent color for the GWIN
/// @param fmt \b IN GOP number
/// @param fmt \b IN:
///  # GOPTRANSCLR_FMT0: RGB mode
///  # GOPTRANSCLR_FMT1: index mode
///  # GOPTRANSCLR_FMT2: ARGB8888 mode
///  # GOPTRANSCLR_FMT3: YUV mode
/// @param ST_Transparent_Color \b IN This structure has an element color which is used for trans color.
/// @return GOP_API_SUCCESS - Success
/********************************************************************************/
E_GOP_API_Result MApi_GOP_GWIN_SetTransparentClr(MS_U8 u8GOP, EN_GOP_TRANSCLR_FMT fmt, ST_Transparent_Color *st_Transcolor)
{
    GOP_TRANSCOLOR trans_set;

    GOP_ENTRY();

    trans_set.bEn = TRUE;
    trans_set.color = st_Transcolor->color;
    trans_set.fmt = fmt;
    trans_set.transclr_property = EN_TRANSCLR_SET_COLOR;

    u32TransColor = st_Transcolor->color;
#ifdef CONFIG_GOP_UTOPIA10
    if(Ioctl_GOP_SetProperty(NULL,E_GOP_TRANSCOLOR,u8GOP,(MS_U32*)&trans_set,sizeof(GOP_TRANSCOLOR)) != GOP_API_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN(GOP_API_FAIL);
    }
#else

    GOP_SET_PROPERTY_PARAM ioctl_info;
    memset(&ioctl_info, 0x0, sizeof(GOP_SET_PROPERTY_PARAM));

    ioctl_info.en_pro = E_GOP_TRANSCOLOR;
    ioctl_info.gop_idx = u8GOP;
    ioctl_info.pSetting = (void*)&trans_set;
    ioctl_info.u32Size = sizeof(GOP_TRANSCOLOR);

    if (UtopiaIoctl(pInstantGOP,MAPI_CMD_GOP_SET_PROPERTY,(void*)&ioctl_info) != UTOPIA_STATUS_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN(GOP_API_FAIL);
    }
#endif
    GOP_RETURN(GOP_API_SUCCESS);
}
/********************************************************************************/
/// Set transparent color (ARGB domain) and enable it for the GWIN
/// @param clr \b IN: transparent color
/// @return GOP_API_SUCCESS - Success
/********************************************************************************/
E_GOP_API_Result MApi_GOP_GWIN_SetFMT0TransClr_EX(MS_U8 u8GOP,MS_U16 clr)
{
    GOP_TRANSCOLOR trans_set;

    GOP_ENTRY();

    trans_set.bEn = TRUE;
    trans_set.color = clr;
    trans_set.fmt = GOPTRANSCLR_FMT0;
    trans_set.transclr_property = EN_TRANSCLR_SET_COLOR;

    u32TransColor = clr;
#ifdef CONFIG_GOP_UTOPIA10
    if(Ioctl_GOP_SetProperty(NULL,E_GOP_TRANSCOLOR,u8GOP,(MS_U32*)&trans_set,sizeof(GOP_TRANSCOLOR)) != GOP_API_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN(GOP_API_FAIL);
    }
#else
    GOP_SET_PROPERTY_PARAM ioctl_info;
    memset(&ioctl_info, 0x0, sizeof(GOP_SET_PROPERTY_PARAM));

    ioctl_info.en_pro = E_GOP_TRANSCOLOR;
    ioctl_info.gop_idx = u8GOP;
    ioctl_info.pSetting = (void*)&trans_set;
    ioctl_info.u32Size = sizeof(GOP_TRANSCOLOR);

    if (UtopiaIoctl(pInstantGOP,MAPI_CMD_GOP_SET_PROPERTY,(void*)&ioctl_info) != UTOPIA_STATUS_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN(GOP_API_FAIL);
    }
#endif
    GOP_RETURN(GOP_API_SUCCESS);
}
E_GOP_API_Result MApi_GOP_GWIN_SetFMT0TransClr(MS_U16 clr)
{
    MS_U8 u8GOP;
    E_GOP_API_Result ret;

    u8GOP = MApi_GOP_GWIN_GetCurrentGOP();
    ret = MApi_GOP_GWIN_SetFMT0TransClr_EX(u8GOP,clr);
    return ret;
}
//-------------------------------------------------------------------------------------------------
/// Get maximum support frame buffer number
/// @return maximum support frame buffer number
//-------------------------------------------------------------------------------------------------
MS_U8 MApi_GOP_GWIN_GetMAXFBID(void)
{
    MS_U32 u32MaxFBID;
    u32MaxFBID = MApi_GOP_GWIN_GetMAX32FBID();
    if( u32MaxFBID> 256) //Out of MS_U8
    {
        GOP_WARN( "[Warning] %s %d( u32MaxFBID:%td....)out of MS_U8 bound, please to use MApi_GOP_GWIN_GetMAX32FBID()\n",__FUNCTION__,__LINE__,(ptrdiff_t)u32MaxFBID );
    }
    return u32MaxFBID;
}

MS_U32 MApi_GOP_GWIN_GetMAX32FBID(void)
{
    return MApi_GOP_GWIN_GetMax32FBNum();
}

//-------------------------------------------------------------------------------------------------
/// Get maximum support gop number
/// @return maximum support gop number
//-------------------------------------------------------------------------------------------------
MS_U8 MApi_GOP_GWIN_GetMaxGOPNum(void)
{
    MS_U8 u8MaxGop = 0;
#ifdef CONFIG_GOP_UTOPIA10
    if(Ioctl_GOP_GetStatus(NULL,E_GOP_STATUS_GOP_MAXNUM,(void*) &u8MaxGop,sizeof(MS_U8)) != GOP_API_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN(GOP_API_FAIL);
    }
#else
    GOP_GET_STATUS_PARAM ioctl_info;
    memset(&ioctl_info, 0x0, sizeof(GOP_GET_STATUS_PARAM));

    CheckGOPInstanceOpen();

    ioctl_info.type = 0;
    ioctl_info.en_status = E_GOP_STATUS_GOP_MAXNUM;
    ioctl_info.pStatus = (void*) &u8MaxGop;
    ioctl_info.u32Size = sizeof(MS_U8);

    if (UtopiaIoctl(pInstantGOP,MAPI_CMD_GOP_GET_STATUS,(void*)&ioctl_info) != UTOPIA_STATUS_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        return GOP_API_FAIL;
    }
#endif
    return u8MaxGop;

}

#endif
//-------------------------------------------------------------------------------------------------
/// Get maximum gwin number by individual gop
/// @param u8GopNum \b IN: Number of GOP
/// @return maximum gwin number by individual gop
//-------------------------------------------------------------------------------------------------
MS_U8 MApi_GOP_GWIN_GetGwinNum(MS_U8 u8GopNum)
{
    GOP_GWIN_NUM  gwin_num;

    gwin_num.gop_idx = u8GopNum;
#ifdef CONFIG_GOP_UTOPIA10
    if(Ioctl_GOP_GetStatus(NULL,E_GOP_STATUS_GWIN_MAXNUM,(MS_U32*)&gwin_num,sizeof(GOP_GWIN_NUM)) != GOP_API_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN(GOP_API_FAIL);
    }
#else
    GOP_GET_STATUS_PARAM ioctl_info;
    memset(&ioctl_info, 0x0, sizeof(GOP_GET_STATUS_PARAM));

    CheckGOPInstanceOpen();
    ioctl_info.type = 0;
    ioctl_info.en_status = E_GOP_STATUS_GWIN_MAXNUM;
    ioctl_info.pStatus = (MS_U32*)&gwin_num;
    ioctl_info.u32Size = sizeof(GOP_GWIN_NUM);

    if (UtopiaIoctl(pInstantGOP,MAPI_CMD_GOP_GET_STATUS,(void*)&ioctl_info) != UTOPIA_STATUS_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        return GOP_API_FAIL;
    }
#endif
    return gwin_num.gwin_num;
}

#ifndef CONFIG_GOP_REDUCE_CODESIZE_FOR_MBOOT

//-------------------------------------------------------------------------------------------------
/// Get maximum support gwin number by all gop
/// @return maximum support gwin number by all gop
//-------------------------------------------------------------------------------------------------
MS_U8 MApi_GOP_GWIN_GetTotalGwinNum(void)
{
    MS_U8 u8TotalWin = 0;

#ifdef CONFIG_GOP_UTOPIA10
    if(Ioctl_GOP_GetStatus(NULL,E_GOP_STATUS_GWIN_TOTALNUM,(void*)&u8TotalWin,sizeof(MS_U8)) != GOP_API_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN(GOP_API_FAIL);
    }
#else
    GOP_GET_STATUS_PARAM ioctl_info;
    memset(&ioctl_info, 0x0, sizeof(GOP_GET_STATUS_PARAM));

    CheckGOPInstanceOpen();
    ioctl_info.type = 0;
    ioctl_info.en_status = E_GOP_STATUS_GWIN_TOTALNUM;
    ioctl_info.pStatus = (void*)&u8TotalWin;
    ioctl_info.u32Size = sizeof(MS_U8);
    if (UtopiaIoctl(pInstantGOP,MAPI_CMD_GOP_GET_STATUS,(void*)&ioctl_info) != UTOPIA_STATUS_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        return GOP_API_FAIL;
    }
#endif
    return u8TotalWin;
}


/*****************************************************************************/
/// Get max GWIN id
/// @return the maximum number of GWIN id
/*****************************************************************************/
MS_U8 MApi_GOP_GWIN_GetMAXWinID(void)
{
    return MApi_GOP_GWIN_GetTotalGwinNum();
}

/******************************************************************************/
/// Set alpha value of GOP0 4 Gwins
/// @param Gwin01AlphaValue \b IN: Alpha coefficient and Enable status of Gwin 0/1
/// @param Gwin23AlphaValue \b IN: Alpha coefficient and Enable status of Gwin 2/3
/// @return GOP_API_SUCCESS - Success
/******************************************************************************/
E_GOP_API_Result MApi_GOP_GWIN_SetAlphaValue(MS_U16 * Gwin01AlphaValue, MS_U16 * Gwin23AlphaValue)
{
    GOP_ENTRY();

    GOP_ERR("[%s] Not Support \n",__FUNCTION__);

    GOP_RETURN(GOP_API_SUCCESS);

}


/********************************************************************************/
/// Get GOP0 4 gwin alpha blending setting
/// @param Gwin01AlphaValue \b OUT: gwin0 and gwin1 alpha setting
/// @param Gwin23AlphaValue \b OUT: gwin2 and gwin3 alpha setting
/// @return GOP_API_SUCCESS - Success
/********************************************************************************/
E_GOP_API_Result MApi_GOP_GWIN_GetAlphaValue(MS_U16 * Gwin01AlphaValue, MS_U16 * Gwin23AlphaValue)
{
    GOP_ENTRY();

    GOP_ERR("[%s] Not Support \n",__FUNCTION__);

    GOP_RETURN(GOP_API_SUCCESS);

}

E_GOP_API_Result MApi_GOP_GWIN_SetPreAlphaMode(MS_U8 u8GOP, MS_BOOL bEnble)
{
	MS_U32 u32PreAlpha;

    GOP_ENTRY();

	u32PreAlpha = (MS_U32)bEnble;
#ifdef CONFIG_GOP_UTOPIA10
    if(Ioctl_GOP_SetProperty(NULL,E_GOP_PREALPHAMODE,u8GOP, &u32PreAlpha,sizeof(MS_BOOL)) != GOP_API_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN(GOP_API_FAIL);
    }
#else

    GOP_SET_PROPERTY_PARAM ioctl_info;
    memset(&ioctl_info, 0x0, sizeof(GOP_SET_PROPERTY_PARAM));

    ioctl_info.en_pro   = E_GOP_PREALPHAMODE;
    ioctl_info.gop_idx  = u8GOP;
    ioctl_info.pSetting = (void*)&u32PreAlpha;
    ioctl_info.u32Size  = sizeof(MS_BOOL);

    if(UtopiaIoctl(pInstantGOP,MAPI_CMD_GOP_SET_PROPERTY,&ioctl_info)!= UTOPIA_STATUS_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN( GOP_API_FAIL);
    }
#endif
    GOP_RETURN(GOP_API_SUCCESS);

}

/******************************************************************************/
/// Get GWin position info
/// @param winId  \b IN: gwin id
/// @param dispX \b OUT: position-X
/// @param dispY \b OUT: position-Y
/// @return GOP_API_SUCCESS - Success
/// @return GOP_API_FAIL - Failure
/******************************************************************************/
E_GOP_API_Result MApi_GOP_GWIN_GetWinPosition(MS_U8 winId, MS_U16 * dispX, MS_U16 *dispY)
{
    GOP_ENTRY();
    GOP_GwinInfo info;

    memset(&info, 0x0, sizeof(GOP_GwinInfo));
    MApi_GOP_GWIN_GetWinInfo(winId,&info);
    *dispX = info.u16DispHPixelStart;
    *dispY = info.u16DispVPixelStart;
    GOP_RETURN(GOP_API_SUCCESS);
}


/******************************************************************************/
/// Get frame buffer address
/// @param fbID \b IN: frame buffer id
/// @param pAddr \b OUT: frame buffer address
/// @return GOP_API_SUCCESS - Success
/******************************************************************************/
E_GOP_API_Result MApi_GOP_GWIN_GetFBAddr(MS_U8 fbID, MS_PHY *pAddr)
{
    return MApi_GOP_GWIN_Get32FBAddr((MS_U32)fbID, pAddr);
}

E_GOP_API_Result MApi_GOP_GWIN_Get32FBAddr(MS_U32 u32fbID, MS_PHY *pAddr)
{
    GOP_GwinFBAttr fbAttr;
    GOP_ENTRY();

    if(MApi_GOP_GWIN_Get32FBInfo(u32fbID, &fbAttr) == GOP_API_SUCCESS)
    {
        *pAddr = fbAttr.addr;
        GOP_RETURN(GOP_API_SUCCESS);
    }
    else
    {
        GOP_RETURN(GOP_API_FAIL);
    }

}

/******************************************************************************/
/// Reset the gwin info. Note that no free any frame buffer in this interface!!
/// @param gId  \b IN: gwin id
/// @return GOP_API_SUCCESS - Success
/// @return GOP_API_FAIL - Failure
/******************************************************************************/
E_GOP_API_Result MApi_GOP_GWIN_ReleaseWin(MS_U8 gId)
{
    GOP_ENTRY();

    MApi_GOP_GWIN_Enable(gId, FALSE);
    MApi_GOP_GWIN_DestroyWin(gId);

    GOP_RETURN(GOP_API_SUCCESS);

}


/******************************************************************************/
/// Get frame bufferid from the gwin id
/// @param gwinId  \b IN gwin id
/// @return return frame buffer id. If return 0xFF, it represents this function fail
/******************************************************************************/
MS_U8 MApi_GOP_GWIN_GetFBfromGWIN(MS_U8 gwinId)
{
    MS_U32 u32CurFBId;

    u32CurFBId = MApi_GOP_GWIN_Get32FBfromGWIN(gwinId);
    if( u32CurFBId > 256) //Out of MS_U8
    {
        GOP_WARN( "[Warning] %s %d( u32CurFBId:%td....)out of MS_U8 bound, please to use MApi_GOP_GWIN_Get32FBfromGWIN()\n",__FUNCTION__,__LINE__,(ptrdiff_t)u32CurFBId );
    }
    return u32CurFBId;
}
#endif
MS_U32 MApi_GOP_GWIN_Get32FBfromGWIN(MS_U8 gwinId)
{
    MS_U32 u32FB = INVALID_POOL_NEXT_FBID;
    GOP_ENTRY();
#ifdef CONFIG_GOP_UTOPIA10
    if(Ioctl_GOP_GWin_GetProperty(NULL,E_GOP_GWIN_GET_FB,gwinId,(MS_U32*)&u32FB,sizeof(MS_U32)) != GOP_API_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN(GOP_API_FAIL);
    }
#else
    GOP_GWIN_PROPERTY_PARAM ioctl_info;
    memset(&ioctl_info, 0x0, sizeof(GOP_GWIN_PROPERTY_PARAM));

    ioctl_info.en_property = E_GOP_GWIN_GET_FB;
    ioctl_info.GwinId = gwinId;
    ioctl_info.pSet = (MS_U32*)&u32FB;
    ioctl_info.u32Size = sizeof(MS_U32);

    if (UtopiaIoctl(pInstantGOP,MAPI_CMD_GOP_GWIN_GET_PROPERTY,(void*)&ioctl_info) != UTOPIA_STATUS_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN(u32FB);
    }
#endif
    GOP_RETURN(u32FB);

}
#ifndef CONFIG_GOP_REDUCE_CODESIZE_FOR_MBOOT


/******************************************************************************/
/// Get region info for a frame buffer
/// @param fbID  \b IN frame buffer id 0 ~ (MAX_GWIN_FB_SUPPORT - 1)
/// @param pX \b OUT position-X
/// @param pY \b OUT position-Y
/// @param pW \b OUT width
/// @param pH \b OUT height
/// @return GOP_API_SUCCESS - Success
/******************************************************************************/
E_GOP_API_Result MApi_GOP_GWIN_GetFBRegion(MS_U8 fbID, MS_U16 *pX, MS_U16 *pY, MS_U16 *pW, MS_U16 *pH)
{
    return MApi_GOP_GWIN_Get32FBRegion((MS_U32)fbID, pX, pY, pW, pH);
}

E_GOP_API_Result MApi_GOP_GWIN_Get32FBRegion(MS_U32 u32fbID, MS_U16 *pX, MS_U16 *pY, MS_U16 *pW, MS_U16 *pH)
{
    GOP_GwinFBAttr fbAttr;
    GOP_ENTRY();

    if(MApi_GOP_GWIN_Get32FBInfo(u32fbID, &fbAttr) == GOP_API_SUCCESS)
    {
        *pX = fbAttr.x0;
        *pY = fbAttr.y0;
        *pW = fbAttr.width;
        *pH = fbAttr.height;
        GOP_RETURN(GOP_API_SUCCESS);
    }
    else
    {
        GOP_RETURN(GOP_API_FAIL);
    }

}

/******************************************************************************/
/// Get position info for a frame buffer
/// @param fbID  \b IN frame buffer id 0 ~ (MAX_GWIN_FB_SUPPORT - 1)
/// @param pX \b OUT position-X
/// @param pY \b OUT position-Y
/// @return GOP_API_SUCCESS - Success
/******************************************************************************/
E_GOP_API_Result MApi_GOP_GWIN_GetFBPosition(MS_U8 fbID, MS_U16 *pX, MS_U16 *pY)
{
    return MApi_GOP_GWIN_Get32FBPosition((MS_U32)fbID, pX, pY);
}

E_GOP_API_Result MApi_GOP_GWIN_Get32FBPosition(MS_U32 u32fbID, MS_U16 *pX, MS_U16 *pY)
{
    GOP_GwinFBAttr fbAttr;
    GOP_ENTRY();
    if(MApi_GOP_GWIN_Get32FBInfo(u32fbID, &fbAttr) == GOP_API_SUCCESS)
    {
        *pX = fbAttr.x0;
        *pY = fbAttr.y0;
        GOP_RETURN(GOP_API_SUCCESS);
    }
    else
    {
        GOP_RETURN(GOP_API_FAIL);
    }

}

/********************************************************************************/
/// Check if GOP destination is op mode
/// @return TRUE:op mode  FALSE: not in op mode
/********************************************************************************/
MS_BOOL MApi_GOP_GWIN_CheckOpmodeIsOn_EX(MS_U8 u8GOP)
{
    EN_GOP_DST_TYPE enGopDst;

    GOP_ENTRY();
#ifdef CONFIG_GOP_UTOPIA10
    if(Ioctl_GOP_GetDst(NULL,u8GOP,(MS_U32*) &enGopDst) != GOP_API_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN(GOP_API_FAIL);
    }
#else
    GOP_GETDST_PARAM ioctl_info;
    memset(&ioctl_info,0x0, sizeof(GOP_GETDST_PARAM));

    ioctl_info.gop_idx = u8GOP;
    ioctl_info.pDst = (MS_U32*) &enGopDst;
    ioctl_info.u32Size = sizeof(EN_GOP_DST_TYPE);

    if (UtopiaIoctl(pInstantGOP,MAPI_CMD_GOP_GET_DST,(void*)&ioctl_info) != UTOPIA_STATUS_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN(GOP_API_FAIL);
    }
#endif
    if (enGopDst == E_GOP_DST_OP0)
    {
        GOP_RETURN(TRUE);
    }
    else
    {
        GOP_RETURN(FALSE);
    }
}

MS_BOOL MApi_GOP_GWIN_CheckOpmodeIsOn(void)
{
    MS_U8 u8GOP;
    MS_BOOL ret;

    u8GOP = MApi_GOP_GWIN_GetCurrentGOP();
    ret = MApi_GOP_GWIN_CheckOpmodeIsOn_EX(u8GOP);
    return ret;
}
/*****************************************************************************/
/// Get Active GWIN id with max vertical pixel length
/// Active means that displaying OSD currently on screen
/// @return the active GWIN's length  with the maximum vertical pixel length
/*****************************************************************************/
MS_U16 MApi_GOP_GWIN_GetMaxActiveGwin_v(void)
{
    MS_U16 v_length=0;
    MS_U8 i=0;
    GOP_GwinInfo gwin;
    MS_U16 maxvlength=0;
    MS_U16 h_length=0;
    GOP_ENTRY();

    if (MApi_GOP_GWIN_CheckOpmodeIsOn()==false)
    {
        GOP_RETURN(GetMaxActiveGwinFalse_op);
    }
    else
    {
        // Check which GWINS are active
        if (MApi_GOP_GWIN_IsEnabled()==false)
        {
            GOP_RETURN(GetMaxActiveGwinFalse_opened);
        }
        else
        //if gwin are active then compare their vertical size
        // Initialize
        {
            for (i=0;i<MAX_GWIN_SUPPORT;i++)
            {
                //printf("\n(1) i:0x%bx",i);
                if (MApi_GOP_GWIN_IsGWINEnabled(i)==true)
                {
                    memset(&gwin, 0, sizeof(GOP_GwinInfo));
                    MApi_GOP_GWIN_GetWinInfo(i,&gwin);
                    h_length=gwin.u16DispHPixelEnd-gwin.u16DispHPixelStart;
                    if (gwin.u16DispVPixelEnd>gwin.u16DispVPixelStart)
                    {

                        if (h_length>4)
                        {
                            v_length=gwin.u16DispVPixelEnd-gwin.u16DispVPixelStart;
                            if (v_length>maxvlength)
                            {
                                maxvlength=v_length;
                            }
                        }
                    }
                    else
                    {
                        GOP_RETURN(GetMaxActiveGwinFalse);
                    }
                }

            }

            GOP_RETURN(maxvlength);

        }

    }
}


/*****************************************************************************/
/// API for acquiring displaying GWIN's properties
/// @param WinProperty  \b OUT: pointer of GOP_GwinDispProperty
/// @return GOP_API_SUCCESS - Success
/*****************************************************************************/
E_GOP_API_Result MApi_GOP_GWIN_GetWinProperty(GOP_GwinDispProperty *WinProperty)
{
    GOP_GwinInfo gWin;
    GOP_ENTRY();

    memset(&gWin, 0, sizeof(GOP_GwinInfo));

    if(GOP_API_SUCCESS == MApi_GOP_GWIN_GetWinInfo(WinProperty->gId,&gWin))
    {
        WinProperty->StartX=gWin.u16DispHPixelStart;
        WinProperty->StartY=gWin.u16DispVPixelStart;
        //WinProperty->GOPWinWidth=gWin.u16DispVPixelEnd-gWin.u16DispHPixelStart;
        WinProperty->Win_Width=gWin.u16DispHPixelEnd-gWin.u16DispHPixelStart+1;
        WinProperty->Win_Height=gWin.u16DispVPixelEnd-gWin.u16DispVPixelStart+1;
        GOP_RETURN(GOP_API_SUCCESS);
    }
    else
    {
        GOP_RETURN(GOP_API_FAIL);
    }

}

/*****************************************************************************/
/// API for acquiring color format of some Framebuffer
/// @param fbId  \b IN frame buffer id
/// @return the format of Frame buffer. If return 0xF, it represents get frame buffer format fail.
/*****************************************************************************/
MS_U16 MApi_GOP_GWIN_GetFBFmt(MS_U8 fbId)
{
    return MApi_GOP_GWIN_Get32FBFmt((MS_U32)fbId);
}

MS_U16 MApi_GOP_GWIN_Get32FBFmt(MS_U32 u32fbId)
{
   GOP_GwinFBAttr fbAttr;
   GOP_ENTRY();

    if (!MApi_GOP_GWIN_Is32FBExist(u32fbId))
    {
        MS_DEBUG_MSG( GOP_ERR("MApi_GOP_GWIN_Get32FBFmt: u32fbId=%d is not in existence\n",u32fbId));
        GOP_RETURN(GWIN_NO_AVAILABLE);
    }
    else
    {
        if(MApi_GOP_GWIN_Get32FBInfo(u32fbId, &fbAttr) == GOP_API_SUCCESS)
        {
            GOP_RETURN(fbAttr.fbFmt);
        }
        else
        {
            GOP_RETURN(GOP_API_FAIL);
        }
    }
}


/*****************************************************************************/
/// API for acquiring current FB's ID
/// @ return current FB'id. if return is 0xFF, it represents get current frame buffer fail
/*****************************************************************************/
MS_U8 MApi_GOP_GWIN_GetCurrentFBID(void)
{
    MS_U32 u32CurFBID;

    u32CurFBID = MApi_GOP_GWIN_GetCurrent32FBID();
    if( u32CurFBID > 256) //Out of MS_U8
    {
        GOP_WARN( "[Warning] %s %d( u32CurFBID:%td....)out of MS_U8 bound, please to use MApi_GOP_GWIN_GetCurrent32FBID()\n",__FUNCTION__,__LINE__,(ptrdiff_t)u32CurFBID );
    }
    return u32CurFBID;
}

MS_U32 MApi_GOP_GWIN_GetCurrent32FBID(void)
{
    MS_U32 u32FbId = INVALID_POOL_NEXT_FBID;
    GOP_ENTRY();

#ifdef CONFIG_GOP_UTOPIA10
    if(Ioctl_GOP_GetStatus(NULL,E_GOP_STATUS_CURRENT_FBID,(MS_U32*) &u32FbId,sizeof(MS_U32)) != GOP_API_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN(GOP_API_FAIL);
    }
#else

    GOP_GET_STATUS_PARAM ioctl_info;
    memset(&ioctl_info, 0x0, sizeof(GOP_GET_STATUS_PARAM));

    ioctl_info.type = 0;
    ioctl_info.en_status = E_GOP_STATUS_CURRENT_FBID;
    ioctl_info.pStatus = (MS_U32*) &u32FbId;
    ioctl_info.u32Size = sizeof(MS_U32);

    if (UtopiaIoctl(pInstantGOP,MAPI_CMD_GOP_GET_STATUS,(void*)&ioctl_info) != UTOPIA_STATUS_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN(GOP_API_FAIL);
    }
#endif
    GOP_RETURN(u32FbId);

}

//******************************************************************************
/// Enable gop transparent color
/// @param fmt \b IN: RGB mode or u8Index mode color format
/// @param bEnable \b IN:
///   - # TRUE Enable gop transparent color
///   - # FALSE Disable gop transparent color
/// @return GOP_API_SUCCESS - Success
//******************************************************************************
E_GOP_API_Result MApi_GOP_GWIN_EnableTransClr_EX(MS_U8 u8GOP,EN_GOP_TRANSCLR_FMT fmt, MS_BOOL bEnable)
{
    GOP_TRANSCOLOR trans_set;

    GOP_ENTRY();
    trans_set.bEn = bEnable;
    trans_set.color = u32TransColor;
    trans_set.fmt = fmt;
    trans_set.transclr_property =  EN_TRANSCLR_ENABLE;
#ifdef CONFIG_GOP_UTOPIA10
    if(Ioctl_GOP_SetProperty(NULL,E_GOP_TRANSCOLOR,u8GOP,(MS_U32*)&trans_set,sizeof(GOP_TRANSCOLOR)) != GOP_API_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN(GOP_API_FAIL);
    }
#else
    GOP_SET_PROPERTY_PARAM ioctl_info;
    memset(&ioctl_info, 0x0, sizeof(GOP_SET_PROPERTY_PARAM));

    ioctl_info.en_pro = E_GOP_TRANSCOLOR;
    ioctl_info.gop_idx = u8GOP;
    ioctl_info.pSetting = (void*)&trans_set;
    ioctl_info.u32Size = sizeof(GOP_TRANSCOLOR);


    if (UtopiaIoctl(pInstantGOP,MAPI_CMD_GOP_SET_PROPERTY,(void*)&ioctl_info) != UTOPIA_STATUS_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN(GOP_API_FAIL);
    }
#endif
    GOP_RETURN(GOP_API_SUCCESS);

}
E_GOP_API_Result MApi_GOP_GWIN_EnableTransClr(EN_GOP_TRANSCLR_FMT fmt, MS_BOOL bEnable)
{
    MS_U8 u8GOP;
    E_GOP_API_Result ret;

    u8GOP = MApi_GOP_GWIN_GetCurrentGOP();
    ret = MApi_GOP_GWIN_EnableTransClr_EX(u8GOP,fmt,bEnable);
    return ret;
}

E_GOP_API_Result MApi_GOP_GWIN_Enable_BGWIN(MS_U8 bgwID, MS_BOOL bEnable)
{
    GOP_ENTRY();
    GOP_ERR("[%s] Not Support \n",__FUNCTION__);
    GOP_RETURN(GOP_API_SUCCESS);
}


E_GOP_API_Result MApi_GOP_GWIN_Set_BGWIN(MS_U8 bgwID, MS_U16 x0, MS_U16 y0, MS_U16 w, MS_U16 h, MS_U16 color)
{
    GOP_ENTRY();
    GOP_ERR("[%s] Not Support \n",__FUNCTION__);
    GOP_RETURN(GOP_API_SUCCESS);
}

E_GOP_API_Result MApi_GOP_GWIN_Set_BGWIN_Alpha(MS_U8 bgwID, MS_BOOL bEnable, MS_U8 alpha)
{
    GOP_ENTRY();
    GOP_ERR("[%s] Not Support \n",__FUNCTION__);
    GOP_RETURN(GOP_API_SUCCESS);
}
#endif
// Alignment stretch window value

/********************************************************************************/
/// Set stretch window property
/// @param u8GOP_num \b IN 0: GOP0  1:GOP1
/// @param eDstType \b IN \copydoc EN_GOP_DST_TYPE
/// @param x \b IN stretch window horizontal start position
/// @param y \b IN stretch window vertical start position
/// @param width \b IN stretch window width
/// @param height \b IN stretch window height
/// @return GOP_API_SUCCESS - Success
/// @return GOP_API_FAIL - Failure
/********************************************************************************/
E_GOP_API_Result MApi_GOP_GWIN_Set_STRETCHWIN(MS_U8 u8GOP_num, EN_GOP_DST_TYPE eDstType,MS_U16 x, MS_U16 y, MS_U16 width, MS_U16 height)
{
    GOP_STRETCH_INFO stretch_info;
    GOP_ENTRY();

    memset(&stretch_info, 0x0, sizeof(GOP_STRETCH_INFO));

    stretch_info.DstRect.x = x;
    stretch_info.DstRect.y = y;
    stretch_info.DstRect.w = width;
    stretch_info.DstRect.h = height;

    stretch_info.SrcRect.x = x;
    stretch_info.SrcRect.y = y;
    stretch_info.SrcRect.w = width;
    stretch_info.SrcRect.h = height;
#ifdef CONFIG_GOP_UTOPIA10
    if(Ioctl_GOP_Set_Stretch(NULL,E_GOP_STRETCH_WIN,u8GOP_num,&stretch_info) != GOP_API_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN(GOP_API_FAIL);
    }
#else

    GOP_STRETCH_SET_PARAM ioctl_info;
    memset(&ioctl_info,0x0, sizeof(GOP_STRETCH_SET_PARAM));

    ioctl_info.gop_idx = u8GOP_num;
    ioctl_info.enStrtchType = E_GOP_STRETCH_WIN;
    ioctl_info.pStretch = (MS_U32*)&stretch_info;
    ioctl_info.u32Size = sizeof(GOP_STRETCH_INFO);

    if (UtopiaIoctl(pInstantGOP,MAPI_CMD_GOP_GWIN_SET_STRETCH,(void*)&ioctl_info) != UTOPIA_STATUS_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN(GOP_API_FAIL);
    }
#endif
    GOP_RETURN(GOP_API_SUCCESS);

}
#ifndef CONFIG_GOP_REDUCE_CODESIZE_FOR_MBOOT
/********************************************************************************/
/// Restore initial stretch window setting of GOP. It will set gop to OP path and stretch window width/height
/// to panel width/height
/// @param u8GOPNum \b IN : Number of GOP
/// @return GOP_API_SUCCESS - Success
/********************************************************************************/
E_GOP_API_Result MApi_GOP_Initialize_StretchWindow(MS_U8 u8GOPNum)
{
#ifndef CONFIG_GOP_GWIN_MISC
    GOP_ERR("\33[0;36m   %s: #define CONFIG_GOP_GWIN_MISC not opened\33[m \n",__FUNCTION__);
    return GOP_API_FUN_NOT_SUPPORTED;
#else
    ST_GOP_TIMING_INFO stTimingInfo;
    MS_PHY *plist;

    GOP_ENTRY();
    plist = (MS_PHY *)(void*)&stTimingInfo;

    MApi_GOP_GetConfigEx(u8GOPNum,E_GOP_TIMING_INFO,plist);

#if (FPGA_TEST)
    MApi_GOP_GWIN_Set_STRETCHWIN(u8GOPNum, E_GOP_DST_VOP,0, 0
        , stTimingInfo.u16DEHSize, stTimingInfo.u16DEVSize);
#else
    MApi_GOP_GWIN_Set_STRETCHWIN(u8GOPNum, E_GOP_DST_OP0,0, 0
        , stTimingInfo.u16DEHSize, stTimingInfo.u16DEVSize);
#endif
    GOP_RETURN(GOP_API_SUCCESS);
#endif
}

E_GOP_API_Result MApi_GOP_GWIN_SetPixelShift(MS_S32 s32Hoffset, MS_S32 s32Voffset)
{
    GOP_STRETCH_OFFSET_ST stStretchOffset;
    MS_U8 u8Win = 0x0;
    GOP_ENTRY();

    stStretchOffset.s32Hoffset = s32Hoffset;
    stStretchOffset.s32Voffset = s32Voffset;
#ifdef CONFIG_GOP_UTOPIA10
    if(Ioctl_GOP_GWin_SetProperty(NULL,E_GOP_GWIN_GPUTILE,u8Win,(MS_U32*)&stStretchOffset,sizeof(GOP_STRETCH_OFFSET_ST)) != GOP_API_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN(GOP_API_FAIL);
    }
#else
    GOP_GWIN_PROPERTY_PARAM ioctl_info;
    memset(&ioctl_info, 0x0, sizeof(GOP_GWIN_PROPERTY_PARAM));

    ioctl_info.en_property = E_GOP_GWIN_STRETCH_OFFSET;
    ioctl_info.GwinId = u8Win;
    ioctl_info.pSet = (MS_U32*) &stStretchOffset;
    ioctl_info.u32Size = sizeof(GOP_STRETCH_OFFSET_ST);

    if(UtopiaIoctl(pInstantGOP,MAPI_CMD_GOP_GWIN_SET_PROPERTY,&ioctl_info) != UTOPIA_STATUS_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN(GOP_API_FAIL);
    }
#endif
    GOP_RETURN(GOP_API_SUCCESS);
}

#endif
/********************************************************************************/
/// Set GOP H-Mirror
/// @param bEnable \b IN
///   - # TRUE enable
///   - # FALSE disable
/// @return GOP_API_SUCCESS - Success
/********************************************************************************/
E_GOP_API_Result MApi_GOP_GWIN_SetHMirror_EX(MS_U8 u8GOP,MS_BOOL bEnable)
{
    GOP_SETMIRROR_PARAM ioctl_info;
    GOP_ENTRY();
    memset(&ioctl_info, 0x0, sizeof(GOP_SETMIRROR_PARAM));


    if(bEnable == TRUE)
        ioctl_info.dir = E_GOP_MIRROR_H_ONLY;
    else
        ioctl_info.dir = E_GOP_MIRROR_H_NONE;
#ifdef CONFIG_GOP_UTOPIA10
    if(Ioctl_GOP_SetMirror(NULL,u8GOP,ioctl_info.dir) != GOP_API_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN(GOP_API_FAIL);
    }
#else
    ioctl_info.gop_idx = (MS_U32)u8GOP;  //get current gop for driver capability
    if (UtopiaIoctl(pInstantGOP,MAPI_CMD_GOP_SET_MIRROR,(void*)&ioctl_info) != UTOPIA_STATUS_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN(GOP_API_FAIL);
    }
#endif

    GOP_RETURN(GOP_API_SUCCESS);
}
E_GOP_API_Result MApi_GOP_GWIN_SetHMirror(MS_BOOL bEnable)
{
    MS_U8 u8GOP;
    E_GOP_API_Result ret;

    u8GOP = MApi_GOP_GWIN_GetCurrentGOP();
    ret = MApi_GOP_GWIN_SetHMirror_EX(u8GOP,bEnable);
    return ret;
}
#ifndef CONFIG_GOP_REDUCE_CODESIZE_FOR_MBOOT

//******************************************************************************
/// Get gop h-mirror and v-mirror status
/// @param *bHMirror \b OUT TRUE:h-Mirror on, FALSE: h-Mirror off
/// @param *bVMirror \b OUT TRUE:v-Mirror on, FALSE: v-Mirror off
/// @return GOP_API_SUCCESS - Success
//******************************************************************************
E_GOP_API_Result MApi_GOP_GWIN_IsMirrorOn_EX(MS_U8 u8GOP,MS_BOOL *bHMirror, MS_BOOL *bVMirror)
{
    EN_GOP_MIRROR_TYPE mirror_type;
    GOP_ENTRY();
#ifdef CONFIG_GOP_UTOPIA10
    if(Ioctl_GOP_GetMirror(NULL,u8GOP,&mirror_type) != GOP_API_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN(GOP_API_FAIL);
    }
#else
    GOP_GETMIRROR_PARAM ioctl_info;

    memset(&ioctl_info, 0x0, sizeof(GOP_GETMIRROR_PARAM));

    ioctl_info.gop_idx = (MS_U32)u8GOP;  //get current gop for driver capability
    ioctl_info.pdir    = &mirror_type;

    if (UtopiaIoctl(pInstantGOP,MAPI_CMD_GOP_GET_MIRROR,(void*)&ioctl_info) != UTOPIA_STATUS_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN(GOP_API_FAIL);
    }
#endif
    if(mirror_type == E_GOP_MIRROR_HV)
    {
        *bHMirror = TRUE;
        *bVMirror = TRUE;
    }
    else if(mirror_type == E_GOP_MIRROR_H_ONLY)
    {
        *bHMirror = TRUE;
        *bVMirror = FALSE;
    }
    else if(mirror_type == E_GOP_MIRROR_V_ONLY)
    {
        *bHMirror = FALSE;
        *bVMirror = TRUE;
    }
    else
    {
        *bHMirror = FALSE;
        *bVMirror = FALSE;
    }

    GOP_RETURN(GOP_API_SUCCESS);

}
E_GOP_API_Result MApi_GOP_GWIN_IsMirrorOn(MS_BOOL *bHMirror, MS_BOOL *bVMirror)
{
    MS_U8 u8GOP;
    E_GOP_API_Result ret;

    u8GOP = MApi_GOP_GWIN_GetCurrentGOP();
    ret = MApi_GOP_GWIN_IsMirrorOn_EX(u8GOP,bHMirror,bVMirror);
    return ret;
}
#endif

/********************************************************************************/
/// Set GOP V-Mirror
/// @param bEnable \b IN
///   - # TRUE enable
///   - # FALSE disable
/// @return GOP_API_SUCCESS - Success
/********************************************************************************/
E_GOP_API_Result MApi_GOP_GWIN_SetVMirror_EX(MS_U8 u8GOP,MS_BOOL bEnable)
{
    GOP_SETMIRROR_PARAM ioctl_info;
    GOP_ENTRY();
    memset(&ioctl_info, 0x0, sizeof(GOP_SETMIRROR_PARAM));

    if( bEnable == TRUE )
        ioctl_info.dir = E_GOP_MIRROR_V_ONLY;
    else
        ioctl_info.dir = E_GOP_MIRROR_V_NONE;
#ifdef CONFIG_GOP_UTOPIA10
    if(Ioctl_GOP_SetMirror(NULL,u8GOP,ioctl_info.dir) != GOP_API_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN(GOP_API_FAIL);
    }
#else
    ioctl_info.gop_idx = (MS_U32)u8GOP;  //get current gop for driver capability
    if (UtopiaIoctl(pInstantGOP,MAPI_CMD_GOP_SET_MIRROR,(void*)&ioctl_info) != UTOPIA_STATUS_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN(GOP_API_FAIL);
    }
#endif
    GOP_RETURN(GOP_API_SUCCESS);

}

E_GOP_API_Result MApi_GOP_GWIN_SetVMirror(MS_BOOL bEnable)
{
    MS_U8 u8GOP;
    E_GOP_API_Result ret;
    u8GOP = MApi_GOP_GWIN_GetCurrentGOP();
    ret = MApi_GOP_GWIN_SetVMirror_EX(u8GOP,bEnable);
    return ret;
}
#ifndef CONFIG_GOP_REDUCE_CODESIZE_FOR_MBOOT

/********************************************************************************/
/// Set GWIN Tile Mode
/// @param u8win \b IN GWIN id
/// @param bEnable \b IN
///   - # TRUE Gwin Tile Mode
///   - # FALSE Gwin Tile Mode
/// @param u8coef \b IN alpha blending coefficient (0-7)
/// @return GOP_API_SUCCESS - Success
/********************************************************************************/
E_GOP_API_Result MApi_GOP_GWIN_SetGPUTile(MS_U8 u8win, EN_GOP_GPU_TILE_MODE TileMode)
{
    EN_GOP_GPU_TILE_MODE mode;
    mode = TileMode;
    GOP_ENTRY();

#ifdef CONFIG_GOP_UTOPIA10
    if(Ioctl_GOP_GWin_SetProperty(NULL,E_GOP_GWIN_GPUTILE,u8win,(MS_U32*)&mode,sizeof(EN_GOP_GPU_TILE_MODE)) != GOP_API_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN(GOP_API_FAIL);
    }
#else
    GOP_GWIN_PROPERTY_PARAM ioctl_info;
    memset(&ioctl_info, 0x0, sizeof(GOP_GWIN_PROPERTY_PARAM));

    ioctl_info.en_property = E_GOP_GWIN_GPUTILE;
    ioctl_info.GwinId = u8win;
    ioctl_info.pSet = (MS_U32*)&mode;
    ioctl_info.u32Size = sizeof(EN_GOP_GPU_TILE_MODE);

    if (UtopiaIoctl(pInstantGOP,MAPI_CMD_GOP_GWIN_SET_PROPERTY,(void*)&ioctl_info) != UTOPIA_STATUS_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN(GOP_API_FAIL);
    }
#endif
    GOP_RETURN(GOP_API_SUCCESS);

}

//-------------------------------------------------------------------------------------------------
/// GOP Exit
/// @param  void                \b IN: none
//-------------------------------------------------------------------------------------------------
void MApi_GOP_Exit()
{
    //App call exit, should wait mutex return to avoid mutex conflict use with other thread.
    //But do not need GOP_RETURN, because all mutex and resouce has been released in atexit.
    #if defined(MSOS_TYPE_LINUX)
    GOP_INFO("---%s %d: PID[%td], TID[%td] tries to exit\n", __FUNCTION__, __LINE__, (ptrdiff_t)getpid(), (ptrdiff_t)MsOS_GetOSThreadID());
    #elif defined(MSOS_TYPE_LINUX_KERNEL)
    GOP_INFO("---%s %d: PID[%td], TID[%td] tries to exit\n", __FUNCTION__, __LINE__, (ptrdiff_t)pInstantGOP, (ptrdiff_t)MsOS_GetOSThreadID());
    #else
    GOP_INFO("---%s %d: PID[%td], TID[%td] tries to exit\n", __FUNCTION__, __LINE__, (ptrdiff_t)0, (ptrdiff_t)MsOS_GetOSThreadID());
    #endif
    GOP_ENTRY();
#ifdef  MSOS_TYPE_LINUX
    MS_U32  value = 0;
#ifdef CONFIG_GOP_UTOPIA10
    if(Ioctl_GOP_MISC(NULL,E_GOP_MISC_AT_EXIT,(MS_U32*)&value,sizeof(int)) != GOP_API_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        return;
    }
#else
    GOP_MISC_PARAM ioctl_info;
    memset(&ioctl_info, 0x0, sizeof(GOP_MISC_PARAM));

    ioctl_info.misc_type = E_GOP_MISC_AT_EXIT;
    ioctl_info.pMISC = (MS_U32*)&value;
    ioctl_info.u32Size  = sizeof(MS_U32);

    if(UtopiaIoctl(pInstantGOP,MAPI_CMD_GOP_MISC,&ioctl_info)!= UTOPIA_STATUS_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        return;
    }
#endif
#else
    GOP_INFO("not enable MSOS_TYPE_LINUX\n");
#endif
}
#endif
//-------------------------------------------------------------------------------------------------
/// Initial individual GOP driver
/// @param pGopInit \b IN:gop driver init info
/// @param u8GOP \b IN: only init by which gop
/// @return GOP_API_SUCCESS - Success
/// @return GOP_API_FAIL - Failure
//-------------------------------------------------------------------------------------------------
E_GOP_API_Result MApi_GOP_InitByGOP(GOP_InitInfo *pGopInit, MS_U8 u8GOP)
{
    _GOP_RegisterAllCBFunc();

#ifdef CONFIG_GOP_UTOPIA10
    if(Ioctl_GOP_Init(NULL,u8GOP,pGopInit) != GOP_API_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN(GOP_API_FAIL);
    }
#else
	CheckGOPInstanceOpen();
    GOP_INIT_PARAM  ioctl_info;
    memset(&ioctl_info, 0x0, sizeof(GOP_INIT_PARAM));

    ioctl_info.gop_idx = u8GOP;
    ioctl_info.pInfo = (MS_U32*) pGopInit;
    ioctl_info.u32Size = sizeof(GOP_InitInfo);

    if (UtopiaIoctl(pInstantGOP,MAPI_CMD_GOP_INIT,(void*)&ioctl_info) != UTOPIA_STATUS_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        return GOP_API_FAIL;
    }
#endif
#ifdef  MSOS_TYPE_LINUX
    atexit(MApi_GOP_Exit);
#endif
    bInit =TRUE;
    return GOP_API_SUCCESS;
}
#ifndef CONFIG_GOP_REDUCE_CODESIZE_FOR_MBOOT


//-------------------------------------------------------------------------------------------------
/// Initial all GOP driver (include gop0, gop1 ext..)
/// @param pGopInit \b IN:gop driver init info
/// @return GOP_API_SUCCESS - Success
/// @return GOP_API_FAIL - Failure
//-------------------------------------------------------------------------------------------------
E_GOP_API_Result MApi_GOP_Init(GOP_InitInfo *pGopInit)
{
    MS_U32 i =0;

    CheckGOPInstanceOpen();
    //init GOP0
    _GOP_RegisterAllCBFunc();
#ifdef CONFIG_GOP_UTOPIA10
    if(Ioctl_GOP_Init(NULL,0,pGopInit) != GOP_API_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN(GOP_API_FAIL);
    }
#else
    GOP_INIT_PARAM  ioctl_info;
    memset(&ioctl_info, 0x0, sizeof(GOP_INIT_PARAM));

    ioctl_info.gop_idx = 0;
    ioctl_info.pInfo = (MS_U32*) pGopInit;
    ioctl_info.u32Size = sizeof(GOP_InitInfo);

    if (UtopiaIoctl(pInstantGOP,MAPI_CMD_GOP_INIT,(void*)&ioctl_info) != UTOPIA_STATUS_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        return GOP_API_FAIL;
    }
#endif
    //init others
    for(i = 1;i<MApi_GOP_GWIN_GetMaxGOPNum();i++)
    {
#ifdef CONFIG_GOP_UTOPIA10
        if(Ioctl_GOP_Init(NULL,i,pGopInit) != GOP_API_SUCCESS)
        {
            GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
            GOP_RETURN(GOP_API_FAIL);
        }
#else
        ioctl_info.gop_idx = i;
        if (UtopiaIoctl(pInstantGOP,MAPI_CMD_GOP_INIT,(void*)&ioctl_info) != UTOPIA_STATUS_SUCCESS)
        {
            GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
            return GOP_API_FAIL;
        }
#endif
    }
    bInit =TRUE;
    return GOP_API_SUCCESS;
}

/******************************************************************************/
/// Set a GWIN's H display size
/// @param gId  \b IN: gwin id
/// @param Width \b IN: gwin display width
/// @param X_Start \b IN: gwin display H display starting position
/// @return GOP_API_SUCCESS - Success
/// @return GOP_API_FAIL - Failure
/******************************************************************************/
E_GOP_API_Result MApi_GOP_GWIN_SetHDisplaySize(MS_U8 gId, MS_U16 Width,MS_U16 X_Start)
{
    GOP_BUFFER_INFO  BuffInfo;

    GOP_ENTRY();

    memset(&BuffInfo, 0, sizeof(GOP_BUFFER_INFO));

#ifdef CONFIG_GOP_UTOPIA10
    if(Ioctl_GOP_GetWinInfo(NULL,gId,&BuffInfo) != GOP_API_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN(GOP_API_FAIL);
    }
#else
    GOP_GWIN_SETWININFO_PARAM WinInfo;
    memset(&WinInfo, 0, sizeof(GOP_GWIN_SETWININFO_PARAM));

    WinInfo.GwinId = gId;
    WinInfo.pinfo = (MS_U32*)&BuffInfo;
    WinInfo.u32Size = sizeof(GOP_BUFFER_INFO);
    if (UtopiaIoctl(pInstantGOP,MAPI_CMD_GOP_GWIN_GET_WININFO,(void*)&WinInfo) != UTOPIA_STATUS_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN(GOP_API_FAIL);
    }
#endif

    if (X_Start % 4 != 0 )
    {
       X_Start = (X_Start +2) / 4; // rounding
       X_Start = X_Start * 4;
    }

    BuffInfo.disp_rect.x = X_Start;
    BuffInfo.disp_rect.w = Width;
    BuffInfo.width = Width;

#ifdef CONFIG_GOP_UTOPIA10
    if(Ioctl_GOP_SetWinInfo(NULL,gId,&BuffInfo) != GOP_API_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN(GOP_API_FAIL);
    }
#else
    if (UtopiaIoctl(pInstantGOP,MAPI_CMD_GOP_GWIN_SET_WININFO,(void*)&WinInfo) != UTOPIA_STATUS_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN(GOP_API_FAIL);
    }
#endif
    GOP_RETURN(GOP_API_SUCCESS);

}

/******************************************************************************/
/// Move a GWIN's display left top position
/// @param gId  \b IN : GWINID
/// @param dispX \b IN: left top x
/// @param dispY \b IN: left top y
/// @return GOP_API_SUCCESS - Success
/// @return GOP_API_FAIL - Failure
/******************************************************************************/
E_GOP_API_Result MApi_GOP_GWIN_SetWinDispPosition(MS_U8 gId, MS_U16 dispX, MS_U16 dispY)
{
    GOP_BUFFER_INFO  BuffInfo;

    GOP_ENTRY();

    memset(&BuffInfo, 0, sizeof(GOP_BUFFER_INFO));

#ifdef CONFIG_GOP_UTOPIA10
    if(Ioctl_GOP_GetWinInfo(NULL,gId,&BuffInfo) != GOP_API_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN(GOP_API_FAIL);
    }
#else
    GOP_GWIN_SETWININFO_PARAM WinInfo;
    memset(&WinInfo, 0, sizeof(GOP_GWIN_SETWININFO_PARAM));

    WinInfo.GwinId = gId;
    WinInfo.pinfo = (MS_U32*)&BuffInfo;
    WinInfo.u32Size = sizeof(GOP_BUFFER_INFO);

    if (UtopiaIoctl(pInstantGOP,MAPI_CMD_GOP_GWIN_GET_WININFO,(void*)&WinInfo) != UTOPIA_STATUS_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN(GOP_API_FAIL);
    }
#endif
    BuffInfo.disp_rect.x = dispX;
    BuffInfo.disp_rect.y = dispY;
#ifdef CONFIG_GOP_UTOPIA10
    if(Ioctl_GOP_GetWinInfo(NULL,gId,&BuffInfo) != GOP_API_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN(GOP_API_FAIL);
    }
#else
    if (UtopiaIoctl(pInstantGOP,MAPI_CMD_GOP_GWIN_SET_WININFO,(void*)&WinInfo) != UTOPIA_STATUS_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN(GOP_API_FAIL);
    }
#endif
    GOP_RETURN(GOP_API_SUCCESS);

}
#endif

/******************************************************************************/
/// Delete the GWIN, free corresponding frame buffer
/// @param gId \b IN \copydoc GWINID
/// @return GOP_API_SUCCESS - Success
/// @return GOP_API_FAIL - Failure
/******************************************************************************/
E_GOP_API_Result MApi_GOP_GWIN_DestroyWin(MS_U8 gId)
{
    GOP_ENTRY();
#ifdef CONFIG_GOP_UTOPIA10
    if(Ioctl_GOP_Win_Destroy(NULL,gId) != GOP_API_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN(GOP_API_FAIL);
    }
#else

    GOP_GWIN_DESTROY_PARAM ioctl_info;
    memset(&ioctl_info, 0x0, sizeof(GOP_GWIN_DESTROY_PARAM));
    ioctl_info.GwinId = gId;

    if (UtopiaIoctl(pInstantGOP,MAPI_CMD_GOP_GWIN_DESTROY,(void*)&ioctl_info) != UTOPIA_STATUS_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN(GOP_API_FAIL);
    }
#endif
    GOP_RETURN(GOP_API_SUCCESS);

}
#ifndef CONFIG_GOP_REDUCE_CODESIZE_FOR_MBOOT

/******************************************************************************/
/// Get corresponding frame buffer attribute from a GWIN id
/// @param gId \b IN GWINID
/// @param x0 \b OUT buffer to store left top x
/// @param y0 \b OUT buffer to store left top y
/// @param width \b OUT buffer to store width
/// @param height \b OUT buffer to store height
/// @param fbPitch \b OUT buffer to store pitch
/// @param fbFmt \b OUT buffer to store FB format
/// @param addr \b OUT buffer to store addr
/// @param size \b OUT buffer to store size
/// @return GOP_API_SUCCESS - Success
/// @return GOP_API_FAIL - Failure
/******************************************************************************/
E_GOP_API_Result MApi_GOP_GWIN_GetWinAttr(MS_U8 gId, MS_U16 *x0, MS_U16 *y0, MS_U16 *width, MS_U16 *height,  MS_U16 *fbPitch, MS_U16 *fbFmt, MS_PHY *addr, MS_U32 *size)
{
    GOP_BUFFER_INFO  BuffInfo;

    GOP_ENTRY();

    memset(&BuffInfo, 0, sizeof(GOP_BUFFER_INFO));

#ifdef CONFIG_GOP_UTOPIA10
    if(Ioctl_GOP_GetWinInfo(NULL,gId,&BuffInfo) != GOP_API_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN(GOP_API_FAIL);
    }
#else
    GOP_GWIN_SETWININFO_PARAM WinInfo;
    memset(&WinInfo, 0, sizeof(GOP_GWIN_SETWININFO_PARAM));

    WinInfo.GwinId = gId;
    WinInfo.pinfo = (MS_U32*)&BuffInfo;
    WinInfo.u32Size = sizeof(GOP_BUFFER_INFO);

    if (UtopiaIoctl(pInstantGOP,MAPI_CMD_GOP_GWIN_GET_WININFO,(void*)&WinInfo) != UTOPIA_STATUS_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN(GOP_API_FAIL);
    }
#endif
    *x0 = BuffInfo.disp_rect.x;
    *y0 = BuffInfo.disp_rect.y;
    *width = BuffInfo.width;
    *height = BuffInfo.height;
    *fbPitch = BuffInfo.pitch;
    *fbFmt = BuffInfo.fbFmt;
    *addr = BuffInfo.addr;
    *size = BuffInfo.width * BuffInfo.height * GOP_GetBPP((EN_GOP_COLOR_TYPE)BuffInfo.fbFmt)/8;

    GOP_RETURN(GOP_API_SUCCESS);
}

/********************************************************************************/
/// Get max V-End from all GWIN
/// @param bActive \b IN
///   - # TRUE enable
///   - # FALSE disable
/// @return max V-End value from all GWIN
/********************************************************************************/
MS_U16 MApi_GOP_GWIN_GetMaxVEnd(MS_BOOL bActive)
{

    GOP_ENTRY();
    GOP_ERR("[%s] Not SUPPORT\n",__FUNCTION__);
    GOP_RETURN(0);
}


/******************************************************************************/
/// Get maximum frame buffer number
/// @return maximum frame buffer number
/******************************************************************************/
MS_U8 MApi_GOP_GWIN_GetMaxFBNum(void)
{
    MS_U32 u32MaxFBNum;
    u32MaxFBNum = MApi_GOP_GWIN_GetMax32FBNum ();
    if( u32MaxFBNum > 256) //Out of MS_U8
    {
        GOP_WARN( "[Warning] %s %d( u32MaxFBNum:%td....)out of MS_U8 bound, please to use MApi_GOP_GWIN_GetMax32FBNum()\n",__FUNCTION__,__LINE__,(ptrdiff_t)u32MaxFBNum );
    }
    return u32MaxFBNum;
}

MS_U32 MApi_GOP_GWIN_GetMax32FBNum(void)
{
    MS_PHY u32FBIDNum=INVALID_POOL_NEXT_FBID;
    MApi_GOP_GetConfigEx(0, E_GOP_GET_MAXFBNUM, &u32FBIDNum);
    return u32FBIDNum;
}


/********************************************************************************/
/// Set second frame buffer pool
/// @param u32DblHeapAdr \b IN: second frame buffer pool starting address
/// @param u32DblHeapLen \b IN: second frame buffer pool length
/// @return GOP_API_SUCCESS - Success
/// @return GOP_API_FAIL - Failure
/********************************************************************************/
E_GOP_API_Result MApi_GOP_GWIN_SetDoubleHeap(MS_U32 u32DblHeapAdr, MS_U32 u32DblHeapLen)
{
    GOP_ERR("[%s] Not SUPPORT\n",__FUNCTION__);
    return GOP_API_SUCCESS;
}

/******************************************************************************/
/// Create frame buffer from given frame buffer id and dram address. User should call MApi_GOP_GWIN_GetFree32FBID()
/// first and then use this frame buffer id as this function input parameter
/// @param fbId  \b IN: frame buffer id
/// @param dispX \b IN: frame buffer starting x
/// @param dispY \b IN: frame buffer starting y
/// @param width \b IN: frame buffer width
/// @param height \b IN: frame buffer height
/// @param fbFmt \b IN: frame buffer color format
/// @param u32FbAddr \b IN: frame buffer starting address
/// @param FBString \b IN: create frame buffer by which one application
/// @return GOP_API_SUCCESS - Success
/// @return GOP_API_CRT_GWIN_NOAVAIL - Failure
/******************************************************************************/
MS_U8 MApi_GOP_GWIN_CreateFBbyStaticAddr2(MS_U8 fbId, MS_U16 dispX, MS_U16 dispY, MS_U16 width, MS_U16 height,  MS_U16 fbFmt, MS_PHY phyFbAddr ,EN_GOP_FRAMEBUFFER_STRING FBString)
{
    return MApi_GOP_GWIN_Create32FBbyStaticAddr2((MS_U32)fbId, dispX, dispY, width, height, fbFmt, phyFbAddr, FBString);
}
#endif
MS_U8 MApi_GOP_GWIN_Create32FBbyStaticAddr2(MS_U32 u32fbId, MS_U16 dispX, MS_U16 dispY, MS_U16 width, MS_U16 height,  MS_U16 fbFmt, MS_PHY phyFbAddr ,EN_GOP_FRAMEBUFFER_STRING FBString)
{
    GOP_BUFFER_INFO BuffInfo;
    GOP_ENTRY();
    memset(&BuffInfo, 0x0, sizeof(GOP_BUFFER_INFO));



    BuffInfo.addr = phyFbAddr;
    BuffInfo.fbFmt = fbFmt;
    BuffInfo.disp_rect.x = dispX;
    BuffInfo.disp_rect.y = dispY;
    BuffInfo.disp_rect.w = width;
    BuffInfo.disp_rect.h = height;

    BuffInfo.width = width;
    BuffInfo.height = height;
    BuffInfo.FBString = FBString;
#ifdef CONFIG_GOP_UTOPIA10
    if(Ioctl_GOP_FBCreate(NULL,GOP_CREATE_BUFFER_BYADDR,&BuffInfo,u32fbId) != GOP_API_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN(GOP_API_FAIL);
    }
#else
    GOP_CREATE_BUFFER_PARAM ioctl_info;
    memset(&ioctl_info, 0x0, sizeof(GOP_CREATE_BUFFER_PARAM));

    ioctl_info.fb_type  = GOP_CREATE_BUFFER_BYADDR;
    ioctl_info.pBufInfo = (MS_U32*)&BuffInfo;
    ioctl_info.u32Size = sizeof(GOP_BUFFER_INFO);
    ioctl_info.fbid = u32fbId;

    if(UtopiaIoctl(pInstantGOP,MAPI_CMD_GOP_FB_CREATE,&ioctl_info)!= UTOPIA_STATUS_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN(GOP_API_FAIL);
    }
#endif

    GOP_RETURN(GOP_API_SUCCESS);

}
#ifndef CONFIG_GOP_REDUCE_CODESIZE_FOR_MBOOT


/******************************************************************************/
/// Create frame buffer from given dram address
/// @param width  \b IN: frame buffer width
/// @param height \b IN: frame buffer height
/// @param fbFmt \b IN: frame buffer color format
/// @param u32FbAddr \b IN: frame buffer starting address
/// @param pitch \b IN: frame buffer pitch
/// @param ret_FBId \b OUT: frame buffer id
/// @return GOP_API_SUCCESS - Success
/// @return GOP_API_FAIL - Failure
/******************************************************************************/
MS_U8 MApi_GOP_GWIN_CreateFBFrom3rdSurf(MS_U16 width, MS_U16 height,  MS_U16 fbFmt, MS_PHY phyFbAddr, MS_U16 pitch, MS_U8* ret_FBId)
{
    MS_U32 u32ret_FBID = 0;
    MS_U8 u8result = 0;
    u8result = MApi_GOP_GWIN_Create32FBFrom3rdSurf(width, height, fbFmt, phyFbAddr, pitch, &u32ret_FBID);
    if(u32ret_FBID > 256)//Out of MS_U8
    {
        GOP_WARN( "[Warning] %s %d( ret_FBID:%td....)out of MS_U8 bound, please to use MApi_GOP_GWIN_Create32FBFrom3rdSurf()\n",__FUNCTION__,__LINE__,(ptrdiff_t)u32ret_FBID );
    }
    *ret_FBId = (MS_U8)u32ret_FBID;
    return u8result;
}

MS_U8 MApi_GOP_GWIN_Create32FBFrom3rdSurf(MS_U16 width, MS_U16 height,  MS_U16 fbFmt, MS_PHY phyFbAddr, MS_U16 pitch, MS_U32* u32ret_FBId)
{

    GOP_BUFFER_INFO BuffInfo;
    GOP_ENTRY();

    memset(&BuffInfo, 0x0, sizeof(GOP_BUFFER_INFO));

    *u32ret_FBId = MApi_GOP_GWIN_GetFree32FBID();

    BuffInfo.addr = phyFbAddr;
    BuffInfo.fbFmt = fbFmt;
    BuffInfo.disp_rect.x = 0;
    BuffInfo.disp_rect.y = 0;
    BuffInfo.disp_rect.w = width;
    BuffInfo.disp_rect.h = height;

    BuffInfo.width = width;
    BuffInfo.height = height;
    BuffInfo.pitch = pitch;
#ifdef CONFIG_GOP_UTOPIA10
    if(Ioctl_GOP_FBCreate(NULL,GOP_CREATE_BUFFER_BYADDR,&BuffInfo,*u32ret_FBId) != GOP_API_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN(GOP_API_FAIL);
    }
#else

    GOP_CREATE_BUFFER_PARAM ioctl_info;
    memset(&ioctl_info, 0x0, sizeof(GOP_CREATE_BUFFER_PARAM));

    ioctl_info.fb_type  = GOP_CREATE_BUFFER_BYADDR;
    ioctl_info.pBufInfo = (MS_U32*)&BuffInfo;
    ioctl_info.u32Size = sizeof(GOP_BUFFER_INFO);
    ioctl_info.fbid = *u32ret_FBId;
    if(UtopiaIoctl(pInstantGOP,MAPI_CMD_GOP_FB_CREATE,&ioctl_info)!= UTOPIA_STATUS_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN(GOP_API_FAIL);
    }
#endif

    GOP_RETURN(GOP_API_SUCCESS);

}

#endif
/******************************************************************************/
/// Create frame buffer from given frame buffer id and dram address. User should call MApi_GOP_GWIN_GetFree32FBID()
/// first and then use this frame buffer id as this function input parameter
/// @param fbId  \b IN: frame buffer id
/// @param dispX \b IN: frame buffer starting x
/// @param dispY \b IN: frame buffer starting y
/// @param width \b IN: frame buffer width
/// @param height \b IN: frame buffer height
/// @param fbFmt \b IN: frame buffer color format
/// @param u32FbAddr \b IN: frame buffer starting address
/// @return GOP_API_SUCCESS - Success
/// @return GOP_API_FAIL - Failure
/******************************************************************************/
MS_U8 MApi_GOP_GWIN_CreateFBbyStaticAddr(MS_U8 fbId, MS_U16 dispX, MS_U16 dispY, MS_U16 width, MS_U16 height,  MS_U16 fbFmt, MS_PHY phyFbAddr)
{
     return MApi_GOP_GWIN_Create32FBbyStaticAddr((MS_U32)fbId, dispX, dispY, width, height,  fbFmt, phyFbAddr);
}

MS_U8 MApi_GOP_GWIN_Create32FBbyStaticAddr(MS_U32 u32fbId, MS_U16 dispX, MS_U16 dispY, MS_U16 width, MS_U16 height,  MS_U16 fbFmt, MS_PHY phyFbAddr)
{
     return MApi_GOP_GWIN_Create32FBbyStaticAddr2(u32fbId, dispX, dispY, width, height,  fbFmt, phyFbAddr, E_GOP_FB_NULL);
}


/******************************************************************************/
/// Set GE to render on the frame buffer
/// @param fbId \b IN: frame buffer id
/// @return GOP_API_SUCCESS - Success
/// @return GOP_API_FAIL - Failure
/******************************************************************************/
E_GOP_API_Result MApi_GOP_GWIN_Switch2FB(MS_U8 fbId)
{
    return MApi_GOP_GWIN_Switch2_32FB((MS_U32)fbId);
}

E_GOP_API_Result MApi_GOP_GWIN_Switch2_32FB(MS_U32 u32fbId)
{
    GOP_CBFmtInfo* pCBFmtInfo=NULL;

    GOP_ENTRY();
    pCBFmtInfo = (PGOP_CBFmtInfo)malloc(sizeof(GOP_CBFmtInfo));
    if(pCBFmtInfo == NULL)
    {
        GOP_ERR("[%s][%d]pCBFmtInfo is NULL\n",__FUNCTION__, __LINE__);
        GOP_RETURN( GOP_API_FAIL);
    }
    memset(pCBFmtInfo, 0x0, sizeof(GOP_CBFmtInfo));
#ifdef CONFIG_GOP_UTOPIA10
    if(Ioctl_GOP_Select(NULL,EN_GOP_SEL_FB,u32fbId,(MS_U32*)pCBFmtInfo) != GOP_API_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        free(pCBFmtInfo);
        GOP_RETURN(GOP_API_FAIL);
    }
#else

    GOP_SELECTION_PROPERTY_PARAM ioctl_info;
    memset(&ioctl_info, 0x0, sizeof(GOP_SELECTION_PROPERTY_PARAM));
    ioctl_info.sel_type = EN_GOP_SEL_FB;
    ioctl_info.id = u32fbId;
    ioctl_info.pinfo = (MS_U32*)pCBFmtInfo;
    ioctl_info.u32Size = sizeof(GOP_CBFmtInfo);

    if(UtopiaIoctl(pInstantGOP,MAPI_CMD_GOP_SELECTION,&ioctl_info)!= UTOPIA_STATUS_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        free(pCBFmtInfo);
        GOP_RETURN( GOP_API_FAIL);
    }
#endif
    fpSetFBFmt(pCBFmtInfo->u16Pitch,pCBFmtInfo->u64Addr,pCBFmtInfo->u16Fmt);

    free(pCBFmtInfo);
    GOP_RETURN(GOP_API_SUCCESS);

}

/******************************************************************************/
/// Change a GWIN's frame buffer, this enables an off screen buffer to be shown
/// @param fbId \b IN frame buffer id
/// @param gwinId \b IN \copydoc GWINID
/// @return GOP_API_SUCCESS - Success
/// @return GOP_API_FAIL - Failure
/******************************************************************************/
E_GOP_API_Result MApi_GOP_GWIN_MapFB2Win(MS_U8 fbId, MS_U8 gwinId)
{
    return MApi_GOP_GWIN_Map32FB2Win((MS_U32)fbId, gwinId);
}

E_GOP_API_Result MApi_GOP_GWIN_Map32FB2Win(MS_U32 u32fbId, MS_U8 u8gwinId)
{
    GOP_ENTRY();

#ifdef CONFIG_GOP_UTOPIA10
    if(Ioctl_GOP_MapFB2Win(NULL,u32fbId,u8gwinId) != GOP_API_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN(GOP_API_FAIL);
    }
#else

    GOP_GWIN_MAPFBINFO_PARAM ioctl_info;
    memset(&ioctl_info, 0x0, sizeof(GOP_GWIN_MAPFBINFO_PARAM));

    ioctl_info.fbid = u32fbId;
    ioctl_info.GwinId = u8gwinId;

    if(UtopiaIoctl(pInstantGOP,MAPI_CMD_GOP_GWIN_MAPFB2WIN,&ioctl_info)!= UTOPIA_STATUS_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN(GOP_API_FAIL);
    }
#endif
    GOP_RETURN(GOP_API_SUCCESS);

}



/******************************************************************************/
/// Get frame buffer information
/// @param fbId \b IN frame buffer id
/// @param fbAttr \b OUT buffer to store \copydoc GOP_GwinFBAttr
/// @return GOP_API_SUCCESS - Success
/// @return GOP_API_FAIL - Failure
/******************************************************************************/
E_GOP_API_Result MApi_GOP_GWIN_GetFBInfo(MS_U8 fbId, GOP_GwinFBAttr *fbAttr)
{
    return MApi_GOP_GWIN_Get32FBInfo((MS_U32)fbId, fbAttr);
}

E_GOP_API_Result MApi_GOP_GWIN_Get32FBInfo(MS_U32 u32fbId, GOP_GwinFBAttr *fbAttr)
{
    GOP_BUFFER_INFO BuffInfo;

    GOP_ENTRY();

    memset(&BuffInfo, 0x0, sizeof(GOP_BUFFER_INFO));
#ifdef CONFIG_GOP_UTOPIA10
    if(Ioctl_GOP_GetFBInfo(NULL,u32fbId,&BuffInfo) != GOP_API_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN(GOP_API_FAIL);
    }
#else
    GOP_FB_INFO_PARAM ioctl_info;
    memset(&ioctl_info, 0x0, sizeof(GOP_FB_INFO_PARAM));

    ioctl_info.pBufInfo = (MS_U32*)&BuffInfo;
    ioctl_info.u32Size = sizeof(GOP_BUFFER_INFO);
    ioctl_info.fbid = u32fbId;

    if(UtopiaIoctl(pInstantGOP,MAPI_CMD_GOP_FB_GETINFO,&ioctl_info)!= UTOPIA_STATUS_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN( GOP_API_FAIL);
    }
#endif
    memset(fbAttr, 0x0, sizeof(GOP_GwinFBAttr));

    fbAttr->width   = BuffInfo.width;
    fbAttr->height  = BuffInfo.height;
    fbAttr->fbFmt   = BuffInfo.fbFmt;
    fbAttr->pitch   = BuffInfo.pitch;
    fbAttr->x0      = BuffInfo.disp_rect.x;
    fbAttr->y0      = BuffInfo.disp_rect.y;
    fbAttr->x1      = BuffInfo.disp_rect.x + BuffInfo.disp_rect.w;
    fbAttr->y1      = BuffInfo.disp_rect.y + BuffInfo.disp_rect.h;
    fbAttr->s_x     = BuffInfo.disp_rect.x;
    fbAttr->s_y     = BuffInfo.disp_rect.y;
    fbAttr->addr    = BuffInfo.addr;

    fbAttr->size    = GOP_CalcPitch(BuffInfo.fbFmt, BuffInfo.width) * BuffInfo.height;

    //printf("[%s] buff size:%lx addr:%lx (%d,%d)\n",__FUNCTION__,fbAttr->size,fbAttr->addr, fbAttr->width, fbAttr->height);

    fbAttr->dispWidth  = BuffInfo.disp_rect.w;
    fbAttr->dispHeight = BuffInfo.disp_rect.h;

    //fbAttr->allocated = pwinFB->in_use;
    //fbAttr->enable = pwinFB->enable;
    //fbAttr->gWinId = pwinFB->gWinId;
    //fbAttr->string = pwinFB->string;

    GOP_RETURN(GOP_API_SUCCESS);

}

#ifndef CONFIG_GOP_REDUCE_CODESIZE_FOR_MBOOT

/******************************************************************************/
/// Set frame buffer info
/// @param fbId  \b IN: frame buffer id
/// @param fbAttr \b IN: frame buffer attribute
/// @return GOP_API_SUCCESS - Success
/// @return GOP_API_FAIL - Failure
/******************************************************************************/
E_GOP_API_Result MApi_GOP_GWIN_SetFBInfo(MS_U8 fbId, GOP_GwinFBAttr *fbAttr)
{
    return MApi_GOP_GWIN_Set32FBInfo((MS_U32)fbId, fbAttr);
}

E_GOP_API_Result MApi_GOP_GWIN_Set32FBInfo(MS_U32 u32fbId, GOP_GwinFBAttr *fbAttr)
{
    GOP_BUFFER_INFO BuffInfo;

    GOP_ENTRY();

    memset(&BuffInfo, 0x0, sizeof(GOP_BUFFER_INFO));

    BuffInfo.addr = fbAttr->addr;
    BuffInfo.disp_rect.x = fbAttr->x0;
    BuffInfo.disp_rect.y = fbAttr->y0;
    BuffInfo.disp_rect.w = fbAttr->x1 - fbAttr->x0;
    BuffInfo.disp_rect.h = fbAttr->y1 - fbAttr->y0;
    BuffInfo.fbFmt       = fbAttr->fbFmt;
    BuffInfo.pitch       = fbAttr->pitch;

    BuffInfo.width  = fbAttr->width;
    BuffInfo.height = fbAttr->height;

#ifdef CONFIG_GOP_UTOPIA10
    if(Ioctl_GOP_SetFBInfo(NULL,u32fbId,&BuffInfo) != GOP_API_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN(GOP_API_FAIL);
    }
#else
    GOP_FB_INFO_PARAM ioctl_info;

    memset(&ioctl_info, 0x0, sizeof(GOP_FB_INFO_PARAM));

    ioctl_info.pBufInfo = (MS_U32*)&BuffInfo;
    ioctl_info.u32Size = sizeof(GOP_BUFFER_INFO);
    ioctl_info.fbid = u32fbId;

    if(UtopiaIoctl(pInstantGOP,MAPI_CMD_GOP_FB_SETINFO,&ioctl_info)!= UTOPIA_STATUS_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN( GOP_API_FAIL);
    }
#endif
    GOP_RETURN(GOP_API_SUCCESS);
}

/********************************************************************************/
/// Switch frame buffer pool. This function is used for when user use MApi_GOP_GWIN_SetDoubleHeap to set second
/// frame buffer pool. User can control frame buffer opened in first or second buffer pool pool
/// @param u8FB \b IN 0: first frame buffer pool , 1: second frame buffer pool
/// @return GOP_API_SUCCESS - Success
/********************************************************************************/
E_GOP_API_Result MApi_GOP_FB_SEL(MS_U8 u8FB)
{
	MS_U32 u32PoolId = (MS_U32)u8FB;
    GOP_ENTRY();
#ifdef CONFIG_GOP_UTOPIA10
    if(Ioctl_GOP_FB_SetProperty(NULL,E_GOP_FB_POOLID,0x0, &u32PoolId,sizeof(MS_U8)) != GOP_API_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN(GOP_API_FAIL);
    }
#else
    GOP_FB_PROPERTY_PARAM ioctl_info;
    memset(&ioctl_info, 0x0, sizeof(GOP_UPDATE_PARAM));

    ioctl_info.en_property = E_GOP_FB_POOLID;
    ioctl_info.FBId = 0x0;
    ioctl_info.pSet = (void*)&u32PoolId;
    ioctl_info.u32Size = sizeof(MS_U8);

    if (UtopiaIoctl(pInstantGOP,MAPI_CMD_GOP_FB_SET_PROPERTY,(void*)&ioctl_info) != UTOPIA_STATUS_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN(GOP_API_FAIL);
    }
#endif
    GOP_RETURN(GOP_API_SUCCESS);
}

/********************************************************************************/
/// Get GOP version number
///@param ppVersion \b OUT: GOP Version
/// @return GOP_API_SUCCESS - Success
/// @return GOP_API_FAIL - Failure
/********************************************************************************/
E_GOP_API_Result MApi_GOP_GetLibVer(const MSIF_Version **ppVersion)
{
    if (!ppVersion)
    {
        return GOP_API_INVALID_PARAMETERS;
    }
    *ppVersion = &_api_gop_version;
    return GOP_API_SUCCESS;
}

/********************************************************************************/
/// Set Debug level of GOP
/// @param level \b IN debug level
/// @return GOP_API_SUCCESS - Success
/// @return GOP_API_FAIL - Failure
/********************************************************************************/
E_GOP_API_Result MApi_GOP_SetDbgLevel(EN_GOP_DEBUG_LEVEL level)
{
    u32GOPDbgLevel_api= level;

    GOP_ENTRY();
#ifdef CONFIG_GOP_UTOPIA10
    if(Ioctl_GOP_MISC(NULL,E_GOP_MISC_SET_DBG_LEVEL,(MS_U32*)&level,sizeof(MS_U32)) != GOP_API_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN(GOP_API_FAIL);
    }
#else

    GOP_MISC_PARAM ioctl_info;
    memset(&ioctl_info, 0x0, sizeof(GOP_MISC_PARAM));

    ioctl_info.misc_type = E_GOP_MISC_SET_DBG_LEVEL;
    ioctl_info.pMISC = (MS_U32*)&level;
    ioctl_info.u32Size  = sizeof(MS_U32);

    if(UtopiaIoctl(pInstantGOP,MAPI_CMD_GOP_MISC,&ioctl_info)!= UTOPIA_STATUS_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN( GOP_API_FAIL);
    }
#endif
    GOP_RETURN(GOP_API_SUCCESS);
}


/********************************************************************************/
/// GOP call back function: Set destination buffer pitch,addr,fmt to GE.
/// This function should be registered before MApi_GOP_Init!!
/// @return GOP_API_SUCCESS - Success
/// @return GOP_API_FAIL - Failure
/********************************************************************************/
E_GOP_API_Result MApi_GOP_RegisterFBFmtCB(MS_U32 (*fpGOP_CB)(MS_U16 pitch,MS_PHY addr , MS_U16 fmt )  )
{
    fpSetFBFmt = fpGOP_CB;

#ifdef GOP_UTOPIA2K
#else
#ifdef CONFIG_GOP_UTOPIA10
    if(Ioctl_GOP_Register_CB(NULL,GOP_CB_GFX_FBFMT,fpSetFBFmt,0) != GOP_API_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN(GOP_API_FAIL);
    }
#else
    GOP_REGISTER_CB_PARAM ioctl_info;
    memset(&ioctl_info, 0x0, sizeof(GOP_REGISTER_CB_PARAM));

    if(pInstantGOP != NULL)
    {
        ioctl_info.cb_type = GOP_CB_GFX_FBFMT;
        ioctl_info.pCB = fpSetFBFmt;
        if(UtopiaIoctl(pInstantGOP,MAPI_CMD_GOP_REGISTER_CB,&ioctl_info)!= UTOPIA_STATUS_SUCCESS)
        {
            GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
            return GOP_API_FAIL;
        }
    }
#endif
#endif
    return GOP_API_SUCCESS;
}


#endif

/********************************************************************************/
/// GOP call back function: Get vedio status (is interlace mode?) from scaler.
/// This function should be registered before MApi_GOP_Init!!
/// @return GOP_API_SUCCESS - Success
/// @return GOP_API_FAIL - Failure
/********************************************************************************/
E_GOP_API_Result MApi_GOP_RegisterXCIsInterlaceCB(MS_BOOL (*fpGOP_CB)(void))
{
    fpXCIsInterlace = fpGOP_CB;

#ifdef GOP_UTOPIA2K
#else
#ifdef CONFIG_GOP_UTOPIA10
    if(Ioctl_GOP_Register_CB(NULL,GOP_CB_XC_INTERLACE,fpXCIsInterlace,0) != GOP_API_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN(GOP_API_FAIL);
    }
#else
    GOP_REGISTER_CB_PARAM ioctl_info;
    memset(&ioctl_info, 0x0, sizeof(GOP_REGISTER_CB_PARAM));
    if(pInstantGOP != NULL)
    {
        ioctl_info.cb_type = GOP_CB_XC_INTERLACE;
        ioctl_info.pCB = fpXCIsInterlace;
        if(UtopiaIoctl(pInstantGOP,MAPI_CMD_GOP_REGISTER_CB,&ioctl_info)!= UTOPIA_STATUS_SUCCESS)
        {
            GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
            return GOP_API_FAIL;
        }
    }
#endif
#endif
    return GOP_API_SUCCESS;
}

/********************************************************************************/
/// GOP call back function: Get capture window h-start from scaler.
/// This function should be registered before MApi_GOP_Init!!
/// @return GOP_API_SUCCESS - Success
/// @return GOP_API_FAIL - Failure
/********************************************************************************/
E_GOP_API_Result MApi_GOP_RegisterXCGetCapHStartCB(MS_U16 (*fpGOP_CB)(void))
{
    fpXCGetCapHStart = fpGOP_CB;

#ifdef GOP_UTOPIA2K
#else
#ifdef CONFIG_GOP_UTOPIA10
    if(Ioctl_GOP_Register_CB(NULL,GOP_CB_XC_PANEL_HSTART,fpXCGetCapHStart,0) != GOP_API_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN(GOP_API_FAIL);
    }
#else
    GOP_REGISTER_CB_PARAM ioctl_info;
    memset(&ioctl_info, 0x0, sizeof(GOP_REGISTER_CB_PARAM));

    if(pInstantGOP != NULL)
    {
        ioctl_info.cb_type = GOP_CB_XC_PANEL_HSTART;
        ioctl_info.pCB = fpXCGetCapHStart;
        if(UtopiaIoctl(pInstantGOP,MAPI_CMD_GOP_REGISTER_CB,&ioctl_info)!= UTOPIA_STATUS_SUCCESS)
        {
            GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
            return GOP_API_FAIL;
        }
    }
#endif
#endif
    return GOP_API_SUCCESS;
}
#ifndef CONFIG_GOP_REDUCE_CODESIZE_FOR_MBOOT


/********************************************************************************/
/// GOP call back function: Notify callback for the end of GOP events such as buffer rearrange.
/// This function should be registered before MApi_GOP_Init!!
/// @return GOP_API_SUCCESS - Success
/// @return GOP_API_FAIL - Failure
/********************************************************************************/
E_GOP_API_Result MApi_GOP_RegisterEventNotify(void (*fpGOP_CB)(MS_U32 u32EventID, void* reserved0))
{
    fpEventNotify = fpGOP_CB;

#ifdef GOP_UTOPIA2K
#else
#ifdef CONFIG_GOP_UTOPIA10
    if(Ioctl_GOP_Register_CB(NULL,GOP_CB_EVENT_NOTIFY,fpEventNotify,0) != GOP_API_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN(GOP_API_FAIL);
    }
#else

    GOP_REGISTER_CB_PARAM ioctl_info;
    memset(&ioctl_info, 0x0, sizeof(GOP_REGISTER_CB_PARAM));
    if(pInstantGOP != NULL)
    {
        ioctl_info.cb_type = GOP_CB_EVENT_NOTIFY;
        ioctl_info.pCB = fpEventNotify;
        if(UtopiaIoctl(pInstantGOP,MAPI_CMD_GOP_REGISTER_CB,&ioctl_info)!= UTOPIA_STATUS_SUCCESS)
        {
            GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
            return GOP_API_FAIL;
        }
    }
#endif
#endif
    return GOP_API_SUCCESS;
}

#endif
/********************************************************************************/
/// GOP call back function: Reduce scaler bandwidth when OSD on.
/// This function should be registered before MApi_GOP_Init!!
/// @return GOP_API_SUCCESS - Success
/// @return GOP_API_FAIL - Failure
/********************************************************************************/
E_GOP_API_Result MApi_GOP_RegisterXCReduceBWForOSDCB(void (*fpGOP_CB)(MS_U8 PqWin, MS_BOOL enable))
{
    fpXCReduceBWForOSD = fpGOP_CB;

#ifdef GOP_UTOPIA2K
#else
#ifdef CONFIG_GOP_UTOPIA10
    if(Ioctl_GOP_Register_CB(NULL,GOP_CB_XC_REDUCE_BW,fpXCReduceBWForOSD,0) != GOP_API_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN(GOP_API_FAIL);
    }
#else

    GOP_REGISTER_CB_PARAM ioctl_info;
    memset(&ioctl_info, 0x0, sizeof(GOP_REGISTER_CB_PARAM));

    if(pInstantGOP != NULL)
    {
        ioctl_info.cb_type = GOP_CB_XC_REDUCE_BW;
        ioctl_info.pCB = fpXCReduceBWForOSD;
        if(UtopiaIoctl(pInstantGOP,MAPI_CMD_GOP_REGISTER_CB,&ioctl_info)!= UTOPIA_STATUS_SUCCESS)
        {
            GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
            return GOP_API_FAIL;
        }
    }
#endif
#endif
    return GOP_API_SUCCESS;
}
#ifndef CONFIG_GOP_REDUCE_CODESIZE_FOR_MBOOT

/********************************************************************************/
/// GOP call back function: Register XC DIP call back
/// This function should be registered before MApi_GOP_Init!!
/// @return GOP_API_SUCCESS - Success
/// @return GOP_API_FAIL - Failure
/********************************************************************************/
E_GOP_API_Result MApi_GOP_RegisterXCSetDwinInfo(MS_BOOL (*fpGOP_CB)(MS_U16 cmd,MS_U32 *arg,MS_U16 size))
{
    fpXCSetDwinInfo = fpGOP_CB;

#ifdef GOP_UTOPIA2K
#else
#ifdef CONFIG_GOP_UTOPIA10
    if(Ioctl_GOP_Register_CB(NULL,GOP_CB_DWIN_INFO,fpXCSetDwinInfo,0) != GOP_API_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN(GOP_API_FAIL);
    }
#else

    GOP_REGISTER_CB_PARAM ioctl_info;
    memset(&ioctl_info, 0x0, sizeof(GOP_REGISTER_CB_PARAM));

    if(pInstantGOP != NULL)
    {
        ioctl_info.cb_type = GOP_CB_DWIN_INFO;
        ioctl_info.pCB = fpXCSetDwinInfo;
        if(UtopiaIoctl(pInstantGOP,MAPI_CMD_GOP_REGISTER_CB,&ioctl_info)!= UTOPIA_STATUS_SUCCESS)
        {
            GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
            return GOP_API_FAIL;
        }
    }
#endif
#endif
    return GOP_API_SUCCESS;
}

#endif
MS_U32 _GOP_RegisterAllCBFunc()
{
#ifdef GOP_UTOPIA2K
#else
    if(fpSetFBFmt == NULL)
    {
#ifdef MSOS_TYPE_NOS
        GOP_INFO("[%s][%d] fpSetFBFmt is NULL.(If on STR state, this is a normal message) \n",__FUNCTION__,__LINE__);
#endif
    }
#ifdef CONFIG_GOP_UTOPIA10
    if(Ioctl_GOP_Register_CB(NULL,GOP_CB_GFX_FBFMT,fpSetFBFmt,0) != GOP_API_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN(GOP_API_FAIL);
    }
#else
    GOP_REGISTER_CB_PARAM ioctl_info;
    memset(&ioctl_info, 0x0, sizeof(GOP_REGISTER_CB_PARAM));

    ioctl_info.cb_type = GOP_CB_GFX_FBFMT;
    ioctl_info.pCB = fpSetFBFmt;
    if(UtopiaIoctl(pInstantGOP,MAPI_CMD_GOP_REGISTER_CB,&ioctl_info)!= UTOPIA_STATUS_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        return GOP_API_FAIL;
    }
#endif
    if(fpXCIsInterlace == NULL)
    {
        GOP_INFO("[%s][%d] fpXCIsInterlace is NULL.(If on STR state, this is a normal message) \n",__FUNCTION__,__LINE__);
    }
#ifdef CONFIG_GOP_UTOPIA10
    if(Ioctl_GOP_Register_CB(NULL,GOP_CB_XC_INTERLACE,fpXCIsInterlace,0) != GOP_API_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN(GOP_API_FAIL);
    }
#else
    ioctl_info.cb_type = GOP_CB_XC_INTERLACE;
    ioctl_info.pCB = fpXCIsInterlace;
    if(UtopiaIoctl(pInstantGOP,MAPI_CMD_GOP_REGISTER_CB,&ioctl_info)!= UTOPIA_STATUS_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        return GOP_API_FAIL;
    }
#endif

    if(fpXCGetCapHStart == NULL)
    {
        GOP_INFO("[%s][%d] fpXCGetCapHStart is NULL.(If on STR state, this is a normal message) \n",__FUNCTION__,__LINE__);
    }
#ifdef CONFIG_GOP_UTOPIA10
    if(Ioctl_GOP_Register_CB(NULL,GOP_CB_XC_PANEL_HSTART,fpXCGetCapHStart,0) != GOP_API_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN(GOP_API_FAIL);
    }
#else

    ioctl_info.cb_type = GOP_CB_XC_PANEL_HSTART;
    ioctl_info.pCB = fpXCGetCapHStart;
    if(UtopiaIoctl(pInstantGOP,MAPI_CMD_GOP_REGISTER_CB,&ioctl_info)!= UTOPIA_STATUS_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        return GOP_API_FAIL;
    }
#endif

    if(fpXCReduceBWForOSD == NULL)
    {
#ifdef MSOS_TYPE_NOS
        GOP_INFO("[%s][%d] fpXCReduceBWForOSD is NULL.(If on STR state, this is a normal message) \n",__FUNCTION__,__LINE__);
#endif
    }
#ifdef CONFIG_GOP_UTOPIA10
    if(Ioctl_GOP_Register_CB(NULL,GOP_CB_XC_REDUCE_BW,fpXCReduceBWForOSD,0) != GOP_API_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN(GOP_API_FAIL);
    }
#else
    ioctl_info.cb_type = GOP_CB_XC_REDUCE_BW;
    ioctl_info.pCB = fpXCReduceBWForOSD;
    if(UtopiaIoctl(pInstantGOP,MAPI_CMD_GOP_REGISTER_CB,&ioctl_info)!= UTOPIA_STATUS_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        return GOP_API_FAIL;
    }
#endif

    if(fpEventNotify == NULL)
    {
#ifdef MSOS_TYPE_NOS
        GOP_INFO("[%s][%d] fpEventNotify is NULL.(If on STR state, this is a normal message) \n",__FUNCTION__,__LINE__);
#endif
    }
#ifdef CONFIG_GOP_UTOPIA10
    if(Ioctl_GOP_Register_CB(NULL,GOP_CB_EVENT_NOTIFY,fpEventNotify,0) != GOP_API_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN(GOP_API_FAIL);
    }
#else

    ioctl_info.cb_type = GOP_CB_EVENT_NOTIFY;
    ioctl_info.pCB = fpEventNotify;
    if(UtopiaIoctl(pInstantGOP,MAPI_CMD_GOP_REGISTER_CB,&ioctl_info)!= UTOPIA_STATUS_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        return GOP_API_FAIL;
    }
#endif
    if(fpXCSetDwinInfo == NULL)
    {
#ifdef MSOS_TYPE_NOS
        GOP_INFO("[%s][%d] fpXCSetDwinInfo is NULL.(If on STR state, this is a normal message) \n",__FUNCTION__,__LINE__);
#endif
    }
#ifdef CONFIG_GOP_UTOPIA10
    if(Ioctl_GOP_Register_CB(NULL,GOP_CB_DWIN_INFO,fpXCSetDwinInfo,0) != GOP_API_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN(GOP_API_FAIL);
    }
#else
    ioctl_info.cb_type = GOP_CB_DWIN_INFO;
    ioctl_info.pCB = fpXCSetDwinInfo;
    if(UtopiaIoctl(pInstantGOP,MAPI_CMD_GOP_REGISTER_CB,&ioctl_info)!= UTOPIA_STATUS_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        return GOP_API_FAIL;
    }
#endif

#endif
    return GOP_API_SUCCESS;
}
#ifndef CONFIG_GOP_REDUCE_CODESIZE_FOR_MBOOT


/********************************************************************************/
/// Get gop static api info.
/// @param *pGopApiInfo \b OUT pointer to api info structure
/// @return GOP_API_SUCCESS - Success
/// @return GOP_API_FAIL - Failure
/********************************************************************************/
E_GOP_API_Result MApi_GOP_GetInfo(GOP_ApiInfo* pGopApiInfo)
{
    GOP_ApiInfo  GopApiInfo;

    GOP_ENTRY();

    memcpy(&GopApiInfo, pGopApiInfo, sizeof(GOP_ApiInfo));
#ifdef CONFIG_GOP_UTOPIA10
    if(Ioctl_GOP_MISC(NULL,E_GOP_MISC_GET_INFO,(MS_U32*)&GopApiInfo,sizeof(GOP_ApiInfo)) != GOP_API_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN(GOP_API_FAIL);
    }
#else
    GOP_MISC_PARAM ioctl_info;
    memset(&ioctl_info, 0x0, sizeof(GOP_MISC_PARAM));

    ioctl_info.misc_type = E_GOP_MISC_GET_INFO;
    ioctl_info.pMISC = (MS_U32*)&GopApiInfo;
    ioctl_info.u32Size  = sizeof(GOP_ApiInfo);

    if(UtopiaIoctl(pInstantGOP,MAPI_CMD_GOP_MISC,&ioctl_info)!= UTOPIA_STATUS_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN( GOP_API_FAIL);
    }
#endif
    memcpy(pGopApiInfo, &GopApiInfo, sizeof(GOP_ApiInfo));
    GOP_RETURN(GOP_API_SUCCESS);
}


/********************************************************************************/
/// Get gop run time api status.
/// @param *GOP_ApiStatus \b OUT pointer to api status structure
/// @return GOP_API_SUCCESS - Success
/// @return GOP_API_FAIL - Failure
/********************************************************************************/
E_GOP_API_Result MApi_GOP_GetStatus(GOP_ApiStatus *pGopApiSts)
{
    GOP_ApiStatus  GopApiSts;

    GOP_ENTRY();

    memcpy(&GopApiSts, pGopApiSts, sizeof(GOP_ApiStatus));
#ifdef CONFIG_GOP_UTOPIA10
    if(Ioctl_GOP_MISC(NULL,E_GOP_MISC_GET_RT_STATUS,(MS_U32*)&GopApiSts,sizeof(GOP_ApiStatus)) != GOP_API_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN(GOP_API_FAIL);
    }
#else

    GOP_MISC_PARAM ioctl_info;
    memset(&ioctl_info, 0x0, sizeof(GOP_MISC_PARAM));

    ioctl_info.misc_type = E_GOP_MISC_GET_RT_STATUS;
    ioctl_info.pMISC = (MS_U32*)&GopApiSts;
    ioctl_info.u32Size  = sizeof(GOP_ApiStatus);

    if(UtopiaIoctl(pInstantGOP,MAPI_CMD_GOP_MISC,&ioctl_info)!= UTOPIA_STATUS_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN( GOP_API_FAIL);
    }
#endif
    memcpy(pGopApiSts, &GopApiSts, sizeof(GOP_ApiStatus));
    GOP_RETURN(GOP_API_SUCCESS);
}


/********************************************************************************/
/// Set GOP Power ON.
/// @return GOP_API_SUCCESS - Success
/// @return GOP_API_FAIL - Failure
/********************************************************************************/
E_GOP_API_Result MApi_GOP_PowerOn(void)
{
    MS_U32  value = 0;

    GOP_ENTRY();
#ifdef CONFIG_GOP_UTOPIA10
    if(Ioctl_GOP_MISC(NULL,E_GOP_MISC_POWER_ON,(MS_U32*)&value,sizeof(int)) != GOP_API_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN(GOP_API_FAIL);
    }
#else
    GOP_MISC_PARAM ioctl_info;
    memset(&ioctl_info, 0x0, sizeof(GOP_MISC_PARAM));

    ioctl_info.misc_type = E_GOP_MISC_POWER_ON;
    ioctl_info.pMISC = (MS_U32*)&value;
    ioctl_info.u32Size  = sizeof(MS_U32);

    if(UtopiaIoctl(pInstantGOP,MAPI_CMD_GOP_MISC,&ioctl_info)!= UTOPIA_STATUS_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN( GOP_API_FAIL);
    }
#endif
    GOP_RETURN(GOP_API_SUCCESS);

}
/********************************************************************************/
/// Set GOP Power OFF for saving power.
/// @return GOP_API_SUCCESS - Success
/// @return GOP_API_FAIL - Failure
/********************************************************************************/
E_GOP_API_Result MApi_GOP_PowerOff(void)
{
    MS_U32  value = 0;

    GOP_ENTRY();
#ifdef CONFIG_GOP_UTOPIA10
    if(Ioctl_GOP_MISC(NULL,E_GOP_MISC_POWER_OFF,(MS_U32*)&value,sizeof(int)) != GOP_API_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN(GOP_API_FAIL);
    }
#else
    GOP_MISC_PARAM ioctl_info;
    memset(&ioctl_info, 0x0, sizeof(GOP_MISC_PARAM));

    ioctl_info.misc_type = E_GOP_MISC_POWER_OFF;
    ioctl_info.pMISC = (MS_U32*)&value;
    ioctl_info.u32Size  = sizeof(MS_U32);

    if(UtopiaIoctl(pInstantGOP,MAPI_CMD_GOP_MISC,&ioctl_info)!= UTOPIA_STATUS_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN( GOP_API_FAIL);
    }
#endif
    GOP_RETURN(GOP_API_SUCCESS);

}

/********************************************************************************/
/// Configure panel hstart timing for GOP; For configuring correct OSD position.
/// @param u8GOP \b IN:  GOP number
/// @param u16PanelHStr \b IN horizontal start
/// @return GOP_API_SUCCESS - Success
/// @return GOP_API_FAIL - Failure
/********************************************************************************/
E_GOP_API_Result MApi_GOP_SetGOPHStart(MS_U8 u8GOP,MS_U16 u16PanelHStr)
{
    MS_U32 u32PHstart;

    GOP_ENTRY();

    u32PHstart = u16PanelHStr;
#ifdef CONFIG_GOP_UTOPIA10
    if(Ioctl_GOP_SetProperty(NULL,E_GOP_HSTART,u8GOP,(void *)&u32PHstart,sizeof(MS_U16)) != GOP_API_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN(GOP_API_FAIL);
    }

#else

    GOP_SET_PROPERTY_PARAM ioctl_info;
    memset(&ioctl_info, 0x0, sizeof(GOP_SET_PROPERTY_PARAM));

    ioctl_info.en_pro   = E_GOP_HSTART;
    ioctl_info.gop_idx  = u8GOP;
    ioctl_info.pSetting = &u32PHstart;
    ioctl_info.u32Size  = sizeof(MS_U16);

    if(UtopiaIoctl(pInstantGOP,MAPI_CMD_GOP_SET_PROPERTY,&ioctl_info)!= UTOPIA_STATUS_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN( GOP_API_FAIL);
    }
#endif
    GOP_RETURN(GOP_API_SUCCESS);

}

/********************************************************************************/
/// Configure panel hstart timing for GOP; For configuring correct OSD position.
/// @param u16PanelHStr \b IN horizontal start
/// @return GOP_API_SUCCESS - Success
/// @return GOP_API_FAIL - Failure
/********************************************************************************/
E_GOP_API_Result MApi_GOP_MIXER_SetOutputTiming(GOP_MIXER_TIMINGTYPE mode,  GOP_MixerTiming *pMT)
{
    GOP_ENTRY();

    GOP_MIXER_TIMING_INFO MixerTimingInfo;

    memcpy(&MixerTimingInfo.pMT, pMT, sizeof(GOP_MixerTiming));
    MixerTimingInfo.mode = mode;
#ifdef CONFIG_GOP_UTOPIA10
    if(Ioctl_GOP_MIXER_SetProperty(NULL,E_GOP_MIXER_OUTPUT_TIMING,(MS_U32*)&MixerTimingInfo,sizeof(GOP_MIXER_TIMING_INFO)) != GOP_API_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN(GOP_API_FAIL);
    }
#else
    GOP_MIXER_PROPERTY_PARAM ioctl_info;
    memset(&ioctl_info, 0x0, sizeof(GOP_MIXER_PROPERTY_PARAM));

    ioctl_info.en_mixer_property = E_GOP_MIXER_OUTPUT_TIMING;
    ioctl_info.pMixerProp = (MS_U32*)&MixerTimingInfo;
    ioctl_info.u32Size = sizeof(GOP_MIXER_TIMING_INFO);

    if(UtopiaIoctl(pInstantGOP,MAPI_CMD_GOP_MIXER_SET_PROPERTY,&ioctl_info)!= UTOPIA_STATUS_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN( GOP_API_FAIL);
    }
#endif
    GOP_RETURN(GOP_API_SUCCESS);
}

/********************************************************************************/
/// Configure mixer timing for GOP destination MIXER2OP; For configuring correct OSD position.
/// @param pMT \b IN:  mixer timing
/// @return GOP_API_SUCCESS - Success
/// @return GOP_API_FAIL - Failure
/********************************************************************************/
E_GOP_API_Result MApi_GOP_MIXER_SetMIXER2OPOutputTiming(GOP_Mixer2OPTiming *pMT)
{
    GOP_ENTRY();

    GOP_Mixer2OPTiming Mixer2OPTiming;

    memcpy(&Mixer2OPTiming, pMT, sizeof(GOP_Mixer2OPTiming));
#ifdef CONFIG_GOP_UTOPIA10
    if(Ioctl_GOP_MIXER_SetProperty(NULL,E_GOP_MIXER_TO_OP_OUTPUT_TIMING,(MS_U32*)&Mixer2OPTiming,sizeof(GOP_Mixer2OPTiming)) != GOP_API_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN(GOP_API_FAIL);
    }
#else

    GOP_MIXER_PROPERTY_PARAM ioctl_info;
    memset(&ioctl_info, 0x0, sizeof(GOP_MIXER_PROPERTY_PARAM));

    ioctl_info.en_mixer_property = E_GOP_MIXER_TO_OP_OUTPUT_TIMING;
    ioctl_info.pMixerProp = (MS_U32*)&Mixer2OPTiming;
    ioctl_info.u32Size = sizeof(GOP_Mixer2OPTiming);

    if(UtopiaIoctl(pInstantGOP,MAPI_CMD_GOP_MIXER_SET_PROPERTY,&ioctl_info)!= UTOPIA_STATUS_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN( GOP_API_FAIL);
    }
#endif
    GOP_RETURN(GOP_API_SUCCESS);
}

/********************************************************************************/
/// Enable Mixer for Vfilter
/// @param bEnable \b IN:
///   - # TRUE Enable Vfilter
///   - # FALSE Disable Vfilter
/// @return GOP_API_SUCCESS - Success
/// @return GOP_API_FAIL - Failure
/********************************************************************************/
E_GOP_API_Result MApi_GOP_MIXER_EnableVfilter(MS_BOOL bEnable)
{
    GOP_ENTRY();

	MS_U32 u32Ena;

	u32Ena = (MS_U32)bEnable;
#ifdef CONFIG_GOP_UTOPIA10
    if(Ioctl_GOP_MIXER_SetProperty(NULL,E_GOP_MIXER_V_FILTER,&u32Ena,sizeof(MS_BOOL)) != GOP_API_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN(GOP_API_FAIL);
    }
#else

    GOP_MIXER_PROPERTY_PARAM ioctl_info;
    memset(&ioctl_info, 0x0, sizeof(GOP_MIXER_PROPERTY_PARAM));

    ioctl_info.en_mixer_property = E_GOP_MIXER_V_FILTER;
    ioctl_info.pMixerProp = (void*)&u32Ena;
    ioctl_info.u32Size = sizeof(MS_BOOL);

    if(UtopiaIoctl(pInstantGOP,MAPI_CMD_GOP_MIXER_SET_PROPERTY,&ioctl_info)!= UTOPIA_STATUS_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN( GOP_API_FAIL);
    }
#endif
    GOP_RETURN(GOP_API_SUCCESS);
}

/*******************************************************************************/
//Set which MUX select which GOP of mixer, when different gop do the alpha blending
/// @param u8GOP \b IN:  GOP number
/// @param u8Mux \b IN:  Mux number
/// @param bEn \b IN:
///   - # TRUE Enable u8GOP to mixer u8Mux
///   - # FALSE Disable u8GOP to mixer u8Mux
//@return GOP_API_SUCCESS - Success
/*******************************************************************************/
E_GOP_API_Result MApi_GOP_MIXER_SetMux(MS_U8 u8GOP,MS_U8 u8Mux, MS_BOOL bEn) //jasmine2
{
    GOP_ENTRY();

    //Need to implement


    GOP_RETURN(GOP_API_SUCCESS);
}

//******************************************************************************
/// Call this API to Nofify GOP could restore from vsync. limitation. That
/// means App. could switch back to its dst.
/// @return GOP_API_SUCCESS - Success
//******************************************************************************
E_GOP_API_Result MApi_GOP_RestoreFromVsyncLimitation(void)
{
    GOP_ENTRY();
    GOP_ERR("[%s] Not SUPPORT\n",__FUNCTION__);
    GOP_RETURN(GOP_API_SUCCESS);
}

/********************************************************************************/
/// Configure GOP brightness
/// @param u8GOP \b IN:  GOP number
/// @param u16BriVal \b IN brightness value
/// @param bMSB \b IN MSB
/// @return GOP_API_SUCCESS - Success
/// @return GOP_API_FAIL - Failure
/********************************************************************************/

E_GOP_API_Result MApi_GOP_SetGOPBrightness(MS_U8 u8GOP,MS_U16 u16BriVal,MS_BOOL bMSB)
{
    GOP_BRIGHTNESS  bri;

    GOP_ENTRY();

    bri.bMSB = bMSB;
    bri.Brightness = u16BriVal;
#ifdef CONFIG_GOP_UTOPIA10
    if(Ioctl_GOP_SetProperty(NULL,E_GOP_BRIGHTNESS,u8GOP,(MS_U32*)&bri,sizeof(GOP_BRIGHTNESS)) != GOP_API_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN(GOP_API_FAIL);
    }
#else

    GOP_SET_PROPERTY_PARAM ioctl_info;
    memset(&ioctl_info, 0x0, sizeof(GOP_SET_PROPERTY_PARAM));

    ioctl_info.en_pro   = E_GOP_BRIGHTNESS;
    ioctl_info.gop_idx  = u8GOP;
    ioctl_info.pSetting = (MS_U32*)&bri;
    ioctl_info.u32Size  = sizeof(GOP_BRIGHTNESS);

    if(UtopiaIoctl(pInstantGOP,MAPI_CMD_GOP_SET_PROPERTY,&ioctl_info)!= UTOPIA_STATUS_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN( GOP_API_FAIL);
    }
#endif
    GOP_RETURN(GOP_API_SUCCESS);

}

/********************************************************************************/
/// Get GOP brightness value
/// @param u8GOP \b IN:  GOP number
/// @param u16BriVal \b OUT brightness value
/// @param bMSB \b OUT MSB
/// @return GOP_API_SUCCESS - Success
/// @return GOP_API_FAIL - Failure
/********************************************************************************/
E_GOP_API_Result MApi_GOP_GetGOPBrightness(MS_U8 u8GOP,MS_U16* u16BriVal,MS_BOOL* bMSB)
{
    GOP_BRIGHTNESS  bri;

    GOP_ENTRY();

    memset(&bri, 0x0, sizeof(GOP_BRIGHTNESS));
#ifdef CONFIG_GOP_UTOPIA10
    if(Ioctl_GOP_GetProperty(NULL,E_GOP_BRIGHTNESS,u8GOP,(void*)&bri,sizeof(GOP_BRIGHTNESS)) != GOP_API_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN(GOP_API_FAIL);
    }
#else

    GOP_SET_PROPERTY_PARAM ioctl_info;
    memset(&ioctl_info, 0x0, sizeof(GOP_SET_PROPERTY_PARAM));

    ioctl_info.en_pro   = E_GOP_BRIGHTNESS;
    ioctl_info.gop_idx  = u8GOP;
    ioctl_info.pSetting = (void*)&bri;
    ioctl_info.u32Size  = sizeof(GOP_BRIGHTNESS);

    if(UtopiaIoctl(pInstantGOP,MAPI_CMD_GOP_GET_PROPERTY,&ioctl_info)!= UTOPIA_STATUS_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN( GOP_API_FAIL);
    }
#endif
    *bMSB      = bri.bMSB;
    *u16BriVal = bri.Brightness;

    GOP_RETURN(GOP_API_SUCCESS);

}

//******************************************************************************
/// API for enable or disable Tile mode for GWIN
/// @param winId \b IN: GWIN id
/// @param bEnable \b IN:
///   - # TRUE Enable Tile Mode for GWIN
///   - # FALSE Disable Tile Mode for GWIN
/// @return GOP_API_SUCCESS - Success
/// @return GOP_API_FAIL - Failure

//******************************************************************************
E_GOP_API_Result MApi_GOP_GWIN_EnableTileMode(MS_U8 winId, MS_BOOL bEnable, EN_GOP_TILE_DATA_TYPE tiletype )
{
    GOP_ENTRY();
    GOP_ERR("[%s] Not SUPPORT bEn:%d type:%d\n",__FUNCTION__,bEnable, tiletype);
    GOP_RETURN(GOP_API_SUCCESS);
}
#endif

#ifndef CONFIG_GOP_REDUCE_CODESIZE_FOR_MBOOT
//******************************************************************************
/// API for set gop pinpong
/// @param winId \b IN: GWIN id
/// @param bEnable \b IN:
///   - # TRUE Enable Tile Mode for GWIN
///   - # FALSE Disable Tile Mode for GWIN
/// @return GOP_API_SUCCESS - Success
/// @return GOP_API_FAIL - Failure

//******************************************************************************
E_GOP_API_Result MApi_GOP_SetPINPON(MS_U8 winId, MS_BOOL bEn,GOP_PINPON_MODE PINPON_Mode)
{
    GOP_SET_PINPON_INFO pinpon_info;
    switch(PINPON_Mode)
    {
        case GOP_PINPON_G3D:
            pinpon_info.mode = E_GOP_PINPON_G3D;
            break;
        case GOP_PINPON_VE:
            pinpon_info.mode = E_GOP_PINPON_VE;
            break;
        case GOP_PINPON_DWIN:
            pinpon_info.mode = E_GOP_PINPON_DWIN;
            break;
        case GOP_PINPON_DIP:
            pinpon_info.mode = E_GOP_PINPON_DIP;
            break;
        case GOP_PINPON_DWIN0:
            pinpon_info.mode = E_GOP_PINPON_DWIN0;
            break;
        case GOP_PINPON_DWIN1:
            pinpon_info.mode = E_GOP_PINPON_DWIN1;
            break;
        default:
            GOP_ERR("MApi_GOP_SetPINPON Enum not mapping!\n");
            return GOP_API_INVALID_PARAMETERS;
    }
    pinpon_info.GwinId  = winId;
    pinpon_info.bEnable = bEn;

    GOP_ENTRY();
#ifdef CONFIG_GOP_UTOPIA10
    if(Ioctl_GOP_PINPON(NULL,E_GOP_SET_PINPON,(MS_U32*)&pinpon_info,sizeof(GOP_SET_PINPON_INFO)) != GOP_API_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN(GOP_API_FAIL);
    }
#else

    GOP_SET_PINPON_PARAM ioctl_info;

    ioctl_info.en_pro   = E_GOP_SET_PINPON;
    ioctl_info.pSetting = (MS_U32*)&pinpon_info;
    ioctl_info.u32Size  = sizeof(GOP_SET_PINPON_INFO);

    if(UtopiaIoctl(pInstantGOP,MAPI_CMD_GOP_SET_PINPON,&ioctl_info)!= UTOPIA_STATUS_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN(GOP_API_FAIL);
    }
#endif
    GOP_RETURN(GOP_API_SUCCESS);
}

//******************************************************************************
/// API for set U/V SWAP
/// @param u8GOP \b IN: GOP number
/// @param bEnable \b IN:
///   - # TRUE Enable U/V SWAP for GOP
///   - # FALSE Disable U/V SWAP for GOP
/// @return GOP_API_SUCCESS - Success
/// @return GOP_API_FAIL - Failure
//******************************************************************************
E_GOP_API_Result MApi_GOP_SetUVSwap(MS_U8 u8GOP, MS_BOOL bEn)
{
    MS_U32  UVSwapEn;

    GOP_ENTRY();

    UVSwapEn = bEn;
#ifdef CONFIG_GOP_UTOPIA10
    if(Ioctl_GOP_SetProperty(NULL,E_GOP_UV_SWAP,u8GOP,(MS_U32*)&UVSwapEn,sizeof(MS_BOOL)) != GOP_API_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN(GOP_API_FAIL);
    }
#else
    GOP_SET_PROPERTY_PARAM ioctl_info;
    memset(&ioctl_info, 0x0, sizeof(GOP_SET_PROPERTY_PARAM));

    ioctl_info.en_pro   = E_GOP_UV_SWAP;
    ioctl_info.gop_idx  = u8GOP;
    ioctl_info.pSetting = (MS_U32*)&UVSwapEn;
    ioctl_info.u32Size  = sizeof(MS_BOOL);

    if(UtopiaIoctl(pInstantGOP,MAPI_CMD_GOP_SET_PROPERTY,&ioctl_info)!= UTOPIA_STATUS_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN( GOP_API_FAIL);
    }
#endif
    GOP_RETURN(GOP_API_SUCCESS);
}

//******************************************************************************
/// API for set Y/C SWAP
/// @param u8GOP \b IN: GOP number
/// @param bEnable \b IN:
///   - # TRUE Enable Y/C SWAP for GOP
///   - # FALSE Disable Y/C SWAP for GOP
/// @return GOP_API_SUCCESS - Success
/// @return GOP_API_FAIL - Failure
//******************************************************************************
E_GOP_API_Result MApi_GOP_SetYCSwap(MS_U8 u8GOP, MS_BOOL bEn)
{

    MS_U32  YCSwapEn;

    GOP_ENTRY();

    YCSwapEn = bEn;
#ifdef CONFIG_GOP_UTOPIA10
    if(Ioctl_GOP_SetProperty(NULL,E_GOP_YC_SWAP,u8GOP,(MS_U32*)&YCSwapEn,sizeof(MS_BOOL)) != GOP_API_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN(GOP_API_FAIL);
    }
#else
    GOP_SET_PROPERTY_PARAM ioctl_info;
    memset(&ioctl_info, 0x0, sizeof(GOP_SET_PROPERTY_PARAM));

    ioctl_info.en_pro   = E_GOP_YC_SWAP;
    ioctl_info.gop_idx  = u8GOP;
    ioctl_info.pSetting = (MS_U32*)&YCSwapEn;
    ioctl_info.u32Size  = sizeof(MS_BOOL);

    if(UtopiaIoctl(pInstantGOP,MAPI_CMD_GOP_SET_PROPERTY,&ioctl_info)!= UTOPIA_STATUS_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN( GOP_API_FAIL);
    }
#endif
    GOP_RETURN(GOP_API_SUCCESS);
}

//******************************************************************************
/// API for set GWIN New Alpha Mode
/// @param gWinId \b IN: GWin ID
/// @param bEnable \b IN:
///   - # TRUE enable new alpha mode
///   - # FALSE disable new alpha mode
/// @return GOP_API_SUCCESS - Success
/// @return GOP_API_FAIL - Failure
//******************************************************************************
E_GOP_API_Result MApi_GOP_GWIN_SetNewAlphaMode(MS_U8 gWinId, MS_BOOL bEnable)
{
	MS_U32 u32Enable;

    GOP_ENTRY();

	u32Enable = (MS_U32)bEnable;
#ifdef CONFIG_GOP_UTOPIA10
    if(Ioctl_GOP_GWin_SetProperty(NULL,E_GOP_GWIN_NEWAPLHA,gWinId, &u32Enable,sizeof(MS_BOOL)) != GOP_API_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN(GOP_API_FAIL);
    }
#else
    GOP_GWIN_PROPERTY_PARAM ioctl_info;
    memset(&ioctl_info, 0x0, sizeof(GOP_GWIN_PROPERTY_PARAM));

    ioctl_info.en_property = E_GOP_GWIN_NEWAPLHA;
    ioctl_info.GwinId = gWinId;
    ioctl_info.pSet = (void*) &u32Enable;
    ioctl_info.u32Size = sizeof(MS_BOOL);

    if(UtopiaIoctl(pInstantGOP,MAPI_CMD_GOP_GWIN_SET_PROPERTY,&ioctl_info)!= UTOPIA_STATUS_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN( GOP_API_FAIL);
    }
#endif

    GOP_RETURN(GOP_API_SUCCESS);
}

//******************************************************************************
/// API for Trigger reg writes in
/// @param u8win \b IN: GWin ID
/// @param bForceWriteIn \b IN:
///   - # TRUE , Register will take effect immediately
///   - # FALSE, Register will take effect after the next vsync
/// @return GOP_API_SUCCESS - Success
/// @return GOP_API_FAIL - Failure
//******************************************************************************
E_GOP_API_Result MApi_GOP_TriggerRegWriteIn(MS_U8 u8win, MS_BOOL bForceWriteIn)
{
    E_GOP_API_Result ret;
    ret = MApi_GOP_TriggerRegWriteIn_Ex(u8win, bForceWriteIn, TRUE);
    return ret;
}

//******************************************************************************
/// API for Trigger reg writes in
/// @param u8win \b IN: GWin ID
/// @param bForceWriteIn \b IN:
///   - # TRUE , Register will take effect immediately
///   - # FALSE, Register will take effect after the next vsync
/// @param bSync \b IN:
///   - # FALSE , Register will fire without waiting vsync (driver will not polling the vsync)
///   - # TRUE, Register will fire and waiting vsync
/// @return GOP_API_SUCCESS - Success
/// @return GOP_API_FAIL - Failure
//******************************************************************************
E_GOP_API_Result MApi_GOP_TriggerRegWriteIn_Ex(MS_U8 u8win, MS_BOOL bForceWriteIn, MS_BOOL bSync)
{
    MS_U32 gop = INVAILD_GOP_NUM;
    GOP_ENTRY();
#ifdef CONFIG_GOP_UTOPIA10
    if(Ioctl_GOP_GWin_GetProperty(NULL,E_GOP_GWIN_GET_GOP,u8win,(MS_U32*)&gop,sizeof(MS_U32)) != GOP_API_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN(GOP_API_FAIL);
    }
#else
    GOP_GWIN_PROPERTY_PARAM ioctl_gwin_info;
    memset(&ioctl_gwin_info, 0x0, sizeof(GOP_GWIN_PROPERTY_PARAM));

    ioctl_gwin_info.en_property = E_GOP_GWIN_GET_GOP;
    ioctl_gwin_info.GwinId = u8win;
    ioctl_gwin_info.pSet = (MS_U32*)&gop;
    ioctl_gwin_info.u32Size = sizeof(MS_U32);
    if (UtopiaIoctl(pInstantGOP,MAPI_CMD_GOP_GWIN_GET_PROPERTY,(void*)&ioctl_gwin_info) != UTOPIA_STATUS_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN(GOP_API_FAIL);
    }
#endif

#ifdef CONFIG_GOP_UTOPIA10
    if(Ioctl_GOP_TriggerRegWriteIn(NULL,gop,E_GOP_UPDATE_FORCEWRITE,bForceWriteIn,bSync) != GOP_API_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN(GOP_API_FAIL);
    }
#else

    GOP_UPDATE_INFO update;

    memset(&update, 0x0, sizeof(GOP_UPDATE_INFO));

    update.gop_idx = gop;
    update.update_type = E_GOP_UPDATE_FORCEWRITE;
    update.bEn = bForceWriteIn;
    update.bSync = bSync;

    GOP_UPDATE_PARAM ioctl_info;
    memset(&ioctl_info, 0x0, sizeof(GOP_UPDATE_PARAM));

    ioctl_info.pUpdateInfo = (MS_U32*)&update;
    ioctl_info.u32Size = sizeof(GOP_UPDATE_INFO);

    if(UtopiaIoctl(pInstantGOP,MAPI_CMD_GOP_UPDATE,&ioctl_info)!= UTOPIA_STATUS_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN( GOP_API_FAIL);
    }

#endif

    GOP_RETURN(GOP_API_SUCCESS);


}



E_GOP_API_Result MApi_GOP_SetGOPContrast( MS_U8 u8GOP, MS_U16 u16YContrast
    , MS_U16 u16UContrast, MS_U16 u16VContrast )
{
    GOP_CONTRAST  contrast;
    MS_U32 u32Ret = 0;

    GOP_ENTRY();

    contrast.y = u16YContrast;
    contrast.u = u16UContrast;
    contrast.v = u16VContrast;
#ifdef CONFIG_GOP_UTOPIA10
    u32Ret = Ioctl_GOP_SetProperty(NULL,E_GOP_CONTRAST,u8GOP,(MS_U32*)&contrast,sizeof(GOP_CONTRAST));
    if(u32Ret != GOP_API_SUCCESS)
    {
        if((E_GOP_API_Result)u32Ret == GOP_API_FUN_NOT_SUPPORTED)
        {
            GOP_ERR("[%s]Function not support\n",__FUNCTION__);
            GOP_RETURN(GOP_API_FUN_NOT_SUPPORTED);
        }
        else
        {
            GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
            GOP_RETURN(GOP_API_FAIL);
        }
    }
#else
    GOP_SET_PROPERTY_PARAM ioctl_info;
    memset(&ioctl_info, 0x0, sizeof(GOP_SET_PROPERTY_PARAM));

    ioctl_info.en_pro   = E_GOP_CONTRAST;
    ioctl_info.gop_idx  = u8GOP;
    ioctl_info.pSetting = (MS_U32*)&contrast;
    ioctl_info.u32Size  = sizeof(GOP_CONTRAST);

    u32Ret = UtopiaIoctl(pInstantGOP,MAPI_CMD_GOP_SET_PROPERTY,&ioctl_info);
    if(u32Ret != UTOPIA_STATUS_SUCCESS)
    {
        if(u32Ret == UTOPIA_STATUS_NOT_SUPPORTED)
        {
            GOP_ERR("[%s]Function not support\n",__FUNCTION__);
            GOP_RETURN(GOP_API_FUN_NOT_SUPPORTED);
        }
        else
        {
            GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
            GOP_RETURN( GOP_API_FAIL);
        }
    }
#endif
    GOP_RETURN(GOP_API_SUCCESS);
}

E_GOP_API_Result MApi_GOP_GetGOPContrast( MS_U8 u8GOP, MS_U16* u16YContrast
    , MS_U16* u16UContrast, MS_U16* u16VContrast )
{

    GOP_CONTRAST  contrast;
    MS_U32 u32Ret = 0;

    GOP_ENTRY();
#ifdef CONFIG_GOP_UTOPIA10
    u32Ret=Ioctl_GOP_GetProperty(NULL,E_GOP_CONTRAST,u8GOP,(MS_U32*)&contrast,sizeof(GOP_CONTRAST));
    if(u32Ret != GOP_API_SUCCESS)
    {
        if((E_GOP_API_Result)u32Ret == GOP_API_FUN_NOT_SUPPORTED)
        {
            GOP_ERR("[%s]Function not support\n",__FUNCTION__);
            GOP_RETURN(GOP_API_FUN_NOT_SUPPORTED);
        }
        else
        {
            GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
            GOP_RETURN(GOP_API_FAIL);
        }
    }
#else
    GOP_SET_PROPERTY_PARAM ioctl_info;
    memset(&ioctl_info, 0x0, sizeof(GOP_SET_PROPERTY_PARAM));

    ioctl_info.en_pro   = E_GOP_CONTRAST;
    ioctl_info.gop_idx  = u8GOP;
    ioctl_info.pSetting = (MS_U32*)&contrast;
    ioctl_info.u32Size  = sizeof(GOP_CONTRAST);

    u32Ret = UtopiaIoctl(pInstantGOP,MAPI_CMD_GOP_GET_PROPERTY,&ioctl_info);

    if(u32Ret != UTOPIA_STATUS_SUCCESS)
    {
        if(u32Ret == UTOPIA_STATUS_NOT_SUPPORTED)
        {
            GOP_ERR("[%s]Function not support\n",__FUNCTION__);
            GOP_RETURN(GOP_API_FUN_NOT_SUPPORTED);
        }
        else
        {
            GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
            GOP_RETURN( GOP_API_FAIL);
        }
    }
#endif
    *u16YContrast = contrast.y;
    *u16UContrast = contrast.u;
    *u16VContrast = contrast.v;

    GOP_RETURN(GOP_API_SUCCESS);

}

E_GOP_API_Result MApi_GOP_GetVECaptureState(PMS_GOP_VECAPTURESTATE pstVECapState)
{
    GOP_ERR("[%s] Not SUPPORT in utopia2\n",__FUNCTION__);
    return GOP_API_FUN_NOT_SUPPORTED;
}

E_GOP_API_Result MApi_GOP_VECaptureWaitOnFrame(PMS_GOP_VECAPTURESTATE pstVECapState)
{
    GOP_ERR("[%s] Not SUPPORT in utopia2\n",__FUNCTION__);
    return GOP_API_FUN_NOT_SUPPORTED;
}

E_GOP_API_Result MApi_GOP_EnaVECapture(PMS_GOP_VECAPTURESTATE pstVECapState)
{
    GOP_ERR("[%s] Not SUPPORT in utopia2\n",__FUNCTION__);
    return GOP_API_FUN_NOT_SUPPORTED;
}

//******************************************************************************
/// API for Enable VEOSD
/// @param u8win \b IN: GOP ID
/// @return GOP_API_SUCCESS - Success
/// @return GOP_API_FAIL - Failure
//******************************************************************************
E_GOP_API_Result MApi_GOP_Enable_VEOSD(MS_U8 u8GOP, MS_BOOL bEn)
{
	MS_U32 u32Ena;

    GOP_ENTRY();

	u32Ena = (MS_U32)bEn;
#ifdef CONFIG_GOP_UTOPIA10
    if(Ioctl_GOP_SetProperty(NULL,E_GOP_VE_OSD,u8GOP, &u32Ena,sizeof(MS_BOOL)) != GOP_API_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN(GOP_API_FAIL);
    }
#else
    GOP_SET_PROPERTY_PARAM ioctl_info;
    memset(&ioctl_info, 0x0, sizeof(GOP_SET_PROPERTY_PARAM));

    ioctl_info.en_pro   = E_GOP_VE_OSD;
    ioctl_info.gop_idx  = u8GOP;
    ioctl_info.pSetting = (void*)&u32Ena;
    ioctl_info.u32Size  = sizeof(MS_BOOL);

    if(UtopiaIoctl(pInstantGOP,MAPI_CMD_GOP_SET_PROPERTY,&ioctl_info)!= UTOPIA_STATUS_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN( GOP_API_FAIL);
    }
#endif
    GOP_RETURN(GOP_API_SUCCESS);
}

#endif
/*******************************************************************************/
//Set configuration for special cases
///        - This is for special case usage. Do NOT overuse this function
/// @param type \b IN:  Config type
///        - Think twice if you are going to add one type
///        - Be careful to create config type. Do NOT affect the normal code flow
/// @param plist \b IN:  Config list for the specific type
//         - This list could be an enum or a structure
//@return GOP_API_SUCCESS - Success
/*******************************************************************************/
E_GOP_API_Result MApi_GOP_SetConfig(EN_GOP_CONFIG_TYPE type, void *plist)
{
    MS_U32 u32Size =0;

    CheckGOPInstanceOpen();

    switch(type)
    {
        case E_GOP_IGNOREINIT:
        {
            u32Size = sizeof(EN_GOP_IGNOREINIT);
            break;
        }
        case E_GOP_CONSALPHA_BITS:
        {
            u32Size = sizeof(EN_GOP_CONSALPHA_BITS);
            break;
        }
        case E_GOP_AUTO_DETECT_BUFFER:
        {
            u32Size = sizeof(ST_GOP_AUTO_DETECT_BUF_INFO);
            break;
        }
        default:
            u32Size = 0;
            break;
    }

#ifdef CONFIG_GOP_UTOPIA10
    if(Ioctl_GOP_SetConfig(NULL,type,plist,u32Size) != GOP_API_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN(GOP_API_FAIL);
    }
#else
    if(type == E_GOP_AUTO_DETECT_BUFFER)
    {
        PST_GOP_AUTO_DETECT_BUF_INFO pstGopAutoDetectInfo = NULL;
        ST_GOP_AUTO_DETECT_BUF_INFO stGopAutoDetectInfo;
        MS_U32 u32CopiedLength = 0;

        memset(&stGopAutoDetectInfo, 0, sizeof(ST_GOP_AUTO_DETECT_BUF_INFO));
        pstGopAutoDetectInfo = (PST_GOP_AUTO_DETECT_BUF_INFO)plist;
        u32CopiedLength = sizeof(ST_GOP_AUTO_DETECT_BUF_INFO);

        if(pstGopAutoDetectInfo == NULL)
        {
            GOP_ERR("[Error] MApi_GOP_SetConfig: Null parameter!!\n");
            return GOP_API_FAIL;
        }
        if(pstGopAutoDetectInfo->u32Version < 1)
        {
            GOP_ERR("[Error] MApi_GOP_SetConfig: please check your u32Version, it should not set to 0!!\n");
            return GOP_API_FAIL;
        }
        if(pstGopAutoDetectInfo->u32Version > ST_GOP_AUTO_DETECT_BUF_INFO_VERSION)
        {
#if defined (__aarch64__)
            GOP_WARN("[Warning]MApi_GOP_SetConfig: this old version GOP lib has only length:%lu driver status!!\n",sizeof(ST_GOP_AUTO_DETECT_BUF_INFO));
#else
            GOP_WARN("[Warning]MApi_GOP_SetConfig: this old version GOP lib has only length:%u driver status!!\n",sizeof(ST_GOP_AUTO_DETECT_BUF_INFO));
#endif
        }
        if((pstGopAutoDetectInfo->u32Version < ST_GOP_AUTO_DETECT_BUF_INFO_VERSION) || (pstGopAutoDetectInfo->u32Length < sizeof(ST_GOP_AUTO_DETECT_BUF_INFO)))
        {
            GOP_WARN("[Warning]MApi_GOP_SetConfig: new version GOP lib shouldn't access to the space which doesn't exist in old structure.\n");
            u32CopiedLength = pstGopAutoDetectInfo->u32Length;
        }
        memcpy(&stGopAutoDetectInfo, pstGopAutoDetectInfo, u32CopiedLength);

        GOP_SETCONFIG_PARAM stIoctl_info;
        memset(&stIoctl_info, 0x0, sizeof(GOP_SETCONFIG_PARAM));

        stIoctl_info.cfg_type = type;
        stIoctl_info.pCfg = &stGopAutoDetectInfo;
        stIoctl_info.u32Size = sizeof(ST_GOP_AUTO_DETECT_BUF_INFO);

        if (UtopiaIoctl(pInstantGOP,MAPI_CMD_GOP_SET_CONFIG,(void*)&stIoctl_info) != UTOPIA_STATUS_SUCCESS)
        {
            GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
            return GOP_API_FAIL;
        }

        if(pstGopAutoDetectInfo->u32Version >= 1)
        {
            memcpy(pstGopAutoDetectInfo, &stGopAutoDetectInfo, u32CopiedLength);
        }
    }
    else
    {
        GOP_SETCONFIG_PARAM ioctl_info;
        memset(&ioctl_info, 0x0, sizeof(GOP_SETCONFIG_PARAM));

        ioctl_info.cfg_type = type;
        ioctl_info.pCfg = plist;
        ioctl_info.u32Size = u32Size;

        if (UtopiaIoctl(pInstantGOP,MAPI_CMD_GOP_SET_CONFIG,(void*)&ioctl_info) != UTOPIA_STATUS_SUCCESS)
        {
            GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
            return GOP_API_FAIL;
        }
    }
#endif

    return GOP_API_SUCCESS;


}
#ifndef CONFIG_GOP_REDUCE_CODESIZE_FOR_MBOOT

/********************************************************************************/
/// Set config by GOP; For dynamic usage.
/// @param u8GOP \b IN:  GOP number
/// @param pstInfo \b IN misc info
/// @return GOP_API_SUCCESS - Success
/// @return GOP_API_FAIL - Failure
/********************************************************************************/
E_GOP_API_Result MApi_GOP_SetConfigEx(MS_U8 u8Gop, EN_GOP_CONFIG_TYPE enType, void *plist)
{
#ifdef CONFIG_GOP_UTOPIA10
    switch(enType)
    {
        case E_GOP_IGNOREINIT:
        {
            if(Ioctl_GOP_SetConfig(NULL,enType,plist,sizeof(EN_GOP_IGNOREINIT)) != GOP_API_SUCCESS)
            {
                GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
                GOP_RETURN(GOP_API_FAIL);
            }
            break;
        }
        case E_GOP_CONSALPHA_BITS:
        {
            if(Ioctl_GOP_SetConfig(NULL,enType,plist,sizeof(EN_GOP_IGNOREINIT)) != GOP_API_SUCCESS)
            {
                GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
                GOP_RETURN(GOP_API_FAIL);
            }
            break;
        }
        case E_GOP_TIMING_INFO:
        {
            if(Ioctl_GOP_SetProperty(NULL,E_GOP_OUTPUT_TIMING,u8Gop,plist,sizeof(ST_GOP_TIMING_INFO)) != GOP_API_SUCCESS)
            {
                GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
                GOP_RETURN(GOP_API_FAIL);
            }
            break;
        }
        case E_GOP_IS_PIXELBASE_ENABLE:
        {
            if(Ioctl_GOP_SetProperty(NULL,E_GOP_PIXELBASE_ENABLE,u8Gop,plist,sizeof(ST_GOP_TIMING_INFO)) != GOP_API_SUCCESS)
            {
                GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
                GOP_RETURN(GOP_API_FAIL);
            }
            break;
        }
        case E_GOP_AFBC_RESET:
        {
            if(Ioctl_GOP_SetProperty(NULL,E_GOP_AFBC_CORE_RESET,u8Gop,plist,sizeof(MS_BOOL)) != GOP_API_SUCCESS)
            {
                GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
                GOP_RETURN(GOP_API_FAIL);
            }
            break;
        }
        case E_GOP_AFBC_ENABLE:
        {
            if(Ioctl_GOP_SetProperty(NULL,E_GOP_AFBC_CORE_ENABLE,u8Gop,plist,sizeof(MS_BOOL)) != GOP_API_SUCCESS)
            {
                GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
                GOP_RETURN(GOP_API_FAIL);
            }
            break;
        }
        case E_GOP_VOP_PATH_SEL:
            if(Ioctl_GOP_SetProperty(NULL,E_GOP_SET_VOP_PATH,u8Gop,plist,sizeof(EN_GOP_VOP_PATH_MODE)) != GOP_API_SUCCESS)
            {
                GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
                GOP_RETURN(GOP_API_FAIL);
            }
            break;
        case E_GOP_AUTO_DETECT_BUFFER:
            if(Ioctl_GOP_SetConfig(NULL,enType,plist,sizeof(ST_GOP_AUTO_DETECT_BUF_INFO)) != GOP_API_SUCCESS)
            {
                GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
                GOP_RETURN(GOP_API_FAIL);
            }
            break;
        default:
            return GOP_API_INVALID_PARAMETERS;
            break;
    }
// Utopipa 2.0
#else
    E_GOP_API_CMD_TYPE enApiCmdType = MAPI_CMD_GOP_SET_CONFIG;
    GOP_SETCONFIG_PARAM stIoctlConfigInfo;
    GOP_SET_PROPERTY_PARAM stIoctlPropInfo;
    PST_GOP_AUTO_DETECT_BUF_INFO pstGopAutoDetectInfo = NULL;
    ST_GOP_AUTO_DETECT_BUF_INFO stGopAutoDetectInfo;
    void *pInfo = NULL;
    MS_U32 u32CopiedLength = 0;

    memset(&stIoctlConfigInfo, 0x0, sizeof(GOP_SETCONFIG_PARAM));
    memset(&stIoctlPropInfo, 0x0, sizeof(GOP_SET_PROPERTY_PARAM));

    CheckGOPInstanceOpen();
    switch(enType)
    {
        case E_GOP_IGNOREINIT:
        {
            stIoctlConfigInfo.cfg_type = enType;
            stIoctlConfigInfo.pCfg = plist;
            stIoctlConfigInfo.u32Size = sizeof(EN_GOP_IGNOREINIT);
            pInfo = (void*)&stIoctlConfigInfo;
            enApiCmdType = MAPI_CMD_GOP_SET_CONFIG;
            break;
        }
        case E_GOP_CONSALPHA_BITS:
        {
            stIoctlConfigInfo.cfg_type = enType;
            stIoctlConfigInfo.pCfg = plist;
            stIoctlConfigInfo.u32Size = sizeof(EN_GOP_CONSALPHA_BITS);
            pInfo = (void*)&stIoctlConfigInfo;
            enApiCmdType = MAPI_CMD_GOP_SET_CONFIG;
            break;
        }
        case E_GOP_TIMING_INFO:
        {
            stIoctlPropInfo.en_pro = E_GOP_OUTPUT_TIMING;
            stIoctlPropInfo.gop_idx = u8Gop;
            stIoctlPropInfo.pSetting = plist;
            stIoctlPropInfo.u32Size = sizeof(ST_GOP_TIMING_INFO);
            pInfo = (void*)&stIoctlPropInfo;
            enApiCmdType = MAPI_CMD_GOP_SET_PROPERTY;
            break;
        }
        case E_GOP_IS_PIXELBASE_ENABLE:
        {
            stIoctlPropInfo.en_pro = E_GOP_PIXELBASE_ENABLE;
            stIoctlPropInfo.gop_idx = u8Gop;
            stIoctlPropInfo.pSetting = plist;
            stIoctlPropInfo.u32Size = sizeof(MS_BOOL);
            pInfo = (void*)&stIoctlPropInfo;
            enApiCmdType = MAPI_CMD_GOP_SET_PROPERTY;
            break;
        }
        case E_GOP_AFBC_RESET:
        {
            stIoctlPropInfo.en_pro = E_GOP_AFBC_CORE_RESET;
            stIoctlPropInfo.gop_idx = u8Gop;
            stIoctlPropInfo.pSetting = plist;
            stIoctlPropInfo.u32Size = sizeof(MS_BOOL);
            pInfo = (void*)&stIoctlPropInfo;
            enApiCmdType = MAPI_CMD_GOP_SET_PROPERTY;
            break;
        }
        case E_GOP_AFBC_ENABLE:
        {
            stIoctlPropInfo.en_pro = E_GOP_AFBC_CORE_ENABLE;
            stIoctlPropInfo.gop_idx = u8Gop;
            stIoctlPropInfo.pSetting = plist;
            stIoctlPropInfo.u32Size = sizeof(MS_BOOL);
            pInfo = (void*)&stIoctlPropInfo;
            enApiCmdType = MAPI_CMD_GOP_SET_PROPERTY;
            break;
        }
        case E_GOP_VOP_PATH_SEL:
            stIoctlPropInfo.en_pro = E_GOP_SET_VOP_PATH;
            stIoctlPropInfo.gop_idx = u8Gop;
            stIoctlPropInfo.pSetting = plist;
            stIoctlPropInfo.u32Size = sizeof(EN_GOP_VOP_PATH_MODE);
            pInfo = (void*)&stIoctlPropInfo;
            enApiCmdType = MAPI_CMD_GOP_SET_PROPERTY;
            break;
        case E_GOP_AUTO_DETECT_BUFFER:
        {
            memset(&stGopAutoDetectInfo, 0, sizeof(ST_GOP_AUTO_DETECT_BUF_INFO));
            pstGopAutoDetectInfo = (PST_GOP_AUTO_DETECT_BUF_INFO)plist;
            u32CopiedLength = sizeof(ST_GOP_AUTO_DETECT_BUF_INFO);

            if(pstGopAutoDetectInfo == NULL)
            {
                GOP_ERR("[Error] MApi_GOP_SetConfigEx: Null parameter!!\n");
                return GOP_API_FAIL;
            }
            if(pstGopAutoDetectInfo->u32Version < 1)
            {
                GOP_ERR("[Error] MApi_GOP_SetConfigEx: please check your u32Version, it should not set to 0!!\n");
                return GOP_API_FAIL;
            }
            if(pstGopAutoDetectInfo->u32Version > ST_GOP_AUTO_DETECT_BUF_INFO_VERSION)
            {
#if defined (__aarch64__)
                GOP_WARN("[Warning]MApi_GOP_SetConfigEx: this old version GOP lib has only length:%lu driver status!!\n",sizeof(ST_GOP_AUTO_DETECT_BUF_INFO));
#else
                GOP_WARN("[Warning]MApi_GOP_SetConfigEx: this old version GOP lib has only length:%u driver status!!\n",sizeof(ST_GOP_AUTO_DETECT_BUF_INFO));
#endif
            }
            if((pstGopAutoDetectInfo->u32Version < ST_GOP_AUTO_DETECT_BUF_INFO_VERSION) || (pstGopAutoDetectInfo->u32Length < sizeof(ST_GOP_AUTO_DETECT_BUF_INFO)))
            {
                GOP_WARN("[Warning]MApi_GOP_SetConfigEx: new version GOP lib shouldn't access to the space which doesn't exist in old structure.\n");
                u32CopiedLength = pstGopAutoDetectInfo->u32Length;
            }
            memcpy(&stGopAutoDetectInfo, pstGopAutoDetectInfo, u32CopiedLength);
            memset(&stIoctlConfigInfo, 0x0, sizeof(GOP_SETCONFIG_PARAM));

            stIoctlConfigInfo.cfg_type = enType;
            stIoctlConfigInfo.pCfg = &stGopAutoDetectInfo;
            stIoctlConfigInfo.u32Size = sizeof(ST_GOP_AUTO_DETECT_BUF_INFO);
            pInfo = (void*)&stIoctlConfigInfo;
            enApiCmdType = MAPI_CMD_GOP_SET_CONFIG;
            break;
        }
        default:
            return GOP_API_INVALID_PARAMETERS;
            break;
    }
    if (UtopiaIoctl(pInstantGOP, enApiCmdType, pInfo) != UTOPIA_STATUS_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        return GOP_API_FAIL;
    }

    if(enType == E_GOP_AUTO_DETECT_BUFFER)
    {
         if(pstGopAutoDetectInfo->u32Version >= 1)
         {
            memcpy(pstGopAutoDetectInfo, &stGopAutoDetectInfo, u32CopiedLength);
         }
    }
#endif
    return GOP_API_SUCCESS;
}


/********************************************************************************/
/// Get config by GOP.
/// @param u8GOP \b IN:  GOP number
/// @param pstInfo \b IN misc info
/// @return GOP_API_SUCCESS - Success
/// @return GOP_API_FAIL - Failure
/********************************************************************************/
E_GOP_API_Result MApi_GOP_GetConfigEx(MS_U8 u8Gop, EN_GOP_CONFIG_TYPE enType, void *plist)
{
#ifdef CONFIG_GOP_UTOPIA10
    switch(enType)
    {
        case E_GOP_IGNOREINIT:
        {
            if(Ioctl_GOP_GetConfig(NULL,enType,plist,sizeof(EN_GOP_IGNOREINIT)) != GOP_API_SUCCESS)
            {
                GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
                GOP_RETURN(GOP_API_FAIL);
            }
            break;
        }
        case E_GOP_CONSALPHA_BITS:
        {
            if(Ioctl_GOP_GetConfig(NULL,enType,plist,sizeof(EN_GOP_IGNOREINIT)) != GOP_API_SUCCESS)
            {
                GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
                GOP_RETURN(GOP_API_FAIL);
            }
            break;
        }
        case E_GOP_TIMING_INFO:
        {
            if(Ioctl_GOP_GetProperty(NULL,E_GOP_OUTPUT_TIMING,u8Gop,plist,sizeof(ST_GOP_TIMING_INFO)) != GOP_API_SUCCESS)
            {
                GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
                GOP_RETURN(GOP_API_FAIL);
            }
            break;
        }
        case E_GOP_IS_PIXELBASE_ENABLE:
        {
            if(Ioctl_GOP_GetProperty(NULL,E_GOP_OUTPUT_TIMING,u8Gop,plist,sizeof(ST_GOP_TIMING_INFO)) != GOP_API_SUCCESS)
            {
                GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
                GOP_RETURN(GOP_API_FAIL);
            }
            break;
        }
        case E_GOP_GET_MAXFBNUM:
        {
            if(Ioctl_GOP_GetProperty(NULL,E_GOP_MAXFBNUM,u8Gop,plist,sizeof(MS_U32)) != GOP_API_SUCCESS)
            {
                GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
                GOP_RETURN(GOP_API_FAIL);
            }
            break;
        }
        case E_GOP_IS_INIT:
        {
            if(Ioctl_GOP_GetProperty(NULL,E_GOP_OUTPUT_TIMING,u8Gop,plist,sizeof(ST_GOP_TIMING_INFO)) != GOP_API_SUCCESS)
            {
                GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
                GOP_RETURN(GOP_API_FAIL);
            }
            break;
        }
        case E_GOP_GET_STRETCH_INFO:
        {
            if(Ioctl_GOP_Get_Stretch(NULL,E_GOP_STRETCH_WIN,u8Gop,plist) != GOP_API_SUCCESS)
            {
                GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
                GOP_RETURN(GOP_API_FAIL);
            }
            break;
        }
        default:
            return GOP_API_INVALID_PARAMETERS;
            break;
    }
// Utopia 2.0
#else
    E_GOP_API_CMD_TYPE enApiCmdType;
    GOP_SETCONFIG_PARAM ioctl_ConfigInfo;
    GOP_SET_PROPERTY_PARAM ioctl_PropInfo;
    void *pInfo = NULL;

    CheckGOPInstanceOpen();

    switch(enType)
    {
        case E_GOP_IGNOREINIT:
        {
            ioctl_ConfigInfo.cfg_type = enType;
            ioctl_ConfigInfo.pCfg = plist;
            ioctl_ConfigInfo.u32Size = sizeof(EN_GOP_IGNOREINIT);
            pInfo = (void*)&ioctl_ConfigInfo;
            enApiCmdType = MAPI_CMD_GOP_GET_CONFIG;
            break;
        }
        case E_GOP_CONSALPHA_BITS:
        {
            ioctl_ConfigInfo.cfg_type = enType;
            ioctl_ConfigInfo.pCfg = plist;
            ioctl_ConfigInfo.u32Size = sizeof(EN_GOP_CONSALPHA_BITS);
            pInfo = (void*)&ioctl_ConfigInfo;
            enApiCmdType = MAPI_CMD_GOP_GET_CONFIG;
            break;
        }
        case E_GOP_TIMING_INFO:
        {
            ioctl_PropInfo.en_pro = E_GOP_OUTPUT_TIMING;
            ioctl_PropInfo.gop_idx = u8Gop;
            ioctl_PropInfo.pSetting = plist;
            ioctl_PropInfo.u32Size = sizeof(ST_GOP_TIMING_INFO);
            pInfo = (void*)&ioctl_PropInfo;
            enApiCmdType = MAPI_CMD_GOP_GET_PROPERTY;
            break;
        }
        case E_GOP_IS_PIXELBASE_ENABLE:
        {
            ioctl_PropInfo.en_pro = E_GOP_PIXELBASE_ENABLE;
            ioctl_PropInfo.gop_idx = u8Gop;
            ioctl_PropInfo.pSetting = plist;
            ioctl_PropInfo.u32Size = sizeof(MS_BOOL);
            pInfo = (void*)&ioctl_PropInfo;
            enApiCmdType = MAPI_CMD_GOP_GET_PROPERTY;
            break;
        }
        case E_GOP_GET_MAXFBNUM:
        {
            ioctl_PropInfo.en_pro = E_GOP_MAXFBNUM;
            ioctl_PropInfo.gop_idx = u8Gop;
            ioctl_PropInfo.pSetting = plist;
            ioctl_PropInfo.u32Size = sizeof(MS_U32);
            pInfo = (void*)&ioctl_PropInfo;
            enApiCmdType = MAPI_CMD_GOP_GET_PROPERTY;
            break;
        }
        case E_GOP_IS_INIT:
        {
            ioctl_PropInfo.en_pro = E_GOP_NUM_IS_INIT;
            ioctl_PropInfo.gop_idx = u8Gop;
            ioctl_PropInfo.pSetting = plist;
            ioctl_PropInfo.u32Size = sizeof(GOP_InitInfo);
            pInfo = (void*)&ioctl_PropInfo;
            enApiCmdType = MAPI_CMD_GOP_GET_PROPERTY;
            break;
        }
        default:
            return GOP_API_INVALID_PARAMETERS;
            break;
    }
    if (UtopiaIoctl(pInstantGOP, enApiCmdType, pInfo) != UTOPIA_STATUS_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        return GOP_API_FAIL;
    }
#endif
    return GOP_API_SUCCESS;
}

//******************************************************************************
/// API for set GWIN resolution in one function
/// @param u8GwinId \b IN: GWin ID
/// @param u8FbId \b IN: Frame Buffer ID
/// @param pGwinInfo \b IN: pointer to GOP_GwinInfo structure
/// @param pStretchInfo \b IN: pointer to GOP_StretchInfo
/// @param direction \b IN: to decide which direction to stretch
/// @param u16DstWidth \b IN: set scaled width if H direction is specified
/// @param u16DstHeight \b IN: set scaled height if V direction is specified
/// @return GOP_API_SUCCESS - Success
//******************************************************************************
E_GOP_API_Result MApi_GOP_GWIN_SetResolution(MS_U8 u8GwinId, MS_U8 u8FbId
    , GOP_GwinInfo* pGwinInfo, GOP_StretchInfo* pStretchInfo, EN_GOP_STRETCH_DIRECTION direction
    , MS_U16 u16DstWidth, MS_U16 u16DstHeight)
{
    return MApi_GOP_GWIN_SetResolution_32FB(u8GwinId, (MS_U32)u8FbId, pGwinInfo, pStretchInfo, direction, u16DstWidth, u16DstHeight);
}

E_GOP_API_Result MApi_GOP_GWIN_SetResolution_32FB(MS_U8 u8GwinId, MS_U32 u32FbId
    , GOP_GwinInfo* pGwinInfo, GOP_StretchInfo* pStretchInfo, EN_GOP_STRETCH_DIRECTION direction
    , MS_U16 u16DstWidth, MS_U16 u16DstHeight)
{

    GOP_GWINDISPLAY_INFO DispInfo;
    GOP_ENTRY();
    memset(&DispInfo, 0x0, sizeof(GOP_GWINDISPLAY_INFO));

    DispInfo.dir = direction;
    DispInfo.gwin = u8GwinId;
    DispInfo.fbid = u32FbId;
    DispInfo.dst_size.w = u16DstWidth;
    DispInfo.dst_size.h = u16DstHeight;

    memcpy(&DispInfo.gwin_info, pGwinInfo, sizeof(GOP_GwinInfo));
    memcpy(&DispInfo.stretch_info, pStretchInfo, sizeof(GOP_StretchInfo));
#ifdef CONFIG_GOP_UTOPIA10
    if(Ioctl_GOP_SetDisplay(NULL,&DispInfo) != GOP_API_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN(GOP_API_FAIL);
    }
#else
    GOP_GWIN_DISPLAY_PARAM ioctl_info;
    memset(&ioctl_info, 0x0, sizeof(GOP_GWIN_DISPLAY_PARAM));

    ioctl_info.GwinId = u8GwinId;
    ioctl_info.u32Size = sizeof(GOP_GWINDISPLAY_INFO);
    ioctl_info.pDisplayInfo = (MS_U32*)&DispInfo;


    if(UtopiaIoctl(pInstantGOP,MAPI_CMD_GOP_GWIN_SETDISPLAY,&ioctl_info)!= UTOPIA_STATUS_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN( GOP_API_FAIL);
    }
#endif
    GOP_RETURN(GOP_API_SUCCESS);
}

//******************************************************************************
/// API for change L/R frame for GOP 3D function
/// @param u8GOP \b IN: GOP ID
/// @return GOP_API_SUCCESS - Success
//******************************************************************************
E_GOP_API_Result MApi_GOP_Set3D_LR_FrameExchange(MS_U8 u8GOP)
{
    MS_U32 value = 0;

    GOP_ENTRY();
#ifdef CONFIG_GOP_UTOPIA10
    if(Ioctl_GOP_SetProperty(NULL,E_GOP_3D_LR_EXCHANGE,u8GOP,(MS_U32*)&value,sizeof(int)) != GOP_API_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN(GOP_API_FAIL);
    }
#else

    GOP_SET_PROPERTY_PARAM ioctl_info;
    memset(&ioctl_info, 0x0, sizeof(GOP_SET_PROPERTY_PARAM));

    ioctl_info.en_pro   = E_GOP_3D_LR_EXCHANGE;
    ioctl_info.gop_idx  = u8GOP;
    ioctl_info.pSetting = (MS_U32*)&value;
    ioctl_info.u32Size  = sizeof(MS_U32);

    if(UtopiaIoctl(pInstantGOP,MAPI_CMD_GOP_SET_PROPERTY,&ioctl_info)!= UTOPIA_STATUS_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN( GOP_API_FAIL);
    }
#endif
    GOP_RETURN(GOP_API_SUCCESS);
}

//******************************************************************************
/// API for notify kernel about 3D gpio pin
/// @param u32GPIO3DPin \b IN: GPIO 3D pin
/// @return GOP_API_SUCCESS - Success
//******************************************************************************
E_GOP_API_Result MApi_GOP_Set_GPIO3DPin(MS_U32 u32GPIO3DPin)
{
    GOP_ERR("[%s] Not SUPPORT in utopia2\n",__FUNCTION__);

    return GOP_API_FAIL;
}

#endif
//******************************************************************************
/// API for Query GOP Capability
/// @param eCapType \b IN: Capability type
/// @param pRet     \b OUT: return value
/// @param ret_size \b IN: input structure size
/// @return GOP_API_SUCCESS - Success
//******************************************************************************
E_GOP_API_Result MApi_GOP_GetChipCaps(EN_GOP_CAPS eCapType, void* pRet, MS_U32 ret_size)
{
    CheckGOPInstanceOpen();

    if(eCapType == E_GOP_CAP_VERSION)
    {
        GOP_LIB_VER * pLibVer;

        if(ret_size != sizeof(GOP_LIB_VER))
        {
            GOP_ERR("[%s] [%d]ERROR, invalid input structure size :0x%tx on E_GOP_CAP_VERSION\n",__FUNCTION__,__LINE__,(ptrdiff_t)ret_size);
            return GOP_API_FAIL;
        }

        pLibVer = (GOP_LIB_VER *)pRet;
#ifdef MSOS_TYPE_LINUX_KERNEL
        char ver[] = MSIF_GOP_LIBVER;
        char blnum[] = MSIF_GOP_BUILDNUM;
        char cl[] = MSIF_GOP_CHANGELIST;

        pLibVer->GOP_LibVer = atoi(ver);
        pLibVer->GOP_BuildNum = atoi(blnum);
        pLibVer->GOP_ChangeList = atoi(cl);
#else
        pLibVer->GOP_LibVer = 0;
        pLibVer->GOP_BuildNum = 0;
        pLibVer->GOP_ChangeList = 0;
#endif

    }
    else if (eCapType == E_GOP_CAP_WORD_UNIT)
    {
		MS_U32 u32Align = 0x0;
#ifdef CONFIG_GOP_UTOPIA10
        if(Ioctl_GOP_GetCaps(NULL,eCapType,&u32Align,sizeof(MS_U32)) != GOP_API_SUCCESS)
        {
            GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
            GOP_RETURN(GOP_API_FAIL);
        }
#else
        GOP_GETCAPS_PARAM gopcaps;

        memset(&gopcaps, 0x0, sizeof(GOP_GETCAPS_PARAM));

        gopcaps.caps = eCapType;
        gopcaps.pInfo = &u32Align;
        gopcaps.u32Size = sizeof(u32Align);

        if(UtopiaIoctl(pInstantGOP,MAPI_CMD_GOP_GET_CHIPCAPS,&gopcaps)!= UTOPIA_STATUS_SUCCESS)
        {
            GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
            return GOP_API_FAIL;
        }
#endif

        if (ret_size == sizeof(MS_U8))
        {
            *((MS_U8 *)pRet) = (MS_U8)u32Align;
        }
        if (ret_size == sizeof(MS_U16))
        {
            *((MS_U16 *)pRet) = (MS_U16)u32Align;
        }
        else if (ret_size == sizeof(MS_U32))
        {
            *((MS_U32 *)pRet) = u32Align;
        }
    }
    else
    {
#ifdef CONFIG_GOP_UTOPIA10
        if(Ioctl_GOP_GetCaps(NULL,eCapType,pRet,ret_size) != GOP_API_SUCCESS)
        {
            GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
            GOP_RETURN(GOP_API_FAIL);
        }
#else
        GOP_GETCAPS_PARAM gopcaps;

        memset(&gopcaps, 0x0, sizeof(GOP_GETCAPS_PARAM));

        gopcaps.caps = eCapType;
        gopcaps.pInfo = pRet;
        gopcaps.u32Size = ret_size;

        if(UtopiaIoctl(pInstantGOP,MAPI_CMD_GOP_GET_CHIPCAPS,&gopcaps)!= UTOPIA_STATUS_SUCCESS)
        {
            GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
            return GOP_API_FAIL;
        }
#endif
    }
    return GOP_API_SUCCESS;
}

#ifndef CONFIG_GOP_REDUCE_CODESIZE_FOR_MBOOT

//******************************************************************************
/// API for Set Scroll rate for Gwin
/// @param winId \b IN: GWIN id
/// @param bEnable \b IN:
///   - # TRUE Enable Tile Mode for GWIN
///   - # FALSE Disable Tile Mode for GWIN
/// @return GOP_API_SUCCESS - Success
/// @return GOP_API_FAIL - Failure

//******************************************************************************
E_GOP_API_Result MApi_GOP_GWIN_SetScrollRate(MS_U8 u8rate)
{
    GOP_ENTRY();
	MS_U32 u32ScrollRate;

	u32ScrollRate = (MS_U32)u8rate;
#ifdef CONFIG_GOP_UTOPIA10
    if(Ioctl_GOP_Scroll(NULL,E_GOP_SCROLL_RATE, &u32ScrollRate,sizeof(MS_U8)) != GOP_API_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN(GOP_API_FAIL);
    }
#else
    GOP_SCROLL_PARAM ioctl_info;
    memset(&ioctl_info, 0x0, sizeof(GOP_SCROLL_PARAM));

    ioctl_info.scroll_type = E_GOP_SCROLL_RATE;
    ioctl_info.pCfg = (void*)&u32ScrollRate;
    ioctl_info.u32Size  = sizeof(MS_U8);

    if(UtopiaIoctl(pInstantGOP,MAPI_CMD_GOP_SET_SCROLL,&ioctl_info)!= UTOPIA_STATUS_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN( GOP_API_FAIL);
    }
#endif
    GOP_RETURN(GOP_API_SUCCESS);

}

E_GOP_API_Result MApi_GOP_GWIN_SetScrollType(MS_U8 u8WinId, GOP_SCROLL_TYPE type,  MS_U16 u16Offset, MS_BOOL bEnable )
{
    GOP_ENTRY();
    GOP_SCROLL_INFO scroll_info;
    memset(&scroll_info, 0x0, sizeof(GOP_SCROLL_INFO));

    scroll_info.u8WinId = u8WinId;
    scroll_info.type = type;
    scroll_info.u16Offset = u16Offset;
    scroll_info.bEnable = bEnable;
#ifdef CONFIG_GOP_UTOPIA10
    if(Ioctl_GOP_Scroll(NULL,E_GOP_SCROLL_CONFIG,(MS_U32*)&scroll_info,sizeof(GOP_SCROLL_INFO)) != GOP_API_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN(GOP_API_FAIL);
    }
#else
    GOP_SCROLL_PARAM ioctl_info;
    memset(&ioctl_info, 0x0, sizeof(GOP_SCROLL_PARAM));

    ioctl_info.scroll_type = E_GOP_SCROLL_CONFIG;
    ioctl_info.pCfg = (MS_U32*)&scroll_info;
    ioctl_info.u32Size  = sizeof(GOP_SCROLL_INFO);

    if(UtopiaIoctl(pInstantGOP,MAPI_CMD_GOP_SET_SCROLL,&ioctl_info)!= UTOPIA_STATUS_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN( GOP_API_FAIL);
    }
#endif

    GOP_RETURN(GOP_API_SUCCESS);
}


E_GOP_API_Result MApi_GOP_GWIN_SetSWScrollBuffer(MS_U16 bufNum, MS_U32* pBuffer)
{
    GOP_ENTRY();
    GOP_ERR("[%s] Not SUPPORT in utopia2\n",__FUNCTION__);
    GOP_RETURN(GOP_API_SUCCESS);


}

/******************************************************************************/
/// Map FBinfo to OCinfo
/// @param u32Addr\b IN Bank Start Addr
/// @param BankMode\b IN Bank Mode
/// @param enBufSize\b IN Bank Sizee
/******************************************************************************/
E_GOP_API_Result MApi_GOP_OC_SetOCFBinfo(MS_PHY phyAddr, EN_GOP_OC_SEL_TYPE BankMode, MS_U32 enBufSize)
{
    GOP_OC_FB_INFO stOCFbInfo;
    memset(&stOCFbInfo, 0x0, sizeof(GOP_OC_FB_INFO));

    GOP_ENTRY();

    stOCFbInfo.u64Addr = phyAddr;
    stOCFbInfo.BankMode = BankMode;
    stOCFbInfo.enBufSize = enBufSize;
#ifdef CONFIG_GOP_UTOPIA10
    if(Ioctl_GOP_MISC(NULL,E_GOP_MISC_OC_FB_INFO,(MS_U32*)&stOCFbInfo,sizeof(GOP_OC_FB_INFO)) != GOP_API_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN(GOP_API_FAIL);
    }
#else
    GOP_MISC_PARAM ioctl_info;
    memset(&ioctl_info, 0x0, sizeof(GOP_MISC_PARAM));

    ioctl_info.misc_type = E_GOP_MISC_OC_FB_INFO;
    ioctl_info.pMISC = (MS_U32*)&stOCFbInfo;
    ioctl_info.u32Size  = sizeof(GOP_OC_FB_INFO);

    if(UtopiaIoctl(pInstantGOP,MAPI_CMD_GOP_MISC,&ioctl_info)!= UTOPIA_STATUS_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN( GOP_API_FAIL);
    }
#endif

    GOP_RETURN(GOP_API_SUCCESS);
}

E_GOP_API_Result MApi_GOP_IsRegUpdated(MS_U8 u8GopType)
{
	MS_U32 u32Update = FALSE;

    GOP_ENTRY();
#ifdef CONFIG_GOP_UTOPIA10
    if(Ioctl_GOP_GetProperty(NULL,E_GOP_REG_UPDATED,u8GopType,&u32Update,sizeof(MS_BOOL)) != GOP_API_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN(GOP_API_FAIL);
    }
#else
    GOP_SET_PROPERTY_PARAM ioctl_info;
    memset(&ioctl_info, 0x0, sizeof(GOP_SET_PROPERTY_PARAM));

    ioctl_info.en_pro   = E_GOP_REG_UPDATED;
    ioctl_info.gop_idx  = u8GopType;
    ioctl_info.pSetting = (void*)&u32Update;
    ioctl_info.u32Size  = sizeof(MS_BOOL);

    if(UtopiaIoctl(pInstantGOP,MAPI_CMD_GOP_GET_PROPERTY,&ioctl_info)!= UTOPIA_STATUS_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN((E_GOP_API_Result)u32Update);
    }
#endif
    GOP_RETURN((E_GOP_API_Result)u32Update);

}

/******************************************************************************/
/// set GOP power state
/// @param enPowerState\b IN power status
/******************************************************************************/
E_GOP_API_Result MApi_GOP_SetPowerState(EN_POWER_MODE enPowerState)
{

    CheckGOPInstanceOpen();
    GOP_ENTRY();
    _GOP_RegisterAllCBFunc();

    GOP_POWERSTATE_PARAM PowerState;
    memset(&PowerState,0x0,sizeof(GOP_POWERSTATE_PARAM));

    PowerState.pInfo = &enPowerState;
    PowerState.u32Size = sizeof(GOP_POWERSTATE_PARAM);
#ifdef CONFIG_GOP_UTOPIA10
    if(Ioctl_GOP_PowerState(NULL,enPowerState,NULL) != GOP_API_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN(GOP_API_FAIL);
    }
#else
    if(UtopiaIoctl(pInstantGOP,MAPI_CMD_GOP_POWERSTATE,&PowerState)!= UTOPIA_STATUS_SUCCESS)
            {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN(GOP_API_FAIL);
    }
#endif
    GOP_RETURN(GOP_API_SUCCESS);
}

E_GOP_API_Result MApi_GOP_GWIN_BeginDraw(void)
{
    GOP_ENTRY();
#ifdef CONFIG_GOP_UTOPIA10
    GOP_RETURN(GOP_API_SUCCESS);
#else
    GOP_MUTEX_PARAM stMutexPara;

    memset(&stMutexPara,0x0,sizeof(GOP_MUTEX_PARAM));
    stMutexPara.en_mutex = E_GOP_LOCK;
    stMutexPara.u32Size = sizeof(GOP_MUTEX_PARAM);

    if(UtopiaIoctl(pInstantGOP,MAPI_CMD_GOP_MUTEX,&stMutexPara)!= UTOPIA_STATUS_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN(GOP_API_FAIL);
    }
    GOP_RETURN(GOP_API_SUCCESS);
#endif

}

E_GOP_API_Result MApi_GOP_GWIN_EndDraw(void)
{
    GOP_ENTRY();
#ifdef CONFIG_GOP_UTOPIA10
GOP_RETURN(GOP_API_SUCCESS);
#else
    GOP_MUTEX_PARAM stMutexPara;

    memset(&stMutexPara,0x0,sizeof(GOP_MUTEX_PARAM));
    stMutexPara.en_mutex = E_GOP_UNLOCK;
    stMutexPara.u32Size = sizeof(GOP_MUTEX_PARAM);

    if(UtopiaIoctl(pInstantGOP,MAPI_CMD_GOP_MUTEX,&stMutexPara)!= UTOPIA_STATUS_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN(GOP_API_FAIL);
    }
    GOP_RETURN(GOP_API_SUCCESS);
#endif

}

/******************************************************************************/
/// API for GOP delete upexpect H/V win size for output
/// @param MS_U8 u8GOP\b IN: GOP ID
/// @param MS_U32 u32ARGB  \b IN: ARGB Color
/// @return GOP_API_SUCCESS - Success
/// @return GOP_API_FAIL - Failure
/******************************************************************************/
E_GOP_API_Result MApi_GOP_GWIN_DeleteWinHVSize(MS_U8 u8GOP, MS_U16 u16HSize, MS_U16 u16VSize)
{
    GOP_DeleteWinSize_PARAM GOP_VaildSize;

    GOP_ENTRY();
    memset(&GOP_VaildSize,0x0,sizeof(GOP_DeleteWinSize_PARAM));

    GOP_VaildSize.u32GOP_idx = u8GOP;
    GOP_VaildSize.u32HSize = (MS_U32)u16HSize;
    GOP_VaildSize.u32VSize = (MS_U32)u16VSize;
#ifdef CONFIG_GOP_UTOPIA10
    if(Ioctl_GOP_MISC(NULL,E_GOP_MISC_DELETE_GWINSIZE,(MS_U32*)&GOP_VaildSize,sizeof(GOP_DeleteWinSize_PARAM)) != GOP_API_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN(GOP_API_FAIL);
    }
#else
    GOP_MISC_PARAM ioctl_info;
    memset(&ioctl_info,0x0,sizeof(GOP_MISC_PARAM));

    ioctl_info.misc_type = E_GOP_MISC_DELETE_GWINSIZE;
    ioctl_info.pMISC = (MS_U32*)&GOP_VaildSize;
    ioctl_info.u32Size  = sizeof(GOP_DeleteWinSize_PARAM);

    if(UtopiaIoctl(pInstantGOP,MAPI_CMD_GOP_MISC,&ioctl_info)!= UTOPIA_STATUS_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN( GOP_API_FAIL);
    }
#endif
    GOP_RETURN(GOP_API_SUCCESS);
}

E_GOP_API_Result MApi_GOP_SetGOPCscTuning(MS_U32 u32GOPNum,ST_GOP_CSC_PARAM *pstGOPCscParam)
{
    GOP_SET_PROPERTY_PARAM stIoctlInfo;
    MS_U32 u32CopiedLength = 0;
    ST_GOP_CSC_PARAM stCopiedCSCParam;
    MS_U32 u32Ret = 0;

    GOP_ENTRY();

    memset(&stCopiedCSCParam, 0, sizeof(ST_GOP_CSC_PARAM));

    if(pstGOPCscParam == NULL)
    {
        GOP_ERR("[%s][%d] pstGOPCscParam is NULL\n",__func__,__LINE__);
        return GOP_API_FAIL;
    }

    GOP_CHK_VERSION(pstGOPCscParam->u32Version, pstGOPCscParam->u32Length, ST_GOP_CSC_PARAM_VERSION, sizeof(ST_GOP_CSC_PARAM), u32CopiedLength);

    if(u32CopiedLength == 0)
    {
        return GOP_API_FAIL;
    }

    memcpy(&stCopiedCSCParam, pstGOPCscParam, u32CopiedLength);


#ifdef CONFIG_GOP_UTOPIA10
    u32Ret = Ioctl_GOP_SetProperty(NULL,E_GOP_SET_CSC_TUNING,u32GOPNum,(void*)&stCopiedCSCParam,u32CopiedLength);
    if(u32Ret != GOP_API_SUCCESS)
    {
        if((E_GOP_API_Result)u32Ret == GOP_API_FUN_NOT_SUPPORTED)
        {
            GOP_ERR("[%s]Function not support\n",__FUNCTION__);
            GOP_RETURN(GOP_API_FUN_NOT_SUPPORTED);
        }
        else
        {
            GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
            GOP_RETURN(GOP_API_FAIL);
        }
    }
    UNUSED(stIoctlInfo);
#else
    memset(&stIoctlInfo, 0x0, sizeof(GOP_SET_PROPERTY_PARAM));

    stIoctlInfo.en_pro   = E_GOP_SET_CSC_TUNING;
    stIoctlInfo.gop_idx  = u32GOPNum;
    stIoctlInfo.pSetting = (void*)&stCopiedCSCParam;
    stIoctlInfo.u32Size  = u32CopiedLength;

    u32Ret = UtopiaIoctl(pInstantGOP,MAPI_CMD_GOP_SET_PROPERTY,&stIoctlInfo);

    if(u32Ret != UTOPIA_STATUS_SUCCESS)
    {
        if(u32Ret == UTOPIA_STATUS_NOT_SUPPORTED)
        {
            GOP_ERR("[%s]Function not support\n",__FUNCTION__);
            GOP_RETURN(GOP_API_FUN_NOT_SUPPORTED);
        }
        else
        {
            GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
            GOP_RETURN( GOP_API_FAIL);
        }
    }
#endif
    GOP_RETURN(GOP_API_SUCCESS);
}

E_GOP_API_Result MApi_GOP_GetGOPCscCtrlInfo(MS_U32 u32GOPNum,ST_GOP_CSC_TUNING_INFO *pstCtrlInfo)
{
    GOP_SET_PROPERTY_PARAM stIoctlInfo;
    MS_U32 u32CopiedLength = 0;
    ST_GOP_CSC_TUNING_INFO stCopiedTuningInfo;
    MS_U32 u32Ret = 0;
    GOP_ENTRY();

    memset(&stCopiedTuningInfo, 0, sizeof(ST_GOP_CSC_TUNING_INFO));

    if(pstCtrlInfo == NULL)
    {
        GOP_ERR("[%s][%d] pstTuningInfo is NULL\n",__func__,__LINE__);
        return GOP_API_FAIL;
    }

    GOP_CHK_VERSION(pstCtrlInfo->u32Version, pstCtrlInfo->u32Length, ST_GOP_CSC_TUNING_INFO_VERSION, sizeof(ST_GOP_CSC_TUNING_INFO), u32CopiedLength);

    if(u32CopiedLength == 0)
    {
        return GOP_API_FAIL;
    }

    memcpy(&stCopiedTuningInfo, pstCtrlInfo, u32CopiedLength);


#ifdef CONFIG_GOP_UTOPIA10
    u32Ret = Ioctl_GOP_GetProperty(NULL,E_GOP_GET_CSC_CTRL_INFO,u32GOPNum,(void*)&stCopiedTuningInfo,u32CopiedLength);
    if(u32Ret != GOP_API_SUCCESS)
    {
        if((E_GOP_API_Result)u32Ret == GOP_API_FUN_NOT_SUPPORTED)
        {
            GOP_ERR("[%s]Function not support\n",__FUNCTION__);
            GOP_RETURN(GOP_API_FUN_NOT_SUPPORTED);
        }
        else
        {
            GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
            GOP_RETURN(GOP_API_FAIL);
        }
    }
    memcpy(pstCtrlInfo, &stCopiedTuningInfo, u32CopiedLength);
    UNUSED(stIoctlInfo);
#else

    memset(&stIoctlInfo, 0x0, sizeof(GOP_SET_PROPERTY_PARAM));

    stIoctlInfo.en_pro   = E_GOP_GET_CSC_CTRL_INFO;
    stIoctlInfo.gop_idx  = u32GOPNum;
    stIoctlInfo.pSetting = (void*)&stCopiedTuningInfo;
    stIoctlInfo.u32Size  = u32CopiedLength;

    u32Ret = UtopiaIoctl(pInstantGOP,MAPI_CMD_GOP_GET_PROPERTY,&stIoctlInfo);
    if(u32Ret != UTOPIA_STATUS_SUCCESS)
    {
        if(u32Ret == UTOPIA_STATUS_NOT_SUPPORTED)
        {
            GOP_ERR("[%s]Function not support\n",__FUNCTION__);
            GOP_RETURN(GOP_API_FUN_NOT_SUPPORTED);
        }
        else
        {
            GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
            GOP_RETURN( GOP_API_FAIL);
        }
    }
    memcpy(pstCtrlInfo, &stCopiedTuningInfo, u32CopiedLength);
#endif
    GOP_RETURN(GOP_API_SUCCESS);
}

E_GOP_API_Result MApi_GOP_SetGOPCscCtrl (MS_U32 u32GOPNum,ST_GOP_CSC_TABLE *pstCSCTbl)
{
    GOP_SET_PROPERTY_PARAM stIoctlInfo;
    MS_U32 u32CopiedLength = 0;
    ST_GOP_CSC_TABLE stCopiedCSCTbl;
    MS_U32 u32Ret = 0;

    GOP_ENTRY();

    memset(&stCopiedCSCTbl, 0, sizeof(ST_GOP_CSC_TABLE));

    if(pstCSCTbl == NULL)
    {
        GOP_ERR("[%s][%d] pstCSCTbl is NULL\n",__func__,__LINE__);
        return GOP_API_FAIL;
    }

    GOP_CHK_VERSION(pstCSCTbl->u32Version, pstCSCTbl->u32Length, ST_GOP_CSC_TABLE_VERSION, sizeof(ST_GOP_CSC_TABLE), u32CopiedLength);

    if(u32CopiedLength == 0)
    {
        return GOP_API_FAIL;
    }

    memcpy(&stCopiedCSCTbl, pstCSCTbl, u32CopiedLength);

#ifdef CONFIG_GOP_UTOPIA10
    u32Ret = Ioctl_GOP_SetProperty(NULL,E_GOP_SET_CSC_CTRL,u32GOPNum,(void*)&stCopiedCSCTbl,u32CopiedLength);
    if(u32Ret != GOP_API_SUCCESS)
    {
        if((E_GOP_API_Result)u32Ret == GOP_API_FUN_NOT_SUPPORTED)
        {
            GOP_ERR("[%s]Function not support\n",__FUNCTION__);
            GOP_RETURN(GOP_API_FUN_NOT_SUPPORTED);
        }
        else
        {
            GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
            GOP_RETURN(GOP_API_FAIL);
        }
    }
    UNUSED(stIoctlInfo);
#else

    memset(&stIoctlInfo, 0x0, sizeof(GOP_SET_PROPERTY_PARAM));

    stIoctlInfo.en_pro   = E_GOP_SET_CSC_CTRL;
    stIoctlInfo.gop_idx  = u32GOPNum;
    stIoctlInfo.pSetting = (void*)&stCopiedCSCTbl;
    stIoctlInfo.u32Size  = u32CopiedLength;

    u32Ret = UtopiaIoctl(pInstantGOP,MAPI_CMD_GOP_SET_PROPERTY,&stIoctlInfo);
    if(u32Ret != UTOPIA_STATUS_SUCCESS)
    {
        if(u32Ret == UTOPIA_STATUS_NOT_SUPPORTED)
        {
            GOP_ERR("[%s]Function not support\n",__FUNCTION__);
            GOP_RETURN(GOP_API_FUN_NOT_SUPPORTED);
        }
        else
        {
            GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
            GOP_RETURN( GOP_API_FAIL);
        }
    }
#endif
    GOP_RETURN(GOP_API_SUCCESS);
}

E_GOP_API_Result MApi_GOP_GetOsdNonTransCnt(MS_U32* pu32Count)
{
    GOP_ENTRY();
#ifdef CONFIG_GOP_UTOPIA10
    if(Ioctl_GOP_MISC(NULL,E_GOP_MISC_GET_OSD_NONTRANS_CNT,(MS_U32*)pu32Count,sizeof(MS_U32)) != GOP_API_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN(GOP_API_FAIL);
    }
#else
    GOP_MISC_PARAM ioctl_info;
    memset(&ioctl_info, 0x0, sizeof(GOP_MISC_PARAM));

    ioctl_info.misc_type = E_GOP_MISC_GET_OSD_NONTRANS_CNT;
    ioctl_info.pMISC = (MS_U32*)pu32Count;
    ioctl_info.u32Size  = sizeof(MS_U32);

    if(UtopiaIoctl(pInstantGOP,MAPI_CMD_GOP_MISC,&ioctl_info)!= UTOPIA_STATUS_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN( GOP_API_FAIL);
    }
#endif
    GOP_RETURN(GOP_API_SUCCESS);

}

E_GOP_API_Result MApi_GOP_SetPixelShiftPD(MS_S32 s32Offset)
{
    MS_U32 u32GOP = 0;
    GOP_ENTRY();

#ifdef CONFIG_GOP_UTOPIA10
    if(Ioctl_GOP_SetProperty(NULL,E_GOP_PIXELSHIFT_PD,u32GOP, (void*)&s32Offset, sizeof(MS_S32)) != GOP_API_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN(GOP_API_FAIL);
    }
#else
    GOP_SET_PROPERTY_PARAM ioctl_info;
    memset(&ioctl_info, 0x0, sizeof(GOP_SET_PROPERTY_PARAM));

    ioctl_info.en_pro   = E_GOP_PIXELSHIFT_PD;
    ioctl_info.gop_idx  = u32GOP;
    ioctl_info.pSetting = (void*)&s32Offset;
    ioctl_info.u32Size  = sizeof(MS_S32);

    if(UtopiaIoctl(pInstantGOP,MAPI_CMD_GOP_SET_PROPERTY,&ioctl_info)!= UTOPIA_STATUS_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN( GOP_API_FAIL);
    }
#endif
    GOP_RETURN(GOP_API_SUCCESS);
}

E_GOP_API_Result MApi_GOP_SetByPassMode(MS_U8 u8GOPIdx, MS_BOOL bEnable)
{
	MS_U32 u32Ena;

	u32Ena = (MS_U32)bEnable;
    GOP_ENTRY();
#ifdef CONFIG_GOP_UTOPIA10
    if(Ioctl_GOP_SetProperty(NULL,E_GOP_BYPASS_MODE, (MS_U32)u8GOPIdx, &u32Ena, sizeof(MS_BOOL)) != GOP_API_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN(GOP_API_FAIL);
    }
#else
    GOP_SET_PROPERTY_PARAM ioctl_info;
    memset(&ioctl_info, 0x0, sizeof(GOP_SET_PROPERTY_PARAM));

    ioctl_info.en_pro   = E_GOP_BYPASS_MODE;
    ioctl_info.gop_idx  = u32GOPNum;
    ioctl_info.pSetting = (void*)&u32Ena;
    ioctl_info.u32Size  = sizeof(MS_BOOL);

    if(UtopiaIoctl(pInstantGOP,MAPI_CMD_GOP_SET_PROPERTY,&ioctl_info)!= UTOPIA_STATUS_SUCCESS)
    {
        GOP_ERR("Ioctl %s fail\n",__FUNCTION__);
        GOP_RETURN(GOP_API_FAIL);
    }
#endif
    GOP_RETURN(GOP_API_SUCCESS);
}


#undef MApi_GOP_C
#endif
#endif
