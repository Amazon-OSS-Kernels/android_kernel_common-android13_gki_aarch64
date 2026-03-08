// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (c) 2023 MediaTek Inc.
*/

#include <dm/ofnode.h>
#include <linux/ioport.h>
#include <linux/io.h>

#include "common.h"
#include "command.h"
#include "debug_impl.h"
#include "system_impl.h"
#include "mtk-pm.h"
#include "standby_impl.h"

#define PM_WK_NAME_SIZE				(10)

#define PM_DRV_SHM_BIT_NUMS			(32)

#define PM_REG_PM_SLEEP				(0)
#define PM_OFS_CFG_00_15_DUMMY		(((0x28 << 1) * 2) + 0)
#define PM_OFS_CFG_16_31_DUMMY		(((0x29 << 1) * 2) + 0)
#define PM_OFS_WK_DUMMY				(((0x39 << 1) * 2) + 0)
#define PM_REG_PM_POR				(1)
#define PM_OFS_WK_KEY_DUMMY			(((0x02 << 1) * 2) + 0)
#define PM_OFS_BR_DUMMY				(((0x7E << 1) * 2) + 0)

struct pm_wakeup_source {
	const char *name;
	uint8_t id;
	uint8_t shm_bit;
	struct list_head node;
};

struct pm_device {
	void __iomem *reg_pm_sleep;
	void __iomem *reg_pm_por;
	struct list_head wk_src_list;
};

static struct pm_device *_default_pm_dev = NULL;

static struct pm_wakeup_source *pm_find_wakeup_source(struct pm_device *pm,
								const char *name, uint8_t id)
{
	struct pm_wakeup_source *wk_src = NULL;

	list_for_each_entry(wk_src, &pm->wk_src_list, node) {
		if (((name) && (strcmp(wk_src->name, name) == 0)) ||
			((id) && (wk_src->id == id)))
			return wk_src;
	}

	return NULL;
}

static int pm_ofnode_read_resource(ofnode node, uint index, void __iomem **base)
{
	int errno = 0;
	struct resource regs;

	errno = ofnode_read_resource(node, index, &regs);
	if (errno < 0) {
		UBOOT_ERROR("ofnode_read_resource(%d) not found.\n", index);
		return errno;
	}
	UBOOT_INFO("reg=<0x%tX 0x%tX>.\n", (size_t)regs.start, (size_t)(regs.end - regs.start));

	*base = ioremap(regs.start, regs.end - regs.start);
	if (*base == NULL) {
		UBOOT_ERROR("ioremap() fail.\n");
		return -ENOMEM;
	}
	return 0;
}

int32_t pm_init(void)
{
	struct pm_device *pm = NULL;
	ofnode node = {0}, sub_node = {0};
	const char *prop = NULL;
	int size = 0;
	struct pm_wakeup_source *wk_src = NULL;
	int errno = 0;
	uint32_t data = 0;
	uint8_t shm_bit = 0;

	PMU_REG_ACCESS(PMU_DUMMY_ADDR, PMU_DUMMY_OFFSET_BOOT_STATUS) = 0;

	if (_default_pm_dev)
		return 0;

	pm = calloc(1, sizeof(struct pm_device));
	if (!pm)
		return -ENOMEM;

	node = ofnode_path("/mtk-pm");
	if (!ofnode_valid(node)) {
		UBOOT_ERROR("ofnode_path(/mtk-pm) not found.\n");
		errno = -ENODEV;
		goto pm_init_err;
	}

	prop = ofnode_get_property(node, "compatible", &size);
	if ((!prop) || (strncmp(prop, "mediatek,pm", size - 1))) {
		UBOOT_ERROR("compatible no match.\n");
		errno = -ENOENT;
		goto pm_init_err;
	}

	if (((errno = pm_ofnode_read_resource(node, PM_REG_PM_SLEEP, &pm->reg_pm_sleep)) < 0) ||
		((errno = pm_ofnode_read_resource(node, PM_REG_PM_POR, &pm->reg_pm_por)) < 0)) {
		goto pm_init_err;
	}
	UBOOT_INFO("pm_sleep=0x%p.\n", pm->reg_pm_sleep);
	UBOOT_INFO("pm_por=0x%p.\n", pm->reg_pm_por);

	_default_pm_dev = pm;

	/* Clear wakeup config. */
	writew(0x0000, pm->reg_pm_sleep + PM_OFS_CFG_00_15_DUMMY);
	writew(0x0000, pm->reg_pm_sleep + PM_OFS_CFG_16_31_DUMMY);
	/* Init wakeup config. */
	INIT_LIST_HEAD(&pm->wk_src_list);
	node = ofnode_find_subnode(node, "wakeup-source");
	ofnode_for_each_subnode(sub_node, node) {
		if (shm_bit >= PM_DRV_SHM_BIT_NUMS) {
			UBOOT_ERROR("wk_src name=%-10s can't get shm_bit.\n", ofnode_get_name(sub_node));
			continue;
		}
		wk_src = calloc(1, sizeof(*wk_src));
		if (wk_src) {
			if (ofnode_read_u32(sub_node, "id", &data) < 0) {
				UBOOT_ERROR("Read necessary attribute fail.\n");
				free(wk_src);
				continue;
			}
			wk_src->name = ofnode_get_name(sub_node);
			wk_src->id = (uint8_t)data;
			wk_src->shm_bit = shm_bit++;

			list_add_tail(&wk_src->node, &pm->wk_src_list);
			data = 0;
			if (ofnode_read_u32(sub_node, "enable", &data) == 0)
				pm_set_wakeup_config(wk_src->name, (bool)data);

			UBOOT_INFO("wk_src name=%-10s, id=0x%02X, enable=%u, shm_bit=%02d.\n",
						wk_src->name, wk_src->id, data, wk_src->shm_bit);
		}
	}

	return 0;

pm_init_err:
	if (pm->reg_pm_sleep)
		iounmap(pm->reg_pm_sleep);
	if (pm->reg_pm_por)
		iounmap(pm->reg_pm_por);
	if (pm)
		free(pm);

	return errno;
}

int32_t pm_get_boot_reason(void)
{
	struct pm_device *pm = _default_pm_dev;

	if (!pm)
		return -EINVAL;

	return readb(pm->reg_pm_por + PM_OFS_BR_DUMMY);
}

int32_t pm_set_boot_reason(uint8_t reason)
{
	struct pm_device *pm = _default_pm_dev;

	if (!pm)
		return -EINVAL;

	writeb(reason, pm->reg_pm_por + PM_OFS_BR_DUMMY);

	return 0;
}

int32_t pm_get_wakeup_reason(void)
{
	struct pm_device *pm = _default_pm_dev;

	if (!pm)
		return -EINVAL;

	return readb(pm->reg_pm_sleep + PM_OFS_WK_DUMMY);
}

int32_t pm_set_wakeup_reason(const char *name)
{
	struct pm_device *pm = _default_pm_dev;
	struct pm_wakeup_source *wk_src = NULL;

	if (!pm)
		return -EINVAL;

	if (name) {
		wk_src = pm_find_wakeup_source(pm, name, 0x00);
		if (wk_src)
			writeb(wk_src->id, pm->reg_pm_sleep + PM_OFS_WK_DUMMY);
	} else {
		writeb(0x00, pm->reg_pm_sleep + PM_OFS_WK_DUMMY);
	}

	return 0;
}

char *pm_get_wakeup_reason_str(void)
{
	struct pm_device *pm = _default_pm_dev;
	struct pm_wakeup_source *wk_src = NULL;
	uint8_t data = 0;

	if (!pm)
		return NULL;

	data = (uint8_t)pm_get_wakeup_reason();
	wk_src = pm_find_wakeup_source(pm, NULL, data);

	return wk_src ? (char *)wk_src->name : NULL;
}

int32_t pm_get_wakeup_key(void)
{
	struct pm_device *pm = _default_pm_dev;
	if (!pm)
		return -EINVAL;

	return readw(pm->reg_pm_por + PM_OFS_WK_KEY_DUMMY);
}

int32_t pm_set_wakeup_key(uint16_t key)
{
	struct pm_device *pm = _default_pm_dev;

	if (!pm)
		return -EINVAL;

	writew(key, pm->reg_pm_por + PM_OFS_WK_KEY_DUMMY);

	return 0;
}

int32_t pm_get_wakeup_config(const char *name, bool *enable)
{
	struct pm_device *pm = _default_pm_dev;
	struct pm_wakeup_source *wk_src = NULL;
	void __iomem *base = NULL;

	if ((!pm) || (!name) || (!enable))
		return -EINVAL;

	wk_src = pm_find_wakeup_source(pm, name, 0x00);
	if (wk_src) {
		if ((wk_src->shm_bit / 16) == 0)
			base = pm->reg_pm_sleep + PM_OFS_CFG_00_15_DUMMY;
		else if ((wk_src->shm_bit / 16) == 1)
			base = pm->reg_pm_sleep + PM_OFS_CFG_16_31_DUMMY;
		else
			return -ENXIO;
		*enable = !!(readw(base) & BIT(wk_src->shm_bit % 16));
	}

	return wk_src ? 0 : -ENXIO;
}

int32_t pm_set_wakeup_config(const char *name, bool enable)
{
	struct pm_device *pm = _default_pm_dev;
	struct pm_wakeup_source *wk_src = NULL;
	void __iomem *base = NULL;
	uint16_t data = 0, mask = 0;

	if ((!pm) || (!name))
		return -EINVAL;

	wk_src = pm_find_wakeup_source(pm, name, 0x00);
	if (wk_src) {
		if ((wk_src->shm_bit / 16) == 0)
			base = pm->reg_pm_sleep + PM_OFS_CFG_00_15_DUMMY;
		else if ((wk_src->shm_bit / 16) == 1)
			base = pm->reg_pm_sleep + PM_OFS_CFG_16_31_DUMMY;
		else
			return -ENXIO;

		mask = BIT(wk_src->shm_bit % 16);
		if (enable)
			data = (readw(base) & ~mask) | (0xFFFF & mask);
		else
			data = (readw(base) & ~mask) | (0x0000 & mask);
		writew(data, base);
	}

	return wk_src ? 0 : -ENXIO;
}
