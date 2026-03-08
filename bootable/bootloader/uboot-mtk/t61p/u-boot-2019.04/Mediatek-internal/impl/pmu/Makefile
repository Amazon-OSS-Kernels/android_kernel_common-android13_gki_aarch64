# // SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
# /*
#  * Copyright (c) 2023 MediaTek Inc.
#  */

UBOOTINCLUDE += -I$(srctree)/Mediatek-internal/impl/pmu/
UBOOTINCLUDE += -I$(srctree)/Mediatek-internal/impl/cm4/

obj-$(CONFIG_REMOTEPROC_MTK_PMU_MCS_51) += pmu_ini_impl.o
obj-$(CONFIG_REMOTEPROC_MTK_PMU_MCS_51) += pmu_mcs51_impl.o
obj-$(CONFIG_MULTICORES_PLATFORM) += pmu_thread.o
