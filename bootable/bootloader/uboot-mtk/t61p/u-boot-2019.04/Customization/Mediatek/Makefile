# SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
#
# Copyright (c) 2023 MediaTek Inc.
#

UBOOTINCLUDE += -I$(srctree)/Mediatek-internal/drivers/include/ \
                -I$(srctree)/Mediatek-internal/drivers/include/chip/ \
                -I$(srctree)/Mediatek-internal/impl/system/include/ \
                -I$(srctree)/Customization/$(CUSTOMER)/ \
                -I$(srctree)/Customization/$(CUSTOMER)/bluetooth/include/ \
                -I$(srctree)/Customization/$(CUSTOMER)/console_lock/include/ \
                -I$(srctree)/Customization/$(CUSTOMER)/pnl_cust/include/ \
                -I$(srctree)/drivers/video/mtk-dtv/

obj-y += bootflow/
obj-y += dtbo/
obj-y += partition/
obj-y += bluetooth/
obj-$(CONFIG_ENABLE_CUST_IC_UPDATE) += pnl_cust/
obj-$(CONFIG_SECOND_STANDBY) += standby/
obj-$(CONFIG_CONSOLE_LOCK) += console_lock/
