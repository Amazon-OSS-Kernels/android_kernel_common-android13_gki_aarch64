// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (c) 2023 MediaTek Inc.
 */
#ifndef _DRV_GE_C_
#define _DRV_GE_C_

//-------------------------------------------------------------------------------------------------
//  Include Files
//-------------------------------------------------------------------------------------------------
#ifndef MSOS_TYPE_LINUX_KERNEL
#include <stdio.h>
#include <string.h>
#endif
#include "MsCommon.h"
#include "MsVersion.h"
#include "MsOS.h"
#include "utopia.h"
#include "utopia_dapi.h"

#include "drvGE.h"
#include "halGE.h"
#include "drvGE_private.h"
#include "regCHIP.h"
#include "halCHIP.h"
#include "regGE.h"
#include "drvSEM.h"
#include "osalGE.h"
#include "drvMMIO.h"
#if (GE_USE_HW_SEM && defined(MSOS_TYPE_OPTEE))
#include "drvSEAL.h"
#endif
//-------------------------------------------------------------------------------------------------
//  Driver Compiler Options
//-------------------------------------------------------------------------------------------------
#define GE_DEBUG_ENABLE             0UL

#define ENABLE_PWR          0UL   // FIXME: for porting
#ifndef MS_DEBUG
#define MS_DEBUG                    1UL
#endif

#if (GFX_UTOPIA20)
#define g_drvGECtxLocal     psGFXInstPriTmp->GFXPrivate_g_drvGECtxLocal
#else
GE_CTX_LOCAL g_drvGECtxLocal;
#endif

#if defined(MSOS_TYPE_LINUX) || defined(MSOS_TYPE_LINUX_KERNEL)
#else
GE_CTX_SHARED g_drvGECtxShared;
#endif

#if defined(MSOS_TYPE_LINUX_KERNEL)
GE_CTX_HAL_LOCAL g_halGECtxLocal;
#endif

typedef struct
{
    MS_U32 u32BlitFlags;
    MS_U32 u32TrapeHeight;
    //Src
    MS_U32 u32SrcX;
    MS_U32 u32SrcY;
    MS_U32 u32SrcW;
    MS_U32 u32SrcH;
    MS_S32 s32DeltaSrcX;  //s5.12
    MS_S32 s32DeltaSrcY;  //s5.12
    //Dst
    MS_U32 u32DstX0;
    MS_U32 u32DstY0;
    MS_U32 u32DstX1;
    MS_U32 u32DstY1;
    MS_S32 s32DeltaDstX0; //s5.12
    MS_S32 s32DeltaDstY0; //s5.12
    MS_S32 s32DeltaDstX1; //s5.12
    MS_U32 s32DeltaDstY1; //s5.12
}GE_TrapeBatchBlitObj;

typedef struct
{
    MS_U32 u32FillFlags;
    MS_U32 u32TrapeHeight;
    //Src
    MS_U32 u32ColorS;
    MS_U32 u32ColorE;
    MS_S32 s32ColorDeltaB;
    MS_S32 s32ColorDeltaG;
    MS_S32 s32ColorDeltaR;
    MS_S32 s32ColorDeltaA;

    //Dst
    MS_U32 u32DstX0;
    MS_U32 u32DstY0;
    MS_U32 u32DstX1;
    MS_U32 u32DstY1;
    MS_S32 s32DeltaDstX0; //s5.12
    MS_S32 s32DeltaDstY0; //s5.12
    MS_S32 s32DeltaDstX1; //s5.12
    MS_U32 s32DeltaDstY1; //s5.12
}GE_TrapeBatchFillObj;

//-------------------------------------------------------------------------------------------------
//  Local Defines
//-------------------------------------------------------------------------------------------------
#define DYNAMIC_POWER_ON_OFF    0UL
#define SEM_HK51ID              0x01UL
#define SEM_HKAeonID            0x02UL
#define SEM_MWMipsID            0x03UL
#define GESEMID                 0x01UL
#define PREGESEMID              0x02UL

#define MDrv_PWR_ClockTurnOn(x)  {}     // FIXME: for porting
#define MDrv_PWR_ClockTurnOff(x)  {}     // FIXME: for porting

#define E_PWR_MODULE_GE 0UL

#ifdef  MSOS_TYPE_LINUX
#include <assert.h>
#include <unistd.h>
#define GE_ASSERT_CHECK(_bool, pri)  if (!(_bool)) {GE_D_FATAL("\nAssert in %s,%d\n", __FUNCTION__, __LINE__); (pri); MsOS_DelayTask(100); assert(0);}
#else
#define GE_ASSERT_CHECK(_bool, pri)  if (!(_bool)) {GE_D_FATAL("\nAssert in %s,%d\n", __FUNCTION__, __LINE__); (pri);};
#endif

#if GE_LOCK_SUPPORT
#if (GE_API_MUTEX == 0)
 #define GE_ENTRY(pGECtx)                 GE_BeginDraw(pGECtx,TRUE);
 #define GE_RETURN(pGECtx, _ret)          do{ GE_EndDraw(pGECtx,TRUE);   return _ret;  }while(0);
#else
#define GE_ENTRY(pGECtx)
#define GE_RETURN(pGECtx, _ret)           return _ret;
#endif
#else
#define GE_ENTRY(pGECtx)                  do{}while(0);
#define GE_RETURN(pGECtx, _ret)           return _ret;
#endif

#if GE_LOCK_SUPPORT //-----------------------------------------------------------------------------
#if GE_USE_HW_SEM
MS_U16 GE_HWSemaphore_count =0;
MS_U16 GE_GetHWSemaphoreID=0;

#define DRV_GE_ENTRY(pGECtx,bHWSemLock)       do{\
                                                       GE_CTX_LOCAL *pGECtxLocal = (GE_CTX_LOCAL*)pGECtx; \
                                                       if(pGECtxLocal->u32CTXInitMagic != 0x55aabeef) {return;}  \
                                                       if (0 > pGECtxLocal->s32GEMutex)    \
                                                       { return;  }                  \
                                                       if((pGECtxLocal->s32GE_Recursive_Lock_Cnt > 0) && (pGECtxLocal->s32GELock == MsOS_GetOSThreadID()))   \
                                                       {   \
                                                           pGECtxLocal->s32GE_Recursive_Lock_Cnt++;   \
                                                       }   \
                                                       else   \
                                                       {   \
                                                           /*Step 1: Check Mutex*/\
                                                           while(!MsOS_ObtainMutex(pGECtxLocal->s32GEMutex, MSOS_WAIT_FOREVER))    \
                                                           {\
                                                               GE_D_DBUG("\33[0;31m [%s][%d] Obtian GE Mutex Fail \33[m;\n",__FUNCTION__,__LINE__);\
                                                           }\
                                                           /*HWSemaphore*/ \
                                                           if(bHWSemLock == TRUE)  \
                                                           {  \
                                                           /*Step 1: Check HWSemaphore*/\
                                                               while (MDrv_SEM_Get_Resource(GESEMID, pGECtxLocal->u32GESEMID) == FALSE)\
                                                               {   \
                                                                   GE_HWSemaphore_count++;\
                                                                   if((GE_HWSemaphore_count%100) == 0 )\
                                                                   {\
                                                                       MsOS_DelayTask(1);\
                                                                   }\
                                                                   if(GE_HWSemaphore_count>10000)\
                                                                   {\
                                                                       MDrv_SEM_Get_ResourceID(GESEMID, &GE_GetHWSemaphoreID);\
                                                                       GE_D_DBUG("\33[0;31m [Entry While][%s][%d]GE_GetHWSemphoreID=%tx; GESEMID=%tx, TID=0x%tx \33[m;\n",__FUNCTION__,__LINE__, (ptrdiff_t)GE_GetHWSemaphoreID, (ptrdiff_t)pGECtxLocal->u32GESEMID,(ptrdiff_t)MsOS_GetOSThreadID());\
                                                                       GE_HWSemaphore_count =0;\
                                                                   }\
                                                               }\
                                                           }\
                                                           /*HWSemaphore*/ \
                                                           pGECtxLocal->s32GELock = MsOS_GetOSThreadID();     \
                                                           pGECtxLocal->s32GE_Recursive_Lock_Cnt++;        \
                                                           /*printf("[%s][%05d][%ld]GFX_LOCK+++[%ld]\n",__FUNCTION__,__LINE__, pGECtxLocal->s32GELock, pGECtxLocal->s32GE_Recursive_Lock_Cnt); */ \
                                                           if(pGECtxLocal->pSharedCtx->u32LstGEClientId!=pGECtxLocal->u32GEClientId)  \
                                                           {   \
                                                               GE_Restore_HAL_Context(&pGECtxLocal->halLocalCtx,pGECtxLocal->pSharedCtx->bNotFirstInit); \
                                                               pGECtxLocal->pSharedCtx->u32LstGEClientId = pGECtxLocal->u32GEClientId;  \
                                                           }  \
                                                       }   \
                                      }while(0)

#define DRV_GE_RETURN(pGECtx,bHWSemLock)     do{    \
                                                             GE_CTX_LOCAL *pGECtxLocal = (GE_CTX_LOCAL*)pGECtx;   \
                                                             GE_ASSERT_CHECK((pGECtxLocal->s32GELock==MsOS_GetOSThreadID()), (GE_D_FATAL("\nGE_RETURN: Fatal Error, Task ID mismatch[%td]->[%td]\n", (ptrdiff_t)pGECtxLocal->s32GELock, (ptrdiff_t)(MS_S32)MsOS_GetOSThreadID())));    \
                                                             GE_ASSERT_CHECK((0<pGECtxLocal->s32GE_Recursive_Lock_Cnt), (GE_D_FATAL("\nGE_RETURN: Fatal Error, No Mutex to release[Cnt=%td]\n", (ptrdiff_t)pGECtxLocal->s32GE_Recursive_Lock_Cnt)));    \
                                                             if(pGECtxLocal->s32GE_Recursive_Lock_Cnt > 1)  \
                                                             {  \
                                                                 pGECtxLocal->s32GE_Recursive_Lock_Cnt--;  \
                                                             }  \
                                                             else \
                                                             {   \
                                                                 pGECtxLocal->s32GE_Recursive_Lock_Cnt--;  \
                                                                 /*HWSemaphore*/ \
                                                                 if(bHWSemLock == TRUE)  \
                                                                 {  \
                                                                     while(MDrv_SEM_Free_Resource(GESEMID, pGECtxLocal->u32GESEMID) ==FALSE)\
                                                                     {\
                                                                        GE_HWSemaphore_count++;\
                                                                        if(GE_HWSemaphore_count>10000)\
                                                                        {\
                                                                            MDrv_SEM_Get_ResourceID(GESEMID, &GE_GetHWSemaphoreID);\
                                                                            GE_D_DBUG("\33[0;31m [Return While][%s][%d] GE_GetHWSemphoreID=%tx, GESEMID=%tx, TID=0x%tx \33[m;\n",__FUNCTION__,__LINE__, (ptrdiff_t)GE_GetHWSemaphoreID, (ptrdiff_t)pGECtxLocal->u32GESEMID,(ptrdiff_t)(MS_S32)MsOS_GetOSThreadID());\
                                                                            GE_HWSemaphore_count =0;\
                                                                        }\
                                                                     }\
                                                                 }\
                                                                 /*HWSemaphore*/ \
                                                                 while(!MsOS_ReleaseMutex(pGECtxLocal->s32GEMutex))   \
                                                                 {\
                                                                    GE_D_DBUG("\33[0;31m [%s][%d] Release GE Mutex Fail \33[m;\n",__FUNCTION__,__LINE__);\
                                                                 }\
                                                             }\
                                         }while(0)

#else
#define DRV_GE_ENTRY(pGECtx,bHWSemLock)       do{\
                                                       GE_CTX_LOCAL *pGECtxLocal = (GE_CTX_LOCAL*)pGECtx; \
                                                       if(pGECtxLocal->u32CTXInitMagic != 0x55aabeef) {return;}  \
                                                       if (0 > pGECtxLocal->s32GEMutex)    \
                                                       { return;  }                  \
                                                       if((pGECtxLocal->s32GE_Recursive_Lock_Cnt > 0) && (pGECtxLocal->s32GELock == MsOS_GetOSThreadID()))   \
                                                       {   \
                                                           pGECtxLocal->s32GE_Recursive_Lock_Cnt++;   \
                                                       }   \
                                                       else   \
                                                       {   \
                                                           /*Step 1: Check Mutex*/\
                                                           while(!MsOS_ObtainMutex(pGECtxLocal->s32GEMutex, MSOS_WAIT_FOREVER))    \
                                                           {\
                                                               GE_D_DBUG("\33[0;31m [%s][%d] Obtian GE Mutex Fail \33[m;\n",__FUNCTION__,__LINE__);\
                                                           }\
                                                           pGECtxLocal->s32GELock = MsOS_GetOSThreadID();     \
                                                           pGECtxLocal->s32GE_Recursive_Lock_Cnt++;        \
                                                           /*printf("[%s][%05d][%ld]GFX_LOCK+++[%ld]\n",__FUNCTION__,__LINE__, pGECtxLocal->s32GELock, pGECtxLocal->s32GE_Recursive_Lock_Cnt); */ \
                                                           if(pGECtxLocal->pSharedCtx->u32LstGEClientId!=pGECtxLocal->u32GEClientId)  \
                                                           {   \
                                                               GE_Restore_HAL_Context(&pGECtxLocal->halLocalCtx,pGECtxLocal->pSharedCtx->bNotFirstInit); \
                                                               pGECtxLocal->pSharedCtx->u32LstGEClientId = pGECtxLocal->u32GEClientId;  \
                                                           }  \
                                                       }   \
                                      }while(0)

#define DRV_GE_RETURN(pGECtx,bHWSemLock)     do{    \
                                                             GE_CTX_LOCAL *pGECtxLocal = (GE_CTX_LOCAL*)pGECtx;   \
                                                             GE_ASSERT_CHECK((pGECtxLocal->s32GELock==MsOS_GetOSThreadID()), (GE_D_FATAL("\nGE_RETURN: Fatal Error, Task ID mismatch[%td]->[%td]\n", (ptrdiff_t)pGECtxLocal->s32GELock, (ptrdiff_t)(MS_S32)MsOS_GetOSThreadID())));    \
                                                             GE_ASSERT_CHECK((0<pGECtxLocal->s32GE_Recursive_Lock_Cnt), (GE_D_FATAL("\nGE_RETURN: Fatal Error, No Mutex to release[Cnt=%td]\n", (ptrdiff_t)pGECtxLocal->s32GE_Recursive_Lock_Cnt)));    \
                                                             if(pGECtxLocal->s32GE_Recursive_Lock_Cnt > 1)  \
                                                             {  \
                                                                 pGECtxLocal->s32GE_Recursive_Lock_Cnt--;  \
                                                             }  \
                                                             else \
                                                             {   \
                                                                 pGECtxLocal->s32GE_Recursive_Lock_Cnt--;  \
                                                                 while(!MsOS_ReleaseMutex(pGECtxLocal->s32GEMutex))   \
                                                                 {\
                                                                    GE_D_DBUG("\33[0;31m [%s][%d] Release GE Mutex Fail \33[m;\n",__FUNCTION__,__LINE__);\
                                                                 }\
                                                             }\
                                         }while(0)
#endif //GE_USE_HW_SEM
#else //-------------------------------------------------------------------------------------------
#if GE_USE_HW_SEM
MS_U16 GE_HWSemaphore_count =0;
MS_U16 GE_GetHWSemaphoreID=0;

#define DRV_GE_ENTRY(pGECtx,bHWSemLock)                   do{\
                                                       GE_CTX_LOCAL *pGECtxLocal = (GE_CTX_LOCAL*)pGECtx; \
                                                       if(pGECtxLocal->u32CTXInitMagic != 0x55aabeef) {return;}  \
                                                       if((pGECtxLocal->s32GE_Recursive_Lock_Cnt > 0))   \
                                                       {   \
                                                           pGECtxLocal->s32GE_Recursive_Lock_Cnt++;   \
                                                       }   \
                                                       else   \
                                                       {   \
                                                           /*Step 1: Check HWSemaphore*/\
                                                           while (MDrv_SEM_Get_Resource(GESEMID, pGECtxLocal->u32GESEMID) == FALSE)\
                                                           {   \
                                                               GE_HWSemaphore_count++;\
                                                               if(GE_HWSemaphore_count>10000)\
                                                               {\
                                                                   MDrv_SEM_Get_ResourceID(GESEMID, &GE_GetHWSemaphoreID);\
                                                                   GE_D_DBUG("\33[0;31m [Entry While][%s][%d]GE_GetHWSemphoreID=%tx; pGECtxLocal->u32GESEMID=%tx \33[m;\n",__FUNCTION__,__LINE__, (ptrdiff_t)GE_GetHWSemaphoreID, (ptrdiff_t)pGECtxLocal->u32GESEMID);\
                                                                   GE_HWSemaphore_count =0;\
                                                               }\
                                                           }\
                                                           pGECtxLocal->s32GE_Recursive_Lock_Cnt++;        \
                                                           if(pGECtxLocal->pSharedCtx->u32LstGEClientId!=pGECtxLocal->u32GEClientId)  \
                                                           {   \
                                                               GE_Restore_HAL_Context(&pGECtxLocal->halLocalCtx,pGECtxLocal->pSharedCtx->bNotFirstInit); \
                                                               pGECtxLocal->pSharedCtx->u32LstGEClientId = pGECtxLocal->u32GEClientId;  \
                                                           }  \
                                                       }   \
                                      }while(0)

#define DRV_GE_RETURN(pGECtx,bHWSemLock)             do{\
                                                            GE_CTX_LOCAL *pGECtxLocal = (GE_CTX_LOCAL*)pGECtx;   \
                                                             GE_ASSERT_CHECK((0<pGECtxLocal->s32GE_Recursive_Lock_Cnt), (GE_D_FATAL("\nGE_RETURN: Fatal Error, No Mutex to release[Cnt=%td]\n", (ptrdiff_t)pGECtxLocal->s32GE_Recursive_Lock_Cnt)));    \
                                                             if(pGECtxLocal->s32GE_Recursive_Lock_Cnt > 1)  \
                                                             {  \
                                                                 pGECtxLocal->s32GE_Recursive_Lock_Cnt--;  \
                                                             }  \
                                                             else \
                                                             {   \
                                                                 pGECtxLocal->s32GE_Recursive_Lock_Cnt--;  \
                                                                     while(MDrv_SEM_Free_Resource(GESEMID, pGECtxLocal->u32GESEMID) ==FALSE)\
                                                                     {\
                                                                        GE_HWSemaphore_count++;\
                                                                        if(GE_HWSemaphore_count>10000)\
                                                                        {\
                                                                            MDrv_SEM_Get_ResourceID(GESEMID, &GE_GetHWSemaphoreID);\
                                                                            GE_D_DBUG("\33[0;31m [Return While][%s][%d] GE_GetHWSemphoreID=%tx; u32GESEMID=%tx\33[m;\n",__FUNCTION__,__LINE__, (ptrdiff_t)GE_GetHWSemaphoreID, (ptrdiff_t)pGECtxLocal->u32GESEMID);\
                                                                            GE_HWSemaphore_count =0;\
                                                                        }\
                                                                     }\
                                                             }\
                                     }while(0)
#else
#define DRV_GE_ENTRY(pGECtx,bHWSemLock)                   do{\
                                                       GE_CTX_LOCAL *pGECtxLocal = (GE_CTX_LOCAL*)pGECtx; \
                                                       if(pGECtxLocal->u32CTXInitMagic != 0x55aabeef) {return;}  \
                                                       if((pGECtxLocal->s32GE_Recursive_Lock_Cnt > 0))   \
                                                       {   \
                                                           pGECtxLocal->s32GE_Recursive_Lock_Cnt++;   \
                                                       }   \
                                                       else   \
                                                       {   \
                                                           pGECtxLocal->s32GE_Recursive_Lock_Cnt++;        \
                                                           if(pGECtxLocal->pSharedCtx->u32LstGEClientId!=pGECtxLocal->u32GEClientId)  \
                                                           {   \
                                                               GE_Restore_HAL_Context(&pGECtxLocal->halLocalCtx,pGECtxLocal->pSharedCtx->bNotFirstInit); \
                                                               pGECtxLocal->pSharedCtx->u32LstGEClientId = pGECtxLocal->u32GEClientId;  \
                                                           }  \
                                                       }   \
                                      }while(0)

#define DRV_GE_RETURN(pGECtx,bHWSemLock)             do{\
                                                            GE_CTX_LOCAL *pGECtxLocal = (GE_CTX_LOCAL*)pGECtx;   \
                                                             GE_ASSERT_CHECK((0<pGECtxLocal->s32GE_Recursive_Lock_Cnt), (GE_D_FATAL("\nGE_RETURN: Fatal Error, No Mutex to release[Cnt=%td]\n", (ptrdiff_t)pGECtxLocal->s32GE_Recursive_Lock_Cnt)));    \
                                                             pGECtxLocal->s32GE_Recursive_Lock_Cnt--;  \
                                     }while(0)
#endif
#endif //------------------------------------------------------------------------------------------

#if GE_DEBUG_ENABLE
#define GE_DBG(_fmt, _args...)      MS_DEBUG_MSG(GE_D_DBUG(_fmt, ##_args))
#else
#define GE_DBG(_fmt, _args...)      { }
#endif

#ifdef MS_DEBUG //---------------------------------------------------------------------------------
#define GE_ERR(_fmt, _args...)      GE_D_ERR(_fmt, ##_args)
#define GE_ASSERT(_cnd, _ret, _fmt, _args...)                   \
                                    do{if (!(_cnd)) {                \
                                        GE_D_ERR(_fmt, ##_args);    \
                                        return _ret;                \
                                        }                                    \
                                    }while(0);
#else //-------------------------------------------------------------------------------------------
#define GE_ERR(_fmt, _args...)      { }
#define GE_ASSERT(_cnd, _ret, _fmt, _args...)                   \
                                    do{if (!(_cnd)) {              \
                                        GE_D_ERR(_fmt, ##_args);  \
                                        return _ret;            \
                                        }                               \
                                    }while(0);  // CAUTION: It should be before GE_ENTRY
#endif //------------------------------------------------------------------------------------------

#define INT_TO_FIX32(X)     (((MS_S32) (X)) << 12)
#define FIX32_TO_INT(X)     ((MS_S32) ((X) >> 12))

#ifdef MSOS_TYPE_LINUX_KERNEL
#define atoi(str) simple_strtoul(((str != NULL) ? str : ""), NULL, 0)
#endif
//-------------------------------------------------------------------------------------------------
//  Local Structurs
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Global Variables
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Local Variables
//-------------------------------------------------------------------------------------------------
static MSIF_Version _drv_ge_version = {
    .DDI = { GE_DRV_VERSION },
};
static MS_U32 dbg_level = 0;
MS_BOOL bDither = FALSE;
#ifdef GE_SPLIT_SUPPORT
MS_BOOL bABL = FALSE;
#endif
#ifdef GE_ALPHA_BLEND_PATCH
extern MS_U16  u16GAlphaConst;
extern MS_U16  u16GAlphaMode;
#endif
#if !defined (MSOS_TYPE_NOS)
static MS_VIRT* pTLBSRCStartVA=NULL;
static MS_VIRT* pTLBDSTStartVA=NULL;
#endif
#ifdef GE_VQ_MIU_HANG_PATCH
static MS_U8 u8SrcMIU,u8DstMIU;
#endif

//-------------------------------------------------------------------------------------------------
//  Debug Functions
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Local Functions
//-------------------------------------------------------------------------------------------------
#if (GE_USE_HW_SEM && defined(MSOS_TYPE_OPTEE))
static MS_BOOL _OPTEE_GE_ENTRY(GE_Context *pGECtx,MS_BOOL bHWSemLock)
{
    GE_CTX_LOCAL *pGECtxLocal = (GE_CTX_LOCAL*)pGECtx;
    if(pGECtxLocal->u32CTXInitMagic != 0x55aabeef)
    {
        return FALSE;
    }
    if((pGECtxLocal->s32GE_Recursive_Lock_Cnt > 0))
    {
        pGECtxLocal->s32GE_Recursive_Lock_Cnt++;
    }
    else
    {
        /*Step 1: Check HWSemaphore*/
        while (MDrv_SEM_Get_Resource(GESEMID, pGECtxLocal->u32GESEMID) == FALSE)
        {
            GE_HWSemaphore_count++;
            if(GE_HWSemaphore_count>10000)
            {
                MDrv_SEM_Get_ResourceID(GESEMID, &GE_GetHWSemaphoreID);
                GE_D_DBUG("\33[0;31m [Entry While][%s][%d]GE_GetHWSemphoreID=%x; pGECtxLocal->u32GESEMID=%x \33[m;\n",__FUNCTION__,__LINE__,GE_GetHWSemaphoreID, pGECtxLocal->u32GESEMID);
                GE_HWSemaphore_count =0;
            }
        }
        if(MDrv_Seal_SecureSlaveSet(E_SEAL_GE0_PROT_NONPM,TRUE) == FALSE)
        {
            GE_D_ERR("[%s][%d] Assert ! MDrv_Seal_SecureSlaveSet GE 0 Fail.\n",__func__,__LINE__);
            return FALSE;
        }
        if(MDrv_Seal_SecureSlaveSet(E_SEAL_GE1_PROT_NONPM,TRUE) == FALSE)
        {
            GE_D_ERR("[%s][%d] Assert ! MDrv_Seal_SecureSlaveSet GE1 Fail.\n",__func__,__LINE__);
            return FALSE;
        }
        MS_U16 u16Tag = 0 , i = 0;
        u16Tag = GE_GetNextTAGID(&pGECtxLocal->halLocalCtx, TRUE);
        for(i=0;i<(GE_WordUnit>>2);i++)
        {
            GE_WriteReg(&pGECtxLocal->halLocalCtx, REG_GE_TAG, u16Tag);
        }
        GE_WaitTAGID(&pGECtxLocal->halLocalCtx, u16Tag);
        GE_WaitIdle(&pGECtxLocal->halLocalCtx);
        pGECtxLocal->u16RegCfg = GE_ReadReg(&pGECtxLocal->halLocalCtx,(REG_GE_CFG+GE_TABLE_REGNUM));
#ifdef GE_VQ_COUNT_PATCH
        GE_WriteReg(&pGECtxLocal->halLocalCtx, REG_GE_CFG, pGECtxLocal->u16RegCfg & ~(GE_CFG_VCMDQ | GE_CFG_CMDQ));
        GE_WriteReg(&pGECtxLocal->halLocalCtx, REG_GE_CFG, (GE_ReadReg(&pGECtxLocal->halLocalCtx,REG_GE_CFG) | GE_CFG_CMDQ));
#else
        GE_WriteReg(&pGECtxLocal->halLocalCtx, REG_GE_CFG, pGECtxLocal->u16RegCfg & (~GE_CFG_VCMDQ));
#endif
        if(MDrv_Seal_SecureMasterSet(E_SEAL_GE0_PROT_NONPM,TRUE) == FALSE)
        {
            GE_D_ERR("[%s][%d] Assert ! MDrv_Seal_SecureMasterSet GE0 Fail.\n",__func__,__LINE__);
            return FALSE;
        }
        if(MDrv_Seal_SecureMasterSet(E_SEAL_GE1_PROT_NONPM,TRUE) == FALSE)
        {
            GE_D_ERR("[%s][%d] Assert ! MDrv_Seal_SecureMasterSet GE1 Fail.\n",__func__,__LINE__);
            return FALSE;
        }
        pGECtxLocal->s32GE_Recursive_Lock_Cnt++;
        if(pGECtxLocal->pSharedCtx->u32LstGEClientId!=pGECtxLocal->u32GEClientId)
        {
            GE_Restore_HAL_Context(&pGECtxLocal->halLocalCtx,pGECtxLocal->pSharedCtx->bNotFirstInit);
            pGECtxLocal->pSharedCtx->u32LstGEClientId = pGECtxLocal->u32GEClientId;
        }
    }
    return TRUE;
}
static MS_BOOL _OPTEE_GE_RETURN(GE_Context *pGECtx,MS_BOOL bHWSemLock)
{
    GE_CTX_LOCAL *pGECtxLocal = (GE_CTX_LOCAL*)pGECtx;
    GE_ASSERT_CHECK((0<pGECtxLocal->s32GE_Recursive_Lock_Cnt), (GE_D_ERR("\nGE_RETURN: Fatal Error, No Mutex to release[Cnt=%d]\n", pGECtxLocal->s32GE_Recursive_Lock_Cnt)));
    if(pGECtxLocal->s32GE_Recursive_Lock_Cnt > 1)
    {
        pGECtxLocal->s32GE_Recursive_Lock_Cnt--;
    }
    else
    {
        pGECtxLocal->s32GE_Recursive_Lock_Cnt--;
        MS_U16 u16Tag = 0 , i = 0;
        u16Tag = GE_GetNextTAGID(&pGECtxLocal->halLocalCtx, TRUE);
        for(i=0;i<(GE_WordUnit>>2);i++)
        {
            GE_WriteReg(&pGECtxLocal->halLocalCtx, REG_GE_TAG, u16Tag);
        }
        GE_WaitTAGID(&pGECtxLocal->halLocalCtx, u16Tag);
        GE_WaitIdle(&pGECtxLocal->halLocalCtx);
        if(MDrv_Seal_SecureMasterSet(E_SEAL_GE0_PROT_NONPM,FALSE) == FALSE)
        {
            GE_D_ERR("[%s][%d] Assert ! MDrv_Seal_SecureMasterSet GE0 Fail.\n",__func__,__LINE__);
            return FALSE;
        }
        if(MDrv_Seal_SecureMasterSet(E_SEAL_GE1_PROT_NONPM,FALSE) == FALSE)
        {
            GE_D_ERR("[%s][%d] Assert ! MDrv_Seal_SecureMasterSet GE1 Fail.\n",__func__,__LINE__);
            return FALSE;
        }
        GE_WriteReg(&pGECtxLocal->halLocalCtx, REG_GE_CFG,pGECtxLocal->u16RegCfg);
        if(MDrv_Seal_SecureSlaveSet(E_SEAL_GE0_PROT_NONPM,FALSE) == FALSE)
        {
            GE_D_ERR("[%s][%d] Assert ! MDrv_Seal_SecureSlaveSet GE0 Fail.\n",__func__,__LINE__);
            return FALSE;
        }
        if(MDrv_Seal_SecureSlaveSet(E_SEAL_GE1_PROT_NONPM,FALSE) == FALSE)
        {
            GE_D_ERR("[%s][%d] Assert ! MDrv_Seal_SecureSlaveSet GE1 Fail.\n",__func__,__LINE__);
            return FALSE;
        }
        while(MDrv_SEM_Free_Resource(GESEMID, pGECtxLocal->u32GESEMID) ==FALSE)
        {
            GE_HWSemaphore_count++;
            if(GE_HWSemaphore_count>10000)
            {
                MDrv_SEM_Get_ResourceID(GESEMID, &GE_GetHWSemaphoreID);
                GE_D_DBUG("\33[0;31m [Return While][%s][%d] GE_GetHWSemphoreID=%x; u32GESEMID=%x\33[m;\n",__FUNCTION__,__LINE__,GE_GetHWSemaphoreID,pGECtxLocal->u32GESEMID);
                GE_HWSemaphore_count =0;
            }
        }
    }
    return TRUE;
}
#endif
static void GE_BeginDraw(GE_Context *pGECtx,MS_BOOL bHWSemLock)
{
#if (GE_USE_HW_SEM && defined(MSOS_TYPE_OPTEE))
    if(_OPTEE_GE_ENTRY(pGECtx,bHWSemLock) == FALSE)
    {
        GE_D_ERR("[%s][%d] _OPTEE_GE_ENTRY Fail.\n",__func__,__LINE__);
    }
#else
    //DRV_GE_ENTRY(pGECtx,bHWSemLock);
#endif
}

static void GE_EndDraw(GE_Context *pGECtx,MS_BOOL bHWSemLock)
{
#if (GE_USE_HW_SEM && defined(MSOS_TYPE_OPTEE))
    if(_OPTEE_GE_RETURN(pGECtx,bHWSemLock) == FALSE)
    {
        GE_D_ERR("[%s][%d] _OPTEE_GE_RETURN Fail.\n",__func__,__LINE__);
    }
#else
    //DRV_GE_RETURN(pGECtx,bHWSemLock);
#endif
}

static MS_BOOL GE_FastClip(GE_Context *pGECtx, MS_U16 x0, MS_U16 y0, MS_U16 x1, MS_U16 y1)
{
    GE_CTX_LOCAL *pGECtxLocal = (GE_CTX_LOCAL*)pGECtx;
    if ( ((x0 < pGECtxLocal->u16ClipL) && (x1 < pGECtxLocal->u16ClipL)) ||
         ((x0 > pGECtxLocal->u16ClipR) && (x1 > pGECtxLocal->u16ClipR)) ||
         ((y0 < pGECtxLocal->u16ClipT) && (y1 < pGECtxLocal->u16ClipT)) ||
         ((y0 > pGECtxLocal->u16ClipB) && (y1 > pGECtxLocal->u16ClipB))    )
    {
        return TRUE; // can't be clipped
    }
    return FALSE;
}

#ifdef CONFIG_GFX_TRAPZOID
static MS_BOOL GE_FastClipTrapezoid(GE_Context *pGECtx, MS_U16 u16X0, MS_U16 u16Y0, MS_U16 u16X1, MS_U16 u16Y1, MS_U16 u16DeltaTop, MS_U16 u16DeltaBottom, MS_BOOL bYTrapezoid)
{
    GE_CTX_LOCAL *pGECtxLocal = (GE_CTX_LOCAL*)pGECtx;
    MS_U16 u16MinusX, u16MaxX, u16MinusY, u16MaxY;

    if(bYTrapezoid)
    {
        u16MinusX = u16X1;
        u16MaxX   = u16X0;
        u16MinusY = MIN(u16Y0, u16Y1);
        u16MaxY   = MAX((u16Y0+u16DeltaTop),(u16Y1+u16DeltaBottom));
    }
    else
    {
        u16MinusY = u16Y0;
        u16MaxY   = u16Y1;
        u16MinusX = MIN(u16X0, u16X1);
        u16MaxX   = MAX((u16X0+u16DeltaTop),(u16X1+u16DeltaBottom));
    }

    if((u16MaxX<pGECtxLocal->u16ClipL) ||
        (u16MinusX>pGECtxLocal->u16ClipR) ||
        (u16MaxY<pGECtxLocal->u16ClipT) ||
        (u16MinusY>pGECtxLocal->u16ClipB))
    {
        return TRUE; // can't be clipped
    }

    return FALSE;
}
#endif

#ifdef CONFIG_GFX_TRAPZOID
static MS_BOOL GE_TrapezoidOverlap(GE_Rect *pGeRect, GE_Trapezoid *pGeTrapezoid)
{
    MS_U16 u16DeltaX, u16DeltaY;
    MS_U16 u16MinorX, u16MajorX, u16MinorY, u16MajorY;
    MS_U16 u16CheckMinorX0, u16CheckMinorX1, u16CheckMajorX0, u16CheckMajorX1;

    u16MinorX = (pGeTrapezoid->u16X0 >= pGeTrapezoid->u16X1) ? pGeTrapezoid->u16X1 : pGeTrapezoid->u16X0;
    u16MajorX = (pGeTrapezoid->u16X0 >= pGeTrapezoid->u16X1) ? pGeTrapezoid->u16X0 : pGeTrapezoid->u16X1;
    u16MinorY = (pGeTrapezoid->u16Y0 >= pGeTrapezoid->u16Y1) ? pGeTrapezoid->u16Y1 : pGeTrapezoid->u16Y0;
    u16MajorY = (pGeTrapezoid->u16Y0 >= pGeTrapezoid->u16Y1) ? pGeTrapezoid->u16Y0 : pGeTrapezoid->u16Y1;

    //check Y:
    if((pGeRect->y > u16MajorY) || ((pGeRect->y+pGeRect->height-1) < u16MajorY))
    {
        return FALSE; // no overlap
    }

    u16CheckMinorX0 = u16CheckMinorX1 = u16CheckMajorX0 = u16CheckMajorX1 = 0;

    //check X:
    u16DeltaX = u16MajorX - u16MinorX;
    u16DeltaY = u16MajorY - u16MinorY;

    //Get u16CheckMinorX0, X1:
    if((pGeRect->y >= u16MinorY) && (pGeRect->y <= u16MajorY))
    {
        u16CheckMinorX0 = (pGeTrapezoid->u16Y0 == u16MinorY) ? pGeTrapezoid->u16X0 : pGeTrapezoid->u16X1;
        if(u16CheckMinorX0 == u16MinorX)
        {
            u16CheckMinorX0 += ((u16DeltaX/u16DeltaY) * (pGeRect->y-u16MinorY));
        }
        else
        {
            u16CheckMinorX0 -= ((u16DeltaX/u16DeltaY) * (pGeRect->y-u16MinorY));
        }
    }

    if(((pGeRect->y+pGeRect->height-1) >= u16MinorY) && ((pGeRect->y+pGeRect->height-1) <= u16MajorY))
    {
        u16CheckMinorX1 = (pGeTrapezoid->u16Y0 == u16MinorY) ? pGeTrapezoid->u16X0 : pGeTrapezoid->u16X1;
        if(u16CheckMinorX1 == u16MinorX)
        {
            u16CheckMinorX1 += ((u16DeltaX/u16DeltaY) * (pGeRect->y+pGeRect->height-1-u16MinorY));
        }
        else
        {
            u16CheckMinorX1 -= ((u16DeltaX/u16DeltaY) * (pGeRect->y+pGeRect->height-1-u16MinorY));
        }
    }

    //Get MajorX0, MajorX1:
    u16MinorX = ((pGeTrapezoid->u16X0+pGeTrapezoid->u16DeltaTop)>= (pGeTrapezoid->u16X1+pGeTrapezoid->u16DeltaBottom)) ? (pGeTrapezoid->u16X1+pGeTrapezoid->u16DeltaBottom) : (pGeTrapezoid->u16X0+pGeTrapezoid->u16DeltaTop);
    u16MajorX = ((pGeTrapezoid->u16X0+pGeTrapezoid->u16DeltaTop)>= (pGeTrapezoid->u16X1+pGeTrapezoid->u16DeltaBottom)) ? (pGeTrapezoid->u16X0+pGeTrapezoid->u16DeltaTop) : (pGeTrapezoid->u16X1+pGeTrapezoid->u16DeltaBottom);
    u16DeltaX = u16MajorX - u16MinorX;

    if((pGeRect->y >= u16MinorY) && (pGeRect->y <= u16MajorY))
    {
        u16CheckMajorX0 = (pGeTrapezoid->u16Y0 == u16MinorY) ? (pGeTrapezoid->u16X0+pGeTrapezoid->u16DeltaTop): (pGeTrapezoid->u16X1+pGeTrapezoid->u16DeltaBottom);
        if(u16CheckMajorX0 == u16MinorX)
        {
            u16CheckMajorX0 += ((u16DeltaX/u16DeltaY) * (pGeRect->y-u16MinorY));
        }
        else
        {
            u16CheckMajorX0 -= ((u16DeltaX/u16DeltaY) * (pGeRect->y-u16MinorY));
        }
    }

    if(((pGeRect->y+pGeRect->height-1) >= u16MinorY) && ((pGeRect->y+pGeRect->height-1) <= u16MajorY))
    {
        u16CheckMajorX1 = (pGeTrapezoid->u16Y0 == u16MinorY) ? (pGeTrapezoid->u16X0+pGeTrapezoid->u16DeltaTop) : (pGeTrapezoid->u16X1+pGeTrapezoid->u16DeltaBottom);
        if(u16CheckMajorX1 == u16MinorX)
        {
            u16CheckMajorX1 += ((u16DeltaX/u16DeltaY) * (pGeRect->y+pGeRect->height-1-u16MinorY));
        }
        else
        {
            u16CheckMajorX1 -= ((u16DeltaX/u16DeltaY) * (pGeRect->y+pGeRect->height-1-u16MinorY));
        }
    }

    if((u16CheckMinorX0!=0) || (u16CheckMajorX0!=0))
    {
        if((pGeRect->x > u16CheckMajorX0) || ((pGeRect->x+pGeRect->width-1)<u16CheckMinorX0))
        {
            return FALSE;
        }
    }

    if((u16CheckMinorX1!=0) || (u16CheckMajorX1!=0))
    {
        if((pGeRect->x > u16CheckMajorX1) || ((pGeRect->x+pGeRect->width-1)<u16CheckMinorX1))
        {
            return FALSE;
        }
    }

    return TRUE;
}
#endif

static MS_U16 GE_GetFmt(GE_BufFmt fmt)
{
    switch(fmt)
    {
        case E_GE_FMT_I1:
                return GE_FMT_I1;
              break;
        case E_GE_FMT_I2:
                return GE_FMT_I2;
              break;
        case E_GE_FMT_I4:
                return GE_FMT_I4;
              break;
        case E_GE_FMT_I8:
                return GE_FMT_I8;
              break;
        case E_GE_FMT_RGB332:
                return E_GE_FMT_RGB332;
              break;
        case E_GE_FMT_FaBaFgBg2266:
                return GE_FMT_FaBaFgBg2266;
              break;
        case E_GE_FMT_1ABFgBg12355:
                return GE_FMT_1ABFgBg12355;
              break;
        case E_GE_FMT_RGB565:
                return GE_FMT_RGB565;
              break;
        case E_GE_FMT_ARGB1555:
                return GE_FMT_ARGB1555;
              break;
        case E_GE_FMT_ARGB4444:
                return GE_FMT_ARGB4444;
              break;
        case E_GE_FMT_ARGB1555_DST:
                return GE_FMT_ARGB1555_DST;
              break;
        case E_GE_FMT_YUV422:
                return GE_FMT_YUV422;
              break;
        case E_GE_FMT_ARGB8888:
                return GE_FMT_ARGB8888;
              break;
        case E_GE_FMT_RGBA5551:
                return GE_FMT_RGBA5551;
              break;
        case E_GE_FMT_RGBA4444:
                return GE_FMT_RGBA4444;
              break;
        case E_GE_FMT_ABGR8888:
                return GE_FMT_ABGR8888;
              break;
        case E_GE_FMT_ABGR1555:
                return GE_FMT_ABGR1555;
              break;
        case E_GE_FMT_BGRA5551:
                return GE_FMT_BGRA5551;
              break;
        case E_GE_FMT_ABGR4444:
                return GE_FMT_ABGR4444;
              break;
        case E_GE_FMT_BGRA4444:
                return GE_FMT_BGRA4444;
              break;
        case E_GE_FMT_BGR565:
                return GE_FMT_BGR565;
              break;
        case E_GE_FMT_RGBA8888:
                return GE_FMT_RGBA8888;
              break;
        case E_GE_FMT_BGRA8888:
                return GE_FMT_BGRA8888;
              break;
        case E_GE_FMT_ACRYCB444:
                return E_GE_FMT_ACRYCB444;
              break;
        case E_GE_FMT_CRYCBA444:
                return E_GE_FMT_CRYCBA444;
              break;
        case E_GE_FMT_ACBYCR444:
                return E_GE_FMT_ACBYCR444;
              break;
        case E_GE_FMT_CBYCRA444:
                return E_GE_FMT_CBYCRA444;
              break;
        default:
                return GE_FMT_GENERIC;
             break;
    }
}

#ifdef GE_VQ_MIU_HANG_PATCH
static MS_BOOL GE_IsVcmdqEnabled(GE_Context *pGECtx)
{
    GE_CTX_LOCAL *pGECtxLocal = (GE_CTX_LOCAL*)pGECtx;
    MS_U16 u16tmp = 0x0;
    MS_BOOL bEnable = TRUE;

    u16tmp = GE_ReadReg(&pGECtxLocal->halLocalCtx, REG_GE_CFG);

    bEnable = ((u16tmp & GE_CFG_VCMDQ) > 0) ? TRUE : FALSE;

    return bEnable;
}
#endif

MS_U32 GE_Divide2Fixed(MS_U16 u16x, MS_U16 u16y, MS_U8 nInteger, MS_U8 nFraction)
{
    MS_U8  neg = 0;
    MS_U32 mask;
    MS_U32 u32x;
    MS_U32 u32y;

    if (u16x == 0)
    {
        return 0;
    }

    if (u16x & 0x8000)
    {
        u32x = 0xFFFF0000 | u16x;
        u32x = ~u32x + 1; //convert to positive
        neg++;
    }
    else
    {
        u32x = u16x;
    }

    if (u16y & 0x8000)
    {
        u32y = 0xFFFF0000 | u16y;
        u32y = ~u32y + 1; //convert to positive
        neg++;
    }
    else
    {
        u32y = u16y;
    }

    // start calculation
    u32x = (u32x << nFraction) / u32y;
    if (neg % 2)
    {
        u32x = ~u32x + 1;
    }
    // total bit number is: 1(s) + nInteger + nFraction
    mask = (1 << (1 + nInteger + nFraction)) - 1;
    u32x &= mask;

    return u32x;
}

#ifdef CONFIG_GFX_TRAPZOID
static void GE_NormalizeTrapezoidDef(GE_Trapezoid *pGeTrapezoid, GE_Normalized_Trapezoid *pGeNormTrapezoid, MS_BOOL bYTrapezoid)
{
    MS_U16 u16DeltaR, u16DeltaM; //Responsible Var and Main Var of Delta Calculation.
    MS_BOOL bMinus;

    //Get Top-Left Point:
    pGeNormTrapezoid->u16X0 = pGeTrapezoid->u16X0;
    pGeNormTrapezoid->u16Y0 = pGeTrapezoid->u16Y0;
    //Get Top-Right Edge and Bottom Edge X/Y:
    if(bYTrapezoid)
    {
        pGeNormTrapezoid->u16X1 = pGeTrapezoid->u16X1;
        pGeNormTrapezoid->u16Y1 = pGeTrapezoid->u16Y0+pGeTrapezoid->u16DeltaTop;
    }
    else
    {
        pGeNormTrapezoid->u16X1 = pGeTrapezoid->u16X0+pGeTrapezoid->u16DeltaTop;
        pGeNormTrapezoid->u16Y1 = pGeTrapezoid->u16Y1;
    }

    //Get DeltaL(s5.12):
    if(bYTrapezoid)
    {
        bMinus = (pGeTrapezoid->u16Y0 > pGeTrapezoid->u16Y1);
        u16DeltaR = bMinus ? (pGeTrapezoid->u16Y0 - pGeTrapezoid->u16Y1) : (pGeTrapezoid->u16Y1 - pGeTrapezoid->u16Y0);
        u16DeltaM = (pGeTrapezoid->u16X0 - pGeTrapezoid->u16X1);
    }
    else
    {
        bMinus = (pGeTrapezoid->u16X0 > pGeTrapezoid->u16X1);
        u16DeltaR = bMinus ? (pGeTrapezoid->u16X0 - pGeTrapezoid->u16X1) : (pGeTrapezoid->u16X1 - pGeTrapezoid->u16X0);
        u16DeltaM = (pGeTrapezoid->u16Y1 - pGeTrapezoid->u16Y0);
    }

    pGeNormTrapezoid->u32DeltaL = GE_Divide2Fixed(u16DeltaR, u16DeltaM, 5, 12); // s5.12

    if(bMinus)
    {
        pGeNormTrapezoid->u32DeltaL = (MS_U32)(1<<(6+12)) - pGeNormTrapezoid->u32DeltaL; //BIT(17)~bIT(0) valid.
    }

    //Get DeltaR of Trapezoid (s5.12):
    if(bYTrapezoid)
    {
        bMinus = ((pGeTrapezoid->u16Y0+pGeTrapezoid->u16DeltaTop) > (pGeTrapezoid->u16Y1+pGeTrapezoid->u16DeltaBottom));
        u16DeltaR = bMinus ? ((pGeTrapezoid->u16Y0+pGeTrapezoid->u16DeltaTop)-(pGeTrapezoid->u16Y1+pGeTrapezoid->u16DeltaBottom)) : ((pGeTrapezoid->u16Y1+pGeTrapezoid->u16DeltaBottom) - (pGeTrapezoid->u16Y0+pGeTrapezoid->u16DeltaTop));
        u16DeltaM = (pGeTrapezoid->u16X0 - pGeTrapezoid->u16X1);
    }
    else
    {
        bMinus = ((pGeTrapezoid->u16X0+pGeTrapezoid->u16DeltaTop) > (pGeTrapezoid->u16X1+pGeTrapezoid->u16DeltaBottom));
        u16DeltaR = bMinus ? ((pGeTrapezoid->u16X0+pGeTrapezoid->u16DeltaTop) - (pGeTrapezoid->u16X1+pGeTrapezoid->u16DeltaBottom)) : ((pGeTrapezoid->u16X1+pGeTrapezoid->u16DeltaBottom) - (pGeTrapezoid->u16X0+pGeTrapezoid->u16DeltaTop));
        u16DeltaM = (pGeTrapezoid->u16Y1 - pGeTrapezoid->u16Y0);
    }

    pGeNormTrapezoid->u32DeltaR = GE_Divide2Fixed(u16DeltaR, u16DeltaM, 5, 12); // s5.12

    if(bMinus)
    {
        pGeNormTrapezoid->u32DeltaR = (MS_U32)(1<<(6+12)) - pGeNormTrapezoid->u32DeltaR; //BIT(17)~bIT(0) valid.
    }
}
#endif

static GE_Result GE_Restore_PaletteContext(GE_CTX_LOCAL *pGECtxLocal)
{
    if((TRUE == pGECtxLocal->bSrcPaletteOn) /*|| (TRUE == pGECtxLocal->bDstPaletteOn)*/)
    { //Palette on in this KickOff, Need Restore Palette Context if need:
        if( (pGECtxLocal->u32GESEMID!=pGECtxLocal->u16GEPrevSEMID)
            || (TRUE == pGECtxLocal->halLocalCtx.bPaletteDirty)
            || ((pGECtxLocal->pSharedCtx->u32LstGEPaletteClientId!=pGECtxLocal->u32GEClientId) && pGECtxLocal->pSharedCtx->u32LstGEPaletteClientId) )
        { //Need restore palette:
            GE_SetPalette(&pGECtxLocal->halLocalCtx);
            pGECtxLocal->pSharedCtx->u32LstGEPaletteClientId = pGECtxLocal->u32GEClientId;
            pGECtxLocal->halLocalCtx.bPaletteDirty = FALSE;
            pGECtxLocal->bSrcPaletteOn = FALSE;
        }
    }

    return (E_GE_OK);
}
#ifdef CONFIG_GFX_TRAPZOID
static GE_Result GE_TrapezoidBlitEx(GE_CTX_LOCAL *pGECtxLocal, GE_TrapeBatchBlitObj *pTrapeBatchBlitObj)
{
    MS_U32 u32BlitIdx;
    GE_Rect geBlitSrc;
    GE_DstBitBltType geBlitDst;
    MS_S32 s32FixSrcX, s32FixSrcY;
    MS_S32 s32FixDstX0, s32FixDstY0, s32FixDstX1, s32FixDstY1;

    //Prepare Fix Type Src
    s32FixSrcX = INT_TO_FIX32(pTrapeBatchBlitObj->u32SrcX);
    s32FixSrcY = INT_TO_FIX32(pTrapeBatchBlitObj->u32SrcY);
    //Prepare Fix Type Dst
    s32FixDstX0 = INT_TO_FIX32(pTrapeBatchBlitObj->u32DstX0);
    s32FixDstY0 = INT_TO_FIX32(pTrapeBatchBlitObj->u32DstY0);
    s32FixDstX1 = INT_TO_FIX32(pTrapeBatchBlitObj->u32DstX1);
    s32FixDstY1 = INT_TO_FIX32(pTrapeBatchBlitObj->u32DstY1);

    for(u32BlitIdx=0; u32BlitIdx<pTrapeBatchBlitObj->u32TrapeHeight; u32BlitIdx++)
    {
        //Prepare Src:
        geBlitSrc.x = FIX32_TO_INT(s32FixSrcX);
        geBlitSrc.y = FIX32_TO_INT(s32FixSrcY);
        geBlitSrc.width = pTrapeBatchBlitObj->u32SrcW;
        geBlitSrc.height = pTrapeBatchBlitObj->u32SrcH;
        //Prepare Dst:
        geBlitDst.dstblk.x = FIX32_TO_INT(s32FixDstX0);
        geBlitDst.dstblk.y = FIX32_TO_INT(s32FixDstY0);
        geBlitDst.dstblk.width = FIX32_TO_INT(s32FixDstX1) - geBlitDst.dstblk.x + 1;
        geBlitDst.dstblk.height = FIX32_TO_INT(s32FixDstY1) - geBlitDst.dstblk.y + 1;
        //BitBlit:
        MDrv_GE_BitBltEX((GE_Context *)pGECtxLocal, &geBlitSrc, &geBlitDst, pTrapeBatchBlitObj->u32BlitFlags, NULL);

        //Delta change of Src:
        s32FixSrcX += pTrapeBatchBlitObj->s32DeltaSrcX;
        s32FixSrcY += pTrapeBatchBlitObj->s32DeltaSrcY;
        //Delta change of Dst:
        s32FixDstX0 += pTrapeBatchBlitObj->s32DeltaDstX0;
        s32FixDstY0 += pTrapeBatchBlitObj->s32DeltaDstY0;
        s32FixDstX1 += pTrapeBatchBlitObj->s32DeltaDstX1;
        s32FixDstY1 += pTrapeBatchBlitObj->s32DeltaDstY1;
    }

    return (E_GE_OK);
}

static GE_Result GE_TrapezoidBlit(GE_CTX_LOCAL *pGECtxLocal, GE_Rect *pSrcRect, GE_Normalized_Trapezoid *pGENormTrapezoid, MS_U32 u32Flags, GE_ScaleInfo* pScaleinfo)
{
    GE_TrapeBatchBlitObj geTrapeBatchBlitObj;
    MS_BOOL bYTrapezoid = (u32Flags & E_GE_FLAG_TRAPEZOID_Y) ? TRUE : FALSE;

    geTrapeBatchBlitObj.u32BlitFlags = (u32Flags & (E_GE_FLAG_BLT_STRETCH | E_GE_FLAG_STRETCH_NEAREST | E_GE_FLAG_BLT_ITALIC));

    //Prepare & Translate Trapezoid BatchBlit Parames:
    if(bYTrapezoid)
    {
        //Trapezoid Height:
        geTrapeBatchBlitObj.u32TrapeHeight = pGENormTrapezoid->u16X0 - pGENormTrapezoid->u16X1 + 1;
        //Prepare Flag:
        geTrapeBatchBlitObj.u32BlitFlags |= (u32Flags & E_GE_FLAG_BLT_MIRROR_V);
        //Prepare Src:
        geTrapeBatchBlitObj.u32SrcX = (u32Flags & E_GE_FLAG_BLT_MIRROR_H) ? pSrcRect->x : (pSrcRect->x+pSrcRect->width);
        geTrapeBatchBlitObj.u32SrcY = (u32Flags & E_GE_FLAG_BLT_MIRROR_V) ? (pSrcRect->y+pSrcRect->height-1) : pSrcRect->y;
        geTrapeBatchBlitObj.u32SrcW = 0x1;
        geTrapeBatchBlitObj.u32SrcH = pSrcRect->height;
        geTrapeBatchBlitObj.s32DeltaSrcX = (u32Flags & E_GE_FLAG_BLT_MIRROR_H) ? pScaleinfo->x : (-(pScaleinfo->x));
        geTrapeBatchBlitObj.s32DeltaSrcY = 0x0;

        //Prepare Dst:
        geTrapeBatchBlitObj.u32DstX0 = pGENormTrapezoid->u16X0;
        geTrapeBatchBlitObj.u32DstY0 = pGENormTrapezoid->u16Y0;
        geTrapeBatchBlitObj.u32DstX1 = pGENormTrapezoid->u16X0;
        geTrapeBatchBlitObj.u32DstY1 = pGENormTrapezoid->u16Y1;
        geTrapeBatchBlitObj.s32DeltaDstX0 = -(INT_TO_FIX32(0x1));
        geTrapeBatchBlitObj.s32DeltaDstY0 = (pGENormTrapezoid->u32DeltaL & (0x1<<(5+12))) ? (-((MS_S32)((MS_U32)((0x1<<(6+12))-pGENormTrapezoid->u32DeltaL)))) : (MS_S32)(pGENormTrapezoid->u32DeltaL);
        geTrapeBatchBlitObj.s32DeltaDstX1 = -(INT_TO_FIX32(0x1));
        geTrapeBatchBlitObj.s32DeltaDstY1 = (pGENormTrapezoid->u32DeltaR & (0x1<<(5+12))) ? (-((MS_S32)((MS_U32)((0x1<<(6+12))-pGENormTrapezoid->u32DeltaR)))) : (MS_S32)(pGENormTrapezoid->u32DeltaR);
    }
    else
    {
        //Trapezoid Height:
        geTrapeBatchBlitObj.u32TrapeHeight = pGENormTrapezoid->u16Y1 - pGENormTrapezoid->u16Y0 + 1;
        //Prepare Flag:
        geTrapeBatchBlitObj.u32BlitFlags |= (u32Flags & E_GE_FLAG_BLT_MIRROR_H);
        //Prepare Src:
        geTrapeBatchBlitObj.u32SrcX = (u32Flags & E_GE_FLAG_BLT_MIRROR_H) ? (pSrcRect->x+pSrcRect->width-1) : pSrcRect->x;
        geTrapeBatchBlitObj.u32SrcY = (u32Flags & E_GE_FLAG_BLT_MIRROR_V) ? (pSrcRect->y+pSrcRect->height) : pSrcRect->y;
        geTrapeBatchBlitObj.u32SrcW = pSrcRect->width;
        geTrapeBatchBlitObj.u32SrcH = 0x1;
        geTrapeBatchBlitObj.s32DeltaSrcX = 0x0;
        geTrapeBatchBlitObj.s32DeltaSrcY = (u32Flags & E_GE_FLAG_BLT_MIRROR_V) ? (-(MS_S32)pScaleinfo->y) : (MS_S32)pScaleinfo->y;
        //Prepare Dst:
        geTrapeBatchBlitObj.u32DstX0 = pGENormTrapezoid->u16X0;
        geTrapeBatchBlitObj.u32DstY0 = pGENormTrapezoid->u16Y0;
        geTrapeBatchBlitObj.u32DstX1 = pGENormTrapezoid->u16X1;
        geTrapeBatchBlitObj.u32DstY1 = pGENormTrapezoid->u16Y0;
        geTrapeBatchBlitObj.s32DeltaDstX0 = (pGENormTrapezoid->u32DeltaL & (0x1<<(5+12))) ? (-(MS_S32)((MS_U32)((0x1<<(6+12))-pGENormTrapezoid->u32DeltaL))) : (MS_S32)(pGENormTrapezoid->u32DeltaL);
        geTrapeBatchBlitObj.s32DeltaDstY0 = INT_TO_FIX32(0x1);
        geTrapeBatchBlitObj.s32DeltaDstX1 = (pGENormTrapezoid->u32DeltaR & (0x1<<(5+12))) ? (-(MS_S32)((MS_U32)((0x1<<(6+12))-pGENormTrapezoid->u32DeltaR))) : (MS_S32)(pGENormTrapezoid->u32DeltaR);
        geTrapeBatchBlitObj.s32DeltaDstY1 = INT_TO_FIX32(0x1);
    }

    return GE_TrapezoidBlitEx(pGECtxLocal, &geTrapeBatchBlitObj);
}

static GE_Result GE_TrapezoidFillEx(GE_CTX_LOCAL *pGECtxLocal, GE_TrapeBatchFillObj *pTrapeBatchFillObj)
{
    MS_U32 u32FillIdx;
    GE_Rect geFillDst;
    MS_S32 s32ColorB, s32ColorG, s32ColorR, s32ColorA;
    MS_S32 s32FixDstX0, s32FixDstY0, s32FixDstX1, s32FixDstY1;

    //Prepare Fix Type Src Color
    s32ColorB = INT_TO_FIX32((pTrapeBatchFillObj->u32ColorS & 0xFF));
    s32ColorG = INT_TO_FIX32(((pTrapeBatchFillObj->u32ColorS>>0x8) & 0xFF));
    s32ColorR = INT_TO_FIX32(((pTrapeBatchFillObj->u32ColorS>>0x10) & 0xFF));
    s32ColorA = INT_TO_FIX32(((pTrapeBatchFillObj->u32ColorS>>0x18) & 0xFF));

    //Prepare Fix Type Dst
    s32FixDstX0 = INT_TO_FIX32(pTrapeBatchFillObj->u32DstX0);
    s32FixDstY0 = INT_TO_FIX32(pTrapeBatchFillObj->u32DstY0);
    s32FixDstX1 = INT_TO_FIX32(pTrapeBatchFillObj->u32DstX1);
    s32FixDstY1 = INT_TO_FIX32(pTrapeBatchFillObj->u32DstY1);

    for(u32FillIdx=0; u32FillIdx<pTrapeBatchFillObj->u32TrapeHeight; u32FillIdx++)
    {
        //Prepare Src:
        pTrapeBatchFillObj->u32ColorS = ((FIX32_TO_INT(s32ColorB) & 0xFF)
                                          | ((FIX32_TO_INT(s32ColorG) & 0xFF)<<0x8)
                                          | ((FIX32_TO_INT(s32ColorR) & 0xFF)<<0x10)
                                          | ((FIX32_TO_INT(s32ColorA) & 0xFF)<<0x18));

        //Prepare Dst:
        geFillDst.x = FIX32_TO_INT(s32FixDstX0);
        geFillDst.y = FIX32_TO_INT(s32FixDstY0);
        geFillDst.width = FIX32_TO_INT(s32FixDstX1) - geFillDst.x + 1;
        geFillDst.height = FIX32_TO_INT(s32FixDstY1) - geFillDst.y + 1;

        //Fill Rect:
        MDrv_GE_FillRect((GE_Context *)pGECtxLocal, &geFillDst, pTrapeBatchFillObj->u32ColorS, pTrapeBatchFillObj->u32ColorE, pTrapeBatchFillObj->u32FillFlags);

        //Delta change of Src ColorS:
        s32ColorB += pTrapeBatchFillObj->s32ColorDeltaB;
        s32ColorG += pTrapeBatchFillObj->s32ColorDeltaG;
        s32ColorR += pTrapeBatchFillObj->s32ColorDeltaR;
        s32ColorA += pTrapeBatchFillObj->s32ColorDeltaA;

        //Delta change of Dst:
        s32FixDstX0 += pTrapeBatchFillObj->s32DeltaDstX0;
        s32FixDstY0 += pTrapeBatchFillObj->s32DeltaDstY0;
        s32FixDstX1 += pTrapeBatchFillObj->s32DeltaDstX1;
        s32FixDstY1 += pTrapeBatchFillObj->s32DeltaDstY1;
    }

    return (E_GE_OK);
}

static GE_Result GE_TrapezoidFill(GE_CTX_LOCAL *pGECtxLocal, MS_BOOL bYTrapezoid, GE_Normalized_Trapezoid *pGENormTrapezoid, MS_U32 u32ColorS, MS_U32 u32ColorE, GE_ColorDelta *pColorDeltaX, GE_ColorDelta *pColorDeltaY)
{
    GE_TrapeBatchFillObj geTrapeBatchFillObj;

    geTrapeBatchFillObj.u32FillFlags = 0x0;
    //Prepare Fill Color:
    geTrapeBatchFillObj.u32ColorS = u32ColorS;
    geTrapeBatchFillObj.u32ColorE = u32ColorE;

    //Prepare & Translate Trapezoid BatchBlit Parames:
    if(bYTrapezoid)
    {
        //Trapezoid Height:
        geTrapeBatchFillObj.u32TrapeHeight = pGENormTrapezoid->u16X0 - pGENormTrapezoid->u16X1 + 1;

        //Prepare Flag:
        if(NULL != pColorDeltaY)
        {
            geTrapeBatchFillObj.u32FillFlags = (E_GE_FLAG_RECT_GRADIENT_Y);
        }

        //Prepare Src Delta:
        if(NULL != pColorDeltaX)
        {
            geTrapeBatchFillObj.s32ColorDeltaB = (pColorDeltaX->b & (0x1<<(7+12))) ? (-(MS_S32)((MS_U32)((0x1<<(1+7+12))-pColorDeltaX->b))) : (MS_S32)(pColorDeltaX->b);
            geTrapeBatchFillObj.s32ColorDeltaG = (pColorDeltaX->g & (0x1<<(7+12))) ? (-(MS_S32)((MS_U32)((0x1<<(1+7+12))-pColorDeltaX->g))) : (MS_S32)(pColorDeltaX->g);
            geTrapeBatchFillObj.s32ColorDeltaR = (pColorDeltaX->r & (0x1<<(7+12))) ? (-(MS_S32)((MS_U32)((0x1<<(1+7+12))-pColorDeltaX->r))) : (MS_S32)(pColorDeltaX->r);
            geTrapeBatchFillObj.s32ColorDeltaA = (pColorDeltaX->a & (0x1<<(4+11))) ? (-(MS_S32)((MS_U32)((0x1<<(1+4+11))-pColorDeltaX->a))) : (MS_S32)(pColorDeltaX->a);
            geTrapeBatchFillObj.s32ColorDeltaA <<= 1;
        }
        else
        {
            geTrapeBatchFillObj.s32ColorDeltaB = geTrapeBatchFillObj.s32ColorDeltaG =
                geTrapeBatchFillObj.s32ColorDeltaR = geTrapeBatchFillObj.s32ColorDeltaA = 0x0;
        }

        //Prepare Dst:
        geTrapeBatchFillObj.u32DstX0 = pGENormTrapezoid->u16X0;
        geTrapeBatchFillObj.u32DstY0 = pGENormTrapezoid->u16Y0;
        geTrapeBatchFillObj.u32DstX1 = pGENormTrapezoid->u16X0;
        geTrapeBatchFillObj.u32DstY1 = pGENormTrapezoid->u16Y1;
        geTrapeBatchFillObj.s32DeltaDstX0 = -(INT_TO_FIX32(0x1));
        geTrapeBatchFillObj.s32DeltaDstY0 = (pGENormTrapezoid->u32DeltaL & (0x1<<(5+12))) ? (-(MS_S32)((MS_U32)((0x1<<(6+12))-pGENormTrapezoid->u32DeltaL))) : (MS_S32)(pGENormTrapezoid->u32DeltaL);
        geTrapeBatchFillObj.s32DeltaDstX1 = -(INT_TO_FIX32(0x1));
        geTrapeBatchFillObj.s32DeltaDstY1 = (pGENormTrapezoid->u32DeltaR & (0x1<<(5+12))) ? (-(MS_S32)((MS_U32)((0x1<<(6+12))-pGENormTrapezoid->u32DeltaR))) : (MS_S32)(pGENormTrapezoid->u32DeltaR);
    }
    else
    {
        //Trapezoid Height:
        geTrapeBatchFillObj.u32TrapeHeight = pGENormTrapezoid->u16Y1 - pGENormTrapezoid->u16Y0 + 1;
        //Prepare Flag:
        if(NULL != pColorDeltaX)
        {
            geTrapeBatchFillObj.u32FillFlags = (E_GE_FLAG_RECT_GRADIENT_X);
        }
        //Prepare Src Delta:
        if(NULL != pColorDeltaY)
        {
            geTrapeBatchFillObj.s32ColorDeltaB = (pColorDeltaY->b & (0x1<<(7+12))) ? (-(MS_S32)((MS_U32)((0x1<<(1+7+12))-pColorDeltaY->b))) : (MS_S32)(pColorDeltaY->b);
            geTrapeBatchFillObj.s32ColorDeltaG = (pColorDeltaY->g & (0x1<<(7+12))) ? (-(MS_S32)((MS_U32)((0x1<<(1+7+12))-pColorDeltaY->g))) : (MS_S32)(pColorDeltaY->g);
            geTrapeBatchFillObj.s32ColorDeltaR = (pColorDeltaY->r & (0x1<<(7+12))) ? (-(MS_S32)((MS_U32)((0x1<<(1+7+12))-pColorDeltaY->r))) : (MS_S32)(pColorDeltaY->r);
            geTrapeBatchFillObj.s32ColorDeltaA = (pColorDeltaY->a & (0x1<<(4+11))) ? (-(MS_S32)((MS_U32)((0x1<<(1+4+11))-pColorDeltaY->a))) : (MS_S32)(pColorDeltaY->a);
            geTrapeBatchFillObj.s32ColorDeltaA <<= 1;
        }
        else
        {
            geTrapeBatchFillObj.s32ColorDeltaB = geTrapeBatchFillObj.s32ColorDeltaG =
                geTrapeBatchFillObj.s32ColorDeltaR = geTrapeBatchFillObj.s32ColorDeltaA = 0x0;
        }

        //Prepare Dst:
        geTrapeBatchFillObj.u32DstX0 = pGENormTrapezoid->u16X0;
        geTrapeBatchFillObj.u32DstY0 = pGENormTrapezoid->u16Y0;
        geTrapeBatchFillObj.u32DstX1 = pGENormTrapezoid->u16X1;
        geTrapeBatchFillObj.u32DstY1 = pGENormTrapezoid->u16Y0;
        geTrapeBatchFillObj.s32DeltaDstX0 = (pGENormTrapezoid->u32DeltaL & (0x1<<(5+12))) ? (-(MS_S32)((MS_U32)((0x1<<(6+12))-pGENormTrapezoid->u32DeltaL))) : (MS_S32)(pGENormTrapezoid->u32DeltaL);
        geTrapeBatchFillObj.s32DeltaDstY0 = INT_TO_FIX32(0x1);
        geTrapeBatchFillObj.s32DeltaDstX1 = (pGENormTrapezoid->u32DeltaR & (0x1<<(5+12))) ? (-(MS_S32)((MS_U32)((0x1<<(6+12))-pGENormTrapezoid->u32DeltaR))) : (MS_S32)(pGENormTrapezoid->u32DeltaR);
        geTrapeBatchFillObj.s32DeltaDstY1 = INT_TO_FIX32(0x1);
    }

    return GE_TrapezoidFillEx(pGECtxLocal, &geTrapeBatchFillObj);
}
#endif
//-------------------------------------------------------------------------------------------------
//  Global Functions
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
/// Allocate semaphore resource for GE
/// @param  pGECtx                     \b IN: GE context
/// @return @ref GE_Result
//-------------------------------------------------------------------------------------------------
GE_Result GE_Get_Resource(GE_Context *pGECtx, MS_BOOL bHWSemLock)
{
    GE_BeginDraw(pGECtx,bHWSemLock);
    return E_GE_OK;
}

/******************************************************************************/
///To release GE control right from system for hk51
/******************************************************************************/
//-------------------------------------------------------------------------------------------------
/// Release semaphore resource occupied by GE
/// @param  pGECtx                     \b IN: GE context
/// @return @ref GE_Result
//-------------------------------------------------------------------------------------------------
GE_Result GE_Free_Resource(GE_Context *pGECtx, MS_BOOL bHWSemLock)
{
    GE_EndDraw(pGECtx,bHWSemLock);
    return E_GE_OK;
}


GE_Result MDrv_GE_Get_Semaphore(GE_Context *pGECtx, MS_U32 u32RPoolID)
{
#if GE_RESOURCE_SEM
    void*           pModule         = NULL;
    GE_CTX_LOCAL *pGECtxLocal = (GE_CTX_LOCAL*)pGECtx;

    if(pGECtxLocal->ctxHeader.bGEMode4MultiProcessAccess ==TRUE)
    {
            if(u32RPoolID >=E_GE_POOL_ID_MAX)
            {
                GE_D_ERR("[%s %d]Unkown GE Pool ID!",__FUNCTION__,__LINE__);
                return E_GE_FAIL;
            }

            UtopiaInstanceGetModule(pGECtxLocal->ctxHeader.pInstance, &pModule);

            /*In order to store resource addr of each instance*/
             if(UtopiaResourceObtain(pModule, E_GE_POOL_ID_INTERNAL_REGISTER, &g_pGFXResource[u32RPoolID]) != UTOPIA_STATUS_SUCCESS)
            {
                return E_GE_FAIL;
            }
        }
#endif
    return E_GE_OK;

}

GE_Result MDrv_GE_Free_Semaphore(GE_Context *pGECtx, MS_U32 u32RPoolID)
{
#if GE_RESOURCE_SEM
    GE_CTX_LOCAL *pGECtxLocal = (GE_CTX_LOCAL*)pGECtx;

    if(pGECtxLocal->ctxHeader.bGEMode4MultiProcessAccess ==TRUE)
    {
            if(u32RPoolID >=E_GE_POOL_ID_MAX)
            {
                GE_D_ERR("[%s %d]Unkown GE Pool ID!",__FUNCTION__,__LINE__);
                return E_GE_FAIL;
            }

            UtopiaResourceRelease(g_pGFXResource[u32RPoolID]);
        }
#endif
    return E_GE_OK;
}

//-------------------------------------------------------------------------------------------------
/// Reset GE setting
/// @param  pGECtx                     \b IN: GE context
/// @return @ref GE_Result
//-------------------------------------------------------------------------------------------------
GE_Result MDrv_GE_Reset(GE_Context *pGECtx)
{
    GE_CTX_LOCAL *pGECtxLocal = (GE_CTX_LOCAL*)pGECtx;

    GE_ENTRY(pGECtx);

    // Reset some GE state settings
    GE_ResetState(&pGECtxLocal->halLocalCtx);

    GE_RETURN(pGECtx,E_GE_OK);
}


//-------------------------------------------------------------------------------------------------
/// Set GE dither
/// @param  pGECtx                     \b IN: GE context
/// @param  enable                  \b IN: enable and update setting
/// @return @ref GE_Result
//-------------------------------------------------------------------------------------------------
GE_Result MDrv_GE_SetDither(GE_Context *pGECtx, MS_BOOL enable)
{
    GE_CTX_LOCAL *pGECtxLocal = (GE_CTX_LOCAL*)pGECtx;

    GE_DBG("%s\n", __FUNCTION__);
    GE_ENTRY(pGECtx);

    HAL_GE_SetDither(&pGECtxLocal->halLocalCtx, enable);

    GE_RETURN(pGECtx,E_GE_OK);
}
//-------------------------------------------------------------------------------------------------
/// Set GE One pixel Mode
/// @param  pGECtx                     \b IN: GE context
/// @param  enable                  \b IN: enable and update setting
/// @return @ref GE_Result
//-------------------------------------------------------------------------------------------------
GE_Result MDrv_GE_SetOnePixelMode(GE_Context *pGECtx, MS_BOOL enable)
{

    if ( pGECtx == NULL )
    {
        GE_D_ERR("\n%s, %d, 1st param. can't be null pointer\n", __FUNCTION__, __LINE__);
        return E_GE_FAIL;
    }

    GE_CTX_LOCAL *pGECtxLocal = (GE_CTX_LOCAL*)pGECtx;
    GE_Result geResult;

    GE_DBG("%s\n", __FUNCTION__);
    GE_ENTRY(pGECtx);

    geResult = GE_SetOnePixelMode(&pGECtxLocal->halLocalCtx, enable);

    GE_RETURN(pGECtx, geResult);
}


//-------------------------------------------------------------------------------------------------
/// Set GE source color key
/// @param  pGECtx                     \b IN: GE context
/// @param  enable                  \b IN: enable and update setting
/// @param  eCKOp                   \b IN: source color key mode
/// @param  ck_low                  \b IN: lower value of color key (E_GE_FMT_ARGB8888)
/// @param  ck_high                 \b IN: upper value of color key (E_GE_FMT_ARGB8888)
/// @return @ref GE_Result
/// @attention
/// <b>[URANUS] <em>color key does not check alpha channel</em></b>
//-------------------------------------------------------------------------------------------------
GE_Result MDrv_GE_SetSrcColorKey(GE_Context *pGECtx, MS_BOOL enable, GE_CKOp eCKOp, MS_U32 ck_low, MS_U32 ck_high)
{
    MS_U16              u16en=0, u16op;
    GE_CTX_LOCAL *pGECtxLocal = (GE_CTX_LOCAL*)pGECtx;

    GE_DBG("%s\n", __FUNCTION__);
    GE_ENTRY(pGECtx);

    u16op = GE_ReadReg(&pGECtxLocal->halLocalCtx, REG_GE_OP_MODE);

    switch(eCKOp)
    {
        case E_GE_CK_EQ:
            u16en = GE_EN_SCK;
            u16op &= ~(GE_BIT0);
            break;
        case E_GE_CK_NE:
            u16en = GE_EN_SCK;
            u16op |= GE_BIT0;
            break;
        case E_GE_ALPHA_EQ:
            u16en = GE_EN_ASCK;
            u16op &= ~(GE_BIT2);
            break;
        case E_GE_ALPHA_NE:
            u16en = GE_EN_ASCK;
            u16op |= GE_BIT2;
            break;
        default:
            break;
    }

    GE_WriteReg(&pGECtxLocal->halLocalCtx, REG_GE_OP_MODE, u16op);

    if (enable)
    {
        // Color key threshold
        GE_WriteReg(&pGECtxLocal->halLocalCtx, REG_GE_SCK_LTH_L, ck_low & 0xFFFF);
        GE_WriteReg(&pGECtxLocal->halLocalCtx, REG_GE_SCK_LTH_H, (ck_low >> 16));
        GE_WriteReg(&pGECtxLocal->halLocalCtx, REG_GE_SCK_HTH_L, (ck_high & 0xFFFF));
        GE_WriteReg(&pGECtxLocal->halLocalCtx, REG_GE_SCK_HTH_H, (ck_high >> 16));
        GE_WriteReg(&pGECtxLocal->halLocalCtx, REG_GE_EN, (GE_ReadReg(&pGECtxLocal->halLocalCtx, REG_GE_EN)|u16en));
    }
    else
    {
        GE_WriteReg(&pGECtxLocal->halLocalCtx, REG_GE_EN, (GE_ReadReg(&pGECtxLocal->halLocalCtx, REG_GE_EN)&(~u16en)));
    }

    GE_RETURN(pGECtx,E_GE_OK);
}


//-------------------------------------------------------------------------------------------------
/// Set GE destination color key
/// @param  pGECtx                     \b IN: GE context
/// @param  enable                  \b IN: enable and update setting
/// @param  eCKOp                   \b IN: source color key mode
/// @param  ck_low                  \b IN: lower value of color key (E_GE_FMT_ARGB8888)
/// @param  ck_high                 \b IN: upper value of color key (E_GE_FMT_ARGB8888)
/// @return @ref GE_Result
/// @attention
/// <b>[URANUS] <em>color key does not check alpha channel</em></b>
//-------------------------------------------------------------------------------------------------
GE_Result MDrv_GE_SetDstColorKey(GE_Context *pGECtx, MS_BOOL enable, GE_CKOp eCKOp, MS_U32 ck_low, MS_U32 ck_high)
{
    MS_U16              u16en=0, u16op;
    GE_CTX_LOCAL *pGECtxLocal = (GE_CTX_LOCAL*)pGECtx;

    GE_DBG("%s\n", __FUNCTION__);
    GE_ENTRY(pGECtx);

    u16op = GE_ReadReg(&pGECtxLocal->halLocalCtx, REG_GE_OP_MODE);

    switch(eCKOp)
    {
        case E_GE_CK_EQ:
            u16en = GE_EN_DCK;
            u16op &= ~(GE_BIT1);
            break;
        case E_GE_CK_NE:
            u16en = GE_EN_DCK;
            u16op |= GE_BIT1;
            break;
        case E_GE_ALPHA_EQ:
            u16en = GE_EN_DSCK;
            u16op &= ~(GE_BIT3);
            break;
        case E_GE_ALPHA_NE:
            u16en = GE_EN_DSCK;
            u16op |= GE_BIT3;
            break;
        default:
            break;
    }

    GE_WriteReg(&pGECtxLocal->halLocalCtx, REG_GE_OP_MODE, u16op);

    if (enable)
    {
        // Color key threshold
        GE_WriteReg(&pGECtxLocal->halLocalCtx, REG_GE_DCK_LTH_L, (ck_low & 0xFFFF));
        GE_WriteReg(&pGECtxLocal->halLocalCtx, REG_GE_DCK_LTH_H, (ck_high >> 16));
        GE_WriteReg(&pGECtxLocal->halLocalCtx, REG_GE_DCK_HTH_L, (ck_low & 0xFFFF));
        GE_WriteReg(&pGECtxLocal->halLocalCtx, REG_GE_DCK_HTH_H, (ck_high >> 16));
        GE_WriteReg(&pGECtxLocal->halLocalCtx, REG_GE_EN, (GE_ReadReg(&pGECtxLocal->halLocalCtx, REG_GE_EN)|u16en));
    }
    else
    {
        GE_WriteReg(&pGECtxLocal->halLocalCtx, REG_GE_EN, (GE_ReadReg(&pGECtxLocal->halLocalCtx, REG_GE_EN)&(~u16en)));
    }

    GE_RETURN(pGECtx,E_GE_OK);
}


//-------------------------------------------------------------------------------------------------
/// Set GE I1,I2,I4 color palette
/// @param  pGECtx                     \b IN: GE context
/// @param  idx                     \b IN: index of palette
/// @param  color                   \b IN: intensity color format (E_GE_FMT_ARGB8888, E_GE_FMT_I8)
/// @return @ref GE_Result
//-------------------------------------------------------------------------------------------------
GE_Result MDrv_GE_SetIntensity(GE_Context *pGECtx, MS_U32 idx, MS_U32 color)
{
    GE_CTX_LOCAL *pGECtxLocal = (GE_CTX_LOCAL*)pGECtx;

    GE_DBG("%s\n", __FUNCTION__);
    GE_ENTRY(pGECtx);

    if (idx < 16)
    {
        GE_WriteReg(&pGECtxLocal->halLocalCtx, REG_GE_C_L(idx), (color & 0xFFFF));
        GE_WriteReg(&pGECtxLocal->halLocalCtx, REG_GE_C_H(idx), (color >> 16));
    }
    else
    {
        GE_RETURN(pGECtx,E_GE_FAIL_PARAM);
    }

    GE_RETURN(pGECtx,E_GE_OK);
}
//-------------------------------------------------------------------------------------------------
/// Get GE I1,I2,I4 color palette
/// @param  pGECtx                     \b IN: GE context
/// @param  idx                     \b IN: index of palette
/// @param  color                   \b IN: intensity color format (E_GE_FMT_ARGB8888, E_GE_FMT_I8)
/// @return @ref GE_Result
//-------------------------------------------------------------------------------------------------
GE_Result MDrv_GE_GetIntensity(GE_Context *pGECtx, MS_U32 idx, MS_U32 *color)
{
    GE_CTX_LOCAL *pGECtxLocal = (GE_CTX_LOCAL*)pGECtx;

    GE_DBG("%s\n", __FUNCTION__);
    GE_ENTRY(pGECtx);

    if (idx < 16)
    {
        *color =((MS_U32)GE_ReadReg(&pGECtxLocal->halLocalCtx, REG_GE_C_H(idx)))<<16|(MS_U32)GE_ReadReg(&pGECtxLocal->halLocalCtx, REG_GE_C_L(idx));
    }
    else
    {
        GE_RETURN(pGECtx,E_GE_FAIL_PARAM);
    }

    GE_RETURN(pGECtx,E_GE_OK);
}


//-------------------------------------------------------------------------------------------------
/// Set GE ROP2
/// @param  pGECtx                     \b IN: GE context
/// @param  enable                  \b IN: enable and update setting
/// @param  eRop2                   \b IN: ROP2 op
/// @return @ref GE_Result
//-------------------------------------------------------------------------------------------------
GE_Result MDrv_GE_SetROP2(GE_Context *pGECtx, MS_BOOL enable, GE_Rop2 eRop2)
{
    MS_U16              u16en, u16rop2;
    GE_CTX_LOCAL *pGECtxLocal = (GE_CTX_LOCAL*)pGECtx;

    GE_DBG("%s\n", __FUNCTION__);
    GE_ENTRY(pGECtx);

    u16en = GE_ReadReg(&pGECtxLocal->halLocalCtx, REG_GE_EN);
    if (enable)
    {
        u16en |= GE_EN_ROP2;

        u16rop2 = (GE_ReadReg(&pGECtxLocal->halLocalCtx, _REG_GE_ROP2) & ~GE_ROP2_MASK) | eRop2;
        GE_WriteReg(&pGECtxLocal->halLocalCtx, _REG_GE_ROP2, u16rop2);
    }
    else
    {
        u16en &= ~GE_EN_ROP2;
    }
    GE_WriteReg(&pGECtxLocal->halLocalCtx, REG_GE_EN, u16en);

    GE_RETURN(pGECtx,E_GE_OK);
}


//-------------------------------------------------------------------------------------------------
/// Set GE line pattern style
/// @param  pGECtx                     \b IN: GE context
/// @param  enable                  \b IN: enable and update setting
/// @param  pattern                 \b IN: 0-0x3F bitmap to represent draw(1) or not draw(0)
/// @param  eRep                    \b IN: repeat pattern once, or 2,3,4 times
/// @return @ref GE_Result
//-------------------------------------------------------------------------------------------------
GE_Result MDrv_GE_SetLinePattern(GE_Context *pGECtx,MS_BOOL enable, MS_U8 pattern, GE_LinePatRep eRep)
{
    MS_U16              u16en, u16style;
    GE_CTX_LOCAL *pGECtxLocal = (GE_CTX_LOCAL*)pGECtx;

    GE_DBG("%s\n", __FUNCTION__);
    GE_ENTRY(pGECtx);

    u16en = GE_ReadReg(&pGECtxLocal->halLocalCtx, REG_GE_EN);
    if (enable)
    {
        u16en |= GE_EN_LINEPAT;

        u16style = GE_ReadReg(&pGECtxLocal->halLocalCtx, REG_GE_LINE_STYLE) & GE_LINEPAT_RST; // keep line pattern setting
        u16style |= ((pattern & GE_LINEPAT_MASK) | ((eRep << GE_LINEPAT_REP_SHFT) & GE_LINEPAT_REP_MASK));
        GE_WriteReg(&pGECtxLocal->halLocalCtx, REG_GE_LINE_STYLE, u16style);
    }
    else
    {
        u16en &= ~GE_EN_LINEPAT;

        GE_WriteReg(&pGECtxLocal->halLocalCtx, REG_GE_LINE_STYLE, GE_LINEPAT_RST); // hardware auto clear
    }
    GE_WriteReg(&pGECtxLocal->halLocalCtx, REG_GE_EN, u16en);

    GE_RETURN(pGECtx,E_GE_OK);
}


//-------------------------------------------------------------------------------------------------
/// Reset GE line pattern for next line drawing
/// @param  pGECtx                     \b IN: GE context
/// @return @ref GE_Result
//-------------------------------------------------------------------------------------------------
GE_Result MDrv_GE_ResetLinePattern(GE_Context *pGECtx)
{
    MS_U16              u16style;
    GE_CTX_LOCAL *pGECtxLocal = (GE_CTX_LOCAL*)pGECtx;

    GE_DBG("%s\n", __FUNCTION__);
    GE_ENTRY(pGECtx);

    u16style = GE_ReadReg(&pGECtxLocal->halLocalCtx, REG_GE_LINE_STYLE) | GE_LINEPAT_RST;
    GE_WriteReg(&pGECtxLocal->halLocalCtx, REG_GE_LINE_STYLE, u16style);

    GE_RETURN(pGECtx,E_GE_OK);
}


//-------------------------------------------------------------------------------------------------
/// Set GE alpha blending of color output. <b><em>Cblend = (coef * Csrc) OP ((1-coef) * Cdst)</em></b>
/// @param  pGECtx                     \b IN: GE context
/// @param  enable                  \b IN: enable and update setting
/// @param  eBlendOp                \b IN: coef and op for blending
/// @return @ref GE_Result
/// @attention
/// <b>[URANUS] <em>It does not support ROP8 operation</em></b>
//-------------------------------------------------------------------------------------------------
GE_Result MDrv_GE_SetAlphaBlend(GE_Context *pGECtx,MS_BOOL enable, GE_BlendOp eBlendOp)
{
    MS_U16              u16en;
    GE_CTX_LOCAL *pGECtxLocal = (GE_CTX_LOCAL*)pGECtx;

    GE_DBG("%s\n", __FUNCTION__);
    GE_ENTRY(pGECtx);
#ifdef GE_SPLIT_SUPPORT
    bABL = enable;
#endif
    u16en = GE_ReadReg(&pGECtxLocal->halLocalCtx, REG_GE_EN);
    if (enable)
    {
        u16en |= GE_EN_BLEND;

        if (GE_SetBlend(&pGECtxLocal->halLocalCtx, eBlendOp) != E_GE_OK)
        {
            GE_RETURN(pGECtx,E_GE_FAIL_PARAM);
        }
    }
    else
    {
        u16en &= ~GE_EN_BLEND;
    }
    GE_WriteReg(&pGECtxLocal->halLocalCtx, REG_GE_EN, u16en);

    GE_RETURN(pGECtx,E_GE_OK);
}

//-------------------------------------------------------------------------------------------------
/// Set GE alpha blending of color output. <b><em>Cblend = (coef * Csrc) OP ((1-coef) * Cdst)</em></b>
/// @param  pGECtx                     \b IN: GE context
/// @param  eBlendOp                \b IN: coef and op for blending
/// @return @ref GE_Result
/// @attention
/// <b>[URANUS] <em>It does not support ROP8 operation</em></b>
//-------------------------------------------------------------------------------------------------
GE_Result MDrv_GE_SetAlphaBlendCoef(GE_Context *pGECtx, GE_BlendOp eBlendOp)
{
   GE_CTX_LOCAL *pGECtxLocal = (GE_CTX_LOCAL*)pGECtx;

    GE_DBG("%s\n", __FUNCTION__);
    GE_ENTRY(pGECtx);

    if (GE_SetBlend(&pGECtxLocal->halLocalCtx, eBlendOp) != E_GE_OK)
    {
        GE_RETURN(pGECtx,E_GE_FAIL_PARAM);
    }

    GE_RETURN(pGECtx,E_GE_OK);
}

//-------------------------------------------------------------------------------------------------
/// Query GFX supported DFB blending functions/flags.
/// @param pU16SupportedBldFlags \b OUT: DFB Blending Functions/Flags supported by GFX.
/// @return GFX_SUCCESS - Success
/// @return GFX_FAIL - Failure
//-------------------------------------------------------------------------------------------------
GE_Result MDrv_GE_QueryDFBBldCaps(GE_Context *pGECtx, MS_U16 *pU16SupportedBldFlags)
{
    GE_CTX_LOCAL *pGECtxLocal = (GE_CTX_LOCAL*)pGECtx;
    GE_Result geResult;

    GE_DBG("%s\n", __FUNCTION__);

    geResult = GE_QueryDFBBldCaps(&pGECtxLocal->halLocalCtx, pU16SupportedBldFlags);

    return geResult;
}

//-------------------------------------------------------------------------------------------------
/// Enable GFX DFB blending
/// @param enable \b IN: true/false
/// @return GFX_SUCCESS - Success
/// @return GFX_FAIL - Failure
//-------------------------------------------------------------------------------------------------
GE_Result MDrv_GE_EnableDFBBlending(GE_Context *pGECtx, MS_BOOL enable)
{
    GE_CTX_LOCAL *pGECtxLocal = (GE_CTX_LOCAL*)pGECtx;
    GE_Result geResult;

    GE_DBG("%s\n", __FUNCTION__);
    GE_ENTRY(pGECtx);

    geResult = GE_EnableDFBBld(&pGECtxLocal->halLocalCtx, enable);

    GE_RETURN(pGECtx, geResult);
}

//-------------------------------------------------------------------------------------------------
/// Set GE DFB blending Flags.
/// @param u16DFBBldFlag       \b IN: DFB Blending Flags. The Flags will be
///                                   The combination of Flags
///                                   [GFX_DFB_BLD_FLAG_COLORALPHA, GFX_DFB_BLD_FLAG_ALPHACHANNEL,
///                                    GFX_DFB_BLD_FLAG_COLORIZE, GFX_DFB_BLD_FLAG_SRCPREMUL,
///                                    GFX_DFB_BLD_FLAG_SRCPREMULCOL, GFX_DFB_BLD_FLAG_DSTPREMUL,
///                                    GFX_DFB_BLD_FLAG_XOR, GFX_DFB_BLD_FLAG_DEMULTIPLY,
///                                    GFX_DFB_BLD_FLAG_SRCALPHAMASK, GFX_DFB_BLD_FLAG_SRCCOLORMASK]
/// @return GFX_SUCCESS - Success
/// @return GFX_FAIL - Failure
//-------------------------------------------------------------------------------------------------
GE_Result MDrv_GE_SetDFBBldFlags(GE_Context *pGECtx, MS_U16 u16DFBBldFlags)
{
    GE_CTX_LOCAL *pGECtxLocal = (GE_CTX_LOCAL*)pGECtx;
    GE_Result geResult;

    GE_DBG("%s\n", __FUNCTION__);
    GE_ENTRY(pGECtx);

    geResult = GE_SetDFBBldFlags(&pGECtxLocal->halLocalCtx, u16DFBBldFlags);

    GE_RETURN(pGECtx, geResult);
}

//-------------------------------------------------------------------------------------------------
/// Set GFX DFB blending Functions/Operations.
/// @param gfxSrcBldOP \b IN: source blending op
/// @param gfxDstBldOP \b IN: dst blending op
/// @return GFX_SUCCESS - Success
/// @return GFX_FAIL - Failure
//-------------------------------------------------------------------------------------------------
GE_Result MDrv_GE_SetDFBBldOP(GE_Context *pGECtx, GE_DFBBldOP geSrcBldOP, GE_DFBBldOP geDstBldOP)
{
    GE_CTX_LOCAL *pGECtxLocal = (GE_CTX_LOCAL*)pGECtx;
    GE_Result geResult;

    GE_DBG("%s\n", __FUNCTION__);
    GE_ENTRY(pGECtx);

    geResult = GE_SetDFBBldOP(&pGECtxLocal->halLocalCtx, geSrcBldOP, geDstBldOP);

    GE_RETURN(pGECtx, geResult);
}

//-------------------------------------------------------------------------------------------------
/// Set GFX DFB blending const color.
/// @param u32ConstColor \b IN: DFB Blending constant color
/// @return GFX_SUCCESS - Success
/// @return GFX_FAIL - Failure
//-------------------------------------------------------------------------------------------------
GE_Result MDrv_GE_SetDFBBldConstColor(GE_Context *pGECtx, GE_RgbColor geRgbColor)
{
    GE_CTX_LOCAL *pGECtxLocal = (GE_CTX_LOCAL*)pGECtx;
    GE_Result geResult;

    GE_DBG("%s\n", __FUNCTION__);
    GE_ENTRY(pGECtx);

    geResult = GE_SetDFBBldConstColor(&pGECtxLocal->halLocalCtx, geRgbColor);

    GE_RETURN(pGECtx, geResult);
}

//-------------------------------------------------------------------------------------------------
/// Set GFX DFB blending source colormask.
/// @param u32ConstColor \b IN: DFB Blending constant color
/// @return GFX_SUCCESS - Success
/// @return GFX_FAIL - Failure
//-------------------------------------------------------------------------------------------------
GE_Result MDrv_GE_SetDFBBldSrcColorMask(GE_Context *pGECtx, GE_RgbColor geRgbColor)
{
    GE_CTX_LOCAL *pGECtxLocal = (GE_CTX_LOCAL*)pGECtx;
    GE_Result geResult;

    GE_DBG("%s\n", __FUNCTION__);
    GE_ENTRY(pGECtx);

    geResult = GE_SetDFBBldSrcColorMask(&pGECtxLocal->halLocalCtx, geRgbColor);

    GE_RETURN(pGECtx, geResult);
}


#ifdef CONFIG_GFX_TRAPZOID
GE_Result MDrv_GE_EnableTrapezoidAA(GE_Context *pGECtx, MS_BOOL bEnable)
{
    GE_CTX_LOCAL *pGECtxLocal = (GE_CTX_LOCAL*)pGECtx;
    GE_Result geResult;

    GE_DBG("%s\n", __FUNCTION__);

    GE_ENTRY(pGECtx);

    geResult = GE_EnableTrapezoidAA(&pGECtxLocal->halLocalCtx, bEnable);

    GE_RETURN(pGECtx, geResult);
}

GE_Result MDrv_GE_EnableTrapSubPixCorr(GE_Context *pGECtx, MS_BOOL bEnable)
{
    GE_CTX_LOCAL *pGECtxLocal = (GE_CTX_LOCAL*)pGECtx;
    GE_Result geResult;

    GE_DBG("%s\n", __FUNCTION__);

    GE_ENTRY(pGECtx);

    geResult = GE_EnableTrapSubPixCorr(&pGECtxLocal->halLocalCtx, bEnable);

    GE_RETURN(pGECtx, geResult);
}
#endif

//-------------------------------------------------------------------------------------------------
/// Set GE clipping window
/// @param  pGECtx                     \b IN: GE context
/// @param rect                     \b IN: pointer to RECT of clipping window
/// @return @ref GE_Result
//-------------------------------------------------------------------------------------------------
GE_Result MDrv_GE_SetClipWindow(GE_Context *pGECtx, GE_Rect *rect)
{
    GE_CTX_LOCAL *pGECtxLocal = (GE_CTX_LOCAL*)pGECtx;

    GE_DBG("%s\n", __FUNCTION__);
    GE_ASSERT((rect), E_GE_FAIL_PARAM, "%s: Null pointer\n", __FUNCTION__);

    GE_ENTRY(pGECtx);

    pGECtxLocal->u16ClipL = rect->x;
    pGECtxLocal->u16ClipT = rect->y;
    pGECtxLocal->u16ClipR = (rect->x + rect->width - 1);
    pGECtxLocal->u16ClipB = (rect->y + rect->height - 1);

    HAL_GE_SetClipWindow(&pGECtxLocal->halLocalCtx, rect);

    GE_RETURN(pGECtx,E_GE_OK);
}


//-------------------------------------------------------------------------------------------------
/// Get GE clipping window
/// @param  pGECtx                     \b IN: GE context
/// @param rect                     \b IN: pointer to RECT of clipping window
/// @return @ref GE_Result
//-------------------------------------------------------------------------------------------------
GE_Result MDrv_GE_GetClipWindow(GE_Context *pGECtx, GE_Rect *rect)
{
    GE_CTX_LOCAL *pGECtxLocal = (GE_CTX_LOCAL*)pGECtx;

    GE_DBG("%s\n", __FUNCTION__);
    GE_ASSERT((rect), E_GE_FAIL_PARAM, "%s: Null pointer\n", __FUNCTION__);

    GE_ENTRY(pGECtx);

    rect->x = GE_ReadReg(&pGECtxLocal->halLocalCtx, REG_GE_CLIP_L);
    rect->y = GE_ReadReg(&pGECtxLocal->halLocalCtx, REG_GE_CLIP_T);
    rect->width = GE_ReadReg(&pGECtxLocal->halLocalCtx, REG_GE_CLIP_R)-rect->x+1;
    rect->height = GE_ReadReg(&pGECtxLocal->halLocalCtx, REG_GE_CLIP_B)-rect->y+1;

    GE_RETURN(pGECtx,E_GE_OK);
}


//-------------------------------------------------------------------------------------------------
/// Set GE bitblt italic style
/// @param  pGECtx                     \b IN: GE context
/// @param  x_offset                \b IN: starting pixel to get italic effect
/// @param  y_offset                \b IN: starting line to get italic effect
/// @param  delta                   \b IN: italic slope delta [s1.3]
/// @return @ref GE_Result
/// @note\n
/// The italic effect can not perform with rotate process or mirror
/// @sa MDrv_GE_BitBlt, E_GE_FLAG_BLT_ITALIC
//-------------------------------------------------------------------------------------------------
GE_Result MDrv_GE_SetItalic(GE_Context *pGECtx, MS_U8 x_offset, MS_U8 y_offset, MS_U8 delta)
{
    GE_CTX_LOCAL *pGECtxLocal = (GE_CTX_LOCAL*)pGECtx;

    GE_DBG("%s\n", __FUNCTION__);
    GE_ENTRY(pGECtx);

    GE_WriteReg(&pGECtxLocal->halLocalCtx, REG_GE_ITALIC_OFFSET, (((MS_U16)y_offset<<GE_ITALIC_Y_SHFT) | x_offset));
    GE_WriteReg(&pGECtxLocal->halLocalCtx, REG_GE_ITALIC_DELTA, delta&0x1F); // s1.3

    GE_RETURN(pGECtx,E_GE_OK);
}


//-------------------------------------------------------------------------------------------------
/// Set GE constant alpha value for any alpha operation
/// @param  pGECtx                     \b IN: GE context
/// @param  aconst                  \b IN: constant alpha value for output alpha and blending
/// @return @ref GE_Result
/// @sa MDrv_GE_SetAlphaBlend, MDrv_GE_SetAlphaSrc
//-------------------------------------------------------------------------------------------------
GE_Result MDrv_GE_SetAlphaConst(GE_Context *pGECtx, MS_U8 aconst)
{
    GE_CTX_LOCAL *pGECtxLocal = (GE_CTX_LOCAL*)pGECtx;

    GE_DBG("%s\n", __FUNCTION__);
    GE_ENTRY(pGECtx);
#ifdef GE_ALPHA_BLEND_PATCH
    u16GAlphaConst = (MS_U16)aconst;
#endif
    GE_WriteReg(&pGECtxLocal->halLocalCtx, REG_GE_ALPHA_CONST, (MS_U16)(aconst&0xff));

    GE_RETURN(pGECtx,E_GE_OK);
}


//-------------------------------------------------------------------------------------------------
/// Set GE alpha source for alpha output. <b><em>Aout</em></b>
/// @param  pGECtx                     \b IN: GE context
/// @param  eAlphaSrc               \b IN: alpha channel comes from
/// @return @ref GE_Result
/// @note
/// <b>(REQUIREMENT) <em>E_GE_ALPHA_ADST requires AlphaBlending(TRUE, )</em></b>
/// @attention
/// <b>[URANUS] <em>It does not support ROP8 operation</em></b>
//-------------------------------------------------------------------------------------------------
GE_Result MDrv_GE_SetAlphaSrc(GE_Context *pGECtx, GE_AlphaSrc eAlphaSrc)
{
    GE_Result           ret;
    GE_CTX_LOCAL *pGECtxLocal = (GE_CTX_LOCAL*)pGECtx;

    GE_DBG("%s\n", __FUNCTION__);
    GE_ENTRY(pGECtx);

    ret = GE_SetAlpha(&pGECtxLocal->halLocalCtx, eAlphaSrc);

    GE_RETURN(pGECtx,ret);
}


//-------------------------------------------------------------------------------------------------
/// Set GE alpha comparison before alpha output. <b><em>Aout = cmp(Aout, Adst)</em></b>
/// @param  pGECtx                     \b IN: GE context
/// @param  enable                  \b IN: enable and update setting
/// @param  eACmpOp                 \b IN: MAX / MIN
/// @return @ref GE_Result
/// @note
/// <b>(REQUIREMENT) <em>It requires AlphaBlending(TRUE, )</em></b>
/// @sa MDrv_GE_SetAlphaSrc
//-------------------------------------------------------------------------------------------------
GE_Result MDrv_GE_SetAlphaCmp(GE_Context *pGECtx, MS_BOOL enable, GE_ACmpOp eACmpOp)
{
    MS_U16              u16en, u16op;
    GE_CTX_LOCAL *pGECtxLocal = (GE_CTX_LOCAL*)pGECtx;

    GE_DBG("%s\n", __FUNCTION__);
    GE_ENTRY(pGECtx);

    u16en = GE_ReadReg(&pGECtxLocal->halLocalCtx, REG_GE_EN);
    if (enable)
    {
        u16en |= GE_EN_ACMP;

        u16op = (GE_ReadReg(&pGECtxLocal->halLocalCtx, REG_GE_OP_MODE) & ~GE_OP_ACMP_MIN) | ((eACmpOp<<GE_OP_ACMP_SHFT)&GE_OP_ACMP_MIN);
        GE_WriteReg(&pGECtxLocal->halLocalCtx, REG_GE_OP_MODE, u16op);
    }
    else
    {
        u16en &= ~GE_EN_ACMP;
    }
    GE_WriteReg(&pGECtxLocal->halLocalCtx, REG_GE_EN, u16en);

    GE_RETURN(pGECtx,E_GE_OK);
}


//-------------------------------------------------------------------------------------------------
/// Set GE destination alpha test for doing alpha blending <b><em>Cout = Adst->[AL,AH] ? Csrc : Cblend</em></b>
/// @param  pGECtx                     \b IN: GE context
/// @param  enable                  \b IN: enable and update setting
/// @param  eATestOp                \b IN: destination alpha test mode
/// @param  a_low                   \b IN: lower bound of alpha value
/// @param  a_high                  \b IN: upper bound of alpha value
/// @return @ref GE_Result
/// @note
/// <b>(REQUIREMENT) <em>AlphaBlending(TRUE, )</em></b>
/// @attention
/// <b>[URANUS] <em>It does not support alpha test</em></b>
//-------------------------------------------------------------------------------------------------
GE_Result MDrv_GE_SetAlphaTest(GE_Context *pGECtx, MS_BOOL enable, GE_ATestOp eATestOp, MS_U8 a_low, MS_U8 a_high)
{
    MS_U16              u16en, u16op;
    GE_CTX_LOCAL *pGECtxLocal = (GE_CTX_LOCAL*)pGECtx;


    GE_DBG("%s\n", __FUNCTION__);
    GE_ENTRY(pGECtx);

    u16en = GE_ReadReg(&pGECtxLocal->halLocalCtx, REG_GE_EN);
    if (enable)
    {
        u16en |= GE_EN_ATEST;
        GE_WriteReg(&pGECtxLocal->halLocalCtx, REG_GE_ATEST_TH, ((MS_U16)a_low<<8) | (a_high));

        u16op = (GE_ReadReg(&pGECtxLocal->halLocalCtx, REG_GE_OP_MODE)&~GE_OP_ATEST_NE) | (eATestOp << GE_OP_ATEST_SHFT);
        GE_WriteReg(&pGECtxLocal->halLocalCtx, REG_GE_OP_MODE, u16op);
    }
    else
    {
        u16en &= ~GE_EN_ATEST;
    }
    GE_WriteReg(&pGECtxLocal->halLocalCtx, REG_GE_EN, u16en);

    GE_RETURN(pGECtx,E_GE_OK);

}


//-------------------------------------------------------------------------------------------------
/// Set GE color to palette table
/// @param  pGECtx                     \b IN: GE context
/// @param  start                   \b IN: start index of palette (0 - GE_PALETTE_NUM)
/// @param  num                     \b IN: number of palette entry to set
/// @param  palette                 \b IN: user defined palette color table
/// @return @ref GE_Result
/// @attention
/// <b>[URANUS] <em>It does not support palette</em></b>
//-------------------------------------------------------------------------------------------------
GE_Result MDrv_GE_SetPalette(GE_Context *pGECtx, MS_U16 start, MS_U16 num, MS_U32 *palette)
{
   GE_CTX_LOCAL *pGECtxLocal = (GE_CTX_LOCAL*)pGECtx;
    MS_U32 i;

    GE_DBG("%s\n", __FUNCTION__);
    GE_ASSERT((palette), E_GE_FAIL_PARAM, "%s: Null pointer\n", __FUNCTION__);

    if ((start + num) > GE_PALETTE_NUM)
    {
        return (E_GE_FAIL_PARAM);
    }

    for (i = 0; i < num; i++)
    {
        pGECtxLocal->halLocalCtx.u32Palette[start+i] = palette[i];
    }

    pGECtxLocal->halLocalCtx.bPaletteDirty = TRUE;

    return (E_GE_OK);
}


//-------------------------------------------------------------------------------------------------
/// Get GE color from palette table
/// @param  pGECtx                     \b IN: GE context
/// @param  start                   \b IN: start index of palette (0 - GE_PALETTE_NUM)
/// @param  num                     \b IN: number of palette entry to set
/// @param  palette                 \b OUT: user buffer to get palette data
/// @return @ref GE_Result
/// <b>[URANUS] <em>It does not support palette</em></b>
//-------------------------------------------------------------------------------------------------
GE_Result MDrv_GE_GetPalette(GE_Context *pGECtx, MS_U16 start, MS_U16 num, MS_U32 *palette)
{
    GE_CTX_LOCAL *pGECtxLocal = (GE_CTX_LOCAL*)pGECtx;
    MS_U32                 i;
    //MS_U32 palette[16]; //NOW ACQUIRE INDEX 0~15 with E_GE_READ_DIRECT mode

    GE_DBG("%s\n", __FUNCTION__);
    GE_ASSERT((palette), E_GE_FAIL_PARAM, "%s: Null pointer\n", __FUNCTION__);

    if ((num-start+1) > GE_PALETTE_NUM)
    {
        return (E_GE_FAIL_PARAM);
    }

    // Wait command queue flush
    for (i = 0; i <(num-start+1); i++)
    {
        palette[i] = pGECtxLocal->halLocalCtx.u32Palette[start+i];
    }

    return E_GE_OK;
}


//-------------------------------------------------------------------------------------------------
/// Set GE YUV operation mode
/// @param  pGECtx                     \b IN: GE context
/// @param  mode                    \b IN: pointer to YUV setting structure.
/// @return @ref GE_Result
/// @attention
/// <b>[URANUS] <em>It does not support RGB2YUV conversion</em></b>
//-------------------------------------------------------------------------------------------------
GE_Result MDrv_GE_SetYUVMode(GE_Context *pGECtx, GE_YUVMode *mode)
{
    GE_CTX_LOCAL *pGECtxLocal = (GE_CTX_LOCAL*)pGECtx;

    GE_DBG("%s\n", __FUNCTION__);
    GE_ASSERT((mode), E_GE_FAIL_PARAM, "%s: Null pointer\n", __FUNCTION__);

    GE_ENTRY(pGECtx);

    HAL_GE_SetYUVMode(&pGECtxLocal->halLocalCtx, mode);

    GE_RETURN(pGECtx,E_GE_OK);
}


//-------------------------------------------------------------------------------------------------
/// Set GE source buffer info
/// @param  pGECtx                     \b IN: GE context
/// @param  src_fmt                 \b IN: source buffer format
/// @param  pix_width               \b IN: pixel width of source buffer (RESERVED)
/// @param  pix_height              \b IN: pixel height of source buffer (RESERVED)
/// @param  addr                    \b IN: source buffer start address [23:0]
/// @param  pitch                   \b IN: source buffer linear pitch in byte unit
/// @param  flags                   \b IN: option flag of dst buffer <b>(RESERVED)</b>
/// @return @ref GE_Result
/// @attention
/// <b>[URANUS] <em>E_GE_FMT_ARGB1555 is RGB555</em></b>
/// <b>[URANUS] <em>E_GE_FMT_* should be 8-byte addr/pitch aligned to do stretch bitblt</em></b>
/// @sa MDrv_GE_GetFmtCaps
//-------------------------------------------------------------------------------------------------
GE_Result MDrv_GE_SetSrcBuffer(GE_Context *pGECtx, GE_BufFmt src_fmt, MS_U16 pix_width, MS_U16 pix_height, MS_PHY addr, MS_U16 pitch, MS_U32 flags)
{
    GE_Result           ret;
    GE_FmtCaps          caps;

    GE_CTX_LOCAL *pGECtxLocal = (GE_CTX_LOCAL*)pGECtx;

    GE_DBG("%s\n", __FUNCTION__);
    GE_ENTRY(pGECtx);

    // pix_width, pix_height: reserved for future use.
    src_fmt = (GE_BufFmt) (0xFF & src_fmt);
    ret = GE_GetFmtCaps(&pGECtxLocal->halLocalCtx, src_fmt, E_GE_BUF_SRC, &caps);
    if (ret != E_GE_OK)
    {
        GE_ERR("%s: format fail\n", __FUNCTION__);
        GE_RETURN(pGECtx,ret);
    }
    if (addr != ((addr + (caps.u8BaseAlign-1)) & ~(caps.u8BaseAlign-1)))
    {
        GE_ERR("%s: address fail\n", __FUNCTION__);
        GE_RETURN(pGECtx,E_GE_FAIL_ADDR);
    }
    if (pitch != ((pitch + (caps.u8PitchAlign-1)) & ~(caps.u8PitchAlign-1)))
    {
        GE_ERR("%s: pitch fail\n", __FUNCTION__);
        GE_RETURN(pGECtx,E_GE_FAIL_PITCH);
    }

    pGECtxLocal->PhySrcAddr = addr;

    HAL_GE_SetBuffer(&pGECtxLocal->halLocalCtx, pitch, GE_GetFmt(src_fmt), E_GE_BUF_SRC);

    HAL_GE_SetBufferAddr(&pGECtxLocal->halLocalCtx,addr,E_GE_BUF_SRC);

    GE_RETURN(pGECtx,E_GE_OK);
}

//-------------------------------------------------------------------------------------------------
/// Get PE destination buffer info
/// @param  pGECtx                     \b IN: GE context
/// @param  bufinfo                 \b IN: pointer to GE_BufInfo struct
/// @return @ref GE_Result
/// @attention
/// <b>[URANUS] <em>E_GE_FMT_ARGB1555 is RGB555, E_GE_FMT_YUV422 can not be destination.</em></b>
/// <b>[URANUS] <em>E_GE_FMT_* should be 8-byte addr/pitch aligned to do stretch bitblt</em></b>
/// @sa @ref MDrv_GE_GetFmtCaps
//-------------------------------------------------------------------------------------------------
GE_Result MDrv_GE_GetBufferInfo(GE_Context *pGECtx, GE_BufInfo *bufinfo)
{

    GE_CTX_LOCAL *pGECtxLocal = (GE_CTX_LOCAL*)pGECtx;
    GE_DBG("%s\n", __FUNCTION__);
    GE_ENTRY(pGECtx);

    //bufinfo->srcfmt = (GE_BufFmt)(GE_ReadReg(&pGECtxLocal->halLocalCtx, REG_GE_FMT) & GE_SRC_FMT_MASK);
    //bufinfo->dstfmt = (GE_BufFmt)((GE_ReadReg(&pGECtxLocal->halLocalCtx, REG_GE_FMT) & GE_DST_FMT_MASK) >> 8);
    bufinfo->srcfmt = pGECtx->pBufInfo.srcfmt;
    bufinfo->dstfmt = pGECtx->pBufInfo.dstfmt;

    bufinfo->srcpit = GE_ReadReg(&pGECtxLocal->halLocalCtx, REG_GE_SRC_PITCH);
    bufinfo->dstpit = GE_ReadReg(&pGECtxLocal->halLocalCtx, REG_GE_DST_PITCH);

    bufinfo->dstaddr = pGECtxLocal->PhyDstAddr;
    bufinfo->srcaddr = pGECtxLocal->PhySrcAddr;

    GE_RETURN(pGECtx,E_GE_OK);
}

//-------------------------------------------------------------------------------------------------
/// Set GE destination buffer info
/// @param  pGECtx                     \b IN: GE context
/// @param  dst_fmt                 \b IN: source buffer format
/// @param  pix_width               \b IN: pixel width of destination buffer (RESERVED)
/// @param  pix_height              \b IN: pixel height of destination buffer (RESERVED)
/// @param  addr                    \b IN: destination buffer start address [23:0]
/// @param  pitch                   \b IN: destination buffer linear pitch in byte unit
/// @param  flags                   \b IN: option flag of dst buffer <b>(RESERVED)</b>
/// @return @ref GE_Result
/// @attention
/// <b>[URANUS] <em>E_GE_FMT_ARGB1555 is RGB555, E_GE_FMT_YUV422 can not be destination.</em></b>
/// <b>[URANUS] <em>E_GE_FMT_* should be 8-byte addr/pitch aligned to do stretch bitblt</em></b>
/// @sa @ref MDrv_GE_GetFmtCaps
//-------------------------------------------------------------------------------------------------
GE_Result MDrv_GE_SetDstBuffer(GE_Context *pGECtx, GE_BufFmt dst_fmt, MS_U16 pix_width, MS_U16 pix_height, MS_PHY addr, MS_U16 pitch, MS_U32 flags)
{
    GE_Result           ret;
    GE_FmtCaps          caps;
    GE_CTX_LOCAL *pGECtxLocal = (GE_CTX_LOCAL*)pGECtx;

    GE_DBG("%s\n", __FUNCTION__);
    GE_ENTRY(pGECtx);

    // pix_width, pix_height: reserved for future use.
    dst_fmt = (GE_BufFmt) (0xFF & dst_fmt);

    ret = GE_GetFmtCaps(&pGECtxLocal->halLocalCtx, dst_fmt, E_GE_BUF_DST, &caps);
    if (ret != E_GE_OK)
    {
        GE_ERR("%s: format fail\n", __FUNCTION__);
        GE_RETURN(pGECtx,ret);
    }
    if (addr != ((addr + (caps.u8BaseAlign-1)) & ~(caps.u8BaseAlign-1)))
    {
        GE_ERR("%s: address fail\n", __FUNCTION__);
        GE_RETURN(pGECtx,E_GE_FAIL_ADDR);
    }
    if (pitch != ((pitch + (caps.u8PitchAlign-1)) & ~(caps.u8PitchAlign-1)))
    {
        GE_ERR("%s: pitch fail\n", __FUNCTION__);
        GE_RETURN(pGECtx,E_GE_FAIL_PITCH);
    }

    pGECtxLocal->PhyDstAddr = addr;

    HAL_GE_SetBuffer(&pGECtxLocal->halLocalCtx, pitch, GE_GetFmt(dst_fmt), E_GE_BUF_DST);

    HAL_GE_SetBufferAddr(&pGECtxLocal->halLocalCtx,addr,E_GE_BUF_DST);

    GE_RETURN(pGECtx,E_GE_OK);
}

//-------------------------------------------------------------------------------------------------
/// Set GE destination buffer of two source buffer
/// @param  pGECtx                     \b IN: GE context
/// @param  dst_fmt                 \b IN: format of destination buffer of two source
/// @param  pSrc0Rect               \b IN: source0 Rect of two source(RESERVED)
/// @param  pSrc1Rect              \b IN: source1 Rect of two source(RESERVED)
/// @param  pDestRect               \b IN: Destination Rect of two source(RESERVED)
/// @param  addr                    \b IN: destination buffer of two source start address [23:0]
/// @param  pitch                   \b IN: destination buffer of two source linear pitch in byte unit
/// @param  flags                   \b IN: option flag of dst buffer <b>(RESERVED)</b>
/// @return @ref GE_Result
/// @attention
/// <b>[URANUS] <em>E_GE_FMT_ARGB1555 is RGB555, E_GE_FMT_YUV422 can not be destination.</em></b>
/// <b>[URANUS] <em>E_GE_FMT_* should be 8-byte addr/pitch aligned to do stretch bitblt</em></b>
/// @sa @ref MDrv_GE_GetFmtCaps
//-------------------------------------------------------------------------------------------------
GE_Result MDrv_GE_BitBltByTwoSourceBuffer(GE_Context *pGECtx, GE_BufFmt dst_fmt, GE_Rect *pSrc0Rect, GE_Rect *pSrc1Rect, GE_Rect *pDestRect, MS_PHY addr, MS_U16 pitch, MS_U32 flags, GE_ScaleInfo* scaleinfo)
{
    GE_Result           ret;
    GE_FmtCaps          caps;
    MS_U16              u16fmt = 0;
    MS_U16              u16en  = 0;
    GE_Point            v0, v1;
    MS_U8 miu=0;
    MS_U16 reg_val=0;
    GE_CTX_LOCAL *pGECtxLocal = (GE_CTX_LOCAL*)pGECtx;

    GE_ENTRY(pGECtx);
    if(FALSE == pGECtxLocal->halLocalCtx.pGeChipPro->bSupportTwoSourceBitbltMode)
    {
        GE_RETURN(pGECtx,E_GE_NOT_SUPPORT);
    }
    // pix_width, pix_height: reserved for future use.
    dst_fmt = (GE_BufFmt) (0xFF & dst_fmt);

    ret = GE_GetFmtCaps(&pGECtxLocal->halLocalCtx, dst_fmt, E_GE_BUF_DST, &caps);
    if (ret != E_GE_OK)
    {
        GE_ERR("%s: format fail\n", __FUNCTION__);
        GE_RETURN(pGECtx,ret);
    }
    if (addr != ((addr + (caps.u8BaseAlign-1)) & ~(caps.u8BaseAlign-1)))
    {
        GE_ERR("%s: address fail\n", __FUNCTION__);
        GE_RETURN(pGECtx,E_GE_FAIL_ADDR);
    }
    if (pitch != ((pitch + (caps.u8PitchAlign-1)) & ~(caps.u8PitchAlign-1)))
    {
        GE_ERR("%s: pitch fail\n", __FUNCTION__);
        GE_RETURN(pGECtx,E_GE_FAIL_PITCH);
    }

    //There are some HW limitations, check it
    if(flags & E_GE_FLAG_BLT_STRETCH)
    {
        MS_U16 src1fmt = (GE_ReadReg(&pGECtxLocal->halLocalCtx, REG_GE_FMT) & GE_DST_FMT_MASK) >> GE_DST_FMT_SHFT;
        MS_U16 destfmt = GE_GetFmt(dst_fmt);
        if(destfmt != src1fmt)
        {
            GE_ERR("[%s] Two source stretch bitblt HW limitation: src1 format and dest format must be the same. src1 format:%d, dest format:%d\n"
                ,__FUNCTION__,src1fmt,destfmt);
            GE_RETURN(pGECtx,E_GE_FAIL_STRETCH);
        }
        #ifdef GE_TwoSourceAlign
        MS_U16 alignValue = (GE_TwoSourceAlign / 8) / caps.u8BaseAlign;
        MS_U16 scr1x = pSrc1Rect->x % alignValue;
        MS_U16 destx = pDestRect->x % alignValue;
        if(scr1x != destx)
        {
            GE_ERR("[%s] Two source stretch bitblt HW limitation: src1 x and dest x must align %d bit position. src1 x:%d, dest x:%d\n"
                ,__FUNCTION__,GE_TwoSourceAlign,pSrc1Rect->x,pDestRect->x);
            GE_RETURN(pGECtx,E_GE_FAIL_STRETCH);
        }
        #endif
    }

    GEWD_ReadReg(&pGECtxLocal->halLocalCtx, REG_GE_EN, &u16en);
    u16en |= GE_EN_GE;
    GEWD_WriteReg(&pGECtxLocal->halLocalCtx, REG_GE_EN, u16en);

    GEWD_ReadReg(&pGECtxLocal->halLocalCtx, REG_GE_FMT,&u16fmt);
    u16fmt = (u16fmt & ~GE_DST_FMT_MASK) | ( GE_GetFmt(dst_fmt) << GE_DST_FMT_SHFT);
    //u16fmt = (GEWD_ReadReg(&pGECtxLocal->halLocalCtx, REG_GE_FMT) & ~GE_DST_FMT_MASK) | ( GE_GetFmt(dst_fmt) << GE_DST_FMT_SHFT);
    GEWD_WriteReg(&pGECtxLocal->halLocalCtx, REG_GE_FMT, u16fmt);

    miu = _GFXAPI_MIU_ID(addr);
    addr = GE_ConvertAPIAddr2HAL(&pGECtxLocal->halLocalCtx, miu, _GFXAPI_PHYS_ADDR_IN_MIU(addr));
    if(miu>=2)
    {
        reg_val = GE_ReadReg(&pGECtxLocal->halLocalCtx, REG_GE_YUV_MODE) | (1<<GE_DST_BUFFER_MIU_H_SHFT);
        GE_WriteReg(&pGECtxLocal->halLocalCtx, REG_GE_YUV_MODE, reg_val);
    }
    else
    {
        reg_val = GE_ReadReg(&pGECtxLocal->halLocalCtx, REG_GE_YUV_MODE) & (~(1<<GE_DST_BUFFER_MIU_H_SHFT));
        GE_WriteReg(&pGECtxLocal->halLocalCtx, REG_GE_YUV_MODE, reg_val);
    }
    GEWD_WriteReg(&pGECtxLocal->halLocalCtx, REG_GE_DST_BASE_L, (addr & 0xFFFF));
    GEWD_WriteReg(&pGECtxLocal->halLocalCtx, REG_GE_DST_BASE_H, (addr >> 16));

    // Set destination pitch
    GEWD_WriteReg(&pGECtxLocal->halLocalCtx, REG_GE_DST_PITCH, pitch);

    v0.x = pDestRect->x;
    v0.y = pDestRect->y;
    v1.x = v0.x + pDestRect->width - 1;
    v1.y = v0.y + pDestRect->height - 1;

    GEWD_WriteReg(&pGECtxLocal->halLocalCtx, REG_GE_PRIM_V0_X, v0.x);
    GEWD_WriteReg(&pGECtxLocal->halLocalCtx, REG_GE_PRIM_V0_Y, v0.y);
    GEWD_WriteReg(&pGECtxLocal->halLocalCtx, REG_GE_PRIM_V1_X, v1.x);
    GEWD_WriteReg(&pGECtxLocal->halLocalCtx, REG_GE_PRIM_V1_Y, v1.y);

    GE_Rect src;
    GE_DstBitBltType dst;
    src.x = pSrc0Rect->x;
    src.y = pSrc0Rect->y;
    src.width = pSrc0Rect->width;
    src.height = pSrc0Rect->height;
    dst.dstblk.x = pSrc1Rect->x;
    dst.dstblk.y = pSrc1Rect->y;
    dst.dstblk.width = pSrc1Rect->width;
    dst.dstblk.height = pSrc1Rect->height;

    ret = MDrv_GE_BitBltEX(pGECtx, &src, &dst, flags, scaleinfo);
    GEWD_ReadReg(&pGECtxLocal->halLocalCtx, REG_GE_EN, &u16en);
    u16en &= ~GE_EN_GE;
    GEWD_WriteReg(&pGECtxLocal->halLocalCtx, REG_GE_EN, u16en);

    GE_RETURN(pGECtx,ret);
}


//-------------------------------------------------------------------------------------------------
/// GE Primitive Drawing - LINE
/// @param  pGECtx                     \b IN: GE context
/// @param  v0                      \b IN: pointer to start point of line
/// @param  v1                      \b IN: pointer to end point of line
/// @param  color                   \b IN: color of LINE
/// @param  color2                  \b IN: 2nd color of gradient
/// @param  flags                   \b IN: LINE drawing flags
/// @return @ref GE_Result
//-------------------------------------------------------------------------------------------------
GE_Result MDrv_GE_DrawLine(GE_Context *pGECtx, GE_Point *v0, GE_Point *v1, MS_U32 color, MS_U32 color2, MS_U32 flags, MS_U32 width)
{
    MS_U16              u16cmd = 0;
    MS_U16              u16cfg;
    MS_U16              u16style = 0;
    MS_U16              dw, dh, dminor, dmajor;
    MS_BOOL             y_major = false;
    MS_U32              u32Start ;
    MS_U32              u32End;
    MS_U32              i, u32OutWidth = 0;
    MS_BOOL             bDstXInv = FALSE;
    GE_CTX_LOCAL*       pGECtxLocal = (GE_CTX_LOCAL*)pGECtx;


    GE_DBG("%s\n", __FUNCTION__);
    GE_ASSERT((v0), E_GE_FAIL_PARAM, "%s: NULL pointer\n", __FUNCTION__);
    GE_ASSERT((v1), E_GE_FAIL_PARAM, "%s: NULL pointer\n", __FUNCTION__);

    GE_ENTRY(pGECtx);

    if(flags & E_GE_FLAG_INWIDTH_IS_OUTWIDTH)
    {
        u32OutWidth = width;
    }
    else
    {
        u32OutWidth = width+1;
    }

#if DYNAMIC_POWER_ON_OFF
    MDrv_PWR_ClockTurnOn(E_PWR_MODULE_GE);
#endif

#if 0//(SUPPORT_LINE_LAST_PATCH == 1 )
    GE_WriteReg(&pGECtxLocal->halLocalCtx,REG_GE_LINE_STYLE, GE_LINEPAT_RST | 0x3f);
    GE_WriteReg(&pGECtxLocal->halLocalCtx,REG_GE_LINE_STYLE,  0x3f);
//#else
    GE_WriteReg(&pGECtxLocal->halLocalCtx,REG_GE_LINE_STYLE, GE_LINE_LAST | GE_LINEPAT_RST | 0x3f);
    GE_WriteReg(&pGECtxLocal->halLocalCtx,REG_GE_LINE_STYLE, GE_LINE_LAST | 0x3f);
#endif

    /*2010.01.11 mark for green key draw line of T079 DynamicLineArt Advanced issue.
       mheg5 app maybe give large dstination v1.x (over 4096) position to ge driver and use clip window
       to do this application.
       We should pass app invaild destination position checking mechanism here and just check clip widnow
       is vaild or not for app*/
#if 0
    if ( (v0->x > 0x7FF) || (v0->y > 0x7FF) || (v1->x > 0x7FF) || (v1->y > 0x7FF) )
    {
        GE_ERR("%s v0, v1 out of range (>0x7FF)\n", __FUNCTION__);
        GE_RETURN(pGECtx,E_GE_NOT_DRAW);
    }
#endif
    // fast clip check
    if (GE_FastClip(pGECtx, v0->x, v0->y, v1->x, v1->y) && (width==1))
    {
        if((flags & E_GE_FLAG_INWIDTH_IS_OUTWIDTH) == 0)
        {
            GE_RETURN(pGECtx,E_GE_NOT_DRAW);
        }
    }

    dw = (v0->x >= v1->x) ? (v0->x - v1->x) : (v1->x - v0->x);
    dh = (v0->y >= v1->y) ? (v0->y - v1->y) : (v1->y - v0->y);

#if 0       // 1 pixel drawing should be allowed.
    if ( (dh == 0) && (dw == 0) )
    {
        GE_RETURN(pGECtx,E_GE_NOT_DRAW);
    }
#endif

    if ( (dh > dw) || (dw == 0) )
    {
        y_major = TRUE;
    }

    GE_Restore_PaletteContext(pGECtxLocal);

    // clear dirty attributes ---------------------------------------------------------------------
    // reset unused register
    GE_SetRotate(&pGECtxLocal->halLocalCtx, E_GE_ROTATE_0);
    u16cfg = GE_ReadReg(&pGECtxLocal->halLocalCtx, REG_GE_CFG) & ~(GE_CFG_BLT_STRETCH | GE_CFG_BLT_ITALIC);
    u16style = GE_ReadReg(&pGECtxLocal->halLocalCtx, REG_GE_LINE_STYLE);
    GE_WriteReg(&pGECtxLocal->halLocalCtx, REG_GE_CFG, u16cfg);
    if(flags & E_GE_FLAG_DRAW_LASTPOINT)
    {
        u16style |= GE_LINE_LAST;
    }
    else
    {
        u16style &= ~GE_LINE_LAST;
    }
    GE_WriteReg(&pGECtxLocal->halLocalCtx,REG_GE_LINE_STYLE, u16style);

    // clear dirty attributes ---------------------------------------------------------------------

    // line delta
    if (y_major)
    {
        dmajor = dh;
        dminor = GE_Divide2Fixed(dw, dmajor, 1, 12); // s1.12
        if (v0->x > v1->x)
        {
            dminor = (MS_U16)(1<<(2+12)) - dminor; // negative
        }
        if (v0->y > v1->y)
        {
            u16cmd |= GE_DST_DIR_X_INV; // line always refer DIR_X
        }
        GE_WriteReg(&pGECtxLocal->halLocalCtx, _REG_GE_LINE_DELTA, GE_LINE_MAJOR_Y | (dminor << GE_LINE_DELTA_SHFT));
    }
    else // x major
    {
        dmajor = dw;
        dminor = GE_Divide2Fixed(dh, dmajor, 1, 12); // s1.12
        if (v0->y > v1->y)
        {
            dminor = (MS_U16)(1<<(2+12)) - dminor; // negative
        }
        if (v0->x > v1->x)
        {
            u16cmd |= GE_DST_DIR_X_INV;
        }
        GE_WriteReg(&pGECtxLocal->halLocalCtx, _REG_GE_LINE_DELTA, GE_LINE_MAJOR_X | (dminor << GE_LINE_DELTA_SHFT));
    }

    // Line color
    GE_WriteReg(&pGECtxLocal->halLocalCtx, REG_GE_PRIM_C_L, (color & 0xFFFF));
    GE_WriteReg(&pGECtxLocal->halLocalCtx, REG_GE_PRIM_C_H, (color >> 16));

    if (flags & E_GE_FLAG_LINE_GRADIENT)
    {
        GE_ColorDelta delta;
        GE_CalcColorDelta(color, color2, dmajor, &delta);

        GE_WriteReg(&pGECtxLocal->halLocalCtx, REG_GE_PRIM_RDX_L, (delta.r & 0xFFFF));
        GE_WriteReg(&pGECtxLocal->halLocalCtx, REG_GE_PRIM_RDX_H, (delta.r >> 16));

        GE_WriteReg(&pGECtxLocal->halLocalCtx, REG_GE_PRIM_GDX_L, (delta.g & 0xFFFF));
        GE_WriteReg(&pGECtxLocal->halLocalCtx, REG_GE_PRIM_GDX_H, (delta.g >> 16));

        GE_WriteReg(&pGECtxLocal->halLocalCtx, REG_GE_PRIM_BDX_L, (delta.b & 0xFFFF));
        GE_WriteReg(&pGECtxLocal->halLocalCtx, REG_GE_PRIM_BDX_H, (delta.b >> 16));

        GE_WriteReg(&pGECtxLocal->halLocalCtx, REG_GE_PRIM_ADX, (delta.a & 0xFFFF));

        u16cmd |= GE_LINE_GRADIENT;
    }

    // Draw Line
    GE_WriteReg(&pGECtxLocal->halLocalCtx, REG_GE_PRIM_V0_X, v0->x);
    GE_WriteReg(&pGECtxLocal->halLocalCtx, REG_GE_PRIM_V0_Y, v0->y);
    GE_WriteReg(&pGECtxLocal->halLocalCtx, REG_GE_PRIM_V1_X, v1->x);
    GE_WriteReg(&pGECtxLocal->halLocalCtx, REG_GE_PRIM_V1_Y, v1->y);
    GE_WriteReg(&pGECtxLocal->halLocalCtx, _REG_GE_LINE_LENGTH, dmajor);

    //GE_WriteReg(&pGECtxLocal->halLocalCtx, REG_GE_CMD, GE_PRIM_LINE | u16cmd);

    // clear if line pattern reset
    //u16style = GE_ReadReg(&pGECtxLocal->halLocalCtx, REG_GE_LINE_STYLE) & ~GE_LINEPAT_RST;

    u32Start = v0->x;
    u32End = v1->x;

    if(y_major)
    {
#if 1//(SUPPORT_LINE_LAST_PATCH == 1 )
        if(v1->y <= v0->y)
        {
            if(v1->y >=2)
                GE_WriteReg(&pGECtxLocal->halLocalCtx, REG_GE_PRIM_V1_Y, v1->y-2);
            else {
                GE_WriteReg(&pGECtxLocal->halLocalCtx, REG_GE_PRIM_V1_Y, v1->y);
             }
        }
        else
            GE_WriteReg(&pGECtxLocal->halLocalCtx, REG_GE_PRIM_V1_Y, v1->y+2);
#endif
        for(i=0;i<u32OutWidth;i++)
        {
#if 1//(SUPPORT_LINE_LAST_PATCH == 1 )
            GE_WriteReg(&pGECtxLocal->halLocalCtx, _REG_GE_LINE_LENGTH, dmajor+1);
#else
//            PE_WriteReg(PE_REG_LENGTH, u32Height);
            GE_WriteReg(&pGECtxLocal->halLocalCtx, _REG_GE_LINE_LENGTH, dmajor);
#endif
            GE_WriteReg(&pGECtxLocal->halLocalCtx, REG_GE_PRIM_V0_X, u32Start);
            GE_WriteReg(&pGECtxLocal->halLocalCtx, REG_GE_PRIM_V1_X, u32End);
//            GE_WriteReg(&pGECtxLocal->halLocalCtx, REG_GE_LINE_STYLE, u16style);
            bDstXInv = (u16cmd & (GE_DST_DIR_X_INV))?TRUE:FALSE;
            HAL_GE_AdjustDstWin(&pGECtxLocal->halLocalCtx,bDstXInv);
#ifdef GE_VQ_MIU_HANG_PATCH
            if ((GE_IsVcmdqEnabled(pGECtx) == TRUE) &&
                (u8DstMIU != pGECtxLocal->halLocalCtx.pHALShared->u8VCmdQMiu)
                )
            {
                //printf("\33[0;36m   %s:%d  u8VCmdQMiu = %d  u8SrcMIU = %d u8DstMIU = %d\33[m \n",__FUNCTION__,__LINE__,pGECtxLocal->halLocalCtx.pHALShared->u8VCmdQMiu,u8SrcMIU,u8DstMIU);
                MDrv_GE_WaitIdle(pGECtx);
            }
#endif
            _GE_SEMAPHORE_ENTRY(pGECtx,E_GE_POOL_ID_INTERNAL_REGISTER);
            GE_WriteReg(&pGECtxLocal->halLocalCtx, REG_GE_CMD, GE_PRIM_LINE | u16cmd);
            _GE_SEMAPHORE_RETURN(pGECtx,E_GE_POOL_ID_INTERNAL_REGISTER);

#ifdef GE_VQ_MIU_HANG_PATCH
            if ((GE_IsVcmdqEnabled(pGECtx) == TRUE) &&
                (u8DstMIU != pGECtxLocal->halLocalCtx.pHALShared->u8VCmdQMiu)
                )
            {
                //printf("\33[0;36m   %s:%d  u8VCmdQMiu = %d  u8SrcMIU = %d u8DstMIU = %d\33[m \n",__FUNCTION__,__LINE__,pGECtxLocal->halLocalCtx.pHALShared->u8VCmdQMiu,u8SrcMIU,u8DstMIU);
                MDrv_GE_WaitIdle(pGECtx);
            }
#endif

            u32Start++;
            u32End++;
        }
    }
    else
    {
    #if 1
#if 1//(SUPPORT_LINE_LAST_PATCH == 1 )
        if(v1->x  <= v0->x)
        {
            if(v1->x >=2)
                GE_WriteReg(&pGECtxLocal->halLocalCtx, REG_GE_PRIM_V1_X, v1->x-2);
            else
                {
                GE_WriteReg(&pGECtxLocal->halLocalCtx, REG_GE_PRIM_V1_X, v1->x);
                }
        }
        else
//            PE_WriteReg(PE_REG_PRI_V1_X, u32X2+2);
            GE_WriteReg(&pGECtxLocal->halLocalCtx, REG_GE_PRIM_V1_X, v1->x+2);
#endif
#endif
        u32Start = v0->y;
        u32End = v1->y;
        for (i=0;i<u32OutWidth;i++)
        {
            GE_WriteReg(&pGECtxLocal->halLocalCtx, _REG_GE_LINE_LENGTH, dmajor+1);

            GE_WriteReg(&pGECtxLocal->halLocalCtx, REG_GE_PRIM_V0_Y, u32Start);
            GE_WriteReg(&pGECtxLocal->halLocalCtx, REG_GE_PRIM_V1_Y, u32End);
//            GE_WriteReg(&pGECtxLocal->halLocalCtx, REG_GE_LINE_STYLE, u16style);
            bDstXInv = (u16cmd & (GE_DST_DIR_X_INV))?TRUE:FALSE;
            HAL_GE_AdjustDstWin(&pGECtxLocal->halLocalCtx,bDstXInv);
#ifdef GE_VQ_MIU_HANG_PATCH
            if ((GE_IsVcmdqEnabled(pGECtx) == TRUE) &&
                (u8DstMIU != pGECtxLocal->halLocalCtx.pHALShared->u8VCmdQMiu)
                )
            {
                //printf("\33[0;36m   %s:%d  u8VCmdQMiu = %d  u8SrcMIU = %d u8DstMIU = %d\33[m \n",__FUNCTION__,__LINE__,pGECtxLocal->halLocalCtx.pHALShared->u8VCmdQMiu,u8SrcMIU,u8DstMIU);
                MDrv_GE_WaitIdle(pGECtx);
            }
#endif
            _GE_SEMAPHORE_ENTRY(pGECtx,E_GE_POOL_ID_INTERNAL_REGISTER);
            GE_WriteReg(&pGECtxLocal->halLocalCtx, REG_GE_CMD, GE_PRIM_LINE | u16cmd);
            _GE_SEMAPHORE_RETURN(pGECtx,E_GE_POOL_ID_INTERNAL_REGISTER);

#ifdef GE_VQ_MIU_HANG_PATCH
            if ((GE_IsVcmdqEnabled(pGECtx) == TRUE) &&
                (u8DstMIU != pGECtxLocal->halLocalCtx.pHALShared->u8VCmdQMiu)
                )
            {
                //printf("\33[0;36m   %s:%d  u8VCmdQMiu = %d  u8SrcMIU = %d u8DstMIU = %d\33[m \n",__FUNCTION__,__LINE__,pGECtxLocal->halLocalCtx.pHALShared->u8VCmdQMiu,u8SrcMIU,u8DstMIU);
                MDrv_GE_WaitIdle(pGECtx);
            }
#endif

//            PE_WriteReg(PE_REG_LENGTH, u32Width+1);
//            PE_WriteReg(PE_REG_PRI_V0_Y, u32Start);
//            PE_WriteReg(PE_REG_PRI_V1_Y, u32End);
//            PE_WriteReg(PE_REG_CMD, u32Value2|_u32Reg60hFlag);
            u32Start++;
            u32End++;
        }
    }


#if DYNAMIC_POWER_ON_OFF
    GE_WaitIdle(&pGECtxLocal->halLocalCtx);
    MDrv_PWR_ClockTurnOff(E_PWR_MODULE_GE);
#endif

    GE_RETURN(pGECtx,E_GE_OK);
}


//-------------------------------------------------------------------------------------------------
/// GE Primitive Drawing - RECT
/// @param  pGECtx                     \b IN: GE context
/// @param  rect                    \b IN: pointer to position of RECT
/// @param  color                   \b IN: color of RECT
/// @param  color2                  \b IN: 2nd color of gradient
/// @param  flags                   \b IN: RECT drawing flags
/// @return @ref GE_Result
//-------------------------------------------------------------------------------------------------
GE_Result MDrv_GE_FillRect(GE_Context *pGECtx, GE_Rect *rect, MS_U32 color, MS_U32 color2, MS_U32 flags)
{
    GE_CTX_LOCAL *pGECtxLocal = (GE_CTX_LOCAL*)pGECtx;

    GE_DBG("%s\n", __FUNCTION__);
    GE_ASSERT((rect), E_GE_FAIL_PARAM, "%s: Null pointer\n", __FUNCTION__);

    GE_ENTRY(pGECtx);

    if ( (rect->width == 0) || (rect->height == 0) )
    {
        GE_RETURN(pGECtx,E_GE_FAIL_PARAM);
    }
    if (GE_FastClip(pGECtx, rect->x, rect->y, rect->x+rect->width-1, rect->y+rect->height-1))
    {
        GE_RETURN(pGECtx,E_GE_NOT_DRAW);
    }

    GE_Restore_PaletteContext(pGECtxLocal);

    return HAL_GE_FillRect(&pGECtxLocal->halLocalCtx, rect, color, color2, flags);
}

#ifdef CONFIG_GFX_TRAPZOID
//-------------------------------------------------------------------------------------------------
/// GE Primitive Drawing - Trapezoid
/// @param  pGECtx                     \b IN: GE context
/// @param  pGeTrapezoid                    \b IN: pointer to position of RECT
/// @param  u32ColorS                   \b IN: color of RECT
/// @param  u32ColorE                  \b IN: 2nd color of gradient
/// @param  u32Flags                   \b IN: Trapezoid drawing flags
/// @return @ref GE_Result
//-------------------------------------------------------------------------------------------------
GE_Result MDrv_GE_FillTrapezoid(GE_Context *pGECtx, GE_Trapezoid *pGeTrapezoid, MS_U32 u32ColorS, MS_U32 u32ColorE, MS_U32 u32Flags)
{
    GE_Result geResult;
    GE_CTX_LOCAL *pGECtxLocal = (GE_CTX_LOCAL*)pGECtx;
    GE_Normalized_Trapezoid geNormTrapezoid;
    GE_ColorDelta geColorDeltaX, geColorDeltaY;
    GE_ColorDelta *pGeColorDeltaX=NULL, *pGeColorDeltaY=NULL;
    MS_BOOL bYTrapezoid = (u32Flags & E_GE_FLAG_TRAPEZOID_Y) ? TRUE : FALSE;

    GE_DBG("%s\n", __FUNCTION__);
    GE_ASSERT((pGeTrapezoid), E_GE_FAIL_PARAM, "%s: Null pointer\n", __FUNCTION__);

    GE_ENTRY(pGECtx);

    //check parameters:
    if(((pGeTrapezoid->u16Y0>=pGeTrapezoid->u16Y1)&&(bYTrapezoid==FALSE))
        || ((pGeTrapezoid->u16X0<=pGeTrapezoid->u16X1)&&(bYTrapezoid==TRUE))
        || (0==pGeTrapezoid->u16DeltaTop)
        || (0==pGeTrapezoid->u16DeltaBottom))
    {
        GE_RETURN(pGECtx, E_GE_FAIL_PARAM);
    }

    if(GE_FastClipTrapezoid(pGECtx, pGeTrapezoid->u16X0, pGeTrapezoid->u16Y0, pGeTrapezoid->u16X1, pGeTrapezoid->u16Y1, pGeTrapezoid->u16DeltaTop, pGeTrapezoid->u16DeltaBottom, bYTrapezoid))
    {
        GE_RETURN(pGECtx,E_GE_NOT_DRAW);
    }

#if DYNAMIC_POWER_ON_OFF
    MDrv_PWR_ClockTurnOn(E_PWR_MODULE_GE);
#endif

    //Transfer Driver definition of Trapezoid to Hardware definition of trapezoid:
    GE_NormalizeTrapezoidDef(pGeTrapezoid, &geNormTrapezoid, bYTrapezoid);

    //Prepare DeltaX, DeltaY:
    if(u32Flags & E_GE_FLAG_TRAPEZOID_GRADIENT_X)
    {
        pGeColorDeltaX = &geColorDeltaX;
        if(bYTrapezoid)
        {
            GE_CalcColorDelta(u32ColorS, u32ColorE, (pGeTrapezoid->u16X0-pGeTrapezoid->u16X1), pGeColorDeltaX);
        }
        else
        {
            GE_CalcColorDelta(u32ColorS, u32ColorE, (pGeTrapezoid->u16DeltaTop), pGeColorDeltaX);
        }
    }

    if(u32Flags & E_GE_FLAG_TRAPEZOID_GRADIENT_Y)
    {
        pGeColorDeltaY = &geColorDeltaY;
        if(bYTrapezoid)
        {
            GE_CalcColorDelta(u32ColorS, u32ColorE, (pGeTrapezoid->u16DeltaTop), pGeColorDeltaY);
        }
        else
        {
            GE_CalcColorDelta(u32ColorS, u32ColorE, (pGeTrapezoid->u16Y1-pGeTrapezoid->u16Y0), pGeColorDeltaY);
        }
    }

    GE_Restore_PaletteContext(pGECtxLocal);

    //Actual Bitblit:
    geResult = GE_FillTrapezoid(&pGECtxLocal->halLocalCtx, bYTrapezoid, &geNormTrapezoid, u32ColorS, pGeColorDeltaX, pGeColorDeltaY);
    if(E_GE_OK != geResult)
    {
        geResult = GE_TrapezoidFill(pGECtxLocal, bYTrapezoid, &geNormTrapezoid, u32ColorS, u32ColorE, pGeColorDeltaX, pGeColorDeltaY);
    }

#if DYNAMIC_POWER_ON_OFF
    GE_WaitIdle();
    MDrv_PWR_ClockTurnOff(E_PWR_MODULE_GE);
#endif

    GE_RETURN(pGECtx, geResult);
}
#endif
//-------------------------------------------------------------------------------------------------
/// GE Primitive Drawing - BLT
/// @param  pGECtx                     \b IN: GE context
/// @param  src                     \b IN: source RECT of bitblt
/// @param  dst                     \b IN: destination RECT of bitblt
/// @param  flags                   \b IN: RECT drawing flags
/// @param  scaleinfo                     \b IN: Customed scale info
/// @return @ref GE_Result
/// @note\n
/// FMT_I1,I2,I4,I8: should be 64-bit aligned in base and pitch to do stretch bitblt.\n
/// FMT_YUV422: the rectangle should be YUV422 block aligned. (4 bytes for 2 pixels in horizontal)
//-------------------------------------------------------------------------------------------------
GE_Result MDrv_GE_BitBltEX(GE_Context *pGECtx, GE_Rect *src, GE_DstBitBltType *dst, MS_U32 flags, GE_ScaleInfo* scaleinfo)
{
    MS_U16              u16tmp,Srcfmt,Dstfmt;

    GE_CTX_LOCAL *pGECtxLocal = (GE_CTX_LOCAL*)pGECtx;

    GE_DBG("%s\n", __FUNCTION__);
    GE_ASSERT((src), E_GE_FAIL_PARAM, "%s: Null pointer\n", __FUNCTION__);
    GE_ASSERT((dst), E_GE_FAIL_PARAM, "%s: Null pointer\n", __FUNCTION__);

    GE_ENTRY(pGECtx);

    if ( (src->width == 0) || (src->height == 0) || (dst->dstblk.width == 0) || (dst->dstblk.height == 0) )
    {
        GE_RETURN(pGECtx,E_GE_FAIL_PARAM);
    }
    if ( GE_FastClip(pGECtx,dst->dstblk.x, dst->dstblk.y, dst->dstblk.x+dst->dstblk.width-1, dst->dstblk.y+dst->dstblk.height-1) )
    {
        if (flags & (
                      E_GE_FLAG_BLT_ITALIC      |
                      E_GE_FLAG_BLT_ROTATE_90   |
                      E_GE_FLAG_BLT_ROTATE_180  |
                      E_GE_FLAG_BLT_ROTATE_270  ))
        {
            // SKIP fast clipping

            //[TODO] Do fast clipping precisely for rotation and italic
        }
        else
        {
            GE_RETURN(pGECtx,E_GE_NOT_DRAW);
        }
    }

    // Check overlap
    if (flags & E_GE_FLAG_BLT_OVERLAP)
    {
        if (flags & (
                      E_GE_FLAG_BLT_STRETCH     |
                      E_GE_FLAG_BLT_ITALIC      |
                      E_GE_FLAG_BLT_MIRROR_H    |
                      E_GE_FLAG_BLT_MIRROR_V    |
                      E_GE_FLAG_BLT_ROTATE_90   |
                      E_GE_FLAG_BLT_ROTATE_180  |
                      E_GE_FLAG_BLT_ROTATE_270  ))
        {
            GE_RETURN(pGECtx,E_GE_FAIL_OVERLAP);
        }
    }

    // Check italic
    if (flags & E_GE_FLAG_BLT_ITALIC)
    {
        if (flags & (
                      E_GE_FLAG_BLT_MIRROR_H    |
                      E_GE_FLAG_BLT_MIRROR_V    |
                      E_GE_FLAG_BLT_ROTATE_90   |
                      E_GE_FLAG_BLT_ROTATE_180  |
                      E_GE_FLAG_BLT_ROTATE_270  ))
        {
            GE_RETURN(pGECtx,E_GE_FAIL_ITALIC);
        }
    }

    u16tmp = HAL_GE_GetFmt(&pGECtxLocal->halLocalCtx);
    Srcfmt = (u16tmp&GE_SRC_FMT_MASK)>>GE_SRC_FMT_SHFT;
    Dstfmt = (u16tmp&GE_DST_FMT_MASK)>>GE_DST_FMT_SHFT;
    if((GE_FMT_I8 == Srcfmt) && (GE_FMT_I8 != Dstfmt) )
    {
        pGECtxLocal->bSrcPaletteOn = TRUE;
    }
    else
    {
        pGECtxLocal->bSrcPaletteOn = FALSE;
    }
    GE_Restore_PaletteContext(pGECtxLocal);

    HAL_GE_BitBltEX(&pGECtxLocal->halLocalCtx, src, dst, flags, scaleinfo);

    GE_RETURN(pGECtx,E_GE_OK);
}
#ifdef CONFIG_GFX_TRAPZOID
//-------------------------------------------------------------------------------------------------
/// GE Primitive Drawing - Trapezoid BLT
/// Case 1: HW supported Trapezoid, done with HAL Level GE Functions;
/// Case 2: HW does not support Trapezoid, done with DRV Level GE Functions, implemented by Rect Bitblit;
/// @param  pGECtx                     \b IN: GE context
/// @param  pSrcRect                     \b IN: source Trapezoid of bitblt
/// @param  pDstRect                     \b IN: destination Trapezoid of bitblt
/// @param  u32Flags                   \b IN: RECT drawing flags
/// @param  pScaleinfo                   \b IN: customized scale info
/// @return @ref GE_Result
/// @note\n
/// FMT_I1,I2,I4,I8: should be 64-bit aligned in base and pitch to do stretch bitblt.\n
/// FMT_YUV422: the rectangle should be YUV422 block aligned. (4 bytes for 2 pixels in horizontal)
//-------------------------------------------------------------------------------------------------
GE_Result MDrv_GE_BitBltEX_Trape(GE_Context *pGECtx,GE_Rect *pSrcRect, GE_DstBitBltType *pDstRect, MS_U32 u32Flags, GE_ScaleInfo* pScaleinfo)
{
    GE_Result geResult;
    GE_CTX_LOCAL *pGECtxLocal = (GE_CTX_LOCAL*)pGECtx;
    GE_Trapezoid *pGeTrapezoid = &pDstRect->dsttrapeblk;
    GE_Normalized_Trapezoid geNormTrapezoid;
    GE_ScaleInfo geScaleinfo, *pGeScaleInfo = (u32Flags & E_GE_FLAG_BYPASS_STBCOEF) ? pScaleinfo : &geScaleinfo;
    MS_BOOL bYTrapezoid = (u32Flags & E_GE_FLAG_TRAPEZOID_Y) ? TRUE : FALSE;

    GE_DBG("%s\n", __FUNCTION__);

    GE_ENTRY(pGECtx);

    //check parameters:
    if(NULL == pGeScaleInfo)
    {
        GE_RETURN(pGECtx, E_GE_FAIL_PARAM);
    }

    if((pSrcRect->width == 0) || (pSrcRect->height == 0)
        || ((pGeTrapezoid->u16Y0>=pGeTrapezoid->u16Y1)&&(bYTrapezoid==FALSE))
        || ((pGeTrapezoid->u16X0<=pGeTrapezoid->u16X1)&&(bYTrapezoid==TRUE))
        || (0==pGeTrapezoid->u16DeltaTop)
        || (0==pGeTrapezoid->u16DeltaBottom))
    {
        GE_RETURN(pGECtx, E_GE_FAIL_PARAM);
    }

    if(GE_FastClipTrapezoid(pGECtx, pGeTrapezoid->u16X0, pGeTrapezoid->u16Y0, pGeTrapezoid->u16X1, pGeTrapezoid->u16Y1, pGeTrapezoid->u16DeltaTop, pGeTrapezoid->u16DeltaBottom, bYTrapezoid))
    {
        if(u32Flags & (E_GE_FLAG_BLT_ITALIC))
        {
            // SKIP fast clipping
            //[TODO] Do fast clipping precisely for italic
        }
        else
        {
            GE_RETURN(pGECtx,E_GE_NOT_DRAW);
        }
    }

    // Check overlap
    if(u32Flags & E_GE_FLAG_BLT_OVERLAP)
    {
        if(FALSE == GE_TrapezoidOverlap(pSrcRect, pGeTrapezoid))
        {
            u32Flags &= (~E_GE_FLAG_BLT_OVERLAP);
        }else
        {
            GE_RETURN(pGECtx, E_GE_FAIL_OVERLAP);
        }
    }

    // Check italic
    if(u32Flags & E_GE_FLAG_BLT_ITALIC)
    {
        if(u32Flags & (
                      E_GE_FLAG_BLT_MIRROR_H |
                      E_GE_FLAG_BLT_MIRROR_V  ))
        {
            GE_RETURN(pGECtx,E_GE_FAIL_ITALIC);
        }
    }

    // Reset-Rotate: Rotate setting won't take effect for Trapezoid bitblit:
    u32Flags &= ~E_GE_FLAG_BLT_ROTATE_270;

#if DYNAMIC_POWER_ON_OFF
        MDrv_PWR_ClockTurnOn(E_PWR_MODULE_GE);
#endif

    //Transfer Driver definition of Trapezoid to Hardware definition of trapezoid:
    GE_NormalizeTrapezoidDef(pGeTrapezoid, &geNormTrapezoid, bYTrapezoid);
    //Set Scale Info:
    if(!(u32Flags & E_GE_FLAG_BYPASS_STBCOEF))
    {
        if(bYTrapezoid)
        {
            GE_CalcBltScaleRatio(&pGECtxLocal->halLocalCtx, pSrcRect->width, pSrcRect->height, (geNormTrapezoid.u16X0-geNormTrapezoid.u16X1+1), pGeTrapezoid->u16DeltaTop, pGeScaleInfo);
        }
        else
        {
            GE_CalcBltScaleRatio(&pGECtxLocal->halLocalCtx, pSrcRect->width, pSrcRect->height, pGeTrapezoid->u16DeltaTop, (geNormTrapezoid.u16Y1-geNormTrapezoid.u16Y0+1), pGeScaleInfo);
        }
    }

    GE_Restore_PaletteContext(pGECtxLocal);

    //Actual Bitblit:
    geResult = GE_BitBltEX_Trape(&pGECtxLocal->halLocalCtx, pSrcRect, &geNormTrapezoid, u32Flags, pGeScaleInfo);
    if(E_GE_OK != geResult)
    {
        geResult = GE_TrapezoidBlit(pGECtxLocal, pSrcRect, &geNormTrapezoid, u32Flags, pGeScaleInfo);
    }

#if DYNAMIC_POWER_ON_OFF
    GE_WaitIdle(&pGECtxLocal->halLocalCtx);
    MDrv_PWR_ClockTurnOff(E_PWR_MODULE_GE);
#endif

    GE_RETURN(pGECtx,(GE_Result)geResult);
}
#endif


//-------------------------------------------------------------------------------------------------
/// Wait GE engine idle
/// @param  pGECtx                     \b IN: GE context
/// @return @ref GE_Result
//-------------------------------------------------------------------------------------------------
GE_Result MDrv_GE_WaitIdle(GE_Context *pGECtx)
{
    GE_CTX_LOCAL *pGECtxLocal = (GE_CTX_LOCAL*)pGECtx;

    GE_DBG("%s\n", __FUNCTION__);
    GE_ENTRY(pGECtx);

    GE_WaitIdle(&pGECtxLocal->halLocalCtx);

    GE_RETURN(pGECtx,E_GE_OK);
}

#if GE_LOCK_SUPPORT
#if defined(MSOS_TYPE_LINUX) || defined(MSOS_TYPE_LINUX_KERNEL)
static void GE_AtExit(void* pInstance)
{
#if(GFX_UTOPIA20)
    GFX_INSTANT_PRIVATE*    psGFXInstPriTmp;
    UtopiaInstanceGetPrivate(pInstance, (void**)&psGFXInstPriTmp);
#endif

    GE_CTX_LOCAL *pGECtxLocal = &g_drvGECtxLocal;
#if GE_USE_HW_SEM
    MS_U16 GE_HWSemaphore_count =0;
    MS_U16 GE_GetHWSemaphoreID=0;
#endif

    if(pGECtxLocal->s32GE_Recursive_Lock_Cnt > 0)
    {
        pGECtxLocal->s32GE_Recursive_Lock_Cnt = 0;
        MsOS_ReleaseMutex(pGECtxLocal->s32GEMutex);
#if GE_USE_HW_SEM
        while(MDrv_SEM_Free_Resource(GESEMID, pGECtxLocal->u32GESEMID) ==FALSE)
        {
           GE_HWSemaphore_count++;
           if(GE_HWSemaphore_count>10000)
           {
               MDrv_SEM_Get_ResourceID(GESEMID, &GE_GetHWSemaphoreID);
               GE_D_DBUG("\33[0;31m [Return While][%s][%d] GE_GetHWSemphoreID=%tx; u32GESEMID=%tx\33[m;\n",__FUNCTION__,__LINE__, (ptrdiff_t)GE_GetHWSemaphoreID, (ptrdiff_t)pGECtxLocal->u32GESEMID);
               GE_HWSemaphore_count =0;
           }
        }
#endif
    }
}
#endif
#endif

void MDrv_GE_Exit(void* pInstance)
{
#if defined(CONFIG_GFX_TAG_INTERRUPT) && (__GE_WAIT_TAG_MODE == __USE_GE_INT_MODE)
    GE_CTX_LOCAL *pGECtxLocal = (GE_CTX_LOCAL*)pInstance;
#endif

#if GE_LOCK_SUPPORT
    #if defined(MSOS_TYPE_LINUX) || defined(MSOS_TYPE_LINUX_KERNEL)
        GE_AtExit(pInstance);
    #else
        GE_D_DBUG("not enable MSOS_TYPE_LINUX\n");
    #endif
#else
    GE_D_ERR("not enable GE_LOCK_SUPPORT\n");
#endif

#if defined(CONFIG_GFX_TAG_INTERRUPT) && (__GE_WAIT_TAG_MODE == __USE_GE_INT_MODE)
    HAL_GE_exit(&pGECtxLocal->halLocalCtx);
#endif

}

//-------------------------------------------------------------------------------------------------
/// GE DDI Initialization
/// @param  cfg                     \b IN: @ref GE_Config
/// @param  ppGECtx                     \b IN: GE context
/// @return @ref GE_Result
//-------------------------------------------------------------------------------------------------
GE_Result MDrv_GE_Init(void* pInstance, GE_Config *cfg, GE_Context **ppGECtx)
{
    MS_VIRT regaddr = 0x0;
    MS_PHY u32regsize = 0x0;
    GE_CTX_LOCAL *pGECtxLocal = NULL;
    GE_CTX_SHARED *pGEShared;
    MS_BOOL bNeedInitShared = FALSE;

    GE_DBG("%s\n", __FUNCTION__);
#if(GFX_UTOPIA20)
        GFX_INSTANT_PRIVATE*    psGFXInstPriTmp;
        UtopiaInstanceGetPrivate(pInstance, (void**)&psGFXInstPriTmp);
        pGECtxLocal = &g_drvGECtxLocal;
        pGECtxLocal->ctxHeader.pInstance =pInstance;
#else
        pGECtxLocal = &g_drvGECtxLocal;
#endif

#if defined(MSOS_TYPE_LINUX) || defined(MSOS_TYPE_LINUX_KERNEL)
    MS_U32 u32ShmId;
    MS_VIRT VAddr;
    MS_U32 u32BufSize;

    if (FALSE == MsOS_SHM_GetId((MS_U8*)"GE driver", sizeof(GE_CTX_SHARED), &u32ShmId, &VAddr, &u32BufSize, MSOS_SHM_QUERY))  // Richard: Query if there is ¡§IPC driver¡¨ exist in share memory
    {
        if (FALSE == MsOS_SHM_GetId((MS_U8*)"GE driver", sizeof(GE_CTX_SHARED), &u32ShmId, &VAddr, &u32BufSize, MSOS_SHM_CREATE)) // Richard: Create ¡§IPC driver¡¨ exist in share memory
        {
            GE_DBG("SHM allocation failed!\n");
            *ppGECtx = NULL;
            return E_GE_FAIL;
        }
        GE_DBG("[%s][%d] This is first initial 0x%08x\n", __FUNCTION__, __LINE__, VAddr);   // Richard: The process first calling this function should print this out
        memset( (MS_U8*)VAddr, 0, sizeof(GE_CTX_SHARED));
        bNeedInitShared = TRUE;

    }
    pGEShared = (GE_CTX_SHARED*)VAddr;
#else
    pGEShared =  &g_drvGECtxShared;
    bNeedInitShared = TRUE;
    memset( pGEShared, 0, sizeof(GE_CTX_SHARED));
#endif

#if (GE_USE_HW_SEM && defined(MSOS_TYPE_OPTEE))
    pGECtxLocal->u32GESEMID = SEM_HK51ID;
#else
    #ifdef __AEON__
    pGECtxLocal->u32GESEMID = SEM_MWMipsID;
    #else
    pGECtxLocal->u32GESEMID = SEM_HKAeonID;
    #endif
#endif
    pGECtxLocal->u16GEPrevSEMID = pGECtxLocal->u32GESEMID;
    pGECtxLocal->bIsComp = cfg->bIsCompt;
    pGECtxLocal->bSrcPaletteOn = FALSE;
    pGECtxLocal->bDstPaletteOn = FALSE;

    // need add lock for protect SHM.

    pGECtxLocal->u32CTXInitMagic = 0x55aabeef;

    pGECtxLocal->pSharedCtx = pGEShared;
    GE_Init_HAL_Context(&pGECtxLocal->halLocalCtx, &pGEShared->halSharedCtx, bNeedInitShared);

    // save Init parameter
    pGEShared->bInit=TRUE;
    pGEShared->u32VCmdQSize=cfg->u32VCmdQSize;
    pGEShared->PhyVCmdQAddr=cfg->PhyVCmdQAddr;
    pGEShared->bIsHK=cfg->bIsHK;
    pGEShared->bIsCompt=cfg->bIsCompt;

    pGECtxLocal->halLocalCtx.bIsComp = pGECtxLocal->bIsComp;

    pGECtxLocal->u32GEClientId = ++pGEShared->u32GEClientAllocator;
    if(0 == pGECtxLocal->u32GEClientId)
         pGECtxLocal->u32GEClientId = ++pGEShared->u32GEClientAllocator;

    pGECtxLocal->ctxHeader.u32GE_DRV_Version =  GE_API_MUTEX;
    pGECtxLocal->ctxHeader.bGEMode4MultiProcessAccess  =  GE_SWTABLE;
    pGECtxLocal->ctxHeader.s32CurrentProcess =  (MS_S32)getpid();
    pGECtxLocal->ctxHeader.s32CurrentThread = 0;

    if (pGECtxLocal->ctxHeader.bGEMode4MultiProcessAccess ==FALSE)
        pGECtxLocal->pSharedCtx->halSharedCtx.bGE_DirectToReg =TRUE;
    else
        pGECtxLocal->pSharedCtx->halSharedCtx.bGE_DirectToReg =FALSE;
    *ppGECtx = (GE_Context *)pGECtxLocal;

    if(!OSAL_GE_GetMapBase(&regaddr, &u32regsize))
    {
        GE_DBG("Get GE IOMAP Base faill!\n");
        return E_GE_FAIL;
    }

    GE_Set_IOMap_Base(&pGECtxLocal->halLocalCtx, regaddr);

    if (!OSAL_GE_GetMapBase2(&regaddr, &u32regsize))
    {
        GE_DBG("Get Chip bank IOMAP Base faill!\n");
        return E_GE_FAIL;
    }
    GE_Set_IOMap_Base2(&pGECtxLocal->halLocalCtx, regaddr);

    pGECtxLocal->s32GE_Recursive_Lock_Cnt = 0;
#if GE_LOCK_SUPPORT
    pGECtxLocal->s32GELock  = -1;

    pGECtxLocal->s32GEMutex = MsOS_CreateMutex(E_MSOS_FIFO, "GE_Mutex", MSOS_PROCESS_SHARED);
    if (pGECtxLocal->s32GEMutex < 0)
    {
            GE_ERR("%s Fail\n", __FUNCTION__);
            return E_GE_FAIL;
    }

#endif

#if (ENABLE_PWR)
    MDrv_PWR_ClockTurnOn(E_PWR_MODULE_GE);
#endif

    GE_Get_Resource(*ppGECtx,TRUE);
    // Enable Command Queue
    (*ppGECtx)->pBufInfo.srcfmt = ((GE_BufFmt)E_GE_FMT_ARGB1555);
    (*ppGECtx)->pBufInfo.dstfmt = ((GE_BufFmt)E_GE_FMT_ARGB1555);


    GE_Init(&pGECtxLocal->halLocalCtx, cfg);
    //Init GE Palette from HW:
    GE_InitCtxHalPalette(&pGECtxLocal->halLocalCtx);

    // Default setting : To avoid GE issues commands too frequently when VC in enabled
    MDrv_GE_SetVCmd_W_Thread(*ppGECtx, 0x5);
    MDrv_GE_SetVCmd_R_Thread(*ppGECtx, 0x5);

    if (bNeedSetActiveCtrlMiu1)
        GE_SetActiveCtrlMiu1(&pGECtxLocal->halLocalCtx);

    pGECtxLocal->pSharedCtx->bNotFirstInit = TRUE;

    GE_Free_Resource(*ppGECtx, TRUE);
    return E_GE_OK;
}

GE_Result MDrv_GE_GetConfig(GE_Config *cfg)
{
    GE_CTX_SHARED *pGEShared;
#if defined(MSOS_TYPE_LINUX) || defined(MSOS_TYPE_LINUX_KERNEL)
    MS_U32 u32ShmId;
    MS_VIRT VAddr;
    MS_U32 u32BufSize;

    if (FALSE == MsOS_SHM_GetId((MS_U8*)"GE driver", sizeof(GE_CTX_SHARED), &u32ShmId, &VAddr, &u32BufSize, MSOS_SHM_QUERY))  // Richard: Query if there is gIPC driverh exist in share memory
    {
        if (FALSE == MsOS_SHM_GetId((MS_U8*)"GE driver", sizeof(GE_CTX_SHARED), &u32ShmId, &VAddr, &u32BufSize, MSOS_SHM_CREATE)) // Richard: Create gIPC driverh exist in share memory
        {
            GE_DBG("SHM allocation failed!\n");
            return E_GE_FAIL;
        }
        GE_DBG("[%s][%d] This is first initial 0x%08x\n", __FUNCTION__, __LINE__, VAddr);   // Richard: The process first calling this function should print this out
        memset( (MS_U8*)VAddr, 0, sizeof(GE_CTX_SHARED));
    }
    pGEShared = (GE_CTX_SHARED*)VAddr;
#else
    pGEShared =  &g_drvGECtxShared;
    memset( pGEShared, 0, sizeof(GE_CTX_SHARED));
#endif
    if(FALSE==pGEShared->bInit)
        return E_GE_FAIL;
    cfg->u32VCmdQSize=pGEShared->u32VCmdQSize;
    cfg->PhyVCmdQAddr=pGEShared->PhyVCmdQAddr;
    cfg->bIsHK=pGEShared->bIsHK;
    cfg->bIsCompt=pGEShared->bIsCompt;

    return E_GE_OK;
}

GE_Result MDrv_GE_Chip_Proprity_Init(GE_Context *pGECtx, GE_CHIP_PROPERTY **ppGeChipPro)
{
    if ( pGECtx == NULL )
    {
        GE_D_ERR("\n%s, %d, 1st param. can't be null pointer\n", __FUNCTION__, __LINE__);
        return E_GE_FAIL;
    }
    GE_CTX_LOCAL *pGECtxLocal = (GE_CTX_LOCAL*)pGECtx;

    GE_DBG("%s\n", __FUNCTION__);
    GE_ENTRY(pGECtx);

    GE_Chip_Proprity_Init(&pGECtxLocal->halLocalCtx);
    *ppGeChipPro = pGECtxLocal->halLocalCtx.pGeChipPro;

    GE_RETURN(pGECtx, E_GE_OK);
}

//[RESERVED]
//-------------------------------------------------------------------------------------------------
/// Shutdown GE HW.
/// @param  pGECtx                     \b IN: GE context
/// @param  miu                     \b IN: miu being procted
/// @param  addr_low                     \b IN: low address of specified area
/// @param  addr_high                     \b IN: high address of specified area
/// @param  eWPType                     \b IN: GE protect operator
/// @return @ref GE_Result
//-------------------------------------------------------------------------------------------------
GE_Result MDrv_GE_WriteProtect(GE_Context *pGECtx, MS_U8 miu, MS_PHY addr_low, MS_PHY addr_high, GE_WPType eWPType)
{
    GE_Result           ret;
    GE_CTX_LOCAL *pGECtxLocal = (GE_CTX_LOCAL*)pGECtx;

    GE_DBG("%s\n", __FUNCTION__);
    GE_ENTRY(pGECtx);

    if ((addr_high&0x07FFFFFF) < (addr_low&0x07FFFFFF))
    {
        GE_RETURN(pGECtx,E_GE_FAIL);
    }

    ret = GE_WriteProtect(&pGECtxLocal->halLocalCtx, miu, addr_low, addr_high, eWPType);

    GE_RETURN(pGECtx,ret);
}
//[RESERVED]


//-------------------------------------------------------------------------------------------------
/// Shutdown GE HW.
/// @param  pGECtx                     \b IN: GE context
/// @return @ref GE_Result
//-------------------------------------------------------------------------------------------------
GE_Result MDrv_GE_PowerOff(GE_Context *pGECtx)
{
    GE_CTX_LOCAL *pGECtxLocal = (GE_CTX_LOCAL*)pGECtx;

    GE_DBG("%s\n", __FUNCTION__);
    GE_ENTRY(pGECtx);

    GE_WaitIdle(&pGECtxLocal->halLocalCtx);
#if (ENABLE_PWR)
    MDrv_PWR_ClockTurnOff(E_PWR_MODULE_GE);
#endif

    GE_RETURN(pGECtx,E_GE_OK);
}

//-------------------------------------------------------------------------------------------------
/// Get GE buffer format capabilities
/// @param  pGECtx                     \b IN: GE context
/// @param fmt                      \b IN:  specified GE buffer format
/// @param type                     \b IN:  type of buffer
/// @param caps                     \b OUT: pointer of GE_FmtCaps returned for specified format
/// @return @ref GE_Result
//-------------------------------------------------------------------------------------------------
GE_Result MDrv_GE_GetFmtCaps(GE_Context *pGECtx, GE_BufFmt fmt, GE_BufType type, GE_FmtCaps *caps)
{
    GE_Result           ret;
    GE_CTX_LOCAL *pGECtxLocal = (GE_CTX_LOCAL*)pGECtx;

    GE_DBG("%s\n", __FUNCTION__);
    GE_ENTRY(pGECtx);

    ret = GE_GetFmtCaps(&pGECtxLocal->halLocalCtx, fmt, type, caps);

    GE_RETURN(pGECtx,ret);
}


//-------------------------------------------------------------------------------------------------
/// Set GE alpha value of ARGB1555 mode.
/// @param pGECtx                      \b IN:  Pointer to GE context.
/// @param coef                     \b IN:  Alpha coef for ARGB1555.
/// @return @ref GE_Result
//-------------------------------------------------------------------------------------------------
GE_Result MDrv_GE_SetAlpha_ARGB1555(GE_Context *pGECtx, MS_U8 coef)
{
    MS_U16 u16tmp;
    GE_CTX_LOCAL *pGECtxLocal = (GE_CTX_LOCAL*)pGECtx;

    GE_DBG("%s\n", __FUNCTION__);
    GE_ENTRY(pGECtx);

    u16tmp = GE_ReadReg(&pGECtxLocal->halLocalCtx, REG_GE_BLEND);
    u16tmp |= (MS_U16)coef<<8; ///- Set Alpha for ARGB1555
    GE_WriteReg(&pGECtxLocal->halLocalCtx, REG_GE_BLEND, u16tmp);

    GE_RETURN(pGECtx,E_GE_OK);
}


//-------------------------------------------------------------------------------------------------
/// Get GE alpha value of ARGB1555 mode.
/// @param pGECtx                      \b IN:  Pointer to GE context.
/// @param coef                     \b OUT:  Alpha coef for ARGB1555.
/// @return @ref GE_Result
//-------------------------------------------------------------------------------------------------
GE_Result MDrv_GE_GetAlpha_ARGB1555(GE_Context *pGECtx, MS_U8* coef)
{
    MS_U16 u16tmp;
    GE_CTX_LOCAL *pGECtxLocal = (GE_CTX_LOCAL*)pGECtx;

    GE_DBG("%s\n", __FUNCTION__);
    GE_ENTRY(pGECtx);

    if(coef == NULL)
        GE_RETURN(pGECtx,E_GE_FAIL_PARAM);

    u16tmp = GE_ReadReg(&pGECtxLocal->halLocalCtx, REG_GE_BLEND);

    *coef = (u16tmp >> 8) &0xff;

    GE_RETURN(pGECtx,E_GE_OK);
}


//-------------------------------------------------------------------------------------------------
/// Set GE TAGID value.
/// The TAGID is taken as GE engine serialization.
/// @param pGECtx                      \b IN:  Pointer to GE context.
/// @param u16tag                     \b IN:  Tag value to be set.
/// @return @ref GE_Result
//-------------------------------------------------------------------------------------------------
GE_Result MDrv_GE_SetTAGID(GE_Context *pGECtx, MS_U16 u16tag)
{
    GE_CTX_LOCAL *pGECtxLocal = (GE_CTX_LOCAL*)pGECtx;
    MS_U8 i=0;
    GE_ENTRY(pGECtx);

    //GE hw will kick off cmd in pairs, so send tagID twice to force kick off
    for(i=0;i<(GE_WordUnit>>2);i++)
    {
        GE_WriteReg(&pGECtxLocal->halLocalCtx, REG_GE_TAG, u16tag);
    }

    GE_RETURN(pGECtx,E_GE_OK);
}

//-------------------------------------------------------------------------------------------------
/// Get GE TAGID value.
/// The TAGID is taken as GE engine serialization.
/// @param pGECtx                      \b IN:  Pointer to GE context.
/// @param u16tag                     \b OUT:  Tag value to be set.
/// @return @ref GE_Result
//-------------------------------------------------------------------------------------------------
GE_Result MDrv_GE_GetTAGID(GE_Context *pGECtx, MS_U16* u16tag)
{
    GE_CTX_LOCAL *pGECtxLocal = (GE_CTX_LOCAL*)pGECtx;

    GE_ENTRY(pGECtx);

    *u16tag = GE_ReadReg(&pGECtxLocal->halLocalCtx, REG_GE_TAG);

    GE_RETURN(pGECtx,E_GE_OK);

}

//-------------------------------------------------------------------------------------------------
/// Wait GE TagID back
/// @param pGECtx                      \b IN:  Pointer to GE context.
/// @param  tagID                     \b IN: tag id number for wating
/// @return @ref GE_Result
//-------------------------------------------------------------------------------------------------
GE_Result MDrv_GE_WaitTAGID(GE_Context *pGECtx, MS_U16 tagID)
{
     GE_CTX_LOCAL *pGECtxLocal = (GE_CTX_LOCAL*)pGECtx;

     GE_ENTRY(pGECtx);

     GE_WaitTAGID(&pGECtxLocal->halLocalCtx, tagID);

     GE_RETURN(pGECtx,E_GE_OK);
}


//-------------------------------------------------------------------------------------------------
/// Get Next GE TagID
/// Get the TAGID in bStepTagBefore steps.
/// @param  pGECtx                     \b IN: GE context
/// @param  bStepTagBefore         \b IN: Steps to the NEXT TAGID.
/// @return tagID
//-------------------------------------------------------------------------------------------------
GE_Result MDrv_GE_GetNextTAGID(GE_Context *pGECtx, MS_BOOL bStepTagBefore, MS_U16* u16NextTagID)
{
    GE_CTX_LOCAL *pGECtxLocal = (GE_CTX_LOCAL*)pGECtx;

    GE_ENTRY(pGECtx);

    *u16NextTagID = GE_GetNextTAGID(&pGECtxLocal->halLocalCtx, bStepTagBefore);

    GE_RETURN(pGECtx,E_GE_OK);
}

//-------------------------------------------------------------------------------------------------
/// Set next TagID Auto to HW
/// Get the Next Tag ID auto and set it to HW.
/// @param  pGECtx                     \b IN: GE context
/// @param  tagID                      \b OUT: The Tag ID which has been sent to hw
/// @return @ref GE_Result
//-------------------------------------------------------------------------------------------------
GE_Result MDrv_GE_SetNextTAGID(GE_Context *pGECtx, MS_U16 *tagID)
{
    GE_CTX_LOCAL *pGECtxLocal = (GE_CTX_LOCAL*)pGECtx;
    MS_U8 i=0;

    GE_ENTRY(pGECtx);

    *tagID = GE_GetNextTAGID(&pGECtxLocal->halLocalCtx, TRUE);

    //GE hw will kick off cmd in pairs, so send tagID twice to force kick off
    for(i=0;i<(GE_WordUnit>>2);i++)
    {
        GE_WriteReg(&pGECtxLocal->halLocalCtx, REG_GE_TAG, *tagID);
    }

    GE_RETURN(pGECtx, E_GE_OK);
}

//-------------------------------------------------------------------------------------------------
/// Set GE Dbglevel (0 => no dbg)
/// @param  pGECtx                     \b IN: GE context
/// @param  level         \b IN: Debug level.
/// @return @ref GE_Result
//-------------------------------------------------------------------------------------------------
GE_Result MDrv_GE_SetDbgLevel(GE_Context *pGECtx, MS_U32 level)
{

    GE_ENTRY(pGECtx);
    dbg_level = level;
    if(dbg_level ==0)
    {
        GE_RETURN(pGECtx,E_GE_OK);
    }
    else
    {
        GE_RETURN(pGECtx,E_GE_OK);
}
}

//-------------------------------------------------------------------------------------------------
/// Get GE internal information
/// @param  pGECtx                     \b IN: GE context
/// @param  dbg         \b OUT: Pointer to GE dbg info.
/// @return @ref GE_Result
//-------------------------------------------------------------------------------------------------
GE_Result MDrv_GE_GetInfo(GE_Context *pGECtx, GE_DbgInfo* dbg)
{
    GE_CTX_LOCAL *pGECtxLocal = (GE_CTX_LOCAL*)pGECtx;

    MS_U32 i = 0;
    MS_U16 semid = 0;
    GE_ENTRY(pGECtx);

    for(i =0;i<0x80;i++ )
    {
        if(GE_Map_Share_Reg(&pGECtxLocal->halLocalCtx,i)== E_GE_OK)
            dbg->gedump[i]  = pGECtxLocal->halLocalCtx.pHALShared->u16ShareRegImage[i];
        else
            dbg->gedump[i]  = pGECtxLocal->halLocalCtx.u16RegGETable[i];
    }
#if GE_LOCK_SUPPORT
    semid = 0;
#else
    MDrv_SEM_Get_ResourceID(GESEMID, &semid );
#endif
    dbg->semstaus = semid;
    GE_RETURN(pGECtx,E_GE_OK);


}

#ifdef CONFIG_UTOPIA_PROC_DBG_SUPPORT
static char* _gpcCmdLine = NULL;
GE_Result MDrv_GE_MDBCMD_EchoCmd(GE_Context *pstGECtx, MS_U64* pu64ReqHdl, char* pcCmdLine)
{
    if(strncmp("performance", pcCmdLine, 11) == 0)
    {
        if(strlen(pcCmdLine) < 26)
        {
            MdbPrint(pu64ReqHdl,"parameter not enough\nExample : echo performance 0 4 b n 7 b s> /proc/utopia_mdb/gfx \n");
            return E_GE_FAIL;
        }
        _gpcCmdLine = strsep(&pcCmdLine," ");
        _gpcCmdLine = pcCmdLine;
        MDrv_GE_BitbltPerformance(pstGECtx);
    }

    return E_GE_OK;
}

GE_Result MDrv_GE_GetGFXMdbInfo(GE_Context *pGECtx, MS_U64 *u64ReqHdl)
{
    GE_CTX_LOCAL *pGECtxLocal = (GE_CTX_LOCAL*)pGECtx;
    MS_U64 u64MiuSize = 0;
    MS_U16 i = 0;
    MS_U16 u16TmpReg00 = 0;
    MS_U16 u16TmpReg01 = 0;
    MS_U16 u16TmpReg02 = 0;
    MS_U16 u16TmpReg03 = 0;
    MS_U16 u16TmpReg04 = 0;
    MS_U16 u16TmpReg05 = 0;
    MS_U16 u16TmpReg06 = 0;
    MS_U16 u16TmpReg07 = 0;
    MS_U16 u16REG_GE_EN = 0;
    MS_U16 u16REG_GE_CFG = 0;
    MS_U16 u16REG_GE_YUV_MODE = 0;
    MS_U16 u16REG_GE_VCMDQ_BASE_H = 0;
    MS_U16 u16REG_GE_VCMDQ_BASE_L = 0;
    MS_U16 u16REG_GE_VCMDQ_SIZE = 0;
    MS_U16 u16REG_GE_SRC_BASE_H = 0;
    MS_U16 u16REG_GE_SRC_BASE_L = 0;
    MS_U16 u16REG_GE_DST_BASE_H = 0;
    MS_U16 u16REG_GE_DST_BASE_L = 0;


    GE_ENTRY(pGECtx);

    u16REG_GE_EN = GE_ReadReg(&pGECtxLocal->halLocalCtx, REG_GE_EN + GE_TABLE_REGNUM);
    u16REG_GE_CFG = GE_ReadReg(&pGECtxLocal->halLocalCtx, REG_GE_CFG + GE_TABLE_REGNUM);
    u16REG_GE_YUV_MODE =GE_ReadReg(&pGECtxLocal->halLocalCtx, REG_GE_YUV_MODE + GE_TABLE_REGNUM);
    u16REG_GE_VCMDQ_BASE_H = GE_ReadReg(&pGECtxLocal->halLocalCtx, REG_GE_VCMDQ_BASE_H + GE_TABLE_REGNUM);
    u16REG_GE_VCMDQ_BASE_L = GE_ReadReg(&pGECtxLocal->halLocalCtx, REG_GE_VCMDQ_BASE_L + GE_TABLE_REGNUM);
    u16REG_GE_VCMDQ_SIZE = GE_ReadReg(&pGECtxLocal->halLocalCtx, REG_GE_VCMDQ_SIZE + GE_TABLE_REGNUM);
    u16REG_GE_SRC_BASE_H = GE_ReadReg(&pGECtxLocal->halLocalCtx, REG_GE_SRC_BASE_H + GE_TABLE_REGNUM);
    u16REG_GE_SRC_BASE_L = GE_ReadReg(&pGECtxLocal->halLocalCtx, REG_GE_SRC_BASE_L + GE_TABLE_REGNUM);
    u16REG_GE_DST_BASE_H = GE_ReadReg(&pGECtxLocal->halLocalCtx, REG_GE_DST_BASE_H + GE_TABLE_REGNUM);
    u16REG_GE_DST_BASE_L = GE_ReadReg(&pGECtxLocal->halLocalCtx, REG_GE_DST_BASE_L + GE_TABLE_REGNUM);

    u64MiuSize = (1 << (9 + (u16REG_GE_VCMDQ_SIZE & GE_VCMDQ_SIZE_MASK))) * GE_WordUnit;
    u64MiuSize = (u64MiuSize >> 10);

    MdbPrint(u64ReqHdl,"---------MStar GE Info---------\n\n");
    if(u16REG_GE_EN & GE_EN_GE)
    {
        MdbPrint(u64ReqHdl,"GE Init                    :    True\n");
    }
    else
    {
        MdbPrint(u64ReqHdl,"GE Init                    :    False\n");
    }
    if(u16REG_GE_EN & GE_EN_BURST)
    {
        MdbPrint(u64ReqHdl,"Burst Mode                 :    Enable\n");
    }
    else
    {
        MdbPrint(u64ReqHdl,"Burst Mode                 :    Disable\n");
    }
    if(u16REG_GE_CFG & GE_CFG_RW_SPLIT)
    {
        MdbPrint(u64ReqHdl,"Split Mode                 :    Enable\n");
    }
    else
    {
        MdbPrint(u64ReqHdl,"Split Mode                 :    Disable\n");
    }
    if(u16REG_GE_CFG & GE_CFG_CMDQ)
    {
        MdbPrint(u64ReqHdl,"Command Queue              :    Enable\n");
    }
    else
    {
        MdbPrint(u64ReqHdl,"Command Queue              :    Disable\n");
    }
    if(u16REG_GE_CFG & GE_CFG_VCMDQ)
    {
        MdbPrint(u64ReqHdl,"Virtual Command Queue      :    Enable\n");
    }
    else
    {
        MdbPrint(u64ReqHdl,"Virtual Command Queue      :    Disable\n");
    }
    MdbPrint(u64ReqHdl,"VCMDQ MIU                  :    %d\n",((u16REG_GE_YUV_MODE & BIT(15)) >> 14) | ((u16REG_GE_VCMDQ_BASE_H & BIT(15)) >> 15));
    MdbPrint(u64ReqHdl,"VCMDQ Address              :    0x%lx\n",(u16REG_GE_VCMDQ_BASE_H<<16) | u16REG_GE_VCMDQ_BASE_L);
    MdbPrint(u64ReqHdl,"VCMDQ Size                 :    %d K\n",u64MiuSize);
    MdbPrint(u64ReqHdl,"Source MIU                 :    %d\n",((u16REG_GE_YUV_MODE & BIT(14)) >> 13) | ((u16REG_GE_SRC_BASE_H & BIT(15)) >> 15));
    MdbPrint(u64ReqHdl,"Source Address             :    0x%lx\n",(u16REG_GE_SRC_BASE_H<<16) | u16REG_GE_SRC_BASE_L);
    MdbPrint(u64ReqHdl,"Destination MIU            :    %d\n",((u16REG_GE_YUV_MODE & BIT(13)) >> 12) | ((u16REG_GE_DST_BASE_H & BIT(15)) >> 15));
    MdbPrint(u64ReqHdl,"Destination Address        :    0x%lx\n",(u16REG_GE_DST_BASE_H<<16) | u16REG_GE_DST_BASE_L);


    MdbPrint(u64ReqHdl,"\n>>>>>>>>>>>>>>>>>>>>> Dump GE register <<<<<<<<<<<<<<<<<<<<<<<\n");
    MdbPrint(u64ReqHdl,"       00/8 01/9 02/A 03/B 04/C 05/D 06/E 07/F \n");
    for (i = 0; i < 0x80; i+=0x8)
    {
        u16TmpReg00 = GE_ReadReg(&pGECtxLocal->halLocalCtx, i + GE_TABLE_REGNUM);
        u16TmpReg01 = GE_ReadReg(&pGECtxLocal->halLocalCtx, i + 1 + GE_TABLE_REGNUM);
        u16TmpReg02 = GE_ReadReg(&pGECtxLocal->halLocalCtx, i + 2 + GE_TABLE_REGNUM);
        u16TmpReg03 = GE_ReadReg(&pGECtxLocal->halLocalCtx, i + 3 + GE_TABLE_REGNUM);
        u16TmpReg04 = GE_ReadReg(&pGECtxLocal->halLocalCtx, i + 4 + GE_TABLE_REGNUM);
        u16TmpReg05 = GE_ReadReg(&pGECtxLocal->halLocalCtx, i + 5 + GE_TABLE_REGNUM);
        u16TmpReg06 = GE_ReadReg(&pGECtxLocal->halLocalCtx, i + 6 + GE_TABLE_REGNUM);
        u16TmpReg07 = GE_ReadReg(&pGECtxLocal->halLocalCtx, i + 7 + GE_TABLE_REGNUM);

        MdbPrint(u64ReqHdl,"h%x%x    %x%x%x%x %x%x%x%x %x%x%x%x %x%x%x%x %x%x%x%x %x%x%x%x %x%x%x%x %x%x%x%x \n", (MS_U8)(i & 0xf0) >> 4, (MS_U8)(i & 0xf), \
                                                       (u16TmpReg00 & 0xf000) >> 12, (u16TmpReg00 & 0xf00) >> 8, (u16TmpReg00 & 0xf0) >> 4, (u16TmpReg00 & 0xf), \
                                                       (u16TmpReg01 & 0xf000) >> 12, (u16TmpReg01 & 0xf00) >> 8, (u16TmpReg01 & 0xf0) >> 4, (u16TmpReg01 & 0xf), \
                                                       (u16TmpReg02 & 0xf000) >> 12, (u16TmpReg02 & 0xf00) >> 8, (u16TmpReg02 & 0xf0) >> 4, (u16TmpReg02 & 0xf), \
                                                       (u16TmpReg03 & 0xf000) >> 12, (u16TmpReg03 & 0xf00) >> 8, (u16TmpReg03 & 0xf0) >> 4, (u16TmpReg03 & 0xf), \
                                                       (u16TmpReg04 & 0xf000) >> 12, (u16TmpReg04 & 0xf00) >> 8, (u16TmpReg04 & 0xf0) >> 4, (u16TmpReg04 & 0xf), \
                                                       (u16TmpReg05 & 0xf000) >> 12, (u16TmpReg05 & 0xf00) >> 8, (u16TmpReg05 & 0xf0) >> 4, (u16TmpReg05 & 0xf), \
                                                       (u16TmpReg06 & 0xf000) >> 12, (u16TmpReg06 & 0xf00) >> 8, (u16TmpReg06 & 0xf0) >> 4, (u16TmpReg06 & 0xf), \
                                                       (u16TmpReg07 & 0xf000) >> 12, (u16TmpReg07 & 0xf00) >> 8, (u16TmpReg07 & 0xf0) >> 4, (u16TmpReg07 & 0xf));

    }
    MdbPrint(u64ReqHdl,"=================================================================\n");

    GE_RETURN(pGECtx,E_GE_OK);
}
#endif

//-------------------------------------------------------------------------------------------------
/// Get GE internal information
/// @param  pGECtx                     \b IN: GE context
/// @param  bstatus         \b OUT: Pointer to GE internal status
/// @return @ref GE_Result
//-------------------------------------------------------------------------------------------------
GE_Result MDrv_GE_GetStatus(GE_Context *pGECtx, MS_BOOL* bstatus)
{
    GE_ENTRY(pGECtx);

    *bstatus = TRUE;

    GE_RETURN(pGECtx,E_GE_OK);
}


//-------------------------------------------------------------------------------------------------
///Enable GE Virtual Command Queue
/// @param  pGECtx                     \b IN: GE context
/// @param  blEnable         \b IN: true: Enable, false: Disable
/// @return @ref GE_Result
//-------------------------------------------------------------------------------------------------
GE_Result MDrv_GE_EnableVCmdQueue(GE_Context *pGECtx, MS_U16 blEnable)
{
    MS_U16 u16tmp;
    GE_CTX_LOCAL *pGECtxLocal = (GE_CTX_LOCAL*)pGECtx;

    GE_ENTRY(pGECtx);

    GE_WaitIdle(&pGECtxLocal->halLocalCtx);

    u16tmp = GE_ReadReg(&pGECtxLocal->halLocalCtx, REG_GE_CFG);

    if (blEnable)
    {
        u16tmp |= GE_CFG_VCMDQ;
        GE_WriteReg(&pGECtxLocal->halLocalCtx, REG_GE_CFG, u16tmp);
    }
    else
    {
        u16tmp &= ~GE_CFG_VCMDQ;
        GE_WriteReg(&pGECtxLocal->halLocalCtx, REG_GE_CFG, u16tmp);
    }


    GE_RETURN(pGECtx,E_GE_OK);
}


//-------------------------------------------------------------------------------------------------
///Set GE Virtual Command Queue spec
/// @param  pGECtx                     \b IN: GE context
/// @param  u32Addr         \b IN: Start address of VCMQ buffer.
/// @param  enBufSize         \b IN:  Size of VCMQ buffer.
/// @return @ref GE_Result
//-------------------------------------------------------------------------------------------------
GE_Result MDrv_GE_SetVCmdBuffer(GE_Context *pGECtx, MS_PHY PhyAddr, GE_VcmqBufSize enBufSize)
{
    GE_Result           ret;
    GE_CTX_LOCAL *pGECtxLocal = (GE_CTX_LOCAL*)pGECtx;

    GE_ENTRY(pGECtx);

    GE_WaitIdle(&pGECtxLocal->halLocalCtx);

    ret = GE_SetVCmdBuffer(&pGECtxLocal->halLocalCtx, PhyAddr, enBufSize);

    GE_RETURN(pGECtx,ret);
}


//-------------------------------------------------------------------------------------------------
///Set GE Virtual Command Write threshold
/// @param  pGECtx                     \b IN: GE context
/// @param  u8W_Threshold         \b IN: Threshold for VCMQ write.
/// @return @ref GE_Result
//-------------------------------------------------------------------------------------------------
GE_Result MDrv_GE_SetVCmd_W_Thread(GE_Context *pGECtx, MS_U8 u8W_Threshold)
{
    GE_CTX_LOCAL *pGECtxLocal = (GE_CTX_LOCAL*)pGECtx;

    GE_ENTRY(pGECtx);

    GE_WaitIdle(&pGECtxLocal->halLocalCtx);

    HAL_GE_SetVCmd_W_Thread(&pGECtxLocal->halLocalCtx, u8W_Threshold);

    GE_RETURN(pGECtx,E_GE_OK);
}


//-------------------------------------------------------------------------------------------------
///Set GE Virtual Command Read threshold
/// @param  pGECtx                     \b IN: GE context
/// @param  u8R_Threshold         \b IN: Threshold for VCMQ read.
/// @return @ref GE_Result
//-------------------------------------------------------------------------------------------------
GE_Result MDrv_GE_SetVCmd_R_Thread(GE_Context *pGECtx, MS_U8 u8R_Threshold)
{
    GE_CTX_LOCAL *pGECtxLocal = (GE_CTX_LOCAL*)pGECtx;

    GE_ENTRY(pGECtx);

    GE_WaitIdle(&pGECtxLocal->halLocalCtx);

    HAL_GE_SetVCmd_R_Thread(&pGECtxLocal->halLocalCtx, u8R_Threshold);

    GE_RETURN(pGECtx,E_GE_OK);
}


////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function \b Name: MDrv_GE_GetLibVer
/// @brief \b Function \b Description: Show the GE driver version
/// @param  pGECtx                     \b IN: GE context
/// @param  ppVersion         \b OUT: Pointer to version string.
/// @return @ref GE_Result
////////////////////////////////////////////////////////////////////////////////
GE_Result MDrv_GE_GetLibVer(GE_Context *pGECtx, const MSIF_Version **ppVersion)
{
    // No mutex check, it can be called before Init
    if (!ppVersion)
    {
        return E_GE_FAIL;
    }

    *ppVersion = &_drv_ge_version;

    return E_GE_OK;
}

////////////////////////////////////////////////////////////////////////////////
/// Configure GE SCK edge refine rtpe.
/// @param  pGECtx                     \b IN: GE context
/// @param  TYPE         \b IN: Type for edge refine.
/// @param  CLR         \b IN: 32-bit custom color if type is E_GE_REPLACE_KEY_2_CUS
/// @return @ref GE_Result
///////////////////////////////////////////////////////////////////////////////
GE_Result MDrv_GE_SetStrBltSckType(GE_Context *pGECtx,GE_StretchCKType TYPE, MS_U32 *CLR)
{
    MS_U16 u16tmp;
    GE_CTX_LOCAL *pGECtxLocal = (GE_CTX_LOCAL*)pGECtx;

    GE_ENTRY(pGECtx);

    GE_WaitIdle(&pGECtxLocal->halLocalCtx);

    u16tmp = GE_ReadReg(&pGECtxLocal->halLocalCtx,REG_GE_BLT_SCK_MODE);

    u16tmp &= ~(GE_BLT_SCK_MODE_MASK);

    switch(TYPE)
    {
        case E_GE_DONOTHING:
            u16tmp |=GE_BLT_SCK_BILINEAR;
        break;

        case E_GE_NEAREST:
            u16tmp |=GE_BLT_SCK_NEAREST;
        break;

        case E_GE_REPLACE_KEY_2_CUS:
            u16tmp |=GE_BLT_SCK_CONST;
        break;
        default:
            GE_D_DBUG("[%s] ERROR Stretch Type\n",__FUNCTION__);
            break;
    }

    GE_WriteReg(&pGECtxLocal->halLocalCtx,REG_GE_BLT_SCK_CONST_L, (*CLR)&0xffff);
    GE_WriteReg(&pGECtxLocal->halLocalCtx,REG_GE_BLT_SCK_CONST_H, (*CLR)>>16);
    GE_WriteReg(&pGECtxLocal->halLocalCtx,REG_GE_BLT_SCK_MODE, u16tmp);

    GE_RETURN(pGECtx,E_GE_OK);
}



////////////////////////////////////////////////////////////////////////////////
/// Get if GE is running as HK.
/// The semaphore allocated by GE differs in HK and non-HK mode.
/// @param  pGECtx                     \b IN: GE context
/// @param  bIsHK         \b OUT: true => running as HK, false => running as co-processor.
/// @return @ref GE_Result
///////////////////////////////////////////////////////////////////////////////
GE_Result MDrv_GE_Get_GetHK(GE_Context *pGECtx, MS_BOOL *bIsHK)
{
    GE_CTX_LOCAL *pGECtxLocal = (GE_CTX_LOCAL*)pGECtx;

    GE_ENTRY(pGECtx);

    if( SEM_HKAeonID == pGECtxLocal->u32GESEMID)
        *bIsHK = TRUE;
    else
        *bIsHK = FALSE;

    GE_RETURN(pGECtx,E_GE_OK);

}

////////////////////////////////////////////////////////////////////////////////
/// Configure GE for running as HK or not.
/// The semaphore allocated by GE differs in HK and non-HK mode.
/// @param  pGECtx                     \b IN: GE context
/// @param  bIsHK         \b OUT: true => running as HK, false => running as co-processor.
/// @return @ref GE_Result
///////////////////////////////////////////////////////////////////////////////
GE_Result MDrv_GE_Set_SetHK(GE_Context *pGECtx, MS_BOOL bIsHK)
{
    GE_CTX_LOCAL *pGECtxLocal = (GE_CTX_LOCAL*)pGECtx;

    if(bIsHK)
        pGECtxLocal->u32GESEMID = SEM_HKAeonID;
    else
        pGECtxLocal->u32GESEMID = SEM_MWMipsID;

    GE_ENTRY(pGECtx);
    GE_RETURN(pGECtx,E_GE_OK);

}



////////////////////////////////////////////////////////////////////////////////
/// Draw Oval. Oval is not directly supported by HW. Software implemented by DrawLine.
/// @param  pGECtx                     \b IN: GE context
/// @param  pOval         \b IN: oval info
/// @return @ref GE_Result
///////////////////////////////////////////////////////////////////////////////
GE_Result MDrv_GE_DrawOval(GE_Context *pGECtx, GE_OVAL_FILL_INFO* pOval)
{
    GE_CTX_LOCAL *pGECtxLocal = (GE_CTX_LOCAL*)pGECtx;

    GE_ENTRY(pGECtx);

    MS_S32 x, y, c_x, c_y;
    MS_S32 Xchange, Ychange;
    MS_S32 EllipseError;
    MS_S32 TwoASquare, TwoBSquare;
    MS_S32 StoppingX, StoppingY;
    MS_U32 Xradius, Yradius;

    MS_U16 u16Color0, u16Color1;
    MS_BOOL bDstXInv = FALSE;

    GE_Restore_PaletteContext(pGECtxLocal);

    Xradius = (pOval->dstBlock.width >> 1) - pOval->u32LineWidth;
    Yradius = (pOval->dstBlock.height >> 1) - pOval->u32LineWidth;

    /* center of ellipse */
    c_x = pOval->dstBlock.x + Xradius + pOval->u32LineWidth;
    c_y = pOval->dstBlock.y + Yradius + pOval->u32LineWidth;

    TwoASquare = (Xradius*Xradius) << 1;
    TwoBSquare = (Yradius*Yradius) << 1;

    /*1st set of points*/
    x = Xradius-1;  /*radius zero == draw nothing*/
    y = 0;

    StoppingX = TwoBSquare*Xradius;
    StoppingY = 0;

    Xchange = (TwoBSquare >> 1) - StoppingX;
    Ychange = TwoASquare >> 1;

    EllipseError = 0;

#if 1//(SUPPORT_LINE_LAST_PATCH == 1 )
    GE_WriteReg(&pGECtxLocal->halLocalCtx,REG_GE_LINE_STYLE, GE_LINEPAT_RST | 0x3f);
    GE_WriteReg(&pGECtxLocal->halLocalCtx,REG_GE_LINE_STYLE,  0x3f);
#else
    GE_WriteReg(&pGECtxLocal->halLocalCtx,REG_GE_LINE_STYLE, GE_LINE_LAST | GE_LINEPAT_RST | 0x3f);
    GE_WriteReg(&pGECtxLocal->halLocalCtx,REG_GE_LINE_STYLE, GE_LINE_LAST | 0x3f);
#endif

    GE_WriteReg(&pGECtxLocal->halLocalCtx,_REG_GE_LINE_DELTA, 0);

    u16Color0 = (((MS_U16)(pOval->color.g)) << 8)+ (MS_U16)pOval->color.b;
    u16Color1 = (((MS_U16)(pOval->color.a)) << 8)+ (MS_U16)pOval->color.r;

    GE_WriteReg(&pGECtxLocal->halLocalCtx,REG_GE_PRIM_C_L, u16Color0);
    GE_WriteReg(&pGECtxLocal->halLocalCtx,REG_GE_PRIM_C_H, u16Color1);

    /*Plot 2 ellipse scan lines for iteration*/
    while (StoppingX > StoppingY)
    {
        GE_WriteReg(&pGECtxLocal->halLocalCtx,REG_GE_PRIM_V0_X, c_x - x);
        GE_WriteReg(&pGECtxLocal->halLocalCtx,REG_GE_PRIM_V1_X, c_x + x);
        GE_WriteReg(&pGECtxLocal->halLocalCtx,REG_GE_PRIM_V0_Y, c_y + y);
        GE_WriteReg(&pGECtxLocal->halLocalCtx,REG_GE_PRIM_V1_Y, c_y + y + 1);
        GE_WriteReg(&pGECtxLocal->halLocalCtx,_REG_GE_LINE_LENGTH, (x << 1) + 1);
        HAL_GE_AdjustDstWin(&pGECtxLocal->halLocalCtx,bDstXInv);
#ifdef GE_VQ_MIU_HANG_PATCH
        if ((GE_IsVcmdqEnabled(pGECtx) == TRUE) &&
            (u8DstMIU != pGECtxLocal->halLocalCtx.pHALShared->u8VCmdQMiu)
            )
        {
            //printf("\33[0;36m   %s:%d  u8VCmdQMiu = %d  u8SrcMIU = %d u8DstMIU = %d\33[m \n",__FUNCTION__,__LINE__,pGECtxLocal->halLocalCtx.pHALShared->u8VCmdQMiu,u8SrcMIU,u8DstMIU);
            MDrv_GE_WaitIdle(pGECtx);
        }
#endif
        _GE_SEMAPHORE_ENTRY(pGECtx,E_GE_POOL_ID_INTERNAL_REGISTER);
        GE_WriteReg(&pGECtxLocal->halLocalCtx,REG_GE_CMD, GE_PRIM_LINE);
        _GE_SEMAPHORE_RETURN(pGECtx,E_GE_POOL_ID_INTERNAL_REGISTER);

#ifdef GE_VQ_MIU_HANG_PATCH
        if ((GE_IsVcmdqEnabled(pGECtx) == TRUE) &&
            (u8DstMIU != pGECtxLocal->halLocalCtx.pHALShared->u8VCmdQMiu)
            )
        {
            //printf("\33[0;36m   %s:%d  u8VCmdQMiu = %d  u8SrcMIU = %d u8DstMIU = %d\33[m \n",__FUNCTION__,__LINE__,pGECtxLocal->halLocalCtx.pHALShared->u8VCmdQMiu,u8SrcMIU,u8DstMIU);
            MDrv_GE_WaitIdle(pGECtx);
        }
#endif

        //GE_WriteReg(&pGECtxLocal->halLocalCtx,REG_GE_PRIM_V0_X, c_x - x);
        //GE_WriteReg(&pGECtxLocal->halLocalCtx,REG_GE_PRIM_V1_X, c_x + x);
        GE_WriteReg(&pGECtxLocal->halLocalCtx,REG_GE_PRIM_V0_Y, c_y - y);
#if 1 //(SUPPORT_LINE_LAST_PATCH == 1 )
        GE_WriteReg(&pGECtxLocal->halLocalCtx,REG_GE_PRIM_V1_Y, c_y - y + 1);
        //GE_WriteReg(&pGECtxLocal->halLocalCtx,REG_GE_LINE_LENGTH, 2*x + 1);
#else
        GE_WriteReg(&pGECtxLocal->halLocalCtx,REG_GE_PRIM_V1_Y, c_y - y );
        GE_WriteReg(&pGECtxLocal->halLocalCtx,REG_GE_LINE_LENGTH, 2*x );
#endif
        HAL_GE_AdjustDstWin(&pGECtxLocal->halLocalCtx,bDstXInv);
#ifdef GE_VQ_MIU_HANG_PATCH
        if ((GE_IsVcmdqEnabled(pGECtx) == TRUE) &&
            (u8DstMIU != pGECtxLocal->halLocalCtx.pHALShared->u8VCmdQMiu)
            )
        {
            //printf("\33[0;36m   %s:%d  u8VCmdQMiu = %d  u8SrcMIU = %d u8DstMIU = %d\33[m \n",__FUNCTION__,__LINE__,pGECtxLocal->halLocalCtx.pHALShared->u8VCmdQMiu,u8SrcMIU,u8DstMIU);
            MDrv_GE_WaitIdle(pGECtx);
        }
#endif
        _GE_SEMAPHORE_ENTRY(pGECtx,E_GE_POOL_ID_INTERNAL_REGISTER);
        GE_WriteReg(&pGECtxLocal->halLocalCtx,REG_GE_CMD, GE_PRIM_LINE);
        _GE_SEMAPHORE_RETURN(pGECtx,E_GE_POOL_ID_INTERNAL_REGISTER);

#ifdef GE_VQ_MIU_HANG_PATCH
        if ((GE_IsVcmdqEnabled(pGECtx) == TRUE) &&
            (u8DstMIU != pGECtxLocal->halLocalCtx.pHALShared->u8VCmdQMiu)
            )
        {
            //printf("\33[0;36m   %s:%d  u8VCmdQMiu = %d  u8SrcMIU = %d u8DstMIU = %d\33[m \n",__FUNCTION__,__LINE__,pGECtxLocal->halLocalCtx.pHALShared->u8VCmdQMiu,u8SrcMIU,u8DstMIU);
            MDrv_GE_WaitIdle(pGECtx);
        }
#endif

        ++y;
        StoppingY    += TwoASquare;
        EllipseError += Ychange;
        Ychange      += TwoASquare;
        if (( (EllipseError << 1) + Xchange) > 0)
        {
            --x;
            StoppingX    -= TwoBSquare;
            EllipseError += Xchange;
            Xchange      += TwoBSquare;
        }
    }

    /*2nd set of points*/
    x = 0;
    y = Yradius-1;  /*radius zero == draw nothing*/
    StoppingX = 0;
    StoppingY = TwoASquare*Yradius;
    Xchange = TwoBSquare >> 1;
    Ychange = (TwoASquare >> 1) - StoppingY;
    EllipseError = 0;

    /*Plot 2 ellipse scan lines for iteration*/
    while (StoppingX < StoppingY)
    {
        GE_WriteReg(&pGECtxLocal->halLocalCtx,REG_GE_PRIM_V0_X, c_x - x);
        GE_WriteReg(&pGECtxLocal->halLocalCtx,REG_GE_PRIM_V1_X, c_x + x);
        GE_WriteReg(&pGECtxLocal->halLocalCtx,REG_GE_PRIM_V0_Y, c_y + y);
        GE_WriteReg(&pGECtxLocal->halLocalCtx,REG_GE_PRIM_V1_Y, c_y + y + 1);
        GE_WriteReg(&pGECtxLocal->halLocalCtx,_REG_GE_LINE_LENGTH, (x << 1) + 1);
//        GE_WriteReg(&pGECtxLocal->halLocalCtx,REG_GE_CMD, GE_PRIM_LINE|_u32Reg60hFlag);
        HAL_GE_AdjustDstWin(&pGECtxLocal->halLocalCtx,bDstXInv);
#ifdef GE_VQ_MIU_HANG_PATCH
        if ((GE_IsVcmdqEnabled(pGECtx) == TRUE) &&
            (u8DstMIU != pGECtxLocal->halLocalCtx.pHALShared->u8VCmdQMiu)
            )
        {
            //printf("\33[0;36m   %s:%d  u8VCmdQMiu = %d  u8SrcMIU = %d u8DstMIU = %d\33[m \n",__FUNCTION__,__LINE__,pGECtxLocal->halLocalCtx.pHALShared->u8VCmdQMiu,u8SrcMIU,u8DstMIU);
            MDrv_GE_WaitIdle(pGECtx);
        }
#endif
        _GE_SEMAPHORE_ENTRY(pGECtx,E_GE_POOL_ID_INTERNAL_REGISTER);
        GE_WriteReg(&pGECtxLocal->halLocalCtx,REG_GE_CMD, GE_PRIM_LINE);
        _GE_SEMAPHORE_RETURN(pGECtx,E_GE_POOL_ID_INTERNAL_REGISTER);

#ifdef GE_VQ_MIU_HANG_PATCH
        if ((GE_IsVcmdqEnabled(pGECtx) == TRUE) &&
            (u8DstMIU != pGECtxLocal->halLocalCtx.pHALShared->u8VCmdQMiu)
            )
        {
            //printf("\33[0;36m   %s:%d  u8VCmdQMiu = %d  u8SrcMIU = %d u8DstMIU = %d\33[m \n",__FUNCTION__,__LINE__,pGECtxLocal->halLocalCtx.pHALShared->u8VCmdQMiu,u8SrcMIU,u8DstMIU);
            MDrv_GE_WaitIdle(pGECtx);
        }
#endif

        //GE_WriteReg(&pGECtxLocal->halLocalCtx,REG_GE_PRIM_V0_X, c_x - x);
        //GE_WriteReg(&pGECtxLocal->halLocalCtx,REG_GE_PRIM_V1_X, c_x + x);
        GE_WriteReg(&pGECtxLocal->halLocalCtx,REG_GE_PRIM_V0_Y, c_y - y);
        GE_WriteReg(&pGECtxLocal->halLocalCtx,REG_GE_PRIM_V1_Y, c_y - y + 1);
        //GE_WriteReg(&pGECtxLocal->halLocalCtx,REG_GE_LINE_LENGTH, 2*x + 1);
//        GE_WriteReg(&pGECtxLocal->halLocalCtx,REG_GE_CMD, GE_PRIM_LINE|_u32Reg60hFlag);
        HAL_GE_AdjustDstWin(&pGECtxLocal->halLocalCtx,bDstXInv);
#ifdef GE_VQ_MIU_HANG_PATCH
        if ((GE_IsVcmdqEnabled(pGECtx) == TRUE) &&
            (u8DstMIU != pGECtxLocal->halLocalCtx.pHALShared->u8VCmdQMiu)
            )
        {
            //printf("\33[0;36m   %s:%d  u8VCmdQMiu = %d  u8SrcMIU = %d u8DstMIU = %d\33[m \n",__FUNCTION__,__LINE__,pGECtxLocal->halLocalCtx.pHALShared->u8VCmdQMiu,u8SrcMIU,u8DstMIU);
            MDrv_GE_WaitIdle(pGECtx);
        }
#endif
        _GE_SEMAPHORE_ENTRY(pGECtx,E_GE_POOL_ID_INTERNAL_REGISTER);
        GE_WriteReg(&pGECtxLocal->halLocalCtx,REG_GE_CMD, GE_PRIM_LINE);
        _GE_SEMAPHORE_RETURN(pGECtx,E_GE_POOL_ID_INTERNAL_REGISTER);

#ifdef GE_VQ_MIU_HANG_PATCH
        if ((GE_IsVcmdqEnabled(pGECtx) == TRUE) &&
            (u8DstMIU != pGECtxLocal->halLocalCtx.pHALShared->u8VCmdQMiu)
            )
        {
            //printf("\33[0;36m   %s:%d  u8VCmdQMiu = %d  u8SrcMIU = %d u8DstMIU = %d\33[m \n",__FUNCTION__,__LINE__,pGECtxLocal->halLocalCtx.pHALShared->u8VCmdQMiu,u8SrcMIU,u8DstMIU);
            MDrv_GE_WaitIdle(pGECtx);
        }
#endif

        ++x;
        StoppingX    += TwoBSquare;
        EllipseError += Xchange;
        Xchange      += TwoBSquare;
        if (((EllipseError << 1) + Ychange) > 0)
        {
            --y;
            StoppingY    -= TwoASquare;
            EllipseError += Ychange;
            Ychange      += TwoASquare;
        }
    }

    GE_RETURN(pGECtx,E_GE_OK);
}

////////////////////////////////////////////////////////////////////////////////
/// Power State Setting
/// @param  pGECtx                \b IN: GE context
/// @param  u16PowerState         \b IN: Power State
/// @return @ref GE_Result
///////////////////////////////////////////////////////////////////////////////
#if defined(MSOS_TYPE_LINUX_KERNEL)
#define STR_RIU_MAP                     g_halGECtxLocal.va_mmio_base
#define STR_RIU                         ((unsigned short volatile *) STR_RIU_MAP)
#define STR_GE_REG(addr)                STR_RIU[ (((addr)*2) + 0x2800UL)]
#define STR_GETLB_REG(addr)             STR_RIU[ (((addr)*2) + 0x62800UL)]
#define STR_CLK_REG(addr)               STR_RIU[ (((addr)*2) + 0x0B00UL)]
#define STR_GE_CMDQ_FREECNT()           ((STR_GE_REG(REG_GE_STAT)&GE_STAT_CMDQ_MASK)>>GE_STAT_CMDQ_SHFT)


static void MDrv_GESTR_DumpReg(void)
{
    MS_U32 i;

    GE_D_DBUG("Dump GE register:\n");
    for (i = 0; i < 0x80; i++)
    {
        if(i % 0x08 == 0) {
            GE_D_DBUG("    \n");
            GE_D_DBUG("h%02x    ", (MS_U8)i );
        }
        GE_D_DBUG("%04x ", STR_GE_REG(i) );
    }
    GE_D_DBUG("    \n");
}

MS_U32 MDrv_GESTR_WaitIdle(void)
{
    MS_U32 waitcount = 0;

    // Wait level-2 command queue flush
    while (((STR_GE_REG(REG_GE_STAT)&GE_STAT_CMDQ2_MASK)>>GE_STAT_CMDQ2_SHFT) != GE_STAT_CMDQ2_MAX)
    {
        if (waitcount >= 1000)
        {
            GE_D_DBUG("[GE] Wait Idle: %tx\n",(ptrdiff_t)STR_GE_CMDQ_FREECNT());
            waitcount = 0;
            MDrv_GESTR_DumpReg();
            break;
        }
        waitcount++;
        MsOS_DelayTask(1);
    }

    waitcount = 0;
    // Wait GE idle
    while (STR_GE_REG(REG_GE_STAT) & GE_STAT_BUSY)
    {
        if (waitcount >= 1000)
        {
            GE_D_DBUG("[GE] Wait Busy: %tx\n",(ptrdiff_t)STR_GE_CMDQ_FREECNT());
            waitcount = 0;
            MDrv_GESTR_DumpReg();
        }
        waitcount++;
        MsOS_DelayTask(1);
    }
    return 0;
}

MS_U32 MDrv_GESTR_SetPalette(MS_U32 *GEPalette)
{
    int i;

    for(i=0; i<GE_PALETTE_NUM; i++)
    {
        STR_GE_REG(REG_GE_CLUT_H) = (ByteSwap16(GEPalette[i]) & 0xFFFF);
        STR_GE_REG(REG_GE_CLUT_L) = (ByteSwap16(GEPalette[i]>>16));
        STR_GE_REG(REG_GE_CLUT_CTRL) = ((i & GE_CLUT_CTRL_IDX_MASK) | GE_CLUT_CTRL_WR);
    }
    return 0;
}

#endif

GE_Result MDrv_GE_SetPowerState(GE_Context *pGECtx, EN_POWER_MODE u16PowerState, void* pModule)
{
    GE_Config   cfg;
    GE_CTX_LOCAL *pGECtxLocal = (GE_CTX_LOCAL*)pGECtx;
#ifdef GE_VQADDR_LOCK_PATCH
    MS_U16 i=0;
#endif

    memset(&cfg, 0 , sizeof(GE_Config));
    switch(u16PowerState)
    {
        case E_POWER_SUSPEND:
#ifdef GE_VQADDR_LOCK_PATCH
             pGECtxLocal->u16RegCfg = GE_ReadReg(&pGECtxLocal->halLocalCtx, REG_GE_CFG);
             pGECtxLocal->u16VcmdqAddr_L = GE_ReadReg(&pGECtxLocal->halLocalCtx, REG_GE_VCMDQ_BASE_L);
             pGECtxLocal->u16VcmdqAddr_H = GE_ReadReg(&pGECtxLocal->halLocalCtx, REG_GE_VCMDQ_BASE_H);
             pGECtxLocal->u16VcmdqAddr_MSB = GE_ReadReg(&pGECtxLocal->halLocalCtx, REG_GE_BASE_MSB);
             pGECtxLocal->u16VcmdqMiuMsb = GE_ReadReg(&pGECtxLocal->halLocalCtx, REG_GE_YUV_MODE);
             pGECtxLocal->u16VcmdqSize = GE_ReadReg(&pGECtxLocal->halLocalCtx, REG_GE_VCMDQ_SIZE);
#endif
             break;
        case E_POWER_RESUME:
#ifdef GE_VQADDR_LOCK_PATCH
            GE_WriteReg(&pGECtxLocal->halLocalCtx, REG_GE_CFG, pGECtxLocal->u16RegCfg);
            for (i=0; i<GE_VQADDR_ENHANCE_LOCK_TIMES; i++)
            {
                GE_WriteReg(&pGECtxLocal->halLocalCtx, REG_GE_VCMDQ_BASE_L, pGECtxLocal->u16VcmdqAddr_L);
                GE_WriteReg(&pGECtxLocal->halLocalCtx, REG_GE_VCMDQ_BASE_H, pGECtxLocal->u16VcmdqAddr_H);
                GE_WriteReg(&pGECtxLocal->halLocalCtx, REG_GE_BASE_MSB, pGECtxLocal->u16VcmdqAddr_MSB);
            }
            GE_WriteReg(&pGECtxLocal->halLocalCtx, REG_GE_YUV_MODE, pGECtxLocal->u16VcmdqMiuMsb);
            GE_WriteReg(&pGECtxLocal->halLocalCtx, REG_GE_VCMDQ_SIZE, pGECtxLocal->u16VcmdqSize);
#endif
            GE_Restore_HAL_Context(&pGECtxLocal->halLocalCtx,pGECtxLocal->pSharedCtx->bNotFirstInit);
            GE_Init(&pGECtxLocal->halLocalCtx, &cfg);
            //Init GE Palette from HW:
            GE_InitCtxHalPalette(&pGECtxLocal->halLocalCtx);
            if (bNeedSetActiveCtrlMiu1)
                GE_SetActiveCtrlMiu1(&pGECtxLocal->halLocalCtx);
            break;
        default:
            break;
    }
     return E_GE_OK;
}

GE_Result MDrv_GE_STR_SetPowerState(EN_POWER_MODE u16PowerState, void* pModule)
{
#if defined(MSOS_TYPE_LINUX_KERNEL)
    MS_U8 i=0UL;
    MS_PHY u32NonPMBankSize=0xFFFF;

    GE_STR_SAVE_AREA* pGFX_STRPrivate=NULL;
    UtopiaModuleGetSTRPrivate(pModule, (void**)&pGFX_STRPrivate);
    if( !MDrv_MMIO_GetBASE(&g_halGECtxLocal.va_mmio_base, &u32NonPMBankSize, MS_MODULE_GE))
    {
        MS_CRITICAL_MSG(GE_D_ERR("Get GE IOMap failure\n"));
        MS_ASSERT(0);
        return E_GE_FAIL;
    }

    switch(u16PowerState)
    {
        case E_POWER_SUSPEND:
        {
            //wait idle
            MDrv_GESTR_WaitIdle();
            //store reg
            for(i =0;i<GE_TABLE_REGNUM;i++ )
            {
                pGFX_STRPrivate->GE0_Reg[i] = STR_GE_REG(i);
                pGFX_STRPrivate->GETLB_Reg[i] = STR_GETLB_REG(i);
            }
            //disable power
            pGFX_STRPrivate->GECLK_Reg = STR_CLK_REG(CHIP_GE_CLK);
#if GE_USE_HW_SEM
            MDrv_SEM_Get_ResourceID(GESEMID, &(pGFX_STRPrivate->HWSemaphoreID));
            if(pGFX_STRPrivate->HWSemaphoreID)
            {
                if(MDrv_SEM_Free_Resource(GESEMID, pGFX_STRPrivate->HWSemaphoreID)==FALSE)
                {
                    GE_D_DBUG("[%s]Free resource fail\n",__FUNCTION__);
                    return E_GE_FAIL;
                }
            }
#endif
#if defined(CONFIG_SUPPORT_MULTI_GATE)
            MHal_GE_SetGClk(&g_halGECtxLocal, FALSE);
#endif
        }
            break;
        case E_POWER_RESUME:
        {
#if defined(CONFIG_SUPPORT_MULTI_GATE)
            MHal_GE_SetGClk(&g_halGECtxLocal, TRUE);
#endif
#if GE_USE_HW_SEM
            if(pGFX_STRPrivate->HWSemaphoreID)
            {
                MDrv_SEM_Get_Resource(GESEMID, pGFX_STRPrivate->HWSemaphoreID);
            }
#endif
            //Restore registers according to the value recorded in pGFX_STRPrivate
            HAL_GE_STR_RestoreReg(&g_halGECtxLocal, pGFX_STRPrivate);
            //resume palette
            MDrv_GESTR_SetPalette(pGFX_STRPrivate->GEPalette);

        }
            break;
        default:
            break;
    }
#endif
     return E_GE_OK;
}
//-------------------------------------------------------------------------------------------------
///Set GE Clock
/// @param  pGECtx                     \b IN: GE context
/// @param  bEnable         \b IN: Enable GE clock or not.
/// @return @ref GE_Result
//-------------------------------------------------------------------------------------------------
GE_Result MDrv_GE_SetClock(GE_Context *pGECtx, MS_BOOL bEnable)
{
    MS_U16 u16tmp = 0;
    GE_CTX_LOCAL *pGECtxLocal = (GE_CTX_LOCAL*)pGECtx;

    GE_ENTRY(pGECtx);

    GE_WaitIdle(&pGECtxLocal->halLocalCtx);

    if (bEnable)
    {
        GE_SetClock(&pGECtxLocal->halLocalCtx, TRUE);
        u16tmp = GE_ReadReg(&pGECtxLocal->halLocalCtx, REG_GE_EN);
        u16tmp |= GE_EN_GE;
        GE_WriteReg(&pGECtxLocal->halLocalCtx, REG_GE_EN, u16tmp);
    }
    else
    {
        u16tmp = GE_ReadReg(&pGECtxLocal->halLocalCtx, REG_GE_EN);
        u16tmp &= ~GE_EN_GE;
        GE_WriteReg(&pGECtxLocal->halLocalCtx, REG_GE_EN, u16tmp);
        GE_SetClock(&pGECtxLocal->halLocalCtx, FALSE);
    }

    GE_RETURN(pGECtx,E_GE_OK);
}
MS_BOOL _GE_TextOutPatch(GE_Context *pGECtx)
{
    GE_ENTRY(pGECtx);
#ifdef GE_TEXTOUT_BLEND_PATCH
    GE_RETURN(pGECtx,TRUE);
#else
    GE_RETURN(pGECtx,FALSE);
#endif
}

GE_Result MDrv_GE_SetTLBMode(GE_Context *pGECtx, GE_TLB_Mode tlb_type)
{
    GE_CTX_LOCAL *pGECtxLocal = (GE_CTX_LOCAL*)pGECtx;

    GE_ENTRY(pGECtx);

    GE_SetTLBMode(&pGECtxLocal->halLocalCtx, tlb_type);
    GE_RETURN(pGECtx,E_GE_OK);
}

GE_Result MDrv_GE_GetTLBSRCADDR(GE_Context *pGECtx, MS_PHY* addr)
{
    GE_CTX_LOCAL *pGECtxLocal = (GE_CTX_LOCAL*)pGECtx;

    GE_ENTRY(pGECtx);
    GE_GetTLBSRCADDR(&pGECtxLocal->halLocalCtx,addr);

    GE_RETURN(pGECtx,E_GE_OK);
}

GE_Result MDrv_GE_GetTLBDSTADDR(GE_Context *pGECtx, MS_PHY* addr)
{
    GE_CTX_LOCAL *pGECtxLocal = (GE_CTX_LOCAL*)pGECtx;

    GE_ENTRY(pGECtx);
    GE_GetTLBDSTADDR(&pGECtxLocal->halLocalCtx,addr);

    GE_RETURN(pGECtx,E_GE_OK);
}

GE_Result MDrv_GE_SetTLBSrcBaseAddr(GE_Context *pGECtx, MS_PHY phyaddr)
{
    GE_CTX_LOCAL *pGECtxLocal = (GE_CTX_LOCAL*)pGECtx;

    GE_ENTRY(pGECtx);

    GE_SetTLBSrcBaseAddr(&pGECtxLocal->halLocalCtx, phyaddr);
    GE_RETURN(pGECtx,E_GE_OK);
}

GE_Result MDrv_GE_SetTLBDstBaseAddr(GE_Context *pGECtx, MS_PHY phyaddr)
{
    GE_CTX_LOCAL *pGECtxLocal = (GE_CTX_LOCAL*)pGECtx;

    GE_ENTRY(pGECtx);

    GE_SetTLBDstBaseAddr(&pGECtxLocal->halLocalCtx, phyaddr);
    GE_RETURN(pGECtx,E_GE_OK);
}

GE_Result MDrv_GE_TLBFlushTable(GE_Context *pGECtx,MS_BOOL bEnable)
{
    GE_CTX_LOCAL *pGECtxLocal = (GE_CTX_LOCAL*)pGECtx;

    GE_ENTRY(pGECtx);

    GE_FlushTLBTable(&pGECtxLocal->halLocalCtx,bEnable);
    GE_RETURN(pGECtx,E_GE_OK);
}

GE_Result MDrv_GE_SetTLBTag(GE_Context *pGECtx,MS_U16 tag)
{
    GE_CTX_LOCAL *pGECtxLocal = (GE_CTX_LOCAL*)pGECtx;

    GE_ENTRY(pGECtx);

    GE_SetTLBTag(&pGECtxLocal->halLocalCtx,tag);
    GE_RETURN(pGECtx,E_GE_OK);
}

GE_Result MDrv_GE_StopTLBFlush(GE_Context *pGECtx)
{
    GE_CTX_LOCAL *pGECtxLocal = (GE_CTX_LOCAL*)pGECtx;

    GE_ENTRY(pGECtx);

    GE_StopFlushTLB(&pGECtxLocal->halLocalCtx);
    GE_RETURN(pGECtx,E_GE_OK);
}

GE_Result MDrv_GE_SetBurstMiuLen(GE_Context *pGECtx,MS_BOOL bEnable,MS_U32 u32BurstLen)
{
    GE_CTX_LOCAL *pGECtxLocal = (GE_CTX_LOCAL*)pGECtx;

    GE_ENTRY(pGECtx);
    HAL_GE_SetBurstMiuLen(&pGECtxLocal->halLocalCtx,bEnable,u32BurstLen);
    GE_RETURN(pGECtx,E_GE_OK);
}
GE_Result MDrv_GE_RestoreRegInfo(GE_Context *pGECtx,GE_RESTORE_REG_ACTION eRESTORE_ACTION, MS_U16 *value)
{
    GE_CTX_LOCAL *pGECtxLocal = (GE_CTX_LOCAL*)pGECtx;

    GE_ENTRY(pGECtx);

    switch(eRESTORE_ACTION)
    {
        case E_GE_SAVE_REG_GE_EN:
            *value = GE_ReadReg(&pGECtxLocal->halLocalCtx, REG_GE_EN);
            break;
        case E_GE_RESTORE_REG_GE_EN:
            GE_WriteReg(&pGECtxLocal->halLocalCtx, REG_GE_EN, *value);
            break;
        case E_GE_DISABLE_REG_EN:
            GE_WriteReg(&pGECtxLocal->halLocalCtx, REG_GE_EN, (*value & ~(GE_EN_ASCK | GE_EN_DSCK | GE_EN_SCK | GE_EN_DCK | GE_EN_BLEND | GE_EN_DFB_BLD | GE_EN_DITHER)));
            break;
        default:
            break;
    }

    GE_RETURN(pGECtx,E_GE_OK);
}

GE_Result MDrv_GE_GetCRC(GE_Context *pGECtx, MS_U32 *pu32CRCvalue)
{
    GE_CTX_LOCAL *pGECtxLocal = (GE_CTX_LOCAL*)pGECtx;

    GE_ENTRY(pGECtx);
    HAL_GE_GetCRC(&pGECtxLocal->halLocalCtx, pu32CRCvalue);
    GE_RETURN(pGECtx,E_GE_OK);
}

GE_Result MDrv_GE_ConfigCRC(GE_Context *pGECtx, ST_DRV_GE_ConfigCRC *pstConfigCRC)
{
    GE_CTX_LOCAL *pGECtxLocal = (GE_CTX_LOCAL*)pGECtx;

    GE_ENTRY(pGECtx);
    HAL_GE_ConfigCRC(&pGECtxLocal->halLocalCtx, pstConfigCRC);
    GE_RETURN(pGECtx,E_GE_OK);
}

#if defined(CONFIG_SUPPORT_MULTI_GATE)
GE_Result MDrv_GE_SetGClk(GE_Context *pGECtx, MS_BOOL bEnClk)
{
    GE_CTX_LOCAL *pGECtxLocal = (GE_CTX_LOCAL*)pGECtx;
    MHal_GE_SetGClk(&pGECtxLocal->halLocalCtx, bEnClk);

    return E_GE_OK;
}
#endif

//-------------------------------------------------------------------------------------------------
///GE performance test function
//-------------------------------------------------------------------------------------------------
#if (defined(CONFIG_UTOPIA_PROC_DBG_SUPPORT) || (GE_PERFORMANCE_TEST == 1))
#define CLK_Reg(address)        (*((volatile MS_U32 *)(REG_CLK_BASE + ((address)<<2) )))
#define REG_ADDR(addr)              (*((volatile MS_U16*)(pGECtxLocal->halLocalCtx.va_mmio_base2 + ((addr) << 1))))
#define REG_CLK_BASE                 (pGECtxLocal->halLocalCtx.va_mmio_base2+0x201600)

#define REG_WR(_reg_, _val_)        do{ REG_ADDR(_reg_) = (_val_);  }while(0)
#define REG_RR(_reg_)               ({ REG_ADDR(_reg_);})
#define REG_PIU_TIMER0(_x_)              (0x003020UL | (_x_ << 1))

#define REG_GE_MIUCLIENT    (0x1006F4UL)

#define MIU_LOG    0UL        //miu flow rate enable  --> we could use this to check GE miu_request percentage

#define REG_MIU0_CTRL       (0x10121AUL)
#define REG_MIU0_CNT        (0x10121CUL)

#define REG_MIU1_CTRL       (0x10061AUL)
#define REG_MIU1_CNT        (0x10061CUL)

#define REG_MIU0_GROUP0_MASK (0x101246UL)
#define REG_MIU0_GROUP1_MASK (0x101266UL)
#define REG_MIU0_GROUP2_MASK (0x101286UL)
#define REG_MIU0_GROUP3_MASK (0x1012A6UL)

#define REG_MIU1_GROUP0_MASK (0x100646UL)
#define REG_MIU1_GROUP1_MASK (0x100666UL)
#define REG_MIU1_GROUP2_MASK (0x100686UL)
#define REG_MIU1_GROUP3_MASK (0x1006A6UL)


#define MAX_TEST_NUM 9UL
MS_S32 test_rect_w[MAX_TEST_NUM] = { 10,100,100,100,500,500,1280, 1920, 3840};
MS_S32 test_rect_h[MAX_TEST_NUM] = {100, 10,100,500,100,500, 720, 1080, 2160};
MS_S32 s32StretchRectW[MAX_TEST_NUM] = { 12,120,120,120,600,600,1536, 2304, 4608};
MS_S32 s32StretchRectH[MAX_TEST_NUM] = {120, 12,120,600,120,600, 864, 1296, 2592};

//MS_S32 test_rect_w[MAX_TEST_NUM] = { 1280,1280,1280,100,500,500,1280};
//MS_S32 test_rect_h[MAX_TEST_NUM] = {720, 720,720,500,100,500, 720};

#define BLTCNT 1000UL
#define CMD_BLT  0x40UL
#define CMD_FILL 0x30UL

#define GE_TEST_CMD CMD_BLT
#define GE_1P_MODE 1UL

#define GE_MIU_SEL 0x000UL   //0: miu0  0x8000: miu1

#define NORMAL_BLT   0x0UL
#define Stretch_BLT  0x1UL
#define BLENDING_BLT 0x4UL

#define STRETCH_RATIO 1.2
#define GE_STRETCH  BLENDING_BLT

#define GE_ARGB8888 0x1UL
#define GE_ARGB4444 0x10UL
#define TEST_CASE GE_ARGB8888 | GE_ARGB4444
char _MDrv_GE_PerformanceGetPara(void)
{
    char c='\0';

#if (GE_PERFORMANCE_TEST==1)
    c = getchar();
    getchar();
#else
    char* psep;
    psep = strsep(&_gpcCmdLine," ");
    if(psep!=NULL)
    {
        c = psep[0];
        GE_D_INFO("para:%c\n",c);
    }
#endif

    return c;
}

MS_U32 _MDrv_GE_PerformanceGetCusSize(void)
{
    char* psep;
    MS_U32 u32Size = 0;

    psep = strsep(&_gpcCmdLine," ");
    u32Size=  atoi(psep);
    GE_D_INFO("para:%d\n",u32Size);

    return u32Size;
}
//---------------------------------------------------------------------------
///Verify GE bitblt performance
/// @param  pGECtx                     \b IN: GE context
/// @param  bEnable         \b IN: Enable GE clock or not.
/// @return @ref GE_Result
//---------------------------------------------------------------------------
GE_Result MDrv_GE_BitbltPerformance(GE_Context *pGECtx)
{
    GE_CTX_LOCAL *pGECtxLocal = (GE_CTX_LOCAL*)pGECtx;
    char c;
    MS_U16 GEstatus;
    //MS_BOOL bBusy,bBusy2,bBusy3;
    MS_U16 rect_w,rect_h;
    MS_U16 stretch_w, stretch_h;
    MS_U8 test_idx = 0;
    MS_U16 GE_Mode = (GE_1P_MODE<<15) | GE_EN_GE;
    MS_U16 GE_Cmd = GE_TEST_CMD;
    MS_U8 GE_Stretch_En = NORMAL_BLT;
    MS_U16 GE_Burst = 0x0000;
    MS_U16 GE_Spilt = 0x0000;
    MS_U16 GE_BASE = GE_MIU_SEL;
    MS_U16 GE_BASE_H = GE_MIU_SEL;
    MS_U16 GE_THRESHOLD = 0x7;
    MS_U16 pitch=0;
    MS_BOOL bPitchAlign=TRUE;
    MS_BOOL bCustomTest;
    MS_BOOL bTestARGB4444, bTestARGB8888;
    MS_U32 u32CusWidth, u32CusHeight, u32CusStretchW, u32CusStretchH, u32TestTimes;
#if (MIU_LOG)
    MS_U16 GE_GROUP = MIU1_GEGROUP;
    MS_U16 GE_group_client;
#endif
    GE_D_INFO("GE_EN:%x\n",GE_Mode);

    GE_ENTRY(pGECtx);

#if 0 //Enhance GE performance(i.e for agate)
    REG_WR(0x1012FE, 0x011E);   //priority 2>3>1>0
    REG_WR(0x101240, 0x8011);   //disable flow control group 0/1/2
    REG_WR(0x101260, 0x8011);
    REG_WR(0x101280, 0x8011);

    REG_WR(0x1012FC, 0xA400);  //FIFO32
    REG_WR(0x101228, 0x4090);  //PACK enable
#endif
#if 0 //Enhance GE performance(i.e for A3)
    REG_WR(0x1012FE, 0x00D2);   //priority 2>0>1>3
    REG_WR(0x101240, 0x8011);   //disable flow control
    REG_WR(0x101260, 0x8011);
    REG_WR(0x101280, 0x8011);

    REG_WR(0x1012FC, 0xA400);   //FIFO32
    REG_WR(0x101228, 0x4000);   //PACK enable

    REG_WR(0x101246, 0xffff);
    REG_WR(0x101248, 0x0000);
    REG_WR(0x101266, 0xffff);
    REG_WR(0x101268, 0x0000);
#endif
        pGECtxLocal->pSharedCtx->halSharedCtx.bGE_DirectToReg =TRUE;

        //=======================Get conditions======Start=================================//
       GE_D_INFO("\n[MIU Sel]       Type '0'(miu0) or '1'(miu1) or '2'(miu2) to sel MIU Base\n");
       //c = getchar();
        c = _MDrv_GE_PerformanceGetPara();
        if (c=='2')
        {
            GE_BASE_H = 0x6000;
            GE_BASE = 0x0000;    //miu2
        }
        else if (c=='1')
        {
            GE_BASE_H = 0x0000;
            GE_BASE = 0x8000;  //miu1
        }
        else
        {
            GE_BASE_H = 0x0000;
            GE_BASE = 0x0000;  //miu0
        }

        //c = getchar();

        GE_D_INFO("[1P/4P(8P) mode]    Type '1'(1P) or '4'(4P/8P) to sel engine mode\n");
        //c = getchar();
        c = _MDrv_GE_PerformanceGetPara();
        if (c=='1')
            GE_Mode = (GE_1P_MODE<<15) | GE_EN_GE;
        else
            GE_Mode = ((!GE_1P_MODE)<<15) | GE_EN_GE;
        //c = getchar();

        GE_D_INFO("[BLT mode]      Type 'b'(Bitblt) or 'f'(Rectfill) to sel BLT mode\n");
        c = _MDrv_GE_PerformanceGetPara();
        //c = getchar();
        if (c=='f')
            GE_Cmd = CMD_FILL;
        else
            GE_Cmd = CMD_BLT;
        //c = getchar();

        GE_D_INFO("[BLT Type]      Type 'n'(Normal_BLT) or 's'(Stretch_BLT) or 'b'(Blending_BLT) or 'a'(Blending And Stretch) to set BLT type\n");
        //c = getchar();
        c = _MDrv_GE_PerformanceGetPara();
        if (c == 'n')
            GE_Stretch_En = NORMAL_BLT;
        else if (c == 's')
            GE_Stretch_En |= Stretch_BLT;
        else if(c == 'b')
            GE_Stretch_En |= BLENDING_BLT;
        else
        {
            GE_Stretch_En |= Stretch_BLT;
            GE_Stretch_En |= BLENDING_BLT;
        }

        //c = getchar();

        GE_D_INFO("[THRESHOLD]     Type '7'(0x7) or 'f'(0xf) or Others to set engine threshold\n");
        //c = getchar();
        c = _MDrv_GE_PerformanceGetPara();

        if (c=='f')
            GE_THRESHOLD = 0xF;
        else if(c =='7')
            GE_THRESHOLD = 0x7;
        else
            GE_THRESHOLD = 0xD;

#ifdef GE_EN_BURST
        //c = getchar();

        GE_D_INFO("[Burst Type]      Type 'b'(Burst) or 'n'(ormal) to speed up\n");
        c = _MDrv_GE_PerformanceGetPara();
        //c = getchar();
        if (c == 'b')
            GE_Burst = 0x4000;
        else
            GE_Burst = 0x0000;
#endif

    if(*(&pGECtxLocal->halLocalCtx.pGeChipPro->bSupportSpiltMode))
    {
        //c = getchar();
        GE_D_INFO("[Spilt Type]      Type 's'(Spilt) or 'n'(ormal) to speed up\n");
        c = _MDrv_GE_PerformanceGetPara();
        //c = getchar();
        if (c == 's')
            GE_Spilt = 0x4000;
        else
            GE_Spilt = 0x0000;
    }

    GE_D_INFO("[Custom Setting]      Type 'y'(Yes) or 'n'(No) for custom setting\n");
    c = _MDrv_GE_PerformanceGetPara();
    if(c == 'y')
    {
        bCustomTest = TRUE;

        GE_D_INFO("[Custom Setting]      Type custom width\n");
        u32CusWidth = _MDrv_GE_PerformanceGetCusSize();

        GE_D_INFO("[Custom Setting]      Type custom height\n");
        u32CusHeight = _MDrv_GE_PerformanceGetCusSize();

        GE_D_INFO("[Custom Setting]      Type custom stretch width\n");
        u32CusStretchW = _MDrv_GE_PerformanceGetCusSize();

        GE_D_INFO("[Custom Setting]      Type custom stretch height\n");
        u32CusStretchH = _MDrv_GE_PerformanceGetCusSize();

        GE_D_INFO("[Custom Setting]      Type custom rendering times\n");
        u32TestTimes = _MDrv_GE_PerformanceGetCusSize();

        GE_D_INFO("[Custom Setting]      Type '1' for ARGB8888 '2' for ARGB4444 '3' for both\n");
        c = _MDrv_GE_PerformanceGetPara();
        if (c == '1')
        {
            bTestARGB4444 = FALSE;
            bTestARGB8888 = TRUE;
        }
        else if (c == '2')
        {
            bTestARGB4444 = TRUE;
            bTestARGB8888 = FALSE;
        }
        else
        {
            bTestARGB4444 = TRUE;
            bTestARGB8888 = TRUE;
        }
    }
    else
    {
        bCustomTest = FALSE;
        u32TestTimes = BLTCNT;
        bTestARGB4444 = TRUE;
        bTestARGB8888 = TRUE;
    }
    //=======================Get conditions====End======================================//


    GE_D_INFO("%s start\n",__FUNCTION__);

    if(bCustomTest == TRUE)
    {
        GE_D_INFO("GE custom test\n");
        rect_w = u32CusWidth;
        rect_h = u32CusHeight;
    }
    else
    {
        rect_w = test_rect_w[test_idx];
        rect_h = test_rect_h[test_idx];
    }

    if(GE_Stretch_En & Stretch_BLT )
    {

        GE_D_INFO("Enable Stretch Blt\n");
        if(bCustomTest == TRUE)
        {
            stretch_w = u32CusStretchW;
            stretch_h = u32CusStretchH;
        }
        else
        {
            stretch_w = s32StretchRectW[test_idx];//rect_w * STRETCH_RATIO;
            stretch_h = s32StretchRectH[test_idx];//rect_h * STRETCH_RATIO;
        }
        GE_WriteReg(&pGECtxLocal->halLocalCtx,REG_GE_CFG, GE_CFG_BLT_STRETCH | GE_CFG_CMDQ);

        GE_WriteReg(&pGECtxLocal->halLocalCtx,REG_GE_BLT_DST_X_OFST, 0x1000);
        GE_WriteReg(&pGECtxLocal->halLocalCtx,REG_GE_BLT_DST_Y_OFST, 0x1000);

        GE_WriteReg(&pGECtxLocal->halLocalCtx,REG_GE_BLT_SRC_DX, (rect_w * 4096)/stretch_w);
        GE_WriteReg(&pGECtxLocal->halLocalCtx,REG_GE_BLT_SRC_DY, (rect_h * 4096)/stretch_h);

    }
    else
    {
        GE_D_INFO("Normal Case\n");
        stretch_w = rect_w;
        stretch_h = rect_h;
    }


    if(GE_Stretch_En & BLENDING_BLT)
    {
        GE_D_INFO("enable alpha blending\n");
        GE_Mode |= BLENDING_BLT;
        MDrv_GE_SetAlphaBlend(pGECtx,TRUE,E_GE_BLEND_ROP8_SRCOVER);
        //stretch_w = rect_w;
        //stretch_h = rect_h;
    }

    CLK_Reg(0x48) = 0x0;
    //REG_WR(0x100B90, 0x00);  //enable miu counter


#if 0
    REG_WR(REG_MIU0_GROUP0_MASK, 0xFFF8);  //enable miu counter
    REG_WR(REG_MIU0_GROUP1_MASK, 0xFFFF);  //enable miu counter
    REG_WR(REG_MIU0_GROUP2_MASK, 0xFFFE);  //enable miu counter
    REG_WR(REG_MIU0_GROUP3_MASK, 0xFFFE);  //enable miu counter

    REG_WR(REG_MIU1_GROUP0_MASK, 0xFFF8);  //enable miu counter
    REG_WR(REG_MIU1_GROUP1_MASK, 0xFFFF);  //enable miu counter
    REG_WR(REG_MIU1_GROUP2_MASK, 0xFFFE);  //enable miu counter
    REG_WR(REG_MIU1_GROUP3_MASK, 0xFFFE);  //enable miu counter
#endif

    MS_U32 i, u32Timer,avgtime,u32Timer_ms;
    MS_U64  Total_Byte;

TEST_START:
    GE_D_INFO("Test Case :%d Rect [ %d x %d ] to [%d x %d]\n",test_idx,rect_w,rect_h,stretch_w,stretch_h);

    GE_WriteReg(&pGECtxLocal->halLocalCtx,REG_GE_EN, GE_Mode|GE_Burst);   //Burst

    GE_WriteReg(&pGECtxLocal->halLocalCtx,REG_GE_CFG, GE_ReadReg(&pGECtxLocal->halLocalCtx,REG_GE_CFG)|GE_Spilt);  //Spilt

    if(GE_ReadReg(&pGECtxLocal->halLocalCtx,REG_GE_EN)& 0x4000)
        GE_D_INFO("\33[0\34m Burst Enable \33[m\n");
    else
        GE_D_INFO("\33[0\34m Burst Disable \33[m\n");


    if(GE_ReadReg(&pGECtxLocal->halLocalCtx,REG_GE_CFG)& 0x4000)
        GE_D_INFO("\33[0\34m Spilt Enable \33[m\n");
    else
        GE_D_INFO("\33[0\34m Spilt Disable \33[m\n");

    //GE_WriteReg(&pGECtxLocal->halLocalCtx,GE_REG_FMT_BLT, 0x1);
    GE_WriteReg(&pGECtxLocal->halLocalCtx,REG_GE_TH, GE_THRESHOLD);

    /*src/dst miu info*/
    GE_WriteReg(&pGECtxLocal->halLocalCtx,REG_GE_YUV_MODE, (GE_ReadReg(&pGECtxLocal->halLocalCtx,REG_GE_YUV_MODE)&0x9fff)|GE_BASE_H);

    /*src info*/
    GE_WriteReg(&pGECtxLocal->halLocalCtx,REG_GE_SRC_BASE_L, 0x2000);
    GE_WriteReg(&pGECtxLocal->halLocalCtx,REG_GE_SRC_BASE_H, 0x3B8 | GE_BASE);

    GE_WriteReg(&pGECtxLocal->halLocalCtx,REG_GE_BLT_SRC_W, rect_w);
    GE_WriteReg(&pGECtxLocal->halLocalCtx,REG_GE_BLT_SRC_H, rect_h);


    GE_WriteReg(&pGECtxLocal->halLocalCtx,REG_GE_PRIM_V2_X, 0);
    GE_WriteReg(&pGECtxLocal->halLocalCtx,REG_GE_PRIM_V2_Y, 0);

    /*dst info*/
    GE_WriteReg(&pGECtxLocal->halLocalCtx,REG_GE_DST_BASE_L, 0x6000);
    GE_WriteReg(&pGECtxLocal->halLocalCtx,REG_GE_DST_BASE_H, 0xBF0 | GE_BASE );

    GE_WriteReg(&pGECtxLocal->halLocalCtx,REG_GE_PRIM_V0_X, 0);
    GE_WriteReg(&pGECtxLocal->halLocalCtx,REG_GE_PRIM_V0_Y, 0);
    GE_WriteReg(&pGECtxLocal->halLocalCtx,REG_GE_PRIM_V1_X, stretch_w-1);
    GE_WriteReg(&pGECtxLocal->halLocalCtx,REG_GE_PRIM_V1_Y, stretch_h-1);

    /*set clip win*/
    GE_WriteReg(&pGECtxLocal->halLocalCtx,REG_GE_CLIP_L, 0);
    GE_WriteReg(&pGECtxLocal->halLocalCtx,REG_GE_CLIP_R, stretch_w-1);
    GE_WriteReg(&pGECtxLocal->halLocalCtx,REG_GE_CLIP_T, 0);
    GE_WriteReg(&pGECtxLocal->halLocalCtx,REG_GE_CLIP_B, stretch_h-1);

    /*Set burst length*/
    GE_WriteReg(&pGECtxLocal->halLocalCtx,_REG_GE_DBG, 0x1000);

    /*set color format*/
    //ARGB8888
    if(bTestARGB8888 == TRUE)
    {
        if(bPitchAlign)
        {
            if (((rect_w*4)%GE_WordUnit)==0)
            {
                pitch=rect_w*4;
                GE_D_INFO("[alignment]SRC pitch=%d\n",pitch);
            }
            else
            {
                pitch=(((rect_w)/GE_WordUnit)+1)*4*GE_WordUnit;
                GE_D_INFO("[Not alignment]SRC pitch=%d\n",pitch);
            }
        }
        else
            pitch=rect_w*4;

        GE_WriteReg(&pGECtxLocal->halLocalCtx,REG_GE_SRC_PITCH, pitch);

        if(bPitchAlign)
        {
            if (((stretch_w*4)%GE_WordUnit)==0)
            {
                pitch=stretch_w*4;
                GE_D_INFO("[alignment]DST pitch=%d\n",pitch);
            }
            else
            {
                pitch=(((stretch_w)/GE_WordUnit)+1)*4*GE_WordUnit;
                GE_D_INFO("[Not alignment]DST pitch=%d\n",pitch);
            }
        }
        else
            pitch=stretch_w*4;

        GE_WriteReg(&pGECtxLocal->halLocalCtx,REG_GE_DST_PITCH, pitch);
        GE_WriteReg(&pGECtxLocal->halLocalCtx,REG_GE_FMT,( GE_GetFmt(E_GE_FMT_ARGB8888) + ( GE_GetFmt(E_GE_FMT_ARGB8888)<<8 )));

        GE_D_INFO("=== cnt:%lu  MIU_SEL_L_bit:0x%x  MIU_SEL_H_bit:0x%x===\n\n",u32TestTimes,GE_BASE,GE_BASE_H);


#if (MIU_LOG)
        if(GE_GROUP == 0x7A) //group2  -- A5/A3/A2
        {
            GE_group_client = 0x2000;
        }
        else if(GE_GROUP == 0x7B) //group 3 -- others
        {
            GE_group_client = 0x3000;
        }

        if(GE_BASE == 0)
        {
            REG_WR(REG_MIU0_CTRL, 0x50 | GE_group_client);  //enable miu counter
            REG_WR(REG_MIU0_CTRL, 0x51 | GE_group_client);  //enable miu counter
        }
        else
        {
            REG_WR(REG_MIU1_CTRL, 0x50 | GE_group_client);  //enable miu counter
            REG_WR(REG_MIU1_CTRL, 0x51 | GE_group_client);  //enable miu counter
        }
#endif
        MDrv_GE_WaitIdle(pGECtx);
        REG_WR(REG_PIU_TIMER0(0x02), 0xFFFF);
        REG_WR(REG_PIU_TIMER0(0x03), 0xFFFF);

        REG_WR(REG_PIU_TIMER0(0x00), 0x0);
        REG_WR(REG_PIU_TIMER0(0x00), 0x1);
        for (i=0; i<u32TestTimes; i++)
        {
            GEstatus = GE_ReadReg(&pGECtxLocal->halLocalCtx,REG_GE_STAT);
            if(((GEstatus & GE_STAT_CMDQ2_MASK)>>GE_STAT_CMDQ2_SHFT) <= 2)
            {
                do
                {
                    GEstatus = GE_ReadReg(&pGECtxLocal->halLocalCtx,REG_GE_STAT);
                    if(((GEstatus & GE_STAT_CMDQ2_MASK)>>GE_STAT_CMDQ2_SHFT)> 2 )
                    {
                        break;
                    }
                }while(1);
            }

            _GE_SEMAPHORE_ENTRY(pGECtx,E_GE_POOL_ID_INTERNAL_REGISTER);
            GE_WriteReg(&pGECtxLocal->halLocalCtx,REG_GE_CMD, GE_Cmd);
            _GE_SEMAPHORE_RETURN(pGECtx,E_GE_POOL_ID_INTERNAL_REGISTER);

#if (MIU_LOG)
            if(i%100 == 0)
            {
                if(GE_BASE == 0)
                    GE_D_INFO("i:%d MIU0: %x\n",i,REG_RR(REG_MIU0_CNT));
                else
                    GE_D_INFO("i:%d MIU1: %x\n",i,REG_RR(REG_MIU1_CNT));

            }
#endif
        }

       /* do
        {
            GEstatus = GE_ReadReg(&pGECtxLocal->halLocalCtx,REG_GE_STAT);
            bBusy = (MS_BOOL)GEstatus & GE_STAT_BUSY;
            if(bBusy == 0)
            {
                GEstatus = GE_ReadReg(&pGECtxLocal->halLocalCtx,REG_GE_STAT);
                bBusy2 = (MS_BOOL)GEstatus & GE_STAT_BUSY;
                if(bBusy2 == 0)
                {
                    GEstatus = GE_ReadReg(&pGECtxLocal->halLocalCtx,REG_GE_STAT);
                    bBusy3 = (MS_BOOL)GEstatus & GE_STAT_BUSY;
                    printf("GE idle:%d\n",bBusy3);
                    break;
                }
            }
        }while(bBusy != 0); */

        MDrv_GE_WaitIdle(pGECtx);
        u32Timer = ((MS_U32)REG_RR(REG_PIU_TIMER0(0x04))) | ((MS_U32)REG_RR(REG_PIU_TIMER0(0x05)) << 16);
        u32Timer_ms = ((u32Timer))/12/1000;

        if(GE_Cmd == CMD_BLT)
        {
            GE_D_INFO("\nMHal_GOP_BLTTest ARGB8888 cnt:%lu total time:%u us\n",u32TestTimes, (MS_U32)(u32Timer/12));
            GE_D_INFO(" MHal_GOP_BLTTest ARGB8888 time:%d ms\n", u32Timer_ms);
        }
        else
        {
            GE_D_INFO("\n GE_RectFill ARGB8888 cnt:%lu total time:%u us\n",u32TestTimes, (MS_U32)(u32Timer/12));
            GE_D_INFO("GE_RectFill ARGB8888 time:%d ms\n", u32Timer_ms);
        }

        Total_Byte = (stretch_w * stretch_h * 4);
        GE_D_INFO("Rect: [%d x %d] Size:%llu (%llu MB)\n",stretch_w,stretch_h,Total_Byte,(MS_U64)(Total_Byte*u32TestTimes/1024/1024));

        //avgtime = (((((MS_U64)(Total_Byte*BLTCNT)/(MS_U64)((u32Timer_ms)))*1000)/1024)/1024);  //XTAL = 12MHz
        avgtime = (((((MS_U64)(Total_Byte*u32TestTimes)/(MS_U64)((u32Timer/12)))*1000*1000)/1024)/1024);

        GE_D_INFO("\n\33[0;31m=== ARGB8888 Result: %d  MB/s ===\33[m\n\n",(MS_U32)avgtime);
    }


    //ARGB4444
    if(bTestARGB4444==TRUE)
    {
        GE_D_INFO("cnt:%lu\n",u32TestTimes);
        if(bPitchAlign)
        {
            if (((rect_w*2)%GE_WordUnit)==0)
            {
                pitch=rect_w*2;
                GE_D_INFO("[alignment]SRC pitch=%d\n",pitch);
            }
            else
            {
                pitch=(((rect_w)/GE_WordUnit)+1)*2*GE_WordUnit;
                GE_D_INFO("[Not alignment]SRC pitch=%d\n",pitch);
            }
        }
        else
            pitch=rect_w*2;

        GE_WriteReg(&pGECtxLocal->halLocalCtx,REG_GE_SRC_PITCH, pitch);

        if(bPitchAlign)
        {
            if (((stretch_w*2)%GE_WordUnit)==0)
            {
                pitch=stretch_w*2;
                GE_D_INFO("[alignment]DST pitch=%d\n",pitch);
            }
            else
            {
                pitch=(((stretch_w)/GE_WordUnit)+1)*GE_WordUnit;
                GE_D_INFO("[Not alignment]DST pitch=%d\n",pitch);
            }
        }
        else
            pitch=stretch_w*2;

        GE_WriteReg(&pGECtxLocal->halLocalCtx,REG_GE_DST_PITCH, pitch);
        GE_WriteReg(&pGECtxLocal->halLocalCtx,REG_GE_FMT, (GE_GetFmt(E_GE_FMT_ARGB4444)+ (GE_GetFmt(E_GE_FMT_ARGB4444)<<8)));
        MDrv_GE_WaitIdle(pGECtx);
        REG_WR(REG_PIU_TIMER0(0x00), 0x0);
        REG_WR(REG_PIU_TIMER0(0x00), 0x1);
        for (i=0; i<u32TestTimes; i++)
        {
            #if 1
            GEstatus = GE_ReadReg(&pGECtxLocal->halLocalCtx,REG_GE_STAT);
            //printf("GEStatus:%x\n",GEstatus);

            if(((GEstatus & GE_STAT_CMDQ2_MASK)>>GE_STAT_CMDQ2_SHFT) <= 2)
            {
                //printf("fifo full\n");
                do
                {
                    GEstatus = GE_ReadReg(&pGECtxLocal->halLocalCtx,REG_GE_STAT);
                    if(((GEstatus & GE_STAT_CMDQ2_MASK)>>GE_STAT_CMDQ2_SHFT)> 2 )
                    {
                        //printf("continue\n");
                        break;
                    }
                }while(1);
            }
            #endif

            //GE_WriteReg(&pGECtxLocal->halLocalCtx,REG_GE_CMD, 0x40);
            _GE_SEMAPHORE_ENTRY(pGECtx,E_GE_POOL_ID_INTERNAL_REGISTER);
            GE_WriteReg(&pGECtxLocal->halLocalCtx,REG_GE_CMD, GE_Cmd);
            _GE_SEMAPHORE_RETURN(pGECtx,E_GE_POOL_ID_INTERNAL_REGISTER);
        }

       /* do
        {
            GEstatus = GE_ReadReg(&pGECtxLocal->halLocalCtx,REG_GE_STAT);
            bBusy = (MS_BOOL)GEstatus & GE_STAT_BUSY;
            if(bBusy == 0)
            {
                GEstatus = GE_ReadReg(&pGECtxLocal->halLocalCtx,REG_GE_STAT);
                bBusy2 = (MS_BOOL)GEstatus & GE_STAT_BUSY;
                if(bBusy2 == 0)
                {
                    GEstatus = GE_ReadReg(&pGECtxLocal->halLocalCtx,REG_GE_STAT);
                    bBusy3 = (MS_BOOL)GEstatus & GE_STAT_BUSY;
                    GE_D_INFO("GE idle:%d\n",bBusy3);
                    break;
                }
            }
        }while(bBusy != 0);*/
        MDrv_GE_WaitIdle(pGECtx);
        u32Timer = ((MS_U32)REG_RR(REG_PIU_TIMER0(0x04))) |
                           ((MS_U32)REG_RR(REG_PIU_TIMER0(0x05)) << 16);

        u32Timer_ms = ((u32Timer))/12/1000;


        if(GE_Cmd == CMD_BLT)
        {
            GE_D_INFO("\nGE_BLTTest ARGB4444 cnt:%lu total time:%u us\n",u32TestTimes, (MS_U32)(u32Timer/12));
            GE_D_INFO("GE_BLTTest ARGB4444 time:%d sec\n", u32Timer_ms);
        }
        else
        {
            GE_D_INFO("\n GE_RectFill ARGB4444 cnt:%lu total time:%u us\n",u32TestTimes, (MS_U32)(u32Timer/12));
            GE_D_INFO("GE_RectFill ARGB4444 time:%d ms\n", u32Timer_ms);
        }

        Total_Byte = (stretch_w * stretch_h * 2);
        GE_D_INFO("Rect: [%d x %d] Size:%llu (%llu MB)\n",stretch_w,stretch_h,Total_Byte,(MS_U64)(Total_Byte*u32TestTimes/1024/1024));

        //avgtime = (((((Total_Byte*BLTCNT)/((u32Timer_ms)))*1000)/1024)/1024);  //XTAL = 12MHz
        avgtime = (((((MS_U64)(Total_Byte*u32TestTimes)/(MS_U64)((u32Timer/12)))*1000*1000)/1024)/1024);
        GE_D_INFO("\n\33[0;32m=== ARGB4444 Result: %d  MB/s ===\33[m\n\n",(MS_U32)avgtime);
    }


    test_idx++;

    if(test_idx == MAX_TEST_NUM || bCustomTest == TRUE)
    {
        GE_D_INFO("End of Test\n");
        MsOS_DelayTask(200);
        GE_RETURN(pGECtx,E_GE_OK);
        while(1);
    }

    GE_D_INFO("Reset Enging\n");
    GE_WriteReg(&pGECtxLocal->halLocalCtx,REG_GE_EN, 0x0);

    rect_w = test_rect_w[test_idx];
    rect_h = test_rect_h[test_idx];

    if(GE_Stretch_En & Stretch_BLT)
    {
        GE_D_INFO("Enable Stretch Blt\n");
        if(bCustomTest == TRUE)
        {
            stretch_w = u32CusStretchW;
            stretch_h = u32CusStretchH;
        }
        else
        {
            stretch_w = s32StretchRectW[test_idx];//rect_w * STRETCH_RATIO;
            stretch_h = s32StretchRectH[test_idx];//rect_h * STRETCH_RATIO;
        }
    }
    else
    {
        stretch_w = rect_w;
        stretch_h = rect_h;
    }
    MsOS_DelayTask(100);

    goto TEST_START;

    GE_RETURN(pGECtx,E_GE_OK);
}
#endif


#endif //_DRV_GE_C_



