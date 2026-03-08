/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2015 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#ifndef __LED_H
#define __LED_H

/**
 * struct led_uc_plat - Platform data the uclass stores about each device
 *
 * @label:	LED label
 */
struct led_uc_plat {
	const char *label;
};

/**
 * struct led_uc_priv - Private data the uclass stores about each device
 *
 * @period_ms:	Flash period in milliseconds
 */
struct led_uc_priv {
	int period_ms;
};

//This is used to action by the sysfs
enum led_control {
	LED_ON_1 = 0x00,
	LED_OFF_1,
	LED_BREATH_1,
	LED_BLINK_ONCE_1,
	LED_BLINK_1,
	LED_KEEP_BLINK_1,
	LED_ON_PERCENTAGE_1,
	LED_BLINK_ONCE_AND_ON_1,
	LED_ON_2 = 0x20,
	LED_OFF_2,
	LED_BREATH_2,
	LED_BLINK_ONCE_2,
	LED_BLINK_2,
	LED_KEEP_BLINK_2,
	LED_ON_PERCENTAGE_2,
	LED_BLINK_ONCE_AND_ON_2,
	LED_STANDBY_SETTING_ON = 0x40,
	LED_STANDBY_SETTING_OFF,
	LED_MODE = 0x80,
	LED_CONTROL_COUNT
};

enum led_mbox_cmd {
	LED_MODE_PWMGPIO_SET = 0X40,
	LED_PWMGPIO_LIGHT_PERCENTAGE_1 = 0X80,
	LED_PWMGPIO_LIGHT_1 = 0X81,
	LED_PWMGPIO_DARK_1 = 0X82,
	LED_PWMGPIO_BREATH_1 = 0X83,
	LED_PWMGPIO_FLICKER_1 = 0X84,
	LED_PWMGPIO_FLICKER_POWERON_1 = 0x85,
	LED_PWMGPIO_LIGHT_PERCENTAGE_2 = 0XB0,
	LED_PWMGPIO_LIGHT_2 = 0XB1,
	LED_PWMGPIO_DARK_2 = 0XB2,
	LED_PWMGPIO_BREATH_2 = 0XB3,
	LED_PWMGPIO_FLICKER_2 = 0XB4,
	LED_PWMGPIO_FLICKER_POWERON_2 = 0XB5,
	LED_MODE_MAX = 0XFF
};

enum led_tv_status {
	LED_TV_STATUS_UPGRADE,
	LED_TV_STATUS_MAX
};

struct led_ops {
	/**
	 * set_state() - set the state of an LED
	 *
	 * @dev:	LED device to change
	 * @argv:	Parameter
	 * @return 0 if OK, -ve on error
	 */
	int (*set_state)(struct udevice *dev, char * const argv[]);

	/**
	 * led_get_state() - get the state of an LED
	 *
	 * @dev:	LED device to change
	 * @return LED state led_control, or -ve on error
	 */
	enum led_control (*get_state)(struct udevice *dev);

	/**
	 * led_set_tv_status() - set the LED behavior for TV status
	 *
	 * @dev:	LED device to change
	 * @argv:	Parameter
	 * @return 0 if OK, -ve on error
	 */
	int (*set_tv_status)(struct udevice *dev, char * const argv[]);
};

#define led_get_ops(dev)	((struct led_ops *)(dev)->driver->ops)

/**
 * led_get_by_label() - Find an LED device by label
 *
 * @label:	LED label to look up
 * @devp:	Returns the associated device, if found
 * @return 0 if found, -ENODEV if not found, other -ve on error
 */
int led_get_by_label(const char *label, struct udevice **devp);

/**
 * led_set_state() - set the state of an LED
 *
 * @dev:	LED device to change
 * @argv:	Parameter
 * @return 0 if OK, -ve on error
 */
int led_set_state(struct udevice *dev, char * const argv[]);

/**
 * led_get_state() - get the state of an LED
 *
 * @dev:	LED device to change
 * @return LED state led_control, or -ve on error
 */
enum led_control led_get_state(struct udevice *dev);

/**
 * led_set_tv_status() - set the LED behavior for TV status
 *
 * @dev:	LED device to change
 * @argv:	Parameter
 * @return 0 if OK, -ve on error
 */
int led_set_tv_status(struct udevice *dev, char * const argv[]);


#endif
