/* SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause) */
/*
 * Copyright (c) 2023 MediaTek Inc.
*/

#ifndef _NJPD_IMPL_HEADER_
#define _NJPD_IMPL_HEADER_
#include <image-njpd.h>

#define BOOTLOGO_FILE_NAME_LENGTH 0x10

typedef struct
{
    unsigned int read_buffer;
    unsigned int read_size;
    unsigned int inter_buffer;
    unsigned int inter_size;
    unsigned int output_buffer;
    unsigned int output_size;
}jpd_setting_para;

unsigned int mtk_jpd_get_alignment_width(void);
unsigned int mtk_jpd_get_alignment_height(void);
unsigned int mtk_jpd_get_alignment_pitch(void);
unsigned int mtk_jpd_get_color_format(void);
unsigned long mtk_jpd_get_output_buffer_addr(void);
unsigned long mtk_jpd_get_output_buffer_size(void);
#endif /* _NJPD_IMPL_HEADER_ */

