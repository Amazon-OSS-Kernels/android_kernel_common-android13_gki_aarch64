# SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
#
# Copyright (c) 2023 MediaTek Inc.
#

UBOOTINCLUDE += -I$(srctree)/Mediatek-internal/drivers/include/ \
                -I$(srctree)/Mediatek-internal/drivers/include/chip \
                -I$(srctree)/Mediatek-internal/impl/system/include/ \
                -I$(srctree)/Mediatek-internal/impl/system/include/secure \
                -I$(srctree)/Mediatek-internal/impl/upgrade/include/ \
                -I$(srctree)/Mediatek-internal/impl/one_package/include/ \
                -I$(srctree)/Mediatek-internal/impl/partition/include/

obj-$(CONFIG_JPEG_DECODER) += jpd/
obj-y += gop/
obj-y += panel/
obj-y += raw/
obj-$(CONFIG_ROM_OVERLAY) += romtblo/
obj-y += system/
obj-y += dtbo/
obj-$(CONFIG_DATA_SEPARATION) += dataindex/
obj-y += upgrade/
obj-y += partition/
obj-$(CONFIG_MTK_PM) += pm/
obj-$(CONFIG_MTK_PMU) += pmu/
obj-$(CONFIG_REMOTEPROC_MTK_VAD_CORTEX_M4) += cm4/
obj-y += one_package/
obj-$(CONFIG_AB_SIDELOAD) += ab/
obj-$(CONFIG_UFS_MTK_DTV) += ufstest/
obj-$(CONFIG_SECOND_STANDBY) += standby/
obj-$(CONFIG_SECOND_STANDBY) += voc/
obj-y += standby_mode/
obj-$(CONFIG_ENABLE_DEMURA) += demura/
obj-$(CONFIG_LVGL_UPGRADE_BAR) += lvgl_v6.1/
obj-$(CONFIG_SOUND) += bootmusic/
obj-$(CONFIG_MTK_LED_SETTING) += led/
