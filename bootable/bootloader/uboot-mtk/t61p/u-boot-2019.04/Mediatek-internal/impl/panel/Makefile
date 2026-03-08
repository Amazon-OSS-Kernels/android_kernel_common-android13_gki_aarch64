# SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
#
# Copyright (c) 2023 MediaTek Inc.
#

UBOOTINCLUDE += -I$(srctree)/Mediatek-internal/drivers/include/ \
                -I$(srctree)/Mediatek-internal/drivers/include/chip/ \
                -I$(srctree)/Mediatek-internal/impl/panel/ \
                -I$(srctree)/Mediatek-internal/impl/demura/ \
                -I$(srctree)/Mediatek-internal/impl/gop/ \
                -I$(srctree)/Mediatek-internal/impl/jpd/ \
                -I$(srctree)/Mediatek-internal/include/ \
                -I$(srctree)/Customization/$(CUSTOMER)/standby/include/ \
                -I$(srctree)/Customization/$(CUSTOMER)/pnl_cust/include/

obj-y += ./panel_impl.o
obj-$(CONFIG_MULTICORES_PLATFORM) += ./panel_thread.o
