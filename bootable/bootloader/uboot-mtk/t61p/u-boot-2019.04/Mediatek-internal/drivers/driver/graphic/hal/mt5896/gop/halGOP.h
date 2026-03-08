/* SPDX-License-Identifier: (GPL-2.0 OR BSD-3-Clause) */
/*
 * Copyright (c) 2020 MediaTek Inc.
 */

#ifndef _HAL_GOP_H_
#define _HAL_GOP_H_

#include "drvGOP.h"
#include "regGOP.h"
//-------------------------------------------------------------------------------------------------
//  Macro and Define
//-------------------------------------------------------------------------------------------------
#define GOP_STRADDR_WORD_UNIT	(0x20)

#define Gop23_GwinCtl_Ofet                      0UL

#define MAX_GOP_MIUCOUNT                        2UL
#define MAX_GOP_MIUSEL                          MAX_GOP_MIUCOUNT-1
#define MAX_GOP_MUX                             5UL
#define MAX_GOP_MUX_SEL                         5UL
#define MAX_GOP_MUX_OPNum                       MAX_GOP_MUX
#define MAX_GOP_DualMUX_Num                     3UL
#define MAX_GOP0_GWIN                           1UL
#define MAX_GOP1_GWIN                           1UL
#define MAX_GOP2_GWIN                           1UL
#define MAX_GOP3_GWIN                           1UL
#define MAX_GOP4_GWIN                           1UL
#define MAX_GOP5_GWIN                           1UL

#define GOP0_Gwin0Id                            0UL
#define GOP0_Gwin1Id                            0UL
#define GOP1_Gwin0Id                            1UL
#define GOP1_Gwin1Id                            1UL
#define GOP2_Gwin0Id                            2UL
#define GOP3_Gwin0Id                            3UL
#define GOP4_Gwin0Id                            4UL
#define GOP5_Gwin0Id                            5UL

#define GOP0_REG_FORM                           ((E_GOP_REG_FORM_NONE) + (E_GOP_PAL_SIZE_NONE))
#define GOP1_REG_FORM                           ((E_GOP_REG_FORM_NONE) + (E_GOP_PAL_SIZE_256))
#define GOP2_REG_FORM                           E_GOP_REG_FORM_T81G + E_GOP_PAL_SIZE_NONE
#define GOP3_REG_FORM                           E_GOP_REG_FORM_T81G + E_GOP_PAL_SIZE_NONE
#define GOP4_REG_FORM                           E_GOP_REG_FORM_NONE
#define GOP5_REG_FORM                           E_GOP_REG_FORM_NONE
#define GOPD_REG_FORM                           E_GOPD_FIFO_DEPTH_64


#define GOP0_GwinIdBase                         GOP0_Gwin0Id
#define GOP1_GwinIdBase                         MAX_GOP0_GWIN
#define GOP2_GwinIdBase                         MAX_GOP0_GWIN + MAX_GOP1_GWIN
#define GOP3_GwinIdBase                         MAX_GOP0_GWIN + MAX_GOP1_GWIN + MAX_GOP2_GWIN
#define GOP4_GwinIdBase                         MAX_GOP0_GWIN + MAX_GOP1_GWIN + MAX_GOP2_GWIN + MAX_GOP3_GWIN
#define GOP5_GwinIdBase                         MAX_GOP0_GWIN + MAX_GOP1_GWIN + MAX_GOP2_GWIN + MAX_GOP3_GWIN + MAX_GOP4_GWIN

#define GOP_MIXER_MUX                           6UL

#define GOP_BIT0    0x01
#define GOP_BIT1    0x02
#define GOP_BIT2    0x04
#define GOP_BIT3    0x08
#define GOP_BIT4    0x10
#define GOP_BIT5    0x20
#define GOP_BIT6    0x40
#define GOP_BIT7    0x80
#define GOP_BIT8    0x0100
#define GOP_BIT9    0x0200
#define GOP_BIT10   0x0400
#define GOP_BIT11   0x0800
#define GOP_BIT12   0x1000
#define GOP_BIT13   0x2000
#define GOP_BIT14   0x4000
#define GOP_BIT15   0x8000

#define GOP_REG_WORD_MASK                       0xFFFFUL

#define RATIO_20BITS                            0x100000UL  //2^20

#define SCALING_MULITPLIER                      RATIO_20BITS
#define HVSP2_RATIO_DEFAULT	(2)
#define GOP_WordUnit                            32
#define GOP_TotalGwinNum                        (MAX_GOP0_GWIN+MAX_GOP1_GWIN+MAX_GOP2_GWIN+MAX_GOP3_GWIN+MAX_GOP4_GWIN+MAX_GOP5_GWIN)
#define HAL_GOP_BankOffset(pGOPHalLocal)        ((pGOPHalLocal)->bank_offset)

#ifndef GOP_MIU0_LENGTH
#define GOP_MIU0_LENGTH                         HAL_MIU1_BASE
#endif

#define GOP_PD_T3D                          (0x0UL)
#define GOP_PD_NORMAL                        (0x0UL)

#define GOP_PUBLIC_UPDATE (SHARED_GOP_MAX_COUNT-1)

#define GFLIP_REG_BANKS (26)

#define GFLIP_REG16_NUM_PER_BANK                128UL

#ifndef GOP_UNUSED
#define GOP_UNUSED(x)    UNUSED_ ## x __attribute__((__unused__))
#endif

#define GOP_CSC_ELEMENT_NUM (10)

#define GOP_HSTRETCH_4TAP_TABLE_NUM              (8UL)
#define GOP_HSTRETCH_4TAP_COEF0_00               (0UL)
#define GOP_HSTRETCH_4TAP_COEF0_25               (2UL)
#define GOP_HSTRETCH_4TAP_COEF3_25               (4UL)
#define GOP_HSTRETCH_4TAP_COEF0_50               (6UL)

#define GOP_STRETCH_4TAP_C_SRAM_TABLE_EN           (2UL)
#define GOP_STRETCH_4TAP_PROCESS_IDX               (2UL)

#define GOP_PQGAMMA_OSDB_LD     (0x0)
#define GOP_PQGAMMA_LD_OSDB     (0x1)
#define GOP_OSDB_PQGAMMA_LD     (0x2)

#define GOP_VSCALE_V4_LIMIT	(1280)

/*the following is for parameters for shared between multiple process context*/
typedef struct __attribute__((packed))
{
    GOP_CHIP_PROPERTY       gopChipProperty;
    DRV_GOPDstType GOP_Dst[SHARED_GOP_MAX_COUNT];
}GOP_CTX_HAL_SHARED;

/*the following is for parameters for used in local process context*/
typedef struct
{
    GOP_CTX_HAL_SHARED      *pHALShared;
    MS_VIRT                 va_mmio_base;
    MS_U32                  bank_offset;
    MS_U16                  u16Clk0Setting; ///Backup Current GOPG clock setting
    MS_U16                  u16Clk1Setting; ///Backup Current GOPD clock setting
    MS_U16                  u16Clk2Setting; ///Backup Current SRAM clock setting
    DRV_GOPDstType          drvGFlipGOPDst[SHARED_GOP_MAX_COUNT];
    GOP_CHIP_PROPERTY       *pGopChipPro;
    DRV_GOP_CONSALPHA_BITS  User_ConsAlpha_bits;

    /*check all gop dst is valid or not for each mux*/
    MS_BOOL                 *pbIsMuxVaildToGopDst;
}GOP_CTX_HAL_LOCAL;

typedef struct
{
    GOP_CTX_HAL_LOCAL GOPHalSTRCtx;
    MS_U16 BankReg[GFLIP_REG_BANKS][GFLIP_REG16_NUM_PER_BANK];
    MS_U16 CKG_GopReg[10];
    MS_U16 GS_GopReg[3];
    MS_U16 XC_GopReg[20];
}GFLIP_REGS_SAVE_AREA;

//-------------------------------------------------------------------------------------------------
//  Type and Structure
//-------------------------------------------------------------------------------------------------
typedef enum
{
    E_GOP0 = 0,
    E_GOP1 = 1,
    E_GOP2 = 2,
    E_GOP3 = 3,
    E_GOP4 = 4,
    E_GOP5 = 5,
}E_GOP_TYPE;

//-------------------------------------------------------------------------------------------------
//  Function and Variable
//-------------------------------------------------------------------------------------------------
MS_BOOL _GetBnkOfstByGop(MS_U8 gop, MS_U32 *pBnkOfst);
void HAL_GOP_Init(GOP_CTX_HAL_LOCAL *pGOPHalLocal, MS_U8 u8GOPNum);
void HAL_GOP_Init_Context(GOP_CTX_HAL_LOCAL *pGOPHalLocal,
                                     GOP_CTX_HAL_SHARED *pHALShared, MS_BOOL bNeedInitShared);
void HAL_GOP_Chip_Proprity_Init(GOP_CTX_HAL_LOCAL *pGOPHalLocal);
void HAL_GOP_Restore_Ctx(GOP_CTX_HAL_LOCAL *pGOPHalLocal);
void HAL_GOP_GWIN_SetBlending(GOP_CTX_HAL_LOCAL *pGOPHalLocal, MS_U8 u8GOP, MS_U8 u8win, MS_BOOL bEnable, MS_U8 u8coef);
void HAL_GOP_SetIOMapBase(GOP_CTX_HAL_LOCAL *pGOPHalLocal, MS_VIRT addr);
void HAL_GOP_SetIOFRCMapBase(GOP_CTX_HAL_LOCAL *pGOPHalLocal, MS_VIRT addr);
void HAL_GOP_SetIOPMMapBase(GOP_CTX_HAL_LOCAL *pGOPHalLocal, MS_VIRT addr);
void HAL_GOP_GWIN_GetMUX(GOP_CTX_HAL_LOCAL*pGOPHalLocal, MS_U8* u8GOPNum, Gop_MuxSel eGopMux);
void HAL_GOP_GWIN_SetMUX(GOP_CTX_HAL_LOCAL*pGOPHalLocal, MS_U8 u8GOPNum, Gop_MuxSel eGopMux);
void HAL_GOP_GetGOPEnum(GOP_CTX_HAL_LOCAL *pGOPHalLocal, GOP_TYPE_DEF* GOP_TYPE);
MS_U16 HAL_GOP_GetBPP(GOP_CTX_HAL_LOCAL *pGOPHalLocal, DRV_GOPColorType fbFmt);
GOP_Result HAL_GOP_SetGOPACKMask(GOP_CTX_HAL_LOCAL *pGOPHalLocal,MS_U16 u16GopMask);
GOP_Result HAL_GOP_SetGOPACK(GOP_CTX_HAL_LOCAL *pGOPHalLocal,MS_U8 gop);
MS_U16 HAL_GOP_GetGOPACK(GOP_CTX_HAL_LOCAL *pGOPHalLocal,MS_U8 gop);
MS_U8 HAL_GOP_GetMaxGwinNumByGOP(GOP_CTX_HAL_LOCAL *pGOPHalLocal, MS_U8 u8GopNum);
MS_U8 HAL_GOP_SelGwinIdByGOP(GOP_CTX_HAL_LOCAL *pGOPHalLocal, MS_U8 u8Gop, MS_U8 u8Idx);
void HAL_GOP_SetIPSel2SC(GOP_CTX_HAL_LOCAL *pGOPHalLocal, MS_IPSEL_GOP ipSelGop);
E_GOP_VIDEOTIMING_MIRRORTYPE HAL_GOP_GetVideoTimingMirrorType(GOP_CTX_HAL_LOCAL *pGOPHalLocal, MS_BOOL bHorizontal , EN_DRV_GOP_XC_MIRRORMODE enMirrorMode);
GOP_Result HAL_GOP_DWIN_SetSourceSel(GOP_CTX_HAL_LOCAL *pGOPHalLocal, DRV_GOP_DWIN_SRC_SEL enSrcSel);
GOP_Result HAL_GOP_DWIN_EnableR2YCSC(GOP_CTX_HAL_LOCAL *pGOPHalLocal, MS_BOOL bEnable);
GOP_Result HAL_GOP_GetGOPDst(GOP_CTX_HAL_LOCAL *pGOPHalLocal, MS_U8 u8gopNum, DRV_GOPDstType *pGopDst);
GOP_Result HAL_GOP_GOPSel(GOP_CTX_HAL_LOCAL *pGOPHalLocal, MS_U8 u8GOPNum);
GOP_Result HAL_GOP_SetGOPHighPri(GOP_CTX_HAL_LOCAL *pGOPHalLocal, MS_U8 gopNum);
GOP_Result HAL_GOP_SetGOPEnable2SC(GOP_CTX_HAL_LOCAL *pGOPHalLocal, MS_U8 gopNum, MS_BOOL bEnable);
GOP_Result HAL_GOP_SetGOPEnable2Mode1(GOP_CTX_HAL_LOCAL *pGOPHalLocal, MS_U8 gopNum, MS_BOOL bEnable);
GOP_Result HAL_GOP_GetGOPAlphaMode1(GOP_CTX_HAL_LOCAL *pGOPHalLocal, MS_U8 gopNum, MS_BOOL *pbEnable);
GOP_Result HAL_GOP_GWIN_SetDstPlane(GOP_CTX_HAL_LOCAL *pGOPHalLocal, MS_U8 GopNum, DRV_GOPDstType eDstType,MS_BOOL bOnlyCheck);
GOP_Result HAL_GOP_SetGOPClk(GOP_CTX_HAL_LOCAL *pGOPHalLocal, MS_U8 gopNum, DRV_GOPDstType eDstType);
GOP_Result HAL_GOP_SetClkForCapture(GOP_CTX_HAL_LOCAL *pGOPHalLocal, DRV_GOP_DWIN_SRC_SEL enSrcSel);
GOP_Result HAL_GOP_MIXER_EnableVfilter(GOP_CTX_HAL_LOCAL *pGOPHalLocal, MS_BOOL bEn);
GOP_Result HAL_GOP_SetClock(GOP_CTX_HAL_LOCAL *pGOPHalLocal,MS_BOOL bEnable);
GOP_Result HAL_ConvertAPIAddr(GOP_CTX_HAL_LOCAL *pGOPHalLocal, MS_U8 gwinid, MS_PHY* u64Adr);
GOP_Result HAL_GOP_MIXER_SetOutputTiming(GOP_CTX_HAL_LOCAL *pGOPHalLocal, MS_U32 u32mode, GOP_DRV_MixerTiming *pTM);
GOP_Result HAL_GOP_SetUVSwap(GOP_CTX_HAL_LOCAL *pGOPHalLocal, MS_U8 u8GOPNum,MS_BOOL bEn);
GOP_Result HAL_GOP_SetYCSwap(GOP_CTX_HAL_LOCAL *pGOPHalLocal, MS_U8 u8GOPNum,MS_BOOL bEn);
GOP_Result HAL_GOP_GWIN_GetNewAlphaMode(GOP_CTX_HAL_LOCAL *pGOPHalLocal, MS_U8 u8win, MS_BOOL* pEnable);
GOP_Result HAL_GOP_GWIN_SetNewAlphaMode(GOP_CTX_HAL_LOCAL *pGOPHalLocal, MS_U8 u8win, MS_BOOL bEnable);
GOP_Result HAL_GOP_SetGOPToVE(GOP_CTX_HAL_LOCAL *pGOPHalLocal, MS_U8 gopNum, MS_BOOL bEn );
GOP_Result HAL_GOP_OC_SetOCEn(GOP_CTX_HAL_LOCAL *pGOPHalLocal, MS_U8 u8GOP, MS_BOOL bOCEn);
GOP_Result HAL_GOP_OC_Get_MIU_Sel(GOP_CTX_HAL_LOCAL *pGOPHalLocal, MS_U8 *MIUId);
GOP_Result HAL_GOP_OC_SetOCInfo(GOP_CTX_HAL_LOCAL *pGOPHalLocal, DRV_GOP_OC_INFO* pOCinfo);
GOP_Result HAL_GOP_DWIN_SetRingBuffer(GOP_CTX_HAL_LOCAL *pGOPHalLocal, MS_U32 u32RingSize,MS_U32 u32BufSize);
GOP_Result HAL_GOP_AdjustField(GOP_CTX_HAL_LOCAL *pGOPHalLocal, MS_U8 GopNum, DRV_GOPDstType eDstType);
GOP_Result HAL_GOP_SetWinFmt(GOP_CTX_HAL_LOCAL *pGOPHalLocal, MS_U8 regForm, MS_U8 u8GOPNum, MS_U8 u8GwinNum, MS_U16 colortype);
GOP_Result HAL_GOP_Set_PINPON(GOP_CTX_HAL_LOCAL *pGOPHalLocal, MS_U8 u8GOPNum, MS_BOOL bEn, E_DRV_GOP_PINPON_MODE pinpon_mode);
GOP_Result HAL_GOP_DeleteWinHVSize(GOP_CTX_HAL_LOCAL *pGOPHalLocal,MS_U8 u8GOP, MS_U16 u16HSize, MS_U16 u16VSize);
GOP_Result HAL_GOP_DumpGOPReg(GOP_CTX_HAL_LOCAL *pGOPHalLocal, MS_U32 u32GopIdx, MS_U16 u16BankIdx, MS_U16 u16Addr, MS_U16* u16Val);
GOP_Result HAL_GOP_RestoreGOPReg(GOP_CTX_HAL_LOCAL *pGOPHalLocal, MS_U32 u32GopIdx, MS_U16 u16BankIdx, MS_U16 u16Addr, MS_U16 u16Val);
GOP_Result HAL_GOP_PowerState(GOP_CTX_HAL_LOCAL *pGOPHalLocal, MS_U32 u32PowerState, GFLIP_REGS_SAVE_AREA* pGOP_STRPrivate);
GOP_Result HAL_GOP_GWIN_SetGPUTileMode(GOP_CTX_HAL_LOCAL *pGOPHalLocal,MS_U8 gwinid, EN_DRV_GOP_GPU_TILE_MODE tile_mode);
GOP_Result MHal_GOP_SetCSCInfo(MS_U32 u32GOP,ST_GOP_CSC_TABLE *pstCSCTbl,MS_BOOL bFlag);
GOP_Result HAL_GOP_IsHDREnabled(GOP_CTX_HAL_LOCAL *pGOPHalLocal, MS_BOOL *pbHDREnable);
GOP_Result HAL_GOP_SetGOPMixerValid(GOP_CTX_HAL_LOCAL *pGOPHalLocal, MS_U8 u8Gop, MS_BOOL bEnable);
GOP_Result HAL_GOP_SetGopGwinHVPixel(GOP_CTX_HAL_LOCAL *pGOPHalLocal, MS_U8 u8GOP, MS_U8 u8win, MS_U16 hstart, MS_U16 hend, MS_U16 vstart, MS_U16 vend);
GOP_Result Hal_SetCropWindow(GOP_CTX_HAL_LOCAL *pGOPHalLocal, MS_U8 u8GOP, EN_GOP_CROP_CTL crop_mode);
GOP_Result HAL_GOP_SetDbgLevel(EN_GOP_DEBUG_LEVEL level);
GOP_Result HAL_GOP_SetDram_Addr(GOP_CTX_HAL_LOCAL *pGOPHalLocal, MS_U8 u8GOP, MS_U8 u8win, MS_PHY PhyAddr, E_DRV_GOP_AddrType eAddrType);
GOP_Result HAL_GOP_GetDram_Addr(GOP_CTX_HAL_LOCAL *pGOPHalLocal, MS_U8 u8GOP, MS_U8 u8win, MS_PHY* pphyAddr, E_DRV_GOP_AddrType eAddrType);
GOP_Result MHal_GOP_VOP_Path_Sel(GOP_CTX_HAL_LOCAL *pGOPHalLocal,EN_DRV_GOP_VOP_PATH enVOPPath);
GOP_Result MHal_GOP_SetCSCCtrl(GOP_CTX_HAL_LOCAL *pGOPHalLocal,MS_U32 u32GOP,ST_GOP_CSC_TABLE *pstCSCTbl);
GOP_Result MHal_GOP_AutoDetectBuf(GOP_CTX_HAL_LOCAL *pstGOPHalLocal, ST_GOP_AUTO_DETECT_BUF_INFO* pstAutoDectInfo);
GOP_Result MHal_GOP_GWIN_SetColorMatrix(GOP_CTX_HAL_LOCAL *pstGOPHalLocal, MS_U8 u8GOP, GOP_OupputColor enGopOutColor, DRV_GOPColorType enColorType, MS_BOOL bCscEnable);
GOP_Result MHAL_GOP_Load_HStretch_New4TapMode_Table(GOP_CTX_HAL_LOCAL *pGOPHalLocal, MS_U8 u8GOP, MS_U16* pu16GopHStretchTable);
GOP_Result MHAL_GOP_Load_VStretch_V4TapMode_Table(GOP_CTX_HAL_LOCAL *pGOPHalLocal, MS_U8 u8GOP, MS_U16* pu16GopVStretchTable);
GOP_Result MHal_GOP_AutoDetectBuf_V2(GOP_CTX_HAL_LOCAL *pstGOPHalLocal, ST_GOP_AUTO_DETECT_BUF_INFO* pstAutoDectInfo);
GOP_Result MHal_GOP_GetOsdNonTransCnt(GOP_CTX_HAL_LOCAL *pstGOPHalLocal, MS_U32* pu32Count);
GOP_Result MHal_GOP_Trigger_MUX(GOP_CTX_HAL_LOCAL *pstGOPHalLocal);
GOP_Result MHal_GOP_FillCscMartrix(GOP_CTX_HAL_LOCAL *pstGOPHalLocal,MS_U16 *pu16CSCValue, EN_GOP_CFD_MC_FORMAT enInFmt, EN_GOP_CFD_MC_FORMAT enOutFmt);
GOP_Result HAL_GOP_GWIN_SetHMirror(GOP_CTX_HAL_LOCAL *pstGOPHalLocal, MS_U8 u8GOP, MS_BOOL bEnable);
GOP_Result HAL_GOP_SetVMirror(GOP_CTX_HAL_LOCAL *pstGOPHalLocal, MS_U8 u8GOP, MS_BOOL bEnable);
MS_BOOL HAL_GOP_GetVMirror(GOP_CTX_HAL_LOCAL *pstGOPHalLocal, MS_U8 u8GOP);
MS_BOOL HAL_GOP_GetHMirror(GOP_CTX_HAL_LOCAL *pstGOPHalLocal, MS_U8 u8GOP);
GOP_Result HAL_GOP_SetGWIN_Enable(GOP_CTX_HAL_LOCAL *pstGOPHalLocal, MS_U8 u8GOP, MS_U8 u8win, MS_BOOL bEnable);
GOP_Result HAL_GOP_GetWinFmt(GOP_CTX_HAL_LOCAL *pstGOPHalLocal, MS_U8 u8GOP, MS_U8 u8Win, MS_U16 *pu16Colortype);
GOP_Result HAL_GOP_SetPitch(GOP_CTX_HAL_LOCAL *pstGOPHalLocal, MS_U8 u8GOP, MS_U8 u8Win, MS_U16 u16FbWidth, MS_U16 u16FbHeight);
MS_U16 HAL_GOP_GetPitch(GOP_CTX_HAL_LOCAL *pstGOPHalLocal, MS_U8 u8GOP, MS_U8 u8Win);
GOP_Result HAL_GOP_GetGopGwinHVPixel(GOP_CTX_HAL_LOCAL *pstGOPHalLocal, MS_U8 u8GOP, MS_U8 u8Win, MS_U16 *u16Hstart, MS_U16 *u16Hend, MS_U16 *u16Vstart, MS_U16 *u16Vend);
GOP_Result HAL_GOP_GWIN_SetProgressive(GOP_CTX_HAL_LOCAL *pstGOPHalLocal, MS_U8 u8GOP, MS_BOOL bEnable);
MS_BOOL HAL_GOP_GWIN_GetProgressive(GOP_CTX_HAL_LOCAL *pstGOPHalLocal, MS_U8 u8GOP);
GOP_Result HAL_GOP_SetGopStretchWin(GOP_CTX_HAL_LOCAL *pstGOPHalLocal, MS_U8 u8GOP, MS_U16 u16x, MS_U16 u16y, MS_U16 u16width, MS_U16 u16height);
GOP_Result HAL_GOP_GetGopStretchWin(GOP_CTX_HAL_LOCAL *pstGOPHalLocal, MS_U8 u8GOP, MS_U16 *u16x, MS_U16 *u16y, MS_U16 *u16width, MS_U16 *u16height);
GOP_Result HAL_GOP_GWIN_SetAlphaInverse(GOP_CTX_HAL_LOCAL *pstGOPHalLocal, MS_U8 u8GOP, MS_BOOL bEnable);
GOP_Result HAL_GOP_GetGopScaleRatio(GOP_CTX_HAL_LOCAL *pstGOPHalLocal, MS_U8 u8GOP, MS_U32 *u32Hratio, MS_U32 *u32Vratio);
GOP_Result HAL_GOP_SetGopVaildH(GOP_CTX_HAL_LOCAL *pstGOPHalLocal, MS_U8 u8GOP, MS_U16 u16ValidH);
GOP_Result HAL_GOP_SetGopVaildV(GOP_CTX_HAL_LOCAL *pstGOPHalLocal, MS_U8 u8GOP, MS_U16 u16ValidV);
GOP_Result HAL_GOP_GWIN_SetTransClr(GOP_CTX_HAL_LOCAL *pstGOPHalLocal, MS_U8 u8GOP, GOP_TransClrFmt fmt, MS_BOOL bEnable);
EN_DRV_GOP_VOP_PATH Hal_GOP_GetVOPPathSel(GOP_CTX_HAL_LOCAL *pstGOPHalLocal);
GOP_Result HAL_GOP_SetPipe(GOP_CTX_HAL_LOCAL *pstGOPHalLocal, MS_U8 u8GOP, MS_U16 u16HSPipe);
GOP_Result HAL_GOP_SetVOPNBL(GOP_CTX_HAL_LOCAL *pstGOPHalLocal, MS_BOOL bEnable);
MS_BOOL HAL_GOP_GetGWINEnable(GOP_CTX_HAL_LOCAL *pstGOPHalLocal, MS_U8 u8GOP, MS_U8 u8Win);
GOP_Result HAL_GOP_GWIN_TriggerRegWriteIn(GOP_CTX_HAL_LOCAL *pstGOPHalLocal, MS_U8 u8GOP, MS_BOOL bEnable, E_DRV_GOPTriggerMode eGOPTriggerMode);
GOP_Result HAL_GOP_SetFirstHs(GOP_CTX_HAL_LOCAL *pstGOPHalLocal, MS_U8 u8GOP, MS_BOOL bEnable);
GOP_Result HAL_GOP_SetHSCALE(GOP_CTX_HAL_LOCAL *pstGOPHalLocal, MS_U8 u8GOP, MS_U16 u16HInitPhase, MS_U64 u64Hratio);
GOP_Result HAL_GOP_SetVSCALE(GOP_CTX_HAL_LOCAL *pstGOPHalLocal, MS_U8 u8GOP, MS_U16 u16VInitPhase, MS_U64 u64Vratio);
GOP_Result HAL_GOP_SetStep1HVSPHscale(GOP_CTX_HAL_LOCAL *pstGOPHalLocal, MS_U8 u8GOP, MS_BOOL bEnable, MS_U32 u32Hratio, MS_U16 u16Hout);
GOP_Result HAL_GOP_SetStep1HVSPVscale(GOP_CTX_HAL_LOCAL *pstGOPHalLocal, MS_U8 u8GOP, MS_BOOL bEnable, MS_U32 u32Vratio, MS_U16 u16Vout);
GOP_Result HAL_GOP_SetStep2HVSPHscale(GOP_CTX_HAL_LOCAL *pstGOPHalLocal, MS_BOOL bEnable, MS_U32 u32Hratio);
GOP_Result HAL_GOP_SetStep2HVSPVscale(GOP_CTX_HAL_LOCAL *pstGOPHalLocal, MS_BOOL bEnable, MS_U32 u32Vratio);
GOP_Result HAL_GOP_SetMixer2OutSize(GOP_CTX_HAL_LOCAL *pstGOPHalLocal, MS_U16 u16Width, MS_U16 u16Height);
GOP_Result HAL_GOP_SetMixer4OutSize(GOP_CTX_HAL_LOCAL *pstGOPHalLocal, MS_U16 u16Width, MS_U16 u16Height);
GOP_Result HAL_GOP_SetHTotal(GOP_CTX_HAL_LOCAL *pstGOPHalLocal, MS_U8 u8GOP, MS_U16 u16Width);
GOP_Result HAL_GOP_SetInterrupt(GOP_CTX_HAL_LOCAL *pstGOPHalLocal, MS_U8 u8GOP, MS_BOOL bEnable);
GOP_Result HAL_GOP_SetBrightness(GOP_CTX_HAL_LOCAL *pstGOPHalLocal, MS_U8 u8GOP, MS_U16 u16BriVal, MS_BOOL bMSB);
GOP_Result HAL_GOP_GetBrightness(GOP_CTX_HAL_LOCAL *pstGOPHalLocal, MS_U8 u8GOP, MS_U16 *u16BriVal, MS_BOOL *bMSB);
GOP_Result HAL_GOP_Set_HStretchMode(GOP_CTX_HAL_LOCAL *pstGOPHalLocal, MS_U8 u8GOP, DRV_GOPStrchHMode HStrchMode);
GOP_Result HAL_GOP_Set_VStretchMode(GOP_CTX_HAL_LOCAL *pstGOPHalLocal, MS_U8 u8GOP, DRV_GOPStrchVMode VStrchMode);
GOP_Result HAL_GOP_AFBC_Core_Enable(GOP_CTX_HAL_LOCAL *pstGOPHalLocal, MS_U8 u8GOP, MS_BOOL bEnable);
GOP_Result MHal_GOP_AFBCMode(GOP_CTX_HAL_LOCAL *pGOPHalLocal, MS_U8 u8GOP, MS_BOOL bEnable);
GOP_Result MHal_GOP_AFBCSetWindow(GOP_CTX_HAL_LOCAL*pGOPHalLocal, MS_U8 u8GOP, DRV_GOP_AFBC_Info* pinfo);
GOP_Result HAL_GOP_ByPassMode(GOP_CTX_HAL_LOCAL*pGOPHalLocal, MS_U32 u32GOPIdx, MS_BOOL bEnable);
GOP_Result HAL_GOP_SetIPVersion(MS_U32 u32ChipVer);
GOP_Result HAL_GOP_MODhbkproch_protect(bool bIsByPassMode);

#endif // _HAL_TEMP_H_
