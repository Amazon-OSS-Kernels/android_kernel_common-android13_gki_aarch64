// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

#include <MsTypes.h>
#include <common.h>
#include <stdio.h>
#include <linux/string.h>
#include <stdlib.h>

//#define DEBUG_TEXT

#include "read_text.h"


#define  MAX_LINE_LEN   (800 * 1024 * 1024)  // Normal Demura case : the length of line can not reach it.
#ifdef MSOS_TYPE_LINUX_KERNEL
#define mst_atoi(str) simple_strtoul(((str != NULL) ? str : ""), NULL, 0);
#else
#define mst_atoi(str) strtoul(((str != NULL) ? str : ""), NULL, 0);
#endif


typedef struct
{
    MS_U8  *fbuf;
    MS_U32 bytes_read;     // the bytes of had read
    MS_U32 init_flag;      // 1 : had inited this struct
    MS_U32 eof_flag;       // 1 : end of file
    MS_U32 file_broken;    // 1 : invalid demura file.
    MS_U32 filesize;
    MS_U32 max_size;       // The max filesize
    MS_U32 brace_depth;    // the depth of symbol "{}"
    MS_U32 bracket_depth;  // the depth of symbol "[]"
}file_info;


static file_info text_info;


void init_text_info(char *file_buf, MS_U32 max_size)
{
    memset(&text_info, 0, sizeof(text_info));
    text_info.fbuf  = (unsigned char *)file_buf;
    text_info.init_flag = 1;
    text_info.max_size  = max_size;
}


#ifdef CONFIG_DEMURA_VENDOR_CSOT

// return 0 --> Do not jump out the while loop
// return 1 --> Do jump out the while loop
static inline int handle_csot_line(line_info *linfo, unsigned char *buf, int i)
{
    char ch = buf[i];
    
    if (ch == '{')
    {
        linfo->pcont_start = buf + i + 1;
        text_info.brace_depth += 1;
    }
    else if (ch == '}')
    {
        buf[i] = '\0';
        text_info.brace_depth -= 1;
        if (text_info.brace_depth == 0)    // For text file, seem it is valid way to get the length of file.
        {
            text_info.eof_flag = 1;
            printf("This file haved reached the End_Of_File position\n");
            return 1;
        }
    }
    else if (ch == ':')    // Get key string
    {
        if (i > 0)
        {
            char temp = buf[i-1];
            if ((temp == '"') && (i >= 2))
            {
                linfo->pkey_start = linfo->pcont_start + 1;
                buf[i-1] = '\0';
            }
            else
            {
                linfo->pkey_start = linfo->pcont_start;
                buf[i] = '\0';
            }
        }
        else
        {
            printf("Warning, this ini file may be damaged !\n");
        }
    }
    
    return 0;
}

#endif


MS_U32 get_line(line_info *linfo)
{
    int i;
    unsigned char ch;
    unsigned char *buf;
    buf = text_info.fbuf + text_info.bytes_read;
    memset(linfo, 0, sizeof(line_info));
    
    DBG_TEXT_PRINT("in %s : ", __FUNCTION__);
    // check if file had been read out
    if (text_info.eof_flag == 1)
    {
        return 0;
    }
    else if (text_info.init_flag != 1)
    {
        printf("text_info had not been inited !\n");
        return 0;
    }
    
    // init line_info struct
    linfo->pcont_start = buf;
    
    for (i = 0; i < MAX_LINE_LEN; i++)
    {
        ch = buf[i];
        DBG_TEXT_PRINT(" %02x", (unsigned int)(ch & 0xff));
        if ( ((buf + i) - text_info.fbuf) > text_info.max_size )
        {
            printf("Invaild demura table file !\n");
            text_info.file_broken = 1;
            return 0;
        }
        
        if ((ch == '\r') || (ch == '\n'))    // Time to return
        {
            if ((buf[i+1] == '\n') || (buf[i+1] == '\r'))
            {
                i = i + 1;
            }
            break;
        }
    #ifdef CONFIG_DEMURA_VENDOR_CSOT
        else if (handle_csot_line(linfo, buf, i))    // handle csot line gramma
        {
            break;
        }
    #endif
    }
    
    // Update text_info
    i = i + 1;      // remeber to add
    linfo->len = i;
    text_info.bytes_read += i;
    DBG_TEXT_PRINT("  (i = %d)\n", i);  // test
    
    return i;
}


MS_U32 split_num_string(char *pstart, char *sep, int *pbuf, int maxsplit)
{
    int i = 0;
    char *tok = NULL;
    char *ptr = pstart;
    char *saveptr = pstart;

    tok = split_strtok (ptr, sep, &saveptr);
    while (tok)
    {
        pbuf[i] = mst_atoi(tok);
        tok = split_strtok (NULL, sep, &saveptr);
        if(tok == NULL) ;
        i = i + 1;
        if ((maxsplit > 0) && (maxsplit == i))
        {
            break;
        }
    }
    return i;
}


MS_U32 get_filesize(void)
{
    if (text_info.eof_flag == 1)
    {
        return text_info.filesize;
    }
    else
    {
        return 0;
    }
}


MS_U32 get_file_status(void)
{
    return text_info.file_broken;
}

char *split_strtok(char *s, const char *delim, char **save_ptr)
{
	char *token;

    if (save_ptr == NULL)
    {
        return NULL;
    }

	if (s == NULL)
		s = *save_ptr;

	/* Scan leading delimiters.  */
	s += strspn(s, delim);
	if (*s == '\0') {
		*save_ptr = s;
		return NULL;
	}

	/* Find the end of the token.  */
	token = s;
	s = strpbrk (token, delim);
	if (s == NULL) {
		/* This token finishes the string.  */
		*save_ptr = memchr(token, '\0', strlen(token));
		if (*save_ptr) {} // fix coverity
	} else {
		/* Terminate the token and make *SAVE_PTR point past it.  */
		*s = '\0';
		*save_ptr = s + 1;
	}
	return token;


}
