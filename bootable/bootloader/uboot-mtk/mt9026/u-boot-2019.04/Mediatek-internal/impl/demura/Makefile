# SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
#
# Copyright (c) 2023 MediaTek Inc.
#

UBOOTINCLUDE += -I$(srctree)/include \
                -I$(srctree)/Mediatek-internal/include \
                -I$(srctree)/Mediatek-internal/drivers/include \
                -I$(srctree)/Mediatek-internal/drivers/include/chip/ \
				-I$(srctree)/Mediatek-internal/drivers/driver/demura/ \
				-I$(srctree)/Mediatek-internal/impl/demura/include/ \
				-I$(srctree)/Mediatek-internal/impl/panel/include/ \
				-I$(srctree)/Mediatek-internal/impl/system/include/



obj-$(CONFIG_ENABLE_DEMURA) += ./demura_impl.o
