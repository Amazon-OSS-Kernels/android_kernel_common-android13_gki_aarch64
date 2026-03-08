// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

#ifndef _MS_ARRAY_H_
#define _MS_ARRAY_H_

struct array_dim2
{
    int    item_size;
    int    dim1_num;
    int    dim2_num;
    void **dim1_buf;
    void  *dim2_buf;
};

struct array_dim4
{
    int      item_size;
    int      dim1_num;
    int      dim2_num;
    int      dim3_num;
    int      dim4_num;
    void ****dim1_buf;
    void  ***dim2_buf;
    void   **dim3_buf;
    void    *dim4_buf;
};


MS_BOOL  malloc_array_dim2(struct array_dim2 *p, int item_size, int dim1_num, int dim2_num);
void free_array_dim2(struct array_dim2 *p);

MS_BOOL  malloc_array_dim4(struct array_dim4 *p, int item_size, int dim1, int dim2, int dim3, int dim4);
void free_array_dim4(struct array_dim4 *p);


#define DEFINE_ARRAY_DIM2(struct_name, isize, dim1_num, dim2_num) ({ \
    malloc_array_dim2(&struct_name, isize, dim1_num, dim2_num); \
    }) 

#define FREE_ARRAY_DIM2(struct_name)    free_array_dim2(&struct_name)
#define PTR_ARRAY_DIM2(struct_name)     (struct_name.dim1_buf)


#define DEFINE_ARRAY_DIM4(struct_name, isize, dim1, dim2, dim3, dim4) ({ \
    malloc_array_dim4(&struct_name, isize, dim1, dim2, dim3, dim4);  \
    })

#define FREE_ARRAY_DIM4(struct_name)    free_array_dim4(&struct_name)
#define PTR_ARRAY_DIM4(struct_name)     (struct_name.dim1_buf)

#endif

