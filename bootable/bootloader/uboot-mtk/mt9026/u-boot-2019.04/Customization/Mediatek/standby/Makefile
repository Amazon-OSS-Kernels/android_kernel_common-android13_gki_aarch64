# SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
#
# Copyright (c) 2023 MediaTek Inc.
#

UBOOTINCLUDE += -I$(srctree)/Customization/$(CUSTOMER)/standby/include/

ifeq ($(CONFIG_MTK_LED_SETTING),y)
UBOOTINCLUDE += -I$(srctree)/Mediatek-internal/impl/led/
endif

obj-$(CONFIG_SECOND_STANDBY) += src/standby.o
