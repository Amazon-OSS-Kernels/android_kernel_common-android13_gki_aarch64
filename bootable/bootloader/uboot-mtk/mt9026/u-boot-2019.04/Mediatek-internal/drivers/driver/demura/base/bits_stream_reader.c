// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

#include <MsTypes.h>
#include <stdio.h>
#include <linux/string.h>
#include <asm/byteorder.h>
#include "bits_stream_reader.h"


int Init_BSReader(BitsStreamReader * pBSR, MS_U32 *pdata, MS_U32 size_in_bytes)
{
    MS_U32 pdata_temp = (MS_U32)((uintptr_t)pdata);
    memset(pBSR, 0, sizeof(BitsStreamReader));
    if (size_in_bytes & 0xE0000000)
    {
        printf("%s Error : data_size (0x%x) is too large !", __FUNCTION__, (unsigned int)size_in_bytes);
        return -1;
    }
    if ((pdata_temp & 3) != 0 )
    {
        // For efficiency
        printf("pdata(%p) should align 4 Byte", pdata);
        return -1;
    }
    pBSR->pdata = pdata;
    pBSR->total_bytes = size_in_bytes;
    pBSR->total_bits  = (size_in_bytes << 3);
    pBSR->cur_bit_pos = 0;
    return 0;
}


MS_U32 BSR_ReadData (BitsStreamReader * pBSR, MS_U8 bit_num)
{
    MS_U32 result    = 0;
    MS_U32 *pbuf     = pBSR->pdata;
    MS_U32 cur_bpos  = pBSR->cur_bit_pos;

    if ((bit_num >= 32) || (bit_num <= 0))
    {
        printf("BSR_ReadData : bit_num(%d) is too large/small !\n", bit_num);
        return 0;
    }
    if ((cur_bpos + bit_num) > pBSR->total_bits)
    {
        printf("BSR_ReadData : Not enough data (0x%x < 0x%x)\n", (unsigned int)(cur_bpos + bit_num), (unsigned int)pBSR->total_bits);
        return 0;
    }

    MS_U32 left_data_pos = (cur_bpos >> 5);
    MS_U32 right_data_pos = ((cur_bpos + bit_num) >> 5);
    
    if (left_data_pos != right_data_pos)
    {
        MS_U64 cal_val = 0;
        MS_U32 left_data  = __le32_to_cpu( pbuf[left_data_pos] );
        MS_U64 right_data = __le32_to_cpu( pbuf[right_data_pos] );
        cal_val = left_data | (right_data << 32);
        cal_val = cal_val >> (cur_bpos & 31);
        cal_val = cal_val & ((((MS_U64)1 << bit_num) - 1));
        result  = (MS_U32)cal_val;
    }
    else
    {
        {
            MS_U32 cal_val = 0;
            cal_val = __le32_to_cpu( pbuf[(cur_bpos >> 5)] );
            cal_val = cal_val >> (cur_bpos & 31);
            cal_val = cal_val & ((1 << bit_num) - 1);
            result  = cal_val;
        }
    }
    pBSR->cur_bit_pos += bit_num;
    return result;
}


void BSR_ShiftBits(BitsStreamReader * pBSR, MS_U32 bit_num)
{
    if ((pBSR->cur_bit_pos + bit_num) > pBSR->total_bits)
    {
        printf("Warning : BSR_ShiftBits bit_num(0x%x + 0x%x) exceed total_bits(0x%x), Trunk !\n", \
                (unsigned int)pBSR->cur_bit_pos, (unsigned int)bit_num, (unsigned int)pBSR->total_bits);
        pBSR->cur_bit_pos = pBSR->total_bits;
    }
    else
    {
        pBSR->cur_bit_pos += bit_num;
    }
}

