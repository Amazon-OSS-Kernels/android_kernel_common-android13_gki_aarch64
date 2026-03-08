/* SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause) */
/*
 * Copyright (c) 2023 MediaTek Inc.
*/

#ifndef _PMU_THREAD_H_
#define _PMU_THREAD_H_

#if defined(CONFIG_MULTICORES_PLATFORM)
#if defined(CONFIG_MTK_PMU)
void *boot_pmu_thread_entry(void *args);
#endif
#endif
#endif /* _PMU_THREAD_H_ */
