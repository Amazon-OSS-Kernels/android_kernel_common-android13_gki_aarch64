// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2015 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <led.h>
#include <asm/gpio.h>
#include <dm/lists.h>
#include <mailbox.h>

#define PM51_MAILBOX_SIZE (14)
#define KEY_LED_USE_PWM		"led_use_pwm"
#define KEY_PWM_CHANNEL		"pwm_channel"
#define KEY_PWM_INVERSE		"pwm_inverse"
#define KEY_LED_STATUS		"led_status"

// Decimal number
#define DEFAULT_PERCENTAGE_VALUE		(30)
#define DEFAULT_BLINK_ON_PERIOD			(30)
#define DEFAULT_BLINK_OFF_PERIOD		(30)
#define DEFAULT_BLINK_CYCLE				(0)
#define DEFAULT_BLINK_OFFTIME_PERIOD	(3000)
#define DEFAULT_BLINK_NUM_PER_CYCLE		(3)

#define DEFAULT_LED_DUTY			(0x200)
#define DEFAULT_LED_PERIOD			(0x200)

// HEX number
#define DEFAULT_BREATH_FREQUENCY	(0x80)
#define DEFAULT_BREATH_PERIOD		(0x10)

struct gpio_led_data {
	struct gpio_desc gpiod;
	u8 pwm_channel;
	u8 pwm_inverse;
};

struct led_breath_priv {
	struct mbox_chan mbox;
	enum led_control state;
	int num_leds;
	bool led_is_pwm;
	struct gpio_led_data leds[];
};

struct led_breath_priv *priv = NULL;
bool is_probe = false;

int ustrtoul_safe(char * const* token, int defaultValue, int index, int argc)
{
	int ret = -1;

	//If user doesn`t input parameter, assign default value
	if (index >= argc)
	{
	    return defaultValue;
	}

	if (token[index] != NULL)
	{
		ret = simple_strtoul(token[index], NULL, 0);
	}
	if (ret < 0)
	{
		return defaultValue;
	}
	return ret;
}

static int breath_led_set_state(struct udevice *dev, char * const argv[])
{
	int ret = 0, state = 0, data = 0, argc = 0;
	u8 send_box[PM51_MAILBOX_SIZE] = {0};
	const u8 led_index = LED_ON_2 - LED_ON_1;

	if(!is_probe)
	{
		pr_err("Not probe yet\n");
		return 0;
	}

	for(argc=0; argc<8; argc++)
	{
		if(argv[argc] == NULL)
			break;
	}

	state = ustrtoul_safe(argv, 0XFE, 2, argc);
	if (strstr(argv[1], "breath-led-1"))
	{
		state += led_index;
	}
	pr_err("%s state=%d\n", __func__, state);

	switch (state)
	{
		case LED_ON_1:
			send_box[0] = LED_PWMGPIO_LIGHT_1;
			data = ustrtoul_safe(argv, DEFAULT_LED_DUTY, 3, argc);
			send_box[1] = data >> 8;
			send_box[2] = data & 0XFF;
			data = ustrtoul_safe(argv, DEFAULT_LED_PERIOD, 4, argc);
			send_box[3] = data >> 8;
			send_box[4] = data & 0XFF;
			break;
		case LED_OFF_1:
			send_box[0] = LED_PWMGPIO_DARK_1;
			break;
		case LED_BLINK_ONCE_1:
			send_box[0] = LED_PWMGPIO_FLICKER_1;
			data = ustrtoul_safe(argv, DEFAULT_LED_DUTY, 3, argc);
			send_box[1] = data >> 8;
			send_box[2] = data & 0XFF;
			data = ustrtoul_safe(argv, DEFAULT_LED_PERIOD, 4, argc);
			send_box[3] = data >> 8;
			send_box[4] = data & 0XFF;
			data = ustrtoul_safe(argv, DEFAULT_BLINK_ON_PERIOD, 5, argc);
			send_box[5] = data >> 8;
			send_box[6] = data & 0XFF;
			data = ustrtoul_safe(argv, DEFAULT_BLINK_OFF_PERIOD, 6, argc);
			send_box[7] = data >> 8;
			send_box[8] = data & 0XFF;
			send_box[9] = 1;
			break;
		case LED_BLINK_1:
			send_box[0] = LED_PWMGPIO_FLICKER_1;
			data = ustrtoul_safe(argv, DEFAULT_LED_DUTY, 3, argc);
			send_box[1] = data >> 8;
			send_box[2] = data & 0XFF;
			data = ustrtoul_safe(argv, DEFAULT_LED_PERIOD, 4, argc);
			send_box[3] = data >> 8;
			send_box[4] = data & 0XFF;
			data = ustrtoul_safe(argv, DEFAULT_BLINK_ON_PERIOD, 5, argc);
			send_box[5] = data >> 8;
			send_box[6] = data & 0XFF;
			data = ustrtoul_safe(argv, DEFAULT_BLINK_OFF_PERIOD, 6, argc);
			send_box[7] = data >> 8;
			send_box[8] = data & 0XFF;
			send_box[9] = ustrtoul_safe(argv, DEFAULT_BLINK_CYCLE, 7, argc);
			break;
		case LED_ON_2:
			send_box[0] = LED_PWMGPIO_LIGHT_2;
			data = ustrtoul_safe(argv, DEFAULT_LED_DUTY, 3, argc);
			send_box[1] = data >> 8;
			send_box[2] = data & 0XFF;
			data = ustrtoul_safe(argv, DEFAULT_LED_PERIOD, 4, argc);
			send_box[3] = data >> 8;
			send_box[4] = data & 0XFF;
			break;
		case LED_OFF_2:
			send_box[0] = LED_PWMGPIO_DARK_2;
			break;
		case LED_BLINK_ONCE_2:
			send_box[0] = LED_PWMGPIO_FLICKER_2;
			data = ustrtoul_safe(argv, DEFAULT_LED_DUTY, 3, argc);
			send_box[1] = data >> 8;
			send_box[2] = data & 0XFF;
			data = ustrtoul_safe(argv, DEFAULT_LED_PERIOD, 4, argc);
			send_box[3] = data >> 8;
			send_box[4] = data & 0XFF;
			data = ustrtoul_safe(argv, DEFAULT_BLINK_ON_PERIOD, 5, argc);
			send_box[5] = data >> 8;
			send_box[6] = data & 0XFF;
			data = ustrtoul_safe(argv, DEFAULT_BLINK_OFF_PERIOD, 6, argc);
			send_box[7] = data >> 8;
			send_box[8] = data & 0XFF;
			send_box[9] = 1;
			break;
		case LED_BLINK_2:
			send_box[0] = LED_PWMGPIO_FLICKER_2;
			data = ustrtoul_safe(argv, DEFAULT_LED_DUTY, 3, argc);
			send_box[1] = data >> 8;
			send_box[2] = data & 0XFF;
			data = ustrtoul_safe(argv, DEFAULT_LED_PERIOD, 4, argc);
			send_box[3] = data >> 8;
			send_box[4] = data & 0XFF;
			data = ustrtoul_safe(argv, DEFAULT_BLINK_ON_PERIOD, 5, argc);
			send_box[5] = data >> 8;
			send_box[6] = data & 0XFF;
			data = ustrtoul_safe(argv, DEFAULT_BLINK_OFF_PERIOD, 6, argc);
			send_box[7] = data >> 8;
			send_box[8] = data & 0XFF;
			send_box[9] = ustrtoul_safe(argv, DEFAULT_BLINK_CYCLE, 7, argc);
			break;
		default:
			pr_err("%s: error cmd input\n", __func__);
			ret = -1;
			break;
	}

	if(ret == 0) {
		ret = mbox_send(&priv->mbox, send_box);
		if (ret) {
			debug("%s: mbox_send failed: %d\n",__func__, ret);
			return -ENOSYS;
		}
	}
	priv->state = state;

	return ret;
}

static enum led_control breath_led_get_state(struct udevice *dev)
{
	if(!is_probe)
	{
		pr_err("Not probe yet\n");
		return 0;
	}
	return priv->state;
}

static int breath_led_set_tv_status(struct udevice *dev, char * const argv[])
{
	int ret = 0, status = 0, argc = 0;
	u8 send_box[PM51_MAILBOX_SIZE] = {0};
	u8 led_index = 0;

	if(!is_probe)
	{
		pr_err("Not probe yet\n");
		return 0;
	}

	for(argc=0; argc<8; argc++)
	{
		if(argv[argc] == NULL)
			break;
	}

	status = ustrtoul_safe(argv, 0XFE, 3, argc);
	struct led_uc_plat *uc_plat = dev_get_uclass_platdata(dev);
	if (strstr(uc_plat->label, "breath-led-1"))
	{
		led_index = 1;
	}

	switch (status)
	{
		//dute/period set 0 means use default value.
		case LED_TV_STATUS_UPGRADE:
			if (led_index == 1)
				send_box[0] = LED_PWMGPIO_FLICKER_POWERON_2; //led1 keep blink
			else
				send_box[0] = LED_PWMGPIO_FLICKER_POWERON_1; //led0 keep blink
			send_box[1] = 0;
			send_box[2] = 0;
			send_box[3] = 0;
			send_box[4] = 0;
			send_box[5] = DEFAULT_BLINK_ON_PERIOD >> 8;
			send_box[6] = DEFAULT_BLINK_ON_PERIOD & 0XFF;
			send_box[7] = DEFAULT_BLINK_OFF_PERIOD >> 8;
			send_box[8] = DEFAULT_BLINK_OFF_PERIOD & 0XFF;
		break;
		case LED_TV_STATUS_MAX:
		default:
			pr_err("%s: error status input\n", __func__);
			ret = -1;
			break;
	}

	if(ret == 0) {
		ret = mbox_send(&priv->mbox, send_box);
		if (ret) {
			debug("%s: mbox_send failed: %d\n",__func__, ret);
			return -ENOSYS;
		}
	}
	return ret;
}

static int generate_set_pwm_gpio_cmd(struct led_breath_priv *priv)
{
	int i = 0, ret = 0;
	u8 send_box[PM51_MAILBOX_SIZE] = {0};
	u8 u8GpioHigh = 0, u8GpioLow = 0, idx = 0;

	send_box[idx++] = LED_MODE_PWMGPIO_SET;
	send_box[idx++] = priv->led_is_pwm;
	send_box[idx++] = priv->num_leds;

	for (i=0; i<priv->num_leds; i++)
	{
		if (priv->led_is_pwm) {
			send_box[idx++] = priv->leds[i].pwm_channel>>8;
			send_box[idx++] = priv->leds[i].pwm_channel;
			send_box[idx++] = priv->leds[i].pwm_inverse;
			pr_err("%s LED%d, pwm_pm ch=%d, inverse=%d\n", __func__, i, priv->leds[i].pwm_channel, priv->leds[i].pwm_inverse);
		}
		else
		{
			u8GpioHigh = gpio_get_number(&priv->leds[i].gpiod)>>8;
			u8GpioLow = gpio_get_number(&priv->leds[i].gpiod);
			send_box[idx++] = u8GpioHigh;
			send_box[idx++] = u8GpioLow;
			send_box[idx++] = priv->leds[i].gpiod.flags == GPIOD_ACTIVE_LOW ? 1 : 0;
			pr_err("%s LED%d, gpio num=%d, is_active_low=%d\n"
				, __func__, i, gpio_get_number(&priv->leds[i].gpiod), priv->leds[i].gpiod.flags == GPIOD_ACTIVE_LOW ? 1 : 0);
		}
	}

	ret = mbox_send(&priv->mbox, send_box);
	if (ret) {
		dev_err(dev, "%s: mbox_send failed: %d\n",__func__, ret);
		return -ENOSYS;
	}

	return ret;
}

static inline int sizeof_gpio_leds_priv(int num_leds)
{
	return sizeof(struct led_breath_priv) +
		(sizeof(struct gpio_led_data) * num_leds);
}

static inline bool led_node_available(ofnode node)
{
	const char *status;
	status = ofnode_read_string(node, "led_status");
	if (status != NULL &&
		(strncmp(status, "okay", 4) == 0 || strncmp(status, "ok", 2) == 0))
		return true;
	else
		return false;
}

static int led_breath_probe(struct udevice *dev)
{
	struct ofnode_phandle_args args;
	ofnode node;
	int ret = 0, count = 0;
	unsigned int led_use_pwm = 0, pwm_channel = 0, pwm_inverse = 0;

	if(is_probe)
	{
		pr_err("is_probe\n");
		return 0;
	}

	dev_for_each_subnode(node, dev) {
		count++;
	}
	if (count == 0) {
		pr_err("Get led count 0\n");
		return -EINVAL;
	}

	priv = malloc(sizeof_gpio_leds_priv(count));
	if (!priv) {
		dev_err(dev, "%s No memory for priv\n", __func__);
		return -ENOMEM;
	}
	memset(priv, 0, sizeof_gpio_leds_priv(count));

	ret = ofnode_read_u32(dev->node, KEY_LED_USE_PWM, &led_use_pwm);
	if(ret < 0) {
		dev_err(dev, "%s Parse led_use_pwm fail\n", __func__);
		return -EINVAL;
	}
	else {
		priv->led_is_pwm = led_use_pwm;
	}

	if (priv->led_is_pwm) {
		dev_for_each_subnode(node, dev) {
			if (!led_node_available(node))
				continue;
			ofnode_read_u32(node, KEY_PWM_CHANNEL, &pwm_channel);
			if (pwm_channel >= 0) {
				priv->leds[priv->num_leds].pwm_channel = pwm_channel;
			}
			else {
				dev_err(dev, "%s Parse pwm_channel fail\n", __func__);
				return -EINVAL;
			}
			ofnode_read_u32(node, KEY_PWM_INVERSE, &pwm_inverse);
			if (pwm_inverse >= 0) {
				priv->leds[priv->num_leds].pwm_inverse = pwm_inverse;
			}
			else {
				dev_err(dev, "%s Parse pwm_inverse fail\n", __func__);
				return -EINVAL;
			}
			priv->num_leds++;
		}
	}
	else {
		struct udevice *gpio_dev;
		for (ret = uclass_first_device(UCLASS_GPIO, &gpio_dev);
			 gpio_dev;
			 ret = uclass_next_device(&gpio_dev)) {
		}
		dev_for_each_subnode(node, dev) {
			if (!led_node_available(node))
				continue;
			ret = gpio_request_by_name_nodev(node, "gpios", 0, &priv->leds[priv->num_leds].gpiod, GPIOD_REQUESTED);
			if(ret) {
				dev_err(dev, "%s Parse gpios fail\n", __func__);
				return -EINVAL;
			}
			else {
				priv->num_leds++;
			}
		}
	}


	/* fill of args */
	args.args[0] = 57;
	args.args_count = 1;

	ret = mbox_get_by_dev_name("mailbox_to_pm51", &args, &priv->mbox);
	if (ret) {
		dev_err(dev, "%s mbox_get_by_index failed: %d\n", __func__, ret);
		return -EINVAL;
	}

	priv->state = LED_OFF_1;

	ret = generate_set_pwm_gpio_cmd(priv);
	if (ret < 0) {
		dev_err(dev, "%s generate_set_pwm_gpio_cmd failed: %d\n", __func__, ret);
		return -EINVAL;
	}

	is_probe = true;

	return 0;
}

static int led_breath_bind(struct udevice *parent)
{
	struct udevice *dev;
	ofnode node;
	int ret = 0;


	dev_for_each_subnode(node, parent) {
		struct led_uc_plat *uc_plat;
		const char *label;

		if (!led_node_available(node))
			continue;

		label = ofnode_read_string(node, "label");
		if (!label) {
			debug("%s: node %s has no label\n", __func__,
				  ofnode_get_name(node));
			return -EINVAL;
		}
		debug("%s: label %s\n", __func__, label);
		ret = device_bind_driver_to_node(parent, "breath_led",
						 ofnode_get_name(node),
						 node, &dev);
		if (ret)
			return ret;
		uc_plat = dev_get_uclass_platdata(dev);
		uc_plat->label = label;
	}

	return ret;
}

static const struct led_ops breath_led_ops = {
	.set_state		= breath_led_set_state,
	.get_state		= breath_led_get_state,
	.set_tv_status 	= breath_led_set_tv_status,
};

static const struct udevice_id led_breath_ids[] = {
	{ .compatible = "breath-leds" },
	{ }
};

U_BOOT_DRIVER(led_breath) = {
	.name	= "breath_led",
	.id	= UCLASS_LED,
	.of_match = led_breath_ids,
	.ops	= &breath_led_ops,
	.bind	= led_breath_bind,
	.probe	= led_breath_probe,
};
