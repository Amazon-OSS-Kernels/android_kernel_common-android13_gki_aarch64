// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

#include <common.h>
#include <mtk_mmu.h>
//#include <drvSYS.h>
#include <debug_impl.h>
#include <system_impl.h>
#include <utility.h>
#if (CONFIG_MTK_ANDROID_HEADER_VERSION > 2)
#include <android_image.h>
#endif

#if defined(CONFIG_TARGET_MT5888)
static bool is_mtk_drv_init = FALSE;
#endif

int mtk_driver_init(void)
{
#if defined(CONFIG_TARGET_MT5888)
    if(is_mtk_drv_init == FALSE)
    {
        MsOS_Init();
        MDrv_SYS_Init();
        is_mtk_drv_init = TRUE;
    }
    else
    {
        return -1;
    }
#endif
    return 0;
}

unsigned long long get_dram_size(void)
{
    unsigned long long high_addr = 0;
    unsigned long long low_addr = 0;
    unsigned long long ret = 0;

    high_addr = (unsigned long long)CPU_REGISTER_ACCESS(MIUP_BANK_ADDR, MIUP_OUT_OF_AREA_END1);
    low_addr = (unsigned long long)CPU_REGISTER_ACCESS(MIUP_BANK_ADDR, MIUP_OUT_OF_AREA_END0);

    ret = MIUP_OUT_OF_AREA_END(high_addr, low_addr);

    return ret;
}

int get_device_unique_id(unsigned char *buf, unsigned int size)
{
    unsigned short *efuse_did;
    unsigned char *ptr;
#if defined(CONFIG_TARGET_MT5888)
    bool ret;
#endif
    unsigned short sum = 0;
    int i;

    if(size < EFUSE_BUF_SIZE)
    {
        UBOOT_ERROR("Input size %d < %d\n",size,EFUSE_BUF_SIZE);
        return -1;
    }

    efuse_did = (unsigned short *)malloc(EFUSE_BUF_SIZE);
    if(efuse_did == NULL)
    {
        UBOOT_ERROR("Allocate efuse_did memory buffer failure.\n");
        return -1;
    }
    memset(efuse_did, 0, EFUSE_BUF_SIZE);
#if defined(CONFIG_TARGET_MT5888)
    ret = MDrv_SYS_GetEfuseDid(efuse_did);
    if(ret == FALSE)
    {
        free(efuse_did);
        return -1;
    }
#else
    for(i=0;i < EFUSE_ELEMENT_NUM; i++)
    {
        efuse_did[i] = CPU_REGISTER_ACCESS(EFUSE_BANK_ADDR,i);
        sum = sum + efuse_did[i];
    }
    if(sum == 0)
    {
        UBOOT_ERROR("Get EFUSE number from register failure.\n");
        free(efuse_did);
        return -1;
    }

#endif
    ptr = (unsigned char *)efuse_did;
    memcpy(buf, ptr, EFUSE_BUF_SIZE);
    free(efuse_did);

    return 0;
}

unsigned long long phyical_addr_to_virtual_addr(unsigned long long phy_addr)
{
    unsigned long long dram_size = 0;
    dram_size = get_dram_size();
    if(dram_size <= 0)
        return ~0;

    if(phy_addr > dram_size)
        return ~0;

    return (unsigned long long)(phy_addr + MIU0_BUS_BASE);
}

unsigned long long virtual_addr_to_phyical_addr(unsigned long long virt_addr)
{
    unsigned long long dram_size = 0;
    dram_size = get_dram_size();
    if(dram_size <= 0)
        return ~0;

    if(virt_addr < MIU0_BUS_BASE || virt_addr > (MIU0_BUS_BASE + dram_size))
        return ~0;

    return (unsigned long long)(virt_addr - MIU0_BUS_BASE);
}

#if (CONFIG_MTK_ANDROID_HEADER_VERSION > 2)
struct mtk_android_header_data mtk_an_hdr = {0};
int mtk_get_vendor_img_hdr_info_u32(char *type, unsigned int *data)
{
    switch(type[0])
    {
        case 'h':
            *data = mtk_an_hdr.header_version;
            break;
        case 'p':
            *data = mtk_an_hdr.vendor_hdr.page_size;
            break;
        case 'k':
            *data = mtk_an_hdr.vendor_hdr.kernel_addr;
            break;
        case 'r':
            *data = mtk_an_hdr.vendor_hdr.ramdisk_addr;
            break;
        case 'v':
            *data = mtk_an_hdr.vendor_hdr.vendor_ramdisk_size;
            break;
        case 'd':
            *data = mtk_an_hdr.vendor_hdr.dtb_size;
            break;
        default:
            printf("Header data field doesn't exist\n");
            return -1;
    }
    return 0;
}

int mtk_get_vendor_img_hdr_info_u64(char *type, unsigned long *data64)
{
    switch(type[0])
    {
        case 'a':
            *data64 = mtk_an_hdr.vendor_img_addr;
            break;
        case 'd':
            *data64 = mtk_an_hdr.vendor_hdr.dtb_addr;
            break;
#if (CONFIG_MTK_ANDROID_HEADER_VERSION > 3)
        case 'i':
            *data64 = mtk_an_hdr.init_boot_img_addr;
            break;
#endif
        default:
            printf("Header data field doesn't exist\n");
            return -1;
    }
    return 0;
}

int mtk_get_vendor_img_hdr_info_str(char *type, uint8_t *buf)
{
    switch(type[0])
    {
        case 'c':
            memcpy(buf, mtk_an_hdr.vendor_hdr.cmdline, strlen((const char *)mtk_an_hdr.vendor_hdr.cmdline));
            break;
        default:
            printf("Header data field doesn't exist\n");
            return -1;
    }
    return 0;
}
#endif