/* SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause) */
/*
 * Copyright (c) 2023 MediaTek Inc.
*/

#ifndef _MTK_RAW_DATA_PUBLIC_HEADER_
#define _MTK_RAW_DATA_PUBLIC_HEADER_

unsigned int mtk_raw_data_get_output_buffer_addr(void);
unsigned int mtk_raw_data_get_output_buffer_size(void);
int mtk_raw_data_load_logo_to_dram(char *partition, const char *path);

#endif
