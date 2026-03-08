// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

#include <common.h>
#include <command.h>
#include <time.h>
#include <fs.h>
#include <MsCommon.h>
#include <debug_impl.h>
#include <iniutility.h>
#include <voc_impl.h>
#include <dt-bindings/mailbox/mt5896_mbox_chan_id_maps.h>
#include "mtk-pm.h"

#include <dm/uclass.h> //ofnode
#include <fdt_support.h>

DECLARE_GLOBAL_DATA_PTR;

#define MBOX_CM4_CONTROLLER_NAME	"mailbox_to_cm4" // arch/arm/dts/mt5896_mailbox.dtsi
#define MBOX_CM4_RX_TIMEOUT		(5000000)
#define MAX_CMD_SIZE			(62)
#define MAX_VOC_MBOX_DELAY		(100)
#define MAX_VOC_MBOX_RETRY		(100)

#define VAD_DEVNODE "/vad_rproc"
#define PMU_DEVNODE "/pmu_rproc"

#define WAKEUP_SOURCE "voice"
#define WAKEUP_BY_PM (2)

// Local Variables
//--------------------------------------------------------------------------------------------------


//--------------------------------------------------------------------------------------------------
// Functions
//--------------------------------------------------------------------------------------------------


int mtk_voc_get_value(void)
{
    char cmd[MAX_CMD_SIZE] = {0};
    int ret = 0;

    ret = snprintf(cmd, sizeof(cmd) - 1, "mbox recv %s %d %d", MBOX_CM4_CONTROLLER_NAME, MBOX_CHAN_ID_VOICE, MBOX_CM4_RX_TIMEOUT);
    if (ret < 0)
    {
        UBOOT_DEBUG("snprintf fail %d\n", ret);
    }
    else
    {
        UBOOT_DEBUG("run_command(%s, 0).\n", cmd);
        if (0 != run_command(cmd, 0))
        {
            UBOOT_ERROR("run_command(%s, 0) failure.\n", cmd);
            return -1;
        }
        UBOOT_DEBUG("mtk_voc_get_value\n");
    }
    return 0;
}

int check_cm4_dts_node(void)
{
    ofnode node_cm4;

    node_cm4 = ofnode_path(VAD_DEVNODE);
    if (ofnode_valid(node_cm4) && ofnode_is_available(node_cm4))
    {
        return 1;
    }

    node_cm4 = ofnode_path(PMU_DEVNODE);
    if (ofnode_valid(node_cm4) && ofnode_is_available(node_cm4))
    {
        return 1;
    }

    return 0;
}

int check_vad_dts_node(void)
{
    ofnode node_cm4;

    node_cm4 = ofnode_path(VAD_DEVNODE);
    if (ofnode_valid(node_cm4) && ofnode_is_available(node_cm4))
        return 1;

    return 0;
}

int mtk_voc_set_value(struct voc_env_info voc_info)
{
    int i = 0;
    char cmd[MAX_CMD_SIZE] = {0};
    int ret = 0;
    UBOOT_DEBUG("mtk_voc_set_value \n");

    if (check_cm4_dts_node() == 0)
    {
        UBOOT_DEBUG("Skipped second_standby mode due to no cm4 dts node\n");
        return 0;
    }

    // mbox format : u8 data[0]=cmd , u8 data[1]=value , u8 data[2]=value , u8 data[3] ,  ...., u8 data[61]
    if (voc_info.enable_mic)
    {
        ret = snprintf(cmd, sizeof(cmd) - 1, "mbox send %s %u %x %x", MBOX_CM4_CONTROLLER_NAME, MBOX_CHAN_ID_VOICE, VOICE_CMD_MIC_SWITCH, voc_info.enable_mic);
        if (ret < 0)
        {
            UBOOT_DEBUG("snprintf fail %d\n", ret);
        }
        else
        {
            UBOOT_DEBUG("run_command(%s, 0).\n", cmd);
            while (0 != run_command(cmd, 0))
            {
                i++;
                mdelay(MAX_VOC_MBOX_DELAY);
                if (i > MAX_VOC_MBOX_RETRY)
                {
                    UBOOT_ERROR("run_command(%s, 0) failure.\n", cmd);
                    break;
                }
            }
        }
    }

    i = 0;
    if (voc_info.enable_seamless)
    {
        ret = snprintf(cmd, sizeof(cmd) - 1, "mbox send %s %u %x %x", MBOX_CM4_CONTROLLER_NAME, MBOX_CHAN_ID_VOICE, VOICE_CMD_SEAMLESS, voc_info.enable_seamless);
        if (ret < 0)
        {
            UBOOT_DEBUG("snprintf fail %d\n", ret);
        }
        else
        {
            UBOOT_DEBUG("run_command(%s, 0).\n", cmd);
            while (0 != run_command(cmd, 0))
            {
                i++;
                mdelay(MAX_VOC_MBOX_DELAY);
                if (i > MAX_VOC_MBOX_RETRY)
                {
                    UBOOT_ERROR("run_command(%s, 0) failure.\n", cmd);
                    break;
                }
            }
        }
    }

    i = 0;
    if (voc_info.enable_wakeup)
    {
        ret = snprintf(cmd, sizeof(cmd) - 1, "mbox send %s %u %x %x", MBOX_CM4_CONTROLLER_NAME, MBOX_CHAN_ID_VOICE, VOICE_CMD_UBOOT_WAKEUP_ENABLE, voc_info.enable_wakeup);
        if (ret < 0)
        {
            UBOOT_DEBUG("snprintf fail %d\n", ret);
        }
        else
        {
            UBOOT_DEBUG("run_command(%s, 0).\n", cmd);
            while (0 != run_command(cmd, 0))
            {
                i++;
                mdelay(MAX_VOC_MBOX_DELAY);
                if (i > MAX_VOC_MBOX_RETRY)
                {
                    UBOOT_ERROR("run_command(%s, 0) failure.\n", cmd);
                    break;
                }
            }
        }
    }

    return 0;
}

int mtk_voc_standby(struct voc_env_info voc_info)
{
    int i = 0;
    bool wakeup_enable = 0;
    char cmd[MAX_CMD_SIZE] = {0};
    int ret = 0;
    UBOOT_DEBUG("mtk_voc_standby \n");

    if (check_cm4_dts_node() == 0)
    {
        UBOOT_DEBUG("Skipped second_standby mode due to no cm4 dts node\n");
        return 0;
    }

    ret = pm_get_wakeup_config(WAKEUP_SOURCE, &wakeup_enable);
    if (ret < 0)
    {
        UBOOT_ERROR("Skipped voc standby due to doesn't know wakeup source\n");
        return 0;
    }

    if (wakeup_enable == 1 && voc_info.enable_wakeup >= WAKEUP_BY_PM)
    {
        return 0;
    }

    // mbox format : u8 data[0]=cmd , u8 data[1]=value , u8 data[2]=value , u8 data[3] ,  ...., u8 data[61]
    if (voc_info.enable_wakeup)
    {
        ret = snprintf(cmd, sizeof(cmd) - 1, "mbox send %s %u %x %x", MBOX_CM4_CONTROLLER_NAME, MBOX_CHAN_ID_VOICE, VOICE_CMD_UBOOT_WAKEUP_ENABLE, 0);
        if (ret < 0)
        {
            UBOOT_DEBUG("snprintf fail %d\n", ret);
        }
        else
        {
            UBOOT_DEBUG("run_command(%s, 0).\n", cmd);
            while (0 != run_command(cmd, 0))
            {
                i++;
                mdelay(MAX_VOC_MBOX_DELAY);
                if (i > MAX_VOC_MBOX_RETRY)
                {
                    UBOOT_ERROR("run_command(%s, 0) failure.\n", cmd);
                    break;
                }
            }
        }
    }

    if (voc_info.enable_mic)
    {
        ret = snprintf(cmd, sizeof(cmd) - 1, "mbox send %s %u %x %x", MBOX_CM4_CONTROLLER_NAME, MBOX_CHAN_ID_VOICE, VOICE_CMD_MIC_SWITCH, 0);
        if (ret < 0)
        {
            UBOOT_DEBUG("snprintf fail %d\n", ret);
        }
        else
        {
            UBOOT_DEBUG("run_command(%s, 0).\n", cmd);
            while (0 != run_command(cmd, 0))
            {
                i++;
                mdelay(MAX_VOC_MBOX_DELAY);
                if (i > MAX_VOC_MBOX_RETRY)
                {
                    UBOOT_ERROR("run_command(%s, 0) failure.\n", cmd);
                    break;
                }
            }
        }
    }

    if (check_vad_dts_node() == 1)
    {
        ret = snprintf(cmd, sizeof(cmd) - 1, "stop_vad");
        if (ret < 0)
        {
            UBOOT_DEBUG("snprintf fail %d\n", ret);
        }
        else
        {
            UBOOT_DEBUG("run_command(%s, 0).\n", cmd);
            while (0 != run_command(cmd, 0))
            {
                i++;
                mdelay(MAX_VOC_MBOX_DELAY);
                if (i > MAX_VOC_MBOX_RETRY)
                {
                    UBOOT_ERROR("run_command(%s, 0) failure.\n", cmd);
                    break;
                }
            }
        }
    }

    return 0;
}
