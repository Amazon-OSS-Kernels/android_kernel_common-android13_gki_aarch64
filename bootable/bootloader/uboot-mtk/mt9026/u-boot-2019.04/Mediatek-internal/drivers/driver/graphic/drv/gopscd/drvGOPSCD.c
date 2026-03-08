// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

#define DRV_GOPSC_EX_C

#include "MsCommon.h"
#include "MsOS.h"

#include "drvGOPSCD.h"
#include "halGOPSCD.h"
#include "regGOPSCD.h"

#ifdef MSOS_TYPE_LINUX_KERNEL
#include <linux/string.h>
#include <asm/div64.h>
#else
#include <string.h>
//#define do_div(x,y) ((x)/=(y))
#endif


//-------------------------------------------------------------------------------------------------
//  Local Defines
//-------------------------------------------------------------------------------------------------

typedef struct
{
    MS_U16 xSkipStart;
    MS_U16 xSkipEnd;
    MS_U16 ySkipStart;
    MS_U16 ySkipEnd;
    MS_U16 srcWidth;
    MS_U16 destWidth;
    MS_U16 srcHeight;
    MS_U16 destHeight;
    MS_BOOL bSkipX;
    MS_BOOL bSkipY;
    MS_BOOL bScalingX;
    MS_BOOL bScalingY;
}Drv_GOPSC_SizeInfo;

Drv_GOPSC_SizeInfo dataGOPSC_SizeInfo;
static MS_U8 u8formula = 1; //0 - Not P2I formula, 1 - P2I formula

static GOPSC_Drv_Result _HalDrvResultConvert(GOPSC_Hal_Result halRet)
{
    GOPSC_Drv_Result drvRet = En_Drv_GOPSC_FAIL;
    switch(halRet)
    {
        case En_Hal_GOPSC_OK:
            drvRet = En_Drv_GOPSC_OK;
            break;
        case En_Hal_GOPSC_INVALID_PARAM:
            drvRet = En_Drv_GOPSC_INVALID_PARAM;
            break;
        case En_Hal_GOPSC_NotSupport:
            drvRet = En_Drv_GOPSC_NotSupport;
            break;
        case En_Hal_GOPSC_FAIL:
        default:
            drvRet = En_Drv_GOPSC_FAIL;
    }
    return drvRet;
}

GOPSC_Drv_Result MDrv_GOPSC_Init_riu_base(MS_U32 DeviceId, MS_VIRT virtriu_base)
{
    return _HalDrvResultConvert(HAL_GOPSC_Init_riu_base(virtriu_base));
}

GOPSC_Drv_Result MDrv_GOPSC_Init(MS_U32 DeviceId)
{
    //Initial Local Variable
    //memset(&gGopSC_Ref, 0 , sizeof(GOP_SC_REF) );
    //memset(&gGopSC_Info, 0 , sizeof(DRV_GOP_SC_INFO) );

    //GOP_Rdy reg always trigger
    MS_U16 value = HAL_GOPSC_Read2Byte(REG_GOP_SC_00_L);
    value |= 0x40;
    HAL_GOPSC_Write2Byte(REG_GOP_SC_00_L,value);
    //No matter Cubic mode or bilinear mode, always set PQ table in initial phase
    HAL_GOPSC_InitTable();
    //Default scaling mode is cubic
    HAL_GOPSC_SetVScalingMode(CUBIC); //init use cubic mode
    //source 0 is GOP, 1 is OP.
    HAL_GOPSC_MuxSel(0); //init source is GOP
    memset(&dataGOPSC_SizeInfo,0,sizeof(dataGOPSC_SizeInfo));
    return En_Drv_GOPSC_OK;
}

GOPSC_Drv_Result MDrv_GOPSC_Enable(MS_U32 DeviceId, MS_BOOL enable)
{
    MS_U16 value = HAL_GOPSC_Read2Byte(REG_GOP_SC_00_L);
    if(TRUE == enable)
    {
        value |= 0x8000;
    }
    else
    {
        value &= ~0x8000;
    }
    HAL_GOPSC_Write2Byte(REG_GOP_SC_00_L,value);
    return En_Drv_GOPSC_OK;
}

GOPSC_Drv_Result MDrv_GOPSC_SetSource(MS_U32 DeviceId, MS_U8 u8SelSrc)
{
    return _HalDrvResultConvert(HAL_GOPSC_MuxSel(u8SelSrc));
}

GOPSC_Drv_Result MDrv_GOPSC_SetVScalingMode(MS_U32 DeviceId, MS_U8 mode)
{
    //Initial Local Variable
    return _HalDrvResultConvert(HAL_GOPSC_SetVScalingMode(mode));
}

GOPSC_Drv_Result MDrv_GOPSC_SetHVSP(MS_U32 DeviceId, MS_BOOL bHspEnable, MS_BOOL bVspEnable, MS_U16 srcWidth, MS_U16 srcHeight,MS_U16 destWidth, MS_U16 destHeight)
{
    //while driver change size, enter scaling factor also
    //H factor = (output * 1048576 / input) +1
    //V factor = ((input-1) * 1048576 / (output-1))  +1

	//V factor = (input) * 1048576 / (output) if GOP use GOPSC result and do P2I
    MS_U32 HScaleFac = (bHspEnable)? (destWidth * 1048576 / srcWidth) + 1 : 0;
    MS_U32 VScaleFac = 0x100000;
	if(bVspEnable)
	{
		if(u8formula == 1)
		{
			VScaleFac = ((srcHeight) * 1048576 / (destHeight));
		}
		else
		{
			VScaleFac = (((srcHeight - 1) * 1048576 / (destHeight - 1)) + 1);
		}
	}
    if(destWidth == srcWidth)
    {
        HScaleFac = 0;
    }
    if(srcHeight == destHeight)
    {
        if(u8formula == 1)
        {
        	VScaleFac = 0x100000;
        }
		else
		{
			VScaleFac = 0x100001;
		}
    }
    if(HScaleFac != 0)
    {
        HAL_GOPSC_Write2Byte(REG_GOP_SC_38_L,destWidth);
    }
    else
    {
        HAL_GOPSC_Write2Byte(REG_GOP_SC_38_L,srcWidth);
    }

    if(VScaleFac != 0)
    {
        HAL_GOPSC_Write2Byte(REG_GOP_SC_52_L,destHeight);
    }
    else
    {
        HAL_GOPSC_Write2Byte(REG_GOP_SC_52_L,srcHeight);
    }

    GOPSC_Hal_Result ret = HAL_GOPSC_SetHVSP(srcWidth,srcHeight,destWidth,destHeight);

    if(En_Hal_GOPSC_OK != ret)
    {
        return _HalDrvResultConvert(ret);
    }

    return _HalDrvResultConvert(HAL_GOPSC_Set_ScalingFactor(VScaleFac,HScaleFac));
}

GOPSC_Drv_Result MDrv_GOPSC_SetSkipPixel(MS_U32 DeviceId, MS_U8 bSkipX, MS_U8 bSkipY,
    MS_U16 xStart, MS_U16 xEnd,MS_U16 yStart, MS_U16 yEnd)
{
    return _HalDrvResultConvert(HAL_GOPSC_SetSkipPixel(bSkipX, bSkipY, xStart, xEnd, yStart, yEnd));
}

GOPSC_Drv_Result MDrv_GOPSC_SetFieldStartPosition(MS_U32 DeviceId, MS_BOOL bEnable,
                            MS_U16 topFieldOffset, MS_U16 bottomFieldOffset)
{
    //FieldOffset register is 8 bits in K3. for feature extension, use MS_U16
    return _HalDrvResultConvert(HAL_GOPSC_Set_FieldStartPosition(bEnable,topFieldOffset,bottomFieldOffset));
}

GOPSC_Drv_Result MDrv_GOPSC_SetDelayLineNumber(MS_U32 DeviceId, MS_U16 dlyLine)
{
    MS_U16 value = HAL_GOPSC_Read2Byte(REG_GOP_SC_3F_L);
    value &= ~0xff;
    value |= (dlyLine & 0xff);
    HAL_GOPSC_Write2Byte(REG_GOP_SC_3F_L,value);
    return En_Drv_GOPSC_OK;
}

GOPSC_Drv_Result MDrv_GOPSC_P2IEnable(MS_U32 DeviceId, MS_BOOL enable)
{
    return _HalDrvResultConvert(HAL_GOPSC_P2I(enable));
}

GOPSC_Drv_Result MDrv_GOPSC_HalfMode(MS_U32 DeviceId, MS_BOOL enable)
{
    MS_U16 value = HAL_GOPSC_Read2Byte(REG_GOP_SC_00_L);
    if(TRUE == enable)
    {
        value |= 0x4000;
    }
    else
    {
        value &= ~0x4000;
    }
    HAL_GOPSC_Write2Byte(REG_GOP_SC_00_L,value);
    return En_Drv_GOPSC_OK;
}

GOPSC_Drv_Result MDrv_GOPSC_TriggerOnce(MS_U32 DeviceId, MS_U8 FrameNum)
{
    return _HalDrvResultConvert(HAL_GOPSC_TriggerOnce(FrameNum));
}

GOPSC_Drv_Result MDrv_GOPSC_SetOutputControlRate(MS_U32 DeviceId, MS_BOOL bEnable,
    MS_U16 SrcWidth, MS_U16 SrcHeight, MS_U16 DestWidth, MS_U16 DestHeight, MS_BOOL bInterlace)
{
    //FieldOffset register is 8 bits in K3. for feature extension, use MS_U16
    MS_U16 value = HAL_GOPSC_Read2Byte(REG_GOP_SC_55_L);
    if(bEnable)
    {
        //OUT_RATE = DST_HSIZE*(DST_VSIZE>>P2I)*256/SRC_HSIZE/SRC_VSIZE;
        MS_U32 OutRate = (((MS_U32)DestWidth*((MS_U32)DestHeight>>(bInterlace)? 1 : 0)*256)/(MS_U32)SrcWidth)/(MS_U32)SrcHeight;
        value &= ~0xff;
        value |= (OutRate & 0xff);
    }
    else
    {
        value &= ~0xff;
        value |= 0x0;
    }
    HAL_GOPSC_Write2Byte(REG_GOP_SC_55_L,value);
    return En_Drv_GOPSC_OK;
}

GOPSC_Drv_Result MDrv_GOPSC_SetScalingFormula(MS_U32 DeviceId, MS_U8 Formula)
{
	u8formula = Formula;
    return En_Drv_GOPSC_OK;
}


