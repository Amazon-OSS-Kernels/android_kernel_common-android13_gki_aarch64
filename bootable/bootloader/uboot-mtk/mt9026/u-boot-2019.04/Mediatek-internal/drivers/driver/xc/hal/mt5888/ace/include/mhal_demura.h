#ifndef _HAL_DEMURA_H_
#define _HAL_DEMURA_H_

#define _BIT0  0x0001
#define _BIT1  0x0002
#define _BIT2  0x0004
#define _BIT3  0x0008
#define _BIT4  0x0010
#define _BIT5  0x0020
#define _BIT6  0x0040
#define _BIT7  0x0080
#define _BIT8  0x0100
#define _BIT9  0x0200
#define _BIT10 0x0400
#define _BIT11 0x0800
#define _BIT12 0x1000
#define _BIT13 0x2000
#define _BIT14 0x4000
#define _BIT15 0x8000

// Config demura interface type
#define CONFIG_DEMURA_URSA13

#if ((defined CONFIG_DEMURA_M7622) || (defined CONFIG_DEMURA_M7632))
#define CONFIG_DEMURA_RGBW
#else
#define CONFIG_DEMURA_RGB
#endif

#define DEMURA_MAX_LAYER             8
#define MSTAR_DEMURA_DAT_BIT_LEN     10
#define MSTAR_CHIP_ID                0x00E8     // For Merlin

#define REG_DEMURA_BASE              0x037700  // --> Bank_0x1377 (Main 0x102F, Sub 0x77)
#define REG_DEMURA_DMA_BASE          0x036700  // --> Bank_0x1367 (Main 0x102F, Sub 0x67)
#define DEMURA_DMA_ADDR_UNIT         32
#define DEMURA_DMA_REQ_LENGTH        4

//#define REG_DMC_DGA_EN_BIT           BIT7
//#define REG_DMC_W_EN_BIT             BIT6


#define BK_REG_L( x, y )            ((x) | (((y) << 1)))
#define BK_REG_H( x, y )            (((x) | (((y) << 1))) + 1)

// Demura info
#define L_BK_DEMURA(x)               BK_REG_L(REG_DEMURA_BASE, x)
#define H_BK_DEMURA(x)               BK_REG_H(REG_DEMURA_BASE, x)
#define L_BK_DEMURA_DMA(x)           BK_REG_L(REG_DEMURA_DMA_BASE, x)
#define H_BK_DEMURA_DMA(x)           BK_REG_H(REG_DEMURA_DMA_BASE, x)


// RBGW Panel Register info
#define REG_HK_BASE              (0x100000)
#define REG_CHPI_CLKEN           (0x003200 + (0x2F<<1))  // --> 0x1032_0x2f[6] = 1 (16 bit mode)
#define REG_CHPI_BYPASS          (0x232600 + (0x01<<1))  // --> 0x3326_0x01[0] = 0 (16 bit mode)

#define REG_CEDS_CLKEN           (0x003200 + (0x2F<<1))  // --> 0x1032_0x2f[7] = 1 (16 bit mode)
#define REG_CEDS_BYPASS          (0x232600 + (0x01<<1))  // --> 0x3326_0x01[0] = 0 (16 bit mode)

#define REG_EPI_CLKEN            (0x003200 + (0x2F<<1))  // --> 0x1032_0x2f[7] = 1 (16 bit mode)
#define REG_EPI_BYPASS           (0x230300 + (0x01<<1))  // --> 0x3303_0x01[5] = 0 (16 bit mode)

void Hal_DEMURA_init_riu_base(MS_VIRT ptr_riu_base);
void Hal_DEMURA_Set_Device_Bank_Offset(void);
void HAL_DEMURA_RegisterTableLoad(MS_U8 high_byte, MS_U32 u32RegisterAddress, MS_U8 u8Value, MS_U8 u8Mask);
MS_U32 HAL_DEMURA_SW_CRC32(MS_U8* pdat, MS_U32 dat_len);
MS_U16 HAL_DEMURA_Get_PanelWidth(DeMuraBinHeader *pHeader);
MS_U16 HAL_DEMURA_Get_PanelHeight(DeMuraBinHeader *pHeader);

void HAL_DEMURA_SetDL_BaseAddr(MS_U32 u32Addr);
void HAL_DEMURA_SetDL_SramIni_Addr(MS_U16 offset);
void HAL_DEMURA_SetDL_Depth(MS_U16 HNodeNum);
void HAL_DEMURA_SetDL_ReqLenth(MS_U16 ReqLenth);
void HAL_DEMURA_SetDL_TriggerMode(AUTODOWNLOAD_TRIG_e eTrigMode);
void HAL_DEMURA_Enable_DL(MS_BOOL bEnable);
void HAL_DEMURA_EnableDemura(MS_BOOL bEnable, DEMURA_PANEL_TYPE ptype);
void HAL_DEMURA_Dump_Layerlevel(void);

MS_U32 HAL_DEMURA_Cal_LutSize(MS_U32 levelCnt, MS_U32 hNode, MS_U32 vNode, MS_U32 sepType);
MS_U32 HAL_DEMURA_Max_RegSize(void);
MS_U32 HAL_DEMURA_Max_RegCount(void);
MS_U32 HAL_DEMURA_Get_PacketType(void);
DEMURA_PANEL_TYPE HAL_DEMURA_Get_PanelType(void);
ST_DEMURA_SUSPEND_INFO HAL_DEMURA_GET_SUSPEND(void);
void HAL_DEMURA_GET_RESUME(ST_DEMURA_SUSPEND_INFO Suspend_Info);
DeMuraStatus HAL_DEMURA_Get_Reg_Status(void);

///////////////////////////////////////////////////////////////////////////////////
//Demura interface
#define max_(a,b)        ((a>b) ? a : b)
#define min_(a,b)        ((a<b) ? a : b)
#define minmax_(a,b,c)   (max_((min_(a,c)),b))
#define posi_blk(CurV,CurH)     (CurV*block_width+CurH)

#define Get_Demura_Inv             0
#define DEMURA_MAX_LAYER_NUM       8
#define DEMURA_MAX_CHANNEL_NUM     3    // R/G/B


#define sw8Byte(x) \
    ((MS_U64)( \
        (((MS_U64)(x) & (MS_U64)0x00000000000000ff) << 56) | \
        (((MS_U64)(x) & (MS_U64)0x000000000000ff00) << 40) | \
        (((MS_U64)(x) & (MS_U64)0x0000000000ff0000) << 24) | \
        (((MS_U64)(x) & (MS_U64)0x00000000ff000000) << 8) | \
        (((MS_U64)(x) & (MS_U64)0x000000ff00000000) >> 8) | \
        (((MS_U64)(x) & (MS_U64)0x0000ff0000000000) >> 24) | \
        (((MS_U64)(x) & (MS_U64)0x00ff000000000000) >> 40) | \
        (((MS_U64)(x) & (MS_U64)0xff00000000000000) >> 56)))

#define sw5Byte(x) \
    ((MS_U64)( \
        (((MS_U64)(x) & (MS_U64)0x00000000ff) << 32) | \
        (((MS_U64)(x) & (MS_U64)0x000000ff00) << 16) | \
        ((MS_U64)(x) & (MS_U64)0x0000ff0000) | \
        (((MS_U64)(x) & (MS_U64)0x00ff000000) >> 16) | \
        (((MS_U64)(x) & (MS_U64)0xff00000000) >> 32)))

#define sw4Byte(x) \
    ((MS_U32)( \
        (((MS_U32)(x) & (MS_U32)0x000000ff) << 24) | \
        (((MS_U32)(x) & (MS_U32)0x0000ff00) << 8) | \
        (((MS_U32)(x) & (MS_U32)0x00ff0000) >> 8) | \
        (((MS_U32)(x) & (MS_U32)0xff000000) >> 24)))

#define sw2Byte(x) \
    ((MS_U16)( \
        (((MS_U16)(x) & (MS_U16)0x00ff) << 8) | \
        (((MS_U16)(x) & (MS_U16)0xff00) >> 8)))

#define BinVersion           ((0 << 8) + 6)
#define DllVersion           ((0 << 8) + 3)
#define HNodeCount           481
#define VNodeCount           271
#define HNodeCount_U10       241
#define VNodeCount_U10       136
#define MAX_INX_BIN_LENGTH   (512 * 1024) //512k

#define MAX_BIN_LENGTH       (6 *1024 * 1024) // 6MB

#define Aligned_Value        16

#define BlockHSize  8
#define BlockVSize  8

#define CusDataSize 0 //0x100

//----------------------- Chip device ID start -----------------------//
#define ID_CHIP_U9          0x003D  // Ursa  9 (Unicorn)
#define ID_CHIP_U10         0x0090  // Ursa 10 (Uniqlo)
#define ID_CHIP_U11         0x0094  // Ursa 11 (Ultrasonic)
#define ID_CHIP_U12         0x008F  // Ursa 12 (UBike)
#define ID_CHIP_TERRA       0x005F  // Tcon Terra
#define ID_CHIP_MASERATI    0x00A4  // Soc Maserati
//----------------------- Chip device ID end -----------------------//

//----------------------- Customer ID start -----------------------//
#define ID_CUS_MSTAR        0x0000
#define ID_CUS_AUO          0x0100
#define ID_CUS_INL          0x0200
#define ID_CUS_HISENSE      0x0300
#define ID_CUS_BOE          0x1100
#define ID_CUS_PANDA        0x1200
#define ID_CUS_HKC          0x1E00
//----------------------- Customer ID end -----------------------//


typedef struct rgb_struct
{
    int r;
    int g;
    int b;
    int dbr;//double dbr;
    int dbg;//double dbg;
    int dbb;//double dbb;
    #if defined CONFIG_DEMURA_RGBW
    int w;
    int dbw;//double dbw;
    #endif
}strgb_structInfo, *LPrgb_structInfo;

typedef struct layer_info_struct
{
    int dram_h_size;
    int dram_v_size;
    int block_h_size;
    int block_v_size;
    int dmc_plane_num;
    int dmc_rgb_mode;
    int real_dram_h_size;
    int real_dram_v_size;
}stlayer_info_structInfo, *LPlayer_info_structInfo;

typedef struct
{
    int address;
    int mask;
    int value;
    char *description;
}reg_struct;

typedef struct
{
    reg_struct Bit07_00;
}_08bits;

typedef struct
{
    reg_struct Bit07_00;
    reg_struct Bit15_08;
}_16bits;

typedef struct
{
    reg_struct Bit07_00;
    reg_struct Bit15_08;
    reg_struct Bit23_16;
}_24bits;

typedef struct
{
    _16bits reg_dmc_black_limit;          // 12 bit
    _16bits reg_dmc_plane_level1;         // 12 bit
    _16bits reg_dmc_plane_level2;         // 12 bit
    _16bits reg_dmc_plane_level3;         // 12 bit
    _16bits reg_dmc_plane_level4;         // 12 bit
    _16bits reg_dmc_plane_level5;         // 12 bit
    _16bits reg_dmc_plane_level6;         // 12 bit
    _16bits reg_dmc_plane_level7;         // 12 bit
    _16bits reg_dmc_plane_level8;         // 12 bit
    _16bits reg_dmc_white_limit;          // 12 bit
    _08bits reg_dmc_data_r_mag4;          //  3 bit
    _08bits reg_dmc_data_r_mag3;          //  3 bit
    _08bits reg_dmc_data_r_mag2;          //  3 bit
    _08bits reg_dmc_data_r_mag1;          //  3 bit
    _08bits reg_dmc_data_r_mag8;          //  3 bit
    _08bits reg_dmc_data_r_mag7;          //  3 bit
    _08bits reg_dmc_data_r_mag6;          //  3 bit
    _08bits reg_dmc_data_r_mag5;          //  3 bit
    _08bits reg_dmc_data_g_mag4;          //  3 bit
    _08bits reg_dmc_data_g_mag3;          //  3 bit
    _08bits reg_dmc_data_g_mag2;          //  3 bit
    _08bits reg_dmc_data_g_mag1;          //  3 bit
    _08bits reg_dmc_data_g_mag8;          //  3 bit
    _08bits reg_dmc_data_g_mag7;          //  3 bit
    _08bits reg_dmc_data_g_mag6;          //  3 bit
    _08bits reg_dmc_data_g_mag5;          //  3 bit
    _08bits reg_dmc_data_b_mag4;          //  3 bit
    _08bits reg_dmc_data_b_mag3;          //  3 bit
    _08bits reg_dmc_data_b_mag2;          //  3 bit
    _08bits reg_dmc_data_b_mag1;          //  3 bit
    _08bits reg_dmc_data_b_mag8;          //  3 bit
    _08bits reg_dmc_data_b_mag7;          //  3 bit
    _08bits reg_dmc_data_b_mag6;          //  3 bit
    _08bits reg_dmc_data_b_mag5;          //  3 bit
    _16bits reg_dmc_plane_b1_coef;        // 14 bit
    _16bits reg_dmc_plane_12_coef;        // 14 bit
    _16bits reg_dmc_plane_23_coef;        // 14 bit
    _16bits reg_dmc_plane_34_coef;        // 14 bit
    _16bits reg_dmc_plane_45_coef;        // 14 bit
    _16bits reg_dmc_plane_56_coef;        // 14 bit
    _16bits reg_dmc_plane_67_coef;        // 14 bit
    _16bits reg_dmc_plane_78_coef;        // 14 bit
    _16bits reg_dmc_plane_8w_coef;        // 14 bit
    _08bits reg_dmc_plane_8w_ks22;        //  1 bit
    _08bits reg_dmc_plane_78_ks22;        //  1 bit
    _08bits reg_dmc_plane_67_ks22;        //  1 bit
    _08bits reg_dmc_plane_56_ks22;        //  1 bit
    _08bits reg_dmc_plane_45_ks22;        //  1 bit
    _08bits reg_dmc_plane_34_ks22;        //  1 bit
    _08bits reg_dmc_plane_23_ks22;        //  1 bit
    _08bits reg_dmc_plane_12_ks22;        //  1 bit
    _08bits reg_dmc_plane_b1_ks22;        //  1 bit
    _08bits reg_dmc_h_block;              //  3 bit
    _08bits reg_dmc_v_block;              //  3 bit
    _08bits reg_dmc_rgb_mode;             //  1 bit
    _08bits reg_dmc_block_size;           //  2 bit
    _08bits reg_dmc_plane_num;            //  4 bit
    _16bits reg_dmc_h_lut_num;            // 11 bit
    _16bits reg_dmc_v_lut_num;            // 11 bit
    _08bits reg_dmc_flow_ctrl_en;         //  1 bit
    _16bits reg_dmc_flow_len;             // 10 bit
    _08bits reg_dmc_grid_line_en;         //  1 bit
    _08bits reg_dmc_grid_line_size;       //  3 bit
    _08bits reg_dmc_grid_line_color;      //  8 bit
    _08bits reg_dmc_dither_rst_e_4_frame; //  1 bit
    _08bits reg_dmc_dither_pse_rst_num;   //  2 bit
    _08bits reg_dmc_dither_en;            //  2 bit
    _08bits reg_dmc_r_en;                 //  1 bit
    _08bits reg_dmc_g_en;                 //  1 bit
    _08bits reg_dmc_b_en;                 //  1 bit
    _16bits reg_dmc_data_r_offset1;       // 14 bit
    _16bits reg_dmc_data_r_offset2;       // 14 bit
    _16bits reg_dmc_data_r_offset3;       // 14 bit
    _16bits reg_dmc_data_r_offset4;       // 14 bit
    _16bits reg_dmc_data_r_offset5;       // 14 bit
    _16bits reg_dmc_data_r_offset6;       // 14 bit
    _16bits reg_dmc_data_r_offset7;       // 14 bit
    _16bits reg_dmc_data_r_offset8;       // 14 bit
    _16bits reg_dmc_data_g_offset1;       // 14 bit
    _16bits reg_dmc_data_g_offset2;       // 14 bit
    _16bits reg_dmc_data_g_offset3;       // 14 bit
    _16bits reg_dmc_data_g_offset4;       // 14 bit
    _16bits reg_dmc_data_g_offset5;       // 14 bit
    _16bits reg_dmc_data_g_offset6;       // 14 bit
    _16bits reg_dmc_data_g_offset7;       // 14 bit
    _16bits reg_dmc_data_g_offset8;       // 14 bit
    _16bits reg_dmc_data_b_offset1;       // 14 bit
    _16bits reg_dmc_data_b_offset2;       // 14 bit
    _16bits reg_dmc_data_b_offset3;       // 14 bit
    _16bits reg_dmc_data_b_offset4;       // 14 bit
    _16bits reg_dmc_data_b_offset5;       // 14 bit
    _16bits reg_dmc_data_b_offset6;       // 14 bit
    _16bits reg_dmc_data_b_offset7;       // 14 bit
    _16bits reg_dmc_data_b_offset8;       // 14 bit
    _16bits reg_dmc_panel_h_size;         // 13 bit
    _16bits reg_dmc_preload_line;         // 12 bit
    _08bits reg_dmc_mono_mode_q_h_r;      //  6 bit
    _08bits reg_dmc_mono_mode_r_h_r;      //  3 bit
}registers;

typedef struct
{

    int reg_dmc_plane_num;       //  4 bit
    int reg_dmc_h_block;         //  3 bit
    int reg_dmc_v_block;         //  3 bit
    int reg_dmc_rgb_mode;        //  1 bit
    int reg_dmc_panel_h_size;    // 13 bit
    //int reg_dmc_r_en;            //  1 bit
    //int reg_dmc_g_en;            //  1 bit
    //int reg_dmc_b_en;            //  1 bit

    int reg_dmc_black_limit;     // 12 bit
    int reg_dmc_plane_level1;    // 12 bit
    int reg_dmc_plane_level2;    // 12 bit
    int reg_dmc_plane_level3;    // 12 bit
    int reg_dmc_plane_level4;    // 12 bit
    int reg_dmc_plane_level5;    // 12 bit
    int reg_dmc_plane_level6;    // 12 bit
    int reg_dmc_plane_level7;    // 12 bit
    int reg_dmc_plane_level8;    // 12 bit
    int reg_dmc_white_limit;     // 12 bit

    int reg_dmc_dither_rst_e_4_frame; //  1 bit
    int reg_dmc_dither_pse_rst_num;   //  2 bit
    int reg_dmc_dither_en;            //  2 bit

    MS_BOOL bROI_en;              // 1 bit
    int  iROI_hor_start_offset;
    int  iROI_hor_end_offset;
    int  iROI_ver_start_offset;
    int  iROI_ver_end_offset;


    strgb_structInfo *Lut_in[8];
    //int    **Lut_out;
    //double **Lut_out_inv;

}interface_info;


typedef enum
{
    TYPE_MSTAR = 0,
    TYPE_INX = 256,
} DEMURA_TYPE;

typedef enum
{
    CRC16 = sizeof(MS_U16),
    CRC32 = sizeof(MS_U32),
    CRC64 = sizeof(MS_U64),
} CRC_TYPE;

typedef struct
{
  MS_U16   nBlackType;
  MS_U16   nBlackChecksum;
  MS_U32   nBlackStartAddr;
  MS_U32   nBlackDataLength;
} BlackHeader;

typedef struct
{
  MS_U64   nIdentification : 8;
  MS_U64   nASIC : 40;
  MS_U64   nVersion : 8;
  MS_U64   nAlgorithm : 8;
  MS_U32   nDate;
  MS_U8    nBlackStructure;
  MS_U8    nDefination;
  MS_U16   nReserved;
  BlackHeader nBlack1;
  BlackHeader nBlack2;
  BlackHeader nBlack3;
  BlackHeader nBlack4;
  MS_U64   nReserved1;
  MS_U64   nReserved2;
  MS_U64   nReserved3;
  MS_U64   nReserved4;
  MS_U64   nReserved5;
  MS_U64   nReserved6;
  MS_U64   nReserved7;
  MS_U32   nReserved8;
  MS_U16   nReserved9;
  MS_U16   nCheckSum;
} DemuraHeader;

// BIN header structure (37 bytes)
typedef struct
{
  MS_U32   nAllBinCheckSum;                                               // 4
  MS_U32   nHeaderCheckSum;                                               // 8
  MS_U32   nHeaderSize;                                                   // 12
  MS_U32   nAllBinSize;                                                   // 16
  MS_U64   nDemuraIdH;                                                    // 24
  MS_U64   nDemuraIdL;                                                    // 32
  MS_U16   nBinVersion;                                                   // 34
  MS_U8    nDataFormat;                                                   // 35
  MS_U8    bR_ch_Enable : 1;                                              // 36
  MS_U8    bG_ch_Enable : 1;
  MS_U8    bB_ch_Enable : 1;
  MS_U8    nReserved : 5;
  MS_U16   nHNodeCount;                                                   // 38
  MS_U16   nVNodeCount;                                                   // 40
  MS_U8    nLayerStart : 4; //Bits[3:0]: Start layer number               // 41
  MS_U8    nLayerEnd : 4;   //Bits[7:4]: End layer number
  MS_U8    bSeparate;       //0: Y channel, 1: Separate R/G/B channels    // 42
  MS_U32   nLDataStartAddr;                                               // 46
  MS_U32   nLDataOriginalSize;                                            // 50
  MS_U32   nLDataFlashSize;                                               // 54
  MS_U32   nRegisterStartAddr;                                            // 58
  MS_U32   nRegisterCount;                                                // 62
  MS_U16   nDllVersion;                                                   // 64
  MS_U32   nProjectID;                                                    // 68
  MS_U32   nCusDataStartAddr;                                             // 72
  MS_U32   nCusDataSize;                                                  // 76
  MS_U32   nDate;                                                         // 80
  MS_U8    nHBlockSize;                                                   // 81
  MS_U8    nVBlockSize;                                                   // 82
  MS_U8    nPacketSize;                                                   // 83
} __attribute__((packed)) MstarSubBinHeader;  //83 byte


typedef struct
{
  MS_U32   RegAddr : 24;
  MS_U32   RegValue : 8;
  MS_U8    RegMask;
}  __attribute__((packed)) RegData;

#endif

