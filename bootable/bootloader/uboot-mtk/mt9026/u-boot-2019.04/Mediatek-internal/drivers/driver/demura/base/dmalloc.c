// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

#include <MsTypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <linux/string.h>
#include <xlist.h>

//#define  DEBUG

#ifdef DEBUG
   #define DBG(fmt, args...)  printf("In %s :", __FUNCTION__); printf(fmt, ##args)
#else
   #define DBG(msg, ...)
#endif


/*  chunk status */
#define CHUNK_INUSE    0x00455355    // "USE"
#define CHUNK_FREE     0

typedef struct _xmalloc_chunk
{
    unsigned int      stat;          // INUSE or FREE
    unsigned int      size;          // malloc_size + sizeof(xmalloc_chunk)
    struct list_head  list;
}xmalloc_chunk;


/*  sizes, alignments */
#define SIZE_SZ                (sizeof(double))
#define MALLOC_ALIGNMENT       (SIZE_SZ + SIZE_SZ)
#define MALLOC_ALIGN_MASK      (MALLOC_ALIGNMENT - 1)
#define MINSIZE                (sizeof(xmalloc_chunk))

#define XMALLOC_MAGIC_NUM       0x20170101

typedef struct _xmalloc_info
{
    void *            malloc_start;
    void *            malloc_end;
    unsigned int      total_size;
    unsigned int      used_size;
    unsigned char     *ptr_max;       // use to dectect the max length of dmalloc
    unsigned int      init_flag;
    struct list_head  chunk_head;
}xmalloc_info;


static xmalloc_info mem_info;


void dmalloc_init(unsigned char *start, unsigned int size)
{
    struct list_head *head = &(mem_info.chunk_head);

    // init mem_info
    memset(&mem_info, 0, sizeof(mem_info));
    mem_info.malloc_start = start;
    mem_info.malloc_end   = start + size;
    mem_info.total_size   = size;
    mem_info.used_size    = 0;
    mem_info.init_flag    = XMALLOC_MAGIC_NUM;
    INIT_LIST_HEAD(head);
    //printf("dmalloc addr : 0x%x, length : 0x%x\n", start, size);

    // init the first chunk
    xmalloc_chunk *pchunk  = (xmalloc_chunk *)start;
    pchunk->stat  = CHUNK_FREE;
    pchunk->size  = size;
    list_add(&(pchunk->list), head);
}


void* dmalloc(unsigned int size)
{
#if 0
    struct list_head  *head;
    xmalloc_chunk     *pchunk, *pnew_chunk;
    unsigned int      alloc_size, remain_size;
    unsigned char     *pbuf, *pbase;

    if (mem_info.init_flag != XMALLOC_MAGIC_NUM)
    {
        printf("dmalloc do not inited !\n");
        return NULL;
    }

    head  = &(mem_info.chunk_head);
    alloc_size = size + sizeof(xmalloc_chunk);
    if ((alloc_size & MALLOC_ALIGN_MASK) != 0)
    {
        alloc_size += MALLOC_ALIGNMENT;
        alloc_size &= (~MALLOC_ALIGN_MASK);
    }
    DBG("alloc_size = 0x%x\n", alloc_size);

    list_for_each_entry(pchunk, head, list)
    {
        if ((pchunk->stat == CHUNK_FREE) && (pchunk->size >= alloc_size))
        {
            pchunk->stat = CHUNK_INUSE;
            pbuf = (unsigned char *)pchunk + sizeof(xmalloc_chunk);
            remain_size = pchunk->size - alloc_size;
            
            if (remain_size >= (MINSIZE*2))  
            {
                // split the space, else merge the remain size
                pchunk->size      = alloc_size;
                
                pbase             = (unsigned char *)pchunk + alloc_size;
                pnew_chunk        = (xmalloc_chunk *)pbase;
                pnew_chunk->stat  = CHUNK_FREE;
                pnew_chunk->size  = remain_size;
                list_add(&(pnew_chunk->list), &(pchunk->list));
            }
            
            // update mem_info.ptr_max
            pbase = (unsigned char *)pchunk + pchunk->size;
            if ((unsigned long)pbase > (unsigned long)mem_info.ptr_max)
            {
                mem_info.ptr_max = pbase;
            }
            
            return (void *)pbuf;
        }
    }
    DBG("malloc space failed !\n");
    return NULL;
#else    
    return malloc(size);
#endif
}


void dfree(void *p)
{
#if 0
    struct list_head  *head, *plist;
    xmalloc_chunk     *pchunk;
    xmalloc_chunk     *prev_chk, *next_chk;
    unsigned int      free_size;

    if (mem_info.init_flag != XMALLOC_MAGIC_NUM)
    {
        printf("dmalloc do not inited !\n");
        return;
    }
    if (p == NULL)
    {
        return;
    }

    head    = &(mem_info.chunk_head);
    pchunk  = (xmalloc_chunk *)((char *)p - sizeof(xmalloc_chunk));
    if (pchunk->stat != CHUNK_INUSE)
    {
        printf("\nxfree(%p) \033[31;1merror\033[0m, not a available chunk\n", p);
        return;
    }
    pchunk->stat  = CHUNK_FREE;

    // if "current_chunk and prev_chunk" ==> "prev_chunk"
    if (! list_is_first(&(pchunk->list), head) )
    {
        plist     = pchunk->list.prev;
        prev_chk  = container_of(plist, xmalloc_chunk, list);
        if (prev_chk->stat == CHUNK_FREE)
        {
            DBG("delete the list of current_chunk\n");
            free_size = pchunk->size;
            list_del(&(pchunk->list));
            prev_chk->size += free_size;
            pchunk = prev_chk;
        }
    }

    // if "current_chunk and next_chunk" ==> "current_chunk"
    if (! list_is_last(&(pchunk->list), head) )
    {
        plist     = pchunk->list.next;
        next_chk  = container_of(plist, xmalloc_chunk, list);
        if (next_chk->stat == CHUNK_FREE)
        {
            DBG("delete the list of next_chunk\n");
            free_size = next_chk->size;
            list_del(plist);
            pchunk->size += free_size;
        }
    }
#else
    if(p != NULL) 
        free(p);
#endif
}


//  The following function just for debug
unsigned int get_dmalloc_length(void)
{
#if 0
    unsigned int len;
    len = (unsigned int)((char *)mem_info.ptr_max - (char *)mem_info.malloc_start);
    return len;
#else
    return 0;
#endif
}


void dump_dmalloc_chunk(void)
{
#if 0
    unsigned int  i;
    xmalloc_chunk *pchk;
    struct list_head  *head;

    if (mem_info.init_flag != XMALLOC_MAGIC_NUM)
    {
        printf("dmalloc do not inited !\n");
        return;
    }

    i  = 0;
    head  = &(mem_info.chunk_head);
    printf("\n");

    list_for_each_entry(pchk, head, list)
    {
        printf("######## chunk_%02d ########\n", i);
        printf("status  : %s\n", (pchk->stat == CHUNK_INUSE ? "in_use" : "free"));
        printf("size    : 0x%x\n", pchk->size);
        printf("ptr_dat : %p\n",  (char *)pchk + sizeof(*pchk));
        printf("\n");
        i = i + 1;
    }
#endif
}

