/*
 * Copyright (c) 2023 Amazon.com, Inc. or its affiliates.  All rights reserved.
 */
#include <common.h>
#include <command.h>
#include <amzn_tv_secure_boot.h>
#include <mtk_ab.h>
#include <idme.h>
#include <environment.h>
#include <iniutility.h>
#include <iniparser.h>
#include <utility.h>
#include <mtk_dataindex.h>
#include <debug_impl.h>

#if defined(CONFIG_DIAG_TRANSITION_DIALOG)

#define CMD_BUF                 128
#define GWIN_WIDTH              720
#define GWIN_HEIGHT             576
#define GRAPHIC_X               32
#define GRAPHIC_Y               32
#define BG_GREEN                "0xFF00FF00"
#define BG_RED                  "0xFFFF0000"

int amzn_diag_fos_transition = 0;

#define INI_INFO_SIZE   64
struct transition_info {
    char background_color[INI_INFO_SIZE];
    char txt_info[INI_INFO_SIZE];
    int graphic_x;
    int graphic_y;
};

#define TRANSITION_CONFIG_INFO_PATH "/vendor/tvconfig/bsp/common/misc/transition_config.ini"

int get_transition_info(struct transition_info *transition_info)
{
    loff_t size;
    int ret;
    unsigned char *ini_file_buf;
    const char *ini_info = NULL;
    iniparser_handle_t ini_handle = NULL;
    struct section *section_entry;
    char partition[INI_INFO_SIZE] = {0};
    const char *relpath = NULL;

    ret = dataindex_resolve_path(partition, PART_NAME_SIZE, &relpath, TRANSITION_CONFIG_INFO_PATH);
    if (ret) {
        UBOOT_ERROR("resolve path=%s fail!\n", relpath);
        return ret;
    }

    ini_file_buf = read_storage_file_to_memory(partition, (char *)relpath, &size);
    if(ini_file_buf == NULL)
    {
        free(ini_file_buf);
        UBOOT_ERROR("Error: Read ini file to DRAM failure\n");
        return -1;
    }

    ret = iniparser_create(ini_file_buf, size, &ini_handle);
    free(ini_file_buf);

    if (ret < 0) {
        UBOOT_ERROR("Error: parse ini [%s] failure\n", relpath);
        return -1;
    }

    ret = iniparser_get_section(ini_handle, "transition_config_info", &section_entry);
    if (ret < 0) {
        iniparser_destroy(ini_handle);
        UBOOT_ERROR("Error: get ini section[%s] failure\n", ini_info);
        return -1;
    }

    if(target_is_production()) {
        ret = iniparser_getstring(section_entry, "lock_background_color", "", transition_info->background_color, INI_INFO_SIZE);
        if(ret < 0)
        {
            iniparser_destroy(ini_handle);
            UBOOT_ERROR("Error: Parse lock_background_color information failure\n");
            return -1;
        }

        ret = iniparser_getstring(section_entry, "lock_txt_info", "", transition_info->txt_info, INI_INFO_SIZE);
        if(ret < 0)
        {
            iniparser_destroy(ini_handle);
            UBOOT_ERROR("Error: Parse lock_txt_info information failure\n");
            return -1;
        }
    } else {
        ret = iniparser_getstring(section_entry, "unlock_background_color", "", transition_info->background_color, INI_INFO_SIZE);
        if(ret < 0)
        {
            iniparser_destroy(ini_handle);
            UBOOT_ERROR("Error: Parse unlock_background_color information failure\n");
            return -1;
        }

        ret = iniparser_getstring(section_entry, "unlock_txt_info", "", transition_info->txt_info, INI_INFO_SIZE);
        if(ret < 0)
        {
            iniparser_destroy(ini_handle);
            UBOOT_ERROR("Error: Parse unlock_txt_info information failure\n");
            return -1;
        }
    }

    iniparser_getint(section_entry, "graphic_x", 0, &transition_info->graphic_x);
    if(ret < 0)
        UBOOT_DEBUG("Error: Parse graphic_x information failure\n");

    iniparser_getint(section_entry, "graphic_y", 0, &transition_info->graphic_y);
    if(ret < 0)
        UBOOT_DEBUG("Error: Parse graphic_y information failure\n");

    iniparser_destroy(ini_handle);
    return 0;
}

#ifdef CONFIG_TRANS_SCREEN_CUSTOMIZED
#include <amzn_tv_common.h>
#define BOARDID_LEN 16
enum board_revision {
	PROTO = 48,
	HVT,
	EVT,
	DVT,
	PVT,
	INVALID
};
int get_board_revision(void)
{
	char board_id[BOARDID_LEN + 1] = "\0";
	const int ret = idme_get_var_external("board_id", board_id, BOARDID_LEN);
	if (ret || strlen(board_id) != BOARDID_LEN)
		return INVALID;
	return board_id[7];
}
void trans_screen_factory_customized(char **bg_color, char **disp_txt)
{
	#define FACTORY_NAME_VAR_SIZE 16
	char factory_name[FACTORY_NAME_VAR_SIZE] = "\0";
	idme_get_oem_data_field("fac=", factory_name, FACTORY_NAME_VAR_SIZE);
	if (strcmp(factory_name, "")) {
		if (!strcmp(factory_name, "hisense")) {
			if (get_board_revision() != PVT)
			{
				*bg_color = BG_GREEN;
				*disp_txt = "OK";
			}
		}
	}
}

#endif

/* The graphics code only works if called later in the init
 * sequence, not in init_idme. */

int initr_diag_fos_trans_screen(void)
{
	char buffer[CMD_BUF]="\0";
	char *bg_color;
	char *disp_txt;
	struct transition_info diag_trasinfo = {};

	if (amzn_diag_fos_transition == 0) return 0;

	snprintf(buffer, CMD_BUF, "  osd_create %d %d", GWIN_WIDTH, GWIN_HEIGHT);
	printf("%s\n", buffer);
	run_command(buffer, 0);

	if(target_is_production()) {
		bg_color = BG_GREEN;
		disp_txt = "OK";
		printf("  device is [LOCKED]\n");
	} else {
		bg_color = BG_RED;
		disp_txt = "UL";
		printf("  device is [UNLOCKED]\n");
	}
#ifdef CONFIG_TRANS_SCREEN_CUSTOMIZED
	trans_screen_factory_customized(&bg_color, &disp_txt);
#endif

	//default value
	strncpy(diag_trasinfo.background_color, bg_color, INI_INFO_SIZE);
	strncpy(diag_trasinfo.txt_info, disp_txt, INI_INFO_SIZE);
	diag_trasinfo.graphic_x = GRAPHIC_X;
	diag_trasinfo.graphic_y = GRAPHIC_Y;

	UBOOT_DEBUG("Oringinal value: color=%s, txt=%s, x=%d,y=%d\n",
                diag_trasinfo.background_color, diag_trasinfo.txt_info,
                diag_trasinfo.graphic_x, diag_trasinfo.graphic_y);

	//if other ODM need modify the default value, 
	//please modify /vendor/tvconfig/bsp/common/misc/transition_config.ini the file from tvconfig partition.
	get_transition_info(&diag_trasinfo);
	UBOOT_DEBUG("Current value: color=%s, txt=%s, x=%d,y=%d\n",
                diag_trasinfo.background_color, diag_trasinfo.txt_info,
                diag_trasinfo.graphic_x, diag_trasinfo.graphic_y);

	memset(buffer, 0 , CMD_BUF);
	snprintf(buffer, CMD_BUF, "  draw_rect %d %d %d %d %s", 0, 0, GWIN_WIDTH, GWIN_HEIGHT, diag_trasinfo.background_color);
	printf("%s\n", buffer);
	run_command(buffer, 0);

	memset(buffer, 0 , CMD_BUF);
	snprintf(buffer, CMD_BUF, "  draw_string %d %d 0xFFFFFFFF 0 %s", diag_trasinfo.graphic_x, diag_trasinfo.graphic_y, diag_trasinfo.txt_info);
	printf("%s\n", buffer);
	run_command(buffer, 0);

	printf("  osd_flush");
	run_command("osd_flush", 0);

	printf("\n\nBootmode transition completed. You may power off now.\n\n");
	while (1)
		mdelay(3000);
        return 0;
}

int do_amzn_transition_show(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
    int ret = 0;

    if (argc < 2) {
        printf("Invalid arguments\n");
        return -1;
    }

    if (!strncmp(argv[1], "show", 4)) {
        amzn_diag_fos_transition = 1;
        //firstly panel backlight on
        run_command("panel_init", 0);
        //second show picture info
        ret = initr_diag_fos_trans_screen();
    }

    return ret;
}

U_BOOT_CMD(
    amzn_transition_picture, 2, 0, do_amzn_transition_show,
    "Amazon Diag transition picture show",
    "command: amzn_transition_picture show --- show diag transition picture\n"
);
#endif

int amzn_boot(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	char cmd[CMD_BUF]="\0";
	if (argc < 2) {
		printf("Invalid arguments\n");
		return -1;
	}

	if (!strncmp(argv[1], "transition", 9) || !strncmp(argv[1], "reset",5)) {
		if (!strncmp(argv[1],"transition",9)) {

#if defined(CONFIG_ANDROID_AB)
			run_command("mmc erase.p dkernel", 0);
			run_command("mmc erase.p diag_super_a", 0);
			run_command("mmc erase.p diag_super_b", 0);
			run_command("mmc erase.p diag_userdata", 0);

			run_command("gpt remove mmc 0 diag_userdata", 0);
			run_command("gpt remove mmc 0 diag_super_b", 0);
			run_command("gpt remove mmc 0 diag_super_a", 0);
			run_command("gpt remove mmc 0 dkernel", 0);

#else
			run_command("mmc erase.p dkernel", 0);
			run_command("mmc erase.p diag_super", 0);
			run_command("mmc erase.p diag_userdata", 0);

			run_command("gpt remove mmc 0 diag_userdata", 0);
			run_command("gpt remove mmc 0 diag_super", 0);
			run_command("gpt remove mmc 0 dkernel", 0);
#endif
			run_command("gpt remove mmc 0 userdata", 0);
			run_command("gpt create_adaptive mmc 0 userdata 0x1000", 0);
		}

		run_command("idme bootmode 1", 0);
		snprintf(cmd, sizeof(cmd)-1, "env_cus_load");
		run_command(cmd,0);

		/* add new env after env_cus_load, only add in end of bootmode 7 */
		env_set("skip_wipe_userdata_once", "yes");
		env_save();

#if defined(CONFIG_DIAG_TRANSITION_DIALOG)
		amzn_diag_fos_transition = 1;
#else
		snprintf(cmd, sizeof(cmd)-1, "reset");
		run_command(cmd,0);
#endif
		return -1;
	}
	return 0;
}

U_BOOT_CMD(
	amzn_boot, 2, 0, amzn_boot,
	"Amazon customized-boot",
	"amzn_boot transition | reset - delete diag partitions\n"
);
