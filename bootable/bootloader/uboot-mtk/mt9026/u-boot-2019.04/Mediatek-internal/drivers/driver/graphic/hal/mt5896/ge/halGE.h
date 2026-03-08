/* SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause) */
/*
 * Copyright (c) 2023 MediaTek Inc.
*/

#ifndef _HAL_GE_H_
#define _HAL_GE_H_


//-------------------------------------------------------------------------------------------------
//  Macro and Define
//-------------------------------------------------------------------------------------------------
//---------------- AC_FULL/MSK ----------------
//  B3[31:24] | B2[23:16] | B1[15:8] | B0[7:0]
//  W32[31:16] | W21[23:8] | W10 [15:0]
//  DW[31:0]
//---------------------------------------------

#define AC_FULLB0           1
#define AC_FULLB1           2
#define AC_FULLB2           3
#define AC_FULLB3           4
#define AC_FULLW10          5
#define AC_FULLW21          6
#define AC_FULLW32          7
#define AC_FULLDW           8
#define AC_MSKB0            9
#define AC_MSKB1            10
#define AC_MSKB2            11
#define AC_MSKB3            12
#define AC_MSKW10           13
#define AC_MSKW21           14
#define AC_MSKW32           15
#define AC_MSKDW            16


//-------------------- Fld --------------------
//    wid[31:16] | shift[15:8] | ac[7:0]
//---------------------------------------------

#define Fld(wid, shft, ac)    (((uint32_t)wid<<16)|(shft<<8)|ac)
#define Fld_wid(fld)        (uint8_t)((fld)>>16)
#define Fld_shft(fld)       (uint8_t)((fld)>>8)
#define Fld_ac(fld)         (uint8_t)((fld))

#define bNeedSetActiveCtrlMiu1      TRUE
#define GE_THRESHOLD_SETTING        0xDUL
#define GE_WordUnit                 32UL
#define GE_MAX_MIU                  2UL
#define GE_TABLE_REGNUM             0x80UL

#define GE_FMT_I1                 0x0UL
#define GE_FMT_I2                 0x1UL
#define GE_FMT_I4                 0x2UL
#define GE_FMT_I8                 0x4UL
#define GE_FMT_FaBaFgBg2266       0x6UL
#define GE_FMT_1ABFgBg12355       0x7UL
#define GE_FMT_RGB565             0x8UL
#define GE_FMT_ARGB1555           0x9UL
#define GE_FMT_ARGB4444           0xAUL
#define GE_FMT_ARGB1555_DST       0xCUL
#define GE_FMT_YUV422             0xEUL
#define GE_FMT_ARGB8888           0xFUL
#define GE_FMT_RGBA5551           0x10UL
#define GE_FMT_ABGR1555           0x11UL
#define GE_FMT_BGRA5551           0x12UL
#define GE_FMT_RGBA4444           0x13UL
#define GE_FMT_ABGR4444           0x14UL
#define GE_FMT_BGRA4444           0x15UL
#define GE_FMT_BGR565             0x16UL
#define GE_FMT_RGBA8888           0x1DUL
#define GE_FMT_ABGR8888           0x1EUL
#define GE_FMT_BGRA8888           0x1FUL
#define GE_FMT_GENERIC            0xFFFFUL

#define GE_VQ_4K                  0x0UL
#define GE_VQ_8K                  0x0UL
#define GE_VQ_16K                 0x1UL
#define GE_VQ_32K                 0x2UL
#define GE_VQ_64K                 0x3UL
#define GE_VQ_128K                0x4UL
#define GE_VQ_256K                0x5UL
#define GE_VQ_512K                0x6UL
#define GE_VQ_1024K               0x7UL
#define GE_VQ_2048K               0x7UL

#ifndef UNUSED
#define UNUSED(var)             (void)(var)
#endif

#ifndef GE_UNUSED
#define GE_UNUSED(x)    UNUSED_ ## x __attribute__((__unused__))
#endif


#define __USE_GE_POLLING_MODE 0
#define __USE_GE_INT_MODE 1  // Kernel must enable CONFIG_MP_PLATFORM_UTOPIA2_INTERRUPT

#ifdef CONFIG_GFX_TAG_INTERRUPT

#if defined(MSOS_TYPE_LINUX) || defined(MSOS_TYPE_LINUX_KERNEL)
#define __GE_WAIT_TAG_MODE __USE_GE_INT_MODE
#else
#define __GE_WAIT_TAG_MODE __USE_GE_POLLING_MODE
#endif

#else

#define __GE_WAIT_TAG_MODE __USE_GE_POLLING_MODE

#endif
//feature
#define GE_USE_HW_SEM (0)

//Patch
#define GE_PITCH_256_ALIGNED_UNDER_4P_MODE (1)// For Curry/Kano, pitch must 256 algined under 4P mode(Hw Cap, Not a bug)
#define COLOR_CONVERT_PATCH (1)//When dst fmt is yuv422, GE must enalbe 1P mode & convert clr type. Only for M7322

#define GE_SCALING_MULITPLIER   (0x1000)

// Debug Log
#include "debug_impl.h"

// Debug Logs, level form low(INFO) to high(FATAL, always show)
// Function information, ex function entry
#define GE_H_INFO(x, args...) UBOOT_INFO(x, ##args)
//  Need debug, illegal paramter.
#define GE_H_DBUG(x, args...) UBOOT_INFO(x, ##args)
// Error, function will be terminated but system not crash
#define GE_H_ERR(x, args...) UBOOT_ERROR(x, ##args)
//-------------------------------------------------------------------------------------------------
//  Type and Structure
//-------------------------------------------------------------------------------------------------
typedef struct _GE_ColorDelta
{
    MS_U32                          r; // s7.12
    MS_U32                          g;
    MS_U32                          b;
    MS_U16                          a; // s4.11
} GE_ColorDelta;

/*the following is for parameters for shared between multiple process context*/
typedef struct __attribute__((packed))
{
   MS_BOOL bGE_DirectToReg;
   MS_U16 global_tagID;
   MS_U16 u16ShareRegImage[GE_TABLE_REGNUM];
   MS_U16 u16ShareRegImageEx[GE_TABLE_REGNUM];
}GE_CTX_HAL_SHARED;

/*the following is for parameters for used in local process context*/
typedef struct
{
   GE_CTX_HAL_SHARED *pHALShared;
   GE_CHIP_PROPERTY  *pGeChipPro;
   MS_VIRT           va_mmio_base;
   MS_VIRT           va_mmio_base2;
   MS_U16            u16RegGETable[GE_TABLE_REGNUM];                 //Store for GE RegInfo
   MS_U16            u16RegGETableEX[GE_TABLE_REGNUM];                 //Store for GE RegInfo
   MS_BOOL           bIsComp;
   MS_BOOL           bPaletteDirty;
   MS_U32            u32Palette[GE_PALETTE_NUM];
   MS_BOOL           bYScalingPatch;
}GE_CTX_HAL_LOCAL;

//-------------------------------------------------------------------------------------------------
//  Function and Variable
//-------------------------------------------------------------------------------------------------
void        GE_Chip_Proprity_Init(GE_CTX_HAL_LOCAL *pGEHalLocal);
void        GE_ResetState(GE_CTX_HAL_LOCAL *pGEHalLocal);
void        GE_WaitIdle(GE_CTX_HAL_LOCAL *pGEHalLocal);
GE_Result   GE_Map_Share_Reg(GE_CTX_HAL_LOCAL *pGEHalLocal, MS_U16 addr);
void        GE_WriteReg(GE_CTX_HAL_LOCAL *pGEHalLocal, MS_U16 addr, MS_U16 value);
MS_U16      GE_ReadReg(GE_CTX_HAL_LOCAL *pGEHalLocal, MS_U16 addr);
void        GE2_WriteReg(GE_CTX_HAL_LOCAL *pGEHalLocal, MS_U16 addr, MS_U16 value);
MS_U16      GE2_ReadReg(GE_CTX_HAL_LOCAL *pGEHalLocal, MS_U16 addr);
GE_Result   GE_InitCtxHalPalette(GE_CTX_HAL_LOCAL *pGEHalLocal);
void        GE_Init_HAL_Context(GE_CTX_HAL_LOCAL *pGEHalLocal, GE_CTX_HAL_SHARED *pHALShared, MS_BOOL bNeedInitShared);
void        GE_Init_RegImage(GE_CTX_HAL_LOCAL *pGEHalLocal);
void        GE_Init(GE_CTX_HAL_LOCAL *pGEHalLocal, GE_Config *cfg);
GE_Result   GE_SetRotate(GE_CTX_HAL_LOCAL *pGEHalLocal, GE_RotateAngle geRotAngle);
GE_Result   GE_SetOnePixelMode(GE_CTX_HAL_LOCAL *pGEHalLocal, MS_BOOL enable);
GE_Result   GE_SetBlend(GE_CTX_HAL_LOCAL *pGEHalLocal, GE_BlendOp eBlendOp);
GE_Result   GE_SetAlpha(GE_CTX_HAL_LOCAL *pGEHalLocal, GE_AlphaSrc eAlphaSrc);
GE_Result   GE_QueryDFBBldCaps(GE_CTX_HAL_LOCAL *pGEHalLocal, MS_U16 *pU16SupportedBldFlags);
GE_Result   GE_EnableDFBBld(GE_CTX_HAL_LOCAL *pGEHalLocal, MS_BOOL enable);
GE_Result   GE_SetDFBBldFlags(GE_CTX_HAL_LOCAL *pGEHalLocal, MS_U16 u16DFBBldFlags);
GE_Result   GE_SetDFBBldOP(GE_CTX_HAL_LOCAL *pGEHalLocal, GE_DFBBldOP geSrcBldOP, GE_DFBBldOP geDstBldOP);
GE_Result   GE_SetDFBBldConstColor(GE_CTX_HAL_LOCAL *pGEHalLocal, GE_RgbColor geRgbColor);
GE_Result   GE_SetDFBBldSrcColorMask(GE_CTX_HAL_LOCAL *pGEHalLocal, GE_RgbColor geRgbColor);
GE_Result   GE_WriteProtect(GE_CTX_HAL_LOCAL *pGEHalLocal, MS_U8 miu, MS_PHY addr_low, MS_PHY addr_high, GE_WPType eWPType);
GE_Result   GE_SetSrcTile(GE_CTX_HAL_LOCAL *pGEHalLocal, MS_BOOL tile);
GE_Result   GE_SetDstTile(GE_CTX_HAL_LOCAL *pGEHalLocal, MS_BOOL tile);
GE_Result   GE_GetFmtCaps(GE_CTX_HAL_LOCAL *pGEHalLocal, GE_BufFmt fmt, GE_BufType type, GE_FmtCaps *caps);
GE_Result   GE_Set_IOMap_Base(GE_CTX_HAL_LOCAL *pGEHalLocal, MS_VIRT addr);
GE_Result   GE_Set_IOMap_Base2(GE_CTX_HAL_LOCAL *pGEHalLocal, MS_VIRT addr);
//GE_Result   GE_Get_BackupRegArray(GE_CTX_HAL_LOCAL *pGEHalLocal, MS_U32* addr);
//void           GE_RestoreReg(GE_CTX_HAL_LOCAL *pGEHalLocal, MS_U16 addr, MS_U16 value);
//MS_U16      GE_BackupReg(GE_CTX_HAL_LOCAL *pGEHalLocal, MS_U16 addr);
MS_U16      GE_GetNextTAGID(GE_CTX_HAL_LOCAL *pGEHalLocal, MS_BOOL bStepTagBefore);
GE_Result   GE_WaitTAGID(GE_CTX_HAL_LOCAL *pGEHalLocal, MS_U16 tagID);
GE_Result   GE_Restore_HAL_Context(GE_CTX_HAL_LOCAL *pGEHalLocal,MS_BOOL bNotFirstInit);
GE_Result   GE_ClrInvalMIUFlg(GE_CTX_HAL_LOCAL *pGEHalLocal,MS_BOOL enable);
GE_Result   GE_EnableDynaClkGate(GE_CTX_HAL_LOCAL *pGEHalLocal,MS_BOOL enable);
GE_Result   GE_CalcBltScaleRatio(GE_CTX_HAL_LOCAL *pGEHalLocal, MS_U16 u16SrcWidth, MS_U16 u16SrcHeight, MS_U16 u16DstWidth, MS_U16 u16DstHeight, GE_ScaleInfo *pScaleinfo);
GE_Result   GE_SetBltScaleRatio(GE_CTX_HAL_LOCAL *pGEHalLocal,GE_Rect *src, GE_DstBitBltType *dst, GE_Flag flags, GE_ScaleInfo* scaleinfo);
GE_Result   GE_BitBltEX_Trape(GE_CTX_HAL_LOCAL *pGEHalLocal, GE_Rect *pSrcRect, GE_Normalized_Trapezoid *pGENormTrapezoid, MS_U32 u32Flags, GE_ScaleInfo* pScaleinfo);
GE_Result   GE_FillTrapezoid(GE_CTX_HAL_LOCAL *pGEHalLocal, MS_BOOL bYTrapezoid, GE_Normalized_Trapezoid *pGENormTrapezoid, MS_U32 u32Color, GE_ColorDelta *pColorDeltaX, GE_ColorDelta *pColorDeltaY);
GE_Result   GE_EnableTrapezoidAA(GE_CTX_HAL_LOCAL *pGEHalLocal, MS_BOOL bEnable);
GE_Result   GE_EnableTrapSubPixCorr(GE_CTX_HAL_LOCAL *pGEHalLocal, MS_BOOL bEnable);
GE_Result   GE_SetDisaMIUAccess(GE_CTX_HAL_LOCAL *pGEHalLocal,MS_BOOL enable);
GE_Result   GE_SetASCK(GE_CTX_HAL_LOCAL *pGEHalLocal, MS_BOOL enable);
GE_Result   GE_SetDSCK(GE_CTX_HAL_LOCAL *pGEHalLocal, MS_BOOL enable);
GE_Result   GE_SetVCmdBuffer(GE_CTX_HAL_LOCAL *pGEHalLocal, MS_PHY PhyAddr, GE_VcmqBufSize enBufSize);
MS_PHY    GE_ConvertAPIAddr2HAL(GE_CTX_HAL_LOCAL *pGEHalLocal, MS_U8 u8MIUId, MS_PHY PhyGE_APIAddrInMIU);
void        GE_SetActiveCtrlMiu1(GE_CTX_HAL_LOCAL *pGEHalLocal);
GE_Result   GE_SetClock(GE_CTX_HAL_LOCAL *pGEHalLocal, MS_BOOL bOnOff);
MS_BOOL     GE_NonOnePixelModeCaps(GE_CTX_HAL_LOCAL *pGEHalLocal, PatchBitBltInfo* patchInfo);
GE_Result   HAL_GE_EnableCalcSrc_WidthHeight(GE_CTX_HAL_LOCAL *pGEHalLocal, MS_BOOL bEnable);
GE_Result   GEWD_ReadReg(GE_CTX_HAL_LOCAL *pGEHalLocal, MS_U16 addr, MS_U16* value);
GE_Result   GEWD_WriteReg(GE_CTX_HAL_LOCAL *pGEHalLocal, MS_U16 addr, MS_U16 value);
GE_Result   GE_SetTLBMode(GE_CTX_HAL_LOCAL *pstGEHalLocal, GE_TLB_Mode enTlb_type);
GE_Result   GE_GetTLBSRCADDR(GE_CTX_HAL_LOCAL *pstGEHalLocal, MS_PHY* phyAddr);
GE_Result   GE_GetTLBDSTADDR(GE_CTX_HAL_LOCAL *pstGEHalLocal, MS_PHY* phyAddr);
GE_Result   GE_SetTLBSrcBaseAddr(GE_CTX_HAL_LOCAL *pstGEHalLocal, MS_PHY phyAddr);
GE_Result   GE_SetTLBDstBaseAddr(GE_CTX_HAL_LOCAL *pstGEHalLocal, MS_PHY phyAddr);
GE_Result   GE_FlushTLBTable(GE_CTX_HAL_LOCAL *pstGEHalLocal, MS_BOOL bEnable);
GE_Result   GE_SetTLBTag(GE_CTX_HAL_LOCAL *pstGEHalLocal, MS_U16 u16Tag);
GE_Result   GE_StopFlushTLB(GE_CTX_HAL_LOCAL *pstGEHalLocal);
GE_Result   GE_Get_MIU_INTERVAL(GE_CTX_HAL_LOCAL *pGEHalLocal, MS_U8 miu, MS_PHY* value);
GE_Result   HAL_GE_AdjustDstWin( GE_CTX_HAL_LOCAL *pGEHalLocal, MS_BOOL bDstXInv );
GE_Result   HAL_GE_AdjustRotateDstWin(GE_CTX_HAL_LOCAL *pGEHalLocal, MS_U8 u8Rotate);
#if defined(CONFIG_GFX_TAG_INTERRUPT) && (__GE_WAIT_TAG_MODE == __USE_GE_INT_MODE)
GE_Result   HAL_GE_exit(GE_CTX_HAL_LOCAL *pGEHalLocal);
#endif
GE_Result   HAL_GE_SetBurstMiuLen(GE_CTX_HAL_LOCAL *pGEHalLocal,MS_BOOL bEnable,MS_U32 u32BurstLen);
GE_Result   HAL_GE_SetBufferAddr(GE_CTX_HAL_LOCAL *pGEHalLocal,MS_PHY PhyAddr,GE_BufType enBuffType);
GE_Result   HAL_GE_GetCRC(GE_CTX_HAL_LOCAL *pGEHalLocal,MS_U32* pu32CRCvalue);
GE_Result   HAL_GE_ConfigCRC(GE_CTX_HAL_LOCAL *pGEHalLocal,ST_DRV_GE_ConfigCRC *pstConfigCRC);
#if defined(MSOS_TYPE_LINUX_KERNEL)
GE_Result HAL_GE_STR_RestoreReg(GE_CTX_HAL_LOCAL *pGEHalLocal, GE_STR_SAVE_AREA *pGFX_STRPrivate);
#endif
#if defined(CONFIG_SUPPORT_MULTI_GATE)
GE_Result MHal_GE_SetGClk(GE_CTX_HAL_LOCAL *pGEHalLocal, MS_BOOL bEnClk);
#endif
#define      GE_SetSrcBufMIUId(pGEHalLocal, u8MIUId)  do{}while(0)
#define      GE_SetDstBufMIUId(pGEHalLocal, u8MIUId)  do{}while(0)
#define      GE_SetVQBufMIUId(pGEHalLocal, u8MIUId)  do{}while(0)
#define      GE_GetSrcBufMIUId(pGEHalLocal, u32GE_HALAddr)     ((MS_U8)(((u32GE_HALAddr)&(1UL<<31))>>31))
#define      GE_GetDstBufMIUId(pGEHalLocal, u32GE_HALAddr)     ((MS_U8)(((u32GE_HALAddr)&(1UL<<31))>>31))
void GE_CalcColorDelta(MS_U32 color0, MS_U32 color1, MS_U16 ratio, GE_ColorDelta *delta);
GE_Result GE_SetPalette(GE_CTX_HAL_LOCAL *pGEHalLocal);
GE_Result HAL_GE_SetVCmd_W_Thread(GE_CTX_HAL_LOCAL *pGEHalLocal, MS_U8 u8W_Threshold);
GE_Result HAL_GE_SetVCmd_R_Thread(GE_CTX_HAL_LOCAL *pGEHalLocal, MS_U8 u8R_Threshold);
GE_Result HAL_GE_SetYUVMode(GE_CTX_HAL_LOCAL *pGEHalLocal, GE_YUVMode *mode);
GE_Result HAL_GE_SetClipWindow(GE_CTX_HAL_LOCAL *pGEHalLocal, GE_Rect *rect);
GE_Result HAL_GE_SetBuffer(GE_CTX_HAL_LOCAL *pGEHalLocal, MS_U16 u16Pitch, MS_U16 u16Fmt,GE_BufType enBuffType);
MS_U16 HAL_GE_GetFmt(GE_CTX_HAL_LOCAL *pGEHalLocal);
GE_Result HAL_GE_SetDither(GE_CTX_HAL_LOCAL *pGEHalLocal, MS_BOOL enable);
GE_Result HAL_GE_BitBltEX(GE_CTX_HAL_LOCAL *pGEHalLocal, GE_Rect *src, GE_DstBitBltType *dst, MS_U32 flags, GE_ScaleInfo* scaleinfo);
GE_Result HAL_GE_FillRect(GE_CTX_HAL_LOCAL *pGEHalLocal, GE_Rect *rect, MS_U32 color, MS_U32 color2, MS_U32 flags);
#endif // _HAL_GE_H_
