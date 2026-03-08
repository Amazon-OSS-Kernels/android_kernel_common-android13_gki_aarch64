/* SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause) */
/*
 * Copyright (c) 2023 MediaTek Inc.
*/

////////////////////////////////////////////////////////////////////////////////////////////////////
// file   osalGE.h
// @brief  Graphic Engine (GE) OSAL
// @author MStar Semiconductor,Inc.
////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef __OSAL_GE_H__
#define __OSAL_GE_H__

////////////////////////////////////////////////////////////////////////////////
// Define & data type
////////////////////////////////////////////////////////////////////////////////
#define GE_MMIO_NONPM_BASE0	(0x1F200000UL)
#define GE_MMIO_NONPM_BASE1	(0x1F000000UL)
typedef struct
{
    MS_U32   r[32];          // GPR regs
#ifdef __AEONR2__
    MS_U32   machi2;         // Highest 32-bits of new 32x32=64 multiplier
#endif
    MS_U32   machi;          // High and low words of
    MS_U32   maclo;          // multiply/accumulate reg

    // These are only saved for exceptions and interrupts
    MS_S32          vector;         // Vector number
    MS_S32          sr;             // Status Reg
    MS_U64          pc;             // Program Counter

    // Saved only for exceptions, and not restored when continued:
    // Effective address of instruction/data access that caused exception
    MS_U32   eear;           // Exception effective address reg
} GE_SavedRegisters;

typedef void (*OSAL_GE_IsrProc)(GE_SavedRegisters *, MS_U32);

////////////////////////////////////////////////////////////////////////////////
// Extern function

//void    OSAL_GE_IntAttach(OSAL_GE_IsrProc Proc, MS_U8 u8Ch);
//void    OSAL_GE_IntDetach(MS_U8 u8Ch);
MS_S32  OSAL_GE_MutexCreate(MS_U8* pu8Name);
MS_BOOL OSAL_GE_Dcache_Flush( MS_VIRT VABase, MS_U32 u32Size);
MS_BOOL OSAL_GE_GetMapBase(MS_VIRT *VAddr, MS_PHY *puSize);
MS_BOOL OSAL_GE_GetMapBase2(MS_VIRT *VAddr, MS_PHY *puSize);
MS_BOOL OSAL_GE_MutexObtain(MS_S32 s32MutexId, MS_U32 u32Timeout);
MS_BOOL OSAL_GE_MutexRelease(MS_S32 s32MutexId);

#endif // #ifndef __OSAL_BDMA_H__
