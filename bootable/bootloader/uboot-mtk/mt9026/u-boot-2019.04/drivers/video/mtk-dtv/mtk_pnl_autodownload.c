#include <common.h>
#include <asm/io.h>
#include <dm.h>
#include <utility.h>
#include <time.h>
#include <linux/delay.h>
#include <system_impl.h>
#include <debug_impl.h>
#include <dts_parser.h>
#include "mtk_pnl_utility.h"
#include "mtk_pnl_autodownload.h"
#include "coda/hwreg_common.h"
#include "coda/TCON_ADL_BKA367_V004.h"
#include "coda/TCON_ADL_CRC_BKA369.h"
#include "coda/TCON_ADL_CRC_BKA369_V004.h"
#include "coda/TCON_ADL_BKA367_V006.h"
#include "coda/TCON_ADL_CRC_BKA369_V006.h"
#include "mtk_tcon_common.h"
#include "mtk_tv_pnl.h"
#ifdef TZ_USAGE
#include "coda/SECU_TCON_ADL_BKA33B_V004.h"
#include "coda/SECU_TCON_ADL_CRC_BKA33C_V004.h"
#define BKA33B_OFST (TCON_ADL_BKA367_V004 - SECU_TCON_ADL_BKA33B_V004)
#define BKA33C_OFST (TCON_ADL_CRC_BKA369_V004 - SECU_TCON_ADL_CRC_BKA33C_V004)
#else
#define BKA33B_OFST 0
#define BKA33C_OFST 0
#endif

#ifdef UFO_XC_AUTO_DOWNLOAD

#ifndef TRUE
#define TRUE                        1
#endif
#ifndef FALSE
#define FALSE                       0
#endif

#define CHECK_CLIENT_RANGE_VALID(enClient) (((enClient) >= E_PNL_XC_AUTODOWNLOAD_CLIENT_PANEL_GAMMA) && ((enClient) < E_PNL_XC_AUTODOWNLOAD_CLIENT_MAX))


#define AUTODOWNLOAD_CLIENT_PANELGAMMA_MEM_SIZE (0x1800)     // 192_entry*256bits_per_cmd/8 = 0x3000(2lut)
#define AUTODOWNLOAD_CLIENT_PCID_MEM_SIZE  (0x43E0)    // 543 * 32_byte_per_cmd
#define AUTODOWNLOAD_CLIENT_VAC_MEM_SIZE  (0xC00)    // 96 * 32_byte_per_cmd
#define AUTODOWNLOAD_CLIENT_OVERDRIVER_MEM_SIZE   (0x2420)  // 289enty * 32byte per cmd = 0x2500

#define XC_AUTODOWNLOAD_PNLGAMMA_WIDTH    (256)
#define XC_AUTODOWNLOAD_PNLGAMMA_DEPTH    (1024)

#define XC_AUTODOWNLOAD_OVERDRIVER_DEPTH  (289)
//-------------------------------------------------------------------------------------------------
//  Global Variables
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Local Variables
//-------------------------------------------------------------------------------------------------
static ST_PNL_XC_AUTODOWNLOAD_CLIENT_INFO _stClientInfo[E_PNL_XC_AUTODOWNLOAD_CLIENT_MAX];
spinlock_t _astAdlSpinlock[E_PNL_XC_AUTODOWNLOAD_CLIENT_MAX];
static bool _bAdlSetting[E_PNL_XC_AUTODOWNLOAD_CLIENT_MAX] = {0};
//-------------------------------------------------------------------------------------------------
//  Local Defines
//-------------------------------------------------------------------------------------------------
static uint32_t _au32AdlMemBaseUnit;
static uint32_t _au32AdlBusWidth;

#define CHECK_ADL_WIDTH_VALID()     ((_au32AdlBusWidth != 0) && (_au32AdlMemBaseUnit != 0))

#define _ADL_SPIN_LOCK(enClient)    _KHal_XC_AdlSpinLock(enClient)  //spin_lock(&_astAdlSpinlock[enClient])
#define _ADL_SPIN_UNLOCK(enClient)  _KHal_XC_AdlSpinUnLock(enClient)  //spin_unlock(&_astAdlSpinlock[enClient])
// MIU Word (Bytes)
#define BYTE_PER_WORD	(32)	//MIU 128:16Byte/W, MIU 256:32Byte/W
//ADL
#define AUTO_DOWNLOAD_MEM_BASE_UNIT_256         BYTE_PER_WORD
#define AUTO_DOWNLOAD_MEM_BASE_UNIT_128         (16)
#define AUTO_DOWNLOAD_BUS_WIDTH_256             (32)
#define AUTO_DOWNLOAD_BUS_WIDTH_128             (16)

#define ALIGN_UPTO_MIU_WORD(x)	((((x) + _au32AdlMemBaseUnit - 1) / _au32AdlMemBaseUnit) * _au32AdlMemBaseUnit)

#define DS_BUFFER_NUM_EX                    (14)

#define AUTODOWNLOAD_CLIENT_HDR_MEM_SIZE     (0x2820*DS_BUFFER_NUM_EX)   //321 * 32 * 14
#define AUTODOWNLOAD_CLIENT_3DLUT_MEM_SIZE   (0x5B20*DS_BUFFER_NUM_EX)   //729 * 32 * 14
#define AUTODOWNLOAD_CLIENT_ADL2RIU_MEM_SIZE (0)     //no ADL2RIU Client
#define AUTODOWNLOAD_CLIENT_DLC_256_MEM_SIZE  (0x400)    // 256_entry * 32_byte_per_cmd = 0x400
#define AUTODOWNLOAD_CLIENT_DC0_FILM_GRAIN_MEM_SIZE  (0xA0C0*DS_BUFFER_NUM_EX)     //1286 * 32 * 14 = 0x8CC40
#define AUTODOWNLOAD_CLIENT_HFR_HDR_MEM_SIZE      (0)   // no HFR_HDR client
#define AUTODOWNLOAD_CLIENT_HFR_3DLUT_MEM_SIZE    (0)   // no HFR_3DLut client
#define AUTODOWNLOAD_CLIENT_HFR_VIP_DLC_256_MEM_SIZE  (0)    //no HDR_VIP client
#define AUTODOWNLOAD_CLIENT_HDR_SUB_SIZE              (0)              // (384+66) * * 32_byte_per_cmd = 0x3840
#define AUTODOWNLOAD_CLIENT_DEMURA_MEM_SIZE  (0)     //Demora client not used in xc_86, it is used in mboot, so there is no need memory
#define DLC_256_ENTRY   (512)
#define AUTODOWNLOAD_CLIENT_PANELGAMMA_MEM_SIZE (0x1800)     // 192_entry*256bits_per_cmd/8 = 0x3000(2lut)
#define AUTODOWNLOAD_CLIENT_XVYCC_MEM_SIZE (0x3840*DS_BUFFER_NUM_EX)     //450 * 32 * 14
#define AUTODOWNLOAD_CLIENT_PCID_MEM_SIZE  (0x43E0)    // 543 * 32_byte_per_cmd
#define AUTODOWNLOAD_CLIENT_DV_HW5_LUT_MEM_SIZE (0x4BA0*DS_BUFFER_NUM_EX)     //605 * 32 * 14
#define AUTODOWNLOAD_CLIENT_SPF_MEM_SIZE  (0xC00)    // 96 * 32_byte_per_cmd
#define AUTODOWNLOAD_CLIENT_VAC_MEM_SIZE  (0xC00)    // 96 * 32_byte_per_cmd

#define AUTODOWNLOAD_CLIENT_OP2LUT_MEM_SIZE  (0x5B20)     // 729_entry * 32_byte_per_cmd = 0x5B20
#define AUTODOWNLOAD_CLIENT_HVSP_MEM_SIZE  (0x2040*DS_BUFFER_NUM_EX) // (129*2)_entry *32 * 14
#define AUTODOWNLOAD_CLIENT_ICC_IHC_MEM_SIZE (0)      // 289_entry*32_byte_per_cmd = 0x2420
#define AUTODOWNLOAD_CLIENT_PQ_GAMMA_MEM_SIZE  (0x600)    // 48_entry * 32_byte_per_cmd = 0x600
#define AUTODOWNLOAD_CLIENT_HSY_MEM_SIZE        (0x1440)      // 162  * 128bit_per_cmd/8

#define AUTODOWNLOAD_CLIENT_SC2VIP_MEM_SIZE  (0)    // 16_entry * 32_byte_per_cmd = 0x200
#define AUTODOWNLOAD_CLIENT_OVERDRIVER_MEM_SIZE   (0x2420)  // 289enty * 32byte per cmd = 0x2500
#define AUTODOWNLOAD_CLIENT_UCD_STAT_TOP_MEM_SIZE  (0)     // 2_entry * 256_byte_per_cmd = 0x200
#define AUTODOWNLOAD_CLIENT_GUCD_B2P_MEM_SIZE  (0x4B00)     // 600 entry * 32
#define AUTODOWNLOAD_CLIENT_GUCD_18_LUT_MEM_SIZE  (0x240)     // 32 *18
#define AUTODOWNLOAD_CLIENT_GUCD_256_LUT_MEM_SIZE  (0)     // 2_entry * 256_byte_per_cmd = 0x200

#define AUTODOWNLOAD_CLIENT_AISR_PQ_MEM_SIZE  (0) // 5(#transition) x 16(#parameter) x 20KB(609x256-bit) = 0x17CA00
#define AUTODOWNLOAD_CLIENT_AISR_HVSP_MEM_SIZE  (0) // 128_entry  * 256bit_per_cmd/8*32bank = 0x1000 *32

#define AUTODOWNLOAD_CLIENT_DIP_XVYCC_MEM_SIZE  (0x3840)    // 450*32byte

#define DEFAULT_INPUT_VSYNC			(240)	//IP Vsync x10, default 24HZ
#define DEFAULT_OUTPUT_VSYNC       (600) //OP Vsync x10, default 60HZ

#define REG_ADL_CLIENT_NOT_SUPPORT (0)

static uint32_t _au32AdlRegClkGate[E_PNL_XC_AUTODOWNLOAD_CLIENT_MAX] = {
    REG_0004_TCON_ADL_BKA367_V004 - BKA33B_OFST, //tcon 1 panel gamma
    REG_01C4_TCON_ADL_BKA367_V004 - BKA33B_OFST, //tcon g od
    REG_01A8_TCON_ADL_CRC_BKA369_V004 - BKA33C_OFST,//tcon i pcid
    REG_0118_TCON_ADL_CRC_BKA369_V004 - BKA33C_OFST, //tcon k vac
};

static uint32_t _au32AdlRegClkGateMask[E_PNL_XC_AUTODOWNLOAD_CLIENT_MAX] = {
    REG_0004_TCON_ADL_BKA367_V004_REG_CLIENT1_CLK_GATE_EN_0004, //tcon 1 panel gamma
    REG_01C4_TCON_ADL_BKA367_V004_REG_CLIENTG_CLK_GATE_EN_01C4, //tcon g od
    REG_01A8_TCON_ADL_CRC_BKA369_V004_REG_CLIENTI_CLK_GATE_EN_01A8,//tcon i pcid
    REG_0118_TCON_ADL_CRC_BKA369_V004_REG_CLIENTK_CLK_GATE_EN_0118, //tcon k vac
};

static uint32_t _au32AdlRegBassAddr_L[E_PNL_XC_AUTODOWNLOAD_CLIENT_MAX] = {
    REG_0008_TCON_ADL_BKA367_V004 - BKA33B_OFST, //tcon 1 panel gamma
    REG_01C8_TCON_ADL_BKA367_V004 - BKA33B_OFST, //tcon g od
    REG_01AC_TCON_ADL_CRC_BKA369_V004 - BKA33C_OFST,//tcon i pcid
    REG_011C_TCON_ADL_CRC_BKA369_V004 - BKA33C_OFST, //tcon k vac
};

static uint32_t _au32AdlRegBassAddr_L_Mask[E_PNL_XC_AUTODOWNLOAD_CLIENT_MAX] = {
    REG_0008_TCON_ADL_BKA367_V004_REG_CLIENT1_BASEADR_0_0008, //tcon 1 panel gamma
    REG_01C8_TCON_ADL_BKA367_V004_REG_CLIENTG_BASEADR_0_01C8, //tcon g od
    REG_01AC_TCON_ADL_CRC_BKA369_V004_REG_CLIENTI_BASEADR_0_01AC,//tcon i pcid
    REG_011C_TCON_ADL_CRC_BKA369_V004_REG_CLIENTK_BASEADR_0_011C, //tcon k vac
};

static uint32_t _au32AdlRegBassAddr_H[E_PNL_XC_AUTODOWNLOAD_CLIENT_MAX] = {

    REG_000C_TCON_ADL_BKA367_V004 - BKA33B_OFST, //tcon 1 panel gamma
    REG_01CC_TCON_ADL_BKA367_V004 - BKA33B_OFST, //tcon g od
    REG_01B0_TCON_ADL_CRC_BKA369_V004 - BKA33C_OFST,//tcon i pcid
    REG_0120_TCON_ADL_CRC_BKA369_V004 - BKA33C_OFST, //tcon k vac
};

static uint32_t _au32AdlRegBassAddr_H_Mask[E_PNL_XC_AUTODOWNLOAD_CLIENT_MAX] = {
    REG_000C_TCON_ADL_BKA367_V004_REG_CLIENT1_BASEADR_1_000C, //tcon 1 panel gamma
    REG_01CC_TCON_ADL_BKA367_V004_REG_CLIENTG_BASEADR_1_01CC, //tcon g od
    REG_01B0_TCON_ADL_CRC_BKA369_V004_REG_CLIENTI_BASEADR_1_01B0,//tcon i pcid
    REG_0120_TCON_ADL_CRC_BKA369_V004_REG_CLIENTK_BASEADR_1_0120, //tcon k vac
};

static uint32_t _au32AdlRegEnable[E_PNL_XC_AUTODOWNLOAD_CLIENT_MAX] = {
    REG_0004_TCON_ADL_BKA367_V004 - BKA33B_OFST, //tcon 1 panel gamma
    REG_01C4_TCON_ADL_BKA367_V004 - BKA33B_OFST, //tcon g od
    REG_01A8_TCON_ADL_CRC_BKA369_V004 - BKA33C_OFST,//tcon i pcid
    REG_0118_TCON_ADL_CRC_BKA369_V004 - BKA33C_OFST, //tcon k vac
};

static uint32_t _au32AdlRegEnableMask[E_PNL_XC_AUTODOWNLOAD_CLIENT_MAX] = {
    REG_0004_TCON_ADL_BKA367_V004_REG_CLIENT1_EN_0004, //tcon 1 panel gamma
    REG_01C4_TCON_ADL_BKA367_V004_REG_CLIENTG_EN_01C4, //tcon g od
    REG_01A8_TCON_ADL_CRC_BKA369_V004_REG_CLIENTI_EN_01A8,//tcon i pcid
    REG_0118_TCON_ADL_CRC_BKA369_V004_REG_CLIENTK_EN_0118, //tcon k vac
};

static uint32_t _au32AdlRegWorkMode[E_PNL_XC_AUTODOWNLOAD_CLIENT_MAX] = {
    REG_0004_TCON_ADL_BKA367_V004 - BKA33B_OFST, //tcon 1 panel gamma
    REG_01C4_TCON_ADL_BKA367_V004 - BKA33B_OFST, //tcon g od
    REG_01A8_TCON_ADL_CRC_BKA369_V004 - BKA33C_OFST,//tcon i pcid
    REG_0118_TCON_ADL_CRC_BKA369_V004 - BKA33C_OFST, //tcon k vac
};

static uint32_t _au32AdlRegWorkModeMask[E_PNL_XC_AUTODOWNLOAD_CLIENT_MAX] = {
    REG_0004_TCON_ADL_BKA367_V004_REG_CLIENT1_TRIG_MODE_0004, //tcon 1 panel gamma
    REG_01C4_TCON_ADL_BKA367_V004_REG_CLIENTG_TRIG_MODE_01C4, //tcon g od
    REG_01A8_TCON_ADL_CRC_BKA369_V004_REG_CLIENTI_TRIG_MODE_01A8,//tcon i pcid
    REG_0118_TCON_ADL_CRC_BKA369_V004_REG_CLIENTK_TRIG_MODE_0118, //tcon k vac
};

static uint32_t _au32AdlRegDepth[E_PNL_XC_AUTODOWNLOAD_CLIENT_MAX] = {
    REG_0020_TCON_ADL_BKA367_V004 - BKA33B_OFST, //tcon 1 panel gamma
    REG_01D0_TCON_ADL_BKA367_V004 - BKA33B_OFST, //tcon g od
    REG_01B4_TCON_ADL_CRC_BKA369_V004 - BKA33C_OFST,//tcon i pcid
    REG_0124_TCON_ADL_CRC_BKA369_V004 - BKA33C_OFST, //tcon k vac
};

static uint32_t _au32AdlRegReqLength[E_PNL_XC_AUTODOWNLOAD_CLIENT_MAX] = {
    REG_002C_TCON_ADL_BKA367_V004 - BKA33B_OFST, //tcon 1 panel gamma
    REG_01D4_TCON_ADL_BKA367_V004 - BKA33B_OFST, //tcon g od
    REG_01B8_TCON_ADL_CRC_BKA369_V004 - BKA33C_OFST,//tcon i pcid
    REG_0128_TCON_ADL_CRC_BKA369_V004 - BKA33C_OFST, //tcon k vac
};

static uint32_t _au32AdlRegFakeActiveEn[E_PNL_XC_AUTODOWNLOAD_CLIENT_MAX] = {
    /*TCON Auto download*/
    REG_0020_TCON_ADL_CRC_BKA369 - BKA33B_OFST, //panel_gamma client (client1)
    REG_0024_TCON_ADL_CRC_BKA369 - BKA33B_OFST, //od client (clientG)
    REG_0024_TCON_ADL_CRC_BKA369 - BKA33B_OFST, //pcid client (clientI)
    REG_0024_TCON_ADL_CRC_BKA369 - BKA33B_OFST, //vac client (clientK)
};

static uint32_t _au32AdlRegFakeActiveEnMask[E_PNL_XC_AUTODOWNLOAD_CLIENT_MAX] = {
    /*TCON Auto download*/
    REG_0020_TCON_ADL_CRC_BKA369_REG_FAKE_ACTIVE_EN_0_0020_0,  //client1
    REG_0024_TCON_ADL_CRC_BKA369_REG_FAKE_ACTIVE_EN_1_0024_17, //clientG
    REG_0024_TCON_ADL_CRC_BKA369_REG_FAKE_ACTIVE_EN_1_0024_19, //clientI
    REG_0024_TCON_ADL_CRC_BKA369_REG_FAKE_ACTIVE_EN_1_0024_21, //clientK
};
//-------------------------------------------------------------------------------------------------
//  Debug Functions
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Local Functions
//-------------------------------------------------------------------------------------------------
static bool _KHal_XC_PA2VA(EN_PNL_XC_AUTODOWNLOAD_CLIENT enClient)
{
    if (CHECK_CLIENT_RANGE_VALID(enClient) == TRUE)
    {
        u64  u64Offset = 0;
        uint32_t u32MiuNo = _stClientInfo[enClient].u32MiuNo;
        u64 phyBaseAddr = _stClientInfo[enClient].phyBaseAddr;
        KHal_XC_GetMiuOffset( u32MiuNo,&u64Offset);
        _stClientInfo[enClient].busBaseAddr = phyBaseAddr+u64Offset;
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

static uint32_t _KHal_XC_GetAutoDownloadMemOffset(EN_PNL_XC_AUTODOWNLOAD_CLIENT enClient, uint8_t u8Index)
{
    uint32_t u32Offset = 0;

    return u32Offset;
}

static void _KHal_XC_AdlSpinLock(EN_PNL_XC_AUTODOWNLOAD_CLIENT enClient)
{
    if (CHECK_CLIENT_RANGE_VALID(enClient) == TRUE)
    {
        spin_lock(&_astAdlSpinlock[enClient]);
        while (_bAdlSetting[enClient] == TRUE)
        {
            spin_unlock(&_astAdlSpinlock[enClient]);
            udelay(10);       /// delay 10 us
            spin_lock(&_astAdlSpinlock[enClient]);
        }
        _bAdlSetting[enClient] = TRUE;
        spin_unlock(&_astAdlSpinlock[enClient]);
    }
}

static void _KHal_XC_AdlSpinUnLock(EN_PNL_XC_AUTODOWNLOAD_CLIENT enClient)
{
    if (CHECK_CLIENT_RANGE_VALID(enClient) == TRUE)
    {
        spin_lock(&_astAdlSpinlock[enClient]);
        _bAdlSetting[enClient] = FALSE;
        spin_unlock(&_astAdlSpinlock[enClient]);
    }
}

static bool _KHal_XC_Set_Auto_Download_WorkMode(uint32_t u32RegAddr, uint32_t u32RegMask, EN_PNL_XC_AUTODOWNLOAD_MODE enMode)
{
    switch (enMode)
    {
        case E_PNL_XC_AUTODOWNLOAD_TRIGGER_MODE:
        {
            W2BYTEMSK(u32RegAddr, 0, u32RegMask);
            break;
        }
        case E_PNL_XC_AUTODOWNLOAD_ENABLE_MODE:
        {
            W2BYTEMSK(u32RegAddr, 1, u32RegMask);
            break;
        }
        default:
            return FALSE;
    }

    return TRUE;
}

bool _KHal_XC_IsAutoDownload_Active(EN_PNL_XC_AUTODOWNLOAD_CLIENT enClient)
{
    bool bStatus = FALSE;
    if (CHECK_CLIENT_RANGE_VALID(enClient) == TRUE)
    {
        bStatus = (bool)(R2BYTEMSK(_au32AdlRegEnable[enClient],_au32AdlRegEnableMask[enClient]));
    }
    return bStatus;
}

uint32_t _KHal_XC_Get_AutoDownload_CleanSize(EN_PNL_XC_AUTODOWNLOAD_CLIENT enClient)
{
    uint32_t u32Size = 0;
    if (CHECK_CLIENT_RANGE_VALID(enClient) == TRUE)
    {
        u32Size = _stClientInfo[enClient].u32Size;
    }
    return u32Size;
}

bool _KHal_XC_IsAutoDownload_NeededClean(EN_PNL_XC_AUTODOWNLOAD_CLIENT enClient)
{
    bool bNeededClean = TRUE;

    return bNeededClean;
}

bool _KHal_XC_IsAutoDownload_Available(EN_PNL_XC_AUTODOWNLOAD_CLIENT enClient, uint8_t* pu8BaseAddr)
{
    bool bFireing[E_PNL_XC_AUTODOWNLOAD_CLIENT_MAX] = {0};
    uint32_t u32CleanSize = 0;
    uint16_t u16InputVFreq = 0;
    uint16_t u16FramePeriod = 0;
    bool bNeededClean = TRUE;
    u64 time_start = 0,time_end = 0,time_diff = 0;

    if (CHECK_CLIENT_RANGE_VALID(enClient) == FALSE)
    {
        return FALSE;
    }

    u16InputVFreq = DEFAULT_OUTPUT_VSYNC;

    u16FramePeriod = 10000/u16InputVFreq;

    bFireing[enClient] = _KHal_XC_IsAutoDownload_Active(enClient);

    time_start = get_timer(0);

    while (bFireing[enClient] == TRUE)
    {
        if ((time_diff) >= u16FramePeriod)
        {
            UBOOT_ERROR("[%s][%d] Client:%d waiting adl firing done time out\n",__func__, __LINE__,enClient);
            break;
        }
        mdelay(1);
        bFireing[enClient] = _KHal_XC_IsAutoDownload_Active(enClient);
        time_end = get_timer(0);
        time_diff = time_end-time_start;
    }

    // writing data now
    if (bFireing[enClient] == TRUE)
    {
        UBOOT_ERROR("[%s][%d] Client:%d Wait a moment, the lastest datas is writing!! InputVFreq = %u\n",__func__, __LINE__, enClient, u16InputVFreq);
        return FALSE;
    }
    else
    {
        // the lasteset datas is writing done.
        if (_stClientInfo[enClient].enStatus == E_PNL_XC_AUTO_DOWNLOAD_FIRED)
        {
            uint8_t *pu8TmpAddr = NULL;
            uint32_t u32MemOffset = _KHal_XC_GetAutoDownloadMemOffset(enClient, _stClientInfo[enClient].u8SectionIdx);

            pu8TmpAddr = pu8BaseAddr + u32MemOffset;

            // clear buffer
            u32CleanSize = _KHal_XC_Get_AutoDownload_CleanSize(enClient);
            bNeededClean = _KHal_XC_IsAutoDownload_NeededClean(enClient);
            if((u32CleanSize != 0) && bNeededClean)
            {
                memset(pu8TmpAddr, 0, u32CleanSize);
            }
            _stClientInfo[enClient].u32StartAddr = 0xFFFFFFFF;
            _stClientInfo[enClient].u32Depth = 0;
            _stClientInfo[enClient].enStatus = E_PNL_XC_AUTO_DOWNLOAD_WRITED;
        }
        else
        {
            _stClientInfo[enClient].enStatus = E_PNL_XC_AUTO_DOWNLOAD_WRITED;
        }
    }

    return TRUE;
}

bool KHal_XC_IsSupportHFRAdl(void)
{
    return SUPPORT_HFR_ADL;
}

bool KHal_XC_IsSupportAISR(void)
{
    return SUPPORT_XC_AISR;
}

//-------------------------------------------------------------------------------------------------
//  Global Functions
bool KHal_XC_GetMiuOffset(uint32_t u32MiuNo, u64 *pu64Offset)
{
    if (u32MiuNo == 0)
    {
        *pu64Offset = MSTAR_MIU0_BUS_BASE;
    }
    else if (u32MiuNo == 1)
    {
        *pu64Offset = MSTAR_MIU1_BUS_BASE;
    }
    else
    {
        return FALSE;
    }

    return TRUE;
}

bool KHal_XC_InitAutoDownload(struct udevice *dev)
{
    struct mtk_panel_priv *priv = dev_get_priv(dev);
    uint32_t pnl_version = 0;

    KHal_XC_InitAutoDownloadSpinlock();
    //watch dog timer to max
    //W2BYTEMSK(REG_01C0_MW_ADL_BKA352, REG_01C0_MW_ADL_BKA352_REG_WD_TIMER_01C0, REG_01C0_MW_ADL_BKA352_REG_WD_TIMER_01C0);
    W2BYTEMSK(REG_01C0_TCON_ADL_BKA367_V004, 0xfff, REG_01C0_TCON_ADL_BKA367_V004_REG_WD_TIMER_01C0);

    if (priv == NULL) {
		UBOOT_ERROR("get device private fail\n");
		return FALSE;
    }
    pnl_version = priv->pnl_lib_version;
    UBOOT_DEBUG("panel lib version=%d\n", pnl_version);

    switch (pnl_version) {
	case VERSION0:
	case VERSION1:
	case VERSION2:
	case VERSION3:
	case VERSION4:
	case VERSION5:
		_au32AdlMemBaseUnit = AUTO_DOWNLOAD_MEM_BASE_UNIT_256;
		_au32AdlBusWidth = AUTO_DOWNLOAD_BUS_WIDTH_256;
		break;
	case VERSION6:
		_au32AdlMemBaseUnit = AUTO_DOWNLOAD_MEM_BASE_UNIT_256;
		_au32AdlBusWidth = AUTO_DOWNLOAD_BUS_WIDTH_128;
		break;
	default:
		UBOOT_ERROR("Invalid panel lib version=%d\n", pnl_version);
		return FALSE;
	}
    UBOOT_DEBUG("_au32AdlMemBaseUnit=%d\n", _au32AdlMemBaseUnit);
    UBOOT_DEBUG("_au32AdlBusWidth=%d\n", _au32AdlBusWidth);

    if (pnl_version == VERSION6) {
		_au32AdlRegClkGate[E_PNL_XC_AUTODOWNLOAD_CLIENT_PANEL_GAMMA] = REG_0004_TCON_ADL_BKA367_V006 - BKA33B_OFST;	//tcon 1 panel gamma
		_au32AdlRegClkGate[E_PNL_XC_AUTODOWNLOAD_CLIENT_OD]          = REG_01C4_TCON_ADL_BKA367_V006 - BKA33B_OFST;	//tcon g od
		_au32AdlRegClkGate[E_PNL_XC_AUTODOWNLOAD_CLIENT_PCID]        = REG_01A8_TCON_ADL_CRC_BKA369_V006 - BKA33C_OFST;	//tcon i pcid
		_au32AdlRegClkGate[E_PNL_XC_AUTODOWNLOAD_CLIENT_VAC]         = REG_ADL_CLIENT_NOT_SUPPORT;	//tcon k vac

		_au32AdlRegClkGateMask[E_PNL_XC_AUTODOWNLOAD_CLIENT_PANEL_GAMMA] = REG_0004_TCON_ADL_BKA367_V006_REG_CLIENT1_CLK_GATE_EN_0004;
		_au32AdlRegClkGateMask[E_PNL_XC_AUTODOWNLOAD_CLIENT_OD]          = REG_01C4_TCON_ADL_BKA367_V006_REG_CLIENTG_CLK_GATE_EN_01C4;
		_au32AdlRegClkGateMask[E_PNL_XC_AUTODOWNLOAD_CLIENT_PCID]        = REG_01A8_TCON_ADL_CRC_BKA369_V006_REG_CLIENTI_CLK_GATE_EN_01A8;
		_au32AdlRegClkGateMask[E_PNL_XC_AUTODOWNLOAD_CLIENT_VAC]         = REG_ADL_CLIENT_NOT_SUPPORT;

		_au32AdlRegBassAddr_L[E_PNL_XC_AUTODOWNLOAD_CLIENT_PANEL_GAMMA] =  REG_0008_TCON_ADL_BKA367_V006 - BKA33B_OFST;
		_au32AdlRegBassAddr_L[E_PNL_XC_AUTODOWNLOAD_CLIENT_OD]          = REG_01C8_TCON_ADL_BKA367_V006 - BKA33B_OFST;
		_au32AdlRegBassAddr_L[E_PNL_XC_AUTODOWNLOAD_CLIENT_PCID]        = REG_01AC_TCON_ADL_CRC_BKA369_V006 - BKA33C_OFST;
		_au32AdlRegBassAddr_L[E_PNL_XC_AUTODOWNLOAD_CLIENT_VAC]         = REG_ADL_CLIENT_NOT_SUPPORT;

		_au32AdlRegBassAddr_L_Mask[E_PNL_XC_AUTODOWNLOAD_CLIENT_PANEL_GAMMA] = REG_0008_TCON_ADL_BKA367_V006_REG_CLIENT1_BASEADR_0_0008;
		_au32AdlRegBassAddr_L_Mask[E_PNL_XC_AUTODOWNLOAD_CLIENT_OD]          = REG_01C8_TCON_ADL_BKA367_V006_REG_CLIENTG_BASEADR_0_01C8;
		_au32AdlRegBassAddr_L_Mask[E_PNL_XC_AUTODOWNLOAD_CLIENT_PCID]        = REG_01AC_TCON_ADL_CRC_BKA369_V006_REG_CLIENTI_BASEADR_0_01AC;
		_au32AdlRegBassAddr_L_Mask[E_PNL_XC_AUTODOWNLOAD_CLIENT_VAC]         = REG_ADL_CLIENT_NOT_SUPPORT;

		_au32AdlRegBassAddr_H[E_PNL_XC_AUTODOWNLOAD_CLIENT_PANEL_GAMMA] = REG_000C_TCON_ADL_BKA367_V006 - BKA33B_OFST;
		_au32AdlRegBassAddr_H[E_PNL_XC_AUTODOWNLOAD_CLIENT_OD]          = REG_01CC_TCON_ADL_BKA367_V006 - BKA33B_OFST;
		_au32AdlRegBassAddr_H[E_PNL_XC_AUTODOWNLOAD_CLIENT_PCID]        = REG_01B0_TCON_ADL_CRC_BKA369_V006 - BKA33C_OFST;
		_au32AdlRegBassAddr_H[E_PNL_XC_AUTODOWNLOAD_CLIENT_VAC]         = REG_ADL_CLIENT_NOT_SUPPORT;

		_au32AdlRegBassAddr_H_Mask[E_PNL_XC_AUTODOWNLOAD_CLIENT_PANEL_GAMMA] = REG_000C_TCON_ADL_BKA367_V006_REG_CLIENT1_BASEADR_1_000C;
		_au32AdlRegBassAddr_H_Mask[E_PNL_XC_AUTODOWNLOAD_CLIENT_OD]          = REG_01CC_TCON_ADL_BKA367_V006_REG_CLIENTG_BASEADR_1_01CC;
		_au32AdlRegBassAddr_H_Mask[E_PNL_XC_AUTODOWNLOAD_CLIENT_PCID]        = REG_01B0_TCON_ADL_CRC_BKA369_V006_REG_CLIENTI_BASEADR_1_01B0;
		_au32AdlRegBassAddr_H_Mask[E_PNL_XC_AUTODOWNLOAD_CLIENT_VAC]         = REG_ADL_CLIENT_NOT_SUPPORT;

		_au32AdlRegEnable[E_PNL_XC_AUTODOWNLOAD_CLIENT_PANEL_GAMMA] = REG_0004_TCON_ADL_BKA367_V006 - BKA33B_OFST;
		_au32AdlRegEnable[E_PNL_XC_AUTODOWNLOAD_CLIENT_OD]          = REG_01C4_TCON_ADL_BKA367_V006 - BKA33B_OFST;
		_au32AdlRegEnable[E_PNL_XC_AUTODOWNLOAD_CLIENT_PCID]        = REG_01A8_TCON_ADL_CRC_BKA369_V006 - BKA33C_OFST;
		_au32AdlRegEnable[E_PNL_XC_AUTODOWNLOAD_CLIENT_VAC]         = REG_ADL_CLIENT_NOT_SUPPORT;

		_au32AdlRegEnableMask[E_PNL_XC_AUTODOWNLOAD_CLIENT_PANEL_GAMMA] = REG_0004_TCON_ADL_BKA367_V006_REG_CLIENT1_EN_0004;
		_au32AdlRegEnableMask[E_PNL_XC_AUTODOWNLOAD_CLIENT_OD]          = REG_01C4_TCON_ADL_BKA367_V006_REG_CLIENTG_EN_01C4;
		_au32AdlRegEnableMask[E_PNL_XC_AUTODOWNLOAD_CLIENT_PCID]        = REG_01A8_TCON_ADL_CRC_BKA369_V006_REG_CLIENTI_EN_01A8;
		_au32AdlRegEnableMask[E_PNL_XC_AUTODOWNLOAD_CLIENT_VAC]         = REG_ADL_CLIENT_NOT_SUPPORT;

		_au32AdlRegWorkMode[E_PNL_XC_AUTODOWNLOAD_CLIENT_PANEL_GAMMA] = REG_0004_TCON_ADL_BKA367_V006 - BKA33B_OFST;
		_au32AdlRegWorkMode[E_PNL_XC_AUTODOWNLOAD_CLIENT_OD]          = REG_01C4_TCON_ADL_BKA367_V006 - BKA33B_OFST;
		_au32AdlRegWorkMode[E_PNL_XC_AUTODOWNLOAD_CLIENT_PCID]        = REG_01A8_TCON_ADL_CRC_BKA369_V006 - BKA33C_OFST;
		_au32AdlRegWorkMode[E_PNL_XC_AUTODOWNLOAD_CLIENT_VAC]         = REG_ADL_CLIENT_NOT_SUPPORT;

		_au32AdlRegWorkModeMask[E_PNL_XC_AUTODOWNLOAD_CLIENT_PANEL_GAMMA] = REG_0004_TCON_ADL_BKA367_V006_REG_CLIENT1_TRIG_MODE_0004;
		_au32AdlRegWorkModeMask[E_PNL_XC_AUTODOWNLOAD_CLIENT_OD]          = REG_01C4_TCON_ADL_BKA367_V006_REG_CLIENTG_TRIG_MODE_01C4;
		_au32AdlRegWorkModeMask[E_PNL_XC_AUTODOWNLOAD_CLIENT_PCID]        = REG_01A8_TCON_ADL_CRC_BKA369_V006_REG_CLIENTI_TRIG_MODE_01A8;
		_au32AdlRegWorkModeMask[E_PNL_XC_AUTODOWNLOAD_CLIENT_VAC]         = REG_ADL_CLIENT_NOT_SUPPORT;

		_au32AdlRegDepth[E_PNL_XC_AUTODOWNLOAD_CLIENT_PANEL_GAMMA] = REG_0020_TCON_ADL_BKA367_V006 - BKA33B_OFST;
		_au32AdlRegDepth[E_PNL_XC_AUTODOWNLOAD_CLIENT_OD]          = REG_01D0_TCON_ADL_BKA367_V006 - BKA33B_OFST;
		_au32AdlRegDepth[E_PNL_XC_AUTODOWNLOAD_CLIENT_PCID]        = REG_01B4_TCON_ADL_CRC_BKA369_V006 - BKA33C_OFST;
		_au32AdlRegDepth[E_PNL_XC_AUTODOWNLOAD_CLIENT_VAC]         = REG_ADL_CLIENT_NOT_SUPPORT;

		_au32AdlRegReqLength[E_PNL_XC_AUTODOWNLOAD_CLIENT_PANEL_GAMMA] = REG_002C_TCON_ADL_BKA367_V006 - BKA33B_OFST;
		_au32AdlRegReqLength[E_PNL_XC_AUTODOWNLOAD_CLIENT_OD]          = REG_01D4_TCON_ADL_BKA367_V006 - BKA33B_OFST;
		_au32AdlRegReqLength[E_PNL_XC_AUTODOWNLOAD_CLIENT_PCID]        = REG_01B8_TCON_ADL_CRC_BKA369_V006 - BKA33C_OFST;
		_au32AdlRegReqLength[E_PNL_XC_AUTODOWNLOAD_CLIENT_VAC]         = REG_ADL_CLIENT_NOT_SUPPORT;

		_au32AdlRegFakeActiveEn[E_PNL_XC_AUTODOWNLOAD_CLIENT_PANEL_GAMMA] = REG_0020_TCON_ADL_CRC_BKA369 - BKA33B_OFST;
		_au32AdlRegFakeActiveEn[E_PNL_XC_AUTODOWNLOAD_CLIENT_OD]          = REG_0024_TCON_ADL_CRC_BKA369 - BKA33B_OFST;
		_au32AdlRegFakeActiveEn[E_PNL_XC_AUTODOWNLOAD_CLIENT_PCID]        = REG_0024_TCON_ADL_CRC_BKA369 - BKA33B_OFST;
		_au32AdlRegFakeActiveEn[E_PNL_XC_AUTODOWNLOAD_CLIENT_VAC]         = REG_ADL_CLIENT_NOT_SUPPORT;

		_au32AdlRegFakeActiveEnMask[E_PNL_XC_AUTODOWNLOAD_CLIENT_PANEL_GAMMA] = REG_0020_TCON_ADL_CRC_BKA369_REG_FAKE_ACTIVE_EN_0_0020_0;
		_au32AdlRegFakeActiveEnMask[E_PNL_XC_AUTODOWNLOAD_CLIENT_OD]          = REG_0024_TCON_ADL_CRC_BKA369_REG_FAKE_ACTIVE_EN_1_0024_17;
		_au32AdlRegFakeActiveEnMask[E_PNL_XC_AUTODOWNLOAD_CLIENT_PCID]        = REG_0024_TCON_ADL_CRC_BKA369_REG_FAKE_ACTIVE_EN_1_0024_19;
		_au32AdlRegFakeActiveEnMask[E_PNL_XC_AUTODOWNLOAD_CLIENT_VAC]         = REG_ADL_CLIENT_NOT_SUPPORT;
	}
    return TRUE;
}

bool KHal_XC_InitAutoDownloadSpinlock(void)
{
    EN_PNL_XC_AUTODOWNLOAD_CLIENT enClient;

    for(enClient = E_PNL_XC_AUTODOWNLOAD_CLIENT_PANEL_GAMMA; enClient < E_PNL_XC_AUTODOWNLOAD_CLIENT_MAX; enClient++)
    {
        spin_lock_init(&_astAdlSpinlock[enClient]);
    }

    memset(_bAdlSetting, 0, (E_PNL_XC_AUTODOWNLOAD_CLIENT_MAX) * sizeof(bool));
    memset(_stClientInfo, 0, E_PNL_XC_AUTODOWNLOAD_CLIENT_MAX * sizeof(ST_PNL_XC_AUTODOWNLOAD_CLIENT_INFO));

    return TRUE;
}

bool KHal_XC_ConfigAutoDownload(EN_PNL_XC_AUTODOWNLOAD_CLIENT enClient, bool bEnable,
                                EN_PNL_XC_AUTODOWNLOAD_MODE enMode, u64 phyBaseAddr, uint32_t u32Size, uint32_t u32MiuNo)
{
    EN_PNL_XC_AUTODOWNLOAD_CLIENT enClientTmp;
    uint16_t u16Idx;
    u64 phyBaseAddrOffset = 0;
    if ((CHECK_CLIENT_RANGE_VALID(enClient) == FALSE) || (CHECK_ADL_WIDTH_VALID() == FALSE))
    {
		UBOOT_ERROR("Invalid input parameter!\n");
        return FALSE;
    }

    _ADL_SPIN_LOCK(enClient);

    phyBaseAddrOffset += ALIGN_UPTO_MIU_WORD(AUTODOWNLOAD_CLIENT_HDR_MEM_SIZE);
    phyBaseAddrOffset += ALIGN_UPTO_MIU_WORD(AUTODOWNLOAD_CLIENT_OP2LUT_MEM_SIZE);
    phyBaseAddrOffset += ALIGN_UPTO_MIU_WORD(AUTODOWNLOAD_CLIENT_HVSP_MEM_SIZE);
    phyBaseAddrOffset += ALIGN_UPTO_MIU_WORD(AUTODOWNLOAD_CLIENT_HSY_MEM_SIZE);
    phyBaseAddrOffset += ALIGN_UPTO_MIU_WORD(AUTODOWNLOAD_CLIENT_PQ_GAMMA_MEM_SIZE);
    phyBaseAddrOffset += ALIGN_UPTO_MIU_WORD(AUTODOWNLOAD_CLIENT_DLC_256_MEM_SIZE);
    // -------pannel gamma client config start------------
    enClientTmp = E_PNL_XC_AUTODOWNLOAD_CLIENT_PANEL_GAMMA;
    _stClientInfo[enClientTmp].phyBaseAddr = phyBaseAddr + phyBaseAddrOffset;
    _stClientInfo[enClientTmp].u32Size = AUTODOWNLOAD_CLIENT_PANELGAMMA_MEM_SIZE;
    _stClientInfo[enClientTmp].u32MiuNo = u32MiuNo;
    _stClientInfo[enClientTmp].u8SectionIdx = 0;
    _stClientInfo[enClientTmp].u8MaxSection = 1;
    _stClientInfo[enClientTmp].u32StartAddr = 0xFFFFFFFF;
    _stClientInfo[enClientTmp].u32Depth = 0;
    _stClientInfo[enClientTmp].enMode = enMode;
    _stClientInfo[enClientTmp].enStatus = E_PNL_XC_AUTO_DOWNLOAD_CONFIGURED;
    _stClientInfo[enClientTmp].bFakeSupport = 0;
    phyBaseAddrOffset += ALIGN_UPTO_MIU_WORD(AUTODOWNLOAD_CLIENT_PANELGAMMA_MEM_SIZE);
    // -------panel gamma client config end--------------

    // -------OverDriver (OD) client config start------------
    enClientTmp = E_PNL_XC_AUTODOWNLOAD_CLIENT_OD;
    _stClientInfo[enClientTmp].phyBaseAddr = phyBaseAddr + phyBaseAddrOffset;
    _stClientInfo[enClientTmp].u32Size = AUTODOWNLOAD_CLIENT_OVERDRIVER_MEM_SIZE;
    _stClientInfo[enClientTmp].u32MiuNo = u32MiuNo;
    _stClientInfo[enClientTmp].u8SectionIdx = 0;
    _stClientInfo[enClientTmp].u8MaxSection = 1;
    _stClientInfo[enClientTmp].u32StartAddr = 0xFFFFFFFF;
    _stClientInfo[enClientTmp].u32Depth = 0;
    _stClientInfo[enClientTmp].enMode = enMode;
    _stClientInfo[enClientTmp].enStatus = E_PNL_XC_AUTO_DOWNLOAD_CONFIGURED;
    _stClientInfo[enClientTmp].bFakeSupport = 0;
    phyBaseAddrOffset += ALIGN_UPTO_MIU_WORD(AUTODOWNLOAD_CLIENT_OVERDRIVER_MEM_SIZE);
    // -------OverDriver (OD) client config end--------------

    phyBaseAddrOffset += ALIGN_UPTO_MIU_WORD(AUTODOWNLOAD_CLIENT_ADL2RIU_MEM_SIZE);
    phyBaseAddrOffset += ALIGN_UPTO_MIU_WORD(AUTODOWNLOAD_CLIENT_3DLUT_MEM_SIZE);
    phyBaseAddrOffset += ALIGN_UPTO_MIU_WORD(AUTODOWNLOAD_CLIENT_UCD_STAT_TOP_MEM_SIZE);
    phyBaseAddrOffset += ALIGN_UPTO_MIU_WORD(AUTODOWNLOAD_CLIENT_GUCD_B2P_MEM_SIZE);
    phyBaseAddrOffset += ALIGN_UPTO_MIU_WORD(AUTODOWNLOAD_CLIENT_GUCD_18_LUT_MEM_SIZE);
    phyBaseAddrOffset += ALIGN_UPTO_MIU_WORD(AUTODOWNLOAD_CLIENT_GUCD_256_LUT_MEM_SIZE);
    phyBaseAddrOffset += ALIGN_UPTO_MIU_WORD(AUTODOWNLOAD_CLIENT_DC0_FILM_GRAIN_MEM_SIZE);

    if(KHal_XC_IsSupportHFRAdl())
    {
        phyBaseAddrOffset += ALIGN_UPTO_MIU_WORD(AUTODOWNLOAD_CLIENT_HFR_VIP_DLC_256_MEM_SIZE);
        phyBaseAddrOffset += ALIGN_UPTO_MIU_WORD(AUTODOWNLOAD_CLIENT_HFR_HDR_MEM_SIZE);
        phyBaseAddrOffset += ALIGN_UPTO_MIU_WORD(AUTODOWNLOAD_CLIENT_HFR_3DLUT_MEM_SIZE);
    }

    if(KHal_XC_IsSupportAISR())
    {
        phyBaseAddrOffset += ALIGN_UPTO_MIU_WORD(AUTODOWNLOAD_CLIENT_AISR_HVSP_MEM_SIZE);
        phyBaseAddrOffset += ALIGN_UPTO_MIU_WORD(AUTODOWNLOAD_CLIENT_AISR_PQ_MEM_SIZE);
    }

    phyBaseAddrOffset += ALIGN_UPTO_MIU_WORD(AUTODOWNLOAD_CLIENT_XVYCC_MEM_SIZE);
    phyBaseAddrOffset += ALIGN_UPTO_MIU_WORD(AUTODOWNLOAD_CLIENT_DV_HW5_LUT_MEM_SIZE);
    phyBaseAddrOffset += ALIGN_UPTO_MIU_WORD(AUTODOWNLOAD_CLIENT_SPF_MEM_SIZE);

    // -------PCID client config start------------
    enClientTmp = E_PNL_XC_AUTODOWNLOAD_CLIENT_PCID;
    _stClientInfo[enClientTmp].phyBaseAddr = phyBaseAddr + phyBaseAddrOffset;
    _stClientInfo[enClientTmp].u32Size = AUTODOWNLOAD_CLIENT_PCID_MEM_SIZE;
    _stClientInfo[enClientTmp].u32MiuNo = u32MiuNo;
    _stClientInfo[enClientTmp].u8SectionIdx = 0;
    _stClientInfo[enClientTmp].u8MaxSection = 1;
    _stClientInfo[enClientTmp].u32StartAddr = 0xFFFFFFFF;
    _stClientInfo[enClientTmp].u32Depth = 0;
    _stClientInfo[enClientTmp].enMode = enMode;
    _stClientInfo[enClientTmp].enStatus = E_PNL_XC_AUTO_DOWNLOAD_CONFIGURED;
    _stClientInfo[enClientTmp].bFakeSupport = 0;
    phyBaseAddrOffset += ALIGN_UPTO_MIU_WORD(AUTODOWNLOAD_CLIENT_PCID_MEM_SIZE);
    // -------PCID client config end------------

    // -------VAC client config start------------
    enClientTmp = E_PNL_XC_AUTODOWNLOAD_CLIENT_VAC;
    _stClientInfo[enClientTmp].phyBaseAddr = phyBaseAddr + phyBaseAddrOffset;
    _stClientInfo[enClientTmp].u32Size = AUTODOWNLOAD_CLIENT_VAC_MEM_SIZE;
    _stClientInfo[enClientTmp].u32MiuNo = u32MiuNo;
    _stClientInfo[enClientTmp].u8SectionIdx = 0;
    _stClientInfo[enClientTmp].u8MaxSection = 1;
    _stClientInfo[enClientTmp].u32StartAddr = 0xFFFFFFFF;
    _stClientInfo[enClientTmp].u32Depth = 0;
    _stClientInfo[enClientTmp].enMode = enMode;
    _stClientInfo[enClientTmp].enStatus = E_PNL_XC_AUTO_DOWNLOAD_CONFIGURED;
    _stClientInfo[enClientTmp].bFakeSupport = 0;
    phyBaseAddrOffset += ALIGN_UPTO_MIU_WORD(AUTODOWNLOAD_CLIENT_VAC_MEM_SIZE);
    // -------VAC client config end------------
    _ADL_SPIN_UNLOCK(enClient);

    // -------pannel gamma client config start------------
    enClientTmp = E_PNL_XC_AUTODOWNLOAD_CLIENT_PANEL_GAMMA;
    _KHal_XC_PA2VA(enClientTmp);
    _stClientInfo[enClientTmp].bEnable = bEnable;
    //set client1 baseaddr
    W2BYTEMSK(_au32AdlRegClkGate[enClientTmp], 1, _au32AdlRegClkGateMask[enClientTmp]);
    W2BYTEMSK(_au32AdlRegBassAddr_L[enClientTmp], (_stClientInfo[enClientTmp].phyBaseAddr / _au32AdlMemBaseUnit) & 0x0000FFFF, _au32AdlRegBassAddr_L_Mask[enClientTmp]);
    W2BYTEMSK(_au32AdlRegBassAddr_H[enClientTmp], ((_stClientInfo[enClientTmp].phyBaseAddr / _au32AdlMemBaseUnit) >> 16 & 0x0000FFFF), _au32AdlRegBassAddr_H_Mask[enClientTmp]);
    //set client1 work mode
    _KHal_XC_Set_Auto_Download_WorkMode(_au32AdlRegWorkMode[E_PNL_XC_AUTODOWNLOAD_CLIENT_PANEL_GAMMA], _au32AdlRegWorkModeMask[E_PNL_XC_AUTODOWNLOAD_CLIENT_PANEL_GAMMA], enMode);
    // -------panel client config end--------------

    // -------OverDriver (OD) client config start------------
    enClientTmp = E_PNL_XC_AUTODOWNLOAD_CLIENT_OD;
    _KHal_XC_PA2VA(enClientTmp);
    _stClientInfo[enClientTmp].bEnable = bEnable;
    //set client2 baseaddr
    W2BYTEMSK(_au32AdlRegClkGate[enClientTmp], 1, _au32AdlRegClkGateMask[enClientTmp]);
    W2BYTEMSK(_au32AdlRegBassAddr_L[enClientTmp], (_stClientInfo[enClientTmp].phyBaseAddr / _au32AdlMemBaseUnit) & 0x0000FFFF, _au32AdlRegBassAddr_L_Mask[enClientTmp]);
    W2BYTEMSK(_au32AdlRegBassAddr_H[enClientTmp], ((_stClientInfo[enClientTmp].phyBaseAddr / _au32AdlMemBaseUnit) >> 16 & 0x0000FFFF), _au32AdlRegBassAddr_H_Mask[enClientTmp]);
    //set client2 work mode
    _KHal_XC_Set_Auto_Download_WorkMode(_au32AdlRegWorkMode[E_PNL_XC_AUTODOWNLOAD_CLIENT_OD], _au32AdlRegWorkModeMask[E_PNL_XC_AUTODOWNLOAD_CLIENT_OD], enMode);
    // -------OverDriver (OD) client config end--------------

    // -------PCID client config start------------
    enClientTmp = E_PNL_XC_AUTODOWNLOAD_CLIENT_PCID;
    _KHal_XC_PA2VA(enClientTmp);
    _stClientInfo[enClientTmp].bEnable = bEnable;

    //set clientL baseaddr
    W2BYTEMSK(_au32AdlRegClkGate[enClientTmp], 1, _au32AdlRegClkGateMask[enClientTmp]);
    W2BYTEMSK(_au32AdlRegBassAddr_L[enClientTmp], (_stClientInfo[enClientTmp].phyBaseAddr / _au32AdlMemBaseUnit) & 0x0000FFFF, _au32AdlRegBassAddr_L_Mask[enClientTmp]);
    W2BYTEMSK(_au32AdlRegBassAddr_H[enClientTmp], ((_stClientInfo[enClientTmp].phyBaseAddr / _au32AdlMemBaseUnit) >> 16 & 0x0000FFFF), _au32AdlRegBassAddr_H_Mask[enClientTmp]);
    //set client d work mode
    _KHal_XC_Set_Auto_Download_WorkMode(_au32AdlRegWorkMode[E_PNL_XC_AUTODOWNLOAD_CLIENT_PCID], _au32AdlRegWorkModeMask[E_PNL_XC_AUTODOWNLOAD_CLIENT_PCID], enMode);
    // -------PCID client config end------------

    // -------VAC client config start------------
    enClientTmp = E_PNL_XC_AUTODOWNLOAD_CLIENT_VAC;
    _KHal_XC_PA2VA(enClientTmp);
    _stClientInfo[enClientTmp].bEnable = bEnable;

    //set clientL baseaddr
    W2BYTEMSK(_au32AdlRegClkGate[enClientTmp], 1, _au32AdlRegClkGateMask[enClientTmp]);
    W2BYTEMSK(_au32AdlRegBassAddr_L[enClientTmp], (_stClientInfo[enClientTmp].phyBaseAddr / _au32AdlMemBaseUnit) & 0x0000FFFF, _au32AdlRegBassAddr_L_Mask[enClientTmp]);
    W2BYTEMSK(_au32AdlRegBassAddr_H[enClientTmp], ((_stClientInfo[enClientTmp].phyBaseAddr / _au32AdlMemBaseUnit) >> 16 & 0x0000FFFF), _au32AdlRegBassAddr_H_Mask[enClientTmp]);

    //set client d work mode
    _KHal_XC_Set_Auto_Download_WorkMode(_au32AdlRegWorkMode[E_PNL_XC_AUTODOWNLOAD_CLIENT_VAC], _au32AdlRegWorkModeMask[E_PNL_XC_AUTODOWNLOAD_CLIENT_VAC], enMode);
    // -------VAC client config end------------

    UBOOT_DEBUG("adl pga phyBaseAddr  = 0x%llx\n", _stClientInfo[E_PNL_XC_AUTODOWNLOAD_CLIENT_PANEL_GAMMA].phyBaseAddr);
    UBOOT_DEBUG("adl od phyBaseAddr   = 0x%llx\n", _stClientInfo[E_PNL_XC_AUTODOWNLOAD_CLIENT_OD].phyBaseAddr);
    UBOOT_DEBUG("adl pcid phyBaseAddr = 0x%llx\n", _stClientInfo[E_PNL_XC_AUTODOWNLOAD_CLIENT_PCID].phyBaseAddr);
    UBOOT_DEBUG("adl vac phyBaseAddr  = 0x%llx\n", _stClientInfo[E_PNL_XC_AUTODOWNLOAD_CLIENT_VAC].phyBaseAddr);

    //set fake active
    for (u16Idx = 0; u16Idx < E_PNL_XC_AUTODOWNLOAD_CLIENT_MAX; u16Idx++)
    {
        W2BYTEMSK(
                _au32AdlRegFakeActiveEn[u16Idx],
                _stClientInfo[u16Idx].bFakeSupport,
                _au32AdlRegFakeActiveEnMask[u16Idx]);
		UBOOT_DEBUG("ADL Client[%d] Fake support=%d\n", u16Idx, _stClientInfo[u16Idx].bFakeSupport);
    }

    return TRUE;
}

bool KHal_XC_WriteAutoDownload(EN_PNL_XC_AUTODOWNLOAD_CLIENT enClient, uint8_t* pu8Data, uint32_t u32Size, void* pParam)
{
    uint32_t u32StartAddr = 0;
    u64 busBaseAddr;
    bool bRet = FALSE;
    if ((CHECK_CLIENT_RANGE_VALID(enClient) == FALSE) || (CHECK_ADL_WIDTH_VALID() == FALSE))
    {
		UBOOT_ERROR("Invalid input parameter!\n");
        return FALSE;
    }

    u32StartAddr = _stClientInfo[enClient].u32StartAddr;

    if (_stClientInfo[enClient].bEnable == FALSE)
    {
        UBOOT_ERROR("[%s][%d] Auto downlaod client[%d] is disabled, please enable first.\n",__func__, __LINE__, enClient);
        return FALSE;
    }

    if(pu8Data == NULL)
    {
        UBOOT_ERROR("[%s][%d] pu8Data is NULL\n",__func__, __LINE__);
        return FALSE;
    }

    if(_stClientInfo[enClient].busBaseAddr == 0)
    {
        UBOOT_ERROR("[%s][%d] _stClientInfo[enClient].pu8BaseAdd is NULL.\n",__func__, __LINE__);
        return FALSE;
    }

    busBaseAddr = _stClientInfo[enClient].busBaseAddr;
    if (u32StartAddr != 0xFFFFFFFF)
    {
		busBaseAddr += (u64)_au32AdlMemBaseUnit * (u64)u32StartAddr;
    }

    if (_stClientInfo[enClient].enMode == E_PNL_XC_AUTODOWNLOAD_TRIGGER_MODE)
    {
        if( FALSE == _KHal_XC_IsAutoDownload_Available(enClient, (uint8_t *)busBaseAddr))
        {
			UBOOT_ERROR("ADL not available client:%d\n", enClient);
            return FALSE;
        }
    }

    _ADL_SPIN_LOCK(enClient);

    switch(enClient)
    {
        case E_PNL_XC_AUTODOWNLOAD_CLIENT_PANEL_GAMMA:
        {
            uint32_t u32MaxSize = AUTODOWNLOAD_CLIENT_PANELGAMMA_MEM_SIZE;

            if (u32Size > u32MaxSize)
            {
				UBOOT_ERROR("Write auto download fail, invaild parameters, enClient: %d, size: %d\n", enClient, u32Size);
                break;
            }

            memcpy((void *)busBaseAddr, pu8Data, u32Size);
            flush_dcache_range(busBaseAddr,busBaseAddr+u32Size);

			_stClientInfo[enClient].u32Depth = u32Size/_au32AdlBusWidth;

            bRet = TRUE;
            break;
        }
        case E_PNL_XC_AUTODOWNLOAD_CLIENT_OD:
        {
            uint32_t u32MaxSize = AUTODOWNLOAD_CLIENT_OVERDRIVER_MEM_SIZE;

            if (u32Size > u32MaxSize)
            {
				UBOOT_ERROR("Write auto download fail, invaild parameters, enClient: %d, size: %d\n", enClient, u32Size);
                break;
            }

            memcpy((void *)busBaseAddr, pu8Data, u32Size);
			flush_dcache_range(busBaseAddr, busBaseAddr+u32Size);

            _stClientInfo[enClient].u32Depth = XC_AUTODOWNLOAD_OVERDRIVER_DEPTH;
            bRet = TRUE;
            break;
        }
        case E_PNL_XC_AUTODOWNLOAD_CLIENT_PCID:
        {
            uint32_t u32MaxSize = AUTODOWNLOAD_CLIENT_PCID_MEM_SIZE;

            if (u32Size > u32MaxSize)
            {
				UBOOT_ERROR("Write auto download fail, invaild parameters, enClient: %d, size: %d\n", enClient, u32Size);
                break;
            }

            memcpy((void *)busBaseAddr, pu8Data, u32Size);
            flush_dcache_range(busBaseAddr,busBaseAddr+u32Size);

			_stClientInfo[enClient].u32Depth = u32Size/_au32AdlBusWidth;
            bRet = TRUE;
            break;
        }
        case E_PNL_XC_AUTODOWNLOAD_CLIENT_VAC:
        {
            uint32_t u32MaxSize = AUTODOWNLOAD_CLIENT_VAC_MEM_SIZE;

            if (u32Size > u32MaxSize)
            {
				UBOOT_ERROR("Write auto download fail, invaild parameters, enClient: %d, size: %d\n", enClient, u32Size);
                break;
            }

            memcpy((void *)busBaseAddr, pu8Data, u32Size);
            flush_dcache_range(busBaseAddr,busBaseAddr+u32Size);

			_stClientInfo[enClient].u32Depth = u32Size/_au32AdlBusWidth;
            bRet = TRUE;
            break;
        }
        default:
        {
            break;
        }

    }

    _ADL_SPIN_UNLOCK(enClient);
    return bRet;
}

bool KHal_XC_FireAutoDownload(EN_PNL_XC_AUTODOWNLOAD_CLIENT enClient)
{
    if ((CHECK_CLIENT_RANGE_VALID(enClient) == FALSE) || (CHECK_ADL_WIDTH_VALID() == FALSE))
    {
		UBOOT_ERROR("Invalid input parameter!\n");
        return FALSE;
    }

    _ADL_SPIN_LOCK(enClient);

    if (_stClientInfo[enClient].bEnable == FALSE)
    {
        _ADL_SPIN_UNLOCK(enClient);
		UBOOT_ERROR("Auto downlaod client[%d] is disabled, please enable first.\n", enClient);
        return FALSE;
    }

    if (_stClientInfo[enClient].u32Depth == 0)
    {
        _ADL_SPIN_UNLOCK(enClient);
		UBOOT_ERROR("Auto downlaod client[%d] depth = 0, please write first.\n", enClient);
        return FALSE;
    }

    switch(enClient)
    {
        case E_PNL_XC_AUTODOWNLOAD_CLIENT_PANEL_GAMMA:
        case E_PNL_XC_AUTODOWNLOAD_CLIENT_OD:
        case E_PNL_XC_AUTODOWNLOAD_CLIENT_PCID:
        case E_PNL_XC_AUTODOWNLOAD_CLIENT_VAC:
        {
            // set baseaddr
			W2BYTEMSK(_au32AdlRegBassAddr_L[enClient], (_stClientInfo[enClient].phyBaseAddr / _au32AdlMemBaseUnit) & 0x0000FFFF, _au32AdlRegBassAddr_L_Mask[enClient]);
			W2BYTEMSK(_au32AdlRegBassAddr_H[enClient], ((_stClientInfo[enClient].phyBaseAddr / _au32AdlMemBaseUnit) >> 16 & 0x0000FFFF), _au32AdlRegBassAddr_H_Mask[enClient]);

            //reg depth
            W2BYTEMSK(_au32AdlRegDepth[enClient], min(_stClientInfo[enClient].u32Depth, (uint32_t)0xFFFF),Fld(16,0,AC_FULLW10));

            //reg req_len
            W2BYTEMSK(_au32AdlRegReqLength[enClient], min(_stClientInfo[enClient].u32Depth, (uint32_t)AUTODOWNLOAD_REQ_LEN),Fld(16,0,AC_FULLW10));

            // enable auto download
            W2BYTEMSK(_au32AdlRegEnable[enClient], 1, _au32AdlRegEnableMask[enClient]);
            break;
        }
        default:
        {
            _ADL_SPIN_UNLOCK(enClient);
            return FALSE;
        }
    }

    _stClientInfo[enClient].enStatus = E_PNL_XC_AUTO_DOWNLOAD_FIRED;
    _ADL_SPIN_UNLOCK(enClient);

    return TRUE;
}
#endif

bool mtk_pnl_autodownload_init(struct udevice *dev)
{
    //get adl mmap
    static bool bInited = FALSE;
    bool bRet = FALSE;
    uint64_t phyBaseAddr, u64Offset = 0;
    uint32_t u32Size;
    uint32_t u32MiuNo;
    struct dts_mmap mmap;
    int ret;

    if (bInited == TRUE)
    {
        return TRUE;
    }
    memset(&mmap, 0, sizeof(mmap));
    ret = parse_dt("/mmap_info/MI_DISP_AUTO_DOWNLOAD", mmap_dt_parser, (void *)&mmap, "reg");
    if (ret < 0 || mmap.address == 0)
    {
        UBOOT_ERROR("Error: Get MI_DISP_AUTO_DOWNLOAD buffer from DTS mmap failure\n");
        return FALSE;
    }
    else
    {
		UBOOT_DEBUG("ADL mmap address=0x%llx, size=0x%llx\n", mmap.address, mmap.size);
    }
#ifdef UFO_XC_AUTO_DOWNLOAD
    bRet = KHal_XC_InitAutoDownload(dev);
    u32MiuNo = 0;
    bRet &= KHal_XC_GetMiuOffset(u32MiuNo, &u64Offset);
    phyBaseAddr = mmap.address - u64Offset;
    u32Size = (uint32_t)mmap.size;
    bRet &= KHal_XC_ConfigAutoDownload(E_PNL_XC_AUTODOWNLOAD_CLIENT_PANEL_GAMMA, TRUE, E_PNL_XC_AUTODOWNLOAD_TRIGGER_MODE, phyBaseAddr, u32Size,u32MiuNo);
#endif
    bInited = TRUE;
    return bRet;
}
