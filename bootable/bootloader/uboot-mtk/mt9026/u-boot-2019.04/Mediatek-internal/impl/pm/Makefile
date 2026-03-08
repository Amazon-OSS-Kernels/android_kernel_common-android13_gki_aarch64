# // SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
# /*
#  * Copyright (c) 2023 MediaTek Inc.
#  */

UBOOTINCLUDE += -I$(srctree)/Mediatek-internal/impl/system/include/ \
                -I$(srctree)/Mediatek-internal/impl/pm/ \
                -I$(srctree)/Mediatek-internal/include/ \

obj-$(CONFIG_MTK_PM) += ./mtk-pm.o
obj-$(CONFIG_MULTICORES_PLATFORM) += ./mtk-pm_thread.o
