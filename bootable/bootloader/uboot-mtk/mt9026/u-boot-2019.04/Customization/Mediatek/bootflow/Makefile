# SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
#
# Copyright (c) 2023 MediaTek Inc.
#

UBOOTINCLUDE += -I$(srctree)/Customization/$(CUSTOMER)/bootflow/include/ \
                -I$(srctree)/Mediatek-internal/impl/gop/

obj-y += src/bootflow.o
obj-$(CONFIG_MULTICORES_PLATFORM) += src/bootflow_thread.o
