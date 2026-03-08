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
#include <vsprintf.h>
#include "utopia.h"
#include "utopia_dapi.h"
#include "MsTypes.h"
#include "drvGOP.h"
#include "halGOP.h"
#include "regGOP.h"
#include "drvMMIO.h"
#include "MsOS.h"
#include "drvGFLIP.h"
#include "halCHIP.h"
#include "drvGOP_priv.h"
#include "apiXC.h"
#include <dts_parser.h>

#if defined(MSOS_TYPE_LINUX_KERNEL)
#include <linux/slab.h>
#define free kfree
#define malloc(size) kmalloc((size), GFP_KERNEL)
#endif

//-------------------------------------------------------------------------------------------------
//  Local Compiler Options
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Local Defines
//-------------------------------------------------------------------------------------------------
#ifdef MSOS_TYPE_LINUX_KERNEL
#define atoi(str) simple_strtoul(((str != NULL) ? str : ""), NULL, 0)
#endif

 //check 1 time/1ms, maximum vsync time is 1000/48 ms(scaler 48hz output), so check 22 times at most

#ifdef GOP_4K2K30
#define GOP_TIMEOUT_CNT_OS              40UL
#else
#define GOP_TIMEOUT_CNT_OS              22UL
#endif
#define GOP_TIMEOUT_CNT_OS_YIELD  	    0x10000UL
#define GOP_TIMEOUT_CNT_NOS     	    0x100000UL

#define PALETTE_BY_REGDMA               1UL
#define msWarning(c)                    do {} while (0)
#define msFatal(c)                      do {} while (0)
#define ERR_FB_ID_OUT_OF_RANGE          0x0300UL
#define ERR_FB_ID_NOT_ALLOCATED         0x0301UL
#define ERR_FB_ID_ALREADY_ALLOCATED     0x0302UL
#define ERR_FB_OUT_OF_MEMORY            0x0303UL
#define ERR_FB_OUT_OF_ENTRY             0x0304UL
#define PAR_IGNORE                      0xFFUL               // PARAM constant for ignore

#define MAX_CSC_GAIN (2047UL)

#ifndef INSTANT_PRIVATE
GFLIP_REGS_SAVE_AREA gstGOP_STRPrivate;
#endif

//=============================================================
// Debug Log
#include "ULog.h"
#include <debug_impl.h>
MS_U32 u32GOPDbgLevel_drv;

#ifdef CONFIG_GOP_DEBUG_LEVEL
// Debug Logs, level form low(INFO) to high(FATAL, always show)
// Function information, ex function entry
#define GOP_D_INFO(x, args...) {UBOOT_INFO(x, ##args);}
// Warning, illegal paramter but can be self fixed in functions
#define GOP_D_WARN(x, args...) {UBOOT_INFO(x, ##args);}
//  Need debug, illegal paramter.
#define GOP_D_DBUG(x, args...) {UBOOT_DEBUG(x, ##args);}
// Error, function will be terminated but system not crash
#define GOP_D_ERR(x, args...) {UBOOT_ERROR(x, ##args);}
// Critical, system crash. (ex. assert)
#define GOP_D_FATAL(x, args...) {UBOOT_ERROR(x, ##args);}
#else
#define GOP_D_INFO(x, args...)
// Warning, illegal paramter but can be self fixed in functions
#define GOP_D_WARN(x, args...)
//  Need debug, illegal paramter.
#define GOP_D_DBUG(x, args...)
// Error, function will be terminated but system not crash
#define GOP_D_ERR(x, args...)
// Critical, system crash. (ex. assert)
#define GOP_D_FATAL(x, args...)
#endif
//=============================================================

#if GOP_LOCK_SUPPORT
//Move GOP mutex define here for gflip to access
//But we do not suggest to use it in DRV layer in normal case, try use it in API layer
    #define  _ObtainMutex() \
    {\
        if(pGOPDrvLocalCtx->apiCtxLocal.pGOPCtxShared == NULL){return FALSE;} \
        if(FALSE == MsOS_ObtainMutex(pGOPDrvLocalCtx->apiCtxLocal.s32GOPMutex, MSOS_WAIT_FOREVER)){APIGOP_ASSERT(0, (GOP_D_FATAL("DRV_GOP_ENTRY: Fatal Error, Obtain GOP mutex fail\n")));}  \
        APIGOP_ASSERT(0==pGOPDrvLocalCtx->apiCtxLocal.s32GOPMutexCnt, (GOP_D_FATAL("DRV_GOP_ENTRY: Fatal Error, No GOP mutex to release\n")));   \
        pGOPDrvLocalCtx->apiCtxLocal.s32GOPMutexCnt++;  \
        pGOPDrvLocalCtx->apiCtxLocal.s32GOPLock=MsOS_GetOSThreadID();  \
        if((pGOPDrvLocalCtx->apiCtxLocal.u32GOPClientId!=pGOPDrvLocalCtx->apiCtxLocal.pGOPCtxShared->u32LstGOPClientId)  \
            &&pGOPDrvLocalCtx->apiCtxLocal.pGOPCtxShared->u32LstGOPClientId)   \
        {   \
            GOP_GOP_Restore_Ctx(&(pGOPDrvLocalCtx->apiCtxLocal));  \
            pGOPDrvLocalCtx->apiCtxLocal.pGOPCtxShared->u32LstGOPClientId = pGOPDrvLocalCtx->apiCtxLocal.u32GOPClientId;  \
        }  \
    }

    //Warning: we do not suggest to use it in DRV layer in normal case, try use it in API layer
    #define DRV_GOP_ENTRY()   \
    {     \
        if (pGOPDrvLocalCtx->apiCtxLocal.s32GOPMutex < 0)  \
        {   \
            GOP_D_DBUG("DRV_GOP_ENTRY: Fatal Error, No GOP mutex to obtain\n"); \
        }   \
        if(0 < pGOPDrvLocalCtx->apiCtxLocal.s32GOPMutexCnt) \
        {    \
            if (pGOPDrvLocalCtx->apiCtxLocal.s32GOPLock!=MsOS_GetOSThreadID())\
            {_ObtainMutex(); }\
            else\
            {pGOPDrvLocalCtx->apiCtxLocal.s32GOPMutexCnt++;}  \
        }    \
        else \
        {_ObtainMutex();} \
     }

    //Warning: we do not suggest to use it in DRV layer in normal case, try use it in API layer
    #define DRV_GOP_RETURN() \
    {   \
        APIGOP_ASSERT((pGOPDrvLocalCtx->apiCtxLocal.s32GOPLock==MsOS_GetOSThreadID()), (GOP_D_FATAL("\nDRV_GOP_RETURN: Fatal Error, Task ID mismatch[%d]->[%d] \n", pGOPDrvLocalCtx->apiCtxLocal.s32GOPLock, (MS_S32)MsOS_GetOSThreadID())));    \
        APIGOP_ASSERT((0<pGOPDrvLocalCtx->apiCtxLocal.s32GOPMutexCnt), (GOP_D_FATAL("\nDRV_GOP_RETURN: Fatal Error, No Mutex to release[Cnt=%d]\n", pGOPDrvLocalCtx->apiCtxLocal.s32GOPMutexCnt)));    \
        if(1 < pGOPDrvLocalCtx->apiCtxLocal.s32GOPMutexCnt)  \
        {pGOPDrvLocalCtx->apiCtxLocal.s32GOPMutexCnt--;}  \
        else if(0 < pGOPDrvLocalCtx->apiCtxLocal.s32GOPMutexCnt)  \
        {   \
          pGOPDrvLocalCtx->apiCtxLocal.s32GOPLock = -1;  \
          pGOPDrvLocalCtx->apiCtxLocal.s32GOPMutexCnt--;\
          if(FALSE == MsOS_ReleaseMutex(pGOPDrvLocalCtx->apiCtxLocal.s32GOPMutex)){APIGOP_ASSERT(0, (GOP_D_FATAL("DRV_GOP_ENTRY: Fatal Error, Release GOP mutex fail\n")));}  \
        }   \
        else{GOP_D_DBUG("\nDRV_GOP_RETURN: Fatal Error, No mutex to release\n");}   \
    }
#else
    #define DRV_GOP_ENTRY() while(0)
    #define DRV_GOP_RETURN()  while(0)
#endif

#if GOP_XC_LOCK_SUPPORT
#if GOP_XC_LOCK_DEBUG
///XC MUTEX for GOP driver access SC registers
///Place it in Drv layer NOT the API Layer is to avoid Mutex rase condition, because GOP functions will use some XC call back functions
    #define  _Obtain_XCMutex() \
    {\
        pGOPDrvLocalCtx->apiCtxLocal.s32GOPXC_MutexCnt++;  \
        pGOPDrvLocalCtx->apiCtxLocal.s32GOPXC_Lock=MsOS_GetOSThreadID();  \
        GOP_D_INFO("[%s][%05d][%d]XC_LOCK+++[%d]\n",__FUNCTION__,__LINE__, pGOPDrvLocalCtx->apiCtxLocal.s32GOPXC_Lock, pGOPDrvLocalCtx->apiCtxLocal.s32GOPXC_MutexCnt);    \
        MsOS_ObtainMutex(pGOPDrvLocalCtx->apiCtxLocal.s32GOPXC_Mutex, MSOS_WAIT_FOREVER);  \
    }

    #define  _Release_XCMutex() \
    {   \
        pGOPDrvLocalCtx->apiCtxLocal.s32GOPXC_MutexCnt--;\
        GOP_D_INFO("[%s][%05d][%d]XC_LOCK---[%d]\n",__FUNCTION__,__LINE__, pGOPDrvLocalCtx->apiCtxLocal.s32GOPXC_Lock, pGOPDrvLocalCtx->apiCtxLocal.s32GOPXC_MutexCnt);    \
        MsOS_ReleaseMutex(pGOPDrvLocalCtx->apiCtxLocal.s32GOPXC_Mutex);  \
    }
#else
    #define  _Obtain_XCMutex() \
    {\
        pGOPDrvLocalCtx->apiCtxLocal.s32GOPXC_MutexCnt++;  \
        pGOPDrvLocalCtx->apiCtxLocal.s32GOPXC_Lock=MsOS_GetOSThreadID();  \
        MsOS_ObtainMutex(pGOPDrvLocalCtx->apiCtxLocal.s32GOPXC_Mutex, MSOS_WAIT_FOREVER);  \
    }

    #define  _Release_XCMutex() \
    {   \
        pGOPDrvLocalCtx->apiCtxLocal.s32GOPXC_MutexCnt--;\
        MsOS_ReleaseMutex(pGOPDrvLocalCtx->apiCtxLocal.s32GOPXC_Mutex);  \
    }
#endif

#define DRV_GOP_XC_ENTRY()   \
{     \
    if(0 < pGOPDrvLocalCtx->apiCtxLocal.s32GOPXC_MutexCnt) \
    {    \
        if (pGOPDrvLocalCtx->apiCtxLocal.s32GOPXC_Lock!=MsOS_GetOSThreadID())   \
        {_Obtain_XCMutex(); }   \
        else   \
        {pGOPDrvLocalCtx->apiCtxLocal.s32GOPXC_MutexCnt++;}  \
    }    \
    else \
    {_Obtain_XCMutex();} \
}

#define DRV_GOP_XC_RETURN() \
{   \
    if(pGOPDrvLocalCtx->apiCtxLocal.s32GOPXC_Lock != MsOS_GetOSThreadID())  \
        {GOP_D_DBUG("DRV_GOP_XC_RETURN: Fatal Error, Task ID mismatch [%d]->[%d]\n", pGOPDrvLocalCtx->apiCtxLocal.s32GOPXC_Lock, (MS_S32)MsOS_GetOSThreadID());}    \
    if(0<pGOPDrvLocalCtx->apiCtxLocal.s32GOPXC_MutexCnt){    \
    if(1 < pGOPDrvLocalCtx->apiCtxLocal.s32GOPXC_MutexCnt)  \
    {pGOPDrvLocalCtx->apiCtxLocal.s32GOPXC_MutexCnt--;}  \
    else   \
    {   \
        _Release_XCMutex();   \
    }   \
    }else{GOP_D_DBUG("DRV_GOP_XC_RETURN: Fatal Error, No mutex to release\n");}   \
}
#else
#define DRV_GOP_XC_ENTRY() while(0)
#define DRV_GOP_XC_RETURN()  while(0)
#endif

#if GOP_XC_SEMPHORE_PROTECT
#define E_XC_POOL_ID_INTERNAL_REGISTER  0
#if GOP_XC_LOCK_DEBUG
#define DRV_GOP_XC_GET_SEMPHORE()   \
{     \
    if (!pGOPDrvLocalCtx->halCtxLocal.pGopChipPro->bXCDirrectBankSupport)   \
    {       \
        if(_GOP_Get_XCSemaphore() != UTOPIA_STATUS_SUCCESS)                 \
        {GOP_D_DBUG("DRV_GOP_XC_GET_SEMPHORE: Fatal Error, %s get XC semaphore fail\n", __FUNCTION__);} \
    }   \
}

#define DRV_GOP_XC_RELEASE_SEMPHORE() \
{   \
    if (!pGOPDrvLocalCtx->halCtxLocal.pGopChipPro->bXCDirrectBankSupport)   \
    {       \
        if (_GOP_Release_XCSemaphore() != UTOPIA_STATUS_SUCCESS) \
        {GOP_D_DBUG("DRV_GOP_XC_RELEASE_SEMPHORE: Fatal Error, %s release XC semaphore fail\n",__FUNCTION__);}  \
    }   \
}
#else
#define DRV_GOP_XC_GET_SEMPHORE()   \
{     \
    if (!pGOPDrvLocalCtx->halCtxLocal.pGopChipPro->bXCDirrectBankSupport)   \
    {       \
        _GOP_Get_XCSemaphore();                         \
    }   \
}

#define DRV_GOP_XC_RELEASE_SEMPHORE() \
{   \
    if (!pGOPDrvLocalCtx->halCtxLocal.pGopChipPro->bXCDirrectBankSupport)   \
    {       \
        _GOP_Release_XCSemaphore();     \
    }   \
}
#endif
#else
#define DRV_GOP_XC_GET_SEMPHORE() while(0)
#define DRV_GOP_XC_RELEASE_SEMPHORE()  while(0)
#endif
#define TESTPATTERN_CALC(x) ((x&0xF0)<<4)+(x&0xF)

// Define return values of check align
#define CHECKALIGN_SUCCESS              1UL
#define CHECKALIGN_FORMAT_FAIL          2UL
#define CHECKALIGN_PARA_FAIL            3UL

#define FRAMEPACKING_1080P_GAP (45)
#define FRAMEPACKING_720P_GAP (30)

#define GOP_ASSERT(x) {printf("\33[0;36m   error in %s:%d    \33[m \n",__FUNCTION__,__LINE__);}

//-------------------------------------------------------------------------------------------------
//  Local Structures
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Global Variables
//-------------------------------------------------------------------------------------------------
GOP_CHIP_PROPERTY gGopChipPro;
MS_U32 u32ChipVer;
MS_U16 gu32GopCapsAFBC[2] = {GOP_CAPS_OFFSET_GOP0_AFBC,
	GOP_CAPS_OFFSET_GOP_AFBC_COMMON};
MS_U16 gu32GopCapsHDup[2] = {GOP_CAPS_OFFSET_GOP0_HDUPLICATE,
	GOP_CAPS_OFFSET_GOP_HDUPLICATE_COMMON};
MS_U16 gu32GopCapsH2Tap[2] = {GOP_CAPS_OFFSET_GOP0_H2TAP,
	GOP_CAPS_OFFSET_GOP_H2TAP_COMMON};
MS_U16 gu32GopCapsH4Tap[2] = {GOP_CAPS_OFFSET_GOP0_H4TAP,
	GOP_CAPS_OFFSET_GOP_H4TAP_COMMON};
MS_U16 gu32GopCapsV4Tap[2] = {GOP_CAPS_OFFSET_GOP0_V4TAP,
	GOP_CAPS_OFFSET_GOP_V4TAP_COMMON};
MS_U16 gu32GopCapsVBilinear[2] = {GOP_CAPS_OFFSET_GOP0_VBILINEAR,
	GOP_CAPS_OFFSET_GOP_VBILINEAR_COMMON};
MS_U32 gu32GopCapsVScaleModeMsk[2] = {GOP_CAPS_OFFSET_GOP0_VSCALE_MODE_MSK,
	GOP_CAPS_OFFSET_GOP_VSCALE_MODE_MSK_COMMON};

#if GOP_XC_SEMPHORE_PROTECT
extern void* pInstantGOP_XC;
extern void* g_pXCResource[];
#endif
extern MS_BOOL (*_fpXCIsInterlace)(void);
extern MS_U16 (*_fpXCGetCapHStart)(void);

//-------------------------------------------------------------------------------------------------
//  Local Variables
//-------------------------------------------------------------------------------------------------
#ifdef INSTANT_PRIVATE
#define g_gopDrvCtxLocal  psGOPInstPri->g_gopDrvCtxLocal
#else
GOP_CTX_DRV_LOCAL g_gopDrvCtxLocal;
#endif

#if defined(MSOS_TYPE_LINUX) || defined(MSOS_TYPE_LINUX_KERNEL)
#else
GOP_CTX_DRV_SHARED g_gopDrvCtxShared;
#endif

#if GOP_XC_SEMPHORE_PROTECT
static MS_BOOL _XC_Inited = FALSE;
#endif

MS_U16 _GopVStretchTable[MAX_GOP_VSTRETCH_MODE_SUPPORT][GOP_VSTRETCH_TABLE_NUM] =
{
        //liner
        {0x14,0x3c,0x13,0x3d,
         0x11,0x3f,0x10,0x00,
         0x0e,0x02,0x0b,0x05,
         0x09,0x07,0x08,0x08,
         0x14,0x3c,0x13,0x3d,
         0x11,0x3f,0x10,0x00,
         0x0e,0x02,0x0b,0x05,
         0x09,0x07,0x08,0x08,
        },
        //
        {0x0f,0x01,0x0e,0x02,
         0x0d,0x03,0x0c,0x04,
         0x0b,0x05,0x0a,0x06,
         0x09,0x07,0x08,0x08,
         0x0f,0x01,0x0e,0x02,
         0x0d,0x03,0x0c,0x04,
         0x0b,0x05,0x0a,0x06,
         0x09,0x07,0x08,0x08,
        },
        //
        {0x0f,0x01,0x0e,0x02,
         0x0d,0x03,0x0c,0x04,
         0x0b,0x05,0x0a,0x06,
         0x09,0x07,0x08,0x08,
         0x0f,0x01,0x0e,0x02,
         0x0d,0x03,0x0c,0x04,
         0x0b,0x05,0x0a,0x06,
         0x09,0x07,0x08,0x08,
        },
        //liner Gain0
        {0x0f,0x01,0x0e,0x02,
         0x0d,0x03,0x0c,0x04,
         0x0b,0x05,0x0a,0x06,
         0x09,0x07,0x08,0x08,
         0x0f,0x01,0x0e,0x02,
         0x0d,0x03,0x0c,0x04,
         0x0b,0x05,0x0a,0x06,
         0x09,0x07,0x08,0x08,
        },
        //liner Gain1
        {0x10,0x00,0x0F,0x01,
         0x0E,0x02,0x0D,0x03,
         0x0C,0x04,0x0B,0x05,
         0x0A,0x06,0x08,0x08,
         0x10,0x00,0x0F,0x01,
         0x0E,0x02,0x0D,0x03,
         0x0C,0x04,0x0B,0x05,
         0x0A,0x06,0x08,0x08,
        },
        //linear Gain2
        {0x10,0x00,0x0F,0x01,
         0x0E,0x02,0x0D,0x03,
         0x0C,0x04,0x0B,0x05,
         0x0A,0x06,0x08,0x08,
         0x10,0x00,0x0F,0x01,
         0x0E,0x02,0x0D,0x03,
         0x0C,0x04,0x0B,0x05,
         0x0A,0x06,0x08,0x08,
        },
        // 4-tap default coef
        {0x78, 0x04, 0x6c, 0xf6,
         0xfb, 0x23, 0x4a, 0xf6
        },
        // 4-tap 100 coef
        {0x80, 0x00, 0x6E, 0xF9,
         0xFF, 0x1A, 0x45, 0xFB
        },
        // 4-tap 105 coef gain 22
        {0x8C, 0xFA, 0x9B, 0xDD,
         0xF9, 0x0F, 0x57, 0xE9
        },
        // 4-tap 105 coef gain 31
        {0x94, 0xF6, 0xD4, 0xBC,
         0xF2, 0x1FE, 0x6B, 0xD5
        },
        // 4-tap 105 coef gain 16
        {0x88, 0xFC, 0x75, 0xF2,
         0xFE, 0x1B, 0x49, 0xF7
        },
        // 4-tap 105 coef gain 17
        {0x88, 0xFC, 0x7A, 0xEF,
         0xFE, 0x19, 0x4B, 0xF5
        }
};

MS_U16 _GopHStretchTable[MAX_GOP_HSTRETCH_MODE_SUPPORT][GOP_STRETCH_TABLE_NUM] =
{

    //6-tap Default
    { 0x03,0x01,0x16,0x01,0x03,
      0x00,0x03,0x02,0x16,0x04,
      0x04,0x01,0x02,0x03,0x13,
      0x07,0x04,0x01,0x02,0x04,
      0x10,0x0b,0x04,0x01,0x03,
      0x01,0x16,0x01,0x03,0x00,
      0x03,0x02,0x16,0x04,0x04,
      0x01,0x02,0x03,0x13,0x07,
      0x04,0x01,0x02,0x04,0x10,
      0x0b,0x04,0x01,0x00,0x01
    },

    //Duplicate ->Set as default
    { 0x03,0x01,0x16,0x01,0x03,
      0x00,0x03,0x02,0x16,0x04,
      0x04,0x01,0x02,0x03,0x13,
      0x07,0x04,0x01,0x02,0x04,
      0x10,0x0b,0x04,0x01,0x03,
      0x01,0x16,0x01,0x03,0x00,
      0x03,0x02,0x16,0x04,0x04,
      0x01,0x02,0x03,0x13,0x07,
      0x04,0x01,0x02,0x04,0x10,
      0x0b,0x04,0x01,0x00,0x01
    },

    //6-tap Linear
    { 0x00,0x00,0x10,0x00,0x00,
      0x00,0x00,0x00,0x0E,0x02,
      0x00,0x00,0x00,0x00,0x0C,
      0x04,0x00,0x00,0x00,0x00,
      0x09,0x07,0x00,0x00,0x00,
      0x00,0x10,0x00,0x00,0x00,
      0x00,0x00,0x0e,0x02,0x00,
      0x00,0x00,0x00,0x0c,0x04,
      0x00,0x00,0x00,0x00,0x09,
      0x07,0x00,0x00,0x00,0x00
    },

    //6-tap Nearest
    { 0x00,0x00,0x10,0x00,0x00,
      0x00,0x00,0x00,0x10,0x00,
      0x00,0x00,0x00,0x00,0x10,
      0x00,0x00,0x00,0x00,0x00,
      0x10,0x00,0x00,0x00,0x00,
      0x00,0x10,0x00,0x00,0x00,
      0x00,0x00,0x10,0x00,0x00,
      0x00,0x00,0x00,0x10,0x00,
      0x00,0x00,0x00,0x00,0x10,
      0x00,0x00,0x00,0x00,0x00
    },

    //6-tap Gain0
    { 0x00,0x01,0x10,0x01,0x00,
      0x00,0x00,0x02,0x0f,0x04,
      0x01,0x00,0x01,0x02,0x0d,
      0x05,0x01,0x00,0x00,0x02,
      0x0c,0x08,0x02,0x00,0x00,
      0x01,0x10,0x01,0x00,0x00,
      0x00,0x02,0x0f,0x04,0x01,
      0x00,0x01,0x02,0x0d,0x05,
      0x01,0x00,0x00,0x02,0x0c,
      0x08,0x02,0x00,0x00,0x00
    },

    //6-tap Gain1
    { 0x02,0x01,0x14,0x01,0x02,
      0x00,0x02,0x02,0x12,0x05,
      0x03,0x00,0x00,0x03,0x10,
      0x06,0x02,0x01,0x01,0x03,
      0x0e,0x0a,0x03,0x01,0x02,
      0x01,0x14,0x01,0x02,0x00,
      0x02,0x02,0x12,0x05,0x03,
      0x00,0x00,0x03,0x10,0x06,
      0x02,0x01,0x01,0x03,0x0e,
      0x0a,0x03,0x01,0x00,0x00
    },

    //6-tap Gain2
    { 0x03,0x01,0x16,0x01,0x03,
      0x00,0x03,0x02,0x16,0x04,
      0x04,0x01,0x02,0x03,0x13,
      0x07,0x04,0x01,0x02,0x04,
      0x10,0x0b,0x04,0x01,0x03,
      0x01,0x16,0x01,0x03,0x00,
      0x03,0x02,0x16,0x04,0x04,
      0x01,0x02,0x03,0x13,0x07,
      0x04,0x01,0x02,0x04,0x10,
      0x0b,0x04,0x01,0x00,0x00
    },

    //6-tap Gain3
    { 0x05,0x01,0x19,0x02,0x05,
      0x00,0x04,0x03,0x18,0x06,
      0x06,0x01,0x03,0x04,0x15,
      0x08,0x04,0x02,0x04,0x04,
      0x13,0x0d,0x06,0x02,0x05,
      0x01,0x19,0x02,0x05,0x00,
      0x04,0x03,0x18,0x06,0x06,
      0x01,0x03,0x04,0x15,0x08,
      0x04,0x02,0x04,0x04,0x13,
      0x0d,0x06,0x02,0x00,0x00
    },

    //6-tap Gain4
    { 0x00,0x01,0x11,0x00,0x00,
      0x00,0x00,0x02,0x0f,0x04,
      0x01,0x00,0x00,0x01,0x0d,
      0x05,0x01,0x00,0x00,0x02,
      0x0c,0x08,0x02,0x00,0x03,
      0x01,0x16,0x01,0x03,0x00,
      0x03,0x02,0x16,0x04,0x04,
      0x01,0x02,0x03,0x13,0x07,
      0x04,0x01,0x02,0x04,0x10,
      0x0b,0x04,0x01,0x00,0x00
    },

    //6-tap Gain5
    { 0x00,0x00,0x10,0x00,0x00,
      0x00,0x00,0x00,0x0E,0x02,
      0x00,0x00,0x00,0x00,0x0C,
      0x04,0x00,0x00,0x00,0x00,
      0x08,0x08,0x00,0x00,0x00,
      0x00,0x10,0x00,0x00,0x00,
      0x00,0x00,0x0E,0x02,0x00,
      0x00,0x00,0x00,0x0C,0x04,
      0x00,0x00,0x00,0x00,0x08,
      0x08,0x00,0x00,0x00,0x00
    },
    //4-tap ->Set as default
    { 0x03,0x01,0x16,0x01,0x03,
      0x00,0x03,0x02,0x16,0x04,
      0x04,0x01,0x02,0x03,0x13,
      0x07,0x04,0x01,0x02,0x04,
      0x10,0x0b,0x04,0x01,0x03,
      0x01,0x16,0x01,0x03,0x00,
      0x03,0x02,0x16,0x04,0x04,
      0x01,0x02,0x03,0x13,0x07,
      0x04,0x01,0x02,0x04,0x10,
      0x0b,0x04,0x01,0x00,0x01
    },
    //2-tap
    { 0x00,0x00,0x10,0x00,0x00,
      0x00,0x00,0x00,0x00,0x00,
      0x00,0x00,0x00,0x00,0x00,
      0x00,0x00,0x00,0x00,0x00,
      0x08,0x08,0x00,0x00,0x00,
      0x00,0x10,0x00,0x00,0x00,
      0x00,0x00,0x00,0x00,0x00,
      0x00,0x00,0x00,0x00,0x00,
      0x00,0x00,0x00,0x00,0x08,
      0x08,0x00,0x00,0x00,0x00
    },
    //4-tap coef of 45
    { 0x44,0x1E,0x39,0x16,
      0x07,0x2A,0x33,0x0d
    },
    //4-tap coef of 50
    { 0x44,0x1E,0x3E,0x13,
      0x04,0x2B,0x36,0x0A
    },
    //4-tap coef of 55
    { H4TAP55_C0,H4TAP55_C1,H4TAP55_C2,H4TAP55_C3,
      H4TAP55_C4,H4TAP55_C5,H4TAP55_C6,H4TAP55_C7
    },
    //4-tap coef of 65
    { H4TAP65_C0,H4TAP65_C1,H4TAP65_C2,H4TAP65_C3,
      H4TAP65_C4,H4TAP65_C5,H4TAP65_C6,H4TAP65_C7
    },
    //4-tap coef of 75
    { H4TAP75_C0,H4TAP75_C1,H4TAP75_C2,H4TAP75_C3,
      H4TAP75_C4,H4TAP75_C5,H4TAP75_C6,H4TAP75_C7
    },
    //4-tap coef of 85
    { H4TAP85_C0,H4TAP85_C1,H4TAP85_C2,H4TAP85_C3,
      H4TAP85_C4,H4TAP85_C5,H4TAP85_C6,H4TAP85_C7
    },
    //4-tap coef of 95
    { H4TAP95_C0,H4TAP95_C1,H4TAP95_C2,H4TAP95_C3,
      H4TAP95_C4,H4TAP95_C5,H4TAP95_C6,H4TAP95_C7
    },
    //4-tap 105 coef
    { H4TAP105_C0,H4TAP105_C1,H4TAP105_C2,H4TAP105_C3,
      H4TAP105_C4,H4TAP105_C5,H4TAP105_C6,H4TAP105_C7
    },
    //tap 100 coef
    {  0x80, 0x00, 0x6E, 0xF9,
       0xFF, 0x1A, 0x45, 0xFB
    },
    //tap 105 coef gain 22
    {  0x8C, 0xFA, 0x9B, 0xDD,
       0xF9, 0x0F, 0x57, 0xE9
    },
    //tap 105 gain 31 coef
    {  0x94, 0xF6, 0xD4, 0xBC,
       0xF2, 0x1FE, 0x6B, 0xD5
    },
    //tap 105 coef gain 16
    {  0x88, 0xFC, 0x75, 0xF2,
       0xFE, 0x1B, 0x49, 0xF7
    },
    //tap 105 coef gain 17
    {  0x88, 0xFC, 0x7A, 0xEF,
       0xFE, 0x19, 0x4B, 0xF5
    }
};
MS_U16 u16MIUSelect[SHARED_GOP_MAX_COUNT]={0};
MS_BOOL bMIUSelect[SHARED_GOP_MAX_COUNT]={0};

MS_U16 u16AFBCMIUSelect[SHARED_GOP_MAX_COUNT]={0};
MS_BOOL bAFBCMIUSelect[SHARED_GOP_MAX_COUNT]={0};

MS_BOOL bCSCSelect[SHARED_GOP_MAX_COUNT]={FALSE};
MS_BOOL bAFBCTrigger=FALSE;

//-------------------------------------------------------------------------------------------------
//  Debug Functions
//-------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//  Local Functions
//------------------------------------------------------------------------------
#if GOP_XC_SEMPHORE_PROTECT
MS_U32 _GOP_Get_XCSemaphore(void)
{
    MS_U32 u32Return = UTOPIA_STATUS_FAIL;

    _XC_Inited = (pInstantGOP_XC != NULL);
    if (!_XC_Inited)
    {
        GOP_D_ERR("[%s,%5d]No instance existed, please get an instance by calling MApi_XC_Init() first\n",__FUNCTION__,__LINE__);
        return u32Return;
    }

    #if 0 // for Debug. It is important to check the tid
    pid_t tid;
    tid = syscall(SYS_gettid);
    printf("tid = (%d)\n",tid);
    #endif

    void* pModule = NULL;
    UtopiaInstanceGetModule(&pInstantGOP_XC, &pModule);
    if(pModule==NULL)
        return UTOPIA_STATUS_ERR_RESOURCE;
    if(UtopiaResourceObtain(pModule, E_XC_POOL_ID_INTERNAL_REGISTER, &g_pXCResource[E_XC_POOL_ID_INTERNAL_REGISTER]) != UTOPIA_STATUS_SUCCESS)
    {
        GOP_D_ERR("UtopiaResourceObtain fail\n");
        return UTOPIA_STATUS_ERR_RESOURCE;
    }
    u32Return = UTOPIA_STATUS_SUCCESS;
    return u32Return;
}

MS_U32 _GOP_Release_XCSemaphore(void)
{
    MS_U32 u32Return;
    if (!_XC_Inited)
    {
        GOP_D_ERR("[%s,%5d]No instance existed, please get an instance by calling MApi_XC_Init() first\n",__FUNCTION__,__LINE__);
        return UTOPIA_STATUS_SUCCESS;
    }
    #if 0 // for Debug. It is important to check the tid
    pid_t tid;
    tid = syscall(SYS_gettid);
    printf("tid = (%d)\n",tid);
    #endif

    u32Return = UtopiaResourceRelease(g_pXCResource[E_XC_POOL_ID_INTERNAL_REGISTER]);
    return u32Return;
}
#endif

static MS_BOOL _IsGwinIdValid(MS_GOP_CTX_LOCAL*pDrvCtx, MS_U8 u8GwinID)
{
    if (u8GwinID>=pDrvCtx->pGopChipProperty->TotalGwinNum)
        return FALSE;
    else
        return TRUE;
}

static MS_BOOL _IsGopNumVaild(MS_GOP_CTX_LOCAL*pDrvCtx, MS_U8 u8GopNum)
{
    if (u8GopNum>=MDrv_GOP_GetMaxGOPNum(pDrvCtx))
        return FALSE;
    else
        return TRUE;
}

static MS_BOOL _IsMuxSelVaild(MS_GOP_CTX_LOCAL*pDrvCtx, MS_U8 u8GopNum)
{
    if (u8GopNum>=MDrv_GOP_GetMaxMuxSel(pDrvCtx))
        return FALSE;
    else
        return TRUE;
}

static MS_U16 GOP_SetFmt(DRV_GOPColorType colorfmt)
{
    MS_U16 ret = GOP_FMT_GENERIC;

    switch(colorfmt)
    {
        case E_DRV_GOP_COLOR_RGB555_BLINK :
            ret = GOP_FMT_RGB555_BLINK;
            break;
        case E_DRV_GOP_COLOR_RGB565 :
            ret = GOP_FMT_RGB565;
            break;
        case E_DRV_GOP_COLOR_ARGB4444 :
            ret = GOP_FMT_ARGB4444;
            break;
        case E_DRV_GOP_COLOR_2266:
            ret = GOP_FMT_FaBaFgBg2266;
            break;
        case E_DRV_GOP_COLOR_I8 :
            ret = GOP_FMT_I8;
            break;
        case E_DRV_GOP_COLOR_ARGB8888 :
            ret = GOP_FMT_ARGB8888;
            break;
        case E_DRV_GOP_COLOR_ARGB1555:
            ret = GOP_FMT_ARGB1555;
            break;
        case E_DRV_GOP_COLOR_ABGR8888 :
            ret = GOP_FMT_ABGR8888;
            break;
        case E_DRV_GOP_COLOR_RGB555YUV422:/*Nos Chakra2*/
            ret = GOP_FMT_ARGB1555;
            break;
        case E_DRV_GOP_COLOR_YUV422:
            ret = GOP_FMT_YUV422;
            break;
        case E_DRV_GOP_COLOR_RGBA5551:
            ret = GOP_FMT_RGBA5551;
            break;
        case E_DRV_GOP_COLOR_RGBA4444 :
            ret = GOP_FMT_RGBA4444;
            break;
        case E_DRV_GOP_COLOR_RGBA8888 :
            ret = GOP_FMT_RGBA8888;
            break;
        case E_DRV_GOP_COLOR_BGR565 :
            ret = GOP_FMT_BGR565;
            break;
        case E_DRV_GOP_COLOR_ABGR4444 :
            ret = GOP_FMT_ABGR4444;
            break;
        case E_DRV_GOP_COLOR_AYUV8888 :
            ret = GOP_FMT_ARGB8888;
            break;
        case E_DRV_GOP_COLOR_ABGR1555 :
            ret = GOP_FMT_ABGR1555;
            break;
        case E_DRV_GOP_COLOR_BGRA5551 :
            ret = GOP_FMT_BGRA5551;
            break;
        case E_DRV_GOP_COLOR_BGRA4444 :
            ret = GOP_FMT_BGRA4444;
            break;
        case E_DRV_GOP_COLOR_BGRA8888 :
            ret = GOP_FMT_BGRA8888;
            break;
        default :
            GOP_ASSERT(0);
            ret = GOP_FMT_GENERIC;
            break;
    }
    return ret;
}

static DRV_GOPColorType GOP_GetFmt(MS_U16 colorfmt)
{
    DRV_GOPColorType ret = E_DRV_GOP_COLOR_INVALID;

    switch(colorfmt)
    {
        case GOP_FMT_RGB555_BLINK :
            ret = E_DRV_GOP_COLOR_RGB555_BLINK;
            break;
        case GOP_FMT_RGB565 :
            ret = E_DRV_GOP_COLOR_RGB565;
            break;
        case GOP_FMT_ARGB4444:
            ret = E_DRV_GOP_COLOR_ARGB4444;
            break;
        case GOP_FMT_FaBaFgBg2266:
            ret = E_DRV_GOP_COLOR_2266;
            break;
        case GOP_FMT_I8:
            ret = E_DRV_GOP_COLOR_I8;
            break;
        case GOP_FMT_ARGB8888:
            ret = E_DRV_GOP_COLOR_ARGB8888;
            break;
        case GOP_FMT_ARGB1555:
            ret = E_DRV_GOP_COLOR_ARGB1555;
            break;
        case GOP_FMT_ABGR8888:
            ret = E_DRV_GOP_COLOR_ABGR8888;
            break;
        case GOP_FMT_ARGB1555_DST:
            ret = E_DRV_GOP_COLOR_RGB555YUV422;
            break;
        case GOP_FMT_YUV422:
            ret = E_DRV_GOP_COLOR_YUV422;
            break;
        case GOP_FMT_RGBA5551:
            ret = E_DRV_GOP_COLOR_RGBA5551;
            break;
        case GOP_FMT_RGBA4444:
            ret = E_DRV_GOP_COLOR_RGBA4444;
            break;
        case GOP_FMT_RGBA8888 :
            ret = E_DRV_GOP_COLOR_RGBA8888;
            break;
        case GOP_FMT_BGR565:
            ret = E_DRV_GOP_COLOR_BGR565;
            break;
        case GOP_FMT_ABGR4444:
            ret = E_DRV_GOP_COLOR_ABGR4444;
            break;
        case GOP_FMT_ABGR1555 :
            ret = E_DRV_GOP_COLOR_ABGR1555;
            break;
        case GOP_FMT_BGRA5551 :
            ret = E_DRV_GOP_COLOR_BGRA5551;
            break;
        case GOP_FMT_BGRA4444 :
            ret = E_DRV_GOP_COLOR_BGRA4444;
            break;
        case GOP_FMT_BGRA8888 :
            ret = E_DRV_GOP_COLOR_BGRA8888;
            break;
        default :
            GOP_ASSERT(0);
            ret = E_DRV_GOP_COLOR_INVALID;
            break;
    }
    return ret;
}
static MS_BOOL _GetGOPAckDelayTimeAndCnt(MS_U32* pu32DelayTimems, MS_U32* pu32TimeoutCnt)
{
    if ((NULL == pu32DelayTimems) || (NULL == pu32TimeoutCnt))
    {
        GOP_ASSERT(FALSE);
        return FALSE;
    }
#if defined(MSOS_TYPE_LINUX) || defined(MSOS_TYPE_LINUX_KERNEL)
	#if GOP_VSYNC_WAIT_BYSLEEP
    *pu32DelayTimems = 1;
    *pu32TimeoutCnt = GOP_TIMEOUT_CNT_OS;
	#else
	*pu32DelayTimems = 1;
    *pu32TimeoutCnt = GOP_TIMEOUT_CNT_OS_YIELD;
    #endif
#else
    *pu32DelayTimems = 0;
    *pu32TimeoutCnt = GOP_TIMEOUT_CNT_NOS;
#endif
    return TRUE;
}

/*
//Return GWIN index in GOP's all GWin by normal gwin id
static MS_U8 _MapGWinIDToGWinIndexinGOP(MS_U8 u8GwinID)
{
    if(u8GwinID < GOP1_GwinIdBase)
    {
        return u8GwinID;//GOP 0
    }
    else if(u8GwinID >= GOP1_GwinIdBase && u8GwinID < GOP2_GwinIdBase)
    {
        return u8GwinID-GOP1_GwinIdBase; //GOP1
    }
    else
    {
        return 0; //GOP2 and gop3 only have 1 gwin
    }
}
*/

#if GOP_LOCK_SUPPORT
static void GOP_GOP_Restore_Ctx(MS_GOP_CTX_LOCAL*pGOPCtx)
{
     MDrv_GOP_Restore_Ctx(pGOPCtx);
}
#endif

static MS_BOOL _MDrv_GOP_CSC_ParamInit(ST_GOP_CSC_PARAM *pstCSCParam)
{
    if(pstCSCParam == NULL)
    {
        return FALSE;
    }
    pstCSCParam->u32Version = ST_GOP_CSC_PARAM_VERSION;
    pstCSCParam->u32Length = sizeof(ST_GOP_CSC_PARAM);
    pstCSCParam->bCscEnable = FALSE;
    pstCSCParam->bUpdateWithVsync = FALSE;
    pstCSCParam->u16Hue = 50;
    pstCSCParam->u16Saturation = 128;
    pstCSCParam->u16Contrast = 1024;
    pstCSCParam->u16Brightness = 1024;
    pstCSCParam->u16RGBGGain[0] = 1024;
    pstCSCParam->u16RGBGGain[1] = 1024;
    pstCSCParam->u16RGBGGain[2] = 1024;
    pstCSCParam->enInputFormat = E_GOP_CFD_CFIO_MAX;
    pstCSCParam->enInputDataFormat = E_GOP_CFD_MC_FORMAT_MAX;
    pstCSCParam->enInputRange = E_GOP_CFD_CFIO_RANGE_FULL;
    pstCSCParam->enOutputFormat = E_GOP_CFD_CFIO_MAX;
    pstCSCParam->enOutputDataFormat = E_GOP_CFD_MC_FORMAT_MAX;
    pstCSCParam->enOutputRange = E_GOP_CFD_CFIO_RANGE_FULL;
    return TRUE;
}

static MS_BOOL _MDrv_GOP_CSC_TableInit(ST_GOP_CSC_TABLE *pstCSCTable)
{
    if(pstCSCTable == NULL)
    {
        return FALSE;
    }
    memset(pstCSCTable, 0, sizeof(ST_GOP_CSC_TABLE));

    return TRUE;
}

void GOP_GWIN_TriggerRegWriteIn(GOP_CTX_DRV_LOCAL*pGOPDrvLocalCtx, E_GOP_TYPE u8GopType, MS_BOOL bForceWriteIn, MS_BOOL bSync)
{
	MS_U8 u8GOPIdx = (MS_U8)u8GopType;
	MS_U16 u16GopAck=0;
	MS_U32 goptimeout = 0;

	if (u8GOPIdx > E_GOP5) {
		GOP_D_ERR("[%s] invalid GOPIdx\n", __func__);
		return;
	}

	if (pGOPDrvLocalCtx->bGOPBankFwr[u8GOPIdx]) {
	/*Defination use for warning user the bnkForceWrite function support or not.*/
		if(pGOPDrvLocalCtx->apiCtxLocal.pGopChipProperty->bBnkForceWrite) {
			HAL_GOP_GWIN_TriggerRegWriteIn(&pGOPDrvLocalCtx->halCtxLocal, u8GopType, TRUE, E_DRV_GOP_BNKFORCEWRITE);
			HAL_GOP_GWIN_TriggerRegWriteIn(&pGOPDrvLocalCtx->halCtxLocal, u8GopType, FALSE, E_DRV_GOP_BNKFORCEWRITE);
		} else {
			GOP_D_DBUG("[%s][%d]  GOP%d not support BankForceWrite\n",__FUNCTION__,__LINE__,u8GopType);
		}
	} else if (bForceWriteIn) {
		HAL_GOP_GWIN_TriggerRegWriteIn(&pGOPDrvLocalCtx->halCtxLocal, u8GopType, TRUE, E_DRV_GOP_FORCEWRITE);
		HAL_GOP_GWIN_TriggerRegWriteIn(&pGOPDrvLocalCtx->halCtxLocal, u8GopType, FALSE, E_DRV_GOP_FORCEWRITE);
	} else {
		HAL_GOP_GWIN_TriggerRegWriteIn(&pGOPDrvLocalCtx->halCtxLocal, u8GopType, TRUE, E_DRV_GOP_WITHVSYNC);

		if (bSync == TRUE) {
			MS_U32 u32DelayTimems = 0;
			MS_U32 u32TimeoutCnt = 0;
			_GetGOPAckDelayTimeAndCnt(&u32DelayTimems, &u32TimeoutCnt);
			do {
				goptimeout++;
				u16GopAck = HAL_GOP_GetGOPACK(&pGOPDrvLocalCtx->halCtxLocal,u8GopType);
				if (0 != u32DelayTimems) {
					#if GOP_VSYNC_WAIT_BYSLEEP
					MsOS_DelayTask(u32DelayTimems); //delay 1 ms
					#else
					MsOS_YieldTask();
					#endif
				}
			} while((!u16GopAck) &&(goptimeout <= u32TimeoutCnt) );

			// Perform force write if wr timeout.
			if (goptimeout > u32TimeoutCnt) {
				GOP_D_INFO("[%s][%d]Perform fwr if wr timeout!!\n",__FUNCTION__,__LINE__);
				if (pGOPDrvLocalCtx->apiCtxLocal.pGopChipProperty->bBnkForceWrite) {
					HAL_GOP_GWIN_TriggerRegWriteIn(&pGOPDrvLocalCtx->halCtxLocal, u8GopType, TRUE, E_DRV_GOP_BNKFORCEWRITE);
					HAL_GOP_GWIN_TriggerRegWriteIn(&pGOPDrvLocalCtx->halCtxLocal, u8GopType, FALSE, E_DRV_GOP_BNKFORCEWRITE);
				} else {
					HAL_GOP_GWIN_TriggerRegWriteIn(&pGOPDrvLocalCtx->halCtxLocal, u8GopType, TRUE, E_DRV_GOP_FORCEWRITE);
					HAL_GOP_GWIN_TriggerRegWriteIn(&pGOPDrvLocalCtx->halCtxLocal, u8GopType, FALSE, E_DRV_GOP_FORCEWRITE);
				}
			}
		}
	}
}

void GOP_GWIN_UpdateReg(GOP_CTX_DRV_LOCAL*pGOPDrvLocalCtx, E_GOP_TYPE u8GopType)
{
	MS_U8 u8GOPIdx = (MS_U8)u8GopType;

	if (u8GOPIdx > E_GOP5) {
		GOP_D_ERR("[%s] invalid u8GopType\n", __func__);
		return;
	}

    if((FALSE == pGOPDrvLocalCtx->apiCtxLocal.bUpdateRegOnce[GOP_PUBLIC_UPDATE]) && (FALSE == pGOPDrvLocalCtx->apiCtxLocal.bUpdateRegOnce[u8GOPIdx]))
    {
        if(MDrv_GOP_GWIN_IsForceWrite((MS_GOP_CTX_LOCAL*)pGOPDrvLocalCtx, u8GopType) == TRUE)
        {
            GOP_GWIN_TriggerRegWriteIn(pGOPDrvLocalCtx, u8GopType, TRUE, TRUE);
        }
        else
        {
            GOP_GWIN_TriggerRegWriteIn(pGOPDrvLocalCtx, u8GopType, FALSE, TRUE);
        }
    }
}

static MS_U32 _GOP_GWIN_AlignChecker(MS_GOP_CTX_LOCAL*pGOPCtx, MS_U8 u8GOPNum, MS_U16 *width, MS_U16 fmtbpp)
{
    MS_U16 alignfactor =0;

    if(pGOPCtx->pGOPCtxShared->bPixelMode[u8GOPNum] == TRUE)
    {
        alignfactor = 1;
    }
    else
    {
        alignfactor = (MS_U16)(((MS_U16)MDrv_GOP_GetWordUnit(pGOPCtx, u8GOPNum))/(fmtbpp>>3) );
    }

    if ((alignfactor != 0) && (*width % alignfactor !=0)) // Not Aligned
    {
        GOP_D_ERR("\n\n%s, This FB format needs to %d-pixels alignment !!!\n\n",__FUNCTION__, alignfactor);
        //*width = (*width + alignfactor - 1) & (~(alignfactor - 1));
        return CHECKALIGN_PARA_FAIL;
    }
    else
    {
        return CHECKALIGN_SUCCESS;
    }
}

static MS_U32 _GOP_GWIN_2PEngineAlignChecker(MS_GOP_CTX_LOCAL*pGOPCtx, MS_U8 u8GOPNum, MS_PHY *width)
{
    MS_U16 alignfactor =0;

    if(pGOPCtx->pGopChipProperty->b2Pto1PSupport==TRUE)
    {
        alignfactor=2;

        if ((alignfactor != 0) && (*width % alignfactor !=0)) // Not Aligned
        {
            GOP_D_ERR("\n\n%s, Not mach to %d-pixels alignment !!!\n\n",__FUNCTION__, alignfactor);
             return CHECKALIGN_PARA_FAIL;
         }
         else
         {
             return CHECKALIGN_SUCCESS;
         }
     }
    return CHECKALIGN_SUCCESS;
}

static MS_U32 _GOP_GWIN_FB_AddrAlignChecker(MS_GOP_CTX_LOCAL*pGOPCtx, MS_U8 u8GOPNum, MS_PHY Address, MS_U16 fmtbpp)
{
    MS_U32 alignfactor=0;

    if(pGOPCtx->pGOPCtxShared->bPixelMode[u8GOPNum] == TRUE)
    {
        alignfactor = (MS_U32)((MDrv_GOP_GetWordUnit(pGOPCtx,u8GOPNum)*(fmtbpp>>3))&0xFF);
    }
    else
    {
        alignfactor = (MS_U32)((MDrv_GOP_GetWordUnit(pGOPCtx,u8GOPNum))&0xFF);
    }

    if ((Address&(alignfactor-1))!=0)
    {
        GOP_D_ERR("%s,%d FB=0x%tx address need %td-bytes aligned!\n",__FUNCTION__,__LINE__, (ptrdiff_t)Address, (ptrdiff_t)alignfactor);
        return CHECKALIGN_PARA_FAIL;
    }
    else
    {
        return CHECKALIGN_SUCCESS;
    }
}

void GOP_GetGopGwinHVPixel(GOP_CTX_DRV_LOCAL*pGOPDrvLocalCtx, MS_U8 u8win, MS_U8 u8GOP, DRV_GOP_GWIN_INFO* pinfo)
{
	HAL_GOP_GetGopGwinHVPixel(&pGOPDrvLocalCtx->halCtxLocal, u8GOP, u8win, &(pinfo->u16DispHPixelStart), &(pinfo->u16DispHPixelEnd), &(pinfo->u16DispVPixelStart), &(pinfo->u16DispVPixelEnd));
}

void GOP_SetGopGwinHVPixel(GOP_CTX_DRV_LOCAL*pGOPDrvLocalCtx, MS_U8 u8win, MS_U8 u8GOP, DRV_GOP_GWIN_INFO* pinfo)
{
    MS_U16 u8Bpp;
    MS_U16 align_start,align_end, u16VEnd;
    MS_U16 u16StretchWidth=0, u16StretchHeight = 0, u16x = 0, u16y = 0;

    u8Bpp = HAL_GOP_GetBPP(&pGOPDrvLocalCtx->halCtxLocal, pinfo->clrType);
    if(u8Bpp == FB_FMT_AS_DEFAULT)
    {
        GOP_D_DBUG("[%s] [%d]  GOP not support the color format = %d    \n",__FUNCTION__,__LINE__,pinfo->clrType);
    }
    u16VEnd = pinfo->u16DispVPixelEnd;

    if(pGOPDrvLocalCtx->apiCtxLocal.pGOPCtxShared->bPixelMode[u8GOP] == TRUE)
    {
        align_start = (pinfo->u16DispHPixelStart);
        align_end   = (pinfo->u16DispHPixelEnd);
    }
    else
    {
        align_start = (pinfo->u16DispHPixelStart)*(u8Bpp>>3) /(GOP_WordUnit);
        align_end   = (pinfo->u16DispHPixelEnd)*(u8Bpp>>3) /(GOP_WordUnit);
    }

	HAL_GOP_GetGopStretchWin(&pGOPDrvLocalCtx->halCtxLocal, u8GOP, &u16x, &u16y, &u16StretchWidth, &u16StretchHeight);
    if(align_end > u16StretchWidth)
    {
        GOP_D_DBUG("[GOP]%s,%s,%d GWIN Hend > StretchWidth!!!align_end=%d,u16StretchWidth=%d\n",__FILE__,__FUNCTION__,__LINE__,(int)align_end,(int)u16StretchWidth);
        align_end = u16StretchWidth;
     }

    HAL_GOP_SetGopGwinHVPixel(&pGOPDrvLocalCtx->halCtxLocal, u8GOP, u8win, align_start, align_end, pinfo->u16DispVPixelStart, u16VEnd);
}


static void GOP_SetGopExtendGwinInfo(GOP_CTX_DRV_LOCAL*pGOPDrvLocalCtx, MS_U8 u8win, DRV_GOP_GWIN_INFO* pinfo)
{
    MS_PHY phyTmp=0;
    MS_U16 u16Bpp=0;
    MS_U16 u16GOP_Unit=0;
    MS_U8 u8GOP=0;
    MS_U16 u16Pitch=0;
    MS_U8 u8MiuSel=0;
    MS_PHY Width=0;
    MS_U8 u8win_temp = 0;

    if (FALSE== _IsGwinIdValid((MS_GOP_CTX_LOCAL*)pGOPDrvLocalCtx, u8win))
    {
        GOP_D_ERR("\n[%s] not support gwin id:%d in this chip version",__FUNCTION__, u8win);
        return;
    }

    switch(u8win)
    {
        case GOP2_Gwin0Id:
            u8GOP = 2;
            u8win_temp = (u8win - MAX_GOP0_GWIN - MAX_GOP1_GWIN);
            break;
        case GOP3_Gwin0Id:
            u8GOP = 3;
            u8win_temp = (u8win - MAX_GOP0_GWIN - MAX_GOP1_GWIN - MAX_GOP2_GWIN);
            break;
        case GOP4_Gwin0Id:
            u8GOP = 4;
            u8win_temp = (u8win - MAX_GOP0_GWIN - MAX_GOP1_GWIN - MAX_GOP2_GWIN - MAX_GOP3_GWIN);
            break;
        case GOP5_Gwin0Id:
            u8GOP = 5;
            u8win_temp = (u8win - MAX_GOP0_GWIN - MAX_GOP1_GWIN - MAX_GOP2_GWIN - MAX_GOP3_GWIN - MAX_GOP4_GWIN);
            break;
        default:
            GOP_D_ERR("[%s] invalud GwinNum\n",__FUNCTION__);
            return;
    }

    if (FALSE== _IsGopNumVaild((MS_GOP_CTX_LOCAL*)pGOPDrvLocalCtx, u8GOP))
    {
        GOP_D_ERR("\n[%s] not support GOP id:%d in this chip version",__FUNCTION__, u8GOP);
        return;
    }

    u16GOP_Unit = MDrv_GOP_GetWordUnit((MS_GOP_CTX_LOCAL*)pGOPDrvLocalCtx, u8GOP);
    u16Bpp = HAL_GOP_GetBPP(&pGOPDrvLocalCtx->halCtxLocal, pinfo->clrType);
    if(u16Bpp == FB_FMT_AS_DEFAULT)
    {
        GOP_D_ERR("[%s] invalud color format\n",__FUNCTION__);
        return;
    }

	_GOP_GWIN_AlignChecker((MS_GOP_CTX_LOCAL*)pGOPDrvLocalCtx, u8GOP, &(pinfo->u16WinX), u16Bpp);
    _GOP_GWIN_AlignChecker((MS_GOP_CTX_LOCAL*)pGOPDrvLocalCtx, u8GOP, &(pinfo->u16DispHPixelStart), u16Bpp);
    _GOP_GWIN_AlignChecker((MS_GOP_CTX_LOCAL*)pGOPDrvLocalCtx, u8GOP, &(pinfo->u16DispHPixelEnd), u16Bpp);
    _GOP_GWIN_AlignChecker((MS_GOP_CTX_LOCAL*)pGOPDrvLocalCtx, u8GOP, &u16Pitch, u16Bpp);
    _GOP_GWIN_FB_AddrAlignChecker((MS_GOP_CTX_LOCAL*)pGOPDrvLocalCtx, u8GOP, pinfo->u64DRAMRBlkStart, u16Bpp);

    if(pGOPDrvLocalCtx->apiCtxLocal.pGOPCtxShared->bPixelMode[u8GOP] == TRUE)
    {
        /*Pixel Unit*/
        u16Pitch = pinfo->u16RBlkHRblkSize/u16GOP_Unit/(u16Bpp/8);
    }
    else
    {
        /*Word Unit*/
        u16Pitch = pinfo->u16RBlkHRblkSize/u16GOP_Unit;
    }

    if(pinfo->u16DispHPixelEnd > pinfo->u16DispHPixelStart)
    {
        Width = (MS_PHY)(pinfo->u16DispHPixelEnd - pinfo->u16DispHPixelStart);
    }
    else
    {
    	if(pGOPDrvLocalCtx->pDrvCtxShared->apiCtxShared.bGopHasInitialized[u8GOP] == TRUE)
    	{
        	GOP_D_DBUG("[%s]WidthEnd 0x%x >WidthStart 0x%x \n",__FUNCTION__, pinfo->u16DispHPixelEnd, pinfo->u16DispHPixelStart);
    	}
    }
    if (CHECKALIGN_PARA_FAIL ==_GOP_GWIN_2PEngineAlignChecker((MS_GOP_CTX_LOCAL*)pGOPDrvLocalCtx, u8GOP, &Width))
    {
        GOP_D_WARN("[%s][%d] Size not Align!!! Original u16DispHPixelStart =0x%x; u16DispHPixelEnd=0x%x  \n",__FUNCTION__,__LINE__,pinfo->u16DispHPixelStart, pinfo->u16DispHPixelEnd );
        pinfo->u16DispHPixelStart = ((pinfo->u16DispHPixelStart+1 )& ~(1));
        pinfo->u16DispHPixelEnd = ((pinfo->u16DispHPixelEnd+1 )& ~(1));
        GOP_D_WARN("[%s][%d] Size not Align!!! Align after u16DispHPixelStart =0x%x; u16DispHPixelEnd=0x%x  \n",__FUNCTION__,__LINE__,pinfo->u16DispHPixelStart, pinfo->u16DispHPixelEnd );
    }
    _GOP_GWIN_2PEngineAlignChecker((MS_GOP_CTX_LOCAL*)pGOPDrvLocalCtx, u8GOP, &(pinfo->u64DRAMRBlkStart));

    if(pGOPDrvLocalCtx->b32BPPTileMode[u8GOP] && pGOPDrvLocalCtx->pDrvCtxShared->apiCtxShared.bVMirror)
    {
        HAL_GOP_SetDram_Addr(&pGOPDrvLocalCtx->halCtxLocal, u8GOP, u8win_temp, (MS_PHY)(pinfo->u16DispVPixelEnd- pinfo->u16DispVPixelStart-1), E_DRV_GOP_DRAMVstr);
    }
    else
    {
        HAL_GOP_SetDram_Addr(&pGOPDrvLocalCtx->halCtxLocal, u8GOP, u8win_temp, 0, E_DRV_GOP_DRAMVstr);//clear the VSTR value
    }

    //GOP HW just read the relative offset of each MIU
    _phy_to_miu_offset(u8MiuSel, phyTmp, pinfo->u64DRAMRBlkStart);
    UNUSED(u8MiuSel);
    phyTmp /= u16GOP_Unit;

    HAL_GOP_SetDram_Addr(&pGOPDrvLocalCtx->halCtxLocal, u8GOP, u8win_temp, phyTmp, E_DRV_GOP_RBLKAddr);
	HAL_GOP_SetPitch(&pGOPDrvLocalCtx->halCtxLocal, u8GOP, u8win_temp, u16Pitch, pinfo->u16RBlkVPixSize);

    GOP_SetGopGwinHVPixel(pGOPDrvLocalCtx, u8win, u8GOP ,pinfo);

    GOP_D_INFO("GWIN_SetWin(%d): [adr, RBsz, offset](Unit:Byte) = [%td, %d, %d]\n", \
                u8win,\
                (ptrdiff_t)pinfo->u64DRAMRBlkStart,\
                pinfo->u16RBlkHPixSize * pinfo->u16RBlkVPixSize / (u16GOP_Unit/(u16Bpp>>3)),\
                (pinfo->u16WinY * pinfo->u16RBlkHPixSize + pinfo->u16WinX)/(u16GOP_Unit/(u16Bpp>>3))   );
    GOP_D_INFO("\t[Vst, Vend, Hst, Hend, GwinHsz](Unit:Pixel) = [%d, %d, %d, %d, %d]\n",\
                pinfo->u16DispVPixelStart, \
                pinfo->u16DispVPixelEnd, \
                pinfo->u16DispHPixelStart, \
                pinfo->u16DispHPixelEnd,\
                pinfo->u16RBlkHPixSize);

}

static void GOP_SetGop1GwinInfo(GOP_CTX_DRV_LOCAL*pGOPDrvLocalCtx, MS_U8 u8win, DRV_GOP_GWIN_INFO* pinfo)
{
    MS_PHY u64tmp;
    MS_U16 bpp;
    MS_U16 u16GOP_Unit=0;
    MS_U16 u16Pitch=0;
    MS_U8 u8MiuSel;
    MS_PHY Width=0;

    u16GOP_Unit = MDrv_GOP_GetWordUnit((MS_GOP_CTX_LOCAL*)pGOPDrvLocalCtx, E_GOP1);

    u8win -= MAX_GOP0_GWIN; //Reset win number to 0~3 in GOP1
    bpp = HAL_GOP_GetBPP(&pGOPDrvLocalCtx->halCtxLocal, pinfo->clrType);
    if(bpp == FB_FMT_AS_DEFAULT)
    {
        GOP_D_ERR("[%s] invalud color format\n",__FUNCTION__);
        return;
    }

	_GOP_GWIN_AlignChecker((MS_GOP_CTX_LOCAL*)pGOPDrvLocalCtx, E_GOP1, &(pinfo->u16WinX), bpp);
    _GOP_GWIN_AlignChecker((MS_GOP_CTX_LOCAL*)pGOPDrvLocalCtx, E_GOP1, &(pinfo->u16DispHPixelStart), bpp);
    _GOP_GWIN_AlignChecker((MS_GOP_CTX_LOCAL*)pGOPDrvLocalCtx, E_GOP1, &(pinfo->u16DispHPixelEnd), bpp);
    _GOP_GWIN_AlignChecker((MS_GOP_CTX_LOCAL*)pGOPDrvLocalCtx, E_GOP1, &u16Pitch, bpp);
    _GOP_GWIN_FB_AddrAlignChecker((MS_GOP_CTX_LOCAL*)pGOPDrvLocalCtx, E_GOP1, pinfo->u64DRAMRBlkStart, bpp);

    if(pGOPDrvLocalCtx->apiCtxLocal.pGOPCtxShared->bPixelMode[E_GOP1] == TRUE)
    {
        /*Pixel Unit*/
        u16Pitch = pinfo->u16RBlkHRblkSize/u16GOP_Unit/(bpp/8);
    }
    else
    {
        /*Word Unit*/
        u16Pitch = pinfo->u16RBlkHRblkSize/u16GOP_Unit;
    }

    if(pinfo->u16DispHPixelEnd > pinfo->u16DispHPixelStart)
    {
        Width = (MS_PHY)(pinfo->u16DispHPixelEnd - pinfo->u16DispHPixelStart);
    }
    else
    {
    	if(pGOPDrvLocalCtx->pDrvCtxShared->apiCtxShared.bGopHasInitialized[E_GOP1] == TRUE)
    	{
        	GOP_D_DBUG("[%s]WidthEnd 0x%x >WidthStart 0x%x \n",__FUNCTION__, pinfo->u16DispHPixelEnd, pinfo->u16DispHPixelStart);
    	}
    }
    if (CHECKALIGN_PARA_FAIL ==_GOP_GWIN_2PEngineAlignChecker((MS_GOP_CTX_LOCAL*)pGOPDrvLocalCtx, E_GOP1, &Width))
    {
        GOP_D_WARN("[%s][%d] Size not Align!!! Original u16DispHPixelStart =0x%x; u16DispHPixelEnd=0x%x  \n",__FUNCTION__,__LINE__,pinfo->u16DispHPixelStart, pinfo->u16DispHPixelEnd );
        pinfo->u16DispHPixelStart = ((pinfo->u16DispHPixelStart+1 )& ~(1));
        pinfo->u16DispHPixelEnd = ((pinfo->u16DispHPixelEnd+1 )& ~(1));
        GOP_D_WARN("[%s][%d] Size not Align!!! Align after u16DispHPixelStart =0x%x; u16DispHPixelEnd=0x%x  \n",__FUNCTION__,__LINE__,pinfo->u16DispHPixelStart, pinfo->u16DispHPixelEnd );
    }
    _GOP_GWIN_2PEngineAlignChecker((MS_GOP_CTX_LOCAL*)pGOPDrvLocalCtx, E_GOP1, &(pinfo->u64DRAMRBlkStart));

    if(pGOPDrvLocalCtx->b32BPPTileMode[1] && pGOPDrvLocalCtx->pDrvCtxShared->apiCtxShared.bVMirror)
    {
        HAL_GOP_SetDram_Addr(&pGOPDrvLocalCtx->halCtxLocal, E_GOP1, u8win, (MS_PHY)(pinfo->u16DispVPixelEnd- pinfo->u16DispVPixelStart-1), E_DRV_GOP_DRAMVstr);
    }
    else
    {
        HAL_GOP_SetDram_Addr(&pGOPDrvLocalCtx->halCtxLocal, E_GOP1, u8win, 0x0, E_DRV_GOP_DRAMVstr); //clear the VSTR value
    }
    //GOP HW just read the relative offset of each MIU
    _phy_to_miu_offset(u8MiuSel, u64tmp, pinfo->u64DRAMRBlkStart);
    u64tmp /= u16GOP_Unit;
    UNUSED(u8MiuSel);
    HAL_GOP_SetDram_Addr(&pGOPDrvLocalCtx->halCtxLocal, E_GOP1, u8win, u64tmp, E_DRV_GOP_RBLKAddr);
	HAL_GOP_SetPitch(&pGOPDrvLocalCtx->halCtxLocal, E_GOP1, u8win, u16Pitch, pinfo->u16RBlkVPixSize);

    GOP_SetGopGwinHVPixel(pGOPDrvLocalCtx, u8win, E_GOP1 ,pinfo);

    GOP_D_INFO("GWIN_SetWin(%d): [adr, RBsz, offset](Unit: Byte) = [%td, %d, %d]\n", \
                u8win,\
                (ptrdiff_t)pinfo->u64DRAMRBlkStart,\
                pinfo->u16RBlkHPixSize * pinfo->u16RBlkVPixSize / (u16GOP_Unit/(bpp>>3)),\
                (pinfo->u16WinY * pinfo->u16RBlkHPixSize + pinfo->u16WinX)/(u16GOP_Unit/(bpp>>3))  );
    GOP_D_INFO("\t[Vst, Vend, Hst, Hend, GwinHsz] = [%d, %d, %d, %d, %d](Unit:Pixel)\n",\
                pinfo->u16DispVPixelStart, \
                pinfo->u16DispVPixelEnd, \
                pinfo->u16DispHPixelStart, \
                pinfo->u16DispHPixelEnd,\
                pinfo->u16RBlkHPixSize);

}

static void GOP_SetGop0GwinInfo(GOP_CTX_DRV_LOCAL*pGOPDrvLocalCtx, MS_U8 u8win, DRV_GOP_GWIN_INFO* pinfo)
{
    MS_U64 u64tmp=0;
    MS_PHY u64tmp1 = 0;
    MS_U16 bpp;
    MS_U16 u16GOP_Unit=0;
    MS_U16 u16Pitch=0;
    MS_U8 u8MiuSel;
    MS_PHY Width=0;

    bpp = HAL_GOP_GetBPP(&pGOPDrvLocalCtx->halCtxLocal, pinfo->clrType);
    if(bpp == FB_FMT_AS_DEFAULT)
    {
        GOP_D_ERR("[%s] invalud color format\n",__FUNCTION__);
        return;
    }

    _GOP_GWIN_AlignChecker((MS_GOP_CTX_LOCAL*)pGOPDrvLocalCtx, E_GOP0, &(pinfo->u16WinX), bpp);
    _GOP_GWIN_AlignChecker((MS_GOP_CTX_LOCAL*)pGOPDrvLocalCtx, E_GOP0, &(pinfo->u16DispHPixelStart), bpp);
    _GOP_GWIN_AlignChecker((MS_GOP_CTX_LOCAL*)pGOPDrvLocalCtx, E_GOP0, &(pinfo->u16DispHPixelEnd), bpp);
	_GOP_GWIN_AlignChecker((MS_GOP_CTX_LOCAL*)pGOPDrvLocalCtx, E_GOP0, &(pinfo->u16RBlkHRblkSize), bpp);

    _GOP_GWIN_FB_AddrAlignChecker((MS_GOP_CTX_LOCAL*)pGOPDrvLocalCtx, E_GOP0, pinfo->u64DRAMRBlkStart, bpp);

    if(pinfo->u16DispHPixelEnd > pinfo->u16DispHPixelStart)
    {
        Width = (MS_PHY)(pinfo->u16DispHPixelEnd - pinfo->u16DispHPixelStart);
    }
    else
    {
    	if(pGOPDrvLocalCtx->pDrvCtxShared->apiCtxShared.bGopHasInitialized[E_GOP0] == TRUE)
    	{
        	GOP_D_DBUG("[%s]WidthEnd 0x%x >WidthStart 0x%x \n",__FUNCTION__, pinfo->u16DispHPixelEnd, pinfo->u16DispHPixelStart);
    	}
    }
    if (CHECKALIGN_PARA_FAIL ==_GOP_GWIN_2PEngineAlignChecker((MS_GOP_CTX_LOCAL*)pGOPDrvLocalCtx, E_GOP0, &Width))
    {
        GOP_D_WARN("[%s][%d] Size not Align!!! Original u16DispHPixelStart =0x%x; u16DispHPixelEnd=0x%x  \n",__FUNCTION__,__LINE__,pinfo->u16DispHPixelStart, pinfo->u16DispHPixelEnd );
        pinfo->u16DispHPixelStart = ((pinfo->u16DispHPixelStart+1 )& ~(1));
        pinfo->u16DispHPixelEnd = ((pinfo->u16DispHPixelEnd+1 )& ~(1));
        GOP_D_WARN("[%s][%d] Size not Align!!! Align after u16DispHPixelStart =0x%x; u16DispHPixelEnd=0x%x  \n",__FUNCTION__,__LINE__,pinfo->u16DispHPixelStart, pinfo->u16DispHPixelEnd );
    }
    _GOP_GWIN_2PEngineAlignChecker((MS_GOP_CTX_LOCAL*)pGOPDrvLocalCtx, E_GOP0, &(pinfo->u64DRAMRBlkStart));

    u16GOP_Unit = MDrv_GOP_GetWordUnit((MS_GOP_CTX_LOCAL*)pGOPDrvLocalCtx, E_GOP0);
    if(pGOPDrvLocalCtx->apiCtxLocal.pGOPCtxShared->bPixelMode[E_GOP0] == TRUE)
    {
        /*Pixel Unit*/
        u16Pitch = pinfo->u16RBlkHRblkSize/u16GOP_Unit/(bpp/8);
    }
    else
    {
        /*Word Unit*/
        u16Pitch = pinfo->u16RBlkHRblkSize/u16GOP_Unit;
    }

#if ENABLE_GOP0_RBLK_MIRROR
    u64tmp = (MS_U64)pinfo->u16WinY * (MS_U64)pinfo->u16RBlkHRblkSize;     // Convert pixel into bytes
    u64tmp  /= u16GOP_Unit;  // Convert to word.
#else
    u64tmp = ((MS_U64)pinfo->u16WinY * (MS_U64)pinfo->u16RBlkHRblkSize  + (MS_U64)pinfo->u16WinX*(bpp>>3));     // Convert pixel into bytes
    u64tmp  /= u16GOP_Unit;  // Convert to word.
#endif //ENABLE_GOP0_RBLK_MIRROR
    HAL_GOP_GetDram_Addr(&pGOPDrvLocalCtx->halCtxLocal, E_GOP0, u8win, &u64tmp1, E_DRV_GOP_DRAMVstr);

    if(pGOPDrvLocalCtx->pDrvCtxShared->apiCtxShared.bVMirror && pGOPDrvLocalCtx->b32BPPTileMode[0])
    {
        HAL_GOP_SetDram_Addr(&pGOPDrvLocalCtx->halCtxLocal, E_GOP0, u8win, (MS_PHY)(pinfo->u16RBlkVPixSize-1), E_DRV_GOP_DRAMVstr);
    }
#if ENABLE_GOP0_RBLK_MIRROR
    else if (u64tmp !=  u64tmp1)
#else
    else if (pGOPDrvLocalCtx->pDrvCtxShared->apiCtxShared.bVMirror &&(u64tmp != u64tmp1)
#endif
    {
        HAL_GOP_SetDram_Addr(&pGOPDrvLocalCtx->halCtxLocal, E_GOP0, u8win, (MS_PHY)u64tmp, E_DRV_GOP_DRAMVstr);
    }
    //GOP HW just read the relative offset of each MIU
    _phy_to_miu_offset(u8MiuSel, u64tmp, pinfo->u64DRAMRBlkStart);
    u64tmp /= u16GOP_Unit;
    UNUSED(u8MiuSel);
    HAL_GOP_SetDram_Addr(&pGOPDrvLocalCtx->halCtxLocal, E_GOP0, u8win, (MS_PHY)u64tmp, E_DRV_GOP_RBLKAddr);
    u64tmp = ((MS_U32)pinfo->u16RBlkHRblkSize * (MS_U32)pinfo->u16RBlkVPixSize)/u16GOP_Unit;

    _GOP_GWIN_FB_AddrAlignChecker((MS_GOP_CTX_LOCAL*)pGOPDrvLocalCtx, E_GOP0, u64tmp, bpp);
    HAL_GOP_SetDram_Addr(&pGOPDrvLocalCtx->halCtxLocal, E_GOP0, u8win, (MS_PHY)u64tmp, E_DRV_GOP_RBLKSize);
	HAL_GOP_SetPitch(&pGOPDrvLocalCtx->halCtxLocal, E_GOP0, u8win, u16Pitch, pinfo->u16RBlkVPixSize);

    GOP_SetGopGwinHVPixel(pGOPDrvLocalCtx, u8win, E_GOP0 ,pinfo);

    GOP_D_INFO("GWIN_SetWin(%d): [adr, RBsz, offset](Unit:Byte) = [%td, %d, %d]\n", \
                u8win,\
                (ptrdiff_t)pinfo->u64DRAMRBlkStart,\
                pinfo->u16RBlkHPixSize * pinfo->u16RBlkVPixSize / (u16GOP_Unit/(bpp>>3)),\
                (pinfo->u16WinY * pinfo->u16RBlkHPixSize + pinfo->u16WinX)/(u16GOP_Unit/(bpp>>3)) );
    GOP_D_INFO("\t[Vst, Vend, Hst, Hend, GwinHsz](Unit:Pixel) = [%d, %d, %d, %d, %d]\n",\
                pinfo->u16DispVPixelStart, \
                pinfo->u16DispVPixelEnd, \
                pinfo->u16DispHPixelStart, \
                pinfo->u16DispHPixelEnd, \
                pinfo->u16RBlkHPixSize);
}

static void GOP_ReadGopExtendGwinInfo(GOP_CTX_DRV_LOCAL*pGOPDrvLocalCtx, MS_U8 u8win, DRV_GOP_GWIN_INFO* pinfo)
{
    MS_PHY phyTmp =0;
    MS_U16 u16Bpp, u16tmp =0;
    MS_U8 u8GOP=0;
    MS_U16 u16GOP_Unit=0;
    MS_U8 u8win_temp = 0;

    if (FALSE== _IsGwinIdValid((MS_GOP_CTX_LOCAL*)pGOPDrvLocalCtx, u8win))
    {
        GOP_D_ERR("\n[%s] not support gwin id:%d in this chip version",__FUNCTION__, u8win);
        return;
    }

    switch(u8win)
    {
        case GOP2_Gwin0Id:
            u8GOP = 2;
            u8win_temp = (u8win - MAX_GOP0_GWIN - MAX_GOP1_GWIN);
            break;
        case GOP3_Gwin0Id:
            u8GOP = 3;
            u8win_temp = (u8win - MAX_GOP0_GWIN - MAX_GOP1_GWIN - MAX_GOP2_GWIN);
            break;
        case GOP4_Gwin0Id:
            u8GOP = 4;
            u8win_temp = (u8win - MAX_GOP0_GWIN - MAX_GOP1_GWIN - MAX_GOP2_GWIN - MAX_GOP3_GWIN);
            break;
        case GOP5_Gwin0Id:
            u8GOP = 5;
            u8win_temp = (u8win - MAX_GOP0_GWIN - MAX_GOP1_GWIN - MAX_GOP2_GWIN - MAX_GOP3_GWIN - MAX_GOP4_GWIN);
            break;
        default:
            GOP_D_ERR("[%s] invalud GwinNum\n",__FUNCTION__);
            return;
    }

    if (FALSE== _IsGopNumVaild((MS_GOP_CTX_LOCAL*)pGOPDrvLocalCtx, u8GOP))
    {
        GOP_D_ERR("\n[%s] not support GOP id:%d in this chip version",__FUNCTION__, u8GOP);
        return;
    }

	HAL_GOP_GetWinFmt(&pGOPDrvLocalCtx->halCtxLocal, u8GOP, u8win_temp, &u16tmp);
    pinfo->clrType = (DRV_GOPColorType)GOP_GetFmt(u16tmp);
    u16Bpp = HAL_GOP_GetBPP(&pGOPDrvLocalCtx->halCtxLocal, pinfo->clrType);
    if(u16Bpp == FB_FMT_AS_DEFAULT)
    {
        return;
    }

    GOP_GetGopGwinHVPixel(pGOPDrvLocalCtx, u8win, u8GOP, pinfo);

	u16tmp = HAL_GOP_GetPitch(&pGOPDrvLocalCtx->halCtxLocal, u8GOP, u8win_temp);
    u16GOP_Unit = MDrv_GOP_GetWordUnit((MS_GOP_CTX_LOCAL*)pGOPDrvLocalCtx, u8GOP);
    if(pGOPDrvLocalCtx->apiCtxLocal.pGOPCtxShared->bPixelMode[u8GOP] == TRUE)
    {
        pinfo->u16RBlkHRblkSize = u16tmp*(u16Bpp/8);
    }
    else
    {
        pinfo->u16RBlkHRblkSize = u16tmp*u16GOP_Unit;
    }
    HAL_GOP_GetDram_Addr(&pGOPDrvLocalCtx->halCtxLocal, u8GOP, u8win_temp, &phyTmp, E_DRV_GOP_RBLKAddr);
    pinfo->u64DRAMRBlkStart = phyTmp*u16GOP_Unit;

    GOP_D_INFO("GWIN_GetWin(%d): [adr(B), Hsz, Vsz, Hsdrm, winX, winY ] = [%td, %d, %d, %d, %d, %d]\n",
                u8win,
                (ptrdiff_t)pinfo->u64DRAMRBlkStart,
                pinfo->u16RBlkHPixSize,
                pinfo->u16RBlkVPixSize,
                pinfo->u16RBlkHRblkSize,
                pinfo->u16WinX,
                pinfo->u16WinY);


}

static void GOP_ReadGop1GwinInfo(GOP_CTX_DRV_LOCAL*pGOPDrvLocalCtx, MS_U8 u8win, DRV_GOP_GWIN_INFO* pinfo)
{
    MS_PHY phyTmp =0;
    MS_U16 u16tmp =0;
    MS_U16 bpp;
    MS_U16 u16GOP_Unit=0;

    u8win -= MAX_GOP0_GWIN; //Reset win number to 0~3 in GOP1
	HAL_GOP_GetWinFmt(&pGOPDrvLocalCtx->halCtxLocal, E_GOP1, u8win, &u16tmp);
    pinfo->clrType = (DRV_GOPColorType)GOP_GetFmt(u16tmp);
    bpp = HAL_GOP_GetBPP(&pGOPDrvLocalCtx->halCtxLocal, pinfo->clrType);
    if(bpp == FB_FMT_AS_DEFAULT)
    {
        return;
    }

    GOP_GetGopGwinHVPixel(pGOPDrvLocalCtx, u8win, E_GOP1, pinfo);

	u16tmp = HAL_GOP_GetPitch(&pGOPDrvLocalCtx->halCtxLocal, E_GOP1, u8win);
    u16GOP_Unit = MDrv_GOP_GetWordUnit((MS_GOP_CTX_LOCAL*)pGOPDrvLocalCtx, E_GOP1);
    if(pGOPDrvLocalCtx->apiCtxLocal.pGOPCtxShared->bPixelMode[E_GOP1] == TRUE)
    {
        pinfo->u16RBlkHRblkSize = u16tmp*(bpp/8);
    }
    else
    {
        pinfo->u16RBlkHRblkSize = u16tmp*u16GOP_Unit;
    }
    HAL_GOP_GetDram_Addr(&pGOPDrvLocalCtx->halCtxLocal, E_GOP1, u8win, &phyTmp, E_DRV_GOP_RBLKAddr);
    pinfo->u64DRAMRBlkStart = phyTmp*u16GOP_Unit;

    GOP_D_INFO("GWIN_GetWin(%d): [adr(B), Hsz, Vsz, Hsdrm, winX, winY ] = [%td, %d, %d, %d, %d, %d]\n",
                u8win,
                (ptrdiff_t)pinfo->u64DRAMRBlkStart,
                pinfo->u16RBlkHPixSize,
                pinfo->u16RBlkVPixSize,
                pinfo->u16RBlkHRblkSize,
                pinfo->u16WinX,
                pinfo->u16WinY);
}

static void GOP_ReadGop0GwinInfo(GOP_CTX_DRV_LOCAL*pGOPDrvLocalCtx, MS_U8 u8win, DRV_GOP_GWIN_INFO* pinfo)
{
    MS_PHY phyTmp =0;
    MS_U16 u16tmp =0;
    MS_U16 bpp;
    MS_U16 u16GOP_Unit=0;

	HAL_GOP_GetWinFmt(&pGOPDrvLocalCtx->halCtxLocal, 0, u8win, &u16tmp);
    pinfo->clrType = (DRV_GOPColorType)GOP_GetFmt(u16tmp);
    bpp = HAL_GOP_GetBPP(&pGOPDrvLocalCtx->halCtxLocal, pinfo->clrType);
    if(bpp == FB_FMT_AS_DEFAULT)
    {
        return;
    }

    GOP_GetGopGwinHVPixel(pGOPDrvLocalCtx, u8win, E_GOP0, pinfo);

	u16tmp = HAL_GOP_GetPitch(&pGOPDrvLocalCtx->halCtxLocal, E_GOP0, u8win);
    u16GOP_Unit = MDrv_GOP_GetWordUnit((MS_GOP_CTX_LOCAL*)pGOPDrvLocalCtx, E_GOP0);
    if(pGOPDrvLocalCtx->apiCtxLocal.pGOPCtxShared->bPixelMode[E_GOP0] == TRUE)
    {
        pinfo->u16RBlkHRblkSize = u16tmp*(bpp/8);
    }
    else
    {
        pinfo->u16RBlkHRblkSize = u16tmp*u16GOP_Unit;
    }
    HAL_GOP_GetDram_Addr(&pGOPDrvLocalCtx->halCtxLocal, E_GOP0, u8win, &phyTmp, E_DRV_GOP_RBLKAddr);
    pinfo->u64DRAMRBlkStart = phyTmp*u16GOP_Unit;

    phyTmp = 0;
    HAL_GOP_GetDram_Addr(&pGOPDrvLocalCtx->halCtxLocal, E_GOP0, u8win, &phyTmp, E_DRV_GOP_RBLKSize);

    if ((pinfo->u16RBlkHRblkSize!=0)&&(phyTmp!=0))
        pinfo->u16RBlkVPixSize = ((phyTmp *u16GOP_Unit) / pinfo->u16RBlkHRblkSize);

    GOP_D_INFO("GWIN_GetWin(%d): [adr(B), Hsz, Vsz, Hsdrm ] = [%td, %d, %d, %d]\n",
                u8win,
                (ptrdiff_t)pinfo->u64DRAMRBlkStart,
                pinfo->u16RBlkHPixSize,
                pinfo->u16RBlkVPixSize,
                pinfo->u16RBlkHRblkSize
                    );
}

MS_BOOL _GWIN_ADDR_Invalid_Check(MS_GOP_CTX_LOCAL*pGOPCtx, MS_U8 u8win)
{

    return TRUE;
}

//------------------------------------------------------------------------------
//  Global Functions
//------------------------------------------------------------------------------
MS_U8 MDrv_DumpGopByGwinId(MS_GOP_CTX_LOCAL*pGOPCtx,MS_U32 GwinID)
{
    MS_U8 gop_num=0;
    MS_U8 gop_gwinnum[SHARED_GOP_MAX_COUNT];
    MS_U8 gop_start[SHARED_GOP_MAX_COUNT];
    GOP_CTX_DRV_LOCAL*pGOPDrvLocalCtx = (GOP_CTX_DRV_LOCAL*)pGOPCtx;

    for (gop_num=0; gop_num < gGopChipPro.TotalGOPNum; gop_num++)
    {
        gop_start[gop_num]=0;
        gop_gwinnum[gop_num]=0;
    }

    for (gop_num=0; gop_num < gGopChipPro.TotalGOPNum; gop_num++)
    {
        gop_gwinnum[gop_num] = MDrv_GOP_GetGwinNum((MS_GOP_CTX_LOCAL*)pGOPDrvLocalCtx,gop_num);
    }

    gop_start[0] = 0;
    for (gop_num=1; gop_num < gGopChipPro.TotalGOPNum; gop_num++)
    {
        gop_start[gop_num] = gop_start[gop_num-1] + gop_gwinnum[gop_num-1];
    }

    for (gop_num=0; gop_num < gGopChipPro.TotalGOPNum; gop_num++)
    {
        if( gop_num == 0 )
        {
            if(GwinID < gop_start[gop_num+1])
                return gop_num;
        }
        else if( gop_num ==  (gGopChipPro.TotalGOPNum-1) )
        {
            if( (GwinID >= gop_start[gop_num]) && (GwinID < pGOPDrvLocalCtx->halCtxLocal.pGopChipPro->TotalGwinNum) )
                return gop_num;
        }
        else
        {
            if( (GwinID >= gop_start[gop_num]) && (GwinID < gop_start[gop_num+1]) )
                return gop_num;
        }
    }

    GOP_ASSERT(0);
    return INVAILD_GOP_NUM;
}

MS_BOOL MDrv_GOP_GetGOPEnum(MS_GOP_CTX_LOCAL*pGOPCtx, GOP_TYPE_DEF* GOP_TYPE)
{
    GOP_CTX_DRV_LOCAL*pGOPDrvLocalCtx = (GOP_CTX_DRV_LOCAL*)pGOPCtx;
    HAL_GOP_GetGOPEnum(&pGOPDrvLocalCtx->halCtxLocal, GOP_TYPE);
    return TRUE;
}

MS_U16 MDrv_GOP_GetGOPACK(MS_GOP_CTX_LOCAL *pstGOPCtx, MS_U8 enGopType)
{
    MS_BOOL bGopAck = FALSE;
    GOP_CTX_DRV_LOCAL*pstGOPDrvLocalCtx = NULL;
    MS_U32 u32BankOffSet=0;

#ifdef GOP_AUTO_CLK_GATING_PATCH
    MS_BOOL bIsAllGwinEn = FALSE;
#endif
    if (NULL == pstGOPCtx)
    {
        return bGopAck;
    }

    pstGOPDrvLocalCtx = (GOP_CTX_DRV_LOCAL*)pstGOPCtx;

    _GetBnkOfstByGop(enGopType, &u32BankOffSet);
    if (MDrv_GOP_GWIN_IsForceWrite(pstGOPCtx, enGopType))
    {
        bGopAck = TRUE;
    }
    else
    {
        bGopAck = (MS_BOOL)HAL_GOP_GetGOPACK(&pstGOPDrvLocalCtx->halCtxLocal, enGopType);
    }

    return bGopAck;
}

MS_U8 MDrv_GOP_GetMaxGOPNum(MS_GOP_CTX_LOCAL*pGOPCtx)
{
    return (MS_U8)gGopChipPro.TotalGOPNum;
}

MS_U8 MDrv_GOP_GetMaxMuxSel(MS_GOP_CTX_LOCAL*pGOPCtx)
{
    return (MS_U8)MAX_GOP_MUX_SEL;
}

MS_U8 MDrv_GOP_GetGwinNum(MS_GOP_CTX_LOCAL*pGOPCtx, MS_U8 u8GopNum)
{
    GOP_CTX_DRV_LOCAL*pGOPDrvLocalCtx = (GOP_CTX_DRV_LOCAL*)pGOPCtx;
    return HAL_GOP_GetMaxGwinNumByGOP(&pGOPDrvLocalCtx->halCtxLocal, u8GopNum);
}

MS_U8 MDrv_GetMaxMuxOPNum(void)
{
    return MAX_GOP_MUX_OPNum;
}

#ifdef CONFIG_GOP_AFBC_FEATURE
MS_BOOL MDrv_GetAFBCCapsSupport(MS_GOP_CTX_LOCAL*pGOPCtx)
{
    MS_U8       i=0;
    MS_BOOL     bAFBCStatus=FALSE;

    for(i=0;i<MDrv_GOP_GetMaxGOPNum(pGOPCtx);i++)
    {
        bAFBCStatus |= pGOPCtx->pGopChipProperty->bAFBC_Support[i];
    }
    return bAFBCStatus;
}
#endif

GOP_Result MDrv_GOP_Sel(MS_GOP_CTX_LOCAL*pGOPCtx, MS_U8 u8GOPNum)
{
	GOP_CTX_DRV_LOCAL*pGOPDrvLocalCtx = (GOP_CTX_DRV_LOCAL*)pGOPCtx;
	pGOPDrvLocalCtx->current_gop = u8GOPNum;

	return HAL_GOP_GOPSel(&pGOPDrvLocalCtx->halCtxLocal, u8GOPNum);
}

MS_U8 MDrv_GOP_Get(MS_GOP_CTX_LOCAL*pGOPCtx)
{
    GOP_CTX_DRV_LOCAL*pGOPDrvLocalCtx = (GOP_CTX_DRV_LOCAL*)pGOPCtx;
    return pGOPDrvLocalCtx->current_gop;
}

MS_BOOL MDrv_GOP_SetIOMapBase(MS_GOP_CTX_LOCAL*pGOPCtx)
{
    MS_VIRT MMIOBaseAdr = 0;
    GOP_CTX_DRV_LOCAL*pGOPDrvLocalCtx = (GOP_CTX_DRV_LOCAL*)pGOPCtx;

     if(pGOPDrvLocalCtx == NULL)
    {
        return UTOPIA_STATUS_PARAMETER_ERROR;
    }
    HAL_GOP_SetIOMapBase(&pGOPDrvLocalCtx->halCtxLocal, MMIOBaseAdr);

    return TRUE;
}

// for identify GOP reg format
MS_U32 MDrv_GOP_GetRegForm(MS_GOP_CTX_LOCAL*pGOPCtx, MS_U8 u8gop, MS_U8 u8gwin)
{
    MS_U32 ret = 0xffff;

    if(u8gop != INVAILD_GOP_NUM)
    {
        switch(u8gop)
        {
            case E_GOP0:
                ret = GOP0_REG_FORM;
                break;
            case E_GOP1:
                ret = GOP1_REG_FORM;
                break;
            case E_GOP2:
                ret = GOP2_REG_FORM;
                break;
            case E_GOP3:
                ret = GOP3_REG_FORM;
                break;
            case E_GOP4:
                ret = GOP4_REG_FORM;
                break;
            case E_GOP5:
                ret = GOP5_REG_FORM;
                break;
            default:
                ret = 0xffff;
        }
    }
    else if(u8gwin != INVALID_GWIN_ID)
    {
        switch(MDrv_DumpGopByGwinId(pGOPCtx,u8gwin))
        {
            case E_GOP0:
                ret = GOP0_REG_FORM;
                break;
            case E_GOP1:
                ret = GOP1_REG_FORM;
                break;
            case E_GOP2:
                ret = GOP2_REG_FORM;
                break;
            case E_GOP3:
                ret = GOP3_REG_FORM;
                break;
            case E_GOP4:
                ret = GOP4_REG_FORM;
                break;
            case E_GOP5:
                ret = GOP5_REG_FORM;
                break;
            default:
                ret = 0xffff;
        }

    }

    return ret ;

}

void* MDrv_GOP_GetShareMemory(MS_BOOL *pbNeedInitShared)
{
    GOP_CTX_DRV_SHARED *pDrvGOPShared=NULL;
    MS_BOOL bNeedInitShared = FALSE;

#if defined(MSOS_TYPE_LINUX) || defined(MSOS_TYPE_LINUX_KERNEL)
    MS_U32 u32ShmId;
    MS_VIRT VAddr;
    MS_U32 u32BufSize;
    char SHM_Id[] = "Linux GOP driver";

    if (FALSE == MsOS_SHM_GetId((MS_U8*)SHM_Id, sizeof(GOP_CTX_DRV_SHARED), &u32ShmId, &VAddr, &u32BufSize, MSOS_SHM_QUERY))
    {
        if (FALSE == MsOS_SHM_GetId((MS_U8*)SHM_Id, sizeof(GOP_CTX_DRV_SHARED), &u32ShmId, &VAddr, &u32BufSize, MSOS_SHM_CREATE))
        {
            GOP_D_ERR("SHM allocation failed!\n");
            return NULL;
        }
        GOP_D_INFO("[%s][%d] This is first initial 0x%tx\n", __FUNCTION__, __LINE__, (ptrdiff_t)VAddr);
        memset( (MS_U8*)VAddr, 0, sizeof(GOP_CTX_DRV_SHARED));
        pDrvGOPShared = (GOP_CTX_DRV_SHARED*)VAddr;
        pDrvGOPShared->apiCtxShared.bInitShared = TRUE;
        bNeedInitShared = TRUE;
        //CSC SHM Init
    }
    pDrvGOPShared = (GOP_CTX_DRV_SHARED*)VAddr;
#else
    pDrvGOPShared =  &g_gopDrvCtxShared;
    bNeedInitShared = TRUE;
#endif

    *pbNeedInitShared = (bNeedInitShared | pDrvGOPShared->apiCtxShared.bInitShared);
    return (void*)pDrvGOPShared;
}


MS_U32 readChipCaps_DTB(void)
{
	int ret = 0, i = 0, length_needed = 0;
	char cstr[100] = "";
	MS_U32 u32GOPNum = 0, u32Tmp =0;

	memset(&gGopChipPro, 0, sizeof(GOP_CHIP_PROPERTY));
	ret = parse_dt("/mtk-gop0", integer_dt_parser, (void*)&u32Tmp, "capability");
	if (ret < 0)
		GOP_D_ERR("Error: GOP0 capability information parse error in DTS\n");

	u32GOPNum = (u32Tmp & GOP_CAPS_OFFSET_GOPNUM_MSK);
	gGopChipPro.TotalGOPNum = u32GOPNum;
	gGopChipPro.bSupportVOPPathSel = 
		(MS_BOOL) ((u32Tmp & BIT(GOP_CAPS_OFFSET_VOP_PATHSEL)) >>
		GOP_CAPS_OFFSET_VOP_PATHSEL);

	gGopChipPro.bSupportCSCTuning =
		    (MS_BOOL) ((u32Tmp & BIT(GOP_CAPS_OFFSET_GOP0_CSC)) >>
			       GOP_CAPS_OFFSET_GOP0_CSC);

	gGopChipPro.bUse3x3MartixCSC =
		    (MS_BOOL) ((u32Tmp & BIT(GOP_CAPS_OFFSET_GOP0_CSC)) >>
			       GOP_CAPS_OFFSET_GOP0_CSC);

	for (i = 0; i < u32GOPNum; i++) {
		length_needed = snprintf(cstr, sizeof(cstr), "/mtk-gop%d", i);
		if (length_needed < 0 || (unsigned) length_needed >= sizeof(cstr))
			GOP_D_ERR("Error: buffer too small\n");

		ret = parse_dt(cstr, integer_dt_parser, (void*)&u32Tmp, "capability");
		if (ret < 0)
			GOP_D_ERR("Error: GOP%d capability information parse error in DTS\n", i);
	
		gGopChipPro.bSupportH4Tap_256Phase[i] =
				(MS_BOOL) ((u32Tmp & BIT(gu32GopCapsH4Tap[((i != 0) ? 1 : 0)])) >>
				gu32GopCapsH4Tap[((i != 0) ? 1 : 0)]);
		gGopChipPro.bSupportH6Tap_8Phase[i] =
				(MS_BOOL) ((u32Tmp & BIT(gu32GopCapsH2Tap[((i != 0) ? 1 : 0)])) >>
				gu32GopCapsH2Tap[((i != 0) ? 1 : 0)]);
		gGopChipPro.bSupportHDuplicate[i] =
				(MS_BOOL) ((u32Tmp & BIT(gu32GopCapsHDup[((i != 0) ? 1 : 0)])) >>
				gu32GopCapsHDup[((i != 0) ? 1 : 0)]);
		gGopChipPro.bSupportV2tap_16Phase[i] = FALSE;
		gGopChipPro.bSupportV4tap[i] =
				(MS_BOOL) ((u32Tmp & BIT(gu32GopCapsV4Tap[((i != 0) ? 1 : 0)])) >>
				gu32GopCapsV4Tap[((i != 0) ? 1 : 0)]);
		gGopChipPro.bSupportV_BiLinear[i] =
				(MS_BOOL) ((u32Tmp & BIT(gu32GopCapsVBilinear[((i != 0) ? 1 : 0)])) >>
				gu32GopCapsVBilinear[((i != 0) ? 1 : 0)]);
		gGopChipPro.bAFBC_Support[i] =
				(MS_BOOL) ((u32Tmp & BIT(gu32GopCapsAFBC[((i != 0) ? 1 : 0)])) >>
				gu32GopCapsAFBC[((i != 0) ? 1 : 0)]);
		if ((u32Tmp & gu32GopCapsVScaleModeMsk[((i != 0) ? 1 : 0)]) != 0) {
			gGopChipPro.bGOPWithVscale[i] = TRUE;
			gGopChipPro.bGOPVscalePipeDelay[i] = FALSE;
		} else {
			gGopChipPro.bGOPWithVscale[i] = FALSE;
			gGopChipPro.bGOPVscalePipeDelay[i] = TRUE;
		}
	}

	ret = parse_dt("/mtk-gop0", integer_dt_parser, (void*)&u32Tmp, "ip-version");
	if (ret < 0)
		GOP_D_ERR("Error: GOP ip-version information parse error in DTS\n");
	u32ChipVer = u32Tmp;

	HAL_GOP_SetIPVersion(u32ChipVer);

	ret = parse_dt("/mediatek-drm-gop-drvconfig", integer_dt_parser, (void*)&gGopChipPro.TotalGwinNum, "GOP_TOTAL_GWIN_NUMBER");
	if (ret < 0)
		GOP_D_ERR("Error: GOP_TOTAL_GWIN_NUMBER information parse error in DTS\n");

	ret = parse_dt("/mediatek-drm-gop-drvconfig", integer_dt_parser, (void*)&gGopChipPro.u16BefRGB3DLupPDOffset, "GOP_BEFORE_3DLUT_PD");
	if (ret < 0)
		GOP_D_ERR("Error: GOP_BEFORE_3DLUT_PD information parse error in DTS\n");

	ret = parse_dt("/mediatek-drm-gop-drvconfig", integer_dt_parser, (void*)&gGopChipPro.u16BefPQGammaPDOffset, "GOP_BEFORE_PQGAMMA_PD");
	if (ret < 0)
		GOP_D_ERR("Error: GOP_BEFORE_PQGAMMA_PD information parse error in DTS\n");

	ret = parse_dt("/mediatek-drm-gop-drvconfig", integer_dt_parser, (void*)&gGopChipPro.u16AftPQGammaPDOffset, "GOP_AFTER_PQGAMMA_PD");
	if (ret < 0)
		GOP_D_ERR("Error: GOP_AFTER_PQGAMMA_PD information parse error in DTS\n");

	ret = parse_dt("/mediatek-drm-gop-drvconfig", integer_dt_parser, (void*)&gGopChipPro.bOPMuxDoubleBuffer, "GOP_SUPPORT_OPMUX_DB");
	if (ret < 0)
		GOP_D_ERR("Error: GOP_SUPPORT_OPMUX_DB information parse error in DTS\n");

	ret = parse_dt("/mediatek-drm-gop-drvconfig", integer_dt_parser, (void*)&gGopChipPro.GOP_PD, "GOP_PD");
	if (ret < 0)
		GOP_D_ERR("Error: GOP_PD information parse error in DTS\n");

	ret = parse_dt("/mediatek-drm-gop-drvconfig", integer_dt_parser, (void*)&gGopChipPro.GOP_NonVS_PD_Offset, "GOP_NONVSCALE_PD");
	if (ret < 0)
		GOP_D_ERR("Error: GOP_NONVSCALE_PD information parse error in DTS\n");

	ret = parse_dt("/mediatek-drm-gop-drvconfig", integer_dt_parser, (void*)&gGopChipPro.GOP_MUX_Delta, "GOP_MUX_DELTA");
	if (ret < 0)
		GOP_D_ERR("Error: GOP_MUX_DELTA information parse error in DTS\n");

	ret = parse_dt("/mediatek-drm-gop-drvconfig", gop_dt_parser, (void*)&gGopChipPro.GOP_MapLayer2Mux, NULL);
	if (ret < 0)
		GOP_D_ERR("Error: GOP_MapLayer2Mux information parse error in DTS\n");

	ret = parse_dt("/mediatek-drm-gop-drvconfig", integer_dt_parser, (void*)&gGopChipPro.bBnkForceWrite, "GOP_SUPPORT_BNKFORCEWRITE");
	if (ret < 0)
		GOP_D_ERR("Error: GOP_SUPPORT_BNKFORCEWRITE information parse error in DTS\n");

	ret = parse_dt("/mediatek-drm-gop-drvconfig", integer_dt_parser, (void*)&gGopChipPro.bPixelModeSupport, "GOP_SUPPORT_PIXELMODE");
	if (ret < 0)
		GOP_D_ERR("Error: GOP_SUPPORT_PIXELMODE information parse error in DTS\n");

	ret = parse_dt("/mediatek-drm-gop-drvconfig", integer_dt_parser, (void*)&gGopChipPro.b2Pto1PSupport, "GOP_SUPPORT_2PTO1P");
	if (ret < 0)
		GOP_D_ERR("Error: GOP_SUPPORT_2PTO1P information parse error in DTS\n");

	ret = parse_dt("/mediatek-drm-gop-drvconfig", integer_dt_parser, (void*)&gGopChipPro.bAutoAdjustMirrorHSize, "GOP_AUTO_ADJUST_HMIRROR");
	if (ret < 0)
		GOP_D_ERR("Error: GOP_AUTO_ADJUST_HMIRROR information parse error in DTS\n");

	gGopChipPro.WordUnit = GOP_WordUnit;
	gGopChipPro.Default_ConsAlpha_bits = DRV_VALID_8BITS;
	gGopChipPro.enGOP3DType = E_DRV_3D_DUP_FULL;
	gGopChipPro.bSupporBrightness3Chan = TRUE;
	gGopChipPro.enBrightnessInfo = E_DRV_BRI_2SCOMPLEMENT;
	gGopChipPro.bSupportCSCDoubleBuffer = TRUE;

	return 0;
}

void Drv_GOP_Init_ChipPro(GOP_CHIP_PROPERTY **pGopChipPro)
{
	if (readChipCaps_DTB() != 0)
		GOP_D_ERR("[%s, %d]readChipCaps_DTB failed \r\n", __func__, __LINE__);

	*pGopChipPro = &gGopChipPro;
}

MS_GOP_CTX_LOCAL*  Drv_GOP_Init_Context(void* pInstance,MS_BOOL *pbNeedInitShared)
{
    GOP_CTX_DRV_SHARED *pDrvGOPShared;
    MS_BOOL bNeedInitShared = FALSE;
    MS_U8 u8Index = 0;

#ifdef INSTANT_PRIVATE
   GOP_INSTANT_PRIVATE* psGOPInstPri = NULL;
   UtopiaInstanceGetPrivate(pInstance, (void*)&psGOPInstPri);
#endif

    pDrvGOPShared = (GOP_CTX_DRV_SHARED*)MDrv_GOP_GetShareMemory(&bNeedInitShared);

    *pbNeedInitShared = bNeedInitShared;
    memset(&g_gopDrvCtxLocal, 0, sizeof(g_gopDrvCtxLocal));
#ifdef MSOS_TYPE_LINUX
    g_gopDrvCtxLocal.s32FdGFlip = -1;
#endif
    if(bNeedInitShared)
    {
        memset(pDrvGOPShared, 0, sizeof(GOP_CTX_DRV_SHARED));
        for(u8Index = 0;u8Index < SHARED_GOP_MAX_COUNT ;u8Index++)
        {
            if(_MDrv_GOP_CSC_ParamInit(&(pDrvGOPShared->apiCtxShared.stCSCParam[u8Index])) == FALSE)
            {
                GOP_D_ERR("_MDrv_GOP_CSC_ParamInit failed!\n");
            }
            if(_MDrv_GOP_CSC_TableInit(&(pDrvGOPShared->apiCtxShared.stCSCTable[u8Index])) == FALSE)
            {
                GOP_D_ERR("_MDrv_GOP_CSC_TableInit failed!\n");
            }
        }
    }

    g_gopDrvCtxLocal.pDrvCtxShared = pDrvGOPShared;
    g_gopDrvCtxLocal.apiCtxLocal.pGOPCtxShared = & pDrvGOPShared->apiCtxShared;

	Drv_GOP_Init_ChipPro(&g_gopDrvCtxLocal.halCtxLocal.pGopChipPro);

    HAL_GOP_Init_Context(&g_gopDrvCtxLocal.halCtxLocal, &pDrvGOPShared->halCtxShared, bNeedInitShared);
    g_gopDrvCtxLocal.apiCtxLocal.pGopChipProperty = g_gopDrvCtxLocal.halCtxLocal.pGopChipPro;

    g_gopDrvCtxLocal.halCtxLocal.User_ConsAlpha_bits = DRV_VALID_8BITS; //const alpha : 8bits
    g_gopDrvCtxLocal.apiCtxLocal.pGopConsAlphaBits = &(g_gopDrvCtxLocal.halCtxLocal.User_ConsAlpha_bits);

    if(bNeedInitShared)
    {
//#ifdef MSOS_TYPE_LINUX
        MS_U32 gId;
        MS_GOP_CTX_LOCAL *pGOPCtxLocal=&g_gopDrvCtxLocal.apiCtxLocal;
        *pbNeedInitShared = bNeedInitShared;

        if(g_gopDrvCtxLocal.apiCtxLocal.pGopChipProperty->TotalGwinNum>=SHARED_GWIN_MAX_COUNT)
        {
            //assert here!!
            GOP_D_DBUG("Error - TotalGwinNum >= SHARED_GWIN_MAX_COUNT!!\n");
        }

        for( gId=0; gId<g_gopDrvCtxLocal.apiCtxLocal.pGopChipProperty->TotalGwinNum; gId++)
        {
            pGOPCtxLocal->pGOPCtxShared->gwinMap[gId].u32CurFBId = INVALID_GWIN_ID;
            pGOPCtxLocal->pGOPCtxShared->gwinMap[gId].bIsShared = FALSE;
            pGOPCtxLocal->pGOPCtxShared->gwinMap[gId].u16SharedCnt = 0x0;
        }


        for( gId=0; gId<MDrv_GOP_GetMaxGOPNum(pGOPCtxLocal); gId++)
        {
            if(gId>=SHARED_GOP_MAX_COUNT)
            {
                GOP_D_ERR("[%s] GOP idx:%d out of bound\n",__FUNCTION__, gId);
                break;
            }
            pGOPCtxLocal->pGOPCtxShared->s32OutputColorType[gId] = -1;
            pGOPCtxLocal->pGOPCtxShared->bGopHasInitialized[gId] = FALSE;
        }
        // chip proprity is shared, init once is ok.
        HAL_GOP_Chip_Proprity_Init(&g_gopDrvCtxLocal.halCtxLocal);
//#endif //MSOS_TYPE_LINUX
    }

       return (MS_GOP_CTX_LOCAL*)&g_gopDrvCtxLocal;

}

void MDrv_GOP_Init(MS_GOP_CTX_LOCAL*pGOPCtx, MS_U8 u8GOPNum, MS_PHY u64GOP_REGDMABASE, MS_U32 u32GOP_RegdmaLen, MS_BOOL bEnableVsyncIntFlip)
{
    MS_U8 u8MiuSel;
    MS_PHY phyOffset;
    GOP_CTX_DRV_LOCAL*pGOPDrvLocalCtx = (GOP_CTX_DRV_LOCAL*)pGOPCtx;
	MS_GOP_CTX_LOCAL apiCtxLocal = pGOPDrvLocalCtx->apiCtxLocal;
	MS_GOP_CTX_SHARED *pGOPCtxShared = apiCtxLocal.pGOPCtxShared;
	MS_U16 u16PnlWidth = pGOPCtxShared->u16PnlWidth[u8GOPNum];
	MS_U16 u16PnlHeight = pGOPCtxShared->u16PnlHeight[u8GOPNum];

    if (FALSE== _IsGopNumVaild((MS_GOP_CTX_LOCAL*)pGOPDrvLocalCtx, u8GOPNum))
    {
        GOP_D_ERR("\n[%s] not support gop%d in this chip version!!",__FUNCTION__, u8GOPNum);
        return;
    }
    _phy_to_miu_offset(u8MiuSel, phyOffset, u64GOP_REGDMABASE);

    HAL_GOP_Init(&pGOPDrvLocalCtx->halCtxLocal,  u8GOPNum);

#ifdef GOP_SUPPORT_SPLIT_MODE
    // this should be improved.....
    pGOPDrvLocalCtx->halCtxLocal.pHALShared->u16GopSplitMode_LRWIDTH[u8GOPNum] = pGOPCtx->pGOPCtxShared->u16PnlWidth[u8GOPNum];
#endif

    UNUSED(u8MiuSel);
    UNUSED(phyOffset);
	if (u16PnlWidth == GOP_8K_WIDTH && u16PnlHeight == GOP_8K_HEIGHT) {
		HAL_GOP_SetStep2HVSPHscale(&pGOPDrvLocalCtx->halCtxLocal, TRUE, GOP_SCALING_2TIMES);
		HAL_GOP_SetStep2HVSPVscale(&pGOPDrvLocalCtx->halCtxLocal, TRUE, GOP_SCALING_2TIMES);
		HAL_GOP_SetMixer2OutSize(&pGOPDrvLocalCtx->halCtxLocal, GOP_8K_WIDTH, GOP_8K_HEIGHT);
		HAL_GOP_SetMixer4OutSize(&pGOPDrvLocalCtx->halCtxLocal, GOP_4K_WIDTH, GOP_4K_HEIGHT);
	} else {
		//If timing, then only use step1 HVSP to scaling up
		HAL_GOP_SetStep2HVSPHscale(&pGOPDrvLocalCtx->halCtxLocal, FALSE, 0x0);
		HAL_GOP_SetStep2HVSPVscale(&pGOPDrvLocalCtx->halCtxLocal, FALSE, 0x0);
		HAL_GOP_SetMixer2OutSize(&pGOPDrvLocalCtx->halCtxLocal, u16PnlWidth, u16PnlHeight);
		HAL_GOP_SetMixer4OutSize(&pGOPDrvLocalCtx->halCtxLocal, u16PnlWidth, u16PnlHeight);
	}

	GOP_GWIN_UpdateReg(pGOPDrvLocalCtx, (E_GOP_TYPE)u8GOPNum);
}

void MDrv_GOP_PerClientInit(MS_GOP_CTX_LOCAL*pGOPCtx, MS_U8 u8GOPNum, MS_PHY u64GOP_REGDMABASE, MS_U32 u32GOP_RegdmaLen, MS_BOOL bEnableVsyncIntFlip)
{
    //Init GFlip:
#if GFLIP_ENABLE
	MDrv_GOP_GWIN_Interrupt(pGOPCtx, u8GOPNum, bEnableVsyncIntFlip);
#endif

	GOP_GWIN_UpdateReg((GOP_CTX_DRV_LOCAL*)pGOPCtx, (E_GOP_TYPE)u8GOPNum);
}

/********************************************************************************/
/// Set GOP progressive mode
/// @param bEnable \b IN
///   - # TRUE  Progressive (read out the DRAM graphic data by FIELD)
///   - # FALSE Interlaced (not care FIELD)
/// @internal please verify the register document and the code
/********************************************************************************/
void MDrv_GOP_GWIN_EnableProgressive(MS_GOP_CTX_LOCAL*pGOPCtx, MS_U8 u8GOPNum, MS_BOOL bEnable)
{
	GOP_CTX_DRV_LOCAL*pGOPDrvLocalCtx = (GOP_CTX_DRV_LOCAL*)pGOPCtx;

	HAL_GOP_GWIN_SetProgressive(&pGOPDrvLocalCtx->halCtxLocal, u8GOPNum, bEnable);
}

/********************************************************************************/
/// Get GOP progressive mode
/// @return
///   - # TRUE  Progressive (read out the DRAM graphic data by FIELD)
///   - # FALSE Interlaced (not care FIELD)
/// @internal please verify the register document and the code
/********************************************************************************/
MS_BOOL MDrv_GOP_GWIN_IsProgressive(MS_GOP_CTX_LOCAL*pGOPCtx, MS_U8 u8GOPNum)
{
	GOP_CTX_DRV_LOCAL*pGOPDrvLocalCtx = (GOP_CTX_DRV_LOCAL*)pGOPCtx;

	return HAL_GOP_GWIN_GetProgressive(&pGOPDrvLocalCtx->halCtxLocal, u8GOPNum);
}

/********************************************************************************/
/// Set the time when new GWIN settings take effect
/// @param bEnable \b IN
///   - # TRUE the new setting moved from internal register buffer
///            to active registers immediately
///   - # FALSE new settings take effect when next VSYNC is coming
/********************************************************************************/
void MDrv_GOP_GWIN_SetForceWrite(MS_GOP_CTX_LOCAL*pGOPCtx, MS_BOOL bEnable)
{
    GOP_CTX_DRV_LOCAL*pGOPDrvLocalCtx = (GOP_CTX_DRV_LOCAL*)pGOPCtx;
    pGOPDrvLocalCtx->gop_gwin_frwr = bEnable;
}

void MDrv_GOP_GWIN_SetBnkForceWrite(MS_GOP_CTX_LOCAL*pGOPCtx, MS_U8 u8GOP, MS_BOOL bEnable)
{
    GOP_CTX_DRV_LOCAL*pGOPDrvLocalCtx = (GOP_CTX_DRV_LOCAL*)pGOPCtx;
    pGOPDrvLocalCtx->bGOPBankFwr[u8GOP] = bEnable;
}

void MDrv_GOP_GWIN_ForceWrite_Update(MS_GOP_CTX_LOCAL*pGOPCtx, MS_U8 u8GOP, MS_BOOL bEnable)
{
    GOP_CTX_DRV_LOCAL*pGOPDrvLocalCtx = (GOP_CTX_DRV_LOCAL*)pGOPCtx;

    if(pGOPDrvLocalCtx->apiCtxLocal.pGopChipProperty->bBnkForceWrite)
        pGOPDrvLocalCtx->bGOPBankFwr[u8GOP] = bEnable;
    pGOPDrvLocalCtx->gop_gwin_frwr = bEnable;
}

/********************************************************************************/
/// Get the status for GWIN settings take effect
/// @param bEnable \b IN
///   - # TRUE the new setting moved from internal register buffer
///            to active registers immediately
///   - # FALSE new settings take effect when next VSYNC is coming
/********************************************************************************/
MS_BOOL MDrv_GOP_GWIN_IsForceWrite(MS_GOP_CTX_LOCAL*pGOPCtx, MS_U8 u8GOP)
{
    GOP_CTX_DRV_LOCAL*pGOPDrvLocalCtx = (GOP_CTX_DRV_LOCAL*)pGOPCtx;
    return (MS_BOOL)(pGOPDrvLocalCtx->gop_gwin_frwr || pGOPDrvLocalCtx->bGOPBankFwr[u8GOP]);
}

/********************************************************************************/
/// Set GWIN output color domain
/// @param type \b IN YUV or RGB
/********************************************************************************/
void MDrv_GOP_GWIN_OutputColor(MS_GOP_CTX_LOCAL*pGOPCtx,MS_U8 u8GOP, GOP_OupputColor type)
{
    //no using
}

/********************************************************************************/
/// Set transparent color for the GWIN in ARGB8888 format
/// @param clr \b IN transparent color
/// @param mask \b IN mask for trs color
/********************************************************************************/
void MDrv_GOP_GWIN_SetTransClr_8888(MS_GOP_CTX_LOCAL*pGOPCtx,MS_U8 u8GOP, MS_U32 clr, MS_U32 mask)
{
}
/********************************************************************************/
/// Set transparent color for the GWIN in YUV format
/// @param u8GOP \b IN GOP number
/// @param clr \b IN transparent color
/// @param mask \b IN mask for trs color
/********************************************************************************/

void MDrv_GOP_GWIN_SetTransClr_YUV(MS_GOP_CTX_LOCAL*pGOPCtx,MS_U8 u8GOP, MS_U32 clr, MS_U32 mask)
{
}
/********************************************************************************/
/// Enable or Disable transparent color
/// @param fmt \b IN @copydoc EN_GOP_TRANSCLR_FMT
/// @param bEnable \b IN
///   - # TRUE enable transparent color
///   - # FALSE disable transparent color
/********************************************************************************/
void MDrv_GOP_GWIN_EnableTransClr(MS_GOP_CTX_LOCAL*pGOPCtx,MS_U8 u8GOP, GOP_TransClrFmt fmt, MS_BOOL bEnable)
{
	GOP_CTX_DRV_LOCAL*pGOPDrvLocalCtx = (GOP_CTX_DRV_LOCAL*)pGOPCtx;

	HAL_GOP_GWIN_SetTransClr(&pGOPDrvLocalCtx->halCtxLocal, u8GOP, fmt, bEnable);
	
	GOP_GWIN_UpdateReg(pGOPDrvLocalCtx, (E_GOP_TYPE)u8GOP);
}

void MDrv_GOP_GWIN_SetHSPipe(MS_GOP_CTX_LOCAL*pGOPCtx, MS_U8 u8GOPNum, MS_U16 u16HSPipe)
{
    GOP_CTX_DRV_LOCAL*pGOPDrvLocalCtx = (GOP_CTX_DRV_LOCAL*)pGOPCtx;
    DRV_GOPDstType pGopDst = E_DRV_GOP_DST_INVALID;
    MS_U32 u32BankOffSet = 0;
    MS_U16 u16NonVS_PD_Delay=0;
	EN_DRV_GOP_VOP_PATH enVOPSel;

    if (FALSE== _IsGopNumVaild(pGOPCtx, u8GOPNum))
    {
        GOP_D_ERR("\n[%s] not support gop%d in this chip version!!",__FUNCTION__, u8GOPNum);
        return;
    }

    HAL_GOP_GetGOPDst(&pGOPDrvLocalCtx->halCtxLocal, u8GOPNum, &pGopDst);

    if (pGOPCtx->pGopChipProperty->bGOPVscalePipeDelay[u8GOPNum])
    {
        if (pGopDst == E_DRV_GOP_DST_OP_DUAL_RATE)
        {
            u16NonVS_PD_Delay = pGOPCtx->pGopChipProperty->GOP_NonVS_DualOpPD_Offset;
            u16HSPipe+= u16NonVS_PD_Delay;
        }
        else if(pGopDst != E_DRV_GOP_DST_IP0)
        {
            u16NonVS_PD_Delay = (pGOPCtx->pGopChipProperty->b2Pto1PSupport==TRUE)?\
                pGOPCtx->pGopChipProperty->GOP_NonVS_PD_Offset*2:  pGOPCtx->pGopChipProperty->GOP_NonVS_PD_Offset;
            u16HSPipe+= u16NonVS_PD_Delay;
        }
        else
        {
            if(!pGOPCtx->pGopChipProperty->bIgnoreIPHPD)
            {
                u16NonVS_PD_Delay = (pGOPCtx->pGopChipProperty->b2Pto1PSupport==TRUE)?\
                    pGOPCtx->pGopChipProperty->GOP_NonVS_PD_Offset*2:  pGOPCtx->pGopChipProperty->GOP_NonVS_PD_Offset;
                u16HSPipe+= u16NonVS_PD_Delay;
            }
        }
    }

    if(pGOPCtx->pGopChipProperty->bSupportVOPPathSel)
    {
		enVOPSel = Hal_GOP_GetVOPPathSel(&pGOPDrvLocalCtx->halCtxLocal);
        switch(enVOPSel)
        {
            case E_DRV_GOP_VOPPATH_BEF_RGB3DLOOKUP:
                u16HSPipe = u16HSPipe - pGOPCtx->pGopChipProperty->u16BefRGB3DLupPDOffset;
                break;
            case E_DRV_GOP_VOPPATH_BEF_PQGAMMA:
                u16HSPipe = u16HSPipe - pGOPCtx->pGopChipProperty->u16BefPQGammaPDOffset;
                break;
            case E_DRV_GOP_VOPPATH_AFT_PQGAMMA:
                u16HSPipe = u16HSPipe - pGOPCtx->pGopChipProperty->u16AftPQGammaPDOffset;
                break;
            default:
                break;
        }
    }

    //Pixel shift PD offset
    u16HSPipe = u16HSPipe - pGOPCtx->pGOPCtxShared->s32PixelShiftPDOffset;

    _GetBnkOfstByGop(u8GOPNum, &u32BankOffSet);

	HAL_GOP_SetPipe(&pGOPDrvLocalCtx->halCtxLocal, u8GOPNum, u16HSPipe);

}

void MDrv_GOP_GWIN_GetMux(MS_GOP_CTX_LOCAL*pGOPCtx, MS_U8* u8GOPNum, Gop_MuxSel eGopMux)
{

    GOP_CTX_DRV_LOCAL*pGOPDrvLocalCtx = (GOP_CTX_DRV_LOCAL*)pGOPCtx;

    HAL_GOP_GWIN_GetMUX(&pGOPDrvLocalCtx->halCtxLocal, u8GOPNum, eGopMux);

}

void MDrv_GOP_GWIN_SetMux(MS_GOP_CTX_LOCAL*pGOPCtx, MS_U8 u8GOPNum, Gop_MuxSel eGopMux)
{
    GOP_CTX_DRV_LOCAL*pGOPDrvLocalCtx = (GOP_CTX_DRV_LOCAL*)pGOPCtx;

    if (!_IsMuxSelVaild(pGOPCtx, u8GOPNum))
    {
        GOP_D_ERR("\n[%s] not support gop%d in this chip version!!",__FUNCTION__, u8GOPNum);
        return;
    }
    HAL_GOP_GWIN_SetMUX(&pGOPDrvLocalCtx->halCtxLocal, u8GOPNum, eGopMux);

}

void MDrv_GOP_MapLayer2Mux(MS_GOP_CTX_LOCAL*pGOPCtx, MS_U32 u32Layer, MS_U8 u8GopNum, MS_U32 *pu32Mux)
{
    GOP_CTX_DRV_LOCAL*pGOPDrvLocalCtx = (GOP_CTX_DRV_LOCAL*)pGOPCtx;
    DRV_GOPDstType gopDst=E_DRV_GOP_DST_OP0;

    MDrv_GOP_GWIN_GetDstPlane(pGOPCtx, u8GopNum, &gopDst);

    *pu32Mux = pGOPDrvLocalCtx->halCtxLocal.pGopChipPro->GOP_MapLayer2Mux[u32Layer];
}

GOP_Result MDrv_GOP_SetGOPHighPri(MS_GOP_CTX_LOCAL*pGOPCtx, MS_U8 u8GOPNum)
{
    GOP_Result GOPRet;
    GOP_CTX_DRV_LOCAL*pGOPDrvLocalCtx = (GOP_CTX_DRV_LOCAL*)pGOPCtx;

    if (FALSE== _IsGopNumVaild(pGOPCtx, u8GOPNum))
    {
        GOP_D_ERR("\n[%s] not support gop%d in this chip version!!",__FUNCTION__, u8GOPNum);
        return GOP_FAIL;
    }
    GOPRet = HAL_GOP_SetGOPHighPri(&pGOPDrvLocalCtx->halCtxLocal, u8GOPNum);
    return GOPRet;
}


/********************************************************************************/
/// Set GOP0 and GOP1 scaler setting
/// @param gopNum \b IN  0: GOP0  1:GOP1
/// @param bEnable \b IN
///   - # TRUE enable
///   - # FALSE disable
/********************************************************************************/
void MDrv_GOP_SetGOPEnable2SC(MS_GOP_CTX_LOCAL*pGOPCtx, MS_U8 gopNum, MS_BOOL bEnable)
{
    GOP_CTX_DRV_LOCAL*pGOPDrvLocalCtx = (GOP_CTX_DRV_LOCAL*)pGOPCtx;

    if (!_IsMuxSelVaild(pGOPCtx, gopNum))
    {
        GOP_D_ERR("\n[%s] not support gop%d in this chip version!!",__FUNCTION__, gopNum);
        return;
    }
    HAL_GOP_SetGOPEnable2SC(&pGOPDrvLocalCtx->halCtxLocal, gopNum, bEnable);
}
#ifdef CONFIG_GOP_GWIN_MISC
void MDrv_GOP_SetGOPEnable2Mode1(MS_GOP_CTX_LOCAL*pGOPCtx, MS_U8 gopNum, MS_BOOL bEnable)
{
    GOP_CTX_DRV_LOCAL*pGOPDrvLocalCtx = (GOP_CTX_DRV_LOCAL*)pGOPCtx;
    DRV_GOPDstType enGopDst = E_DRV_GOP_DST_INVALID;
    if (!_IsMuxSelVaild(pGOPCtx, gopNum))
    {
        GOP_D_ERR("\n[%s] not support gop%d in this chip version!!",__FUNCTION__, gopNum);
        return;
    }
    HAL_GOP_SetGOPEnable2Mode1(&pGOPDrvLocalCtx->halCtxLocal, gopNum, bEnable);

    HAL_GOP_GetGOPDst(&pGOPDrvLocalCtx->halCtxLocal, gopNum, &enGopDst);
    if ((enGopDst == E_DRV_GOP_DST_OP0) || (enGopDst == E_DRV_GOP_DST_OP1))
    {
        MDrv_GOP_GWIN_SetAlphaInverse(pGOPCtx, gopNum, !bEnable);
    }
}

void MDrv_GOP_GetGOPEnable2Mode1(MS_GOP_CTX_LOCAL*pGOPCtx, MS_U8 gopNum, MS_BOOL *pbEnable)
{
    GOP_CTX_DRV_LOCAL*pGOPDrvLocalCtx = (GOP_CTX_DRV_LOCAL*)pGOPCtx;
    if (!_IsMuxSelVaild(pGOPCtx, gopNum))
    {
        GOP_D_ERR("\n[%s] not support gop%d in this chip version!!",__FUNCTION__, gopNum);
        return;
    }
    HAL_GOP_GetGOPAlphaMode1(&pGOPDrvLocalCtx->halCtxLocal, gopNum, pbEnable);
}
#endif

GOP_Result MDrv_GOP_Set_VE_MUX(MS_GOP_CTX_LOCAL*pGOPCtx, MS_U8 gopNum, MS_U8 bEn)
{
    GOP_Result GOPRet;
    GOP_CTX_DRV_LOCAL*pGOPDrvLocalCtx = (GOP_CTX_DRV_LOCAL*)pGOPCtx;

    if (!_IsMuxSelVaild(pGOPCtx, gopNum))
    {
        GOP_D_ERR("\n[%s] not support gop%d in this chip version!!",__FUNCTION__, gopNum);
        return GOP_FAIL;
    }
    GOPRet = HAL_GOP_SetGOPToVE(&pGOPDrvLocalCtx->halCtxLocal, gopNum, bEn);
    return GOPRet;
}

/********************************************************************************/
/// Set GOP destination (OP/IP) setting to scaler
/// @param ipSelGop \b IN \copydoc MS_IPSEL_GOP
/********************************************************************************/
void MDrv_GOP_SetIPSel2SC(MS_GOP_CTX_LOCAL*pGOPCtx, MS_IPSEL_GOP ipSelGop)
{
    GOP_CTX_DRV_LOCAL*pGOPDrvLocalCtx = (GOP_CTX_DRV_LOCAL*)pGOPCtx;
    HAL_GOP_SetIPSel2SC(&pGOPDrvLocalCtx->halCtxLocal, ipSelGop);
}

/********************************************************************************/
/// Set GOP destination clock
/// @param gopNum \b IN 0:GOP0  1:GOP1
/// @param eDstType \b IN \copydoc EN_GOP_DST_TYPE
/********************************************************************************/
GOP_Result MDrv_GOP_SetGOPClk(MS_GOP_CTX_LOCAL*pGOPCtx, MS_U8 gopNum, DRV_GOPDstType eDstType)
{
    GOP_Result GopRet;
    GOP_CTX_DRV_LOCAL*pGOPDrvLocalCtx = (GOP_CTX_DRV_LOCAL*)pGOPCtx;

    if (FALSE== _IsGopNumVaild(pGOPCtx, gopNum))
    {
        GOP_D_ERR("\n[%s] not support gop%d in this chip version!!",__FUNCTION__, gopNum);
        return GOP_FAIL;
    }
    GopRet = HAL_GOP_SetGOPClk(&pGOPDrvLocalCtx->halCtxLocal, gopNum, eDstType);
    return GopRet;
}

void MDrv_GOP_SetClkForCapture(MS_GOP_CTX_LOCAL*pGOPCtx, DRV_GOP_DWIN_SRC_SEL enSrcSel)
{
    GOP_CTX_DRV_LOCAL*pGOPDrvLocalCtx = (GOP_CTX_DRV_LOCAL*)pGOPCtx;
    HAL_GOP_SetClkForCapture(&pGOPDrvLocalCtx->halCtxLocal, enSrcSel);
}

/********************************************************************************/
/// Set GOP alpha inverse
/// @param bEnable \b IN
///   - # TRUE enable alpha inverse
///   - # FALSE disable alpha inverse
/********************************************************************************/
void MDrv_GOP_GWIN_SetAlphaInverse(MS_GOP_CTX_LOCAL*pGOPCtx, MS_U8 u8gopNum, MS_BOOL bEnable)
{
    MS_U32 u32BankOffSet=0;
    DRV_GOPDstType enGopDst = E_DRV_GOP_DST_INVALID;
    GOP_CTX_DRV_LOCAL*pGOPDrvLocalCtx = (GOP_CTX_DRV_LOCAL*)pGOPCtx;

    _GetBnkOfstByGop(u8gopNum, &u32BankOffSet);
    HAL_GOP_GetGOPDst(&pGOPDrvLocalCtx->halCtxLocal, u8gopNum, &enGopDst);
    if (enGopDst == E_DRV_GOP_DST_OP0)
    {
        MS_BOOL bPreAlpha = FALSE;

        MDrv_GOP_GetGOPEnable2Mode1(pGOPCtx, u8gopNum, &bPreAlpha);
        if(bPreAlpha)
        {
            if(bEnable)
            {
                GOP_D_WARN("GOP%d AlphaInverse will be set to false dueto new alpha mode,please ignore this message. %s\n",u8gopNum,__FUNCTION__);
            }
            bEnable = FALSE;
        }
    }

	HAL_GOP_GWIN_SetAlphaInverse(&pGOPDrvLocalCtx->halCtxLocal, u8gopNum, bEnable);
}

//-------------------------------------------------------------------------------------------------
/// Set GOP Destination DisplayPlane
/// @param u8GOP_num \b IN: GOP number: 0 or 1
/// @param eDstType \b IN: GOP Destination DisplayPlane Type
/// @return TRUE: sucess / FALSE: fail
//-------------------------------------------------------------------------------------------------
GOP_Result MDrv_GOP_GWIN_SetDstPlane(MS_GOP_CTX_LOCAL*pGOPCtx, MS_U8 u8GOPNum, DRV_GOPDstType eDstType,MS_BOOL bOnlyCheck)
{
    GOP_Result ret;
    GOP_CTX_DRV_LOCAL*pGOPDrvLocalCtx = (GOP_CTX_DRV_LOCAL*)pGOPCtx;

    if (FALSE== _IsGopNumVaild(pGOPCtx, u8GOPNum))
    {
        GOP_D_ERR("\n[%s] not support gop%d in this chip version!!",__FUNCTION__, u8GOPNum);
        return GOP_FAIL;
    }
    ret = HAL_GOP_GWIN_SetDstPlane(&pGOPDrvLocalCtx->halCtxLocal,u8GOPNum, eDstType,bOnlyCheck);
    if(bOnlyCheck == FALSE)
    {
        //Adjust field by dst
        HAL_GOP_AdjustField(&pGOPDrvLocalCtx->halCtxLocal,u8GOPNum,eDstType);

        GOP_GWIN_UpdateReg(pGOPDrvLocalCtx, (E_GOP_TYPE)u8GOPNum);
    }
    return ret;
}

GOP_Result MDrv_GOP_GWIN_GetDstPlane(MS_GOP_CTX_LOCAL*pGOPCtx, MS_U8 u8GOPNum, DRV_GOPDstType *pGopDst)
{
    GOP_Result ret;

    GOP_CTX_DRV_LOCAL*pGOPDrvLocalCtx = (GOP_CTX_DRV_LOCAL*)pGOPCtx;
    if (FALSE== _IsGopNumVaild(pGOPCtx, u8GOPNum))
    {
        GOP_D_ERR("\n[%s] not support gop%d in this chip version!!",__FUNCTION__, u8GOPNum);
        return GOP_FAIL;
    }
    ret = HAL_GOP_GetGOPDst(&pGOPDrvLocalCtx->halCtxLocal, u8GOPNum, pGopDst);
    return ret;
}

/********************************************************************************/
/// Set stretch window property
/// @param u8GOP_num \b IN 0: GOP0  1:GOP1
/// @param eDstType \b IN \copydoc EN_GOP_DST_TYPE
/// @param x \b IN stretch window horizontal start position
/// @param y \b IN stretch window vertical start position
/// @param width \b IN stretch window width
/// @param height \b IN stretch window height
/********************************************************************************/
void MDrv_GOP_GWIN_SetStretchWin(MS_GOP_CTX_LOCAL*pGOPCtx, MS_U8 u8GOP,MS_U16 x, MS_U16 y, MS_U16 width, MS_U16 height)
{
	MS_U32 u32HStretchRatio;
	MS_U32 u32VStretchRatio;
	MS_U16 u16OutputValidH = 0, u16OutputValidV = 0;
	GOP_CTX_DRV_LOCAL*pGOPDrvLocalCtx = (GOP_CTX_DRV_LOCAL*)pGOPCtx;

	if ( (x > pGOPDrvLocalCtx->apiCtxLocal.pGOPCtxShared->u16PnlWidth[u8GOP])
	|| (y > pGOPDrvLocalCtx->apiCtxLocal.pGOPCtxShared->u16PnlHeight[u8GOP])) {
		APIGOP_ASSERT(FALSE, GOP_D_FATAL("[%s]invalid parameters:x= %u,y= %u,PNL width= %u,PNL height= %u!'\n",__FUNCTION__,
		x, y, pGOPDrvLocalCtx->apiCtxLocal.pGOPCtxShared->u16PnlWidth[u8GOP], pGOPDrvLocalCtx->apiCtxLocal.pGOPCtxShared->u16PnlHeight[u8GOP]));
	}

	HAL_GOP_SetGopStretchWin(&pGOPDrvLocalCtx->halCtxLocal, u8GOP, x, y, width, height);

	HAL_GOP_GetGopScaleRatio(&pGOPDrvLocalCtx->halCtxLocal, u8GOP, &u32HStretchRatio, &u32VStretchRatio);

	if(u32HStretchRatio == SCALING_MULITPLIER) {
#ifndef OUTPUT_VAILD_SIZE_PATCH
		u16OutputValidH = (x + width);
#else
		MS_U16 u16OutputWidth = 0;
		if((pGOPDrvLocalCtx->apiCtxLocal.pGOPCtxShared->u16HScaleSrc[u8GOP]) > (pGOPDrvLocalCtx->apiCtxLocal.pGOPCtxShared->u16HScaleDst[u8GOP])){
			u16OutputWidth = ALIGN_4(pGOPDrvLocalCtx->apiCtxLocal.pGOPCtxShared->u16PnlWidth[u8GOP]);
			u16OutputValidH = u16OutputWidth;
		} else {
			u16OutputValidH = GOP_OUTVALID_DEFAULT;
		}
#endif
	} else {
#ifndef OUTPUT_VAILD_SIZE_PATCH
		u16OutputValidH = (x + pGOPDrvLocalCtx->apiCtxLocal.pGOPCtxShared->u16HScaleDst[u8GOP]);
#else
		u16OutputValidH = GOP_OUTVALID_DEFAULT;
#endif
	}

	if(u32VStretchRatio == SCALING_MULITPLIER) {
#ifndef OUTPUT_VAILD_SIZE_PATCH
		u16OutputValidV = (y + height);
#else
		u16OutputValidV = GOP_OUTVALID_DEFAULT;
#endif
	} else {
#ifndef OUTPUT_VAILD_SIZE_PATCH
		u16OutputValidV = (y + pGOPDrvLocalCtx->apiCtxLocal.pGOPCtxShared->u16VScaleDst[u8GOP]);
#else
		u16OutputValidV = GOP_OUTVALID_DEFAULT;
#endif
	}
	HAL_GOP_SetGopVaildH(&pGOPDrvLocalCtx->halCtxLocal, u8GOP, u16OutputValidH);
	HAL_GOP_SetGopVaildV(&pGOPDrvLocalCtx->halCtxLocal, u8GOP, u16OutputValidV);

}

void MDrv_GOP_GWIN_SetHTotal(MS_GOP_CTX_LOCAL*pGOPCtx, MS_U8 u8GOP, MS_U16 width)
{
	GOP_CTX_DRV_LOCAL*pGOPDrvLocalCtx = (GOP_CTX_DRV_LOCAL*)pGOPCtx;

	HAL_GOP_SetHTotal(&pGOPDrvLocalCtx->halCtxLocal, u8GOP, width);
}

//Internal use only, get the real HW register setting on stretch win
//For API usage, please use g_pGOPCtxLocal->pGOPCtxShared->u16StretchWinWidth
void MDrv_GOP_GWIN_Get_StretchWin(MS_GOP_CTX_LOCAL*pGOPCtx, MS_U8 u8GOP,MS_U16 *x, MS_U16 *y, MS_U16 *width, MS_U16 *height)
{
#ifdef GOP_SUPPORT_PIXEL_SHIFT_MODE
    DRV_GOPDstType gopDst=E_DRV_GOP_DST_INVALID;
#endif

    GOP_CTX_DRV_LOCAL*pGOPDrvLocalCtx = (GOP_CTX_DRV_LOCAL*)pGOPCtx;

	HAL_GOP_GetGopStretchWin(&pGOPDrvLocalCtx->halCtxLocal, u8GOP, x, y, width, height);

#ifdef GOP_SUPPORT_PIXEL_SHIFT_MODE
    MDrv_GOP_GWIN_GetDstPlane(pGOPCtx, u8GOP, &gopDst);
    if ((gopDst == E_DRV_GOP_DST_OP0) || (gopDst == E_DRV_GOP_DST_FRC) || (gopDst == E_DRV_GOP_DST_OP_DUAL_RATE))
    {
        if ((pGOPCtx->pGOPCtxShared->s32GOPStretchWinHOffset > 0) && (*x >= pGOPCtx->pGOPCtxShared->s32GOPStretchWinHOffset))
        {
            *x = *x - pGOPCtx->pGOPCtxShared->s32GOPStretchWinHOffset;
        }
        if ((pGOPCtx->pGOPCtxShared->s32GOPStretchWinVOffset > 0) && (*y >= pGOPCtx->pGOPCtxShared->s32GOPStretchWinVOffset))
       {
            *y = *y - pGOPCtx->pGOPCtxShared->s32GOPStretchWinVOffset;
       }
    }
#endif

}

/********************************************************************************/
/// Set stretch window H-Stretch
/// @param bEnable \b IN
///   - # TRUE enable
///   - # FALSE disable
/// @param src \b IN original stretch size
/// @param dst \b IN stretch out size
/********************************************************************************/
void MDrv_GOP_GWIN_Set_HSCALE(MS_GOP_CTX_LOCAL*pGOPCtx,MS_U8 u8GOP, MS_BOOL bEnable, MS_U16 src, MS_U16 dst )
{
    GOP_CTX_DRV_LOCAL*pGOPDrvLocalCtx = (GOP_CTX_DRV_LOCAL*)pGOPCtx;
    MS_GOP_CTX_LOCAL apiCtxLocal = pGOPDrvLocalCtx->apiCtxLocal;
    MS_GOP_CTX_SHARED *pGOPCtxShared = apiCtxLocal.pGOPCtxShared;
    MS_U16 u16PnlWidth = pGOPCtxShared->u16PnlWidth[u8GOP];
    MS_U32 u32Hratio = 0, u32HOut = 0;


    if (_IsGopNumVaild(pGOPCtx, u8GOP) == FALSE)
    {
        GOP_D_WARN("[%s] not support gop%d in this chip version!!\n",__FUNCTION__,u8GOP);
        return;
    }

    u32HOut = dst;
    if (u16PnlWidth == GOP_8K_WIDTH)
    {
        if (bEnable == TRUE)
        {
            if ((dst / HVSP2_RATIO_DEFAULT) > src)
            {
                //step1 HVSP scaling to half size,
                //others size use step2 HVSP when 8k timing
                u32Hratio = (MS_U32)(src * SCALING_MULITPLIER) / (dst / HVSP2_RATIO_DEFAULT);
                u32HOut = (dst/HVSP2_RATIO_DEFAULT);
            }
            else
            {
                u32Hratio = 0;
                bEnable = FALSE;
            }
        }
        else
        {
            u32Hratio = 0;
        }
    }
    else
    {
        if (bEnable == TRUE && dst != 0)
            u32Hratio = (MS_U32)(src * SCALING_MULITPLIER) / dst;
        else
            u32Hratio = 0;
    }
    HAL_GOP_SetStep1HVSPHscale(&pGOPDrvLocalCtx->halCtxLocal, u8GOP, bEnable, u32Hratio, u32HOut);
}

/********************************************************************************/
/// Set stretch window V-Stretch
/// @param bEnable \b IN
///   - # TRUE enable
///   - # FALSE disable
/// @param src \b IN original stretch size
/// @param dst \b IN stretch out size
/********************************************************************************/
void MDrv_GOP_GWIN_Set_VSCALE(MS_GOP_CTX_LOCAL*pGOPCtx,MS_U8 u8GOP, MS_BOOL bEnable, MS_U16 src, MS_U16 dst )
{
    GOP_CTX_DRV_LOCAL*pGOPDrvLocalCtx = (GOP_CTX_DRV_LOCAL*)pGOPCtx;
    MS_GOP_CTX_LOCAL apiCtxLocal = pGOPDrvLocalCtx->apiCtxLocal;
    MS_GOP_CTX_SHARED *pGOPCtxShared = apiCtxLocal.pGOPCtxShared;
    MS_U16 u16PnlHeight = pGOPCtxShared->u16PnlHeight[u8GOP];
    MS_U32 u32Vratio = 0, u32VOut = 0;

    if (_IsGopNumVaild(pGOPCtx, u8GOP) == FALSE)
    {
        GOP_D_WARN("[%s] not support gop%d in this chip version!!\n",__FUNCTION__,u8GOP);
        return;
    }

    u32VOut = dst;
    if (u16PnlHeight == GOP_8K_HEIGHT)
    {
        if (bEnable == TRUE)
        {
            if ((dst / HVSP2_RATIO_DEFAULT) > src)
            {
                //step1 HVSP scaling to half size,
                //others size use step2 HVSP when 8k timing
                u32Vratio = (MS_U32)(src * SCALING_MULITPLIER) / (dst / HVSP2_RATIO_DEFAULT);
                u32VOut = (dst/HVSP2_RATIO_DEFAULT);
            }
            else
            {
                u32Vratio = 0;
                bEnable = FALSE;
            }
        }
        else
        {
            u32Vratio = 0;
        }
    }
    else
    {
        if (bEnable == TRUE && dst != 0)
            u32Vratio = (MS_U32)(src * SCALING_MULITPLIER) / dst;
        else
            u32Vratio = 0;
    }
    HAL_GOP_SetStep1HVSPVscale(&pGOPDrvLocalCtx->halCtxLocal, u8GOP, bEnable, u32Vratio, u32VOut);
}


//-------------------------------------------------------------------------------------------------
/// Set GOP H-Stretch Mode
/// @param HStrchMode \b IN \copydoc EN_GOP_STRETCH_HMODE
//-------------------------------------------------------------------------------------------------
void MDrv_GOP_GWIN_Set_HStretchMode(MS_GOP_CTX_LOCAL*pGOPCtx,MS_U8 u8GOP, DRV_GOPStrchHMode HStrchMode)
{
	GOP_CTX_DRV_LOCAL*pGOPDrvLocalCtx = (GOP_CTX_DRV_LOCAL*)pGOPCtx;

	HAL_GOP_Set_HStretchMode(&pGOPDrvLocalCtx->halCtxLocal, u8GOP, HStrchMode);
}


//-------------------------------------------------------------------------------------------------
/// Set GOP V-Stretch Mode
/// @param VStrchMode \b IN \copydoc EN_GOP_STRETCH_VMODE
//-------------------------------------------------------------------------------------------------
void MDrv_GOP_GWIN_Set_VStretchMode(MS_GOP_CTX_LOCAL*pGOPCtx,MS_U8 u8GOP, DRV_GOPStrchVMode VStrchMode)
{
	GOP_CTX_DRV_LOCAL*pGOPDrvLocalCtx = (GOP_CTX_DRV_LOCAL*)pGOPCtx;

	HAL_GOP_Set_VStretchMode(&pGOPDrvLocalCtx->halCtxLocal, u8GOP, VStrchMode);
}

#ifdef CONFIG_GOP_SCROLL
/********************************************************************************/
/// Set GWIN auto scrolling frame rate (0~63)
/// @param u8rate \b IN frame rate
/********************************************************************************/
void MDrv_GOP_GWIN_SetScrollRate(MS_GOP_CTX_LOCAL*pGOPCtx, MS_U8 u8rate)
{
    /*only gop0 has scrolling*/
    //no use
}


/********************************************************************************/
/// Set horizontal auto scroll
/// @param u8win   \b IN GWIN id 0 ~ (MAX_GWIN_SUPPORT-1)
/// @param type    \b IN @copydoc GOPSCROLLTYPE
/// @param bEnable \b IN
///   - # TRUE enable
///   - # FALSE disable
/********************************************************************************/
void MDrv_GOP_GWIN_SetHScroll(MS_GOP_CTX_LOCAL*pGOPCtx, MS_U8 u8win, E_GOP_SCROLL_TYPE type, MS_U32 offset, MS_BOOL bEnable)
{
    //no use
}

/********************************************************************************/
/// Set GWIN vertical auto scrolling
/// @param u8win \b IN GWIN id 0 ~ (MAX_GWIN_SUPPORT - 1)
/// @param type  \b IN \copydoc GOPSCROLLTYPE
/// @param bEnable \b IN
///   - # TRUE enable
///   - # FALSE disable
/********************************************************************************/
void MDrv_GOP_GWIN_SetVScroll(MS_GOP_CTX_LOCAL*pGOPCtx, MS_U8 u8win, E_GOP_SCROLL_TYPE type, MS_U32 offset, MS_BOOL bEnable)
{
    //no use
}


//-------------------------------------------------------------------------------------------------
/// Set Scroll auto stop
/// @param u8Wid    \b IN: GWin ID
/// @param bTrue    \b IN: TRUE: enable auto-stop mode, FALSE: disable auto-stop modea
//-------------------------------------------------------------------------------------------------
void MDrv_GOP_GWIN_Scroll_EnableAutoStop(MS_GOP_CTX_LOCAL*pGOPCtx, MS_U8 u8win, MS_BOOL bEnable)
{
    //no use
}


//-------------------------------------------------------------------------------------------------
/// Set Scroll auto stop Horizontal offset
/// @param u8Wid             \b IN: GWin ID
/// @param u32ScrollAutoHStop \b IN: scorll auto stop H offset (GWIN H scrolling will stop when moving count meet this offset)
/// @return TRUE: sucess / FALSE: fail
//-------------------------------------------------------------------------------------------------
MS_BOOL MDrv_GOP_GWIN_Scroll_AutoStop_HSet(MS_GOP_CTX_LOCAL*pGOPCtx, MS_U8 u8win, MS_U32 u32AutoHStopOffset)
{
	//no use
	return TRUE;
}


//-------------------------------------------------------------------------------------------------
/// Set Scroll auto stop Verticall offset
/// @param u8Wid             \b IN: GWin ID
/// @param u32AutoVStopOffset  \b IN: scorll auto stop V offset (GWIN V scrolling will stop when moving count meet this offset)
/// @return TRUE: sucess / FALSE: fail
//-------------------------------------------------------------------------------------------------
MS_BOOL MDrv_GOP_GWIN_Scroll_AutoStop_VSet(MS_GOP_CTX_LOCAL*pGOPCtx, MS_U8 u8win, MS_U32 u32AutoVStopOffset)
{
	//no use
	return TRUE;
}
#endif

/********************************************************************************/
/// Set GWIN alpha blending
/// @param u8win \b IN \copydoc GWINID
/// @param bEnable \b IN
///   - # TRUE enable alpha blending
///   - # FALSE disable alpha blending
/// @param u8coef \b IN alpha blending coefficient (0-7)
/********************************************************************************/
void MDrv_GOP_GWIN_SetBlending(MS_GOP_CTX_LOCAL*pGOPCtx, MS_U8 u8win, MS_BOOL bEnable, MS_U8 u8coef)
{
	DRV_GOP_CONSALPHA_BITS*	pVal;
	MS_U8 u8GOP = 0;

    GOP_CTX_DRV_LOCAL*pGOPDrvLocalCtx = (GOP_CTX_DRV_LOCAL*)pGOPCtx;

    if (FALSE== _IsGwinIdValid(pGOPCtx, u8win))
    {
        GOP_D_ERR("\n[%s] not support gwin id:%d in this chip version",__FUNCTION__, u8win);
        return;
    }
    pVal = pGOPCtx->pGopConsAlphaBits;
#ifndef MSOS_TYPE_NOS
    if(DRV_NONE_SETTING == *pVal)
    {
        *pVal = pGOPCtx->pGopChipProperty->Default_ConsAlpha_bits;
        GOP_D_DBUG("\n[%s] because User_ConsAlpha_bits is not set by user, so drv set it to the default bits enum(%d)!!!\n",__FUNCTION__,
                    pGOPCtx->pGopChipProperty->Default_ConsAlpha_bits);
     }
#endif
    pGOPDrvLocalCtx->halCtxLocal.User_ConsAlpha_bits = *pVal;
	u8GOP = MDrv_DumpGopByGwinId(pGOPCtx, u8win);
    HAL_GOP_GWIN_SetBlending(&pGOPDrvLocalCtx->halCtxLocal, u8GOP, u8win, bEnable, u8coef);
    GOP_GWIN_UpdateReg(pGOPDrvLocalCtx, (E_GOP_TYPE)MDrv_DumpGopByGwinId(pGOPCtx,u8win));
}

/********************************************************************************/
/// Set GWIN data format to GOP registers
/// @param u8win \b IN \copydoc GWINID
/// @param clrtype \b IN \copydoc EN_GOP_COLOR_TYPE
/********************************************************************************/
void MDrv_GOP_GWIN_SetWinFmt(MS_GOP_CTX_LOCAL*pGOPCtx, MS_U8 u8win, DRV_GOPColorType clrtype)
{
    MS_U8 u8GOP = 0;
    MS_U8 regform = 0;

    GOP_CTX_DRV_LOCAL*pGOPDrvLocalCtx = (GOP_CTX_DRV_LOCAL*)pGOPCtx;

    if (FALSE== _IsGwinIdValid(pGOPCtx, u8win))
    {
        GOP_D_ERR("\n[%s] not support gwin id:%d in this chip version",__FUNCTION__, u8win);
        return;
    }

    u8GOP = MDrv_DumpGopByGwinId(pGOPCtx,u8win);

    //  add GOP reg struct type.
    regform = MDrv_GOP_GetRegForm(pGOPCtx, PAR_IGNORE, u8win);

    HAL_GOP_SetWinFmt(&pGOPDrvLocalCtx->halCtxLocal, regform, u8GOP, u8win, GOP_SetFmt(clrtype)<<4);
}

/********************************************************************************/
/// Enable GWIN for display
/// @param u8win \b IN GWIN id
/// @param bEnable \b IN
///   - # TRUE Show GWIN
///   - # FALSE Hide GWIN
/********************************************************************************/
void MDrv_GOP_GWIN_EnableGwin(MS_GOP_CTX_LOCAL*pGOPCtx, MS_U8 u8win, MS_BOOL bEnable)
{
    MS_U8 u8GOP =0;
    GOP_CTX_DRV_LOCAL*pGOPDrvLocalCtx = (GOP_CTX_DRV_LOCAL*)pGOPCtx;

    if (FALSE== _IsGwinIdValid(pGOPCtx, u8win))
    {
        GOP_D_ERR("\n[%s] not support gwin id:%d in this chip version",__FUNCTION__, u8win);
        return;
    }
    GOP_D_INFO("MDrv_GOP_GWIN_Enable(gId=%d) == %d\n",u8win, bEnable);

    pGOPDrvLocalCtx->apiCtxLocal.pGOPCtxShared->bGWINEnable[u8win] = bEnable;
    u8GOP = MDrv_DumpGopByGwinId(pGOPCtx,u8win);
	HAL_GOP_SetGWIN_Enable(&pGOPDrvLocalCtx->halCtxLocal, u8GOP, u8win, bEnable);

    GOP_GWIN_UpdateReg(pGOPDrvLocalCtx, (E_GOP_TYPE)MDrv_DumpGopByGwinId(pGOPCtx,u8win));

}

static GOP_Result _GOPFBAddCheck(MS_GOP_CTX_LOCAL*pGOPCtx,MS_U8 gopnum,DRV_GOP_GWIN_INFO* pinfo)
{
    MS_U8 u8MiuSel;
    MS_PHY phyOffset = 0;

    _phy_to_miu_offset(u8MiuSel, phyOffset, pinfo->u64DRAMRBlkStart);
    pinfo->u64DRAMRBlkStart = phyOffset;
    if (FALSE==_IsGopNumVaild(pGOPCtx, gopnum))
    {
        GOP_D_ERR("\n[%s] not support gop id:%d in this chip version",__FUNCTION__, gopnum);
        return GOP_FAIL;
    }
	UNUSED(u8MiuSel);

    return GOP_SUCCESS;
}

GOP_Result MDrv_GOP_GWIN_SetGwinInfo(MS_GOP_CTX_LOCAL*pGOPCtx, MS_U8 u8win, DRV_GOP_GWIN_INFO* pinfo)
{
    GOP_CTX_DRV_LOCAL*pGOPDrvLocalCtx = (GOP_CTX_DRV_LOCAL*)pGOPCtx;
    MS_U8 u8GOP;
    GOP_OupputColor enGopOutColor;
    MS_BOOL bCscEnable = FALSE;

    if (FALSE== _IsGwinIdValid(pGOPCtx, u8win))
    {
        GOP_D_ERR("\n[%s] not support gwin id:%d in this chip version",__FUNCTION__, u8win);
        return GOP_FAIL;
    }
    u8GOP = MDrv_DumpGopByGwinId(pGOPCtx,u8win);

    if (FALSE== _IsGopNumVaild(pGOPCtx, u8GOP))
    {
        GOP_D_ERR("\n[%s] not support gop id:%d in this chip version",__FUNCTION__, u8GOP);
        return GOP_FAIL;
    }

    if(pGOPDrvLocalCtx->apiCtxLocal.pGopChipProperty->bUse3x3MartixCSC == TRUE)
    {
        bCscEnable = pGOPDrvLocalCtx->apiCtxLocal.pGOPCtxShared->stCSCParam[u8GOP].bCscEnable;
        if(pGOPDrvLocalCtx->apiCtxLocal.pGOPCtxShared->s32OutputColorType[u8GOP] == GOP_OUTPUTCOLOR_INVALID)
        {
            enGopOutColor = DRV_GOPOUT_RGB;
            MHal_GOP_GWIN_SetColorMatrix(&pGOPDrvLocalCtx->halCtxLocal, u8GOP, enGopOutColor, pinfo->clrType, bCscEnable);
        }
        else
        {
            enGopOutColor = (GOP_OupputColor) (pGOPDrvLocalCtx->apiCtxLocal.pGOPCtxShared->s32OutputColorType[u8GOP]);
            MHal_GOP_GWIN_SetColorMatrix(&pGOPDrvLocalCtx->halCtxLocal, u8GOP, enGopOutColor, pinfo->clrType, bCscEnable);
        }
    }

    switch(u8GOP)
    {
        case E_GOP0:
            if (GOP_SUCCESS ==_GOPFBAddCheck(pGOPCtx,0,pinfo))
                GOP_SetGop0GwinInfo(pGOPDrvLocalCtx, u8win, pinfo);
            else
                return GOP_FAIL;

            break;
        case E_GOP1:
            if (GOP_SUCCESS ==_GOPFBAddCheck(pGOPCtx,1,pinfo))
                GOP_SetGop1GwinInfo(pGOPDrvLocalCtx, u8win, pinfo);
            else
                return GOP_FAIL;

            break;
        case E_GOP2:
        case E_GOP3:
        case E_GOP4:
        case E_GOP5:
            if (GOP_SUCCESS ==_GOPFBAddCheck(pGOPCtx,u8GOP,pinfo))
                GOP_SetGopExtendGwinInfo(pGOPDrvLocalCtx, u8win, pinfo);
            else
                return GOP_FAIL;
            break;
        default:
            break;
    }

    return GOP_SUCCESS;
}

GOP_Result MDrv_GOP_GWIN_GetGwinInfo(MS_GOP_CTX_LOCAL*pGOPCtx, MS_U8 u8win, DRV_GOP_GWIN_INFO* pinfo)
{
    GOP_CTX_DRV_LOCAL*pGOPDrvLocalCtx = (GOP_CTX_DRV_LOCAL*)pGOPCtx;
    MS_U8 u8GOP;
    GOP_WinFB_INFO pwinFB;

    if (FALSE== _IsGwinIdValid(pGOPCtx, u8win))
    {
        GOP_D_ERR("\n[%s] not support gwin id:%d in this chip version",__FUNCTION__, u8win);
        return GOP_FAIL;
    }

    if(pGOPCtx->pGOPCtxShared->gwinMap[u8win].u32CurFBId > DRV_MAX_GWIN_FB_SUPPORT)
    {
        GOP_D_ERR("[%s][%d] WrongFBID=%td\n",__FUNCTION__,__LINE__,(ptrdiff_t)pGOPCtx->pGOPCtxShared->gwinMap[u8win].u32CurFBId);
        return GOP_FAIL;
    }

    #if WINFB_INSHARED
            pwinFB = pGOPCtx->pGOPCtxShared->winFB[pGOPCtx->pGOPCtxShared->gwinMap[u8win].u32CurFBId];
    #else
            pwinFB = pGOPCtx->winFB[pGOPCtx->pGOPCtxShared->gwinMap[u8win].u32CurFBId];
    #endif

    pinfo->u16RBlkHPixSize = pwinFB.width;
    pinfo->u16RBlkVPixSize = pwinFB.height;

    u8GOP = MDrv_DumpGopByGwinId(pGOPCtx,u8win);
    switch(u8GOP)
    {
        case E_GOP0:
            GOP_ReadGop0GwinInfo(pGOPDrvLocalCtx, u8win,  pinfo);
            HAL_ConvertAPIAddr(&pGOPDrvLocalCtx->halCtxLocal, u8win, &(pinfo->u64DRAMRBlkStart));
            break;
        case E_GOP1:
            GOP_ReadGop1GwinInfo(pGOPDrvLocalCtx, u8win,  pinfo);
            HAL_ConvertAPIAddr(&pGOPDrvLocalCtx->halCtxLocal, u8win, &(pinfo->u64DRAMRBlkStart));
            break;
        case E_GOP2:
        case E_GOP3:
        case E_GOP4:
        case E_GOP5:
            GOP_ReadGopExtendGwinInfo(pGOPDrvLocalCtx, u8win,  pinfo);
            HAL_ConvertAPIAddr(&pGOPDrvLocalCtx->halCtxLocal, u8win, &(pinfo->u64DRAMRBlkStart));
            break;
        default:
            break;
    }

    return GOP_SUCCESS;
}

void MDrv_GOP_GWIN_IsGWINEnabled(MS_GOP_CTX_LOCAL*pGOPCtx, MS_U8 u8win, MS_BOOL* pbEnable )
{
    MS_U8 u8GOP;
    GOP_CTX_DRV_LOCAL*pGOPDrvLocalCtx = (GOP_CTX_DRV_LOCAL*)pGOPCtx;

    if (FALSE== _IsGwinIdValid(pGOPCtx, u8win))
    {
        GOP_D_ERR("\n[%s] not support gwin id:%d in this chip version",__FUNCTION__, u8win);
        return;
    }
    u8GOP = MDrv_DumpGopByGwinId(pGOPCtx,u8win);

	*pbEnable = HAL_GOP_GetGWINEnable(&pGOPDrvLocalCtx->halCtxLocal, u8GOP, u8win);
}

void MDrv_GOP_IsGOPMirrorEnable(MS_GOP_CTX_LOCAL*pGOPCtx, MS_U8 u8GOPNum, MS_BOOL *bHMirror, MS_BOOL *bVMirror)
{
	GOP_CTX_DRV_LOCAL*pGOPDrvLocalCtx = (GOP_CTX_DRV_LOCAL*)pGOPCtx;

	if (FALSE== _IsGopNumVaild((MS_GOP_CTX_LOCAL*)pGOPDrvLocalCtx, u8GOPNum)) {
		GOP_D_ERR("\n[%s] not support gop%d in this chip version!!",__FUNCTION__, u8GOPNum);
		return;
	}

	*bHMirror = HAL_GOP_GetHMirror(&pGOPDrvLocalCtx->halCtxLocal, u8GOPNum);
	*bVMirror = HAL_GOP_GetVMirror(&pGOPDrvLocalCtx->halCtxLocal, u8GOPNum);
}

void MDrv_GOP_GWIN_EnableHMirror(MS_GOP_CTX_LOCAL*pGOPCtx, MS_U8 u8GOPNum, MS_BOOL bEnable)
{
	GOP_CTX_DRV_LOCAL*pGOPDrvLocalCtx = (GOP_CTX_DRV_LOCAL*)pGOPCtx;

	if (FALSE== _IsGopNumVaild((MS_GOP_CTX_LOCAL*)pGOPDrvLocalCtx, u8GOPNum)){
		GOP_D_ERR("\n[%s] not support gop%d in this chip version!!",__FUNCTION__, u8GOPNum);
		return;
	}

	HAL_GOP_GWIN_SetHMirror(&pGOPDrvLocalCtx->halCtxLocal, u8GOPNum, bEnable);
}


void MDrv_GOP_GWIN_EnableVMirror(MS_GOP_CTX_LOCAL*pGOPCtx, MS_U8 u8GOPNum, MS_BOOL bEnable)
{
	GOP_CTX_DRV_LOCAL*pGOPDrvLocalCtx = (GOP_CTX_DRV_LOCAL*)pGOPCtx;

	if (FALSE== _IsGopNumVaild((MS_GOP_CTX_LOCAL*)pGOPDrvLocalCtx, u8GOPNum)) {
		GOP_D_ERR("\n[%s] not support gop%d in this chip version!!",__FUNCTION__, u8GOPNum);
		return;
	}

	HAL_GOP_SetVMirror(&pGOPDrvLocalCtx->halCtxLocal, u8GOPNum, bEnable);
}

GOP_Result MDrv_GOP_ConvertAPIAddr2HAL(MS_GOP_CTX_LOCAL*pGOPCtx, MS_U8 u8Gwinid, MS_PHY* u64ApiAdr)
{
    GOP_CTX_DRV_LOCAL*pGOPDrvLocalCtx = (GOP_CTX_DRV_LOCAL*)pGOPCtx;
    return HAL_ConvertAPIAddr(&pGOPDrvLocalCtx->halCtxLocal, u8Gwinid, u64ApiAdr);
}

/******************************************************************************/
/// Set Scaler VOP New blending level
/******************************************************************************/
void MDrv_GOP_SetVOPNBL(MS_GOP_CTX_LOCAL*pGOPCtx,MS_BOOL bEnable)
{
	GOP_CTX_DRV_LOCAL*pGOPDrvLocalCtx = (GOP_CTX_DRV_LOCAL*)pGOPCtx;

	HAL_GOP_SetVOPNBL(&pGOPDrvLocalCtx->halCtxLocal, bEnable);
}

void MDrv_GOP_GWIN_UpdateReg(MS_GOP_CTX_LOCAL*pGOPCtx, MS_U8 u8GOP)
{
    GOP_CTX_DRV_LOCAL*pGOPDrvLocalCtx = (GOP_CTX_DRV_LOCAL*)pGOPCtx;
    GOP_GWIN_UpdateReg(pGOPDrvLocalCtx, (E_GOP_TYPE)u8GOP);
}

void MDrv_GOP_GWIN_UpdateRegWithSync(MS_GOP_CTX_LOCAL*pGOPCtx, MS_U8 u8GOP, MS_BOOL bSync)
{
    GOP_CTX_DRV_LOCAL*pGOPDrvLocalCtx = (GOP_CTX_DRV_LOCAL*)pGOPCtx;
    if(MDrv_GOP_GWIN_IsForceWrite(pGOPCtx, u8GOP) == TRUE)
    {
        GOP_GWIN_TriggerRegWriteIn(pGOPDrvLocalCtx, (E_GOP_TYPE)u8GOP, TRUE, bSync);
    }
    else
    {
        GOP_GWIN_TriggerRegWriteIn(pGOPDrvLocalCtx, (E_GOP_TYPE)u8GOP, FALSE, bSync);
    }

}

void MDrv_GOP_TriggerRegWriteIn(MS_GOP_CTX_LOCAL*pGOPCtx, MS_U8 u8GOP, MS_BOOL bForceWriteIn, MS_BOOL bSync)
{
    GOP_CTX_DRV_LOCAL*pGOPDrvLocalCtx = (GOP_CTX_DRV_LOCAL*)pGOPCtx;
    GOP_GWIN_TriggerRegWriteIn(pGOPDrvLocalCtx, (E_GOP_TYPE)u8GOP, bForceWriteIn, bSync);
}

void MDrv_GOP_GWIN_UpdateRegWithMaskSync(MS_GOP_CTX_LOCAL*pGOPCtx, MS_U16 u16GopMask, MS_BOOL bSync)
{
	GOP_CTX_DRV_LOCAL*pGOPDrvLocalCtx = (GOP_CTX_DRV_LOCAL*)pGOPCtx;
	MS_U16 u16GopAck=0;
	MS_U32 goptimeout = 0;
	MS_U16 u16GopIdx = 0;

	for (u16GopIdx = 0; u16GopIdx < gGopChipPro.TotalGOPNum; u16GopIdx++) {
		if ((u16GopMask & (1<<u16GopIdx)) != 0)
			break;
	}
	if (u16GopIdx == gGopChipPro.TotalGOPNum)
		return;

	if(pGOPDrvLocalCtx->gop_gwin_frwr) {
		HAL_GOP_GWIN_TriggerRegWriteIn(&pGOPDrvLocalCtx->halCtxLocal, u16GopIdx, TRUE, E_DRV_GOP_FORCEWRITE);
		HAL_GOP_GWIN_TriggerRegWriteIn(&pGOPDrvLocalCtx->halCtxLocal, u16GopIdx, FALSE, E_DRV_GOP_FORCEWRITE);
	} else {
		HAL_GOP_SetGOPACKMask(&pGOPDrvLocalCtx->halCtxLocal, u16GopMask);

		for (u16GopIdx = 0; u16GopIdx < gGopChipPro.TotalGOPNum; u16GopIdx++) {
			if ((u16GopMask & (1<<u16GopIdx))!=0)
				break;
		}
		if (u16GopIdx == gGopChipPro.TotalGOPNum)
			return;


		if (bSync == TRUE) {
			MS_U32 u32DelayTimems = 0;
			MS_U32 u32TimeoutCnt = 0;
			_GetGOPAckDelayTimeAndCnt(&u32DelayTimems, &u32TimeoutCnt);
			do {
				goptimeout++;
				u16GopAck = HAL_GOP_GetGOPACK(&pGOPDrvLocalCtx->halCtxLocal,u16GopIdx);
				if (0 != u32DelayTimems)
					MsOS_DelayTask(u32DelayTimems); //delay 1 ms
			} while ((!u16GopAck) && (goptimeout <= u32TimeoutCnt));


			// Perform force write if wr timeout.
			if (goptimeout > u32TimeoutCnt) {
				//printf("Perform fwr if wr timeout!!\n");
				HAL_GOP_GWIN_TriggerRegWriteIn(&pGOPDrvLocalCtx->halCtxLocal, u16GopIdx, TRUE, E_DRV_GOP_FORCEWRITE);
				HAL_GOP_GWIN_TriggerRegWriteIn(&pGOPDrvLocalCtx->halCtxLocal, u16GopIdx, FALSE, E_DRV_GOP_FORCEWRITE);
			}
		}
	}
}

void MDrv_GOP_Restore_Ctx(MS_GOP_CTX_LOCAL*pGOPCtx)
{

}

MS_U8 MDrv_GOP_GetWordUnit(MS_GOP_CTX_LOCAL*pGOPCtx, MS_U8 u8GOPNum)
{
    MS_U16 u16GOP_Unit=0;

    if(pGOPCtx->pGOPCtxShared->bPixelMode[u8GOPNum])
    {
        u16GOP_Unit = 1;
    }
    else
    {
        u16GOP_Unit = GOP_WordUnit;
    }
    return u16GOP_Unit;
}

GOP_Result MDrv_GOP_SetClock(MS_GOP_CTX_LOCAL*pGOPCtx,MS_BOOL bEnable)
{
    GOP_CTX_DRV_LOCAL*pGOPDrvLocalCtx = (GOP_CTX_DRV_LOCAL*)pGOPCtx;
    return HAL_GOP_SetClock(&pGOPDrvLocalCtx->halCtxLocal,bEnable);
}
MS_PHY MDrv_GOP_GetMIULen(MS_GOP_CTX_LOCAL*pGOPCtx)
{
    return GOP_MIU0_LENGTH;
}

//-------------------------------------------------------------------------------------------------
/// Set GOP Hsync Pipeline Delay Offset
//-------------------------------------------------------------------------------------------------
MS_U16 MDrv_GOP_GetHPipeOfst(MS_GOP_CTX_LOCAL*pGOPCtx, MS_U8 u8GOP_num, DRV_GOPDstType GopDst)
{
    MS_U8 u8Gop=0, i=0;
    MS_U16 u16Offset=0;
    MS_BOOL bHDREnable=FALSE;
    MS_U8 u8mux_sel=0;
    GOP_CTX_DRV_LOCAL*pGOPDrvLocalCtx = (GOP_CTX_DRV_LOCAL*)pGOPCtx;
    if (!_IsGopNumVaild(pGOPCtx, u8GOP_num))
    {
        GOP_D_ERR("\n[%s] not support gop id:%d in this chip version",__FUNCTION__, u8GOP_num);
        return GOP_FAIL;
    }

    switch(GopDst)
    {
      case E_DRV_GOP_DST_FRC:
      case E_DRV_GOP_DST_OP1:
      case E_DRV_GOP_DST_VE:
            break;
      case E_DRV_GOP_DST_BYPASS:
        for (i=E_GOP_FRC_MUX0; i<=E_GOP_FRC_MUX3; i++)
        {
            MDrv_GOP_GWIN_GetMux(pGOPCtx, &u8Gop, (Gop_MuxSel)i);
            if (u8Gop==u8GOP_num)
            {
                u16Offset = i/E_GOP_FRC_MUX2* pGOPDrvLocalCtx->halCtxLocal.pGopChipPro->GOP_MUX_Delta;
                break;
            }
        }
        break;

      case E_DRV_GOP_DST_OP_DUAL_RATE:
        for (i=E_GOP_DUALRATE_OP_MUX0; i<=E_GOP_DUALRATE_OP_MUX2; i++)
        {
            MDrv_GOP_GWIN_GetMux(pGOPCtx, &u8Gop, (Gop_MuxSel)i);
            if (u8Gop==u8GOP_num)
            {
                u16Offset = pGOPDrvLocalCtx->halCtxLocal.pGopChipPro->GOP_DualRateMux_Offset[(i-E_GOP_DUALRATE_OP_MUX0)];
                break;
            }
        }
        break;

      case E_DRV_GOP_DST_OP0:
      {
          MDrv_GOP_IsHDREnabled(pGOPCtx, &bHDREnable);
          if(bHDREnable==FALSE)
          {
              for (i=0; i<MAX_GOP_MUX; i++)
              {
                  MDrv_GOP_GWIN_GetMux(pGOPCtx, &u8Gop, (Gop_MuxSel)i);
                  if (u8Gop==u8GOP_num)
                  {
                      u16Offset = pGOPDrvLocalCtx->halCtxLocal.pGopChipPro->GOP_Mux_Offset[i]* pGOPDrvLocalCtx->halCtxLocal.pGopChipPro->GOP_MUX_Delta;
                      break;
                  }
              }
          }
          else
          {
              for (i=0; i<MAX_GOP_MUX; i++)
              {
                  MDrv_GOP_GWIN_GetMux(pGOPCtx, &u8mux_sel, (Gop_MuxSel)i);
                  if (u8mux_sel==GOP_MIXER_MUX)
                  {
                      u16Offset = pGOPDrvLocalCtx->halCtxLocal.pGopChipPro->GOP_Mux_Offset[i]* pGOPDrvLocalCtx->halCtxLocal.pGopChipPro->GOP_MUX_Delta;
                      break;
                  }
              }
          }
          break;
      }
      default:
        for (i=0; i<MAX_GOP_MUX; i++)
        {
            MDrv_GOP_GWIN_GetMux(pGOPCtx, &u8Gop, (Gop_MuxSel)i);
            if (u8Gop==u8GOP_num)
            {
                u16Offset = pGOPDrvLocalCtx->halCtxLocal.pGopChipPro->GOP_Mux_Offset[i]* pGOPDrvLocalCtx->halCtxLocal.pGopChipPro->GOP_MUX_Delta;
                break;
            }
        }
        break;
   }

    if ((!pGOPDrvLocalCtx->halCtxLocal.pGopChipPro->bGop2VStretch) && u8GOP_num==2)
    {
        /*if gop2 no v stretch, gop2 will have 2 level HSPD*/
        u16Offset+=5;
    }
    return u16Offset;
}

GOP_Result MDrv_GOP_MIXER_SetOutputTiming(MS_GOP_CTX_LOCAL*pGOPCtx, MS_U32 u32mode, GOP_DRV_MixerTiming *pTM)
{
    GOP_CTX_DRV_LOCAL*pGOPDrvLocalCtx = (GOP_CTX_DRV_LOCAL*)pGOPCtx;

    if(u32mode >= E_GOP_MIXER_TIMEING_MAX)
        return GOP_INVALID_PARAMETERS;

    HAL_GOP_MIXER_SetOutputTiming(&pGOPDrvLocalCtx->halCtxLocal, u32mode, pTM);
    return GOP_SUCCESS;
}

GOP_Result MDrv_GOP_MIXER_EnableVfilter(MS_GOP_CTX_LOCAL*pGOPCtx, MS_BOOL bEnable)
{
    GOP_CTX_DRV_LOCAL*pGOPDrvLocalCtx = (GOP_CTX_DRV_LOCAL*)pGOPCtx;
    HAL_GOP_MIXER_EnableVfilter(&pGOPDrvLocalCtx->halCtxLocal, bEnable);
    return GOP_SUCCESS;
}

MS_U8 MDrv_GOP_SelGwinIdByGOP(MS_GOP_CTX_LOCAL*pGOPCtx, MS_U8 u8GOP, MS_U8 u8GWinIdx)
{
    GOP_CTX_DRV_LOCAL*pGOPDrvLocalCtx = (GOP_CTX_DRV_LOCAL*)pGOPCtx;

    return HAL_GOP_SelGwinIdByGOP(&pGOPDrvLocalCtx->halCtxLocal, u8GOP, u8GWinIdx);
}

GOP_Result MDrv_GOP_SetGOPBrightness(MS_GOP_CTX_LOCAL*pGOPCtx, MS_U8 u8GOP, MS_U16 u16BriVal,MS_BOOL bMSB)
{
	GOP_CTX_DRV_LOCAL*pGOPDrvLocalCtx = (GOP_CTX_DRV_LOCAL*)pGOPCtx;

	HAL_GOP_SetBrightness(&pGOPDrvLocalCtx->halCtxLocal, u8GOP, u16BriVal, bMSB);

	GOP_GWIN_UpdateReg(pGOPDrvLocalCtx, (E_GOP_TYPE)u8GOP);

	return GOP_SUCCESS;
}

GOP_Result MDrv_GOP_GetGOPBrightness(MS_GOP_CTX_LOCAL*pGOPCtx, MS_U8 u8GOP,MS_U16* u16BriVal,MS_BOOL* bMSB)
{
	GOP_CTX_DRV_LOCAL*pGOPDrvLocalCtx = (GOP_CTX_DRV_LOCAL*)pGOPCtx;

	HAL_GOP_GetBrightness(&pGOPDrvLocalCtx->halCtxLocal, u8GOP, u16BriVal, bMSB);

	return GOP_SUCCESS;
}

void MDrv_GOP_GWIN_Load_HStretchModeTable(MS_GOP_CTX_LOCAL*pGOPCtx,MS_U8 u8GOP,DRV_GOPStrchHMode HStrchMode)
{
    GOP_CTX_DRV_LOCAL*pGOPDrvLocalCtx = (GOP_CTX_DRV_LOCAL*)pGOPCtx;

    switch(HStrchMode)
    {
        case E_DRV_GOP_HSTRCH_6TAPE:
        case E_DRV_GOP_HSTRCH_6TAPE_LINEAR:
        case E_DRV_GOP_HSTRCH_6TAPE_NEAREST:
        case E_DRV_GOP_HSTRCH_6TAPE_GAIN0:
        case E_DRV_GOP_HSTRCH_6TAPE_GAIN1:
        case E_DRV_GOP_HSTRCH_6TAPE_GAIN2:
        case E_DRV_GOP_HSTRCH_6TAPE_GAIN3:
        case E_DRV_GOP_HSTRCH_6TAPE_GAIN4:
        case E_DRV_GOP_HSTRCH_6TAPE_GAIN5:
        case E_DRV_GOP_HSTRCH_2TAPE:
        case E_DRV_GOP_HSTRCH_DUPLICATE:
        case E_DRV_GOP_HSTRCH_4TAPE:
            break;
        case E_DRV_GOP_HSTRCH_NEW4TAP_45:
        case E_DRV_GOP_HSTRCH_NEW4TAP_50:
        case E_DRV_GOP_HSTRCH_NEW4TAP_55:
        case E_DRV_GOP_HSTRCH_NEW4TAP_65:
        case E_DRV_GOP_HSTRCH_NEW4TAP_75:
        case E_DRV_GOP_HSTRCH_NEW4TAP_85:
        case E_DRV_GOP_HSTRCH_NEW4TAP_95:
        case E_DRV_GOP_HSTRCH_NEW4TAP_100:
        case E_DRV_GOP_HSTRCH_NEW4TAP_105:
        case E_DRV_GOP_HSTRCH_NEW4TAP_105_GAIN16:
        case E_DRV_GOP_HSTRCH_NEW4TAP_105_GAIN17:
        case E_DRV_GOP_HSTRCH_NEW4TAP_105_GAIN22:
        case E_DRV_GOP_HSTRCH_NEW4TAP_105_GAIN31:
            if(MHAL_GOP_Load_HStretch_New4TapMode_Table(&pGOPDrvLocalCtx->halCtxLocal, u8GOP, _GopHStretchTable[HStrchMode]) != GOP_SUCCESS)
            {
                GOP_D_ERR("[%s] MHAL_GOP_Load_HStretch_New4TapMode_Table fail\n", __FUNCTION__);
            }
            break;
        default:
            break;

    }


}

void MDrv_GOP_GWIN_Load_VStretchModeTable(MS_GOP_CTX_LOCAL*pGOPCtx,MS_U8 u8GOP, DRV_GOPStrchVMode VStrchMode)
{
    GOP_CTX_DRV_LOCAL*pGOPDrvLocalCtx = (GOP_CTX_DRV_LOCAL*)pGOPCtx;

    switch(VStrchMode)
    {
        case E_DRV_GOP_VSTRCH_LINEAR:
        case E_DRV_GOP_VSTRCH_LINEAR_GAIN0:
        case E_DRV_GOP_VSTRCH_LINEAR_GAIN1:
        case E_DRV_GOP_VSTRCH_DUPLICATE:
        case E_DRV_GOP_VSTRCH_NEAREST:
            break;
        case E_DRV_GOP_VSTRCH_4TAP:
        case E_DRV_GOP_HSTRCH_V4TAP_100:
        case E_DRV_GOP_VSTRCH_V4TAP_105_GAIN16:
        case E_DRV_GOP_VSTRCH_V4TAP_105_GAIN17:
        case E_DRV_GOP_VSTRCH_V4TAP_105_GAIN22:
        case E_DRV_GOP_VSTRCH_V4TAP_105_GAIN31:
            if(MHAL_GOP_Load_VStretch_V4TapMode_Table(&pGOPDrvLocalCtx->halCtxLocal,u8GOP,_GopVStretchTable[VStrchMode]) != GOP_SUCCESS)
            {
                GOP_D_ERR("[%s] MHAL_GOP_Load_VStretch_V4TapMode_Table fail\n", __FUNCTION__);
            }
            break;
        default:
            break;

    }
}
GOP_Result MDrv_GOP_Set_PINPON(MS_GOP_CTX_LOCAL*pGOPCtx, MS_U8 u8GOPNum,MS_BOOL bEn, E_DRV_GOP_PINPON_MODE pinpon_mode)
{
    GOP_CTX_DRV_LOCAL*pGOPDrvLocalCtx = (GOP_CTX_DRV_LOCAL*)pGOPCtx;

    HAL_GOP_Set_PINPON(&pGOPDrvLocalCtx->halCtxLocal, u8GOPNum, bEn, pinpon_mode);

    return GOP_SUCCESS;
}

GOP_Result MDrv_GOP_GWIN_SetGPUTileMode(MS_GOP_CTX_LOCAL*pGOPCtx, MS_U8 u8win, EN_DRV_GOP_GPU_TILE_MODE mode)
{
    GOP_CTX_DRV_LOCAL*pGOPDrvLocalCtx = (GOP_CTX_DRV_LOCAL*)pGOPCtx;

    HAL_GOP_GWIN_SetGPUTileMode(&pGOPDrvLocalCtx->halCtxLocal, u8win, (EN_DRV_GOP_GPU_TILE_MODE)mode);
    GOP_GWIN_UpdateReg(pGOPDrvLocalCtx, (E_GOP_TYPE)MDrv_DumpGopByGwinId(pGOPCtx,u8win));
    return GOP_SUCCESS;
}

#ifdef CONFIG_GOP_YUV_SWAP

GOP_Result MDrv_GOP_SetUVSwap(MS_GOP_CTX_LOCAL*pGOPCtx, MS_U8 u8GOPNum,MS_BOOL bEn)
{
    GOP_CTX_DRV_LOCAL*pGOPDrvLocalCtx = (GOP_CTX_DRV_LOCAL*)pGOPCtx;

    HAL_GOP_SetUVSwap(&pGOPDrvLocalCtx->halCtxLocal,u8GOPNum,bEn);

    return GOP_SUCCESS;
}

GOP_Result MDrv_GOP_SetYCSwap(MS_GOP_CTX_LOCAL*pGOPCtx, MS_U8 u8GOPNum,MS_BOOL bEn)
{
    GOP_CTX_DRV_LOCAL*pGOPDrvLocalCtx = (GOP_CTX_DRV_LOCAL*)pGOPCtx;

    HAL_GOP_SetYCSwap(&pGOPDrvLocalCtx->halCtxLocal,u8GOPNum,bEn);

    return GOP_SUCCESS;
}
#endif
//-------------------------------------------------------------------------------------------------
/// Set GWIN New Alpha Mode
/// @param u8Wid    \b IN: GWin ID
/// @param bTrue    \b IN: TRUE: enable new alpha mode, FALSE: disable new alpha mode
//-------------------------------------------------------------------------------------------------
#ifdef CONFIG_GOP_GWIN_MISC
GOP_Result MDrv_GOP_GWIN_GetGwinNewAlphaModeEnable(MS_GOP_CTX_LOCAL*pGOPCtx, MS_U8 u8win, MS_BOOL* pEnable)
{
    GOP_CTX_DRV_LOCAL*pGOPDrvLocalCtx = (GOP_CTX_DRV_LOCAL*)pGOPCtx;

    HAL_GOP_GWIN_GetNewAlphaMode(&pGOPDrvLocalCtx->halCtxLocal, u8win, pEnable);
    return GOP_SUCCESS;
}

GOP_Result MDrv_GOP_GWIN_SetNewAlphaMode(MS_GOP_CTX_LOCAL*pGOPCtx, MS_U8 u8win, MS_BOOL bEnable)
{
    GOP_CTX_DRV_LOCAL*pGOPDrvLocalCtx = (GOP_CTX_DRV_LOCAL*)pGOPCtx;

    HAL_GOP_GWIN_SetNewAlphaMode(&pGOPDrvLocalCtx->halCtxLocal, u8win, bEnable);

    GOP_GWIN_UpdateReg(pGOPDrvLocalCtx, (E_GOP_TYPE)MDrv_DumpGopByGwinId(pGOPCtx,u8win));

    return GOP_SUCCESS;
}
#endif

#ifdef CONFIG_GOP_CONTRAST
GOP_Result MDrv_GOP_SetGOPContrast(MS_GOP_CTX_LOCAL* pGOPCtx, MS_U8 u8GOP
    , MS_U16 u16YContrast, MS_U16 u16UContrast, MS_U16 u16VContrast )
{
    GOP_CTX_DRV_LOCAL* pGOPDrvLocalCtx = (GOP_CTX_DRV_LOCAL*)pGOPCtx;


    if(u8GOP >= gGopChipPro.TotalGOPNum)
    {
        GOP_D_ERR("\n[%s] not support gop num:%d in this chip version",__FUNCTION__, u8GOP);
        return GOP_FAIL;
    }

    if(pGOPDrvLocalCtx->halCtxLocal.pGopChipPro->bSupportContrast == FALSE)
    {
        if(pGOPDrvLocalCtx->halCtxLocal.pGopChipPro->bSupportCSCTuning == TRUE && pGOPCtx->pGOPCtxShared->stCSCParam[u8GOP].bCscEnable == TRUE)
        {
            GOP_D_ERR("[%s] not support GOP Contrast in this chip, use CSC for compatibility \n",__FUNCTION__);
            ST_GOP_CSC_PARAM stCSCParam;
            MS_U16 u16GainIdx = 0;
            memset(&stCSCParam, 0 , sizeof(ST_GOP_CSC_PARAM));
            memcpy(&stCSCParam,&(pGOPCtx->pGOPCtxShared->stCSCParam[u8GOP]),sizeof(ST_GOP_CSC_PARAM));

            //CscGainValue = GopContrast*64 (Max=2047)
            stCSCParam.u16RGBGGain[0]=u16VContrast*64;
            stCSCParam.u16RGBGGain[1]=u16YContrast*64;
            stCSCParam.u16RGBGGain[2]=u16UContrast*64;
            for(u16GainIdx=0 ; u16GainIdx<3; u16GainIdx++)
            {
                if(stCSCParam.u16RGBGGain[u16GainIdx] > MAX_CSC_GAIN)
                {
                    stCSCParam.u16RGBGGain[u16GainIdx] = MAX_CSC_GAIN;
                }
            }

            MDrv_GOP_CSC_Tuning(pGOPCtx, (MS_U32)u8GOP, &stCSCParam);
            GOP_GWIN_UpdateReg(pGOPDrvLocalCtx, (E_GOP_TYPE)u8GOP);
            return GOP_SUCCESS ;
        }
        else
        {
            GOP_D_ERR("[%s] not support GOP Contrast in this chip\n",__FUNCTION__);
            return GOP_FUN_NOT_SUPPORTED;
        }
    }

    if ( ((u16YContrast) > 0x3F) | ((u16UContrast) > 0x3F) | ((u16VContrast) > 0x3F) ) // Y, U, V = 6, 6, 6 bits each
    {
        return GOP_FAIL ;
    }

    GOP_GWIN_UpdateReg(pGOPDrvLocalCtx, (E_GOP_TYPE)u8GOP);

    return GOP_SUCCESS;
}

GOP_Result MDrv_GOP_GetGOPContrast(MS_GOP_CTX_LOCAL* pGOPCtx, MS_U8 u8GOP
    , MS_U32* u32YContrast, MS_U32* u32UContrast, MS_U32* u32VContrast )
{
    MS_U32 u32BankOffSet =0;
    GOP_CTX_DRV_LOCAL* pGOPDrvLocalCtx = (GOP_CTX_DRV_LOCAL*)pGOPCtx;

    _GetBnkOfstByGop(u8GOP, &u32BankOffSet);

    if(u8GOP >= gGopChipPro.TotalGOPNum)
    {
        GOP_D_ERR("\n[%s] not support gop num:%d in this chip version",__FUNCTION__, u8GOP);
        return GOP_FAIL;
    }

    if(pGOPDrvLocalCtx->halCtxLocal.pGopChipPro->bSupportContrast == FALSE)
    {
        if(pGOPDrvLocalCtx->halCtxLocal.pGopChipPro->bSupportCSCTuning == TRUE)
        {
            GOP_D_ERR("[%s] not support GOP Contrast in this chip, use CSC for compatibility \n",__FUNCTION__);
            //CscGainValue = GopContrast*64
            *u32YContrast = (pGOPCtx->pGOPCtxShared->stCSCParam[u8GOP].u16RGBGGain[1])/64;
            *u32UContrast = (pGOPCtx->pGOPCtxShared->stCSCParam[u8GOP].u16RGBGGain[2])/64;
            *u32VContrast = (pGOPCtx->pGOPCtxShared->stCSCParam[u8GOP].u16RGBGGain[0])/64;
            return GOP_SUCCESS;
        }
        else
        {
            GOP_D_ERR("[%s] not support GOP Contrast in this chip\n",__FUNCTION__);
            return GOP_FUN_NOT_SUPPORTED;
        }
    }

    return GOP_SUCCESS;
}
#endif

GOP_Result MDrv_GOP_Set3D_LR_FrameExchange(MS_GOP_CTX_LOCAL* pGOPCtx, MS_U8 u8GOP)
{
    MS_U32 u32BankOffSet =0;
    MS_PHY phyMainBase=0, phySubBase=0;
    MS_U8 u8Win = 0, u8MaxWinNum=0;
    GOP_CTX_DRV_LOCAL* pGOPDrvLocalCtx = (GOP_CTX_DRV_LOCAL*)pGOPCtx;
    if(pGOPDrvLocalCtx->halCtxLocal.pGopChipPro->enGOP3DType == E_DRV_3D_NONE)
    {
        return GOP_FUN_NOT_SUPPORTED;
    }

    if(pGOPDrvLocalCtx->halCtxLocal.pGopChipPro->enGOP3DType != E_DRV_3D_NONE)
    {
        _GetBnkOfstByGop(u8GOP, &u32BankOffSet);
        u8MaxWinNum = HAL_GOP_GetMaxGwinNumByGOP(&pGOPDrvLocalCtx->halCtxLocal, u8GOP);

        for(u8Win=0; u8Win<u8MaxWinNum; u8Win++)
        {
            //Read back main/sub base registers
            HAL_GOP_GetDram_Addr(&pGOPDrvLocalCtx->halCtxLocal, u8GOP, u8Win, &phyMainBase, E_DRV_GOP_RBLKAddr);

            HAL_GOP_GetDram_Addr(&pGOPDrvLocalCtx->halCtxLocal, u8GOP, u8Win, &phySubBase, E_DRV_GOP_RBLK3DAddr);

            //Swap main/sub base register value
            HAL_GOP_SetDram_Addr(&pGOPDrvLocalCtx->halCtxLocal, u8GOP, u8Win, phyMainBase, E_DRV_GOP_RBLK3DAddr);
            HAL_GOP_SetDram_Addr(&pGOPDrvLocalCtx->halCtxLocal, u8GOP, u8Win, phySubBase, E_DRV_GOP_RBLKAddr);
        }
    }
    pGOPCtx->pGOPCtxShared->bGOP3DLRSwitch[u8GOP] = !pGOPCtx->pGOPCtxShared->bGOP3DLRSwitch[u8GOP];  //Invert L/R swap flag
    return GOP_SUCCESS;
}

MS_U16 MDrv_GOP_GetBPP(MS_GOP_CTX_LOCAL*pGOPCtx, DRV_GOPColorType fbFmt)
{
    return HAL_GOP_GetBPP(&(((GOP_CTX_DRV_LOCAL*)pGOPCtx)->halCtxLocal), fbFmt);
}

E_GOP_VIDEOTIMING_MIRRORTYPE Mdrv_GOP_GetVideoTimingMirrorType(MS_GOP_CTX_LOCAL* pGOPCtx, MS_BOOL bHorizontal)
{
    GOP_CTX_DRV_LOCAL*pGOPDrvLocalCtx = (GOP_CTX_DRV_LOCAL*)pGOPCtx;
    EN_DRV_GOP_XC_MIRRORMODE enMirrorMode = E_DRV_GOP_XC_MIRROR_NORMAL;

    return HAL_GOP_GetVideoTimingMirrorType(&pGOPDrvLocalCtx->halCtxLocal, bHorizontal, enMirrorMode);
}

/********************************************************************************/
/// Set GOP OC(OSD Compression)
/********************************************************************************/
GOP_Result MDrv_GOP_OC_SetOCEn(MS_GOP_CTX_LOCAL*pGOPCtx, MS_U8 u8GOPNum, MS_BOOL bOCEn)
{
    GOP_Result ret;

    GOP_CTX_DRV_LOCAL*pGOPDrvLocalCtx = (GOP_CTX_DRV_LOCAL*)pGOPCtx;

    ret = HAL_GOP_OC_SetOCEn(&pGOPDrvLocalCtx->halCtxLocal, u8GOPNum,  bOCEn);

    return ret;

}

GOP_Result MDrv_GOP_OC_SetOCInfo(MS_GOP_CTX_LOCAL*pGOPCtx,DRV_GOP_OC_INFO* OCinfo)
{
    MS_U8 regmiu =0;
    MS_U8 u8MiuSel;
    MS_PHY phyOffset = 0;

    GOP_CTX_DRV_LOCAL*pGOPDrvLocalCtx = (GOP_CTX_DRV_LOCAL*)pGOPCtx;

    //OC HW just read the relative offset of each MIU
    _phy_to_miu_offset(u8MiuSel, phyOffset, OCinfo->u64GOP_OC_DRAM_RBLK);
    OCinfo->u64GOP_OC_DRAM_RBLK = phyOffset;
    HAL_GOP_OC_Get_MIU_Sel(&pGOPDrvLocalCtx->halCtxLocal, &regmiu);
    if( regmiu != u8MiuSel)
    {
        GOP_D_ERR("\n %s, Wrong MIU setting! Your address(0x%tx) is across MIU0, but OC MIU setting is still MIU:%d\n",__FUNCTION__,\
        (ptrdiff_t)OCinfo->u64GOP_OC_DRAM_RBLK , regmiu);
        return GOP_FAIL;
    }

    HAL_GOP_OC_SetOCInfo(&pGOPDrvLocalCtx->halCtxLocal, OCinfo);

    return GOP_SUCCESS;
}

MS_BOOL MDrv_GOP_GWIN_BeginDraw(void)
{
#if 0//GOP_LOCK_SUPPORT
    if (g_gopDrvCtxLocal.apiCtxLocal.s32GOPMutex >= 0)
    {
        DRV_GOP_ENTRY();
    }
    else
    {
        printf("%s: Error, no mutex to obtain\n", __FUNCTION__);
        return FALSE;
    }
#endif
    return TRUE;
}

MS_BOOL MDrv_GOP_GWIN_EndDraw(void)
{
#if 0//GOP_LOCK_SUPPORT
    if (g_gopDrvCtxLocal.apiCtxLocal.s32GOPMutex >= 0)
    {
        DRV_GOP_RETURN();
    }
    else
    {
        printf("%s: Error, no mutex to obtain\n", __FUNCTION__);
        return FALSE;
    }
#endif
    return TRUE;
}

#ifdef CONFIG_GOP_AFBC_FEATURE
GOP_Result MDrv_GOP_AFBC_Core_Reset(MS_GOP_CTX_LOCAL*pGOPCtx, MS_U8 u8GOP)
{
	//normal case not use reset
	return GOP_SUCCESS;
}

GOP_Result MDrv_GOP_AFBC_Core_Enable(MS_GOP_CTX_LOCAL*pGOPCtx, MS_U8 u8GOP, MS_BOOL bEna)
{
	GOP_CTX_DRV_LOCAL*pGOPDrvLocalCtx = (GOP_CTX_DRV_LOCAL*)pGOPCtx;

	HAL_GOP_AFBC_Core_Enable(&pGOPDrvLocalCtx->halCtxLocal, u8GOP, bEna);

	return GOP_SUCCESS;
}

GOP_Result MDrv_GOP_GWIN_AFBCMode(MS_GOP_CTX_LOCAL*pGOPCtx, MS_BOOL u8GOP, MS_BOOL bEnable, MS_U8 eCTL)
{
	GOP_CTX_DRV_LOCAL*pGOPDrvLocalCtx = (GOP_CTX_DRV_LOCAL*)pGOPCtx;

	if (MHal_GOP_AFBCMode(&pGOPDrvLocalCtx->halCtxLocal, u8GOP, bEnable) != GOP_SUCCESS)
		return GOP_FAIL;

	return GOP_SUCCESS;
}

GOP_Result MDrv_GOP_GWIN_AFBCSetWindow(MS_GOP_CTX_LOCAL*pGOPCtx, MS_U8 u8GOP, DRV_GOP_AFBC_Info* pinfo, MS_BOOL bChangePitch)
{
    GOP_CTX_DRV_LOCAL*pGOPDrvLocalCtx = (GOP_CTX_DRV_LOCAL*)pGOPCtx;

    if(pGOPDrvLocalCtx->apiCtxLocal.pGopChipProperty->bAFBC_Support[u8GOP] ==FALSE)
    {
        GOP_D_ERR("[%s] GOP AFBC mode not support GOP %d \n",__FUNCTION__, u8GOP);
        return GOP_FUN_NOT_SUPPORTED;
    }

    if(MHal_GOP_AFBCSetWindow(&pGOPDrvLocalCtx->halCtxLocal, u8GOP, pinfo) != GOP_SUCCESS)
    {
        return GOP_FAIL;
    }

    if(pGOPCtx->pGopChipProperty->bAFBC_Merge_GOP_Trig ==FALSE)
    {
        bAFBCTrigger=TRUE;
    }
    return GOP_SUCCESS;
}
#endif

GOP_Result MDrv_GOP_GWIN_DeleteWinHVSize(MS_GOP_CTX_LOCAL*pGOPCtx, MS_U8 u8GOP, MS_U16 u16HSize, MS_U16 u16VSize)
{

    GOP_CTX_DRV_LOCAL*pGOPDrvLocalCtx = (GOP_CTX_DRV_LOCAL*)pGOPCtx;

    HAL_GOP_DeleteWinHVSize(&pGOPDrvLocalCtx->halCtxLocal,u8GOP, u16HSize, u16VSize);
    return GOP_SUCCESS;
}

void MDrv_GOP_SelfFirstHs(MS_GOP_CTX_LOCAL*pGOPCtx, MS_U8 u8Gop, MS_BOOL bEnable)
{
	GOP_CTX_DRV_LOCAL*pGOPDrvLocalCtx = (GOP_CTX_DRV_LOCAL*)pGOPCtx;

	HAL_GOP_SetFirstHs(&pGOPDrvLocalCtx->halCtxLocal, u8Gop, bEnable);
}

GOP_Result MDrv_GOP_IsHDREnabled(MS_GOP_CTX_LOCAL*pGOPCtx, MS_BOOL* pbHDREnable)
{
    GOP_CTX_DRV_LOCAL*pGOPDrvLocalCtx = (GOP_CTX_DRV_LOCAL*)pGOPCtx;

    HAL_GOP_IsHDREnabled(&pGOPDrvLocalCtx->halCtxLocal, pbHDREnable);

    return GOP_SUCCESS;
}

GOP_Result MDrv_GOP_SetGOPMixerValid(MS_GOP_CTX_LOCAL*pGOPCtx, MS_U8 u8Gop, MS_BOOL bEnable)
{
    GOP_CTX_DRV_LOCAL*pGOPDrvLocalCtx = (GOP_CTX_DRV_LOCAL*)pGOPCtx;

    HAL_GOP_SetGOPMixerValid(&pGOPDrvLocalCtx->halCtxLocal, u8Gop, bEnable);

    return GOP_SUCCESS;
}

GOP_Result MDrv_GOP_SetDbgLevel(EN_GOP_DEBUG_LEVEL level)
{
    u32GOPDbgLevel_drv = level;
	HAL_GOP_SetDbgLevel(level);
    return GOP_SUCCESS;
}

GOP_Result MDrv_GOP_GWIN_PowerState(void* pInstance, MS_U32 u32PowerState, void* pModule)
{
    return GOP_SUCCESS;
}


void MDrv_GOP_GWIN_Interrupt(MS_GOP_CTX_LOCAL*pGOPCtx,MS_U8 u8Gop,MS_BOOL bEable)
{
	GOP_CTX_DRV_LOCAL*pGOPDrvLocalCtx = (GOP_CTX_DRV_LOCAL*)pGOPCtx;

	HAL_GOP_SetInterrupt(&pGOPDrvLocalCtx->halCtxLocal, u8Gop, bEable);
}

GOP_Result MDrv_GOP_VOP_Path_Sel(MS_GOP_CTX_LOCAL* pGOPCtx,EN_GOP_VOP_PATH_MODE enGOPPath)
{
    GOP_CTX_DRV_LOCAL*pGOPDrvLocalCtx = (GOP_CTX_DRV_LOCAL*)pGOPCtx;

    if(pGOPDrvLocalCtx == NULL)
    {
        return GOP_FAIL;
    }

    if(MHal_GOP_VOP_Path_Sel(&pGOPDrvLocalCtx->halCtxLocal,(EN_DRV_GOP_VOP_PATH)enGOPPath) != GOP_SUCCESS)
    {
        GOP_D_ERR("[%s][%d] MHal_GOP_VOP_Path_Sel Fail \n",__func__,__LINE__);
        return GOP_FAIL;
    }
    return GOP_SUCCESS;
}

GOP_Result MDrv_GOP_CSC_Tuning(MS_GOP_CTX_LOCAL* pGOPCtx,MS_U32 u32GOPNum,ST_GOP_CSC_PARAM *pstCSCParam)
{
    GOP_CTX_DRV_LOCAL*pGOPDrvLocalCtx = (GOP_CTX_DRV_LOCAL*)pGOPCtx;
    ST_DRV_GOP_CFD_OUTPUT stCFDOutput,stCFDOutputTmp;
    ST_GOP_CSC_PARAM stCSCParamTmp;
    MS_U32 u32BankOffSet = 0;

    if(pGOPDrvLocalCtx == NULL || pGOPCtx == NULL || pstCSCParam == NULL)
    {
        GOP_D_ERR("[%s][%d] Local Ctx or csc param is null \n",__func__,__LINE__);
        return GOP_FAIL;
    }

    if(pGOPDrvLocalCtx->halCtxLocal.pGopChipPro->bSupportCSCTuning == FALSE)
    {
        GOP_D_WARN("[%s][%d] GOP_FUN_NOT_SUPPORTED!!! \n",__FUNCTION__,__LINE__);
        return GOP_FUN_NOT_SUPPORTED;
    }

    if(pstCSCParam->bCscEnable == TRUE)
    {
        if((pstCSCParam->enInputFormat ==  E_GOP_CFD_CFIO_MAX) || pstCSCParam->enOutputFormat ==  E_GOP_CFD_CFIO_MAX)
        {
            GOP_D_WARN("[%s][%d] enInputFormat or enOutputFormat type is E_GOP_CFD_CFIO_MAX \n",__FUNCTION__,__LINE__);
            return GOP_FAIL;
        }
        if((pstCSCParam->enInputDataFormat ==  E_GOP_CFD_MC_FORMAT_MAX) || pstCSCParam->enOutputDataFormat ==  E_GOP_CFD_MC_FORMAT_MAX)
        {
            GOP_D_WARN("[%s][%d] enInputDataFormat or enOutputDataFormat type is E_GOP_CFD_MC_FORMAT_MAX \n",__FUNCTION__,__LINE__);
            return GOP_FAIL;
        }
        memset(&stCFDOutput, 0, sizeof(ST_DRV_GOP_CFD_OUTPUT));
        memcpy(&(pGOPCtx->pGOPCtxShared->stCSCParam[u32GOPNum]),pstCSCParam,sizeof(ST_GOP_CSC_PARAM));
        _GetBnkOfstByGop(u32GOPNum, &u32BankOffSet);

        if(MDrv_GOP_GFLIP_CSC_Tuning(pGOPCtx,u32GOPNum,pstCSCParam,&stCFDOutput) == FALSE)
        {
            GOP_D_ERR("[%s][%d] MDrv_GOP_GFLIP_CSC_Tuning  Fail \n",__func__,__LINE__);
            return GOP_FAIL;
        }

        pGOPCtx->pGOPCtxShared->stCSCTable[u32GOPNum].u32Version = ST_GOP_CSC_TABLE_VERSION;
        pGOPCtx->pGOPCtxShared->stCSCTable[u32GOPNum].u32Length = sizeof(ST_GOP_CSC_TABLE);
        pGOPCtx->pGOPCtxShared->stCSCTable[u32GOPNum].u16CscControl = stCFDOutput.u16CSCValue[0];
        pGOPCtx->pGOPCtxShared->stCSCTable[u32GOPNum].stCSCMatrix.Matrix[0][0] = stCFDOutput.u16CSCValue[1];
        pGOPCtx->pGOPCtxShared->stCSCTable[u32GOPNum].stCSCMatrix.Matrix[0][1] = stCFDOutput.u16CSCValue[2];
        pGOPCtx->pGOPCtxShared->stCSCTable[u32GOPNum].stCSCMatrix.Matrix[0][2] = stCFDOutput.u16CSCValue[3];
        pGOPCtx->pGOPCtxShared->stCSCTable[u32GOPNum].stCSCMatrix.Matrix[1][0] = stCFDOutput.u16CSCValue[4];
        pGOPCtx->pGOPCtxShared->stCSCTable[u32GOPNum].stCSCMatrix.Matrix[1][1] = stCFDOutput.u16CSCValue[5];
        pGOPCtx->pGOPCtxShared->stCSCTable[u32GOPNum].stCSCMatrix.Matrix[1][2] = stCFDOutput.u16CSCValue[6];
        pGOPCtx->pGOPCtxShared->stCSCTable[u32GOPNum].stCSCMatrix.Matrix[2][0] = stCFDOutput.u16CSCValue[7];
        pGOPCtx->pGOPCtxShared->stCSCTable[u32GOPNum].stCSCMatrix.Matrix[2][1] = stCFDOutput.u16CSCValue[8];
        pGOPCtx->pGOPCtxShared->stCSCTable[u32GOPNum].stCSCMatrix.Matrix[2][2] = stCFDOutput.u16CSCValue[9];
        pGOPCtx->pGOPCtxShared->stCSCTable[u32GOPNum].u16BrightnessOffsetR = stCFDOutput.u16BriValue[2];
        pGOPCtx->pGOPCtxShared->stCSCTable[u32GOPNum].u16BrightnessOffsetG = stCFDOutput.u16BriValue[1];
        pGOPCtx->pGOPCtxShared->stCSCTable[u32GOPNum].u16BrightnessOffsetB = stCFDOutput.u16BriValue[0];

        //Set Y2R table with CSC
        if(pstCSCParam->enInputDataFormat == E_GOP_CFD_MC_FORMAT_YUV422 && pstCSCParam->enOutputDataFormat == E_GOP_CFD_MC_FORMAT_RGB)
        {
            MHal_GOP_FillCscMartrix(&pGOPDrvLocalCtx->halCtxLocal,stCFDOutput.u16CSCValue,E_GOP_CFD_MC_FORMAT_YUV422,E_GOP_CFD_MC_FORMAT_RGB);
        }
        else
        {
            memset(&stCFDOutputTmp, 0, sizeof(ST_DRV_GOP_CFD_OUTPUT));
            memcpy(&stCSCParamTmp,pstCSCParam,sizeof(ST_GOP_CSC_PARAM));
            stCSCParamTmp.enInputFormat = E_GOP_CFD_CFIO_YUV_DEFAULT;
            stCSCParamTmp.enInputDataFormat = E_GOP_CFD_MC_FORMAT_YUV422;
            stCSCParamTmp.enInputRange = E_GOP_CFD_CFIO_RANGE_FULL;
            stCSCParamTmp.enOutputFormat = E_GOP_CFD_CFIO_RGB_DEFAULT;
            stCSCParamTmp.enOutputDataFormat = E_GOP_CFD_MC_FORMAT_RGB;
            stCSCParamTmp.enOutputRange = E_GOP_CFD_CFIO_RANGE_FULL;
            if(MDrv_GOP_GFLIP_CSC_Tuning(pGOPCtx,u32GOPNum,&stCSCParamTmp,&stCFDOutputTmp) == FALSE)
            {
                GOP_D_ERR("[%s][%d] MDrv_GOP_GFLIP_CSC_Tuning  Fail \n",__func__,__LINE__);
            }
            MHal_GOP_FillCscMartrix(&pGOPDrvLocalCtx->halCtxLocal,stCFDOutputTmp.u16CSCValue,E_GOP_CFD_MC_FORMAT_YUV422,E_GOP_CFD_MC_FORMAT_RGB);
        }

         //Set R2R table with CSC
        if(pstCSCParam->enInputDataFormat == E_GOP_CFD_MC_FORMAT_RGB && pstCSCParam->enOutputDataFormat == E_GOP_CFD_MC_FORMAT_RGB)
        {
            MHal_GOP_FillCscMartrix(&pGOPDrvLocalCtx->halCtxLocal,stCFDOutput.u16CSCValue,E_GOP_CFD_MC_FORMAT_RGB,E_GOP_CFD_MC_FORMAT_RGB);
        }
        else
        {
            memset(&stCFDOutputTmp, 0, sizeof(ST_DRV_GOP_CFD_OUTPUT));
            memcpy(&stCSCParamTmp,pstCSCParam,sizeof(ST_GOP_CSC_PARAM));
            stCSCParamTmp.enInputFormat = E_GOP_CFD_CFIO_RGB_DEFAULT;
            stCSCParamTmp.enInputDataFormat = E_GOP_CFD_MC_FORMAT_RGB;
            stCSCParamTmp.enInputRange = E_GOP_CFD_CFIO_RANGE_FULL;
            stCSCParamTmp.enOutputFormat = E_GOP_CFD_CFIO_RGB_DEFAULT;
            stCSCParamTmp.enOutputDataFormat = E_GOP_CFD_MC_FORMAT_RGB;
            stCSCParamTmp.enOutputRange = E_GOP_CFD_CFIO_RANGE_FULL;
            if(MDrv_GOP_GFLIP_CSC_Tuning(pGOPCtx,u32GOPNum,&stCSCParamTmp,&stCFDOutputTmp) == FALSE)
            {
                GOP_D_ERR("[%s][%d] MDrv_GOP_GFLIP_CSC_Tuning  Fail \n",__func__,__LINE__);
            }
            MHal_GOP_FillCscMartrix(&pGOPDrvLocalCtx->halCtxLocal,stCFDOutputTmp.u16CSCValue,E_GOP_CFD_MC_FORMAT_RGB,E_GOP_CFD_MC_FORMAT_RGB);
        }

        if(pstCSCParam->bUpdateWithVsync == TRUE)
        {
            if(pGOPDrvLocalCtx->halCtxLocal.pGopChipPro->bSupportCSCDoubleBuffer == TRUE)
            {
                if(MHal_GOP_SetCSCCtrl(&pGOPDrvLocalCtx->halCtxLocal,u32GOPNum,&(pGOPCtx->pGOPCtxShared->stCSCTable[u32GOPNum])) != GOP_SUCCESS)
                {
                    GOP_D_ERR("[%s][%d] MHal_GOP_SetCSC Fail \n",__func__,__LINE__);
                    return GOP_FAIL;
                }
            }
            else
            {
                bCSCSelect[u32GOPNum] = TRUE;
                if(MHal_GOP_SetCSCInfo(u32GOPNum,&(pGOPCtx->pGOPCtxShared->stCSCTable[u32GOPNum]),TRUE) != GOP_SUCCESS)
                {
                    GOP_D_ERR("[%s][%d] MHal_GOP_SetCSCInfo Fail \n",__func__,__LINE__);
                    return GOP_FAIL;
                }
            }
        }
        else
        {
            if(MHal_GOP_SetCSCCtrl(&pGOPDrvLocalCtx->halCtxLocal,u32GOPNum,&(pGOPCtx->pGOPCtxShared->stCSCTable[u32GOPNum])) != GOP_SUCCESS)
            {
                GOP_D_ERR("[%s][%d] MHal_GOP_SetCSC Fail \n",__func__,__LINE__);
                return GOP_FAIL;
            }
        }
    }
    else
    {
        if(_MDrv_GOP_CSC_ParamInit(&(pGOPCtx->pGOPCtxShared->stCSCParam[u32GOPNum])) == FALSE)
        {
            GOP_D_ERR("[%s][%d] _MDrv_GOP_CSC_ParamInit  Fail \n",__func__,__LINE__);
            return GOP_FAIL;
        }
        if(_MDrv_GOP_CSC_TableInit(&(pGOPCtx->pGOPCtxShared->stCSCTable[u32GOPNum])) == FALSE)
        {
            GOP_D_ERR("[%s][%d] _MDrv_GOP_CSC_TableInit  Fail \n",__func__,__LINE__);
            return GOP_FAIL;
        }
		MHal_GOP_GWIN_SetColorMatrix(&pGOPDrvLocalCtx->halCtxLocal, (MS_U8)u32GOPNum, DRV_GOPOUT_RGB, E_DRV_GOP_COLOR_ARGB8888, FALSE);
    }

    return GOP_SUCCESS;
}
GOP_Result MDrv_GOP_SetCSCCtrl(MS_GOP_CTX_LOCAL* pGOPCtx,MS_U32 u32GOPNum,ST_GOP_CSC_TABLE *pstCSCTbl)
{
    GOP_CTX_DRV_LOCAL*pGOPDrvLocalCtx = (GOP_CTX_DRV_LOCAL*)pGOPCtx;

    if(pGOPDrvLocalCtx == NULL || pGOPCtx == NULL || pstCSCTbl == NULL)
    {
        GOP_D_ERR("[%s][%d] pointer is NULL \n",__func__,__LINE__);
        return GOP_FAIL;
    }

    memcpy(&(pGOPCtx->pGOPCtxShared->stCSCTable[u32GOPNum]),pstCSCTbl,sizeof(ST_GOP_CSC_TABLE));

    if((MHal_GOP_SetCSCCtrl(&pGOPDrvLocalCtx->halCtxLocal,u32GOPNum,pstCSCTbl)) != GOP_SUCCESS)
    {
        GOP_D_ERR("[%s][%d] MHal_GOP_SetCSC Fail \n",__func__,__LINE__);
        return GOP_FAIL;
    }
    return GOP_SUCCESS;
}

GOP_Result MDrv_GOP_AutoDectBuf(MS_GOP_CTX_LOCAL*pstGOPCtx, ST_GOP_AUTO_DETECT_BUF_INFO* pstAutoDectInfo)
{
    GOP_CTX_DRV_LOCAL*pstGOPDrvLocalCtx = (GOP_CTX_DRV_LOCAL*)pstGOPCtx;

    if(pstGOPDrvLocalCtx == NULL || pstGOPCtx == NULL || pstAutoDectInfo == NULL)
    {
        GOP_D_ERR("[%s][%d] pointer is NULL \n",__FUNCTION__,__LINE__);
        return GOP_FAIL;
    }

    if(pstAutoDectInfo->u8GOPNum >= gGopChipPro.TotalGOPNum)
    {
        GOP_D_ERR("[%s][%d] GOP %d is out of bound!! \n",__FUNCTION__,__LINE__,pstAutoDectInfo->u8GOPNum);
        return GOP_FAIL;
    }

    if(pstGOPDrvLocalCtx->apiCtxLocal.pGopChipProperty->bAutoDetectVersion_1_0[pstAutoDectInfo->u8GOPNum] == TRUE)
    {
        if(MHal_GOP_AutoDetectBuf(&pstGOPDrvLocalCtx->halCtxLocal, pstAutoDectInfo) != GOP_SUCCESS)
        {
            GOP_D_ERR("[%s][%d] MHal_GOP_AutoDetectBuf fail!!\n",__func__,__LINE__);
            return GOP_FAIL;
        }
    }
    if(pstGOPDrvLocalCtx->apiCtxLocal.pGopChipProperty->bAutoDetectVersion_2_0[pstAutoDectInfo->u8GOPNum] == TRUE)
    {
        if(MHal_GOP_AutoDetectBuf_V2(&pstGOPDrvLocalCtx->halCtxLocal, pstAutoDectInfo) != GOP_SUCCESS)
        {
            GOP_D_ERR("[%s][%d] MHal_GOP_AutoDetectBuf_V2 fail!!\n",__func__,__LINE__);
            return GOP_FAIL;
        }
    }

    return GOP_SUCCESS;
}

GOP_Result MDrv_GOP_GetOsdNonTransCnt(MS_GOP_CTX_LOCAL*pstGOPCtx, MS_U32* pu32Count)
{
    GOP_CTX_DRV_LOCAL*pstGOPDrvLocalCtx = (GOP_CTX_DRV_LOCAL*)pstGOPCtx;

    MHal_GOP_GetOsdNonTransCnt(&pstGOPDrvLocalCtx->halCtxLocal, pu32Count);

    return GOP_SUCCESS;
}

GOP_Result MDrv_GOP_GWIN_Trigger_MUX(MS_GOP_CTX_LOCAL* pstGOPCtx)
{
    GOP_CTX_DRV_LOCAL*pstGOPDrvLocalCtx = (GOP_CTX_DRV_LOCAL*)pstGOPCtx;

    MHal_GOP_Trigger_MUX(&pstGOPDrvLocalCtx->halCtxLocal);

    return GOP_SUCCESS;
}

GOP_Result MDrv_GOP_ByPassMode(MS_GOP_CTX_LOCAL* pstGOPCtx, MS_U32 u32GOPIdx, MS_BOOL bEnable)
{
    GOP_CTX_DRV_LOCAL*pstGOPDrvLocalCtx = (GOP_CTX_DRV_LOCAL*)pstGOPCtx;

    return HAL_GOP_ByPassMode(&pstGOPDrvLocalCtx->halCtxLocal, u32GOPIdx, bEnable);
}
