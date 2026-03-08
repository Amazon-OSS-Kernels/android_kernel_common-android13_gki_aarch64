// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (c) 2023 MediaTek Inc.
*/

#include <common.h>
#include <exports.h>
#include <time.h>
#include <utility.h>
#include <command.h>
#include <debug_impl.h>
#include <iniutility.h>
#include <jpd_impl.h>
#include <mtk_jpd.h>
#include <mtk_gegop.h>
#include <mtk_panel.h>
#include <mtk_rawdata.h>
#include <standby_mode.h>
#include <mtk-pm.h>
#ifdef CONFIG_DATA_SEPARATION
#include <mtk_dataindex.h>
#define LOGO_SECTION        "BootLogoMusic"
#define LOGO_INI_KEY        "m_pBootLogoCfg_File"
#define LOGO_JPG_KEY            "m_pBootLogo_JPG_File"
#define LOGO_RAW_KEY            "m_pBootLogo_RAW_File"
#define AVB_ORANGE_STATE_RAW_KEY            "m_pboot_avbOrangeState_RAW_File"
#define AVB_RED_STATE_RAW_KEY            "m_pboot_avbRedState_RAW_File"

#endif
#define DEFAULT_LOGO_PART   "bootdata"
#define DEFAULT_LOGO_INI    "/bootlogo.ini"
#define ARGV_AVB_STATE  2

int do_raw_data_logo_load (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    int ret = -1;
    uint64_t start,end;
    const char *avb_state = NULL;
#ifdef CONFIG_DATA_SEPARATION
    char raw_logo_select[BOOTARGS_CFG_LEN]={0};
#endif

    if(argc > ARGV_AVB_STATE)
    {
        avb_state = argv[ARGV_AVB_STATE];
        UBOOT_DEBUG("avb_state %s\n",avb_state);
    }

#ifdef CONFIG_DATA_SEPARATION
    char filepath[FILE_PATH_SIZE], part[PART_NAME_SIZE];
    const char *relpath;
#endif
#if (CONFIG_HAPS == 1)
    char *ptr;
    ptr = env_get("two_haps");
    if(ptr == NULL)
        return 0;
    else if(strncmp(ptr,ENV_SETTING_ON,ENV_SETTING_ON_SIZE) != 0)
        return 0;
#endif

    start = get_timer(0);
#ifdef CONFIG_DATA_SEPARATION
    memset(part, 0, sizeof(part));
    memset(filepath, 0, sizeof(filepath));
    if(avb_state != NULL)
    {
        if(strncmp(avb_state, "orange", strlen("orange")) == 0)
        {
            if(strlen(AVB_ORANGE_STATE_RAW_KEY) >= BOOTARGS_CFG_LEN)
            {
                strncpy(raw_logo_select, AVB_ORANGE_STATE_RAW_KEY, BOOTARGS_CFG_LEN-1);
                raw_logo_select[BOOTARGS_CFG_LEN-1] = '\0';
            }
            else
            {
                strncpy(raw_logo_select, AVB_ORANGE_STATE_RAW_KEY, strlen(AVB_ORANGE_STATE_RAW_KEY));
                raw_logo_select[strlen(AVB_ORANGE_STATE_RAW_KEY)] = '\0';
            }
        }
        else if(strncmp(avb_state, "red", strlen("red")) == 0)
        {
            if(strlen(AVB_RED_STATE_RAW_KEY) >= BOOTARGS_CFG_LEN)
            {
                strncpy(raw_logo_select, AVB_RED_STATE_RAW_KEY, BOOTARGS_CFG_LEN-1);
                raw_logo_select[BOOTARGS_CFG_LEN-1] = '\0';
            }
            else
            {
                strncpy(raw_logo_select, AVB_RED_STATE_RAW_KEY, strlen(AVB_RED_STATE_RAW_KEY));
                raw_logo_select[strlen(AVB_ORANGE_STATE_RAW_KEY)] = '\0';
            }
        }
    }
    else
    {
        if(strlen(LOGO_RAW_KEY) >= BOOTARGS_CFG_LEN)
        {
            strncpy(raw_logo_select, LOGO_RAW_KEY, BOOTARGS_CFG_LEN-1);
            raw_logo_select[BOOTARGS_CFG_LEN-1] = '\0';
        }
        else
        {
            strncpy(raw_logo_select, LOGO_RAW_KEY, strlen(LOGO_RAW_KEY));
            raw_logo_select[strlen(AVB_ORANGE_STATE_RAW_KEY)] = '\0';
        }
    }

    if(dataindex_get_key(filepath, FILE_PATH_SIZE, LOGO_SECTION, (const char *)raw_logo_select, NULL) == 0)
    {
        if(dataindex_resolve_path(part, PART_NAME_SIZE, &relpath, filepath) == 0)
        {
            ret = mtk_raw_data_load_logo_to_dram(part, relpath);
            if(ret < 0)
            {
                UBOOT_DEBUG("Load raw data logo to DRAM failure with data separation\n");
                ret = mtk_raw_data_load_logo_to_dram(DEFAULT_LOGO_PART, argv[1]);
            }
        }
        else
        {
            UBOOT_DEBUG("resolve path fail: %s",filepath);
            ret = mtk_raw_data_load_logo_to_dram(DEFAULT_LOGO_PART, argv[1]);
        }
    }
    else
    {
        UBOOT_DEBUG("cannot get %s:%s from dataindex file\n",LOGO_SECTION,raw_logo_select);
#endif
        ret = mtk_raw_data_load_logo_to_dram(DEFAULT_LOGO_PART, argv[1]);
#ifdef CONFIG_DATA_SEPARATION
    }
#endif
    if(ret < 0)
    {
        UBOOT_ERROR("Load raw data logo to DRAM failure with default setting\n");
        return -1;
    }

    end = get_timer(0);
    UBOOT_BOOTTIME("[Read Logo][start:%llu][end:%llu][total time:%llu]\n", start, end, end-start);
    return 0;
}

U_BOOT_CMD(
    rawlogo, CONFIG_SYS_MAXARGS, 1,    do_raw_data_logo_load,
    "rawlogo   - Read raw data logo to memory\n",
    NULL
);

#if defined(CONFIG_JPEG_DECODER)
int do_jpeg_decode (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    int ret = -1;
    uint64_t start,end;
#ifdef CONFIG_DATA_SEPARATION
    char filepath[FILE_PATH_SIZE], part[PART_NAME_SIZE];
    const char *relpath;
#endif
#if (CONFIG_HAPS == 1)
    char *ptr;
    ptr = env_get("two_haps");
    if(ptr == NULL)
        return 0;
    else if(strncmp(ptr,ENV_SETTING_ON,ENV_SETTING_ON_SIZE) != 0)
        return 0;
#endif

    start = get_timer(0);
#ifdef CONFIG_DATA_SEPARATION
    memset(part, 0, sizeof(part));
    memset(filepath, 0, sizeof(filepath));
    if(dataindex_get_key(filepath, FILE_PATH_SIZE, LOGO_SECTION, LOGO_JPG_KEY, NULL) == 0)
    {
        if(dataindex_resolve_path(part, PART_NAME_SIZE, &relpath, filepath) == 0)
        {
            ret = mtk_jpd_decode(part, relpath);
            if(ret == 0)
            {
                UBOOT_DEBUG("Decode jpeg bootlogo failed with data separation\n");
                ret = mtk_jpd_decode(DEFAULT_LOGO_PART, argv[1]);
            }
        }
        else
        {
            UBOOT_DEBUG("resolve path fail: %s",filepath);
            ret = mtk_jpd_decode(DEFAULT_LOGO_PART, argv[1]);
        }
    }
    else
    {
        UBOOT_DEBUG("cannot get %s:%s from dataindex file\n",LOGO_SECTION,LOGO_JPG_KEY);
#endif
        ret = mtk_jpd_decode(DEFAULT_LOGO_PART, argv[1]);
#ifdef CONFIG_DATA_SEPARATION
    }
#endif

    if(ret == 0)
    {
        UBOOT_ERROR("Decode jpeg bootlogo failed with default setting\n");
        return -1;
    }
    end = get_timer(0);
    UBOOT_BOOTTIME("[Decode Logo][start:%llu][end:%llu][total time:%llu]\n", start, end, end-start);
    return 0;
}

U_BOOT_CMD(
    jpgdecode, CONFIG_SYS_MAXARGS, 1,    do_jpeg_decode,
    "jpgdecode   - Decode JPEG format picture\n",
    NULL
);
#endif

int do_showlogo (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    int ret = 0;
    logo_disp_config_params logo_config;
    unsigned int logo_output_buf;
    unsigned int logo_output_buf_size;
    uint64_t start,end;
    struct bootlogo_info logo_info;
    char command[COMMAND_BUF_SIZE] = {0};
#ifdef CONFIG_DATA_SEPARATION
    char filepath[FILE_PATH_SIZE], part[PART_NAME_SIZE];
    const char *relpath;
#endif
#if (CONFIG_HAPS == 1)
    char *ptr;
    ptr = env_get("two_haps");
    if(ptr == NULL)
        return 0;
    else if(strncmp(ptr,ENV_SETTING_ON,ENV_SETTING_ON_SIZE) != 0)
        return 0;
#endif

    UBOOT_TRACE("IN\n");

    const char *avb_state = NULL;
    if (argc == ARGV_AVB_STATE)
    {
        avb_state = argv[1];
    }

    if(pm_check_back_ground_active() == 1)
    {
        printf("pm_check_back_ground_active skip boot logo !!!! \n");
        return 0;
    }

    if(is_recovery_mode() == 1)
    {
        printf("is_recovery_mode show boot logo !!!! \n");
    }

    start = get_timer(0);
    time_stamp_store_to_register(start,(sizeof(int)/sizeof(short)));
    if(mtk_is_panel_ready() == 0)
    {
        UBOOT_ERROR("Panel is not ready\n");
        return -1;
    }

    memset(&logo_info, 0, sizeof(logo_info));
    if (avb_state != NULL)
    {
        if(strncmp(avb_state, "orange", strlen("orange")) == 0  || strncmp(avb_state, "red", strlen("red")) == 0)
        {
#ifdef CONFIG_DATA_SEPARATION
            memset(part, 0, sizeof(part));
            memset(filepath, 0, sizeof(filepath));
            if(dataindex_get_key(filepath, FILE_PATH_SIZE, LOGO_SECTION, LOGO_INI_KEY, NULL) == 0)
            {
                if(dataindex_resolve_path(part, PART_NAME_SIZE, &relpath, filepath) == 0)
                {
                    ret = get_bootlogo_info(part, relpath, &logo_info, avb_state);
                    if(ret < 0)
                    {
                        UBOOT_DEBUG("Read bootlogo information failure with data separation\n");
                        ret = get_bootlogo_info(DEFAULT_LOGO_PART, DEFAULT_LOGO_INI, &logo_info, avb_state);
                    }
                }
                else
                {
                    UBOOT_DEBUG("resolve path fail: %s",filepath);
                    ret = get_bootlogo_info(DEFAULT_LOGO_PART, DEFAULT_LOGO_INI, &logo_info, avb_state);
                }
            }
            else
            {
                UBOOT_DEBUG("cannot get %s:%s from dataindex file\n",LOGO_SECTION,LOGO_INI_KEY);
#endif
                ret = get_bootlogo_info(DEFAULT_LOGO_PART, DEFAULT_LOGO_INI, &logo_info, avb_state);
#ifdef CONFIG_DATA_SEPARATION
            }
#endif
        }
        else
        {
            UBOOT_ERROR("avb state <%s> is not supported. Please check bootlogo.ini\n", avb_state);
            return -1;
        }
    }
    else
    {
#ifdef CONFIG_DATA_SEPARATION
        memset(part, 0, sizeof(part));
        memset(filepath, 0, sizeof(filepath));
        if(dataindex_get_key(filepath, FILE_PATH_SIZE, LOGO_SECTION, LOGO_INI_KEY, NULL) == 0)
        {
            if(dataindex_resolve_path(part, PART_NAME_SIZE, &relpath, filepath) == 0)
            {
                ret = get_bootlogo_info(part, relpath, &logo_info, NULL);
                if(ret < 0)
                {
                    UBOOT_DEBUG("Read bootlogo information failure with data separation\n");
                    ret = get_bootlogo_info(DEFAULT_LOGO_PART, DEFAULT_LOGO_INI, &logo_info, NULL);
                }
            }
            else
            {
                UBOOT_DEBUG("resolve path fail: %s",filepath);
                ret = get_bootlogo_info(DEFAULT_LOGO_PART, DEFAULT_LOGO_INI, &logo_info, NULL);
            }
        }
        else
        {
            UBOOT_DEBUG("cannot get %s:%s from dataindex file\n",LOGO_SECTION,LOGO_INI_KEY);
#endif
            ret = get_bootlogo_info(DEFAULT_LOGO_PART, DEFAULT_LOGO_INI, &logo_info, NULL);
#ifdef CONFIG_DATA_SEPARATION
        }
#endif
    }

    if(ret < 0)
    {
        UBOOT_ERROR("Read bootlogo information failure with default setting\n");
        return -1;
    }

    if(strncmp(logo_info.type, LOGO_FORMAT_RAW, LOGO_FORMAT_RAW_STRING_SIZE) == 0)
    {
        if (strstr(logo_info.path, " ") != NULL || strstr(logo_info.path, ";") != NULL)
        {
            UBOOT_DEBUG("Warning! space or ; exist, please remove it\n");
            return -1;
        }
        if(avb_state != NULL)
            ret = snprintf(command, sizeof(command), "rawlogo %s %s", logo_info.path, avb_state);
        else
            ret = snprintf(command, sizeof(command), "rawlogo %s", logo_info.path);
        if(ret < 0)
            return -1;
        ret = run_command(command, 0);
        if(ret != 0)
            return -1;
        logo_output_buf = mtk_raw_data_get_output_buffer_addr();
        logo_output_buf_size = mtk_raw_data_get_output_buffer_size();
    }
#if defined(CONFIG_JPEG_DECODER)
    else if(strncmp(logo_info.type, LOGO_FORMAT_JPEG, LOGO_FORMAT_JPEG_STRING_SIZE) == 0)
    {
        if (strstr(logo_info.path, " ") != NULL || strstr(logo_info.path, ";") != NULL)
        {
            UBOOT_DEBUG("Warning! space or ; exist, please remove it\n");
            return -1;
        }
        ret = snprintf(command, sizeof(command), "jpgdecode %s", logo_info.path);
        if(ret < 0)
            return -1;
        ret = run_command(command, 0);
        if(ret != 0)
            return -1;
        logo_output_buf = mtk_jpd_get_output_buffer_addr();
        logo_output_buf_size = mtk_jpd_get_output_buffer_size();
    }
#endif
    else
    {
        UBOOT_ERROR("Bootlogo type is unsupported. Please check bootlogo.ini.\n");
        return -1;
    }

    if(argc > 4)
    {
        logo_config.disp_x = simple_strtoul(argv[1], NULL, 10);
        logo_config.disp_y = simple_strtoul(argv[2], NULL, 10);
        logo_config.horizontal_stretch = simple_strtoul(argv[3], NULL, 10);
        logo_config.vertical_stretch = simple_strtoul(argv[4], NULL, 10);
    }
    else
    {
        logo_config.disp_x = 0;
        logo_config.disp_y = 0;
        logo_config.horizontal_stretch = 1;
        logo_config.vertical_stretch = 1;
    }
    UBOOT_DEBUG("disp_x=0x%x,disp_y=0x%x,horizontal_stretch=0x%x,vertical_stretch=0x%x\n",logo_config.disp_x,logo_config.disp_y,logo_config.horizontal_stretch,logo_config.vertical_stretch);

    ret = mtk_gop_show_logo(logo_config,&logo_info,logo_output_buf,logo_output_buf_size);
    if(ret != 0)
    {
        UBOOT_ERROR("Show bootlogo failure\n");
    }

    end = get_timer(0);
    time_stamp_store_to_register(end,(sizeof(int)/sizeof(short)));
    UBOOT_BOOTTIME("[Show logo][start:%llu][end:%llu][total time:%llu]\n", start, end, end-start);
    UBOOT_TRACE("OK\n");
    return ret;
}

U_BOOT_CMD(
    showlogo, CONFIG_SYS_MAXARGS, 1,    do_showlogo,
    "Display Logo\n",
    "[state] - Display bootlogo/Display avb <state>\n"
);
