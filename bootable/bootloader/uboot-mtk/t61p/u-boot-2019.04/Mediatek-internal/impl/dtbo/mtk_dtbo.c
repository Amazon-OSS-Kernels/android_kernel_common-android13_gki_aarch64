// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

#include <common.h>
#include <malloc.h>
#include <fdt_support.h>
#include <debug_impl.h>
#include <image-android-dt.h>
#include <mtk_dtbo.h>
#ifdef CONFIG_LIBUFDT_OVERLAY
#include <ufdt_overlay.h>
#endif

DECLARE_GLOBAL_DATA_PTR;

static DTB_INFO_T dtb_info;

bool is_uboot_dtb(void)
{
    return dtb_info.is_uboot_dtb;
}

void get_uboot_dtb_addr(unsigned long *dtb_addr)
{
	*dtb_addr = (unsigned long)gd->fdt_blob;
}

void set_dtbo_image_address(unsigned long dtbo_img_addr)
{
	dtb_info.dtbo_img_addr = dtbo_img_addr;
}

void set_dtb_image_address(unsigned long dtb_addr, bool is_uboot_dtb)
{
	dtb_info.dtb_addr = dtb_addr;
	dtb_info.is_uboot_dtb = is_uboot_dtb;
}

void init_dtbo_selection(void)
{
	UBOOT_INFO("init_dtbo_selection return\n");
}

static int dt_binary_selection(unsigned long dtbo_img_addr,int select_id,unsigned long *dtb_addr,unsigned long *dtb_size)
{
	bool ret;
	ret = android_dt_check_header(dtbo_img_addr);
	if(ret == false)
	{
		UBOOT_ERROR("Check device tree binary image header magic number failure, please check dtbo.img or storage cmd.\n");
		return -EINVAL;
	}
	ret = android_dt_get_fdt_by_index(dtbo_img_addr,select_id,dtb_addr,(unsigned int *)dtb_size);
	if(ret == false)
	{
		UBOOT_ERROR("Select dtb from dtbo.img failure, please check device id or dtbo.img\n");
		return -EINVAL;
	}
	return 0;
}

unsigned long get_dtb_image_address(void)
{
    return dtb_info.dtb_addr;
}

int dtb_overlay(int dtbo_id)
{
	int ret;
	unsigned long dtb_addr = 0;
	unsigned long dtb_size = 0;
#ifdef CONFIG_LIBUFDT_OVERLAY
	struct fdt_header *source;
#endif

	ret = dt_binary_selection(dtb_info.dtbo_img_addr, dtbo_id, &dtb_addr, &dtb_size);
	if (ret == 0)
	{
		struct fdt_header *blob = (struct fdt_header *)dtb_addr;
		ret = fdt_check_header(blob);
		if (ret < 0)
		{
			UBOOT_ERROR("Device tree binary header verify failure at dtb_source address %p\n",blob);
			return ret;
		}
#ifdef CONFIG_LIBUFDT_OVERLAY
		source = (struct fdt_header *)dtb_info.dtb_addr;

		source = ufdt_apply_overlay((struct fdt_header *)dtb_info.dtb_addr, fdt32_to_cpu(source->totalsize), (void *)blob, dtb_size);
		if (source == NULL)
		{
			UBOOT_ERROR("ufdt_apply_overlay execute failure.\n");
			return ret;
		}
		else
		{
			UBOOT_DEBUG("%d dtb of dtbo.img execute overlay to destination success.\n",dtbo_id);
			fdt_open_into(source, (void *)dtb_info.dtb_addr, fdt32_to_cpu(source->totalsize));
			free(source);
		}
#else
		ret = fdt_overlay_apply_verbose((void *)dtb_info.dtb_addr, blob);
		if (ret < 0)
		{
			UBOOT_ERROR("fdt_overlay_apply_verbose execute failure.\n");
			return ret;
		}
#endif
	}
	else
	{
		UBOOT_ERROR("dt_binary_selection execute failure.\n");
		return ret;
	}
	return 0;
}
