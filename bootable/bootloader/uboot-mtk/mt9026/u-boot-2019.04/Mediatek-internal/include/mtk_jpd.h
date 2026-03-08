/* SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause) */
/*
 * Copyright (c) 2023 MediaTek Inc.
*/

#ifndef _MTK_JPD_PUBLIC_HEADER_
#define _MTK_JPD_PUBLIC_HEADER_

#define LOGO_FORMAT_RAW                 "RAW"
#define LOGO_FORMAT_RAW_STRING_SIZE     3
#define LOGO_FORMAT_JPEG                "JPEG"
#define LOGO_FORMAT_JPEG_STRING_SIZE    4

bool mtk_jpd_decode(char *part, const char *path);

#endif
