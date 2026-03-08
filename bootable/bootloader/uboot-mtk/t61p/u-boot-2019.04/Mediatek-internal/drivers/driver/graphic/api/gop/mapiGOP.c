// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

//-------------------------------------------------------------------------------------------------
//  Include Files
//-------------------------------------------------------------------------------------------------
#ifndef MSOS_TYPE_LINUX_KERNEL
#include <stdio.h>
#include <string.h>
//#include <sys/types.h>
//#include <signal.h>
#include <errno.h>
#else
#include <linux/slab.h>
#endif
#include "utopia.h"
//#include "utopia_dapi.h"
#include "util_symbol.h"
#include "MsTypes.h"
//#include "MsOS.h"
#include "MsCommon.h"
//#include "MsVersion.h"
#include "apiGOP.h"
#include "apiGOP_priv.h"
#include "drvGOP.h"
#include "drvGFLIP.h"
#include "drvGOP_priv.h"
#include "halGOP.h"
//#include "halCHIP.h"
//#include "apiXC.h"

enum
{
    GOP_POOL_ID_GOP0=0
} eGOPPoolID;
//-------------------------------------------------------------------------------------------------
//  Local Compiler Options
//-------------------------------------------------------------------------------------------------
#define DUMP_INFO   0

#define FPGA_TEST   0UL

//-------------------------------------------------------------------------------------------------
//  Local Defines
//-------------------------------------------------------------------------------------------------
#ifdef MSOS_TYPE_LINUX_KERNEL
#define atoi(str) simple_strtoul(((str != NULL) ? str : ""), NULL, 0)
#endif

#define GetMaxActiveGwinFalse               4UL
#define GetMaxActiveGwinFalse_op            5UL
#define GetMaxActiveGwinFalse_opened        6UL
#define GWIN_SDRAM_NULL                     0x30UL
#define msWarning(c)                        do {} while (0)
#define XC_MAIN_WINDOW                      0UL


// Define return values of check align
#define CHECKALIGN_SUCCESS                  1UL
#define CHECKALIGN_FORMAT_FAIL              2UL
#define CHECKALIGN_PARA_FAIL                3UL

//#define GWIN_SDRAM_PG_UNIT 0x00000010  // 16-byte page-unit for gwin_sdram
#define GWIN_SDRAM_PG_ALIGN(_x)    ALIGN_32(_x)
#define IS_GWIN_SDRAM_ALIGN(_x)  ((_x)== GWIN_SDRAM_PG_ALIGN(_x))

static MS_BOOL bInitBkFWR[SHARED_GOP_MAX_COUNT]={[0 ... (SHARED_GOP_MAX_COUNT-1)] = FALSE};

static MS_BOOL bFirstInit = FALSE;

#ifdef INSTANT_PRIVATE
#define g_pGOPCtxLocal  psGOPInstPri->g_pGOPCtxLocal
#else
MS_GOP_CTX_LOCAL *g_pGOPCtxLocal = NULL;
#endif
static GOP_TYPE_DEF GOPTYPE;

#define PAR_IGNORE    (0xFFUL)

#ifdef MSOS_TYPE_LINUX_KERNEL
static MS_U8 _devGFLIPCnt =0;
#endif

//#include "ULog.h"
#include <debug_impl.h>
MS_U32 u32GOPDbgLevel_mapi = 0; //set default value to 0

#ifdef CONFIG_GOP_DEBUG_LEVEL
// Debug Logs, level form low(INFO) to high(FATAL, always show)
// Function information, ex function entry
#define GOP_M_INFO(x, args...) if (u32GOPDbgLevel_mapi >= E_GOP_Debug_Level_HIGH) { UBOOT_INFO(x, ##args); }
// Warning, illegal paramter but can be self fixed in functions
#define GOP_M_WARN(x, args...) if (u32GOPDbgLevel_mapi >= E_GOP_Debug_Level_HIGH) { UBOOT_INFO(x, ##args); }
//  Need debug, illegal paramter.
#define GOP_M_DBUG(x, args...) if (u32GOPDbgLevel_mapi >= E_GOP_Debug_Level_MED) { UBOOT_DEBUG(x, ##args); }
// Error, function will be terminated but system not crash
#define GOP_M_ERR(x, args...) { UBOOT_ERROR(x, ##args); }
// Critical, system crash. (ex. assert)
#define GOP_M_FATAL(x, args...) { UBOOT_ERROR(x, ##args); }
#else
#define GOP_M_INFO(x, args...)
// Warning, illegal paramter but can be self fixed in functions
#define GOP_M_WARN(x, args...)
//  Need debug, illegal paramter.
#define GOP_M_DBUG(x, args...)
// Error, function will be terminated but system not crash
#define GOP_M_ERR(x, args...)
// Critical, system crash. (ex. assert)
#define GOP_M_FATAL(x, args...)
#endif

#define APIGOP_RetFailPri(FailType) \
               do {\
                    switch ((E_GOP_API_Result)FailType)\
                    {\
                        case GOP_API_FAIL:\
                            GOP_M_DBUG("\nGOP_FAIL");\
                            break;\
                        case GOP_API_NON_ALIGN_ADDRESS:\
                            GOP_M_DBUG("\nGOP_NON_ALIGN_ADDRESS");\
                            break;\
                        case GOP_API_NON_ALIGN_PITCH:\
                            GOP_M_DBUG("\nGOP_NON_ALIGN_PITCH");\
                            break;\
                        case GOP_API_DEPEND_NOAVAIL:\
                            GOP_M_DBUG("\nGOP_DEPEND_NOAVAIL");\
                            break;\
                        case GOP_API_MUTEX_OBTAIN_FAIL:\
                            GOP_M_DBUG("\nGOP_MUTEX_OBTAIN_FAIL");\
                            break;\
                        case GOP_API_MUTEX_OBTAINED:\
                            GOP_M_DBUG("\nGOP_MUTEX_OBTAINED");\
                            break;\
                        case GOP_API_INVALID_BUFF_INFO:\
                            GOP_M_DBUG("\nGOP_INVALID_BUFF_INFO");\
                            break;\
                        case GOP_API_INVALID_PARAMETERS:\
                            GOP_M_DBUG("\nGOP_INVALID_PARAMETERS");\
                            break;\
                        case GOP_API_FUN_NOT_SUPPORTED:\
                            GOP_M_DBUG("\nGOP_FUN_NOT_SUPPORTED");\
                            break;\
                        case GOP_API_ENUM_NOT_SUPPORTED:\
                            GOP_M_DBUG("\nGOP_ENUM_NOT_SUPPORTED");\
                            break;\
                        case GOP_API_CRT_GWIN_FAIL:\
                            GOP_M_DBUG("\nGOP_CRT_GWIN_FAIL");\
                            break;\
                        case GOP_API_CRT_GWIN_NOAVAIL:\
                            GOP_M_DBUG("\nGOP_CRT_GWIN_NOAVAIL");\
                            break;\
                        case GOP_API_SUCCESS:\
                            break;\
                        default:\
                            break;\
                    }\
                }while(0)


#define APIGOP_CHECK_INIT() \
{                            \
    if(g_pGOPCtxLocal ==NULL) \
    {                        \
        APIGOP_ASSERT(FALSE, GOP_M_FATAL("\nshould call MApi_GOP_Init first\n")); \
        return GOP_API_FAIL;\
    }                        \
}

//-------------------------------------------------------------------------------------------------
//  Local Structures
//-------------------------------------------------------------------------------------------------
/// GOP HVMirror GWIN position option define
typedef enum
{
    E_Set_HPos,
    E_Set_VPos,
    E_Set_X,
    E_Set_Y,
}HMirror_Opt;

typedef MS_U32 (*FP_SetFBFmt)(MS_U16 ,MS_PHY , MS_U16 );
typedef MS_BOOL (*FP_XC_IsInterlace)(void);
typedef MS_U16 (*FP_XC_GetCapHStart)(void);
typedef void (*FP_XC_ReduceBWForOSD)(MS_U8 , MS_BOOL);
typedef void (*FP_EventNotify)(MS_U32 , void*);
#ifndef MSOS_TYPE_LINUX_KERNEL
typedef MS_BOOL (*FP_XC_SetDwinInfo)(MS_U16 cmd,MS_U32 *arg,MS_U16 size);
#endif

static FP_SetFBFmt fpSetFBFmt = NULL;
FP_XC_IsInterlace _fpXCIsInterlace = NULL;
FP_XC_GetCapHStart _fpXCGetCapHStart = NULL;
static FP_XC_ReduceBWForOSD fpXCReduceBWForOSD = NULL;
static FP_EventNotify fpEventNotify = NULL;
#ifndef MSOS_TYPE_LINUX_KERNEL
static FP_XC_SetDwinInfo fpXCSetDwinInfo = NULL;
#endif



#ifndef CONFIG_UTOPIAXP_REMOVE_IMPL
static MS_BOOL _GOP_IsFbIdValid(void* pInstance,MS_U32 u32FbId)
{
    if (u32FbId > DRV_MAX_GWIN_FB_SUPPORT)
        return FALSE;
    else
        return TRUE;
}


static GOP_WinFB_INFO* _GetWinFB(void* pInstance,MS_U32 FbId)
{
#ifdef INSTANT_PRIVATE
    GOP_INSTANT_PRIVATE* psGOPInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void*)&psGOPInstPri);
#endif

    if (!_GOP_IsFbIdValid(pInstance,FbId))
    {
        GOP_M_ERR("[%s][%d]FbId %td  is out of range\n",__FUNCTION__,__LINE__,(ptrdiff_t)FbId);
        return NULL;
    }
#if WINFB_INSHARED
        //memcpy(pWinFB,&(g_pGOPCtxLocal->pGOPCtxShared->winFB[FbId]),sizeof(GOP_WinFB_INFO));
    return &(g_pGOPCtxLocal->pGOPCtxShared->winFB[FbId]);
#else
        //memcpy(pWinFB,g_pGOPCtxLocal->winFB[FbId],sizeof(GOP_WinFB_INFO));
    return &g_pGOPCtxLocal->winFB[FbId];
#endif

}

#ifdef CONFIG_GOP_AFBC_FEATURE
static void _GOP_MAP_FBID_AFBCCmd(void* pInstance,MS_U32 u32fbId,MS_U8* u8AFBC_Cmd, MS_BOOL* AFBC_Enable)
{
#ifdef INSTANT_PRIVATE
    GOP_INSTANT_PRIVATE* psGOPInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void*)&psGOPInstPri);
#endif
    GOP_WinFB_INFO* pwinFB;

    pwinFB = _GetWinFB(pInstance,u32fbId);
    if(pwinFB == NULL)
    {
        GOP_M_ERR("[%s][%d]GetWinFB Fail : WrongFBID=%td\n",__FUNCTION__,__LINE__,(ptrdiff_t)u32fbId);
        return;
    }

    switch(pwinFB->FBString)
    {
        case E_DRV_GOP_FB_AFBC_SPLT_YUVTRNSFER_ARGB8888:
            *AFBC_Enable = TRUE;
            *u8AFBC_Cmd = (MS_U8)(E_DRV_GOP_AFBC_SPILT|E_DRV_GOP_AFBC_YUV_TRANSFER|E_DRV_GOP_AFBC_ARGB8888);
            break;
        case E_DRV_GOP_FB_AFBC_NONSPLT_YUVTRS_ARGB8888:
            *AFBC_Enable = TRUE;
            *u8AFBC_Cmd = (MS_U8)(E_DRV_GOP_AFBC_YUV_TRANSFER|E_DRV_GOP_AFBC_ARGB8888);
            break;
        case E_DRV_GOP_FB_AFBC_SPLT_NONYUVTRS_ARGB8888:
            *AFBC_Enable = TRUE;
            *u8AFBC_Cmd = (MS_U8)(E_DRV_GOP_AFBC_SPILT|E_DRV_GOP_AFBC_ARGB8888);
            break;
        case E_DRV_GOP_FB_AFBC_NONSPLT_NONYUVTRS_ARGB8888:
        case E_DRV_GOP_FB_AFBC_V1P2_ARGB8888:
            *AFBC_Enable = TRUE;
            *u8AFBC_Cmd = (MS_U8)E_DRV_GOP_AFBC_ARGB8888;
            break;
        case E_DRV_GOP_FB_AFBC_V1P2_RGB565:
            *AFBC_Enable = TRUE;
            *u8AFBC_Cmd = (MS_U8)E_DRV_GOP_AFBC_RGB565;
            break;
        case E_DRV_GOP_FB_AFBC_V1P2_RGB888:
            *AFBC_Enable = TRUE;
            *u8AFBC_Cmd = (MS_U8)E_DRV_GOP_AFBC_RGB888;
            break;
        default:
            *AFBC_Enable = FALSE;
            *u8AFBC_Cmd = (MS_U8)E_DRV_GOP_FB_NULL;
            break;
    }
}
#endif

//-------------------------------------------------------------------------------------------------
//  Debug Functions
//-------------------------------------------------------------------------------------------------

static MS_U8 _GOP_GetMaxGOPNum(void* pInstance)
{
#ifdef INSTANT_PRIVATE
    GOP_INSTANT_PRIVATE* psGOPInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void*)&psGOPInstPri);
#endif

    return MDrv_GOP_GetMaxGOPNum(g_pGOPCtxLocal);
}


static MS_BOOL _GOP_IsGopNumValid(void* pInstance,MS_U32 gop)
{
    if (gop>=_GOP_GetMaxGOPNum(pInstance))
        return FALSE;
    else
        return TRUE;
}
static MS_BOOL _GOP_IsGwinIdValid(void* pInstance,MS_U8 u8GwinID)
{
#ifdef INSTANT_PRIVATE
    GOP_INSTANT_PRIVATE* psGOPInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void*)&psGOPInstPri);
#endif

    if(u8GwinID>=SHARED_GWIN_MAX_COUNT)
        return FALSE;
    else if (u8GwinID>=g_pGOPCtxLocal->pGopChipProperty->TotalGwinNum)
        return FALSE;
    else
        return TRUE;
    }


static void _GOP_GetGop0WinInfo(void* pInstance,MS_U8 u8win, GOP_GwinInfo* pinfo)
{
    DRV_GOP_GWIN_INFO sGetInfo;
    DRV_GOPDstType Gop0Dst =E_DRV_GOP_DST_INVALID;
    GOP_WinFB_INFO* pwinFB;

#ifdef INSTANT_PRIVATE
    GOP_INSTANT_PRIVATE* psGOPInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void*)&psGOPInstPri);
#endif

    if (!_GOP_IsGwinIdValid(pInstance,u8win))
    {
        GOP_M_ERR("[%s][%d]GWIN %d  is out of range\n",__FUNCTION__,__LINE__,u8win);
        return;
    }


    pwinFB = _GetWinFB(pInstance,g_pGOPCtxLocal->pGOPCtxShared->gwinMap[u8win].u32CurFBId);

    if(pwinFB == NULL)
    {
        GOP_M_ERR("[%s][%d]GetWinFB Fail : gwinMap[%d].u32CurFBId=%td\n",__FUNCTION__,__LINE__,u8win,(ptrdiff_t)g_pGOPCtxLocal->pGOPCtxShared->gwinMap[u8win].u32CurFBId);
        return;
    }

    MDrv_GOP_GWIN_GetDstPlane(g_pGOPCtxLocal, 0, &Gop0Dst);
    if (Gop0Dst ==E_DRV_GOP_DST_INVALID)
    {
        APIGOP_ASSERT(FALSE, GOP_M_FATAL("\n Get GOP destination fail!\n"));
        return;
    }
    memset(&sGetInfo, 0, sizeof(DRV_GOP_GWIN_INFO));
    MDrv_GOP_GWIN_GetGwinInfo(g_pGOPCtxLocal, u8win, &sGetInfo);
    pinfo->u16RBlkHPixSize = sGetInfo.u16RBlkHPixSize;
    pinfo->u16RBlkVPixSize = sGetInfo.u16RBlkVPixSize;
    //2009.03.14 Fix the TTX,CC position error
    sGetInfo.u16WinX = pwinFB->s_x;
    sGetInfo.u16WinY = pwinFB->s_y;

    pinfo->clrType = (EN_GOP_COLOR_TYPE)sGetInfo.clrType;

    if (HAS_BIT(g_pGOPCtxLocal->u16GOP_VMirror_VPos, u8win))
    {
        pinfo->u16DispVPixelStart = g_pGOPCtxLocal->sMirrorInfo[u8win].u16NonMirrorVStr;
        pinfo->u16DispVPixelEnd= g_pGOPCtxLocal->sMirrorInfo[u8win].u16NonMirrorVEnd;

        if (g_pGOPCtxLocal->Gwin_V_Dup)
            pinfo->u16DispVPixelEnd = (pinfo->u16DispVPixelEnd + pinfo->u16DispVPixelStart) >> 1;        // pix
    }
    else
    {
        pinfo->u16DispVPixelStart = sGetInfo.u16DispVPixelStart;              // pix
        if (g_pGOPCtxLocal->Gwin_V_Dup)
        {
            if( ((Gop0Dst == E_DRV_GOP_DST_IP0) || (Gop0Dst == E_DRV_GOP_DST_IP1)))
                pinfo->u16DispVPixelEnd = (((sGetInfo.u16DispVPixelEnd - pinfo->u16DispVPixelStart) << 1) + pinfo->u16DispVPixelStart) >> 1;
            else if((Gop0Dst == E_DRV_GOP_DST_VE) || (Gop0Dst == E_DRV_GOP_DST_OP1))
                pinfo->u16DispVPixelEnd = (((sGetInfo.u16DispVPixelEnd - pinfo->u16DispVPixelStart) << 1) + pinfo->u16DispVPixelStart) >> 1;
            else
                pinfo->u16DispVPixelEnd = (sGetInfo.u16DispVPixelEnd + pinfo->u16DispVPixelStart) >> 1;        // pix
        }
        else
        {
            if( ((Gop0Dst == E_DRV_GOP_DST_IP0) || (Gop0Dst == E_DRV_GOP_DST_IP1)))
                pinfo->u16DispVPixelEnd = ((sGetInfo.u16DispVPixelEnd - pinfo->u16DispVPixelStart) << 1) + pinfo->u16DispVPixelStart;
            else if((Gop0Dst == E_DRV_GOP_DST_VE) || (Gop0Dst == E_DRV_GOP_DST_OP1))
                pinfo->u16DispVPixelEnd = ((sGetInfo.u16DispVPixelEnd - pinfo->u16DispVPixelStart) << 1) + pinfo->u16DispVPixelStart;
            else if (((Gop0Dst == E_DRV_GOP_DST_OP0) || (Gop0Dst == E_DRV_GOP_DST_FRC) || (Gop0Dst == E_DRV_GOP_DST_BYPASS)) && !MDrv_GOP_GWIN_IsProgressive(g_pGOPCtxLocal,0))
            {
                pinfo->u16DispVPixelEnd = ((sGetInfo.u16DispVPixelEnd - pinfo->u16DispVPixelStart) << 1) + pinfo->u16DispVPixelStart;
            }
            else
                pinfo->u16DispVPixelEnd = sGetInfo.u16DispVPixelEnd;            // pix
        }
    }

    if (HAS_BIT(g_pGOPCtxLocal->u16GOP_HMirror_HPos, u8win))
    {
        pinfo->u16DispHPixelStart = g_pGOPCtxLocal->sMirrorInfo[u8win].u16NonMirrorHStr;
        pinfo->u16DispHPixelEnd= g_pGOPCtxLocal->sMirrorInfo[u8win].u16NonMirrorHEnd;

        if (g_pGOPCtxLocal->Gwin_H_Dup)
            pinfo->u16DispHPixelEnd = (pinfo->u16DispHPixelEnd + pinfo->u16DispHPixelStart) >> 1;     // pix
    }
    else
    {
        pinfo->u16DispHPixelStart = sGetInfo.u16DispHPixelStart;               // pix
        pinfo->u16DispHPixelEnd = sGetInfo.u16DispHPixelEnd;                             // pix

        if (g_pGOPCtxLocal->Gwin_H_Dup)
            pinfo->u16DispHPixelEnd = (pinfo->u16DispHPixelEnd + pinfo->u16DispHPixelStart) >> 1;     // pix
    }

    pinfo->u16RBlkHRblkSize = sGetInfo.u16RBlkHRblkSize; //3
    pinfo->u32DRAMRBlkStart = sGetInfo.u64DRAMRBlkStart;
    pinfo->u16WinY = sGetInfo.u16WinY;  //Original Y pix of FB
    pinfo->u16WinX= sGetInfo.u16WinX;   //Original X pix of FB
}

static void _GOP_GetGop1WinInfo(void* pInstance,MS_U8 u8win, GOP_GwinInfo* pinfo)
{
    DRV_GOP_GWIN_INFO sGetInfo;
    DRV_GOPDstType Gop1Dst =E_DRV_GOP_DST_INVALID;

#ifdef INSTANT_PRIVATE
    GOP_INSTANT_PRIVATE* psGOPInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void*)&psGOPInstPri);
#endif

    if (!_GOP_IsGwinIdValid(pInstance,u8win))
    {
        GOP_M_ERR("[%s][%d]GWIN %d  is out of range\n",__FUNCTION__,__LINE__,u8win);
        return;
    }


    MDrv_GOP_GWIN_GetDstPlane(g_pGOPCtxLocal, 1, &Gop1Dst);
    if (Gop1Dst ==E_DRV_GOP_DST_INVALID)
    {
        APIGOP_ASSERT(FALSE, GOP_M_FATAL("\n Get GOP destination fail!\n"));
        return;
    }

    memset(&sGetInfo, 0, sizeof(DRV_GOP_GWIN_INFO));
    MDrv_GOP_GWIN_GetGwinInfo(g_pGOPCtxLocal, u8win, &sGetInfo);
    pinfo->u16RBlkHPixSize = sGetInfo.u16RBlkHPixSize;
    pinfo->u16RBlkVPixSize = sGetInfo.u16RBlkVPixSize;
    pinfo->clrType = (EN_GOP_COLOR_TYPE)sGetInfo.clrType;

    if (HAS_BIT(g_pGOPCtxLocal->u16GOP_VMirror_VPos, u8win))
    {
        pinfo->u16DispVPixelStart = g_pGOPCtxLocal->sMirrorInfo[u8win].u16NonMirrorVStr;
        pinfo->u16DispVPixelEnd = g_pGOPCtxLocal->sMirrorInfo[u8win].u16NonMirrorVEnd;

        if (g_pGOPCtxLocal->Gwin_V_Dup)
            pinfo->u16DispVPixelEnd = (pinfo->u16DispVPixelEnd + pinfo->u16DispVPixelStart) >> 1;       // pix
    }
    else
    {
        pinfo->u16DispVPixelStart = sGetInfo.u16DispVPixelStart;            // pix

        if (g_pGOPCtxLocal->Gwin_V_Dup)
        {
            if( ((Gop1Dst == E_DRV_GOP_DST_IP0) || (Gop1Dst == E_DRV_GOP_DST_IP1)))
                pinfo->u16DispVPixelEnd = (((sGetInfo.u16DispVPixelEnd - pinfo->u16DispVPixelStart) << 1) + pinfo->u16DispVPixelStart) >> 1;
            else if((Gop1Dst == E_DRV_GOP_DST_VE) || (Gop1Dst == E_DRV_GOP_DST_OP1))
                pinfo->u16DispVPixelEnd = (((sGetInfo.u16DispVPixelEnd - pinfo->u16DispVPixelStart) << 1) + pinfo->u16DispVPixelStart) >> 1;
            else
                pinfo->u16DispVPixelEnd = (sGetInfo.u16DispVPixelEnd + pinfo->u16DispVPixelStart) >> 1;        // pix
        }
        else
        {
            if( ((Gop1Dst == E_DRV_GOP_DST_IP0) || (Gop1Dst == E_DRV_GOP_DST_IP1)))
                pinfo->u16DispVPixelEnd = ((sGetInfo.u16DispVPixelEnd - pinfo->u16DispVPixelStart) << 1) + pinfo->u16DispVPixelStart;
            else if((Gop1Dst == E_DRV_GOP_DST_VE) || (Gop1Dst == E_DRV_GOP_DST_OP1))
                pinfo->u16DispVPixelEnd = ((sGetInfo.u16DispVPixelEnd - pinfo->u16DispVPixelStart) << 1) + pinfo->u16DispVPixelStart;
            else if (((Gop1Dst == E_DRV_GOP_DST_OP0) || (Gop1Dst == E_DRV_GOP_DST_FRC) || (Gop1Dst == E_DRV_GOP_DST_BYPASS)) && !MDrv_GOP_GWIN_IsProgressive(g_pGOPCtxLocal,1))
            {
                pinfo->u16DispVPixelEnd = ((sGetInfo.u16DispVPixelEnd - pinfo->u16DispVPixelStart) << 1) + pinfo->u16DispVPixelStart;
            }
            else
                pinfo->u16DispVPixelEnd = sGetInfo.u16DispVPixelEnd;            // pix
        }
    }

    if (HAS_BIT(g_pGOPCtxLocal->u16GOP_HMirror_HPos, u8win))
    {
        pinfo->u16DispHPixelStart = g_pGOPCtxLocal->sMirrorInfo[u8win].u16NonMirrorHStr;
        pinfo->u16DispHPixelEnd = g_pGOPCtxLocal->sMirrorInfo[u8win].u16NonMirrorHEnd;

        if (g_pGOPCtxLocal->Gwin_H_Dup)
            pinfo->u16DispHPixelEnd = (pinfo->u16DispHPixelEnd + pinfo->u16DispHPixelStart) >> 1;   // pix
    }
    else
    {
        pinfo->u16DispHPixelStart = sGetInfo.u16DispHPixelStart;  // pix

        if (g_pGOPCtxLocal->Gwin_H_Dup)
            pinfo->u16DispHPixelEnd = (sGetInfo.u16DispHPixelEnd + pinfo->u16DispHPixelStart) >> 1;     // pix
        else
            pinfo->u16DispHPixelEnd = sGetInfo.u16DispHPixelEnd;    // pix
    }

    pinfo->u16RBlkHRblkSize = sGetInfo.u16RBlkHRblkSize;
    pinfo->u32DRAMRBlkStart = sGetInfo.u64DRAMRBlkStart;

    if (HAS_BIT(g_pGOPCtxLocal->u16GOP_HMirrorRBLK_Adr, u8win) ||
        HAS_BIT(g_pGOPCtxLocal->u16GOP_VMirrorRBLK_Adr, u8win))
    {
        pinfo->u32DRAMRBlkStart = g_pGOPCtxLocal->sMirrorInfo[u8win].u64NonMirrorFBAdr;
    }
}

static void _GOP_GetGop23WinInfo(void* pInstance,MS_U8 u8win, GOP_GwinInfo* pinfo)
{
    DRV_GOP_GWIN_INFO sGetInfo;
    DRV_GOPDstType Gop23Dst =E_DRV_GOP_DST_INVALID;

#ifdef INSTANT_PRIVATE
    GOP_INSTANT_PRIVATE* psGOPInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void*)&psGOPInstPri);
#endif

    if (!_GOP_IsGwinIdValid(pInstance,u8win))
    {
        GOP_M_ERR("[%s][%d]GWIN %d  is out of range\n",__FUNCTION__,__LINE__,u8win);
        return;
    }


    MDrv_GOP_GWIN_GetDstPlane(g_pGOPCtxLocal, MDrv_DumpGopByGwinId(g_pGOPCtxLocal,u8win), &Gop23Dst);
    if (Gop23Dst ==E_DRV_GOP_DST_INVALID)
    {
        APIGOP_ASSERT(FALSE, GOP_M_FATAL("\n Get GOP destination fail!\n"));
        return;
    }

    memset(&sGetInfo, 0, sizeof(DRV_GOP_GWIN_INFO));
    MDrv_GOP_GWIN_GetGwinInfo(g_pGOPCtxLocal, u8win, &sGetInfo);
    pinfo->u16RBlkHPixSize = sGetInfo.u16RBlkHPixSize;
    pinfo->u16RBlkVPixSize = sGetInfo.u16RBlkVPixSize;
    pinfo->clrType = (EN_GOP_COLOR_TYPE)sGetInfo.clrType;

    if (HAS_BIT(g_pGOPCtxLocal->u16GOP_VMirror_VPos, u8win))
    {
        pinfo->u16DispVPixelStart = g_pGOPCtxLocal->sMirrorInfo[u8win].u16NonMirrorVStr;
        pinfo->u16DispVPixelEnd = g_pGOPCtxLocal->sMirrorInfo[u8win].u16NonMirrorVEnd;

        if (g_pGOPCtxLocal->Gwin_V_Dup)
            pinfo->u16DispVPixelEnd = (pinfo->u16DispVPixelEnd + pinfo->u16DispVPixelStart) >> 1;       // pix
    }
    else
    {
        pinfo->u16DispVPixelStart = sGetInfo.u16DispVPixelStart;            // pix

        if (g_pGOPCtxLocal->Gwin_V_Dup)
        {
            if( ((Gop23Dst == E_DRV_GOP_DST_IP0) || (Gop23Dst == E_DRV_GOP_DST_IP1)))
                pinfo->u16DispVPixelEnd = (((sGetInfo.u16DispVPixelEnd - pinfo->u16DispVPixelStart) << 1) + pinfo->u16DispVPixelStart) >> 1;
            else if((Gop23Dst == E_DRV_GOP_DST_VE) || (Gop23Dst == E_DRV_GOP_DST_OP1))
                pinfo->u16DispVPixelEnd = (((sGetInfo.u16DispVPixelEnd - pinfo->u16DispVPixelStart) << 1) + pinfo->u16DispVPixelStart) >> 1;
            else
                pinfo->u16DispVPixelEnd = (sGetInfo.u16DispVPixelEnd + pinfo->u16DispVPixelStart) >> 1;        // pix
        }
        else
        {
            if( ((Gop23Dst == E_DRV_GOP_DST_IP0) || (Gop23Dst == E_DRV_GOP_DST_IP1)))
                pinfo->u16DispVPixelEnd = ((sGetInfo.u16DispVPixelEnd - pinfo->u16DispVPixelStart) << 1) + pinfo->u16DispVPixelStart;
            else if((Gop23Dst == E_DRV_GOP_DST_VE) || (Gop23Dst == E_DRV_GOP_DST_OP1))
                pinfo->u16DispVPixelEnd = ((sGetInfo.u16DispVPixelEnd - pinfo->u16DispVPixelStart) << 1) + pinfo->u16DispVPixelStart;
            else if (((Gop23Dst == E_DRV_GOP_DST_OP0) || (Gop23Dst == E_DRV_GOP_DST_FRC) || (Gop23Dst == E_DRV_GOP_DST_BYPASS)) && !MDrv_GOP_GWIN_IsProgressive(g_pGOPCtxLocal, MDrv_DumpGopByGwinId(g_pGOPCtxLocal,u8win)))
            {
                pinfo->u16DispVPixelEnd = ((sGetInfo.u16DispVPixelEnd - pinfo->u16DispVPixelStart) << 1) + pinfo->u16DispVPixelStart;
            }
            else
                pinfo->u16DispVPixelEnd = sGetInfo.u16DispVPixelEnd;            // pix
        }
    }

    if (HAS_BIT(g_pGOPCtxLocal->u16GOP_HMirror_HPos, u8win))
    {
        pinfo->u16DispHPixelStart = g_pGOPCtxLocal->sMirrorInfo[u8win].u16NonMirrorHStr;
        pinfo->u16DispHPixelEnd = g_pGOPCtxLocal->sMirrorInfo[u8win].u16NonMirrorHEnd;

        if (g_pGOPCtxLocal->Gwin_H_Dup)
            pinfo->u16DispHPixelEnd = (pinfo->u16DispHPixelEnd + pinfo->u16DispHPixelStart) >> 1;   // pix
    }
    else
    {
        pinfo->u16DispHPixelStart = sGetInfo.u16DispHPixelStart;  // pix

        if (g_pGOPCtxLocal->Gwin_H_Dup)
            pinfo->u16DispHPixelEnd = (sGetInfo.u16DispHPixelEnd + pinfo->u16DispHPixelStart) >> 1;     // pix
        else
            pinfo->u16DispHPixelEnd = sGetInfo.u16DispHPixelEnd;    // pix
    }

    pinfo->u16RBlkHRblkSize = sGetInfo.u16RBlkHRblkSize;
    pinfo->u32DRAMRBlkStart = sGetInfo.u64DRAMRBlkStart;

    if (HAS_BIT(g_pGOPCtxLocal->u16GOP_HMirrorRBLK_Adr, u8win) ||
        HAS_BIT(g_pGOPCtxLocal->u16GOP_VMirrorRBLK_Adr, u8win))
    {
        pinfo->u32DRAMRBlkStart = g_pGOPCtxLocal->sMirrorInfo[u8win].u64NonMirrorFBAdr;
    }
}




MS_U8 _GOP_SelGwinId2(void* pInstance,MS_U32 u32GOP, MS_U32 u8GwinIdx)
{
    MS_U8 tmp;

#ifdef INSTANT_PRIVATE
    GOP_INSTANT_PRIVATE* psGOPInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void*)&psGOPInstPri);
#endif

    if (!_GOP_IsGopNumValid(pInstance,u32GOP))
    {
        GOP_M_ERR("[%s][%d]GOP %td  is out of range\n",__FUNCTION__,__LINE__, (ptrdiff_t)u32GOP);
        return GWIN_NO_AVAILABLE;
    }
    tmp = MDrv_GOP_SelGwinIdByGOP(g_pGOPCtxLocal, u32GOP, u8GwinIdx);

    return tmp;

}

static E_GOP_API_Result _GOP_MAP_MUX_Enum(void* pInstance,EN_Gop_MuxSel GOPMux, Gop_MuxSel *pGOPMux)
{
    E_GOP_API_Result ret = GOP_API_FAIL;

    switch (GOPMux)
    {
        case EN_GOP_MUX0:
            *pGOPMux = E_GOP_MUX0;
            ret = GOP_API_SUCCESS;
            break;
        case EN_GOP_MUX1:
            *pGOPMux = E_GOP_MUX1;
            ret = GOP_API_SUCCESS;
            break;
        case EN_GOP_MUX2:
            *pGOPMux = E_GOP_MUX2;
            ret = GOP_API_SUCCESS;
            break;
        case EN_GOP_MUX3:
            *pGOPMux = E_GOP_MUX3;
            ret = GOP_API_SUCCESS;
            break;
        case EN_GOP_MUX4:
            *pGOPMux = E_GOP_MUX4;
            ret = GOP_API_SUCCESS;
            break;
        case EN_GOP_IP0_MUX:
            *pGOPMux = E_GOP_IP0_MUX;
            ret = GOP_API_SUCCESS;
            break;
        case EN_GOP_IP1_MUX:
            *pGOPMux = E_GOP_IP1_MUX;
            ret = GOP_API_SUCCESS;
            break;
        case EN_GOP_VOP0_MUX:
            *pGOPMux = E_GOP_VOP0_MUX;
            ret = GOP_API_SUCCESS;
            break;
        case EN_GOP_VOP1_MUX:
            *pGOPMux = E_GOP_VOP1_MUX;
            ret = GOP_API_SUCCESS;
            break;
        case EN_GOP_FRC_MUX0:
            *pGOPMux = E_GOP_FRC_MUX0;
            ret = GOP_API_SUCCESS;
            break;
        case EN_GOP_FRC_MUX1:
            *pGOPMux = E_GOP_FRC_MUX1;
            ret = GOP_API_SUCCESS;
            break;
        case EN_GOP_FRC_MUX2:
            *pGOPMux = E_GOP_FRC_MUX2;
            ret = GOP_API_SUCCESS;
            break;
        case EN_GOP_FRC_MUX3:
            *pGOPMux = E_GOP_FRC_MUX3;
            ret = GOP_API_SUCCESS;
            break;
        case EN_GOP_BYPASS_MUX0:
            *pGOPMux = E_GOP_BYPASS_MUX0;
            ret = GOP_API_SUCCESS;
            break;
        case EN_GOP_IP0_SUB_MUX:
            *pGOPMux = E_GOP_IP0_SUB_MUX;
            ret = GOP_API_SUCCESS;
            break;
        case EN_GOP_VOP0_SUB_MUX:
            *pGOPMux = E_GOP_VOP0_SUB_MUX;
            ret = GOP_API_SUCCESS;
            break;
        case EN_GOP_OP1_MUX:
            *pGOPMux = E_GOP_OP1_MUX;
            ret = GOP_API_SUCCESS;
            break;
        case EN_GOP_DIP_MUX:
            *pGOPMux = E_GOP_DIP_MUX;
            ret = GOP_API_SUCCESS;
            break;
        case EN_GOP_GS_MUX:
            *pGOPMux = E_GOP_GS_MUX;
            ret = GOP_API_SUCCESS;
            break;
        default:
            *pGOPMux = E_GOP_MUX_INVAILD;
            GOP_M_ERR("\n MAP GOP Mux error!! \n");
            ret = GOP_API_FAIL;
            break;
    }
    return ret;
}
static MS_U32 _GOP_Map_APIDst2DRV_Enum(void* pInstance,EN_GOP_DST_TYPE GopDst, DRV_GOPDstType *pGopDst)
{
#ifdef INSTANT_PRIVATE
    GOP_INSTANT_PRIVATE* psGOPInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void*)&psGOPInstPri);
#endif
    MS_U32 ret = GOP_API_FAIL;

    switch (GopDst)
    {
        case E_GOP_DST_IP0:
            *pGopDst = E_DRV_GOP_DST_IP0;
            ret = (MS_U32)GOP_API_SUCCESS;
            break;
        case E_GOP_DST_MIXER2VE:
            *pGopDst = E_DRV_GOP_DST_MIXER2VE;
            ret = (MS_U32)GOP_API_SUCCESS;
            break;
        case E_GOP_DST_OP0:
            *pGopDst = E_DRV_GOP_DST_OP0;
            ret = (MS_U32)GOP_API_SUCCESS;
            break;
        case E_GOP_DST_VOP:
            *pGopDst = E_DRV_GOP_DST_VOP;
            ret = (MS_U32)GOP_API_SUCCESS;
            break;
        case E_GOP_DST_IP1:
            *pGopDst = E_DRV_GOP_DST_IP1;
            ret = (MS_U32)GOP_API_SUCCESS;
            break;
        case E_GOP_DST_IP_MAIN:
            *pGopDst = E_DRV_GOP_DST_IP0; //For compatible temporaily
            GOP_M_WARN("\n %s, E_GOP_DST_IP_MAIN is going to be removed, please choose E_GOP_DST_IP0!! \n", __FUNCTION__);
            ret = (MS_U32)GOP_API_SUCCESS;
            break;
        case E_GOP_DST_IP_SUB:            //For compatible temporaily
            *pGopDst = E_DRV_GOP_DST_IP0_SUB;
            ret = (MS_U32)GOP_API_SUCCESS;
            break;
        case E_GOP_DST_MIXER2OP:
            *pGopDst = E_DRV_GOP_DST_MIXER2OP;
            ret = (MS_U32)GOP_API_SUCCESS;
            break;
        case E_GOP_DST_VOP_SUB:
            *pGopDst = E_DRV_GOP_DST_VOP_SUB;
            ret = (MS_U32)GOP_API_SUCCESS;
            break;
        case E_GOP_DST_FRC:
            *pGopDst = E_DRV_GOP_DST_FRC;
            ret = (MS_U32)GOP_API_SUCCESS;
            break;
        case E_GOP_DST_VE:
            *pGopDst = E_DRV_GOP_DST_VE;
            ret = (MS_U32)GOP_API_SUCCESS;
            break;
        case E_GOP_DST_BYPASS:
            *pGopDst = E_DRV_GOP_DST_BYPASS;
            ret = (MS_U32)GOP_API_SUCCESS;
            break;
        case E_GOP_DST_OP1:
            *pGopDst = E_DRV_GOP_DST_OP1;
            ret = (MS_U32)GOP_API_SUCCESS;
            break;
        case E_GOP_DST_MIXER2OP1:
            *pGopDst = E_DRV_GOP_DST_MIXER2OP1;
            ret = (MS_U32)GOP_API_SUCCESS;
            break;
        case E_GOP_DST_DIP:
            *pGopDst = E_DRV_GOP_DST_DIP;
            ret = (MS_U32)GOP_API_SUCCESS;
            break;
        case E_GOP_DST_GOPScaling:
            *pGopDst = E_DRV_GOP_DST_GOPScaling;
            ret = (MS_U32)GOP_API_SUCCESS;
            break;
        case E_GOP_DST_OP_DUAL_RATE:
            if(g_pGOPCtxLocal->pGopChipProperty->b120HzDstOP == TRUE)
            {
                *pGopDst = E_DRV_GOP_DST_OP0;
            }
            else
            {
                *pGopDst = E_DRV_GOP_DST_OP_DUAL_RATE;
            }
            ret = (MS_U32)GOP_API_SUCCESS;
            break;
        default:
            *pGopDst = E_DRV_GOP_DST_INVALID;
            GOP_M_WARN("\n MAP GOP Dst plane error!! \n");
            ret = (MS_U32)GOP_API_FAIL;
            break;
    }
    return ret;
}

static MS_U32 _GOP_Map_DRVDst2API_Enum_(void* pInstance,EN_GOP_DST_TYPE *pGopDst, DRV_GOPDstType GopDst)
{
    MS_U32 ret = 0;

    switch (GopDst)
    {
        case E_DRV_GOP_DST_IP0:
            *pGopDst = E_GOP_DST_IP0;
            ret = (MS_U32)GOP_API_SUCCESS;
            break;
        case E_DRV_GOP_DST_MIXER2VE:
            *pGopDst = E_GOP_DST_MIXER2VE;
            ret = (MS_U32)GOP_API_SUCCESS;
            break;
        case E_DRV_GOP_DST_OP0:
            *pGopDst = E_GOP_DST_OP0;
            ret = (MS_U32)GOP_API_SUCCESS;
            break;
        case E_DRV_GOP_DST_VOP:
            *pGopDst = E_GOP_DST_VOP;
            ret = (MS_U32)GOP_API_SUCCESS;
            break;
        case E_DRV_GOP_DST_IP1:
            *pGopDst = E_GOP_DST_IP1;
            ret = (MS_U32)GOP_API_SUCCESS;
            break;
        case E_DRV_GOP_DST_IP0_SUB:            //For compatible temporaily
            *pGopDst = E_GOP_DST_IP_SUB;
            ret = (MS_U32)GOP_API_SUCCESS;
            break;
        case E_DRV_GOP_DST_MIXER2OP:
            *pGopDst = E_GOP_DST_MIXER2OP;
            ret = (MS_U32)GOP_API_SUCCESS;
            break;
        case E_DRV_GOP_DST_VOP_SUB:
            *pGopDst = E_GOP_DST_VOP_SUB;
            ret = (MS_U32)GOP_API_SUCCESS;
            break;
        case E_DRV_GOP_DST_FRC:
            *pGopDst = E_GOP_DST_FRC;
            ret = (MS_U32)GOP_API_SUCCESS;
            break;
        case E_DRV_GOP_DST_VE:
            *pGopDst = E_GOP_DST_VE;
            ret = (MS_U32)GOP_API_SUCCESS;
            break;
        case E_DRV_GOP_DST_BYPASS:
            *pGopDst = E_GOP_DST_BYPASS;
            ret = (MS_U32)GOP_API_SUCCESS;
            break;
        case E_DRV_GOP_DST_OP1:
            *pGopDst = E_GOP_DST_OP1;
            ret = (MS_U32)GOP_API_SUCCESS;
            break;
        case E_DRV_GOP_DST_MIXER2OP1:
            *pGopDst = E_GOP_DST_MIXER2OP1;
            ret = (MS_U32)GOP_API_SUCCESS;
            break;
        case E_DRV_GOP_DST_DIP:
            *pGopDst = E_GOP_DST_DIP;
            ret = (MS_U32)GOP_API_SUCCESS;
            break;
        case E_DRV_GOP_DST_GOPScaling:
            *pGopDst = E_GOP_DST_GOPScaling;
            ret = (MS_U32)GOP_API_SUCCESS;
            break;
        case E_DRV_GOP_DST_OP_DUAL_RATE :
            *pGopDst = E_GOP_DST_OP_DUAL_RATE;
            ret = (MS_U32)GOP_API_SUCCESS;
            break;
        default:
            *pGopDst = (EN_GOP_DST_TYPE)0xff;
            GOP_M_WARN("\n MAP GOP Dst plane error!! \n");
            ret = (MS_U32)GOP_API_FAIL;
            break;
    }
    return ret;
}

static MS_U32 _GOP_CalcPitch(void* pInstance,MS_U8 fbFmt, MS_U16 width)
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

static MS_U16 _GOP_GetBPP(void* pInstance,EN_GOP_COLOR_TYPE fbFmt)
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
        bpp = 0xFFFF;
        break;
    }
    return bpp;

}

static MS_U8 _GOP_GWIN_IsGwinCreated(void* pInstance,MS_U8 gId)
{
#ifdef INSTANT_PRIVATE
    GOP_INSTANT_PRIVATE* psGOPInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void*)&psGOPInstPri);
#endif

    if (!_GOP_IsGwinIdValid(pInstance,gId))
    {
        GOP_M_ERR("[%s][%d]GWIN %d  is out of range\n",__FUNCTION__,__LINE__,gId);
        return FALSE;
    }

    if((gId< g_pGOPCtxLocal->pGopChipProperty->TotalGwinNum)
        && (GWIN_ID_INVALID ==g_pGOPCtxLocal->pGOPCtxShared->gwinMap[gId].u32CurFBId))
    {
        return FALSE;
    }

    GOP_M_INFO("IsGwinCreated Debug: gId=%d\n", gId);
    return TRUE;
}

#if 0
static MS_BOOL  bCursorSupport(void* pInstance,MS_U8 u8GOP)
{
#if CURSOR_SUPPORT
    //if cursor GOP, dont need to do adjust
    if(u8GOP == 3)
        return TRUE;
    else
        return FALSE;
#else
    return FALSE;
#endif
}
#endif

// Alignment stretch window value
static void _GOP_GWIN_Align_StretchWin(void* pInstance, MS_U8 u8GOP, EN_GOP_DST_TYPE eDstType, MS_U16 *pu16x, MS_U16 *pu16y, MS_U16 *pu16Width, MS_U16 *pu16height, MS_U16 BPP)
{
    MS_U16 u16align_offset;
	MS_U16 u16GOP_Unit;

#ifdef INSTANT_PRIVATE
    GOP_INSTANT_PRIVATE* psGOPInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void*)&psGOPInstPri);
#endif

	u16GOP_Unit = MDrv_GOP_GetWordUnit(g_pGOPCtxLocal, u8GOP);

    if (((eDstType == E_GOP_DST_OP0) || (eDstType == E_GOP_DST_FRC) || (eDstType == E_GOP_DST_BYPASS)) && !MDrv_GOP_GWIN_IsProgressive(g_pGOPCtxLocal,u8GOP))
    {
        *pu16height /=2;
        *pu16y = *pu16y/2;
    }


	if(u16GOP_Unit ==1)
	{
		*pu16Width =((*pu16Width + 1)>> 1) << 1; //do 2 pixel align
    }
	else
	{
		u16align_offset = (u16GOP_Unit * 8 /BPP -1);
    	*pu16Width = (*pu16Width + u16align_offset)&(~u16align_offset) ; //do 8-byte align
	}
}

//Adjust stretch window for GOP 3D
static void _GOP_GWIN_Adjust_3DStretchWin(void* pInstance,MS_U8 u8Gop, MS_U16 *pu16x, MS_U16 *pu16y, MS_U16 *pu16Width, MS_U16 *pu16Height)
{
#ifdef INSTANT_PRIVATE
    GOP_INSTANT_PRIVATE* psGOPInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void*)&psGOPInstPri);
#endif

    if(g_pGOPCtxLocal->pGopChipProperty->enGOP3DType == E_DRV_3D_DUP_HALF)
    {
        if((g_pGOPCtxLocal->pGOPCtxShared->GOP_StereoMode[u8Gop] == E_DRV_GOP_3D_LINE_ALTERNATIVE) ||
           (g_pGOPCtxLocal->pGOPCtxShared->GOP_StereoMode[u8Gop] == E_DRV_GOP_3D_TOP_BOTTOM)
          )
        {
            *pu16Height = *pu16Height <<1;
            //Line by line, do not need middle
            if(g_pGOPCtxLocal->pGOPCtxShared->GOP_StereoMode[u8Gop] == E_DRV_GOP_3D_TOP_BOTTOM)
            {
                *pu16Height +=g_pGOPCtxLocal->u16GOPGWinMiddle[u8Gop]; //Stretch win includes middle
            }
        }
        else if(g_pGOPCtxLocal->pGOPCtxShared->GOP_StereoMode[u8Gop] == E_DRV_GOP_3D_LINE_FRAMEPACKING)
        {
             //Frame packing, no middle, always full screen
             //Adjust Gap only
             *pu16Height = (*pu16Height << 1) + g_pGOPCtxLocal->GOP_FramePacking_Gap;
        }
        else if(g_pGOPCtxLocal->pGOPCtxShared->GOP_StereoMode[u8Gop] == E_DRV_GOP_3D_SIDE_BY_SYDE)
        {
            *pu16Width = *pu16Width <<1;
            *pu16Width +=g_pGOPCtxLocal->u16GOPGWinMiddle[u8Gop]; //Stretch win includes middle
        }
    }
}

static MS_U8 _GOP_GWIN_IsGwinExistInClient(void* pInstance,MS_U8 gId)
{
    MS_BOOL result0, result1, result2, result3, result4;
    MS_U32 MaxGwinSupport;

    GOP_WinFB_INFO* pwinFB;

#ifdef INSTANT_PRIVATE
    GOP_INSTANT_PRIVATE* psGOPInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void*)&psGOPInstPri);
#endif

    if (!_GOP_IsGwinIdValid(pInstance,gId))
    {
        GOP_M_ERR("[%s][%d]GWIN %d  is out of range\n",__FUNCTION__,__LINE__,gId);
        return FALSE;
    }

    MaxGwinSupport = g_pGOPCtxLocal->pGopChipProperty->TotalGwinNum;

    pwinFB = _GetWinFB(pInstance,g_pGOPCtxLocal->pGOPCtxShared->gwinMap[gId].u32CurFBId);

    if(pwinFB == NULL)
    {
        GOP_M_ERR("[%s][%d]GetWinFB Fail : gwinMap[%d].u32CurFBId=%td\n",__FUNCTION__,__LINE__,gId,(ptrdiff_t)g_pGOPCtxLocal->pGOPCtxShared->gwinMap[gId].u32CurFBId);
        return FALSE;
    }

    result0 = (MaxGwinSupport<=gId);
    result1 = (FALSE == g_pGOPCtxLocal->pGOPCtxShared->gwinMap[gId].bIsShared);
#if defined(MSOS_TYPE_LINUX) || defined(MSOS_TYPE_LINUX_KERNEL)
    MS_U32 pid = (GETPIDTYPE)getpid();
    result2 = (g_pGOPCtxLocal->pGOPCtxShared->gwinMap[gId].u32GOPClientId != pid);
#else
    result2 = (g_pGOPCtxLocal->pGOPCtxShared->gwinMap[gId].u32GOPClientId != g_pGOPCtxLocal->u32GOPClientId);
#endif
    result3 = (DRV_MAX_GWIN_FB_SUPPORT <= g_pGOPCtxLocal->pGOPCtxShared->gwinMap[gId].u32CurFBId);
    result4 = (0 == pwinFB->in_use);

    if ( result0 || (result1 && (result2 || result3 || result4) ) )
    {
#if defined(MSOS_TYPE_LINUX) || defined(MSOS_TYPE_LINUX_KERNEL)
		GOP_M_INFO("result0 = %d,result1 = %d,result2 = %d,result3 = %d,result4 = %d\n",result0,result1,result2,result3,result4);
        GOP_M_INFO("[%s][%d]GWIN %d  is not exist\n",__FUNCTION__,__LINE__,gId);
#else
        GOP_M_INFO("IsGwinExistInClient Debug: gId =%d, \n" , gId);
		GOP_M_INFO("gwinMap[gId].u32GOPClientId: gId =%td, \n" , (ptrdiff_t)(g_pGOPCtxLocal->pGOPCtxShared->gwinMap[gId].u32GOPClientId) );
		GOP_M_INFO("_pGOPCtxLocal->u32GOPClientId =%td, \n" , (ptrdiff_t)(g_pGOPCtxLocal->u32GOPClientId) );
		GOP_M_INFO("gwinMap[gId].u32CurFBId =%td, \n" , (ptrdiff_t)(g_pGOPCtxLocal->pGOPCtxShared->gwinMap[gId].u32CurFBId));
		GOP_M_INFO("gwinMap[gId].u32CurFBId].in_use =%d, \n" , pwinFB->in_use);
#endif
        return FALSE;
    }
    return TRUE;

}


#if 0
static void _GOP_GWIN_IsEnableMirror(void* pInstance,DRV_GOPDstType eGopDst, MS_BOOL *pbHMirror, MS_BOOL *pbVMirror)
{

#ifdef INSTANT_PRIVATE
    GOP_INSTANT_PRIVATE* psGOPInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void*)&psGOPInstPri);
#endif

    if((pbHMirror == NULL) || (pbVMirror == NULL))
    {
        return;
    }

    if(  (eGopDst == E_DRV_GOP_DST_OP0)
       ||(eGopDst == E_DRV_GOP_DST_OP_DUAL_RATE)
       ||(eGopDst == E_DRV_GOP_DST_BYPASS)
       ||(eGopDst == E_DRV_GOP_DST_FRC)
       ||(  ((eGopDst == E_DRV_GOP_DST_IP0) || (eGopDst == E_DRV_GOP_DST_IP0_SUB)))
          //&&(E_GOP_VIDEOTIMING_MIRROR_BYMVOP == Mdrv_GOP_GetVideoTimingMirrorType(g_pGOPCtxLocal, TRUE)))
      )
    {
        *pbHMirror = g_pGOPCtxLocal->pGOPCtxShared->bHMirror;
        *pbVMirror = g_pGOPCtxLocal->pGOPCtxShared->bVMirror;
    }
    else
    {
        #if (FPGA_TEST)  //FPGA use VOP path to verify mirror.
            *pbHMirror = g_pGOPCtxLocal->pGOPCtxShared->bHMirror;
            *pbVMirror = g_pGOPCtxLocal->pGOPCtxShared->bVMirror;
        #else
            *pbHMirror = FALSE;
            *pbVMirror = FALSE;
        #endif
    }
}
#endif

static MS_BOOL _GetGOPEnum(void* pInstance)
{
#ifdef INSTANT_PRIVATE
    GOP_INSTANT_PRIVATE* psGOPInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void*)&psGOPInstPri);
#endif

    memset(&GOPTYPE, 0xff, sizeof(GOP_TYPE_DEF));
    return MDrv_GOP_GetGOPEnum(g_pGOPCtxLocal, &GOPTYPE);
}


static MS_BOOL _GOP_Init_Ctx(void* pInstance,MS_BOOL *pbFirstDrvInstant)
{
    MS_BOOL bNeedInitShared =FALSE;

#ifdef INSTANT_PRIVATE
    GOP_INSTANT_PRIVATE* psGOPInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void*)&psGOPInstPri);
#endif

    _GetGOPEnum(pInstance);

    if(g_pGOPCtxLocal)
    {
        if(bFirstInit)
        {
            *pbFirstDrvInstant = TRUE;
        }
        else
        {
            *pbFirstDrvInstant = FALSE;
        }

        return TRUE;
    }
    g_pGOPCtxLocal = Drv_GOP_Init_Context(pInstance,&bNeedInitShared);
    if(NULL == g_pGOPCtxLocal)
    {
        GOP_M_ERR("Error : g_pGOPCtxLocal = NULL\n");
        return FALSE;
    }

    g_pGOPCtxLocal->pGOPCtxShared->bInitShared = bNeedInitShared;

    if(pbFirstDrvInstant)
        *pbFirstDrvInstant = bNeedInitShared;


#if defined(MSOS_TYPE_LINUX) || defined(MSOS_TYPE_LINUX_KERNEL)
    g_pGOPCtxLocal->u32GOPClientId = (GETPIDTYPE)getpid();
#else
    g_pGOPCtxLocal->u32GOPClientId = ++g_pGOPCtxLocal->pGOPCtxShared->u32ClientIdAllocator;
    if(0 == g_pGOPCtxLocal->u32GOPClientId)
        g_pGOPCtxLocal->u32GOPClientId = ++g_pGOPCtxLocal->pGOPCtxShared->u32ClientIdAllocator;
#endif

    memset(g_pGOPCtxLocal->sMirrorInfo, 0, g_pGOPCtxLocal->pGopChipProperty->TotalGwinNum * sizeof(GOP_GwinMirror_Info));

    memset(&g_pGOPCtxLocal->sGOPConfig, 0, sizeof(GOP_Config));

    memset(g_pGOPCtxLocal->MS_MuxGop, 0, MAX_GOP_MUX_SUPPORT*sizeof(MS_U8));
    g_pGOPCtxLocal->IsChgMux= FALSE;
    g_pGOPCtxLocal->IsClkClosed= FALSE;
    g_pGOPCtxLocal->u8ChgIpMuxGop = 0xFF;
#if GOP_LOCK_SUPPORT
    char GOP_MUTEX[] = "GOP_Mutex";

    g_pGOPCtxLocal->s32GOPMutexCnt = 0;
    g_pGOPCtxLocal->s32GOPMutex = MsOS_CreateMutex(E_MSOS_FIFO, (char *)GOP_MUTEX, MSOS_PROCESS_SHARED);
    if (0 > g_pGOPCtxLocal->s32GOPMutex)
    {
        GOP_M_ERR("%screate gop mutex fail\n", __FUNCTION__);
        return FALSE;
    }
    g_pGOPCtxLocal->s32GOPLock = -1;
#endif

#if GOP_XC_LOCK_SUPPORT
    char XC_MUTEX[] = "_XC_Mutex";

    g_pGOPCtxLocal->s32GOPXC_MutexCnt = 0;
    g_pGOPCtxLocal->s32GOPXC_Mutex = MsOS_CreateMutex(E_MSOS_FIFO, (char *)XC_MUTEX, MSOS_PROCESS_SHARED);
    if (0 > g_pGOPCtxLocal->s32GOPXC_Mutex)
    {
        GOP_M_ERR("[%s][%06d] create XC mutex fail _GOPXC_Mutex=0x%x\n", __FUNCTION__, __LINE__, g_pGOPCtxLocal->s32GOPXC_Mutex);
    }
    g_pGOPCtxLocal->s32GOPXC_Lock = -1;
#endif
    return TRUE;
}

MS_BOOL GOP_GetWinInfo(void* pInstance,MS_U32 u32win, GOP_GwinInfo* pinfo)
{
    MS_U8 u8GOP;
#ifdef INSTANT_PRIVATE
    GOP_INSTANT_PRIVATE* psGOPInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void*)&psGOPInstPri);
#endif

    if (!_GOP_IsGwinIdValid(pInstance, u32win))
    {
        GOP_M_ERR("[%s][%d]GWIN %td  is out of range\n",__FUNCTION__,__LINE__,(ptrdiff_t)u32win);
        return FALSE;
    }
    memset(pinfo, 0, sizeof(GOP_GwinInfo));
    u8GOP = MDrv_DumpGopByGwinId(g_pGOPCtxLocal,u32win);

    if(u8GOP == GOPTYPE.GOP0)
    {
        _GOP_GetGop0WinInfo(pInstance,u32win,  pinfo);
    }
    else if (u8GOP == GOPTYPE.GOP1)
    {
        _GOP_GetGop1WinInfo(pInstance,u32win,  pinfo);
    }
    else
    {
        _GOP_GetGop23WinInfo(pInstance,u32win, pinfo);
    }

    return TRUE;
}

static void _SetGop0WinInfo(void* pInstance,MS_U8 u8win, GOP_GwinInfo* pinfo)
{
    MS_U16 u16Bpp = 0;
#ifdef INSTANT_PRIVATE
    GOP_INSTANT_PRIVATE* psGOPInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void*)&psGOPInstPri);
#endif

    MDrv_GOP_GWIN_SetWinFmt(g_pGOPCtxLocal, u8win, (DRV_GOPColorType)pinfo->clrType);
    //MDrv_GOP_GWIN_UpdateRegOnce(g_pGOPCtxLocal, FALSE);
    MDrv_GOP_GWIN_SetGwinInfo(g_pGOPCtxLocal, u8win, (DRV_GOP_GWIN_INFO*)pinfo);

    u16Bpp = _GOP_GetBPP(pInstance,pinfo->clrType);
    if(u16Bpp == 0 || u16Bpp > 64)
    {
        GOP_M_ERR("[%s][%d] Invalid bpp = %lu\n",__FUNCTION__,__LINE__,(unsigned long)u16Bpp);
        return;
    }
    GOP_M_INFO("GWIN_SetWin(%d): [adr(B), RBsz, offset] = [%td, %d, %d]\n", \
                u8win,\
                (ptrdiff_t)pinfo->u32DRAMRBlkStart,\
                pinfo->u16RBlkHPixSize * pinfo->u16RBlkVPixSize / (64/u16Bpp),\
                (pinfo->u16WinY * pinfo->u16RBlkHPixSize + pinfo->u16WinX)/(64/u16Bpp) );
    GOP_M_INFO("\t[Vst, Vend, Hst, Hend, GwinHsz] = [%d, %d, %d, %d, %d]\n",\
                pinfo->u16DispVPixelStart, \
                pinfo->u16DispVPixelEnd, \
                pinfo->u16DispHPixelStart/(64/u16Bpp), \
                pinfo->u16DispHPixelEnd/(64/u16Bpp),\
                pinfo->u16RBlkHPixSize/(64/u16Bpp) );
}

static void _SetGop1WinInfo(void* pInstance,MS_U8 u8win, GOP_GwinInfo* pinfo)
{
#ifdef INSTANT_PRIVATE
    GOP_INSTANT_PRIVATE* psGOPInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void*)&psGOPInstPri);
#endif

    MDrv_GOP_GWIN_SetWinFmt(g_pGOPCtxLocal, u8win, (DRV_GOPColorType)pinfo->clrType);
    MDrv_GOP_GWIN_SetGwinInfo(g_pGOPCtxLocal, u8win, (DRV_GOP_GWIN_INFO*)pinfo);
}

static void _SetGop23WinInfo(void* pInstance,MS_U8 u8win, GOP_GwinInfo* pinfo)
{
#ifdef INSTANT_PRIVATE
    GOP_INSTANT_PRIVATE* psGOPInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void*)&psGOPInstPri);
#endif

    MDrv_GOP_GWIN_SetGwinInfo(g_pGOPCtxLocal, u8win, (DRV_GOP_GWIN_INFO*)pinfo);
    MDrv_GOP_GWIN_SetWinFmt(g_pGOPCtxLocal, u8win, (DRV_GOPColorType)pinfo->clrType);
}

#if 0
static void _GOP_ArrangePoolMem(void* pInstance,GOP_FB_POOL *pFBPool)
{
    MS_PHY lastAddr, curStartAdr;
    MS_U64 size2move, moveSize, gapSize, count;
    GOP_GwinInfo gWin;
    MS_PHY *src_PA,*dst_PA;
    GOP_WinFB_INFO* pwinFB;
    MS_U8 u8GOP;
    MS_U8 u8WinID;

#ifdef INSTANT_PRIVATE
    GOP_INSTANT_PRIVATE* psGOPInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void*)&psGOPInstPri);
#endif

    MS_U32 u32NextFBId = pFBPool->u32FirstFBIdInPool;

    memset(&gWin, 0,  sizeof(GOP_GwinInfo));

    if(0 == pFBPool->u32GWinFB_Pool_MemLen)
        return;
    GOP_M_INFO("Arrange GOP pool %d\n", pFBPool->poolId);
    lastAddr = pFBPool->GWinFB_Pool_BaseAddr;


    if(fpEventNotify != NULL)
        fpEventNotify(E_GOP_EVENT_BUF_ARRANGE_BEGIN, NULL);
    else
    {
        //APIGOP_ASSERT(TRUE, printf("\nfpEvenNotifyBegin is Null"));
        GOP_M_INFO("%s: fpEvenNotifyBegin is not hooked. Bypass switch...\n", __FUNCTION__);
    }

    while(INVALID_POOL_NEXT_FBID != u32NextFBId)
    {
        pwinFB = _GetWinFB(pInstance,u32NextFBId);

        if(pwinFB == NULL)
        {
            GOP_M_ERR("[%s][%d]GetWinFB Fail : WrongFBID=%td\n",__FUNCTION__,__LINE__,(ptrdiff_t)u32NextFBId);
            return;
        }

        APIGOP_ASSERT(DRV_MAX_GWIN_FB_SUPPORT>u32NextFBId, GOP_M_FATAL("\nInvalid next fbid %td", (ptrdiff_t)u32NextFBId));
        APIGOP_ASSERT(pwinFB->in_use && pwinFB->poolId==pFBPool->poolId
                                , GOP_M_FATAL("\nSerious error, inconsistant Pool list at  fbid %td", (ptrdiff_t)u32NextFBId));
        APIGOP_ASSERT(pwinFB->addr >=  lastAddr
                                , GOP_M_FATAL("\nSerious error, inconsistant Pool list at  fbid %td", (ptrdiff_t)u32NextFBId));
        APIGOP_ASSERT(pwinFB->addr>= pFBPool->GWinFB_Pool_BaseAddr &&
                      (pwinFB->addr+pwinFB->size)<= (pFBPool->GWinFB_Pool_BaseAddr+pFBPool->u32GWinFB_Pool_MemLen)
                                , GOP_M_FATAL("\nSerious error, inconsistant Pool list at  fbid %td", (ptrdiff_t)u32NextFBId));

        APIGOP_ASSERT(IS_GWIN_SDRAM_ALIGN(pwinFB->addr)
                      , GOP_M_FATAL("\nInvalid alignment for addr 0x%tx", (ptrdiff_t)pwinFB->addr));
        if(pwinFB->addr == lastAddr)
        {
            lastAddr += GWIN_SDRAM_PG_ALIGN(pwinFB->size);
            u32NextFBId = pwinFB->nextFBIdInPool;
            continue;
        }

        curStartAdr = pwinFB->addr;
        gapSize = curStartAdr - lastAddr;
        moveSize = count = 0;
        size2move = pwinFB->size;

        while (moveSize < size2move)
        {
            count = (moveSize + gapSize > size2move) ? (size2move - moveSize) : gapSize;
            src_PA = (MS_PHY *)MS_PA2KSEG1(curStartAdr + moveSize);
            dst_PA = (MS_PHY *)MS_PA2KSEG1(lastAddr + moveSize);
            memcpy(dst_PA,src_PA, count);
            moveSize += count;
        }

        pwinFB->addr = lastAddr;

        for (u8WinID = 0; (MS_U8)u8WinID < (MS_U8)g_pGOPCtxLocal->pGopChipProperty->TotalGwinNum; u8WinID++)
        {
            if ((g_pGOPCtxLocal->pGOPCtxShared->gwinMap[(MS_U8)u8WinID].u32CurFBId ==u32NextFBId)
#if defined(MSOS_TYPE_LINUX) || defined(MSOS_TYPE_LINUX_KERNEL)
             && (g_pGOPCtxLocal->pGOPCtxShared->gwinMap[(MS_U8)u8WinID].u32GOPClientId == (GETPIDTYPE)getpid())
#else
             && (g_pGOPCtxLocal->pGOPCtxShared->gwinMap[(MS_U8)u8WinID].u32GOPClientId ==g_pGOPCtxLocal->u32GOPClientId)
#endif
            )
            {
                if(GOP_GetWinInfo(pInstance,(MS_U8)u8WinID, &gWin))
                {
                    gWin.u32DRAMRBlkStart = lastAddr;
                    u8GOP = MDrv_DumpGopByGwinId(g_pGOPCtxLocal,u8WinID);

                    if(u8GOP == GOPTYPE.GOP0)
                    {
                        _SetGop0WinInfo(pInstance,(MS_U8)u8WinID, &gWin);
                    }
                    else if(u8GOP == GOPTYPE.GOP1)
                    {
                        _SetGop1WinInfo(pInstance,(MS_U8)u8WinID, &gWin);
                    }
                    else
                    {
                        _SetGop23WinInfo(pInstance,(MS_U8)u8WinID, &gWin);
                    }
                    MDrv_GOP_GWIN_UpdateReg(g_pGOPCtxLocal,u8GOP);
                }
                else
                {
                    APIGOP_ASSERT(FALSE, GOP_M_FATAL("!!!Alert!!! Serious error in arrange pool memory\n"));
                }
            }
        }

    lastAddr = GWIN_SDRAM_PG_ALIGN(pwinFB->addr + pwinFB->size);
    u32NextFBId = pwinFB->nextFBIdInPool;

    }

    if(fpEventNotify != NULL)
        fpEventNotify(E_GOP_EVENT_BUF_ARRANGE_END, NULL);
    else
    {
        APIGOP_ASSERT(TRUE, GOP_M_FATAL("\nfpEvenNotifyEnd is Null"));
        GOP_M_INFO("%s: fpEvenNotifyEnd is not hooked. Bypass switch...\n", __FUNCTION__);
    }
}
#endif

static void _GOP_FreePoolMem(void* pInstance,GOP_FB_POOL *pFBPool, MS_U32 u32fbId)
{
    MS_U32 u32CurFBId, u32PreFBId;
    MS_PHY lastAddr;
    GOP_WinFB_INFO* pwinFB;
    GOP_WinFB_INFO* pPrewinFB;

    if (!_GOP_IsFbIdValid(pInstance,u32fbId))
    {
        GOP_M_ERR("[%s][%d]FbId %td  is out of range\n",__FUNCTION__,__LINE__,(ptrdiff_t)u32fbId);
        return;
    }

    u32CurFBId = pFBPool->u32FirstFBIdInPool;
    u32PreFBId = INVALID_POOL_NEXT_FBID;
    lastAddr = pFBPool->GWinFB_Pool_BaseAddr;

    while(INVALID_POOL_NEXT_FBID != u32CurFBId)
    {
        pwinFB = _GetWinFB(pInstance,u32CurFBId);

        if(pwinFB == NULL)
        {
            GOP_M_ERR("[%s][%d]GetWinFB Fail : WrongFBID=%td\n",__FUNCTION__,__LINE__,(ptrdiff_t)u32CurFBId);
            return;
        }

        APIGOP_ASSERT(DRV_MAX_GWIN_FB_SUPPORT>u32CurFBId, GOP_M_FATAL("\nInvalid next fbid %td", (ptrdiff_t)u32CurFBId));
            APIGOP_ASSERT(pwinFB->in_use && pwinFB->poolId==pFBPool->poolId
                                , GOP_M_FATAL("\nSerious error, inconsistant Pool list at  fbid %td", (ptrdiff_t)u32CurFBId));
            APIGOP_ASSERT(pwinFB->addr >=  lastAddr
                                , GOP_M_FATAL("\nSerious error, inconsistant Pool list at  fbid %td", (ptrdiff_t)u32CurFBId));
            APIGOP_ASSERT(pwinFB->addr>= pFBPool->GWinFB_Pool_BaseAddr &&
                          (pwinFB->addr+pwinFB->size)<= (pFBPool->GWinFB_Pool_BaseAddr+pFBPool->u32GWinFB_Pool_MemLen)
                                , GOP_M_FATAL("\nSerious error, inconsistant Pool list at  fbid %td", (ptrdiff_t)u32CurFBId));
        if(u32CurFBId == u32fbId)
            break;
        lastAddr = pwinFB->addr+pwinFB->size;
        u32PreFBId = u32CurFBId;
        u32CurFBId = pwinFB->nextFBIdInPool;
    }
    APIGOP_ASSERT(INVALID_POOL_NEXT_FBID!=u32CurFBId, GOP_M_FATAL("\n Inconsistent pool list for %td", (ptrdiff_t)u32CurFBId));

    pwinFB = _GetWinFB(pInstance,u32CurFBId);

    if(pwinFB == NULL)
    {
        GOP_M_ERR("[%s][%d]GetWinFB Fail : WrongFBID=%td\n",__FUNCTION__,__LINE__,(ptrdiff_t)u32CurFBId);
        return;
    }

    if(INVALID_POOL_NEXT_FBID == u32PreFBId)
            pFBPool->u32FirstFBIdInPool =  pwinFB->nextFBIdInPool;
    else
    {
        pPrewinFB = _GetWinFB(pInstance,u32PreFBId);

        if(pPrewinFB == NULL)
        {
            GOP_M_ERR("[%s][%d]GetWinFB Fail : WrongFBID=%td\n",__FUNCTION__,__LINE__,(ptrdiff_t)u32PreFBId);
            return;
        }

        pPrewinFB->nextFBIdInPool = pwinFB->nextFBIdInPool;
    }
    pwinFB->nextFBIdInPool = INVALID_POOL_NEXT_FBID;

}

static void _GOP_GetPDByDst(void* pInstance,MS_U8 u8GOP_num, DRV_GOPDstType GopDst,MS_U16 *u16StrwinStr)
{
#ifdef INSTANT_PRIVATE
    GOP_INSTANT_PRIVATE* psGOPInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void*)&psGOPInstPri);
#endif
    MS_BOOL bHDREnable=FALSE;
    //MS_WINDOW_TYPE stCapWin;
    //memset(&stCapWin, 0, sizeof(MS_WINDOW_TYPE));

    switch(GopDst)
    {
        case E_DRV_GOP_DST_IP0:
            if(g_pGOPCtxLocal->pGopChipProperty->bIgnoreIPHPD)
                *u16StrwinStr = 0;
            else
            {
                MS_U32 u32Hstart = 0;
                *u16StrwinStr = u32Hstart + g_pGOPCtxLocal->pGopChipProperty->GOP_IP_PD;
            }
            break;
        case E_DRV_GOP_DST_IP1:
            if(g_pGOPCtxLocal->pGopChipProperty->bIgnoreIP1HPD)
                *u16StrwinStr = 0;
            break;
        case E_DRV_GOP_DST_OP1:
            *u16StrwinStr = g_pGOPCtxLocal->pGopChipProperty->GOP_OP1_PD;
            break;
        case E_DRV_GOP_DST_OP0:
        case E_DRV_GOP_DST_GOPScaling:
            MDrv_GOP_IsHDREnabled(g_pGOPCtxLocal, &bHDREnable);
            if(bHDREnable==FALSE)
            {
                *u16StrwinStr = g_pGOPCtxLocal->pGOPCtxShared->u16PnlHStr[u8GOP_num] + g_pGOPCtxLocal->pGopChipProperty->GOP_PD;
            }
            else
            {
                *u16StrwinStr = g_pGOPCtxLocal->pGOPCtxShared->u16PnlHStr[u8GOP_num] + g_pGOPCtxLocal->pGopChipProperty->GOP_HDR_OP_PD;
            }
            break;
        case E_DRV_GOP_DST_FRC:
        case E_DRV_GOP_DST_DIP:
            *u16StrwinStr =0;
            break;
        case E_DRV_GOP_DST_BYPASS:
            *u16StrwinStr =g_pGOPCtxLocal->pGopChipProperty->GOP_Mux_FRC_offset;
            break;
        case E_DRV_GOP_DST_VOP:
            *u16StrwinStr = g_pGOPCtxLocal->pGopChipProperty->GOP_MVOP_PD;
            break;
        case E_DRV_GOP_DST_MIXER2VE:
        case E_DRV_GOP_DST_MIXER2OP:
            *u16StrwinStr = g_pGOPCtxLocal->pGopChipProperty->GOP_MIXER_PD;
            break;
        case E_DRV_GOP_DST_VE:
            *u16StrwinStr = g_pGOPCtxLocal->pGopChipProperty->GOP_VE_PD;
            break;
        case E_DRV_GOP_DST_OP_DUAL_RATE:
            *u16StrwinStr = g_pGOPCtxLocal->pGopChipProperty->GOP_DUAL_OP_PD;
            break;
        default:
            *u16StrwinStr = g_pGOPCtxLocal->pGopChipProperty->GOP_PD;
            break;
   }

}

static void _GOP_AdjustHSPD(void* pInstance,MS_U8 u8GOP_num, MS_U16 u16StrwinStr, DRV_GOPDstType GopDst)
{
    MS_U16 u16Offset;
#ifdef INSTANT_PRIVATE
    GOP_INSTANT_PRIVATE* psGOPInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void*)&psGOPInstPri);
#endif

    if (!_GOP_IsGopNumValid(pInstance,u8GOP_num))
    {
        GOP_M_ERR("[%s][%d]GOP %d  is out of range\n",__FUNCTION__,__LINE__,u8GOP_num);
        return;
    }

    switch(GopDst)
    {
    case E_DRV_GOP_DST_IP0:
    case E_DRV_GOP_DST_DIP:
    case E_DRV_GOP_DST_IP1:
        u16Offset = 0;
        break;
    case E_DRV_GOP_DST_MIXER2VE:
    case E_DRV_GOP_DST_MIXER2OP:
    case E_DRV_GOP_DST_VE:
    case E_DRV_GOP_DST_FRC:
        if(g_pGOPCtxLocal->pGopChipProperty->bIgnoreVEHPD)
                u16Offset = 0;
            else
                u16Offset = MDrv_GOP_GetHPipeOfst(g_pGOPCtxLocal,u8GOP_num, GopDst);
        break;
    case E_DRV_GOP_DST_OP0:
    case E_DRV_GOP_DST_VOP:
    case E_DRV_GOP_DST_BYPASS:
    case E_DRV_GOP_DST_OP_DUAL_RATE:
    default:
        u16Offset = MDrv_GOP_GetHPipeOfst(g_pGOPCtxLocal,u8GOP_num, GopDst);
        break;
    }

    MDrv_GOP_GWIN_SetHSPipe(g_pGOPCtxLocal, u8GOP_num, u16Offset+u16StrwinStr);
}

static void _GOP_InitHSPDByGOP(void* pInstance,MS_U8 u8GOP_num)
{
    MS_U16 u16StrwinStr=0;
    DRV_GOPDstType GopDst = E_DRV_GOP_DST_OP0;

#ifdef INSTANT_PRIVATE
    GOP_INSTANT_PRIVATE* psGOPInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void*)&psGOPInstPri);
#endif

    if (!_GOP_IsGopNumValid(pInstance,u8GOP_num))
    {
        GOP_M_ERR("[%s][%d]GOP %d  is out of range\n",__FUNCTION__,__LINE__,u8GOP_num);
        return;
    }

    MDrv_GOP_GWIN_GetDstPlane(g_pGOPCtxLocal, u8GOP_num, &GopDst);
    _GOP_GetPDByDst(pInstance,u8GOP_num, GopDst,&u16StrwinStr);
    _GOP_AdjustHSPD(pInstance,u8GOP_num, u16StrwinStr, GopDst);
}

static void _GOP_Get_StretchWin(void* pInstance,MS_U8 gop,MS_U16 *x,MS_U16 *y,MS_U16 *w,MS_U16 *h)
{
    DRV_GOPDstType eDstType = E_DRV_GOP_DST_INVALID;

#ifdef INSTANT_PRIVATE
    GOP_INSTANT_PRIVATE* psGOPInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void*)&psGOPInstPri);
#endif
    MDrv_GOP_GWIN_Get_StretchWin(g_pGOPCtxLocal, gop, x, y, w, h);
    MDrv_GOP_GWIN_GetDstPlane(g_pGOPCtxLocal,gop, &eDstType);

    if((eDstType == E_DRV_GOP_DST_OP0) || (eDstType == E_DRV_GOP_DST_FRC) || (eDstType == E_DRV_GOP_DST_BYPASS))
    {
        if (!MDrv_GOP_GWIN_IsProgressive(g_pGOPCtxLocal,gop))
        {
            (*h) *=2;
            (*y) = (*y)*2;
        }
    }
}

static void _GOP_SetOCCapability(void* pInstance,MS_U8 u8GOP, DRV_GOPDstType GopDst)
{
#ifdef INSTANT_PRIVATE
    GOP_INSTANT_PRIVATE* psGOPInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void*)&psGOPInstPri);
#endif

    if (!_GOP_IsGopNumValid(pInstance,u8GOP))
    {
        GOP_M_ERR("[%s][%d]GOP %d  is out of range\n",__FUNCTION__,__LINE__,u8GOP);
        return;
    }

    switch(GopDst)
    {
        case E_DRV_GOP_DST_IP0:
            MDrv_GOP_GWIN_SetAlphaInverse(g_pGOPCtxLocal, u8GOP, FALSE);
            MDrv_GOP_OC_SetOCEn(g_pGOPCtxLocal, u8GOP, FALSE);
            break;

        case E_DRV_GOP_DST_VOP:
            MDrv_GOP_OC_SetOCEn(g_pGOPCtxLocal, u8GOP, FALSE);
            break;

        case E_DRV_GOP_DST_OP0:
        case E_DRV_GOP_DST_BYPASS:
        case E_DRV_GOP_DST_GOPScaling:
        case E_DRV_GOP_DST_OP_DUAL_RATE:
            MDrv_GOP_GWIN_SetAlphaInverse(g_pGOPCtxLocal, u8GOP, TRUE);
            MDrv_GOP_OC_SetOCEn(g_pGOPCtxLocal, u8GOP, FALSE);
            break;

        case E_DRV_GOP_DST_FRC:
            MDrv_GOP_GWIN_SetAlphaInverse(g_pGOPCtxLocal, u8GOP, TRUE);
            MDrv_GOP_OC_SetOCEn(g_pGOPCtxLocal, u8GOP, TRUE);
            break;

        default:
            break;
      }

}

#if (DUMP_INFO)
static void _GOP_GWIN_UpdateAllInfo(void* pInstance,)
{
    MS_U8 u8Index = 0,u16Index = 0;
    GOP_WinFB_INFO* pwinFB;

#ifdef INSTANT_PRIVATE
    GOP_INSTANT_PRIVATE* psGOPInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void*)&psGOPInstPri);
#endif
    GOP_M_INFO("GwinNo.         CurFBId         IsShared        SharedCnt       ClientId\n");
    GOP_M_INFO("========================================================================\n");
    for (u8Index=0; u8Index<(MS_U8)g_pGOPCtxLocal->pGopChipProperty->TotalGwinNum; u8Index++)
    {
        GOP_M_INFO("%02d\t\t%04ld\t\t%02d\t\t%04d\t\t%04ld\n",u8Index,g_pGOPCtxLocal->pGOPCtxShared->gwinMap[u8Index].u32CurFBId,g_pGOPCtxLocal->pGOPCtxShared->gwinMap[u8Index].bIsShared ,g_pGOPCtxLocal->pGOPCtxShared->gwinMap[u8Index].u16SharedCnt,g_pGOPCtxLocal->pGOPCtxShared->gwinMap[u8Index].u32GOPClientId);
    }
    GOP_M_INFO("========================================================================\n");
    GOP_M_INFO("FbNo.   in_use  addr            size            widthxheight    pitch   fmt  ClientId\n");
    GOP_M_INFO("===========================================================================\n");
    for (u16Index=0; u16Index<DRV_MAX_GWIN_FB_SUPPORT; u16Index++)
    {
        pwinFB = _GetWinFB(pInstance,u16Index);
        if(pwinFB)
        {
            GOP_M_INFO("[%02d]\t%02d\t%08lx\t%08lx\t%04d x %04d\t%04d\t%02d\t%04ld\n",u16Index,pwinFB->in_use,pwinFB->addr,pwinFB->size,pwinFB->width,pwinFB->height,pwinFB->pitch,pwinFB->fbFmt,pwinFB->u32GOPClientId);
        }
    }
    GOP_M_INFO("===========================================================================\n");
}
#endif

static MS_U32 GOP_FB_Destroy(void* pInstance,MS_U32 u32fbId)
{
    GOP_WinFB_INFO* pwinFB;
#ifdef INSTANT_PRIVATE
    GOP_INSTANT_PRIVATE* psGOPInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void*)&psGOPInstPri);
#endif

    pwinFB = _GetWinFB(pInstance,u32fbId);

    if(pwinFB == NULL)
    {
        GOP_M_ERR("\n[%s] pwinFB is null! \n", __FUNCTION__);
        return GOP_API_FAIL;
    }

    switch(pwinFB->poolId)
    {
        case GOP_WINFB_POOL1:
          _GOP_FreePoolMem(pInstance,&g_pGOPCtxLocal->pGOPCtxShared->fbPool1, u32fbId);
            break;
        case GOP_WINFB_POOL2:
          _GOP_FreePoolMem(pInstance,&g_pGOPCtxLocal->pGOPCtxShared->fbPool1, u32fbId);
            break;
        default:
            break;
    }

    pwinFB->poolId = GOP_WINFB_POOL_NULL;
    pwinFB->enable = FALSE;
    pwinFB->in_use = 0;
    pwinFB->obtain = 0;
    pwinFB->x0 = 0;
    pwinFB->y0 = 0;
    pwinFB->x1 = 0;
    pwinFB->y1 = 0;

    pwinFB->gWinId = GWIN_ID_INVALID;
    pwinFB->width = 0;
    pwinFB->height= 0;
    pwinFB->pitch = 0;
    pwinFB->fbFmt =0;
    pwinFB->addr  = 0;
    pwinFB->size  = 0;
    pwinFB->dispHeight = 0;
    pwinFB->dispWidth  = 0;
    pwinFB->s_x = 0;
    pwinFB->s_y = 0;
    pwinFB->string = E_GOP_FB_NULL;
    pwinFB->u32GOPClientId = INVALID_CLIENT_PID;

    return GWIN_OK;
}

static MS_U32 GOP_SetWinInfo(void* pInstance,MS_U32 u32Gwin, GOP_GwinInfo* pinfo)
{
    MS_U8 u8GOP;

#ifdef INSTANT_PRIVATE
    GOP_INSTANT_PRIVATE* psGOPInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void*)&psGOPInstPri);
#endif

    if (!_GOP_IsGwinIdValid(pInstance,u32Gwin))
    {
        GOP_M_ERR("\n[%s] not support gwin id:%td in this chip version", __FUNCTION__, (ptrdiff_t)u32Gwin);
        return GOP_API_FAIL;
    }
    u8GOP = MDrv_DumpGopByGwinId(g_pGOPCtxLocal,u32Gwin);


    switch(u8GOP)
    {
        case E_GOP0:
            _SetGop0WinInfo(pInstance,u32Gwin, pinfo);
            break;
        case E_GOP1:
            _SetGop1WinInfo(pInstance,u32Gwin, pinfo);
            break;
        case E_GOP2:
        case E_GOP3:
        case E_GOP4:
        case E_GOP5:
            _SetGop23WinInfo(pInstance,u32Gwin, pinfo);
            break;
        default:
            break;
    }

    MDrv_GOP_GWIN_UpdateReg(g_pGOPCtxLocal,u8GOP);

    return GOP_API_SUCCESS;
}

static MS_U32 GOP_Win_Destroy(void* pInstance,MS_U32 gId)
{
    MS_U32 u32fbId;
    GOP_GwinInfo gWin;
    GOP_WinFB_INFO* pwinFB;

#ifdef INSTANT_PRIVATE
    GOP_INSTANT_PRIVATE* psGOPInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void*)&psGOPInstPri);
#endif

    GOP_M_INFO("\33[0;36m   %s:%d   gId = %td \33[m \n",__FUNCTION__,__LINE__,(ptrdiff_t)gId);

    if (!_GOP_IsGwinIdValid(pInstance,gId))
    {
        GOP_M_ERR("[%s][%d]GWIN %td  is out of range\n",__FUNCTION__,__LINE__,(ptrdiff_t)gId);
        return GOP_API_FAIL;
    }
    if (!_GOP_GWIN_IsGwinExistInClient(pInstance,gId))
    {
        GOP_M_ERR("[%s][%d]GWIN %td  is not exist\n",__FUNCTION__,__LINE__,(ptrdiff_t)gId);
        return GOP_API_FAIL;
    }

    MDrv_GOP_GWIN_EnableGwin(g_pGOPCtxLocal, gId, FALSE);
    u32fbId = g_pGOPCtxLocal->pGOPCtxShared->gwinMap[gId].u32CurFBId;

    pwinFB = _GetWinFB(pInstance,u32fbId);
    if (!_GOP_IsFbIdValid(pInstance,u32fbId))
    {
        GOP_M_ERR("[%s][%d]FbId %td  is out of range\n",__FUNCTION__,__LINE__,(ptrdiff_t)u32fbId);
        return GOP_API_FAIL;
    }

    if ((GOP_WINFB_POOL_NULL!=pwinFB->poolId) && (GOP_FB_Destroy(pInstance,u32fbId) != GWIN_OK))
    {
        GOP_M_ERR("[%s][%d], failed to delete FB %td\n", __FUNCTION__, __LINE__, (ptrdiff_t)u32fbId);

    }

    g_pGOPCtxLocal->pGOPCtxShared->gwinMap[gId].u32CurFBId= GWIN_ID_INVALID;
    g_pGOPCtxLocal->pGOPCtxShared->gwinMap[gId].u32GOPClientId = 0;
    g_pGOPCtxLocal->pGOPCtxShared->gwinMap[gId].bIsShared = FALSE;
    g_pGOPCtxLocal->pGOPCtxShared->gwinMap[gId].u16SharedCnt = 0;

    memset(&gWin, 0, sizeof(GOP_GwinInfo));
    gWin.clrType = E_GOP_COLOR_ARGB8888;

    GOP_SetWinInfo(pInstance,gId,&gWin);

    RESET_BIT(g_pGOPCtxLocal->u16GOP_VMirrorRBLK_Adr, gId);
    RESET_BIT(g_pGOPCtxLocal->u16GOP_HMirrorRBLK_Adr, gId);
    RESET_BIT(g_pGOPCtxLocal->u16GOP_HMirror_HPos, gId);
    RESET_BIT(g_pGOPCtxLocal->u16GOP_VMirror_VPos, gId);

    return GOP_API_SUCCESS;
}

#if defined(MSOS_TYPE_LINUX) || defined(MSOS_TYPE_LINUX_KERNEL)
static void GOP_AtExit(void* pInstance)
{
    MS_U32 u32TempID=0, u32GWinFBID;
    MS_U32 u32CurPID = (GETPIDTYPE)getpid();
    E_GOP_API_Result enRet = GOP_API_FAIL;
    GOP_WinFB_INFO* pwinFB;
#ifdef INSTANT_PRIVATE
    GOP_INSTANT_PRIVATE* psGOPInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void*)&psGOPInstPri);
#endif

    if(g_pGOPCtxLocal ==NULL)
    {
        return;
    }

#if GOP_LOCK_SUPPORT
    GOP_M_INFO("---%s %d: PID[%tu], TID[%td] exited, MutexCnt=%d\n", __FUNCTION__, __LINE__, (ptrdiff_t)u32CurPID, (ptrdiff_t)MsOS_GetOSThreadID(), g_pGOPCtxLocal->s32GOPMutexCnt);
#else
    GOP_M_INFO("---%s %d: PID[%tu], TID[%td] exited\n", __FUNCTION__, __LINE__, (ptrdiff_t)u32CurPID, (ptrdiff_t)MsOS_GetOSThreadID());
#endif
    for(u32TempID=0; u32TempID < g_pGOPCtxLocal->pGopChipProperty->TotalGwinNum; u32TempID++)
    {
        //Check and release APP's GWIN resource
        if(((FALSE == g_pGOPCtxLocal->pGOPCtxShared->gwinMap[u32TempID].bIsShared)
             && (DRV_MAX_GWIN_FB_SUPPORT>g_pGOPCtxLocal->pGOPCtxShared->gwinMap[u32TempID].u32CurFBId)
             && (g_pGOPCtxLocal->pGOPCtxShared->gwinMap[u32TempID].u32GOPClientId==u32CurPID))
             || ((TRUE == g_pGOPCtxLocal->pGOPCtxShared->gwinMap[u32TempID].bIsShared)
                  && (0x0 == g_pGOPCtxLocal->pGOPCtxShared->gwinMap[u32TempID].u16SharedCnt)))

        {
            u32GWinFBID = g_pGOPCtxLocal->pGOPCtxShared->gwinMap[u32TempID].u32CurFBId;
            enRet = (E_GOP_API_Result)GOP_Win_Destroy(pInstance,u32TempID);
            GOP_M_INFO("\n---%s %d: Try DestroyWIN[%tu], return[%u]\n", __FUNCTION__, __LINE__, (ptrdiff_t)u32TempID, (MS_U8)enRet);
            enRet = (E_GOP_API_Result)GOP_FB_Destroy(pInstance,u32GWinFBID);
            GOP_M_INFO("\n---%s %d: Try DestroyWIN's FB[%tu], return[%u]\n", __FUNCTION__, __LINE__, (ptrdiff_t)u32GWinFBID, (MS_U8)enRet);
        }
    }


    for(u32TempID=0; u32TempID < DRV_MAX_GWIN_FB_SUPPORT; u32TempID++)
    {
        pwinFB = _GetWinFB(pInstance,u32TempID);
        // Check and release APP1's un-maped FB resource
        // Note: If this APP1's FB is currently mapped to GWIN, means this FB is used by APP2
        if(  (pwinFB->u32GOPClientId == u32CurPID)
           &&(pwinFB->gWinId == GWIN_ID_INVALID))
        {
            enRet = (E_GOP_API_Result)GOP_Win_Destroy(pInstance,u32TempID);
            GOP_M_INFO("\n---%s %d: Try DestroyFB[%tu], return[%u]\n", __FUNCTION__, __LINE__,(ptrdiff_t)u32TempID, (MS_U8)enRet);
        }
    }

#if GOP_LOCK_SUPPORT

    if(g_pGOPCtxLocal->s32GOPMutexCnt > 0)
    {
        g_pGOPCtxLocal->s32GOPMutexCnt = 0;
        MsOS_ReleaseMutex(g_pGOPCtxLocal->s32GOPMutex);
    }
#endif

#if GOP_XC_LOCK_SUPPORT
    if(g_pGOPCtxLocal->s32GOPXC_MutexCnt > 0)
    {
        g_pGOPCtxLocal->s32GOPXC_MutexCnt = 0;
        MsOS_ReleaseMutex(g_pGOPCtxLocal->s32GOPXC_Mutex);
    }
#endif
}
#endif

static MS_BOOL GOP_PreInit_Ctx(void* pInstance,MS_BOOL *pbFirstDrvInstant)
{
   MS_BOOL bNeedInitShared =FALSE;
#ifdef INSTANT_PRIVATE
   GOP_INSTANT_PRIVATE* psGOPInstPri = NULL;
   UtopiaInstanceGetPrivate(pInstance, (void*)&psGOPInstPri);
#endif

   _GetGOPEnum(pInstance);

   if(g_pGOPCtxLocal)
   {
        if(bFirstInit)
        {
            *pbFirstDrvInstant = TRUE;
        }
        else
        {
            *pbFirstDrvInstant = FALSE;
        }

        return TRUE;
   }
   g_pGOPCtxLocal = Drv_GOP_Init_Context(pInstance,&bNeedInitShared);
   if(NULL == g_pGOPCtxLocal)
         return FALSE;
   if(pbFirstDrvInstant)
       *pbFirstDrvInstant = bNeedInitShared;

   g_pGOPCtxLocal->pGOPCtxShared->bDummyInit = bNeedInitShared;

#if 0
   if(bNeedInitShared)
   {
        MS_U32 gId;

        u16LoopTime = 0;
        g_pGOPCtxLocal->pGOPCtxShared->gwinMap = NULL;
        while (u16LoopTime<0xFF)
        {
            g_pGOPCtxLocal->pGOPCtxShared->gwinMap = malloc(MDrv_GOP_GetTotalGwinNum(g_pGOPCtxLocal)*sizeof(GWIN_MAP));
            if (g_pGOPCtxLocal->pGOPCtxShared->gwinMap!=NULL)
            {
                break;
            }
            u16LoopTime++;
        }
        if (g_pGOPCtxLocal->pGOPCtxShared->gwinMap!=NULL)
        {
            for( gId=0; gId<MApi_GOP_GWIN_GetTotalGwinNum(); gId++)
                g_pGOPCtxLocal->pGOPCtxShared->gwinMap[gId].u32CurFBId= GWIN_ID_INVALID;
        }
        else
        {
            printf("\n[%s]allocate memory for gwinMap info fail!!!!!!!", __FUNCTION__);
        }

        u16LoopTime = 0;
        g_pGOPCtxLocal->pGOPCtxShared->s32OutputColorType = NULL;
        while (u16LoopTime<0xFF)
        {
            g_pGOPCtxLocal->pGOPCtxShared->s32OutputColorType = malloc(MApi_GOP_GWIN_GetMaxGOPNum()*sizeof(MS_S32));
            if (g_pGOPCtxLocal->pGOPCtxShared->s32OutputColorType!=NULL)
            {
                break;
            }
            u16LoopTime++;
        }
        if (g_pGOPCtxLocal->pGOPCtxShared->s32OutputColorType!=NULL)
        {
            for( gId=0; gId<MApi_GOP_GWIN_GetMaxGOPNum(); gId++)
                g_pGOPCtxLocal->pGOPCtxShared->s32OutputColorType[gId] = -1;
        }
        else
        {
            printf("\n[%s]allocate memory for s32OutputColorType info fail!!!!!!!", __FUNCTION__);
        }
   }
#endif

#if defined(MSOS_TYPE_LINUX) || defined(MSOS_TYPE_LINUX_KERNEL)
   g_pGOPCtxLocal->u32GOPClientId = (GETPIDTYPE)getpid();
#else
   g_pGOPCtxLocal->u32GOPClientId = ++g_pGOPCtxLocal->pGOPCtxShared->u32ClientIdAllocator;
   if(0 == g_pGOPCtxLocal->u32GOPClientId)
       g_pGOPCtxLocal->u32GOPClientId = ++g_pGOPCtxLocal->pGOPCtxShared->u32ClientIdAllocator;
#endif

    memset(g_pGOPCtxLocal->sMirrorInfo, 0, g_pGOPCtxLocal->pGopChipProperty->TotalGwinNum * sizeof(GOP_GwinMirror_Info));

    memset(&g_pGOPCtxLocal->sGOPConfig, 0, sizeof(GOP_Config));

    memset(g_pGOPCtxLocal->MS_MuxGop, 0, MAX_GOP_MUX_SUPPORT*sizeof(MS_U8));
    g_pGOPCtxLocal->IsChgMux= FALSE;
    g_pGOPCtxLocal->IsClkClosed= FALSE;
    g_pGOPCtxLocal->u8ChgIpMuxGop = 0xFF;
#if GOP_LOCK_SUPPORT
    char GOP_MUTEX[] = "GOP_Mutex";

    g_pGOPCtxLocal->s32GOPMutexCnt = 0;
    g_pGOPCtxLocal->s32GOPMutex = MsOS_CreateMutex(E_MSOS_FIFO, (char *)GOP_MUTEX, MSOS_PROCESS_SHARED);
    if (0 > g_pGOPCtxLocal->s32GOPMutex)
    {
            GOP_M_ERR("%s %d create gop mutex fail\n", __FUNCTION__,__LINE__);
            return FALSE;
    }
    g_pGOPCtxLocal->s32GOPLock = -1;
#endif

#if GOP_XC_LOCK_SUPPORT
    char XC_MUTEX[] = "_XC_Mutex";

    g_pGOPCtxLocal->s32GOPXC_MutexCnt = 0;
    g_pGOPCtxLocal->s32GOPXC_Mutex = MsOS_CreateMutex(E_MSOS_FIFO, (char *)XC_MUTEX, MSOS_PROCESS_SHARED);
    if (0 > g_pGOPCtxLocal->s32GOPXC_Mutex)
    {
        GOP_M_INFO("[%s][%06d] create XC mutex fail _GOPXC_Mutex=0x%lx\n", __FUNCTION__, __LINE__, g_pGOPCtxLocal->s32GOPXC_Mutex);
    }
    g_pGOPCtxLocal->s32GOPXC_Lock = -1;
#endif

   return TRUE;
}


MS_U32 GOP_MapFB2Win(void* pInstance,MS_U32 u32fbId, MS_U32 GwinId)
{
    GOP_GwinInfo  gWin;
    MS_U32 u32Temp;
    GOP_WinFB_INFO* pwinFB;
    GOP_WinFB_INFO* pwinFB2;

    MS_BOOL         bAFBC_Enable=FALSE;
    MS_U8           u8GOP=0;
    DRV_GOP_AFBC_Info  sAFBCWinProperty;
    MS_U8 u8AFBC_cmd = 0;
    memset(&sAFBCWinProperty, 0, sizeof(DRV_GOP_AFBC_Info));
#ifdef INSTANT_PRIVATE
    GOP_INSTANT_PRIVATE* psGOPInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void*)&psGOPInstPri);
#endif

    if (u32fbId >= DRV_MAX_GWIN_FB_SUPPORT)
    {
        msWarning(ERR_FB_ID_OUT_OF_RANGE);
        GOP_M_ERR("[%s][%d]: fbId max already reached! FBId: %td, winId: %td\n",__FUNCTION__, __LINE__, (ptrdiff_t)u32fbId, (ptrdiff_t)GwinId);
        return (GOP_API_FAIL);
    }

    if (!_GOP_IsGwinIdValid(pInstance,GwinId))
    {
        GOP_M_ERR("\n[%s][%d] not support gwin id:%td in this chip version",__FUNCTION__, __LINE__, (ptrdiff_t)GwinId);
        return (GOP_API_FAIL);
    }
    memset(&gWin, 0, sizeof(GOP_GwinInfo));
    pwinFB = _GetWinFB(pInstance,u32fbId);

    if(pwinFB == NULL)
    {
        GOP_M_ERR("[%s][%d]GetWinFB Fail : WrongFBID=%td\n",__FUNCTION__,__LINE__,(ptrdiff_t)u32fbId);
        return (GOP_API_FAIL);
    }
    //Add support to Map a Non-Created Gwin with a dedicated FB:
    if(!_GOP_GWIN_IsGwinCreated(pInstance,GwinId))
    {
        //Create GWin for The client:
        g_pGOPCtxLocal->pGOPCtxShared->gwinMap[GwinId].u32CurFBId = u32fbId;

#if defined(MSOS_TYPE_LINUX) || defined(MSOS_TYPE_LINUX_KERNEL)
        g_pGOPCtxLocal->pGOPCtxShared->gwinMap[GwinId].u32GOPClientId= (GETPIDTYPE)getpid();
#else
        g_pGOPCtxLocal->pGOPCtxShared->gwinMap[GwinId].u32GOPClientId= g_pGOPCtxLocal->u32GOPClientId;
#endif

        MDrv_GOP_GWIN_EnableGwin(g_pGOPCtxLocal, GwinId, FALSE);

        pwinFB->enable = TRUE;
    }
    u32Temp = g_pGOPCtxLocal->pGOPCtxShared->gwinMap[GwinId].u32CurFBId;
    if (DRV_MAX_GWIN_FB_SUPPORT > u32Temp)
    {
        pwinFB2 = _GetWinFB(pInstance,u32Temp);

        if(pwinFB2 == NULL)
        {
            GOP_M_ERR("[%s][%d]GetWinFB Fail : WrongFBID=%td\n",__FUNCTION__,__LINE__,(ptrdiff_t)u32Temp);
            return (GOP_API_FAIL);
        }

        msWarning(ERR_GWIN_ID_ALREADY_MAPPED);
        GOP_M_INFO("Warning: [%s][%d]: u8gwinId already assigned to fbId2(%td), fbId(%td)\n",__FUNCTION__,__LINE__, (ptrdiff_t)u32Temp, (ptrdiff_t)u32fbId);
        pwinFB2->gWinId = GWIN_ID_INVALID;
    }

    g_pGOPCtxLocal->pGOPCtxShared->gwinMap[GwinId].u32CurFBId= u32fbId;

    if (!_GOP_GWIN_IsGwinExistInClient(pInstance,GwinId ))
    {
        msWarning(ERR_GWIN_ID_OUT_OF_RANGE);
        GOP_M_ERR("[%s][%d]: winId=%td is not in existence\n",__FUNCTION__,__LINE__,(ptrdiff_t)GwinId);
        return (GOP_API_FAIL);
    }

    pwinFB->gWinId = GwinId;

    gWin.u16DispHPixelStart = pwinFB->x0;
    gWin.u16DispVPixelStart = pwinFB->y0;
    gWin.u16RBlkHPixSize = pwinFB->width;
    gWin.u16RBlkVPixSize = pwinFB->height;
    u32Temp = _GOP_CalcPitch(pInstance,pwinFB->fbFmt, pwinFB->width);
    if(pwinFB->pitch < u32Temp)
    {
        gWin.u16RBlkHRblkSize = (MS_U16)u32Temp;//FB's pitch is invalid
    }
    else
    {
        gWin.u16RBlkHRblkSize = pwinFB->pitch;//FB's pitch is ok
    }
    gWin.u16DispHPixelEnd = pwinFB->x1;
    gWin.u16DispVPixelEnd = pwinFB->y1;
    gWin.u32DRAMRBlkStart = pwinFB->addr;
    gWin.u16WinX = gWin.u16WinY = 0;
    gWin.clrType = (EN_GOP_COLOR_TYPE)pwinFB->fbFmt;

    GOP_SetWinInfo(pInstance,GwinId,&gWin);

    u8GOP = MDrv_DumpGopByGwinId(g_pGOPCtxLocal,GwinId);
#ifdef CONFIG_GOP_AFBC_FEATURE
    if(g_pGOPCtxLocal->pGopChipProperty->bAFBC_Support[u8GOP]==TRUE)
    {
        _GOP_MAP_FBID_AFBCCmd(pInstance,u32fbId,&u8AFBC_cmd,&bAFBC_Enable);

        sAFBCWinProperty.u16HPixelStart = gWin.u16DispHPixelStart;
        sAFBCWinProperty.u16HPixelEnd = gWin.u16DispHPixelEnd;
        sAFBCWinProperty.u16Pitch = gWin.u16RBlkHRblkSize;
        sAFBCWinProperty.u16VPixelStart = gWin.u16DispVPixelStart;
        sAFBCWinProperty.u16VPixelEnd= gWin.u16DispVPixelEnd;
        sAFBCWinProperty.u64DRAMAddr = gWin.u32DRAMRBlkStart;
        sAFBCWinProperty.u8Fmt = u8AFBC_cmd;
        sAFBCWinProperty.bHMirror = g_pGOPCtxLocal->pGOPCtxShared->bHMirror;
        sAFBCWinProperty.bVMirror = g_pGOPCtxLocal->pGOPCtxShared->bVMirror;
        MDrv_GOP_GWIN_AFBCSetWindow(g_pGOPCtxLocal, u8GOP, &sAFBCWinProperty, TRUE);

        MDrv_GOP_GWIN_AFBCMode(g_pGOPCtxLocal, u8GOP, bAFBC_Enable,u8AFBC_cmd);
    }
#endif
    return GOP_API_SUCCESS;
}


MS_U32 GOP_Set_Hscale(void* pInstance,MS_U8 u8GOP,MS_BOOL bEnable, MS_U16 src, MS_U16 dst)
{

#ifdef INSTANT_PRIVATE
    GOP_INSTANT_PRIVATE* psGOPInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void*)&psGOPInstPri);
#endif

    if(!g_pGOPCtxLocal->pGopChipProperty->bPixelModeSupport)
    {
        if (src > dst)
        {
            GOP_M_ERR("[%s][%d]GOP can not scaling down\n",__FUNCTION__,__LINE__);
            return GOP_API_FAIL;
        }
    }

    if (bEnable)
    {
        g_pGOPCtxLocal->pGOPCtxShared->u16HScaleSrc[u8GOP] = src;
        g_pGOPCtxLocal->pGOPCtxShared->u16HScaleDst[u8GOP] = dst;
    }
    else
    {
        g_pGOPCtxLocal->pGOPCtxShared->u16HScaleSrc[u8GOP] = src;
        g_pGOPCtxLocal->pGOPCtxShared->u16HScaleDst[u8GOP] = 0;
    }
    MDrv_GOP_GWIN_Set_HSCALE(g_pGOPCtxLocal,u8GOP, bEnable, src, dst);
    MDrv_GOP_GWIN_UpdateReg(g_pGOPCtxLocal,u8GOP);
    return GOP_API_SUCCESS;
}

MS_U32 GOP_Set_Vscale(void* pInstance,MS_U8 u8GOP,MS_BOOL bEnable, MS_U16 src, MS_U16 dst)
{

#ifdef INSTANT_PRIVATE
    GOP_INSTANT_PRIVATE* psGOPInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void*)&psGOPInstPri);
#endif

	if(!g_pGOPCtxLocal->pGopChipProperty->bPixelModeSupport)
    {
        if (src > dst)
        {
            GOP_M_ERR("[%s][%d]GOP can not scaling down\n",__FUNCTION__,__LINE__);
            return GOP_API_FAIL;
        }
    }

    if (bEnable)
    {
        g_pGOPCtxLocal->pGOPCtxShared->u16VScaleDst[u8GOP] = dst;
    }
    else
    {
        g_pGOPCtxLocal->pGOPCtxShared->u16VScaleDst[u8GOP] = 0;
    }
    MDrv_GOP_GWIN_Set_VSCALE(g_pGOPCtxLocal, u8GOP,bEnable, src, dst);
    MDrv_GOP_GWIN_UpdateReg(g_pGOPCtxLocal,u8GOP);
    return GOP_API_SUCCESS;

}

MS_U32 GOP_SetStretchWin(void* pInstance,MS_U8 u8GOP_num, EN_GOP_DST_TYPE eDstType,MS_U16 x, MS_U16 y, MS_U16 width, MS_U16 height)
{
    MS_U16 u16OrgWidth = 0, u16Temp = 0;
    MS_U16 u16PnlHStr = 0, u16PnlVStr = 0;

#ifdef INSTANT_PRIVATE
    GOP_INSTANT_PRIVATE* psGOPInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void*)&psGOPInstPri);
#endif

    MS_U16 u16StrenthWidth = g_pGOPCtxLocal->pGOPCtxShared->u16HScaleDst[u8GOP_num];
    MS_U16 u16StrenthHeight = g_pGOPCtxLocal->pGOPCtxShared->u16VScaleDst[u8GOP_num];
    MS_U16 u16PnlWidth = g_pGOPCtxLocal->pGOPCtxShared->u16PnlWidth[u8GOP_num];
    MS_U16 u16PnlHeight = g_pGOPCtxLocal->pGOPCtxShared->u16PnlHeight[u8GOP_num];

    if (!_GOP_IsGopNumValid(pInstance,u8GOP_num))
    {
        GOP_M_ERR("[%s][%d]GOP %d  is out of range\n",__FUNCTION__,__LINE__,u8GOP_num);
        return GOP_API_FAIL;
    }

    //Get real original hw Stretch win width
    _GOP_Get_StretchWin(pInstance,u8GOP_num, &u16Temp, &u16Temp, &u16OrgWidth, &u16Temp);
    _GOP_GWIN_Align_StretchWin(pInstance, u8GOP_num, eDstType, &x, &y, &width, &height, 16);
    _GOP_GWIN_Align_StretchWin(pInstance, u8GOP_num, eDstType, &u16PnlHStr, &u16PnlVStr, &u16Temp, &u16PnlHeight, 16);
    //Store API use stretch window
    g_pGOPCtxLocal->pGOPCtxShared->u16APIStretchWidth[u8GOP_num] = width;
    g_pGOPCtxLocal->pGOPCtxShared->u16APIStretchHeight[u8GOP_num]= height;

    _GOP_GWIN_Adjust_3DStretchWin(pInstance,u8GOP_num, &x, &y, &width, &height);
    if (u16OrgWidth <= width)//Set a large one, set htt first
    {
        MDrv_GOP_GWIN_SetHTotal(g_pGOPCtxLocal,u8GOP_num ,width);
    }

    if(g_pGOPCtxLocal->pGOPCtxShared->bHMirror && (u16StrenthWidth < u16PnlWidth))
    {
        if(x + u16StrenthWidth > u16PnlWidth)
            x = 0;
        else
            x = u16PnlWidth - x - ((u16StrenthWidth !=0) ? u16StrenthWidth : width);
    }

    if(g_pGOPCtxLocal->pGOPCtxShared->bVMirror && (u16StrenthHeight < u16PnlHeight))
    {
        if(y + u16StrenthHeight > u16PnlHeight)
            y = 0;
        else
            y = u16PnlHeight - y - ((u16StrenthHeight !=0) ? u16StrenthHeight : height);
    }

    MDrv_GOP_GWIN_SetStretchWin(g_pGOPCtxLocal, u8GOP_num, x, y, width, height);
    _GOP_InitHSPDByGOP(pInstance,u8GOP_num);
    MDrv_GOP_GWIN_UpdateReg(g_pGOPCtxLocal,u8GOP_num);

    if (u16OrgWidth > width)//Set a small one, set htt after Gwin regsiter updated
    {
        MDrv_GOP_GWIN_SetHTotal(g_pGOPCtxLocal,u8GOP_num ,width);
    }

    return GOP_API_SUCCESS;
}

MS_U32 GOP_SetAlphaInverse(void* pInstance,MS_U32 gop, MS_BOOL bEn)
{
#ifdef INSTANT_PRIVATE
    GOP_INSTANT_PRIVATE* psGOPInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void*)&psGOPInstPri);
#endif

    MDrv_GOP_GWIN_SetAlphaInverse(g_pGOPCtxLocal, gop, bEn);
    return GOP_API_SUCCESS;
}

MS_U32 GOP_SetContrast(void* pInstance,MS_U8 u8GOP, MS_U16 u16YContrast, MS_U16 u16UContrast, MS_U16 u16VContrast )
{
    E_GOP_API_Result enRet = GOP_API_FAIL;

#ifdef INSTANT_PRIVATE
    GOP_INSTANT_PRIVATE* psGOPInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void*)&psGOPInstPri);
#endif

    enRet = (E_GOP_API_Result)MDrv_GOP_SetGOPContrast( g_pGOPCtxLocal, u8GOP, u16YContrast, u16UContrast, u16VContrast );

    return enRet;
}

MS_U32 GOP_GetContrast(void* pInstance,MS_U8 u8GOP, MS_U32* YContrast, MS_U32* UContrast, MS_U32* VContrast )
{
    E_GOP_API_Result enRet= GOP_API_FAIL;

#ifdef INSTANT_PRIVATE
    GOP_INSTANT_PRIVATE* psGOPInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void*)&psGOPInstPri);
#endif

    enRet = (E_GOP_API_Result)MDrv_GOP_GetGOPContrast( g_pGOPCtxLocal, u8GOP, YContrast, UContrast, VContrast );
    return enRet;
}



MS_U32 GOP_SetBrightness(void* pInstance,MS_U8 u8GOP,MS_U16 u16BriVal,MS_BOOL bMSB)
{
    E_GOP_API_Result enRet=GOP_API_FAIL;

#ifdef INSTANT_PRIVATE
    GOP_INSTANT_PRIVATE* psGOPInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void*)&psGOPInstPri);
#endif

    enRet = (E_GOP_API_Result)MDrv_GOP_SetGOPBrightness(g_pGOPCtxLocal, u8GOP, u16BriVal,bMSB);

    return enRet;
}

MS_U32 GOP_GetBrightness(void* pInstance,MS_U8 u8GOP,MS_U16* u16BriVal,MS_BOOL* bMSB)
{
    E_GOP_API_Result enRet=GOP_API_FAIL;

#ifdef INSTANT_PRIVATE
    GOP_INSTANT_PRIVATE* psGOPInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void*)&psGOPInstPri);
#endif

    enRet =(E_GOP_API_Result)MDrv_GOP_GetGOPBrightness(g_pGOPCtxLocal,u8GOP,u16BriVal,bMSB);

    return enRet;
}


MS_U32 GOP_SetOutputColor(void* pInstance,MS_U8 u8GOP, EN_GOP_OUTPUT_COLOR type)
{

#ifdef INSTANT_PRIVATE
    GOP_INSTANT_PRIVATE* psGOPInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void*)&psGOPInstPri);
#endif

    g_pGOPCtxLocal->pGOPCtxShared->s32OutputColorType[u8GOP] = (MS_S32)type;
    MDrv_GOP_GWIN_OutputColor(g_pGOPCtxLocal,u8GOP, (GOP_OupputColor)type);

    return GOP_API_SUCCESS;
}

MS_U32 GOP_SetUVSwap(void* pInstance,MS_U8 u8GOP, MS_BOOL bEn)
{

#ifdef INSTANT_PRIVATE
    GOP_INSTANT_PRIVATE* psGOPInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void*)&psGOPInstPri);
#endif

    if (!_GOP_IsGopNumValid(pInstance,u8GOP))
    {
        GOP_M_ERR("[%s][%d]GOP %d  is out of range\n",__FUNCTION__,__LINE__,u8GOP);
        return GOP_API_FAIL;
    }

    MDrv_GOP_SetUVSwap(g_pGOPCtxLocal, u8GOP, bEn);
    MDrv_GOP_GWIN_UpdateReg(g_pGOPCtxLocal,u8GOP);

    return GOP_API_SUCCESS;

}


MS_U32 GOP_SetYCSwap(void* pInstance,MS_U8 u8GOP, MS_BOOL bEn)
{

#ifdef INSTANT_PRIVATE
    GOP_INSTANT_PRIVATE* psGOPInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void*)&psGOPInstPri);
#endif

    if (!_GOP_IsGopNumValid(pInstance,u8GOP))
    {
        GOP_M_ERR("[%s][%d]GOP %d  is out of range\n",__FUNCTION__,__LINE__,u8GOP);
        return GOP_API_FAIL;
    }


    MDrv_GOP_SetYCSwap(g_pGOPCtxLocal, u8GOP, bEn);
    MDrv_GOP_GWIN_UpdateReg(g_pGOPCtxLocal,u8GOP);

    return GOP_API_SUCCESS;

}

MS_U32 GOP_SetProgressive(void* pInstance,MS_U8 gop,MS_BOOL bEn)
{
    MS_U8 u8WinIdx = 0x0;
    MS_U8 u8GopIdx = 0x0;
    MS_U8 u8StartWin = 0x0;
    MS_U8 u8EndWin = 0x0;
    MS_U8 i = 0x0;
    MS_U8 u8GOPGwinMaxCount = 0x0;
    MS_U16 u16StretchX = 0x0;
    MS_U16 u16StretchY = 0x0;
    MS_U16 u16StretchW = 0x0;
    MS_U16 u16StretchH = 0x0;
    GOP_WinFB_INFO* pwinFB;
    DRV_GOPDstType enDst = E_DRV_GOP_DST_INVALID;
    EN_GOP_DST_TYPE GopDst = E_GOP_DST_OP0;
#ifdef INSTANT_PRIVATE
    GOP_INSTANT_PRIVATE* psGOPInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void*)&psGOPInstPri);
#endif

    for (i=0; i < MDrv_GOP_GetMaxGOPNum(g_pGOPCtxLocal); i++)
    {
        if(u8GOPGwinMaxCount<MDrv_GOP_GetGwinNum(pInstance, i))
            u8GOPGwinMaxCount=MDrv_GOP_GetGwinNum(pInstance, i);
    }
    GOP_GwinInfo winInfo[u8GOPGwinMaxCount];
    MS_BOOL bGwinFBInit[u8GOPGwinMaxCount];
    memset(winInfo,0,sizeof(GOP_GwinInfo)*u8GOPGwinMaxCount);
    memset(bGwinFBInit,0,sizeof(MS_BOOL)*u8GOPGwinMaxCount);

    for (u8GopIdx = 0; u8GopIdx < gop; u8GopIdx++)
    {
        u8StartWin += MDrv_GOP_GetGwinNum(pInstance, u8GopIdx);
    }
    u8EndWin = u8StartWin + MDrv_GOP_GetGwinNum(pInstance, gop);

    if((u8EndWin-u8StartWin)>u8GOPGwinMaxCount)
    {
        GOP_M_ERR("[%s][%d]GWINCount %d  is out of range\n",__FUNCTION__,__LINE__,u8EndWin-u8StartWin);
        return GOP_API_FAIL;
    }

    for (i=0,u8WinIdx=u8StartWin; u8WinIdx<u8EndWin; u8WinIdx++,i++)
    {
        if (!_GOP_IsFbIdValid(pInstance,g_pGOPCtxLocal->pGOPCtxShared->gwinMap[u8WinIdx].u32CurFBId))
        {
            continue;
        }
        pwinFB = _GetWinFB(pInstance,g_pGOPCtxLocal->pGOPCtxShared->gwinMap[u8WinIdx].u32CurFBId);
        if(pwinFB == NULL)
            bGwinFBInit[i]=FALSE;
        else
            bGwinFBInit[i]=TRUE;
    }

    for (i=0,u8WinIdx=u8StartWin; u8WinIdx<u8EndWin; u8WinIdx++,i++)
    {
        if(bGwinFBInit[i])
            GOP_GetWinInfo(pInstance, u8WinIdx, &winInfo[i]);
    }
    _GOP_Get_StretchWin(pInstance, gop, &u16StretchX, &u16StretchY, &u16StretchW, &u16StretchH);

    MDrv_GOP_GWIN_EnableProgressive(g_pGOPCtxLocal, gop, bEn);

    for (i=0,u8WinIdx=u8StartWin; u8WinIdx<u8EndWin; u8WinIdx++,i++)
    {
        if(bGwinFBInit[i])
            GOP_SetWinInfo(pInstance, u8WinIdx, &winInfo[i]);
    }

    MDrv_GOP_GWIN_GetDstPlane(g_pGOPCtxLocal, gop, &enDst);
    _GOP_Map_DRVDst2API_Enum_(pInstance,&GopDst, enDst);
    GOP_SetStretchWin(pInstance, gop, GopDst, u16StretchX, u16StretchY, u16StretchW, u16StretchH);
    MDrv_GOP_GWIN_UpdateReg(g_pGOPCtxLocal,gop);
    return GOP_API_SUCCESS;
}

MS_U32 GOP_SetHStart(void* pInstance,MS_U8 u8GOP,MS_U16 u16PanelHStr)
{

#ifdef INSTANT_PRIVATE
    GOP_INSTANT_PRIVATE* psGOPInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void*)&psGOPInstPri);
#endif

    if (!_GOP_IsGopNumValid(pInstance,u8GOP))
    {
        GOP_M_ERR("[%s][%d]GOP %d  is out of range\n",__FUNCTION__,__LINE__,u8GOP);
        return GOP_API_FAIL;
    }

    g_pGOPCtxLocal->pGOPCtxShared->u16PnlHStr[u8GOP] = u16PanelHStr;
    _GOP_InitHSPDByGOP(pInstance,u8GOP);
    MDrv_GOP_GWIN_UpdateReg(g_pGOPCtxLocal, u8GOP);
    return GOP_API_SUCCESS;

}

MS_U32 GOP_SetPanelTiming(void* pInstance,MS_U8 u8GOP,PST_GOP_TIMING_INFO pstInfo)
{
#ifdef INSTANT_PRIVATE
    GOP_INSTANT_PRIVATE* psGOPInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void*)&psGOPInstPri);
#endif

    if (!_GOP_IsGopNumValid(pInstance,u8GOP))
    {
        GOP_M_ERR("[%s][%d]GOP %d  is out of range\n",__FUNCTION__,__LINE__,u8GOP);
        return GOP_API_FAIL;
    }

    g_pGOPCtxLocal->pGOPCtxShared->u16PnlHStr[u8GOP] = pstInfo->u16DEHStart;
    g_pGOPCtxLocal->pGOPCtxShared->u16PnlWidth[u8GOP] = pstInfo->u16DEHSize;
    g_pGOPCtxLocal->pGOPCtxShared->u16PnlHeight[u8GOP] = pstInfo->u16DEVSize;
    _GOP_InitHSPDByGOP(pInstance,u8GOP);
    return GOP_API_SUCCESS;
}

MS_U32 GOP_GetPanelTiming(void* pInstance,MS_U8 u8GOP,PST_GOP_TIMING_INFO pstInfo)
{
#ifdef INSTANT_PRIVATE
    GOP_INSTANT_PRIVATE* psGOPInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void*)&psGOPInstPri);
#endif

    if (!_GOP_IsGopNumValid(pInstance,u8GOP))
    {
        GOP_M_ERR("[%s][%d]GOP %d  is out of range\n",__FUNCTION__,__LINE__,u8GOP);
        return GOP_API_FAIL;
    }

    pstInfo->u16DEHStart = g_pGOPCtxLocal->pGOPCtxShared->u16PnlHStr[u8GOP];
    pstInfo->u16DEHSize = g_pGOPCtxLocal->pGOPCtxShared->u16PnlWidth[u8GOP];
    pstInfo->u16DEVSize = g_pGOPCtxLocal->pGOPCtxShared->u16PnlHeight[u8GOP];
    return GOP_API_SUCCESS;
}

MS_U32 GOP_CreateBufferByAddr(void* pInstance,PGOP_BUFFER_INFO  pBuff,MS_U32 u32fbId)
{
    MS_U32 u32Size =0;
    GOP_WinFB_INFO* pstWinFB = NULL;
    MS_U32 u32Width =0;
    MS_U32 u32Height =0;
    MS_U32 u32Pitch =0;
    MS_U32 u32FbFmt =0;
    MS_PHY phyFbAddr =0;
    MS_U32 u32DispX,u32DispY =0;
    EN_GOP_FRAMEBUFFER_STRING enFBString = E_GOP_FB_NULL;

    if (!_GOP_IsFbIdValid(pInstance,u32fbId))
    {
        GOP_M_ERR("[%s][%d]FbId %td  is out of range\n",__FUNCTION__,__LINE__,(ptrdiff_t)u32fbId);
        return GWIN_NO_AVAILABLE;
    }

    pstWinFB = _GetWinFB(pInstance,u32fbId);

    if(pstWinFB == NULL)
    {
        GOP_M_ERR("[%s][%d]GetWinFB Fail : WrongFBID=%td\n",__FUNCTION__,__LINE__,(ptrdiff_t)u32fbId);
        return GWIN_NO_AVAILABLE;
    }

    if(pstWinFB->in_use)
    {
        msWarning(ERR_FB_ID_ALREADY_ALLOCATED);
        GOP_M_ERR("[%s][%d]FbId %td is already allocated\n",__FUNCTION__,__LINE__,(ptrdiff_t)u32fbId);
        return GWIN_NO_AVAILABLE;

    }

    GOP_M_INFO("[%s][%d]: new gwinFBId = %td \n",__FUNCTION__,__LINE__, (ptrdiff_t)u32fbId);


    u32Width     = pBuff->width;
    u32Height    = pBuff->height;
    u32Pitch     = pBuff->pitch;
    u32FbFmt     = pBuff->fbFmt;
    u32DispX     = pBuff->disp_rect.x;
    u32DispY     = pBuff->disp_rect.y;
    phyFbAddr = pBuff->addr;
    enFBString  = pBuff->FBString;

    u32Size =  _GOP_CalcPitch(pInstance,u32FbFmt, u32Width) *u32Height;

    pstWinFB->enable = FALSE;
    pstWinFB->in_use = 1;
    pstWinFB->obtain = 1;
    pstWinFB->x0 = u32DispX;
    pstWinFB->y0 = u32DispY;
    pstWinFB->width = u32Width;
    pstWinFB->height= u32Height;
    pstWinFB->x1 = pstWinFB->x0 + u32Width;
    pstWinFB->y1 = pstWinFB->y0 +u32Height;
    pstWinFB->addr  = phyFbAddr; //addr;
    pstWinFB->size  = u32Size;
    pstWinFB->FBString  = (EN_DRV_FRAMEBUFFER_STRING)enFBString;

    // FB format
    pstWinFB->fbFmt  = (u32FbFmt != FB_FMT_AS_DEFAULT) ? u32FbFmt : E_MS_FMT_ARGB1555;//g_pGOPCtxLocal->winFB[g_pGOPCtxLocal->pGOPCtxShared->gwinMap[GWIN_OSD_DEFAULT]].fbFmt;
    if(u32Pitch ==0)
    {
        pstWinFB->pitch = _GOP_CalcPitch(pInstance,u32FbFmt, u32Width);
    }
    else
    {
        pstWinFB->pitch = u32Pitch;
    }
    // update ptrs
    pstWinFB->poolId = GOP_WINFB_POOL_NULL;
    pstWinFB->nextFBIdInPool = INVALID_POOL_NEXT_FBID;
    pstWinFB->string = E_GOP_FB_NULL;
    pstWinFB->u32GOPClientId = (GETPIDTYPE)getpid();
    GOP_M_INFO("\33[0;36m   %s:%d   FBId = %td \33[m \n",__FUNCTION__,__LINE__, (ptrdiff_t)u32fbId);

    return GOP_API_SUCCESS;
}


MS_U32 GOP_Win_Destroy_Abnormal(void* pInstance,MS_U32 gId)
{
    MS_U32 u32fbId;
    GOP_GwinInfo gWin;
    GOP_WinFB_INFO* pwinFB;

#ifdef INSTANT_PRIVATE
    GOP_INSTANT_PRIVATE* psGOPInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void*)&psGOPInstPri);
#endif

    GOP_M_INFO("\33[0;36m   %s:%d   gId = %td \33[m \n",__FUNCTION__,__LINE__,(ptrdiff_t)gId);

    if (!_GOP_IsGwinIdValid(pInstance,gId))
    {
        GOP_M_ERR("[%s][%d]GWIN %td  is out of range\n",__FUNCTION__,__LINE__,(ptrdiff_t)gId);
        return GOP_API_FAIL;
    }

    MDrv_GOP_GWIN_EnableGwin(g_pGOPCtxLocal, gId, FALSE);
    u32fbId = g_pGOPCtxLocal->pGOPCtxShared->gwinMap[gId].u32CurFBId;

    pwinFB = _GetWinFB(pInstance,u32fbId);
    if (!_GOP_IsFbIdValid(pInstance,u32fbId))
    {
        GOP_M_ERR("[%s][%d]FbId %td  is out of range\n",__FUNCTION__,__LINE__,(ptrdiff_t)u32fbId);
        return GOP_API_FAIL;
    }

    if ((GOP_WINFB_POOL_NULL!=pwinFB->poolId) && (GOP_FB_Destroy(pInstance,u32fbId) != GWIN_OK))
    {
        GOP_M_INFO("[%s][%d], failed to delete FB %td\n", __FUNCTION__, __LINE__, (ptrdiff_t)u32fbId);

    }

    g_pGOPCtxLocal->pGOPCtxShared->gwinMap[gId].u32CurFBId= GWIN_ID_INVALID;
    g_pGOPCtxLocal->pGOPCtxShared->gwinMap[gId].u32GOPClientId = 0;
    g_pGOPCtxLocal->pGOPCtxShared->gwinMap[gId].bIsShared = FALSE;
    g_pGOPCtxLocal->pGOPCtxShared->gwinMap[gId].u16SharedCnt = 0;

    memset(&gWin, 0, sizeof(GOP_GwinInfo));
    gWin.clrType = E_GOP_COLOR_ARGB8888;

    GOP_SetWinInfo(pInstance,gId,&gWin);

    RESET_BIT(g_pGOPCtxLocal->u16GOP_VMirrorRBLK_Adr, gId);
    RESET_BIT(g_pGOPCtxLocal->u16GOP_HMirrorRBLK_Adr, gId);
    RESET_BIT(g_pGOPCtxLocal->u16GOP_HMirror_HPos, gId);
    RESET_BIT(g_pGOPCtxLocal->u16GOP_VMirror_VPos, gId);

    return GOP_API_SUCCESS;

}


void GOP_ResetGOP(void* pInstance,MS_U8 u8GOP)
{
    MS_U32 i = 0,j = 0;
    MS_U8 u8GwinId;
    GOP_WinFB_INFO* pwinFB;
    E_GOP_API_Result enRet = GOP_API_FAIL;

#ifdef INSTANT_PRIVATE
        GOP_INSTANT_PRIVATE* psGOPInstPri = NULL;
        UtopiaInstanceGetPrivate(pInstance, (void*)&psGOPInstPri);
#endif

    if(g_pGOPCtxLocal ==NULL)
    {
        MS_BOOL bFirstInstance;
        GOP_PreInit_Ctx(pInstance, &bFirstInstance);
        if (MDrv_GOP_SetIOMapBase(g_pGOPCtxLocal) != TRUE)
        {
            APIGOP_ASSERT(FALSE, GOP_M_FATAL("\nget IO base fail"));
            return ;
        }
    }

    u8GwinId = _GOP_SelGwinId2(pInstance,u8GOP, 0);
	if (u8GwinId == GWIN_NO_AVAILABLE) {
		GOP_M_ERR("[%s][%d]Error win ID=%d\n",__FUNCTION__,__LINE__,u8GwinId);
		return;
	}
    for(i = u8GwinId; i < u8GwinId + MDrv_GOP_GetGwinNum(g_pGOPCtxLocal,u8GOP); i++)
    {
        //Check and release APP's GWIN resource
        if(((FALSE == g_pGOPCtxLocal->pGOPCtxShared->gwinMap[i].bIsShared)
             && (DRV_MAX_GWIN_FB_SUPPORT>g_pGOPCtxLocal->pGOPCtxShared->gwinMap[i].u32CurFBId))
             || ((TRUE == g_pGOPCtxLocal->pGOPCtxShared->gwinMap[i].bIsShared)
                  && (0x0 == g_pGOPCtxLocal->pGOPCtxShared->gwinMap[i].u16SharedCnt)))

        {
            for(j=0; j < DRV_MAX_GWIN_FB_SUPPORT; j++)
            {
                pwinFB = _GetWinFB(pInstance,j);
                if(pwinFB != NULL)
                {
                    if ((pwinFB->u32GOPClientId == g_pGOPCtxLocal->pGOPCtxShared->gwinMap[i].u32GOPClientId)
                 && (g_pGOPCtxLocal->pGOPCtxShared->gwinMap[i].u32CurFBId == j))
                    {
                        enRet = (E_GOP_API_Result)GOP_FB_Destroy(pInstance,j);
                        GOP_M_INFO("\n---%s %d: Try DestroyWIN's FB[%tu], return[%u]\n", __FUNCTION__, __LINE__, (ptrdiff_t)j, (MS_U8)enRet);
                    }
                }
            }
            if (g_pGOPCtxLocal->pGOPCtxShared->gwinMap[i].u32GOPClientId != 0)
            {
                enRet = (E_GOP_API_Result)GOP_Win_Destroy_Abnormal(pInstance,i);
                GOP_M_INFO("\n---%s %d: Try DestroyWIN[%tu], return[%u]\n", __FUNCTION__, __LINE__,(ptrdiff_t) i, (MS_U8)enRet);
            }
        }
    }
}

static void GOP_GWIN_InitByGOP(void* pInstance,MS_BOOL bFirstIntance, MS_U8 u8GOP)
{
    MS_U8 u8Index = 0;
    MS_U8 u8BytePerPix = 2;
    MS_U16 u16Width=0, u16Height=0, u16Pitch=0, u16FbFmt=0;
    MS_PHY phyAddr=0;
    MS_U32 u32Index =0;
    GOP_GwinInfo stGwin;
    GOP_WinFB_INFO* pstWinFB=NULL;
    MS_U32 u32MaxGwinSupport=0;

#ifdef INSTANT_PRIVATE
    GOP_INSTANT_PRIVATE* psGOPInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void*)&psGOPInstPri);
#endif

    //GE_BufInfo buffinfo;
    if (!_GOP_IsGopNumValid(pInstance,u8GOP))
    {
        GOP_M_ERR("[%s][%d]GOP %d  is out of range\n",__FUNCTION__,__LINE__,u8GOP);
        return;
    }
    GOP_M_INFO("MDrv_GOP_GWIN_Init\n");
    memset(&stGwin, 0, sizeof(GOP_GwinInfo));
    if(g_pGOPCtxLocal->pGOPCtxShared->bInitShared)
    {
        g_pGOPCtxLocal->pGOPCtxShared->bInitShared = FALSE;
        //First init of whole GOP Driver
        for (u32Index = 0; u32Index <DRV_MAX_GWIN_FB_SUPPORT; u32Index++)
        {
            pstWinFB = _GetWinFB(pInstance,u32Index);

            if(pstWinFB == NULL)
            {
                GOP_M_ERR("[%s][%d]GetWinFB Fail : WrongFBID=%td\n",__FUNCTION__,__LINE__,(ptrdiff_t)u32Index);
                return;
            }

            pstWinFB->enable = FALSE;
            pstWinFB->in_use = 0;
            pstWinFB->obtain = 0;
            pstWinFB->gWinId = GWIN_ID_INVALID; // orig GWIN_OSD_DEFAULT;
            pstWinFB->x0 = stGwin.u16DispHPixelStart = 0;
            pstWinFB->y0 = stGwin.u16DispVPixelStart = 0;
            pstWinFB->width = stGwin.u16RBlkHPixSize = 0;
            pstWinFB->height= stGwin.u16RBlkVPixSize = 0;
            pstWinFB->x1 = stGwin.u16DispHPixelEnd = 0;
            pstWinFB->y1 = stGwin.u16DispVPixelEnd = 0;
            pstWinFB->pitch  = 0;
            pstWinFB->addr  = stGwin.u32DRAMRBlkStart = 0;
            pstWinFB->size   = 0;
            stGwin.u16WinX = stGwin.u16WinY = 0;
            // FB format
            pstWinFB->fbFmt  = FB_FMT_AS_DEFAULT;
            // update ptrs
            pstWinFB->poolId = GOP_WINFB_POOL_NULL;
            pstWinFB->nextFBIdInPool = INVALID_POOL_NEXT_FBID;
            pstWinFB->string = E_GOP_FB_NULL;
            pstWinFB->u32GOPClientId = INVALID_CLIENT_PID;
        }
    }
    if(FALSE ==g_pGOPCtxLocal->pGOPCtxShared->bGopHasInitialized[u8GOP])
    {
        memset(&stGwin, 0, sizeof(GOP_GwinInfo));
        for (u8Index = 0; u8Index<MDrv_GOP_GetGwinNum(g_pGOPCtxLocal,u8GOP); u8Index++)
        {
            u32Index = _GOP_SelGwinId2(pInstance,u8GOP, u8Index);
		if (u32Index == GWIN_NO_AVAILABLE) {
			GOP_M_ERR("[%s][%d]Error win ID=%d\n",__FUNCTION__,__LINE__,u32Index);
			return;
		}
            g_pGOPCtxLocal->pGOPCtxShared->gwinMap[u32Index].u32CurFBId= GWIN_ID_INVALID;
            stGwin.clrType = (EN_GOP_COLOR_TYPE)E_GOP_COLOR_ARGB1555;
            //Disable all Gwin when GOP first init
            if(!(g_pGOPCtxLocal->sGOPConfig.eGopIgnoreInit & E_GOP_IGNORE_GWIN))
            {
                    //GWIN must be turn off in Vsync to avoid garbage
                    MDrv_GOP_GWIN_ForceWrite_Update(g_pGOPCtxLocal, u8GOP, FALSE);
                    MDrv_GOP_GWIN_EnableGwin(g_pGOPCtxLocal, u32Index, FALSE);
                    MDrv_GOP_GWIN_ForceWrite_Update(g_pGOPCtxLocal, u8GOP, TRUE);
                GOP_SetWinInfo(pInstance,u32Index, &stGwin);
                MDrv_GOP_GWIN_SetBlending(g_pGOPCtxLocal, u32Index, FALSE, 0x0);
            }
        }
    }

    pstWinFB = _GetWinFB(pInstance,DRV_MAX_GWIN_FB_SUPPORT);
    u32MaxGwinSupport = g_pGOPCtxLocal->pGopChipProperty->TotalGwinNum;

    if(pstWinFB == NULL)
    {
        GOP_M_ERR("[%s][%d]GetWinFB Fail : WrongFBID=%ld\n",__FUNCTION__,__LINE__,DRV_MAX_GWIN_FB_SUPPORT);
        return;
    }

    pstWinFB->enable = TRUE;
    pstWinFB->in_use = 1;
    pstWinFB->obtain = 1;
    pstWinFB->gWinId = u32MaxGwinSupport;
    pstWinFB->x0 = 0;
    pstWinFB->y0 = 0;
    pstWinFB->width = u16Width;
    pstWinFB->height = u16Height;
    pstWinFB->x1  = pstWinFB->x0 + pstWinFB->width;
    pstWinFB->y1  = pstWinFB->y0 + pstWinFB->height;
    pstWinFB->pitch = (u16Pitch & 0xFFF8);  // pitch must be 4-pix alignment;
    pstWinFB->addr   = phyAddr;
    pstWinFB->size  =  ((MS_U32)u16Width) * ((MS_U32)u16Height) * ((MS_U32)u8BytePerPix);
    // FB format
    pstWinFB->fbFmt  = u16FbFmt;
    // update ptrs
    pstWinFB->poolId = GOP_WINFB_POOL_NULL;
    pstWinFB->nextFBIdInPool = INVALID_POOL_NEXT_FBID;
    pstWinFB->string = E_GOP_FB_NULL;
    pstWinFB->u32GOPClientId = INVALID_CLIENT_PID;

    g_pGOPCtxLocal->pGOPCtxShared->gwinMap[u32MaxGwinSupport].u32CurFBId= DRV_MAX_GWIN_FB_SUPPORT;

}


MS_U32 GOP_GWIN_OutputLayerSwitch(void* pInstance,MS_U8 u8GOP)
{
    E_GOP_API_Result GOPRet;


//#ifndef MSOS_TYPE_LINUX
    MS_U8 i;

#ifdef INSTANT_PRIVATE
    GOP_INSTANT_PRIVATE* psGOPInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void*)&psGOPInstPri);
#endif

    if (!_GOP_IsGopNumValid(pInstance,u8GOP))
    {
        GOP_M_ERR("[%s][%d]GOP %d  is out of range\n",__FUNCTION__,__LINE__,u8GOP);
        return GOP_API_FAIL;
    }

    GOPRet = (E_GOP_API_Result)MDrv_GOP_SetGOPHighPri(g_pGOPCtxLocal, u8GOP);
    for (i=0; i<_GOP_GetMaxGOPNum(pInstance); i++)
    {
        _GOP_InitHSPDByGOP(pInstance,i);
    }

    return GOPRet;
}
MS_U32 GOP_GetCurrentGOP(void* pInstance)
{
#ifdef INSTANT_PRIVATE
    GOP_INSTANT_PRIVATE* psGOPInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void*)&psGOPInstPri);
#endif

    return MDrv_GOP_Get(g_pGOPCtxLocal);
}

MS_U32 GOP_GetCurrentGWin(void* pInstance)
{
#ifdef INSTANT_PRIVATE
    GOP_INSTANT_PRIVATE* psGOPInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void*)&psGOPInstPri);
#endif

    return g_pGOPCtxLocal->current_winId;
}

MS_U32 GOP_GetCurrentFBID(void* pInstance)
{
    GOP_WinFB_INFO* pwinFB;

#ifdef INSTANT_PRIVATE
    GOP_INSTANT_PRIVATE* psGOPInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void*)&psGOPInstPri);
#endif

    MS_U8 curWinId = GOP_GetCurrentGWin(pInstance);
    if (!_GOP_GWIN_IsGwinExistInClient(pInstance,curWinId))
    {
        GOP_M_ERR("[%s][%d]GWIN %d  is not exist\n",__FUNCTION__,__LINE__,curWinId);
        return GWIN_ID_INVALID;
    }

    pwinFB = _GetWinFB(pInstance,g_pGOPCtxLocal->pGOPCtxShared->gwinMap[curWinId].u32CurFBId);

    if(pwinFB == NULL)
    {
        GOP_M_ERR("[%s][%d]GetWinFB Fail : WrongFBID=%td\n",__FUNCTION__,__LINE__,(ptrdiff_t)g_pGOPCtxLocal->pGOPCtxShared->gwinMap[curWinId].u32CurFBId);
        return GWIN_ID_INVALID;
    }
    if (!_GOP_IsFbIdValid(pInstance,g_pGOPCtxLocal->pGOPCtxShared->gwinMap[curWinId].u32CurFBId))
    {
        GOP_M_ERR("[%s][%d]FbId %td  is out of range\n",__FUNCTION__,__LINE__,(ptrdiff_t)g_pGOPCtxLocal->pGOPCtxShared->gwinMap[curWinId].u32CurFBId);
        return GWIN_ID_INVALID;
    }

    if (0==pwinFB->in_use)
    {
        GOP_M_ERR("[%s][%d]FbId %td is not allocated\n",__FUNCTION__,__LINE__,(ptrdiff_t)g_pGOPCtxLocal->pGOPCtxShared->gwinMap[curWinId].u32CurFBId);
        return GWIN_ID_INVALID;

    }
    return g_pGOPCtxLocal->pGOPCtxShared->gwinMap[curWinId].u32CurFBId;

}


MS_U32 GOP_GWIN_SetHMirror(void* pInstance,MS_BOOL bEnable)
{
    MS_U8 u8GwinID=0, u8GWinBase=0, u8GOPWinNum=0, u8GOPIdx=0;
    MS_U16 u16GwinState = 0;
    GOP_GwinInfo sGWININfo;
    MS_BOOL bUpdateRegOnce = FALSE, bHMirror = FALSE, bVMirror = FALSE;

#ifdef INSTANT_PRIVATE
    GOP_INSTANT_PRIVATE* psGOPInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void*)&psGOPInstPri);
#endif

    bUpdateRegOnce = g_pGOPCtxLocal->bUpdateRegOnce[GOP_PUBLIC_UPDATE];
    g_pGOPCtxLocal->pGOPCtxShared->bHMirror = bEnable;

    for(u8GOPIdx=0; u8GOPIdx < MDrv_GOP_GetMaxGOPNum(g_pGOPCtxLocal);u8GOPIdx++)
    {
        MDrv_GOP_IsGOPMirrorEnable(g_pGOPCtxLocal, u8GOPIdx, &bHMirror, &bVMirror);
        if(bHMirror == bEnable)
        {
            continue; //Duplicate mirror setting, skip it
        }

        u8GOPWinNum = MDrv_GOP_GetGwinNum(g_pGOPCtxLocal, u8GOPIdx);
        if(u8GOPWinNum == 0xff)
        {
            continue; //NO Gwin, skip it
        }

        u8GWinBase = _GOP_SelGwinId2(pInstance, u8GOPIdx, 0);
        u16GwinState = 0; //Clear GWIN on/off state
         //Turn off all on state GWIN
        for (u8GwinID=u8GWinBase; u8GwinID<u8GWinBase+u8GOPWinNum; u8GwinID++)
        {
            if (!_GOP_IsGwinIdValid(pInstance,u8GwinID))
            {
                GOP_M_ERR("[%s][%d]GWIN %d  is out of range\n",__FUNCTION__,__LINE__,u8GwinID);
                return GOP_API_FAIL;
            }

            if (g_pGOPCtxLocal->pGOPCtxShared->bGWINEnable[u8GwinID] != TRUE)
            {
                continue;
            }

            if(_GOP_GWIN_IsGwinExistInClient(pInstance,u8GwinID)== TRUE)
            {
                MDrv_GOP_GWIN_EnableGwin(g_pGOPCtxLocal, u8GwinID, FALSE);
                u16GwinState |= (1<<u8GwinID);
            }
            else
            {
                u16GwinState = 0xFFFF;
                break; //This GWIn does not exist in current client, skip gop mirror setting
            }
        }

        if(u16GwinState == 0xFFFF)
        {
            continue;//This GWIn does not exist in current client, skip gop mirror setting
        }

        //Enable mirror
        MDrv_GOP_GWIN_EnableHMirror(g_pGOPCtxLocal, u8GOPIdx, bEnable);
        //Reset ON state GWIN info after mirror enable
        if(u16GwinState !=0)
        {
            g_pGOPCtxLocal->bUpdateRegOnce[GOP_PUBLIC_UPDATE] = TRUE; //Update GWIN info and GWIN enable one time
            for (u8GwinID=u8GWinBase; u8GwinID<u8GWinBase+u8GOPWinNum; u8GwinID++)
            {
                if(u16GwinState & (1<<u8GwinID))
                {
                    if(GOP_GetWinInfo(pInstance,u8GwinID, &sGWININfo) == TRUE)
                    {
                        GOP_SetWinInfo(pInstance,u8GwinID, &sGWININfo);
                    }
                    MDrv_GOP_GWIN_EnableGwin(g_pGOPCtxLocal, u8GwinID, TRUE);
                }
            }
            g_pGOPCtxLocal->bUpdateRegOnce[GOP_PUBLIC_UPDATE] = bUpdateRegOnce;
        }
        MDrv_GOP_GWIN_UpdateReg(g_pGOPCtxLocal, u8GOPIdx);
    }

    if (bEnable == FALSE)
    {
        g_pGOPCtxLocal->u16GOP_HMirror_HPos = 0;
        g_pGOPCtxLocal->u16GOP_HMirrorRBLK_Adr = 0;
    }
    return GOP_API_SUCCESS;
}


MS_U32 GOP_GWIN_SetVMirror(void* pInstance,MS_BOOL bEnable)
{
    MS_U8 u8GwinID=0, u8GWinBase=0, u8GOPWinNum=0, u8GOPIdx=0;
    MS_U16 u16GwinState = 0;
    GOP_GwinInfo sGWININfo;
    MS_BOOL bUpdateRegOnce = FALSE, bHMirror = FALSE, bVMirror = FALSE;

#ifdef INSTANT_PRIVATE
    GOP_INSTANT_PRIVATE* psGOPInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void*)&psGOPInstPri);
#endif

    bUpdateRegOnce = g_pGOPCtxLocal->bUpdateRegOnce[GOP_PUBLIC_UPDATE];
    g_pGOPCtxLocal->pGOPCtxShared->bVMirror = bEnable;

    for(u8GOPIdx=0; u8GOPIdx < MDrv_GOP_GetMaxGOPNum(g_pGOPCtxLocal);u8GOPIdx++)
    {
        MDrv_GOP_IsGOPMirrorEnable(g_pGOPCtxLocal, u8GOPIdx, &bHMirror, &bVMirror);
        if(bVMirror == bEnable)
        {
            continue; //Duplicate mirror setting, skip it
        }

        u8GOPWinNum = MDrv_GOP_GetGwinNum(g_pGOPCtxLocal, u8GOPIdx);
		if(u8GOPWinNum == 0xff)
        {
            continue; //NO Gwin, skip it
        }

        u8GWinBase = _GOP_SelGwinId2(pInstance, u8GOPIdx, 0);
        u16GwinState = 0; //Clear GWIN on/off state
         //Turn off all on state GWIN
        for (u8GwinID=u8GWinBase; u8GwinID<u8GWinBase+u8GOPWinNum; u8GwinID++)
        {
            if (!_GOP_IsGwinIdValid(pInstance,u8GwinID))
            {
                GOP_M_ERR("[%s][%d]GWIN %d  is out of range\n",__FUNCTION__,__LINE__,u8GwinID);
                return GOP_API_FAIL;
            }

            if (g_pGOPCtxLocal->pGOPCtxShared->bGWINEnable[u8GwinID] != TRUE)
            {
                continue;
            }

            if(_GOP_GWIN_IsGwinExistInClient(pInstance,u8GwinID)== TRUE)
            {
                MDrv_GOP_GWIN_EnableGwin(g_pGOPCtxLocal, u8GwinID, FALSE);
                u16GwinState |= (1<<u8GwinID);
            }
            else
            {
                u16GwinState = 0xFFFF;
                break; //This GWIn does not exist in current client, skip gop mirror setting
            }
        }

        if(u16GwinState == 0xFFFF)
        {
            continue; //There is GWIn does not exist in current client, skip gop mirror setting
        }

        //Enable mirror
        MDrv_GOP_GWIN_EnableVMirror(g_pGOPCtxLocal, u8GOPIdx, bEnable);
        //Reset ON state GWIN info after mirror enable
        if(u16GwinState !=0)
        {
            g_pGOPCtxLocal->bUpdateRegOnce[GOP_PUBLIC_UPDATE] = TRUE; //Update GWIN info and GWIN enable one time
            for (u8GwinID=u8GWinBase; u8GwinID<u8GWinBase+u8GOPWinNum; u8GwinID++)
            {
                if(u16GwinState & (1<<u8GwinID))
                {
                    if(GOP_GetWinInfo(pInstance,u8GwinID, &sGWININfo) == TRUE)
                    {
                        GOP_SetWinInfo(pInstance,u8GwinID, &sGWININfo);
                    }
                    MDrv_GOP_GWIN_EnableGwin(g_pGOPCtxLocal, u8GwinID, TRUE);
                }
            }
            g_pGOPCtxLocal->bUpdateRegOnce[GOP_PUBLIC_UPDATE] = bUpdateRegOnce;
        }
        MDrv_GOP_GWIN_UpdateReg(g_pGOPCtxLocal, u8GOPIdx);
    }

    if (bEnable == FALSE)
    {
        g_pGOPCtxLocal->u16GOP_VMirrorRBLK_Adr = 0;
        g_pGOPCtxLocal->u16GOP_VMirror_VPos = 0;
    }

    return GOP_API_SUCCESS;

}

MS_U32 GOP_GWIN_SetNewAlphaMode(void* pInstance,MS_U8 gwin, MS_BOOL bEn)
{
#ifdef INSTANT_PRIVATE
    GOP_INSTANT_PRIVATE* psGOPInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void*)&psGOPInstPri);
#endif

    if (!_GOP_IsGwinIdValid(pInstance,gwin))
    {
        GOP_M_ERR("[%s][%d]GWIN %d  is out of range\n",__FUNCTION__,__LINE__,gwin);
        return GOP_API_FAIL;
    }

    MDrv_GOP_GWIN_SetNewAlphaMode(g_pGOPCtxLocal, gwin, bEn);

    return GOP_API_SUCCESS;

}

MS_U32 GOP_GWIN_IsEnable(void* pInstance,MS_U8 winId)
{
    MS_BOOL IsEnable =FALSE;

#ifdef INSTANT_PRIVATE
    GOP_INSTANT_PRIVATE* psGOPInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void*)&psGOPInstPri);
#endif

    if (!_GOP_IsGwinIdValid(pInstance,winId))
    {
        GOP_M_ERR("[%s][%d]GWIN %d  is out of range\n",__FUNCTION__,__LINE__,winId);
        return FALSE;
    }
    MDrv_GOP_GWIN_IsGWINEnabled(g_pGOPCtxLocal, winId, &IsEnable);
    return IsEnable;
}

MS_U32 GOP_GWIN_SetEnable(void* pInstance,MS_U8 winId, MS_BOOL bEnable)
{
    MS_U16 regval2 =0,i;
    MS_U32 u32fbId;
    GOP_WinFB_INFO* pwinFB;
#ifdef INSTANT_PRIVATE
    GOP_INSTANT_PRIVATE* psGOPInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void*)&psGOPInstPri);
#endif
    if (!_GOP_IsGwinIdValid(pInstance,winId))
    {
        GOP_M_WARN("[%s][%d]GWIN %d  is out of range\n",__FUNCTION__,__LINE__,winId);
        return GOP_API_FAIL;
    }

    //printf("GWINID=%d,Enable=%d\n",(MS_U8)winId,(MS_U8)bEnable);
    if (!_GOP_GWIN_IsGwinExistInClient(pInstance,winId))
    {
        GOP_M_ERR("[%s][%d]GWIN %d  is not exist\n",__FUNCTION__,__LINE__,winId);
        //__ASSERT(0);
    }

    //   printf(" u8win=%02bx, bEnable=%02bx\n",winId,bEnable);
    u32fbId = g_pGOPCtxLocal->pGOPCtxShared->gwinMap[winId].u32CurFBId;
    pwinFB = _GetWinFB(pInstance,u32fbId);

    if(pwinFB == NULL)
    {
        GOP_M_ERR("[%s][%d]GetWinFB Fail : WrongFBID=%d\n",__FUNCTION__,__LINE__,(unsigned int)u32fbId);
        goto fail;
    }

    if (!_GOP_IsFbIdValid(pInstance,u32fbId))
    {
        GOP_M_ERR("[%s][%d]FbId %d  is out of range\n",__FUNCTION__,__LINE__,(unsigned int)u32fbId);
        goto fail;
    }

    if (0 == pwinFB->in_use)
    {
        msWarning(ERR_GWIN_ID_NOT_ALLOCATED);
        GOP_M_ERR("[%s][%d]GWIN %d not allocated\n",__FUNCTION__,__LINE__,winId);
        goto fail;
    }
     pwinFB->enable = bEnable;

    //regval2 = MApi_GOP_GWIN_GetActiveGWIN();

    for(i = 0; i < g_pGOPCtxLocal->pGopChipProperty->TotalGwinNum ; i++)
    {
        if(GOP_GWIN_IsEnable(pInstance,i) == TRUE)
        {
            regval2 |= 1<<i;
        }
    }

    if (bEnable == 0) // Turn off GOP
    {
        MDrv_GOP_GWIN_EnableGwin(g_pGOPCtxLocal, winId, FALSE);
        if ((regval2 & ~(1<<winId)) == 0)
        {
            if(fpXCReduceBWForOSD!=NULL)
            {
                fpXCReduceBWForOSD(XC_MAIN_WINDOW, FALSE);
            }
        }
    }
    else //turn on GOP
    {
        //printf("gop on, interlace=%bx\n", IsSrcInterlace());
        if(fpXCReduceBWForOSD!=NULL)
        {
            fpXCReduceBWForOSD(XC_MAIN_WINDOW, TRUE);
        }
        MDrv_GOP_GWIN_EnableGwin(g_pGOPCtxLocal, winId, TRUE);
    }
    return GOP_API_SUCCESS;

fail:
    if (bEnable == 0) // Turn off GOP
    {
        MDrv_GOP_GWIN_EnableGwin(g_pGOPCtxLocal, winId, FALSE);
    }
    return GOP_API_FAIL;
}

MS_U32 GOP_GWIN_SetBlending(void* pInstance,MS_U8 u8win, MS_BOOL bEnable, MS_U8 u8coef)
{
#ifdef INSTANT_PRIVATE
    GOP_INSTANT_PRIVATE* psGOPInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void*)&psGOPInstPri);
#endif

    if (!_GOP_IsGwinIdValid(pInstance,u8win))
    {
        GOP_M_ERR("[%s][%d]GWIN %d  is out of range\n",__FUNCTION__,__LINE__,u8win);
        return GOP_API_FAIL;
    }
    MDrv_GOP_GWIN_SetBlending(g_pGOPCtxLocal, u8win,bEnable,u8coef);
    return GOP_API_SUCCESS;

}

MS_U32 GOP_GWIN_Set_VStretchMode(void* pInstance,MS_U8 u8GOP,EN_GOP_STRETCH_VMODE VStrchMode)
{
#ifdef INSTANT_PRIVATE
    GOP_INSTANT_PRIVATE* psGOPInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void*)&psGOPInstPri);
#endif

    switch(VStrchMode)
    {
        case E_GOP_VSTRCH_LINEAR:
        case E_GOP_VSTRCH_LINEAR_GAIN0:
        case E_GOP_VSTRCH_LINEAR_GAIN1:
            if(g_pGOPCtxLocal->pGopChipProperty->bSupportV2tap_16Phase[u8GOP] == TRUE)
            {
                MDrv_GOP_GWIN_Set_VStretchMode(g_pGOPCtxLocal,u8GOP,E_DRV_GOP_VSTRCH_LINEAR);
                MDrv_GOP_GWIN_Load_VStretchModeTable(g_pGOPCtxLocal,u8GOP,E_DRV_GOP_VSTRCH_LINEAR);
            }
            else
            {
                GOP_M_WARN("[%s]GOP%d not support V 2-tap 16 phase mode:%d\n",__FUNCTION__,u8GOP,VStrchMode);
                return (GOP_API_FUN_NOT_SUPPORTED);
            }
            break;
        case E_GOP_VSTRCH_DUPLICATE:
            MDrv_GOP_GWIN_Set_VStretchMode(g_pGOPCtxLocal,u8GOP, E_DRV_GOP_VSTRCH_DUPLICATE);
            break;
        case E_GOP_VSTRCH_NEAREST:
            MDrv_GOP_GWIN_Set_VStretchMode(g_pGOPCtxLocal,u8GOP, E_DRV_GOP_VSTRCH_NEAREST);
            break;
        case E_GOP_VSTRCH_LINEAR_GAIN2:
            if(g_pGOPCtxLocal->pGopChipProperty->bSupportV_BiLinear[u8GOP] == TRUE)
            {
                MDrv_GOP_GWIN_Set_VStretchMode(g_pGOPCtxLocal,u8GOP, E_DRV_GOP_VSTRCH_LINEAR_GAIN2);
            }
            else
            {
                GOP_M_WARN("[%s]GOP%d not support Bi-Linear mode:%d\n",__FUNCTION__,u8GOP,VStrchMode);
                return (GOP_API_FUN_NOT_SUPPORTED);
            }
            break;
        case E_GOP_VSTRCH_4TAP:
            if(g_pGOPCtxLocal->pGopChipProperty->bSupportV4tap[u8GOP] == TRUE)
            {
                MDrv_GOP_GWIN_Set_VStretchMode(g_pGOPCtxLocal,u8GOP, E_DRV_GOP_VSTRCH_4TAP);
                MDrv_GOP_GWIN_Load_VStretchModeTable(g_pGOPCtxLocal,u8GOP,E_DRV_GOP_VSTRCH_4TAP);
            }
            else
            {
                GOP_M_WARN("[%s]GOP%d not support VStrchMode:%d\n",__FUNCTION__,u8GOP,VStrchMode);
                return(GOP_API_FUN_NOT_SUPPORTED);
            }
            break;
        case E_GOP_VSTRCH_4TAP_100:
            if(g_pGOPCtxLocal->pGopChipProperty->bSupportV4tap[u8GOP] == TRUE)
            {
                MDrv_GOP_GWIN_Set_VStretchMode(g_pGOPCtxLocal,u8GOP, E_DRV_GOP_VSTRCH_4TAP);
                MDrv_GOP_GWIN_Load_VStretchModeTable(g_pGOPCtxLocal,u8GOP,E_DRV_GOP_HSTRCH_V4TAP_100);
            }
            else
            {
                GOP_M_WARN("[%s]GOP%d not support VStrchMode:%d\n",__FUNCTION__,u8GOP,VStrchMode);
                return(GOP_API_FUN_NOT_SUPPORTED);
            }
            break;
        case E_GOP_VSTRCH_4TAP_105_GAIN22:
            if(g_pGOPCtxLocal->pGopChipProperty->bSupportV4tap[u8GOP] == TRUE)
            {
                MDrv_GOP_GWIN_Set_VStretchMode(g_pGOPCtxLocal,u8GOP, E_DRV_GOP_VSTRCH_4TAP);
                MDrv_GOP_GWIN_Load_VStretchModeTable(g_pGOPCtxLocal,u8GOP,E_DRV_GOP_VSTRCH_V4TAP_105_GAIN22);
            }
            else
            {
                GOP_M_WARN("[%s]GOP%d not support VStrchMode:%d\n",__FUNCTION__,u8GOP,VStrchMode);
                return(GOP_API_FUN_NOT_SUPPORTED);
            }
            break;
        case E_GOP_VSTRCH_4TAP_105_GAIN31:
            if(g_pGOPCtxLocal->pGopChipProperty->bSupportV4tap[u8GOP] == TRUE)
            {
                MDrv_GOP_GWIN_Set_VStretchMode(g_pGOPCtxLocal,u8GOP, E_DRV_GOP_VSTRCH_4TAP);
                MDrv_GOP_GWIN_Load_VStretchModeTable(g_pGOPCtxLocal,u8GOP,E_DRV_GOP_VSTRCH_V4TAP_105_GAIN31);
            }
            else
            {
                GOP_M_WARN("[%s]GOP%d not support VStrchMode:%d\n",__FUNCTION__,u8GOP,VStrchMode);
                return(GOP_API_FUN_NOT_SUPPORTED);
            }
            break;
        case E_GOP_VSTRCH_4TAP_105_GAIN16:
            if(g_pGOPCtxLocal->pGopChipProperty->bSupportV4tap[u8GOP] == TRUE)
            {
                MDrv_GOP_GWIN_Set_VStretchMode(g_pGOPCtxLocal,u8GOP, E_DRV_GOP_VSTRCH_4TAP);
                MDrv_GOP_GWIN_Load_VStretchModeTable(g_pGOPCtxLocal,u8GOP,E_DRV_GOP_VSTRCH_V4TAP_105_GAIN16);
            }
            else
            {
                GOP_M_WARN("[%s]GOP%d not support VStrchMode:%d\n",__FUNCTION__,u8GOP,VStrchMode);
                return(GOP_API_FUN_NOT_SUPPORTED);
            }
            break;
        case E_GOP_VSTRCH_4TAP_105_GAIN17:
            if(g_pGOPCtxLocal->pGopChipProperty->bSupportV4tap[u8GOP] == TRUE)
            {
                MDrv_GOP_GWIN_Set_VStretchMode(g_pGOPCtxLocal,u8GOP, E_DRV_GOP_VSTRCH_4TAP);
                MDrv_GOP_GWIN_Load_VStretchModeTable(g_pGOPCtxLocal,u8GOP,E_DRV_GOP_VSTRCH_V4TAP_105_GAIN17);
            }
            else
            {
                GOP_M_WARN("[%s]GOP%d not support VStrchMode:%d\n",__FUNCTION__,u8GOP,VStrchMode);
                return(GOP_API_FUN_NOT_SUPPORTED);
            }
            break;
        default:
            GOP_M_WARN("[%s]not support enum VStrchMode:%d\n",__FUNCTION__,VStrchMode);
            return(GOP_API_ENUM_NOT_SUPPORTED);
    }
    return GOP_API_SUCCESS;
}

E_GOP_API_Result _GOP_GWIN_IsSupport_HStretchMode(void* pInstance,MS_U8 u8GOP,EN_GOP_STRETCH_HMODE enHStrchMode)
{
#ifdef INSTANT_PRIVATE
    GOP_INSTANT_PRIVATE* psGOPInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void*)&psGOPInstPri);
#endif

    switch(enHStrchMode)
    {
        case E_GOP_HSTRCH_DUPLICATE:
            if(g_pGOPCtxLocal->pGopChipProperty->bSupportHDuplicate[u8GOP] == TRUE)
            {
                return GOP_API_SUCCESS;
            }
            else
            {
                GOP_M_WARN("[%s]GOP%d not support H Duplicate mode:%d\n",__FUNCTION__,u8GOP,E_GOP_HSTRCH_DUPLICATE);
                return GOP_API_ENUM_NOT_SUPPORTED;
            }
        case E_GOP_HSTRCH_6TAPE:
        case E_GOP_HSTRCH_6TAPE_LINEAR:
        case E_GOP_HSTRCH_6TAPE_NEAREST:
        case E_GOP_HSTRCH_6TAPE_GAIN0:
        case E_GOP_HSTRCH_6TAPE_GAIN1:
        case E_GOP_HSTRCH_6TAPE_GAIN2:
        case E_GOP_HSTRCH_6TAPE_GAIN3:
        case E_GOP_HSTRCH_6TAPE_GAIN4:
        case E_GOP_HSTRCH_6TAPE_GAIN5:
        case E_GOP_HSTRCH_2TAPE:
            if(g_pGOPCtxLocal->pGopChipProperty->bSupportH6Tap_8Phase[u8GOP] == TRUE)
            {
                return GOP_API_SUCCESS;
            }
            else
            {
                GOP_M_WARN("[%s]GOP%d not support H 6-tap mode:%d\n",__FUNCTION__,u8GOP,enHStrchMode);
                return GOP_API_ENUM_NOT_SUPPORTED;
            }
        case E_GOP_HSTRCH_4TAPE:
        case E_GOP_HSTRCH_NEW4TAP:
        case E_GOP_HSTRCH_NEW4TAP_50:
        case E_GOP_HSTRCH_NEW4TAP_55:
        case E_GOP_HSTRCH_NEW4TAP_65:
        case E_GOP_HSTRCH_NEW4TAP_75:
        case E_GOP_HSTRCH_NEW4TAP_85:
        case E_GOP_HSTRCH_NEW4TAP_95:
        case E_GOP_HSTRCH_NEW4TAP_100:
        case E_GOP_HSTRCH_NEW4TAP_105:
        case E_GOP_HSTRCH_NEW4TAP_105_GAIN16:
        case E_GOP_HSTRCH_NEW4TAP_105_GAIN17:
        case E_GOP_HSTRCH_NEW4TAP_105_GAIN22:
        case E_GOP_HSTRCH_NEW4TAP_105_GAIN31:
            if(g_pGOPCtxLocal->pGopChipProperty->bSupportH4Tap_256Phase[u8GOP] == TRUE)
            {
                return GOP_API_SUCCESS;
            }
            else
            {
                GOP_M_WARN("[%s]GOP%d not support H 4-tap 256phase mode:%d\n",__FUNCTION__,u8GOP,enHStrchMode);
                return GOP_API_ENUM_NOT_SUPPORTED;
            }
        default:
             return GOP_API_ENUM_NOT_SUPPORTED;
    }
}

MS_U32 GOP_GWIN_Set_HStretchMode(void* pInstance,MS_U8 u8GOP,EN_GOP_STRETCH_HMODE HStrchMode)
{
#ifdef INSTANT_PRIVATE
    GOP_INSTANT_PRIVATE* psGOPInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void*)&psGOPInstPri);
#endif

    if(_GOP_GWIN_IsSupport_HStretchMode(pInstance, u8GOP, HStrchMode) != GOP_API_SUCCESS)
    {
        return GOP_API_FUN_NOT_SUPPORTED;
    }

    switch(HStrchMode)
    {
        case E_GOP_HSTRCH_DUPLICATE:
            MDrv_GOP_GWIN_Set_HStretchMode(g_pGOPCtxLocal, u8GOP, E_DRV_GOP_HSTRCH_DUPLICATE);
            break;
        case E_GOP_HSTRCH_6TAPE:
            MDrv_GOP_GWIN_Set_HStretchMode(g_pGOPCtxLocal, u8GOP, E_DRV_GOP_HSTRCH_6TAPE);
            MDrv_GOP_GWIN_Load_HStretchModeTable(g_pGOPCtxLocal, u8GOP, E_DRV_GOP_HSTRCH_6TAPE);
            break;
        case E_GOP_HSTRCH_6TAPE_LINEAR:
            MDrv_GOP_GWIN_Set_HStretchMode(g_pGOPCtxLocal, u8GOP, E_DRV_GOP_HSTRCH_6TAPE);
            MDrv_GOP_GWIN_Load_HStretchModeTable(g_pGOPCtxLocal, u8GOP,E_DRV_GOP_HSTRCH_6TAPE_LINEAR);
            break;
        case E_GOP_HSTRCH_6TAPE_NEAREST:
            MDrv_GOP_GWIN_Set_HStretchMode(g_pGOPCtxLocal, u8GOP, E_DRV_GOP_HSTRCH_6TAPE);
            MDrv_GOP_GWIN_Load_HStretchModeTable(g_pGOPCtxLocal, u8GOP, E_DRV_GOP_HSTRCH_6TAPE_NEAREST);
            break;
        case E_GOP_HSTRCH_6TAPE_GAIN0:
            MDrv_GOP_GWIN_Set_HStretchMode(g_pGOPCtxLocal, u8GOP, E_DRV_GOP_HSTRCH_6TAPE);
            MDrv_GOP_GWIN_Load_HStretchModeTable(g_pGOPCtxLocal,u8GOP, E_DRV_GOP_HSTRCH_6TAPE_GAIN0);
            break;
        case E_GOP_HSTRCH_6TAPE_GAIN1:
            MDrv_GOP_GWIN_Set_HStretchMode(g_pGOPCtxLocal, u8GOP, E_DRV_GOP_HSTRCH_6TAPE);
            MDrv_GOP_GWIN_Load_HStretchModeTable(g_pGOPCtxLocal, u8GOP, E_DRV_GOP_HSTRCH_6TAPE_GAIN1);
            break;
        case E_GOP_HSTRCH_6TAPE_GAIN2:
            MDrv_GOP_GWIN_Set_HStretchMode(g_pGOPCtxLocal, u8GOP, E_DRV_GOP_HSTRCH_6TAPE);
            MDrv_GOP_GWIN_Load_HStretchModeTable(g_pGOPCtxLocal, u8GOP, E_DRV_GOP_HSTRCH_6TAPE_GAIN2);
            break;
        case E_GOP_HSTRCH_6TAPE_GAIN3:
            MDrv_GOP_GWIN_Set_HStretchMode(g_pGOPCtxLocal, u8GOP, E_DRV_GOP_HSTRCH_6TAPE);
            MDrv_GOP_GWIN_Load_HStretchModeTable(g_pGOPCtxLocal, u8GOP, E_DRV_GOP_HSTRCH_6TAPE_GAIN3);
            break;
        case E_GOP_HSTRCH_6TAPE_GAIN4:
            MDrv_GOP_GWIN_Set_HStretchMode(g_pGOPCtxLocal, u8GOP, E_DRV_GOP_HSTRCH_6TAPE);
            MDrv_GOP_GWIN_Load_HStretchModeTable(g_pGOPCtxLocal, u8GOP, E_DRV_GOP_HSTRCH_6TAPE_GAIN4);
            break;
        case E_GOP_HSTRCH_6TAPE_GAIN5:
            MDrv_GOP_GWIN_Set_HStretchMode(g_pGOPCtxLocal, u8GOP, E_DRV_GOP_HSTRCH_6TAPE);
            MDrv_GOP_GWIN_Load_HStretchModeTable(g_pGOPCtxLocal, u8GOP, E_DRV_GOP_HSTRCH_6TAPE_GAIN5);
            break;
        case E_GOP_HSTRCH_2TAPE:
            MDrv_GOP_GWIN_Set_HStretchMode(g_pGOPCtxLocal, u8GOP, E_DRV_GOP_HSTRCH_6TAPE);
            MDrv_GOP_GWIN_Load_HStretchModeTable(g_pGOPCtxLocal, u8GOP, E_DRV_GOP_HSTRCH_2TAPE);
            break;
        case E_GOP_HSTRCH_4TAPE:
            MDrv_GOP_GWIN_Set_HStretchMode(g_pGOPCtxLocal, u8GOP, E_DRV_GOP_HSTRCH_4TAPE);
            break;
        case E_GOP_HSTRCH_NEW4TAP:
            MDrv_GOP_GWIN_Set_HStretchMode(g_pGOPCtxLocal, u8GOP, E_DRV_GOP_HSTRCH_4TAPE);
            MDrv_GOP_GWIN_Load_HStretchModeTable(g_pGOPCtxLocal, u8GOP, E_DRV_GOP_HSTRCH_NEW4TAP_45);
            break;
        case E_GOP_HSTRCH_NEW4TAP_50:
            MDrv_GOP_GWIN_Set_HStretchMode(g_pGOPCtxLocal, u8GOP, E_DRV_GOP_HSTRCH_4TAPE);
            MDrv_GOP_GWIN_Load_HStretchModeTable(g_pGOPCtxLocal, u8GOP, E_DRV_GOP_HSTRCH_NEW4TAP_50);
            break;
        case E_GOP_HSTRCH_NEW4TAP_55:
            MDrv_GOP_GWIN_Set_HStretchMode(g_pGOPCtxLocal, u8GOP, E_DRV_GOP_HSTRCH_4TAPE);
            MDrv_GOP_GWIN_Load_HStretchModeTable(g_pGOPCtxLocal, u8GOP, E_DRV_GOP_HSTRCH_NEW4TAP_55);
            break;
        case E_GOP_HSTRCH_NEW4TAP_65:
            MDrv_GOP_GWIN_Set_HStretchMode(g_pGOPCtxLocal, u8GOP, E_DRV_GOP_HSTRCH_4TAPE);
            MDrv_GOP_GWIN_Load_HStretchModeTable(g_pGOPCtxLocal, u8GOP, E_DRV_GOP_HSTRCH_NEW4TAP_65);
            break;
        case E_GOP_HSTRCH_NEW4TAP_75:
            MDrv_GOP_GWIN_Set_HStretchMode(g_pGOPCtxLocal, u8GOP, E_DRV_GOP_HSTRCH_4TAPE);
            MDrv_GOP_GWIN_Load_HStretchModeTable(g_pGOPCtxLocal, u8GOP, E_DRV_GOP_HSTRCH_NEW4TAP_75);
            break;
        case E_GOP_HSTRCH_NEW4TAP_85:
            MDrv_GOP_GWIN_Set_HStretchMode(g_pGOPCtxLocal, u8GOP, E_DRV_GOP_HSTRCH_4TAPE);
            MDrv_GOP_GWIN_Load_HStretchModeTable(g_pGOPCtxLocal, u8GOP, E_DRV_GOP_HSTRCH_NEW4TAP_85);
            break;
        case E_GOP_HSTRCH_NEW4TAP_95:
            MDrv_GOP_GWIN_Set_HStretchMode(g_pGOPCtxLocal, u8GOP, E_DRV_GOP_HSTRCH_4TAPE);
            MDrv_GOP_GWIN_Load_HStretchModeTable(g_pGOPCtxLocal, u8GOP, E_DRV_GOP_HSTRCH_NEW4TAP_95);
            break;
        case E_GOP_HSTRCH_NEW4TAP_105:
            MDrv_GOP_GWIN_Set_HStretchMode(g_pGOPCtxLocal, u8GOP, E_DRV_GOP_HSTRCH_4TAPE);
            MDrv_GOP_GWIN_Load_HStretchModeTable(g_pGOPCtxLocal, u8GOP, E_DRV_GOP_HSTRCH_NEW4TAP_105);
            break;
        case E_GOP_HSTRCH_NEW4TAP_100:
            MDrv_GOP_GWIN_Set_HStretchMode(g_pGOPCtxLocal, u8GOP, E_DRV_GOP_HSTRCH_4TAPE);
            MDrv_GOP_GWIN_Load_HStretchModeTable(g_pGOPCtxLocal, u8GOP, E_DRV_GOP_HSTRCH_NEW4TAP_100);
            break;
         case E_GOP_HSTRCH_NEW4TAP_105_GAIN22:
            MDrv_GOP_GWIN_Set_HStretchMode(g_pGOPCtxLocal, u8GOP, E_DRV_GOP_HSTRCH_4TAPE);
            MDrv_GOP_GWIN_Load_HStretchModeTable(g_pGOPCtxLocal, u8GOP, E_DRV_GOP_HSTRCH_NEW4TAP_105_GAIN22);
            break;
        case E_GOP_HSTRCH_NEW4TAP_105_GAIN31:
            MDrv_GOP_GWIN_Set_HStretchMode(g_pGOPCtxLocal, u8GOP, E_DRV_GOP_HSTRCH_4TAPE);
            MDrv_GOP_GWIN_Load_HStretchModeTable(g_pGOPCtxLocal, u8GOP, E_DRV_GOP_HSTRCH_NEW4TAP_105_GAIN31);
            break;
        case E_GOP_HSTRCH_NEW4TAP_105_GAIN16:
            MDrv_GOP_GWIN_Set_HStretchMode(g_pGOPCtxLocal, u8GOP, E_DRV_GOP_HSTRCH_4TAPE);
            MDrv_GOP_GWIN_Load_HStretchModeTable(g_pGOPCtxLocal, u8GOP, E_DRV_GOP_HSTRCH_NEW4TAP_105_GAIN16);
            break;
        case E_GOP_HSTRCH_NEW4TAP_105_GAIN17:
            MDrv_GOP_GWIN_Set_HStretchMode(g_pGOPCtxLocal, u8GOP, E_DRV_GOP_HSTRCH_4TAPE);
            MDrv_GOP_GWIN_Load_HStretchModeTable(g_pGOPCtxLocal, u8GOP, E_DRV_GOP_HSTRCH_NEW4TAP_105_GAIN17);
            break;
        default:
            return GOP_API_ENUM_NOT_SUPPORTED;
    }

    return GOP_API_SUCCESS;
}

MS_U32 GOP_GWIN_EnableT3DMode(void* pInstance,MS_BOOL bEnable)
{
#ifdef INSTANT_PRIVATE
    GOP_INSTANT_PRIVATE* psGOPInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void*)&psGOPInstPri);
#endif

#ifdef ENABLE_GOP_T3DPATCH
    MS_U8 u8GOPNum = 0;
    DRV_GOPDstType enGopDst = E_DRV_GOP_DST_INVALID;
    MS_U16 u16StrwinStr;

    if (bEnable)
    {
        g_pGOPCtxLocal->pGopChipProperty->GOP_PD = GOP_PD_T3D;
    }
    else
    {
        g_pGOPCtxLocal->pGopChipProperty->GOP_PD = GOP_PD_NORMAL;
    }

    for (u8GOPNum = 0; u8GOPNum < MDrv_GOP_GetMaxGOPNum(g_pGOPCtxLocal); u8GOPNum++)
    {
        if (!(g_pGOPCtxLocal->pGOPCtxShared->bGopHasInitialized[u8GOPNum]))
        {
            continue;
        }

        MDrv_GOP_GWIN_GetDstPlane(g_pGOPCtxLocal, u8GOPNum, &enGopDst);
        if (E_DRV_GOP_DST_OP0 == enGopDst)
        {
            _GOP_GetPDByDst(pInstance,u8GOPNum, enGopDst, &u16StrwinStr);
            _GOP_AdjustHSPD(pInstance,u8GOPNum, u16StrwinStr, enGopDst);
        }
    }
    MDrv_GOP_GWIN_UpdateRegWithSync(g_pGOPCtxLocal, u8GOPNum, TRUE);
#endif
    return(GOP_API_SUCCESS);
}

MS_U32 GOP_PowerOn(void* pInstance)
{
#ifdef INSTANT_PRIVATE
    GOP_INSTANT_PRIVATE* psGOPInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void*)&psGOPInstPri);
#endif

	  if (g_pGOPCtxLocal->IsClkClosed)
    {
        GOP_M_ERR("\n%s, The GOP has been powered on!!!\n",__FUNCTION__);
        MDrv_GOP_SetClock(g_pGOPCtxLocal,TRUE);
        g_pGOPCtxLocal->IsClkClosed = FALSE;
        return(GOP_API_SUCCESS);
    }
    else
    {
        GOP_M_ERR("\n%s,Warning!!! No need to power on again!\n",__FUNCTION__);
        return(GOP_API_FAIL);
    }
}
MS_U32 GOP_PowerOff(void* pInstance)
{
#ifdef INSTANT_PRIVATE
    GOP_INSTANT_PRIVATE* psGOPInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void*)&psGOPInstPri);
#endif

	  if (!g_pGOPCtxLocal->IsClkClosed)
    {
        GOP_M_ERR("\n%s,Warning!!! The GOP has been powered off, please turn on it if you want to use\n",__FUNCTION__);
        MDrv_GOP_SetClock(g_pGOPCtxLocal,FALSE);
        g_pGOPCtxLocal->IsClkClosed =TRUE;
        return(GOP_API_SUCCESS);
    }
    else
    {
        GOP_M_ERR("\n%s,Warning!!! No need to power off again!\n",__FUNCTION__);
        return(GOP_API_FAIL);
    }
}

MS_U32 GOP_GetRTStatus(void* pInstance,GOP_ApiStatus *pGopApiSts)
{
	  MS_U8 i, j;

#ifdef INSTANT_PRIVATE
    GOP_INSTANT_PRIVATE* psGOPInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void*)&psGOPInstPri);
#endif

    for (i=0; i<MDrv_GOP_GetMaxGOPNum(g_pGOPCtxLocal); i++)
    {
        pGopApiSts->eGOPNumDstType[i] = (EN_GOP_DST_TYPE)0xFF;
        pGopApiSts->u16GopNumStrWinHSize[i] = pGopApiSts->u16GopNumStrWinVSize[i] = 0xFF;

        switch (i)
        {
            case 0: //gop0
            {
                MS_U8 u8GWinNum = MDrv_GOP_GetGwinNum(g_pGOPCtxLocal,i);
                for (j=0; j < u8GWinNum; j++)
                {
                    pGopApiSts->Gop0GwinIsEnable[j] = FALSE;
                }
            }
            break;
            case 1: //gop1
            {
                MS_U8 u8GWinNum = MDrv_GOP_GetGwinNum(g_pGOPCtxLocal,i);
                for (j=0; j < u8GWinNum; j++)
                {
                    pGopApiSts->Gop1GwinIsEnable[j] = FALSE;
                }
            }
            break;
            case 2: //gop2
            {
                pGopApiSts->Gop2GwinIsEnable = FALSE;
            }
            break;
            case 3: //gop3
            {
                pGopApiSts->Gop3GwinIsEnable = FALSE;
            }
            break;
            case 4: //gop4
            {
                pGopApiSts->Gop4GwinIsEnable = FALSE;
            }
            break;
            case 5: //gop5
            {
                pGopApiSts->Gop5GwinIsEnable = FALSE;
            }
            break;
            default:
                break;
        }
    }

    for (i=0; i<MDrv_GOP_GetMaxGOPNum(g_pGOPCtxLocal); i++)
    {
        //need refine check using DRV_GOPDstType or EN_GOP_DST_TYPE
        DRV_GOPDstType enGOPDst = E_DRV_GOP_DST_INVALID;
        pGopApiSts->u16GopNumStrWinHSize[i] = g_pGOPCtxLocal->pGOPCtxShared->u16APIStretchWidth[i];
        pGopApiSts->u16GopNumStrWinVSize[i] = g_pGOPCtxLocal->pGOPCtxShared->u16APIStretchHeight[i];
        //need refine check using DRV_GOPDstType or EN_GOP_DST_TYPE
        (MDrv_GOP_GWIN_GetDstPlane(g_pGOPCtxLocal, i, &enGOPDst));
        (_GOP_Map_DRVDst2API_Enum_(pInstance,&(pGopApiSts->eGOPNumDstType[i]), enGOPDst));

        switch (i)
        {
            case 0: //gop0
            {
                MS_U8 u8GWinNum = MDrv_GOP_GetGwinNum(g_pGOPCtxLocal,i);
                for (j=0; j < u8GWinNum; j++)
                {
                    pGopApiSts->Gop0GwinIsEnable[j] = GOP_GWIN_IsEnable(pInstance,j);
                }
            }
            break;
            case 1: //gop1
            {
                MS_U8 u8GWinNum = MDrv_GOP_GetGwinNum(g_pGOPCtxLocal,i);
                for (j=0; j < u8GWinNum; j++)
                {
                    pGopApiSts->Gop1GwinIsEnable[j] = GOP_GWIN_IsEnable(pInstance,_GOP_SelGwinId2(pInstance,GOPTYPE.GOP1, (MS_U32)j));
                }
            }
            break;
            case 2: //gop2
            {
                pGopApiSts->Gop2GwinIsEnable = GOP_GWIN_IsEnable(pInstance,_GOP_SelGwinId2(pInstance,GOPTYPE.GOP2, 0));
            }
            break;
            case 3: //gop3
            {
                pGopApiSts->Gop3GwinIsEnable = GOP_GWIN_IsEnable(pInstance,_GOP_SelGwinId2(pInstance,GOPTYPE.GOP3, 0));
            }
            break;
            case 4: //gop4
            {
                pGopApiSts->Gop4GwinIsEnable = GOP_GWIN_IsEnable(pInstance,_GOP_SelGwinId2(pInstance,GOPTYPE.GOP4, 0));
            }
            break;
            case 5: //gop5
            {
                pGopApiSts->Gop5GwinIsEnable = GOP_GWIN_IsEnable(pInstance,_GOP_SelGwinId2(pInstance,GOPTYPE.GOP5, 0));
            }
            break;
            default:
                break;
        }
    }
    return(GOP_API_SUCCESS);
}

MS_U32 GOP_GetInfo(void* pInstance,GOP_ApiInfo* pGopApiInfo)
{
	  MS_U8 u8GwinNum=0, i;
    EN_GOP_DST_TYPE GopDst;
    DRV_GOPDstType drvGopDst;

#ifdef INSTANT_PRIVATE
    GOP_INSTANT_PRIVATE* psGOPInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void*)&psGOPInstPri);
#endif

    for (i=0; i<MDrv_GOP_GetMaxGOPNum(g_pGOPCtxLocal); i++)
    {
        u8GwinNum += MDrv_GOP_GetGwinNum(g_pGOPCtxLocal,i);
    }

    for (i=0; i < MAX_GOP_DST_SUPPORT; i++)
    {
        //need refine check using DRV_GOPDstType or EN_GOP_DST_TYPE
        _GOP_Map_APIDst2DRV_Enum(pInstance,(EN_GOP_DST_TYPE)i, &drvGopDst);
        if (MDrv_GOP_GWIN_SetDstPlane(g_pGOPCtxLocal, 0, drvGopDst,TRUE)==GOP_SUCCESS)
        {
            _GOP_Map_DRVDst2API_Enum_(pInstance,&GopDst, drvGopDst);
            pGopApiInfo->SupportGOPDst[i] = GopDst;
        }
        else
        {
            pGopApiInfo->SupportGOPDst[i] = (EN_GOP_DST_TYPE)0xFF;
        }
    }

    pGopApiInfo->u8MaxGOPNum = MDrv_GOP_GetMaxGOPNum(g_pGOPCtxLocal);
    pGopApiInfo->u8MaxGwinNum = u8GwinNum;
    return(GOP_API_SUCCESS);
}
MS_U32 GOP_GWIN_SetGWinShared(void* pInstance,MS_U8 winId, MS_BOOL bIsShared)
{
#ifdef INSTANT_PRIVATE
    GOP_INSTANT_PRIVATE* psGOPInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void*)&psGOPInstPri);
#endif

	  if (!_GOP_IsGwinIdValid(pInstance,winId))
    {
        GOP_M_ERR("[%s][%d]GWIN %d  is out of range\n",__FUNCTION__,__LINE__,winId);
        return(GOP_API_FAIL);
    }

    g_pGOPCtxLocal->pGOPCtxShared->gwinMap[winId].bIsShared = true;
    return GOP_API_SUCCESS;
}
MS_U32 GOP_GWIN_SetGWinSharedCnt(void* pInstance,MS_U8 winId, MS_U16 u16SharedCnt)
{
#ifdef INSTANT_PRIVATE
    GOP_INSTANT_PRIVATE* psGOPInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void*)&psGOPInstPri);
#endif

	  if (!_GOP_IsGwinIdValid(pInstance,winId))
    {
        GOP_M_ERR("[%s][%d]GWIN %d  is out of range\n",__FUNCTION__,__LINE__,winId);
        return(GOP_API_FAIL);
    }

    g_pGOPCtxLocal->pGOPCtxShared->gwinMap[winId].u16SharedCnt = u16SharedCnt;
    return GOP_API_SUCCESS;
}

MS_U32 GOP_GWIN_Switch2Gwin(void* pInstance,MS_U8 winId, void* pSet)
{
	MS_U32 u32fbId;
    MS_U16 fbFmt;
    MS_PHY u64GeDstAdr=0;
    GOP_WinFB_INFO* pwinFB;
    DRV_GOP_CBFmtInfo* FmtInfo = (DRV_GOP_CBFmtInfo*)pSet;

#ifdef INSTANT_PRIVATE
    GOP_INSTANT_PRIVATE* psGOPInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void*)&psGOPInstPri);
#endif

    if (!_GOP_IsGwinIdValid(pInstance,winId))
    {
        GOP_M_ERR("[%s][%d]GWIN %d  is out of range\n",__FUNCTION__,__LINE__,winId);
        return(GOP_API_FAIL);
    }
    if (!_GOP_GWIN_IsGwinExistInClient(pInstance,winId))
    {
        GOP_M_ERR("[%s][%d]GWIN %d  is not exist\n",__FUNCTION__,__LINE__,winId);
        return(GOP_API_FAIL);
    }

    u32fbId = g_pGOPCtxLocal->pGOPCtxShared->gwinMap[winId].u32CurFBId;
    pwinFB = _GetWinFB(pInstance,u32fbId);

    if(pwinFB == NULL)
    {
        GOP_M_ERR("[%s][%d]GetWinFB Fail : WrongFBID=%td\n",__FUNCTION__,__LINE__, (ptrdiff_t)u32fbId);
        return(GOP_API_FAIL);
    }
    if (!_GOP_IsFbIdValid(pInstance,u32fbId))
    {
        GOP_M_ERR("[%s][%d]FbId %td  is out of range\n",__FUNCTION__,__LINE__, (ptrdiff_t)u32fbId);
        return(GOP_API_FAIL);
    }

    if (0==pwinFB->in_use)
    {
        GOP_M_ERR("[%s][%d]FbId %td is not allocated\n",__FUNCTION__,__LINE__, (ptrdiff_t)u32fbId);
        return(GOP_API_FAIL);

    }

    g_pGOPCtxLocal->current_winId = winId;
    u64GeDstAdr=pwinFB->addr;
    MDrv_GOP_ConvertAPIAddr2HAL(g_pGOPCtxLocal, winId, &u64GeDstAdr);
    fbFmt = pwinFB->fbFmt;
    if ( (pwinFB->fbFmt & 0xFF) == E_MS_FMT_ARGB1555 )
        fbFmt = E_MS_FMT_ARGB1555_DST | (E_MS_FMT_ARGB1555_DST << 8);

    if(fpSetFBFmt != NULL)
    {
        fpSetFBFmt(pwinFB->pitch, u64GeDstAdr,fbFmt);
        FmtInfo->u64Addr = u64GeDstAdr;
        FmtInfo->u16Pitch = pwinFB->pitch;
        FmtInfo->u16Fmt = fbFmt;
    }
    else
    {
        GOP_M_INFO("%s: SetFBFmt is not hooked. Bypass switch...\n", __FUNCTION__);
    }
    return(GOP_API_SUCCESS);
}

MS_U32 GOP_GWIN_IsGwinExist(void* pInstance,MS_U8 gId)
{
    MS_U32 ret;
    if (!_GOP_IsGwinIdValid(pInstance,gId))
    {
        GOP_M_ERR("[%s][%d]GWIN %d  is out of range\n",__FUNCTION__,__LINE__,gId);
        return(GOP_API_FAIL);
    }
    ret = _GOP_GWIN_IsGwinExistInClient(pInstance,gId);
    return ret;
}
MS_U32 GOP_GWIN_GetFreeWinID(void* pInstance)
{
	MS_U8 GWID = GWIN_ID_INVALID;
    MS_U8 i=0,j=0;
    MS_U16 k=0,l=0;

#ifdef INSTANT_PRIVATE
    GOP_INSTANT_PRIVATE* psGOPInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void*)&psGOPInstPri);
#endif

    if (!_GOP_IsGopNumValid(pInstance,MDrv_GOP_Get(g_pGOPCtxLocal)))
    {
        GOP_M_ERR("[%s][%d]GOP %d  is out of range\n",__FUNCTION__,__LINE__,MDrv_GOP_Get(g_pGOPCtxLocal));
        return(GWIN_ID_INVALID);
    }

    if (MDrv_GOP_Get(g_pGOPCtxLocal)==1) //gop1
    {
        i = _GOP_SelGwinId2(pInstance,GOPTYPE.GOP1,0);
        j = MDrv_GOP_GetGwinNum(g_pGOPCtxLocal,GOPTYPE.GOP1);
    }
    else if (MDrv_GOP_Get(g_pGOPCtxLocal)==0)
    {
        i = _GOP_SelGwinId2(pInstance,GOPTYPE.GOP0,0);
        j = MDrv_GOP_GetGwinNum(g_pGOPCtxLocal,GOPTYPE.GOP0);
    }
    else if (MDrv_GOP_Get(g_pGOPCtxLocal)==2)
    {
        i = _GOP_SelGwinId2(pInstance,GOPTYPE.GOP2,0);
        j = MDrv_GOP_GetGwinNum(g_pGOPCtxLocal,GOPTYPE.GOP2);
    }
    else if (MDrv_GOP_Get(g_pGOPCtxLocal)==3)
    {
        i = _GOP_SelGwinId2(pInstance,GOPTYPE.GOP3,0);
        j = MDrv_GOP_GetGwinNum(g_pGOPCtxLocal,GOPTYPE.GOP3);
    }
    else if (MDrv_GOP_Get(g_pGOPCtxLocal)==4)
    {
        i = _GOP_SelGwinId2(pInstance,GOPTYPE.GOP4,0);
        j = MDrv_GOP_GetGwinNum(g_pGOPCtxLocal,GOPTYPE.GOP4);
    }
    else if (MDrv_GOP_Get(g_pGOPCtxLocal)==5)
    {
        i = _GOP_SelGwinId2(pInstance,GOPTYPE.GOP5,0);
        j = MDrv_GOP_GetGwinNum(g_pGOPCtxLocal,GOPTYPE.GOP5);
    }
    else
    {
        return(GWIN_ID_INVALID);
    }

    if(!_GOP_IsGwinIdValid(pInstance,i))
    {
        GOP_M_ERR("[%s][%d]GWIN %d  is out of range\n",__FUNCTION__,__LINE__,i);
        return(GWIN_ID_INVALID);
    }

    k = i+j;

    if (k >= SHARED_GWIN_MAX_COUNT)
    {
        GOP_M_ERR("[%s][%d]GWIN number %d  is out of range\n",__FUNCTION__,__LINE__,k);
        return(GWIN_ID_INVALID);
    }

    for(l=i; l < k; l++)
    {
        if(DRV_MAX_GWIN_FB_SUPPORT<=g_pGOPCtxLocal->pGOPCtxShared->gwinMap[l].u32CurFBId)
        {
            GWID = l;
            break;
        }
    }
    return(GWID);
}
MS_U32 GOP_GWIN_GetFBfromGWIN(void* pInstance,MS_U8 gwinId)
{
	  GOP_WinFB_INFO* pwinFB;

#ifdef INSTANT_PRIVATE
    GOP_INSTANT_PRIVATE* psGOPInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void*)&psGOPInstPri);
#endif

    if (!_GOP_IsGwinIdValid(pInstance,gwinId))
    {
        GOP_M_ERR("[%s][%d]GWIN %d  is out of range\n",__FUNCTION__,__LINE__,gwinId);
        return(GWIN_ID_INVALID);
    }
    if (!_GOP_GWIN_IsGwinExistInClient(pInstance,gwinId))
    {
        GOP_M_ERR("[%s][%d]GWIN %d  is not exist\n",__FUNCTION__,__LINE__,gwinId);
        return(GWIN_ID_INVALID);
    }

    pwinFB = _GetWinFB(pInstance,g_pGOPCtxLocal->pGOPCtxShared->gwinMap[gwinId].u32CurFBId);

    if(pwinFB == NULL)
    {
        GOP_M_ERR("[%s][%d]GetWinFB Fail : WrongFBID=%td\n",__FUNCTION__,__LINE__,(ptrdiff_t)g_pGOPCtxLocal->pGOPCtxShared->gwinMap[gwinId].u32CurFBId);
        return(GWIN_ID_INVALID);
    }
    if (!_GOP_IsFbIdValid(pInstance,g_pGOPCtxLocal->pGOPCtxShared->gwinMap[gwinId].u32CurFBId))
    {
        GOP_M_ERR("[%s][%d]FbId %td  is out of range\n",__FUNCTION__,__LINE__,(ptrdiff_t)g_pGOPCtxLocal->pGOPCtxShared->gwinMap[gwinId].u32CurFBId);
        return(GWIN_ID_INVALID);
    }
    if (0 == pwinFB->in_use)
    {
        GOP_M_ERR("[%s][%d]GWIN %d not allocated\n",__FUNCTION__,__LINE__,gwinId);
        return(GWIN_ID_INVALID);
    }

    return(g_pGOPCtxLocal->pGOPCtxShared->gwinMap[gwinId].u32CurFBId);
}

MS_U32 GOP_FB_Get(void* pInstance)
{
#ifdef INSTANT_PRIVATE
    GOP_INSTANT_PRIVATE* psGOPInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void*)&psGOPInstPri);
#endif

	  if(GOP_WINFB_POOL2 == g_pGOPCtxLocal->pGOPCtxShared->fb_currentPoolId)
    {
        return 1;
    }
    else
    {
        return 0;
    }
/* //Change to if to fix parasoft error
    switch(g_pGOPCtxLocal->pGOPCtxShared->fb_currentPoolId)
    {
        case GOP_WINFB_POOL2:
            return 1;
         default:
            return 0;
    }*/
		return GOP_API_SUCCESS;
}
MS_U32 GOP_FB_SEL(void* pInstance,MS_U8 u8FB)
{
#ifdef INSTANT_PRIVATE
    GOP_INSTANT_PRIVATE* psGOPInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void*)&psGOPInstPri);
#endif

    switch(u8FB)
    {
        case 0: // gwinFB1
            g_pGOPCtxLocal->pGOPCtxShared->fb_currentPoolId = GOP_WINFB_POOL1;
            break;
        case 1: // gwinFB2
            g_pGOPCtxLocal->pGOPCtxShared->fb_currentPoolId = GOP_WINFB_POOL2;
            break;
        default:
            break;
    }
    return GOP_API_SUCCESS;
}
MS_U32 GOP_GWIN_Is32FBExist(void* pInstance,MS_U32 u32fbId)
{
	GOP_WinFB_INFO* pwinFB;

    if (!_GOP_IsFbIdValid(pInstance,u32fbId))
    {
        GOP_M_ERR("[%s][%d]FbId %td  is out of range\n",__FUNCTION__,__LINE__,(ptrdiff_t)u32fbId);
        return FALSE;
    }

    pwinFB = _GetWinFB(pInstance,u32fbId);

    if(pwinFB == NULL)
    {
        GOP_M_ERR("[%s][%d]GetWinFB Fail : WrongFBID=%td\n",__FUNCTION__,__LINE__,(ptrdiff_t)u32fbId);
        return FALSE;
    }

    return pwinFB->obtain;
}

MS_U32 GOP_GWIN_GetFreeFbID(void* pInstance)
{
    MS_U32 i;
    MS_U32 u32FBId = 0xFFFFFFFF;
    GOP_WinFB_INFO* pwinFB;
    for(i=0; i<DRV_MAX_GWIN_FB_SUPPORT; i++)
    {
        if(!GOP_GWIN_Is32FBExist(pInstance,i))
        {
            u32FBId = i;
            pwinFB = _GetWinFB(pInstance,u32FBId);
            pwinFB->obtain = 1;
            break;
        }
    }
    return u32FBId;
}

MS_U32 GOP_GWIN_SetGPUtileMode(void* pInstance,MS_U8 u8Gwin, EN_GOP_GPU_TILE_MODE tilemode)
{
#ifdef INSTANT_PRIVATE
    GOP_INSTANT_PRIVATE* psGOPInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void*)&psGOPInstPri);
#endif

    if (!_GOP_IsGwinIdValid(pInstance,u8Gwin))
    {
        GOP_M_ERR("[%s][%d]GWIN %d  is out of range\n",__FUNCTION__,__LINE__,u8Gwin);
        return GOP_API_FAIL;
    }

    MDrv_GOP_GWIN_SetGPUTileMode(g_pGOPCtxLocal, u8Gwin, (EN_DRV_GOP_GPU_TILE_MODE)tilemode);

    return GOP_API_SUCCESS;
}

MS_U32 GOP_SetGOPClk(void* pInstance,MS_U8 gopNum, EN_GOP_DST_TYPE eDstType)
{
    DRV_GOPDstType GopDst;

#ifdef INSTANT_PRIVATE
    GOP_INSTANT_PRIVATE* psGOPInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void*)&psGOPInstPri);
#endif

    if (!_GOP_IsGopNumValid(pInstance,gopNum))
    {
        GOP_M_ERR("[%s][%d]GOP %d  is out of range\n",__FUNCTION__,__LINE__,gopNum);
        return(GOP_API_FAIL);
    }
    (_GOP_Map_APIDst2DRV_Enum(pInstance,eDstType, &GopDst));
    (MDrv_GOP_SetGOPClk(g_pGOPCtxLocal, gopNum, GopDst));
    return(GOP_API_SUCCESS);
}
MS_U32 GOP_GWIN_SetBnkForceWrite(void* pInstance,MS_U8 u8GOP, MS_BOOL bEnable)
{
#ifdef INSTANT_PRIVATE
    GOP_INSTANT_PRIVATE* psGOPInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void*)&psGOPInstPri);
#endif

    if(g_pGOPCtxLocal == NULL)
    {
        bInitBkFWR[u8GOP] = bEnable;
        return GOP_API_SUCCESS;
    }
    else
    {
        if(!g_pGOPCtxLocal->pGopChipProperty->bBnkForceWrite)
        {
            GOP_M_ERR("[%s][%d] GOP not support bank force write function in this chip version \n",__FUNCTION__,__LINE__);
            MDrv_GOP_GWIN_SetBnkForceWrite(g_pGOPCtxLocal, u8GOP, FALSE);
            return(GOP_API_FAIL);
        }
        else
        {
            bInitBkFWR[u8GOP] = bEnable;
            MDrv_GOP_GWIN_SetBnkForceWrite(g_pGOPCtxLocal, u8GOP, bEnable);
            return(GOP_API_SUCCESS);
        }
    }
}
MS_U32 GOP_GWIN_SetForceWrite(void* pInstance,MS_BOOL bEnable)
{
    //It's used for supernova special used. (by Jupiter)
    //There should no driver request before init.
    //Supernova wants to easly control AP flow, it will set gop force write before gop init.
    //Driver must recode the FRWR status and set it during gop init.
#ifdef INSTANT_PRIVATE
    GOP_INSTANT_PRIVATE* psGOPInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void*)&psGOPInstPri);
#endif

    if(g_pGOPCtxLocal == NULL)
    {
        return GOP_API_SUCCESS;
    }
    else
    {
        g_pGOPCtxLocal->bInitFWR = bEnable;
        MDrv_GOP_GWIN_SetForceWrite(g_pGOPCtxLocal, bEnable);
        return(GOP_API_SUCCESS);
    }
}
MS_U32 GOP_Set3D_LR_FrameExchange(void* pInstance,MS_U8 u8GOP)
{
	GOP_Result enRet = GOP_FAIL;
#ifdef INSTANT_PRIVATE
    GOP_INSTANT_PRIVATE* psGOPInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void*)&psGOPInstPri);
#endif

    if (!_GOP_IsGopNumValid(pInstance,u8GOP))
    {
        GOP_M_ERR("[%s][%d]GOP %d  is out of range\n",__FUNCTION__,__LINE__,u8GOP);
        return(GOP_API_FAIL);
    }

    enRet = MDrv_GOP_Set3D_LR_FrameExchange(g_pGOPCtxLocal, u8GOP);
    MDrv_GOP_GWIN_UpdateReg(g_pGOPCtxLocal,u8GOP);
    return enRet;
}
MS_U32 GOP_Enable_VEOSD(void* pInstance,MS_U8 u8GOP, MS_BOOL bEn)
{
    MS_U8 GOPNum = u8GOP;
    MS_BOOL bret;
    DRV_GOPDstType pGopDst = E_DRV_GOP_DST_INVALID;
#ifdef INSTANT_PRIVATE
    GOP_INSTANT_PRIVATE* psGOPInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void*)&psGOPInstPri);
#endif

    MDrv_GOP_GWIN_GetDstPlane(g_pGOPCtxLocal, u8GOP, &pGopDst);

    if((pGopDst == E_DRV_GOP_DST_MIXER2VE) || (pGopDst == E_DRV_GOP_DST_MIXER2OP) || (pGopDst == E_DRV_GOP_DST_MIXER2OP1))
    {
        GOPNum = GOPTYPE.MIXER-1;
    }
    else if(pGopDst == E_DRV_GOP_DST_INVALID)
    {
        GOP_M_ERR("[%s][%d]Wrong GOP Destination!",__FUNCTION__,__LINE__);
        return GOP_API_FAIL;
    }

    bret = MDrv_GOP_Set_VE_MUX(g_pGOPCtxLocal, GOPNum, bEn);

    if(bret)
    {
        return GOP_API_SUCCESS;
    }
    else
    {
        return GOP_API_FAIL;
    }
}

MS_U32 GOP_OutputLayerSwitch(void* pInstance,MS_U8 u8GOP)
{
    MS_U8 i;
#ifdef INSTANT_PRIVATE
    GOP_INSTANT_PRIVATE* psGOPInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void*)&psGOPInstPri);
#endif

    if (!_GOP_IsGopNumValid(pInstance,u8GOP))
    {
        GOP_M_ERR("[%s][%d]GOP %d  is out of range\n",__FUNCTION__,__LINE__,u8GOP);
        return(GOP_API_FAIL);
    }

    GOP_M_INFO("---%s %d: PID[%td], TID[%td] u8GOP=%u\n", __FUNCTION__, __LINE__, (ptrdiff_t)getpid(), (ptrdiff_t)MsOS_GetOSThreadID(), u8GOP);

    MDrv_GOP_SetGOPHighPri(g_pGOPCtxLocal, u8GOP);
    for (i=0; i<MDrv_GOP_GetMaxGOPNum(g_pGOPCtxLocal); i++)
    {
        _GOP_InitHSPDByGOP(pInstance,i);
    }
    return GOP_API_SUCCESS;
}

MS_U32 GOP_IsRegUpdated(void* pInstance,MS_U8 u8GopType)
{
#ifdef INSTANT_PRIVATE
    GOP_INSTANT_PRIVATE* psGOPInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void*)&psGOPInstPri);
#endif

    if (MDrv_GOP_GetGOPACK(g_pGOPCtxLocal, u8GopType))
    {
        return(GOP_API_SUCCESS);
    }
    return(GOP_API_FAIL);
}

MS_U32 GOP_OC_SetOCFBinfo(void* pInstance,MS_PHY u64Addr, EN_GOP_OC_SEL_TYPE BankMode, MS_U32 enBufSize)
{
    DRV_GOP_OC_INFO OCinfo;
    MS_U16 PanelWidth,PanelHeight;
    E_GOP_API_Result enRet = GOP_API_FAIL;
    MS_U8 u8CurGOP = 0;
#ifdef INSTANT_PRIVATE
    GOP_INSTANT_PRIVATE* psGOPInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void*)&psGOPInstPri);
#endif

    u8CurGOP = MDrv_GOP_Get(g_pGOPCtxLocal);
    memset(&OCinfo, 0, sizeof(DRV_GOP_OC_INFO));

    PanelWidth  = g_pGOPCtxLocal->pGOPCtxShared->u16PnlWidth[u8CurGOP];
    PanelHeight = g_pGOPCtxLocal->pGOPCtxShared->u16PnlHeight[u8CurGOP];

    OCinfo.u16GOP_OC_HSIZE = PanelWidth;
    OCinfo.u16GOP_OC_VSIZE = PanelHeight;
    OCinfo.u32GOP_OC_FETCH = ( ((PanelWidth >>1) + (PanelWidth /10)) << 2) * PanelHeight;                               // PanelWidth *0.6(Compression Ratio) *4(8888fmt)

    if(enBufSize < (OCinfo.u32GOP_OC_FETCH << BankMode))
    {
        GOP_M_ERR("[%s] ERROR, OC Buffer size not enough , input BufSize:0x%tx\n",__FUNCTION__,(ptrdiff_t)enBufSize);
        return (GOP_API_FAIL);
    }
    else
    {
        switch(BankMode)
        {
            case E_GOP_OC_SEL_2BankMode:
                OCinfo.u8GOP_OC_BANK_MODE = 1;
                break;
            case E_GOP_OC_SEL_4BankMode:
                OCinfo.u8GOP_OC_BANK_MODE = 0;
                break;
            default:
                OCinfo.u8GOP_OC_BANK_MODE = 1;
                break;
        }
    }

    OCinfo.u64GOP_OC_DRAM_RBLK = u64Addr ;
    OCinfo.u64GOP_OC_ADDRPROTECT_STR = u64Addr;
    OCinfo.u64GOP_OC_ADDRPROTECT_END = u64Addr + enBufSize;

    enRet = (E_GOP_API_Result)MDrv_GOP_OC_SetOCInfo(g_pGOPCtxLocal,&OCinfo);

    return(enRet);
}

MS_U32 GOP_MIXER_SetOutputTiming(void* pInstance,GOP_MIXER_TIMINGTYPE mode,  GOP_MixerTiming *pMT)
{
	E_GOP_API_Result enRet=GOP_API_FAIL;
    GOP_DRV_MixerTiming MT;
#ifdef INSTANT_PRIVATE
    GOP_INSTANT_PRIVATE* psGOPInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void*)&psGOPInstPri);
#endif

    if(pMT != NULL)
    {
        MT.hstart = pMT->hstart;
        MT.vstart = pMT->vstart;
        MT.vend = pMT->vend;
        MT.hend = pMT->hend;
        MT.htotal = pMT->htotal;
        MT.hsyncdelay = 0;
    }

    if(pMT == NULL)
    {
        return(GOP_API_INVALID_PARAMETERS);
    }

    enRet = (E_GOP_API_Result)MDrv_GOP_MIXER_SetOutputTiming(g_pGOPCtxLocal, mode, &MT);
    return(enRet);
}
MS_U32 GOP_MIXER_SetMIXER2OPOutputTiming(void* pInstance,GOP_Mixer2OPTiming *pMT)
{
	E_GOP_API_Result enRet=GOP_API_FAIL;
    GOP_DRV_MixerTiming MT;
#ifdef INSTANT_PRIVATE
    GOP_INSTANT_PRIVATE* psGOPInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void*)&psGOPInstPri);
#endif


    if(pMT != NULL)
    {
        MT.hstart = pMT->hstart;
        MT.vstart = pMT->vstart;
        MT.vend = pMT->vend;
        MT.hend = pMT->hend;
        MT.htotal = pMT->htotal;
        MT.hsyncdelay = pMT->hsyncdelay;
    }

    if(pMT == NULL)
    {
        return(GOP_API_INVALID_PARAMETERS);
    }

    enRet = (E_GOP_API_Result)MDrv_GOP_MIXER_SetOutputTiming(g_pGOPCtxLocal, GOP_CUSTOM_OP , &MT);
    return(enRet);
}
MS_U32 GOP_MIXER_EnableVfilter(void* pInstance,MS_BOOL bEnable)
{
	E_GOP_API_Result enRet=GOP_API_FAIL;
#ifdef INSTANT_PRIVATE
    GOP_INSTANT_PRIVATE* psGOPInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void*)&psGOPInstPri);
#endif
    enRet = (E_GOP_API_Result)MDrv_GOP_MIXER_EnableVfilter(g_pGOPCtxLocal, bEnable);

    return(enRet);
}

MS_U32 Ioctl_GOP_Get_ScaleDst(void* pInstance,MS_U8 u8GOPNum,MS_U16 *pHScaleDst,MS_U16 *pVScaleDst)
{
#ifdef INSTANT_PRIVATE
    GOP_INSTANT_PRIVATE* psGOPInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void*)&psGOPInstPri);
#endif

    *pHScaleDst = g_pGOPCtxLocal->pGOPCtxShared->u16HScaleDst[u8GOPNum];
    *pVScaleDst = g_pGOPCtxLocal->pGOPCtxShared->u16VScaleDst[u8GOPNum];
		return GOP_API_SUCCESS;
}

//==================================================================
//////////////////////////////////////////////////
//  - GOP ioctl function.
//////////////////////////////////////////////////

MS_U32 Ioctl_GOP_Init(void* pInstance,MS_U8 u8GOP,GOP_InitInfo *pGopInit)
{
    MS_BOOL bFirstInstance = FALSE;
    MS_BOOL resCtxInit;
    DRV_GOPDstType GopDst;
#ifdef INSTANT_PRIVATE
    GOP_INSTANT_PRIVATE* psGOPInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void*)&psGOPInstPri);
#endif

    //NULL pointer protect
    if(NULL == pGopInit)
    {
        GOP_M_INFO("[%s][%d] input parameter is NULL\n",__FUNCTION__,__LINE__);
        //GOP_LIB_VER GOPLib;
        resCtxInit = _GOP_Init_Ctx(pInstance,&bFirstInstance);
        APIGOP_ASSERT(resCtxInit, GOP_M_FATAL("failed to init GOP context\n"));
        if (bFirstInstance)
        {
            g_pGOPCtxLocal->pGOPCtxShared->bDummyInit = TRUE;
        }

        if (MDrv_GOP_SetIOMapBase(g_pGOPCtxLocal) != TRUE)
        {
            APIGOP_ASSERT(FALSE, GOP_M_FATAL("\nget IO base fail"));
            return GOP_API_FAIL;
        }
    }
    else
    {
        //GOP_LIB_VER GOPLib;
        resCtxInit = _GOP_Init_Ctx(pInstance,&bFirstInstance);
        APIGOP_ASSERT(resCtxInit, GOP_M_FATAL("failed to init GOP context\n"));
        bFirstInit = FALSE;
        if (g_pGOPCtxLocal->pGOPCtxShared->bDummyInit)
        {
            //other GOP init should work as the first instance flow
            bFirstInstance = TRUE;
            g_pGOPCtxLocal->pGOPCtxShared->bInitShared = TRUE;
            g_pGOPCtxLocal->pGOPCtxShared->bDummyInit = FALSE;
        }

        if (!_GOP_IsGopNumValid(pInstance,u8GOP))
        {
            GOP_M_ERR("[%s][%d]GOP %d  is out of range\n",__FUNCTION__,__LINE__,u8GOP);
            return GOP_API_FAIL;
        }

        //init GOP global variable
        if(g_pGOPCtxLocal->pGOPCtxShared->bInitShared)
        {
            g_pGOPCtxLocal->Gwin_H_Dup = FALSE;
            g_pGOPCtxLocal->Gwin_V_Dup = FALSE;
            g_pGOPCtxLocal->current_winId = GOP0_Gwin0Id;
            g_pGOPCtxLocal->pGOPCtxShared->fbPool1.GWinFB_Pool_BaseAddr = pGopInit->u32GOPRBAdr;
            g_pGOPCtxLocal->pGOPCtxShared->fbPool1.u32GWinFB_Pool_MemLen = pGopInit->u32GOPRBLen;
            g_pGOPCtxLocal->pGOPCtxShared->fbPool1.u32FirstFBIdInPool = INVALID_POOL_NEXT_FBID;
            g_pGOPCtxLocal->pGOPCtxShared->fbPool1.poolId= GOP_WINFB_POOL1;
            g_pGOPCtxLocal->pGOPCtxShared->fbPool2.GWinFB_Pool_BaseAddr = 0;
            g_pGOPCtxLocal->pGOPCtxShared->fbPool2.u32GWinFB_Pool_MemLen = 0;
            g_pGOPCtxLocal->pGOPCtxShared->fbPool2.u32FirstFBIdInPool = INVALID_POOL_NEXT_FBID;
            g_pGOPCtxLocal->pGOPCtxShared->fbPool2.poolId= GOP_WINFB_POOL2;
            g_pGOPCtxLocal->pGOPCtxShared->fb_currentPoolId = GOP_WINFB_POOL1;
            g_pGOPCtxLocal->pGOPCtxShared->phyGOPRegdmaAdr = pGopInit->u32GOPRegdmaAdr;
            g_pGOPCtxLocal->pGOPCtxShared->u32GOPRegdmaLen = pGopInit->u32GOPRegdmaLen;
        }

        if(pGopInit->bEnableVsyncIntFlip == TRUE)
            g_pGOPCtxLocal->pGOPCtxShared->bEnableVsyncIntFlip = TRUE;

        if (MDrv_GOP_SetIOMapBase(g_pGOPCtxLocal) != TRUE)
        {
            APIGOP_ASSERT(FALSE, GOP_M_FATAL("\nget IO base fail"));
            return GOP_API_FAIL;
        }

        g_pGOPCtxLocal->pGOPCtxShared->u16PnlWidth[u8GOP] = pGopInit->u16PanelWidth;
        g_pGOPCtxLocal->pGOPCtxShared->u16PnlHeight[u8GOP] = pGopInit->u16PanelHeight;
        g_pGOPCtxLocal->pGOPCtxShared->u16PnlHStr[u8GOP] = pGopInit->u16PanelHStr;

        MDrv_GOP_GWIN_ForceWrite_Update(g_pGOPCtxLocal, u8GOP, TRUE);//if use double buffer write in GOP init state, GOP HW cannot wait V-Sync to update register

        if(FALSE == g_pGOPCtxLocal->pGOPCtxShared->bGopHasInitialized[u8GOP])
        {

            if(g_pGOPCtxLocal->pGopChipProperty->bPixelModeSupport)
            {
                g_pGOPCtxLocal->pGOPCtxShared->bPixelMode[u8GOP] = TRUE;
            }
            //Not initialized yet:
            GOP_GWIN_InitByGOP(pInstance,bFirstInstance, u8GOP);

	    if(!(g_pGOPCtxLocal->sGOPConfig.eGopIgnoreInit & E_GOP_IGNORE_GWIN))
        	    MDrv_GOP_Init(g_pGOPCtxLocal, u8GOP, pGopInit->u32GOPRegdmaAdr, pGopInit->u32GOPRegdmaLen, pGopInit->bEnableVsyncIntFlip);
    #if (FPGA_TEST)
            MDrv_GOP_GWIN_EnableProgressive(g_pGOPCtxLocal, u8GOP, TRUE);
            MDrv_GOP_GWIN_SetDstPlane(g_pGOPCtxLocal, u8GOP, E_DRV_GOP_DST_VOP,FALSE);
            MDrv_GOP_GWIN_SetStretchWin(u8GOP, E_GOP_DST_VOP,0, 0, g_pGOPCtxLocal->pGOPCtxShared->u16PnlWidth, g_pGOPCtxLocal->pGOPCtxShared->u16PnlHeight) ;

            (_GOP_Map_APIDst2DRV_Enum(pInstance,E_GOP_DST_VOP, &GopDst));
            MDrv_GOP_SetGOPClk(g_pGOPCtxLocal, u8GOP, E_GOP_DST_VOP);
            MDrv_GOP_GWIN_OutputColor(g_pGOPCtxLocal,u8GOP,DRV_GOPOUT_RGB);
    #else
            if(!(g_pGOPCtxLocal->sGOPConfig.eGopIgnoreInit & E_GOP_IGNORE_SET_DST_OP))
            {
                MDrv_GOP_GWIN_EnableProgressive(g_pGOPCtxLocal, u8GOP, TRUE);
			    MDrv_GOP_GWIN_SetAlphaInverse(g_pGOPCtxLocal,u8GOP,TRUE);
                MDrv_GOP_GWIN_SetDstPlane(g_pGOPCtxLocal, u8GOP, E_DRV_GOP_DST_OP0,FALSE);
                (_GOP_Map_APIDst2DRV_Enum(pInstance,E_GOP_DST_OP0, &GopDst));
                MDrv_GOP_SetGOPClk(g_pGOPCtxLocal, u8GOP, GopDst);
            }

            if(!(g_pGOPCtxLocal->sGOPConfig.eGopIgnoreInit & E_GOP_IGNORE_STRETCHWIN))
                MDrv_GOP_GWIN_SetStretchWin(g_pGOPCtxLocal,u8GOP,0, 0, g_pGOPCtxLocal->pGOPCtxShared->u16PnlWidth[u8GOP], g_pGOPCtxLocal->pGOPCtxShared->u16PnlHeight[u8GOP]) ;
    #endif
            if (u8GOP==0)
            {
                MDrv_GOP_SetIPSel2SC(g_pGOPCtxLocal, MS_DRV_NIP_SEL_GOP0);
            }
            else if (u8GOP==1)
            {
                MDrv_GOP_SetIPSel2SC(g_pGOPCtxLocal, MS_DRV_NIP_SEL_GOP1);
            }
            else if (u8GOP==2)
            {
                MDrv_GOP_SetIPSel2SC(g_pGOPCtxLocal, MS_DRV_NIP_SEL_GOP2);
            }
            if(!(g_pGOPCtxLocal->sGOPConfig.eGopIgnoreInit & E_GOP_IGNORE_ENABLE_TRANSCLR))
                MDrv_GOP_GWIN_EnableTransClr(g_pGOPCtxLocal,u8GOP, (GOP_TransClrFmt)0, FALSE);

            if(!g_pGOPCtxLocal->pGopChipProperty->bInitNotEnableGOPToSC)  //u4 mux init setting, enable GOP to SC in GOP init would cause problem
                MDrv_GOP_SetGOPEnable2SC(g_pGOPCtxLocal, u8GOP, TRUE);
            if(!(g_pGOPCtxLocal->sGOPConfig.eGopIgnoreInit & E_GOP_IGNORE_GWIN))
            {
                 g_pGOPCtxLocal->pGOPCtxShared->bGopHasInitialized[u8GOP] = TRUE;
            }
        }

        MDrv_GOP_PerClientInit(g_pGOPCtxLocal, u8GOP, pGopInit->u32GOPRegdmaAdr, pGopInit->u32GOPRegdmaLen, pGopInit->bEnableVsyncIntFlip);
        _GOP_InitHSPDByGOP(pInstance,u8GOP); //Fix me, temporary solution for removing calling of "MApi_GOP_GWIN_OutputLayerSwitch"

        if(bFirstInstance)
        {
            MDrv_GOP_SetVOPNBL(g_pGOPCtxLocal,FALSE);
        }
        MDrv_GOP_GWIN_ForceWrite_Update(g_pGOPCtxLocal, u8GOP, FALSE);
    }

    //restore force write setting
    MDrv_GOP_GWIN_SetForceWrite(g_pGOPCtxLocal, g_pGOPCtxLocal->bInitFWR);
    MDrv_GOP_GWIN_SetBnkForceWrite(g_pGOPCtxLocal,u8GOP,bInitBkFWR[u8GOP]);
// apiGOP_DBG("[OUT]\n");
    return GOP_API_SUCCESS;
}


MS_U32 Ioctl_GOP_GetCaps(void* pInstance,EN_GOP_CAPS eCapType, MS_U32* pRet, MS_U32 ret_size)
{
#ifdef INSTANT_PRIVATE
    GOP_INSTANT_PRIVATE* psGOPInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void*)&psGOPInstPri);
#endif

    if(eCapType != E_GOP_CAP_VERSION)
        APIGOP_CHECK_INIT();

    E_GOP_API_Result enRet = GOP_API_SUCCESS;
    switch(eCapType)
    {
        case E_GOP_CAP_WORD_UNIT:
        {
            *((MS_U16 *)pRet) = g_pGOPCtxLocal->pGopChipProperty->WordUnit;
            break;
        }
        case E_GOP_CAP_GWIN_NUM:
        {
            *((MS_U16*)pRet) = g_pGOPCtxLocal->pGopChipProperty->TotalGwinNum;
            break;
         }
        case E_GOP_CAP_VERSION:
        {
            GOP_LIB_VER * pLibVer;

#ifndef MSOS_TYPE_LINUX_KERNEL //Check size error when if(is_compat_task()==1)
            if(ret_size != sizeof(GOP_LIB_VER))
            {
                GOP_M_ERR("[%s] ERROR, invalid input structure size :%td on E_GOP_CAP_VERSION\n",__FUNCTION__,(ptrdiff_t)ret_size);
                return GOP_API_FAIL;
             }
#endif
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

            break;
        }
        case E_GOP_CAP_CONSALPHA_VALIDBITS:
            {
                *((DRV_GOP_CONSALPHA_BITS*)pRet) = g_pGOPCtxLocal->pGopChipProperty->Default_ConsAlpha_bits;
                break;
            }
        case E_GOP_CAP_PIXELMODE_SUPPORT:
            {
                *((MS_BOOL*)pRet) = g_pGOPCtxLocal->pGopChipProperty->bPixelModeSupport;
                break;
            }
        case E_GOP_CAP_STRETCH:
        {
            if(pRet != NULL)
            {
                MS_U8 i;
                PGOP_CAP_STRETCH_INFO pGOPStretchInfo= (PGOP_CAP_STRETCH_INFO)pRet;
#ifndef MSOS_TYPE_LINUX_KERNEL //Check size error when if(is_compat_task()==1)
                if(ret_size != sizeof(GOP_CAP_STRETCH_INFO))
                {
                    GOP_M_ERR("[%s] ERROR, invalid input structure size :%td on E_GOP_CAP_STRETCH\n",__FUNCTION__,(ptrdiff_t)ret_size);
                    return GOP_API_FAIL;
                }
#endif
                pGOPStretchInfo->GOP_VStretch_Support=0;
                for(i=0;i<MIN(sizeof(g_pGOPCtxLocal->pGopChipProperty->bGOPWithVscale),(sizeof(pGOPStretchInfo->GOP_VStretch_Support)*8));i++)
                {
                    if(g_pGOPCtxLocal->pGopChipProperty->bGOPWithVscale[i])
                    {
                        pGOPStretchInfo->GOP_VStretch_Support |= BIT(i);
                    }
                    else
                    {
                        pGOPStretchInfo->GOP_VStretch_Support &= ~(BIT(i));
                    }
                }
            }
            else
            {
                APIGOP_ASSERT(0, GOP_M_FATAL("Invalid input parameter: NULL pointer\n"));
            }
            break;
        }
        case E_GOP_CAP_AFBC_SUPPORT:
        {
            if(pRet != NULL)
            {
                MS_U8 i;
                PGOP_CAP_AFBC_INFO pGOPAFBCInfo= (PGOP_CAP_AFBC_INFO)pRet;
                if(ret_size != sizeof(GOP_CAP_AFBC_INFO))
                {
                    GOP_M_ERR("[%s] ERROR, invalid input structure size :%td on E_GOP_CAP_AFBC_MODE_SUPPORT\n",__FUNCTION__,(ptrdiff_t)ret_size);
                    return GOP_API_FAIL;
                }
                pGOPAFBCInfo->GOP_AFBC_Support=0;
                for(i=0; i < MDrv_GOP_GetMaxGOPNum(g_pGOPCtxLocal); i++)
                {
                    if(g_pGOPCtxLocal->pGopChipProperty->bAFBC_Support[i])
                    {
                        pGOPAFBCInfo->GOP_AFBC_Support |= BIT(i);
                    }
                    else
                    {
                        pGOPAFBCInfo->GOP_AFBC_Support &= ~(BIT(i));
                    }
                }
            }
            else
            {
                APIGOP_ASSERT(0, GOP_M_ERR("Invalid input parameter: NULL pointer\n"));
            }
            break;
        }
        case E_GOP_CAP_BNKFORCEWRITE:
        {
            *pRet = g_pGOPCtxLocal->pGopChipProperty->bBnkForceWrite;
            break;
        }
        case E_GOP_CAP_BRIGHTNESS_INFO:
        {
            if(g_pGOPCtxLocal->pGopChipProperty->enBrightnessInfo == E_DRV_BRI_SIGNBIT)
            {
                *pRet = E_GOP_BRIG_SIGNBIT;
            }
            else if(g_pGOPCtxLocal->pGopChipProperty->enBrightnessInfo == E_DRV_BRI_2SCOMPLEMENT)
            {
                *pRet = E_GOP_BRIG_2SCOMPLEMENT;
            }
            else
            {
                enRet = GOP_API_FAIL;
            }
            break;
        }
        case E_GOP_CAP_SUPPORT_V4TAP_256PHASE:
        {
            if(pRet != NULL)
            {
                PGOP_CAP_STRETCH_INFO pstGOPStretchInfo = (PGOP_CAP_STRETCH_INFO)pRet;
                MS_U8 u8GOPNum = 0;
                if(ret_size != sizeof(GOP_CAP_STRETCH_INFO))
                {
                    GOP_M_ERR("[%s] ERROR, invalid input structure size :%td on E_GOP_CAP_SUPPORT_V4TAP_256PHASE\n",__FUNCTION__,(ptrdiff_t)ret_size);
                    return GOP_API_FAIL;
                }
                for(u8GOPNum = 0; u8GOPNum < GOP_MAX_SUPPORT_DEFAULT; u8GOPNum++)
                {
                    if(g_pGOPCtxLocal->pGopChipProperty->bSupportV4tap[u8GOPNum])
                    {
                        pstGOPStretchInfo->GOP_VStretch_Support |= BIT(u8GOPNum);
                    }
                    else
                    {
                        pstGOPStretchInfo->GOP_VStretch_Support &= ~(BIT(u8GOPNum));
                    }
                }
            }
            break;
        }
        case E_GOP_CAP_SUPPORT_V_SCALING_BILINEAR:
        {
            if(pRet != NULL)
            {
                PGOP_CAP_STRETCH_INFO pstGOPStretchInfo = (PGOP_CAP_STRETCH_INFO)pRet;
                MS_U8 u8GOPNum = 0;
                if(ret_size != sizeof(GOP_CAP_STRETCH_INFO))
                {
                    GOP_M_ERR("[%s] ERROR, invalid input structure size :%td on E_GOP_CAP_SUPPORT_V_SCALING_BILINEAR\n",__FUNCTION__,(ptrdiff_t)ret_size);
                    return GOP_API_FAIL;
                }
                for(u8GOPNum = 0; u8GOPNum < GOP_MAX_SUPPORT_DEFAULT; u8GOPNum++)
                {
                    if(g_pGOPCtxLocal->pGopChipProperty->bSupportV_BiLinear[u8GOPNum])
                    {
                        pstGOPStretchInfo->GOP_VStretch_Support |= BIT(u8GOPNum);
                    }
                    else
                    {
                        pstGOPStretchInfo->GOP_VStretch_Support &= ~(BIT(u8GOPNum));
                    }
                }
            }
            break;
        }
        case E_GOP_CAP_SUPPORT_H4TAP_256PHASE:
        {
            *pRet = (MS_U32)g_pGOPCtxLocal->pGopChipProperty->bH4tap256Phase;
            break;
        }
        case E_GOP_CAP_SUPPORT_MIXER_SDR2HDR:
        {
            *pRet = (MS_U32)g_pGOPCtxLocal->pGopChipProperty->bSupportMixerSDR2HDR;
            break;
        }
        case E_GOP_CAP_SUPPORT_CSC:
        {
            *pRet = (MS_BOOL)g_pGOPCtxLocal->pGopChipProperty->bSupportCSCTuning;
            break;
        }
        default:
            break;
    }

    return enRet;
}


MS_U32 Ioctl_GOP_SetConfig(void* pInstance,EN_GOP_CONFIG_TYPE cfg_type,MS_U32* pCfg,MS_U32 u32Size)
{
    MS_BOOL bFirstInstance;
    MS_BOOL resCtxInit;
    EN_GOP_IGNOREINIT *pGOPIgnoreInit;
    DRV_GOP_CONSALPHA_BITS *pGOPAlphaBits;
    ST_GOP_AUTO_DETECT_BUF_INFO *pstGopAutoDetectInfo;
#ifdef INSTANT_PRIVATE
    GOP_INSTANT_PRIVATE* psGOPInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void*)&psGOPInstPri);
#endif

    switch(cfg_type)
    {
    case E_GOP_IGNOREINIT:
        resCtxInit = GOP_PreInit_Ctx(pInstance,&bFirstInstance);
        APIGOP_ASSERT(resCtxInit, GOP_M_FATAL("failed to init GOP context\n"));

        pGOPIgnoreInit = (EN_GOP_IGNOREINIT *)(pCfg);
        g_pGOPCtxLocal->sGOPConfig.eGopIgnoreInit = (DRV_GOP_IGNOREINIT)(*pGOPIgnoreInit);
        /*To control that do not affect the normal flow in MApi_GOP_Init and MApi_GOP_InitByGOP*/
        if(E_GOP_IGNORE_DISABLE != *pGOPIgnoreInit)
        {
            bFirstInit = TRUE;
        }
        break;
    case E_GOP_CONSALPHA_BITS:


        pGOPAlphaBits = (DRV_GOP_CONSALPHA_BITS *)(pCfg);
        *(g_pGOPCtxLocal->pGopConsAlphaBits) =  *pGOPAlphaBits;
        break;
    case E_GOP_AUTO_DETECT_BUFFER:
    {
        pstGopAutoDetectInfo = (ST_GOP_AUTO_DETECT_BUF_INFO *)(pCfg);

        if(pstGopAutoDetectInfo == NULL)
        {
            GOP_M_ERR("[%s][%d] pstGopAutoDetectInfo is NULL  ERROR!! \n",__FUNCTION__,__LINE__);
            return GOP_API_FAIL;
        }
        if(MDrv_GOP_AutoDectBuf(g_pGOPCtxLocal, pstGopAutoDetectInfo) != GOP_SUCCESS)
        {
            return GOP_API_FAIL;
        }
        break;
    }
    default:

        break;
    }

    return GOP_API_SUCCESS;




}

MS_U32 Ioctl_GOP_GetConfig(void* pInstance,EN_GOP_CONFIG_TYPE cfg_type,MS_U32* pCfg,MS_U32 u32Size)
{
#ifdef INSTANT_PRIVATE
    GOP_INSTANT_PRIVATE* psGOPInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void*)&psGOPInstPri);
#endif

    switch(cfg_type)
    {
    case E_GOP_IGNOREINIT:
    {
        EN_GOP_IGNOREINIT *penGOPIgnoreInit;
        penGOPIgnoreInit = (EN_GOP_IGNOREINIT*)pCfg;
        *penGOPIgnoreInit = (EN_GOP_IGNOREINIT)g_pGOPCtxLocal->sGOPConfig.eGopIgnoreInit;
        break;
    }
    case E_GOP_CONSALPHA_BITS:
    {
        DRV_GOP_CONSALPHA_BITS *penGOPAlphaBits;
        penGOPAlphaBits = (DRV_GOP_CONSALPHA_BITS *)pCfg;
        *penGOPAlphaBits = *(g_pGOPCtxLocal->pGopConsAlphaBits);
        break;
    }
    default:

        break;
    }

    return GOP_API_SUCCESS;
}

MS_U32 Ioctl_GOP_SetProperty(void* pInstance,EN_GOP_PROPERTY type,MS_U32 gop, MS_U32* pSet, MS_U32 u32Size)
{
#ifdef INSTANT_PRIVATE
    GOP_INSTANT_PRIVATE* psGOPInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void*)&psGOPInstPri);
#endif

    switch(type)
    { 
        case E_GOP_CONTRAST:
        {
            PGOP_CONTRAST pRGB = (PGOP_CONTRAST)pSet;
            MS_U32 u32Ret = 0;
            if(u32Size != sizeof(GOP_CONTRAST))
            {
                GOP_M_ERR("[%s][%d] input structure size ERROR!! \n",__FUNCTION__,__LINE__);
                return GOP_API_FAIL;
            }

            u32Ret = GOP_SetContrast(pInstance,gop,pRGB->y, pRGB->u, pRGB->v);

            if(GOP_API_SUCCESS != (E_GOP_API_Result)u32Ret)
            {
                if(GOP_API_FUN_NOT_SUPPORTED == (E_GOP_API_Result)u32Ret)
                {
                    return GOP_API_FUN_NOT_SUPPORTED;
                }
                else
                {
                    return GOP_API_FAIL;
                }
            }
            break;
        }
        case E_GOP_BRIGHTNESS:
        {
            PGOP_BRIGHTNESS pBri = (PGOP_BRIGHTNESS)pSet;
            if(u32Size != sizeof(GOP_BRIGHTNESS))
            {
                GOP_M_ERR("[%s][%d] input structure size ERROR!! \n",__FUNCTION__,__LINE__);
                return GOP_API_FAIL;
            }

            GOP_SetBrightness(pInstance,gop,pBri->Brightness, pBri->bMSB);
            break;
        }
        case E_GOP_OUTPUT_COLOR:
        {
            EN_GOP_OUTPUT_COLOR output;
            if(u32Size != sizeof(EN_GOP_OUTPUT_COLOR))
            {
                GOP_M_ERR("[%s][%d] input structure size ERROR!! \n",__FUNCTION__,__LINE__);
                return GOP_API_FAIL;
            }

            output = *((EN_GOP_OUTPUT_COLOR*)pSet);
            GOP_SetOutputColor(pInstance,gop,output);

            break;

        }
        case E_GOP_UV_SWAP:
        {
            MS_BOOL bEn;
            if(u32Size != sizeof(MS_BOOL))
            {
                GOP_M_ERR("[%s][%d] input structure size ERROR!! \n",__FUNCTION__,__LINE__);
                return GOP_API_FAIL;
            }

            bEn = *((MS_BOOL*)pSet);
            GOP_SetUVSwap(pInstance,gop,bEn);

            break;
        }
        case E_GOP_YC_SWAP:
        {
            MS_BOOL bEn;
            if(u32Size != sizeof(MS_BOOL))
            {
                GOP_M_ERR("[%s][%d] input structure size ERROR!! \n",__FUNCTION__,__LINE__);
                return GOP_API_FAIL;
            }

            bEn = *((MS_BOOL*)pSet);
            GOP_SetYCSwap(pInstance,gop,bEn);

            break;
        }
        case E_GOP_HSTART:
        {
            MS_U16 u16PanelHStr;

            if(u32Size != sizeof(MS_U16))
            {
                GOP_M_ERR("[%s][%d] input structure size ERROR!! \n",__FUNCTION__,__LINE__);
                return GOP_API_FAIL;
            }

            u16PanelHStr = *((MS_U16*)pSet);
            GOP_SetHStart(pInstance,gop,u16PanelHStr);

            break;
        }
        case E_GOP_TRANSCOLOR:
        {
            PGOP_TRANSCOLOR ptrans_clr;
            MS_U32 clr;
            MS_U32 fullclr = 0;
            EN_GOP_TRANSCLR_FMT fmt;

            if(u32Size != sizeof(GOP_TRANSCOLOR))
            {
                GOP_M_ERR("[%s][%d] input structure size ERROR!! \n",__FUNCTION__,__LINE__);
                return GOP_API_FAIL;
            }

            ptrans_clr = (PGOP_TRANSCOLOR)pSet;

            clr = ptrans_clr->color;
            fmt = ptrans_clr->fmt;

         if(ptrans_clr->transclr_property == EN_TRANSCLR_SET_COLOR)
        {
            if(fmt == GOPTRANSCLR_FMT0)
            {
                /***** !RGB555 *****/
                fullclr = ((clr>>10)&0x1f)<<3;      // R
                fullclr |= ((clr>>12)&0x7);
                fullclr = fullclr << 8;
                fullclr |= ((clr>>5)&0x1f)<<3;      // G
                fullclr |= ((clr>>7)&0x7);
                fullclr = fullclr << 8;
                fullclr |= (clr&0x1f)<<3;           // B
                fullclr |= ((clr>>2)&0x7);           // B
                MDrv_GOP_GWIN_SetTransClr_8888(g_pGOPCtxLocal,gop, fullclr, 0);
            }
            else if(fmt == GOPTRANSCLR_FMT2)  //ARGB8888
            {
                fullclr = ptrans_clr->color;
                MDrv_GOP_GWIN_SetTransClr_8888(g_pGOPCtxLocal,gop, fullclr, 0);
            }
            else if(fmt == GOPTRANSCLR_FMT3)
            {
                fullclr = (clr&0xff)<<16;  //V
                fullclr |= ((clr>>8)&0xff)<<8; //Y
                fullclr |= ((clr>>16)&0xff);    //U
                MDrv_GOP_GWIN_SetTransClr_YUV(g_pGOPCtxLocal, gop, fullclr, 0x0);
            }
            else
            {
                GOP_M_ERR("[%s][%d] FORMAT SETTING ERROR!! \n",__FUNCTION__,__LINE__);
                return GOP_API_FAIL;
            }
            //MDrv_GOP_GWIN_SetTransClr_8888(g_pGOPCtxLocal,gop, fullclr, 0);
         }
         else if(ptrans_clr->transclr_property == EN_TRANSCLR_ENABLE)
         {
            //MDrv_GOP_GWIN_EnableTransClr(g_pGOPCtxLocal,gop, (GOP_TransClrFmt)GOPTRANSCLR_FMT0,ptrans_clr->bEn);
            //Return fail when wrong format is derivered
            if((GOP_TransClrFmt)fmt > DRV_GOPTRANSCLR_FMT3)
            {
                GOP_M_ERR("[%s][%d] FORMAT SETTING ERROR!! \n",__FUNCTION__,__LINE__);
                return GOP_API_FAIL;
            }
            MDrv_GOP_GWIN_EnableTransClr(g_pGOPCtxLocal,gop, (GOP_TransClrFmt)fmt,ptrans_clr->bEn);
         }
            break;

        }
        case E_GOP_PROGRESSIVE:
        {
            MS_BOOL bEn;
            if(u32Size != sizeof(MS_BOOL))
            {
                GOP_M_ERR("[%s][%d] input structure size ERROR!! \n",__FUNCTION__,__LINE__);
                return GOP_API_FAIL;
            }

            bEn = *((MS_BOOL*)pSet);
            GOP_SetProgressive(pInstance,gop,bEn);
            break;
        }
        case E_GOP_ALPHAINVERSE:
        {
            MS_BOOL bEn;
            if(u32Size != sizeof(MS_BOOL))
            {
                GOP_M_ERR("[%s][%d] input structure size ERROR!! \n",__FUNCTION__,__LINE__);
                return GOP_API_FAIL;
            }
            bEn = *((MS_BOOL*)pSet);
            GOP_SetAlphaInverse(pInstance,gop, bEn);
            break;
        }
        case E_GOP_PREALPHAMODE:
        {
            MS_BOOL bEn;
            if(u32Size != sizeof(MS_BOOL))
            {
                GOP_M_ERR("[%s][%d] input structure size ERROR!! \n",__FUNCTION__,__LINE__);
                return GOP_API_FAIL;
            }
            bEn = *((MS_BOOL*)pSet);

            MDrv_GOP_SetGOPEnable2Mode1(g_pGOPCtxLocal, gop, bEn);

            break;
        }
        case E_GOP_OUTPUT_TIMING:
        {
            PST_GOP_TIMING_INFO pstInfo;
            if(u32Size != sizeof(ST_GOP_TIMING_INFO))
            {
                GOP_M_ERR("[%s][%d] input structure size ERROR!! \n",__FUNCTION__,__LINE__);
                return GOP_API_FAIL;
            }
            pstInfo = (PST_GOP_TIMING_INFO)pSet;
            GOP_SetPanelTiming(pInstance,gop, pstInfo);
            break;
        }
        case E_GOP_PIXELBASE_ENABLE:
        {
            if(u32Size != sizeof(MS_BOOL))
            {
                GOP_M_ERR("[%s][%d] input structure size ERROR!! \n",__FUNCTION__,__LINE__);
                return GOP_API_FAIL;
            }
            g_pGOPCtxLocal->pGOPCtxShared->bPixelMode[gop]=*((MS_BOOL*)pSet);
            break;
        }
        case E_GOP_CLK:
        {
            EN_GOP_DST_TYPE eDstType;
            if(u32Size != sizeof(EN_GOP_DST_TYPE))
            {
                GOP_M_ERR("[%s][%d] input structure size ERROR!! \n",__FUNCTION__,__LINE__);
                return GOP_API_FAIL;
            }
            eDstType = *((EN_GOP_DST_TYPE*)pSet);
            GOP_SetGOPClk(pInstance,gop,eDstType);
            break;
        }
        case E_GOP_FORCE_WRITE:
        {
            MS_BOOL bEn;
            if(u32Size != sizeof(MS_BOOL))
            {
                GOP_M_ERR("[%s][%d] input structure size ERROR!! \n",__FUNCTION__,__LINE__);
                return GOP_API_FAIL;
            }
            bEn = *((MS_BOOL*)pSet);
            GOP_GWIN_SetForceWrite(pInstance,bEn);
            break;
        }
        case E_GOP_BANK_FORCE_WRITE:
        {
            MS_BOOL bEn;
            if(u32Size != sizeof(MS_BOOL))
            {
                GOP_M_ERR("[%s][%d] input structure size ERROR!! \n",__FUNCTION__,__LINE__);
                return GOP_API_FAIL;
            }
            bEn = *((MS_BOOL*)pSet);
            GOP_GWIN_SetBnkForceWrite(pInstance,gop,bEn);
            break;
        }
        case E_GOP_3D_LR_EXCHANGE:
        {
            GOP_Set3D_LR_FrameExchange(pInstance,gop);
            break;
        }
        case E_GOP_VE_OSD:
        {
            MS_BOOL bEn;
            if(u32Size != sizeof(MS_BOOL))
            {
                GOP_M_ERR("[%s][%d] input structure size ERROR!! \n",__FUNCTION__,__LINE__);
                return GOP_API_FAIL;
            }
            bEn = *((MS_BOOL*)pSet);
            GOP_Enable_VEOSD(pInstance,gop,bEn);
            break;
        }
        case E_GOP_OUTPUT_LAYER_SWITCH:
        {
            GOP_OutputLayerSwitch(pInstance,gop);
            break;
        }
        case E_GOP_REG_UPDATED:
        {
            break;
        }
        case E_GOP_RESOURCE:
        {
            GOP_ResetGOP(pInstance,gop);
            break;
        }
        case E_GOP_AFBC_CORE_RESET:
        {
#ifdef CONFIG_GOP_AFBC_FEATURE
            if(u32Size != sizeof(MS_BOOL))
            {
                GOP_M_ERR("[%s][%d] input structure size ERROR!! \n",__FUNCTION__,__LINE__);
                return GOP_API_FAIL;
            }
            MDrv_GOP_AFBC_Core_Reset(g_pGOPCtxLocal,gop);
#endif
            break;
        }
        case E_GOP_AFBC_CORE_ENABLE:
        {
#ifdef CONFIG_GOP_AFBC_FEATURE
            MS_BOOL bEn;
            if(u32Size != sizeof(MS_BOOL))
            {
                GOP_M_ERR("[%s][%d] input structure size ERROR!! \n",__FUNCTION__,__LINE__);
                return GOP_API_FAIL;
            }
            bEn = *((MS_BOOL*)pSet);
            MDrv_GOP_AFBC_Core_Enable(g_pGOPCtxLocal,gop,bEn);
#endif
            break;
        }
        case E_GOP_SET_VOP_PATH:
        {
            EN_GOP_VOP_PATH_MODE enGOPPath = E_GOP_VOPPATH_DEF;
            MS_U8 u8Index = 0;

            if(pSet == NULL)
            {
                GOP_M_ERR("[%s][%d] pSet is NULL  ERROR!! \n",__FUNCTION__,__LINE__);
                return GOP_API_FAIL;
            }
            if(u32Size != sizeof(EN_GOP_VOP_PATH_MODE))
            {
                GOP_M_ERR("[%s][%d] input structure size ERROR!! \n",__FUNCTION__,__LINE__);
                return GOP_API_FAIL;
            }

            enGOPPath = *((EN_GOP_VOP_PATH_MODE*)pSet);
            MDrv_GOP_VOP_Path_Sel(g_pGOPCtxLocal,enGOPPath);
            for (u8Index=0; u8Index<_GOP_GetMaxGOPNum(pInstance); u8Index++)
            {
                _GOP_InitHSPDByGOP(pInstance,u8Index);
                MDrv_GOP_GWIN_UpdateReg(g_pGOPCtxLocal,u8Index);
            }
            break;
        }
        case E_GOP_SET_CSC_TUNING:
        {
            ST_GOP_CSC_PARAM *pstGOPRaram = (ST_GOP_CSC_PARAM *)pSet;
            GOP_Result enRet;

            if(pstGOPRaram == NULL)
            {
                GOP_M_ERR("[%s][%d] pstGOPRaram is NULL  ERROR!! \n",__FUNCTION__,__LINE__);
                return GOP_API_FAIL;
            }
            enRet = MDrv_GOP_CSC_Tuning(g_pGOPCtxLocal,gop,pstGOPRaram);
            if(enRet != GOP_SUCCESS)
            {
                if(enRet == GOP_FUN_NOT_SUPPORTED)
                {
                    return GOP_API_FUN_NOT_SUPPORTED;
                }
                else
                {
                    return GOP_API_FAIL;
                }
            }

            MDrv_GOP_GWIN_UpdateReg(g_pGOPCtxLocal,gop);

            break;
        }
        case E_GOP_SET_CSC_CTRL:
        {
            ST_GOP_CSC_TABLE *pstGOPRaram = (ST_GOP_CSC_TABLE *)pSet;
            GOP_Result enRet;

            if(pstGOPRaram == NULL)
            {
                GOP_M_ERR("[%s][%d] pstGOPRaram is NULL  ERROR!! \n",__FUNCTION__,__LINE__);
                return GOP_API_FAIL;
            }
            enRet = MDrv_GOP_SetCSCCtrl(g_pGOPCtxLocal,gop,pstGOPRaram);

            if(enRet != GOP_SUCCESS)
            {
                if(enRet == GOP_FUN_NOT_SUPPORTED)
                {
                    return GOP_API_FUN_NOT_SUPPORTED;
                }
                else
                {
                    return GOP_API_FAIL;
                }
            }

            MDrv_GOP_GWIN_UpdateReg(g_pGOPCtxLocal,gop);
            break;
        }
        case E_GOP_PIXELSHIFT_PD:
        {
            MS_S32 s32Offset;
            MS_U8 u8Index = 0;
            s32Offset = *((MS_S32*)pSet);

            if(g_pGOPCtxLocal->pGopChipProperty->b2Pto1PSupport == TRUE)
            {
                g_pGOPCtxLocal->pGOPCtxShared->s32PixelShiftPDOffset  = (s32Offset+(s32Offset+1)/2)*2;
            }
            else
            {
                g_pGOPCtxLocal->pGOPCtxShared->s32PixelShiftPDOffset = s32Offset;
            }

            for (u8Index=0; u8Index<_GOP_GetMaxGOPNum(pInstance); u8Index++)
            {
                _GOP_InitHSPDByGOP(pInstance,u8Index);
                MDrv_GOP_GWIN_UpdateReg(g_pGOPCtxLocal,u8Index);
            }
            break;
        }
        case E_GOP_BYPASS_MODE:
        {
            MS_BOOL bEn;
            bEn = *((MS_BOOL*)pSet);

            if (MDrv_GOP_ByPassMode(g_pGOPCtxLocal, gop, bEn) != GOP_SUCCESS)
            {
                GOP_M_ERR("[%s][%d]E_GOP_BYPASS_MODE ERROR!! \n",__FUNCTION__,__LINE__);
                return GOP_API_FAIL;
            }
            break;
        }
        default:
            break;
    }

    return GOP_API_SUCCESS;

}


MS_U32 Ioctl_GOP_GetProperty(void* pInstance,EN_GOP_PROPERTY type,MS_U32 gop, MS_U32* pSet, MS_U32 u32Size)
{
#ifdef INSTANT_PRIVATE
    GOP_INSTANT_PRIVATE* psGOPInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void*)&psGOPInstPri);
#endif

    switch(type)
    {
        case E_GOP_CONTRAST:
        {
            PGOP_CONTRAST pRGB = (PGOP_CONTRAST)pSet;
            MS_U32 u32Ret = 0;
            if(u32Size != sizeof(GOP_CONTRAST))
            {
                GOP_M_ERR("[%s][%d] input structure size ERROR!! \n",__FUNCTION__,__LINE__);
                return GOP_API_FAIL;
            }

            u32Ret = GOP_GetContrast(pInstance,gop,&(pRGB->y), &(pRGB->u), &(pRGB->v));
            if(GOP_API_SUCCESS != (E_GOP_API_Result)u32Ret)
            {
                if(GOP_API_FUN_NOT_SUPPORTED == (E_GOP_API_Result)u32Ret)
                {
                    return GOP_API_FUN_NOT_SUPPORTED;
                }
                else
                {
                    return GOP_API_FAIL;
                }
            }

            break;
        }
        case E_GOP_BRIGHTNESS:
        {
            PGOP_BRIGHTNESS pBri = (PGOP_BRIGHTNESS)pSet;
            MS_U16* pu16Brightness = (MS_U16*)&(pBri->Brightness);
            if(u32Size != sizeof(GOP_BRIGHTNESS))
            {
                GOP_M_ERR("[%s][%d] input structure size ERROR!! \n",__FUNCTION__,__LINE__);
                return GOP_API_FAIL;
            }

            GOP_GetBrightness(pInstance,gop, pu16Brightness, &(pBri->bMSB));
            break;
        }
        case E_GOP_HSTART:
        {
            MS_U16 *pu16HStart = (MS_U16 *)pSet;
            if (u32Size != sizeof(MS_U16))
            {
                GOP_M_ERR("[%s][%d] input structure size ERROR!! \n",__FUNCTION__,__LINE__);
                return GOP_API_FAIL;
            }
            *pu16HStart = g_pGOPCtxLocal->pGOPCtxShared->u16PnlHStr[gop];
            break;
        }
        case E_GOP_OUTPUT_TIMING:
        {
            PST_GOP_TIMING_INFO pstInfo;
            if(u32Size != sizeof(ST_GOP_TIMING_INFO))
            {
                GOP_M_ERR("[%s][%d] input structure size ERROR!! \n",__FUNCTION__,__LINE__);
                return GOP_API_FAIL;
            }
            pstInfo = (PST_GOP_TIMING_INFO)pSet;
            GOP_GetPanelTiming(pInstance,gop, pstInfo);
            break;
        }
        case E_GOP_PIXELBASE_ENABLE:
        {
            PGOP_SET_PROPERTY_PARAM pstInfo;
            MS_BOOL *pbEn;
            if(u32Size != sizeof(MS_BOOL))
            {
                GOP_M_ERR("[%s][%d] input structure size ERROR!! \n",__FUNCTION__,__LINE__);
                return GOP_API_FAIL;
            }
            pstInfo = (PGOP_SET_PROPERTY_PARAM)(void*)pSet;
            pbEn = (MS_BOOL*)(pstInfo->pSetting);
            *pbEn = g_pGOPCtxLocal->pGOPCtxShared->bPixelMode[pstInfo->gop_idx];
            break;
        }
        case E_GOP_PREALPHAMODE:
        case E_GOP_FORCE_WRITE:
        case E_GOP_BANK_FORCE_WRITE:
        case E_GOP_3D_LR_EXCHANGE:
        case E_GOP_VE_OSD:
        case E_GOP_OUTPUT_LAYER_SWITCH:
        case E_GOP_OUTPUT_COLOR:
        case E_GOP_UV_SWAP:
        case E_GOP_YC_SWAP:
            break;
        case E_GOP_REG_UPDATED:
        {
            MS_BOOL *pUpdated = (MS_BOOL *)pSet;
            if(u32Size != sizeof(MS_BOOL))
            {
                GOP_M_ERR("[%s][%d] input structure size ERROR!! \n",__FUNCTION__,__LINE__);
                return GOP_API_FAIL;
            }
            *pUpdated = GOP_IsRegUpdated(pInstance,gop);
            break;
        }
        case E_GOP_MAXFBNUM:
        {
            MS_U32 *pMAXFBNum = (MS_U32 *)pSet;
            if(u32Size != sizeof(MS_U32))
            {
                GOP_M_ERR("[%s][%d] input structure size ERROR!! \n",__FUNCTION__,__LINE__);
                return GOP_API_FAIL;
            }
            *pMAXFBNum = DRV_MAX_GWIN_FB_SUPPORT;
            break;
        }
        case E_GOP_NUM_IS_INIT:
        {
            GOP_InitInfo *pGOPInitInfo = (GOP_InitInfo *)pSet;
            if(u32Size != sizeof(GOP_InitInfo))
            {
                GOP_M_ERR("[%s][%d] input structure size ERROR!! \n",__FUNCTION__,__LINE__);
                return GOP_API_FAIL;
            }
            GOP_CTX_DRV_SHARED *pDrvGOPShared;
            MS_BOOL bNeedInitShared = FALSE;
            pDrvGOPShared = (GOP_CTX_DRV_SHARED*)MDrv_GOP_GetShareMemory(&bNeedInitShared);

            if(pDrvGOPShared == NULL)
            {
                GOP_M_ERR("[%s][%d] Pointer pDrvGOPShared is null!! \n",__FUNCTION__,__LINE__);
                return GOP_API_FAIL;
            }
            if(pDrvGOPShared->apiCtxShared.bGopHasInitialized[gop])
            {
                pGOPInitInfo->u16PanelWidth = pDrvGOPShared->apiCtxShared.u16PnlWidth[gop];
                pGOPInitInfo->u16PanelHeight = pDrvGOPShared->apiCtxShared.u16PnlHeight[gop];
                pGOPInitInfo->u16PanelHStr = pDrvGOPShared->apiCtxShared.u16PnlHStr[gop];
                pGOPInitInfo->bEnableVsyncIntFlip = pDrvGOPShared->apiCtxShared.bEnableVsyncIntFlip;
                pGOPInitInfo->u32GOPRBAdr = pDrvGOPShared->apiCtxShared.fbPool1.GWinFB_Pool_BaseAddr;
                pGOPInitInfo->u32GOPRBLen = pDrvGOPShared->apiCtxShared.fbPool1.u32GWinFB_Pool_MemLen;
                pGOPInitInfo->u32GOPRegdmaAdr = pDrvGOPShared->apiCtxShared.phyGOPRegdmaAdr;
                pGOPInitInfo->u32GOPRegdmaLen = pDrvGOPShared->apiCtxShared.u32GOPRegdmaLen;
                //printf("pGOPInitInfo->u16PanelWidth=%d\n",pGOPInitInfo->u16PanelWidth);
                //printf("pGOPInitInfo->u16PanelHeight=%d\n",pGOPInitInfo->u16PanelHeight);
                //printf("pGOPInitInfo->u16PanelHStr=%d\n",pGOPInitInfo->u16PanelHStr);
                //printf("pGOPInitInfo->bEnableVsyncIntFlip=%d\n",pGOPInitInfo->bEnableVsyncIntFlip);
                //printf("pGOPInitInfo->u32GOPRBAdr=0x%x\n",pGOPInitInfo->u32GOPRBAdr);
                //printf("pGOPInitInfo->u32GOPRBLen=%d\n",pGOPInitInfo->u32GOPRBLen);
                //printf("pGOPInitInfo->u32GOPRegdmaAdr=0x%x\n",pGOPInitInfo->u32GOPRegdmaAdr);
                //printf("pGOPInitInfo->u32GOPRegdmaLen=%d\n",pGOPInitInfo->u32GOPRegdmaLen);
            }
            else
                 return GOP_API_FAIL;
            break;
        }
        case E_GOP_GET_CSC_CTRL_INFO:
        {
            ST_GOP_CSC_TUNING_INFO *pstTuningInfo = (ST_GOP_CSC_TUNING_INFO *)pSet;
            if(pstTuningInfo == NULL)
            {
                GOP_M_ERR("[%s][%d]  pstTuningInfo is NULL\n",__func__,__LINE__);
            }
            else
            {
                if(pstTuningInfo->pstCSCParam != NULL)
                {
                    memcpy(pstTuningInfo->pstCSCParam,&(g_pGOPCtxLocal->pGOPCtxShared->stCSCParam[gop]), sizeof(ST_GOP_CSC_PARAM));
                }
                if(pstTuningInfo->pstCSCTable != NULL)
                {
                    memcpy(pstTuningInfo->pstCSCTable,&(g_pGOPCtxLocal->pGOPCtxShared->stCSCTable[gop]), sizeof(ST_GOP_CSC_TABLE));
                }
            }
            break;
        }
        default:
            break;

    }

    return GOP_API_SUCCESS;

}

MS_U32 Ioctl_GOP_SetDst(void* pInstance,MS_U8 u8GOP, EN_GOP_DST_TYPE dsttype)
{
   DRV_GOPDstType GopDst;

#ifdef INSTANT_PRIVATE
    GOP_INSTANT_PRIVATE* psGOPInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void*)&psGOPInstPri);
#endif

    if ((!_GOP_IsGopNumValid(pInstance,u8GOP)))
    {
        GOP_M_ERR("[%s][%d]GOP %d  is out of range\n",__FUNCTION__,__LINE__,u8GOP);
        return GOP_API_FAIL;
    }

    GOP_M_INFO("---%s %d: PID[%td], TID[%td] u8GOP=%u, DstType=0x%tx\n", __FUNCTION__, __LINE__, (ptrdiff_t)getpid(), (ptrdiff_t)MsOS_GetOSThreadID(), u8GOP, (ptrdiff_t)dsttype);

    /*Set GOP dst and check input dst type is vaild or not for different chip first */
    (_GOP_Map_APIDst2DRV_Enum(pInstance,dsttype, &GopDst));
    (MDrv_GOP_GWIN_SetDstPlane(g_pGOPCtxLocal, u8GOP, GopDst,FALSE));

   // Set GOP dst output blending to SC/Mixer
    switch(GopDst)
    {

        case E_DRV_GOP_DST_IP0:
            MDrv_GOP_GWIN_SetMux(g_pGOPCtxLocal, u8GOP, E_GOP_IP0_MUX);
            _GOP_InitHSPDByGOP(pInstance,u8GOP);
            MDrv_GOP_SetGOPEnable2SC(g_pGOPCtxLocal, u8GOP, FALSE);
            _GOP_SetOCCapability(pInstance,u8GOP, GopDst);
            if (u8GOP==0)
                MDrv_GOP_SetIPSel2SC(g_pGOPCtxLocal, MS_DRV_IP0_SEL_GOP0);
            else if (u8GOP==1)
                MDrv_GOP_SetIPSel2SC(g_pGOPCtxLocal, MS_DRV_IP0_SEL_GOP1);
            else
                MDrv_GOP_SetIPSel2SC(g_pGOPCtxLocal, MS_DRV_IP0_SEL_GOP2);
            //MApi_GOP_GWIN_OutputColor(GOPOUT_YUV);
            MDrv_GOP_SelfFirstHs(g_pGOPCtxLocal, u8GOP, TRUE);
            break;

        case E_DRV_GOP_DST_MIXER2VE:
            //jasmine:FIXME
            if(g_pGOPCtxLocal->pGopChipProperty->bGOPMixerToVE)
            {
                MDrv_GOP_SetGOPEnable2SC(g_pGOPCtxLocal, u8GOP, FALSE);
                MDrv_GOP_GWIN_EnableProgressive(g_pGOPCtxLocal, u8GOP, TRUE);
                MDrv_GOP_GWIN_OutputColor(g_pGOPCtxLocal,u8GOP,DRV_GOPOUT_YUV);
                MDrv_GOP_GWIN_SetAlphaInverse(g_pGOPCtxLocal, u8GOP, TRUE);

                MDrv_GOP_SetGOPClk(g_pGOPCtxLocal,u8GOP,E_DRV_GOP_DST_MIXER2VE);

                (MDrv_GOP_Set_VE_MUX(g_pGOPCtxLocal, (GOPTYPE.MIXER-1),TRUE));
            }
            else
            {
                GOP_M_INFO("[%s][%d] Not Support GOPMixerToVE in this chip version.\n",__FUNCTION__,__LINE__);
            }
            MDrv_GOP_SelfFirstHs(g_pGOPCtxLocal, u8GOP, FALSE);
            break;

        case E_DRV_GOP_DST_OP0:
            _GOP_SetOCCapability(pInstance,u8GOP, GopDst);

            if (u8GOP==0)
                MDrv_GOP_SetIPSel2SC(g_pGOPCtxLocal, MS_DRV_NIP_SEL_GOP0);
            else if (u8GOP==1)
                MDrv_GOP_SetIPSel2SC(g_pGOPCtxLocal, MS_DRV_NIP_SEL_GOP1);
            else
                MDrv_GOP_SetIPSel2SC(g_pGOPCtxLocal, MS_DRV_NIP_SEL_GOP2);

            MDrv_GOP_SetGOPEnable2SC(g_pGOPCtxLocal, u8GOP, TRUE);

            _GOP_InitHSPDByGOP(pInstance,u8GOP);
            if (g_pGOPCtxLocal->pGopChipProperty->bOpInterlace == TRUE)
            {
                MDrv_GOP_GWIN_EnableProgressive(g_pGOPCtxLocal, u8GOP, FALSE);
            }
            else
            {
                MDrv_GOP_GWIN_EnableProgressive(g_pGOPCtxLocal, u8GOP, TRUE);
            }
            MDrv_GOP_SelfFirstHs(g_pGOPCtxLocal, u8GOP, FALSE);
            break;

        case E_DRV_GOP_DST_OP1:
            if(g_pGOPCtxLocal->pGopChipProperty->bNewMux)
            {
                //_CleanGopMux(u8GOP);
                MDrv_GOP_GWIN_SetMux(g_pGOPCtxLocal, u8GOP, E_GOP_OP1_MUX);
                _GOP_InitHSPDByGOP(pInstance,u8GOP);
            }

            MDrv_GOP_SetIPSel2SC(g_pGOPCtxLocal, MS_DRV_SC1OP_SEL);
            MDrv_GOP_GWIN_EnableProgressive(g_pGOPCtxLocal, u8GOP, FALSE);
            MDrv_GOP_GWIN_SetAlphaInverse(g_pGOPCtxLocal, u8GOP, TRUE);
            MDrv_GOP_SetGOPEnable2SC(g_pGOPCtxLocal, u8GOP, FALSE);
            _GOP_InitHSPDByGOP(pInstance,u8GOP);
            MDrv_GOP_SelfFirstHs(g_pGOPCtxLocal, u8GOP, FALSE);
            break;

        case E_DRV_GOP_DST_VOP:
                MDrv_GOP_GWIN_SetMux(g_pGOPCtxLocal, u8GOP, E_GOP_VOP0_MUX);
                _GOP_InitHSPDByGOP(pInstance,u8GOP);

#if (FPGA_TEST)
            GOP_SetOutputColor(pInstance,u8GOP,GOPOUT_RGB);
#else
            GOP_SetOutputColor(pInstance,u8GOP,GOPOUT_YUV);
#endif

            if (u8GOP==0)
                MDrv_GOP_SetIPSel2SC(g_pGOPCtxLocal, MS_DRV_NIP_SEL_GOP0);
            else if (u8GOP==1)
                MDrv_GOP_SetIPSel2SC(g_pGOPCtxLocal, MS_DRV_NIP_SEL_GOP1);
            else
                MDrv_GOP_SetIPSel2SC(g_pGOPCtxLocal, MS_DRV_NIP_SEL_GOP2);

            MDrv_GOP_SetIPSel2SC(g_pGOPCtxLocal, MS_DRV_MVOP_SEL);
            MDrv_GOP_SetGOPEnable2SC(g_pGOPCtxLocal, u8GOP, FALSE);
            _GOP_SetOCCapability(pInstance,u8GOP, GopDst);
            MDrv_GOP_SelfFirstHs(g_pGOPCtxLocal, u8GOP, FALSE);
            break;

        case E_DRV_GOP_DST_VOP_SUB:
            if(g_pGOPCtxLocal->pGopChipProperty->bNewMux)
            {
                MDrv_GOP_GWIN_SetMux(g_pGOPCtxLocal, u8GOP, E_GOP_VOP0_SUB_MUX);
                _GOP_InitHSPDByGOP(pInstance,u8GOP);
            }

#if (FPGA_TEST)
            GOP_SetOutputColor(pInstance,u8GOP,GOPOUT_RGB);
#else
            GOP_SetOutputColor(pInstance,u8GOP,GOPOUT_YUV);
#endif
            if (u8GOP==0)
                MDrv_GOP_SetIPSel2SC(g_pGOPCtxLocal, MS_DRV_NIP_SEL_GOP0);
            else if (u8GOP==1)
                MDrv_GOP_SetIPSel2SC(g_pGOPCtxLocal, MS_DRV_NIP_SEL_GOP1);
            else
                MDrv_GOP_SetIPSel2SC(g_pGOPCtxLocal, MS_DRV_NIP_SEL_GOP2);

            MDrv_GOP_SetIPSel2SC(g_pGOPCtxLocal, MS_DRV_SUBMVOP_SEL);
            MDrv_GOP_SetGOPEnable2SC(g_pGOPCtxLocal, u8GOP, FALSE);
            MDrv_GOP_SelfFirstHs(g_pGOPCtxLocal, u8GOP, FALSE);
            break;
        case E_DRV_GOP_DST_IP1:
            if(g_pGOPCtxLocal->pGopChipProperty->bNewMux)
            {
                //_CleanGopMux(u8GOP);
                MDrv_GOP_GWIN_SetMux(g_pGOPCtxLocal, u8GOP, E_GOP_IP1_MUX);
                _GOP_InitHSPDByGOP(pInstance,u8GOP);
            }
            MDrv_GOP_SetGOPEnable2SC(g_pGOPCtxLocal, u8GOP, FALSE);
            MDrv_GOP_GWIN_SetAlphaInverse(g_pGOPCtxLocal, u8GOP, FALSE);
            MDrv_GOP_SetIPSel2SC(g_pGOPCtxLocal, MS_DRV_IP1_SEL);
            MDrv_GOP_SetGOPClk(g_pGOPCtxLocal,u8GOP,E_DRV_GOP_DST_IP1);
            MDrv_GOP_SelfFirstHs(g_pGOPCtxLocal, u8GOP, TRUE);
            break;

        case E_DRV_GOP_DST_IP0_SUB:
            if(g_pGOPCtxLocal->pGopChipProperty->bNewMux)
            {
                //_CleanGopMux(u8GOP);
                MDrv_GOP_GWIN_SetMux(g_pGOPCtxLocal, u8GOP, E_GOP_IP0_SUB_MUX);
                _GOP_InitHSPDByGOP(pInstance,u8GOP);
            }
            MDrv_GOP_SetGOPEnable2SC(g_pGOPCtxLocal, u8GOP, FALSE);
            MDrv_GOP_GWIN_SetAlphaInverse(g_pGOPCtxLocal, u8GOP, FALSE);
            MDrv_GOP_SetIPSel2SC(g_pGOPCtxLocal, MS_DRV_SUBIP0_SEL);
            MDrv_GOP_SelfFirstHs(g_pGOPCtxLocal, u8GOP, TRUE);
            break;
        case E_DRV_GOP_DST_DIP:
            if(g_pGOPCtxLocal->pGopChipProperty->bNewMux)
            {
                //_CleanGopMux(u8GOP);
                MDrv_GOP_GWIN_SetMux(g_pGOPCtxLocal, u8GOP, E_GOP_DIP_MUX);
                _GOP_InitHSPDByGOP(pInstance,u8GOP);
            }
            MDrv_GOP_SetGOPEnable2SC(g_pGOPCtxLocal, u8GOP, FALSE);
            MDrv_GOP_GWIN_SetAlphaInverse(g_pGOPCtxLocal, u8GOP, FALSE);
            MDrv_GOP_SetIPSel2SC(g_pGOPCtxLocal, MS_DRV_DIP_SEL);
            MDrv_GOP_SelfFirstHs(g_pGOPCtxLocal, u8GOP, FALSE);
            break;
        case E_DRV_GOP_DST_MIXER2OP:
            MDrv_GOP_SetGOPEnable2SC(g_pGOPCtxLocal, (GOPTYPE.MIXER-1), TRUE); //To find the mux which is mixer, open SC blending
            //MDrv_GOP_SetGOPEnable2SC(g_pGOPCtxLocal, u8GOP, FALSE);
            MDrv_GOP_GWIN_EnableProgressive(g_pGOPCtxLocal, u8GOP, TRUE);
            MDrv_GOP_GWIN_OutputColor(g_pGOPCtxLocal,u8GOP, DRV_GOPOUT_YUV);
            MDrv_GOP_GWIN_SetAlphaInverse(g_pGOPCtxLocal, u8GOP,TRUE);

            //MDrv_GOP_GWIN_SetHSPipe(g_pGOPCtxLocal, u8GOP, 0);
            _GOP_InitHSPDByGOP(pInstance,u8GOP);
            MDrv_GOP_SelfFirstHs(g_pGOPCtxLocal, u8GOP, FALSE);
            break;

        case E_DRV_GOP_DST_MIXER2OP1:
            MDrv_GOP_GWIN_SetMux(g_pGOPCtxLocal, (GOPTYPE.MIXER-1), E_GOP_OP1_MUX);
            MDrv_GOP_SetIPSel2SC(g_pGOPCtxLocal, MS_DRV_SC1OP_SEL);
            //MDrv_GOP_SetGOPEnable2SC(g_pGOPCtxLocal, GOPTYPE.MIXER, TRUE); //To find the mux which is mixer, open SC blending
            //MDrv_GOP_SetGOPEnable2SC(g_pGOPCtxLocal, u8GOP, FALSE);
            MDrv_GOP_GWIN_EnableProgressive(g_pGOPCtxLocal, u8GOP, FALSE);
            MDrv_GOP_GWIN_OutputColor(g_pGOPCtxLocal,u8GOP, DRV_GOPOUT_YUV);
            MDrv_GOP_GWIN_SetAlphaInverse(g_pGOPCtxLocal, u8GOP,TRUE);

            //MDrv_GOP_GWIN_SetHSPipe(g_pGOPCtxLocal, u8GOP, 0);
            _GOP_InitHSPDByGOP(pInstance,u8GOP);
            MDrv_GOP_SelfFirstHs(g_pGOPCtxLocal, u8GOP, FALSE);
            break;

        case E_DRV_GOP_DST_FRC:
            MDrv_GOP_SetGOPEnable2SC(g_pGOPCtxLocal, u8GOP, TRUE);
            _GOP_InitHSPDByGOP(pInstance,u8GOP); //require update registers at the end of this function after dwin capture in OP
            _GOP_SetOCCapability(pInstance,u8GOP, GopDst);
            MDrv_GOP_SelfFirstHs(g_pGOPCtxLocal, u8GOP, FALSE);
            break;
        case E_DRV_GOP_DST_VE:
            MDrv_GOP_SetGOPEnable2SC(g_pGOPCtxLocal, u8GOP, FALSE);
            MDrv_GOP_GWIN_EnableProgressive(g_pGOPCtxLocal, u8GOP, FALSE);
            MDrv_GOP_GWIN_OutputColor(g_pGOPCtxLocal,u8GOP,DRV_GOPOUT_YUV);
            MDrv_GOP_GWIN_SetAlphaInverse(g_pGOPCtxLocal,u8GOP,FALSE);

            MDrv_GOP_SetGOPClk(g_pGOPCtxLocal,u8GOP,E_DRV_GOP_DST_VE);

            if(!(g_pGOPCtxLocal->sGOPConfig.eGopIgnoreInit & E_GOP_BOOTLOGO_IGNORE_VEOSDEN))
            {
                (MDrv_GOP_Set_VE_MUX(g_pGOPCtxLocal, u8GOP, TRUE));
            }
            MDrv_GOP_SelfFirstHs(g_pGOPCtxLocal, u8GOP, FALSE);
            break;
        case E_DRV_GOP_DST_GOPScaling:
            _GOP_SetOCCapability(pInstance,u8GOP, GopDst);

            MDrv_GOP_GWIN_SetMux(g_pGOPCtxLocal, u8GOP, E_GOP_GS_MUX);

            if (u8GOP==0)
                MDrv_GOP_SetIPSel2SC(g_pGOPCtxLocal, MS_DRV_NIP_SEL_GOP0);
            else if (u8GOP==1)
                MDrv_GOP_SetIPSel2SC(g_pGOPCtxLocal, MS_DRV_NIP_SEL_GOP1);
            else
                MDrv_GOP_SetIPSel2SC(g_pGOPCtxLocal, MS_DRV_NIP_SEL_GOP2);

            MDrv_GOP_SetGOPEnable2SC(g_pGOPCtxLocal, u8GOP, TRUE);
            _GOP_InitHSPDByGOP(pInstance,u8GOP);
            MDrv_GOP_SelfFirstHs(g_pGOPCtxLocal, u8GOP, FALSE);
            break;
        case E_DRV_GOP_DST_BYPASS:
            _GOP_SetOCCapability(pInstance,u8GOP, GopDst);
            _GOP_InitHSPDByGOP(pInstance,u8GOP);
            MDrv_GOP_GWIN_SetMux(g_pGOPCtxLocal, u8GOP, E_GOP_BYPASS_MUX0);
            MDrv_GOP_SetGOPEnable2SC(g_pGOPCtxLocal, u8GOP, FALSE);
            MDrv_GOP_SelfFirstHs(g_pGOPCtxLocal, u8GOP, FALSE);
            break;

        case E_DRV_GOP_DST_OP_DUAL_RATE:
            _GOP_SetOCCapability(pInstance,u8GOP, GopDst);
            _GOP_InitHSPDByGOP(pInstance,u8GOP);
            MDrv_GOP_SetGOPEnable2SC(g_pGOPCtxLocal, u8GOP, FALSE);
            MDrv_GOP_SelfFirstHs(g_pGOPCtxLocal, u8GOP, FALSE);
            break;

        default:
            MS_ASSERT(0);
            return GOP_API_ENUM_NOT_SUPPORTED;
    }

    (MDrv_GOP_SetGOPClk(g_pGOPCtxLocal, u8GOP, GopDst));
    MDrv_GOP_GWIN_UpdateReg(g_pGOPCtxLocal, u8GOP);
    return GOP_API_SUCCESS;
}

MS_U32 Ioctl_GOP_GetDst(void* pInstance,MS_U32 gop, MS_U32* pDst)
{
#ifdef INSTANT_PRIVATE
    GOP_INSTANT_PRIVATE* psGOPInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void*)&psGOPInstPri);
#endif

    DRV_GOPDstType enDst = E_DRV_GOP_DST_INVALID;

    (MDrv_GOP_GWIN_GetDstPlane(g_pGOPCtxLocal, gop, &enDst));
    _GOP_Map_DRVDst2API_Enum_(pInstance,(EN_GOP_DST_TYPE *)pDst, enDst);
    return GOP_API_SUCCESS;

}

MS_U32 Ioctl_GOP_SetMux(void* pInstance,PGOP_SETMUX pGopMuxConfig, MS_U32 u32SizeOfMuxInfo)
{
    MS_U8 i, u8GOPIdx;
    Gop_MuxSel EnGOPMux;
#ifdef INSTANT_PRIVATE
    GOP_INSTANT_PRIVATE* psGOPInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void*)&psGOPInstPri);
#endif

    if(u32SizeOfMuxInfo != sizeof(GOP_SETMUX) || (pGopMuxConfig == NULL))
    {
        GOP_M_ERR("\nthe GOP_MuxConfig is NULL or struct mismatch in drivr\n");
        return GOP_API_FAIL;
    }
    GOP_M_INFO("---%s %d: PID[%td], TID[%td] MuxCount=%tu\n", __FUNCTION__, __LINE__, (ptrdiff_t)getpid(), (ptrdiff_t)MsOS_GetOSThreadID(), (ptrdiff_t)pGopMuxConfig->MuxCount);

#if 0
    for(i=0; i<pGopMuxConfig->u8MuxCounts; i++)
    {
        MDrv_GOP_GWIN_SetMux(g_pGOPCtxLocal, pGopMuxConfig->GopMux[i].u8GopIndex, (Gop_MuxSel) pGopMuxConfig->GopMux[i].u8MuxIndex);
    }

#endif

    for(i=0; i<pGopMuxConfig->MuxCount; i++)
    {
        _GOP_MAP_MUX_Enum(pInstance,(EN_Gop_MuxSel)pGopMuxConfig->mux[i], &EnGOPMux);
        MDrv_GOP_GWIN_SetMux(g_pGOPCtxLocal, pGopMuxConfig->gop[i], EnGOPMux);
    }

    if(g_pGOPCtxLocal->pGopChipProperty->bOPMuxDoubleBuffer == TRUE)
    {
        MDrv_GOP_GWIN_Trigger_MUX(g_pGOPCtxLocal);
    }
    else
    {
        GOP_M_WARN("[%s]GOP no OPMux double buffer, so layer configuration (dfbrc.ini & OsdConfig.ini) need to match with each other\n",__FUNCTION__);
    }

    for(u8GOPIdx = 0; u8GOPIdx < MDrv_GOP_GetMaxGOPNum(g_pGOPCtxLocal); u8GOPIdx++)
    {
        if (g_pGOPCtxLocal->pGOPCtxShared->bGopHasInitialized[u8GOPIdx])
        {
            _GOP_InitHSPDByGOP(pInstance, u8GOPIdx);
        }
    }

    return GOP_API_SUCCESS;

}


MS_U32 Ioctl_GOP_SetMirror(void* pInstance,MS_U32 gop, EN_GOP_MIRROR_TYPE mirror)
{
    // To do: to modify and control each gop mirror setting.
    switch(mirror)
    {
        case E_GOP_MIRROR_NONE:
        default:
        {
            GOP_GWIN_SetHMirror(pInstance,FALSE);
            GOP_GWIN_SetVMirror(pInstance,FALSE);
            break;
        }
        case E_GOP_MIRROR_H_ONLY:
        {
            GOP_GWIN_SetHMirror(pInstance,TRUE);
            //GOP_GWIN_SetVMirror(pInstance,FALSE);
            break;
        }
        case E_GOP_MIRROR_H_NONE:
        {
            GOP_GWIN_SetHMirror(pInstance,FALSE);
            break;
        }
        case E_GOP_MIRROR_V_ONLY:
        {
            //GOP_GWIN_SetHMirror(pInstance,FALSE);
            GOP_GWIN_SetVMirror(pInstance,TRUE);
            break;
        }
        case E_GOP_MIRROR_V_NONE:
        {
            GOP_GWIN_SetVMirror(pInstance,FALSE);
            break;
        }
        case E_GOP_MIRROR_HV:
        {
            GOP_GWIN_SetHMirror(pInstance,TRUE);
            GOP_GWIN_SetVMirror(pInstance,TRUE);
            break;
        }

    }

    return GOP_API_SUCCESS;
}

MS_U32 Ioctl_GOP_GetMirror(void* pInstance,MS_U32 gop, EN_GOP_MIRROR_TYPE* mirror)
{
    MS_BOOL bHMirror;
    MS_BOOL bVMirror;
#ifdef INSTANT_PRIVATE
    GOP_INSTANT_PRIVATE* psGOPInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void*)&psGOPInstPri);
#endif
    //need refine
    bHMirror = g_pGOPCtxLocal->pGOPCtxShared->bHMirror;
    bVMirror = g_pGOPCtxLocal->pGOPCtxShared->bVMirror;

    if((bHMirror == TRUE)&&(bVMirror == TRUE))
    {
        *mirror = E_GOP_MIRROR_HV;
    }
    else if((bHMirror == TRUE)&&(bVMirror == FALSE))
    {
        *mirror = E_GOP_MIRROR_H_ONLY;
    }
    else if((bHMirror == FALSE)&&(bVMirror == TRUE))
    {
        *mirror = E_GOP_MIRROR_V_ONLY;
    }
    else
    {
        *mirror = E_GOP_MIRROR_NONE;
    }

    return GOP_API_SUCCESS;
}


MS_U32 Ioctl_GOP_GetStatus(void* pInstance,EN_GOP_STATUS type, MS_U32* pStatus, MS_U32 u32Size)
{
#ifdef INSTANT_PRIVATE
    GOP_INSTANT_PRIVATE* psGOPInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void*)&psGOPInstPri);
#endif

    if(pStatus == NULL)
    {
        GOP_M_ERR("[%s][%d] ERROR - Input null pointer on type:%d\n",__FUNCTION__,__LINE__,type);
        return GOP_API_INVALID_PARAMETERS;
    }

    switch(type)
    {
        case E_GOP_STATUS_INIT:
        {
            PGOP_INIT_STATUS ptr = (PGOP_INIT_STATUS)pStatus;
            if(u32Size != sizeof(GOP_INIT_STATUS))
            {
                return GOP_API_INVALID_PARAMETERS;
            }
            ptr->bInit = g_pGOPCtxLocal->pGOPCtxShared->bGopHasInitialized[ptr->gop_idx];

            break;
        }
        case E_GOP_STATUS_GOP_MAXNUM:
        {
            MS_U8 *pMaxGopNum = (MS_U8 *) pStatus;
            *pMaxGopNum = MDrv_GOP_GetMaxGOPNum(g_pGOPCtxLocal);
            break;
        }
        case E_GOP_STATUS_GWIN_MAXNUM:
        {
            PGOP_GWIN_NUM ptr = (PGOP_GWIN_NUM)pStatus;
            if(u32Size != sizeof(GOP_GWIN_NUM))
            {
                GOP_M_ERR("[%s] Error (%d)\n",__FUNCTION__,__LINE__);
                return (GOP_API_INVALID_PARAMETERS);
            }
            if(ptr->gop_idx > MDrv_GOP_GetMaxGOPNum(g_pGOPCtxLocal))
            {
                GOP_M_ERR("[%s] Error (%d)\n",__FUNCTION__,__LINE__);
                return (GOP_API_INVALID_PARAMETERS);
            }

            ptr->gwin_num = MDrv_GOP_GetGwinNum(g_pGOPCtxLocal,ptr->gop_idx);
            break;
        }
        case E_GOP_STATUS_GWIN_TOTALNUM:
        {
            MS_U8 *pGwinNum = (MS_U8 *) pStatus;
            if(u32Size != sizeof(MS_U8))
            {
                GOP_M_ERR("[%s] Error (%d)\n",__FUNCTION__,__LINE__);
                return (GOP_API_INVALID_PARAMETERS);
            }
            *pGwinNum = (MS_U8)g_pGOPCtxLocal->pGopChipProperty->TotalGwinNum;
            break;

        }
        case E_GOP_STATUS_VERSION:
        {
            break;
        }
        case E_GOP_STATUS_MUX:
        {

            break;
        }
        case E_GOP_STATUS_CURRENT_GOP:
        {
            MS_U32* pCurrentGop = (MS_U32*)pStatus;
            *pCurrentGop = GOP_GetCurrentGOP(pInstance);
            break;
        }
        case E_GOP_STATUS_CURRENT_GWIN:
        {
            MS_U32* pCurrentGwin = (MS_U32*)pStatus;
            *pCurrentGwin = GOP_GetCurrentGWin(pInstance);
            break;
        }
        case E_GOP_STATUS_CURRENT_FBID:
        {
            MS_U32* pCurrentFbid = (MS_U32*)pStatus;
            *pCurrentFbid = GOP_GetCurrentFBID(pInstance);
            break;
        }
        default:
        {
            GOP_M_ERR("[%s] invalid input case:%d\n",__FUNCTION__, type);
        }

    }

    return GOP_API_SUCCESS;

}

MS_U32 Ioctl_GOP_Set_Stretch(void* pInstance,EN_GOP_STRETCH_TYPE enStretchType,MS_U32 gop ,PGOP_STRETCH_INFO pStretchInfo)
{
    MS_BOOL bEn = FALSE;
    DRV_GOPDstType enGopDst = E_DRV_GOP_DST_INVALID;
    EN_GOP_DST_TYPE enApiGopDst = MAX_GOP_DST_SUPPORT;
    E_GOP_API_Result enRet = GOP_API_SUCCESS;
#ifdef INSTANT_PRIVATE
    GOP_INSTANT_PRIVATE* psGOPInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void*)&psGOPInstPri);
#endif

    MDrv_GOP_GWIN_GetDstPlane(g_pGOPCtxLocal,gop, &enGopDst);

    if(enStretchType & E_GOP_STRETCH_WIN)
    {
        _GOP_Map_DRVDst2API_Enum_(pInstance,&enApiGopDst, enGopDst);
        enRet = (E_GOP_API_Result)GOP_SetStretchWin(pInstance, gop, enApiGopDst, pStretchInfo->SrcRect.x, pStretchInfo->SrcRect.y,pStretchInfo->SrcRect.w,pStretchInfo->SrcRect.h);
    }

    if(enStretchType & E_GOP_STRETCH_HSCALE)
    {
        if(pStretchInfo->SrcRect.w != pStretchInfo->DstRect.w)
        {
            bEn = TRUE;
        }
        enRet = (E_GOP_API_Result)GOP_Set_Hscale(pInstance,gop,bEn,pStretchInfo->SrcRect.w,pStretchInfo->DstRect.w);
    }

    if(enStretchType & E_GOP_STRETCH_VSCALE)
    {
        if(pStretchInfo->SrcRect.h != pStretchInfo->DstRect.h)
        {
            bEn = TRUE;
        }
        enRet = (E_GOP_API_Result)GOP_Set_Vscale(pInstance,gop,bEn,pStretchInfo->SrcRect.h,pStretchInfo->DstRect.h);
    }

    if(enStretchType & E_GOP_STRETCH_HSTRETCH_MODE)
    {
        enRet = (E_GOP_API_Result)GOP_GWIN_Set_HStretchMode(pInstance,gop,pStretchInfo->enHMode);
    }

    if(enStretchType & E_GOP_STRETCH_VSTRETCH_MODE)
    {
        enRet = (E_GOP_API_Result)GOP_GWIN_Set_VStretchMode(pInstance,gop,pStretchInfo->enVMode);
    }

    return enRet;
}

#ifdef GOP_SUPPORT_PIXEL_SHIFT_MODE
MS_U32 GOP_SetStretchWinOffset(void* pInstance, MS_S32 s32Xoffset, MS_S32 s32Yoffset)
{
    MS_U8 u8GOP_num = 0x0;
    MS_U16 u16x[SHARED_GOP_MAX_COUNT] = {0};
    MS_U16 u16y[SHARED_GOP_MAX_COUNT] = {0};
    MS_U16 u16Width[SHARED_GOP_MAX_COUNT] = {0};
    MS_U16 u16Height[SHARED_GOP_MAX_COUNT] = {0};

#ifdef INSTANT_PRIVATE
    GOP_INSTANT_PRIVATE* psGOPInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void*)&psGOPInstPri);
#endif

    if (!_GOP_IsGopNumValid(pInstance,u8GOP_num))
    {
        GOP_M_ERR("[%s][%d]GOP %d  is out of range\n",__FUNCTION__,__LINE__,u8GOP_num);
        return GOP_API_FAIL;
    }

    for (u8GOP_num=0x0; u8GOP_num<_GOP_GetMaxGOPNum(pInstance); u8GOP_num++)
    {
        u16x[u8GOP_num] = 0x0;
        u16y[u8GOP_num] = 0x0;
        u16Width[u8GOP_num] = 0x0;
        u16Height[u8GOP_num] = 0x0;
    }

    for (u8GOP_num=0x0; u8GOP_num<_GOP_GetMaxGOPNum(pInstance); u8GOP_num++)
    {
        if ((g_pGOPCtxLocal->pGOPCtxShared->u16PnlWidth[u8GOP_num] > 0) &&
            (g_pGOPCtxLocal->pGOPCtxShared->u16PnlHeight[u8GOP_num] > 0))
        {
            _GOP_Get_StretchWin(pInstance,u8GOP_num, &u16x[u8GOP_num], &u16y[u8GOP_num], &u16Width[u8GOP_num], &u16Height[u8GOP_num]);
        }
    }

    //Store API use stretch window
    g_pGOPCtxLocal->pGOPCtxShared->s32GOPStretchWinHOffset = s32Xoffset;
    g_pGOPCtxLocal->pGOPCtxShared->s32GOPStretchWinVOffset = s32Yoffset;

    for (u8GOP_num=0x0; u8GOP_num<_GOP_GetMaxGOPNum(pInstance); u8GOP_num++)
    {
        if ((g_pGOPCtxLocal->pGOPCtxShared->u16PnlWidth[u8GOP_num] > 0) &&
            (g_pGOPCtxLocal->pGOPCtxShared->u16PnlHeight[u8GOP_num] > 0))
        {
            MDrv_GOP_GWIN_SetStretchWin(g_pGOPCtxLocal, u8GOP_num, u16x[u8GOP_num], u16y[u8GOP_num], u16Width[u8GOP_num], u16Height[u8GOP_num]);
            MDrv_GOP_GWIN_UpdateReg(g_pGOPCtxLocal,u8GOP_num);
        }
    }


    return GOP_API_SUCCESS;
}
#endif

MS_U32 Ioctl_GOP_Get_Stretch(void* pInstance,EN_GOP_STRETCH_TYPE enStretchType,MS_U32 gop ,PGOP_STRETCH_INFO pStretchInfo)
{
#ifdef INSTANT_PRIVATE
    GOP_INSTANT_PRIVATE* psGOPInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void*)&psGOPInstPri);
#endif

    if(enStretchType & E_GOP_STRETCH_WIN)
    {
        MS_U16 x,y,w,h;

        MDrv_GOP_GWIN_Get_StretchWin(g_pGOPCtxLocal, gop, &x, &y, &w, &h);
        pStretchInfo->SrcRect.x = (MS_U32)x;
        pStretchInfo->SrcRect.y = (MS_U32)y;
        pStretchInfo->SrcRect.w = (MS_U32)w;
        pStretchInfo->SrcRect.h = (MS_U32)h;
    }

    return GOP_API_SUCCESS;
}


MS_U32 Ioctl_GOP_GWin_SetProperty(void* pInstance,EN_GOP_GWIN_PROPERTY type,MS_U32 gwin, MS_U32* pSet, MS_U32 u32Size)
{
    switch(type)
    {
        case E_GOP_GWIN_BLENDING:
        {
            PGOP_GWIN_BLENDING pInfo = (PGOP_GWIN_BLENDING)pSet;

            if(u32Size != sizeof(GOP_GWIN_BLENDING))
                return GOP_API_INVALID_PARAMETERS;

            GOP_GWIN_SetBlending(pInstance,(MS_U8)gwin,pInfo->bEn, (MS_U8)pInfo->Coef);

            break;
        }
        case E_GOP_GWIN_NEWAPLHA:
        {
            MS_BOOL bEn;

            if(u32Size != sizeof(MS_BOOL))
                return GOP_API_INVALID_PARAMETERS;

            bEn = *((MS_BOOL*)pSet);

            GOP_GWIN_SetNewAlphaMode(pInstance,gwin,bEn);
            break;
        }
        case E_GOP_GWIN_ENABLE:
        {
            MS_BOOL bEn;

            if(u32Size != sizeof(MS_BOOL))
                return GOP_API_INVALID_PARAMETERS;

            bEn = *((MS_BOOL*)pSet);

            GOP_GWIN_SetEnable(pInstance,gwin,bEn);

            break;
        }
        case E_GOP_GWIN_SHARE:
        {
            MS_BOOL bEn;
            if(u32Size != sizeof(MS_BOOL))
                return GOP_API_INVALID_PARAMETERS;
            bEn = *((MS_BOOL*)pSet);
            GOP_GWIN_SetGWinShared(pInstance,(MS_U8)gwin, bEn);
            break;
        }
        case E_GOP_GWIN_SHARE_CNT:
        {
            MS_U16 u16SharedCnt;
            if(u32Size != sizeof(MS_U16))
                return GOP_API_INVALID_PARAMETERS;
            u16SharedCnt = *((MS_U16*)pSet);
            GOP_GWIN_SetGWinSharedCnt(pInstance,(MS_U8)gwin, u16SharedCnt);
            break;
        }
        case E_GOP_GWIN_SWITCH_2_GWIN:
        {
            GOP_GWIN_Switch2Gwin(pInstance, (MS_U8)gwin, (void*)pSet);
            break;
        }
        case E_GOP_GWIN_EXIST:
        {
            break;
        }
        case E_GOP_GWIN_FREE_ID:
        {
            break;
        }
        case E_GOP_GWIN_GET_FB:
        {
            break;
        }
        case E_GOP_GWIN_GET_GOP:
        {
            break;
        }
        case E_GOP_GWIN_GPUTILE:
        {
            EN_GOP_GPU_TILE_MODE tilemode;
            if(u32Size != sizeof(EN_GOP_GPU_TILE_MODE))
            {
                return GOP_API_INVALID_PARAMETERS;
            }
            tilemode = (EN_GOP_GPU_TILE_MODE)*pSet;
            GOP_GWIN_SetGPUtileMode(pInstance,gwin,tilemode);
            break;
        }
        case E_GOP_GWIN_STRETCH_OFFSET:
        {
#ifdef GOP_SUPPORT_PIXEL_SHIFT_MODE
            GOP_STRETCH_OFFSET_ST * pStretchOffsetInfo;
            if(u32Size != sizeof(GOP_STRETCH_OFFSET_ST))
            {
                return GOP_API_INVALID_PARAMETERS;
            }
            pStretchOffsetInfo = (GOP_STRETCH_OFFSET_ST *)pSet;
            GOP_SetStretchWinOffset(pInstance, pStretchOffsetInfo->s32Hoffset, pStretchOffsetInfo->s32Voffset);
#endif
            break;
        }
		default:
			break;
    }


    return GOP_API_SUCCESS;

}



MS_U32 Ioctl_GOP_GWin_GetProperty(void* pInstance,EN_GOP_GWIN_PROPERTY type,MS_U32 gwin, MS_U32* pSet, MS_U32 u32Size)
{
#ifdef INSTANT_PRIVATE
        GOP_INSTANT_PRIVATE* psGOPInstPri = NULL;
        UtopiaInstanceGetPrivate(pInstance, (void*)&psGOPInstPri);
#endif
    switch(type)
    {
        case E_GOP_GWIN_BLENDING:
        {
            break;
        }
        case E_GOP_GWIN_NEWAPLHA:
        {
            MS_BOOL bEn=FALSE;

            if(u32Size != sizeof(MS_BOOL))
                return GOP_API_INVALID_PARAMETERS;
            MDrv_GOP_GWIN_GetGwinNewAlphaModeEnable(g_pGOPCtxLocal, gwin, &bEn);
            *pSet = bEn;

            break;
        }
        case E_GOP_GWIN_ENABLE:
        {
            MS_BOOL *pbEn = (MS_BOOL*)pSet;

            if(u32Size != sizeof(MS_BOOL))
                return GOP_API_INVALID_PARAMETERS;

            *pbEn = GOP_GWIN_IsEnable(pInstance,gwin);

            break;
        }
        case E_GOP_GWIN_BLINK:
        {
            break;
        }
        case E_GOP_GWIN_SHARE:
        {
            break;
        }
        case E_GOP_GWIN_SHARE_CNT:
        {
            break;
        }
        case E_GOP_GWIN_SWITCH_2_GWIN:
        {
            break;
        }
        case E_GOP_GWIN_EXIST:
        {
            MS_U8 *pExist = (MS_U8*)pSet;

            if(u32Size != sizeof(MS_U8))
                return GOP_API_INVALID_PARAMETERS;

            *pExist = (MS_U8)GOP_GWIN_IsGwinExist(pInstance,gwin);
            break;
        }
        case E_GOP_GWIN_FREE_ID:
        {
            MS_U8 *pFreeId = (MS_U8*)pSet;

            if(u32Size != sizeof(MS_U8))
                return GOP_API_INVALID_PARAMETERS;
            *pFreeId = (MS_U8)GOP_GWIN_GetFreeWinID(pInstance);
            break;
        }
        case E_GOP_GWIN_GET_FB:
        {
            MS_U32 *pFbId = (MS_U32*)pSet;

            if(u32Size != sizeof(MS_U32))
                return GOP_API_INVALID_PARAMETERS;

            *pFbId = GOP_GWIN_GetFBfromGWIN(pInstance,gwin);
            break;
        }
        case E_GOP_GWIN_GET_GOP:
        {
            MS_U32 *pGOP = (MS_U32*)pSet;
            if(u32Size != sizeof(MS_U32))
                return GOP_API_INVALID_PARAMETERS;

            *pGOP = (MS_U32)MDrv_DumpGopByGwinId(g_pGOPCtxLocal,gwin);
            break;
        }
        case E_GOP_GWIN_GPUTILE:
        {
            break;
        }
        case E_GOP_GWIN_STRETCH_OFFSET:
        {
              break;
        }
		default:
			break;
    }


    return GOP_API_SUCCESS;

}

MS_U32 Ioctl_GOP_FB_SetProperty(void* pInstance,EN_GOP_FB_PROPERTY type,MS_U32 FBId, MS_U32* pSet, MS_U32 u32Size)
{
    switch(type)
    {
        case E_GOP_FB_POOLID:
        {
            MS_U8 *pPool = (MS_U8*)pSet;

            if(u32Size != sizeof(MS_U8))
                return GOP_API_INVALID_PARAMETERS;

            GOP_FB_SEL(pInstance,*pPool);
            break;
        }
        case E_GOP_FB_EXIST:
        {
            break;
        }
        case E_GOP_FB_OBTAIN:
        {
            break;
        }
    }

    return GOP_API_SUCCESS;
}

MS_U32 Ioctl_GOP_FB_GetProperty(void* pInstance,EN_GOP_FB_PROPERTY type,MS_U32 FBId, MS_U32* pSet, MS_U32 u32Size)
{
    switch(type)
    {
        case E_GOP_FB_POOLID:
        {
            MS_U8 *pPool = (MS_U8*)pSet;

            if(u32Size != sizeof(MS_U8))
                return GOP_API_INVALID_PARAMETERS;

            *pPool = (MS_U8)GOP_FB_Get(pInstance);
            break;
        }

        case E_GOP_FB_EXIST:
        {
            MS_U8 *pExist = (MS_U8*)pSet;

            if(u32Size != sizeof(MS_U8))
                return GOP_API_INVALID_PARAMETERS;
            *pExist = (MS_U8)GOP_GWIN_Is32FBExist(pInstance,FBId);
            break;
        }
        case E_GOP_FB_OBTAIN:
        {
            MS_U32 *pFreeFbID = (MS_U32*)pSet;

            if(u32Size != sizeof(MS_U32))
                return GOP_API_INVALID_PARAMETERS;
            *pFreeFbID = (MS_U32)GOP_GWIN_GetFreeFbID(pInstance);
            break;
        }
    }

    return GOP_API_SUCCESS;
}

MS_U32 Ioctl_GOP_DWIN_SetProperty(void* pInstance,EN_GOP_DWIN_PROPERTY type,MS_U32* pSet, MS_U32 u32Size)
{
#ifdef INSTANT_PRIVATE
    GOP_INSTANT_PRIVATE* psGOPInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void*)&psGOPInstPri);
#endif

    switch(type)
    {
        case E_GOP_DWIN_INTERRUP_INFO:
        {
            break;
        }
        case E_GOP_DWIN_CAPTURE_INFO:
        {
            break;
        }
	default:
		break;
    }
    return GOP_API_SUCCESS;
}

MS_U32 Ioctl_GOP_MIXER_SetProperty(void* pInstance,EN_GOP_MIXER_PROPERTY type,MS_U32* pSet, MS_U32 u32Size)
{
    switch(type)
    {
        case E_GOP_MIXER_OUTPUT_TIMING:
        {
            PGOP_MIXER_TIMING_INFO pMTInfo = (PGOP_MIXER_TIMING_INFO )pSet;

            if(u32Size != sizeof(GOP_MIXER_TIMING_INFO))
                return GOP_API_INVALID_PARAMETERS;
            GOP_MIXER_SetOutputTiming(pInstance,pMTInfo->mode,&(pMTInfo->pMT));
            break;
        }
        case E_GOP_MIXER_TO_OP_OUTPUT_TIMING:
        {
            GOP_Mixer2OPTiming *pMT = (GOP_Mixer2OPTiming *)pSet;

            if(u32Size != sizeof(GOP_Mixer2OPTiming))
                return GOP_API_INVALID_PARAMETERS;
            GOP_MIXER_SetMIXER2OPOutputTiming(pInstance,pMT);
            break;
        }
        case E_GOP_MIXER_V_FILTER:
        {
            MS_BOOL bEn;

            if(u32Size != sizeof(MS_BOOL))
                return GOP_API_INVALID_PARAMETERS;

            bEn = (MS_BOOL)*pSet;
            GOP_MIXER_EnableVfilter(pInstance,bEn);
            break;
        }
		default:
		break;
    }
    return GOP_API_SUCCESS;
}

MS_U32 Ioctl_GOP_MIXER_GetProperty(void* pInstance,EN_GOP_MIXER_PROPERTY type,MS_U32* pSet, MS_U32 u32Size)
{
    switch(type)
    {
        case E_GOP_MIXER_OUTPUT_TIMING:
        {
            break;
        }
        case E_GOP_MIXER_TO_OP_OUTPUT_TIMING:
        {
            break;
        }
        case E_GOP_MIXER_V_FILTER:
        {
            break;
        }
		default:
		break;
    }
    return GOP_API_SUCCESS;
}

MS_U32 Ioctl_GOP_MapFB2Win(void* pInstance,MS_U32 fbid, MS_U32 gwin)
{
    E_GOP_API_Result eRet=GOP_API_FAIL;

    eRet = (E_GOP_API_Result)GOP_MapFB2Win(pInstance,fbid,gwin);
    return eRet;
}

MS_U32 Ioctl_GOP_SetWinInfo(void* pInstance,MS_U32 gwinId,PGOP_BUFFER_INFO pInfo)
{
    MS_U8 u8GOP;
    GOP_GwinInfo  wininfo;
    DRV_GOP_AFBC_Info  sAFBCWinProperty;
    memset(&sAFBCWinProperty, 0, sizeof(DRV_GOP_AFBC_Info));
#ifdef INSTANT_PRIVATE
    GOP_INSTANT_PRIVATE* psGOPInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void*)&psGOPInstPri);
#endif

    wininfo.clrType = (EN_GOP_COLOR_TYPE)pInfo->fbFmt;
    wininfo.u16DispHPixelStart = pInfo->disp_rect.x;
    wininfo.u16DispVPixelStart = pInfo->disp_rect.y;
    wininfo.u16RBlkHPixSize = pInfo->width;
    wininfo.u16RBlkVPixSize = pInfo->height;
    wininfo.u32DRAMRBlkStart = pInfo->addr;
    wininfo.u16RBlkHRblkSize = pInfo->pitch;
    wininfo.u16DispHPixelEnd = pInfo->disp_rect.x + pInfo->disp_rect.w;
    wininfo.u16DispVPixelEnd = pInfo->disp_rect.y + pInfo->disp_rect.h;
    wininfo.u16WinX = 0;
    wininfo.u16WinY = 0;

    if ((wininfo.u16DispHPixelEnd - wininfo.u16DispHPixelStart) > wininfo.u16RBlkHPixSize)
    {
        GOP_M_ERR("[%s][%d] display H Area > Buffer, x=%td, w=%td, disp_w=%td\n",__FUNCTION__,__LINE__, (ptrdiff_t)pInfo->disp_rect.x, (ptrdiff_t)pInfo->disp_rect.w, (ptrdiff_t)pInfo->width);
        wininfo.u16DispHPixelEnd = wininfo.u16DispHPixelStart + wininfo.u16RBlkHPixSize;
    }
    if ((wininfo.u16DispVPixelEnd - wininfo.u16DispVPixelStart) > wininfo.u16RBlkVPixSize)
    {
        GOP_M_ERR("[%s][%d] display V Area > Buffer, y=%td, h=%td, disp_h=%td\n",__FUNCTION__,__LINE__, (ptrdiff_t)pInfo->disp_rect.y, (ptrdiff_t)pInfo->disp_rect.h, (ptrdiff_t)pInfo->height);
        wininfo.u16DispVPixelEnd = wininfo.u16DispVPixelStart + wininfo.u16RBlkVPixSize;
    }

    GOP_M_INFO("\33[0;36m   %s:%d   u8win = %td \33[m \n",__FUNCTION__,__LINE__, (ptrdiff_t)gwinId);

    if (!_GOP_IsGwinIdValid(pInstance,gwinId))
    {
        GOP_M_ERR("[%s][%d]GWIN %td  is out of range\n",__FUNCTION__,__LINE__,(ptrdiff_t)gwinId);
        return GOP_API_FAIL;
    }
    u8GOP = MDrv_DumpGopByGwinId(g_pGOPCtxLocal,gwinId);

    if(u8GOP == GOPTYPE.GOP0)
    {
        _SetGop0WinInfo(pInstance,gwinId, &wininfo);
    }
    else if(u8GOP == GOPTYPE.GOP1)
    {
        _SetGop1WinInfo(pInstance,gwinId, &wininfo);
    }
    else
    {
        _SetGop23WinInfo(pInstance,gwinId, &wininfo);
    }
#ifdef CONFIG_GOP_AFBC_FEATURE
    if(g_pGOPCtxLocal->pGopChipProperty->bAFBC_Support[u8GOP]==TRUE)
    {
        sAFBCWinProperty.u16HPixelStart = wininfo.u16DispHPixelStart;
        sAFBCWinProperty.u16HPixelEnd = wininfo.u16DispHPixelEnd;
        sAFBCWinProperty.u16Pitch = wininfo.u16RBlkHRblkSize;
        sAFBCWinProperty.u16VPixelStart = wininfo.u16DispVPixelStart;
        sAFBCWinProperty.u16VPixelEnd= wininfo.u16DispVPixelEnd;
        sAFBCWinProperty.u64DRAMAddr = wininfo.u32DRAMRBlkStart;
        //MDrv_GOP_GWIN_AFBCSetWindow(g_pGOPCtxLocal, u8GOP, &sAFBCWinProperty,FALSE);
    }
#endif
    MDrv_GOP_GWIN_UpdateReg(g_pGOPCtxLocal,u8GOP);

    return GOP_API_SUCCESS;
}



MS_U32 Ioctl_GOP_GetWinInfo(void* pInstance,MS_U32 gwinId,PGOP_BUFFER_INFO pInfo)
{
    GOP_GwinInfo wininfo;


    if (!_GOP_IsGwinIdValid(pInstance,gwinId))
    {
        GOP_M_ERR("[%s][%d]GWIN %td  is out of range\n",__FUNCTION__,__LINE__,(ptrdiff_t)gwinId);
        return GOP_API_FAIL;
    }

    if (pInfo == NULL)
    {
        GOP_M_ERR("[%s][%d]pinfo is NULL\n",__FUNCTION__,__LINE__);
        return GOP_API_FAIL;
    }

    if (!_GOP_GWIN_IsGwinExistInClient(pInstance,gwinId)) //To check the gwin is exist
    {
        GOP_M_ERR("[%s][%d]GWIN %td  is not exist\n",__FUNCTION__,__LINE__,(ptrdiff_t)gwinId);
        return GOP_API_FAIL;
        //ASSERT(0);
    }

    GOP_GetWinInfo(pInstance,gwinId, &wininfo);

    pInfo->addr            = wininfo.u32DRAMRBlkStart;
    pInfo->disp_rect.x     = wininfo.u16DispHPixelStart;
    pInfo->disp_rect.y     = wininfo.u16DispVPixelStart;
    pInfo->disp_rect.w     = wininfo.u16DispHPixelEnd - wininfo.u16DispHPixelStart;
    pInfo->disp_rect.h     = wininfo.u16DispVPixelEnd - wininfo.u16DispVPixelStart;
    pInfo->width           = wininfo.u16RBlkHPixSize;
    pInfo->height          = wininfo.u16RBlkVPixSize;
    pInfo->pitch           = wininfo.u16RBlkHRblkSize;
    pInfo->fbFmt           = (MS_U32)wininfo.clrType;

    return GOP_API_SUCCESS;

}


MS_U32 Ioctl_GOP_Win_Enable(void* pInstance,MS_U32 gwinId,MS_BOOL bEn)
{
    MS_U16 regval2;
    MS_U32 u32fbId;
    GOP_WinFB_INFO* pwinFB;
    MS_U8 gwinNum = 0;
    MS_U8 i;
#ifdef INSTANT_PRIVATE
    GOP_INSTANT_PRIVATE* psGOPInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void*)&psGOPInstPri);
#endif


    if (!_GOP_IsGwinIdValid(pInstance,gwinId))
    {
        GOP_M_ERR("[%s][%d]GWIN %td  is out of range\n",__FUNCTION__,__LINE__,(ptrdiff_t)gwinId);
        return GOP_API_FAIL;
    }

    //printf("GWINID=%d,Enable=%d\n",(MS_U8)winId,(MS_U8)bEnable);
    if (!_GOP_GWIN_IsGwinExistInClient(pInstance,gwinId))
    {
        GOP_M_ERR("[%s][%d]GWIN %td  is not exist\n",__FUNCTION__,__LINE__,(ptrdiff_t)gwinId);
        //__ASSERT(0);
        return GOP_API_FAIL;
    }

    //   printf(" u8win=%02bx, bEnable=%02bx\n",u8win,bEnable);
    u32fbId = g_pGOPCtxLocal->pGOPCtxShared->gwinMap[gwinId].u32CurFBId;
    pwinFB = _GetWinFB(pInstance,u32fbId);

    if(pwinFB == NULL)
    {
        GOP_M_ERR("[%s][%d]GetWinFB Fail : WrongFBID=%td\n",__FUNCTION__,__LINE__,(ptrdiff_t)u32fbId);
        return GOP_API_FAIL;
    }

    if (!_GOP_IsFbIdValid(pInstance,u32fbId))
    {
        GOP_M_ERR("[%s][%d]FbId %td  is out of range\n",__FUNCTION__,__LINE__,(ptrdiff_t)u32fbId);
        return GOP_API_FAIL;
    }

    if (0 == pwinFB->in_use)
    {
        msWarning(ERR_GWIN_ID_NOT_ALLOCATED);
        GOP_M_ERR("[%s][%d]GWIN %td not allocated\n",__FUNCTION__,__LINE__,(ptrdiff_t)gwinId);
        return GOP_API_FAIL;
    }
     pwinFB->enable = bEn;

    for(i = 0; i < (MS_U8)g_pGOPCtxLocal->pGopChipProperty->TotalGwinNum ; i++)
    {
        if(GOP_GWIN_IsEnable(pInstance,i) == TRUE)
        {
            gwinNum |= 1<<i;
        }
    }

    regval2 = gwinNum;

    if (bEn == 0) // Turn off GOP
    {
        MDrv_GOP_GWIN_EnableGwin(g_pGOPCtxLocal, gwinId, FALSE);
        if ((regval2 & ~(1<<gwinId)) == 0)
        {
            if(fpXCReduceBWForOSD!=NULL)
            {
                fpXCReduceBWForOSD(XC_MAIN_WINDOW, FALSE);
            }
        }
    }
    else //turn on GOP
    {
        //printf("gop on, interlace=%bx\n", IsSrcInterlace());
        if(fpXCReduceBWForOSD!=NULL)
        {
            fpXCReduceBWForOSD(XC_MAIN_WINDOW, TRUE);
        }
        MDrv_GOP_GWIN_EnableGwin(g_pGOPCtxLocal, gwinId, TRUE);
    }

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
MS_U32 Ioctl_GOP_SetDisplay(void* pInstance,PGOP_GWINDISPLAY_INFO pInfo)
{
    MS_U8 u8GOP;
    MS_U32 GwinId = pInfo->gwin;
    MS_U16 u16Tmp, u16OrigWidth = 0;
    MS_U16 gwin_w, stretch_w;
    MS_BOOL u16OrgUpdateRegOnce = FALSE;
    GOP_StretchInfo* pStretchInfo;
    GOP_GwinInfo* pGwinInfo = NULL;
    MS_U32 u32FbId;
    EN_GOP_STRETCH_DIRECTION  direction;
    MS_U16 u16DstWidth, u16DstHeight;
#ifdef INSTANT_PRIVATE
    GOP_INSTANT_PRIVATE* psGOPInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void*)&psGOPInstPri);
#endif

    u32FbId = pInfo->fbid;
    direction = pInfo->dir;
    u16DstWidth = pInfo->dst_size.w;
    u16DstHeight = pInfo->dst_size.h;

    pGwinInfo = &(pInfo->gwin_info);
    pStretchInfo = &(pInfo->stretch_info);

    u16OrgUpdateRegOnce = g_pGOPCtxLocal->bUpdateRegOnce[GOP_PUBLIC_UPDATE];
    //Save old update_reg_once state, and change to update_reg_once=TRUE internally
    if(u16OrgUpdateRegOnce == FALSE)
    {
        g_pGOPCtxLocal->bUpdateRegOnce[GOP_PUBLIC_UPDATE] = TRUE;
    }
    GOP_MapFB2Win(pInstance,u32FbId,GwinId);

    u8GOP = MDrv_DumpGopByGwinId(g_pGOPCtxLocal,GwinId);
    //MApi_GOP_GWIN_Set_STRETCHWIN(u8GOP, pStretchInfo->eDstType,pStretchInfo->x, pStretchInfo->y
    //    , pStretchInfo->width, pStretchInfo->height);
    _GOP_Get_StretchWin(pInstance,u8GOP, &u16Tmp, &u16Tmp, &u16OrigWidth, &u16Tmp);

    gwin_w = pGwinInfo->u16DispHPixelEnd - pGwinInfo->u16DispHPixelStart;

    if(gwin_w > pStretchInfo->width)
        stretch_w = gwin_w;
    else
        stretch_w = pStretchInfo->width;

    _GOP_GWIN_Align_StretchWin(pInstance,u8GOP,pStretchInfo->eDstType , &pStretchInfo->x, &pStretchInfo->y, &stretch_w, &pStretchInfo->height, _GOP_GetBPP(pInstance,pGwinInfo->clrType));
    //Store API use stretch window
    g_pGOPCtxLocal->pGOPCtxShared->u16APIStretchWidth[u8GOP] = stretch_w;
    g_pGOPCtxLocal->pGOPCtxShared->u16APIStretchHeight[u8GOP]= pStretchInfo->height;

    _GOP_GWIN_Adjust_3DStretchWin(pInstance,u8GOP, &pStretchInfo->x, &pStretchInfo->y, &stretch_w, &pStretchInfo->height);
    if (u16OrigWidth <= stretch_w)//Set a large one, set htt first
        MDrv_GOP_GWIN_SetHTotal(g_pGOPCtxLocal, u8GOP, stretch_w);

    MDrv_GOP_GWIN_SetStretchWin(g_pGOPCtxLocal, u8GOP, pStretchInfo->x, pStretchInfo->y, stretch_w, pStretchInfo->height);

    if(u16DstWidth <= pStretchInfo->width)
        GOP_Set_Hscale(pInstance,u8GOP,FALSE, pStretchInfo->width, u16DstWidth);
    else if (direction & E_GOP_H_STRETCH)
        GOP_Set_Hscale(pInstance,u8GOP,TRUE, pStretchInfo->width, u16DstWidth);

    if(u16DstHeight < pStretchInfo->height)
        GOP_Set_Vscale(pInstance,u8GOP,FALSE, pStretchInfo->height, u16DstHeight);
    else if (direction & E_GOP_V_STRETCH)
        GOP_Set_Vscale(pInstance,u8GOP,TRUE, pStretchInfo->height, u16DstHeight);

    GOP_SetWinInfo(pInstance,GwinId,pGwinInfo);

    //Restore update_reg_once and trigger register writes in
    if(u16OrgUpdateRegOnce == FALSE)
    {
        g_pGOPCtxLocal->bUpdateRegOnce[GOP_PUBLIC_UPDATE] = u16OrgUpdateRegOnce;
    }

    if (u16OrigWidth > stretch_w)//Set a small one, set htt after Gwin regsiter updated
        MDrv_GOP_GWIN_SetHTotal(g_pGOPCtxLocal, u8GOP, stretch_w);

    return GOP_API_SUCCESS;
}



MS_U32 Ioctl_GOP_Win_Destroy(void* pInstance,MS_U32 gId)
{
    E_GOP_API_Result eRet=GOP_API_FAIL;
    eRet = (E_GOP_API_Result)GOP_Win_Destroy(pInstance,gId);
    return eRet;
}

MS_U32 Ioctl_GOP_FBCreate(void* pInstance,EN_GOP_CREATEBUFFER_TYPE fbtype,PGOP_BUFFER_INFO  pBuff,MS_U32 fbId)
{

    if(fbtype == GOP_CREATE_BUFFER_BYADDR)
        GOP_CreateBufferByAddr(pInstance,pBuff,fbId);

    return GOP_API_SUCCESS;

}


MS_U32 Ioctl_GOP_FBDestroy(void* pInstance,MS_U32* pId)
{
    GOP_WinFB_INFO* pwinFB;
    MS_U32 u32fbId;

    u32fbId = *pId;

    if (DRV_MAX_GWIN_FB_SUPPORT <= u32fbId)
    {
        msWarning(ERR_FB_ID_OUT_OF_RANGE);
        GOP_M_ERR("%s(u32fbId:%td....), fbId out of bound\n", __FUNCTION__, (ptrdiff_t)u32fbId);
        return GWIN_NO_AVAILABLE;

    }
    pwinFB = _GetWinFB(pInstance,u32fbId);

    if(pwinFB == NULL)
    {
        GOP_M_ERR("[%s][%d]GetWinFB Fail : WrongFBID=%td\n",__FUNCTION__,__LINE__, (ptrdiff_t)u32fbId);
        return GWIN_NO_AVAILABLE;
    }

    if(0 == pwinFB->in_use)
    {
        msWarning(ERR_FB_ID_NOT_ALLOCATED);
        GOP_M_ERR("[%s][%d]: u32fbId=%td is not in existence\n", __FUNCTION__, __LINE__, (ptrdiff_t)u32fbId);
        return GWIN_NO_AVAILABLE;

    }

    GOP_FB_Destroy(pInstance,u32fbId);

    return GWIN_OK;

}

MS_U32 Ioctl_GOP_GetFBInfo(void* pInstance,MS_U32 u32fbId, PGOP_BUFFER_INFO pBuff)
{
    GOP_WinFB_INFO* pwinFB;


    if (!_GOP_IsFbIdValid(pInstance,u32fbId))
    {
        GOP_M_ERR("[%s][%d]FbId %td  is out of range\n",__FUNCTION__,__LINE__,(ptrdiff_t)u32fbId);
        return GOP_API_FAIL;
    }

    pwinFB = _GetWinFB(pInstance,u32fbId);

    if(pwinFB == NULL)
    {
        GOP_M_ERR("[%s][%d]GetWinFB Fail : WrongFBID=%td\n",__FUNCTION__,__LINE__,(ptrdiff_t)u32fbId);
        return GOP_API_FAIL;
    }

    if(pwinFB->in_use ==0)
    {
        msWarning(ERR_FB_ID_NOT_ALLOCATED);
        GOP_M_ERR("[%s][%d]: u32fbId=%td is not in existence\n",__FUNCTION__,__LINE__,(ptrdiff_t)u32fbId);
        return GOP_API_FAIL;
    }

    pBuff->addr= pwinFB->addr;
    pBuff->fbFmt = pwinFB->fbFmt;
    pBuff->width = pwinFB->width;
    pBuff->height = pwinFB->height;
    pBuff->pitch = pwinFB->pitch;
    pBuff->disp_rect.x = pwinFB->x0;
    pBuff->disp_rect.w = pwinFB->x1 - pwinFB->x0;
    pBuff->disp_rect.y = pwinFB->y0;
    pBuff->disp_rect.h = pwinFB->y1 - pwinFB->y0;

    return GOP_API_SUCCESS;

}

MS_U32 Ioctl_GOP_SetFBInfo(void* pInstance,MS_U32 u32fbId, PGOP_BUFFER_INFO pBuff)
{
    GOP_WinFB_INFO* pwinFB;


    if (!_GOP_IsFbIdValid(pInstance,u32fbId))
    {
        GOP_M_ERR("[%s][%d]FbId %td  is out of range\n",__FUNCTION__,__LINE__,(ptrdiff_t)u32fbId);
        return GOP_API_FAIL;
    }
    pwinFB = _GetWinFB(pInstance,u32fbId);

    if(pwinFB == NULL)
    {
        GOP_M_ERR("[%s][%d]GetWinFB Fail : WrongFBID=%td\n",__FUNCTION__,__LINE__,(ptrdiff_t)u32fbId);
        return GOP_API_FAIL;
    }

    pwinFB->enable      = 1   ;
    pwinFB->gWinId      = 0xFF   ;
    pwinFB->fbFmt       = pBuff->fbFmt    ;
    pwinFB->width       = pBuff->width    ;
    pwinFB->height      = pBuff->height   ;

    pwinFB->x0          = pBuff->disp_rect.x;
    pwinFB->x1          = pBuff->disp_rect.x + pBuff->disp_rect.w;
    pwinFB->y0          = pBuff->disp_rect.y       ;
    pwinFB->y1          = pBuff->disp_rect.y + pBuff->disp_rect.h;
    pwinFB->s_x         = 0;
    pwinFB->s_y         = 0;
    pwinFB->dispWidth   = pBuff->disp_rect.w ;
    pwinFB->dispHeight  = pBuff->disp_rect.h;

    if(pwinFB->in_use &&  GOP_WINFB_POOL_NULL== pwinFB->poolId)
    {
        pwinFB->addr        = pBuff->addr     ;
        pwinFB->size        = pBuff->pitch * pBuff->height;
        pwinFB->pitch       = pBuff->pitch    ;
     }

    return GOP_API_SUCCESS;
}


MS_U32 GOP_TriggerRegWriteIn(void* pInstance,MS_U32 gop, MS_BOOL bForceWriteIn, MS_BOOL bSync)
{
#ifdef INSTANT_PRIVATE
    GOP_INSTANT_PRIVATE* psGOPInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void*)&psGOPInstPri);
#endif

    MDrv_GOP_TriggerRegWriteIn(g_pGOPCtxLocal, gop, bForceWriteIn, bSync);
    return GOP_API_SUCCESS;


}
MS_U32 GOP_UpdateOnce(void* pInstance,MS_U32 gop, MS_BOOL bUpdateOnce, MS_BOOL bSync)
{
#ifdef INSTANT_PRIVATE
    GOP_INSTANT_PRIVATE* psGOPInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void*)&psGOPInstPri);
#endif

    g_pGOPCtxLocal->bUpdateRegOnce[GOP_PUBLIC_UPDATE] = bUpdateOnce;
    if(bUpdateOnce == FALSE)
    {
        MDrv_GOP_GWIN_UpdateRegWithSync(g_pGOPCtxLocal, gop, bSync);
    }
    return GOP_API_SUCCESS;

}

MS_U32 GOP_UpdateCurrentOnce(void* pInstance, MS_U16 u16GopMask, MS_BOOL bUpdateOnce, MS_BOOL bSync)
{
#ifdef INSTANT_PRIVATE
    GOP_INSTANT_PRIVATE* psGOPInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void*)&psGOPInstPri);
#endif
    MS_U16 u16GopIdx = 0;
    for (u16GopIdx = 0; u16GopIdx < MDrv_GOP_GetMaxGOPNum(g_pGOPCtxLocal); u16GopIdx++)
    {
        if ((u16GopMask & (1<<u16GopIdx)) != 0)
        {
            g_pGOPCtxLocal->bUpdateRegOnce[u16GopIdx] = bUpdateOnce;
        }
    }
    if(bUpdateOnce == FALSE)
    {
        MDrv_GOP_GWIN_UpdateRegWithMaskSync(g_pGOPCtxLocal, u16GopMask, bSync);
    }
    return GOP_API_SUCCESS;

}

MS_U32 Ioctl_GOP_TriggerRegWriteIn(void* pInstance,MS_U32 gop, EN_GOP_UPDATE_TYPE update_type, MS_BOOL bEn, MS_BOOL bSync)
{

#ifdef INSTANT_PRIVATE
    GOP_INSTANT_PRIVATE* psGOPInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void*)&psGOPInstPri);
#endif

    switch(update_type)
    {
        case E_GOP_UPDATE_FORCEWRITE:
        {
            GOP_TriggerRegWriteIn(pInstance,gop, bEn, bSync);
            break;
        }
        case E_GOP_UPDATE_CURRENT_ONCE:
        {
            GOP_UpdateCurrentOnce(pInstance, gop, bEn, bSync);
            break;
        }
        default:
        case E_GOP_UPDATE_ONCE:
        {
            GOP_UpdateOnce(pInstance,gop, bEn, bSync);
            break;
        }

    }

    return GOP_API_SUCCESS;

}


MS_U32 Ioctl_GOP_Select(void* pInstance,EN_GOP_SELECT_TYPE sel, MS_U32 id, MS_U32* pSet)
{
#ifdef INSTANT_PRIVATE
    GOP_INSTANT_PRIVATE* psGOPInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void*)&psGOPInstPri);
#endif
    DRV_GOP_CBFmtInfo* FmtInfo = (DRV_GOP_CBFmtInfo*)pSet;

    switch(sel)
    {
        case EN_GOP_SEL_GOP:
        {
            if (!_GOP_IsGopNumValid(pInstance,id))
            {
                GOP_M_ERR("[%s][%d]GOP %td  is out of range\n",__FUNCTION__,__LINE__,(ptrdiff_t)id);
                return GOP_API_FAIL;
            }
            MDrv_GOP_Sel(g_pGOPCtxLocal, id);
            break;
        }
        case EN_GOP_SEL_GWIN:
            break;
        case EN_GOP_SEL_FB:
        {
            MS_U16 fbFmt;
            GOP_WinFB_INFO* pwinFB;

           // printf("[%s] We DO NOT suggest to use this function\n",__FUNCTION__);
           // printf("[%s] Please use GFX MApi_GFX_SetDstBufferInfo() !!\n",__FUNCTION__);

            if (!_GOP_IsFbIdValid(pInstance,id))
            {
                GOP_M_ERR("[%s][%d]FbId %td  is out of range\n",__FUNCTION__,__LINE__,(ptrdiff_t)id);
                return GOP_API_FAIL;
            }
            pwinFB = _GetWinFB(pInstance,id);

            if(pwinFB == NULL)
            {
                GOP_M_ERR("[%s][%d]GetWinFB Fail : WrongFBID=%td\n",__FUNCTION__,__LINE__,(ptrdiff_t)id);
                return GOP_API_FAIL;
            }

            if(0 == pwinFB->in_use)
            {
                msWarning(ERR_FB_ID_NOT_ALLOCATED);
                GOP_M_ERR("[%s][%d]: u32fbId=%td is not in existence\n",__FUNCTION__,__LINE__,(ptrdiff_t)id);
                return GOP_API_FAIL;
            }

            fbFmt = pwinFB->fbFmt;
            if ( (pwinFB->fbFmt & 0xFF) == E_MS_FMT_ARGB1555 )
                fbFmt = E_MS_FMT_ARGB1555_DST | (E_MS_FMT_ARGB1555_DST << 8);

            if(fpSetFBFmt != NULL)
            {
                fpSetFBFmt(pwinFB->pitch, pwinFB->addr,fbFmt);
                FmtInfo->u64Addr = pwinFB->addr;
                FmtInfo->u16Pitch = pwinFB->pitch;
                FmtInfo->u16Fmt = fbFmt;
            }
            else
            {
                FmtInfo->u64Addr = pwinFB->addr;
                FmtInfo->u16Pitch = pwinFB->pitch;
                FmtInfo->u16Fmt = fbFmt;
            }
            break;
        }
    }

    return GOP_API_SUCCESS;

}

MS_U32 Ioctl_GOP_Register_CB(void* pInstance,GOP_CB_TYPE cb_type, void* pCB, MS_U32 u32Size)
{
#ifdef MSOS_TYPE_LINUX_KERNEL

#else
    switch(cb_type)
    {
        case GOP_CB_XC_INTERLACE:
        {
            _fpXCIsInterlace = (FP_XC_IsInterlace)pCB;
            break;
        }
        case GOP_CB_XC_REDUCE_BW:
        {
            fpXCReduceBWForOSD = (FP_XC_ReduceBWForOSD)pCB;
            break;
        }
        case GOP_CB_XC_PANEL_HSTART:
        {
            _fpXCGetCapHStart = (FP_XC_GetCapHStart)pCB;
            break;
        }
        case GOP_CB_GFX_FBFMT:
        {
            fpSetFBFmt = (FP_SetFBFmt)pCB;
            break;
        }
        case GOP_CB_EVENT_NOTIFY:
        {
            fpEventNotify = (FP_EventNotify)pCB;
            break;
        }
        case GOP_CB_DWIN_INFO:
        {
            fpXCSetDwinInfo = (FP_XC_SetDwinInfo)pCB;
            break;
        }
        default:
        {
            GOP_M_ERR("[%s][%d] invalid input call back function type\n",__FUNCTION__,__LINE__);
            return GOP_API_INVALID_PARAMETERS;
        }
    }
#endif
    return (GOP_API_SUCCESS);
}

MS_U32 Ioctl_GOP_MISC(void* pInstance,EN_GOP_MISC_TYPE type, MS_U32* pSet, MS_U32 u32Size)
{
#ifdef INSTANT_PRIVATE
    GOP_INSTANT_PRIVATE* psGOPInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void*)&psGOPInstPri);
#endif

    switch(type)
    {
        case E_GOP_MISC_ENABLE_T3D:
        {
            MS_BOOL bEn;
            if(u32Size != sizeof(MS_BOOL))
            {
                GOP_M_ERR("[%s][%d] size ERROR!! \n",__FUNCTION__,__LINE__);
                return GOP_API_FAIL;
            }

            bEn = *((MS_BOOL*)pSet);
            GOP_GWIN_EnableT3DMode(pInstance,bEn);
            break;
        }
        case E_GOP_MISC_SET_CAPTURE_CLK:
        {
            if(u32Size != sizeof(MS_U32))
            {
                GOP_M_ERR("[%s][%d] size ERROR!! \n",__FUNCTION__,__LINE__);
                return GOP_API_FAIL;
            }
            MDrv_GOP_SetClkForCapture(g_pGOPCtxLocal, GOP_DRV_DWIN_SRC_OP);
            break;
        }
        case E_GOP_MISC_POWER_OFF:
        {
            if(u32Size != sizeof(MS_U32))
            {
                GOP_M_ERR("[%s][%d] size ERROR!! \n",__FUNCTION__,__LINE__);
                return GOP_API_FAIL;
            }
            GOP_PowerOff(pInstance);
            break;
        }
        case E_GOP_MISC_POWER_ON:
        {
            if(u32Size != sizeof(MS_U32))
            {
                GOP_M_ERR("[%s][%d] size ERROR!! \n",__FUNCTION__,__LINE__);
                return GOP_API_FAIL;
            }
            GOP_PowerOn(pInstance);
            break;
        }
        case E_GOP_MISC_GET_RT_STATUS:
        {
            if(u32Size != sizeof(GOP_ApiStatus))
            {
                GOP_M_ERR("[%s][%d] size ERROR!! \n",__FUNCTION__,__LINE__);
                return GOP_API_FAIL;
            }
            GOP_ApiStatus *pGopApiSts = (GOP_ApiStatus *)pSet;
            GOP_GetRTStatus(pInstance,pGopApiSts);
            break;
        }
        case E_GOP_MISC_GET_INFO:
        {
            if(u32Size != sizeof(GOP_ApiInfo))
            {
                GOP_M_ERR("[%s][%d] size ERROR!! \n",__FUNCTION__,__LINE__);
                return GOP_API_FAIL;
            }
            GOP_ApiInfo *pGopApiInfo = (GOP_ApiInfo *)pSet;
            GOP_GetInfo(pInstance,pGopApiInfo);
            break;
        }
        case E_GOP_MISC_AT_EXIT:
        {
            if(u32Size != sizeof(MS_U32))
            {
                GOP_M_ERR("[%s][%d] size ERROR!! \n",__FUNCTION__,__LINE__);
                return GOP_API_FAIL;
            }
            Ioctl_GOP_AtExit(pInstance);
            break;
        }
        case E_GOP_MISC_OC_FB_INFO:
        {
            if(u32Size != sizeof(GOP_OC_FB_INFO))
            {
                GOP_M_ERR("[%s][%d] size ERROR!! \n",__FUNCTION__,__LINE__);
                return GOP_API_FAIL;
            }
            PGOP_OC_FB_INFO pOCFbInfo = (PGOP_OC_FB_INFO)(void*)pSet;
            GOP_OC_SetOCFBinfo(pInstance,pOCFbInfo->u64Addr,pOCFbInfo->BankMode,pOCFbInfo->enBufSize);
            break;
        }
        case E_GOP_MISC_DELETE_GWINSIZE:
        {
            if(u32Size != sizeof(GOP_DeleteWinSize_PARAM))
            {
                GOP_M_ERR("[%s][%d] size ERROR!! \n",__FUNCTION__,__LINE__);
                return GOP_API_FAIL;
            }

            GOP_DeleteWinSize_PARAM* gop_VaildSize = (GOP_DeleteWinSize_PARAM*)pSet;
            MDrv_GOP_GWIN_DeleteWinHVSize(g_pGOPCtxLocal, gop_VaildSize->u32GOP_idx, gop_VaildSize->u32HSize, gop_VaildSize->u32VSize);
            break;
        }
        case E_GOP_MISC_SET_DBG_LEVEL:
        {
            if(u32Size != sizeof(MS_U32))
            {
                GOP_M_ERR("[%s][%d] size ERROR!! \n",__FUNCTION__,__LINE__);
                return GOP_API_FAIL;
            }
            u32GOPDbgLevel_mapi= (MS_U32)*pSet;
            MDrv_GOP_SetDbgLevel((EN_GOP_DEBUG_LEVEL)u32GOPDbgLevel_mapi);
            break;
        }
        case E_GOP_MISC_GET_OSD_NONTRANS_CNT:
        {
            if(u32Size != sizeof(MS_U32))
            {
                GOP_M_ERR("[%s][%d] size ERROR!! \n",__FUNCTION__,__LINE__);
                return GOP_API_FAIL;
            }
            MDrv_GOP_GetOsdNonTransCnt(g_pGOPCtxLocal, pSet);
            break;
        }
    }
    return GOP_API_SUCCESS;
}

MS_U32 Ioctl_GOP_PowerState(void* pInstance,MS_U32 u32PowerState, void* pModule)
{
    static MS_U16 u16ActWin = 0;

    MS_U8 i=0;
    MS_U32 u32Return = GOP_API_FAIL;
    MS_U8 u8GwinIdx;
    MS_U16 u16WinNum = 0;

    GOP_CTX_DRV_SHARED* pDrvGOPShared=NULL;
    MS_BOOL bNeedInitShared = FALSE;
#ifdef INSTANT_PRIVATE
        GOP_INSTANT_PRIVATE* psGOPInstPri = NULL;
        UtopiaInstanceGetPrivate(pInstance, (void*)&psGOPInstPri);
#endif

    GOP_M_INFO("[%s][%d] IoctlGOP STR PowerState=%tx\n",__FUNCTION__,__LINE__,(ptrdiff_t)u32PowerState);

    pDrvGOPShared = (GOP_CTX_DRV_SHARED*)MDrv_GOP_GetShareMemory(&bNeedInitShared);

    if(pDrvGOPShared == NULL)
    {
        GOP_M_ERR("[%s][%d] Pointer pDrvGOPShared is null!! \n",__FUNCTION__,__LINE__);
        return GOP_API_FAIL;
    }
    if(pDrvGOPShared->apiCtxShared.bEnableVsyncIntFlip ==FALSE)//Usr RedLine:GFLIP  Suspend/Resume
    {
        switch(u32PowerState)
        {
            case E_GOP_POWER_SUSPEND:
                GOP_M_INFO("[%s][%d] IoctlGOP STR Suspend Start!!!!!\n",__FUNCTION__,__LINE__);
                MDrv_GOP_GWIN_PowerState(pInstance, u32PowerState, pModule);
                GOP_M_INFO("[%s][%d] IoctlGOP STR Suspend End!!!!!\n",__FUNCTION__,__LINE__);
                u32Return=GOP_API_SUCCESS;
                break;
            case E_GOP_POWER_RESUME:
                GOP_M_INFO("[%s][%d] IoctlGOP Resume Start!!!!!\n",__FUNCTION__,__LINE__);
                MDrv_GOP_GWIN_PowerState(pInstance, u32PowerState, pModule);
                GOP_M_INFO("[%s][%d] IoctlGOP STR Resume End!!!!!\n",__FUNCTION__,__LINE__);
                u32Return=GOP_API_SUCCESS;
                break;
            default:
                GOP_M_ERR("[%s][%d] PowerState:%tx not Implement now!!!\n",__FUNCTION__,__LINE__,(ptrdiff_t)u32PowerState);
                break;
        }
    }
    else//Kernel RedLine:GFLIP  Suspend/Resume
    {
        u16WinNum = (MS_U8)g_pGOPCtxLocal->pGopChipProperty->TotalGwinNum;

        switch(u32PowerState)
        {
            case E_GOP_POWER_SUSPEND:
            {
                for(i = 0; i < u16WinNum ; i++)
                {
                    if(GOP_GWIN_IsEnable(pInstance,i) == TRUE)
                    {
                        u16ActWin |= 1<<i;
                        GOP_GWIN_SetEnable(pInstance,i, FALSE);
                    }
                }

            }
                break;
            case E_GOP_POWER_RESUME:
            {
                if (u16ActWin != INVALID_GWIN_ID)
                {
                    for (u8GwinIdx = 0; u8GwinIdx < u16WinNum; u8GwinIdx++)
                    {
                        if (0 != (u16ActWin & (1<<u8GwinIdx)))
                        {
                            GOP_GWIN_SetEnable(pInstance,u8GwinIdx, TRUE);
                            GOP_GWIN_SetForceWrite(pInstance,TRUE);
                            GOP_GWIN_SetForceWrite(pInstance,FALSE);
                        }
                    }
                }
                else
                {
                    GOP_M_INFO("[%s] Error!!! Need suspend first!!!\n",__FUNCTION__);
                }
                u16ActWin = 0;
            }
                break;
            default:
                GOP_M_ERR("[%s][%d] PowerState:%tx not Implement now!!!\n",__FUNCTION__,__LINE__,(ptrdiff_t)u32PowerState);
                break;
        }
        u32Return=GOP_API_SUCCESS;
    }

    return u32Return;
}

MS_U32 Ioctl_GOP_PINPON(void* pInstance,EN_GOP_PINPON_PROPERTY en_pro, MS_U32* pSet, MS_U32 u32Size)
{
#ifdef INSTANT_PRIVATE
    GOP_INSTANT_PRIVATE* psGOPInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void*)&psGOPInstPri);
#endif
    switch(en_pro)
    {
        case E_GOP_SET_PINPON:
        {
            if(u32Size != sizeof(GOP_SET_PINPON_INFO))
            {
                GOP_M_ERR("[%s][%d] size ERROR!! \n",__FUNCTION__,__LINE__);
                return GOP_API_FAIL;
            }
            PGOP_SET_PINPON_INFO pPINPONInfo = (PGOP_SET_PINPON_INFO)pSet;

            MS_U8 u8GOP;
            MS_U8 u8FirstGwinByGOP;

            u8GOP = MDrv_DumpGopByGwinId(g_pGOPCtxLocal, pPINPONInfo->GwinId);
            u8FirstGwinByGOP = MDrv_GOP_SelGwinIdByGOP(g_pGOPCtxLocal, u8GOP, 0);

            //check it is the first gwin on this GOP
            if(pPINPONInfo->GwinId != u8FirstGwinByGOP)
            {
                GOP_M_ERR("[%s][%d] gwin:%d PINPON mode only support the first gwin on GOP:%d\n",__FUNCTION__,__LINE__,pPINPONInfo->GwinId,u8GOP);
                return GOP_API_FAIL;
            }

            MDrv_GOP_Set_PINPON(g_pGOPCtxLocal, u8GOP, pPINPONInfo->bEnable, (E_DRV_GOP_PINPON_MODE)pPINPONInfo->mode);
            break;
        }
    }
    return GOP_API_SUCCESS;
}

MS_U32 Ioctl_GOP_Scroll(void* pInstance,EN_GOP_SCROLL_TYPE type, MS_U32* pSet, MS_U32 u32Size)
{
#ifdef INSTANT_PRIVATE
    GOP_INSTANT_PRIVATE* psGOPInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void*)&psGOPInstPri);
#endif

    switch(type)
    {
        case E_GOP_SCROLL_CONFIG:
        {
            if(u32Size != sizeof(GOP_SCROLL_INFO))
            {
                GOP_M_ERR("[%s][%d] size ERROR!! \n",__FUNCTION__,__LINE__);
                return GOP_API_FAIL;
            }
            PGOP_SCROLL_INFO pScrollInfo = (PGOP_SCROLL_INFO)pSet;
            if (pScrollInfo->type == E_GOP_SCROLL_DOWN || pScrollInfo->type == E_GOP_SCROLL_UP)
            {
                MDrv_GOP_GWIN_SetVScroll(g_pGOPCtxLocal, pScrollInfo->u8WinId,  (E_GOP_SCROLL_TYPE)pScrollInfo->type,  (MS_U32)pScrollInfo->u16Offset, pScrollInfo->bEnable);
            }
            else
            {
                MDrv_GOP_GWIN_SetHScroll(g_pGOPCtxLocal,pScrollInfo->u8WinId,  (E_GOP_SCROLL_TYPE)pScrollInfo->type,  (MS_U32)pScrollInfo->u16Offset, pScrollInfo->bEnable);
            }
            break;
        }
        case E_GOP_SCROLL_RATE:
        {
            MS_U8 u8rate;
            if(u32Size != sizeof(MS_U8))
            {
                GOP_M_ERR("[%s][%d] size ERROR!! \n",__FUNCTION__,__LINE__);
                return GOP_API_FAIL;
            }

            u8rate = (MS_U8)*pSet;
            MDrv_GOP_GWIN_SetScrollRate(g_pGOPCtxLocal, u8rate);
            break;
        }
    }
    return GOP_API_SUCCESS;
}
MS_U32 Ioctl_GOP_SetLayer(void* pInstance,PGOP_SETLayer pstLayer,MS_U32 u32Size)
{
    MS_U16 u16Idx = 0;
    Gop_MuxSel enMux = E_GOP_MUX_INVAILD;

#ifdef INSTANT_PRIVATE
    GOP_INSTANT_PRIVATE* psGOPInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void*)&psGOPInstPri);
#endif

    for (u16Idx = 0; u16Idx < pstLayer->u32LayerCount; u16Idx++)
    {
        MDrv_GOP_MapLayer2Mux(g_pGOPCtxLocal, pstLayer->u32Layer[u16Idx], pstLayer->u32Gop[u16Idx], (MS_U32 *)&enMux);
        MDrv_GOP_GWIN_SetMux(g_pGOPCtxLocal, pstLayer->u32Gop[u16Idx], enMux);
        if (g_pGOPCtxLocal->pGOPCtxShared->bGopHasInitialized[pstLayer->u32Gop[u16Idx]])
        {
            _GOP_InitHSPDByGOP(pInstance, pstLayer->u32Gop[u16Idx]);
        }
    }

    if(g_pGOPCtxLocal->pGopChipProperty->bOPMuxDoubleBuffer == TRUE)
    {
        MDrv_GOP_GWIN_Trigger_MUX(g_pGOPCtxLocal);
    }
    else
    {
        GOP_M_WARN("[%s]GOP no OPMux double buffer, so layer configuration (dfbrc.ini & OsdConfig.ini) need to match with each other\n",__FUNCTION__);
    }

    return GOP_API_SUCCESS;
}
MS_U32 Ioctl_GOP_GetLayer(void* pInstance,PGOP_SETLayer pstLayer,MS_U32 u32Size)
{
#ifdef INSTANT_PRIVATE
    GOP_INSTANT_PRIVATE* psGOPInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void*)&psGOPInstPri);
#endif

    MS_U16 u16Idx = 0;
    DRV_GOPDstType enDst[SHARED_GOP_MAX_COUNT] = {E_DRV_GOP_DST_INVALID};
    MS_BOOL bHasOP0 = FALSE;
    MS_BOOL bHasDualOP0 = FALSE;

    //get all gop dst
    for (u16Idx = 0; u16Idx < MDrv_GOP_GetMaxGOPNum(g_pGOPCtxLocal); u16Idx++)
    {
        enDst[u16Idx] = E_DRV_GOP_DST_INVALID;
        if (g_pGOPCtxLocal->pGOPCtxShared->bGopHasInitialized[u16Idx])
        {
            MDrv_GOP_GWIN_GetDstPlane(g_pGOPCtxLocal, u16Idx, &enDst[u16Idx]);
        }
    }

    pstLayer->u32LayerCount = 0;
    // record all gop
    for (u16Idx = 0; u16Idx < MDrv_GOP_GetMaxGOPNum(g_pGOPCtxLocal); u16Idx++)
    {
        switch (enDst[u16Idx])
        {
            case E_DRV_GOP_DST_OP0:
            {
                if (!bHasOP0)
                {
                    MS_U32 u32Layer = 0;
                    // record all op gop
                    for (u32Layer = 0; u32Layer < MAX_GOP_MUX_OPNum; u32Layer++)
                    {
                        MS_U8 u8Gop = 0;
                        MDrv_GOP_GWIN_GetMux(g_pGOPCtxLocal, &u8Gop, g_pGOPCtxLocal->pGopChipProperty->GOP_MapLayer2Mux[u32Layer]);
                        if ((u8Gop < MDrv_GOP_GetMaxGOPNum(g_pGOPCtxLocal)) && (enDst[u8Gop] == E_DRV_GOP_DST_OP0))
                        {
                            pstLayer->u32Gop[pstLayer->u32LayerCount] = u8Gop;
                            pstLayer->u32Layer[pstLayer->u32LayerCount] = u32Layer;
                            pstLayer->u32LayerCount++;
                        }
                    }
                    bHasOP0 = TRUE;
                }
                break;
            }
            case E_DRV_GOP_DST_OP_DUAL_RATE:
                if (!bHasDualOP0)
                {
                    MS_U32 u32Layer = 0;

                    for (u32Layer = 0; u32Layer < MAX_GOP_DualMUX_Num; u32Layer++)
                    {
                        MS_U8 u8Gop = 0;
                        MDrv_GOP_GWIN_GetMux(g_pGOPCtxLocal, &u8Gop, g_pGOPCtxLocal->pGopChipProperty->GOP_MapLayer2DualOpMux[u32Layer]);
                        if ((u8Gop < MDrv_GOP_GetMaxGOPNum(g_pGOPCtxLocal)) && (enDst[u8Gop] == E_DRV_GOP_DST_OP_DUAL_RATE))
                        {
                            pstLayer->u32Gop[pstLayer->u32LayerCount] = u8Gop;
                            pstLayer->u32Layer[pstLayer->u32LayerCount] = u32Layer;
                            pstLayer->u32LayerCount++;
                        }
                    }
                    bHasDualOP0 = TRUE;
                }
                break;
            default:
                break;

        }
    }
    return GOP_API_SUCCESS;
}

int Ioctl_GOP_Interrupt(void* pInstance,MS_U8 u8Gop,MS_BOOL bEnable)
{
#ifdef INSTANT_PRIVATE
    GOP_INSTANT_PRIVATE* psGOPInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void*)&psGOPInstPri);
#endif

    MDrv_GOP_GWIN_Interrupt(g_pGOPCtxLocal,u8Gop,bEnable);
    return GOP_API_SUCCESS;
}

void Ioctl_GOP_AtExit(void* pInstance)
{
#if defined(MSOS_TYPE_LINUX) || defined(MSOS_TYPE_LINUX_KERNEL)
    GOP_AtExit(pInstance);
#else
    GOP_M_INFO("not enable MSOS_TYPE_LINUX\n");
#endif
}
#endif

#ifndef CONFIG_GOP_UTOPIA10
// this func will be call to init by utopia20 framework
void GOPRegisterToUtopia(void)
{
    // 1. deal with module
    void* pUtopiaModule = NULL;
	UtopiaModuleCreate(MODULE_GOP, 8, &pUtopiaModule);
    UtopiaModuleRegister(pUtopiaModule);
    // register func for module, after register here, then ap call UtopiaOpen/UtopiaIoctl/UtopiaClose can call to these registered standard func
    UtopiaModuleSetupFunctionPtr(pUtopiaModule, (FUtopiaOpen)GOPOpen, (FUtopiaClose)GOPClose, (FUtopiaIOctl)GOPIoctl);

    // 2. deal with resource
    void* psResource = NULL;
    // start func to add res, call once will create 2 access in resource. Also can declare BDMA_POOL_ID_BDMA1 for another channel depend on driver owner.
	UtopiaModuleAddResourceStart(pUtopiaModule, GOP_POOL_ID_GOP0);
    // resource can alloc private for internal use, ex, BDMA_RESOURCE_PRIVATE
    UtopiaResourceCreate("gop0", sizeof(GOP_RESOURCE_PRIVATE), &psResource);
    // func to reg res
    UtopiaResourceRegister(pUtopiaModule, psResource, GOP_POOL_ID_GOP0);

	UtopiaModuleAddResourceEnd(pUtopiaModule, GOP_POOL_ID_GOP0);


}

MS_U32 GOPOpen(void** ppInstance, const void* const pAttribute)
{
    GOP_INSTANT_PRIVATE *pGopPri = NULL;
	GOP_M_INFO("\n[GOP INFO] gop open\n");
    //UTOPIA_TRACE(MS_UTOPIA_DB_LEVEL_TRACE,printf("enter %s %d\n",__FUNCTION__,__LINE__));
    // instance is allocated here, also can allocate private for internal use, ex, BDMA_INSTANT_PRIVATE
    UtopiaInstanceCreate(sizeof(GOP_INSTANT_PRIVATE), ppInstance);
#ifndef CONFIG_UTOPIAXP_REMOVE_IMPL
    // setup func in private and assign the calling func in func ptr in instance private
    UtopiaInstanceGetPrivate(*ppInstance, (void**)&pGopPri);

#ifdef MSOS_TYPE_LINUX_KERNEL
#if GFLIP_ENABLE
    extern MS_U32 MDrv_GFLIP_InitTimer(void);
    if(_devGFLIPCnt==0)
    {
        MDrv_GFLIP_InitTimer();
    }
    _devGFLIPCnt++;
#endif
#endif
#endif
	return UTOPIA_STATUS_SUCCESS;
}

MS_U32 GOPIoctl(void* pInstance, MS_U32 u32Cmd, void* pArgs)
{
#ifndef CONFIG_UTOPIAXP_REMOVE_IMPL
	void*                       pModule         = NULL;
	UtopiaInstanceGetModule(pInstance, &pModule);
    MS_U32                      u32Ret          = GOP_API_SUCCESS;
    GOP_INSTANT_PRIVATE*        psGOPInstPri    = NULL;
    MS_BOOL                     bLocked         = FALSE;
	UtopiaInstanceGetPrivate(pInstance, (void**)&psGOPInstPri);

    if(u32Cmd == MAPI_CMD_GOP_RESETPOOL)
    {
#if defined(MSOS_TYPE_LINUX) || defined(MSOS_TYPE_LINUX_KERNEL)
        GOP_CTX_DRV_SHARED* pDrvGOPShared=NULL;
        MS_BOOL bNeedInitShared = FALSE;
        pDrvGOPShared = (GOP_CTX_DRV_SHARED*)(void*)MDrv_GOP_GetShareMemory(&bNeedInitShared);
        if(pDrvGOPShared == NULL)
        {
           GOP_M_ERR("[%s] [%d] Shm pointer is NULL!!!! \n",__FUNCTION__,__LINE__);
           return UTOPIA_STATUS_FAIL;
        }
        if((pDrvGOPShared->apiCtxShared.GOPLockPid != 0))
        {
#if defined(MSOS_TYPE_LINUX)
            if (kill(pDrvGOPShared->apiCtxShared.GOPLockPid, 0) == ESRCH)
#endif
            {
                GOP_M_INFO("[%s] [%d] apiCtxShared.GOPLockPid 0x%tx not existance!!! \n",__FUNCTION__,__LINE__,  (ptrdiff_t)pDrvGOPShared->apiCtxShared.GOPLockPid);
                UtopiaModuleResetPool(pModule,GOP_POOL_ID_GOP0);
            }
        }
#endif
        return UTOPIA_STATUS_SUCCESS;
    }

    if( (u32Cmd != MAPI_CMD_GOP_INTERRUPT) && (u32Cmd != MAPI_CMD_GOP_MUTEX) && (g_pGOPCtxLocal != NULL) && (g_pGOPCtxLocal->pGOPCtxShared->s32GOPLockTid != MsOS_GetOSThreadID()) )
    {
        if(UtopiaResourceObtain(pModule, GOP_POOL_ID_GOP0, &(psGOPInstPri->pResource)) != 0)
        {
                GOP_M_ERR("[%s %d]UtopiaResourceObtainToInstant fail\n",__FUNCTION__,__LINE__);
            return 0xFFFFFFFF;
        }
#if defined(MSOS_TYPE_LINUX) || defined(MSOS_TYPE_LINUX_KERNEL)
        g_pGOPCtxLocal->pGOPCtxShared->GOPLockPid = (GETPIDTYPE)getpid();
#endif
        bLocked=TRUE;
    }
    //printf("[%s] cmd:%lx\n",__FUNCTION__,u32Cmd);

    switch(u32Cmd)
    {
        case MAPI_CMD_GOP_INIT:
        {
            PGOP_INIT_PARAM ptr = (PGOP_INIT_PARAM)pArgs;

#ifndef MSOS_TYPE_LINUX_KERNEL //Check size error when if(is_compat_task()==1)
            if(ptr->u32Size != sizeof(GOP_InitInfo) && g_pGOPCtxLocal != NULL)
            {
                GOP_M_INFO("[%s][%d] input structure size non match!! (input:%tx require:%tx\n",__FUNCTION__,__LINE__,(ptrdiff_t)ptr->u32Size, sizeof(GOP_INIT_PARAM));
                g_pGOPCtxLocal->pGOPCtxShared->s32GOPLockTid = 0;
#if defined(MSOS_TYPE_LINUX) || defined(MSOS_TYPE_LINUX_KERNEL)
                g_pGOPCtxLocal->pGOPCtxShared->GOPLockPid = 0;
#endif
                UtopiaResourceRelease(psGOPInstPri->pResource);
                return UTOPIA_STATUS_PARAMETER_ERROR;
            }
#endif
            u32Ret = Ioctl_GOP_Init(pInstance,ptr->gop_idx,(GOP_InitInfo*)(void*)ptr->pInfo);

            break;
        }
        case MAPI_CMD_GOP_GET_CHIPCAPS:
        {
            PGOP_GETCAPS_PARAM ptr = (PGOP_GETCAPS_PARAM)pArgs;

            u32Ret = Ioctl_GOP_GetCaps(pInstance,(EN_GOP_CAPS)ptr->caps,ptr->pInfo,ptr->u32Size);

            break;
        }
        case MAPI_CMD_GOP_SET_CONFIG:
        {
            PGOP_SETCONFIG_PARAM ptr = (PGOP_SETCONFIG_PARAM)pArgs;
            u32Ret = Ioctl_GOP_SetConfig(pInstance,ptr->cfg_type, ptr->pCfg, ptr->u32Size);
            break;

        }
        case MAPI_CMD_GOP_GET_CONFIG:
        {
            PGOP_SETCONFIG_PARAM ptr = (PGOP_SETCONFIG_PARAM)pArgs;
            u32Ret = Ioctl_GOP_GetConfig(pInstance,ptr->cfg_type, ptr->pCfg, ptr->u32Size);
            break;
        }
        case MAPI_CMD_GOP_SET_PROPERTY:
        {
            PGOP_SET_PROPERTY_PARAM ptr = (PGOP_SET_PROPERTY_PARAM)pArgs;


            u32Ret = Ioctl_GOP_SetProperty(pInstance,ptr->en_pro ,ptr->gop_idx, ptr->pSetting,ptr->u32Size);

            break;
        }
        case MAPI_CMD_GOP_GET_PROPERTY:
        {
            PGOP_SET_PROPERTY_PARAM ptr = (PGOP_SET_PROPERTY_PARAM)pArgs;


            u32Ret = Ioctl_GOP_GetProperty(pInstance,ptr->en_pro ,ptr->gop_idx, ptr->pSetting,ptr->u32Size);

            break;
        }
        case MAPI_CMD_GOP_SET_DST:
        {
            PGOP_SETDST_PARAM  ptr = (PGOP_SETDST_PARAM)pArgs;

            u32Ret = Ioctl_GOP_SetDst(pInstance,ptr->gop_idx, ptr->en_dst);
            break;
        }
        case MAPI_CMD_GOP_GET_DST:
        {
            PGOP_GETDST_PARAM  ptr = (PGOP_GETDST_PARAM)pArgs;
            if(ptr->u32Size != sizeof(EN_GOP_DST_TYPE) && g_pGOPCtxLocal != NULL)
            {
                GOP_M_INFO("[%s][%d] Cmd:%td input structure size non match!! (input:0x%tx require:0x%tx\n",__FUNCTION__,__LINE__,(ptrdiff_t)u32Cmd,(ptrdiff_t)ptr->u32Size, sizeof(EN_GOP_DST_TYPE));
                g_pGOPCtxLocal->pGOPCtxShared->s32GOPLockTid = 0;
#if defined(MSOS_TYPE_LINUX) || defined(MSOS_TYPE_LINUX_KERNEL)
                g_pGOPCtxLocal->pGOPCtxShared->GOPLockPid = 0;
#endif
                UtopiaResourceRelease(psGOPInstPri->pResource);
                return UTOPIA_STATUS_PARAMETER_ERROR;
            }

            u32Ret = Ioctl_GOP_GetDst(pInstance,ptr->gop_idx, ptr->pDst);
            break;
        }
        case MAPI_CMD_GOP_SET_MUX:
        {
            PGOP_SETMUX_PARAM  ptr = (PGOP_SETMUX_PARAM)pArgs;
            //PGOP_MuxConfig  pMux = (PGOP_MuxConfig)ptr->pMuxInfo;
            PGOP_SETMUX  pMux = (PGOP_SETMUX)ptr->pMuxInfo;
#ifndef MSOS_TYPE_LINUX_KERNEL //Check size error when if(is_compat_task()==1)
            if(ptr->u32Size != sizeof(GOP_SETMUX) && g_pGOPCtxLocal != NULL)
            {
                g_pGOPCtxLocal->pGOPCtxShared->s32GOPLockTid = 0;
#if defined(MSOS_TYPE_LINUX) || defined(MSOS_TYPE_LINUX_KERNEL)
                g_pGOPCtxLocal->pGOPCtxShared->GOPLockPid = 0;
#endif
                UtopiaResourceRelease(psGOPInstPri->pResource);
                return UTOPIA_STATUS_PARAMETER_ERROR;
            }
#endif
            u32Ret = Ioctl_GOP_SetMux(pInstance,pMux, ptr->u32Size);
            break;
        }
        case MAPI_CMD_GOP_SET_LAYER:
        {
            PGOP_SETLAYER_PARAM ptr = (PGOP_SETLAYER_PARAM)pArgs;
#ifndef MSOS_TYPE_LINUX_KERNEL //Check size error when if(is_compat_task()==1)
            if(ptr->u32Size != sizeof(GOP_SETLayer) && g_pGOPCtxLocal != NULL)
            {
	            g_pGOPCtxLocal->pGOPCtxShared->s32GOPLockTid = 0;
#if defined(MSOS_TYPE_LINUX) || defined(MSOS_TYPE_LINUX_KERNEL)
                g_pGOPCtxLocal->pGOPCtxShared->GOPLockPid = 0;
#endif
                UtopiaResourceRelease(psGOPInstPri->pResource);
                return UTOPIA_STATUS_PARAMETER_ERROR;
            }
#endif
            if(g_pGOPCtxLocal == NULL)
            {
                return UTOPIA_STATUS_FAIL;
            }
            Ioctl_GOP_SetLayer(pInstance,(PGOP_SETLayer)ptr->pLayerInfo,ptr->u32Size);
            break;
        }

        case MAPI_CMD_GOP_GET_LAYER:
        {
            PGOP_SETLAYER_PARAM ptr = (PGOP_SETLAYER_PARAM)pArgs;
#ifndef MSOS_TYPE_LINUX_KERNEL //Check size error when if(is_compat_task()==1)
            if(ptr->u32Size != sizeof(GOP_SETLayer) && g_pGOPCtxLocal != NULL)
            {
	            g_pGOPCtxLocal->pGOPCtxShared->s32GOPLockTid = 0;
#if defined(MSOS_TYPE_LINUX) || defined(MSOS_TYPE_LINUX_KERNEL)
                g_pGOPCtxLocal->pGOPCtxShared->GOPLockPid = 0;
#endif
                UtopiaResourceRelease(psGOPInstPri->pResource);
                return UTOPIA_STATUS_PARAMETER_ERROR;
            }
#endif
            if(g_pGOPCtxLocal == NULL)
            {
                return UTOPIA_STATUS_FAIL;
            }
            Ioctl_GOP_GetLayer(pInstance,(PGOP_SETLayer)ptr->pLayerInfo,ptr->u32Size);
            break;
        }

        case MAPI_CMD_GOP_SET_MIRROR:
        {
            PGOP_SETMIRROR_PARAM ptr = (PGOP_SETMIRROR_PARAM)pArgs;
            u32Ret = Ioctl_GOP_SetMirror(pInstance,ptr->gop_idx, ptr->dir);

            break;
        }
        case MAPI_CMD_GOP_GET_MIRROR:
        {
            PGOP_GETMIRROR_PARAM ptr = (PGOP_GETMIRROR_PARAM)pArgs;
            u32Ret = Ioctl_GOP_GetMirror(pInstance,ptr->gop_idx, ptr->pdir);

            break;
        }
        case MAPI_CMD_GOP_GET_STATUS:
        {
            PGOP_GET_STATUS_PARAM ptr = (PGOP_GET_STATUS_PARAM)pArgs;

            u32Ret = Ioctl_GOP_GetStatus(pInstance,ptr->en_status, ptr->pStatus,ptr->u32Size);

            break;
        }
        //Stretch Win
        case MAPI_CMD_GOP_GWIN_SET_STRETCH:
        {
            PGOP_STRETCH_SET_PARAM ptr = (PGOP_STRETCH_SET_PARAM)pArgs;
            PGOP_STRETCH_INFO pInfo = (PGOP_STRETCH_INFO)ptr->pStretch;
            if(ptr->u32Size != sizeof(GOP_STRETCH_INFO))
            {
                GOP_M_ERR("[%s] (%d) Info structure Error!!\n",__FUNCTION__,__LINE__);
                return GOP_API_INVALID_PARAMETERS;
            }

            u32Ret = Ioctl_GOP_Set_Stretch(pInstance,ptr->enStrtchType,ptr->gop_idx,pInfo);
            break;
        }
        case MAPI_CMD_GOP_GWIN_GET_STRETCH:
        {
            PGOP_STRETCH_SET_PARAM ptr = (PGOP_STRETCH_SET_PARAM)pArgs;
            PGOP_STRETCH_INFO pInfo = (PGOP_STRETCH_INFO)ptr->pStretch;
            if(ptr->u32Size != sizeof(GOP_STRETCH_INFO))
            {
                GOP_M_ERR("[%s] (%d) Info structure Error!!\n",__FUNCTION__,__LINE__);
                return GOP_API_INVALID_PARAMETERS;
            }

            u32Ret = Ioctl_GOP_Get_Stretch(pInstance,ptr->enStrtchType,ptr->gop_idx,pInfo);
            break;
        }

        //GWIN info
        case MAPI_CMD_GOP_GWIN_SET_PROPERTY:
        {
            PGOP_GWIN_PROPERTY_PARAM ptr = (PGOP_GWIN_PROPERTY_PARAM)pArgs;

            u32Ret = Ioctl_GOP_GWin_SetProperty(pInstance,ptr->en_property, ptr->GwinId, ptr->pSet, ptr->u32Size);

            break;
        }
        case MAPI_CMD_GOP_GWIN_GET_PROPERTY:
        {
            PGOP_GWIN_PROPERTY_PARAM ptr = (PGOP_GWIN_PROPERTY_PARAM)pArgs;

            u32Ret = Ioctl_GOP_GWin_GetProperty(pInstance,ptr->en_property, ptr->GwinId, ptr->pSet, ptr->u32Size);

            break;
        }

        case MAPI_CMD_GOP_GWIN_MAPFB2WIN:
        {
            PGOP_GWIN_MAPFBINFO_PARAM ptr = (PGOP_GWIN_MAPFBINFO_PARAM)pArgs;
            //PGOP_BUFFER_INFO pInfo = (PGOP_BUFFER_INFO)ptr->pinfo;

            u32Ret = Ioctl_GOP_MapFB2Win(pInstance,ptr->fbid,ptr->GwinId);

            break;
        }
        case MAPI_CMD_GOP_GWIN_SET_WININFO:
        {
            PGOP_GWIN_SETWININFO_PARAM ptr = (PGOP_GWIN_SETWININFO_PARAM)pArgs;
            PGOP_BUFFER_INFO pInfo = (PGOP_BUFFER_INFO)ptr->pinfo;

#ifndef MSOS_TYPE_LINUX_KERNEL //Check size error when if(is_compat_task()==1)
            if(ptr->u32Size != sizeof(GOP_BUFFER_INFO) && g_pGOPCtxLocal != NULL)
            {
                GOP_M_INFO("[%s] (%d) Info structure Error!!\n",__FUNCTION__,__LINE__);
                g_pGOPCtxLocal->pGOPCtxShared->s32GOPLockTid = 0;
#if defined(MSOS_TYPE_LINUX) || defined(MSOS_TYPE_LINUX_KERNEL)
                g_pGOPCtxLocal->pGOPCtxShared->GOPLockPid = 0;
#endif
                UtopiaResourceRelease(psGOPInstPri->pResource);
                return UTOPIA_STATUS_PARAMETER_ERROR;
            }
#endif
            u32Ret = Ioctl_GOP_SetWinInfo(pInstance,ptr->GwinId,pInfo);

            break;
        }
        case MAPI_CMD_GOP_GWIN_GET_WININFO:
        {
            PGOP_GWIN_SETWININFO_PARAM ptr = (PGOP_GWIN_SETWININFO_PARAM)pArgs;
            PGOP_BUFFER_INFO pInfo = (PGOP_BUFFER_INFO)ptr->pinfo;
#ifndef MSOS_TYPE_LINUX_KERNEL //Check size error when if(is_compat_task()==1)
            if(ptr->u32Size != sizeof(GOP_BUFFER_INFO) && g_pGOPCtxLocal != NULL)
            {
                GOP_M_INFO("[%s] (%d) Info structure Error!!\n",__FUNCTION__,__LINE__);
                g_pGOPCtxLocal->pGOPCtxShared->s32GOPLockTid = 0;
#if defined(MSOS_TYPE_LINUX) || defined(MSOS_TYPE_LINUX_KERNEL)
                g_pGOPCtxLocal->pGOPCtxShared->GOPLockPid = 0;
#endif
                UtopiaResourceRelease(psGOPInstPri->pResource);
                return UTOPIA_STATUS_PARAMETER_ERROR;
            }
#endif
            u32Ret = Ioctl_GOP_GetWinInfo(pInstance,ptr->GwinId,pInfo);

            break;
        }

#if 0
        case MAPI_CMD_GOP_GWIN_ENABLE:
        {
            PGOP_GWIN_WIN_ENABLE_PARAM ptr = (PGOP_GWIN_WIN_ENABLE_PARAM)pArgs;
            MS_BOOL bEn;

            bEn = (MS_BOOL)*(ptr->pEn);
            u32Ret = Ioctl_GOP_Win_Enable(ptr->GwinId, bEn);

            break;
        }
#endif
        case MAPI_CMD_GOP_GWIN_SETDISPLAY:
        {
            PGOP_GWIN_DISPLAY_PARAM ptr = (PGOP_GWIN_DISPLAY_PARAM)pArgs;
            PGOP_GWINDISPLAY_INFO pInfo = (PGOP_GWINDISPLAY_INFO)ptr->pDisplayInfo;
#ifndef MSOS_TYPE_LINUX_KERNEL //Check size error when if(is_compat_task()==1)
            if(ptr->u32Size != sizeof(GOP_GWINDISPLAY_INFO) && g_pGOPCtxLocal != NULL)
            {
                g_pGOPCtxLocal->pGOPCtxShared->s32GOPLockTid = 0;
#if defined(MSOS_TYPE_LINUX) || defined(MSOS_TYPE_LINUX_KERNEL)
                g_pGOPCtxLocal->pGOPCtxShared->GOPLockPid = 0;
#endif
                UtopiaResourceRelease(psGOPInstPri->pResource);
                return UTOPIA_STATUS_PARAMETER_ERROR;
            }
#endif
            u32Ret = Ioctl_GOP_SetDisplay(pInstance,pInfo);
            break;
        }
        case MAPI_CMD_GOP_GWIN_DESTROY:
        {
            PGOP_GWIN_DESTROY_PARAM ptr = (PGOP_GWIN_DESTROY_PARAM)pArgs;

            u32Ret = Ioctl_GOP_Win_Destroy(pInstance,ptr->GwinId);
        }
            break;
        //FB Info
        case MAPI_CMD_GOP_FB_CREATE:
        {
            PGOP_CREATE_BUFFER_PARAM ptr = (PGOP_CREATE_BUFFER_PARAM)pArgs;
            PGOP_BUFFER_INFO pBuff = (PGOP_BUFFER_INFO)ptr->pBufInfo;
#ifndef MSOS_TYPE_LINUX_KERNEL //Check size error when if(is_compat_task()==1)
            if(ptr->u32Size != sizeof(GOP_BUFFER_INFO) && g_pGOPCtxLocal != NULL)  //error handling for invalid ptr
            {
                GOP_M_INFO("[%s] (%d) Info structure Error!!\n",__FUNCTION__,__LINE__);
                g_pGOPCtxLocal->pGOPCtxShared->s32GOPLockTid = 0;
#if defined(MSOS_TYPE_LINUX) || defined(MSOS_TYPE_LINUX_KERNEL)
                g_pGOPCtxLocal->pGOPCtxShared->GOPLockPid = 0;
#endif
                UtopiaResourceRelease(psGOPInstPri->pResource);
                return UTOPIA_STATUS_PARAMETER_ERROR;
            }
#endif

            u32Ret = Ioctl_GOP_FBCreate(pInstance,ptr->fb_type,pBuff,ptr->fbid);
            break;
        }
        case MAPI_CMD_GOP_FB_DESTROY:
        {
            PGOP_DELETE_BUFFER_PARAM ptr = (PGOP_DELETE_BUFFER_PARAM)pArgs;

            if(ptr->u32Size != sizeof(MS_U32) && g_pGOPCtxLocal != NULL)  //error handling for invalid ptr
            {
                GOP_M_INFO("[%s] (%d) Info structure Error!!\n",__FUNCTION__,__LINE__);
                g_pGOPCtxLocal->pGOPCtxShared->s32GOPLockTid = 0;
#if defined(MSOS_TYPE_LINUX) || defined(MSOS_TYPE_LINUX_KERNEL)
                g_pGOPCtxLocal->pGOPCtxShared->GOPLockPid = 0;
#endif
                UtopiaResourceRelease(psGOPInstPri->pResource);
                return UTOPIA_STATUS_PARAMETER_ERROR;
            }
            u32Ret = Ioctl_GOP_FBDestroy(pInstance,ptr->pBufId);

            break;
        }
        case MAPI_CMD_GOP_FB_SETINFO:
        {
            PGOP_FB_INFO_PARAM ptr = (PGOP_FB_INFO_PARAM)pArgs;
            PGOP_BUFFER_INFO pBuff  = (PGOP_BUFFER_INFO)ptr->pBufInfo;
#ifndef MSOS_TYPE_LINUX_KERNEL //Check size error when if(is_compat_task()==1)
            if(ptr->u32Size != sizeof(GOP_BUFFER_INFO) && g_pGOPCtxLocal != NULL)  //error handling for invalid ptr
            {
                GOP_M_INFO("[%s] (%d) Info structure Error!!\n",__FUNCTION__,__LINE__);
                g_pGOPCtxLocal->pGOPCtxShared->s32GOPLockTid = 0;
#if defined(MSOS_TYPE_LINUX) || defined(MSOS_TYPE_LINUX_KERNEL)
                g_pGOPCtxLocal->pGOPCtxShared->GOPLockPid = 0;
#endif
                UtopiaResourceRelease(psGOPInstPri->pResource);
                return UTOPIA_STATUS_PARAMETER_ERROR;
            }
#endif
            u32Ret = Ioctl_GOP_SetFBInfo(pInstance,ptr->fbid,pBuff);

            break;
        }
        case MAPI_CMD_GOP_FB_GETINFO:
        {
            PGOP_FB_INFO_PARAM ptr = (PGOP_FB_INFO_PARAM)pArgs;
            PGOP_BUFFER_INFO pBuff  = (PGOP_BUFFER_INFO)ptr->pBufInfo;

            u32Ret = Ioctl_GOP_GetFBInfo(pInstance,ptr->fbid,pBuff);
            break;
        }

        case MAPI_CMD_GOP_FB_SET_PROPERTY:
        {
            PGOP_FB_PROPERTY_PARAM ptr = (PGOP_FB_PROPERTY_PARAM)pArgs;

            u32Ret = Ioctl_GOP_FB_SetProperty(pInstance,ptr->en_property, ptr->FBId, ptr->pSet, ptr->u32Size);

            break;
        }
        case MAPI_CMD_GOP_FB_GET_PROPERTY:
        {
            PGOP_FB_PROPERTY_PARAM ptr = (PGOP_FB_PROPERTY_PARAM)pArgs;

            u32Ret = Ioctl_GOP_FB_GetProperty(pInstance,ptr->en_property, ptr->FBId, ptr->pSet, ptr->u32Size);

            break;
        }
        //Scroll
        case MAPI_CMD_GOP_SET_SCROLL:
        {
            PGOP_SCROLL_PARAM ptr = (PGOP_SCROLL_PARAM)pArgs;
            u32Ret = Ioctl_GOP_Scroll(pInstance,ptr->scroll_type,ptr->pCfg,ptr->u32Size);
            break;
        }
        //PINPON
        case MAPI_CMD_GOP_SET_PINPON:
        {
            PGOP_SET_PINPON_PARAM ptr = (PGOP_SET_PINPON_PARAM)pArgs;
            u32Ret = Ioctl_GOP_PINPON(pInstance,ptr->en_pro,ptr->pSetting,ptr->u32Size);
            break;
        }
        case MAPI_CMD_GOP_DWIN_SET_PROPERTY:
        {
            PGOP_DWIN_PROPERTY_PARAM ptr = (PGOP_DWIN_PROPERTY_PARAM)pArgs;

            u32Ret = Ioctl_GOP_DWIN_SetProperty(pInstance,ptr->en_dwin_property,ptr->pDwinProp,ptr->u32Size);
            break;
        }
        //MISC
        case MAPI_CMD_GOP_UPDATE:
        {
            PGOP_UPDATE_PARAM ptr = (PGOP_UPDATE_PARAM)pArgs;
            PGOP_UPDATE_INFO pInfo = (PGOP_UPDATE_INFO)ptr->pUpdateInfo;

            if(ptr->u32Size != sizeof(GOP_UPDATE_INFO) && g_pGOPCtxLocal != NULL)
            {
                g_pGOPCtxLocal->pGOPCtxShared->s32GOPLockTid = 0;
#if defined(MSOS_TYPE_LINUX) || defined(MSOS_TYPE_LINUX_KERNEL)
                g_pGOPCtxLocal->pGOPCtxShared->GOPLockPid = 0;
#endif
                UtopiaResourceRelease(psGOPInstPri->pResource);
                return UTOPIA_STATUS_PARAMETER_ERROR;
            }

            u32Ret = Ioctl_GOP_TriggerRegWriteIn(pInstance,pInfo->gop_idx,pInfo->update_type,pInfo->bEn,pInfo->bSync);
            break;
        }
        case MAPI_CMD_GOP_SELECTION:
        {
            PGOP_SELECTION_PROPERTY_PARAM ptr = (PGOP_SELECTION_PROPERTY_PARAM)pArgs;

            u32Ret = Ioctl_GOP_Select(pInstance,ptr->sel_type, ptr->id, ptr->pinfo);

            break;
        }
        case MAPI_CMD_GOP_REGISTER_CB:
        {
            PGOP_REGISTER_CB_PARAM ptr = (PGOP_REGISTER_CB_PARAM)pArgs;

            u32Ret = Ioctl_GOP_Register_CB(pInstance,ptr->cb_type, ptr->pCB, ptr->u32Size);

            break;
        }
        case MAPI_CMD_GOP_MUTEX:
        {
            GOP_MUTEX_PARAM *ptr = (GOP_MUTEX_PARAM*)pArgs;

#ifndef MSOS_TYPE_LINUX_KERNEL //Check size error when if(is_compat_task()==1)
            if(ptr->u32Size != sizeof(GOP_MUTEX_PARAM))
            {
                return UTOPIA_STATUS_PARAMETER_ERROR;
            }
#endif
            if(ptr->en_mutex == E_GOP_LOCK)
            {
                if(UtopiaResourceObtain(pModule, GOP_POOL_ID_GOP0, &(psGOPInstPri->pResource)) != 0)
                {
                    GOP_M_ERR("[%s %d]UtopiaResourceObtainToInstant fail\n",__FUNCTION__,__LINE__);
                    return 0xFFFFFFFF;
                }
                g_pGOPCtxLocal->pGOPCtxShared->s32GOPLockTid = MsOS_GetOSThreadID();
#if defined(MSOS_TYPE_LINUX) || defined(MSOS_TYPE_LINUX_KERNEL)
                g_pGOPCtxLocal->pGOPCtxShared->GOPLockPid = (GETPIDTYPE)getpid();
#endif
            }
            else if(ptr->en_mutex == E_GOP_UNLOCK)
            {
                g_pGOPCtxLocal->pGOPCtxShared->s32GOPLockTid = 0;
#if defined(MSOS_TYPE_LINUX) || defined(MSOS_TYPE_LINUX_KERNEL)
                g_pGOPCtxLocal->pGOPCtxShared->GOPLockPid = 0;
#endif
                UtopiaResourceRelease(psGOPInstPri->pResource);
            }
            break;
        }
        case MAPI_CMD_GOP_MIXER_SET_PROPERTY:
        {
            PGOP_MIXER_PROPERTY_PARAM ptr = (PGOP_MIXER_PROPERTY_PARAM)pArgs;

            u32Ret = Ioctl_GOP_MIXER_SetProperty(pInstance,ptr->en_mixer_property,ptr->pMixerProp,ptr->u32Size);
            break;
        }
        case MAPI_CMD_GOP_MIXER_GET_PROPERTY:
        {
            PGOP_MIXER_PROPERTY_PARAM ptr = (PGOP_MIXER_PROPERTY_PARAM)pArgs;

            u32Ret = Ioctl_GOP_MIXER_GetProperty(pInstance,ptr->en_mixer_property,ptr->pMixerProp,ptr->u32Size);
            break;
        }
        case MAPI_CMD_GOP_MISC:
        {
            PGOP_MISC_PARAM ptr = (PGOP_MISC_PARAM)pArgs;
            u32Ret = Ioctl_GOP_MISC(pInstance,ptr->misc_type,ptr->pMISC,ptr->u32Size);
            break;
        }
        case MAPI_CMD_GOP_POWERSTATE:
        {
            PGOP_POWERSTATE_PARAM ptr = (PGOP_POWERSTATE_PARAM)pArgs;
            MS_U8* u8Val = ptr->pInfo;
            u32Ret = Ioctl_GOP_PowerState(pInstance, (MS_U32)*u8Val, pModule);
            break;
        }
        case MAPI_CMD_GOP_INTERRUPT:
        {
            PGOP_INTERRUPT_PARAM ptr = (PGOP_INTERRUPT_PARAM)pArgs;
            MS_BOOL *pbEnable =(MS_BOOL *)ptr->pSetting;
            u32Ret = Ioctl_GOP_Interrupt(pInstance,ptr->gop_idx,*pbEnable);
            break;
        }
        default:
            break;
    }

    if(bLocked && g_pGOPCtxLocal != NULL)
    {
        g_pGOPCtxLocal->pGOPCtxShared->s32GOPLockTid = 0;
#if defined(MSOS_TYPE_LINUX) || defined(MSOS_TYPE_LINUX_KERNEL)
        g_pGOPCtxLocal->pGOPCtxShared->GOPLockPid = 0;
#endif
        UtopiaResourceRelease(psGOPInstPri->pResource);
    }
    if(u32Ret != GOP_API_SUCCESS)
    {
        if(u32Ret == GOP_API_FUN_NOT_SUPPORTED)
        {
            return UTOPIA_STATUS_NOT_SUPPORTED;
        }
        else
        {
            GOP_M_ERR("[%s][%d] ERROR on cmd:0x%tx\n",__FUNCTION__,__LINE__, (ptrdiff_t)u32Cmd);
            return UTOPIA_STATUS_FAIL;
        }
    }

    //printf("(%s) Done\n\n",__FUNCTION__);

#endif
	return UTOPIA_STATUS_SUCCESS;
}

MS_U32 GOPClose(void* pInstance)
{
#ifndef CONFIG_UTOPIAXP_REMOVE_IMPL
#ifdef MSOS_TYPE_LINUX_KERNEL
        extern MS_U32 MDrv_GFLIP_Del_Timer(void);
        if(_devGFLIPCnt==1)
        {
            MDrv_GFLIP_Del_Timer();
        }
#endif

    Ioctl_GOP_AtExit(pInstance);
#endif
	UtopiaInstanceDelete(pInstance);

	return TRUE;
}
#endif

