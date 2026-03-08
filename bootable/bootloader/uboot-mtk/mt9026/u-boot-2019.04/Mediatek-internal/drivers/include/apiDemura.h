// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

#ifndef _MS_API_DEMURA_H_
#define _MS_API_DEMURA_H_

#include "apiPNL.h"
//#include "mtk_demura.h"
#include <system_impl.h>

#define DEMURA_BDM_STRING         "DMR"
#define DEMURA_BUFFER_DESC_SIZE   0x400  // 1024 Byte
#define DEMURA_BIN_ALIGN_UNIT     0x400
#define DEMURA_BIN_MAX_NUM        10
#define DEMURA_MSTAR_IP           "mstar demura"
#define DEMURA_FCIC_IP            "fcic demura"

#define check_str_resume() 0
#define BA2PA(addr) (virtual_addr_to_phyical_addr(addr))
#define PA2BA(addr) (phyical_addr_to_virtual_addr(addr))


typedef struct
{
    char    ip_string[16];
    MS_U64  phy_addr;
    MS_U64  virt_addr;
    MS_U32  length;
    MS_U32  len_align;
    MS_U32  id_num;
    MS_U32  reserve[5];
} demura_bin_desc;             // 64 Byte


typedef struct
{
    MS_U8   magic_str[4];      // "DMR"
    MS_U32  bin_cur_num;
    MS_U64  mmap_phy_addr;
    MS_U64  mmap_virt_addr;    // 32 Byte
    MS_U32  mmap_length;
    MS_U32  next_bin_addr;     // 36 Byte
    MS_U32  left_space;        // 40 Byte
    MS_U32  reserve[14];       // 96 Byte

    // Always be the last item, offset --> 96
    demura_bin_desc bdesc[DEMURA_BIN_MAX_NUM];
} demura_buf_desc;

typedef enum
{
    E_DEMURA_MULTI_NOT  = 0,
    E_DEMURA_MULTI_AUO,                 //1
    E_DEMURA_MULTI_LGD_START,           //2
    E_DEMURA_MULTI_LGD_V18,             //3
    E_DEMURA_MULTI_LGD_SQ18_19_MONO,    //4
    E_DEMURA_MULTI_LGD_SQ18_19_COLOR,   //5
    E_DEMURA_MULTI_LGD_V19_MONO,        //6
    E_DEMURA_MULTI_LGD_V19_COLOR,       //7
    E_DEMURA_MULTI_LGD_END,             //8
    E_DEMURA_MULTI_NOVA,                //9
    E_DEMURA_MULTI_CSOT_HISILICON,      //10
    E_DEMURA_MULTI_CSOT_HIMAX,          //11
    E_DEMURA_MULTI_SDC,                 //12
    E_DEMURA_MULTI_INX,                 //13
    E_DEMURA_MULTI_CSOT_CSOT,           //14
    E_DEMURA_MULTI_HKC_NOVA,            //15
    E_DEMURA_MULTI_MTK,                 //16
    E_DEMURA_MULTI_BOE_ESWIN,           //17
    E_DEMURA_MULTI_H_K_C_NOVA_120HZ,    //18
    E_DEMURA_MULTI_SIO,                 //19
    E_DEMURA_MULTI_MAX                  //20
}Demura_Panel_Vendor;

typedef struct
{
    unsigned short u16PanelWidth;
    unsigned short u16PanelHeight;
    MS_BOOL bOn;
}Demura_Panel_Data;

typedef enum
{
    EN_DEMURA_MULTI_NOT  = 0,
    EN_DEMURA_MULTI_AUO,
    EN_DEMURA_MULTI_LGD_START,
    EN_DEMURA_MULTI_LGD_V18,
    EN_DEMURA_MULTI_LGD_SQ18_19_MONO,
    EN_DEMURA_MULTI_LGD_SQ18_19_COLOR,
    EN_DEMURA_MULTI_LGD_V19_MONO,
    EN_DEMURA_MULTI_LGD_V19_COLOR,
    EN_DEMURA_MULTI_LGD_END,
    EN_DEMURA_MULTI_NOVA,
    EN_DEMURA_MULTI_CSOT_HI_SILICON,
    EN_DEMURA_MULTI_CSOT_HIMAX,
    EN_DEMURA_MULTI_SDC,
    EN_DEMURA_MULTI_INX,
    EN_DEMURA_MULTI_CSOT_CSOT,
    EN_DEMURA_MULTI_HKC_NOVA,
    EN_DEMURA_MULTI_MTK,
    EN_DEMURA_MULTI_BOE_ESWIN,
    EN_DEMURA_MULTI_H_K_C_NOVA_120HZ,
    EN_DEMURA_MULTI_SIO, /* Add for test */
    EN_DEMURA_MULTI_MAX
}EN_DEMURA_MULTI_VENDOR;

typedef enum {
    E_DEMURA_API_BIN_TYPE_MAIN,
    E_DEMURA_API_BIN_TYPE_DLG,
    E_DEMURA_API_BIN_TYPE_MAX
}EN_DEMURA_API_BIN_TYPE;

typedef enum
{
    E_DEMURA_API_BIN_ACT_OFF,
    E_DEMURA_API_BIN_ACT_FIRST = E_DEMURA_API_BIN_ACT_OFF,
    E_DEMURA_API_BIN_ACT_ON,
    E_DEMURA_API_BIN_ACT_LAST = E_DEMURA_API_BIN_ACT_ON,
    E_DEMURA_API_BIN_ACT_NO_DLG,
    E_DEMURA_API_BIN_ACT_MAX
}EN_DEMURA_API_BIN_ACT;

MS_BOOL MApi_Demura_Bypass(MS_BOOL bOnOff);

#if defined(CONFIG_DEMURA_VENDOR_MULTI)
MS_BOOL MApi_Demura_Init(Demura_Panel_Data panel_data, EN_DEMURA_MULTI_VENDOR multi_vendor);
#else
MS_BOOL MApi_Demura_Init(Demura_Panel_Data panel_data);
#endif

MS_U32 load_file_to_dram(char *path, MS_U64 *dram_addr);
MS_BOOL push_demura_bin(demura_bin_desc *pbin);
MS_U64  add_dbin_buf(const char *ip_string, MS_U32 filesize, MS_U32 id_num);
void    pop_demura_bin(void);
MS_U32  get_dbuf_length(void);

demura_bin_desc *get_demura_bin(char *ip_string, MS_U32 id_num);

void MApi_MsDemura_SetVersion(int version);
void MApi_MsDemura_SetPnlInfo(MS_U16 width, MS_U16 height);
MS_BOOL MApi_MsDemura_Enable(MS_BOOL bOnOff);
MS_BOOL MApi_MsDemura_BYPASS(MS_BOOL bOnOff);
void MApi_MsDemura_SetBinType(MS_U8 u8type); /* EN_DEMURA_API_BIN_TYPE */
void MApi_MsDemura_SetAct(MS_U8 act); /* EN_DEMURA_API_BIN_ACT */

void MApi_MsDemura_Setfile(MS_U8 u8file);
MS_U8 MApi_MsDemura_Getfile(void);
#endif
