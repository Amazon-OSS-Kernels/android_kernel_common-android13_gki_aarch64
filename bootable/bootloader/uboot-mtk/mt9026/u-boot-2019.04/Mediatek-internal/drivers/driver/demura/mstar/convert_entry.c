// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

#include <command.h>
#include <common.h>
#include <malloc.h>
#include <stdio.h>
#include <debug_impl.h>
#include <MsTypes.h>
//#include <ShareType.h>
////#include <MsStr.h>

#include <dmalloc.h>
#include <CommonDataType.h>
#include <demura.h>
#include <demuraDll.h>
#include <demura_config.h>
#include <ms_utils.h>
#include <vendor/decoder_input.h>
#if defined(CONFIG_DEMURA_VENDOR_BACKLIGHT)
#include <dts_parser.h>
#include <panel_impl.h>
#endif

#define DEC_2 (2)
#define DEC_3 (3)
#define DEC_16 (16)
#define HEX_FFFF (0xFFFF)

#define ABS(x)   ((x) > 0 ? (x) : -(x))

void memadd(int *arr1, int *arr2, int len)
{
    int i;
    for (i = 0; i < (len >> DEC_2); i++)
    {            //int add is 4 bytes
        *(arr1 + i) = *(arr1 + i) + *(arr2 + i);
    }
}

void strgb_structInfo_memaddsub(strgb_structInfo *arr1, strgb_structInfo *arr2, int sub, int len)
{
    int i;
    for (i = 0; i < len; i++)
    {
        *(&(arr1[i].r)) = *(&(arr1[i].r)) + *(&(arr2[i].r)) - sub;
        *(&(arr1[i].g)) = *(&(arr1[i].g)) + *(&(arr2[i].g)) - sub;
        *(&(arr1[i].b)) = *(&(arr1[i].b)) + *(&(arr2[i].b)) - sub;

        // *(&(arr1[i].dbr)) = *(&(arr1[i].dbr)) + *(&(arr2[i].dbr)) - sub;
        // *(&(arr1[i].dbg)) = *(&(arr1[i].dbg)) + *(&(arr2[i].dbg)) - sub;
        // *(&(arr1[i].dbb)) = *(&(arr1[i].dbb)) + *(&(arr2[i].dbb)) - sub;
    }
}

MS_BOOL init_demura_heap(void)
{
 #if 0
    MS_U32 base_addr = 0;
    MS_U32 buf_len   = CONVERT_BUFFER_SIZE;

    if (get_large_memory(&base_addr, buf_len) == FALSE)
    {
        UBOOT_ERROR("failed to get Demura_Convert_Buf\n");
        return FALSE;
    }

    // init dmalloc buffer
    dmalloc_init((unsigned char *)base_addr, buf_len);
#endif
    return TRUE;
}

MS_BOOL Alloc_LutIn_Space(void *pdat_info, BinOutputInfo *pbin_info)
{
    int n_Hnode     = pbin_info->HNode;
    int n_Vnode     = pbin_info->VNode;
    int nTotalNode  = n_Hnode * n_Vnode;
    int level_count = pbin_info->LevelCount;

    if ((n_Hnode == 0) || (n_Vnode == 0) || (level_count == 0))
    {
        UBOOT_ERROR("n_Hnode = %d, n_Vnode = %d, level_count = %d\n", n_Hnode, n_Vnode, level_count);
        UBOOT_ERROR("Alloc_LutIn_Space Failed !\n");
        return FALSE;
    }

    #if defined (CONFIG_DEMURA_URSA11)
        int n_Layer     = 6;     // Wait to fix it.
        int layer_size  = sizeof(double) * nTotalNode;
        interface_info *pChanData = (interface_info *)pdat_info;

        int idx, channel;
        for(channel = 0; channel < 3; channel++)
        {
            double **lut_in  = (double **)malloc(sizeof(double *) * 6);

            for(idx = 0; idx < n_Layer; idx++)
            {
                char *pLut_in = dmalloc(layer_size);
                CHECK_DMALLOC_SPACE(pLut_in, layer_size);
                lut_in[idx] = (double *)pLut_in;
            }
            pChanData[channel].Lut_in = lut_in;
        }

    #elif defined (CONFIG_DEMURA_URSA13)
        int n_Layer     = level_count > 0 ? level_count : 8;
        int layer_size  = sizeof(strgb_structInfo) * nTotalNode;
        interface_info *pDataInfo = (interface_info *)pdat_info;

        int idx;
        for (idx = 0; idx < n_Layer; idx++)
        {
            char *pLut_in  = (char *)dmalloc(layer_size);
            CHECK_DMALLOC_SPACE(pLut_in, layer_size);
            pDataInfo->Lut_in[idx] = (strgb_structInfo *)pLut_in;
        }
        for (idx = n_Layer; idx < 8; idx++)
        {
            pDataInfo->Lut_in[idx] = NULL;   // Init to NULL value
        }
    #elif (defined (CONFIG_DEMURA_M7622) || defined (CONFIG_DEMURA_M7632))   // Morton case
        int n_Layer     = level_count > 0 ? level_count : 8;
        int layer_size  = sizeof(strgb_structInfo) * nTotalNode;
        interface_info *pDataInfo = (interface_info *)pdat_info;

        int idx;
        for (idx = 0; idx < n_Layer; idx++)
        {
            char *pLut_in  = (char *)dmalloc(layer_size);
            CHECK_DMALLOC_SPACE(pLut_in, layer_size);
            pDataInfo->Lut_in[idx] = (strgb_structInfo *)pLut_in;
        }
        for (idx = n_Layer; idx < 8; idx++)
        {
            pDataInfo->Lut_in[idx] = NULL;   // Init to NULL value
        }
    #elif defined (CONFIG_DEMURA_MT5896)   // M6
        int n_Layer     = level_count > 0 ? level_count : 9;
        int layer_size  = sizeof(strgb_structInfo) * nTotalNode;
        interface_info *pDataInfo = (interface_info *)pdat_info;

        int idx;
        for (idx = 0; idx < n_Layer; idx++)
        {
            char *pLut_in  = (char *)dmalloc(layer_size);
            CHECK_DMALLOC_SPACE(pLut_in, layer_size);
            pDataInfo->Lut_in[idx] = (strgb_structInfo *)pLut_in;
        }
        for (idx = n_Layer; idx < 8; idx++)
        {
            pDataInfo->Lut_in[idx] = NULL;   // Init to NULL value
        }
    #endif

    return TRUE;
}

int demura_generate_bin_file(interface_info *DataInfo, BinOutputInfo *pbin_info,
    EN_DEMURA_MULTI_VENDOR multi_vendor)
{
    int ret = TRUE;

    memset(DataInfo, 0, sizeof(interface_info));

    if (multi_vendor == EN_DEMURA_MULTI_AUO)
    {
        ret = Decode_To_Mstar_Format_AUO((void *)DataInfo, pbin_info);
    }
    else if ((multi_vendor > EN_DEMURA_MULTI_LGD_START) && (multi_vendor < EN_DEMURA_MULTI_LGD_END))
    {
        ret = Decode_To_Mstar_Format_LGD((void *)DataInfo, pbin_info, multi_vendor);
    }
    else if (multi_vendor == EN_DEMURA_MULTI_NOVA)
    {
        ret = Decode_To_Mstar_Format_NOVA((void *)DataInfo, pbin_info, multi_vendor, NOVA_Shift);
    }
    else if (multi_vendor == EN_DEMURA_MULTI_CSOT_HI_SILICON)
    {
        ret = Decode_To_Mstar_Format_CSOT_HI_SILICON((void *)DataInfo, pbin_info);
    }
    else if (multi_vendor == EN_DEMURA_MULTI_CSOT_HIMAX)
    {
        ret = Decode_To_Mstar_Format_CSOT_HIMAX((void *)DataInfo, pbin_info);
    }
    else if (multi_vendor == EN_DEMURA_MULTI_SDC)
    {
        ret = Decode_To_Mstar_Format_SDC((void *)DataInfo, pbin_info);
    }
    else if (multi_vendor == EN_DEMURA_MULTI_INX)
    {
        ret = Decode_To_Mstar_Format_INX((void *)DataInfo, pbin_info);
    }
    else if (multi_vendor == EN_DEMURA_MULTI_CSOT_CSOT)
    {
        ret = Decode_To_Mstar_Format_CSOT_CSOT((void *)DataInfo, pbin_info);
    }
    else if (multi_vendor == EN_DEMURA_MULTI_HKC_NOVA)
    {
        ret = Decode_To_Mstar_Format_NOVA((void *)DataInfo, pbin_info, multi_vendor, H_K_C_NOVA_Shift); //H_K_C's demura use NOVA format, just offset 0x1000
    }
    else if (multi_vendor == EN_DEMURA_MULTI_BOE_ESWIN)
    {
        ret = Decode_To_Mstar_Format_BOE_ESWIN((void *)DataInfo, pbin_info);
    }
    else
    {
        switch (multi_vendor)
        {
            case EN_DEMURA_MULTI_H_K_C_NOVA_120HZ:
                ret = Decode_To_Mstar_Format_NOVA((void *)DataInfo, pbin_info, multi_vendor, H_K_C_NOVA_Shift_120HZ); //H_K_C's demura use NOVA format, just offset 0x2000
                break;
            case EN_DEMURA_MULTI_SIO:
                ret = Decode_To_Mstar_Format_SIO((void *)DataInfo, pbin_info);
                break;
            default:
                ret = FALSE;
                break;
        }
    }
    return ret;
}

int init_backlight_demura_bin_info(interface_info *DataInfo, BinOutputInfo *pbin_info)
{
    int ret = TRUE;
#if defined(CONFIG_DEMURA_VENDOR_BACKLIGHT)
    st_cust_dmc_info st_cust_dmc_info;

    memset(&st_cust_dmc_info, 0x00, sizeof(st_cust_dmc_info));
    parse_dt("/video_out", cus_demura_dt_parser, (void*)&st_cust_dmc_info, NULL);
    if ((st_cust_dmc_info.bl_dmc_enable || env_get(DEMURA_ENV_VENDORID_BACKLIGHT)) && (get_demura_file() != E_MS_UTIL_BIN_FILE_BACKLIGHT_ONLY))
    {
#if defined(CONFIG_DEMURA_URSA11)
        memcpy(&(pbin_info->Info), DataInfo, sizeof(interface_info) * DEC_3);
#elif defined(CONFIG_DEMURA_URSA13)
        memcpy(&(pbin_info->Info), DataInfo, sizeof(interface_info));
#elif defined(CONFIG_DEMURA_M7622)
        memcpy(&(pbin_info->Info), DataInfo, sizeof(interface_info));
#elif defined(CONFIG_DEMURA_M7632)
        memcpy(&(pbin_info->Info), DataInfo, sizeof(interface_info));
#elif defined(CONFIG_DEMURA_MT5896)
        memcpy(&(pbin_info->Info), DataInfo, sizeof(interface_info));
#endif

        return 0;
    }

#endif
    return ret;
}

#if defined(CONFIG_DEMURA_VENDOR_MULTI)
int do_demura_convert(BinOutputInfo *pbin_info, EN_DEMURA_MULTI_VENDOR multi_vendor)
{
    MS_BOOL ret; // fix coverity
    //char  cmd[256];

    if (check_str_resume() == 1)
    {
        //UBOOT_DEBUG("Can not covert demura file during STR\n");
        return 0;
    }

    UBOOT_DEBUG("=================================\n");
    UBOOT_DEBUG("DEMURA_URSA_TYPE : %s\n", CONFIG_DEMURA_URSA_STRING);
    UBOOT_DEBUG("DEMURA_VENDOR    : %s\n",  CONFIG_DEMURA_VENDOR_STRING);
    UBOOT_DEBUG("=================================\n");

    if (init_spi_flash() != TRUE)
    {
        UBOOT_ERROR("init_spi_flash error!\n");
        return -1;
    }

//================================ Generate Demura Bin ===============================

#if defined(CONFIG_DEMURA_URSA11)
    interface_info DataInfo[DEC_3];
#elif defined(CONFIG_DEMURA_URSA13)
    interface_info DataInfo;
#elif defined(CONFIG_DEMURA_M7622)
    interface_info DataInfo;
#elif defined(CONFIG_DEMURA_M7632)
    interface_info DataInfo;
#elif defined(CONFIG_DEMURA_MT5896)
    interface_info DataInfo;
#endif

    ret = demura_generate_bin_file((interface_info *)&DataInfo, pbin_info, multi_vendor);

    if (ret == FALSE)
    {
        UBOOT_ERROR("Decode_To_Mstar_Format error\n");
        return -1;
    }

    if (!init_backlight_demura_bin_info((interface_info *)&DataInfo, pbin_info))
    {
        return 0;
    }

    if ((pbin_info->HNode > HEX_FFFF) || (pbin_info->VNode > HEX_FFFF))
    {
        UBOOT_ERROR("HNode(%d)/VNode(%d) size over spec!\n", pbin_info->HNode, pbin_info->VNode);
        return -1;
    }

    if (mstar_demura_interface((interface_info *)&DataInfo, pbin_info) == FALSE)
    {
        UBOOT_ERROR("demura interface error\n");
        return -1;
    }

//================================ End Generate Demura Bin ===============================

    MS_U8  *pout = pbin_info->bin_buf;
    MS_U32 fsize = pbin_info->bin_size;

    printf("output_addr    = 0x%p\n", pout);
    printf("bin_size       = 0x%x\n", (uint)fsize);
    printf("dmalloc_length = 0x%x\n", get_dmalloc_length());

    // Enable SPI_FLASH - Write protect
    /*memset(cmd, 0, sizeof(cmd));
    snprintf(cmd, sizeof(cmd), "sf protect lock");
    if (run_command(cmd, 0) != 0)
    {
      printf("Enable SPI_FLASH Write protect Fail !\n");
    }
    memset(cmd, 0, sizeof(cmd));
    snprintf(cmd, sizeof(cmd), "sf protect status");
    run_command(cmd, 0);
    */
    printf("\n");
    return 0;
}
#else
int do_demura_convert(BinOutputInfo *pbin_info)
{
    MS_BOOL ret = TRUE;
    //char  cmd[256];

    if (check_str_resume() == 1)
    {
        //UBOOT_DEBUG("Can not covert demura file during STR\n");
        return 0;
    }

    UBOOT_DEBUG("==================================\n");
    UBOOT_DEBUG("DEMURA_URSA_TYPE : U%s\n", CONFIG_DEMURA_URSA_STRING);
    UBOOT_DEBUG("DEMURA_VENDOR    : %s\n",  CONFIG_DEMURA_VENDOR_STRING);
    UBOOT_DEBUG("==================================\n");

    if (init_spi_flash() != TRUE)
    {
        UBOOT_ERROR("init_spi_flash error!\n");
        return -1;
    }

//================================ Generate Demura Bin ===============================

#if defined(CONFIG_DEMURA_URSA11)
    interface_info DataInfo[DEC_3];
#elif defined(CONFIG_DEMURA_URSA13)
    interface_info DataInfo;
#elif defined(CONFIG_DEMURA_M7622)
    interface_info DataInfo;
#elif defined(CONFIG_DEMURA_M7632)
    interface_info DataInfo;
#elif defined(CONFIG_DEMURA_MT5896)
    interface_info DataInfo;
#else
    #error "Unkown DEMURA_URSA_TYPE !"
#endif

    memset(&DataInfo, 0, sizeof(DataInfo));

    // Ready to standard covert flow, Now comment .
    // Alloc_LutIn_Space(pdat_info, pbin_info)

    ret = Decode_To_Mstar_Format((void *)&DataInfo, pbin_info);
    if (ret == FALSE)
    {
        UBOOT_ERROR("Decode_To_Mstar_Format error\n");
        return -1;
    }

#if defined(CONFIG_DEMURA_VENDOR_BACKLIGHT)
#if defined(CONFIG_DEMURA_URSA11)
    memcpy(&(pbin_info->Info), DataInfo, sizeof(interface_info) * DEC_3);
#elif defined(CONFIG_DEMURA_URSA13)
    memcpy(&(pbin_info->Info), DataInfo, sizeof(interface_info));
#elif defined(CONFIG_DEMURA_M7622)
    memcpy(&(pbin_info->Info), DataInfo, sizeof(interface_info));
#elif defined(CONFIG_DEMURA_M7632)
    memcpy(&(pbin_info->Info), DataInfo, sizeof(interface_info));
#elif defined(CONFIG_DEMURA_MT5896)
    memcpy(&(pbin_info->Info), DataInfo, sizeof(interface_info));
#else
    #error "Unkown DEMURA_URSA_TYPE !"
#endif

    return 0;
#endif

    mstar_demura_interface((interface_info *)&DataInfo, pbin_info);

//================================ End Generate Demura Bin ===============================

    MS_U8  *pout = pbin_info->bin_buf;
    MS_U32 fsize = pbin_info->bin_size;

    printf("output_addr    = 0x%p\n", pout);
    printf("bin_size       = 0x%x\n", (uint)fsize);
    printf("dmalloc_length = 0x%x\n", get_dmalloc_length());

    // Enable SPI_FLASH - Write protect
    /*memset(cmd, 0, sizeof(cmd));
    snprintf(cmd, sizeof(cmd), "sf protect lock");
    if (run_command(cmd, 0) != 0)
    {
      printf("Enable SPI_FLASH Write protect Fail !\n");
    }
    memset(cmd, 0, sizeof(cmd));
    snprintf(cmd, sizeof(cmd), "sf protect status");
    run_command(cmd, 0);
    */
    printf("\n");
    return 0;
}
#endif

void demura_merge_by_pnl_bin_level(BinOutputInfo *bin_info, BinOutputInfo *bin_backlight_info, MS_U16 layer_bound)
{
    MS_U32 idx;
    MS_U16 bound = layer_bound;

    for (idx=0; idx < bin_backlight_info->LevelCount; idx++)
    {
        switch (idx)
        {
            case BL_LEVEL_CNT_0:
                UBOOT_DEBUG("plane_level[%d] panel demura=[0x%x], backlight demura=[0x%x]\n",
                    idx, bin_info->Info.reg_dmc_plane_level1, bin_backlight_info->Info.reg_dmc_plane_level1);
                if (ABS(bin_info->Info.reg_dmc_plane_level1 - bin_backlight_info->Info.reg_dmc_plane_level1) <= bound)
                {
                    strgb_structInfo_memaddsub(bin_info->Info.Lut_in[idx],
                        bin_backlight_info->Info.Lut_in[idx],
                        (bin_backlight_info->Info.reg_dmc_plane_level1 >> DEC_2),
                        (bin_info->HNode*bin_info->VNode));
                }
                break;
            case BL_LEVEL_CNT_1:
                UBOOT_DEBUG("plane_level[%d] panel demura=[0x%x], backlight demura=[0x%x]\n",
                    idx, bin_info->Info.reg_dmc_plane_level2, bin_backlight_info->Info.reg_dmc_plane_level2);
                if (ABS(bin_info->Info.reg_dmc_plane_level2 - bin_backlight_info->Info.reg_dmc_plane_level2) <= bound)
                {
                    strgb_structInfo_memaddsub(bin_info->Info.Lut_in[idx],
                        bin_backlight_info->Info.Lut_in[idx],
                        (bin_backlight_info->Info.reg_dmc_plane_level2 >> DEC_2),
                        (bin_info->HNode*bin_info->VNode));
                }
                break;
            case BL_LEVEL_CNT_2:
                UBOOT_DEBUG("plane_level[%d] panel demura=[0x%x], backlight demura=[0x%x]\n",
                    idx, bin_info->Info.reg_dmc_plane_level3, bin_backlight_info->Info.reg_dmc_plane_level3);
                if (ABS(bin_info->Info.reg_dmc_plane_level3 - bin_backlight_info->Info.reg_dmc_plane_level3) <= bound)
                {
                    strgb_structInfo_memaddsub(bin_info->Info.Lut_in[idx],
                        bin_backlight_info->Info.Lut_in[idx],
                        (bin_backlight_info->Info.reg_dmc_plane_level3 >> DEC_2),
                        (bin_info->HNode*bin_info->VNode));
                }
                break;
            case BL_LEVEL_CNT_3:
                UBOOT_DEBUG("plane_level[%d] panel demura=[0x%x], backlight demura=[0x%x]\n",
                    idx, bin_info->Info.reg_dmc_plane_level4, bin_backlight_info->Info.reg_dmc_plane_level4);
                if (ABS(bin_info->Info.reg_dmc_plane_level4 - bin_backlight_info->Info.reg_dmc_plane_level4) <= bound)
                {
                    strgb_structInfo_memaddsub(bin_info->Info.Lut_in[idx],
                        bin_backlight_info->Info.Lut_in[idx],
                        (bin_backlight_info->Info.reg_dmc_plane_level4 >> DEC_2),
                        (bin_info->HNode*bin_info->VNode));
                }
                break;
            case BL_LEVEL_CNT_4:
                UBOOT_DEBUG("plane_level[%d] panel demura=[0x%x], backlight demura=[0x%x]\n",
                    idx, bin_info->Info.reg_dmc_plane_level5, bin_backlight_info->Info.reg_dmc_plane_level5);
                if (ABS(bin_info->Info.reg_dmc_plane_level5 - bin_backlight_info->Info.reg_dmc_plane_level5) <= bound)
                {
                    strgb_structInfo_memaddsub(bin_info->Info.Lut_in[idx],
                        bin_backlight_info->Info.Lut_in[idx],
                        (bin_backlight_info->Info.reg_dmc_plane_level5 >> DEC_2),
                        (bin_info->HNode*bin_info->VNode));
                }
                break;
            case BL_LEVEL_CNT_5:
                UBOOT_DEBUG("plane_level[%d] panel demura=[0x%x], backlight demura=[0x%x]\n",
                    idx, bin_info->Info.reg_dmc_plane_level6, bin_backlight_info->Info.reg_dmc_plane_level6);
                if (ABS(bin_info->Info.reg_dmc_plane_level6 - bin_backlight_info->Info.reg_dmc_plane_level6) <= bound)
                {
                    strgb_structInfo_memaddsub(bin_info->Info.Lut_in[idx],
                        bin_backlight_info->Info.Lut_in[idx],
                        (bin_backlight_info->Info.reg_dmc_plane_level6 >> DEC_2),
                        (bin_info->HNode*bin_info->VNode));
                }
                break;
            case BL_LEVEL_CNT_6:
                UBOOT_DEBUG("plane_level[%d] panel demura=[0x%x], backlight demura=[0x%x]\n",
                    idx, bin_info->Info.reg_dmc_plane_level7, bin_backlight_info->Info.reg_dmc_plane_level7);
                if (ABS(bin_info->Info.reg_dmc_plane_level7 - bin_backlight_info->Info.reg_dmc_plane_level7) <= bound)
                {
                    strgb_structInfo_memaddsub(bin_info->Info.Lut_in[idx],
                        bin_backlight_info->Info.Lut_in[idx],
                        (bin_backlight_info->Info.reg_dmc_plane_level7 >> DEC_2),
                        (bin_info->HNode*bin_info->VNode));
                }
                break;
            case BL_LEVEL_CNT_7:
                UBOOT_DEBUG("plane_level[%d] panel demura=[0x%x], backlight demura=[0x%x]\n",
                    idx, bin_info->Info.reg_dmc_plane_level8, bin_backlight_info->Info.reg_dmc_plane_level8);
                if (ABS(bin_info->Info.reg_dmc_plane_level8 - bin_backlight_info->Info.reg_dmc_plane_level8) <= bound)
                {
                    strgb_structInfo_memaddsub(bin_info->Info.Lut_in[idx],
                        bin_backlight_info->Info.Lut_in[idx],
                        (bin_backlight_info->Info.reg_dmc_plane_level8 >> DEC_2),
                        (bin_info->HNode*bin_info->VNode));
                }
                break;
            default:
                break;
        }
        if (bin_backlight_info->Info.Lut_in[idx] != NULL)
        {
            dfree(bin_backlight_info->Info.Lut_in[idx]);
            bin_backlight_info->Info.Lut_in[idx] = NULL;
        }
    }
}

void demura_merge_by_backlight_bin_level(BinOutputInfo *bin_info, BinOutputInfo *bin_backlight_info, MS_U16 layer_bound)
{
    MS_U32 idx;
    MS_U16 bound = layer_bound;

    for (idx = 0; idx < bin_info->LevelCount; idx++)
    {
        switch (idx)
        {
            case BL_LEVEL_CNT_0:
                UBOOT_DEBUG("plane_level[%d] backlight demura=[0x%x], panel demura=[0x%x]\n",
                    idx, bin_backlight_info->Info.reg_dmc_plane_level1, bin_info->Info.reg_dmc_plane_level1);
                if (ABS(bin_backlight_info->Info.reg_dmc_plane_level1 - bin_info->Info.reg_dmc_plane_level1) <= bound)
                {
                    bin_backlight_info->Info.reg_dmc_plane_level1 = bin_info->Info.reg_dmc_plane_level1;
                    strgb_structInfo_memaddsub(bin_backlight_info->Info.Lut_in[idx],
                        bin_info->Info.Lut_in[idx],
                        (bin_info->Info.reg_dmc_plane_level1 >> DEC_2),
                        (bin_info->HNode*bin_info->VNode));
                }
                break;
            case BL_LEVEL_CNT_1:
                UBOOT_DEBUG("plane_level[%d] backlight demura=[0x%x], panel demura=[0x%x]\n",
                    idx, bin_backlight_info->Info.reg_dmc_plane_level2, bin_info->Info.reg_dmc_plane_level2);
                if (ABS(bin_backlight_info->Info.reg_dmc_plane_level2 - bin_info->Info.reg_dmc_plane_level2) <= bound)
                {
                    bin_backlight_info->Info.reg_dmc_plane_level2 = bin_info->Info.reg_dmc_plane_level2;
                    strgb_structInfo_memaddsub(bin_backlight_info->Info.Lut_in[idx],
                        bin_info->Info.Lut_in[idx],
                        (bin_info->Info.reg_dmc_plane_level2 >> DEC_2),
                        (bin_info->HNode*bin_info->VNode));
                }
                break;
            case BL_LEVEL_CNT_2:
                UBOOT_DEBUG("plane_level[%d] backlight demura=[0x%x], panel demura=[0x%x]\n",
                    idx, bin_backlight_info->Info.reg_dmc_plane_level3, bin_info->Info.reg_dmc_plane_level3);
                if (ABS(bin_backlight_info->Info.reg_dmc_plane_level3 - bin_info->Info.reg_dmc_plane_level3) <= bound)
                {
                    bin_backlight_info->Info.reg_dmc_plane_level3 = bin_info->Info.reg_dmc_plane_level3;
                    strgb_structInfo_memaddsub(bin_backlight_info->Info.Lut_in[idx],
                        bin_info->Info.Lut_in[idx],
                        (bin_info->Info.reg_dmc_plane_level3 >> DEC_2),
                        (bin_info->HNode*bin_info->VNode));
                }
                break;
            case BL_LEVEL_CNT_3:
                UBOOT_DEBUG("plane_level[%d] backlight demura=[0x%x], panel demura=[0x%x]\n",
                    idx, bin_backlight_info->Info.reg_dmc_plane_level4, bin_info->Info.reg_dmc_plane_level4);
                if (ABS(bin_backlight_info->Info.reg_dmc_plane_level4 - bin_info->Info.reg_dmc_plane_level4) <= bound)
                {
                    bin_backlight_info->Info.reg_dmc_plane_level4 = bin_info->Info.reg_dmc_plane_level4;
                    strgb_structInfo_memaddsub(bin_backlight_info->Info.Lut_in[idx],
                        bin_info->Info.Lut_in[idx],
                        (bin_info->Info.reg_dmc_plane_level4 >> DEC_2),
                        (bin_info->HNode*bin_info->VNode));
                }
                break;
            case BL_LEVEL_CNT_4:
                UBOOT_DEBUG("plane_level[%d] backlight demura=[0x%x], panel demura=[0x%x]\n",
                    idx, bin_backlight_info->Info.reg_dmc_plane_level5, bin_info->Info.reg_dmc_plane_level5);
                if (ABS(bin_backlight_info->Info.reg_dmc_plane_level5 - bin_info->Info.reg_dmc_plane_level5) <= bound)
                {
                    bin_backlight_info->Info.reg_dmc_plane_level5 = bin_info->Info.reg_dmc_plane_level5;
                    strgb_structInfo_memaddsub(bin_backlight_info->Info.Lut_in[idx],
                        bin_info->Info.Lut_in[idx],
                        (bin_info->Info.reg_dmc_plane_level5 >> DEC_2),
                        (bin_info->HNode*bin_info->VNode));
                }
                break;
            case BL_LEVEL_CNT_5:
                UBOOT_DEBUG("plane_level[%d] backlight demura=[0x%x], panel demura=[0x%x]\n",
                    idx, bin_backlight_info->Info.reg_dmc_plane_level6, bin_info->Info.reg_dmc_plane_level6);
                if (ABS(bin_backlight_info->Info.reg_dmc_plane_level6 - bin_info->Info.reg_dmc_plane_level6) <= bound)
                {
                    bin_backlight_info->Info.reg_dmc_plane_level6 = bin_info->Info.reg_dmc_plane_level6;
                    strgb_structInfo_memaddsub(bin_backlight_info->Info.Lut_in[idx],
                        bin_info->Info.Lut_in[idx],
                        (bin_info->Info.reg_dmc_plane_level6 >> DEC_2),
                        (bin_info->HNode*bin_info->VNode));
                }
                break;
            case BL_LEVEL_CNT_6:
                UBOOT_DEBUG("plane_level[%d] backlight demura=[0x%x], panel demura=[0x%x]\n",
                    idx, bin_backlight_info->Info.reg_dmc_plane_level7, bin_info->Info.reg_dmc_plane_level7);
                if (ABS(bin_backlight_info->Info.reg_dmc_plane_level7 - bin_info->Info.reg_dmc_plane_level7) <= bound)
                {
                    bin_backlight_info->Info.reg_dmc_plane_level7 = bin_info->Info.reg_dmc_plane_level7;
                    strgb_structInfo_memaddsub(bin_backlight_info->Info.Lut_in[idx],
                        bin_info->Info.Lut_in[idx],
                        (bin_info->Info.reg_dmc_plane_level7 >> DEC_2),
                        (bin_info->HNode*bin_info->VNode));
                }
                break;
            case BL_LEVEL_CNT_7:
                UBOOT_DEBUG("plane_level[%d] backlight demura=[0x%x], panel demura=[0x%x]\n",
                    idx, bin_backlight_info->Info.reg_dmc_plane_level8, bin_info->Info.reg_dmc_plane_level8);
                if (ABS(bin_backlight_info->Info.reg_dmc_plane_level8 == bin_info->Info.reg_dmc_plane_level8) <= bound)
                {
                    bin_backlight_info->Info.reg_dmc_plane_level8 = bin_info->Info.reg_dmc_plane_level8;
                    strgb_structInfo_memaddsub(bin_backlight_info->Info.Lut_in[idx],
                        bin_info->Info.Lut_in[idx],
                        (bin_info->Info.reg_dmc_plane_level8 >> DEC_2),
                        (bin_info->HNode*bin_info->VNode));
                }
                break;
            default:
                break;
        }
        if (bin_info->Info.Lut_in[idx] != NULL)
        {
            dfree(bin_info->Info.Lut_in[idx]);
            bin_info->Info.Lut_in[idx] = NULL;
        }
    }
}

int do_demura_merge(BinOutputInfo *bin_info, BinOutputInfo *bin_backlight_info, MS_U16 layer_bound)
{
    if (bin_info->Info.reg_dmc_rgb_mode != bin_backlight_info->Info.reg_dmc_rgb_mode)
    {
        UBOOT_DEBUG("mode is not matched. panel/bl demura is [%s][%s]\n", bin_info->Info.reg_dmc_rgb_mode ? "rgb" : "mono", \
                                                                          bin_backlight_info->Info.reg_dmc_rgb_mode ? "rgb" : "mono");
        return -1;
    }
    if (bin_info->HNode == bin_backlight_info->HNode && bin_info->VNode == bin_backlight_info->VNode)
    {
        UBOOT_DEBUG("LevelCount panel demura=[%d], backlight demura=[%d]\n", bin_info->LevelCount, bin_backlight_info->LevelCount);
        if (bin_info->LevelCount >= bin_backlight_info->LevelCount)
        {
            demura_merge_by_pnl_bin_level(bin_info, bin_backlight_info, layer_bound);
            //dfree(bin_backlight_info->bin_buf);
        }
        else
        {
            demura_merge_by_backlight_bin_level(bin_info, bin_backlight_info, layer_bound);
            //dfree(bin_info->bin_buf);
            memcpy(bin_info, bin_backlight_info, sizeof(BinOutputInfo));
        }
    }
    else
    {
        UBOOT_DEBUG("block size is not matched. panel demura HVnode[%d][%d], bl demura HVnode[%d][%d]\n", bin_info->HNode, bin_info->VNode, bin_backlight_info->HNode, bin_backlight_info->VNode);
        return -1;
    }
    return 0;
}


void set_demura_version(int version)
{
    demura_set_version(version);
}

void set_demura_pnl(MS_U16 width, MS_U16 height)
{
    demura_set_panelinfo(width, height);
}
