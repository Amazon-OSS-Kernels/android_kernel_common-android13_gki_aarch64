/* SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause) */
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

//////////////////////////////////////////////////////////////////////////////////////////////////
///
/// @file   drvSEM.h
/// @brief  SEM Driver Interface
/// @author MStar Semiconductor Inc.
///////////////////////////////////////////////////////////////////////////////////////////////////


/*! \defgroup G_SEM SEM interface
    \ingroup  G_PERIPHERAL

    \brief

    a semaphore is a variable or abstract data type that is used
    for controlling access, by multiple processes, to a common
    resource in a concurrent system such as a multiprogramming
    operating system.

    <b>Features</b>
    - SEM initialize
    - SEM resource manager
    - SEAM lock & unlock

    <b> Operation Code Flow: </b> \n
    -# SEM initilize
    -# Get resource
    -# Lock
    -# UnLock

    \defgroup G_SEM_INIT Initialization Task relative
    \ingroup  G_SEM
    \defgroup G_SEM_COMMON Common Task relative
    \ingroup  G_SEM
    \defgroup G_SEM_CONTROL Control relative
    \ingroup  G_SEM
    \defgroup G_SEM_INT  Interrupt relative
    \ingroup  G_SEM
    \defgroup G_SEM_ToBeModified GPIO api to be modified
    \ingroup  G_SEM
    \defgroup G_SEM_ToBeRemove GPIO api to be removed
    \ingroup  G_SEM
*/

#ifndef __DRV_SEM_H__
#define __DRV_SEM_H__

#include "MsCommon.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define SEM_DRV_VERSION                 /* Character String for DRV/API version             */  \
    MSIF_TAG,                           /* 'MSIF'                                           */  \
    MSIF_CLASS,                         /* '00'                                             */  \
    MSIF_CUS,                           /* 0x0000                                           */  \
    MSIF_MOD,                           /* 0x0000                                           */  \
    MSIF_CHIP,                                                                                  \
    MSIF_CPU,                                                                                   \
    {'S','E','M','_'},                  /* IP__                                             */  \
    {'0','1'},                          /* 0.0 ~ Z.Z                                        */  \
    {'0','1'},                          /* 00 ~ 99                                          */  \
    {'0','0','2','6','4','8','8','5'},  /* CL#                                              */  \
    MSIF_OS

//-------------------------------------------------------------------------------------------------
//  Local Defines
//-------------------------------------------------------------------------------------------------
#define SEM_WAIT_FOREVER    (0xffffff00)
#define SEM_WAIT_NONE       (0x00000000)
#define SEM_ID_PREFIX       (0x36970000)
//-------------------------------------------------------------------------------------------------
// Type and Structure Declaration
//-------------------------------------------------------------------------------------------------
typedef enum
{
    E_SEM_NONE = SEM_ID_PREFIX,
    E_SEM_DUMMY,
    E_SEM_GE0,
    E_SEM_GE1,
    E_SEM_BDMA,
    E_SEM_PM,
    E_SEM_MBX_RECV,
    E_SEM_MBX_SEND,
    E_SEM_TEE,
    E_SEM_USER0,
    E_SEM_USER1,
    E_SEM_USER2,
    E_SEM_USER3,
    E_SEM_CA,
    E_SEM_IIC,
    E_SEM_AESDMA0,
    E_SEM_AESDMA1,
    E_SEM_EFUSE,
    E_SEM_FCIE,
    E_SEM_TSP,
    /* add new here */
    E_SEM_MAX_SUPPORTED_DEV
}eSemId;

typedef enum _SEM_DbgLvl
{
    E_SEM_DBGLVL_NONE = 0,      /// no debug message
    E_SEM_DBGLVL_WARNING,       /// show warning only
    E_SEM_DBGLVL_ERROR,         /// show error only
    E_SEM_DBGLVL_INFO,          /// show error & informaiton
    E_SEM_DBGLVL_ALL,           /// show error, information & funciton name
}SEM_DbgLvl;

//-------------------------------------------------------------------------------------------------
// Extern Functions
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
/// Set IR enable function.
/// @ingroup G_SEM_INIT
/// @param bEnable \b IN: TRUE: enable IR, FALSE: disable IR
/// @return E_IR_OK: Success
/// @return E_IR_FAIL or other values: Failure
//-------------------------------------------------------------------------------------------------
MS_BOOL MDrv_SEM_Init(void);
//-------------------------------------------------------------------------------------------------
/// Set IR enable function.
/// @ingroup G_SEM_CONTROL
/// @param bEnable \b IN: TRUE: enable IR, FALSE: disable IR
/// @return E_IR_OK: Success
/// @return E_IR_FAIL or other values: Failure
//-------------------------------------------------------------------------------------------------
MS_BOOL MDrv_SEM_Get_Resource(MS_U8 u8SemID, MS_U16 u16ResId);
//-------------------------------------------------------------------------------------------------
/// Set IR enable function.
/// @ingroup G_SEM_CONTROL
/// @param bEnable \b IN: TRUE: enable IR, FALSE: disable IR
/// @return E_IR_OK: Success
/// @return E_IR_FAIL or other values: Failure
//-------------------------------------------------------------------------------------------------
MS_BOOL MDrv_SEM_Free_Resource(MS_U8 u8SemID, MS_U16 u16ResId);
//-------------------------------------------------------------------------------------------------
/// Set IR enable function.
/// @ingroup G_SEM_CONTROL
/// @param bEnable \b IN: TRUE: enable IR, FALSE: disable IR
/// @return E_IR_OK: Success
/// @return E_IR_FAIL or other values: Failure
//-------------------------------------------------------------------------------------------------
MS_BOOL MDrv_SEM_Reset_Resource(MS_U8 u8SemID);
//-------------------------------------------------------------------------------------------------
/// Set IR enable function.
/// @ingroup G_SEM_CONTROL
/// @param bEnable \b IN: TRUE: enable IR, FALSE: disable IR
/// @return E_IR_OK: Success
/// @return E_IR_FAIL or other values: Failure
//-------------------------------------------------------------------------------------------------
MS_BOOL MDrv_SEM_Get_ResourceID(MS_U8 u8SemID, MS_U16* pu16ResId);
//-------------------------------------------------------------------------------------------------
/// Set IR enable function.
/// @ingroup G_SEM_CONTROL
/// @param bEnable \b IN: TRUE: enable IR, FALSE: disable IR
/// @return E_IR_OK: Success
/// @return E_IR_FAIL or other values: Failure
//-------------------------------------------------------------------------------------------------
MS_U32 MDrv_SEM_Get_Num(void);
//-------------------------------------------------------------------------------------------------
/// Set IR enable function.
/// @ingroup G_SEM_COMMON
/// @param bEnable \b IN: TRUE: enable IR, FALSE: disable IR
/// @return E_IR_OK: Success
/// @return E_IR_FAIL or other values: Failure
//-------------------------------------------------------------------------------------------------
MS_BOOL MDrv_SEM_GetLibVer(const MSIF_Version **ppVersion);
//-------------------------------------------------------------------------------------------------
/// Set IR enable function.
/// @ingroup G_SEM_COMMON
/// @param bEnable \b IN: TRUE: enable IR, FALSE: disable IR
/// @return E_IR_OK: Success
/// @return E_IR_FAIL or other values: Failure
//-------------------------------------------------------------------------------------------------
MS_U32 MDrv_SEM_SetPowerState(EN_POWER_MODE u16PowerState);
//-------------------------------------------------------------------------------------------------
/// Set IR enable function.
/// @ingroup G_SEM_CONTROL
/// @param bEnable \b IN: TRUE: enable IR, FALSE: disable IR
/// @return E_IR_OK: Success
/// @return E_IR_FAIL or other values: Failure
//-------------------------------------------------------------------------------------------------
MS_BOOL MDrv_SEM_Lock(eSemId SemId, MS_U32 u32WaitMs);
//-------------------------------------------------------------------------------------------------
/// Set IR enable function.
/// @ingroup G_SEM_CONTROL
/// @param bEnable \b IN: TRUE: enable IR, FALSE: disable IR
/// @return E_IR_OK: Success
/// @return E_IR_FAIL or other values: Failure
//-------------------------------------------------------------------------------------------------
MS_BOOL MDrv_SEM_Unlock(eSemId SemId);
//-------------------------------------------------------------------------------------------------
/// Set IR enable function.
/// @ingroup G_SEM_CONTROL
/// @param bEnable \b IN: TRUE: enable IR, FALSE: disable IR
/// @return E_IR_OK: Success
/// @return E_IR_FAIL or other values: Failure
//-------------------------------------------------------------------------------------------------
MS_BOOL MDrv_SEM_Delete(eSemId SemId);
//-------------------------------------------------------------------------------------------------
/// Set SEM debug function level.
/// @ingroup G_SEM_COMMON
/// @param eLevel \b IN: E_SEM_DBGLVL_NONE/E_SEM_DBGLVL_WARNING/E_SEM_DBGLVL_ERROR/E_SEM_DBGLVL_INFO/E_SEM_DBGLVL_ALL
/// @return TRUE: Success
/// @return other values: Failure
//-------------------------------------------------------------------------------------------------
MS_BOOL MDrv_SEM_SetDbgLevel(SEM_DbgLvl eLevel);

//==============================================================================
// Semaphore Usage in Utopia
// See halSEM.c
//==============================================================================

#ifdef __cplusplus
} // closing brace for extern "C"
#endif

#endif // #ifndef __DRV_SEM_H__
