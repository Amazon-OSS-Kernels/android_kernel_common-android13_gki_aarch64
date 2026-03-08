# SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
#
# Copyright (c) 2023 MediaTek Inc.
#

UBOOTINCLUDE += -I$(srctree)/Mediatek-internal/drivers/include/ \
                -I$(srctree)/Mediatek-internal/drivers/include/chip/ \
                -I$(srctree)/Mediatek-internal/impl/system/include/ \
                -I$(srctree)/Mediatek-internal/impl/panel/ \
                -I$(srctree)/Mediatek-internal/impl/gop/ \
                -I$(srctree)/Mediatek-internal/impl/jpd/ \
                -I$(srctree)/Mediatek-internal/impl/standby_mode/ \
                -I$(srctree)/Mediatek-internal/impl/pmu/ \
                -I$(srctree)/Mediatek-internal/impl/cm4/ \
                -I$(srctree)/Mediatek-internal/impl/led/ \
                -I$(srctree)/Mediatek-internal/impl/partition/include/ \
                -I$(srctree)/Mediatek-internal/include/

obj-y += ./debug_impl.o
obj-y += ./flow.o
obj-y += ./cmdtable.o
obj-y += ./dts_parser.o
obj-y += ./utility.o
obj-y += ./iniparser.o
obj-y += ./iniutility.o
obj-y += ./system.o
obj-y += ./rpmb.o
obj-y += ./boot_impl.o
obj-$(CONFIG_MULTICORES_PLATFORM) += ./flow_thread.o
