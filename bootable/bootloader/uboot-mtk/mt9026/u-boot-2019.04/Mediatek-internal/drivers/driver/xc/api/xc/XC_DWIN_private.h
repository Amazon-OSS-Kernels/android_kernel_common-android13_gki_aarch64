#include "mvideo_context.h"

#include "MsTypes.h"

typedef struct _DIP_SHARE_RESOURCE_PRIVATE
{
    XC_DIP_InternalStatus  g_DIPSrcInfo[MAX_DIP_WINDOW];
    MS_BOOL bLock[MAX_DIP_WINDOW];
    MS_BOOL bDIPRLock;
    SCALER_DIP_WIN eDIPRControlByWindow;
    MS_BOOL bIsLoadSRAM[MAX_DIP_WINDOW];
    MS_BOOL bDIP3DDIInit[MAX_DIP_WINDOW];
}DIP_SHARE_RESOURCE_PRIVATE;

typedef struct _DIP_RESOURCE_PRIVATE
{
}DIP_RESOURCE_PRIVATE;

typedef struct _DIP_INSTANCE_PRIVATE
{
    MS_BOOL bInit;
    MS_U8 u8DbgLevel;
    SCALER_DIP_WIN eWindow;
    void* pDIPResource[MAX_DIP_WINDOW];
    void* pResource;
    void* pShareResource;
}DIP_INSTANCE_PRIVATE;

typedef enum
{
    E_DIP_BUFFER_NONE = 0x0,
    E_DIP_DI_DNR_BUFFER = 0x1,
}EN_DIP_BUF_TYPE;

typedef struct DLL_PACKED
{
    EN_DIP_BUF_TYPE enBufType;
    MS_PHY phyBufAddr;
    MS_U32 u32BufSize;
}ST_DIP_Buffer;

typedef enum
{
    EN_DIP_MODE_NONE = 0x0,
    EN_DIP_MODE_DI = 0x1,
    EN_DIP_MODE_PDNR = 0x2,
    EN_DIP_MODE_DIPR_BLENDING = 0x3,
}EN_DIP_MODE;

typedef enum
{
    E_XC_DIPR0 = 0x0,
}EN_XC_DIPR_ENGINE;

typedef enum
{
    E_XC_BLENDING_COMPOSER = 0x0,
    E_XC_BLENDING_HFR = 0x1,
}EN_XC_DIPR_BLENDING_DST;

typedef struct DLL_PACKED
{
    EN_XC_DIPR_ENGINE enDIPRSel;
    EN_XC_DIPR_BLENDING_DST enDIPRDst;
    MS_BOOL bEnable;
}ST_DIP_DIPR_BLENDING_MODE_SETTINGS;

typedef struct DLL_PACKED
{
    EN_DIP_BUF_TYPE enBufType;
    MS_PHY phyBufAddr;
    MS_U32 u32BufSize;
    SCALER_DIP_WIN eWindow;
}ST_DIP_SET_BUFFER_INFO;

typedef struct DLL_PACKED
{
    SCALER_DIP_WIN eWindow;
    EN_DIP_MODE enMode;
    void *pModeSettings;
}ST_DIP_SET_PROCESS_MODE_INFO;

typedef struct DLL_PACKED
{
   INPUT_SOURCE_TYPE_t enXCInputSrcType;
   SCALER_DIP_SOURCE_TYPE enDipCapSource;
   MS_WINDOW_TYPE stXCCapWin;
   MS_BOOL bInterlace;
   MS_BOOL bPathEnabled;
}ST_DIP_XCPathInfo;

typedef struct DLL_PACKED
{
    SCALER_DIP_WIN eWindow;
    ST_DIP_XCPathInfo stXCInfo;
}ST_DIP_SET_XCPATH_INFO;

typedef struct DLL_PACKED
{
    SCALER_DIP_WIN eWindow;
    SCALER_DIP_SOURCE_TYPE enSource;
    INPUT_SOURCE_TYPE_t enInputSourceType;
    MS_U16 u16VsyncPos;
    MS_U16 u16Width;
    MS_U16 u16Height;
    MS_U32 u32HDMIClkRate;
    MS_BOOL bIsHDMI2P1;
}ST_DIP_HDMI_Info;

void DIPRegisterToUtopia(FUtopiaOpen ModuleType);
MS_U32 DIPOpen(void** ppInstance, const void* const pAttribute);
MS_U32 DIPClose(void* pInstance);
MS_U32 DIPIoctl(void* pInstance, MS_U32 u32Cmd, void* pArgs);
#ifdef CONFIG_UTOPIA_PROC_DBG_SUPPORT
MS_U32 DIPMdbIoctl(MS_U32 u32Cmd, const void* const pArgs);
#endif
