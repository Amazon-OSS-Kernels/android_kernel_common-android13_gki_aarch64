// SPDX-License-Identifier: (GPL-2.0 OR BSD-3-Clause)
/*
 * Copyright (c) 2020 MediaTek Inc.
 */

#include "MsCommon.h"
#ifndef MSOS_TYPE_LINUX_KERNEL
#include <string.h>
#endif
#include <vsprintf.h>
#include <linux/io.h>
#include "MsTypes.h"
#include "halGOP.h"
#include "regGOP.h"
#include "halCHIP.h"
#include "drvSYS.h"
#include <dts_parser.h>
#include <utility.h>
#ifdef CONFIG_DATA_SEPARATION
#include <mtk_dataindex.h>
#endif

//------------------------------------------------------------------------------
//  Driver Compiler Options
//------------------------------------------------------------------------------
#define HAL_GOP_DEBUGINFO(x)   //x

//------------------------------------------------------------------------------
//  Local Defines
//------------------------------------------------------------------------------

// Debug Log
#include "ULog.h"
#include <debug_impl.h>
MS_U32 u32GOPDbgLevel_hal = 0;
MS_U32 u32ChipVersion = 0;
MS_U8 GOPCursor = INVAILD_GOP_NUM;

#ifdef CONFIG_GOP_DEBUG_LEVEL
// Debug Logs, level form low(INFO) to high(FATAL, always show)
// Function information, ex function entry
#define GOP_H_INFO(x, args...) {UBOOT_INFO(x, ##args);}
// Warning, illegal paramter but can be self fixed in functions
#define GOP_H_WARN(x, args...) {UBOOT_INFO(x, ##args);}
//  Need debug, illegal paramter.
#define GOP_H_DBUG(x, args...) {UBOOT_DEBUG(x, ##args);}
// Error, function will be terminated but system not crash
#define GOP_H_ERR(x, args...) {UBOOT_ERROR(x, ##args);}
// Critical, system crash. (ex. assert)
#define GOP_H_FATAL(x, args...) {UBOOT_ERROR(x, ##args);}
#else
#define GOP_H_INFO(x, args...)
// Warning, illegal paramter but can be self fixed in functions
#define GOP_H_WARN(x, args...)
//  Need debug, illegal paramter.
#define GOP_H_DBUG(x, args...)
// Error, function will be terminated but system not crash
#define GOP_H_ERR(x, args...)
// Critical, system crash. (ex. assert)
#define GOP_H_FATAL(x, args...)
#endif

// OSD PQ
#define PQBIN_SECTION               "PQ_OSD"
#define PQBIN_KEY_MAIN              "m_pBinOsdPqMainMboot"
#define PQBIN_KEY_EX                "m_pBinOsdPqExMboot"
#define PQBIN_HEADER                "IP_COMM_HEADER  "

#define PQBIN_IP_NUM_OFST           1
#define PQBIN_IP_TBL_OFST           3

#define PQBIN_IP_TBL_SIZE           16
#define PQBIN_IP_TBL_REG_NUM_OFST   2
#define PQBIN_IP_TBL_PTR_OFST       8

#define PQBIN_IP_DATA_OFST          16
#define PQBIN_REG_DATA_SIZE         5
#define PQBIN_REG_BANK_OFST         0
#define PQBIN_REG_BKOFT_OFST        2
#define PQBIN_REG_MASK_OFST         3
#define PQBIN_REG_VALUE_OFST        4

#define READ_BUF_CHECK(buf, bufsize, start, size) \
    do { \
        if (start + size > bufsize) \
        { \
            GOP_H_ERR("read exceeds buffer size\n"); \
            free(buf); \
            return; \
        } \
    } while (0);

//------------------------------------------------------------------------------
//  Local Var
//------------------------------------------------------------------------------
MS_BOOL bIsMuxVaildToGopDst[MAX_GOP_MUX][MAX_DRV_GOP_DST_SUPPORT] =
{
    /*IP0,      IP0_SUB,  MIXER2VE, OP0,         VOP,   IP1,       IP1_SUB, MIXER2OP*/
    {TRUE,    FALSE, FALSE,    TRUE,    TRUE, FALSE, FALSE,  FALSE},         /*All Gop Dst case is vaild or FALSE for mux 0 */
    {TRUE,    FALSE, FALSE,    FALSE,   TRUE, FALSE, FALSE,  FALSE},        /*All Gop Dst case is vaild or FALSE for mux 1 */
    {TRUE,    FALSE, FALSE,    TRUE,    TRUE, FALSE, FALSE,  FALSE},         /*All Gop Dst case is vaild or FALSE for mux 0 */
    {TRUE,    FALSE, FALSE,    TRUE,    TRUE, FALSE, FALSE,  FALSE},         /*All Gop Dst case is vaild or FALSE for mux 0 */
};
#ifdef GOP_CMDQ_ENABLE
extern MS_U16 u16MIUSelect[SHARED_GOP_MAX_COUNT];
extern MS_U8 bMIUSelect[SHARED_GOP_MAX_COUNT];
#endif
static MS_BOOL _bHalCSCSelect[SHARED_GOP_MAX_COUNT]={FALSE};
static ST_GOP_CSC_TABLE _stHalCSCTbl[SHARED_GOP_MAX_COUNT];

MS_U16 gu16GopR2yBt601[GOP_CSC_ELEMENT_NUM] =
{
    0xE010, 0x01C0, 0x1E88,
    0x1FB7, 0x0106, 0x0203,
    0x0064, 0x1F69, 0x1ED7,0x01C0
};
MS_U16 gu16GopY2rBt601[GOP_CSC_ELEMENT_NUM] =
{
    0x001D, 0x0667, 0x04AC,
    0x0000, 0x1CBD, 0x04AC,
    0x1E6E, 0x0000, 0x04AC,0x0818
};
MS_U16 gu16GopY2rCSC[GOP_CSC_ELEMENT_NUM] =
{
    0x001D, 0x0667, 0x04AC,
    0x0000, 0x1CBD, 0x04AC,
    0x1E6E, 0x0000, 0x04AC,0x0818
};
MS_U16 gu16GopR2rCSC[GOP_CSC_ELEMENT_NUM] =
{
    0x0030, 0x0400, 0x0000,
    0x0000, 0x0000, 0x0400,
    0x0000, 0x0000, 0x0000,0x0400
};

MS_U16 gGopIdentity[10] = {
	0x000D, 0x0400, 0x0000,
	0x0000, 0x0000, 0x0400,
	0x0000, 0x0000, 0x0000, 0x0400
};

MS_U16 gGopLimitY2RFullBT709[GOP_CSC_ELEMENT_NUM] = {
	0x003D, 0x0731, 0x04AC,
	0x0000, 0x1DDD, 0x04AC,
	0x1F25, 0x0000, 0x04AC, 0x0879
};

MS_U16 gGopFullR2YLimitBT709[10] = {
	0xE030, 0x01C0, 0x1E69,
	0x1FD7, 0x00BA, 0x0273,
	0x003F, 0x1F99, 0x1EA6, 0x01C0
};

GOP_CHIP_PROPERTY gHalGopChipPro;

//------------------------------------------------------------------------------
//  Global Functions
//------------------------------------------------------------------------------
uint32_t __u4IO32GOPAccessFld(
	uint8_t write, uint32_t tmp32, uint32_t val32, uint32_t fld)
{
	uint32_t t = 0;

	switch (Fld_ac(fld)) {
	case AC_FULLB0:
	case AC_FULLB1:
	case AC_FULLB2:
	case AC_FULLB3:
		if (write == 1)
			t = (tmp32&(~((uint32_t)0xFF<<
				(8*(Fld_ac(fld)-AC_FULLB0))))) |
				((val32&0xFF)<<(8*(Fld_ac(fld)-AC_FULLB0)));
		else
			t = (tmp32&((uint32_t)0xFF<<
				(8*(Fld_ac(fld)-AC_FULLB0)))) >>
				(8*(Fld_ac(fld)-AC_FULLB0));
		break;
	case AC_FULLW10:
	case AC_FULLW21:
	case AC_FULLW32:
		if (write == 1)
			t = (tmp32&(~((uint32_t)0xFFFF<<
				(8*(Fld_ac(fld)-AC_FULLW10))))) |
				((val32&0xFFFF)<<(8*(Fld_ac(fld)-AC_FULLW10)));
		else
			t = (tmp32&(((uint32_t)0xFFFF<<
				(8*(Fld_ac(fld)-AC_FULLW10))))) >>
				(8*(Fld_ac(fld)-AC_FULLW10));
		break;
	case AC_FULLDW:
		t = val32;
		break;
	case AC_MSKB0:
	case AC_MSKB1:
	case AC_MSKB2:
	case AC_MSKB3:
		if (write == 1)
			t = (tmp32&(~(((((uint32_t)1<<Fld_wid(fld))-1)<<
				Fld_shift(fld))))) |
				(((val32&(((uint32_t)1<<Fld_wid(fld))-1))<<
				Fld_shift(fld)));
		else
			t = (tmp32&(((((uint32_t)1<<Fld_wid(fld))-1)<<
				Fld_shift(fld)))) >>
				Fld_shift(fld);
		break;
	case AC_MSKW10:
	case AC_MSKW21:
	case AC_MSKW32:
		if (write == 1)
			t = (tmp32&(~(((((uint32_t)1<<Fld_wid(fld))-1)<<
				Fld_shift(fld))))) |
				(((val32&(((uint32_t)1<<Fld_wid(fld))-1))<<
				Fld_shift(fld)));
		else
			t = (tmp32&(((((uint32_t)1<<Fld_wid(fld))-1)<<
				Fld_shift(fld)))) >>
				Fld_shift(fld);
		break;
	case AC_MSKDW:
		if (write == 1)
			t = (tmp32&(~(((((uint32_t)1<<Fld_wid(fld))-1)<<
				Fld_shift(fld))))) |
				(((val32&(((uint32_t)1<<Fld_wid(fld))-1))<<
				Fld_shift(fld)));
		else
			t = (tmp32&(((((uint32_t)1<<Fld_wid(fld))-1)<<
				Fld_shift(fld)))) >>
				Fld_shift(fld);
		break;
	default:
		break;
	}
	return t;
}

void HAL_GOP_Write2byte(
	uint32_t u32P_Addr,
	uint32_t u32Value)
{
	writel(u32Value, (volatile void *)((uintptr_t)(u32P_Addr + REG_BASE)));
}

void HAL_GOP_Write2bytemask(
	uint32_t u32P_Addr,
	uint32_t u32Value,
	uint32_t fld)
{
	uint32_t tmp = 0x00;

	tmp = __u4IO32GOPAccessFld(
				1,
				readl((const volatile void *)((uintptr_t)(u32P_Addr + REG_BASE))),
				u32Value,
				fld);
			writel(tmp, (volatile void *)((uintptr_t)(u32P_Addr + REG_BASE)));
}

void HAL_GOP_Writebyte(
    uint32_t u32P_Addr,
    uint8_t u8Value)
{
    writeb(u8Value, (volatile void *)((uintptr_t)(u32P_Addr + REG_BASE)));
}

uint32_t HAL_GOP_Read2bytemask(
	uint32_t u32P_Addr,
	uint32_t fld)
{
	uint32_t tmp;

	tmp = __u4IO32GOPAccessFld(
				0,
				readl((const volatile void *)((uintptr_t)(u32P_Addr + REG_BASE))),
				0,
				fld);
	return tmp;
}

uint32_t HAL_GOP_Read2byte(
	uint32_t u32P_Addr)
{
	return readl((const volatile void *)((uintptr_t)(u32P_Addr + REG_BASE)));
}

uint8_t HAL_GOP_Readbyte(
    uint32_t u32P_Addr)
{
    return readb((const volatile void *)((uintptr_t)(u32P_Addr + REG_BASE)));
}


MS_BOOL _HAL_GOP_SetTgen(MS_BOOL bIsByPassMode)
{
	char node_name[32]={0};
	MS_U32 u32Start = 0, u32Length = 0, length_needed = 0;

	if(bIsByPassMode == TRUE) {
		length_needed = snprintf(node_name, sizeof(node_name), "%s/%s", "/graphic_out", "loadfw-timing-info");
		if (length_needed < 0 || (unsigned) length_needed >= sizeof(node_name))
			GOP_H_ERR("Error: buffer too small\n");
	} else {
		length_needed = snprintf(node_name, sizeof(node_name), "%s/%s", "/graphic_out", "panel_info");
		if (length_needed < 0 || (unsigned) length_needed >= sizeof(node_name))
			GOP_H_ERR("Error: buffer too small\n");
	}

	if (parse_dt(node_name, integer_dt_parser, (void*)&u32Start, "hsync_start") != 0) {
		GOP_H_ERR("[%s][%d]parsing hsync_start error\n", __FUNCTION__, __LINE__);
		return FALSE;
	}
	HAL_GOP_Write2bytemask(REG_0080_GOPG_BKA4D9, u32Start, REG_0080_GOPG_BKA4D9_REG_TG_HS_ST);

	if (parse_dt(node_name, integer_dt_parser, (void*)&u32Length, "hsync_width") != 0) {
		GOP_H_ERR("[%s][%d]parsing hsync_width error\n", __FUNCTION__, __LINE__);
		return FALSE;
	}
	HAL_GOP_Write2bytemask(REG_0084_GOPG_BKA4D9, (u32Start+u32Length-1), REG_0084_GOPG_BKA4D9_REG_TG_HS_END);

	if (parse_dt(node_name, integer_dt_parser, (void*)&u32Start, "de_hstart") != 0) {
		GOP_H_ERR("[%s][%d]parsing de_hstart error\n", __FUNCTION__, __LINE__);
		return FALSE;
	}
	HAL_GOP_Write2bytemask(REG_0088_GOPG_BKA4D9, u32Start, REG_0088_GOPG_BKA4D9_REG_TG_HFDE_ST);

	if (parse_dt(node_name, integer_dt_parser, (void*)&u32Length, "resolution_width") != 0) {
		GOP_H_ERR("[%s][%d]parsing resolution_width error\n", __FUNCTION__, __LINE__);
		return FALSE;
	}
	HAL_GOP_Write2bytemask(REG_008C_GOPG_BKA4D9, (u32Start+u32Length-1), REG_008C_GOPG_BKA4D9_REG_TG_HFDE_END);

	if (parse_dt(node_name, integer_dt_parser, (void*)&u32Length, "typ_h_total") != 0) {
		GOP_H_ERR("[%s][%d]parsing max_h_total error\n", __FUNCTION__, __LINE__);
		return FALSE;
	}
	HAL_GOP_Write2bytemask(REG_0090_GOPG_BKA4D9, (u32Length-1), REG_0090_GOPG_BKA4D9_REG_TG_HTT);

	if (parse_dt(node_name, integer_dt_parser, (void*)&u32Start, "vsync_start") != 0) {
		GOP_H_ERR("[%s][%d]parsing vsync_start error\n", __FUNCTION__, __LINE__);
		return FALSE;
	}
	HAL_GOP_Write2bytemask(REG_0094_GOPG_BKA4D9, u32Start, REG_0094_GOPG_BKA4D9_REG_TG_VS_ST);

	if (parse_dt(node_name, integer_dt_parser, (void*)&u32Length, "vsync_width") != 0) {
		GOP_H_ERR("[%s][%d]parsing vsync_width error\n", __FUNCTION__, __LINE__);
		return FALSE;
	}
	HAL_GOP_Write2bytemask(REG_0098_GOPG_BKA4D9, (u32Start+u32Length-1), REG_0098_GOPG_BKA4D9_REG_TG_VS_END);

	if (parse_dt(node_name, integer_dt_parser, (void*)&u32Start, "de_vstart") != 0) {
		GOP_H_ERR("[%s][%d]parsing de_vstart error\n", __FUNCTION__, __LINE__);
		return FALSE;
	}
	HAL_GOP_Write2bytemask(REG_009C_GOPG_BKA4D9, u32Start, REG_009C_GOPG_BKA4D9_REG_TG_VFDE_ST);

	if (parse_dt(node_name, integer_dt_parser, (void*)&u32Length, "resolution_height") != 0) {
		GOP_H_ERR("[%s][%d]parsing resolution_height error\n", __FUNCTION__, __LINE__);
		return FALSE;
	}
	HAL_GOP_Write2bytemask(REG_00A0_GOPG_BKA4D9, (u32Start+u32Length-1), REG_00A0_GOPG_BKA4D9_REG_TG_VFDE_END);

	if (parse_dt(node_name, integer_dt_parser, (void*)&u32Length, "typ_v_total") != 0) {
		GOP_H_ERR("[%s][%d]parsing max_v_total error\n", __FUNCTION__, __LINE__);
		return FALSE;
	}
	HAL_GOP_Write2bytemask(REG_00A4_GOPG_BKA4D9, (u32Length-1), REG_00A4_GOPG_BKA4D9_REG_TG_VTT);



	return TRUE;
}

void _HAL_GOP_PQ_LoadBin(const char *filekey)
{
#ifdef CONFIG_DATA_SEPARATION
    char filepath[FILE_PATH_SIZE] = {0};
    char part[PART_NAME_SIZE] = {0};
    const char *relpath;
    unsigned char *buf = NULL;
    MS_U16 ip_num = 0;          /* 2 bytes */
    MS_U16 reg_num = 0;         /* 2 bytes */
    MS_U32 ip_ptr = 0;          /* 4 bytes */
    MS_U16 reg_bank = 0;        /* 2 bytes */
    MS_U8 reg_offset = 0;       /* 1 bytes */
    MS_U8 reg_mask = 0;         /* 1 bytes */
    MS_U8 reg_value = 0;        /* 1 bytes */
    MS_U32 reg_addr = 0;
    MS_U16 i = 0, j = 0;
    loff_t file_size = 0, file_idx = 0;
    loff_t ip_tbl_offset = 0;

    if (dataindex_get_key(filepath, FILE_PATH_SIZE, PQBIN_SECTION, filekey, NULL) != 0)
    {
        GOP_H_ERR("cannot get %s:%s from dataindex file\n", PQBIN_SECTION, filekey);
        return;
    }
    GOP_H_DBUG("get file path success: %s\n", filepath);

    if (dataindex_resolve_path(part, PART_NAME_SIZE, &relpath, filepath) != 0)
    {
        GOP_H_ERR("resolve path failed: %s\n", filepath);
        return;
    }

    buf = read_storage_file_to_memory(part, relpath, &file_size);
    if (buf == NULL)
    {
        GOP_H_ERR("read file to DRAM failed\n");
        return;
    }

    // find starting header
    j = 0;
    for (file_idx = 0; file_idx < file_size; file_idx++)
    {
        if (buf[file_idx] == PQBIN_HEADER[j])
        {
            if (j == (strlen(PQBIN_HEADER)-1))
            {
                break;
            }
            else
            {
                j++;
            }
        }
        else
        {
            j = 0;
        }
    }
    if (file_idx >= file_size)
    {
        GOP_H_ERR("find starting header failed\n");
        free(buf);
        return;
    }
    ip_tbl_offset = file_idx + PQBIN_IP_TBL_OFST;

    file_idx = file_idx + PQBIN_IP_NUM_OFST;
    READ_BUF_CHECK(buf, file_size, file_idx, sizeof(MS_U16));
    ip_num = *(MS_U16 *)(buf + file_idx);

    for (i = 0; i < ip_num; i++)
    {
        file_idx = ip_tbl_offset + i * PQBIN_IP_TBL_SIZE + PQBIN_IP_TBL_REG_NUM_OFST;
        READ_BUF_CHECK(buf, file_size, file_idx, sizeof(MS_U16));
        reg_num = *(MS_U16 *)(buf + file_idx);

        file_idx = ip_tbl_offset + i * PQBIN_IP_TBL_SIZE + PQBIN_IP_TBL_PTR_OFST;
        READ_BUF_CHECK(buf, file_size, file_idx, sizeof(MS_U32));
        ip_ptr = *(MS_U32 *)(buf + file_idx);

        for (j = 0; j < reg_num; j++)
        {
            file_idx = ip_ptr + PQBIN_IP_DATA_OFST + j * PQBIN_REG_DATA_SIZE + PQBIN_REG_BANK_OFST;
            READ_BUF_CHECK(buf, file_size, file_idx, sizeof(MS_U16));
            reg_bank = *(MS_U16 *)(buf + file_idx);

            file_idx = ip_ptr + PQBIN_IP_DATA_OFST + j * PQBIN_REG_DATA_SIZE + PQBIN_REG_BKOFT_OFST;
            READ_BUF_CHECK(buf, file_size, file_idx, sizeof(MS_U8));
            reg_offset = *(MS_U8 *)(buf + file_idx);

            file_idx = ip_ptr + PQBIN_IP_DATA_OFST + j * PQBIN_REG_DATA_SIZE + PQBIN_REG_MASK_OFST;
            READ_BUF_CHECK(buf, file_size, file_idx, sizeof(MS_U8));
            reg_mask = *(MS_U8 *)(buf + file_idx);

            file_idx = ip_ptr + PQBIN_IP_DATA_OFST + j * PQBIN_REG_DATA_SIZE + PQBIN_REG_VALUE_OFST;
            READ_BUF_CHECK(buf, file_size, file_idx, sizeof(MS_U8));
            reg_value = *(MS_U8 *)(buf + file_idx);

            reg_addr = (MS_U32)(((reg_bank * 0x100 + reg_offset) << 1) - (reg_offset & 1));
            HAL_GOP_Writebyte(reg_addr, (HAL_GOP_Readbyte(reg_addr) & (~reg_mask)) | (reg_value & reg_mask));
        }
    }

    free(buf);
#else
    GOP_H_DBUG("not support data separation\n");
#endif
}

void HAL_GOP_GetGOPEnum(GOP_CTX_HAL_LOCAL *pGOPHalLocal, GOP_TYPE_DEF* GOP_TYPE)
{
    GOP_TYPE->GOP0 = E_GOP0;
    GOP_TYPE->GOP1 = E_GOP1;
    GOP_TYPE->GOP2 = E_GOP2;
    GOP_TYPE->GOP3 = E_GOP3;
    GOP_TYPE->GOP4 = E_GOP4;
    GOP_TYPE->GOP5 = E_GOP5;
}

GOP_Result HAL_GOP_SetWinFmt(GOP_CTX_HAL_LOCAL *pGOPHalLocal, MS_U8 regForm, MS_U8 u8GOPNum, MS_U8 u8GwinNum, MS_U16 colortype)
{
	MS_U16 u16RegVal = 0;
	MS_U32 u32BankOffSet = 0;

	_GetBnkOfstByGop(u8GOPNum, &u32BankOffSet);

	colortype = (colortype >> 4);
	switch(colortype) {
	case GOP_FMT_ARGB4444:
	case GOP_FMT_ABGR4444:
	case GOP_FMT_RGBA4444:
	case GOP_FMT_BGRA4444:
		u16RegVal = GOP_FMT_ARGB4444;
		break;
	case GOP_FMT_ARGB8888:
	case GOP_FMT_ABGR8888:
	case GOP_FMT_RGBA8888:
	case GOP_FMT_BGRA8888:
		u16RegVal = GOP_FMT_ARGB8888;
		break;
	case GOP_FMT_ARGB1555:
	case GOP_FMT_ARGB1555_DST:
	case GOP_FMT_ABGR1555:
	case GOP_FMT_RGBA5551:
	case GOP_FMT_BGRA5551:
		u16RegVal = GOP_FMT_ARGB1555;
		break;
	case GOP_FMT_RGB565:
	case GOP_FMT_BGR565:
		u16RegVal = GOP_FMT_RGB565;
		break;
	case GOP_FMT_YUV422:
		u16RegVal = GOP_FMT_YUV422;
		break;
	default:
		break;
	}

	HAL_GOP_Write2bytemask(u32BankOffSet + REG_0004_GOP0_BKA4DE, u16RegVal, REG_0004_GOP0_BKA4DE_REG_DATA_FMT);

	switch(colortype) {
	case GOP_FMT_RGB565:
	case GOP_FMT_ARGB4444:
	case GOP_FMT_ARGB8888:
	case GOP_FMT_ARGB1555:
	case GOP_FMT_ARGB1555_DST:
		HAL_GOP_Write2bytemask(u32BankOffSet + REG_0004_GOP0_BKA4DE, 0x0, REG_0004_GOP0_BKA4DE_REG_UV_SWAP);
		HAL_GOP_Write2bytemask(u32BankOffSet + REG_0004_GOP0_BKA4DE, 0x0, REG_0004_GOP0_BKA4DE_REG_AD_SWAP);
		break;
	case GOP_FMT_ABGR8888:
	case GOP_FMT_ABGR4444:
	case GOP_FMT_ABGR1555:
		HAL_GOP_Write2bytemask(u32BankOffSet + REG_0004_GOP0_BKA4DE, 0x1, REG_0004_GOP0_BKA4DE_REG_UV_SWAP);
		HAL_GOP_Write2bytemask(u32BankOffSet + REG_0004_GOP0_BKA4DE, 0x0, REG_0004_GOP0_BKA4DE_REG_AD_SWAP);
		break;
	case GOP_FMT_RGBA5551:
	case GOP_FMT_RGBA4444:
	case GOP_FMT_RGBA8888:
		HAL_GOP_Write2bytemask(u32BankOffSet + REG_0004_GOP0_BKA4DE, 0x0, REG_0004_GOP0_BKA4DE_REG_UV_SWAP);
		HAL_GOP_Write2bytemask(u32BankOffSet + REG_0004_GOP0_BKA4DE, 0x1, REG_0004_GOP0_BKA4DE_REG_AD_SWAP);
		break;
	case GOP_FMT_BGRA5551:
	case GOP_FMT_BGRA4444:
	case GOP_FMT_BGRA8888:
		HAL_GOP_Write2bytemask(u32BankOffSet + REG_0004_GOP0_BKA4DE, 0x1, REG_0004_GOP0_BKA4DE_REG_UV_SWAP);
		HAL_GOP_Write2bytemask(u32BankOffSet + REG_0004_GOP0_BKA4DE, 0x1, REG_0004_GOP0_BKA4DE_REG_AD_SWAP);
		break;
	case GOP_FMT_BGR565:
		HAL_GOP_Write2bytemask(u32BankOffSet + REG_0004_GOP0_BKA4DE, 0x1, REG_0004_GOP0_BKA4DE_REG_UV_SWAP);
		break;
	default:
		break;
	}
    return GOP_SUCCESS;
}

GOP_Result HAL_GOP_Set_PINPON(GOP_CTX_HAL_LOCAL *pGOPHalLocal, MS_U8 u8GOPNum, MS_BOOL bEn, E_DRV_GOP_PINPON_MODE pinpon_mode)
{
	return GOP_SUCCESS;
}


MS_BOOL _GetBnkOfstByGop(MS_U8 gop, MS_U32 *pBnkOfst)
{
	if(gop >= gHalGopChipPro.TotalGOPNum) {
		GOP_H_ERR("[%s][%d] Out of GOP support!!! GOP=%d\n",__FUNCTION__,__LINE__ ,gop);
		return FALSE;
	}

	if (gop == E_GOP0)
		*pBnkOfst = GOP0_BNK_ADDR_OFST;
	else if (gop == E_GOP1)
		*pBnkOfst = GOP1_BNK_ADDR_OFST;
	else if (gop == E_GOP2)
		*pBnkOfst = GOP2_BNK_ADDR_OFST;
	else if (gop == E_GOP3)
		*pBnkOfst = GOP3_BNK_ADDR_OFST;
	else if (gop == E_GOP4)
		*pBnkOfst = GOP4_BNK_ADDR_OFST;

	return TRUE;
}

GOP_Result HAL_GOP_SetGOPACKMask(GOP_CTX_HAL_LOCAL *pGOPHalLocal,MS_U16 u16GopMask)
{
	MS_U16 u16RegVal = 0;

	u16RegVal = HAL_GOP_Read2byte(REG_0020_GOPG_BKA4D9);
	u16RegVal |= u16GopMask;
	HAL_GOP_Write2byte(REG_0020_GOPG_BKA4D9, u16RegVal);

	return GOP_SUCCESS;
}

GOP_Result HAL_GOP_SetGOPACK(GOP_CTX_HAL_LOCAL *pGOPHalLocal,MS_U8 gop)
{
	MS_U32 ChipMajor = (u32ChipVersion & GOP_CAPS_MAJOR_IPVERSION_OFFSET_MSK) >> GOP_CAPS_MAJOR_IPVERSION_OFFSET_SHT;

	if(gop >= gHalGopChipPro.TotalGOPNum) {
		GOP_H_ERR("[%s][%d] Out of GOP support!!! GOP=%d\n",__FUNCTION__,__LINE__ ,gop);
		return GOP_FAIL;
	}

	if (gop == E_GOP0)
		HAL_GOP_Write2bytemask(REG_0020_GOPG_BKA4D9, 1, REG_0020_GOPG_BKA4D9_REG_GOP0_CFG_TRIG);
	else if (gop == E_GOP1)
		HAL_GOP_Write2bytemask(REG_0020_GOPG_BKA4D9, 1, REG_0020_GOPG_BKA4D9_REG_GOP1_CFG_TRIG);
	else if (gop == E_GOP2)
		HAL_GOP_Write2bytemask(REG_0020_GOPG_BKA4D9, 1, REG_0020_GOPG_BKA4D9_REG_GOP2_CFG_TRIG);
	else if (gop == E_GOP3) {
		if (ChipMajor == GOP_M6_SERIES_MAJOR)
			HAL_GOP_Write2bytemask(REG_0020_GOPG_BKA4D9, 1, REG_0020_GOPG_BKA4D9_REG_GOP3_CFG_TRIG);
		else if (ChipMajor == GOP_MOKONA_SERIES_MAJOR)
			HAL_GOP_Write2bytemask(REG_0020_GOPG_BKA4D9, 1, REG_0020_GOPG_BKA4D9_REG_GOPC_CFG_TRIG);
	} else if (gop == E_GOP4)
		HAL_GOP_Write2bytemask(REG_0020_GOPG_BKA4D9, 1, REG_0020_GOPG_BKA4D9_REG_GOPC_CFG_TRIG);

	return GOP_SUCCESS;
}

MS_U16 HAL_GOP_GetGOPACK(GOP_CTX_HAL_LOCAL *pGOPHalLocal,MS_U8 gop)
{
	MS_BOOL bUpdated = FALSE;
	MS_U16 u16RegVal = 0;
	MS_U32 ChipMajor = (u32ChipVersion & GOP_CAPS_MAJOR_IPVERSION_OFFSET_MSK) >> GOP_CAPS_MAJOR_IPVERSION_OFFSET_SHT;

	switch(gop) {
	case 0:
		u16RegVal = HAL_GOP_Read2bytemask(REG_0020_GOPG_BKA4D9, REG_0020_GOPG_BKA4D9_REG_GOP0_CFG_ACK);
		break;
	case 1:
		u16RegVal = HAL_GOP_Read2bytemask(REG_0020_GOPG_BKA4D9, REG_0020_GOPG_BKA4D9_REG_GOP1_CFG_ACK);
		break;
	case 2:
		u16RegVal = HAL_GOP_Read2bytemask(REG_0020_GOPG_BKA4D9, REG_0020_GOPG_BKA4D9_REG_GOP2_CFG_ACK);
		break;
	case 3:
		if (ChipMajor == GOP_M6_SERIES_MAJOR)
			u16RegVal = HAL_GOP_Read2bytemask(REG_0020_GOPG_BKA4D9, REG_0020_GOPG_BKA4D9_REG_GOP3_CFG_ACK);
		else if (ChipMajor == GOP_MOKONA_SERIES_MAJOR)
			u16RegVal = HAL_GOP_Read2bytemask(REG_0020_GOPG_BKA4D9, REG_0020_GOPG_BKA4D9_REG_GOPC_CFG_ACK);
		break;
	case 4:
		u16RegVal = HAL_GOP_Read2bytemask(REG_0020_GOPG_BKA4D9, REG_0020_GOPG_BKA4D9_REG_GOPC_CFG_ACK);
		break;
	default:
		break;
	}

	if (u16RegVal == 0)
		bUpdated = TRUE;
	else
		bUpdated = FALSE;

	return bUpdated;
}

void HAL_GOP_Init(GOP_CTX_HAL_LOCAL *pGOPHalLocal, MS_U8 u8GOPNum)
{
	MS_U32 u32Bankoffset = 0;
	MS_U32 ChipMajor = (u32ChipVersion & GOP_CAPS_MAJOR_IPVERSION_OFFSET_MSK) >> GOP_CAPS_MAJOR_IPVERSION_OFFSET_SHT;
	MS_U32 ChipMinor = (u32ChipVersion & GOP_CAPS_MINOR_IPVERSION_OFFSET_MSK);

	_GetBnkOfstByGop(u8GOPNum, &u32Bankoffset);

	HAL_GOP_Write2bytemask(REG_0008_GOPG_BKA4D9, 1, REG_0008_GOPG_BKA4D9_REG_GOP_GLOBAL_SW_RST);

	//disable GOP src enable
	HAL_GOP_Write2bytemask(REG_004C_GOPG_BKA4D9, 0, REG_004C_GOPG_BKA4D9_REG_MIXER4_SRC0_EN);
	HAL_GOP_Write2bytemask(REG_004C_GOPG_BKA4D9, 0, REG_004C_GOPG_BKA4D9_REG_MIXER4_SRC1_EN);
	HAL_GOP_Write2bytemask(REG_004C_GOPG_BKA4D9, 0, REG_004C_GOPG_BKA4D9_REG_MIXER4_SRC2_EN);
	HAL_GOP_Write2bytemask(REG_004C_GOPG_BKA4D9, 0, REG_004C_GOPG_BKA4D9_REG_MIXER4_SRC3_EN);
	HAL_GOP_Write2bytemask(REG_006C_GOPG_BKA4D9, 0, REG_006C_GOPG_BKA4D9_REG_MIXER2_SRC0_EN);
	HAL_GOP_Write2bytemask(REG_006C_GOPG_BKA4D9, 0, REG_006C_GOPG_BKA4D9_REG_MIXER2_SRC1_EN);

	//disable H scaling
	HAL_GOP_Write2bytemask(u32Bankoffset + REG_0058_GOP0_BKA4DE, 0, REG_0058_GOP0_BKA4DE_REG_HSP_RATIO_0);
	HAL_GOP_Write2bytemask(u32Bankoffset + REG_005C_GOP0_BKA4DE, 0, REG_005C_GOP0_BKA4DE_REG_HSP_RATIO_1);
	HAL_GOP_Write2bytemask(u32Bankoffset + REG_005C_GOP0_BKA4DE, 0, REG_005C_GOP0_BKA4DE_REG_HSP_EN);
	HAL_GOP_Write2bytemask(u32Bankoffset + REG_0074_GOP0_BKA4DE, 0, REG_0074_GOP0_BKA4DE_REG_HSP_INI_RATIO_M1);

	//disable H scaling
	HAL_GOP_Write2bytemask(u32Bankoffset + REG_0060_GOP0_BKA4DE, 0, REG_0060_GOP0_BKA4DE_REG_VSP_RATIO_0);
	HAL_GOP_Write2bytemask(u32Bankoffset + REG_0064_GOP0_BKA4DE, 0, REG_0064_GOP0_BKA4DE_REG_VSP_RATIO_1);
	HAL_GOP_Write2bytemask(u32Bankoffset + REG_0064_GOP0_BKA4DE, 0, REG_0064_GOP0_BKA4DE_REG_VSP_EN);
	HAL_GOP_Write2bytemask(u32Bankoffset + REG_007C_GOP0_BKA4DE, 0, REG_007C_GOP0_BKA4DE_REG_VSP_INI_RATIO_M1);

	//Mixer 4 output graphic alpha and RMA enable
	HAL_GOP_Write2bytemask(REG_0048_GOPG_BKA4D9, 1, REG_0048_GOPG_BKA4D9_REG_MIXER4_OUT_GFX_ALPHA);
	HAL_GOP_Write2bytemask(REG_0048_GOPG_BKA4D9, 1, REG_0048_GOPG_BKA4D9_REG_MIXER4_RMA_EN);

	//Mixer 2 output graphic alpha and RMA enable
	HAL_GOP_Write2bytemask(REG_0068_GOPG_BKA4D9, 1, REG_0068_GOPG_BKA4D9_REG_MIXER2_OUT_GFX_ALPHA);
	HAL_GOP_Write2bytemask(REG_0068_GOPG_BKA4D9, 1, REG_0068_GOPG_BKA4D9_REG_MIXER2_RMA_EN);

	HAL_GOP_Write2bytemask(REG_0008_GOPG_BKA4D9, 0, REG_0008_GOPG_BKA4D9_REG_GOP_GLOBAL_SW_RST);

	//clock
	HAL_GOP_Write2bytemask(REG_0E3C_CKGEN01, 0x0, REG_0E3C_CKGEN01_REG_CKG_XC_GOP_SCTCON); // gop engine clock
	HAL_GOP_Write2bytemask(REG_0DB8_CKGEN01, 0x0, REG_0DB8_CKGEN01_REG_CKG_GOP0_SRAM);
	HAL_GOP_Write2bytemask(REG_0DC0_CKGEN01, 0x0, REG_0DC0_CKGEN01_REG_CKG_GOP1_SRAM);
	HAL_GOP_Write2bytemask(REG_0DC8_CKGEN01, 0x0, REG_0DC8_CKGEN01_REG_CKG_GOP2_SRAM);
	HAL_GOP_Write2bytemask(REG_0DD0_CKGEN01, 0x0, REG_0DD0_CKGEN01_REG_CKG_GOP3_SRAM);

	//clock sw en always 1
	HAL_GOP_Write2bytemask(REG_17CC_CKGEN01, 0x1, REG_17CC_CKGEN01_REG_SW_EN_XC_GOP0_DST2SCTCON);
	HAL_GOP_Write2bytemask(REG_17D0_CKGEN01, 0x1, REG_17D0_CKGEN01_REG_SW_EN_XC_GOP_SCL2SCTCON);
	HAL_GOP_Write2bytemask(REG_17D4_CKGEN01, 0x1, REG_17D4_CKGEN01_REG_SW_EN_XC_GOP2SCTCON);
	HAL_GOP_Write2bytemask(REG_17D8_CKGEN01, 0x1, REG_17D8_CKGEN01_REG_SW_EN_XC_GOPC_DST2SCTCON);
	HAL_GOP_Write2bytemask(REG_17DC_CKGEN01, 0x1, REG_17DC_CKGEN01_REG_SW_EN_XC_GOPG_DST2SCTCON);
	HAL_GOP_Write2bytemask(REG_18F0_CKGEN01, 0x1, REG_18F0_CKGEN01_REG_SW_EN_GOP0_SRAM2SCTCON);
	HAL_GOP_Write2bytemask(REG_18F4_CKGEN01, 0x1, REG_18F4_CKGEN01_REG_SW_EN_GOP1_SRAM2SCTCON);
	HAL_GOP_Write2bytemask(REG_18F8_CKGEN01, 0x1, REG_18F8_CKGEN01_REG_SW_EN_GOP2_SRAM2SCTCON);
	HAL_GOP_Write2bytemask(REG_18FC_CKGEN01, 0x1, REG_18FC_CKGEN01_REG_SW_EN_GOP3_SRAM2SCTCON);
	HAL_GOP_Write2bytemask(REG_1990_CKGEN01, 0x1, REG_1990_CKGEN01_REG_SW_EN_XC_FD_GOP02SCTCON);

	HAL_GOP_Write2bytemask(REG_0E34_CKGEN01, 0x0, REG_0E34_CKGEN01_REG_CKG_XC_GOP0_DST_SCTCON);
	HAL_GOP_Write2bytemask(REG_0E40_CKGEN01, 0x0, REG_0E40_CKGEN01_REG_CKG_XC_GOPC_DST_SCTCON);
	HAL_GOP_Write2bytemask(REG_0E44_CKGEN01, 0x0, REG_0E44_CKGEN01_REG_CKG_XC_GOPG_DST_SCTCON);

    _HAL_GOP_PQ_LoadBin(PQBIN_KEY_MAIN);
    _HAL_GOP_PQ_LoadBin(PQBIN_KEY_EX);

	if (ChipMajor >= GOP_MOKONA_SERIES_MAJOR) {
		HAL_GOP_Write2bytemask(REG_01E0_GOPG_BKA4D9, 1, REG_01E0_GOPG_BKA4D9_REG_OSDB_VIDEO_DE_SEL);
		HAL_GOP_Write2bytemask(REG_001C_GOPG_BKA4D9, 1, REG_001C_GOPG_BKA4D9_REG_AFBC_CORE0_VS_REF);
		HAL_GOP_Write2bytemask(REG_0004_OSD_ABF_LB_BKA4DD, 0, REG_0004_OSD_ABF_LB_BKA4DD_REG_BYPASS_EN);
	} else if (ChipMajor == GOP_M6_SERIES_MAJOR && (ChipMinor == GOP_M6L_SERIES_MINOR || ChipMinor == GOP_M6L_E3_SERIES_MINOR)) {
		HAL_GOP_Write2bytemask(REG_0004_OSD_ABF_LB_BKA4DD, 0, REG_0004_OSD_ABF_LB_BKA4DD_REG_BYPASS_EN);
	}

}

void HAL_GOP_Chip_Proprity_Init(GOP_CTX_HAL_LOCAL *pGOPHalLocal)
{
	memcpy(&gHalGopChipPro, pGOPHalLocal->pGopChipPro, sizeof(GOP_CHIP_PROPERTY));
}

MS_U8 HAL_GOP_GetMaxGwinNumByGOP(GOP_CTX_HAL_LOCAL *pGOPHalLocal, MS_U8 u8GopNum)
{
	switch(u8GopNum) {
	case E_GOP0:
		return (MS_U8)MAX_GOP0_GWIN;
	case E_GOP1:
		return (MS_U8)MAX_GOP1_GWIN;
	case E_GOP2:
		return (MS_U8)MAX_GOP2_GWIN;
	case E_GOP3:
		return (MS_U8)MAX_GOP3_GWIN;
	case E_GOP4:
		return (MS_U8)MAX_GOP4_GWIN;
	default:
		MS_ASSERT(0);
		return 0xFF;
    }
}

MS_U8 HAL_GOP_SelGwinIdByGOP(GOP_CTX_HAL_LOCAL *pGOPHalLocal, MS_U8 u8Gop, MS_U8 u8Idx)
{
	MS_U8 u8GWinId = 0;

	//Adjust GWIN ID by different Chip
	if(u8Gop >= gHalGopChipPro.TotalGOPNum) {
		GOP_H_ERR("[%s][%d] Out of GOP support!!! GOP=%d\n",__FUNCTION__,__LINE__ ,u8Gop);
		MS_ASSERT(0);
		return 0xFF;
	}

	switch(u8Gop) {
	case E_GOP0:
		u8GWinId = GOP0_GwinIdBase + u8Idx;
		break;
	case E_GOP1:
		u8GWinId = GOP1_GwinIdBase + u8Idx;
		break;
	case E_GOP2:
		u8GWinId = GOP2_GwinIdBase + u8Idx;
		break;
	case E_GOP3:
		u8GWinId = GOP3_GwinIdBase + u8Idx;
		break;
	case E_GOP4:
		u8GWinId = GOP4_GwinIdBase + u8Idx;
		break;
	default:
		break;
	}

	return u8GWinId;

}

GOP_Result HAL_GOP_GOPSel(GOP_CTX_HAL_LOCAL *pGOPHalLocal, MS_U8 u8GOPNum)
{
	if(u8GOPNum >= gHalGopChipPro.TotalGOPNum) {
		GOP_H_ERR("[%s][%d] Out of GOP support!!! GOP=%d\n",__FUNCTION__,__LINE__ ,u8GOPNum);
		MS_ASSERT(0);
		return GOP_FAIL;
	}

	switch(u8GOPNum) {
	case 0:
		pGOPHalLocal->bank_offset = GOP0_BNK_ADDR_OFST;
		return GOP_SUCCESS;
	case 1:
		pGOPHalLocal->bank_offset = GOP1_BNK_ADDR_OFST;
            return GOP_SUCCESS;
	case 2:
		pGOPHalLocal->bank_offset = GOP2_BNK_ADDR_OFST;
		return GOP_SUCCESS;
	case 3:
		pGOPHalLocal->bank_offset = GOP3_BNK_ADDR_OFST;
		return GOP_SUCCESS;
	case 4:
		pGOPHalLocal->bank_offset = GOP4_BNK_ADDR_OFST;
		return GOP_SUCCESS;
	default:
		MS_ASSERT(0);
		return GOP_FAIL;
	}
}

MS_U16 HAL_GOP_GetBPP(GOP_CTX_HAL_LOCAL *pGOPHalLocal, DRV_GOPColorType fbFmt)
{
    MS_U16 bpp=0;

    switch ( fbFmt )
    {
    case E_DRV_GOP_COLOR_RGB555_BLINK :
    case E_DRV_GOP_COLOR_RGB565 :
    case E_DRV_GOP_COLOR_ARGB1555:
    case E_DRV_GOP_COLOR_RGBA5551:
    case E_DRV_GOP_COLOR_ARGB4444 :
    case E_DRV_GOP_COLOR_RGBA4444 :
    case E_DRV_GOP_COLOR_RGB555YUV422:
    case E_DRV_GOP_COLOR_YUV422:
    case E_DRV_GOP_COLOR_2266:
        bpp = 16;
        break;
    case E_DRV_GOP_COLOR_ARGB8888 :
    case E_DRV_GOP_COLOR_ABGR8888 :
        bpp = 32;
        break;

    case E_DRV_GOP_COLOR_I8 :
        bpp = 8;
        break;

    default :
        //print err
        MS_ASSERT(0);
        bpp = 0xFFFF;
        break;
    }
    return bpp;

}

void HAL_GOP_GWIN_SetBlending(GOP_CTX_HAL_LOCAL *pGOPHalLocal, MS_U8 u8GOP, MS_U8 u8win, MS_BOOL bEnable, MS_U8 u8coef)
{
	MS_U32 u32Bankoffset = 0;

	_GetBnkOfstByGop(u8GOP, &u32Bankoffset);

	HAL_GOP_Write2bytemask(u32Bankoffset + REG_0004_GOP0_BKA4DE, bEnable?1:0, REG_0004_GOP0_BKA4DE_REG_PIX_ALPHA_EN);
	HAL_GOP_Write2bytemask(u32Bankoffset + REG_0030_GOP0_BKA4DE, u8coef, REG_0030_GOP0_BKA4DE_REG_ALPHA_DEF0);
}

void HAL_GOP_SetIOMapBase(GOP_CTX_HAL_LOCAL *pGOPHalLocal, MS_VIRT addr)
{
	//no use
}
void HAL_GOP_SetIOFRCMapBase(GOP_CTX_HAL_LOCAL *pGOPHalLocal, MS_VIRT addr)
{
}
void HAL_GOP_SetIOPMMapBase(GOP_CTX_HAL_LOCAL *pGOPHalLocal, MS_VIRT addr)
{
}


GOP_Result HAL_GOP_GWIN_SetDstPlane(GOP_CTX_HAL_LOCAL *pGOPHalLocal, MS_U8 GopNum, DRV_GOPDstType eDstType,MS_BOOL bOnlyCheck)
{
	MS_U32 ChipMajor = (u32ChipVersion & GOP_CAPS_MAJOR_IPVERSION_OFFSET_MSK) >> GOP_CAPS_MAJOR_IPVERSION_OFFSET_SHT;

	if( GopNum >= gHalGopChipPro.TotalGOPNum) {
		GOP_H_ERR("[%s][%d] Out of GOP support!!! GOP=%d\n",__FUNCTION__,__LINE__ ,GopNum);
		return GOP_INVALID_PARAMETERS;
	}

	if (ChipMajor >= GOP_MOKA_SERIES_MAJOR) {
		switch (eDstType) {
		case E_DRV_GOP_DST_OP0:
			if (GopNum == E_GOP0)
				HAL_GOP_Write2bytemask(REG_003C_GOPG_BKA4D9, GOP_DST_OP, REG_003C_GOPG_BKA4D9_REG_GOP0_DEST_SEL_003C);
			else if(GopNum == E_GOP1)
				HAL_GOP_Write2bytemask(REG_003C_GOPG_BKA4D9, GOP_DST_OP, REG_003C_GOPG_BKA4D9_REG_GOP1_DEST_SEL_003C);
			else if(GopNum == E_GOP2)
				HAL_GOP_Write2bytemask(REG_003C_GOPG_BKA4D9, GOP_DST_OP, REG_003C_GOPG_BKA4D9_REG_GOP2_DEST_SEL_003C);

			break;
		case E_DRV_GOP_DST_FRC:
			if (GopNum == E_GOP0)
				HAL_GOP_Write2bytemask(REG_003C_GOPG_BKA4D9, GOP_DST_VG, REG_003C_GOPG_BKA4D9_REG_GOP0_DEST_SEL_003C);
			else if(GopNum == E_GOP1)
				HAL_GOP_Write2bytemask(REG_003C_GOPG_BKA4D9, GOP_DST_VG, REG_003C_GOPG_BKA4D9_REG_GOP1_DEST_SEL_003C);
			else if(GopNum == E_GOP2)
				HAL_GOP_Write2bytemask(REG_003C_GOPG_BKA4D9, GOP_DST_VG, REG_003C_GOPG_BKA4D9_REG_GOP2_DEST_SEL_003C);

			HAL_GOP_Write2bytemask(REG_0004_GOPG_BKA4D9, 0x1, REG_0004_GOPG_BKA4D9_REG_VG_SEP_PATH);
			HAL_GOP_Write2bytemask(REG_0004_GOPG_BKA4D9, 0x1, REG_0004_GOPG_BKA4D9_REG_S2PQ_POS_0004);
			break;
		default:
			break;
		}
		return GOP_SUCCESS;
	}

	switch (eDstType) {
	case E_DRV_GOP_DST_IP0:
		if (GopNum == E_GOP0)
			HAL_GOP_Write2bytemask(REG_0004_GOPG_BKA4D9, 0x2, REG_0004_GOPG_BKA4D9_REG_GOP0_DEST_SEL);
		else
			GOP_H_ERR("[%s][%d] GOP=%d not support blending to XC IP\n",__FUNCTION__,__LINE__ ,GopNum);
	break;
	case E_DRV_GOP_DST_OP0:
		if (GopNum == E_GOP0) {
			HAL_GOP_Write2bytemask(REG_0004_GOPG_BKA4D9, 0x0, REG_0004_GOPG_BKA4D9_REG_GOP0_DEST_SEL);
		} else if (GopNum == GOPCursor) {
			if (ChipMajor == GOP_M6_SERIES_MAJOR)
				HAL_GOP_Write2bytemask(REG_0004_GOPG_BKA4D9, 0x0, REG_0004_GOPG_BKA4D9_REG_GOPC_DEST_SEL);
			else if (ChipMajor == GOP_MOKONA_SERIES_MAJOR)
				HAL_GOP_Write2bytemask(REG_0004_GOPG_BKA4D9, 0x1, REG_0004_GOPG_BKA4D9_REG_GOPC_DEST_SEL);
		}

		HAL_GOP_Write2bytemask(REG_0004_GOPG_BKA4D9, 0x0, REG_0004_GOPG_BKA4D9_REG_VG_SEP_PATH);
	break;
	case E_DRV_GOP_DST_FRC:
		HAL_GOP_Write2bytemask(REG_00C0_GOPG_BKA4D9, 0x0, REG_00C0_GOPG_BKA4D9_REG_TG_VG_SYNC_EN);
		if (ChipMajor == GOP_M6_SERIES_MAJOR)
			HAL_GOP_Write2bytemask(REG_0004_GOPG_BKA4D9, 0x0, REG_0004_GOPG_BKA4D9_REG_GOPC_DEST_SEL);
		else if (ChipMajor == GOP_MOKONA_SERIES_MAJOR)
			HAL_GOP_Write2bytemask(REG_0004_GOPG_BKA4D9, 0x1, REG_0004_GOPG_BKA4D9_REG_GOPC_DEST_SEL);

		HAL_GOP_Write2bytemask(REG_0004_GOPG_BKA4D9, 0x1, REG_0004_GOPG_BKA4D9_REG_VG_SEP_PATH);
		if(_HAL_GOP_SetTgen(FALSE) != TRUE) {
			GOP_H_ERR("[%s][%d]Parsing DTB fail\n", __FUNCTION__, __LINE__);
			return GOP_FAIL;
		}
	break;
	default:
		return GOP_FUN_NOT_SUPPORTED;
	}

	return GOP_SUCCESS;
}

void HAL_GOP_GWIN_GetMUX(GOP_CTX_HAL_LOCAL*pGOPHalLocal, MS_U8* u8GOPNum, Gop_MuxSel eGopMux)
{
	MS_U32 u32GOP = 0;
	MS_U16 u16RegVal = 0;

	switch (eGopMux) {
		case E_GOP_MUX0:
			u32GOP = HAL_GOP_Read2bytemask(REG_004C_GOPG_BKA4D9, REG_004C_GOPG_BKA4D9_REG_MIXER4_LAYER0_SRC_D);
		break;
		case E_GOP_MUX1:
			u32GOP = HAL_GOP_Read2bytemask(REG_004C_GOPG_BKA4D9, REG_004C_GOPG_BKA4D9_REG_MIXER4_LAYER1_SRC_D);
		break;
		case E_GOP_MUX2:
			u32GOP = HAL_GOP_Read2bytemask(REG_004C_GOPG_BKA4D9, REG_004C_GOPG_BKA4D9_REG_MIXER4_LAYER2_SRC_D);
		break;
		case E_GOP_MUX3:
			u32GOP = HAL_GOP_Read2bytemask(REG_004C_GOPG_BKA4D9, REG_004C_GOPG_BKA4D9_REG_MIXER4_LAYER3_SRC_D);
		break;
		case E_GOP_MUX4:
			u16RegVal = HAL_GOP_Read2bytemask(REG_006C_GOPG_BKA4D9, REG_006C_GOPG_BKA4D9_REG_MIXER2_LAYER1_SRC_D);
			if (u16RegVal == 1)
				u32GOP = GOPCursor;
			else
				u32GOP = HAL_GOP_Read2bytemask(REG_004C_GOPG_BKA4D9, REG_004C_GOPG_BKA4D9_REG_MIXER4_LAYER3_SRC_D);
		break;
		default:
		break;
	}

	*u8GOPNum = (MS_U8)u32GOP;
}

void HAL_GOP_GWIN_SetMUX(GOP_CTX_HAL_LOCAL*pGOPHalLocal, MS_U8 u8GOPNum, Gop_MuxSel eGopMux)
{
	MS_U32 ChipMajor = (u32ChipVersion & GOP_CAPS_MAJOR_IPVERSION_OFFSET_MSK) >> GOP_CAPS_MAJOR_IPVERSION_OFFSET_SHT;

	if (ChipMajor == GOP_M6_SERIES_MAJOR) {
		if (u8GOPNum != GOPCursor) {//GOPC use Mixer2
			switch (eGopMux) {
			case E_GOP_MUX0:
				HAL_GOP_Write2bytemask(REG_004C_GOPG_BKA4D9, u8GOPNum, REG_004C_GOPG_BKA4D9_REG_MIXER4_LAYER0_SRC_D);
				HAL_GOP_Write2bytemask(REG_0050_GOPG_BKA4D9, u8GOPNum, REG_0050_GOPG_BKA4D9_REG_MIXER4_LAYER0_SRC_A);
				break;
			case E_GOP_MUX1:
				HAL_GOP_Write2bytemask(REG_004C_GOPG_BKA4D9, u8GOPNum, REG_004C_GOPG_BKA4D9_REG_MIXER4_LAYER1_SRC_D);
				HAL_GOP_Write2bytemask(REG_0050_GOPG_BKA4D9, u8GOPNum, REG_0050_GOPG_BKA4D9_REG_MIXER4_LAYER1_SRC_A);
				break;
			case E_GOP_MUX2:
				HAL_GOP_Write2bytemask(REG_004C_GOPG_BKA4D9, u8GOPNum, REG_004C_GOPG_BKA4D9_REG_MIXER4_LAYER2_SRC_D);
				HAL_GOP_Write2bytemask(REG_0050_GOPG_BKA4D9, u8GOPNum, REG_0050_GOPG_BKA4D9_REG_MIXER4_LAYER2_SRC_A);
				break;
			case E_GOP_MUX3:
				HAL_GOP_Write2bytemask(REG_004C_GOPG_BKA4D9, u8GOPNum, REG_004C_GOPG_BKA4D9_REG_MIXER4_LAYER3_SRC_D);
				HAL_GOP_Write2bytemask(REG_0050_GOPG_BKA4D9, u8GOPNum, REG_0050_GOPG_BKA4D9_REG_MIXER4_LAYER3_SRC_A);
				break;
			default:
				GOP_H_ERR("[%s][%d] GOP%d not support to layer%d\n",__FUNCTION__,__LINE__ ,u8GOPNum, eGopMux);
				break;
			}
		} else if (u8GOPNum == GOPCursor) {
			if (eGopMux != E_GOP_MUX4) {
				GOP_H_ERR("[%s][%d] GOP cursor not support to layer%d\n",__FUNCTION__, __LINE__, eGopMux);
			} else {
				HAL_GOP_Write2bytemask(REG_006C_GOPG_BKA4D9, 0x0, REG_006C_GOPG_BKA4D9_REG_MIXER2_LAYER0_SRC_D);
				HAL_GOP_Write2bytemask(REG_0070_GOPG_BKA4D9, 0x0, REG_0070_GOPG_BKA4D9_REG_MIXER2_LAYER0_SRC_A);
				HAL_GOP_Write2bytemask(REG_006C_GOPG_BKA4D9, 0x1, REG_006C_GOPG_BKA4D9_REG_MIXER2_LAYER1_SRC_D);
				HAL_GOP_Write2bytemask(REG_0070_GOPG_BKA4D9, 0x1, REG_0070_GOPG_BKA4D9_REG_MIXER2_LAYER1_SRC_A);
			}
		}
	} else if (ChipMajor >= GOP_MOKONA_SERIES_MAJOR) {
		switch (eGopMux) {
			case E_GOP_MUX0:
				HAL_GOP_Write2bytemask(REG_004C_GOPG_BKA4D9, u8GOPNum, REG_004C_GOPG_BKA4D9_REG_MIXER4_LAYER0_SRC_D);
				HAL_GOP_Write2bytemask(REG_0050_GOPG_BKA4D9, u8GOPNum, REG_0050_GOPG_BKA4D9_REG_MIXER4_LAYER0_SRC_A);
				if (ChipMajor >= GOP_MOKA_SERIES_MAJOR) {
					HAL_GOP_Write2bytemask(REG_012C_GOPG_BKA4D9, u8GOPNum, REG_012C_GOPG_BKA4D9_REG_VG_MIXER4_LAYER0_SRC_D_012C);
					HAL_GOP_Write2bytemask(REG_0130_GOPG_BKA4D9, u8GOPNum, REG_0130_GOPG_BKA4D9_REG_VG_MIXER4_LAYER0_SRC_A_0130);
				}
				break;
			case E_GOP_MUX1:
				HAL_GOP_Write2bytemask(REG_004C_GOPG_BKA4D9, u8GOPNum, REG_004C_GOPG_BKA4D9_REG_MIXER4_LAYER1_SRC_D);
				HAL_GOP_Write2bytemask(REG_0050_GOPG_BKA4D9, u8GOPNum, REG_0050_GOPG_BKA4D9_REG_MIXER4_LAYER1_SRC_A);
				if (ChipMajor >= GOP_MOKA_SERIES_MAJOR) {
					HAL_GOP_Write2bytemask(REG_012C_GOPG_BKA4D9, u8GOPNum, REG_012C_GOPG_BKA4D9_REG_VG_MIXER4_LAYER1_SRC_D_012C);
					HAL_GOP_Write2bytemask(REG_0130_GOPG_BKA4D9, u8GOPNum, REG_0130_GOPG_BKA4D9_REG_VG_MIXER4_LAYER1_SRC_A_0130);
				}
				break;
			case E_GOP_MUX2:
				HAL_GOP_Write2bytemask(REG_004C_GOPG_BKA4D9, u8GOPNum, REG_004C_GOPG_BKA4D9_REG_MIXER4_LAYER2_SRC_D);
				HAL_GOP_Write2bytemask(REG_0050_GOPG_BKA4D9, u8GOPNum, REG_0050_GOPG_BKA4D9_REG_MIXER4_LAYER2_SRC_A);
				if (ChipMajor >= GOP_MOKA_SERIES_MAJOR) {
					HAL_GOP_Write2bytemask(REG_012C_GOPG_BKA4D9, u8GOPNum, REG_012C_GOPG_BKA4D9_REG_VG_MIXER4_LAYER2_SRC_D_012C);
					HAL_GOP_Write2bytemask(REG_0130_GOPG_BKA4D9, u8GOPNum, REG_0130_GOPG_BKA4D9_REG_VG_MIXER4_LAYER2_SRC_A_0130);
				}
				break;
			case E_GOP_MUX3:
				HAL_GOP_Write2bytemask(REG_004C_GOPG_BKA4D9, u8GOPNum, REG_004C_GOPG_BKA4D9_REG_MIXER4_LAYER3_SRC_D);
				HAL_GOP_Write2bytemask(REG_0050_GOPG_BKA4D9, u8GOPNum, REG_0050_GOPG_BKA4D9_REG_MIXER4_LAYER3_SRC_A);
				break;
			default:
				GOP_H_ERR("[%s][%d] GOP%d not support to layer%d\n",__FUNCTION__,__LINE__ ,u8GOPNum, eGopMux);
				break;
			}
	}
}

GOP_Result HAL_GOP_SetGOPEnable2SC(GOP_CTX_HAL_LOCAL *pGOPHalLocal, MS_U8 gopNum, MS_BOOL bEnable)
{
	MS_BOOL src0, src1, src2, src3, src4;
	MS_U32 ChipMajor = (u32ChipVersion & GOP_CAPS_MAJOR_IPVERSION_OFFSET_MSK) >> GOP_CAPS_MAJOR_IPVERSION_OFFSET_SHT;

	switch(gopNum) {
	case E_GOP0:
		HAL_GOP_Write2bytemask(REG_0050_GOPG_BKA4D9, bEnable?1:0, REG_0050_GOPG_BKA4D9_REG_MIXER4_LAYER0_EN);
		break;
	case E_GOP1:
		HAL_GOP_Write2bytemask(REG_0050_GOPG_BKA4D9, bEnable?1:0, REG_0050_GOPG_BKA4D9_REG_MIXER4_LAYER1_EN);
		break;
	case E_GOP2:
		HAL_GOP_Write2bytemask(REG_0050_GOPG_BKA4D9, bEnable?1:0, REG_0050_GOPG_BKA4D9_REG_MIXER4_LAYER2_EN);
		break;
	case E_GOP3:
		HAL_GOP_Write2bytemask(REG_0050_GOPG_BKA4D9, bEnable?1:0, REG_0050_GOPG_BKA4D9_REG_MIXER4_LAYER3_EN);
		break;
	case E_GOP4:
		HAL_GOP_Write2bytemask(REG_0070_GOPG_BKA4D9, bEnable?1:0, REG_0070_GOPG_BKA4D9_REG_MIXER2_LAYER1_EN);
		break;
	default:
		break;
	}

	if(bEnable == TRUE) {
		HAL_GOP_Write2bytemask(REG_0070_GOPG_BKA4D9, 0x1, REG_0070_GOPG_BKA4D9_REG_MIXER2_LAYER0_EN);
		if (ChipMajor == GOP_M6_SERIES_MAJOR)
			HAL_GOP_Write2bytemask(REG_01F0_GOPG_BKA4D9, 0x1, REG_01F0_GOPG_BKA4D9_REG_OSDB0_EN);
		else if (ChipMajor >= GOP_MOKONA_SERIES_MAJOR)
			HAL_GOP_Write2bytemask(REG_01F0_GOPG_BKA4D9, 0x1, REG_01F0_GOPG_BKA4D9_REG_OSDB1_EN);
	} else {
		src0 = HAL_GOP_Read2bytemask(REG_004C_GOPG_BKA4D9, REG_004C_GOPG_BKA4D9_REG_MIXER4_SRC0_EN);
		src1 = HAL_GOP_Read2bytemask(REG_004C_GOPG_BKA4D9, REG_004C_GOPG_BKA4D9_REG_MIXER4_SRC1_EN);
		src2 = HAL_GOP_Read2bytemask(REG_004C_GOPG_BKA4D9, REG_004C_GOPG_BKA4D9_REG_MIXER4_SRC2_EN);
		src3 = HAL_GOP_Read2bytemask(REG_004C_GOPG_BKA4D9, REG_004C_GOPG_BKA4D9_REG_MIXER4_SRC3_EN);
		src4 = HAL_GOP_Read2bytemask(REG_006C_GOPG_BKA4D9, REG_006C_GOPG_BKA4D9_REG_MIXER2_SRC1_EN);
		if ((src0 || src1 || src2 || src3 || src4) == FALSE) {
			if (ChipMajor == GOP_M6_SERIES_MAJOR)
				HAL_GOP_Write2bytemask(REG_01F0_GOPG_BKA4D9, 0x0, REG_01F0_GOPG_BKA4D9_REG_OSDB0_EN);
			else if (ChipMajor >= GOP_MOKONA_SERIES_MAJOR)
				HAL_GOP_Write2bytemask(REG_01F0_GOPG_BKA4D9, 0x0, REG_01F0_GOPG_BKA4D9_REG_OSDB1_EN);
		}
	}

	return GOP_SUCCESS;
}

GOP_Result HAL_GOP_SetGOPEnable2Mode1(GOP_CTX_HAL_LOCAL *pGOPHalLocal, MS_U8 gopNum, MS_BOOL bEnable)
{
	MS_U32 u32Bankoffset = 0;

	_GetBnkOfstByGop(gopNum, &u32Bankoffset);

	HAL_GOP_Write2bytemask(u32Bankoffset + REG_0048_GOP0_BKA4DE, bEnable?1:0, REG_0048_GOP0_BKA4DE_REG_PRE_ALPHA_MD);

	return GOP_SUCCESS;
}

GOP_Result HAL_GOP_GetGOPAlphaMode1(GOP_CTX_HAL_LOCAL *pGOPHalLocal, MS_U8 gopNum, MS_BOOL *pbEnable)
{
	MS_U32 u32RegVal = 0;

	switch(gopNum) {
	case E_GOP0:
		u32RegVal = HAL_GOP_Read2bytemask(REG_0048_GOP0_BKA4DE, REG_0048_GOP0_BKA4DE_REG_PRE_ALPHA_MD);
	break;
	case E_GOP1:
		u32RegVal = HAL_GOP_Read2bytemask(REG_0048_GOP1_BKA4E4, REG_0048_GOP1_BKA4E4_REG_PRE_ALPHA_MD);
	break;
	case E_GOP2:
		u32RegVal = HAL_GOP_Read2bytemask(REG_0048_GOP2_BKA4E8, REG_0048_GOP2_BKA4E8_REG_PRE_ALPHA_MD);
	break;
	case E_GOP3:
		u32RegVal = HAL_GOP_Read2bytemask(REG_0048_GOP3_BKA4EC, REG_0048_GOP3_BKA4EC_REG_PRE_ALPHA_MD);
	break;
	case E_GOP4:
		u32RegVal = HAL_GOP_Read2bytemask(REG_0048_GOPC_BKA4D3, REG_0048_GOPC_BKA4D3_REG_PRE_ALPHA_MD);
	break;
	default:
	break;
	}

	*pbEnable = (MS_BOOL)u32RegVal;

	return GOP_SUCCESS;
}

GOP_Result HAL_GOP_SetGOPHighPri(GOP_CTX_HAL_LOCAL *pGOPHalLocal, MS_U8 gopNum)
{
	return GOP_SUCCESS;
}

GOP_Result HAL_GOP_SetGOPClk(GOP_CTX_HAL_LOCAL *pGOPHalLocal, MS_U8 gopNum, DRV_GOPDstType eDstType)
{
	if (gopNum >= gHalGopChipPro.TotalGOPNum) {
		GOP_H_ERR("[%s][%d] Out of GOP support!!! GOP=%d\n",__FUNCTION__,__LINE__ ,gopNum);
		MS_ASSERT(0);
		return GOP_INVALID_PARAMETERS;
	}

	switch (eDstType) {
	case E_DRV_GOP_DST_IP0:
		if (gopNum == 0)
			HAL_GOP_Write2bytemask(REG_0E34_CKGEN01, 0x1, REG_0E34_CKGEN01_REG_CKG_XC_GOP0_DST_SCTCON);
		else
			GOP_H_ERR("[%s][%d] GOP%d not support blending to IP\n", __FUNCTION__, __LINE__, gopNum);
	break;
	case E_DRV_GOP_DST_OP0:
		if (gopNum == 0)
			HAL_GOP_Write2bytemask(REG_0E34_CKGEN01, 0x0, REG_0E34_CKGEN01_REG_CKG_XC_GOP0_DST_SCTCON);

		if (gopNum == 4)
			HAL_GOP_Write2bytemask(REG_0E40_CKGEN01, 0x0, REG_0E40_CKGEN01_REG_CKG_XC_GOPC_DST_SCTCON);

		HAL_GOP_Write2bytemask(REG_0E44_CKGEN01, 0x0, REG_0E44_CKGEN01_REG_CKG_XC_GOPG_DST_SCTCON);
	break;
	case E_DRV_GOP_DST_FRC:
		if (gopNum == 0)
			HAL_GOP_Write2bytemask(REG_0E34_CKGEN01, 0x0, REG_0E34_CKGEN01_REG_CKG_XC_GOP0_DST_SCTCON);
		if (gopNum == 4)
			HAL_GOP_Write2bytemask(REG_0E40_CKGEN01, 0x0, REG_0E40_CKGEN01_REG_CKG_XC_GOPC_DST_SCTCON);

		HAL_GOP_Write2bytemask(REG_0E44_CKGEN01, 0x4, REG_0E44_CKGEN01_REG_CKG_XC_GOPG_DST_SCTCON);
	break;
	default:
		MS_ASSERT(0);
		return GOP_ENUM_NOT_SUPPORTED;
	}

	return GOP_SUCCESS;
}


GOP_Result HAL_GOP_SetClkForCapture(GOP_CTX_HAL_LOCAL *pGOPHalLocal, DRV_GOP_DWIN_SRC_SEL enSrcSel)
{
    return GOP_SUCCESS;
}
GOP_Result HAL_GOP_SetClock(GOP_CTX_HAL_LOCAL *pGOPHalLocal,MS_BOOL bEnable)
{
	return GOP_SUCCESS;
}

void  HAL_GOP_Init_Context(GOP_CTX_HAL_LOCAL *pGOPHalLocal, GOP_CTX_HAL_SHARED *pHALShared, MS_BOOL bNeedInitShared)
{
    MS_U32 u32GopIdx;

    pGOPHalLocal->pHALShared = pHALShared;

    for(u32GopIdx=0; u32GopIdx < gHalGopChipPro.TotalGOPNum; u32GopIdx++)
    {
        pGOPHalLocal->drvGFlipGOPDst[u32GopIdx] = E_DRV_GOP_DST_OP0;
    }

    pGOPHalLocal->pbIsMuxVaildToGopDst = (MS_BOOL *)bIsMuxVaildToGopDst;
}
void  HAL_GOP_Restore_Ctx(GOP_CTX_HAL_LOCAL *pGOPHalLocal)
{
}

GOP_Result HAL_ConvertAPIAddr(GOP_CTX_HAL_LOCAL *pGOPHalLocal, MS_U8 gwinid, MS_PHY* u64Adr)
{
	return GOP_SUCCESS;
}

GOP_Result HAL_GOP_GetGOPDst(GOP_CTX_HAL_LOCAL *pGOPHalLocal, MS_U8 u8gopNum, DRV_GOPDstType *pGopDst)
{
	MS_U32 ChipMajor = (u32ChipVersion & GOP_CAPS_MAJOR_IPVERSION_OFFSET_MSK) >> GOP_CAPS_MAJOR_IPVERSION_OFFSET_SHT;
	MS_U16 u16Regval = 0;

	if(u8gopNum >= gHalGopChipPro.TotalGOPNum) {
		GOP_H_ERR("[%s][%d] Out of GOP support!!! GOP=%d\n",__FUNCTION__,__LINE__ ,u8gopNum);
		*pGopDst = E_DRV_GOP_DST_INVALID;
		return GOP_FAIL;
	}

	if (ChipMajor >= GOP_MOKA_SERIES_MAJOR) {
		if (u8gopNum == E_GOP0)
			u16Regval = HAL_GOP_Read2bytemask(REG_003C_GOPG_BKA4D9, REG_003C_GOPG_BKA4D9_REG_GOP0_DEST_SEL_003C);
		else if(u8gopNum == E_GOP1)
			u16Regval = HAL_GOP_Read2bytemask(REG_003C_GOPG_BKA4D9, REG_003C_GOPG_BKA4D9_REG_GOP1_DEST_SEL_003C);
		else if(u8gopNum == E_GOP2)
			u16Regval = HAL_GOP_Read2bytemask(REG_003C_GOPG_BKA4D9, REG_003C_GOPG_BKA4D9_REG_GOP2_DEST_SEL_003C);

		switch (u16Regval) {
		case GOP_DST_OP:
			*pGopDst = E_DRV_GOP_DST_OP0;
			break;
		case GOP_DST_VG:
			*pGopDst = E_DRV_GOP_DST_FRC;
			break;
		case GOP_DST_IP:
			*pGopDst = E_DRV_GOP_DST_IP0;
			break;
		default:
			break;
		}
		return GOP_SUCCESS;
	}

	if (u8gopNum == E_GOP0) {
		u16Regval = HAL_GOP_Read2bytemask(REG_0004_GOPG_BKA4D9, REG_0004_GOPG_BKA4D9_REG_GOP0_DEST_SEL);
		if (u16Regval == GOP_DST_IP) {
			*pGopDst = E_DRV_GOP_DST_IP0;
			return GOP_SUCCESS;
		}
	}

	u16Regval = HAL_GOP_Read2bytemask(REG_0004_GOPG_BKA4D9, REG_0004_GOPG_BKA4D9_REG_VG_SEP_PATH);
	if (u16Regval == GOP_DST_OP)
		*pGopDst = E_DRV_GOP_DST_OP0;
	else
		*pGopDst = E_DRV_GOP_DST_FRC;

	return GOP_SUCCESS;

}

void HAL_GOP_SetIPSel2SC(GOP_CTX_HAL_LOCAL *pGOPHalLocal, MS_IPSEL_GOP ipSelGop)
{
	//M6 series no need to set
}

GOP_Result HAL_GOP_DWIN_SetSourceSel(GOP_CTX_HAL_LOCAL *pGOPHalLocal, DRV_GOP_DWIN_SRC_SEL enSrcSel)
{
    return GOP_FUN_NOT_SUPPORTED;
}

GOP_Result HAL_GOP_DWIN_EnableR2YCSC(GOP_CTX_HAL_LOCAL *pGOPHalLocal, MS_BOOL bEnable)
{
    return GOP_FUN_NOT_SUPPORTED;
}

GOP_Result HAL_GOP_MIXER_SetOutputTiming(GOP_CTX_HAL_LOCAL *pGOPHalLocal, MS_U32 u32mode, GOP_DRV_MixerTiming *pTM)
{
    return GOP_FUN_NOT_SUPPORTED;
}

GOP_Result HAL_GOP_MIXER_EnableVfilter(GOP_CTX_HAL_LOCAL *pGOPHalLocal, MS_BOOL bEn)
{
    return GOP_FUN_NOT_SUPPORTED;
}

GOP_Result HAL_GOP_SetUVSwap(GOP_CTX_HAL_LOCAL *pGOPHalLocal, MS_U8 u8GOPNum,MS_BOOL bEn)
{
	MS_U32 u32BankOffSet = 0;

	_GetBnkOfstByGop(u8GOPNum, &u32BankOffSet);

	HAL_GOP_Write2bytemask(u32BankOffSet + REG_0004_GOP0_BKA4DE, bEn?0x1:0x0, REG_0004_GOP0_BKA4DE_REG_UV_SWAP);

	return GOP_SUCCESS;
}

GOP_Result HAL_GOP_SetYCSwap(GOP_CTX_HAL_LOCAL *pGOPHalLocal, MS_U8 u8GOPNum,MS_BOOL bEn)
{
	MS_U32 u32BankOffSet =0;

	_GetBnkOfstByGop(u8GOPNum, &u32BankOffSet);

	HAL_GOP_Write2bytemask(u32BankOffSet + REG_0004_GOP0_BKA4DE, bEn?0x1:0x0, REG_0004_GOP0_BKA4DE_REG_AD_SWAP);

	return GOP_SUCCESS;
}

GOP_Result HAL_GOP_GWIN_GetNewAlphaMode(GOP_CTX_HAL_LOCAL *pGOPHalLocal, MS_U8 u8win, MS_BOOL* pEnable)
{
	MS_U32 u32RegVal = 0;

	if (u8win < GOP1_GwinIdBase) {
		u32RegVal = HAL_GOP_Read2bytemask(REG_0048_GOP0_BKA4DE, REG_0048_GOP0_BKA4DE_REG_PRE_ALPHA_MD);
	} else if (u8win < GOP2_GwinIdBase) {
		u32RegVal = HAL_GOP_Read2bytemask(REG_0048_GOP1_BKA4E4, REG_0048_GOP1_BKA4E4_REG_PRE_ALPHA_MD);
	} else if (u8win < GOP3_GwinIdBase) {
		u32RegVal = HAL_GOP_Read2bytemask(REG_0048_GOP2_BKA4E8, REG_0048_GOP2_BKA4E8_REG_PRE_ALPHA_MD);
	} else if (u8win < GOP4_GwinIdBase) {
		u32RegVal = HAL_GOP_Read2bytemask(REG_0048_GOP3_BKA4EC, REG_0048_GOP3_BKA4EC_REG_PRE_ALPHA_MD);
	} else if (u8win < GOP5_GwinIdBase) {
        u32RegVal = HAL_GOP_Read2bytemask(REG_0048_GOPC_BKA4D3, REG_0048_GOPC_BKA4D3_REG_PRE_ALPHA_MD);
	} else {
		GOP_H_ERR("%s Not support this GWIN num%d!!!\n",__FUNCTION__, u8win);
		return GOP_INVALID_PARAMETERS;
	}

	*pEnable = (MS_BOOL)u32RegVal;

	return GOP_SUCCESS;
}

GOP_Result HAL_GOP_GWIN_SetNewAlphaMode(GOP_CTX_HAL_LOCAL *pGOPHalLocal, MS_U8 u8win, MS_BOOL bEnable)
{
	if (u8win < GOP1_GwinIdBase) {
		HAL_GOP_Write2bytemask(REG_0048_GOP0_BKA4DE, bEnable?1:0, REG_0048_GOP0_BKA4DE_REG_PRE_ALPHA_MD);
	} else if (u8win < GOP2_GwinIdBase) {
		HAL_GOP_Write2bytemask(REG_0048_GOP1_BKA4E4, bEnable?1:0, REG_0048_GOP1_BKA4E4_REG_PRE_ALPHA_MD);
	} else if (u8win < GOP3_GwinIdBase) {
		HAL_GOP_Write2bytemask(REG_0048_GOP2_BKA4E8, bEnable?1:0, REG_0048_GOP2_BKA4E8_REG_PRE_ALPHA_MD);
	} else if (u8win < GOP4_GwinIdBase) {
		HAL_GOP_Write2bytemask(REG_0048_GOP3_BKA4EC, bEnable?1:0, REG_0048_GOP3_BKA4EC_REG_PRE_ALPHA_MD);
	} else if (u8win < GOP5_GwinIdBase) {
		HAL_GOP_Write2bytemask(REG_0048_GOPC_BKA4D3, bEnable?1:0, REG_0048_GOPC_BKA4D3_REG_PRE_ALPHA_MD);
	} else {
		GOP_H_ERR("%s Not support this GWIN num%d!!!\n",__FUNCTION__, u8win);
		return GOP_INVALID_PARAMETERS;
	}

	return GOP_SUCCESS;
}

GOP_Result HAL_GOP_SetGOPToVE(GOP_CTX_HAL_LOCAL *pGOPHalLocal, MS_U8 gopNum, MS_BOOL bEn )
{
    return GOP_FUN_NOT_SUPPORTED;
}

E_GOP_VIDEOTIMING_MIRRORTYPE HAL_GOP_GetVideoTimingMirrorType(GOP_CTX_HAL_LOCAL *pGOPHalLocal, MS_BOOL bHorizontal , EN_DRV_GOP_XC_MIRRORMODE enMirrorMode)
{
	return E_GOP_VIDEOTIMING_MIRROR_MAX;
}

GOP_Result HAL_GOP_OC_SetOCEn(GOP_CTX_HAL_LOCAL *pGOPHalLocal, MS_U8 u8GOP, MS_BOOL bOCEn)
{
    return GOP_SUCCESS;
}

GOP_Result HAL_GOP_OC_SetOCInfo(GOP_CTX_HAL_LOCAL *pGOPHalLocal, DRV_GOP_OC_INFO* pOCinfo)
{
    return GOP_FUN_NOT_SUPPORTED;
}

GOP_Result HAL_GOP_OC_Get_MIU_Sel(GOP_CTX_HAL_LOCAL *pGOPHalLocal, MS_U8 *MIUId)
{
    return GOP_FUN_NOT_SUPPORTED;
}

GOP_Result HAL_GOP_DWIN_SetRingBuffer(GOP_CTX_HAL_LOCAL *pGOPHalLocal, MS_U32 u32RingSize,MS_U32 u32BufSize)
{
	return GOP_FUN_NOT_SUPPORTED;
}

GOP_Result HAL_GOP_AdjustField(GOP_CTX_HAL_LOCAL *pGOPHalLocal, MS_U8 GopNum, DRV_GOPDstType eDstType)
{
    return GOP_FUN_NOT_SUPPORTED;
}

/********************************************************************************/
///GOP Scaling down (internal)
/********************************************************************************/

MS_BOOL HAL_GOP_EnableScalingDownSram(GOP_CTX_HAL_LOCAL *pGOPHalLocal,MS_U8 u8GOP,MS_BOOL bEn)
{
    return GOP_FUN_NOT_SUPPORTED;
}

GOP_Result HAL_GOP_DeleteWinHVSize(GOP_CTX_HAL_LOCAL *pGOPHalLocal,MS_U8 u8GOP, MS_U16 u16HSize, MS_U16 u16VSize)
{
	return GOP_SUCCESS;
}

GOP_Result  HAL_GOP_DumpGOPReg(GOP_CTX_HAL_LOCAL *pGOPHalLocal, MS_U32 u32GopIdx, MS_U16 u16BankIdx, MS_U16 u16Addr, MS_U16* u16Val)
{
    return GOP_SUCCESS;
}

GOP_Result  HAL_GOP_RestoreGOPReg(GOP_CTX_HAL_LOCAL *pGOPHalLocal, MS_U32 u32GopIdx, MS_U16 u16BankIdx, MS_U16 u16Addr, MS_U16 u16Val)
{
	return GOP_SUCCESS;
}

GOP_Result HAL_GOP_PowerState(GOP_CTX_HAL_LOCAL *pGOPHalLocal, MS_U32 u32PowerState, GFLIP_REGS_SAVE_AREA* pGOP_STRPrivate)
{
	return GOP_SUCCESS;
}
GOP_Result HAL_GOP_GWIN_SetGPUTileMode(GOP_CTX_HAL_LOCAL *pGOPHalLocal,MS_U8 gwinid, EN_DRV_GOP_GPU_TILE_MODE tile_mode)
{
	return GOP_SUCCESS;
}

GOP_Result Hal_SetCropWindow(GOP_CTX_HAL_LOCAL *pGOPHalLocal,MS_U8 u8GOP,EN_GOP_CROP_CTL crop_mode)
{
    return GOP_SUCCESS;
}

GOP_Result HAL_GOP_SetGOPMixerValid(GOP_CTX_HAL_LOCAL *pGOPHalLocal, MS_U8 u8Gop, MS_BOOL bEnable)
{
    return GOP_SUCCESS;
}

GOP_Result HAL_GOP_SetDram_Addr(GOP_CTX_HAL_LOCAL *pGOPHalLocal, MS_U8 u8GOP, MS_U8 u8win, MS_PHY PhyAddr, E_DRV_GOP_AddrType eAddrType)
{
	MS_U16 u16tmp = 0;
	MS_U32 u32BankOffSet = 0;
	MS_PHY phytmp = 0;

	_GetBnkOfstByGop(u8GOP, &u32BankOffSet);

	switch(eAddrType) {
	case E_DRV_GOP_RBLKAddr:
		phytmp = (PhyAddr/GOP_STRADDR_WORD_UNIT);
		u16tmp = (MS_U16)(phytmp & 0xFFFF);
		HAL_GOP_Write2bytemask(u32BankOffSet + REG_0008_GOP0_BKA4DE, u16tmp, REG_0008_GOP0_BKA4DE_REG_DRAM_BASE_STR_0);
		u16tmp = (MS_U16)((phytmp & 0x1FFF0000) >> 16);
		HAL_GOP_Write2bytemask(u32BankOffSet + REG_000C_GOP0_BKA4DE, u16tmp, REG_000C_GOP0_BKA4DE_REG_DRAM_BASE_STR_1);
	break;
	default:
	break;
	}

	return GOP_SUCCESS;
}

GOP_Result HAL_GOP_GetDram_Addr(GOP_CTX_HAL_LOCAL *pGOPHalLocal, MS_U8 u8GOP, MS_U8 u8win, MS_PHY* pphyAddr, E_DRV_GOP_AddrType eAddrType)
{
	MS_U16 u16RegVal = 0;
	MS_U32 u32RegVal = 0;
	MS_U32 u32BankOffSet = 0;

	_GetBnkOfstByGop(u8GOP, &u32BankOffSet);

	switch(eAddrType) {
	case E_DRV_GOP_RBLKAddr:
		u16RegVal = HAL_GOP_Read2bytemask(u32BankOffSet + REG_0008_GOP0_BKA4DE, REG_0008_GOP0_BKA4DE_REG_DRAM_BASE_STR_0);
		u32RegVal = (MS_U32)u16RegVal;
		u16RegVal = HAL_GOP_Read2bytemask(u32BankOffSet + REG_000C_GOP0_BKA4DE, REG_000C_GOP0_BKA4DE_REG_DRAM_BASE_STR_1);
		u32RegVal |= ((MS_U32)u16RegVal << 16);
		*pphyAddr = ((MS_PHY)u32RegVal * GOP_STRADDR_WORD_UNIT);
	break;
	default:
		*pphyAddr = 0;
	break;
	}

	return GOP_SUCCESS;
}

GOP_Result MHal_GOP_SetCSCInfo(MS_U32 u32GOP,ST_GOP_CSC_TABLE *pstCSCTbl,MS_BOOL bFlag)
{
    _bHalCSCSelect[u32GOP] = bFlag;
    memcpy(&_stHalCSCTbl[u32GOP],pstCSCTbl,sizeof(ST_GOP_CSC_TABLE));
    return GOP_SUCCESS;
}

GOP_Result HAL_GOP_IsHDREnabled(GOP_CTX_HAL_LOCAL *pGOPHalLocal, MS_BOOL *pbHDREnable)
{
    *pbHDREnable= FALSE;
    return GOP_FUN_NOT_SUPPORTED;
}

GOP_Result HAL_GOP_SetGopGwinHVPixel(GOP_CTX_HAL_LOCAL *pGOPHalLocal, MS_U8 u8GOP, MS_U8 u8win, MS_U16 hstart, MS_U16 hend, MS_U16 vstart, MS_U16 vend)
{
	MS_U32 u32Bankoffset = 0;
	GOP_Result ret = GOP_SUCCESS;

	_GetBnkOfstByGop(u8GOP, &u32Bankoffset);

	HAL_GOP_Write2bytemask(u32Bankoffset + REG_0018_GOP0_BKA4DE, hstart, REG_0018_GOP0_BKA4DE_REG_PIC_X_POS);
	HAL_GOP_Write2bytemask(u32Bankoffset + REG_001C_GOP0_BKA4DE, vstart, REG_001C_GOP0_BKA4DE_REG_PIC_Y_POS);

	return ret;
}

GOP_Result HAL_GOP_SetDbgLevel(EN_GOP_DEBUG_LEVEL level)
{
    u32GOPDbgLevel_hal= level;
	return GOP_SUCCESS;
}
GOP_Result MHal_GOP_VOP_Path_Sel(GOP_CTX_HAL_LOCAL *pGOPHalLocal,EN_DRV_GOP_VOP_PATH enVOPPath)
{
    MS_U32 ChipMajor = (u32ChipVersion & GOP_CAPS_MAJOR_IPVERSION_OFFSET_MSK) >> GOP_CAPS_MAJOR_IPVERSION_OFFSET_SHT;
    MS_U32 ChipMinor = (u32ChipVersion & GOP_CAPS_MINOR_IPVERSION_OFFSET_MSK);

    if (ChipMajor == GOP_M6_SERIES_MAJOR && ChipMinor < GOP_M6L_SERIES_MINOR)
    {
        GOP_H_ERR("[%s][%d] This chip not support\n",__FUNCTION__,__LINE__);
        return GOP_FAIL;
    }

    switch (enVOPPath)
    {
        case E_DRV_GOP_VOPPATH_DEF:
            HAL_GOP_Write2bytemask(REG_0100_SCDISP_BKA4FA, GOP_PQGAMMA_OSDB_LD, REG_0100_SCDISP_BKA4FA_REG_SCDISP_PATH_SEL);
            break;
        case E_DRV_GOP_VOPPATH_BEF_PQGAMMA:
            HAL_GOP_Write2bytemask(REG_0100_SCDISP_BKA4FA, GOP_OSDB_PQGAMMA_LD, REG_0100_SCDISP_BKA4FA_REG_SCDISP_PATH_SEL);
            break;
        case E_DRV_GOP_VOPPATH_AFT_PQGAMMA:
            HAL_GOP_Write2bytemask(REG_0100_SCDISP_BKA4FA, GOP_PQGAMMA_OSDB_LD, REG_0100_SCDISP_BKA4FA_REG_SCDISP_PATH_SEL);
            break;
        case E_DRV_GOP_VOPPATH_AFT_LD:
            HAL_GOP_Write2bytemask(REG_0100_SCDISP_BKA4FA, GOP_PQGAMMA_LD_OSDB, REG_0100_SCDISP_BKA4FA_REG_SCDISP_PATH_SEL);
            break;
        default:
            GOP_H_ERR("[%s][%d] Not support OSDB position %d\n",__FUNCTION__,__LINE__, enVOPPath);
            return GOP_FAIL;
    }

    return GOP_SUCCESS;
}
GOP_Result MHal_GOP_SetCSCCtrl(GOP_CTX_HAL_LOCAL *pGOPHalLocal,MS_U32 u32GOP,ST_GOP_CSC_TABLE *pstCSCTbl)
{
	MS_U32 u32BankOffSet = 0;

	if(pGOPHalLocal == NULL || pstCSCTbl == NULL)
		return GOP_FAIL;

	if(_GetBnkOfstByGop(u32GOP, &u32BankOffSet) != TRUE)
		return GOP_FAIL;

	HAL_GOP_Write2byte(u32BankOffSet + REG_0080_GOP0_BKA4DE, pstCSCTbl->u16CscControl);
	HAL_GOP_Write2bytemask(u32BankOffSet + REG_0084_GOP0_BKA4DE, pstCSCTbl->stCSCMatrix.Matrix[0][0], REG_0084_GOP0_BKA4DE_REG_MAIN_R2Y_SRGB11);
	HAL_GOP_Write2bytemask(u32BankOffSet + REG_0088_GOP0_BKA4DE, pstCSCTbl->stCSCMatrix.Matrix[0][1], REG_0088_GOP0_BKA4DE_REG_MAIN_R2Y_SRGB12);
	HAL_GOP_Write2bytemask(u32BankOffSet + REG_008C_GOP0_BKA4DE, pstCSCTbl->stCSCMatrix.Matrix[0][2], REG_008C_GOP0_BKA4DE_REG_MAIN_R2Y_SRGB13);
	HAL_GOP_Write2bytemask(u32BankOffSet + REG_0090_GOP0_BKA4DE, pstCSCTbl->stCSCMatrix.Matrix[1][0], REG_0090_GOP0_BKA4DE_REG_MAIN_R2Y_SRGB21);
	HAL_GOP_Write2bytemask(u32BankOffSet + REG_0094_GOP0_BKA4DE, pstCSCTbl->stCSCMatrix.Matrix[1][1], REG_0094_GOP0_BKA4DE_REG_MAIN_R2Y_SRGB22);
	HAL_GOP_Write2bytemask(u32BankOffSet + REG_0098_GOP0_BKA4DE, pstCSCTbl->stCSCMatrix.Matrix[1][2], REG_0098_GOP0_BKA4DE_REG_MAIN_R2Y_SRGB23);
	HAL_GOP_Write2bytemask(u32BankOffSet + REG_009C_GOP0_BKA4DE, pstCSCTbl->stCSCMatrix.Matrix[2][0], REG_009C_GOP0_BKA4DE_REG_MAIN_R2Y_SRGB31);
	HAL_GOP_Write2bytemask(u32BankOffSet + REG_00A0_GOP0_BKA4DE, pstCSCTbl->stCSCMatrix.Matrix[2][1], REG_00A0_GOP0_BKA4DE_REG_MAIN_R2Y_SRGB32);
	HAL_GOP_Write2bytemask(u32BankOffSet + REG_00A4_GOP0_BKA4DE, pstCSCTbl->stCSCMatrix.Matrix[2][2], REG_00A4_GOP0_BKA4DE_REG_MAIN_R2Y_SRGB33);
	HAL_GOP_Write2bytemask(u32BankOffSet + REG_00B4_GOP0_BKA4DE, pstCSCTbl->u16BrightnessOffsetR, REG_00B4_GOP0_BKA4DE_REG_R_OFFSET);
	HAL_GOP_Write2bytemask(u32BankOffSet + REG_00B0_GOP0_BKA4DE, pstCSCTbl->u16BrightnessOffsetG, REG_00B0_GOP0_BKA4DE_REG_G_OFFSET);
	HAL_GOP_Write2bytemask(u32BankOffSet + REG_00AC_GOP0_BKA4DE, pstCSCTbl->u16BrightnessOffsetB, REG_00AC_GOP0_BKA4DE_REG_B_OFFSET);

	return GOP_SUCCESS;
}

GOP_Result MHal_GOP_AutoDetectBuf(GOP_CTX_HAL_LOCAL *pstGOPHalLocal, ST_GOP_AUTO_DETECT_BUF_INFO* pstAutoDectInfo)
{
	//m6 series not support
	return GOP_SUCCESS;
}

GOP_Result _MHal_GOP_SetCscMatrix(GOP_CTX_HAL_LOCAL *pstGOPHalLocal, MS_U8 u8GOP,MS_U16 *pu16CscMatrix)
{
	MS_U32 u32BankOffSet = 0;

	if(_GetBnkOfstByGop(u8GOP, &u32BankOffSet) != TRUE) {
		return GOP_FAIL;
	}

	HAL_GOP_Write2byte(u32BankOffSet + REG_0080_GOP0_BKA4DE, pu16CscMatrix[0]);
	HAL_GOP_Write2bytemask(u32BankOffSet + REG_0084_GOP0_BKA4DE, pu16CscMatrix[1], REG_0084_GOP0_BKA4DE_REG_MAIN_R2Y_SRGB11);
	HAL_GOP_Write2bytemask(u32BankOffSet + REG_0088_GOP0_BKA4DE, pu16CscMatrix[2], REG_0088_GOP0_BKA4DE_REG_MAIN_R2Y_SRGB12);
	HAL_GOP_Write2bytemask(u32BankOffSet + REG_008C_GOP0_BKA4DE, pu16CscMatrix[3], REG_008C_GOP0_BKA4DE_REG_MAIN_R2Y_SRGB13);
	HAL_GOP_Write2bytemask(u32BankOffSet + REG_0090_GOP0_BKA4DE, pu16CscMatrix[4], REG_0090_GOP0_BKA4DE_REG_MAIN_R2Y_SRGB21);
	HAL_GOP_Write2bytemask(u32BankOffSet + REG_0094_GOP0_BKA4DE, pu16CscMatrix[5], REG_0094_GOP0_BKA4DE_REG_MAIN_R2Y_SRGB22);
	HAL_GOP_Write2bytemask(u32BankOffSet + REG_0098_GOP0_BKA4DE, pu16CscMatrix[6], REG_0098_GOP0_BKA4DE_REG_MAIN_R2Y_SRGB23);
	HAL_GOP_Write2bytemask(u32BankOffSet + REG_009C_GOP0_BKA4DE, pu16CscMatrix[7], REG_009C_GOP0_BKA4DE_REG_MAIN_R2Y_SRGB31);
	HAL_GOP_Write2bytemask(u32BankOffSet + REG_00A0_GOP0_BKA4DE, pu16CscMatrix[8], REG_00A0_GOP0_BKA4DE_REG_MAIN_R2Y_SRGB32);
	HAL_GOP_Write2bytemask(u32BankOffSet + REG_00A4_GOP0_BKA4DE, pu16CscMatrix[9], REG_00A4_GOP0_BKA4DE_REG_MAIN_R2Y_SRGB33);

	return GOP_SUCCESS;
}

GOP_Result MHal_GOP_GWIN_SetColorMatrix(GOP_CTX_HAL_LOCAL *pstGOPHalLocal, MS_U8 u8GOP, GOP_OupputColor enGopOutColor, DRV_GOPColorType enColorType, MS_BOOL bCscEnable)
{
	MS_U32 u32BankOffSet = 0;

	if(_GetBnkOfstByGop(u8GOP, &u32BankOffSet) != TRUE)
		return GOP_FAIL;

	switch(enColorType) {
	case E_DRV_GOP_COLOR_YUV422:
	case E_DRV_GOP_COLOR_AYUV8888:
		if(enGopOutColor == DRV_GOPOUT_RGB) {
			if(bCscEnable == FALSE)
				_MHal_GOP_SetCscMatrix(pstGOPHalLocal, u8GOP,gGopLimitY2RFullBT709);
			else
				_MHal_GOP_SetCscMatrix(pstGOPHalLocal, u8GOP,gu16GopY2rCSC);
		} else {
			if(bCscEnable == FALSE)
				HAL_GOP_Write2bytemask(u32BankOffSet + REG_0080_GOP0_BKA4DE, 0x0, REG_0080_GOP0_BKA4DE_REG_MAIN_R2Y_COL3X3_EN);
		}
	break;
	case E_DRV_GOP_COLOR_RGB555_BLINK:
	case E_DRV_GOP_COLOR_RGB565:
	case E_DRV_GOP_COLOR_ARGB4444:
	case E_DRV_GOP_COLOR_2266:
	case E_DRV_GOP_COLOR_I8:
	case E_DRV_GOP_COLOR_ARGB8888:
	case E_DRV_GOP_COLOR_ARGB1555:
	case E_DRV_GOP_COLOR_ABGR8888:
	case E_DRV_GOP_COLOR_RGB555YUV422:
	case E_DRV_GOP_COLOR_RGBA5551:
	case E_DRV_GOP_COLOR_RGBA4444:
	case E_DRV_GOP_COLOR_RGBA8888:
	case E_DRV_GOP_COLOR_BGR565:
	case E_DRV_GOP_COLOR_ABGR4444:
	case E_DRV_GOP_COLOR_ABGR1555:
	case E_DRV_GOP_COLOR_BGRA5551:
	case E_DRV_GOP_COLOR_BGRA4444:
	case E_DRV_GOP_COLOR_BGRA8888:
	default:
		if(enGopOutColor == DRV_GOPOUT_YUV) {
			if(bCscEnable == FALSE)
				_MHal_GOP_SetCscMatrix(pstGOPHalLocal, u8GOP,gGopFullR2YLimitBT709);
		} else {
			if(bCscEnable == FALSE)
				HAL_GOP_Write2bytemask(u32BankOffSet + REG_0080_GOP0_BKA4DE, 0x0, REG_0080_GOP0_BKA4DE_REG_MAIN_R2Y_COL3X3_EN);
			else
				_MHal_GOP_SetCscMatrix(pstGOPHalLocal, u8GOP,gGopIdentity);
		}
	break;
	}

	return GOP_SUCCESS;
}

GOP_Result MHAL_GOP_Load_HStretch_New4TapMode_Table(GOP_CTX_HAL_LOCAL *pGOPHalLocal, MS_U8 u8GOP, MS_U16* pu16GopHStretchTable)
{
	MS_U8 u8CoefIdx = 0;
	MS_U32 u32BankOffSet = 0;

	if (u8GOP >= gHalGopChipPro.TotalGOPNum) {
		GOP_H_ERR("[%s][%d] Out of GOP support!!! GOP=%d\n",
		__FUNCTION__, __LINE__, u8GOP);
		return GOP_FAIL;
	}

    _GetBnkOfstByGop(u8GOP, &u32BankOffSet);

	while (u8CoefIdx < GOP_HSTRETCH_4TAP_TABLE_NUM) {
		switch (u8CoefIdx) {
		case GOP_HSTRETCH_4TAP_COEF0_00:
			HAL_GOP_Write2bytemask(u32BankOffSet + REG_00C4_GOP0_HVSP_BKA4E1, pu16GopHStretchTable[u8CoefIdx], REG_00C4_GOP0_HVSP_BKA4E1_REG_COEF1_00_C_H_00C4);
			HAL_GOP_Write2bytemask(u32BankOffSet + REG_0054_GOP0_HVSP_BKA4E1, pu16GopHStretchTable[u8CoefIdx], REG_0054_GOP0_HVSP_BKA4E1_REG_COEF1_00_Y_H_0054);
			HAL_GOP_Write2bytemask(u32BankOffSet + REG_00C0_GOP0_HVSP_BKA4E1, pu16GopHStretchTable[u8CoefIdx+1], REG_00C0_GOP0_HVSP_BKA4E1_REG_COEF0_00_C_H_00C0);
			HAL_GOP_Write2bytemask(u32BankOffSet + REG_0050_GOP0_HVSP_BKA4E1, pu16GopHStretchTable[u8CoefIdx+1], REG_0050_GOP0_HVSP_BKA4E1_REG_COEF0_00_Y_H_0050);
			break;
		case GOP_HSTRETCH_4TAP_COEF0_25:
			HAL_GOP_Write2bytemask(u32BankOffSet + REG_00CC_GOP0_HVSP_BKA4E1, pu16GopHStretchTable[u8CoefIdx], REG_00CC_GOP0_HVSP_BKA4E1_REG_COEF1_25_C_H_00CC);
			HAL_GOP_Write2bytemask(u32BankOffSet + REG_005C_GOP0_HVSP_BKA4E1, pu16GopHStretchTable[u8CoefIdx], REG_005C_GOP0_HVSP_BKA4E1_REG_COEF1_25_Y_H_005C);
			HAL_GOP_Write2bytemask(u32BankOffSet + REG_00C8_GOP0_HVSP_BKA4E1, pu16GopHStretchTable[u8CoefIdx+1], REG_00C8_GOP0_HVSP_BKA4E1_REG_COEF0_25_C_H_00C8);
			HAL_GOP_Write2bytemask(u32BankOffSet + REG_0058_GOP0_HVSP_BKA4E1, pu16GopHStretchTable[u8CoefIdx+1], REG_0058_GOP0_HVSP_BKA4E1_REG_COEF0_25_Y_H_0058);
			break;
		case GOP_HSTRETCH_4TAP_COEF3_25:
			HAL_GOP_Write2bytemask(u32BankOffSet + REG_00D4_GOP0_HVSP_BKA4E1, pu16GopHStretchTable[u8CoefIdx], REG_00D4_GOP0_HVSP_BKA4E1_REG_COEF3_25_C_H_00D4);
			HAL_GOP_Write2bytemask(u32BankOffSet + REG_0064_GOP0_HVSP_BKA4E1, pu16GopHStretchTable[u8CoefIdx], REG_0064_GOP0_HVSP_BKA4E1_REG_COEF3_25_Y_H_0064);
			HAL_GOP_Write2bytemask(u32BankOffSet + REG_00D0_GOP0_HVSP_BKA4E1, pu16GopHStretchTable[u8CoefIdx+1], REG_00D0_GOP0_HVSP_BKA4E1_REG_COEF2_25_C_H_00D0);
			HAL_GOP_Write2bytemask(u32BankOffSet + REG_0060_GOP0_HVSP_BKA4E1, pu16GopHStretchTable[u8CoefIdx+1], REG_0060_GOP0_HVSP_BKA4E1_REG_COEF2_25_Y_H_0060);
			break;
		case GOP_HSTRETCH_4TAP_COEF0_50:
			HAL_GOP_Write2bytemask(u32BankOffSet + REG_00DC_GOP0_HVSP_BKA4E1, pu16GopHStretchTable[u8CoefIdx], REG_00DC_GOP0_HVSP_BKA4E1_REG_COEF1_50_C_H_00DC);
			HAL_GOP_Write2bytemask(u32BankOffSet + REG_006C_GOP0_HVSP_BKA4E1, pu16GopHStretchTable[u8CoefIdx], REG_006C_GOP0_HVSP_BKA4E1_REG_COEF1_50_Y_H_006C);
			HAL_GOP_Write2bytemask(u32BankOffSet + REG_00D8_GOP0_HVSP_BKA4E1, pu16GopHStretchTable[u8CoefIdx+1], REG_00D8_GOP0_HVSP_BKA4E1_REG_COEF0_50_C_H_00D8);
			HAL_GOP_Write2bytemask(u32BankOffSet + REG_0068_GOP0_HVSP_BKA4E1, pu16GopHStretchTable[u8CoefIdx+1], REG_0068_GOP0_HVSP_BKA4E1_REG_COEF0_50_Y_H_0068);
			break;
		}
		u8CoefIdx = u8CoefIdx + GOP_STRETCH_4TAP_PROCESS_IDX;
	}

	return GOP_SUCCESS;
}

GOP_Result MHAL_GOP_Load_VStretch_V4TapMode_Table(GOP_CTX_HAL_LOCAL *pGOPHalLocal, MS_U8 u8GOP, MS_U16* pu16GopVStretchTable)
{
	return GOP_SUCCESS;
}

GOP_Result MHal_GOP_AutoDetectBuf_V2(GOP_CTX_HAL_LOCAL *pstGOPHalLocal, ST_GOP_AUTO_DETECT_BUF_INFO* pstAutoDectInfo)
{
	//m6 series not support
	return GOP_SUCCESS;
}

GOP_Result MHal_GOP_GetOsdNonTransCnt(GOP_CTX_HAL_LOCAL *pstGOPHalLocal, MS_U32* pu32Count)
{
	MS_U16 u16RegValMSB = 0,  u16RegValLSB = 0;

	u16RegValLSB = HAL_GOP_Read2bytemask(REG_0058_GOPG_BKA4D9, REG_0058_GOPG_BKA4D9_REG_MIXER4_OSD_ROI_RECORD_0);
	u16RegValMSB = HAL_GOP_Read2bytemask(REG_005C_GOPG_BKA4D9, REG_005C_GOPG_BKA4D9_REG_MIXER4_OSD_ROI_RECORD_1);
	*pu32Count = (((MS_U32)u16RegValMSB) << REG_GOP_NONTRANS_COUNT_SHITF) | ((MS_U32)u16RegValLSB);

	return GOP_SUCCESS;
}

GOP_Result MHal_GOP_Trigger_MUX(GOP_CTX_HAL_LOCAL *pstGOPHalLocal)
{
	HAL_GOP_Write2bytemask(REG_0020_GOPG_BKA4D9, 0x1, REG_0020_GOPG_BKA4D9_REG_GOPG_CFG_TRIG);

	return GOP_SUCCESS;
}

GOP_Result MHal_GOP_FillCscMartrix(GOP_CTX_HAL_LOCAL *pstGOPHalLocal,MS_U16 *pu16CSCValue, EN_GOP_CFD_MC_FORMAT enInFmt, EN_GOP_CFD_MC_FORMAT enOutFmt)
{
    if(enInFmt == E_GOP_CFD_MC_FORMAT_YUV422 && enOutFmt == E_GOP_CFD_MC_FORMAT_RGB)
    {
        memcpy(gu16GopY2rCSC,pu16CSCValue,sizeof(MS_U16)*GOP_CSC_ELEMENT_NUM);
    }
    else if (enInFmt == E_GOP_CFD_MC_FORMAT_RGB && enOutFmt == E_GOP_CFD_MC_FORMAT_RGB)
    {
        memcpy(gu16GopR2rCSC,pu16CSCValue,sizeof(MS_U16)*GOP_CSC_ELEMENT_NUM);
    }

    return GOP_SUCCESS;
}

GOP_Result HAL_GOP_GWIN_SetHMirror(GOP_CTX_HAL_LOCAL *pstGOPHalLocal, MS_U8 u8GOP, MS_BOOL bEnable)
{
	return GOP_SUCCESS;
}

GOP_Result HAL_GOP_SetVMirror(GOP_CTX_HAL_LOCAL *pstGOPHalLocal, MS_U8 u8GOP, MS_BOOL bEnable)
{
	MS_U32 u32Bankoffset = 0;

	_GetBnkOfstByGop(u8GOP, &u32Bankoffset);
	HAL_GOP_Write2bytemask(u32Bankoffset + REG_0004_GOP0_BKA4DE, bEnable?1:0, REG_0004_GOP0_BKA4DE_REG_MIRROR_V);

	return GOP_SUCCESS;
}

MS_BOOL HAL_GOP_GetVMirror(GOP_CTX_HAL_LOCAL *pstGOPHalLocal, MS_U8 u8GOP)
{
	MS_BOOL bEnable;
	MS_U16 u16RegVal = 0;
	MS_U32 u32Bankoffset = 0;

	_GetBnkOfstByGop(u8GOP, &u32Bankoffset);
	u16RegVal = HAL_GOP_Read2bytemask(u32Bankoffset + REG_0004_GOP0_BKA4DE, REG_0004_GOP0_BKA4DE_REG_MIRROR_V);
	bEnable = (MS_BOOL)u16RegVal;

	return bEnable;
}

MS_BOOL HAL_GOP_GetHMirror(GOP_CTX_HAL_LOCAL *pstGOPHalLocal, MS_U8 u8GOP)
{
	return FALSE;
}

GOP_Result HAL_GOP_SetGWIN_Enable(GOP_CTX_HAL_LOCAL *pstGOPHalLocal, MS_U8 u8GOP, MS_U8 u8win, MS_BOOL bEnable)
{
	MS_BOOL src0, src1, src2, src3;
	MS_U32 u32Bankoffset = 0;

	_GetBnkOfstByGop(u8GOP, &u32Bankoffset);
	HAL_GOP_Write2bytemask(u32Bankoffset + REG_0004_GOP0_BKA4DE, bEnable?1:0, REG_0004_GOP0_BKA4DE_REG_GOP_EN);
	switch(u8GOP) {
	case 0:
		HAL_GOP_Write2bytemask(REG_0004_GOP0_BKA4DE, bEnable?1:0, REG_0004_GOP0_BKA4DE_REG_GOP_EN);
		HAL_GOP_Write2bytemask(REG_004C_GOPG_BKA4D9, bEnable?1:0, REG_004C_GOPG_BKA4D9_REG_MIXER4_SRC0_EN);
		break;
	case 1:
		HAL_GOP_Write2bytemask(REG_0004_GOP1_BKA4E4, bEnable?1:0, REG_0004_GOP1_BKA4E4_REG_GOP_EN);
		HAL_GOP_Write2bytemask(REG_004C_GOPG_BKA4D9, bEnable?1:0, REG_004C_GOPG_BKA4D9_REG_MIXER4_SRC1_EN);
		break;
	case 2:
		HAL_GOP_Write2bytemask(REG_0004_GOP2_BKA4E8, bEnable?1:0, REG_0004_GOP2_BKA4E8_REG_GOP_EN);
		HAL_GOP_Write2bytemask(REG_004C_GOPG_BKA4D9, bEnable?1:0, REG_004C_GOPG_BKA4D9_REG_MIXER4_SRC2_EN);
		break;
	case 3:
		HAL_GOP_Write2bytemask(REG_0004_GOP3_BKA4EC, bEnable?1:0, REG_0004_GOP3_BKA4EC_REG_GOP_EN);
		HAL_GOP_Write2bytemask(REG_004C_GOPG_BKA4D9, bEnable?1:0, REG_004C_GOPG_BKA4D9_REG_MIXER4_SRC3_EN);
		break;
	case 4:
		HAL_GOP_Write2bytemask(REG_0004_GOPC_BKA4D3, bEnable?1:0, REG_0004_GOPC_BKA4D3_REG_GOP_EN);
		HAL_GOP_Write2bytemask(REG_006C_GOPG_BKA4D9, bEnable?1:0, REG_006C_GOPG_BKA4D9_REG_MIXER2_SRC1_EN);
		break;
	default:
		break;
	}

	if (bEnable) {
		if (u8GOP != GOPCursor)
			HAL_GOP_Write2bytemask(REG_006C_GOPG_BKA4D9, 1, REG_006C_GOPG_BKA4D9_REG_MIXER2_SRC0_EN);
	} else {
		src0 = HAL_GOP_Read2bytemask(REG_004C_GOPG_BKA4D9, REG_004C_GOPG_BKA4D9_REG_MIXER4_SRC0_EN);
		src1 = HAL_GOP_Read2bytemask(REG_004C_GOPG_BKA4D9, REG_004C_GOPG_BKA4D9_REG_MIXER4_SRC1_EN);
		src2 = HAL_GOP_Read2bytemask(REG_004C_GOPG_BKA4D9, REG_004C_GOPG_BKA4D9_REG_MIXER4_SRC2_EN);
		src3 = HAL_GOP_Read2bytemask(REG_004C_GOPG_BKA4D9, REG_004C_GOPG_BKA4D9_REG_MIXER4_SRC3_EN);
		if ((src0 || src1 || src2 || src3) == FALSE)
			HAL_GOP_Write2bytemask(REG_006C_GOPG_BKA4D9, 0x0, REG_006C_GOPG_BKA4D9_REG_MIXER2_SRC0_EN);
	}

	return GOP_SUCCESS;
}

GOP_Result HAL_GOP_GetWinFmt(GOP_CTX_HAL_LOCAL *pstGOPHalLocal, MS_U8 u8GOP, MS_U8 u8Win, MS_U16 *pu16Colortype)
{
	MS_BOOL bUVSwap = 0, bADSwap = 0;
	MS_U16 u16RegVal = 0;
	MS_U32 u32Bankoffset = 0;

	_GetBnkOfstByGop(u8GOP, &u32Bankoffset);
	u16RegVal = HAL_GOP_Read2bytemask(u32Bankoffset + REG_0004_GOP0_BKA4DE, REG_0004_GOP0_BKA4DE_REG_DATA_FMT);
	bUVSwap = HAL_GOP_Read2bytemask(u32Bankoffset + REG_0004_GOP0_BKA4DE, REG_0004_GOP0_BKA4DE_REG_UV_SWAP);
	bADSwap = HAL_GOP_Read2bytemask(u32Bankoffset + REG_0004_GOP0_BKA4DE, REG_0004_GOP0_BKA4DE_REG_AD_SWAP);

	switch(u16RegVal) {
	case GOP_FMT_ARGB4444:
		if (bUVSwap == FALSE && bADSwap == FALSE)
			*pu16Colortype = GOP_FMT_ARGB4444;
		else if (bUVSwap == TRUE && bADSwap == FALSE)
			*pu16Colortype = GOP_FMT_ABGR4444;
		else if (bUVSwap == FALSE && bADSwap == TRUE)
			*pu16Colortype = GOP_FMT_RGBA4444;
		else
			*pu16Colortype = GOP_FMT_BGRA4444;
		break;
	case GOP_FMT_ARGB8888:
		if (bUVSwap == FALSE && bADSwap == FALSE)
			*pu16Colortype = GOP_FMT_ARGB8888;
		else if (bUVSwap == TRUE && bADSwap == FALSE)
			*pu16Colortype = GOP_FMT_ABGR8888;
		else if (bUVSwap == FALSE && bADSwap == TRUE)
			*pu16Colortype = GOP_FMT_RGBA8888;
		else
			*pu16Colortype = GOP_FMT_BGRA8888;
		break;
	case GOP_FMT_ARGB1555:
		if (bUVSwap == FALSE && bADSwap == FALSE)
			*pu16Colortype = GOP_FMT_ARGB1555;
		else if (bUVSwap == TRUE && bADSwap == FALSE)
			*pu16Colortype = GOP_FMT_ABGR1555;
		else if (bUVSwap == FALSE && bADSwap == TRUE)
			*pu16Colortype = GOP_FMT_RGBA5551;
		else
			*pu16Colortype = GOP_FMT_BGRA5551;
		break;
	case GOP_FMT_RGB565:
		if (bUVSwap == FALSE)
			*pu16Colortype = GOP_FMT_RGB565;
		else
			*pu16Colortype = GOP_FMT_BGR565;
		break;
	case GOP_FMT_YUV422:
		*pu16Colortype = GOP_FMT_YUV422;
		break;
	default:
		break;
	}

	*pu16Colortype = (*pu16Colortype << 4);

	return GOP_SUCCESS;
}

GOP_Result HAL_GOP_SetPitch(GOP_CTX_HAL_LOCAL *pstGOPHalLocal, MS_U8 u8GOP, MS_U8 u8Win, MS_U16 u16FbWidth, MS_U16 u16FbHeight)
{
	MS_U32 u32Bankoffset = 0;

	_GetBnkOfstByGop(u8GOP, &u32Bankoffset);

	HAL_GOP_Write2bytemask(u32Bankoffset + REG_0010_GOP0_BKA4DE, u16FbWidth, REG_0010_GOP0_BKA4DE_REG_PIC_HSIZE);
	HAL_GOP_Write2bytemask(u32Bankoffset + REG_0014_GOP0_BKA4DE, u16FbHeight, REG_0014_GOP0_BKA4DE_REG_PIC_VSIZE);

	return GOP_SUCCESS;
}

MS_U16 HAL_GOP_GetPitch(GOP_CTX_HAL_LOCAL *pstGOPHalLocal, MS_U8 u8GOP, MS_U8 u8Win)
{
	MS_U16 u16RegVal = 0;
	MS_U32 u32Bankoffset = 0;

	_GetBnkOfstByGop(u8GOP, &u32Bankoffset);

	u16RegVal = HAL_GOP_Read2bytemask(u32Bankoffset + REG_0010_GOP0_BKA4DE, REG_0010_GOP0_BKA4DE_REG_PIC_HSIZE);

	return u16RegVal;
}

GOP_Result HAL_GOP_GetGopGwinHVPixel(GOP_CTX_HAL_LOCAL *pstGOPHalLocal, MS_U8 u8GOP, MS_U8 u8Win, MS_U16 *u16Hstart, MS_U16 *u16Hend, MS_U16 *u16Vstart, MS_U16 *u16Vend)
{
	MS_U16 u16Dis_H_size = 0, u16Dis_V_size = 0;
	MS_U32 u32Bankoffset = 0;

	_GetBnkOfstByGop(u8GOP, &u32Bankoffset);

	*u16Hstart = HAL_GOP_Read2bytemask(u32Bankoffset + REG_0018_GOP0_BKA4DE, REG_0018_GOP0_BKA4DE_REG_PIC_X_POS);
	*u16Vstart = HAL_GOP_Read2bytemask(u32Bankoffset + REG_001C_GOP0_BKA4DE, REG_001C_GOP0_BKA4DE_REG_PIC_Y_POS);

	u16Dis_H_size = HAL_GOP_Read2bytemask(u32Bankoffset + REG_0020_GOP0_BKA4DE, REG_0020_GOP0_BKA4DE_REG_DISP_HSIZE);
	u16Dis_V_size = HAL_GOP_Read2bytemask(u32Bankoffset + REG_0024_GOP0_BKA4DE, REG_0024_GOP0_BKA4DE_REG_DISP_VSIZE);
	*u16Hend = *u16Hstart + u16Dis_H_size;
	*u16Vend = *u16Vstart + u16Dis_V_size;

	return GOP_SUCCESS;
}

GOP_Result HAL_GOP_GWIN_SetProgressive(GOP_CTX_HAL_LOCAL *pstGOPHalLocal, MS_U8 u8GOP, MS_BOOL bEnable)
{
	return GOP_SUCCESS;
}

MS_BOOL HAL_GOP_GWIN_GetProgressive(GOP_CTX_HAL_LOCAL *pstGOPHalLocal, MS_U8 u8GOP)
{
	return TRUE;
}

GOP_Result HAL_GOP_SetGopStretchWin(GOP_CTX_HAL_LOCAL *pstGOPHalLocal, MS_U8 u8GOP, MS_U16 u16x, MS_U16 u16y, MS_U16 u16width, MS_U16 u16height)
{
	MS_U32 u32Bankoffset = 0;
	MS_U32 ChipMajor = (u32ChipVersion & GOP_CAPS_MAJOR_IPVERSION_OFFSET_MSK) >> GOP_CAPS_MAJOR_IPVERSION_OFFSET_SHT;
	MS_U32 ChipMinor = (u32ChipVersion & GOP_CAPS_MINOR_IPVERSION_OFFSET_MSK);

	_GetBnkOfstByGop(u8GOP, &u32Bankoffset);

	HAL_GOP_Write2bytemask(u32Bankoffset + REG_0028_GOP0_BKA4DE, u16x, REG_0028_GOP0_BKA4DE_REG_DISP_HSTR);
	HAL_GOP_Write2bytemask(u32Bankoffset + REG_002C_GOP0_BKA4DE, u16y, REG_002C_GOP0_BKA4DE_REG_DISP_VSTR);
	HAL_GOP_Write2bytemask(u32Bankoffset + REG_0020_GOP0_BKA4DE, u16width, REG_0020_GOP0_BKA4DE_REG_DISP_HSIZE);
	HAL_GOP_Write2bytemask(u32Bankoffset + REG_0024_GOP0_BKA4DE, u16height, REG_0024_GOP0_BKA4DE_REG_DISP_VSIZE);

	if (ChipMajor == GOP_MOKA_SERIES_MAJOR && ChipMinor == GOP_MOKA_SERIES_MINOR) {
		if (u8GOP == E_GOP1) {
			if (u16width > GOP_VSCALE_V4_LIMIT)
				 HAL_GOP_Write2bytemask(u32Bankoffset + REG_004C_GOP0_HVSP_BKA4E1, 0, REG_004C_GOP0_HVSP_BKA4E1_REG_12LB_EN_004C);
			else
				 HAL_GOP_Write2bytemask(u32Bankoffset + REG_004C_GOP0_HVSP_BKA4E1, 1, REG_004C_GOP0_HVSP_BKA4E1_REG_12LB_EN_004C);
		}
	}

	return GOP_SUCCESS;
}

GOP_Result HAL_GOP_GetGopStretchWin(GOP_CTX_HAL_LOCAL *pstGOPHalLocal, MS_U8 u8GOP, MS_U16 *u16x, MS_U16 *u16y, MS_U16 *u16width, MS_U16 *u16height)
{
	MS_U32 u32Bankoffset = 0;

	_GetBnkOfstByGop(u8GOP, &u32Bankoffset);

	*u16x = HAL_GOP_Read2bytemask(u32Bankoffset + REG_0028_GOP0_BKA4DE, REG_0028_GOP0_BKA4DE_REG_DISP_HSTR);
	*u16y = HAL_GOP_Read2bytemask(u32Bankoffset + REG_002C_GOP0_BKA4DE, REG_002C_GOP0_BKA4DE_REG_DISP_VSTR);
	*u16width = HAL_GOP_Read2bytemask(u32Bankoffset + REG_0020_GOP0_BKA4DE, REG_0020_GOP0_BKA4DE_REG_DISP_HSIZE);
	*u16height = HAL_GOP_Read2bytemask(u32Bankoffset + REG_0024_GOP0_BKA4DE, REG_0024_GOP0_BKA4DE_REG_DISP_VSIZE);

	return GOP_SUCCESS;
}

GOP_Result HAL_GOP_GWIN_SetAlphaInverse(GOP_CTX_HAL_LOCAL *pstGOPHalLocal, MS_U8 u8GOP, MS_BOOL bEnable)
{
	MS_U32 u32Bankoffset = 0;

	_GetBnkOfstByGop(u8GOP, &u32Bankoffset);
	//m6 family always disable alpha inverse
	HAL_GOP_Write2bytemask(u32Bankoffset + REG_0004_GOP0_BKA4DE, 0x0, REG_0004_GOP0_BKA4DE_REG_ALPHA_INV);

	return GOP_SUCCESS;
}

GOP_Result HAL_GOP_GetGopScaleRatio(GOP_CTX_HAL_LOCAL *pstGOPHalLocal, MS_U8 u8GOP, MS_U32 *u32Hratio, MS_U32 *u32Vratio)
{
	MS_U16 u16Regval = 0;
	MS_U32 u32Bankoffset = 0;

	_GetBnkOfstByGop(u8GOP, &u32Bankoffset);

	u16Regval = HAL_GOP_Read2bytemask(u32Bankoffset + REG_0058_GOP0_BKA4DE, REG_0058_GOP0_BKA4DE_REG_HSP_RATIO_0);
	*u32Hratio = (MS_U32)u16Regval;
	u16Regval = HAL_GOP_Read2bytemask(u32Bankoffset + REG_005C_GOP0_BKA4DE, REG_005C_GOP0_BKA4DE_REG_HSP_RATIO_1);
	*u32Hratio |= (((MS_U32)u16Regval << 16) & REG_GOP_HVSP1_HRATIO_MSB_MSK);

	u16Regval = HAL_GOP_Read2bytemask(u32Bankoffset + REG_0060_GOP0_BKA4DE, REG_0060_GOP0_BKA4DE_REG_VSP_RATIO_0);
	*u32Vratio = (MS_U32)u16Regval;
	u16Regval = HAL_GOP_Read2bytemask(u32Bankoffset + REG_0064_GOP0_BKA4DE, REG_0064_GOP0_BKA4DE_REG_VSP_RATIO_1);
	*u32Vratio |= (((MS_U32)u16Regval << 16) & REG_GOP_HVSP1_VRATIO_MSB_MSK);

	return GOP_SUCCESS;
}

GOP_Result HAL_GOP_SetGopVaildH(GOP_CTX_HAL_LOCAL *pstGOPHalLocal, MS_U8 u8GOP, MS_U16 u16ValidH)
{
	return GOP_SUCCESS;
}

GOP_Result HAL_GOP_SetGopVaildV(GOP_CTX_HAL_LOCAL *pstGOPHalLocal, MS_U8 u8GOP, MS_U16 u16ValidV)
{
	return GOP_SUCCESS;
}

GOP_Result HAL_GOP_GWIN_SetTransClr(GOP_CTX_HAL_LOCAL *pstGOPHalLocal, MS_U8 u8GOP, GOP_TransClrFmt fmt, MS_BOOL bEnable)
{
	MS_U32 u32Bankoffset = 0;

	_GetBnkOfstByGop(u8GOP, &u32Bankoffset);

	HAL_GOP_Write2bytemask(u32Bankoffset + REG_0050_GOP0_BKA4DE, bEnable?1:0, REG_0050_GOP0_BKA4DE_REG_TRANS_CLR_EN);

	return GOP_SUCCESS;
}

EN_DRV_GOP_VOP_PATH Hal_GOP_GetVOPPathSel(GOP_CTX_HAL_LOCAL *pstGOPHalLocal)
{
	return E_DRV_GOP_VOPPATH_DEF;
}

GOP_Result HAL_GOP_SetPipe(GOP_CTX_HAL_LOCAL *pstGOPHalLocal, MS_U8 u8GOP, MS_U16 u16HSPipe)
{
	return GOP_SUCCESS;
}

GOP_Result HAL_GOP_SetVOPNBL(GOP_CTX_HAL_LOCAL *pstGOPHalLocal, MS_BOOL bEnable)
{
	return GOP_SUCCESS;
}

MS_BOOL HAL_GOP_GetGWINEnable(GOP_CTX_HAL_LOCAL *pstGOPHalLocal, MS_U8 u8GOP, MS_U8 u8Win)
{
	MS_BOOL bEnable = FALSE;
	MS_U16 u16RegVal = 0;
	switch(u8GOP) {
	case E_GOP0:
		u16RegVal = HAL_GOP_Read2bytemask(REG_0004_GOP0_BKA4DE, REG_0004_GOP0_BKA4DE_REG_GOP_EN);
		break;
	case E_GOP1:
		u16RegVal = HAL_GOP_Read2bytemask(REG_0004_GOP1_BKA4E4, REG_0004_GOP1_BKA4E4_REG_GOP_EN);
		break;
	case E_GOP2:
		u16RegVal = HAL_GOP_Read2bytemask(REG_0004_GOP2_BKA4E8, REG_0004_GOP2_BKA4E8_REG_GOP_EN);
		break;
	case E_GOP3:
		u16RegVal = HAL_GOP_Read2bytemask(REG_0004_GOP3_BKA4EC, REG_0004_GOP3_BKA4EC_REG_GOP_EN);
		break;
	case E_GOP4:
		u16RegVal = HAL_GOP_Read2bytemask(REG_0004_GOPC_BKA4D3, REG_0004_GOPC_BKA4D3_REG_GOP_EN);
		break;
	default:
		break;
	}

	bEnable = (MS_BOOL)u16RegVal;
	return bEnable;
}

GOP_Result HAL_GOP_GWIN_TriggerRegWriteIn(GOP_CTX_HAL_LOCAL *pstGOPHalLocal, MS_U8 u8GOP, MS_BOOL bEnable, E_DRV_GOPTriggerMode eGOPTriggerMode)
{
	MS_U16 u16RegVal = 0;
	DRV_GOPDstType eGopDst = E_DRV_GOP_DST_IP0;
	MS_U32 ChipMajor = (u32ChipVersion & GOP_CAPS_MAJOR_IPVERSION_OFFSET_MSK) >> GOP_CAPS_MAJOR_IPVERSION_OFFSET_SHT;

	HAL_GOP_GetGOPDst(pstGOPHalLocal, u8GOP, &eGopDst);

	if (eGOPTriggerMode == E_DRV_GOP_FORCEWRITE || eGOPTriggerMode == E_DRV_GOP_BNKFORCEWRITE) {
		if (u8GOP == E_GOP0)
			HAL_GOP_Write2bytemask(REG_0024_GOPG_BKA4D9, bEnable?1:0, REG_0024_GOPG_BKA4D9_REG_GOP0_FORCE_WR);
		else if (u8GOP == E_GOP1)
			HAL_GOP_Write2bytemask(REG_0024_GOPG_BKA4D9, bEnable?1:0, REG_0024_GOPG_BKA4D9_REG_GOP1_FORCE_WR);
		else if (u8GOP == E_GOP2)
			HAL_GOP_Write2bytemask(REG_0024_GOPG_BKA4D9, bEnable?1:0, REG_0024_GOPG_BKA4D9_REG_GOP2_FORCE_WR);
		else if (u8GOP == E_GOP3) {
			if (ChipMajor == GOP_M6_SERIES_MAJOR)
				HAL_GOP_Write2bytemask(REG_0024_GOPG_BKA4D9, bEnable?1:0, REG_0024_GOPG_BKA4D9_REG_GOP3_FORCE_WR);
			else if (ChipMajor == GOP_MOKONA_SERIES_MAJOR)
				HAL_GOP_Write2bytemask(REG_0024_GOPG_BKA4D9, bEnable?1:0, REG_0024_GOPG_BKA4D9_REG_GOPC_FORCE_WR);
		} else if (u8GOP == E_GOP4)
			HAL_GOP_Write2bytemask(REG_0024_GOPG_BKA4D9, bEnable?1:0, REG_0024_GOPG_BKA4D9_REG_GOPC_FORCE_WR);

		if (ChipMajor >= GOP_MOKA_SERIES_MAJOR) {
			if (eGopDst == E_DRV_GOP_DST_FRC)
				HAL_GOP_Write2bytemask(REG_0024_GOPG_BKA4D9, bEnable?1:0, REG_0024_GOPG_BKA4D9_REG_GOPG_VG_FORCE_WR_0024);
			else
				HAL_GOP_Write2bytemask(REG_0024_GOPG_BKA4D9, bEnable?1:0, REG_0024_GOPG_BKA4D9_REG_GOPG_FORCE_WR);
		} else {
			HAL_GOP_Write2bytemask(REG_0024_GOPG_BKA4D9, bEnable?1:0, REG_0024_GOPG_BKA4D9_REG_GOPG_FORCE_WR);
		}
	} else if (eGOPTriggerMode == E_DRV_GOP_WITHVSYNC) {
		if (u8GOP == E_GOP0)
			HAL_GOP_Write2bytemask(REG_0020_GOPG_BKA4D9, bEnable?1:0, REG_0020_GOPG_BKA4D9_REG_GOP0_CFG_TRIG);
		else if (u8GOP == E_GOP1)
			HAL_GOP_Write2bytemask(REG_0020_GOPG_BKA4D9, bEnable?1:0, REG_0020_GOPG_BKA4D9_REG_GOP1_CFG_TRIG);
		else if (u8GOP == E_GOP2)
			HAL_GOP_Write2bytemask(REG_0020_GOPG_BKA4D9, bEnable?1:0, REG_0020_GOPG_BKA4D9_REG_GOP2_CFG_TRIG);
		else if (u8GOP == E_GOP3) {
			if (ChipMajor == GOP_M6_SERIES_MAJOR)
				HAL_GOP_Write2bytemask(REG_0020_GOPG_BKA4D9, bEnable?1:0, REG_0020_GOPG_BKA4D9_REG_GOP3_CFG_TRIG);
			else if (ChipMajor == GOP_MOKONA_SERIES_MAJOR)
				HAL_GOP_Write2bytemask(REG_0020_GOPG_BKA4D9, bEnable?1:0, REG_0020_GOPG_BKA4D9_REG_GOPC_CFG_TRIG);
		} else if (u8GOP == E_GOP4)
			HAL_GOP_Write2bytemask(REG_0020_GOPG_BKA4D9, bEnable?1:0, REG_0020_GOPG_BKA4D9_REG_GOPC_CFG_TRIG);

		if (ChipMajor >= GOP_MOKA_SERIES_MAJOR) {
			if (eGopDst == E_DRV_GOP_DST_FRC)
				HAL_GOP_Write2bytemask(REG_0020_GOPG_BKA4D9, bEnable?1:0, REG_0020_GOPG_BKA4D9_REG_GOPG_VG_CFG_TRIG_0020);
			else
				HAL_GOP_Write2bytemask(REG_0020_GOPG_BKA4D9, bEnable?1:0, REG_0020_GOPG_BKA4D9_REG_GOPG_CFG_TRIG);
		} else {
			HAL_GOP_Write2bytemask(REG_0020_GOPG_BKA4D9, bEnable?1:0, REG_0020_GOPG_BKA4D9_REG_GOPG_CFG_TRIG);
		}
	} else if (eGOPTriggerMode == E_DRV_GOP_WITHSYNC_MULTIGOPUPDATED) {
		u16RegVal = HAL_GOP_Read2byte(REG_0020_GOPG_BKA4D9);
		u16RegVal |= (u8GOP | REG_GOPG_DB_TRIGGER);
		HAL_GOP_Write2byte(REG_0020_GOPG_BKA4D9, u16RegVal);
	}

	return GOP_SUCCESS;
}

GOP_Result HAL_GOP_SetFirstHs(GOP_CTX_HAL_LOCAL *pstGOPHalLocal, MS_U8 u8GOP, MS_BOOL bEnable)
{
	return GOP_SUCCESS;
}

GOP_Result HAL_GOP_SetHSCALE(GOP_CTX_HAL_LOCAL *pstGOPHalLocal, MS_U8 u8GOP, MS_U16 u16HInitPhase, MS_U64 u64Hratio)
{
	return GOP_SUCCESS;
}

GOP_Result HAL_GOP_SetVSCALE(GOP_CTX_HAL_LOCAL *pstGOPHalLocal, MS_U8 u8GOP, MS_U16 u16VInitPhase, MS_U64 u64Vratio)
{
	return GOP_SUCCESS;
}

GOP_Result HAL_GOP_SetStep1HVSPHscale(GOP_CTX_HAL_LOCAL *pstGOPHalLocal, MS_U8 u8GOP, MS_BOOL bEnable, MS_U32 u32Hratio, MS_U16 u16Hout)
{
	MS_U32 u32Bankoffset = 0;

	_GetBnkOfstByGop(u8GOP, &u32Bankoffset);

	HAL_GOP_Write2bytemask(u32Bankoffset + REG_0058_GOP0_BKA4DE, u32Hratio & REG_GOP_HVSP1_RATIO_LSB_MSK, REG_0058_GOP0_BKA4DE_REG_HSP_RATIO_0);
	HAL_GOP_Write2bytemask(u32Bankoffset + REG_005C_GOP0_BKA4DE, (u32Hratio & REG_GOP_HVSP1_HRATIO_MSB_MSK) >> 16, REG_005C_GOP0_BKA4DE_REG_HSP_RATIO_1);
	HAL_GOP_Write2bytemask(u32Bankoffset + REG_005C_GOP0_BKA4DE, bEnable?1:0, REG_005C_GOP0_BKA4DE_REG_HSP_EN);
	HAL_GOP_Write2bytemask(u32Bankoffset + REG_0068_GOP0_BKA4DE, u16Hout, REG_0068_GOP0_BKA4DE_REG_HSP_OUT_HSIZE);
	return GOP_SUCCESS;
}

GOP_Result HAL_GOP_SetStep1HVSPVscale(GOP_CTX_HAL_LOCAL *pstGOPHalLocal, MS_U8 u8GOP, MS_BOOL bEnable, MS_U32 u32Vratio, MS_U16 u16Vout)
{
	MS_U32 u32Bankoffset = 0;

	_GetBnkOfstByGop(u8GOP, &u32Bankoffset);

	HAL_GOP_Write2bytemask(u32Bankoffset + REG_0060_GOP0_BKA4DE, u32Vratio & REG_GOP_HVSP1_RATIO_LSB_MSK, REG_0060_GOP0_BKA4DE_REG_VSP_RATIO_0);
	HAL_GOP_Write2bytemask(u32Bankoffset + REG_0064_GOP0_BKA4DE, (u32Vratio & REG_GOP_HVSP1_VRATIO_MSB_MSK) >> 16, REG_0064_GOP0_BKA4DE_REG_VSP_RATIO_1);
	HAL_GOP_Write2bytemask(u32Bankoffset + REG_0064_GOP0_BKA4DE, bEnable?1:0, REG_0064_GOP0_BKA4DE_REG_VSP_EN);
	HAL_GOP_Write2bytemask(u32Bankoffset + REG_006C_GOP0_BKA4DE, u16Vout, REG_006C_GOP0_BKA4DE_REG_VSP_OUT_VSIZE);

	return GOP_SUCCESS;
}

GOP_Result HAL_GOP_SetStep2HVSPHscale(GOP_CTX_HAL_LOCAL *pstGOPHalLocal, MS_BOOL bEnable, MS_U32 u32Hratio)
{
	HAL_GOP_Write2bytemask(REG_00E0_GOPG_BKA4D9, u32Hratio & REG_GOP_HVSP2_RATIO_LSB_MSK, REG_00E0_GOPG_BKA4D9_REG_GG_HSP_RATIO_0);
	HAL_GOP_Write2bytemask(REG_00E4_GOPG_BKA4D9, (u32Hratio & REG_GOP_HVSP2_HRATIO_MSB_MSK) >> 16, REG_00E4_GOPG_BKA4D9_REG_GG_HSP_RATIO_1);
	HAL_GOP_Write2bytemask(REG_00E4_GOPG_BKA4D9, bEnable?1:0, REG_00E4_GOPG_BKA4D9_REG_GG_HSP_EN);

	return GOP_SUCCESS;
}

GOP_Result HAL_GOP_SetStep2HVSPVscale(GOP_CTX_HAL_LOCAL *pstGOPHalLocal, MS_BOOL bEnable, MS_U32 u32Vratio)
{
	HAL_GOP_Write2bytemask(REG_00E8_GOPG_BKA4D9, u32Vratio & REG_GOP_HVSP2_RATIO_LSB_MSK, REG_00E8_GOPG_BKA4D9_REG_GG_VSP_RATIO_0);
	HAL_GOP_Write2bytemask(REG_00EC_GOPG_BKA4D9, (u32Vratio & REG_GOP_HVSP2_VRATIO_MSB_MSK) >> 16, REG_00EC_GOPG_BKA4D9_REG_GG_VSP_RATIO_1);
	HAL_GOP_Write2bytemask(REG_00EC_GOPG_BKA4D9, bEnable?1:0, REG_00EC_GOPG_BKA4D9_REG_GG_VSP_EN);

	return GOP_SUCCESS;
}

GOP_Result HAL_GOP_SetMixer2OutSize(GOP_CTX_HAL_LOCAL *pstGOPHalLocal, MS_U16 u16Width, MS_U16 u16Height)
{
	HAL_GOP_Write2bytemask(REG_0060_GOPG_BKA4D9, u16Width, REG_0060_GOPG_BKA4D9_REG_MIXER2_DISP_HSIZE);
	HAL_GOP_Write2bytemask(REG_0064_GOPG_BKA4D9, u16Height, REG_0064_GOPG_BKA4D9_REG_MIXER2_DISP_VSIZE);

	return GOP_SUCCESS;
}

GOP_Result HAL_GOP_SetMixer4OutSize(GOP_CTX_HAL_LOCAL *pstGOPHalLocal, MS_U16 u16Width, MS_U16 u16Height)
{
	MS_U32 ChipMajor = (u32ChipVersion & GOP_CAPS_MAJOR_IPVERSION_OFFSET_MSK) >> GOP_CAPS_MAJOR_IPVERSION_OFFSET_SHT;

	HAL_GOP_Write2bytemask(REG_0040_GOPG_BKA4D9, u16Width, REG_0040_GOPG_BKA4D9_REG_MIXER4_DISP_HSIZE);
	HAL_GOP_Write2bytemask(REG_0044_GOPG_BKA4D9, u16Height, REG_0044_GOPG_BKA4D9_REG_MIXER4_DISP_VSIZE);
	if (ChipMajor >= GOP_MOKA_SERIES_MAJOR) {
		HAL_GOP_Write2bytemask(REG_0120_GOPG_BKA4D9, u16Width, REG_0120_GOPG_BKA4D9_REG_VG_MIXER4_DISP_HSIZE_0120);
		HAL_GOP_Write2bytemask(REG_0124_GOPG_BKA4D9, u16Height, REG_0124_GOPG_BKA4D9_REG_VG_MIXER4_DISP_VSIZE_0124);
	}

	return GOP_SUCCESS;
}

GOP_Result HAL_GOP_SetHTotal(GOP_CTX_HAL_LOCAL *pstGOPHalLocal, MS_U8 u8GOP, MS_U16 u16Width)
{
	return GOP_SUCCESS;
}

GOP_Result HAL_GOP_SetInterrupt(GOP_CTX_HAL_LOCAL *pstGOPHalLocal, MS_U8 u8GOP, MS_BOOL bEnable)
{
	MS_U16 u16RegVal = 0;

	u16RegVal = HAL_GOP_Read2bytemask(REG_0004_GOPG_BKA4D9, REG_0004_GOPG_BKA4D9_REG_VG_SEP_PATH);

	if (u16RegVal == 0) {
		HAL_GOP_Write2bytemask(REG_0028_GOPG_BKA4D9, bEnable?0:1, REG_0028_GOPG_BKA4D9_REG_SCTC_INT_MASK);
		HAL_GOP_Write2bytemask(REG_0028_GOPG_BKA4D9, 0x1, REG_0028_GOPG_BKA4D9_REG_OCTG_INT_MASK);
	} else {
		HAL_GOP_Write2bytemask(REG_0028_GOPG_BKA4D9, bEnable?0:1, REG_0028_GOPG_BKA4D9_REG_OCTG_INT_MASK);
		HAL_GOP_Write2bytemask(REG_0028_GOPG_BKA4D9, 0x1, REG_0028_GOPG_BKA4D9_REG_SCTC_INT_MASK);
	}

	return GOP_SUCCESS;
}

GOP_Result HAL_GOP_SetBrightness(GOP_CTX_HAL_LOCAL *pstGOPHalLocal, MS_U8 u8GOP, MS_U16 u16BriVal, MS_BOOL bMSB)
{
	MS_U32 u32Bankoffset = 0;
	MS_U16 u16RegVal = 0;

	_GetBnkOfstByGop(u8GOP, &u32Bankoffset);

	u16RegVal = (bMSB<<10) | (u16BriVal<<2);
	u16RegVal += BRIGHTNESS_OFFSET;
	HAL_GOP_Write2bytemask(u32Bankoffset + REG_00AC_GOP0_BKA4DE, u16RegVal, REG_00AC_GOP0_BKA4DE_REG_B_OFFSET);
	HAL_GOP_Write2bytemask(u32Bankoffset + REG_00B0_GOP0_BKA4DE, u16RegVal, REG_00B0_GOP0_BKA4DE_REG_G_OFFSET);
	HAL_GOP_Write2bytemask(u32Bankoffset + REG_00B4_GOP0_BKA4DE, u16RegVal, REG_00B4_GOP0_BKA4DE_REG_R_OFFSET);

	return GOP_SUCCESS;
}

GOP_Result HAL_GOP_GetBrightness(GOP_CTX_HAL_LOCAL *pstGOPHalLocal, MS_U8 u8GOP, MS_U16 *u16BriVal, MS_BOOL *bMSB)
{
	MS_U32 u32Bankoffset = 0;
	MS_U16 u16RegVal = 0;

	_GetBnkOfstByGop(u8GOP, &u32Bankoffset);

	u16RegVal = HAL_GOP_Read2bytemask(u32Bankoffset + REG_00AC_GOP0_BKA4DE, REG_00AC_GOP0_BKA4DE_REG_B_OFFSET);
	u16RegVal -= BRIGHTNESS_OFFSET;
	*bMSB = (MS_BOOL)((u16RegVal & 0x400)>>10);
	*u16BriVal = (u16RegVal & 0x3FF)>>2;

	return GOP_SUCCESS;
}

GOP_Result HAL_GOP_Set_HStretchMode(GOP_CTX_HAL_LOCAL *pstGOPHalLocal, MS_U8 u8GOP, DRV_GOPStrchHMode HStrchMode)
{
	MS_U32 u32Bankoffset = 0;

	if (u8GOP >= gHalGopChipPro.TotalGOPNum) {
		GOP_H_ERR("[%s][%d] Out of GOP support!!! GOP=%d\n",
			__FUNCTION__, __LINE__, u8GOP);
		return GOP_FAIL;
	}

	_GetBnkOfstByGop(u8GOP, &u32Bankoffset);

	switch (HStrchMode) {
	case E_DRV_GOP_HSTRCH_DUPLICATE:
		HAL_GOP_Write2bytemask(u32Bankoffset + REG_002C_GOP0_HVSP_BKA4E1, 0x0, REG_002C_GOP0_HVSP_BKA4E1_REG_MODE_Y_HO_002C);
		HAL_GOP_Write2bytemask(u32Bankoffset + REG_002C_GOP0_HVSP_BKA4E1, 0x0, REG_002C_GOP0_HVSP_BKA4E1_REG_MODE_C_HO_002C);
		HAL_GOP_Write2bytemask(u32Bankoffset + REG_002C_GOP0_HVSP_BKA4E1, 0x0, REG_002C_GOP0_HVSP_BKA4E1_REG_MODE_A_HO_002C);
		HAL_GOP_Write2bytemask(u32Bankoffset + REG_002C_GOP0_HVSP_BKA4E1, 0x0, REG_002C_GOP0_HVSP_BKA4E1_REG_Y_RAM_EN_HO_002C);
		break;
	case E_DRV_GOP_HSTRCH_6TAPE:
	case E_DRV_GOP_HSTRCH_6TAPE_LINEAR:
		HAL_GOP_Write2bytemask(u32Bankoffset + REG_002C_GOP0_HVSP_BKA4E1, 0x1, REG_002C_GOP0_HVSP_BKA4E1_REG_MODE_Y_HO_002C);
		HAL_GOP_Write2bytemask(u32Bankoffset + REG_002C_GOP0_HVSP_BKA4E1, 0x1, REG_002C_GOP0_HVSP_BKA4E1_REG_MODE_C_HO_002C);
		HAL_GOP_Write2bytemask(u32Bankoffset + REG_002C_GOP0_HVSP_BKA4E1, 0x1, REG_002C_GOP0_HVSP_BKA4E1_REG_MODE_A_HO_002C);
		HAL_GOP_Write2bytemask(u32Bankoffset + REG_002C_GOP0_HVSP_BKA4E1, 0x0, REG_002C_GOP0_HVSP_BKA4E1_REG_Y_RAM_EN_HO_002C);
		break;
	case E_DRV_GOP_HSTRCH_4TAPE:
	case E_DRV_GOP_HSTRCH_NEW4TAP_45:
	case E_DRV_GOP_HSTRCH_NEW4TAP_50:
	case E_DRV_GOP_HSTRCH_NEW4TAP_55:
	case E_DRV_GOP_HSTRCH_NEW4TAP_65:
	case E_DRV_GOP_HSTRCH_NEW4TAP_75:
	case E_DRV_GOP_HSTRCH_NEW4TAP_85:
	case E_DRV_GOP_HSTRCH_NEW4TAP_95:
	case E_DRV_GOP_HSTRCH_NEW4TAP_100:
	case E_DRV_GOP_HSTRCH_NEW4TAP_105:
	case E_DRV_GOP_HSTRCH_NEW4TAP_105_GAIN16:
	case E_DRV_GOP_HSTRCH_NEW4TAP_105_GAIN17:
	case E_DRV_GOP_HSTRCH_NEW4TAP_105_GAIN22:
	case E_DRV_GOP_HSTRCH_NEW4TAP_105_GAIN31:
		HAL_GOP_Write2bytemask(u32Bankoffset + REG_002C_GOP0_HVSP_BKA4E1, 0x0, REG_002C_GOP0_HVSP_BKA4E1_REG_MODE_Y_HO_002C);
		HAL_GOP_Write2bytemask(u32Bankoffset + REG_002C_GOP0_HVSP_BKA4E1, GOP_STRETCH_4TAP_C_SRAM_TABLE_EN, REG_002C_GOP0_HVSP_BKA4E1_REG_MODE_C_HO_002C);
		HAL_GOP_Write2bytemask(u32Bankoffset + REG_002C_GOP0_HVSP_BKA4E1, 0x1, REG_002C_GOP0_HVSP_BKA4E1_REG_MODE_A_HO_002C);
		HAL_GOP_Write2bytemask(u32Bankoffset + REG_002C_GOP0_HVSP_BKA4E1, 0x1, REG_002C_GOP0_HVSP_BKA4E1_REG_Y_RAM_EN_HO_002C);
		break;
	default:
		GOP_H_ERR("[%s][%d] GOP:%d, not support H stretch:%d\n", __FUNCTION__, __LINE__, u8GOP, HStrchMode);
		return GOP_FUN_NOT_SUPPORTED;
	}

	return GOP_SUCCESS;
}

GOP_Result HAL_GOP_Set_VStretchMode(GOP_CTX_HAL_LOCAL *pstGOPHalLocal, MS_U8 u8GOP, DRV_GOPStrchVMode VStrchMode)
{
	MS_U32 u32Bankoffset = 0;

	if (u8GOP >= gHalGopChipPro.TotalGOPNum) {
		GOP_H_ERR("[%s][%d] Out of GOP support!!! GOP=%d\n",
			__FUNCTION__, __LINE__, u8GOP);
		return GOP_FAIL;
	}

	_GetBnkOfstByGop(u8GOP, &u32Bankoffset);

	switch (VStrchMode) {
	case E_DRV_GOP_VSTRCH_DUPLICATE:
		HAL_GOP_Write2bytemask(u32Bankoffset + REG_002C_GOP0_HVSP_BKA4E1, 0x0, REG_002C_GOP0_HVSP_BKA4E1_REG_MODE_Y_VE_002C);
		HAL_GOP_Write2bytemask(u32Bankoffset + REG_002C_GOP0_HVSP_BKA4E1, 0x0, REG_002C_GOP0_HVSP_BKA4E1_REG_MODE_C_VE_002C);
		HAL_GOP_Write2bytemask(u32Bankoffset + REG_002C_GOP0_HVSP_BKA4E1, 0x0, REG_002C_GOP0_HVSP_BKA4E1_REG_MODE_A_VE_002C);
		HAL_GOP_Write2bytemask(u32Bankoffset + REG_002C_GOP0_HVSP_BKA4E1, 0x0, REG_002C_GOP0_HVSP_BKA4E1_REG_Y_RAM_EN_VE_002C);
	break;
	case E_DRV_GOP_VSTRCH_LINEAR_GAIN2:
		HAL_GOP_Write2bytemask(u32Bankoffset + REG_002C_GOP0_HVSP_BKA4E1, 0x1, REG_002C_GOP0_HVSP_BKA4E1_REG_MODE_Y_VE_002C);
		HAL_GOP_Write2bytemask(u32Bankoffset + REG_002C_GOP0_HVSP_BKA4E1, 0x1, REG_002C_GOP0_HVSP_BKA4E1_REG_MODE_C_VE_002C);
		HAL_GOP_Write2bytemask(u32Bankoffset + REG_002C_GOP0_HVSP_BKA4E1, 0x1, REG_002C_GOP0_HVSP_BKA4E1_REG_MODE_A_VE_002C);
		HAL_GOP_Write2bytemask(u32Bankoffset + REG_002C_GOP0_HVSP_BKA4E1, 0x0, REG_002C_GOP0_HVSP_BKA4E1_REG_Y_RAM_EN_VE_002C);
	break;
	default:
		GOP_H_ERR("[%s][%d] GOP:%d, not support V stretch:%d\n", __FUNCTION__, __LINE__, u8GOP, VStrchMode);
		return GOP_FUN_NOT_SUPPORTED;
	}

	return GOP_SUCCESS;
}

GOP_Result HAL_GOP_AFBC_Core_Enable(GOP_CTX_HAL_LOCAL *pstGOPHalLocal, MS_U8 u8GOP, MS_BOOL bEnable)
{
	MS_U32 u32Bankoffset = 0;

	_GetBnkOfstByGop(u8GOP, &u32Bankoffset);
	HAL_GOP_Write2bytemask(u32Bankoffset + REG_0004_GOP0_BKA4DE, bEnable?0x1:0x0, REG_0004_GOP0_BKA4DE_REG_GOP_AFBC_EN);

	return GOP_SUCCESS;
}

GOP_Result MHal_GOP_AFBCMode(GOP_CTX_HAL_LOCAL *pGOPHalLocal, MS_U8 u8GOP, MS_BOOL bEnable)
{
	MS_U32 u32BankOffSet=0;

	_GetBnkOfstByGop(u8GOP, &u32BankOffSet);
	HAL_GOP_Write2bytemask(u32BankOffSet + REG_0004_GOP0_BKA4DE, bEnable?0x1:0x0, REG_0004_GOP0_BKA4DE_REG_GOP_AFBC_EN);

	return GOP_SUCCESS;
}

GOP_Result MHal_GOP_AFBCSetWindow(GOP_CTX_HAL_LOCAL*pGOPHalLocal, MS_U8 u8GOP, DRV_GOP_AFBC_Info* pinfo)
{
	MS_U8 u8Halignfactor = 0, u8Valignfactor = 0;
	MS_U16 u16Width = 0, u16Height = 0, u16tmp = 0;
	MS_U32 u32BankOffSet = 0;
	MS_PHY phytmp = 0;

	//32*8 block
	u8Halignfactor = 64;
	u8Valignfactor = 8;
	u16Width = pinfo->u16HPixelEnd-pinfo->u16HPixelStart;
	u16Width = (u16Width + u8Halignfactor - 1) & (~(u8Halignfactor - 1));
	u16Height= pinfo->u16VPixelEnd-pinfo->u16VPixelStart;
	u16Height = (u16Height + u8Valignfactor - 1) & (~(u8Valignfactor - 1));

	if(u16Width == 0 || u16Height == 0) {
		GOP_H_ERR("[%s][%d]  Fail!!! Width=%d, Height=%d,Pitch=%d  \n",__FUNCTION__,__LINE__,\
			pinfo->u16HPixelEnd-pinfo->u16HPixelStart, pinfo->u16VPixelEnd-pinfo->u16VPixelStart, pinfo->u16Pitch);
		return GOP_FAIL;
	}

	phytmp = (pinfo->u64DRAMAddr/GOP_STRADDR_WORD_UNIT);
	u16tmp = (MS_U16)(phytmp & 0xFFFF);
	HAL_GOP_Write2bytemask(u32BankOffSet + REG_0008_GOP0_BKA4DE, phytmp, REG_0008_GOP0_BKA4DE_REG_DRAM_BASE_STR_0);
	u16tmp = (MS_U16)((phytmp & 0x1FFF0000) >> 16);
	HAL_GOP_Write2bytemask(u32BankOffSet + REG_000C_GOP0_BKA4DE, u16tmp, REG_000C_GOP0_BKA4DE_REG_DRAM_BASE_STR_1);

	HAL_GOP_Write2bytemask(u32BankOffSet + REG_0010_GOP0_BKA4DE, u16Width, REG_0010_GOP0_BKA4DE_REG_PIC_HSIZE);
	HAL_GOP_Write2bytemask(u32BankOffSet + REG_0014_GOP0_BKA4DE, u16Height, REG_0014_GOP0_BKA4DE_REG_PIC_VSIZE);


	switch (pinfo->u8Fmt) {
	case  E_DRV_GOP_AFBC_RGB565:
		HAL_GOP_Write2bytemask(u32BankOffSet + REG_0004_GOP0_BKA4DE, 0x1, REG_0004_GOP0_BKA4DE_REG_DATA_FMT);
	break;
	case E_DRV_GOP_AFBC_RGB888:
		HAL_GOP_Write2bytemask(u32BankOffSet + REG_0004_GOP0_BKA4DE, 0x14, REG_0004_GOP0_BKA4DE_REG_DATA_FMT);
	break;
	case E_DRV_GOP_AFBC_ARGB8888:
		HAL_GOP_Write2bytemask(u32BankOffSet + REG_0004_GOP0_BKA4DE, 0x5, REG_0004_GOP0_BKA4DE_REG_DATA_FMT);
	break;
	default:
	break;
	}

	return GOP_SUCCESS;
}

GOP_Result HAL_GOP_ByPassMode(GOP_CTX_HAL_LOCAL*pGOPHalLocal, MS_U32 u32GOPIdx, MS_BOOL bEnable)
{
	if(u32GOPIdx == E_GOP0) {
		HAL_GOP_Write2bytemask(REG_0004_GOPG_BKA4D9, bEnable? 0x1:0x0, REG_0004_GOPG_BKA4D9_REG_GOP0_DEST_SEL);
		HAL_GOP_Write2bytemask(REG_0048_GOPG_BKA4D9, bEnable? 0x1:0x0, REG_0048_GOPG_BKA4D9_REG_MIXER4_BYP_EN);
		HAL_GOP_Write2bytemask(REG_0048_GOPG_BKA4D9, bEnable? 0x0:0x1, REG_0048_GOPG_BKA4D9_REG_MIXER4_RMA_EN);
		HAL_GOP_Write2bytemask(REG_0068_GOPG_BKA4D9, bEnable? 0x1:0x0, REG_0068_GOPG_BKA4D9_REG_MIXER2_BYP_EN);
		HAL_GOP_Write2bytemask(REG_0068_GOPG_BKA4D9, bEnable? 0x0:0x1, REG_0068_GOPG_BKA4D9_REG_MIXER2_RMA_EN);
		HAL_GOP_Write2bytemask(REG_0004_GOPG_BKA4D9, bEnable? 0x1:0x0, REG_0004_GOPG_BKA4D9_REG_ABF_BYPASS);
		if (bEnable) {
			HAL_GOP_Write2bytemask(REG_005C_GOP0_BKA4DE, 0x0, REG_005C_GOP0_BKA4DE_REG_HSP_EN);
			HAL_GOP_Write2bytemask(REG_0064_GOP0_BKA4DE, 0x0, REG_0064_GOP0_BKA4DE_REG_VSP_EN);
		}
	} else {
		GOP_H_ERR("[%s][%d]Only GOP0 support 8K Bypass path\n", __FUNCTION__, __LINE__);
		return GOP_FAIL;
	}

	if(_HAL_GOP_SetTgen(bEnable) != TRUE) {
		GOP_H_ERR("[%s][%d]Parsing DTB fail\n", __FUNCTION__, __LINE__);
		return GOP_FAIL;
	}

	if(HAL_GOP_MODhbkproch_protect(bEnable) != TRUE) {
		GOP_H_ERR("[%s][%d] hbkporch protect fail\n", __FUNCTION__, __LINE__);
		return GOP_FAIL;
	}

    return GOP_SUCCESS;
}

GOP_Result HAL_GOP_SetIPVersion(MS_U32 u32ChipVer)
{
	MS_U32 ChipMajor = (u32ChipVer & GOP_CAPS_MAJOR_IPVERSION_OFFSET_MSK) >> GOP_CAPS_MAJOR_IPVERSION_OFFSET_SHT;

	u32ChipVersion = u32ChipVer;

	if (ChipMajor == GOP_M6_SERIES_MAJOR)
		GOPCursor = E_GOP4;
	else if (ChipMajor == GOP_MOKONA_SERIES_MAJOR)
		GOPCursor = E_GOP3;

	return GOP_SUCCESS;
}

GOP_Result HAL_GOP_MODhbkproch_protect(bool bIsByPassMode)
{
	#define OFFSET_32BIT 32
	#define NODE_NAME_SIZE 32
	#define VX1_16LANE 16
	#define VX1_8LANE 8
	#define VX1_4LANE 4
	#define VX1_2LANE 2
	#define MINUS_LANE_2 2
	#define MINUS_LANE_4 4
	#define META_DATA_SHIFT_LINE 5
	char node_name[NODE_NAME_SIZE] = {0};
	MS_U32 u32HdeStart = 0, u32HSyncWidth = 0, u32HdeSize = 0, u32Htt = 0;
	MS_U32 u32VdeSize = 0, u32Vtt = 0, u32clk_l = 0, u32clk_h = 0,u16vfreq = 0, lanes = 0;
	MS_U64 u64clk = 0;
	MS_U32 length_needed = 0;

	MS_U8 u8H_minus_line = 0;
	MS_U8 u8Meta_data_mode_shift_line = 0;
	MS_U16 u16Hstart_div_lane = 0;
	MS_U16 u16Hread_de_st_set = 0;

	#define IS_OUT_8K4K(width, height) (((width <= 8000) && (width >= 7500)) && \
									((height <= 4500) && (height >= 4000)))
	#define IS_OUT_4K2K(width, height) (((width <= 4000) && (width >= 3750)) && \
									((height <= 2250) && (height >= 2000)))
	#define IS_OUT_2K1K(width, height) (((width <= 2000) && (width >= 1875)) && \
									((height <= 1125) && (height >= 1000)))
	#define IS_VFREQ_60HZ_GROUP(vfreq) ((vfreq < 65) && (vfreq > 45))
	#define IS_VFREQ_120HZ_GROUP(vfreq) ((vfreq < 125) && (vfreq > 90))
	#define IS_VFREQ_144HZ_GROUP(vfreq) ((vfreq < 149) && (vfreq > 139))


	if(bIsByPassMode == TRUE) {
		length_needed = snprintf(node_name, sizeof(node_name), "%s/%s", "/graphic_out", "loadfw-timing-info");
		if (length_needed < 0 || (unsigned) length_needed >= sizeof(node_name))
			GOP_H_ERR("Error: buffer too small\n");
	} else {
		length_needed = snprintf(node_name, sizeof(node_name), "%s/%s", "/graphic_out", "panel_info");
		if (length_needed < 0 || (unsigned) length_needed >= sizeof(node_name))
			GOP_H_ERR("Error: buffer too small\n");
	}


	if (parse_dt(node_name, integer_dt_parser, (void*)&u32HSyncWidth, "hsync_width") != 0) {
		GOP_H_ERR("[%s][%d]parsing hsync_width error\n", __FUNCTION__, __LINE__);
		return FALSE;
	}

	if (parse_dt(node_name, integer_dt_parser, (void*)&u32HdeStart, "de_hstart") != 0) {
		GOP_H_ERR("[%s][%d]parsing de_hstart error\n", __FUNCTION__, __LINE__);
		return FALSE;
	}

	if (parse_dt(node_name, integer_dt_parser, (void*)&u32HdeSize, "resolution_width") != 0) {
		GOP_H_ERR("[%s][%d]parsing resolution_width error\n", __FUNCTION__, __LINE__);
		return FALSE;
	}

	if (parse_dt(node_name, integer_dt_parser, (void*)&u32Htt, "typ_h_total") != 0) {
		GOP_H_ERR("[%s][%d]parsing typ_h_total error\n", __FUNCTION__, __LINE__);
		return FALSE;
	}



	if (parse_dt(node_name, integer_dt_parser, (void*)&u32VdeSize, "resolution_height") != 0) {
		GOP_H_ERR("[%s][%d]parsing resolution_height error\n", __FUNCTION__, __LINE__);
		return FALSE;
	}

	if (parse_dt(node_name, integer_dt_parser, (void*)&u32Vtt, "typ_v_total") != 0) {
		GOP_H_ERR("[%s][%d]parsing typ_v_total error\n", __FUNCTION__, __LINE__);
		return FALSE;
	}

	if (parse_dt(node_name, integer_dt_parser, (void*)&u32clk_l, "typ_clk_low") != 0) {
		GOP_H_ERR("[%s][%d]parsing typ_framerate error\n", __FUNCTION__, __LINE__);
		return FALSE;
	}

	if (parse_dt(node_name, integer_dt_parser, (void*)&u32clk_h, "typ_clk_high") != 0) {
		GOP_H_ERR("[%s][%d]parsing typ_framerate error\n", __FUNCTION__, __LINE__);
		return FALSE;
	}

	u64clk = ((MS_U64)u32clk_l | (((MS_U64)u32clk_h) << OFFSET_32BIT));

	// calculate lane
	if (u32Htt != 0 && u32Vtt != 0 ) {
		u16vfreq = u64clk / u32Htt / u32Vtt;

	    GOP_H_DBUG("[%s][%d]u32clk_h:%u\n", __FUNCTION__, __LINE__,u32clk_h);
		GOP_H_DBUG("[%s][%d]u32clk_l:%u\n", __FUNCTION__, __LINE__,u32clk_l);
		GOP_H_DBUG("[%s][%d]u16vfreq:%u\n", __FUNCTION__, __LINE__,u16vfreq);

		if (IS_OUT_4K2K(u32HdeSize, u32VdeSize)) {
			if (IS_VFREQ_60HZ_GROUP(u16vfreq)) {
				lanes = VX1_8LANE;
			}
			if (IS_VFREQ_120HZ_GROUP(u16vfreq)) {
				lanes = VX1_16LANE;
			}
			if (IS_VFREQ_144HZ_GROUP(u16vfreq)) {
				lanes = VX1_16LANE;
			}
		}

		if (IS_OUT_2K1K(u32HdeSize, u32VdeSize)) {
			if (IS_VFREQ_60HZ_GROUP(u16vfreq)) {
				lanes = VX1_2LANE;
			}
			if (IS_VFREQ_120HZ_GROUP(u16vfreq)) {
				lanes = VX1_4LANE;
			}
		}
	} else {
		GOP_H_ERR("Output HTT/VTT Error, SHOULD NOT BE ZERO\n");
		return FALSE;
	}

	if(lanes == 0) {
		GOP_H_ERR("lanes = 0, SHOULD NOT BE ZERO\n");
		return FALSE;
        }

	// set hbackproch setting

	HAL_GOP_Write2bytemask(REG_01E4_MODOSD1, 0x1, REG_01E4_MODOSD1_REG_HS_WID_ADJ_EN);

	if (lanes >= VX1_8LANE)
		HAL_GOP_Write2bytemask(REG_000C_MODOSD1, (lanes/VX1_8LANE)-1, REG_000C_MODOSD1_REG_MFT_VLD_WRAP_FIX_HTT);

	if (lanes == VX1_8LANE)
		u8H_minus_line = MINUS_LANE_4; //4
	else if (lanes == VX1_16LANE)
		u8H_minus_line = MINUS_LANE_2; //2
	else
		u8H_minus_line = 0;

	if (HAL_GOP_Read2bytemask(REG_0020_MODOSD2,REG_0020_MODOSD2_REG_META_DATA_EN) == 0)
		u8Meta_data_mode_shift_line = 0;
	else
		u8Meta_data_mode_shift_line = META_DATA_SHIFT_LINE; //5

	u16Hstart_div_lane = (u32HdeStart/lanes);

	if ((u16Hstart_div_lane + u8Meta_data_mode_shift_line) >= (u8H_minus_line))
		u16Hread_de_st_set = u16Hstart_div_lane - u8H_minus_line + u8Meta_data_mode_shift_line;
	else
		u16Hread_de_st_set = 0;

	HAL_GOP_Write2bytemask(REG_003C_MODOSD1, u16Hread_de_st_set, REG_003C_MODOSD1_REG_MFT_READ_HDE_ST);

	if ((u32HSyncWidth / lanes) == 0)
		HAL_GOP_Write2bytemask(REG_0014_MODOSD1, 0, REG_0014_MODOSD1_REG_HS_FP_DELAY_VALUE);
	else
		HAL_GOP_Write2bytemask(REG_0014_MODOSD1, (u32HSyncWidth/lanes) - 1, REG_0014_MODOSD1_REG_HS_FP_DELAY_VALUE);

	return GOP_SUCCESS;
}


