// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

#include <stdio.h>
#include <linux/string.h>
#include <dmalloc.h>
#include <MsTypes.h>
#include "ms_array.h"

MS_BOOL  malloc_array_dim2(struct array_dim2 *p, int item_size, int dim1_num, int dim2_num)
{
    memset(p, 0, sizeof(*p));
    p->item_size = item_size;
    p->dim1_num  = dim1_num;
    p->dim2_num  = dim2_num;

    p->dim1_buf = (void **)dmalloc(sizeof(char **) * dim1_num);
    //CHECK_DMALLOC_SPACE(p->dim1_buf, (unsigned int)(sizeof(char **) * dim1_num));
    if(p->dim1_buf==NULL)
    {
        return FALSE;
    }

    p->dim2_buf = (void *)dmalloc(item_size * dim1_num * dim2_num);
    //CHECK_DMALLOC_SPACE(p->dim2_buf, (unsigned int)(item_size * dim1_num * dim2_num));
    if(p->dim2_buf==NULL)
    {
        dfree(p->dim1_buf);
        p->dim1_buf = NULL;
        return FALSE;
    }

    int i;
    for (i = 0; i < dim1_num; i++)
    {
        p->dim1_buf[i] = (void *)((size_t)(p->dim2_buf) + dim2_num * item_size * i);
    }
    return TRUE;
}


void free_array_dim2(struct array_dim2 *p)
{
    dfree(p->dim2_buf);
    dfree(p->dim1_buf);
    p->dim1_buf = NULL;
    p->dim2_buf = NULL;
}


MS_BOOL  malloc_array_dim4(struct array_dim4 *p, int item_size, int dim1, int dim2, int dim3, int dim4)
{
    memset(p, 0, sizeof(*p));
    p->item_size = item_size;
    p->dim1_num  = dim1;
    p->dim2_num  = dim2;
    p->dim3_num  = dim3;
    p->dim4_num  = dim4;

    p->dim1_buf = (void ****)dmalloc(sizeof(char *) * dim1);
    //CHECK_DMALLOC_SPACE(p->dim1_buf, (unsigned int)(sizeof(char *) * dim1));
    if(p->dim1_buf==NULL)
    {
        return FALSE;
    }

    p->dim2_buf = (void ***)dmalloc(sizeof(char *) * dim1 * dim2);
    //CHECK_DMALLOC_SPACE(p->dim2_buf, (unsigned int)(sizeof(char *) * dim1 * dim2));
    if(p->dim2_buf==NULL)
    {
        dfree(p->dim1_buf);
        p->dim1_buf = NULL;
        return FALSE;
    }


    p->dim3_buf = (void **)dmalloc(sizeof(char *) * dim1 * dim2 * dim3);
    //CHECK_DMALLOC_SPACE(p->dim3_buf, (unsigned int)(sizeof(char *) * dim1 * dim2 * dim3));
    if(p->dim3_buf==NULL)
    {
        dfree(p->dim2_buf);
        dfree(p->dim1_buf);
        p->dim1_buf = NULL;
        p->dim2_buf = NULL;
        return FALSE;
    }

    p->dim4_buf = (void *)dmalloc(item_size * dim1 * dim2 * dim3 * dim4);
    //CHECK_DMALLOC_SPACE(p->dim4_buf, (unsigned int)(item_size * dim1 * dim2 * dim3 * dim4));
    if(p->dim4_buf==NULL)
    {
        dfree(p->dim3_buf);
        dfree(p->dim2_buf);
        dfree(p->dim1_buf);
        p->dim1_buf = NULL;
        p->dim2_buf = NULL;
        p->dim3_buf = NULL;
        return FALSE;
    }

    int i, j, k;
    for (i = 0; i < dim1; i++)
    {
        p->dim1_buf[i] = (void ***)((size_t)(p->dim2_buf) +  sizeof(char *) * i * dim2);
        for (j = 0; j < dim2; j++)
        {
            p->dim1_buf[i][j] = (void **)((size_t)(p->dim3_buf) + sizeof(char *) * ((i * dim2 + j) *dim3));
            for (k = 0; k < dim3; k++)
            {
                p->dim1_buf[i][j][k] = (void *)((size_t)(p->dim4_buf) + item_size * (((i * dim2 + j)* dim3 + k) * dim4));
            }
        }
    }

    return TRUE;
}


void free_array_dim4(struct array_dim4 *p)
{
    dfree(p->dim4_buf);
    dfree(p->dim3_buf);
    dfree(p->dim2_buf);
    dfree(p->dim1_buf);
    p->dim1_buf = NULL;
    p->dim2_buf = NULL;
    p->dim3_buf = NULL;
    p->dim4_buf = NULL;
}
