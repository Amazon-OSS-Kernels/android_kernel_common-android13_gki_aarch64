/* SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause) */
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

#ifndef _PNL_IMPL_HEADER_
#define _PNL_IMPL_HEADER_
//#include <apiPNL.h>
#include <MsTypes.h>

#ifndef UNUSED
#define UNUSED(x) ((x)=(x))
#endif

#ifndef LVDS_PN_SWAP_L
#define LVDS_PN_SWAP_L                  0x00
#endif
#ifndef LVDS_PN_SWAP_H
#define LVDS_PN_SWAP_H                  0x00
#endif
#ifdef CONFIG_ENABLE_CUST_IC_UPDATE
#define CUST_IC_MAX_COUNT               (6)
#endif
/// Max order Size  for CH order
#define MAX_ORDER_SIZE      8
#define PWM_MAX_CH   8

typedef enum
{
    PANEL_TYPE_FULLHD_SEC_LE32A,
    PANEL_TYPE_SXGA_AU17_EN05,
    PANEL_TYPE_WXGA_AU20_T200XW02,
    PANEL_TYPE_WXGA_PLUS_CMO190_M190A1,
    PANEL_TYPE_WSXGA_AU22_M201EW01,
    PANEL_TYPE_FULLHD_CMO216_H1L01,
    PANEL_TYPE_RES_FULL_HD,
    PANEL_TYPE_WUXGA_CMO260_J2,
    PANEL_TYPE_DACOUT_480I,
    PANEL_TYPE_DACOUT_480P,
    PANEL_TYPE_DACOUT_576I,
    PANEL_TYPE_DACOUT_576P,
    PANEL_TYPE_DACOUT_720P_50,
    PANEL_TYPE_DACOUT_720P_60,
    PANEL_TYPE_DACOUT_1080I_50,
    PANEL_TYPE_DACOUT_1080I_60,
    PANEL_TYPE_DACOUT_1080P_50,
    PANEL_TYPE_DACOUT_1080P_60,
}CurrentPanelType;

typedef enum{
    PANEL_RES_MIN = 0,

    PNL_FULLHD_SEC_LE32A = PANEL_RES_MIN,
    // FOR NORMAL LVDS PANEL
    PNL_SXGA_AU17_EN05       = 1,            // 1280X1024, PNL_AU17_EN05_SXGA
    PNL_WXGA_AU20_T200XW02   = 2,            // 1366X768, PNL_AU20_T200XW02_WXGA,
    PNL_WXGA_PLUS_CMO190_M190A1 = 3,         // 1440X900, PNL_CMO19_M190A1_WXGA, PNL_AU19PW01_WXGA
    PNL_WSXGA_AU22_M201EW01  = 4,            // 1680X1050, PNL_AU20_M201EW01_WSXGA,
    PNL_FULLHD_CMO216_H1L01  = 5,            // 1920X1080, PNL_AU37_T370HW01_HD, PNL_CMO216H1_L01_HD.H
    PANEL_RES_FULL_HD       = 5,            // for auotbuild compatitable

    // FOR DAC/HDMI TX OUTPUT
    DACOUT_480I              = 6,            // JUST FOR U3 DAC OUTPUT 480I TIMING USAGE
    DACOUT_480P              = 7,            // JUST FOR U3 DAC OUTPUT 480P TIMING USAGE
    DACOUT_576I              = 8,            // JUST FOR U3 DAC OUTPUT 576I TIMING USAGE
    DACOUT_576P              = 9,            // JUST FOR U3 DAC OUTPUT 576P TIMING USAGE
    DACOUT_720P_50           = 10,           // JUST FOR U3 DAC OUTPUT 720P TIMING USAGE
    DACOUT_720P_60           = 11,           // JUST FOR U3 DAC OUTPUT 720P TIMING USAGE
    DACOUT_1080I_50          = 12,           // JUST FOR U3 DAC OUTPUT 1080I TIMING USAGE
    DACOUT_1080I_60          = 13,           // JUST FOR U3 DAC OUTPUT 1080I TIMING USAGE
    DACOUT_1080P_50          = 14,           // JUST FOR U3 DAC OUTPUT 1080P TIMING USAGE
    DACOUT_1080P_60          = 15,           // JUST FOR U3 DAC OUTPUT 1080P TIMING USAGE

    DACOUT_1080P_30          = 16,           // JUST FOR U3 DAC OUTPUT 1080P TIMING USAGE
    DACOUT_1080P_25          = 17,           // JUST FOR U3 DAC OUTPUT 1080P TIMING USAGE
    DACOUT_1080P_24          = 18,           // JUST FOR U3 DAC OUTPUT 1080P TIMING USAGE
    DACOUT_640x480_60        = 19,           // JUST FOR U3 DAC OUTPUT 1080P TIMING USAGE
    DACOUT_1920x2205P_24     = 20,           // JUST FOR U3 DAC OUTPUT 1080P TIMING USAGE

    DACOUT_1280x1470P_50     = 21,           // JUST FOR U3 DAC OUTPUT 1080P TIMING USAGE
    DACOUT_1280x1470P_60     = 22,           // JUST FOR U3 DAC OUTPUT 1080P TIMING USAGE
    DACOUT_3840x2160P_24     = 23,           // JUST FOR U3 DAC OUTPUT 1080P TIMING USAGE
    DACOUT_3840x2160P_25     = 24,           // JUST FOR U3 DAC OUTPUT 1080P TIMING USAGE
    DACOUT_3840x2160P_30     = 25,           // JUST FOR U3 DAC OUTPUT 1080P TIMING USAGE

    DACOUT_3840x2160P_50    =26,
    DACOUT_3840x2160P_60    =27,
    DACOUT_4096x2160P_24    =28,
    DACOUT_4096x2160P_25    =29,
    DACOUT_4096x2160P_30    =30,

    DACOUT_4096x2160P_50    =31,
    DACOUT_4096x2160P_60    =32,
    DACOUT_3840x2160P420_30    =33,
    DACOUT_3840x2160P420_50    =34,
    DACOUT_3840x2160P420_60    =35,
    DACOUT_4096x2160P420_30    =36,
    DACOUT_4096x2160P420_50    =37,
    DACOUT_4096x2160P420_60    =38,
    PNL_WUXGA_CMO260_J2,           // 1920*1200
    PNL_RES_MAX_NUM,
}PANEL_RESOLUTION_TYPE;

typedef enum
{
    //this valuse is offset in chunk header.
    VB1_CH_ORDER_16V=0x00,
    VB1_CH_ORDER_8V,
    VB1_CH_ORDER_4V,
    VB1_CH_ORDER_2V,
    VB1_CH_ORDER_1V,
    VB1_CH_ORDER_4O,
    VB1_CH_ORDER_2O,
    VB1_CH_ORDER_ORTHER=0xFF
} EN_VB1_CH_ORTHER_ITEM;

enum en_boot_backlight_control_sel {
	E_BOOT_BL_CTRL_OLED,
	E_BOOT_BL_CTRL_PWM_ADIM,
	E_BOOT_BL_CTRL_PWM,
	E_BOOT_BL_CTRL_SPI,
	E_BOOT_BL_CTRL_OLED_MAX,
};

#ifdef CONFIG_ENABLE_CUST_IC_UPDATE
typedef enum {
    E_PNL_CUST_IC_NONE = 0,
    E_PNL_CUST_IC_PMIC,
    E_PNL_CUST_IC_PGAMMAIC,
    E_PNL_CUST_IC_LEVELSHIFTIC,
    E_PNL_CUST_IC_VCOMIC,
    E_PNL_CUST_IC_SECOND_PMIC,
    E_PNL_CUST_IC_SECOND_PGAMMAIC,
    E_PNL_CUST_IC_SECOND_LEVELSHIFTIC,
    E_PNL_CUST_IC_SECOND_VCOMIC,
    E_PNL_CUST_IC_GENERAL,
    E_PNL_CUST_IC_MAX,
} en_pnl_cust_ic_type;
#endif

typedef enum
{
    BOOT_PNL_VERSION0100 = 0x01,
    BOOT_PNL_VERSION0200 = 0x02,
    BOOT_PNL_VERSION0203 = 0x0203,
    BOOT_PNL_VERSION0300 = 0x03,
    BOOT_PNL_VERSION0400 = 0x04,
    BOOT_PNL_VERSION0500 = 0x05,
    BOOT_PNL_VERSION0600 = 0x06,
    BOOT_PNL_VERSION0700 = 0x07,
    BOOT_PNL_VERSIONMAX,
} en_boot_pnl_version;

typedef enum
{
    LINK_TTL,                              ///< TTL  type
    LINK_LVDS,                             ///< LVDS type
    LINK_RSDS,                             ///< RSDS type
    LINK_MINILVDS,                         ///< TCON
    LINK_ANALOG_MINILVDS,                  ///< Analog TCON
    LINK_DIGITAL_MINILVDS,                 ///< Digital TCON
    LINK_MFC,                              ///< Ursa (TTL output to Ursa)
    LINK_DAC_I,                            ///< DAC output
    LINK_DAC_P,                            ///< DAC output
    LINK_PDPLVDS,                          ///< For PDP(Vsync use Manually MODE)
    LINK_EXT,                              /// EXT LPLL TYPE
} APIPNL_LINK_TYPE;

typedef enum
{
    E_APIPNL_GAMMA_8BIT_MAPPING = 0,      ///< mapping 1024 to 256 gamma entries
    E_APIPNL_GAMMA_10BIT_MAPPING,         ///< mapping 1024 to 1024 gamma entries
    E_APIPNL_GAMMA_ALL_MAPPING            ///< the library can map to any entries
} APIPNL_GAMMA_MAPPEING_MODE;

/// Define TI bit mode
typedef enum
{
    TI_10BIT_MODE = 0,
    TI_8BIT_MODE = 2,
    TI_6BIT_MODE = 3,
} APIPNL_TIBITMODE;

/// Define which panel output timing change mode is used to change VFreq for same panel
typedef enum
{
    E_PNL_CHG_DCLK   = 0,      ///<change output DClk to change Vfreq.
    E_PNL_CHG_HTOTAL = 1,      ///<change H total to change Vfreq.
    E_PNL_CHG_VTOTAL = 2,      ///<change V total to change Vfreq.
} APIPNL_OUT_TIMING_MODE;

/// Define panel output format bit mode
typedef enum
{
    OUTPUT_10BIT_MODE = 0,//default is 10bit, becasue 8bit panel can use 10bit config and 8bit config.
    OUTPUT_6BIT_MODE = 1, //but 10bit panel(like PDP panel) can only use 10bit config.
    OUTPUT_8BIT_MODE = 2, //and some PDA panel is 6bit.
} APIPNL_OUTPUTFORMAT_BITMODE;

/// Define aspect ratio
typedef enum
{
    E_PNL_ASPECT_RATIO_4_3    = 0,         ///< set aspect ratio to 4 : 3
    E_PNL_ASPECT_RATIO_WIDE,               ///< set aspect ratio to 16 : 9
    E_PNL_ASPECT_RATIO_OTHER,              ///< resvered for other aspect ratio other than 4:3/ 16:9
} E_PNL_ASPECT_RATIO;

typedef enum _PWM_ChNum
{
    E_PWM_CH0,
    E_PWM_CH1,
    E_PWM_CH2,
    E_PWM_CH3,
    E_PWM_CH4,
    E_PWM_CH5,
    E_PWM_CH6,
    E_PWM_CH7,
    E_PWM_CH8,
    E_PWM_CH9
} PWM_ChNum;

typedef enum _PWM_Result
{
    E_PWM_OK,
    E_PWM_FAIL
} PWM_Result;

/// A panel struct type used to specify the panel attributes, and settings from Board layout
typedef struct __attribute__((packed))
{
    const char *m_pPanelName;                ///<  PanelName
#if !(defined(UFO_PUBLIC_HEADER_212) || defined(UFO_PUBLIC_HEADER_300))
#if !defined (__aarch64__)
    MS_U32 u32AlignmentDummy0;
#endif
#endif
    //
    //  Panel output
    //
    MS_U8 m_bPanelDither :1;                 ///<  PANEL_DITHER, keep the setting
    APIPNL_LINK_TYPE m_ePanelLinkType   :4;  ///<  PANEL_LINK

    ///////////////////////////////////////////////
    // Board related setting
    ///////////////////////////////////////////////
    MS_U8 m_bPanelDualPort  :1;              ///<  VOP_21[8], MOD_4A[1],    PANEL_DUAL_PORT, refer to m_bPanelDoubleClk
    MS_U8 m_bPanelSwapPort  :1;              ///<  MOD_4A[0],               PANEL_SWAP_PORT, refer to "LVDS output app note" A/B channel swap
    MS_U8 m_bPanelSwapOdd_ML    :1;          ///<  PANEL_SWAP_ODD_ML
    MS_U8 m_bPanelSwapEven_ML   :1;          ///<  PANEL_SWAP_EVEN_ML
    MS_U8 m_bPanelSwapOdd_RB    :1;          ///<  PANEL_SWAP_ODD_RB
    MS_U8 m_bPanelSwapEven_RB   :1;          ///<  PANEL_SWAP_EVEN_RB

    MS_U8 m_bPanelSwapLVDS_POL  :1;          ///<  MOD_40[5], PANEL_SWAP_LVDS_POL, for differential P/N swap
    MS_U8 m_bPanelSwapLVDS_CH   :1;          ///<  MOD_40[6], PANEL_SWAP_LVDS_CH, for pair swap
    MS_U8 m_bPanelPDP10BIT      :1;          ///<  MOD_40[3], PANEL_PDP_10BIT ,for pair swap
    MS_U8 m_bPanelLVDS_TI_MODE  :1;          ///<  MOD_40[2], PANEL_LVDS_TI_MODE, refer to "LVDS output app note"

    ///////////////////////////////////////////////
    // For TTL Only
    ///////////////////////////////////////////////
    MS_U8 m_ucPanelDCLKDelay;                ///<  PANEL_DCLK_DELAY
    MS_U8 m_bPanelInvDCLK   :1;              ///<  MOD_4A[4],                   PANEL_INV_DCLK
    MS_U8 m_bPanelInvDE     :1;              ///<  MOD_4A[2],                   PANEL_INV_DE
    MS_U8 m_bPanelInvHSync  :1;              ///<  MOD_4A[12],                  PANEL_INV_HSYNC
    MS_U8 m_bPanelInvVSync  :1;              ///<  MOD_4A[3],                   PANEL_INV_VSYNC

    ///////////////////////////////////////////////
    // Output driving current setting
    ///////////////////////////////////////////////
    // driving current setting (0x00=4mA, 0x01=6mA, 0x02=8mA, 0x03=12mA)
    MS_U8 m_ucPanelDCKLCurrent;              ///<  define PANEL_DCLK_CURRENT
    MS_U8 m_ucPanelDECurrent;                ///<  define PANEL_DE_CURRENT
    MS_U8 m_ucPanelODDDataCurrent;           ///<  define PANEL_ODD_DATA_CURRENT
    MS_U8 m_ucPanelEvenDataCurrent;          ///<  define PANEL_EVEN_DATA_CURRENT

    ///////////////////////////////////////////////
    // panel on/off timing
    ///////////////////////////////////////////////
    MS_U16 m_wPanelOnTiming1;                ///<  time between panel & data while turn on power
    MS_U16 m_wPanelOnTiming2;                ///<  time between data & back light while turn on power
    MS_U16 m_wPanelOffTiming1;               ///<  time between back light & data while turn off power
    MS_U16 m_wPanelOffTiming2;               ///<  time between data & panel while turn off power

    ///////////////////////////////////////////////
    // panel timing spec.
    ///////////////////////////////////////////////
    // sync related
    MS_U8 m_ucPanelHSyncWidth;               ///<  VOP_01[7:0], PANEL_HSYNC_WIDTH
    MS_U8 m_ucPanelHSyncBackPorch;           ///<  PANEL_HSYNC_BACK_PORCH, no register setting, provide value for query only

                                             ///<  not support Manuel VSync Start/End now
                                             ///<  VOP_02[10:0] VSync start = Vtt - VBackPorch - VSyncWidth
                                             ///<  VOP_03[10:0] VSync end = Vtt - VBackPorch
    MS_U8 m_ucPanelVSyncWidth;               ///<  define PANEL_VSYNC_WIDTH
    MS_U8 m_ucPanelVBackPorch;               ///<  define PANEL_VSYNC_BACK_PORCH

    // DE related
    MS_U16 m_wPanelHStart;                   ///<  VOP_04[11:0], PANEL_HSTART, DE H Start (PANEL_HSYNC_WIDTH + PANEL_HSYNC_BACK_PORCH)
    MS_U16 m_wPanelVStart;                   ///<  VOP_06[11:0], PANEL_VSTART, DE V Start
    MS_U16 m_wPanelWidth;                    ///< PANEL_WIDTH, DE width (VOP_05[11:0] = HEnd = HStart + Width - 1)
    MS_U16 m_wPanelHeight;                   ///< PANEL_HEIGHT, DE height (VOP_07[11:0], = Vend = VStart + Height - 1)

    // DClk related
    MS_U16 m_wPanelMaxHTotal;                ///<  PANEL_MAX_HTOTAL. Reserved for future using.
    MS_U16 m_wPanelHTotal;                   ///<  VOP_0C[11:0], PANEL_HTOTAL
    MS_U16 m_wPanelMinHTotal;                ///<  PANEL_MIN_HTOTAL. Reserved for future using.

    MS_U16 m_wPanelMaxVTotal;                ///<  PANEL_MAX_VTOTAL. Reserved for future using.
    MS_U16 m_wPanelVTotal;                   ///<  VOP_0D[11:0], PANEL_VTOTAL
    MS_U16 m_wPanelMinVTotal;                ///<  PANEL_MIN_VTOTAL. Reserved for future using.

    MS_U64 m_dwPanelDCLK;                     ///<  LPLL_0F[23:0], PANEL_DCLK          ,{0x3100_10[7:0], 0x3100_0F[15:0]}
                                             ///<  spread spectrum
    MS_U16 m_wSpreadSpectrumStep;            ///<  move to board define, no use now.
    MS_U16 m_wSpreadSpectrumSpan;            ///<  move to board define, no use now.

    MS_U8 m_ucDimmingCtl;                    ///<  Initial Dimming Value
    MS_U8 m_ucMaxPWMVal;                     ///<  Max Dimming Value
    MS_U8 m_ucMinPWMVal;                     ///<  Min Dimming Value

    MS_U8 m_bPanelDeinterMode   :1;          ///<  define PANEL_DEINTER_MODE,  no use now
    E_PNL_ASPECT_RATIO m_ucPanelAspectRatio; ///<  Panel Aspect Ratio, provide information to upper layer application for aspect ratio setting.
  /*
    *
    * Board related params
    *
    *  If a board ( like BD_MST064C_D01A_S ) swap LVDS TX polarity
    *    : This polarity swap value =
    *      (LVDS_PN_SWAP_H<<8) | LVDS_PN_SWAP_L from board define,
    *  Otherwise
    *    : The value shall set to 0.
    */
    MS_U16 m_u16LVDSTxSwapValue;
    APIPNL_TIBITMODE m_ucTiBitMode;                         ///< MOD_4B[1:0], refer to "LVDS output app note"
    APIPNL_OUTPUTFORMAT_BITMODE m_ucOutputFormatBitMode;

    MS_U8 m_bPanelSwapOdd_RG    :1;          ///<  define PANEL_SWAP_ODD_RG
    MS_U8 m_bPanelSwapEven_RG   :1;          ///<  define PANEL_SWAP_EVEN_RG
    MS_U8 m_bPanelSwapOdd_GB    :1;          ///<  define PANEL_SWAP_ODD_GB
    MS_U8 m_bPanelSwapEven_GB   :1;          ///<  define PANEL_SWAP_EVEN_GB

    /**
    *  Others
    */
    MS_U8 m_bPanelDoubleClk     :1;             ///<  LPLL_03[7], define Double Clock ,LVDS dual mode
    MS_U32 m_dwPanelMaxSET;                     ///<  define PANEL_MAX_SET
    MS_U32 m_dwPanelMinSET;                     ///<  define PANEL_MIN_SET
    APIPNL_OUT_TIMING_MODE m_ucOutTimingMode;   ///<Define which panel output timing change mode is used to change VFreq for same panel
    MS_U8 m_bPanelNoiseDith     :1;             ///<  PAFRC mixed with noise dither disable

    //Vsync related
    MS_U16 m_u16VsyncStart;                   ///<  Vsync Start

    //HDMI2.1 VRR
    MS_BOOL bVRR_en;                         // variable refresh rate
    MS_U16 u16VRRMaxRate;                    // vrr max frame rate
    MS_U16 u16VRRMinRate;                    // vrr min frame rate
    MS_BOOL bCinemaVRR_en;
    MS_U16 u16M_delta;                       // change vtt every time by m_delta
    MS_U32 u32panel_type;
    MS_BOOL using_tcon_en;                         // variable refresh rate
    //vcc and backlight
    MS_U32 m_bVccBlCusCtrl;                  // bit control of VCC and backlight
    MS_U32 vcc_to_custic_delay;              // delay between vcc and pmic initial
} PanelType;

/// VB1ChannelOrder
typedef struct
{
    // 16V VB1 Channel order, Max Number @ref MAX_ORDER_SIZE
    unsigned char              u8Vb116vOrder[MAX_ORDER_SIZE];
    // 8V VB1 Channel order, Max Number @ref MAX_ORDER_SIZE
    unsigned char              u8Vb18vOrder[MAX_ORDER_SIZE];
    // 4V VB1 Channel order, Max Number @ref MAX_ORDER_SIZE
    unsigned char              u8Vb14vOrder[MAX_ORDER_SIZE];
    // 2V VB1 Channel order, Max Number @ref MAX_ORDER_SIZE
    unsigned char              u8Vb12vOrder[MAX_ORDER_SIZE];
    // 1V VB1 Channel order, Max Number @ref MAX_ORDER_SIZE
    unsigned char              u8Vb11vOrder[MAX_ORDER_SIZE];
    // 4O VB1 Channel order, Max Number @ref MAX_ORDER_SIZE
    unsigned char              u8Vb14OOrder[MAX_ORDER_SIZE];
    // 2O VB1 Channel order, Max Number @ref MAX_ORDER_SIZE
    unsigned char              u8Vb12OOrder[MAX_ORDER_SIZE];
}pnl_VB1ChannelOrder;

typedef struct
{
    unsigned char   m_u8BOARD_PWM_PORT;
    unsigned short  m_u16BOARD_LVDS_CONNECT_TYPE;
    unsigned char   m_bPANEL_PDP_10BIT;
    unsigned char   m_bPANEL_SWAP_LVDS_POL;
    unsigned char   m_bPANEL_SWAP_LVDS_CH;
    unsigned char   m_bPANEL_CONNECTOR_SWAP_PORT;
    unsigned int  m_u32LVDS_PN_SWAP;
    char    m_sGPIO1_PAD_NAME[32];
    unsigned short  m_u16GPIO1_INDEX;
    unsigned char   m_u8GPIO1_VALUE;
    char    m_sGPIO2_PAD_NAME[32];
    unsigned short  m_u16GPIO2_INDEX;
    unsigned char   m_u8GPIO2_VALUE;
}st_board_para;

typedef struct
{
    unsigned int  m_u32PWMPeriod[PWM_MAX_CH];
    unsigned short  m_u16DivPWM[PWM_MAX_CH];
    unsigned int  m_u32PWMDuty[PWM_MAX_CH];
    unsigned char   m_bPolPWM[PWM_MAX_CH];
    unsigned int  m_u32maxPWM[PWM_MAX_CH];
    unsigned int  m_u32minPWM[PWM_MAX_CH];
    unsigned short  m_u16PWMPort[PWM_MAX_CH];
    unsigned int  m_u32PWMDelay[PWM_MAX_CH];
    unsigned int  m_u32PWMInitDuty[PWM_MAX_CH];
    unsigned int  m_VCCBLCusCtrl;
    unsigned int  m_PwmToBlDelay;
    unsigned int  m_PwmPortMax;
}st_pwm_setting;

typedef struct
{
	unsigned char  m_u8LDMLedMSPCH;
	unsigned short m_u16LDMLedMSPMode;
	unsigned int   m_u32LDMLedMSPCLK;
	unsigned char  m_u8LDMLedMSPDeviceNum;
	unsigned short m_u16LDMLedMSPCmdLength;
	unsigned int   m_au32LDMLedMSPCmdID[256];
	unsigned int   m_au32LDMLedMSPCmdAddr[256];
	unsigned int   m_au32LDMLedMSPCmdValue[256];
	bool           m_bLDMLedMSPWBitEn;
	unsigned int   m_au32LDMLedMSPWBit[8];
	bool           m_bLDMLedMSPRBitEn;
	unsigned int   m_au32LDMLedMSPRBit[8];
}st_ldm_spi;

typedef struct
{
	unsigned int  	m_u32LDMSupport;
	bool   		m_bLDMCusPathuBoot;
	bool		m_bLEDMSPIEN;
}st_ldm_setting;

#ifdef CONFIG_ENABLE_CUST_IC_UPDATE
typedef struct {
    uint32_t info_version;
    uint32_t info_length;
    uint32_t ic_type;
    char *ic_bin_file_path;
    en_pnl_cust_ic_type bin_format_type;
    bool force_init;
    bool with_nvm;
    uint16_t i2c_bus;
    uint16_t i2c_mode;
    uint16_t i2c_dev_addr;
    uint16_t i2c_reg_offset;
    uint16_t vcc_dly;
    uint16_t gpio_pre_num;
    uint16_t gpio_pre_ops;
    uint16_t gpio_pre_dly;
    uint16_t gpio_post_num;
    uint16_t gpio_post_ops;
    uint16_t gpio_post_dly;
    uint16_t auto_update_from_flash;
    uint16_t read_mode;
    uint16_t write_mode;
    uint16_t i2c_burn_cmd;
    uint16_t i2c_burn_offset;
    uint16_t i2c_ctrl_reg;
    uint16_t i2c_ctrl_reg_offset;
    uint16_t data_start;
    uint16_t data_end;
    uint16_t nvm_chk_en;
    uint16_t nvm_chk_offset;
    uint16_t nvm_chk_val;
    uint16_t nvm_chk_i2c_post_dly;
    uint16_t nvm_chk_rst_dly;
    uint16_t *checksum_bypass_offset;
    uint16_t checksum_bypass_size;
}st_cust_ic_info;

typedef struct {
    bool override_enable;
    uint32_t addr_start;
    uint16_t addr_length;
    uint32_t vcom_sel;
}st_cust_pmic_vcom_info;

typedef struct{
    st_cust_ic_info pmic_info;
    st_cust_ic_info pgamma_info;
    st_cust_ic_info levelshift_info;
    st_cust_ic_info vcomic_info;
    st_cust_pmic_vcom_info pmic_vcom_info;
    st_cust_ic_info pmic_sub_info;
    st_cust_ic_info pgamma_sub_info;
    st_cust_ic_info levelshift_sub_info;
    st_cust_ic_info vcomic_sub_info;
}st_multi_cust_ic_info;

typedef struct {
    bool bVRR_HighFrameRateMode_Support;
    int ucVRR_HFR_ON_I2C_DATA_SIZE;
    unsigned char ucVRR_HFR_ON_I2C_DATA[7];
    int ucVRR_HFR_OFF_I2C_DATA_SIZE;
    unsigned char ucVRR_HFR_OFF_I2C_DATA[7];
    uint16_t ucVRR_HFR_ON_I2C_bus;
    uint16_t ucVRR_HFR_ON_I2C_mode;
    uint16_t ucVRR_HFR_ON_I2C_dev_addr;
    uint16_t ucVRR_HFR_ON_I2C_offst;
    bool bVRR_HighPixelClockMode_Support;
    bool bGAME_Direct60HzMode_Support;
    int ucGAME_60HZ_ON_I2C_DATA_SIZE;
    uint32_t ucGAME_60HZ_ON_I2C_DATA_0;
    uint32_t ucGAME_60HZ_ON_I2C_DATA_1;
    bool bSupportGpioModeChange;
    int u16GPIO_NUM_MODE_CHG;
    int ucGPIO_CTRL_MODE_CHG_ON;
    int ucGPIO_CTRL_MODE_CHG_OFF;
    int u16DELAY_MODE_CHG;
    bool bSupportTconI2cWP;
    int u16GPIO_NUM_TCON_I2C_WP;
    int ucGPIO_TCON_I2C_WP_ON;
    int ucGPIO_TCON_I2C_WP_OFF;
    bool bVRR_HFR_ON_I2C_SEC_Support;
    int ucVRR_HFR_ON_I2C_SEC_DATA_SIZE;
    unsigned char ucVRR_HFR_ON_I2C_SEC_DATA[7];
    int ucVRR_HFR_OFF_I2C_SEC_DATA_SIZE;
    unsigned char ucVRR_HFR_OFF_I2C_SEC_DATA[7];
    uint16_t ucVRR_HFR_ON_I2C_SEC_bus;
    uint16_t ucVRR_HFR_ON_I2C_SEC_mode;
    uint16_t ucVRR_HFR_ON_I2C_SEC_dev_addr;
    uint16_t ucVRR_HFR_ON_I2C_SEC_offst;
}st_cust_tcon_info;

typedef struct
{
    unsigned int    OLED_Support;
    unsigned char   slave_addr;
    unsigned int    channel_id;
    unsigned char   i2c_mode;
    unsigned char   lumin_gain_addr;
}st_oled_i2c_info;
#endif

typedef struct
{
    bool dmc_dlg_enable;
    bool bl_dmc_enable;
    uint16_t bl_dmc_vendorid;
    char *bl_dmc_partiton;
    char *bl_dmc_vendor_bin;
    uint16_t bl_dmc_bound;
}st_cust_dmc_info;

typedef struct
{
    char *panel_spi_bus;
    uint32_t panel_spi_speed;
    uint32_t panel_spi_mode;
}st_panel_spi_info;

unsigned int mtk_get_current_panel_width(void);
unsigned int mtk_get_current_panel_height(void);
unsigned int mtk_get_current_panel_hstart(void);
void mtk_panel_test_on_init(void);

#endif
