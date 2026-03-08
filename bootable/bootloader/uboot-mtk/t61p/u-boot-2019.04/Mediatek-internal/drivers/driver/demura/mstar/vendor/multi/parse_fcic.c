#include <command.h>
#include <common.h>
#include <stdio.h>
#include <linux/string.h>
#include <MsTypes.h>
#include <debug_impl.h>
#include <dmalloc.h>
#include <ms_utils.h>
#include <ms_array.h>
#include <demura.h>
#include <crc_libs.h>
#include <demura_config.h>
#include <CommonDataType.h>
#include <convert_entry.h>

#include <apiPNL.h>
#include <MDrvDemura.h>
#include <halDemura.h>
#include "vendor.h"
#include "demura_config.h"
#include "environment.h"

#undef ALIGN
#define ALIGN(x,a)        __ALIGN_MASK((x),(typeof(x))(a)-1)
#define __ALIGN_MASK(x,mask)    (((x)+(mask))&~(mask))

#define minmax(v,a,b) (((v)<(a)) ? (a) : ((v)>(b)) ? (b) : (v))

void CUC_cvt_blk_size_add_CIC(int ****data, EN_DEMURA_MULTI_VENDOR multi_vendor)
{
if ((multi_vendor == EN_DEMURA_MULTI_LGD_SQ18_19_COLOR) || (multi_vendor == EN_DEMURA_MULTI_LGD_SQ18_19_MONO))
{
    // CUC(R,G,B) block size conversion by linear interpolation, and add CIC at last
    int i,j,c,t;
    for (t = 0; t < 4; t++)
    {
        for (c = 0; c < 3; c++)
        {
            // Horizontal Color 32x32 -> 8x32 linear interpolation
            for (j = 0; j < 69; j++)
            {
                int tmp[481] = { 0 };
                for (i = 0; i < 481; i++)
                {
                    int dist_x_0 = (i % 4);
                    int dist_x_1 = 4 - dist_x_0;
                    tmp[i] = (data[j][i / 4][c][t] * dist_x_1 + data[j][minmax(i / 4 + 1, 0, 120)][c][t] * dist_x_0 + 2) >> 2;

                }
                for (i = 0; i < 481; i++)
                {
                    data[j][i][c][t] = tmp[i];
                }
            }

            // Vertical Color 8x32 -> 8x8 linear interpolation
            // And add comp_W to each comp_C (for C = R,G,B)
            for (i = 0; i < 481; i++)
            {
                int tmp[271] = { 0 };
                for (j = 0; j < 271; j++)
                {
                    int dist_y_0 = (j % 4);
                    int dist_y_1 = 4 - dist_y_0;
                    tmp[j] = (data[j / 4][i][c][t] * dist_y_1 + data[minmax(j / 4 + 1, 0, 67)][i][c][t] * dist_y_0 + 2) >> 2;

                }
                for (j = 0; j < 271; j++)
                {
                    data[j][i][c][t] = minmax(tmp[j] + data[j][i][3][t], -128, 127);
                }
            }
        }
    }
}
else if ((multi_vendor == EN_DEMURA_MULTI_LGD_V19_COLOR) || (multi_vendor == EN_DEMURA_MULTI_LGD_V19_MONO))
{
    // CUC(R,G,B) block size conversion by linear interpolation, and add CIC at last
    int i, j, c, t;

    for (t = 0; t < 4; t++)
    {
        for (c = 0; c < 3; c++)
        {
            // Horizontal Color 32x32 -> 8x32 linear interpolation
            for (j = 0; j < 69; j++)
            {
                int tmp[241] = { 0 };
                for (i = 0; i < 241; i++)
                {
                    int dist_x_0 = (i % 2);
                    int dist_x_1 = 2 - dist_x_0;
                    tmp[i] = (data[j][i / 2][c][t] * dist_x_1 + data[j][minmax(i / 2 + 1, 0, 120)][c][t] * dist_x_0 + 1) / 2;

                }
                for (i = 0; i < 241; i++)
                {
                    data[j][i][c][t] = tmp[i];
                }
            }

            // Vertical Color 8x32 -> 8x8 linear interpolation
            // And add comp_W to each comp_C (for C = R,G,B)
            for (i = 0; i < 241; i++)
            {
                int tmp[136] = { 0 };
                for (j = 0; j < 136; j++)
                {
                    int dist_y_0 = (j % 2);
                    int dist_y_1 = 2 - dist_y_0;
                    tmp[j] = (data[j / 2][i][c][t] * dist_y_1 + data[minmax(j / 2 + 1, 0, 67)][i][c][t] * dist_y_0 + 1) / 2;

                }
                for (j = 0; j < 136; j++)
                {
                    data[j][i][c][t] = minmax(tmp[j] + data[j][i][3][t], -128, 127);
                }
            }
        }
    }
}
}

MS_BOOL Load_LGD_Bin(char **pbuf, int flash_start_addr, int bin_size)
{
    MS_BOOL  bRet;
    char *buffer;
    MS_U32 fsize;

    UBOOT_TRACE("IN\n");
    int start   = flash_start_addr;

    fsize = ALIGN(bin_size, 32);
    buffer = (char *)dmalloc(fsize + 32);
    CHECK_DMALLOC_SPACE(buffer, (uint)fsize + 32);
    memset(buffer, 0, sizeof(buffer));  //coverity issue

    // read binary file
    bRet = read_spi_flash((MS_U8 *)buffer, start, fsize);  // optimize,
    if (bRet == FALSE)
    {
        UBOOT_ERROR("read LGD Demura binary error\n");
        dfree(buffer);
        return FALSE;
    }

	int i;
	for(i=0;i<10;i++)
	{
		printf("%2x ",*(buffer+i));
	}
	printf("\n");
    *pbuf = buffer;
    UBOOT_TRACE("OK\n");
    return TRUE;
}
#if defined (CONFIG_DEMURA_URSA11)
#else
static void LG_Set_U13_Format_Init_Cal(
    EN_DEMURA_MULTI_VENDOR multi_vendor,
    int *start_addr,
    int *data_str_addr,
    int *chroma_4tab_addr,
    int *empty_dummy_addr,
    int *gain_offset_addr,
    int *checksum_addr,
    int *h_size,
    int *offset_addr)
{
    if ((multi_vendor == EN_DEMURA_MULTI_LGD_V19_COLOR) || (multi_vendor == EN_DEMURA_MULTI_LGD_V19_MONO))
    {
        *start_addr = 0x80000; // must be times of 32 (0x20)
        *data_str_addr = 0x4;
        *chroma_4tab_addr = 0x085E4;
        //empty_dummy_addr = 0xA1A30;
        *gain_offset_addr = 0x21A30;
        *checksum_addr = 0x21A3C;

        *h_size = 252;
        *offset_addr = *gain_offset_addr;
    }
    else if ((multi_vendor == EN_DEMURA_MULTI_LGD_SQ18_19_COLOR) || (multi_vendor == EN_DEMURA_MULTI_LGD_SQ18_19_MONO))
    {
        *start_addr = 0x0; // must be times of 32 (0x20)
        *data_str_addr = 0x4;
        *chroma_4tab_addr = 0x208d8;
        *empty_dummy_addr = 0x39d24;
        *gain_offset_addr = 0x5c304;
        *checksum_addr = 0x5c310;

        *h_size = 492;
        *offset_addr = *empty_dummy_addr;
    }
    else if (multi_vendor == EN_DEMURA_MULTI_LGD_V18)
    {
        *start_addr = 0x60000;        // must be times of 32 (0x20)
        *data_str_addr = 0x00004;     // 0x60004 , For optimize
        *gain_offset_addr = 0x085e4;  // 0x685e4
        *checksum_addr = 0x085f0;     // 0x685f0
    }
}

MS_BOOL LG_Set_U13_Format(int n_Hnode, int n_Vnode, interface_info *info, EN_DEMURA_MULTI_VENDOR multi_vendor)
{
    // Create data array
    int **** data;
    int fcic_gain[4]={0};
    int fcic_ofst[4]={0};

    int start_addr = 0; // must be times of 32 (0x20)
    int data_str_addr = 0;
    int chroma_4tab_addr = 0;
    int empty_dummy_addr = 0;
    int gain_offset_addr = 0;
    int checksum_addr = 0;
    char *fbuf = NULL;
    int h_size = 0;
    int offset_addr = 0;
    MS_BOOL bRet;
    struct array_dim4 _data;
    if(DEFINE_ARRAY_DIM4(_data, sizeof(int), 271, 492, 4, 4)!=TRUE)
    {
        return FALSE;
    }

    data = (int ****)PTR_ARRAY_DIM4(_data);

    UBOOT_TRACE("IN\n");;


    LG_Set_U13_Format_Init_Cal(multi_vendor, &start_addr, &data_str_addr,
                                &chroma_4tab_addr, &empty_dummy_addr,
                                &gain_offset_addr, &checksum_addr,
                                &h_size, &offset_addr);

    bRet = Load_LGD_Bin(&fbuf, start_addr, (checksum_addr + 16));
    if (bRet == FALSE || fbuf == NULL)
    {
        UBOOT_ERROR("Load_LGD_Bin Error\n");
        FREE_ARRAY_DIM4(_data);
        return FALSE;
    }

    //! Start Parsing
    int byte_address = 0;
    MS_BOOL end_flag = FALSE;
    int check_sum = 0;
    int total_sum = 0;
    int h_cnt = 0, v_cnt = 0, c_cnt = 0, t_cnt = 0;
    int i, j, t, c;


    while (end_flag != TRUE)
    {
        int i;
        char *line = NULL;  // char line[32];
        line = fbuf + byte_address;

        for (i = 0; i < 32; i++) // Parse 32 bytes in one time
        {
            if (byte_address + i < data_str_addr) // Skip bulk detect bytes
                 continue;

            //! End after get Checksum byte
            if (byte_address + i >= checksum_addr)
            {
                check_sum = line[i] & 0xff;
                end_flag = TRUE;
                break;
            }

            /*if ( (multi_vendor == EN_DEMURA_MULTI_LGD_V19_COLOR) || (multi_vendor == EN_DEMURA_MULTI_LGD_V19_MONO) ||
             (multi_vendor == EN_DEMURA_MULTI_LGD_SQ18_19_COLOR) || (multi_vendor == EN_DEMURA_MULTI_LGD_SQ18_19_MONO) )*/

            if ( (multi_vendor <= EN_DEMURA_MULTI_LGD_V19_COLOR) && (multi_vendor >= EN_DEMURA_MULTI_LGD_SQ18_19_MONO))
            {
                if (byte_address + i >= gain_offset_addr) // 12 bytes Gain & Offset
                {
                    if (byte_address + i == gain_offset_addr)
                        fcic_gain[2] = line[i] & 0xff;
                    else if (byte_address + i == gain_offset_addr + 1)
                        fcic_gain[1] = line[i] & 0xff;
                    else if (byte_address + i == gain_offset_addr + 2)
                        fcic_gain[0] = line[i] & 0xff;
                    else if (byte_address + i == gain_offset_addr + 4)
                        fcic_ofst[0] = line[i] & 0xff;
                    else if (byte_address + i == gain_offset_addr + 7)
                        fcic_gain[3] = line[i] & 0xff;
                    else if (byte_address + i == gain_offset_addr + 9)
                        fcic_ofst[3] = line[i] & 0xff;
                    else if (byte_address + i == gain_offset_addr + 10)
                        fcic_ofst[2] = line[i] & 0xff;
                    else if (byte_address + i == gain_offset_addr + 11)
                        fcic_ofst[1] = line[i] & 0xff;
                }
                else if (byte_address + i < chroma_4tab_addr) // luma (3:W, 1:Gray64)
                {
                    data[v_cnt][h_cnt / 4 * 4 + (3 - i % 4)][3][1] = line[i];
                    //! Add index of data matrix
                    h_cnt++;
                    if (h_cnt == h_size)
                    {
                        h_cnt = 0;
                        v_cnt++;
                    }
                }
                else if (byte_address + i < offset_addr) // chroma (0~2:RGB)
                {
                    if (byte_address + i == chroma_4tab_addr)
                    {
                        UBOOT_DEBUG("Start parsing chroma, reset counters\n");
                        v_cnt = 0;
                        h_cnt = 0;
                        c_cnt = 0;
                        t_cnt = 0;
                    }

                    data[v_cnt][h_cnt][c_cnt][t_cnt] = line[i];

                    //! Add index of data matrix
                    t_cnt++;
                    if (t_cnt == 4)
                    {
                        t_cnt = 0;
                        h_cnt++;
                        if (h_cnt == 125)
                        {
                            h_cnt = 0;
                            v_cnt++;
                            if (v_cnt == 69)
                            {
                                v_cnt = 0;
                                c_cnt++;
                            }
                         }
                    }
                }
                //! (total_sum % 256) to verify check_sum
                total_sum += line[i];
                total_sum %= 256;
            }
            else if (multi_vendor == EN_DEMURA_MULTI_LGD_V18)
            {
                if (byte_address + i >= gain_offset_addr) // 12 bytes Gain & Offset
                {
                    if (byte_address + i == gain_offset_addr)
                        fcic_gain[2] = line[i] & 0xff;
                    else if (byte_address + i == gain_offset_addr + 1)
                        fcic_gain[1] = line[i] & 0xff;
                    else if (byte_address + i == gain_offset_addr + 2)
                        fcic_gain[0] = line[i] & 0xff;
                    else if (byte_address + i == gain_offset_addr + 4)
                        fcic_ofst[0] = line[i] & 0xff;
                    else if (byte_address + i == gain_offset_addr + 7)
                        fcic_gain[3] = line[i] & 0xff;
                    else if (byte_address + i == gain_offset_addr + 9)
                        fcic_ofst[3] = line[i] & 0xff;
                    else if (byte_address + i == gain_offset_addr + 10)
                        fcic_ofst[2] = line[i] & 0xff;
                    else if (byte_address + i == gain_offset_addr + 11)
                        fcic_ofst[1] = line[i] & 0xff;
                }
                else
                {
                    data[v_cnt][h_cnt / 4 * 4 + (3 - i % 4)][3][1] = line[i];
                    //! Add index of data matrix
                    h_cnt++;
                    if (h_cnt == 252)
                    {
                        h_cnt = 0;
                        v_cnt++;
                    }
                }
                //! (total_sum % 256) to verify check_sum
                total_sum += (int)(line[i]);
                total_sum %= 256;
            }
        }
        byte_address += 32;
    }

    UBOOT_DEBUG("Checksum = %x\n", check_sum);
    UBOOT_DEBUG("2's complement(total_sum %% 256) = %x\n", ((-1 * total_sum) & 0xff));

    if (((-1 * total_sum) & 0xff) == check_sum)
    {
        UBOOT_DEBUG("\nChecksum PASS !!!\n");
    }
    else
    {
        UBOOT_ERROR("\nChecksum FAIL !!!\n");
        FREE_ARRAY_DIM4(_data);
        dfree(fbuf);
        return FALSE;
    }

    // Sign spi flash data
    MS_U32 sample_crc32;
    char  strbuf[1024];

    sample_crc32 = MDrv_CRC32_Cal_DeMura((MS_U8 *)(fbuf+SAMPLE_DAT_START), SAMPLE_DAT_LEN);

    if (sample_crc32 == 0xffffFFFF)
    {
        UBOOT_ERROR("MDrv_CRC32_Cal_DeMura error\n");
    }
    memset(strbuf, 0, sizeof(strbuf));
    if (snprintf(strbuf, sizeof(strbuf)-1, "0x%x:0x%x", (unsigned int)check_sum, (unsigned int)sample_crc32) <= 1024)
    {
        env_set(ENV_DEMURA_SIG, strbuf);
    }
#if CONFIG_DEMURA_ENV_SAVEENV
    env_save();
#endif
    // End sign spi flash data


    UBOOT_DEBUG("Tab : gain = %d, ofst = %d\n", fcic_gain[0], fcic_ofst[0]);
    UBOOT_DEBUG("Tab : gain = %d, ofst = %d\n", fcic_gain[1], fcic_ofst[1]);
    UBOOT_DEBUG("Tab : gain = %d, ofst = %d\n", fcic_gain[2], fcic_ofst[2]);
    UBOOT_DEBUG("Tab : gain = %d, ofst = %d\n", fcic_gain[3], fcic_ofst[3]);
    //! The data provided by LGD is 1's complement
    //! Convert 1's complement to 2's complement
    //! For Full&Simple FCIC, firstly convert G64 where the 1-layer data provided for "W" channel only

    for (j = 0; j < n_Vnode; j++)
    {
        for (i = 0; i < n_Hnode; i++)
        {
            int sign = 0;
            int value = data[j][i][3][1] & 0x7f;
            //value = (value) / 2; // s6.1 to s7(s0_6.0)
            if ((data[j][i][3][1] & 0x80) == 0x80)
                sign = -1;
            else
                sign = 1;

            data[j][i][3][1] = sign * value;
        }
    }

    for (c = 0; c < 4; c++)
    {
        for (t = 0; t < 4; t++)
        {
            for (j = 0; j < n_Vnode; j++)
            {
                for (i = 0; i < n_Hnode; i++)
                {
                    int sign = 0;
                    int value = data[j][i][c][t] & 0x7f;
                    if ((data[j][i][c][t] & 0x80) == 0x80)
                        sign = -1;
                    else
                        sign = 1;

                    //! For Full&Simple FCIC, calculate G64 with gain&offset to generate the other tabs for "W" channel only
                    if (c == 3)
                    {
                        //test if (t != 1) // Skip 64 layer for gain & offset zero pattern
                        {
                            int final_out = ((int)(data[j][i][c][1]) * fcic_gain[t]) / 64 + fcic_ofst[t];
                            if (final_out < -128 || final_out > 127)
                            {
                                UBOOT_DEBUG("FCIC: Overflow according to data with the gains & offsets\n");
                                //FREE_ARRAY_DIM4(_data);
                                //dfree(fbuf);
                                //return FALSE;
                            }
                            data[j][i][c][t] = final_out;
                        }
                    }
                    else
                    {
                        data[j][i][c][t] = sign * value;
                    }
                }
            }
        }
    }

    ///////////////////////////////////////////////////////////////////////////////////////
    // Step 3:    Convert block size of RGB from 32x32 to 8x8 or 16x16
    //            Add CIC to CUC:
    //            ->    comp_R = comp_R + comp_Luma
    //            ->    comp_G = comp_G + comp_Luma
    //            ->    comp_B = comp_B + comp_Luma
    //            ->    comp_W = comp_Luma
    ///////////////////////////////////////////////////////////////////////////////////////
    if ((multi_vendor == EN_DEMURA_MULTI_LGD_V19_COLOR) || (multi_vendor == EN_DEMURA_MULTI_LGD_SQ18_19_COLOR))
    {
        printf("CONFIG_DEMURA_VENDOR_LGD_CIC_CUC\n");

        CUC_cvt_blk_size_add_CIC(data, multi_vendor);
    }
    else
        printf("CONFIG_DEMURA_VENDOR_LGD_CIC_ONLY \n");

    int nLayer = 0;
    for (t = 0; t < 4; t++)
    {
        //get level
        switch (t)
        {
            case 0:
                nLayer = info->reg_dmc_plane_level1 >> 2;
                break;
            case 1:
                nLayer = info->reg_dmc_plane_level2 >> 2;
                break;
            case 2:
                nLayer = info->reg_dmc_plane_level3 >> 2;
                break;
            case 3:
                nLayer = info->reg_dmc_plane_level4 >> 2;
                break;
        }

        for (j = 0; j < n_Vnode; j++)
        {
            for (i = 0; i < n_Hnode; i++)
            {
            if ((multi_vendor == EN_DEMURA_MULTI_LGD_V19_COLOR) || (multi_vendor == EN_DEMURA_MULTI_LGD_SQ18_19_COLOR))
            {
                    info->Lut_in[t][j * n_Hnode + i].dbr = (double)(data[j][i][0][t] >> 1) + nLayer;
                    info->Lut_in[t][j * n_Hnode + i].r   = (data[j][i][0][t] >> 1) + nLayer;
                    info->Lut_in[t][j * n_Hnode + i].dbg = (double)(data[j][i][1][t] >> 1) + nLayer;
                    info->Lut_in[t][j * n_Hnode + i].g   = (data[j][i][1][t] >> 1) + nLayer;
                    info->Lut_in[t][j * n_Hnode + i].dbb = (double)(data[j][i][2][t] >> 1) + nLayer;
                    info->Lut_in[t][j * n_Hnode + i].b   = (data[j][i][2][t] >> 1) + nLayer;
                #if (defined (CONFIG_DEMURA_M7622) || defined (CONFIG_DEMURA_M7632))
                    info->Lut_in[t][j * n_Hnode + i].dbw = (double)(data[j][i][3][t] >> 1) + nLayer;
                    info->Lut_in[t][j * n_Hnode + i].w   = (data[j][i][3][t] >> 1) + nLayer;
                #endif
            }
            else
            {
                info->Lut_in[t][j * n_Hnode + i].dbr = (double)(data[j][i][3][t] >> 1) + nLayer;
                info->Lut_in[t][j * n_Hnode + i].r   = (data[j][i][3][t] >> 1) + nLayer;
                info->Lut_in[t][j * n_Hnode + i].dbg = (double)(data[j][i][3][t] >> 1) + nLayer;
                info->Lut_in[t][j * n_Hnode + i].g   = (data[j][i][3][t] >> 1) + nLayer;
                info->Lut_in[t][j * n_Hnode + i].dbb = (double)(data[j][i][3][t] >> 1) + nLayer;
                info->Lut_in[t][j * n_Hnode + i].b   = (data[j][i][3][t] >> 1) + nLayer;
                #if (defined (CONFIG_DEMURA_M7622) || defined (CONFIG_DEMURA_M7632))
                    info->Lut_in[t][j * n_Hnode + i].dbw = (double)(data[j][i][3][t] >> 1) + nLayer;
                    info->Lut_in[t][j * n_Hnode + i].w   = (data[j][i][3][t] >> 1) + nLayer;
                #endif
            }
            }
        }
    }

    FREE_ARRAY_DIM4(_data);
    dfree(fbuf);
    UBOOT_TRACE("OK\n");
    return TRUE;
}
#endif
MS_BOOL parse_LG_bin_1tab(int n_Hnode, int n_Vnode, interface_info *info, EN_DEMURA_MULTI_VENDOR multi_vendor)
{
    MS_BOOL bRet = FALSE;

    #if defined (CONFIG_DEMURA_URSA11)
        //bRet = LG_Set_U11_Format(phdr, pinfo);
    #else
        bRet = LG_Set_U13_Format(n_Hnode, n_Vnode,info,multi_vendor);
    #endif

    return bRet;
}

MS_BOOL transfer_LGD_Data_1tab(void *pdat_info, BinOutputInfo *pbin_info, EN_DEMURA_MULTI_VENDOR multi_vendor)
{
#if defined (CONFIG_DEMURA_URSA13)
    interface_info *pDataInfo = (interface_info *)pdat_info;
#elif (defined (CONFIG_DEMURA_M7622) || defined (CONFIG_DEMURA_M7632) || defined (CONFIG_DEMURA_MT5896))
    interface_info *pDataInfo = (interface_info *)pdat_info;
#else
//#error "Unkown DEMURA_URSA_TYPE !"
#endif


#if defined (CONFIG_DEMURA_URSA11)
#else
int n_Hnode=0, n_Vnode=0;
if ((multi_vendor == EN_DEMURA_MULTI_LGD_V19_COLOR) || (multi_vendor == EN_DEMURA_MULTI_LGD_V19_MONO))
{
        n_Hnode = 241;
        n_Vnode = 136;
        pDataInfo->reg_dmc_h_block            = 0x4;         //  3 bit
        pDataInfo->reg_dmc_v_block            = 0x4;         //  3 bit
}
else if ((multi_vendor == EN_DEMURA_MULTI_LGD_SQ18_19_COLOR) || (multi_vendor == EN_DEMURA_MULTI_LGD_SQ18_19_MONO))
{
        n_Hnode = 481;
        n_Vnode = 271;
        pDataInfo->reg_dmc_h_block            = 0x3;         //  3 bit
        pDataInfo->reg_dmc_v_block            = 0x3;         //  3 bit
}
else if (multi_vendor == EN_DEMURA_MULTI_LGD_V18)
{
        n_Hnode = 241;
        n_Vnode = 136;
        pDataInfo->reg_dmc_h_block            = 0x4;         //  3 bit
        pDataInfo->reg_dmc_v_block            = 0x4;         //  3 bit
}

    pDataInfo->reg_dmc_plane_num              = 0x4;         //  4 bit

    if ((multi_vendor == EN_DEMURA_MULTI_LGD_V19_COLOR) || (multi_vendor == EN_DEMURA_MULTI_LGD_SQ18_19_COLOR))
        pDataInfo->reg_dmc_rgb_mode               = 0x1;         //  1 bit
    else
        pDataInfo->reg_dmc_rgb_mode               = 0x0;         //  1 bit

    pDataInfo->reg_dmc_panel_h_size           = 0xF00;       // 13 bit
    pDataInfo->reg_dmc_black_limit            = 0x000;       // 12 bit , format 10.2
    pDataInfo->reg_dmc_plane_level1           = 0x200;       // 12 bit , format 10.2
    pDataInfo->reg_dmc_plane_level2           = 0x400;       // 12 bit , format 10.2
    pDataInfo->reg_dmc_plane_level3           = 0x800;       // 12 bit , format 10.2
    pDataInfo->reg_dmc_plane_level4           = 0xC00;       // 12 bit , format 10.2
    pDataInfo->reg_dmc_plane_level5           = 0;           // 12 bit , format 10.2
    pDataInfo->reg_dmc_plane_level6           = 0;           // 12 bit , format 10.2
    pDataInfo->reg_dmc_plane_level7           = 0;           // 12 bit , format 10.2
    pDataInfo->reg_dmc_plane_level8           = 0;           // 12 bit , format 10.2
    pDataInfo->reg_dmc_white_limit            = 0xFFF;       // 12 bit , format 10.2
    pDataInfo->reg_dmc_dither_en              = 0x1;         //  2 bit , 0 : rounding , 1 : dither , 2~3 : truncate
    pDataInfo->reg_dmc_dither_rst_e_4_frame   = 0x0;         //  1 bit
    pDataInfo->reg_dmc_dither_pse_rst_num     = 0x0;         //  2 bit

    // Init Output message
    pbin_info->HNode          = n_Hnode;
    pbin_info->VNode          = n_Vnode;
    pbin_info->LevelCount     = pDataInfo->reg_dmc_plane_num;
    pbin_info->Blk_h_size     = 1 << (pDataInfo->reg_dmc_h_block);
    pbin_info->Blk_v_size     = 1 << (pDataInfo->reg_dmc_v_block);

    MS_BOOL bRet = Alloc_LutIn_Space(pdat_info, pbin_info);
    if (bRet == FALSE)
    {
        UBOOT_ERROR("Alloc_LutIn_Space error\n");
        return FALSE;
    }

    bRet = parse_LG_bin_1tab(n_Hnode, n_Vnode, pDataInfo, multi_vendor);
    if (bRet == FALSE)
    {
        UBOOT_ERROR("parse_LG_bin_1tab error\n");
        return FALSE;
    }

    return TRUE;
#endif
}


