// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2015 Thomas Chou <thomas@wytron.com.tw>
 */

#include <common.h>
#include <dm.h>
#include <dm/device-internal.h>
#include <errno.h>
#include <linux/soc/mediatek/mtk-miup.h>

/**
 * mtk_miup_probe - Probe the device @dev if not already done
 *
 * @dev: U-Boot device to probe
 *
 * @return 0 on success, an error otherwise.
 */
int mtk_miup_probe(struct udevice *dev)
{
	if (device_active(dev))
		return 0;

	return device_probe(dev);
}

UCLASS_DRIVER(mtk_miup) = {
	.id		= UCLASS_MIUP,
	.name		= "mtk-miup",
	.per_device_auto_alloc_size = sizeof(struct miup_info),
};
