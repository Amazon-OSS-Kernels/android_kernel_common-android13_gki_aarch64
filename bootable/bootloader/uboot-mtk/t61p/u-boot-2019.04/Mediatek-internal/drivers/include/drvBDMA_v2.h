/* SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause) */
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

//////////////////////////////////////////////////////////////////////////////////////////////////
///
/// @file   drvBDMA_v2.h
/// @brief  MIU Driver Interface
/// @author MStar Semiconductor Inc.
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _DRV_BDMA_V2_H_
#define _DRV_BDMA_V2_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include "UFO.h"
//-------------------------------------------------------------------------------------------------
// Defines
//-------------------------------------------------------------------------------------------------
// below for utopia20 structure

typedef enum {
    MDrv_CMD_BDMA_MemCopy,
    MDrv_CMD_BDMA_Search,
    MDrv_CMD_BDMA_CRC32,
    MDrv_CMD_BDMA_Pattern_Fill,
    MDrv_CMD_BDMA_FlashCopy2Dram,
    MDrv_CMD_BDMA_MOBFSearch,
    MDrv_CMD_BDMA_GetStatus,
    MDrv_CMD_BDMA_GetInfo,
    MDrv_CMD_BDMA_GetLibVer,
    MDrv_CMD_BDMA_GetMinSize,
    MDrv_CMD_BDMA_SetDbgLevel,
    MDrv_CMD_BDMA_SetSPIOffsetForMCU,
    MDrv_CMD_BDMA_WaitFlashDone,
    MDrv_CMD_BDMA_Init,
    MDrv_CMD_BDMA_CopyHnd,
    MDrv_CMD_BDMA_SetPowerState,
} eBdmaIoctlOpt;

//////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
// Macros
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
// Type and Structure Declaration
//-------------------------------------------------------------------------------------------------
typedef struct _BDMA_API_V1
{
}BDMA_API_V1;

typedef struct _BDMA_API_V2
{
}BDMA_API_V2;

#ifdef BIFROST_32BIT_MODE
// below are for utopia20 cmd and parameters structure
typedef struct _BDMA_INIT_PARAM
{
    MS_U32 u64Miu1Base;
}BDMA_INIT_PARAM, *PBDMA_INIT_PARAM;
// for cmd MDrv_CMD_BDMA_MemCopy
typedef struct _BDMA_MEMCOPY_PARAM
{
    MS_U32 phy64SrcAddr;
    MS_U32 phy64DstAddr;
    MS_U32 u32Len;
}BDMA_MEMCOPY_PARAM, *PBDMA_MEMCOPY_PARAM;

// for cmd MDrv_CMD_BDMA_Search
typedef struct DLL_PACKED _BDMA_SEARCH_PARAM
{
    MS_PHYADDR u32Addr;
    MS_U32 u32Len;
    MS_U32 u32Pattern;
    MS_U32 u32ExcluBit;
    BDMA_SrcDev eDev;
#if defined(UFO_PUBLIC_HEADER_212) || defined(UFO_PUBLIC_HEADER_300) || defined(UFO_PUBLIC_HEADER_500) || defined(UFO_PUBLIC_HEADER_500_3)
    MS_U32* pSearchAddr;
#else
    MS_U32 pSearchAddr;
#endif
}BDMA_SEARCH_PARAM, *PBDMA_SEARCH_PARAM;

// for cmd MDrv_CMD_BDMA_CRC32
typedef struct _BDMA_CRC32_PARAM
{
    MS_U32 phy64Addr;
    MS_U32 u32Len;
    MS_U32 u32Poly;
    MS_U32 u32Seed;
    BDMA_SrcDev eDev;
    MS_BOOL bReflect;
    MS_U32 u32CRC32;
}BDMA_CRC32_PARAM, *PBDMA_CRC32_PARAM;

// for cmd MDrv_CMD_BDMA_Pattern_Fill
typedef struct DLL_PACKED _BDMA_PATTERN_FILL_PARAM
{
    MS_PHY u32Addr;
    MS_U32 u32Len;
    MS_U32 u32Pattern;
    BDMA_DstDev eDev;
}BDMA_PATTERN_FILL_PARAM, *PBDMA_PATTERN_FILL_PARAM;

// for cmd MDrv_CMD_BDMA_FlashCopy2Dram
typedef struct DLL_PACKED _BDMA_FLASHCOPY2DRAM_PARAM
{
    MS_PHY u32FlashAddr;
    MS_PHY u32DramAddr;
    MS_U32 u32Len;
}BDMA_FLASHCOPY2DRAM_PARAM, *PBDMA_FLASHCOPY2DRAM_PARAM;

// for cmd MDrv_CMD_BDMA_CopyHnd
typedef struct DLL_PACKED _BDMA_COPYHND_PARAM
{
    MS_U32 phy64SrcAddr;
    MS_U32 phy64DstAddr;
    MS_U32 u32Len;
    BDMA_CpyType eCpyType;
    MS_U8 u8OpCfg;
}BDMA_COPYHND_PARAM, *PBDMA_COPYHND_PARAM;

// for cmd MDrv_CMD_BDMA_MOBFSearch
#ifdef MOBF_ENABLE
typedef struct DLL_PACKED _BDMA_MOBFSEARCH_PARAM
{
    MS_U32 phy64Addr;
    MS_U32 u32Len;
    BDMA_MOBF_PS* pMobfPsCfg;
    BDMA_SrcDev eDev;
    MS_U32 phy64SearchAddr;
}BDMA_MOBFSEARCH_PARAM, *PBDMA_MOBFSEARCH_PARAM;
#endif
#else
// below are for utopia20 cmd and parameters structure
typedef struct _BDMA_INIT_PARAM
{
    MS_PHY u64Miu1Base;
}BDMA_INIT_PARAM, *PBDMA_INIT_PARAM;
// for cmd MDrv_CMD_BDMA_MemCopy
typedef struct _BDMA_MEMCOPY_PARAM
{
    MS_PHY phy64SrcAddr;
    MS_PHY phy64DstAddr;
    MS_U32 u32Len;
}BDMA_MEMCOPY_PARAM, *PBDMA_MEMCOPY_PARAM;

// for cmd MDrv_CMD_BDMA_Search
typedef struct DLL_PACKED _BDMA_SEARCH_PARAM
{
    MS_PHY u32Addr;
    MS_U32 u32Len;
    MS_U32 u32Pattern;
    MS_U32 u32ExcluBit;
    BDMA_SrcDev eDev;
#if defined(UFO_PUBLIC_HEADER_212) || defined(UFO_PUBLIC_HEADER_300) || defined(UFO_PUBLIC_HEADER_500) || defined(UFO_PUBLIC_HEADER_500_3)
    MS_U32* pSearchAddr;
#else
    MS_U32 pSearchAddr;
#endif
}BDMA_SEARCH_PARAM, *PBDMA_SEARCH_PARAM;

// for cmd MDrv_CMD_BDMA_CRC32
typedef struct _BDMA_CRC32_PARAM
{
    MS_PHY phy64Addr;
    MS_U32 u32Len;
    MS_U32 u32Poly;
    MS_U32 u32Seed;
    BDMA_SrcDev eDev;
    MS_BOOL bReflect;
    MS_U32 u32CRC32;
}BDMA_CRC32_PARAM, *PBDMA_CRC32_PARAM;

// for cmd MDrv_CMD_BDMA_Pattern_Fill
typedef struct DLL_PACKED _BDMA_PATTERN_FILL_PARAM
{
    MS_PHY u32Addr;
    MS_U32 u32Len;
    MS_U32 u32Pattern;
    BDMA_DstDev eDev;
}BDMA_PATTERN_FILL_PARAM, *PBDMA_PATTERN_FILL_PARAM;

// for cmd MDrv_CMD_BDMA_FlashCopy2Dram
typedef struct DLL_PACKED _BDMA_FLASHCOPY2DRAM_PARAM
{
    MS_PHY u32FlashAddr;
    MS_PHY u32DramAddr;
    MS_U32 u32Len;
}BDMA_FLASHCOPY2DRAM_PARAM, *PBDMA_FLASHCOPY2DRAM_PARAM;

// for cmd MDrv_CMD_BDMA_CopyHnd
typedef struct DLL_PACKED _BDMA_COPYHND_PARAM
{
    MS_PHY phy64SrcAddr;
    MS_PHY phy64DstAddr;
    MS_U32 u32Len;
    BDMA_CpyType eCpyType;
    MS_U8 u8OpCfg;
}BDMA_COPYHND_PARAM, *PBDMA_COPYHND_PARAM;

// for cmd MDrv_CMD_BDMA_MOBFSearch
#ifdef MOBF_ENABLE
typedef struct DLL_PACKED _BDMA_MOBFSEARCH_PARAM
{
    MS_PHY phy64Addr;
    MS_U32 u32Len;
    BDMA_MOBF_PS* pMobfPsCfg;
    BDMA_SrcDev eDev;
    MS_PHY phy64SearchAddr;
}BDMA_MOBFSEARCH_PARAM, *PBDMA_MOBFSEARCH_PARAM;
#endif
#endif

// for cmd MDrv_CMD_BDMA_GetStatus
typedef struct DLL_PACKED _BDMA_GETSTATUS_PARAM
{
    BDMA_Status *pStatus;
}BDMA_GETSTATUS_PARAM, *PBDMA_GETSTATUS_PARAM;


// for cmd MDrv_CMD_BDMA_GetInfo
typedef struct _BDMA_GETINFO_PARAM
{
    MS_U8       u8ChNum;
    MS_U16      u16ChipVer;
    MS_U32      u32IOMap;
    MS_U32      u32MIU1Base;
    MS_S32      s32Mutex;
    MS_BOOL     bInit;
    BDMA_DbgLv  eDbgLv;
    BDMA_HwInfo sHwCap;
}BDMA_GETINFO_PARAM, *PBDMA_GETINFO_PARAM;

// for cmd MDrv_CMD_BDMA_GetLibVer
typedef struct DLL_PACKED _BDMA_GETLIBVER_PARAM
{
    const MSIF_Version **ppVersion;
}BDMA_GETLIBVER_PARAM, *PBDMA_GETLIBVER_PARAM;

// for cmd MDrv_CMD_BDMA_GetMinSize
typedef struct _BDMA_GETMINSIZE_PARAM
{
    MS_U32 u32MinSize;
}BDMA_GETMINSIZE_PARAM, *PBDMA_GETMINSIZE_PARAM;

// for cmd MDrv_CMD_BDMA_SetDbgLevel
typedef struct _BDMA_SETDBGLEVEL_PARAM
{
    BDMA_DbgLv eLevel;
}BDMA_SETDBGLEVEL_PARAM, *PBDMA_SETDBGLEVEL_PARAM;

// for cmd MDrv_CMD_BDMA_SetPowerState
typedef struct _BDMA_SETPOWERSTATE_PARAM
{
    EN_POWER_MODE u16PowerState;
}BDMA_SETPOWERSTATE_PARAM, *PBDMA_SETPOWERSTATE_PARAM;

//-------------------------------------------------------------------------------------------------
// Extern Global Variabls
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Extern Functions
//-------------------------------------------------------------------------------------------------

//////////////////////////////////////////////////////////////

#ifdef __cplusplus
}
#endif

#endif // _DRV_BDMA_V2_H_

