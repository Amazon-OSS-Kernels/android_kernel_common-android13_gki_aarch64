# SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
#
# Copyright (c) 2023 MediaTek Inc.
#

UBOOTINCLUDE += -I$(srctree)/Customization/$(CUSTOMER)/dtbo/include/ \
		-I$(srctree)/Mediatek-internal/impl/romtblo/ \
		-I$(srctree)/Mediatek-internal/impl/upgrade/include/ \
		-I$(srctree)/Mediatek-internal/secure/

obj-y += src/dtbo.o
obj-y += src/overlay_setting.o
obj-y += src/dtbo_cfg_sel.o
