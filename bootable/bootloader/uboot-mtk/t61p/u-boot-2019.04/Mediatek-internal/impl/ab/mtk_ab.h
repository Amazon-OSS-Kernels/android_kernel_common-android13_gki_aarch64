/* SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause) */
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

#ifndef _AB_IMPL_HEADER_
#define _AB_IMPL_HEADER_

#define PART_SUFFIX_A	"_a"
#define PART_SUFFIX_B	"_b"

enum PARTITION_SELECT
{
    PARTITION_SELECT_A = 0,
    PARTITION_SELECT_B,
    PARTITION_SELECT_MAX,
};

int get_ab_selection_result(void);
int ab_pre_selection(void);
int ab_selection(char *storage, char *part, int pre_select);

#endif