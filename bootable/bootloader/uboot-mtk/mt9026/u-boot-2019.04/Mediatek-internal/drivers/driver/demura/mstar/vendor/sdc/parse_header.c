#include <command.h>
#include <common.h>
#include <malloc.h>
#include <stdio.h>
#include <linux/string.h>
#include <asm/byteorder.h>
#include <MsTypes.h>
#include <debug_impl.h>
#include <dmalloc.h>
#include <ms_utils.h>
#include <demura.h>
//#include <convert_entry.h>

#include "vendor.h"
#include "parse_header.h"


MS_U16 get_lut_checksum(void)
{
    MS_U8   buffer[2];
    MS_BOOL bRet;
    MS_U16  checksum = 0xFFFF;

    if (init_spi_flash() != TRUE)
    {
        UBOOT_DEBUG("init_spi_flash error!\n");
        return checksum;
    }

    bRet = read_spi_flash(buffer, DMC_LUT_CHECKSUM, sizeof(buffer));
    if (bRet == FALSE)
    {
        UBOOT_ERROR("Read CHOT_Header from spi flash failed !\n");
        return checksum;
    }

    checksum = ((MS_U16)buffer[1] << 8) | buffer[0];
    return checksum;
}

MS_BOOL load_vendor_header(SDC_Demura_Header *phdr)
{
    MS_U8 read_len = HEADER_LEN + 1; // 1 bytes for checksum
    MS_U8 *buffer = (MS_U8 *)malloc(read_len);
    if (buffer == NULL)
    {
        printf("In %s, malloc 0x%x byte failed!\n", __FUNCTION__, read_len);
        return FALSE;
    }

    MS_BOOL bRet = read_spi_flash(buffer, HEADER_START, read_len);
    if (bRet == FALSE)
    {
        UBOOT_ERROR("Read CHOT Header from spi flash failed !\n");
        free(buffer);
        return FALSE;
    }

    phdr->hdr_buffer = buffer;
    return TRUE;
}


MS_BOOL parse_vendor_header(SDC_Demura_Header *phdr)
{
    MS_U8  *buffer        = phdr->hdr_buffer;
    MS_U8 header_checksum = 0, bit_control;
    MS_U32 checksum = 0, i = 0;

    header_checksum = buffer[DMC_HEADER_CHECKSUM - HEADER_START];
    phdr->PARAMETER_CRC = header_checksum;
    for(i = 0; i < HEADER_LEN; i++)
    {
        checksum += buffer[i];
    }
    checksum = 0xFC - (checksum & 0xFF);
    printf("header_checksum = 0x%x, calculate checksum = 0x%x \n", header_checksum, checksum);

    if(checksum != header_checksum)
    {
        printf("verify dmc header checksum failed !\n");
        free(buffer);
        phdr->hdr_buffer = NULL;
        return FALSE;
    }

    // Bit control #1
    bit_control = ((buffer[0] & 0x01)<<7) | ((buffer[1] & 0xFE)>>1);
    phdr->Value_1[6] = (bit_control & (_BIT7 | _BIT6))>>6; // [7:6]
    phdr->Value_1[5] = (bit_control & (_BIT5 | _BIT4))>>4; // [5:4]
    phdr->Value_1[4] = (bit_control & (_BIT3 | _BIT2))>>2; // [3:2]
    phdr->Value_1[3] = (bit_control & (_BIT1 | _BIT0)); // [1:0]

    // Bit control #2
    bit_control = ((buffer[1] & 0x01)<<7) | ((buffer[2] & 0xFE)>>1);
    phdr->Value_1[2] = (bit_control & (_BIT7 | _BIT6))>>6; // [7:6]
    phdr->Value_1[1] = (bit_control & (_BIT5 | _BIT4))>>4; // [5:4]
    phdr->Value_1[0] = (bit_control & (_BIT3 | _BIT2))>>2; // [3:2]


    MS_U16 get_plane[MAX_PLANE_NUM];
    for(i = 0; i < MAX_PLANE_NUM; i++)
    {
        get_plane[i] = ((buffer[2+i] & 0x01)<<7) | ((buffer[3+i] & 0xFE)>>1);
    }

    for(i = 0; i < MAX_PLANE_NUM; i++)
    {
        if(get_plane[i] > 0)
            phdr->PLANE[i] = get_plane[i];
        else
            phdr->PLANE[i] = 0;

        printf("Value_1[%d] = %d, PLANE[%d] = %d ", i, phdr->Value_1[i],i, phdr->PLANE[i]);
        printf("\n");
    }

    return TRUE;
}

MS_BOOL load_vendor_lut(SDC_Demura_Header *phdr)
{
    MS_U32 lut_size   = LUT_LEN + 1; // LUT checksum
    MS_U8 lut_checksum = 0;
    MS_U32 checksum = 0, i = 0;
    MS_U8  *lut_buf   = dmalloc(lut_size);
    CHECK_DMALLOC_SPACE(lut_buf, lut_size);

    MS_BOOL bRet = read_spi_flash(lut_buf, LUT_START, lut_size);
    if (bRet == FALSE)
    {
        UBOOT_ERROR("Read CHOT Lut from spi flash failed !\n");
        dfree(lut_buf);
        return FALSE;
    }

    lut_checksum   = lut_buf[LUT_LEN];
    phdr->TABLE_CRC = lut_checksum;

    for(i = 0; i < LUT_LEN; i++)
    {
        checksum += lut_buf[i];
    }
    checksum = 0xFC - (checksum & 0xFF);
    printf("lut_checksum = 0x%x, calculate checksum = 0x%x \n", lut_checksum, checksum);

    if (lut_checksum != checksum)
    {
        printf("In %s, verify lut checksum failed !\n", __FUNCTION__);
        dfree(lut_buf);
        return FALSE;
    }

    phdr->lut_buffer = lut_buf;
    phdr->lut_offset = 0;
    phdr->lut_length = lut_size;
    return TRUE;
}

