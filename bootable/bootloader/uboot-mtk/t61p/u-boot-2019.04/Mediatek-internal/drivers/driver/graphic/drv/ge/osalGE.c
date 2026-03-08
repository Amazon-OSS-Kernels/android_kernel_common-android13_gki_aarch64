// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

////////////////////////////////////////////////////////////////////////////////////////////////////
// file    osalGE.c
// @brief  Graphic Engine (GE) OSAL
// @author MStar Semiconductor,Inc.
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "MsCommon.h"
#include "MsTypes.h"
#include "osalGE.h"
#include "drvMMIO.h"
#ifdef MSOS_TYPE_NOS

extern void mhal_dcache_flush(MS_U32 u32Base, MS_U32 u32Size);

#define GE_DCACHE_FLUSH mhal_dcache_flush
#else
#include "MsOS.h"

#define GE_DCACHE_FLUSH MsOS_Dcache_Flush
#endif

#if defined(MCU_MIPS_34K) || defined(MCU_MIPS_74K) || defined(MCU_MIPS_1004K)
#define REG_BASE_MAP    (0xBF200000)
#elif (defined(MCU_AEON))
#define REG_BASE_MAP    (0xA0000000)
#else   //mips 4ke
#define REG_BASE_MAP    (0xBF800000)
#endif

MS_BOOL OSAL_GE_GetMapBase(MS_VIRT *VAddr, MS_PHY *puSize)
{
	*VAddr = GE_MMIO_NONPM_BASE0;
    return TRUE;
}

MS_BOOL OSAL_GE_GetMapBase2(MS_VIRT *VAddr, MS_PHY *puSize)
{
	*VAddr = GE_MMIO_NONPM_BASE1;
    return TRUE;
}

// Mutex
MS_S32 OSAL_GE_MutexCreate(MS_U8* pu8Name)
{
    return 0;
}

MS_BOOL OSAL_GE_MutexObtain(MS_S32 s32MutexId, MS_U32 u32Timeout)
{
    return TRUE;
}

MS_BOOL OSAL_GE_MutexRelease(MS_S32 s32MutexId)
{
    return TRUE;
}

MS_BOOL OSAL_GE_Dcache_Flush( MS_VIRT VABase, MS_U32 u32Size )
{
    GE_DCACHE_FLUSH(VABase, u32Size);
    return TRUE;
}
#if 0       // No IRQ supported in GE
void OSAL_GE_IntAttach(OSAL_GE_IsrProc Proc, MS_U8 u8Ch)
{
#ifdef MSOS_TYPE_NOS
    MsOS_AttachInterrupt(E_INT_IRQ_BDMA_CH0+u8Ch, (InterruptCb)Proc);
#else
 //   MsOS_AttachInterrupt(E_INT_IRQ_BDMA_CH0+u8Ch, Proc);
#endif
}

void OSAL_GE_IntDetach(MS_U8 u8Ch)
{
#ifdef MSOS_TYPE_NOS
    MsOS_DetachInterrupt(E_INT_IRQ_BDMA_CH0+u8Ch);
#else
    MsOS_DetachInterrupt((InterruptNum)(E_INT_IRQ_BDMA_CH0+u8Ch));
#endif
}
#endif

