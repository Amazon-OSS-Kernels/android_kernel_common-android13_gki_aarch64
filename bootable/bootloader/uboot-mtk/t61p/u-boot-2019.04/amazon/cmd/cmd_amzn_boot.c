/*
 * Copyright (c) 2023 Amazon.com, Inc. or its affiliates.  All rights reserved.
 */
#include <common.h>
#include <command.h>
#include <idme.h>
#include <amzn_tv_secure_boot.h>
#include <mtk_ab.h>
#include <environment.h>

#if defined(CONFIG_DIAG_TRANSITION_DIALOG)

#define CMD_BUF                 128
#define GWIN_WIDTH              720
#define GWIN_HEIGHT             576
#define GRAPHIC_X               30
#define GRAPHIC_Y               30
#define BG_GREEN                "0x80008000"
#define BG_RED                  "0x80800000"

int amzn_diag_fos_transition = 0;

/* The graphics code only works if called later in the init
 * sequence, not in init_idme. */

int initr_diag_fos_trans_screen(void)
{
	char buffer[CMD_BUF]="\0";
	char *bg_color;
	char *disp_txt;

        if (amzn_diag_fos_transition == 0) return 0;

	snprintf(buffer, CMD_BUF, "  osd_create %d %d", GWIN_WIDTH, GWIN_HEIGHT);
	printf("%s\n", buffer);
	run_command(buffer, 0);

	if(target_is_production()) {
		bg_color = BG_GREEN;
		disp_txt = "LOCKED";
		printf("  device is [LOCKED]\n");
	} else {
		bg_color = BG_RED;
		disp_txt = "UNLOCKED";
		printf("  device is [UNLOCKED]\n");
	}

	memset(buffer, 0 , CMD_BUF);
	snprintf(buffer, CMD_BUF, "  draw_rect %d %d %d %d %s", 0, 0, GWIN_WIDTH, GWIN_HEIGHT, bg_color);
	printf("%s\n", buffer);
	run_command(buffer, 0);

	memset(buffer, 0 , CMD_BUF);
	snprintf(buffer, CMD_BUF, "  draw_string %d %d 0x300000000 0 %s", 290, 250, disp_txt);
	printf("%s\n", buffer);
	run_command(buffer, 0);

	printf("  osd_flush");
	run_command("osd_flush", 0);

	printf("\n\nBootmode transition completed. You may power off now.\n\n");
	while (1)
		mdelay(3000);
        return 0;
}
#endif

int amzn_boot(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	char cmd[CMD_BUF]="\0";
	if (argc < 2) {
		printf("Invalid arguments\n");
		return -1;
	}

	if (!strncmp(argv[1],"transition",9) || !strncmp(argv[1], "reset",5)) {
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
