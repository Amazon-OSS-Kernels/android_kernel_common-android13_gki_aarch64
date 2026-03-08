// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2015 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <led.h>
#include <dm/device-internal.h>
#include <dm/root.h>
#include <dm/uclass-internal.h>

int led_get_by_label(const char *label, struct udevice **devp)
{
	struct udevice *dev;
	struct uclass *uc;
	int ret;

	ret = uclass_get(UCLASS_LED, &uc);
	if (ret)
		return ret;
	uclass_foreach_dev(dev, uc) {
		struct led_uc_plat *uc_plat = dev_get_uclass_platdata(dev);

		/* Ignore the top-level LED node */
		if (uc_plat->label && !strcmp(label, uc_plat->label))
			return uclass_get_device_tail(dev, 0, devp);
	}

	return -ENODEV;
}

int led_set_state(struct udevice *dev, char * const argv[])
{
	struct led_ops *ops = led_get_ops(dev);

	if (!ops->set_state)
		return -ENOSYS;

	return ops->set_state(dev, argv);
}

enum led_control led_get_state(struct udevice *dev)
{
	struct led_ops *ops = led_get_ops(dev);

	if (!ops->get_state)
		return -ENOSYS;

	return ops->get_state(dev);
}

int led_set_tv_status(struct udevice *dev, char * const argv[])
{
    struct led_ops *ops = led_get_ops(dev);

    if (!ops->set_tv_status)
        return -ENOSYS;

    return ops->set_tv_status(dev, argv);
}

int led_default_state(void)
{
	struct udevice *dev;
	struct uclass *uc;
	const char *default_state;
	int ret;

	ret = uclass_get(UCLASS_LED, &uc);
	if (ret)
		return ret;
	for (uclass_find_first_device(UCLASS_LED, &dev);
	     dev;
	     uclass_find_next_device(&dev)) {
		default_state = dev_read_string(dev, "default-state");
		if (!default_state)
			continue;
		ret = device_probe(dev);
		if (ret)
			return ret;
		/* default-state = "keep" : device is only probed */
	}

	return ret;
}

UCLASS_DRIVER(led) = {
	.id		= UCLASS_LED,
	.name		= "led",
	.per_device_platdata_auto_alloc_size = sizeof(struct led_uc_plat),
};
