#ifndef _MTK_PNL_AUTODOWNLOAD_H
#define _MTK_PNL_AUTODOWNLOAD_H

#define UFO_XC_AUTO_DOWNLOAD

#define SUPPORT_XC_AISR                     (FALSE)
#define SUPPORT_HFR_ADL                     (FALSE)

typedef enum
{
  E_CHIP_MIU_0 = 0,
  E_CHIP_MIU_1,
  E_CHIP_MIU_2,
  E_CHIP_MIU_3,
  E_CHIP_MIU_NUM,
} CHIP_MIU_ID;

#define MSTAR_MIU0_BUS_BASE                      0x20000000UL
#define MSTAR_MIU1_BUS_BASE                      0xA0000000UL

#define ADD_MIU_OFFSET(MiuSel, PhyAddr)     if (MiuSel == E_CHIP_MIU_0) \
                                                {PhyAddr += MSTAR_MIU0_BUS_BASE;} \
                                             else if (MiuSel == E_CHIP_MIU_1) \
                                                 {PhyAddr += MSTAR_MIU1_BUS_BASE;} \

#define AUTODOWNLOAD_REQ_LEN               (0x200)

typedef enum
{
    E_PNL_XC_AUTODOWNLOAD_CLIENT_PANEL_GAMMA,
    E_PNL_XC_AUTODOWNLOAD_CLIENT_OD,
    E_PNL_XC_AUTODOWNLOAD_CLIENT_PCID,
    E_PNL_XC_AUTODOWNLOAD_CLIENT_VAC,
    E_PNL_XC_AUTODOWNLOAD_CLIENT_MAX,
} EN_PNL_XC_AUTODOWNLOAD_CLIENT;

typedef enum
{
    E_PNL_XC_AUTODOWNLOAD_TRIGGER_MODE,
    E_PNL_XC_AUTODOWNLOAD_ENABLE_MODE,
} EN_PNL_XC_AUTODOWNLOAD_MODE;

typedef enum
{
    E_PNL_XC_AUTODOWNLOAD_TRIGGER_MODE_HW,
    E_PNL_XC_AUTODOWNLOAD_TRIGGER_MODE_SW,
    E_PNL_XC_AUTODOWNLOAD_TRIGGER_MODE_MAX,
} EN_PNL_XC_AUTODOWNLOAD_TRIGGER_MODE;

typedef enum
{
    E_PNL_XC_AUTO_DOWNLOAD_NONE,
    E_PNL_XC_AUTO_DOWNLOAD_CONFIGURED,
    E_PNL_XC_AUTO_DOWNLOAD_WRITED,
    E_PNL_XC_AUTO_DOWNLOAD_FIRED,
} EN_PNL_XC_AUTO_DOWNLOAD_STATUS;


typedef struct
{
    u64 phyBaseAddr;                 /// baseaddr
    uint32_t u32Size;                     /// size
    uint32_t u32MiuNo;                    /// miu no
    bool bEnable;                    /// enable/disable the client
    EN_PNL_XC_AUTODOWNLOAD_MODE enMode;/// work mode
    uint32_t u32StartAddr;                /// sram start addr
    uint32_t u32Depth;                    /// data length
    //uint8_t *pu8BaseAddr;
    u64 busBaseAddr;
    bool bUnmapAddr;
    EN_PNL_XC_AUTO_DOWNLOAD_STATUS enStatus;
    EN_PNL_XC_AUTODOWNLOAD_TRIGGER_MODE enTriggerMode;
    uint8_t u8SectionIdx;
    uint8_t u8MaxSection;
    bool bFakeSupport;
} ST_PNL_XC_AUTODOWNLOAD_CLIENT_INFO;


#ifdef UFO_XC_AUTO_DOWNLOAD
bool KHal_XC_GetMiuOffset(uint32_t u32MiuNo, u64 *pu64Offset);
bool KHal_XC_InitAutoDownload(struct udevice *dev);
bool KHal_XC_InitAutoDownloadSpinlock(void);
bool KHal_XC_ConfigAutoDownload(EN_PNL_XC_AUTODOWNLOAD_CLIENT enClient, bool bEnable,
                                EN_PNL_XC_AUTODOWNLOAD_MODE enMode, u64 phyBaseAddr, uint32_t u32Size, uint32_t u32MiuNo);
bool KHal_XC_WriteAutoDownload(EN_PNL_XC_AUTODOWNLOAD_CLIENT enClient, uint8_t* pu8Data, uint32_t u32Size, void* pParam);
bool KHal_XC_FireAutoDownload(EN_PNL_XC_AUTODOWNLOAD_CLIENT enClient);
#else
#define KHal_XC_GetMiuOffset(args...) (FALSE)
#define KHal_XC_InitAutoDownload(args...) (FALSE)
#define KHal_XC_InitAutoDownloadSpinlock(args...) (FALSE)
#define KHal_XC_ConfigAutoDownload(args...) (FALSE)
#define KHal_XC_WriteAutoDownload(args...) (FALSE)
#define KHal_XC_FireAutoDownload(args...) (FALSE)
#endif

bool mtk_pnl_autodownload_init(struct udevice *dev);

#endif

