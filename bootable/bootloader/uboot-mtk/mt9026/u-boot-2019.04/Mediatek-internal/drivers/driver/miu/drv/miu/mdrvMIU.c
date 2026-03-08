#include "MsTypes.h"
#ifdef MSOS_TYPE_LINUX_KERNEL
#include <linux/module.h>   /* Needed by all modules */
#include <linux/kernel.h>
#include <linux/slab.h>
#else
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#endif
#include "utopia_dapi.h"
#include "utopia.h"
#include "drvMIU.h"
#include "drvMIU_private.h"
#include "MsOS.h"
#include "ULog.h"

#define MIUDBG(x)
#define TAG_MIU "MIU"

enum
{
    MIU_POOL_ID_MIU0=0
} eMIUPoolID;

#ifdef CONFIG_UTOPIA_PROC_DBG_SUPPORT
MS_U32 MIUMdbIoctl(MS_U32 cmd, const void* const pArgs)
{
    MDBCMD_CMDLINE_PARAMETER *paraCmdLine;
    MDBCMD_GETINFO_PARAMETER *paraGetInfo;
    MDBCMD_CMDLINE_PARAMETER *pstparaCmdLine;
    EN_MIU_MDBCMDLINE eMdbCmdLine = MIU_MDBCMDLINE_NONE;
    char *strMIUBandwidth[] = {"status", "v1", "v2", "v3"};
    char *strMIUDDRPhase[] = {"err_check", "dram_size", "dram_clock"};
    MS_U32 strMIUMdbCmd_index;
    char *strMIUMdbCmd[] = {"help", "miu_protect", "miu_select", "miu_mask", "miu_BW", "ddr_phase", "dbg_level"};

    switch(cmd)
    {
        case MDBCMD_CMDLINE:
            paraCmdLine = (MDBCMD_CMDLINE_PARAMETER *)pArgs;
//            MdbPrint(paraCmdLine->u64ReqHdl,"LINE:%d, MDBCMD_CMDLINE\n", __LINE__);
//            MdbPrint(paraCmdLine->u64ReqHdl,"u32CmdSize: %d\n", paraCmdLine->u32CmdSize);
//            MdbPrint(paraCmdLine->u64ReqHdl,"pcCmdLine: %s\n", paraCmdLine->pcCmdLine);
/////////
            for(strMIUMdbCmd_index = 0; strMIUMdbCmd_index<(sizeof(strMIUMdbCmd)/sizeof(char*)); strMIUMdbCmd_index++)
            {
                char *pstr;
                char *pstr_sub;
                MS_U32 u32index;
                pstr = strstr(paraCmdLine->pcCmdLine, strMIUMdbCmd[strMIUMdbCmd_index]);
                if(pstr != NULL)
                {
                    switch(strMIUMdbCmd_index)
                    {
                        case 0:
                            eMdbCmdLine = MIU_MDBCMDLINE_HELP;
                            break;
                        case 1:
                            eMdbCmdLine = MIU_MDBCMDLINE_MIU_PROTECT;
                            break;
                        case 2:
                            eMdbCmdLine = MIU_MDBCMDLINE_MIU_SELECT;
                            break;
                        case 3:
                            eMdbCmdLine = MIU_MDBCMDLINE_MIU_MASK;
                            break;
                        case 4:
                            for( u32index = 0; u32index < (sizeof(strMIUBandwidth)/sizeof(char*)); u32index++)
                            {
                                pstr_sub = strstr(paraCmdLine->pcCmdLine, strMIUBandwidth[u32index]);
                                if(pstr_sub != NULL)
                                {
                                    if(u32index == 0)
                                        eMdbCmdLine = MIU_MDBCMDLINE_MIU_BANDWIDTH_BANKINFO;
                                    else if(u32index == 1 || u32index == 2)
                                        eMdbCmdLine = MIU_MDBCMDLINE_MIU_BANDWIDTH_LOADING;
                                    else if(u32index == 3)
                                        eMdbCmdLine = MIU_MDBCMDLINE_MIU_BW_MONITOR_ARB3;
                                    break;
                                }
                            }
                            break;
                        case 5:
                            for( u32index = 0; u32index < (sizeof(strMIUDDRPhase)/sizeof(char*)); u32index++)
                            {
                                pstr_sub = strstr(paraCmdLine->pcCmdLine, strMIUDDRPhase[u32index]);
                                if(pstr_sub != NULL)
                                {

                                    if(u32index == 0)
                                        eMdbCmdLine = MIU_MDBCMDLINE_MIU_DDR_PHASE_ERR_CHECK;
                                    else if(u32index == 1)
                                        eMdbCmdLine = MIU_MDBCMDLINE_MIU_DRAMSIZE;
                                    else if(u32index == 2)
                                        eMdbCmdLine = MIU_MDBCMDLINE_MIU_DRAMCLOCK;
                                    break;
                                }
                            }
                            break;
                        case 6:
                            eMdbCmdLine = MIU_MDBCMDLINE_MIU_SET_DBG_LEVEL;
                            break;
                        default:
                            eMdbCmdLine = MIU_MDBCMDLINE_NONE;
                            break;
                    }
                    break;
                }
            }
            MDrv_MIU_MdbCmdLine(paraCmdLine, eMdbCmdLine);
            //////////
            paraCmdLine->result = MDBRESULT_SUCCESS_FIN;
            break;
        case MDBCMD_GETINFO:
            paraGetInfo = (MDBCMD_GETINFO_PARAMETER *)pArgs;
            pstparaCmdLine = (MDBCMD_CMDLINE_PARAMETER*)malloc(sizeof(MDBCMD_CMDLINE_PARAMETER));
            if(pstparaCmdLine == NULL)
            {
                MdbPrint(paraGetInfo->u64ReqHdl,"[PROC ERROR][malloc fail]\n");
                break;
            }
            memcpy(pstparaCmdLine, pArgs, sizeof(MDBCMD_CMDLINE_PARAMETER));
            strcpy(pstparaCmdLine->pcCmdLine, "miu_protect");
            MDrv_MIU_MdbCmdLine(pstparaCmdLine, MIU_MDBCMDLINE_MIU_PROTECT);
            strcpy(pstparaCmdLine->pcCmdLine, "dram_size");
            MDrv_MIU_MdbCmdLine(pstparaCmdLine, MIU_MDBCMDLINE_MIU_DRAMSIZE);
            paraGetInfo->result = MDBRESULT_SUCCESS_FIN;
            free(pstparaCmdLine);
            break;
        default:
            paraGetInfo = (MDBCMD_GETINFO_PARAMETER *)pArgs;
            MdbPrint(paraGetInfo->u64ReqHdl,"unknown cmd\n");
            break;
    }
    return 0;
}
#endif

MS_U32 MIUStr(MS_U32 u32PowerState, void* pModule)
{
    MS_U32 u32Return = UTOPIA_STATUS_SUCCESS;
// UTOPIA remove so dummy code
#ifndef CONFIG_UTOPIAXP_REMOVE_IMPL
    u32Return =  MDrv_MIU_SetPowerState((EN_POWER_MODE)u32PowerState);

    UtopiaStrSendCondition("miu",u32PowerState ,0);
#endif
    return u32Return;
}

// this func will be call to init by utopia20 framework
void MIURegisterToUtopia(FUtopiaOpen ModuleType)
{
    // 1. deal with module
    void* pUtopiaModule = NULL;
    UtopiaModuleCreate(MODULE_MIU, 8, &pUtopiaModule);
    UtopiaModuleRegister(pUtopiaModule);
    // register func for module, after register here, then ap call UtopiaOpen/UtopiaIoctl/UtopiaClose can call to these registered standard func
    UtopiaModuleSetupFunctionPtr(pUtopiaModule, (FUtopiaOpen)MIUOpen, (FUtopiaClose)MIUClose, (FUtopiaIOctl)MIUIoctl);
    #ifdef CONFIG_UTOPIA_PROC_DBG_SUPPORT
    UtopiaModuleRegisterMdbNode("miu", (FUtopiaMdbIoctl)MIUMdbIoctl);
    #endif

    #if defined(MSOS_TYPE_LINUX_KERNEL)
    UtopiaModuleSetupSTRFunctionPtr(pUtopiaModule, (FUtopiaSTR)MIUStr);
    #endif

    // 2. deal with resource
    void* psResource = NULL;
    // start func to add res, call once will create 2 access in resource.
    UtopiaModuleAddResourceStart(pUtopiaModule, MIU_POOL_ID_MIU0);
    // resource can alloc private for internal use, ex, MIU_RESOURCE_PRIVATE
    UtopiaResourceCreate("miu0", sizeof(MIU_RESOURCE_PRIVATE), &psResource);
    // func to reg res
    UtopiaResourceRegister(pUtopiaModule, psResource, MIU_POOL_ID_MIU0);

    // end function to add res
    UtopiaModuleAddResourceEnd(pUtopiaModule, MIU_POOL_ID_MIU0);
    // MIURegisterToUtopia only call once, so driver init code can put here,
    //MDrv_MIU_Init();
}

MS_U32 MIUOpen(void** ppInstance, MS_U32 u32ModuleVersion, void* pAttribute)
{
    ULOGD(TAG_MIU, "[MIU INFO] miu open\n");
// UTOPIA remove so dummy code
#ifndef CONFIG_UTOPIAXP_REMOVE_IMPL
    MIU_INSTANT_PRIVATE *pMIUPri = NULL;
    void *pMIUPriVoid = NULL;
    //UTOPIA_TRACE(MS_UTOPIA_DB_LEVEL_TRACE,printf("enter %s %d\n",__FUNCTION__,__LINE__));
    // instance is allocated here, also can allocate private for internal use, ex, MIU_INSTANT_PRIVATE
    UtopiaInstanceCreate(sizeof(MIU_INSTANT_PRIVATE), ppInstance);
    // setup func in private and assign the calling func in func ptr in instance private
    UtopiaInstanceGetPrivate(*ppInstance, &pMIUPriVoid);
    pMIUPri = (MIU_INSTANT_PRIVATE*)pMIUPriVoid;

    pMIUPri->fpMIU_Init = (IOCTL_MIU_INIT)_MDrv_MIU_Init;
    pMIUPri->fpMIU_Mask_Req_OPM_R = (IOCTL_MIU_MASK_IP)_MDrv_MIU_Mask_Req_OPM_R;
    pMIUPri->fpMIU_Mask_Req_DNRB_R = (IOCTL_MIU_MASK_IP)_MDrv_MIU_Mask_Req_DNRB_R;
    pMIUPri->fpMIU_Mask_Req_DNRB_W = (IOCTL_MIU_MASK_IP)_MDrv_MIU_Mask_Req_DNRB_W;
    pMIUPri->fpMIU_Mask_Req_DNRB_RW = (IOCTL_MIU_MASK_IP)_MDrv_MIU_Mask_Req_DNRB_RW;
    pMIUPri->fpMIU_Mask_Req_SC_RW = (IOCTL_MIU_MASK_IP)_MDrv_MIU_Mask_Req_SC_RW;
    pMIUPri->fpMIU_Mask_Req_MVOP_R = (IOCTL_MIU_MASK_IP)_MDrv_MIU_Mask_Req_MVOP_R;
    pMIUPri->fpMIU_Mask_Req_MVD_R = (IOCTL_MIU_MASK_IP)_MDrv_MIU_Mask_Req_MVD_R;
    pMIUPri->fpMIU_Mask_Req_MVD_W = (IOCTL_MIU_MASK_IP)_MDrv_MIU_Mask_Req_MVD_W;
    pMIUPri->fpMIU_Mask_Req_MVD_RW = (IOCTL_MIU_MASK_IP)_MDrv_MIU_Mask_Req_MVD_RW;
    pMIUPri->fpMIU_Mask_Req_AUDIO_RW = (IOCTL_MIU_MASK_IP)_MDrv_MIU_Mask_Req_AUDIO_RW;
    pMIUPri->fpMIU_Mask_Req = (IOCTL_MIU_MASK_REQ)_MDrv_MIU_MaskReq;
    pMIUPri->fpMIU_UnMask_Req = (IOCTL_MIU_MASK_REQ)_MDrv_MIU_UnMaskReq;
    pMIUPri->fpMIU_SetSsc = (IOCTL_MIU_SET_SSC)_MDrv_MIU_SetSsc;
    pMIUPri->fpMIU_SetSscValue = (IOCTL_MIU_SET_SSCVALUE)_MDrv_MIU_SetSscValue;
    pMIUPri->fpMIU_Protect = (IOCTL_MIU_PROTECT)_MDrv_MIU_Protect;
    pMIUPri->fpMIU_ProtectEx = (IOCTL_MIU_PROTECTEx)_MDrv_MIU_ProtectEx;
    pMIUPri->fpMIU_SelMIU = (IOCTL_MIU_SELMIU)_MDrv_MIU_SelMIU;
    pMIUPri->fpMIU_GetProtectInfo = (IOCTL_MIU_GETPROTECTINFO)_MDrv_MIU_GetProtectInfo;
    pMIUPri->fpMIU_SetGroupPriority = (IOCTL_MIU_SETGROUPPRIORITY)_MDrv_MIU_SetGroupPriority;
    pMIUPri->fpMIU_SetHighPriority = (IOCTL_MIU_SETHIGHPRIORITY)_MDrv_MIU_SetHPriorityMask;
    pMIUPri->fpMIU_PrintProtectInfo = (IOCTL_MIU_PRINTPROTECTINFO)_MDrv_MIU_PrintProtectInfo;
    pMIUPri->fpMIU_Dram_Size = (IOCTL_MIU_DRAM_SIZE)_MDrv_MIU_Dram_Size;
    pMIUPri->fpMIU_ProtectAlign = (IOCTL_MIU_PROTECTALIGN)_MDrv_MIU_ProtectAlign;
    pMIUPri->fpMIU_GetDramType = (IOCTL_MIU_GETDRAMTYPE)_MDrv_MIU_GetDramType;
    pMIUPri->fpMIU_IsSupportMIU1 = (IOCTL_MIU_ISSUPPORTMIU1)_MDrv_MIU_IsSupportMIU1;
    pMIUPri->fpMIU_SetBWMonitor = (IOCTL_MIU_BWMONITOR)_MDrv_MIU_Set_BW_Monitor;
    pMIUPri->fpMIU_DelBWMonitor = (IOCTL_MIU_BWMONITOR)_MDrv_MIU_Del_BW_Monitor;
    pMIUPri->fpMIU_QueryBWMonitor = (IOCTL_MIU_BWMONITOR)_MDrv_MIU_Query_BW_Monitor;
    pMIUPri->fpMIU_GetBWMonitor = (IOCTL_MIU_BWMONITOR)_MDrv_MIU_Get_BW;
    pMIUPri->fpMIU_ResetBWMonitor = (IOCTL_MIU_BWMONITORRESET)_MDrv_MIU_Reset_All_BW_Monitor;
    pMIUPri->fpMIU_ConfigHWAttr = (IOCTL_MIU_ConfigHWAttr)_MDrv_MIU_ConfigHWAttr;
    pMIUPri->fpMIU_SetClientUtil = (IOCTL_MIU_SETCLIENTUTIL)_MDrv_MIU_SetClientUtilization;
    pMIUPri->fpMIU_GetClientUtil = (IOCTL_MIU_GETCLIENTUTIL)_MDrv_MIU_GetClientUtilization;
    pMIUPri->fpMIU_SetClientEffi = (IOCTL_MIU_SETCLIENTEFFI)_MDrv_MIU_SetClientEfficiency;
    pMIUPri->fpMIU_GetClientEffi = (IOCTL_MIU_GETCLIENTEFFI)_MDrv_MIU_GetClientEfficiency;
    pMIUPri->fpMIU_SetBWValue = (IOCTL_MIU_SETBWVALUE)_MDrv_MIU_SetBWValue;
    pMIUPri->fpMIU_GetBWValue = (IOCTL_MIU_GETBWVALUE)_MDrv_MIU_GetBWValue;
    pMIUPri->fpMIU_GetMIUInfo = (IOCTL_MIU_GETMIUINFO)_MDrv_MIU_GetMIUInfo;
    pMIUPri->fpMIU_GetBWMonitorAll = (IOCTL_MIU_BWMONITOR)_MDrv_MIU_Get_BW_All;
    pMIUPri->fpMIU_StartBWMonitor = (IOCTL_MIU_BWMONITORRESET)_MDrv_MIU_Start_BW_Monitor;
    pMIUPri->fpMIU_SetCPUHpri = (IOCTL_MIU_SETCPUHPRI)_MDrv_MIU_SetCPUHPriority;
#endif
    return UTOPIA_STATUS_SUCCESS;
}

// FIXME: why static?
MS_U32 MIUIoctl(void* pInstance, MS_U32 u32Cmd, void* pArgs)
{
// UTOPIA remove so dummy code
#ifndef CONFIG_UTOPIAXP_REMOVE_IMPL
    void* pModule = NULL;
    UtopiaInstanceGetModule(pInstance, &pModule);
    void* pResource = NULL;
    PMIU_MIU_MASK pMIUMask;
    PMIU_MIUREQ_MASK pMIUMaskReq;
    PMIU_SET_SSC pMIUSetSsc;
    PMIU_SET_SSCVALUE pMIUSetSscValue;
    PMIU_PROTECT pMIUProtect;
    PMIU_PROTECTEx pMIUProtectEx;
    PMIU_SELMIU pMIUSelMIU;
    PMIU_GETPROTECTINFO pMIUGetProtectInfo;
    PMIU_SETGROUPPRIORITY pMIUSetGroupPri;
    PMIU_SETHIGHPRIORITY pMIUSetHighPri;
    PMIU_DRAM_SIZE pMIUDramSizePri;
    PMIU_PROTECT_ALIGN pMIUProtectAlignPri;
    PMIU_GETDRAMTYPE pMIUGetDramTypePri;
    PMIU_BWMONITOR pMIU_BWMONITOR;
    PMIU_ConfigHWAttr pMIUConfigHWAttr;
    PMIU_BWUTILIZATION pMIU_BWUTILIZATION;
    PMIU_BWEFFICIENCY pMIU_BWEFFICIENCY;
    PMIU_SETGETBW pMIU_BWVALUE;
    PMIU_GETINFO pMIU_DRAMINFO;
    PMIU_SETCPUHPRI pMIU_CPUHPri;
    MS_BOOL bRet = FALSE;
    //MS_U32 u32Ret;

    //utopia_secure_check(pInstant->pPrivate); // FIXME: check what?

    //if (NULL == pArgs)
    //    return UTOPIA_STATUS_PARAMETER_ERROR;

    MIU_INSTANT_PRIVATE* psMIUInstPri = NULL;
    void* psMIUInstPriVoid = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void**)&psMIUInstPriVoid);
    psMIUInstPri = (MIU_INSTANT_PRIVATE*)psMIUInstPriVoid;

    switch(u32Cmd)
    {
        case MDrv_CMD_MIU_Init:
            MIUDBG("MIUIoctl - MDrv_CMD_MIU_Init\n");
            // when calling func occpuy h/w, need to call UtopiaResourceObtainToInstant to obtain res to lock other process
            if(UtopiaResourceObtain(pModule, MIU_POOL_ID_MIU0, &pResource) != 0)
            {
                ULOGE(TAG_MIU, "UtopiaResourceObtainToInstant fail\n");
                return UTOPIA_STATUS_ERR_RESOURCE;
            }
            bRet = psMIUInstPri->fpMIU_Init();
            UtopiaResourceRelease(pResource);
            if (TRUE == bRet)
                return UTOPIA_STATUS_SUCCESS;
            else
                return UTOPIA_STATUS_FAIL;

        case MDrv_CMD_MIU_Mask_Req_OPM_R:
            MIUDBG("MIUIoctl - MDrv_CMD_MIU_Mask_Req_OPM_R\n");
            pMIUMask = (PMIU_MIU_MASK)pArgs;
            // when calling func occpuy h/w, need to call UtopiaResourceObtainToInstant to obtain res to lock other process
            if(UtopiaResourceObtain(pModule, MIU_POOL_ID_MIU0, &pResource) != 0)
            {
                ULOGE(TAG_MIU, "UtopiaResourceObtainToInstant fail\n");
                return UTOPIA_STATUS_ERR_RESOURCE;
            }
            psMIUInstPri->fpMIU_Mask_Req_OPM_R(pMIUMask->u8Mask,pMIUMask->u8Miu);
            UtopiaResourceRelease(pResource);
            return UTOPIA_STATUS_SUCCESS;

        case MDrv_CMD_MIU_Mask_Req_DNRB_R:
            MIUDBG("MIUIoctl - MDrv_CMD_MIU_Mask_Req_DNRB_R\n");
            pMIUMask = (PMIU_MIU_MASK)pArgs;
            // when calling func occpuy h/w, need to call UtopiaResourceObtainToInstant to obtain res to lock other process
            if(UtopiaResourceObtain(pModule, MIU_POOL_ID_MIU0, &pResource) != 0)
            {
                ULOGE(TAG_MIU, "UtopiaResourceObtainToInstant fail\n");
                return UTOPIA_STATUS_ERR_RESOURCE;
            }
            psMIUInstPri->fpMIU_Mask_Req_DNRB_R(pMIUMask->u8Mask,pMIUMask->u8Miu);
            UtopiaResourceRelease(pResource);
            return UTOPIA_STATUS_SUCCESS;

        case MDrv_CMD_MIU_Mask_Req_DNRB_W:
            MIUDBG("MIUIoctl - MDrv_CMD_MIU_Mask_Req_DNRB_W\n");
            pMIUMask = (PMIU_MIU_MASK)pArgs;
            // when calling func occpuy h/w, need to call UtopiaResourceObtainToInstant to obtain res to lock other process
            if(UtopiaResourceObtain(pModule, MIU_POOL_ID_MIU0, &pResource) != 0)
            {
                ULOGE(TAG_MIU, "UtopiaResourceObtainToInstant fail\n");
                return UTOPIA_STATUS_ERR_RESOURCE;
            }
            psMIUInstPri->fpMIU_Mask_Req_DNRB_W(pMIUMask->u8Mask,pMIUMask->u8Miu);
            UtopiaResourceRelease(pResource);
            return UTOPIA_STATUS_SUCCESS;

        case MDrv_CMD_MIU_Mask_Req_DNRB_RW:
            MIUDBG("MIUIoctl - MDrv_CMD_MIU_Mask_Req_DNRB_RW\n");
            pMIUMask = (PMIU_MIU_MASK)pArgs;
            // when calling func occpuy h/w, need to call UtopiaResourceObtainToInstant to obtain res to lock other process
            if(UtopiaResourceObtain(pModule, MIU_POOL_ID_MIU0, &pResource) != 0)
            {
                ULOGE(TAG_MIU, "UtopiaResourceObtainToInstant fail\n");
                return UTOPIA_STATUS_ERR_RESOURCE;
            }
            psMIUInstPri->fpMIU_Mask_Req_DNRB_RW(pMIUMask->u8Mask,pMIUMask->u8Miu);
            UtopiaResourceRelease(pResource);
            return UTOPIA_STATUS_SUCCESS;

        case MDrv_CMD_MIU_Mask_Req_SC_RW:
            MIUDBG("MIUIoctl - MDrv_CMD_MIU_Mask_Req_SC_RW\n");
            pMIUMask = (PMIU_MIU_MASK)pArgs;
            // when calling func occpuy h/w, need to call UtopiaResourceObtainToInstant to obtain res to lock other process
            if(UtopiaResourceObtain(pModule, MIU_POOL_ID_MIU0, &pResource) != 0)
            {
                ULOGE(TAG_MIU, "UtopiaResourceObtainToInstant fail\n");
                return UTOPIA_STATUS_ERR_RESOURCE;
            }
            psMIUInstPri->fpMIU_Mask_Req_SC_RW(pMIUMask->u8Mask,pMIUMask->u8Miu);
            UtopiaResourceRelease(pResource);
            return UTOPIA_STATUS_SUCCESS;

        case MDrv_CMD_MIU_Mask_Req_MVOP_R:
            MIUDBG("MIUIoctl - MDrv_CMD_MIU_Mask_Req_MVOP_R\n");
            pMIUMask = (PMIU_MIU_MASK)pArgs;
            // when calling func occpuy h/w, need to call UtopiaResourceObtainToInstant to obtain res to lock other process
            if(UtopiaResourceObtain(pModule, MIU_POOL_ID_MIU0, &pResource) != 0)
            {
                ULOGE(TAG_MIU, "UtopiaResourceObtainToInstant fail\n");
                return UTOPIA_STATUS_ERR_RESOURCE;
            }
            psMIUInstPri->fpMIU_Mask_Req_MVOP_R(pMIUMask->u8Mask,pMIUMask->u8Miu);
            UtopiaResourceRelease(pResource);
            return UTOPIA_STATUS_SUCCESS;

        case MDrv_CMD_MIU_Mask_Req_MVD_R:
            MIUDBG("MIUIoctl - MDrv_CMD_MIU_Mask_Req_MVD_R\n");
            pMIUMask = (PMIU_MIU_MASK)pArgs;
            // when calling func occpuy h/w, need to call UtopiaResourceObtainToInstant to obtain res to lock other process
            if(UtopiaResourceObtain(pModule, MIU_POOL_ID_MIU0, &pResource) != 0)
            {
                ULOGE(TAG_MIU, "UtopiaResourceObtainToInstant fail\n");
                return UTOPIA_STATUS_ERR_RESOURCE;
            }
            psMIUInstPri->fpMIU_Mask_Req_MVD_R(pMIUMask->u8Mask,pMIUMask->u8Miu);
            UtopiaResourceRelease(pResource);
            return UTOPIA_STATUS_SUCCESS;

        case MDrv_CMD_MIU_Mask_Req_MVD_W:
            MIUDBG("MIUIoctl - MDrv_CMD_MIU_Mask_Req_MVD_W\n");
            pMIUMask = (PMIU_MIU_MASK)pArgs;
            // when calling func occpuy h/w, need to call UtopiaResourceObtainToInstant to obtain res to lock other process
            if(UtopiaResourceObtain(pModule, MIU_POOL_ID_MIU0, &pResource) != 0)
            {
                ULOGE(TAG_MIU, "UtopiaResourceObtainToInstant fail\n");
                return UTOPIA_STATUS_ERR_RESOURCE;
            }
            psMIUInstPri->fpMIU_Mask_Req_MVD_W(pMIUMask->u8Mask,pMIUMask->u8Miu);
            UtopiaResourceRelease(pResource);
            return UTOPIA_STATUS_SUCCESS;

        case MDrv_CMD_MIU_Mask_Req_MVD_RW:
            MIUDBG("MIUIoctl - MDrv_CMD_MIU_Mask_Req_MVD_RW\n");
            pMIUMask = (PMIU_MIU_MASK)pArgs;
            // when calling func occpuy h/w, need to call UtopiaResourceObtainToInstant to obtain res to lock other process
            if(UtopiaResourceObtain(pModule, MIU_POOL_ID_MIU0, &pResource) != 0)
            {
                ULOGE(TAG_MIU, "UtopiaResourceObtainToInstant fail\n");
                return UTOPIA_STATUS_ERR_RESOURCE;
            }
            psMIUInstPri->fpMIU_Mask_Req_MVD_RW(pMIUMask->u8Mask,pMIUMask->u8Miu);
            UtopiaResourceRelease(pResource);
            return UTOPIA_STATUS_SUCCESS;

        case MDrv_CMD_MIU_Mask_Req_AUDIO_RW:
            MIUDBG("MIUIoctl - MDrv_CMD_MIU_Mask_Req_AUDIO_RW\n");
            pMIUMask = (PMIU_MIU_MASK)pArgs;
            // when calling func occpuy h/w, need to call UtopiaResourceObtainToInstant to obtain res to lock other process
            if(UtopiaResourceObtain(pModule, MIU_POOL_ID_MIU0, &pResource) != 0)
            {
                ULOGE(TAG_MIU, "UtopiaResourceObtainToInstant fail\n");
                return UTOPIA_STATUS_ERR_RESOURCE;
            }
            psMIUInstPri->fpMIU_Mask_Req_AUDIO_RW(pMIUMask->u8Mask,pMIUMask->u8Miu);
            UtopiaResourceRelease(pResource);
            return UTOPIA_STATUS_SUCCESS;

        case MDrv_CMD_MIU_Mask_Req:
            MIUDBG("MIUIoctl - MDrv_CMD_MIU_Mask_Req\n");
            pMIUMaskReq = (PMIU_MIUREQ_MASK)pArgs;
            // when calling func occpuy h/w, need to call UtopiaResourceObtainToInstant to obtain res to lock other process
            if(UtopiaResourceObtain(pModule, MIU_POOL_ID_MIU0, &pResource) != 0)
            {
                ULOGE(TAG_MIU, "UtopiaResourceObtainToInstant fail\n");
                return UTOPIA_STATUS_ERR_RESOURCE;
            }
            psMIUInstPri->fpMIU_Mask_Req(pMIUMaskReq->u8Miu,pMIUMaskReq->eClientID);
            UtopiaResourceRelease(pResource);
            return UTOPIA_STATUS_SUCCESS;

        case MDrv_CMD_MIU_UnMask_Req:
            MIUDBG("MIUIoctl - MDrv_CMD_MIU_UnMask_Req\n");
            pMIUMaskReq = (PMIU_MIUREQ_MASK)pArgs;
            // when calling func occpuy h/w, need to call UtopiaResourceObtainToInstant to obtain res to lock other process
            if(UtopiaResourceObtain(pModule, MIU_POOL_ID_MIU0, &pResource) != 0)
            {
                ULOGE(TAG_MIU, "UtopiaResourceObtainToInstant fail\n");
                return UTOPIA_STATUS_ERR_RESOURCE;
            }
            psMIUInstPri->fpMIU_UnMask_Req(pMIUMaskReq->u8Miu,pMIUMaskReq->eClientID);
            UtopiaResourceRelease(pResource);
            return UTOPIA_STATUS_SUCCESS;

        case MDrv_CMD_MIU_SetSsc:
            MIUDBG("MIUIoctl - MDrv_CMD_MIU_SetSsc\n");
            pMIUSetSsc = (PMIU_SET_SSC)pArgs;
            // when calling func occpuy h/w, need to call UtopiaResourceObtainToInstant to obtain res to lock other process
            if(UtopiaResourceObtain(pModule, MIU_POOL_ID_MIU0, &pResource) != 0)
            {
                ULOGE(TAG_MIU, "UtopiaResourceObtainToInstant fail\n");
                return UTOPIA_STATUS_ERR_RESOURCE;
            }
            bRet = psMIUInstPri->fpMIU_SetSsc(pMIUSetSsc->u16Fmodulation,pMIUSetSsc->u16FDeviation,pMIUSetSsc->bEnable);
            pMIUSetSsc->bRet = bRet;
            UtopiaResourceRelease(pResource);
            if (TRUE == bRet)
                return UTOPIA_STATUS_SUCCESS;
            else
                return UTOPIA_STATUS_FAIL;

        case MDrv_CMD_MIU_SetSscValue:
            MIUDBG("MIUIoctl - MDrv_CMD_MIU_SetSscValue\n");
            pMIUSetSscValue = (PMIU_SET_SSCVALUE)pArgs;
            // when calling func occpuy h/w, need to call UtopiaResourceObtainToInstant to obtain res to lock other process
            if(UtopiaResourceObtain(pModule, MIU_POOL_ID_MIU0, &pResource) != 0)
            {
                ULOGE(TAG_MIU, "UtopiaResourceObtainToInstant fail\n");
                return UTOPIA_STATUS_ERR_RESOURCE;
            }
            bRet = psMIUInstPri->fpMIU_SetSscValue(pMIUSetSscValue->u8MiuDev,pMIUSetSscValue->u16Fmodulation,pMIUSetSscValue->u16FDeviation,pMIUSetSscValue->bEnable);
            UtopiaResourceRelease(pResource);
            if (TRUE == bRet)
                return UTOPIA_STATUS_SUCCESS;
            else
                return UTOPIA_STATUS_FAIL;

        case MDrv_CMD_MIU_Protect:
            MIUDBG("MIUIoctl - MDrv_CMD_MIU_Protect\n");
            pMIUProtect = (PMIU_PROTECT)pArgs;
            // when calling func occpuy h/w, need to call UtopiaResourceObtainToInstant to obtain res to lock other process
            if(UtopiaResourceObtain(pModule, MIU_POOL_ID_MIU0, &pResource) != 0)
            {
                ULOGE(TAG_MIU, "UtopiaResourceObtainToInstant fail\n");
                return UTOPIA_STATUS_ERR_RESOURCE;
            }
            bRet = psMIUInstPri->fpMIU_Protect(pMIUProtect->u8Blockx, pMIUProtect->pu8ProtectId, pMIUProtect->phy64Start, pMIUProtect->phy64End, pMIUProtect->bSetFlag);
            pMIUProtect->bRet = bRet;
            UtopiaResourceRelease(pResource);
            if (TRUE == bRet)
                return UTOPIA_STATUS_SUCCESS;
            else
                return UTOPIA_STATUS_FAIL;

        case MDrv_CMD_MIU_ProtectEx:
            MIUDBG("MIUIoctl - MDrv_CMD_MIU_ProtectEx\n");
            pMIUProtectEx = (PMIU_PROTECTEx)pArgs;
            // when calling func occpuy h/w, need to call UtopiaResourceObtainToInstant to obtain res to lock other process
            if(UtopiaResourceObtain(pModule, MIU_POOL_ID_MIU0, &pResource) != 0)
            {
                ULOGE(TAG_MIU, "UtopiaResourceObtainToInstant fail\n");
                return UTOPIA_STATUS_ERR_RESOURCE;
            }
            bRet = psMIUInstPri->fpMIU_ProtectEx(pMIUProtectEx->u8Blockx, pMIUProtectEx->pu32ProtectId, pMIUProtectEx->phy64Start, pMIUProtectEx->phy64End, pMIUProtectEx->bSetFlag);
            UtopiaResourceRelease(pResource);
            if (TRUE == bRet)
                return UTOPIA_STATUS_SUCCESS;
            else
                return UTOPIA_STATUS_FAIL;

        case MDrv_CMD_MIU_SelMIU:
            MIUDBG("MIUIoctl - MDrv_CMD_MIU_SelMIU\n");
            pMIUSelMIU = (PMIU_SELMIU)pArgs;
            // when calling func occpuy h/w, need to call UtopiaResourceObtainToInstant to obtain res to lock other process
            if(UtopiaResourceObtain(pModule, MIU_POOL_ID_MIU0, &pResource) != 0)
            {
                ULOGE(TAG_MIU, "UtopiaResourceObtainToInstant fail\n");
                return UTOPIA_STATUS_ERR_RESOURCE;
            }
            bRet = psMIUInstPri->fpMIU_SelMIU(pMIUSelMIU->eClientID, pMIUSelMIU->eType);
            UtopiaResourceRelease(pResource);
            if (TRUE == bRet)
                return UTOPIA_STATUS_SUCCESS;
            else
                return UTOPIA_STATUS_FAIL;

        case MDrv_CMD_MIU_GetProtectInfo:
            MIUDBG("MIUIoctl - MDrv_CMD_MIU_GetProtectInfo\n");
            pMIUGetProtectInfo = (PMIU_GETPROTECTINFO)pArgs;
            // when calling func occpuy h/w, need to call UtopiaResourceObtainToInstant to obtain res to lock other process
            if(UtopiaResourceObtain(pModule, MIU_POOL_ID_MIU0, &pResource) != 0)
            {
                ULOGE(TAG_MIU, "UtopiaResourceObtainToInstant fail\n");
                return UTOPIA_STATUS_ERR_RESOURCE;
            }
            bRet = psMIUInstPri->fpMIU_GetProtectInfo(pMIUGetProtectInfo->u8MiuDev, pMIUGetProtectInfo->pInfo);
            pMIUGetProtectInfo->bRet = bRet;
            UtopiaResourceRelease(pResource);
            if (TRUE == bRet)
                return UTOPIA_STATUS_SUCCESS;
            else
                return UTOPIA_STATUS_FAIL;

        case MDrv_CMD_MIU_SetGroupPriority:
            MIUDBG("MIUIoctl - MDrv_CMD_MIU_SetGroupPriority\n");
            pMIUSetGroupPri = (PMIU_SETGROUPPRIORITY)pArgs;
            // when calling func occpuy h/w, need to call UtopiaResourceObtainToInstant to obtain res to lock other process
            if(UtopiaResourceObtain(pModule, MIU_POOL_ID_MIU0, &pResource) != 0)
            {
                ULOGE(TAG_MIU, "UtopiaResourceObtainToInstant fail\n");
                return UTOPIA_STATUS_ERR_RESOURCE;
            }
            bRet = psMIUInstPri->fpMIU_SetGroupPriority(pMIUSetGroupPri->u8MiuDev, pMIUSetGroupPri->sPriority);
            pMIUSetGroupPri->bRet = bRet;
            UtopiaResourceRelease(pResource);
            if (TRUE == bRet)
                return UTOPIA_STATUS_SUCCESS;
            else
                return UTOPIA_STATUS_FAIL;

        case MDrv_CMD_MIU_SetHighPriority:
            MIUDBG("MIUIoctl - MDrv_CMD_MIU_SetHighPriority\n");
            pMIUSetHighPri = (PMIU_SETHIGHPRIORITY)pArgs;
            // when calling func occpuy h/w, need to call UtopiaResourceObtainToInstant to obtain res to lock other process
            if(UtopiaResourceObtain(pModule, MIU_POOL_ID_MIU0, &pResource) != 0)
            {
                ULOGE(TAG_MIU, "UtopiaResourceObtainToInstant fail\n");
                return UTOPIA_STATUS_ERR_RESOURCE;
            }
            bRet = psMIUInstPri->fpMIU_SetHighPriority(pMIUSetHighPri->u8MiuDev,pMIUSetHighPri->eClientID,pMIUSetHighPri->bMask);
            pMIUSetHighPri->bRet = bRet;
            UtopiaResourceRelease(pResource);
            if (TRUE == bRet)
                return UTOPIA_STATUS_SUCCESS;
            else
                return UTOPIA_STATUS_FAIL;

        case MDrv_CMD_MIU_PrintProtectInfo:
            MIUDBG("MIUIoctl - MDrv_CMD_MIU_PrintProtectInfo\n");
            // when calling func occpuy h/w, need to call UtopiaResourceObtainToInstant to obtain res to lock other process
            if(UtopiaResourceObtain(pModule, MIU_POOL_ID_MIU0, &pResource) != 0)
            {
                ULOGE(TAG_MIU, "Ioctl UtopiaResourceObtainToInstant fail\n");
                return UTOPIA_STATUS_ERR_RESOURCE;
            }
            psMIUInstPri->fpMIU_PrintProtectInfo();
            UtopiaResourceRelease(pResource);
            return UTOPIA_STATUS_SUCCESS;

        case MDrv_CMD_MIU_Dram_Size:
            MIUDBG("MIUIoctl - MDrv_CMD_MIU_Dram_Size\n");
            pMIUDramSizePri = (PMIU_DRAM_SIZE)pArgs;
            // when calling func occpuy h/w, need to call UtopiaResourceObtainToInstant to obtain res to lock other process
            if(UtopiaResourceObtain(pModule, MIU_POOL_ID_MIU0, &pResource) != 0)
            {
                ULOGE(TAG_MIU, "Ioctl UtopiaResourceObtainToInstant fail\n");
                return UTOPIA_STATUS_ERR_RESOURCE;
            }
            bRet = psMIUInstPri->fpMIU_Dram_Size(pMIUDramSizePri->u8MiuDev, pMIUDramSizePri->DramSize);
            pMIUDramSizePri->bRet = bRet;
            UtopiaResourceRelease(pResource);
            if (TRUE == bRet)
                return UTOPIA_STATUS_SUCCESS;
            else
                return UTOPIA_STATUS_FAIL;

        case MDrv_CMD_MIU_GetDramType:
            MIUDBG("MIUIoctl - MDrv_CMD_MIU_GetDramType\n");
            pMIUGetDramTypePri = (PMIU_GETDRAMTYPE)pArgs;
            bRet = psMIUInstPri->fpMIU_GetDramType(pMIUGetDramTypePri->eMiu, pMIUGetDramTypePri->pType);
            if (TRUE == bRet)
                return UTOPIA_STATUS_SUCCESS;
            else
                return UTOPIA_STATUS_FAIL;

        case MDrv_CMD_MIU_ProtectAlign:
            MIUDBG("MIUIoctl - MDrv_CMD_MIU_ProtectAlign\n");
            pMIUProtectAlignPri = (PMIU_PROTECT_ALIGN)pArgs;
            // when calling func occpuy h/w, need to call UtopiaResourceObtainToInstant to obtain res to lock other process
            if(UtopiaResourceObtain(pModule, MIU_POOL_ID_MIU0, &pResource) != 0)
            {
                ULOGE(TAG_MIU, "UtopiaResourceObtainToInstant fail\n");
                return UTOPIA_STATUS_ERR_RESOURCE;
            }
            bRet = psMIUInstPri->fpMIU_ProtectAlign(pMIUProtectAlignPri->u32PageShift);
            pMIUProtectAlignPri->bRet = bRet;
            UtopiaResourceRelease(pResource);
            if (TRUE == bRet)
                return UTOPIA_STATUS_SUCCESS;
            else
                return UTOPIA_STATUS_FAIL;

        case MDrv_CMD_MIU_IsSupportMIU1:
            MIUDBG("MIUIoctl - MDrv_CMD_MIU_IsSupportMIU1\n");
            if(UtopiaResourceObtain(pModule, MIU_POOL_ID_MIU0, &pResource) != 0)
            {
                ULOGE(TAG_MIU, "UtopiaResourceObtainToInstant fail\n");
                return UTOPIA_STATUS_ERR_RESOURCE;
            }
            bRet = psMIUInstPri->fpMIU_IsSupportMIU1();
            UtopiaResourceRelease(pResource);
            if (TRUE == bRet)
                return UTOPIA_STATUS_SUCCESS;
            else
                return UTOPIA_STATUS_FAIL;

        case MDrv_CMD_MIU_SetBWMonitor:
            MIUDBG("MIUIoctl - MDrv_CMD_MIU_SetBWMonitor\n");
            pMIU_BWMONITOR = (PMIU_BWMONITOR)pArgs;
            bRet = psMIUInstPri->fpMIU_SetBWMonitor(pMIU_BWMONITOR->u8MiuDev, pMIU_BWMONITOR->pBWInfo);
            if (TRUE == bRet)
                return UTOPIA_STATUS_SUCCESS;
            else
                return UTOPIA_STATUS_FAIL;

        case MDrv_CMD_MIU_DelBWMonitor:
            MIUDBG("MIUIoctl - MDrv_CMD_MIU_DelBWMonitor\n");
            pMIU_BWMONITOR = (PMIU_BWMONITOR)pArgs;
            bRet = psMIUInstPri->fpMIU_DelBWMonitor(pMIU_BWMONITOR->u8MiuDev, pMIU_BWMONITOR->pBWInfo);
            if (TRUE == bRet)
                return UTOPIA_STATUS_SUCCESS;
            else
                return UTOPIA_STATUS_FAIL;

        case MDrv_CMD_MIU_QueryBWMonitor:
            MIUDBG("MIUIoctl - MDrv_CMD_MIU_QueryBWMonitor\n");
            pMIU_BWMONITOR = (PMIU_BWMONITOR)pArgs;
            bRet = psMIUInstPri->fpMIU_QueryBWMonitor(pMIU_BWMONITOR->u8MiuDev, pMIU_BWMONITOR->pBWInfo);
            if (TRUE == bRet)
                return UTOPIA_STATUS_SUCCESS;
            else
                return UTOPIA_STATUS_FAIL;

        case MDrv_CMD_MIU_GetBW:
            MIUDBG("MIUIoctl - MDrv_CMD_MIU_GetBW\n");
            pMIU_BWMONITOR = (PMIU_BWMONITOR)pArgs;
            bRet = psMIUInstPri->fpMIU_GetBWMonitor(pMIU_BWMONITOR->u8MiuDev, pMIU_BWMONITOR->pBWInfo);
            if (TRUE == bRet)
                return UTOPIA_STATUS_SUCCESS;
            else
                return UTOPIA_STATUS_FAIL;

        case MDrv_CMD_MIU_GetBWAll:
            MIUDBG("MIUIoctl - MDrv_CMD_MIU_GetBWAll\n");
            pMIU_BWMONITOR = (PMIU_BWMONITOR)pArgs;
            bRet = psMIUInstPri->fpMIU_GetBWMonitorAll(pMIU_BWMONITOR->u8MiuDev, pMIU_BWMONITOR->pBWInfo);
            if (TRUE == bRet)
                return UTOPIA_STATUS_SUCCESS;
            else
                return UTOPIA_STATUS_FAIL;

        case MDrv_CMD_MIU_ResetAllBWMonitor:
            MIUDBG("MIUIoctl - MDrv_CMD_MIU_ResetAllBWMonitor\n");
            pMIU_BWMONITOR = (PMIU_BWMONITOR)pArgs;
            bRet = psMIUInstPri->fpMIU_ResetBWMonitor(pMIU_BWMONITOR->u8MiuDev);
            if (TRUE == bRet)
                return UTOPIA_STATUS_SUCCESS;
            else
                return UTOPIA_STATUS_FAIL;

        case MDrv_CMD_MIU_StartBWMonitor:
            MIUDBG("MIUIoctl - MDrv_CMD_MIU_ResetAllBWMonitor\n");
            pMIU_BWMONITOR = (PMIU_BWMONITOR)pArgs;
            bRet = psMIUInstPri->fpMIU_StartBWMonitor(pMIU_BWMONITOR->u8MiuDev);
            if (TRUE == bRet)
                return UTOPIA_STATUS_SUCCESS;
            else
                return UTOPIA_STATUS_FAIL;

        case MDrv_CMD_MIU_ConfigHWAttr:
            MIUDBG("MIUIoctl - MDrv_CMD_MIU_ConfigHWAttr\n");
            pMIUConfigHWAttr = (PMIU_ConfigHWAttr)pArgs;
            // when calling func occpuy h/w, need to call UtopiaResourceObtainToInstant to obtain res to lock other process
            if(UtopiaResourceObtain(pModule, MIU_POOL_ID_MIU0, &pResource) != 0)
            {
                ULOGE(TAG_MIU, "UtopiaResourceObtainToInstant fail\n");
                return UTOPIA_STATUS_ERR_RESOURCE;
            }
            bRet = psMIUInstPri->fpMIU_ConfigHWAttr(pMIUConfigHWAttr->u8MiuDev, pMIUConfigHWAttr->eAttrType, pMIUConfigHWAttr->u16Value, pMIUConfigHWAttr->u16Length);
            pMIUConfigHWAttr->bRet = bRet;
            UtopiaResourceRelease(pResource);
            if (TRUE == bRet)
                return UTOPIA_STATUS_SUCCESS;
            else
                return UTOPIA_STATUS_FAIL;
        // MDrv_CMD_MIU_SetClientUtilization, MDrv_CMD_MIU_GetClientUtilization, MDrv_CMD_MIU_SetClientEfficiency, MDrv_CMD_MIU_GetClientEfficiency
        // frequently used, Not to lock UTPA resource.
        case MDrv_CMD_MIU_SetClientUtilization:
            MIUDBG("MIUIoctl - MDrv_CMD_MIU_SetClientUtilization\n");
            pMIU_BWUTILIZATION = (PMIU_BWUTILIZATION)pArgs;
            bRet = psMIUInstPri->fpMIU_SetClientUtil((MIU_ID)pMIU_BWUTILIZATION->u8MiuDev, pMIU_BWUTILIZATION->eClientID);
            if (TRUE == bRet)
                return UTOPIA_STATUS_SUCCESS;
            else
                return UTOPIA_STATUS_FAIL;

        case MDrv_CMD_MIU_GetClientUtilization:
            MIUDBG("MIUIoctl - MDrv_CMD_MIU_GetClientUtilization\n");
            pMIU_BWUTILIZATION = (PMIU_BWUTILIZATION)pArgs;
            bRet = psMIUInstPri->fpMIU_GetClientUtil((MIU_ID)pMIU_BWUTILIZATION->u8MiuDev, pMIU_BWUTILIZATION->pu32BWUtilization);
            if (TRUE == bRet)
                return UTOPIA_STATUS_SUCCESS;
            else
                return UTOPIA_STATUS_FAIL;

        case MDrv_CMD_MIU_SetClientEfficiency:
            MIUDBG("MIUIoctl - MDrv_CMD_MIU_SetClientEfficiency\n");
            pMIU_BWEFFICIENCY = (PMIU_BWEFFICIENCY)pArgs;
            bRet = psMIUInstPri->fpMIU_SetClientEffi((MIU_ID)pMIU_BWEFFICIENCY->u8MiuDev, pMIU_BWEFFICIENCY->eClientID);
            if (TRUE == bRet)
                return UTOPIA_STATUS_SUCCESS;
            else
                return UTOPIA_STATUS_FAIL;

        case MDrv_CMD_MIU_GetClientEfficiency:
            MIUDBG("MIUIoctl - MDrv_CMD_MIU_GetClientEfficiency\n");
            pMIU_BWEFFICIENCY = (PMIU_BWEFFICIENCY)pArgs;
            bRet = psMIUInstPri->fpMIU_GetClientEffi((MIU_ID)pMIU_BWEFFICIENCY->u8MiuDev, pMIU_BWEFFICIENCY->pu32BWEfficiency);
            if (TRUE == bRet)
                return UTOPIA_STATUS_SUCCESS;
            else
                return UTOPIA_STATUS_FAIL;

        case MDrv_CMD_MIU_SetBWValue:
            MIUDBG("MIUIoctl - MDrv_CMD_MIU_SetBWValue\n");
            pMIU_BWVALUE = (PMIU_SETGETBW)pArgs;
            bRet = psMIUInstPri->fpMIU_SetBWValue(pMIU_BWVALUE->u32ClientID, pMIU_BWVALUE->eBWINfoType);
            if (TRUE == bRet)
                return UTOPIA_STATUS_SUCCESS;
            else
                return UTOPIA_STATUS_FAIL;

        case MDrv_CMD_MIU_GetBWValue:
            MIUDBG("MIUIoctl - MDrv_CMD_MIU_GetBWValue\n");
            pMIU_BWVALUE = (PMIU_SETGETBW)pArgs;
            bRet = psMIUInstPri->fpMIU_GetBWValue(pMIU_BWVALUE->u32ClientID, pMIU_BWVALUE->eBWINfoType, pMIU_BWVALUE->peUMAMode, pMIU_BWVALUE->pBWValue);
            if (TRUE == bRet)
                return UTOPIA_STATUS_SUCCESS;
            else
                return UTOPIA_STATUS_FAIL;

        case MDrv_CMD_MIU_GetMIUInfo:
            MIUDBG("MIUIoctl - MDrv_CMD_MIU_GetMIUInfo\n");
            pMIU_DRAMINFO = (PMIU_GETINFO)pArgs;
            // when calling func occpuy h/w, need to call UtopiaResourceObtainToInstant to obtain res to lock other process
            if(UtopiaResourceObtain(pModule, MIU_POOL_ID_MIU0, &pResource) != 0)
            {
                ULOGE(TAG_MIU, "UtopiaResourceObtainToInstant fail\n");
                return UTOPIA_STATUS_ERR_RESOURCE;
            }
            bRet = psMIUInstPri->fpMIU_GetMIUInfo(pMIU_DRAMINFO->eMiu ,pMIU_DRAMINFO->pstMIUInfo);
            UtopiaResourceRelease(pResource);
            if (TRUE == bRet)
                return UTOPIA_STATUS_SUCCESS;
            else
                return UTOPIA_STATUS_FAIL;

        case MDrv_CMD_MIU_SetCPUHPriority:
            MIUDBG("MIUIoctl - MDrv_CMD_MIU_SetCPUHPriority\n");
            pMIU_CPUHPri = (PMIU_SETCPUHPRI)pArgs;
            // when calling func occpuy h/w, need to call UtopiaResourceObtainToInstant to obtain res to lock other process
            if(UtopiaResourceObtain(pModule, MIU_POOL_ID_MIU0, &pResource) != 0)
            {
                ULOGE(TAG_MIU, "UtopiaResourceObtainToInstant fail\n");
                return UTOPIA_STATUS_ERR_RESOURCE;
            }
            bRet = psMIUInstPri->fpMIU_SetCPUHpri(pMIU_CPUHPri->eScope, pMIU_CPUHPri->u64Start, pMIU_CPUHPri->u64End, pMIU_CPUHPri->bEnable);
            UtopiaResourceRelease(pResource);
            if (TRUE == bRet)
                return UTOPIA_STATUS_SUCCESS;
            else
                return UTOPIA_STATUS_FAIL;

        default:
            break;
    };
#endif
    return UTOPIA_STATUS_SUCCESS; // FIXME: error code
}

MS_U32 MIUClose(void* pInstance)
{
    UtopiaInstanceDelete(pInstance);

    return UTOPIA_STATUS_SUCCESS;
}
