# SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
#
# Copyright (c) 2023 MediaTek Inc.
#

UBOOTINCLUDE += -I$(srctree)/lib/libufdt/include/ \
                -I$(srctree)/Mediatek-internal/drivers/include/ \
                -I$(srctree)/Mediatek-internal/drivers/include/chip/ \
                -I$(srctree)/Mediatek-internal/impl/jpd/ \
                -I$(srctree)/Mediatek-internal/impl/gop/ \
                -I$(srctree)/Mediatek-internal/impl/panel/ \
                -I$(srctree)/Mediatek-internal/impl/raw/ \
                -I$(srctree)/Mediatek-internal/impl/cm4/ \
                -I$(srctree)/Mediatek-internal/include/

ifdef MEDIATEK_PMU_CM4_DTB_SIZE
EXTRA_CFLAGS += -DPMU_CM4_DTB_SIZE=$(MEDIATEK_PMU_CM4_DTB_SIZE)
else
EXTRA_CFLAGS += -DPMU_CM4_DTB_SIZE=0x6C00
endif
obj-y += ./cm4_impl.o
obj-$(CONFIG_MULTICORES_PLATFORM) += ./cm4_thread.o
