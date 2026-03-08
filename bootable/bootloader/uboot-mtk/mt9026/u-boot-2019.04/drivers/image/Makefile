# SPDX-License-Identifier: GPL-2.0+
#
# Copyright (c) 2021 MediaTek Inc.
obj-$(CONFIG_MTK_NJPD) += mtk-njpd.o
obj-$(CONFIG_MTK_NJPD) += mtk-njpd-uclass.o
obj-$(CONFIG_MTK_NJPD) += njpd/mtk_sti_msos.o
obj-$(CONFIG_MTK_NJPD) += njpd/api/mtk_api_jpd.o
#obj-$(CONFIG_MTK_NJPD) += njpd/api/mtk_jpeg_memory.o
obj-$(CONFIG_MTK_NJPD) += njpd/drv/mtk_drv_jpd.o
obj-$(CONFIG_MTK_NJPD) += njpd/hal/mtk_hal_jpd.o
ccflags-$(CONFIG_MTK_NJPD) += -I$(srctree)/$(src)/njpd/
ccflags-$(CONFIG_MTK_NJPD) += -I$(srctree)/$(src)/njpd/api/
ccflags-$(CONFIG_MTK_NJPD) += -I$(srctree)/$(src)/njpd/drv/
ccflags-$(CONFIG_MTK_NJPD) += -I$(srctree)/$(src)/njpd/hal/
