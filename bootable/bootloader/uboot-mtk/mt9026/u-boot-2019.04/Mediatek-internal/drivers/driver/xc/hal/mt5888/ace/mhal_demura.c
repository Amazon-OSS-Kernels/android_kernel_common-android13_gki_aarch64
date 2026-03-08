#define  MHAL_DEMURA_C

#if !(defined(CONFIG_UTOPIAXP_REMOVE_IMPL))
// Common Definition
#include "MsCommon.h"
#include "MsOS.h"

// Internal Definition
#include "apiXC_Ace.h"
#include "mhal_ace.h"
#include "drvDemura.h"
#include "mhal_demura.h"
#include "demura_hwreg_utility2.h"
#include "drvACE.h"
#include "hwreg_ace.h"
#ifdef MSOS_TYPE_LINUX_KERNEL
#include <linux/kernel.h>
#include <linux/string.h>
#else
#include <string.h>
#endif
#ifdef MSOS_TYPE_LINUX
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h> // O_RDWR
#endif
#include "UFO.h"
#include "mdrv_xc_io.h"
//#include "drvDemura.h"
//#include "mhal_demura.h"
//-------------------------------------------------------------------------------------------------
//  Local Structures
//-------------------------------------------------------------------------------------------------
#include "ACE_private.h"
#include "ACE_context.h"

#include "utopia_dapi.h"
#include "utopia.h"
#endif

MS_VIRT _DEMURA_RIU_BASE;

// Put this function here because hwreg_utility2 only for hal.
void Hal_DEMURA_init_riu_base(MS_VIRT ptr_riu_base)
{
    _DEMURA_RIU_BASE = ptr_riu_base;
}

void Hal_DEMURA_Set_Device_Bank_Offset(void)
{
    memset(u32XCDeviceBankOffset, 0, sizeof(MS_U32)*XC_ACE_MAX_DEVICE_NUM);
    u32XCDeviceBankOffset[XC_ACE_DEVICE0] = E_HALACE_DEVICE0_XC_BANK_OFFSET; // Set SC0 reg bank offset
}

MS_U32 crc32_table[256]=
{
    0x00000000, 0x04C11DB7, 0x09823B6E, 0x0D4326D9,
    0x130476DC, 0x17C56B6B, 0x1A864DB2, 0x1E475005,
    0x2608EDB8, 0x22C9F00F, 0x2F8AD6D6, 0x2B4BCB61,
    0x350C9B64, 0x31CD86D3, 0x3C8EA00A, 0x384FBDBD,
    0x4C11DB70, 0x48D0C6C7, 0x4593E01E, 0x4152FDA9,
    0x5F15ADAC, 0x5BD4B01B, 0x569796C2, 0x52568B75,
    0x6A1936C8, 0x6ED82B7F, 0x639B0DA6, 0x675A1011,
    0x791D4014, 0x7DDC5DA3, 0x709F7B7A, 0x745E66CD,
    0x9823B6E0, 0x9CE2AB57, 0x91A18D8E, 0x95609039,
    0x8B27C03C, 0x8FE6DD8B, 0x82A5FB52, 0x8664E6E5,
    0xBE2B5B58, 0xBAEA46EF, 0xB7A96036, 0xB3687D81,
    0xAD2F2D84, 0xA9EE3033, 0xA4AD16EA, 0xA06C0B5D,
    0xD4326D90, 0xD0F37027, 0xDDB056FE, 0xD9714B49,
    0xC7361B4C, 0xC3F706FB, 0xCEB42022, 0xCA753D95,
    0xF23A8028, 0xF6FB9D9F, 0xFBB8BB46, 0xFF79A6F1,
    0xE13EF6F4, 0xE5FFEB43, 0xE8BCCD9A, 0xEC7DD02D,
    0x34867077, 0x30476DC0, 0x3D044B19, 0x39C556AE,
    0x278206AB, 0x23431B1C, 0x2E003DC5, 0x2AC12072,
    0x128E9DCF, 0x164F8078, 0x1B0CA6A1, 0x1FCDBB16,
    0x018AEB13, 0x054BF6A4, 0x0808D07D, 0x0CC9CDCA,
    0x7897AB07, 0x7C56B6B0, 0x71159069, 0x75D48DDE,
    0x6B93DDDB, 0x6F52C06C, 0x6211E6B5, 0x66D0FB02,
    0x5E9F46BF, 0x5A5E5B08, 0x571D7DD1, 0x53DC6066,
    0x4D9B3063, 0x495A2DD4, 0x44190B0D, 0x40D816BA,
    0xACA5C697, 0xA864DB20, 0xA527FDF9, 0xA1E6E04E,
    0xBFA1B04B, 0xBB60ADFC, 0xB6238B25, 0xB2E29692,
    0x8AAD2B2F, 0x8E6C3698, 0x832F1041, 0x87EE0DF6,
    0x99A95DF3, 0x9D684044, 0x902B669D, 0x94EA7B2A,
    0xE0B41DE7, 0xE4750050, 0xE9362689, 0xEDF73B3E,
    0xF3B06B3B, 0xF771768C, 0xFA325055, 0xFEF34DE2,
    0xC6BCF05F, 0xC27DEDE8, 0xCF3ECB31, 0xCBFFD686,
    0xD5B88683, 0xD1799B34, 0xDC3ABDED, 0xD8FBA05A,
    0x690CE0EE, 0x6DCDFD59, 0x608EDB80, 0x644FC637,
    0x7A089632, 0x7EC98B85, 0x738AAD5C, 0x774BB0EB,
    0x4F040D56, 0x4BC510E1, 0x46863638, 0x42472B8F,
    0x5C007B8A, 0x58C1663D, 0x558240E4, 0x51435D53,
    0x251D3B9E, 0x21DC2629, 0x2C9F00F0, 0x285E1D47,
    0x36194D42, 0x32D850F5, 0x3F9B762C, 0x3B5A6B9B,
    0x0315D626, 0x07D4CB91, 0x0A97ED48, 0x0E56F0FF,
    0x1011A0FA, 0x14D0BD4D, 0x19939B94, 0x1D528623,
    0xF12F560E, 0xF5EE4BB9, 0xF8AD6D60, 0xFC6C70D7,
    0xE22B20D2, 0xE6EA3D65, 0xEBA91BBC, 0xEF68060B,
    0xD727BBB6, 0xD3E6A601, 0xDEA580D8, 0xDA649D6F,
    0xC423CD6A, 0xC0E2D0DD, 0xCDA1F604, 0xC960EBB3,
    0xBD3E8D7E, 0xB9FF90C9, 0xB4BCB610, 0xB07DABA7,
    0xAE3AFBA2, 0xAAFBE615, 0xA7B8C0CC, 0xA379DD7B,
    0x9B3660C6, 0x9FF77D71, 0x92B45BA8, 0x9675461F,
    0x8832161A, 0x8CF30BAD, 0x81B02D74, 0x857130C3,
    0x5D8A9099, 0x594B8D2E, 0x5408ABF7, 0x50C9B640,
    0x4E8EE645, 0x4A4FFBF2, 0x470CDD2B, 0x43CDC09C,
    0x7B827D21, 0x7F436096, 0x7200464F, 0x76C15BF8,
    0x68860BFD, 0x6C47164A, 0x61043093, 0x65C52D24,
    0x119B4BE9, 0x155A565E, 0x18197087, 0x1CD86D30,
    0x029F3D35, 0x065E2082, 0x0B1D065B, 0x0FDC1BEC,
    0x3793A651, 0x3352BBE6, 0x3E119D3F, 0x3AD08088,
    0x2497D08D, 0x2056CD3A, 0x2D15EBE3, 0x29D4F654,
    0xC5A92679, 0xC1683BCE, 0xCC2B1D17, 0xC8EA00A0,
    0xD6AD50A5, 0xD26C4D12, 0xDF2F6BCB, 0xDBEE767C,
    0xE3A1CBC1, 0xE760D676, 0xEA23F0AF, 0xEEE2ED18,
    0xF0A5BD1D, 0xF464A0AA, 0xF9278673, 0xFDE69BC4,
    0x89B8FD09, 0x8D79E0BE, 0x803AC667, 0x84FBDBD0,
    0x9ABC8BD5, 0x9E7D9662, 0x933EB0BB, 0x97FFAD0C,
    0xAFB010B1, 0xAB710D06, 0xA6322BDF, 0xA2F33668,
    0xBCB4666D, 0xB8757BDA, 0xB5365D03, 0xB1F740B4
};
#define WIDTH       (8 * sizeof(MS_U32))

MS_U32 HAL_DEMURA_CRC32_Cal_Seed(MS_U8* pdat, MS_U32 dat_len, MS_U32 u32Seed)
{
    MS_U32 u32DataIdx;
    MS_U8 u8Data;
    MS_U32 remainder = u32Seed;

    for ( u32DataIdx = 0; u32DataIdx < dat_len; ++u32DataIdx)
    {
        u8Data = pdat[u32DataIdx] ^ (remainder >> (WIDTH - 8));
        remainder = crc32_table[u8Data] ^ (remainder << 8);
    }

    return (remainder);
}

MS_U32 HAL_DEMURA_SW_CRC32(MS_U8* pdat, MS_U32 dat_len)
{
    return HAL_DEMURA_CRC32_Cal_Seed(pdat, dat_len, 0xFFFFFFFF);
}

void HAL_DEMURA_RegisterTableLoad(MS_U8 high_byte, MS_U32 u32RegisterAddress, MS_U8 u8Value, MS_U8 u8Mask)
{
    if((high_byte & _BIT0) == _BIT0) // high byte
        SC_W2BYTEMSK(0, u32RegisterAddress-1, ((MS_U16)u8Value<<8), ((MS_U16)u8Mask<<8));
    else
        SC_W2BYTEMSK(0, u32RegisterAddress, u8Value, u8Mask);
}

MS_U16 HAL_DEMURA_Get_PanelWidth(DeMuraBinHeader *pHeader)
{
    MS_U16 panel_width;
    MS_U16 packet_notes;

    if (pHeader->nPacketSize > 1)
    {
        //printf("Do not support nPacketSize = %d\n", pHeader->nPacketSize);
        return 0;
    }

    if (pHeader->u4EndLayer > 4)
    {
        packet_notes = 1;
        if (pHeader->nPacketSize == 0)
        {
            //printf("This PacketSize(%d) can not contain Layers(%d)\n",
            //DEMURA_PACKET_SIZE(pHeader->nPacketSize), pHeader->u4EndLayer - pHeader->u4StartLayer);
            return 0;
        }
    }
    else if (pHeader->u4EndLayer > 2)
    {
        packet_notes = (2 >> (1 - pHeader->nPacketSize));
    }
    else
    {
        packet_notes = (4 >> (1 - pHeader->nPacketSize));
    }

    panel_width = packet_notes * (1 << (pHeader->nHBlockSize)) * (pHeader->u16HNodeCount - 1);
    if (pHeader->bSeperateRGB == 0)
    {
        // Mono mode: R/G/B channel used the same table
        panel_width = panel_width * 4;
    }

    //printf("The panel_width in LUT is %d\n", panel_width);
    return panel_width;
}


MS_U16 HAL_DEMURA_Get_PanelHeight(DeMuraBinHeader *pHeader)
{
    MS_U16 panel_height;
    panel_height = (1 << (pHeader->nVBlockSize)) * (pHeader->u16VNodeCount - 1);
    //printf("The panel_height in LUT is %d\n", panel_height);
    return panel_height;
}


void HAL_DEMURA_SetDL_BaseAddr(MS_U32 u32Addr)
{
    MS_U32 base_addr = u32Addr / DEMURA_DMA_ADDR_UNIT;

    SC_W2BYTE(0, REG_SC_BK67_06_L, (base_addr&0xFFFF));
    SC_W2BYTE(0, REG_SC_BK67_07_L, ((base_addr >> 16)&0x07FF));
}


void HAL_DEMURA_SetDL_SramIni_Addr(MS_U16 offset)
{
    SC_W2BYTE(0, REG_SC_BK67_10_L, offset);
}


void HAL_DEMURA_SetDL_Depth(MS_U16 HNodeNum)
{
    SC_W2BYTE(0, REG_SC_BK67_0A_L, HNodeNum);
}


void HAL_DEMURA_SetDL_ReqLenth(MS_U16 ReqLenth)
{
    SC_W2BYTE(0, REG_SC_BK67_0D_L, ReqLenth);
}

void HAL_DEMURA_SetDL_TriggerMode(AUTODOWNLOAD_TRIG_e eTrigMode)
{
    SC_W2BYTEMSK(0, REG_SC_BK67_01_L, (eTrigMode << 5), _BIT5);

}

void HAL_DEMURA_Enable_DL(MS_BOOL bEnable)
{
    SC_W2BYTEMSK(0, REG_SC_BK67_01_L, (bEnable << 2), _BIT2);
}

void HAL_DEMURA_EnableDemura(MS_BOOL bEnable, DEMURA_PANEL_TYPE ptype)
{
    MS_U16 reg_val  = 0;
    MS_U16 reg_mask = 0;
    MS_U8  reg_sram = 0;

    reg_mask = (_BIT6 | _BIT2|_BIT1|_BIT0);

    switch (ptype)
    {
        case E_DEMURA_RGB_PANEL:
            reg_val  = (_BIT2|_BIT1|_BIT0);
            break;
        case E_DEMURA_RGBW_PANEL:
            reg_val = reg_mask;
            break;
        default:
            break;
    }

    if (bEnable == TRUE)
    {
        SC_W2BYTEMSK(0, REG_SC_BK77_2F_L, reg_val, reg_mask);
    }
    else
    {
        SC_W2BYTEMSK(0, REG_SC_BK77_2F_L, 0x00, reg_mask);
    }
    
    // Add for demura sram enable
    reg_sram = MDrv_ReadByte(0x1133A8);
    reg_sram = reg_sram | _BIT2;
    MDrv_WriteByte(0x1133A8, reg_sram);
}


void HAL_DEMURA_Dump_Layerlevel(void)
{
    MS_U32 i = 0;
    MS_U32 layer_num = 8;
    MS_U32 layer_reg_start = 0x11;

    //printf("Layer level(8 bit): ");
    for (i = 0; i < layer_num; i++)
    {
        MS_U32 addr = layer_reg_start + i;
        MS_U32 val  = SC_R2BYTEMSK(0, _PK_L_(0x77, addr), 0x0FFF);
        //printf("%d ", (int)(val >> 4));  // 12bit --> 8bit
    }
    //printf("\n");
}


/******************** Convert Part ********************/

MS_U32 HAL_DEMURA_Cal_LutSize(MS_U32 levelCnt, MS_U32 hNode, MS_U32 vNode, MS_U32 sepType)
{
    int  LevelCount;
    int  cal_packet_size;    // unit : bit
    int  packet_size;        // unit : bit
    MS_U32 layer_dat_size;   // unit : byte

    switch (levelCnt)
    {
        case 8:
        case 7:
        case 6:
        case 5:
            LevelCount = 8;
            break;
        case 4:
        case 3:
            LevelCount = 4;
            break;
        case 2:
        case 1:
            LevelCount = 2;
            break;
        default:
            LevelCount = 8;
    }

    if (sepType == 1)
    {
        cal_packet_size = LevelCount * 3 * 8;
    }
    else
    {
        cal_packet_size = LevelCount * 8;
    }

    if (cal_packet_size > 128)
    {
        packet_size = 256;
    }
    else
    {
        packet_size = 128;
    }

    layer_dat_size = hNode * vNode * (packet_size / 8);
    return layer_dat_size;
}


MS_U32 HAL_DEMURA_Max_RegSize(void)
{
   return (256 * 5);   // unit : byte
}


MS_U32 HAL_DEMURA_Max_RegCount(void)
{
   return   256;   // unit : byte
}

MS_U32 HAL_DEMURA_Get_PacketType(void)
{
    // 1 : 256bit
    return 1;
}


DEMURA_PANEL_TYPE HAL_DEMURA_Get_PanelType(void)
{
    DEMURA_PANEL_TYPE pret;

    if (((MDrv_Read2Byte(REG_HK_BASE + REG_CHPI_CLKEN)&_BIT6) == _BIT6)
         && ((MDrv_Read2Byte(REG_HK_BASE + REG_CHPI_BYPASS)&_BIT0) == 0))
    {
        // Interface CHPI
        pret = E_DEMURA_RGBW_PANEL;
    }
    else if (((MDrv_Read2Byte(REG_HK_BASE + REG_CEDS_CLKEN)&_BIT7) == _BIT7)
         && ((MDrv_Read2Byte(REG_HK_BASE + REG_CEDS_BYPASS)&_BIT0) == 0))
    {
        // Interface CEDS
        pret = E_DEMURA_RGBW_PANEL;
    }
    else if (((MDrv_Read2Byte(REG_HK_BASE + REG_EPI_CLKEN)&_BIT7) == _BIT7)
         && ((MDrv_Read2Byte(REG_HK_BASE + REG_EPI_BYPASS)&_BIT5) == 0))
    {
        // Interface EPI
        pret = E_DEMURA_RGBW_PANEL;
    }
    else
    {
        pret = E_DEMURA_RGB_PANEL;
    }

    return pret;
}

ST_DEMURA_SUSPEND_INFO HAL_DEMURA_GET_SUSPEND(void)
{
    ST_DEMURA_SUSPEND_INFO suspend_info;
	MS_U8 u8Count = 0;
    MS_U16 addr;
    MS_U32 phy_addr;

    // u16ADL_EN
    addr = 0x01;
    suspend_info.u16ADL_EN= SC_R2BYTEMSK(0, _PK_L_(0x67, addr), 0xFFFF);

    // lut_start_addr
    addr = 0x07;
    phy_addr = SC_R2BYTEMSK(0, _PK_L_(0x67, addr), 0x07FF);
    addr = 0x06;
    phy_addr = (phy_addr<<16) | SC_R2BYTEMSK(0, _PK_L_(0x67, addr), 0xFFFF);
    suspend_info.lut_start_addr = phy_addr;

    // u16HNodeCount
    addr = 0x0A;
    suspend_info.u16HNodeCount = SC_R2BYTEMSK(0, _PK_L_(0x67, addr), 0xFFFF);

    // u16ReqLength
    addr = 0x0D;
    suspend_info.u16ReqLength = SC_R2BYTEMSK(0, _PK_L_(0x67, addr), 0xFFFF);

    // Reg 0x10~0x4F
    for(u8Count = 0; u8Count < 0x40; u8Count++)
    {
        suspend_info.reg[u8Count].u16Addr = 0x10+u8Count;
        addr = suspend_info.reg[u8Count].u16Addr;
        suspend_info.reg[u8Count].u16Value = SC_R2BYTEMSK(0, _PK_L_(0x77, addr), 0xFFFF);
    }
	// Reg 0x60~0x6A
	for(u8Count = 0x40; u8Count < 0x4b; u8Count++)
    {
        suspend_info.reg[u8Count].u16Addr = 0x60+(u8Count-0x40);
        addr = suspend_info.reg[u8Count].u16Addr;
        suspend_info.reg[u8Count].u16Value = SC_R2BYTEMSK(0, _PK_L_(0x77, addr), 0xFFFF);
    }

    return suspend_info;
}

void HAL_DEMURA_GET_RESUME(ST_DEMURA_SUSPEND_INFO Suspend_Info)
{
	MS_U8 u8Count = 0;
    MS_U16 addr;

    // ADL
    addr = 0x01;
    SC_W2BYTEMSK(0, _PK_L_(0x67, addr), Suspend_Info.u16ADL_EN, 0xFFFF);

    // lut_start_addr
    addr = 0x06;
    SC_W2BYTEMSK(0, _PK_L_(0x67, addr), (Suspend_Info.lut_start_addr&0xFFFF), 0xFFFF);
    addr = 0x07;
    SC_W2BYTEMSK(0, _PK_L_(0x67, addr), ((Suspend_Info.lut_start_addr>>16)&0x7FFF), 0x7FFF);

    // u16HNodeCount
    addr = 0x0A;
    SC_W2BYTEMSK(0, _PK_L_(0x67, addr), Suspend_Info.u16HNodeCount, 0xFFFF);

    // u16ReqLength
    addr = 0x0D;
    SC_W2BYTEMSK(0, _PK_L_(0x67, addr), Suspend_Info.u16ReqLength, 0xFFFF);

    // Reg 0x10~0x4F Reg 0x60~0x6A
    for(u8Count = 0; u8Count < 0x4b; u8Count++)
    {
        addr = Suspend_Info.reg[u8Count].u16Addr;
        SC_W2BYTEMSK(0, _PK_L_(0x77, addr), Suspend_Info.reg[u8Count].u16Value, 0xFFFF);
    }
    
}

DeMuraStatus HAL_DEMURA_Get_Reg_Status(void)
{
    MS_U16 addr, mask;
    DeMuraStatus demura_status;

    addr = 0x10;
    mask = 0x0fff;
    demura_status.u16BlackLimit = SC_R2BYTEMSK(0, _PK_L_(0x77, addr), mask);
    addr = 0x11;
    mask = 0x0fff;
    demura_status.u16PlaneLevel1 = SC_R2BYTEMSK(0, _PK_L_(0x77, addr), mask);
    addr = 0x12;
    mask = 0x0fff;
    demura_status.u16PlaneLevel2 = SC_R2BYTEMSK(0, _PK_L_(0x77, addr), mask);
    addr = 0x13;
    mask = 0x0fff;
    demura_status.u16PlaneLevel3 = SC_R2BYTEMSK(0, _PK_L_(0x77, addr), mask);
    addr = 0x14;
    mask = 0x0fff;
    demura_status.u16PlaneLevel4 = SC_R2BYTEMSK(0, _PK_L_(0x77, addr), mask);
    addr = 0x15;
    mask = 0x0fff;
    demura_status.u16PlaneLevel5 = SC_R2BYTEMSK(0, _PK_L_(0x77, addr), mask);
    addr = 0x16;
    mask = 0x0fff;
    demura_status.u16PlaneLevel6 = SC_R2BYTEMSK(0, _PK_L_(0x77, addr), mask);
    addr = 0x17;
    mask = 0x0fff;
    demura_status.u16PlaneLevel7 = SC_R2BYTEMSK(0, _PK_L_(0x77, addr), mask);
    addr = 0x18;
    mask = 0x0fff;
    demura_status.u16PlaneLevel8 = SC_R2BYTEMSK(0, _PK_L_(0x77, addr), mask);
    addr = 0x19;
    mask = 0x0fff;
    demura_status.u16WhiteLimit = SC_R2BYTEMSK(0, _PK_L_(0x77, addr), mask);

    addr = 0x2f;
    mask = _BIT0;
    demura_status.bB_ch_Enable = SC_R2BYTEMSK(0, _PK_L_(0x77, addr), mask);
    addr = 0x2f;
    mask = _BIT1;
    demura_status.bG_ch_Enable = SC_R2BYTEMSK(0, _PK_L_(0x77, addr), mask)>>1;
    addr = 0x2f;
    mask = _BIT2;
    demura_status.bR_ch_Enable = SC_R2BYTEMSK(0, _PK_L_(0x77, addr), mask)>>2;
    addr = 0x2f;
    mask = _BIT6;
    demura_status.bW_ch_Enable = SC_R2BYTEMSK(0, _PK_L_(0x77, addr), mask)>>6;
    addr = 0x2a;
    mask = _BIT7;
    demura_status.bSeperateRGB = SC_R2BYTEMSK(0, _PK_L_(0x77, addr), mask)>>7;
    addr = 0x2a;
    mask = 0x000f;
    demura_status.u8PlaneNum = SC_R2BYTEMSK(0, _PK_L_(0x77, addr), mask);

    addr = 0x2a;
    mask = 0x7000;
    demura_status.nHBlockSize = SC_R2BYTEMSK(0, _PK_L_(0x77, addr), mask)>>12;
    addr = 0x2a;
    mask = 0x0700;
    demura_status.nVBlockSize = SC_R2BYTEMSK(0, _PK_L_(0x77, addr), mask)>>8;

    addr = 0x2b;
    mask = 0x07ff;
    demura_status.u16HNodeCount = SC_R2BYTEMSK(0, _PK_L_(0x77, addr), mask);
    addr = 0x2c;
    mask = 0x07ff;
    demura_status.u16VNodeCount = SC_R2BYTEMSK(0, _PK_L_(0x77, addr), mask);

    return demura_status;
}
