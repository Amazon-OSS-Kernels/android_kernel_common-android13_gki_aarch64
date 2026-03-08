// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

#include <stdio.h>
#include <common.h>
#include <MsTypes.h>
#include <debug_impl.h>
#include <linux/string.h>
#include <asm/byteorder.h>
#include "vendor.h"


// seed ==> 0xFF  or  0x00
MS_U8 inx_crc8(MS_U8 *data, MS_U8 seed, MS_U32 crcStart, MS_U32 crcLength)
{
    MS_U8 coef[8] = { 1, 2, 4, 8, 16, 32, 64, 128 };
    MS_U8 crc_out = seed;

    MS_U8 shift[9];
    MS_U8 crc[9];
    MS_U8 din[9];

    MS_U32 index, i;
    for (index = crcStart; index < crcStart + crcLength; index += 1)
    {

        for (i = 0; i < 8; i += 1)
        {
            if ((crc_out & coef[i]) != 0)
                crc[i] = 1;
            else
                crc[i] = 0;

            if ((data[index] & coef[i]) != 0)
                din[i] = 1;
            else
                din[i] = 0;
        }

        shift[1] = (MS_U8)(din[7] ^ crc[7]);
        shift[2] = (MS_U8)(din[6] ^ crc[6]);
        shift[3] = (MS_U8)(din[5] ^ crc[5]);
        shift[4] = (MS_U8)(din[4] ^ crc[4]);
        shift[5] = (MS_U8)(din[3] ^ crc[3] ^ shift[1]);
        shift[6] = (MS_U8)(din[2] ^ crc[2] ^ shift[1] ^ shift[2]);
        shift[7] = (MS_U8)(din[1] ^ crc[1] ^ shift[1] ^ shift[2] ^ shift[3]);
        shift[8] = (MS_U8)(din[0] ^ crc[0] ^ shift[2] ^ shift[3] ^ shift[4]);

        crc[7] = (MS_U8)(shift[1] ^ shift[3] ^ shift[4] ^ shift[5]);
        crc[6] = (MS_U8)(shift[2] ^ shift[4] ^ shift[5] ^ shift[6]);
        crc[5] = (MS_U8)(shift[3] ^ shift[5] ^ shift[6] ^ shift[7]);
        crc[4] = (MS_U8)(shift[4] ^ shift[6] ^ shift[7] ^ shift[8]);
        crc[3] = (MS_U8)(shift[5] ^ shift[7] ^ shift[8]);
        crc[2] = (MS_U8)(shift[6] ^ shift[8]);
        crc[1] = (MS_U8)(shift[7]);
        crc[0] = (MS_U8)(shift[8]);

        crc_out = 0;

        for (i = 0; i < 8; i += 1)
        {
            crc_out += (MS_U8)(crc[i] * coef[i]);
        }
    }
    return crc_out;
}


MS_BOOL parse_vendor_header(MS_U8 *buf, INX_Demura_Header *phdr)
{
    unsigned int i = 0;
    MS_U32 sum  = 0;
    MS_U8  *pch = NULL;
    INX_Block_INFO *pblk;

    memcpy(phdr, buf, sizeof(INX_Demura_Header));
    if (phdr->magic_id != INX_INDENTIFICATION)
    {
        UBOOT_ERROR("Header Indentifiction  Error(0x%x != 0x%x)\n", phdr->magic_id, INX_INDENTIFICATION);
        return FALSE;
    }
    pch = &(phdr->asic[0]);
    if ((pch[0] == 0x09) && (pch[1] == 0x09)&& (pch[2] == 0x09) \
         && (pch[3] == 0x09)&& (pch[4] == 0x09))
    {
        UBOOT_ERROR("ASIC Error(ASIC_Value == 0x0909090909)\n");
        return FALSE;
    }
    if ((pch[0] == 0xFF) && (pch[1] == 0xFF)&& (pch[2] == 0xFF) \
         && (pch[3] == 0xFF)&& (pch[4] == 0xFF))
    {
        UBOOT_ERROR("ASIC Error(ASIC_Value == 0xFfFfFfFfFf)\n");
        return FALSE;
    }

    // Adjust Demura Header Byte-Order
    phdr->date = inx32_to_cpu(phdr->date);
    for (i = 0; i < ARRAY_SIZE(phdr->block); i++)
    {
        pblk = &(phdr->block[i]);
        pblk->data_checksum = inx16_to_cpu(pblk->data_checksum) >> 8;  // Low Byte : 0x00
        pblk->data_start    = inx32_to_cpu(pblk->data_start);
        pblk->data_length   = inx32_to_cpu(pblk->data_length);
    }
    phdr->lut_checksum    = inx16_to_cpu(phdr->lut_checksum);
    phdr->header_checksum = inx16_to_cpu(phdr->header_checksum);

    // Header checksum
    sum = 0;
    for (i = 0; i < offsetof(INX_Demura_Header, header_checksum); i++)
    {
        sum += buf[i];
    }
    if (phdr->header_checksum != sum)
    {
        UBOOT_ERROR("Header Checksum Error(0x%x != 0x%x)\n", (uint)phdr->header_checksum, (uint)sum);
        return FALSE;
    }

    if ((phdr->function_define & BIT0) == 0)
    {
        UBOOT_ERROR("Demura Definition Error(0x%x & BIT0 != BIT0)\n", (uint)phdr->function_define);
#ifndef (INX_DEMURA_DL_FLOW)
        return FALSE;
#endif
    }


    // Adjust Extend_Block_Header Byte-Order
    if (phdr->extension_block)
    {
        buf = buf + offsetof(INX_Demura_Header, extend);

        memcpy(&(phdr->extend), buf, sizeof(_INX_Extend_Header));
        for (i = 0; i < ARRAY_SIZE(phdr->extend.block); i++)
        {
            pblk = &(phdr->extend.block[i]);
            pblk->data_checksum = inx16_to_cpu(pblk->data_checksum) >> 8;  // Low Byte : 0x00
            pblk->data_start    = inx32_to_cpu(pblk->data_start);
            pblk->data_length   = inx32_to_cpu(pblk->data_length);
        }
        phdr->extend.header_checksum = inx16_to_cpu(phdr->extend.header_checksum);

        sum = 0;
        for (i = 0; i < offsetof(_INX_Extend_Header, header_checksum); i++)
        {
            sum += buf[i];
        }
        if (phdr->extend.header_checksum != sum)
        {
            UBOOT_ERROR("Extend Header Checksum Error(0x%x != 0x%x)\n", (uint)phdr->extend.header_checksum, (uint)sum);
            return FALSE;
        }
    }
    return TRUE;
}


MS_BOOL fetch_decode_info(INX_Demura_Header *phdr, Vendor2Mstar_Info *pv2m)
{
    #if(INX_DEMURA_DL_FLOW)
    pv2m->function_define = phdr->function_define;
    #endif
    
    // ==> Currently, this value is fixed !
    pv2m->hblock_size = 8;
    pv2m->vblock_size = 8;
    pv2m->hblock_num  = 481;
    pv2m->vblock_num  = 271;
    // ==> End

    pv2m->top_limit    = phdr->top_limit;
    pv2m->bot_limit    = phdr->bot_limit;
    pv2m->build_date = phdr->date;
    if (phdr->date >= 0x20000000)
    {
        pv2m->build_date = (pv2m->build_date << 8) + 0x10;  // Dummy build time : AM.10
    }

    unsigned int i;
    INX_Block_INFO *pblk;
    MS_BOOL found;

    for (i =0; i < ARRAY_SIZE(phdr->gray_level); i++)
    {
        if (phdr->gray_level[i] == phdr->top_limit)
            break;
        pv2m->layer_levels[i] = phdr->gray_level[i];
    }
    pv2m->layer_count = i;

    // Search the valid data block
    found = FALSE;
    for (i = 0; i < ARRAY_SIZE(phdr->block); i++)
    {
        pblk = &(phdr->block[i]);
        //printf("pblk->type[1] = 0x%x\n", pblk->type[1]);
        if ((pblk->type[1] & 0x0F) != 0x01)
        {
            continue;
        }
        found = TRUE;
        pv2m->data_start    = pblk->data_start;
        pv2m->data_length   = pblk->data_length;
        pv2m->data_checksum = pblk->data_checksum;
    }

    if (found == FALSE)
    {
        for (i = 0; i < ARRAY_SIZE(phdr->extend.block); i++)
        {
            pblk = &(phdr->extend.block[i]);
            if ((pblk->type[1] & 0x0F) != 0x01)
            {
                continue;
            }
            found = TRUE;
            pv2m->data_start    = pblk->data_start;
            pv2m->data_length   = pblk->data_length;
            pv2m->data_checksum = pblk->data_checksum;
        }
    }

    if (found == FALSE)
    {
        UBOOT_ERROR("Can not find Data Block for Mstar\n");
    }
    return found;
}

