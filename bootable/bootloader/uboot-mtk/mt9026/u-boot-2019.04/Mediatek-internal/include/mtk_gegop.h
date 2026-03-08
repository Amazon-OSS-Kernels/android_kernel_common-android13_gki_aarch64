/* SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause) */
/*
 * Copyright (c) 2023 MediaTek Inc.
*/

#ifndef _MTK_GOP_PUBLIC_HEADER_
#define _MTK_GOP_PUBLIC_HEADER_
#include <iniutility.h>

typedef enum
{
    E_LOGO_TYPE_NONE,
    E_LOGO_TYPE_JPG,
    E_LOGO_TYPE_RAWDATA,
} EN_LOGO_TYPE;

typedef struct
{
    unsigned short disp_x;
    unsigned short disp_y;
    unsigned char horizontal_stretch;
    unsigned char vertical_stretch;
}logo_disp_config_params;

int mtk_gop_get_resolution(int *resolution_width, int *resolution_height, int *hsync_start);
int mtk_gop_show_logo(logo_disp_config_params config, struct bootlogo_info *logo_info, unsigned int logo_output_buf, unsigned int logo_output_buf_size);
#if defined(CONFIG_MULTICORES_PLATFORM)
void *show_logo_thread_entry(void *args);
#endif

#endif
