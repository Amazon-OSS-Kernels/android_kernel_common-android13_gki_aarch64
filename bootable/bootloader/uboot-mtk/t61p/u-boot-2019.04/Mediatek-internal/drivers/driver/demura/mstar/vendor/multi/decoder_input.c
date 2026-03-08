// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

#include <command.h>
#include <common.h>
#include <stdio.h>
#include <debug_impl.h>
#include <MsTypes.h>
//#include <ShareType.h>
#include <malloc.h>

#include <CommonDataType.h>
#include <dmalloc.h>
#include <ms_vector.h>
#include <read_text.h>
#include <ms_utils.h>
#include <demura.h>
#include <ms_array.h>
#include <demura_common.h>
#include <demuraDll.h>

//#include <apiPNL.h>
#include <MDrvDemura.h>
#include <halDemura.h>
#include <convert_entry.h>
#include "decompress_8206.h"
#include "vendor.h"
#include "parse_fcic.h"

#include "crc_libs.h"
#include "demura_config.h"
#include "environment.h"
#include "parse_header.h"

#define INX_HEADER_MAX_SIZE     0x200
#define SAMPLE_DAT_START_SDC    2*(481*271+1)     // Byte
#define SAMPLE_DAT_START    1024     // Byte
#define SAMPLE_DAT_LEN      4096     // Byte

#define DEMURA_ROUNDING     0x0
#define DEMURA_DITHER       0x1
#define DEMURA_TRUNCATE     0x2

#define DEC_2    (2)
#define DEC_15   (15)
#define DEC_16   (16)
#define DEC_1024 (1024)
#define HEX_ALL  (0xFFFFFFFF)

#if(FORCE_TO_4_LAYER)
static MS_U8 SDC_Select_Layer[4] = {2,4,5,6};
#endif

static MS_BOOL Gen_Sf_Signature_AUO(AUO_Demura_Header *phdr);
#if defined (CONFIG_DEMURA_URSA13)
static void dump_interface_info(interface_info *pDataInfo);
#endif

// INX Start
static Vendor2Mstar_Info dec_info;


char get_env_demura_dither(void)
{
    if(env_get(DEMURA_ENV_DITHER))
    {
        if(strcmp(env_get(DEMURA_ENV_DITHER), "0")==0)
        {
            return DEMURA_ROUNDING;         //  2 bit , 0 : rounding , 1 : dither , 2~3 : truncate
        }
        else if(strcmp(env_get(DEMURA_ENV_DITHER), "2")==0)
        {
            return DEMURA_TRUNCATE;         //  2 bit , 0 : rounding , 1 : dither , 2~3 : truncate
        }
        else
        {
            return DEMURA_DITHER;         //  2 bit , 0 : rounding , 1 : dither , 2~3 : truncate
        }
    }
    else
    {
        return DEMURA_DITHER;         //  2 bit , 0 : rounding , 1 : dither , 2~3 : truncate
    }
}

static MS_BOOL Gen_Sf_Signature(char* strbuf, MS_U32 strlen)
{
    if ((get_demura_act() == E_MS_UTIL_BIN_ACT_ON) || (get_demura_act() == E_MS_UTIL_BIN_ACT_NO_DLG))
    {
#if defined(CONFIG_DEMURA_VENDOR_BACKLIGHT)
        if (E_MS_UTIL_BIN_FILE_BACKLIGHT == get_demura_file())
        {
            if (env_get(ENV_DEMURA_SIG_BL) == NULL)
            {
                UBOOT_TRACE("creat backlight signature to env: %s\n", strbuf);
                env_set(ENV_DEMURA_SIG_BL, strbuf);
#if CONFIG_DEMURA_ENV_SAVEENV
                env_save();
#endif
            }
            else if (strncmp(strbuf, env_get(ENV_DEMURA_SIG_BL), strlen) != 0)
            {
                UBOOT_TRACE("write backlight signature to env: %s\n", strbuf);
                env_set(ENV_DEMURA_SIG_BL, strbuf);
#if CONFIG_DEMURA_ENV_SAVEENV
                env_save();
#endif
            }
        }
        else
        {
            if (env_get(ENV_DEMURA_SIG) == NULL)
            {
                UBOOT_TRACE("creat signature to env: %s\n", strbuf);
                env_set(ENV_DEMURA_SIG, strbuf);
#if CONFIG_DEMURA_ENV_SAVEENV
                env_save();
#endif
            }
            else if (strncmp(strbuf, env_get(ENV_DEMURA_SIG), strlen) != 0)
            {
                UBOOT_TRACE("write signature to env: %s\n", strbuf);
                env_set(ENV_DEMURA_SIG, strbuf);
#if CONFIG_DEMURA_ENV_SAVEENV
                env_save();
#endif
            }
        }
#else
        if (env_get(ENV_DEMURA_SIG) == NULL)
        {
            UBOOT_TRACE("creat signature to env: %s\n", strbuf);
            env_set(ENV_DEMURA_SIG, strbuf);
#if CONFIG_DEMURA_ENV_SAVEENV
            env_save();
#endif
        }
        else if (strncmp(strbuf, env_get(ENV_DEMURA_SIG), strlen) != 0)
        {
            UBOOT_TRACE("write signature to env: %s\n", strbuf);
            env_set(ENV_DEMURA_SIG, strbuf);
#if CONFIG_DEMURA_ENV_SAVEENV
            env_save();
#endif
        }
#endif
    }
    return TRUE;
}

extern E_COLOR_MODE Demura_Original_Mode;
#if defined (CONFIG_DEMURA_URSA11)
#else
static MS_BOOL INX_set_u13_interface(Vendor2Mstar_Info *pv2m, void *pDataInfo, BinOutputInfo *pbin_info)
{
    int n_Hnode     = pv2m->hblock_num;       // 481;
    int n_Vnode     = pv2m->vblock_num;       // 271;
    int hblock_size = pv2m->hblock_size;      //   8;
    int vblock_size = pv2m->vblock_size;      //   8;
    int level_count = pv2m->layer_count;
    interface_info *pInfo_Out = (interface_info *)pDataInfo;

    pInfo_Out->reg_dmc_plane_num              = level_count;                    //  4 bit
    pInfo_Out->reg_dmc_h_block                = hblock_size > 4 ? 0x03 : 0x02;  //  3 bit
    pInfo_Out->reg_dmc_v_block                = vblock_size > 4 ? 0x03 : 0x02;  //  3 bit
    pInfo_Out->reg_dmc_rgb_mode               = 0x0;         //  1 bit
    pInfo_Out->reg_dmc_panel_h_size           = 0xF00;       // 13 bit
    pInfo_Out->reg_dmc_black_limit            = pv2m->bot_limit * 16;       // 12 bit , format 10.2
    pInfo_Out->reg_dmc_plane_level1           = pv2m->layer_levels[0] * 16;     // 12 bit , format 10.2
    pInfo_Out->reg_dmc_plane_level2           = pv2m->layer_levels[1] * 16;     // 12 bit , format 10.2
    pInfo_Out->reg_dmc_plane_level3           = pv2m->layer_levels[2] * 16;     // 12 bit , format 10.2
    pInfo_Out->reg_dmc_plane_level4           = pv2m->layer_levels[3] * 16;     // 12 bit , format 10.2
    pInfo_Out->reg_dmc_plane_level5           = pv2m->layer_levels[4] * 16;     // 12 bit , format 10.2
    pInfo_Out->reg_dmc_plane_level6           = 0;           // 12 bit , format 10.2
    pInfo_Out->reg_dmc_plane_level7           = 0;           // 12 bit , format 10.2
    pInfo_Out->reg_dmc_plane_level8           = 0;           // 12 bit , format 10.2
    pInfo_Out->reg_dmc_white_limit            = pv2m->top_limit * 16;       // 12 bit , format 10.2
    pInfo_Out->reg_dmc_dither_en              = get_env_demura_dither();
    pInfo_Out->reg_dmc_dither_rst_e_4_frame   = 0x0;         //  1 bit
    pInfo_Out->reg_dmc_dither_pse_rst_num     = 0x0;         //  2 bit

    // Remember to set !
    pbin_info->Sep_type                       = pInfo_Out->reg_dmc_rgb_mode;

    int idx;
    for (idx = 0; idx < level_count; idx++)
    {
        int nLayer = 0;
        switch (idx)
        {
            case 0:
                nLayer = pInfo_Out->reg_dmc_plane_level1 / 4;
                break;
            case 1:
                nLayer = pInfo_Out->reg_dmc_plane_level2 / 4;
                break;
            case 2:
                nLayer = pInfo_Out->reg_dmc_plane_level3 / 4;
                break;
            case 3:
                nLayer = pInfo_Out->reg_dmc_plane_level4 / 4;
                break;
            case 4:
                nLayer = pInfo_Out->reg_dmc_plane_level5 / 4;
                break;
            case 5:
                nLayer = pInfo_Out->reg_dmc_plane_level6 / 4;
                break;
            case 6:
                nLayer = pInfo_Out->reg_dmc_plane_level7 / 4;
                break;
            case 7:
                nLayer = pInfo_Out->reg_dmc_plane_level8 / 4;
                break;
            default:
                break;
        }
        int **layer_value_lut = pv2m->LutIn[idx];

        int i, j, idx_image_size = 0;
        for (i = 0; i < n_Vnode; i++)
        {
            for (j = 0; j < n_Hnode ; j++)
            {
                //(*pInfo_Out).Lut_in[idx][idx_image_size].dbr   = layer_value_lut[i][j] + nLayer;
                //(*pInfo_Out).Lut_in[idx][idx_image_size].dbg   = layer_value_lut[i][j] + nLayer;
                //(*pInfo_Out).Lut_in[idx][idx_image_size].dbb   = layer_value_lut[i][j] + nLayer;
                //#if (defined (CONFIG_DEMURA_M7622) || defined (CONFIG_DEMURA_M7632))
                //(*pInfo_Out).Lut_in[idx][idx_image_size].dbw   = layer_value_lut[i][j] + nLayer;
                //#endif

                (*pInfo_Out).Lut_in[idx][idx_image_size].r   = (int)(layer_value_lut[i][j] + nLayer);
                (*pInfo_Out).Lut_in[idx][idx_image_size].g   = (int)(layer_value_lut[i][j] + nLayer);
                (*pInfo_Out).Lut_in[idx][idx_image_size].b   = (int)(layer_value_lut[i][j] + nLayer);
                #if (defined (CONFIG_DEMURA_M7622) || defined (CONFIG_DEMURA_M7632))
                (*pInfo_Out).Lut_in[idx][idx_image_size].w   = layer_value_lut[i][j] + nLayer;
                #endif
                idx_image_size += 1;
            }
        }
    }
    return TRUE;
}
#endif

MS_BOOL INX_Decode_To_Mstar_Format(Vendor2Mstar_Info *pv2m, void *pDataInfo, BinOutputInfo *pbin_info)
{
    MS_BOOL bRet = FALSE;
    DeCmpxInfo cmpx;
    MS_U32 sample_crc32;
    MS_S32 strlen;
    char  strbuf[1024];

    int    n_Hnode         =  pv2m->hblock_num;
    int    n_Vnode         =  pv2m->vblock_num;
    MS_U32 length          =  pv2m->data_length;
    MS_U32 *buffer         =  pv2m->input_buf;

    struct array_dim2 _LutOut1;
    struct array_dim2 _LutOut2;
    struct array_dim2 _LutOut3;
    struct array_dim2 _LutOut4;
    struct array_dim2 _LutOut5;

    pbin_info->HNode       =  n_Hnode;
    pbin_info->VNode       =  n_Vnode;
    pbin_info->Blk_h_size  =  pv2m->hblock_size;
    pbin_info->Blk_v_size  =  pv2m->vblock_size;
    pbin_info->LevelCount  =  pv2m->layer_count;

    if (buffer == NULL)
    {
        UBOOT_ERROR("Error : INX Compress data input buffer == NULL !\n");
        return FALSE;
    }

    // ***************** Prepare Ouput Buffer *****************
    if(DEFINE_ARRAY_DIM2(_LutOut1, sizeof(int), n_Vnode, n_Hnode)!=TRUE)
    {
        return FALSE;
    }
    if(DEFINE_ARRAY_DIM2(_LutOut2, sizeof(int), n_Vnode, n_Hnode)!=TRUE)
    {
        FREE_ARRAY_DIM2(_LutOut1);
        return FALSE;
    }
    if(DEFINE_ARRAY_DIM2(_LutOut3, sizeof(int), n_Vnode, n_Hnode)!=TRUE)
    {
        FREE_ARRAY_DIM2(_LutOut1);
        FREE_ARRAY_DIM2(_LutOut2);
        return FALSE;
    }
    if(DEFINE_ARRAY_DIM2(_LutOut4, sizeof(int), n_Vnode, n_Hnode)!=TRUE)
    {
        FREE_ARRAY_DIM2(_LutOut1);
        FREE_ARRAY_DIM2(_LutOut2);
        FREE_ARRAY_DIM2(_LutOut3);
        return FALSE;
    }
    if(DEFINE_ARRAY_DIM2(_LutOut5, sizeof(int), n_Vnode, n_Hnode)!=TRUE)
    {
        FREE_ARRAY_DIM2(_LutOut1);
        FREE_ARRAY_DIM2(_LutOut2);
        FREE_ARRAY_DIM2(_LutOut3);
        FREE_ARRAY_DIM2(_LutOut4);
        return FALSE;
    }

    // Enter decompress
    memset(&cmpx, 0, sizeof(cmpx));
    cmpx.Height      = n_Vnode;
    cmpx.Width       = n_Hnode;
    cmpx.ByteCmpxIn  = buffer;
    cmpx.CmpxSize    = length;
    cmpx.LutOut[0]   = (int **)PTR_ARRAY_DIM2(_LutOut1);
    cmpx.LutOut[1]   = (int **)PTR_ARRAY_DIM2(_LutOut2);
    cmpx.LutOut[2]   = (int **)PTR_ARRAY_DIM2(_LutOut3);
    cmpx.LutOut[3]   = (int **)PTR_ARRAY_DIM2(_LutOut4);
    cmpx.LutOut[4]   = (int **)PTR_ARRAY_DIM2(_LutOut5);

    DeCmpx(&cmpx);
    printf("Decompress Done !\n");
    memcpy(pv2m->LutIn, cmpx.LutOut, sizeof(cmpx.LutOut));

    // Sign spi flash data
    sample_crc32 = MDrv_CRC32_Cal_DeMura((MS_U8 *)buffer + SAMPLE_DAT_START, SAMPLE_DAT_LEN);
    if (sample_crc32 == 0xffffFFFF)
    {
        UBOOT_ERROR("MDrv_CRC32_Cal_DeMura error\n");
        dfree(buffer);
        goto done;
    }
    memset(strbuf, 0, sizeof(strbuf));
    strlen = snprintf(strbuf, sizeof(strbuf)-1, "0x%x:0x%x", (unsigned int)pv2m->data_checksum, (unsigned int)sample_crc32);
    if (strlen < 0)
    {
        return FALSE;
    }

    Gen_Sf_Signature(strbuf, strlen);

    dfree(buffer);
    pv2m->input_buf = NULL;

    // alloc Lut_in buffer
    bRet = Alloc_LutIn_Space(pDataInfo, pbin_info);
    if (bRet == FALSE)
    {
        UBOOT_ERROR("Alloc_LutIn_Space error\n");
        goto done;
    }

    // set interface_info
#if defined (CONFIG_DEMURA_URSA11)
#else
    bRet = INX_set_u13_interface(pv2m, pDataInfo, pbin_info);
    if (bRet == FALSE)
    {
        UBOOT_ERROR("set_u13_interface error\n");
    }
#endif
done:
    FREE_ARRAY_DIM2(_LutOut1);
    FREE_ARRAY_DIM2(_LutOut2);
    FREE_ARRAY_DIM2(_LutOut3);
    FREE_ARRAY_DIM2(_LutOut4);
    FREE_ARRAY_DIM2(_LutOut5);
    memset(pv2m->LutIn, 0, sizeof(pv2m->LutIn));    // Clean up

    return bRet;
}


MS_BOOL INX_get_demura_header(INX_Demura_Header *phdr)
{
    MS_BOOL bRet = FALSE;
    MS_U8 buf[INX_HEADER_MAX_SIZE] = {0};

#if(INX_DEMURA_DL_FLOW)
    MS_U8 count = 0;
    // Legality check > 20 times => return FALSE
    do
    {
        count++;

        bRet = read_spi_flash(buf, 0, INX_HEADER_MAX_SIZE);
        if (bRet == FALSE)
        {
            UBOOT_ERROR("Read INX Demura from spi flash failed !\n");
        }

        bRet = parse_vendor_header_inx(buf, phdr);
        if (bRet != TRUE)
        {
            UBOOT_ERROR("parse_vendor_header\n");
        }

        UBOOT_DEBUG("Legality Check time = %d\n", count);
    }while((count <= 20) && (bRet != TRUE));

    if (bRet != TRUE)
    {
        UBOOT_ERROR("Legality Check Failed! \n");
        return FALSE;
    }
#else
    bRet = read_spi_flash(buf, 0, INX_HEADER_MAX_SIZE);
    if (bRet == FALSE)
    {
        UBOOT_ERROR("Read INX Demura from spi flash failed !\n");
        return FALSE;
    }

    bRet = parse_vendor_header_inx(buf, phdr);
    if (bRet != TRUE)
    {
        UBOOT_ERROR("parse_vendor_header\n");
        return FALSE;
    }
#endif

    UBOOT_DEBUG("\nBuild Date : %x/%02x/%02x\n", (uint)((phdr->date>>16)&0xFFFF), \
                 (uint)((phdr->date>>8)&0xFF), (uint)(phdr->date & 0xFF));
    return TRUE;
}


static MS_BOOL INX_Load_Decode_Info(void)
{
    INX_Demura_Header header;

    memset(&dec_info, 0, sizeof(dec_info));
    if (init_spi_flash() != TRUE)
    {
        UBOOT_DEBUG("init_spi_flash error!\n");
        return FALSE;
    }
    if (INX_get_demura_header(&header) != TRUE)
    {
        UBOOT_DEBUG("get_demura_header error!\n");
        return FALSE;
    }
    if (fetch_decode_info_inx(&header, &dec_info) != TRUE)
    {
        UBOOT_DEBUG("fetch_decode_info error\n");
        return FALSE;
    }

    UBOOT_TRACE("OK\n");
    return TRUE;
}


static void INX_Dump_Decode_Info(void)
{
    uint i;
    // Print data information
    UBOOT_INFO("%s Layer count : %d, Bot Limit : %d, Top Limit : %d\n", CONFIG_DEMURA_VENDOR_STRING, \
                (int)(dec_info.layer_count), (int)dec_info.bot_limit, (int)dec_info.top_limit);
    UBOOT_INFO("%s Layer value :", CONFIG_DEMURA_VENDOR_STRING);
    for (i = 0; i < dec_info.layer_count; i++)
    {
        printf(" %d", (int)(dec_info.layer_levels[i]));
    }
    printf("\n");
}


MS_BOOL Decode_To_Mstar_Format_INX(void *pDataInfo, BinOutputInfo *pbin_info)
{
    MS_BOOL bRet = FALSE;

    //=============================== read setting ===============================
    if (Check_MuraTable_INX() != TRUE)
    {
        UBOOT_DEBUG("Load_Decode_Info error, Unable to Decode_To_Mstar_Format\n");
        return FALSE;
    }
    INX_Dump_Decode_Info();

    // Decode Vendor data
    bRet = INX_Decode_To_Mstar_Format(&dec_info, pDataInfo, pbin_info);
    if (bRet == FALSE)
    {
        UBOOT_ERROR("INX_Decode_To_Mstar_Format error\n");
        return FALSE;
    }

    //MS_BOOL demura_enable[3]  = {TRUE, TRUE, TRUE};

    pbin_info->Build_Date     =  dec_info.build_date;
    pbin_info->reg_base_addr  =  0x7700;
    pbin_info->project_id     =  ((MSTAR_CHIP_ID << 16) + (1ULL << 15) + EN_DEMURA_MULTI_INX);
    pbin_info->data_type      =  IC_DRAM;
    pbin_info->Gain_type      =  10;

    //memcpy(pbin_info->channel_enable, demura_enable, sizeof(demura_enable));

    return TRUE;
}


MS_BOOL If_Need_Decode_INX(void)
{
    UBOOT_TRACE("IN\n");
    if (INX_Load_Decode_Info() != TRUE)
    {
        UBOOT_DEBUG("Load_Decode_Info error, Unable to decode data\n");
        return FALSE;
    }

#if defined(CONFIG_DEMURA_VENDOR_BACKLIGHT)
    char *sig_str;
    if (E_MS_UTIL_BIN_FILE_BACKLIGHT == get_demura_file())
    {
        sig_str = env_get(ENV_DEMURA_SIG_BL);
    }
    else
    {
        sig_str = env_get(ENV_DEMURA_SIG);
    }
#else
    char *sig_str = env_get(ENV_DEMURA_SIG);
#endif

    if (sig_str == NULL)
    {
        INX_Dump_Decode_Info();
        return TRUE;
    }
    else
    {
        MS_U32 blk_checksum;
        MS_U32 sample_crc32, cal_crc32;
        MS_U8  buf[SAMPLE_DAT_LEN];

        UBOOT_DEBUG("sig_str = %s\n", sig_str);
        if(2 == sscanf(sig_str, "0x%x:0x%x", &blk_checksum, &sample_crc32))
        {
            UBOOT_DEBUG("blk_checksum = 0x%x\n", (uint)blk_checksum);
            UBOOT_DEBUG("sample_crc32 = 0x%x\n", (uint)sample_crc32);

            if (read_spi_flash(buf, (dec_info.data_start + SAMPLE_DAT_START), SAMPLE_DAT_LEN) == TRUE)
            {
                cal_crc32 = MDrv_CRC32_Cal_DeMura(buf, SAMPLE_DAT_LEN);
                UBOOT_DEBUG("cal_crc32 = 0x%x\n", (uint)cal_crc32);
                if ((sample_crc32 == cal_crc32) && (dec_info.data_checksum == blk_checksum))
                {
                    UBOOT_DEBUG("Data Match, No Need to decode again\n");
                    UBOOT_TRACE("OK\n");
                    return FALSE;
                }
            }
        }

        INX_Dump_Decode_Info();
        UBOOT_TRACE("OK\n");
        return TRUE;
    }
}


MS_BOOL Check_MuraTable_INX(void)
{
    MS_U32 length, offset;
    MS_U32 cal_sum;
    MS_U8  *bufTemp;
    MS_U32 *buffer;
    MS_BOOL bRet = FALSE;
#if(INX_DEMURA_DL_FLOW)
    MS_U8 count = 0;
#endif

    UBOOT_TRACE("IN\n");
    if (INX_Load_Decode_Info() != TRUE)
    {
        UBOOT_ERROR("Load_Decode_Info error, \033[31mCheck_MuraTable --> Failed\033[0m\n");
        UBOOT_TRACE("OK\n");
        return FALSE;
    }
    length = dec_info.data_length;
    offset = dec_info.data_start;

    bufTemp = (MS_U8 *)dmalloc(length + 4);
    CHECK_DMALLOC_SPACE(bufTemp, (uint)(length + 4));
    buffer  = (MS_U32 *)ALIGN((size_t)bufTemp, 4);

#if(INX_DEMURA_DL_FLOW)
    // Demura Enable Condition Check > 3 times => return FALSE
    do
    {
        count++;

        bRet = read_spi_flash((MS_U8 *)buffer, offset, length);
        if (bRet == FALSE)
        {
            UBOOT_ERROR("Read Data block from spi flash failed !\n");
        }

        // Calculate checksum
        cal_sum = inx_crc8((MS_U8 *)buffer, INX_CRC8_SEED, 0, length);
        UBOOT_DEBUG("Block calculate checksum = 0x%x, checksum = 0x%x\n", (uint)cal_sum, (uint)dec_info.data_checksum);
        if (cal_sum != dec_info.data_checksum)
        {
            UBOOT_ERROR("Data Checksum Error. Calculate(0x%x) != Checksum(0x%x)\n", (uint)cal_sum, (uint)dec_info.data_checksum);
        }
    }while((count <= 3) && (bRet != TRUE));

    if(bRet != TRUE)
    {
        UBOOT_ERROR("Demura Enable Condition Check Failed !\n");
        dfree(buffer);
        return FALSE;
    }

    if ((dec_info.function_define & BIT0) == 0)
    {
        UBOOT_ERROR("Header Disable Demura! \n");
        return FALSE;
    }
#else
    bRet = read_spi_flash((MS_U8 *)buffer, offset, length);
    if (bRet == FALSE)
    {
        UBOOT_ERROR("Read Data block from spi flash failed !\n");
        dfree(buffer);
        return FALSE;
    }

    // Calculate checksum
    cal_sum = inx_crc8((MS_U8 *)buffer, INX_CRC8_SEED, 0, length);
    UBOOT_DEBUG("Block calculate checksum = 0x%x, checksum = 0x%x\n", (uint)cal_sum, (uint)dec_info.data_checksum);
    if (cal_sum != dec_info.data_checksum)
    {
        UBOOT_ERROR("Data Checksum Error. Calculate(0x%x) != Checksum(0x%x)\n", (uint)cal_sum, (uint)dec_info.data_checksum);
        dfree(buffer);
        return FALSE;
    }
#endif

    // Keep block data, avoid to read it from spi_flash again.
    dec_info.input_buf = buffer;
    UBOOT_TRACE("OK\n");
    return TRUE;
}

// INX End

// CSOT_HIMAX Start
static MS_BOOL Gen_Sf_Signature_CSOT_HIMAX(CSOT_Himax_Demura_Header *phdr)
{
    char   strbuf[1024];
    MS_U32 sample_crc32;
    MS_S32 strlen;
    MS_U8  *buffer;

    buffer = phdr->lut_buffer;

    sample_crc32 = MDrv_CRC32_Cal_DeMura(buffer + SAMPLE_DAT_START, SAMPLE_DAT_LEN);
    if (sample_crc32 == 0xffffFFFF)
    {
        UBOOT_ERROR("MDrv_CRC32_Cal_DeMura error\n");
        return FALSE;
    }
    memset(strbuf, 0, sizeof(strbuf));
    strlen = snprintf(strbuf, sizeof(strbuf)-1, "0x%x:0x%x", (unsigned int)phdr->TABLE_CRC, (unsigned int)sample_crc32);
    if (strlen < 0)
    {
        return FALSE;
    }

    Gen_Sf_Signature(strbuf, strlen);
    return TRUE;
}

MS_BOOL If_Need_Decode_CSOT_HIMAX(void)
{
    UBOOT_TRACE("IN\n");
#if defined(CONFIG_DEMURA_VENDOR_BACKLIGHT)
    char *sig_str;
    if (E_MS_UTIL_BIN_FILE_BACKLIGHT == get_demura_file())
    {
        sig_str = env_get(ENV_DEMURA_SIG_BL);
    }
    else
    {
        sig_str = env_get(ENV_DEMURA_SIG);
    }
#else
    char *sig_str = env_get(ENV_DEMURA_SIG);
#endif

    if (sig_str == NULL)
    {
        UBOOT_DEBUG("Empty Board, should decoding data\n");
        UBOOT_TRACE("OK\n");
        return TRUE;
    }
    else
    {
        MS_U16 lut_checksum_sf;
        MS_U32 lut_checksum_bd;
        MS_U32 sample_crc32, cal_crc32;
        MS_U8  buf[SAMPLE_DAT_LEN];

        lut_checksum_sf = get_lut_checksum_csot_himax();

        UBOOT_DEBUG("sig_str = %s\n", sig_str);
        if(2 == sscanf(sig_str, "0x%x:0x%x", &lut_checksum_bd, &sample_crc32))
        {
            UBOOT_TRACE("lut_checksum_sf = 0x%x\n", (uint)lut_checksum_sf);
            UBOOT_TRACE("lut_checksum_bd = 0x%x\n", (uint)lut_checksum_bd);
            UBOOT_TRACE("sample_crc32  = 0x%x\n",   (uint)sample_crc32);

            if (read_spi_flash(buf, (CSOT_HIMAX_LUT_START + SAMPLE_DAT_START), SAMPLE_DAT_LEN) == TRUE)
            {
                cal_crc32 = MDrv_CRC32_Cal_DeMura(buf, SAMPLE_DAT_LEN);
                UBOOT_DEBUG("cal_crc32     = 0x%x\n", (uint)cal_crc32);
                if ((sample_crc32 == cal_crc32) && (lut_checksum_sf == lut_checksum_bd))
                {
                    UBOOT_TRACE("Data Match, No Need to decode again\n");
                    UBOOT_TRACE("OK\n");
                    return FALSE;
                }
            }
        }

        UBOOT_TRACE("OK\n");
        return TRUE;
    }

    UBOOT_TRACE("OK\n");
    return FALSE;
}
#if defined (CONFIG_DEMURA_URSA11)
static void CSOT_Himax_Init_U11_Param(CSOT_Himax_Demura_Header *phdr, interface_info *pDataInfo)
{
    interface_info *DataInfo   = (interface_info *)pDataInfo;
    interface_info *R_Info_Out = &(DataInfo[0]);
    interface_info *G_Info_Out = &(DataInfo[1]);
    interface_info *B_Info_Out = &(DataInfo[2]);

    int channel;
    for (channel = 0; channel < 3; channel++)
    {
        DataInfo[channel].bOut_dither_en = TRUE;
    }

    (*R_Info_Out).iChannel = 0;
    (*G_Info_Out).iChannel = 1;
    (*B_Info_Out).iChannel = 2;

    int layer_level_num      = CSOT_HIMAX_USE_PLANE_NUM;//MAX_PLANE_NUM

    layer_level_num = ((layer_level_num>=1)&&(layer_level_num<=4)) ? layer_level_num : 3;

    int idx;
    for (idx = 0; idx < 6; idx++)
    {
        (*R_Info_Out).bLayer_level_en[idx] = FALSE;
        (*G_Info_Out).bLayer_level_en[idx] = FALSE;
        (*B_Info_Out).bLayer_level_en[idx] = FALSE;
    }

    switch(layer_level_num)
    {
        case 0:
            break;
        case 1:
        case 2:
        case 3:
        case 4:
            for (idx = 0; idx < layer_level_num + 2; idx++)
            {
                (*R_Info_Out).bLayer_level_en[idx] = TRUE;
                (*G_Info_Out).bLayer_level_en[idx] = TRUE;
                (*B_Info_Out).bLayer_level_en[idx] = TRUE;
            }
            break;
        default:
            break;
    }

    for (idx = 0; idx < 6; idx++)
    {
        (*R_Info_Out).iLayer_level[idx] = ((idx!=0) ? 1023 : 0);
        (*G_Info_Out).iLayer_level[idx] = ((idx!=0) ? 1023 : 0);
        (*B_Info_Out).iLayer_level[idx] = ((idx!=0) ? 1023 : 0);
    }
    int black_level    = phdr->LOW_BOUND; //12 bits
    int layer_level1   = phdr->PLANE0_LV;//12bits
    int layer_level2   = phdr->PLANE1_LV;
    int layer_level3   = phdr->PLANE2_LV;
    int layer_level4   = phdr->PLANE3_LV;
    int white_level    = phdr->HIGH_BOUND;

    black_level  = ((black_level + 2)>>2);   // 12 bit -> 10 bit
    layer_level1 = ((layer_level1 + 2)>>2);  // 12 bit -> 10 bit
    layer_level2 = ((layer_level2 + 2)>>2);  // 12 bit -> 10 bit
    layer_level3 = ((layer_level3 + 2)>>2);  // 12 bit -> 10 bit
    layer_level4 = ((layer_level4 + 2)>>2);  // 12 bit -> 10 bit
    white_level  = ((white_level + 2)>>2);   // 12 bit -> 10 bit

    switch(layer_level_num)
    {
    case 1:
        (*R_Info_Out).iLayer_level[0] = black_level;
        (*R_Info_Out).iLayer_level[1] = layer_level1;
        (*R_Info_Out).iLayer_level[2] = white_level;

        (*G_Info_Out).iLayer_level[0] = black_level;
        (*G_Info_Out).iLayer_level[1] = layer_level1;
        (*G_Info_Out).iLayer_level[2] = white_level;

        (*B_Info_Out).iLayer_level[0] = black_level;
        (*B_Info_Out).iLayer_level[1] = layer_level1;
        (*B_Info_Out).iLayer_level[2] = white_level;
        break;
    case 2:
        (*R_Info_Out).iLayer_level[0] = black_level;
        (*R_Info_Out).iLayer_level[1] = layer_level1;
        (*R_Info_Out).iLayer_level[2] = layer_level2;
        (*R_Info_Out).iLayer_level[3] = white_level;

        (*G_Info_Out).iLayer_level[0] = black_level;
        (*G_Info_Out).iLayer_level[1] = layer_level1;
        (*G_Info_Out).iLayer_level[2] = layer_level2;
        (*G_Info_Out).iLayer_level[3] = white_level;

        (*B_Info_Out).iLayer_level[0] = black_level;
        (*B_Info_Out).iLayer_level[1] = layer_level1;
        (*B_Info_Out).iLayer_level[2] = layer_level2;
        (*B_Info_Out).iLayer_level[3] = white_level;
        break;
    case 3:
        (*R_Info_Out).iLayer_level[0] = black_level;
        (*R_Info_Out).iLayer_level[1] = layer_level1;
        (*R_Info_Out).iLayer_level[2] = layer_level2;
        (*R_Info_Out).iLayer_level[3] = layer_level3;
        (*R_Info_Out).iLayer_level[4] = white_level;

        (*G_Info_Out).iLayer_level[0] = black_level;
        (*G_Info_Out).iLayer_level[1] = layer_level1;
        (*G_Info_Out).iLayer_level[2] = layer_level2;
        (*G_Info_Out).iLayer_level[3] = layer_level3;
        (*G_Info_Out).iLayer_level[4] = white_level;

        (*B_Info_Out).iLayer_level[0] = black_level;
        (*B_Info_Out).iLayer_level[1] = layer_level1;
        (*B_Info_Out).iLayer_level[2] = layer_level2;
        (*B_Info_Out).iLayer_level[3] = layer_level3;
        (*B_Info_Out).iLayer_level[4] = white_level;
        break;
    case 4:
        (*R_Info_Out).iLayer_level[0] = black_level;
        (*R_Info_Out).iLayer_level[1] = layer_level1;
        (*R_Info_Out).iLayer_level[2] = layer_level2;
        (*R_Info_Out).iLayer_level[3] = layer_level3;
        (*R_Info_Out).iLayer_level[4] = layer_level4;
        (*R_Info_Out).iLayer_level[5] = white_level;

        (*G_Info_Out).iLayer_level[0] = black_level;
        (*G_Info_Out).iLayer_level[1] = layer_level1;
        (*G_Info_Out).iLayer_level[2] = layer_level2;
        (*G_Info_Out).iLayer_level[3] = layer_level3;
        (*G_Info_Out).iLayer_level[4] = layer_level4;
        (*G_Info_Out).iLayer_level[5] = white_level;

        (*B_Info_Out).iLayer_level[0] = black_level;
        (*B_Info_Out).iLayer_level[1] = layer_level1;
        (*B_Info_Out).iLayer_level[2] = layer_level2;
        (*B_Info_Out).iLayer_level[3] = layer_level3;
        (*B_Info_Out).iLayer_level[4] = layer_level4;
        (*B_Info_Out).iLayer_level[5] = white_level;
        break;
    }
    //++++++++++++ layerX level end ++++++++++++//

    //++++++++++++ low luma weight start ++++++++++++//
    (*R_Info_Out).bLow_luma_en         = FALSE;
    (*R_Info_Out).iLow_luma_thrd       = 0x00;
    (*R_Info_Out).iLow_luma_slope      = 0x0F;
    (*R_Info_Out).iLow_luma_min_weight = 0x0;

    (*G_Info_Out).bLow_luma_en         = FALSE;
    (*G_Info_Out).iLow_luma_thrd       = 0x00;
    (*G_Info_Out).iLow_luma_slope      = 0x0F;
    (*G_Info_Out).iLow_luma_min_weight = 0x0;

    (*B_Info_Out).bLow_luma_en         = FALSE;
    (*B_Info_Out).iLow_luma_thrd       = 0x00;
    (*B_Info_Out).iLow_luma_slope      = 0x0F;
    (*B_Info_Out).iLow_luma_min_weight = 0x0;
    //++++++++++++ low luma weight end ++++++++++++//

    //++++++++++++ high luma weight start ++++++++++++//
    (*R_Info_Out).bHigh_luma_en         = FALSE;
    (*R_Info_Out).iHigh_luma_thrd       = 0xFF;
    (*R_Info_Out).iHigh_luma_slope      = 0x0F;
    (*R_Info_Out).iHigh_luma_min_weight = 0x0;

    (*G_Info_Out).bHigh_luma_en         = FALSE;
    (*G_Info_Out).iHigh_luma_thrd       = 0xFF;
    (*G_Info_Out).iHigh_luma_slope      = 0x0F;
    (*G_Info_Out).iHigh_luma_min_weight = 0x0;

    (*B_Info_Out).bHigh_luma_en         = FALSE;
    (*B_Info_Out).iHigh_luma_thrd       = 0xFF;
    (*B_Info_Out).iHigh_luma_slope      = 0x0F;
    (*B_Info_Out).iHigh_luma_min_weight = 0x0;
    //++++++++++++ high luma weight end ++++++++++++//

    for (idx = 0; idx < 6; idx++)
    {
        int cnt_idx;
        for (cnt_idx = 0; cnt_idx < (phdr->DEMURA_TBL_H* phdr->DEMURA_TBL_V); cnt_idx++)
        {
            (*R_Info_Out).Lut_in[idx][cnt_idx] = 0.0;
            (*G_Info_Out).Lut_in[idx][cnt_idx] = 0.0;
            (*B_Info_Out).Lut_in[idx][cnt_idx] = 0.0;
        }
    }
/*
    for(idx = 0; idx < 10; idx++)
        printf("Lut = %f\t", (*R_Info_Out).Lut_in[0][idx]);
    printf("\n");
*/
}
static MS_BOOL CSOT_Himax_Set_U11_Format(CSOT_Himax_Demura_Header *phdr, interface_info *pinfo)
{
    interface_info *DataInfo   = (interface_info *)pinfo;
    interface_info *R_Info_Out = &(DataInfo[0]);
    interface_info *G_Info_Out = &(DataInfo[1]);
    interface_info *B_Info_Out = &(DataInfo[2]);
    int i, j, k, data = 0;
    int table_addr = 0;
    int nLayer = 0, idx_image_size;
    MS_BOOL pol = 0;
    double LUT_data = 0.0;
    CSOT_Himax_Init_U11_Param(phdr, pinfo);
    // assign LUT 12 bit format
    for (i = 0; i < phdr->DEMURA_TBL_V; i++)
    {
        for(k = 0; k < CSOT_HIMAX_USE_PLANE_NUM; k++)
        {
            for (j = 0; j < (phdr->DEMURA_TBL_H+3) ; j++)
            {
                if(j == 0)
                    pol = 0;

                // LUT index for the plane, H size = 481
                idx_image_size = i*phdr->DEMURA_TBL_H+j;

                // 12 bit data
                if(pol == 0) //8bit + 4bit
                {
                    data = ((MS_U16)phdr->lut_buffer[table_addr]) | (((MS_U16)phdr->lut_buffer[table_addr + 1]&0x0f)<<8);
                    pol = 1;
                    table_addr+=1;
                }
                else
                {
                    data = (((MS_U16)phdr->lut_buffer[table_addr] & 0xf0)>>4) | (((MS_U16)phdr->lut_buffer[table_addr + 1]&0xff)<<4);
                    pol = 0;
                    table_addr+=2;
                }

                if(j > (phdr->DEMURA_TBL_H - 1)) // Skip dummy 3 nodes
                {
                }
                else
                {
                // 2048 is zero.
                data -= 2048;



                LUT_data = ((double)data/4.0); // 10bit data + layer (12bit to 10bit)

                (*R_Info_Out).Lut_in[k][idx_image_size]       = LUT_data;
                (*G_Info_Out).Lut_in[k][idx_image_size]       = LUT_data;
                (*B_Info_Out).Lut_in[k][idx_image_size]       = LUT_data;

                }
            }
        }
    }

    // Free buffer
    dfree(phdr->hdr_buffer);
    phdr->hdr_buffer = NULL;

    dfree(phdr->lut_buffer);
    phdr->lut_buffer = NULL;

    return TRUE;
}
#else
static void CSOT_Himax_Init_U13_Param(CSOT_Himax_Demura_Header *phdr, interface_info *pDataInfo)
{
    MS_U16 width, height;
    demura_get_panelinfo(&width, &height);
    CSOT_Himax_Demura_Header header;
    memcpy(&header, phdr, sizeof(CSOT_Himax_Demura_Header));
    if (width >= 7680)
    {
        pDataInfo->reg_dmc_h_block            = 4;//header.DEMURA_BLK_H;         //  3 bit
        pDataInfo->reg_dmc_v_block            = 4;//header.DEMURA_BLK_V;         //  3 bit
    }
    else
    {
        pDataInfo->reg_dmc_h_block            = 3;//header.DEMURA_BLK_H;         //  3 bit
        pDataInfo->reg_dmc_v_block            = 3;//header.DEMURA_BLK_V;         //  3 bit
    }
    pDataInfo->reg_dmc_plane_num              = CSOT_HIMAX_USE_PLANE_NUM;         //  4 bit
    pDataInfo->reg_dmc_rgb_mode               = 0;         //  1 bit
    pDataInfo->reg_dmc_panel_h_size           = 0xF00;       // 13 bit
    pDataInfo->reg_dmc_black_limit              = header.LOW_BOUND;       // 12 bit , format 10.2
    pDataInfo->reg_dmc_plane_level1           = header.PLANE0_LV;       // 12 bit , format 10.2
    pDataInfo->reg_dmc_plane_level2           = header.PLANE1_LV;       // 12 bit , format 10.2
    pDataInfo->reg_dmc_plane_level3           = header.PLANE2_LV;       // 12 bit , format 10.2
    pDataInfo->reg_dmc_plane_level4           = header.PLANE3_LV;       // 12 bit , format 10.2
    pDataInfo->reg_dmc_plane_level5           = 0;           // 12 bit , format 10.2
    pDataInfo->reg_dmc_plane_level6           = 0;           // 12 bit , format 10.2
    pDataInfo->reg_dmc_plane_level7           = 0;           // 12 bit , format 10.2
    pDataInfo->reg_dmc_plane_level8           = 0;           // 12 bit , format 10.2
    pDataInfo->reg_dmc_white_limit            = header.HIGH_BOUND;       // 12 bit , format 10.2
    pDataInfo->reg_dmc_dither_en              = get_env_demura_dither();
    pDataInfo->reg_dmc_dither_rst_e_4_frame   = 0x0;         //  1 bit
    pDataInfo->reg_dmc_dither_pse_rst_num     = 0x0;         //  2 bit
    pDataInfo->bROI_en                        = 0x0;
    pDataInfo->iROI_hor_end_offset            = 0x0;
    pDataInfo->iROI_hor_start_offset          = 0x0;
    pDataInfo->iROI_ver_end_offset            = 0x0;
    pDataInfo->iROI_ver_start_offset          = 0x0;
}
static MS_BOOL CSOT_Himax_Set_U13_Format(CSOT_Himax_Demura_Header *phdr, interface_info *pinfo)
{
    int i, j, k, data = 0;
    int table_addr = 0;
    int nLayer = 0, idx_image_size;
    MS_BOOL pol = 0;
    double LUT_data = 0.0;
    CSOT_Himax_Init_U13_Param(phdr, pinfo);
    // assign LUT 12 bit format
    for (i = 0; i < phdr->DEMURA_TBL_V; i++)
    {
        for(k = 0; k < pinfo->reg_dmc_plane_num; k++)
        {
            for (j = 0; j < (phdr->DEMURA_TBL_H+3) ; j++)
            {
                if(j == 0)
                    pol = 0;

                // LUT index for the plane, H size = 481
                idx_image_size = i*phdr->DEMURA_TBL_H+j;

                // 12 bit data
                if(pol == 0) //8bit + 4bit
                {
                    data = ((MS_U16)phdr->lut_buffer[table_addr]) | (((MS_U16)phdr->lut_buffer[table_addr + 1]&0x0f)<<8);
                    pol = 1;
                    table_addr+=1;
                }
                else
                {
                    data = (((MS_U16)phdr->lut_buffer[table_addr] & 0xf0)>>4) | (((MS_U16)phdr->lut_buffer[table_addr + 1]&0xff)<<4);
                    pol = 0;
                    table_addr+=2;
                }

                if(j > (phdr->DEMURA_TBL_H - 1)) // Skip dummy 3 nodes
                {
                }
                else
                {
                // 2048 is zero.
                data -= 2048;

                switch(k)
                {
                    case 0:
                        nLayer = pinfo->reg_dmc_plane_level1;
                        break;
                    case 1:
                        nLayer = pinfo->reg_dmc_plane_level2;
                        break;
                    case 2:
                        nLayer = pinfo->reg_dmc_plane_level3;
                        break;
                    case 3:
                        nLayer = pinfo->reg_dmc_plane_level4;
                        break;
                    case 4:
                        nLayer = pinfo->reg_dmc_plane_level5;
                        break;
                    case 5:
                        nLayer = pinfo->reg_dmc_plane_level6;
                        break;
                    case 6:
                        nLayer = pinfo->reg_dmc_plane_level7;
                        break;
                    case 7:
                        nLayer = pinfo->reg_dmc_plane_level8;
                        break;
                    default:
                        break;
                }

                LUT_data = ((double)data/4.0) + ((double)nLayer/4.0); // 10bit data + layer (12bit to 10bit)

                //pinfo->Lut_in[k][idx_image_size].dbr   = (double)LUT_data;
                //pinfo->Lut_in[k][idx_image_size].dbg   = (double)LUT_data;
                //pinfo->Lut_in[k][idx_image_size].dbb   = (double)LUT_data;
                //#if (defined (CONFIG_DEMURA_M7622) || defined (CONFIG_DEMURA_M7632))
                //pinfo->Lut_in[k][idx_image_size].dbw   = (double)LUT_data;
                //#endif

                //LUT_data = LUT_data>>4; // 14bit -> 10bit
                pinfo->Lut_in[k][idx_image_size].r   = (int)LUT_data;
                pinfo->Lut_in[k][idx_image_size].g   = (int)LUT_data;
                pinfo->Lut_in[k][idx_image_size].b   = (int)LUT_data;
                #if (defined (CONFIG_DEMURA_M7622) || defined (CONFIG_DEMURA_M7632))
                pinfo->Lut_in[k][idx_image_size].w   = (int)LUT_data;
                #endif

                }
            }
        }
    }

    // Free buffer
    dfree(phdr->hdr_buffer);
    phdr->hdr_buffer = NULL;

    dfree(phdr->lut_buffer);
    phdr->lut_buffer = NULL;

    return TRUE;
}
#endif
static MS_BOOL CSOT_Himax_Decode_To_Mstar_Format(CSOT_Himax_Demura_Header *phdr, interface_info *pinfo)
{
    MS_BOOL bRet = FALSE;

    #if defined (CONFIG_DEMURA_URSA11)
        bRet = CSOT_Himax_Set_U11_Format(phdr, pinfo);
    #else
        bRet = CSOT_Himax_Set_U13_Format(phdr, pinfo);
    #endif

    return bRet;
}



MS_BOOL Decode_To_Mstar_Format_CSOT_HIMAX(void *pDataInfo, BinOutputInfo *pbin_info)
{
    MS_BOOL bRet = FALSE;
    CSOT_Himax_Demura_Header header;

    // Parse header information
    memset(&header, 0, sizeof(header));
    if (load_vendor_header_csot_himax(&header) != TRUE)
    {
        printf("load_vendor_header Error\n");
        return FALSE;
    }

    if (parse_vendor_header_csot_himax(&header) != TRUE)
    {
        printf("parse_vendor_header Error\n");
        return FALSE;
    }

    if (load_vendor_lut_csot_himax(&header) != TRUE)
    {
        printf("load_vendor_lut Error\n");
        dfree(header.hdr_buffer);
        header.hdr_buffer = NULL;
        return FALSE;
    }

    Gen_Sf_Signature_CSOT_HIMAX(&header);
    // Alloc space for Lut_in
    header.DEMURA_TBL_H = 481;
    header.DEMURA_TBL_V = 271;
    int n_Hnode = header.DEMURA_TBL_H;
    int n_Vnode = header.DEMURA_TBL_V;



    // Init Output message
    pbin_info->HNode          = n_Hnode;
    pbin_info->VNode          = n_Vnode;
    pbin_info->LevelCount     = CSOT_HIMAX_USE_PLANE_NUM;

    pbin_info->Blk_h_size     = 1 << 3;
    pbin_info->Blk_v_size     = 1 << 3;
    //dump_interface_info(pDataInfo);

    bRet = Alloc_LutIn_Space(pDataInfo, pbin_info);
    if (bRet == FALSE)
    {
        printf("Alloc_LutIn_Space error\n");
        dfree(header.hdr_buffer);
        header.hdr_buffer = NULL;
        dfree(header.lut_buffer);
        header.lut_buffer = NULL;
        return FALSE;
    }

    // Decode Vendor data
    bRet = CSOT_Himax_Decode_To_Mstar_Format(&header, (interface_info *)pDataInfo);
    if (bRet == FALSE)
    {
        printf("CSOT_Himax_Decode_To_Mstar_Format error\n");
        dfree(header.hdr_buffer);
        header.hdr_buffer = NULL;
        dfree(header.lut_buffer);
        header.lut_buffer = NULL;
        return FALSE;
    }

/*
    #if defined (CONFIG_DEMURA_URSA13) || defined (CONFIG_DEMURA_URSA11)
    MS_BOOL demura_enable[3]  = {TRUE, TRUE, TRUE};
    #else
    MS_BOOL demura_enable[4]  = {TRUE, TRUE, TRUE, TRUE};
    #endif
*/
    pbin_info->reg_base_addr  =  0x7700;
    pbin_info->project_id     =  ((MSTAR_CHIP_ID << 16) + (1ULL << 15) + EN_DEMURA_MULTI_CSOT_HIMAX);
    pbin_info->data_type      =  IC_DRAM;
    pbin_info->Gain_type      =  10;
    #if defined (CONFIG_DEMURA_URSA13) || defined (CONFIG_DEMURA_URSA11)
    pbin_info->Sep_type       =  0;
    #else
    pbin_info->Sep_type       = 0;
    #endif
    pbin_info->Build_Date     =  0x19021814;

    //memcpy(pbin_info->channel_enable, demura_enable, sizeof(demura_enable));

    return TRUE;
}
// CSOT_HIMAX End


// SDC
static MS_BOOL Gen_Sf_Signature_SDC(SDC_Demura_Header *phdr)
{
    char   strbuf[1024];
    MS_U32 sample_crc32;
    MS_S32 strlen;
    MS_U8  *buffer;

    buffer = phdr->lut_buffer;

    sample_crc32 = MDrv_CRC32_Cal_DeMura(buffer + SAMPLE_DAT_START_SDC, SAMPLE_DAT_LEN);
    if (sample_crc32 == 0xffffFFFF)
    {
        UBOOT_ERROR("MDrv_CRC32_Cal_DeMura error\n");
        return FALSE;
    }
    memset(strbuf, 0, sizeof(strbuf));
    strlen = snprintf(strbuf, sizeof(strbuf)-1, "0x%x:0x%x", (unsigned int)phdr->TABLE_CRC, (unsigned int)sample_crc32);
    if (strlen < 0)
    {
        return FALSE;
    }

    Gen_Sf_Signature(strbuf, strlen);
    return TRUE;
}


MS_BOOL If_Need_Decode_SDC(void)
{
    UBOOT_TRACE("IN\n");
#if defined(CONFIG_DEMURA_VENDOR_BACKLIGHT)
    char *sig_str;
    if (E_MS_UTIL_BIN_FILE_BACKLIGHT == get_demura_file())
    {
        sig_str = env_get(ENV_DEMURA_SIG_BL);
    }
    else
    {
        sig_str = env_get(ENV_DEMURA_SIG);
    }
#else
    char *sig_str = env_get(ENV_DEMURA_SIG);
#endif

    if (sig_str == NULL)
    {
        UBOOT_DEBUG("Empty Board, should decoding data\n");
        UBOOT_TRACE("OK\n");
        return TRUE;
    }
    else
    {
        MS_U16 lut_checksum_sf;
        MS_U32 lut_checksum_bd;
        MS_U32 sample_crc32, cal_crc32;
        MS_U8  buf[SAMPLE_DAT_LEN];

        lut_checksum_sf = get_lut_checksum_sdc();

        UBOOT_DEBUG("sig_str = %s\n", sig_str);
        if(2 == sscanf(sig_str, "0x%x:0x%x", &lut_checksum_bd, &sample_crc32))
        {
            UBOOT_DEBUG("lut_checksum_sf = 0x%x\n", (uint)lut_checksum_sf);
            UBOOT_DEBUG("lut_checksum_bd = 0x%x\n", (uint)lut_checksum_bd);
            UBOOT_DEBUG("sample_crc32  = 0x%x\n",   (uint)sample_crc32);

            if (read_spi_flash(buf, (SDC_LUT_START + SAMPLE_DAT_START_SDC), SAMPLE_DAT_LEN) == TRUE)
            {
                cal_crc32 = MDrv_CRC32_Cal_DeMura(buf, SAMPLE_DAT_LEN);
                UBOOT_DEBUG("cal_crc32     = 0x%x\n", (uint)cal_crc32);
                if ((sample_crc32 == cal_crc32) && (lut_checksum_sf == lut_checksum_bd))
                {
                    UBOOT_DEBUG("Data Match, No Need to decode again\n");
                    UBOOT_TRACE("OK\n");
                    return FALSE;
                }
            }
        }

        UBOOT_TRACE("OK\n");
        return TRUE;
    }

    UBOOT_TRACE("OK\n");
    return FALSE;
}


#if defined (CONFIG_DEMURA_URSA11)
static void SDC_Init_U11_Param(SDC_Demura_Header *phdr, interface_info *pDataInfo)
{
    interface_info *DataInfo   = (interface_info *)pDataInfo;
    interface_info *R_Info_Out = &(DataInfo[0]);
    interface_info *G_Info_Out = &(DataInfo[1]);
    interface_info *B_Info_Out = &(DataInfo[2]);

    int channel;
    for (channel = 0; channel < 3; channel++)
    {
        DataInfo[channel].bOut_dither_en = TRUE;
    }

    (*R_Info_Out).iChannel = 0;
    (*G_Info_Out).iChannel = 1;
    (*B_Info_Out).iChannel = 2;

    int layer_level_num      = 4;//MAX_PLANE_NUM

    layer_level_num = ((layer_level_num>=1)&&(layer_level_num<=4)) ? layer_level_num : 3;

    int idx;
    for (idx = 0; idx < 6; idx++)
    {
        (*R_Info_Out).bLayer_level_en[idx] = FALSE;
        (*G_Info_Out).bLayer_level_en[idx] = FALSE;
        (*B_Info_Out).bLayer_level_en[idx] = FALSE;
    }

    switch(layer_level_num)
    {
        case 0:
            break;
        case 1:
        case 2:
        case 3:
        case 4:
            for (idx = 0; idx < layer_level_num + 2; idx++)
            {
                (*R_Info_Out).bLayer_level_en[idx] = TRUE;
                (*G_Info_Out).bLayer_level_en[idx] = TRUE;
                (*B_Info_Out).bLayer_level_en[idx] = TRUE;
            }
            break;
        default:
            break;
    }

    for (idx = 0; idx < 6; idx++)
    {
        (*R_Info_Out).iLayer_level[idx] = ((idx!=0) ? 1023 : 0);
        (*G_Info_Out).iLayer_level[idx] = ((idx!=0) ? 1023 : 0);
        (*B_Info_Out).iLayer_level[idx] = ((idx!=0) ? 1023 : 0);
    }
    int black_level    = 0x00; //8 bits
    int layer_level1   = phdr->PLANE[0];
    int layer_level2   = phdr->PLANE[1];
    int layer_level3   = phdr->PLANE[2];
    int layer_level4   = phdr->PLANE[3];
    int white_level    = 0xFF;

    black_level  = ((black_level)<<2);   // 8 bit -> 10 bit
    layer_level1 = ((layer_level1 + 2)<<2);  // 8 bit -> 10 bit
    layer_level2 = ((layer_level2 + 2)<<2);  // 8 bit -> 10 bit
    layer_level3 = ((layer_level3 + 2)<<2);  // 8 bit -> 10 bit
    layer_level4 = ((layer_level4 + 2)<<2);  // 8 bit -> 10 bit
    white_level  = ((white_level)<<4);   // 8 bit -> 10 bit

    switch(layer_level_num)
    {
    case 1:
        (*R_Info_Out).iLayer_level[0] = black_level;
        (*R_Info_Out).iLayer_level[1] = layer_level1;
        (*R_Info_Out).iLayer_level[2] = white_level;

        (*G_Info_Out).iLayer_level[0] = black_level;
        (*G_Info_Out).iLayer_level[1] = layer_level1;
        (*G_Info_Out).iLayer_level[2] = white_level;

        (*B_Info_Out).iLayer_level[0] = black_level;
        (*B_Info_Out).iLayer_level[1] = layer_level1;
        (*B_Info_Out).iLayer_level[2] = white_level;
        break;
    case 2:
        (*R_Info_Out).iLayer_level[0] = black_level;
        (*R_Info_Out).iLayer_level[1] = layer_level1;
        (*R_Info_Out).iLayer_level[2] = layer_level2;
        (*R_Info_Out).iLayer_level[3] = white_level;

        (*G_Info_Out).iLayer_level[0] = black_level;
        (*G_Info_Out).iLayer_level[1] = layer_level1;
        (*G_Info_Out).iLayer_level[2] = layer_level2;
        (*G_Info_Out).iLayer_level[3] = white_level;

        (*B_Info_Out).iLayer_level[0] = black_level;
        (*B_Info_Out).iLayer_level[1] = layer_level1;
        (*B_Info_Out).iLayer_level[2] = layer_level2;
        (*B_Info_Out).iLayer_level[3] = white_level;
        break;
    case 3:
        (*R_Info_Out).iLayer_level[0] = black_level;
        (*R_Info_Out).iLayer_level[1] = layer_level1;
        (*R_Info_Out).iLayer_level[2] = layer_level2;
        (*R_Info_Out).iLayer_level[3] = layer_level3;
        (*R_Info_Out).iLayer_level[4] = white_level;

        (*G_Info_Out).iLayer_level[0] = black_level;
        (*G_Info_Out).iLayer_level[1] = layer_level1;
        (*G_Info_Out).iLayer_level[2] = layer_level2;
        (*G_Info_Out).iLayer_level[3] = layer_level3;
        (*G_Info_Out).iLayer_level[4] = white_level;

        (*B_Info_Out).iLayer_level[0] = black_level;
        (*B_Info_Out).iLayer_level[1] = layer_level1;
        (*B_Info_Out).iLayer_level[2] = layer_level2;
        (*B_Info_Out).iLayer_level[3] = layer_level3;
        (*B_Info_Out).iLayer_level[4] = white_level;
        break;
    case 4:
        (*R_Info_Out).iLayer_level[0] = black_level;
        (*R_Info_Out).iLayer_level[1] = layer_level1;
        (*R_Info_Out).iLayer_level[2] = layer_level2;
        (*R_Info_Out).iLayer_level[3] = layer_level3;
        (*R_Info_Out).iLayer_level[4] = layer_level4;
        (*R_Info_Out).iLayer_level[5] = white_level;

        (*G_Info_Out).iLayer_level[0] = black_level;
        (*G_Info_Out).iLayer_level[1] = layer_level1;
        (*G_Info_Out).iLayer_level[2] = layer_level2;
        (*G_Info_Out).iLayer_level[3] = layer_level3;
        (*G_Info_Out).iLayer_level[4] = layer_level4;
        (*G_Info_Out).iLayer_level[5] = white_level;

        (*B_Info_Out).iLayer_level[0] = black_level;
        (*B_Info_Out).iLayer_level[1] = layer_level1;
        (*B_Info_Out).iLayer_level[2] = layer_level2;
        (*B_Info_Out).iLayer_level[3] = layer_level3;
        (*B_Info_Out).iLayer_level[4] = layer_level4;
        (*B_Info_Out).iLayer_level[5] = white_level;
        break;
    }
    //++++++++++++ layerX level end ++++++++++++//

    //++++++++++++ low luma weight start ++++++++++++//
    (*R_Info_Out).bLow_luma_en         = FALSE;
    (*R_Info_Out).iLow_luma_thrd       = 0x00;
    (*R_Info_Out).iLow_luma_slope      = 0x0F;
    (*R_Info_Out).iLow_luma_min_weight = 0x0;

    (*G_Info_Out).bLow_luma_en         = FALSE;
    (*G_Info_Out).iLow_luma_thrd       = 0x00;
    (*G_Info_Out).iLow_luma_slope      = 0x0F;
    (*G_Info_Out).iLow_luma_min_weight = 0x0;

    (*B_Info_Out).bLow_luma_en         = FALSE;
    (*B_Info_Out).iLow_luma_thrd       = 0x00;
    (*B_Info_Out).iLow_luma_slope      = 0x0F;
    (*B_Info_Out).iLow_luma_min_weight = 0x0;
    //++++++++++++ low luma weight end ++++++++++++//

    //++++++++++++ high luma weight start ++++++++++++//
    (*R_Info_Out).bHigh_luma_en         = FALSE;
    (*R_Info_Out).iHigh_luma_thrd       = 0xFF;
    (*R_Info_Out).iHigh_luma_slope      = 0x0F;
    (*R_Info_Out).iHigh_luma_min_weight = 0x0;

    (*G_Info_Out).bHigh_luma_en         = FALSE;
    (*G_Info_Out).iHigh_luma_thrd       = 0xFF;
    (*G_Info_Out).iHigh_luma_slope      = 0x0F;
    (*G_Info_Out).iHigh_luma_min_weight = 0x0;

    (*B_Info_Out).bHigh_luma_en         = FALSE;
    (*B_Info_Out).iHigh_luma_thrd       = 0xFF;
    (*B_Info_Out).iHigh_luma_slope      = 0x0F;
    (*B_Info_Out).iHigh_luma_min_weight = 0x0;
    //++++++++++++ high luma weight end ++++++++++++//

    for (idx = 0; idx < 6; idx++)
    {
        int cnt_idx;
        for (cnt_idx = 0; cnt_idx < (phdr->DEMURA_TBL_H* phdr->DEMURA_TBL_V); cnt_idx++)
        {
            (*R_Info_Out).Lut_in[idx][cnt_idx] = 0.0;
            (*G_Info_Out).Lut_in[idx][cnt_idx] = 0.0;
            (*B_Info_Out).Lut_in[idx][cnt_idx] = 0.0;
        }
    }
/*
    for(idx = 0; idx < 10; idx++)
        printf("Lut = %f\t", (*R_Info_Out).Lut_in[0][idx]);
    printf("\n");
*/
}
static MS_BOOL SDC_Set_U11_Format(SDC_Demura_Header *phdr, interface_info *pinfo)
{
    interface_info *DataInfo   = (interface_info *)pinfo;
    interface_info *R_Info_Out = &(DataInfo[0]);
    interface_info *G_Info_Out = &(DataInfo[1]);
    interface_info *B_Info_Out = &(DataInfo[2]);
    int idx, i, j, k, data = 0;
    int table_addr = 0;
    int nLayer = 0, idx_image_size;
    double Value_1 = 0.0, LUT_data = 0.0;
    SDC_Init_U11_Param(phdr, pinfo);
    // assign LUT 12 bit format
    for(k = (4 - 1); k >= 0; k--)
    {
        for (i = 0; i < phdr->DEMURA_TBL_V; i++)
        {
            for (j = 0; j < phdr->DEMURA_TBL_H ; j++)
            {
                // Update Value_1
                Value_1 = (1<<phdr->Value_1[k])*(255.0/4096.0);

                // 8 bit LUT
                data = phdr->lut_buffer[table_addr];
                if(data >= 128) // signed bit 7
                    data = data - 256;

                table_addr++;

                if((i == (phdr->DEMURA_TBL_V - 1)) && (j == (phdr->DEMURA_TBL_H - 1))) // skip last dummy 0xFF
                    table_addr++;



                LUT_data = (double)(data*4*Value_1);
                idx_image_size = i*phdr->DEMURA_TBL_H+j;

                (*R_Info_Out).Lut_in[k][idx_image_size]       = LUT_data;
                (*G_Info_Out).Lut_in[k][idx_image_size]       = LUT_data;
                (*B_Info_Out).Lut_in[k][idx_image_size]       = LUT_data;
            }
        }
    }

}
#else
void dump_interface_info_sdc(interface_info *pDataInfo)
{
    printf("PLANE_NUM = %d  \n", pDataInfo->reg_dmc_plane_num);
    printf("H_BLOCK = %d, V_BLOCK = %d  \n", pDataInfo->reg_dmc_h_block, pDataInfo->reg_dmc_v_block);
    printf("RGB_MODE = %d   \n", pDataInfo->reg_dmc_rgb_mode);
    printf("PANEL_H_SIZE = %d   \n", pDataInfo->reg_dmc_panel_h_size);
    printf("BLACK_LIMIT = 0x%x, WHITE_LIMIT = 0x%x  \n", pDataInfo->reg_dmc_black_limit, pDataInfo->reg_dmc_white_limit);

    printf("PLANE_LEVEL 1 = 0x%x   \n", pDataInfo->reg_dmc_plane_level1);
    printf("PLANE_LEVEL 2 = 0x%x   \n", pDataInfo->reg_dmc_plane_level2);
    printf("PLANE_LEVEL 3 = 0x%x   \n", pDataInfo->reg_dmc_plane_level3);
    printf("PLANE_LEVEL 4 = 0x%x   \n", pDataInfo->reg_dmc_plane_level4);
    printf("PLANE_LEVEL 5 = 0x%x   \n", pDataInfo->reg_dmc_plane_level5);
    printf("PLANE_LEVEL 6 = 0x%x   \n", pDataInfo->reg_dmc_plane_level6);
    printf("PLANE_LEVEL 7 = 0x%x   \n", pDataInfo->reg_dmc_plane_level7);
    printf("PLANE_LEVEL 8 = 0x%x   \n", pDataInfo->reg_dmc_plane_level8);

    printf("DITHER_4_FRAME = %d, DITHER_RST_NUM = %d, DITHER_EN = %d    \n",
        pDataInfo->reg_dmc_dither_rst_e_4_frame, pDataInfo->reg_dmc_dither_pse_rst_num, pDataInfo->reg_dmc_dither_en);

    printf("ROI_EN = %d, ROI_H_START = %d, ROI_H_END = %d, ROI_V_START = %d, ROI_V_END = %d \n",
        pDataInfo->bROI_en, pDataInfo->iROI_hor_start_offset, pDataInfo->iROI_hor_end_offset,
        pDataInfo->iROI_ver_start_offset, pDataInfo->iROI_ver_end_offset);
}
static void SDC_Init_U13_Param(SDC_Demura_Header *phdr, interface_info *pDataInfo)
{
    SDC_Demura_Header header;
    memcpy(&header, phdr, sizeof(SDC_Demura_Header));
    pDataInfo->reg_dmc_h_block            = 0x3;         //  3 bit
    pDataInfo->reg_dmc_v_block            = 0x3;         //  3 bit

    #if(FORCE_TO_4_LAYER)
    pDataInfo->reg_dmc_plane_num              = 4;         //  4 bit
    #else
    pDataInfo->reg_dmc_plane_num              = SDC_MAX_PLANE_NUM;
    #endif
    pDataInfo->reg_dmc_rgb_mode               = 0x0;         //  1 bit
    pDataInfo->reg_dmc_panel_h_size           = 0xF00;       // 13 bit
    pDataInfo->reg_dmc_black_limit            = 0x000;       // 12 bit , format 10.2
    pDataInfo->reg_dmc_plane_level1           = header.PLANE[0]<<4;       // 12 bit , format 10.2
    pDataInfo->reg_dmc_plane_level2           = header.PLANE[1]<<4;       // 12 bit , format 10.2
    pDataInfo->reg_dmc_plane_level3           = header.PLANE[2]<<4;       // 12 bit , format 10.2
    pDataInfo->reg_dmc_plane_level4           = header.PLANE[3]<<4;       // 12 bit , format 10.2
    pDataInfo->reg_dmc_plane_level5           = header.PLANE[4]<<4;           // 12 bit , format 10.2
    pDataInfo->reg_dmc_plane_level6           = header.PLANE[5]<<4;           // 12 bit , format 10.2
    pDataInfo->reg_dmc_plane_level7           = header.PLANE[6]<<4;           // 12 bit , format 10.2
    pDataInfo->reg_dmc_plane_level8           = 0;           // 12 bit , format 10.2
    pDataInfo->reg_dmc_white_limit            = 0xFFF;       // 12 bit , format 10.2
    pDataInfo->reg_dmc_dither_en              = get_env_demura_dither();
    pDataInfo->reg_dmc_dither_rst_e_4_frame   = 0x0;         //  1 bit
    pDataInfo->reg_dmc_dither_pse_rst_num     = 0x0;         //  2 bit


}
static MS_BOOL SDC_Set_U13_Format(SDC_Demura_Header *phdr, interface_info *pinfo)
{
    int /*idx,*/ i, j, k, data = 0;
    int table_addr = 0;
    int nLayer = 0, idx_image_size;
    double Value_1 = 0.0, LUT_data = 0.0;
    SDC_Init_U13_Param(phdr, pinfo);
    // assign LUT 12 bit format
    for(k = (SDC_MAX_PLANE_NUM - 1); k >= 0; k--)
    {
        for (i = 0; i < phdr->DEMURA_TBL_V; i++)
        {
            for (j = 0; j < phdr->DEMURA_TBL_H ; j++)
            {
                // Update Value_1
                Value_1 = (1<<phdr->Value_1[k])*(255.0/4096.0);

                // 8 bit LUT
                data = phdr->lut_buffer[table_addr];
                if(data >= 128) // signed bit 7
                    data = data - 256;

                table_addr++;

                if((i == (phdr->DEMURA_TBL_V - 1)) && (j == (phdr->DEMURA_TBL_H - 1))) // skip last dummy 0xFF
                    table_addr++;

                switch(k)
                {
                    case 0:
                        nLayer = pinfo->reg_dmc_plane_level1;
                        break;
                    case 1:
                        nLayer = pinfo->reg_dmc_plane_level2;
                        break;
                    case 2:
                        nLayer = pinfo->reg_dmc_plane_level3;
                        break;
                    case 3:
                        nLayer = pinfo->reg_dmc_plane_level4;
                        break;
                    case 4:
                        nLayer = pinfo->reg_dmc_plane_level5;
                        break;
                    case 5:
                        nLayer = pinfo->reg_dmc_plane_level6;
                        break;
                    case 6:
                        nLayer = pinfo->reg_dmc_plane_level7;
                        break;
                    case 7:
                        nLayer = pinfo->reg_dmc_plane_level8;
                        break;
                    default:
                        break;
                }

                LUT_data = (double)(data*4*Value_1) + (double)(nLayer/4.0);
                idx_image_size = i*phdr->DEMURA_TBL_H+j;

                //pinfo->Lut_in[k][idx_image_size].dbr   = (double)LUT_data;
                //pinfo->Lut_in[k][idx_image_size].dbg   = (double)LUT_data;
                //pinfo->Lut_in[k][idx_image_size].dbb   = (double)LUT_data;
                //#if (defined (CONFIG_DEMURA_M7622) || defined (CONFIG_DEMURA_M7632))
                //pinfo->Lut_in[k][idx_image_size].dbw   = (double)LUT_data;
                //#endif

                //LUT_data = LUT_data>>4; // 14bit -> 10bit
                pinfo->Lut_in[k][idx_image_size].r   = (int)LUT_data;
                pinfo->Lut_in[k][idx_image_size].g   = (int)LUT_data;
                pinfo->Lut_in[k][idx_image_size].b   = (int)LUT_data;
                #if (defined (CONFIG_DEMURA_M7622) || defined (CONFIG_DEMURA_M7632))
                pinfo->Lut_in[k][idx_image_size].w   = (int)LUT_data;
                #endif
            }
        }
    }

    #if(FORCE_TO_4_LAYER)
    int layer_size  = sizeof(strgb_structInfo) * 481 * 271;

    for(k = 0; k < 4; k++)
    {
        //pinfo->Lut_in[k] = pinfo->Lut_in[SDC_Select_Layer[k]-1];
        memcpy(pinfo->Lut_in[k], pinfo->Lut_in[SDC_Select_Layer[k]-1], layer_size);
    }
    pinfo->reg_dmc_plane_level1           = phdr->PLANE[SDC_Select_Layer[0]-1]<<4;       // 12 bit , format 10.2
    pinfo->reg_dmc_plane_level2           = phdr->PLANE[SDC_Select_Layer[1]-1]<<4;       // 12 bit , format 10.2
    pinfo->reg_dmc_plane_level3           = phdr->PLANE[SDC_Select_Layer[2]-1]<<4;       // 12 bit , format 10.2
    pinfo->reg_dmc_plane_level4           = phdr->PLANE[SDC_Select_Layer[3]-1]<<4;       // 12 bit , format 10.2
    pinfo->reg_dmc_plane_level5           = 0;           // 12 bit , format 10.2
    pinfo->reg_dmc_plane_level6           = 0;           // 12 bit , format 10.2
    pinfo->reg_dmc_plane_level7           = 0;           // 12 bit , format 10.2
    #endif

    // Free buffer
    dfree(phdr->hdr_buffer);
    phdr->hdr_buffer = NULL;

    dfree(phdr->lut_buffer);
    phdr->lut_buffer = NULL;

    return TRUE;
}
#endif
static MS_BOOL SDC_Decode_To_Mstar_Format(SDC_Demura_Header *phdr, interface_info *pinfo)
{
    MS_BOOL bRet = FALSE;

    #if defined (CONFIG_DEMURA_URSA11)
        bRet = SDC_Set_U11_Format(phdr, pinfo);
    #else
        bRet = SDC_Set_U13_Format(phdr, pinfo);
    #endif

    return bRet;
}

MS_BOOL Decode_To_Mstar_Format_SDC(void *pDataInfo, BinOutputInfo *pbin_info)
{
    MS_BOOL bRet = FALSE;
    SDC_Demura_Header header;


    // Parse header information
    memset(&header, 0, sizeof(header));
    if (load_vendor_header_sdc(&header) != TRUE)
    {
        printf("load_vendor_header Error\n");
        return FALSE;
    }

    if (parse_vendor_header_sdc(&header) != TRUE)
    {
        printf("parse_vendor_header Error\n");
        return FALSE;
    }

    if (load_vendor_lut_sdc(&header) != TRUE)
    {
        printf("load_vendor_lut Error\n");
        dfree(header.hdr_buffer);
        header.hdr_buffer = NULL;
        return FALSE;
    }

    Gen_Sf_Signature_SDC(&header);
    // Alloc space for Lut_in
    int n_Hnode = 481;
    int n_Vnode = 271;

    header.DEMURA_TBL_H = n_Hnode;
    header.DEMURA_TBL_V = n_Vnode;



    // Init Output message
    pbin_info->HNode          = n_Hnode;
    pbin_info->VNode          = n_Vnode;
    #if defined (CONFIG_DEMURA_URSA11)
    pbin_info->LevelCount     = 4;//test pDataInfo->reg_dmc_plane_num;
    pbin_info->Blk_h_size     = 1 << 3;
    pbin_info->Blk_v_size     = 1 << 3;
    #else
    pbin_info->LevelCount     = SDC_MAX_PLANE_NUM;//test pDataInfo->reg_dmc_plane_num;
    pbin_info->Blk_h_size     = 1 << 3;
    pbin_info->Blk_v_size     = 1 << 3;
    #endif
    bRet = Alloc_LutIn_Space(pDataInfo, pbin_info);
    if (bRet == FALSE)
    {
        printf("Alloc_LutIn_Space error\n");
        dfree(header.hdr_buffer);
        header.hdr_buffer = NULL;

        dfree(header.lut_buffer);
        header.lut_buffer = NULL;
        return FALSE;
    }

    // Decode Vendor data
    bRet = SDC_Decode_To_Mstar_Format(&header, (interface_info *)pDataInfo);
    if (bRet == FALSE)
    {
        printf("SDC_Decode_To_Mstar_Format error\n");
        return FALSE;
    }


/*
    #if(CONFIG_DEMURA_URSA_TYPE == 13)
    MS_BOOL demura_enable[3]  = {TRUE, TRUE, TRUE};
    #else
    MS_BOOL demura_enable[4]  = {TRUE, TRUE, TRUE, TRUE};
    #endif
*/

    pbin_info->reg_base_addr  =  0x7700;
    pbin_info->project_id     =  ((MSTAR_CHIP_ID << 16) + (1ULL << 15) + EN_DEMURA_MULTI_SDC);
    pbin_info->data_type      =  IC_DRAM;
    pbin_info->Gain_type      =  10;
    pbin_info->Sep_type       =  0;
    pbin_info->Build_Date     =  0x19021814;

    //memcpy(pbin_info->channel_enable, demura_enable, sizeof(demura_enable));

    return TRUE;
}
// end SDC

// CSOT_Hisi
static MS_BOOL Gen_Sf_Signature_CSOT_HISILICON(CSOT_Hisilicon_Demura_Header *phdr)
{
    char   strbuf[1024];
    MS_U32 sample_crc32;
    MS_S32 strlen;
    MS_U8  *buffer;

    buffer = phdr->lut_buffer;

    sample_crc32 = MDrv_CRC32_Cal_DeMura(buffer + SAMPLE_DAT_START, SAMPLE_DAT_LEN);
    if (sample_crc32 == 0xffffFFFF)
    {
        UBOOT_ERROR("MDrv_CRC32_Cal_DeMura error\n");
        return FALSE;
    }
    memset(strbuf, 0, sizeof(strbuf));
    strlen = snprintf(strbuf, sizeof(strbuf)-1, "0x%x:0x%x", (unsigned int)phdr->TABLE_CRC, (unsigned int)sample_crc32);
    if (strlen < 0)
    {
        return FALSE;
    }

    Gen_Sf_Signature(strbuf, strlen);
    return TRUE;
}

MS_BOOL If_Need_Decode_CSOT_HI_SILICON(void)
{
    UBOOT_TRACE("IN\n");
#if defined(CONFIG_DEMURA_VENDOR_BACKLIGHT)
    char *sig_str;
    if (E_MS_UTIL_BIN_FILE_BACKLIGHT == get_demura_file())
    {
        sig_str = env_get(ENV_DEMURA_SIG_BL);
    }
    else
    {
        sig_str = env_get(ENV_DEMURA_SIG);
    }
#else
    char *sig_str = env_get(ENV_DEMURA_SIG);
#endif
    if (sig_str == NULL)
    {
        UBOOT_DEBUG("Empty Board, should decoding data\n");
        UBOOT_TRACE("OK\n");
        return TRUE;
    }
    else
    {
        MS_U16 lut_checksum_sf;
        MS_U32 lut_checksum_bd;
        MS_U32 sample_crc32, cal_crc32;
        MS_U8  buf[SAMPLE_DAT_LEN];

        lut_checksum_sf = get_lut_checksum_csot_hisilicon();

        UBOOT_DEBUG("sig_str = %s\n", sig_str);
        if(2 == sscanf(sig_str, "0x%x:0x%x", &lut_checksum_bd, &sample_crc32))
        {
            UBOOT_DEBUG("lut_checksum_sf = 0x%x\n", (uint)lut_checksum_sf);
            UBOOT_DEBUG("lut_checksum_bd = 0x%x\n", (uint)lut_checksum_bd);
            UBOOT_DEBUG("sample_crc32  = 0x%x\n",   (uint)sample_crc32);

            if (read_spi_flash(buf, (CSOT_HISI_LUT_START + SAMPLE_DAT_START), SAMPLE_DAT_LEN) == TRUE)
            {
                cal_crc32 = MDrv_CRC32_Cal_DeMura(buf, SAMPLE_DAT_LEN);
                UBOOT_DEBUG("cal_crc32     = 0x%x\n", (uint)cal_crc32);
                if ((sample_crc32 == cal_crc32) && (lut_checksum_sf == lut_checksum_bd))
                {
                    UBOOT_DEBUG("Data Match, No Need to decode again\n");
                    UBOOT_TRACE("OK\n");
                    return FALSE;
                }
            }
        }

        UBOOT_TRACE("OK\n");
        return TRUE;
    }

    UBOOT_TRACE("OK\n");
    return FALSE;
}
#if defined (CONFIG_DEMURA_URSA11)
static void CSOT_Hisilicon_Init_U11_Param(CSOT_Hisilicon_Demura_Header *phdr, interface_info *pDataInfo)
{
    interface_info *DataInfo   = (interface_info *)pDataInfo;
    interface_info *R_Info_Out = &(DataInfo[0]);
    interface_info *G_Info_Out = &(DataInfo[1]);
    interface_info *B_Info_Out = &(DataInfo[2]);

    int channel;
    for (channel = 0; channel < 3; channel++)
    {
        DataInfo[channel].bOut_dither_en = TRUE;
    }

    (*R_Info_Out).iChannel = 0;
    (*G_Info_Out).iChannel = 1;
    (*B_Info_Out).iChannel = 2;

    int layer_level_num      = phdr->DEMURA_PLANE_NUM;//MAX_PLANE_NUM

    layer_level_num = ((layer_level_num>=1)&&(layer_level_num<=4)) ? layer_level_num : 3;

    int idx;
    for (idx = 0; idx < 6; idx++)
    {
        (*R_Info_Out).bLayer_level_en[idx] = FALSE;
        (*G_Info_Out).bLayer_level_en[idx] = FALSE;
        (*B_Info_Out).bLayer_level_en[idx] = FALSE;
    }

    switch(layer_level_num)
    {
        case 0:
            break;
        case 1:
        case 2:
        case 3:
        case 4:
            for (idx = 0; idx < layer_level_num + 2; idx++)
            {
                (*R_Info_Out).bLayer_level_en[idx] = TRUE;
                (*G_Info_Out).bLayer_level_en[idx] = TRUE;
                (*B_Info_Out).bLayer_level_en[idx] = TRUE;
            }
            break;
        default:
            break;
    }

    for (idx = 0; idx < 6; idx++)
    {
        (*R_Info_Out).iLayer_level[idx] = ((idx!=0) ? 1023 : 0);
        (*G_Info_Out).iLayer_level[idx] = ((idx!=0) ? 1023 : 0);
        (*B_Info_Out).iLayer_level[idx] = ((idx!=0) ? 1023 : 0);
    }
    int black_level    = 0x000; //12 bits ->10bits
    int layer_level1   = 100;//12 bits ->10bits
    int layer_level2   = 240;
    int layer_level3   = 900;
    int layer_level4   = 0;
    int white_level    = 0xFFF/4;//12 bits ->10bits
/*
    black_level  = ((black_level)<<4);   // 8 bit -> 12 bit
    layer_level1 = ((layer_level1 + 2)<<4);  // 8 bit -> 12 bit
    layer_level2 = ((layer_level2 + 2)<<4);  // 8 bit -> 12 bit
    layer_level3 = ((layer_level3 + 2)<<4);  // 8 bit -> 12 bit
    layer_level4 = ((layer_level4 + 2)<<4);  // 8 bit -> 12 bit
    white_level  = ((white_level)<<4);   // 8 bit -> 12 bit
*/
    switch(layer_level_num)
    {
    case 1:
        (*R_Info_Out).iLayer_level[0] = black_level;
        (*R_Info_Out).iLayer_level[1] = layer_level1;
        (*R_Info_Out).iLayer_level[2] = white_level;

        (*G_Info_Out).iLayer_level[0] = black_level;
        (*G_Info_Out).iLayer_level[1] = layer_level1;
        (*G_Info_Out).iLayer_level[2] = white_level;

        (*B_Info_Out).iLayer_level[0] = black_level;
        (*B_Info_Out).iLayer_level[1] = layer_level1;
        (*B_Info_Out).iLayer_level[2] = white_level;
        break;
    case 2:
        (*R_Info_Out).iLayer_level[0] = black_level;
        (*R_Info_Out).iLayer_level[1] = layer_level1;
        (*R_Info_Out).iLayer_level[2] = layer_level2;
        (*R_Info_Out).iLayer_level[3] = white_level;

        (*G_Info_Out).iLayer_level[0] = black_level;
        (*G_Info_Out).iLayer_level[1] = layer_level1;
        (*G_Info_Out).iLayer_level[2] = layer_level2;
        (*G_Info_Out).iLayer_level[3] = white_level;

        (*B_Info_Out).iLayer_level[0] = black_level;
        (*B_Info_Out).iLayer_level[1] = layer_level1;
        (*B_Info_Out).iLayer_level[2] = layer_level2;
        (*B_Info_Out).iLayer_level[3] = white_level;
        break;
    case 3:
        (*R_Info_Out).iLayer_level[0] = black_level;
        (*R_Info_Out).iLayer_level[1] = layer_level1;
        (*R_Info_Out).iLayer_level[2] = layer_level2;
        (*R_Info_Out).iLayer_level[3] = layer_level3;
        (*R_Info_Out).iLayer_level[4] = white_level;

        (*G_Info_Out).iLayer_level[0] = black_level;
        (*G_Info_Out).iLayer_level[1] = layer_level1;
        (*G_Info_Out).iLayer_level[2] = layer_level2;
        (*G_Info_Out).iLayer_level[3] = layer_level3;
        (*G_Info_Out).iLayer_level[4] = white_level;

        (*B_Info_Out).iLayer_level[0] = black_level;
        (*B_Info_Out).iLayer_level[1] = layer_level1;
        (*B_Info_Out).iLayer_level[2] = layer_level2;
        (*B_Info_Out).iLayer_level[3] = layer_level3;
        (*B_Info_Out).iLayer_level[4] = white_level;
        break;
    case 4:
        (*R_Info_Out).iLayer_level[0] = black_level;
        (*R_Info_Out).iLayer_level[1] = layer_level1;
        (*R_Info_Out).iLayer_level[2] = layer_level2;
        (*R_Info_Out).iLayer_level[3] = layer_level3;
        (*R_Info_Out).iLayer_level[4] = layer_level4;
        (*R_Info_Out).iLayer_level[5] = white_level;

        (*G_Info_Out).iLayer_level[0] = black_level;
        (*G_Info_Out).iLayer_level[1] = layer_level1;
        (*G_Info_Out).iLayer_level[2] = layer_level2;
        (*G_Info_Out).iLayer_level[3] = layer_level3;
        (*G_Info_Out).iLayer_level[4] = layer_level4;
        (*G_Info_Out).iLayer_level[5] = white_level;

        (*B_Info_Out).iLayer_level[0] = black_level;
        (*B_Info_Out).iLayer_level[1] = layer_level1;
        (*B_Info_Out).iLayer_level[2] = layer_level2;
        (*B_Info_Out).iLayer_level[3] = layer_level3;
        (*B_Info_Out).iLayer_level[4] = layer_level4;
        (*B_Info_Out).iLayer_level[5] = white_level;
        break;
    }
    //++++++++++++ layerX level end ++++++++++++//

    //++++++++++++ low luma weight start ++++++++++++//
    (*R_Info_Out).bLow_luma_en         = FALSE;
    (*R_Info_Out).iLow_luma_thrd       = 0x00;
    (*R_Info_Out).iLow_luma_slope      = 0x0F;
    (*R_Info_Out).iLow_luma_min_weight = 0x0;

    (*G_Info_Out).bLow_luma_en         = FALSE;
    (*G_Info_Out).iLow_luma_thrd       = 0x00;
    (*G_Info_Out).iLow_luma_slope      = 0x0F;
    (*G_Info_Out).iLow_luma_min_weight = 0x0;

    (*B_Info_Out).bLow_luma_en         = FALSE;
    (*B_Info_Out).iLow_luma_thrd       = 0x00;
    (*B_Info_Out).iLow_luma_slope      = 0x0F;
    (*B_Info_Out).iLow_luma_min_weight = 0x0;
    //++++++++++++ low luma weight end ++++++++++++//

    //++++++++++++ high luma weight start ++++++++++++//
    (*R_Info_Out).bHigh_luma_en         = FALSE;
    (*R_Info_Out).iHigh_luma_thrd       = 0xFF;
    (*R_Info_Out).iHigh_luma_slope      = 0x0F;
    (*R_Info_Out).iHigh_luma_min_weight = 0x0;

    (*G_Info_Out).bHigh_luma_en         = FALSE;
    (*G_Info_Out).iHigh_luma_thrd       = 0xFF;
    (*G_Info_Out).iHigh_luma_slope      = 0x0F;
    (*G_Info_Out).iHigh_luma_min_weight = 0x0;

    (*B_Info_Out).bHigh_luma_en         = FALSE;
    (*B_Info_Out).iHigh_luma_thrd       = 0xFF;
    (*B_Info_Out).iHigh_luma_slope      = 0x0F;
    (*B_Info_Out).iHigh_luma_min_weight = 0x0;
    //++++++++++++ high luma weight end ++++++++++++//

    for (idx = 0; idx < 6; idx++)
    {
        int cnt_idx;
        for (cnt_idx = 0; cnt_idx < (phdr->DEMURA_TBL_H* phdr->DEMURA_TBL_V); cnt_idx++)
        {
            (*R_Info_Out).Lut_in[idx][cnt_idx] = 0.0;
            (*G_Info_Out).Lut_in[idx][cnt_idx] = 0.0;
            (*B_Info_Out).Lut_in[idx][cnt_idx] = 0.0;
        }
    }
/*
    for(idx = 0; idx < 10; idx++)
        printf("Lut = %f\t", (*R_Info_Out).Lut_in[0][idx]);
    printf("\n");
*/
}
static MS_BOOL CSOT_Hisilicon_Set_U11_Format(CSOT_Hisilicon_Demura_Header *phdr, interface_info *pinfo)
{
    interface_info *DataInfo   = (interface_info *)pinfo;
    interface_info *R_Info_Out = &(DataInfo[0]);
    interface_info *G_Info_Out = &(DataInfo[1]);
    interface_info *B_Info_Out = &(DataInfo[2]);
    int i, j, k, data = 0;
    int table_addr = 0;
    int nLayer = 0, LUT_data, idx_image_size;
    int burst_num = 0, data_per_burst = 0;
    MS_BOOL pol = 0;

    CSOT_Hisilicon_Init_U11_Param(phdr, pinfo);
    data_per_burst = 10; // 128bit per burst, 12bit per data => 10 data and 8bit dummy byte.
    burst_num = (phdr->DEMURA_TBL_H/data_per_burst) + ((phdr->DEMURA_TBL_H%data_per_burst > 0)?1:0);
    //printf("burst_num = %d \n", burst_num);


    // assign LUT 12 bit format
    for (i = 0; i < phdr->DEMURA_TBL_V; i++)
    {
        for(k = 0; k < phdr->DEMURA_PLANE_NUM; k++)
        {
            for (j = 0; j < phdr->DEMURA_TBL_H ; j++)
            {
                // LUT index for the plane
                idx_image_size = i*phdr->DEMURA_TBL_H+j;

                // 12 bit each for 10 bit value
                if(pol == 0) //8bit + 4bit
                {
                    data = ((MS_U16)phdr->lut_buffer[table_addr]) | (((MS_U16)phdr->lut_buffer[table_addr + 1]&0x0f)<<8);
                    pol = 1;
                    table_addr+=1;
                }
                else
                {
                    data = (((MS_U16)phdr->lut_buffer[table_addr] & 0xf0)>>4) | (((MS_U16)phdr->lut_buffer[table_addr + 1]&0xff)<<4);
                    pol = 0;
                    table_addr+=2;
                }

                if((j+1)%data_per_burst == 0) // The last data of the burst
                {
                    table_addr++;
                    pol = 0;
                }
                if(j == (phdr->DEMURA_TBL_H - 1)) // The last data of the H
                {
                    table_addr+=15; // skip dummy 15 bytes
                    pol = 0;
                }

                if((data & _BIT11) == _BIT11) // signed bit
                    data = (0x1000 - data)*(-1);


                LUT_data = (data/4);

                (*R_Info_Out).Lut_in[k][idx_image_size]       = LUT_data;
                (*G_Info_Out).Lut_in[k][idx_image_size]       = LUT_data;
                (*B_Info_Out).Lut_in[k][idx_image_size]       = LUT_data;
            }
        }
    }


    return TRUE;
}
#else
static void CSOT_Hisilicon_Init_U13_Param(CSOT_Hisilicon_Demura_Header *phdr, interface_info *pDataInfo)
{
    CSOT_Hisilicon_Demura_Header header;
    memcpy(&header, phdr, sizeof(CSOT_Hisilicon_Demura_Header));
    pDataInfo->reg_dmc_h_block            = header.DEMURA_BLK_H;         //  3 bit
    pDataInfo->reg_dmc_v_block            = header.DEMURA_BLK_V;         //  3 bit

    pDataInfo->reg_dmc_plane_num              = header.DEMURA_PLANE_NUM;         //  4 bit
    pDataInfo->reg_dmc_rgb_mode               = header.DEMURA_MODE;         //  1 bit
    pDataInfo->reg_dmc_panel_h_size           = 0xF00;       // 13 bit
    pDataInfo->reg_dmc_black_limit            = 0x000;       // 12 bit , format 10.2
    pDataInfo->reg_dmc_plane_level1           = 100*4;       // 12 bit , format 10.2
    pDataInfo->reg_dmc_plane_level2           = 240*4;       // 12 bit , format 10.2
    pDataInfo->reg_dmc_plane_level3           = 900*4;       // 12 bit , format 10.2
    pDataInfo->reg_dmc_plane_level4           = 0;       // 12 bit , format 10.2
    pDataInfo->reg_dmc_plane_level5           = 0;           // 12 bit , format 10.2
    pDataInfo->reg_dmc_plane_level6           = 0;           // 12 bit , format 10.2
    pDataInfo->reg_dmc_plane_level7           = 0;           // 12 bit , format 10.2
    pDataInfo->reg_dmc_plane_level8           = 0;           // 12 bit , format 10.2
    pDataInfo->reg_dmc_white_limit            = 0xFFF;       // 12 bit , format 10.2
    pDataInfo->reg_dmc_dither_en              = get_env_demura_dither();
    pDataInfo->reg_dmc_dither_rst_e_4_frame   = 0x0;         //  1 bit
    pDataInfo->reg_dmc_dither_pse_rst_num     = 0x0;         //  2 bit
}
static MS_BOOL CSOT_Hisilicon_Set_U13_Format(CSOT_Hisilicon_Demura_Header *phdr, interface_info *pinfo)
{
    int i, j, k, data = 0;
    int table_addr = 0;
    int nLayer = 0, LUT_data, idx_image_size;
    int /*burst_num = 0,*/ data_per_burst = 0;
    MS_BOOL pol = 0;

    CSOT_Hisilicon_Init_U13_Param(phdr, pinfo);
    data_per_burst = 10; // 128bit per burst, 12bit per data => 10 data and 8bit dummy byte.
    //burst_num = (phdr->DEMURA_TBL_H/data_per_burst) + ((phdr->DEMURA_TBL_H%data_per_burst > 0)?1:0);
    //printf("burst_num = %d \n", burst_num);


    // assign LUT 12 bit format
    for (i = 0; i < phdr->DEMURA_TBL_V; i++)
    {
        for(k = 0; k < pinfo->reg_dmc_plane_num; k++)
        {
            for (j = 0; j < phdr->DEMURA_TBL_H ; j++)
            {
                // LUT index for the plane
                idx_image_size = i*phdr->DEMURA_TBL_H+j;

                // 12 bit each for 10 bit value
                if(pol == 0) //8bit + 4bit
                {
                    data = ((MS_U16)phdr->lut_buffer[table_addr]) | (((MS_U16)phdr->lut_buffer[table_addr + 1]&0x0f)<<8);
                    pol = 1;
                    table_addr+=1;
                }
                else
                {
                    data = (((MS_U16)phdr->lut_buffer[table_addr] & 0xf0)>>4) | (((MS_U16)phdr->lut_buffer[table_addr + 1]&0xff)<<4);
                    pol = 0;
                    table_addr+=2;
                }

                if((j+1)%data_per_burst == 0) // The last data of the burst
                {
                    table_addr++;
                    pol = 0;
                }
                if(j == (phdr->DEMURA_TBL_H - 1)) // The last data of the H
                {
                    table_addr+=15; // skip dummy 15 bytes
                    pol = 0;
                }

                if((data & _BIT11) == _BIT11) // signed bit
                    data = (0x1000 - data)*(-1);

                switch(k)
                {
                    case 0:
                        nLayer = pinfo->reg_dmc_plane_level1;
                        break;
                    case 1:
                        nLayer = pinfo->reg_dmc_plane_level2;
                        break;
                    case 2:
                        nLayer = pinfo->reg_dmc_plane_level3;
                        break;
                    case 3:
                        nLayer = pinfo->reg_dmc_plane_level4;
                        break;
                    case 4:
                        nLayer = pinfo->reg_dmc_plane_level5;
                        break;
                    case 5:
                        nLayer = pinfo->reg_dmc_plane_level6;
                        break;
                    case 6:
                        nLayer = pinfo->reg_dmc_plane_level7;
                        break;
                    case 7:
                        nLayer = pinfo->reg_dmc_plane_level8;
                        break;
                    default:
                        break;
                }

                LUT_data = (data/4) + (nLayer/4);

                //pinfo->Lut_in[k][idx_image_size].dbr   = (double)LUT_data;
                //pinfo->Lut_in[k][idx_image_size].dbg   = (double)LUT_data;
                //pinfo->Lut_in[k][idx_image_size].dbb   = (double)LUT_data;
                //#if (defined (CONFIG_DEMURA_M7622) || defined (CONFIG_DEMURA_M7632))
                //pinfo->Lut_in[k][idx_image_size].dbw   = LUT_data;
                //#endif

                pinfo->Lut_in[k][idx_image_size].r   = LUT_data;
                pinfo->Lut_in[k][idx_image_size].g   = LUT_data;
                pinfo->Lut_in[k][idx_image_size].b   = LUT_data;
                #if (defined (CONFIG_DEMURA_M7622) || defined (CONFIG_DEMURA_M7632))
                pinfo->Lut_in[k][idx_image_size].w   = LUT_data;
                #endif
            }
        }
    }

    // Free buffer
    dfree(phdr->hdr_buffer);
    phdr->hdr_buffer = NULL;

    dfree(phdr->lut_buffer);
    phdr->lut_buffer = NULL;

    return TRUE;
}
#endif
static MS_BOOL CSOT_Hisilicon_Decode_To_Mstar_Format(CSOT_Hisilicon_Demura_Header *phdr, interface_info *pinfo)
{
    MS_BOOL bRet = FALSE;

    #if defined (CONFIG_DEMURA_URSA11)
        bRet = CSOT_Hisilicon_Set_U11_Format(phdr, pinfo);
    #else
        bRet = CSOT_Hisilicon_Set_U13_Format(phdr, pinfo);
    #endif

    return bRet;
}


MS_BOOL Decode_To_Mstar_Format_CSOT_HI_SILICON(void *pDataInfo, BinOutputInfo *pbin_info)
{
    MS_BOOL bRet = FALSE;
    CSOT_Hisilicon_Demura_Header header;


    // Parse header information
    memset(&header, 0, sizeof(header));
    if (load_vendor_header_csot_hisilicon(&header) != TRUE)
    {
        printf("load_vendor_header Error\n");
        return FALSE;
    }

    if (parse_vendor_header_csot_hisilicon(&header) != TRUE)
    {
        printf("parse_vendor_header Error\n");
        dfree(header.hdr_buffer);
        header.hdr_buffer = NULL;
        return FALSE;
    }

    if (load_vendor_lut_csot_hisilicon(&header) != TRUE)
    {
        printf("load_vendor_lut Error\n");
        dfree(header.hdr_buffer);
        header.hdr_buffer = NULL;
        return FALSE;
    }

    Gen_Sf_Signature_CSOT_HISILICON(&header);
    // Alloc space for Lut_in
    int n_Hnode = header.DEMURA_TBL_H;
    int n_Vnode = header.DEMURA_TBL_V;



    // Init Output message
    pbin_info->HNode          = n_Hnode;
    pbin_info->VNode          = n_Vnode;
    pbin_info->LevelCount     = header.DEMURA_PLANE_NUM;
    pbin_info->Blk_h_size     = 1 << (header.DEMURA_BLK_H);
    pbin_info->Blk_v_size     = 1 << (header.DEMURA_BLK_V);
    #if defined (CONFIG_DEMURA_URSA13)
    dump_interface_info(pDataInfo);
    #endif
    bRet = Alloc_LutIn_Space(pDataInfo, pbin_info);
    if (bRet == FALSE)
    {
        printf("Alloc_LutIn_Space error\n");
        dfree(header.hdr_buffer);
        header.hdr_buffer = NULL;

        dfree(header.lut_buffer);
        header.lut_buffer = NULL;
        return FALSE;
    }

    // Decode Vendor data
    bRet = CSOT_Hisilicon_Decode_To_Mstar_Format(&header, (interface_info *)pDataInfo);
    if (bRet == FALSE)
    {
        printf("CHOT_Decode_To_Mstar_Format error\n");
        return FALSE;
    }

/*
    #if defined (CONFIG_DEMURA_URSA13) || defined (CONFIG_DEMURA_URSA11)
    MS_BOOL demura_enable[3]  = {TRUE, TRUE, TRUE};
    #else
    MS_BOOL demura_enable[4]  = {TRUE, TRUE, TRUE, TRUE};
    #endif
*/

    pbin_info->reg_base_addr  =  0x7700;
    pbin_info->project_id     =  ((MSTAR_CHIP_ID << DEC_16) + (1ULL << DEC_15) + EN_DEMURA_MULTI_CSOT_HI_SILICON);
    pbin_info->data_type      =  IC_DRAM;
    pbin_info->Gain_type      =  10;
    pbin_info->Sep_type       =  0;
    pbin_info->Build_Date     =  0x19071010;

    //memcpy(pbin_info->channel_enable, demura_enable, sizeof(demura_enable));

    return TRUE;
}
// End CSOT_Hisi

// NOVA
#define _0XFFFF 0xFFFF
static MS_BOOL Gen_Sf_Signature_NOVA(CHOT_Demura_Header *phdr)
{
    char   strbuf[1024];
    MS_U32 sample_crc32;
    MS_S32 strlen;
    MS_U8  *buffer;

    buffer = phdr->lut_buffer + phdr->lut_offset;

    sample_crc32 = MDrv_CRC32_Cal_DeMura(buffer + SAMPLE_DAT_START, SAMPLE_DAT_LEN);
    if (sample_crc32 == 0xffffFFFF)
    {
        UBOOT_ERROR("MDrv_CRC32_Cal_DeMura error\n");
        return FALSE;
    }
    memset(strbuf, 0, sizeof(strbuf));
    strlen = snprintf(strbuf, sizeof(strbuf)-1, "0x%x:0x%x", (unsigned int)phdr->TABLE_CRC & _0XFFFF, (unsigned int)sample_crc32);
    if (strlen < 0)
    {
        return FALSE;
    }

    Gen_Sf_Signature(strbuf, strlen);
    return TRUE;
}


MS_BOOL If_Need_Decode_NOVA(MS_U32 flash_offset)
{
    UBOOT_TRACE("IN\n");
#if defined(CONFIG_DEMURA_VENDOR_BACKLIGHT)
    char *sig_str;
    if (E_MS_UTIL_BIN_FILE_BACKLIGHT == get_demura_file())
    {
        sig_str = env_get(ENV_DEMURA_SIG_BL);
    }
    else
    {
        sig_str = env_get(ENV_DEMURA_SIG);
    }
#else
    char *sig_str = env_get(ENV_DEMURA_SIG);
#endif

    if (sig_str == NULL)
    {
        UBOOT_TRACE("Empty Board, should decoding data\n");
        UBOOT_TRACE("OK\n");
        return TRUE;
    }
    else
    {
        MS_U16 lut_checksum_sf;
        MS_U32 lut_checksum_bd;
        MS_U32 sample_crc32, cal_crc32;
        MS_U8  buf[SAMPLE_DAT_LEN];

        lut_checksum_sf = get_lut_checksum_chot(flash_offset);

        UBOOT_TRACE("sig_str = %s\n", sig_str);
        if(2 == sscanf(sig_str, "0x%x:0x%x", &lut_checksum_bd, &sample_crc32))
        {
            UBOOT_TRACE("lut_checksum_sf = 0x%x\n", (uint)lut_checksum_sf);
            UBOOT_TRACE("lut_checksum_bd = 0x%x\n", (uint)lut_checksum_bd);
            UBOOT_TRACE("sample_crc32  = 0x%x\n",   (uint)sample_crc32);

            if (read_spi_flash(buf, (flash_offset + (CHOT_LUT_START + SAMPLE_DAT_START)), SAMPLE_DAT_LEN) == TRUE)
            {
                cal_crc32 = MDrv_CRC32_Cal_DeMura(buf, SAMPLE_DAT_LEN);
                UBOOT_DEBUG("cal_crc32     = 0x%x\n", (uint)cal_crc32);
                if ((sample_crc32 == cal_crc32) && (lut_checksum_sf == lut_checksum_bd))
                {
                    UBOOT_DEBUG("Data Match, No Need to decode again\n");
                    UBOOT_TRACE("OK\n");
                    return FALSE;
                }
            }
        }

        UBOOT_TRACE("OK\n");
        return TRUE;
    }

    UBOOT_TRACE("OK\n");
    return FALSE;
}


#if defined (CONFIG_DEMURA_URSA13)
static void dump_interface_info(interface_info *pDataInfo)
{
    printf("PLANE_NUM = %d  \n", pDataInfo->reg_dmc_plane_num);
    printf("H_BLOCK = %d, V_BLOCK = %d  \n", pDataInfo->reg_dmc_h_block, pDataInfo->reg_dmc_v_block);
    printf("RGB_MODE = %d   \n", pDataInfo->reg_dmc_rgb_mode);
    printf("PANEL_H_SIZE = %d   \n", pDataInfo->reg_dmc_panel_h_size);
    printf("BLACK_LIMIT = 0x%x, WHITE_LIMIT = 0x%x  \n", pDataInfo->reg_dmc_black_limit, pDataInfo->reg_dmc_white_limit);

    printf("PLANE_LEVEL 1 = 0x%x   \n", pDataInfo->reg_dmc_plane_level1);
    printf("PLANE_LEVEL 2 = 0x%x   \n", pDataInfo->reg_dmc_plane_level2);
    printf("PLANE_LEVEL 3 = 0x%x   \n", pDataInfo->reg_dmc_plane_level3);
    printf("PLANE_LEVEL 4 = 0x%x   \n", pDataInfo->reg_dmc_plane_level4);
    printf("PLANE_LEVEL 5 = 0x%x   \n", pDataInfo->reg_dmc_plane_level5);
    printf("PLANE_LEVEL 6 = 0x%x   \n", pDataInfo->reg_dmc_plane_level6);
    printf("PLANE_LEVEL 7 = 0x%x   \n", pDataInfo->reg_dmc_plane_level7);
    printf("PLANE_LEVEL 8 = 0x%x   \n", pDataInfo->reg_dmc_plane_level8);

    printf("DITHER_4_FRAME = %d, DITHER_RST_NUM = %d, DITHER_EN = %d    \n",
        pDataInfo->reg_dmc_dither_rst_e_4_frame, pDataInfo->reg_dmc_dither_pse_rst_num, pDataInfo->reg_dmc_dither_en);

    printf("ROI_EN = %d, ROI_H_START = %d, ROI_H_END = %d, ROI_V_START = %d, ROI_V_END = %d \n",
        pDataInfo->bROI_en, pDataInfo->iROI_hor_start_offset, pDataInfo->iROI_hor_end_offset,
        pDataInfo->iROI_ver_start_offset, pDataInfo->iROI_ver_end_offset);
}
#endif
#if defined (CONFIG_DEMURA_URSA11)
static void NOVA_Init_U11_Param(CHOT_Demura_Header *phdr, interface_info *pDataInfo)
{
    interface_info *DataInfo   = (interface_info *)pDataInfo;
    interface_info *R_Info_Out = &(DataInfo[0]);
    interface_info *G_Info_Out = &(DataInfo[1]);
    interface_info *B_Info_Out = &(DataInfo[2]);

    int channel;
    for (channel = 0; channel < 3; channel++)
    {
        DataInfo[channel].bOut_dither_en = TRUE;
    }

    (*R_Info_Out).iChannel = 0;
    (*G_Info_Out).iChannel = 1;
    (*B_Info_Out).iChannel = 2;

    int layer_level_num      = phdr->DEMURA_PLANE_NUM;//MAX_PLANE_NUM

    layer_level_num = ((layer_level_num>=1)&&(layer_level_num<=4)) ? layer_level_num : 3;

    int idx;
    for (idx = 0; idx < 6; idx++)
    {
        (*R_Info_Out).bLayer_level_en[idx] = FALSE;
        (*G_Info_Out).bLayer_level_en[idx] = FALSE;
        (*B_Info_Out).bLayer_level_en[idx] = FALSE;
    }

    switch(layer_level_num)
    {
        case 0:
            break;
        case 1:
        case 2:
        case 3:
        case 4:
            for (idx = 0; idx < layer_level_num + 2; idx++)
            {
                (*R_Info_Out).bLayer_level_en[idx] = TRUE;
                (*G_Info_Out).bLayer_level_en[idx] = TRUE;
                (*B_Info_Out).bLayer_level_en[idx] = TRUE;
            }
            break;
        default:
            break;
    }

    for (idx = 0; idx < 6; idx++)
    {
        (*R_Info_Out).iLayer_level[idx] = ((idx!=0) ? 1023 : 0);
        (*G_Info_Out).iLayer_level[idx] = ((idx!=0) ? 1023 : 0);
        (*B_Info_Out).iLayer_level[idx] = ((idx!=0) ? 1023 : 0);
    }

    int black_level    = phdr->LOWER_BOUND;
    int layer_level1   = phdr->PLANE[0];
    int layer_level2   = phdr->PLANE[1];
    int layer_level3   = phdr->PLANE[2];
    int layer_level4   = phdr->PLANE[3];
    int white_level    = phdr->UPPER_BOUND;
/*
    black_level  = ((black_level)<<4);   // 8 bit -> 12 bit
    layer_level1 = ((layer_level1 + 2)<<4);  // 8 bit -> 12 bit
    layer_level2 = ((layer_level2 + 2)<<4);  // 8 bit -> 12 bit
    layer_level3 = ((layer_level3 + 2)<<4);  // 8 bit -> 12 bit
    layer_level4 = ((layer_level4 + 2)<<4);  // 8 bit -> 12 bit
    white_level  = ((white_level)<<4);   // 8 bit -> 12 bit
*/
    switch(layer_level_num)
    {
    case 1:
        (*R_Info_Out).iLayer_level[0] = black_level;
        (*R_Info_Out).iLayer_level[1] = layer_level1;
        (*R_Info_Out).iLayer_level[2] = white_level;

        (*G_Info_Out).iLayer_level[0] = black_level;
        (*G_Info_Out).iLayer_level[1] = layer_level1;
        (*G_Info_Out).iLayer_level[2] = white_level;

        (*B_Info_Out).iLayer_level[0] = black_level;
        (*B_Info_Out).iLayer_level[1] = layer_level1;
        (*B_Info_Out).iLayer_level[2] = white_level;
        break;
    case 2:
        (*R_Info_Out).iLayer_level[0] = black_level;
        (*R_Info_Out).iLayer_level[1] = layer_level1;
        (*R_Info_Out).iLayer_level[2] = layer_level2;
        (*R_Info_Out).iLayer_level[3] = white_level;

        (*G_Info_Out).iLayer_level[0] = black_level;
        (*G_Info_Out).iLayer_level[1] = layer_level1;
        (*G_Info_Out).iLayer_level[2] = layer_level2;
        (*G_Info_Out).iLayer_level[3] = white_level;

        (*B_Info_Out).iLayer_level[0] = black_level;
        (*B_Info_Out).iLayer_level[1] = layer_level1;
        (*B_Info_Out).iLayer_level[2] = layer_level2;
        (*B_Info_Out).iLayer_level[3] = white_level;
        break;
    case 3:
        (*R_Info_Out).iLayer_level[0] = black_level;
        (*R_Info_Out).iLayer_level[1] = layer_level1;
        (*R_Info_Out).iLayer_level[2] = layer_level2;
        (*R_Info_Out).iLayer_level[3] = layer_level3;
        (*R_Info_Out).iLayer_level[4] = white_level;

        (*G_Info_Out).iLayer_level[0] = black_level;
        (*G_Info_Out).iLayer_level[1] = layer_level1;
        (*G_Info_Out).iLayer_level[2] = layer_level2;
        (*G_Info_Out).iLayer_level[3] = layer_level3;
        (*G_Info_Out).iLayer_level[4] = white_level;

        (*B_Info_Out).iLayer_level[0] = black_level;
        (*B_Info_Out).iLayer_level[1] = layer_level1;
        (*B_Info_Out).iLayer_level[2] = layer_level2;
        (*B_Info_Out).iLayer_level[3] = layer_level3;
        (*B_Info_Out).iLayer_level[4] = white_level;
        break;
    case 4:
        (*R_Info_Out).iLayer_level[0] = black_level;
        (*R_Info_Out).iLayer_level[1] = layer_level1;
        (*R_Info_Out).iLayer_level[2] = layer_level2;
        (*R_Info_Out).iLayer_level[3] = layer_level3;
        (*R_Info_Out).iLayer_level[4] = layer_level4;
        (*R_Info_Out).iLayer_level[5] = white_level;

        (*G_Info_Out).iLayer_level[0] = black_level;
        (*G_Info_Out).iLayer_level[1] = layer_level1;
        (*G_Info_Out).iLayer_level[2] = layer_level2;
        (*G_Info_Out).iLayer_level[3] = layer_level3;
        (*G_Info_Out).iLayer_level[4] = layer_level4;
        (*G_Info_Out).iLayer_level[5] = white_level;

        (*B_Info_Out).iLayer_level[0] = black_level;
        (*B_Info_Out).iLayer_level[1] = layer_level1;
        (*B_Info_Out).iLayer_level[2] = layer_level2;
        (*B_Info_Out).iLayer_level[3] = layer_level3;
        (*B_Info_Out).iLayer_level[4] = layer_level4;
        (*B_Info_Out).iLayer_level[5] = white_level;
        break;
    }
    //++++++++++++ layerX level end ++++++++++++//

    //++++++++++++ low luma weight start ++++++++++++//
    (*R_Info_Out).bLow_luma_en         = FALSE;
    (*R_Info_Out).iLow_luma_thrd       = 0x00;
    (*R_Info_Out).iLow_luma_slope      = 0x0F;
    (*R_Info_Out).iLow_luma_min_weight = 0x0;

    (*G_Info_Out).bLow_luma_en         = FALSE;
    (*G_Info_Out).iLow_luma_thrd       = 0x00;
    (*G_Info_Out).iLow_luma_slope      = 0x0F;
    (*G_Info_Out).iLow_luma_min_weight = 0x0;

    (*B_Info_Out).bLow_luma_en         = FALSE;
    (*B_Info_Out).iLow_luma_thrd       = 0x00;
    (*B_Info_Out).iLow_luma_slope      = 0x0F;
    (*B_Info_Out).iLow_luma_min_weight = 0x0;
    //++++++++++++ low luma weight end ++++++++++++//

    //++++++++++++ high luma weight start ++++++++++++//
    (*R_Info_Out).bHigh_luma_en         = FALSE;
    (*R_Info_Out).iHigh_luma_thrd       = 0xFF;
    (*R_Info_Out).iHigh_luma_slope      = 0x0F;
    (*R_Info_Out).iHigh_luma_min_weight = 0x0;

    (*G_Info_Out).bHigh_luma_en         = FALSE;
    (*G_Info_Out).iHigh_luma_thrd       = 0xFF;
    (*G_Info_Out).iHigh_luma_slope      = 0x0F;
    (*G_Info_Out).iHigh_luma_min_weight = 0x0;

    (*B_Info_Out).bHigh_luma_en         = FALSE;
    (*B_Info_Out).iHigh_luma_thrd       = 0xFF;
    (*B_Info_Out).iHigh_luma_slope      = 0x0F;
    (*B_Info_Out).iHigh_luma_min_weight = 0x0;
    //++++++++++++ high luma weight end ++++++++++++//

    for (idx = 0; idx < 6; idx++)
    {
        int cnt_idx;
        for (cnt_idx = 0; cnt_idx < (phdr->DEMURA_TBL_H* phdr->DEMURA_TBL_V); cnt_idx++)
        {
            (*R_Info_Out).Lut_in[idx][cnt_idx] = 0.0;
            (*G_Info_Out).Lut_in[idx][cnt_idx] = 0.0;
            (*B_Info_Out).Lut_in[idx][cnt_idx] = 0.0;
        }
    }
/*
    for(idx = 0; idx < 10; idx++)
        printf("Lut = %f\t", (*R_Info_Out).Lut_in[0][idx]);
    printf("\n");
*/
}
static MS_BOOL NOVA_Set_U11_Format(CHOT_Demura_Header *phdr, interface_info *pinfo)
{
    interface_info *DataInfo   = (interface_info *)pinfo;
    interface_info *R_Info_Out = &(DataInfo[0]);
    interface_info *G_Info_Out = &(DataInfo[1]);
    interface_info *B_Info_Out = &(DataInfo[2]);
    // init morton interface data
    NOVA_Init_U11_Param(phdr, pinfo);

    //dump_interface_info(pinfo);

    int Lut_buffer[phdr->DEMURA_PLANE_NUM][phdr->DEMURA_TBL_V][phdr->DEMURA_TBL_H];
    int idx, i, j, k, data = 0;
    int table_addr = CHOT_LUT_START - CHOT_LUT_CHECKSUM; // 0x42-0x40 Skip CRC
    MS_BOOL pol = 0;

    // assign LUT 12 bit format
    for (i = 0; i < phdr->DEMURA_TBL_V; i++)
    {
        for (j = 0; j < phdr->DEMURA_TBL_H ; j++)
        {
            for(k = 0; k < phdr->DEMURA_PLANE_NUM; k++)
            {
                if(pol == 0) //8bit + 4bit
                {
                    data = ((MS_U16)phdr->lut_buffer[table_addr]<<4) | (((MS_U16)phdr->lut_buffer[table_addr + 1]&0xf0)>>4);
                    pol = 1;
                    table_addr+=1;
                }
                else
                {
                    data = (((MS_U16)phdr->lut_buffer[table_addr] & 0x0f)<<8) | (((MS_U16)phdr->lut_buffer[table_addr + 1]&0xff));
                    pol = 0;
                    table_addr+=2;
                }
                // Update 12bit buffer
                if(data >= 0x800)
                    data = data - 0x1000; // signed bit
                Lut_buffer[k][i][j] = data;
            }
        }
    }

    for (idx = 0; idx < phdr->DEMURA_PLANE_NUM; idx++)
    {
        int n_Hnode     = phdr->DEMURA_TBL_H;       // 481;
        int n_Vnode     = phdr->DEMURA_TBL_V;       // 271;

        int idx_image_size = 0;
        double LUT_data = 0;

        for (i = 0; i < n_Vnode; i++)
        {
            for (j = 0; j < n_Hnode ; j++)
            {
                LUT_data = ((double)Lut_buffer[idx][i][j]/4.0);

                (*R_Info_Out).Lut_in[idx+1][idx_image_size]       = LUT_data;
                (*G_Info_Out).Lut_in[idx+1][idx_image_size]       = LUT_data;
                (*B_Info_Out).Lut_in[idx+1][idx_image_size]       = LUT_data;

                idx_image_size += 1;
            }
        }
    }

    return TRUE;
}
#else
static void init_morton_data(CHOT_Demura_Header *phdr, interface_info *pDataInfo)
{
    printf("============================================\n");
    pDataInfo->reg_dmc_rgb_mode               = 0;           //  1 bit
    pDataInfo->reg_dmc_panel_h_size           = 0xF00;                     // 13 bit
    //pDataInfo->reg_dmc_black_limit            = 0x100;                     // 12 bit , format 10.2
    //pDataInfo->reg_dmc_plane_level1           = phdr->PLANE[0] << 4;     // 12 bit , format 10.2
    //pDataInfo->reg_dmc_plane_level2           = phdr->PLANE[1] << 4;     // 12 bit , format 10.2
    //pDataInfo->reg_dmc_plane_level3           = phdr->PLANE[2] << 4;     // 12 bit , format 10.2
    //pDataInfo->reg_dmc_plane_level4           = phdr->PLANE[3] << 4;     // 12 bit , format 10.2
    //pDataInfo->reg_dmc_plane_level5           = phdr->PLANE[4] << 4;     // 12 bit , format 10.2
    //pDataInfo->reg_dmc_plane_level6           = 0;     // 12 bit , format 10.2
    //pDataInfo->reg_dmc_plane_level7           = 0;           // 12 bit , format 10.2
    //pDataInfo->reg_dmc_plane_level8           = 0;           // 12 bit , format 10.2
    //pDataInfo->reg_dmc_white_limit            = 0xEF0;       // 12 bit , format 10.2
    pDataInfo->reg_dmc_dither_en              = get_env_demura_dither();
    pDataInfo->reg_dmc_dither_rst_e_4_frame   = 0x0;         //  1 bit
    pDataInfo->reg_dmc_dither_pse_rst_num     = 0x0;         //  2 bit
    pDataInfo->bROI_en                        = 0x0;
    pDataInfo->iROI_hor_end_offset            = 0x0;
    pDataInfo->iROI_hor_start_offset          = 0x0;
    pDataInfo->iROI_ver_end_offset            = 0x0;
    pDataInfo->iROI_ver_start_offset          = 0x0;
    // Update CHOT demura header to mstar interface info
    pDataInfo->reg_dmc_plane_num = phdr->DEMURA_PLANE_NUM;
    pDataInfo->reg_dmc_black_limit = phdr->LOWER_BOUND<<2;
    pDataInfo->reg_dmc_plane_level1 = phdr->PLANE[0]<<2;
    pDataInfo->reg_dmc_plane_level2 = phdr->PLANE[1]<<2;
    pDataInfo->reg_dmc_plane_level3 = phdr->PLANE[2]<<2;
    pDataInfo->reg_dmc_plane_level4 = phdr->PLANE[3]<<2;
    pDataInfo->reg_dmc_plane_level5 = phdr->PLANE[4]<<2;
    pDataInfo->reg_dmc_plane_level6 = phdr->PLANE[5]<<2;
    pDataInfo->reg_dmc_plane_level7 = phdr->PLANE[6]<<2;
    pDataInfo->reg_dmc_plane_level8 = phdr->PLANE[7]<<2;
    pDataInfo->reg_dmc_white_limit = phdr->UPPER_BOUND<<2;

    switch(phdr->DEMURA_BLK_H)  // 0: 4x, 1: 8x, 2: 16x customer N -> mstar 2^n
    {
        case 0:
            pDataInfo->reg_dmc_h_block = 2;
            break;
        case 1:
            pDataInfo->reg_dmc_h_block = 3;
            break;
        case 2:
            pDataInfo->reg_dmc_h_block = 4;
            break;
        default:
            pDataInfo->reg_dmc_h_block = 3;
            break;
    }

    switch(phdr->DEMURA_BLK_V)  // 0: 4x, 1: 8x, 2: 16x customer N -> mstar 2^n
    {
        case 0:
            pDataInfo->reg_dmc_v_block = 2;
            break;
        case 1:
            pDataInfo->reg_dmc_v_block = 3;
            break;
        case 2:
            pDataInfo->reg_dmc_v_block = 4;
            break;
        default:
            pDataInfo->reg_dmc_v_block = 3;
            break;
    }
//=============================== End Demura Data Set //===============================
}

static MS_BOOL NOVA_Init_U13_Format_Lut_Buffer(int ***lut_buffer, CHOT_Demura_Header *phdr,
                                              interface_info *pinfo)
{
    int i, j;
    int ret = TRUE;

    CHECK_DMALLOC_SPACE(lut_buffer, (int)(pinfo->reg_dmc_plane_num * sizeof(int**)));
    for (i = 0; i < pinfo->reg_dmc_plane_num; i++)
    {
        lut_buffer[i] = (int **)dmalloc(phdr->DEMURA_TBL_V * sizeof(int*));
        if (lut_buffer[i] == NULL)
        {
            printf("In File(%s), Function(%s), Line(%d), Can Not get memory(0x%x bytes)\n",
                  __FILE__, __FUNCTION__, __LINE__, (int)(phdr->DEMURA_TBL_V * sizeof(int*)));
            ret = FALSE;
            goto FINISH;
        }
        for (j = 0; j < phdr->DEMURA_TBL_V; j++)
        {
            lut_buffer[i][j] = (int *)dmalloc(phdr->DEMURA_TBL_H * sizeof(int));
            if (lut_buffer[i][j] == NULL)
            {
                printf("In File(%s), Function(%s), Line(%d), Can Not get memory(0x%x bytes)\n",
                      __FILE__, __FUNCTION__, __LINE__, (int)(phdr->DEMURA_TBL_H * sizeof(int)));
                ret = FALSE;
                goto FINISH;
            }
        }
    }

FINISH:
    if (!ret)
    {
        for (i = 0; i < pinfo->reg_dmc_plane_num; i++)
        {
            for (j = 0; j < phdr->DEMURA_TBL_V; j++)
            {
                dfree(lut_buffer[i][j]);
            }
            dfree(lut_buffer[i]);
        }
        dfree(lut_buffer);
    }
    return ret;
}

static MS_BOOL NOVA_Set_U13_Format(CHOT_Demura_Header *phdr, interface_info *pinfo)
{
    // init morton interface data
    init_morton_data(phdr, pinfo);

    //dump_interface_info(pinfo);

    int ***Lut_buffer;
    int idx, i, j, k, data = 0;
    int table_addr = CHOT_LUT_START - CHOT_LUT_CHECKSUM; // Skip CRC
    MS_BOOL pol = 0;
    int ret;

    Lut_buffer = (int ***)dmalloc(pinfo->reg_dmc_plane_num * sizeof(int**));
    ret = NOVA_Init_U13_Format_Lut_Buffer(Lut_buffer, phdr, pinfo);
    if (!ret || (Lut_buffer == NULL))
    {
        return FALSE;
    }

    // assign LUT 12 bit format
    for (i = 0; i < phdr->DEMURA_TBL_V; i++)
    {
        for (j = 0; j < phdr->DEMURA_TBL_H ; j++)
        {
            for(k = 0; k < pinfo->reg_dmc_plane_num; k++)
            {
                if(pol == 0) //8bit + 4bit
                {
                    data = ((MS_U16)phdr->lut_buffer[table_addr]<<4) | (((MS_U16)phdr->lut_buffer[table_addr + 1]&0xf0)>>4);
                    pol = 1;
                    table_addr+=1;
                }
                else
                {
                    data = (((MS_U16)phdr->lut_buffer[table_addr] & 0x0f)<<8) | (((MS_U16)phdr->lut_buffer[table_addr + 1]&0xff));
                    pol = 0;
                    table_addr+=2;
                }
                // Update 12bit buffer
                if(data >= 0x800)
                    data = data - 0x1000; // signed bit
                Lut_buffer[k][i][j] = data;
            }
        }
    }

    for (idx = 0; idx < pinfo->reg_dmc_plane_num; idx++)
    {
        int nLayer = 0;
        switch (idx)
        {
            case 0:
                nLayer = pinfo->reg_dmc_plane_level1;
                break;
            case 1:
                nLayer = pinfo->reg_dmc_plane_level2;
                break;
            case 2:
                nLayer = pinfo->reg_dmc_plane_level3;
                break;
            case 3:
                nLayer = pinfo->reg_dmc_plane_level4;
                break;
            case 4:
                nLayer = pinfo->reg_dmc_plane_level5;
                break;
            case 5:
                nLayer = pinfo->reg_dmc_plane_level6;
                break;
            case 6:
                nLayer = pinfo->reg_dmc_plane_level7;
                break;
            case 7:
                nLayer = pinfo->reg_dmc_plane_level8;
                break;
            default:
                break;
        }
        //int **layer_value_lut = Lut_buffer[idx];

        int n_Hnode     = phdr->DEMURA_TBL_H;       // 481;
        int n_Vnode     = phdr->DEMURA_TBL_V;       // 271;

        int idx_image_size = 0;
        double LUT_data = 0;

        //int cnt = 0;

        for (i = 0; i < n_Vnode; i++)
        {
            for (j = 0; j < n_Hnode ; j++)
            {
                LUT_data = (double)(((((Lut_buffer[idx][i][j]))/4) + (nLayer/4)));//(double)(((((Lut_buffer[idx][i][j])&0xFFF)>>2) + (nLayer>>2))&0x3FF); // 12bit to 10bit
                // Lut_buffer is S12, nLayer is U12

                //pinfo->Lut_in[idx][idx_image_size].dbr   = LUT_data;//(double)(Lut_buffer[idx][i][j] + nLayer);
                //pinfo->Lut_in[idx][idx_image_size].dbg   = LUT_data;//(double)(Lut_buffer[idx][i][j] + nLayer);
                //pinfo->Lut_in[idx][idx_image_size].dbb   = LUT_data;//(double)(Lut_buffer[idx][i][j] + nLayer);
                //#if (defined (CONFIG_DEMURA_M7622) || defined (CONFIG_DEMURA_M7632))
                //pinfo->Lut_in[idx][idx_image_size].dbw   = LUT_data;
                //#endif

                //(pinfo->Lut_in[idx]+idx_image_size)->r   = (Lut_buffer[idx][i][j] + nLayer);//(pinfo->Lut_in[idx])->r   = (Lut_buffer[idx][i][j] + nLayer);
                pinfo->Lut_in[idx][idx_image_size].r   = (int)LUT_data;//(Lut_buffer[idx][i][j] + nLayer);
                pinfo->Lut_in[idx][idx_image_size].g   = (int)LUT_data;//(Lut_buffer[idx][i][j] + nLayer);
                pinfo->Lut_in[idx][idx_image_size].b   = (int)LUT_data;//(Lut_buffer[idx][i][j] + nLayer);
                #if (defined (CONFIG_DEMURA_M7622) || defined (CONFIG_DEMURA_M7632))
                pinfo->Lut_in[idx][idx_image_size].w   = (int)LUT_data;
                #endif
                idx_image_size += 1;
            }
        }
    }

    // Free buffer
    dfree(phdr->hdr_buffer);
    phdr->hdr_buffer = NULL;

    dfree(phdr->lut_buffer);
    phdr->lut_buffer = NULL;

    for (i = 0; i < pinfo->reg_dmc_plane_num; i++)
    {
        for (j = 0; j < phdr->DEMURA_TBL_V; j++)
        {
            dfree(Lut_buffer[i][j]);
        }
        dfree(Lut_buffer[i]);
    }
    dfree(Lut_buffer);

    return TRUE;
}
#endif
static MS_BOOL NOVA_Decode_To_Mstar_Format(CHOT_Demura_Header *phdr, interface_info *pinfo)
{
    MS_BOOL bRet = FALSE;

    #if defined (CONFIG_DEMURA_URSA11)
        bRet = NOVA_Set_U11_Format(phdr, pinfo);
    #else
        bRet = NOVA_Set_U13_Format(phdr, pinfo);
    #endif

    return bRet;
}

MS_BOOL Decode_To_Mstar_Format_NOVA(void *pDataInfo, BinOutputInfo *pbin_info, EN_DEMURA_MULTI_VENDOR multi_vendor, MS_U32 flash_offset)
{
    MS_BOOL bRet = FALSE;
    CHOT_Demura_Header header;

    // Parse header information
    memset(&header, 0, sizeof(header));
    if (load_vendor_header_chot(&header, flash_offset) != TRUE)
    {
        printf("load_vendor_header Error\n");
        return FALSE;
    }

    if (parse_vendor_header_chot(&header) != TRUE)
    {
        printf("parse_vendor_header Error\n");
        return FALSE;
    }
    //dump_vendor_header_chot(&header);

    if (load_vendor_lut_chot(&header, flash_offset) != TRUE)
    {
        printf("load_vendor_lut Error\n");
        dfree(header.hdr_buffer);
        header.hdr_buffer = NULL;
        return FALSE;
    }

    dump_vendor_header_chot(&header);

    Gen_Sf_Signature_NOVA(&header);

    // Alloc space for Lut_in
    pbin_info->HNode       =  header.DEMURA_TBL_H;
    pbin_info->VNode       =  header.DEMURA_TBL_V;
    pbin_info->LevelCount  =  header.DEMURA_PLANE_NUM;

    bRet = Alloc_LutIn_Space(pDataInfo, pbin_info);
    if (bRet == FALSE)
    {
        printf("Alloc_LutIn_Space error\n");
        dfree(header.hdr_buffer);
        header.hdr_buffer = NULL;
        dfree(header.lut_buffer);
        header.lut_buffer = NULL;
        return FALSE;
    }

    // Decode Vendor data
    bRet = NOVA_Decode_To_Mstar_Format(&header, (interface_info *)pDataInfo);
    if (bRet == FALSE)
    {
        printf("CHOT_Decode_To_Mstar_Format error\n");
        return FALSE;
    }

    //MS_BOOL demura_enable[3]  = {TRUE, TRUE, TRUE};

    pbin_info->reg_base_addr  =  0x7700;
    pbin_info->project_id     =  ((MSTAR_CHIP_ID << 16) + (1ULL << 15) + multi_vendor);
    pbin_info->data_type      =  IC_DRAM;
    pbin_info->Gain_type      =  10;
    pbin_info->Sep_type       =  0;
    pbin_info->Build_Date     =  0x19030614;

    //memcpy(pbin_info->channel_enable, demura_enable, sizeof(demura_enable));

    return TRUE;
}
// end NOVA

// LGD
MS_BOOL If_Need_Decode_LGD(EN_DEMURA_MULTI_VENDOR multi_vendor)
{
    UBOOT_TRACE("IN\n");
    if (init_spi_flash() != TRUE)
    {
        UBOOT_DEBUG("init_spi_flash error!\n");
        return TRUE;
    }
#if defined(CONFIG_DEMURA_VENDOR_BACKLIGHT)
    char *sig_str;
    if (E_MS_UTIL_BIN_FILE_BACKLIGHT == get_demura_file())
    {
        sig_str = env_get(ENV_DEMURA_SIG_BL);
    }
    else
    {
        sig_str = env_get(ENV_DEMURA_SIG);
    }
#else
    char *sig_str = env_get(ENV_DEMURA_SIG);
#endif

    if (sig_str == NULL)
    {
        UBOOT_DEBUG("Can not find env(%s)\n", ENV_DEMURA_SIG);
        return TRUE;
    }
    else
    {
        MS_U8  checksum_sf=0;
        MS_U32 checksum_bd=0;
        MS_U32 sample_crc32=0, cal_crc32=0;
        MS_U8  buf[SAMPLE_DAT_LEN]={0};
        MS_U32 lgd_checksum_adr=0, lgd_mura_start_adr=0;

        if ((multi_vendor == EN_DEMURA_MULTI_LGD_V19_COLOR) || (multi_vendor == EN_DEMURA_MULTI_LGD_V19_MONO))
        {
            lgd_checksum_adr = 0xA1A3C;
            lgd_mura_start_adr = 0x80000;
        }
        else if ((multi_vendor == EN_DEMURA_MULTI_LGD_SQ18_19_COLOR) || (multi_vendor == EN_DEMURA_MULTI_LGD_SQ18_19_MONO))
        {
            lgd_checksum_adr = 0x5C310;
            lgd_mura_start_adr = 0x0;
        }
        else if(multi_vendor == EN_DEMURA_MULTI_LGD_V18)
        {
            lgd_checksum_adr = 0x685f0;
            lgd_mura_start_adr = 0x60000;
        }

        if (read_spi_flash(&checksum_sf, lgd_checksum_adr, sizeof(checksum_sf)) == FALSE)
        {
            UBOOT_ERROR("Read LGD Demura Checksum Failed !\n");
            return TRUE;
        }
        UBOOT_DEBUG("checksum_sf  = 0x%x\n", (uint)checksum_sf);

        UBOOT_DEBUG("sig_str = %s\n", sig_str);
        if(2 == sscanf(sig_str, "0x%x:0x%x", &checksum_bd, &sample_crc32))
        {
            UBOOT_DEBUG("checksum_bd  = 0x%x\n", (uint)checksum_bd);
            UBOOT_DEBUG("sample_crc32 = 0x%x\n", (uint)sample_crc32);

            if (read_spi_flash(buf, (lgd_mura_start_adr + SAMPLE_DAT_START), SAMPLE_DAT_LEN) == TRUE)
            {
                cal_crc32 = MDrv_CRC32_Cal_DeMura(buf, SAMPLE_DAT_LEN);
                UBOOT_DEBUG("cal_crc32 = 0x%x\n", (uint)cal_crc32);
                if ((sample_crc32 == cal_crc32) && (checksum_bd == checksum_sf))
                {
                    UBOOT_DEBUG("Data Match, No Need to decode again\n");
                    UBOOT_TRACE("OK\n");
                    return FALSE;
                }
            }
        }
        UBOOT_TRACE("OK\n");
        return TRUE;
    }

    UBOOT_TRACE("OK\n");
    return FALSE;
}

MS_BOOL Decode_To_Mstar_Format_LGD(void *pDataInfo, BinOutputInfo *pbin_info, EN_DEMURA_MULTI_VENDOR multi_vendor)
{
    MS_BOOL bRet = FALSE;

#ifndef CONFIG_DEMURA_LGD_DEMO
    bRet = parse_LG_bin(pDataInfo, pbin_info);
    if (bRet == FALSE)
    {
        UBOOT_ERROR("parse_LG_bin error\n");
        return FALSE;
    }

    // Decode Vendor data
    bRet = set_u13_interface(pDataInfo, pbin_info);
    if (bRet == FALSE)
    {
        UBOOT_ERROR("set_u13_interface error\n");
        return FALSE;
    }
#else
    bRet = transfer_LGD_Data_1tab(pDataInfo, pbin_info, multi_vendor);
    if (bRet == FALSE)
    {
        UBOOT_ERROR("transfer_LGD_Data_1tab error\n");
        return FALSE;
    }
#endif

    //MS_BOOL demura_enable[3]  = {TRUE, TRUE, TRUE};

    pbin_info->Build_Date     =  0x18032315;  // 2018.03.23
    pbin_info->reg_base_addr  =  0x7700;
    pbin_info->project_id     =  ((MSTAR_CHIP_ID << 16) + (1ULL << 15) + multi_vendor);
    pbin_info->data_type      =  IC_DRAM;
    pbin_info->Gain_type      =  10;
    pbin_info->Vendor_ID      =  EN_DEMURA_ID_LGD;

    //memcpy(pbin_info->channel_enable, demura_enable, sizeof(demura_enable));
    return TRUE;
}

#if defined (CONFIG_DEMURA_URSA11)
static MS_BOOL check_dmc_parameter_error(int h_size, int v_size, char* error_msg)
{
    MS_BOOL dmc_parameter_error = FALSE;

    if(h_size==4)
    {
         dmc_parameter_error = TRUE;
         error_msg = "block_h_siz error : 4 pixel\n";
    }

    if(v_size==4)
    {
        dmc_parameter_error = TRUE;
        error_msg = "block_v_siz error : 4 pixel\n";
    }

    return dmc_parameter_error;
}

static MS_BOOL set_u11_interface(AUO_Demura_Header *phdr, dmc_registers *pdmc_reg, void *pDataInfo, BinOutputInfo *pbin_info)
{
    MS_BOOL dmc_parameter_error_flag = FALSE;
    char *dmc_parameter_error_msg = "";

    interface_info *DataInfo   = (interface_info *)pDataInfo;
    interface_info *R_Info_Out = &(DataInfo[0]);
    interface_info *G_Info_Out = &(DataInfo[1]);
    interface_info *B_Info_Out = &(DataInfo[2]);

    DataInfo[0].bOut_dither_en = TRUE;
    DataInfo[1].bOut_dither_en = TRUE;
    DataInfo[2].bOut_dither_en = TRUE;


    //++++++++++++ LUT H/V Size start +++++++++++//
    int Mstar_lut_h_size         = register_16bits_inv( (*pdmc_reg).reg_dmc_h_lut_num, 7, 4);
    int Mstar_lut_v_size         = register_16bits_inv( (*pdmc_reg).reg_dmc_v_lut_num, 2, 8);
    //++++++++++++ LUT H/V Size end ++++++++++++//

    //++++++++++++ Block H/V Size start ++++++++++++//
    int Mstar_block_h_size       = 1<<(register_08bits_inv( (*pdmc_reg).reg_dmc_h_block, 3));
    int Mstar_block_v_size       = 1<<(register_08bits_inv( (*pdmc_reg).reg_dmc_v_block, 3));
    pbin_info->Blk_h_size        = Mstar_block_h_size;
    pbin_info->Blk_v_size        = Mstar_block_v_size;

    dmc_parameter_error_flag = check_dmc_parameter_error(Mstar_block_h_size, Mstar_block_v_size, dmc_parameter_error_msg);

    //++++++++++++ Block H/V Size end ++++++++++++//

    //++++++++++++ Y mode (Mono mode) start ++++++++++++//
    int Mstar_y_mode_en          = (register_08bits_inv( (*pdmc_reg).reg_dmc_rgb_mode, 1)==1) ? 0 : 1;
    pbin_info->Sep_type          = ( Mstar_y_mode_en == 1 ? 0 : 1);
    //++++++++++++ Y mode (Mono mode) end ++++++++++++//

    (*R_Info_Out).iChannel = 0;
    (*G_Info_Out).iChannel = 1;
    (*B_Info_Out).iChannel = 2;

    //++++++++++++ layerX level enable start ++++++++++++//
    int layer_level_num      = register_08bits_inv( (*pdmc_reg).reg_dmc_plane_num, 4);

    layer_level_num = ((layer_level_num>=1)&&(layer_level_num<=8)) ? layer_level_num : 3;

    int idx;

    (*R_Info_Out).bLayer_level_en[0] = FALSE;
    (*G_Info_Out).bLayer_level_en[0] = FALSE;
    (*B_Info_Out).bLayer_level_en[0] = FALSE;

    (*R_Info_Out).bLayer_level_en[1] = FALSE;
    (*G_Info_Out).bLayer_level_en[1] = FALSE;
    (*B_Info_Out).bLayer_level_en[1] = FALSE;

    (*R_Info_Out).bLayer_level_en[2] = FALSE;
    (*G_Info_Out).bLayer_level_en[2] = FALSE;
    (*B_Info_Out).bLayer_level_en[2] = FALSE;

    (*R_Info_Out).bLayer_level_en[3] = FALSE;
    (*G_Info_Out).bLayer_level_en[3] = FALSE;
    (*B_Info_Out).bLayer_level_en[3] = FALSE;

    (*R_Info_Out).bLayer_level_en[4] = FALSE;
    (*G_Info_Out).bLayer_level_en[4] = FALSE;
    (*B_Info_Out).bLayer_level_en[4] = FALSE;

    (*R_Info_Out).bLayer_level_en[5] = FALSE;
    (*G_Info_Out).bLayer_level_en[5] = FALSE;
    (*B_Info_Out).bLayer_level_en[5] = FALSE;


    switch(layer_level_num)
    {
        case 0:
            break;
        case 1:
        case 2:
        case 3:
        case 4:
            for (idx = 0; idx < layer_level_num + 2; idx++)
            {
                (*R_Info_Out).bLayer_level_en[idx] = TRUE;
                (*G_Info_Out).bLayer_level_en[idx] = TRUE;
                (*B_Info_Out).bLayer_level_en[idx] = TRUE;
            }
            break;
        default:
            dmc_parameter_error_flag = TRUE;
            dmc_parameter_error_msg = "layer_level_num : over 4 layer level\n";
            break;
    }

    if (dmc_parameter_error_flag == TRUE)
    {
        UBOOT_ERROR("%s", dmc_parameter_error_msg);
        return FALSE;
    }
    //++++++++++++ layerX level enable end ++++++++++++//

    //++++++++++++ layerX level start ++++++++++++//
    (*R_Info_Out).iLayer_level[0] = 0;
    (*G_Info_Out).iLayer_level[0] = 0;
    (*B_Info_Out).iLayer_level[0] = 0;

    (*R_Info_Out).iLayer_level[1] = 1023;
    (*G_Info_Out).iLayer_level[1] = 1023;
    (*B_Info_Out).iLayer_level[1] = 1023;

    (*R_Info_Out).iLayer_level[2] = 1023;
    (*G_Info_Out).iLayer_level[2] = 1023;
    (*B_Info_Out).iLayer_level[2] = 1023;

    (*R_Info_Out).iLayer_level[3] = 1023;
    (*G_Info_Out).iLayer_level[3] = 1023;
    (*B_Info_Out).iLayer_level[3] = 1023;

    (*R_Info_Out).iLayer_level[4] = 1023;
    (*G_Info_Out).iLayer_level[4] = 1023;
    (*B_Info_Out).iLayer_level[4] = 1023;

    (*R_Info_Out).iLayer_level[5] = 1023;
    (*G_Info_Out).iLayer_level[5] = 1023;
    (*B_Info_Out).iLayer_level[5] = 1023;


    int black_level    = register_16bits_inv( (*pdmc_reg).reg_dmc_black_limit , 4, 8);    // 12bit (format 10.2)
    int layer_level1   = register_16bits_inv( (*pdmc_reg).reg_dmc_plane_level1, 4, 8);   // 12bit (format 10.2)
    int layer_level2   = register_16bits_inv( (*pdmc_reg).reg_dmc_plane_level2, 4, 8);   // 12bit (format 10.2)
    int layer_level3   = register_16bits_inv( (*pdmc_reg).reg_dmc_plane_level3, 4, 8);   // 12bit (format 10.2)
    int layer_level4   = register_16bits_inv( (*pdmc_reg).reg_dmc_plane_level4, 4, 8);   // 12bit (format 10.2)
    int white_level    = register_16bits_inv( (*pdmc_reg).reg_dmc_white_limit , 4, 8);    // 12bit (format 10.2)

    black_level  = ((black_level + 2)>>2);   // 12 bit -> 10 bit
    layer_level1 = ((layer_level1 + 2)>>2);  // 12 bit -> 10 bit
    layer_level2 = ((layer_level2 + 2)>>2);  // 12 bit -> 10 bit
    layer_level3 = ((layer_level3 + 2)>>2);  // 12 bit -> 10 bit
    layer_level4 = ((layer_level4 + 2)>>2);  // 12 bit -> 10 bit
    white_level  = ((white_level + 2)>>2);   // 12 bit -> 10 bit
    switch(layer_level_num)
    {
    case 1:
        (*R_Info_Out).iLayer_level[0] = black_level;
        (*R_Info_Out).iLayer_level[1] = layer_level1;
        (*R_Info_Out).iLayer_level[2] = white_level;

        (*G_Info_Out).iLayer_level[0] = black_level;
        (*G_Info_Out).iLayer_level[1] = layer_level1;
        (*G_Info_Out).iLayer_level[2] = white_level;

        (*B_Info_Out).iLayer_level[0] = black_level;
        (*B_Info_Out).iLayer_level[1] = layer_level1;
        (*B_Info_Out).iLayer_level[2] = white_level;
        break;
    case 2:
        (*R_Info_Out).iLayer_level[0] = black_level;
        (*R_Info_Out).iLayer_level[1] = layer_level1;
        (*R_Info_Out).iLayer_level[2] = layer_level2;
        (*R_Info_Out).iLayer_level[3] = white_level;

        (*G_Info_Out).iLayer_level[0] = black_level;
        (*G_Info_Out).iLayer_level[1] = layer_level1;
        (*G_Info_Out).iLayer_level[2] = layer_level2;
        (*G_Info_Out).iLayer_level[3] = white_level;

        (*B_Info_Out).iLayer_level[0] = black_level;
        (*B_Info_Out).iLayer_level[1] = layer_level1;
        (*B_Info_Out).iLayer_level[2] = layer_level2;
        (*B_Info_Out).iLayer_level[3] = white_level;
        break;
    case 3:
        (*R_Info_Out).iLayer_level[0] = black_level;
        (*R_Info_Out).iLayer_level[1] = layer_level1;
        (*R_Info_Out).iLayer_level[2] = layer_level2;
        (*R_Info_Out).iLayer_level[3] = layer_level3;
        (*R_Info_Out).iLayer_level[4] = white_level;

        (*G_Info_Out).iLayer_level[0] = black_level;
        (*G_Info_Out).iLayer_level[1] = layer_level1;
        (*G_Info_Out).iLayer_level[2] = layer_level2;
        (*G_Info_Out).iLayer_level[3] = layer_level3;
        (*G_Info_Out).iLayer_level[4] = white_level;

        (*B_Info_Out).iLayer_level[0] = black_level;
        (*B_Info_Out).iLayer_level[1] = layer_level1;
        (*B_Info_Out).iLayer_level[2] = layer_level2;
        (*B_Info_Out).iLayer_level[3] = layer_level3;
        (*B_Info_Out).iLayer_level[4] = white_level;
        break;
    case 4:
        (*R_Info_Out).iLayer_level[0] = black_level;
        (*R_Info_Out).iLayer_level[1] = layer_level1;
        (*R_Info_Out).iLayer_level[2] = layer_level2;
        (*R_Info_Out).iLayer_level[3] = layer_level3;
        (*R_Info_Out).iLayer_level[4] = layer_level4;
        (*R_Info_Out).iLayer_level[5] = white_level;

        (*G_Info_Out).iLayer_level[0] = black_level;
        (*G_Info_Out).iLayer_level[1] = layer_level1;
        (*G_Info_Out).iLayer_level[2] = layer_level2;
        (*G_Info_Out).iLayer_level[3] = layer_level3;
        (*G_Info_Out).iLayer_level[4] = layer_level4;
        (*G_Info_Out).iLayer_level[5] = white_level;

        (*B_Info_Out).iLayer_level[0] = black_level;
        (*B_Info_Out).iLayer_level[1] = layer_level1;
        (*B_Info_Out).iLayer_level[2] = layer_level2;
        (*B_Info_Out).iLayer_level[3] = layer_level3;
        (*B_Info_Out).iLayer_level[4] = layer_level4;
        (*B_Info_Out).iLayer_level[5] = white_level;
        break;
    }
    //++++++++++++ layerX level end ++++++++++++//

    //++++++++++++ low luma weight start ++++++++++++//
    (*R_Info_Out).bLow_luma_en         = FALSE;
    (*R_Info_Out).iLow_luma_thrd       = 0x00;
    (*R_Info_Out).iLow_luma_slope      = 0x0F;
    (*R_Info_Out).iLow_luma_min_weight = 0x0;

    (*G_Info_Out).bLow_luma_en         = FALSE;
    (*G_Info_Out).iLow_luma_thrd       = 0x00;
    (*G_Info_Out).iLow_luma_slope      = 0x0F;
    (*G_Info_Out).iLow_luma_min_weight = 0x0;

    (*B_Info_Out).bLow_luma_en         = FALSE;
    (*B_Info_Out).iLow_luma_thrd       = 0x00;
    (*B_Info_Out).iLow_luma_slope      = 0x0F;
    (*B_Info_Out).iLow_luma_min_weight = 0x0;
    //++++++++++++ low luma weight end ++++++++++++//

    //++++++++++++ high luma weight start ++++++++++++//
    (*R_Info_Out).bHigh_luma_en         = FALSE;
    (*R_Info_Out).iHigh_luma_thrd       = 0xFF;
    (*R_Info_Out).iHigh_luma_slope      = 0x0F;
    (*R_Info_Out).iHigh_luma_min_weight = 0x0;

    (*G_Info_Out).bHigh_luma_en         = FALSE;
    (*G_Info_Out).iHigh_luma_thrd       = 0xFF;
    (*G_Info_Out).iHigh_luma_slope      = 0x0F;
    (*G_Info_Out).iHigh_luma_min_weight = 0x0;

    (*B_Info_Out).bHigh_luma_en         = FALSE;
    (*B_Info_Out).iHigh_luma_thrd       = 0xFF;
    (*B_Info_Out).iHigh_luma_slope      = 0x0F;
    (*B_Info_Out).iHigh_luma_min_weight = 0x0;
    //++++++++++++ high luma weight end ++++++++++++//
    //---------------------- DMC register to Mstar register end ----------------------//


    //---------------------- DMC LUT Mapping to Mstar LUT start ----------------------//
    unsigned char *AUO_IN_DATA    =  phdr->lut_buffer;
    int dmc_lut_start_addr        =  phdr->lut_offset;

    for (idx = 0; idx < 6; idx++)
    {
        int cnt_idx;
        for (cnt_idx = 0; cnt_idx < (Mstar_lut_h_size * Mstar_lut_v_size); cnt_idx++)
        {
            (*R_Info_Out).Lut_in[idx][cnt_idx] = 0.0;
            (*G_Info_Out).Lut_in[idx][cnt_idx] = 0.0;
            (*B_Info_Out).Lut_in[idx][cnt_idx] = 0.0;
        }
    }

    if((layer_level_num>=1)&&(layer_level_num<=4))
    {
        int layer_level_idx;
        for (layer_level_idx = 1; layer_level_idx <= layer_level_num; layer_level_idx++)
        {

            int r_layer_gain=0;
            int g_layer_gain=0;
            int b_layer_gain=0;

            int r_layer_offset=0;
            int g_layer_offset=0;
            int b_layer_offset=0;

            switch(layer_level_idx)
            {
            case 1:
                r_layer_gain = register_08bits_inv((*pdmc_reg).reg_dmc_data_r_mag1, 3);
                g_layer_gain = register_08bits_inv((*pdmc_reg).reg_dmc_data_g_mag1, 3);
                b_layer_gain = register_08bits_inv((*pdmc_reg).reg_dmc_data_b_mag1, 3);

                r_layer_offset = register_16bits_inv((*pdmc_reg).reg_dmc_data_r_offset1, 6, 8);
                g_layer_offset = register_16bits_inv((*pdmc_reg).reg_dmc_data_g_offset1, 6, 8);
                b_layer_offset = register_16bits_inv((*pdmc_reg).reg_dmc_data_b_offset1, 6, 8);
                break;
            case 2:
                r_layer_gain = register_08bits_inv((*pdmc_reg).reg_dmc_data_r_mag2, 3);
                g_layer_gain = register_08bits_inv((*pdmc_reg).reg_dmc_data_g_mag2, 3);
                b_layer_gain = register_08bits_inv((*pdmc_reg).reg_dmc_data_b_mag2, 3);

                r_layer_offset = register_16bits_inv((*pdmc_reg).reg_dmc_data_r_offset2, 6, 8);
                g_layer_offset = register_16bits_inv((*pdmc_reg).reg_dmc_data_g_offset2, 6, 8);
                b_layer_offset = register_16bits_inv((*pdmc_reg).reg_dmc_data_b_offset2, 6, 8);
                break;
            case 3:
                r_layer_gain = register_08bits_inv((*pdmc_reg).reg_dmc_data_r_mag3, 3);
                g_layer_gain = register_08bits_inv((*pdmc_reg).reg_dmc_data_g_mag3, 3);
                b_layer_gain = register_08bits_inv((*pdmc_reg).reg_dmc_data_b_mag3, 3);

                r_layer_offset = register_16bits_inv((*pdmc_reg).reg_dmc_data_r_offset3, 6, 8);
                g_layer_offset = register_16bits_inv((*pdmc_reg).reg_dmc_data_g_offset3, 6, 8);
                b_layer_offset = register_16bits_inv((*pdmc_reg).reg_dmc_data_b_offset3, 6, 8);
                break;
            case 4:
                r_layer_gain = register_08bits_inv((*pdmc_reg).reg_dmc_data_r_mag4, 3);
                g_layer_gain = register_08bits_inv((*pdmc_reg).reg_dmc_data_g_mag4, 3);
                b_layer_gain = register_08bits_inv((*pdmc_reg).reg_dmc_data_b_mag4, 3);

                r_layer_offset = register_16bits_inv((*pdmc_reg).reg_dmc_data_r_offset4, 6, 8);
                g_layer_offset = register_16bits_inv((*pdmc_reg).reg_dmc_data_g_offset4, 6, 8);
                b_layer_offset = register_16bits_inv((*pdmc_reg).reg_dmc_data_b_offset4, 6, 8);
                break;
            }

            int one_layer_lut_HxV_size = (Mstar_y_mode_en==1) ? ( Mstar_lut_h_size * Mstar_lut_v_size) : ( Mstar_lut_h_size * Mstar_lut_v_size*3);
            int auo_dmc_lut_address_idx_offset = (Mstar_y_mode_en==1) ? 1 : 3;

            int auo_dmc_lut_address_idx;
            for (auo_dmc_lut_address_idx = 0; auo_dmc_lut_address_idx < one_layer_lut_HxV_size; auo_dmc_lut_address_idx = auo_dmc_lut_address_idx + auo_dmc_lut_address_idx_offset)
            {
                int real_dmc_lut_address = (one_layer_lut_HxV_size*(layer_level_idx - 1)) + auo_dmc_lut_address_idx + dmc_lut_start_addr;

                if(Mstar_y_mode_en==1)
                {
                    // Mono mode
                    // DMC parameters only use R-Channel register data of DMC_DATA_R_MAGn and DMC_DATA_R_OFFSETn in Mono mode.
                    int lut_value = (int)AUO_IN_DATA[real_dmc_lut_address];

                    double mstar_lut_in_double = DMC_LUT_Fix_To_Double( r_layer_offset, r_layer_gain, lut_value);

                    int cnt_idx = auo_dmc_lut_address_idx;
                    (*R_Info_Out).Lut_in[layer_level_idx][cnt_idx]       = mstar_lut_in_double;
                    (*G_Info_Out).Lut_in[layer_level_idx][cnt_idx]       = mstar_lut_in_double;
                    (*B_Info_Out).Lut_in[layer_level_idx][cnt_idx]       = mstar_lut_in_double;

//printf("real_dmc_lut_address = 0x%x, r_layer_offset = 0x%x, r_layer_gain = 0x%x, ", real_dmc_lut_address, r_layer_offset, r_layer_gain);
//printf("0x%x_0x%x : %lf\n", layer_level_idx, cnt_idx, mstar_lut_in_double);
//printf("lut_value = 0x%x, 0x%x_0x%x : %lf\n", lut_value, layer_level_idx, cnt_idx, mstar_lut_in_double);

                }
                else
                {
                    // RGB mode
                    int r_lut_value = (int)AUO_IN_DATA[real_dmc_lut_address];
                    int g_lut_value = (int)AUO_IN_DATA[real_dmc_lut_address+1];
                    int b_lut_value = (int)AUO_IN_DATA[real_dmc_lut_address+2];

                    double mstar_r_lut_in_double = DMC_LUT_Fix_To_Double( r_layer_offset, r_layer_gain, r_lut_value);
                    double mstar_g_lut_in_double = DMC_LUT_Fix_To_Double( g_layer_offset, g_layer_gain, g_lut_value);
                    double mstar_b_lut_in_double = DMC_LUT_Fix_To_Double( b_layer_offset, b_layer_gain, b_lut_value);

                    int cnt_idx = (int)((double)auo_dmc_lut_address_idx/(double)3);

                    (*R_Info_Out).Lut_in[layer_level_idx][cnt_idx]       = mstar_r_lut_in_double;
                    (*G_Info_Out).Lut_in[layer_level_idx][cnt_idx]       = mstar_g_lut_in_double;
                    (*B_Info_Out).Lut_in[layer_level_idx][cnt_idx]       = mstar_b_lut_in_double;
                }
            }
        }
    }
    //---------------------- DMC LUT Mapping to Mstar LUT end ----------------------//
    return TRUE;
}
#elif defined (CONFIG_DEMURA_MT5896)
static MS_BOOL set_u13_interface(AUO_Demura_Header *phdr, dmc_registers *pdmc_reg, void *pDataInfo, BinOutputInfo *pbin_info)
{
    //MS_BOOL dmc_parameter_error_flag = FALSE;
    //char *dmc_parameter_error_msg = "";

    interface_info *pInfo_Out = (interface_info *)pDataInfo;
    double mstar_lut_in_double = 0;

    //++++++++++++ LUT H/V Size start +++++++++++//
    int Mstar_lut_h_size         = pdmc_reg->reg_dmc_h_lut_num.val;
    int Mstar_lut_v_size         = pdmc_reg->reg_dmc_v_lut_num.val;
    //++++++++++++ LUT H/V Size end ++++++++++++//
//printf("[%ds:%d] Mstar_lut_h_size = %d, Mstar_lut_v_size = %d\n", __FUNCTION__, __LINE__, Mstar_lut_h_size, Mstar_lut_v_size);
    //++++++++++++ Block H/V Size start ++++++++++++//
    (*pInfo_Out).reg_dmc_h_block = pdmc_reg->reg_dmc_h_block.val;
    (*pInfo_Out).reg_dmc_v_block = pdmc_reg->reg_dmc_v_block.val;
    pbin_info->Blk_h_size        = (*pInfo_Out).reg_dmc_h_block;
    pbin_info->Blk_v_size        = (*pInfo_Out).reg_dmc_v_block;

    (*pInfo_Out).reg_dmc_panel_h_size = (Mstar_lut_h_size - 1) * (1<<(pdmc_reg->reg_dmc_h_block.val));

//printf("[%ds:%d] reg_dmc_h_block = %d, reg_dmc_v_block = %d\n", __FUNCTION__, __LINE__, (*pInfo_Out).reg_dmc_h_block, (*pInfo_Out).reg_dmc_v_block);
//printf("[%ds:%d] pbin_info->Blk_h_size = %d, pbin_info->Blk_v_size = %d\n", __FUNCTION__, __LINE__, pbin_info->Blk_h_size, pbin_info->Blk_v_size);

    //++++++++++++ Block H/V Size end ++++++++++++//

    //++++++++++++ Y mode (Mono mode) start ++++++++++++//
    (*pInfo_Out).reg_dmc_rgb_mode   = (pdmc_reg->reg_dmc_rgb_mode.val == 1) ? 1 : 0;

    pbin_info->Sep_type          = (*pInfo_Out).reg_dmc_rgb_mode;

    //++++++++++++ Y mode (Mono mode) end ++++++++++++//


    //++++++++++++ layerX level enable start ++++++++++++//
    int layer_level_num      = pdmc_reg->reg_dmc_plane_num.val;
//printf("[%ds:%d] (reg_dmc_rgb_mode = %d, layer_level_num = %d\n", __FUNCTION__, __LINE__, (*pInfo_Out).reg_dmc_rgb_mode, layer_level_num);

    layer_level_num = ((layer_level_num>=1)&&(layer_level_num<=8)) ? layer_level_num : 3;
    (*pInfo_Out).reg_dmc_plane_num = layer_level_num;

    switch(layer_level_num)
    {
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
    case 8:
        break;
    default:
        //dmc_parameter_error_flag = TRUE;
        UBOOT_TRACE("layer_level_num : over 8 layer level\n");
        break;
    }
    //++++++++++++ layerX level enable end ++++++++++++//

    //++++++++++++ layerX level start ++++++++++++//
    (*pInfo_Out).reg_dmc_black_limit  = 0x100;       // 12 bit , format 10.2
    (*pInfo_Out).reg_dmc_plane_level1 = 0;
    (*pInfo_Out).reg_dmc_plane_level2 = 0;
    (*pInfo_Out).reg_dmc_plane_level3 = 0;
    (*pInfo_Out).reg_dmc_plane_level4 = 0;
    (*pInfo_Out).reg_dmc_plane_level5 = 0;
    (*pInfo_Out).reg_dmc_plane_level6 = 0;
    (*pInfo_Out).reg_dmc_plane_level7 = 0;
    (*pInfo_Out).reg_dmc_plane_level8 = 0;
    (*pInfo_Out).reg_dmc_white_limit  = 0xEF0;       // 12 bit , format 10.2

    int black_level    = pdmc_reg->reg_dmc_black_limit.val;
    int layer_level1   = pdmc_reg->reg_dmc_plane_level1.val;
    int layer_level2   = pdmc_reg->reg_dmc_plane_level2.val;
    int layer_level3   = pdmc_reg->reg_dmc_plane_level3.val;
    int layer_level4   = pdmc_reg->reg_dmc_plane_level4.val;
    int layer_level5   = pdmc_reg->reg_dmc_plane_level5.val;
    int layer_level6   = pdmc_reg->reg_dmc_plane_level6.val;
    int white_level    = pdmc_reg->reg_dmc_white_limit.val;

    switch(layer_level_num)
    {
    case 1:
        (*pInfo_Out).reg_dmc_black_limit  = black_level;       // 12 bit , format 10.2
        (*pInfo_Out).reg_dmc_plane_level1 = layer_level1;
        (*pInfo_Out).reg_dmc_white_limit  = white_level;       // 12 bit , format 10.2
        break;
    case 2:
        (*pInfo_Out).reg_dmc_black_limit  = black_level;       // 12 bit , format 10.2
        (*pInfo_Out).reg_dmc_plane_level1 = layer_level1;
        (*pInfo_Out).reg_dmc_plane_level2 = layer_level2;
        (*pInfo_Out).reg_dmc_white_limit  = white_level;       // 12 bit , format 10.2
        break;
    case 3:
        (*pInfo_Out).reg_dmc_black_limit  = black_level;       // 12 bit , format 10.2
        (*pInfo_Out).reg_dmc_plane_level1 = layer_level1;
        (*pInfo_Out).reg_dmc_plane_level2 = layer_level2;
        (*pInfo_Out).reg_dmc_plane_level3 = layer_level3;
        (*pInfo_Out).reg_dmc_white_limit  = white_level;       // 12 bit , format 10.2
        break;
    case 4:
        (*pInfo_Out).reg_dmc_black_limit  = black_level;       // 12 bit , format 10.2
        (*pInfo_Out).reg_dmc_plane_level1 = layer_level1;
        (*pInfo_Out).reg_dmc_plane_level2 = layer_level2;
        (*pInfo_Out).reg_dmc_plane_level3 = layer_level3;
        (*pInfo_Out).reg_dmc_plane_level4 = layer_level4;
        (*pInfo_Out).reg_dmc_white_limit  = white_level;       // 12 bit , format 10.2
        break;
    case 5:
        (*pInfo_Out).reg_dmc_black_limit  = black_level;       // 12 bit , format 10.2
        (*pInfo_Out).reg_dmc_plane_level1 = layer_level1;
        (*pInfo_Out).reg_dmc_plane_level2 = layer_level2;
        (*pInfo_Out).reg_dmc_plane_level3 = layer_level3;
        (*pInfo_Out).reg_dmc_plane_level4 = layer_level4;
        (*pInfo_Out).reg_dmc_plane_level5 = layer_level5;
        (*pInfo_Out).reg_dmc_white_limit  = white_level;       // 12 bit , format 10.2
        break;
    case 6:
        (*pInfo_Out).reg_dmc_black_limit  = black_level;       // 12 bit , format 10.2
        (*pInfo_Out).reg_dmc_plane_level1 = layer_level1;
        (*pInfo_Out).reg_dmc_plane_level2 = layer_level2;
        (*pInfo_Out).reg_dmc_plane_level3 = layer_level3;
        (*pInfo_Out).reg_dmc_plane_level4 = layer_level4;
        (*pInfo_Out).reg_dmc_plane_level5 = layer_level5;
        (*pInfo_Out).reg_dmc_plane_level6 = layer_level6;
        (*pInfo_Out).reg_dmc_white_limit  = white_level;       // 12 bit , format 10.2
        break;
    default :
        //dmc_parameter_error_flag = TRUE;
        UBOOT_TRACE("layer_level_num : over 6 layer level\n");
        break;
    }

    //++++++++++++ layerX level end ++++++++++++//

    (*pInfo_Out).reg_dmc_dither_en              = get_env_demura_dither();
    (*pInfo_Out).reg_dmc_dither_rst_e_4_frame   = 0x0;         //  1 bit
    (*pInfo_Out).reg_dmc_dither_pse_rst_num     = 0x0;         //  2 bit
    //---------------------- DMC register to Mstar register end ----------------------//


    //---------------------- DMC LUT Mapping to Mstar LUT start ----------------------//
    unsigned char *AUO_IN_DATA    =  phdr->lut_buffer;
    int dmc_lut_start_addr        =  phdr->lut_offset;

    // Init to zero value
    int idx;
    for (idx = 0; idx < layer_level_num; idx++)
    {
        int cnt_idx;
        for (cnt_idx = 0; cnt_idx < Mstar_lut_h_size*Mstar_lut_v_size; cnt_idx++)
        {
            //(*pInfo_Out).Lut_in[idx][cnt_idx].dbr = 0.0;
            //(*pInfo_Out).Lut_in[idx][cnt_idx].dbg = 0.0;
            //(*pInfo_Out).Lut_in[idx][cnt_idx].dbb = 0.0;

            (*pInfo_Out).Lut_in[idx][cnt_idx].r = 0;
            (*pInfo_Out).Lut_in[idx][cnt_idx].g = 0;
            (*pInfo_Out).Lut_in[idx][cnt_idx].b = 0;
        }
    }

    if((layer_level_num>=1)&&(layer_level_num<=6))
    {
        int layer_level_idx;
        for (layer_level_idx = 1; layer_level_idx <= layer_level_num; layer_level_idx++)
        {
            int r_layer_gain=0;
            int g_layer_gain=0;
            int b_layer_gain=0;

            int r_layer_offset=0;
            int g_layer_offset=0;
            int b_layer_offset=0;

            int layer_level = 0;  // layer level 10bit data path

            switch(layer_level_idx)
            {
            case 1:
                r_layer_gain = pdmc_reg->reg_dmc_data_r_mag1.val;
                g_layer_gain = pdmc_reg->reg_dmc_data_g_mag1.val;
                b_layer_gain = pdmc_reg->reg_dmc_data_b_mag1.val;

                r_layer_offset = pdmc_reg->reg_dmc_data_r_offset1.val;
                g_layer_offset = pdmc_reg->reg_dmc_data_g_offset1.val;
                b_layer_offset = pdmc_reg->reg_dmc_data_b_offset1.val;

                layer_level = layer_level1 >> 2;
                break;
            case 2:
                r_layer_gain = pdmc_reg->reg_dmc_data_r_mag2.val;
                g_layer_gain = pdmc_reg->reg_dmc_data_g_mag2.val;
                b_layer_gain = pdmc_reg->reg_dmc_data_b_mag2.val;

                r_layer_offset = pdmc_reg->reg_dmc_data_r_offset2.val;
                g_layer_offset = pdmc_reg->reg_dmc_data_g_offset2.val;
                b_layer_offset = pdmc_reg->reg_dmc_data_b_offset2.val;

                layer_level = layer_level2 >> 2;
                break;
            case 3:
                r_layer_gain = pdmc_reg->reg_dmc_data_r_mag3.val;
                g_layer_gain = pdmc_reg->reg_dmc_data_g_mag3.val;
                b_layer_gain = pdmc_reg->reg_dmc_data_b_mag3.val;

                r_layer_offset = pdmc_reg->reg_dmc_data_r_offset3.val;
                g_layer_offset = pdmc_reg->reg_dmc_data_g_offset3.val;
                b_layer_offset = pdmc_reg->reg_dmc_data_b_offset3.val;

                layer_level = layer_level3 >> 2;
                break;
            case 4:
                r_layer_gain = pdmc_reg->reg_dmc_data_r_mag4.val;
                g_layer_gain = pdmc_reg->reg_dmc_data_g_mag4.val;
                b_layer_gain = pdmc_reg->reg_dmc_data_b_mag4.val;

                r_layer_offset = pdmc_reg->reg_dmc_data_r_offset4.val;
                g_layer_offset = pdmc_reg->reg_dmc_data_g_offset4.val;
                b_layer_offset = pdmc_reg->reg_dmc_data_b_offset4.val;

                layer_level = layer_level4 >> 2;
                break;
             case 5:
                r_layer_gain = pdmc_reg->reg_dmc_data_r_mag5.val;
                g_layer_gain = pdmc_reg->reg_dmc_data_g_mag5.val;
                b_layer_gain = pdmc_reg->reg_dmc_data_b_mag5.val;

                r_layer_offset = pdmc_reg->reg_dmc_data_r_offset5.val;
                g_layer_offset = pdmc_reg->reg_dmc_data_g_offset5.val;
                b_layer_offset = pdmc_reg->reg_dmc_data_b_offset5.val;

                layer_level = layer_level5 >> 2;
                break;
             case 6:
                r_layer_gain = pdmc_reg->reg_dmc_data_r_mag6.val;
                g_layer_gain = pdmc_reg->reg_dmc_data_g_mag6.val;
                b_layer_gain = pdmc_reg->reg_dmc_data_b_mag6.val;

                r_layer_offset = pdmc_reg->reg_dmc_data_r_offset6.val;
                g_layer_offset = pdmc_reg->reg_dmc_data_g_offset6.val;
                b_layer_offset = pdmc_reg->reg_dmc_data_b_offset6.val;

                layer_level = layer_level6 >> 2;
            }

            int one_layer_lut_HxV_size = ((*pInfo_Out).reg_dmc_rgb_mode == 0) ? (Mstar_lut_h_size*Mstar_lut_v_size) : (Mstar_lut_h_size*Mstar_lut_v_size*3);
            int auo_dmc_lut_address_idx_offset = ((*pInfo_Out).reg_dmc_rgb_mode == 0) ? 1 : 3;

            int auo_dmc_lut_address_idx;
            for (auo_dmc_lut_address_idx = 0; auo_dmc_lut_address_idx < one_layer_lut_HxV_size; auo_dmc_lut_address_idx = auo_dmc_lut_address_idx + auo_dmc_lut_address_idx_offset)
            {
                int real_dmc_lut_address = (one_layer_lut_HxV_size*(layer_level_idx - 1)) + auo_dmc_lut_address_idx + dmc_lut_start_addr;

                if((*pInfo_Out).reg_dmc_rgb_mode == 0)
                {
                    // Mono mode
                    // DMC parameters only use R-Channel register data of DMC_DATA_R_MAGn and DMC_DATA_R_OFFSETn in Mono mode.
                    int lut_value = (int)AUO_IN_DATA[real_dmc_lut_address];

                    if(Demura_Original_Mode == E_RGB_MODE)
                    {
                        mstar_lut_in_double = DMC_LUT_Fix_To_Double( g_layer_offset, g_layer_gain, lut_value);
                    }
                    else
                    {
                        mstar_lut_in_double = DMC_LUT_Fix_To_Double( r_layer_offset, r_layer_gain, lut_value);
                    }

                    int cnt_idx = auo_dmc_lut_address_idx;
                    //(*pInfo_Out).Lut_in[layer_level_idx-1][cnt_idx].dbr     = mstar_lut_in_double  + layer_level;
                    //(*pInfo_Out).Lut_in[layer_level_idx-1][cnt_idx].dbg     = mstar_lut_in_double + layer_level;
                    //(*pInfo_Out).Lut_in[layer_level_idx-1][cnt_idx].dbb     = mstar_lut_in_double + layer_level;

                    (*pInfo_Out).Lut_in[layer_level_idx-1][cnt_idx].r       = (int)mstar_lut_in_double + layer_level;
                    (*pInfo_Out).Lut_in[layer_level_idx-1][cnt_idx].g       = (int)mstar_lut_in_double + layer_level;
                    (*pInfo_Out).Lut_in[layer_level_idx-1][cnt_idx].b       = (int)mstar_lut_in_double + layer_level;
                }
                else
                {
                    // RGB mode
                    int r_lut_value = (int)AUO_IN_DATA[real_dmc_lut_address];
                    int g_lut_value = (int)AUO_IN_DATA[real_dmc_lut_address+1];
                    int b_lut_value = (int)AUO_IN_DATA[real_dmc_lut_address+2];

                    double mstar_r_lut_in_double = DMC_LUT_Fix_To_Double( r_layer_offset, r_layer_gain, r_lut_value);
                    double mstar_g_lut_in_double = DMC_LUT_Fix_To_Double( g_layer_offset, g_layer_gain, g_lut_value);
                    double mstar_b_lut_in_double = DMC_LUT_Fix_To_Double( b_layer_offset, b_layer_gain, b_lut_value);

                    int cnt_idx = (int)((double)auo_dmc_lut_address_idx/(double)3);

                    //(*pInfo_Out).Lut_in[layer_level_idx-1][cnt_idx].dbr     = mstar_r_lut_in_double + layer_level;
                    //(*pInfo_Out).Lut_in[layer_level_idx-1][cnt_idx].dbg     = mstar_g_lut_in_double + layer_level;
                    //(*pInfo_Out).Lut_in[layer_level_idx-1][cnt_idx].dbb     = mstar_b_lut_in_double + layer_level;

                    (*pInfo_Out).Lut_in[layer_level_idx-1][cnt_idx].r       = (int)mstar_r_lut_in_double + layer_level;
                    (*pInfo_Out).Lut_in[layer_level_idx-1][cnt_idx].g       = (int)mstar_g_lut_in_double + layer_level;
                    (*pInfo_Out).Lut_in[layer_level_idx-1][cnt_idx].b       = (int)mstar_b_lut_in_double + layer_level;
                }
            }
        }
    }

    //---------------------- DMC LUT Mapping to Mstar LUT end ----------------------//
    return TRUE;
}
#else

static MS_BOOL set_u13_interface(AUO_Demura_Header *phdr, dmc_registers *pdmc_reg, void *pDataInfo, BinOutputInfo *pbin_info)
{
    MS_BOOL dmc_parameter_error_flag = FALSE;
    char *dmc_parameter_error_msg = "";

    interface_info *pInfo_Out = (interface_info *)pDataInfo;
    double mstar_lut_in_double = 0;

    //++++++++++++ LUT H/V Size start +++++++++++//
    int Mstar_lut_h_size         = register_16bits_inv( (*pdmc_reg).reg_dmc_h_lut_num, 7, 4);
    int Mstar_lut_v_size         = register_16bits_inv( (*pdmc_reg).reg_dmc_v_lut_num, 2, 8);
    //++++++++++++ LUT H/V Size end ++++++++++++//
printf("[%ds:%d] Mstar_lut_h_size = %d, Mstar_lut_v_size = %d\n", __FUNCTION__, __LINE__, Mstar_lut_h_size, Mstar_lut_v_size);
    //++++++++++++ Block H/V Size start ++++++++++++//
    (*pInfo_Out).reg_dmc_h_block = register_08bits_inv( (*pdmc_reg).reg_dmc_h_block, 3);
    (*pInfo_Out).reg_dmc_v_block = register_08bits_inv( (*pdmc_reg).reg_dmc_v_block, 3);
    pbin_info->Blk_h_size        = (*pInfo_Out).reg_dmc_h_block;
    pbin_info->Blk_v_size        = (*pInfo_Out).reg_dmc_v_block;

    (*pInfo_Out).reg_dmc_panel_h_size = (Mstar_lut_h_size - 1) * (1<<(register_08bits_inv( (*pdmc_reg).reg_dmc_h_block, 3)));

printf("[%ds:%d] reg_dmc_h_block = %d, reg_dmc_v_block = %d\n", __FUNCTION__, __LINE__, (*pInfo_Out).reg_dmc_h_block, (*pInfo_Out).reg_dmc_v_block);
printf("[%ds:%d] pbin_info->Blk_h_size = %d, pbin_info->Blk_v_size = %d\n", __FUNCTION__, __LINE__, pbin_info->Blk_h_size, pbin_info->Blk_v_size);

    //++++++++++++ Block H/V Size end ++++++++++++//

    //++++++++++++ Y mode (Mono mode) start ++++++++++++//
    (*pInfo_Out).reg_dmc_rgb_mode   = (register_08bits_inv( (*pdmc_reg).reg_dmc_rgb_mode, 1)==1) ? 1 : 0;

    pbin_info->Sep_type          = (*pInfo_Out).reg_dmc_rgb_mode;

    //++++++++++++ Y mode (Mono mode) end ++++++++++++//


    //++++++++++++ layerX level enable start ++++++++++++//
    int layer_level_num      = register_08bits_inv( (*pdmc_reg).reg_dmc_plane_num, 4);
printf("[%ds:%d] (reg_dmc_rgb_mode = %d, layer_level_num = %d\n", __FUNCTION__, __LINE__, (*pInfo_Out).reg_dmc_rgb_mode, player_level_num);

    layer_level_num = ((layer_level_num>=1)&&(layer_level_num<=8)) ? layer_level_num : 3;
    (*pInfo_Out).reg_dmc_plane_num = layer_level_num;

    switch(layer_level_num)
    {
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
    case 8:
        break;
    default:
        dmc_parameter_error_flag = TRUE;
        dmc_parameter_error_msg = "layer_level_num : over 8 layer level\n";
        break;
    }
    //++++++++++++ layerX level enable end ++++++++++++//

    //++++++++++++ layerX level start ++++++++++++//
    (*pInfo_Out).reg_dmc_black_limit  = 0x100;       // 12 bit , format 10.2
    (*pInfo_Out).reg_dmc_plane_level1 = 0;
    (*pInfo_Out).reg_dmc_plane_level2 = 0;
    (*pInfo_Out).reg_dmc_plane_level3 = 0;
    (*pInfo_Out).reg_dmc_plane_level4 = 0;
    (*pInfo_Out).reg_dmc_plane_level5 = 0;
    (*pInfo_Out).reg_dmc_plane_level6 = 0;
    (*pInfo_Out).reg_dmc_plane_level7 = 0;
    (*pInfo_Out).reg_dmc_plane_level8 = 0;
    (*pInfo_Out).reg_dmc_white_limit  = 0xEF0;       // 12 bit , format 10.2

    int black_level    = register_16bits_inv( (*pdmc_reg).reg_dmc_black_limit , 4, 8);    // 12bit (format 10.2)
    int layer_level1   = register_16bits_inv( (*pdmc_reg).reg_dmc_plane_level1, 4, 8);   // 12bit (format 10.2)
    int layer_level2   = register_16bits_inv( (*pdmc_reg).reg_dmc_plane_level2, 4, 8);   // 12bit (format 10.2)
    int layer_level3   = register_16bits_inv( (*pdmc_reg).reg_dmc_plane_level3, 4, 8);   // 12bit (format 10.2)
    int layer_level4   = register_16bits_inv( (*pdmc_reg).reg_dmc_plane_level4, 4, 8);   // 12bit (format 10.2)
    int layer_level5   = register_16bits_inv( (*pdmc_reg).reg_dmc_plane_level5, 4, 8);   // 12bit (format 10.2)
    int layer_level6   = register_16bits_inv( (*pdmc_reg).reg_dmc_plane_level6, 4, 8);   // 12bit (format 10.2)
    int white_level    = register_16bits_inv( (*pdmc_reg).reg_dmc_white_limit , 4, 8);   // 12bit (format 10.2)

    switch(layer_level_num)
    {
    case 1:
        (*pInfo_Out).reg_dmc_black_limit  = black_level;       // 12 bit , format 10.2
        (*pInfo_Out).reg_dmc_plane_level1 = layer_level1;
        (*pInfo_Out).reg_dmc_white_limit  = white_level;       // 12 bit , format 10.2
        break;
    case 2:
        (*pInfo_Out).reg_dmc_black_limit  = black_level;       // 12 bit , format 10.2
        (*pInfo_Out).reg_dmc_plane_level1 = layer_level1;
        (*pInfo_Out).reg_dmc_plane_level2 = layer_level2;
        (*pInfo_Out).reg_dmc_white_limit  = white_level;       // 12 bit , format 10.2
        break;
    case 3:
        (*pInfo_Out).reg_dmc_black_limit  = black_level;       // 12 bit , format 10.2
        (*pInfo_Out).reg_dmc_plane_level1 = layer_level1;
        (*pInfo_Out).reg_dmc_plane_level2 = layer_level2;
        (*pInfo_Out).reg_dmc_plane_level3 = layer_level3;
        (*pInfo_Out).reg_dmc_white_limit  = white_level;       // 12 bit , format 10.2
        break;
    case 4:
        (*pInfo_Out).reg_dmc_black_limit  = black_level;       // 12 bit , format 10.2
        (*pInfo_Out).reg_dmc_plane_level1 = layer_level1;
        (*pInfo_Out).reg_dmc_plane_level2 = layer_level2;
        (*pInfo_Out).reg_dmc_plane_level3 = layer_level3;
        (*pInfo_Out).reg_dmc_plane_level4 = layer_level4;
        (*pInfo_Out).reg_dmc_white_limit  = white_level;       // 12 bit , format 10.2
        break;
    case 5:
        (*pInfo_Out).reg_dmc_black_limit  = black_level;       // 12 bit , format 10.2
        (*pInfo_Out).reg_dmc_plane_level1 = layer_level1;
        (*pInfo_Out).reg_dmc_plane_level2 = layer_level2;
        (*pInfo_Out).reg_dmc_plane_level3 = layer_level3;
        (*pInfo_Out).reg_dmc_plane_level4 = layer_level4;
        (*pInfo_Out).reg_dmc_plane_level5 = layer_level5;
        (*pInfo_Out).reg_dmc_white_limit  = white_level;       // 12 bit , format 10.2
        break;
    case 6:
        (*pInfo_Out).reg_dmc_black_limit  = black_level;       // 12 bit , format 10.2
        (*pInfo_Out).reg_dmc_plane_level1 = layer_level1;
        (*pInfo_Out).reg_dmc_plane_level2 = layer_level2;
        (*pInfo_Out).reg_dmc_plane_level3 = layer_level3;
        (*pInfo_Out).reg_dmc_plane_level4 = layer_level4;
        (*pInfo_Out).reg_dmc_plane_level5 = layer_level5;
        (*pInfo_Out).reg_dmc_plane_level6 = layer_level6;
        (*pInfo_Out).reg_dmc_white_limit  = white_level;       // 12 bit , format 10.2
        break;
    default :
        dmc_parameter_error_flag = TRUE;
        dmc_parameter_error_msg = "layer_level_num : over 6 layer level\n";
        break;
    }

    //++++++++++++ layerX level end ++++++++++++//

    (*pInfo_Out).reg_dmc_dither_en              = get_env_demura_dither();
    (*pInfo_Out).reg_dmc_dither_rst_e_4_frame   = 0x0;         //  1 bit
    (*pInfo_Out).reg_dmc_dither_pse_rst_num     = 0x0;         //  2 bit
    //---------------------- DMC register to Mstar register end ----------------------//


    //---------------------- DMC LUT Mapping to Mstar LUT start ----------------------//
    unsigned char *AUO_IN_DATA    =  phdr->lut_buffer;
    int dmc_lut_start_addr        =  phdr->lut_offset;

    // Init to zero value
    int idx;
    for (idx = 0; idx < layer_level_num; idx++)
    {
        int cnt_idx;
        for (cnt_idx = 0; cnt_idx < Mstar_lut_h_size*Mstar_lut_v_size; cnt_idx++)
        {
            //(*pInfo_Out).Lut_in[idx][cnt_idx].dbr = 0.0;
            //(*pInfo_Out).Lut_in[idx][cnt_idx].dbg = 0.0;
            //(*pInfo_Out).Lut_in[idx][cnt_idx].dbb = 0.0;

            (*pInfo_Out).Lut_in[idx][cnt_idx].r = 0;
            (*pInfo_Out).Lut_in[idx][cnt_idx].g = 0;
            (*pInfo_Out).Lut_in[idx][cnt_idx].b = 0;
        }
    }

    if((layer_level_num>=1)&&(layer_level_num<=6))
    {
        int layer_level_idx;
        for (layer_level_idx = 1; layer_level_idx <= layer_level_num; layer_level_idx++)
        {
            int r_layer_gain=0;
            int g_layer_gain=0;
            int b_layer_gain=0;

            int r_layer_offset=0;
            int g_layer_offset=0;
            int b_layer_offset=0;

            int layer_level = 0;  // layer level 10bit data path

            switch(layer_level_idx)
            {
            case 1:
                r_layer_gain = register_08bits_inv((*pdmc_reg).reg_dmc_data_r_mag1, 3);
                g_layer_gain = register_08bits_inv((*pdmc_reg).reg_dmc_data_g_mag1, 3);
                b_layer_gain = register_08bits_inv((*pdmc_reg).reg_dmc_data_b_mag1, 3);

                r_layer_offset = register_16bits_inv((*pdmc_reg).reg_dmc_data_r_offset1, 6, 8);
                g_layer_offset = register_16bits_inv((*pdmc_reg).reg_dmc_data_g_offset1, 6, 8);
                b_layer_offset = register_16bits_inv((*pdmc_reg).reg_dmc_data_b_offset1, 6, 8);

                layer_level = layer_level1 >> 2;
                break;
            case 2:
                r_layer_gain = register_08bits_inv((*pdmc_reg).reg_dmc_data_r_mag2, 3);
                g_layer_gain = register_08bits_inv((*pdmc_reg).reg_dmc_data_g_mag2, 3);
                b_layer_gain = register_08bits_inv((*pdmc_reg).reg_dmc_data_b_mag2, 3);

                r_layer_offset = register_16bits_inv((*pdmc_reg).reg_dmc_data_r_offset2, 6, 8);
                g_layer_offset = register_16bits_inv((*pdmc_reg).reg_dmc_data_g_offset2, 6, 8);
                b_layer_offset = register_16bits_inv((*pdmc_reg).reg_dmc_data_b_offset2, 6, 8);

                layer_level = layer_level2 >> 2;
                break;
            case 3:
                r_layer_gain = register_08bits_inv((*pdmc_reg).reg_dmc_data_r_mag3, 3);
                g_layer_gain = register_08bits_inv((*pdmc_reg).reg_dmc_data_g_mag3, 3);
                b_layer_gain = register_08bits_inv((*pdmc_reg).reg_dmc_data_b_mag3, 3);

                r_layer_offset = register_16bits_inv((*pdmc_reg).reg_dmc_data_r_offset3, 6, 8);
                g_layer_offset = register_16bits_inv((*pdmc_reg).reg_dmc_data_g_offset3, 6, 8);
                b_layer_offset = register_16bits_inv((*pdmc_reg).reg_dmc_data_b_offset3, 6, 8);

                layer_level = layer_level3 >> 2;
                break;
            case 4:
                r_layer_gain = register_08bits_inv((*pdmc_reg).reg_dmc_data_r_mag4, 3);
                g_layer_gain = register_08bits_inv((*pdmc_reg).reg_dmc_data_g_mag4, 3);
                b_layer_gain = register_08bits_inv((*pdmc_reg).reg_dmc_data_b_mag4, 3);

                r_layer_offset = register_16bits_inv((*pdmc_reg).reg_dmc_data_r_offset4, 6, 8);
                g_layer_offset = register_16bits_inv((*pdmc_reg).reg_dmc_data_g_offset4, 6, 8);
                b_layer_offset = register_16bits_inv((*pdmc_reg).reg_dmc_data_b_offset4, 6, 8);

                layer_level = layer_level4 >> 2;
                break;
             case 5:
                r_layer_gain = register_08bits_inv((*pdmc_reg).reg_dmc_data_r_mag5, 3);
                g_layer_gain = register_08bits_inv((*pdmc_reg).reg_dmc_data_g_mag5, 3);
                b_layer_gain = register_08bits_inv((*pdmc_reg).reg_dmc_data_b_mag5, 3);

                r_layer_offset = register_16bits_inv((*pdmc_reg).reg_dmc_data_r_offset5, 6, 8);
                g_layer_offset = register_16bits_inv((*pdmc_reg).reg_dmc_data_g_offset5, 6, 8);
                b_layer_offset = register_16bits_inv((*pdmc_reg).reg_dmc_data_b_offset5, 6, 8);

                layer_level = layer_level5 >> 2;
                break;
             case 6:
                r_layer_gain = register_08bits_inv((*pdmc_reg).reg_dmc_data_r_mag6, 3);
                g_layer_gain = register_08bits_inv((*pdmc_reg).reg_dmc_data_g_mag6, 3);
                b_layer_gain = register_08bits_inv((*pdmc_reg).reg_dmc_data_b_mag6, 3);

                r_layer_offset = register_16bits_inv((*pdmc_reg).reg_dmc_data_r_offset6, 6, 8);
                g_layer_offset = register_16bits_inv((*pdmc_reg).reg_dmc_data_g_offset6, 6, 8);
                b_layer_offset = register_16bits_inv((*pdmc_reg).reg_dmc_data_b_offset6, 6, 8);

                layer_level = layer_level6 >> 2;
            }

            int one_layer_lut_HxV_size = ((*pInfo_Out).reg_dmc_rgb_mode == 0) ? (Mstar_lut_h_size*Mstar_lut_v_size) : (Mstar_lut_h_size*Mstar_lut_v_size*3);
            int auo_dmc_lut_address_idx_offset = ((*pInfo_Out).reg_dmc_rgb_mode == 0) ? 1 : 3;

            int auo_dmc_lut_address_idx;
            for (auo_dmc_lut_address_idx = 0; auo_dmc_lut_address_idx < one_layer_lut_HxV_size; auo_dmc_lut_address_idx = auo_dmc_lut_address_idx + auo_dmc_lut_address_idx_offset)
            {
                int real_dmc_lut_address = (one_layer_lut_HxV_size*(layer_level_idx - 1)) + auo_dmc_lut_address_idx + dmc_lut_start_addr;

                if((*pInfo_Out).reg_dmc_rgb_mode == 0)
                {
                    // Mono mode
                    // DMC parameters only use R-Channel register data of DMC_DATA_R_MAGn and DMC_DATA_R_OFFSETn in Mono mode.
                    int lut_value = (int)AUO_IN_DATA[real_dmc_lut_address];

                    if(Demura_Original_Mode == E_RGB_MODE)
                    {
                        mstar_lut_in_double = DMC_LUT_Fix_To_Double( g_layer_offset, g_layer_gain, lut_value);
                    }
                    else
                    {
                        mstar_lut_in_double = DMC_LUT_Fix_To_Double( r_layer_offset, r_layer_gain, lut_value);
                    }

                    int cnt_idx = auo_dmc_lut_address_idx;
                    //(*pInfo_Out).Lut_in[layer_level_idx-1][cnt_idx].dbr     = mstar_lut_in_double  + layer_level;
                    //(*pInfo_Out).Lut_in[layer_level_idx-1][cnt_idx].dbg     = mstar_lut_in_double + layer_level;
                    //(*pInfo_Out).Lut_in[layer_level_idx-1][cnt_idx].dbb     = mstar_lut_in_double + layer_level;

                    (*pInfo_Out).Lut_in[layer_level_idx-1][cnt_idx].r       = (int)mstar_lut_in_double + layer_level;
                    (*pInfo_Out).Lut_in[layer_level_idx-1][cnt_idx].g       = (int)mstar_lut_in_double + layer_level;
                    (*pInfo_Out).Lut_in[layer_level_idx-1][cnt_idx].b       = (int)mstar_lut_in_double + layer_level;
                }
                else
                {
                    // RGB mode
                    int r_lut_value = (int)AUO_IN_DATA[real_dmc_lut_address];
                    int g_lut_value = (int)AUO_IN_DATA[real_dmc_lut_address+1];
                    int b_lut_value = (int)AUO_IN_DATA[real_dmc_lut_address+2];

                    double mstar_r_lut_in_double = DMC_LUT_Fix_To_Double( r_layer_offset, r_layer_gain, r_lut_value);
                    double mstar_g_lut_in_double = DMC_LUT_Fix_To_Double( g_layer_offset, g_layer_gain, g_lut_value);
                    double mstar_b_lut_in_double = DMC_LUT_Fix_To_Double( b_layer_offset, b_layer_gain, b_lut_value);

                    int cnt_idx = (int)((double)auo_dmc_lut_address_idx/(double)3);

                    //(*pInfo_Out).Lut_in[layer_level_idx-1][cnt_idx].dbr     = mstar_r_lut_in_double + layer_level;
                    //(*pInfo_Out).Lut_in[layer_level_idx-1][cnt_idx].dbg     = mstar_g_lut_in_double + layer_level;
                    //(*pInfo_Out).Lut_in[layer_level_idx-1][cnt_idx].dbb     = mstar_b_lut_in_double + layer_level;

                    (*pInfo_Out).Lut_in[layer_level_idx-1][cnt_idx].r       = (int)mstar_r_lut_in_double + layer_level;
                    (*pInfo_Out).Lut_in[layer_level_idx-1][cnt_idx].g       = (int)mstar_g_lut_in_double + layer_level;
                    (*pInfo_Out).Lut_in[layer_level_idx-1][cnt_idx].b       = (int)mstar_b_lut_in_double + layer_level;
                }
            }
        }
    }

    //---------------------- DMC LUT Mapping to Mstar LUT end ----------------------//
    return TRUE;
}

#endif

MS_BOOL AUO_Decode_To_Mstar_Format(AUO_Demura_Header *phdr, void *pDataInfo, BinOutputInfo *pbin_info)
{
    //---------------------- Capture DMC Parameter (register) start ----------------------//
    MS_BOOL bRet = FALSE;
    unsigned char *AUO_IN_DATA = phdr->hdr_buffer;

    //int dmc_para_start_addr = 0x10002; // (dec : 65538)
    //int auo_dmc_parameter_end_address  = 0x1006A; // (dec : 65642)
    int dmc_para_start_addr = phdr->hdr_offset; // (dec : 2)

#if 1
    dmc_registers _dmc_reg = {
        {REG_00A8_DEMURA_BKA377, 0x0001, REG_00A8_DEMURA_BKA377_REG_DMC_RGB_MODE},      //reg_dmc_rgb_mode;        //  1 bit
        {REG_00A8_DEMURA_BKA377, 0x0001, REG_00A8_DEMURA_BKA377_REG_DMC_BLOCK_SIZE},    //reg_dmc_block_size;      //  2 bit
        {REG_00A8_DEMURA_BKA377, 0x0003, REG_00A8_DEMURA_BKA377_REG_DMC_PLANE_NUM},     //reg_dmc_plane_num;       //  4 bit
        {REG_00A8_DEMURA_BKA377, 0x0003, REG_00A8_DEMURA_BKA377_REG_DMC_H_BLOCK},       //reg_dmc_h_block;         //  3 bit
        {REG_00A8_DEMURA_BKA377, 0x0003, REG_00A8_DEMURA_BKA377_REG_DMC_V_BLOCK},       //reg_dmc_v_block;         //  3 bit
        {REG_00AC_DEMURA_BKA377, 0x01E1, REG_00AC_DEMURA_BKA377_REG_DMC_H_LUT_NUM},     //reg_dmc_h_lut_num;       // 11 bit
        {REG_00B0_DEMURA_BKA377, 0x010F, REG_00B0_DEMURA_BKA377_REG_DMC_V_LUT_NUM},     //reg_dmc_v_lut_num;       // 10 bit
        {REG_00A4_DEMURA_BKA377, 0x0000, REG_00A4_DEMURA_BKA377_REG_DMC_PLANE_B1_KS22}, //reg_dmc_plane_b1_ks22;   //  1 bit
        {REG_00A4_DEMURA_BKA377, 0x0000, REG_00A4_DEMURA_BKA377_REG_DMC_PLANE_12_KS22}, //reg_dmc_plane_12_ks22;   //  1 bit
        {REG_00A4_DEMURA_BKA377, 0x0000, REG_00A4_DEMURA_BKA377_REG_DMC_PLANE_23_KS22}, //reg_dmc_plane_23_ks22;   //  1 bit
        {REG_00A4_DEMURA_BKA377, 0x0000, REG_00A4_DEMURA_BKA377_REG_DMC_PLANE_34_KS22}, //reg_dmc_plane_34_ks22;   //  1 bit
        {REG_00A4_DEMURA_BKA377, 0x0000, REG_00A4_DEMURA_BKA377_REG_DMC_PLANE_45_KS22}, //reg_dmc_plane_45_ks22;   //  1 bit
        {REG_00A4_DEMURA_BKA377, 0x0000, REG_00A4_DEMURA_BKA377_REG_DMC_PLANE_56_KS22}, //reg_dmc_plane_56_ks22;   //  1 bit
        {REG_00A4_DEMURA_BKA377, 0x0000, REG_00A4_DEMURA_BKA377_REG_DMC_PLANE_67_KS22}, //reg_dmc_plane_67_ks22;   //  1 bit
        {REG_00A4_DEMURA_BKA377, 0x0000, REG_00A4_DEMURA_BKA377_REG_DMC_PLANE_78_KS22}, //reg_dmc_plane_78_ks22;   //  1 bit
        {REG_00A4_DEMURA_BKA377, 0x0000, REG_00A4_DEMURA_BKA377_REG_DMC_PLANE_89_KS22}, //reg_dmc_plane_89_ks22;   //  1 bit
        {REG_00A4_DEMURA_BKA377, 0x0000, REG_00A4_DEMURA_BKA377_REG_DMC_PLANE_9W_KS22}, //reg_dmc_plane_9w_ks22;   //  1 bit
        {REG_0040_DEMURA_BKA377, 0x0000, REG_0040_DEMURA_BKA377_REG_DMC_BLACK_LIMIT},   //reg_dmc_black_limit;     // 12 bit
        {REG_0044_DEMURA_BKA377, 0x0001, REG_0044_DEMURA_BKA377_REG_DMC_PLANE_LEVEL1},  //reg_dmc_plane_level1;    // 12 bit
        {REG_0048_DEMURA_BKA377, 0x0FFF, REG_0048_DEMURA_BKA377_REG_DMC_PLANE_LEVEL2},  //reg_dmc_plane_level2;    // 12 bit
        {REG_004C_DEMURA_BKA377, 0x0FFF, REG_004C_DEMURA_BKA377_REG_DMC_PLANE_LEVEL3},  //reg_dmc_plane_level3;    // 12 bit
        {REG_0050_DEMURA_BKA377, 0x0FFF, REG_0050_DEMURA_BKA377_REG_DMC_PLANE_LEVEL4},  //reg_dmc_plane_level4;    // 12 bit
        {REG_0054_DEMURA_BKA377, 0x0FFF, REG_0054_DEMURA_BKA377_REG_DMC_PLANE_LEVEL5},  //reg_dmc_plane_level5;    // 12 bit
        {REG_0058_DEMURA_BKA377, 0x0FFF, REG_0058_DEMURA_BKA377_REG_DMC_PLANE_LEVEL6},  //reg_dmc_plane_level6;    // 12 bit
        {REG_005C_DEMURA_BKA377, 0x0FFF, REG_005C_DEMURA_BKA377_REG_DMC_PLANE_LEVEL7},  //reg_dmc_plane_level7;    // 12 bit
        {REG_0060_DEMURA_BKA377, 0x0FFF, REG_0060_DEMURA_BKA377_REG_DMC_PLANE_LEVEL8},  //reg_dmc_plane_level8;    // 12 bit
        {REG_0180_DEMURA_BKA377, 0x0FFF, REG_0180_DEMURA_BKA377_REG_DMC_PLANE_LEVEL9},  //reg_dmc_plane_level9;    // 12 bit
        {REG_0064_DEMURA_BKA377, 0x0FFF, REG_0064_DEMURA_BKA377_REG_DMC_WHITE_LIMIT},   //reg_dmc_white_limit;     // 12 bit
        {REG_0080_DEMURA_BKA377, 0x0000, REG_0080_DEMURA_BKA377_REG_DMC_PLANE_B1_COEF}, //reg_dmc_plane_b1_coef;   // 14 bit
        {REG_0084_DEMURA_BKA377, 0x0000, REG_0084_DEMURA_BKA377_REG_DMC_PLANE_12_COEF}, //reg_dmc_plane_12_coef;   // 14 bit
        {REG_0088_DEMURA_BKA377, 0x0000, REG_0088_DEMURA_BKA377_REG_DMC_PLANE_23_COEF}, //reg_dmc_plane_23_coef;   // 14 bit
        {REG_008C_DEMURA_BKA377, 0x0000, REG_008C_DEMURA_BKA377_REG_DMC_PLANE_34_COEF}, //reg_dmc_plane_34_coef;   // 14 bit
        {REG_0090_DEMURA_BKA377, 0x0000, REG_0090_DEMURA_BKA377_REG_DMC_PLANE_45_COEF}, //reg_dmc_plane_45_coef;   // 14 bit
        {REG_0094_DEMURA_BKA377, 0x0000, REG_0094_DEMURA_BKA377_REG_DMC_PLANE_56_COEF}, //reg_dmc_plane_56_coef;   // 14 bit
        {REG_0098_DEMURA_BKA377, 0x0000, REG_0098_DEMURA_BKA377_REG_DMC_PLANE_67_COEF}, //reg_dmc_plane_67_coef;   // 14 bit
        {REG_009C_DEMURA_BKA377, 0x0000, REG_009C_DEMURA_BKA377_REG_DMC_PLANE_78_COEF}, //reg_dmc_plane_78_coef;   // 14 bit
        {REG_0188_DEMURA_BKA377, 0x0000, REG_0188_DEMURA_BKA377_REG_DMC_PLANE_89_COEF}, //reg_dmc_plane_89_coef    // 14 bit
        {REG_00A0_DEMURA_BKA377, 0x0000, REG_00A0_DEMURA_BKA377_REG_DMC_PLANE_9W_COEF}, //reg_dmc_plane_9w_coef;   // 14 bit
        {REG_0068_DEMURA_BKA377, 0x0000, REG_0068_DEMURA_BKA377_REG_DMC_DATA_R_MAG1},   //reg_dmc_data_r_mag1;     //  3 bit
        {REG_0068_DEMURA_BKA377, 0x0000, REG_0068_DEMURA_BKA377_REG_DMC_DATA_R_MAG2},   //reg_dmc_data_r_mag2;     //  3 bit
        {REG_0068_DEMURA_BKA377, 0x0000, REG_0068_DEMURA_BKA377_REG_DMC_DATA_R_MAG3},   //reg_dmc_data_r_mag3;     //  3 bit
        {REG_0068_DEMURA_BKA377, 0x0000, REG_0068_DEMURA_BKA377_REG_DMC_DATA_R_MAG4},   //reg_dmc_data_r_mag4;     //  3 bit
        {REG_006C_DEMURA_BKA377, 0x0000, REG_006C_DEMURA_BKA377_REG_DMC_DATA_R_MAG5},   //reg_dmc_data_r_mag5;     //  3 bit
        {REG_006C_DEMURA_BKA377, 0x0000, REG_006C_DEMURA_BKA377_REG_DMC_DATA_R_MAG6},   //reg_dmc_data_r_mag6;     //  3 bit
        {REG_006C_DEMURA_BKA377, 0x0000, REG_006C_DEMURA_BKA377_REG_DMC_DATA_R_MAG7},   //reg_dmc_data_r_mag7;     //  3 bit
        {REG_006C_DEMURA_BKA377, 0x0000, REG_006C_DEMURA_BKA377_REG_DMC_DATA_R_MAG8},   //reg_dmc_data_r_mag8;     //  3 bit
        {REG_0184_DEMURA_BKA377, 0x0000, REG_0184_DEMURA_BKA377_REG_DMC_DATA_R_MAG9},   //reg_dmc_data_r_mag9;     //  3 bit
        {REG_0070_DEMURA_BKA377, 0x0000, REG_0070_DEMURA_BKA377_REG_DMC_DATA_G_MAG1},   //reg_dmc_data_g_mag1;     //  3 bit
        {REG_0070_DEMURA_BKA377, 0x0000, REG_0070_DEMURA_BKA377_REG_DMC_DATA_G_MAG2},   //reg_dmc_data_g_mag2;     //  3 bit
        {REG_0070_DEMURA_BKA377, 0x0000, REG_0070_DEMURA_BKA377_REG_DMC_DATA_G_MAG3},   //reg_dmc_data_g_mag3;     //  3 bit
        {REG_0070_DEMURA_BKA377, 0x0000, REG_0070_DEMURA_BKA377_REG_DMC_DATA_G_MAG4},   //reg_dmc_data_g_mag4;     //  3 bit
        {REG_0074_DEMURA_BKA377, 0x0000, REG_0074_DEMURA_BKA377_REG_DMC_DATA_G_MAG5},   //reg_dmc_data_g_mag5;     //  3 bit
        {REG_0074_DEMURA_BKA377, 0x0000, REG_0074_DEMURA_BKA377_REG_DMC_DATA_G_MAG6},   //reg_dmc_data_g_mag6;     //  3 bit
        {REG_0074_DEMURA_BKA377, 0x0000, REG_0074_DEMURA_BKA377_REG_DMC_DATA_G_MAG7},   //reg_dmc_data_g_mag7;     //  3 bit
        {REG_0074_DEMURA_BKA377, 0x0000, REG_0074_DEMURA_BKA377_REG_DMC_DATA_G_MAG8},   //reg_dmc_data_g_mag8;     //  3 bit
        {REG_0184_DEMURA_BKA377, 0x0000, REG_0184_DEMURA_BKA377_REG_DMC_DATA_G_MAG9},   //reg_dmc_data_g_mag9;     //  3 bit
        {REG_0078_DEMURA_BKA377, 0x0000, REG_0078_DEMURA_BKA377_REG_DMC_DATA_B_MAG1},   //reg_dmc_data_b_mag1;     //  3 bit
        {REG_0078_DEMURA_BKA377, 0x0000, REG_0078_DEMURA_BKA377_REG_DMC_DATA_B_MAG2},   //reg_dmc_data_b_mag2;     //  3 bit
        {REG_0078_DEMURA_BKA377, 0x0000, REG_0078_DEMURA_BKA377_REG_DMC_DATA_B_MAG3},   //reg_dmc_data_b_mag3;     //  3 bit
        {REG_0078_DEMURA_BKA377, 0x0000, REG_0078_DEMURA_BKA377_REG_DMC_DATA_B_MAG4},   //reg_dmc_data_b_mag4;     //  3 bit
        {REG_007C_DEMURA_BKA377, 0x0000, REG_007C_DEMURA_BKA377_REG_DMC_DATA_B_MAG5},   //reg_dmc_data_b_mag5;     //  3 bit
        {REG_007C_DEMURA_BKA377, 0x0000, REG_007C_DEMURA_BKA377_REG_DMC_DATA_B_MAG6},   //reg_dmc_data_b_mag6;     //  3 bit
        {REG_007C_DEMURA_BKA377, 0x0000, REG_007C_DEMURA_BKA377_REG_DMC_DATA_B_MAG7},   //reg_dmc_data_b_mag7;     //  3 bit
        {REG_007C_DEMURA_BKA377, 0x0000, REG_007C_DEMURA_BKA377_REG_DMC_DATA_B_MAG8},   //reg_dmc_data_b_mag8;     //  3 bit
        {REG_0184_DEMURA_BKA377, 0x0000, REG_0184_DEMURA_BKA377_REG_DMC_DATA_B_MAG9},   //reg_dmc_data_b_mag9;     //  3 bit
        {REG_00C0_DEMURA_BKA377, 0x0000, REG_00C0_DEMURA_BKA377_REG_DMC_DATA_R_OFFSET1}, //reg_dmc_data_r_offset1;   // 14 bit
        {REG_00C4_DEMURA_BKA377, 0x0000, REG_00C4_DEMURA_BKA377_REG_DMC_DATA_R_OFFSET2}, //reg_dmc_data_r_offset2;   // 14 bit
        {REG_00C8_DEMURA_BKA377, 0x0000, REG_00C8_DEMURA_BKA377_REG_DMC_DATA_R_OFFSET3}, //reg_dmc_data_r_offset3;   // 14 bit
        {REG_00CC_DEMURA_BKA377, 0x0000, REG_00CC_DEMURA_BKA377_REG_DMC_DATA_R_OFFSET4}, //reg_dmc_data_r_offset4;   // 14 bit
        {REG_00D0_DEMURA_BKA377, 0x0000, REG_00D0_DEMURA_BKA377_REG_DMC_DATA_R_OFFSET5}, //reg_dmc_data_r_offset5;   // 14 bit
        {REG_00D4_DEMURA_BKA377, 0x0000, REG_00D4_DEMURA_BKA377_REG_DMC_DATA_R_OFFSET6}, //reg_dmc_data_r_offset6;   // 14 bit
        {REG_00D8_DEMURA_BKA377, 0x0000, REG_00D8_DEMURA_BKA377_REG_DMC_DATA_R_OFFSET7}, //reg_dmc_data_r_offset7;   // 14 bit
        {REG_00DC_DEMURA_BKA377, 0x0000, REG_00DC_DEMURA_BKA377_REG_DMC_DATA_R_OFFSET8}, //reg_dmc_data_r_offset8;   // 14 bit
        {REG_018C_DEMURA_BKA377, 0x0000, REG_018C_DEMURA_BKA377_REG_DMC_DATA_R_OFFSET9}, //reg_dmc_data_r_offset9;   // 14 bit
        {REG_00E0_DEMURA_BKA377, 0x0000, REG_00E0_DEMURA_BKA377_REG_DMC_DATA_G_OFFSET1}, //reg_dmc_data_g_offset1;   // 14 bit
        {REG_00E4_DEMURA_BKA377, 0x0000, REG_00E4_DEMURA_BKA377_REG_DMC_DATA_G_OFFSET2}, //reg_dmc_data_g_offset2;   // 14 bit
        {REG_00E8_DEMURA_BKA377, 0x0000, REG_00E8_DEMURA_BKA377_REG_DMC_DATA_G_OFFSET3}, //reg_dmc_data_g_offset3;   // 14 bit
        {REG_00EC_DEMURA_BKA377, 0x0000, REG_00EC_DEMURA_BKA377_REG_DMC_DATA_G_OFFSET4}, //reg_dmc_data_g_offset4;   // 14 bit
        {REG_00F0_DEMURA_BKA377, 0x0000, REG_00F0_DEMURA_BKA377_REG_DMC_DATA_G_OFFSET5}, //reg_dmc_data_g_offset5;   // 14 bit
        {REG_00F4_DEMURA_BKA377, 0x0000, REG_00F4_DEMURA_BKA377_REG_DMC_DATA_G_OFFSET6}, //reg_dmc_data_g_offset6;   // 14 bit
        {REG_00F8_DEMURA_BKA377, 0x0000, REG_00F8_DEMURA_BKA377_REG_DMC_DATA_G_OFFSET7}, //reg_dmc_data_g_offset7;   // 14 bit
        {REG_00FC_DEMURA_BKA377, 0x0000, REG_00FC_DEMURA_BKA377_REG_DMC_DATA_G_OFFSET8}, //reg_dmc_data_g_offset8;   // 14 bit
        {REG_0190_DEMURA_BKA377, 0x0000, REG_0190_DEMURA_BKA377_REG_DMC_DATA_G_OFFSET9}, //reg_dmc_data_g_offset9;   // 14 bit
        {REG_0100_DEMURA_BKA377, 0x0000, REG_0100_DEMURA_BKA377_REG_DMC_DATA_B_OFFSET1}, //reg_dmc_data_b_offset1;   // 14 bit
        {REG_0104_DEMURA_BKA377, 0x0000, REG_0104_DEMURA_BKA377_REG_DMC_DATA_B_OFFSET2}, //reg_dmc_data_b_offset2;   // 14 bit
        {REG_0108_DEMURA_BKA377, 0x0000, REG_0108_DEMURA_BKA377_REG_DMC_DATA_B_OFFSET3}, //reg_dmc_data_b_offset3;   // 14 bit
        {REG_010C_DEMURA_BKA377, 0x0000, REG_010C_DEMURA_BKA377_REG_DMC_DATA_B_OFFSET4}, //reg_dmc_data_b_offset4;   // 14 bit
        {REG_0110_DEMURA_BKA377, 0x0000, REG_0110_DEMURA_BKA377_REG_DMC_DATA_B_OFFSET5}, //reg_dmc_data_b_offset5;   // 14 bit
        {REG_0114_DEMURA_BKA377, 0x0000, REG_0114_DEMURA_BKA377_REG_DMC_DATA_B_OFFSET6}, //reg_dmc_data_b_offset6;   // 14 bit
        {REG_0118_DEMURA_BKA377, 0x0000, REG_0118_DEMURA_BKA377_REG_DMC_DATA_B_OFFSET7}, //reg_dmc_data_b_offset7;   // 14 bit
        {REG_011C_DEMURA_BKA377, 0x0000, REG_011C_DEMURA_BKA377_REG_DMC_DATA_B_OFFSET8}, //reg_dmc_data_b_offset8;   // 14 bit
        {REG_0194_DEMURA_BKA377, 0x0000, REG_0194_DEMURA_BKA377_REG_DMC_DATA_B_OFFSET9}  //reg_dmc_data_b_offset9;   // 14 bit
    };

        //++++++++++++ Y mode (Mono mode) start ++++++++++++//
    if(Demura_Original_Mode == E_RGB_MODE)
        _dmc_reg.reg_dmc_rgb_mode.val = 0;
    else
        _dmc_reg.reg_dmc_rgb_mode.val = (AUO_IN_DATA[dmc_para_start_addr + 0] & 0x80) >> 7;
    //++++++++++++ Y mode (Mono mode) end ++++++++++++//

    _dmc_reg.reg_dmc_block_size.val = (AUO_IN_DATA[dmc_para_start_addr + 0] & 0x30) >> 4;
    _dmc_reg.reg_dmc_plane_num.val =  AUO_IN_DATA[dmc_para_start_addr + 0] & 0xF;


    //++++++++++++ Block H/V Size start ++++++++++++//
    _dmc_reg.reg_dmc_h_block.val               = (AUO_IN_DATA[dmc_para_start_addr + 1] & 0x70) >> 4;
    _dmc_reg.reg_dmc_v_block.val               =  AUO_IN_DATA[dmc_para_start_addr + 1] & 0x7;
    //++++++++++++ Block H/V Size end ++++++++++++//

    //++++++++++++ LUT H/V Size start +++++++++++//
    _dmc_reg.reg_dmc_h_lut_num.val = ((u16)AUO_IN_DATA[dmc_para_start_addr + 2] << 4)  | (AUO_IN_DATA[dmc_para_start_addr + 3] >>4);           // dmc_h_lut_num <-> Mstar_lut_h_size

    _dmc_reg.reg_dmc_v_lut_num.val = ((u16)(AUO_IN_DATA[dmc_para_start_addr + 3] & 3) << 8) | AUO_IN_DATA[dmc_para_start_addr + 4];           // dmc_v_lut_num <-> Mstar_lut_v_size
    //++++++++++++ LUT H/V Size end +++++++++++//

    //++++++++++++ layerX level start ++++++++++++//
    _dmc_reg.reg_dmc_black_limit.val = ((u16)AUO_IN_DATA[dmc_para_start_addr + 7] << 8) | AUO_IN_DATA[dmc_para_start_addr + 8];

    _dmc_reg.reg_dmc_plane_level1.val = ((u16)AUO_IN_DATA[dmc_para_start_addr + 9] << 8) | AUO_IN_DATA[dmc_para_start_addr + 10];
    _dmc_reg.reg_dmc_plane_level2.val = ((u16)AUO_IN_DATA[dmc_para_start_addr + 11] << 8) | AUO_IN_DATA[dmc_para_start_addr + 12];
    _dmc_reg.reg_dmc_plane_level3.val = ((u16)AUO_IN_DATA[dmc_para_start_addr + 13] << 8) | AUO_IN_DATA[dmc_para_start_addr + 14];
    _dmc_reg.reg_dmc_plane_level4.val = ((u16)AUO_IN_DATA[dmc_para_start_addr + 15] << 8) | AUO_IN_DATA[dmc_para_start_addr + 16];

    _dmc_reg.reg_dmc_white_limit.val = ((u16)AUO_IN_DATA[dmc_para_start_addr + 25] << 8) | AUO_IN_DATA[dmc_para_start_addr + 26];
    //++++++++++++ layerX level end ++++++++++++//

    //++++++++++++ layerX gain start ++++++++++++//
    _dmc_reg.reg_dmc_data_r_mag1.val = (AUO_IN_DATA[dmc_para_start_addr + 45] & 0x70) >> 4;
    _dmc_reg.reg_dmc_data_r_mag2.val = (AUO_IN_DATA[dmc_para_start_addr + 45] & 0x07);
    _dmc_reg.reg_dmc_data_r_mag3.val = (AUO_IN_DATA[dmc_para_start_addr + 46] & 0x70) >> 4;
    _dmc_reg.reg_dmc_data_r_mag4.val = (AUO_IN_DATA[dmc_para_start_addr + 46] & 0x07);
    _dmc_reg.reg_dmc_data_g_mag1.val = (AUO_IN_DATA[dmc_para_start_addr + 49] & 0x70) >> 4;
    _dmc_reg.reg_dmc_data_g_mag2.val = (AUO_IN_DATA[dmc_para_start_addr + 49] & 0x07);
    _dmc_reg.reg_dmc_data_g_mag3.val = (AUO_IN_DATA[dmc_para_start_addr + 50] & 0x70) >> 4;
    _dmc_reg.reg_dmc_data_g_mag4.val = (AUO_IN_DATA[dmc_para_start_addr + 50] & 0x07);
    _dmc_reg.reg_dmc_data_b_mag1.val = (AUO_IN_DATA[dmc_para_start_addr + 53] & 0x70) >> 4;
    _dmc_reg.reg_dmc_data_b_mag2.val = (AUO_IN_DATA[dmc_para_start_addr + 53] & 0x07);
    _dmc_reg.reg_dmc_data_b_mag3.val = (AUO_IN_DATA[dmc_para_start_addr + 54] & 0x70) >> 4;
    _dmc_reg.reg_dmc_data_b_mag4.val = (AUO_IN_DATA[dmc_para_start_addr + 54] & 0x07);
    //++++++++++++ layerX gain end ++++++++++++//

    //++++++++++++ layerX level offset start ++++++++++++//
    _dmc_reg.reg_dmc_data_r_offset1.val = (((u16)AUO_IN_DATA[dmc_para_start_addr + 57] & 0x3F) << 8) | AUO_IN_DATA[dmc_para_start_addr + 58];
    _dmc_reg.reg_dmc_data_r_offset2.val = (((u16)AUO_IN_DATA[dmc_para_start_addr + 59] & 0x3F) << 8) | AUO_IN_DATA[dmc_para_start_addr + 60];
    _dmc_reg.reg_dmc_data_r_offset3.val = (((u16)AUO_IN_DATA[dmc_para_start_addr + 61] & 0x3F) << 8) | AUO_IN_DATA[dmc_para_start_addr + 62];
    _dmc_reg.reg_dmc_data_r_offset4.val = (((u16)AUO_IN_DATA[dmc_para_start_addr + 63] & 0x3F) << 8) | AUO_IN_DATA[dmc_para_start_addr + 64];

    _dmc_reg.reg_dmc_data_g_offset1.val = (((u16)AUO_IN_DATA[dmc_para_start_addr + 73] & 0x3F) << 8) | AUO_IN_DATA[dmc_para_start_addr + 74];
    _dmc_reg.reg_dmc_data_g_offset2.val = (((u16)AUO_IN_DATA[dmc_para_start_addr + 75] & 0x3F) << 8) | AUO_IN_DATA[dmc_para_start_addr + 76];
    _dmc_reg.reg_dmc_data_g_offset3.val = (((u16)AUO_IN_DATA[dmc_para_start_addr + 77] & 0x3F) << 8) | AUO_IN_DATA[dmc_para_start_addr + 78];
    _dmc_reg.reg_dmc_data_g_offset4.val = (((u16)AUO_IN_DATA[dmc_para_start_addr + 79] & 0x3F) << 8) | AUO_IN_DATA[dmc_para_start_addr + 80];

    _dmc_reg.reg_dmc_data_b_offset1.val = (((u16)AUO_IN_DATA[dmc_para_start_addr + 89] & 0x3F) << 8) | AUO_IN_DATA[dmc_para_start_addr + 90];
    _dmc_reg.reg_dmc_data_b_offset2.val = (((u16)AUO_IN_DATA[dmc_para_start_addr + 91] & 0x3F) << 8) | AUO_IN_DATA[dmc_para_start_addr + 92];
    _dmc_reg.reg_dmc_data_b_offset3.val = (((u16)AUO_IN_DATA[dmc_para_start_addr + 93] & 0x3F) << 8) | AUO_IN_DATA[dmc_para_start_addr + 94];
    _dmc_reg.reg_dmc_data_b_offset4.val = (((u16)AUO_IN_DATA[dmc_para_start_addr + 95] & 0x3F) << 8) | AUO_IN_DATA[dmc_para_start_addr + 96];
    //++++++++++++ layerX level offset end ++++++++++++//
#else

    dmc_registers _dmc_reg = {
        {{dmc_para_start_addr +   0, 0x80, 0x80, "reg_dmc_rgb_mode"      }},
        {{dmc_para_start_addr +   0, 0x30, 0x10, "reg_dmc_block_size"    }},
        {{dmc_para_start_addr +   0, 0x0F, 0x03, "reg_dmc_plane_num"     }},
        {{dmc_para_start_addr +   1, 0x70, 0x30, "reg_dmc_h_block"       }},
        {{dmc_para_start_addr +   1, 0x07, 0x03, "reg_dmc_v_block"       }},
        {{dmc_para_start_addr +   3, 0xF0, 0x10, "reg_dmc_h_lut_num"     }, {dmc_para_start_addr +   2, 0x7F, 0x1e,"reg_dmc_h_lut_num"}},
        {{dmc_para_start_addr +   4, 0xFF, 0x0f, "reg_dmc_v_lut_num"     }, {dmc_para_start_addr +   3, 0x03, 0x01,"reg_dmc_v_lut_num"}},
        {{dmc_para_start_addr +   5, 0x80, 0x00, "reg_dmc_plane_b1_ks22" }},
        {{dmc_para_start_addr +   5, 0x40, 0x00, "reg_dmc_plane_12_ks22" }},
        {{dmc_para_start_addr +   5, 0x20, 0x00, "reg_dmc_plane_23_ks22" }},
        {{dmc_para_start_addr +   5, 0x10, 0x00, "reg_dmc_plane_34_ks22" }},
        {{dmc_para_start_addr +   5, 0x08, 0x00, "reg_dmc_plane_45_ks22" }},
        {{dmc_para_start_addr +   5, 0x04, 0x00, "reg_dmc_plane_56_ks22" }},
        {{dmc_para_start_addr +   5, 0x02, 0x00, "reg_dmc_plane_67_ks22" }},
        {{dmc_para_start_addr +   5, 0x01, 0x00, "reg_dmc_plane_78_ks22" }},
        {{dmc_para_start_addr +   6, 0x80, 0x00, "reg_dmc_plane_8w_ks22" }},
        {{dmc_para_start_addr +   8, 0xFF, 0x00, "reg_dmc_black_limit"   }, {dmc_para_start_addr +   7, 0x0F, 0x00, "reg_dmc_black_limit"  }},
        {{dmc_para_start_addr +  10, 0xFF, 0xff, "reg_dmc_plane_level1"  }, {dmc_para_start_addr +   9, 0x0F, 0x0f, "reg_dmc_plane_level1" }},
        {{dmc_para_start_addr +  12, 0xFF, 0xff, "reg_dmc_plane_level2"  }, {dmc_para_start_addr +  11, 0x0F, 0x0f, "reg_dmc_plane_level2" }},
        {{dmc_para_start_addr +  14, 0xFF, 0xff, "reg_dmc_plane_level3"  }, {dmc_para_start_addr +  13, 0x0F, 0x0f, "reg_dmc_plane_level3" }},
        {{dmc_para_start_addr +  16, 0xFF, 0xff, "reg_dmc_plane_level4"  }, {dmc_para_start_addr +  15, 0x0F, 0x0f, "reg_dmc_plane_level4" }},
        {{dmc_para_start_addr +  18, 0xFF, 0xff, "reg_dmc_plane_level5"  }, {dmc_para_start_addr +  17, 0x0F, 0x0f, "reg_dmc_plane_level5" }},
        {{dmc_para_start_addr +  20, 0xFF, 0xff, "reg_dmc_plane_level6"  }, {dmc_para_start_addr +  19, 0x0F, 0x0f, "reg_dmc_plane_level6" }},
        {{dmc_para_start_addr +  22, 0xFF, 0xff, "reg_dmc_plane_level7"  }, {dmc_para_start_addr +  21, 0x0F, 0x0f, "reg_dmc_plane_level7" }},
        {{dmc_para_start_addr +  24, 0xFF, 0xff, "reg_dmc_plane_level8"  }, {dmc_para_start_addr +  23, 0x0F, 0x0f, "reg_dmc_plane_level8" }},
        {{dmc_para_start_addr +  26, 0xFF, 0xff, "reg_dmc_white_limit"   }, {dmc_para_start_addr +  25, 0x0F, 0x0f, "reg_dmc_white_limit"  }},
        {{dmc_para_start_addr +  28, 0xFF, 0x00, "reg_dmc_plane_b1_coef" }, {dmc_para_start_addr +  27, 0x3F, 0x00, "reg_dmc_plane_b1_coef"}},
        {{dmc_para_start_addr +  30, 0xFF, 0x00, "reg_dmc_plane_12_coef" }, {dmc_para_start_addr +  29, 0x3F, 0x00, "reg_dmc_plane_12_coef"}},
        {{dmc_para_start_addr +  32, 0xFF, 0x00, "reg_dmc_plane_23_coef" }, {dmc_para_start_addr +  31, 0x3F, 0x00, "reg_dmc_plane_23_coef"}},
        {{dmc_para_start_addr +  34, 0xFF, 0x00, "reg_dmc_plane_34_coef" }, {dmc_para_start_addr +  33, 0x3F, 0x00, "reg_dmc_plane_34_coef"}},
        {{dmc_para_start_addr +  36, 0xFF, 0x00, "reg_dmc_plane_45_coef" }, {dmc_para_start_addr +  35, 0x3F, 0x00, "reg_dmc_plane_45_coef"}},
        {{dmc_para_start_addr +  38, 0xFF, 0x00, "reg_dmc_plane_56_coef" }, {dmc_para_start_addr +  37, 0x3F, 0x00, "reg_dmc_plane_56_coef"}},
        {{dmc_para_start_addr +  40, 0xFF, 0x00, "reg_dmc_plane_67_coef" }, {dmc_para_start_addr +  39, 0x3F, 0x00, "reg_dmc_plane_67_coef"}},
        {{dmc_para_start_addr +  42, 0xFF, 0x00, "reg_dmc_plane_78_coef" }, {dmc_para_start_addr +  41, 0x3F, 0x00, "reg_dmc_plane_78_coef"}},
        {{dmc_para_start_addr +  44, 0xFF, 0x00, "reg_dmc_plane_8w_coef" }, {dmc_para_start_addr +  43, 0x3F, 0x00, "reg_dmc_plane_8w_coef"}},
        {{dmc_para_start_addr +  45, 0x70, 0x00, "reg_dmc_data_r_mag1"   }},
        {{dmc_para_start_addr +  45, 0x07, 0x00, "reg_dmc_data_r_mag2"   }},
        {{dmc_para_start_addr +  46, 0x70, 0x00, "reg_dmc_data_r_mag3"   }},
        {{dmc_para_start_addr +  46, 0x07, 0x00, "reg_dmc_data_r_mag4"   }},
        {{dmc_para_start_addr +  47, 0x70, 0x00, "reg_dmc_data_r_mag5"   }},
        {{dmc_para_start_addr +  47, 0x07, 0x00, "reg_dmc_data_r_mag6"   }},
        {{dmc_para_start_addr +  48, 0x70, 0x00, "reg_dmc_data_r_mag7"   }},
        {{dmc_para_start_addr +  48, 0x07, 0x00, "reg_dmc_data_r_mag8"   }},
        {{dmc_para_start_addr +  49, 0x70, 0x00, "reg_dmc_data_g_mag1"   }},
        {{dmc_para_start_addr +  49, 0x07, 0x00, "reg_dmc_data_g_mag2"   }},
        {{dmc_para_start_addr +  50, 0x70, 0x00, "reg_dmc_data_g_mag3"   }},
        {{dmc_para_start_addr +  50, 0x07, 0x00, "reg_dmc_data_g_mag4"   }},
        {{dmc_para_start_addr +  51, 0x70, 0x00, "reg_dmc_data_g_mag5"   }},
        {{dmc_para_start_addr +  51, 0x07, 0x00, "reg_dmc_data_g_mag6"   }},
        {{dmc_para_start_addr +  52, 0x70, 0x00, "reg_dmc_data_g_mag7"   }},
        {{dmc_para_start_addr +  52, 0x07, 0x00, "reg_dmc_data_g_mag8"   }},
        {{dmc_para_start_addr +  53, 0x70, 0x00, "reg_dmc_data_b_mag1"   }},
        {{dmc_para_start_addr +  53, 0x07, 0x00, "reg_dmc_data_b_mag2"   }},
        {{dmc_para_start_addr +  54, 0x70, 0x00, "reg_dmc_data_b_mag3"   }},
        {{dmc_para_start_addr +  54, 0x07, 0x00, "reg_dmc_data_b_mag4"   }},
        {{dmc_para_start_addr +  55, 0x70, 0x00, "reg_dmc_data_b_mag5"   }},
        {{dmc_para_start_addr +  55, 0x07, 0x00, "reg_dmc_data_b_mag6"   }},
        {{dmc_para_start_addr +  56, 0x70, 0x00, "reg_dmc_data_b_mag7"   }},
        {{dmc_para_start_addr +  56, 0x07, 0x00, "reg_dmc_data_b_mag8"   }},
        {{dmc_para_start_addr +  58, 0xFF, 0x00, "reg_dmc_data_r_offset1"}, {dmc_para_start_addr +  57, 0x3F, 0x00, "reg_dmc_data_r_offset1"}},
        {{dmc_para_start_addr +  60, 0xFF, 0x00, "reg_dmc_data_r_offset2"}, {dmc_para_start_addr +  59, 0x3F, 0x00, "reg_dmc_data_r_offset2"}},
        {{dmc_para_start_addr +  62, 0xFF, 0x00, "reg_dmc_data_r_offset3"}, {dmc_para_start_addr +  61, 0x3F, 0x00, "reg_dmc_data_r_offset3"}},
        {{dmc_para_start_addr +  64, 0xFF, 0x00, "reg_dmc_data_r_offset4"}, {dmc_para_start_addr +  63, 0x3F, 0x00, "reg_dmc_data_r_offset4"}},
        {{dmc_para_start_addr +  66, 0xFF, 0x00, "reg_dmc_data_r_offset5"}, {dmc_para_start_addr +  65, 0x3F, 0x00, "reg_dmc_data_r_offset5"}},
        {{dmc_para_start_addr +  68, 0xFF, 0x00, "reg_dmc_data_r_offset6"}, {dmc_para_start_addr +  67, 0x3F, 0x00, "reg_dmc_data_r_offset6"}},
        {{dmc_para_start_addr +  70, 0xFF, 0x00, "reg_dmc_data_r_offset7"}, {dmc_para_start_addr +  69, 0x3F, 0x00, "reg_dmc_data_r_offset7"}},
        {{dmc_para_start_addr +  72, 0xFF, 0x00, "reg_dmc_data_r_offset8"}, {dmc_para_start_addr +  71, 0x3F, 0x00, "reg_dmc_data_r_offset8"}},
        {{dmc_para_start_addr +  74, 0xFF, 0x00, "reg_dmc_data_g_offset1"}, {dmc_para_start_addr +  73, 0x3F, 0x00, "reg_dmc_data_g_offset1"}},
        {{dmc_para_start_addr +  76, 0xFF, 0x00, "reg_dmc_data_g_offset2"}, {dmc_para_start_addr +  75, 0x3F, 0x00, "reg_dmc_data_g_offset2"}},
        {{dmc_para_start_addr +  78, 0xFF, 0x00, "reg_dmc_data_g_offset3"}, {dmc_para_start_addr +  77, 0x3F, 0x00, "reg_dmc_data_g_offset3"}},
        {{dmc_para_start_addr +  80, 0xFF, 0x00, "reg_dmc_data_g_offset4"}, {dmc_para_start_addr +  79, 0x3F, 0x00, "reg_dmc_data_g_offset4"}},
        {{dmc_para_start_addr +  82, 0xFF, 0x00, "reg_dmc_data_g_offset5"}, {dmc_para_start_addr +  81, 0x3F, 0x00, "reg_dmc_data_g_offset5"}},
        {{dmc_para_start_addr +  84, 0xFF, 0x00, "reg_dmc_data_g_offset6"}, {dmc_para_start_addr +  83, 0x3F, 0x00, "reg_dmc_data_g_offset6"}},
        {{dmc_para_start_addr +  86, 0xFF, 0x00, "reg_dmc_data_g_offset7"}, {dmc_para_start_addr +  85, 0x3F, 0x00, "reg_dmc_data_g_offset7"}},
        {{dmc_para_start_addr +  88, 0xFF, 0x00, "reg_dmc_data_g_offset8"}, {dmc_para_start_addr +  87, 0x3F, 0x00, "reg_dmc_data_g_offset8"}},
        {{dmc_para_start_addr +  90, 0xFF, 0x00, "reg_dmc_data_b_offset1"}, {dmc_para_start_addr +  89, 0x3F, 0x00, "reg_dmc_data_b_offset1"}},
        {{dmc_para_start_addr +  92, 0xFF, 0x00, "reg_dmc_data_b_offset2"}, {dmc_para_start_addr +  91, 0x3F, 0x00, "reg_dmc_data_b_offset2"}},
        {{dmc_para_start_addr +  94, 0xFF, 0x00, "reg_dmc_data_b_offset3"}, {dmc_para_start_addr +  93, 0x3F, 0x00, "reg_dmc_data_b_offset3"}},
        {{dmc_para_start_addr +  96, 0xFF, 0x00, "reg_dmc_data_b_offset4"}, {dmc_para_start_addr +  95, 0x3F, 0x00, "reg_dmc_data_b_offset4"}},
        {{dmc_para_start_addr +  98, 0xFF, 0x00, "reg_dmc_data_b_offset5"}, {dmc_para_start_addr +  97, 0x3F, 0x00, "reg_dmc_data_b_offset5"}},
        {{dmc_para_start_addr + 100, 0xFF, 0x00, "reg_dmc_data_b_offset6"}, {dmc_para_start_addr +  99, 0x3F, 0x00, "reg_dmc_data_b_offset6"}},
        {{dmc_para_start_addr + 102, 0xFF, 0x00, "reg_dmc_data_b_offset7"}, {dmc_para_start_addr + 101, 0x3F, 0x00, "reg_dmc_data_b_offset7"}},
        {{dmc_para_start_addr + 104, 0xFF, 0x00, "reg_dmc_data_b_offset8"}, {dmc_para_start_addr + 103, 0x3F, 0x00, "reg_dmc_data_b_offset8"}},
    };

    //++++++++++++ Y mode (Mono mode) start ++++++++++++//
    if(Demura_Original_Mode == E_RGB_MODE)
        _dmc_reg.reg_dmc_rgb_mode.Bit07_00.value = 0;
    else
    _dmc_reg.reg_dmc_rgb_mode.Bit07_00.value              = AUO_IN_DATA[_dmc_reg.reg_dmc_rgb_mode.Bit07_00.address];            // dmc_rgb_mode (0 : mono mode / 1 : rgb mode) <-> mstar_y_mode_en (0 : rgb mode / 1 : mono mode ) , ///// ??è¦?å???/////
    //++++++++++++ Y mode (Mono mode) end ++++++++++++//

    //++++++++++++ layerX level enable start ++++++++++++//
    _dmc_reg.reg_dmc_plane_num.Bit07_00.value             = AUO_IN_DATA[_dmc_reg.reg_dmc_plane_num.Bit07_00.address];           // dmc_plane_num <-> bLayer_level_en[?+2] , ///// +2 ??å???è¦?è??æ?®cut off ?¯ä½¿?¨layer levelä¾?å???ï???ä»¥Mstar??worse case?ªè?½??target 4 å±?/////
    //++++++++++++ layerX level enable end ++++++++++++//

    //++++++++++++ Block H/V Size start ++++++++++++//
    _dmc_reg.reg_dmc_h_block.Bit07_00.value               = AUO_IN_DATA[_dmc_reg.reg_dmc_h_block.Bit07_00.address];             // dmc_h_block <-> Mstar_block_h_size , ///// AUOçµ¦ç?????å?ªæ¬¡æ?¹ï¼?Mstar ä»?é?¢çµ¦ç??¯ç?´?¥block?¸ï???è¦?è???ï?æ³¨æ?AUO??å°?æ??¯è?½?? -> block?? /////

    _dmc_reg.reg_dmc_v_block.Bit07_00.value               = AUO_IN_DATA[_dmc_reg.reg_dmc_v_block.Bit07_00.address];             // dmc_v_block <-> Mstar_block_v_size , ///// AUOçµ¦ç?????å?ªæ¬¡æ?¹ï¼?Mstar ä»?é?¢çµ¦ç??¯ç?´?¥block?¸ï???è¦?è???ï?æ³¨æ?AUO??å°?æ??¯è?½?? -> block?? /////
    //++++++++++++ Block H/V Size end ++++++++++++//

    //++++++++++++ LUT H/V Size start +++++++++++//
    _dmc_reg.reg_dmc_h_lut_num.Bit07_00.value             = AUO_IN_DATA[_dmc_reg.reg_dmc_h_lut_num.Bit07_00.address];           // dmc_h_lut_num <-> Mstar_lut_h_size
    _dmc_reg.reg_dmc_h_lut_num.val             = AUO_IN_DATA[_dmc_reg.reg_dmc_h_lut_num.Bit15_08.address];

    _dmc_reg.reg_dmc_v_lut_num.Bit07_00.value             = AUO_IN_DATA[_dmc_reg.reg_dmc_v_lut_num.Bit07_00.address];           // dmc_v_lut_num <-> Mstar_lut_v_size
    _dmc_reg.reg_dmc_v_lut_num.val             = AUO_IN_DATA[_dmc_reg.reg_dmc_v_lut_num.Bit15_08.address];
    //++++++++++++ LUT H/V Size end +++++++++++//

    //++++++++++++ layerX level start ++++++++++++//
    _dmc_reg.reg_dmc_black_limit.Bit07_00.value           = AUO_IN_DATA[_dmc_reg.reg_dmc_black_limit.Bit07_00.address];         // reg_dmc_black_limit <-> iLayer_level[0] ï¼?///// AUO??2bitï¼?Mstar??0bit /////
    _dmc_reg.reg_dmc_black_limit.val           = AUO_IN_DATA[_dmc_reg.reg_dmc_black_limit.Bit15_08.address];

    _dmc_reg.reg_dmc_plane_level1.Bit07_00.value          = AUO_IN_DATA[_dmc_reg.reg_dmc_plane_level1.Bit07_00.address];        // reg_dmc_plane_level1 <-> iLayer_level[1] ï¼?///// AUO??2bitï¼?Mstar??0bit /////
    _dmc_reg.reg_dmc_plane_level1.val          = AUO_IN_DATA[_dmc_reg.reg_dmc_plane_level1.Bit15_08.address];

    _dmc_reg.reg_dmc_plane_level2.Bit07_00.value          = AUO_IN_DATA[_dmc_reg.reg_dmc_plane_level2.Bit07_00.address];        // reg_dmc_plane_level2 <-> iLayer_level[2] ï¼?///// AUO??2bitï¼?Mstar??0bit /////
    _dmc_reg.reg_dmc_plane_level2.val          = AUO_IN_DATA[_dmc_reg.reg_dmc_plane_level2.Bit15_08.address];

    _dmc_reg.reg_dmc_plane_level3.Bit07_00.value          = AUO_IN_DATA[_dmc_reg.reg_dmc_plane_level3.Bit07_00.address];        // reg_dmc_plane_level3 <-> iLayer_level[3] ï¼?///// AUO??2bitï¼?Mstar??0bit /////
    _dmc_reg.reg_dmc_plane_level3.val          = AUO_IN_DATA[_dmc_reg.reg_dmc_plane_level3.Bit15_08.address];

    _dmc_reg.reg_dmc_plane_level4.Bit07_00.value          = AUO_IN_DATA[_dmc_reg.reg_dmc_plane_level4.Bit07_00.address];        // reg_dmc_plane_level4 <-> iLayer_level[4] ï¼?///// AUO??2bitï¼?Mstar??0bit /////
    _dmc_reg.reg_dmc_plane_level4.val          = AUO_IN_DATA[_dmc_reg.reg_dmc_plane_level4.Bit15_08.address];

    _dmc_reg.reg_dmc_white_limit.Bit07_00.value           = AUO_IN_DATA[_dmc_reg.reg_dmc_white_limit.Bit07_00.address];         // reg_dmc_white_limit <-> iLayer_level[5] or iLayer_level[4] ï¼?///// AUO??2bitï¼?Mstar??0bit /////
    _dmc_reg.reg_dmc_white_limit.val           = AUO_IN_DATA[_dmc_reg.reg_dmc_white_limit.Bit15_08.address];
    //++++++++++++ layerX level end ++++++++++++//

    //++++++++++++ layerX gain start ++++++++++++//
    _dmc_reg.reg_dmc_data_r_mag1.Bit07_00.value           = AUO_IN_DATA[_dmc_reg.reg_dmc_data_r_mag1.Bit07_00.address];         // reg_dmc_data_r_mag1 <-> r layer gain 1

    _dmc_reg.reg_dmc_data_r_mag2.Bit07_00.value           = AUO_IN_DATA[_dmc_reg.reg_dmc_data_r_mag2.Bit07_00.address];         // reg_dmc_data_r_mag2 <-> r layer gain 2

    _dmc_reg.reg_dmc_data_r_mag3.Bit07_00.value           = AUO_IN_DATA[_dmc_reg.reg_dmc_data_r_mag3.Bit07_00.address];         // reg_dmc_data_r_mag3 <-> r layer gain 3

    _dmc_reg.reg_dmc_data_r_mag4.Bit07_00.value           = AUO_IN_DATA[_dmc_reg.reg_dmc_data_r_mag4.Bit07_00.address];         // reg_dmc_data_r_mag4 <-> r layer gain 4

    _dmc_reg.reg_dmc_data_g_mag1.Bit07_00.value           = AUO_IN_DATA[_dmc_reg.reg_dmc_data_g_mag1.Bit07_00.address];         // reg_dmc_data_g_mag1 <-> g layer gain 1

    _dmc_reg.reg_dmc_data_g_mag2.Bit07_00.value           = AUO_IN_DATA[_dmc_reg.reg_dmc_data_g_mag2.Bit07_00.address];         // reg_dmc_data_g_mag2 <-> g layer gain 2

    _dmc_reg.reg_dmc_data_g_mag3.Bit07_00.value           = AUO_IN_DATA[_dmc_reg.reg_dmc_data_g_mag3.Bit07_00.address];         // reg_dmc_data_g_mag3 <-> g layer gain 3

    _dmc_reg.reg_dmc_data_g_mag4.Bit07_00.value           = AUO_IN_DATA[_dmc_reg.reg_dmc_data_g_mag4.Bit07_00.address];         // reg_dmc_data_g_mag4 <-> g layer gain 4

    _dmc_reg.reg_dmc_data_b_mag1.Bit07_00.value           = AUO_IN_DATA[_dmc_reg.reg_dmc_data_b_mag1.Bit07_00.address];         // reg_dmc_data_b_mag1 <-> b layer gain 1

    _dmc_reg.reg_dmc_data_b_mag2.Bit07_00.value           = AUO_IN_DATA[_dmc_reg.reg_dmc_data_b_mag2.Bit07_00.address];         // reg_dmc_data_b_mag2 <-> b layer gain 2

    _dmc_reg.reg_dmc_data_b_mag3.Bit07_00.value           = AUO_IN_DATA[_dmc_reg.reg_dmc_data_b_mag3.Bit07_00.address];         // reg_dmc_data_b_mag3 <-> b layer gain 3

    _dmc_reg.reg_dmc_data_b_mag4.Bit07_00.value           = AUO_IN_DATA[_dmc_reg.reg_dmc_data_b_mag4.Bit07_00.address];         // reg_dmc_data_b_mag4 <-> b layer gain 4
    //++++++++++++ layerX gain end ++++++++++++//

    //++++++++++++ layerX level offset start ++++++++++++//
    _dmc_reg.reg_dmc_data_r_offset1.Bit07_00.value        = AUO_IN_DATA[_dmc_reg.reg_dmc_data_r_offset1.Bit07_00.address];      // reg_dmc_data_r_offset1
    _dmc_reg.reg_dmc_data_r_offset1.val        = AUO_IN_DATA[_dmc_reg.reg_dmc_data_r_offset1.Bit15_08.address];

    _dmc_reg.reg_dmc_data_r_offset2.Bit07_00.value        = AUO_IN_DATA[_dmc_reg.reg_dmc_data_r_offset2.Bit07_00.address];      // reg_dmc_data_r_offset2
    _dmc_reg.reg_dmc_data_r_offset2.val        = AUO_IN_DATA[_dmc_reg.reg_dmc_data_r_offset2.Bit15_08.address];

    _dmc_reg.reg_dmc_data_r_offset3.Bit07_00.value        = AUO_IN_DATA[_dmc_reg.reg_dmc_data_r_offset3.Bit07_00.address];      // reg_dmc_data_r_offset3
    _dmc_reg.reg_dmc_data_r_offset3.val        = AUO_IN_DATA[_dmc_reg.reg_dmc_data_r_offset3.Bit15_08.address];

    _dmc_reg.reg_dmc_data_r_offset4.Bit07_00.value        = AUO_IN_DATA[_dmc_reg.reg_dmc_data_r_offset4.Bit07_00.address];      // reg_dmc_data_r_offset4
    _dmc_reg.reg_dmc_data_r_offset4.val        = AUO_IN_DATA[_dmc_reg.reg_dmc_data_r_offset4.Bit15_08.address];

    _dmc_reg.reg_dmc_data_g_offset1.Bit07_00.value        = AUO_IN_DATA[_dmc_reg.reg_dmc_data_g_offset1.Bit07_00.address];      // reg_dmc_data_g_offset1
    _dmc_reg.reg_dmc_data_g_offset1.val        = AUO_IN_DATA[_dmc_reg.reg_dmc_data_g_offset1.Bit15_08.address];

    _dmc_reg.reg_dmc_data_g_offset2.Bit07_00.value        = AUO_IN_DATA[_dmc_reg.reg_dmc_data_g_offset2.Bit07_00.address];      // reg_dmc_data_g_offset2
    _dmc_reg.reg_dmc_data_g_offset2.val        = AUO_IN_DATA[_dmc_reg.reg_dmc_data_g_offset2.Bit15_08.address];

    _dmc_reg.reg_dmc_data_g_offset3.Bit07_00.value        = AUO_IN_DATA[_dmc_reg.reg_dmc_data_g_offset3.Bit07_00.address];      // reg_dmc_data_g_offset3
    _dmc_reg.reg_dmc_data_g_offset3.val        = AUO_IN_DATA[_dmc_reg.reg_dmc_data_g_offset3.Bit15_08.address];

    _dmc_reg.reg_dmc_data_g_offset4.Bit07_00.value        = AUO_IN_DATA[_dmc_reg.reg_dmc_data_g_offset4.Bit07_00.address];      // reg_dmc_data_g_offset4
    _dmc_reg.reg_dmc_data_g_offset4.val        = AUO_IN_DATA[_dmc_reg.reg_dmc_data_g_offset4.Bit15_08.address];

    _dmc_reg.reg_dmc_data_b_offset1.Bit07_00.value        = AUO_IN_DATA[_dmc_reg.reg_dmc_data_b_offset1.Bit07_00.address];      // reg_dmc_data_b_offset1
    _dmc_reg.reg_dmc_data_b_offset1.val        = AUO_IN_DATA[_dmc_reg.reg_dmc_data_b_offset1.Bit15_08.address];

    _dmc_reg.reg_dmc_data_b_offset2.Bit07_00.value        = AUO_IN_DATA[_dmc_reg.reg_dmc_data_b_offset2.Bit07_00.address];      // reg_dmc_data_b_offset2
    _dmc_reg.reg_dmc_data_b_offset2.val        = AUO_IN_DATA[_dmc_reg.reg_dmc_data_b_offset2.Bit15_08.address];

    _dmc_reg.reg_dmc_data_b_offset3.Bit07_00.value        = AUO_IN_DATA[_dmc_reg.reg_dmc_data_b_offset3.Bit07_00.address];      // reg_dmc_data_b_offset3
    _dmc_reg.reg_dmc_data_b_offset3.val        = AUO_IN_DATA[_dmc_reg.reg_dmc_data_b_offset3.Bit15_08.address];

    _dmc_reg.reg_dmc_data_b_offset4.Bit07_00.value        = AUO_IN_DATA[_dmc_reg.reg_dmc_data_b_offset4.Bit07_00.address];      // reg_dmc_data_b_offset4
    _dmc_reg.reg_dmc_data_b_offset4.val        = AUO_IN_DATA[_dmc_reg.reg_dmc_data_b_offset4.Bit15_08.address];
    //++++++++++++ layerX level offset end ++++++++++++//

    //---------------------- Capture DMC Parameter (register) end ----------------------//
#endif

    //---------------------- DMC register to Mstar register start ----------------------//
    #if defined (CONFIG_DEMURA_URSA11)
    bRet = set_u11_interface(phdr, &_dmc_reg, pDataInfo, pbin_info);
    #elif defined (CONFIG_DEMURA_URSA13)
    bRet = set_u13_interface(phdr, &_dmc_reg, pDataInfo, pbin_info);
    #elif (defined (CONFIG_DEMURA_M7622) || defined (CONFIG_DEMURA_M7632) || defined (CONFIG_DEMURA_MT5896))
    bRet = set_u13_interface(phdr, &_dmc_reg, pDataInfo, pbin_info);
    #else
    #error "Unkown DEMURA_URSA_TYPE"
    #endif
    if (bRet == FALSE)
    {
        printf("set_demura_uxx_interface error\n");
    }

    // Sign the auo dmc data !
    Gen_Sf_Signature_AUO(phdr);

    //---------------------- DMC LUT Mapping to Mstar LUT end ----------------------//

    // Free AUO DMC Buffer
    dfree(phdr->hdr_buffer);
    phdr->hdr_buffer = NULL;
    phdr->hdr_offset = 0;
    phdr->hdr_length = 0;

    dfree(phdr->lut_buffer);
    phdr->lut_buffer = NULL;
    phdr->lut_offset = 0;
    phdr->lut_length = 0;
    UBOOT_DEBUG("free hdr_buffer\n");

    return bRet;
}


MS_BOOL Decode_To_Mstar_Format_AUO(void *pDataInfo, BinOutputInfo *pbin_info)
{
    MS_BOOL bRet = FALSE;
    static AUO_Demura_Header header;

    UBOOT_DEBUG("IN\n");

    // Parse header information
    memset(&header, 0, sizeof(header));
    if (load_vendor_header(&header) != TRUE)
    {
        UBOOT_ERROR("load_vendor_header Error\n");
        return FALSE;
    }
    UBOOT_TRACE("&header = 0x%p\n", &header);
    if (parse_vendor_header(&header) != TRUE)
    {
        UBOOT_ERROR("parse_vendor_header Error\n");
        return FALSE;
    }
    dump_vendor_header(&header);

    if (load_vendor_lut(&header) != TRUE)
    {
        UBOOT_ERROR("load_vendor_lut Error\n");
        dfree(header.hdr_buffer);
        header.hdr_buffer = NULL;
        return FALSE;
    }

    if (!header.hdr_buffer)
    {
        UBOOT_ERROR("load vendor header Error\n");
        return FALSE;
    }

    // Alloc space for Lut_in
    pbin_info->HNode       =  header.dmc_h_lut_num;
    pbin_info->VNode       =  header.dmc_v_lut_num;
    pbin_info->LevelCount  =  header.dmc_panel_num;

    bRet = Alloc_LutIn_Space(pDataInfo, pbin_info);
    if (bRet == FALSE)
    {
        UBOOT_ERROR("Alloc_LutIn_Space error\n");
        dfree(header.hdr_buffer);
        header.hdr_buffer = NULL;
        dfree(header.lut_buffer);
        header.lut_buffer = NULL;
        return FALSE;
    }

    // Decode Vendor data
    bRet = AUO_Decode_To_Mstar_Format(&header, pDataInfo, pbin_info);
    if (bRet == FALSE)
    {
        UBOOT_ERROR("AUO_Decode_To_Mstar_Format error\n");
        return FALSE;
    }

    //MS_BOOL demura_enable[3]  = {TRUE, TRUE, TRUE};

    pbin_info->reg_base_addr  =  0x7700;
    pbin_info->project_id     =  ((MSTAR_CHIP_ID << 16) + (1ULL << 15) + EN_DEMURA_MULTI_AUO);
    pbin_info->data_type      =  IC_DRAM;
    pbin_info->Gain_type      =  10;

    //memcpy(pbin_info->channel_enable, demura_enable, sizeof(demura_enable));
    UBOOT_DEBUG("OK\n");
    return TRUE;
}


static MS_BOOL Gen_Sf_Signature_AUO(AUO_Demura_Header *phdr)
{
    char   strbuf[1024];
    MS_U32 sample_crc32;
    MS_S32 strlen;
    MS_U8  *buffer;

    buffer = phdr->lut_buffer + phdr->lut_offset;

    sample_crc32 = MDrv_CRC32_Cal_DeMura(buffer + SAMPLE_DAT_START, SAMPLE_DAT_LEN);
    if (sample_crc32 == 0xffffFFFF)
    {
        UBOOT_ERROR("MDrv_CRC32_Cal_DeMura error\n");
        return FALSE;
    }
    memset(strbuf, 0, sizeof(strbuf));
    strlen = snprintf(strbuf, sizeof(strbuf)-1, "0x%x:0x%x", (unsigned int)phdr->lut_checksum, (unsigned int)sample_crc32);
    if (strlen < 0)
    {
        return FALSE;
    }

    Gen_Sf_Signature(strbuf, strlen);
    return TRUE;
}


MS_BOOL If_Need_Decode_AUO(void)
{
    UBOOT_TRACE("IN\n");
#if defined(CONFIG_DEMURA_VENDOR_BACKLIGHT)
    char *sig_str;
    if (E_MS_UTIL_BIN_FILE_BACKLIGHT == get_demura_file())
    {
        sig_str = env_get(ENV_DEMURA_SIG_BL);
    }
    else
    {
        sig_str = env_get(ENV_DEMURA_SIG);
    }
#else
    char *sig_str = env_get(ENV_DEMURA_SIG);
#endif
    if (sig_str == NULL)
    {
        UBOOT_DEBUG("Empty Board, should decoding data\n");
        UBOOT_TRACE("OK\n");
        return TRUE;
    }
    else
    {
        MS_U16 lut_checksum_sf;
        MS_U32 lut_checksum_bd;
        MS_U32 sample_crc32, cal_crc32;
        MS_U8  buf[SAMPLE_DAT_LEN];

        lut_checksum_sf = get_lut_checksum();

        UBOOT_DEBUG("sig_str = %s\n", sig_str);
        if(2 == sscanf(sig_str, "0x%x:0x%x", &lut_checksum_bd, &sample_crc32))
        {
            UBOOT_DEBUG("lut_checksum_sf = 0x%x\n", (uint)lut_checksum_sf);
            UBOOT_DEBUG("lut_checksum_bd = 0x%x\n", (uint)lut_checksum_bd);
            UBOOT_DEBUG("sample_crc32  = 0x%x\n",   (uint)sample_crc32);

#if (DEMURA_ONLY_MONO_MODE)
            if(lut_checksum_sf == lut_checksum_bd)
            {
                UBOOT_DEBUG("Data Match, No Need to decode again\n");
                UBOOT_TRACE("OK\n");
                return FALSE;
            }
#else
            if (read_spi_flash(buf, (DMC_LUT_START + SAMPLE_DAT_START), SAMPLE_DAT_LEN) == TRUE)
            {
                cal_crc32 = MDrv_CRC32_Cal_DeMura(buf, SAMPLE_DAT_LEN);
                UBOOT_DEBUG("cal_crc32     = 0x%x\n", (uint)cal_crc32);
                if ((sample_crc32 == cal_crc32) && (lut_checksum_sf == lut_checksum_bd))
                {
                    UBOOT_DEBUG("Data Match, No Need to decode again\n");
                    UBOOT_TRACE("OK\n");
                    return FALSE;
                }
            }
#endif
        }

        UBOOT_TRACE("OK\n");
        return TRUE;
    }

    UBOOT_TRACE("OK\n");
    return FALSE;
}

static MS_BOOL Gen_Sf_Signature_CSOT_CSOT(CSOT_CSOT_Demura_Header *phdr)
{
    char   strbuf[1024];
    MS_U32 sample_crc32;
    MS_S32 strlen;
    MS_U8  *buffer;

    buffer = phdr->lut_buffer[0];

    sample_crc32 = MDrv_CRC32_Cal_DeMura(buffer + SAMPLE_DAT_START, SAMPLE_DAT_LEN);
    if (sample_crc32 == 0xffffFFFF)
    {
        UBOOT_ERROR("MDrv_CRC32_Cal_DeMura error\n");
        return FALSE;
    }
    memset(strbuf, 0, sizeof(strbuf));

    strlen = snprintf(strbuf, sizeof(strbuf)-1, "0x%x:0x%x", (unsigned int)phdr->TABLE_CRC[0], (unsigned int)sample_crc32);
    if (strlen < 0)
    {
        UBOOT_ERROR("Record CRC Fail\n");
       return FALSE;
    }
    Gen_Sf_Signature(strbuf, strlen);
    return TRUE;
}

MS_BOOL If_Need_Decode_CSOT_CSOT(void)
{
    UBOOT_TRACE("IN\n");

#if defined(CONFIG_DEMURA_VENDOR_BACKLIGHT)
    char *sig_str;
    if (E_MS_UTIL_BIN_FILE_BACKLIGHT == get_demura_file())
    {
        sig_str = env_get(ENV_DEMURA_SIG_BL);
    }
    else
    {
        sig_str = env_get(ENV_DEMURA_SIG);
    }
#else
    char *sig_str = env_get(ENV_DEMURA_SIG);
#endif

    if (sig_str == NULL)
    {
        UBOOT_DEBUG("Empty Board, should decoding data\n");
        UBOOT_TRACE("OK\n");
        return TRUE;
    }
    else
    {
        MS_U16 lut_checksum_sf;
        MS_U32 lut_checksum_bd;
        MS_U32 sample_crc32, cal_crc32;
        MS_U8  buf[SAMPLE_DAT_LEN];

        //lut_checksum_sf = get_lut_checksum();
        lut_checksum_sf = get_CSOT_CSOT_CRC(3);

        UBOOT_DEBUG("sig_str = %s\n", sig_str);
        if(2 == sscanf(sig_str, "0x%x:0x%x", &lut_checksum_bd, &sample_crc32))
        {
            UBOOT_DEBUG("lut_checksum_sf = 0x%x\n", (uint)lut_checksum_sf);
            UBOOT_DEBUG("lut_checksum_bd = 0x%x\n", (uint)lut_checksum_bd);
            UBOOT_DEBUG("sample_crc32  = 0x%x\n",   (uint)sample_crc32);

            if (read_spi_flash(buf, (CSOT_CSOT_LUT_START + SAMPLE_DAT_START), SAMPLE_DAT_LEN) == TRUE)
            {
                cal_crc32 = MDrv_CRC32_Cal_DeMura(buf, SAMPLE_DAT_LEN);
                UBOOT_DEBUG("cal_crc32     = 0x%x\n", (uint)cal_crc32);
                if ((sample_crc32 == cal_crc32) && (lut_checksum_sf == lut_checksum_bd))
                {
                    UBOOT_DEBUG("Data Match, No Need to decode again\n");
                    UBOOT_TRACE("OK\n");
                    return FALSE;
                }
            }
        }

        UBOOT_TRACE("OK\n");
        return TRUE;
    }

    UBOOT_TRACE("OK\n");
    return FALSE;
}
#if defined (CONFIG_DEMURA_URSA11)
static void CSOT_CSOT_Init_U11_Param(CSOT_CSOT_Demura_Header *phdr, interface_info *pDataInfo)
{
    interface_info *DataInfo   = (interface_info *)pDataInfo;
    interface_info *R_Info_Out = &(DataInfo[0]);
    interface_info *G_Info_Out = &(DataInfo[1]);
    interface_info *B_Info_Out = &(DataInfo[2]);

    int channel;
    for (channel = 0; channel < 3; channel++)
    {
        DataInfo[channel].bOut_dither_en = TRUE;
    }

    (*R_Info_Out).iChannel = 0;
    (*G_Info_Out).iChannel = 1;
    (*B_Info_Out).iChannel = 2;

    int layer_level_num      = phdr->DEMURA_PLANE_NUM;//MAX_PLANE_NUM

    layer_level_num = ((layer_level_num>=1)&&(layer_level_num<=4)) ? layer_level_num : 3;

    int idx;
    for (idx = 0; idx < 6; idx++)
    {
        (*R_Info_Out).bLayer_level_en[idx] = FALSE;
        (*G_Info_Out).bLayer_level_en[idx] = FALSE;
        (*B_Info_Out).bLayer_level_en[idx] = FALSE;
    }

    switch(layer_level_num)
    {
        case 0:
            break;
        case 1:
        case 2:
        case 3:
        case 4:
            for (idx = 0; idx < layer_level_num + 2; idx++)
            {
                (*R_Info_Out).bLayer_level_en[idx] = TRUE;
                (*G_Info_Out).bLayer_level_en[idx] = TRUE;
                (*B_Info_Out).bLayer_level_en[idx] = TRUE;
            }
            break;
        default:
            break;
    }

    for (idx = 0; idx < 6; idx++)
    {
        (*R_Info_Out).iLayer_level[idx] = ((idx!=0) ? 1023 : 0);
        (*G_Info_Out).iLayer_level[idx] = ((idx!=0) ? 1023 : 0);
        (*B_Info_Out).iLayer_level[idx] = ((idx!=0) ? 1023 : 0);
    }

    int black_level    = phdr->DEMURA_BLACK_LIMIT>>2; //12bits ->10 bits
    int layer_level1   = phdr->DEMURA_PLANE_LEVEL1>>2;//12bits ->10 bits
    int layer_level2   = phdr->DEMURA_PLANE_LEVEL2>>2;
    int layer_level3   = phdr->DEMURA_PLANE_LEVEL3>>2;
    int layer_level4   = phdr->DEMURA_PLANE_LEVEL4>>2;
    int white_level    = phdr->DEMURA_WHITE_LIMIT>>2;
/*
    black_level  = ((black_level)<<4);   // 8 bit -> 12 bit
    layer_level1 = ((layer_level1 + 2)<<4);  // 8 bit -> 12 bit
    layer_level2 = ((layer_level2 + 2)<<4);  // 8 bit -> 12 bit
    layer_level3 = ((layer_level3 + 2)<<4);  // 8 bit -> 12 bit
    layer_level4 = ((layer_level4 + 2)<<4);  // 8 bit -> 12 bit
    white_level  = ((white_level)<<4);   // 8 bit -> 12 bit
*/
    switch(layer_level_num)
    {
    case 1:
        (*R_Info_Out).iLayer_level[0] = black_level;
        (*R_Info_Out).iLayer_level[1] = layer_level1;
        (*R_Info_Out).iLayer_level[2] = white_level;

        (*G_Info_Out).iLayer_level[0] = black_level;
        (*G_Info_Out).iLayer_level[1] = layer_level1;
        (*G_Info_Out).iLayer_level[2] = white_level;

        (*B_Info_Out).iLayer_level[0] = black_level;
        (*B_Info_Out).iLayer_level[1] = layer_level1;
        (*B_Info_Out).iLayer_level[2] = white_level;
        break;
    case 2:
        (*R_Info_Out).iLayer_level[0] = black_level;
        (*R_Info_Out).iLayer_level[1] = layer_level1;
        (*R_Info_Out).iLayer_level[2] = layer_level2;
        (*R_Info_Out).iLayer_level[3] = white_level;

        (*G_Info_Out).iLayer_level[0] = black_level;
        (*G_Info_Out).iLayer_level[1] = layer_level1;
        (*G_Info_Out).iLayer_level[2] = layer_level2;
        (*G_Info_Out).iLayer_level[3] = white_level;

        (*B_Info_Out).iLayer_level[0] = black_level;
        (*B_Info_Out).iLayer_level[1] = layer_level1;
        (*B_Info_Out).iLayer_level[2] = layer_level2;
        (*B_Info_Out).iLayer_level[3] = white_level;
        break;
    case 3:
        (*R_Info_Out).iLayer_level[0] = black_level;
        (*R_Info_Out).iLayer_level[1] = layer_level1;
        (*R_Info_Out).iLayer_level[2] = layer_level2;
        (*R_Info_Out).iLayer_level[3] = layer_level3;
        (*R_Info_Out).iLayer_level[4] = white_level;

        (*G_Info_Out).iLayer_level[0] = black_level;
        (*G_Info_Out).iLayer_level[1] = layer_level1;
        (*G_Info_Out).iLayer_level[2] = layer_level2;
        (*G_Info_Out).iLayer_level[3] = layer_level3;
        (*G_Info_Out).iLayer_level[4] = white_level;

        (*B_Info_Out).iLayer_level[0] = black_level;
        (*B_Info_Out).iLayer_level[1] = layer_level1;
        (*B_Info_Out).iLayer_level[2] = layer_level2;
        (*B_Info_Out).iLayer_level[3] = layer_level3;
        (*B_Info_Out).iLayer_level[4] = white_level;
        break;
    case 4:
        (*R_Info_Out).iLayer_level[0] = black_level;
        (*R_Info_Out).iLayer_level[1] = layer_level1;
        (*R_Info_Out).iLayer_level[2] = layer_level2;
        (*R_Info_Out).iLayer_level[3] = layer_level3;
        (*R_Info_Out).iLayer_level[4] = layer_level4;
        (*R_Info_Out).iLayer_level[5] = white_level;

        (*G_Info_Out).iLayer_level[0] = black_level;
        (*G_Info_Out).iLayer_level[1] = layer_level1;
        (*G_Info_Out).iLayer_level[2] = layer_level2;
        (*G_Info_Out).iLayer_level[3] = layer_level3;
        (*G_Info_Out).iLayer_level[4] = layer_level4;
        (*G_Info_Out).iLayer_level[5] = white_level;

        (*B_Info_Out).iLayer_level[0] = black_level;
        (*B_Info_Out).iLayer_level[1] = layer_level1;
        (*B_Info_Out).iLayer_level[2] = layer_level2;
        (*B_Info_Out).iLayer_level[3] = layer_level3;
        (*B_Info_Out).iLayer_level[4] = layer_level4;
        (*B_Info_Out).iLayer_level[5] = white_level;
        break;
    }
    //++++++++++++ layerX level end ++++++++++++//

    //++++++++++++ low luma weight start ++++++++++++//
    (*R_Info_Out).bLow_luma_en         = FALSE;
    (*R_Info_Out).iLow_luma_thrd       = 0x00;
    (*R_Info_Out).iLow_luma_slope      = 0x0F;
    (*R_Info_Out).iLow_luma_min_weight = 0x0;

    (*G_Info_Out).bLow_luma_en         = FALSE;
    (*G_Info_Out).iLow_luma_thrd       = 0x00;
    (*G_Info_Out).iLow_luma_slope      = 0x0F;
    (*G_Info_Out).iLow_luma_min_weight = 0x0;

    (*B_Info_Out).bLow_luma_en         = FALSE;
    (*B_Info_Out).iLow_luma_thrd       = 0x00;
    (*B_Info_Out).iLow_luma_slope      = 0x0F;
    (*B_Info_Out).iLow_luma_min_weight = 0x0;
    //++++++++++++ low luma weight end ++++++++++++//

    //++++++++++++ high luma weight start ++++++++++++//
    (*R_Info_Out).bHigh_luma_en         = FALSE;
    (*R_Info_Out).iHigh_luma_thrd       = 0xFF;
    (*R_Info_Out).iHigh_luma_slope      = 0x0F;
    (*R_Info_Out).iHigh_luma_min_weight = 0x0;

    (*G_Info_Out).bHigh_luma_en         = FALSE;
    (*G_Info_Out).iHigh_luma_thrd       = 0xFF;
    (*G_Info_Out).iHigh_luma_slope      = 0x0F;
    (*G_Info_Out).iHigh_luma_min_weight = 0x0;

    (*B_Info_Out).bHigh_luma_en         = FALSE;
    (*B_Info_Out).iHigh_luma_thrd       = 0xFF;
    (*B_Info_Out).iHigh_luma_slope      = 0x0F;
    (*B_Info_Out).iHigh_luma_min_weight = 0x0;
    //++++++++++++ high luma weight end ++++++++++++//

    for (idx = 0; idx < 6; idx++)
    {
        int cnt_idx;
        for (cnt_idx = 0; cnt_idx < (phdr->DEMURA_TBL_H* phdr->DEMURA_TBL_V); cnt_idx++)
        {
            (*R_Info_Out).Lut_in[idx][cnt_idx] = 0.0;
            (*G_Info_Out).Lut_in[idx][cnt_idx] = 0.0;
            (*B_Info_Out).Lut_in[idx][cnt_idx] = 0.0;
        }
    }
/*
    for(idx = 0; idx < 10; idx++)
        printf("Lut = %f\t", (*R_Info_Out).Lut_in[0][idx]);
    printf("\n");
*/
}
MS_BOOL CSOT_CSOT_Set_U11_Format(CSOT_CSOT_Demura_Header *phdr, interface_info *pinfo)
{
    interface_info *DataInfo   = (interface_info *)pinfo;
    interface_info *R_Info_Out = &(DataInfo[0]);
    interface_info *G_Info_Out = &(DataInfo[1]);
    interface_info *B_Info_Out = &(DataInfo[2]);
    // CSOT only porting mono mode.
    int i, j, k, c, data = 0;
    int table_addr = 0;
    int nLayer = 0, idx_image_size;
    int burst_num = 0, data_per_burst = 0, data_last_burst = 0, skip_last_byte = 0, burst_bit = 128;
    MS_BOOL pol = 0;
    MS_U8 IntBitWidth = 0, DecimalsBitWidth = 0, DataBit = CSOT_CSOT_DATA_BIT_NUM;
    //MS_U16 gain = 0, offset = 0;
    MS_U16 gain[3] = {0}, offset[3] = {0};
    double LUT_data = 0;

    // init interface data
    CSOT_CSOT_Init_U11_Param(phdr, pinfo);


    data_per_burst = burst_bit/DataBit; // 128bit per burst, 12bit per data => 10 data and 8bit dummy byte.
    //printf("data_per_burst = %d \n", data_per_burst);
    burst_num = (phdr->DEMURA_TBL_H/data_per_burst) + ((phdr->DEMURA_TBL_H%data_per_burst > 0)?1:0);
    //printf("burst_num = %d \n", burst_num);
    data_last_burst = phdr->DEMURA_TBL_H%data_per_burst;
    //printf("data_last_dummy = %d \n", data_last_burst);
    skip_last_byte = burst_bit/8 - data_last_burst;
    //printf("skip_last_byte = %d \n", skip_last_byte);

    IntBitWidth = phdr->DEMURA_INT_BIT_WIDTH;
    DecimalsBitWidth = phdr->DEMURA_DEC_BIT_WIDTH;
    //printf("IntBitWidth = %d, DecimalsBitWidth = %d \n", IntBitWidth, DecimalsBitWidth);

    // Get CSOT gain and offset value
    gain[0] = phdr->DEMURA_GAIN_R;
    offset[0] = phdr->DEMURA_OFFSET_R;
    gain[1] = phdr->DEMURA_GAIN_G;
    offset[1] = phdr->DEMURA_OFFSET_G;
    gain[2] = phdr->DEMURA_GAIN_B;
    offset[2] = phdr->DEMURA_OFFSET_B;

    if(phdr->DEMURA_MODE== 1) // RGB mode
    {
        // assign LUT 12 bit format
        for (i = 0; i < phdr->DEMURA_TBL_V; i++)
        {
            for(k = 0; k < phdr->DEMURA_PLANE_NUM; k++)
            {
                for(c = 0; c < 3; c++)
                {
                    // RGB data
                    for (j = 0; j < phdr->DEMURA_TBL_H ; j++)
                    {
                        // LUT index for the plane
                        idx_image_size = i*phdr->DEMURA_TBL_H+j;

                        // 12 bit each for 10 bit value
                        if(pol == 0) //8bit + 4bit
                        {
                            data = ((MS_U16)phdr->lut_buffer[table_addr]) | (((MS_U16)phdr->lut_buffer[table_addr + 1]&0x0f)<<8);
                            pol = 1;
                            table_addr+=1;
                        }
                        else
                        {
                            data = (((MS_U16)phdr->lut_buffer[table_addr] & 0xf0)>>4) | (((MS_U16)phdr->lut_buffer[table_addr + 1]&0xff)<<4);
                            pol = 0;
                            table_addr+=2;
                        }

                        if((j+1)%data_per_burst == 0) // The last data of the burst
                        {
                            table_addr++;
                            pol = 0;
                        }
                        if(j == (phdr->DEMURA_TBL_H - 1)) // The last data of the H
                        {
                            table_addr+=skip_last_byte; // skip dummy 15 bytes
                            pol = 0;
                        }

                        if((data & _BIT11) == _BIT11) // signed bit
                            data = (0x1000 - data)*(-1);


                        // LUT_comp_value = LUT_data*gain/16 + offset
                        LUT_data = ((double)(data*gain[c]/16.0 + offset[c])/(1<<DecimalsBitWidth)); // 10bit data + layer (12bit to 10bit)

                        (*R_Info_Out).Lut_in[k][idx_image_size]       = LUT_data;
                        (*G_Info_Out).Lut_in[k][idx_image_size]       = LUT_data;
                        (*B_Info_Out).Lut_in[k][idx_image_size]       = LUT_data;

                    }
                }
            }
        }
    }
    else // Mono mode
    {
        // assign LUT 12 bit format
        for (i = 0; i < phdr->DEMURA_TBL_V; i++)
        {
            for(k = 0; k < phdr->DEMURA_PLANE_NUM; k++)
            {
                for (j = 0; j < phdr->DEMURA_TBL_H ; j++)
                {
                    // LUT index for the plane
                    idx_image_size = i*phdr->DEMURA_TBL_H+j;

                    // 12 bit each for 10 bit value
                    if(pol == 0) //8bit + 4bit
                    {
                        data = ((MS_U16)phdr->lut_buffer[table_addr]) | (((MS_U16)phdr->lut_buffer[table_addr + 1]&0x0f)<<8);
                        pol = 1;
                        table_addr+=1;
                    }
                    else
                    {
                        data = (((MS_U16)phdr->lut_buffer[table_addr] & 0xf0)>>4) | (((MS_U16)phdr->lut_buffer[table_addr + 1]&0xff)<<4);
                        pol = 0;
                        table_addr+=2;
                    }

                    if((j+1)%data_per_burst == 0) // The last data of the burst
                    {
                        table_addr++;
                        pol = 0;
                    }
                    if(j == (phdr->DEMURA_TBL_H - 1)) // The last data of the H
                    {
                        table_addr+=skip_last_byte; // skip dummy 15 bytes
                        pol = 0;
                    }

                    if((data & _BIT11) == _BIT11) // signed bit
                        data = (0x1000 - data)*(-1);



                    // LUT_comp_value = LUT_data*gain/16 + offset
                    LUT_data = ((double)(data*gain[0]/16.0 + offset[0])/(1<<DecimalsBitWidth)); // 10bit data + layer (12bit to 10bit)

                    (*R_Info_Out).Lut_in[k][idx_image_size]       = LUT_data;
                    (*G_Info_Out).Lut_in[k][idx_image_size]       = LUT_data;
                    (*B_Info_Out).Lut_in[k][idx_image_size]       = LUT_data;


                }
            }
        }
    }

    return TRUE;
}
#else
static void CSOT_CSOT_Init_U13_Param(CSOT_CSOT_Demura_Header *phdr, interface_info *pDataInfo)
{
    CSOT_CSOT_Demura_Header header;
    MS_U16 n_Hnode = 0;

    memcpy(&header, phdr, sizeof(CSOT_CSOT_Demura_Header));

    n_Hnode = header.DEMURA_TBL_H;

    pDataInfo->reg_dmc_h_block            = header.DEMURA_BLK_H;         //  3 bit
    pDataInfo->reg_dmc_v_block            = header.DEMURA_BLK_V;         //  3 bit

    pDataInfo->reg_dmc_plane_num              = header.DEMURA_PLANE_NUM;         //  4 bit
    pDataInfo->reg_dmc_rgb_mode               = header.DEMURA_MODE;         //  1 bit
    pDataInfo->reg_dmc_panel_h_size           = (n_Hnode - 1)*(1 << (pDataInfo->reg_dmc_h_block));//H_node = H_size/H_block_size + 1;       // 13 bit
    pDataInfo->reg_dmc_black_limit            = header.DEMURA_BLACK_LIMIT<<2;       // 12 bit , format 10.2
    pDataInfo->reg_dmc_plane_level1           = header.DEMURA_PLANE_LEVEL1<<2;       // 12 bit , format 10.2
    pDataInfo->reg_dmc_plane_level2           = header.DEMURA_PLANE_LEVEL2<<2;       // 12 bit , format 10.2
    pDataInfo->reg_dmc_plane_level3           = header.DEMURA_PLANE_LEVEL3<<2;       // 12 bit , format 10.2
    pDataInfo->reg_dmc_plane_level4           = header.DEMURA_PLANE_LEVEL4<<2;       // 12 bit , format 10.2
    pDataInfo->reg_dmc_plane_level5           = header.DEMURA_PLANE_LEVEL5<<2;           // 12 bit , format 10.2
    pDataInfo->reg_dmc_plane_level6           = header.DEMURA_PLANE_LEVEL6<<2;           // 12 bit , format 10.2
    pDataInfo->reg_dmc_plane_level7           = header.DEMURA_PLANE_LEVEL7<<2;           // 12 bit , format 10.2
    pDataInfo->reg_dmc_plane_level8           = header.DEMURA_PLANE_LEVEL8<<2;           // 12 bit , format 10.2
    pDataInfo->reg_dmc_white_limit            = header.DEMURA_WHITE_LIMIT<<2;       // 12 bit , format 10.2
    pDataInfo->reg_dmc_dither_en              = get_env_demura_dither();
    pDataInfo->reg_dmc_dither_rst_e_4_frame   = 0x0;         //  1 bit
    pDataInfo->reg_dmc_dither_pse_rst_num     = 0x0;         //  2 bit
    pDataInfo->bROI_en                        = 0x0;
    pDataInfo->iROI_hor_end_offset            = 0x0;
    pDataInfo->iROI_hor_start_offset          = 0x0;
    pDataInfo->iROI_ver_end_offset            = 0x0;
    pDataInfo->iROI_ver_start_offset          = 0x0;
}

MS_BOOL CSOT_CSOT_Set_U13_Format(CSOT_CSOT_Demura_Header *phdr, interface_info *pinfo)
{
    // CSOT only porting mono mode.
    int s, i, j, k, c, data = 0;
    int table_addr = 0;
    int nLayer = 0, idx_image_size;
    int /*burst_num = 0,*/ data_per_burst = 0, data_last_burst = 0, skip_last_byte = 0, burst_bit = 128;
    MS_BOOL pol = 0;
    MS_U8 /*IntBitWidth = 0,*/ DecimalsBitWidth = 0, DataBit = CSOT_CSOT_DATA_BIT_NUM;
    //MS_U16 gain = 0, offset = 0;
    MS_U16 gain[3] = {0}, offset[3] = {0};
    double LUT_data = 0;
    int c_max, _sectionNum, _DEMURA_TBL_H;

    CSOT_CSOT_Init_U13_Param(phdr, pinfo);

    data_per_burst = burst_bit/DataBit; // 128bit per burst, 12bit per data => 10 data and 8bit dummy byte.
    //printf("data_per_burst = %d \n", data_per_burst);
    //burst_num = (phdr->DEMURA_TBL_H/data_per_burst) + ((phdr->DEMURA_TBL_H%data_per_burst > 0)?1:0);
    //printf("burst_num = %d \n", burst_num);
    data_last_burst = phdr->DEMURA_TBL_H%data_per_burst;
    //printf("data_last_dummy = %d \n", data_last_burst);
    skip_last_byte = burst_bit/8 - data_last_burst;
    //printf("skip_last_byte = %d \n", skip_last_byte);

    //IntBitWidth = phdr->DEMURA_INT_BIT_WIDTH;
    DecimalsBitWidth = phdr->DEMURA_DEC_BIT_WIDTH;
    //printf("IntBitWidth = %d, DecimalsBitWidth = %d \n", IntBitWidth, DecimalsBitWidth);
    _sectionNum = ((CSOT_File_Header*)(phdr->file_header))->sectionNum - 1;

    // Get CSOT gain and offset value
    gain[0] = phdr->DEMURA_GAIN_R;
    offset[0] = phdr->DEMURA_OFFSET_R;
    gain[1] = phdr->DEMURA_GAIN_G;
    offset[1] = phdr->DEMURA_OFFSET_G;
    gain[2] = phdr->DEMURA_GAIN_B;
    offset[2] = phdr->DEMURA_OFFSET_B;

    if(phdr->DEMURA_MODE== 1) // RGB mode
        c_max = 3;
    else
        c_max = 1;
    // assign LUT 12 bit format

    _DEMURA_TBL_H = (phdr->DEMURA_TBL_H-1)/_sectionNum + 1;
    for (s = 0; s < _sectionNum; s++)   //0~240 240~480
    {
        pol = 0;
        table_addr = 0;
        for (i = 0; i < phdr->DEMURA_TBL_V; i++)
        {
            for (k = 0; k < pinfo->reg_dmc_plane_num; k++)
            {
                for (c = 0; c < c_max; c++)
                {
                    // RGB data
                    for (j = 0; j < (_DEMURA_TBL_H) ; j++)
                    {
                        // LUT index for the plane
                        idx_image_size = i * (phdr->DEMURA_TBL_H) + j + s * (_DEMURA_TBL_H - 1);
                        // 12 bit each for 10 bit value
                        if (pol == 0) //8bit + 4bit
                        {
                            data = ((MS_U16)(phdr->lut_buffer[s])[table_addr])
                                | (((MS_U16)(phdr->lut_buffer[s])[table_addr + 1] & _0X0F) << _8);
                            pol = _1;
                            table_addr += _1;
                        }
                        else
                        {
                            data = (((MS_U16)phdr->lut_buffer[s][table_addr] & _0XF0)>> _4)
                                | (((MS_U16)phdr->lut_buffer[s][table_addr + 1] & _0XFF) << _4);
                            pol = 0;
                            table_addr += _2;
                        }

                        if ((j + 1) % data_per_burst == 0) // The last data of the burst
                        {
                            table_addr++;
                            pol = 0;
                        }
                        if (j == (_DEMURA_TBL_H - 1)) // The last data of the H
                        {
                            table_addr+=skip_last_byte; // skip dummy 15 bytes
                            pol = 0;
                        }

                        if ((data & _BIT11) == _BIT11) // signed bit
                        {
                            data = (_0X1000 - data)*(-1);
                        }

                        switch (k)
                        {
                            case _0:
                                nLayer = pinfo->reg_dmc_plane_level1;
                                break;
                            case _1:
                                nLayer = pinfo->reg_dmc_plane_level2;
                                break;
                            case _2:
                                nLayer = pinfo->reg_dmc_plane_level3;
                                break;
                            case _3:
                                nLayer = pinfo->reg_dmc_plane_level4;
                                break;
                            case _4:
                                nLayer = pinfo->reg_dmc_plane_level5;
                                break;
                            case _5:
                                nLayer = pinfo->reg_dmc_plane_level6;
                                break;
                            case _6:
                                nLayer = pinfo->reg_dmc_plane_level7;
                                break;
                            case _7:
                                nLayer = pinfo->reg_dmc_plane_level8;
                                break;
                            default:
                                break;
                        }

                        // LUT_comp_value = LUT_data*gain/16 + offset
                        LUT_data = ((double)(data*gain[c] / (double)_16 + offset[c])
                            / (1 << DecimalsBitWidth)) + (double)(nLayer / (double)_4); // 10bit data + layer (12bit to 10bit)
                        if (c == 0)
                        {
                            if (phdr->DEMURA_MODE == 1) // RGB mode
                            {
                                // R data
                                //pinfo->Lut_in[k][idx_image_size].dbr   = LUT_data;
                                pinfo->Lut_in[k][idx_image_size].r   = (int)LUT_data;
                            }
                            else
                            {
                                //pinfo->Lut_in[k][idx_image_size].dbr   = LUT_data;
                                //pinfo->Lut_in[k][idx_image_size].dbg   = LUT_data;
                                //pinfo->Lut_in[k][idx_image_size].dbb   = LUT_data;
                                //#if (defined (CONFIG_DEMURA_M7622) || defined (CONFIG_DEMURA_M7632))
                                //pinfo->Lut_in[k][idx_image_size].dbw   = LUT_data;
                                //#endif

                                pinfo->Lut_in[k][idx_image_size].r   = (int)LUT_data;
                                pinfo->Lut_in[k][idx_image_size].g   = (int)LUT_data;
                                pinfo->Lut_in[k][idx_image_size].b   = (int)LUT_data;
                                #if (defined (CONFIG_DEMURA_M7622) || defined (CONFIG_DEMURA_M7632))
                                pinfo->Lut_in[k][idx_image_size].w   = (int)LUT_data;
                                #endif
                            }
                        }
                        else if (c == _1)
                        {
                            // G data
                            //pinfo->Lut_in[k][idx_image_size].dbg   = LUT_data;
                            pinfo->Lut_in[k][idx_image_size].g   = (int)LUT_data;
                        }
                        else if (c == _2)
                        {
                            // B data
                            //pinfo->Lut_in[k][idx_image_size].dbb   = LUT_data;
                            pinfo->Lut_in[k][idx_image_size].b   = (int)LUT_data;
                            #if (defined (CONFIG_DEMURA_M7622) || defined (CONFIG_DEMURA_M7632))
                            //pinfo->Lut_in[k][idx_image_size].dbw   = (pinfo->Lut_in[k][idx_image_size].dbr
                            //    + pinfo->Lut_in[k][idx_image_size].dbg
                            //    + pinfo->Lut_in[k][idx_image_size].dbb)/_3;
                            pinfo->Lut_in[k][idx_image_size].w   = (int)pinfo->Lut_in[k][idx_image_size].dbw;
                            #endif
                        }

                    }
                }
            }
        }
    }
    return TRUE;
}

#endif
static MS_BOOL CSOT_CSOT_Decode_To_Mstar_Format(CSOT_CSOT_Demura_Header *phdr, interface_info *pinfo)
{
    MS_BOOL bRet = FALSE;

    #if defined (CONFIG_DEMURA_URSA11)
        bRet = CSOT_CSOT_Set_U11_Format(phdr, pinfo);
    #else
        bRet = CSOT_CSOT_Set_U13_Format(phdr, pinfo);
    #endif

    return bRet;
}

MS_BOOL Decode_To_Mstar_Format_CSOT_CSOT(void *pDataInfo, BinOutputInfo *pbin_info)
{
    MS_BOOL bRet = FALSE;
    CSOT_CSOT_Demura_Header header;

    // Parse header information
    memset(&header, 0, sizeof(header));
    if (load_vendor_header_csot_csot(&header) != TRUE)
    {
        printf("load_vendor_header Error\n");
        return FALSE;
    }

    if (parse_vendor_header_csot_csot(&header) != TRUE)
    {
        printf("parse_vendor_header Error\n");
        return FALSE;
    }
    //dump_vendor_header(&header);

    if (load_vendor_lut_csot_csot(&header) != TRUE)
    {
        printf("load_vendor_lut Error\n");
        dfree(header.file_header);
        dfree(header.section_header);
        dfree(header.demura_header);
        header.file_header = NULL;
        header.section_header = NULL;
        header.demura_header = NULL;
        return FALSE;
    }

    Gen_Sf_Signature_CSOT_CSOT(&header);
    // Alloc space for Lut_in
    int n_Hnode = header.DEMURA_TBL_H;
    int n_Vnode = header.DEMURA_TBL_V;

    // Init Output message
    pbin_info->HNode          = n_Hnode;
    pbin_info->VNode          = n_Vnode;
    pbin_info->LevelCount     = header.DEMURA_PLANE_NUM;
    pbin_info->Blk_h_size     = 1 << (header.DEMURA_BLK_H);
    pbin_info->Blk_v_size     = 1 << (header.DEMURA_BLK_V);

    bRet = Alloc_LutIn_Space(pDataInfo, pbin_info);
    if (bRet == FALSE)
    {
        printf("Alloc_LutIn_Space error\n");
        dfree(header.file_header);
        dfree(header.section_header);
        dfree(header.demura_header);
        header.file_header = NULL;
        header.section_header = NULL;
        header.demura_header = NULL;
        dfree(header.lut_buffer[0]);
        dfree(header.lut_buffer[1]);
        header.lut_buffer[0] = NULL;
        header.lut_buffer[1] = NULL;
        return FALSE;
    }

    // Decode Vendor data
    bRet = CSOT_CSOT_Decode_To_Mstar_Format(&header, (interface_info *)pDataInfo);
    if (bRet == FALSE)
    {
        printf("CSOT_CSOT_Decode_To_Mstar_Format error\n");
        return FALSE;
    }

/*
    #if defined (CONFIG_DEMURA_URSA13)
    MS_BOOL demura_enable[3]  = {TRUE, TRUE, TRUE};
    #else
    MS_BOOL demura_enable[4]  = {TRUE, TRUE, TRUE, TRUE};
    #endif
*/
    pbin_info->reg_base_addr  =  0x7700;
    pbin_info->project_id     =  ((MSTAR_CHIP_ID << 16) + (1ULL << 15) + EN_DEMURA_MULTI_CSOT_CSOT);
    pbin_info->data_type      =  IC_DRAM;
    pbin_info->Gain_type      =  10;
    pbin_info->Sep_type       =  header.DEMURA_MODE;
    pbin_info->Build_Date     =  0x20060214;

    //memcpy(pbin_info->channel_enable, demura_enable, sizeof(demura_enable));

    return TRUE;
}

// { BOE_ESWIN Start
MS_BOOL If_Need_Decode_BOE_ESWIN(void)
{
    UBOOT_TRACE("IN\n");
#if defined(CONFIG_DEMURA_VENDOR_BACKLIGHT)
    char *sig_str;
    if (E_MS_UTIL_BIN_FILE_BACKLIGHT == get_demura_file())
    {
        sig_str = env_get(ENV_DEMURA_SIG_BL);
    }
    else
    {
        sig_str = env_get(ENV_DEMURA_SIG);
    }
#else
    char *sig_str = env_get(ENV_DEMURA_SIG);
#endif

    if (sig_str == NULL)
    {
        UBOOT_DEBUG("Empty Board, should decoding data\n");
        UBOOT_TRACE("OK\n");
        return TRUE;
    }
    else
    {
        MS_U16 lut_checksum_sf;
        MS_U32 lut_checksum_bd;
        MS_U32 sample_crc32, cal_crc32;
        MS_U8  buf[SAMPLE_DAT_LEN];

        lut_checksum_sf = get_lut_checksum_csot_himax();

        UBOOT_DEBUG("sig_str = %s\n", sig_str);
        if(2 == sscanf(sig_str, "0x%x:0x%x", &lut_checksum_bd, &sample_crc32))
        {
            UBOOT_TRACE("lut_checksum_sf = 0x%x\n", (uint)lut_checksum_sf);
            UBOOT_TRACE("lut_checksum_bd = 0x%x\n", (uint)lut_checksum_bd);
            UBOOT_TRACE("sample_crc32  = 0x%x\n",   (uint)sample_crc32);

            if (read_spi_flash(buf, (CSOT_HIMAX_LUT_START + SAMPLE_DAT_START), SAMPLE_DAT_LEN) == TRUE)
            {
                cal_crc32 = MDrv_CRC32_Cal_DeMura(buf, SAMPLE_DAT_LEN);
                UBOOT_DEBUG("cal_crc32     = 0x%x\n", (uint)cal_crc32);
                if ((sample_crc32 == cal_crc32) && (lut_checksum_sf == lut_checksum_bd))
                {
                    UBOOT_TRACE("Data Match, No Need to decode again\n");
                    UBOOT_TRACE("OK\n");
                    return FALSE;
                }
            }
        }

        UBOOT_TRACE("OK\n");
        return TRUE;
    }

    UBOOT_TRACE("OK\n");
    return FALSE;
}

static void BOE_ESWIN_Init_Param(BOE_ESWIN_Demura_Header *phdr, interface_info *pDataInfo)
{
    pDataInfo->reg_dmc_h_block            = HAL_DEMURA_Get_HBlockShift(phdr->u8HblockSize);         //  3 bit
    pDataInfo->reg_dmc_v_block            = HAL_DEMURA_Get_VBlockShift(phdr->u8VblockSize);         //  3 bit

    pDataInfo->reg_dmc_plane_num              = phdr->u8PlaneNum;         //  4 bit
    pDataInfo->reg_dmc_rgb_mode               = phdr->enDemuraMod;         //  1 bit
    pDataInfo->reg_dmc_panel_h_size           = (phdr->u16HLutNum - 1)*(1 << (pDataInfo->reg_dmc_h_block));//H_node = H_size/H_block_size + 1;       // 13 bit
    pDataInfo->reg_dmc_black_limit            = phdr->u16BlackLimit << 2;       // 12 bit , format 10.2
    pDataInfo->reg_dmc_plane_level1           = phdr->u16PlaneLevel[0] << 2;       // 12 bit , format 10.2
    pDataInfo->reg_dmc_plane_level2           = phdr->u16PlaneLevel[1] << 2;       // 12 bit , format 10.2
    pDataInfo->reg_dmc_plane_level3           = phdr->u16PlaneLevel[2] << 2;       // 12 bit , format 10.2
    pDataInfo->reg_dmc_plane_level4           = phdr->u16PlaneLevel[3] << 2;       // 12 bit , format 10.2
    pDataInfo->reg_dmc_plane_level5           = phdr->u16PlaneLevel[4] << 2;           // 12 bit , format 10.2
    pDataInfo->reg_dmc_plane_level6           = phdr->u16PlaneLevel[5] << 2;           // 12 bit , format 10.2
    pDataInfo->reg_dmc_plane_level7           = phdr->u16PlaneLevel[6] << 2;           // 12 bit , format 10.2
    pDataInfo->reg_dmc_plane_level8           = phdr->u16PlaneLevel[7] << 2;           // 12 bit , format 10.2
    pDataInfo->reg_dmc_white_limit            = phdr->u16WhiteLimit << 2;       // 12 bit , format 10.2
    pDataInfo->reg_dmc_dither_en              = get_env_demura_dither();
    pDataInfo->reg_dmc_dither_rst_e_4_frame   = 0x0;         //  1 bit
    pDataInfo->reg_dmc_dither_pse_rst_num     = 0x0;         //  2 bit
    pDataInfo->bROI_en                        = 0x0;
    pDataInfo->iROI_hor_end_offset            = 0x0;
    pDataInfo->iROI_hor_start_offset          = 0x0;
    pDataInfo->iROI_ver_end_offset            = 0x0;
    pDataInfo->iROI_ver_start_offset          = 0x0;
#if 0
    UBOOT_TRACE("pDataInfo->reg_dmc_plane_level1=%x \n", pDataInfo->reg_dmc_plane_level1);
    UBOOT_TRACE("pDataInfo->reg_dmc_plane_level2=%x \n", pDataInfo->reg_dmc_plane_level2);
    UBOOT_TRACE("pDataInfo->reg_dmc_plane_level3=%x \n", pDataInfo->reg_dmc_plane_level3);
    UBOOT_TRACE("pDataInfo->reg_dmc_plane_level4=%x \n", pDataInfo->reg_dmc_plane_level4);
    UBOOT_TRACE("pDataInfo->reg_dmc_plane_level5=%x \n", pDataInfo->reg_dmc_plane_level5);
    UBOOT_TRACE("pDataInfo->reg_dmc_plane_level6=%x \n", pDataInfo->reg_dmc_plane_level6);
    UBOOT_TRACE("pDataInfo->reg_dmc_plane_level7=%x \n", pDataInfo->reg_dmc_plane_level7);
    UBOOT_TRACE("pDataInfo->reg_dmc_plane_level8=%x \n", pDataInfo->reg_dmc_plane_level8);
#endif
}

static void BOE_ESWIN_Parse_Lut(BOE_ESWIN_Demura_Header *phdr, MS_U8 *pLine, interface_info *pinfo, int v)
{
    int idx_image_size = v * phdr->u16HLutNum;
    int segnum, session;
    MS_U8 plane = 0;
    int j;
    int hnode = 0;
    int LUT_data;
    int nLayer = 0;
    int remain_idx = 0;
    int remain_bytes = 0;
    int src_idx = 0, base;
    MS_U8 u8Data[BOE_ESWIN_REPEATED_BYTES];
    for (session = 0; session < phdr->u32SegsPerLine; session++)
    {
        base = session * phdr->u32SegAlignedBytes;
        src_idx = 0;

        for (segnum = 0; segnum < phdr->u32SegNum; segnum += BOE_ESWIN_REPEATED_MAX_IDX)
        {
            memset(&u8Data[0], 0, sizeof(u8Data));
            remain_idx = BOE_ESWIN_REPEATED_MAX_IDX;
            remain_bytes = BOE_ESWIN_REPEATED_BYTES;
            if ((segnum + BOE_ESWIN_REPEATED_MAX_IDX) > phdr->u32SegNum)
            {
                remain_idx = phdr->u32SegNum - segnum;
                remain_bytes = phdr->u32SegAlignedBytes - src_idx;
            }
            if (remain_idx == BOE_ESWIN_REPEATED_MAX_IDX)
            {
                memcpy(&u8Data[0], &pLine[base + src_idx], BOE_ESWIN_REPEATED_BYTES);
                src_idx += BOE_ESWIN_REPEATED_BYTES;
            }
            else
            {
                memcpy(&u8Data[0], &pLine[base + src_idx], remain_bytes);
                src_idx += BOE_ESWIN_REPEATED_BYTES;
            }
            for (j = 0; (j < remain_idx) && (hnode < phdr->u16HLutNum); j++)
            {
                nLayer = BOE_ESWIN_GET_LAYER(plane, pinfo);
                LUT_data = BOE_ESWIN_GET_LUT_DATA(j, u8Data) - BOE_ESWIN_DATA_BASE + nLayer;
                //pinfo->Lut_in[plane][idx_image_size + hnode].dbr   = (double)LUT_data;
                //pinfo->Lut_in[plane][idx_image_size + hnode].dbg   = (double)LUT_data;
                //pinfo->Lut_in[plane][idx_image_size + hnode].dbb   = (double)LUT_data;
#if (defined (CONFIG_DEMURA_M7622) || defined (CONFIG_DEMURA_M7632))
                //pinfo->Lut_in[plane][idx_image_size + hnode].dbw   = (double)LUT_data;
#endif
                pinfo->Lut_in[plane][idx_image_size + hnode].r   = LUT_data;
                pinfo->Lut_in[plane][idx_image_size + hnode].g   = LUT_data;
                pinfo->Lut_in[plane][idx_image_size + hnode].b   = LUT_data;
#if (defined (CONFIG_DEMURA_M7622) || defined (CONFIG_DEMURA_M7632))
                pinfo->Lut_in[plane][idx_image_size + hnode].w   = LUT_data;
#endif
                plane++;
                if (plane >= phdr->u8PlaneNum)
                {
                    hnode++;
                    plane = 0;
                }
            }
        }
    }
}

static MS_BOOL BOE_ESWIN_Decode_To_Mstar_Format(BOE_ESWIN_Demura_Header *phdr, interface_info *pinfo)
{
    int v;
    int idx_image_size;
    MS_U8 *pLut = phdr->lut_buffer;
    MS_BOOL bRet = TRUE;

    BOE_ESWIN_Init_Param(phdr, pinfo);
    for (v = 0; v < phdr->u16VLutNum; v++)
    {
        idx_image_size = v * phdr->u32BytesPerLine;
        BOE_ESWIN_Parse_Lut(phdr, &pLut[idx_image_size], pinfo, v);
    }

    return bRet;
}

MS_BOOL Decode_To_Mstar_Format_BOE_ESWIN(void *pDataInfo, BinOutputInfo *pbin_info)
{
    MS_BOOL bRet = FALSE;
    BOE_ESWIN_Demura_Header header;

    // Parse header information
    memset(&header, 0, sizeof(header));
    if (load_vendor_header_boe_eswin(&header) != TRUE)
    {
        UBOOT_ERROR("load_vendor_header Error\n");
        return FALSE;
    }

    if (parse_vendor_header_boe_eswin(&header) != TRUE)
    {
        UBOOT_ERROR("parse_vendor_header Error\n");
        return FALSE;
    }

    if (load_vendor_lut_boe_eswin(&header) != TRUE)
    {
        UBOOT_ERROR("load_vendor_lut Error\n");
        return FALSE;
    }

    //env_set(ENV_DEMURA_SIG, strbuf);
    // Alloc space for Lut_in
    int n_Hnode = header.u16HLutNum;
    int n_Vnode = header.u16VLutNum;



    // Init Output message
    pbin_info->HNode          = n_Hnode;
    pbin_info->VNode          = n_Vnode;
    pbin_info->LevelCount     = header.u8PlaneNum;

    pbin_info->Blk_h_size     = header.u8HblockSize;
    pbin_info->Blk_v_size     = header.u8VblockSize;
    //dump_interface_info(pDataInfo);

    bRet = Alloc_LutIn_Space(pDataInfo, pbin_info);
    if (bRet == FALSE)
    {
        printf("Alloc_LutIn_Space error\n");
        if (header.lut_buffer)
        {
            dfree(header.lut_buffer);
            header.lut_buffer = NULL;
        }
        return FALSE;
    }

    // Decode Vendor data
    bRet = BOE_ESWIN_Decode_To_Mstar_Format(&header, (interface_info *)pDataInfo);
    if (bRet == FALSE)
    {
        printf("BOE_ESWIN_Decode_To_Mstar_Format error\n");
        return FALSE;
    }

/*
    #if defined (CONFIG_DEMURA_URSA13) || defined (CONFIG_DEMURA_URSA11)
    MS_BOOL demura_enable[3]  = {TRUE, TRUE, TRUE};
    #else
    MS_BOOL demura_enable[4]  = {TRUE, TRUE, TRUE, TRUE};
    #endif
*/
    pbin_info->reg_base_addr  =  0x7700;
    pbin_info->project_id     =  ((MSTAR_CHIP_ID << 16) + (1ULL << 15) + EN_DEMURA_MULTI_CSOT_HIMAX);
    pbin_info->data_type      =  IC_DRAM;
    pbin_info->Gain_type      =  10;
    #if defined (CONFIG_DEMURA_URSA13) || defined (CONFIG_DEMURA_URSA11)
    pbin_info->Sep_type       =  0;
    #else
    pbin_info->Sep_type       = 0;
    #endif
    pbin_info->Build_Date     =  0x19021814;

    //memcpy(pbin_info->channel_enable, demura_enable, sizeof(demura_enable));

    return TRUE;
}
// } BOE_ESWIN End

// { SIO start
#if 0
static MS_BOOL SIO_Decode_To_Mstar_Format(SIO_DEMURA_BIN_HEADER *phdr, interface_info *pinfo)
{
    int idx_image_size;
    MS_U8 *plut = (MS_U8 *)phdr->lut_buffer;
    int v, h, p, LUT_data = 0, table_addr;

    switch (phdr->HBlock)
    {
        case 4: pinfo->reg_dmc_h_block = 2; break;//  3 bit
        case 8: pinfo->reg_dmc_h_block = 3; break;//  3 bit
        case 16: pinfo->reg_dmc_h_block = 4; break;//  3 bit
        default: return FALSE;
    }

    switch (phdr->VBlock)
    {
        case 4: pinfo->reg_dmc_v_block = 2; break;//  3 bit
        case 8: pinfo->reg_dmc_v_block = 3; break;//  3 bit
        case 16: pinfo->reg_dmc_v_block = 4; break;//  3 bit
        default: return FALSE;
    }
    pinfo->reg_dmc_plane_num          = phdr->DEMURA_CTRL_REG_PLANE_NUM;         //  4 bit
    pinfo->reg_dmc_rgb_mode           = 0;         //  1 bit
    pinfo->reg_dmc_panel_h_size       = 3840;//H_node = H_size/H_block_size + 1;       // 13 bit
    pinfo->reg_dmc_black_limit = phdr->DEMURA_CTRL_REG_LOW_R_LIMIT << 2; // 12 bit , format 10.2
    pinfo->reg_dmc_plane_level1 = phdr->DEMURA_CTRL_REG_PLANE_LEVEL[0] << 2; // 12 bit , format 10.2
    pinfo->reg_dmc_plane_level2 = phdr->DEMURA_CTRL_REG_PLANE_LEVEL[1] << 2; // 12 bit , format 10.2
    pinfo->reg_dmc_plane_level3 = phdr->DEMURA_CTRL_REG_PLANE_LEVEL[2] << 2; // 12 bit , format 10.2
    pinfo->reg_dmc_plane_level4 = phdr->DEMURA_CTRL_REG_PLANE_LEVEL[3] << 2; // 12 bit , format 10.2
    pinfo->reg_dmc_plane_level5 = phdr->DEMURA_CTRL_REG_PLANE_LEVEL[4] << 2; // 12 bit , format 10.2
    pinfo->reg_dmc_plane_level6 = phdr->DEMURA_CTRL_REG_PLANE_LEVEL[5] << 2; // 12 bit , format 10.2
    pinfo->reg_dmc_plane_level7 = phdr->DEMURA_CTRL_REG_PLANE_LEVEL[6] << 2; // 12 bit , format 10.2
    pinfo->reg_dmc_plane_level8 = phdr->DEMURA_CTRL_REG_PLANE_LEVEL[7] << 2; // 12 bit , format 10.2
    pinfo->reg_dmc_white_limit = phdr->DEMURA_CTRL_REG_HIGH_R_LIMIT << 2; // 12 bit , format 10.2
    pinfo->reg_dmc_dither_en = 0x1; //  2 bit , 0 : rounding , 1 : dither , 2~3 : truncate
    pinfo->reg_dmc_dither_rst_e_4_frame = 0x0;         //  1 bit
    pinfo->reg_dmc_dither_pse_rst_num = 0x0;         //  2 bit
    pinfo->bROI_en                    = 0x0;
    pinfo->iROI_hor_end_offset        = 0x0;
    pinfo->iROI_hor_start_offset      = 0x0;
    pinfo->iROI_ver_end_offset        = 0x0;
    pinfo->iROI_ver_start_offset      = 0x0;

    for (p = 0; p < phdr->DEMURA_CTRL_REG_PLANE_NUM; p++)
    {
       for (v = 0; v < phdr->VLutNum; v++)
       {
           for (h = 0; h < phdr->HLutNum ; h++)
           {
               // LUT index for the plane
               idx_image_size = v * phdr->HLutNum + h;

               table_addr = (v * SIO_DEMURA_U32_ALIGN_4BYTES(phdr->HLutNum) * phdr->DEMURA_CTRL_REG_PLANE_NUM) + (p * SIO_DEMURA_U32_ALIGN_4BYTES(phdr->HLutNum)) + h;
               LUT_data = plut[table_addr + SIO_DEMURA_U32_SWAP_OFFSET(h)];
               if (plut[table_addr + SIO_DEMURA_U32_SWAP_OFFSET(h)] >= 0x80)
               {
                   LUT_data -= 256;
               }
               //LUT_data = max(min(1023, LUT_data), 0);
               pinfo->Lut_in[p][idx_image_size].dbr   = (LUT_data) + phdr->DEMURA_CTRL_REG_PLANE_LEVEL[p];
               pinfo->Lut_in[p][idx_image_size].dbg   = (LUT_data) + phdr->DEMURA_CTRL_REG_PLANE_LEVEL[p];
               pinfo->Lut_in[p][idx_image_size].dbb   = (LUT_data) + phdr->DEMURA_CTRL_REG_PLANE_LEVEL[p];
               #if (defined (CONFIG_DEMURA_M7622) || defined (CONFIG_DEMURA_M7632))
               pinfo->Lut_in[p][idx_image_size].dbw   = (LUT_data) + phdr->DEMURA_CTRL_REG_PLANE_LEVEL[p];
               #endif

               pinfo->Lut_in[p][idx_image_size].r   = (int)(LUT_data) + phdr->DEMURA_CTRL_REG_PLANE_LEVEL[p];
               pinfo->Lut_in[p][idx_image_size].g   = (int)(LUT_data) + phdr->DEMURA_CTRL_REG_PLANE_LEVEL[p];
               pinfo->Lut_in[p][idx_image_size].b   = (int)(LUT_data) + phdr->DEMURA_CTRL_REG_PLANE_LEVEL[p];
               #if (defined (CONFIG_DEMURA_M7622) || defined (CONFIG_DEMURA_M7632))
               pinfo->Lut_in[p][idx_image_size].w   = (int)(LUT_data) + phdr->DEMURA_CTRL_REG_PLANE_LEVEL[p];
               #endif
            }
        }
    }


    return TRUE;
}

MS_BOOL Decode_To_Mstar_Format_SIO(void *pDataInfo, BinOutputInfo *pbin_info)
{
    MS_BOOL bRet;
    SIO_DEMURA_BIN_HEADER header;

    // Parse header information
    memset(&header, 0, sizeof(header));
    if (load_vendor_header_sio(&header) != TRUE)
    {
        UBOOT_ERROR("load_vendor_header Error\n");
        return FALSE;
    }

    if (parse_vendor_header_sio(&header) != TRUE)
    {
        UBOOT_ERROR("parse_vendor_header Error\n");
        dfree(header.hdr_buffer);
        header.hdr_buffer = NULL;
        return FALSE;
    }

    if (load_vendor_lut_sio(&header) != TRUE)
    {
        UBOOT_ERROR("load_vendor_lut Error\n");
        dfree(header.hdr_buffer);
        header.hdr_buffer = NULL;
        return FALSE;
    }

    //env_set(ENV_DEMURA_SIG, strbuf);

    // Init Output message
    pbin_info->HNode          = header.HLutNum;
    pbin_info->VNode          = header.VLutNum;
    pbin_info->LevelCount     = header.DEMURA_CTRL_REG_PLANE_NUM;
    pbin_info->Blk_h_size     = header.HBlock;
    pbin_info->Blk_v_size     = header.VBlock;
    //dump_interface_info(pDataInfo);

    bRet = Alloc_LutIn_Space(pDataInfo, pbin_info);
    if (bRet == FALSE)
    {
        printf("Alloc_LutIn_Space error\n");
        dfree(header.lut_buffer);
        header.lut_buffer = NULL;
        dfree(header.hdr_buffer);
        header.hdr_buffer = NULL;
        return FALSE;
    }

    // Decode Vendor data
    bRet = SIO_Decode_To_Mstar_Format(&header, (interface_info *)pDataInfo);
    if (bRet == FALSE)
    {
        printf("SIO_Decode_To_Mstar_Format error\n");
        dfree(header.lut_buffer);
        header.lut_buffer = NULL;
        dfree(header.hdr_buffer);
        header.hdr_buffer = NULL;
        return FALSE;
    }

/*
    #if defined (CONFIG_DEMURA_URSA13) || defined (CONFIG_DEMURA_URSA11)
    MS_BOOL demura_enable[3]  = {TRUE, TRUE, TRUE};
    #else
    MS_BOOL demura_enable[4]  = {TRUE, TRUE, TRUE, TRUE};
    #endif
*/
    pbin_info->reg_base_addr  =  0x7700;
    pbin_info->project_id     =  ((MSTAR_CHIP_ID << 16) + (1ULL << 15) + EN_DEMURA_MULTI_CSOT_HIMAX);
    pbin_info->data_type      =  IC_DRAM;
    pbin_info->Gain_type      =  10;
    #if defined (CONFIG_DEMURA_URSA13) || defined (CONFIG_DEMURA_URSA11)
    pbin_info->Sep_type       =  0;
    #else
    pbin_info->Sep_type       = 0;
    #endif
    pbin_info->Build_Date     =  0x19021814;

    //memcpy(pbin_info->channel_enable, demura_enable, sizeof(demura_enable));
    dfree(header.lut_buffer);
    header.lut_buffer = NULL;
    dfree(header.hdr_buffer);
    header.hdr_buffer = NULL;
    return TRUE;
}
#endif
// } SIO End

// { SIO start
static MS_BOOL Gen_Sf_Signature_SIO(SIO_DEMURA_BIN_HEADER *phdr)
{
    char   strbuf[DEC_1024];
    MS_U32 sample_crc32;
    MS_U8  *buffer;
    MS_S32 s32Ret = 0;

    buffer = phdr->lut_buffer;
    sample_crc32 = MDrv_CRC32_Cal_DeMura(buffer + SAMPLE_DAT_START, SAMPLE_DAT_LEN);
    if (sample_crc32 == HEX_ALL)
    {
        UBOOT_ERROR("MDrv_CRC32_Cal_DeMura error\n");
        return FALSE;
    }
    memset(strbuf, 0, sizeof(strbuf));
    s32Ret = snprintf(strbuf, sizeof(strbuf)-1, "0x%x:0x%x", (unsigned int)phdr->HEADER_LUT_CHECKSUM, (unsigned int)sample_crc32);
    if ((s32Ret < 0) || (s32Ret >= sizeof(strbuf)-1))
    {
        UBOOT_ERROR("size %d \n", s32Ret);
        return FALSE;
    }
    Gen_Sf_Signature(strbuf, s32Ret);
    return TRUE;
}

MS_BOOL If_Need_Decode_SIO(void)
{
    UBOOT_TRACE("IN\n");
    if (init_spi_flash() != TRUE)
    {
        UBOOT_DEBUG("init_spi_flash error!\n");
        return TRUE;
    }
#if defined(CONFIG_DEMURA_VENDOR_BACKLIGHT)
    char *sig_str;
    if (E_MS_UTIL_BIN_FILE_BACKLIGHT == get_demura_file())
    {
        sig_str = env_get(ENV_DEMURA_SIG_BL);
    }
    else
    {
        sig_str = env_get(ENV_DEMURA_SIG);
    }
#else
    char *sig_str = env_get(ENV_DEMURA_SIG);
#endif

    if (sig_str == NULL)
    {
        UBOOT_DEBUG("Empty Board, should decoding data\n");
        UBOOT_TRACE("OK\n");
        return TRUE;
    }
    else
    {
        MS_U16 lut_checksum_sf;
        MS_U32 lut_checksum_bd;
        MS_U32 sample_crc32, cal_crc32;
        MS_U8  buf[SAMPLE_DAT_LEN];
        SIO_DEMURA_BIN_HEADER header;

        memset(&header, 0, sizeof(header));
        if (load_vendor_header_sio(&header) != TRUE)
        {
            UBOOT_ERROR("load_vendor_header Error\n");
            dfree(header.hdr_buffer);
            return FALSE;
        }
        if (parse_vendor_header_sio(&header) != TRUE)
        {
            UBOOT_ERROR("parse_vendor_header Error\n");
            dfree(header.hdr_buffer);
            return FALSE;
        }

        lut_checksum_sf = header.HEADER_LUT_CHECKSUM;

        UBOOT_DEBUG("sig_str = %s\n", sig_str);
        if (DEC_2 == sscanf(sig_str, "0x%x:0x%x", &lut_checksum_bd, &sample_crc32))
        {
            UBOOT_DEBUG("lut_checksum_sf = 0x%x\n", (uint)lut_checksum_sf);
            UBOOT_DEBUG("lut_checksum_bd = 0x%x\n", (uint)lut_checksum_bd);
            UBOOT_DEBUG("sample_crc32  = 0x%x\n",   (uint)sample_crc32);

            if (read_spi_flash(buf, (SIO_LUT_START + SAMPLE_DAT_START), SAMPLE_DAT_LEN) == TRUE)
            {
                cal_crc32 = MDrv_CRC32_Cal_DeMura(buf, SAMPLE_DAT_LEN);
                UBOOT_DEBUG("cal_crc32     = 0x%x\n", (uint)cal_crc32);
                if ((sample_crc32 == cal_crc32) && (lut_checksum_sf == lut_checksum_bd))
                {
                    UBOOT_DEBUG("Data Match, No Need to decode again\n");
                    UBOOT_TRACE("OK\n");
                    dfree(header.hdr_buffer);
                    return FALSE;
                }
            }
        }
        UBOOT_TRACE("OK\n");
        dfree(header.hdr_buffer);
        return TRUE;
    }

    UBOOT_TRACE("OK\n");
    return FALSE;
}

static MS_BOOL SIO_Decode_To_Mstar_Format(SIO_DEMURA_BIN_HEADER *phdr, interface_info *pinfo)
{
    int idx_image_size;
    MS_U8 *plut = (MS_U8 *)phdr->lut_buffer;
    int v, h, p, LUT_data = 0, table_addr;

    switch (phdr->HBlock)
    {
        case 4: pinfo->reg_dmc_h_block = 2; break;//  3 bit
        case 8: pinfo->reg_dmc_h_block = 3; break;//  3 bit
        case 16: pinfo->reg_dmc_h_block = 4; break;//  3 bit
        default: return FALSE;
    }

    switch (phdr->VBlock)
    {
        case 4: pinfo->reg_dmc_v_block = 2; break;//  3 bit
        case 8: pinfo->reg_dmc_v_block = 3; break;//  3 bit
        case 16: pinfo->reg_dmc_v_block = 4; break;//  3 bit
        default: return FALSE;
    }
    pinfo->reg_dmc_plane_num          = phdr->DEMURA_CTRL_REG_PLANE_NUM;         //  4 bit
    pinfo->reg_dmc_rgb_mode           = 0;         //  1 bit
    pinfo->reg_dmc_panel_h_size       = 3840;//H_node = H_size/H_block_size + 1;       // 13 bit
    pinfo->reg_dmc_black_limit = phdr->DEMURA_CTRL_REG_LOW_R_LIMIT << 2; // 12 bit , format 10.2
    pinfo->reg_dmc_plane_level1 = phdr->DEMURA_CTRL_REG_PLANE_LEVEL[0] << 2; // 12 bit , format 10.2
    pinfo->reg_dmc_plane_level2 = phdr->DEMURA_CTRL_REG_PLANE_LEVEL[1] << 2; // 12 bit , format 10.2
    pinfo->reg_dmc_plane_level3 = phdr->DEMURA_CTRL_REG_PLANE_LEVEL[2] << 2; // 12 bit , format 10.2
    pinfo->reg_dmc_plane_level4 = phdr->DEMURA_CTRL_REG_PLANE_LEVEL[3] << 2; // 12 bit , format 10.2
    pinfo->reg_dmc_plane_level5 = phdr->DEMURA_CTRL_REG_PLANE_LEVEL[4] << 2; // 12 bit , format 10.2
    pinfo->reg_dmc_plane_level6 = phdr->DEMURA_CTRL_REG_PLANE_LEVEL[5] << 2; // 12 bit , format 10.2
    pinfo->reg_dmc_plane_level7 = phdr->DEMURA_CTRL_REG_PLANE_LEVEL[6] << 2; // 12 bit , format 10.2
    pinfo->reg_dmc_plane_level8 = phdr->DEMURA_CTRL_REG_PLANE_LEVEL[7] << 2; // 12 bit , format 10.2
    pinfo->reg_dmc_white_limit = phdr->DEMURA_CTRL_REG_HIGH_R_LIMIT << 2; // 12 bit , format 10.2
    pinfo->reg_dmc_dither_en = 0x1; //  2 bit , 0 : rounding , 1 : dither , 2~3 : truncate
    pinfo->reg_dmc_dither_rst_e_4_frame = 0x0;         //  1 bit
    pinfo->reg_dmc_dither_pse_rst_num = 0x0;         //  2 bit
    pinfo->bROI_en                    = 0x0;
    pinfo->iROI_hor_end_offset        = 0x0;
    pinfo->iROI_hor_start_offset      = 0x0;
    pinfo->iROI_ver_end_offset        = 0x0;
    pinfo->iROI_ver_start_offset      = 0x0;

    for (p = 0; p < phdr->DEMURA_CTRL_REG_PLANE_NUM; p++)
    {
       for (v = 0; v < phdr->VLutNum; v++)
       {
           for (h = 0; h < phdr->HLutNum ; h++)
           {
               // LUT index for the plane
               idx_image_size = v * phdr->HLutNum + h;

               table_addr = (v * SIO_DEMURA_U32_ALIGN_4BYTES(phdr->HLutNum) * phdr->DEMURA_CTRL_REG_PLANE_NUM) + (p * SIO_DEMURA_U32_ALIGN_4BYTES(phdr->HLutNum)) + h;
               LUT_data = plut[table_addr + SIO_DEMURA_U32_SWAP_OFFSET(h)];
               if (plut[table_addr + SIO_DEMURA_U32_SWAP_OFFSET(h)] >= 0x80)
               {
                   LUT_data -= 256;
               }
               //LUT_data = max(min(1023, LUT_data), 0);
               //pinfo->Lut_in[p][idx_image_size].dbr   = (LUT_data) + phdr->DEMURA_CTRL_REG_PLANE_LEVEL[p];
               //pinfo->Lut_in[p][idx_image_size].dbg   = (LUT_data) + phdr->DEMURA_CTRL_REG_PLANE_LEVEL[p];
               //pinfo->Lut_in[p][idx_image_size].dbb   = (LUT_data) + phdr->DEMURA_CTRL_REG_PLANE_LEVEL[p];
               //#if (defined (CONFIG_DEMURA_M7622) || defined (CONFIG_DEMURA_M7632))
               //pinfo->Lut_in[p][idx_image_size].dbw   = (LUT_data) + phdr->DEMURA_CTRL_REG_PLANE_LEVEL[p];
               //#endif

               pinfo->Lut_in[p][idx_image_size].r   = (int)(LUT_data) + phdr->DEMURA_CTRL_REG_PLANE_LEVEL[p];
               pinfo->Lut_in[p][idx_image_size].g   = (int)(LUT_data) + phdr->DEMURA_CTRL_REG_PLANE_LEVEL[p];
               pinfo->Lut_in[p][idx_image_size].b   = (int)(LUT_data) + phdr->DEMURA_CTRL_REG_PLANE_LEVEL[p];
               #if (defined (CONFIG_DEMURA_M7622) || defined (CONFIG_DEMURA_M7632))
               pinfo->Lut_in[p][idx_image_size].w   = (int)(LUT_data) + phdr->DEMURA_CTRL_REG_PLANE_LEVEL[p];
               #endif
            }
        }
    }

    // Sign the SIO dmc data !
    Gen_Sf_Signature_SIO(phdr);

    return TRUE;
}

MS_BOOL Decode_To_Mstar_Format_SIO(void *pDataInfo, BinOutputInfo *pbin_info)
{
    MS_BOOL bRet;
    SIO_DEMURA_BIN_HEADER header;

    // Parse header information
    memset(&header, 0, sizeof(header));
    if (load_vendor_header_sio(&header) != TRUE)
    {
        UBOOT_ERROR("load_vendor_header Error\n");
        return FALSE;
    }

    if (parse_vendor_header_sio(&header) != TRUE)
    {
        UBOOT_ERROR("parse_vendor_header Error\n");
        dfree(header.hdr_buffer);
        header.hdr_buffer = NULL;
        return FALSE;
    }

    if (load_vendor_lut_sio(&header) != TRUE)
    {
        UBOOT_ERROR("load_vendor_lut Error\n");
        dfree(header.hdr_buffer);
        header.hdr_buffer = NULL;
        return FALSE;
    }

    //env_set(ENV_DEMURA_SIG, strbuf);

    // Init Output message
    pbin_info->HNode          = header.HLutNum;
    pbin_info->VNode          = header.VLutNum;
    pbin_info->LevelCount     = header.DEMURA_CTRL_REG_PLANE_NUM;
    pbin_info->Blk_h_size     = header.HBlock;
    pbin_info->Blk_v_size     = header.VBlock;
    //dump_interface_info(pDataInfo);

    bRet = Alloc_LutIn_Space(pDataInfo, pbin_info);
    if (bRet == FALSE)
    {
        printf("Alloc_LutIn_Space error\n");
        dfree(header.lut_buffer);
        header.lut_buffer = NULL;
        dfree(header.hdr_buffer);
        header.hdr_buffer = NULL;
        return FALSE;
    }

    // Decode Vendor data
    bRet = SIO_Decode_To_Mstar_Format(&header, (interface_info *)pDataInfo);
    if (bRet == FALSE)
    {
        printf("SIO_Decode_To_Mstar_Format error\n");
        dfree(header.lut_buffer);
        header.lut_buffer = NULL;
        dfree(header.hdr_buffer);
        header.hdr_buffer = NULL;
        return FALSE;
    }

/*
    #if defined (CONFIG_DEMURA_URSA13) || defined (CONFIG_DEMURA_URSA11)
    MS_BOOL demura_enable[3]  = {TRUE, TRUE, TRUE};
    #else
    MS_BOOL demura_enable[4]  = {TRUE, TRUE, TRUE, TRUE};
    #endif
*/
    pbin_info->reg_base_addr  =  0x7700;
    pbin_info->project_id     =  ((MSTAR_CHIP_ID << DEC_16) + (1ULL << DEC_15) + EN_DEMURA_MULTI_SIO);
    pbin_info->data_type      =  IC_DRAM;
    pbin_info->Gain_type      =  10;
    #if defined (CONFIG_DEMURA_URSA13) || defined (CONFIG_DEMURA_URSA11)
    pbin_info->Sep_type       =  0;
    #else
    pbin_info->Sep_type       = 0;
    #endif
    pbin_info->Build_Date     =  0x19021814;

    //memcpy(pbin_info->channel_enable, demura_enable, sizeof(demura_enable));
    dfree(header.lut_buffer);
    header.lut_buffer = NULL;
    dfree(header.hdr_buffer);
    header.hdr_buffer = NULL;
    return TRUE;
}

// } SIO End

