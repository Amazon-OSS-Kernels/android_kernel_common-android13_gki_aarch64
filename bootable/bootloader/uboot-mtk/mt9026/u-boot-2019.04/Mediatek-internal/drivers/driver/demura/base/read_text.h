// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

#ifndef _READ_TEXT_H_
#define _READ_TEXT_H_


typedef struct
{
    MS_U32 len;
    MS_U8*  pcont_start;    // pointer to the content between '{}'
    MS_U8*  pkey_start;     // pointer to the key behind ':'
}line_info;


//#define DEBUG_TEXT

#ifdef  DEBUG_TEXT
    #define DBG_TEXT_PRINT(msg...)     printf(msg)
#else
    #define DBG_TEXT_PRINT(msg...)
#endif


void init_text_info(char *file_buf, MS_U32 max_size);
MS_U32 get_line(line_info *linfo);
MS_U32 get_filesize(void);
MS_U32 get_file_status(void);
MS_U32 split_num_string(char *pstart, char *sep, int *pbuf, int maxsplit);
char *split_strtok(char *s, const char *delim, char **save_ptr);

#endif
