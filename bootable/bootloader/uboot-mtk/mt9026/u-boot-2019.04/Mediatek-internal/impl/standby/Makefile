# SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
#
# Copyright (c) 2023 MediaTek Inc.
#

UBOOTINCLUDE += -I$(srctree)/Mediatek-internal/impl/system/include/ \
                -I$(srctree)/Mediatek-internal/impl/standby/ \
                -I$(srctree)/Customization/$(CUSTOMER)/standby/include/ \
                -I$(srctree)/Mediatek-internal/include/ \

obj-$(CONFIG_SECOND_STANDBY) += ./standby_impl.o
obj-$(CONFIG_MULTICORES_PLATFORM) += ./standby_thread.o
