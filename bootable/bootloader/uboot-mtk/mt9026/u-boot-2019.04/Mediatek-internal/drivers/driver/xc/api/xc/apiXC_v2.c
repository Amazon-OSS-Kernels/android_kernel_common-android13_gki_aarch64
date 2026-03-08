
#define  _XC_C_

#include "MsTypes.h"
#ifndef MSOS_TYPE_LINUX_KERNEL
#include <stdio.h>
#include <string.h>
#else
#include <linux/namei.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/ioctl.h>
#include <linux/seq_file.h>
#include <linux/poll.h>
#include <linux/sched.h>
#include <linux/completion.h>
#endif
#include "MsCommon.h"
#include "utopia.h"
#include "utopia_dapi.h"
#include "drvXC_IOPort.h"
#include "apiXC.h"
#include "mvideo_context.h"
#include "apiXC_Adc.h"
#include "apiXC_Auto.h"
#include "apiXC_ModeParse.h"
#include "drv_sc_display.h"
#include "drv_sc_isr.h"
#include "apiXC_PCMonitor.h"
#include "drvXC_HDMI_if.h"
#include "mhal_xc_chip_config.h"
#include "drv_sc_ip.h"
#if (LD_ENABLE==1)
#include "mdrv_ld.h"
#include "mdrv_ldalgo.h"
#endif
#include "mdrv_sc_3d.h"
#include "drv_sc_menuload.h"
#include "drvXC_ADC_Internal.h"
#include "mhal_sc.h"
#if FRC_INSIDE
#include "mdrv_frc.h"
#include "mhal_frc.h"
#endif
#include "XC_private.h"
#include "apiXC_v2.h"
#include "apiXC_Adc_v2.h"
#include "apiXC_Auto_v2.h"
#include "apiXC_ModeParse_v2.h"
#include "apiXC_PCMonitor_v2.h"
#include "drvXC_HDMI_if_v2.h"
#include "xc_hwreg_utility2.h"

#include "utopia_api_database.h"
#include "halCHIP.h"
#include "apiXC_Ace.h"

#define XC_RES_NAME_MAX_LEN  64

// ========== Global definition ==========
void* pModuleXC = NULL;
#ifdef CONFIG_UTOPIA_PROC_DBG_SUPPORT
extern MS_U32 u32SetWindowTime;
//extern MS_U16 QM_InputSourceToIndex(void* pInstance, PQ_WIN eWindow, MS_U8 enInputSourceType);
#endif
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
static MS_BOOL bIsXCRegistered = FALSE;
#endif
#ifdef MSOS_TYPE_LINUX_KERNEL
#define atoi(str) simple_strtoul(((str != NULL) ? str : ""), NULL, 0)
void* pInstanceXCStr = NULL;
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
void* pInstance1XCStr = NULL;
#endif
MS_U32 XCStr(MS_U32 u32PowerState, void* pModule)
{
    MS_U32 u32Return=UTOPIA_STATUS_FAIL;
    stXC_SET_POWERSTATE XCArgs;

#ifndef MSOS_TYPE_OPTEE
    XC_RESOURCE_PRIVATE* pXCResourcePrivate = NULL;
#else
    XC_RESOURCE_PRIVATE_FOR_TEE* pXCResourcePrivate = NULL;
#endif
    XC_INSTANCE_PRIVATE *psXCInstPri = NULL;

    XC_LOG_TRACE(XC_DBGLEVEL_SETWINDOW,"============= XCStr Start  =============\n");

    if (pInstanceXCStr == NULL)
    {
        if(UtopiaOpen(MODULE_XC, &pInstanceXCStr, 0, NULL) != UTOPIA_STATUS_SUCCESS)
        {
            printf("UtopiaOpen XC failed\n");
            UtopiaStrSendCondition("xc",u32PowerState ,0);
            XC_LOG_TRACE(XC_DBGLEVEL_SETWINDOW,"============= XCStr Send Condition=============\n");
            return UTOPIA_STATUS_FAIL;
        }
    }

#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
    if (pInstance1XCStr == NULL)
    {
        XC_INSTANT_ATTRIBUTE stXCInstantAttribute;
        INIT_XC_INSTANT_ATTR(stXCInstantAttribute);
        stXCInstantAttribute.u32DeviceID = E_XC_DEVICE1;
        if(UtopiaOpen(MODULE_XC, &pInstance1XCStr, 0, &stXCInstantAttribute) != UTOPIA_STATUS_SUCCESS)
        {
            printf("UtopiaOpen XC failed\n");
            UtopiaStrSendCondition("xc",u32PowerState ,0);
            XC_LOG_TRACE(XC_DBGLEVEL_SETWINDOW,"============= XCStr Send Condition=============\n");
            return E_APIXC_RET_FAIL;
        }
    }
#endif
    UtopiaInstanceGetPrivate(pInstanceXCStr, (void**)&psXCInstPri);

    UtopiaResourceGetPrivate(g_pXCResource[_XC_SELECT_INTERNAL_VARIABLE(psXCInstPri->u32DeviceID)],(void**)(&pXCResourcePrivate));

    switch(u32PowerState)
    {
        case E_POWER_SUSPEND:
            XC_LOG_TRACE(XC_DBGLEVEL_SETWINDOW,"============= XCStr Suspend Start  =============\n");

            XCArgs.enPowerState = u32PowerState;
            XCArgs.u32ReturnValue = UTOPIA_STATUS_FAIL;

            if(UtopiaIoctl(pInstanceXCStr, E_XC_CMD_SET_POWERSTATE, (void*)&XCArgs) != UTOPIA_STATUS_SUCCESS)
            {
                printf("Obtain XC engine fail\n");
                UtopiaStrSendCondition("xc",u32PowerState ,0);
                XC_LOG_TRACE(XC_DBGLEVEL_SETWINDOW,"============= XCStr Suspend Send Condition=============\n");
                return UTOPIA_STATUS_FAIL;
            }
            u32Return=XCArgs.u32ReturnValue;
            XC_LOG_TRACE(XC_DBGLEVEL_SETWINDOW,"============= XCStr Suspend End  u32Return=%d =============\n",XCArgs.u32ReturnValue);

            break;
        case E_POWER_RESUME:
            XC_LOG_TRACE(XC_DBGLEVEL_SETWINDOW,"============= XCStr Resume Start  =============\n");

            XCArgs.enPowerState = u32PowerState;
            XCArgs.u32ReturnValue = UTOPIA_STATUS_FAIL;

            if(UtopiaIoctl(pInstanceXCStr, E_XC_CMD_SET_POWERSTATE, (void*)&XCArgs) != UTOPIA_STATUS_SUCCESS)
            {
                printf("Obtain XC engine fail\n");
                UtopiaStrSendCondition("xc",u32PowerState ,0);
                XC_LOG_TRACE(XC_DBGLEVEL_SETWINDOW,"============= XCStr Resume Send Condition=============\n");
                return UTOPIA_STATUS_FAIL;
            }
            u32Return=XCArgs.u32ReturnValue;
            XC_LOG_TRACE(XC_DBGLEVEL_SETWINDOW,"============= XCStr Resume End u32Return=%d =============\n",XCArgs.u32ReturnValue);

            break;
        default:
            printf("[%s][%d] PowerState:%td not Implement now!!!\n",__FUNCTION__,__LINE__,(ptrdiff_t)u32PowerState);
            break;
    }
    //delete the pid and instance for str from PID_LIST.
    if(utopia_delete_xc_instance_table(pInstanceXCStr))
    {
        UtopiaInstanceDelete(pInstanceXCStr);
        pInstanceXCStr = NULL;
    }
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
    if(utopia_delete_xc_instance_table(pInstance1XCStr))
    {
        UtopiaInstanceDelete(pInstance1XCStr);
        pInstance1XCStr = NULL;
    }
#endif
#if (XC_STR_DTV_ENABLE_THREAD ==1)
    if((!IsSrcTypeDTV(gSrcInfo[MAIN_WINDOW].enInputSourceType) )||(u32PowerState == E_POWER_SUSPEND))
    {
        UtopiaStrSendCondition("xc",u32PowerState ,0);
        XC_LOG_TRACE(XC_DBGLEVEL_SETWINDOW,"============= XCStr Send Condition=============\n");
    }
#else
    UtopiaStrSendCondition("xc",u32PowerState ,0);
    XC_LOG_TRACE(XC_DBGLEVEL_SETWINDOW,"============= XCStr Send Condition=============\n");

#endif
    XC_LOG_TRACE(XC_DBGLEVEL_SETWINDOW,"============= XCStr End  =============\n");

    return u32Return;
}
#endif

#ifdef CONFIG_UTOPIA_PROC_DBG_SUPPORT
MS_U32 XCMdbIoctl(MS_U32 cmd, const void* const pArgs)
{
    MDBCMD_CMDLINE_PARAMETER *paraCmdLine=NULL;
    MDBCMD_GETINFO_PARAMETER *paraGetInfo=NULL;
    void *pInstance=NULL;

    pInstance = UtopiaModuleGetLocalInstantList(MODULE_XC, pInstance);
    if( pInstance == NULL )
    {
        switch(cmd)
        {
            case MDBCMD_CMDLINE:
                paraCmdLine = (MDBCMD_CMDLINE_PARAMETER *)pArgs;
                paraCmdLine->result = MDBRESULT_SUCCESS_NOT_FIN;
                break;
            case MDBCMD_GETINFO:
                paraGetInfo = (MDBCMD_GETINFO_PARAMETER *)pArgs;
                paraGetInfo->result = MDBRESULT_SUCCESS_NOT_FIN;
                break;
            default:
                break;
        }
        return 0;
    }

    switch(cmd)
    {
        case MDBCMD_CMDLINE:
            paraCmdLine = (MDBCMD_CMDLINE_PARAMETER *)pArgs;
            MDrv_XCMdbIoctl_EchoCmd(paraCmdLine);
            paraCmdLine->result = MDBRESULT_SUCCESS_FIN;
            break;
        case MDBCMD_GETINFO:
            paraGetInfo = (MDBCMD_GETINFO_PARAMETER *)pArgs;
            MDrv_XCMdbIoctl_Cat(paraGetInfo);
            paraGetInfo->result = MDBRESULT_SUCCESS_FIN;
            break;
        default:
            break;
    }
    return 0;
}
#endif


void XCRegisterToUtopia(FUtopiaOpen ModuleType)
{
    MS_U32 u32ResourceStatusCheck[E_XC_POOL_ID_MAX] = {UTOPIA_STATUS_FAIL};

    // 1. deal with module
    UtopiaModuleCreate(MODULE_XC, 0, &pModuleXC);
    UtopiaModuleRegister(pModuleXC);
    // register func for module, after register here, then ap call UtopiaOpen/UtopiaIoctl/UtopiaClose can call to these registered standard func
    UtopiaModuleSetupFunctionPtr(pModuleXC, (FUtopiaOpen)ModuleType, (FUtopiaClose)XCClose, (FUtopiaIOctl)XCIoctl);

#ifdef CONFIG_UTOPIA_PROC_DBG_SUPPORT
    UtopiaModuleRegisterMdbNode("xc", (FUtopiaMdbIoctl)XCMdbIoctl);
#endif

#if defined(MSOS_TYPE_LINUX_KERNEL)
    UtopiaModuleSetupSTRFunctionPtr(pModuleXC, (FUtopiaSTR)XCStr);
    UtopiaModuleSetSTRPrivate(pModuleXC, sizeof(XC_REGS_SAVE_AREA));
#endif
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
#ifndef MSOS_TYPE_LINUX_KERNEL
    if (!bIsXCRegistered)
    {
        g_pDevice0Instance = NULL;
        g_pDevice1Instance = NULL;
        bIsXCRegistered = TRUE;
    }
#endif
#endif
    // 3. resource can allocate private for internal use
    MS_U8 u8DeviceIndex = 0;
    for(u8DeviceIndex = 0; u8DeviceIndex < MAX_XC_DEVICE_NUM ; u8DeviceIndex++)
    {
        MS_U8 u8DeviceOffset = (E_XC_POOL_ID_NUM*u8DeviceIndex);
        void* psResource = NULL;

        UtopiaModuleAddResourceStart(pModuleXC, u8DeviceOffset + E_XC_POOL_ID_INTERNAL_MENULOAD);
        if(u8DeviceIndex == 0)
        {
            u32ResourceStatusCheck[u8DeviceOffset + E_XC_POOL_ID_INTERNAL_MENULOAD] = UtopiaResourceCreate("MENULOAD", sizeof(MLOAD_RESOURCE_PRIVATE), &psResource);
        }
        else
        {
            char u8ResName[XC_RES_NAME_MAX_LEN];
            memset(&u8ResName, 0x0, sizeof(u8ResName));
            snprintf(u8ResName, XC_RES_NAME_MAX_LEN, "MENULOAD_%d", u8DeviceIndex);
            u32ResourceStatusCheck[u8DeviceOffset + E_XC_POOL_ID_INTERNAL_MENULOAD] = UtopiaResourceCreate(u8ResName, sizeof(MLOAD_RESOURCE_PRIVATE), &psResource);
        }
        UtopiaResourceRegister(pModuleXC, psResource, u8DeviceOffset + E_XC_POOL_ID_INTERNAL_MENULOAD);
        UtopiaModuleAddResourceEnd(pModuleXC, u8DeviceOffset + E_XC_POOL_ID_INTERNAL_MENULOAD);

        UtopiaModuleAddResourceStart(pModuleXC, u8DeviceOffset + E_XC_POOL_ID_INTERNAL_REGISTER);
        if(u8DeviceIndex == 0)
        {
            u32ResourceStatusCheck[u8DeviceOffset + E_XC_POOL_ID_INTERNAL_REGISTER] = UtopiaResourceCreate("XC_REGISTER", 0, &psResource);
        }
        else
        {
            char u8ResName[XC_RES_NAME_MAX_LEN];
            memset(&u8ResName, 0x0, sizeof(u8ResName));
            snprintf(u8ResName, XC_RES_NAME_MAX_LEN, "XC_REGISTER_%d", u8DeviceIndex);
            u32ResourceStatusCheck[u8DeviceOffset + E_XC_POOL_ID_INTERNAL_REGISTER] = UtopiaResourceCreate(u8ResName, 0, &psResource);
        }
        UtopiaResourceRegister(pModuleXC, psResource, u8DeviceOffset + E_XC_POOL_ID_INTERNAL_REGISTER);
        UtopiaModuleAddResourceEnd(pModuleXC, u8DeviceOffset + E_XC_POOL_ID_INTERNAL_REGISTER);

        UtopiaModuleAddResourceStart(pModuleXC, _XC_SELECT_INTERNAL_VARIABLE(u8DeviceIndex));
        if(u8DeviceIndex == 0)
        {
#if  defined (MSOS_TYPE_OPTEE)
            u32ResourceStatusCheck[_XC_SELECT_INTERNAL_VARIABLE(u8DeviceIndex)] = UtopiaResourceCreate("XC_VARIABLE", sizeof(XC_RESOURCE_PRIVATE_FOR_TEE), &psResource);
#elif  defined (MSOS_TYPE_NUTTX)
#else
            u32ResourceStatusCheck[_XC_SELECT_INTERNAL_VARIABLE(u8DeviceIndex)] = UtopiaResourceCreate("XC_VARIABLE", sizeof(XC_RESOURCE_PRIVATE), &psResource);
#endif
        }
        else
        {
            char u8ResName[XC_RES_NAME_MAX_LEN];
            memset(&u8ResName, 0x0, sizeof(u8ResName));
            snprintf(u8ResName, XC_RES_NAME_MAX_LEN, "XC_VARIABLE_%d", u8DeviceIndex);
#if  defined (MSOS_TYPE_OPTEE)
            u32ResourceStatusCheck[_XC_SELECT_INTERNAL_VARIABLE(u8DeviceIndex)] = UtopiaResourceCreate(u8ResName, sizeof(XC_RESOURCE_PRIVATE_FOR_TEE), &psResource);
#elif defined (MSOS_TYPE_NUTTX)
#else
            u32ResourceStatusCheck[_XC_SELECT_INTERNAL_VARIABLE(u8DeviceIndex)] = UtopiaResourceCreate(u8ResName, sizeof(XC_RESOURCE_PRIVATE), &psResource);
#endif
        }
        UtopiaResourceRegister(pModuleXC, psResource, _XC_SELECT_INTERNAL_VARIABLE(u8DeviceIndex));
        UtopiaModuleAddResourceEnd(pModuleXC, _XC_SELECT_INTERNAL_VARIABLE(u8DeviceIndex));

        //4. init resource private members here   (aka, global variable)
#if  defined (MSOS_TYPE_OPTEE)
        XC_RESOURCE_PRIVATE_FOR_TEE* pXCResourcePrivate = NULL;
        UtopiaResourceGetPrivate(psResource,(void**)(&pXCResourcePrivate));
        //pXCResourcePrivate->bResourceRegistered will automatically cleared by UtopiaResourceRegister
        if (u32ResourceStatusCheck[_XC_SELECT_INTERNAL_VARIABLE(u8DeviceIndex)] == UTOPIA_STATUS_SHM_EXIST)
        {
            // do nothing, since it is already inited
        }
        else
        {
            memset(&(pXCResourcePrivate->sthal_Optee),0,sizeof(ST_HAL_OPTEE));
        }
#elif  defined (MSOS_TYPE_NUTTX)
#else
        XC_RESOURCE_PRIVATE* pXCResourcePrivate = NULL;
        UtopiaResourceGetPrivate(psResource,(void**)(&pXCResourcePrivate));

        //pXCResourcePrivate->bResourceRegistered will automatically cleared by UtopiaResourceRegister
        if (u32ResourceStatusCheck[_XC_SELECT_INTERNAL_VARIABLE(u8DeviceIndex)] == UTOPIA_STATUS_SHM_EXIST)
        {
            // do nothing, since it is already inited
        }
        else
        {
            // Init flow control related variables here. Other global variable should be
            // inited in each of its init function relatively.
            pXCResourcePrivate->bResourceRegistered = TRUE;
            pXCResourcePrivate->bXCMWEStatus = FALSE;

            // apiXC_Auto.c
            pXCResourcePrivate->stapiXC_Auto.bHaltAutoGeometry = FALSE;
            pXCResourcePrivate->stapiXC_Auto._u8ThresholdValue = 0x05;
            pXCResourcePrivate->stapiXC_Auto.g_enCalbrationMode = E_XC_SW_CALIBRATION;

            // apiXC_PCMonitor.c
            pXCResourcePrivate->stapiXC_PCMonitor._bEnableAutoNoSignalDetect[MAIN_WINDOW] = FALSE;
            pXCResourcePrivate->stapiXC_PCMonitor._bEnableAutoNoSignalDetect[SUB_WINDOW] = FALSE;
            pXCResourcePrivate->stapiXC_PCMonitor.u8DeNoiseCount[MAIN_WINDOW] = 0;
            pXCResourcePrivate->stapiXC_PCMonitor.u8DeNoiseCount[SUB_WINDOW] = 0;
            pXCResourcePrivate->stapiXC_PCMonitor.u8DeStableCnt[MAIN_WINDOW] = 0;
            pXCResourcePrivate->stapiXC_PCMonitor.u8DeStableCnt[SUB_WINDOW] = 0;
            pXCResourcePrivate->stapiXC_PCMonitor._bNewModeInterlacedDetect = FALSE;

            // mdrv_hdmi.c
            pXCResourcePrivate->stdrv_HDMI._ResetDone = FALSE;
            pXCResourcePrivate->stdrv_HDMI._bHDCP_Vsync_done = FALSE;
            pXCResourcePrivate->stdrv_HDMI._bHDCP_Vsync_count = FALSE;
            pXCResourcePrivate->stdrv_HDMI.u32VsyncTimer = 0;
            pXCResourcePrivate->stdrv_HDMI._prev_enPowerState = E_POWER_MECHANICAL;

#if (LD_ENABLE==1)
            // mdrv_ld.c
            pXCResourcePrivate->stdrv_LD._bMDrvLD_InitFinished = FALSE;
            pXCResourcePrivate->stdrv_LD._u16LedPos = 0;

            // mdrv_ldalgo.c
            pXCResourcePrivate->stdrv_LDALGO._u8H_Down_Ratio = 0;
            pXCResourcePrivate->stdrv_LDALGO._u8V_Down_Ratio = 0;
            pXCResourcePrivate->stdrv_LDALGO._u8H_Down_Ratio_3D = 0;
            pXCResourcePrivate->stdrv_LDALGO._u8V_Down_Ratio_3D = 0;
            pXCResourcePrivate->stdrv_LDALGO._u8Pack_Length_LD = 0;
            pXCResourcePrivate->stdrv_LDALGO._u8Pack_Length_PWM = 0;
            pXCResourcePrivate->stdrv_LDALGO._u8Frame_Idx = 0;
            pXCResourcePrivate->stdrv_LDALGO._u8Frame_Idx_Final = 0;
            pXCResourcePrivate->stdrv_LDALGO._u8Frame_Idx_Test = 0;
            pXCResourcePrivate->stdrv_LDALGO._u8Iluma_Pre = 0;
            pXCResourcePrivate->stdrv_LDALGO._u16Histogram_Zero = 0;
            pXCResourcePrivate->stdrv_LDALGO._u32Lluma_Sum = 0;
            pXCResourcePrivate->stdrv_LDALGO._u8DebugArrayDumpDummyCnt = 0;
            pXCResourcePrivate->stdrv_LDALGO._u8Gamma_Control = 0;
            pXCResourcePrivate->stdrv_LDALGO._u8OSDEnable = 1;
            pXCResourcePrivate->stdrv_LDALGO._enLDBlock_Num = LD_BLOCK_DEFALUT;
            pXCResourcePrivate->stdrv_LDALGO._bLDoff = TRUE;
#endif

            // mdrv_sc_3d.c
#if HW_2DTO3D_SUPPORT
            pXCResourcePrivate->stdrvXC_3D.bAdjustDNRSize = FALSE;
            pXCResourcePrivate->stdrvXC_3D.u32OriginalDNRSize = 0;
#endif
            pXCResourcePrivate->stdrvXC_3D.s_u8flg = 0;
            pXCResourcePrivate->stdrvXC_3D.s_u8Ptr = 0;
            pXCResourcePrivate->stdrvXC_3D.u8Ptr = 0;
            pXCResourcePrivate->stdrvXC_3D.bFirstFlag = TRUE;
            pXCResourcePrivate->stdrvXC_3D.ePreInputMode = E_XC_3D_INPUT_MODE_NONE;
            pXCResourcePrivate->stdrvXC_3D.ePreOutputMode = E_XC_3D_OUTPUT_MODE_NONE;

#if HW_2DTO3D_SUPPORT
#if (HW_2DTO3D_VER >= 3) //new 2d to 3d architecture
            pXCResourcePrivate->stdrvXC_3D._s16TBB_BGColorSelWei = 0;
            pXCResourcePrivate->stdrvXC_3D._s16TpBGCOLOR_ValidWei = 0;//instead of TpBGCOLOR_Valid
            pXCResourcePrivate->stdrvXC_3D._s16DnBGCOLOR_ValidWei = 0;//instead of DnBGCOLOR_Valid
            pXCResourcePrivate->stdrvXC_3D._s16DistantViewWei = 0;// for blue sky ratio
            pXCResourcePrivate->stdrvXC_3D._s16DistantViewWei_Bottom = 0;// for blue sky ratio
            pXCResourcePrivate->stdrvXC_3D._s16TBA_BGColorWei = 0;
            pXCResourcePrivate->stdrvXC_3D._s16TBB_BGColorWei = 0;
            pXCResourcePrivate->stdrvXC_3D._s16BothTpBGSKY_COLORCnt = 0;
            pXCResourcePrivate->stdrvXC_3D._s16BothDnBGSKY_COLORCnt = 0;
            pXCResourcePrivate->stdrvXC_3D._s16BlueSkyRatio = 0;

            pXCResourcePrivate->stdrvXC_3D._u16Y_Sep = 0;        // horizontal detection
            pXCResourcePrivate->stdrvXC_3D._u16Y_Sep_OUT = 0;    // horizontal detection for output frame position

            pXCResourcePrivate->stdrvXC_3D._u8Pre_BlueAndFlatBlkCnt = 0;
            pXCResourcePrivate->stdrvXC_3D._u8Pre_BlueAndFlatBlkCnt_Dn = 0;

            //keeps blueSkyRatio stable
            pXCResourcePrivate->stdrvXC_3D._u8Pre_BlueSkyRatio1 = 0;
            pXCResourcePrivate->stdrvXC_3D._u8Pre_BlueSkyRatio2 = 0;

            //for blkbased Depth
            pXCResourcePrivate->stdrvXC_3D._u8Lowpass_En = 1; // 1: 3x3 2: 5x5  3:7x7  4:disable bilinear
            pXCResourcePrivate->stdrvXC_3D._u8Bilateral_En = 0;
            pXCResourcePrivate->stdrvXC_3D._u8DepthAB = 0; //1:TableA 2:TableB 3:ABweight
            pXCResourcePrivate->stdrvXC_3D._u8Bilinear_En = 1;
            pXCResourcePrivate->stdrvXC_3D._s16DistantViewRatio3 = 0;

            //multiview adjust dummy
            pXCResourcePrivate->stdrvXC_3D._s8Shift_Value = 40;
            pXCResourcePrivate->stdrvXC_3D._s8Clamp_Value = 28;
            pXCResourcePrivate->stdrvXC_3D._s8Multi_Gain = 16;
            pXCResourcePrivate->stdrvXC_3D._s32Depth_Control_Var_IIR = 16;
            pXCResourcePrivate->stdrvXC_3D._s32Mean_Depth_IIR = 128;
            pXCResourcePrivate->stdrvXC_3D._u8Horizontal_Step = 32;
            pXCResourcePrivate->stdrvXC_3D._u8Vertical_Step = 32;

            //for IIR
            pXCResourcePrivate->stdrvXC_3D.u8IIR_En = 1;
            pXCResourcePrivate->stdrvXC_3D.u16BL_IIRweight = 96;
            pXCResourcePrivate->stdrvXC_3D.u16PreDistantViewRatio1 = 0;
            pXCResourcePrivate->stdrvXC_3D.u16PreDistantViewRatio2 = 0;
            pXCResourcePrivate->stdrvXC_3D.u16Pre_YSep = 0;

            pXCResourcePrivate->stdrvXC_3D.s_bFirst = TRUE;

            //for DepthDectionBLK
            //pXCResourcePrivate->stdrvXC_3D.u8BLKblueSkyRatio1 = 0;
            //pXCResourcePrivate->stdrvXC_3D.u8BLKblueSkyRatio2 = 0;
            //for Trend Detection
            pXCResourcePrivate->stdrvXC_3D.u8VerGLB_Wei =0;
            //for DEBUGFn
            pXCResourcePrivate->stdrvXC_3D.u8DBG_En = 0;
            pXCResourcePrivate->stdrvXC_3D.u8Round = 0;
            //for job-division of LUT update
            pXCResourcePrivate->stdrvXC_3D.job_id = 0;
            pXCResourcePrivate->stdrvXC_3D.job_freq = 0;
            pXCResourcePrivate->stdrvXC_3D.updateSRAM_state = 1;
            pXCResourcePrivate->stdrvXC_3D.updateSRAM_PreState = 3;
            pXCResourcePrivate->stdrvXC_3D.NoMot_Cnt = 0;
            pXCResourcePrivate->stdrvXC_3D.NoMot_Flg = 0;
            //BGColorDetection
            pXCResourcePrivate->stdrvXC_3D.BGColorBlkCntTp = 0;
            pXCResourcePrivate->stdrvXC_3D.BGColorBlkCntBn = 0;
            pXCResourcePrivate->stdrvXC_3D.BGColor_CbTp = 128;
            pXCResourcePrivate->stdrvXC_3D.BGColor_CrTp = 128;
            pXCResourcePrivate->stdrvXC_3D.BGColor_CbBn = 128;
            pXCResourcePrivate->stdrvXC_3D.BGColor_CrBn = 128;
            pXCResourcePrivate->stdrvXC_3D.BGColorBlkCntforTp = 0;
            pXCResourcePrivate->stdrvXC_3D.BGColorBlkCntforBn = 0;
            pXCResourcePrivate->stdrvXC_3D._u8TBABGColorWei = 0;
            //for MainObj Detection
            pXCResourcePrivate->stdrvXC_3D._u8MainObjWei = 0;
            //for LGE ver GLOBAL depth
            pXCResourcePrivate->stdrvXC_3D._s16BLKblueSkyRatio1 = 0;
            pXCResourcePrivate->stdrvXC_3D._s16BLKblueSkyRatio2 = 0;

            //for TrendDetection
            pXCResourcePrivate->stdrvXC_3D.u8VerGLB_Wei = 0;
            //for DEBUGFn
            pXCResourcePrivate->stdrvXC_3D.u8DBG_En = 0;

            //for Size related settings
            pXCResourcePrivate->stdrvXC_3D._u16PanelWidth = 0;
            pXCResourcePrivate->stdrvXC_3D._u16PanelHeight = 0;
            pXCResourcePrivate->stdrvXC_3D._u8NBLK_DR_H = 0;
            pXCResourcePrivate->stdrvXC_3D._u8NBLK_DR_V = 0;
#endif
#endif

#if HW_2DTO3D_SUPPORT
            pXCResourcePrivate->stdrvXC_3D._u32HisWeight = 0;
            pXCResourcePrivate->stdrvXC_3D._u32ArtWeight = 0;
            pXCResourcePrivate->stdrvXC_3D._u32ReturnWeightCur = 0;
            pXCResourcePrivate->stdrvXC_3D._u32ReturnWeight = 0;
            pXCResourcePrivate->stdrvXC_3D._u32X = 0xe8c9af37;
            pXCResourcePrivate->stdrvXC_3D._u32RemapAvgCurSum = 0;
            pXCResourcePrivate->stdrvXC_3D._u32RemapAvgBseSum = 0;
            pXCResourcePrivate->stdrvXC_3D._u32RemapAvgCur = 0;
            pXCResourcePrivate->stdrvXC_3D._u32RemapAvgBse = 0;
            pXCResourcePrivate->stdrvXC_3D._u8RemapAvgBias = 0;
            pXCResourcePrivate->stdrvXC_3D._u16SBSCount = 0;
            pXCResourcePrivate->stdrvXC_3D._u16TBCount = 0;
            pXCResourcePrivate->stdrvXC_3D._u162DCount = 0;
            pXCResourcePrivate->stdrvXC_3D._u16TotalCount = 0;
#endif

            // mdrv_sc_display.c
            pXCResourcePrivate->stdrvXC_Display._enFpllFsm = FPLL_IDLE_STATE;
            pXCResourcePrivate->stdrvXC_Display._enFpllThreshMode = FPLL_THRESH_MODE_IDLE_STATE;
            pXCResourcePrivate->stdrvXC_Display._bFSM_FPLL_Working = FALSE;
            pXCResourcePrivate->stdrvXC_Display._bFSM_FPLL_Setting = FALSE;
            pXCResourcePrivate->stdrvXC_Display._bFPLL_Thresh_Mode = FALSE;
            pXCResourcePrivate->stdrvXC_Display._bForceFreerun = FALSE;
            pXCResourcePrivate->stdrvXC_Display._ForceVFreq = VFREQ_FROM_PANEL;
            pXCResourcePrivate->stdrvXC_Display._bHSyncChanged = FALSE;
            pXCResourcePrivate->stdrvXC_Display._bVSyncChanged = FALSE;
            pXCResourcePrivate->stdrvXC_Display._bNeedReloadMCNR = FALSE;
            pXCResourcePrivate->stdrvXC_Display._bEnableMCNR[MAIN_WINDOW] = FALSE;
            pXCResourcePrivate->stdrvXC_Display._bEnableMCNR[SUB_WINDOW] = FALSE;
            pXCResourcePrivate->stdrvXC_Display._bForceDisableMCNR = FALSE;
            pXCResourcePrivate->stdrvXC_Display._u32MCNRTimer = 0;
            pXCResourcePrivate->stdrvXC_Display._abSkipOPWOffInSetWindow[MAIN_WINDOW] = FALSE;
            pXCResourcePrivate->stdrvXC_Display._abSkipOPWOffInSetWindow[SUB_WINDOW] = FALSE;
            pXCResourcePrivate->stdrvXC_Display.bFPLL_FSM = FALSE;
            pXCResourcePrivate->stdrvXC_Display._u8CoastClearDebounce=0;
            pXCResourcePrivate->stdrvXC_Display._bDisableCoastDebounce=0;
            pXCResourcePrivate->stdrvXC_Display._bUseCustomFRCTable  = FALSE;
            pXCResourcePrivate->stdrvXC_Display.u32OutputPhaseStable = FALSE;
            pXCResourcePrivate->stdrvXC_Display.u32OutputPRDLock     = FALSE;
            pXCResourcePrivate->stdrvXC_Display._bSkipDisableOPWriteOffInFPLL[MAIN_WINDOW] = FALSE;
            pXCResourcePrivate->stdrvXC_Display._bSkipDisableOPWriteOffInFPLL[SUB_WINDOW] = FALSE;
            pXCResourcePrivate->stdrvXC_Display.s_u32DelayT = 0;
            pXCResourcePrivate->stdrvXC_Display.s_u32InputVsyncStart = 0;
            pXCResourcePrivate->stdrvXC_Display.s_u16OrigPrd = 0xFFFF;         // max PRD error
            pXCResourcePrivate->stdrvXC_Display.s_u16OrigPhase = 0xFFFF;             // max phase error
            pXCResourcePrivate->stdrvXC_Display.s_u32FSMTimer = 0;
            pXCResourcePrivate->stdrvXC_Display.g_u8FrcIn = 0;
            pXCResourcePrivate->stdrvXC_Display.g_u8FrcOut = 0;
            pXCResourcePrivate->stdrvXC_Display._bGameModeStatus = FALSE;
            pXCResourcePrivate->stdrvXC_Display.u16ToFRCDSTrigY = 0;
            // mdrv_sc_dynamicscaling.c
            pXCResourcePrivate->stdrvXC_Dynamicscaling._bDSForceIndexEnable[MAIN_WINDOW] = FALSE;
            pXCResourcePrivate->stdrvXC_Dynamicscaling._bDSForceIndexEnable[SUB_WINDOW] = FALSE;

            // mdrv_sc_ip.c
            pXCResourcePrivate->stdrvXC_IP.u8Count = 1;
#if ENABLE_DEBUG_CAPTURE_N_FRAMES
            pXCResourcePrivate->stdrvXC_IP.bFinishedInputVSyncISR = FALSE;
            pXCResourcePrivate->stdrvXC_IP.u8Cnt = 0;
#endif

            // mdrv_sc_scaling.c
            pXCResourcePrivate->stdrvXC_Scaling._u32DNRBaseAddr0[0] = 0;
            pXCResourcePrivate->stdrvXC_Scaling._u32DNRBaseAddr0[1] = 0;
            pXCResourcePrivate->stdrvXC_Scaling._u32DNRBufSize[0] = 0;
            pXCResourcePrivate->stdrvXC_Scaling._u32DNRBufSize[1] = 0;
            pXCResourcePrivate->stdrvXC_Scaling._u32FRCMBaseAddr0[0] = 0;
            pXCResourcePrivate->stdrvXC_Scaling._u32FRCMBaseAddr0[1] = 0;
            pXCResourcePrivate->stdrvXC_Scaling._u32FRCMBufSize[0] = 0;
            pXCResourcePrivate->stdrvXC_Scaling._u32FRCMBufSize[1] = 0;
            pXCResourcePrivate->stdrvXC_Scaling._bHAutoFitPrescalingEna = FALSE;
            pXCResourcePrivate->stdrvXC_Scaling._bVAutoFitPrescalingEna = FALSE;
            pXCResourcePrivate->stdrvXC_Scaling._bHSizeChangedManually = FALSE;
            pXCResourcePrivate->stdrvXC_Scaling.u32DNRBaseOffset[0] = 0;
            pXCResourcePrivate->stdrvXC_Scaling.u32DNRBaseOffset[1] = 0;
            pXCResourcePrivate->stdrvXC_Scaling.u32FRCMBaseOffset[0] = 0;
            pXCResourcePrivate->stdrvXC_Scaling.u32FRCMBaseOffset[1] = 0;
            pXCResourcePrivate->stdrvXC_Scaling._au8StoreFrameNumFactor[0] = 0;
            pXCResourcePrivate->stdrvXC_Scaling._au8StoreFrameNumFactor[1] = 0;
            pXCResourcePrivate->stdrvXC_Scaling._bMainWinSkipWaitOutputVsync = FALSE;
            pXCResourcePrivate->stdrvXC_Scaling._bSubWinSkipWaitOutputVsync = FALSE;
            pXCResourcePrivate->stdrvXC_Scaling._bPQSetHPreScalingratio = FALSE;
#ifdef ENABLE_SCALING_WO_MUTE
            pXCResourcePrivate->stdrvXC_Scaling._u8OPMFetchRatio[MAIN_WINDOW] = 1;
            pXCResourcePrivate->stdrvXC_Scaling._u8OPMFetchRatio[SUB_WINDOW] = 1;
#endif
            pXCResourcePrivate->stdrvXC_Scaling._au32PreFBAddress[MAIN_WINDOW] = 0;
            pXCResourcePrivate->stdrvXC_Scaling._au32PreFBAddress[SUB_WINDOW] = 0;
            pXCResourcePrivate->stdrvXC_Scaling._au32PreFBSize[MAIN_WINDOW] = 0;
            pXCResourcePrivate->stdrvXC_Scaling._au32PreFBSize[SUB_WINDOW] = 0;
            pXCResourcePrivate->stdrvXC_Scaling._u16HScaleSrc[MAIN_WINDOW] = 0;
            pXCResourcePrivate->stdrvXC_Scaling._u16HScaleSrc[SUB_WINDOW] = 0;
            pXCResourcePrivate->stdrvXC_Scaling._u16HScaleDst[MAIN_WINDOW] = 0;
            pXCResourcePrivate->stdrvXC_Scaling._u16HScaleDst[SUB_WINDOW] = 0;
            pXCResourcePrivate->stdrvXC_Scaling._u16VScaleSrc[MAIN_WINDOW] = 0;
            pXCResourcePrivate->stdrvXC_Scaling._u16VScaleSrc[SUB_WINDOW] = 0;
            pXCResourcePrivate->stdrvXC_Scaling._u16VScaleDst[MAIN_WINDOW] = 0;
            pXCResourcePrivate->stdrvXC_Scaling._u16VScaleDst[SUB_WINDOW] = 0;
            pXCResourcePrivate->stdrvXC_Scaling._u8HSD_Idx = 0;
#ifdef ENABLE_XC_WRITE_CMD_API
            memset(&(pXCResourcePrivate->stXC_DS_CmdCnt), 0, sizeof(XC_DS_CMDCNT));
#endif
            // mvideo.c
            pXCResourcePrivate->stdrvXC_MVideo.bNeedInitShared = TRUE;
            memset(&(pXCResourcePrivate->stdrvXC_MVideo._SContext), 0, sizeof(XC_Context));
            memset(&(pXCResourcePrivate->stdrvXC_MVideo._SCShared), 0, sizeof(XC_Context_Shared));
            pXCResourcePrivate->stdrvXC_MVideo.s_eMemFmt[MAIN_WINDOW] = E_MS_XC_MEM_FMT_AUTO;
            pXCResourcePrivate->stdrvXC_MVideo.s_eMemFmt[SUB_WINDOW] = E_MS_XC_MEM_FMT_AUTO;
            gSrcInfo[MAIN_WINDOW].Status2.stXCConfigCMA[CMA_XC_SELF_MEM].u32HeapID = XC_INVALID_HEAP_ID;
            gSrcInfo[SUB_WINDOW].Status2.stXCConfigCMA[CMA_XC_SELF_MEM].u32HeapID = XC_INVALID_HEAP_ID;
            gSrcInfo[MAIN_WINDOW].Status2.stXCConfigCMA[CMA_XC_COBUFF_MEM].u32HeapID = XC_INVALID_HEAP_ID;
            gSrcInfo[SUB_WINDOW].Status2.stXCConfigCMA[CMA_XC_COBUFF_MEM].u32HeapID = XC_INVALID_HEAP_ID;
            gSrcInfo[MAIN_WINDOW].Status2.stXCConfigCMA[CMA_XC_HDR_DMA_MEM].u32HeapID = XC_INVALID_HEAP_ID;
            gSrcInfo[SUB_WINDOW].Status2.stXCConfigCMA[CMA_XC_HDR_DMA_MEM].u32HeapID = XC_INVALID_HEAP_ID;
            pXCResourcePrivate->stdrvXC_MVideo.s_bKeepPixelPointerAppear = FALSE;
            pXCResourcePrivate->stdrvXC_MVideo.enOldInputSrc_ForScalerSetSource[MAIN_WINDOW] = INPUT_SOURCE_NONE;
            pXCResourcePrivate->stdrvXC_MVideo.enOldInputSrc_ForScalerSetSource[SUB_WINDOW] = INPUT_SOURCE_NONE;
            pXCResourcePrivate->stdrvXC_MVideo._bSkipSWReset = FALSE;   ///< Flag for: If the system needs to show Logo from MBoot layer to APP layer without flashing. \n We bypass SW reset in MApi_XC_Init.
            pXCResourcePrivate->stdrvXC_MVideo._bDFBInit = FALSE;
            pXCResourcePrivate->stdrvXC_MVideo.s8HPixelShift = 0;
            pXCResourcePrivate->stdrvXC_MVideo.s8VPixelShift = 0;
            pXCResourcePrivate->stdrvXC_MVideo.gu16HPixelShiftRange = 0;
            pXCResourcePrivate->stdrvXC_MVideo.gu16VPixelShiftRange = 0;
            pXCResourcePrivate->stdrvXC_MVideo._bSetDualMainWinDone = FALSE;
            pXCResourcePrivate->stdrvXC_MVideo._bSetDualSubWinDone = FALSE;
            pXCResourcePrivate->stdrvXC_MVideo.u32CMATimeDiff = 0;

#ifndef DISABLE_HW_PATTERN_FUNCTION
#ifdef SUPPORT_HW_TESTPATTERN
            pXCResourcePrivate->stdrvXC_MVideo.bIsHWPatternEnabled = FALSE;
            pXCResourcePrivate->stdrvXC_MVideo.bIsMute = FALSE;
            pXCResourcePrivate->stdrvXC_MVideo.bIsInputSrcDisabled = FALSE;
#endif
#endif

#if SUPPORT_OP2_TEST_PATTERN
            pXCResourcePrivate->stdrvXC_MVideo.u16OP2_color_3x3 = 0;
            pXCResourcePrivate->stdrvXC_MVideo.bIsOP2TestPattern= FALSE;
            pXCResourcePrivate->stdrvXC_MVideo.bIsOP2InputSrcDisabled = FALSE;
            pXCResourcePrivate->stdrvXC_MVideo.bIsOP2Mute= FALSE;
#endif
#if FRC_INSIDE
            pXCResourcePrivate->stdrvXC_MVideo.bEnableUserMode = FALSE;
#endif

            pXCResourcePrivate->stdrvXC_MVideo_Context.g_bEnableLockFreqOnly = TRUE;
            pXCResourcePrivate->stdrvXC_MVideo_Context.g_bSwitchMVOPToSCPath = FALSE;

#if (ENABLE_NONSTD_INPUT_MCNR==1)
            pXCResourcePrivate->stdrvXC_MVideo.bIsUnStdSignal = FALSE;
            pXCResourcePrivate->stdrvXC_MVideo.u8StdSignalStbCnt = 0;
            pXCResourcePrivate->stdrvXC_MVideo.u8UnStdOutput[MAIN_WINDOW] = 0;
            pXCResourcePrivate->stdrvXC_MVideo.u8UnStdOutput[SUB_WINDOW] = 1;
#endif

            pXCResourcePrivate->stdrvXC_MVideo.OSDC_FREQ = 600;  // default 60Hz
            pXCResourcePrivate->stdrvXC_MVideo._enPrevPowerState = E_POWER_MECHANICAL;
            pXCResourcePrivate->stdrvXC_MVideo.bSignalStableForStr = FALSE;
            pXCResourcePrivate->stdrvXC_MVideo.bModuleInited = FALSE;
            pXCResourcePrivate->stdrvXC_MVideo.bTimingUnstableForStr = FALSE;
            pXCResourcePrivate->stdrvXC_MVideo.u8DebounceForStr = 0;
            pXCResourcePrivate->stdrvXC_MVideo.abXCNeedPipelineBinding[MAIN_WINDOW] = FALSE;
            pXCResourcePrivate->stdrvXC_MVideo.abXCNeedPipelineBinding[SUB_WINDOW] = FALSE;
            pXCResourcePrivate->stdrvXC_MVideo.abXCNeedSetPolicy[MAIN_WINDOW] = FALSE;
            pXCResourcePrivate->stdrvXC_MVideo.abXCNeedSetPolicy[SUB_WINDOW] = FALSE;
            pXCResourcePrivate->stdrvXC_MVideo.abXCCMANeedAuthorize[MAIN_WINDOW] = FALSE;
            pXCResourcePrivate->stdrvXC_MVideo.abXCCMANeedAuthorize[SUB_WINDOW] = FALSE;

            // mvideo_context.c
            pXCResourcePrivate->stdrvXC_MVideo_Context.s_HDMISYNC_type = HDMI_SYNC_DE;
            pXCResourcePrivate->stdrvXC_MVideo_Context._bSupportHVMode = FALSE;

#if (XC_SUPPORT_IOMMU == TRUE)
            MS_U8 u8WindowId =0;
            for(u8WindowId = MAIN_WINDOW;u8WindowId<MAX_WINDOW;u8WindowId++)
            {
                pXCResourcePrivate->stdrvXC_MVideo.abSecure[u8WindowId] = FALSE;
                pXCResourcePrivate->stdrvXC_MVideo.aphyXCMMABaseAddress[u8WindowId] = XC_INVALID_MEM;
            }
            pXCResourcePrivate->stdrvXC_MVideo.phyHSEMMABaseAddress = XC_INVALID_MEM;
            pXCResourcePrivate->stdrvXC_MVideo.phyHDMIDMAMMABaseAddress = XC_INVALID_MEM;
            pXCResourcePrivate->stdrvXC_MVideo.phyFRCLeftMMABaseAddress = XC_INVALID_MEM;
            pXCResourcePrivate->stdrvXC_MVideo.phyFRCRightMMABaseAddress = XC_INVALID_MEM;
            pXCResourcePrivate->stdrvXC_MVideo.phyFRCPQMMABaseAddress = XC_INVALID_MEM;
            pXCResourcePrivate->stdrvXC_MVideo.u32MMATimeDiff = 0;
#endif
            pXCResourcePrivate->stdrvXC_MVideo.au32pipeID[MAIN_WINDOW] = 0;
            pXCResourcePrivate->stdrvXC_MVideo.au32pipeID[SUB_WINDOW] = 0;
#if (XC_SUPPORT_HDR_DMA_CMA == TRUE)
            pXCResourcePrivate->stdrvXC_MVideo.phyHDMIDMACMABaseAddress  = MSOS_INVALID_ADDR;
            pXCResourcePrivate->stdrvXC_MVideo.u32PreCMAHDMIDMASize = 0;
#endif

#ifndef DISABLE_HW_PATTERN_FUNCTION
#ifdef SUPPORT_HW_TESTPATTERN
            // For HW internal test pattern using
            pXCResourcePrivate->stdrvXC_MVideo_Context.bOP1_PatGen_Init = FALSE;
#endif
#endif

#if FRC_INSIDE
            pXCResourcePrivate->stdrvXC_MVideo_Context.bIsFRCInited = FALSE;
            pXCResourcePrivate->stdrvXC_MVideo_Context.enFRC_CurrentInputTiming = E_XC_FRC_InputTiming_4K2K;
#if (HW_DESIGN_4K2K_VER == 6) // For Manhattan, FSC+FRC
            pXCResourcePrivate->stdrvXC_MVideo_Context.g_bFSCEnabled = FALSE;
            pXCResourcePrivate->stdrvXC_MVideo_Context.g_bFRCEnabled = FALSE;
            pXCResourcePrivate->stdrvXC_MVideo_Context.g_u16GOPOffsetH = 0;
            pXCResourcePrivate->stdrvXC_MVideo_Context.g_u16GOPOffsetV = 0;
            pXCResourcePrivate->stdrvXC_MVideo_Context.g_u16MEMCLevel = MEMC_LEVEL_MID;
            pXCResourcePrivate->stdrvXC_MVideo_Context.g_u16DeJudderLevel = 0;
            pXCResourcePrivate->stdrvXC_MVideo_Context.g_u16DeBlurLevel = 0;
            pXCResourcePrivate->stdrvXC_MVideo_Context.g_u16DE_VStartdelta = 0;
            pXCResourcePrivate->stdrvXC_MVideo_Context.bIsFRCMute = FALSE;
            pXCResourcePrivate->stdrvXC_MVideo_Context.g_bIsAddrUpdated = FALSE;
#endif
            pXCResourcePrivate->sthal_FRC.g_u16MemcLevelStatus = MEMC_LEVEL_MID;
#endif
            pXCResourcePrivate->stdrvXC_MVideo_Context.g_bSpreandEnable = FALSE;
#if (HW_DESIGN_HDMITX_VER == 2)
            pXCResourcePrivate->stdrvXC_MVideo_Context.enHDMITXColorDepth = E_XC_HDMITX_COLORDEPTH_8BIT;
#endif

#ifdef MSOS_TYPE_LINUX_KERNEL
            // xc_hwreg_utility2.h
            pXCResourcePrivate->stXC_Utility._XC_EventFlag = 0;
            pXCResourcePrivate->stXC_Utility._XC_VSyncRun = FALSE;
            pXCResourcePrivate->stXC_Utility._XC_VSyncCount = 0;
            pXCResourcePrivate->stXC_Utility._XC_VSyncMax = 0;
#endif

            // mhal_adc.c
            pXCResourcePrivate->sthal_ADC.bIsYPbPrLooseLPF = FALSE;
            pXCResourcePrivate->sthal_ADC._bSourceSwitched = FALSE;
            pXCResourcePrivate->sthal_ADC._bEnableHWCalibration = FALSE;
            pXCResourcePrivate->sthal_ADC._bUseBandgap = FALSE;
            pXCResourcePrivate->sthal_ADC._eCvbsOutType = ADC_CVBSOUT_DISABLE_1;
            pXCResourcePrivate->sthal_ADC.enLastSourceType = ADC_INPUTSOURCE_UNKNOW;
            pXCResourcePrivate->sthal_ADC.u16LastHtotal = 0;
            pXCResourcePrivate->sthal_ADC.u16LastPixelClk = 0;
            pXCResourcePrivate->sthal_ADC.u16LastSamplingRatio = 1;
#ifdef ADC_AV_CALIBRATION_ONLY_ONCE
            pXCResourcePrivate->sthal_ADC._bIsAVCalibrationDone = FALSE;
#endif
            // mhal_ip.c
            pXCResourcePrivate->sthal_IP._bSetVGASogEn = FALSE;
            pXCResourcePrivate->sthal_IP._u8SetSogDetectStatus = 0;

            // mhal_sc.c
            pXCResourcePrivate->sthal_SC.bDynamicScalingEnable[MAIN_WINDOW] = 0;
            pXCResourcePrivate->sthal_SC.bDynamicScalingEnable[SUB_WINDOW] = 0;
            pXCResourcePrivate->sthal_SC.u16OP1_fclk_gate_en = 0;
            pXCResourcePrivate->sthal_SC.u16OP1_reg_ip2_clk_gate_en = 0;
            pXCResourcePrivate->sthal_SC.u16OP1_ficlk_status = 0;
            pXCResourcePrivate->sthal_SC.u32OP1_reg_pre_down_f2 = 0;
            pXCResourcePrivate->sthal_SC.u32OP1_reg_pre_down_f1 = 0;
            pXCResourcePrivate->sthal_SC.u16OP1_color_3x3 = 0;
            pXCResourcePrivate->sthal_SC.u16OP1_color_vip = 0;
            pXCResourcePrivate->sthal_SC.u16OP1_reg_di1lb_en = 0;
            pXCResourcePrivate->sthal_SC.u16OP1_bypass_miu = 0;
            pXCResourcePrivate->sthal_SC.u16OP1_vop_dualpixel = 0;
            pXCResourcePrivate->sthal_SC.g_u16TmpValue = 0;
            pXCResourcePrivate->sthal_SC.g_bCntFlg = 1;
            pXCResourcePrivate->sthal_SC.bVSDshift[MAIN_WINDOW] = FALSE;
            pXCResourcePrivate->sthal_SC.bVSDshift[SUB_WINDOW] = FALSE;
            pXCResourcePrivate->sthal_SC.su16OldValue = 0;
            pXCResourcePrivate->sthal_SC.su16OldValue_BWD_Status = 0;
            pXCResourcePrivate->sthal_SC.sbOldValueReaded = FALSE;
            // init as hw default value
            pXCResourcePrivate->sthal_SC.u16OldValue[E_STORE_VALUE_AUTO_TUNE_AREA_TRIG] = 0x08;
            pXCResourcePrivate->sthal_SC.u16OldValue[E_STORE_VALUE_DISP_AREA_TRIG] = 0x0A;
            pXCResourcePrivate->sthal_SC.u16NewValue[E_STORE_VALUE_AUTO_TUNE_AREA_TRIG] = 0;
            pXCResourcePrivate->sthal_SC.u16NewValue[E_STORE_VALUE_DISP_AREA_TRIG] = 0;
            pXCResourcePrivate->sthal_SC.bUsingNewValue[E_STORE_VALUE_AUTO_TUNE_AREA_TRIG] = FALSE;
            pXCResourcePrivate->sthal_SC.bUsingNewValue[E_STORE_VALUE_DISP_AREA_TRIG] = FALSE;
#if (HW_DESIGN_4K2K_VER == 4)
            pXCResourcePrivate->sthal_SC.u16OldValue[E_STORE_VALUE_IP_AUTO_TUNE_AREA_TRIG] = 0x03;
            pXCResourcePrivate->sthal_SC.u16OldValue[E_STORE_VALUE_IP_DISP_AREA_TRIG] = 0x05;
#endif
            pXCResourcePrivate->sthal_SC.u16OldValue[E_STORE_VALUE_INIT_CMD_NUM] = 0;

            pXCResourcePrivate->sthal_SC.bTrig_pStored = FALSE;
            pXCResourcePrivate->sthal_SC.u8Time_count = 0;
#if SUPPORT_SEAMLESS_ZAPPING
            pXCResourcePrivate->sthal_SC.u32DynamicScalingBaseAddr = 0;
            pXCResourcePrivate->sthal_SC.u8DynamicScalingIndexDepth = 0;
            pXCResourcePrivate->sthal_SC.bSeamlessZappingEnable[MAIN_WINDOW] = FALSE;
            pXCResourcePrivate->sthal_SC.bSeamlessZappingEnable[SUB_WINDOW] = FALSE;
            pXCResourcePrivate->sthal_SC.u8SeamlessZappingCurIndex[MAIN_WINDOW] = 0;
            pXCResourcePrivate->sthal_SC.u8SeamlessZappingCurIndex[SUB_WINDOW] = 0;
            pXCResourcePrivate->sthal_SC.u8SeamlessZappingNextIndex[MAIN_WINDOW] = 0;
            pXCResourcePrivate->sthal_SC.u8SeamlessZappingNextIndex[SUB_WINDOW] = 0;
#endif

#if (HW_DESIGN_4K2K_VER == 7)
            pXCResourcePrivate->sthal_SC.bSc1HvspFilterIsSet = FALSE;
#endif
            pXCResourcePrivate->u8REEClassNum = 0;
            pXCResourcePrivate->u8XCMbxMsgClass = 0;
            pXCResourcePrivate->u32TotalMailNum = 0;
            pXCResourcePrivate->u32ResidentDataSize = 0;
            pXCResourcePrivate->u32DataSize = 0;
            pXCResourcePrivate->u16IPVsyncTrig = 0;
            memset(&(pXCResourcePrivate->stXC_DLC_Dynamic_Range_Info), 0, sizeof(XC_DYNAMIC_RANGE_INFO));
            memset(&pXCResourcePrivate->stXCQosInputSourceInfo,0, sizeof(ST_XC_QOS_INPUTSOURCE_INFO));
            pXCResourcePrivate->u16HistogramShift = 0;

            pXCResourcePrivate->stdrvXC_MVideo_Context.g_bSetVRROn = TRUE;
            pXCResourcePrivate->stdrvXC_MVideo_Context.g_bEnableCusSTR = FALSE;
        }
#endif
    }
}

#ifndef MSOS_TYPE_NUTTX
void XC_Initialize(XC_INSTANCE_PRIVATE* psXCInstPri, const void* const pAttribute)
{
    psXCInstPri->fpXC_GetLibVer = MApi_XC_GetLibVer_U2;
    psXCInstPri->fpXC_GetInfo = MApi_XC_GetInfo_U2;
    psXCInstPri->fpXC_GetStatus = MApi_XC_GetStatus_U2;
    psXCInstPri->fpXC_GetStatus_nodelay = MApi_XC_GetStatusNodelay_U2;
    psXCInstPri->fpXC_GetStatusEx = MApi_XC_GetStatusEx_U2;
    psXCInstPri->fpXC_SetDbgLevel = MApi_XC_SetDbgLevel_U2;
    psXCInstPri->fpXC_FPLLCustomerMode = MApi_XC_FPLLCustomerMode_U2;
    psXCInstPri->fpXC_SetIOMapBase = MDrv_XC_SetIOMapBase_U2;
    psXCInstPri->fpXC_Init = MApi_XC_Init_U2;
    psXCInstPri->fpXC_GetConfig = MApi_XC_GetConfig_U2;
    psXCInstPri->fpXC_Init_MISC = MApi_XC_Init_MISC_U2;
    psXCInstPri->fpXC_GetMISCStatus = MApi_XC_GetMISCStatus_U2;
    psXCInstPri->fpXC_GetCapability = MApi_XC_GetCapability_U2;
    psXCInstPri->fpXC_GetChipCaps = MApi_XC_GetChipCaps_U2;
    psXCInstPri->fpXC_Exit = MApi_XC_Exit_U2;
    psXCInstPri->fpXC_SetDynamicScaling = MApi_XC_SetDynamicScaling_U2;
    psXCInstPri->fpXC_SetDynamicScalingFlag = MApi_XC_Set_DynamicScalingFlag_U2;
    psXCInstPri->fpXC_Get_DNRBaseOffset = MApi_XC_Get_DNRBaseOffset_U2;
    psXCInstPri->fpXC_Get_FrameNumFactor = MApi_XC_Get_FrameNumFactor_U2;
    psXCInstPri->fpXC_SetWindow = MApi_XC_SetWindow_U2;
    psXCInstPri->fpXC_SetDualWindow = MApi_XC_SetDualWindow_U2;
    psXCInstPri->fpXC_SetTravelingWindow = MApi_XC_SetTravelingWindow_U2;
    psXCInstPri->fpXC_SetInputSource = MApi_XC_SetInputSource_U2;
    psXCInstPri->fpXC_IsYUVSpace =    MApi_XC_IsYUVSpace_U2;
    psXCInstPri->fpXC_IsMemoryFormat422 =    MApi_XC_IsMemoryFormat422_U2;
    psXCInstPri->fpXC_EnableForceRGBin =    MApi_XC_EnableForceRGBin_U2;
    psXCInstPri->fpXC_EnableMirrorModeEx =    MApi_XC_EnableMirrorModeEx_U2;
    psXCInstPri->fpXC_GetMirrorModeTypeEx =    MApi_XC_GetMirrorModeTypeEx_U2;
#ifdef UFO_XC_SCAN_TYPE
    psXCInstPri->fpXC_SetScanType = MApi_XC_SetScanType_U2;
#endif
    psXCInstPri->fpXC_GetSyncStatus =    MApi_XC_GetSyncStatus_U2;
    psXCInstPri->fpXC_ConfigCMA =    MApi_XC_ConfigCMA_U2;
    psXCInstPri->fpXC_WaitOutputVSync =    MApi_XC_WaitOutputVSync_U2;
    psXCInstPri->fpXC_WaitInputVSync =    MApi_XC_WaitInputVSync_U2;
    psXCInstPri->fpXC_SetHdmiSyncMode =    MApi_XC_SetHdmiSyncMode_U2;
    psXCInstPri->fpXC_GetHdmiSyncMode =    MApi_XC_GetHdmiSyncMode_U2;
    psXCInstPri->fpXC_SetRepWindow =    MApi_XC_SetRepWindow_U2;
    psXCInstPri->fpXC_SkipOPWriteOffInSetWindow = MApi_XC_SkipOPWriteOffInSetWindow_U2;
    psXCInstPri->fpXC_GetSkipOPWriteOffInSetWindow = MApi_XC_GetSkipOPWriteOffInSetWindow_U2;
    psXCInstPri->fpXC_SetSkipDisableOPWriteOffInFPLL = MApi_XC_SetSkipDisableOPWriteOffInFPLL_U2;
    psXCInstPri->fpXC_Set_OPWriteOffEnable =    MApi_XC_Set_OPWriteOffEnable_U2;
    psXCInstPri->fpXC_Set_OPWriteOffEnableToReg = MApi_XC_Set_OPWriteOffEnableToReg_U2;
    psXCInstPri->fpXC_ForceSet_OPWriteOffEnable = MApi_XC_ForceSet_OPWriteOffEnable_U2;
    psXCInstPri->fpXC_Get_OPWriteOffEnable = MApi_XC_Get_OPWriteOffEnable_U2;
    psXCInstPri->fpXC_SetDispWinToReg =    MApi_XC_SetDispWinToReg_U2;
    psXCInstPri->fpXC_GetDispWinFromReg =    MApi_XC_GetDispWinFromReg_U2;
    psXCInstPri->fpXC_FreezeImg =    MApi_XC_FreezeImg_U2;
    psXCInstPri->fpXC_IsFreezeImg =    MApi_XC_IsFreezeImg_U2;
    psXCInstPri->fpXC_HSE_Control =    MApi_XC_HSE_Control_U2;
    psXCInstPri->fpXC_Set_Event=    MApi_XC_SetEvent_U2;
    psXCInstPri->fpXC_GenerateBlackVideoForBothWin =    MApi_XC_GenerateBlackVideoForBothWin_U2;
    psXCInstPri->fpXC_Set_BLSK =    MApi_XC_Set_BLSK_U2;
    psXCInstPri->fpXC_GenerateBlackVideo =    MApi_XC_GenerateBlackVideo_U2;
    psXCInstPri->fpXC_IsBlackVideoEnable =    MApi_XC_IsBlackVideoEnable_U2;
    psXCInstPri->fpXC_EnableFrameBufferLess =    MApi_XC_EnableFrameBufferLess_U2;
    psXCInstPri->fpXC_IsCurrentFrameBufferLessMode =    MApi_XC_IsCurrentFrameBufferLessMode_U2;
    psXCInstPri->fpXC_EnableRequest_FrameBufferLess =    MApi_XC_EnableRequest_FrameBufferLess_U2;
    psXCInstPri->fpXC_IsCurrentRequest_FrameBufferLessMode =    MApi_XC_IsCurrentRequest_FrameBufferLessMode_U2;
    psXCInstPri->fpXC_Get_3D_HW_Version =    MApi_XC_Get_3D_HW_Version_U2;
    psXCInstPri->fpXC_Get_3D_IsSupportedHW2DTo3D =    MApi_XC_Get_3D_IsSupportedHW2DTo3D_U2;
    psXCInstPri->fpXC_Set_3D_Mode =    MApi_XC_Set_3D_Mode_U2;
    psXCInstPri->fpXC_Set_3D_MainWin_FirstMode =    MApi_XC_Set_3D_MainWin_FirstMode_U2;
    psXCInstPri->fpXC_Set_3D_LR_Frame_Exchg =    MApi_XC_Set_3D_LR_Frame_Exchg_U2;
    psXCInstPri->fpXC_3D_Is_LR_Frame_Exchged =    MApi_XC_3D_Is_LR_Frame_Exchged_U2;
    psXCInstPri->fpXC_Get_3D_Input_Mode =    MApi_XC_Get_3D_Input_Mode_U2;
    psXCInstPri->fpXC_Get_3D_Output_Mode =    MApi_XC_Get_3D_Output_Mode_U2;
    psXCInstPri->fpXC_Get_3D_Panel_Type =    MApi_XC_Get_3D_Panel_Type_U2;
    psXCInstPri->fpXC_Get_3D_MainWin_First =    MApi_XC_Get_3D_MainWin_First_U2;
    psXCInstPri->fpXC_3DMainSub_IPSync =    MApi_XC_3DMainSub_IPSync_U2;
    psXCInstPri->fpXC_Set_3D_VerVideoOffset =    MApi_XC_Set_3D_VerVideoOffset_U2;
    psXCInstPri->fpXC_Get_3D_VerVideoOffset =    MApi_XC_Get_3D_VerVideoOffset_U2;
    psXCInstPri->fpXC_Is3DFormatSupported =    MApi_XC_Is3DFormatSupported_U2;
    psXCInstPri->fpXC_Set_3D_HShift =    MApi_XC_Set_3D_HShift_U2;
    psXCInstPri->fpXC_Enable_3D_LR_Sbs2Line =    MApi_XC_Enable_3D_LR_Sbs2Line_U2;
    psXCInstPri->fpXC_Get_3D_HShift =    MApi_XC_Get_3D_HShift_U2;
    psXCInstPri->fpXC_Set_3D_HW2DTo3D_Buffer =    MApi_XC_Set_3D_HW2DTo3D_Buffer_U2;
    psXCInstPri->fpXC_Set_3D_HW2DTo3D_Parameters =    MApi_XC_Set_3D_HW2DTo3D_Parameters_U2;
    psXCInstPri->fpXC_Get_3D_HW2DTo3D_Parameters =    MApi_XC_Get_3D_HW2DTo3D_Parameters_U2;
    psXCInstPri->fpXC_Set_3D_Detect3DFormat_Parameters =    MApi_XC_Set_3D_Detect3DFormat_Parameters_U2;
    psXCInstPri->fpXC_Get_3D_Detect3DFormat_Parameters =    MApi_XC_Get_3D_Detect3DFormat_Parameters_U2;
    psXCInstPri->fpXC_Detect3DFormatByContent =    MApi_XC_Detect3DFormatByContent_U2;
    psXCInstPri->fpXC_DetectNL =    MApi_XC_DetectNL_U2;
    psXCInstPri->fpXC_3D_PostPQSetting =    MApi_XC_3D_PostPQSetting_U2;
    psXCInstPri->fpXC_Set_3D_FPInfo =    MApi_XC_Set_3D_FPInfo_U2;
    psXCInstPri->fpXC_EnableAutoDetect3D =    MApi_XC_EnableAutoDetect3D_U2;
    psXCInstPri->fpXC_GetAutoDetect3DFlag =    MApi_XC_GetAutoDetect3DFlag_U2;
    psXCInstPri->fpXC_Set_3D_SubWinClk =    MApi_XC_Set_3D_SubWinClk_U2;
    psXCInstPri->fpXC_3D_Is_LR_Sbs2Line =    MApi_XC_3D_Is_LR_Sbs2Line_U2;
    psXCInstPri->fpSC_3D_Is_Skip_Default_LR_Flag =    MApi_SC_3D_Is_Skip_Default_LR_Flag_U2;
    psXCInstPri->fpXC_3D_Enable_Skip_Default_LR_Flag =    MApi_XC_3D_Enable_Skip_Default_LR_Flag_U2;
    psXCInstPri->fpXC_Mux_Init =    MApi_XC_Mux_Init_U2;
    psXCInstPri->fpXC_Mux_SourceMonitor =    MApi_XC_Mux_SourceMonitor_U2;
    psXCInstPri->fpXC_Mux_CreatePath =    MApi_XC_Mux_CreatePath_U2;
    psXCInstPri->fpXC_Mux_DeletePath =    MApi_XC_Mux_DeletePath_U2;
    psXCInstPri->fpXC_Mux_EnablePath =    MApi_XC_Mux_EnablePath_U2;
    psXCInstPri->fpXC_Mux_TriggerPathSyncEvent =    MApi_XC_Mux_TriggerPathSyncEvent_U2;
    psXCInstPri->fpXC_Mux_TriggerDestOnOffEvent =    MApi_XC_Mux_TriggerDestOnOffEvent_U2;
    psXCInstPri->fpXC_Mux_OnOffPeriodicHandler =    MApi_XC_Mux_OnOffPeriodicHandler_U2;
    psXCInstPri->fpXC_Mux_GetPathInfo =    MApi_XC_Mux_GetPathInfo_U2;
    psXCInstPri->fpXC_Mux_SetSupportMhlPathInfo =    MApi_XC_Mux_SetSupportMhlPathInfo_U2;
    psXCInstPri->fpXC_Mux_SetMhlHotPlugInverseInfo =    MApi_XC_Mux_SetMhlHotPlugInverseInfo_U2;
    psXCInstPri->fpXC_Mux_GetHDMIPort =    MApi_XC_Mux_GetHDMIPort_U2;
    psXCInstPri->fpXC_Mux_GetMappingTab =    MApi_XC_Mux_GetPortMappingMatrix_U2;
    psXCInstPri->fpXC_MUX_MApinputSourceToVDYMuxPORT =    MApi_XC_MUX_MapInputSourceToVDYMuxPORT_U2;
    psXCInstPri->fpXC_Set_NR =    MApi_XC_Set_NR_U2;
    psXCInstPri->fpXC_FilmMode_P =    MApi_XC_FilmMode_P_U2;
    psXCInstPri->fpXC_GetUCEnabled =    MApi_XC_GetUCEnabled_U2;
    psXCInstPri->fpXC_GenSpecificTiming =    MApi_XC_GenSpecificTiming_U2;
    psXCInstPri->fpXC_GetDEBypassMode =    MApi_XC_GetDEBypassMode_U2;
    psXCInstPri->fpXC_GetDEWindow =    MApi_XC_GetDEWindow_U2;
    psXCInstPri->fpXC_GetDEWidthHeightInDEByPassMode =    MApi_XC_GetDEWidthHeightInDEByPassMode_U2;
    psXCInstPri->fpXC_GetCaptureWindow =    MApi_XC_GetCaptureWindow_U2;
    psXCInstPri->fpXC_SetCaptureWindowVstart =    MApi_XC_SetCaptureWindowVstart_U2;
    psXCInstPri->fpXC_SetCaptureWindowHstart =    MApi_XC_SetCaptureWindowHstart_U2;
    psXCInstPri->fpXC_SetCaptureWindowVsize =    MApi_XC_SetCaptureWindowVsize_U2;
    psXCInstPri->fpXC_SetCaptureWindowHsize =    MApi_XC_SetCaptureWindowHsize_U2;
    psXCInstPri->fpXC_SoftwareReset =    MApi_XC_SoftwareReset_U2;
    psXCInstPri->fpXC_CalculateHFreqx10 =    MApi_XC_CalculateHFreqx10_U2;
    psXCInstPri->fpXC_CalculateHFreqx1K =    MApi_XC_CalculateHFreqx1K_U2;
    psXCInstPri->fpXC_CalculateVFreqx10 =    MApi_XC_CalculateVFreqx10_U2;
    psXCInstPri->fpXC_CalculateVFreqx1K =    MApi_XC_CalculateVFreqx1K_U2;
    psXCInstPri->fpXC_GetAccurateVFreqx1K =    MApi_XC_GetAccurateVFreqx1K_U2;
    psXCInstPri->fpXC_InterruptAttach =    MApi_XC_InterruptAttach_U2;
    psXCInstPri->fpXC_InterruptDeAttach =    MApi_XC_InterruptDeAttach_U2;
    psXCInstPri->fpXC_InterruptIsAttached =    MApi_XC_InterruptIsAttached_U2;
    psXCInstPri->fpXC_DisableInputSource =    MApi_XC_DisableInputSource_U2;
    psXCInstPri->fpXC_IsInputSourceDisabled =    MApi_XC_IsInputSourceDisabled_U2;
    psXCInstPri->fpXC_ChangePanelType =    MApi_XC_ChangePanelType_U2;
    psXCInstPri->fpXC_GetCurrentReadBank =    MApi_XC_GetCurrentReadBank_U2;
    psXCInstPri->fpXC_GetCurrentWriteBank =    MApi_XC_GetCurrentWriteBank_U2;
    psXCInstPri->fpXC_SetAutoPreScaling =    MApi_XC_SetAutoPreScaling_U2;
    psXCInstPri->fpXC_GetVSyncWidth =    MApi_XC_GetVSyncWidth_U2;
    psXCInstPri->fpXC_set_GOP_Enable =    MApi_XC_set_GOP_Enable_U2;
    psXCInstPri->fpXC_ip_sel_for_gop =    MApi_XC_ip_sel_for_gop_U2;
    psXCInstPri->fpXC_SetVOPNBL =    MApi_XC_SetVOPNBL_U2;
    psXCInstPri->fpXC_GetDstInfo =    MApi_XC_GetDstInfo_U2;
    psXCInstPri->fpXC_Set_FD_Mask_ByWin = MApi_XC_Set_FD_Mask_ByWin_U2;
    psXCInstPri->fpXC_Get_FD_Mask_ByWin = MApi_XC_Get_FD_Mask_ByWin_U2;
    psXCInstPri->fpXC_Get_FD_Mask_Status = MApi_XC_Get_FD_Mask_Status_U2;
    psXCInstPri->fpXC_SetIP1TestPattern =    MApi_XC_SetIP1TestPattern_U2;
#ifdef UFO_XC_TEST_PATTERN
    psXCInstPri->fpXC_GenerateTestPattern =    MApi_XC_GenerateTestPattern_U2;
#endif
    psXCInstPri->fpXC_InitIPForInternalTiming =    MApi_XC_InitIPForInternalTiming_U2;
    psXCInstPri->fpXC_SetIPMux =    MApi_XC_SetIPMux_U2;
    psXCInstPri->fpXC_Is_H_Sync_Active =    MApi_XC_Is_H_Sync_Active_U2;
    psXCInstPri->fpXC_Is_V_Sync_Active =    MApi_XC_Is_V_Sync_Active_U2;
    psXCInstPri->fpXC_GetAutoPositionWindow =    MApi_XC_GetAutoPositionWindow_U2;
    psXCInstPri->fpXC_SetFrameBufferAddress =    MApi_XC_SetFrameBufferAddress_U2;
    psXCInstPri->fpXC_SetFRCMFrameBufferAddress =    MApi_XC_SetFRCMFrameBufferAddress_U2;
    psXCInstPri->fpXC_IsFrameBufferEnoughForCusScaling =    MApi_XC_IsFrameBufferEnoughForCusScaling_U2;
    psXCInstPri->fpXC_SetScalerMemoryRequest =    MApi_XC_SetScalerMemoryRequest_U2;
    psXCInstPri->fpXC_Get_PixelData =    MApi_XC_Get_PixelData_U2;
    psXCInstPri->fpXC_GetAvailableSize =    MApi_XC_GetAvailableSize_U2;
    psXCInstPri->fpXC_SetFrameColor =    MApi_XC_SetFrameColor_U2;
    psXCInstPri->fpXC_SetDispWindowColor =    MApi_XC_SetDispWindowColor_U2;
    psXCInstPri->fpXC_PollEvent =    MApi_XC_PollEvent_U2;
    psXCInstPri->fpXC_Cancel_PollEvent =    MApi_XC_Cancel_PollEvent_U2;
    psXCInstPri->fpXC_EventTrigger =    MApi_XC_EventTrigger_U2;
    psXCInstPri->fpXC_SupportSourceToVE =    MApi_XC_SupportSourceToVE_U2;
    psXCInstPri->fpXC_SetOutputCapture =    MApi_XC_SetOutputCapture_U2;
    psXCInstPri->fpXC_SetGammaOnOff  =    MApi_XC_SetGammaOnOff_U2;
    psXCInstPri->fpXC_SetPreGammaGain =    MApi_XC_SetPreGammaGain_U2;
    psXCInstPri->fpXC_SetPreGammaOffset =    MApi_XC_SetPreGammaOffset_U2;
    psXCInstPri->fpXC_SetPanelTiming =    MApi_XC_SetPanelTiming_U2;
    psXCInstPri->fpXC_SetOutTimingMode =    MApi_XC_SetOutTimingMode_U2;
    psXCInstPri->fpXC_SetFreeRunTiming =    MApi_XC_SetFreeRunTiming_U2;
    psXCInstPri->fpXC_Set_CustomerSyncInfo =    MApi_XC_Set_CustomerSyncInfo_U2;
    psXCInstPri->fpXC_WaitFPLLDone =    MApi_XC_WaitFPLLDone_U2;
    psXCInstPri->fpXC_GetOutputVFreqX100 =    MApi_XC_GetOutputVFreqX100_U2;
    psXCInstPri->fpXC_GetOP1OutputVFreqX100 =    MApi_XC_GetOP1OutputVFreqX100_U2;
    psXCInstPri->fpXC_FrameLockCheck =    MApi_XC_FrameLockCheck_U2;
    psXCInstPri->fpXC_CustomizeFRCTable =    MApi_XC_CustomizeFRCTable_U2;
    psXCInstPri->fpXC_OutputFrameCtrl =    MApi_XC_OutputFrameCtrl_U2;
    psXCInstPri->fpSC_Enable_FPLL_FSM =    MApi_SC_Enable_FPLL_FSM_U2;
    psXCInstPri->fpSC_ForceFreerun =    MApi_SC_ForceFreerun_U2;
    psXCInstPri->fpSC_IsForceFreerun =    MApi_SC_IsForceFreerun_U2;
    psXCInstPri->fpSC_SetFreerunVFreq =    MApi_SC_SetFreerunVFreq_U2;
    psXCInstPri->fpXC_SetExPanelInfo =    MApi_XC_SetExPanelInfo_U2;
    psXCInstPri->fpXC_Enable_FPLL_Thresh_Mode =    MApi_XC_Enable_FPLL_Thresh_Mode_U2;
    psXCInstPri->fpXC_Get_FPLL_Thresh_Mode = MApi_XC_Get_FPLL_Thresh_Mode_U2;
    psXCInstPri->fpXC_EnableIPAutoNoSignal =    MApi_XC_EnableIPAutoNoSignal_U2;
    psXCInstPri->fpXC_GetIPAutoNoSignal =    MApi_XC_GetIPAutoNoSignal_U2;
    psXCInstPri->fpXC_EnableIPAutoCoast =    MApi_XC_EnableIPAutoCoast_U2;
    psXCInstPri->fpXC_EnableIPCoastDebounce =    MApi_XC_EnableIPCoastDebounce_U2;
    psXCInstPri->fpXC_ClearIPCoastStatus =    MApi_XC_ClearIPCoastStatus_U2;
    psXCInstPri->fpXC_EnableFpllManualSetting =    MApi_XC_EnableFpllManualSetting_U2;
    psXCInstPri->fpXC_FpllBoundaryTest =    MApi_XC_FpllBoundaryTest_U2;
    psXCInstPri->fpXC_SetOffLineDetection  =    MApi_XC_SetOffLineDetection_U2;
    psXCInstPri->fpXC_GetOffLineDetection =    MApi_XC_GetOffLineDetection_U2;
    psXCInstPri->fpXC_SetOffLineSogThreshold =    MApi_XC_SetOffLineSogThreshold_U2;
    psXCInstPri->fpXC_SetOffLineSogBW =    MApi_XC_SetOffLineSogBW_U2;
    psXCInstPri->fpXC_OffLineInit =    MApi_XC_OffLineInit_U2;
    psXCInstPri->fpXC_GetOffLineInfo =    MApi_XC_GetOffLineInfo_U2;
    psXCInstPri->fpXC_Set_Extra_fetch_adv_line =    MApi_XC_Set_Extra_fetch_adv_line_U2;
    psXCInstPri->fpXC_SetVGASogEn =    MApi_XC_SetVGASogEn_U2;
    psXCInstPri->fpXC_EnableWindow =    MApi_XC_EnableWindow_U2;
    psXCInstPri->fpXC_Is_SubWindowEanble =    MApi_XC_Is_SubWindowEanble_U2;
    psXCInstPri->fpXC_SetBorderFormat =    MApi_XC_SetBorderFormat_U2;
    psXCInstPri->fpXC_EnableBorder =    MApi_XC_EnableBorder_U2;
    psXCInstPri->fpXC_ZorderMainWindowFirst =    MApi_XC_ZorderMainWindowFirst_U2;
    psXCInstPri->fpXC_PQ_LoadFunction =    MApi_XC_PQ_LoadFunction_U2;
    psXCInstPri->fpXC_Check_HNonLinearScaling =    MApi_XC_Check_HNonLinearScaling_U2;
    psXCInstPri->fpXC_EnableEuroHdtvSupport =    MApi_XC_EnableEuroHdtvSupport_U2;
    psXCInstPri->fpXC_EnableEuroHdtvDetection =    MApi_XC_EnableEuroHdtvDetection_U2;
    psXCInstPri->fpXC_ReadByte =    MApi_XC_ReadByte_U2;
    psXCInstPri->fpXC_WriteByte =    MApi_XC_WriteByte_U2;
    psXCInstPri->fpXC_WriteByteMask =    MApi_XC_WriteByteMask_U2;
    psXCInstPri->fpXC_Write2ByteMask =    MApi_XC_Write2ByteMask_U2;
    psXCInstPri->fpXC_W2BYTE =    MApi_XC_W2BYTE_U2;
    psXCInstPri->fpXC_R2BYTE =    MApi_XC_R2BYTE_U2;
    psXCInstPri->fpXC_W4BYTE =    MApi_XC_W4BYTE_U2;
    psXCInstPri->fpXC_R4BYTE =    MApi_XC_R4BYTE_U2;
    psXCInstPri->fpXC_R2BYTEMSK =    MApi_XC_R2BYTEMSK_U2;
    psXCInstPri->fpXC_W2BYTEMSK =    MApi_XC_W2BYTEMSK_U2;
    psXCInstPri->fpXC_MLoad_Init =    MApi_XC_MLoad_Init_U2;
    psXCInstPri->fpXC_MLoad_Enable =    MApi_XC_MLoad_Enable_U2;
    psXCInstPri->fpXC_MLoad_Cus_Init =    MApi_XC_MLoad_Cus_Init_U2;
    psXCInstPri->fpXC_MLoad_Cus_Enable =    MApi_XC_MLoad_Cus_Enable_U2;
    psXCInstPri->fpXC_MLoad_GetStatus =    MApi_XC_MLoad_GetStatus_U2;
    psXCInstPri->fpXC_MLoad_WriteCmd_And_Fire =    MApi_XC_MLoad_WriteCmd_And_Fire_U2;
    psXCInstPri->fpXC_MLoad_WriteCmds_And_Fire =    MApi_XC_MLoad_WriteCmds_And_Fire_U2;
    psXCInstPri->fpXC_MLoad_Combine_Enable = MApi_XC_Combine_MLoadEn_U2;
    psXCInstPri->fpXC_MLG_Init =    MApi_XC_MLG_Init_U2;
    psXCInstPri->fpXC_MLG_Enable =    MApi_XC_MLG_Enable_U2;
    psXCInstPri->fpXC_MLG_GetCaps =    MApi_XC_MLG_GetCaps_U2;
    psXCInstPri->fpXC_MLG_GetStatus =    MApi_XC_MLG_GetStatus_U2;
    psXCInstPri->fpXC_SetOSD2VEMode =    MApi_XC_SetOSD2VEMode_U2;
    psXCInstPri->fpXC_IP2_PreFilter_Enable =    MApi_XC_IP2_PreFilter_Enable_U2;
    psXCInstPri->fpXC_Get_Pixel_RGB =    MApi_XC_Get_Pixel_RGB_U2;
    psXCInstPri->fpXC_KeepPixelPointerAppear =    MApi_XC_KeepPixelPointerAppear_U2;
    psXCInstPri->fpXC_Set_MemFmtEx =    MApi_XC_Set_MemFmtEx_U2;
    psXCInstPri->fpXC_IsRequestFrameBufferLessMode =    MApi_XC_IsRequestFrameBufferLessMode_U2;
    psXCInstPri->fpXC_SkipSWReset =    MApi_XC_SkipSWReset_U2;
    psXCInstPri->fpXC_EnableRepWindowForFrameColor =    MApi_XC_EnableRepWindowForFrameColor_U2;
    psXCInstPri->fpXC_SetOSDLayer =    MApi_XC_SetOSDLayer_U2;
    psXCInstPri->fpXC_GetOSDLayer =    MApi_XC_GetOSDLayer_U2;
    psXCInstPri->fpXC_SetVideoAlpha =    MApi_XC_SetVideoAlpha_U2;
    psXCInstPri->fpXC_GetVideoAlpha =    MApi_XC_GetVideoAlpha_U2;
    psXCInstPri->fpXC_SkipWaitVsync =    MApi_XC_SkipWaitVsync_U2;
    psXCInstPri->fpXC_SetCMAHeapID =    MApi_XC_SetCMAHeapID_U2;
    psXCInstPri->fpXC_OP2VOPDESel =    MApi_XC_OP2VOPDESel_U2;
    psXCInstPri->fpXC_FRC_SetWindow =    MApi_XC_FRC_SetWindow_U2;
    psXCInstPri->fpXC_Enable_TwoInitFactor =    MApi_XC_Enable_TwoInitFactor_U2;
    psXCInstPri->fpXC_IsFieldPackingModeSupported =    MApi_XC_IsFieldPackingModeSupported_U2;
    psXCInstPri->fpXC_PreInit =    MApi_XC_PreInit_U2;
    psXCInstPri->fpXC_Get_BufferData =    MApi_XC_Get_BufferData_U2;
    psXCInstPri->fpXC_Set_BufferData =    MApi_XC_Set_BufferData_U2;
    psXCInstPri->fpXC_EnableMainWindow =    MApi_XC_EnableMainWindow_U2;
    psXCInstPri->fpXC_EnableSubWindow =    MApi_XC_EnableSubWindow_U2;
    psXCInstPri->fpXC_DisableSubWindow =    MApi_XC_DisableSubWindow_U2;
    psXCInstPri->fpXC_GetPixelShift =    MApi_XC_GetPixelShift_U2;
    psXCInstPri->fpXC_SetPixelShift =    MApi_XC_SetPixelShift_U2;
    psXCInstPri->fpXC_SetVideoOnOSD = MApi_XC_SetVideoOnOSD_U2;
    psXCInstPri->fpXC_SetOSDLayerBlending = MApi_XC_SetOSDLayerBlending_U2;
    psXCInstPri->fpXC_SetOSDLayerAlpha = MApi_XC_SetOSDLayerAlpha_U2;
    psXCInstPri->fpXC_SetOSDBlendingFormula =    MApi_XC_SetOSDBlendingFormula_U2;
    psXCInstPri->fpXC_ReportPixelInfo =    MApi_XC_ReportPixelInfo_U2;
    psXCInstPri->fpXC_SetScaling =    MApi_XC_SetScaling_U2;
    psXCInstPri->fpXC_SetMCDIBufferAddress =    MApi_XC_SetMCDIBufferAddress_U2;
    psXCInstPri->fpXC_EnableMCDI =    MApi_XC_EnableMCDI_U2;
    psXCInstPri->fpXC_SendCmdToFRC =    MApi_XC_SendCmdToFRC_U2;
    psXCInstPri->fpXC_GetMsgFromFRC =    MApi_XC_GetMsgFromFRC_U2;
    psXCInstPri->fpXC_GetFrcSwVersion =    MApi_XC_GetFrcSwVersion_U2;
    psXCInstPri->fpXC_GetFrcFscCtrlPolicy = MApi_XC_GetFrcFscCtrlPolicy_U2;
    psXCInstPri->fpXC_EnableRWBankAuto =    MApi_XC_EnableRWBankAuto_U2;
    psXCInstPri->fpXC_SetWRBankMappingNum =    MApi_XC_SetWRBankMappingNum_U2;
    psXCInstPri->fpXC_GetWRBankMappingNum =    MApi_XC_GetWRBankMappingNum_U2;
    psXCInstPri->fpXC_GetWRBankMappingNumForZap = MApi_XC_GetWRBankMappingNumForZap_U2;
    psXCInstPri->fpXC_SetBOBMode = MApi_XC_SetBOBMode_U2;
    psXCInstPri->fpXC_SetForceReadBank = MApi_XC_SetForceReadBank_U2;
    psXCInstPri->fpXC_SetLockPoint = MApi_XC_SetLockPoint_U2;
    psXCInstPri->fpXC_LD_Init =    MApi_XC_LD_Init_U2;
    psXCInstPri->fpXC_LD_SetMemoryAddress =    MApi_XC_LD_SetMemoryAddress_U2;
    psXCInstPri->fpXC_LD_Get_Value = MApi_XC_LD_Get_Value_U2;
    psXCInstPri->fpXC_LD_SetLevel =    MApi_XC_LD_SetLevel_U2;
    psXCInstPri->fpSet_TurnoffLDBL =    MApi_Set_TurnoffLDBL_U2;
    psXCInstPri->fpSet_notUpdateSPIDataFlags =    MApi_Set_notUpdateSPIDataFlags_U2;
    psXCInstPri->fpSet_UsermodeLDFlags =    MApi_Set_UsermodeLDFlags_U2;
    psXCInstPri->fpSet_BLLevel =    MApi_Set_BLLevel_U2;
    psXCInstPri->fpXC_Set_BWS_Mode =    MApi_XC_Set_BWS_Mode_U2;
    psXCInstPri->fpXC_FRC_ColorPathCtrl =    MApi_XC_FRC_ColorPathCtrl_U2;
    psXCInstPri->fpXC_FRC_OP2_SetRGBGain =    MApi_XC_FRC_OP2_SetRGBGain_U2;
    psXCInstPri->fpXC_FRC_OP2_SetRGBOffset =    MApi_XC_FRC_OP2_SetRGBOffset_U2;
    psXCInstPri->fpXC_FRC_OP2_SetDither =    MApi_XC_FRC_OP2_SetDither_U2;
    psXCInstPri->fpXC_FRC_BypassMFC =    MApi_XC_FRC_BypassMFC_U2;
    psXCInstPri->fpXC_FRC_Mute =    MApi_XC_FRC_Mute_U2;
    psXCInstPri->fpXC_ForceReadFrame =    MApi_XC_ForceReadFrame_U2;
    psXCInstPri->fpXC_SetCsc =    MApi_XC_SetCsc_U2;
    psXCInstPri->fpXC_RegisterPQSetFPLLThreshMode =    MApi_XC_RegisterPQSetFPLLThreshMode_U2;
    psXCInstPri->fpXC_GetFreeRunStatus =    MApi_XC_GetFreeRunStatus_U2;
    psXCInstPri->fpXC_BYPASS_SetCSC = MApi_XC_BYPASS_SetCSC_U2;
    psXCInstPri->fpXC_Get_DSForceIndexSupported =    MApi_XC_Get_DSForceIndexSupported_U2;
    psXCInstPri->fpXC_Set_DSForceIndex =    MApi_XC_Set_DSForceIndex_U2;
    psXCInstPri->fpXC_Set_DSIndexSourceSelect =    MApi_XC_Set_DSIndexSourceSelect_U2;
    psXCInstPri->fpXC_GetDynamicScalingStatus = MApi_XC_GetDynamicScalingStatus_U2;
    psXCInstPri->fpXC_OSDC_InitSetting =    MApi_XC_OSDC_InitSetting_U2;
    psXCInstPri->fpXC_OSDC_SetOutVfreqx10 = MApi_XC_OSDC_SetOutVfreqx10_U2;
    psXCInstPri->fpXC_OSDC_Control =    MApi_XC_OSDC_Control_U2;
    psXCInstPri->fpXC_OSDC_GetDstInfo =    MApi_XC_OSDC_GetDstInfo_U2;
    psXCInstPri->fpXC_Set_PowerState = MApi_XC_SetPowerState_U2;
    psXCInstPri->fpXC_BYPASS_SetOSDVsyncPos = MApi_XC_BYPASS_SetOSDVsyncPos_U2;
    psXCInstPri->fpXC_BYPASS_SetInputSrc = MApi_XC_BYPASS_SetInputSrc_U2;
    psXCInstPri->fpXC_SetSeamlessZapping = MApi_XC_SetSeamlessZapping_U2;
    psXCInstPri->fpXC_GetSeamlessZappingStatus = MApi_XC_GetSeamlessZappingStatus_U2;
    psXCInstPri->fpXC_Vtrack_SetPayloadData = MApi_XC_Vtrack_SetPayloadData_U2;
    psXCInstPri->fpXC_Vtrack_SetAllPayloadData = MApi_XC_Vtrack_SetAllPayloadData_U2;
    psXCInstPri->fpXC_Vtrack_SetVtrackKey = MApi_XC_Vtrack_SetVtrackKey_U2;
    psXCInstPri->fpXC_Vtrack_SetUserDefindedSetting = MApi_XC_Vtrack_SetUserDefindedSetting_U2;
    psXCInstPri->fpXC_Vtrack_Enable = MApi_XC_Vtrack_Enable_U2;
    psXCInstPri->fpXC_PreSetPQInfo = MApi_XC_PreSetPQInfo_U2;
    psXCInstPri->fpXC_Is_OP1_TestPattern_Enabled = MApi_XC_Is_OP1_TestPattern_Enabled_U2;
    psXCInstPri->fpXC_Set_OP1_TestPattern = MApi_XC_Set_OP1_TestPattern_U2;
    psXCInstPri->fpXC_Set_OP2_TestPattern = MApi_XC_Set_OP2_Pattern_U2;
    psXCInstPri->fpXC_Check_WhiteBalance_Pattern_Mode = MApi_XC_CheckWhiteBalancePatternModeSupport_U2;
    psXCInstPri->fpXC_Set_HLinearScaling = MApi_XC_SetHLinearScaling_U2;
    psXCInstPri->fpXC_EnableT3D = MApi_XC_EnableT3D_U2;
    psXCInstPri->fpXC_Set_FRC_InputTiming = MApi_XC_Set_FRC_InputTiming_U2;
    psXCInstPri->fpXC_Get_FRC_InputTiming = MApi_XC_Get_FRC_InputTiming_U2;
    psXCInstPri->fpXC_Get_VirtualBox_Info = MApi_XC_Get_VirtualBox_Info_U2;
    psXCInstPri->fpXC_Set_OSD_DETECT = MApi_XC_Set_OSD_Detect_U2;
    psXCInstPri->fpXC_Get_OSD_DETECT = MApi_XC_Get_OSD_Detect_U2;
    psXCInstPri->fpXC_Enable_LockFreqOnly = MApi_XC_Enable_LockFreqOnly_U2;
    psXCInstPri->fpXC_Is2K2KToFrcMode = MApi_XC_Is2K2KToFrcMode_U2;
    psXCInstPri->fpXC_SetXCVOPdata = MApi_XC_Set_XC_VOP_U2;
    psXCInstPri->fpXC_GetSWDSIndex =  MApi_XC_GetSWDSIndex_U2;
    psXCInstPri->fpXC_Get_Is_SupportSWDS = MApi_XC_Is_SupportSWDS_U2;
    psXCInstPri->fpXC_Get_FRCM_FrameNum = MApi_XC_Get_FRCM_FrameNum_U2;
    psXCInstPri->fpXC_SetPixelShiftFeatures = MApi_XC_SetPixelShiftFeatures_U2;
    psXCInstPri->fpXC_SetForceWrite =   MApi_XC_SetForceWrite_U2;
    psXCInstPri->fpXC_GetForceWrite =   MApi_XC_GetForceWrite_U2;
#ifdef UFO_XC_PQ_PATH
    psXCInstPri->fpXC_GetPQPathStatus = MApi_XC_GetPQPathStatus_U2;
#endif
#ifdef UFO_XC_AUTO_DOWNLOAD
    psXCInstPri->fpXC_AutoDownloadConfig = MApi_XC_AutoDownload_Config_U2;
    psXCInstPri->fpXC_AutoDownloadWrite = MApi_XC_AutoDownload_Write_U2;
    psXCInstPri->fpXC_AutoDownloadFire = MApi_XC_AutoDownload_Fire_U2;
#endif
#ifdef UFO_XC_HDR
#if (UFO_XC_HDR_VERSION == 2)
    psXCInstPri->fpXC_HDRControl = MApi_XC_HDR_Control_U2;
    psXCInstPri->fpXC_GetOttShmInfo = MApi_XC_GetOttShareMemoryInfo_U2;
#endif
#endif
    psXCInstPri->fpXC_OPTEE_Ctrl = MApi_XC_OPTEE_Control_U2;
    psXCInstPri->fpXC_OPTEE_CheckRBase = MApi_XC_OPTEE_CheckRBase_U2;
    psXCInstPri->fpXC_OPTEE_Mux = MApi_XC_OPTEE_Mux_U2;
    psXCInstPri->fpXC_GetResourceByPipeID = MApi_XC_GetResourceByPipeID_U2;
    psXCInstPri->fpXC_ConfigPipe = MApi_XC_ConfigPipe_U2;
    psXCInstPri->fpXC_CheckPipe = MApi_XC_CheckPipe_U2;
    psXCInstPri->fpXC_SetPQBinPath = MApi_XC_SetPQBinPath_U2;
#ifdef UFO_XC_SET_DSINFO_V0
    psXCInstPri->fpXC_SetDSInfo = MApi_XC_SetDSInfo_U2;
#endif
#ifdef ENABLE_XC_WRITE_CMD_API
    psXCInstPri->fpXC_WriteCmd = MApi_XC_WriteCommand_U2;
    psXCInstPri->fpXC_SWDSFire = MApi_XC_SWDS_Fire_U2;
#endif
    psXCInstPri->fpXC_WriteAndFireReginVsyncBlank = MApi_XC_WriteAndFireReginVsyncBlank_U2;
    psXCInstPri->fpXC_GetDNRBufferSize = MApi_XC_GetDNRBufferSize_U2;
//** OBSELETE FUNCTIONS **//
    psXCInstPri->fpsc_get_output_vfreq =    mvideo_sc_get_output_vfreq_U2;
    psXCInstPri->fpXC_Get_Current_OutputVFreqX100 =    MApi_XC_Get_Current_OutputVFreqX100_U2;
    psXCInstPri->fpXC_SetMemoryWriteRequest =    MApi_XC_SetMemoryWriteRequest_U2;
    psXCInstPri->fpXC_Set_MemFmt =    MApi_XC_Set_MemFmt_U2;
    psXCInstPri->fpXC_SetOutputAdjustSetting =    MApi_XC_SetOutputAdjustSetting_U2;
    psXCInstPri->fsScaler_FPLL_FSM =    msAPI_Scaler_FPLL_FSM_U2;
    psXCInstPri->fpXC_EnableMirrorMode =    MApi_XC_EnableMirrorMode_U2;
    psXCInstPri->fpXC_EnableMirrorMode2 =    MApi_XC_EnableMirrorMode2_U2;
#ifdef SUPPORT_BWD
    psXCInstPri->fpXC_SetBwdConfig = MApi_XC_SetBwdConfig_U2;
#endif
//** OBSELETE FUNCTIONS**//
    psXCInstPri->fpXC_dump_shm = MApi_XC_Dump_SHM_U2;
    psXCInstPri->fpXC_IsSupport2StepScaling = MApi_XC_IsSupport2StepScaling_U2;
    psXCInstPri->fpXC_VideoPlaybackCtrl = MApi_XC_VideoPlaybackCtrl_U2;

#ifdef UFO_XC_SETBLACKVIDEOBYMODE
    psXCInstPri->fpXC_GenerateBlackVideoByMode =    MApi_XC_GenerateBlackVideoByMode_U2;
#endif
    psXCInstPri->fpXC_swds_fire = MApi_SWDS_Fire_U2;

#ifdef UFO_XC_SUPPORT_DUAL_MIU
    psXCInstPri->fpXC_enableDualMode = MApi_XC_EnableMiuDualMode_U2;
#endif

#ifdef UFO_XC_GET_3D_FORMAT
    psXCInstPri->fpXC_Get3DFormat = MApi_XC_Get3DFormat_U2;
#endif

    psXCInstPri->fpXC_SetFBLevel =    MApi_XC_Set_FB_Level_U2;
    psXCInstPri->fpXC_GetFBLevel =    MApi_XC_Get_FB_Level_U2;
    psXCInstPri->fpXC_Set_SWDR_Info = MApi_XC_SetSWDRInfo_U2;
    psXCInstPri->fpXC_Get_SWDR_Info = MApi_XC_GetSWDRInfo_U2;
    psXCInstPri->fpXC_Set_SWDR_Input = MApi_XC_SetSWDRInputData_U2;
#ifdef UFO_XC_SUPPORT_HDMI_DOWNSCALE_OUTPUT_POLICY
    psXCInstPri->fpXC_GetHDMIPolicy = MApi_XC_GetHDMIPolicy_U2;
#endif
    psXCInstPri->fpXC_GetDSInfo = MApi_XC_GetDSInfo_U2;
    psXCInstPri->fpXC_Get_DS_Version = MApi_XC_Get_DS_Version_U2;
    psXCInstPri->fpXC_Set_MemoryAddress = MApi_XC_SetMemoryAddress_U2;
    psXCInstPri->fpXC_ConnectDisplay = MApi_XC_ConnectDisplay_U2;
    psXCInstPri->fpXC_SetFrameInfo = MApi_XC_SetFrameInfo_U2;
    psXCInstPri->fpXC_PQ_Gamma = MApi_XC_PQ_Gamma_U2;
    psXCInstPri->fpXC_PQ_RGBPattern = MApi_XC_PQ_RGBPattern_U2;
    psXCInstPri->fpXC_PQ_BlueStretch = MApi_XC_PQ_BlueStretchIn_U2;
    psXCInstPri->fpXC_GetVideoCaptureInfo = MApi_XC_GetVideoCaptureInfo_U2;
    psXCInstPri->fpXC_PQ_SetHNonlinearScaling = MApi_XC_H_NonLinearScaling_U2;
    psXCInstPri->fpXC_MemoryEnable = MApi_XC_Memory_Enable_U2;
    psXCInstPri->fpXC_GetVideoLatency = MApi_XC_GetVideoLatency_U2;
    psXCInstPri->fpXC_GetIPSTATUS =  MApi_XC_GetIPStatus_U2;
    psXCInstPri->fpXC_TestFunction = MApi_XC_TestFunction_U2;
    psXCInstPri->fpXC_QOS_SetScalerBWInfo = MApi_XC_SetScalerBWLevel_U2;
    psXCInstPri->fpXC_QOS_SetFRCBWInfo = MApi_XC_SetFRCBWLevel_U2;
    psXCInstPri->fpXC_QOS_CallBackInfo = MApi_XC_QOS_CallBackFun_U2;
    psXCInstPri->fpXC_SetLowLatencyMode = MApi_XC_SetLowLatencyMode_U2;
    psXCInstPri->fpXC_Set_DSForceIndexEXT = MApi_XC_Set_DSForceIndex_EXT_U2;
    psXCInstPri->fpXC_SetVRROn = MApi_XC_SetVRROn_U2;
    psXCInstPri->fpXC_CusSTR_Enable = MApi_XC_Enable_CusSTR_U2;
    psXCInstPri->fpXC_Get_CusSTRStatus = MApi_XC_Get_CusSTRStatus_U2;
    psXCInstPri->fpXC_SetRWPointDiff = MApi_XC_Set_RWPointDiff_U2;
    psXCInstPri->fpXC_SrcSig_CTL = MApi_XC_SourceSignal_Control_U2;
}

MS_U32 XCOpenFull(void** ppInstance, const void* const pAttribute)
{
    XC_INSTANCE_PRIVATE *psXCInstPri = NULL;
    XC_INSTANT_ATTRIBUTE *pstXCInstAttr = (XC_INSTANT_ATTRIBUTE*)pAttribute;
#ifdef MSOS_TYPE_LINUX_KERNEL
    XC_INSTANT_ATTRIBUTE tmppstXCInstAttr;
#endif
    if (UtopiaInstanceCreate(sizeof(XC_INSTANCE_PRIVATE), ppInstance) != UTOPIA_STATUS_SUCCESS)
    {
        printf("[%s,%5d] Create Instance Failed\n",__FUNCTION__,__LINE__);
        return UTOPIA_STATUS_FAIL;
    }

    UtopiaInstanceGetPrivate(*ppInstance, (void**)&psXCInstPri);
    psXCInstPri->u32DeviceID = 0;
#ifdef MSOS_TYPE_LINUX_KERNEL
    if(pAttribute != NULL)
    {
        if(FALSE == MsOS_KernelMemoryRegionCheck(pAttribute))
        {
            if(0 == copy_from_user(&tmppstXCInstAttr, pstXCInstAttr,sizeof(tmppstXCInstAttr)))
            {
                psXCInstPri->u32DeviceID = tmppstXCInstAttr.u32DeviceID;
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
                pstXCInstAttr = &tmppstXCInstAttr;
#endif
            }
            else
            {
                return UTOPIA_STATUS_FAIL;
            }
        }
        else
        {
            psXCInstPri->u32DeviceID = pstXCInstAttr->u32DeviceID;
        }
    }
#else
    if(pAttribute != NULL)
    {
        psXCInstPri->u32DeviceID = pstXCInstAttr->u32DeviceID;
    }
#endif
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
    if((pstXCInstAttr == NULL) // Default: eXCType = E_XC_INSTANT_TYPE_XC
     ||(pstXCInstAttr->eXCType == E_XC_INSTANT_TYPE_XC)
    )
    {
#ifdef MSOS_TYPE_LINUX_KERNEL
         utopia_create_xc_instance_table(*ppInstance, psXCInstPri->u32DeviceID);
#else
        if (psXCInstPri->u32DeviceID == 1)
        {
            g_pDevice1Instance = *ppInstance;
        }
        else
        {
            g_pDevice0Instance = *ppInstance;
        }
#endif
    }
#endif

#ifdef CONFIG_UTOPIA_SIZE_CUSTOMER
    XC_U1_U2_RELATION(ppInstance,pAttribute);
#else
    // init Instance private members here
    XC_Initialize(psXCInstPri, pAttribute);
    ADC_Initialize(psXCInstPri, pAttribute);
    AUTO_Initialize(psXCInstPri, pAttribute);

    MODEPARSE_Initialize(psXCInstPri, pAttribute);
    PCMONITOR_Initialize(psXCInstPri, pAttribute);
    HDMI_IF_Initialize(psXCInstPri, pAttribute);
#endif

#ifdef UFO_XC_VMARK
    VMARK_Initialize(psXCInstPri, pAttribute);
#endif
    return UTOPIA_STATUS_SUCCESS;
}


// this is for those who has code size problem and needs the minimum set of functions.
MS_U32 XCOpenMinimum(void** ppInstance, const void* const pAttribute)
{
    XC_INSTANCE_PRIVATE *psXCInstPri = NULL;

    if (UtopiaInstanceCreate(sizeof(XC_INSTANCE_PRIVATE), ppInstance) != UTOPIA_STATUS_SUCCESS)
    {
        printf("[%s,%5d] Create Instance Failed\n",__FUNCTION__,__LINE__);
        return UTOPIA_STATUS_FAIL;
    }

    UtopiaInstanceGetPrivate(*ppInstance, (void**)&psXCInstPri);

    if(pAttribute != NULL)
    {
        MS_U32* u32Attr = (MS_U32*)pAttribute;
        psXCInstPri->u32DeviceID = *u32Attr;
    }
    else
    {
        psXCInstPri->u32DeviceID = 0;
    }
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
#ifdef MSOS_TYPE_LINUX_KERNEL
     utopia_create_xc_instance_table(*ppInstance, psXCInstPri->u32DeviceID);
#else
    if(psXCInstPri->u32DeviceID == 1)
    {
        g_pDevice1Instance = *ppInstance;
    }
    else
    {
        g_pDevice0Instance = *ppInstance;
    }
#endif
#endif
    psXCInstPri->fpXC_GetCapability = MApi_XC_GetCapability_U2;
    psXCInstPri->fpXC_PreInit =    MApi_XC_PreInit_U2;
    psXCInstPri->fpXC_Init_MISC = MApi_XC_Init_MISC_U2;
    psXCInstPri->fpXC_Init = MApi_XC_Init_U2;
    psXCInstPri->fpXC_SetExPanelInfo =    MApi_XC_SetExPanelInfo_U2;
    psXCInstPri->fpXC_FRC_Mute = MApi_XC_FRC_Mute_U2;
    psXCInstPri->fpXC_ADC_SetCVBSOut =  MApi_XC_ADC_SetCVBSOut_U2;
    psXCInstPri->fpXC_SetOutputCapture =    MApi_XC_SetOutputCapture_U2;
    psXCInstPri->fpXC_SetDispWindowColor =    MApi_XC_SetDispWindowColor_U2;
    psXCInstPri->fpXC_OSDC_InitSetting =    MApi_XC_OSDC_InitSetting_U2;
    psXCInstPri->fpXC_OSDC_SetOutVfreqx10 = MApi_XC_OSDC_SetOutVfreqx10_U2;
    psXCInstPri->fpXC_OSDC_Control =    MApi_XC_OSDC_Control_U2;
    psXCInstPri->fpXC_SetFrameColor =    MApi_XC_SetFrameColor_U2;
    psXCInstPri->fpXC_WriteByteMask =    MApi_XC_WriteByteMask_U2;
    psXCInstPri->fpXC_Write2ByteMask =    MApi_XC_Write2ByteMask_U2;
    psXCInstPri->fpXC_W2BYTE =    MApi_XC_W2BYTE_U2;
    psXCInstPri->fpXC_W4BYTE =    MApi_XC_W4BYTE_U2;
    psXCInstPri->fpXC_W2BYTEMSK =    MApi_XC_W2BYTEMSK_U2;
    psXCInstPri->fpXC_MLG_GetStatus =    MApi_XC_MLG_GetStatus_U2;
    psXCInstPri->fpXC_SkipSWReset =    MApi_XC_SkipSWReset_U2;
    psXCInstPri->fpXC_GenerateBlackVideo =    MApi_XC_GenerateBlackVideo_U2;
    psXCInstPri->fpXC_Mux_Init =    MApi_XC_Mux_Init_U2;
    psXCInstPri->fpXC_Mux_CreatePath =    MApi_XC_Mux_CreatePath_U2;
    psXCInstPri->fpXC_Mux_EnablePath =    MApi_XC_Mux_EnablePath_U2;
    psXCInstPri->fpXC_SetInputSource = MApi_XC_SetInputSource_U2;
    psXCInstPri->fpXC_DisableInputSource =    MApi_XC_DisableInputSource_U2;
    psXCInstPri->fpXC_SkipWaitVsync =    MApi_XC_SkipWaitVsync_U2;
    psXCInstPri->fpXC_SetWindow = MApi_XC_SetWindow_U2;
    psXCInstPri->fpXC_GetMISCStatus = MApi_XC_GetMISCStatus_U2;
    psXCInstPri->fpXC_SetOSD2VEMode =    MApi_XC_SetOSD2VEMode_U2;
    psXCInstPri->fpXC_AutoDownloadConfig = MApi_XC_AutoDownload_Config_U2;
    psXCInstPri->fpXC_AutoDownloadWrite = MApi_XC_AutoDownload_Write_U2;
    psXCInstPri->fpXC_AutoDownloadFire = MApi_XC_AutoDownload_Fire_U2;
    psXCInstPri->fpXC_GetMirrorModeTypeEx = MApi_XC_GetMirrorModeTypeEx_U2;
    psXCInstPri->fpXC_GetStatus = MApi_XC_GetStatus_U2;

    return UTOPIA_STATUS_SUCCESS;
}

MS_U32 XCOpenForTEE(void** ppInstance, const void* const pAttribute)
{
    XC_INSTANCE_PRIVATE *psXCInstPri = NULL;

    if (UtopiaInstanceCreate(sizeof(XC_INSTANCE_PRIVATE), ppInstance) != UTOPIA_STATUS_SUCCESS)
    {
        printf("[%s,%5d] Create Instance Failed\n",__FUNCTION__,__LINE__);
        return UTOPIA_STATUS_FAIL;
    }

    UtopiaInstanceGetPrivate(*ppInstance, (void**)&psXCInstPri);

    if(pAttribute != NULL)
    {
        MS_U32* u32Attr = (MS_U32*)pAttribute;
        psXCInstPri->u32DeviceID = *u32Attr;
    }
    else
    {
        psXCInstPri->u32DeviceID = 0;
    }
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
#ifdef MSOS_TYPE_LINUX_KERNEL
     utopia_create_xc_instance_table(*ppInstance, psXCInstPri->u32DeviceID);
#else
    if(psXCInstPri->u32DeviceID == 1)
    {
        g_pDevice1Instance = *ppInstance;
    }
    else
    {
        g_pDevice0Instance = *ppInstance;
    }
#endif
#endif
    psXCInstPri->fpXC_Init = MApi_XC_Init_U2;
    psXCInstPri->fpXC_GetChipCaps = MApi_XC_GetChipCaps_U2;
    psXCInstPri->fpXC_OPTEE_Ctrl = MApi_XC_OPTEE_Control_U2;
    psXCInstPri->fpXC_OPTEE_CheckRBase = MApi_XC_OPTEE_CheckRBase_U2;
    psXCInstPri->fpXC_OPTEE_Mux = MApi_XC_OPTEE_Mux_U2;
    psXCInstPri->fpXC_GetResourceByPipeID = MApi_XC_GetResourceByPipeID_U2;
    psXCInstPri->fpXC_ConfigPipe = MApi_XC_ConfigPipe_U2;
    psXCInstPri->fpXC_CheckPipe = MApi_XC_CheckPipe_U2;
    psXCInstPri->fpXC_SetDispWinToReg =    MApi_XC_SetDispWinToReg_U2;
    psXCInstPri->fpXC_GetDispWinFromReg = MApi_XC_GetDispWinFromReg_U2;
    psXCInstPri->fpXC_Vtrack_SetPayloadData = MApi_XC_Vtrack_SetPayloadData_U2;
    psXCInstPri->fpXC_Vtrack_SetUserDefindedSetting = MApi_XC_Vtrack_SetUserDefindedSetting_U2;
    psXCInstPri->fpXC_Vtrack_Enable = MApi_XC_Vtrack_Enable_U2;
    psXCInstPri->fpXC_GetStatus = MApi_XC_GetStatus_U2;
    psXCInstPri->fpXC_GenerateBlackVideo = MApi_XC_GenerateBlackVideo_U2;
#ifdef UFO_XC_VMARK
    VMARK_Initialize(psXCInstPri, pAttribute);
#endif
    psXCInstPri->fpXC_Set_PowerState = MApi_XC_SetPowerState_U2;

    return UTOPIA_STATUS_SUCCESS;
}
#endif
MS_U32 XCOpenForR2TEE(void** ppInstance, const void* const pAttribute)
{
    XC_INSTANCE_PRIVATE *psXCInstPri = NULL;

    if (UtopiaInstanceCreate(sizeof(XC_INSTANCE_PRIVATE), ppInstance) != UTOPIA_STATUS_SUCCESS)
    {
        printf("[%s,%5d] Create Instance Failed\n",__FUNCTION__,__LINE__);
        return UTOPIA_STATUS_FAIL;
    }

    UtopiaInstanceGetPrivate(*ppInstance, (void**)&psXCInstPri);

    if(pAttribute != NULL)
    {
        MS_U32* u32Attr = (MS_U32*)pAttribute;
        psXCInstPri->u32DeviceID = *u32Attr;
    }
    else
    {
        psXCInstPri->u32DeviceID = 0;
    }
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
#ifdef MSOS_TYPE_LINUX_KERNEL
    utopia_create_xc_instance_table(*ppInstance, psXCInstPri->u32DeviceID);
#else
    if(psXCInstPri->u32DeviceID == 1)
    {
        g_pDevice1Instance = *ppInstance;
    }
    else
    {
        g_pDevice0Instance = *ppInstance;
    }
#endif
#endif
    psXCInstPri->fpXC_Init = MApi_XC_Init_U2;
    psXCInstPri->fpXC_GetStatus = MApi_XC_GetStatus_U2;
    return UTOPIA_STATUS_SUCCESS;
}

MS_U32 XCIoctl(void* pInstance, MS_U32 u32Cmd, void* pArgs)
{
    E_XC_IOCTL_CMDS eCmds = (E_XC_IOCTL_CMDS) u32Cmd;

    if( eCmds >= E_XC_CMD_NUM )
    {
        printf(" IOCTL command out of bounds, please check!\n");
        return UTOPIA_STATUS_NOT_SUPPORTED;
    }

    MS_U32 u32Return = UTOPIA_STATUS_FAIL;
    // APIXC
    // E_XC_CMD_NONE = 0 so eCmds can not compare with E_XC_CMD_NONE
    // eCmds is unsigned value, can not compare with 0
    if (eCmds <= E_XC_CMD_MAX)
    {
        u32Return = _XCIoctl(pInstance, u32Cmd, pArgs);
    }
#ifndef MSOS_TYPE_NUTTX
    // APIXC_ADC
    else if((eCmds >= E_XC_ADC_CMD_NONE)
            && (eCmds <= E_XC_ADC_CMD_MAX))
    {
        u32Return = _ADCIoctl(pInstance, u32Cmd, pArgs);
    }
    // APIXC_AUTO
    else if((eCmds >= E_XC_AUTO_CMD_NONE)
            && (eCmds <= E_XC_AUTO_CMD_MAX))
    {
        u32Return = _AUTOIoctl(pInstance, u32Cmd, pArgs);
    }

    // APIXC_MODEPARSE
    else if((eCmds >= E_XC_MODEPARSE_CMD_NONE)
            && (eCmds <= E_XC_MODEPARSE_CMD_MAX))
    {
        u32Return = _MODEPARSEIoctl(pInstance, u32Cmd, pArgs);
    }
    // APIXC_PCMONITOR
    else if((eCmds >= E_XC_PCMONITOR_CMD_NONE)
            && (eCmds <= E_XC_PCMONITOR_CMD_MAX))
    {
        u32Return = _PCMONITORIoctl(pInstance, u32Cmd, pArgs);
    }
    // DRVXC_HDMI_IF
    else if((eCmds >= E_XC_HDMI_IF_CMD_NONE)
            && (eCmds <= E_XC_HDMI_IF_CMD_MAX))
    {
        u32Return = _HDMI_IF_Ioctl(pInstance, u32Cmd, pArgs);
    }
#ifdef UFO_XC_VMARK
    else if((eCmds >= E_XC_VMARK_CMD_NONE)
            && (eCmds <= E_XC_VMARK_CMD_MAX))
    {
        u32Return = _VMARKIoctl(pInstance, u32Cmd, pArgs);
    }
#endif
#endif
    return u32Return;
}

MS_U32 _XCIoctl(void* pInstance, MS_U32 u32Cmd, void* pArgs)
{
    //void* pModule = NULL;
    //void* pResource = NULL;
    XC_INSTANCE_PRIVATE* psXCInstPri = NULL;

    //UtopiaInstanceGetModule(pInstance, &pModule);

    if(UtopiaInstanceGetPrivate(pInstance, (void*)&psXCInstPri)!=UTOPIA_STATUS_SUCCESS)
    {
        printf("[%s,%5d] Instance Get Private failed\n",__FUNCTION__,__LINE__);
    }

#ifdef MSOS_TYPE_LINUX_KERNEL
    if(psXCInstPri->u32DeviceID == 0)
    {
        if(pInstanceXCStr != pInstance)
        {
            pu32XCInst_private = pInstance;
        }
    }
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
    else if(psXCInstPri->u32DeviceID == 1)
    {
        if(pInstance1XCStr != pInstance)
        {
            pu32XCInst_1_private = pInstance;
        }
    }
    else
    {
        printf("u32InstanceID = %td is over 1!!\n", (ptrdiff_t)psXCInstPri->u32DeviceID);
    }
#endif
#else
    if(psXCInstPri->u32DeviceID == 0)
    {
        pu32XCInst_private = pInstance;
    }
    else if(psXCInstPri->u32DeviceID == 1)
    {
        pu32XCInst_1_private = pInstance;
    }
    else
    {
        printf("u32InstanceID = %td is over 1!!\n", (ptrdiff_t)psXCInstPri->u32DeviceID);
    }
#endif

    //printf("[%s,%5d] u32Cmd type = %ld\n",__FUNCTION__,__LINE__,u32Cmd);
    MS_U32 u32Return = UTOPIA_STATUS_FAIL;
    switch(u32Cmd)
    {
        case E_XC_CMD_GET_LIB_VERSION:
        {
            pstXC_GET_LIB_VER pXCArgs = (pstXC_GET_LIB_VER) pArgs;
            pXCArgs->eReturnValue = psXCInstPri->fpXC_GetLibVer(pInstance, pXCArgs->ppVersion);

            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_GET_INFO:
        {

            pstXC_CMD_GET_INFO  pXCArgs = (pstXC_CMD_GET_INFO) pArgs;
#ifndef MSOS_TYPE_LINUX_KERNEL
            pXCArgs->stReturnValue = psXCInstPri->fpXC_GetInfo(pInstance);
#else
            memcpy(pXCArgs->stReturnValue,psXCInstPri->fpXC_GetInfo(pInstance),sizeof(XC_ApiInfo));
#endif

            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_GET_STATUS:
        {

            pstXC_GET_STATUS pXCArgs = (pstXC_GET_STATUS) pArgs;
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
            USE_OTHER_SC_AS_SUB_WINDOW();
#endif
            pXCArgs->bReturnValue = psXCInstPri->fpXC_GetStatus(pInstance,pXCArgs->pDrvStatus, pXCArgs->eWindow);

            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }

        case E_XC_CMD_GET_STATUS_NODELAY:
        {

            PST_XC_GET_STATUS_NODELAY pXCArgs = (PST_XC_GET_STATUS_NODELAY) pArgs;
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
            USE_OTHER_SC_AS_SUB_WINDOW();
#endif
            pXCArgs->u16ReturnValue = psXCInstPri->fpXC_GetStatus_nodelay(pInstance,pXCArgs->pDrvStatus, pXCArgs->eWindow);

            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_GET_STATUS_EX:
        {

            pstXC_GET_STATUS_EX  pXCArgs = (pstXC_GET_STATUS_EX) pArgs;
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
            USE_OTHER_SC_AS_SUB_WINDOW();
#endif
            pXCArgs->u16ReturnValue = psXCInstPri->fpXC_GetStatusEx(pInstance,pXCArgs->pDrvStatusEx, pXCArgs->eWindow);

            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_SET_DBG_LEVEL:
        {

            pstXC_SET_DBG_LEVEL pXCArgs = (pstXC_SET_DBG_LEVEL) pArgs;
            pXCArgs->bReturnValue = psXCInstPri->fpXC_SetDbgLevel(pInstance,pXCArgs->u16DbgSwitch);

            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }

        case E_XC_CMD_SET_FPLL_CUSTOMERMODE:
        {

            pstXC_SET_FPLL_CUSTOMERMODE pXCArgs = (pstXC_SET_FPLL_CUSTOMERMODE) pArgs;
            psXCInstPri->fpXC_FPLLCustomerMode(pInstance,pXCArgs->eMode, pXCArgs->eFlag, pXCArgs->u32Data);

            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_SET_IOMAPBASE:

        {

            pstXC_CMD_SET_IOMAPBASE pXCArgs = (pstXC_CMD_SET_IOMAPBASE) pArgs;
            pXCArgs->bReturnValue = psXCInstPri->fpXC_SetIOMapBase(pInstance);

            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_INIT:
        {

            pstXC_INIT pXCArgs = (pstXC_INIT) pArgs;
            pXCArgs->bReturnValue = psXCInstPri->fpXC_Init(pInstance,pXCArgs->pXC_InitData, pXCArgs->u32InitDataLen);
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
            CHECK_DEVICE1_INSTANCE();
            if (pXCArgs->pXC_InitData == NULL)
            {
                u32Return = UTOPIA_STATUS_PARAMETER_ERROR;
                break;
            }

            XC_INITDATA stInitData;
            memset(&stInitData,0x00,sizeof(XC_INITDATA));

            //because use sc1 main as sc0 sub, sc1 mian use sc0 sub frame buffer
            memcpy(&stInitData,pXCArgs->pXC_InitData,sizeof(XC_INITDATA));
            stInitData.u32Main_FB_Start_Addr = pXCArgs->pXC_InitData->u32Sub_FB_Start_Addr;
            stInitData.u32Main_FB_Size = pXCArgs->pXC_InitData->u32Sub_FB_Size;

            pXCArgs->bReturnValue = psXCInstPri->fpXC_Init(g_pDevice1Instance,&stInitData, pXCArgs->u32InitDataLen);
#endif
            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_INIT_MISC:
        {

            pstXC_INIT_MISC pXCArgs = (pstXC_INIT_MISC) pArgs;
            pXCArgs->eReturnValue = psXCInstPri->fpXC_Init_MISC(pInstance,pXCArgs->pXC_Init_Misc, pXCArgs->u32InitMiscDataLen);

            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_GET_MISC_STATUS:
        {

            pstXC_GET_MISC_STATUS pXCArgs = (pstXC_GET_MISC_STATUS) pArgs;
            pXCArgs->eReturnValue = psXCInstPri->fpXC_GetMISCStatus(pInstance,pXCArgs->pXC_Init_Misc);

            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_GET_CAPABILITY:
        {

            pstXC_GET_CAPABILITY pXCArgs = (pstXC_GET_CAPABILITY) pArgs;
            pXCArgs->u32ReturnValue = psXCInstPri->fpXC_GetCapability(pInstance,pXCArgs->u32Id);

            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_GET_CHIPCAPS:
        {


            pstXC_GET_CHIP_CAPS pXCArgs = (pstXC_GET_CHIP_CAPS) pArgs;
            pXCArgs->eReturnValue = psXCInstPri->fpXC_GetChipCaps(pInstance,pXCArgs->eCapType, pXCArgs->pRet, pXCArgs->ret_size);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_EXIT:
        {

            pstXC_EXIT pXCArgs = (pstXC_EXIT) pArgs;
            pXCArgs->bReturnValue = psXCInstPri->fpXC_Exit(pInstance);
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
            CHECK_DEVICE1_INSTANCE();
            pXCArgs->bReturnValue = psXCInstPri->fpXC_Exit(g_pDevice1Instance);
#endif
            printf("XC Exited\n");

            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_SET_DYNAMIC_SCALING:
        {

            pstXC_SET_DYNAMIC_SCALING pXCArgs = (pstXC_SET_DYNAMIC_SCALING) pArgs;
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
            if(pXCArgs->eWindow == SUB_WINDOW)
            {
                if(pXCArgs->pstDSInfo->bIPS_DS_On)
                {
                    pXCArgs->pstDSInfo->bIPM_DS_On = TRUE;
                    pXCArgs->pstDSInfo->bOP_DS_On = TRUE;
                    pXCArgs->pstDSInfo->bIPS_DS_On = FALSE;
                }
                else
                {
                    pXCArgs->pstDSInfo->bIPM_DS_On = FALSE;
                    pXCArgs->pstDSInfo->bOP_DS_On = FALSE;
                    pXCArgs->pstDSInfo->bIPS_DS_On = FALSE;
                }
            }

            USE_OTHER_SC_AS_SUB_WINDOW();
#endif
            pXCArgs->bReturnValue = psXCInstPri->fpXC_SetDynamicScaling(pInstance,pXCArgs->pstDSInfo, pXCArgs->u32DSInfoLen, pXCArgs->eWindow);

            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }

        case E_XC_CMD_SET_DYNAMIC_SCALING_FLAG:
        {

            pstXC_SET_DYNAMIC_SCALING_FLAG pXCArgs = (pstXC_SET_DYNAMIC_SCALING_FLAG) pArgs;
            psXCInstPri->fpXC_SetDynamicScalingFlag(pInstance,pXCArgs->bEnable);
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
            CHECK_DEVICE1_INSTANCE();
            psXCInstPri->fpXC_SetDynamicScalingFlag(g_pDevice1Instance,pXCArgs->bEnable);
#endif

            break;
        }

        case E_XC_CMD_GET_DNR_BASEOFFSET:
        {

            pstXC_GET_DNR_BASEOFFSET pXCArgs = (pstXC_GET_DNR_BASEOFFSET) pArgs;
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
            USE_OTHER_SC_AS_SUB_WINDOW();
#endif
            pXCArgs->u32ReturnValue = psXCInstPri->fpXC_Get_DNRBaseOffset(pInstance, pXCArgs->eWindow);

            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_GET_STORE_FRAMENUM:
        case E_XC_CMD_GET_FRAMENUM_FACTOR:
        {

            pstXC_GET_FRAMENUM_FACTOR pXCArgs = (pstXC_GET_FRAMENUM_FACTOR) pArgs;
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
            USE_OTHER_SC_AS_SUB_WINDOW();
#endif
            pXCArgs->u8ReturnValue = psXCInstPri->fpXC_Get_FrameNumFactor(pInstance, pXCArgs->eWindow);

            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_SET_WINDOW:
        {
            pstXC_SET_WINDOW pXCArgs = (pstXC_SET_WINDOW) pArgs;
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
            USE_OTHER_SC_AS_SUB_WINDOW();
#endif
            pXCArgs->bReturnValue = psXCInstPri->fpXC_SetWindow(pInstance, pXCArgs->pstXC_SetWin_Info, pXCArgs->u32InitDataLen, pXCArgs->eWindow);

            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }

        case E_XC_CMD_SET_DUALWINDOW:
        {
            pstXC_SET_DUALWINDOW pXCArgs = (pstXC_SET_DUALWINDOW) pArgs;
            pXCArgs->bReturnValue = psXCInstPri->fpXC_SetDualWindow(pInstance, pXCArgs->pstXC_SetWin_Info_Main, pXCArgs->pstXC_SetWin_Info_Sub);

            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }

        case E_XC_CMD_SET_TRAVELING_WINDOW:
        {


            pstXC_SET_TRAVELING_WINDOW pXCArgs = (pstXC_SET_TRAVELING_WINDOW) pArgs;
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
            USE_OTHER_SC_AS_SUB_WINDOW();
#endif
            pXCArgs->bReturnValue = psXCInstPri->fpXC_SetTravelingWindow(pInstance, pXCArgs->pstXC_SetWin_Info, pXCArgs->u32InitDataLen, pXCArgs->eWindow);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_SET_INPUTSOURCE:
        {

            pstXC_SET_INPUTSOURCE pXCArgs = (pstXC_SET_INPUTSOURCE) pArgs;
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
            USE_OTHER_SC_AS_SUB_WINDOW();
#endif
            psXCInstPri->fpXC_SetInputSource(pInstance, pXCArgs->enInputSourceType, pXCArgs->eWindow);

            u32Return = UTOPIA_STATUS_SUCCESS;

            break;
        }


        case E_XC_CMD_CHECK_YUVSPACE:
        {


            pstXC_CHECK_YUVSPACE pXCArgs = (pstXC_CHECK_YUVSPACE) pArgs;
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
            USE_OTHER_SC_AS_SUB_WINDOW();
#endif
            pXCArgs->bReturnValue = psXCInstPri->fpXC_IsYUVSpace(pInstance, pXCArgs->eWindow);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_CHECK_MEMORYFORMAT422:
        {


            pstXC_CHECK_MEMORYFORMAT422 pXCArgs = (pstXC_CHECK_MEMORYFORMAT422) pArgs;
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
            USE_OTHER_SC_AS_SUB_WINDOW();
#endif
            pXCArgs->bReturnValue = psXCInstPri->fpXC_IsMemoryFormat422(pInstance, pXCArgs->eWindow);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_SET_FORCE_RGBIN:
        {


            pstXC_SET_FORCE_RGBIN pXCArgs = (pstXC_SET_FORCE_RGBIN) pArgs;
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
            USE_OTHER_SC_AS_SUB_WINDOW();
#endif
            psXCInstPri->fpXC_EnableForceRGBin(pInstance, pXCArgs->bEnable, pXCArgs->eWindow);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_SET_MIRRORMODE_EX:
        {


            pstXC_SET_MIRRORMODE_EX pXCArgs = (pstXC_SET_MIRRORMODE_EX) pArgs;
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
            //USE_OTHER_SC_AS_SUB_WINDOW();
            pXCArgs->bReturnValue = psXCInstPri->fpXC_EnableMirrorModeEx(g_pDevice1Instance, pXCArgs->eMirrorMode, MAIN_WINDOW);
#endif
            pXCArgs->bReturnValue = psXCInstPri->fpXC_EnableMirrorModeEx(pInstance, pXCArgs->eMirrorMode, pXCArgs->eWindow);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_GET_MIRRORMODE_TYPEEX:
        {


            pstXC_GET_MIRRORMODE_TYPEEX pXCArgs = (pstXC_GET_MIRRORMODE_TYPEEX) pArgs;
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
            USE_OTHER_SC_AS_SUB_WINDOW();
#endif
            pXCArgs->eReturnValue = psXCInstPri->fpXC_GetMirrorModeTypeEx(pInstance, pXCArgs->eWindow);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }

#ifdef UFO_XC_SCAN_TYPE
        case E_XC_CMD_SET_SCAN_TYPE:
            {
                pstXC_SET_SCAN_TYPE pXCArgs = (pstXC_SET_SCAN_TYPE) pArgs;
                psXCInstPri->fpXC_SetScanType(pInstance, pXCArgs->enScanType);

                u32Return = UTOPIA_STATUS_SUCCESS;
                break;
            }
#endif

        case E_XC_CMD_GET_SYNC_STATUS:
        {


            pstXC_GET_SYNC_STATUS pXCArgs = (pstXC_GET_SYNC_STATUS) pArgs;
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
            USE_OTHER_SC_AS_SUB_WINDOW();
#endif
            psXCInstPri->fpXC_GetSyncStatus(pInstance, pXCArgs->eCurrentSrc, pXCArgs->sXC_Sync_Status, pXCArgs->eWindow);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }

        case E_XC_CMD_CONFIG_CMA:
        {


            pstXC_CONFIG_CMA pXCArgs = (pstXC_CONFIG_CMA) pArgs;
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
            USE_OTHER_SC_AS_SUB_WINDOW();
#endif
            psXCInstPri->fpXC_ConfigCMA(pInstance, pXCArgs->pstXC_CMA_Config, pXCArgs->enCmaClient, pXCArgs->u32InitDataLen, pXCArgs->eWindow);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }

        case E_XC_CMD_SET_WAIT_OUTPUT_VSYNC:
        {


            pstXC_SET_WAIT_OUTPUT_VSYNC pXCArgs = (pstXC_SET_WAIT_OUTPUT_VSYNC) pArgs;
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
            USE_OTHER_SC_AS_SUB_WINDOW();
#endif
            pXCArgs->u8ReturnValue = psXCInstPri->fpXC_WaitOutputVSync(pInstance, pXCArgs->u8NumVSyncs, pXCArgs->u16Timeout, pXCArgs->eWindow);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_SET_WAIT_INPUT_VSYNC:
        {


            pstXC_SET_WAIT_INPUT_VSYNC pXCArgs = (pstXC_SET_WAIT_INPUT_VSYNC) pArgs;
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
            USE_OTHER_SC_AS_SUB_WINDOW();
#endif
            pXCArgs->u8ReturnValue = psXCInstPri->fpXC_WaitInputVSync(pInstance, pXCArgs->u8NumVSyncs, pXCArgs->u16Timeout, pXCArgs->eWindow);



            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_SET_HDMI_SYNCMODE:
        {


            pstXC_SET_HDMI_SYNCMODE pXCArgs = (pstXC_SET_HDMI_SYNCMODE) pArgs;
            psXCInstPri->fpXC_SetHdmiSyncMode(pInstance, pXCArgs->eSynctype);
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
            CHECK_DEVICE1_INSTANCE();
            psXCInstPri->fpXC_SetHdmiSyncMode(g_pDevice1Instance, pXCArgs->eSynctype);
#endif

            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_GET_HDMI_SYNCMODE:
        {


            pstXC_GET_HDMI_SYNCMODE pXCArgs = (pstXC_GET_HDMI_SYNCMODE) pArgs;
            pXCArgs->eReturnValue = psXCInstPri->fpXC_GetHdmiSyncMode(pInstance);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_SET_REPORT_WINDOW:
        {
            pstXC_SET_REPORT_WINDOW pXCArgs = (pstXC_SET_REPORT_WINDOW) pArgs;
            psXCInstPri->fpXC_SetRepWindow(pInstance, pXCArgs->bEnable, pXCArgs->Window, pXCArgs->u8Color);
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
            CHECK_DEVICE1_INSTANCE();
            psXCInstPri->fpXC_SetRepWindow(g_pDevice1Instance, pXCArgs->bEnable, pXCArgs->Window, pXCArgs->u8Color);
#endif
            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }

        case E_XC_CMD_SET_SKIP_OPWRITEOFF_IN_SETWINDOW:
        {
            pstXC_SET_SKIP_OPWRITEOFF_IN_SETWINDOW pXCArgs = (pstXC_SET_SKIP_OPWRITEOFF_IN_SETWINDOW) pArgs;
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
            USE_OTHER_SC_AS_SUB_WINDOW();
#endif
            psXCInstPri->fpXC_SkipOPWriteOffInSetWindow(pInstance, pXCArgs->bEnable, pXCArgs->eWindow);

            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }

        case E_XC_CMD_GET_SKIP_OPWRITEOFF_IN_SETWINDOW:
        {
            pstXC_GET_SKIP_OPWRITEOFF_IN_SETWINDOW pXCArgs = (pstXC_GET_SKIP_OPWRITEOFF_IN_SETWINDOW) pArgs;
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
            USE_OTHER_SC_AS_SUB_WINDOW();
#endif
            pXCArgs->bReturnValue = psXCInstPri->fpXC_GetSkipOPWriteOffInSetWindow(pInstance,pXCArgs->eWindow);

            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }

        case E_XC_SET_SKIP_DISABLE_OPWRITEOFF_IN_FPLL:
        {
            pstXC_SET_SKIP_DISABLE_OPWRITEOFF_IN_FPLL pXCArgs = (pstXC_SET_SKIP_DISABLE_OPWRITEOFF_IN_FPLL) pArgs;
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
            USE_OTHER_SC_AS_SUB_WINDOW();
#endif
            psXCInstPri->fpXC_SetSkipDisableOPWriteOffInFPLL(pInstance, pXCArgs->bEnable, pXCArgs->eWindow);

            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }

        case E_XC_CMD_SET_OPWRITEOFF_ENABLE:
        {
            pstXC_SET_OPWRITEOFF_ENABLE pXCArgs = (pstXC_SET_OPWRITEOFF_ENABLE) pArgs;
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
            USE_OTHER_SC_AS_SUB_WINDOW();
#endif
            psXCInstPri->fpXC_Set_OPWriteOffEnable(pInstance, pXCArgs->bEnable, pXCArgs->eWindow);

            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }

        case E_XC_CMD_SET_OPWRITEOFF_ENABLE_TO_REG:
        {
            pstXC_SET_OPWRITEOFF_ENABLE_TO_REG pXCArgs = (pstXC_SET_OPWRITEOFF_ENABLE_TO_REG) pArgs;
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
            USE_OTHER_SC_AS_SUB_WINDOW();
#endif
            psXCInstPri->fpXC_Set_OPWriteOffEnableToReg(pInstance, pXCArgs->bEnable, pXCArgs->eWindow);

            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }

        case E_XC_CMD_FORCESET_OPWRITEOFF_ENABLE:
        {
            pstXC_FORCESET_OPWRITEOFF_ENABLE pXCArgs = (pstXC_FORCESET_OPWRITEOFF_ENABLE) pArgs;
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
            USE_OTHER_SC_AS_SUB_WINDOW();
#endif
            psXCInstPri->fpXC_ForceSet_OPWriteOffEnable(pInstance, pXCArgs->bEnable, pXCArgs->eWindow);

            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }

        case E_XC_CMD_GET_OPWRITEOFF_ENABLE:
        {
            pstXC_GET_OPWRITEOFF_ENABLE pXCArgs = (pstXC_GET_OPWRITEOFF_ENABLE) pArgs;
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
            USE_OTHER_SC_AS_SUB_WINDOW();
#endif
            pXCArgs->bReturnValue = psXCInstPri->fpXC_Get_OPWriteOffEnable(pInstance, pXCArgs->eWindow);

            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }

        case E_XC_CMD_SET_DISPLAY_WIN_TO_REG:
        {
            pstXC_SET_DISPLAY_WIN_TO_REG pXCArgs = (pstXC_SET_DISPLAY_WIN_TO_REG) pArgs;
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
            USE_OTHER_SC_AS_SUB_WINDOW();
#endif
            psXCInstPri->fpXC_SetDispWinToReg(pInstance, pXCArgs->pstDspwin, pXCArgs->eWindow);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_GET_DISPLAY_WIN_FROM_REG:
        {


            pstXC_GET_DISPLAY_WIN_FROM_REG pXCArgs = (pstXC_GET_DISPLAY_WIN_FROM_REG) pArgs;
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
            USE_OTHER_SC_AS_SUB_WINDOW();
#endif
            psXCInstPri->fpXC_GetDispWinFromReg(pInstance, pXCArgs->pstDspwin, pXCArgs->eWindow);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_SET_FREEZEIMG:
        {

            pstXC_SET_FREEZEIMG pXCArgs = (pstXC_SET_FREEZEIMG) pArgs;
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
            USE_OTHER_SC_AS_SUB_WINDOW();
#endif
            psXCInstPri->fpXC_FreezeImg(pInstance, pXCArgs->bEnable, pXCArgs->eWindow);

            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }
        case E_XC_CMD_CHECK_FREEZEIMG:
        {


            pstXC_CHECK_FREEZEIMG pXCArgs = (pstXC_CHECK_FREEZEIMG) pArgs;
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
            USE_OTHER_SC_AS_SUB_WINDOW();
#endif
            pXCArgs->bReturnValue = psXCInstPri->fpXC_IsFreezeImg(pInstance, pXCArgs->eWindow);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }

        case E_XC_CMD_HSE_CONTROL:
        {

            pstXC_HSE_CONTROL pXCArgs = (pstXC_HSE_CONTROL) pArgs;

            pXCArgs->eReturnValue = psXCInstPri->fpXC_HSE_Control(pInstance, pXCArgs->bEnable);

            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }

        case E_XC_CMD_SET_EVENT:
        {

            pstXC_SET_EVENT pXCArgs = (pstXC_SET_EVENT) pArgs;

            pXCArgs->eReturnValue = psXCInstPri->fpXC_Set_Event(pInstance, pXCArgs->pstXCEvent);

            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }

        case E_XC_CMD_SET_BOTHWINDOW_BLACKVIDEO:
        {


            pstXC_SET_BOTHWINDOW_BLACKVIDEO pXCArgs = (pstXC_SET_BOTHWINDOW_BLACKVIDEO) pArgs;
            psXCInstPri->fpXC_GenerateBlackVideoForBothWin(pInstance, pXCArgs->bEnable);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_SET_BLACKSCREEN:
        {


            pstXC_SET_BLACKSCREEN pXCArgs = (pstXC_SET_BLACKSCREEN) pArgs;
            psXCInstPri->fpXC_Set_BLSK(pInstance, pXCArgs->bEnable);
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
            CHECK_DEVICE1_INSTANCE();
            psXCInstPri->fpXC_Set_BLSK(g_pDevice1Instance, pXCArgs->bEnable);
#endif
            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_SET_BLACKVIDEO:
        {


            pstXC_SET_BLACKVIDEO pXCArgs = (pstXC_SET_BLACKVIDEO) pArgs;
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
            USE_OTHER_SC_AS_SUB_WINDOW();
#endif
            psXCInstPri->fpXC_GenerateBlackVideo(pInstance, pXCArgs->bEnable, pXCArgs->eWindow);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_CHECK_BLACKVIDEO_ENABLE:
        {

            pstXC_CHECK_BLACKVIDEO_ENABLE pXCArgs = (pstXC_CHECK_BLACKVIDEO_ENABLE) pArgs;
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
            USE_OTHER_SC_AS_SUB_WINDOW();
#endif
            pXCArgs->bReturnValue = psXCInstPri->fpXC_IsBlackVideoEnable(pInstance, pXCArgs->eWindow);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_SET_FRAMEBUFFERLESS:
        {


            pstXC_SET_FRAMEBUFFERLESS pXCArgs = (pstXC_SET_FRAMEBUFFERLESS) pArgs;
            psXCInstPri->fpXC_EnableFrameBufferLess(pInstance, pXCArgs->bEnable);
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
            CHECK_DEVICE1_INSTANCE();
            psXCInstPri->fpXC_EnableFrameBufferLess(g_pDevice1Instance, pXCArgs->bEnable);
#endif

            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_CHECK_FRAMEBUFFERLESS:
        {


            pstXC_CHECK_FRAMEBUFFERLESS pXCArgs = (pstXC_CHECK_FRAMEBUFFERLESS) pArgs;
            pXCArgs->bReturnValue = psXCInstPri->fpXC_IsCurrentFrameBufferLessMode(pInstance);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_SET_REQUEST_FRAMEBUFFERLESS:
        {


            pstXC_SET_REQUEST_FRAMEBUFFERLESS pXCArgs = (pstXC_SET_REQUEST_FRAMEBUFFERLESS) pArgs;
            psXCInstPri->fpXC_EnableRequest_FrameBufferLess(pInstance, pXCArgs->bEnable);
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
            CHECK_DEVICE1_INSTANCE();
            psXCInstPri->fpXC_EnableRequest_FrameBufferLess(g_pDevice1Instance, pXCArgs->bEnable);
#endif
            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_CHECK_REQUEST_FRAMEBUFFERLESS:
        {


            pstXC_CHECK_REQUEST_FRAMEBUFFERLESS pXCArgs = (pstXC_CHECK_REQUEST_FRAMEBUFFERLESS) pArgs;
            pXCArgs->bReturnValue = psXCInstPri->fpXC_IsCurrentRequest_FrameBufferLessMode(pInstance);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_GET_3D_HWVERSION:
        {


            pstXC_GET_3D_HWVERSION pXCArgs = (pstXC_GET_3D_HWVERSION) pArgs;
            pXCArgs->u16ReturnValue = psXCInstPri->fpXC_Get_3D_HW_Version(pInstance);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_CHECK_3D_SUPPORT_HW2DTO3D:
        {


            pstXC_CHECK_3D_SUPPORT_HW2DTO3D pXCArgs = (pstXC_CHECK_3D_SUPPORT_HW2DTO3D) pArgs;
            pXCArgs->bReturnValue = psXCInstPri->fpXC_Get_3D_IsSupportedHW2DTo3D(pInstance);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_SET_3D_MODE:
        {


            pstXC_SET_3D_MODE pXCArgs = (pstXC_SET_3D_MODE) pArgs;
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
            USE_OTHER_SC_AS_SUB_WINDOW();
#endif
            pXCArgs->bReturnValue = psXCInstPri->fpXC_Set_3D_Mode(pInstance,
                                    pXCArgs->e3dInputMode,
                                    pXCArgs->e3dOutputMode,
                                    pXCArgs->e3dPanelType,
                                    pXCArgs->eWindow);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_SET_3D_MAINWIN_FIRST:
        {


            pstXC_SET_3D_MAINWIN_FIRST pXCArgs = (pstXC_SET_3D_MAINWIN_FIRST) pArgs;
            pXCArgs->bReturnValue = psXCInstPri->fpXC_Set_3D_MainWin_FirstMode(pInstance, pXCArgs->bMainFirst);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_SET_3D_LR_EXCHANGE:
        {


            pstXC_SET_3D_LR_EXCHANGE pXCArgs = (pstXC_SET_3D_LR_EXCHANGE) pArgs;
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
            USE_OTHER_SC_AS_SUB_WINDOW();
#endif
            pXCArgs->bReturnValue = psXCInstPri->fpXC_Set_3D_LR_Frame_Exchg(pInstance, pXCArgs->eWindow);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_CHECK_3D_LR_EXCHANGED:
        {


            pstXC_CHECK_3D_LR_EXCHANGED pXCArgs = (pstXC_CHECK_3D_LR_EXCHANGED) pArgs;
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
            USE_OTHER_SC_AS_SUB_WINDOW();
#endif
            pXCArgs->bReturnValue = psXCInstPri->fpXC_3D_Is_LR_Frame_Exchged(pInstance, pXCArgs->eWindow);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_GET_3D_INPUT_MODE:
        {


            pstXC_GET_3D_INPUT_MODE pXCArgs = (pstXC_GET_3D_INPUT_MODE) pArgs;
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
            USE_OTHER_SC_AS_SUB_WINDOW();
#endif
            pXCArgs->eReturnValue = psXCInstPri->fpXC_Get_3D_Input_Mode(pInstance, pXCArgs->eWindow);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_GET_3D_OUTPUT_MODE:
        {


            pstXC_GET_3D_OUTPUT_MODE pXCArgs = (pstXC_GET_3D_OUTPUT_MODE) pArgs;
            pXCArgs->eReturnValue = psXCInstPri->fpXC_Get_3D_Output_Mode(pInstance);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


#ifdef UFO_XC_GET_3D_FORMAT
        case E_XC_CMD_GET3DFORMAT:
        {


            pstXC_GET3DFORMAT pXCArgs = (pstXC_GET3DFORMAT) pArgs;
            pXCArgs->bReturnValue = psXCInstPri->fpXC_Get3DFormat(pInstance, pXCArgs->e3DAttrType, pXCArgs->para, pXCArgs->p3DFormat);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }
#endif


        case E_XC_CMD_GET_3D_PANELTYPE:
        {


            pstXC_GET_3D_PANELTYPE pXCArgs = (pstXC_GET_3D_PANELTYPE) pArgs;
            pXCArgs->eReturnValue = psXCInstPri->fpXC_Get_3D_Panel_Type(pInstance);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_GET_3D_MAINWIN_FIRST:
        {


            pstXC_GET_3D_MAINWIN_FIRST pXCArgs = (pstXC_GET_3D_MAINWIN_FIRST) pArgs;
            pXCArgs->bReturnValue = psXCInstPri->fpXC_Get_3D_MainWin_First(pInstance);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_CHECK_3D_MAINSUB_IPSYNC:
        {


            pstXC_CHECK_3D_MAINSUB_IPSYNC pXCArgs = (pstXC_CHECK_3D_MAINSUB_IPSYNC) pArgs;
            pXCArgs->bReturnValue = psXCInstPri->fpXC_3DMainSub_IPSync(pInstance);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_SET_3D_VERTICALVIDEO_OFFSET:
        {


            pstXC_SET_3D_VERTICALVIDEO_OFFSET pXCArgs = (pstXC_SET_3D_VERTICALVIDEO_OFFSET) pArgs;
            pXCArgs->bReturnValue = psXCInstPri->fpXC_Set_3D_VerVideoOffset(pInstance, pXCArgs->u163DVerVideoOffset);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_GET_3D_VERTICALVIDEO_OFFSET:
        {


            pstXC_GET_3D_VERTICALVIDEO_OFFSET pXCArgs = (pstXC_GET_3D_VERTICALVIDEO_OFFSET) pArgs;
            pXCArgs->u16ReturnValue = psXCInstPri->fpXC_Get_3D_VerVideoOffset(pInstance);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_CHECK_3D_FORMAT_SUPPORTED:
        {


            pstXC_CHECK_3D_FORMAT_SUPPORTED pXCArgs = (pstXC_CHECK_3D_FORMAT_SUPPORTED) pArgs;
            pXCArgs->bReturnValue = psXCInstPri->fpXC_Is3DFormatSupported(pInstance, pXCArgs->e3dInputMode, pXCArgs->e3dOutputMode);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_SET_3D_HORIZONTAL_SHIFT:
        {


            pstXC_SET_3D_HORIZONTAL_SHIFT pXCArgs = (pstXC_SET_3D_HORIZONTAL_SHIFT) pArgs;
            pXCArgs->bReturnValue = psXCInstPri->fpXC_Set_3D_HShift(pInstance, pXCArgs->u16HShift);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_SET_3D_LR_SBS2LINE:
        {


            pstXC_SET_3D_LR_SBS2LINE pXCArgs = (pstXC_SET_3D_LR_SBS2LINE) pArgs;
            pXCArgs->bReturnValue = psXCInstPri->fpXC_Enable_3D_LR_Sbs2Line(pInstance, pXCArgs->bEnable);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_GET_3D_HORIZONTAL_SHIFT:
        {


            pstXC_GET_3D_HORIZONTAL_SHIFT pXCArgs = (pstXC_GET_3D_HORIZONTAL_SHIFT) pArgs;
            pXCArgs->u16ReturnValue = psXCInstPri->fpXC_Get_3D_HShift(pInstance);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_SET_3D_HW2DTO3D_BUFFER:
        {


            pstXC_SET_3D_HW2DTO3D_BUFFER pXCArgs = (pstXC_SET_3D_HW2DTO3D_BUFFER) pArgs;
            pXCArgs->bReturnValue = psXCInstPri->fpXC_Set_3D_HW2DTo3D_Buffer(pInstance, pXCArgs->u32HW2DTO3D_DD_Buf, pXCArgs->u32HW2DTO3D_DR_Buf);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_SET_3D_HW2DTO3D_PARAMETERS:
        {


            pstXC_SET_3D_HW2DTO3D_PARAMETERS pXCArgs = (pstXC_SET_3D_HW2DTO3D_PARAMETERS) pArgs;
            pXCArgs->bReturnValue = psXCInstPri->fpXC_Set_3D_HW2DTo3D_Parameters(pInstance, pXCArgs->st3DHw2DTo3DPara);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_GET_3D_HW2DTO3D_PARAMETERS:
        {


            pstXC_GET_3D_HW2DTO3D_PARAMETERS pXCArgs = (pstXC_GET_3D_HW2DTO3D_PARAMETERS) pArgs;
            pXCArgs->bReturnValue = psXCInstPri->fpXC_Get_3D_HW2DTo3D_Parameters(pInstance, pXCArgs->pst3DHw2DTo3DPara);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_SET_3D_DETECT_3DFORMAT_PARAMETERS:
        {


            pstXC_SET_3D_DETECT_3DFORMAT_PARAMETERS pXCArgs = (pstXC_SET_3D_DETECT_3DFORMAT_PARAMETERS) pArgs;
            pXCArgs->bReturnValue = psXCInstPri->fpXC_Set_3D_Detect3DFormat_Parameters(pInstance, pXCArgs->pstDetect3DFormatPara);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_GET_3D_DETECT_3DFORMAT_PARAMETERS:
        {


            pstXC_GET_3D_DETECT_3DFORMAT_PARAMETERS pXCArgs = (pstXC_GET_3D_DETECT_3DFORMAT_PARAMETERS) pArgs;
            pXCArgs->bReturnValue = psXCInstPri->fpXC_Get_3D_Detect3DFormat_Parameters(pInstance, pXCArgs->pstDetect3DFormatPara);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_GET_3D_FORMAT_DETECTED_BY_CONTENT:
        {


            pstXC_GET_3D_FORMAT_DETECTED_BY_CONTENT pXCArgs = (pstXC_GET_3D_FORMAT_DETECTED_BY_CONTENT) pArgs;
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
            USE_OTHER_SC_AS_SUB_WINDOW();
#endif
            pXCArgs->eReturnValue = psXCInstPri->fpXC_Detect3DFormatByContent(pInstance, pXCArgs->eWindow);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_SET_NINELATTICE:
        {


            pstXC_SET_NINELATTICE pXCArgs = (pstXC_SET_NINELATTICE) pArgs;
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
            USE_OTHER_SC_AS_SUB_WINDOW();
#endif
            pXCArgs->bReturnValue = psXCInstPri->fpXC_DetectNL(pInstance, pXCArgs->eWindow, pXCArgs->pstDetectNLatticePara);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_SET_3D_POST_PQSETTING:
        {


            pstXC_SET_3D_POST_PQSETTING pXCArgs = (pstXC_SET_3D_POST_PQSETTING) pArgs;
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
            USE_OTHER_SC_AS_SUB_WINDOW();
#endif
            pXCArgs->bReturnValue = psXCInstPri->fpXC_3D_PostPQSetting(pInstance, pXCArgs->eWindow);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_SET_3D_FRAMEPACKING_INFO:
        {


            pstXC_SET_3D_FRAMEPACKING_INFO pXCArgs = (pstXC_SET_3D_FRAMEPACKING_INFO) pArgs;
            pXCArgs->bReturnValue = psXCInstPri->fpXC_Set_3D_FPInfo(pInstance, pXCArgs->pstFPInfoPara);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_SET_3D_AUTODETECT:
        {


            pstXC_SET_3D_AUTODETECT pXCArgs = (pstXC_SET_3D_AUTODETECT) pArgs;
            pXCArgs->bReturnValue = psXCInstPri->fpXC_EnableAutoDetect3D(pInstance, pXCArgs->bEnable, pXCArgs->enDetectMethod);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_GET_3D_AUTODETECT_3DFLAG:
        {


            pstXC_GET_3D_AUTODETECT_3DFLAG pXCArgs = (pstXC_GET_3D_AUTODETECT_3DFLAG) pArgs;
            pXCArgs->bReturnValue = psXCInstPri->fpXC_GetAutoDetect3DFlag(pInstance, pXCArgs->penDetectMethod, pXCArgs->pbEnable);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_SET_3D_SUBWINCLOCK:
        {


            pstXC_SET_3D_SUBWINCLOCK pXCArgs = (pstXC_SET_3D_SUBWINCLOCK) pArgs;
            pXCArgs->bReturnValue = psXCInstPri->fpXC_Set_3D_SubWinClk(pInstance);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_CHECK_3D_LR_SBS2LINE:
        {


            pstXC_CHECK_3D_LR_SBS2LINE pXCArgs = (pstXC_CHECK_3D_LR_SBS2LINE) pArgs;
            pXCArgs->bReturnValue = psXCInstPri->fpXC_3D_Is_LR_Sbs2Line(pInstance);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_CHECK_3D_SKIP_DEFAULT_LR_FLAG:
        {


            pstXC_CHECK_3D_SKIP_DEFAULT_LR_FLAG pXCArgs = (pstXC_CHECK_3D_SKIP_DEFAULT_LR_FLAG) pArgs;
            pXCArgs->bReturnValue = psXCInstPri->fpSC_3D_Is_Skip_Default_LR_Flag(pInstance);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_SET_3D_SKIP_DEFAULT_LR_FLAG:
        {


            pstXC_SET_3D_SKIP_DEFAULT_LR_FLAG pXCArgs = (pstXC_SET_3D_SKIP_DEFAULT_LR_FLAG) pArgs;
            pXCArgs->bReturnValue = psXCInstPri->fpXC_3D_Enable_Skip_Default_LR_Flag(pInstance, pXCArgs->bEnable);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_SET_MUX_INIT:
        {


            pstXC_SET_MUX_INIT pXCArgs = (pstXC_SET_MUX_INIT) pArgs;
            psXCInstPri->fpXC_Mux_Init(pInstance, pXCArgs->input_source_to_input_port);
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
            CHECK_DEVICE1_INSTANCE();
            psXCInstPri->fpXC_Mux_Init(g_pDevice1Instance, pXCArgs->input_source_to_input_port);
#endif
            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_SET_MUX_SOURCE_MONITOR:
        {


            pstXC_SET_MUX_SOURCE_MONITOR pXCArgs = (pstXC_SET_MUX_SOURCE_MONITOR) pArgs;
            psXCInstPri->fpXC_Mux_SourceMonitor(pInstance, pXCArgs->bRealTimeMonitorOnly);
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
            CHECK_DEVICE1_INSTANCE();
            psXCInstPri->fpXC_Mux_SourceMonitor(g_pDevice1Instance, pXCArgs->bRealTimeMonitorOnly);
#endif
            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_SET_MUX_CREATE_PATH:
        {


            pstXC_SET_MUX_CREATE_PATH pXCArgs = (pstXC_SET_MUX_CREATE_PATH) pArgs;
            pXCArgs->s16ReturnValue = psXCInstPri->fpXC_Mux_CreatePath(pInstance, pXCArgs->Path_Info, pXCArgs->u32InitDataLen);
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
            CHECK_DEVICE1_INSTANCE();
            psXCInstPri->fpXC_Mux_CreatePath(g_pDevice1Instance, pXCArgs->Path_Info, pXCArgs->u32InitDataLen);
#endif
            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_SET_MUX_DELETE_PATH:
        {

            pstXC_SET_MUX_DELETE_PATH pXCArgs = (pstXC_SET_MUX_DELETE_PATH) pArgs;
            pXCArgs->s16ReturnValue = psXCInstPri->fpXC_Mux_DeletePath(pInstance, pXCArgs->src, pXCArgs->dest);
            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_SET_MUX_ENABLE_PATH:
        {


            pstXC_SET_MUX_ENABLE_PATH pXCArgs = (pstXC_SET_MUX_ENABLE_PATH) pArgs;
            pXCArgs->s16ReturnValue = psXCInstPri->fpXC_Mux_EnablePath(pInstance, pXCArgs->PathId);
            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_SET_MUX_TRIGGER_PATH_SYNC_EVENT:
        {


            pstXC_SET_MUX_TRIGGER_PATH_SYNC_EVENT pXCArgs = (pstXC_SET_MUX_TRIGGER_PATH_SYNC_EVENT) pArgs;
            psXCInstPri->fpXC_Mux_TriggerPathSyncEvent(pInstance, pXCArgs->src, pXCArgs->para);
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
            CHECK_DEVICE1_INSTANCE();
            psXCInstPri->fpXC_Mux_TriggerPathSyncEvent(g_pDevice1Instance, pXCArgs->src, pXCArgs->para);
#endif
            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_SET_MUX_TRIGGER_DEST_ONOFF_EVENT:
        {


            pstXC_SET_MUX_TRIGGER_DEST_ONOFF_EVENT pXCArgs = (pstXC_SET_MUX_TRIGGER_DEST_ONOFF_EVENT) pArgs;
            psXCInstPri->fpXC_Mux_TriggerDestOnOffEvent(pInstance, pXCArgs->src, pXCArgs->para);
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
            CHECK_DEVICE1_INSTANCE();
            psXCInstPri->fpXC_Mux_TriggerDestOnOffEvent(g_pDevice1Instance, pXCArgs->src, pXCArgs->para);
#endif
            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_SET_MUX_ONOFF_PERIODIC_HANDLER:
        {


            pstXC_SET_MUX_ONOFF_PERIODIC_HANDLER pXCArgs = (pstXC_SET_MUX_ONOFF_PERIODIC_HANDLER) pArgs;
            pXCArgs->s16ReturnValue = psXCInstPri->fpXC_Mux_OnOffPeriodicHandler(pInstance, pXCArgs->src, pXCArgs->bEnable);
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
            CHECK_DEVICE1_INSTANCE();
            psXCInstPri->fpXC_Mux_OnOffPeriodicHandler(g_pDevice1Instance, pXCArgs->src, pXCArgs->bEnable);
#endif
            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_GET_MUX_PATHINFO:
        {


            pstXC_GET_MUX_PATHINFO pXCArgs = (pstXC_GET_MUX_PATHINFO) pArgs;
            pXCArgs->u8ReturnValue = psXCInstPri->fpXC_Mux_GetPathInfo(pInstance, pXCArgs->Paths);
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
            CHECK_DEVICE1_INSTANCE();
            psXCInstPri->fpXC_Mux_GetPathInfo(g_pDevice1Instance, pXCArgs->Paths);
#endif
            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_SET_MUX_SUPPORT_MHL_PATHINFO:
        {


            pstXC_SET_MUX_SUPPORT_MHL_PATHINFO pXCArgs = (pstXC_SET_MUX_SUPPORT_MHL_PATHINFO) pArgs;
            pXCArgs->eReturnValue = psXCInstPri->fpXC_Mux_SetSupportMhlPathInfo(pInstance, pXCArgs->u8MhlSupportInfo);
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
            CHECK_DEVICE1_INSTANCE();
            psXCInstPri->fpXC_Mux_SetSupportMhlPathInfo(g_pDevice1Instance, pXCArgs->u8MhlSupportInfo);
#endif

            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_SET_MUX_MHL_HOTPLUG_INVERSE_INFO:
        {


            pstXC_SET_MUX_MHL_HOTPLUG_INVERSE_INFO pXCArgs = (pstXC_SET_MUX_MHL_HOTPLUG_INVERSE_INFO) pArgs;
            pXCArgs->eReturnValue = psXCInstPri->fpXC_Mux_SetMhlHotPlugInverseInfo(pInstance, pXCArgs->bIsMhlHotPlugInverse);
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
            CHECK_DEVICE1_INSTANCE();
            psXCInstPri->fpXC_Mux_SetMhlHotPlugInverseInfo(g_pDevice1Instance, pXCArgs->bIsMhlHotPlugInverse);
#endif
            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_GET_MUX_HDMIPORT:
        {


            pstXC_GET_MUX_HDMIPORT pXCArgs = (pstXC_GET_MUX_HDMIPORT) pArgs;
            pXCArgs->eReturnValue = psXCInstPri->fpXC_Mux_GetHDMIPort(pInstance, pXCArgs->src);
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
            CHECK_DEVICE1_INSTANCE();
            psXCInstPri->fpXC_Mux_GetHDMIPort(g_pDevice1Instance, pXCArgs->src);
#endif
            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }

        case E_XC_CMD_GET_MUX_MAPPINGTAB:
        {
            pstXC_GET_MUX_MAPPINGTAB pXCArgs = (pstXC_GET_MUX_MAPPINGTAB) pArgs;
            psXCInstPri->fpXC_Mux_GetMappingTab(pInstance, pXCArgs->mapping_tab,pXCArgs->length);
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
            CHECK_DEVICE1_INSTANCE();
            psXCInstPri->fpXC_Mux_GetMappingTab(g_pDevice1Instance, pXCArgs->mapping_tab,pXCArgs->length);
#endif
            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }

        case E_XC_CMD_GET_MUX_INPUTSOURCE2VDYMUXPORT:
        {


            pstXC_GET_MUX_INPUTSOURCE2VDYMUXPORT pXCArgs = (pstXC_GET_MUX_INPUTSOURCE2VDYMUXPORT) pArgs;
            pXCArgs->u8ReturnValue = psXCInstPri->fpXC_MUX_MApinputSourceToVDYMuxPORT(pInstance, pXCArgs->u8InputSourceType);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_SET_NR:
        {


            pstXC_SET_NR pXCArgs = (pstXC_SET_NR) pArgs;
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
            USE_OTHER_SC_AS_SUB_WINDOW();
#endif
            psXCInstPri->fpXC_Set_NR(pInstance, pXCArgs->bEn, pXCArgs->eWindow);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_SET_FILM_MODE_PATCH:
        {


            psXCInstPri->fpXC_FilmMode_P(pInstance);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_CHECK_UC_ENABLED:
        {


            pstXC_CHECK_UC_ENABLED pXCArgs = (pstXC_CHECK_UC_ENABLED) pArgs;
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
            USE_OTHER_SC_AS_SUB_WINDOW();
#endif
            pXCArgs->bReturnValue = psXCInstPri->fpXC_GetUCEnabled(pInstance, pXCArgs->eWindow);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_SET_GENERATE_SPECIFIC_TIMING:
        {


            pstXC_SET_GENERATE_SPECIFIC_TIMING pXCArgs = (pstXC_SET_GENERATE_SPECIFIC_TIMING) pArgs;
            psXCInstPri->fpXC_GenSpecificTiming(pInstance, pXCArgs->timingtype);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_GET_DE_BYPASS_MODE:
        {


            pstXC_GET_DE_BYPASS_MODE pXCArgs = (pstXC_GET_DE_BYPASS_MODE) pArgs;
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
            USE_OTHER_SC_AS_SUB_WINDOW();
#endif
            pXCArgs->bReturnValue = psXCInstPri->fpXC_GetDEBypassMode(pInstance, pXCArgs->eWindow);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_GET_DE_WINDOW:
        {


            pstXC_GET_DE_WINDOW pXCArgs = (pstXC_GET_DE_WINDOW) pArgs;
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
            USE_OTHER_SC_AS_SUB_WINDOW();
#endif
            psXCInstPri->fpXC_GetDEWindow(pInstance, pXCArgs->psWin, pXCArgs->eWindow);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_GET_DE_WIN_WIDTH_HEIGHT_IN_DE_BYPASS_MODE:
        {


            pstXC_GET_DE_WIN_WIDTH_HEIGHT_IN_DE_BYPASS_MODE pXCArgs = (pstXC_GET_DE_WIN_WIDTH_HEIGHT_IN_DE_BYPASS_MODE) pArgs;
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
            USE_OTHER_SC_AS_SUB_WINDOW();
#endif
            psXCInstPri->fpXC_GetDEWidthHeightInDEByPassMode(pInstance, pXCArgs->pu16Width, pXCArgs->pu16Height, pXCArgs->eWindow);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_GET_CAPTURE_WINDOW:
        {


            pstXC_GET_CAPTURE_WINDOW pXCArgs = (pstXC_GET_CAPTURE_WINDOW) pArgs;
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
            USE_OTHER_SC_AS_SUB_WINDOW();
#endif
            psXCInstPri->fpXC_GetCaptureWindow(pInstance, pXCArgs->capture_win, pXCArgs->eWindow);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_SET_CAPTURE_WINDOW_VSTART:
        {


            pstXC_SET_CAPTURE_WINDOW_VSTART pXCArgs = (pstXC_SET_CAPTURE_WINDOW_VSTART) pArgs;
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
            USE_OTHER_SC_AS_SUB_WINDOW();
#endif
            psXCInstPri->fpXC_SetCaptureWindowVstart(pInstance, pXCArgs->u16Vstart, pXCArgs->eWindow);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_SET_CAPTURE_WINDOW_HSTART:
        {


            pstXC_SET_CAPTURE_WINDOW_HSTART pXCArgs = (pstXC_SET_CAPTURE_WINDOW_HSTART) pArgs;
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
            USE_OTHER_SC_AS_SUB_WINDOW();
#endif
            psXCInstPri->fpXC_SetCaptureWindowHstart(pInstance, pXCArgs->u16Hstart, pXCArgs->eWindow);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_SET_CAPTURE_WINDOW_VSIZE:
        {


            pstXC_SET_CAPTURE_WINDOW_VSIZE pXCArgs = (pstXC_SET_CAPTURE_WINDOW_VSIZE) pArgs;
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
            USE_OTHER_SC_AS_SUB_WINDOW();
#endif
            psXCInstPri->fpXC_SetCaptureWindowVsize(pInstance, pXCArgs->u16Vsize, pXCArgs->eWindow);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_SET_CAPTURE_WINDOW_HSIZE:
        {


            pstXC_SET_CAPTURE_WINDOW_HSIZE pXCArgs = (pstXC_SET_CAPTURE_WINDOW_HSIZE) pArgs;
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
            USE_OTHER_SC_AS_SUB_WINDOW();
#endif
            psXCInstPri->fpXC_SetCaptureWindowHsize(pInstance, pXCArgs->u16Hsize, pXCArgs->eWindow);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_SET_SOFTWARE_RESET:
        {


            pstXC_SET_SOFTWARE_RESET pXCArgs = (pstXC_SET_SOFTWARE_RESET) pArgs;
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
            USE_OTHER_SC_AS_SUB_WINDOW();
#endif
            psXCInstPri->fpXC_SoftwareReset(pInstance, pXCArgs->u8Reset, pXCArgs->eWindow);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_GET_HFREQX10:
        {


            pstXC_GET_HFREQX10 pXCArgs = (pstXC_GET_HFREQX10) pArgs;
            pXCArgs->u16ReturnValue = psXCInstPri->fpXC_CalculateHFreqx10(pInstance, pXCArgs->u16HPeriod);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_GET_HFREQX1K:
        {


            pstXC_GET_HFREQX1K pXCArgs = (pstXC_GET_HFREQX1K) pArgs;
            pXCArgs->u32ReturnValue = psXCInstPri->fpXC_CalculateHFreqx1K(pInstance, pXCArgs->u16HPeriod);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_GET_VFREQX10:
        {


            pstXC_GET_VFREQX10 pXCArgs = (pstXC_GET_VFREQX10) pArgs;
            pXCArgs->u16ReturnValue = psXCInstPri->fpXC_CalculateVFreqx10(pInstance, pXCArgs->u16HFreq, pXCArgs->u16VTotal);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_GET_VFREQX1K:
        {


            pstXC_GET_VFREQX1K pXCArgs = (pstXC_GET_VFREQX1K) pArgs;
            pXCArgs->u32ReturnValue = psXCInstPri->fpXC_CalculateVFreqx1K(pInstance, pXCArgs->u32HFreqx1K, pXCArgs->u16VTotal);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_GET_ACCURATE_VFREQX1k:
        {


            pstXC_GET_ACCURATE_VFREQX1k pXCArgs = (pstXC_GET_ACCURATE_VFREQX1k) pArgs;
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
            USE_OTHER_SC_AS_SUB_WINDOW();
#endif
            pXCArgs->u32ReturnValue = psXCInstPri->fpXC_GetAccurateVFreqx1K(pInstance, pXCArgs->eWindow);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_SET_INTERRUPT_ATTACH:
        {


            pstXC_SET_INTERRUPT_ATTACH pXCArgs = (pstXC_SET_INTERRUPT_ATTACH) pArgs;
            pXCArgs->bReturnValue = psXCInstPri->fpXC_InterruptAttach(pInstance, pXCArgs->eIntNum, pXCArgs->pIntCb, pXCArgs->pParam);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_SET_INTERRUPT_DEATTACH:
        {


            pstXC_SET_INTERRUPT_DEATTACH pXCArgs = (pstXC_SET_INTERRUPT_DEATTACH) pArgs;
            pXCArgs->bReturnValue = psXCInstPri->fpXC_InterruptDeAttach(pInstance, pXCArgs->eIntNum, pXCArgs->pIntCb, pXCArgs->pParam);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }

        case E_XC_CMD_IS_INTERRUPT_ATTACHED:
        {
            pstXC_INTERRUPT_IS_ATTACHED pXCArgs = (pstXC_INTERRUPT_IS_ATTACHED) pArgs;
            pXCArgs->bReturnValue = psXCInstPri->fpXC_InterruptIsAttached(pInstance, pXCArgs->eIntNum, pXCArgs->pIntCb, pXCArgs->pParam);

            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }

        case E_XC_CMD_SET_DISABLE_INPUTSOURCE:
        {


            pstXC_SET_DISABLE_INPUTSOURCE pXCArgs = (pstXC_SET_DISABLE_INPUTSOURCE) pArgs;
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
            if(!MDrv_XC_GetMWEStatus(pInstance))
            {
                USE_OTHER_SC_AS_SUB_WINDOW();
            }
#endif
            psXCInstPri->fpXC_DisableInputSource(pInstance, pXCArgs->bDisable, pXCArgs->eWindow);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_CHECK_INPUTSOURCE_DISABLED:
        {


            pstXC_CHECK_INPUTSOURCE_DISABLED pXCArgs = (pstXC_CHECK_INPUTSOURCE_DISABLED) pArgs;
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
            USE_OTHER_SC_AS_SUB_WINDOW();
#endif
            pXCArgs->bReturnValue = psXCInstPri->fpXC_IsInputSourceDisabled(pInstance, pXCArgs->eWindow);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_SET_CHANGE_PANELTYPE:
        {


            pstXC_SET_CHANGE_PANELTYPE pXCArgs = (pstXC_SET_CHANGE_PANELTYPE) pArgs;
            psXCInstPri->fpXC_ChangePanelType(pInstance, pXCArgs->pstPanelInfo);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_GET_CURRENT_READBANK:
        {


            pstXC_GET_CURRENT_READBANK pXCArgs = (pstXC_GET_CURRENT_READBANK) pArgs;
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
            USE_OTHER_SC_AS_SUB_WINDOW();
#endif
            pXCArgs->u16ReturnValue = psXCInstPri->fpXC_GetCurrentReadBank(pInstance, pXCArgs->eWindow);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_GET_CURRENT_WRITEBANK:
        {


            pstXC_GET_CURRENT_WRITEBANK pXCArgs = (pstXC_GET_CURRENT_WRITEBANK) pArgs;
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
            USE_OTHER_SC_AS_SUB_WINDOW();
#endif
            pXCArgs->u16ReturnValue = psXCInstPri->fpXC_GetCurrentWriteBank(pInstance, pXCArgs->eWindow);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_SET_AUTO_PRESCALING:
        {


            pstXC_SET_AUTO_PRESCALING pXCArgs = (pstXC_SET_AUTO_PRESCALING) pArgs;
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
            USE_OTHER_SC_AS_SUB_WINDOW();
#endif
            psXCInstPri->fpXC_SetAutoPreScaling(pInstance, pXCArgs->bEnable, pXCArgs->eWindow);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_GET_VSYNC_WIDTH:
        {


            pstXC_GET_VSYNC_WIDTH pXCArgs = (pstXC_GET_VSYNC_WIDTH) pArgs;
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
            USE_OTHER_SC_AS_SUB_WINDOW();
#endif
            pXCArgs->u8ReturnValue = psXCInstPri->fpXC_GetVSyncWidth(pInstance, pXCArgs->eWindow);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_SET_GOP_ENABLE:
        {


            pstXC_SET_GOP_ENABLE pXCArgs = (pstXC_SET_GOP_ENABLE) pArgs;
            pXCArgs->bReturnValue = psXCInstPri->fpXC_set_GOP_Enable(pInstance, pXCArgs->MaxGOP, pXCArgs->UseNum, pXCArgs->u8MuxNum, pXCArgs->bEnable);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_SET_SELECT_IP_FOR_GOP:
        {


            pstXC_SET_SELECT_IP_FOR_GOP pXCArgs = (pstXC_SET_SELECT_IP_FOR_GOP) pArgs;
            psXCInstPri->fpXC_ip_sel_for_gop(pInstance, pXCArgs->u8MuxNum, pXCArgs->ipSelGop);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_SET_VOP_NEW_BLENDING_LEVEL:
        {


            psXCInstPri->fpXC_SetVOPNBL(pInstance);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_GET_DESTINATION_INFO:
        {


            pstXC_GET_DESTINATION_INFO pXCArgs = (pstXC_GET_DESTINATION_INFO) pArgs;
            pXCArgs->bReturnValue = psXCInstPri->fpXC_GetDstInfo(pInstance, pXCArgs->pDstInfo, pXCArgs->u32SizeofDstInfo, pXCArgs->XCDstType);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_SET_FDMASK_BYWIN:
        {


            pstXC_SET_FDMASK_BYWIN pXCArgs = (pstXC_SET_FDMASK_BYWIN) pArgs;
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
            USE_OTHER_SC_AS_SUB_WINDOW();
#endif
            psXCInstPri->fpXC_Set_FD_Mask_ByWin(pInstance, pXCArgs->bEnable, pXCArgs->eWindow);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_GET_FDMASK_BYWIN:
        {


            pstXC_GET_FDMASK_BYWIN pXCArgs = (pstXC_GET_FDMASK_BYWIN) pArgs;
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
            USE_OTHER_SC_AS_SUB_WINDOW();
#endif
            pXCArgs->bReturnValue = psXCInstPri->fpXC_Get_FD_Mask_ByWin(pInstance, pXCArgs->eWindow);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_SET_IP1_TESTPATTERN:
        {


            pstXC_SET_IP1_TESTPATTERN pXCArgs = (pstXC_SET_IP1_TESTPATTERN) pArgs;
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
            USE_OTHER_SC_AS_SUB_WINDOW();
#endif
            psXCInstPri->fpXC_SetIP1TestPattern(pInstance, pXCArgs->u8Enable, pXCArgs->u6Pattern_type, pXCArgs->eWindow);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }
#ifdef UFO_XC_TEST_PATTERN
        case E_XC_CMD_SET_TESTPATTERN:
        {


            pstXC_SET_TESTPATTERN pXCArgs = (pstXC_SET_TESTPATTERN) pArgs;
            psXCInstPri->fpXC_GenerateTestPattern(pInstance, pXCArgs->ePatternMode,pXCArgs->para, pXCArgs->u16Length);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }
#endif
        case E_XC_CMD_SET_INIT_IP_FOR_INTERNAL_TIMING:
        {


            pstXC_SET_INIT_IP_FOR_INTERNAL_TIMING pXCArgs = (pstXC_SET_INIT_IP_FOR_INTERNAL_TIMING) pArgs;
            psXCInstPri->fpXC_InitIPForInternalTiming(pInstance, pXCArgs->timingtype);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_SET_IPMUX:
        {


            pstXC_SET_IPMUX pXCArgs = (pstXC_SET_IPMUX) pArgs;
            psXCInstPri->fpXC_SetIPMux(pInstance, pXCArgs->u8Val);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_CHECK_HSYNC_ACTIVE:
        {


            pstXC_CHECK_HSYNC_ACTIVE pXCArgs = (pstXC_CHECK_HSYNC_ACTIVE) pArgs;
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
            USE_OTHER_SC_AS_SUB_WINDOW();
#endif
            pXCArgs->bReturnValue = psXCInstPri->fpXC_Is_H_Sync_Active(pInstance, pXCArgs->eWindow);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_CHECK_VSYNC_ACTIVE:
        {


            pstXC_CHECK_VSYNC_ACTIVE pXCArgs = (pstXC_CHECK_VSYNC_ACTIVE) pArgs;
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
            USE_OTHER_SC_AS_SUB_WINDOW();
#endif
            pXCArgs->bReturnValue = psXCInstPri->fpXC_Is_V_Sync_Active(pInstance, pXCArgs->eWindow);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_GET_AUTO_POSITION_WINDOW:
        {


            pstXC_GET_AUTO_POSITION_WINDOW pXCArgs = (pstXC_GET_AUTO_POSITION_WINDOW) pArgs;
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
            USE_OTHER_SC_AS_SUB_WINDOW();
#endif
            pXCArgs->bReturnValue = psXCInstPri->fpXC_GetAutoPositionWindow(pInstance, pXCArgs->u8ValidData, pXCArgs->eWindow, pXCArgs->pstAutoPositionWindow);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_SET_FRAMEBUFFER_ADDRESS:
        {


            pstXC_SET_FRAMEBUFFER_ADDRESS pXCArgs = (pstXC_SET_FRAMEBUFFER_ADDRESS) pArgs;
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
            USE_OTHER_SC_AS_SUB_WINDOW();
#endif
            psXCInstPri->fpXC_SetFrameBufferAddress(pInstance, pXCArgs->u32FBAddress, pXCArgs->u32FBSize, pXCArgs->eWindow);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }

        case E_XC_CMD_SET_FRCM_FRAMEBUFFER_ADDRESS:
        {

            pstXC_SET_FRAMEBUFFER_ADDRESS pXCArgs = (pstXC_SET_FRAMEBUFFER_ADDRESS) pArgs;
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
            USE_OTHER_SC_AS_SUB_WINDOW();
#endif
            psXCInstPri->fpXC_SetFRCMFrameBufferAddress(pInstance, pXCArgs->u32FBAddress, pXCArgs->u32FBSize, pXCArgs->eWindow);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }
        case E_XC_CMD_CHECK_FRAMEBUFFER_ENOUGH_FOR_CUSSCALING:
        {


            pstXC_CHECK_FRAMEBUFFER_ENOUGH_FOR_CUSSCALING pXCArgs = (pstXC_CHECK_FRAMEBUFFER_ENOUGH_FOR_CUSSCALING) pArgs;
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
            USE_OTHER_SC_AS_SUB_WINDOW();
#endif
            pXCArgs->bReturnValue = psXCInstPri->fpXC_IsFrameBufferEnoughForCusScaling(pInstance, pXCArgs->pstXC_SetWin_Info, pXCArgs->eWindow);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_SET_SCALER_MEMORY_REQUEST:
        {


            pstXC_SET_SCALER_MEMORY_REQUEST pXCArgs = (pstXC_SET_SCALER_MEMORY_REQUEST) pArgs;
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
            USE_OTHER_SC_AS_SUB_WINDOW();
#endif
            pXCArgs->eReturnValue = psXCInstPri->fpXC_SetScalerMemoryRequest(pInstance, pXCArgs->bEnable, pXCArgs->eRequestType, pXCArgs->eWindow);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_GET_PIXEL_DATA:
        {


            pstXC_GET_PIXEL_DATA pXCArgs = (pstXC_GET_PIXEL_DATA) pArgs;
            psXCInstPri->fpXC_Get_PixelData(pInstance, pXCArgs->u16CorX, pXCArgs->u16CorY, pXCArgs->pixel);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_GET_AVAILABLE_SIZE:
        {


            pstXC_GET_AVAILABLE_SIZE pXCArgs = (pstXC_GET_AVAILABLE_SIZE) pArgs;
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
            USE_OTHER_SC_AS_SUB_WINDOW();
#endif
            pXCArgs->u32ReturnValue = psXCInstPri->fpXC_GetAvailableSize(pInstance, pXCArgs->eWindow, pXCArgs->u8FBNum, pXCArgs->u32InputSize);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_SET_FRAME_COLOR:
        {


            pstXC_SET_FRAME_COLOR pXCArgs = (pstXC_SET_FRAME_COLOR) pArgs;
            psXCInstPri->fpXC_SetFrameColor(pInstance, pXCArgs->u32aRGB);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_SET_DISPLAY_WINDOW_COLOR:
        {


            pstXC_SET_DISPLAY_WINDOW_COLOR pXCArgs = (pstXC_SET_DISPLAY_WINDOW_COLOR) pArgs;
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
            USE_OTHER_SC_AS_SUB_WINDOW();
#endif
            psXCInstPri->fpXC_SetDispWindowColor(pInstance, pXCArgs->u8Color, pXCArgs->eWindow);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }

        case E_XC_CMD_POLL_EVENT:
        {
            pstXC_POLL_EVENT pXCArgs = (pstXC_POLL_EVENT) pArgs;
            pXCArgs->bReturnValue = psXCInstPri->fpXC_PollEvent(pInstance);

            u32Return = UTOPIA_STATUS_SUCCESS;

            break;
        }

        case E_XC_CMD_CANCEL_POLL_EVENT:
        {
            pstXC_CANCEL_POLL_EVENT pXCArgs = (pstXC_CANCEL_POLL_EVENT) pArgs;
            pXCArgs->bReturnValue = psXCInstPri->fpXC_Cancel_PollEvent(pInstance);

            u32Return = UTOPIA_STATUS_SUCCESS;

            break;
        }

        case E_XC_CMD_EVENT_TRIGGER:
        {
            pstXC_EventTrigger pXCArgs = (pstXC_EventTrigger) pArgs;
            pXCArgs->eReturnValue = psXCInstPri->fpXC_EventTrigger(pInstance, pXCArgs->pstXCEventMisc);

            u32Return = UTOPIA_STATUS_SUCCESS;

            break;
        }

        case E_XC_CMD_GET_SUPPORT_SOURCE_TO_VE:
        {


            pstXC_GET_SUPPORT_SOURCE_TO_VE pXCArgs = (pstXC_GET_SUPPORT_SOURCE_TO_VE) pArgs;
            pXCArgs->eReturnValue = psXCInstPri->fpXC_SupportSourceToVE(pInstance, pXCArgs->pOutputCapability);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_SET_OUTPUT_CAPTURE:
        {


            pstXC_SET_OUTPUT_CAPTURE pXCArgs = (pstXC_SET_OUTPUT_CAPTURE) pArgs;
            pXCArgs->eReturnValue = psXCInstPri->fpXC_SetOutputCapture(pInstance, pXCArgs->bEnable, pXCArgs->eSourceToVE);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_SET_GAMMA_ONOFF:
        {


            pstXC_SET_GAMMA_ONOFF pXCArgs = (pstXC_SET_GAMMA_ONOFF) pArgs;
            psXCInstPri->fpXC_SetGammaOnOff(pInstance, pXCArgs->bEnable);
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
            CHECK_DEVICE1_INSTANCE();
            psXCInstPri->fpXC_SetGammaOnOff(g_pDevice1Instance, pXCArgs->bEnable);
#endif

            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_SET_PREGAMMA_GAIN:
        {


            pstXC_SET_PREGAMMA_GAIN pXCArgs = (pstXC_SET_PREGAMMA_GAIN) pArgs;
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
            USE_OTHER_SC_AS_SUB_WINDOW();
#endif
            psXCInstPri->fpXC_SetPreGammaGain(pInstance, pXCArgs->eWindow, pXCArgs->eVop_Channel, pXCArgs->u16Val);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_SET_PREGAMMA_OFFSET:
        {


            pstXC_SET_PREGAMMA_OFFSET pXCArgs = (pstXC_SET_PREGAMMA_OFFSET) pArgs;
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
            USE_OTHER_SC_AS_SUB_WINDOW();
#endif
            psXCInstPri->fpXC_SetPreGammaOffset(pInstance, pXCArgs->eWindow, pXCArgs->eVop_Channel, pXCArgs->u16Val);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_SET_PANEL_TIMING:
        {


            pstXC_SET_PANEL_TIMING pXCArgs = (pstXC_SET_PANEL_TIMING) pArgs;
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
            USE_OTHER_SC_AS_SUB_WINDOW();
#endif
            psXCInstPri->fpXC_SetPanelTiming(pInstance, pXCArgs->pTimingInfo, pXCArgs->eWindow);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_SET_OUTPUT_TIMING_MODE:
        {


            pstXC_SET_OUTPUT_TIMING_MODE pXCArgs = (pstXC_SET_OUTPUT_TIMING_MODE) pArgs;
            psXCInstPri->fpXC_SetOutTimingMode(pInstance, pXCArgs->enPnl_Out_Timing_Mode);
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
            CHECK_DEVICE1_INSTANCE();
            psXCInstPri->fpXC_SetOutTimingMode(g_pDevice1Instance, pXCArgs->enPnl_Out_Timing_Mode);
#endif

            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_SET_FREERUN_TIMING:
        {


            psXCInstPri->fpXC_SetFreeRunTiming(pInstance);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_SET_CUSTOMER_SYNC_INFO:
        {


            pstXC_SET_CUSTOMER_SYNC_INFO pXCArgs = (pstXC_SET_CUSTOMER_SYNC_INFO) pArgs;
            pXCArgs->bReturnValue = psXCInstPri->fpXC_Set_CustomerSyncInfo(pInstance,
                                    pXCArgs->u16FrontPorch,
                                    pXCArgs->u16SyncWidth,
                                    pXCArgs->u16OutputVfreq);

            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_GET_WAIT_FPLL_DONE:
        {


            pstXC_GET_WAIT_FPLL_DONE pXCArgs = (pstXC_GET_WAIT_FPLL_DONE) pArgs;
            pXCArgs->bReturnValue = psXCInstPri->fpXC_WaitFPLLDone(pInstance);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_GET_OUTPUT_VFREQX100:
        {


            pstXC_GET_OUTPUT_VFREQX100 pXCArgs = (pstXC_GET_OUTPUT_VFREQX100) pArgs;
            pXCArgs->u16ReturnValue = psXCInstPri->fpXC_GetOutputVFreqX100(pInstance);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }

        case E_XC_CMD_GET_OP1_OUTPUT_VFREQX100:
        {


            pstXC_GET_OP1_OUTPUT_VFREQX100 pXCArgs = (pstXC_GET_OP1_OUTPUT_VFREQX100) pArgs;
            pXCArgs->u16ReturnValue = psXCInstPri->fpXC_GetOP1OutputVFreqX100(pInstance);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_CHECK_FRAMELOCK:
        {


            pstXC_CHECK_FRAMELOCK pXCArgs = (pstXC_CHECK_FRAMELOCK) pArgs;
            pXCArgs->bReturnValue = psXCInstPri->fpXC_FrameLockCheck(pInstance);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_SET_CUSTOMIZE_FRC_TABLE:
        {


            pstXC_SET_CUSTOMIZE_FRC_TABLE pXCArgs = (pstXC_SET_CUSTOMIZE_FRC_TABLE) pArgs;
            pXCArgs->eReturnValue = psXCInstPri->fpXC_CustomizeFRCTable(pInstance, pXCArgs->stFRCTable);
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
            CHECK_DEVICE1_INSTANCE();
            pXCArgs->eReturnValue = psXCInstPri->fpXC_CustomizeFRCTable(g_pDevice1Instance, pXCArgs->stFRCTable);
#endif

            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_SET_OUTPUT_FRAME_CONTROL:
        {


            pstXC_SET_OUTPUT_FRAME_CONTROL pXCArgs = (pstXC_SET_OUTPUT_FRAME_CONTROL) pArgs;
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
            USE_OTHER_SC_AS_SUB_WINDOW();
#endif
            pXCArgs->eReturnValue = psXCInstPri->fpXC_OutputFrameCtrl(pInstance, pXCArgs->bEnable, pXCArgs->pstOutFrameInfo, pXCArgs->eWindow);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_SET_FPLL_FSM_ENABLE:
        {


            pstXC_SET_FPLL_FSM_ENABLE pXCArgs = (pstXC_SET_FPLL_FSM_ENABLE) pArgs;
            psXCInstPri->fpSC_Enable_FPLL_FSM(pInstance, pXCArgs->bTrue);
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
            CHECK_DEVICE1_INSTANCE();
            psXCInstPri->fpSC_Enable_FPLL_FSM(g_pDevice1Instance, pXCArgs->bTrue);
#endif

            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_SET_FORCE_FREERUN:
        {


            pstXC_SET_FORCE_FREERUN pXCArgs = (pstXC_SET_FORCE_FREERUN) pArgs;
            psXCInstPri->fpSC_ForceFreerun(pInstance, pXCArgs->bEnable);
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
            CHECK_DEVICE1_INSTANCE();
            psXCInstPri->fpSC_ForceFreerun(g_pDevice1Instance, pXCArgs->bEnable);
#endif

            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_CHECK_FORCE_FREERUN:
        {


            pstXC_CHECK_FORCE_FREERUN pXCArgs = (pstXC_CHECK_FORCE_FREERUN) pArgs;
            pXCArgs->bReturnValue = psXCInstPri->fpSC_IsForceFreerun(pInstance);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_SET_FREERUN_FREQ:
        {


            pstXC_SET_FREERUN_FREQ pXCArgs = (pstXC_SET_FREERUN_FREQ) pArgs;
            psXCInstPri->fpSC_SetFreerunVFreq(pInstance, pXCArgs->VFreq);
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
            CHECK_DEVICE1_INSTANCE();
            psXCInstPri->fpSC_SetFreerunVFreq(g_pDevice1Instance, pXCArgs->VFreq);
#endif
            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_SET_EXPANEL_INFO:
        {


            pstXC_SET_EXPANEL_INFO pXCArgs = (pstXC_SET_EXPANEL_INFO) pArgs;
            pXCArgs->bReturnValue = psXCInstPri->fpXC_SetExPanelInfo(pInstance, pXCArgs->bEnable, pXCArgs->pPanelInfoEx);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_SET_FPLL_THRESHMODE:
        {

            pstXC_SET_FPLL_THRESHMODE pXCArgs = (pstXC_SET_FPLL_THRESHMODE) pArgs;
            psXCInstPri->fpXC_Enable_FPLL_Thresh_Mode(pInstance, pXCArgs->bEnable);
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
            CHECK_DEVICE1_INSTANCE();
            psXCInstPri->fpXC_Enable_FPLL_Thresh_Mode(g_pDevice1Instance, pXCArgs->bEnable);
#endif

            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }

        case E_XC_CMD_GET_FPLL_THRESHMODE:
        {
            pstXC_GET_FPLL_THRESHMODE pXCArgs = (pstXC_GET_FPLL_THRESHMODE) pArgs;
            pXCArgs->bReturnValue = psXCInstPri->fpXC_Get_FPLL_Thresh_Mode(pInstance);

            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }

        case E_XC_CMD_SET_IPAUTONOSIGNAL:
        {


            pstXC_SET_IPAUTONOSIGNAL pXCArgs = (pstXC_SET_IPAUTONOSIGNAL) pArgs;
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
            USE_OTHER_SC_AS_SUB_WINDOW();
#endif
            pXCArgs->eReturnValue = psXCInstPri->fpXC_EnableIPAutoNoSignal(pInstance, pXCArgs->bEnable, pXCArgs->eWindow);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_GET_IPAUTONOSIGNAL:
        {

            pstXC_GET_IPAUTONOSIGNAL pXCArgs = (pstXC_GET_IPAUTONOSIGNAL) pArgs;
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
            USE_OTHER_SC_AS_SUB_WINDOW();
#endif
            pXCArgs->bReturnValue = psXCInstPri->fpXC_GetIPAutoNoSignal(pInstance, pXCArgs->eWindow);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_SET_IPAUTOCOAST:
        {


            pstXC_SET_IPAUTOCOAST pXCArgs = (pstXC_SET_IPAUTOCOAST) pArgs;
            psXCInstPri->fpXC_EnableIPAutoCoast(pInstance, pXCArgs->bEnable);
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
            CHECK_DEVICE1_INSTANCE();
            psXCInstPri->fpXC_EnableIPAutoCoast(g_pDevice1Instance, pXCArgs->bEnable);
#endif

            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_SET_IPCOAST_DEBOUNCE_ENABLE:
        {


            psXCInstPri->fpXC_EnableIPCoastDebounce(pInstance);
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
            CHECK_DEVICE1_INSTANCE();
            psXCInstPri->fpXC_EnableIPCoastDebounce(g_pDevice1Instance);
#endif

            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_SET_CLEAR_IPCOAST_STATUS:
        {


            psXCInstPri->fpXC_ClearIPCoastStatus(pInstance);
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
            CHECK_DEVICE1_INSTANCE();
            psXCInstPri->fpXC_ClearIPCoastStatus(g_pDevice1Instance);
#endif

            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_SET_FPLL_MANUAL_SETTING_ENABLE:
        {


            pstXC_SET_FPLL_MANUAL_SETTING_ENABLE pXCArgs = (pstXC_SET_FPLL_MANUAL_SETTING_ENABLE) pArgs;
            psXCInstPri->fpXC_EnableFpllManualSetting(pInstance, pXCArgs->bEnable);
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
            CHECK_DEVICE1_INSTANCE();
            psXCInstPri->fpXC_EnableFpllManualSetting(g_pDevice1Instance, pXCArgs->bEnable);
#endif

            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_SET_FPLL_BOUNDARY_TEST:
        {


            pstXC_SET_FPLL_BOUNDARY_TEST pXCArgs = (pstXC_SET_FPLL_BOUNDARY_TEST) pArgs;
            psXCInstPri->fpXC_FpllBoundaryTest(pInstance, pXCArgs->u32FpllSetOffset, pXCArgs->u16TestCnt);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_SET_OFFLINE_DETECTION:
        {


            pstXC_SET_OFFLINE_DETECTION pXCArgs = (pstXC_SET_OFFLINE_DETECTION) pArgs;
            psXCInstPri->fpXC_SetOffLineDetection(pInstance, pXCArgs->u8InputSourceType);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_GET_OFFLINE_DETECTION:
        {


            pstXC_GET_OFFLINE_DETECTION pXCArgs = (pstXC_GET_OFFLINE_DETECTION) pArgs;
            pXCArgs->u8ReturnValue = psXCInstPri->fpXC_GetOffLineDetection(pInstance, pXCArgs->u8InputSourceType);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_SET_OFFLINE_SOG_THRESHOLD:
        {


            pstXC_SET_OFFLINE_SOG_THRESHOLD pXCArgs = (pstXC_SET_OFFLINE_SOG_THRESHOLD) pArgs;
            pXCArgs->bReturnValue = psXCInstPri->fpXC_SetOffLineSogThreshold(pInstance, pXCArgs->u8Threshold);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_SET_OFFLINE_SOG_BW:
        {


            pstXC_SET_OFFLINE_SOG_BW pXCArgs = (pstXC_SET_OFFLINE_SOG_BW) pArgs;
            pXCArgs->bReturnValue = psXCInstPri->fpXC_SetOffLineSogBW(pInstance, pXCArgs->u8BW);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_SET_OFFLINE_INIT:
        {


            pstXC_SET_OFFLINE_INIT pXCArgs = (pstXC_SET_OFFLINE_INIT) pArgs;
            pXCArgs->bReturnValue = psXCInstPri->fpXC_OffLineInit(pInstance);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_GET_OFFLINE_INFO:
        {
            pstXC_GET_OFFLINE_INFO pXCArgs = (pstXC_GET_OFFLINE_INFO) pArgs;
            pXCArgs->u16ReturnValue = psXCInstPri->fpXC_GetOffLineInfo(pInstance, pXCArgs->u8InputSourceType);

            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_SET_EXTRA_FETCH_ADV_LINE:
        {


            pstXC_SET_EXTRA_FETCH_ADV_LINE pXCArgs = (pstXC_SET_EXTRA_FETCH_ADV_LINE) pArgs;
            psXCInstPri->fpXC_Set_Extra_fetch_adv_line(pInstance, pXCArgs->u8val);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_SET_VGA_SOG_ENABLE:
        {


            pstXC_SET_VGA_SOG_ENABLE pXCArgs = (pstXC_SET_VGA_SOG_ENABLE) pArgs;
            pXCArgs->eReturnValue = psXCInstPri->fpXC_SetVGASogEn(pInstance, pXCArgs->bVGASogEn);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_SET_WINDOW_ENABLE:
        {


            pstXC_SET_WINDOW_ENABLE pXCArgs = (pstXC_SET_WINDOW_ENABLE) pArgs;
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
            if(!MDrv_XC_GetMWEStatus(pInstance))
            {
                USE_OTHER_SC_AS_SUB_WINDOW();
            }
#endif
            psXCInstPri->fpXC_EnableWindow(pInstance, pXCArgs->bEnable, pXCArgs->eWindow);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_CHECK_SUBWINDOW_ENABLE:
        {


            pstXC_CHECK_SUBWINDOW_ENABLE pXCArgs = (pstXC_CHECK_SUBWINDOW_ENABLE) pArgs;
#if (PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB == 0)
            pXCArgs->bReturnValue = psXCInstPri->fpXC_Is_SubWindowEanble(pInstance);
#else
            if(!MDrv_XC_GetMWEStatus(pInstance))
            {
                CHECK_DEVICE1_INSTANCE();
                pXCArgs->bReturnValue = psXCInstPri->fpXC_Is_SubWindowEanble(g_pDevice1Instance);
            }
            else
            {
                pXCArgs->bReturnValue = psXCInstPri->fpXC_Is_SubWindowEanble(pInstance);
            }
#endif

            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_SET_BORDER_FORMAT:
        {


            pstXC_SET_BORDER_FORMAT pXCArgs = (pstXC_SET_BORDER_FORMAT) pArgs;
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
            USE_OTHER_SC_AS_SUB_WINDOW();
#endif
            psXCInstPri->fpXC_SetBorderFormat(pInstance,
                                              pXCArgs->u8Left,
                                              pXCArgs->u8Right,
                                              pXCArgs->u8Up,
                                              pXCArgs->u8Down,
                                              pXCArgs->u8color,
                                              pXCArgs->eWindow);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_SET_BORDER_ENABLE:
        {


            pstXC_SET_BORDER_ENABLE pXCArgs = (pstXC_SET_BORDER_ENABLE) pArgs;
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
            USE_OTHER_SC_AS_SUB_WINDOW();
#endif
            psXCInstPri->fpXC_EnableBorder(pInstance, pXCArgs->bEnable, pXCArgs->eWindow);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_SET_ZORDER_MAINWINDOW_FIRST:
        {
            pstXC_SET_ZORDER_MAINWINDOW_FIRST pXCArgs = (pstXC_SET_ZORDER_MAINWINDOW_FIRST) pArgs;
            psXCInstPri->fpXC_ZorderMainWindowFirst(pInstance, pXCArgs->bMainFirst);
            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_SET_PQ_LOADFUNCTION:
        {


            pstXC_SET_PQ_LOADFUNCTION pXCArgs = (pstXC_SET_PQ_LOADFUNCTION) pArgs;
#ifdef MSOS_TYPE_LINUX_KERNEL
            stXC_SET_PQ_LOADFUNCTION stXCArgs;
            if(FALSE == MsOS_KernelMemoryRegionCheck(pXCArgs))
            {
                if (0 == copy_from_user(&stXCArgs,(void**)pXCArgs ,sizeof(stXCArgs)))
                {
                    psXCInstPri->fpXC_PQ_LoadFunction(pInstance, stXCArgs.function_Info, stXCArgs.u32InitDataLen);
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
                    pXCArgs = &stXCArgs;
#endif
                }
                else
                {
                    return UTOPIA_STATUS_FAIL;
                }
            }
            else
            {
                psXCInstPri->fpXC_PQ_LoadFunction(pInstance, pXCArgs->function_Info, pXCArgs->u32InitDataLen);
            }
#else
            psXCInstPri->fpXC_PQ_LoadFunction(pInstance, pXCArgs->function_Info, pXCArgs->u32InitDataLen);
#endif
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
            CHECK_DEVICE1_INSTANCE();
            psXCInstPri->fpXC_PQ_LoadFunction(g_pDevice1Instance, pXCArgs->function_Info, pXCArgs->u32InitDataLen);
#endif

            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_CHECK_H_NONLINEAR_SCALING:
        {


            pstXC_CHECK_H_NONLINEAR_SCALING pXCArgs = (pstXC_CHECK_H_NONLINEAR_SCALING) pArgs;
            pXCArgs->bReturnValue = psXCInstPri->fpXC_Check_HNonLinearScaling(pInstance);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_SET_EUROHDTV_SUPPORT_ENABLE:
        {


            pstXC_SET_EUROHDTV_SUPPORT_ENABLE pXCArgs = (pstXC_SET_EUROHDTV_SUPPORT_ENABLE) pArgs;
            psXCInstPri->fpXC_EnableEuroHdtvSupport(pInstance, pXCArgs->bEnable);
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
            CHECK_DEVICE1_INSTANCE();
            psXCInstPri->fpXC_EnableEuroHdtvSupport(g_pDevice1Instance, pXCArgs->bEnable);
#endif
            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_SET_EUROHDTV_DETECTION_ENABLE:
        {


            pstXC_SET_EUROHDTV_DETECTION_ENABLE pXCArgs = (pstXC_SET_EUROHDTV_DETECTION_ENABLE) pArgs;
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
            USE_OTHER_SC_AS_SUB_WINDOW();
#endif
            psXCInstPri->fpXC_EnableEuroHdtvDetection(pInstance, pXCArgs->bEnable, pXCArgs->eWindow);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_READBYTE:
        {


            pstXC_READBYTE pXCArgs = (pstXC_READBYTE) pArgs;
            pXCArgs->u8ReturnValue = psXCInstPri->fpXC_ReadByte(pInstance, pXCArgs->u32Reg);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_WRITEBYTE:
        {


            pstXC_WRITEBYTE pXCArgs = (pstXC_WRITEBYTE) pArgs;
            psXCInstPri->fpXC_WriteByte(pInstance, pXCArgs->u32Reg, pXCArgs->u8Val);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_WRITEBYTEMSK:
        {

            pstXC_WRITEBYTEMSK pXCArgs = (pstXC_WRITEBYTEMSK) pArgs;
            psXCInstPri->fpXC_WriteByteMask(pInstance, pXCArgs->u32Reg, pXCArgs->u8Val, pXCArgs->u8Msk);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_WRITE2BYTEMSK:
        {


            pstXC_WRITE2BYTEMSK pXCArgs = (pstXC_WRITE2BYTEMSK) pArgs;
            psXCInstPri->fpXC_Write2ByteMask(pInstance, pXCArgs->u32Reg, pXCArgs->u16val, pXCArgs->u16Mask);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_W2BYTE:
        {


            pstXC_W2BYTE pXCArgs = (pstXC_W2BYTE) pArgs;
            psXCInstPri->fpXC_W2BYTE(pInstance, pXCArgs->u32Reg, pXCArgs->u16Val);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_R2BYTE:
        {


            pstXC_R2BYTE pXCArgs = (pstXC_R2BYTE) pArgs;
            pXCArgs->u16ReturnValue = psXCInstPri->fpXC_R2BYTE(pInstance, pXCArgs->u32Reg);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_W4BYTE:
        {


            pstXC_W4BYTE pXCArgs = (pstXC_W4BYTE) pArgs;
            psXCInstPri->fpXC_W4BYTE(pInstance, pXCArgs->u32Reg, pXCArgs->u32Val);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_R4BYTE:
        {


            pstXC_R4BYTE pXCArgs = (pstXC_R4BYTE) pArgs;
            pXCArgs->u32ReturnValue = psXCInstPri->fpXC_R4BYTE(pInstance, pXCArgs->u32Reg);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_R2BYTEMSK:
        {


            pstXC_R2BYTEMSK pXCArgs = (pstXC_R2BYTEMSK) pArgs;
            pXCArgs->u16ReturnValue = psXCInstPri->fpXC_R2BYTEMSK(pInstance, pXCArgs->u32Reg, pXCArgs->u16mask);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_W2BYTEMSK:
        {


            pstXC_W2BYTEMSK pXCArgs = (pstXC_W2BYTEMSK) pArgs;
            psXCInstPri->fpXC_W2BYTEMSK(pInstance, pXCArgs->u32Reg, pXCArgs->u16Val, pXCArgs->u16mask);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_MLOAD_INIT:
        {
            pstXC_MLOAD_INIT pXCArgs = (pstXC_MLOAD_INIT) pArgs;
            MS_U32 u32MloadBufLen = pXCArgs->u32BufByteLen;
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
            u32MloadBufLen = pXCArgs->u32BufByteLen/2;
#endif
            psXCInstPri->fpXC_MLoad_Init(pInstance, pXCArgs->PhyAddr, u32MloadBufLen);
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
            CHECK_DEVICE1_INSTANCE();
            psXCInstPri->fpXC_MLoad_Init(g_pDevice1Instance, pXCArgs->PhyAddr + u32MloadBufLen, u32MloadBufLen);
#endif
            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_MLOAD_ENABLE:
        {


            pstXC_MLOAD_ENABLE pXCArgs = (pstXC_MLOAD_ENABLE) pArgs;
            psXCInstPri->fpXC_MLoad_Enable(pInstance, pXCArgs->bEnable);
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
            CHECK_DEVICE1_INSTANCE();
            psXCInstPri->fpXC_MLoad_Enable(g_pDevice1Instance, pXCArgs->bEnable);
#endif

            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }
        case E_XC_CMD_MLOAD_CUS_INIT:
        {
            pstXC_MLOAD_CUS_INIT pXCArgs = (pstXC_MLOAD_CUS_INIT) pArgs;

            psXCInstPri->fpXC_MLoad_Cus_Init(pInstance, pXCArgs->eMloadSelect, pXCArgs->PhyAddr, pXCArgs->u32BufByteLen);

            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }

        case E_XC_CMD_MLOAD_CUS_ENABLE:
        {
            pstXC_MLOAD_CUS_ENABLE pXCArgs = (pstXC_MLOAD_CUS_ENABLE) pArgs;
            psXCInstPri->fpXC_MLoad_Cus_Enable(pInstance, pXCArgs->eMloadSelect, pXCArgs->bEnable);
            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }
        case E_XC_CMD_MLOAD_GETSTATUS:
        {


            pstXC_MLOAD_GETSTATUS pXCArgs = (pstXC_MLOAD_GETSTATUS) pArgs;
            pXCArgs->eReturnValue = psXCInstPri->fpXC_MLoad_GetStatus(pInstance);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_MLOAD_WRITE_CMD_AND_FIRE:
        {


            pstXC_MLOAD_WRITE_CMD_AND_FIRE pXCArgs = (pstXC_MLOAD_WRITE_CMD_AND_FIRE) pArgs;
            pXCArgs->bReturnValue = psXCInstPri->fpXC_MLoad_WriteCmd_And_Fire(pInstance, pXCArgs->u32Addr, pXCArgs->u16Data, pXCArgs->u16Mask);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_MLOAD_WRITE_CMDS_AND_FIRE:
        {


            pstXC_MLOAD_WRITE_CMDS_AND_FIRE pXCArgs = (pstXC_MLOAD_WRITE_CMDS_AND_FIRE) pArgs;
            pXCArgs->bReturnValue = psXCInstPri->fpXC_MLoad_WriteCmds_And_Fire(pInstance, pXCArgs->pu32Addr, pXCArgs->pu16Data, pXCArgs->pu16Mask, pXCArgs->u16CmdCnt);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }

        case E_XC_CMD_MLOAD_COMBINE:
        {
            pstXC_MLOAD_COMBINE pXCArgs = (pstXC_MLOAD_COMBINE) pArgs;
            pXCArgs->eReturnValue = psXCInstPri->fpXC_MLoad_Combine_Enable(pInstance, pXCArgs->bEn);
            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }

        case E_XC_CMD_MLG_INIT:
        {


            pstXC_MLG_INIT pXCArgs = (pstXC_MLG_INIT) pArgs;
            psXCInstPri->fpXC_MLG_Init(pInstance, pXCArgs->PhyAddr, pXCArgs->u32BufByteLen);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_MLG_ENABLE:
        {


            pstXC_MLG_ENABLE pXCArgs = (pstXC_MLG_ENABLE) pArgs;
            psXCInstPri->fpXC_MLG_Enable(pInstance, pXCArgs->bEnable);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_MLG_GET_CAPS:
        {


            pstXC_MLG_GET_CAPS pXCArgs = (pstXC_MLG_GET_CAPS) pArgs;
            pXCArgs->bReturnValue = psXCInstPri->fpXC_MLG_GetCaps(pInstance);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_MLG_GETSTATUS:
        {


            pstXC_MLG_GETSTATUS pXCArgs = (pstXC_MLG_GETSTATUS) pArgs;
            pXCArgs->eReturnValue = psXCInstPri->fpXC_MLG_GetStatus(pInstance);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_SET_OSD2VEMODE:
        {


            pstXC_SET_OSD2VEMODE pXCArgs = (pstXC_SET_OSD2VEMODE) pArgs;
            psXCInstPri->fpXC_SetOSD2VEMode(pInstance, pXCArgs->eVOPSelOSD_MUX);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_SET_IP2_PREFILTER_ENABLE:
        {


            pstXC_SET_IP2_PREFILTER_ENABLE pXCArgs = (pstXC_SET_IP2_PREFILTER_ENABLE) pArgs;
            psXCInstPri->fpXC_IP2_PreFilter_Enable(pInstance, pXCArgs->bEnable);
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
            CHECK_DEVICE1_INSTANCE();
            psXCInstPri->fpXC_IP2_PreFilter_Enable(g_pDevice1Instance, pXCArgs->bEnable);
#endif

            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_GET_PIXELRGB:
        {


            pstXC_GET_PIXELRGB pXCArgs = (pstXC_GET_PIXELRGB) pArgs;
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
            USE_OTHER_SC_AS_SUB_WINDOW();
#endif
            pXCArgs->eReturnValue = psXCInstPri->fpXC_Get_Pixel_RGB(pInstance, pXCArgs->pData, pXCArgs->eWindow);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_SET_KEEP_PIXELPOINTER_APPEAR:
        {


            pstXC_SET_KEEP_PIXELPOINTER_APPEAR pXCArgs = (pstXC_SET_KEEP_PIXELPOINTER_APPEAR) pArgs;
            psXCInstPri->fpXC_KeepPixelPointerAppear(pInstance, pXCArgs->bEnable);
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
            CHECK_DEVICE1_INSTANCE();
            psXCInstPri->fpXC_KeepPixelPointerAppear(g_pDevice1Instance, pXCArgs->bEnable);
#endif

            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_SET_MEMORY_FORMAT_EX:
        {


            pstXC_SET_MEMORY_FORMAT_EX pXCArgs = (pstXC_SET_MEMORY_FORMAT_EX) pArgs;
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
            USE_OTHER_SC_AS_SUB_WINDOW();
#endif
            pXCArgs->bReturnValue = psXCInstPri->fpXC_Set_MemFmtEx(pInstance, pXCArgs->eMemFmt, pXCArgs->eWindow);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_CHECK_REQUEST_FRAMEBUFFERLESS_MODE:
        {


            pstXC_CHECK_REQUEST_FRAMEBUFFERLESS_MODE pXCArgs = (pstXC_CHECK_REQUEST_FRAMEBUFFERLESS_MODE) pArgs;
            pXCArgs->bReturnValue = psXCInstPri->fpXC_IsRequestFrameBufferLessMode(pInstance);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_SET_SKIP_SWRESET:
        {


            pstXC_SET_SKIP_SWRESET pXCArgs = (pstXC_SET_SKIP_SWRESET) pArgs;
            pXCArgs->eReturnValue = psXCInstPri->fpXC_SkipSWReset(pInstance, pXCArgs->bFlag);
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
            CHECK_DEVICE1_INSTANCE();
            pXCArgs->eReturnValue = psXCInstPri->fpXC_SkipSWReset(g_pDevice1Instance, pXCArgs->bFlag);
#endif

            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_SET_REPORTWINDOW_FOR_FRAMECOLOR:
        {


            pstXC_SET_REPORTWINDOW_FOR_FRAMECOLOR pXCArgs = (pstXC_SET_REPORTWINDOW_FOR_FRAMECOLOR) pArgs;
            psXCInstPri->fpXC_EnableRepWindowForFrameColor(pInstance, pXCArgs->bEnable);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_SET_OSDLAYER:
        {


            pstXC_SET_OSDLAYER pXCArgs = (pstXC_SET_OSDLAYER) pArgs;
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
            USE_OTHER_SC_AS_SUB_WINDOW();
#endif
            pXCArgs->eReturnValue = psXCInstPri->fpXC_SetOSDLayer(pInstance, pXCArgs->eVOPOSDLayer, pXCArgs->eWindow);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_GET_OSDLAYER:
        {


            pstXC_GET_OSDLAYER pXCArgs = (pstXC_GET_OSDLAYER) pArgs;
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
            USE_OTHER_SC_AS_SUB_WINDOW();
#endif
            pXCArgs->eReturnValue = psXCInstPri->fpXC_GetOSDLayer(pInstance, pXCArgs->eWindow);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_SET_VIDEO_ALPHA:
        {


            pstXC_SET_VIDEO_ALPHA pXCArgs = (pstXC_SET_VIDEO_ALPHA) pArgs;
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
            USE_OTHER_SC_AS_SUB_WINDOW();
#endif
            pXCArgs->eReturnValue = psXCInstPri->fpXC_SetVideoAlpha(pInstance, pXCArgs->u8Val, pXCArgs->eWindow);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_GET_VIDEO_ALPHA:
        {


            pstXC_GET_VIDEO_ALPHA pXCArgs = (pstXC_GET_VIDEO_ALPHA) pArgs;
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
            USE_OTHER_SC_AS_SUB_WINDOW();
#endif
            pXCArgs->eReturnValue = psXCInstPri->fpXC_GetVideoAlpha(pInstance, pXCArgs->pu8Val, pXCArgs->eWindow);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_SKIP_WAIT_VSYNC:
        {


            pstXC_SKIP_WAIT_VSYNC pXCArgs = (pstXC_SKIP_WAIT_VSYNC) pArgs;
            pXCArgs->eReturnValue = psXCInstPri->fpXC_SkipWaitVsync(pInstance, pXCArgs->eWindow, pXCArgs->bIsSkipWaitVsyn);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_SET_CMA_HEAP_ID:
        {


            pstXC_SET_CMA_HEAP_ID pXCArgs = (pstXC_SET_CMA_HEAP_ID) pArgs;
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
            USE_OTHER_SC_AS_SUB_WINDOW();
#endif
            pXCArgs->eReturnValue = psXCInstPri->fpXC_SetCMAHeapID(pInstance, pXCArgs->u8CMAHeapID, pXCArgs->eWindow);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_SET_OP2VOP_DE_SELECTION:
        {


            pstXC_SET_OP2VOP_DE_SELECTION pXCArgs = (pstXC_SET_OP2VOP_DE_SELECTION) pArgs;
            pXCArgs->eReturnValue = psXCInstPri->fpXC_OP2VOPDESel(pInstance, pXCArgs->eVopDESel);
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
            CHECK_DEVICE1_INSTANCE();
            pXCArgs->eReturnValue = psXCInstPri->fpXC_OP2VOPDESel(g_pDevice1Instance, pXCArgs->eVopDESel);
#endif

            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_SET_FRC_WINDOW:
        {


            pstXC_SET_FRC_WINDOW pXCArgs = (pstXC_SET_FRC_WINDOW) pArgs;
            psXCInstPri->fpXC_FRC_SetWindow(pInstance, pXCArgs->e3dInputMode, pXCArgs->e3dOutputMode, pXCArgs->e3dPanelType);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_SET_2INITFACTOR_ENABLE:
        {


            pstXC_SET_2INITFACTOR_ENABLE pXCArgs = (pstXC_SET_2INITFACTOR_ENABLE) pArgs;
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
            USE_OTHER_SC_AS_SUB_WINDOW();
#endif
            pXCArgs->bReturnValue = psXCInstPri->fpXC_Enable_TwoInitFactor(pInstance, pXCArgs->bEnable, pXCArgs->eWindow);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_CHECK_FIELDPACKINGMODE_SUPPORTED:
        {


            pstXC_CHECK_FIELDPACKINGMODE_SUPPORTED pXCArgs = (pstXC_CHECK_FIELDPACKINGMODE_SUPPORTED) pArgs;
            pXCArgs->bReturnValue = psXCInstPri->fpXC_IsFieldPackingModeSupported(pInstance);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_PREINIT:
        {


            pstXC_PREINIT pXCArgs = (pstXC_PREINIT) pArgs;
            pXCArgs->eReturnValue = psXCInstPri->fpXC_PreInit(pInstance, pXCArgs->eType, pXCArgs->para, pXCArgs->u32Length);
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
            CHECK_DEVICE1_INSTANCE();
            pXCArgs->eReturnValue = psXCInstPri->fpXC_PreInit(g_pDevice1Instance, pXCArgs->eType, pXCArgs->para, pXCArgs->u32Length);
#endif

            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_GET_BUFFERDATA:
        {


            pstXC_GET_BUFFERDATA pXCArgs = (pstXC_GET_BUFFERDATA) pArgs;
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
            USE_OTHER_SC_AS_SUB_WINDOW();
#endif
            psXCInstPri->fpXC_Get_BufferData(pInstance, pXCArgs->eBufType, pXCArgs->pRect, pXCArgs->pRectBuf, pXCArgs->eWindow);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_SET_BUFFERDATA:
        {


            pstXC_SET_BUFFERDATA pXCArgs = (pstXC_SET_BUFFERDATA) pArgs;
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
            USE_OTHER_SC_AS_SUB_WINDOW();
#endif
            psXCInstPri->fpXC_Set_BufferData(pInstance, pXCArgs->eBufType, pXCArgs->pDstRect, pXCArgs->pSrcBuf, pXCArgs->pSrcRect, pXCArgs->eWindow);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_SET_MAINWINDOW_ENABLE:
        {


            pstXC_SET_MAINWINDOW_ENABLE pXCArgs = (pstXC_SET_MAINWINDOW_ENABLE) pArgs;
            psXCInstPri->fpXC_EnableMainWindow(pInstance, pXCArgs->bEnable);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_SET_SUBWINDOW_ENABLE:
        {


            pstXC_SET_SUBWINDOW_ENABLE pXCArgs = (pstXC_SET_SUBWINDOW_ENABLE) pArgs;
#if (PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB == 0)
            psXCInstPri->fpXC_EnableSubWindow(pInstance, pXCArgs->pstDispWin);
#else
            if(!MDrv_XC_GetMWEStatus(pInstance))
            {
                CHECK_DEVICE1_INSTANCE();
                psXCInstPri->fpXC_EnableSubWindow(g_pDevice1Instance, pXCArgs->pstDispWin);
            }
            else
            {
                psXCInstPri->fpXC_EnableSubWindow(pInstance, pXCArgs->pstDispWin);
            }
#endif

            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_SET_SUBWINDOW_DISABLE:
        {


#if (PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB == 0)
            psXCInstPri->fpXC_DisableSubWindow(pInstance);
#else
            if(!MDrv_XC_GetMWEStatus(pInstance))
            {
                CHECK_DEVICE1_INSTANCE();
                psXCInstPri->fpXC_DisableSubWindow(g_pDevice1Instance);
            }
            else
            {
                psXCInstPri->fpXC_DisableSubWindow(pInstance);
            }
#endif

            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }
        case E_XC_CMD_GET_PIXEL_SHIFT:
        {


            pstXC_GET_PIXEL_SHIFT pXCArgs = (pstXC_GET_PIXEL_SHIFT) pArgs;
            pXCArgs->eReturnValue = psXCInstPri->fpXC_GetPixelShift(pInstance, pXCArgs->ps8HOffset, pXCArgs->ps8VOffset);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }

        case E_XC_CMD_SET_PIXEL_SHIFT:
        {


            pstXC_SET_PIXEL_SHIFT pXCArgs = (pstXC_SET_PIXEL_SHIFT) pArgs;
            pXCArgs->eReturnValue = psXCInstPri->fpXC_SetPixelShift(pInstance, pXCArgs->s8H, pXCArgs->s8V);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_SET_VIDEO_ON_OSD:
        {
            pstXC_SET_VIDEO_ON_OSD pXCArgs = (pstXC_SET_VIDEO_ON_OSD) pArgs;
            pXCArgs->eReturnValue = psXCInstPri->fpXC_SetVideoOnOSD(pInstance, pXCArgs->enlayer, pXCArgs->eWindow);

            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }
        case E_XC_CMD_SET_OSD_LAYER_BLEANDING:
        {
            pstXC_SET_OSD_LAYER_BLENDING pXCArgs = (pstXC_SET_OSD_LAYER_BLENDING) pArgs;
            pXCArgs->eReturnValue = psXCInstPri->fpXC_SetOSDLayerBlending(pInstance, pXCArgs->u8Layer, pXCArgs->bEnable, pXCArgs->eWindow);

            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }
        case E_XC_CMD_SET_OSD_LAYER_ALPHA:
        {
            pstXC_SET_OSD_LAYER_ALPHA pXCArgs = (pstXC_SET_OSD_LAYER_ALPHA) pArgs;
            pXCArgs->eReturnValue = psXCInstPri->fpXC_SetOSDLayerAlpha(pInstance, pXCArgs->u8Layer, pXCArgs->u8Alpha);

            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }
        case E_XC_CMD_SET_OSD_BLENDING_FORMULA:
        {
            pstXC_SET_OSD_BLENDING_FORMULA pXCArgs = (pstXC_SET_OSD_BLENDING_FORMULA) pArgs;
            pXCArgs->eReturnValue = psXCInstPri->fpXC_SetOSDBlendingFormula(pInstance, pXCArgs->enOsdIndex, pXCArgs->enType, pXCArgs->eWindow);

            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_GET_REPORT_PIXEL_INFO:
        {


            pstXC_GET_REPORT_PIXEL_INFO pXCArgs = (pstXC_GET_REPORT_PIXEL_INFO) pArgs;
            pXCArgs->bReturnValue = psXCInstPri->fpXC_ReportPixelInfo(pInstance, pXCArgs->pstRepPixInfo);

            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_SET_SCALING:
        {


            pstXC_SET_SCALING pXCArgs = (pstXC_SET_SCALING) pArgs;
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
            USE_OTHER_SC_AS_SUB_WINDOW();
#endif
            psXCInstPri->fpXC_SetScaling(pInstance, pXCArgs->bEnable, pXCArgs->eScaling_type, pXCArgs->eVector_type, pXCArgs->eWindow);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_SET_MCDI_BUFFERADDRESS:
        {


            pstXC_SET_MCDI_BUFFERADDRESS pXCArgs = (pstXC_SET_MCDI_BUFFERADDRESS) pArgs;
            psXCInstPri->fpXC_SetMCDIBufferAddress(pInstance, pXCArgs->u32FBAddress, pXCArgs->u32FBSize, pXCArgs->eType);
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
            CHECK_DEVICE1_INSTANCE();
            psXCInstPri->fpXC_SetMCDIBufferAddress(g_pDevice1Instance, pXCArgs->u32FBAddress, pXCArgs->u32FBSize, pXCArgs->eType);
#endif

            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_SET_MCDI_ENABLE:
        {


            pstXC_SET_MCDI_ENABLE pXCArgs = (pstXC_SET_MCDI_ENABLE) pArgs;
            psXCInstPri->fpXC_EnableMCDI(pInstance, pXCArgs->bEnable, pXCArgs->eType);
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
            CHECK_DEVICE1_INSTANCE();
            psXCInstPri->fpXC_EnableMCDI(g_pDevice1Instance, pXCArgs->bEnable, pXCArgs->eType);
#endif


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_SEND_CMD2FRC:
        {


            pstXC_SEND_CMD2FRC pXCArgs = (pstXC_SEND_CMD2FRC) pArgs;
            pXCArgs->bReturnValue = psXCInstPri->fpXC_SendCmdToFRC(pInstance, pXCArgs->u8Cmd, pXCArgs->count, pXCArgs->pFRC_R2_Para);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_GET_MSG_FROM_FRC:
        {


            pstXC_GET_MSG_FROM_FRC pXCArgs = (pstXC_GET_MSG_FROM_FRC) pArgs;
            pXCArgs->bReturnValue = psXCInstPri->fpXC_GetMsgFromFRC(pInstance, pXCArgs->pu8Cmd, pXCArgs->pu8ParaCount, pXCArgs->pu8Para);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }

        case E_XC_CMD_GET_FRCSWVERSION:
        {


            pstXC_FRCSWVERSION pXCArgs = (pstXC_FRCSWVERSION) pArgs;
            pXCArgs->enReturnValue = psXCInstPri->fpXC_GetFrcSwVersion(pInstance, pXCArgs->pstXCFrcSwVersion);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }

        case E_XC_CMD_GET_FRCFSCCTRLPOLICY:
        {            pstXC_FRCFSCCTRLPOLICY pXCArgs = (pstXC_FRCFSCCTRLPOLICY) pArgs;
            pXCArgs->enReturnValue = psXCInstPri->fpXC_GetFrcFscCtrlPolicy(pInstance, pXCArgs->pstXCFrcFscCtrlPolicy);

            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }

        case E_XC_CMD_SET_AUTO_RWBANK_ENABLE:
        {


            pstXC_SET_AUTO_RWBANK_ENABLE pXCArgs = (pstXC_SET_AUTO_RWBANK_ENABLE) pArgs;
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
            USE_OTHER_SC_AS_SUB_WINDOW();
#endif
            psXCInstPri->fpXC_EnableRWBankAuto(pInstance, pXCArgs->bEnable, pXCArgs->eWindow);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_SET_RWBANK_MAPPING_NUM:
        {


            pstXC_SET_RWBANK_MAPPING_NUM pXCArgs = (pstXC_SET_RWBANK_MAPPING_NUM) pArgs;
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
            USE_OTHER_SC_AS_SUB_WINDOW();
#endif
            psXCInstPri->fpXC_SetWRBankMappingNum(pInstance, pXCArgs->u8Val, pXCArgs->eWindow);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_GET_RWBANK_MAPPING_NUM:
        {
            pstXC_GET_RWBANK_MAPPING_NUM pXCArgs = (pstXC_GET_RWBANK_MAPPING_NUM) pArgs;
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
            USE_OTHER_SC_AS_SUB_WINDOW();
#endif
            pXCArgs->u8ReturnValue = psXCInstPri->fpXC_GetWRBankMappingNum(pInstance, pXCArgs->eWindow);

            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }

        case E_XC_CMD_GET_RWBANK_MAPPING_NUM_FOR_ZAP:
        {
            pstXC_GET_RWBANK_MAPPING_NUM_FOR_ZAP pXCArgs = (pstXC_GET_RWBANK_MAPPING_NUM_FOR_ZAP) pArgs;
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
            USE_OTHER_SC_AS_SUB_WINDOW();
#endif
            pXCArgs->u8ReturnValue = psXCInstPri->fpXC_GetWRBankMappingNumForZap(pInstance, pXCArgs->eWindow);

            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }

        case E_XC_CMD_SET_BOBMODE:
        {
            pstXC_SET_BOBMODE pXCArgs = (pstXC_SET_BOBMODE) pArgs;
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
            USE_OTHER_SC_AS_SUB_WINDOW();
#endif
            pXCArgs->bReturnValue = psXCInstPri->fpXC_SetBOBMode(pInstance, pXCArgs->bEnable, pXCArgs->eWindow);

            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }

        case E_XC_CMD_SET_FORCE_READBANK:
        {
            pstXC_SET_FORCE_READBANK pXCArgs = (pstXC_SET_FORCE_READBANK) pArgs;
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
            USE_OTHER_SC_AS_SUB_WINDOW();
#endif
            psXCInstPri->fpXC_SetForceReadBank(pInstance, pXCArgs->bEnable,pXCArgs->u8Bank, pXCArgs->eWindow);

            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }

        case E_XC_CMD_SET_LOCK_POINT:
        {
            pstXC_SET_LOCK_POINT pXCArgs = (pstXC_SET_LOCK_POINT) pArgs;
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
            USE_OTHER_SC_AS_SUB_WINDOW();
#endif
            psXCInstPri->fpXC_SetLockPoint(pInstance, pXCArgs->u16LockPoint, pXCArgs->eWindow);

            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }

        case E_XC_CMD_LD_INIT:
        {
            pstXC_LD_INIT pXCArgs = (pstXC_LD_INIT) pArgs;
            pXCArgs->bReturnValue = psXCInstPri->fpXC_LD_Init(pInstance, pXCArgs->enLDPanelType);

            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_SET_LD_MEMORYADDRESS:
        {
            pstXC_SET_LD_MEMORYADDRESS pXCArgs = (pstXC_SET_LD_MEMORYADDRESS) pArgs;
            pXCArgs->bReturnValue = psXCInstPri->fpXC_LD_SetMemoryAddress(pInstance,
                                    pXCArgs->u8MIU,
                                    pXCArgs->u32LDFBase0,
                                    pXCArgs->u32LDFBase1,
                                    pXCArgs->u32LDBBase0,
                                    pXCArgs->u32LDBBase1,
                                    pXCArgs->u32EDGE2DBase,
                                    pXCArgs->u32LEDOffset);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }

        case E_XC_CMD_GET_LD_VALUE:
        {
            pstXC_GET_LD_VALUE pXCArgs = (pstXC_GET_LD_VALUE) pArgs;
            pXCArgs->bReturnValue = psXCInstPri->fpXC_LD_Get_Value(pInstance,
                                    pXCArgs->pu8LDValue,
                                    pXCArgs->u8WSize,
                                    pXCArgs->u8HSize);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }

        case E_XC_CMD_SET_LD_LEVEL:
        {


            pstXC_SET_LD_LEVEL pXCArgs = (pstXC_SET_LD_LEVEL) pArgs;
            pXCArgs->bReturnValue = psXCInstPri->fpXC_LD_SetLevel(pInstance, pXCArgs->enMode);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_SET_LD_TURNOFF_LDBL:
        {


            pstXC_SET_LD_TURNOFF_LDBL pXCArgs = (pstXC_SET_LD_TURNOFF_LDBL) pArgs;
            pXCArgs->bReturnValue = psXCInstPri->fpSet_TurnoffLDBL(pInstance, pXCArgs->bTurnoffBL);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_SET_NOTUPDATE_SPI_DATA_FLAGS:
        {


            pstXC_SET_NOTUPDATE_SPI_DATA_FLAGS pXCArgs = (pstXC_SET_NOTUPDATE_SPI_DATA_FLAGS) pArgs;
            pXCArgs->bReturnValue = psXCInstPri->fpSet_notUpdateSPIDataFlags(pInstance, pXCArgs->bRefreshSPIData);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_SET_LD_USERMODEFLAGS:
        {


            pstXC_SET_LD_USERMODEFLAGS pXCArgs = (pstXC_SET_LD_USERMODEFLAGS) pArgs;
            pXCArgs->bReturnValue = psXCInstPri->fpSet_UsermodeLDFlags(pInstance, pXCArgs->bUserLDFlags);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_SET_BL_LEVEL:
        {


            pstXC_SET_BL_LEVEL pXCArgs = (pstXC_SET_BL_LEVEL) pArgs;
            pXCArgs->bReturnValue = psXCInstPri->fpSet_BLLevel(pInstance, pXCArgs->u8BLLevel);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_SET_BWS_MODE:
        {


            pstXC_SET_BWS_MODE pXCArgs = (pstXC_SET_BWS_MODE) pArgs;
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
            USE_OTHER_SC_AS_SUB_WINDOW();
#endif
            psXCInstPri->fpXC_Set_BWS_Mode(pInstance, pXCArgs->bEnable, pXCArgs->eWindow);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_SET_FRC_COLOR_PATH_CONTROL:
        {


            pstXC_SET_FRC_COLOR_PATH_CONTROL pXCArgs = (pstXC_SET_FRC_COLOR_PATH_CONTROL) pArgs;
            psXCInstPri->fpXC_FRC_ColorPathCtrl(pInstance, pXCArgs->u16Path_sel, pXCArgs->bEnable);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_SET_FRC_OP2_RGBGAIN:
        {


            pstXC_SET_FRC_OP2_RGBGAIN pXCArgs = (pstXC_SET_FRC_OP2_RGBGAIN) pArgs;
            psXCInstPri->fpXC_FRC_OP2_SetRGBGain(pInstance, pXCArgs->u16RedGain, pXCArgs->u16GreenGain, pXCArgs->u16BlueGain);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_SET_FRC_OP2_RGBOFFSET:
        {


            pstXC_SET_FRC_OP2_RGBOFFSET pXCArgs = (pstXC_SET_FRC_OP2_RGBOFFSET) pArgs;
            psXCInstPri->fpXC_FRC_OP2_SetRGBOffset(pInstance, pXCArgs->u16RedOffset, pXCArgs->u16GreenOffset, pXCArgs->u16BlueOffset);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_SET_FRC_OP2_DITHER:
        {


            pstXC_SET_FRC_OP2_DITHER pXCArgs = (pstXC_SET_FRC_OP2_DITHER) pArgs;
            psXCInstPri->fpXC_FRC_OP2_SetDither(pInstance, pXCArgs->u16dither);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_FRC_BYPASS:
        {


            pstXC_FRC_BYPASS pXCArgs = (pstXC_FRC_BYPASS) pArgs;
            pXCArgs->bReturnValue = psXCInstPri->fpXC_FRC_BypassMFC(pInstance, pXCArgs->bEnable);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }

        case E_XC_CMD_FRC_MUTE:
        {


            pstXC_FRC_MUTE pXCArgs = (pstXC_FRC_MUTE) pArgs;
            psXCInstPri->fpXC_FRC_Mute(pInstance, pXCArgs->bEnable);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }

        case E_XC_CMD_SET_FORCE_READ_FRAME:
        {


            pstXC_SET_FORCE_READ_FRAME pXCArgs = (pstXC_SET_FORCE_READ_FRAME) pArgs;
            pXCArgs->eReturnValue = psXCInstPri->fpXC_ForceReadFrame(pInstance, pXCArgs->bEnable, pXCArgs->u16FrameIndex);
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
            CHECK_DEVICE1_INSTANCE();
            pXCArgs->eReturnValue = psXCInstPri->fpXC_ForceReadFrame(g_pDevice1Instance, pXCArgs->bEnable, pXCArgs->u16FrameIndex);
#endif

            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_SET_CSC:
        {


            pstXC_SET_CSC pXCArgs = (pstXC_SET_CSC) pArgs;
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
            USE_OTHER_SC_AS_SUB_WINDOW();
#endif
            psXCInstPri->fpXC_SetCsc(pInstance, pXCArgs->bEnable, pXCArgs->eWindow);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_SET_REGISTER_PQ_FPLL_THRESH_MODE:
        {


            pstXC_SET_REGISTER_PQ_FPLL_THRESH_MODE pXCArgs = (pstXC_SET_REGISTER_PQ_FPLL_THRESH_MODE) pArgs;
            psXCInstPri->fpXC_RegisterPQSetFPLLThreshMode(pInstance, pXCArgs->fpPQCB);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }

        case E_XC_CMD_GET_FREERUN_STATUS:
        {
            pstXC_GET_FREERUN_STATUS pXCArgs = (pstXC_GET_FREERUN_STATUS) pArgs;
            pXCArgs->bReturnValue = psXCInstPri->fpXC_GetFreeRunStatus(pInstance);

            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }

        case E_XC_CMD_SET_BYPASS_CSC:
        {
            pstXC_SET_BYPASS_CSC pXCArgs = (pstXC_SET_BYPASS_CSC) pArgs;
            psXCInstPri->fpXC_BYPASS_SetCSC(pInstance, pXCArgs->bEnable);

            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }

        case E_XC_CMD_GET_DS_FORCE_INDEX_SUPPORTED:
        {
            pstXC_GET_DS_FORCE_INDEX_SUPPORTED pXCArgs = (pstXC_GET_DS_FORCE_INDEX_SUPPORTED) pArgs;
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
            USE_OTHER_SC_AS_SUB_WINDOW();
#endif
            pXCArgs->eReturnValue = psXCInstPri->fpXC_Get_DSForceIndexSupported(pInstance, pXCArgs->eWindow);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_SET_DS_FORCE_INDEX:
        {


            pstXC_SET_DS_FORCE_INDEX pXCArgs = (pstXC_SET_DS_FORCE_INDEX) pArgs;
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
            USE_OTHER_SC_AS_SUB_WINDOW();
#endif
            pXCArgs->eReturnValue = psXCInstPri->fpXC_Set_DSForceIndex(pInstance, pXCArgs->bEnable, pXCArgs->u8Index, pXCArgs->eWindow);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }

        case E_XC_CMD_SET_DS_FORCE_INDEX_EXT:
        {
            pstXC_SET_DS_FORCE_INDEX_EXT pXCArgs = (pstXC_SET_DS_FORCE_INDEX_EXT) pArgs;
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
            USE_OTHER_SC_AS_SUB_WINDOW();
#endif
            pXCArgs->enReturnValue = psXCInstPri->fpXC_Set_DSForceIndexEXT(pInstance, pXCArgs->enWindow, pXCArgs->pstDSForceIndexExt);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }

        case E_XC_CMD_SET_DS_INDEX_SOURCE_SELECT:
        {
            pstXC_SET_DS_INDEX_SOURCE_SELECT pXCArgs = (pstXC_SET_DS_INDEX_SOURCE_SELECT) pArgs;
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
            USE_OTHER_SC_AS_SUB_WINDOW();
#endif
            pXCArgs->eReturnValue = psXCInstPri->fpXC_Set_DSIndexSourceSelect(pInstance, pXCArgs->eDSIdxSrc, pXCArgs->eWindow);

            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }

        case E_XC_CMD_GET_DS_STATUS:
        {
            pstXC_GET_DS_STATUS pXCArgs = (pstXC_GET_DS_STATUS) pArgs;
            pXCArgs->bReturnValue = psXCInstPri->fpXC_GetDynamicScalingStatus(pInstance);

            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }

        case E_XC_CMD_OSDC_INITSETTING:
        {
            pstXC_OSDC_INITSETTING pXCArgs = (pstXC_OSDC_INITSETTING) pArgs;
            pXCArgs->eReturnValue = psXCInstPri->fpXC_OSDC_InitSetting(pInstance, pXCArgs->e_osdctgen_type, pXCArgs->pstOC_Tgen_Cus, pXCArgs->pstOC_Ctrl);

            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }

        case E_XC_CMD_OSDC_SET_OUTPUTVFREQX10:
        {
            pstXC_OSDC_SET_OUTPUTVFREQX10 pXCArgs = (pstXC_OSDC_SET_OUTPUTVFREQX10) pArgs;
            psXCInstPri->fpXC_OSDC_SetOutVfreqx10(pInstance, pXCArgs->u16Vfreq);

            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }

        case E_XC_CMD_OSDC_CONTROL:
        {


            pstXC_OSDC_CONTROL pXCArgs = (pstXC_OSDC_CONTROL) pArgs;
            pXCArgs->eReturnValue = psXCInstPri->fpXC_OSDC_Control(pInstance, pXCArgs->eCtrl_type);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_OSDC_GET_DESTINATION_INFO:
        {
            pstXC_OSDC_GET_DESTINATION_INFO pXCArgs = (pstXC_OSDC_GET_DESTINATION_INFO) pArgs;
            pXCArgs->eReturnValue = psXCInstPri->fpXC_OSDC_GetDstInfo(pInstance, pXCArgs->pDstInfo, pXCArgs->u32SizeofDstInfo);

            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }

        case E_XC_CMD_SET_POWERSTATE:
        {
            pstXC_SET_POWERSTATE pXCArgs = (pstXC_SET_POWERSTATE) pArgs;
            pXCArgs->u32ReturnValue = psXCInstPri->fpXC_Set_PowerState(pInstance, pXCArgs->enPowerState);

#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
            CHECK_DEVICE1_INSTANCE();

            if (g_pDevice1Instance !=NULL)
            {
                psXCInstPri->fpXC_Set_PowerState(g_pDevice1Instance, pXCArgs->enPowerState);
            }
#endif

            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }

        case E_XC_CMD_SET_BYPASS_OSDVSYNC_POS:
        {
            pstXC_SET_BYPASS_OSDVSYNC_POS pXCArgs = (pstXC_SET_BYPASS_OSDVSYNC_POS) pArgs;
            psXCInstPri->fpXC_BYPASS_SetOSDVsyncPos(pInstance, pXCArgs->u16VsyncStart, pXCArgs->u16VsyncEnd);

            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }

        case E_XC_CMD_SET_BYPASS_INPUTSRC:
        {
            pstXC_SET_BYPASS_INPUTSRC pXCArgs = (pstXC_SET_BYPASS_INPUTSRC) pArgs;
            psXCInstPri->fpXC_BYPASS_SetInputSrc(pInstance, pXCArgs->bEnable, pXCArgs->input);

            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }

        case E_XC_CMD_SET_SEAMLESS_ZAPPING:
        {
            pstXC_SET_SEAMLESS_ZAPPING pXCArgs = (pstXC_SET_SEAMLESS_ZAPPING) pArgs;
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
            USE_OTHER_SC_AS_SUB_WINDOW();
#endif
            pXCArgs->eReturnValue = psXCInstPri->fpXC_SetSeamlessZapping(pInstance, pXCArgs->eWindow, pXCArgs->bEnable);

            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }

        case E_XC_CMD_GET_SEAMLESS_ZAPPING_STATUS:
        {
            pstXC_GET_SEAMLESS_ZAPPING_STATUS pXCArgs = (pstXC_GET_SEAMLESS_ZAPPING_STATUS) pArgs;
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
            USE_OTHER_SC_AS_SUB_WINDOW();
#endif
            pXCArgs->eReturnValue = psXCInstPri->fpXC_GetSeamlessZappingStatus(pInstance, pXCArgs->eWindow, pXCArgs->pbEnable);

            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }

        case E_XC_CMD_SET_VTRACK_PAYLOADDATA:
        {
            pstXC_SET_VTRACK_PAYLOADDATA pXCArgs = (pstXC_SET_VTRACK_PAYLOADDATA) pArgs;
            pXCArgs->eReturnValue = psXCInstPri->fpXC_Vtrack_SetPayloadData(pInstance, pXCArgs->u16Timecode, pXCArgs->u8OperatorID);

            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }

        case E_XC_CMD_SET_VTRACK_ALLPAYLOADDATA:
        {
            pstXC_SET_VTRACK_ALLPAYLOADDATA pXCArgs = (pstXC_SET_VTRACK_ALLPAYLOADDATA) pArgs;
            pXCArgs->eReturnValue = psXCInstPri->fpXC_Vtrack_SetAllPayloadData(pInstance, pXCArgs->vtrack_payload);

            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }

        case E_XC_CMD_SET_VTRACK_VTRACKKEY:
        {
            pstXC_SET_VTRACK_VTRACKKEY pXCArgs = (pstXC_SET_VTRACK_VTRACKKEY) pArgs;
            pXCArgs->eReturnValue = psXCInstPri->fpXC_Vtrack_SetVtrackKey(pInstance, pXCArgs->u64VtrackKey);

            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }

        case E_XC_CMD_SET_VTRACK_USERDEFINED_SETTING:
        {
            pstXC_SET_VTRACK_USERDEFINED_SETTING pXCArgs = (pstXC_SET_VTRACK_USERDEFINED_SETTING) pArgs;
            pXCArgs->eReturnValue = psXCInstPri->fpXC_Vtrack_SetUserDefindedSetting(pInstance, pXCArgs->bUserDefinded, pXCArgs->pu8Setting);

            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }

        case E_XC_CMD_SET_VTRACK_ENABLE:
        {
            pstXC_SET_VTRACK_ENABLE pXCArgs = (pstXC_SET_VTRACK_ENABLE) pArgs;
            pXCArgs->eReturnValue = psXCInstPri->fpXC_Vtrack_Enable(pInstance, pXCArgs->u8FrameRate, pXCArgs->bEnable);

            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }

        case E_XC_CMD_PRESET_PQINFO:
        {
            pstXC_PRESET_PQINFO pXCArgs = (pstXC_PRESET_PQINFO) pArgs;
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
            USE_OTHER_SC_AS_SUB_WINDOW();
#endif
            psXCInstPri->fpXC_PreSetPQInfo(pInstance, pXCArgs->pstXC_SetWin_Info, pXCArgs->eWindow);

            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }

        case E_XC_GET_OP1_TESTPATTERN_ENABLED:
        {
            pstXC_GET_OP1_TESTPATTERN_ENABLED pXCArgs = (pstXC_GET_OP1_TESTPATTERN_ENABLED) pArgs;
            pXCArgs->bReturnValue= psXCInstPri->fpXC_Is_OP1_TestPattern_Enabled(pInstance);

            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }

        case E_XC_SET_OP1_TESTPATTERN:
        {
            pstXC_SET_OP1_TESTPATTERN pXCArgs = (pstXC_SET_OP1_TESTPATTERN) pArgs;
            pXCArgs->eReturnValue= psXCInstPri->fpXC_Set_OP1_TestPattern(pInstance,pXCArgs->ePattern,pXCArgs->eMode);

            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_SET_OP2_TESTPATTERN:
        {
            pstXC_SET_OP2_TESTPATTERN pXCArgs = (pstXC_SET_OP2_TESTPATTERN) pArgs;
            pXCArgs->eReturnValue = psXCInstPri->fpXC_Set_OP2_TestPattern(pInstance
                                    ,pXCArgs->bEnable
                                    ,pXCArgs->u16R_Data
                                    ,pXCArgs->u16G_Data
                                    ,pXCArgs->u16B_Data);

            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }
        case E_XC_CHECK_WHITEBALANCE_PATTERN_MODE:
        {
            pstXC_CHECK_WB_TESTPATTERN pXCArgs = (pstXC_CHECK_WB_TESTPATTERN) pArgs;
            pXCArgs->eReturnValue = psXCInstPri->fpXC_Check_WhiteBalance_Pattern_Mode(pInstance
                                    ,pXCArgs->eWBPattern
                                                                                     );

            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }
        case E_XC_SET_HLINEARSCALING:
        {
            pstXC_SET_HLINEARSCALING pXCArgs = (pstXC_SET_HLINEARSCALING) pArgs;
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
            USE_OTHER_SC_AS_SUB_WINDOW();
#endif
            pXCArgs->eReturnValue = psXCInstPri->fpXC_Set_HLinearScaling(pInstance
                                    ,pXCArgs->bEnable
                                    ,pXCArgs->bSign
                                    ,pXCArgs->u16Delta
                                    ,pXCArgs->eWindow);

            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }
        case E_XC_SET_ENABLE_T3D:
        {
            pstXC_EnableT3D pXCArgs = (pstXC_EnableT3D) pArgs;
            pXCArgs->eReturnValue = psXCInstPri->fpXC_EnableT3D(pInstance
                                    ,pXCArgs->bEnable);

            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }
        case E_XC_SET_FRC_INPUTTIMING:
        {
            pstXC_Set_FRC_InputTiming pXCArgs = (pstXC_Set_FRC_InputTiming) pArgs;
            pXCArgs->eReturnValue = psXCInstPri->fpXC_Set_FRC_InputTiming(pInstance
                                    ,pXCArgs->enFRC_InputTiming);

            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }
        case E_XC_GET_FRC_INPUTTIMING:
        {
            pstXC_Get_FRC_InputTiming pXCArgs = (pstXC_Get_FRC_InputTiming) pArgs;
            pXCArgs->eReturnValue = psXCInstPri->fpXC_Get_FRC_InputTiming(pInstance, pXCArgs->penFRC_InputTiming);

            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }
        case E_XC_CMD_GET_VBOX_INFO:
        {
            pstXC_Get_VBOX_Info pXCArgs = (pstXC_Get_VBOX_Info) pArgs;
            pXCArgs->eReturnValue = psXCInstPri->fpXC_Get_VirtualBox_Info(pInstance, pXCArgs->pstXC_VboxInfo);

            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }
        case E_XC_SET_OSD_DETECT:
        {
            pstXC_Set_OSD_DETECT pXCArgs = (pstXC_Set_OSD_DETECT) pArgs;
            pXCArgs->eReturnValue = psXCInstPri->fpXC_Set_OSD_DETECT(pInstance, pXCArgs->bEnable,pXCArgs->Threhold);
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
            CHECK_DEVICE1_INSTANCE();
            pXCArgs->eReturnValue = psXCInstPri->fpXC_Set_OSD_DETECT(g_pDevice1Instance, pXCArgs->bEnable,pXCArgs->Threhold);
#endif
            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }
        case E_XC_GET_OSD_DETECT:
        {
            pstXC_Get_OSD_DETECT pXCArgs = (pstXC_Get_OSD_DETECT) pArgs;
            pXCArgs->eReturnValue = psXCInstPri->fpXC_Get_OSD_DETECT(pInstance, pXCArgs->pbOSD);
            /*
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
            CHECK_DEVICE1_INSTANCE();
            pXCArgs->eReturnValue = psXCInstPri->fpXC_Get_OSD_DETECT(g_pDevice1Instance, pXCArgs->pbOSD);
#endif
            */
            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }
        case E_XC_ENABLE_LOCKFREQONLY:
        {
            pstXC_Enable_LockFreqOnly pXCArgs = (pstXC_Enable_LockFreqOnly) pArgs;
            pXCArgs->eReturnValue = psXCInstPri->fpXC_Enable_LockFreqOnly(pInstance, pXCArgs->bEnable);

            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }
        case E_XC_IS2K2KTOFRCMODE:
        {
            pstXC_Is2K2KToFrcMode pXCArgs = (pstXC_Is2K2KToFrcMode) pArgs;
            pXCArgs->bReturnValue = psXCInstPri->fpXC_Is2K2KToFrcMode(pInstance);

            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }
        case E_XC_CMD_SET_XC_VOP:
        {
            pstXC_SetXC_VOP pXCArgs = (pstXC_SetXC_VOP) pArgs;
            pXCArgs->bReturn = psXCInstPri->fpXC_SetXCVOPdata(pInstance, pXCArgs->stXC_VOPData);

            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }

// OBSOLETE command **START**, please avoid using these
        case E_XC_CMD_GET_OUTPUT_VFREQ:
        {


            pstXC_SetXC_VOP pXCArgs = (pstXC_SetXC_VOP) pArgs;
            pXCArgs->bReturn = psXCInstPri->fpsc_get_output_vfreq(pInstance);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_GET_CURRENT_OUTPUTVFREQX100:
        {


            pstXC_GET_CURRENT_OUTPUTVFREQX100 pXCArgs = (pstXC_GET_CURRENT_OUTPUTVFREQX100) pArgs;
            pXCArgs->u32ReturnValue = psXCInstPri->fpXC_Get_Current_OutputVFreqX100(pInstance);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_SET_MEMORY_WRITE_REQUEST:
        {


            pstXC_SET_MEMORY_WRITE_REQUEST pXCArgs = (pstXC_SET_MEMORY_WRITE_REQUEST) pArgs;
            pXCArgs->eReturnValue = psXCInstPri->fpXC_SetMemoryWriteRequest(pInstance, pXCArgs->bEnable);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_SET_MEMORY_FORMAT:
        {


            pstXC_SET_MEMORY_FORMAT pXCArgs = (pstXC_SET_MEMORY_FORMAT) pArgs;
            pXCArgs->bReturnValue = psXCInstPri->fpXC_Set_MemFmt(pInstance, pXCArgs->eMemFmt);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_SET_OUTPUT_ADJUST_SETTING:
        {


            pstXC_SET_OUTPUT_ADJUST_SETTING pXCArgs = (pstXC_SET_OUTPUT_ADJUST_SETTING) pArgs;
            pXCArgs->eReturnValue = psXCInstPri->fpXC_SetOutputAdjustSetting(pInstance, pXCArgs->stAdjSetting);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_SET_FPLL_FSM:
        {


            pstXC_SET_FPLL_FSM pXCArgs = (pstXC_SET_FPLL_FSM) pArgs;
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
            USE_OTHER_SC_AS_SUB_WINDOW();
#endif
            psXCInstPri->fsScaler_FPLL_FSM(pInstance, pXCArgs->eWindow);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_SET_MIRROR_MODE_ENABLE:
        {


            pstXC_SET_MIRROR_MODE_ENABLE pXCArgs = (pstXC_SET_MIRROR_MODE_ENABLE) pArgs;
            psXCInstPri->fpXC_EnableMirrorMode(pInstance, pXCArgs->bEnable);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_SET_MIRROR_MODE2_ENABLE:
        {


            pstXC_SET_MIRROR_MODE2_ENABLE pXCArgs = (pstXC_SET_MIRROR_MODE2_ENABLE) pArgs;
            psXCInstPri->fpXC_EnableMirrorMode2(pInstance, pXCArgs->eMirrorMode);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }

        case E_XC_CMD_GET_SWDSIndex:
        {
            pstXC_GET_SWDSIndex pXCArgs = (pstXC_GET_SWDSIndex) pArgs;
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
            USE_OTHER_SC_AS_SUB_WINDOW();
#endif
            pXCArgs->u8ReturnValue = psXCInstPri->fpXC_GetSWDSIndex(pInstance, pXCArgs->eWindow);

            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }

        case E_XC_CMD_GET_IS_SupportSWDS:
        {
            pstXC_GET_Is_SupportSWDS pXCArgs = (pstXC_GET_Is_SupportSWDS) pArgs;
            pXCArgs->bEnable = psXCInstPri->fpXC_Get_Is_SupportSWDS(pInstance);

            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }
#ifdef UFO_XC_SET_DSINFO_V0
        case E_XC_CMD_SET_DSINFO:
        {
            pstXC_SET_DSINFO pXCArgs = (pstXC_SET_DSINFO) pArgs;
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
            if(pXCArgs->eWindow == SUB_WINDOW)
            {
                pXCArgs->pstSetDSInfo->bUpdate_DS_CMD[MAIN_WINDOW] = pXCArgs->pstSetDSInfo->bUpdate_DS_CMD[SUB_WINDOW];
                pXCArgs->pstSetDSInfo->bEnableDNR[MAIN_WINDOW] = pXCArgs->pstSetDSInfo->bEnableDNR[SUB_WINDOW];
                pXCArgs->pstSetDSInfo->bEnable_ForceP[MAIN_WINDOW] = pXCArgs->pstSetDSInfo->bEnable_ForceP[SUB_WINDOW];
                pXCArgs->pstSetDSInfo->bDynamicScalingEnable[MAIN_WINDOW] = pXCArgs->pstSetDSInfo->bDynamicScalingEnable[SUB_WINDOW];
            }

            USE_OTHER_SC_AS_SUB_WINDOW();
#endif
            psXCInstPri->fpXC_SetDSInfo(pInstance,pXCArgs->pstSetDSInfo, pXCArgs->u32DSInfoLen, pXCArgs->eWindow);

            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }
#endif
        case E_XC_CMD_GET_FRCM_FRAMENUM:
        {

            pstXC_GET_FRCM_FRAMENUM pXCArgs = (pstXC_GET_FRCM_FRAMENUM) pArgs;
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
            USE_OTHER_SC_AS_SUB_WINDOW();
#endif
            pXCArgs->u8ReturnValue = psXCInstPri->fpXC_Get_FRCM_FrameNum(pInstance, pXCArgs->eWindow);

            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }

        case E_XC_CMD_SET_PIXEL_SHIFT_FEATURES:
        {
            pstXC_SET_PIXELSHIFT_FEATURES pXCArgs = (pstXC_SET_PIXELSHIFT_FEATURES) pArgs;
            pXCArgs->eReturnValue = psXCInstPri->fpXC_SetPixelShiftFeatures(pInstance,
                                    pXCArgs->u16HPixelShiftRange,
                                    pXCArgs->u16VPixelShiftRange,
                                    pXCArgs->ePixelShiftFeature);

            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }
        case E_XC_CMD_DUMP_SHM:
        {
            psXCInstPri->fpXC_dump_shm(pInstance);
            break;
        }
        case E_XC_CMD_SET_BWD_CONFIG:
        {
            pstXC_SetBwdConfig pXCArgs = (pstXC_SetBwdConfig)pArgs;
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
            USE_OTHER_SC_AS_SUB_WINDOW();
#endif
            pXCArgs->bReturnValue = psXCInstPri->fpXC_SetBwdConfig(pInstance,pXCArgs->eType,pXCArgs->pstParam, pXCArgs->eWindow);
            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }
#ifdef UFO_XC_SUPPORT_DUAL_MIU
        case E_XC_CMD_ENABLE_DUAL_MODE:
        {
            pstXC_EnableDualMode pXCArgs = (pstXC_EnableDualMode)pArgs;
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
            USE_OTHER_SC_AS_SUB_WINDOW();
#endif
            pXCArgs->bReturnValue = psXCInstPri->fpXC_enableDualMode(pInstance,pXCArgs->bEnable,pXCArgs->u32LeftFactor, pXCArgs->u32RightFactor,pXCArgs->eWindow);
            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }
#endif

        case E_XC_CMD_IS_SUPPORT_2_STEP_SCALING:
        {
            pstXC_IsSupport2StepScaling pXCArgs = (pstXC_IsSupport2StepScaling)pArgs;
            pXCArgs->bReturnValue = psXCInstPri->fpXC_IsSupport2StepScaling(pInstance);
            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }

        case E_XC_CMD_SET_FORCE_WRITE:
        {


            pstXC_SET_FORCE_WRITE pXCArgs = (pstXC_SET_FORCE_WRITE) pArgs;
            pXCArgs->eReturnValue = psXCInstPri->fpXC_SetForceWrite(pInstance, pXCArgs->enMuteType, pXCArgs->bIsForceWrite);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }

        case E_XC_CMD_GET_FORCE_WRITE:
        {
            pstXC_GET_FORCE_WRITE pXCArgs = (pstXC_GET_FORCE_WRITE) pArgs;
            pXCArgs->eReturnValue = psXCInstPri->fpXC_GetForceWrite(pInstance, pXCArgs->enMuteType);

            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }
        case E_XC_CMD_VIDEO_PLAYBACK_CTRL:
        {
            pstXC_VIDEO_PLAYBACK_CTRL pXCArgs = (pstXC_VIDEO_PLAYBACK_CTRL) pArgs;
            pXCArgs->eReturnValue = psXCInstPri->fpXC_VideoPlaybackCtrl(pInstance,pXCArgs->u32Cmd,pXCArgs->pCmdArgs,pXCArgs->u32CmdArgsSize);

            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }

#ifdef UFO_XC_PQ_PATH
            case E_XC_CMD_GET_PQ_PATH:
            {
                pstXC_GetPqPathStatus pXCArgs = (pstXC_GetPqPathStatus)pArgs;
                pXCArgs->bReturnValue = psXCInstPri->fpXC_GetPQPathStatus(pInstance, pXCArgs->ePqPathType, pXCArgs->u16Width, pXCArgs->u16Height);
                u32Return = UTOPIA_STATUS_SUCCESS;
                break;
            }
#endif

#ifdef UFO_XC_AUTO_DOWNLOAD
        case E_XC_CMD_AUTODOWNLOAD_CONFIG_CTRL:
        {
            pstXC_AUTODOWNLOAD_CONFIG_CTRL pXCArgs = (pstXC_AUTODOWNLOAD_CONFIG_CTRL) pArgs;
            pXCArgs->eReturnValue = psXCInstPri->fpXC_AutoDownloadConfig(pInstance,pXCArgs->pstConfigInfo);

            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }
        case E_XC_CMD_AUTODOWNLOAD_WRITE_CTRL:
        {
            pstXC_AUTODOWNLOAD_WRITE_CTRL pXCArgs = (pstXC_AUTODOWNLOAD_WRITE_CTRL) pArgs;
            pXCArgs->eReturnValue = psXCInstPri->fpXC_AutoDownloadWrite(pInstance,pXCArgs->pstDataInfo);

            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }
        case E_XC_CMD_AUTODOWNLOAD_FIRE_CTRL:
        {
            pstXC_AUTODOWNLOAD_FIRE_CTRL pXCArgs = (pstXC_AUTODOWNLOAD_FIRE_CTRL) pArgs;
            pXCArgs->eReturnValue = psXCInstPri->fpXC_AutoDownloadFire(pInstance,pXCArgs->enClient);

            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }
#endif
#ifdef UFO_XC_HDR
#if (UFO_XC_HDR_VERSION == 2)
        case E_XC_CMD_HDR_CTRL:
        {
            pstXC_HDR_CTRL pXCArgs = (pstXC_HDR_CTRL) pArgs;
            pXCArgs->eReturnValue = psXCInstPri->fpXC_HDRControl(pInstance,pXCArgs->enCtrlType,pXCArgs->pParam);

            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }
        case E_XC_CMD_GET_OTT_SHMINFO:
        {
            pstXC_GETOTTSHMINFO pXCArgs = (pstXC_GETOTTSHMINFO) pArgs;
            pXCArgs->eReturnValue = psXCInstPri->fpXC_GetOttShmInfo(pInstance,pXCArgs->pstShareMemInfo);

            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }
#endif
#endif
#ifdef UFO_XC_SETBLACKVIDEOBYMODE
        case E_XC_CMD_SET_BLACKVIDEO_BYMODE:
            {
                pstXC_SET_BLACKVIDEO_BYMODE pXCArgs = (pstXC_SET_BLACKVIDEO_BYMODE) pArgs;
                pXCArgs->eReturnValue = psXCInstPri->fpXC_GenerateBlackVideoByMode(pInstance, pXCArgs->bEnable, pXCArgs->eWindow, pXCArgs->eMode);

                u32Return = UTOPIA_STATUS_SUCCESS;
                break;
            }
#endif

        case E_XC_CMD_SWDS_FIRE:
            {
                pstXC_SWDS_FIRE pXCArgs = (pstXC_SWDS_FIRE) pArgs;
                psXCInstPri->fpXC_swds_fire(pInstance, pXCArgs->eWindow);

                u32Return = UTOPIA_STATUS_SUCCESS;
                break;
            }
        case E_XC_CMD_OPTEE_CTL:
        {
            pstXC_OPTEE_CTRL pXCArgs = (pstXC_OPTEE_CTRL) pArgs;
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
            if(pXCArgs->xc_handler->eWindow == SUB_WINDOW)
            {
                CHECK_DEVICE1_INSTANCE();
                pInstance = g_pDevice1Instance;
                pXCArgs->xc_handler->eWindow = MAIN_WINDOW;
            }
#endif
            pXCArgs->bReturn = psXCInstPri->fpXC_OPTEE_Ctrl(pInstance,pXCArgs->action, pXCArgs->xc_handler);
            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }
        case E_XC_CMD_OPTEE_CHECKRBASE:
        {
            pstXC_OPTEE_CheckRBase pXCArgs = (pstXC_OPTEE_CheckRBase) pArgs;
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
            if(pXCArgs->eWindow == SUB_WINDOW)
            {
                CHECK_DEVICE1_INSTANCE();
                pInstance = g_pDevice1Instance;
                pXCArgs->eWindow = MAIN_WINDOW;
            }
#endif
            pXCArgs->bReturn = psXCInstPri->fpXC_OPTEE_CheckRBase(pInstance,pXCArgs->eWindow);
            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }
        case E_XC_CMD_OPTEE_MUX:
        {
            pstXC_OPTEE_MUX pXCArgs = (pstXC_OPTEE_MUX) pArgs;
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
            if(pXCArgs->xc_mux_data.eWindow == SUB_WINDOW)
            {
                CHECK_DEVICE1_INSTANCE();
                pInstance = g_pDevice1Instance;
                pXCArgs->xc_mux_data.eWindow = MAIN_WINDOW;
            }
#endif
            pXCArgs->bReturn = psXCInstPri->fpXC_OPTEE_Mux(pInstance,pXCArgs->action, pXCArgs->xc_mux_data);
            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }
        case E_XC_CMD_GET_RES_BY_PIPE:
        {
            pstXC_GetResourceByPipeID pXCArgs = (pstXC_GetResourceByPipeID) pArgs;
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
            if(pXCArgs->u32PipeID == pipeID_SC1_main)
            {
                CHECK_DEVICE1_INSTANCE();
                pInstance = g_pDevice1Instance;
            }
#endif
            pXCArgs->u32ReturnValue = psXCInstPri->fpXC_GetResourceByPipeID(pInstance,pXCArgs->u32PipeID,pXCArgs->U32RetResNum,pXCArgs->pResource);

            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }
        case E_XC_CMD_CONFIG_PIPE:
        {
            pstXC_ConfigPipe pXCArgs = (pstXC_ConfigPipe) pArgs;
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
            if(pXCArgs->u32PipeID == pipeID_SC1_main)
            {
                CHECK_DEVICE1_INSTANCE();
                pInstance = g_pDevice1Instance;
            }
#endif
            pXCArgs->u32ReturnValue = psXCInstPri->fpXC_ConfigPipe(pInstance,pXCArgs->u32PipeID,pXCArgs->u32SecureDMA,pXCArgs->u32OperationMode);

            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }
        case E_XC_CMD_CHECK_PIPE:
        {
            pstXC_CheckPipe pXCArgs = (pstXC_CheckPipe) pArgs;
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
            if(pXCArgs->u32PipeID == pipeID_SC1_main)
            {
                CHECK_DEVICE1_INSTANCE();
                pInstance = g_pDevice1Instance;
            }
#endif
            pXCArgs->u32ReturnValue = psXCInstPri->fpXC_CheckPipe(pInstance,pXCArgs->u32PipeID,pXCArgs->u32SecureDMA,pXCArgs->u32OperationMode);

            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }
        case E_XC_CMD_SET_FB_LEVEL:
        {
            pstXC_SET_FB_LEVEL pXCArgs = (pstXC_SET_FB_LEVEL) pArgs;
            pXCArgs->eReturnValue = psXCInstPri->fpXC_SetFBLevel(pInstance, pXCArgs->eFBLevel, pXCArgs->eWindow);
            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }
        case E_XC_CMD_GET_FB_LEVEL:
        {
            pstXC_GET_FB_LEVEL pXCArgs = (pstXC_GET_FB_LEVEL) pArgs;
            pXCArgs->eReturnValue = psXCInstPri->fpXC_GetFBLevel(pInstance, pXCArgs->eWindow);

            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }
        case E_XC_CMD_GET_CONFIG:
        {
            pstXC_GET_CONFIG pXCArgs = (pstXC_GET_CONFIG) pArgs;
            pXCArgs->eReturnValue = psXCInstPri->fpXC_GetConfig(pInstance,pXCArgs->pXC_InitData);
            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }


        case E_XC_CMD_GET_FDMASK_STATUS:
        {


            pstXC_GET_FDMASK_STATUS pXCArgs = (pstXC_GET_FDMASK_STATUS) pArgs;
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
            USE_OTHER_SC_AS_SUB_WINDOW();
#endif
            pXCArgs->bReturnValue = psXCInstPri->fpXC_Get_FD_Mask_Status(pInstance, pXCArgs->eWindow);


            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }

        case E_XC_CMD_SET_SWDR_INFO:
        {
            pstXC_SWDR_INFO pXCArgs = (pstXC_SWDR_INFO) pArgs;
            pXCArgs->bReturnValue = psXCInstPri->fpXC_Set_SWDR_Info(pInstance,pXCArgs->pSWDR_INFO);
            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }
        case E_XC_CMD_GET_SWDR_INFO:
        {
            pstXC_SWDR_INFO pXCArgs = (pstXC_SWDR_INFO) pArgs;
            pXCArgs->bReturnValue = psXCInstPri->fpXC_Get_SWDR_Info(pInstance,pXCArgs->pSWDR_INFO);
            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }

        case E_XC_CMD_SET_SWDR_INPUTDATA:
        {
            pstXC_SWDR_INPUTDATA pXCArgs = (pstXC_SWDR_INPUTDATA) pArgs;
            pXCArgs->bReturnValue = psXCInstPri->fpXC_Set_SWDR_Input(pInstance,pXCArgs->pSWDRInput,pXCArgs->eWindow);
            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }
#ifdef UFO_XC_SUPPORT_HDMI_DOWNSCALE_OUTPUT_POLICY
        case E_XC_CMD_GET_HDMIPOLICY:
        {

            pstXC_GET_HDMIPOLICY pXCArgs = (pstXC_GET_HDMIPOLICY) pArgs;
            pXCArgs-> u32HDMIPolicyInfo = psXCInstPri->fpXC_GetHDMIPolicy(pInstance);
            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }
#endif
        case E_XC_CMD_GET_DSINFO:
        {
            pstXC_GET_DSINFO pXCArgs = (pstXC_GET_DSINFO) pArgs;
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
            USE_OTHER_SC_AS_SUB_WINDOW();
#endif
            psXCInstPri->fpXC_GetDSInfo(pInstance,pXCArgs->pstGetDSInfo, pXCArgs->u32DSInfoLen, pXCArgs->eWindow);

            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }
        case E_XC_GET_DS_VERSION:
        {
            pstXC_GET_DS_VERSION pXCArgs = (pstXC_GET_DS_VERSION) pArgs;
            pXCArgs->u16DSVersion = psXCInstPri->fpXC_Get_DS_Version(pInstance);

            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }
#ifdef ENABLE_XC_WRITE_CMD_API
         case E_XC_WRITE_CMD:
        {
            pstXC_WRITE_CMD pXCArgs = (pstXC_WRITE_CMD) pArgs;
            pXCArgs->eReturnValue = psXCInstPri->fpXC_WriteCmd(pInstance,pXCArgs->pstXCWriteCmd);

            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }
         case E_XC_SWDS_FIRE:
        {
            pstXC_SWDSFIRE pXCArgs = (pstXC_SWDSFIRE) pArgs;
            pXCArgs->eReturnValue = psXCInstPri->fpXC_SWDSFire(pInstance,pXCArgs->eWindow);

            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }
#endif
         case E_XC_CMD_WRITE_AND_FIRE_REGIN_VSYNC_BLANK:
        {
            pstXC_WRITEANDFIREREGINVSYNCBLANK pXCArgs = (pstXC_WRITEANDFIREREGINVSYNCBLANK) pArgs;
            pXCArgs->eReturnValue = psXCInstPri->fpXC_WriteAndFireReginVsyncBlank(pInstance,pXCArgs->pstMenuloadInfo,pXCArgs->enWindow);

            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }
         case E_XC_GET_DNRBUFFERSIZE:
        {
            pstXC_GETDNRBUFFERSIZE pXCArgs = (pstXC_GETDNRBUFFERSIZE) pArgs;
            pXCArgs->eReturnValue = psXCInstPri->fpXC_GetDNRBufferSize(pInstance,pXCArgs->eWindow,pXCArgs->pu32BufferSize);

            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }
        case E_XC_CMD_SET_PQBIN_PATH:
        {
            pstXC_SETPQBINPATH pXCArgs = (pstXC_SETPQBINPATH)pArgs;
            psXCInstPri->fpXC_SetPQBinPath(pInstance,pXCArgs->eXCPqBinPath, pXCArgs->u8size, pXCArgs->b_PQBinFilePath);
            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }
        case E_XC_CMD_SET_MEMORY_ADDRESS:
        {
            pstXC_SET_MEMORY_ADDRESS pXCArgs = (pstXC_SET_MEMORY_ADDRESS)pArgs;
            pXCArgs->eReturnValue = psXCInstPri->fpXC_Set_MemoryAddress(pInstance, pXCArgs->pstMemory_Info);
            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }
        case E_XC_CONNECTDISPLAY:
        {
            pstXC_CONNECTDISPLAY pXCArgs = (pstXC_CONNECTDISPLAY) pArgs;
            pXCArgs->enReturnValue =psXCInstPri->fpXC_ConnectDisplay(pInstance,pXCArgs->pstXCDispaly, pXCArgs->enWindow);
            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }
        case E_XC_CMD_SET_FRAME_INFO:
        {
            pstXC_FRAME_INFO pXCArgs = (pstXC_FRAME_INFO)pArgs;
            pXCArgs->bReturnValue = psXCInstPri->fpXC_SetFrameInfo(pInstance, pXCArgs->pstFrameInfo, pXCArgs->eWindow);
            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }
        case E_XC_CMD_PQ_GAMMA:
        {
            pstXC_PQ_GAMMA_IN pXCArgs = (pstXC_PQ_GAMMA_IN)pArgs;
            pXCArgs->enReturnValue = psXCInstPri->fpXC_PQ_Gamma(pInstance, pXCArgs->pstGammaIn);
            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }
        case E_XC_CMD_PQ_RGB_PATTERN_IN:
        {
            pstXC_PQ_RGB_PATTERN_IN pXCArgs = (pstXC_PQ_RGB_PATTERN_IN)pArgs;
            pXCArgs->enReturnValue = psXCInstPri->fpXC_PQ_RGBPattern(pInstance, pXCArgs->pstRGBPatternIn);
            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }
        case E_XC_CMD_PQ_BLUE_STRETCH_IN:
        {
            pstXC_PQ_BLUE_STRETCH_IN pXCArgs = (pstXC_PQ_BLUE_STRETCH_IN)pArgs;
            pXCArgs->enReturnValue = psXCInstPri->fpXC_PQ_BlueStretch(pInstance, pXCArgs->pstBlueStretchIn);
            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }
        case E_XC_CMD_GET_VIDEO_CAPTURE_INFO:
        {
            pstXC_VIDEO_CAPTURE_INFO  pXCArgs = (pstXC_VIDEO_CAPTURE_INFO) pArgs;
#if PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB
            USE_OTHER_SC_AS_SUB_WINDOW();
#endif
            pXCArgs->enReturnValue = psXCInstPri->fpXC_GetVideoCaptureInfo(pInstance,pXCArgs->pDrvStatus, pXCArgs->eWindow);

            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }
        case E_XC_CMD_SET_H_NONLINEAR_SCALING:
        {
            pstXC_H_NONLINEAR_SCALING_IN pXCArgs = (pstXC_H_NONLINEAR_SCALING_IN)pArgs;
            pXCArgs->enReturnValue = psXCInstPri->fpXC_PQ_SetHNonlinearScaling(pInstance, pXCArgs->pstHNLIn);
            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }
        case E_XC_CMD_MEM_ENABLE:
        {
            pstXC_MEM_ENABLE pXCArgs = (pstXC_MEM_ENABLE) pArgs;
            pXCArgs->eReturnValue = psXCInstPri->fpXC_MemoryEnable(pInstance,pXCArgs->pstMemEnableCtrl);
            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }
        case E_XC_CMD_GET_VIDEO_LATENCY:
        {
            pstXC_VIDEO_LATENCY_INFO pXCArgs = (pstXC_VIDEO_LATENCY_INFO) pArgs;
            pXCArgs->enReturnValue = psXCInstPri->fpXC_GetVideoLatency(pInstance,pXCArgs->pstVideoLatencyInfo, pXCArgs->enWindow);
            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }
        case E_XC_CMD_GET_IP_STATUS:
        {
            pstXC_GETIPSTATUS pXCArgs = (pstXC_GETIPSTATUS)pArgs;
            pXCArgs->enReturnValue = psXCInstPri->fpXC_GetIPSTATUS(pInstance,pXCArgs->pstGetIPStatus);
            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }
        case E_XC_CMD_TEST_FUNCTION:
        {
            pstXC_TEST_FUNCTION pXCArgs = (pstXC_TEST_FUNCTION)pArgs;
            pXCArgs->enReturnValue = psXCInstPri->fpXC_TestFunction(pInstance, pXCArgs->pstXCTestFunction);
            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }
        case E_XC_CMD_SET_SCALER_BW_LEVEL:
        {
            pstXC_QOS_SCALER_BW_INFO pXCArgs = (pstXC_QOS_SCALER_BW_INFO)pArgs;
            pXCArgs->enReturnValue = psXCInstPri->fpXC_QOS_SetScalerBWInfo(pInstance, pXCArgs->pstQosBWInfo);
            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }
        case E_XC_CMD_SET_FRC_BW_LEVEL:
        {
            pstXC_QOS_FRC_BW_INFO pXCArgs = (pstXC_QOS_FRC_BW_INFO)pArgs;
            pXCArgs->enReturnValue = psXCInstPri->fpXC_QOS_SetFRCBWInfo(pInstance, pXCArgs->pstQosBWInfo);
            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }
        case E_XC_CMD_SET_QOS_CALLBACKFUN:
        {
            pstXC_QOS_CALLBACK_INFO pXCArgs = (pstXC_QOS_CALLBACK_INFO)pArgs;
            pXCArgs->enReturnValue = psXCInstPri->fpXC_QOS_CallBackInfo(pInstance, pXCArgs->pstCallBack);
            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }
        case E_XC_CMD_SET_LOWLATENCYMODE:
        {
            pstXC_SET_LOWLATENCYMODE pXCArgs = (pstXC_SET_LOWLATENCYMODE) pArgs;
            psXCInstPri->fpXC_SetLowLatencyMode(pInstance,pXCArgs->pstSetLowLatencyModeInfo,pXCArgs->u8WinId);
            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }
        case E_XC_CMD_SET_VRRON:
        {
            pstXC_SET_VRRON pXCArgs = (pstXC_SET_VRRON) pArgs;
            pXCArgs->enReturnValue = psXCInstPri->fpXC_SetVRROn(pInstance, pXCArgs->bEnable);
            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }
        case E_XC_CMD_CUSSTR_ENABLE:
        {
            pstXC_CUSSTR_ENABLE pXCArgs = (pstXC_CUSSTR_ENABLE) pArgs;
            pXCArgs->enReturnValue = psXCInstPri->fpXC_CusSTR_Enable(pInstance, pXCArgs->bEnable);
            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }
        case E_XC_CMD_GET_CUSSTRSTATUS:
        {
            pstXC_GET_CUSSTRSTATUS pXCArgs = (pstXC_GET_CUSSTRSTATUS) pArgs;
            pXCArgs->bEnable = psXCInstPri->fpXC_Get_CusSTRStatus(pInstance);
            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }
        case E_XC_CMD_SET_RW_POINT_DIFF:
        {
            pstXC_SET_RWPOINTDIFF pXCArgs = (pstXC_SET_RWPOINTDIFF) pArgs;
            pXCArgs->enReturnValue = psXCInstPri->fpXC_SetRWPointDiff(pInstance, pXCArgs->stRWPointDiff);
            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }
        case E_XC_CMD_SRC_SIG_CONTROL:
        {
            pstXC_SRCSIG_CTL pXCArgs = (pstXC_SRCSIG_CTL) pArgs;
            pXCArgs->enReturnValue = psXCInstPri->fpXC_SrcSig_CTL(pInstance, pXCArgs->pstSrcSigCTL);
            u32Return = UTOPIA_STATUS_SUCCESS;
            break;
        }
// OBSOLETE command **END**, please avoid using these
        case E_XC_CMD_NONE:
        case E_XC_CMD_MAX:
        default:
        {
            printf("Command %td is not existed\n",(ptrdiff_t)u32Cmd);
            u32Return = UTOPIA_STATUS_ERR_INVALID_HANDLE;
            break;
        }
    }
    return u32Return;
}

MS_U32 XCClose(void* pInstance)
{
#if (PIP_PATCH_USING_SC1_MAIN_AS_SC0_SUB)
#ifdef MSOS_TYPE_LINUX_KERNEL
    if(utopia_delete_xc_instance_table(pInstance))
    {
        printf("Do nothing\n");
    }
#endif
#endif
    UtopiaInstanceDelete(pInstance);
    return UTOPIA_STATUS_SUCCESS;
}


#undef _XC_C_
