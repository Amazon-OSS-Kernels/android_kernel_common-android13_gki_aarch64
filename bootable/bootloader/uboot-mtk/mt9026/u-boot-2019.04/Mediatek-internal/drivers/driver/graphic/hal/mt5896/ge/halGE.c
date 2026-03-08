// SPDX-License-Identifier: (GPL-2.0 OR BSD-3-Clause)
/*
 * Copyright (c) 2020 MediaTek Inc.
 */

//-------------------------------------------------------------------------------------------------
//  Include Files
//-------------------------------------------------------------------------------------------------
#include "MsCommon.h"
#ifndef MSOS_TYPE_LINUX_KERNEL
#include <string.h>
#endif
#include "regGE.h"
#include "drvGE.h"
#include "halGE.h"
#include "halCHIP.h"
#include "drvMIU.h"
#ifdef MSOS_TYPE_LINUX
#include "halMPool.h"
#endif
#include <linux/io.h>
#include "GE0_2410.h"
#include "ckgen.h"
//-------------------------------------------------------------------------------------------------
//  Driver Compiler Options
//-------------------------------------------------------------------------------------------------
#define GE_DITHER_RAND_ENABLE       0UL                                   //[TBD] Add new option for SetDither if rand is used in the future.
#define GE_PATCH_ENABLE             0UL

#define GE_LOG_ENABLE               0UL
#define MS_DEBUG                    1UL

//-------------------------------------------------------------------------------------------------
//  Local Defines
//-------------------------------------------------------------------------------------------------
#define GE_MIU_ADDR_MASK            0x7FFFFFFFUL                          // 128MB

/*#define GE_CMDQ_FREECNT()           ((GE_REG(REG_GE_STAT)&GE_STAT_CMDQ_MASK)>>GE_STAT_CMDQ_SHFT)
#define GE_VCMDQ_FREECNT()          (GE_REG(REG_GE_VCMDQ_STAT) + ((GE_REG(REG_GE_BIST_STAT)&GE_VCMDQ_STAT_H_MASK) << 16))

#define GE_BUSY()                   (GE_REG(REG_GE_STAT) & GE_STAT_BUSY)*/
#define GE_CMDQ_FREECNT()	mtk_read2bytemask(REG_001C_GE0, REG_GE_CMQ2_STATUS)
#define GE_VCMDQ_FREECNT()	(mtk_read2byte(REG_0010_GE0) + (mtk_read2bytemask(REG_0014_GE0, REG_GE_VCMQ_STATUS_1) << 16))

#define GE_BUSY()	(0)

#define GE_CMDQ_ENABLE              1UL // Always Enable
#define GE_CMD_SIZE_MAX             GE_STAT_CMDQ_MAX
#define GE_VCMD_SIZE_MAX            GE_STAT_VCMDQ_MAX
#define GE_CMD_SIZE                 1UL // 1 queue entry available for 2 commands, but we just check entry for convenience

#define GE_MAP_VCMD_SIZE_TO_HWDEF(x)  ((x))

//#define GE_YIELD()                  MsOS_YieldTask()
#define GE_YIELD()

#define GE_DBG(_fmt, _args...)      { }

#define GE_BURST_LEN                 128UL

#define GE_TAG_INTERRUPT_WAITING_TIME 10 // ms
#define GE_TAG_INTERRUPT_DEBUG_PRINT_THRESHOLD (500/GE_TAG_INTERRUPT_WAITING_TIME)

#if (defined(COLOR_CONVERT_PATCH)&&(COLOR_CONVERT_PATCH == 1))
#define DIFF_SRC_DST_CLR_FMT(secfmt, dstfmt) (secfmt!= dstfmt&& !((secfmt== E_MS_FMT_ARGB1555)&&(dstfmt== E_MS_FMT_ARGB1555_DST)))
#endif

#define GE_CLK_528M BIT(4)
//-------------------------------------------------------------------------------------------------
//  Local Structures
//-------------------------------------------------------------------------------------------------
#if (__GE_WAIT_TAG_MODE == __USE_GE_INT_MODE)
typedef enum
{
    E_GE_CLEAR_INT = 0x0001,
    E_GE_MASK_INT = 0x0002,
    E_GE_UNMASK_INT = 0x0004,
    E_GE_INT_TAG_MODE = 0x0008,
    E_GE_INT_NORMAL_MODE = 0x0010
} E_GE_INT_OP;
#endif

//-------------------------------------------------------------------------------------------------
//  Global Variables
//-------------------------------------------------------------------------------------------------                 // line pattern reset


#define REG_GE_INVALID 0xFF

const MS_U8  _GE_Reg_Backup[] = {
    REG_GE_VCMDQ_BASE_L, REG_GE_VCMDQ_BASE_H, REG_GE_BASE_MSB, REG_GE_YUV_MODE, REG_GE_VCMDQ_SIZE,
    REG_GE_EN, REG_GE_CFG, REG_GE_TH, _REG_GE_ROP2, REG_GE_BLEND, REG_GE_ALPHA, REG_GE_ALPHA_CONST,
    REG_GE_SCK_HTH_L, REG_GE_SCK_HTH_H, REG_GE_SCK_LTH_L, REG_GE_SCK_LTH_H, REG_GE_DCK_HTH_L,
    REG_GE_DCK_HTH_H, REG_GE_DCK_LTH_L, REG_GE_DCK_LTH_H, REG_GE_OP_MODE, REG_GE_ATEST_TH,
    REG_GE_SRC_BASE_L, REG_GE_SRC_BASE_H,REG_GE_DST_BASE_L, REG_GE_DST_BASE_H,
    REG_GE_SRC_PITCH, REG_GE_DST_PITCH, REG_GE_FMT,
    0x0035, 0x0036, 0x0037, 0x0038, 0x0039, 0x003a, 0x003b, 0x003c, 0x003d, 0x003e,  // I0~I4
    0x003f, 0x0040, 0x0041, 0x0042, 0x0043, 0x0044, 0x0045, 0x0046, 0x0047, 0x0048,  // I5-I9
    0x0049, 0x004a, 0x004b, 0x004c, 0x004d, 0x004e, 0x004f, 0x0050, 0x0051, 0x0052,  // I10-I14
    0x0053, 0x0054,                                                                  // I15
    REG_GE_CLIP_L, REG_GE_CLIP_R, REG_GE_CLIP_T, REG_GE_CLIP_B, REG_GE_ROT_MODE, REG_GE_BLT_SCK_MODE,
    REG_GE_BLT_SCK_CONST_L, REG_GE_BLT_SCK_CONST_H, REG_GE_BLT_DST_X_OFST, REG_GE_BLT_DST_Y_OFST,
    _REG_GE_LINE_DELTA, REG_GE_LINE_STYLE, _REG_GE_LINE_LENGTH, REG_GE_BLT_SRC_DX, REG_GE_BLT_SRC_DY,
    REG_GE_ITALIC_OFFSET, REG_GE_ITALIC_DELTA, REG_GE_PRIM_V0_X, REG_GE_PRIM_V0_Y, REG_GE_PRIM_V1_X,
    REG_GE_PRIM_V1_Y, REG_GE_PRIM_V2_X, REG_GE_PRIM_V2_Y, REG_GE_BLT_SRC_W, REG_GE_BLT_SRC_H,
    REG_GE_PRIM_C_L, REG_GE_PRIM_C_H, REG_GE_PRIM_RDX_L, REG_GE_PRIM_RDX_H, REG_GE_PRIM_RDY_L,
    REG_GE_PRIM_RDY_H, REG_GE_PRIM_GDX_L, REG_GE_PRIM_GDX_H, REG_GE_PRIM_GDY_L, REG_GE_PRIM_GDY_H,
    REG_GE_PRIM_BDX_L, REG_GE_PRIM_BDX_H, REG_GE_PRIM_BDY_L, REG_GE_PRIM_BDY_H, REG_GE_PRIM_ADX,
    REG_GE_PRIM_ADY, REG_GE_INVALID
};


//-------------------------------------------------------------------------------------------------
//  Debug Functions
//-------------------------------------------------------------------------------------------------


//------------------------------------------------------------------------------
//  Local Var
//------------------------------------------------------------------------------
GE_CHIP_PROPERTY g_GeChipPro =
{
    .WordUnit =                         GE_WordUnit,

    .bSupportFourePixelMode =               TRUE,
    .bFourPixelModeStable =                 TRUE,

    .bSupportMultiPixel =               FALSE,
    .bSupportSpiltMode =                TRUE,
    .bSupportTwoSourceBitbltMode =      FALSE,
    .bSupportTLBMode =                  TRUE,
    .MIUSupportMaxNUM =                 GE_MAX_MIU,
    .BltDownScaleCaps =
    {
        .u8RangeMax =                   1,
        .u8RangeMin =                   32,
        .u8ContinuousRangeMin =         1,
        .bFullRangeSupport =            TRUE,

        .u8ShiftRangeMax =              0,              /// 1   = 2^0   = 1<<0
        .u8ShiftRangeMin =              5,              /// 32  = 2^5   = 1<<5
        .u8ShiftContinuousRangeMin =    0,              /// 1   = 2^0   = 1<<0
    }
};


#if (__GE_WAIT_TAG_MODE == __USE_GE_INT_MODE)
static MS_VIRT virtHalIomapBaseAddr = 0;
static MS_BOOL bGeIrqInited = FALSE;
static MS_S32 s32GeWaitTagEventHandle = -1;
static MS_S32 s32WaitingTagPid = -1;

void            _GE_WaitTag_InterruptCbk(InterruptNum eIntNum);
static GE_Result _GE_Ctrl_IntMode(GE_CTX_HAL_LOCAL *pGEHalLocal, E_GE_INT_OP int_op);
static void     _GE_Print_GeWaitTagTimeout_Msg(GE_CTX_HAL_LOCAL *pGEHalLocal, MS_U16 tagID);
#endif

MS_U16 u16Riu_tmp = 0;

//-------------------------------------------------------------------------------------------------
//  Local Functions
//-------------------------------------------------------------------------------------------------
uint32_t __IO32AccessFld(
	uint8_t write, uint32_t tmp32, uint32_t val32, uint32_t fld)
{
	uint32_t t = 0;

	switch (Fld_ac(fld)) {
	case AC_FULLB0:
	case AC_FULLB1:
	case AC_FULLB2:
	case AC_FULLB3:
		if (write == 1)
			t = (tmp32&(~((uint32_t)0xFF<<
				(8*(Fld_ac(fld)-AC_FULLB0))))) |
				((val32&0xFF)<<(8*(Fld_ac(fld)-AC_FULLB0)));
		else
			t = (tmp32&((uint32_t)0xFF<<
				(8*(Fld_ac(fld)-AC_FULLB0)))) >>
				(8*(Fld_ac(fld)-AC_FULLB0));
		break;
	case AC_FULLW10:
	case AC_FULLW21:
	case AC_FULLW32:
		if (write == 1)
			t = (tmp32&(~((uint32_t)0xFFFF<<
				(8*(Fld_ac(fld)-AC_FULLW10))))) |
				((val32&0xFFFF)<<(8*(Fld_ac(fld)-AC_FULLW10)));
		else
			t = (tmp32&(((uint32_t)0xFFFF<<
				(8*(Fld_ac(fld)-AC_FULLW10))))) >>
				(8*(Fld_ac(fld)-AC_FULLW10));
		break;
	case AC_FULLDW:
		t = val32;
		break;
	case AC_MSKB0:
	case AC_MSKB1:
	case AC_MSKB2:
	case AC_MSKB3:
		if (write == 1)
			t = (tmp32&(~(((((uint32_t)1<<Fld_wid(fld))-1)<<
				Fld_shft(fld))))) |
				(((val32&(((uint32_t)1<<Fld_wid(fld))-1))<<
				Fld_shft(fld)));
		else
			t = (tmp32&(((((uint32_t)1<<Fld_wid(fld))-1)<<
				Fld_shft(fld)))) >>
				Fld_shft(fld);
		break;
	case AC_MSKW10:
	case AC_MSKW21:
	case AC_MSKW32:
		if (write == 1)
			t = (tmp32&(~(((((uint32_t)1<<Fld_wid(fld))-1)<<
				Fld_shft(fld))))) |
				(((val32&(((uint32_t)1<<Fld_wid(fld))-1))<<
				Fld_shft(fld)));
		else
			t = (tmp32&(((((uint32_t)1<<Fld_wid(fld))-1)<<
				Fld_shft(fld)))) >>
				Fld_shft(fld);
		break;
	case AC_MSKDW:
		if (write == 1)
			t = (tmp32&(~(((((uint32_t)1<<Fld_wid(fld))-1)<<
				Fld_shft(fld))))) |
				(((val32&(((uint32_t)1<<Fld_wid(fld))-1))<<
				Fld_shft(fld)));
		else
			t = (tmp32&(((((uint32_t)1<<Fld_wid(fld))-1)<<
				Fld_shft(fld)))) >>
				Fld_shft(fld);
		break;
	default:
		break;
	}
	return t;
}

void mtk_write2byte(
	uint32_t u32P_Addr,
	uint32_t u32Value)
{
	writel(u32Value, (volatile void *)((uintptr_t)(u32P_Addr + REG_BASE)));
}

void mtk_write2bytemask(
	uint32_t u32P_Addr,
	uint32_t u32Value,
	uint32_t fld)
{
	uint32_t tmp = 0x00;

	tmp = __IO32AccessFld(
		1,
		readl((const volatile void *)((uintptr_t)(u32P_Addr + REG_BASE))),
		u32Value,
		fld);
	writel(tmp, (volatile void *)((uintptr_t)(u32P_Addr + REG_BASE)));
}

uint32_t mtk_read2byte(
	uint32_t u32P_Addr)
{
	return readl((const volatile void *)((uintptr_t)(u32P_Addr + REG_BASE)));
}

uint32_t mtk_read2bytemask(
	uint32_t u32P_Addr,
	uint32_t fld)
{
	uint32_t tmp;

	tmp = __IO32AccessFld(0,
	readl((const volatile void *)((uintptr_t)(u32P_Addr + REG_BASE))),
	0, fld);

	return tmp;
}

static void GE_DumpReg(GE_CTX_HAL_LOCAL *pGEHalLocal)
{
    MS_U32 i;

    GE_H_DBUG("Dump GE register:\n");
    for (i = 0; i < 0x80; i++)
    {
        if(i % 0x08 == 0) {
            GE_H_DBUG("    \n");
            GE_H_DBUG("h%02x    ", (MS_U8)i );
        }
        GE_H_DBUG("%04x ",  mtk_read2byte((i*4)+GE0_2410_BASE));
    }

    GE_H_DBUG("    \n");
}

static void GE_Reset(GE_CTX_HAL_LOCAL *pGEHalLocal)
{
    MS_U16 reg0, reg1;

    reg0 = mtk_read2byte(REG_0000_GE0);
    reg1 = mtk_read2byte(REG_0004_GE0);

    mtk_write2byte(REG_0000_GE0, 0);
    mtk_write2byte(REG_0004_GE0, 0);

    mtk_write2byte(REG_0000_GE0, reg0);
    mtk_write2byte(REG_0004_GE0, reg1);

}

void GE_WaitCmdQAvail(GE_CTX_HAL_LOCAL *pGEHalLocal, MS_U32 u32Count)
{
    #ifdef MS_DEBUG
    MS_U32 waitcount = 0;
    #endif
    MS_U16 tmp1 = 0;
    MS_U32 u32CmdMax;

    /// VCMQ enabled
    if (mtk_read2bytemask(REG_0004_GE0, REG_EN_GE_VCMQ) != 0)
    {
        // 16 Bytes one command in VCMDQ.
        u32CmdMax = (512 << (mtk_read2bytemask(REG_00A8_GE0, REG_GE_VCMQ_SIZE)));
        u32Count = MIN(u32CmdMax, u32Count);

        while (GE_CMDQ_FREECNT() < u32Count)
        {
            #ifdef MS_DEBUG
            if (waitcount >= 0x80000)
            {
                GE_H_DBUG("[GE] V0 Wait command queue: %d : %tx, %tx\n", tmp1, (ptrdiff_t)GE_CMDQ_FREECNT(), (ptrdiff_t)u32Count);
                waitcount = 0;
                tmp1++;
                if(tmp1 > 10)
                {
                    GE_DumpReg(pGEHalLocal);
                    GE_Reset(pGEHalLocal);
                }
            }
            waitcount++;
            #endif
            GE_YIELD();
        }
        tmp1 = 0;
        waitcount = 0;


        //If u32Count >= u32CmdMax, It will be dead loop. But since it won't happen, and if match
        //Full VCMDQ, hw will hang, so keep the logic.
        while ( (MS_U32)GE_VCMDQ_FREECNT() >= (MS_U32)(u32CmdMax- u32Count))
        {
            #ifdef MS_DEBUG
            if (waitcount >= 0x80000)
            {
                GE_H_DBUG("[GE] Wait VCMQ : %d : %tx, %tx\n", tmp1, (ptrdiff_t)GE_VCMDQ_FREECNT(), (ptrdiff_t)u32Count);
                waitcount = 0;
                tmp1++;
                if(tmp1 > 10)
                {
                    GE_DumpReg(pGEHalLocal);
                    GE_Reset(pGEHalLocal);
                }
            }
            waitcount++;
            #endif
            GE_YIELD();
        }
    }
    else
    {
        u32Count = MIN(GE_CMD_SIZE_MAX, u32Count);

        while (GE_CMDQ_FREECNT() < u32Count)
        {
            #ifdef MS_DEBUG
            if (waitcount >= 0x80000)
            {
                GE_H_DBUG("[GE] Wait command queue: %d : %tx, %tx\n", tmp1, (ptrdiff_t)GE_CMDQ_FREECNT(), (ptrdiff_t)u32Count);
                waitcount = 0;
                tmp1++;
                if(tmp1 > 10)
                {
                    GE_DumpReg(pGEHalLocal);
                    GE_Reset(pGEHalLocal);
                }
            }
            waitcount++;
            #endif
            GE_YIELD();
        }

    }

}


MS_U16 GE_CODA_ReadReg(GE_CTX_HAL_LOCAL *pGEHalLocal, MS_U32 addr)
{
	MS_U16 u16NoFIFOMask;

	switch (addr) {	//for registers which do not go through command queue
	case REG_0000_GE0://REG_GE_EN
		u16NoFIFOMask = GE_EN_GE;
		break;
	case REG_0008_GE0: //REG_GE_DEBUG
	case REG_000C_GE0: //REG_GE_TH
	case REG_0014_GE0: //REG_GE_BIST_STAT
	case REG_001C_GE0: //REG_GE_STAT
	case REG_0010_GE0: //REG_GE_VCMDQ_STAT
	case REG_00C8_GE0: //REG_GE_TAG
	case REG_00C4_GE0: //REG_GE_TAG_H
	case REG_00A0_GE0: //REG_GE_VCMDQ_BASE_L
	case REG_00A4_GE0: //REG_GE_VCMDQ_BASE_H
	case REG_0020_GE0: //REG_GE_MIU_PROT_LTH_L(0):
	case REG_0024_GE0: //REG_GE_MIU_PROT_LTH_H(0):
	case REG_0028_GE0: //REG_GE_MIU_PROT_HTH_L(0):
	case REG_002C_GE0: //REG_GE_MIU_PROT_HTH_H(0):
	case REG_0030_GE0: //REG_GE_MIU_PROT_LTH_L(1):
	case REG_0034_GE0: //REG_GE_MIU_PROT_LTH_H(1):
	case REG_0038_GE0: //REG_GE_MIU_PROT_HTH_L(1):
	case REG_003C_GE0: //REG_GE_MIU_PROT_HTH_H(1):
		u16NoFIFOMask = 0xffff;
		break;
	case REG_00A8_GE0: //REG_GE_VCMDQ_SIZE:
		u16NoFIFOMask = GE_VCMDQ_SIZE_MASK;
		break;
	case REG_008C_GE0: //REG_GE_BASE_MSB:
		u16NoFIFOMask = GE_VCMDQ_MSB_MASK;
		break;
	default:
		u16NoFIFOMask = 0;
		break;
	}

	if (u16NoFIFOMask == 0)
		return pGEHalLocal->u16RegGETable[(addr-GE0_2410_BASE)/4];

	return (mtk_read2byte(addr) & u16NoFIFOMask) |
		(pGEHalLocal->u16RegGETable[(addr-GE0_2410_BASE)/4] & ~u16NoFIFOMask);
}


void GE_CODA_WriteReg(GE_CTX_HAL_LOCAL *pGEHalLocal, MS_U32 addr, MS_U16 value)
{
	if ((addr-GE0_2410_BASE)/4 < GE_TABLE_REGNUM) {
		pGEHalLocal->u16RegGETable[(addr-GE0_2410_BASE)/4] = value;
	} else {
		GE_H_DBUG("[%s][%d] Reg Index [%d]is out of GE_TABLE_REGNUM [0x%lx]range!!!!\n",
			  __func__, __LINE__, addr, GE_TABLE_REGNUM);
	}

    GE_WaitCmdQAvail(pGEHalLocal, GE_CMD_SIZE);

    mtk_write2byte(addr, value);
}

void GE_Chip_Proprity_Init(GE_CTX_HAL_LOCAL *pGEHalLocal)
{
    pGEHalLocal->pGeChipPro = &g_GeChipPro;
}

GE_Result _GE_SetBltScaleRatio2HW(GE_CTX_HAL_LOCAL *pGEHalLocal, GE_ScaleInfo *pScaleinfo)
{
    MS_U16 u16RegVal;

    //check parameters
    if(pScaleinfo == NULL)
    {
        return E_GE_FAIL_PARAM;
    }
    GE_CODA_WriteReg(pGEHalLocal, REG_0190_GE0, (MS_U16) (pScaleinfo->x & 0xFFFF));
    GE_CODA_WriteReg(pGEHalLocal, REG_0194_GE0, (MS_U16) (pScaleinfo->y & 0xFFFF));
    //Set Initial DeltaX, DeltaY:
    GE_CODA_WriteReg(pGEHalLocal, REG_0178_GE0, (MS_U16) (pScaleinfo->init_x & 0xFFFF));
    GE_CODA_WriteReg(pGEHalLocal, REG_017C_GE0, (MS_U16) (pScaleinfo->init_y & 0xFFFF));

    //set MSBs of REG_GE_BLT_SRC_DY, REG_GE_BLT_SRC_DY:
    u16RegVal = GE_CODA_ReadReg(pGEHalLocal, REG_0178_GE0) & ~(GE_STBB_DX_MSB);
    u16RegVal |= (((pScaleinfo->x >> 16) << GE_STBB_DX_MSB_SHFT) & GE_STBB_DX_MSB);
    GE_CODA_WriteReg(pGEHalLocal, REG_0178_GE0, u16RegVal);

    u16RegVal = GE_CODA_ReadReg(pGEHalLocal, REG_017C_GE0) & ~(GE_STBB_DY_MSB);
    u16RegVal |= (((pScaleinfo->y >> 16) << GE_STBB_DY_MSB_SHFT) & GE_STBB_DY_MSB);
    GE_CODA_WriteReg(pGEHalLocal, REG_017C_GE0, u16RegVal);

    return E_GE_OK;
}

void GE_SetActiveCtrlMiu1(GE_CTX_HAL_LOCAL *pGEHalLocal)
{
    //MDrv_MIU_SelMIU(MIU_CLIENT_GE_RW,MIU_SELTYPE_MIU_ALL);
}

//-------------------------------------------------------------------------------------------------
//  Global Functions
//-------------------------------------------------------------------------------------------------

MS_U8  _GFXAPI_MIU_ID(MS_PHY ge_fbaddr)
{
    MS_U8 u8MIUSelTMP = 0;
    MS_PHY PhyOffset = 0;

    _phy_to_miu_offset(u8MIUSelTMP, PhyOffset, ge_fbaddr);
    UNUSED(PhyOffset);

    return u8MIUSelTMP;
}

MS_PHY _GFXAPI_PHYS_ADDR_IN_MIU(MS_PHY ge_fbaddr)
{
    MS_U8 u8MIUSelTMP = 0;
    MS_PHY PhyOffset = 0;

    _phy_to_miu_offset(u8MIUSelTMP, PhyOffset, ge_fbaddr);
    UNUSED(u8MIUSelTMP);

    return PhyOffset;
}

#if (__GE_WAIT_TAG_MODE == __USE_GE_INT_MODE)
void _GE_WaitTag_InterruptCbk(InterruptNum eIntNum)
{
#if defined(MSOS_TYPE_LINUX)
    MS_S32 s32CurPid = (MS_S32)getpid();
#endif

#if defined(MSOS_TYPE_LINUX)
    if (s32WaitingTagPid == s32CurPid)
#endif
    {
        (*((volatile MS_U16 *)(virtHalIomapBaseAddr + GE_BANK_NUM*2 + ((REG_GE_SRCMASK_GB)<<2) ))) = 0xE0;

        if (s32GeWaitTagEventHandle > 0)
        {
            if (FALSE == MsOS_SetEvent(s32GeWaitTagEventHandle, 0x1))
            {
                GE_DBG("[%s, %d]:  MsOS_ReleaseSemaphore failed\r\n", __FUNCTION__, __LINE__);
            }
        }
    }

    MsOS_EnableInterrupt(E_INT_IRQ_GE);
#ifdef MSOS_TYPE_LINUX
    MsOS_CompleteInterrupt(E_INT_IRQ_GE);
#endif
}

static GE_Result _GE_Ctrl_IntMode(GE_CTX_HAL_LOCAL *pGEHalLocal, E_GE_INT_OP int_op)
{
    MS_U16  u16IntReg=0;

    u16IntReg = GE_ReadReg(pGEHalLocal, REG_GE_SRCMASK_GB);

    if (E_GE_CLEAR_INT & int_op)
    {
        u16IntReg |= GE_INT_MODE_CLEAR;
        GE_WriteReg(pGEHalLocal, REG_GE_SRCMASK_GB, u16IntReg);
        u16IntReg &= (~GE_INT_MODE_CLEAR);
    }

    if (E_GE_MASK_INT & int_op)
    {
        u16IntReg |= GE_INT_TAG_MASK;
    }

    if (E_GE_UNMASK_INT & int_op)
    {
        u16IntReg &= (~GE_INT_TAG_MASK);
    }

    if (E_GE_INT_TAG_MODE & int_op)
    {
        u16IntReg |= GE_INT_TAG_MODE;
    }

    if (E_GE_INT_NORMAL_MODE & int_op)
    {
        u16IntReg &= (~GE_INT_TAG_MODE);
    }

    GE_WriteReg(pGEHalLocal, REG_GE_SRCMASK_GB, u16IntReg);

    return E_GE_OK;
}

static MS_BOOL _GE_IsTagInterruptEnabled(GE_CTX_HAL_LOCAL *pGEHalLocal)
{
    MS_BOOL bret = FALSE;
    MS_U16  u16IntReg=0;

    u16IntReg = GE_ReadReg(pGEHalLocal, REG_GE_SRCMASK_GB);

    bret = FALSE;
    if ((GE_INT_TAG_MODE & u16IntReg) > 0)
    {
        bret = TRUE;
    }

    return bret;
}

static void _GE_Print_GeWaitTagTimeout_Msg(GE_CTX_HAL_LOCAL *pGEHalLocal, MS_U16 tagID)
{
    MS_U16 tmp_reg = 0;

    GE_DBG("[%s, %d]: >>>>>>>>>>>>>>>>>> ge wait event timeout <<<<<<<<<<<<<<<<<<<\r\n", __FUNCTION__, __LINE__);
    GE_DBG("[%s, %d]: current proc id = %td \r\n", __FUNCTION__, __LINE__, (ptrdiff_t)s32WaitingTagPid);

    tmp_reg = GE_ReadReg(pGEHalLocal, REG_GE_SRCMASK_GB);
    GE_DBG("[%s, %d]: ge int status = 0x%x \r\n", __FUNCTION__, __LINE__, tmp_reg);

    tmp_reg = INTR_CTNL_BK(0x56);
    GE_DBG("[%s, %d]: cpu int mask = 0x%x \r\n", __FUNCTION__, __LINE__, tmp_reg);
    tmp_reg = INTR_CTNL_BK(0x5E);
    GE_DBG("[%s, %d]: cpu int status = 0x%x \r\n", __FUNCTION__, __LINE__, tmp_reg);

    tmp_reg = GE_ReadReg(pGEHalLocal, REG_GE_INT_TAG_COND_L);
    GE_DBG("[%s, %d]: int_tag = 0x%x \r\n", __FUNCTION__, __LINE__, tmp_reg);

    tmp_reg = GE_ReadReg(pGEHalLocal, REG_GE_TAG);
    GE_DBG("[%s, %d]: tag = 0x%x \r\n", __FUNCTION__, __LINE__, tmp_reg);

    GE_DBG("[%s, %d]: tagID = 0x%x \r\n", __FUNCTION__, __LINE__, tagID);
}

#endif

static MS_U8 GE_MapVQsize2Reg(MS_U32 u32BufSize)
{
    switch(u32BufSize)
    {
        case 0x1000:
            return GE_VQ_4K;
        case 0x2000:
            return GE_VQ_8K;
        case 0x4000:
            return GE_VQ_16K;
        case 0x8000:
            return GE_VQ_32K;
        case 0x10000:
            return GE_VQ_64K;
        case 0x20000:
            return GE_VQ_128K;
        case 0x40000:
            return GE_VQ_256K;
        case 0x80000:
            return GE_VQ_512K;
        case 0x100000:
            return GE_VQ_1024K;
        case 0x200000:
            return GE_VQ_2048K;
        default:
            return 0;
    }
}

static MS_U8 GE_MapVQ2Reg(GE_VcmqBufSize enBufSize)
{
    switch(enBufSize)
    {
        case E_GE_VCMD_4K:
            return GE_VQ_8K;
        case E_GE_VCMD_8K:
            return GE_VQ_8K;
        case E_GE_VCMD_16K:
            return GE_VQ_16K;
        case E_GE_VCMD_32K:
            return GE_VQ_32K;
        case E_GE_VCMD_64K:
            return GE_VQ_64K;
        case E_GE_VCMD_128K:
            return GE_VQ_128K;
        case E_GE_VCMD_256K:
            return GE_VQ_256K;
        case E_GE_VCMD_512K:
            return GE_VQ_512K;
        case E_GE_VCMD_1024K:
            return GE_VQ_1024K;
        default:
            return 0;
    }
}

MS_PHY GE_ConvertAPIAddr2HAL(GE_CTX_HAL_LOCAL *pGEHalLocal, MS_U8 u8MIUId, MS_PHY PhyGE_APIAddrInMIU)
{
     return PhyGE_APIAddrInMIU;
}

void GE_WaitIdle(GE_CTX_HAL_LOCAL *pGEHalLocal)
{
    #ifdef MS_DEBUG
    MS_U32 waitcount = 0;
    #endif
    MS_U16 tmp1 = 0;
    MS_U8  i = 0;

    for(i=0;i<(GE_WordUnit>>2);i++)
    {
        GE_CODA_WriteReg(pGEHalLocal, REG_00C4_GE0, 0);
        GE_CODA_WriteReg(pGEHalLocal, REG_00C8_GE0,
			GE_GetNextTAGID(pGEHalLocal, FALSE)); //dummy
    }
                                // GE will pack 2 register commands before CMDQ
                                // We need to push fifo if there is one command in the fifo before
                                // CMDQ. Then the GE status register will be consistant after idle.
    GE_WaitCmdQAvail(pGEHalLocal, GE_STAT_CMDQ_MAX); // Wait CMDQ empty


    // Wait level-2 command queue flush
    while (mtk_read2bytemask(REG_001C_GE0, REG_GE_CMQ1_STATUS) != GE_STAT_CMDQ2_MAX)
    {
        #ifdef MS_DEBUG
        if (waitcount >= 0x80000)
        {
            GE_H_DBUG("[GE] Wait Idle: %u : %tx\n", tmp1, (ptrdiff_t)GE_CMDQ_FREECNT());
            waitcount = 0;
            tmp1++;
            if(tmp1 > 10)
            {
                GE_DumpReg(pGEHalLocal);
                GE_Reset(pGEHalLocal);
            }
        }
        waitcount++;
        #endif

        GE_YIELD();
    }


#ifdef MS_DEBUG
    waitcount = 0;
    tmp1 = 0;
#endif

    // Wait GE idle
    while (mtk_read2bytemask(REG_001C_GE0, REG_GE_BUSY))
    {
        #ifdef MS_DEBUG
        if (waitcount >= 0x80000)
        {
            GE_H_DBUG("[GE] Wait Busy: %u : %tx\n", tmp1, (ptrdiff_t)GE_CMDQ_FREECNT());
            waitcount = 0;
            tmp1++;
            if(tmp1 > 10)
            {
                GE_DumpReg(pGEHalLocal);
                GE_Reset(pGEHalLocal);
            }
        }
        waitcount++;
        #endif

        GE_YIELD();
    }


}

GE_Result GE_Map_Share_Reg(GE_CTX_HAL_LOCAL *pGEHalLocal, MS_U16 addr)
{
    if(addr == REG_GE_CFG)
        return E_GE_OK;
    else
        return E_GE_FAIL;
#if 0
        switch(addr)
        {
            case REG_GE_CFG:
                 return E_GE_OK;
            default:
                return E_GE_FAIL;
        }
#endif

}

GE_Result GE_Map_Share_RegEX(GE_CTX_HAL_LOCAL *pGEHalLocal, MS_U16 addr)
{
    return E_GE_FAIL;
}

MS_U16 GE_ReadReg(GE_CTX_HAL_LOCAL *pGEHalLocal, MS_U16 addr)
{
    return 0;
}

void GE_WriteReg(GE_CTX_HAL_LOCAL *pGEHalLocal, MS_U16 addr, MS_U16 value)
{
	return;
}

MS_U16 GE2_ReadReg(GE_CTX_HAL_LOCAL *pGEHalLocal, MS_U16 addr)
{
    MS_U16 u16NoFIFOMask=0;

    if(GE_TABLE_REGNUM <= addr)
    {
        GE_WaitIdle(pGEHalLocal);
        return GE2_REG(addr-GE_TABLE_REGNUM);
    }

    if(0 == u16NoFIFOMask)
    {
        if(GE_Map_Share_RegEX(pGEHalLocal,addr)== E_GE_OK)
            return pGEHalLocal->pHALShared->u16ShareRegImageEx[addr];
        else
        {
            if(pGEHalLocal->pHALShared->bGE_DirectToReg ==TRUE)
            {
                return (GE_REG(addr)|pGEHalLocal->u16RegGETable[addr]);
            }
            else
            {
            return pGEHalLocal->u16RegGETableEX[addr];
        }
    }
    }
    return (GE2_REG(addr)&u16NoFIFOMask)|(pGEHalLocal->u16RegGETableEX[addr]&~u16NoFIFOMask);

}

void GE2_WriteReg(GE_CTX_HAL_LOCAL *pGEHalLocal, MS_U16 addr, MS_U16 value)
{
    // CMDQ special command
    if(addr < GE_TABLE_REGNUM)
    {
        if(GE_Map_Share_RegEX(pGEHalLocal,addr)== E_GE_OK)
        {
            pGEHalLocal->pHALShared->u16ShareRegImageEx[addr]= value;
        }
        pGEHalLocal->u16RegGETableEX[addr] = value;
    }
    else
    {
        GE_H_DBUG("[%s][%d] Reg Index [%x]is out of GE_TABLE_REGNUM [0x%lx]range!!!!\n",__FUNCTION__,__LINE__, addr, GE_TABLE_REGNUM);
    }
    GE_WaitCmdQAvail(pGEHalLocal, GE_CMD_SIZE);
    GE2_REG(addr)= value;
    return;
}


void GE_RestoreReg(GE_CTX_HAL_LOCAL *pGEHalLocal, MS_U16 addr, MS_U16 value)
{
    // CMDQ special command
    switch (addr)
    {
    case REG_GE_CMD:
        break;
    //[OBSOLETE]
    default:
        GE_WriteReg(pGEHalLocal, addr, value);
        break;
    }
}


void GE_ResetState(GE_CTX_HAL_LOCAL *pGEHalLocal)
{
    GE_WaitIdle(pGEHalLocal);

    GE_CODA_WriteReg(pGEHalLocal, REG_0000_GE0, GE_EN_GE);

    GE_CODA_WriteReg(pGEHalLocal, REG_000C_GE0, 0x0000);

    GE_CODA_WriteReg(pGEHalLocal, REG_0188_GE0, GE_LINEPAT_RST);
    GE_CODA_WriteReg(pGEHalLocal, REG_0164_GE0, GE_BLT_SCK_NEAREST);
    GE_CODA_WriteReg(pGEHalLocal, REG_0044_GE0, GE_ALPHA_ARGB1555);
}


void GE_Init_RegImage(GE_CTX_HAL_LOCAL *pGEHalLocal)
{
    MS_U8 addr;

    for(addr = 0; addr<GE_TABLE_REGNUM; addr++)
    {
        pGEHalLocal->u16RegGETable[addr] = mtk_read2byte((addr*4)+GE0_2410_BASE);
    }

}

void GE_Init(GE_CTX_HAL_LOCAL *pGEHalLocal, GE_Config *cfg)
{
    MS_U16 u16temp =0;

    GE_SetClock(pGEHalLocal,TRUE);

    GE_WaitIdle(pGEHalLocal);

    u16temp = GE_CODA_ReadReg(pGEHalLocal, REG_0004_GE0);

    if ((u16temp & BIT(1)) != BIT(1)) //if VQ is Not Enabled
    {
        // Set default FMT for avoiding 1st set buffinfo error.
        GE_CODA_WriteReg(pGEHalLocal, REG_00D0_GE0, (GE_FMT_ARGB1555<<GE_SRC_FMT_SHFT)+(GE_FMT_ARGB1555<<GE_DST_FMT_SHFT));

        if (cfg->u32VCmdQSize >= GE_VCMDQ_SIZE_MIN)
        {
            MS_PHY PhyVQAddr = cfg->PhyVCmdQAddr;

            HAL_GE_SetBufferAddr(pGEHalLocal,PhyVQAddr,E_GE_BUF_VCMDQ);
            GE_CODA_WriteReg(pGEHalLocal, REG_00A8_GE0,  GE_MapVQsize2Reg((GE_VcmqBufSize)cfg->u32VCmdQSize));
            GE_CODA_WriteReg(pGEHalLocal, REG_0004_GE0, GE_CFG_CMDQ | GE_CFG_VCMDQ);
        }
        else
        {
            //GE_H_DBUG("[%s][%d]VCmdQSize = 0x%tx < GE_VCMDQ_SIZE_MIN = 0x%tx\n",__func__,__LINE__,(ptrdiff_t)cfg->u32VCmdQSize,(ptrdiff_t)GE_VCMDQ_SIZE_MIN);
        }

        GE_ResetState(pGEHalLocal);
    }
    else
    {
        //No need to set command queue
        GE_H_DBUG(" warning!!! Virtual Command queue has been activated!! \n");
    }
    GE_Init_RegImage(pGEHalLocal);

    u16temp = GE_CODA_ReadReg(pGEHalLocal, REG_000C_GE0);
    u16temp &= ~(GE_TH_STBB_MASK);
    GE_CODA_WriteReg(pGEHalLocal, REG_000C_GE0, (u16temp | GE_THRESHOLD_SETTING));
    //Mask Interrupt
    GE_CODA_WriteReg(pGEHalLocal, REG_0078_GE0, 0x00C0);

#if (__GE_WAIT_TAG_MODE == __USE_GE_INT_MODE)

    if (FALSE == bGeIrqInited)
    {
        if (FALSE == _GE_IsTagInterruptEnabled(pGEHalLocal))
        {
            _GE_Ctrl_IntMode(pGEHalLocal, E_GE_INT_TAG_MODE | E_GE_MASK_INT);
        }

        if (FALSE == MsOS_AttachInterrupt(E_INT_IRQ_GE, _GE_WaitTag_InterruptCbk))
        {
            GE_DBG("[%s, %d]: MsOS_AttachInterrupt failed \r\n", __FUNCTION__, __LINE__);
            return ;
        }
        MsOS_EnableInterrupt(E_INT_IRQ_GE);

        bGeIrqInited = TRUE;
    }

    if (s32GeWaitTagEventHandle < 0)
    {
        s32GeWaitTagEventHandle = MsOS_CreateEventGroup("GE_Wait_Event_Handle");
        if (s32GeWaitTagEventHandle < 0)
        {
            GE_DBG("[%s, %d]: ge_semid_waitTag = %td", __FUNCTION__, __LINE__, (ptrdiff_t)s32GeWaitTagEventHandle);
            return;
        }
    }
#endif

}

GE_Result GE_SetRotate(GE_CTX_HAL_LOCAL *pGEHalLocal, GE_RotateAngle geRotAngle)
{
    MS_U16 u16RegVal;

    u16RegVal = (GE_CODA_ReadReg(pGEHalLocal, REG_0164_GE0) & ~REG_GE_ROT_MODE_MASK) | (geRotAngle<<REG_GE_ROT_MODE_SHFT);
    GE_CODA_WriteReg(pGEHalLocal, REG_0164_GE0, u16RegVal);

    return E_GE_OK;
}

GE_Result GE_SetOnePixelMode(GE_CTX_HAL_LOCAL *pGEHalLocal, MS_BOOL enable)
{

    MS_U16    u16en;
    //GE_DBG("%s\n", __FUNCTION__);

    u16en = GE_CODA_ReadReg(pGEHalLocal, REG_0000_GE0);
    if (enable)
    {
        u16en |= GE_EN_ONE_PIXEL_MODE;
    }
    else
    {
        u16en &= (~GE_EN_ONE_PIXEL_MODE);
    }
    u16en |= GE_EN_BURST;
    GE_CODA_WriteReg(pGEHalLocal, REG_0000_GE0, u16en);

    return E_GE_OK;
}

GE_Result GE_SetBlend(GE_CTX_HAL_LOCAL *pGEHalLocal, GE_BlendOp eBlendOp)
{
    MS_U16              u16op;

    switch (eBlendOp)
    {
    case E_GE_BLEND_ONE:
    case E_GE_BLEND_CONST:
    case E_GE_BLEND_ASRC:
    case E_GE_BLEND_ADST:
    case E_GE_BLEND_ROP8_ALPHA:
    case E_GE_BLEND_ROP8_SRCOVER:
    case E_GE_BLEND_ROP8_DSTOVER:
    case E_GE_BLEND_ZERO:
    case E_GE_BLEND_CONST_INV:
    case E_GE_BLEND_ASRC_INV:
    case E_GE_BLEND_ADST_INV:
    case E_GE_BLEND_ALPHA_ADST:
    case E_GE_BLEND_SRC_ATOP_DST:
    case E_GE_BLEND_DST_ATOP_SRC:
    case E_GE_BLEND_SRC_XOR_DST:
    case E_GE_BLEND_INV_CONST:
    case E_GE_BLEND_FADEIN:
    case E_GE_BLEND_FADEOUT:

        u16op = eBlendOp;
        break;
    default:
        return E_GE_FAIL_PARAM;
        break;
    }

    u16op = (GE_ReadReg(pGEHalLocal, REG_GE_BLEND) & ~GE_BLEND_MASK) | u16op;
    GE_WriteReg(pGEHalLocal, REG_GE_BLEND, u16op);

    return E_GE_OK;
}


GE_Result GE_SetAlpha(GE_CTX_HAL_LOCAL *pGEHalLocal, GE_AlphaSrc eAlphaSrc)
{
    MS_U16              u16src;

    switch (eAlphaSrc)
    {
    case E_GE_ALPHA_CONST:
    case E_GE_ALPHA_ASRC:
    case E_GE_ALPHA_ADST:
    case E_GE_ALPHA_ROP8_SRC:
    case E_GE_ALPHA_ROP8_IN:
    case E_GE_ALPHA_ROP8_DSTOUT:
    case E_GE_ALPHA_ROP8_SRCOUT:
    case E_GE_ALPHA_ROP8_OVER:
    case E_GE_ALPHA_ROP8_INV_CONST:
    case E_GE_ALPHA_ROP8_INV_ASRC:
    case E_GE_ALPHA_ROP8_INV_ADST:
    case E_GE_ALPHA_ROP8_SRC_ATOP_DST:
    case E_GE_ALPHA_ROP8_DST_ATOP_SRC:
    case E_GE_ALPHA_ROP8_SRC_XOR_DST:
    case E_GE_ALPHA_ROP8_INV_SRC_ATOP_DST:
    case E_GE_ALPHA_ROP8_INV_DST_ATOP_SRC:

        u16src = eAlphaSrc;
        break;
    default:
        return E_GE_FAIL_PARAM;
        break;
    }

    u16src = (GE_ReadReg(pGEHalLocal, REG_GE_ALPHA) & ~GE_ALPHA_MASK) | (u16src<<GE_ALPHA_SHFT);
    GE_WriteReg(pGEHalLocal, REG_GE_ALPHA, u16src);

    return E_GE_OK;
}

GE_Result   GE_QueryDFBBldCaps(GE_CTX_HAL_LOCAL *pGEHalLocal, MS_U16 *pU16SupportedBldFlags)
{
    if(NULL == pU16SupportedBldFlags)
    {
        return E_GE_FAIL_PARAM;
    }

    (*pU16SupportedBldFlags) = E_GE_DFB_BLD_FLAG_ALL;

    return E_GE_OK;
}

GE_Result   GE_EnableDFBBld(GE_CTX_HAL_LOCAL *pGEHalLocal, MS_BOOL enable)
{
    MS_U16 u16RegVal;

    u16RegVal = GE_ReadReg(pGEHalLocal, REG_GE_EN);

    if (enable)
    {
        u16RegVal |= GE_EN_DFB_BLD;
    }
    else
    {
        u16RegVal &= ~GE_EN_DFB_BLD;
    }

    GE_WriteReg(pGEHalLocal, REG_GE_EN, u16RegVal);

    return E_GE_OK;
}

GE_Result   GE_SetDFBBldFlags(GE_CTX_HAL_LOCAL *pGEHalLocal, MS_U16 u16DFBBldFlags)
{
    MS_U16 u16RegVal;

    u16RegVal = (GE_ReadReg(pGEHalLocal, REG_GE_DFB_BLD_FLAGS) & ~GE_DFB_BLD_FLAGS_MASK);

    if(u16DFBBldFlags & E_GE_DFB_BLD_FLAG_COLORALPHA)
    {
        u16RegVal |= GE_DFB_BLD_FLAG_COLORALPHA;
    }

    if(u16DFBBldFlags & E_GE_DFB_BLD_FLAG_ALPHACHANNEL)
    {
        u16RegVal |= GE_DFB_BLD_FLAG_ALPHACHANNEL;
    }

    if(u16DFBBldFlags & E_GE_DFB_BLD_FLAG_COLORIZE)
    {
        u16RegVal |= GE_DFB_BLD_FLAG_COLORIZE;
    }

    if(u16DFBBldFlags & E_GE_DFB_BLD_FLAG_SRCPREMUL)
    {
        u16RegVal |= GE_DFB_BLD_FLAG_SRCPREMUL;
    }

    if(u16DFBBldFlags & E_GE_DFB_BLD_FLAG_SRCPREMULCOL)
    {
        u16RegVal |= GE_DFB_BLD_FLAG_SRCPREMULCOL;
    }

    if(u16DFBBldFlags & E_GE_DFB_BLD_FLAG_DSTPREMUL)
    {
        u16RegVal |= GE_DFB_BLD_FLAG_DSTPREMUL;
    }

    if(u16DFBBldFlags & E_GE_DFB_BLD_FLAG_XOR)
    {
        u16RegVal |= GE_DFB_BLD_FLAG_XOR;
    }

    if(u16DFBBldFlags & E_GE_DFB_BLD_FLAG_DEMULTIPLY)
    {
        u16RegVal |= GE_DFB_BLD_FLAG_DEMULTIPLY;
    }

    GE_WriteReg(pGEHalLocal, REG_GE_DFB_BLD_FLAGS, u16RegVal);


    u16RegVal = (GE_ReadReg(pGEHalLocal, REG_GE_DFB_BLD_OP) & ~GE_DFB_SRC_COLORMASK);

    if(u16DFBBldFlags & (E_GE_DFB_BLD_FLAG_SRCCOLORMASK | E_GE_DFB_BLD_FLAG_SRCALPHAMASK))
    {
        u16RegVal |= (1 << GE_DFB_SRC_COLORMASK_SHIFT);
    }



    return E_GE_OK;
}

GE_Result   GE_SetDFBBldOP(GE_CTX_HAL_LOCAL *pGEHalLocal, GE_DFBBldOP geSrcBldOP, GE_DFBBldOP geDstBldOP)
{
    MS_U16 u16RegVal;

    u16RegVal = (GE_ReadReg(pGEHalLocal, REG_GE_DFB_BLD_OP) & ~(GE_DFB_SRCBLD_OP_MASK|GE_DFB_DSTBLD_OP_MASK));
    u16RegVal |= ((geSrcBldOP<<GE_DFB_SRCBLD_OP_SHFT) | (geDstBldOP<<GE_DFB_DSTBLD_OP_SHFT));

    GE_WriteReg(pGEHalLocal, REG_GE_DFB_BLD_OP, u16RegVal);

    return E_GE_OK;
}

GE_Result   GE_SetDFBBldConstColor(GE_CTX_HAL_LOCAL *pGEHalLocal, GE_RgbColor geRgbColor)
{
    MS_U16 u16RegVal;

    u16RegVal = ((GE_ReadReg(pGEHalLocal, REG_GE_ALPHA_CONST) & ~GE_ALPHA_CONST_MASK) | (geRgbColor.a & 0xFF));
    GE_WriteReg(pGEHalLocal, REG_GE_ALPHA_CONST, u16RegVal);

    u16RegVal = ((GE_ReadReg(pGEHalLocal, _REG_GE_R_CONST) & ~GE_R_CONST_MASK) | ((geRgbColor.r<<GE_R_CONST_SHIFT) & GE_R_CONST_MASK));
    GE_WriteReg(pGEHalLocal, _REG_GE_R_CONST, u16RegVal);

    u16RegVal = ((GE_ReadReg(pGEHalLocal, _REG_GE_G_CONST) & ~GE_G_CONST_MASK) | ((geRgbColor.g<<GE_G_CONST_SHIFT) & GE_G_CONST_MASK));
    GE_WriteReg(pGEHalLocal, _REG_GE_G_CONST, u16RegVal);

    u16RegVal = ((GE_ReadReg(pGEHalLocal, _REG_GE_B_CONST) & ~GE_B_CONST_MASK) | ((geRgbColor.b<<GE_B_CONST_SHIFT) & GE_B_CONST_MASK));
    GE_WriteReg(pGEHalLocal, _REG_GE_B_CONST, u16RegVal);

    return E_GE_OK;
}

GE_Result   GE_SetDFBBldSrcColorMask(GE_CTX_HAL_LOCAL *pGEHalLocal, GE_RgbColor geRgbColor)
{
//no more hw function
/*
    MS_U16 u16RegVal;

    u16RegVal = ((GE_ReadReg(pGEHalLocal, REG_GE_ALPHA_CONST) & ~GE_ALPHA_SRCMASK_MASK) | (geRgbColor.a & 0xFF));
    GE_WriteReg(pGEHalLocal, REG_GE_ALPHA_CONST, u16RegVal);

    u16RegVal = ((GE_ReadReg(pGEHalLocal, REG_GE_OP_MODE) & ~GE_SRCCOLOR_MASK_R) | ((geRgbColor.r<<GE_SRCCOLOR_MASK_R_SHIFT) & GE_SRCCOLOR_MASK_R));
    GE_WriteReg(pGEHalLocal, REG_GE_OP_MODE, u16RegVal);

    u16RegVal = (geRgbColor.g<<GE_SRCCOLOR_MASK_G_SHIFT) | (geRgbColor.b<<GE_SRCCOLOR_MASK_B_SHIFT);
    GE_WriteReg(pGEHalLocal, REG_GE_SRCMASK_GB, u16RegVal);
*/
    return E_GE_OK;
}


GE_Result GE_WriteProtect(GE_CTX_HAL_LOCAL *pGEHalLocal, MS_U8 miu, MS_PHY addr_low, MS_PHY addr_high, GE_WPType eWPType)
{
    MS_U16              u16cfg;

    if (miu > 1)
    {
        return E_GE_FAIL;
    }

    if ( (eWPType == E_GE_WP_IN_RANGE) || (eWPType == E_GE_WP_OUT_RANGE) )
    {
        // range setting
        GE_WriteReg(pGEHalLocal, REG_GE_MIU_PROT_LTH_L(miu),  addr_low & (GE_MIU_ADDR_MASK&0xFFFF));
        GE_WriteReg(pGEHalLocal, REG_GE_MIU_PROT_LTH_H(miu), ((addr_low>>16) & (GE_MIU_ADDR_MASK>>16)) | (eWPType<<GE_MIU_PROT_MODE_SHFT));
        GE_WriteReg(pGEHalLocal, REG_GE_MIU_PROT_HTH_L(miu), addr_high & (GE_MIU_ADDR_MASK&0xFFFF));
        GE_WriteReg(pGEHalLocal, REG_GE_MIU_PROT_HTH_H(miu), (addr_high>>16) & (GE_MIU_ADDR_MASK>>16));
        // enable setting
        u16cfg = GE_ReadReg(pGEHalLocal, REG_GE_CFG) | (GE_CFG_MIU0_PROT << miu);
        GE_WriteReg(pGEHalLocal, REG_GE_CFG, u16cfg);
    }
    else if (eWPType == E_GE_WP_DISABLE)
    {
        u16cfg = GE_ReadReg(pGEHalLocal, REG_GE_CFG) & ~(GE_CFG_MIU0_PROT<<miu);
        GE_WriteReg(pGEHalLocal, REG_GE_CFG, u16cfg);
    }
    else
    {
        return E_GE_FAIL;
    }

    return E_GE_OK;
}


GE_Result GE_SetSrcTile(GE_CTX_HAL_LOCAL *pGEHalLocal, MS_BOOL tile)
{
    //GE_DBG("%s\n", __FUNCTION__);

    return E_GE_NOT_SUPPORT;

}


GE_Result GE_SetDstTile(GE_CTX_HAL_LOCAL *pGEHalLocal, MS_BOOL tile)
{
    //GE_DBG("%s\n", __FUNCTION__);

    return E_GE_NOT_SUPPORT;

}
GE_Result GE_SetASCK(GE_CTX_HAL_LOCAL *pGEHalLocal, MS_BOOL enable)
{
    MS_U16              u16cfg;

    u16cfg = GE_ReadReg(pGEHalLocal, REG_GE_EN);
    if (enable)
    {
        u16cfg |= GE_EN_ASCK;
    }
    else
    {
        u16cfg &= ~GE_EN_ASCK;
    }
    GE_WriteReg(pGEHalLocal, REG_GE_EN, u16cfg);

    return E_GE_OK;
}
GE_Result GE_SetADCK(GE_CTX_HAL_LOCAL *pGEHalLocal, MS_BOOL enable)
{
    MS_U16              u16cfg;

    u16cfg = GE_ReadReg(pGEHalLocal, REG_GE_EN);
    if (enable)
    {
        u16cfg |= GE_EN_DSCK;
    }
    else
    {
        u16cfg &= ~GE_EN_DSCK;
    }
    GE_WriteReg(pGEHalLocal, REG_GE_EN, u16cfg);

    return E_GE_OK;
}


GE_Result GE_GetFmtCaps(GE_CTX_HAL_LOCAL *pGEHalLocal, GE_BufFmt fmt, GE_BufType type, GE_FmtCaps *caps)
{
    static const MS_U8 _u8GETileWidth[E_GE_FMT_RGB332+1] = {8, 4, 2, 0, 1, 1};

    caps->fmt = fmt;
    if (type == E_GE_BUF_SRC)
    {
        switch (fmt)
        {
        case E_GE_FMT_I1:
        case E_GE_FMT_I2:
        case E_GE_FMT_I4:
        case E_GE_FMT_I8:
        case E_GE_FMT_RGB332:
            caps->u8BaseAlign = 1;
            caps->u8PitchAlign = 1;
            caps->u8Non1pAlign = 0;
            caps->u8HeightAlign = 1;
            caps->u8StretchAlign = 1;
            caps->u8TileBaseAlign = 0x80;//[HWBUG] 8;
            caps->u8TileWidthAlign = _u8GETileWidth[fmt];
            caps->u8TileHeightAlign = 16;
            break;
        case E_GE_FMT_RGB565:
        case E_GE_FMT_RGBA5551:
        case E_GE_FMT_RGBA4444:
        case E_GE_FMT_ARGB1555:
        case E_GE_FMT_1ABFgBg12355:
        case E_GE_FMT_ARGB4444:
        case E_GE_FMT_YUV422:
        case E_GE_FMT_FaBaFgBg2266:
        case E_GE_FMT_ABGR1555:
        case E_GE_FMT_BGRA5551:
        case E_GE_FMT_ABGR4444:
        case E_GE_FMT_BGRA4444:
        case E_GE_FMT_BGR565:
            caps->u8BaseAlign = 2;
            caps->u8PitchAlign = 2;
            caps->u8Non1pAlign = 0;
            caps->u8HeightAlign = 1;
            caps->u8StretchAlign = 2;
            caps->u8TileBaseAlign = 0x80;//[HWBUG] 8;
            caps->u8TileWidthAlign = 16;
            caps->u8TileHeightAlign = 16;
            break;
        case E_GE_FMT_ABGR8888:
        case E_GE_FMT_ARGB8888:
        case E_GE_FMT_RGBA8888:
        case E_GE_FMT_BGRA8888:
        case E_GE_FMT_ACRYCB444:
        case E_GE_FMT_CRYCBA444:
        case E_GE_FMT_ACBYCR444:
        case E_GE_FMT_CBYCRA444:
            caps->u8BaseAlign = 4;
            caps->u8PitchAlign = 4;
            caps->u8Non1pAlign = 0;
            caps->u8HeightAlign = 1;
            caps->u8StretchAlign = 4;
            caps->u8TileBaseAlign = 0x80;//[HWBUG] 8;
            caps->u8TileWidthAlign = 8;
            caps->u8TileHeightAlign = 16;
            break;
        // Not Support
        default:
            caps->fmt = E_GE_FMT_GENERIC;
            caps->u8BaseAlign = 4;
            caps->u8PitchAlign = 4;
            caps->u8Non1pAlign = 0;
            caps->u8HeightAlign = 1;
            caps->u8StretchAlign = 4;
            caps->u8TileBaseAlign = 0;
            caps->u8TileWidthAlign = 0;
            caps->u8TileHeightAlign = 0;
            return E_GE_FAIL_FORMAT;
        }
    }
    else
    {
        switch (fmt)
        {
        case E_GE_FMT_I8:
        case E_GE_FMT_RGB332:
            caps->u8BaseAlign = 1;
            caps->u8PitchAlign = 1;
            caps->u8Non1pAlign = 0;
            caps->u8HeightAlign = 1;
            caps->u8StretchAlign = 1;
            caps->u8TileBaseAlign = 8;
            caps->u8TileWidthAlign = _u8GETileWidth[fmt];
            caps->u8TileHeightAlign = 16;
            break;
        case E_GE_FMT_RGB565:
        case E_GE_FMT_ARGB1555:
        case E_GE_FMT_RGBA5551:
        case E_GE_FMT_RGBA4444:
        case E_GE_FMT_1ABFgBg12355:
        case E_GE_FMT_ARGB4444:
        case E_GE_FMT_YUV422:
        case E_GE_FMT_FaBaFgBg2266:
        case E_GE_FMT_ARGB1555_DST:
        case E_GE_FMT_ABGR1555:
        case E_GE_FMT_BGRA5551:
        case E_GE_FMT_ABGR4444:
        case E_GE_FMT_BGRA4444:
        case E_GE_FMT_BGR565:
            caps->u8BaseAlign = 2;
            caps->u8PitchAlign = 2;
            caps->u8Non1pAlign = 0;
            caps->u8HeightAlign = 1;
            caps->u8StretchAlign = 2;
            caps->u8TileBaseAlign = 8;
            caps->u8TileWidthAlign = 16;
            caps->u8TileHeightAlign = 16;
            break;
        case E_GE_FMT_ABGR8888:
        case E_GE_FMT_ARGB8888:
        case E_GE_FMT_RGBA8888:
        case E_GE_FMT_BGRA8888:
        case E_GE_FMT_ACRYCB444:
        case E_GE_FMT_CRYCBA444:
        case E_GE_FMT_ACBYCR444:
        case E_GE_FMT_CBYCRA444:
            caps->u8BaseAlign = 4;
            caps->u8PitchAlign = 4;
            caps->u8Non1pAlign = 0;
            caps->u8HeightAlign = 1;
            caps->u8StretchAlign = 4;
            caps->u8TileBaseAlign = 8;
            caps->u8TileWidthAlign = 8;
            caps->u8TileHeightAlign = 16;
            break;
        // Not Support
        case E_GE_FMT_I1:
        case E_GE_FMT_I2:
        case E_GE_FMT_I4:
        default:
            caps->fmt = E_GE_FMT_GENERIC;
            caps->u8BaseAlign = 4;
            caps->u8PitchAlign = 4;
            caps->u8Non1pAlign = 0;
            caps->u8HeightAlign = 1;
            caps->u8StretchAlign = 4;
            caps->u8TileBaseAlign = 0;
            caps->u8TileWidthAlign = 0;
            caps->u8TileHeightAlign = 0;
            return E_GE_FAIL_FORMAT;
        }
    }

    return E_GE_OK;
}


GE_Result GE_Set_IOMap_Base(GE_CTX_HAL_LOCAL *pGEHalLocal, MS_VIRT addr)
{
    pGEHalLocal->va_mmio_base = addr;

#if (__GE_WAIT_TAG_MODE == __USE_GE_INT_MODE)
    virtHalIomapBaseAddr = addr;
#endif

    return E_GE_OK;
}


static MS_S32 direct_serial_diff( MS_U16 tagID1,  MS_U16 tagID2)
{
    if(tagID1 < tagID2)
    {
        if((tagID2-tagID1)>0x7FFF)
         {
             return (MS_S32)(0xFFFFUL-tagID2+tagID1+1);
         }
        else
            return -(MS_S32)(tagID2-tagID1);
    }
    else
    {
        if((tagID1-tagID2)>0x7FFF)
         {
             return -(MS_S32)(0xFFFF-tagID1+tagID2+1);
         }
        else
            return (MS_S32)(tagID1-tagID2);
    }
}

//-------------------------------------------------------------------------------------------------
/// Wait GE TagID back
/// @param  tagID                     \b IN: tag id number for wating
/// @return @ref GE_Result
//-------------------------------------------------------------------------------------------------
GE_Result GE_WaitTAGID(GE_CTX_HAL_LOCAL *pGEHalLocal, MS_U16 tagID)
{
     MS_U16 tagID_HW;
     MS_U32 u32Temp;
#if (__GE_WAIT_TAG_MODE == __USE_GE_INT_MODE)
    MS_U8  timeout_count = 0;
    MS_U32 event = 0x0;
    MS_BOOL bHadPrintOutDbgMsg = FALSE;
    MS_U8  i = 0;
#endif

#if (__GE_WAIT_TAG_MODE == __USE_GE_INT_MODE)
    tagID_HW = GE_ReadReg(pGEHalLocal, REG_GE_TAG);
    if (tagID_HW >= tagID)
    {
        return E_GE_OK;
    }

#if defined(MSOS_TYPE_LINUX)
    s32WaitingTagPid = (MS_S32)getpid();
#endif

    MsOS_ClearEvent(s32GeWaitTagEventHandle, 0x1);

    // unmask ge interrupt
    _GE_Ctrl_IntMode(pGEHalLocal, E_GE_CLEAR_INT | E_GE_UNMASK_INT);

    for(i=0;i<(GE_WordUnit>>2);i++)
    {
        GE_WriteReg(pGEHalLocal, REG_GE_INT_TAG_COND_H, 0);
        GE_WriteReg(pGEHalLocal, REG_GE_INT_TAG_COND_L, tagID);
    }

    while (MsOS_WaitEvent(s32GeWaitTagEventHandle, 0x1, &event, E_OR_CLEAR, GE_TAG_INTERRUPT_WAITING_TIME) == FALSE)
    {
        tagID_HW = GE_ReadReg(pGEHalLocal, REG_GE_TAG);
        if (direct_serial_diff(tagID_HW, tagID) >= 0)
        {
            break;
        }

        timeout_count++;

        if ((bHadPrintOutDbgMsg == FALSE) && (timeout_count > GE_TAG_INTERRUPT_DEBUG_PRINT_THRESHOLD))
        {
            _GE_Print_GeWaitTagTimeout_Msg(pGEHalLocal, tagID);
            bHadPrintOutDbgMsg = TRUE;
        }

        if(GE_ReadReg(pGEHalLocal, REG_GE_STAT) & GE_STAT_BUSY)
            continue;

        break;
    }

    // mask ge interrupt
    s32WaitingTagPid = 0;
    _GE_Ctrl_IntMode(pGEHalLocal, E_GE_MASK_INT);

#endif

     while(1)
     {

         tagID_HW = GE_ReadReg(pGEHalLocal, REG_GE_TAG);
         if(direct_serial_diff(tagID_HW, tagID) >= 0)
         {
            //printf("tagIDHW = %04x %04x\n", tagID_HW, tagID);
             break;
         }

         u32Temp = GE_ReadReg(pGEHalLocal, REG_GE_STAT);
         if((u32Temp&GE_STAT_CMDQ_MASK) < (16UL<<11))
            continue;
          if((u32Temp&GE_STAT_CMDQ2_MASK) < (16UL<<3))
            continue;
          if(GE_ReadReg(pGEHalLocal, REG_GE_CFG) & GE_CFG_VCMDQ)
          {
               u32Temp = GE_ReadReg(pGEHalLocal, REG_GE_VCMDQ_STAT);
               u32Temp |= (GE_ReadReg(pGEHalLocal, REG_GE_BIST_STAT)&1)<<16;
               if(u32Temp)
                    continue;

          }

           if(GE_ReadReg(pGEHalLocal, REG_GE_STAT) & GE_STAT_BUSY)
              continue;

          break;
          //GE_YIELD();

        }

    return E_GE_OK;

}
//-------------------------------------------------------------------------------------------------
/// MDrv_GE_SAVE_CHIP_IMAGE
//-------------------------------------------------------------------------------------------------
GE_Result GE_Restore_HAL_Context(GE_CTX_HAL_LOCAL *pGEHalLocal,MS_BOOL bNotFirstInit)
{
    MS_U16 i = 0;
    MS_U16 u16RegVal;

    //GE_WaitIdle(pGEHalLocal);

    while( (_GE_Reg_Backup[i] != REG_GE_INVALID) )
    {
        if(_GE_Reg_Backup[i]>= GE_TABLE_REGNUM)
        {
            break;
        }
        if(bNotFirstInit)
        {
            u16RegVal = GE_ReadReg(pGEHalLocal, _GE_Reg_Backup[i]);
        }
        else
        {
            u16RegVal = GE_ReadReg(pGEHalLocal, _GE_Reg_Backup[i] + GE_TABLE_REGNUM);
        }
        GE_RestoreReg(pGEHalLocal, _GE_Reg_Backup[i], u16RegVal);
        i++;
    }

    //GE_DBG(printf("GE_Restore_HAL_Context finished \n\n"));

    return E_GE_OK;
}

//-------------------------------------------------------------------------------------------------
/// Calculate Blit Scale Ratio:
//-------------------------------------------------------------------------------------------------
GE_Result GE_CalcBltScaleRatio(GE_CTX_HAL_LOCAL *pGEHalLocal, MS_U16 u16SrcWidth, MS_U16 u16SrcHeight, MS_U16 u16DstWidth, MS_U16 u16DstHeight, GE_ScaleInfo *pScaleinfo)
{
    if(NULL == pScaleinfo)
    {
        return E_GE_FAIL_PARAM;
    }

    if(u16SrcWidth >= (u16DstWidth<< g_GeChipPro.BltDownScaleCaps.u8ShiftRangeMin))
    {
        pScaleinfo->x = 0xFFFFFFFF;
    }
    else
    {
        pScaleinfo->x = GE_Divide2Fixed(u16SrcWidth, u16DstWidth, g_GeChipPro.BltDownScaleCaps.u8ShiftRangeMin, 12);
    }

    if(u16SrcHeight >= (u16DstHeight<< g_GeChipPro.BltDownScaleCaps.u8ShiftRangeMin))
    {
        pScaleinfo->y = 0xFFFFFFFF;
    }
    else
    {
        pScaleinfo->y = GE_Divide2Fixed(u16SrcHeight, u16DstHeight, g_GeChipPro.BltDownScaleCaps.u8ShiftRangeMin, 12);
    }

    /* HW use format S0.12 which means Bit(12) should be Sign bit
    // If overflow, S bit maybe wrong, handle it as actually value we hoped*/
    pScaleinfo->init_x = GE_Divide2Fixed(u16SrcWidth-u16DstWidth, 2 * u16DstWidth, 0, 12);
    if(u16SrcWidth >= u16DstWidth)
    {
        pScaleinfo->init_x &= (~(1<<12));
    }
    else
    {
        pScaleinfo->init_x |= (1<<12);
    }

    pScaleinfo->init_y = GE_Divide2Fixed(u16SrcHeight-u16DstHeight, 2 * u16DstHeight, 0, 12);
    if(u16SrcHeight >= u16DstHeight)
    {
        pScaleinfo->init_y &= (~(1<<12));
    }
    else
    {
        pScaleinfo->init_y |= (1<<12);
    }

    if (pGEHalLocal->bYScalingPatch)
    {
        if (u16SrcHeight<=5)
            pScaleinfo->init_y = (1<<12);
    }
    return E_GE_OK;
}

//-------------------------------------------------------------------------------------------------
/// Set GE scale register
/// @param  GE_Rect *src                    \b IN: src coordinate setting
/// @param  GE_DstBitBltType *dst           \b IN: dst coordinate setting
/// @return @ref GE_Result
//-------------------------------------------------------------------------------------------------
GE_Result GE_SetBltScaleRatio(GE_CTX_HAL_LOCAL *pGEHalLocal,GE_Rect *src, GE_DstBitBltType *dst, GE_Flag flags, GE_ScaleInfo* scaleinfo)
{
    GE_ScaleInfo geScaleinfo, *pGeScaleInfo = scaleinfo;

    if(flags & E_GE_FLAG_BYPASS_STBCOEF)
    {
        _GE_SetBltScaleRatio2HW(pGEHalLocal, pGeScaleInfo);
    }
    else if (flags & E_GE_FLAG_BLT_STRETCH)
    {
        /* Safe Guard. Prevent set scaling ratio < 1/32. Also prevent 0 h/w */
        if ((src->width-1) >= (dst->dstblk.width << g_GeChipPro.BltDownScaleCaps.u8ShiftRangeMin))
        {
            if(pGEHalLocal->bIsComp == FALSE)
            {
                return E_GE_FAIL_PARAM;
            }

            dst->dstblk.width = ((src->width-1) >> g_GeChipPro.BltDownScaleCaps.u8ShiftRangeMin) + 1;
        }
        if ((src->height-1) >= (dst->dstblk.height << g_GeChipPro.BltDownScaleCaps.u8ShiftRangeMin))
        {
            if(pGEHalLocal->bIsComp == FALSE)
            {
                return E_GE_FAIL_PARAM;
            }

            dst->dstblk.height = ((src->height-1) >> g_GeChipPro.BltDownScaleCaps.u8ShiftRangeMin) + 1;
        }

        pGeScaleInfo = &geScaleinfo;
        GE_CalcBltScaleRatio(pGEHalLocal, src->width, src->height, dst->dstblk.width, dst->dstblk.height, pGeScaleInfo);
        _GE_SetBltScaleRatio2HW(pGEHalLocal, pGeScaleInfo);
    }
    else
    {
        pGeScaleInfo = &geScaleinfo;

        pGeScaleInfo->x = (1<<12);
        pGeScaleInfo->y = (1<<12);
        pGeScaleInfo->init_x = 0;
        pGeScaleInfo->init_y = 0;

        _GE_SetBltScaleRatio2HW(pGEHalLocal, pGeScaleInfo);
    }

    return E_GE_OK;
}

GE_Result GE_BitBltEX_Trape(GE_CTX_HAL_LOCAL *pGEHalLocal, GE_Rect *pSrcRect, GE_Normalized_Trapezoid *pGENormTrapezoid, MS_U32 u32Flags, GE_ScaleInfo* pScaleinfo)
{
    return E_GE_NOT_SUPPORT;
}

//-------------------------------------------------------------------------------------------------
/// GE Primitive Drawing - TRAPEZOID
/// @param  pGENormTrapezoid                    \b IN: pointer to position of TRAPEZOID
/// @param  u32ColorS                   \b IN: start color of TRAPEZOID when gradient
/// @param  u32ColorE                   \b IN: end color of TRAPEZOID when gradient
/// @param  pColorDeltaX                  \b IN: x gradient color
/// @param  pColorDeltaY                   \b IN:  y gradient color
/// @return @ref GE_Result
//-------------------------------------------------------------------------------------------------
GE_Result GE_FillTrapezoid(GE_CTX_HAL_LOCAL *pGEHalLocal, MS_BOOL bYTrapezoid, GE_Normalized_Trapezoid *pGENormTrapezoid, MS_U32 u32Color, GE_ColorDelta *pColorDeltaX, GE_ColorDelta *pColorDeltaY)
{
    return E_GE_NOT_SUPPORT;
}

//-------------------------------------------------------------------------------------------------
/// Set GE DISABLE MIU ACCESS
/// @param  enable                  \b IN: enable and update setting
/// @return @ref GE_Result
//-------------------------------------------------------------------------------------------------
GE_Result GE_SetDisaMIUAccess(GE_CTX_HAL_LOCAL *pGEHalLocal,MS_BOOL enable)
{
    MS_U16              u16en;

    GE_DBG("%s\n", __FUNCTION__);

    u16en = GE_ReadReg(pGEHalLocal,REG_GE_CFG);
    if (enable)
    {
        u16en |= GE_CFG_DISABLE_MIU_ACS;
    }
    else
    {
        u16en &= ~GE_CFG_DISABLE_MIU_ACS;
    }
    GE_WriteReg(pGEHalLocal,REG_GE_CFG, u16en);

    return E_GE_OK;
}
//-------------------------------------------------------------------------------------------------
/// Set GE Clear Invalid MIU Flag
/// @param  enable                  \b IN: enable and update setting
/// @return @ref GE_Result
//-------------------------------------------------------------------------------------------------
GE_Result GE_ClrInvalMIUFlg(GE_CTX_HAL_LOCAL *pGEHalLocal,MS_BOOL enable)
{
    MS_U16              u16en;

    GE_DBG("%s\n", __FUNCTION__);

    u16en = GE_ReadReg(pGEHalLocal,REG_GE_CFG);
    if (enable)
    {
        u16en |= GE_CFG_CLR_MIU_FLG;
    }
    else
    {
        u16en &= ~GE_CFG_CLR_MIU_FLG;
    }
    GE_WriteReg(pGEHalLocal,REG_GE_CFG, u16en);

    return E_GE_OK;
}

//-------------------------------------------------------------------------------------------------
/// Set Enable Dynamic Clock Gating
/// @param  enable                  \b IN: enable and update setting
/// @return @ref GE_Result
//-------------------------------------------------------------------------------------------------
GE_Result GE_EnableDynaClkGate(GE_CTX_HAL_LOCAL *pGEHalLocal,MS_BOOL enable)
{
    MS_U16              u16en;

    GE_DBG("%s\n", __FUNCTION__);

    u16en = GE_ReadReg(pGEHalLocal,REG_GE_CFG);
    if (enable)
    {
        u16en |= GE_CFG_EN_DNY_CLK_GATE;
    }
    else
    {
        u16en &= ~GE_CFG_EN_DNY_CLK_GATE;
    }
    GE_WriteReg(pGEHalLocal,REG_GE_CFG, u16en);

    return E_GE_OK;
}

GE_Result GE_EnableTrapezoidAA(GE_CTX_HAL_LOCAL *pGEHalLocal, MS_BOOL bEnable)
{
    //GE_DBG("%s\n", __FUNCTION__);

    return E_GE_NOT_SUPPORT;

}

GE_Result GE_EnableTrapSubPixCorr(GE_CTX_HAL_LOCAL *pGEHalLocal, MS_BOOL bEnable)
{
    //GE_DBG("%s\n", __FUNCTION__);

    return E_GE_NOT_SUPPORT;

}

MS_U16  GE_GetNextTAGID(GE_CTX_HAL_LOCAL *pGEHalLocal, MS_BOOL bStepTagBefore)
{
    MS_U16 tagID;
    if(bStepTagBefore)
    {
        if(0 == ++pGEHalLocal->pHALShared->global_tagID)
             ++pGEHalLocal->pHALShared->global_tagID;
    }
    tagID =pGEHalLocal->pHALShared->global_tagID;

    return tagID;
}

GE_Result GE_SetVCmdBuffer(GE_CTX_HAL_LOCAL *pGEHalLocal, MS_PHY PhyAddr, GE_VcmqBufSize enBufSize)
{
    MS_U16 u16RegVal;

    if(enBufSize >= E_GE_VCMD_1024K)
    {
        return E_GE_NOT_SUPPORT;
    }

    //GE_SetVQBufMIUId(pGEHalLocal, _GFXAPI_MIU_ID(PhyAddr));
    //PhyAddr = GE_ConvertAPIAddr2HAL(pGEHalLocal, _GFXAPI_MIU_ID(PhyAddr), _GFXAPI_PHYS_ADDR_IN_MIU(PhyAddr));

    HAL_GE_SetBufferAddr(pGEHalLocal,PhyAddr,E_GE_BUF_VCMDQ);

    u16RegVal = (GE_ReadReg(pGEHalLocal, REG_GE_VCMDQ_SIZE) & ~GE_VCMDQ_SIZE_MASK) | ((GE_MapVQ2Reg(enBufSize) & GE_VCMDQ_SIZE_MASK));
    GE_WriteReg(pGEHalLocal, REG_GE_VCMDQ_SIZE, u16RegVal);

    return E_GE_OK;
}

GE_Result GE_InitCtxHalPalette(GE_CTX_HAL_LOCAL *pGEHalLocal)
{
    MS_U32 u32Idx;

    for(u32Idx=0; u32Idx<GE_PALETTE_NUM; u32Idx++)
    {
        GE_WriteReg(pGEHalLocal, REG_GE_CLUT_CTRL, ((u32Idx) & GE_CLUT_CTRL_IDX_MASK) | GE_CLUT_CTRL_RD);
        GE_WaitIdle(pGEHalLocal);
        pGEHalLocal->u32Palette[u32Idx] = ByteSwap32((((MS_U32)GE_ReadReg(pGEHalLocal, REG_GE_CLUT_H)<<16) | GE_ReadReg(pGEHalLocal, REG_GE_CLUT_L)));
    }

    pGEHalLocal->bPaletteDirty = FALSE;

    return (E_GE_OK);
}

void GE_Init_HAL_Context(GE_CTX_HAL_LOCAL *pGEHalLocal, GE_CTX_HAL_SHARED *pHALShared, MS_BOOL bNeedInitShared)
{
     memset(pGEHalLocal, 0, sizeof(*pGEHalLocal));

     if(bNeedInitShared)
     {
         memset(pHALShared, 0, sizeof(*pHALShared));
         pHALShared->global_tagID = 1;
     }
     pGEHalLocal->pHALShared = pHALShared;
     pGEHalLocal->bYScalingPatch = FALSE;
}

GE_Result GE_Set_IOMap_Base2(GE_CTX_HAL_LOCAL *pGEHalLocal, MS_VIRT addr)
{
    pGEHalLocal->va_mmio_base2 = addr;
    return E_GE_OK;
}

GE_Result GE_SetClock(GE_CTX_HAL_LOCAL *pGEHalLocal, MS_BOOL bOnOff)
{
    mtk_write2bytemask(REG_02A0_CKGEN01, GE_CLK_528M, REG_02A0_CKGEN01_REG_CKG_GE);
    mtk_write2bytemask(REG_15CC_CKGEN01, 1, REG_15CC_CKGEN01_REG_SW_EN_GE_FIFO_R2GE);
    mtk_write2bytemask(REG_15D0_CKGEN01, 1, REG_15D0_CKGEN01_REG_SW_EN_GE_FIFO_W2GE);
    mtk_write2bytemask(REG_15D4_CKGEN01, 1, REG_15D4_CKGEN01_REG_SW_EN_GE_PSRAM2GE);
    mtk_write2bytemask(REG_15D8_CKGEN01, 1, REG_15D8_CKGEN01_REG_SW_EN_GE2GE);
    mtk_write2bytemask(REG_1B74_CKGEN01, 1, REG_1B74_CKGEN01_REG_SW_EN_MCU_NONPM2GE);
    mtk_write2bytemask(REG_14E4_CKGEN00, 1, REG_SW_EN_SMI2GE);

    return E_GE_OK;
}

MS_BOOL GE_NonOnePixelModeCaps(GE_CTX_HAL_LOCAL *pGEHalLocal, PatchBitBltInfo* patchInfo)
{
    GE_ScaleInfo geScaleinfo;
    GE_Result ret;

    patchInfo->scaleinfo =&geScaleinfo;
    ret = GE_CalcBltScaleRatio(pGEHalLocal, patchInfo->src.width , patchInfo->src.height ,patchInfo->dst.dstblk.width , patchInfo->dst.dstblk.height, patchInfo->scaleinfo);

    if(ret == E_GE_FAIL_PARAM)
    {
   return pGEHalLocal->pGeChipPro->bFourPixelModeStable;
}
    else if ((patchInfo->scaleinfo->x != GE_SCALING_MULITPLIER) || (patchInfo->scaleinfo->y != GE_SCALING_MULITPLIER))
    {
         return FALSE;
    }
    else
    {
   return pGEHalLocal->pGeChipPro->bFourPixelModeStable;
    }
}

GE_Result HAL_GE_EnableCalcSrc_WidthHeight(GE_CTX_HAL_LOCAL *pGEHalLocal, MS_BOOL bEnable)
{
    MS_U16 u16en;

    u16en = GE_CODA_ReadReg(pGEHalLocal, REG_0000_GE0);

    if(bEnable)
    {
        if(u16en & GE_EN_BURST)
        {
            GE_CODA_WriteReg(pGEHalLocal, REG_0000_GE0, u16en | GE_EN_CALC_SRC_WH);
        }
    }
    else
    {
        GE_CODA_WriteReg(pGEHalLocal, REG_0000_GE0, u16en & (~GE_EN_CALC_SRC_WH));
    }

    return E_GE_OK;
}

GE_Result GEWD_ReadReg(GE_CTX_HAL_LOCAL *pGEHalLocal, MS_U16 addr, MS_U16* value)
{
    //For two source buffer read register
    return E_GE_NOT_SUPPORT;
}

GE_Result GEWD_WriteReg(GE_CTX_HAL_LOCAL *pGEHalLocal, MS_U16 addr, MS_U16 value)
{
    //For two source buffer write register
    return E_GE_NOT_SUPPORT;
}

GE_Result GE_SetTLBMode(GE_CTX_HAL_LOCAL *GE_UNUSED(pstGEHalLocal), GE_TLB_Mode GE_UNUSED(enTlb_type))
{

    return E_GE_OK;
}

GE_Result GE_GetTLBSRCADDR(GE_CTX_HAL_LOCAL *GE_UNUSED(pstGEHalLocal), MS_PHY* GE_UNUSED(phyAddr))
{
    return E_GE_NOT_SUPPORT;
}

GE_Result GE_GetTLBDSTADDR(GE_CTX_HAL_LOCAL *GE_UNUSED(pstGEHalLocal), MS_PHY* GE_UNUSED(phyAddr))
{
    return E_GE_NOT_SUPPORT;
}

GE_Result GE_SetTLBSrcBaseAddr(GE_CTX_HAL_LOCAL *GE_UNUSED(pstGEHalLocal), MS_PHY GE_UNUSED(phyAddr))
{
    return E_GE_NOT_SUPPORT;
}

GE_Result GE_SetTLBDstBaseAddr(GE_CTX_HAL_LOCAL *GE_UNUSED(pstGEHalLocal), MS_PHY GE_UNUSED(phyAddr))
{
    return E_GE_NOT_SUPPORT;
}

GE_Result GE_FlushTLBTable(GE_CTX_HAL_LOCAL *GE_UNUSED(pstGEHalLocal), MS_BOOL GE_UNUSED(bEnable))
{
    return E_GE_NOT_SUPPORT;
}

GE_Result GE_SetTLBTag(GE_CTX_HAL_LOCAL *GE_UNUSED(pstGEHalLocal), MS_U16 GE_UNUSED(u16Tag))
{
    return E_GE_NOT_SUPPORT;
}

GE_Result GE_StopFlushTLB(GE_CTX_HAL_LOCAL *GE_UNUSED(pstGEHalLocal))
{
    return E_GE_NOT_SUPPORT;
}


GE_Result GE_Get_MIU_INTERVAL(GE_CTX_HAL_LOCAL *pGEHalLocal, MS_U8 miu, MS_PHY* value)
{
    MS_PHY PhyOffset = 0;

    _miu_offset_to_phy(miu, PhyOffset, *value);

    return E_GE_OK;
}

GE_Result HAL_GE_AdjustDstWin( GE_CTX_HAL_LOCAL *pGEHalLocal, MS_BOOL bDstXInv )
{
    MS_U16  u16ClipL=0,u16ClipR=0;
    MS_U16  u16DstX=0;

    u16DstX  = GE_CODA_ReadReg(pGEHalLocal, REG_01A8_GE0);
    if( bDstXInv==FALSE )
    {
        u16ClipR = GE_CODA_ReadReg(pGEHalLocal, REG_0158_GE0);
        if( u16ClipR < u16DstX )
        {
            GE_CODA_WriteReg(pGEHalLocal, REG_01A8_GE0, u16ClipR);
        }
    }
    else
    {
        u16ClipL = GE_CODA_ReadReg(pGEHalLocal, REG_0154_GE0);
        if( u16ClipL > u16DstX )
        {
            GE_CODA_WriteReg(pGEHalLocal, REG_01A8_GE0, u16ClipL);
        }
    }

    return E_GE_OK;
}

GE_Result HAL_GE_AdjustRotateDstWin( GE_CTX_HAL_LOCAL *pGEHalLocal, MS_U8 u8Rotate )
{
    return E_GE_OK;
}

GE_Result HAL_GE_exit(GE_CTX_HAL_LOCAL *pGEHalLocal)
{
#if (__GE_WAIT_TAG_MODE == __USE_GE_INT_MODE)
    if (TRUE == MsOS_DetachInterrupt(E_INT_IRQ_GE))
    {
        bGeIrqInited = FALSE;
    }

    if (s32GeWaitTagEventHandle > 0)
    {
        if (TRUE == MsOS_DeleteEventGroup(s32GeWaitTagEventHandle))
        {
            s32GeWaitTagEventHandle = -1;
        }
    }
#endif

    return E_GE_OK;
}

GE_Result HAL_GE_SetBurstMiuLen(GE_CTX_HAL_LOCAL *pGEHalLocal,MS_BOOL bEnable,MS_U32 u32BurstLen)
{
    MS_U16 u16Reg = 0;

    u16Reg = GE_ReadReg(pGEHalLocal, _REG_GE_DBG);
    u16Reg &= ( ~GE_DBG_MIU_MAX_LEG );
    if(u32BurstLen>0)
    {
        u16Reg |= (MS_U16)(((u32BurstLen - 1)<<8) & GE_DBG_MIU_MAX_LEG );
    }
    GE_WriteReg(pGEHalLocal, _REG_GE_DBG, u16Reg);

    u16Reg = GE_ReadReg(pGEHalLocal, REG_GE_CFG);
    if(bEnable)
        u16Reg |= GE_CFG_LENGTH_LIMIT;
    else
        u16Reg &= (~GE_CFG_LENGTH_LIMIT);
    GE_WriteReg(pGEHalLocal, REG_GE_CFG, u16Reg);

    return E_GE_OK;
}
GE_Result HAL_GE_SetBufferAddr(GE_CTX_HAL_LOCAL *pGEHalLocal,MS_PHY PhyAddr,GE_BufType enBuffType)
{
    MS_U16 u16tmp_REG_GE_BASE_MSB = 0;

    switch(enBuffType)
    {
        case E_GE_BUF_SRC:
            GE_CODA_WriteReg(pGEHalLocal, REG_0080_GE0, (PhyAddr & 0xFFFF));
            GE_CODA_WriteReg(pGEHalLocal, REG_0084_GE0, ((PhyAddr >> 16) & 0x7FFF));
            u16tmp_REG_GE_BASE_MSB = (GE_CODA_ReadReg(pGEHalLocal, REG_008C_GE0) & ~GE_SRC_MSB_MASK);
            u16tmp_REG_GE_BASE_MSB = (u16tmp_REG_GE_BASE_MSB | ((PhyAddr >> 31) & 0x7));
            GE_CODA_WriteReg(pGEHalLocal, REG_008C_GE0, u16tmp_REG_GE_BASE_MSB);
            break;
        case E_GE_BUF_DST:
            GE_CODA_WriteReg(pGEHalLocal, REG_0098_GE0, (PhyAddr & 0xFFFF));
            GE_CODA_WriteReg(pGEHalLocal, REG_009C_GE0, ((PhyAddr >> 16) & 0x7FFF));
            u16tmp_REG_GE_BASE_MSB = (GE_CODA_ReadReg(pGEHalLocal, REG_008C_GE0) & ~GE_DST_MSB_MASK);
            u16tmp_REG_GE_BASE_MSB = (u16tmp_REG_GE_BASE_MSB | (((PhyAddr >> 31) & 0x7)<<4));
            GE_CODA_WriteReg(pGEHalLocal, REG_008C_GE0, u16tmp_REG_GE_BASE_MSB);
            break;
        case E_GE_BUF_VCMDQ:
            GE_CODA_WriteReg(pGEHalLocal, REG_00A0_GE0, (PhyAddr & 0xFFFF));
            GE_CODA_WriteReg(pGEHalLocal, REG_00A4_GE0, ((PhyAddr >> 16) & 0x7FFF));
            u16tmp_REG_GE_BASE_MSB = (GE_CODA_ReadReg(pGEHalLocal, REG_008C_GE0) & ~GE_VCMDQ_MSB_MASK);
            u16tmp_REG_GE_BASE_MSB = (u16tmp_REG_GE_BASE_MSB | (((PhyAddr >> 31) & 0x7)<<8));
            GE_CODA_WriteReg(pGEHalLocal, REG_008C_GE0, u16tmp_REG_GE_BASE_MSB);
            break;
        default:
            GE_H_ERR("[%s][%d]Buffer Type Error!!!!\n",__func__,__LINE__);
            break;
    }
    return E_GE_OK;
}

GE_Result HAL_GE_GetCRC(GE_CTX_HAL_LOCAL *pGEHalLocal,MS_U32* pu32CRCvalue)
{
    MS_U16 u16CRC_L = 0;
    MS_U16 u16CRC_H = 0;

    u16CRC_L = GE2_REG(REG_GE_CRC_L);
    u16CRC_H = GE2_REG(REG_GE_CRC_H);

    *pu32CRCvalue = (((MS_U32)u16CRC_H<<16) | u16CRC_L);

    return E_GE_OK;
}

GE_Result HAL_GE_ConfigCRC(GE_CTX_HAL_LOCAL *pGEHalLocal,ST_DRV_GE_ConfigCRC *pstConfigCRC)
{
    MS_U16 u16Reg;
    switch(pstConfigCRC->enConfigCRC)
    {
        case E_DRV_GE_CRC_ENABLE:
            u16Reg = GE2_REG(REG_GE2_CTRL);
            if(*((MS_BOOL*)pstConfigCRC->pCRCsetting) )
            {
                u16Reg |= GE2_EN_CRC;
            }
            else
            {
                u16Reg &= (~GE2_EN_CRC);
            }
            GE2_WriteReg(pGEHalLocal, REG_GE2_CTRL, u16Reg);
            break;
    case E_DRV_GE_CRC_CLEAR:
            u16Reg = GE2_REG(REG_GE2_CFG);
            if(*((MS_BOOL*)pstConfigCRC->pCRCsetting) )
            {
                u16Reg |= GE2_CLEAR_CRC;
            }
            else
            {
                u16Reg &= (~GE2_CLEAR_CRC);
            }
            GE2_WriteReg(pGEHalLocal, REG_GE2_CFG, u16Reg);
            break;
    default:
        break;

    }

    return E_GE_OK;
}

#if defined(MSOS_TYPE_LINUX_KERNEL)
GE_Result HAL_GE_STR_RestoreReg(GE_CTX_HAL_LOCAL *pGEHalLocal, GE_STR_SAVE_AREA *pGFX_STRPrivate)
{
    MS_U8 i=0UL;

            //enable power
    CLK_REG(CHIP_GE_CLK) = pGFX_STRPrivate->GECLK_Reg;

    for( i = 0; _GE_Reg_Backup[i] < REG_GE_INVALID; ++i)
    {
        GE2_REG(_GE_Reg_Backup[i]) = pGFX_STRPrivate->GETLB_Reg[_GE_Reg_Backup[i]];
        GE_REG(_GE_Reg_Backup[i]) = pGFX_STRPrivate->GE0_Reg[_GE_Reg_Backup[i]];
    }

    return E_GE_OK;
}
#endif

#if defined(CONFIG_SUPPORT_MULTI_GATE)
GE_Result MHal_GE_SetGClk(GE_CTX_HAL_LOCAL *pGEHalLocal, MS_BOOL bEnClk)
{
    MS_U16 u16RegVal=0, u16RegVal2=0;

    u16RegVal = GE2_REG(REG_GE2_CTRL);
    u16RegVal2 = GCLK_REG(REG_GE_GCLK2);

    if(bEnClk == TRUE)
    {
        u16RegVal |= (REG_GE2_GCLK);
        u16RegVal2 |= (REG_GE_GCLK2_MSK);
        GE2_REG(REG_GE2_CTRL) = u16RegVal;
        GCLK_REG(REG_GE_GCLK2) = u16RegVal2;
    }
    else
    {
        u16RegVal &= (~REG_GE2_GCLK);
        u16RegVal2 &= (~REG_GE_GCLK2_MSK);
        GE2_REG(REG_GE2_CTRL) = u16RegVal;
        //GCLK_REG(REG_GE_GCLK2) = u16RegVal2;
    }

    return E_GE_OK;
}
#endif
void GE_CalcColorDelta(MS_U32 color0, MS_U32 color1, MS_U16 ratio, GE_ColorDelta *delta)
{
    MS_U8                        a0, r0, g0, b0;
    MS_U8                        a1, r1, g1, b1;


    // Get A,R,G,B
    //[TODO] special format
    b0 = (color0)       & 0xFF;
    g0 = (color0 >> 8)  & 0xFF;
    r0 = (color0 >> 16) & 0xFF;
    a0 = (color0 >> 24);
    b1 = (color1)       & 0xFF;
    g1 = (color1 >> 8)  & 0xFF;
    r1 = (color1 >> 16) & 0xFF;
    a1 = (color1 >> 24);

    //[TODO] revise and take advantage on Divid2Fixed for negative value
    if (b0 > b1)
    {
        delta->b = GE_Divide2Fixed((b0-b1), ratio, 7, 12);
        delta->b = (MS_U32)(1<<(1+7+12)) - delta->b; // negative
    }
    else
    {
        delta->b = GE_Divide2Fixed((b1-b0), ratio, 7, 12);
    }

    if (g0 > g1)
    {
        delta->g = GE_Divide2Fixed((g0-g1), ratio, 7, 12);
        delta->g = (MS_U32)(1<<(1+7+12)) - delta->g; // negative
    }
    else
    {
        delta->g = GE_Divide2Fixed((g1-g0), ratio, 7, 12);
    }

    if (r0 > r1)
    {
        delta->r = GE_Divide2Fixed((r0-r1), ratio, 7, 12);
        delta->r = (MS_U32)(1<<(1+7+12)) - delta->r; // negative
    }
    else
    {
        delta->r = GE_Divide2Fixed((r1-r0), ratio, 7, 12);
    }

    if (a0 > a1)
    {
        delta->a = (MS_U16)GE_Divide2Fixed((a0-a1), ratio, 4, 11);
        delta->a = (MS_U16)(1<<(1+4+11)) - delta->a; // negative
    }
    else
    {
        delta->a = (MS_U16)GE_Divide2Fixed((a1-a0), ratio, 4, 11);
    }
}

GE_Result GE_SetPalette(GE_CTX_HAL_LOCAL *pGEHalLocal)
{
    MS_U32 u32Idx= 0;

    for(u32Idx=0; u32Idx<GE_PALETTE_NUM; u32Idx++)
    {
        GE_WriteReg(pGEHalLocal, REG_GE_CLUT_H, ByteSwap16(pGEHalLocal->u32Palette[u32Idx]) & 0xFFFF);
        GE_WriteReg(pGEHalLocal, REG_GE_CLUT_L, ByteSwap16(pGEHalLocal->u32Palette[u32Idx]>>16));
        GE_WriteReg(pGEHalLocal, REG_GE_CLUT_CTRL, ((u32Idx) & GE_CLUT_CTRL_IDX_MASK) | GE_CLUT_CTRL_WR);
    }

    return (E_GE_OK);
}

GE_Result HAL_GE_SetVCmd_W_Thread(GE_CTX_HAL_LOCAL *pGEHalLocal, MS_U8 u8W_Threshold)
{
    MS_U16 u16tmp;

    u16tmp = GE_CODA_ReadReg(pGEHalLocal, REG_000C_GE0);

    u16tmp &= ~(GE_TH_CMDQ_MASK);
    u16tmp |= u8W_Threshold << 4;

    GE_CODA_WriteReg(pGEHalLocal, REG_000C_GE0, u16tmp);

    return E_GE_OK;
}

GE_Result HAL_GE_SetVCmd_R_Thread(GE_CTX_HAL_LOCAL *pGEHalLocal, MS_U8 u8R_Threshold)
{
    MS_U16 u16tmp;

    u16tmp = GE_CODA_ReadReg(pGEHalLocal, REG_000C_GE0);

    u16tmp &= ~(GE_TH_CMDQ2_MASK);
    u16tmp |= u8R_Threshold << 8;

    GE_CODA_WriteReg(pGEHalLocal, REG_000C_GE0, u16tmp);

    return E_GE_OK;
}

GE_Result HAL_GE_SetYUVMode(GE_CTX_HAL_LOCAL *pGEHalLocal, GE_YUVMode *mode)
{
    MS_U16 u16YuvMode = 0, u16Reg = 0;

    u16Reg = GE_CODA_ReadReg(pGEHalLocal, REG_007C_GE0);

    u16Reg &= ~(GE_YUV_CSC_MASK);
    u16YuvMode |= mode->rgb2yuv << GE_YUV_RGB2YUV_SHFT;
    u16YuvMode |= mode->out_range << GE_YUV_OUT_RANGE_SHFT;
    u16YuvMode |= mode->in_range << GE_YUV_IN_RANGE_SHFT;

    u16YuvMode |= mode->src_fmt << GE_YUV_SRC_YUV422_SHFT;
    u16YuvMode |= mode->dst_fmt << GE_YUV_DST_YUV422_SHFT;

    GE_CODA_WriteReg(pGEHalLocal, REG_007C_GE0, u16Reg|u16YuvMode);

    return E_GE_OK;
}

GE_Result HAL_GE_SetClipWindow(GE_CTX_HAL_LOCAL *pGEHalLocal, GE_Rect *rect)
{
    GE_CODA_WriteReg(pGEHalLocal, REG_0154_GE0, rect->x);
    GE_CODA_WriteReg(pGEHalLocal, REG_0154_GE0, rect->y);
    GE_CODA_WriteReg(pGEHalLocal, REG_0158_GE0, (rect->x + rect->width - 1));
    GE_CODA_WriteReg(pGEHalLocal, REG_0160_GE0, (rect->y + rect->height - 1));

    return E_GE_OK;
}

GE_Result HAL_GE_SetBuffer(GE_CTX_HAL_LOCAL *pGEHalLocal, MS_U16 u16Pitch, MS_U16 u16Fmt,GE_BufType enBuffType)
{
    MS_U16 u16Tmp;

    switch(enBuffType)
    {
        case E_GE_BUF_SRC:
            GE_CODA_WriteReg(pGEHalLocal, REG_00C0_GE0, u16Pitch);
            u16Tmp = (GE_CODA_ReadReg(pGEHalLocal, REG_00D0_GE0) & ~GE_SRC_FMT_MASK) | ( u16Fmt << GE_SRC_FMT_SHFT);
            GE_CODA_WriteReg(pGEHalLocal, REG_00D0_GE0, u16Tmp);
            break;
        case E_GE_BUF_DST:
            GE_CODA_WriteReg(pGEHalLocal, REG_00CC_GE0, u16Pitch);
            u16Tmp = (GE_CODA_ReadReg(pGEHalLocal, REG_00D0_GE0) & ~GE_DST_FMT_MASK) | ( u16Fmt << GE_DST_FMT_SHFT);
            GE_CODA_WriteReg(pGEHalLocal, REG_00D0_GE0, u16Tmp);
            break;
        default:
            GE_H_ERR("[%s][%d]Buffer Type Error!!!!\n",__func__,__LINE__);
            break;
    }

    return E_GE_OK;
}

MS_U16 HAL_GE_GetFmt(GE_CTX_HAL_LOCAL *pGEHalLocal)
{
    return GE_CODA_ReadReg(pGEHalLocal, REG_00D0_GE0);
}

GE_Result HAL_GE_SetDither(GE_CTX_HAL_LOCAL *pGEHalLocal, MS_BOOL enable)
{
    MS_U16              u16en;

    u16en = GE_CODA_ReadReg(pGEHalLocal, REG_0000_GE0);
    if (enable)
    {
        u16en |= GE_EN_DITHER;
    }
    else
    {
        u16en &= ~GE_EN_DITHER;
    }
    GE_CODA_WriteReg(pGEHalLocal, REG_0000_GE0, u16en);

    return E_GE_OK;
}

static MS_BOOL GE_RectOverlap(GE_Rect *rect0, GE_DstBitBltType *rect1)
{
    if ( (rect0->x+rect0->width-1  < rect1->dstblk.x)            ||
         (rect0->x > rect1->dstblk.x+rect1->dstblk.width-1)      ||
         (rect0->y+rect0->height-1 < rect1->dstblk.y)            ||
         (rect0->y > rect1->dstblk.y+rect1->dstblk.height-1))
    {
        return FALSE; // no overlap
    }

    return TRUE;
}

GE_Result HAL_GE_BitBltEX(GE_CTX_HAL_LOCAL *pGEHalLocal, GE_Rect *src, GE_DstBitBltType *dst, MS_U32 flags, GE_ScaleInfo* scaleinfo)
{
    MS_U16              u16cmd = 0;
    MS_U16              u16cfg = 0;
    MS_U16              u16en = 0;
    MS_U16              temp;
    MS_BOOL             bOverlap = FALSE;
    MS_BOOL             bNonOnePixelMode = FALSE;
    GE_Point            v0, v1, v2;
    MS_U8               u8Rot;
    PatchBitBltInfo     patchBitBltInfo;
    MS_BOOL bDstXInv = FALSE;
    MS_U16              u16tmp,Srcfmt,Dstfmt;
    MS_U16              u16tmp_dy;


    // clear dirty attributes ---------------------------------------------------------------------
    GE_SetRotate(pGEHalLocal, E_GE_ROTATE_0);
    u16cfg = GE_CODA_ReadReg(pGEHalLocal, REG_0004_GE0) & ~(GE_CFG_BLT_STRETCH | GE_CFG_BLT_ITALIC);
    // clear dirty attributes ---------------------------------------------------------------------


    if(flags & E_GE_FLAG_BYPASS_STBCOEF)
    {
        if(scaleinfo == NULL)
        {
            return E_GE_FAIL_PARAM;
        }

        u16cfg |= GE_CFG_BLT_STRETCH;
        if (flags & E_GE_FLAG_STRETCH_NEAREST)
        {
            u16cmd |= GE_STRETCH_NEAREST;
        }
        u16cmd |= GE_STRETCH_CLAMP;

    }
    else if (flags & E_GE_FLAG_BLT_STRETCH)
    {

        u16cfg |= GE_CFG_BLT_STRETCH;
        if (flags & E_GE_FLAG_STRETCH_NEAREST)
        {
            u16cmd |= GE_STRETCH_NEAREST;
        }
        u16cmd |= GE_STRETCH_CLAMP;

    }
    else
    {
        if ( (src->width != dst->dstblk.width) || (src->height != dst->dstblk.height) )
        {
            return E_GE_FAIL_STRETCH;
        }

    }

    if( GE_SetBltScaleRatio(pGEHalLocal, src, dst, (GE_Flag)flags, scaleinfo) == E_GE_FAIL_PARAM)
        return E_GE_FAIL_PARAM;

    v0.x = dst->dstblk.x;
    v0.y = dst->dstblk.y;
    v1.x = v0.x + dst->dstblk.width - 1;
    v1.y = v0.y + dst->dstblk.height - 1;
    v2.x = src->x;
    v2.y = src->y;

    if (flags & E_GE_FLAG_BLT_OVERLAP)
        bOverlap = GE_RectOverlap(src, dst);

    if (bOverlap)
    {
         GE_DBG("[%s][%d] Overlap\n", __FUNCTION__, __LINE__);

        if (v2.x < v0.x)
        {
            // right to left
            temp = v0.x;
            v0.x = v1.x;
            v1.x = temp;
            v2.x += src->width - 1;
            u16cmd |= GE_SRC_DIR_X_INV | GE_DST_DIR_X_INV;
        }
        if (v2.y < v0.y)
        {
            // bottom up
            temp = v0.y;
            v0.y = v1.y;
            v1.y = temp;
            v2.y += src->height - 1;
            u16cmd |= GE_SRC_DIR_Y_INV | GE_DST_DIR_Y_INV;
        }
    }

    if (flags & E_GE_FLAG_BLT_MIRROR_H)
    {
        u16cmd |= GE_SRC_DIR_X_INV;
    }
    if (flags & E_GE_FLAG_BLT_MIRROR_V)
    {
        u16cmd |= GE_SRC_DIR_Y_INV;
    }

    if (flags & E_GE_FLAG_BLT_DST_MIRROR_H)
    {
        temp = v0.x;
        v0.x = v1.x;
        v1.x = temp;
        u16cmd |= GE_DST_DIR_X_INV;
    }
    if (flags & E_GE_FLAG_BLT_DST_MIRROR_V)
    {
        temp = v0.y;
        v0.y = v1.y;
        v1.y = temp;
        u16cmd |= GE_DST_DIR_Y_INV;
    }

    if (flags & E_GE_FLAG_BLT_ITALIC)
    {
        u16cfg |= GE_CFG_BLT_ITALIC;
    }
    if (flags & GE_FLAG_BLT_ROTATE_MASK) // deal with ALL rotation mode
    {
        GE_SetRotate(pGEHalLocal, (GE_RotateAngle)((flags & E_GE_FLAG_BLT_ROTATE_270)>>GE_FLAG_BLT_ROTATE_SHFT));
    }

    GE_CODA_WriteReg(pGEHalLocal, REG_01A0_GE0, v0.x);
    GE_CODA_WriteReg(pGEHalLocal, REG_01A4_GE0, v0.y);
    GE_CODA_WriteReg(pGEHalLocal, REG_01A8_GE0, v1.x);
    GE_CODA_WriteReg(pGEHalLocal, REG_01AC_GE0, v1.y);
    GE_CODA_WriteReg(pGEHalLocal, REG_01B0_GE0, v2.x);
    GE_CODA_WriteReg(pGEHalLocal, REG_01B4_GE0, v2.y);
    GE_CODA_WriteReg(pGEHalLocal, REG_01B8_GE0, src->width);

    temp = src->height;

    if (pGEHalLocal->bYScalingPatch)
    {
        if (temp>5)
        {
            temp-=5;
        }
    }

    GE_CODA_WriteReg(pGEHalLocal, REG_01BC_GE0, temp);

    u8Rot = GE_CODA_ReadReg(pGEHalLocal, REG_0164_GE0 ) & REG_GE_ROT_MODE_MASK;

    // Set dst coordinate
    if (u8Rot == 0)
    {
        if (!bOverlap)
        {
            if(!(flags & E_GE_FLAG_BLT_DST_MIRROR_H))
            {
            GE_CODA_WriteReg(pGEHalLocal, REG_01A0_GE0, v0.x);
                GE_CODA_WriteReg(pGEHalLocal, REG_01A8_GE0, v0.x + dst->dstblk.width - 1);
            }
            if(!(flags & E_GE_FLAG_BLT_DST_MIRROR_V))
            {
            GE_CODA_WriteReg(pGEHalLocal, REG_01A4_GE0, v0.y);
            GE_CODA_WriteReg(pGEHalLocal, REG_01AC_GE0, v0.y + dst->dstblk.height- 1);
        }
    }
    }
    else if (u8Rot == 1)
    {
        GE_CODA_WriteReg(pGEHalLocal, REG_01A0_GE0, v0.x + dst->dstblk.height - 1);
        GE_CODA_WriteReg(pGEHalLocal, REG_01A8_GE0, v0.y);
        GE_CODA_WriteReg(pGEHalLocal, REG_01A4_GE0, v0.x + dst->dstblk.height + dst->dstblk.width - 2);
        GE_CODA_WriteReg(pGEHalLocal, REG_01AC_GE0, v0.y + dst->dstblk.height- 1);
    }
    else if (u8Rot == 2)
    {
        GE_CODA_WriteReg(pGEHalLocal, REG_01A0_GE0, v0.x + dst->dstblk.width - 1);
        GE_CODA_WriteReg(pGEHalLocal, REG_01A8_GE0, v0.y + dst->dstblk.height - 1);
        GE_CODA_WriteReg(pGEHalLocal, REG_01A4_GE0, v0.x + dst->dstblk.width + dst->dstblk.width - 2);
        GE_CODA_WriteReg(pGEHalLocal, REG_01AC_GE0, v0.y + dst->dstblk.height + dst->dstblk.height - 2);
    }
    else if (u8Rot == 3)
    {
        GE_CODA_WriteReg(pGEHalLocal, REG_01A0_GE0, v0.x);
        GE_CODA_WriteReg(pGEHalLocal, REG_01A8_GE0, v0.y + dst->dstblk.width - 1);
        GE_CODA_WriteReg(pGEHalLocal, REG_01A4_GE0, v0.x + dst->dstblk.width - 1);
        GE_CODA_WriteReg(pGEHalLocal, REG_01AC_GE0, v0.y + dst->dstblk.height + dst->dstblk.width - 2);
    }

    u16cfg |= GE_CODA_ReadReg(pGEHalLocal, REG_0004_GE0) & GE_CFG_CMDQ_MASK;

    //enable split mode
    if(pGEHalLocal->pGeChipPro->bSupportSpiltMode)
    {
            u16cfg |= GE_CFG_RW_SPLIT;
    }

    GE_CODA_WriteReg(pGEHalLocal, REG_0004_GE0, u16cfg);

    // To check if 1p mode set to TRUE and Non-1p mode limitations
    u16en = GE_CODA_ReadReg(pGEHalLocal, REG_0004_GE0);

    patchBitBltInfo.flags = flags;
    patchBitBltInfo.src.width = src->width;
    patchBitBltInfo.src.height= src->height;
    patchBitBltInfo.dst.dstblk.width =  dst->dstblk.width;
    patchBitBltInfo.dst.dstblk.height=  dst->dstblk.height;
    patchBitBltInfo.scaleinfo = scaleinfo;
    bNonOnePixelMode = GE_NonOnePixelModeCaps(pGEHalLocal, &patchBitBltInfo);

#if(GE_PITCH_256_ALIGNED_UNDER_4P_MODE == 1)
    //[Curry/Kano] Only dst pitch 256 aligned -> 4P mode is avalible.
    if(GE_CODA_ReadReg(pGEHalLocal, REG_00C0_GE0) % (GE_WordUnit))
    {
        bNonOnePixelMode = FALSE; // Must excuted under 1P mode
    }else if(GE_CODA_ReadReg(pGEHalLocal, REG_00CC_GE0) % (GE_WordUnit))
    {
        bNonOnePixelMode = FALSE; // Must excuted under 1P mode
    }
#endif

#if (defined(COLOR_CONVERT_PATCH)&&(COLOR_CONVERT_PATCH == 1))
    u16tmp = GE_CODA_ReadReg(pGEHalLocal, REG_00D0_GE0);
    Srcfmt = (u16tmp&GE_SRC_FMT_MASK)>>GE_SRC_FMT_SHFT;
    Dstfmt = (u16tmp&GE_DST_FMT_MASK)>>GE_DST_FMT_SHFT;

    if(Dstfmt == E_MS_FMT_YUV422 || DIFF_SRC_DST_CLR_FMT(Srcfmt, Dstfmt))
    {
        bNonOnePixelMode = FALSE;
    }
#endif

    if(bNonOnePixelMode && (u16en & GE_EN_ONE_PIXEL_MODE))
    {
        GE_SetOnePixelMode(pGEHalLocal, FALSE);
    }
    else if((!bNonOnePixelMode) && (!(u16en & GE_EN_ONE_PIXEL_MODE)))
    {
        GE_SetOnePixelMode(pGEHalLocal, TRUE);
    }

    u16tmp = GE_CODA_ReadReg(pGEHalLocal, REG_0190_GE0);
    u16tmp_dy = GE_CODA_ReadReg(pGEHalLocal, REG_0194_GE0);
    if(((flags&E_GE_FLAG_BLT_STRETCH)&&((u16tmp!=0x1000)||(u16tmp_dy!=0x1000)))||(u8Rot!=0))
    {
        HAL_GE_EnableCalcSrc_WidthHeight(pGEHalLocal, FALSE);
    }
    else
    {
        HAL_GE_EnableCalcSrc_WidthHeight(pGEHalLocal, TRUE);
    }
    bDstXInv = (u16cmd & (GE_DST_DIR_X_INV))?TRUE:FALSE;
    if(!u8Rot)
    {
        HAL_GE_AdjustDstWin(pGEHalLocal,bDstXInv);
    }
    else
    {
        HAL_GE_AdjustRotateDstWin(pGEHalLocal,u8Rot);
    }

    //if srcfmt == dstfmt, dont use Dither
    u16tmp = GE_CODA_ReadReg(pGEHalLocal, REG_00D0_GE0);
    Srcfmt = (u16tmp&GE_SRC_FMT_MASK)>>GE_SRC_FMT_SHFT;
    Dstfmt = (u16tmp&GE_DST_FMT_MASK)>>GE_DST_FMT_SHFT;
    if( (Srcfmt == Dstfmt)
      ||((Srcfmt == E_MS_FMT_ARGB1555)&&(Dstfmt == E_MS_FMT_ARGB1555_DST))
      ||((Srcfmt == E_MS_FMT_ARGB1555_DST)&&(Dstfmt == E_MS_FMT_ARGB1555))
      )
    {
       HAL_GE_SetDither(pGEHalLocal, FALSE);
    }

    //if Srcfmt = Dstfmt = YUV422 => disable UV_FILTER , else enable
    if(Srcfmt == E_MS_FMT_YUV422 && Dstfmt == E_MS_FMT_YUV422)
    {
        u16tmp = GE_CODA_ReadReg(pGEHalLocal, REG_007C_GE0);
        u16tmp = (u16tmp & ~GE_UV_FILTER);
        GE_CODA_WriteReg(pGEHalLocal, REG_007C_GE0, u16tmp);
    }
    else
    {
        u16tmp = GE_CODA_ReadReg(pGEHalLocal, REG_007C_GE0);
        u16tmp = (u16tmp | GE_UV_FILTER);
        GE_CODA_WriteReg(pGEHalLocal, REG_007C_GE0, u16tmp);
    }

    GE_CODA_WriteReg(pGEHalLocal, REG_0180_GE0, GE_PRIM_BITBLT | u16cmd);

//Backup
    if( u16en & GE_EN_ONE_PIXEL_MODE)
        GE_SetOnePixelMode(pGEHalLocal, TRUE);
    else if(!(u16en & GE_EN_ONE_PIXEL_MODE))
        GE_SetOnePixelMode(pGEHalLocal, FALSE);


    return E_GE_OK;
}


GE_Result HAL_GE_FillRect(GE_CTX_HAL_LOCAL *pGEHalLocal, GE_Rect *rect, MS_U32 color, MS_U32 color2, MS_U32 flags)
{
    MS_U16              u16cmd = 0;
    MS_U16              u16cfg;
    MS_U16              u16fmt;
    MS_U16              u16fmt2;
    GE_ColorDelta       delta;
    MS_U32 color_right_top = 0;
    MS_BOOL bDstXInv = FALSE;

    // clear dirty attributes ---------------------------------------------------------------------
    // reset unused register
    GE_SetRotate(pGEHalLocal, E_GE_ROTATE_0);
    u16cfg = GE_CODA_ReadReg(pGEHalLocal, REG_0004_GE0) & ~(GE_CFG_BLT_STRETCH | GE_CFG_BLT_ITALIC);

    //enable split mode
    u16cfg |= GE_CFG_RW_SPLIT;

    GE_CODA_WriteReg(pGEHalLocal, REG_0004_GE0, u16cfg);
    // clear dirty attributes ---------------------------------------------------------------------

    GE_CODA_WriteReg(pGEHalLocal, REG_01A0_GE0, rect->x);
    GE_CODA_WriteReg(pGEHalLocal, REG_01A4_GE0, rect->y);
    GE_CODA_WriteReg(pGEHalLocal, REG_01A8_GE0, rect->x+rect->width-1);
    GE_CODA_WriteReg(pGEHalLocal, REG_01AC_GE0, rect->y+rect->height-1);

    //Fill Rect must set source width and height for 4P mode check
    GE_CODA_WriteReg(pGEHalLocal, REG_01B8_GE0, rect->width);
    GE_CODA_WriteReg(pGEHalLocal, REG_01BC_GE0, rect->height);
    //Fill Rect must set source color format same as dest color format for 4P mode check
    u16fmt = GE_CODA_ReadReg(pGEHalLocal,REG_00D0_GE0) & GE_DST_FMT_MASK;
    u16fmt2 = ((u16fmt >> GE_DST_FMT_SHFT) & GE_SRC_FMT_MASK);
    u16fmt |= u16fmt2;
    GE_CODA_WriteReg(pGEHalLocal, REG_00D0_GE0, u16fmt);

    GE_CODA_WriteReg(pGEHalLocal, REG_01C0_GE0, color & 0xFFFF);
    GE_CODA_WriteReg(pGEHalLocal, REG_01C4_GE0, color >> 16);

    if((flags&(E_GE_FLAG_RECT_GRADIENT_X|E_GE_FLAG_RECT_GRADIENT_Y))==(E_GE_FLAG_RECT_GRADIENT_X|E_GE_FLAG_RECT_GRADIENT_Y))
    {
        MS_U8     a0, r0, g0, b0;
        MS_U8     a1, r1, g1, b1;
        MS_U8     a2, r2, g2, b2;

        // Get A,R,G,B
        //[TODO] special format
        b0 = (color)       & 0xFF;
        g0 = (color >> 8)  & 0xFF;
        r0 = (color >> 16) & 0xFF;
        a0 = (color >> 24);
        b2 = (color2)       & 0xFF;
        g2 = (color2 >> 8)  & 0xFF;
        r2 = (color2 >> 16) & 0xFF;
        a2 = (color2 >> 24);
        b1 = ((MS_S32)b2-b0)*(MS_S32)rect->width*rect->width/((MS_S32)rect->width*rect->width+(MS_S32)rect->height*rect->height)+(MS_S32)b0;
        g1 = ((MS_S32)g2-g0)*(MS_S32)rect->width*rect->width/((MS_S32)rect->width*rect->width+(MS_S32)rect->height*rect->height)+(MS_S32)g0;
        r1 = ((MS_S32)r2-r0)*(MS_S32)rect->width*rect->width/((MS_S32)rect->width*rect->width+(MS_S32)rect->height*rect->height)+(MS_S32)r0;
        a1 = ((MS_S32)a2-a0)*(MS_S32)rect->width*rect->width/((MS_S32)rect->width*rect->width+(MS_S32)rect->height*rect->height)+(MS_S32)a0;
        color_right_top = ((MS_U32)a1)<<24| ((MS_U32)r1)<<16|((MS_U32)g1)<<8|b1;
    }
    else if(flags&E_GE_FLAG_RECT_GRADIENT_X)
        color_right_top = color2;
    else if(flags&E_GE_FLAG_RECT_GRADIENT_Y)
        color_right_top = color;

    if ( (flags & E_GE_FLAG_RECT_GRADIENT_X) && (rect->width > 1) )
    {
        GE_CalcColorDelta(color, color_right_top, rect->width-1, &delta);
        GE_CODA_WriteReg(pGEHalLocal, REG_01C8_GE0, delta.r & 0xFFFF);
        GE_CODA_WriteReg(pGEHalLocal, REG_01CC_GE0, delta.r >> 16);
        GE_CODA_WriteReg(pGEHalLocal, REG_01D8_GE0, delta.g & 0xFFFF);
        GE_CODA_WriteReg(pGEHalLocal, REG_01DC_GE0, delta.g >> 16);
        GE_CODA_WriteReg(pGEHalLocal, REG_01E8_GE0, delta.b & 0xFFFF);
        GE_CODA_WriteReg(pGEHalLocal, REG_01EC_GE0, delta.b >> 16);
        GE_CODA_WriteReg(pGEHalLocal, REG_01F8_GE0, delta.a);
        u16cmd |= GE_RECT_GRADIENT_H;
    }

    if ( (flags & E_GE_FLAG_RECT_GRADIENT_Y) && (rect->height > 1) )
    {
        GE_CalcColorDelta(color_right_top, color2, rect->height-1, &delta);
        GE_CODA_WriteReg(pGEHalLocal, REG_01D0_GE0, delta.r & 0xFFFF);
        GE_CODA_WriteReg(pGEHalLocal, REG_01D4_GE0, delta.r >> 16);
        GE_CODA_WriteReg(pGEHalLocal, REG_01E0_GE0, delta.g & 0xFFFF);
        GE_CODA_WriteReg(pGEHalLocal, REG_01E4_GE0, delta.g >> 16);
        GE_CODA_WriteReg(pGEHalLocal, REG_01F0_GE0, delta.b & 0xFFFF);
        GE_CODA_WriteReg(pGEHalLocal, REG_01F4_GE0, delta.b >> 16);
        GE_CODA_WriteReg(pGEHalLocal, REG_01FC_GE0, delta.a);
        u16cmd |= GE_RECT_GRADIENT_V;
    }

    HAL_GE_AdjustDstWin(pGEHalLocal,bDstXInv);

    GE_CODA_WriteReg(pGEHalLocal, REG_0180_GE0, GE_PRIM_RECT | u16cmd);

    return E_GE_OK;
}
