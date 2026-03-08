// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (c) 2023 MediaTek Inc.
*/

#include <common.h>
#include <command.h>

#include "debug_impl.h"

#if defined(CONFIG_REMOTEPROC_MTK_PMU_CORTEX_M4)
#include "cm4_impl.h"
#define PMU_DEVNODE "/pmu_rproc"
#endif
#if defined(CONFIG_REMOTEPROC_MTK_PMU_MCS_51)
#include "pmu_ini_impl.h"
#include "pmu_mcs51_impl.h"
#endif
static bool bPmuInit=0;

int do_boot_pmu(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int ret = 0;
#if defined(CONFIG_REMOTEPROC_MTK_PMU_CORTEX_M4) || defined(CONFIG_REMOTEPROC_MTK_PMU_MCS_51)
	unsigned int start = 0, end = 0;
#endif

	UBOOT_TRACE("do command: boot_pmu.\n");
	if (bPmuInit)
	{
		UBOOT_ERROR("do command: boot_pmu. PMU already init\n");
	}
	else
	{
#if defined(CONFIG_REMOTEPROC_MTK_PMU_CORTEX_M4)
		start = get_timer(0);
		ret = cm4_loader(PMU_DEVNODE);
		end = get_timer(0);
		UBOOT_BOOTTIME("[boot_pmu][cm4][ret=%d][start:%u][end:%u][total time:%u].\n",
						ret, start, end, end - start);
#endif
#if defined(CONFIG_REMOTEPROC_MTK_PMU_MCS_51)
		start = get_timer(0);
		pmu_ini_loader();
		ret = pmu_mcs51_loader();
		end = get_timer(0);
		UBOOT_BOOTTIME("[boot_pmu][mcs51][ret=%d][start:%u][end:%u][total time:%u].\n",
						ret, start, end, end - start);
#endif
		bPmuInit = 1;
	}
	return ret;
}

U_BOOT_CMD(
	boot_pmu, 2, 1, do_boot_pmu,
	"boot coprocessor with pmu\n",
	NULL
);

int do_consult_pmu(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    int ret = 0;
#if defined(CONFIG_REMOTEPROC_MTK_PMU_CORTEX_M4) || defined(CONFIG_REMOTEPROC_MTK_PMU_MCS_51)
    unsigned int start = 0, end = 0;
#endif

    UBOOT_TRACE("do command: consult_pmu.\n");
#if defined(CONFIG_REMOTEPROC_MTK_PMU_CORTEX_M4)
    start = get_timer(0);
    ret = cm4_consulter(PMU_DEVNODE);
    end = get_timer(0);
    UBOOT_BOOTTIME("[consult_pmu][cm4][ret=%d][start:%u][end:%u][total time:%u].\n",
                    ret, start, end, end - start);
#endif
#if defined(CONFIG_REMOTEPROC_MTK_PMU_MCS_51)
    start = get_timer(0);
    ret = pmu_mcs51_consulter();
    end = get_timer(0);
    UBOOT_BOOTTIME("[consult_pmu][mcs51][ret=%d][start:%u][end:%u][total time:%u].\n",
                    ret, start, end, end - start);
#endif
    return ret;
}

U_BOOT_CMD(
    consult_pmu, 2, 1, do_consult_pmu,
    "consult coprocessor with pmu\n",
    NULL
);
