/* SPDX-License-Identifier: GPL-2.0-only OR BSD-3-Clause */
/******************************************************************************
 *
 * This file is provided under a dual license.  When you use or
 * distribute this software, you may choose to be licensed under
 * version 2 of the GNU General Public License ("GPLv2 License")
 * or BSD License.
 *
 * GPLv2 License
 *
 * Copyright(C) 2019 MediaTek Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See http://www.gnu.org/licenses/gpl-2.0.html for more details.
 *
 * BSD LICENSE
 *
 * Copyright(C) 2019 MediaTek Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *  * Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *****************************************************************************/

//=============================================================================
// Include Files
//=============================================================================
#include "platform.h"
#include "halCM4.h"
#include "halCPUINT.h"
#include "halBDMA.h"

//=============================================================================
// Debug Macros
//=============================================================================
//=============================================================================
// CM4 HAL Driver Function
//=============================================================================
extern mtk_pmu_info g_pmu_info;

//TODO: get chip family ID from system API instead of riu register
int MHal_Get_Chip_ID(void)
{
	int chip_id;

	//0x00108_00[15:0] 16-bit offset chip id
	//0x00108_01[15:8] 16-bit offset chip version
	chip_id = INREG16((g_pmu_info.pm_top)|(OFFSET8(0x00)));
	PMU_log(PMU_DEBUG,"%s: Chip ID:%X\n", __func__, chip_id);

	return chip_id;
}

int MHal_CM4_Set_PLL(int chip_id)
{
    switch (chip_id)
    {
        case CHIP_MT5897:
        {
            INSREG8((g_pmu_info.paganini)|(OFFSET8(0x02)), 0x04, 0x00); //reg_dsppll_pd_ckout_div2, CKOUT_DIV_PD
            INSREG8((g_pmu_info.paganini)|(OFFSET8(0x07)), 0x07, 0x04); //reg_dsppll_loop_div_second, MPLL second loop divider
            INSREG8((g_pmu_info.paganini)|(OFFSET8(0x07)), 0x20, 0x00); //reg_dsppll_loop_div_second, MPLL second loop divider
            INSREG8((g_pmu_info.paganini)|(OFFSET8(0x04)), 0x30, 0x00); //reg_dsppll_input_div_first, 00:div1, 01:div2
            INSREG8((g_pmu_info.paganini)|(OFFSET8(0x05)), 0x03, 0x00); //reg_dsppll_loop_div_first, 00:div1, 01:div2, 10:div4,11:div8
            OUTREG8((g_pmu_info.paganini)|(OFFSET8(0x06)), 0x32); //reg_dsppll_loop_div_second, MPLL second loop divider
            break;
        }
        case CHIP_MT5879:
        {
            INSREG8((g_pmu_info.paganini)|(OFFSET8(0x02)), 0x04, 0x00); //reg_dsppll_pd_ckout_div2, CKOUT_DIV_PD
            INSREG8((g_pmu_info.paganini)|(OFFSET8(0x07)), 0x07, 0x02); //reg_dsppll_loop_div_second, MPLL second loop divider
            INSREG8((g_pmu_info.paganini)|(OFFSET8(0x07)), 0x20, 0x00); //reg_dsppll_loop_div_second, MPLL second loop divider
            INSREG8((g_pmu_info.paganini)|(OFFSET8(0x04)), 0x30, 0x00); //reg_dsppll_input_div_first, 00:div1, 01:div2
            INSREG8((g_pmu_info.paganini)|(OFFSET8(0x05)), 0x03, 0x00); //reg_dsppll_loop_div_first, 00:div1, 01:div2, 10:div4,11:div8
            OUTREG8((g_pmu_info.paganini)|(OFFSET8(0x06)), 0x42); //reg_dsppll_loop_div_second, MPLL second loop divider
            break;
        }
        default:
        {
            PMU_log(PMU_DEBUG,"%s: Use default PLL\n", __func__);
        }
    }
    return 0;
}

int MHal_CM4_Set_DMIC_PLL(int chip_id)
{
    switch (chip_id)
    {
        case CHIP_MT5897:
        {
            INSREG8((g_pmu_info.ckgen00_pm)|(OFFSET8(0x22)), 0x0f, 0x0c); //reg_ckg_vrec_mac
            INSREG8((g_pmu_info.ckgen00_pm)|(OFFSET8(0x12)), 0x0f, 0x08); //reg_ckg_dig_mic
            INSREG8((g_pmu_info.paganini)|(OFFSET8(0xe0)), 0x01, 0x01); //reg_paga_pll_div_enable
            INSREG8((g_pmu_info.paganini)|(OFFSET8(0xe0)), 0xf8, 0x10); //reg_paga_pll_div_96
            OUTREG8((g_pmu_info.paganini)|(OFFSET8(REG_PAGA_PLL_DIV_48)), BIT0|BIT2); //reg_paga_pll_div_48
            break;
        }
        case CHIP_MT5879:
        {
            INSREG8((g_pmu_info.ckgen00_pm)|(OFFSET8(0x22)), 0x0f, 0x0c); //reg_ckg_vrec_mac
            INSREG8((g_pmu_info.ckgen00_pm)|(OFFSET8(0x12)), 0x0f, 0x00); //reg_ckg_dig_mic
            INSREG8((g_pmu_info.paganini)|(OFFSET8(0xe0)), 0x01, 0x01); //reg_paga_pll_div_enable
            INSREG8((g_pmu_info.paganini)|(OFFSET8(0xe0)), 0xf8, 0x40); //reg_paga_pll_div_96
            OUTREG8((g_pmu_info.paganini)|(OFFSET8(REG_PAGA_PLL_DIV_48)), 0x00); //reg_paga_pll_div_48
            break;
        }
        default:
        {
            PMU_log(PMU_DEBUG,"%s: Use default PLL\n", __func__);
            INSREG8((g_pmu_info.ckgen00_pm)|(OFFSET8(0x22)), 0x0f, 0x0c); //reg_ckg_vrec_mac
            INSREG8((g_pmu_info.ckgen00_pm)|(OFFSET8(0x12)), 0x0f, 0x08); //reg_ckg_dig_mic
        }
    }
    return 0;
}

void mhal_cm4_init(mtk_pmu_info *info)
{
    // PLL 624MHz (default)
    INSREG8((g_pmu_info.paganini)|(OFFSET8(0x03)), 0x01, 0x00); //reg_dsppll_pd, MPLL power down control, active low;
    INSREG8((g_pmu_info.paganini)|(OFFSET8(0x02)), 0x04, 0x00); //reg_dsppll_in_select,  input clock selection: 1'b0 1. 0V clock input;
    INSREG8((g_pmu_info.paganini)|(OFFSET8(0x02)), 0x08, 0x00); //reg_dsppll_pd_ckout_div2p5, CKOUT_DIV_PD
    INSREG8((g_pmu_info.paganini)|(OFFSET8(0x02)), 0x10, 0x00); //reg_dsppll_pd_ckout_div3, CKOUT_DIV_PD
    INSREG8((g_pmu_info.paganini)|(OFFSET8(0x02)), 0x20, 0x00); //reg_dsppll_pd_ckout_div5, CKOUT_DIV_PD
    INSREG8((g_pmu_info.paganini)|(OFFSET8(0x02)), 0x40, 0x00); //reg_dsppll_pd_postdiv, CKOUT_DIV_PD
    INSREG8((g_pmu_info.paganini)|(OFFSET8(0x07)), 0x07, 0x00); //reg_dsppll_loop_div_second, MPLL second loop divider
    INSREG8((g_pmu_info.paganini)|(OFFSET8(0x07)), 0x20, 0x00); //reg_dsppll_loop_div_second, MPLL second loop divider
    INSREG8((g_pmu_info.paganini)|(OFFSET8(0x04)), 0x30, 0x00); //reg_dsppll_input_div_first, 00:div1, 01:div2
    INSREG8((g_pmu_info.paganini)|(OFFSET8(0x05)), 0x03, 0x00); //reg_dsppll_loop_div_first, 00:div1, 01:div2, 10:div4,11:div8
    OUTREG8((g_pmu_info.paganini)|(OFFSET8(0x06)), 0x34); //reg_dsppll_loop_div_second, MPLL second loop divider
    MHal_CM4_Set_PLL(MHal_Get_Chip_ID()); //Change PLL according by chip id

    // AC ON
    OUTREG16((g_pmu_info.paganini)|(OFFSET(0xe0)), 0x8001); //reg_paga_48_96_ck_div_ctrl
    INSREG8((g_pmu_info.ckgen01_pm)|(OFFSET8(0x1e)), 0x01, 0x00); //reg_sw_en_imi2paganini_imi
    INSREG8((g_pmu_info.ckgen01_pm)|(OFFSET8(0x0c)), 0x01, 0x00); //reg_sw_en_cm42cm4
    INSREG8((g_pmu_info.ckgen01_pm)|(OFFSET8(0x02)), 0x01, 0x00); //reg_sw_en_cm4_core2cm4_core
    INSREG8((g_pmu_info.ckgen01_pm)|(OFFSET8(0x06)), 0x01, 0x00); //reg_sw_en_cm4_systick2cm4_systick
    INSREG8((g_pmu_info.ckgen01_pm)|(OFFSET8(0x08)), 0x01, 0x00); //reg_sw_en_cm4_tck2tck_int
    INSREG8((g_pmu_info.ckgen01_pm)|(OFFSET8(0x0a)), 0x01, 0x00); //reg_sw_en_cm4_tsvalueb2cm4_tsvalueb

    INSREG8((g_pmu_info.paganini)|(OFFSET8(0xe6)), 0x02, 0x00); //reg_cm4_core_en_lock
    INSREG8((g_pmu_info.paganini)|(OFFSET8(0xf3)), 0x02, 0x02); //reg_clkgen_imi_dfs_ctrl
    INSREG8((g_pmu_info.paganini)|(OFFSET8(0xf7)), 0x02, 0x02); //reg_clkgen_cm4_dfs_ctrl

    INSREG8((g_pmu_info.ckgen00_pm)|(OFFSET8(0x16)), 0x1f, 0x08); //reg_ckg_nf_synth_ref
    INSREG8((g_pmu_info.ckgen00_pm)|(OFFSET8(0x0e)), 0x1f, 0x04); //reg_ckg_codec_i2s_rx_bck
    INSREG8((g_pmu_info.ckgen00_pm)|(OFFSET8(0x1e)), 0x0f, 0x08); //reg_ckg_src_a1_256fsi
    INSREG8((g_pmu_info.ckgen00_pm)|(OFFSET8(0x10)), 0x1f, 0x10); //reg_ckg_codec_i2s_rx_ms_bck_p

    INSREG8((g_pmu_info.ckgen00_pm)|(OFFSET8(0x26)), 0x07, 0x04); //reg_ckg_paganini_imi_div2
    INSREG8((g_pmu_info.paganini)|(OFFSET8(0xec)), 0x02, 0x02); //reg_nf_synth_en_trig
    INSREG8((g_pmu_info.paganini)|(OFFSET8(0x61)), 0x80, 0x80); //reg_i2s_tdm_cfg_00

    // CM4 TSV & SYSTICK
    INSREG8((g_pmu_info.ckgen00_pm)|(OFFSET8(0x20)), 0x1f, 0x04); //reg_ckg_paga_pll_sel
    INSREG8((g_pmu_info.ckgen00_pm)|(OFFSET8(0x1c)), 0x0f, 0x04); //reg_ckg_pll2pagadiv
    INSREG8((g_pmu_info.ckgen00_pm)|(OFFSET8(0x00)), 0x07, 0x04); //reg_ckg_cm4
    INSREG8((g_pmu_info.ckgen00_pm)|(OFFSET8(0x04)), 0x0f, 0x04); //reg_ckg_cm4_dfs
    INSREG8((g_pmu_info.ckgen00_pm)|(OFFSET8(0x02)), 0x07, 0x04); //reg_ckg_cm4_core
    INSREG8((g_pmu_info.ckgen00_pm)|(OFFSET8(0x0a)), 0x07, 0x04); //reg_ckg_cm4_tsvalueb
    INSREG8((g_pmu_info.ckgen00_pm)|(OFFSET8(0x06)), 0x07, 0x04); //reg_ckg_cm4_systick

    if (MHal_Get_Chip_ID() == CHIP_MT5897) {
        INSREG8((g_pmu_info.ckgen00_pm)|(OFFSET8(0x20)), 0x1f, 0x08); //reg_ckg_paga_pll_sel
        OUTREG8((g_pmu_info.ckgen00_pm)|(OFFSET8(0x21)), 0x10); //reg_ckg_paga_pll_sel2
        INSREG8((g_pmu_info.ckgen00_pm)|(OFFSET8(0x1c)), 0x0f, 0x08); //reg_ckg_pll2pagadiv
    }

    INSREG8((g_pmu_info.paganini)|(OFFSET8(0xf6)), 0x3f, 0x1f); //reg_clkgen_cm4_dfs_ctrl
    INSREG8((g_pmu_info.paganini)|(OFFSET8(0xf7)), 0x81, 0x00); //reg_clkgen_cm4_dfs_ctrl
    INSREG8((g_pmu_info.paganini)|(OFFSET8(0xf7)), 0x81, 0x01); //reg_clkgen_cm4_dfs_ctrl
    INSREG8((g_pmu_info.paganini)|(OFFSET8(0xf7)), 0x81, 0x00); //reg_clkgen_cm4_dfs_ctrl

    // IMI CLK  XTAIL -> DFS OFF -> PLL -> DFS_ON
    INSREG8((g_pmu_info.ckgen00_pm)|(OFFSET8(0x18)), 0x0f, 0x04); //reg_ckg_paganini_imi
    INSREG8((g_pmu_info.ckgen00_pm)|(OFFSET8(0x1a)), 0x0f, 0x04); //reg_ckg_paganini_imi_dfs

    INSREG8((g_pmu_info.paganini)|(OFFSET8(0xf2)), 0x3f, 0x1f); //reg_clkgen_imi_dfs_ctrl
    INSREG8((g_pmu_info.paganini)|(OFFSET8(0xf3)), 0x81, 0x00); //reg_clkgen_imi_dfs_ctrl
    INSREG8((g_pmu_info.paganini)|(OFFSET8(0xf3)), 0x81, 0x01); //reg_clkgen_imi_dfs_ctrl
    INSREG8((g_pmu_info.paganini)|(OFFSET8(0xf3)), 0x81, 0x00); //reg_clkgen_imi_dfs_ctrl

    // CLK Release
    INSREG8((g_pmu_info.ckgen01_pm)|(OFFSET8(0x1e)), 0x01, 0x01); //reg_sw_en_imi2paganini_imi
    INSREG8((g_pmu_info.ckgen01_pm)|(OFFSET8(0x0c)), 0x01, 0x01); //reg_sw_en_cm42cm4
    INSREG8((g_pmu_info.ckgen01_pm)|(OFFSET8(0x02)), 0x01, 0x01); //reg_sw_en_cm4_core2cm4_core
    INSREG8((g_pmu_info.ckgen01_pm)|(OFFSET8(0x06)), 0x01, 0x01); //reg_sw_en_cm4_systick2cm4_systick
    INSREG8((g_pmu_info.ckgen01_pm)|(OFFSET8(0x08)), 0x01, 0x01); //reg_sw_en_cm4_tck2tck_int
    INSREG8((g_pmu_info.ckgen01_pm)|(OFFSET8(0x0a)), 0x01, 0x01); //reg_sw_en_cm4_tsvalueb2cm4_tsvalueb
    INSREG8((g_pmu_info.ckgen01_pm)|(OFFSET8(0x90)), 0x07, 0x07); //reg_sw_en_mcu_pmu2cm4
    INSREG8((g_pmu_info.paganini)|(OFFSET8(0xe6)), 0x02, 0x02); //reg_cm4_core_en_lock

    // DMIC BCK Gen Clk
    MHal_CM4_Set_DMIC_PLL(MHal_Get_Chip_ID());

    //vrec dma setting
    OUTREG16((g_pmu_info.vrec)|(OFFSET(0x62)), 0x0100); //RIU_AUDMA_W1_CFG01[15:0],  dram_base_addr
    OUTREG16((g_pmu_info.vrec)|(OFFSET(0x6a)), 0x0100); //RIU_AUDMA_W1_CFG05[15:0],  overrun_th
    OUTREG16((g_pmu_info.vrec)|(OFFSET(0x60)), 0x8000); //RIU_AUDMA_W1_CFG00
    OUTREG16((g_pmu_info.vrec)|(OFFSET(0x60)), 0x020b); //RIU_AUDMA_W1_CFG00 [0]:miu_req_en, [1]:dma_wr_en, [3]:lr_swap, [9]:bit_mode

    OUTREG16((g_pmu_info.vrec)|(OFFSET(0x80)), 0x0003); //reg_dma_mch_if_cfg1, [2:0] channel mode, 0:2ch, 1:4ch

    OUTREG16((g_pmu_info.vrec)|(OFFSET(0x22)), 0x0001); //reg_dmic_ctrl1, [0] enable phase select for ch1 and ch2
    OUTREG16((g_pmu_info.vrec)|(OFFSET(0x24)), 0x8000); //reg_dmic_ctrl2, [15] enable auto phase selection
    OUTREG16((g_pmu_info.vrec)|(OFFSET(0x26)), 0x0001); //reg_dmic_ctrl3, [0] enable phase select for ch3 and ch4
    OUTREG16((g_pmu_info.vrec)|(OFFSET(0x28)), 0x8000); //reg_dmic_ctrl4, [15] enable auto phase selection
    OUTREG16((g_pmu_info.vrec)|(OFFSET(0x2A)), 0x0001); //reg_dmic_ctrl5, [0] enable phase select for ch5 and ch6
    OUTREG16((g_pmu_info.vrec)|(OFFSET(0x2C)), 0x8000); //reg_dmic_ctrl6, [15] enable auto phase selection
    OUTREG16((g_pmu_info.vrec)|(OFFSET(0x2E)), 0x0001); //reg_dmic_ctrl7, [0] enable phase select for ch7 and ch8
    OUTREG16((g_pmu_info.vrec)|(OFFSET(0x30)), 0x8000); //reg_dmic_ctrl8, [15] enable auto phase selection
    OUTREG16((g_pmu_info.vrec)|(OFFSET(0x20)), 0x0000); //reg_dmic_ctrl0
    OUTREG16((g_pmu_info.vrec)|(OFFSET(0x0C)), 0x011F); //reg_vrec_ctrl06, [0] enable deciamtion filter, ch1/ch2 [1] enable deciamtion filter, ch3/ch4 [2] enable deciamtion filter, ch5/ch6 [3] enable deciamtion filter, ch7/ch8 [4] enable deciamtion filter[8] enable output clock  generation

    if (MHal_Get_Chip_ID() == CHIP_MT5896) { //Ovburst0 is only for 5896 currently.
	mhal_bdma_set_phase_checking(TRUE);
        OUTREG16((g_pmu_info.paganini)|(OFFSET(REG_PAGA_0X22)), 0x0000); //reg_paga_ctrl_11
        PMU_log(PMU_WARNING,"%s: ovburst = 0\n", __func__);
    }
    else {
        OUTREG16((g_pmu_info.paganini)|(OFFSET(REG_PAGA_0X22)), BIT14); //reg_paga_ctrl_11
    }

    //VREC
    INSREG8((g_pmu_info.vrec)|(OFFSET8(0x02)), 0x09, 0x09); //reg_vrec_ctrl01, [0] enable phase selection and decimation filter
    INSREG8((g_pmu_info.vad_0)|(OFFSET8(0x14)), 0x01, 0x01); //reg_vrec_ctrl0a
    INSREG8((g_pmu_info.paganini)|(OFFSET8(0x3b)), 0x0c, 0x04); //reg_paga_ctrl_1d
    INSREG8((g_pmu_info.vrec)|(OFFSET8(0xa6)), 0x01, 0x01); //reg_pp_ctrl3
    INSREG8((g_pmu_info.vrec)|(OFFSET8(0xa1)), 0x0f, 0x01); //reg_pp_ctrl0
    INSREG8((g_pmu_info.vad_0)|(OFFSET8(0x03)), 0xc0, 0xc0); //reg_vrec_ctrl01, [14] reset decimation filter 0:de-active 1:active
    INSREG8((g_pmu_info.vad_0)|(OFFSET8(0x23)), 0x80, 0x80); //reg_dmic_ctrl1
    INSREG8((g_pmu_info.paganini)|(OFFSET8(0x3a)), 0x02, 0x02); //reg_paga_ctrl_1d
    INSREG8((g_pmu_info.paganini)|(OFFSET8(0x3a)), 0x01, 0x01); //reg_paga_ctrl_1d
    INSREG8((g_pmu_info.paganini)|(OFFSET8(0x3c)), 0x01, 0x01); //reg_paga_ctrl_1e
    INSREG8((g_pmu_info.paganini)|(OFFSET8(0x3c)), 0x0c, 0x00); //reg_paga_ctrl_1e
    OUTREG8((g_pmu_info.vrec)|(OFFSET8(0x0c)), 0x1f); //reg_vrec_ctrl06, [0] enable deciamtion filter, ch1/ch2 [1] enable deciamtion filter, ch3/ch4 [2] enable deciamtion filter, ch5/ch6 [3] enable deciamtion filter, ch7/ch8 [4] enable deciamtion filter
    OUTREG8((g_pmu_info.vrec)|(OFFSET8(0x0d)), 0x81); //reg_vrec_ctrl06, [8] enable output clock  generation[15] enable SRAM initialization for decimation filter
    INSREG8((g_pmu_info.vrec)|(OFFSET8(0x08)), 0x11, 0x01); //reg_vrec_ctrl04, [4:0] auto selection for (sampling rate, decimation rate, output clock) (8k, 10, 400k)
    OUTREG8((g_pmu_info.vrec)|(OFFSET8(0x02)), 0x29); //reg_vrec_ctrl01, [0] enable phase selection and decimation filter [1] invert output clock [5:4] channel mode for digital microphone 0:1ch 1:2ch 2:4ch 3:8ch
    OUTREG8((g_pmu_info.vrec)|(OFFSET8(0x03)), 0x00); //reg_vrec_ctrl01
    OUTREG8((g_pmu_info.vrec)|(OFFSET8(0x20)), 0x00); //reg_dmic_ctrl0
    OUTREG8((g_pmu_info.vrec)|(OFFSET8(0x21)), 0x00); //reg_dmic_ctrl0
    OUTREG8((g_pmu_info.vrec)|(OFFSET8(0x40)), 0x09); //reg_cic_ctrl0, [0] enable decimation filter
    OUTREG8((g_pmu_info.vrec)|(OFFSET8(0x41)), 0x0f); //reg_cic_ctrl0, [0] enable decimation filter
    OUTREG8((g_pmu_info.vrec)|(OFFSET8(0x0c)), 0x13); //reg_vrec_ctrl06
    OUTREG8((g_pmu_info.vrec)|(OFFSET8(0x0d)), 0x05); //reg_vrec_ctrl06
    if (MHal_Get_Chip_ID() == CHIP_MT5879) {
        OUTREG8((g_pmu_info.vrec)|(OFFSET8(0x0d)), 0x01); //reg_vrec_ctrl06
    }
    OUTREG8((g_pmu_info.vrec)|(OFFSET8(0x22)), 0x01); //reg_dmic_ctrl1
    OUTREG8((g_pmu_info.vrec)|(OFFSET8(0x23)), 0x00); //reg_dmic_ctrl1
    INSREG8((g_pmu_info.vrec)|(OFFSET8(0x21)), 0x01, 0x01); //reg_dmic_ctrl0
    OUTREG8((g_pmu_info.vrec)|(OFFSET8(0x26)), 0x01); //reg_dmic_ctrl3, [0] enable phase select for ch3 and ch4
    OUTREG8((g_pmu_info.vrec)|(OFFSET8(0x27)), 0x00); //reg_dmic_ctrl3
    INSREG8((g_pmu_info.vrec)|(OFFSET8(0x21)), 0x02, 0x02); //reg_dmic_ctrl0
    OUTREG8((g_pmu_info.vrec)|(OFFSET8(0x42)), 0x01); //reg_cic_ctrl1, [0] order for decimation filter ch1/ch2
    OUTREG8((g_pmu_info.vrec)|(OFFSET8(0x44)), 0x01); //reg_cic_ctrl2, [0] order for decimation filter ch3/ch4
    OUTREG8((g_pmu_info.vrec)|(OFFSET8(0x46)), 0x01); //reg_cic_ctrl3, [0] order for decimation filter ch5/ch6
    OUTREG8((g_pmu_info.vrec)|(OFFSET8(0x48)), 0x01); //reg_cic_ctrl4, [0] order for decimation filter ch7/ch8
    OUTREG8((g_pmu_info.vrec)|(OFFSET8(0x08)), 0x19); //reg_vrec_ctrl04, [3:0]: BCK [5:4]: FS  BCK:  3M   FS: 16K
    INSREG8((g_pmu_info.vrec)|(OFFSET8(0x40)), 0x09, 0x00); //reg_cic_ctrl0
    INSREG8((g_pmu_info.vrec)|(OFFSET8(0x40)), 0x09, 0x01); //reg_cic_ctrl0, [0] enable decimation filter

    // PPCS REG setting
    OUTREG8((g_pmu_info.vrec)|(OFFSET8(0xa0)), 0x00); //reg_pp_ctrl0
    OUTREG8((g_pmu_info.vrec)|(OFFSET8(0xa1)), 0x00); //reg_pp_ctrl0
    OUTREG8((g_pmu_info.vrec)|(OFFSET8(0xa2)), 0x21); //reg_pp_ctrl1
    OUTREG8((g_pmu_info.vrec)|(OFFSET8(0xa3)), 0x43); //reg_pp_ctrl1
    OUTREG8((g_pmu_info.vrec)|(OFFSET8(0xa4)), 0x65); //reg_pp_ctrl2
    OUTREG8((g_pmu_info.vrec)|(OFFSET8(0xa5)), 0x87); //reg_pp_ctrl2
    OUTREG8((g_pmu_info.vrec)|(OFFSET8(0xa6)), 0x0a); //reg_pp_ctrl3
    OUTREG8((g_pmu_info.vrec)|(OFFSET8(0xa7)), 0x73); //reg_pp_ctrl3
    OUTREG8((g_pmu_info.vrec)|(OFFSET8(0xa8)), 0x00); //reg_pp_ctrl4
    OUTREG8((g_pmu_info.vrec)|(OFFSET8(0xa9)), 0x00); //reg_pp_ctrl4
    OUTREG8((g_pmu_info.vrec)|(OFFSET8(0xaa)), 0x00); //reg_pp_ctrl5
    OUTREG8((g_pmu_info.vrec)|(OFFSET8(0xab)), 0x00); //reg_pp_ctrl5
    OUTREG8((g_pmu_info.vrec)|(OFFSET8(0xac)), 0x00); //reg_pp_ctrl6
    OUTREG8((g_pmu_info.vrec)|(OFFSET8(0xad)), 0x00); //reg_pp_ctrl6
    OUTREG8((g_pmu_info.vrec)|(OFFSET8(0xae)), 0x06); //reg_pp_ctrl7
    OUTREG8((g_pmu_info.vrec)|(OFFSET8(0xaf)), 0x07); //reg_pp_ctrl7
    OUTREG8((g_pmu_info.vrec)|(OFFSET8(0xb0)), 0x00); //reg_pp_ctrl8
    OUTREG8((g_pmu_info.vrec)|(OFFSET8(0xb1)), 0x00); //reg_pp_ctrl8
    OUTREG8((g_pmu_info.vrec)|(OFFSET8(0xb2)), 0x00); //reg_pp_ctrl9
    OUTREG8((g_pmu_info.vrec)|(OFFSET8(0xb3)), 0x00); //reg_pp_ctrl9

    //dma setting
    OUTREG16((g_pmu_info.vrec)|(OFFSET(0x6a)), 0x0100);
    OUTREG16((g_pmu_info.vrec)|(OFFSET(0x60)), 0x8000);
    OUTREG16((g_pmu_info.vrec)|(OFFSET(0x60)), 0x0009);
    //aec_dma setting
    OUTREG16((g_pmu_info.paganini)|(OFFSET(0xca)), 0x0100); //overrun_th
    OUTREG16((g_pmu_info.paganini)|(OFFSET(0xc0)), 0x8000); //RIU_AIDMA_W1_CFG, [15]sft_reset
    OUTREG16((g_pmu_info.paganini)|(OFFSET(0xc0)), 0x0009); //RIU_AIDMA_W1_CFG, [0]:miu_req_en, [3]:lr_swap

    //i2s tdm rx setting
    INSREG8((g_pmu_info.paganini)|(OFFSET8(0x30)), 0x01, 0x01); //reg_paga_ctrl_18
    INSREG8((g_pmu_info.paganini)|(OFFSET8(0x30)), 0x10, 0x10); //reg_paga_ctrl_18
    INSREG8((g_pmu_info.paganini)|(OFFSET8(0x30)), 0x40, 0x40); //reg_paga_ctrl_18
    INSREG8((g_pmu_info.paganini)|(OFFSET8(0xec)), 0x02, 0x02); //reg_nf_synth_en_trig
    INSREG8((g_pmu_info.paganini)|(OFFSET8(0x61)), 0x80, 0x80); //reg_i2s_tdm_cfg_00

    //aec dma crossdie regen setting
    INSREG8((g_pmu_info.paganini)|(OFFSET8(0x84)), 0x03, 0x00); //reg_paga_dma_seq_en_bypass
    INSREG8((g_pmu_info.paganini)|(OFFSET8(0x84)), 0xc0, 0x40); //reg_paga_dma_seq_en_bypass
    INSREG16((g_pmu_info.vrec)|(OFFSET(0x04)), 0x0001, 0x0001); //reg_vrec_ctrl02, [0] enable phase selection and decimation filter

    // PLL 624MHz
    //wriu -b 0x0043203 0x01 0x00
    //wriu -b 0x0043202 0x04 0x00
    //wriu -b 0x0043202 0x08 0x00
    //wriu -b 0x0043202 0x10 0x00
    //wriu -b 0x0043202 0x20 0x00
    //wriu -b 0x0043202 0x40 0x00
    //wriu -b 0x0043207 0x07 0x00
    //wriu -b 0x0043207 0x20 0x00
    //wriu -b 0x0043204 0x30 0x00
    //wriu -b 0x0043205 0x03 0x00
    //wriu -b 0x0043206 0xff 0x34

    // AC ON
    //wriu -w 0x00432e0  0x8001

    //wriu -b 0x0000d1e 0x01 0x00
    //wriu -b 0x0000d0c 0x01 0x00
    //wriu -b 0x0000d02 0x01 0x00
    //wriu -b 0x0000d06 0x01 0x00
    //wriu -b 0x0000d08 0x01 0x00
    //wriu -b 0x0000d0a 0x01 0x00
    //wriu -b 0x00432e6 0x02 0x00
    //wriu -b 0x00432f3 0x02 0x02
    //wriu -b 0x00432f7 0x02 0x02

    //wriu -b 0x0000c16 0x1f 0x08
    //wriu -b 0x0000c0e 0x1f 0x04
    //wriu -b 0x0000c1e 0x0f 0x08
    //wriu -b 0x0000c10 0x1f 0x10

    //wriu -b 0x0000c26 0x07 0x04

    //wriu -b 0x00432ec 0x02 0x02
    //wriu -b 0x0043261 0x80 0x80

    // CM4 TSV & SYSTICK
    //wriu -b 0x0000c20 0x1f 0x04
    //wriu -b 0x0000c1c 0x0f 0x04
    //wriu -b 0x0000c00 0x07 0x04
    //wriu -b 0x0000c04 0x0f 0x04
    //wriu -b 0x0000c02 0x07 0x04
    //wriu -b 0x0000c0a 0x07 0x04
    //wriu -b 0x0000c06 0x07 0x04

    //wriu -b 0x00432f6 0x3f 0x1f
    //wriu -b 0x00432f7 0x81 0x00
    //wriu -b 0x00432f7 0x81 0x01
    //wriu -b 0x00432f7 0x81 0x00

    // IMI CLK  XTAIL -> DFS OFF -> PLL -> DFS_ON
    //wriu -b 0x0000c18 0x0f 0x04
    //wriu -b 0x0000c1a 0x0f 0x04

    //wriu -b 0x00432f2 0x3f 0x1f
    //wriu -b 0x00432f3 0x81 0x00
    //wriu -b 0x00432f3 0x81 0x01
    //wriu -b 0x00432f3 0x81 0x00

    // CLK Release
    //wriu -b 0x0000d1e 0x01 0x01
    //wriu -b 0x0000d0c 0x01 0x01
    //wriu -b 0x0000d02 0x01 0x01
    //wriu -b 0x0000d06 0x01 0x01
    //wriu -b 0x0000d08 0x01 0x01
    //wriu -b 0x0000d0a 0x01 0x01
    //wriu -b 0x0000d90 0x07 0x07 //reg_sw_en_mcu_pmu2cm4
    //wriu -b 0x00432e6 0x02 0x02

    // DMIC BCK Gen Clk
    //wriu -b 0x0000c22 0x0f 0x0c
    //wriu -b 0x0000c12 0x0f 0x08

    //dma setting
    //wriu -w 0x0043162  0x0100
    //wriu -w 0x004316a  0x0100
    //wriu -w 0x0043160  0x8000
    //wriu -w 0x0043160  0x020b

    //wriu -w 0x0043180  0x0003

    //wriu -w 0x0043122  0x0001
    //wriu -w 0x0043124  0x8000
    //wriu -w 0x0043126  0x0001
    //wriu -w 0x0043128  0x8000
    //wriu -w 0x004312A  0x0001
    //wriu -w 0x004312C  0x8000
    //wriu -w 0x004312E  0x0001
    //wriu -w 0x0043130  0x8000
    //wriu -w 0x0043120  0x0000
    //wriu -w 0x004310C  0x011F

    //wriu -w 0x0043222  0x4000

    //VREC
    //wriu -b 0x0043102 0x09 0x09
    //wriu -b 0x0043014 0x01 0x01
    //wriu -b 0x004323b 0x0c 0x04
    //wriu -b 0x00431a6 0x01 0x01
    //wriu -b 0x00431a1 0x0f 0x01
    //wriu -b 0x0043003 0xc0 0xc0
    //wriu -b 0x0043023 0x80 0x80
    //wriu -b 0x004323a 0x02 0x02
    //wriu -b 0x004323a 0x01 0x01
    //wriu -b 0x004323c 0x01 0x01
    //wriu -b 0x004323c 0x0c 0x00
    //wriu -b 0x004310c 0xff 0x1f
    //wriu -b 0x004310d 0xff 0x81
    //wriu -b 0x0043108 0x11 0x01
    //wriu -b 0x0043102 0xff 0x29
    //wriu -b 0x0043103 0xff 0x00
    //wriu -b 0x0043120 0xff 0x00
    //wriu -b 0x0043121 0xff 0x00
    //wriu -b 0x0043140 0xff 0x09
    //wriu -b 0x0043141 0xff 0x0f
    //wriu -b 0x004310c 0xff 0x13
    //wriu -b 0x004310d 0xff 0x05
    //wriu -b 0x0043122 0xff 0x01
    //wriu -b 0x0043123 0xff 0x00
    //wriu -b 0x0043121 0x01 0x01
    //wriu -b 0x0043126 0xff 0x01
    //wriu -b 0x0043127 0xff 0x00
    //wriu -b 0x0043121 0x02 0x02
    //wriu -b 0x0043142 0xff 0x01
    //wriu -b 0x0043144 0xff 0x01
    //wriu -b 0x0043146 0xff 0x01
    //wriu -b 0x0043148 0xff 0x01
    //wriu -b 0x0043108 0xff 0x19 //reg_vrec_ctrl04, [5:0]    [5:4]: FS    [3:0]: BCK
                                //[5:4]FS:  0: 8K,   1: 16K,   2: 32K   , 3:48KHZ
                                //[3:0]BCK:  0: 100k,   1: 400K  , 2: 800K, 3: 1000K ,4: 12000K ,5:1600K ,6: 2000K ,7: 3000K , 8: 4000K HZ
                                // p.s.     BCK: 1000kHZ , Fs: 48k   not support
    //wriu -b 0x0043140 0x09 0x00
    //wriu -b 0x0043140 0x09 0x01

    // PPCS REG setting
    //wriu -b 0x00431a0 0xff 0x00
    //wriu -b 0x00431a1 0xff 0x00
    //wriu -b 0x00431a2 0xff 0x21
    //wriu -b 0x00431a3 0xff 0x43
    //wriu -b 0x00431a4 0xff 0x65
    //wriu -b 0x00431a5 0xff 0x87
    //wriu -b 0x00431a6 0xff 0x0a
    //wriu -b 0x00431a7 0xff 0x73
    //wriu -b 0x00431a8 0xff 0x00
    //wriu -b 0x00431a9 0xff 0x00
    //wriu -b 0x00431aa 0xff 0x00
    //wriu -b 0x00431ab 0xff 0x00
    //wriu -b 0x00431ac 0xff 0x00
    //wriu -b 0x00431ad 0xff 0x00
    //wriu -b 0x00431ae 0xff 0x06
    //wriu -b 0x00431af 0xff 0x07
    //wriu -b 0x00431b0 0xff 0x00
    //wriu -b 0x00431b1 0xff 0x00
    //wriu -b 0x00431b2 0xff 0x00
    //wriu -b 0x00431b3 0xff 0x00

    //dma setting
    //wriu -w 0x004316a 0x0100
    //wriu -w 0x0043160 0x8000
    //wriu -w 0x0043160 0x0009

    //aec_dma setting
    //wriu -w 0x00432ca 0x0100
    //wriu -w 0x00432c0 0x8000
    //wriu -w 0x00432c0 0x0009

    //i2s tdm rx setting
    //wriu -b 0x0043230 0x01 0x01
    //wriu -b 0x0043230 0x10 0x10
    //wriu -b 0x0043230 0x40 0x40
    //wriu -b 0x00432ec 0x02 0x02
    //wriu -b 0x0043261 0x80 0x80

    //aec dma crossdie regen setting
    //wriu -b 0x0043284 0x03 0x00
    //wriu -b 0x0043284 0xc0 0x40

    //wriu -b 0x0043104 0x0001 0x0001
}

void mhal_cm4_stop(void)
{
	//sw reset cm4 cpu
	CLRREG16((g_pmu_info.pm_misc) | (OFFSET(REG_PM_MISC_0X4E)), BIT4);
	// [0]:soft reset [1]:software reset for per-project peripherals
	CLRREG16((g_pmu_info.coin_cpu) | (OFFSET(REG_COIN_CPU_0X00)), BIT0 | BIT1);

	//reg_sw_en_imi2paganini_imi
	INSREG8((g_pmu_info.ckgen01_pm) | (OFFSET8(REG_CKGEN1_PM_0X1E)), 0x01, 0x00);
	//reg_sw_en_cm42cm4
	INSREG8((g_pmu_info.ckgen01_pm) | (OFFSET8(REG_CKGEN1_PM_0X0C)), 0x01, 0x00);
	//reg_sw_en_cm4_core2cm4_core
	INSREG8((g_pmu_info.ckgen01_pm) | (OFFSET8(REG_CKGEN1_PM_0X02)), 0x01, 0x00);
	//reg_sw_en_cm4_systick2cm4_systick
	INSREG8((g_pmu_info.ckgen01_pm) | (OFFSET8(REG_CKGEN1_PM_0X06)), 0x01, 0x00);
	//reg_sw_en_cm4_tck2tck_int
	INSREG8((g_pmu_info.ckgen01_pm) | (OFFSET8(REG_CKGEN1_PM_0X08)), 0x01, 0x00);
	//reg_sw_en_cm4_tsvalueb2cm4_tsvalueb
	INSREG8((g_pmu_info.ckgen01_pm) | (OFFSET8(REG_CKGEN1_PM_0X0A)), 0x01, 0x00);
}

void mhal_cm4_halt(void)
{
    CLRREG16((g_pmu_info.pm_misc)|(OFFSET(0x4E)), 0x10); //reset cm4 cpu
    MHal_CPUINT_Clear(E_CPUINT_CP, E_CPUINT_HK0);
    CLRREG16((g_pmu_info.inturrpt)|(OFFSET(0x4E)), 0x10); //fiq113 HK0 to CP

    //CLRREG16((GET_REG16_ADDR(PM_MISC_BASE_ADDR, 0x27), 0x10); //reset cm4 cpu
    //MHal_CPUINT_Clear(E_CPUINT_CP, E_CPUINT_HK0);
}

void mhal_cm4_run(void)
{
    SETREG16((g_pmu_info.pm_misc)|(OFFSET(0x4E)), 0x10); //reset cm4 cpu
    SETREG16((g_pmu_info.coin_cpu)|(OFFSET(0x00)), 0x03); // [0]:soft reset [1]:software reset for per-project peripherals

    //SETREG16(GET_REG16_ADDR(PM_MISC_BASE_ADDR, 0x27), 0x10);
}

void mhal_cm4_dc_on(void)
{
    INSREG8((g_pmu_info.ckgen00_pm)|(OFFSET(0x1a)), 0x0f, 0x08);
    //reg_ckg_paganini_imi_dfs [3:2]:
    //clkgfmux_imi_dfs_ref_int_ck.sel
    //0: xtal, 1: pagapll_div2, 2:smi_ck

    //wriu -b 0x0000c1a 0x0f 0x08
}

void mhal_cm4_dc_off(void)
{
    //switch to AC ON stage
    INSREG8((g_pmu_info.ckgen00_pm)|(OFFSET(0x18)), 0x0f, 0x04);
    //reg_ckg_paganini_imi_dfs [3:2]:
    //clkgfmux_imi_dfs_ref_int_ck.sel
    //0: xtal, 1: pagapll_div2, 2:smi_ck

    //wriu -b 0x0000c18 0x0f 0x04
}

void mhal_cm4_init_off(void)
{
    // turn off CM4 clock
    INSREG8((g_pmu_info.paganini)|(OFFSET8(0x02)), 0x04, 0x04); // CM4 PLL PD
    INSREG8((g_pmu_info.paganini)|(OFFSET8(0xF0)), 0x01, 0x00); // XTAIL off
    OUTREG8((g_pmu_info.paganini)|(OFFSET8(0x38)), 0xff); // power off audio sram/imi
    OUTREG8((g_pmu_info.paganini)|(OFFSET8(0x39)), 0xff); // power off audio sram/imi
    INSREG8((g_pmu_info.paganini)|(OFFSET8(0x27)), 0x80, 0x80); // power off audio sram/imi
    OUTREG8((g_pmu_info.paganini)|(OFFSET8(0xf8)), 0x00); // cm4 clk off
    OUTREG8((g_pmu_info.paganini)|(OFFSET8(0xf9)), 0x00); // cm4 clk off
    OUTREG8((g_pmu_info.paganini)|(OFFSET8(0xf4)), 0x00); // imi clk off
    OUTREG8((g_pmu_info.paganini)|(OFFSET8(0xf5)), 0x00); // imi clk off

    OUTREG8((g_pmu_info.paganini)|(OFFSET8(0x30)), 0x00); // NF synthizer off
    OUTREG8((g_pmu_info.paganini)|(OFFSET8(0x31)), 0x00); // NF synthizer off

    //wriu -b 0x043202 0x04 0x04 // CM4 PLL PD
    //wriu -b 0x0432F0 0x01 0x00 // XTAIL off
    //wriu -b 0x043238 0xff 0xff // power off audio sram/imi
    //wriu -b 0x043239 0xff 0xff // power off audio sram/imi
    //wriu -b 0x043227 0x80 0x80 // po//wer off audio sram/imi
    //wriu -b 0x0432f8 0xff 0x00 // cm4 clk off
    //wriu -b 0x0432f9 0xff 0x00 // cm4 clk off
    //wriu -b 0x0432f4 0xff 0x00 // imi clk off
    //wriu -b 0x0432f5 0xff 0x00 // imi clk off

    //wriu -b 0x043230 0xff 0x00 // NF synthizer off
    //wriu -b 0x043231 0xff 0x00 // NF synthizer off
}
