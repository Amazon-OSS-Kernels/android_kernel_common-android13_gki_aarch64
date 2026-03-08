/* SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause) */
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

#ifndef _PNL_IMPL_HEADER_
#define _PNL_IMPL_HEADER_
#include <apiPNL.h>

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

APIPNL_LINK_EXT_TYPE getLinkExtType(void);
unsigned int mtk_get_current_panel_width(void);
unsigned int mtk_get_current_panel_height(void);
unsigned int mtk_get_current_panel_hstart(void);
void mtk_panel_test_on_init(void);

#endif
