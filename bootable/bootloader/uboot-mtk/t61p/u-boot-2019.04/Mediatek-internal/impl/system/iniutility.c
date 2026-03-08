// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

#include <common.h>
#include <apiGFX.h>
#include <utility.h>
#include <iniparser.h>
#include <iniutility.h>
#include <debug_impl.h>
#include <standby_mode.h>
#if defined(CONFIG_MTK_PM)
#include <mtk-pm.h>
#endif
#include <fs.h>
#ifdef CONFIG_MULTICORES_PLATFORM
#include <smp/thread_info.h>
extern smp_spin_lock_t fs_spin_lock;
#endif

int get_bootlogo_info(const char *partition, const char *file, struct bootlogo_info *bootlogo, const char *avb_state)
{
    loff_t size;
    int ret, color_format = 0;
    unsigned char *ini_file_buf;
    const char *ini_info = NULL;
    iniparser_handle_t ini_handle = NULL;
    struct section *section_entry;

    ini_file_buf = read_storage_file_to_memory(partition,file,&size);
    if(ini_file_buf == NULL)
    {
        free(ini_file_buf);
        UBOOT_ERROR("Error: Read ini file to DRAM failure\n");
        return -1;
    }

    if(avb_state != NULL)
        ini_info = avb_state;
    else
        ini_info = "logo";

    ret = iniparser_create(ini_file_buf, size, &ini_handle);
    free(ini_file_buf);

    if (ret < 0) {
        UBOOT_ERROR("Error: parse ini [%s] failure\n", file);
        return -1;
    }

    ret = iniparser_get_section(ini_handle, ini_info, &section_entry);
    if (ret < 0) {
        iniparser_destroy(ini_handle);
        UBOOT_ERROR("Error: get ini section[%s] failure\n", ini_info);
        return -1;
    }

    ret = iniparser_getstring(section_entry, "type", "", bootlogo->type, INI_INFO_SIZE);
    if(ret < 0)
    {
        iniparser_destroy(ini_handle);
        UBOOT_ERROR("Error: Parse logo type information failure\n");
        return -1;
    }

    ret = iniparser_getstring(section_entry, "path", "", bootlogo->path, INI_INFO_SIZE);
    if(ret < 0)
    {
        iniparser_destroy(ini_handle);
        UBOOT_ERROR("Error: Parse logo file path information failure\n");
        return -1;
    }
    ret = iniparser_getint(section_entry, "width", 0, &bootlogo->width);
    if(ret < 0)
        UBOOT_DEBUG("Error: Parse logo width information failure\n");

    iniparser_getint(section_entry, "height", 0, &bootlogo->height);
    if(ret < 0)
        UBOOT_DEBUG("Error: Parse logo height information failure\n");

    iniparser_getint(section_entry, "color_format", 0, &color_format);
    if(ret < 0)
        UBOOT_DEBUG("Error: Parse logo color_format information failure\n");

    switch(color_format)
    {
        case 0:
            bootlogo->color_format = GFX_FMT_ARGB8888;
            break;
        case 1:
            bootlogo->color_format = GFX_FMT_YUV422;
            break;
        default:
            bootlogo->color_format = GFX_FMT_ABGR8888;
            break;
    }
    iniparser_destroy(ini_handle);
    return 0;
}

int get_network_info(const char *partition, const char *file, struct network_info *network)
{
    loff_t size;
    int ret;
    unsigned char *ini_file_buf;
    iniparser_handle_t ini_handle = NULL;
    struct section *section_entry;

    ini_file_buf = read_storage_file_to_memory(partition,file,&size);
    if(ini_file_buf == NULL)
    {
        free(ini_file_buf);
        UBOOT_DEBUG("Error: Read ini file to DRAM failure\n");
        return -1;
    }

    ret = iniparser_create(ini_file_buf, size, &ini_handle);
    free(ini_file_buf);
    if (ret < 0) {
        UBOOT_ERROR("Error: parse ini [%s] failure\n", file);
        return -1;
    }

    ret = iniparser_get_section(ini_handle, "ETHERNET", &section_entry);
    if (ret < 0) {
        iniparser_destroy(ini_handle);
        UBOOT_ERROR("Error: get ini section[%s] failure\n", "ETHERNET");
        return -1;
    }

    ret = iniparser_getstring(section_entry, "ethaddr", "", network->ethaddr, ETH_STRING_SIZE);
    if (ret < 0)
    {
        iniparser_destroy(ini_handle);
        UBOOT_ERROR("Error: Parse ethaddr information failure\n");
        return -1;
    }
    iniparser_destroy(ini_handle);
    return 0;
}

int get_voc_info(const char *partition, const char *file, struct voc_env_info *voc_env)
{
    loff_t size;
    int ret;
    unsigned char *ini_file_buf;
    iniparser_handle_t ini_handle = NULL;
    struct section *section_entry;

    ini_file_buf = read_storage_file_to_memory(partition,file,&size);
    if(ini_file_buf == NULL)
    {
        free(ini_file_buf);
        UBOOT_ERROR("Error: Read ini file to DRAM failure\n");
        return -1;
    }

    ret = iniparser_create(ini_file_buf, size, &ini_handle);
    free(ini_file_buf);
    if (ret < 0) {
        UBOOT_ERROR("Error: parse ini [%s] failure\n", file);
        return -1;
    }

    ret = iniparser_get_section(ini_handle, "mi_voc_env", &section_entry);
    if (ret < 0) {
        iniparser_destroy(ini_handle);
        UBOOT_ERROR("Error: get ini section[%s] failure\n", "mi_voc_env");
        return -1;
    }

    iniparser_getint(section_entry, "enable_mic", 0, &voc_env->enable_mic);
    iniparser_getint(section_entry, "enable_wakeup", 0, &voc_env->enable_wakeup);
    iniparser_getint(section_entry, "enable_seamless", 0, &voc_env->enable_seamless);
    iniparser_getint(section_entry, "enable_smartspeaker", 0, &voc_env->enable_smartspeaker);
    iniparser_destroy(ini_handle);
    return 0;
}

int load_standby_qhb_info(const char *partition, const char *file, struct standby_qhb_info *standby_info) {
    loff_t size;
    int ret;
    unsigned char *ini_file_buf;
    iniparser_handle_t ini_handle = NULL;
    struct section *section_entry;
    UBOOT_TRACE("IN\n");
    memset(standby_info,0,sizeof(struct standby_qhb_info));
    UBOOT_DEBUG(" load_standby_qhb_info partition:%s  file:%s \n",partition,file);
    ini_file_buf = read_storage_file_to_memory(partition,file,&size);
    if(ini_file_buf == NULL)
    {
        free(ini_file_buf);
        UBOOT_ERROR("Error: Read ini file to DRAM failure\n");
        return -1;
    }

    ret = iniparser_create(ini_file_buf, size, &ini_handle);
    free(ini_file_buf);
    if (ret < 0) {
        UBOOT_ERROR("Error: parse ini [%s] failure\n", file);
        return -1;
    }

    ret = iniparser_get_section(ini_handle, "standby", &section_entry);
    if (ret < 0) {
        iniparser_destroy(ini_handle);
        UBOOT_ERROR("Error: get ini section[%s] failure\n", "standby");
        return -1;
    }
    iniparser_getint(section_entry, "qhb_mode", 1, &standby_info->qhb_mode);
    iniparser_getint(section_entry, "enter_standby", 0, &standby_info->enter_standby);
    //0(Direct mode),1(Memory mode),2(Secondary mode)
    iniparser_getint(section_entry, "second_standby_mode", 0, &standby_info->second_standby_mode);
    iniparser_destroy(ini_handle);
    UBOOT_TRACE("OUT\n");
    return 0;
}

int store_standby_qhb_info(const char *partition, const char *file, struct standby_qhb_info *standby_info) {
    loff_t size;
    int ret;
    unsigned char *ini_file_buf;
    iniparser_handle_t ini_handle = NULL;
    struct section *section_entry;
    unsigned char *out_ini = NULL;
    unsigned int out_size = 0;

    char device_name[STORAGE_DEVICE_BUF_SIZE] = {0};
    unsigned int device = 0;
    char storage_info[STORAGE_INDEX_BUF_SIZE] = {0};
    static loff_t actwrite = 0;
#ifdef CONFIG_MULTICORES_PLATFORM
    unsigned long irq_flag = 0;
#endif

    UBOOT_TRACE("IN\n");
    ini_file_buf = read_storage_file_to_memory(partition, file, &size);
    if(ini_file_buf == NULL)
    {
        free(ini_file_buf);
        UBOOT_ERROR("Error: Read ini file to DRAM failure\n");
        return -1;
    }

    ret = iniparser_create(ini_file_buf, size, &ini_handle);
    if (ret < 0) {
        UBOOT_ERROR("Error: parse ini [%s] failure\n", file);
        return -1;
    }

    ret = iniparser_get_section(ini_handle, "standby", &section_entry);
    if (ret < 0) {
        free(ini_file_buf);
        iniparser_destroy(ini_handle);
        UBOOT_ERROR("Error: get ini section[%s] failure\n", "standby");
        return -1;
    }

    iniparser_setint(section_entry, "qhb_mode", standby_info->qhb_mode);
    iniparser_setint(section_entry, "enter_standby", standby_info->enter_standby);
    iniparser_setint(section_entry, "second_standby_mode", standby_info->second_standby_mode);
    UBOOT_DEBUG("set standby_info->qhb_mode:%d\n",standby_info->qhb_mode);
    UBOOT_DEBUG("set standby_info->enter_standby:%d\n",standby_info->enter_standby);
    UBOOT_DEBUG("set standby_info->second_standby_mode:%d\n",standby_info->second_standby_mode);

    ret = iniparser_create_dumpdata(ini_handle, &out_ini, &out_size);
    if (ret < 0)
    {
        free(ini_file_buf);
        iniparser_destroy(ini_handle);
        UBOOT_ERROR("iniparser_create_dumpdata failed\n");
        return -1;
    }
    //write_storage_file(partition, file, out_ini, out_size);

    ret = sys_get_boot_device(device_name, sizeof(device_name));
    if(ret < 0)
    {
        free(ini_file_buf);
        iniparser_destroy(ini_handle);
        free(out_ini);
        UBOOT_ERROR("Error: Get booting device name failure, Unknown storage device.\n");
        return -1;
    }
    else
        UBOOT_DEBUG("device_name = %s\n", device_name);

    ret = sys_get_storage_info(device,partition,storage_info);
    if(ret < 0)
    {
        free(ini_file_buf);
        iniparser_destroy(ini_handle);
        free(out_ini);
        UBOOT_ERROR("Error: sys_get_storage_info failure\n");
        return -1;
    }
    else
        UBOOT_DEBUG("device = %d, storage_info = %s\n", device, storage_info);

#if defined(CONFIG_MULTICORES_PLATFORM)
    smp_spin_lock_save(&fs_spin_lock, irq_flag);
#endif
    if (fs_set_blk_dev(device_name, storage_info, FS_TYPE_EXT))
    {
#if defined(CONFIG_MULTICORES_PLATFORM)
    smp_spin_unlock_restore(&fs_spin_lock, irq_flag);
#endif
        free(ini_file_buf);
        iniparser_destroy(ini_handle);
        free(out_ini);
        UBOOT_ERROR("Error: partition %s select failure\n",partition);
        return -1;
    }

    ret = fs_write(file, (ulong)out_ini, 0, out_size, &actwrite);
    if (ret < 0)
    {
#if defined(CONFIG_MULTICORES_PLATFORM)
        smp_spin_unlock_restore(&fs_spin_lock, irq_flag);
#endif
        free(ini_file_buf);
        iniparser_destroy(ini_handle);
        free(out_ini);
        UBOOT_ERROR("Error: Read %s file failure\n",file);
        return -1;
    }

#if defined(CONFIG_MULTICORES_PLATFORM)
    smp_spin_unlock_restore(&fs_spin_lock, irq_flag);
#endif
    free(ini_file_buf);
    iniparser_destroy(ini_handle);
    free(out_ini);

    UBOOT_TRACE("OUT\n");
    return 0;
}

int get_wdt_info(const char *partition, const char *file, struct wdt_info *wdtinfo)
{
    loff_t size;
    int ret;
    unsigned char *ini_file_buf;
    iniparser_handle_t ini_handle = NULL;
    struct section *section_entry;
    static struct wdt_info _wdt_info;
    static bool b_load = 0;

    if (b_load == 0)
    {
        ini_file_buf = read_storage_file_to_memory(partition,file,&size);
        if(ini_file_buf == NULL)
        {
            free(ini_file_buf);
            UBOOT_DEBUG("Error: Read ini file to DRAM failure\n");
            return -1;
        }
        ret = iniparser_create(ini_file_buf, size, &ini_handle);
        free(ini_file_buf);
        if (ret < 0) {
            UBOOT_ERROR("Error: parse ini [%s] failure\n", file);
            return -1;
        }
        ret = iniparser_get_section(ini_handle, "WDT", &section_entry);
        if (ret < 0) {
            iniparser_destroy(ini_handle);
            UBOOT_ERROR("Error: get ini section[%s] failure\n", "WDT");
            return -1;
        }
        ret = iniparser_getstring(section_entry, "status", "", _wdt_info.status, WDT_STRING_SIZE);
        if (ret < 0)
        {
            iniparser_destroy(ini_handle);
            UBOOT_ERROR("Error: Parse status information failure\n");
            return -1;
        }
        iniparser_destroy(ini_handle);
    }
    b_load = 1;
    memcpy(wdtinfo->status, _wdt_info.status, WDT_STRING_SIZE);
    return 0;
}

int get_panel_ssc_info(const char *partition, const char *file, struct panel_ssc_info *ssc_info)
{
	unsigned char *ini_file_buf;
    loff_t size;
    iniparser_handle_t ini_handle = NULL;
    struct section *section_entry;
    char *section_name = "MISC_SSC_CFG";
	char *key_overywriteEn = "SSC_OVERWRITE_EN";
    char *key_en = "SSC_CTRL_EN";
    char *key_modulation = "SSC_MODULATION";
	char *key_deviation = "SSC_DEVIATION";
    int ret = 0;

	ini_file_buf = read_storage_file_to_memory(partition, file,  &size);
	if(ini_file_buf == NULL)
	{
		free(ini_file_buf);
		UBOOT_ERROR("Read ini file to DRAM failure\n");
		return -1;
	}

	ret = iniparser_create(ini_file_buf, size, &ini_handle);
	free(ini_file_buf);
	if (ret < 0) {
		UBOOT_ERROR("Parser ini [%s] failure\n", file);
		return -1;
	}

	ret = iniparser_get_section(ini_handle, section_name, &section_entry);
	if (ret < 0) {
		UBOOT_ERROR("Get ini section[%s] failure\n", section_name);
		goto EXIT;
	}
	ret = iniparser_getbool(section_entry, key_overywriteEn, false, (bool *)&ssc_info->ssc_overwrite_enable);
	if (ret < 0) {
		UBOOT_ERROR("Get ini key[%s] failure\n", key_overywriteEn);
		goto EXIT;
	}
	ret = iniparser_getbool(section_entry, key_en, false, (bool *)&ssc_info->ssc_enable);
	if (ret < 0) {
		UBOOT_ERROR("Get ini key[%s] failure\n", key_en);
		goto EXIT;
	}
	ret = iniparser_getint(section_entry, key_modulation, 0, &ssc_info->ssc_modulation);
	if (ret < 0) {
		UBOOT_ERROR("Get ini key[%s] failure\n", key_modulation);
		goto EXIT;
	}
	ret = iniparser_getint(section_entry, key_deviation, 0, &ssc_info->ssc_deviation);
	if (ret < 0) {
		UBOOT_ERROR("Get ini key[%s] failure\n", key_modulation);
		goto EXIT;
	}
EXIT:
	iniparser_destroy(ini_handle);
	return ret;
}

int get_panel_mirror_info(const char *partition, const char *file, struct panel_mirror_info *info)
{
    unsigned char *ini_file_buf;
    loff_t size;
    iniparser_handle_t ini_handle = NULL;
    struct section *section_entry;
    char *section_name = "MISC_MIRROR_CFG";
    char *key_valid = "MIRROR_OSD";
    char *key_mode = "MIRROR_OSD_TYPE";
    int ret = 0;
    static struct panel_mirror_info s_st_preinfo = {0xFF, 0xFF};

    if ((s_st_preinfo.valid != 0xFF) && (s_st_preinfo.mirror_mode != 0xFF))
    {
        info->valid = s_st_preinfo.valid;
        info->mirror_mode = s_st_preinfo.mirror_mode;
        return ret;
    }

    ini_file_buf = read_storage_file_to_memory(partition, file,  &size);
    if(ini_file_buf == NULL)
    {
        free(ini_file_buf);
        UBOOT_ERROR("Read ini file to DRAM failure\n");
        return -1;
    }

    ret = iniparser_create(ini_file_buf, size, &ini_handle);
    free(ini_file_buf);
    if (ret < 0) {
        UBOOT_ERROR("Parser ini [%s] failure\n", file);
        return -1;
    }

    ret = iniparser_get_section(ini_handle, section_name, &section_entry);
    if (ret < 0) {
        UBOOT_ERROR("Get ini section[%s] failure\n", section_name);
        goto EXIT;
    }
    ret = iniparser_getbool(section_entry, key_valid, false, (bool *)&info->valid);
    if (ret < 0) {
        UBOOT_ERROR("Get ini key[%s] failure\n", key_valid);
        goto EXIT;
    }
    ret = iniparser_getint(section_entry, key_mode, 0, &info->mirror_mode);
    if (ret < 0) {
        UBOOT_ERROR("Get ini key[%s] failure\n", key_mode);
        goto EXIT;
    }
    s_st_preinfo.valid = info->valid;
    s_st_preinfo.mirror_mode = info->mirror_mode;
EXIT:
    iniparser_destroy(ini_handle);
    return ret;
}
int get_upgrade_info(const char *partition, const char *file, struct upgrade_info *upgradeinfo)
{
    loff_t size;
    int ret;
    unsigned char *ini_file_buf;
    iniparser_handle_t ini_handle = NULL;
    struct section *section_entry;
    char *section_name = "upgrade";
    unsigned int device = 0;
    char storage_info[STORAGE_INDEX_BUF_SIZE] = {0};

    ini_file_buf = read_storage_file_to_memory(partition, file,  &size);
    if(ini_file_buf == NULL)
    {
        free(ini_file_buf);
        UBOOT_DEBUG("Read ini file to DRAM failure\n");
        return -1;
    }

    ret = iniparser_create(ini_file_buf, size, &ini_handle);
    free(ini_file_buf);
    if (ret < 0) {
        UBOOT_ERROR("Parser ini [%s] failure\n", file);
        return -1;
    }
    ret = iniparser_get_section(ini_handle, section_name, &section_entry);
    if (ret < 0) {
        UBOOT_ERROR("Get ini section[%s] failure\n", section_name);
        goto EXIT;
    }

    ret = iniparser_getstring(section_entry, "upgrade_mode", "", upgradeinfo->upgrade_mode, INI_INFO_SIZE);
    if (ret < 0)
    {
        UBOOT_ERROR("Error: Parse upgrade_mode information failure\n");
        goto EXIT;
    }
    ret = iniparser_getstring(section_entry,"upgrade_partition", "", upgradeinfo->upgrade_partition, INI_INFO_SIZE);
    if (ret < 0)
    {
        UBOOT_ERROR("Error: Parse upgrade_partition information failure\n");
        goto EXIT;
    }
    ret = iniparser_getstring(section_entry,"upgrade_filename", "", upgradeinfo->upgrade_filename, UPGRADE_FILE_NAME_SIZE);
    if (ret < 0)
    {
        UBOOT_ERROR("Error: Parse upgrade_filename information failure\n");
        goto EXIT;
    }
    ret = iniparser_getstring(section_entry,"upgrade_power_mode", "", upgradeinfo->upgrade_power_mode, INI_INFO_SIZE);
    if (ret < 0)
    {
        UBOOT_ERROR("Error: Parse upgrade_power_mode information failure\n");
        goto EXIT;
    }
    ret = iniparser_getstring(section_entry,"upgrade_complete", "", upgradeinfo->upgrade_complete, INI_INFO_SIZE);
    if (ret < 0)
    {
        UBOOT_ERROR("Error: Parse upgrade_complete information failure\n");
        goto EXIT;
    }
    ret = iniparser_getstring(section_entry,"auto_usb_upgrade", "", upgradeinfo->auto_usb_upgrade, INI_INFO_SIZE);
    if (ret < 0)
    {
        UBOOT_ERROR("Error: Parse auto_usb_upgrade information failure\n");
        goto EXIT;
    }
    UBOOT_DEBUG("upgrade_mode =%s\n",upgradeinfo->upgrade_mode);
    UBOOT_DEBUG("upgrade_partition =%s\n",upgradeinfo->upgrade_partition);
    UBOOT_DEBUG("upgrade_filename =%s\n",upgradeinfo->upgrade_filename);
    UBOOT_DEBUG("upgrade_power_mode =%s\n",upgradeinfo->upgrade_power_mode);
    UBOOT_DEBUG("upgrade_complete =%s\n",upgradeinfo->upgrade_complete);
    UBOOT_DEBUG("auto_usb_upgrade =%s\n",upgradeinfo->auto_usb_upgrade);
    if (strlen(upgradeinfo->upgrade_partition)>0)
    {
        memset(storage_info,0,STORAGE_INDEX_BUF_SIZE);
        ret = sys_get_storage_info(device, upgradeinfo->upgrade_partition, storage_info);
        if (ret < 0)
        {
            UBOOT_ERROR("Error: get upgrade_partition [%s] failure\n",upgradeinfo->upgrade_partition);
            goto EXIT;
        }
        memcpy(upgradeinfo->upgrade_storage_info,storage_info,STORAGE_INDEX_BUF_SIZE);
    }
    EXIT:
    iniparser_destroy(ini_handle);
    return ret;
}

int get_bootmusic_info(const char *partition, const char *file, struct bootmusic_info *bootmusic)
{
    loff_t size;
    int ret;
    unsigned char *ini_file_buf;
    iniparser_handle_t ini_handle = NULL;
    struct section *section_entry;

    ini_file_buf = read_storage_file_to_memory(partition,file,&size);
    if(ini_file_buf == NULL)
    {
        free(ini_file_buf);
        UBOOT_ERROR("Error: Read ini file to DRAM failure\n");
        return -1;
    }

    ret = iniparser_create(ini_file_buf, size, &ini_handle);
    free(ini_file_buf);
    if (ret < 0) {
        UBOOT_ERROR("Error: parse ini [%s] failure\n", file);
        return -1;
    }

    ret = iniparser_get_section(ini_handle, "MUSIC_CFG", &section_entry);
    if (ret < 0) {
        iniparser_destroy(ini_handle);
        UBOOT_ERROR("Error: get ini section[%s] failure\n", "MUSIC_CFG");
        return -1;
    }

    iniparser_getbool(section_entry, "MUSIC_ON", 0, &bootmusic->enable);
    iniparser_getint(section_entry, "MUSIC_VOL", 0, &bootmusic->volume);
    iniparser_getstring(section_entry, "MUSIC_NAME", "", bootmusic->filename, INI_INFO_SIZE);

    iniparser_destroy(ini_handle);

    return 0;
}

int set_upgrade_complete(const char *partition, const char *file, struct upgrade_info *upgradeinfo, int complete_flag, unsigned char **rawdata_out, unsigned int *size_out)
{
    loff_t size;
    int ret;
    unsigned char *ini_file_buf;
    iniparser_handle_t ini_handle = NULL;
    struct section *section_entry;
    char *section_name = "upgrade";
    unsigned int out_size=0;
    UBOOT_TRACE("IN\n");
    UBOOT_DEBUG(" set_upgrade_complete partition:%s  file:%s \n",partition,file);
    ini_file_buf = read_storage_file_to_memory(partition, file,  &size);
    if(ini_file_buf == NULL)
    {
        free(ini_file_buf);
        UBOOT_DEBUG("Read ini file to DRAM failure\n");
        return -1;
    }

    ret = iniparser_create(ini_file_buf, size, &ini_handle);
    free(ini_file_buf);
    if (ret < 0) {
        UBOOT_ERROR("Parser ini [%s] failure\n", file);
        return -1;
    }
    ret = iniparser_get_section(ini_handle, section_name, &section_entry);
    if (ret < 0) {
        UBOOT_ERROR("Get ini section[%s] failure\n", section_name);
        goto EXIT;
    }
    UBOOT_DEBUG("gupgradeinfo->upgrade_mode=%s\n", upgradeinfo->upgrade_mode);

    ret = iniparser_setstring(section_entry, "upgrade_complete", upgradeinfo->upgrade_mode);
    if (ret < 0)
    {
        UBOOT_ERROR("Error: Write upgrade_complete information failure\n");
        goto EXIT;
    }
    UBOOT_DEBUG(" Clean upgrade_mode ...\n");
    ret = iniparser_setstring(section_entry, "upgrade_mode", "");
    if (ret < 0)
    {
        UBOOT_ERROR("Error: Write upgrade_mode information failure\n");
        goto EXIT;
    }

    ret = iniparser_create_dumpdata(ini_handle, &(*rawdata_out), &out_size);
    *size_out = out_size;

    if(write_storage_file(partition, file, *rawdata_out, &size)==NULL)
        UBOOT_ERROR("Error: Ext4 Write ini file failure !!!\n");

    if(ret==NULL)
    {
        UBOOT_ERROR("Error: Ext4 Write upgrade_mode information failure\n");
    }
    EXIT:
    iniparser_destroy(ini_handle);
    UBOOT_TRACE("OUT\n");
    return ret;
}

int get_console_info(const char *partition, const char *file, struct console_info *consoleinfo)
{
    loff_t size;
    int ret;
    unsigned char *ini_file_buf;
    iniparser_handle_t ini_handle = NULL;
    struct section *section_entry;
    static struct console_info _consoleinfo;
    static bool b_load = 0;

    if (b_load == 0)
    {
        ini_file_buf = read_storage_file_to_memory(partition, file, &size);
        if (ini_file_buf == NULL) {
            free(ini_file_buf);
            UBOOT_ERROR("Error: Read ini file for console_info failure\n");
            return -1;
        }

        ret = iniparser_create(ini_file_buf, size, &ini_handle);
        free(ini_file_buf);
        if (ret < 0) {
            UBOOT_ERROR("Error: parse ini [%s] failure\n", file);
            return -1;
        }

        ret = iniparser_get_section(ini_handle, "CONSOLE", &section_entry);
        if (ret <= 0) {
            iniparser_destroy(ini_handle);
            UBOOT_ERROR("Error: get ini section[%s] failure\n", "CONSOLE");
            return -1;
        }

        ret = iniparser_getstring(section_entry, "console_hdmi", "off", _consoleinfo.console_hdmi, CONSOLE_INFO_SIZE);
        if (ret < 0)
            UBOOT_ERROR("Error: Parse console_hdmi information failure\n");

        ret = iniparser_getstring(section_entry, "console_onoff", "on", _consoleinfo.console_onoff, CONSOLE_INFO_SIZE);
        if (ret < 0)
            UBOOT_ERROR("Error: Parse console_onoff information failure\n");

        iniparser_destroy(ini_handle);
    }

    b_load = 1;
    memcpy(consoleinfo->console_hdmi, _consoleinfo.console_hdmi, CONSOLE_INFO_SIZE);
    memcpy(consoleinfo->console_onoff, _consoleinfo.console_onoff, CONSOLE_INFO_SIZE);

    return 0;
}

int get_panel_dlg_info(const char *partition, const char *file, struct panel_dlg_info *info)
{
#if 0
    unsigned char *ini_file_buf;
    loff_t size;
    iniparser_handle_t ini_handle = NULL;
    struct section *section_entry;
    char *section_name = "panel";
#endif
    char *key_mode = "PanelModeNum";
    char *key_mode_gop = "mdrv-graphic.dlgmode";
    const char dlg_boot0[] = "PanelModeNum=0";
    const char dlg_boot1[] = "PanelModeNum=1";
    const char dlg_boot_gop0[] = "mdrv-graphic.dlgmode=0";
    const char dlg_boot_gop1[] = "mdrv-graphic.dlgmode=1";
    int ret = 0;
    static int s_iPrePanel_dlg = 0xFF;

    /* To Save the DLG mode to speed up boot time  */
    if (s_iPrePanel_dlg != 0xFF)
    {
        info->panel_dlg = s_iPrePanel_dlg;
        return ret;
    }

#if 0
    ini_file_buf = read_storage_file_to_memory(partition, file,  &size);
    if(ini_file_buf == NULL)
    {
        free(ini_file_buf);
        UBOOT_ERROR("Read ini file to DRAM failure\n");
        return -1;
    }

    ret = iniparser_create(ini_file_buf, size, &ini_handle);
    free(ini_file_buf);
    if (ret < 0) {
        UBOOT_ERROR("Parser ini [%s] failure\n", file);
        return -1;
    }

    ret = iniparser_get_section(ini_handle, section_name, &section_entry);
    if (ret < 0) {
        UBOOT_ERROR("Get ini section[%s] failure\n", section_name);
        goto EXIT;
    }

    ret = iniparser_getint(section_entry, key_mode, 0, &info->panel_dlg);
    if (ret < 0) {
        UBOOT_ERROR("Get ini key[%s] failure\n", key_mode);
        goto EXIT;
    }
EXIT:
    iniparser_destroy(ini_handle);
#endif
    info->panel_dlg = 0;
    if (ret < 0)
    {
        add_bootargs(key_mode,(char *)dlg_boot0,1);
        add_bootargs(key_mode_gop,(char *)dlg_boot_gop0,1);
        UBOOT_INFO("add_bootargs %s\n", dlg_boot0);
    }
    else
    {
        if(info->panel_dlg)
        {
            add_bootargs(key_mode,(char *)dlg_boot1,1);
            add_bootargs(key_mode_gop,(char *)dlg_boot_gop1,1);
            UBOOT_INFO("add_bootargs %s\n", dlg_boot1);
        }
        else
        {
            add_bootargs(key_mode,(char *)dlg_boot0,1);
            add_bootargs(key_mode_gop,(char *)dlg_boot_gop0,1);
            UBOOT_INFO("add_bootargs %s\n", dlg_boot0);
        }
    }
    s_iPrePanel_dlg = info->panel_dlg;
    return ret;
}

int get_panel_pwm_duty_info(const char *partition, const char *file, struct panel_pwm_duty_info *info)
{
    unsigned char *ini_file_buf;
    loff_t size;
    iniparser_handle_t ini_handle = NULL;
    struct section *section_entry;
    char *section_name = "PWM";
    char *key_mode0 = "pwm_duty";
    char *key_mode1 = "dual_pwm_duty";
    int ret = 0;

    info->pwm_duty = PWM_DUTY_INVALID;
    info->dual_pwm_duty = PWM_DUTY_INVALID;

    ini_file_buf = read_storage_file_to_memory(partition, file,  &size);
    if(ini_file_buf == NULL)
    {
        free(ini_file_buf);
        UBOOT_ERROR("Read ini file to DRAM failure\n");
        return -1;
    }

    ret = iniparser_create(ini_file_buf, size, &ini_handle);
    free(ini_file_buf);
    if (ret < 0) {
        UBOOT_ERROR("Parser ini [%s] failure\n", file);
        return -1;
    }

    ret = iniparser_get_section(ini_handle, section_name, &section_entry);
    if (ret < 0) {
        UBOOT_ERROR("Get ini section[%s] failure\n", section_name);
        goto EXIT;
    }

    ret = iniparser_getint(section_entry, key_mode0, PWM_DUTY_INVALID, &info->pwm_duty);
    if (ret < 0) {
        UBOOT_DEBUG("Get ini key[%s] failure\n", key_mode0);
        //goto EXIT;
    }
    UBOOT_DEBUG("Get ini key[%s] is 0x%x\n", key_mode0, info->pwm_duty);

    ret = iniparser_getint(section_entry, key_mode1, PWM_DUTY_INVALID, &info->dual_pwm_duty);
    if (ret < 0) {
        UBOOT_DEBUG("Get ini key[%s] failure\n", key_mode1);
        //goto EXIT;
    }
    UBOOT_DEBUG("Get ini key[%s] is 0x%x\n", key_mode1, info->dual_pwm_duty);

EXIT:
    iniparser_destroy(ini_handle);

    return ret;
}
