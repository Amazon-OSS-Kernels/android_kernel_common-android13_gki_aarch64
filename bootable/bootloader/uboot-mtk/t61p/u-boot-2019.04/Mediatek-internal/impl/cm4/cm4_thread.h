/* SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause) */
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

#ifndef _CM4_THREAD_H_
#define _CM4_THREAD_

#if defined(CONFIG_MULTICORES_PLATFORM)
#if defined(CONFIG_REMOTEPROC_MTK_VAD_CORTEX_M4)
void *run_cm4_thread_entry(void *args);
#endif
#endif
#endif /* _CM4_THREAD_H_ */

