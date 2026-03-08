// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

#include <common.h>
#include <linux/libfdt.h>
#include <fdt.h>
#include <system_impl.h>
#include <dts_parser.h>
#include <debug_impl.h>
#include <jpd_impl.h>
#include <gegop_impl.h>
#include <panel_impl.h>
#include <demura_impl.h>

#define OFFSET_32        32
#define MAX_LEVEL        32
#define MAX_CUST_PATH_LEN   (128)

#define DEMURA_ENV_PARTITION_BACKLIGHT        "dmc_partition_backlight"
#define DEMURA_ENV_VENDOR_BACKLIGHT_BIN       "dmc_vendor_backlight_bin"
#define DEMURA_ENV_VENDOR_BACKLIGHT           "dmc_vendorid_backlight"

DECLARE_GLOBAL_DATA_PTR;

void integer_dt_parser(struct fdt_content *fdt_data, void *out, const char *field_target)
{
    int *value = (int*)out;
    if (!value || !out)
        return;

    if(strstr(fdt_data->field,field_target) != NULL)
    {
        *value = fdt32_to_cpu(fdt_data->attribute[0]);
    }
}

void string_dt_parser(struct fdt_content *fdt_data, void *out, const char *field_target)
{
    char *buffer = (char*)out;
    if (!buffer || !out)
        return;

    if(strstr(fdt_data->field,field_target) != NULL)
    {
        memcpy(buffer, fdt_data->attribute, fdt_data->len);
    }
}

void mmap_dt_parser(struct fdt_content *fdt_data, void *out, const char *field_target)
{
    struct dts_mmap *mmap = (struct dts_mmap *)out;
    if (!mmap || !out)
        return;

    if(strstr(fdt_data->field,field_target) != NULL)
    {
        mmap->address = (((unsigned long long)fdt32_to_cpu(fdt_data->attribute[0]))<<DTS_SHIFT_32_BIT)|fdt32_to_cpu(fdt_data->attribute[1]);
        mmap->size = (((unsigned long long)fdt32_to_cpu(fdt_data->attribute[2]))<<DTS_SHIFT_32_BIT)|fdt32_to_cpu(fdt_data->attribute[3]);
    }
}

void jpd_dt_parser(struct fdt_content *fdt_data, void *out, const char *field_target)
{
    jpd_setting_para *jpd_setting = (jpd_setting_para *)out;
    if (!jpd_setting || !out)
        return;

    if(strstr(fdt_data->field,"read-buf-start") != NULL)
    {
        jpd_setting->read_buffer = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"read-buf-size") != NULL)
    {
        jpd_setting->read_size = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"inter-buf-start") != NULL)
    {
        jpd_setting->inter_buffer = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"inter-buf-size") != NULL)
    {
        jpd_setting->inter_size = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"out-buf-start") != NULL)
    {
        jpd_setting->output_buffer = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"out-buf-size") != NULL)
    {
        jpd_setting->output_size = fdt32_to_cpu(fdt_data->attribute[0]);
    }
}

void gop_dt_parser(struct fdt_content *fdt_data, void *out, const char *field_target)
{
    int i;
    if (!out)
        return;

    if(strstr(fdt_data->field,"GRAPHIC_LAYER_ZORDER") != NULL)
    {
        st_gopmux_para *gop_setting = (st_gopmux_para*)out;
        for(i=0;i < gop_setting->gop_layer_num;i++)
        {
            gop_setting->gop_layer_index[i] = fdt32_to_cpu(fdt_data->attribute[i]);
        }
    }

    if (strstr (fdt_data->field,"GOP_MAPLAYER_TO_MUX") != NULL) {
        MS_U32 *GOP_Map_Layer = (MS_U32 *)out;
        for (i = 0; i < (fdt_data->len)/sizeof(int); i++) {
            *(GOP_Map_Layer + i) =  fdt32_to_cpu(fdt_data->attribute[i]);
        }
    }
}

void pwm_dt_parser(struct fdt_content *fdt_data, void *out, const char *field_target)
{
    st_pwm_setting *pwm_setting = (st_pwm_setting*)out;
    if (!pwm_setting || !out)
        return;

    if(strstr(fdt_data->field,"u32PeriodPWM") != NULL)
    {
        pwm_setting->m_u32PWMPeriod[0] = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"u16DivPWM") != NULL)
    {
        pwm_setting->m_u16DivPWM[0] = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"u32MaxPWMvalue") != NULL)
    {
        pwm_setting->m_u32maxPWM[0] = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"u32MinPWMvalue") != NULL)
    {
        pwm_setting->m_u32minPWM[0] = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"u16PWMPort") != NULL)
    {
        pwm_setting->m_u16PWMPort[0] = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"u32DutyPWM") != NULL)
    {
        pwm_setting->m_u32PWMDuty[0] = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"bPolPWM") != NULL)
    {
        pwm_setting->m_bPolPWM[0] = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if (strstr(fdt_data->field, "PWM_to_BL_delay") != NULL)
    {
        pwm_setting->m_PwmToBlDelay = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    //Multi PWM start
    if(strstr(fdt_data->field,"multiPwmPort") != NULL)
    {
        pwm_setting->m_PwmPortMax = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    // pwm 0
    if(strstr(fdt_data->field,"u32Period_PWM0") != NULL)
    {
        pwm_setting->m_u32PWMPeriod[0] = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"u16Div_PWM0") != NULL)
    {
        pwm_setting->m_u16DivPWM[0] = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"u32Max_PWMvalue0") != NULL)
    {
        pwm_setting->m_u32maxPWM[0] = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"u32Min_PWMvalue0") != NULL)
    {
        pwm_setting->m_u32minPWM[0] = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"u16PWM_Port0") != NULL)
    {
        pwm_setting->m_u16PWMPort[0] = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"u32Duty_PWM0") != NULL)
    {
        pwm_setting->m_u32PWMDuty[0] = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"bPol_PWM0") != NULL)
    {
        pwm_setting->m_bPolPWM[0] = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"PWM0_Delay") != NULL)
    {
        pwm_setting->m_u32PWMDelay[0] = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"u32Duty_PWMInit0") != NULL)
    {
        pwm_setting->m_u32PWMInitDuty[0] = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    // pwm 1
    if(strstr(fdt_data->field,"u32Period_PWM1") != NULL)
    {
        pwm_setting->m_u32PWMPeriod[1] = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"u16Div_PWM1") != NULL)
    {
        pwm_setting->m_u16DivPWM[1] = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"u32Max_PWMvalue1") != NULL)
    {
        pwm_setting->m_u32maxPWM[1] = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"u32Min_PWMvalue1") != NULL)
    {
        pwm_setting->m_u32minPWM[1] = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"u16PWM_Port1") != NULL)
    {
        pwm_setting->m_u16PWMPort[1] = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"u32Duty_PWM1") != NULL)
    {
        pwm_setting->m_u32PWMDuty[1] = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"bPol_PWM1") != NULL)
    {
        pwm_setting->m_bPolPWM[1] = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"PWM1_Delay") != NULL)
    {
        pwm_setting->m_u32PWMDelay[0] = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"u32Duty_PWMInit1") != NULL)
    {
        pwm_setting->m_u32PWMInitDuty[1] = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    // pwm 2
    if(strstr(fdt_data->field,"u32Period_PWM2") != NULL)
    {
        pwm_setting->m_u32PWMPeriod[2] = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"u16Div_PWM2") != NULL)
    {
        pwm_setting->m_u16DivPWM[2] = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"u32Max_PWMvalue2") != NULL)
    {
        pwm_setting->m_u32maxPWM[2] = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"u32Min_PWMvalue2") != NULL)
    {
        pwm_setting->m_u32minPWM[2] = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"u16PWM_Port2") != NULL)
    {
        pwm_setting->m_u16PWMPort[2] = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"u32Duty_PWM2") != NULL)
    {
        pwm_setting->m_u32PWMDuty[2] = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"bPol_PWM2") != NULL)
    {
        pwm_setting->m_bPolPWM[2] = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"PWM2_Delay") != NULL)
    {
        pwm_setting->m_u32PWMDelay[2] = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"u32Duty_PWMInit2") != NULL)
    {
        pwm_setting->m_u32PWMInitDuty[2] = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    // pwm 3
    if(strstr(fdt_data->field,"u32Period_PWM3") != NULL)
    {
        pwm_setting->m_u32PWMPeriod[3] = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"u16Div_PWM3") != NULL)
    {
        pwm_setting->m_u16DivPWM[3] = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"u32Max_PWMvalue3") != NULL)
    {
        pwm_setting->m_u32maxPWM[3] = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"u32Min_PWMvalue3") != NULL)
    {
        pwm_setting->m_u32minPWM[3] = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"u16PWM_Port3") != NULL)
    {
        pwm_setting->m_u16PWMPort[3] = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"u32Duty_PWM3") != NULL)
    {
        pwm_setting->m_u32PWMDuty[3] = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"bPol_PWM3") != NULL)
    {
        pwm_setting->m_bPolPWM[3] = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"PWM3_Delay") != NULL)
    {
        pwm_setting->m_u32PWMDelay[3] = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"u32Duty_PWMInit3") != NULL)
    {
        pwm_setting->m_u32PWMInitDuty[3] = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    // pwm 4
    if(strstr(fdt_data->field,"u32Period_PWM4") != NULL)
    {
        pwm_setting->m_u32PWMPeriod[4] = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"u16Div_PWM4") != NULL)
    {
        pwm_setting->m_u16DivPWM[4] = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"u32Max_PWMvalue4") != NULL)
    {
        pwm_setting->m_u32maxPWM[4] = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"u32Min_PWMvalue4") != NULL)
    {
        pwm_setting->m_u32minPWM[4] = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"u16PWM_Port4") != NULL)
    {
        pwm_setting->m_u16PWMPort[4] = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"u32Duty_PWM4") != NULL)
    {
        pwm_setting->m_u32PWMDuty[4] = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"bPol_PWM4") != NULL)
    {
        pwm_setting->m_bPolPWM[4] = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"PWM4_Delay") != NULL)
    {
        pwm_setting->m_u32PWMDelay[4] = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"u32Duty_PWMInit4") != NULL)
    {
        pwm_setting->m_u32PWMInitDuty[4] = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    // pwm 5
    if(strstr(fdt_data->field,"u32Period_PWM5") != NULL)
    {
        pwm_setting->m_u32PWMPeriod[5] = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"u16Div_PWM5") != NULL)
    {
        pwm_setting->m_u16DivPWM[5] = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"u32Max_PWMvalue5") != NULL)
    {
        pwm_setting->m_u32maxPWM[5] = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"u32Min_PWMvalue5") != NULL)
    {
        pwm_setting->m_u32minPWM[5] = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"u16PWM_Port5") != NULL)
    {
        pwm_setting->m_u16PWMPort[5] = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"u32Duty_PWM5") != NULL)
    {
        pwm_setting->m_u32PWMDuty[5] = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"bPol_PWM5") != NULL)
    {
        pwm_setting->m_bPolPWM[5] = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"PWM5_Delay") != NULL)
    {
        pwm_setting->m_u32PWMDelay[5] = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"u32Duty_PWMInit5") != NULL)
    {
        pwm_setting->m_u32PWMInitDuty[5] = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    // pwm 6
    if(strstr(fdt_data->field,"u32Period_PWM6") != NULL)
    {
        pwm_setting->m_u32PWMPeriod[6] = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"u16Div_PWM6") != NULL)
    {
        pwm_setting->m_u16DivPWM[6] = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"u32Max_PWMvalue6") != NULL)
    {
        pwm_setting->m_u32maxPWM[6] = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"u32Min_PWMvalue6") != NULL)
    {
        pwm_setting->m_u32minPWM[6] = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"u16PWM_Port6") != NULL)
    {
        pwm_setting->m_u16PWMPort[6] = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"u32Duty_PWM6") != NULL)
    {
        pwm_setting->m_u32PWMDuty[6] = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"bPol_PWM6") != NULL)
    {
        pwm_setting->m_bPolPWM[6] = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"PWM6_Delay") != NULL)
    {
        pwm_setting->m_u32PWMDelay[6] = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"u32Duty_PWMInit6") != NULL)
    {
        pwm_setting->m_u32PWMInitDuty[6] = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    // pwm 7
    if(strstr(fdt_data->field,"u32Period_PWM7") != NULL)
    {
        pwm_setting->m_u32PWMPeriod[7] = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"u16Div_PWM7") != NULL)
    {
        pwm_setting->m_u16DivPWM[7] = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"u32Max_PWMvalue7") != NULL)
    {
        pwm_setting->m_u32maxPWM[7] = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"u32Min_PWMvalue7") != NULL)
    {
        pwm_setting->m_u32minPWM[7] = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"u16PWM_Port7") != NULL)
    {
        pwm_setting->m_u16PWMPort[7] = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"u32Duty_PWM7") != NULL)
    {
        pwm_setting->m_u32PWMDuty[7] = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"bPol_PWM7") != NULL)
    {
        pwm_setting->m_bPolPWM[7] = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"PWM7_Delay") != NULL)
    {
        pwm_setting->m_u32PWMDelay[7] = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"u32Duty_PWMInit7") != NULL)
    {
        pwm_setting->m_u32PWMInitDuty[7] = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    //Multi PWM end
}

void cus_panel_dt_parser(struct fdt_content *fdt_data, void *out, const char *field_target)
{
	PanelType *panel_data = (PanelType*)out;

	if (!panel_data || !out)
		return;

	if(strstr(fdt_data->field,"VCC_BL_CusCtrl") != NULL)
	{
		panel_data->m_bVccBlCusCtrl = fdt32_to_cpu(fdt_data->attribute[0]);
	}
	if(strstr(fdt_data->field,"Panel_Type") != NULL)
	{
		panel_data->u32panel_type = fdt32_to_cpu(fdt_data->attribute[0]);
	}
	if(strstr(fdt_data->field,"vcc_to_custic_delay") != NULL)
	{
		panel_data->vcc_to_custic_delay = fdt32_to_cpu(fdt_data->attribute[0]);
	}
	if (strstr(fdt_data->field, "TCON_Enable") != NULL)
	{
		panel_data->using_tcon_en = fdt32_to_cpu(fdt_data->attribute[0]);
	}
}

void cus_demura_dt_parser(struct fdt_content *fdt_data, void *out, const char *field_target)
{
    st_cust_dmc_info *cust_dmc_info = (st_cust_dmc_info*)out;

    if (!cust_dmc_info || !out)
    {
        return;
    }
    if (strstr(fdt_data->field, "dmc_dlg_enable") != NULL)
    {
        cust_dmc_info->dmc_dlg_enable = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if (strstr(fdt_data->field, "bl_dmc_enable") != NULL)
    {
        cust_dmc_info->bl_dmc_enable = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if (strstr(fdt_data->field, "bl_dmc_vendorid") != NULL)
    {
        cust_dmc_info->bl_dmc_vendorid = fdt32_to_cpu(fdt_data->attribute[0]);
        if (env_get(DEMURA_ENV_VENDOR_BACKLIGHT) != NULL)
            cust_dmc_info->bl_dmc_vendorid = (uint16_t)mst_atoi(env_get(DEMURA_ENV_VENDOR_BACKLIGHT));
    }
    if (strstr(fdt_data->field, "bl_dmc_bound") != NULL)
    {
        cust_dmc_info->bl_dmc_bound = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if (strstr(fdt_data->field, "bl_dmc_partition") != NULL)
    {
        cust_dmc_info->bl_dmc_partiton = malloc(MAX_CUST_PATH_LEN);
        if (cust_dmc_info->bl_dmc_partiton != NULL)
        {
            memset(cust_dmc_info->bl_dmc_partiton, 0x00, MAX_CUST_PATH_LEN);
            memcpy(cust_dmc_info->bl_dmc_partiton, fdt_data->attribute, fdt_data->len);
        }
    }
    if (strstr(fdt_data->field, "bl_dmc_vendor_bin") != NULL)
    {
        cust_dmc_info->bl_dmc_vendor_bin = malloc(MAX_CUST_PATH_LEN);
        if (cust_dmc_info->bl_dmc_vendor_bin != NULL)
	{
            memset(cust_dmc_info->bl_dmc_vendor_bin, 0x00, MAX_CUST_PATH_LEN);
            memcpy(cust_dmc_info->bl_dmc_vendor_bin, fdt_data->attribute, fdt_data->len);
        }
        if (env_get(DEMURA_ENV_VENDOR_BACKLIGHT_BIN) != NULL)
            cust_dmc_info->bl_dmc_vendor_bin = env_get(DEMURA_ENV_VENDOR_BACKLIGHT_BIN);
	}
    return;
}


void panel_dt_parser(struct fdt_content *fdt_data, void *out, const char *field_target)
{
    PanelType *panel_data = (PanelType*)out;
	MS_U32 u32TypClk_H = 0, u32TypClk_L = 0;

    if (!panel_data || !out)
        return;

    if(strstr(fdt_data->field,"on_timing1") != NULL)
    {
        panel_data->m_wPanelOnTiming1 = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"on_timing2") != NULL)
    {
        panel_data->m_wPanelOnTiming2 = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"off_timing1") != NULL)
    {
        panel_data->m_wPanelOffTiming1 = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"off_timing2") != NULL)
    {
        panel_data->m_wPanelOffTiming2 = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"hsync_width") != NULL)
    {
        panel_data->m_ucPanelHSyncWidth = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"h_backporch") != NULL)
    {
        panel_data->m_ucPanelHSyncBackPorch = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"vsync_width") != NULL)
    {
        panel_data->m_ucPanelVSyncWidth = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"v_backporch") != NULL)
    {
        panel_data->m_ucPanelVBackPorch = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"de_hstart") != NULL)
    {
        panel_data->m_wPanelHStart = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"de_vstart") != NULL)
    {
        panel_data->m_wPanelVStart = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"resolution_width") != NULL)
    {
        panel_data->m_wPanelWidth = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"resolution_height") != NULL)
    {
        panel_data->m_wPanelHeight = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"max_h_total") != NULL)
    {
        panel_data->m_wPanelMaxHTotal = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"typ_h_total") != NULL)
    {
        panel_data->m_wPanelHTotal = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"min_h_total") != NULL)
    {
        panel_data->m_wPanelMinHTotal = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"max_v_total") != NULL)
    {
        panel_data->m_wPanelMaxVTotal = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"typ_v_total") != NULL)
    {
        panel_data->m_wPanelVTotal = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"min_v_total") != NULL)
    {
        panel_data->m_wPanelMinVTotal = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"typ_clk_high") != NULL)
    {
    	u32TypClk_H = fdt32_to_cpu(fdt_data->attribute[0]);
    }
	if(strstr(fdt_data->field,"typ_clk_low") != NULL)
    {
    	u32TypClk_L = fdt32_to_cpu(fdt_data->attribute[0]);
    }
	panel_data->m_dwPanelDCLK = ((MS_U64)u32TypClk_H << OFFSET_32) | u32TypClk_L;
    if(strstr(fdt_data->field,"m_ucDimmingCtl") != NULL)
    {
        panel_data->m_ucDimmingCtl = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"m_ucMaxPWMVal") != NULL)
    {
        panel_data->m_ucMaxPWMVal = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"m_ucMinPWMVal") != NULL)
    {
        panel_data->m_ucMinPWMVal = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"m_ucTiBitMode") != NULL)
    {
        panel_data->m_ucTiBitMode = fdt32_to_cpu(fdt_data->attribute[0]);
    }
}

#ifdef CONFIG_ENABLE_CUST_IC_UPDATE
void cust_pmic_dt_parser(struct fdt_content *fdt_data, void *out, const char *field_target)
{
    st_multi_cust_ic_info *cust_ic_setting = (st_multi_cust_ic_info*)out;
    if (!cust_ic_setting || !out)
        return;

    if(strstr(fdt_data->field,"pmic_info_version") != NULL)
    {
        cust_ic_setting->pmic_info.info_version = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"pmic_info_length") != NULL)
    {
        cust_ic_setting->pmic_info.info_length = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"pmic_type") != NULL)
    {
        cust_ic_setting->pmic_info.ic_type = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"pmic_bin_file_path") != NULL)
    {
        cust_ic_setting->pmic_info.ic_bin_file_path = malloc(MAX_CUST_PATH_LEN);
        if(cust_ic_setting->pmic_info.ic_bin_file_path != NULL) {
            memset(cust_ic_setting->pmic_info.ic_bin_file_path, 0x00, MAX_CUST_PATH_LEN);
            memcpy(cust_ic_setting->pmic_info.ic_bin_file_path, fdt_data->attribute, fdt_data->len);
        }
    }
    if(strstr(fdt_data->field,"pmic_checksum_bypass_offset") != NULL)
    {
        if (fdt_data->len > 0)
        {
            int array_size = fdt_data->len/sizeof(u32);
            cust_ic_setting->pmic_info.checksum_bypass_offset = malloc(fdt_data->len);
            if(cust_ic_setting->pmic_info.checksum_bypass_offset != NULL) {
                memset(cust_ic_setting->pmic_info.checksum_bypass_offset, 0, fdt_data->len);
                for (int i=0; i<array_size;i++)
                {
                    cust_ic_setting->pmic_info.checksum_bypass_offset[i] = fdt32_to_cpu(fdt_data->attribute[i]);
                }
            }
            cust_ic_setting->pmic_info.checksum_bypass_size = array_size;
        }
        else {
            cust_ic_setting->pmic_info.checksum_bypass_size = 0;
        }
    }
    if(strstr(fdt_data->field,"pmic_bin_format_type") != NULL)
    {
        cust_ic_setting->pmic_info.bin_format_type = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"pmic_force_init") != NULL)
    {
        cust_ic_setting->pmic_info.force_init = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"pmic_with_nvm") != NULL)
    {
        cust_ic_setting->pmic_info.with_nvm = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"pmic_i2c_bus") != NULL)
    {
        cust_ic_setting->pmic_info.i2c_bus = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"pmic_i2c_mode") != NULL)
    {
        cust_ic_setting->pmic_info.i2c_mode = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"pmic_i2c_dev_addr") != NULL)
    {
        cust_ic_setting->pmic_info.i2c_dev_addr = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"pmic_i2c_reg_offst") != NULL)
    {
        cust_ic_setting->pmic_info.i2c_reg_offset = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"pmic_pre_vcc_dly") != NULL)
    {
        cust_ic_setting->pmic_info.vcc_dly = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"pmic_pre_gpio_num") != NULL)
    {
        cust_ic_setting->pmic_info.gpio_pre_num = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"pmic_pre_gpio_ops") != NULL)
    {
        cust_ic_setting->pmic_info.gpio_pre_ops = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"pmic_pre_delay") != NULL)
    {
        cust_ic_setting->pmic_info.gpio_pre_dly = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"pmic_post_gpio_num") != NULL)
    {
        cust_ic_setting->pmic_info.gpio_post_num = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"pmic_post_gpio_ops") != NULL)
    {
        cust_ic_setting->pmic_info.gpio_post_ops = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"pmic_post_delay") != NULL)
    {
        cust_ic_setting->pmic_info.gpio_post_dly = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"pmic_auto_from_flash") != NULL)
    {
        cust_ic_setting->pmic_info.auto_update_from_flash = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if (strstr(fdt_data->field, "pmic_i2c_read_mode") != NULL)
    {
        cust_ic_setting->pmic_info.read_mode= fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if (strstr(fdt_data->field, "pmic_i2c_write_mode") != NULL)
    {
        cust_ic_setting->pmic_info.write_mode= fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if (strstr(fdt_data->field, "pimc_i2c_burn_cmd") != NULL)
    {
        cust_ic_setting->pmic_info.i2c_burn_cmd = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if (strstr(fdt_data->field, "pimc_i2c_burn_offset") != NULL)
    {
        cust_ic_setting->pmic_info.i2c_burn_offset = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if (strstr(fdt_data->field, "pmic_i2c_ctrl_reg") != NULL)
    {
        cust_ic_setting->pmic_info.i2c_ctrl_reg = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if (strstr(fdt_data->field, "pmic_i2c_ctrl_reg_offset") != NULL)
    {
        cust_ic_setting->pmic_info.i2c_ctrl_reg_offset = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if (strstr(fdt_data->field, "pimc_data_start") != NULL)
    {
        cust_ic_setting->pmic_info.data_start = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if (strstr(fdt_data->field, "pimc_data_end") != NULL)
    {
        cust_ic_setting->pmic_info.data_end = fdt32_to_cpu(fdt_data->attribute[0]);
    }

    if(strstr(fdt_data->field,"pmic_vcom_override_enable") != NULL)
    {
        cust_ic_setting->pmic_vcom_info.override_enable = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"pmic_vcom_addr_start") != NULL)
    {
        cust_ic_setting->pmic_vcom_info.addr_start = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"pmic_vcom_addr_length") != NULL)
    {
        cust_ic_setting->pmic_vcom_info.addr_length = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"pmic_nvm_chk_en") != NULL)
    {
        cust_ic_setting->pmic_info.nvm_chk_en = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"pmic_nvm_chk_offset") != NULL)
    {
        cust_ic_setting->pmic_info.nvm_chk_offset = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"pmic_nvm_chk_val") != NULL)
    {
        cust_ic_setting->pmic_info.nvm_chk_val = fdt32_to_cpu(fdt_data->attribute[0]);
    }
	if(strstr(fdt_data->field,"pmic_nvm_chk_i2c_post_dly") != NULL)
    {
        cust_ic_setting->pmic_info.nvm_chk_i2c_post_dly = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"pmic_nvm_chk_rst_dly") != NULL)
    {
        cust_ic_setting->pmic_info.nvm_chk_rst_dly = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    return;
}

void cust_pgamma_dt_parser(struct fdt_content *fdt_data, void *out, const char *field_target)
{
    st_multi_cust_ic_info *cust_ic_setting = (st_multi_cust_ic_info*)out;
    if (!cust_ic_setting || !out)
        return;

    if(strstr(fdt_data->field,"pgamma_info_version") != NULL)
    {
        cust_ic_setting->pgamma_info.info_version = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"pgamma_info_length") != NULL)
    {
        cust_ic_setting->pgamma_info.info_length = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"pgamma_type") != NULL)
    {
        cust_ic_setting->pgamma_info.ic_type = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"pgamma_bin_file_path") != NULL)
    {
        cust_ic_setting->pgamma_info.ic_bin_file_path = malloc(MAX_CUST_PATH_LEN);
        if (cust_ic_setting->pgamma_info.ic_bin_file_path != NULL) {
            memset(cust_ic_setting->pgamma_info.ic_bin_file_path, 0x00, MAX_CUST_PATH_LEN);
            memcpy(cust_ic_setting->pgamma_info.ic_bin_file_path, fdt_data->attribute, fdt_data->len);
        }
    }
    if(strstr(fdt_data->field,"pgamma_checksum_bypass_offset") != NULL)
    {
        if (fdt_data->len > 0)
        {
            int array_size = fdt_data->len/sizeof(u32);
            cust_ic_setting->pgamma_info.checksum_bypass_offset = malloc(fdt_data->len);
            if(cust_ic_setting->pgamma_info.checksum_bypass_offset != NULL) {
                memset(cust_ic_setting->pgamma_info.checksum_bypass_offset, 0, fdt_data->len);
                for (int i=0; i<array_size;i++)
                {
                    cust_ic_setting->pgamma_info.checksum_bypass_offset[i] = fdt32_to_cpu(fdt_data->attribute[i]);
                }
            }
            cust_ic_setting->pgamma_info.checksum_bypass_size = array_size;
        }
        else {
            cust_ic_setting->pgamma_info.checksum_bypass_size = 0;
        }
    }

    if(strstr(fdt_data->field,"pgamma_bin_format_type") != NULL)
    {
        cust_ic_setting->pgamma_info.bin_format_type = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"pgamma_force_init") != NULL)
    {
        cust_ic_setting->pgamma_info.force_init = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"pgamma_with_nvm") != NULL)
    {
        cust_ic_setting->pgamma_info.with_nvm = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"pgamma_i2c_bus") != NULL)
    {
        cust_ic_setting->pgamma_info.i2c_bus = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"pgamma_i2c_mode") != NULL)
    {
        cust_ic_setting->pgamma_info.i2c_mode = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"pgamma_i2c_dev_addr") != NULL)
    {
        cust_ic_setting->pgamma_info.i2c_dev_addr = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"pgamma_i2c_reg_offst") != NULL)
    {
        cust_ic_setting->pgamma_info.i2c_reg_offset = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"pgamma_pre_vcc_dly") != NULL)
    {
        cust_ic_setting->pgamma_info.vcc_dly = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"pgamma_pre_gpio_num") != NULL)
    {
        cust_ic_setting->pgamma_info.gpio_pre_num = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"pgamma_pre_gpio_ops") != NULL)
    {
        cust_ic_setting->pgamma_info.gpio_pre_ops = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"pgamma_pre_delay") != NULL)
    {
        cust_ic_setting->pgamma_info.gpio_pre_dly = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"pgamma_post_gpio_num") != NULL)
    {
        cust_ic_setting->pgamma_info.gpio_post_num = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"pgamma_post_gpio_ops") != NULL)
    {
        cust_ic_setting->pgamma_info.gpio_post_ops = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"pgamma_post_delay") != NULL)
    {
        cust_ic_setting->pgamma_info.gpio_post_dly = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"pgamma_auto_from_flash") != NULL)
    {
        cust_ic_setting->pgamma_info.auto_update_from_flash = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if (strstr(fdt_data->field, "pgamma_i2c_read_mode") != NULL)
    {
        cust_ic_setting->pgamma_info.read_mode= fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if (strstr(fdt_data->field, "pgamma_i2c_write_mode") != NULL)
    {
        cust_ic_setting->pgamma_info.write_mode= fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if (strstr(fdt_data->field, "pgamma_i2c_burn_cmd") != NULL)
    {
        cust_ic_setting->pgamma_info.i2c_burn_cmd = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if (strstr(fdt_data->field, "pgamma_i2c_burn_offset") != NULL)
    {
        cust_ic_setting->pgamma_info.i2c_burn_offset = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if (strstr(fdt_data->field, "pgamma_i2c_ctrl_reg") != NULL)
    {
        cust_ic_setting->pgamma_info.i2c_ctrl_reg = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if (strstr(fdt_data->field, "pgamma_i2c_ctrl_reg_offset") != NULL)
    {
        cust_ic_setting->pgamma_info.i2c_ctrl_reg_offset = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if (strstr(fdt_data->field, "pgamma_data_start") != NULL)
    {
        cust_ic_setting->pgamma_info.data_start = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if (strstr(fdt_data->field, "pgamma_data_end") != NULL)
    {
        cust_ic_setting->pgamma_info.data_end = fdt32_to_cpu(fdt_data->attribute[0]);
    }

    return;
}

void cust_levelshift_dt_parser(struct fdt_content *fdt_data, void *out, const char *field_target)
{
    st_multi_cust_ic_info *cust_ic_setting = (st_multi_cust_ic_info*)out;
    if (!cust_ic_setting || !out)
        return;

    if(strstr(fdt_data->field,"levelshift_info_version") != NULL)
    {
        cust_ic_setting->levelshift_info.info_version = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"levelshift_info_length") != NULL)
    {
        cust_ic_setting->levelshift_info.info_length = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"levelshift_type") != NULL)
    {
        cust_ic_setting->levelshift_info.ic_type = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"levelshift_bin_file_path") != NULL)
    {
        cust_ic_setting->levelshift_info.ic_bin_file_path = malloc(MAX_CUST_PATH_LEN);
        if (cust_ic_setting->levelshift_info.ic_bin_file_path != NULL) {
            memset(cust_ic_setting->levelshift_info.ic_bin_file_path, 0x00, MAX_CUST_PATH_LEN);
            memcpy(cust_ic_setting->levelshift_info.ic_bin_file_path, fdt_data->attribute, fdt_data->len);
        }
    }
    if(strstr(fdt_data->field,"levelshift_checksum_bypass_offset") != NULL)
    {
        if (fdt_data->len > 0)
        {
            int array_size = fdt_data->len/sizeof(u32);
            cust_ic_setting->levelshift_info.checksum_bypass_offset = malloc(fdt_data->len);
            if(cust_ic_setting->levelshift_info.checksum_bypass_offset != NULL) {
                memset(cust_ic_setting->levelshift_info.checksum_bypass_offset, 0, fdt_data->len);
                for (int i=0; i<array_size;i++)
                {
                    cust_ic_setting->levelshift_info.checksum_bypass_offset[i] = fdt32_to_cpu(fdt_data->attribute[i]);
                }
            }
            cust_ic_setting->levelshift_info.checksum_bypass_size = array_size;
        }
        else {
            cust_ic_setting->levelshift_info.checksum_bypass_size = 0;
        }
    }
    if(strstr(fdt_data->field,"levelshift_bin_format_type") != NULL)
    {
        cust_ic_setting->levelshift_info.bin_format_type = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"levelshift_force_init") != NULL)
    {
        cust_ic_setting->levelshift_info.force_init = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"levelshift_with_nvm") != NULL)
    {
        cust_ic_setting->levelshift_info.with_nvm = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"levelshift_i2c_bus") != NULL)
    {
        cust_ic_setting->levelshift_info.i2c_bus = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"levelshift_i2c_mode") != NULL)
    {
        cust_ic_setting->levelshift_info.i2c_mode = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"levelshift_i2c_dev_addr") != NULL)
    {
        cust_ic_setting->levelshift_info.i2c_dev_addr = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"levelshift_i2c_reg_offst") != NULL)
    {
        cust_ic_setting->levelshift_info.i2c_reg_offset = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"levelshift_pre_vcc_dly") != NULL)
    {
        cust_ic_setting->levelshift_info.vcc_dly = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"levelshift_pre_gpio_num") != NULL)
    {
        cust_ic_setting->levelshift_info.gpio_pre_num = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"levelshift_pre_gpio_ops") != NULL)
    {
        cust_ic_setting->levelshift_info.gpio_pre_ops = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"levelshift_pre_delay") != NULL)
    {
        cust_ic_setting->levelshift_info.gpio_pre_dly = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"levelshift_post_gpio_num") != NULL)
    {
        cust_ic_setting->levelshift_info.gpio_post_num = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"levelshift_post_gpio_ops") != NULL)
    {
        cust_ic_setting->levelshift_info.gpio_post_ops = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"levelshift_post_delay") != NULL)
    {
        cust_ic_setting->levelshift_info.gpio_post_dly = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"levelshift_auto_from_flash") != NULL)
    {
        cust_ic_setting->levelshift_info.auto_update_from_flash = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if (strstr(fdt_data->field, "levelshift_i2c_read_mode") != NULL)
    {
        cust_ic_setting->levelshift_info.read_mode= fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if (strstr(fdt_data->field, "levelshift_i2c_write_mode") != NULL)
    {
        cust_ic_setting->levelshift_info.write_mode= fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if (strstr(fdt_data->field, "levelshift_i2c_burn_cmd") != NULL)
    {
        cust_ic_setting->levelshift_info.i2c_burn_cmd = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if (strstr(fdt_data->field, "levelshift_i2c_burn_offset") != NULL)
    {
        cust_ic_setting->levelshift_info.i2c_burn_offset = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if (strstr(fdt_data->field, "levelshift_i2c_ctrl_reg") != NULL)
    {
        cust_ic_setting->levelshift_info.i2c_ctrl_reg = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if (strstr(fdt_data->field, "levelshift_i2c_ctrl_reg_offset") != NULL)
    {
        cust_ic_setting->levelshift_info.i2c_ctrl_reg_offset = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if (strstr(fdt_data->field, "levelshift_data_start") != NULL)
    {
        cust_ic_setting->levelshift_info.data_start = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if (strstr(fdt_data->field, "levelshift_data_end") != NULL)
    {
        cust_ic_setting->levelshift_info.data_end = fdt32_to_cpu(fdt_data->attribute[0]);
    }

    return;
}

void cust_vcomic_dt_parser(struct fdt_content *fdt_data, void *out, const char *field_target)
{
    st_multi_cust_ic_info *cust_ic_setting = (st_multi_cust_ic_info*)out;
    if (!cust_ic_setting || !out)
        return;

    if(strstr(fdt_data->field,"vcomic_info_version") != NULL)
    {
        cust_ic_setting->vcomic_info.info_version = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"vcomic_info_length") != NULL)
    {
        cust_ic_setting->vcomic_info.info_length = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"vcomic_type") != NULL)
    {
        cust_ic_setting->vcomic_info.ic_type = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"vcomic_bin_file_path") != NULL)
    {
        cust_ic_setting->vcomic_info.ic_bin_file_path = malloc(MAX_CUST_PATH_LEN);
        if (cust_ic_setting->vcomic_info.ic_bin_file_path != NULL) {
            memset(cust_ic_setting->vcomic_info.ic_bin_file_path, 0x00, MAX_CUST_PATH_LEN);
            memcpy(cust_ic_setting->vcomic_info.ic_bin_file_path, fdt_data->attribute, fdt_data->len);
        }
    }
    if(strstr(fdt_data->field,"vcomic_checksum_bypass_offset") != NULL)
    {
        if (fdt_data->len > 0)
        {
            int array_size = fdt_data->len/sizeof(u32);
            cust_ic_setting->vcomic_info.checksum_bypass_offset = malloc(fdt_data->len);
            if(cust_ic_setting->vcomic_info.checksum_bypass_offset != NULL) {
                memset(cust_ic_setting->vcomic_info.checksum_bypass_offset, 0, fdt_data->len);
                for (int i=0; i<array_size;i++)
                {
                    cust_ic_setting->vcomic_info.checksum_bypass_offset[i] = fdt32_to_cpu(fdt_data->attribute[i]);
                }
            }
            cust_ic_setting->vcomic_info.checksum_bypass_size = array_size;
        }
        else {
            cust_ic_setting->vcomic_info.checksum_bypass_size = 0;
        }
    }
    if(strstr(fdt_data->field,"vcomic_bin_format_type") != NULL)
    {
        cust_ic_setting->vcomic_info.bin_format_type = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"vcomic_force_init") != NULL)
    {
        cust_ic_setting->vcomic_info.force_init = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"vcomic_with_nvm") != NULL)
    {
        cust_ic_setting->vcomic_info.with_nvm = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"vcomic_i2c_bus") != NULL)
    {
        cust_ic_setting->vcomic_info.i2c_bus = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"vcomic_i2c_mode") != NULL)
    {
        cust_ic_setting->vcomic_info.i2c_mode = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"vcomic_i2c_dev_addr") != NULL)
    {
        cust_ic_setting->vcomic_info.i2c_dev_addr = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"vcomic_i2c_reg_offst") != NULL)
    {
        cust_ic_setting->vcomic_info.i2c_reg_offset = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"vcomic_pre_vcc_dly") != NULL)
    {
        cust_ic_setting->vcomic_info.vcc_dly = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"vcomic_pre_gpio_num") != NULL)
    {
        cust_ic_setting->vcomic_info.gpio_pre_num = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"vcomic_pre_gpio_ops") != NULL)
    {
        cust_ic_setting->vcomic_info.gpio_pre_ops = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"vcomic_pre_delay") != NULL)
    {
        cust_ic_setting->vcomic_info.gpio_pre_dly = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"vcomic_post_gpio_num") != NULL)
    {
        cust_ic_setting->vcomic_info.gpio_post_num = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"vcomic_post_gpio_ops") != NULL)
    {
        cust_ic_setting->vcomic_info.gpio_post_ops = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"vcomic_post_delay") != NULL)
    {
        cust_ic_setting->vcomic_info.gpio_post_dly = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"vcomic_auto_from_flash") != NULL)
    {
        cust_ic_setting->vcomic_info.auto_update_from_flash = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if (strstr(fdt_data->field, "vcomic_i2c_read_mode") != NULL)
    {
        cust_ic_setting->vcomic_info.read_mode= fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if (strstr(fdt_data->field, "vcomic_i2c_write_mode") != NULL)
    {
        cust_ic_setting->vcomic_info.write_mode= fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if (strstr(fdt_data->field, "vcomic_i2c_burn_cmd") != NULL)
    {
        cust_ic_setting->vcomic_info.i2c_burn_cmd = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if (strstr(fdt_data->field, "vcomic_i2c_burn_offset") != NULL)
    {
        cust_ic_setting->vcomic_info.i2c_burn_offset = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if (strstr(fdt_data->field, "vcomic_i2c_ctrl_reg") != NULL)
    {
        cust_ic_setting->vcomic_info.i2c_ctrl_reg = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if (strstr(fdt_data->field, "vcomic_i2c_ctrl_reg_offset") != NULL)
    {
        cust_ic_setting->vcomic_info.i2c_ctrl_reg_offset = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if (strstr(fdt_data->field, "vcomic_data_start") != NULL)
    {
        cust_ic_setting->vcomic_info.data_start = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if (strstr(fdt_data->field, "vcomic_data_end") != NULL)
    {
        cust_ic_setting->vcomic_info.data_end = fdt32_to_cpu(fdt_data->attribute[0]);
    }

    return;
}

void cust_pmic_sub_dt_parser_part1(st_multi_cust_ic_info *cust_ic_setting, struct fdt_content *fdt_data)
{
    if (!cust_ic_setting || !fdt_data)
    {
        return;
    }

    if (strstr(fdt_data->field, "pmic_sub_info_version") != NULL)
    {
        cust_ic_setting->pmic_sub_info.info_version = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if (strstr(fdt_data->field, "pmic_sub_info_length") != NULL)
    {
        cust_ic_setting->pmic_sub_info.info_length = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if (strstr(fdt_data->field, "pmic_sub_type") != NULL)
    {
        cust_ic_setting->pmic_sub_info.ic_type = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if (strstr(fdt_data->field, "pmic_sub_bin_file_path") != NULL)
    {
        cust_ic_setting->pmic_sub_info.ic_bin_file_path = malloc(MAX_CUST_PATH_LEN);
        if (cust_ic_setting->pmic_sub_info.ic_bin_file_path != NULL)
        {
            memset(cust_ic_setting->pmic_sub_info.ic_bin_file_path, 0x00, MAX_CUST_PATH_LEN);
            memcpy(cust_ic_setting->pmic_sub_info.ic_bin_file_path, fdt_data->attribute, fdt_data->len);
        }
    }
    if(strstr(fdt_data->field,"pmic_sub_checksum_bypass_offset") != NULL)
    {
        if (fdt_data->len > 0)
        {
            int array_size = fdt_data->len/sizeof(u32);
            cust_ic_setting->pmic_sub_info.checksum_bypass_offset = malloc(fdt_data->len);
            if(cust_ic_setting->pmic_sub_info.checksum_bypass_offset != NULL) {
                memset(cust_ic_setting->pmic_sub_info.checksum_bypass_offset, 0, fdt_data->len);
                for (int i=0; i<array_size;i++)
                {
                    cust_ic_setting->pmic_sub_info.checksum_bypass_offset[i] = fdt32_to_cpu(fdt_data->attribute[i]);
                }
            }
            cust_ic_setting->pmic_sub_info.checksum_bypass_size = array_size;
        }
        else {
            cust_ic_setting->pmic_sub_info.checksum_bypass_size = 0;
        }
    }

    if (strstr(fdt_data->field, "pmic_sub_bin_format_type") != NULL)
    {
        cust_ic_setting->pmic_sub_info.bin_format_type = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if (strstr(fdt_data->field, "pmic_sub_force_init") != NULL)
    {
        cust_ic_setting->pmic_sub_info.force_init = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if (strstr(fdt_data->field, "pmic_sub_with_nvm") != NULL)
    {
        cust_ic_setting->pmic_sub_info.with_nvm = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if (strstr(fdt_data->field, "pmic_sub_i2c_bus") != NULL)
    {
        cust_ic_setting->pmic_sub_info.i2c_bus = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if (strstr(fdt_data->field, "pmic_sub_i2c_mode") != NULL)
    {
        cust_ic_setting->pmic_sub_info.i2c_mode = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if (strstr(fdt_data->field, "pmic_sub_i2c_dev_addr") != NULL)
    {
        cust_ic_setting->pmic_sub_info.i2c_dev_addr = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if (strstr(fdt_data->field, "pmic_sub_i2c_reg_offst") != NULL)
    {
        cust_ic_setting->pmic_sub_info.i2c_reg_offset = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if (strstr(fdt_data->field, "pmic_pre_vcc_dly") != NULL)
    {
        cust_ic_setting->pmic_sub_info.vcc_dly = fdt32_to_cpu(fdt_data->attribute[0]);
    }
}

void cust_pmic_sub_dt_parser_part2(st_multi_cust_ic_info *cust_ic_setting, struct fdt_content *fdt_data)
{
    if (!cust_ic_setting || !fdt_data)
    {
        return;
    }

    if (strstr(fdt_data->field, "pmic_sub_pre_gpio_num") != NULL)
    {
        cust_ic_setting->pmic_sub_info.gpio_pre_num = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if (strstr(fdt_data->field, "pmic_sub_pre_gpio_ops") != NULL)
    {
        cust_ic_setting->pmic_sub_info.gpio_pre_ops = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if (strstr(fdt_data->field, "pmic_sub_pre_delay") != NULL)
    {
        cust_ic_setting->pmic_sub_info.gpio_pre_dly = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if (strstr(fdt_data->field, "pmic_sub_post_gpio_num") != NULL)
    {
        cust_ic_setting->pmic_sub_info.gpio_post_num = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if (strstr(fdt_data->field, "pmic_sub_post_gpio_ops") != NULL)
    {
        cust_ic_setting->pmic_sub_info.gpio_post_ops = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if (strstr(fdt_data->field, "pmic_sub_post_delay") != NULL)
    {
        cust_ic_setting->pmic_sub_info.gpio_post_dly = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if (strstr(fdt_data->field, "pmic_sub_auto_from_flash") != NULL)
    {
        cust_ic_setting->pmic_sub_info.auto_update_from_flash = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if (strstr(fdt_data->field, "pmic_sub_i2c_read_mode") != NULL)
    {
        cust_ic_setting->pmic_sub_info.read_mode= fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if (strstr(fdt_data->field, "pmic_sub_i2c_write_mode") != NULL)
    {
        cust_ic_setting->pmic_sub_info.write_mode= fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if (strstr(fdt_data->field, "second_pimc_i2c_burn_cmd") != NULL)
    {
        cust_ic_setting->pmic_sub_info.i2c_burn_cmd = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if (strstr(fdt_data->field, "second_pimc_i2c_burn_offset") != NULL)
    {
        cust_ic_setting->pmic_sub_info.i2c_burn_offset = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if (strstr(fdt_data->field, "pmic_sub_i2c_ctrl_reg") != NULL)
    {
        cust_ic_setting->pmic_sub_info.i2c_ctrl_reg = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if (strstr(fdt_data->field, "pmic_sub_i2c_ctrl_reg_offset") != NULL)
    {
        cust_ic_setting->pmic_sub_info.i2c_ctrl_reg_offset = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if (strstr(fdt_data->field, "pmic_sub_data_start") != NULL)
    {
        cust_ic_setting->pmic_sub_info.data_start = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if (strstr(fdt_data->field, "pmic_sub_data_end") != NULL)
    {
        cust_ic_setting->pmic_sub_info.data_end = fdt32_to_cpu(fdt_data->attribute[0]);
    }
}

void cust_pmic_sub_dt_parser(struct fdt_content *fdt_data, void *out, const char *field_target)
{
    st_multi_cust_ic_info *cust_ic_setting = (st_multi_cust_ic_info*)out;
    if (!cust_ic_setting || !out)
    {
        return;
    }

    cust_pmic_sub_dt_parser_part1(cust_ic_setting, fdt_data);
    cust_pmic_sub_dt_parser_part2(cust_ic_setting, fdt_data);

    return;
}

void cust_pgamma_sub_dt_parser_part1(st_multi_cust_ic_info *cust_ic_setting, struct fdt_content *fdt_data)
{
    if (!cust_ic_setting || !fdt_data)
    {
        return;
    }

    if (strstr(fdt_data->field, "pgamma_sub_info_version") != NULL)
    {
        cust_ic_setting->pgamma_sub_info.info_version = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if (strstr(fdt_data->field, "pgamma_sub_info_length") != NULL)
    {
        cust_ic_setting->pgamma_sub_info.info_length = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if (strstr(fdt_data->field, "pgamma_sub_type") != NULL)
    {
        cust_ic_setting->pgamma_sub_info.ic_type = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if (strstr(fdt_data->field, "pgamma_sub_bin_file_path") != NULL)
    {
        cust_ic_setting->pgamma_sub_info.ic_bin_file_path = malloc(MAX_CUST_PATH_LEN);
        if (cust_ic_setting->pgamma_sub_info.ic_bin_file_path != NULL)
        {
            memset(cust_ic_setting->pgamma_sub_info.ic_bin_file_path, 0x00, MAX_CUST_PATH_LEN);
            memcpy(cust_ic_setting->pgamma_sub_info.ic_bin_file_path, fdt_data->attribute, fdt_data->len);
        }
    }
    if(strstr(fdt_data->field,"pgamma_sub_checksum_bypass_offset") != NULL)
    {
        if (fdt_data->len > 0)
        {
            int array_size = fdt_data->len/sizeof(u32);
            cust_ic_setting->pgamma_sub_info.checksum_bypass_offset = malloc(fdt_data->len);
            if(cust_ic_setting->pgamma_sub_info.checksum_bypass_offset != NULL) {
                memset(cust_ic_setting->pgamma_sub_info.checksum_bypass_offset, 0, fdt_data->len);
                for (int i=0; i<array_size;i++)
                {
                    cust_ic_setting->pgamma_sub_info.checksum_bypass_offset[i] = fdt32_to_cpu(fdt_data->attribute[i]);
                }
            }
            cust_ic_setting->pgamma_sub_info.checksum_bypass_size = array_size;
        }
        else {
            cust_ic_setting->pgamma_sub_info.checksum_bypass_size = 0;
        }
    }

    if (strstr(fdt_data->field, "pgamma_sub_bin_format_type") != NULL)
    {
        cust_ic_setting->pgamma_sub_info.bin_format_type = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if (strstr(fdt_data->field, "pgamma_sub_force_init") != NULL)
    {
        cust_ic_setting->pgamma_sub_info.force_init = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if (strstr(fdt_data->field, "pgamma_sub_with_nvm") != NULL)
    {
        cust_ic_setting->pgamma_sub_info.with_nvm = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if (strstr(fdt_data->field, "pgamma_sub_i2c_bus") != NULL)
    {
        cust_ic_setting->pgamma_sub_info.i2c_bus = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if (strstr(fdt_data->field, "pgamma_sub_i2c_mode") != NULL)
    {
        cust_ic_setting->pgamma_sub_info.i2c_mode = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if (strstr(fdt_data->field, "pgamma_sub_i2c_dev_addr") != NULL)
    {
        cust_ic_setting->pgamma_sub_info.i2c_dev_addr = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if (strstr(fdt_data->field, "pgamma_sub_i2c_reg_offst") != NULL)
    {
        cust_ic_setting->pgamma_sub_info.i2c_reg_offset = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if (strstr(fdt_data->field, "pgamma_sub_pre_vcc_dly") != NULL)
    {
        cust_ic_setting->pgamma_sub_info.vcc_dly = fdt32_to_cpu(fdt_data->attribute[0]);
    }
}

void cust_pgamma_sub_dt_parser_part2(st_multi_cust_ic_info *cust_ic_setting, struct fdt_content *fdt_data)
{
    if (!cust_ic_setting || !fdt_data)
    {
        return;
    }

    if (strstr(fdt_data->field, "pgamma_sub_pre_gpio_num") != NULL)
    {
        cust_ic_setting->pgamma_sub_info.gpio_pre_num = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if (strstr(fdt_data->field, "pgamma_sub_pre_gpio_ops") != NULL)
    {
        cust_ic_setting->pgamma_sub_info.gpio_pre_ops = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if (strstr(fdt_data->field, "pgamma_sub_pre_delay") != NULL)
    {
        cust_ic_setting->pgamma_sub_info.gpio_pre_dly = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if (strstr(fdt_data->field, "pgamma_sub_post_gpio_num") != NULL)
    {
        cust_ic_setting->pgamma_sub_info.gpio_post_num = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if (strstr(fdt_data->field, "pgamma_sub_post_gpio_ops") != NULL)
    {
        cust_ic_setting->pgamma_sub_info.gpio_post_ops = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if (strstr(fdt_data->field, "pgamma_sub_post_delay") != NULL)
    {
        cust_ic_setting->pgamma_sub_info.gpio_post_dly = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if (strstr(fdt_data->field, "pgamma_sub_auto_from_flash") != NULL)
    {
        cust_ic_setting->pgamma_sub_info.auto_update_from_flash = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if (strstr(fdt_data->field, "pgamma_sub_i2c_read_mode") != NULL)
    {
        cust_ic_setting->pgamma_sub_info.read_mode = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if (strstr(fdt_data->field, "pgamma_sub_i2c_write_mode") != NULL)
    {
        cust_ic_setting->pgamma_sub_info.write_mode = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if (strstr(fdt_data->field, "pgamma_sub_i2c_burn_cmd") != NULL)
    {
        cust_ic_setting->pgamma_sub_info.i2c_burn_cmd = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if (strstr(fdt_data->field, "pgamma_sub_i2c_burn_offset") != NULL)
    {
        cust_ic_setting->pgamma_sub_info.i2c_burn_offset = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if (strstr(fdt_data->field, "pgamma_sub_i2c_ctrl_reg") != NULL)
    {
        cust_ic_setting->pgamma_sub_info.i2c_ctrl_reg = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if (strstr(fdt_data->field, "pgamma_sub_i2c_ctrl_reg_offset") != NULL)
    {
        cust_ic_setting->pgamma_sub_info.i2c_ctrl_reg_offset = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if (strstr(fdt_data->field, "pgamma_sub_data_start") != NULL)
    {
        cust_ic_setting->pgamma_sub_info.data_start = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if (strstr(fdt_data->field, "pgamma_sub_data_end") != NULL)
    {
        cust_ic_setting->pgamma_sub_info.data_end = fdt32_to_cpu(fdt_data->attribute[0]);
    }
}

void cust_pgamma_sub_dt_parser(struct fdt_content *fdt_data, void *out, const char *field_target)
{
    st_multi_cust_ic_info *cust_ic_setting = (st_multi_cust_ic_info*)out;
    if (!cust_ic_setting || !out)
    {
        return;
    }

    cust_pgamma_sub_dt_parser_part1(cust_ic_setting, fdt_data);
    cust_pgamma_sub_dt_parser_part2(cust_ic_setting, fdt_data);

    return;
}

void cust_levelshift_sub_dt_parser_part1(st_multi_cust_ic_info *cust_ic_setting, struct fdt_content *fdt_data)
{
    if (!cust_ic_setting || !fdt_data)
    {
        return;
    }

    if (strstr(fdt_data->field, "levelshift_sub_info_version") != NULL)
    {
        cust_ic_setting->levelshift_sub_info.info_version = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if (strstr(fdt_data->field, "levelshift_sub_info_length") != NULL)
    {
        cust_ic_setting->levelshift_sub_info.info_length = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if (strstr(fdt_data->field, "levelshift_sub_type") != NULL)
    {
        cust_ic_setting->levelshift_sub_info.ic_type = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if (strstr(fdt_data->field, "levelshift_sub_bin_file_path") != NULL)
    {
        cust_ic_setting->levelshift_sub_info.ic_bin_file_path = malloc(MAX_CUST_PATH_LEN);
        if (cust_ic_setting->levelshift_sub_info.ic_bin_file_path != NULL)
        {
            memset(cust_ic_setting->levelshift_sub_info.ic_bin_file_path, 0x00, MAX_CUST_PATH_LEN);
            memcpy(cust_ic_setting->levelshift_sub_info.ic_bin_file_path, fdt_data->attribute, fdt_data->len);
        }
    }
    if(strstr(fdt_data->field,"levelshift_sub_checksum_bypass_offset") != NULL)
    {
        if (fdt_data->len > 0)
        {
            int array_size = fdt_data->len/sizeof(u32);
            cust_ic_setting->levelshift_sub_info.checksum_bypass_offset = malloc(fdt_data->len);
            if(cust_ic_setting->levelshift_sub_info.checksum_bypass_offset != NULL) {
                memset(cust_ic_setting->levelshift_sub_info.checksum_bypass_offset, 0, fdt_data->len);
                for (int i=0; i<array_size;i++)
                {
                    cust_ic_setting->levelshift_sub_info.checksum_bypass_offset[i] = fdt32_to_cpu(fdt_data->attribute[i]);
                }
            }
            cust_ic_setting->levelshift_sub_info.checksum_bypass_size = array_size;
        }
        else {
            cust_ic_setting->levelshift_sub_info.checksum_bypass_size = 0;
        }
    }
    if (strstr(fdt_data->field, "levelshift_sub_bin_format_type") != NULL)
    {
        cust_ic_setting->levelshift_sub_info.bin_format_type = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if (strstr(fdt_data->field, "levelshift_sub_force_init") != NULL)
    {
        cust_ic_setting->levelshift_sub_info.force_init = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if (strstr(fdt_data->field, "levelshift_sub_with_nvm") != NULL)
    {
        cust_ic_setting->levelshift_sub_info.with_nvm = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if (strstr(fdt_data->field, "levelshift_sub_i2c_bus") != NULL)
    {
        cust_ic_setting->levelshift_sub_info.i2c_bus = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if (strstr(fdt_data->field, "levelshift_sub_i2c_mode") != NULL)
    {
        cust_ic_setting->levelshift_sub_info.i2c_mode = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if (strstr(fdt_data->field, "levelshift_sub_i2c_dev_addr") != NULL)
    {
        cust_ic_setting->levelshift_sub_info.i2c_dev_addr = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if (strstr(fdt_data->field, "levelshift_sub_i2c_reg_offst") != NULL)
    {
        cust_ic_setting->levelshift_sub_info.i2c_reg_offset = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if (strstr(fdt_data->field, "levelshift_sub_pre_vcc_dly") != NULL)
    {
        cust_ic_setting->levelshift_sub_info.vcc_dly = fdt32_to_cpu(fdt_data->attribute[0]);
    }
}

void cust_levelshift_sub_dt_parser_part2(st_multi_cust_ic_info *cust_ic_setting, struct fdt_content *fdt_data)
{
    if (!cust_ic_setting || !fdt_data)
    {
        return;
    }

    if (strstr(fdt_data->field, "levelshift_sub_pre_gpio_num") != NULL)
    {
        cust_ic_setting->levelshift_sub_info.gpio_pre_num = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if (strstr(fdt_data->field, "levelshift_sub_pre_gpio_ops") != NULL)
    {
        cust_ic_setting->levelshift_sub_info.gpio_pre_ops = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if (strstr(fdt_data->field, "levelshift_sub_pre_delay") != NULL)
    {
        cust_ic_setting->levelshift_sub_info.gpio_pre_dly = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if (strstr(fdt_data->field, "levelshift_sub_post_gpio_num") != NULL)
    {
        cust_ic_setting->levelshift_sub_info.gpio_post_num = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if (strstr(fdt_data->field, "levelshift_sub_post_gpio_ops") != NULL)
    {
        cust_ic_setting->levelshift_sub_info.gpio_post_ops = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if (strstr(fdt_data->field, "levelshift_sub_post_delay") != NULL)
    {
        cust_ic_setting->levelshift_sub_info.gpio_post_dly = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if (strstr(fdt_data->field, "levelshift_sub_auto_from_flash") != NULL)
    {
        cust_ic_setting->levelshift_sub_info.auto_update_from_flash = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if (strstr(fdt_data->field, "levelshift_sub_i2c_read_mode") != NULL)
    {
        cust_ic_setting->levelshift_sub_info.read_mode = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if (strstr(fdt_data->field, "levelshift_sub_i2c_write_mode") != NULL)
    {
        cust_ic_setting->levelshift_sub_info.write_mode = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if (strstr(fdt_data->field, "levelshift_sub_i2c_burn_cmd") != NULL)
    {
        cust_ic_setting->levelshift_sub_info.i2c_burn_cmd = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if (strstr(fdt_data->field, "levelshift_sub_i2c_burn_offset") != NULL)
    {
        cust_ic_setting->levelshift_sub_info.i2c_burn_offset = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if (strstr(fdt_data->field, "levelshift_sub_i2c_ctrl_reg") != NULL)
    {
        cust_ic_setting->levelshift_sub_info.i2c_ctrl_reg = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if (strstr(fdt_data->field, "levelshift_sub_i2c_ctrl_reg_offset") != NULL)
    {
        cust_ic_setting->levelshift_sub_info.i2c_ctrl_reg_offset = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if (strstr(fdt_data->field, "levelshift_sub_data_start") != NULL)
    {
        cust_ic_setting->levelshift_sub_info.data_start = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if (strstr(fdt_data->field, "levelshift_sub_data_end") != NULL)
    {
        cust_ic_setting->levelshift_sub_info.data_end = fdt32_to_cpu(fdt_data->attribute[0]);
    }
}

void cust_levelshift_sub_dt_parser(struct fdt_content *fdt_data, void *out, const char *field_target)
{
    st_multi_cust_ic_info *cust_ic_setting = (st_multi_cust_ic_info*)out;
    if (!cust_ic_setting || !out)
    {
        return;
    }

    cust_levelshift_sub_dt_parser_part1(cust_ic_setting, fdt_data);
    cust_levelshift_sub_dt_parser_part2(cust_ic_setting, fdt_data);

    return;
}

void cust_vcomic_sub_dt_parser_part1(st_multi_cust_ic_info *cust_ic_setting, struct fdt_content *fdt_data)
{
    if (!cust_ic_setting || !fdt_data)
    {
        return;
    }

    if (strstr(fdt_data->field, "vcomic_sub_info_version") != NULL)
    {
        cust_ic_setting->vcomic_sub_info.info_version = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if (strstr(fdt_data->field, "vcomic_sub_info_length") != NULL)
    {
        cust_ic_setting->vcomic_sub_info.info_length = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if (strstr(fdt_data->field, "vcomic_sub_type") != NULL)
    {
        cust_ic_setting->vcomic_sub_info.ic_type = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if (strstr(fdt_data->field, "vcomic_sub_bin_file_path") != NULL)
    {
        cust_ic_setting->vcomic_sub_info.ic_bin_file_path = malloc(MAX_CUST_PATH_LEN);
        if (cust_ic_setting->vcomic_sub_info.ic_bin_file_path != NULL)
        {
            memset(cust_ic_setting->vcomic_sub_info.ic_bin_file_path, 0x00, MAX_CUST_PATH_LEN);
            memcpy(cust_ic_setting->vcomic_sub_info.ic_bin_file_path, fdt_data->attribute, fdt_data->len);
        }
    }
    if(strstr(fdt_data->field,"vcomic_sub_checksum_bypass_offset") != NULL)
    {
        if (fdt_data->len > 0)
        {
            int array_size = fdt_data->len/sizeof(u32);
            cust_ic_setting->vcomic_sub_info.checksum_bypass_offset = malloc(fdt_data->len);
            if(cust_ic_setting->vcomic_sub_info.checksum_bypass_offset != NULL) {
                memset(cust_ic_setting->vcomic_sub_info.checksum_bypass_offset, 0, fdt_data->len);
                for (int i=0; i<array_size;i++)
                {
                    cust_ic_setting->vcomic_sub_info.checksum_bypass_offset[i] = fdt32_to_cpu(fdt_data->attribute[i]);
                }
            }
            cust_ic_setting->vcomic_sub_info.checksum_bypass_size = array_size;
        }
        else {
            cust_ic_setting->vcomic_sub_info.checksum_bypass_size = 0;
        }
    }
    if (strstr(fdt_data->field, "vcomic_sub_bin_format_type") != NULL)
    {
        cust_ic_setting->vcomic_sub_info.bin_format_type = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if (strstr(fdt_data->field, "vcomic_sub_force_init") != NULL)
    {
        cust_ic_setting->vcomic_sub_info.force_init = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if (strstr(fdt_data->field, "vcomic_sub_with_nvm") != NULL)
    {
        cust_ic_setting->vcomic_sub_info.with_nvm = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if (strstr(fdt_data->field, "vcomic_sub_i2c_bus") != NULL)
    {
        cust_ic_setting->vcomic_sub_info.i2c_bus = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if (strstr(fdt_data->field, "vcomic_sub_i2c_mode") != NULL)
    {
        cust_ic_setting->vcomic_sub_info.i2c_mode = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if (strstr(fdt_data->field, "vcomic_sub_i2c_dev_addr") != NULL)
    {
        cust_ic_setting->vcomic_sub_info.i2c_dev_addr = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if (strstr(fdt_data->field, "vcomic_sub_i2c_reg_offst") != NULL)
    {
        cust_ic_setting->vcomic_sub_info.i2c_reg_offset = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if (strstr(fdt_data->field, "vcomic_sub_pre_vcc_dly") != NULL)
    {
        cust_ic_setting->vcomic_sub_info.vcc_dly = fdt32_to_cpu(fdt_data->attribute[0]);
    }
}

void cust_vcomic_sub_dt_parser_part2(st_multi_cust_ic_info *cust_ic_setting, struct fdt_content *fdt_data)
{
    if (!cust_ic_setting || !fdt_data)
    {
        return;
    }

    if (strstr(fdt_data->field, "vcomic_sub_pre_gpio_num") != NULL)
    {
        cust_ic_setting->vcomic_sub_info.gpio_pre_num = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if (strstr(fdt_data->field, "vcomic_sub_pre_gpio_ops") != NULL)
    {
        cust_ic_setting->vcomic_sub_info.gpio_pre_ops = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if (strstr(fdt_data->field, "vcomic_sub_pre_delay") != NULL)
    {
        cust_ic_setting->vcomic_sub_info.gpio_pre_dly = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if (strstr(fdt_data->field, "vcomic_sub_post_gpio_num") != NULL)
    {
        cust_ic_setting->vcomic_sub_info.gpio_post_num = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if (strstr(fdt_data->field, "vcomic_sub_post_gpio_ops") != NULL)
    {
        cust_ic_setting->vcomic_sub_info.gpio_post_ops = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if (strstr(fdt_data->field, "vcomic_sub_post_delay") != NULL)
    {
        cust_ic_setting->vcomic_sub_info.gpio_post_dly = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if (strstr(fdt_data->field, "vcomic_sub_auto_from_flash") != NULL)
    {
        cust_ic_setting->vcomic_sub_info.auto_update_from_flash = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if (strstr(fdt_data->field, "vcomic_sub_i2c_read_mode") != NULL)
    {
        cust_ic_setting->vcomic_sub_info.read_mode = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if (strstr(fdt_data->field, "vcomic_sub_i2c_write_mode") != NULL)
    {
        cust_ic_setting->vcomic_sub_info.write_mode = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if (strstr(fdt_data->field, "vcomic_sub_i2c_burn_cmd") != NULL)
    {
        cust_ic_setting->vcomic_sub_info.i2c_burn_cmd = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if (strstr(fdt_data->field, "vcomic_sub_i2c_burn_offset") != NULL)
    {
        cust_ic_setting->vcomic_sub_info.i2c_burn_offset = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if (strstr(fdt_data->field, "vcomic_sub_i2c_ctrl_reg") != NULL)
    {
        cust_ic_setting->vcomic_sub_info.i2c_ctrl_reg = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if (strstr(fdt_data->field, "vcomic_sub_i2c_ctrl_reg_offset") != NULL)
    {
        cust_ic_setting->vcomic_sub_info.i2c_ctrl_reg_offset = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if (strstr(fdt_data->field, "vcomic_sub_data_start") != NULL)
    {
        cust_ic_setting->vcomic_sub_info.data_start = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if (strstr(fdt_data->field, "vcomic_sub_data_end") != NULL)
    {
        cust_ic_setting->vcomic_sub_info.data_end = fdt32_to_cpu(fdt_data->attribute[0]);
    }
}

void cust_vcomic_sub_dt_parser(struct fdt_content *fdt_data, void *out, const char *field_target)
{
    st_multi_cust_ic_info *cust_ic_setting = (st_multi_cust_ic_info*)out;
    if (!cust_ic_setting || !out)
    {
        return;
    }

    cust_vcomic_sub_dt_parser_part1(cust_ic_setting, fdt_data);
    cust_vcomic_sub_dt_parser_part2(cust_ic_setting, fdt_data);

    return;
}

void cust_tcon_dt_parser(struct fdt_content *fdt_data, void *out, const char *field_target)
{
    st_cust_tcon_info *tcon_info = (st_cust_tcon_info*)out;
    if (!tcon_info || !out)
    {
        return;
    }

    if (strstr(fdt_data->field, "bVRR_HighFrameRateMode_Support") != NULL)
    {
        tcon_info->bVRR_HighFrameRateMode_Support = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if (strstr(fdt_data->field, "ucVRR_HFR_ON_I2C_DATA_SIZE") != NULL)
    {
        tcon_info->ucVRR_HFR_ON_I2C_DATA_SIZE = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if (strstr(fdt_data->field, "ucVRR_HFR_ON_I2C_DATA_0") != NULL)
    {
        tcon_info->ucVRR_HFR_ON_I2C_DATA[0] = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if (strstr(fdt_data->field, "ucVRR_HFR_ON_I2C_DATA_1") != NULL)
    {
        tcon_info->ucVRR_HFR_ON_I2C_DATA[1] = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if (strstr(fdt_data->field, "ucVRR_HFR_ON_I2C_DATA_2") != NULL)
    {
        tcon_info->ucVRR_HFR_ON_I2C_DATA[2] = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if (strstr(fdt_data->field, "ucVRR_HFR_ON_I2C_DATA_3") != NULL)
    {
        tcon_info->ucVRR_HFR_ON_I2C_DATA[3] = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if (strstr(fdt_data->field, "ucVRR_HFR_ON_I2C_DATA_4") != NULL)
    {
        tcon_info->ucVRR_HFR_ON_I2C_DATA[4] = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if (strstr(fdt_data->field, "ucVRR_HFR_ON_I2C_DATA_5") != NULL)
    {
        tcon_info->ucVRR_HFR_ON_I2C_DATA[5] = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if (strstr(fdt_data->field, "ucVRR_HFR_ON_I2C_DATA_6") != NULL)
    {
        tcon_info->ucVRR_HFR_ON_I2C_DATA[6] = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if (strstr(fdt_data->field, "ucVRR_HFR_OFF_I2C_DATA_SIZE") != NULL)
    {
        tcon_info->ucVRR_HFR_OFF_I2C_DATA_SIZE = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if (strstr(fdt_data->field, "ucVRR_HFR_OFF_I2C_DATA_0") != NULL)
    {
        tcon_info->ucVRR_HFR_OFF_I2C_DATA[0] = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if (strstr(fdt_data->field, "ucVRR_HFR_OFF_I2C_DATA_1") != NULL)
    {
        tcon_info->ucVRR_HFR_OFF_I2C_DATA[1] = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if (strstr(fdt_data->field, "ucVRR_HFR_OFF_I2C_DATA_2") != NULL)
    {
        tcon_info->ucVRR_HFR_OFF_I2C_DATA[2] = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if (strstr(fdt_data->field, "ucVRR_HFR_OFF_I2C_DATA_3") != NULL)
    {
        tcon_info->ucVRR_HFR_OFF_I2C_DATA[3] = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if (strstr(fdt_data->field, "ucVRR_HFR_OFF_I2C_DATA_4") != NULL)
    {
        tcon_info->ucVRR_HFR_OFF_I2C_DATA[4] = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if (strstr(fdt_data->field, "ucVRR_HFR_OFF_I2C_DATA_5") != NULL)
    {
        tcon_info->ucVRR_HFR_OFF_I2C_DATA[5] = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if (strstr(fdt_data->field, "ucVRR_HFR_OFF_I2C_DATA_6") != NULL)
    {
        tcon_info->ucVRR_HFR_OFF_I2C_DATA[6] = fdt32_to_cpu(fdt_data->attribute[0]);
    }

    if (strstr(fdt_data->field, "ucVRR_HFR_ON_I2C_bus") != NULL)
    {
        tcon_info->ucVRR_HFR_ON_I2C_bus = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if (strstr(fdt_data->field, "ucVRR_HFR_ON_I2C_mode") != NULL)
    {
        tcon_info->ucVRR_HFR_ON_I2C_mode = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if (strstr(fdt_data->field, "ucVRR_HFR_ON_I2C_dev_addr") != NULL)
    {
        tcon_info->ucVRR_HFR_ON_I2C_dev_addr = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if (strstr(fdt_data->field, "ucVRR_HFR_ON_I2C_offst") != NULL)
    {
        tcon_info->ucVRR_HFR_ON_I2C_offst = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if (strstr(fdt_data->field, "bVRR_HighPixelClockMode_Support") != NULL)
    {
        tcon_info->bVRR_HighPixelClockMode_Support = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if (strstr(fdt_data->field, "bGAME_Direct60HzMode_Support") != NULL)
    {
        tcon_info->bGAME_Direct60HzMode_Support = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if (strstr(fdt_data->field, "ucGAME_60HZ_ON_I2C_DATA_SIZE") != NULL)
    {
        tcon_info->ucGAME_60HZ_ON_I2C_DATA_SIZE = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if (strstr(fdt_data->field, "ucGAME_60HZ_ON_I2C_DATA_0") != NULL)
    {
        tcon_info->ucGAME_60HZ_ON_I2C_DATA_0 = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if (strstr(fdt_data->field, "ucGAME_60HZ_ON_I2C_DATA_1") != NULL)
    {
        tcon_info->ucGAME_60HZ_ON_I2C_DATA_1 = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if (strstr(fdt_data->field, "bSupportGpioModeChange") != NULL)
    {
        tcon_info->bSupportGpioModeChange = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if (strstr(fdt_data->field, "u16GPIO_NUM_MODE_CHG") != NULL)
    {
        tcon_info->u16GPIO_NUM_MODE_CHG = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if (strstr(fdt_data->field, "ucGPIO_CTRL_MODE_CHG_ON") != NULL)
    {
        tcon_info->ucGPIO_CTRL_MODE_CHG_ON = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if (strstr(fdt_data->field, "ucGPIO_CTRL_MODE_CHG_OFF") != NULL)
    {
        tcon_info->ucGPIO_CTRL_MODE_CHG_OFF = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if (strstr(fdt_data->field, "u16DELAY_MODE_CHG") != NULL)
    {
        tcon_info->u16DELAY_MODE_CHG = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if (strstr(fdt_data->field, "bSupportTconI2cWP") != NULL)
    {
        tcon_info->bSupportTconI2cWP = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if (strstr(fdt_data->field, "u16GPIO_NUM_TCON_I2C_WP") != NULL)
    {
        tcon_info->u16GPIO_NUM_TCON_I2C_WP = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if (strstr(fdt_data->field, "ucGPIO_TCON_I2C_WP_ON") != NULL)
    {
        tcon_info->ucGPIO_TCON_I2C_WP_ON = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if (strstr(fdt_data->field, "ucGPIO_TCON_I2C_WP_OFF") != NULL)
    {
        tcon_info->ucGPIO_TCON_I2C_WP_OFF = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if (strstr(fdt_data->field, "DLG_I2C_Sec_CusCtrl") != NULL)
    {
        tcon_info->bVRR_HFR_ON_I2C_SEC_Support = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    //support tcon sec i2c case
    if (tcon_info->bVRR_HFR_ON_I2C_SEC_Support)
    {
        if (strstr(fdt_data->field, "ucVRR_HFR_ON_I2C_SEC_DATA_SIZE") != NULL)
        {
            tcon_info->ucVRR_HFR_ON_I2C_SEC_DATA_SIZE = fdt32_to_cpu(fdt_data->attribute[0]);
        }
        if (strstr(fdt_data->field, "ucVRR_HFR_ON_I2C_SEC_DATA_0") != NULL)
        {
            tcon_info->ucVRR_HFR_ON_I2C_SEC_DATA[0] = fdt32_to_cpu(fdt_data->attribute[0]);
        }
        if (strstr(fdt_data->field, "ucVRR_HFR_ON_I2C_SEC_DATA_1") != NULL)
        {
            tcon_info->ucVRR_HFR_ON_I2C_SEC_DATA[1] = fdt32_to_cpu(fdt_data->attribute[0]);
        }
        if (strstr(fdt_data->field, "ucVRR_HFR_ON_I2C_SEC_DATA_2") != NULL)
        {
            tcon_info->ucVRR_HFR_ON_I2C_SEC_DATA[2] = fdt32_to_cpu(fdt_data->attribute[0]);
        }
        if (strstr(fdt_data->field, "ucVRR_HFR_ON_I2C_SEC_DATA_3") != NULL)
        {
            tcon_info->ucVRR_HFR_ON_I2C_SEC_DATA[3] = fdt32_to_cpu(fdt_data->attribute[0]);
        }
        if (strstr(fdt_data->field, "ucVRR_HFR_ON_I2C_SEC_DATA_4") != NULL)
        {
            tcon_info->ucVRR_HFR_ON_I2C_SEC_DATA[4] = fdt32_to_cpu(fdt_data->attribute[0]);
        }
        if (strstr(fdt_data->field, "ucVRR_HFR_ON_I2C_SEC_DATA_5") != NULL)
        {
            tcon_info->ucVRR_HFR_ON_I2C_SEC_DATA[5] = fdt32_to_cpu(fdt_data->attribute[0]);
        }
        if (strstr(fdt_data->field, "ucVRR_HFR_ON_I2C_SEC_DATA_6") != NULL)
        {
            tcon_info->ucVRR_HFR_ON_I2C_SEC_DATA[6] = fdt32_to_cpu(fdt_data->attribute[0]);
        }
        if (strstr(fdt_data->field, "ucVRR_HFR_OFF_I2C_SEC_DATA_SIZE") != NULL)
        {
            tcon_info->ucVRR_HFR_OFF_I2C_SEC_DATA_SIZE = fdt32_to_cpu(fdt_data->attribute[0]);
        }
        if (strstr(fdt_data->field, "ucVRR_HFR_OFF_I2C_SEC_DATA_0") != NULL)
        {
            tcon_info->ucVRR_HFR_OFF_I2C_SEC_DATA[0] = fdt32_to_cpu(fdt_data->attribute[0]);
        }
        if (strstr(fdt_data->field, "ucVRR_HFR_OFF_I2C_SEC_DATA_1") != NULL)
        {
            tcon_info->ucVRR_HFR_OFF_I2C_SEC_DATA[1] = fdt32_to_cpu(fdt_data->attribute[0]);
        }
        if (strstr(fdt_data->field, "ucVRR_HFR_OFF_I2C_SEC_DATA_2") != NULL)
        {
            tcon_info->ucVRR_HFR_OFF_I2C_SEC_DATA[2] = fdt32_to_cpu(fdt_data->attribute[0]);
        }
        if (strstr(fdt_data->field, "ucVRR_HFR_OFF_I2C_SEC_DATA_3") != NULL)
        {
            tcon_info->ucVRR_HFR_OFF_I2C_SEC_DATA[3] = fdt32_to_cpu(fdt_data->attribute[0]);
        }
        if (strstr(fdt_data->field, "ucVRR_HFR_OFF_I2C_SEC_DATA_4") != NULL)
        {
            tcon_info->ucVRR_HFR_OFF_I2C_SEC_DATA[4] = fdt32_to_cpu(fdt_data->attribute[0]);
        }
        if (strstr(fdt_data->field, "ucVRR_HFR_OFF_I2C_SEC_DATA_5") != NULL)
        {
            tcon_info->ucVRR_HFR_OFF_I2C_SEC_DATA[5] = fdt32_to_cpu(fdt_data->attribute[0]);
        }
        if (strstr(fdt_data->field, "ucVRR_HFR_OFF_I2C_SEC_DATA_6") != NULL)
        {
            tcon_info->ucVRR_HFR_OFF_I2C_SEC_DATA[6] = fdt32_to_cpu(fdt_data->attribute[0]);
        }
        if (strstr(fdt_data->field, "ucVRR_HFR_ON_I2C_SEC_bus") != NULL)
        {
            tcon_info->ucVRR_HFR_ON_I2C_SEC_bus = fdt32_to_cpu(fdt_data->attribute[0]);
        }
        if (strstr(fdt_data->field, "ucVRR_HFR_ON_I2C_SEC_mode") != NULL)
        {
        tcon_info->ucVRR_HFR_ON_I2C_SEC_mode = fdt32_to_cpu(fdt_data->attribute[0]);
        }
        if (strstr(fdt_data->field, "ucVRR_HFR_ON_I2C_SEC_dev_addr") != NULL)
        {
            tcon_info->ucVRR_HFR_ON_I2C_SEC_dev_addr = fdt32_to_cpu(fdt_data->attribute[0]);
        }
        if (strstr(fdt_data->field, "ucVRR_HFR_ON_I2C_SEC_offst") != NULL)
        {
            tcon_info->ucVRR_HFR_ON_I2C_SEC_offst = fdt32_to_cpu(fdt_data->attribute[0]);
        }
    }

    return;
}

void cust_oled_dt_parser(struct fdt_content *fdt_data, void *out, const char *field_target)
{
    st_oled_i2c_info *oled_i2c_info = (st_oled_i2c_info*)out;
    if (!oled_i2c_info || !out)
    {
        return;
    }

    if (strstr(fdt_data->field, "OLED_Support") != NULL)
    {
        oled_i2c_info->OLED_Support = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if (strstr(fdt_data->field, "slave_addr") != NULL)
    {
        oled_i2c_info->slave_addr = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if (strstr(fdt_data->field, "channel_id") != NULL)
    {
        oled_i2c_info->channel_id = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if (strstr(fdt_data->field, "i2c_mode") != NULL)
    {
        oled_i2c_info->i2c_mode = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if (strstr(fdt_data->field, "lumin_gain_addr") != NULL)
    {
        oled_i2c_info->lumin_gain_addr = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    return;
}
#endif

void panel_spi_bus_info_dt_parser(struct fdt_content *fdt_data, void *out, const char *field_target)
{
    st_panel_spi_info *panel_spi_bus = (st_panel_spi_info*)out;
    if (!panel_spi_bus || !out)
        return;

    if(strstr(fdt_data->field,"panel_spi_bus") != NULL)
    {
        panel_spi_bus->panel_spi_bus = malloc(MAX_CUST_PATH_LEN);
        if(panel_spi_bus->panel_spi_bus != NULL) {
            memset(panel_spi_bus->panel_spi_bus, 0x00, MAX_CUST_PATH_LEN);
            memcpy(panel_spi_bus->panel_spi_bus, fdt_data->attribute, fdt_data->len);
        }
    }
    if(strstr(fdt_data->field,"panel_spi_speed") != NULL)
    {
        panel_spi_bus->panel_spi_speed = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"panel_spi_mode") != NULL)
    {
        panel_spi_bus->panel_spi_mode = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    return;
}

void ldm_dt_parser(struct fdt_content *fdt_data, void *out, const char *field_target)
{
    st_ldm_setting *ldm_setting = (st_ldm_setting*)out;
    if (!ldm_setting || !out)
        return;

    if(strstr(fdt_data->field,"LDM_SUPPORT") != NULL)
    {
        ldm_setting->m_u32LDMSupport= fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"LDM_CUS_IP_PATH_uBoot") != NULL)
    {
        ldm_setting->m_bLDMCusPathuBoot = (bool)fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"LED_MSPI_EN") != NULL)
    {
        ldm_setting->m_bLEDMSPIEN = (bool)fdt32_to_cpu(fdt_data->attribute[0]);
    }
}

void board_dt_parser(struct fdt_content *fdt_data, void *out, const char *field_target)
{
    st_board_para *board_setting = (st_board_para*)out;
    if (!board_setting || !out)
        return;

    if(strstr(fdt_data->field,"m_bPanelPDP10BIT") != NULL)
    {
        board_setting->m_bPANEL_PDP_10BIT = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"m_bPanelSwapLVDS_POL") != NULL)
    {
        board_setting->m_bPANEL_SWAP_LVDS_POL = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"m_bPanelSwapLVDS_CH") != NULL)
    {
        board_setting->m_bPANEL_SWAP_LVDS_CH = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"m_bPanelSwapPort") != NULL)
    {
        board_setting->m_bPANEL_CONNECTOR_SWAP_PORT = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"m_u16LVDSTxSwapValue") != NULL)
    {
        board_setting->m_u32LVDS_PN_SWAP = fdt32_to_cpu(fdt_data->attribute[0]);
    }
}

void misc_dt_parser(struct fdt_content *fdt_data, void *out, const char *field_target)
{
    st_sys_misc_setting *misc_setting = (st_sys_misc_setting*)out;
    if (!misc_setting || !out)
    {
        return;
    }

	if(strstr(fdt_data->field,"typ_h_total") != NULL)
	{
		misc_setting->m_wPanelHTotal = fdt32_to_cpu(fdt_data->attribute[0]);
	}
	if(strstr(fdt_data->field,"typ_v_total") != NULL)
    {
        misc_setting->m_wPanelVTotal = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"typ_framerate") != NULL)
    {
        misc_setting->m_dwPanelframerate = fdt32_to_cpu(fdt_data->attribute[0]);
		misc_setting->m_u16PanelDCLK = misc_setting->m_wPanelHTotal *
			misc_setting->m_wPanelVTotal * misc_setting->m_dwPanelframerate;
    }
    if(strstr(fdt_data->field,"link_type") != NULL)
    {
        misc_setting->m_u16Panel_ext_type = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"m_wPanelOnTiming3") != NULL)
    {
        misc_setting->m_wPanelOnTiming3 = fdt32_to_cpu(fdt_data->attribute[0]);
    }
    if(strstr(fdt_data->field,"onrf_op") != NULL)
    {
        misc_setting->m_onrf_op = fdt32_to_cpu(fdt_data->attribute[0]);
    }
}

int parse_dt(const char *tag_target, void (*parser_cb)(struct fdt_content *, void*, const char*), void *out, const char *field_target)
{
    const struct fdt_property *fdt_prop;
    struct fdt_header *dts_header;
    int  nodeoffset, nextoffset, level = 0, len;
    uint32_t tag;
    struct fdt_content fdt_data;

    dts_header = (struct fdt_header *)gd->fdt_blob;
    nodeoffset = fdt_path_offset(dts_header, tag_target);
    while(level >= 0)
    {
        tag = fdt_next_tag(dts_header, nodeoffset, &nextoffset);
        switch(tag)
        {
            case FDT_BEGIN_NODE:
                level++;
                if (level >= MAX_LEVEL)
                {
                    UBOOT_ERROR("Nested too deep, aborting.\n");
                    return -1;
                }
                break;
            case FDT_END_NODE:
                level--;
                if (level == 0)
                    level = -1; /* exit the loop */
                break;
            case FDT_PROP:
                fdt_prop = fdt_offset_ptr(dts_header, nodeoffset,sizeof(*fdt_prop));
                if(fdt_prop == NULL)
                {
                    UBOOT_ERROR("fdt_prop is NULL\n");
                    return -1;
                }

                fdt_data.field = (char *)fdt_string(dts_header,fdt32_to_cpu(fdt_prop->nameoff));
                len = fdt32_to_cpu(fdt_prop->len);
                fdt_data.len = len;
                fdt_data.attribute = (__be32 *)fdt_prop->data;
                if (len < 0)
                {
                    UBOOT_ERROR("Dts data parsing error, %s\n",fdt_strerror(len));
                    return -1;
                }
                else if (len == 0)
                {
                    /* the property has no value */
                    ;
                }
                else
                {
                    if (level <= MAX_LEVEL)
                    {
                        parser_cb(&fdt_data, out, field_target);
                    }
                }
                break;
            default:
                if (level <= MAX_LEVEL)
                    UBOOT_ERROR("Unknown tag 0x%08X\n", tag);
                return -1;
        }
        nodeoffset = nextoffset;
    }

    return 0;
}
