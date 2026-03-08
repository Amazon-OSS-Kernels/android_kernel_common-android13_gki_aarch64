// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

#include <config.h>
#include <common.h>
#include <command.h>
#include <malloc.h>
#include <dm/uclass.h>
#include <apiXC.h>
#include <apiPNL.h>
#include <mhal_panel.h>
#include <panel_impl.h>
#include <debug_impl.h>
#include <Gamma12BIT_256.inc>
#include <MsVersion.h>
#include <MsCommon.h>
#include <system_impl.h>
#include <dts_parser.h>
#include <mtk_panel.h>
#include <time.h>
#include <asm-generic/gpio.h>
#include <pwm.h>
#include <dm/uclass-id.h>
#include <display.h>
#include <iniutility.h>
#include <environment.h>
//#include <mtk_demura.h>
#include <utility.h>
#include <cli.h>
#ifdef CONFIG_ENABLE_DEMURA
#include <demura_impl.h>
#include <apiDemura.h>
#endif
#ifdef CONFIG_ENABLE_CUST_IC_UPDATE
#include <mtk_pnl_cust.h>
#endif
#include <standby.h>
#include <standby_impl.h>
#include <mtk-pm.h>

#define PANEL_PWM_DUTY_PARTITION_PATH		"persist"
#define PANEL_PWM_DUTY_INI_PATH		"pwm_duty.ini"
#define PANEL_DLG_PARTITION_PATH		"persist"
#define PANEL_DLG_INI_PATH		"panel_mode_cfg.ini"
#define SECOND_PWM_FULL_DUTY    0xFFFFFF

#define MST_ATOI(str) strtoul(((str != NULL) ? str : ""), NULL, 0);

#define OLED_MONITOR_PERIOD (100)
#define OLED_ERR_LIMIT 3

//-------------------------------------------------------------------------------------------------
//  Global Variables
//-------------------------------------------------------------------------------------------------
unsigned long lPanleTimer =0;
unsigned long lPanelOnTiming=0;

unsigned long lVccOnTiming=0;
unsigned long lVccToModDelayTiming=0;
unsigned long lModOnTiming=0;
unsigned long lModToBLonTiming=0;

#ifdef CONFIG_ENABLE_DEMURA
#ifdef CONFIG_ENABLE_DEMURA_DLG
extern bool panel_dlg_already_read;
extern int panel_dlg_read_ret;
extern struct panel_dlg_info panel_dlg_info;
#endif
#endif

//--------------------------------------------------------------------------------------------------
// Local Variables
//--------------------------------------------------------------------------------------------------
static PanelType panelpara;
#ifdef CONFIG_ENABLE_CUST_IC_UPDATE
static st_multi_cust_ic_info multi_cust_ic;
static st_cust_tcon_info tcon_info;
static st_oled_i2c_info oled_i2c_info;
#endif

#ifdef CONFIG_ENABLE_DEMURA
static DemuraImpl_Panel_Data PnlData = {0, 0, true, false, true, E_DEMURA_BIN_ORDER_NO_DLG};
#ifdef CONFIG_ENABLE_DEMURA_DLG
static DemuraImpl_Panel_Data PnlData2 = {0, 0, true, false, false, E_DEMURA_BIN_ORDER_NO_DLG};
#endif
#ifdef CONFIG_DEMURA_VENDOR_MULTI
static Demura_Panel_Vendor genDemuraVendorSelect;
#endif /* #ifdef CONFIG_DEMURA_VENDOR_MULTI */
#endif /* #ifdef CONFIG_ENABLE_DEMURA */

static st_board_para boardpara;
static unsigned char PanelLinkExtType=10;
static MS_BOOL bPanelEnable=FALSE;
static MS_BOOL bPanleReady=FALSE;
static st_pwm_setting pwm_setting;
static st_sys_misc_setting misc_setting;
unsigned long lpanelVCCOnTimer = 0;
CurrentPanelType eCurPanelType = PANEL_TYPE_FULLHD_CMO216_H1L01;
static st_ldm_setting ldm_setting;
static bool bOledPanelLock;



static int _msAPI_Scaler_SetPanelVCC(bool bEnable)
{
    int ret;

    int panel_gpio_pin = 0;
    ret = parse_dt("/board",integer_dt_parser,(void*)&panel_gpio_pin,"backlight-gpio2");
    if(ret < 0)
    {
        UBOOT_ERROR("Error: board information parse error in DTS\n");
        return -1;
    }

    if(bEnable)
    {
        ret = gpio_request(panel_gpio_pin, "panel_init");
        if (ret && ret != -EBUSY) {
            UBOOT_ERROR("gpio: requesting pin %u failed\n", panel_gpio_pin);
            return -1;
        }
        gpio_direction_output(panel_gpio_pin,1);
    }
    else
    {
        ret = gpio_request(panel_gpio_pin, "panel_init");
        if (ret && ret != -EBUSY) {
            UBOOT_ERROR("gpio: requesting pin %u failed\n", panel_gpio_pin);
            return -1;
        }
        gpio_direction_output(panel_gpio_pin,0);
    }
    return 0;
}

static void MApi_PNL_En(bool bPanelOn, unsigned short u16PanelTiming)
{
    if(bPanelOn)
    {
        /* enable/disable VCC */
        g_IPanel.Enable(bPanelOn);
    }
    else
    {
        g_IPanel.Enable(bPanelOn);
        /* enable/disable VCC */
        if(u16PanelTiming != 0)
            MsOS_DelayTask(u16PanelTiming);
        else
            MsOS_DelayTask(5);

        _msAPI_Scaler_SetPanelVCC(bPanelOn);
    }
}

static void MApi_PNL_On(unsigned short u16PanelOnTiming)
{
    _msAPI_Scaler_SetPanelVCC(TRUE);
    lpanelVCCOnTimer = get_timer(0);
}

bool MApi_check_is_trunk_flow(void)
{
	bool bIsTrunkFlow = true;

	if ((panelpara.u32panel_type&0x80) == 0x80)
		bIsTrunkFlow = false;
	else
		bIsTrunkFlow = true;

	return bIsTrunkFlow;
}

bool MApi_check_is_oled(void)
{
	bool bIsOledModule = false;

	if ((panelpara.u32panel_type&0x01) == 0)
		bIsOledModule = true;
	else
		bIsOledModule = false;

	return bIsOledModule;
}

bool MApi_check_is_internal_module(void)
{
	bool bIsInternalModule = true;

	/* bit2=0 = internal ; bit2=1 = external */
	if ((panelpara.u32panel_type&0x04) == 0x04)
		bIsInternalModule = false;
	else
		bIsInternalModule = true;

	return bIsInternalModule;
}

enum en_boot_backlight_control_sel _mtk_render_backlght_ctrl_sel(void)
{
	enum en_boot_backlight_control_sel eblctrlsel = E_BOOT_BL_CTRL_OLED_MAX;

	switch((panelpara.u32panel_type&0x18)>>3) {
	case 0:
		eblctrlsel = E_BOOT_BL_CTRL_OLED;
	break;
	case 1:
		eblctrlsel = E_BOOT_BL_CTRL_PWM_ADIM;
	break;
	case 2:
		eblctrlsel = E_BOOT_BL_CTRL_PWM;
	break;
	case 3:
		eblctrlsel = E_BOOT_BL_CTRL_SPI;
	break;
	default:
		eblctrlsel = E_BOOT_BL_CTRL_OLED;
	break;
	}

	return eblctrlsel;
}

#ifndef CONFIG_MTK_PANEL
static int MApi_PNL_SetInverter(bool bEnable)
{
    int ret;

    int panel_gpio_pin;
    ret = parse_dt("/board",integer_dt_parser,(void*)&panel_gpio_pin,"backlight-gpio1");
    if(ret < 0)
    {
        UBOOT_ERROR("Error: backlight gpio information parse error in DTS\n");
        return -1;
    }

    if(bEnable)
    {
        ret = gpio_request(panel_gpio_pin, "panel_init");
        if (ret && ret != -EBUSY) {
            UBOOT_ERROR("gpio: requesting pin %u failed\n", panel_gpio_pin);
            return -1;
        }
        gpio_direction_output(panel_gpio_pin,1);
    }
    else
    {
        ret = gpio_request(panel_gpio_pin, "panel_init");
        if (ret && ret != -EBUSY) {
            UBOOT_ERROR("gpio: requesting pin %u failed\n", panel_gpio_pin);
            return -1;
        }
        gpio_direction_output(panel_gpio_pin,0);
    }
    return 0;
}

static void MApi_PNL_SetBackLight(bool bEnable)
{
    if ( bEnable )
    {
        MApi_PNL_SetInverter(ENABLE);
    }
    else
    {
        MApi_PNL_SetInverter(DISABLE);
    }
}
#endif

static void setLinkExtType(APIPNL_LINK_EXT_TYPE linkExtType)
{
    PanelLinkExtType=linkExtType;
}

APIPNL_LINK_EXT_TYPE getLinkExtType(void)
{
    return PanelLinkExtType;
}

static void SetDivison(unsigned short u16GPIOINDEX,unsigned char u8GPIOVALUE)
{
    int ret;
    UBOOT_DEBUG("GPIO:%d Set:%d\n",u16GPIOINDEX,u8GPIOVALUE);
    ret = gpio_request(u16GPIOINDEX, "panel_init");
    if (ret && ret != -EBUSY) {
        UBOOT_ERROR("gpio: requesting pin %u failed\n", u16GPIOINDEX);
    }
    gpio_direction_output(u16GPIOINDEX, u8GPIOVALUE);
}

static void SetPWM(unsigned int Period,unsigned int Duty,unsigned short PwmDiv,
                   PWM_ChNum PWMPort,unsigned short bPolPWM,
                   unsigned int DutyInit,unsigned int DutyDelay)
{
    PWM_Result result = E_PWM_FAIL;
    struct udevice *dev;

    //uclass_get_device(UCLASS_PWM, 0, &dev);

    switch(PWMPort) {
        case E_PWM_CH0:
            uclass_get_device_by_name(UCLASS_PWM, "pwmscan0", &dev);
            break;
        case E_PWM_CH1:
            uclass_get_device_by_name(UCLASS_PWM, "pwmscan1", &dev);
            break;
        case E_PWM_CH2:
            uclass_get_device_by_name(UCLASS_PWM, "pwmscan2", &dev);
            break;
        case E_PWM_CH3:
            uclass_get_device_by_name(UCLASS_PWM, "pwmscan3", &dev);
            break;
        case E_PWM_CH4:
            uclass_get_device_by_name(UCLASS_PWM, "pwmscan4", &dev);
            break;
        case E_PWM_CH5:
            uclass_get_device_by_name(UCLASS_PWM, "pwmscan5", &dev);
            break;
        case E_PWM_CH6:
            uclass_get_device_by_name(UCLASS_PWM, "pwmscan6", &dev);
            break;
        case E_PWM_CH7:
            uclass_get_device_by_name(UCLASS_PWM, "pwmscan7", &dev);
            break;
        default:
            uclass_get_device(UCLASS_PWM, 0, &dev);
            break;
    }

    if (!dev) {
        UBOOT_DEBUG("\nPWMPort %d retry default uclass_get_device\n",PWMPort);
        uclass_get_device(UCLASS_PWM, 0, &dev);
    }

    if (!dev)
       return;

    UBOOT_DEBUG("\nPWMPort %d enable\n",PWMPort);
    if(DutyInit)
    {
       UBOOT_DEBUG("\nPeriod = %d DutyInit:%d\n",Period,DutyInit);
       result = pwm_set_config(dev, PWMPort, Period, DutyInit);
       if(DutyDelay)
       {
          UBOOT_DEBUG("Init Duty 0x%x ms between DutyInit \n",DutyDelay);
          mdelay(DutyDelay);
       }
    }
    UBOOT_DEBUG("\nPeriod = %d Duty:%d\n",Period,Duty);
    result = pwm_set_config(dev, PWMPort, Period, Duty);
    if(result == E_PWM_FAIL)
    {
        UBOOT_DEBUG("Sorry , call pwm_set_config() failed!~\n");
    }
    if (!dev)
        return;
    result = pwm_set_enable(dev, PWMPort, 1);
    if(result == E_PWM_FAIL)
    {
        UBOOT_DEBUG("Sorry , call pwm_set_enable() failed!~\n");
    }
    UBOOT_DEBUG("\nbPolPWM = %d\n",bPolPWM);
    if(bPolPWM == 1)
        pwm_set_invert(dev, PWMPort, 1);
    else
        pwm_set_invert(dev, PWMPort, 0);
}

static int PWM_init(bool onoff)
{
    unsigned int u32PWMDuty[PWM_MAX_CH] = {0};
    unsigned int u32PWMPeriod[PWM_MAX_CH] = {0};
    unsigned short u32PWMDIV[PWM_MAX_CH] = {0};
    unsigned int u32maxPWM[PWM_MAX_CH] = {0};
    unsigned int u32minPWM[PWM_MAX_CH] = {0};
    unsigned int u32PWMtoBLdelay =0;
    unsigned int u32PWMDutyInit[PWM_MAX_CH] = {0};
    unsigned int u32PWMDutyDelay[PWM_MAX_CH] = {0};
    PWM_ChNum PWMPort[PWM_MAX_CH] = {0};
    int i;
    int ret = 0;
    struct panel_pwm_duty_info pnl_pwm_duty_info;

    ret = parse_dt("/video_out",pwm_dt_parser,(void*)&pwm_setting,NULL);
    if(ret < 0)
    {
        UBOOT_DEBUG("Error: pwm information parse error in DTS\n");
        return -1;
    }

    PWMPort[0] = (PWM_ChNum)pwm_setting.m_u16PWMPort[0];
    if(PWMPort[0] < 0 || PWMPort[0] >9)
    {
        UBOOT_DEBUG("PWM CH out of range ,So use default CH2\n");
        PWMPort[0] = (PWM_ChNum)E_PWM_CH2;
    }

    if (pwm_setting.m_PwmPortMax == NULL)
    {
        UBOOT_DEBUG("m_PwmPortMax is null\n");
        pwm_setting.m_PwmPortMax = 1;
    }
    else
    {
        UBOOT_DEBUG("m_PwmPortMax = %d\n", pwm_setting.m_PwmPortMax);
    }

    for (i = 0; i < pwm_setting.m_PwmPortMax; i++) {
        u32PWMDIV[i] = pwm_setting.m_u16DivPWM[i];
        u32PWMPeriod[i] = pwm_setting.m_u32PWMPeriod[i];
        u32maxPWM[i] = pwm_setting.m_u32maxPWM[i];
        u32minPWM[i] = pwm_setting.m_u32minPWM[i];
        u32PWMtoBLdelay = pwm_setting.m_PwmToBlDelay;
        u32PWMDutyInit[i] = pwm_setting.m_u32PWMDelay[i];
        u32PWMDutyDelay[i] = pwm_setting.m_u32PWMInitDuty[i];

        PWMPort[i] = (PWM_ChNum)pwm_setting.m_u16PWMPort[i];
        if(PWMPort[i] < 0 || PWMPort[i] > 9 || !u32PWMPeriod[i])
        {
            UBOOT_DEBUG("Unexpected! PWMPort[%d] = %d\n", i, PWMPort[i]);
            continue;
        }

        memset(&pnl_pwm_duty_info, 0, sizeof(struct panel_pwm_duty_info));
        ret = get_panel_pwm_duty_info(PANEL_PWM_DUTY_PARTITION_PATH, PANEL_PWM_DUTY_INI_PATH, &pnl_pwm_duty_info);
        if(ret < 0)
        {
            UBOOT_DEBUG("pwm_duty parse error in ini, use dts setting.\n");
        }
        if(pnl_pwm_duty_info.pwm_duty != PWM_DUTY_INVALID)
        {
            u32PWMDuty[i] = pnl_pwm_duty_info.pwm_duty;
            UBOOT_DEBUG("u32PWMDuty_T in ini = 0x%x\n",pnl_pwm_duty_info.pwm_duty);
        }
        else
        {
            if(pwm_setting.m_u32PWMDuty[i] > 0)
            {
                u32PWMDuty[i] = pwm_setting.m_u32PWMDuty[i];
            }
            else
            {
                u32PWMDuty[i] = ((u32maxPWM[i] + u32minPWM[i]) >> 1); // set to 50% duty.
            }
        }

        UBOOT_DEBUG("Multi PWM = %d\n", i);
        UBOOT_DEBUG("u32PWMPeriod_T = 0x%x\n",u32PWMPeriod[i]);
        UBOOT_DEBUG("u32PWMDuty_T = 0x%x\n",u32PWMDuty[i]);
        UBOOT_DEBUG("u32PWMDIV_T = 0x%x\n",u32PWMDIV[i]);
        UBOOT_DEBUG("u32maxPWM_T = 0x%x\n",u32maxPWM[i]);
        UBOOT_DEBUG("u32minPWM_T = 0x%x\n",u32minPWM[i]);
        UBOOT_DEBUG("u32PWMtoBLdelay = 0x%x\n",u32PWMtoBLdelay);

        if (onoff)
        {
          SetPWM(u32PWMPeriod[i],u32PWMDuty[i], u32PWMDIV[i],
                 PWMPort[i],pwm_setting.m_bPolPWM[i], u32PWMDutyInit[i], u32PWMDutyDelay[i]);
        }
        else if (pwm_setting.m_bPolPWM[i])
        {
          // The case of PWM polarity 1, need to set PWM high level, after circuit transfer to minimum level of backlight
          // Here set period=duty=0, PWM will output high level.
          // And cannot set polarity here, or it will becomes low level.
          SetPWM(0, 0, 0,PWMPort[i],0, u32PWMDutyInit[i], u32PWMDutyDelay[i]);
        }
        else
        {
          // The case of PWM polarity 0, SetPWM should not be called.
          // SetPWM will enable PWM and start to output waveform even duty set to 0
        }
    }
    return 0;
}

static APIPNL_LINK_EXT_TYPE mtk_get_panel_linkext_type(void)
{
    APIPNL_LINK_EXT_TYPE eType = LINK_EPI34_8P;
    int ret;
    st_sys_misc_setting misc_setting;
    memset(&misc_setting, 0, sizeof(st_sys_misc_setting));
    ret = parse_dt("/video_out",misc_dt_parser,(void*)&misc_setting,NULL);
    if(ret < 0)
    {
        UBOOT_ERROR("Error: misc information parse error in DTS\n");
        return -1;
    }

    if((APIPNL_LINK_EXT_TYPE)misc_setting.m_u16Panel_ext_type!=0)
        eType = (APIPNL_LINK_EXT_TYPE)misc_setting.m_u16Panel_ext_type;
    return eType;
}

static void MsDrv_SetChannelOrder(unsigned char chOrderType,pnl_VB1ChannelOrder * channelOrder_data)
{
    unsigned short u16OutputOrder0_3=0;
    unsigned short u16OutputOrder4_7=0;
    unsigned short u16OutputOrder8_11=0;
    unsigned short u16OutputOrder12_13=0;
    UBOOT_TRACE("IN\n");
    if(chOrderType==VB1_CH_ORDER_16V)
    {
        u16OutputOrder0_3=(channelOrder_data->u8Vb116vOrder[0]<<8)+(channelOrder_data->u8Vb116vOrder[1]);
        u16OutputOrder4_7=(channelOrder_data->u8Vb116vOrder[2]<<8)+(channelOrder_data->u8Vb116vOrder[3]);
        u16OutputOrder8_11=(channelOrder_data->u8Vb116vOrder[4]<<8)+(channelOrder_data->u8Vb116vOrder[5]);
        u16OutputOrder12_13=(channelOrder_data->u8Vb116vOrder[6]<<8)+(channelOrder_data->u8Vb116vOrder[7]);
    }
    else if(chOrderType==VB1_CH_ORDER_8V)
    {
        u16OutputOrder0_3=(channelOrder_data->u8Vb18vOrder[0]<<8)+(channelOrder_data->u8Vb18vOrder[1]);
        u16OutputOrder4_7=(channelOrder_data->u8Vb18vOrder[2]<<8)+(channelOrder_data->u8Vb18vOrder[3]);
        u16OutputOrder8_11=(channelOrder_data->u8Vb18vOrder[4]<<8)+(channelOrder_data->u8Vb18vOrder[5]);
        u16OutputOrder12_13=(channelOrder_data->u8Vb18vOrder[6]<<8)+(channelOrder_data->u8Vb18vOrder[7]);
    }
     else if(chOrderType==VB1_CH_ORDER_4V)
    {
        u16OutputOrder0_3=(channelOrder_data->u8Vb14vOrder[0]<<8)+(channelOrder_data->u8Vb14vOrder[1]);
        u16OutputOrder4_7=(channelOrder_data->u8Vb14vOrder[2]<<8)+(channelOrder_data->u8Vb14vOrder[3]);
        u16OutputOrder8_11=(channelOrder_data->u8Vb14vOrder[4]<<8)+(channelOrder_data->u8Vb14vOrder[5]);
        u16OutputOrder12_13=(channelOrder_data->u8Vb14vOrder[6]<<8)+(channelOrder_data->u8Vb14vOrder[7]);
    }
      else if(chOrderType==VB1_CH_ORDER_2V)
    {
        u16OutputOrder0_3=(channelOrder_data->u8Vb12vOrder[0]<<8)+(channelOrder_data->u8Vb12vOrder[1]);
        u16OutputOrder4_7=(channelOrder_data->u8Vb12vOrder[2]<<8)+(channelOrder_data->u8Vb12vOrder[3]);
        u16OutputOrder8_11=(channelOrder_data->u8Vb12vOrder[4]<<8)+(channelOrder_data->u8Vb12vOrder[5]);
        u16OutputOrder12_13=(channelOrder_data->u8Vb12vOrder[6]<<8)+(channelOrder_data->u8Vb12vOrder[7]);
    }
      else if(chOrderType==VB1_CH_ORDER_1V)
    {
        u16OutputOrder0_3=(channelOrder_data->u8Vb11vOrder[0]<<8)+(channelOrder_data->u8Vb11vOrder[1]);
        u16OutputOrder4_7=(channelOrder_data->u8Vb11vOrder[2]<<8)+(channelOrder_data->u8Vb11vOrder[3]);
        u16OutputOrder8_11=(channelOrder_data->u8Vb11vOrder[4]<<8)+(channelOrder_data->u8Vb11vOrder[5]);
        u16OutputOrder12_13=(channelOrder_data->u8Vb11vOrder[6]<<8)+(channelOrder_data->u8Vb11vOrder[7]);
    }
    if((u16OutputOrder0_3+u16OutputOrder4_7+u16OutputOrder8_11+u16OutputOrder12_13)!=0)
    {
        MApi_PNL_MOD_OutputChannelOrder(APIPNL_OUTPUT_CHANNEL_ORDER_USER,u16OutputOrder0_3,u16OutputOrder4_7,u16OutputOrder8_11,u16OutputOrder12_13);
    }
    else
    {
        UBOOT_INFO("MOD Output Channel Order not set!!!!\n");
    }
    UBOOT_TRACE("OK\n");
}

static bool MsDrv_PNL_Init(PanelType* panel_data)
{
    bool ret=TRUE;
    char* s=NULL;
    unsigned short u16PanelDCLK=0;
    UBOOT_TRACE("IN\n");

    UNUSED(s);
    UNUSED(u16PanelDCLK);
    MApi_PNL_PreInit(E_PNL_NO_OUTPUT);
    MApi_PNL_SkipTimingChange(FALSE);

    if (panel_data->m_ePanelLinkType == LINK_TTL)
    {
        UBOOT_DEBUG("Panle Link Type=LINK_TTL \n");
        MApi_PNL_MOD_OutputConfig_User(PANEL_LINK_TTL_CFG0_7, PANEL_LINK_TTL_CFG8_15, PANEL_LINK_TTL_CFG16_21);
    }
    else if(panel_data->m_ePanelLinkType == LINK_LVDS)
    {
        MApi_PNL_MOD_OutputConfig_User(PANEL_LINK_LVDS_CFG0_7, PANEL_LINK_LVDS_CFG8_15, PANEL_LINK_LVDS_CFG16_21);
        UBOOT_DEBUG("Panle Link Type=LINK_LVDS \n");
    }
    else if(panel_data->m_ePanelLinkType == LINK_EXT)
    {
        UBOOT_DEBUG("Panle Link Type=LINK_EXT \n");
        UBOOT_DEBUG("Link Ext Type=%u \n", getLinkExtType());
        MApi_PNL_SetLPLLTypeExt(getLinkExtType());
        MApi_PNL_ForceSetPanelDCLK(misc_setting.m_u16PanelDCLK,TRUE);

        pnl_VB1ChannelOrder  channelOrder_data;
        memset(&channelOrder_data, 0, sizeof(pnl_VB1ChannelOrder));
        if((LINK_VBY1_10BIT_8LANE==getLinkExtType())||(LINK_VBY1_8BIT_8LANE==getLinkExtType()))
        {
            MsDrv_SetChannelOrder(VB1_CH_ORDER_8V,&channelOrder_data);
            MApi_PNL_MOD_OutputConfig_User(PANEL_LINK_EXT_8LANE_CFG0_7, PANEL_LINK_EXT_8LANE_CFG8_15, PANEL_LINK_EXT_8LANE_CFG16_21);
            UBOOT_DEBUG("VB1 Link Type=LINK_VBY1_10/8BIT_8LANE, u16PanelDCLK = %u\n", u16PanelDCLK);
        }
        else  if((LINK_VBY1_8BIT_16LANE==getLinkExtType())||(LINK_VBY1_10BIT_16LANE==getLinkExtType()))
        {
            MsDrv_SetChannelOrder(VB1_CH_ORDER_16V,&channelOrder_data);
            MApi_PNL_MOD_OutputConfig_User(PANEL_LINK_EXT_16LANE_CFG0_7, PANEL_LINK_EXT_16LANE_CFG8_15, PANEL_LINK_EXT_16LANE_CFG16_21);
            UBOOT_DEBUG("VB1 Link Type=LINK_VBY1_10/8BIT_16LANE, u16PanelDCLK = %u\n", u16PanelDCLK);
        }
        else if((LINK_VBY1_10BIT_4LANE==getLinkExtType())||(LINK_VBY1_8BIT_4LANE==getLinkExtType()))
        {
            MsDrv_SetChannelOrder(VB1_CH_ORDER_4V,&channelOrder_data);
            MApi_PNL_MOD_OutputConfig_User(PANEL_LINK_EXT_4LANE_CFG0_7, PANEL_LINK_EXT_4LANE_CFG8_15, PANEL_LINK_EXT_4LANE_CFG16_21);
            UBOOT_DEBUG("VB1 Link Type=LINK_VBY1_10/8BIT_4LANE, u16PanelDCLK = %u\n", u16PanelDCLK);
        }
        else if((LINK_VBY1_10BIT_2LANE==getLinkExtType())||(LINK_VBY1_8BIT_2LANE==getLinkExtType()))
        {
            MsDrv_SetChannelOrder(VB1_CH_ORDER_2V,&channelOrder_data);
            MApi_PNL_MOD_OutputConfig_User(PANEL_LINK_EXT_2LANE_CFG0_7, PANEL_LINK_EXT_2LANE_CFG8_15, PANEL_LINK_EXT_2LANE_CFG16_21);
            UBOOT_DEBUG("VB1 Link Type=LINK_VBY1_10/8BIT_2LANE, u16PanelDCLK = %u\n", u16PanelDCLK);
        }
        else if((LINK_VBY1_10BIT_1LANE==getLinkExtType())||(LINK_VBY1_8BIT_1LANE==getLinkExtType()))
        {
            MsDrv_SetChannelOrder(VB1_CH_ORDER_1V,&channelOrder_data);
            MApi_PNL_MOD_OutputConfig_User(PANEL_LINK_EXT_1LANE_CFG0_7, PANEL_LINK_EXT_1LANE_CFG8_15, PANEL_LINK_EXT_1LANE_CFG16_21);
            UBOOT_DEBUG("VB1 Link Type=LINK_VBY1_10/8BIT_1LANE, u16PanelDCLK = %u\n", u16PanelDCLK);
        }
        else
        {
            UBOOT_ERROR("VB1 Link Type NA !!!!\n");
        }
    }
    else if(panel_data->m_ePanelLinkType == LINK_DAC_P)
    {
        MApi_PNL_ForceSetPanelDCLK(misc_setting.m_u16PanelDCLK,TRUE);
    }
    if (misc_setting.m_u8PixelShiftEnable)
    {
        UBOOT_DEBUG("Enable PixelShift\n");
        MApi_PNL_Init_MISC(E_APIPNL_MISC_PIXELSHIFT_ENABLE);
    }

    //if (FALSE == pm_check_back_ground_active())
    {
        MApi_PNL_On(panel_data->m_wPanelOnTiming1);//Power on TCON and Delay Timming1
    }

    //load board para
    UBOOT_DEBUG("GPIO1_PAD_NAME %s\n",boardpara.m_sGPIO1_PAD_NAME);
    UBOOT_DEBUG("GPIO2_PAD_NAME %s\n",boardpara.m_sGPIO2_PAD_NAME);
    if ((strlen(boardpara.m_sGPIO1_PAD_NAME) != 0) && (strlen(boardpara.m_sGPIO2_PAD_NAME) != 0))
    {
        UBOOT_DEBUG("\n%s:SetDivison \n", __FUNCTION__);
        SetDivison(boardpara.m_u16GPIO1_INDEX,boardpara.m_u8GPIO1_VALUE);
        SetDivison(boardpara.m_u16GPIO2_INDEX,boardpara.m_u8GPIO2_VALUE);
    }
    else
    {
        UBOOT_DEBUG("\n%s:Bypass SetDivison \n", __FUNCTION__);
    }

    MApi_PNL_Init(panel_data);
    g_IPanel.Dump();
    g_IPanel.SetGammaTbl(E_APIPNL_GAMMA_12BIT, tAllGammaTab, GAMMA_MAPPING_MODE);

    //Need to Init XC first.
    if(TRUE != MApi_XC_Init(NULL,0))
    {
        UBOOT_ERROR("%s %d XC Init Fail\n",__FUNCTION__,__LINE__);
    }

    ret=MApi_PNL_PreInit(E_PNL_CLK_DATA);

    //if(FALSE == pm_check_back_ground_active())
    {
        MApi_PNL_En(TRUE, panel_data->m_wPanelOnTiming2);//Delay Timming2
    }
    lPanelOnTiming=panel_data->m_wPanelOnTiming2;
    lPanleTimer=get_timer(0);

    UBOOT_TRACE("OK\n");

    return ret;
}

unsigned int mtk_get_current_panel_width(void)
{
    return panelpara.m_wPanelWidth;
}

unsigned int mtk_get_current_panel_height(void)
{
	unsigned int ret = 0;

#ifdef CONFIG_ENABLE_DEMURA
#ifdef CONFIG_ENABLE_DEMURA_DLG
	if (!panel_dlg_already_read)
	{
		panel_dlg_read_ret = get_panel_dlg_info(PANEL_DLG_PARTITION_PATH, PANEL_DLG_INI_PATH, &panel_dlg_info);
		panel_dlg_already_read = true;
	}

	if ((panel_dlg_read_ret >= 0) && (panel_dlg_info.panel_dlg == 1))
		ret = PnlData2.u16PanelHeight; //DLG on
	else
		ret = PnlData.u16PanelHeight; //DLG off
#else
		ret = PnlData.u16PanelHeight; //No DLG
#endif

#else
	ret = panelpara.m_wPanelHeight; //No Demura
#endif
	return ret;
}

unsigned int mtk_get_current_panel_hstart(void)
{
    return panelpara.m_wPanelHStart;
}

bool mtk_is_panel_enable(void)
{
    return bPanelEnable;
}

int mtk_is_panel_ready(void)
{
    if(bPanleReady==TRUE)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

#define CHAR_BUF_SIZE 50
int _mtk_panel_oled_increase_errorcount(void)
{
    char *pcOledErrorCount = NULL;
    int nOledErrorCount = 0;
    char buf[CHAR_BUF_SIZE] = "\0";
    int ret = 0;

    //get boot env: OLEDPanelErrorCount
    pcOledErrorCount = env_get("OLEDPanelErrorCount");
    if (pcOledErrorCount == NULL)
    {
        env_set("OLEDPanelErrorCount", "0x1");
        UBOOT_ERROR("[OLED ERR_DET detect] OLEDPanelErrorCount = 0x1\n");
        return 0;
    }

    nOledErrorCount = MST_ATOI(pcOledErrorCount);
    //set env: OLEDPanelErrorCount to 1
    if (nOledErrorCount < 0)
    {
        env_set("OLEDPanelErrorCount", "0x1");
        UBOOT_ERROR("[OLED ERR_DET detect] OLEDPanelErrorCount = 0x1\n");
        return 0;
    }

    nOledErrorCount++;
    if (nOledErrorCount > OLED_ERR_LIMIT)
    {
        nOledErrorCount = OLED_ERR_LIMIT;
    }

    //increase OLEDPanelErrorCount
    UBOOT_ERROR("[OLED ERR_DET detect] OLEDPanelErrorCount = 0x%X\n", nOledErrorCount);
    ret = snprintf(buf, CHAR_BUF_SIZE, "0x%X", nOledErrorCount);
    if (ret < 0)
    {
        env_set("OLEDPanelErrorCount", "0x1");
        UBOOT_ERROR("[OLED ERR_DET detect] OLEDPanelErrorCount = 0x1\n");
        return 0;
    }
    env_set("OLEDPanelErrorCount", buf);

    return 0;
}


int mtk_panel_oled_detect_error(void)
{
    struct udevice *dev;
    struct gpio_desc gpio_oled_errdet;
    int ret = 0;
    char *pcOledIgnor = NULL;
    int nOledIgnor = 0;

    pcOledIgnor = env_get("IgnorOledBoot");
    if (pcOledIgnor != NULL)
    {
        nOledIgnor = MST_ATOI(pcOledIgnor);
    }
    if (nOledIgnor > 0)
    {
        UBOOT_DEBUG("%s Ignore OLED error detection in AC on\n", __func__);
        return 0;
    }

    ret = uclass_get_device_by_name(UCLASS_DISPLAY, "video_out", &dev);

    if (!ret)
    {
        gpio_request_by_name(dev, "oled_err_det-gpios", 0, &gpio_oled_errdet, GPIOD_IS_IN);
    }
    else
    {
        UBOOT_ERROR(" cannot get gpio-gpios from dtso");
        return 0;
    }

//check VCC, BL, EVDD
    if (dm_gpio_get_value(&gpio_oled_errdet) == 1)
    {
        UBOOT_ERROR("[OLED ERR_DET detect] wait %d ms to make sure\n", OLED_MONITOR_PERIOD);

        //Delay 100ms and check error det gpio again
        mdelay(OLED_MONITOR_PERIOD);
        if (dm_gpio_get_value(&gpio_oled_errdet) == 1)
        {
            _mtk_panel_oled_increase_errorcount();
            UBOOT_ERROR("[OLED ERR_DET detect] longer than %d ms\n", OLED_MONITOR_PERIOD);
            return 1;
        }
    }

    UBOOT_ERROR("[%s] Pass\n", __func__);
    return 0;

}

MS_BOOL mtk_panel_oled_handle_error(void)
{
    char* pcOledErrorCount;
    MS_U8 u8OledErrorCount = 0;

    pcOledErrorCount = env_get("OLEDPanelErrorCount");
    if (pcOledErrorCount == NULL)
    {
        return TRUE;
    }

    u8OledErrorCount = MST_ATOI(pcOledErrorCount);

    UBOOT_ERROR("[OLED ERR_DET detect] OLEDPanelErrorCount = %u \n", u8OledErrorCount);
    if (u8OledErrorCount >= OLED_ERR_LIMIT)
    {
        return FALSE;
    }

    return TRUE;
}

bool mtk_is_panel_oled_locked(void)
{
    return bOledPanelLock;
}

int _mtk_panel_force_standby(void)
{
    /* Clear the wakeup reason to avoid PM wakeup immediately right after poweroff */
    pm_set_wakeup_reason(NULL);
    /* Write remote boot status */
    PMU_REG_ACCESS(PMU_DUMMY_ADDR,PMU_DUMMY_OFFSET_BOOT_STATUS) = REMOTE_STATE_SECOND_STANDBY;
    UBOOT_DEBUG("Enter standby [%lx:%lx]=%lx\n",(unsigned long)PMU_DUMMY_ADDR, (unsigned long)PMU_DUMMY_OFFSET_BOOT_STATUS, (unsigned long)PMU_REG_ACCESS(PMU_DUMMY_ADDR,PMU_DUMMY_OFFSET_BOOT_STATUS));

    return do_standby_mode_power_control();
}

void mtk_panel_backlight_off(void)
{
    PWM_init(0);
}

void mtk_panel_backlight_on(void)
{
    UBOOT_TRACE("IN\n");
    #define ONRFDONE_TO_UNMUTE_DELAY_TIME 150
    unsigned long lpanelDelayTime=0;
    unsigned long ltotalTime=get_timer(0);
    unsigned long ltotalTime2=0;
    unsigned long lcurtime=get_timer(0);
    unsigned long lonrf_done_time=0;
    unsigned long ldifftime_from_vcc_on=0;
    unsigned long ldifftime_from_onrf_done=0;
    bool bonrf_gpio=0;
    struct gpio_desc gpio_oled_onrf;
    struct udevice *dev;
    int ret;
    st_sys_misc_setting misc_data;
    memset(&misc_data, 0, sizeof(misc_data));
    ret = parse_dt("/video_out",misc_dt_parser,(void*)&misc_data,NULL);
    if(ret < 0)
    {
        UBOOT_ERROR("Error: misc information parse error in DTS\n");
    }

    if(misc_data.m_wPanelOnTiming3>0)
    {
        UBOOT_DEBUG("Delay PanelOnTiming3 0x%x ms between PWM_EN BL_EN \n",misc_data.m_wPanelOnTiming3);
        mdelay(misc_data.m_wPanelOnTiming3);
    }
    else
    {
        UBOOT_DEBUG("NO PanelOnTiming3 Delay .... \n");
    }
    if (lModToBLonTiming>(ltotalTime-lModOnTiming))
    {
        lpanelDelayTime=(lModToBLonTiming-(ltotalTime-lModOnTiming));
        UBOOT_DEBUG("\n---%s:%d Set MsDrv_PNL_BackLigth_On DelayTask %lu \n", __FUNCTION__, __LINE__,lpanelDelayTime);
        mdelay(lpanelDelayTime);
    }
    else
    {
        UBOOT_DEBUG("\n---%s:%d Set MsDrv_PNL_BackLigth_On No Delay \n", __FUNCTION__, __LINE__);
    }

    ltotalTime=get_timer(0);
    if((MApi_check_is_trunk_flow() == false) &&
        (MApi_check_is_oled() == false) &&
        (MApi_check_is_internal_module() == true))
    {
        if (!mtk_get_panel_gpio_cusctrl())
        {
            #ifdef CONFIG_MTK_PANEL
            mtk_panel_enable_backlight(TRUE);
            #else
            MApi_PNL_SetBackLight(TRUE);//Power on backlight
            #endif
        }
    }
    else
    {
        PWM_init(1);
    }
    ltotalTime2=get_timer(0);
    if (pwm_setting.m_PwmToBlDelay>(ltotalTime2-ltotalTime)) {
        lpanelDelayTime = pwm_setting.m_PwmToBlDelay - (ltotalTime2-ltotalTime);
        UBOOT_DEBUG("Delay 0x%x ms between PWM_EN BL_EN \n",pwm_setting.m_PwmToBlDelay);
        UBOOT_DEBUG("Delay %lu ms between PWM_EN BL_EN (real) \n",lpanelDelayTime);
        mdelay(lpanelDelayTime);
    } else {
        UBOOT_DEBUG("NO PWM to BL Delay .... \n");
    }
#ifdef CONFIG_ENABLE_CUST_IC_UPDATE
    mtk_pnl_cust_settings_ontiming2_backlight(&multi_cust_ic);
#endif
	if((MApi_check_is_trunk_flow() == false) &&
		(MApi_check_is_oled() == false) &&
		(MApi_check_is_internal_module() == true))
	{
		PWM_init(1);
	} else {
		if (!mtk_get_panel_gpio_cusctrl())
		{
		#ifdef CONFIG_MTK_PANEL
		    mtk_panel_enable_backlight(TRUE);
		#else
		    MApi_PNL_SetBackLight(TRUE);//Power on backlight
		#endif
		}
	}

    if ((MApi_check_is_trunk_flow() == false) && (MApi_check_is_oled() == true))
    {
        ret = uclass_get_device_by_name(UCLASS_DISPLAY, "video_out", &dev);
        gpio_request_by_name(dev, "oled_onrf-gpios", 0, &gpio_oled_onrf, GPIOD_IS_IN);
        while (ldifftime_from_onrf_done < ONRFDONE_TO_UNMUTE_DELAY_TIME)
        {
            lcurtime = get_timer(0);
            bonrf_gpio = dm_gpio_get_value(&gpio_oled_onrf);
            UBOOT_DEBUG("ldifftime_from_vcc_on=%lu\n", ldifftime_from_vcc_on);
            UBOOT_DEBUG("ldifftime_from_onrf_done=%lu\n", ldifftime_from_onrf_done);
            UBOOT_DEBUG("bonrf_gpio=%d\n", bonrf_gpio);
            /* step1: Wait for bonrf_gpio=H or onrf time out, than remember the time(lonrf_done_time) and got to else */
            if ((ldifftime_from_vcc_on < misc_data.m_onrf_op) && (bonrf_gpio == false))
            {
                if (lcurtime >= lVccOnTiming)
                {
                    ldifftime_from_vcc_on = lcurtime - lVccOnTiming;
                }
                else
                {
                    ldifftime_from_vcc_on = misc_data.m_onrf_op;
                }
                lonrf_done_time = lcurtime;
            }
            else
            {
            /* Step2: After lonrf_done_time H or onrf time out, need to wait more 150ms to protect panel */
                if (lcurtime >= lonrf_done_time)
                {
                    ldifftime_from_onrf_done = lcurtime - lonrf_done_time;
                }
                else
                {
                    ldifftime_from_onrf_done = ONRFDONE_TO_UNMUTE_DELAY_TIME;
                }
            }
        }
        /* step3: unMute */
        mtk_panel_mute(false);
    }

    if (oled_i2c_info.OLED_Support == 1)
    {
        if (mtk_panel_oled_detect_error() == 1)
        {
            UBOOT_ERROR("[OLED ERR_DET detect] Turn off backlight\n");
            //Power off backlight
            #ifdef CONFIG_MTK_PANEL
                mtk_panel_enable_backlight(FALSE);
            #else
                MApi_PNL_SetBackLight(FALSE);
            #endif

            /* env_save() for OLEDPanelErrorCount, and reset bootargs for distortion issue */
            run_command("reset_bootargs",0);
            env_save();
            ret = _mtk_panel_force_standby();
            if(ret != 0)
                UBOOT_ERROR("[OLED ERR_DET detect] Fail to standby\n");
            else
                UBOOT_ERROR("[OLED ERR_DET detect] Force standby\n");
        }
    }

#ifdef CONFIG_ENABLE_CUST_IC_UPDATE
    mtk_pnl_cust_settings_after_backlight(&multi_cust_ic);
#endif
    UBOOT_TRACE("OK\n");
}

int mtk_panel_init(void)
{
    unsigned short u16Panel_SwingLevel;
    int ret;
    UBOOT_TRACE("IN\n");

    memset(&panelpara, 0, sizeof(panelpara));
    memset(&boardpara, 0, sizeof(boardpara));
    memset(&misc_setting, 0, sizeof(misc_setting));

    //load panel para
    ret = parse_dt("/video_out/panel_info",panel_dt_parser,(void*)&panelpara,NULL);

    if(ret < 0)
    {
        UBOOT_ERROR("Error: panel information parse error in DTS\n");
        return -1;
    }
    panelpara.m_ePanelLinkType = LINK_EXT;

    //load board para
    ret = parse_dt("/video_out",board_dt_parser,(void*)&boardpara,NULL);
    if(ret < 0)
    {
        UBOOT_ERROR("Error: board information parse error in DTS\n");
        return -1;
    }

    //panel setting by each board
    panelpara.m_bPanelPDP10BIT = boardpara.m_bPANEL_PDP_10BIT;
    panelpara.m_bPanelSwapLVDS_POL = boardpara.m_bPANEL_SWAP_LVDS_POL;
    panelpara.m_bPanelSwapLVDS_CH = boardpara.m_bPANEL_SWAP_LVDS_CH;
    panelpara.m_bPanelSwapPort ^= boardpara.m_bPANEL_CONNECTOR_SWAP_PORT;
    panelpara.m_u16LVDSTxSwapValue = boardpara.m_u32LVDS_PN_SWAP;

    ret = parse_dt("/video_out",misc_dt_parser,(void*)&misc_setting,NULL);
    if(ret < 0)
    {
        UBOOT_ERROR("Error: misc information parse error in DTS\n");
        return -1;
    }

    if(panelpara.m_ePanelLinkType >= LINK_EXT )
    {
        UBOOT_DEBUG(">> SW setting: m_ePanelLinkExtType = %d <<<\n",panelpara.m_ePanelLinkType);
        UBOOT_DEBUG(">> mtk_get_panel_linkext_type = %d <<<\n",mtk_get_panel_linkext_type());
        UBOOT_DEBUG(">> u16PanelDCLK = %u <<<\n",misc_setting.m_u16PanelDCLK);
        setLinkExtType(mtk_get_panel_linkext_type());
    }

    MApi_BD_LVDS_Output_Type(4);
    UBOOT_DEBUG("MApi_BD_LVDS_Output_Type =0x%x\n",4);
    if(MsDrv_PNL_Init(&panelpara)==FALSE)
    {
        bPanleReady=FALSE;
        return -1;
    }

    u16Panel_SwingLevel = 0xFA;
    if(MApi_PNL_Control_Out_Swing(u16Panel_SwingLevel)!=TRUE)
    {
        bPanleReady=FALSE;
        return -1;
    }

    bPanleReady=TRUE;

    UBOOT_TRACE("OK\n");
    return 0;
}

int mtk_panel_check_DLG_supported_version(int version)
{
	int ret = 0;
    st_cust_dmc_info st_cust_dmc_info;
    memset(&st_cust_dmc_info, 0x00, sizeof(st_cust_dmc_info));

    parse_dt("/video_out", cus_demura_dt_parser, (void*)&st_cust_dmc_info, NULL);
    UBOOT_TRACE("dmc dlg is %s.\n", st_cust_dmc_info.dmc_dlg_enable ? "enable" : "disable");
    if (st_cust_dmc_info.dmc_dlg_enable)
    {
	ret = (version == BOOT_PNL_VERSION0200) |
		(version == BOOT_PNL_VERSION0203) |
		(version == BOOT_PNL_VERSION0400) |
              (version == BOOT_PNL_VERSION0500) |
              (version == BOOT_PNL_VERSION0600);
    }
	return ret;
}

int mtk_set_panel_vcc_cusctrl(MS_BOOL vcc_bl_cusctrl, bool using_tcon_en)
{
	struct udevice *dev;
	struct gpio_desc gpio_vcc;
	int ret = 0;
	ret = uclass_get_device_by_name(UCLASS_DISPLAY, "video_out", &dev);

	if (!ret) {
		gpio_request_by_name(dev, "vcc-gpios", 0, &gpio_vcc, GPIOD_IS_OUT);
	} else {
		UBOOT_ERROR("cannot get vcc-gpios from dtso");
		return -1;
	}

	if ((pm_check_back_ground_active() == 0) || using_tcon_en)
	{
		//enable VCC gpio
		if(vcc_bl_cusctrl != 1){
			//get VCC/BL gpio
			ret = dm_gpio_set_value(&gpio_vcc, 1);
			if (ret != 0)
				UBOOT_ERROR("get gpio VCC value = %d\n",
				dm_gpio_get_value(&gpio_vcc));
		}
	} else {
		if(vcc_bl_cusctrl != 1){
			//get VCC/BL gpio
			ret = dm_gpio_set_value(&gpio_vcc, 0);
			if (ret != 0)
				UBOOT_ERROR("get gpio VCC value = %d\n",
				dm_gpio_get_value(&gpio_vcc));
		}
		UBOOT_INFO("QHB case, vcc return\n");
	}
	return 0;
}

int mtk_get_panel_gpio_cusctrl(void)
{
	return panelpara.m_bVccBlCusCtrl;
}
#ifdef CONFIG_MTK_PANEL
int mtk_panel_init_device(void)
{
    UBOOT_TRACE("IN\n");
    struct udevice *dev;
    int ret;
	int ret_ldm = 0;
#ifdef CONFIG_ENABLE_DEMURA
    int nPropVal = 0;
#endif
	unsigned int pnl_lib_version = 0xFFFFFFFF;
    st_cust_dmc_info st_cust_dmc_info;

    memset(&st_cust_dmc_info, 0x00, sizeof(st_cust_dmc_info));

    ret = parse_dt("/video_out/panel_info",panel_dt_parser,(void*)&panelpara,NULL);
    if(ret < 0)
    {
        UBOOT_ERROR("Error: panel information parse error in DTS\n");
        return -1;
    }
	ret = parse_dt("/video_out",cus_panel_dt_parser,(void*)&panelpara,NULL);
    if(ret < 0)
    {
        UBOOT_ERROR("Error: cus panel information parse error in DTS\n");
        return -1;
    }
    ret_ldm = parse_dt("/mediatek-ldm",ldm_dt_parser,(void*)&ldm_setting,NULL);
    if(ret_ldm < 0)
    {
        UBOOT_ERROR("Error: ldm information parse error in DTS\n");
    }
	if (parse_dt("/video_out/", integer_dt_parser,
        (void *)&pnl_lib_version, "pnl_lib_version") < 0)
    {
        pnl_lib_version = 0xFFFFFFFF;
    }

    parse_dt("/video_out", cus_demura_dt_parser, (void*)&st_cust_dmc_info, NULL);

#ifdef CONFIG_ENABLE_CUST_IC_UPDATE
    parse_dt("/video_out", cust_pmic_dt_parser, (void*)&multi_cust_ic, NULL);
    parse_dt("/video_out", cust_pgamma_dt_parser, (void*)&multi_cust_ic, NULL);
    parse_dt("/video_out", cust_levelshift_dt_parser, (void*)&multi_cust_ic, NULL);
    parse_dt("/video_out", cust_vcomic_dt_parser, (void*)&multi_cust_ic, NULL);

    parse_dt("/video_out", cust_pmic_sub_dt_parser, (void*)&multi_cust_ic, NULL);
    parse_dt("/video_out", cust_pgamma_sub_dt_parser, (void*)&multi_cust_ic, NULL);
    parse_dt("/video_out", cust_levelshift_sub_dt_parser,(void*)&multi_cust_ic, NULL);
    parse_dt("/video_out", cust_vcomic_sub_dt_parser, (void*)&multi_cust_ic, NULL);

    parse_dt("/video_out", cust_tcon_dt_parser, (void*)&tcon_info,NULL);
    parse_dt("/video_out", cust_oled_dt_parser, (void*)&oled_i2c_info, NULL);
#endif

    UBOOT_DEBUG("[mtk_panel_init_device] Get Diplay device from dts\n");
    lVccToModDelayTiming = panelpara.m_wPanelOnTiming1;
    lModToBLonTiming = panelpara.m_wPanelOnTiming2;

#ifdef CONFIG_ENABLE_CUST_IC_UPDATE
    mtk_pnl_cust_settings_befor_vcc(&multi_cust_ic);

    if (oled_i2c_info.OLED_Support == 1)
    {
        if (mtk_panel_oled_handle_error() == FALSE)
        {
            UBOOT_ERROR("OLED HW error over 3 times, stop power on panel !!!\n");
            //run_command("led pwm_start LOCKTV",0);
            bOledPanelLock = true;
            return -1;
        }
        else
        {
            bOledPanelLock = false;
        }
    }

#endif
	mtk_set_panel_vcc_cusctrl(panelpara.m_bVccBlCusCtrl, panelpara.using_tcon_en);

    uclass_get_device_by_name(UCLASS_DISPLAY, "video_out", &dev);
    uclass_get_device_by_name(UCLASS_DISPLAY, "ext_video_out", &dev);
    uclass_get_device_by_name(UCLASS_DISPLAY, "graphic_out", &dev);

    uclass_get_device_by_name(UCLASS_DISPLAY, "mediatek-ldm", &dev);

    lVccOnTiming=get_timer(0);

#ifdef CONFIG_ENABLE_DEMURA
    //get tcon bin demura
    ret = uclass_get_device_by_name(UCLASS_DISPLAY, "video_out", &dev);
    if (!ret)
    {
        if (display_get_property(dev, E_PNL_PROP_TCON_EN, &nPropVal) == 0)
        {
            PnlData.btcon = nPropVal;
        }
        if (display_get_property(dev, E_PNL_PROP_DEMURA_SEL, &nPropVal) == 0)
        {
            UBOOT_TRACE("Demura select=%d -> %d\n", genDemuraVendorSelect, nPropVal);
            genDemuraVendorSelect = nPropVal;
        }
        if (display_get_property(dev, E_PNL_PROP_DEMURA_EN, &nPropVal) == 0)
        {
            UBOOT_TRACE("Demura en=%d -> %d\n", PnlData.bon, nPropVal);
            PnlData.bon = nPropVal;
        }
    }
    else
        UBOOT_ERROR("Get video out device is fail\n");

    //if support demura, disable demura bypass first.
    if (PnlData.bon || st_cust_dmc_info.bl_dmc_enable)
    {
        MApi_Demura_Bypass(FALSE);
    }
#endif

#ifdef CONFIG_ENABLE_CUST_IC_UPDATE
    ret = uclass_get_device_by_name(UCLASS_DISPLAY, "video_out", &dev);
    if (!ret)
    {
        if (display_get_property(dev, E_PNL_PROP_VCOM_SEL, &nPropVal) == 0)
        {
            UBOOT_TRACE("Vcom sel=%d -> %d\n", multi_cust_ic.pmic_vcom_info.vcom_sel, nPropVal);
            multi_cust_ic.pmic_vcom_info.vcom_sel = nPropVal;
        }
    }
    else
        UBOOT_ERROR("Get video out device is fail\n");
    if (PnlData.btcon == TRUE)
    {
        //delay between vcc and pmic
        UBOOT_DEBUG("Delay 0x%x ms from between vcc and pmic\n", panelpara.vcc_to_custic_delay);
        if (panelpara.vcc_to_custic_delay > 0)
        {
            mdelay(panelpara.vcc_to_custic_delay);
        }
        mtk_pnl_cust_settings_vcc_ontiming1(&multi_cust_ic);
    }
    mtk_pnl_cust_set_panel_mode(&tcon_info);
#endif

#ifdef CONFIG_ENABLE_DEMURA
#ifdef CONFIG_ENABLE_DEMURA_DLG
    PnlData2.u16PanelWidth = panelpara.m_wPanelWidth;
    PnlData2.u16PanelHeight = panelpara.m_wPanelHeight>>1;
#endif
    PnlData.u16PanelWidth = panelpara.m_wPanelWidth;
    PnlData.u16PanelHeight = panelpara.m_wPanelHeight;
    UBOOT_TRACE("%d sel=%d\n",__LINE__, genDemuraVendorSelect);

#ifdef DEMURA_EFUSE_CHECK //SW efuse mode
	MS_BOOL efuse;

	efuse = MDrv_SYS_Query(E_SYS_QUERY_47);
	printf("[%s]efuse=%d\n", __func__, efuse);
	if (efuse) {
#ifdef CONFIG_DEMURA_VENDOR_MULTI
#ifdef CONFIG_ENABLE_DEMURA_DLG
		if (mtk_panel_check_DLG_supported_version(pnl_lib_version)) {
		#ifdef CONFIG_ENABLE_CUST_IC_UPDATE
			if (tcon_info.bVRR_HighFrameRateMode_Support) {
				if (mtk_pnl_is_dlg_mode() == TRUE) {
				UBOOT_TRACE("[%d]Demura DLG mode ON \n", __LINE__);
				PnlData.u8DemuraBinOrder = E_DEMURA_BIN_ORDER_FIRST;
				mtk_demura_init(PnlData, genDemuraVendorSelect, E_DEMURA_BIN_MAIN);
				PnlData2.u8DemuraBinOrder = E_DEMURA_BIN_ORDER_LAST;
				mtk_demura_init(PnlData2, genDemuraVendorSelect, E_DEMURA_BIN_DLG);
			} else {
				UBOOT_TRACE("[%d]Demura DLG mode OFF \n", __LINE__);
				PnlData2.u8DemuraBinOrder = E_DEMURA_BIN_ORDER_FIRST;
				mtk_demura_init(PnlData2, genDemuraVendorSelect, E_DEMURA_BIN_DLG);
				PnlData.u8DemuraBinOrder = E_DEMURA_BIN_ORDER_LAST;
				mtk_demura_init(PnlData, genDemuraVendorSelect, E_DEMURA_BIN_MAIN);
			}
			} else {
				PnlData.u8DemuraBinOrder = E_DEMURA_BIN_ORDER_NO_DLG;
				MApi_Demura_Init(PnlData);
			}
		#else
            UBOOT_TRACE("[%d]Demura DLG mode OFF \n", __LINE__);
		    PnlData2.u8DemuraBinOrder = E_DEMURA_BIN_ORDER_FIRST;
			mtk_demura_init(PnlData2, genDemuraVendorSelect, E_DEMURA_BIN_DLG);
			PnlData.u8DemuraBinOrder = E_DEMURA_BIN_ORDER_LAST;
			mtk_demura_init(PnlData, genDemuraVendorSelect, E_DEMURA_BIN_MAIN);
		#endif
		} else {
			PnlData.u8DemuraBinOrder = E_DEMURA_BIN_ORDER_NO_DLG;
			mtk_demura_init(PnlData, genDemuraVendorSelect, E_DEMURA_BIN_MAIN);
		}
#else
		PnlData.u8DemuraBinOrder = E_DEMURA_BIN_ORDER_NO_DLG;
		mtk_demura_init(PnlData, genDemuraVendorSelect, E_DEMURA_BIN_MAIN);
#endif

#else
		PnlData.u8DemuraBinOrder = E_DEMURA_BIN_ORDER_NO_DLG;
		MApi_Demura_Init(PnlData);
#endif
    }
#else /* #else DEMURA_EFUSE_CHECK */
#ifdef CONFIG_DEMURA_VENDOR_MULTI
#ifdef CONFIG_ENABLE_DEMURA_DLG
	if (mtk_panel_check_DLG_supported_version(pnl_lib_version)) {
#ifdef CONFIG_ENABLE_CUST_IC_UPDATE
		if (tcon_info.bVRR_HighFrameRateMode_Support) {
			if(mtk_pnl_is_dlg_mode() == TRUE) {
			UBOOT_TRACE("[%d]Demura DLG mode ON \n", __LINE__);
			PnlData.u8DemuraBinOrder = E_DEMURA_BIN_ORDER_FIRST;
			mtk_demura_init(PnlData, genDemuraVendorSelect, E_DEMURA_BIN_MAIN);
			PnlData2.u8DemuraBinOrder = E_DEMURA_BIN_ORDER_LAST;
			mtk_demura_init(PnlData2, genDemuraVendorSelect, E_DEMURA_BIN_DLG);
		} else {
			UBOOT_TRACE("[%d]Demura DLG mode OFF \n", __LINE__);
			PnlData2.u8DemuraBinOrder = E_DEMURA_BIN_ORDER_FIRST;
			mtk_demura_init(PnlData2, genDemuraVendorSelect, E_DEMURA_BIN_DLG);
			PnlData.u8DemuraBinOrder = E_DEMURA_BIN_ORDER_LAST;
			mtk_demura_init(PnlData, genDemuraVendorSelect, E_DEMURA_BIN_MAIN);
		}
		} else {
			PnlData.u8DemuraBinOrder = E_DEMURA_BIN_ORDER_NO_DLG;
			mtk_demura_init(PnlData, genDemuraVendorSelect, E_DEMURA_BIN_MAIN);
		}
#else
        UBOOT_TRACE("[%d]Demura DLG mode OFF \n", __LINE__);
		PnlData2.u8DemuraBinOrder = E_DEMURA_BIN_ORDER_FIRST;
		mtk_demura_init(PnlData2, genDemuraVendorSelect, E_DEMURA_BIN_DLG);
		PnlData.u8DemuraBinOrder = E_DEMURA_BIN_ORDER_LAST;
		mtk_demura_init(PnlData, genDemuraVendorSelect, E_DEMURA_BIN_MAIN);
#endif
	} else {
		PnlData.u8DemuraBinOrder = E_DEMURA_BIN_ORDER_NO_DLG;
		mtk_demura_init(PnlData, genDemuraVendorSelect, E_DEMURA_BIN_MAIN);
	}
#else
	PnlData.u8DemuraBinOrder = E_DEMURA_BIN_ORDER_NO_DLG;
	mtk_demura_init(PnlData, genDemuraVendorSelect, E_DEMURA_BIN_MAIN);
#endif

#else
	PnlData.u8DemuraBinOrder = E_DEMURA_BIN_ORDER_NO_DLG;
	mtk_demura_init(PnlData);
#endif

#endif /* #ifdef DEMURA_EFUSE_CHECK */
#endif /* #ifdef CONFIG_ENABLE_DEMURA */

    bPanleReady=TRUE;
    UBOOT_TRACE("OK\n");
    return 0;
}

int mtk_panel_enable(bool en)
{
    struct udevice *dev;
    int ret = 0;
    unsigned long lCurrentTimer = 0;
    unsigned long lModOnDelayTime = 0;
    bool bqhq_en = 0;

    //struct dm_display_ops *ops = display_get_ops(dev);
    struct display_timing timing;
    UBOOT_DEBUG("[mtk_panel_enable] en = %d\n",en);
    lCurrentTimer = get_timer(0);
    if ((lCurrentTimer - lVccOnTiming) < lVccToModDelayTiming){
        lModOnDelayTime = (lVccToModDelayTiming - (lCurrentTimer - lVccOnTiming));
        mdelay(lModOnDelayTime);
        UBOOT_DEBUG("\n---%s:%d Set mtk_panel_enable DelayTask %lu \n", __FUNCTION__, __LINE__,lModOnDelayTime);
    }
#ifdef CONFIG_ENABLE_CUST_IC_UPDATE
    mtk_pnl_cust_settings_ontiming1_data(&multi_cust_ic);
#endif
    if (en)
        timing.flags = 1<<5; //set active high bit
    else
        timing.flags = 1<<4; //set active low bit

    if ((pm_check_back_ground_active() == 1) &&
        (panelpara.using_tcon_en == 0))
    {
        UBOOT_INFO("QHB case, do_panel_output_enable return\n");
        bqhq_en = 1;
    }
    else
    {
        UBOOT_DEBUG("NOT QHB case, do_panel_output_enable continuous\n");
    }

    if (bqhq_en)
    {
        return 0;
    }

    ret = uclass_get_device_by_name(UCLASS_DISPLAY, "video_out", &dev);
    if (!ret) {
        display_enable(dev, 0, &timing);
    } else {
        UBOOT_DEBUG("Video_out path is not enabled\n");
    }

    ret = uclass_get_device_by_name(UCLASS_DISPLAY, "ext_video_out", &dev);
    if (!ret) {
        display_enable(dev, 0, &timing);
    } else {
        UBOOT_DEBUG("Ext_video_out path is not enabled\n");
    }

    ret = uclass_get_device_by_name(UCLASS_DISPLAY, "graphic_out", &dev);
    if (!ret) {
        display_enable(dev, 0, &timing);
    } else {
        UBOOT_DEBUG("Graphic_out path is not enabled\n");
    }
    ret = uclass_get_device_by_name(UCLASS_DISPLAY, "mediatek-ldm", &dev);
    if (!ret) {
        display_enable(dev, 0, &timing);
    } else {
        UBOOT_DEBUG("LDM is not enabled\n");
    }

    lModOnTiming = get_timer(0);
#ifdef CONFIG_ENABLE_CUST_IC_UPDATE
    mtk_pnl_cust_settings_data_ontiming2(&multi_cust_ic);
#endif
    bPanelEnable = TRUE;
    return 0;
}

void mtk_panel_test_on_init(void)
{
    struct udevice *dev;
    struct gpio_desc gpio_panel_test_on;
    int ret = 0;

    ret = uclass_get_device_by_name(UCLASS_DISPLAY, "video_out", &dev);

    if ((MApi_check_is_trunk_flow()) || (MApi_check_is_oled()))
        return;

    if (!ret) {
        ret = gpio_request_by_name(dev, "panel_test_on-gpios", 0, &gpio_panel_test_on, GPIOD_IS_OUT);
        if (ret) {
            UBOOT_DEBUG("Cannot find panel_test_on gpio\n");
        } else {
            UBOOT_DEBUG("panel_test_on gpio flag = 0x%lx\n", gpio_panel_test_on.flags);
            dm_gpio_set_value(&gpio_panel_test_on, 0);
        }
    } else {
        UBOOT_DEBUG("Video_out path is not enabled\n");
    }
}

int mtk_panel_enable_backlight(bool en)
{
    struct udevice *dev;
    struct gpio_desc gpio_backlight;
    int ret = 0;
    ret = uclass_get_device_by_name(UCLASS_DISPLAY, "video_out", &dev);

    if (!ret) {
        gpio_request_by_name(dev, "backlight-gpios", 0, &gpio_backlight, GPIOD_IS_OUT);

        UBOOT_DEBUG("en = %d , BL gpio flag = 0x%lx\n",en, gpio_backlight.flags);

        if (en == TRUE) {
            dm_gpio_set_value(&gpio_backlight, 1);
        } else {
            dm_gpio_set_value(&gpio_backlight, 0);
        }
    } else {
            UBOOT_DEBUG("Video_out path is not enabled\n");
    }

    return 0;
}

int mtk_panel_enable_vcc(bool en)
{
    struct udevice *dev;
    struct gpio_desc gpio_vcc;
    int ret = 0;
    ret = uclass_get_device_by_name(UCLASS_DISPLAY, "video_out", &dev);

    if (!ret) {
        gpio_request_by_name(dev, "vcc-gpios", 0, &gpio_vcc, GPIOD_IS_OUT);

        UBOOT_DEBUG("en = %d , panel vcc gpio flag = 0x%lx\n",en, gpio_vcc.flags);

        if (en == TRUE) {
            dm_gpio_set_value(&gpio_vcc, 1);
        } else {
            dm_gpio_set_value(&gpio_vcc, 0);
        }
    } else {
            UBOOT_DEBUG("Video_out path is not enabled\n");
    }

    return 0;
}

int mtk_panel_mute(bool en)
{
    struct udevice *dev;
    int ret = 0;
    #define MUTE_ENABLE_BIT 3
    #define MUTE_DISABLE_BIT 2

    //struct dm_display_ops *ops = display_get_ops(dev);
    struct display_timing timing;
    UBOOT_DEBUG("[mtk_panel_mute] en = %d\n",en);

    if (en)
        timing.flags = 1<<MUTE_ENABLE_BIT; //set active high bit
    else
        timing.flags = 1<<MUTE_DISABLE_BIT; //set active low bit

    if ((pm_check_back_ground_active() == 1) &&
	  panelpara.using_tcon_en && (en == false)) {
	  UBOOT_DEBUG("[mtk_panel_mute] QHB+TCONLESS case, no need to disable mute\n");
        return 0;
    }

    ret = uclass_get_device_by_name(UCLASS_DISPLAY, "video_out", &dev);
    if (!ret) {
        display_enable(dev, 0, &timing);
    } else {
        UBOOT_DEBUG("Video_out path is not enabled\n");
    }

    ret = uclass_get_device_by_name(UCLASS_DISPLAY, "ext_video_out", &dev);
    if (!ret) {
        display_enable(dev, 0, &timing);
    } else {
        UBOOT_DEBUG("Ext_video_out path is not enabled\n");
    }

    ret = uclass_get_device_by_name(UCLASS_DISPLAY, "graphic_out", &dev);
    if (!ret) {
        display_enable(dev, 0, &timing);
    } else {
        UBOOT_DEBUG("Graphic_out path is not enabled\n");
    }

    return 0;
}

#endif
