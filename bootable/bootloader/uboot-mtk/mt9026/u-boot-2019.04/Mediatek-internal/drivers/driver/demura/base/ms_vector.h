// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

#ifndef _MS_VECTOR_H_
#define _MS_VECTOR_H_


typedef struct
{
    MS_U32  item_size;      // the element size
    MS_U32  max_num;        // the max data num of vector
    MS_U8   *pbuf;          // pointer to data
    MS_U32  dat_num;        // the valid data num of vector
} m_vector;


MS_BOOL alloc_vector(int item_size, int max_num, m_vector *pvec);
void  free_vector(m_vector *ptr);
void clear_vector(m_vector *ptr);
MS_BOOL merge_vector(m_vector *dest_vec, m_vector *src_vec);

#endif
