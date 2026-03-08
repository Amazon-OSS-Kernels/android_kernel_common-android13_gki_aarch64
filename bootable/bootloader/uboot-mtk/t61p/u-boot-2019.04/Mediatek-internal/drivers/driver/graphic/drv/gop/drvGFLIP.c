// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

#define _MDRV_GFLIP_C

//=============================================================================
// Include Files
//=============================================================================
#include "MsCommon.h"
#include "regGOP.h"
#include "halGOP.h"
#include "drvGFLIP.h"
//#include "halCHIP.h"
#if defined(MSOS_TYPE_LINUX)
    #include <sys/ioctl.h>
    #include <unistd.h>
    #include <fcntl.h> // O_RDWR
    #include "mdrv_gflip_io.h"
    #include <string.h>
#elif   defined(MSOS_TYPE_LINUX_KERNEL)
    #include "mdrv_gflip_io.h"
#endif
#include "drvGOP_priv.h"

//=============================================================================
// Compile options
//=============================================================================

//=============================================================================
// Debug Macros
//=============================================================================
#ifdef CONFIG_GOP_DEBUG_LEVEL
#define GFLIP_DEBUG
#endif
#ifdef GFLIP_DEBUG
    #define GFLIP_PRINT(fmt, args...)      printf("[GFlip (Driver)][%05d] " fmt, __LINE__, ## args)
    #define GFLIP_ASSERT(_cnd, _fmt, _args...)                   \
                                    if (!(_cnd)) {              \
                                        GFLIP_PRINT(_fmt, ##_args);  \
                                    }
#else
    #define GFLIP_PRINT(_fmt, _args...)
    #define GFLIP_ASSERT(_cnd, _fmt, _args...)
#endif

//=============================================================================
// Macros
//=============================================================================

//=============================================================================
// Global Variables
//=============================================================================
#if defined(MSOS_TYPE_LINUX) || defined(MSOS_TYPE_LINUX_KERNEL)
#else
GOP_CTX_DRV_LOCAL*pGFLIP_IntGOPDrvLocalCtx = NULL;
#endif
//////////////////////////////////////////////////////////////////////////////////////////////////////

//=============================================================================
// GFLIP Driver Function
//=============================================================================
E_GFLIP_CFD_CFIO _MDrv_GFLIP_Convert_CFIO(EN_GOP_CFD_CFIO enFormat)
{
    switch(enFormat)
    {
        case E_GOP_CFD_CFIO_RGB_NOTSPECIFIED:
            return E_GFLIP_CFD_CFIO_RGB_NOTSPECIFIED;
        case E_GOP_CFD_CFIO_RGB_BT601_625:
            return E_GFLIP_CFD_CFIO_RGB_BT601_625;
        case E_GOP_CFD_CFIO_RGB_BT601_525:
            return E_GFLIP_CFD_CFIO_RGB_BT601_525;
        case E_GOP_CFD_CFIO_RGB_BT709:
            return E_GFLIP_CFD_CFIO_RGB_BT709;
        case E_GOP_CFD_CFIO_RGB_BT2020:
            return E_GFLIP_CFD_CFIO_RGB_BT2020;
        case E_GOP_CFD_CFIO_SRGB:
            return E_GFLIP_CFD_CFIO_SRGB;
        case E_GOP_CFD_CFIO_ADOBE_RGB:
            return E_GFLIP_CFD_CFIO_ADOBE_RGB;
        case E_GOP_CFD_CFIO_YUV_NOTSPECIFIED:
            return E_GFLIP_CFD_CFIO_YUV_NOTSPECIFIED;
        case E_GOP_CFD_CFIO_YUV_BT601_625:
            return E_GFLIP_CFD_CFIO_YUV_BT601_625;
        case E_GOP_CFD_CFIO_YUV_BT601_525:
            return E_GFLIP_CFD_CFIO_YUV_BT601_525;
        case E_GOP_CFD_CFIO_YUV_BT709:
            return E_GFLIP_CFD_CFIO_YUV_BT709;
        case E_GOP_CFD_CFIO_YUV_BT2020_NCL:
            return E_GFLIP_CFD_CFIO_YUV_BT2020_NCL;
        case E_GOP_CFD_CFIO_YUV_BT2020_CL:
            return E_GFLIP_CFD_CFIO_YUV_BT2020_CL;
        case E_GOP_CFD_CFIO_XVYCC_601:
            return E_GFLIP_CFD_CFIO_XVYCC_601;
        case E_GOP_CFD_CFIO_XVYCC_709:
            return E_GFLIP_CFD_CFIO_XVYCC_709;
        case E_GOP_CFD_CFIO_SYCC601:
            return E_GFLIP_CFD_CFIO_SYCC601;
        case E_GOP_CFD_CFIO_ADOBE_YCC601:
            return E_GFLIP_CFD_CFIO_ADOBE_YCC601;
        case E_GOP_CFD_CFIO_DOLBY_HDR_TEMP:
            return E_GFLIP_CFD_CFIO_DOLBY_HDR_TEMP;
        case E_GOP_CFD_CFIO_SYCC709:
            return E_GFLIP_CFD_CFIO_SYCC709;
        case E_GOP_CFD_CFIO_DCIP3_THEATER:
            return E_GFLIP_CFD_CFIO_DCIP3_THEATER;
        case E_GOP_CFD_CFIO_DCIP3_D65:
            return E_GFLIP_CFD_CFIO_DCIP3_D65;
        default:
            return E_GFLIP_CFD_CFIO_RESERVED_START;
    }
}

E_GFLIP_CFD_MC_FORMAT _MDrv_GFLIP_Convert_MC_Format(EN_GOP_CFD_MC_FORMAT enFormat)
{
    switch(enFormat)
    {
        case E_GOP_CFD_MC_FORMAT_RGB:
            return E_GFLIP_CFD_MC_FORMAT_RGB;
        case E_GOP_CFD_MC_FORMAT_YUV422:
            return E_GFLIP_CFD_MC_FORMAT_YUV422;
        case E_GOP_CFD_MC_FORMAT_YUV444:
            return E_GFLIP_CFD_MC_FORMAT_YUV444;
        case E_GOP_CFD_MC_FORMAT_YUV420:
            return E_GFLIP_CFD_MC_FORMAT_YUV420;
        default:
            return E_GFLIP_CFD_MC_FORMAT_RESERVED_START;
    }
}

E_GFLIP_CFD_CFIO_RANGE _MDrv_GFLIP_Convert_CFIO_Range(EN_GOP_CFD_CFIO_RANGE enRange)
{
    switch(enRange)
    {
        case E_GOP_CFD_CFIO_RANGE_LIMIT:
            return E_GFLIP_CFD_CFIO_RANGE_LIMIT;
        case E_GOP_CFD_CFIO_RANGE_FULL:
            return E_GFLIP_CFD_CFIO_RANGE_FULL;
        default:
            return E_GFLIP_CFD_CFIO_RANGE_RESERVED_START;
    }
}

MS_BOOL MDrv_GOP_GFLIP_CSC_Tuning(MS_GOP_CTX_LOCAL*pstGOPCtx,MS_U32 u32GOPNum,ST_GOP_CSC_PARAM *pstCSCParam,ST_DRV_GOP_CFD_OUTPUT *pstCFDOut)
{
	return FALSE;
}

