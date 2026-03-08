# SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
#
# Copyright (c) 2023 MediaTek Inc.
#

UBOOTINCLUDE += -I$(srctree)/Mediatek-internal/drivers/include/ \
                -I$(srctree)/Mediatek-internal/drivers/include/chip/ \
                -I$(srctree)/Mediatek-internal/drivers/panel/ \
                -I$(srctree)/Mediatek-internal/drivers/gop/ \
                -I$(srctree)/Mediatek-internal/include/ \
                -I$(srctree)/Mediatek-internal/impl/ab/ \
                -I$(srctree)/Mediatek-internal/impl/jpd/ \
                -I$(srctree)/Mediatek-internal/impl/gop/ \
                -I$(srctree)/Mediatek-internal/impl/panel/ \
                -I$(srctree)/Mediatek-internal/impl/pmu/ \
                -I$(srctree)/Mediatek-internal/impl/cm4/ \
                -I$(srctree)/Mediatek-internal/impl/voc/ \
                -I$(srctree)/Mediatek-internal/impl/standby/ \
                -I$(srctree)/Mediatek-internal/impl/standby_mode/ \
                -I$(srctree)/Mediatek-internal/impl/romtblo/ \
                -I$(srctree)/Mediatek-internal/impl/system/include/ \
                -I$(srctree)/Customization/$(CUSTOMER)/dtbo/include/ \
                -I$(srctree)/Customization/$(CUSTOMER)/standby/include/ \
                -I$(srctree)/drivers/mmc/mtk-dtv/ \
                -I$(srctree)/Mediatek-internal/impl/demura/ \
                -I$(srctree)/Mediatek-internal/impl/ufstest/

obj-y += ./cmd_boot.o
obj-y += ./cmd_system.o
obj-y += ./cmd_bootlogo.o
obj-y += ./cmd_panel.o
obj-y += ./cmd_table.o
obj-y += ./cmd_debug.o
obj-y += ./cmd_dtbo.o
obj-y += ./cmd_dramc.o
obj-$(CONFIG_ROM_OVERLAY) += ./cmd_romtblo.o
obj-$(CONFIG_MTK_PM) += ./cmd_pm.o
obj-$(CONFIG_MTK_PMU) += ./cmd_pmu.o
obj-$(CONFIG_REMOTEPROC_MTK_VAD_CORTEX_M4) += ./cmd_vad.o
obj-$(CONFIG_SECOND_STANDBY) += ./cmd_voc.o
obj-y += ./cmd_upgrade.o
obj-y += ./cmd_partition.o
obj-y += ./cmd_utility.o
obj-y += ./cmd_devid.o
obj-$(CONFIG_SOUND) += ./cmd_bootmusic.o
obj-$(CONFIG_AB_SIDELOAD) += ./cmd_ab.o
obj-$(CONFIG_MMC_MTK_DTV) += ./cmd_emmc.o
obj-$(CONFIG_UFS_MTK_DTV) += ./cmd_ufstest.o
obj-y += ./cmd_console_log.o
obj-$(CONFIG_SECOND_STANDBY) += ./cmd_standby.o
obj-$(CONFIG_DATA_SEPARATION) += ./cmd_dataindex.o
obj-$(CONFIG_NAND_MTK_DTV) += ./cmd_nand.o
obj-$(CONFIG_ENABLE_DEMURA) += ./cmd_demura.o
obj-y += ./cmd_efuse.o


