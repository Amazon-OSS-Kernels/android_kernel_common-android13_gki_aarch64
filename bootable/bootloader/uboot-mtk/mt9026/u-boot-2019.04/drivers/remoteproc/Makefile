# SPDX-License-Identifier: GPL-2.0+
#
# (C) Copyright 2015
# Texas Instruments Incorporated - http://www.ti.com/
#

obj-$(CONFIG_$(SPL_)REMOTEPROC) += rproc-uclass.o rproc-elf-loader.o

# Remote proc drivers - Please keep this list alphabetically sorted.
obj-$(CONFIG_K3_SYSTEM_CONTROLLER) += k3_system_controller.o
obj-$(CONFIG_REMOTEPROC_MTK_PMU_CORTEX_M4) += mtk-pmu/
obj-$(CONFIG_REMOTEPROC_MTK_VAD_CORTEX_M4) += mtk-pmu/
obj-$(CONFIG_REMOTEPROC_MTK_PMU_MCS_51) += mtk_pmu_mcs51_rproc.o
obj-$(CONFIG_REMOTEPROC_MTK_PQU) += mtk-pqu/
obj-$(CONFIG_REMOTEPROC_K3) += k3_rproc.o
obj-$(CONFIG_REMOTEPROC_SANDBOX) += sandbox_testproc.o
obj-$(CONFIG_REMOTEPROC_TI_POWER) += ti_power_proc.o
