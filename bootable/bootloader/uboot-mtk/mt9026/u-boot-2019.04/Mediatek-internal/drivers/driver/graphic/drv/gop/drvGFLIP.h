/* SPDX-License-Identifier: (GPL-2.0 OR BSD-3-Clause) */
/*
 * Copyright (c) 2020 MediaTek Inc.
 */

#ifndef _MDRV_GFLIP_H
#define _MDRV_GFLIP_H

#ifdef _MDRV_GFLIP_C
#define INTERFACE
#else
#define INTERFACE extern
#endif

#if defined(__cplusplus)
extern "C" {
#endif
#include "drvGOP.h"
#include "halGOP.h"
#include "mdrv_gflip_st.h"


#define MAX_GOP_GWIN             MAX_GOP0_GWIN
#define MAX_FLIP_ADDR_FIFO      (0x10UL)

//=============================================================================
// Defines & Macros
//=============================================================================

//=============================================================================
// Type and Structure Declaration
//=============================================================================
typedef struct DLL_PACKED
{
    MS_PHY u64Addr; //in
    MS_U32 u32TagId; //in
}GFLIP_INFO, *PGFLIP_INFO;

typedef struct __attribute__((packed))
{
  MS_GOP_CTX_SHARED apiCtxShared;
  GOP_CTX_HAL_SHARED halCtxShared;
}GOP_CTX_DRV_SHARED;
typedef struct DLL_PACKED
{
    MS_GOP_CTX_LOCAL  apiCtxLocal;
    GOP_CTX_DRV_SHARED *pDrvCtxShared;//pointer to shared context paramemetrs
    GOP_CTX_HAL_LOCAL    halCtxLocal;
    MS_U8 *pu8REGMAP_Base[SHARED_GOP_MAX_COUNT];
    MS_U16 u16REGMAP_Offset[SHARED_GOP_MAX_COUNT];
    MS_U32 u32REGMAP_Len[SHARED_GOP_MAX_COUNT];
    //GFLIP parameters
    MS_BOOL bEnableVsyncIntFlip[SHARED_GOP_MAX_COUNT];
#ifdef MSOS_TYPE_LINUX
    MS_S32 s32FdGFlip;
#elif defined(MSOS_TYPE_LINUX_KERNEL)
#else
    GFLIP_INFO GFlipInfo[SHARED_GOP_MAX_COUNT][MAX_GOP_GWIN][MAX_FLIP_ADDR_FIFO];
    MS_U32 u32GFlipInfoReadPtr[SHARED_GOP_MAX_COUNT][MAX_GOP_GWIN];
    MS_U32 u32GFlipInfoWritePtr[SHARED_GOP_MAX_COUNT][MAX_GOP_GWIN];
    MS_U16 u16GOPRefCnt[SHARED_GOP_MAX_COUNT];
    MS_BOOL bGFlipInVsyncLimitation[SHARED_GOP_MAX_COUNT];
    GFLIP_DWININT_INFO gflipDWinIntInfo;
#endif
    MS_BOOL gop_gwin_frwr;
    MS_BOOL gop_dwin_frwr;
    MS_BOOL bGOPBankFwr[SHARED_GOP_MAX_COUNT];
    MS_BOOL b16BPPTileMode[SHARED_GOP_MAX_COUNT];
    MS_BOOL b32BPPTileMode[SHARED_GOP_MAX_COUNT];

    MS_U8 current_gop;
}GOP_CTX_DRV_LOCAL;

//=============================================================================
// Function
//=============================================================================

//Drv Interface related(drv interface):
INTERFACE MS_BOOL MDrv_GOP_GFLIP_CSC_Tuning(MS_GOP_CTX_LOCAL*pstGOPCtx,MS_U32 u32GOPNum,ST_GOP_CSC_PARAM *pstCSCParam,ST_DRV_GOP_CFD_OUTPUT *pstCFDOut);

#if defined(__cplusplus)
}
#endif

#undef INTERFACE

#endif //_MDRV_GFLIP_H

