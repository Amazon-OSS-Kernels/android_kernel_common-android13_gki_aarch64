/* SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause) */
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

#ifndef _INIUTILITY_H
#define _INIUTILITY_H
#define INI_INFO_SIZE   64
#define UPGRADE_FILE_NAME_SIZE   64
#define ETH_STRING_SIZE 18
#define WDT_STRING_SIZE 32
#define CONSOLE_INFO_SIZE  5
#define PWM_DUTY_INVALID 0xFFFFFFFF

struct bootlogo_info {
    char type[INI_INFO_SIZE];
    char path[INI_INFO_SIZE];
    int width;
    int height;
    int color_format;
};

struct network_info {
    char ethaddr[ETH_STRING_SIZE];
};

struct voc_env_info {
    int enable_mic;
    int enable_wakeup;
    int enable_seamless;
    int enable_smartspeaker;
};

struct standby_qhb_info {
    int qhb_mode;
    int enter_standby;
    int second_standby_mode;//0(Direct mode),1(Memory mode),2(Secondary mode)
};

struct wdt_info {
    char status[WDT_STRING_SIZE];
};

struct panel_ssc_info {
	int ssc_overwrite_enable;
	int ssc_enable;
	int ssc_modulation;
	int ssc_deviation;
};

struct panel_mirror_info {
    int valid;
    int mirror_mode;
};
struct upgrade_info {
    char upgrade_mode[INI_INFO_SIZE];
    char upgrade_filename[UPGRADE_FILE_NAME_SIZE];
    char upgrade_partition[INI_INFO_SIZE];
    char upgrade_storage_info[INI_INFO_SIZE];
    char upgrade_power_mode[INI_INFO_SIZE];
    char upgrade_complete[INI_INFO_SIZE];
    char auto_usb_upgrade[INI_INFO_SIZE];
};

struct bootmusic_info {
    bool enable;
    int volume;
    char filename[INI_INFO_SIZE];
    char part[INI_INFO_SIZE];
};

struct console_info {
    char console_hdmi[CONSOLE_INFO_SIZE];
    char console_onoff[CONSOLE_INFO_SIZE];
};

struct panel_dlg_info {
    int panel_dlg;
};

struct panel_pwm_duty_info {
    int pwm_duty;
    int dual_pwm_duty;
};

int get_bootlogo_info(const char *partition, const char *file, struct bootlogo_info *bootlogo, const char *avb_state);
int get_network_info(const char *partition, const char *file, struct network_info *network);
int get_voc_info(const char *partition, const char *file, struct voc_env_info *voc_env);
int load_standby_qhb_info(const char *partition, const char *file, struct standby_qhb_info *standby_info);
int store_standby_qhb_info(const char *partition, const char *file, struct standby_qhb_info *standby_info);
int get_wdt_info(const char *partition, const char *file, struct wdt_info *wdtinfo);
int get_panel_mirror_info(const char *partition, const char *file, struct panel_mirror_info *info);
int get_upgrade_info(const char *partition, const char *file, struct upgrade_info *upgradeinfo);
int get_bootmusic_info(const char *partition, const char *file, struct bootmusic_info *bootmusic);
int set_upgrade_complete(const char *partition, const char *file, struct upgrade_info *upgradeinfo, int complete_flag, unsigned char **rawdata_out, unsigned int *size_out);
int get_panel_ssc_info(const char *partition, const char *file, struct panel_ssc_info *ssc_info);
int get_console_info(const char *partition, const char *file, struct console_info *consoleinfo);
int get_panel_dlg_info(const char *partition, const char *file, struct panel_dlg_info *info);
int get_panel_pwm_duty_info(const char *partition, const char *file, struct panel_pwm_duty_info *info);

#endif // #ifndef _INIUTILITY_H