// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2017 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <common.h>
#include <command.h>
#include <dm.h>
#include <led.h>
#include <dm/uclass-internal.h>

static int show_led_state(struct udevice *dev)
{
	int ret;

	ret = led_get_state(dev);
	if (ret >= LED_CONTROL_COUNT)
		ret = -EINVAL;
	if (ret >= 0)
		printf("%d\n", ret);

	return ret;
}

static int list_leds(void)
{
	struct udevice *dev;
	int ret;

	for (uclass_find_first_device(UCLASS_LED, &dev);
	     dev;
	     uclass_find_next_device(&dev)) {
		struct led_uc_plat *plat = dev_get_uclass_platdata(dev);

		if (!plat->label)
			continue;
		printf("%-15s ", plat->label);
		if (device_active(dev)) {
			ret = show_led_state(dev);
			if (ret < 0)
				printf("Error %d\n", ret);
		} else {
			printf("<inactive>\n");
		}
	}

	return 0;
}

int doled(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	enum led_control cmd;
	const char *led_label;
	struct udevice *dev;
	int ret;

	/* Validate arguments */
	if (argc < 2)
		return CMD_RET_USAGE;
	led_label = argv[1];
	if (*led_label == 'l')
		return list_leds();

	if (argc >= 3)
	{
		cmd = simple_strtoul(argv[2], NULL, 10);

		ret = led_get_by_label(led_label, &dev);
		if (ret) {
			printf("LED '%s' not found (err=%d)\n", led_label, ret);
			return CMD_RET_FAILURE;
		}
		switch (cmd) {
			case LED_OFF_1:
			case LED_ON_1:
			case LED_BLINK_ONCE_1:
			case LED_BLINK_1:
			case LED_KEEP_BLINK_1:
			case LED_OFF_2:
			case LED_ON_2:
			case LED_BLINK_ONCE_2:
			case LED_BLINK_2:
			case LED_KEEP_BLINK_2:
				ret = led_set_state(dev, argv);
			break;
			case LED_MODE:
				ret = led_set_tv_status(dev, argv);
			break;
			case LED_CONTROL_COUNT:
				printf("LED '%s': ", led_label);
				ret = show_led_state(dev);
			break;
			default:
				printf("LED cmd not support\n");
			break;
		}
		if (ret < 0) {
			printf("LED '%s' operation failed (err=%d)\n", led_label, ret);
			return CMD_RET_FAILURE;
		}
	}
	else
		list_leds();

	return 0;
}

#define BLINK "|3 [<on_period> <off_period> <cycle> in ms]"

#define BREATH "|4 [<duty> <breath_period>]"


U_BOOT_CMD(
	led, 8, 1, doled,
	"manage LEDs",
	"Change LED state:\n"
	"LIGHT:\tled <led_label> 0 <duty> <period>\n"
	"DARK:\tled <led_label> 1\n"
	"BLINK:\tled <led_label> 4 [<duty> <period> <on_period> <off_period> <cycle>]\n"
	"SetMode:\tled <led_label> 128 [<mode>]\n"
	"led [<led_label>]\tGet LED state\n"
	"led list\t\tshow a list of LEDs"
);
