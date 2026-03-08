// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

#ifndef _XMALLOC_H_
#define _XMALLOC_H_


void  dmalloc_init(unsigned char *start, unsigned int size);
void* dmalloc(unsigned int size);
void  dfree(void *p);

unsigned int get_dmalloc_length(void);
void dump_dmalloc_chunk(void);


#if 0
#define  CHECK_DMALLOC_SPACE(ptr, alloc_size)   \
    if (ptr == NULL) \
    {                \
        printf("In File(%s), Function(%s), Line(%d), Can Not get memory(0x%x bytes) from dmalloc\n", \
                __FILE__, __FUNCTION__, __LINE__, alloc_size); \
        dump_dmalloc_chunk();  \
        return FALSE;          \
    }
#else
#define  CHECK_DMALLOC_SPACE(ptr, alloc_size)   \
    if (ptr == NULL) \
    {                \
        printf("In File(%s), Function(%s), Line(%d), Can Not get memory(0x%x bytes) from dmalloc\n", \
                __FILE__, __FUNCTION__, __LINE__, alloc_size); \
        return FALSE;          \
    }
#endif

#endif
