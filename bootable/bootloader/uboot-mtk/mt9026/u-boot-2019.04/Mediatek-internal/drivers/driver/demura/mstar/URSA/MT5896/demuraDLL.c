// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

#include "utopia.h"
#include <stdio.h>
#include <stdlib.h>
#include <common.h>
#include <MsTypes.h>
#include <linux/string.h>
#include <CommonDataType.h>
#include <crc_libs.h>
#include <ms_vector.h>
#include <dmalloc.h>
#include <demura.h>
#include <demura_common.h>
#include <demuraDll.h>

#include <apiPNL.h>
#include <MDrvDemura.h>
#include <halDemura.h>
#include "string.h"
#include "halRegOp.h"

#define DEMURA_DLL_DUMP_LOG (0)

#ifdef MSOS_TYPE_LINUX_KERNEL
#define mst_atoi(str) simple_strtoul(((str != NULL) ? str : ""), NULL, 0);
#else
#define mst_atoi(str) strtoul(((str != NULL) ? str : ""), NULL, 0);
#endif


#define TEST_INTERFACE 1


unsigned char *bin_buf;
unsigned char *bin_cur_pos;
MstarSubBinHeader m_MstarSubHeader; // DMC
DemuraHeader m_Header;
ST_DEMURA_UFC_HEADER m_DemuraUfcBinHeader;
MS_U16 g_u16PanelWidth = 0;
MS_U16 g_u16PanelHeight = 0;

//---------------------------------------------------------------
MS_U32 StrToHex(const char *psHex, int startIndex)
{
  MS_U32 dwHex = 0;

  if (startIndex == 0)
  {
    if (psHex[0] == '0' && (psHex[1] == 'x' || psHex[1] == 'X'))
    {
      return mst_atoi(psHex);
    }
  }

    int count=0;

  while (*psHex != '\0')
  {
    char c = *psHex;

    if (c >= '0' && c <= '9')
    {
      c -= '0';
    }
    else if (c >= 'A' && c <= 'F')
    {
      c = c - 'A' + 10;
    }
    else if (c >= 'a' && c <= 'f')
    {
      c = c - 'a' + 10;
    }

    dwHex = dwHex * 16 + c;
    psHex++;
    count++;
  }

  return dwHex;
}

//---------------------------------------------------------------
int get_block_index(int n_block_size)
{
     switch(n_block_size)
     {
        case 4:
            return 2;

        case 8:
            return 3;

        case 16:
            return 4;

        case 32:
            return 5;

        case 64:
            return 6;

        case 128:
            return 7;

        case 256:
            return 8;

        default:
            return 3;
    }
}
//---------------------------------------------------------------
int get_block_value(int n_block_idx)
{
     switch(n_block_idx)
     {
        case 2:
            return 4;

        case 3:
            return 8;

        case 4:
            return 16;

        case 5:
            return 32;

        case 6:
            return 64;

        case 7:
            return 128;

        case 8:
            return 256;

        default:
            return 8;
    }
}

static MS_U32 calculate_ufc_bin_size(BinOutputInfo *pbin)
{

    MS_U32 bin_size, header_size;
    MS_U32 layer_size;

    header_size = sizeof(ST_DEMURA_UFC_HEADER);
    layer_size  = HAL_DEMURA_Cal_LutSize(pbin->LevelCount, pbin->HNode, pbin->VNode, pbin->Sep_type);
    bin_size    = header_size + layer_size ;

    //printf("[%s:%d]calculate bin size = 0x%x\n", __FUNCTION__, __LINE__, (unsigned int)bin_size);
    return bin_size;
}

static MS_U32 calculate_bin_size(BinOutputInfo *pbin)
{
    #define BIN_SIZE_ALIGNMENT  0x1000
    #define BIN_SIZE_ALIGNMASK  (BIN_SIZE_ALIGNMENT - 1)

    MS_U32 bin_size, header_size;
    MS_U32 reg_size, layer_size;

    header_size = sizeof(MstarSubBinHeader);

    #if (TEST_INTERFACE)
    layer_size  = HAL_DEMURA_Cal_LutSize(pbin->LevelCount, pbin->HNode, pbin->VNode, pbin->Sep_type);
    reg_size    = HAL_DEMURA_Max_RegSize();
    #else
    layer_size  = Mhal_DeMura_Cal_Layer_Size(pbin);
    reg_size    = Mhal_DeMura_Cal_Reg_Size();
    #endif

    bin_size    = header_size + layer_size + reg_size;

    bin_size   += ((bin_size * 20)/100);    // redundancy space
    if (bin_size & BIN_SIZE_ALIGNMASK)
    {
        bin_size +=  BIN_SIZE_ALIGNMENT;
        bin_size &= (~BIN_SIZE_ALIGNMASK);
    }
    //printf("[%s:%d]calculate bin size = 0x%x\n", __FUNCTION__, __LINE__, (unsigned int)bin_size);
    return bin_size;
}

static void _demura_set_ufc_bin(BinOutputInfo *pbin)
{
    MS_U16 width, height;
    demura_get_panelinfo(&width, &height);
    registers *p_regs = demura_core_get_regs();
    MS_U32 lut = HAL_DEMURA_Cal_LutSize(pbin->LevelCount, pbin->HNode, pbin->VNode, pbin->Sep_type);
    MS_U32 header_crc = 0;
    MS_U8 *Lut_data = (MS_U8 *)&m_DemuraUfcBinHeader;
    memset(&m_DemuraUfcBinHeader, 0, sizeof(ST_DEMURA_UFC_HEADER));
    if(snprintf((char *)&m_DemuraUfcBinHeader.format[0], MHAL_HEADER_FORMAT_NUM, "%s", UFC_HEADER_DEMURA_FORMAT) < 0)
    {
        return;
    }

    m_DemuraUfcBinHeader.version[0] = DEMURA_GET_UFC_U32ARRAY0(UFC_HEADER_VERSION);
    m_DemuraUfcBinHeader.version[1] = DEMURA_GET_UFC_U32ARRAY1(UFC_HEADER_VERSION);
    m_DemuraUfcBinHeader.version[2] = DEMURA_GET_UFC_U32ARRAY2(UFC_HEADER_VERSION);
    m_DemuraUfcBinHeader.version[3] = DEMURA_GET_UFC_U32ARRAY3(UFC_HEADER_VERSION);
    m_DemuraUfcBinHeader.header_size[0] = DEMURA_GET_UFC_U32ARRAY0(UFC_DEMURA_HEADER_SIZE);
    m_DemuraUfcBinHeader.header_size[1] = DEMURA_GET_UFC_U32ARRAY1(UFC_DEMURA_HEADER_SIZE);
    m_DemuraUfcBinHeader.header_size[2] = DEMURA_GET_UFC_U32ARRAY2(UFC_DEMURA_HEADER_SIZE);
    m_DemuraUfcBinHeader.header_size[3] = DEMURA_GET_UFC_U32ARRAY3(UFC_DEMURA_HEADER_SIZE);
    m_DemuraUfcBinHeader.lut_size[0] = DEMURA_GET_UFC_U32ARRAY0(lut);
    m_DemuraUfcBinHeader.lut_size[1] = DEMURA_GET_UFC_U32ARRAY1(lut);
    m_DemuraUfcBinHeader.lut_size[2] = DEMURA_GET_UFC_U32ARRAY2(lut);
    m_DemuraUfcBinHeader.lut_size[3] = DEMURA_GET_UFC_U32ARRAY3(lut);

    m_DemuraUfcBinHeader.h_size[0] = DEMURA_GET_UFC_U8ARRAY0(width);
    m_DemuraUfcBinHeader.h_size[1] = DEMURA_GET_UFC_U8ARRAY1(width);
    m_DemuraUfcBinHeader.v_size[0] = DEMURA_GET_UFC_U8ARRAY0(height);
    m_DemuraUfcBinHeader.v_size[1] = DEMURA_GET_UFC_U8ARRAY1(height);
    m_DemuraUfcBinHeader.mode = p_regs->reg_dmc_rgb_mode.val;
    m_DemuraUfcBinHeader.plane_num = p_regs->reg_dmc_plane_num.val; /* 33 */
    m_DemuraUfcBinHeader.gain_r[0] = p_regs->reg_dmc_data_r_mag1.val;
    m_DemuraUfcBinHeader.gain_r[1] = p_regs->reg_dmc_data_r_mag2.val;
    m_DemuraUfcBinHeader.gain_r[2] = p_regs->reg_dmc_data_r_mag3.val;
    m_DemuraUfcBinHeader.gain_r[3] = p_regs->reg_dmc_data_r_mag4.val;
    m_DemuraUfcBinHeader.gain_r[4] = p_regs->reg_dmc_data_r_mag5.val;
    m_DemuraUfcBinHeader.gain_r[5] = p_regs->reg_dmc_data_r_mag6.val;
    m_DemuraUfcBinHeader.gain_r[6] = p_regs->reg_dmc_data_r_mag7.val;
    m_DemuraUfcBinHeader.gain_r[7] = p_regs->reg_dmc_data_r_mag8.val;
    m_DemuraUfcBinHeader.gain_g[0] = p_regs->reg_dmc_data_g_mag1.val;
    m_DemuraUfcBinHeader.gain_g[1] = p_regs->reg_dmc_data_g_mag2.val;
    m_DemuraUfcBinHeader.gain_g[2] = p_regs->reg_dmc_data_g_mag3.val;
    m_DemuraUfcBinHeader.gain_g[3] = p_regs->reg_dmc_data_g_mag4.val;
    m_DemuraUfcBinHeader.gain_g[4] = p_regs->reg_dmc_data_g_mag5.val;
    m_DemuraUfcBinHeader.gain_g[5] = p_regs->reg_dmc_data_g_mag6.val;
    m_DemuraUfcBinHeader.gain_g[6] = p_regs->reg_dmc_data_g_mag7.val;
    m_DemuraUfcBinHeader.gain_g[7] = p_regs->reg_dmc_data_g_mag8.val;
    m_DemuraUfcBinHeader.gain_b[0] = p_regs->reg_dmc_data_b_mag1.val;
    m_DemuraUfcBinHeader.gain_b[1] = p_regs->reg_dmc_data_b_mag2.val;
    m_DemuraUfcBinHeader.gain_b[2] = p_regs->reg_dmc_data_b_mag3.val;
    m_DemuraUfcBinHeader.gain_b[3] = p_regs->reg_dmc_data_b_mag4.val;
    m_DemuraUfcBinHeader.gain_b[4] = p_regs->reg_dmc_data_b_mag5.val;
    m_DemuraUfcBinHeader.gain_b[5] = p_regs->reg_dmc_data_b_mag6.val;
    m_DemuraUfcBinHeader.gain_b[6] = p_regs->reg_dmc_data_b_mag7.val;
    m_DemuraUfcBinHeader.gain_b[7] = p_regs->reg_dmc_data_b_mag8.val;
    m_DemuraUfcBinHeader.offset_r[0][0] = DEMURA_GET_UFC_U8ARRAY0(p_regs->reg_dmc_data_r_offset1.val);
    m_DemuraUfcBinHeader.offset_r[0][1] = DEMURA_GET_UFC_U8ARRAY1(p_regs->reg_dmc_data_r_offset1.val);
    m_DemuraUfcBinHeader.offset_r[1][0] = DEMURA_GET_UFC_U8ARRAY0(p_regs->reg_dmc_data_r_offset2.val);
    m_DemuraUfcBinHeader.offset_r[1][1] = DEMURA_GET_UFC_U8ARRAY1(p_regs->reg_dmc_data_r_offset2.val);
    m_DemuraUfcBinHeader.offset_r[2][0] = DEMURA_GET_UFC_U8ARRAY0(p_regs->reg_dmc_data_r_offset3.val);
    m_DemuraUfcBinHeader.offset_r[2][1] = DEMURA_GET_UFC_U8ARRAY1(p_regs->reg_dmc_data_r_offset3.val);
    m_DemuraUfcBinHeader.offset_r[3][0] = DEMURA_GET_UFC_U8ARRAY0(p_regs->reg_dmc_data_r_offset4.val);
    m_DemuraUfcBinHeader.offset_r[3][1] = DEMURA_GET_UFC_U8ARRAY1(p_regs->reg_dmc_data_r_offset4.val);
    m_DemuraUfcBinHeader.offset_r[4][0] = DEMURA_GET_UFC_U8ARRAY0(p_regs->reg_dmc_data_r_offset5.val);
    m_DemuraUfcBinHeader.offset_r[4][1] = DEMURA_GET_UFC_U8ARRAY1(p_regs->reg_dmc_data_r_offset5.val);
    m_DemuraUfcBinHeader.offset_r[5][0] = DEMURA_GET_UFC_U8ARRAY0(p_regs->reg_dmc_data_r_offset6.val);
    m_DemuraUfcBinHeader.offset_r[5][1] = DEMURA_GET_UFC_U8ARRAY1(p_regs->reg_dmc_data_r_offset6.val);
    m_DemuraUfcBinHeader.offset_r[6][0] = DEMURA_GET_UFC_U8ARRAY0(p_regs->reg_dmc_data_r_offset7.val);
    m_DemuraUfcBinHeader.offset_r[6][1] = DEMURA_GET_UFC_U8ARRAY1(p_regs->reg_dmc_data_r_offset7.val);
    m_DemuraUfcBinHeader.offset_r[7][0] = DEMURA_GET_UFC_U8ARRAY0(p_regs->reg_dmc_data_r_offset8.val);
    m_DemuraUfcBinHeader.offset_r[7][1] = DEMURA_GET_UFC_U8ARRAY1(p_regs->reg_dmc_data_r_offset8.val);
    m_DemuraUfcBinHeader.offset_g[0][0] = DEMURA_GET_UFC_U8ARRAY0(p_regs->reg_dmc_data_g_offset1.val);
    m_DemuraUfcBinHeader.offset_g[0][1] = DEMURA_GET_UFC_U8ARRAY1(p_regs->reg_dmc_data_g_offset1.val);
    m_DemuraUfcBinHeader.offset_g[1][0] = DEMURA_GET_UFC_U8ARRAY0(p_regs->reg_dmc_data_g_offset2.val);
    m_DemuraUfcBinHeader.offset_g[1][1] = DEMURA_GET_UFC_U8ARRAY1(p_regs->reg_dmc_data_g_offset2.val);
    m_DemuraUfcBinHeader.offset_g[2][0] = DEMURA_GET_UFC_U8ARRAY0(p_regs->reg_dmc_data_g_offset3.val);
    m_DemuraUfcBinHeader.offset_g[2][1] = DEMURA_GET_UFC_U8ARRAY1(p_regs->reg_dmc_data_g_offset3.val);
    m_DemuraUfcBinHeader.offset_g[3][0] = DEMURA_GET_UFC_U8ARRAY0(p_regs->reg_dmc_data_g_offset4.val);
    m_DemuraUfcBinHeader.offset_g[3][1] = DEMURA_GET_UFC_U8ARRAY1(p_regs->reg_dmc_data_g_offset4.val);
    m_DemuraUfcBinHeader.offset_g[4][0] = DEMURA_GET_UFC_U8ARRAY0(p_regs->reg_dmc_data_g_offset5.val);
    m_DemuraUfcBinHeader.offset_g[4][1] = DEMURA_GET_UFC_U8ARRAY1(p_regs->reg_dmc_data_g_offset5.val);
    m_DemuraUfcBinHeader.offset_g[5][0] = DEMURA_GET_UFC_U8ARRAY0(p_regs->reg_dmc_data_g_offset6.val);
    m_DemuraUfcBinHeader.offset_g[5][1] = DEMURA_GET_UFC_U8ARRAY1(p_regs->reg_dmc_data_g_offset6.val);
    m_DemuraUfcBinHeader.offset_g[6][0] = DEMURA_GET_UFC_U8ARRAY0(p_regs->reg_dmc_data_g_offset7.val);
    m_DemuraUfcBinHeader.offset_g[6][1] = DEMURA_GET_UFC_U8ARRAY1(p_regs->reg_dmc_data_g_offset7.val);
    m_DemuraUfcBinHeader.offset_g[7][0] = DEMURA_GET_UFC_U8ARRAY0(p_regs->reg_dmc_data_g_offset8.val);
    m_DemuraUfcBinHeader.offset_g[7][1] = DEMURA_GET_UFC_U8ARRAY1(p_regs->reg_dmc_data_g_offset8.val);
    m_DemuraUfcBinHeader.offset_b[0][0] = DEMURA_GET_UFC_U8ARRAY0(p_regs->reg_dmc_data_b_offset1.val);
    m_DemuraUfcBinHeader.offset_b[0][1] = DEMURA_GET_UFC_U8ARRAY1(p_regs->reg_dmc_data_b_offset1.val);
    m_DemuraUfcBinHeader.offset_b[1][0] = DEMURA_GET_UFC_U8ARRAY0(p_regs->reg_dmc_data_b_offset2.val);
    m_DemuraUfcBinHeader.offset_b[1][1] = DEMURA_GET_UFC_U8ARRAY1(p_regs->reg_dmc_data_b_offset2.val);
    m_DemuraUfcBinHeader.offset_b[2][0] = DEMURA_GET_UFC_U8ARRAY0(p_regs->reg_dmc_data_b_offset3.val);
    m_DemuraUfcBinHeader.offset_b[2][1] = DEMURA_GET_UFC_U8ARRAY1(p_regs->reg_dmc_data_b_offset3.val);
    m_DemuraUfcBinHeader.offset_b[3][0] = DEMURA_GET_UFC_U8ARRAY0(p_regs->reg_dmc_data_b_offset4.val);
    m_DemuraUfcBinHeader.offset_b[3][1] = DEMURA_GET_UFC_U8ARRAY1(p_regs->reg_dmc_data_b_offset4.val);
    m_DemuraUfcBinHeader.offset_b[4][0] = DEMURA_GET_UFC_U8ARRAY0(p_regs->reg_dmc_data_b_offset5.val);
    m_DemuraUfcBinHeader.offset_b[4][1] = DEMURA_GET_UFC_U8ARRAY1(p_regs->reg_dmc_data_b_offset5.val);
    m_DemuraUfcBinHeader.offset_b[5][0] = DEMURA_GET_UFC_U8ARRAY0(p_regs->reg_dmc_data_b_offset6.val);
    m_DemuraUfcBinHeader.offset_b[5][1] = DEMURA_GET_UFC_U8ARRAY1(p_regs->reg_dmc_data_b_offset6.val);
    m_DemuraUfcBinHeader.offset_b[6][0] = DEMURA_GET_UFC_U8ARRAY0(p_regs->reg_dmc_data_b_offset7.val);
    m_DemuraUfcBinHeader.offset_b[6][1] = DEMURA_GET_UFC_U8ARRAY1(p_regs->reg_dmc_data_b_offset7.val);
    m_DemuraUfcBinHeader.offset_b[7][0] = DEMURA_GET_UFC_U8ARRAY0(p_regs->reg_dmc_data_b_offset8.val);
    m_DemuraUfcBinHeader.offset_b[7][1] = DEMURA_GET_UFC_U8ARRAY1(p_regs->reg_dmc_data_b_offset8.val);

    m_DemuraUfcBinHeader.h_blk_size = p_regs->reg_dmc_h_block.val; /* 106 */
    m_DemuraUfcBinHeader.v_blk_size = p_regs->reg_dmc_v_block.val; /* 107 */
    m_DemuraUfcBinHeader.black_limit[0] = DEMURA_GET_UFC_U8ARRAY0(p_regs->reg_dmc_black_limit.val);
    m_DemuraUfcBinHeader.black_limit[1] = DEMURA_GET_UFC_U8ARRAY1(p_regs->reg_dmc_black_limit.val);

    m_DemuraUfcBinHeader.plane[0][0] = DEMURA_GET_UFC_U8ARRAY0(p_regs->reg_dmc_plane_level1.val);
    m_DemuraUfcBinHeader.plane[0][1] = DEMURA_GET_UFC_U8ARRAY1(p_regs->reg_dmc_plane_level1.val);
    m_DemuraUfcBinHeader.plane[1][0] = DEMURA_GET_UFC_U8ARRAY0(p_regs->reg_dmc_plane_level2.val);
    m_DemuraUfcBinHeader.plane[1][1] = DEMURA_GET_UFC_U8ARRAY1(p_regs->reg_dmc_plane_level2.val);
    m_DemuraUfcBinHeader.plane[2][0] = DEMURA_GET_UFC_U8ARRAY0(p_regs->reg_dmc_plane_level3.val);
    m_DemuraUfcBinHeader.plane[2][1] = DEMURA_GET_UFC_U8ARRAY1(p_regs->reg_dmc_plane_level3.val);
    m_DemuraUfcBinHeader.plane[3][0] = DEMURA_GET_UFC_U8ARRAY0(p_regs->reg_dmc_plane_level4.val);
    m_DemuraUfcBinHeader.plane[3][1] = DEMURA_GET_UFC_U8ARRAY1(p_regs->reg_dmc_plane_level4.val);
    m_DemuraUfcBinHeader.plane[4][0] = DEMURA_GET_UFC_U8ARRAY0(p_regs->reg_dmc_plane_level5.val);
    m_DemuraUfcBinHeader.plane[4][1] = DEMURA_GET_UFC_U8ARRAY1(p_regs->reg_dmc_plane_level5.val);
    m_DemuraUfcBinHeader.plane[5][0] = DEMURA_GET_UFC_U8ARRAY0(p_regs->reg_dmc_plane_level6.val);
    m_DemuraUfcBinHeader.plane[5][1] = DEMURA_GET_UFC_U8ARRAY1(p_regs->reg_dmc_plane_level6.val);
    m_DemuraUfcBinHeader.plane[6][0] = DEMURA_GET_UFC_U8ARRAY0(p_regs->reg_dmc_plane_level7.val);
    m_DemuraUfcBinHeader.plane[6][1] = DEMURA_GET_UFC_U8ARRAY1(p_regs->reg_dmc_plane_level7.val);
    m_DemuraUfcBinHeader.plane[7][0] = DEMURA_GET_UFC_U8ARRAY0(p_regs->reg_dmc_plane_level8.val);
    m_DemuraUfcBinHeader.plane[7][1] = DEMURA_GET_UFC_U8ARRAY1(p_regs->reg_dmc_plane_level8.val);
    m_DemuraUfcBinHeader.white_limit[0] = DEMURA_GET_UFC_U8ARRAY0(p_regs->reg_dmc_white_limit.val);
    m_DemuraUfcBinHeader.white_limit[1] = DEMURA_GET_UFC_U8ARRAY1(p_regs->reg_dmc_white_limit.val);
    m_DemuraUfcBinHeader.dither_en = p_regs->reg_dmc_dither_en.val;

    for (lut = 0; lut < UFC_DEMURA_HEADER_CHECKSUM_NUM; lut++)
    {
        header_crc += Lut_data[lut];
    }

    m_DemuraUfcBinHeader.header_crc[0] = DEMURA_GET_UFC_U32ARRAY0(header_crc);
    m_DemuraUfcBinHeader.header_crc[1] = DEMURA_GET_UFC_U32ARRAY1(header_crc);
    m_DemuraUfcBinHeader.header_crc[2] = DEMURA_GET_UFC_U32ARRAY2(header_crc);
    m_DemuraUfcBinHeader.header_crc[3] = DEMURA_GET_UFC_U32ARRAY3(header_crc);

#if DEMURA_DLL_DUMP_LOG
    printf("%s@%d\n", __FUNCTION__,__LINE__);
    printf("g_u16PanelWidth = %u\n", g_u16PanelWidth);
    printf("g_u16PanelHeight = %u\n", g_u16PanelHeight);
    printf("reg_dmc_rgb_mode = %u\n", p_regs->reg_dmc_rgb_mode.val);
    printf("reg_dmc_plane_num = %u\n", p_regs->reg_dmc_plane_num.val);
    printf("reg_dmc_data_r_mag1= %u \n", p_regs->reg_dmc_data_r_mag1.val);
    printf("reg_dmc_data_r_mag2= %u \n", p_regs->reg_dmc_data_r_mag2.val);
    printf("reg_dmc_data_r_mag3= %u \n", p_regs->reg_dmc_data_r_mag3.val);
    printf("reg_dmc_data_r_mag4= %u \n", p_regs->reg_dmc_data_r_mag4.val);
    printf("reg_dmc_data_r_mag5= %u \n", p_regs->reg_dmc_data_r_mag5.val);
    printf("reg_dmc_data_r_mag6= %u \n", p_regs->reg_dmc_data_r_mag6.val);
    printf("reg_dmc_data_r_mag7= %u \n", p_regs->reg_dmc_data_r_mag7.val);
    printf("reg_dmc_data_r_mag8= %u \n", p_regs->reg_dmc_data_r_mag8.val);
    printf("reg_dmc_data_g_mag1= %u \n", p_regs->reg_dmc_data_g_mag1.val);
    printf("reg_dmc_data_g_mag2= %u \n", p_regs->reg_dmc_data_g_mag2.val);
    printf("reg_dmc_data_g_mag3= %u \n", p_regs->reg_dmc_data_g_mag3.val);
    printf("reg_dmc_data_g_mag4= %u \n", p_regs->reg_dmc_data_g_mag4.val);
    printf("reg_dmc_data_g_mag5= %u \n", p_regs->reg_dmc_data_g_mag5.val);
    printf("reg_dmc_data_g_mag6= %u \n", p_regs->reg_dmc_data_g_mag6.val);
    printf("reg_dmc_data_g_mag7= %u \n", p_regs->reg_dmc_data_g_mag7.val);
    printf("reg_dmc_data_g_mag8= %u \n", p_regs->reg_dmc_data_g_mag8.val);
    printf("reg_dmc_data_b_mag1= %u \n", p_regs->reg_dmc_data_b_mag1.val);
    printf("reg_dmc_data_b_mag2= %u \n", p_regs->reg_dmc_data_b_mag2.val);
    printf("reg_dmc_data_b_mag3= %u \n", p_regs->reg_dmc_data_b_mag3.val);
    printf("reg_dmc_data_b_mag4= %u \n", p_regs->reg_dmc_data_b_mag4.val);
    printf("reg_dmc_data_b_mag5= %u \n", p_regs->reg_dmc_data_b_mag5.val);
    printf("reg_dmc_data_b_mag6= %u \n", p_regs->reg_dmc_data_b_mag6.val);
    printf("reg_dmc_data_b_mag7= %u \n", p_regs->reg_dmc_data_b_mag7.val);
    printf("reg_dmc_data_b_mag8= %u \n", p_regs->reg_dmc_data_b_mag8.val);

    printf("reg_dmc_data_r_offset1 = %u\n", p_regs->reg_dmc_data_r_offset1.val);
    printf("reg_dmc_data_r_offset2 = %u\n", p_regs->reg_dmc_data_r_offset2.val);
    printf("reg_dmc_data_r_offset3 = %u\n", p_regs->reg_dmc_data_r_offset3.val);
    printf("reg_dmc_data_r_offset4 = %u\n", p_regs->reg_dmc_data_r_offset4.val);
    printf("reg_dmc_data_r_offset5 = %u\n", p_regs->reg_dmc_data_r_offset5.val);
    printf("reg_dmc_data_r_offset6 = %u\n", p_regs->reg_dmc_data_r_offset6.val);
    printf("reg_dmc_data_r_offset7 = %u\n", p_regs->reg_dmc_data_r_offset7.val);
    printf("reg_dmc_data_r_offset8 = %u\n", p_regs->reg_dmc_data_r_offset8.val);
    printf("reg_dmc_data_g_offset1 = %u\n", p_regs->reg_dmc_data_g_offset1.val);
    printf("reg_dmc_data_g_offset2 = %u\n", p_regs->reg_dmc_data_g_offset2.val);
    printf("reg_dmc_data_g_offset3 = %u\n", p_regs->reg_dmc_data_g_offset3.val);
    printf("reg_dmc_data_g_offset4 = %u\n", p_regs->reg_dmc_data_g_offset4.val);
    printf("reg_dmc_data_g_offset5 = %u\n", p_regs->reg_dmc_data_g_offset5.val);
    printf("reg_dmc_data_g_offset6 = %u\n", p_regs->reg_dmc_data_g_offset6.val);
    printf("reg_dmc_data_g_offset7 = %u\n", p_regs->reg_dmc_data_g_offset7.val);
    printf("reg_dmc_data_g_offset8 = %u\n", p_regs->reg_dmc_data_g_offset8.val);
    printf("reg_dmc_data_b_offset1 = %u\n", p_regs->reg_dmc_data_b_offset1.val);
    printf("reg_dmc_data_b_offset2 = %u\n", p_regs->reg_dmc_data_b_offset2.val);
    printf("reg_dmc_data_b_offset3 = %u\n", p_regs->reg_dmc_data_b_offset3.val);
    printf("reg_dmc_data_b_offset4 = %u\n", p_regs->reg_dmc_data_b_offset4.val);
    printf("reg_dmc_data_b_offset5 = %u\n", p_regs->reg_dmc_data_b_offset5.val);
    printf("reg_dmc_data_b_offset6 = %u\n", p_regs->reg_dmc_data_b_offset6.val);
    printf("reg_dmc_data_b_offset7 = %u\n", p_regs->reg_dmc_data_b_offset7.val);
    printf("reg_dmc_data_b_offset8 = %u\n", p_regs->reg_dmc_data_b_offset8.val);
    printf("reg_dmc_black_limit = %u\n", p_regs->reg_dmc_black_limit.val);
    printf("reg_dmc_plane_level1 = %u \n", p_regs->reg_dmc_plane_level1.val);
    printf("reg_dmc_plane_level2 = %u \n", p_regs->reg_dmc_plane_level2.val);
    printf("reg_dmc_plane_level3 = %u \n", p_regs->reg_dmc_plane_level3.val);
    printf("reg_dmc_plane_level4 = %u \n", p_regs->reg_dmc_plane_level4.val);
    printf("reg_dmc_plane_level5 = %u \n", p_regs->reg_dmc_plane_level5.val);
    printf("reg_dmc_plane_level6 = %u \n", p_regs->reg_dmc_plane_level6.val);
    printf("reg_dmc_plane_level7 = %u \n", p_regs->reg_dmc_plane_level7.val);
    printf("reg_dmc_plane_level8 = %u \n", p_regs->reg_dmc_plane_level8.val);
    printf("reg_dmc_white_limit = %u\n", p_regs->reg_dmc_white_limit.val);
    printf("header_crc = %x\n", header_crc);
#endif
}

//---------------------------------------------------------------
MS_BOOL mstar_demura_interface (interface_info *Info1, BinOutputInfo *pbin_info)
{
    //Inital Value
#ifdef CONFIG_MTK_DEMURA_UFC_BIN
#else
    int i;
    MS_U32  project_id;
    MS_U32 u32Tmp, u32Value;
#endif
    int m_alignment;
    MS_BOOL ret;

    MS_U32  reg_base_addr;
    MS_U32  lut_h_size, lut_v_size;

    reg_base_addr   =  pbin_info->reg_base_addr;
    lut_h_size      =  pbin_info->HNode;
    lut_v_size      =  pbin_info->VNode;

    m_alignment = Aligned_Value - (sizeof(MstarSubBinHeader)%Aligned_Value);
    if (m_alignment == Aligned_Value)
    {
        m_alignment = 0;
    }

    memset((void *)&m_MstarSubHeader, 0, sizeof(MstarSubBinHeader));
    memset((void *)&m_Header, 0, sizeof(DemuraHeader));
    memset((void *)&m_DemuraUfcBinHeader, 0, sizeof(DemuraSubBinHeaderUFC));

    /******************************* begin to covert *****************************************/

    int lut_out_size = calculate_bin_size(pbin_info);
    m_vector  Lut_out_vector;

    ret = alloc_vector(1, lut_out_size, &Lut_out_vector);
    if (ret == FALSE)
    {
        printf("alloc_vector for Lut_out_vector failed\n");
        return FALSE;
    }

    #if (TEST_INTERFACE)
    int reg_num  = HAL_DEMURA_Max_RegCount();
    #else
    int reg_num  = Mhal_DeMura_Max_Reg_Count();
    #endif

    m_vector  Reg_vector;
//    ret = alloc_vector(sizeof(reg_struct), reg_num, &Reg_vector);
    ret = alloc_vector(sizeof(demura_reg), reg_num, &Reg_vector);
    if (ret == FALSE)
    {
        printf("alloc_vector for Reg_vector failed\n");
        dfree(Lut_out_vector.pbuf);
        Lut_out_vector.pbuf = NULL;
        return FALSE;
    }

    demura_core_ver10(reg_base_addr, lut_h_size, lut_v_size, Info1, &Reg_vector, &Lut_out_vector);
    /***************************************************************************************/
#ifdef CONFIG_MTK_DEMURA_UFC_BIN
    {
        int bin_size =  calculate_ufc_bin_size(pbin_info);
        m_vector Bin_vector;
        ret = alloc_vector(1, bin_size, &Bin_vector);
        if (ret == FALSE)
        {
            printf("alloc_vector for Bin_vector failed\n");
            dfree(Lut_out_vector.pbuf);
            Lut_out_vector.pbuf = NULL;
            dfree(Reg_vector.pbuf);
            Reg_vector.pbuf = NULL;
            return FALSE;
        }
        bin_buf = Bin_vector.pbuf;
        bin_cur_pos = bin_buf;

        _demura_set_ufc_bin(pbin_info);
        memcpy(bin_buf, &m_DemuraUfcBinHeader, sizeof(DemuraSubBinHeaderUFC));
        bin_cur_pos += sizeof(DemuraSubBinHeaderUFC);
        memcpy(bin_cur_pos, Lut_out_vector.pbuf, bin_size - sizeof(DemuraSubBinHeaderUFC));


        pbin_info->bin_buf  = bin_buf;
        pbin_info->bin_size = bin_size;

        dfree(Lut_out_vector.pbuf);
        Lut_out_vector.pbuf = NULL;
        dfree(Reg_vector.pbuf);
        Reg_vector.pbuf = NULL;
    }
#else
    /***************************** prepare to output data ************************************/
    int bin_size = calculate_bin_size(pbin_info);
    m_vector Bin_vector;
    ret = alloc_vector(1, bin_size, &Bin_vector);
    if (ret == FALSE)
    {
        printf("alloc_vector for Bin_vector failed\n");
        dfree(Lut_out_vector.pbuf);
        Lut_out_vector.pbuf = NULL;
        dfree(Reg_vector.pbuf);
        Reg_vector.pbuf = NULL;
        return FALSE;
    }
    bin_buf = Bin_vector.pbuf;
    bin_cur_pos = bin_buf;
    /***************************************************************************************/


    int packet_type = 0;  // 0:128bits, 1:256bits
    // packect size
    #if 0
    switch (project_id >> 16)
    {
        case ID_CHIP_U9:
            packet_type = 1;
            break;

        case ID_CHIP_U10:
            packet_type = 0;
            break;

        case ID_CHIP_U11:
            packet_type = 1;
            break;

        case ID_CHIP_U12:
            packet_type = 1;
            break;

        case ID_CHIP_TERRA:
            packet_type = 0;
            break;
        case ID_CHIP_MORTON:
            packet_type = 1;
            break;
        /*case ID_CHIP_MERLIN:
            packet_type = 1;
            break;   */
        default:
            break;
    }
    #endif

    #if (TEST_INTERFACE)
    packet_type = HAL_DEMURA_Get_PacketType();
    #else
    packet_type = Mhal_DeMura_Get_PacketType();
    #endif
    //int packet_size = 1;

    /********************************** layer data **********************************/
    // align to 16 byte
    bin_cur_pos = bin_buf + (sizeof(MstarSubBinHeader) + m_alignment + CusDataSize);
    memset(bin_buf, 0, (MS_U32)(bin_cur_pos - bin_buf));

    int DataLength = Lut_out_vector.dat_num;     // Write Data length
    MS_U8* Lut_data = Lut_out_vector.pbuf;
    memcpy(bin_cur_pos, Lut_data, DataLength);
    bin_cur_pos += DataLength;

    /*******************************************************************************/

    //check format, if compressed than do it
    int gzip_size = DataLength;

    /****************************** write Register ********************************/
    RegData RegisterData;
    int registerCount = 0;
    //reg_struct *reg_data  = (reg_struct *)Reg_vector.pbuf;
    demura_reg *reg_data  = (demura_reg *)Reg_vector.pbuf;
    for (i = 0 ; i < Reg_vector.dat_num; i++)
    {
        u32Tmp = HAL_DEMURA_Fld2Mask(reg_data[i].mask);
        u32Value = HAL_DEMURA_Val2FldMask(reg_data[i].val, reg_data[i].mask);
#if DEMURA_DLL_DUMP_LOG
        printf("%s@%d %lx %x %x=> %x & %x\n", __FUNCTION__,__LINE__, reg_data[i].addr, reg_data[i].val, reg_data[i].mask, u32Tmp, u32Value);
#endif
        RegisterData.RegAddr  = reg_data[i].addr - DEMURA_BKA377; // L
        RegisterData.RegValue = u32Value & 0xFF;
        RegisterData.RegMask  = u32Tmp & 0xFF;

#if DEMURA_DLL_DUMP_LOG
        printf("%s@%d reg:%3x val:%02x mask:%02x\n", __FUNCTION__,__LINE__, RegisterData.RegAddr, RegisterData.RegValue, RegisterData.RegMask);
#endif

        if (RegisterData.RegMask != 0)
        {
            memcpy(bin_cur_pos, &RegisterData, sizeof(RegData));
            bin_cur_pos += sizeof(RegData);
            registerCount++;
        }
        RegisterData.RegAddr  = reg_data[i].addr - DEMURA_BKA377 + 1; // H
        RegisterData.RegValue = (u32Value & 0xFF00) >> 8;
        RegisterData.RegMask  = (u32Tmp & 0xFF00) >> 8;

#if DEMURA_DLL_DUMP_LOG
        printf("%s@%d reg:%3x val:%02x mask:%02x\n", __FUNCTION__,__LINE__, RegisterData.RegAddr, RegisterData.RegValue, RegisterData.RegMask);
#endif

        if (RegisterData.RegMask != 0)
        {
            memcpy(bin_cur_pos, &RegisterData, sizeof(RegData));
            bin_cur_pos += sizeof(RegData);
            registerCount++;
        }
    }

    /*******************************************************************************/

    //write SubHeader info
    m_MstarSubHeader.nAllBinCheckSum = 0;
    m_MstarSubHeader.nHeaderCheckSum = 0;

    m_MstarSubHeader.nHeaderSize = sizeof(MstarSubBinHeader);
    m_MstarSubHeader.nAllBinSize = sizeof(MstarSubBinHeader) + m_alignment + gzip_size + (registerCount * sizeof(RegisterData));

    m_MstarSubHeader.nDemuraIdH = sw8Byte(0x6D73746172206465);  //Identification for a demura bin: "mstar demura", not be little endian
    m_MstarSubHeader.nDemuraIdL = sw8Byte(0x6D75726100000000);  //Hex values: 6D 73 74 61 72 20 64 65 6D 75 72 61 00 00 00 00
    m_MstarSubHeader.nBinVersion = BinVersion;

    m_MstarSubHeader.nDataFormat = pbin_info->data_type;
    m_MstarSubHeader.bR_ch_Enable = pbin_info->channel_enable[0];
    m_MstarSubHeader.bG_ch_Enable = pbin_info->channel_enable[1];
    m_MstarSubHeader.bB_ch_Enable = pbin_info->channel_enable[2];
    //m_MstarSubHeader.bW_ch_Enable = pbin_info->channel_enable[3];

    m_MstarSubHeader.nReserved = 0;

    //calculate H node
    int Reg_H_Node = lut_h_size;
    int H_offset = 1;;

    // reg_dmc_rgb_mode = 0 => ymode
    // reg_dmc_rgb_mode = 1 => RGB Mode
    if (Info1->reg_dmc_rgb_mode == 1)
    {
        switch(Info1->reg_dmc_plane_num)
        {
        case 1:
        case 2:
            H_offset = 4;
            break;
        case 3:
        case 4:
            H_offset = 2;
            break;
        case 5:
        case 6:
        case 7:
        case 8:
            H_offset = 1;
            break;
        }
    }
    else
    {
        switch(Info1->reg_dmc_plane_num)
        {
        case 1:
        case 2:
            H_offset = 16;
            break;
        case 3:
        case 4:
            H_offset = 8;
            break;
        case 5:
        case 6:
        case 7:
        case 8:
            H_offset = 4;
            break;
        }
    }

    Reg_H_Node = (lut_h_size / H_offset) + (((lut_h_size % H_offset) == 0)?0:1);

    m_MstarSubHeader.nHNodeCount = Reg_H_Node;
    m_MstarSubHeader.nVNodeCount = lut_v_size;

/*
    //Get Start Layer
    int nLayerStart = 0;
    int nLayerEnd = 0;

    int color = 0;
    for (color = 0 ; color < 3; color++)
    {
        if (channel_enable[color] == TRUE)
        {
            break;
        }
    }
*/

    m_MstarSubHeader.nLayerStart = 1;
    m_MstarSubHeader.nLayerEnd = Info1->reg_dmc_plane_num;
    m_MstarSubHeader.bSeparate = Info1->reg_dmc_rgb_mode;

    m_MstarSubHeader.nLDataStartAddr = sizeof(MstarSubBinHeader) + m_alignment + CusDataSize;
    m_MstarSubHeader.nLDataOriginalSize = DataLength;
    m_MstarSubHeader.nLDataFlashSize = gzip_size;

    m_MstarSubHeader.nRegisterStartAddr = sizeof(MstarSubBinHeader) + m_alignment + gzip_size + CusDataSize;
    m_MstarSubHeader.nRegisterCount = registerCount;

    m_MstarSubHeader.nDllVersion = DllVersion;
    m_MstarSubHeader.nProjectID = pbin_info->project_id;

    m_MstarSubHeader.nCusDataStartAddr = 0x00000000;
    m_MstarSubHeader.nCusDataSize = CusDataSize;//0x00000000;

    m_MstarSubHeader.nHBlockSize = Info1->reg_dmc_h_block;
    m_MstarSubHeader.nVBlockSize = Info1->reg_dmc_v_block;
    m_MstarSubHeader.nPacketSize = packet_type;

    //get date
    if (pbin_info->Build_Date == 0x00)
    {
        //char NowDate[20];
        //sprintf(NowDate,"%02d%02d%02d%02d", 17, 5, 5, 10);
        //m_MstarSubHeader.nDate = sw4Byte(StrToHex(NowDate, 0));
	 pbin_info->Build_Date = 0x20111315;
	 m_MstarSubHeader.nDate = sw4Byte(pbin_info->Build_Date);
    }
    else
    {
        m_MstarSubHeader.nDate = sw4Byte(pbin_info->Build_Date);
    }

    // update Demura_bin_size
    m_MstarSubHeader.nAllBinSize = (int)(bin_cur_pos - bin_buf);

    // CRC32 Demura_bin_header
    m_MstarSubHeader.nHeaderCheckSum = MDrv_CRC32_Cal_DeMura((MS_U8*)&m_MstarSubHeader + 8, sizeof(MstarSubBinHeader)-8);

    //write Mstar SubHeader, to make sure 'CRC32 Demura_all_bin' OK
    memcpy(bin_buf, &m_MstarSubHeader, sizeof(m_MstarSubHeader));

    // CRC32 Demura_all_bin
    m_MstarSubHeader.nAllBinCheckSum = MDrv_CRC32_Cal_DeMura(bin_buf + 4, m_MstarSubHeader.nAllBinSize-4);

    //write Mstar SubHeader
    memcpy(bin_buf, &m_MstarSubHeader, sizeof(m_MstarSubHeader));

    pbin_info->bin_buf  = bin_buf;
    pbin_info->bin_size = m_MstarSubHeader.nAllBinSize;

    dfree(Lut_out_vector.pbuf);
    Lut_out_vector.pbuf = NULL;
    dfree(Reg_vector.pbuf);
    Reg_vector.pbuf = NULL;
#endif
    return TRUE;
}
//---------------------------------------------------------------

void demura_set_version(int version)
{
    demura_store_version(version);
}

void demura_set_panelinfo(MS_U16 width, MS_U16 height)
{
    g_u16PanelWidth = width;
    g_u16PanelHeight = height;
}

void demura_get_panelinfo(MS_U16 *width, MS_U16 *height)
{
    *width = g_u16PanelWidth;
    *height = g_u16PanelHeight;
}

int demura_get_h_nodes(int panel_width, int block_width_shift, int plane_num, int mode)
{
    int h_lut = ((panel_width >> block_width_shift) + 1);
    if (demura_get_version() == E_DEMURA_VERSION_S11)
    {
        return h_lut;
    }
    else
    {
        if (mode == 0) //mono mode
        {
            if (plane_num >= 5)
            {
                return DEMURA_CALC_H_NODES(h_lut, 4);
            }
            else if (plane_num >= 3)
            {
                return DEMURA_CALC_H_NODES(h_lut, 8);
            }
            else
            {
                return DEMURA_CALC_H_NODES(h_lut, 16);
            }
        }
        else
        {
            if (plane_num >= 5)
            {
                return DEMURA_CALC_H_NODES(h_lut, 1);
            }
            else if (plane_num >= 3)
            {
                return DEMURA_CALC_H_NODES(h_lut, 2);
            }
            else
            {
                return DEMURA_CALC_H_NODES(h_lut, 4);
            }
        }
    }
    return h_lut;
}

