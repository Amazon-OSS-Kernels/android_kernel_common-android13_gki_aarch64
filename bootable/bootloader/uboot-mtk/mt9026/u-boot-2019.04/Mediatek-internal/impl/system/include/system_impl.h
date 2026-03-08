/* SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause) */
/*
 * Copyright (c) 2023 MediaTek Inc.
*/

#ifndef _SYSTEM_IMPL_
#define _SYSTEM_IMPL_
#if (CONFIG_MTK_ANDROID_HEADER_VERSION > 2)
#include <android_image.h>
#endif

#define CMD_BUF                             (128)
#define ADD_BOOTARGS_KEY_SIZE               (24)
#define ADD_BOOTARGS_CFG_SIZE               (64)
#define UUID_BUF_SIZE                       (21)
#define EFUSE_BUF_SIZE                      (12)
#define EFUSE_ELEMENT_NUM                   (EFUSE_BUF_SIZE/sizeof(short))
#define RIU_BUS_ADDR                        (0x1C000000UL)
#define EFUSE_BANK_ADDR                     (0x203)
#define MIUP_BANK_ADDR                      (0x1BF4)
#define RANDOM_BANK_ADDR                    (0x29C4)
#define RAND_BANK_OFFSET                    (0x3)
#define TIMESTAMP_BANK_ADDR                 (0x2009)
#define TIMESTAMP_OFFSET                    (0x20)
#define TIMESTAMP_OFFSET_MAX                (0x4F)
#define TIMESTAMP_MASK                      (0xFFFF)
#define TIMESTAMP_SHIFT                     (16)
#define MIUP_OUT_OF_AREA_END0               (0xE)
#define MIUP_OUT_OF_AREA_END1               (0xF)
#define CPU_REGISTER_ACCESS(addr,idx)     *(volatile unsigned short *)(RIU_BUS_ADDR + ((addr<<9) + (idx<<2)))
#define MIUP_OUT_OF_AREA_END(H,L)           ((((H << 16) + L + 1) >> 1) << 13)
#ifdef CONFIG_MULTICORES_PLATFORM
#define MUTLI_CORE_CPU_ZERO                 (0)
#define MUTLI_CORE_CPU_ONE                  (1)
#define MUTLI_CORE_CPU_TWO                  (2)
#define MUTLI_CORE_CPU_THREE                (3)
#endif

typedef enum
{
    //this valuse is offset in chunk header.
    CH_ITEM_FIRST=0x00,
    CH_UBOOT_ROM_START=CH_ITEM_FIRST,
    CH_UBOOT_RAM_START,//0x04
    CH_UBOOT_RAM_END,//0x08
    CH_UBOOT_ROM_END,//0x0c
    CH_UBOOT_RAM_ENTRY,//0x10
    RESERVED1,//0x14,Reserved1
    RESERVED2,//0x18,Reserved2
    CH_BINARY_ID,//0x1c
    CH_LOGO_ROM_OFFSET,//0x20
    CH_LOGO_ROM_SIZE,//0x24
    CH_SBOOT_ROM_OFFSET,//0x28
    CH_SBOOT_SIZE,//0x2c
    CH_SBOOT_RAM_OFFSET,//0x30
    CH_PM_ROM_OFFSET,//0x34
    CH_PM_SIZE,//0x38
    CH_PM_RAM_OFFSET,//0x3c
    CH_SECURITY_INFO_LOADER_ROM_OFFSET,//0x40
    CH_SECURITY_INFO_LOADER_SIZE,//0x44
    CH_SECURITY_INFO_LOADER_RAM_OFFSET,//0x48
    CH_CUSTOMER_KEY_BANK_ROM_OFFSET,//0x4c
    CH_CUSTOMER_KEY_BANK_SIZE,//0x50
    CH_CUSTOMER_KEY_BANK_RAM_OFFSET,//0x54
    CH_SECURITY_INFO_AP_ROM_OFFSET,//0x58
    CH_SECURITY_INFO_AP_SIZE,//0x5C
    CH_UBOOT_ENVIRONMENT_ROM_OFFSET,//0x60
    CH_UBOOT_ENVIRONMENT_SIZE,//0x64
    CH_DDR_BACKUP_TABLE_ROM_OFFSET,//0x68
    CH_POWER_SEQUENCE_TABLE_ROM_OFFSET,//0x6c
    CH_UBOOT_POOL_ROM_OFFSET,//0x70
    CH_UBOOT_POOL_SIZE,//0x74
    CH_NUTTX_CONFIG_OFFSET,//0x78
    CH_NUTTX_CONFIG_SIZE,//0x7c
    CH_RESCURE_ENVIRONMENT_ROM_OFFSET,//0x80
    CH_RESCURE_ENVIRONMENT_SIZE,//0x84
    CH_RESCURE_STATUS_ROM_OFFSET,//0x88
    CH_RESCURE_STATUS_SIZE,//0x8c
    CH_MTK_LOADER_ROM_OFFSET,//0x90
    CH_MTK_LOADER_SIZE,//0x94
    CH_MTK_LOADER_CHAR,//0x98
    CH_AVB_ORANGE_JPG_OFFSET,//0x9C
    CH_AVB_ORANGE_JPG_SIZE,//0xA4
    CH_AVB_RED_JPG_OFFSET,//0xA8
    CH_AVB_RED_JPG_SIZE,//0xAC
    CH_ITEM_LAST=CH_AVB_RED_JPG_SIZE
} EN_CHUNK_HEADER_ITEM;

typedef struct
{
    unsigned int  m_u32MusicVol;                  //MsDrvAudio.c
    unsigned char   m_u8LogoGopIdx;                 //MsDrvGop.c
    unsigned char   m_u8MirrorMode;                 //MsApiGEGOP.c
    unsigned short  m_u16Panel_SwingLevel;          //MsApiPanel.c
    unsigned short  m_u16Panel_ext_type;            //MsApiPanel.c
    char    m_Panel_LVDS_Connect_Type[10];  //MsApiPanel.c
    unsigned short  m_wPanelHTotal;                 //MsApiPanel.c MsDrvPanel.c
    unsigned short  m_wPanelVTotal;
    unsigned short  m_dwPanelframerate;
    unsigned short  m_u16PanelDCLK;
    unsigned int  m_u32ursa_type;                 //Ursa_common.c
    char    m_Ursa_Bin_Name[CMD_BUF];       //CusUrsa8Init.c
    unsigned char   m_u8PixelShiftEnable;           //MsDrvPanel.c
    unsigned char   m_u8MOD_H_MirrorMode;           //MsApiGEGOP.c & MsDrvPanel.c
    unsigned char   m_u8VideoMirrorMode;            //MsApiGEGOP.c
    unsigned short  m_u16OsdWidth; //panel ini, OSD resolution for DFB in Ursa video/osd different lane case
    unsigned short  m_u16OsdHeight; //panel ini, OSD resolution for DFB in Ursa video/osd different lane case
    unsigned int  m_wPanelOnTiming3;                 //MsApiPanel.c
    bool  m_OSDCenableFlag; //OSD t-ten clock enable in Ursa video/osd different lane case
    unsigned int  m_onrf_op;
}st_sys_misc_setting;

#if (CONFIG_MTK_ANDROID_HEADER_VERSION > 2)
struct mtk_android_header_data {
	unsigned int header_version;
	unsigned long vendor_img_addr;
	struct vendor_boot_img_hdr vendor_hdr;
#if (CONFIG_MTK_ANDROID_HEADER_VERSION > 3)
	unsigned long init_boot_img_addr;
#endif
};

int mtk_get_vendor_img_hdr_info_u32(char *type, unsigned int *data);
int mtk_get_vendor_img_hdr_info_u64(char *type, unsigned long *data64);
int mtk_get_vendor_img_hdr_info_str(char *type, uint8_t *buf);
#endif

unsigned long long phyical_addr_to_virtual_addr(unsigned long long phy_addr);
unsigned long long virtual_addr_to_phyical_addr(unsigned long long virt_addr);
unsigned long long get_dram_size(void);
int mtk_driver_init(void);

#endif
