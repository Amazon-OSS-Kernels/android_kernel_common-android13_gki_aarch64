// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

#include <common.h>
#include <command.h>
#include <time.h>
#include <fs.h>
#include <dm/uclass.h>
#include <mtk_mmu.h>
#include <MsCommon.h>
#include <debug_impl.h>
#include <jpd_impl.h>
#include <panel_impl.h>
#include <system_impl.h>
#include <dts_parser.h>
#include <utility.h>
#include <mtk_jpd.h>
#include <apiGFX.h>

//--------------------------------------------------------------------------------------------------
// Local Variables
//--------------------------------------------------------------------------------------------------
static void *read_buf_virtual_addr = NULL;
static void *inter_buf_virtual_addr = NULL;
static void *out_buf_virtual_addr = NULL;
static dma_addr_t read_buf_pa = 0;
static dma_addr_t out_buf_pa = 0;
static dma_addr_t inter_buf_pa = 0;
static size_t read_buf_size = 0;
static size_t out_buf_size = 0;
static size_t inter_buf_size = 0;
static const char *logo_file_path = NULL;
static loff_t read_position = 0;
static bool eof_flag = FALSE;
static unsigned char *file_buf = NULL;
static loff_t logo_size = 0;
static JPEG_Alignment alignment = {0};

static EN_JPEG_Result fill_jpg_data(JPEG_Buf buf, size_t *filled_length, bool *eof)
{
    ulong filled_buffer_addr = 0;
    loff_t filled_buffer_size = 0;
    loff_t bytes_read = 0;

    UBOOT_TRACE("IN\n");

    if (filled_length == NULL || eof == NULL)
    {
        UBOOT_ERROR("Invalid filled_length or eof\n");
        return E_JPEG_FAILED;
    }

    *filled_length = (size_t)bytes_read;
    *eof = eof_flag;
    if (eof_flag)
    {
        UBOOT_DEBUG("Logo file is already filled to the end\n");
        return E_JPEG_OKAY;
    }

    filled_buffer_addr = (ulong)buf.va;
    filled_buffer_size = (loff_t)buf.size;
    UBOOT_DEBUG("filled_buffer_addr=0x%lx, filled_buffer_size:0x%lx\n" , filled_buffer_addr, (unsigned long)filled_buffer_size);
    if (filled_buffer_addr == 0 || filled_buffer_size == 0)
    {
        UBOOT_ERROR("Invalid filled_buffer_addr or filled_buffer_size\n");
        return E_JPEG_FAILED;
    }

    if (read_position < logo_size)
    {
        if ((read_position + filled_buffer_size) >= logo_size)
        {
            bytes_read = logo_size - read_position;
            eof_flag = TRUE;
        }
        else
        {
            bytes_read = filled_buffer_size;
        }

        if(memcpy((char *)filled_buffer_addr, (char *)(file_buf + read_position), bytes_read) == NULL)
        {
            UBOOT_ERROR("Copy data form %p failure\n",file_buf);
            return E_JPEG_FAILED;
        }
    }
    else
    {
        eof_flag = TRUE;
    }

    read_position += bytes_read;
    *filled_length = bytes_read;
    *eof = eof_flag;
    UBOOT_DEBUG("Read %s file success. filled_length=%lu, eof=%u\n", logo_file_path, (unsigned long)*filled_length, *eof);

    UBOOT_TRACE("OUT\n");
    return E_JPEG_OKAY;
}

static int jpd_init(void)
{
    int ret;
    jpd_setting_para jpd_para = {0};
    struct dts_mmap mmap;
    UBOOT_TRACE("IN\n");

    ret = parse_dt("/mtk_njpd",jpd_dt_parser,(void*)&jpd_para,NULL);
    if(ret != 0)
    {
        UBOOT_ERROR("Get njpd setting from dts failure\n");
        return -1;
    }

    memset(&mmap, 0, sizeof(mmap));
    ret = parse_dt("/mmap_info/MI_JPD_READ_BUF",mmap_dt_parser,(void*)&mmap,"reg");
    if(ret < 0 || mmap.address == 0)
    {
        UBOOT_ERROR("Error: Get JPD read buffer from DTS mmap failure, use default buffer\n");
        if (jpd_para.read_buffer < MIU0_BUS_BASE)
        {
            UBOOT_ERROR("read_buf:%x is small than %lx\n", jpd_para.read_buffer, MIU0_BUS_BASE);
            return -1;
        }
        read_buf_pa = virtual_addr_to_phyical_addr((unsigned long long)jpd_para.read_buffer);
        read_buf_size = jpd_para.read_size;
    }
    else
    {
        if (mmap.address < MIU0_BUS_BASE)
        {
            UBOOT_ERROR("read_buf:%llx is small than %lx\n", mmap.address, MIU0_BUS_BASE);
            return -1;
        }
        read_buf_pa = virtual_addr_to_phyical_addr(mmap.address);
        read_buf_size = mmap.size;
    }

    memset(&mmap, 0, sizeof(mmap));
    ret = parse_dt("/mmap_info/MI_JPD_INTER_BUF",mmap_dt_parser,(void*)&mmap,"reg");
    if(ret < 0 || mmap.address == 0)
    {
        UBOOT_ERROR("Error: Get JPD inter buffer from DTS mmap failure, use default buffer\n");
        if (jpd_para.inter_buffer < MIU0_BUS_BASE)
        {
            UBOOT_ERROR("inter_buf:%x is small than %lx\n", jpd_para.inter_buffer, MIU0_BUS_BASE);
            return -1;
        }
        inter_buf_pa = virtual_addr_to_phyical_addr((unsigned long long)jpd_para.inter_buffer);
        inter_buf_size = jpd_para.inter_size;
    }
    else
    {
        if (mmap.address < MIU0_BUS_BASE)
        {
            UBOOT_ERROR("inter_buf:%llx is small than %lx\n", mmap.address, MIU0_BUS_BASE);
            return -1;
        }
        inter_buf_pa = virtual_addr_to_phyical_addr(mmap.address);
        inter_buf_size = mmap.size;
    }

    memset(&mmap, 0, sizeof(mmap));
    ret = parse_dt("/mmap_info/MI_JPD_OUT",mmap_dt_parser,(void*)&mmap,"reg");
    if(ret < 0 || mmap.address == 0)
    {
        UBOOT_ERROR("Error: Get JPD out buffer from DTS mmap failure, use default buffer\n");
        if (jpd_para.output_buffer < MIU0_BUS_BASE)
        {
            UBOOT_ERROR("out_buf:%x is small than %lx\n", jpd_para.output_buffer, MIU0_BUS_BASE);
            return -1;
        }
        out_buf_pa = virtual_addr_to_phyical_addr((unsigned long long)jpd_para.output_buffer);
        out_buf_size = jpd_para.output_size;
    }
    else
    {
        if (mmap.address < MIU0_BUS_BASE)
        {
            UBOOT_ERROR("out_buf:%llx is small than %lx\n", mmap.address, MIU0_BUS_BASE);
            return -1;
        }
        out_buf_pa = virtual_addr_to_phyical_addr(mmap.address);
        out_buf_size = mmap.size;
    }
    read_position = 0;
    eof_flag = FALSE;

    UBOOT_DEBUG("read_buf_pa = 0x%lx, out_buf_pa = 0x%lx, inter_buf_pa = 0x%lx\n",(unsigned long)read_buf_pa, (unsigned long)out_buf_pa, (unsigned long)inter_buf_pa);
    UBOOT_DEBUG("read_buf_size = 0x%lx, out_buf_size = 0x%lx, inter_buf_size = 0x%lx\n",(unsigned long)read_buf_size, (unsigned long)out_buf_size, (unsigned long)inter_buf_size);

    read_buf_virtual_addr = (void *)phyical_addr_to_virtual_addr(read_buf_pa);
    out_buf_virtual_addr = (void *)phyical_addr_to_virtual_addr(out_buf_pa);
    inter_buf_virtual_addr = (void *)phyical_addr_to_virtual_addr(inter_buf_pa);
    UBOOT_DEBUG("read_buf_virtual_addr = 0x%p, out_buf_virtual_addr = 0x%p, inter_buf_virtual_addr = 0x%p\n",read_buf_virtual_addr, out_buf_virtual_addr, inter_buf_virtual_addr);

    UBOOT_TRACE("OUT\n");
    return 0;
}

unsigned int mtk_jpd_get_alignment_width(void)
{
    UBOOT_DEBUG("Get alignment width=%u\n", alignment.width);
    return (unsigned int)alignment.width;
}

unsigned int mtk_jpd_get_alignment_height(void)
{
    UBOOT_DEBUG("Get alignment height=%u\n", alignment.height);
    return (unsigned int)alignment.height;
}

unsigned int mtk_jpd_get_alignment_pitch(void)
{
    UBOOT_DEBUG("Get alignment pitch=%u\n", alignment.pitch);
    return (unsigned int)alignment.pitch;
}

unsigned int mtk_jpd_get_color_format(void)
{
    UBOOT_DEBUG("Get color format=%u\n", GFX_FMT_YUV422);
    return (unsigned int)GFX_FMT_YUV422;
}

unsigned long mtk_jpd_get_output_buffer_addr(void)
{
    UBOOT_DEBUG("out_buf_pa=0x%lx\n", (unsigned long)out_buf_pa);
    return (unsigned long)out_buf_pa;
}

unsigned long mtk_jpd_get_output_buffer_size(void)
{
    UBOOT_DEBUG("out_buf_size=0x%lx\n", (unsigned long)out_buf_size);
    return (unsigned long)out_buf_size;
}

bool mtk_jpd_decode(char *part, const char *path)
{
    struct udevice *dev;
    int ret;
    bool eof;
    EN_JPEG_Result jpeg_ret;
    JPEG_Buf read_buf, write_buf, inter_buf;
    JPEG_Pic max_resolution;
    JPEG_FillDataFunc pFillDataFunc = fill_jpg_data;

    UBOOT_TRACE("IN\n");
    logo_file_path = path;
    max_resolution.width = (unsigned short)mtk_get_current_panel_width();
    max_resolution.height = (unsigned short)mtk_get_current_panel_height();
    if(max_resolution.width == 0 || max_resolution.height == 0)
    {
        UBOOT_ERROR("Get Panel width and height failure\n");
        return FALSE;
    }

    if (jpd_init() != 0)
    {
        UBOOT_ERROR("jpd_init failed\n");
        return FALSE;
    }

    if(read_buf_pa==0x0 || out_buf_pa==0x0 ||inter_buf_pa==0x0)
    {
        UBOOT_ERROR("Get JPD Mmap Fail Skip JPD Decode!!!\n");
        return FALSE;
    }

    //Preload data for init JPEG decoder data.
    read_buf = (JPEG_Buf){.va = read_buf_virtual_addr, .pa = read_buf_pa, .filled_length = 0, .size = read_buf_size};
    write_buf = (JPEG_Buf){.va = out_buf_virtual_addr, .pa = out_buf_pa, .filled_length = 0, .size = out_buf_size};
    inter_buf = (JPEG_Buf){.va = inter_buf_virtual_addr, .pa = inter_buf_pa, .filled_length = 0, .size = inter_buf_size};
    file_buf = read_storage_file_to_memory(part, logo_file_path,&logo_size);
    if(file_buf == NULL || logo_size == 0)
    {
        UBOOT_ERROR("Read logo file to memory failure\n");
        return FALSE;
    }

    jpeg_ret = fill_jpg_data(read_buf, &read_buf.filled_length, &eof);
    if (jpeg_ret == E_JPEG_FAILED)
    {
        UBOOT_ERROR("Initial read file error!!\n");
        return FALSE;
    }

    ret = uclass_get_device_by_name(UCLASS_NJPD, "mtk_njpd", &dev);
    if (!ret)
    {
        max_resolution.width = 0;
        max_resolution.height = 0;
        jpeg_ret = njpd_start_decode(dev, read_buf, write_buf, inter_buf, max_resolution, pFillDataFunc);
        if (jpeg_ret == E_JPEG_FAILED)
        {
            UBOOT_ERROR("JPEG start decode failed!!\n");
            return FALSE;
        }

        jpeg_ret = njpd_get_alignment(dev, &alignment);
        if (jpeg_ret == E_JPEG_FAILED)
        {
            UBOOT_ERROR("JPEG get alignment failed!!\n");
            return FALSE;
        }

        while (1)
        {
            EN_JPEG_DecodeState jpeg_decode_state = njpd_wait_decode_done(dev);
            UBOOT_TRACE("jpeg_decode_state = %d\n", jpeg_decode_state);
            if (jpeg_decode_state == E_JPEG_DEC_FAILED)
            {
                UBOOT_ERROR("JPEG decode failed!!\n");
                break;
            }
            else if (jpeg_decode_state == E_JPEG_DEC_DONE)
            {
                UBOOT_DEBUG("JPEG decode done\n");
                break;
            }
            else if (jpeg_decode_state == E_JPEG_DEC_PARTIAL_DONE)
            {
                UBOOT_TRACE("JPEG decode partial done\n");
            }
        }

        jpeg_ret = njpd_stop_decode(dev);
        if (jpeg_ret == E_JPEG_FAILED)
        {
            UBOOT_ERROR("JPEG stop decode failed!!\n");
            return FALSE;
        }
    }
    else
    {
        UBOOT_ERROR("Get njpd device failure, skip decode\n");
        return FALSE;
    }

    UBOOT_TRACE("OUT\n");
    return TRUE;
}