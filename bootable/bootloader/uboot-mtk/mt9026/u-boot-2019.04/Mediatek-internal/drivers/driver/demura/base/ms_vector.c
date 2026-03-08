// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

/*
 * For the coversion of vendor_to_mstar binary needs lots of memory,
 * but the malloc buffer of mboot is about 10MB .
 * No need to expand the malloc buffer, for we had large unused memory
 * during the mboot stage. So just use it.
*/
#include <MsTypes.h>
#include <stdio.h>
#include <linux/string.h>
#include <dmalloc.h>
#include <ms_vector.h>


MS_BOOL alloc_vector(int item_size, int max_num, m_vector *pvec)
{
    unsigned char *pbuf;
    unsigned int size = item_size * max_num;

    pbuf = dmalloc(size);
    if (pbuf == NULL)
    {
        printf("[%s:%d] dmalloc space failed(0x%x)!\n", __FUNCTION__, __LINE__, size);
        return FALSE;
    }
    pvec->item_size = item_size;
    pvec->max_num   = max_num;
    pvec->pbuf      = pbuf;
    pvec->dat_num   = 0;

    return TRUE;
}


void  free_vector(m_vector *ptr)
{
    dfree(ptr->pbuf);
}


void clear_vector(m_vector *ptr)
{
    ptr->dat_num = 0;
}


MS_BOOL merge_vector(m_vector *dest_vec, m_vector *src_vec)
{
    char *pdest, *psrc;
    MS_U32 cont_size;
    
    if (dest_vec->item_size != src_vec->item_size)
    {
        printf("merge_vector confilct!\n");
        printf("dest_vec->item_size = 0x%x\n", (unsigned int)dest_vec->item_size);
        printf("src_vec->item_size  = 0x%x\n", (unsigned int)src_vec->item_size);
        return FALSE;
    }
    if ((dest_vec->dat_num + src_vec->dat_num) > dest_vec->max_num)
    {
        printf("dest vector do not have enough space!\n");
        printf("dest_vec->dat_num + src_vec->dat_num = 0x%x\n", (unsigned int)(dest_vec->dat_num + src_vec->dat_num));
        printf("dest_vec->max_num  = 0x%x\n", (unsigned int)dest_vec->max_num);
        return FALSE;
    }
    
    psrc      = (char *)(src_vec->pbuf);
    cont_size = src_vec->dat_num * src_vec->item_size;
    pdest     = (char *)(dest_vec->pbuf) + dest_vec->dat_num * dest_vec->item_size;
    memcpy(pdest, psrc, cont_size);
    
    dest_vec->dat_num  +=  src_vec->dat_num;
    return TRUE;
}

