#if !defined(MSOS_TYPE_LINUX_KERNEL)
#include "string.h"
#include <stdio.h>
#else
#include <linux/string.h>
#include <linux/slab.h>
#endif
#include "MsTypes.h"
#include "utopia_dapi.h"
#include "drvSEM.h"
#include "drvSEM_priv.h"
#include "MsOS.h"
#include "utopia.h"
#include "ULog.h"
#define TAG_SEM "SEM"

enum
{
    SEM_POOL_ID_SEM0 = 0
} eSEM_PoolID;

//--------------------------------------------------------------------------------------------------
// Utopia2.0 will call this function to register module
//--------------------------------------------------------------------------------------------------
void SEMRegisterToUtopia(void)
{
    // 1. create a module(module_name, SHM_size), and register to utopia2.0
    void* pUtopiaModule = NULL;
    UtopiaModuleCreate(MODULE_SEM, 8, &pUtopiaModule);
    UtopiaModuleRegister(pUtopiaModule);

    // register func for module, after register here, then ap call UtopiaOpen/UtopiaIoctl/UtopiaClose can call to these registered standard func
    UtopiaModuleSetupFunctionPtr(pUtopiaModule, (FUtopiaOpen)SEMOpen, (FUtopiaClose)SEMClose, (FUtopiaIOctl)SEMIoctl);

    // 2. Resource register
    void* psResource = NULL;

    // start func to add resources of a certain Resource_Pool
    UtopiaModuleAddResourceStart(pUtopiaModule, SEM_POOL_ID_SEM0);

    // create a resouce and regiter it to a certain Resource_Pool, resource can alloc private for internal use
    UtopiaResourceCreate("sem0", sizeof(SEM_RESOURCE_PRIVATE), &psResource);
    UtopiaResourceRegister(pUtopiaModule, psResource, SEM_POOL_ID_SEM0);

    //UtopiaResourceCreate("sem1", sizeof(SEM_RESOURCE_PRIVATE), &psResource);
    //UtopiaResourceRegister(pUtopiaModule, psResource, SEM_POOL_ID_SEM0);

    // end func to add resources of a certain Resource_Pool(this will set the ResourceSemaphore of this ResourcePool)
    UtopiaModuleAddResourceEnd(pUtopiaModule, SEM_POOL_ID_SEM0);
}

//--------------------------------------------------------------------------------------------------
// Utopia2.0 will call this function to get a instance to use SEM
// @ \b in: 32ModuleVersion => this is for checking if API version is the same
//--------------------------------------------------------------------------------------------------
MS_U32 SEMOpen(void** ppInstance, MS_U32 u32ModuleVersion, void* pAttribute)
{
    //ULOGE(TAG_SEM, "\033[35mFunction = %s, Line = %d, [SEM INFO] OPEN INSTANCE...\033[m\n", __PRETTY_FUNCTION__, __LINE__);
    SEM_INSTANT_PRIVATE *pSEMPri = NULL;

    // instance is allocated here, also can allocate private for internal use
    UtopiaInstanceCreate(sizeof(SEM_INSTANT_PRIVATE), ppInstance);
    // set the pSEMPri point to the private of UTOPIA_INSTANCE
    UtopiaInstanceGetPrivate(*ppInstance, (void**)&pSEMPri);

    // setup func in private and assign the calling func in func ptr in instance private
    pSEMPri->fpSEMGetResource   = (IOCTL_SEM_GETRESOURCE)MDrv_SEM_Get_Resource_U2K;
    pSEMPri->fpSEMFreeResource  = (IOCTL_SEM_FREERESOURCE)MDrv_SEM_Free_Resource_U2K;
    pSEMPri->fpSEMResetResource = (IOCTL_SEM_RESETRESOURCE)MDrv_SEM_Reset_Resource_U2K;
    pSEMPri->fpSEMGetResourceID = (IOCTL_SEM_GETRESOURCEID)MDrv_SEM_Get_ResourceID_U2K;
    pSEMPri->fpSEMInit          = (IOCTL_SEM_INIT)MDrv_SEM_Init_U2K;
    pSEMPri->fpSEMSetDbgLevel   = (IOCTL_SEM_SETDBGLEVEL)MDrv_SEM_SetDbgLevel_U2K;
    pSEMPri->fpSEMLock          = (IOCTL_SEM_LOCK)MDrv_SEM_Lock_U2K;
    pSEMPri->fpSEMUnlock        = (IOCTL_SEM_UNLOCK)MDrv_SEM_Unlock_U2K;
    pSEMPri->fpSEMDelete        = (IOCTL_SEM_DELETE)MDrv_SEM_Delete_U2K;

    return UTOPIA_STATUS_SUCCESS;
}

MS_U32 SEMIoctl(void* pInstance, MS_U32 u32Cmd, void* pArgs)
{
    void* pModule = NULL;
    UtopiaInstanceGetModule(pInstance, &pModule);

    SEM_INSTANT_PRIVATE* psSEMInstPri = NULL;
    UtopiaInstanceGetPrivate(pInstance, (void**)&psSEMInstPri);

    PSEM_GETRESOURCE_PARAM pGetResource = NULL;
    PSEM_FREERESOURCE_PARAM pFreeResource = NULL;
    PSEM_RESETRESOURCE_PARAM pResetResource = NULL;
    PSEM_GETRESOURCEID_PARAM pGetResourceID = NULL;
    PSEM_SETDBGLEVEL_PARAM pSetDbgLevel = NULL;
    PSEM_LOCK_PARAM pLock = NULL;
    PSEM_UNLOCK_PARAM pUnlock = NULL;
    PSEM_DELETE_PARAM pDelete = NULL;

    switch (u32Cmd) {
        case MDrv_CMD_SEM_Get_Resource:
            //ULOGD(TAG_SEM, "SEMIoctl - MDrv_CMD_SEM_Get_Resource\n");
            /* Check Parameter */
            if (NULL == (pGetResource = (PSEM_GETRESOURCE_PARAM)pArgs))
                return UTOPIA_STATUS_PARAMETER_ERROR;
            /* Call function */
            if (TRUE == psSEMInstPri->fpSEMGetResource(pGetResource->u8SemID, pGetResource->u16ResId))
                return UTOPIA_STATUS_SUCCESS;
            else
                return UTOPIA_STATUS_FAIL;
        case MDrv_CMD_SEM_Free_Resource:
            //ULOGD(TAG_SEM, "SEMIoctl - MDrv_CMD_SEM_Free_Resource\n");
            /* Check Parameter */
            if (NULL == (pFreeResource = (PSEM_FREERESOURCE_PARAM)pArgs))
                return UTOPIA_STATUS_PARAMETER_ERROR;
            /* Call function */
            if (TRUE == psSEMInstPri->fpSEMFreeResource(pFreeResource->u8SemID, pFreeResource->u16ResId))
                return UTOPIA_STATUS_SUCCESS;
            else
                return UTOPIA_STATUS_FAIL;
        case MDrv_CMD_SEM_Reset_Resource:
            //ULOGD(TAG_SEM, "SEMIoctl - MDrv_CMD_SEM_Reset_Resource\n");
            /* Check Parameter */
            if (NULL == (pResetResource = (PSEM_RESETRESOURCE_PARAM)pArgs))
                return UTOPIA_STATUS_PARAMETER_ERROR;
            /* Call function */
            if (TRUE == psSEMInstPri->fpSEMResetResource(pResetResource->u8SemID))
                return UTOPIA_STATUS_SUCCESS;
            else
                return UTOPIA_STATUS_FAIL;
        case MDrv_CMD_SEM_Get_ResourceID:
            //ULOGD(TAG_SEM, "SEMIoctl - MDrv_CMD_SEM_Get_ResourceID\n");
            /* Check Parameter */
            if (NULL == (pGetResourceID = (PSEM_GETRESOURCEID_PARAM)pArgs))
                return UTOPIA_STATUS_PARAMETER_ERROR;
            /* Call function */
            if (TRUE == psSEMInstPri->fpSEMGetResourceID(pGetResourceID->u8SemID, pGetResourceID->pu16ResId))
                return UTOPIA_STATUS_SUCCESS;
            else
                return UTOPIA_STATUS_FAIL;
        case MDrv_CMD_SEM_Init:
            //ULOGD(TAG_SEM, "SEMIoctl - MDrv_CMD_SEM_Init\n");
            /* Call function */
            if (TRUE == psSEMInstPri->fpSEMInit())
                return UTOPIA_STATUS_SUCCESS;
            else
                return UTOPIA_STATUS_FAIL;
        case MDrv_CMD_SEM_SetDbgLevel:
            //ULOGD(TAG_SEM, "SEMIoctl - MDrv_CMD_SEM_SetDbgLevel\n");
            /* Check Parameter */
            if (NULL == (pSetDbgLevel = (PSEM_SETDBGLEVEL_PARAM)pArgs))
                return UTOPIA_STATUS_PARAMETER_ERROR;
            /* Call function */
            if (TRUE == psSEMInstPri->fpSEMSetDbgLevel(pSetDbgLevel->eLevel))
                return UTOPIA_STATUS_SUCCESS;
            else
                return UTOPIA_STATUS_FAIL;
        case MDrv_CMD_SEM_Lock:
            //ULOGD(TAG_SEM, "SEMIoctl - MDrv_CMD_SEM_Lock\n");
            /* Check Parameter */
            if (NULL == (pLock = (PSEM_LOCK_PARAM)pArgs))
                return UTOPIA_STATUS_PARAMETER_ERROR;
            /* Call function */
            if (TRUE == psSEMInstPri->fpSEMLock(pLock->eDeviceSemID, pLock->u32WaitMs))
                return UTOPIA_STATUS_SUCCESS;
            else
                return UTOPIA_STATUS_FAIL;
        case MDrv_CMD_SEM_Unlock:
            //ULOGD(TAG_SEM, "SEMIoctl - MDrv_CMD_SEM_Unlock\n");
            /* Check Parameter */
            if (NULL == (pUnlock = (PSEM_UNLOCK_PARAM)pArgs))
                return UTOPIA_STATUS_PARAMETER_ERROR;
            /* Call function */
            if (TRUE == psSEMInstPri->fpSEMUnlock(pUnlock->eDeviceSemID))
                return UTOPIA_STATUS_SUCCESS;
            else
                return UTOPIA_STATUS_FAIL;
        case MDrv_CMD_SEM_Delete:
            //ULOGD(TAG_SEM, "SEMIoctl - MDrv_CMD_SEM_Delete\n");
            /* Check Parameter */
            if (NULL == (pDelete = (PSEM_DELETE_PARAM)pArgs))
                return UTOPIA_STATUS_PARAMETER_ERROR;
            /* Call function */
            if (TRUE == psSEMInstPri->fpSEMDelete(pDelete->eDeviceSemID))
                return UTOPIA_STATUS_SUCCESS;
            else
                return UTOPIA_STATUS_FAIL;
        default:
            break;
    };

    return UTOPIA_STATUS_FAIL;
}

MS_U32 SEMClose(void* pInstance)
{
    UtopiaInstanceDelete(pInstance);

    return UTOPIA_STATUS_SUCCESS;
}
