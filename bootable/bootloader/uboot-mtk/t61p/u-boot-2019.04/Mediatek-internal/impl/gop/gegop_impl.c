// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (c) 2023 MediaTek Inc.
*/

#include <common.h>
#include <command.h>
#include <config.h>
#include <apiGOP.h>
#include <apiGFX.h>
#include <apiPNL.h>
#include <system_impl.h>
#include <gegop_impl.h>
#include <jpd_impl.h>
#include <panel_impl.h>
#include <debug_impl.h>
#include <dts_parser.h>
#include <iniutility.h>
#include <mtk_gegop.h>
#include <mtk_panel.h>
#include <mtk_jpd.h>
#include <upgrade_osd.h>
#include "string_utility.c"
#include "loader_charset.h"
#include "loader_charset_eng.h"

#define ASC_CHAR_TABLE      loader_eng_character_set
#define ASC_FONT_TABLE      loader_eng_font
#define CHAR_WIDTH_TABLE    loader_eng_width_data

#define UNI_CHAR_TABLE      p_loader_character_set
#define UNI_FONT_TABLE      p_loader_font
#define UCHAR_WIDTH_TABLE   p_loader_width_data
#define UNI_CHAR_ARRAY      font24X24

#define MAX_CHAR_HEIGHT     32
#define ONE_CHAR_HEIGHT     24
#define EACH_CHAR_INTERVAL  2
#define SPACE_WIDTH         12
#define PROGRESS_NUM        32
#define PROGRESS_WIDTH      10
#define PROGRESS_INTERVAL   4
#define UNICODE             0
#define ASC                 1
#if (ENABLE_DISPLAY_PICTURE_UPDATE)
#define CANVAS_WIDTH        1280
#define CANVAS_HEIGHT       720
#else
#define CANVAS_WIDTH        720
#define CANVAS_HEIGHT       576
#endif
#define ID_MAGIC            0x554E

#define GOP_NONE_MIRROR     0
#define GOP_V_MIRROR        1
#define GOP_H_MIRROR        2
#define GOP_V_H_MIRROR      3

#define OFFSET_32           32
#define PANEL_DLG_PARTITION_PATH		"persist"
#define PANEL_DLG_INI_PATH		"panel_mode_cfg.ini"

unsigned char logo_miu_idx = 0;
unsigned int logo_type = E_LOGO_TYPE_NONE;
u16 u16osd_w = 0;
u16 u16osd_h = 0;
u8 using_font = FONT32X32;
osd_font_info font_info= {0};
unsigned char u8osd_layer_fb_id=FB_AND_WINDOW_ID_MAX;
unsigned char u8osd_layer_fb_id2=FB_AND_WINDOW_ID_MAX;
unsigned char u8osd_layer_gwin_id=FB_AND_WINDOW_ID_MAX;
unsigned char u8osd_layer_gwin_id2=FB_AND_WINDOW_ID_MAX;

bool panel_dlg_already_read = false;
int panel_dlg_read_ret = 0;
struct panel_dlg_info panel_dlg_info;

static unsigned int g_u32canvas_buffer=0;
static u16 line_offset = 0;
static u16 text_to_tal_width = 0;
static u16 g_u16record_osd_create_w;
static u16 g_u16record_osd_create_h;
static u16 g_u16record_osd_create_x;
static u16 g_u16record_osd_create_y;
static unsigned char font16X16[32] = {0};//16X16
static unsigned char font32X32[128] = {0};//32X32
static unsigned char font24X24[72] = {0};//24X24
static int is_gop_init = 0;
static bool is_120hz_output = false;
static bool separate_window = false;

static u32 u32font_bit_mask[] =
{
    0x80000000, 0x40000000, 0x20000000, 0x10000000,
    0x08000000, 0x04000000, 0x02000000, 0x01000000,
    0x00800000, 0x00400000, 0x00200000, 0x00100000,
    0x00080000, 0x00040000, 0x00020000, 0x00010000,
    0x00008000, 0x00004000, 0x00002000, 0x00001000,
    0x00000800, 0x00000400, 0x00000200, 0x00000100
};

//for new font small and large
static u8 u8font_bit_mask[] =
{
	0x80,
	0x40,
	0x20,
	0x10,
	0x08,
	0x04,
	0x02,
	0x01
};

static u8 font_size_table[8][4] =
{
    {32,32,12,2},
    {16,16,12,2}
};

const unsigned char *char_table[8]=
{
    loader_eng_font,
    loader_eng_font_small
};
const unsigned char *font_width_table[8] =
{
    loader_eng_width_data,
    loader_eng_width_data_small
};

static unsigned char sc_is_interlace(void)
{
    return 0;
}

static unsigned short sc_get_h_cap_start(void)
{
    return 0x60;
}

static void sys_pq_reduce_bw_for_osd(unsigned char pq_win, unsigned char osd_on)
{
    pq_win=osd_on=0;
}

static int get_pixel_byte(GFX_Buffer_Format color_fmt)
{
    int bytespp;

    switch (color_fmt)
    {
        case GFX_FMT_ABGR8888:
        case GFX_FMT_ARGB8888:
            bytespp = 4;
            break;
        case GFX_FMT_YUV422:
        case GFX_FMT_RGB565:
            bytespp = 2;
            break;
        default:
            bytespp = 0;
            break;
    }
    return bytespp;
}

static unsigned int get_dst_pitch(unsigned int src_pitch, GFX_Buffer_Format color_fmt)
{
    unsigned int dst_pitch;

    switch (color_fmt)
    {
        case GFX_FMT_ABGR8888:
        case GFX_FMT_ARGB8888:
            dst_pitch = src_pitch << 1;
            break;
        case GFX_FMT_YUV422:
            dst_pitch = src_pitch;
            break;
        default:
            dst_pitch = src_pitch;
            break;
    }
    return dst_pitch;
}

static void set_gop_win_stretch(unsigned char logo_gop_idx,GFX_BufferInfo *dst_info, bool hor_stretch, bool ver_stretch,unsigned short disp_x,unsigned short disp_y)
{
    int gwin_disp_x_max = 0;
    int gwin_disp_y_max = 0;
    int hsync_start = 0;

    if(mtk_gop_get_resolution(&gwin_disp_x_max, &gwin_disp_y_max, &hsync_start) != 0)
        UBOOT_ERROR("Warning: resolution information get failure, w:%d h:%d s:%d\n",gwin_disp_x_max,gwin_disp_y_max,hsync_start);

    MApi_GOP_GWIN_Set_HSCALE(hor_stretch, dst_info->u32Width, gwin_disp_x_max);
    if(dst_info->u32Width < gwin_disp_x_max)
        MApi_GOP_GWIN_Set_HStretchMode_EX(logo_gop_idx,E_GOP_HSTRCH_NEW4TAP_95);

    MApi_GOP_GWIN_Set_VSCALE(ver_stretch, dst_info->u32Height, gwin_disp_y_max);
    if(dst_info->u32Height < gwin_disp_y_max)
        MApi_GOP_GWIN_Set_VStretchMode_EX(logo_gop_idx,E_GOP_VSTRCH_LINEAR_GAIN2);
}

static unsigned int cal_decoded_logo_size(GFX_BufferInfo *dst_info)
{
    int bytespp  = 1;
    unsigned int logo_size = 0;

    UBOOT_TRACE("IN\n");
    bytespp = get_pixel_byte(dst_info->u32ColorFmt);
    if (bytespp == 0)
    {
        UBOOT_ERROR("Can not recognize dst_info->u32ColorFmt(%d)", (int)dst_info->u32ColorFmt);
        UBOOT_TRACE("OK\n");
        return 0;
    }

    UBOOT_DEBUG("bytespp = %d\n", bytespp);
    logo_size = dst_info->u32Width * dst_info->u32Height * bytespp;

    // Align to 1KB
    logo_size = (logo_size + 1023) & 0xFFFFFC00;
    UBOOT_DEBUG("logo_size = 0x%x\n", logo_size);
    UBOOT_TRACE("OK\n");
    return logo_size;
}

static bool gop_check_logo_buf_size(GFX_BufferInfo *dst_info, unsigned int buf_size, unsigned int *request_buf_size)
{
    unsigned int logo_buf_size;

    UBOOT_TRACE("IN\n");
    logo_buf_size = cal_decoded_logo_size(dst_info);
    *request_buf_size = logo_buf_size;
    if (logo_buf_size == 0)
    {
        *request_buf_size = 0xFFFFffff;
        UBOOT_ERROR("cal_decoded_logo_size Error\n");
        UBOOT_TRACE("OK\n");
        return FALSE;
    }

    if(buf_size >= logo_buf_size)
    {
        UBOOT_TRACE("OK\n");
        return TRUE;
    }
    else
    {
        // lower picture quality
        if (get_pixel_byte(dst_info->u32ColorFmt) > 2) // ARGB ColorFmt
        {
            dst_info->u32ColorFmt = GFX_FMT_YUV422;
            dst_info->u32Pitch    = (dst_info->u32Pitch >> 1);
            UBOOT_ERROR("E_MMAP_ID_BOOTLOGO_BUFFER too small, Change RGB8888 --> GFX_FMT_YUV422\n");

            logo_buf_size = cal_decoded_logo_size(dst_info);
            *request_buf_size = logo_buf_size;
            if (logo_buf_size == 0)
            {
                *request_buf_size = 0xFFFFffff;
                UBOOT_ERROR("cal_decoded_logo_size Error\n");
                UBOOT_TRACE("OK\n");
                return FALSE;
            }
            if(buf_size >= logo_buf_size)
            {
                UBOOT_TRACE("OK\n");
                return TRUE;
            }
        }
    }
    UBOOT_TRACE("OK\n");
    return FALSE;
}

static int gop_get_logo_addrs(size_t *phy_addr, GFX_BufferInfo *dst_info, unsigned int logo_output_buf_size)
{
    unsigned int logo_buf_size = 0;
    unsigned int logo_request_buf_size = 0;
    int ret;
    struct dts_mmap mmap;

    UBOOT_TRACE("IN\n");
    logo_buf_size = logo_output_buf_size;
    memset(&mmap, 0, sizeof(mmap));
    ret = parse_dt("/mmap_info/MI_BOOTLOGO_BUF",mmap_dt_parser,(void*)&mmap,"reg");
    if(ret < 0 || mmap.address == 0)
    {
        UBOOT_ERROR("Error: Get GOP buffer from DTS mmap failure, use default value in DTS GOP buffer\n");
        ret = parse_dt("/mediatek-drm-tv-kms",integer_dt_parser,(void*)phy_addr,"GRAPHIC_BOOTLOGO_BUF_ADDR");
        if(ret < 0)
        {
            UBOOT_ERROR("Error: Get GOP buffer from DTS GOP failure, Please check u-boot dts\n");
            return -1;
        }
    }
    else
        *phy_addr = virtual_addr_to_phyical_addr(mmap.address);

    if (gop_check_logo_buf_size(dst_info, logo_buf_size, &logo_request_buf_size) == TRUE)
    {
        logo_miu_idx = 0;
        UBOOT_DEBUG("BOOTLOGO SET GOP MIU[%d]\n",logo_miu_idx);
        return 0;
    }

    UBOOT_TRACE("OK\n");
    return 0;
}

static int gop_get_vg_separate(char *status)
{
    UBOOT_TRACE("IN\n");
    if (parse_dt(GOP_DTS_NODE_NAME, string_dt_parser, (void*)status, GOP_VG_SEPARATE_PROP) != 0)
    {
        UBOOT_ERROR("Error: %s:%s information parse error in DTS\n",GOP_DTS_NODE_NAME,GOP_VG_SEPARATE_PROP);
        return -1;
    }
    UBOOT_INFO("%s:%s information:%s\n",GOP_DTS_NODE_NAME,GOP_VG_SEPARATE_PROP,status);
    UBOOT_TRACE("OK\n");
    return 0;
}

int mtk_gop_get_resolution(int *resolution_width, int *resolution_height, int *hsync_start)
{
    char node_name[32]={0};
    char status[8]={0};
    int snprintf_len = 0;

    UBOOT_TRACE("IN\n");
    if(gop_get_vg_separate(status) != 0)
    {
        UBOOT_ERROR("Warning: VG separate information get failure, use default path\n");
        memset(status, 0, sizeof(status));
        strncpy(status, GOP_VG_SEPARATE_DISABLE, sizeof(status));
    }

    if(!panel_dlg_already_read)
    {
        panel_dlg_read_ret = get_panel_dlg_info(PANEL_DLG_PARTITION_PATH, PANEL_DLG_INI_PATH, &panel_dlg_info);
        panel_dlg_already_read = true;
    }

    if (strncmp(status, GOP_VG_SEPARATE_ENABLE, strlen(GOP_VG_SEPARATE_ENABLE)) == 0)
    {
        snprintf_len = snprintf(node_name, sizeof(node_name), "%s/%s",GOP_DTS_NODE_NAME,RESOLUTION_DTS_SUBNAME_GFX);

        if(snprintf_len < 0 || (unsigned)snprintf_len >= sizeof(node_name))
        {
            UBOOT_ERROR("The array size is too small(%lu), snprintf fail '%s %s'.\n", sizeof(node_name), GOP_DTS_NODE_NAME, RESOLUTION_DTS_SUBNAME_GFX);
            return -1;
        }
    }
    else
    {
        if((panel_dlg_read_ret >= 0) && (panel_dlg_info.panel_dlg == 1))
        {
            snprintf_len = snprintf(node_name, sizeof(node_name), "%s/%s",PANEL_DTS_NODE_NAME,RESOLUTION_DTS_SUBNAME_MAIN_DLG);

            if(snprintf_len < 0 || (unsigned)snprintf_len >= sizeof(node_name))
            {
                UBOOT_ERROR("The array size is too small(%lu), snprintf fail '%s %s'.\n", sizeof(node_name), PANEL_DTS_NODE_NAME, RESOLUTION_DTS_SUBNAME_MAIN_DLG);
                return -1;
            }
        }
        else
        {
        snprintf_len = snprintf(node_name, sizeof(node_name), "%s/%s",PANEL_DTS_NODE_NAME,RESOLUTION_DTS_SUBNAME_MAIN);

        if(snprintf_len < 0 || (unsigned)snprintf_len >= sizeof(node_name))
        {
            UBOOT_ERROR("The array size is too small(%lu), snprintf fail '%s %s'.\n", sizeof(node_name), PANEL_DTS_NODE_NAME, RESOLUTION_DTS_SUBNAME_MAIN);
            return -1;
        }
    }
    }

    UBOOT_INFO("DTS node name:%s\n",node_name);
    if (parse_dt(node_name, integer_dt_parser, (void*)resolution_width, DISPLAY_RESOLUTION_WIDTH) != 0)
    {
        UBOOT_ERROR("Error: %s:%s information parse error in DTS\n",node_name,DISPLAY_RESOLUTION_WIDTH);
        return -1;
    }
    UBOOT_INFO("%s:%s information:%d\n",node_name,DISPLAY_RESOLUTION_WIDTH,*resolution_width);

    if (parse_dt(node_name, integer_dt_parser, (void*)resolution_height, DISPLAY_RESOLUTION_HEIGHT) != 0)
    {
        UBOOT_ERROR("Error: %s:%s information parse error in DTS\n",node_name,DISPLAY_RESOLUTION_HEIGHT);
        return -1;
    }
    UBOOT_INFO("%s:%s information:%d\n",node_name,DISPLAY_RESOLUTION_HEIGHT,*resolution_height);

    if (parse_dt(node_name, integer_dt_parser, (void*)hsync_start, DISPLAY_HSYNC_START) != 0)
    {
        UBOOT_ERROR("Error: %s:%s information parse error in DTS\n",node_name,DISPLAY_HSYNC_START);
        return -1;
    }
    UBOOT_INFO("%s:%s information:%d\n",node_name,DISPLAY_HSYNC_START,*hsync_start);
    UBOOT_TRACE("OK\n");
    return 0;
}

static int mtk_gop_check_resolution_rate(void)
{
    char node_name[BUFFER_LENGTH]={0};
    char status[BUFFER_LENGTH]={0};
    unsigned int typ_h_total = 0,typ_v_total = 0;
	unsigned int typ_clk_h = 0, typ_clk_l = 0;
	MS_U64 typ_clk;
    int resolution_width=0, resolution_height=0, hsync_start=0;
    int snprintf_len = 0;

    UBOOT_TRACE("IN\n");
    if(gop_get_vg_separate(status) != 0)
    {
        UBOOT_ERROR("Warning: VG separate information get failure, use default path\n");
        memset(status, 0, sizeof(status));
        strncpy(status, GOP_VG_SEPARATE_DISABLE, sizeof(status));
    }

    if(!panel_dlg_already_read)
    {
        panel_dlg_read_ret = get_panel_dlg_info(PANEL_DLG_PARTITION_PATH, PANEL_DLG_INI_PATH, &panel_dlg_info);
        panel_dlg_already_read = true;
    }

    if (strncmp(status, GOP_VG_SEPARATE_ENABLE, strlen(GOP_VG_SEPARATE_ENABLE)) == 0)
    {
        snprintf_len = snprintf(node_name, sizeof(node_name), "%s/%s",GOP_DTS_NODE_NAME,RESOLUTION_DTS_SUBNAME_GFX);
        if(snprintf_len < 0 || (unsigned)snprintf_len >= sizeof(node_name))
        {
            UBOOT_ERROR("The array size is too small(%lu), snprintf fail '%s %s'.\n", sizeof(node_name), GOP_DTS_NODE_NAME, RESOLUTION_DTS_SUBNAME_GFX);
            return -1;
        }
    }
    else
    {
        if((panel_dlg_read_ret >= 0) && (panel_dlg_info.panel_dlg == 1))
        {
            UBOOT_INFO("Panel DLG ON\n");
            snprintf_len = snprintf(node_name, sizeof(node_name), "%s/%s",PANEL_DTS_NODE_NAME,RESOLUTION_DTS_SUBNAME_MAIN_DLG);
            if(snprintf_len < 0 || (unsigned)snprintf_len >= sizeof(node_name))
            {
                UBOOT_ERROR("The array size is too small(%lu), snprintf fail '%s %s'.\n", sizeof(node_name), PANEL_DTS_NODE_NAME, RESOLUTION_DTS_SUBNAME_MAIN_DLG);
                return -1;
            }
        }
        else
        {
        snprintf_len = snprintf(node_name, sizeof(node_name), "%s/%s",PANEL_DTS_NODE_NAME,RESOLUTION_DTS_SUBNAME_MAIN);
        if(snprintf_len < 0 || (unsigned)snprintf_len >= sizeof(node_name))
        {
            UBOOT_ERROR("The array size is too small(%lu), snprintf fail '%s %s'.\n", sizeof(node_name), PANEL_DTS_NODE_NAME, RESOLUTION_DTS_SUBNAME_MAIN);
            return -1;
        }
    }
    }
    UBOOT_INFO("DTS node name:%s\n",node_name);
    if (parse_dt(node_name, integer_dt_parser, (void*)&typ_h_total, DISPLAY_TYP_H_TOTAL) != 0)
    {
        UBOOT_ERROR("Error: %s:%s information parse error in DTS\n",node_name,DISPLAY_TYP_H_TOTAL);
        return -1;
    }
    UBOOT_INFO("%s:%s information:%d\n",node_name,DISPLAY_TYP_H_TOTAL,typ_h_total);
    if (parse_dt(node_name, integer_dt_parser, (void*)&typ_v_total, DISPLAY_TYP_V_TOTAL) != 0)
    {
        UBOOT_ERROR("Error: %s:%s information parse error in DTS\n",node_name,DISPLAY_TYP_V_TOTAL);
        return -1;
    }
    UBOOT_INFO("%s:%s information:%d\n",node_name,DISPLAY_TYP_V_TOTAL,typ_v_total);
	if (parse_dt(node_name, integer_dt_parser, (void*)&typ_clk_h, DISPLAY_TYP_CLK_H) != 0)
	{
		UBOOT_ERROR("Error: %s:%s information parse error in DTS\n",node_name,DISPLAY_TYP_CLK_H);
		return -1;
	}
	if (parse_dt(node_name, integer_dt_parser, (void*)&typ_clk_l, DISPLAY_TYP_CLK_L) != 0)
	{
		UBOOT_ERROR("Error: %s:%s information parse error in DTS\n",node_name,DISPLAY_TYP_CLK_L);
		return -1;
	}
	typ_clk = ((MS_U64)typ_clk_h << OFFSET_32) | typ_clk_l;
	UBOOT_INFO("%s:%s information:%d\n",node_name,DISPLAY_TYP_CLK_H,typ_clk_h);
	UBOOT_INFO("%s:%s information:%d\n",node_name,DISPLAY_TYP_CLK_L,typ_clk_l);

    if((((typ_clk/typ_h_total)/typ_v_total) == OUTPUT_FRAME_RATE_120) ||
		(((typ_clk/typ_h_total)/typ_v_total) == OUTPUT_FRAME_RATE_144))
        is_120hz_output = true;
	else
		is_120hz_output = false;

    if(mtk_gop_get_resolution(&resolution_width, &resolution_height, &hsync_start) != 0)
        UBOOT_ERROR("Warning: resolution information get failure, w:%d h:%d s:%d\n",resolution_width,resolution_height,hsync_start);

    UBOOT_INFO("resolution[%d:%d] Hz:%lu\n",resolution_width,resolution_height,((typ_clk/typ_h_total)/typ_v_total));
    if((resolution_width == RESOLUTION_WIDTH_4K && resolution_height == RESOLUTION_HEIGHT_4K && is_120hz_output == true) ||
		(resolution_width == RESOLUTION_WIDTH_8K && resolution_height == RESOLUTION_HEIGHT_8K && is_120hz_output == true))
        separate_window = true;
    else
        separate_window = false;

    UBOOT_INFO("is_120hz_output:%d separate_window:%d\n",is_120hz_output,separate_window);
    UBOOT_TRACE("OK\n");
    return 0;
}

static void gfx_init(void)
{
    GFX_Config gfxcfg;
    memset(&gfxcfg,0,sizeof(gfxcfg));
    gfxcfg.bIsCompt = TRUE;
    gfxcfg.bIsHK = TRUE;
    MApi_GFX_Init(&gfxcfg);
}

static int gfx_blt(GFX_DrawRect *rect,GFX_BufferInfo *src_info, GFX_BufferInfo *dst_info )
{
    GFX_Point gfx_pt0 = { rect->dstblk.x, rect->dstblk.y };
    GFX_Point gfx_pt1 = { rect->dstblk.x+rect->dstblk.width,rect->dstblk.y+rect->dstblk.height };
    GFX_Buffer_Format colorfmt = dst_info->u32ColorFmt;

    MApi_GFX_SetSrcBufferInfo(src_info, 0);

    MApi_GFX_SetDstBufferInfo(dst_info, 0);

    MApi_GFX_SetClip(&gfx_pt0, &gfx_pt1);

    if ((colorfmt == GFX_FMT_ABGR8888) || (colorfmt == GFX_FMT_ARGB8888))
    {
        MApi_GFX_SetDC_CSC_FMT(GFX_YUV_RGB2YUV_255, GFX_YUV_OUT_PC, GFX_YUV_IN_255, GFX_YUV_YUYV,  GFX_YUV_YUYV);
    }
    else
    {
        MApi_GFX_SetDC_CSC_FMT(GFX_YUV_RGB2YUV_PC, GFX_YUV_OUT_PC, GFX_YUV_IN_255, GFX_YUV_YVYU,  GFX_YUV_YUYV);
    }

    if( (rect->srcblk.width!= rect->dstblk.width)||(rect->srcblk.height!= rect->dstblk.height) )
    {
        MApi_GFX_BitBlt(rect, GFXDRAW_FLAG_SCALE);
    }
    else
    {
        MApi_GFX_BitBlt(rect, 0);
    }
    MApi_GFX_FlushQueue();

    return 1;
}

static int gop_init(unsigned char logo_gop_idx)
{
    UBOOT_TRACE("IN\n");
    GOP_InitInfo gop_init_info;
    st_gopmux_para gop_setting;
    unsigned short gop_layer_num;
    int resolution_width=0, resolution_height=0, hsync_start=0;
    int i,ret;

    //MApi_GOP_RegisterFBFmtCB(( U32(*)(MS_U16 pitch,U32 addr , MS_U16 fmt ))OSD_RESOURCE_SetFBFmt);
    MApi_GOP_RegisterXCIsInterlaceCB(sc_is_interlace);
    MApi_GOP_RegisterXCGetCapHStartCB(sc_get_h_cap_start);
    MApi_GOP_RegisterXCReduceBWForOSDCB(sys_pq_reduce_bw_for_osd);
    if(mtk_gop_get_resolution(&resolution_width, &resolution_height, &hsync_start) != 0)
        UBOOT_ERROR("Warning: resolution information get failure, w:%d h:%d s:%d\n",resolution_width,resolution_height,hsync_start);

    gop_init_info.u16PanelWidth = (unsigned short)(resolution_width & 0xFFFF);
    gop_init_info.u16PanelHeight = (unsigned short)(resolution_height & 0xFFFF);
    gop_init_info.u16PanelHStr = (unsigned short)(hsync_start & 0xFFFF);
    gop_init_info.u32GOPRBAdr = 0x0;//((GOP_GWIN_RB_MEMORY_TYPE & MIU1) ? (GOP_GWIN_RB_ADR | MIU_INTERVAL) : (GOP_GWIN_RB_ADR));
    gop_init_info.u32GOPRBLen = 0x0;

    //there is a GOP_REGDMABASE_MIU1_ADR for MIU1
    gop_init_info.u32GOPRegdmaAdr = 0;//((GOP_REGDMABASE_MEMORY_TYPE & MIU1) ? (GOP_REGDMABASE_ADR | MIU_INTERVAL) : (GOP_REGDMABASE_ADR));
    gop_init_info.u32GOPRegdmaLen = 0;
    gop_init_info.bEnableVsyncIntFlip = FALSE;


    MApi_GOP_GWIN_SetForceWrite(TRUE);
    MApi_GOP_InitByGOP(&gop_init_info, logo_gop_idx);
    MApi_GOP_GWIN_SwitchGOP(logo_gop_idx);

    // read the MIU in which the bootlogo stored
    UBOOT_DEBUG("BOOTLOGO SET GOP INDEX[%d]\n",logo_gop_idx);
    UBOOT_DEBUG("BOOTLOGO SET GOP MIU[%d]\n",logo_miu_idx);
    MApi_GOP_GWIN_OutputColor(GOPOUT_RGB);  //TV

    memset(&gop_setting, 0, sizeof(st_gopmux_para));
    gop_layer_num = MApi_GOP_GWIN_GetMaxGOPNum();
    gop_setting.gop_layer_num = gop_layer_num;
    gop_setting.gop_layer_index = (unsigned short *)malloc(gop_layer_num*2);
    if(gop_setting.gop_layer_index == NULL)
    {
        UBOOT_ERROR("Error: malloc fail!\n");
        return -1;
    }

    ret = parse_dt("/mediatek-drm-tv-kms",gop_dt_parser,(void*)&gop_setting,NULL);
    if(ret < 0)
    {
        UBOOT_ERROR("Error: GOP information parse error in DTS\n");
        free(gop_setting.gop_layer_index);
        return -1;
    }

    GOP_LayerConfig layerConfig;
    layerConfig.u32LayerCounts = gop_setting.gop_layer_num;
    for(i=0;i<gop_setting.gop_layer_num;i++)
    {
        layerConfig.stGopLayer[i].u32LayerIndex = i;
        layerConfig.stGopLayer[i].u32GopIndex = gop_setting.gop_layer_index[i];
    }
    MApi_GOP_GWIN_SetLayer(&layerConfig, sizeof(GOP_LayerConfig));

    MApi_GOP_GWIN_SetForceWrite(FALSE);
    free(gop_setting.gop_layer_index);
    UBOOT_TRACE("OK\n");
    return 0;
}

static void gop_show(unsigned char logo_gop_idx, GFX_BufferInfo *dst_info,bool hor_stretch, bool ver_stretch,unsigned short disp_x,unsigned short disp_y)
{
    UBOOT_TRACE("IN\n");
    unsigned int width;
    unsigned int pixelmode=FALSE;
    char status[8]={0};
    EN_GOP_CONSALPHA_BITS gop_consalpha_bit;
    MApi_GOP_GWIN_UpdateRegOnceEx2(logo_gop_idx, true, false);
    MApi_GOP_GWIN_SwitchGOP(logo_gop_idx);

    if(gop_get_vg_separate(status) != 0)
    {
        UBOOT_ERROR("Warning: VG separate information get failure, use default path\n");
        memset(status, 0, sizeof(status));
        strncpy(status, GOP_VG_SEPARATE_DISABLE, sizeof(status));
    }

    if (strncmp(status, GOP_VG_SEPARATE_ENABLE, strlen(GOP_VG_SEPARATE_ENABLE)) == 0)
    {
        UBOOT_DEBUG("VG separate path:E_GOP_DST_FRC\n");
        MApi_GOP_GWIN_SetGOPDst(logo_gop_idx, E_GOP_DST_FRC);
    }
    else
    {
        UBOOT_DEBUG("VG separate path:E_GOP_DST_OP0\n");
        MApi_GOP_GWIN_SetGOPDst(logo_gop_idx, E_GOP_DST_OP0);
    }

    MApi_GOP_GetChipCaps(E_GOP_CAP_PIXELMODE_SUPPORT, &pixelmode,sizeof(pixelmode));
    if (pixelmode==FALSE)//word mode
    {
        unsigned int wordunit=0;
        unsigned int aligement =0;
        unsigned int bytespp = 0;
        MApi_GOP_GetChipCaps(E_GOP_CAP_WORD_UNIT , & wordunit, sizeof(unsigned int));
        bytespp = get_pixel_byte(dst_info->u32ColorFmt);
        if (bytespp == 0)
        {
            UBOOT_ERROR("get_pixel_byte Error\n");
            bytespp = 2;
        }

        aligement = wordunit/ bytespp;   //YUV422: 2
        width=ALIGN(dst_info->u32Width, aligement);
        if((width + disp_x) > mtk_get_current_panel_width())
        {
            width = dst_info->u32Width & ~(aligement - 1);
        }
        dst_info->u32Width=width;
        UBOOT_DEBUG("word mode width 0x%x \n",(unsigned int)width);
    }
    else //pixel mode
    {
        UBOOT_DEBUG("pixel mode width 0x%x \n",(unsigned int)dst_info->u32Width);
    }
    UBOOT_DEBUG("===== logo_gop_idx[0x%x] hor_stretch[0x%x] ver_stretch[0x%x] disp_x[0x%x] disp_y[0x%x]=====\n\n", logo_gop_idx, hor_stretch, ver_stretch,disp_x,disp_y);

    u8osd_layer_fb_id = MApi_GOP_GWIN_GetFreeFBID();
    if(u8osd_layer_gwin_id != 0xFF)
    {
        UBOOT_INFO("GWIN %d has used, Picture is already on screen, delete GWIN first\n", u8osd_layer_gwin_id);
        MApi_GOP_GWIN_DeleteWin(u8osd_layer_gwin_id);
    }
    set_gop_win_stretch(logo_gop_idx,dst_info, hor_stretch, ver_stretch,disp_x,disp_y);
    if(separate_window == true && (dst_info->u32Width == RESOLUTION_WIDTH_4K && dst_info->u32Height == RESOLUTION_HEIGHT_4K))
        MApi_GOP_GWIN_Set_STRETCHWIN(logo_gop_idx, E_GOP_DST_OP0,disp_x,disp_y,dst_info->u32Width/SEPERATE_NUM,dst_info->u32Height);
    else
        MApi_GOP_GWIN_Set_STRETCHWIN(logo_gop_idx, E_GOP_DST_OP0,disp_x,disp_y, dst_info->u32Width, dst_info->u32Height);

    u8osd_layer_gwin_id = MApi_GOP_GWIN_GetFreeWinID();
    UBOOT_DEBUG("===== dst_info->u32Width[0x%x] dst_info->u32Height[0x%x] dst_info->u32ColorFmt[0x%x] dst_info->u32Addr[0x%x] =====\n\n", dst_info->u32Width, dst_info->u32Height, dst_info->u32ColorFmt, (unsigned int)dst_info->u32Addr);
    MApi_GOP_GWIN_CreateFBbyStaticAddr(u8osd_layer_fb_id, 0, 0, dst_info->u32Width, dst_info->u32Height, dst_info->u32ColorFmt, dst_info->u32Addr);

    MApi_GOP_GWIN_MapFB2Win(u8osd_layer_fb_id, u8osd_layer_gwin_id);
    MApi_GOP_GetChipCaps(E_GOP_CAP_CONSALPHA_VALIDBITS, &gop_consalpha_bit,sizeof(gop_consalpha_bit));
    MApi_GOP_SetConfig(E_GOP_CONSALPHA_BITS, &gop_consalpha_bit);
    MApi_GOP_GWIN_SetBlending(u8osd_layer_gwin_id, FALSE, 0xFF);
    MApi_GOP_GWIN_Enable(u8osd_layer_gwin_id, TRUE);
    MApi_GOP_GWIN_UpdateRegOnceEx2(logo_gop_idx , false, false);
    UBOOT_TRACE("OK\n");
}

static void gop_show_seperate(unsigned char logo_gop_idx, GFX_BufferInfo *dst_info,bool hor_stretch, bool ver_stretch,unsigned short disp_x,unsigned short disp_y)
{
    UBOOT_TRACE("IN\n");
    unsigned int width;
    unsigned int pixelmode=FALSE;
    char status[BUFFER_LENGTH]={0};
    EN_GOP_CONSALPHA_BITS gop_consalpha_bit;
    MApi_GOP_GWIN_UpdateRegOnceEx2(logo_gop_idx, true, false);
    MApi_GOP_GWIN_SwitchGOP(logo_gop_idx);

    if(gop_get_vg_separate(status) != 0)
    {
        UBOOT_ERROR("Warning: VG separate information get failure, use default path\n");
        memset(status, 0, sizeof(status));
        strncpy(status, GOP_VG_SEPARATE_DISABLE, sizeof(status));
    }

    if (strncmp(status, GOP_VG_SEPARATE_ENABLE, strlen(GOP_VG_SEPARATE_ENABLE)) == 0)
    {
        UBOOT_DEBUG("VG separate path:E_GOP_DST_FRC\n");
        MApi_GOP_GWIN_SetGOPDst(logo_gop_idx, E_GOP_DST_FRC);
    }
    else
    {
        UBOOT_DEBUG("VG separate path:E_GOP_DST_OP0\n");
        MApi_GOP_GWIN_SetGOPDst(logo_gop_idx, E_GOP_DST_OP0);
    }

    MApi_GOP_GetChipCaps(E_GOP_CAP_PIXELMODE_SUPPORT, &pixelmode,sizeof(pixelmode));
    if (pixelmode==FALSE)//word mode
    {
        unsigned int wordunit=0;
        unsigned int aligement =0;
        unsigned int bytespp = 0;
        MApi_GOP_GetChipCaps(E_GOP_CAP_WORD_UNIT , & wordunit, sizeof(unsigned int));
        bytespp = get_pixel_byte(dst_info->u32ColorFmt);
        if (bytespp == 0)
        {
            UBOOT_ERROR("get_pixel_byte Error\n");
            bytespp = BYTESPP;
        }

        aligement = wordunit/ bytespp;   //YUV422: 2
        width=ALIGN(dst_info->u32Width, aligement);
        if((width + disp_x) > mtk_get_current_panel_width())
        {
            width = dst_info->u32Width & ~(aligement - 1);
        }
        dst_info->u32Width=width;
        UBOOT_DEBUG("word mode width 0x%x \n",(unsigned int)width);
    }
    else //pixel mode
    {
        UBOOT_DEBUG("pixel mode width 0x%x \n",(unsigned int)dst_info->u32Width);
    }
    UBOOT_DEBUG("===== logo_gop_idx[0x%x] hor_stretch[0x%x] ver_stretch[0x%x] disp_x[0x%x] disp_y[0x%x]=====\n\n", logo_gop_idx, hor_stretch, ver_stretch,disp_x,disp_y);

    u8osd_layer_fb_id2 = MApi_GOP_GWIN_GetFreeFBID();
    if(u8osd_layer_gwin_id2 != FB_AND_WINDOW_ID_MAX)
    {
        UBOOT_INFO("GWIN %d has used, Picture is already on screen, delete GWIN first\n", u8osd_layer_gwin_id2);
        MApi_GOP_GWIN_DeleteWin(u8osd_layer_gwin_id2);
    }

    set_gop_win_stretch(logo_gop_idx,dst_info, hor_stretch, ver_stretch,disp_x,disp_y);
    MApi_GOP_GWIN_Set_STRETCHWIN(logo_gop_idx, E_GOP_DST_OP0, dst_info->u32Width/SEPERATE_NUM, 0, dst_info->u32Width/SEPERATE_NUM, dst_info->u32Height);
    u8osd_layer_gwin_id2 = MApi_GOP_GWIN_GetFreeWinID();

    UBOOT_DEBUG("===== dst_info->u32Width[0x%x] dst_info->u32Height[0x%x] dst_info->u32ColorFmt[0x%x] dst_info->u32Addr[0x%x] =====\n\n", dst_info->u32Width, dst_info->u32Height, dst_info->u32ColorFmt, (unsigned int)dst_info->u32Addr);
    MApi_GOP_GWIN_CreateFBbyStaticAddr(u8osd_layer_fb_id2, (dst_info->u32Width/SEPERATE_NUM), 0, dst_info->u32Width, dst_info->u32Height, dst_info->u32ColorFmt, dst_info->u32Addr);

    MApi_GOP_GWIN_MapFB2Win(u8osd_layer_fb_id2, u8osd_layer_gwin_id2);
    MApi_GOP_GetChipCaps(E_GOP_CAP_CONSALPHA_VALIDBITS, &gop_consalpha_bit,sizeof(gop_consalpha_bit));
    MApi_GOP_SetConfig(E_GOP_CONSALPHA_BITS, &gop_consalpha_bit);
    MApi_GOP_GWIN_SetBlending(u8osd_layer_gwin_id2, FALSE, FB_AND_WINDOW_ID_MAX);
    MApi_GOP_GWIN_Enable(u8osd_layer_gwin_id2, TRUE);
    MApi_GOP_GWIN_UpdateRegOnceEx2(logo_gop_idx , false, false);
    UBOOT_TRACE("OK\n");
}

static int gop_display_logo(GFX_DrawRect *rect,GFX_BufferInfo *src_info,GFX_BufferInfo *dst_info, bool hor_stretch, bool ver_stretch,unsigned short disp_x,unsigned short disp_y)
{
    bool panel_mirror_mode = FALSE;
    unsigned char logo_gop_idx =0;
    int ret;
    int mirror_mode = GOP_NONE_MIRROR;
    ret = parse_dt("/mediatek-drm-tv-kms",integer_dt_parser,(void*)&logo_gop_idx,"GRAPHIC_BOOTLOGO_GOPIDX");
    if(ret < 0)
    {
        UBOOT_ERROR("Error: GRAPHIC_BOOTLOGO_GOPIDX information parse error in DTS\n");
        return -1;
    }

    UBOOT_TRACE("IN\n");
    UBOOT_INFO("[%s] GOP Boot Logo Index %d \n",__FUNCTION__,(int)logo_gop_idx);
    if(mtk_gop_check_resolution_rate() != 0)
        UBOOT_DEBUG("Check panel resolution and rate failure\n");

    gfx_init();
    if(is_gop_init == 0)
    {
        ret = gop_init(logo_gop_idx);
        if(ret < 0)
        {
            UBOOT_ERROR("Error: gop_init function execute failure\n");
            return -1;
        }

        if(separate_window == true && (dst_info->u32Width == RESOLUTION_WIDTH_4K && dst_info->u32Height == RESOLUTION_HEIGHT_4K) && logo_gop_idx > 0)
        {
            ret = gop_init(logo_gop_idx-1);
            if(ret < 0)
            {
                UBOOT_ERROR("Error: gop_init function execute failure\n");
                return -1;
            }
        }
        is_gop_init = 1;
    }

    flush_dcache_all();
    gfx_blt(rect,src_info,dst_info);
    src_info->u32Pitch =src_info->u32Pitch>>1;
    dst_info->u32Pitch =dst_info->u32Pitch>>1;

    ret = parse_dt("/video_out",integer_dt_parser,(void*)&mirror_mode,"PanelMirrorMode");

    if(ret < 0)
    {
        UBOOT_ERROR("Error: PanelMirrorMode information parse error in DTS\n");
        return -1;
    }

    if(GOP_V_H_MIRROR==mirror_mode)
    {
        UBOOT_INFO("<<set_mirror V & H ON!!>> \n");
        MApi_GOP_GWIN_SetVMirror(TRUE);
        if(panel_mirror_mode==FALSE)
        {
            MApi_GOP_GWIN_SetHMirror(TRUE);
        }
        else
        {
            UBOOT_INFO("<<Skip H ON!!>> \n");
        }
    }
    else if(GOP_V_MIRROR==mirror_mode)
    {
        UBOOT_INFO("<<set_mirror V ON!!>> \n");
        MApi_GOP_GWIN_SetVMirror(TRUE);
    }
    else if(GOP_H_MIRROR==mirror_mode)
    {
        if(panel_mirror_mode==FALSE)
        {
            UBOOT_INFO("<<set_mirror H ON!!>> \n");
            MApi_GOP_GWIN_SetHMirror(TRUE);
        }
        else
        {
            UBOOT_INFO("<<MOD H_Mirror Enable !!>> \n");
        }
    }

    gop_show(logo_gop_idx,dst_info,hor_stretch,ver_stretch,disp_x,disp_y);
    if(separate_window == true && (dst_info->u32Width == RESOLUTION_WIDTH_4K && dst_info->u32Height == RESOLUTION_HEIGHT_4K) && logo_gop_idx > 0)
        gop_show_seperate(logo_gop_idx-1,dst_info,hor_stretch,ver_stretch,disp_x,disp_y);

    UBOOT_TRACE("OK\n");
    return 0;
}

int mtk_gop_show_LvglPicture(logo_disp_config_params config, struct bootlogo_info *logo_info, unsigned int logo_output_buf, unsigned int logo_output_buf_size)
{
    GFX_BufferInfo src_info, dst_info;
    GFX_DrawRect rect;
    int mirror_mode = GOP_NONE_MIRROR;
    bool panel_mirror_mode = FALSE;
    unsigned char logo_gop_idx = 0;
    size_t addr = 0;
    int ret = -1;

    UBOOT_TRACE("IN\n");

    src_info.u32Addr = logo_output_buf;
    src_info.u32ColorFmt = logo_info->color_format;
    src_info.u32Width = (unsigned int)logo_info->width;
    src_info.u32Height = (unsigned int)logo_info->height;
    src_info.u32Pitch = (unsigned int)(logo_info->width << 1);
    dst_info.u32ColorFmt = logo_info->color_format;
    rect.srcblk.x = 0;
    rect.srcblk.y = 0;
    rect.srcblk.width = logo_info->width;
    rect.srcblk.height = logo_info->height;
    rect.dstblk.x = 0;
    rect.dstblk.y = 0;

    UBOOT_DEBUG("src_info.u32Addr=0x%X\n", (unsigned int)src_info.u32Addr);
    UBOOT_DEBUG("src_info.u32ColorFmt=0x%X\n", (unsigned int)src_info.u32ColorFmt);
    UBOOT_DEBUG("src_info.u32Width=0x%X\n", (unsigned int)src_info.u32Width);
    UBOOT_DEBUG("src_info.u32Height=0x%X\n", (unsigned int)src_info.u32Height);
    UBOOT_DEBUG("src_info.u32Pitch=0x%X\n", (unsigned int)src_info.u32Pitch);
    UBOOT_DEBUG("dst_info.u32ColorFmt=0x%X\n", (unsigned int)dst_info.u32ColorFmt);
    UBOOT_DEBUG("rect.srcblk.x=0x%X\n", (unsigned int)rect.srcblk.x);
    UBOOT_DEBUG("rect.srcblk.y=0x%X\n", (unsigned int)rect.srcblk.y);
    UBOOT_DEBUG("rect.srcblk.width=0x%X\n", (unsigned int)rect.srcblk.width);
    UBOOT_DEBUG("rect.srcblk.height=0x%X\n", (unsigned int)rect.srcblk.height);
    UBOOT_DEBUG("rect.dstblk.x=0x%X\n", (unsigned int)rect.dstblk.x);
    UBOOT_DEBUG("rect.dstblk.y=0x%X\n", (unsigned int)rect.dstblk.y);

    if( src_info.u32Width > mtk_get_current_panel_width())
    {
        rect.dstblk.width = (mtk_get_current_panel_width()&0xFFFFF0)/*u16Pitch*/;
        dst_info.u32Width = (unsigned int)(mtk_get_current_panel_width()&0xFFFFF0);
        dst_info.u32Pitch = get_dst_pitch((mtk_get_current_panel_width()&0xFFFFF0), dst_info.u32ColorFmt);
    }
    else
    {
        rect.dstblk.width = ((src_info.u32Width+15)&0xFFFFF0)/*u16Pitch*/;
        dst_info.u32Width = (unsigned int)((src_info.u32Width+15)&0xFFFFF0);
        dst_info.u32Pitch = get_dst_pitch(((src_info.u32Pitch+15)&0xFFFFF0), dst_info.u32ColorFmt);
    }
    UBOOT_DEBUG("rect.dstblk.width=0x%X\n", (unsigned int)rect.dstblk.width);
    UBOOT_DEBUG("dst_info.u32Width=0x%X\n", (unsigned int)dst_info.u32Width);
    UBOOT_DEBUG("dst_info.u32Pitch=0x%X\n", (unsigned int)dst_info.u32Pitch);

    if( src_info.u32Height > mtk_get_current_panel_height())
    {
        rect.dstblk.height = mtk_get_current_panel_height();
        dst_info.u32Height = mtk_get_current_panel_height();
    }
    else
    {
        rect.dstblk.height = src_info.u32Height;
        dst_info.u32Height = (unsigned int)src_info.u32Height;
    }
    UBOOT_DEBUG("rect.dstblk.height=0x%X\n", (unsigned int)rect.dstblk.height);
    UBOOT_DEBUG("dst_info.u32Height=0x%X\n", (unsigned int)dst_info.u32Height);

    ret = gop_get_logo_addrs(&addr, &dst_info, logo_output_buf_size);
    if(ret != 0)
    {
        UBOOT_ERROR("get lvgl addr error\n");
        return -1;
    }
    dst_info.u32Addr = (MS_PHYADDR)addr;
    UBOOT_DEBUG("dst_info.u32Addr=0x%X\n", (unsigned int)dst_info.u32Addr);

    ret = parse_dt("/mediatek-drm-tv-kms", integer_dt_parser, (void*)&logo_gop_idx, "GRAPHIC_BOOTLOGO_GOPIDX");
    if(ret < 0)
    {
        UBOOT_ERROR("Error: GRAPHIC_BOOTLOGO_GOPIDX information parse error in DTS\n");
        return -1;
    }
    UBOOT_INFO("[%s] GOP LVGL Index %d \n", __FUNCTION__, (int)logo_gop_idx);

    if(mtk_gop_check_resolution_rate() != 0)
        UBOOT_DEBUG("Check panel resolution and rate failure\n");

    gfx_init();
    if(is_gop_init == 0)
    {
        ret = gop_init(logo_gop_idx);
        if(ret < 0)
        {
            UBOOT_ERROR("Error: gop_init function execute failure\n");
            return -1;
        }

        if(separate_window == true && (dst_info.u32Width == RESOLUTION_WIDTH_4K && dst_info.u32Height == RESOLUTION_HEIGHT_4K) && logo_gop_idx > 0)
        {
            ret = gop_init(logo_gop_idx - 1);
            if(ret < 0)
            {
                UBOOT_ERROR("Error: gop_init function execute failure\n");
                return -1;
            }
        }

        is_gop_init = 1;
    }

    flush_dcache_all();
    gfx_blt(&rect, &src_info, &dst_info);
    src_info.u32Pitch = src_info.u32Pitch >> 1;
    dst_info.u32Pitch = dst_info.u32Pitch >> 1;

    ret = parse_dt("/video_out", integer_dt_parser, (void*)&mirror_mode, "PanelMirrorMode");
    if(ret < 0)
    {
        UBOOT_ERROR("Error: PanelMirrorMode information parse error in DTS\n");
        return -1;
    }

    if(GOP_V_H_MIRROR == mirror_mode)
    {
        UBOOT_INFO("<<set_mirror V & H ON!!>> \n");
        MApi_GOP_GWIN_SetVMirror(TRUE);
        if(panel_mirror_mode == FALSE)
        {
            MApi_GOP_GWIN_SetHMirror(TRUE);
        }
        else
        {
            UBOOT_INFO("<<Skip H ON!!>> \n");
        }
    }
    else if(GOP_V_MIRROR == mirror_mode)
    {
        UBOOT_INFO("<<set_mirror V ON!!>> \n");
        MApi_GOP_GWIN_SetVMirror(TRUE);
    }
    else if(GOP_H_MIRROR == mirror_mode)
    {
        if(panel_mirror_mode == FALSE)
        {
            UBOOT_INFO("<<set_mirror H ON!!>> \n");
            MApi_GOP_GWIN_SetHMirror(TRUE);
        }
        else
        {
            UBOOT_INFO("<<MOD H_Mirror Enable !!>> \n");
        }
    }

    gop_show(logo_gop_idx, &dst_info, config.horizontal_stretch, config.vertical_stretch, config.disp_x, config.disp_y);
    if(separate_window == true && (dst_info.u32Width == RESOLUTION_WIDTH_4K && dst_info.u32Height == RESOLUTION_HEIGHT_4K) && logo_gop_idx > 0)
        gop_show_seperate(logo_gop_idx - 1, &dst_info, config.horizontal_stretch, config.vertical_stretch, config.disp_x, config.disp_y);

    UBOOT_TRACE("OK\n");
    return 0;
}

int mtk_gop_show_logo(logo_disp_config_params config, struct bootlogo_info *logo_info, unsigned int logo_output_buf, unsigned int logo_output_buf_size)
{
    int ret = -1;
    GFX_BufferInfo src_info, dst_info;
    GFX_DrawRect rect;
    bool horizontal_stretch=config.horizontal_stretch;
    bool vertical_stretch=config.vertical_stretch;
    size_t addr=0;

    UBOOT_TRACE("IN\n");

    src_info.u32Addr = logo_output_buf;
    UBOOT_DEBUG("src_info.u32Addr=0x%x\n",(unsigned int)src_info.u32Addr);

    if(strncmp(logo_info->type, LOGO_FORMAT_RAW, LOGO_FORMAT_RAW_STRING_SIZE) == 0)
    {
        src_info.u32ColorFmt = logo_info->color_format;
        src_info.u32Width = (unsigned int)logo_info->width;
        src_info.u32Height = (unsigned int)logo_info->height;
        src_info.u32Pitch = (unsigned int)(logo_info->width << 1);
        dst_info.u32ColorFmt = logo_info->color_format;
        rect.srcblk.x = 0;
        rect.srcblk.y = 0;
        rect.srcblk.width = logo_info->width;
        rect.srcblk.height = logo_info->height;
        rect.dstblk.x = 0;
        rect.dstblk.y = 0;
    }
    else if(strncmp(logo_info->type, LOGO_FORMAT_JPEG, LOGO_FORMAT_JPEG_STRING_SIZE) == 0)
    {
        src_info.u32ColorFmt = mtk_jpd_get_color_format();
        src_info.u32Width = mtk_jpd_get_alignment_width();
        src_info.u32Height = mtk_jpd_get_alignment_height();
        src_info.u32Pitch = mtk_jpd_get_alignment_pitch()<<1;
        dst_info.u32ColorFmt = mtk_jpd_get_color_format();
        rect.srcblk.x = 0;
        rect.srcblk.y = 0;
        rect.srcblk.width = mtk_jpd_get_alignment_width();
        rect.srcblk.height = mtk_jpd_get_alignment_height();
        rect.dstblk.x = 0;
        rect.dstblk.y = 0;
    }
    else
    {
        UBOOT_ERROR("No boot logo information, show logo command execute failure.\n");
        return -1;
    }

    UBOOT_DEBUG("src_info.u32ColorFmt=0x%x\n",(unsigned int)src_info.u32ColorFmt);
    UBOOT_DEBUG("src_info.u32Width=0x%x\n",(unsigned int)src_info.u32Width);
    UBOOT_DEBUG("src_info.u32Height=0x%x\n",(unsigned int)src_info.u32Height);
    UBOOT_DEBUG("src_info.u32Pitch=0x%x\n",(unsigned int)src_info.u32Pitch);
    UBOOT_DEBUG("dst_info.u32ColorFmt=0x%x\n",(unsigned int)dst_info.u32ColorFmt);
    UBOOT_DEBUG("rect.srcblk.x=0x%x\n",(unsigned int)rect.srcblk.x);
    UBOOT_DEBUG("rect.srcblk.y=0x%x\n",(unsigned int)rect.srcblk.y);
    UBOOT_DEBUG("rect.srcblk.width=0x%x\n",(unsigned int)rect.srcblk.width);
    UBOOT_DEBUG("rect.srcblk.height=0x%x\n",(unsigned int)rect.srcblk.height);
    UBOOT_DEBUG("rect.dstblk.x=0x%x\n",(unsigned int)rect.dstblk.x);
    UBOOT_DEBUG("rect.dstblk.y=0x%x\n",(unsigned int)rect.dstblk.y);

    if( src_info.u32Width > mtk_get_current_panel_width())
    {
        rect.dstblk.width = (mtk_get_current_panel_width()&0xFFFFF0)/*u16Pitch*/;
        dst_info.u32Width = (unsigned int)(mtk_get_current_panel_width()&0xFFFFF0);
        dst_info.u32Pitch = get_dst_pitch((mtk_get_current_panel_width()&0xFFFFF0), dst_info.u32ColorFmt);
    }
    else
    {
        rect.dstblk.width = ((src_info.u32Width+15)&0xFFFFF0)/*u16Pitch*/;
        dst_info.u32Width = (unsigned int)((src_info.u32Width+15)&0xFFFFF0);
        dst_info.u32Pitch = get_dst_pitch(((src_info.u32Pitch+15)&0xFFFFF0), dst_info.u32ColorFmt);
    }
    UBOOT_DEBUG("rect.dstblk.width=0x%x\n",(unsigned int)rect.dstblk.width);
    UBOOT_DEBUG("dst_info.u32Width=0x%x\n",(unsigned int)dst_info.u32Width);
    UBOOT_DEBUG("dst_info.u32Pitch=0x%x\n",(unsigned int)dst_info.u32Pitch);

    if( src_info.u32Height > mtk_get_current_panel_height())
    {
        rect.dstblk.height = mtk_get_current_panel_height();
        dst_info.u32Height = mtk_get_current_panel_height();
    }
    else
    {
        rect.dstblk.height = src_info.u32Height;
        dst_info.u32Height = (unsigned int)src_info.u32Height;
    }
    UBOOT_DEBUG("rect.dstblk.height=0x%x\n",(unsigned int)rect.dstblk.height);
    UBOOT_DEBUG("dst_info.u32Height=0x%x\n",(unsigned int)dst_info.u32Height);

    ret = gop_get_logo_addrs(&addr, &dst_info, logo_output_buf_size);
    if (ret != 0)
    {
        UBOOT_ERROR("get bootlogo addr error\n");
        return -1;
    }
    dst_info.u32Addr = (MS_PHYADDR)addr;

    UBOOT_DEBUG("dst_info.u32Addr=0x%x\n",(unsigned int)dst_info.u32Addr);

    // initilize the GOP of Subtitle instead of UI;
    // otherwise, then bootlogo will be closed unexpectedly,
    // because UI is initilalized during booting up
    gop_display_logo(&rect,&src_info,&dst_info,horizontal_stretch,vertical_stretch,config.disp_x,config.disp_y);
    UBOOT_TRACE("OK\n");
    return 0;
}

unsigned char mtk_get_logo_miu_idx(void)
{
    return logo_miu_idx;
}

static char _mtk_get_font_info(osd_font_info *p_font_info,u8 using_font )
{
    if(using_font > FONT_MAX || FONT_MAX < 0 )
    {
        UBOOT_ERROR("Error: This font is out of range !\n");
        return false;
    }
    if(p_font_info == NULL)
    {
        UBOOT_ERROR("Error: NULL Pointer \n");
        return false;
    }

    p_font_info->font_width= font_size_table[using_font][0];
    p_font_info->font_height= font_size_table[using_font][1];
    p_font_info->space_width= font_size_table[using_font][2];
    p_font_info->interval_width= font_size_table[using_font][3];
    p_font_info->p_char_table = (unsigned char *)char_table[using_font];
    p_font_info->p_font_width_table = (unsigned char *)font_width_table[using_font];

    return true;
}

static size_t _wstrlen(const u8 * s)
{
    const u8 *sc;

    for (sc = s; *sc != '\0' && *sc + 1 != '\0'; sc = sc + 2)
        /* nothing */;
    return sc - s;
}

static char _parse_argv(char* argv, u8* outputbuffer, u32 u32outputbuffersize)
{
    char is_asc = -1;
    u16 id;
    u32 hexmagic;
    u32 decmagic;
    u8 *c_str = NULL;
    u16 k, j, u16_len;
    int snprintf_len = 0;
    if (argv == NULL || outputbuffer == NULL)
    {
        return -1;
    }
    hexmagic = simple_strtoul(argv, NULL, 16);
    decmagic = simple_strtoul(argv, NULL, 10);
    if ((hexmagic & 0xffff) == ID_MAGIC || (decmagic & 0xffff) == ID_MAGIC)
    {
        if ((hexmagic & 0xffff) == ID_MAGIC)
        {
            id = (hexmagic >> 16) & 0xffff;
        }
        else
        {
            id = (decmagic >> 16) & 0xffff;
        }
        c_str = osd_get_string(id);
        if (c_str == NULL)
        {
            return -1;
        }
        if (get_language_id() == ENGLISH)
        {
            k = 1;
            j = 0;
            while (((c_str[j] | (c_str[j + 1] << 8)) < 0xa0) && ((c_str[j] | (c_str[j + 1] << 8)) > 0x00))
            {
                c_str[k] = c_str[2 * k];
                k += 1;
                j += 2;
            }
            c_str[k - 1] = '\0';
            snprintf_len = snprintf((char*)outputbuffer, u32outputbuffersize, "%s", c_str);
            if(snprintf_len < 0)
                UBOOT_DEBUG("_parse_argv snprintf failure.\n");
            is_asc = ASC;
        }
        else
        {
            u16_len = _wstrlen(c_str);
            for (k = 0; k < u16_len; k++)
            {
                outputbuffer[k] = c_str[k];
            }
            is_asc = UNICODE;
        }
    }
    else
    {
        snprintf_len = snprintf((char*)outputbuffer, u32outputbuffersize, "%s ", argv);
        if(snprintf_len < 0)
           UBOOT_DEBUG("_parse_argv snprintf failure.\n");
        is_asc = ASC;
    }
    return is_asc;

}

static s16 _find_asc_char_index(const char* p_str_text)
{
    u16 u16_len = strlen(ASC_CHAR_TABLE);
    s16 idx = 0;
    for (; idx < u16_len; ++idx)
    {
        if (ASC_CHAR_TABLE[idx] == p_str_text[0])
        {
            return idx;
        }
    }

    return -1;
}

static s16 _find_uni_char_index(const u8* p_str_text)
{
    u16 u16_len = _wstrlen(UNI_CHAR_TABLE);
    s16 idx = 0;
    for (; idx < u16_len; idx=idx+2)
    {
        if (UNI_CHAR_TABLE[idx] == p_str_text[0] && UNI_CHAR_TABLE[idx+1] == p_str_text[1])
        {
            return idx >> 1;
        }
    }

    return -1;
}

static u16 _get_asc_str_text_total_width(const char* p_str_text)
{
    u16 u16_total = 0;
    u16 u16_len = strlen(p_str_text);
    u16 i = 0;
    for (; i < u16_len; ++i)
    {
        s16 s16_index = _find_asc_char_index(&p_str_text[i]);
        u16_total += ((s16_index < 0) ? font_info.space_width: (font_info.p_font_width_table[s16_index] + font_info.interval_width));
    }

    return (u16_total > font_info.interval_width)?(u16_total - font_info.interval_width):0;
}

static u16 _get_uni_str_text_total_width(const u8* p_str_text)
{
    u16 u16_total = 0;
    u16 u16_len = _wstrlen(p_str_text) >> 1;
    u16 i = 0;
    for (; i < u16_len; ++i)
    {
        s16 s16_index = _find_uni_char_index(&p_str_text[i]);
        u16_total += ((s16_index < 0) ? SPACE_WIDTH : (UCHAR_WIDTH_TABLE[s16_index] + EACH_CHAR_INTERVAL));
    }

    return u16_total - EACH_CHAR_INTERVAL;
}

static unsigned char _draw_asc_char(u16 x, u16 y, u16 W, GFX_RgbColor color)
{
    u16 h = 0;
    u16 i = 0;
    unsigned char cast_font[8]={0};
    y+=(MAX_CHAR_HEIGHT-font_info.font_height);
    for (; h < font_info.font_height; ++h)
    {

        GFX_Point point;
        point.y = h + y;

        u16 w = 0;
		for(i=0;i<font_info.font_height/8;i++)
		{
			if(using_font == FONT32X32)
				{
					cast_font[i] = font32X32[h*4+i];
				}
			if(using_font  ==FONT16X16)
				{
					cast_font[i] = font16X16[h*2+i];
				}
		}
	     i=0;
        for (; w < W; ++w)
        {
            //Draw one ponit
            i = w/8;
	        cast_font[0] = cast_font[i];
            if (cast_font[0]  & u8font_bit_mask[w-i*8])
            {
                point.x = w + x;
                mtk_draw_pixel(point, color);
            }
        }
    }

    return TRUE;
}

static unsigned char _draw_uni_char(u16 x, u16 y, u16 W, GFX_RgbColor color)
{
    u16 h = 0;
    y+=(MAX_CHAR_HEIGHT-ONE_CHAR_HEIGHT);
    for (; h < ONE_CHAR_HEIGHT; ++h)
    {
        u16 u16index = 3 * h;
        u32 u32line = (UNI_CHAR_ARRAY[u16index] << 24) | (UNI_CHAR_ARRAY[u16index + 1] << 16) | (UNI_CHAR_ARRAY[u16index + 2] << 8);

        if (0 == u32line)
        {
            continue;
        }

        GFX_Point point;
        point.y = h + y;

        u16 w = 0;
        for (; w < W; ++w)
        {
            //Draw one ponit
            if (u32line & u32font_bit_mask[w])
            {
                point.x = w + x;
                mtk_draw_pixel(point, color);
            }
        }
    }

    return TRUE;
}

static unsigned char _draw_one_asc_char_by_index(s16 s16index, u16 x, u16 y, GFX_RgbColor color)
{
    if (s16index < 0)
    {
        return FALSE;
    }
    if(using_font == 0)
    {
        memcpy(font32X32, &(font_info.p_char_table[s16index * 128]), 128);
        _draw_asc_char(x, y, font_info.p_font_width_table[s16index], color);
    }
    if(using_font == 1)
    {
            memcpy(font16X16, &(font_info.p_char_table[s16index * 32]), 32);
            _draw_asc_char(x, y,font_info.p_font_width_table[s16index], color);
    }
    return TRUE;
}

static unsigned char _draw_one_uni_char_by_index(s16 s16index, u16 x, u16 y, GFX_RgbColor color)
{
    if (s16index < 0)
    {
        return FALSE;
    }

    memcpy(UNI_CHAR_ARRAY, &UNI_FONT_TABLE[s16index * 72], 72);
    _draw_uni_char(x, y, UCHAR_WIDTH_TABLE[s16index], color);
    return TRUE;
}

static unsigned char _draw_asc_str_text(const char* p_str_text, u16 x, u16 y, GFX_RgbColor color, text_attrib e_text_attrib)
{
    switch (e_text_attrib)
    {
        case TEXT_ALIGN_MIDDLE:
        {
            GOP_GwinFBAttr s_fb_info;
            MApi_GOP_GWIN_GetFBInfo(u8osd_layer_fb_id, &s_fb_info);
            x=(g_u16record_osd_create_w-text_to_tal_width)>>1;
            x=x+g_u16record_osd_create_x;
            break;
        }

        case TEXT_ALIGN_RIGHT:
             x -= text_to_tal_width;
            break;

        case TEXT_ALIGN_LEFT:
        default:
            break;
    }

    u16 u16_offset = line_offset;
    u16 u16_len = strlen(p_str_text);
    u16 i = 0;

    for(; i < u16_len; ++i)
    {
        x += u16_offset;
        s16 s16_index = _find_asc_char_index(&p_str_text[i]);
        _draw_one_asc_char_by_index(s16_index, x, y, color);
        u16_offset = (s16_index < 0) ? font_info.space_width: (font_info.p_font_width_table[s16_index] + font_info.interval_width);
        line_offset += u16_offset;

    }

    return TRUE;
}

static unsigned char _draw_uni_str_text(const u8* p_str_text, u16 x, u16 y, GFX_RgbColor color, text_attrib e_text_attrib)
{
    switch (e_text_attrib)
    {
        case TEXT_ALIGN_MIDDLE:
        {
            GOP_GwinFBAttr s_fb_info;
            MApi_GOP_GWIN_GetFBInfo(u8osd_layer_fb_id, &s_fb_info);
            x=(g_u16record_osd_create_w-text_to_tal_width)>>1;
            x=x+g_u16record_osd_create_x;
            break;
        }

        case TEXT_ALIGN_RIGHT:
            x -= text_to_tal_width;
            break;

        case TEXT_ALIGN_LEFT:
        default:
            break;
    }

    u16 u16_offset = line_offset;
    u16 u16_len = _wstrlen(p_str_text);
    u16 i = 0;

    for(; i < u16_len; i += 2)
    {
        x += u16_offset;
        s16 s16_index = _find_uni_char_index(&p_str_text[i]);
        _draw_one_uni_char_by_index(s16_index, x, y, color);
        u16_offset = (s16_index < 0) ? SPACE_WIDTH : (UCHAR_WIDTH_TABLE[s16_index] + EACH_CHAR_INTERVAL);
        line_offset += u16_offset;
    }

    return TRUE;
}

void mtk_osd_create(u8 u8logo_gop_index, GFX_Block* p_blk, size_t gop_buffer)
{
    u32 i=0;
    u16* u16temp_addr = NULL;
    size_t u16temp_addr_phyical=0;
    GFX_BufferInfo dst_info;
    unsigned char gbPnlModMirrorMode = FALSE;
    int ret;

    gfx_init();
    gop_init(u8logo_gop_index);

    g_u16record_osd_create_w = p_blk->width;
    g_u16record_osd_create_h = p_blk->height;
    g_u16record_osd_create_x = p_blk->x;
    g_u16record_osd_create_y = p_blk->y;
    //p_blk->width = g_IPanel.Width();
    //p_blk->height = g_IPanel.Height();

    //set gop canvas address
    g_u32canvas_buffer=gop_buffer+(p_blk->width* p_blk->height*2);

    //init  gop canvas address
    u16temp_addr = (u16*)(unsigned long)g_u32canvas_buffer;
    for (i = 0; i < CANVAS_WIDTH * CANVAS_HEIGHT; ++i)
    {
        *(u16temp_addr++) = 0x8000;
    }
    //init gop display address
    u16temp_addr = (u16*)(unsigned long)gop_buffer;
    for (i = 0; i < p_blk->width *  p_blk->height; ++i)
    {
            *(u16temp_addr++) = 0x8000;
    }
    flush_dcache_all();

    //convert VA to PA for HW use
    u16temp_addr_phyical = g_u32canvas_buffer;
    g_u32canvas_buffer=virtual_addr_to_phyical_addr(u16temp_addr_phyical);
    u16temp_addr_phyical = gop_buffer;
    gop_buffer=virtual_addr_to_phyical_addr(u16temp_addr_phyical);

    dst_info.u32Addr = gop_buffer;
    dst_info.u32Width = (p_blk->width&0xFFFFF0);
    dst_info.u32Height =  p_blk->height;
    dst_info.u32ColorFmt = GFX_FMT_YUV422;
    dst_info.u32Pitch = ((p_blk->width&0xFFFFF0)<<1);

    u16osd_w = p_blk->width;
    u16osd_h = p_blk->height;

    // start - draw background all black
    u16 x = g_u16record_osd_create_x;
    u16 y = g_u16record_osd_create_y;
    u16 w = g_u16record_osd_create_w;
    u16 h = g_u16record_osd_create_h;
    u32 t_color = 0x80000000;
    GFX_RgbColor color;

    color.a = t_color >> 24;
    color.r = (t_color >> 16) & 0xFF;
    color.g = (t_color >> 8) & 0xFF;
    color.b = t_color & 0xFF;
    GFX_Block dst_blk = {x, y, w, h};

    mtk_draw_rect(&dst_blk, color);
    // end - draw background all black

    st_sys_misc_setting misc_setting;
    memset(&misc_setting, 0, sizeof(misc_setting));
#if (CONFIG_LOGO_STORE_IN_MBOOT)
    GetOSDMirrorMode(&misc_setting);
#else
    ret = parse_dt("/video_out",misc_dt_parser,(void*)&misc_setting,NULL);
    if(ret != 0)
        UBOOT_ERROR("misc_setting data can not get correct from dts!\n");
#endif

    int mirror_mode=misc_setting.m_u8MirrorMode;
    if(1==mirror_mode)
    {
        UBOOT_DEBUG("<<set_mirror V & H ON!!>> \n");
        MApi_GOP_GWIN_SetVMirror(TRUE);
        if(gbPnlModMirrorMode==FALSE)
        {
            MApi_GOP_GWIN_SetHMirror(TRUE);
        }
        else
        {
          UBOOT_DEBUG("<<Skip H ON!!>> \n");
        }
    }
    else if(2==mirror_mode)
    {
        UBOOT_DEBUG("<<set_mirror V ON!!>> \n");
        MApi_GOP_GWIN_SetVMirror(TRUE);
    }
    else if(3==mirror_mode)
    {
        if(gbPnlModMirrorMode==FALSE)
        {
            UBOOT_DEBUG("<<set_mirror H ON!!>> \n");
            MApi_GOP_GWIN_SetHMirror(TRUE);
        }
        else
        {
            UBOOT_DEBUG("<<MOD H_Mirror Enable !!>> \n");
        }
    }
    gop_show(u8logo_gop_index, &dst_info,TRUE,TRUE,p_blk->x,p_blk->y);
}

void mtk_draw_rect(GFX_Block* p_blk, GFX_RgbColor color)
{
    GFX_Point v0 = {0, 0};
    GFX_Point v1 = {CANVAS_WIDTH, CANVAS_HEIGHT};
    MApi_GFX_SetClip(&v0, &v1);
    GFX_RgbColor tmpcolor;
    GFX_BufferInfo g_fx_dst_buf;

    g_fx_dst_buf.u32ColorFmt = (GFX_Buffer_Format)E_MS_FMT_YUV422;
    g_fx_dst_buf.u32Addr = g_u32canvas_buffer;
    g_fx_dst_buf.u32Width = CANVAS_WIDTH;
    g_fx_dst_buf.u32Height = CANVAS_HEIGHT;
    g_fx_dst_buf.u32Pitch = CANVAS_WIDTH << 1;
    MApi_GFX_SetDstBufferInfo(&g_fx_dst_buf, 0);

    GFX_RectFillInfo g_fx_fill_block;
    g_fx_fill_block.dstBlock.x = p_blk->x;
    g_fx_fill_block.dstBlock.y = p_blk->y;
    g_fx_fill_block.dstBlock.height = p_blk->height;
    g_fx_fill_block.dstBlock.width = p_blk->width;

    g_fx_fill_block.fmt = (GFX_Buffer_Format)E_MS_FMT_YUV422;

    if(g_fx_dst_buf.u32ColorFmt == GFX_FMT_YUV422)
    {
        tmpcolor.r = (u8)( 0.439*color.r-0.368*color.g-0.071*color.b+128);
        tmpcolor.g = (u8)( 0.257*color.r+0.504*color.g+0.098*color.b+16);
        tmpcolor.b = (u8)(-0.148*color.r-0.291*color.g+0.439*color.b+128);
		tmpcolor.a = 0xFF;
    }
    else
    {
		tmpcolor.r = color.r;
        tmpcolor.g = color.g;
        tmpcolor.b = color.b;
		tmpcolor.a = 0xFF;
    }
    memcpy(&g_fx_fill_block.colorRange.color_s,&tmpcolor,sizeof(tmpcolor));
    memcpy(&g_fx_fill_block.colorRange.color_e,&tmpcolor,sizeof(tmpcolor));

    g_fx_fill_block.flag = GFXRECT_FLAG_COLOR_CONSTANT;

    MApi_GFX_BeginDraw();
    MApi_GFX_RectFill(&g_fx_fill_block);
    MApi_GFX_EndDraw();
}

void mtk_select_font(u8 font_choose)
{
	using_font = font_choose;
}

void mtk_draw_string_text(const char* p_str_text, u16 x, u16 y, GFX_RgbColor color, text_attrib text_attrib)
{
    char ret = -1;
    u16 i;
    u8 str_text[100] = {0};
    char word[100] = {0};
    const char* p_str=p_str_text;

    line_offset = 0;
    text_to_tal_width = 0;
    _mtk_get_font_info(&font_info,  using_font);
    while (*p_str != '\0')
    {
        for (i = 0; (*p_str != ' ' && *p_str != '\0'); i++)
        {
            word[i] = *p_str++;
        }
        word[i] = '\0';
        p_str++;

        ret = _parse_argv(word, str_text, sizeof(str_text));
        if (ASC == ret)
        {
            text_to_tal_width += _get_asc_str_text_total_width((const char*)str_text);
        }
        else if (UNICODE == ret)
        {
            text_to_tal_width += _get_uni_str_text_total_width(str_text);
        }
        else
        {
            return;
        }
    }

    p_str = p_str_text;
    while (*p_str != '\0')
    {
        for (i = 0; (*p_str != ' ' && *p_str != '\0'); i++)
        {
            word[i] = *p_str++;
        }
        word[i] = '\0';
        p_str++;

        ret = _parse_argv(word, str_text, sizeof(str_text));
        if (ASC == ret)
        {
            _draw_asc_str_text((const char*)str_text, x, y, color, text_attrib);
        }
        else if (UNICODE == ret)
        {
            _draw_uni_str_text(str_text, x, y, color, text_attrib);
        }
        else
        {
            return;
        }
    }
}

void mtk_draw_progress(u16 x, u16 y, GFX_RgbColor fcolor, u8 per)
{
    if (per > 100)
    {
        per = 100;
    }

    u16 u16_start_x = x;
    u16 u16_total = (PROGRESS_NUM * PROGRESS_WIDTH) * per / 100;
    u16 u16_count = u16_total / PROGRESS_WIDTH;
    int snprintf_len = 0;

    //memset(ASC_CHAR_ARRAY, 0xFF, 96);
	memset(font32X32, 0xFF, 128);

    u16 i = 0;
    for (; i < u16_count; ++i)
    {
        _draw_asc_char(u16_start_x, y, PROGRESS_WIDTH, fcolor);
        u16_start_x += (PROGRESS_WIDTH + PROGRESS_INTERVAL);
    }

    u16 u16_left = u16_total % PROGRESS_WIDTH;
    if (u16_left != 0)
    {
        _draw_asc_char(u16_start_x, y, u16_left, fcolor);
        u16_start_x += (u16_left + PROGRESS_INTERVAL);
    }

    char c_str[5] = {0};
    memset(c_str, 0, sizeof(c_str));
    snprintf_len = snprintf(c_str, sizeof(c_str), "%d%%", per);
    if(snprintf_len < 0 || (unsigned)snprintf_len >= sizeof(c_str))
    {
        UBOOT_ERROR("The array size is too small(%lu), snprintf fail '%d%%'.\n", sizeof(c_str), per);
        return;
    }

    line_offset = 0;
    text_to_tal_width = 0;
    text_to_tal_width = _get_asc_str_text_total_width(c_str);
    _draw_asc_str_text(c_str, x + PROGRESS_NUM * (PROGRESS_WIDTH + PROGRESS_INTERVAL), y, fcolor, TEXT_ALIGN_LEFT);
}

void mtk_flush_canvas_to_screen(void)
{
    if (0xFF == u8osd_layer_fb_id)
    {
        return;
    }
    GOP_GwinFBAttr s_fb_info;

    MApi_GOP_GWIN_GetFBInfo(u8osd_layer_fb_id, &s_fb_info);
    GFX_Point v0 = {s_fb_info.x0, s_fb_info.y0};
    GFX_Point v1 = {s_fb_info.x1, s_fb_info.y1};
    MApi_GFX_SetClip(&v0, &v1);

    GFX_BufferInfo g_fx_dst_buf;
    g_fx_dst_buf.u32ColorFmt = (GFX_Buffer_Format)(s_fb_info.fbFmt & 0xFF);
    g_fx_dst_buf.u32Addr = s_fb_info.addr;
    g_fx_dst_buf.u32Width = s_fb_info.width;
    g_fx_dst_buf.u32Height = s_fb_info.height;
    g_fx_dst_buf.u32Pitch = s_fb_info.pitch;
    MApi_GFX_SetDstBufferInfo(&g_fx_dst_buf, 0);

    GFX_BufferInfo g_fx_src_buf;
    g_fx_src_buf.u32ColorFmt = (GFX_Buffer_Format)E_MS_FMT_YUV422;
    g_fx_src_buf.u32Addr = g_u32canvas_buffer;
    g_fx_src_buf.u32Width = CANVAS_WIDTH;
    g_fx_src_buf.u32Height = CANVAS_HEIGHT;
    g_fx_src_buf.u32Pitch = CANVAS_WIDTH << 1;
    MApi_GFX_SetSrcBufferInfo(&g_fx_src_buf, 0);

    GFX_DrawRect g_fx_bitblt_info;
    g_fx_bitblt_info.dstblk.height = s_fb_info.height;
    g_fx_bitblt_info.dstblk.width = s_fb_info.width;
    g_fx_bitblt_info.dstblk.x = 0;
    g_fx_bitblt_info.dstblk.y = 0;

    g_fx_bitblt_info.srcblk.height = CANVAS_HEIGHT;
    g_fx_bitblt_info.srcblk.width = CANVAS_WIDTH;
    g_fx_bitblt_info.srcblk.x = 0;
    g_fx_bitblt_info.srcblk.y = 0;

    MApi_GFX_BitBlt(&g_fx_bitblt_info, GFXDRAW_FLAG_SCALE);
    MApi_GFX_FlushQueue();

}

void mtk_draw_pixel(GFX_Point p, GFX_RgbColor color)
{
    GFX_Block dst_blk = {p.x, p.y, 1, 1};
    mtk_draw_rect(&dst_blk, color);
}
