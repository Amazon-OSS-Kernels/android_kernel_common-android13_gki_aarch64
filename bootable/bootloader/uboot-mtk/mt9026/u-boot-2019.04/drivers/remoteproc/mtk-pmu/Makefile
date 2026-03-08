# SPDX-License-Identifier: GPL-2.0+
#
# (C) Copyright 2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
obj-$(CONFIG_REMOTEPROC_MTK_PMU_CORTEX_M4) += ./CPUINT/halCPUINT.o
obj-$(CONFIG_REMOTEPROC_MTK_PMU_CORTEX_M4) += ./CM4/halCM4.o
obj-$(CONFIG_REMOTEPROC_MTK_PMU_CORTEX_M4) += ./VBDMA/halBDMA.o
obj-$(CONFIG_REMOTEPROC_MTK_PMU_CORTEX_M4) += ./VBDMA/drvBDMA.o
#obj-$(CONFIG_REMOTEPROC_MTK_PMU_CORTEX_M4) += ./MBX/halMBX.o
#obj-$(CONFIG_REMOTEPROC_MTK_PMU_CORTEX_M4) += ./MBX/drvMBX.o
obj-$(CONFIG_REMOTEPROC_MTK_PMU_CORTEX_M4) += mdrv_pmu.o
obj-$(CONFIG_REMOTEPROC_MTK_PMU_CORTEX_M4) += mtk_pmu_cm4_rproc.o
ccflags-$(CONFIG_REMOTEPROC_MTK_PMU_CORTEX_M4) += -I$(srctree)/$(src)/
ccflags-$(CONFIG_REMOTEPROC_MTK_PMU_CORTEX_M4) += -I$(srctree)/$(src)/CM4/
ccflags-$(CONFIG_REMOTEPROC_MTK_PMU_CORTEX_M4) += -I$(srctree)/$(src)/VBDMA/
ccflags-$(CONFIG_REMOTEPROC_MTK_PMU_CORTEX_M4) += -I$(srctree)/$(src)/CPUINT/
cppflags-$(CONFIG_REMOTEPROC_MTK_PMU_CORTEX_M4) += -I$(srctree)/$(src)/
cppflags-$(CONFIG_REMOTEPROC_MTK_PMU_CORTEX_M4) += -I$(srctree)/$(src)/CM4/
cppflags-$(CONFIG_REMOTEPROC_MTK_PMU_CORTEX_M4) += -I$(srctree)/$(src)/VBDMA/
cppflags-$(CONFIG_REMOTEPROC_MTK_PMU_CORTEX_M4) += -I$(srctree)/$(src)/CPUINT/

obj-$(CONFIG_REMOTEPROC_MTK_VAD_CORTEX_M4) += ./CPUINT/halCPUINT.o
obj-$(CONFIG_REMOTEPROC_MTK_VAD_CORTEX_M4) += ./CM4/halCM4.o
obj-$(CONFIG_REMOTEPROC_MTK_VAD_CORTEX_M4) += ./VBDMA/halBDMA.o
obj-$(CONFIG_REMOTEPROC_MTK_VAD_CORTEX_M4) += ./VBDMA/drvBDMA.o
#obj-$(CONFIG_REMOTEPROC_MTK_VAD_CORTEX_M4) += ./MBX/halMBX.o
#obj-$(CONFIG_REMOTEPROC_MTK_VAD_CORTEX_M4) += ./MBX/drvMBX.o
obj-$(CONFIG_REMOTEPROC_MTK_VAD_CORTEX_M4) += mdrv_pmu.o
obj-$(CONFIG_REMOTEPROC_MTK_VAD_CORTEX_M4) += mtk_vad_cm4_rproc.o
ccflags-$(CONFIG_REMOTEPROC_MTK_VAD_CORTEX_M4) += -I$(srctree)/$(src)/
ccflags-$(CONFIG_REMOTEPROC_MTK_VAD_CORTEX_M4) += -I$(srctree)/$(src)/CM4/
ccflags-$(CONFIG_REMOTEPROC_MTK_VAD_CORTEX_M4) += -I$(srctree)/$(src)/VBDMA/
ccflags-$(CONFIG_REMOTEPROC_MTK_VAD_CORTEX_M4) += -I$(srctree)/$(src)/CPUINT/
cppflags-$(CONFIG_REMOTEPROC_MTK_VAD_CORTEX_M4) += -I$(srctree)/$(src)/
cppflags-$(CONFIG_REMOTEPROC_MTK_VAD_CORTEX_M4) += -I$(srctree)/$(src)/CM4/
cppflags-$(CONFIG_REMOTEPROC_MTK_VAD_CORTEX_M4) += -I$(srctree)/$(src)/VBDMA/
cppflags-$(CONFIG_REMOTEPROC_MTK_VAD_CORTEX_M4) += -I$(srctree)/$(src)/CPUINT/
