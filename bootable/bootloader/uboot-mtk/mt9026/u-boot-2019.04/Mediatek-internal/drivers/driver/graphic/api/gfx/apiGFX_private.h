/* SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause) */
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

#ifndef _APIGOP_PRIV_H_
#define _APIGOP_PRIV_H_

////////////////////////////////////////////////////////////////////////////////
// Header Files
////////////////////////////////////////////////////////////////////////////////
#ifdef __cplusplus
extern "C"
{
#endif
//Local define
#define CONFIG_GFX_UTOPIA10	(1)
#define PATCH_T3_SCK_FAIL 1 //[2009.07.02]T3/U3 GE HW issue:set source alpha is max for filtering the background color

#define GFX_DEBUGINFO_LEVEL(a, x)           if (g_apiGFXLocal.u32dbglvl <= a) {MS_DEBUG_MSG(x);}
#define GFX_DBG_HDR     "[GFXDBG]"

#define GFX_DEBUGINFO(x)                    //MS_DEBUG_MSG(x)

#undef GFX_ASSERT
#ifdef GFX_DEBUG
#define GFX_ASSERT(_bool, _f)                 if (!(_bool)) { MS_CRITICAL_MSG(_f); while (1); }
#else
#define GFX_ASSERT(_bool, _f)                 while (0)
#endif // #ifdef SCL_DEBUG

#include "debug_impl.h"

// Debug Logs, level form low(INFO) to high(FATAL, always show)
// Function information, ex function entry
#define GFX_INFO(x, args...) UBOOT_INFO(x, ##args)
//  Need debug, illegal paramter.
#define GFX_DBUG(x, args...) UBOOT_DEBUG(x, ##args)
// Error, function will be terminated but system not crash
#define GFX_ERR(x, args...) UBOOT_ERROR(x, ##args)
// Critical, system crash. (ex. assert)
#define GFX_FATAL(x, args...) UBOOT_ERROR(x, ##args)

typedef struct
{
    GFX_Set_Colorkey*           pColorKey;              //////////ColorKey
    GFX_Set_ROP*                pROP;                   //////////ROP
}GFX_Function;

typedef struct _GFX_SetConfig
{
    MS_BOOL                     bOnePixelMode;          //////////OnePixelMode
    MS_BOOL                     bDither;                //////////Dither
    MS_BOOL                     bVQEnable;              //////////Virual Cmd Queue
    MS_BOOL                     bABLEnable;             //////////Alpha Blending Enable
    GFX_RotateAngle             eRotate;                //////////Rotate
    GFX_Set_Mirror*             pMirror;                //////////Mirror
    MS_U32                      u32TagID;               //////////TagID
    GFX_Set_Clip_Property*      pClip;                  //////////Clip
}GFX_SetConfig;
#ifndef MSOS_TYPE_OPTEE
typedef struct
{
    GFX_FireInfo*               pFireInfo;
    GFX_OvalFillInfo*           pDrawOvalInfo;
}GFX_Set_DrawOvalInfo;

typedef struct
{
    GFX_FireInfo*               pFireInfo;
    MS_S32                      fHandle;
    MS_U8                       charArray[256];
    MS_U32                      u32StrWidth;
    GFX_TextOutInfo*            pTextInfo;
    GFX_FontInfo *              pFonttable;
}GFX_TextOut_V2;
#endif

void            GFXRegisterToUtopia                 (FUtopiaOpen ModuleType);
MS_U32          GFXOpen                             (void** ppInstance, const void* const pAttribute);
MS_U32          GFXClose                            (void* pInstance);
MS_U32          GFXIoctl                            (void* pInstance, MS_U32 u32Cmd, void* pArgs);
#ifdef CONFIG_GFX_UTOPIA10
MS_U16 Ioctl_GFX_Init(void* pInstance, void* pArgs);
MS_U16 Ioctl_GFX_GetCaps(void* pInstance, void* pArgs);
MS_U16 Ioctl_GFX_GetInfo(void* pInstance, void* pArgs);
MS_S32 Ioctl_GFX_LineDraw(void* pInstance, void* pArgs);
MS_S32 Ioctl_GFX_RectFill(void* pInstance, void* pArgs);
MS_S32 Ioctl_GFX_TriFill(void* pInstance, void* pArgs);
MS_S32 Ioctl_GFX_SpanFill(void* pInstance, void* pArgs);
MS_S32 Ioctl_GFX_BitBlt(void* pInstance, void* pArgs);
MS_S32 Ioctl_GFX_TwoSrcBitBlt(void* pInstance, void* pArgs);
MS_S32 Ioctl_GFX_SetABL(void* pInstance, void* pArgs);
MS_S32 Ioctl_GFX_SetConfig(void* pInstance, void* pArgs);
MS_U16 Ioctl_GFX_GetConfig(void* pInstance, void* pArgs);
MS_S32 Ioctl_GFX_Bitmap(void* pInstance, void* pArgs);
MS_S32 Ioctl_GFX_Font(void* pInstance, void* pArgs);
MS_S32 Ioctl_GFX_PowerState(void* pInstance, void* pArgs);
MS_S32 Ioctl_GFX_CB(void* pInstance, void* pArgs);
MS_S32 Ioctl_GFX_MISC(void* pInstance, void* pArgs);
MS_S32 Ioctl_GFX_TLB(void* pInstance, void* pArgs);
MS_S32 Ioctl_GFX_DrawOval(void* pInstance, void* pArgs);
#endif

GFX_Result _MApi_GE_GetCRC(MS_U32* pu32CRCvalue);
GFX_Result _MApi_GE_ConfigCRC(EN_GE_ConfigCRC enConfigCRC , void* pArgs);

#ifdef CONFIG_UTOPIA_PROC_DBG_SUPPORT
MS_U32          GFXMdbIoctl                         (MS_U32 u32Cmd, const void* const pArgs);
#endif

#ifdef __cplusplus
}
#endif
#endif
