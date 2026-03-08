// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

#include <common.h>
#include <command.h>
#include <dm.h>
#include <time.h>
#include <debug_impl.h>
#include <sound.h>
#include <audio_codec.h>
#include <standby_mode.h>
#include <mtk-pm.h>
#include <utility.h>
#include <iniutility.h>

#ifdef CONFIG_DATA_SEPARATION
#include <mtk_dataindex.h>
#define MUSIC_SECTION        "BootLogoMusic"
#define MUSIC_INI_KEY        "m_pBootLogoMusicCfg_File"
#endif

#define BASE	10

/* Initilaise sound subsystem */
static int do_init(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	struct udevice *snd_dev;
	struct udevice *copr_dev;
	struct udevice *codec_dev;
	int ret;

	debug("[cmd] do audio init\n");
	/* do audio init */
	ret = uclass_get_device_by_name(UCLASS_SOUND, "mtk-sound-soc", &snd_dev);
	if (!ret) {
		ret = sound_setup(snd_dev);
		if (ret)
			return -ENOSYS;
	}
	else {
		debug("Initialise Audio soc driver failed (ret=%d)\n", ret);
		return CMD_RET_FAILURE;
	}
	/* do dsp init */
	ret = uclass_get_device_by_name(UCLASS_SOUND, "mtk-sound-coprocessor", &copr_dev);
	if (!ret) {
		ret = sound_setup(copr_dev);
		if (ret)
			return -ENOSYS;
	}
	if (ret) {
		debug("Initialise Audio coprocessor driver failed (ret=%d)\n", ret);
		return CMD_RET_FAILURE;
	}
#ifndef CONFIG_HAPS
	/* do amp codec init */
	ret = uclass_get_device_by_phandle(UCLASS_AUDIO_CODEC, snd_dev,
							"audio-codec",
							&codec_dev);
	if (ret) {
		debug("Read 'audio-codec' from DTS fail (%d)", ret);
		return ret;
	}
	else {
		ret = audio_codec_set_params(codec_dev, 0,0,0,0,0);
		if (ret)
			return -ENOSYS;
	}
#endif
	return ret;
}

static int do_play(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	struct udevice *snd_dev;
	struct sound_ops *sound_ops;
	int ret = 0;

	debug("[cmd] do play bootmusic\n");

	ret = uclass_get_device_by_name(UCLASS_SOUND, "mtk-sound-soc", &snd_dev);
	if (!ret) {
		sound_ops = sound_get_ops(snd_dev);
		if (!sound_ops->play)
			return -ENOSYS;
		ret = sound_ops->play(snd_dev, 0, 0);
		if (ret)
			return -ENOSYS;
	}
	if (ret) {
		debug("Sound device failed to play (err=%d)\n", ret);
		return CMD_RET_FAILURE;
	}

	return 0;
}

static cmd_tbl_t cmd_sound_sub[] = {
	U_BOOT_CMD_MKENT(init, 0, 1, do_init, "", ""),
	U_BOOT_CMD_MKENT(play, 0, 1, do_play, "", ""),
};

static int do_check_music_on(void)
{
	char *opt_music = NULL;
	int music_on = -1;

	opt_music = env_get("music");
	if (opt_music != NULL) {
		music_on = simple_strtol(opt_music, NULL, BASE);
		if(music_on != 1)
			return 0;
	}

	return 1;
}

/* process sound command */
static int do_bootmusic(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	cmd_tbl_t *c;
	int r;

	debug("[cmd] do bootmusic cmd\n");
	if (pm_check_back_ground_active() == 1) {
		debug("pm_check_back_ground_active skip boot music !!!! \n");
		return 0;
	}

	if (is_recovery_mode() == 1) {
		printf("is_recovery_mode skip boot music !!! \n");
		return 0;
	}

	if (do_check_music_on() == 0) {
                debug("music off from cmd, skip boot music !!!\n");
		return 0;
	}

	if (argc < 1)
		return CMD_RET_USAGE;

	/* Strip off leading 'sound' command argument */
	argc--;
	argv++;

	c = find_cmd_tbl(argv[0], &cmd_sound_sub[0], ARRAY_SIZE(cmd_sound_sub));

	if (c)
		r = c->cmd(cmdtp, flag, argc, argv);
	else
		return CMD_RET_USAGE;

	if (r < 0)
		UBOOT_DEBUG("do bootmusic fail, return < 0\n");

	return CMD_RET_SUCCESS;
}

U_BOOT_CMD(
    bootmusic, 4, 1, do_bootmusic,
    "mediatek sound sub-system",
    "init  - init mediatek sound driver\n"
    "bootmusic play  - play bootmusic\n"
);
