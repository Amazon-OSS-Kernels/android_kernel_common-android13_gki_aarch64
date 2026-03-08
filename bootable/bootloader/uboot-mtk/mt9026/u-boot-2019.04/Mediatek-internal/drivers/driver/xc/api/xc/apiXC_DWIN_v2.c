#include "MsTypes.h"
#ifndef MSOS_TYPE_LINUX_KERNEL
#include <stdio.h>
#include <string.h>
#endif
#include "utopia.h"
#include "utopia_dapi.h"
#include "MsCommon.h"
#include "mhal_xc_chip_config.h"
#include "apiXC_DWIN_v2.h"
#include "XC_DWIN_private.h"
#include "mvideo_context.h"
#include "drv_sc_DIP_scaling.h"
#include "xc_hwreg_utility2.h"

#if (defined ANDROID) && (defined TV_OS)
#include <cutils/log.h>
#define printf LOGD
#ifndef LOGD // android 4.1 rename LOGx to ALOGx
#define LOGD ALOGD
#endif
#endif

#if(defined(CONFIG_MLOG))
#include "ULog.h"

// Debug Logs, level form low(INFO) to high(FATAL, always show)
// Function information, ex function entry
#ifndef XC_INFO
#define XC_INFO(x, args...) ULOGI("DIP API", x, ##args)
#endif
// Warning, illegal paramter but can be self fixed in functions
#ifndef XC_WARN
#define XC_WARN(x, args...) ULOGW("DIP API", x, ##args)
#endif
//  Need debug, illegal paramter.
#ifndef XC_DBUG
#define XC_DBUG(x, args...) ULOGD("DIP API", x, ##args)
#endif
// Error, function will be terminated but system not crash
#ifndef XC_ERR
#define XC_ERR(x, args...) ULOGE("DIP API", x, ##args)
#endif
// Critical, system crash. (ex. assert)
#ifndef XC_FATAL
#define XC_FATAL(x, args...) ULOGF("DIP API", x, ##args)
#endif

#else
// Debug Logs, level form low(INFO) to high(FATAL, always show)
// Function information, ex function entry
#ifndef XC_INFO
#define XC_INFO(x, args...) printf("DIP API", x, ##args)
#endif
// Warning, illegal paramter but can be self fixed in functions
#ifndef XC_WARN
#define XC_WARN(x, args...) printf("DIP API", x, ##args)
#endif
//  Need debug, illegal paramter.
#ifndef XC_DBUG
#define XC_DBUG(x, args...) printf("DIP API", x, ##args)
#endif
// Error, function will be terminated but system not crash
#ifndef XC_ERR
#define XC_ERR(x, args...) printf("DIP API", x, ##args)
#endif
// Critical, system crash. (ex. assert)
#ifndef XC_FATAL
#define XC_FATAL(x, args...) printf("DIP API", x, ##args)
#endif

#endif

#ifdef CONFIG_UTOPIA_PROC_DBG_SUPPORT
MS_U32 DIPMdbIoctl(MS_U32 u32Cmd, const void* const pArgs)
{
    MDBCMD_CMDLINE_PARAMETER *paraCmdLine;
    MDBCMD_GETINFO_PARAMETER *paraGetInfo;
    switch(u32Cmd)
    {
        case MDBCMD_CMDLINE:
            paraCmdLine = (MDBCMD_CMDLINE_PARAMETER *)pArgs;
            MdbPrint(paraCmdLine->u64ReqHdl,"---------MStar DIP state---------\n");
            MdbPrint(paraCmdLine->u64ReqHdl,"DIP only support get info function now\n");
            MdbPrint(paraCmdLine->u64ReqHdl,"---------------------------------\n");
            MDrv_XC_DIP_Mdb_Cmdline(paraCmdLine->u64ReqHdl,paraCmdLine->u32CmdSize,paraCmdLine->pcCmdLine);
            paraCmdLine->result = MDBRESULT_SUCCESS_FIN;
            break;
        case MDBCMD_GETINFO:
            paraGetInfo = (MDBCMD_GETINFO_PARAMETER *)pArgs;
            MdbPrint(paraGetInfo->u64ReqHdl,"=====Read all DIP Information Start=====\n");
            MDrv_XC_DIP_Mdb_GetInfo(paraGetInfo->u64ReqHdl);
            paraGetInfo->result = MDBRESULT_SUCCESS_FIN;
            MdbPrint(paraGetInfo->u64ReqHdl,"=====Read all DIP Information End=====\n");
            break;
        default:
            break;
    }
    return 0;
}
#endif

void DIPRegisterToUtopia(FUtopiaOpen ModuleType)
{
    SCALER_DIP_WIN i;
    char name[8] = "\0";
    void* psResource = NULL;
    // 1. deal with module
    void* pUtopiaModule = NULL;
    DIP_SHARE_RESOURCE_PRIVATE* pDipResPri = NULL;

    UtopiaModuleCreate(MODULE_DIP, 8, &pUtopiaModule);
    UtopiaModuleRegister(pUtopiaModule);
    UtopiaModuleSetupFunctionPtr(pUtopiaModule, (FUtopiaOpen)DIPOpen, (FUtopiaClose)DIPClose, (FUtopiaIOctl)DIPIoctl);
    #ifdef CONFIG_UTOPIA_PROC_DBG_SUPPORT
    UtopiaModuleRegisterMdbNode("dip", (FUtopiaMdbIoctl)DIPMdbIoctl);
    #endif

    // 2. deal with resource
    for(i=DIP_WINDOW ;i<MAX_DIP_WINDOW;i++)
    {
        snprintf(name,sizeof(name),"DIP%d",i);
        UtopiaModuleAddResourceStart(pUtopiaModule,i);
        UtopiaResourceCreate(name, sizeof(DIP_RESOURCE_PRIVATE), &psResource);
        UtopiaResourceRegister(pUtopiaModule, psResource, i);
        UtopiaModuleAddResourceEnd(pUtopiaModule, i);
    }
    UtopiaModuleAddResourceStart(pUtopiaModule,MAX_DIP_WINDOW);
    UtopiaResourceCreate("DIPR", sizeof(DIP_SHARE_RESOURCE_PRIVATE), &psResource);

    UtopiaResourceGetPrivate(psResource,(void*)&(pDipResPri));
    for(i=DIP_WINDOW ;i<MAX_DIP_WINDOW;i++)
    {
        pDipResPri->bIsLoadSRAM[i] = FALSE;
    }

    UtopiaResourceRegister(pUtopiaModule, psResource, MAX_DIP_WINDOW);
    UtopiaModuleAddResourceEnd(pUtopiaModule,MAX_DIP_WINDOW);
}

MS_U32 DIPOpen(void** ppInstance, const void* const pAttribute)
{
    MS_U8 u8index = 0;
    XC_INFO("\n[DIP INFO] dip open");
    DIP_INSTANCE_PRIVATE *pDipPri = NULL;
    UtopiaInstanceCreate(sizeof(DIP_INSTANCE_PRIVATE), ppInstance);
#ifndef CONFIG_UTOPIAXP_REMOVE_IMPL
    UtopiaInstanceGetPrivate(*ppInstance, (void**)&pDipPri);

    pDipPri->bInit = FALSE;
    pDipPri->u8DbgLevel = E_XC_DIP_Debug_Level_LOW;
    pDipPri->eWindow = MAX_DIP_WINDOW;
    for(u8index=0;u8index<MAX_DIP_WINDOW;u8index++)
    {
        pDipPri->pDIPResource[u8index] = NULL;
    }
    pDipPri->pResource = NULL;
    pDipPri->pShareResource = NULL;
#endif
    return UTOPIA_STATUS_SUCCESS;
}

MS_U32 DIPIoctl(void* pInstance, MS_U32 u32Cmd, void* pArgs)
{
#ifndef CONFIG_UTOPIAXP_REMOVE_IMPL
    //printf("\n[DIP INFO] dip ioctl cmd = %ld\n",u32Cmd);
    DIP_RESOURCE* pDipResource = NULL;
    //DIP_INPUT_SOURCE* pInputSource = NULL;
    DIP_SET_SRC_INFO_WINDOW* pSetSrcInfoWindow = NULL;
    DIP_SET_WINDOW* pSetWindow = NULL;
    DIP_SET_WIN_PROPERTY* pSetWinProperty = NULL;
    DIP_SET_PROPERTY* pSetProperty = NULL;
    DIP_BUF_INFO* pBufInfo = NULL;
    DIP_INTER_STUS* pIntrStus = NULL;
    DIP_INTER_CB* pIntrCb = NULL;
    DIP_TRIGGER_MODE* pTrigMode = NULL;
    DIP_SET_DIPR_PROPERTY_EX* pSetDIPRProperty_ex = NULL;
    DIP_CMDQ_SET_ACTION* pDipCmdq = NULL;
    DIP_INSTANCE_PRIVATE* pDipPri = NULL;
    DIP_SHARE_RESOURCE_PRIVATE* pDipResPri = NULL;
    MS_U32 u32Ret = UTOPIA_STATUS_SUCCESS;
    DIP_CONFIG_PIPE* pDIP_Config_Pipe = NULL;
    DIP_SETCONFIG_PARAM* pSetConfig = NULL;
    DIP_SETPOWERSTATE* pSetPowerState = NULL;
    ST_DIP_SC1_SCMI_DATA* pSC1_scmi_data = NULL;
    ST_DIP_SET_SECURE_MODE* pSet_secure_mode = NULL;
    ST_DIP_GET_CHIP_CAPS* pstDIPChipCaps = NULL;
    ST_DIP_CAPS_BUS_SIZE*  pstDIPBusSize = NULL;
    ST_DIP_CAPS_DIPR_WIDTH_MAX* pstDIPRCapsWidthMax = NULL;
    ST_DIP_CAPS_DIPR_WIDTH_MIN* pstDIPRCapsWidthMin = NULL;
    ST_DRV_CAPS_DIPR_WIDTH_MAX stDrvDiprWidthMax = {};
    ST_DIP_CAPS_DIPW_WIDTH_MAX* pstDIPWCapsWidthMax = NULL;
    ST_DIP_CAPS_DI_ENGINE* pstDIEngineSupport = NULL;
    ST_DIP_CAPS_IRQ* pstDIPCapsIrqSupport = NULL;
    ST_DIP_CAPS_VSCALING_MAX_WIDTH* pstDIPCapsVscalingMaxWidth = NULL;
    ST_DIP_GET_XC_STATUS* pstGetXCStatus = NULL;
    MS_BOOL bDIPDrvResourceControl=FALSE;
    MS_BOOL bDIPRHWIsShare=FALSE;
    MS_BOOL bDIPRGetShareStatus=FALSE;
    MS_BOOL bDIPResourceStatus=FALSE;
    MS_BOOL bRet=FALSE;
    ST_DIP_SET_BUFFER_INFO *pstSetBuffInfo = NULL;
    ST_DIP_SET_PROCESS_MODE_INFO *pstSetModeInfo = NULL;
    ST_DIP_SET_XCPATH_INFO *pstXCPathInfo = NULL;
    ST_DIP_HDMI_Info *pstHDMIInfo = NULL;
    #ifndef MSOS_TYPE_OPTEE
    MS_U32 count = 0;
    #endif

    UtopiaInstanceGetPrivate(pInstance, (void*)&pDipPri);

    void* pModule = NULL;
    UtopiaInstanceGetModule(pInstance, &pModule);
    //Keep share memory point
    if(pDipPri->pShareResource == NULL)
    {
        if(UtopiaResourceObtain(pModule,MAX_DIP_WINDOW, &(pDipPri->pShareResource)) != 0)
        {
            XC_ERR("UtopiaResourceObtainToInstant fail\n");
            return UTOPIA_STATUS_FAIL;
        }
        UtopiaResourceRelease(pDipPri->pShareResource);
    }
    switch(u32Cmd)
    {
        case MDrv_CMD_DIP_INIT:
            pDipResource = (DIP_RESOURCE*)pArgs;
            MDrv_XC_DIP_Init(pInstance,pDipResource->eWindow);
            break;
        case MDrv_CMD_DIP_GET_RESOURCE:
            #ifndef MSOS_TYPE_OPTEE
            pDipResource = (DIP_RESOURCE*)pArgs;
            //XC_INFO("MDrv_CMD_DIP_GET_RESOURCE[%d]  start\n",pDipResource->eWindow);
            bDIPDrvResourceControl = MDrv_XC_DIP_DrvResourceControl();
            if(bDIPDrvResourceControl == TRUE)
            {
                bDIPResourceStatus=MDrv_XC_DIP_Resource_Control(pInstance, XC_DIP_GET_RESOURCE, pDipResource->eWindow);
                if(bDIPResourceStatus == FALSE)
                {
                    return UTOPIA_STATUS_FAIL;
                }
            }
            else
            {
                //take to use mutex lock
                if(UtopiaResourceObtain(pModule,MAX_DIP_WINDOW, &(pDipPri->pShareResource)) != 0)
                {
                    XC_ERR("UtopiaResourceObtainToInstant fail\n");
                    return UTOPIA_STATUS_FAIL;
                }
                UtopiaResourceGetPrivate(pDipPri->pShareResource,(void*)&(pDipResPri));

                //obtain check
                if(pDipPri->pResource != NULL)
                {
                    XC_INFO("MDrv_CMD_DIP_GET_RESOURCE[%d]   have already obtained\n",pDipResource->eWindow);
                    //take to use mutex unlock
                    UtopiaResourceRelease(pDipPri->pShareResource);
                    return UTOPIA_STATUS_FAIL;
                }
                while(UtopiaResourceTryObtain(pModule, pDipResource->eWindow, &(pDipPri->pResource)) == UTOPIA_STATUS_NO_RESOURCE)
                {
                    MsOS_DelayTask(1);
                    count++;
                    if(count == DIP_TRY_RESOURCE_TIMES)
                    {
                         //take to use mutex unlock
                         XC_ERR("[%s][%d]Try obtain fail and timeout,pDipResource->eWindow = [%d], thread=[%td]\n",__FUNCTION__,__LINE__,pDipResource->eWindow,(ptrdiff_t)MsOS_GetOSThreadID());
                         UtopiaResourceRelease(pDipPri->pShareResource);
                         return UTOPIA_STATUS_FAIL;
                    }
                }
                pDipResPri->bLock[pDipResource->eWindow] = 1;

                //take to use mutex unlock
                UtopiaResourceRelease(pDipPri->pShareResource);
                //XC_INFO("MDrv_CMD_DIP_GET_RESOURCE[%d]   end\n",pDipResource->eWindow);
            }
            #endif
            break;
        case MDrv_CMD_DIP_RELEASE_RESOURCE:
            pDipResource = (DIP_RESOURCE*)pArgs;
            //XC_INFO("MDrv_CMD_DIP_RELEASE_RESOURCE[%d]   start\n",pDipResource->eWindow);
            //DIPR HW share flow
            bDIPRHWIsShare = MDrv_XC_DIP_DIPR_ShareCheck();
            if(bDIPRHWIsShare == TRUE)
            {
                MDrv_XC_DIP_DIPR_ShareControl(pInstance, XC_DIP_DIPR_RELEASE_RESOURCE, pDipResource->eWindow);
            }
            bDIPDrvResourceControl = MDrv_XC_DIP_DrvResourceControl();
            if(bDIPDrvResourceControl == TRUE)
            {
                bDIPResourceStatus=MDrv_XC_DIP_Resource_Control(pInstance, XC_DIP_RELEASE_RESOURCE, pDipResource->eWindow);
                if(bDIPResourceStatus == FALSE)
                {
                    return UTOPIA_STATUS_FAIL;
                }
            }
            else
            {
                //take to use mutex lock
                if(UtopiaResourceObtain(pModule,MAX_DIP_WINDOW, &(pDipPri->pShareResource)) != 0)
                {
                    XC_ERR("UtopiaResourceObtainToInstant fail\n");
                    return UTOPIA_STATUS_FAIL;
                }
                UtopiaResourceGetPrivate(pDipPri->pShareResource,(void*)&(pDipResPri));

                //release check
                if(pDipPri->pResource == NULL)
                {
                    XC_INFO("MDrv_CMD_DIP_RELEASE_RESOURCE[%d]   have already released\n",pDipResource->eWindow);
                    //take to use mutex unlock
                    UtopiaResourceRelease(pDipPri->pShareResource);
                    return UTOPIA_STATUS_FAIL;
                }
                UtopiaResourceRelease(pDipPri->pResource);
                pDipPri->pResource = NULL;
                pDipResPri->bLock[pDipResource->eWindow] = 0;

                //take to use mutex unlock
                UtopiaResourceRelease(pDipPri->pShareResource);
                //XC_INFO("MDrv_CMD_DIP_RELEASE_RESOURCE[%d]   end\n",pDipResource->eWindow);
            }
            break;
        case MDrv_CMD_DIP_QUERY_RESOURCE:
            pDipResource = (DIP_RESOURCE*)pArgs;
            bDIPDrvResourceControl = MDrv_XC_DIP_DrvResourceControl();
            if(bDIPDrvResourceControl == TRUE)
            {
                UtopiaResourceGetPrivate(pDipPri->pShareResource,(void*)&(pDipResPri));
                if(pDipResPri->bLock[pDipResource->eWindow] == 1)
                {
                    u32Ret = UTOPIA_STATUS_NO_RESOURCE;
                }
            }
            else
            {
                //take to use mutex lock
                if(UtopiaResourceObtain(pModule,MAX_DIP_WINDOW, &(pDipPri->pShareResource)) != 0)
                {
                    XC_ERR("UtopiaResourceObtainToInstant fail\n");
                    return UTOPIA_STATUS_FAIL;
                }
                UtopiaResourceGetPrivate(pDipPri->pShareResource,(void*)&(pDipResPri));

                if(pDipResPri->bLock[pDipResource->eWindow] == 1)
                {
                    u32Ret = UTOPIA_STATUS_NO_RESOURCE;
                }
                //take to use mutex unlock
                UtopiaResourceRelease(pDipPri->pShareResource);
            }
            break;

        case MDrv_CMD_DIP_CMDQ_SETACTION:
            pDipCmdq = (DIP_CMDQ_SET_ACTION*)pArgs;
            MDrv_XC_DIP_CMDQ_SetAction(pInstance, pDipCmdq->eAction, pDipCmdq->eWindow);
            break;

        case MDrv_CMD_DIP_SET_INPUTSOURCE:
            //pInputSource = (DIP_INPUT_SOURCE*)pArgs;

            break;
        case MDrv_CMD_DIP_SET_SRC_INFO:
            pSetSrcInfoWindow = (DIP_SET_SRC_INFO_WINDOW*)pArgs;
            MDrv_XC_DIP_SetSourceInfo(pInstance,&(pSetSrcInfoWindow->stDIPSetSrcInfo), pSetSrcInfoWindow->eWindow);
            break;
        case MDrv_CMD_DIP_SET_WINDOW:
            pSetWindow = (DIP_SET_WINDOW*)pArgs;
            //_XC_SEMAPHORE_ENTRY(pInstance,E_XC_ID_VAR)
            MDrv_XC_DIP_SetWindow(pInstance,&(pSetWindow->WinInfo), pSetWindow->u32DataLen, pSetWindow->eWindow);
            //_XC_SEMAPHORE_RETURN(pInstance,E_XC_ID_VAR);
            break;
        case MDrv_CMD_DIP_SET_WINPROPERTY:
            pSetWinProperty = (DIP_SET_WIN_PROPERTY*)pArgs;
            MDrv_XC_DIP_SetWinProperty(pInstance,&(pSetWinProperty->WinProperty),pSetWinProperty->eWindow);
            break;
        case MDrv_CMD_DIP_GET_BUFINFO:
            pBufInfo = (DIP_BUF_INFO*)pArgs;
            pBufInfo->BufInfo = MDrv_XC_DIP_GetBufferInfo(pInstance,pBufInfo->eWindow);
            break;
        case MDrv_CMD_DIP_INTERRUPT:
            pIntrStus = (DIP_INTER_STUS*)pArgs;
            if(pIntrStus->eFlag == DIP_INTR_ENABLE)
                MDrv_XC_DIP_EnableIntr(pInstance,pIntrStus->IntrStautus,TRUE,pIntrStus->eWindow);
            else if(pIntrStus->eFlag == DIP_INTR_GET)
                pIntrStus->IntrStautus = MDrv_XC_DIP_GetIntrStatus(pInstance,pIntrStus->eWindow);
            else if(pIntrStus->eFlag == DIP_INTR_CLEAR)
                MDrv_XC_DIP_ClearIntr(pInstance,pIntrStus->IntrStautus,pIntrStus->eWindow);
            #ifdef STELLAR
            else if(pIntrStus->eFlag == DIP_INTR_QUERY)
                pIntrStus->IntrStautus = MDrv_XC_DIP_QueryIntr(pInstance,pIntrStus->eWindow);
            #endif
            else if(pIntrStus->eFlag == DIP_INTR_INDEP)
            {
                pIntrStus->IntrStautus = (MS_U16)MDrv_XC_DIP_CheckIntIndependent(pInstance,pIntrStus->eWindow);
            }
            else
                MDrv_XC_DIP_EnableIntr(pInstance,pIntrStus->IntrStautus,FALSE,pIntrStus->eWindow);
            break;
        case MDrv_CMD_DIP_INTERRUPT_CB:
            pIntrCb = (DIP_INTER_CB*)pArgs;
            if(pIntrCb->eFlag == DIP_INTR_ATTACH)
                MDrv_XC_DIP_InterruptAttach(pInstance,pIntrCb->pIntCb,pIntrCb->eWindow);
            else if(pIntrCb->eFlag == DIP_INTR_DETACH)
                MDrv_XC_DIP_InterruptDetach(pInstance,pIntrCb->eWindow);
            break;
        case MDrv_CMD_DIP_TRIGGER:
            pTrigMode = (DIP_TRIGGER_MODE*)pArgs;
            if(pTrigMode->eTrigMode == DIP_TRIG_ON)
            {
                //_XC_SEMAPHORE_ENTRY(pInstance,E_XC_ID_VAR)
                MDrv_XC_DIP_EnableCaptureStream(pInstance,TRUE,pTrigMode->eWindow);
                //_XC_SEMAPHORE_RETURN(pInstance,E_XC_ID_VAR);
            }
            else if(pTrigMode->eTrigMode == DIP_TRIG_ONESHOT)
            {
                bRet = MDrv_XC_DIP_CaptureOneFrame(pInstance,pTrigMode->eWindow);
                if(bRet == FALSE)
                {
                    u32Ret = UTOPIA_STATUS_FAIL;
                }
            }
            else if(pTrigMode->eTrigMode == DIP_TRIG_ONESHOTFAST)
                MDrv_XC_DIP_CaptureOneFrame2(pInstance,pTrigMode->eWindow);
            else if(pTrigMode->eTrigMode == DIP_TRIG_RESET)
                MDrv_XC_DIP_SWReset(pInstance,pTrigMode->eWindow);
            else
            {
                //_XC_SEMAPHORE_ENTRY(pInstance,E_XC_ID_VAR)
                MDrv_XC_DIP_EnableCaptureStream(pInstance,FALSE,pTrigMode->eWindow);
                //_XC_SEMAPHORE_RETURN(pInstance,E_XC_ID_VAR);
            }
            break;
        case MDrv_CMD_DIP_SET_DIPR_PROPERTY:
            pSetDIPRProperty_ex = (DIP_SET_DIPR_PROPERTY_EX*)pArgs;
            //DIPR HW share flow
            bDIPRHWIsShare = MDrv_XC_DIP_DIPR_ShareCheck();
            if(bDIPRHWIsShare == TRUE)
            {
                bDIPRGetShareStatus = MDrv_XC_DIP_DIPR_ShareControl(pInstance, XC_DIP_DIPR_GET_RESOURCE, pSetDIPRProperty_ex->eWindow);
                if(bDIPRGetShareStatus == FALSE)
                {
                    u32Ret = UTOPIA_STATUS_NO_RESOURCE;
                }
            }
            if(pSetDIPRProperty_ex->enDIPRFlag==E_DIP_SET_DIPR_NORMAL)
            {
                MDrv_XC_DIP_SetDIPRProperty(pInstance,&(pSetDIPRProperty_ex->stDIPRProperty_ex.stDIPRProperty),pSetDIPRProperty_ex->eWindow);
            }
            else if(pSetDIPRProperty_ex->enDIPRFlag==E_DIP_SET_DIPR_EX)
            {
                MDrv_XC_DIP_SetDIPRProperty_EX(pInstance,&(pSetDIPRProperty_ex->stDIPRProperty_ex),pSetDIPRProperty_ex->eWindow);
            }
            else
            {
                XC_ERR("MDrv_CMD_DIP_SET_DIPR_PROPERTY Invalid enDIPRFlag \n");
            }
            break;
        case MDrv_CMD_DIP_SET_PROPERTY:
            pSetProperty = (DIP_SET_PROPERTY*)pArgs;

            if (pSetProperty->eFlag& DIP_PRO_SCANMODE)
                MDrv_XC_DIP_SelectSourceScanType(pInstance,(EN_XC_DWIN_SCAN_TYPE)pSetProperty->Property.eScanMode,pSetProperty->eWindow);
            if (pSetProperty->eFlag & DIP_PRO_DATAFMT)
                MDrv_XC_DIP_SetDataFmt(pInstance,(EN_DRV_XC_DWIN_DATA_FMT)pSetProperty->Property.eDataFmt,pSetProperty->eWindow);
            if (pSetProperty->eFlag & DIP_PRO_R2Y)
                MDrv_XC_DWIN_EnableR2YCSC(pInstance,pSetProperty->Property.bR2Y,pSetProperty->eWindow);
            if (pSetProperty->eFlag & DIP_PRO_Y2R)
                MDrv_XC_DIP_SetY2R(pInstance,pSetProperty->Property.bY2R,pSetProperty->eWindow);
            if (pSetProperty->eFlag & DIP_PRO_SWAPUV)
                MDrv_XC_DIP_SetUVSwap(pInstance,pSetProperty->Property.bSwapUV,pSetProperty->eWindow);
            if (pSetProperty->eFlag & DIP_PRO_SWAPYC)
                MDrv_XC_DIP_SetYCSwap(pInstance,pSetProperty->Property.bSwapYC,pSetProperty->eWindow);
            if (pSetProperty->eFlag & DIP_PRO_SWAPRGB)
                MDrv_XC_DIP_SetRGBSwap(pInstance,pSetProperty->Property.stSwapRGB.bSwap,pSetProperty->eWindow);
            if (pSetProperty->eFlag & DIP_PRO_INTERLACE_W)
                MDrv_XC_DIP_SetInterlaceWrite(pInstance,pSetProperty->Property.bInterlaceW,pSetProperty->eWindow);
            if (pSetProperty->eFlag & DIP_PRO_ALPHA)
                MDrv_XC_DIP_SetAlphaValue(pInstance,pSetProperty->Property.u8Alpha,pSetProperty->eWindow);
            if (pSetProperty->eFlag & DIP_PRO_FRC)
                MDrv_XC_DIP_SetFRC(pInstance,pSetProperty->Property.stFrameRateControl.bFrameRateCtrl,pSetProperty->Property.stFrameRateControl.u16In,pSetProperty->Property.stFrameRateControl.u16Out,pSetProperty->eWindow);
            if (pSetProperty->eFlag & DIP_PRO_OP_CAPTURE)
                MDrv_XC_DIP_SetOutputCapture(pInstance,pSetProperty->Property.stOpCapture.bOpCapture,pSetProperty->Property.stOpCapture.eOpCapture,pSetProperty->eWindow);
            if (pSetProperty->eFlag & DIP_PRO_MIRROR)
                MDrv_XC_DIP_SetMirror(pInstance,pSetProperty->Property.stMirror.bHMirror,pSetProperty->Property.stMirror.bVMirror,pSetProperty->eWindow);
            if (pSetProperty->eFlag & DIP_PRO_PINPON)
                MDrv_XC_DIP_SetPinpon(pInstance,pSetProperty->Property.stPinpon.bPinpon,pSetProperty->Property.stPinpon.u32PinponAddr,pSetProperty->eWindow);
            if (pSetProperty->eFlag & DIP_PRO_ROTATION)
                MDrv_XC_DIP_SetRotation(pInstance,pSetProperty->Property.stRotation.bRotation,pSetProperty->Property.stRotation.eRoDirection,pSetProperty->eWindow);
            if (pSetProperty->eFlag & DIP_PRO_HVSP)
            {
                if( MDrv_XC_DIP_SetHVSP(pInstance,pSetProperty->Property.bHVSP,pSetProperty->eWindow) == FALSE)
                    return UTOPIA_STATUS_NO_RESOURCE;
            }
            if (pSetProperty->eFlag & DIP_PRO_TILEBLOCK)
            {
                if( MDrv_XC_DIP_Set420TileBlock(pInstance,pSetProperty->Property.eTileBlock,pSetProperty->eWindow) == FALSE)
                    return UTOPIA_STATUS_NOT_SUPPORTED;
            }
            break;
        case MDrv_CMD_DIP_SET_DBG_LEVEL:
                pDipPri->u8DbgLevel = *(MS_U8 *)(pArgs);
                MDrv_XC_DIP_SetBdgLevel(*(EN_XC_DIP_DEBUG_LEVEL *)(pArgs));
            break;

        case MDrv_CMD_DIP_CONFIG_PIPE:
                pDIP_Config_Pipe = (DIP_CONFIG_PIPE*)pArgs;
                pDIP_Config_Pipe->u32ReturnValue = MDrv_XC_DIP_ConfigPipe_U2(pInstance, pDIP_Config_Pipe->u32PipeID, pDIP_Config_Pipe->u32SecureDMA, pDIP_Config_Pipe->u32OperationMode);
                break;
        case MDrv_CMD_DIP_SET_CONFIG:
                pSetConfig = (DIP_SETCONFIG_PARAM*)pArgs;
                if(pSetConfig->cfg_type == E_XC_DIP_IMI_ENABLE)
                {
                    MDrv_XC_DIP_IMI_ENABLE(pInstance,*(MS_BOOL*)pSetConfig->pCfg,pSetConfig->eWindow);
                }
                else if(pSetConfig->cfg_type == E_XC_DIP_SC1_SCMI_DATA)
                {
                    pSC1_scmi_data = (ST_DIP_SC1_SCMI_DATA*)pSetConfig->pCfg;
                    MDrv_XC_DIP_SC1_SCMI_ENABLE(pInstance,pSC1_scmi_data->bEnable,pSC1_scmi_data->u8SC1FrameCnt,pSetConfig->eWindow);
                }
                else if(pSetConfig->cfg_type == E_XC_DIP_SWITCH_SECURE_MODE)
                {
                    pSet_secure_mode = (ST_DIP_SET_SECURE_MODE*)pSetConfig->pCfg;
                    MDrv_XC_DIP_Set_Secure_Mode(pInstance,pSet_secure_mode->bSecureEnable,pSet_secure_mode->stSecureExtend,pSetConfig->eWindow);
                }
                else if(pSetConfig->cfg_type == E_XC_DIP_SCALING_ADVANCE_MODE)
                {
                    if(MDrv_XC_DIP_SCALING_ADVANCE_MODE(pInstance,*(MS_BOOL*)pSetConfig->pCfg,pSetConfig->eWindow) != E_DIP_SUCCESS)
                    {
                        u32Ret = UTOPIA_STATUS_FAIL;
                    }
                }
                else if(pSetConfig->cfg_type == E_XC_DIP_FRAMERATE_HALF_LR_WRITE)
                {
                    MDrv_XC_DIP_Set_LR_Clip(pInstance, *(MS_BOOL*)pSetConfig->pCfg, pSetConfig->eWindow);
                }
                else
                {
                    XC_ERR("MDrv_CMD_DIP_SET_CONFIG Invalid cfg_type\n");
                }
                break;
#ifdef STELLAR
        case MDrv_CMD_DIP_GET_LATEST_FRAME:
                {
                    DIP_LATEST_FRAME_INFO* pFrameInfo;
                    pFrameInfo = (DIP_LATEST_FRAME_INFO*)pArgs;
                    pFrameInfo->u16Frame = MDrv_XC_DIP_GetLatestFrame(pInstance, pFrameInfo->eWindow);
                }
                break;
#endif
        case MDrv_CMD_DIP_SET_POWERSTATE:
                    pSetPowerState = (DIP_SETPOWERSTATE*)pArgs;
                    MDrv_XC_DIP_Set_PowerState(pInstance,pSetPowerState->enPowerState);
                    break;
        case MDrv_CMD_DIP_GET_CHIPCAPS:
            pstDIPChipCaps = (ST_DIP_GET_CHIP_CAPS*)pArgs;
            // version check
            if((pstDIPChipCaps->u32Version<1)||(pstDIPChipCaps->u32Length<sizeof(ST_DIP_GET_CHIP_CAPS)))
            {
                XC_ERR("%s,%d - invalid Version %td or invalid version Length %td\n",__func__,__LINE__,(ptrdiff_t)(pstDIPChipCaps->u32Version),(ptrdiff_t)(pstDIPChipCaps->u32Length));
                return UTOPIA_STATUS_FAIL;
            }

            if(pstDIPChipCaps->enDIPCaps == E_DIP_CAPS_BUS_SIZE)
            {
                pstDIPBusSize = (ST_DIP_CAPS_BUS_SIZE*)pstDIPChipCaps->pRet;
                MDrv_XC_DIP_GetBusSize(pInstance, &(pstDIPBusSize->u16BusSize), pstDIPChipCaps->eWindow);
            }
            else if(pstDIPChipCaps->enDIPCaps == E_DIP_CAPS_DIPR_MAX_WIDTH)
            {
                pstDIPRCapsWidthMax = (ST_DIP_CAPS_DIPR_WIDTH_MAX*)pstDIPChipCaps->pRet;
                stDrvDiprWidthMax.enDataFmt = pstDIPRCapsWidthMax->enDataFmt;
                stDrvDiprWidthMax.enRdTileFmt = pstDIPRCapsWidthMax->enRdTileFmt;
                stDrvDiprWidthMax.bMfdEna = pstDIPRCapsWidthMax->bMfdEna;
                MDrv_XC_DIP_GetDIPRCapsWidthMax(pInstance, stDrvDiprWidthMax, &(pstDIPRCapsWidthMax->u16Width));
            }
            else if(pstDIPChipCaps->enDIPCaps == E_DIP_CAPS_DIPR_MIN_WIDTH)
            {
                pstDIPRCapsWidthMin = (ST_DIP_CAPS_DIPR_WIDTH_MIN*)pstDIPChipCaps->pRet;
                MDrv_XC_DIP_GetDIPRCapsWidthMin(pInstance, pstDIPRCapsWidthMin->u16Bpp, &(pstDIPRCapsWidthMin->u16Width));
            }
            else if(pstDIPChipCaps->enDIPCaps == E_DIP_CAPS_DIP_IRQ_SUPPORT)
            {
                pstDIPCapsIrqSupport = (ST_DIP_CAPS_IRQ*)pstDIPChipCaps->pRet;
                MDrv_XC_DIP_GetDIPCapsIrqSupport(pInstance, &(pstDIPCapsIrqSupport->bIrqSupport));
            }
            else if(pstDIPChipCaps->enDIPCaps == E_DIP_CAPS_VSCALING_MAX_WIDTH)
            {
                pstDIPCapsVscalingMaxWidth = (ST_DIP_CAPS_VSCALING_MAX_WIDTH*)pstDIPChipCaps->pRet;
                MDrv_XC_DIP_GetVScalingMaxWidth(pInstance, &(pstDIPCapsVscalingMaxWidth->u16MaxWidth), pstDIPChipCaps->eWindow);
            }
            else if(pstDIPChipCaps->enDIPCaps == E_DIP_CAPS_DIPW_MAX_WIDTH)
            {
                pstDIPWCapsWidthMax = (ST_DIP_CAPS_DIPW_WIDTH_MAX*)pstDIPChipCaps->pRet;
                MDrv_XC_DIP_GetDIPWCapsWidthMax(pInstance, pstDIPWCapsWidthMax->enDataFmt, &(pstDIPWCapsWidthMax->u16Width));
            }
            else if(pstDIPChipCaps->enDIPCaps == E_DIP_CAPS_DI_ENGINE)
            {
                pstDIEngineSupport = (ST_DIP_CAPS_DI_ENGINE*)pstDIPChipCaps->pRet;
                MDrv_XC_DIP_GetDIEngineSupport(pInstance, &(pstDIEngineSupport->bDIEngineSupport), pstDIPChipCaps->eWindow);
            }
            else
            {
                XC_ERR("[DIP] MDrv_CMD_DIP_GET_CHIPCAPS Invalid\n");
                u32Ret = UTOPIA_STATUS_FAIL;
            }
            break;
        case MDrv_CMD_DIP_FILL_XC_API_INFO:
            pstGetXCStatus = (ST_DIP_GET_XC_STATUS*)pArgs;
            MDrv_XC_DIP_FillXCApiStatus(pInstance, (ST_DRV_DIP_GET_XC_STATUS*)pstGetXCStatus);
            break;
        case MDrv_CMD_DIP_SET_BUFFER:
            pstSetBuffInfo = (ST_DIP_SET_BUFFER_INFO *)pArgs;
            MDrv_XC_DIP_SetBuffer(pInstance,(ST_DRV_DIP_Buffer *)pstSetBuffInfo);
            break;
        case MDrv_CMD_DIP_SET_PROCESS_MODE:
            pstSetModeInfo = (ST_DIP_SET_PROCESS_MODE_INFO *)pArgs;
            if(pstSetModeInfo->enMode == EN_DIP_MODE_DI)
            {
                MS_BOOL bEnable = *(MS_BOOL *)pstSetModeInfo->pModeSettings;
                MDrv_XC_DIP_SetDIMode(pInstance,bEnable,pstSetModeInfo->eWindow);
            }
            else if(pstSetModeInfo->enMode == EN_DIP_MODE_PDNR)
            {
                MS_BOOL bEnable = *(MS_BOOL *)pstSetModeInfo->pModeSettings;
                MDrv_XC_DIP_SetPDNRMode(pInstance,bEnable,pstSetModeInfo->eWindow);
            }
            else if(pstSetModeInfo->enMode == EN_DIP_MODE_DIPR_BLENDING)
            {
                ST_DIP_DIPR_BLENDING_MODE_SETTINGS stDIPRSettings;
                memset(&stDIPRSettings, 0 ,sizeof(ST_DIP_DIPR_BLENDING_MODE_SETTINGS));
                memcpy(&stDIPRSettings, (ST_DIP_DIPR_BLENDING_MODE_SETTINGS *)pstSetModeInfo->pModeSettings ,sizeof(ST_DIP_DIPR_BLENDING_MODE_SETTINGS));
                MDrv_XC_DIP_SetDIPRBlending(pInstance,(EN_DRV_XC_DIPR_ENGINE)stDIPRSettings.enDIPRSel,(EN_DRV_XC_DIPR_BLENDING_DST)stDIPRSettings.enDIPRDst,stDIPRSettings.bEnable,pstSetModeInfo->eWindow);
            }
            else
            {
                XC_INFO("[DIP] enMode is EN_DIP_MODE_NONE\n");
            }
            break;
        case MDrv_CMD_DIP_SET_XC_PATH_INFO:
            pstXCPathInfo = (ST_DIP_SET_XCPATH_INFO *)pArgs;
            MDrv_XC_DIP_SET_XC_PATH_INFO(pInstance, (ST_DRV_DIP_SET_XCPATH_INFO*)pstXCPathInfo);
            break;
        case MDrv_CMD_DIP_HDMI_INFO:
            pstHDMIInfo = (ST_DIP_HDMI_Info*)pArgs;
            MDrv_XC_DIP_SetHDMIInfo(pInstance,(ST_DRV_DIP_HDMI_INFO *)pstHDMIInfo);
            break;
        default:
            break;
    }
    return u32Ret; // FIXME: error code
#else
    return UTOPIA_STATUS_SUCCESS;
#endif
}

MS_U32 DIPClose(void* pInstance)
{
    XC_INFO("\n[DIP INFO] dip close");
    UtopiaInstanceDelete(pInstance);
    return UTOPIA_STATUS_SUCCESS;
}
