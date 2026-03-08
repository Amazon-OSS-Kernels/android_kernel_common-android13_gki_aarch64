/* SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause) */
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

///////////////////////////////////////////////////////////////////////////////////////////////////
///
/// @file   drvGE.h
/// @brief  GE Driver Interface
/// @author MStar Semiconductor Inc.
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _DRV_PRI_GE_H_
#define _DRV_PRI_GE_H_

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct
{
    GE_BlendOp                  eBldCoef;
    GE_AlphaSrc                 eABLSrc;
    MS_U32                      u32ABLConstCoef;
}GE_ABLINFO;

typedef struct
{
    MS_U8  u8X0;       ///< Position of the left edge of the bounding box
    MS_U8  u8Width;    ///< Width of the bounding box
    MS_U8  u8Y0;       ///< Position of the top edge of the bounding box
    MS_U8  u8Height;   ///< Height of the bounding box
} GE_GlyphBbox;

typedef struct
{
    MS_PHY addr;       // flat address of whole memory map
    MS_U32 len;
    MS_BOOL inUsed;
    MS_U32 width;
    MS_U32 pitch;
    MS_U32 height;
    GE_BufFmt fmt;
} GE_BitmapInfo;

typedef struct
{
    /// font start address
    MS_PHY addr;
    /// font length
    MS_U32 len;
    /// font structure is used
    MS_BOOL inUsed;
    /// character width (pixel unit)
    MS_U32 width;
    /// character pitch (byte unit)
    MS_U32 pitch;
    /// character height (pixel unit)
    MS_U32 height;
    /// one character length (byte unit)
    MS_U32 offset;
    /// Bounding Box offset
    GE_GlyphBbox* pBBox;
    /// font format
    GE_BufFmt fmt;
} GE_FontInfo;

typedef struct __attribute__((packed))
{
    GE_CTX_HAL_SHARED           halSharedCtx;
    MS_U32                      u32GEClientAllocator;
    MS_U32                      u32LstGEClientId;//for indicating context switch
    MS_U32                      u32LstGEPaletteClientId;
    MS_BOOL                     bNotFirstInit;
    MS_U32                      u32HWSemaphoreCnt;
    MS_U32                      u32VCmdQSize;                       /// MIN:4K, MAX:512K, <MIN:Disable
    MS_PHY                      PhyVCmdQAddr;                       //  8-byte aligned
    MS_BOOL                     bIsHK;                              /// Running as HK or Co-processor
    MS_BOOL                     bIsCompt;                           /// Running as compatible mode. In compatible mode, the parameter checking loose for NOS APP compatibility.  TRUE=>compatible with 51/Chakra , FALSE=>linux OS style
    MS_BOOL                     bInit;

}GE_CTX_SHARED;

typedef struct
{
    GE_Context                  ctxHeader;
    GE_CTX_SHARED*              pSharedCtx;//pointer to shared context paramemetrs
    GE_CTX_HAL_LOCAL            halLocalCtx;
    MS_U16                      u16ClipL;
    MS_U16                      u16ClipR;
    MS_U16                      u16ClipT;
    MS_U16                      u16ClipB;
#if GE_LOCK_SUPPORT
    MS_S32                      s32GEMutex;
    MS_S32                      s32GELock;
#endif
    MS_S32                      s32GE_Recursive_Lock_Cnt; //recursive lock number when bGELock is off
    MS_U32                      u32GEClientId;
    MS_U32                      u32GESEMID;
    MS_U16                      u16GEPrevSEMID; //backup previous Sem ID for palette restore.
    MS_BOOL                     bIsComp;
    MS_U32                      u32CTXInitMagic;
    MS_BOOL                     bSrcPaletteOn;
    MS_BOOL                     bDstPaletteOn;
    MS_PHY                      PhySrcAddr;
    MS_PHY                      PhyDstAddr;
    MS_U16                      u16VcmdqAddr_L;
    MS_U16                      u16VcmdqAddr_H;
    MS_U16                      u16VcmdqAddr_MSB;
    MS_U16                      u16VcmdqMiuMsb;
    MS_U16                      u16VcmdqSize;
    MS_U16                      u16RegCfg;
}GE_CTX_LOCAL;

typedef void (*FPGFX_CBGetFont)(MS_S8, GE_FontInfo*);
typedef void (*FPGFX_CBGetBMP)(MS_S16, GE_BitmapInfo*);

typedef struct
{
    MS_U32                  u32dbglvl;
    FPGFX_CBGetBMP          fpGetBMP;
    FPGFX_CBGetFont         fpGetFont;
   // GFX_BlendCoef           _blendcoef;
    MS_U8                   _angle;
    MS_BOOL                 _bNearest;
    MS_BOOL                 _bPatchMode;
    MS_BOOL                 _bMirrorH;
    MS_BOOL                 _bMirrorV;
    MS_BOOL                 _bDstMirrorH;
    MS_BOOL                 _bDstMirrorV;
    MS_BOOL                 _bItalic;
    MS_BOOL                 _line_enable;
    MS_U8                   _line_pattern;
    MS_U8                   _line_factor;
    MS_BOOL                 bDither;

#ifdef DBGLOG
    //debug use only
    MS_BOOL _bOutFileLog;
    MS_U16  *_pu16OutLogAddr;
    MS_U16  _u16LogCount;
#endif
    GE_Context*             g_pGEContext ;
    GE_CHIP_PROPERTY *      pGeChipProperty;
    GE_ABLINFO              pABLInfo;
    MS_U32                  u32LockStatus;
    MS_BOOL                 _bInit;
    MS_U32                  u32geRgbColor;
} GFX_API_LOCAL;


typedef struct _GFX_Resource_PRIVATE
{

}GFX_Resource_PRIVATE;

typedef struct _GFX_INSTANT_PRIVATE
{
    /*Resource*/
    GFX_Resource_PRIVATE*       pResource;

    GFX_API_LOCAL               GFXPrivate_g_apiGFXLocal;
    GE_CTX_LOCAL                GFXPrivate_g_drvGECtxLocal;

    //GFX_BitBltInfo*             pGFXBufferInfo;
    //GFX_Function*               pGFXFunction;
    //GFX_SetConfig*              pGFXSetConfig;
}GFX_INSTANT_PRIVATE;

// Debug Log
#include "debug_impl.h"

// Debug Logs, level form low(INFO) to high(FATAL, always show)
// Function information, ex function entry
#define GE_D_INFO(x, args...) UBOOT_INFO(x, ##args)
//  Need debug, illegal paramter.
#define GE_D_DBUG(x, args...) UBOOT_DEBUG(x, ##args)
// Error, function will be terminated but system not crash
#define GE_D_ERR(x, args...) UBOOT_ERROR(x, ##args)
// Critical, system crash. (ex. assert)
#define GE_D_FATAL(x, args...) UBOOT_ERROR(x, ##args)

#ifdef __cplusplus
}
#endif

#endif

