// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

#include <common.h>
#include <command.h>
#include <time.h>
#include <fs.h>
#include <mtk_mmu.h>
#include <MsCommon.h>
#include <debug_impl.h>
#include <system_impl.h>
#include <dts_parser.h>
#include <utility.h>
#include <mtk_rawdata.h>
#include <jpd_impl.h>

//--------------------------------------------------------------------------------------------------
// Local Variables
//--------------------------------------------------------------------------------------------------
static size_t out_buf_pa = 0;
static unsigned int out_buf_size = 0;

unsigned int mtk_raw_data_get_output_buffer_addr(void)
{
    return (unsigned int)out_buf_pa;
}

unsigned int mtk_raw_data_get_output_buffer_size(void)
{
    return (unsigned int)out_buf_size;
}

int mtk_raw_data_load_logo_to_dram(char *partition, const char *path)
{
    loff_t logo_len = 0;
    unsigned char *file_buf;
    unsigned char *logo_file_addr;
    struct dts_mmap mmap;
    jpd_setting_para jpd_para;
    int ret;
    UBOOT_TRACE("IN\n");

    file_buf = read_storage_file_to_memory(partition,path,&logo_len);
    if(file_buf == NULL)
    {
        UBOOT_ERROR("Error: Read raw data file failure\n");
        return -1;
    }

    ret = parse_dt("/mtk_njpd",jpd_dt_parser,(void*)&jpd_para,NULL);
    if(ret != 0)
    {
        UBOOT_ERROR("Get njpd setting from dts failure\n");
        free(file_buf);
        return -1;
    }

    memset(&mmap, 0, sizeof(mmap));
    ret = parse_dt("/mmap_info/MI_JPD_OUT",mmap_dt_parser,(void*)&mmap,"reg");
    if(ret < 0 || mmap.address == 0)
    {
        UBOOT_ERROR("Error: Get JPD out buffer from DTS mmap failure, use default buffer\n");
        if (jpd_para.output_buffer < MIU0_BUS_BASE)
        {
            UBOOT_ERROR("out_buf:%x is small than %lx\n", jpd_para.output_buffer, MIU0_BUS_BASE);
            free(file_buf);
            return -1;
        }
        out_buf_pa = virtual_addr_to_phyical_addr(jpd_para.output_buffer);
        out_buf_size = jpd_para.output_size;
    }
    else
    {
        if (mmap.address < MIU0_BUS_BASE)
        {
            UBOOT_ERROR("out_buf:%llx is small than %lx\n", mmap.address, MIU0_BUS_BASE);
            free(file_buf);
            return -1;
        }
        out_buf_pa = virtual_addr_to_phyical_addr(mmap.address);
        out_buf_size = mmap.size;
    }

    logo_file_addr = (unsigned char *)phyical_addr_to_virtual_addr(out_buf_pa);
    memcpy(logo_file_addr, file_buf, (unsigned long)logo_len);
    free(file_buf);

    UBOOT_DEBUG("%s file, put on %p, size=0x%lX\n",path,logo_file_addr,(unsigned long)logo_len);
    UBOOT_TRACE("OK\n");
    return 0;
}
