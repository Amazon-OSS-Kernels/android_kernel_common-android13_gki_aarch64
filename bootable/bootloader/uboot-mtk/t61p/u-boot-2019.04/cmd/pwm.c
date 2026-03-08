// SPDX-License-Identifier: GPL-2.0+
/*
 * MediaTek Inc. (C) 2020. All rights reserved.
 */

#include <common.h>
#include <command.h>
#include <errno.h>
#include <dm.h>
#include <pwm.h>

static int do_pwm_setup(cmd_tbl_t *cmdtp, int flag,
			int argc, char *const argv[])
{
	struct udevice *pwm;
	unsigned long chip, channel, period, duty, invert;
	int ret;

	if (argc < 5)
		return cmd_usage(cmdtp);

	chip = simple_strtoul(argv[1], NULL, 0);

	ret = uclass_get_device(UCLASS_PWM, chip, &pwm);
	if (ret) {
		printf("Cannot get PWM chip %ld handle : ret=%d\n", chip, ret);
		return ret;
	}

	if (!pwm) {
		printf("PWM chip %ld is NULL\n", chip);
		return -1;
	}

	printf("PWM handle %p, name %s\n", pwm, pwm->name);

	channel = simple_strtoul(argv[2], NULL, 0);
	period = simple_strtoul(argv[3], NULL, 10);
	duty = simple_strtoul(argv[4], NULL, 10);

	invert = 0;
	if (argc >= 6) {
		if (strncasecmp(argv[5], "normal", strlen("normal"))) {
			if (strncasecmp
			    (argv[5], "inversed", strlen("inversed"))) {
				return cmd_usage(cmdtp);
			}
			invert = 1;
		}
	}

	printf("PWM chip %ld channel %ld period %ld, duty %ld, invert %ld\n",
	       chip, channel, period, duty, invert);

	if ((period == 0) || (duty == 0)) {
		ret = pwm_set_enable(pwm, channel, false);
	} else {
		ret = pwm_set_config(pwm, channel, period, duty);
		if (ret == 0) {
			ret = pwm_set_invert(pwm, channel, invert);
			if (ret == 0)
				ret = pwm_set_enable(pwm, channel, true);
		}
	}

	return ret;
}

U_BOOT_CMD(pwm, 6, 1, do_pwm_setup,
	   "enable/disable pwm waveform output",
	   "<chip> <channel> <period> <duty> [polarity]\n"
	   "    chip : pwm chip index for setup, may check which chip is valid by dm command\n"
	   "    channel : channel for setup\n"
	   "    period : period of pwm (unit is nanoseconds, set 0 to disable pwm)\n"
	   "    duty : duty cycle of pwm (unit is nanoseconds, set 0 to disable pwm)\n"
	   "    polarity : specify pwm polarity (\"normal\" or \"inversed\")\n"
	   "               (optional, default normal)\n"
);
