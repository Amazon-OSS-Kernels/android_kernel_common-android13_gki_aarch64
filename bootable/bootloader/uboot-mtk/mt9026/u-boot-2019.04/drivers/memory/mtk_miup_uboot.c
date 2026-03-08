// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2014
 * Heiko Schocher, DENX Software Engineering, hs@denx.de.
 */
#include <common.h>
#include <dm/device.h>
#include <dm/uclass-internal.h>
#include <linux/soc/mediatek/mtk-miup.h>

static void mem_probe_uclass_mem_devs(void)
{
	struct udevice *dev;
	int idx = 0;

	/* Probe devices with DM compliant drivers */
	while (!uclass_find_device(UCLASS_MIUP, idx, &dev) && dev) {
		mtk_miup_probe(dev);
		idx++;
	}
}

int mtk_miup_probe_devices(void)
{
	mem_probe_uclass_mem_devs();
	return 0;
}
