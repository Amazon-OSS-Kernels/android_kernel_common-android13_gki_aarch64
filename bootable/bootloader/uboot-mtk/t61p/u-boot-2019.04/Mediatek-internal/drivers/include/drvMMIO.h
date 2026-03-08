/* SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause) */
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

///////////////////////////////////////////////////////////////////////////////////////////////////
///
/// @file   drvMMIO.h
/// @brief  MMIO SYS Driver Interface
/// @author MStar Semiconductor Inc.
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef __DRV_MMIO_H__
#define __DRV_MMIO_H__


#ifdef __cplusplus
extern "C"
{
#endif

#include "MsTypes.h"

//-------------------------------------------------------------------------------------------------
//  Driver Capability
//-------------------------------------------------------------------------------------------------
#define MSIF_MMIO_LIB_CODE              {'M','M','I','O'}    //Lib code
#define MSIF_MMIO_LIBVER                {'0','1'}            //LIB version
#define MSIF_MMIO_BUILDNUM              {'0','2'}            //Build Number
#define MSIF_MMIO_CHANGELIST            {'0','0','4','0','9','9','0','8'} //P4 ChangeList Number

#define MMIO_DRV_VERSION                /* Character String for DRV/API version             */  \
    MSIF_TAG,                           /* 'MSIF'                                           */  \
    MSIF_CLASS,                         /* '00'                                             */  \
    MSIF_CUS,                           /* 0x0000                                           */  \
    MSIF_MOD,                           /* 0x0000                                           */  \
    MSIF_CHIP,                                                                                  \
    MSIF_CPU,                                                                                   \
    MSIF_MMIO_LIB_CODE,                  /* IP__                                             */  \
    MSIF_MMIO_LIBVER,                    /* 0.0 ~ Z.Z                                        */  \
    MSIF_MMIO_BUILDNUM,                  /* 00 ~ 99                                          */  \
    MSIF_MMIO_CHANGELIST,                /* CL#                                              */  \
    MSIF_OS

//-------------------------------------------------------------------------------------------------
//  Macro and Define
//-------------------------------------------------------------------------------------------------
// HW register space
#define DRV_MMIO_PM_BANK            0x0001
#define DRV_MMIO_NONPM_BANK         0x0002
#define DRV_MMIO_FLASH_BANK0        0x0003
#define DRV_MMIO_FRC_BANK           0x0004
#define DRV_MMIO_EXTRIU_BANK        0x0005
#define DRV_MMIO_OTP_BANK           0x0010
#define DRV_MMIO_SPRAM_BANK         0x0011
#define DRV_MMIO_OTP_BANK2          0x0012
#define DRV_MMIO_XC_EXT_BANK        0x0013
#define DRV_MMIO_VDEC_BANK	    0x0014


// Specific HW IP base
#define DRV_MMIO_SC_BANK            0x0100
#define DRV_MMIO_TSP_BANK           0x0101
#define DRV_MMIO_NDS_BANK           0x0102
#define DRV_MMIO_NSK_BANK           0x0103
#define DRV_MMIO_CAM_BANK           0x0104
#define DRV_MMIO_NGA_BANK           0x0105
#define DRV_MMIO_SC1_BANK           0x0106


//-------------------------------------------------------------------------------------------------
//  Type and Structure
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Function and Variable
//-------------------------------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////////////////////////
/// @file   drvMMIO.h
/// @brief  MMIO
/// @author MStar Semiconductor Inc.
/// @version 1.0
/////////////////////////////////////////////////////////////////////////////////////////////////////

/*! \defgroup SYS SYS modules
 *
 * \defgroup MMIO MMIO interface (drvMMIO.h)
 *  \ingroup SYS

	\brief

	MMIO maps the RIU to userspace so that drivers are capable to access it directly.

	<b> Diagram </b>
	\image html mmio_1.png

	<b> Code Flow </b>
	-# Init(setup mapping)
	-# Each driver get its own base
	\image html mmio_2.png

 * \defgroup MMIO_BASIC basic mmio interface
 *  \ingroup MMIO
*/

//-------------------------------------------------------------------------------------------------
/// Init MMIO driver
/// @ingroup MMIO_BASIC
//-------------------------------------------------------------------------------------------------
MS_BOOL MDrv_MMIO_Init(void);
//-------------------------------------------------------------------------------------------------
/// Get the virtual address of iomap for register
/// @ingroup MMIO_BASIC
//-------------------------------------------------------------------------------------------------
MS_BOOL MDrv_MMIO_GetBASE(MS_VIRT *ptrBaseaddr, MS_PHY *u32Basesize, MS_U32 u32Module);
//-------------------------------------------------------------------------------------------------
/// DeInit MMIO driver
/// @ingroup MMIO_BASIC
//-------------------------------------------------------------------------------------------------
MS_BOOL MDrv_MMIO_Close(void);


#ifdef __cplusplus
}
#endif


#endif // __DRV_MMIO_H__
