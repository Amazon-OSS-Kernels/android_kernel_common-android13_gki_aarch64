/* SPDX-License-Identifier: (GPL-2.0 OR BSD-3-Clause) */  
/*
* Copyright (c) 2020 MediaTek Inc.
*/ 
    
#ifndef _MTK_TV_LPLL_TBL_V006_H_
#define _MTK_TV_LPLL_TBL_V006_H_
    
//#include <drm/mtk_tv_drm.h>
#include "coda/lpll.h"
typedef struct  {
	uint32_t address;
	uint32_t value;
	uint32_t mask;
} TBLStruct, *pTBLStruct;
   typedef enum  { E_PNL_SUPPORTED_LPLL_LVDS_1ch_HD60_225to300MHz_VER006,	//0
	E_PNL_SUPPORTED_LPLL_LVDS_1ch_HD60_150to225MHz_VER006,	//1
	E_PNL_SUPPORTED_LPLL_LVDS_1ch_HD60_150to150MHz_VER006,	//2
	E_PNL_SUPPORTED_LPLL_LVDS_2ch_2K60_225to300MHz_VER006,	//3
	E_PNL_SUPPORTED_LPLL_LVDS_2ch_2K60_150to225MHz_VER006,	//4
	E_PNL_SUPPORTED_LPLL_LVDS_2ch_2K60_150to150MHz_VER006,	//5
	E_PNL_SUPPORTED_LPLL_HS_LVDS_1ch_2K60_225to300MHz_VER006,	//6
	E_PNL_SUPPORTED_LPLL_HS_LVDS_1ch_2K60_150to225MHz_VER006,	//7
	E_PNL_SUPPORTED_LPLL_HS_LVDS_1ch_2K60_150to150MHz_VER006,	//8
	E_PNL_SUPPORTED_LPLL_HS_LVDS_2ch_4K2K30_225to300MHz_VER006,	//9
	E_PNL_SUPPORTED_LPLL_HS_LVDS_2ch_4K2K30_150to225MHz_VER006,	//10
	E_PNL_SUPPORTED_LPLL_HS_LVDS_2ch_4K2K30_150to150MHz_VER006,	//11
	E_PNL_SUPPORTED_LPLL_MINILVDS_1bk_3pair_6bit_HD60_150to300MHz_VER006,	//12
	E_PNL_SUPPORTED_LPLL_MINILVDS_1bk_3pair_6bit_HD60_150to150MHz_VER006,	//13
	E_PNL_SUPPORTED_LPLL_MINILVDS_1bk_6pair_6bit_HD60_150to300MHz_VER006,	//14
	E_PNL_SUPPORTED_LPLL_MINILVDS_1bk_6pair_6bit_HD60_150to150MHz_VER006,	//15
	E_PNL_SUPPORTED_LPLL_MINILVDS_2bk_3pair_6bit_2K60_150to300MHz_VER006,	//16
	E_PNL_SUPPORTED_LPLL_MINILVDS_2bk_3pair_6bit_2K60_150to150MHz_VER006,	//17
	E_PNL_SUPPORTED_LPLL_MINILVDS_2bk_6pair_6bit_2K60_150to300MHz_VER006,	//18
	E_PNL_SUPPORTED_LPLL_MINILVDS_2bk_6pair_6bit_2K60_150to150MHz_VER006,	//19
	E_PNL_SUPPORTED_LPLL_MINILVDS_1bk_3pair_8bit_HD60_150to300MHz_VER006,	//20
	E_PNL_SUPPORTED_LPLL_MINILVDS_1bk_3pair_8bit_HD60_150to150MHz_VER006,	//21
	E_PNL_SUPPORTED_LPLL_MINILVDS_1bk_6pair_8bit_HD60_150to300MHz_VER006,	//22
	E_PNL_SUPPORTED_LPLL_MINILVDS_1bk_6pair_8bit_HD60_150to150MHz_VER006,	//23
	E_PNL_SUPPORTED_LPLL_MINILVDS_2bk_3pair_8bit_2K60_150to300MHz_VER006,	//24
	E_PNL_SUPPORTED_LPLL_MINILVDS_2bk_3pair_8bit_2K60_150to150MHz_VER006,	//25
	E_PNL_SUPPORTED_LPLL_MINILVDS_2bk_6pair_8bit_2K60_150to300MHz_VER006,	//26
	E_PNL_SUPPORTED_LPLL_MINILVDS_2bk_6pair_8bit_2K60_150to150MHz_VER006,	//27
	E_PNL_SUPPORTED_LPLL_EPI_28_8bit_2pair_2K60_CML_225to300MHz_VER006,	//28
	E_PNL_SUPPORTED_LPLL_EPI_28_8bit_2pair_2K60_CML_150to225MHz_VER006,	//29
	E_PNL_SUPPORTED_LPLL_EPI_28_8bit_2pair_2K60_CML_150to150MHz_VER006,	//30
	E_PNL_SUPPORTED_LPLL_EPI_28_8bit_4pair_2K60_CML_225to300MHz_VER006,	//31
	E_PNL_SUPPORTED_LPLL_EPI_28_8bit_4pair_2K60_CML_150to225MHz_VER006,	//32
	E_PNL_SUPPORTED_LPLL_EPI_28_8bit_4pair_2K60_CML_150to150MHz_VER006,	//33
	E_PNL_SUPPORTED_LPLL_EPI_28_8bit_6pair_2K120_CML_300to600MHz_VER006,	//34
	E_PNL_SUPPORTED_LPLL_EPI_28_8bit_6pair_2K120_CML_300to300MHz_VER006,	//35
	E_PNL_SUPPORTED_LPLL_EPI_28_8bit_8pair_2K120_CML_450to600MHz_VER006,	//36
	E_PNL_SUPPORTED_LPLL_EPI_28_8bit_8pair_2K120_CML_300to450MHz_VER006,	//37
	E_PNL_SUPPORTED_LPLL_EPI_28_8bit_8pair_2K120_CML_300to300MHz_VER006,	//38
	E_PNL_SUPPORTED_LPLL_EPI_28_8bit_4x2_4K2K60_CML_450to600MHz_VER006,	//39
	E_PNL_SUPPORTED_LPLL_EPI_28_8bit_4x2_4K2K60_CML_300to450MHz_VER006,	//40
	E_PNL_SUPPORTED_LPLL_EPI_28_8bit_4x2_4K2K60_CML_300to300MHz_VER006,	//41
	E_PNL_SUPPORTED_LPLL_EPI_28_8bit_6x1_4K2K60_CML_350to600MHz_VER006,	//42
	E_PNL_SUPPORTED_LPLL_EPI_28_8bit_6x1_4K2K60_CML_300to350MHz_VER006,	//43
	E_PNL_SUPPORTED_LPLL_EPI_28_8bit_6x1_4K2K60_CML_300to300MHz_VER006,	//44
	E_PNL_SUPPORTED_LPLL_EPI_28_8bit_8x1_4K2K60_CML_450to600MHz_VER006,	//45
	E_PNL_SUPPORTED_LPLL_EPI_28_8bit_8x1_4K2K60_CML_300to450MHz_VER006,	//46
	E_PNL_SUPPORTED_LPLL_EPI_28_8bit_8x1_4K2K60_CML_300to300MHz_VER006,	//47
	E_PNL_SUPPORTED_LPLL_EPI_28_8bit_12x1_4K2K60_CML_300to600MHz_VER006,	//48
	E_PNL_SUPPORTED_LPLL_EPI_28_8bit_12x1_4K2K60_CML_300to300MHz_VER006,	//49
	E_PNL_SUPPORTED_LPLL_EPI_28_8bit_12x1_4K1K144_CML_180to360MHz_VER006,	//50
	E_PNL_SUPPORTED_LPLL_EPI_28_8bit_12x1_4K1K144_CML_180to180MHz_VER006,	//51
	E_PNL_SUPPORTED_LPLL_EPI_28_8bit_16x1_4K2K60_CML_450to600MHz_VER006,	//52
	E_PNL_SUPPORTED_LPLL_EPI_28_8bit_16x1_4K2K60_CML_300to450MHz_VER006,	//53
	E_PNL_SUPPORTED_LPLL_EPI_28_8bit_16x1_4K2K60_CML_300to300MHz_VER006,	//54
	E_PNL_SUPPORTED_LPLL_EPI_24_10bit_16x1_4K2K60_CML_300to600MHz_VER006,	//55
	E_PNL_SUPPORTED_LPLL_EPI_24_10bit_16x1_4K2K60_CML_300to300MHz_VER006,	//56
	E_PNL_SUPPORTED_LPLL_EPI_24_10bit_4x2_4K2K60_CML_350to600MHz_VER006,	//57
	E_PNL_SUPPORTED_LPLL_EPI_24_10bit_4x2_4K2K60_CML_300to350MHz_VER006,	//58
	E_PNL_SUPPORTED_LPLL_EPI_24_10bit_4x2_4K2K60_CML_300to300MHz_VER006,	//59
	E_PNL_SUPPORTED_LPLL_EPI_24_10bit_12x1_4K2K60_CML_300to600MHz_VER006,	//60
	E_PNL_SUPPORTED_LPLL_EPI_24_10bit_12x1_4K2K60_CML_300to300MHz_VER006,	//61
	E_PNL_SUPPORTED_LPLL_EPI_24_10bit_12x1_4K1K144_CML_280to360MHz_VER006,	//62
	E_PNL_SUPPORTED_LPLL_EPI_24_10bit_12x1_4K1K144_CML_180to280MHz_VER006,	//63
	E_PNL_SUPPORTED_LPLL_EPI_24_10bit_12x1_4K1K144_CML_180to180MHz_VER006,	//64
	E_PNL_SUPPORTED_LPLL_EPI_28_8bit_2pair_2K60_LVDS_225to300MHz_VER006,	//65
	E_PNL_SUPPORTED_LPLL_EPI_28_8bit_2pair_2K60_LVDS_150to225MHz_VER006,	//66
	E_PNL_SUPPORTED_LPLL_EPI_28_8bit_2pair_2K60_LVDS_150to150MHz_VER006,	//67
	E_PNL_SUPPORTED_LPLL_EPI_28_8bit_4pair_2K60_LVDS_225to300MHz_VER006,	//68
	E_PNL_SUPPORTED_LPLL_EPI_28_8bit_4pair_2K60_LVDS_150to225MHz_VER006,	//69
	E_PNL_SUPPORTED_LPLL_EPI_28_8bit_4pair_2K60_LVDS_150to150MHz_VER006,	//70
	E_PNL_SUPPORTED_LPLL_EPI_28_8bit_6pair_2K120_LVDS_300to600MHz_VER006,	//71
	E_PNL_SUPPORTED_LPLL_EPI_28_8bit_6pair_2K120_LVDS_300to300MHz_VER006,	//72
	E_PNL_SUPPORTED_LPLL_EPI_28_8bit_8pair_2K120_LVDS_450to600MHz_VER006,	//73
	E_PNL_SUPPORTED_LPLL_EPI_28_8bit_8pair_2K120_LVDS_300to450MHz_VER006,	//74
	E_PNL_SUPPORTED_LPLL_EPI_28_8bit_8pair_2K120_LVDS_300to300MHz_VER006,	//75
	E_PNL_SUPPORTED_LPLL_EPI_28_8bit_4x2_4K2K60_LVDS_450to600MHz_VER006,	//76
	E_PNL_SUPPORTED_LPLL_EPI_28_8bit_4x2_4K2K60_LVDS_300to450MHz_VER006,	//77
	E_PNL_SUPPORTED_LPLL_EPI_28_8bit_4x2_4K2K60_LVDS_300to300MHz_VER006,	//78
	E_PNL_SUPPORTED_LPLL_EPI_28_8bit_6x1_4K2K60_LVDS_350to600MHz_VER006,	//79
	E_PNL_SUPPORTED_LPLL_EPI_28_8bit_6x1_4K2K60_LVDS_300to350MHz_VER006,	//80
	E_PNL_SUPPORTED_LPLL_EPI_28_8bit_6x1_4K2K60_LVDS_300to300MHz_VER006,	//81
	E_PNL_SUPPORTED_LPLL_EPI_28_8bit_8x1_4K2K60_LVDS_450to600MHz_VER006,	//82
	E_PNL_SUPPORTED_LPLL_EPI_28_8bit_8x1_4K2K60_LVDS_300to450MHz_VER006,	//83
	E_PNL_SUPPORTED_LPLL_EPI_28_8bit_8x1_4K2K60_LVDS_300to300MHz_VER006,	//84
	E_PNL_SUPPORTED_LPLL_EPI_28_8bit_12x1_4K2K60_LVDS_300to600MHz_VER006,	//85
	E_PNL_SUPPORTED_LPLL_EPI_28_8bit_12x1_4K2K60_LVDS_300to300MHz_VER006,	//86
	E_PNL_SUPPORTED_LPLL_EPI_28_8bit_12x1_4K1K144_LVDS_180to360MHz_VER006,	//87
	E_PNL_SUPPORTED_LPLL_EPI_28_8bit_12x1_4K1K144_LVDS_180to180MHz_VER006,	//88
	E_PNL_SUPPORTED_LPLL_EPI_28_8bit_16x1_4K_450to600MHz_VER006,	//89
	E_PNL_SUPPORTED_LPLL_EPI_28_8bit_16x1_4K_300to450MHz_VER006,	//90
	E_PNL_SUPPORTED_LPLL_EPI_28_8bit_16x1_4K_300to300MHz_VER006,	//91
	E_PNL_SUPPORTED_LPLL_EPI_24_10bit_16x1_4K2K60_LVDS_300to600MHz_VER006,	//92
	E_PNL_SUPPORTED_LPLL_EPI_24_10bit_16x1_4K2K60_LVDS_300to300MHz_VER006,	//93
	E_PNL_SUPPORTED_LPLL_EPI_24_10bit_4x2_4K2K60_LVDS_350to600MHz_VER006,	//94
	E_PNL_SUPPORTED_LPLL_EPI_24_10bit_4x2_4K2K60_LVDS_300to350MHz_VER006,	//95
	E_PNL_SUPPORTED_LPLL_EPI_24_10bit_4x2_4K2K60_LVDS_300to300MHz_VER006,	//96
	E_PNL_SUPPORTED_LPLL_EPI_24_10bit_12x1_4K2K60_LVDS_300to600MHz_VER006,	//97
	E_PNL_SUPPORTED_LPLL_EPI_24_10bit_12x1_4K2K60_LVDS_300to300MHz_VER006,	//98
	E_PNL_SUPPORTED_LPLL_EPI_24_10bit_12x1_4K1K144_LVDS_280to360MHz_VER006,	//99
	E_PNL_SUPPORTED_LPLL_EPI_24_10bit_12x1_4K1K144_LVDS_180to280MHz_VER006,	//100
	E_PNL_SUPPORTED_LPLL_EPI_24_10bit_12x1_4K1K144_LVDS_180to180MHz_VER006,	//101
	E_PNL_SUPPORTED_LPLL_CMPI_27_8bit_4x2_4K2K60_450to600MHz_VER006,	//102
	E_PNL_SUPPORTED_LPLL_CMPI_27_8bit_4x2_4K2K60_300to450MHz_VER006,	//103
	E_PNL_SUPPORTED_LPLL_CMPI_27_8bit_4x2_4K2K60_300to300MHz_VER006,	//104
	E_PNL_SUPPORTED_LPLL_CMPI_27_8bit_6x1_4K2K60_350to600MHz_VER006,	//105
	E_PNL_SUPPORTED_LPLL_CMPI_27_8bit_6x1_4K2K60_300to350MHz_VER006,	//106
	E_PNL_SUPPORTED_LPLL_CMPI_27_8bit_6x1_4K2K60_300to300MHz_VER006,	//107
	E_PNL_SUPPORTED_LPLL_CMPI_27_8bit_8x1_4K2K60_450to600MHz_VER006,	//108
	E_PNL_SUPPORTED_LPLL_CMPI_27_8bit_8x1_4K2K60_300to450MHz_VER006,	//109
	E_PNL_SUPPORTED_LPLL_CMPI_27_8bit_8x1_4K2K60_300to300MHz_VER006,	//110
	E_PNL_SUPPORTED_LPLL_CMPI_27_8bit_12x1_4K2K60_300to600MHz_VER006,	//111
	E_PNL_SUPPORTED_LPLL_CMPI_27_8bit_12x1_4K2K60_300to300MHz_VER006,	//112
	E_PNL_SUPPORTED_LPLL_CMPI_27_8bit_12x1_4K1K144_180to360MHz_VER006,	//113
	E_PNL_SUPPORTED_LPLL_CMPI_27_8bit_12x1_4K1K144_180to180MHz_VER006,	//114
	E_PNL_SUPPORTED_LPLL_CMPI_24_10bit_4x2_4K2K60_350to600MHz_VER006,	//115
	E_PNL_SUPPORTED_LPLL_CMPI_24_10bit_4x2_4K2K60_300to350MHz_VER006,	//116
	E_PNL_SUPPORTED_LPLL_CMPI_24_10bit_4x2_4K2K60_300to300MHz_VER006,	//117
	E_PNL_SUPPORTED_LPLL_CMPI_24_10bit_8x1_4K2K60_350to600MHz_VER006,	//118
	E_PNL_SUPPORTED_LPLL_CMPI_24_10bit_8x1_4K2K60_300to350MHz_VER006,	//119
	E_PNL_SUPPORTED_LPLL_CMPI_24_10bit_8x1_4K2K60_300to300MHz_VER006,	//120
	E_PNL_SUPPORTED_LPLL_CMPI_24_10bit_12x1_4K2K60_300to600MHz_VER006,	//121
	E_PNL_SUPPORTED_LPLL_CMPI_24_10bit_12x1_4K2K60_300to300MHz_VER006,	//122
	E_PNL_SUPPORTED_LPLL_CMPI_24_10bit_12x1_4K1K144_280to360MHz_VER006,	//123
	E_PNL_SUPPORTED_LPLL_CMPI_24_10bit_12x1_4K1K144_180to280MHz_VER006,	//124
	E_PNL_SUPPORTED_LPLL_CMPI_24_10bit_12x1_4K1K144_180to180MHz_VER006,	//125
	E_PNL_SUPPORTED_LPLL_ISP_8bit_4x2_4K2K60_450to600MHz_VER006,	//126
	E_PNL_SUPPORTED_LPLL_ISP_8bit_4x2_4K2K60_300to450MHz_VER006,	//127
	E_PNL_SUPPORTED_LPLL_ISP_8bit_4x2_4K2K60_300to300MHz_VER006,	//128
	E_PNL_SUPPORTED_LPLL_ISP_8bit_6x1_4K2K60_350to600MHz_VER006,	//129
	E_PNL_SUPPORTED_LPLL_ISP_8bit_6x1_4K2K60_300to350MHz_VER006,	//130
	E_PNL_SUPPORTED_LPLL_ISP_8bit_6x1_4K2K60_300to300MHz_VER006,	//131
	E_PNL_SUPPORTED_LPLL_ISP_8bit_6x2_4K2K60_300to600MHz_VER006,	//132
	E_PNL_SUPPORTED_LPLL_ISP_8bit_6x2_4K2K60_300to300MHz_VER006,	//133
	E_PNL_SUPPORTED_LPLL_ISP_8bit_8x1_4K2K60_450to600MHz_VER006,	//134
	E_PNL_SUPPORTED_LPLL_ISP_8bit_8x1_4K2K60_300to450MHz_VER006,	//135
	E_PNL_SUPPORTED_LPLL_ISP_8bit_8x1_4K2K60_300to300MHz_VER006,	//136
	E_PNL_SUPPORTED_LPLL_ISP_8bit_12x1_4K2K60_300to600MHz_VER006,	//137
	E_PNL_SUPPORTED_LPLL_ISP_8bit_12x1_4K2K60_300to300MHz_VER006,	//138
	E_PNL_SUPPORTED_LPLL_ISP_8bit_12x1_4K1K144_180to360MHz_VER006,	//139
	E_PNL_SUPPORTED_LPLL_ISP_8bit_12x1_4K1K144_180to180MHz_VER006,	//140
	E_PNL_SUPPORTED_LPLL_ISP_10bit_4x2_4K2K60_400to600MHz_VER006,	//141
	E_PNL_SUPPORTED_LPLL_ISP_10bit_4x2_4K2K60_300to400MHz_VER006,	//142
	E_PNL_SUPPORTED_LPLL_ISP_10bit_4x2_4K2K60_300to300MHz_VER006,	//143
	E_PNL_SUPPORTED_LPLL_ISP_10bit_6x2_4K2K60_300to600MHz_VER006,	//144
	E_PNL_SUPPORTED_LPLL_ISP_10bit_6x2_4K2K60_300to300MHz_VER006,	//145
	E_PNL_SUPPORTED_LPLL_ISP_10bit_8x1_4K2K60_400to600MHz_VER006,	//146
	E_PNL_SUPPORTED_LPLL_ISP_10bit_8x1_4K2K60_300to400MHz_VER006,	//147
	E_PNL_SUPPORTED_LPLL_ISP_10bit_8x1_4K2K60_300to300MHz_VER006,	//148
	E_PNL_SUPPORTED_LPLL_ISP_10bit_12x1_4K2K60_300to600MHz_VER006,	//149
	E_PNL_SUPPORTED_LPLL_ISP_10bit_12x1_4K2K60_300to300MHz_VER006,	//150
	E_PNL_SUPPORTED_LPLL_ISP_10bit_12x1_4K1K144_300to360MHz_VER006,	//151
	E_PNL_SUPPORTED_LPLL_ISP_10bit_12x1_4K1K144_180to300MHz_VER006,	//152
	E_PNL_SUPPORTED_LPLL_ISP_10bit_12x1_4K1K144_180to180MHz_VER006,	//153
	E_PNL_SUPPORTED_LPLL_CHPI_8bit_4x2_4K2K60_450to600MHz_VER006,	//154
	E_PNL_SUPPORTED_LPLL_CHPI_8bit_4x2_4K2K60_300to450MHz_VER006,	//155
	E_PNL_SUPPORTED_LPLL_CHPI_8bit_4x2_4K2K60_300to300MHz_VER006,	//156
	E_PNL_SUPPORTED_LPLL_CHPI_8bit_6x1_4K2K60_300to600MHz_VER006,	//157
	E_PNL_SUPPORTED_LPLL_CHPI_8bit_6x1_4K2K60_300to300MHz_VER006,	//158
	E_PNL_SUPPORTED_LPLL_CHPI_8bit_6x2_4K2K60_300to600MHz_VER006,	//159
	E_PNL_SUPPORTED_LPLL_CHPI_8bit_6x2_4K2K60_300to300MHz_VER006,	//160
	E_PNL_SUPPORTED_LPLL_CHPI_8bit_8x1_4K2K60_450to600MHz_VER006,	//161
	E_PNL_SUPPORTED_LPLL_CHPI_8bit_8x1_4K2K60_300to450MHz_VER006,	//162
	E_PNL_SUPPORTED_LPLL_CHPI_8bit_8x1_4K2K60_300to300MHz_VER006,	//163
	E_PNL_SUPPORTED_LPLL_CHPI_8bit_12x1_4K2K60_300to600MHz_VER006,	//164
	E_PNL_SUPPORTED_LPLL_CHPI_8bit_12x1_4K2K60_300to300MHz_VER006,	//165
	E_PNL_SUPPORTED_LPLL_CHPI_8bit_12x1_4K1K144_180to360MHz_VER006,	//166
	E_PNL_SUPPORTED_LPLL_CHPI_8bit_12x1_4K1K144_180to180MHz_VER006,	//167
	E_PNL_SUPPORTED_LPLL_CHPI_10bit_4x2_4K2K60_350to600MHz_VER006,	//168
	E_PNL_SUPPORTED_LPLL_CHPI_10bit_4x2_4K2K60_300to350MHz_VER006,	//169
	E_PNL_SUPPORTED_LPLL_CHPI_10bit_4x2_4K2K60_300to300MHz_VER006,	//170
	E_PNL_SUPPORTED_LPLL_CHPI_10bit_8x1_4K2K60_350to600MHz_VER006,	//171
	E_PNL_SUPPORTED_LPLL_CHPI_10bit_8x1_4K2K60_300to350MHz_VER006,	//172
	E_PNL_SUPPORTED_LPLL_CHPI_10bit_8x1_4K2K60_300to300MHz_VER006,	//173
	E_PNL_SUPPORTED_LPLL_CHPI_10bit_12x1_4K2K60_500to600MHz_VER006,	//174
	E_PNL_SUPPORTED_LPLL_CHPI_10bit_12x1_4K2K60_300to500MHz_VER006,	//175
	E_PNL_SUPPORTED_LPLL_CHPI_10bit_12x1_4K2K60_300to300MHz_VER006,	//176
	E_PNL_SUPPORTED_LPLL_CHPI_10bit_12x1_4K1K144_280to360MHz_VER006,	//177
	E_PNL_SUPPORTED_LPLL_CHPI_10bit_12x1_4K1K144_180to280MHz_VER006,	//178
	E_PNL_SUPPORTED_LPLL_CHPI_10bit_12x1_4K1K144_180to180MHz_VER006,	//179
	E_PNL_SUPPORTED_LPLL_Vx1_10bit_2_lane_2K60_150to300MHz_VER006,	//180
	E_PNL_SUPPORTED_LPLL_Vx1_10bit_2_lane_2K60_150to150MHz_VER006,	//181
	E_PNL_SUPPORTED_LPLL_Vx1_10bit_4_lane_4K2K30_300to600MHz_VER006,	//182
	E_PNL_SUPPORTED_LPLL_Vx1_10bit_4_lane_4K2K30_300to300MHz_VER006,	//183
	E_PNL_SUPPORTED_LPLL_Vx1_10bit_8_lane_4K2K60_300to600MHz_VER006,	//184
	E_PNL_SUPPORTED_LPLL_Vx1_10bit_8_lane_4K2K60_300to300MHz_VER006,	//185
	E_PNL_SUPPORTED_LPLL_Vx1_10bit_8_lane_4K1K144_180to360MHz_VER006,	//186
	E_PNL_SUPPORTED_LPLL_Vx1_10bit_8_lane_4K1K144_180to180MHz_VER006,	//187
	E_PNL_SUPPORTED_LPLL_Vx1_8bit_2_lane_2K60_225to300MHz_VER006,	//188
	E_PNL_SUPPORTED_LPLL_Vx1_8bit_2_lane_2K60_150to225MHz_VER006,	//189
	E_PNL_SUPPORTED_LPLL_Vx1_8bit_2_lane_2K60_150to150MHz_VER006,	//190
	E_PNL_SUPPORTED_LPLL_Vx1_8bit_4_lane_4K2K30_450to600MHz_VER006,	//191
	E_PNL_SUPPORTED_LPLL_Vx1_8bit_4_lane_4K2K30_300to450MHz_VER006,	//192
	E_PNL_SUPPORTED_LPLL_Vx1_8bit_4_lane_4K2K30_300to300MHz_VER006,	//193
	E_PNL_SUPPORTED_LPLL_Vx1_8bit_8_lane_4K2K60_450to600MHz_VER006,	//194
	E_PNL_SUPPORTED_LPLL_Vx1_8bit_8_lane_4K2K60_300to450MHz_VER006,	//195
	E_PNL_SUPPORTED_LPLL_Vx1_8bit_8_lane_4K2K60_300to300MHz_VER006,	//196
	E_PNL_SUPPORTED_LPLL_Vx1_8bit_8_lane_4K1K144_225to360MHz_VER006,	//197
	E_PNL_SUPPORTED_LPLL_Vx1_8bit_8_lane_4K1K144_180to225MHz_VER006,	//198
	E_PNL_SUPPORTED_LPLL_Vx1_8bit_8_lane_4K1K144_180to180MHz_VER006,	//199
	E_PNL_SUPPORTED_LPLL_MAX_VER006,  
} E_PNL_SUPPORTED_LPLL_TYPE_VER006;
   
//======version006 start======
    
#define LPLL_REG_NUM_VER006    (35)
#define MODA_REG_NUM_VER006    (10)
#define MODD_REG_NUM_VER006    (1)
#define MPLL_REG_NUM_VER006    (6)
#define PNL_NUM_VER006         (200)
    TBLStruct LPLLSettingTBL_VER006[PNL_NUM_VER006][LPLL_REG_NUM_VER006] =  {
	 {			//E_PNL_SUPPORTED_LPLL_LVDS_1ch_HD60_225to300MHz_VER006    NO.0
		//Address,Value,Mask
		{
		0x48600C, 0x1, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x3, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x2, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x2, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x0, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x0, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x1, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x0, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x0, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x0, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x0, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x0, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_LVDS_1ch_HD60_150to225MHz_VER006    NO.1
		//Address,Value,Mask
		{
		0x48600C, 0x1, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x3, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x0, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x2, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x0, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x1, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x1, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x0, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x0, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x0, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x0, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x0, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_LVDS_1ch_HD60_150to150MHz_VER006    NO.2
		//Address,Value,Mask
		{
		0x48600C, 0x1, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x3, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x0, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x2, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x0, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x1, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x1, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x0, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x0, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x0, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x0, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x0, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_LVDS_2ch_2K60_225to300MHz_VER006    NO.3
		//Address,Value,Mask
		{
		0x48600C, 0x1, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x3, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x2, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x2, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x0, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x0, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x1, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x0, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x0, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x0, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x0, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x0, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_LVDS_2ch_2K60_150to225MHz_VER006    NO.4
		//Address,Value,Mask
		{
		0x48600C, 0x1, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x3, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x0, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x2, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x0, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x1, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x1, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x0, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x0, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x0, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x0, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x0, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_LVDS_2ch_2K60_150to150MHz_VER006    NO.5
		//Address,Value,Mask
		{
		0x48600C, 0x1, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x3, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x0, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x2, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x0, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x1, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x1, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x0, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x0, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x0, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x0, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x0, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_HS_LVDS_1ch_2K60_225to300MHz_VER006    NO.6
		//Address,Value,Mask
		{
		0x48600C, 0x1, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x3, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x2, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x1, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x0, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x0, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x1, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x0, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x0, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x0, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x0, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x0, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_HS_LVDS_1ch_2K60_150to225MHz_VER006    NO.7
		//Address,Value,Mask
		{
		0x48600C, 0x1, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x3, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x0, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x1, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x0, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x1, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x1, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x0, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x0, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x0, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x0, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x0, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_HS_LVDS_1ch_2K60_150to150MHz_VER006    NO.8
		//Address,Value,Mask
		{
		0x48600C, 0x1, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x3, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x0, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x1, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x0, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x1, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x1, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x0, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x0, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x0, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x0, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x0, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_HS_LVDS_2ch_4K2K30_225to300MHz_VER006    NO.9
		//Address,Value,Mask
		{
		0x48600C, 0x1, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x7, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x1, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x0, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x0, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x1, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x0, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x0, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x0, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x0, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x0, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_HS_LVDS_2ch_4K2K30_150to225MHz_VER006    NO.10
		//Address,Value,Mask
		{
		0x48600C, 0x2, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x7, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x1, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x0, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x1, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x1, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x0, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x1, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x0, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x0, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x0, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_HS_LVDS_2ch_4K2K30_150to150MHz_VER006    NO.11
		//Address,Value,Mask
		{
		0x48600C, 0x2, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x7, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x1, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x0, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x1, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x1, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x0, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x1, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x0, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x0, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x0, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_MINILVDS_1bk_3pair_6bit_HD60_150to300MHz_VER006    NO.12
		//Address,Value,Mask
		{
		0x48600C, 0x1, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x3, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x0, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x3, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x2, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x1, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x0, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x0, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x1, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x1, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x0, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x0, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x0, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x0, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x0, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_MINILVDS_1bk_3pair_6bit_HD60_150to150MHz_VER006    NO.13
		//Address,Value,Mask
		{
		0x48600C, 0x1, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x3, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x0, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x3, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x2, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x1, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x0, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x0, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x1, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x1, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x0, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x0, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x0, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x0, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x0, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_MINILVDS_1bk_6pair_6bit_HD60_150to300MHz_VER006    NO.14
		//Address,Value,Mask
		{
		0x48600C, 0x1, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x3, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x0, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x3, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x3, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x2, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x0, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x0, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x1, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x1, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x0, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x0, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x0, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x0, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x0, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_MINILVDS_1bk_6pair_6bit_HD60_150to150MHz_VER006    NO.15
		//Address,Value,Mask
		{
		0x48600C, 0x1, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x3, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x0, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x3, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x3, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x2, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x0, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x0, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x1, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x1, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x0, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x0, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x0, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x0, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x0, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_MINILVDS_2bk_3pair_6bit_2K60_150to300MHz_VER006    NO.16
		//Address,Value,Mask
		{
		0x48600C, 0x1, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x3, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x0, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x3, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x2, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x1, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x0, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x0, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x1, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x1, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x0, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x0, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x0, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x0, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x0, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_MINILVDS_2bk_3pair_6bit_2K60_150to150MHz_VER006    NO.17
		//Address,Value,Mask
		{
		0x48600C, 0x1, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x3, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x0, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x3, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x2, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x1, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x0, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x0, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x1, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x1, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x0, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x0, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x0, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x0, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x0, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_MINILVDS_2bk_6pair_6bit_2K60_150to300MHz_VER006    NO.18
		//Address,Value,Mask
		{
		0x48600C, 0x1, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x3, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x0, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x3, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x3, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x2, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x0, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x0, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x1, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x1, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x0, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x0, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x0, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x0, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x0, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_MINILVDS_2bk_6pair_6bit_2K60_150to150MHz_VER006    NO.19
		//Address,Value,Mask
		{
		0x48600C, 0x1, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x3, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x0, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x3, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x3, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x2, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x0, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x0, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x1, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x1, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x0, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x0, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x0, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x0, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x0, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_MINILVDS_1bk_3pair_8bit_HD60_150to300MHz_VER006    NO.20
		//Address,Value,Mask
		{
		0x48600C, 0x1, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x3, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x0, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x2, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x1, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x0, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x0, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x0, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x1, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x1, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x0, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x0, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x0, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x0, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x0, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_MINILVDS_1bk_3pair_8bit_HD60_150to150MHz_VER006    NO.21
		//Address,Value,Mask
		{
		0x48600C, 0x1, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x3, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x0, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x2, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x1, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x0, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x0, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x0, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x1, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x1, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x0, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x0, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x0, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x0, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x0, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_MINILVDS_1bk_6pair_8bit_HD60_150to300MHz_VER006    NO.22
		//Address,Value,Mask
		{
		0x48600C, 0x1, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x3, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x0, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x2, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x2, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x1, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x0, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x0, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x1, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x1, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x0, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x0, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x0, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x0, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x0, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_MINILVDS_1bk_6pair_8bit_HD60_150to150MHz_VER006    NO.23
		//Address,Value,Mask
		{
		0x48600C, 0x1, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x3, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x0, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x2, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x2, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x1, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x0, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x0, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x1, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x1, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x0, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x0, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x0, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x0, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x0, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_MINILVDS_2bk_3pair_8bit_2K60_150to300MHz_VER006    NO.24
		//Address,Value,Mask
		{
		0x48600C, 0x1, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x3, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x0, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x2, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x1, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x0, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x0, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x0, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x1, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x1, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x0, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x0, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x0, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x0, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x0, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_MINILVDS_2bk_3pair_8bit_2K60_150to150MHz_VER006    NO.25
		//Address,Value,Mask
		{
		0x48600C, 0x1, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x3, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x0, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x2, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x1, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x0, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x0, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x0, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x1, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x1, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x0, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x0, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x0, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x0, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x0, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_MINILVDS_2bk_6pair_8bit_2K60_150to300MHz_VER006    NO.26
		//Address,Value,Mask
		{
		0x48600C, 0x1, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x3, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x0, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x2, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x2, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x1, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x0, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x0, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x1, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x1, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x0, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x0, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x0, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x0, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x0, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_MINILVDS_2bk_6pair_8bit_2K60_150to150MHz_VER006    NO.27
		//Address,Value,Mask
		{
		0x48600C, 0x1, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x3, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x0, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x2, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x2, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x1, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x0, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x0, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x1, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x1, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x0, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x0, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x0, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x0, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x0, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_2pair_2K60_CML_225to300MHz_VER006    NO.28
		//Address,Value,Mask
		{
		0x48600C, 0x1, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x7, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x0, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x0, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x0, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x1, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x1, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_2pair_2K60_CML_150to225MHz_VER006    NO.29
		//Address,Value,Mask
		{
		0x48600C, 0x2, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x7, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x0, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x1, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x1, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x1, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x1, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_2pair_2K60_CML_150to150MHz_VER006    NO.30
		//Address,Value,Mask
		{
		0x48600C, 0x2, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x7, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x0, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x1, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x1, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x1, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x1, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_4pair_2K60_CML_225to300MHz_VER006    NO.31
		//Address,Value,Mask
		{
		0x48600C, 0x1, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x7, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x1, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x0, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x0, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x0, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x1, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x1, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_4pair_2K60_CML_150to225MHz_VER006    NO.32
		//Address,Value,Mask
		{
		0x48600C, 0x2, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x7, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x1, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x0, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x1, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x1, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x1, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x1, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_4pair_2K60_CML_150to150MHz_VER006    NO.33
		//Address,Value,Mask
		{
		0x48600C, 0x2, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x7, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x1, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x0, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x1, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x1, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x1, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x1, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_6pair_2K120_CML_300to600MHz_VER006    NO.34
		//Address,Value,Mask
		{
		0x48600C, 0x2, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x7, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x0, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x1, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x1, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x0, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x0, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_6pair_2K120_CML_300to300MHz_VER006    NO.35
		//Address,Value,Mask
		{
		0x48600C, 0x2, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x7, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x0, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x1, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x1, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x0, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x0, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_8pair_2K120_CML_450to600MHz_VER006    NO.36
		//Address,Value,Mask
		{
		0x48600C, 0x1, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x7, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x1, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x0, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x0, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x0, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x0, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x1, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_8pair_2K120_CML_300to450MHz_VER006    NO.37
		//Address,Value,Mask
		{
		0x48600C, 0x2, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x7, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x1, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x0, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x1, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x1, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x0, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x1, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_8pair_2K120_CML_300to300MHz_VER006    NO.38
		//Address,Value,Mask
		{
		0x48600C, 0x2, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x7, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x1, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x0, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x1, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x1, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x0, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x1, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_4x2_4K2K60_CML_450to600MHz_VER006    NO.39
		//Address,Value,Mask
		{
		0x48600C, 0x1, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x7, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x0, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x0, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x0, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x0, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x1, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_4x2_4K2K60_CML_300to450MHz_VER006    NO.40
		//Address,Value,Mask
		{
		0x48600C, 0x2, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x7, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x0, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x1, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x1, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x0, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x1, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_4x2_4K2K60_CML_300to300MHz_VER006    NO.41
		//Address,Value,Mask
		{
		0x48600C, 0x2, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x7, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x0, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x1, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x1, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x0, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x1, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_6x1_4K2K60_CML_350to600MHz_VER006    NO.42
		//Address,Value,Mask
		{
		0x48600C, 0x1, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x2, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x7, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x0, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x0, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x0, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x1, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x0, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x0, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_6x1_4K2K60_CML_300to350MHz_VER006    NO.43
		//Address,Value,Mask
		{
		0x48600C, 0x6, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x2, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x7, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x0, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x1, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x0, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x0, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x0, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_6x1_4K2K60_CML_300to300MHz_VER006    NO.44
		//Address,Value,Mask
		{
		0x48600C, 0x6, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x2, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x7, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x0, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x1, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x0, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x0, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x0, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_8x1_4K2K60_CML_450to600MHz_VER006    NO.45
		//Address,Value,Mask
		{
		0x48600C, 0x1, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x7, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x0, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x0, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x0, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x0, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x1, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_8x1_4K2K60_CML_300to450MHz_VER006    NO.46
		//Address,Value,Mask
		{
		0x48600C, 0x2, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x7, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x0, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x1, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x1, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x0, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x1, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_8x1_4K2K60_CML_300to300MHz_VER006    NO.47
		//Address,Value,Mask
		{
		0x48600C, 0x2, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x7, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x0, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x1, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x1, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x0, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x1, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_12x1_4K2K60_CML_300to600MHz_VER006    NO.48
		//Address,Value,Mask
		{
		0x48600C, 0x2, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x7, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x0, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x1, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x1, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x0, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x0, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_12x1_4K2K60_CML_300to300MHz_VER006    NO.49
		//Address,Value,Mask
		{
		0x48600C, 0x2, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x7, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x0, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x1, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x1, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x0, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x0, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_12x1_4K1K144_CML_180to360MHz_VER006    NO.50
		//Address,Value,Mask
		{
		0x48600C, 0x2, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x7, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x0, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x1, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x1, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x1, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x0, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_12x1_4K1K144_CML_180to180MHz_VER006    NO.51
		//Address,Value,Mask
		{
		0x48600C, 0x2, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x7, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x0, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x1, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x1, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x1, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x0, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_16x1_4K2K60_CML_450to600MHz_VER006    NO.52
		//Address,Value,Mask
		{
		0x48600C, 0x1, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x7, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x1, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x0, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x0, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x0, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x0, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x1, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_16x1_4K2K60_CML_300to450MHz_VER006    NO.53
		//Address,Value,Mask
		{
		0x48600C, 0x2, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x7, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x1, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x0, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x1, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x1, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x0, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x1, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_16x1_4K2K60_CML_300to300MHz_VER006    NO.54
		//Address,Value,Mask
		{
		0x48600C, 0x2, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x7, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x1, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x0, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x1, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x1, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x0, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x1, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_24_10bit_16x1_4K2K60_CML_300to600MHz_VER006    NO.55
		//Address,Value,Mask
		{
		0x48600C, 0x3, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x9, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x0, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x1, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x1, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x0, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_24_10bit_16x1_4K2K60_CML_300to300MHz_VER006    NO.56
		//Address,Value,Mask
		{
		0x48600C, 0x3, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x9, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x0, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x1, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x1, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x0, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_24_10bit_4x2_4K2K60_CML_350to600MHz_VER006    NO.57
		//Address,Value,Mask
		{
		0x48600C, 0x1, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x2, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x9, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x0, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x0, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x0, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x1, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x0, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_24_10bit_4x2_4K2K60_CML_300to350MHz_VER006    NO.58
		//Address,Value,Mask
		{
		0x48600C, 0x3, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x9, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x0, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x1, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x1, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x0, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x1, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_24_10bit_4x2_4K2K60_CML_300to300MHz_VER006    NO.59
		//Address,Value,Mask
		{
		0x48600C, 0x3, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x9, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x0, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x1, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x1, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x0, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x1, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_24_10bit_12x1_4K2K60_CML_300to600MHz_VER006    NO.60
		//Address,Value,Mask
		{
		0x48600C, 0x2, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x2, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x0, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x1, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x0, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x0, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x1, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_24_10bit_12x1_4K2K60_CML_300to300MHz_VER006    NO.61
		//Address,Value,Mask
		{
		0x48600C, 0x2, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x2, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x0, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x1, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x0, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x0, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x1, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_24_10bit_12x1_4K1K144_CML_280to360MHz_VER006    NO.62
		//Address,Value,Mask
		{
		0x48600C, 0x0, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x2, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x0, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x0, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x0, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x1, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x1, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_24_10bit_12x1_4K1K144_CML_180to280MHz_VER006    NO.63
		//Address,Value,Mask
		{
		0x48600C, 0x2, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x2, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x0, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x1, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x0, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x1, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x1, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_24_10bit_12x1_4K1K144_CML_180to180MHz_VER006    NO.64
		//Address,Value,Mask
		{
		0x48600C, 0x2, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x2, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x0, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x1, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x0, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x1, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x1, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_2pair_2K60_LVDS_225to300MHz_VER006    NO.65
		//Address,Value,Mask
		{
		0x48600C, 0x1, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x7, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x0, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x0, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x0, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x1, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x1, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_2pair_2K60_LVDS_150to225MHz_VER006    NO.66
		//Address,Value,Mask
		{
		0x48600C, 0x2, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x7, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x0, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x1, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x1, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x1, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x1, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_2pair_2K60_LVDS_150to150MHz_VER006    NO.67
		//Address,Value,Mask
		{
		0x48600C, 0x2, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x7, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x0, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x1, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x1, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x1, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x1, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_4pair_2K60_LVDS_225to300MHz_VER006    NO.68
		//Address,Value,Mask
		{
		0x48600C, 0x1, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x7, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x1, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x0, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x0, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x0, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x1, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x1, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_4pair_2K60_LVDS_150to225MHz_VER006    NO.69
		//Address,Value,Mask
		{
		0x48600C, 0x2, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x7, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x1, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x0, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x1, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x1, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x1, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x1, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_4pair_2K60_LVDS_150to150MHz_VER006    NO.70
		//Address,Value,Mask
		{
		0x48600C, 0x2, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x7, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x1, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x0, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x1, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x1, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x1, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x1, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_6pair_2K120_LVDS_300to600MHz_VER006    NO.71
		//Address,Value,Mask
		{
		0x48600C, 0x2, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x7, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x0, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x1, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x1, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x0, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x0, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_6pair_2K120_LVDS_300to300MHz_VER006    NO.72
		//Address,Value,Mask
		{
		0x48600C, 0x2, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x7, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x0, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x1, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x1, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x0, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x0, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_8pair_2K120_LVDS_450to600MHz_VER006    NO.73
		//Address,Value,Mask
		{
		0x48600C, 0x1, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x7, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x1, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x0, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x0, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x0, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x0, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x1, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_8pair_2K120_LVDS_300to450MHz_VER006    NO.74
		//Address,Value,Mask
		{
		0x48600C, 0x2, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x7, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x1, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x0, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x1, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x1, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x0, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x1, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_8pair_2K120_LVDS_300to300MHz_VER006    NO.75
		//Address,Value,Mask
		{
		0x48600C, 0x2, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x7, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x1, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x0, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x1, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x1, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x0, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x1, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_4x2_4K2K60_LVDS_450to600MHz_VER006    NO.76
		//Address,Value,Mask
		{
		0x48600C, 0x1, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x7, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x0, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x0, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x0, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x0, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x1, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_4x2_4K2K60_LVDS_300to450MHz_VER006    NO.77
		//Address,Value,Mask
		{
		0x48600C, 0x2, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x7, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x0, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x1, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x1, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x0, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x1, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_4x2_4K2K60_LVDS_300to300MHz_VER006    NO.78
		//Address,Value,Mask
		{
		0x48600C, 0x2, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x7, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x0, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x1, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x1, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x0, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x1, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_6x1_4K2K60_LVDS_350to600MHz_VER006    NO.79
		//Address,Value,Mask
		{
		0x48600C, 0x1, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x2, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x7, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x0, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x0, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x0, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x1, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x0, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x0, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_6x1_4K2K60_LVDS_300to350MHz_VER006    NO.80
		//Address,Value,Mask
		{
		0x48600C, 0x6, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x2, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x7, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x0, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x1, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x0, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x0, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x0, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_6x1_4K2K60_LVDS_300to300MHz_VER006    NO.81
		//Address,Value,Mask
		{
		0x48600C, 0x6, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x2, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x7, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x0, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x1, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x0, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x0, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x0, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_8x1_4K2K60_LVDS_450to600MHz_VER006    NO.82
		//Address,Value,Mask
		{
		0x48600C, 0x1, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x7, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x0, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x0, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x0, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x0, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x1, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_8x1_4K2K60_LVDS_300to450MHz_VER006    NO.83
		//Address,Value,Mask
		{
		0x48600C, 0x2, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x7, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x0, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x1, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x1, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x0, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x1, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_8x1_4K2K60_LVDS_300to300MHz_VER006    NO.84
		//Address,Value,Mask
		{
		0x48600C, 0x2, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x7, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x0, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x1, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x1, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x0, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x1, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_12x1_4K2K60_LVDS_300to600MHz_VER006    NO.85
		//Address,Value,Mask
		{
		0x48600C, 0x2, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x7, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x0, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x1, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x1, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x0, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x0, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_12x1_4K2K60_LVDS_300to300MHz_VER006    NO.86
		//Address,Value,Mask
		{
		0x48600C, 0x2, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x7, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x0, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x1, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x1, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x0, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x0, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_12x1_4K1K144_LVDS_180to360MHz_VER006    NO.87
		//Address,Value,Mask
		{
		0x48600C, 0x2, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x7, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x0, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x1, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x1, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x1, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x0, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_12x1_4K1K144_LVDS_180to180MHz_VER006    NO.88
		//Address,Value,Mask
		{
		0x48600C, 0x2, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x7, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x0, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x1, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x1, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x1, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x0, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_16x1_4K_450to600MHz_VER006    NO.89
		//Address,Value,Mask
		{
		0x48600C, 0x1, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x7, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x1, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x0, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x0, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x0, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x0, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x1, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_16x1_4K_300to450MHz_VER006    NO.90
		//Address,Value,Mask
		{
		0x48600C, 0x2, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x7, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x1, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x0, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x1, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x1, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x0, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x1, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_16x1_4K_300to300MHz_VER006    NO.91
		//Address,Value,Mask
		{
		0x48600C, 0x2, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x7, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x1, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x0, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x1, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x1, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x0, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x1, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_24_10bit_16x1_4K2K60_LVDS_300to600MHz_VER006    NO.92
		//Address,Value,Mask
		{
		0x48600C, 0x3, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x9, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x0, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x1, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x1, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x0, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_24_10bit_16x1_4K2K60_LVDS_300to300MHz_VER006    NO.93
		//Address,Value,Mask
		{
		0x48600C, 0x3, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x9, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x0, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x1, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x1, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x0, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_24_10bit_4x2_4K2K60_LVDS_350to600MHz_VER006    NO.94
		//Address,Value,Mask
		{
		0x48600C, 0x1, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x2, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x9, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x0, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x0, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x0, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x1, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x0, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_24_10bit_4x2_4K2K60_LVDS_300to350MHz_VER006    NO.95
		//Address,Value,Mask
		{
		0x48600C, 0x3, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x9, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x0, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x1, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x1, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x0, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x1, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_24_10bit_4x2_4K2K60_LVDS_300to300MHz_VER006    NO.96
		//Address,Value,Mask
		{
		0x48600C, 0x3, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x9, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x0, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x1, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x1, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x0, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x1, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_24_10bit_12x1_4K2K60_LVDS_300to600MHz_VER006    NO.97
		//Address,Value,Mask
		{
		0x48600C, 0x2, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x2, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x0, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x1, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x0, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x0, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x1, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_24_10bit_12x1_4K2K60_LVDS_300to300MHz_VER006    NO.98
		//Address,Value,Mask
		{
		0x48600C, 0x2, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x2, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x0, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x1, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x0, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x0, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x1, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_24_10bit_12x1_4K1K144_LVDS_280to360MHz_VER006    NO.99
		//Address,Value,Mask
		{
		0x48600C, 0x0, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x2, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x0, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x0, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x0, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x1, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x1, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_24_10bit_12x1_4K1K144_LVDS_180to280MHz_VER006    NO.100
		//Address,Value,Mask
		{
		0x48600C, 0x2, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x2, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x0, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x1, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x0, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x1, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x1, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_24_10bit_12x1_4K1K144_LVDS_180to180MHz_VER006    NO.101
		//Address,Value,Mask
		{
		0x48600C, 0x2, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x2, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x0, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x1, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x0, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x1, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x1, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_CMPI_27_8bit_4x2_4K2K60_450to600MHz_VER006    NO.102
		//Address,Value,Mask
		{
		0x48600C, 0x1, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x9, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x0, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x0, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x1, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x1, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x2, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x8, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_CMPI_27_8bit_4x2_4K2K60_300to450MHz_VER006    NO.103
		//Address,Value,Mask
		{
		0x48600C, 0x3, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x9, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x0, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x1, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x1, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x8, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_CMPI_27_8bit_4x2_4K2K60_300to300MHz_VER006    NO.104
		//Address,Value,Mask
		{
		0x48600C, 0x3, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x9, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x0, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x1, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x1, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x8, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_CMPI_27_8bit_6x1_4K2K60_350to600MHz_VER006    NO.105
		//Address,Value,Mask
		{
		0x48600C, 0x1, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x2, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x9, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x0, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x0, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x0, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x1, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x0, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_CMPI_27_8bit_6x1_4K2K60_300to350MHz_VER006    NO.106
		//Address,Value,Mask
		{
		0x48600C, 0x3, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x9, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x0, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x1, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x1, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x0, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x1, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_CMPI_27_8bit_6x1_4K2K60_300to300MHz_VER006    NO.107
		//Address,Value,Mask
		{
		0x48600C, 0x3, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x9, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x0, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x1, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x1, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x0, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x1, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_CMPI_27_8bit_8x1_4K2K60_450to600MHz_VER006    NO.108
		//Address,Value,Mask
		{
		0x48600C, 0x1, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x9, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x0, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x0, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x1, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x1, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x2, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x8, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_CMPI_27_8bit_8x1_4K2K60_300to450MHz_VER006    NO.109
		//Address,Value,Mask
		{
		0x48600C, 0x3, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x9, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x0, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x1, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x1, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x8, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_CMPI_27_8bit_8x1_4K2K60_300to300MHz_VER006    NO.110
		//Address,Value,Mask
		{
		0x48600C, 0x3, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x9, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x0, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x1, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x1, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x8, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_CMPI_27_8bit_12x1_4K2K60_300to600MHz_VER006    NO.111
		//Address,Value,Mask
		{
		0x48600C, 0x3, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x9, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x0, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x1, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x1, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x0, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_CMPI_27_8bit_12x1_4K2K60_300to300MHz_VER006    NO.112
		//Address,Value,Mask
		{
		0x48600C, 0x3, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x9, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x0, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x1, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x1, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x0, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_CMPI_27_8bit_12x1_4K1K144_180to360MHz_VER006    NO.113
		//Address,Value,Mask
		{
		0x48600C, 0x3, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x9, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x0, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x1, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x1, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x1, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_CMPI_27_8bit_12x1_4K1K144_180to180MHz_VER006    NO.114
		//Address,Value,Mask
		{
		0x48600C, 0x3, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x9, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x0, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x1, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x1, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x1, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_CMPI_24_10bit_4x2_4K2K60_350to600MHz_VER006    NO.115
		//Address,Value,Mask
		{
		0x48600C, 0x1, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x2, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x9, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x0, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x0, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x0, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x1, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x0, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_CMPI_24_10bit_4x2_4K2K60_300to350MHz_VER006    NO.116
		//Address,Value,Mask
		{
		0x48600C, 0x3, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x9, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x0, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x1, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x1, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x0, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x1, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_CMPI_24_10bit_4x2_4K2K60_300to300MHz_VER006    NO.117
		//Address,Value,Mask
		{
		0x48600C, 0x3, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x9, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x0, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x1, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x1, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x0, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x1, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_CMPI_24_10bit_8x1_4K2K60_350to600MHz_VER006    NO.118
		//Address,Value,Mask
		{
		0x48600C, 0x1, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x2, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x9, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x0, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x0, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x0, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x1, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x0, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_CMPI_24_10bit_8x1_4K2K60_300to350MHz_VER006    NO.119
		//Address,Value,Mask
		{
		0x48600C, 0x3, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x9, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x0, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x1, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x1, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x0, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x1, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_CMPI_24_10bit_8x1_4K2K60_300to300MHz_VER006    NO.120
		//Address,Value,Mask
		{
		0x48600C, 0x3, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x9, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x0, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x1, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x1, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x0, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x1, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_CMPI_24_10bit_12x1_4K2K60_300to600MHz_VER006    NO.121
		//Address,Value,Mask
		{
		0x48600C, 0x2, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x2, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x0, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x1, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x0, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x0, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x1, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_CMPI_24_10bit_12x1_4K2K60_300to300MHz_VER006    NO.122
		//Address,Value,Mask
		{
		0x48600C, 0x2, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x2, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x0, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x1, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x0, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x0, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x1, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_CMPI_24_10bit_12x1_4K1K144_280to360MHz_VER006    NO.123
		//Address,Value,Mask
		{
		0x48600C, 0x0, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x2, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x0, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x0, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x0, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x1, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x1, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_CMPI_24_10bit_12x1_4K1K144_180to280MHz_VER006    NO.124
		//Address,Value,Mask
		{
		0x48600C, 0x2, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x2, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x0, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x1, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x0, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x1, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x1, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_CMPI_24_10bit_12x1_4K1K144_180to180MHz_VER006    NO.125
		//Address,Value,Mask
		{
		0x48600C, 0x2, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x2, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x0, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x1, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x0, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x1, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x1, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_ISP_8bit_4x2_4K2K60_450to600MHz_VER006    NO.126
		//Address,Value,Mask
		{
		0x48600C, 0x1, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x9, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x0, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x1, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x1, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x1, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x2, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x8, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_ISP_8bit_4x2_4K2K60_300to450MHz_VER006    NO.127
		//Address,Value,Mask
		{
		0x48600C, 0x3, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x9, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x1, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x1, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x1, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x8, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_ISP_8bit_4x2_4K2K60_300to300MHz_VER006    NO.128
		//Address,Value,Mask
		{
		0x48600C, 0x3, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x9, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x1, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x1, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x1, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x8, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_ISP_8bit_6x1_4K2K60_350to600MHz_VER006    NO.129
		//Address,Value,Mask
		{
		0x48600C, 0x1, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x2, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x9, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x0, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x1, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x0, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x1, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x0, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_ISP_8bit_6x1_4K2K60_300to350MHz_VER006    NO.130
		//Address,Value,Mask
		{
		0x48600C, 0x3, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x9, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x1, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x1, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x1, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x0, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x1, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_ISP_8bit_6x1_4K2K60_300to300MHz_VER006    NO.131
		//Address,Value,Mask
		{
		0x48600C, 0x3, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x9, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x1, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x1, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x1, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x0, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x1, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_ISP_8bit_6x2_4K2K60_300to600MHz_VER006    NO.132
		//Address,Value,Mask
		{
		0x48600C, 0x3, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x9, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x1, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x1, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x1, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x0, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_ISP_8bit_6x2_4K2K60_300to300MHz_VER006    NO.133
		//Address,Value,Mask
		{
		0x48600C, 0x3, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x9, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x1, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x1, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x1, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x0, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_ISP_8bit_8x1_4K2K60_450to600MHz_VER006    NO.134
		//Address,Value,Mask
		{
		0x48600C, 0x1, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x9, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x0, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x1, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x1, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x1, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x2, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x8, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_ISP_8bit_8x1_4K2K60_300to450MHz_VER006    NO.135
		//Address,Value,Mask
		{
		0x48600C, 0x3, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x9, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x1, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x1, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x1, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x8, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_ISP_8bit_8x1_4K2K60_300to300MHz_VER006    NO.136
		//Address,Value,Mask
		{
		0x48600C, 0x3, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x9, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x1, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x1, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x1, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x8, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_ISP_8bit_12x1_4K2K60_300to600MHz_VER006    NO.137
		//Address,Value,Mask
		{
		0x48600C, 0x3, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x9, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x1, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x1, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x1, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x0, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_ISP_8bit_12x1_4K2K60_300to300MHz_VER006    NO.138
		//Address,Value,Mask
		{
		0x48600C, 0x3, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x9, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x1, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x1, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x1, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x0, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_ISP_8bit_12x1_4K1K144_180to360MHz_VER006    NO.139
		//Address,Value,Mask
		{
		0x48600C, 0x3, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x9, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x1, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x1, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x1, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x1, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_ISP_8bit_12x1_4K1K144_180to180MHz_VER006    NO.140
		//Address,Value,Mask
		{
		0x48600C, 0x3, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x9, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x1, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x1, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x1, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x1, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_ISP_10bit_4x2_4K2K60_400to600MHz_VER006    NO.141
		//Address,Value,Mask
		{
		0x48600C, 0x1, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x9, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x0, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x1, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x1, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x0, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x1, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x4, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_ISP_10bit_4x2_4K2K60_300to400MHz_VER006    NO.142
		//Address,Value,Mask
		{
		0x48600C, 0x6, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x9, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x1, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x1, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x1, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x0, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x4, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_ISP_10bit_4x2_4K2K60_300to300MHz_VER006    NO.143
		//Address,Value,Mask
		{
		0x48600C, 0x6, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x9, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x1, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x1, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x1, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x0, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x4, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_ISP_10bit_6x2_4K2K60_300to600MHz_VER006    NO.144
		//Address,Value,Mask
		{
		0x48600C, 0x3, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x9, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x1, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x1, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x1, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x1, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x2, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x4, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_ISP_10bit_6x2_4K2K60_300to300MHz_VER006    NO.145
		//Address,Value,Mask
		{
		0x48600C, 0x3, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x9, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x1, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x1, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x1, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x1, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x2, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x4, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_ISP_10bit_8x1_4K2K60_400to600MHz_VER006    NO.146
		//Address,Value,Mask
		{
		0x48600C, 0x1, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x9, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x0, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x1, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x1, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x0, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x1, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x4, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_ISP_10bit_8x1_4K2K60_300to400MHz_VER006    NO.147
		//Address,Value,Mask
		{
		0x48600C, 0x6, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x9, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x1, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x1, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x1, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x0, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x4, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_ISP_10bit_8x1_4K2K60_300to300MHz_VER006    NO.148
		//Address,Value,Mask
		{
		0x48600C, 0x6, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x9, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x1, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x1, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x1, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x0, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x4, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_ISP_10bit_12x1_4K2K60_300to600MHz_VER006    NO.149
		//Address,Value,Mask
		{
		0x48600C, 0x3, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x9, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x1, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x1, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x1, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x1, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x2, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x4, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_ISP_10bit_12x1_4K2K60_300to300MHz_VER006    NO.150
		//Address,Value,Mask
		{
		0x48600C, 0x3, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x9, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x1, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x1, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x1, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x1, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x2, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x4, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_ISP_10bit_12x1_4K1K144_300to360MHz_VER006    NO.151
		//Address,Value,Mask
		{
		0x48600C, 0x1, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x9, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x0, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x1, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x1, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x1, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x2, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x5, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_ISP_10bit_12x1_4K1K144_180to300MHz_VER006    NO.152
		//Address,Value,Mask
		{
		0x48600C, 0x3, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x9, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x1, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x1, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x1, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x1, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x2, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x5, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_ISP_10bit_12x1_4K1K144_180to180MHz_VER006    NO.153
		//Address,Value,Mask
		{
		0x48600C, 0x3, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x9, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x1, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x1, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x1, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x1, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x2, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x5, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_CHPI_8bit_4x2_4K2K60_450to600MHz_VER006    NO.154
		//Address,Value,Mask
		{
		0x48600C, 0x2, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x3, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x0, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x1, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x0, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x1, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x0, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x1, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x2, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x4, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_CHPI_8bit_4x2_4K2K60_300to450MHz_VER006    NO.155
		//Address,Value,Mask
		{
		0x48600C, 0x5, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x3, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x0, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x1, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x1, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x1, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x0, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x4, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_CHPI_8bit_4x2_4K2K60_300to300MHz_VER006    NO.156
		//Address,Value,Mask
		{
		0x48600C, 0x5, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x3, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x0, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x1, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x1, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x1, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x0, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x4, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_CHPI_8bit_6x1_4K2K60_300to600MHz_VER006    NO.157
		//Address,Value,Mask
		{
		0x48600C, 0x1, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x3, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x5, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x0, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x1, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x0, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x1, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x0, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x1, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x0, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_CHPI_8bit_6x1_4K2K60_300to300MHz_VER006    NO.158
		//Address,Value,Mask
		{
		0x48600C, 0x1, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x3, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x5, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x0, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x1, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x0, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x1, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x0, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x1, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x0, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_CHPI_8bit_6x2_4K2K60_300to600MHz_VER006    NO.159
		//Address,Value,Mask
		{
		0x48600C, 0x1, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x5, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x0, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x1, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x1, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x1, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x1, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x0, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x1, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_CHPI_8bit_6x2_4K2K60_300to300MHz_VER006    NO.160
		//Address,Value,Mask
		{
		0x48600C, 0x1, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x5, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x0, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x1, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x1, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x1, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x1, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x0, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x1, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_CHPI_8bit_8x1_4K2K60_450to600MHz_VER006    NO.161
		//Address,Value,Mask
		{
		0x48600C, 0x2, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x3, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x0, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x1, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x0, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x1, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x0, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x1, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x2, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x4, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_CHPI_8bit_8x1_4K2K60_300to450MHz_VER006    NO.162
		//Address,Value,Mask
		{
		0x48600C, 0x5, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x3, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x0, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x1, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x1, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x1, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x0, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x4, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_CHPI_8bit_8x1_4K2K60_300to300MHz_VER006    NO.163
		//Address,Value,Mask
		{
		0x48600C, 0x5, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x3, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x0, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x1, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x1, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x1, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x0, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x4, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_CHPI_8bit_12x1_4K2K60_300to600MHz_VER006    NO.164
		//Address,Value,Mask
		{
		0x48600C, 0x1, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x5, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x0, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x1, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x1, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x1, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x1, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x0, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x1, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_CHPI_8bit_12x1_4K2K60_300to300MHz_VER006    NO.165
		//Address,Value,Mask
		{
		0x48600C, 0x1, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x5, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x0, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x1, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x1, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x1, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x1, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x0, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x1, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_CHPI_8bit_12x1_4K1K144_180to360MHz_VER006    NO.166
		//Address,Value,Mask
		{
		0x48600C, 0x1, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x5, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x0, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x1, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x1, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x1, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x1, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x1, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x1, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_CHPI_8bit_12x1_4K1K144_180to180MHz_VER006    NO.167
		//Address,Value,Mask
		{
		0x48600C, 0x1, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x5, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x0, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x1, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x1, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x1, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x1, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x1, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x1, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_CHPI_10bit_4x2_4K2K60_350to600MHz_VER006    NO.168
		//Address,Value,Mask
		{
		0x48600C, 0x1, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0xF, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x0, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x1, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x0, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x1, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x0, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x1, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x4, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_CHPI_10bit_4x2_4K2K60_300to350MHz_VER006    NO.169
		//Address,Value,Mask
		{
		0x48600C, 0x6, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0xF, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x0, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x1, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x1, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x1, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x1, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x4, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_CHPI_10bit_4x2_4K2K60_300to300MHz_VER006    NO.170
		//Address,Value,Mask
		{
		0x48600C, 0x6, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0xF, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x0, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x1, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x1, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x1, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x1, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x4, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_CHPI_10bit_8x1_4K2K60_350to600MHz_VER006    NO.171
		//Address,Value,Mask
		{
		0x48600C, 0x1, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0xF, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x0, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x1, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x0, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x1, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x0, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x1, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x4, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_CHPI_10bit_8x1_4K2K60_300to350MHz_VER006    NO.172
		//Address,Value,Mask
		{
		0x48600C, 0x6, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0xF, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x0, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x1, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x1, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x1, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x1, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x4, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_CHPI_10bit_8x1_4K2K60_300to300MHz_VER006    NO.173
		//Address,Value,Mask
		{
		0x48600C, 0x6, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0xF, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x0, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x1, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x1, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x1, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x1, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x4, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_CHPI_10bit_12x1_4K2K60_500to600MHz_VER006    NO.174
		//Address,Value,Mask
		{
		0x48600C, 0x1, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x2, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x5, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x0, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x1, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x0, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x1, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x1, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x1, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x2, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x4, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_CHPI_10bit_12x1_4K2K60_300to500MHz_VER006    NO.175
		//Address,Value,Mask
		{
		0x48600C, 0x4, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x2, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x5, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x0, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x1, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x1, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x1, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x0, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x4, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_CHPI_10bit_12x1_4K2K60_300to300MHz_VER006    NO.176
		//Address,Value,Mask
		{
		0x48600C, 0x4, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x2, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x5, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x0, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x1, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x1, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x1, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x0, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x4, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_CHPI_10bit_12x1_4K1K144_280to360MHz_VER006    NO.177
		//Address,Value,Mask
		{
		0x48600C, 0x1, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x2, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x5, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x0, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x1, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x0, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x1, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x1, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x1, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x2, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x5, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_CHPI_10bit_12x1_4K1K144_180to280MHz_VER006    NO.178
		//Address,Value,Mask
		{
		0x48600C, 0x4, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x2, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x5, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x0, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x1, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x1, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x1, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x0, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x5, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_CHPI_10bit_12x1_4K1K144_180to180MHz_VER006    NO.179
		//Address,Value,Mask
		{
		0x48600C, 0x4, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x2, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x5, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x0, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x1, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x1, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x1, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x0, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x5, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_Vx1_10bit_2_lane_2K60_150to300MHz_VER006    NO.180
		//Address,Value,Mask
		{
		0x48600C, 0x1, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x2, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x5, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x0, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x0, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x1, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x1, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x1, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_Vx1_10bit_2_lane_2K60_150to150MHz_VER006    NO.181
		//Address,Value,Mask
		{
		0x48600C, 0x1, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x2, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x5, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x0, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x0, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x1, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x1, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x1, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_Vx1_10bit_4_lane_4K2K30_300to600MHz_VER006    NO.182
		//Address,Value,Mask
		{
		0x48600C, 0x1, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x2, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x5, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x0, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x0, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x1, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x0, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x1, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_Vx1_10bit_4_lane_4K2K30_300to300MHz_VER006    NO.183
		//Address,Value,Mask
		{
		0x48600C, 0x1, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x2, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x5, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x0, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x0, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x1, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x0, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x1, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_Vx1_10bit_8_lane_4K2K60_300to600MHz_VER006    NO.184
		//Address,Value,Mask
		{
		0x48600C, 0x1, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x2, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x5, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x0, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x0, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x1, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x0, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x1, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_Vx1_10bit_8_lane_4K2K60_300to300MHz_VER006    NO.185
		//Address,Value,Mask
		{
		0x48600C, 0x1, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x2, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x5, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x0, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x0, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x1, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x0, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x1, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_Vx1_10bit_8_lane_4K1K144_180to360MHz_VER006    NO.186
		//Address,Value,Mask
		{
		0x48600C, 0x1, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x2, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x5, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x0, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x0, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x1, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x1, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x1, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_Vx1_10bit_8_lane_4K1K144_180to180MHz_VER006    NO.187
		//Address,Value,Mask
		{
		0x48600C, 0x1, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x2, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x5, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x0, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x0, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x1, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x1, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x1, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_Vx1_8bit_2_lane_2K60_225to300MHz_VER006    NO.188
		//Address,Value,Mask
		{
		0x48600C, 0x2, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x3, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x0, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x0, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x0, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x1, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x2, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x5, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_Vx1_8bit_2_lane_2K60_150to225MHz_VER006    NO.189
		//Address,Value,Mask
		{
		0x48600C, 0x5, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x3, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x0, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x1, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x0, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x5, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_Vx1_8bit_2_lane_2K60_150to150MHz_VER006    NO.190
		//Address,Value,Mask
		{
		0x48600C, 0x5, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x3, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x0, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x1, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x0, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x5, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_Vx1_8bit_4_lane_4K2K30_450to600MHz_VER006    NO.191
		//Address,Value,Mask
		{
		0x48600C, 0x2, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x3, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x0, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x0, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x0, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x1, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x2, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x4, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_Vx1_8bit_4_lane_4K2K30_300to450MHz_VER006    NO.192
		//Address,Value,Mask
		{
		0x48600C, 0x5, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x3, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x0, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x1, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x0, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x4, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_Vx1_8bit_4_lane_4K2K30_300to300MHz_VER006    NO.193
		//Address,Value,Mask
		{
		0x48600C, 0x5, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x3, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x0, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x1, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x0, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x4, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_Vx1_8bit_8_lane_4K2K60_450to600MHz_VER006    NO.194
		//Address,Value,Mask
		{
		0x48600C, 0x2, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x3, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x0, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x0, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x0, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x1, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x2, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x4, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_Vx1_8bit_8_lane_4K2K60_300to450MHz_VER006    NO.195
		//Address,Value,Mask
		{
		0x48600C, 0x5, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x3, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x0, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x1, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x0, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x4, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_Vx1_8bit_8_lane_4K2K60_300to300MHz_VER006    NO.196
		//Address,Value,Mask
		{
		0x48600C, 0x5, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x3, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x0, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x1, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x0, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x4, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_Vx1_8bit_8_lane_4K1K144_225to360MHz_VER006    NO.197
		//Address,Value,Mask
		{
		0x48600C, 0x2, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x3, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x0, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x0, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x0, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x1, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x2, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x5, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_Vx1_8bit_8_lane_4K1K144_180to225MHz_VER006    NO.198
		//Address,Value,Mask
		{
		0x48600C, 0x5, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x3, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x0, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x1, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x0, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x5, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
	},   {		//E_PNL_SUPPORTED_LPLL_Vx1_8bit_8_lane_4K1K144_180to180MHz_VER006    NO.199
		//Address,Value,Mask
		{
		0x48600C, 0x5, Fld(3, 2, AC_MSKW10)},	//reg_lpll1_ibias_ictrl
		{
		0x486054, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_input_div_first
		{
		0x486004, 0x3, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_loop_div_first
		{
		0x486004, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll1_loop_div_second
		{
		0x486008, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_lpll1_scalar_div_first
		{
		0x486008, 0x0, Fld(4, 8, AC_MSKW10)},	//reg_lpll1_scalar_div_second
		{
		0x4860D4, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_lpll1_skew_div
		{
		0x4860B8, 0x0, Fld(3, 0, AC_MSKW10)},	//reg_lpll1_fifo_div
		{
		0x48600C, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll1_fifo_div5_en
		{
		0x4860B8, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll1_en_fix_clk
		{
		0x4860D4, 0x1, Fld(1, 5, AC_MSKW10)},	//reg_lpll_2ndpll_clk_sel
		{
		0x4860DC, 0x1, Fld(1, 3, AC_MSKW10)},	//reg_lpll1_test[19]
		{
		0x4860DC, 0x0, Fld(1, 4, AC_MSKW10)},	//reg_lpll1_test[20]
		{
		0x4860B8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_duap_lp_en
		{
		0x486010, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_lpll1_sdiv3p5_en
		{
		0x48600C, 0x0, Fld(1, 10, AC_MSKW10)},	//reg_lpll1_sdiv2p5_en
		{
		0x4860B8, 0x0, Fld(1, 14, AC_MSKW10)},	//reg_lpll1_en_mini
		{
		0x4860BC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_en_fifo
		{
		0x4860BC, 0x1, Fld(1, 1, AC_MSKW10)},	//reg_lpll1_en_scalar
		{
		0x4860DC, 0x0, Fld(1, 2, AC_MSKW10)},	//reg_lpll1_test[18]
		{
		0x4860D8, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_lpll1_test[15]
		{
		0x4860DC, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_lpll1_test[16]
		{
		0x4860DC, 0x0, Fld(1, 6, AC_MSKW10)},	//reg_lpll1_test[22]
		{
		0x4860CC, 0x0, Fld(1, 5, AC_MSKW10)},	//reg_lpll2_pd
		{
		0x4860CC, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll2_ibias_ctrl
		{
		0x4860C0, 0x0, Fld(5, 0, AC_MSKW10)},	//reg_lpll2_input_div
		{
		0x4860C4, 0x1, Fld(2, 0, AC_MSKW10)},	//reg_lpll2_loop_div_first
		{
		0x4860C4, 0x3, Fld(5, 8, AC_MSKW10)},	//reg_lpll2_loop_div_second
		{
		0x4860C8, 0x5, Fld(4, 0, AC_MSKW10)},	//reg_lpll2_post_div
		{
		0x4860E4, 0x0, Fld(1, 11, AC_MSKW10)},	//reg_lpll2_test[11]
		{
		0x4860E4, 0x1, Fld(1, 12, AC_MSKW10)},	//reg_lpll2_test[12]
		{
		0x4860E4, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll2_test[8]
		{
		0x4860CC, 0x0, Fld(1, 7, AC_MSKW10)},	//reg_lpll2_div14_en
		{
		0x4860E0, 0x0, Fld(1, 9, AC_MSKW10)},	//reg_lpll1_scalar2fifo_en
		{
		0x4860E0, 0x0, Fld(1, 8, AC_MSKW10)},	//reg_lpll1_scalar2fifo_div2
},  };

TBLStruct MODASettingTBL_VER006[PNL_NUM_VER006][MODA_REG_NUM_VER006] =  {
	 {			//E_PNL_SUPPORTED_LPLL_LVDS_1ch_HD60_225to300MHz_VER006    NO.0
		//Address,Value,Mask
		{
		0x484600, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x0, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_LVDS_1ch_HD60_150to225MHz_VER006    NO.1
		//Address,Value,Mask
		{
		0x484600, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x0, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_LVDS_1ch_HD60_150to150MHz_VER006    NO.2
		//Address,Value,Mask
		{
		0x484600, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x0, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_LVDS_2ch_2K60_225to300MHz_VER006    NO.3
		//Address,Value,Mask
		{
		0x484600, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x0, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_LVDS_2ch_2K60_150to225MHz_VER006    NO.4
		//Address,Value,Mask
		{
		0x484600, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x0, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_LVDS_2ch_2K60_150to150MHz_VER006    NO.5
		//Address,Value,Mask
		{
		0x484600, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x0, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_HS_LVDS_1ch_2K60_225to300MHz_VER006    NO.6
		//Address,Value,Mask
		{
		0x484600, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x0, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_HS_LVDS_1ch_2K60_150to225MHz_VER006    NO.7
		//Address,Value,Mask
		{
		0x484600, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x0, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_HS_LVDS_1ch_2K60_150to150MHz_VER006    NO.8
		//Address,Value,Mask
		{
		0x484600, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x0, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_HS_LVDS_2ch_4K2K30_225to300MHz_VER006    NO.9
		//Address,Value,Mask
		{
		0x484600, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x0, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_HS_LVDS_2ch_4K2K30_150to225MHz_VER006    NO.10
		//Address,Value,Mask
		{
		0x484600, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x0, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_HS_LVDS_2ch_4K2K30_150to150MHz_VER006    NO.11
		//Address,Value,Mask
		{
		0x484600, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x0, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_MINILVDS_1bk_3pair_6bit_HD60_150to300MHz_VER006    NO.12
		//Address,Value,Mask
		{
		0x484600, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x0, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_MINILVDS_1bk_3pair_6bit_HD60_150to150MHz_VER006    NO.13
		//Address,Value,Mask
		{
		0x484600, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x0, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_MINILVDS_1bk_6pair_6bit_HD60_150to300MHz_VER006    NO.14
		//Address,Value,Mask
		{
		0x484600, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x0, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_MINILVDS_1bk_6pair_6bit_HD60_150to150MHz_VER006    NO.15
		//Address,Value,Mask
		{
		0x484600, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x0, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_MINILVDS_2bk_3pair_6bit_2K60_150to300MHz_VER006    NO.16
		//Address,Value,Mask
		{
		0x484600, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x0, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_MINILVDS_2bk_3pair_6bit_2K60_150to150MHz_VER006    NO.17
		//Address,Value,Mask
		{
		0x484600, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x0, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_MINILVDS_2bk_6pair_6bit_2K60_150to300MHz_VER006    NO.18
		//Address,Value,Mask
		{
		0x484600, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x0, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_MINILVDS_2bk_6pair_6bit_2K60_150to150MHz_VER006    NO.19
		//Address,Value,Mask
		{
		0x484600, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x0, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_MINILVDS_1bk_3pair_8bit_HD60_150to300MHz_VER006    NO.20
		//Address,Value,Mask
		{
		0x484600, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x0, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_MINILVDS_1bk_3pair_8bit_HD60_150to150MHz_VER006    NO.21
		//Address,Value,Mask
		{
		0x484600, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x0, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_MINILVDS_1bk_6pair_8bit_HD60_150to300MHz_VER006    NO.22
		//Address,Value,Mask
		{
		0x484600, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x0, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_MINILVDS_1bk_6pair_8bit_HD60_150to150MHz_VER006    NO.23
		//Address,Value,Mask
		{
		0x484600, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x0, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_MINILVDS_2bk_3pair_8bit_2K60_150to300MHz_VER006    NO.24
		//Address,Value,Mask
		{
		0x484600, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x0, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_MINILVDS_2bk_3pair_8bit_2K60_150to150MHz_VER006    NO.25
		//Address,Value,Mask
		{
		0x484600, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x0, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_MINILVDS_2bk_6pair_8bit_2K60_150to300MHz_VER006    NO.26
		//Address,Value,Mask
		{
		0x484600, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x0, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_MINILVDS_2bk_6pair_8bit_2K60_150to150MHz_VER006    NO.27
		//Address,Value,Mask
		{
		0x484600, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x0, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_2pair_2K60_CML_225to300MHz_VER006    NO.28
		//Address,Value,Mask
		{
		0x484600, 0x1, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x1, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x0, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x3FFF, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x500, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_2pair_2K60_CML_150to225MHz_VER006    NO.29
		//Address,Value,Mask
		{
		0x484600, 0x1, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x1, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x0, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x3FFF, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x500, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_2pair_2K60_CML_150to150MHz_VER006    NO.30
		//Address,Value,Mask
		{
		0x484600, 0x1, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x1, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x0, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x3FFF, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x500, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_4pair_2K60_CML_225to300MHz_VER006    NO.31
		//Address,Value,Mask
		{
		0x484600, 0x1, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x1, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x0, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x3FFF, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x500, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_4pair_2K60_CML_150to225MHz_VER006    NO.32
		//Address,Value,Mask
		{
		0x484600, 0x1, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x1, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x0, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x3FFF, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x500, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_4pair_2K60_CML_150to150MHz_VER006    NO.33
		//Address,Value,Mask
		{
		0x484600, 0x1, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x1, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x0, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x3FFF, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x500, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_6pair_2K120_CML_300to600MHz_VER006    NO.34
		//Address,Value,Mask
		{
		0x484600, 0x1, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x1, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x0, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x3FFF, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x500, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_6pair_2K120_CML_300to300MHz_VER006    NO.35
		//Address,Value,Mask
		{
		0x484600, 0x1, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x1, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x0, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x3FFF, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x500, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_8pair_2K120_CML_450to600MHz_VER006    NO.36
		//Address,Value,Mask
		{
		0x484600, 0x1, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x1, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x0, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x3FFF, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x500, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_8pair_2K120_CML_300to450MHz_VER006    NO.37
		//Address,Value,Mask
		{
		0x484600, 0x1, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x1, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x0, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x3FFF, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x500, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_8pair_2K120_CML_300to300MHz_VER006    NO.38
		//Address,Value,Mask
		{
		0x484600, 0x1, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x1, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x0, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x3FFF, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x500, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_4x2_4K2K60_CML_450to600MHz_VER006    NO.39
		//Address,Value,Mask
		{
		0x484600, 0x1, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x1, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x0, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x3FFF, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x500, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_4x2_4K2K60_CML_300to450MHz_VER006    NO.40
		//Address,Value,Mask
		{
		0x484600, 0x1, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x1, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x0, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x3FFF, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x500, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_4x2_4K2K60_CML_300to300MHz_VER006    NO.41
		//Address,Value,Mask
		{
		0x484600, 0x1, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x1, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x0, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x3FFF, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x500, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_6x1_4K2K60_CML_350to600MHz_VER006    NO.42
		//Address,Value,Mask
		{
		0x484600, 0x1, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x1, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x0, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x3FFF, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x500, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_6x1_4K2K60_CML_300to350MHz_VER006    NO.43
		//Address,Value,Mask
		{
		0x484600, 0x1, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x1, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x0, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x3FFF, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x500, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_6x1_4K2K60_CML_300to300MHz_VER006    NO.44
		//Address,Value,Mask
		{
		0x484600, 0x1, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x1, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x0, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x3FFF, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x500, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_8x1_4K2K60_CML_450to600MHz_VER006    NO.45
		//Address,Value,Mask
		{
		0x484600, 0x1, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x1, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x0, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x3FFF, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x500, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_8x1_4K2K60_CML_300to450MHz_VER006    NO.46
		//Address,Value,Mask
		{
		0x484600, 0x1, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x1, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x0, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x3FFF, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x500, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_8x1_4K2K60_CML_300to300MHz_VER006    NO.47
		//Address,Value,Mask
		{
		0x484600, 0x1, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x1, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x0, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x3FFF, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x500, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_12x1_4K2K60_CML_300to600MHz_VER006    NO.48
		//Address,Value,Mask
		{
		0x484600, 0x1, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x1, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x0, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x3FFF, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x500, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_12x1_4K2K60_CML_300to300MHz_VER006    NO.49
		//Address,Value,Mask
		{
		0x484600, 0x1, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x1, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x0, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x3FFF, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x500, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_12x1_4K1K144_CML_180to360MHz_VER006    NO.50
		//Address,Value,Mask
		{
		0x484600, 0x1, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x1, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x0, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x3FFF, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x500, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_12x1_4K1K144_CML_180to180MHz_VER006    NO.51
		//Address,Value,Mask
		{
		0x484600, 0x1, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x1, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x0, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x3FFF, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x500, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_16x1_4K2K60_CML_450to600MHz_VER006    NO.52
		//Address,Value,Mask
		{
		0x484600, 0x1, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x1, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x0, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x3FFF, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x500, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_16x1_4K2K60_CML_300to450MHz_VER006    NO.53
		//Address,Value,Mask
		{
		0x484600, 0x1, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x1, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x0, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x3FFF, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x500, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_16x1_4K2K60_CML_300to300MHz_VER006    NO.54
		//Address,Value,Mask
		{
		0x484600, 0x1, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x1, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x0, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x3FFF, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x500, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_24_10bit_16x1_4K2K60_CML_300to600MHz_VER006    NO.55
		//Address,Value,Mask
		{
		0x484600, 0x1, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x1, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x0, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x3FFF, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x500, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_24_10bit_16x1_4K2K60_CML_300to300MHz_VER006    NO.56
		//Address,Value,Mask
		{
		0x484600, 0x1, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x1, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x0, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x3FFF, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x500, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_24_10bit_4x2_4K2K60_CML_350to600MHz_VER006    NO.57
		//Address,Value,Mask
		{
		0x484600, 0x1, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x1, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x0, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x3FFF, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x500, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_24_10bit_4x2_4K2K60_CML_300to350MHz_VER006    NO.58
		//Address,Value,Mask
		{
		0x484600, 0x1, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x1, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x0, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x3FFF, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x500, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_24_10bit_4x2_4K2K60_CML_300to300MHz_VER006    NO.59
		//Address,Value,Mask
		{
		0x484600, 0x1, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x1, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x0, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x3FFF, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x500, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_24_10bit_12x1_4K2K60_CML_300to600MHz_VER006    NO.60
		//Address,Value,Mask
		{
		0x484600, 0x1, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x1, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x0, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x3FFF, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x500, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_24_10bit_12x1_4K2K60_CML_300to300MHz_VER006    NO.61
		//Address,Value,Mask
		{
		0x484600, 0x1, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x1, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x0, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x3FFF, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x500, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_24_10bit_12x1_4K1K144_CML_280to360MHz_VER006    NO.62
		//Address,Value,Mask
		{
		0x484600, 0x1, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x1, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x0, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x3FFF, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x500, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_24_10bit_12x1_4K1K144_CML_180to280MHz_VER006    NO.63
		//Address,Value,Mask
		{
		0x484600, 0x1, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x1, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x0, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x3FFF, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x500, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_24_10bit_12x1_4K1K144_CML_180to180MHz_VER006    NO.64
		//Address,Value,Mask
		{
		0x484600, 0x1, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x1, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x0, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x3FFF, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x500, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_2pair_2K60_LVDS_225to300MHz_VER006    NO.65
		//Address,Value,Mask
		{
		0x484600, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x1, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_2pair_2K60_LVDS_150to225MHz_VER006    NO.66
		//Address,Value,Mask
		{
		0x484600, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x1, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_2pair_2K60_LVDS_150to150MHz_VER006    NO.67
		//Address,Value,Mask
		{
		0x484600, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x1, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_4pair_2K60_LVDS_225to300MHz_VER006    NO.68
		//Address,Value,Mask
		{
		0x484600, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x1, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_4pair_2K60_LVDS_150to225MHz_VER006    NO.69
		//Address,Value,Mask
		{
		0x484600, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x1, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_4pair_2K60_LVDS_150to150MHz_VER006    NO.70
		//Address,Value,Mask
		{
		0x484600, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x1, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_6pair_2K120_LVDS_300to600MHz_VER006    NO.71
		//Address,Value,Mask
		{
		0x484600, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x1, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_6pair_2K120_LVDS_300to300MHz_VER006    NO.72
		//Address,Value,Mask
		{
		0x484600, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x1, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_8pair_2K120_LVDS_450to600MHz_VER006    NO.73
		//Address,Value,Mask
		{
		0x484600, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x1, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_8pair_2K120_LVDS_300to450MHz_VER006    NO.74
		//Address,Value,Mask
		{
		0x484600, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x1, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_8pair_2K120_LVDS_300to300MHz_VER006    NO.75
		//Address,Value,Mask
		{
		0x484600, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x1, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_4x2_4K2K60_LVDS_450to600MHz_VER006    NO.76
		//Address,Value,Mask
		{
		0x484600, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x1, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_4x2_4K2K60_LVDS_300to450MHz_VER006    NO.77
		//Address,Value,Mask
		{
		0x484600, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x1, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_4x2_4K2K60_LVDS_300to300MHz_VER006    NO.78
		//Address,Value,Mask
		{
		0x484600, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x1, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_6x1_4K2K60_LVDS_350to600MHz_VER006    NO.79
		//Address,Value,Mask
		{
		0x484600, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x1, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_6x1_4K2K60_LVDS_300to350MHz_VER006    NO.80
		//Address,Value,Mask
		{
		0x484600, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x1, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_6x1_4K2K60_LVDS_300to300MHz_VER006    NO.81
		//Address,Value,Mask
		{
		0x484600, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x1, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_8x1_4K2K60_LVDS_450to600MHz_VER006    NO.82
		//Address,Value,Mask
		{
		0x484600, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x1, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_8x1_4K2K60_LVDS_300to450MHz_VER006    NO.83
		//Address,Value,Mask
		{
		0x484600, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x1, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_8x1_4K2K60_LVDS_300to300MHz_VER006    NO.84
		//Address,Value,Mask
		{
		0x484600, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x1, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_12x1_4K2K60_LVDS_300to600MHz_VER006    NO.85
		//Address,Value,Mask
		{
		0x484600, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x1, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_12x1_4K2K60_LVDS_300to300MHz_VER006    NO.86
		//Address,Value,Mask
		{
		0x484600, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x1, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_12x1_4K1K144_LVDS_180to360MHz_VER006    NO.87
		//Address,Value,Mask
		{
		0x484600, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x1, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_12x1_4K1K144_LVDS_180to180MHz_VER006    NO.88
		//Address,Value,Mask
		{
		0x484600, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x1, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_16x1_4K_450to600MHz_VER006    NO.89
		//Address,Value,Mask
		{
		0x484600, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x1, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_16x1_4K_300to450MHz_VER006    NO.90
		//Address,Value,Mask
		{
		0x484600, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x1, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_16x1_4K_300to300MHz_VER006    NO.91
		//Address,Value,Mask
		{
		0x484600, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x1, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_24_10bit_16x1_4K2K60_LVDS_300to600MHz_VER006    NO.92
		//Address,Value,Mask
		{
		0x484600, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x1, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_24_10bit_16x1_4K2K60_LVDS_300to300MHz_VER006    NO.93
		//Address,Value,Mask
		{
		0x484600, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x1, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_24_10bit_4x2_4K2K60_LVDS_350to600MHz_VER006    NO.94
		//Address,Value,Mask
		{
		0x484600, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x1, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_24_10bit_4x2_4K2K60_LVDS_300to350MHz_VER006    NO.95
		//Address,Value,Mask
		{
		0x484600, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x1, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_24_10bit_4x2_4K2K60_LVDS_300to300MHz_VER006    NO.96
		//Address,Value,Mask
		{
		0x484600, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x1, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_24_10bit_12x1_4K2K60_LVDS_300to600MHz_VER006    NO.97
		//Address,Value,Mask
		{
		0x484600, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x1, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_24_10bit_12x1_4K2K60_LVDS_300to300MHz_VER006    NO.98
		//Address,Value,Mask
		{
		0x484600, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x1, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_24_10bit_12x1_4K1K144_LVDS_280to360MHz_VER006    NO.99
		//Address,Value,Mask
		{
		0x484600, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x1, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_24_10bit_12x1_4K1K144_LVDS_180to280MHz_VER006    NO.100
		//Address,Value,Mask
		{
		0x484600, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x1, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_24_10bit_12x1_4K1K144_LVDS_180to180MHz_VER006    NO.101
		//Address,Value,Mask
		{
		0x484600, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x1, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_CMPI_27_8bit_4x2_4K2K60_450to600MHz_VER006    NO.102
		//Address,Value,Mask
		{
		0x484600, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x2, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_CMPI_27_8bit_4x2_4K2K60_300to450MHz_VER006    NO.103
		//Address,Value,Mask
		{
		0x484600, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x2, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_CMPI_27_8bit_4x2_4K2K60_300to300MHz_VER006    NO.104
		//Address,Value,Mask
		{
		0x484600, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x2, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_CMPI_27_8bit_6x1_4K2K60_350to600MHz_VER006    NO.105
		//Address,Value,Mask
		{
		0x484600, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x2, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_CMPI_27_8bit_6x1_4K2K60_300to350MHz_VER006    NO.106
		//Address,Value,Mask
		{
		0x484600, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x2, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_CMPI_27_8bit_6x1_4K2K60_300to300MHz_VER006    NO.107
		//Address,Value,Mask
		{
		0x484600, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x2, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_CMPI_27_8bit_8x1_4K2K60_450to600MHz_VER006    NO.108
		//Address,Value,Mask
		{
		0x484600, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x2, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_CMPI_27_8bit_8x1_4K2K60_300to450MHz_VER006    NO.109
		//Address,Value,Mask
		{
		0x484600, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x2, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_CMPI_27_8bit_8x1_4K2K60_300to300MHz_VER006    NO.110
		//Address,Value,Mask
		{
		0x484600, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x2, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_CMPI_27_8bit_12x1_4K2K60_300to600MHz_VER006    NO.111
		//Address,Value,Mask
		{
		0x484600, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x2, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_CMPI_27_8bit_12x1_4K2K60_300to300MHz_VER006    NO.112
		//Address,Value,Mask
		{
		0x484600, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x2, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_CMPI_27_8bit_12x1_4K1K144_180to360MHz_VER006    NO.113
		//Address,Value,Mask
		{
		0x484600, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x2, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_CMPI_27_8bit_12x1_4K1K144_180to180MHz_VER006    NO.114
		//Address,Value,Mask
		{
		0x484600, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x2, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_CMPI_24_10bit_4x2_4K2K60_350to600MHz_VER006    NO.115
		//Address,Value,Mask
		{
		0x484600, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x2, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_CMPI_24_10bit_4x2_4K2K60_300to350MHz_VER006    NO.116
		//Address,Value,Mask
		{
		0x484600, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x2, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_CMPI_24_10bit_4x2_4K2K60_300to300MHz_VER006    NO.117
		//Address,Value,Mask
		{
		0x484600, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x2, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_CMPI_24_10bit_8x1_4K2K60_350to600MHz_VER006    NO.118
		//Address,Value,Mask
		{
		0x484600, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x2, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_CMPI_24_10bit_8x1_4K2K60_300to350MHz_VER006    NO.119
		//Address,Value,Mask
		{
		0x484600, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x2, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_CMPI_24_10bit_8x1_4K2K60_300to300MHz_VER006    NO.120
		//Address,Value,Mask
		{
		0x484600, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x2, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_CMPI_24_10bit_12x1_4K2K60_300to600MHz_VER006    NO.121
		//Address,Value,Mask
		{
		0x484600, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x2, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_CMPI_24_10bit_12x1_4K2K60_300to300MHz_VER006    NO.122
		//Address,Value,Mask
		{
		0x484600, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x2, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_CMPI_24_10bit_12x1_4K1K144_280to360MHz_VER006    NO.123
		//Address,Value,Mask
		{
		0x484600, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x2, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_CMPI_24_10bit_12x1_4K1K144_180to280MHz_VER006    NO.124
		//Address,Value,Mask
		{
		0x484600, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x2, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_CMPI_24_10bit_12x1_4K1K144_180to180MHz_VER006    NO.125
		//Address,Value,Mask
		{
		0x484600, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x2, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_ISP_8bit_4x2_4K2K60_450to600MHz_VER006    NO.126
		//Address,Value,Mask
		{
		0x484600, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x3, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_ISP_8bit_4x2_4K2K60_300to450MHz_VER006    NO.127
		//Address,Value,Mask
		{
		0x484600, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x3, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_ISP_8bit_4x2_4K2K60_300to300MHz_VER006    NO.128
		//Address,Value,Mask
		{
		0x484600, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x3, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_ISP_8bit_6x1_4K2K60_350to600MHz_VER006    NO.129
		//Address,Value,Mask
		{
		0x484600, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x3, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_ISP_8bit_6x1_4K2K60_300to350MHz_VER006    NO.130
		//Address,Value,Mask
		{
		0x484600, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x3, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_ISP_8bit_6x1_4K2K60_300to300MHz_VER006    NO.131
		//Address,Value,Mask
		{
		0x484600, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x3, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_ISP_8bit_6x2_4K2K60_300to600MHz_VER006    NO.132
		//Address,Value,Mask
		{
		0x484600, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x3, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_ISP_8bit_6x2_4K2K60_300to300MHz_VER006    NO.133
		//Address,Value,Mask
		{
		0x484600, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x3, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_ISP_8bit_8x1_4K2K60_450to600MHz_VER006    NO.134
		//Address,Value,Mask
		{
		0x484600, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x3, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_ISP_8bit_8x1_4K2K60_300to450MHz_VER006    NO.135
		//Address,Value,Mask
		{
		0x484600, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x3, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_ISP_8bit_8x1_4K2K60_300to300MHz_VER006    NO.136
		//Address,Value,Mask
		{
		0x484600, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x3, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_ISP_8bit_12x1_4K2K60_300to600MHz_VER006    NO.137
		//Address,Value,Mask
		{
		0x484600, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x3, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_ISP_8bit_12x1_4K2K60_300to300MHz_VER006    NO.138
		//Address,Value,Mask
		{
		0x484600, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x3, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_ISP_8bit_12x1_4K1K144_180to360MHz_VER006    NO.139
		//Address,Value,Mask
		{
		0x484600, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x3, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_ISP_8bit_12x1_4K1K144_180to180MHz_VER006    NO.140
		//Address,Value,Mask
		{
		0x484600, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x3, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_ISP_10bit_4x2_4K2K60_400to600MHz_VER006    NO.141
		//Address,Value,Mask
		{
		0x484600, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x3, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_ISP_10bit_4x2_4K2K60_300to400MHz_VER006    NO.142
		//Address,Value,Mask
		{
		0x484600, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x3, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_ISP_10bit_4x2_4K2K60_300to300MHz_VER006    NO.143
		//Address,Value,Mask
		{
		0x484600, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x3, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_ISP_10bit_6x2_4K2K60_300to600MHz_VER006    NO.144
		//Address,Value,Mask
		{
		0x484600, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x3, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_ISP_10bit_6x2_4K2K60_300to300MHz_VER006    NO.145
		//Address,Value,Mask
		{
		0x484600, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x3, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_ISP_10bit_8x1_4K2K60_400to600MHz_VER006    NO.146
		//Address,Value,Mask
		{
		0x484600, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x3, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_ISP_10bit_8x1_4K2K60_300to400MHz_VER006    NO.147
		//Address,Value,Mask
		{
		0x484600, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x3, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_ISP_10bit_8x1_4K2K60_300to300MHz_VER006    NO.148
		//Address,Value,Mask
		{
		0x484600, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x3, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_ISP_10bit_12x1_4K2K60_300to600MHz_VER006    NO.149
		//Address,Value,Mask
		{
		0x484600, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x3, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_ISP_10bit_12x1_4K2K60_300to300MHz_VER006    NO.150
		//Address,Value,Mask
		{
		0x484600, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x3, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_ISP_10bit_12x1_4K1K144_300to360MHz_VER006    NO.151
		//Address,Value,Mask
		{
		0x484600, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x3, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_ISP_10bit_12x1_4K1K144_180to300MHz_VER006    NO.152
		//Address,Value,Mask
		{
		0x484600, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x3, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_ISP_10bit_12x1_4K1K144_180to180MHz_VER006    NO.153
		//Address,Value,Mask
		{
		0x484600, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x3, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_CHPI_8bit_4x2_4K2K60_450to600MHz_VER006    NO.154
		//Address,Value,Mask
		{
		0x484600, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x3, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x8000, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_CHPI_8bit_4x2_4K2K60_300to450MHz_VER006    NO.155
		//Address,Value,Mask
		{
		0x484600, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x3, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x8000, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_CHPI_8bit_4x2_4K2K60_300to300MHz_VER006    NO.156
		//Address,Value,Mask
		{
		0x484600, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x3, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x8000, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_CHPI_8bit_6x1_4K2K60_300to600MHz_VER006    NO.157
		//Address,Value,Mask
		{
		0x484600, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x3, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x8000, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_CHPI_8bit_6x1_4K2K60_300to300MHz_VER006    NO.158
		//Address,Value,Mask
		{
		0x484600, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x3, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x8000, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_CHPI_8bit_6x2_4K2K60_300to600MHz_VER006    NO.159
		//Address,Value,Mask
		{
		0x484600, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x3, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x8000, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_CHPI_8bit_6x2_4K2K60_300to300MHz_VER006    NO.160
		//Address,Value,Mask
		{
		0x484600, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x3, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x8000, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_CHPI_8bit_8x1_4K2K60_450to600MHz_VER006    NO.161
		//Address,Value,Mask
		{
		0x484600, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x3, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x8000, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_CHPI_8bit_8x1_4K2K60_300to450MHz_VER006    NO.162
		//Address,Value,Mask
		{
		0x484600, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x3, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x8000, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_CHPI_8bit_8x1_4K2K60_300to300MHz_VER006    NO.163
		//Address,Value,Mask
		{
		0x484600, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x3, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x8000, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_CHPI_8bit_12x1_4K2K60_300to600MHz_VER006    NO.164
		//Address,Value,Mask
		{
		0x484600, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x3, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x8000, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_CHPI_8bit_12x1_4K2K60_300to300MHz_VER006    NO.165
		//Address,Value,Mask
		{
		0x484600, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x3, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x8000, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_CHPI_8bit_12x1_4K1K144_180to360MHz_VER006    NO.166
		//Address,Value,Mask
		{
		0x484600, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x3, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x8000, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_CHPI_8bit_12x1_4K1K144_180to180MHz_VER006    NO.167
		//Address,Value,Mask
		{
		0x484600, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x3, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x8000, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_CHPI_10bit_4x2_4K2K60_350to600MHz_VER006    NO.168
		//Address,Value,Mask
		{
		0x484600, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x3, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x8000, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_CHPI_10bit_4x2_4K2K60_300to350MHz_VER006    NO.169
		//Address,Value,Mask
		{
		0x484600, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x3, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x8000, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_CHPI_10bit_4x2_4K2K60_300to300MHz_VER006    NO.170
		//Address,Value,Mask
		{
		0x484600, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x3, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x8000, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_CHPI_10bit_8x1_4K2K60_350to600MHz_VER006    NO.171
		//Address,Value,Mask
		{
		0x484600, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x3, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x8000, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_CHPI_10bit_8x1_4K2K60_300to350MHz_VER006    NO.172
		//Address,Value,Mask
		{
		0x484600, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x3, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x8000, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_CHPI_10bit_8x1_4K2K60_300to300MHz_VER006    NO.173
		//Address,Value,Mask
		{
		0x484600, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x3, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x8000, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_CHPI_10bit_12x1_4K2K60_500to600MHz_VER006    NO.174
		//Address,Value,Mask
		{
		0x484600, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x3, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x8000, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_CHPI_10bit_12x1_4K2K60_300to500MHz_VER006    NO.175
		//Address,Value,Mask
		{
		0x484600, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x3, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x8000, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_CHPI_10bit_12x1_4K2K60_300to300MHz_VER006    NO.176
		//Address,Value,Mask
		{
		0x484600, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x3, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x8000, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_CHPI_10bit_12x1_4K1K144_280to360MHz_VER006    NO.177
		//Address,Value,Mask
		{
		0x484600, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x3, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x8000, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_CHPI_10bit_12x1_4K1K144_180to280MHz_VER006    NO.178
		//Address,Value,Mask
		{
		0x484600, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x3, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x8000, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_CHPI_10bit_12x1_4K1K144_180to180MHz_VER006    NO.179
		//Address,Value,Mask
		{
		0x484600, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x3, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x1, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x8000, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_Vx1_10bit_2_lane_2K60_150to300MHz_VER006    NO.180
		//Address,Value,Mask
		{
		0x484600, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x0, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x3FFF, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x0, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_Vx1_10bit_2_lane_2K60_150to150MHz_VER006    NO.181
		//Address,Value,Mask
		{
		0x484600, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x0, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x3FFF, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x0, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_Vx1_10bit_4_lane_4K2K30_300to600MHz_VER006    NO.182
		//Address,Value,Mask
		{
		0x484600, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x0, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x3FFF, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x0, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_Vx1_10bit_4_lane_4K2K30_300to300MHz_VER006    NO.183
		//Address,Value,Mask
		{
		0x484600, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x0, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x3FFF, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x0, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_Vx1_10bit_8_lane_4K2K60_300to600MHz_VER006    NO.184
		//Address,Value,Mask
		{
		0x484600, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x0, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x3FFF, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x0, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_Vx1_10bit_8_lane_4K2K60_300to300MHz_VER006    NO.185
		//Address,Value,Mask
		{
		0x484600, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x0, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x3FFF, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x0, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_Vx1_10bit_8_lane_4K1K144_180to360MHz_VER006    NO.186
		//Address,Value,Mask
		{
		0x484600, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x0, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x3FFF, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x0, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_Vx1_10bit_8_lane_4K1K144_180to180MHz_VER006    NO.187
		//Address,Value,Mask
		{
		0x484600, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x0, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x3FFF, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x0, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_Vx1_8bit_2_lane_2K60_225to300MHz_VER006    NO.188
		//Address,Value,Mask
		{
		0x484600, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x0, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x3FFF, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x0, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_Vx1_8bit_2_lane_2K60_150to225MHz_VER006    NO.189
		//Address,Value,Mask
		{
		0x484600, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x0, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x3FFF, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x0, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_Vx1_8bit_2_lane_2K60_150to150MHz_VER006    NO.190
		//Address,Value,Mask
		{
		0x484600, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x0, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x3FFF, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x0, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_Vx1_8bit_4_lane_4K2K30_450to600MHz_VER006    NO.191
		//Address,Value,Mask
		{
		0x484600, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x0, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x3FFF, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x0, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_Vx1_8bit_4_lane_4K2K30_300to450MHz_VER006    NO.192
		//Address,Value,Mask
		{
		0x484600, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x0, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x3FFF, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x0, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_Vx1_8bit_4_lane_4K2K30_300to300MHz_VER006    NO.193
		//Address,Value,Mask
		{
		0x484600, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x0, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x3FFF, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x0, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_Vx1_8bit_8_lane_4K2K60_450to600MHz_VER006    NO.194
		//Address,Value,Mask
		{
		0x484600, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x0, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x3FFF, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x0, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_Vx1_8bit_8_lane_4K2K60_300to450MHz_VER006    NO.195
		//Address,Value,Mask
		{
		0x484600, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x0, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x3FFF, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x0, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_Vx1_8bit_8_lane_4K2K60_300to300MHz_VER006    NO.196
		//Address,Value,Mask
		{
		0x484600, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x0, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x3FFF, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x0, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_Vx1_8bit_8_lane_4K1K144_225to360MHz_VER006    NO.197
		//Address,Value,Mask
		{
		0x484600, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x0, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x3FFF, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x0, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_Vx1_8bit_8_lane_4K1K144_180to225MHz_VER006    NO.198
		//Address,Value,Mask
		{
		0x484600, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x0, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x3FFF, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x0, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
	},   {		//E_PNL_SUPPORTED_LPLL_Vx1_8bit_8_lane_4K1K144_180to180MHz_VER006    NO.199
		//Address,Value,Mask
		{
		0x484600, 0x0, Fld(1, 13, AC_MSKW10)},	//reg_gcr_en_cml
		{
		0x484600, 0x0, Fld(2, 14, AC_MSKW10)},	//reg_reg_vcm_sel
		{
		0x484620, 0x3FFF, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_vby1_ch_19_00
		{
		0x484604, 0x1, Fld(1, 8, AC_MSKW10)},	//reg_gcr_en_reg
		{
		0x484604, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_pd_mod
		{
		0x484760, 0x0, Fld(1, 15, AC_MSKW10)},	//reg_hw_rint_en
		{
		0x484618, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_gcr_en_rint_ch_19_00
		{
		0x484C38, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[15:0]
		{
		0x484C3C, 0x0, Fld(16, 0, AC_MSKW10)},	//reg_test_mod[31:16]
		{
		0x484760, 0x400, Fld(15, 0, AC_MSKW10)},	//reg_hw_rint_th
},  };

TBLStruct MODDSettingTBL_VER006[PNL_NUM_VER006][MODD_REG_NUM_VER006] =  {
	 {			//E_PNL_SUPPORTED_LPLL_LVDS_1ch_HD60_225to300MHz_VER006    NO.0
		//Address,Value,Mask
		{
		0x484304, 0x1, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_LVDS_1ch_HD60_150to225MHz_VER006    NO.1
		//Address,Value,Mask
		{
		0x484304, 0x1, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_LVDS_1ch_HD60_150to150MHz_VER006    NO.2
		//Address,Value,Mask
		{
		0x484304, 0x1, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_LVDS_2ch_2K60_225to300MHz_VER006    NO.3
		//Address,Value,Mask
		{
		0x484304, 0x1, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_LVDS_2ch_2K60_150to225MHz_VER006    NO.4
		//Address,Value,Mask
		{
		0x484304, 0x1, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_LVDS_2ch_2K60_150to150MHz_VER006    NO.5
		//Address,Value,Mask
		{
		0x484304, 0x1, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_HS_LVDS_1ch_2K60_225to300MHz_VER006    NO.6
		//Address,Value,Mask
		{
		0x484304, 0x1, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_HS_LVDS_1ch_2K60_150to225MHz_VER006    NO.7
		//Address,Value,Mask
		{
		0x484304, 0x1, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_HS_LVDS_1ch_2K60_150to150MHz_VER006    NO.8
		//Address,Value,Mask
		{
		0x484304, 0x1, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_HS_LVDS_2ch_4K2K30_225to300MHz_VER006    NO.9
		//Address,Value,Mask
		{
		0x484304, 0x1, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_HS_LVDS_2ch_4K2K30_150to225MHz_VER006    NO.10
		//Address,Value,Mask
		{
		0x484304, 0x1, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_HS_LVDS_2ch_4K2K30_150to150MHz_VER006    NO.11
		//Address,Value,Mask
		{
		0x484304, 0x1, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_MINILVDS_1bk_3pair_6bit_HD60_150to300MHz_VER006    NO.12
		//Address,Value,Mask
		{
		0x484304, 0x1, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_MINILVDS_1bk_3pair_6bit_HD60_150to150MHz_VER006    NO.13
		//Address,Value,Mask
		{
		0x484304, 0x1, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_MINILVDS_1bk_6pair_6bit_HD60_150to300MHz_VER006    NO.14
		//Address,Value,Mask
		{
		0x484304, 0x1, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_MINILVDS_1bk_6pair_6bit_HD60_150to150MHz_VER006    NO.15
		//Address,Value,Mask
		{
		0x484304, 0x1, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_MINILVDS_2bk_3pair_6bit_2K60_150to300MHz_VER006    NO.16
		//Address,Value,Mask
		{
		0x484304, 0x1, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_MINILVDS_2bk_3pair_6bit_2K60_150to150MHz_VER006    NO.17
		//Address,Value,Mask
		{
		0x484304, 0x1, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_MINILVDS_2bk_6pair_6bit_2K60_150to300MHz_VER006    NO.18
		//Address,Value,Mask
		{
		0x484304, 0x1, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_MINILVDS_2bk_6pair_6bit_2K60_150to150MHz_VER006    NO.19
		//Address,Value,Mask
		{
		0x484304, 0x1, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_MINILVDS_1bk_3pair_8bit_HD60_150to300MHz_VER006    NO.20
		//Address,Value,Mask
		{
		0x484304, 0x1, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_MINILVDS_1bk_3pair_8bit_HD60_150to150MHz_VER006    NO.21
		//Address,Value,Mask
		{
		0x484304, 0x1, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_MINILVDS_1bk_6pair_8bit_HD60_150to300MHz_VER006    NO.22
		//Address,Value,Mask
		{
		0x484304, 0x1, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_MINILVDS_1bk_6pair_8bit_HD60_150to150MHz_VER006    NO.23
		//Address,Value,Mask
		{
		0x484304, 0x1, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_MINILVDS_2bk_3pair_8bit_2K60_150to300MHz_VER006    NO.24
		//Address,Value,Mask
		{
		0x484304, 0x1, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_MINILVDS_2bk_3pair_8bit_2K60_150to150MHz_VER006    NO.25
		//Address,Value,Mask
		{
		0x484304, 0x1, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_MINILVDS_2bk_6pair_8bit_2K60_150to300MHz_VER006    NO.26
		//Address,Value,Mask
		{
		0x484304, 0x1, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_MINILVDS_2bk_6pair_8bit_2K60_150to150MHz_VER006    NO.27
		//Address,Value,Mask
		{
		0x484304, 0x1, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_2pair_2K60_CML_225to300MHz_VER006    NO.28
		//Address,Value,Mask
		{
		0x484304, 0x1, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_2pair_2K60_CML_150to225MHz_VER006    NO.29
		//Address,Value,Mask
		{
		0x484304, 0x1, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_2pair_2K60_CML_150to150MHz_VER006    NO.30
		//Address,Value,Mask
		{
		0x484304, 0x1, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_4pair_2K60_CML_225to300MHz_VER006    NO.31
		//Address,Value,Mask
		{
		0x484304, 0x1, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_4pair_2K60_CML_150to225MHz_VER006    NO.32
		//Address,Value,Mask
		{
		0x484304, 0x1, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_4pair_2K60_CML_150to150MHz_VER006    NO.33
		//Address,Value,Mask
		{
		0x484304, 0x1, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_6pair_2K120_CML_300to600MHz_VER006    NO.34
		//Address,Value,Mask
		{
		0x484304, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_6pair_2K120_CML_300to300MHz_VER006    NO.35
		//Address,Value,Mask
		{
		0x484304, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_8pair_2K120_CML_450to600MHz_VER006    NO.36
		//Address,Value,Mask
		{
		0x484304, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_8pair_2K120_CML_300to450MHz_VER006    NO.37
		//Address,Value,Mask
		{
		0x484304, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_8pair_2K120_CML_300to300MHz_VER006    NO.38
		//Address,Value,Mask
		{
		0x484304, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_4x2_4K2K60_CML_450to600MHz_VER006    NO.39
		//Address,Value,Mask
		{
		0x484304, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_4x2_4K2K60_CML_300to450MHz_VER006    NO.40
		//Address,Value,Mask
		{
		0x484304, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_4x2_4K2K60_CML_300to300MHz_VER006    NO.41
		//Address,Value,Mask
		{
		0x484304, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_6x1_4K2K60_CML_350to600MHz_VER006    NO.42
		//Address,Value,Mask
		{
		0x484304, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_6x1_4K2K60_CML_300to350MHz_VER006    NO.43
		//Address,Value,Mask
		{
		0x484304, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_6x1_4K2K60_CML_300to300MHz_VER006    NO.44
		//Address,Value,Mask
		{
		0x484304, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_8x1_4K2K60_CML_450to600MHz_VER006    NO.45
		//Address,Value,Mask
		{
		0x484304, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_8x1_4K2K60_CML_300to450MHz_VER006    NO.46
		//Address,Value,Mask
		{
		0x484304, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_8x1_4K2K60_CML_300to300MHz_VER006    NO.47
		//Address,Value,Mask
		{
		0x484304, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_12x1_4K2K60_CML_300to600MHz_VER006    NO.48
		//Address,Value,Mask
		{
		0x484304, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_12x1_4K2K60_CML_300to300MHz_VER006    NO.49
		//Address,Value,Mask
		{
		0x484304, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_12x1_4K1K144_CML_180to360MHz_VER006    NO.50
		//Address,Value,Mask
		{
		0x484304, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_12x1_4K1K144_CML_180to180MHz_VER006    NO.51
		//Address,Value,Mask
		{
		0x484304, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_16x1_4K2K60_CML_450to600MHz_VER006    NO.52
		//Address,Value,Mask
		{
		0x484304, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_16x1_4K2K60_CML_300to450MHz_VER006    NO.53
		//Address,Value,Mask
		{
		0x484304, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_16x1_4K2K60_CML_300to300MHz_VER006    NO.54
		//Address,Value,Mask
		{
		0x484304, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_24_10bit_16x1_4K2K60_CML_300to600MHz_VER006    NO.55
		//Address,Value,Mask
		{
		0x484304, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_24_10bit_16x1_4K2K60_CML_300to300MHz_VER006    NO.56
		//Address,Value,Mask
		{
		0x484304, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_24_10bit_4x2_4K2K60_CML_350to600MHz_VER006    NO.57
		//Address,Value,Mask
		{
		0x484304, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_24_10bit_4x2_4K2K60_CML_300to350MHz_VER006    NO.58
		//Address,Value,Mask
		{
		0x484304, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_24_10bit_4x2_4K2K60_CML_300to300MHz_VER006    NO.59
		//Address,Value,Mask
		{
		0x484304, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_24_10bit_12x1_4K2K60_CML_300to600MHz_VER006    NO.60
		//Address,Value,Mask
		{
		0x484304, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_24_10bit_12x1_4K2K60_CML_300to300MHz_VER006    NO.61
		//Address,Value,Mask
		{
		0x484304, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_24_10bit_12x1_4K1K144_CML_280to360MHz_VER006    NO.62
		//Address,Value,Mask
		{
		0x484304, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_24_10bit_12x1_4K1K144_CML_180to280MHz_VER006    NO.63
		//Address,Value,Mask
		{
		0x484304, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_24_10bit_12x1_4K1K144_CML_180to180MHz_VER006    NO.64
		//Address,Value,Mask
		{
		0x484304, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_2pair_2K60_LVDS_225to300MHz_VER006    NO.65
		//Address,Value,Mask
		{
		0x484304, 0x1, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_2pair_2K60_LVDS_150to225MHz_VER006    NO.66
		//Address,Value,Mask
		{
		0x484304, 0x1, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_2pair_2K60_LVDS_150to150MHz_VER006    NO.67
		//Address,Value,Mask
		{
		0x484304, 0x1, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_4pair_2K60_LVDS_225to300MHz_VER006    NO.68
		//Address,Value,Mask
		{
		0x484304, 0x1, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_4pair_2K60_LVDS_150to225MHz_VER006    NO.69
		//Address,Value,Mask
		{
		0x484304, 0x1, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_4pair_2K60_LVDS_150to150MHz_VER006    NO.70
		//Address,Value,Mask
		{
		0x484304, 0x1, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_6pair_2K120_LVDS_300to600MHz_VER006    NO.71
		//Address,Value,Mask
		{
		0x484304, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_6pair_2K120_LVDS_300to300MHz_VER006    NO.72
		//Address,Value,Mask
		{
		0x484304, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_8pair_2K120_LVDS_450to600MHz_VER006    NO.73
		//Address,Value,Mask
		{
		0x484304, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_8pair_2K120_LVDS_300to450MHz_VER006    NO.74
		//Address,Value,Mask
		{
		0x484304, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_8pair_2K120_LVDS_300to300MHz_VER006    NO.75
		//Address,Value,Mask
		{
		0x484304, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_4x2_4K2K60_LVDS_450to600MHz_VER006    NO.76
		//Address,Value,Mask
		{
		0x484304, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_4x2_4K2K60_LVDS_300to450MHz_VER006    NO.77
		//Address,Value,Mask
		{
		0x484304, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_4x2_4K2K60_LVDS_300to300MHz_VER006    NO.78
		//Address,Value,Mask
		{
		0x484304, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_6x1_4K2K60_LVDS_350to600MHz_VER006    NO.79
		//Address,Value,Mask
		{
		0x484304, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_6x1_4K2K60_LVDS_300to350MHz_VER006    NO.80
		//Address,Value,Mask
		{
		0x484304, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_6x1_4K2K60_LVDS_300to300MHz_VER006    NO.81
		//Address,Value,Mask
		{
		0x484304, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_8x1_4K2K60_LVDS_450to600MHz_VER006    NO.82
		//Address,Value,Mask
		{
		0x484304, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_8x1_4K2K60_LVDS_300to450MHz_VER006    NO.83
		//Address,Value,Mask
		{
		0x484304, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_8x1_4K2K60_LVDS_300to300MHz_VER006    NO.84
		//Address,Value,Mask
		{
		0x484304, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_12x1_4K2K60_LVDS_300to600MHz_VER006    NO.85
		//Address,Value,Mask
		{
		0x484304, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_12x1_4K2K60_LVDS_300to300MHz_VER006    NO.86
		//Address,Value,Mask
		{
		0x484304, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_12x1_4K1K144_LVDS_180to360MHz_VER006    NO.87
		//Address,Value,Mask
		{
		0x484304, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_12x1_4K1K144_LVDS_180to180MHz_VER006    NO.88
		//Address,Value,Mask
		{
		0x484304, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_16x1_4K_450to600MHz_VER006    NO.89
		//Address,Value,Mask
		{
		0x484304, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_16x1_4K_300to450MHz_VER006    NO.90
		//Address,Value,Mask
		{
		0x484304, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_16x1_4K_300to300MHz_VER006    NO.91
		//Address,Value,Mask
		{
		0x484304, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_24_10bit_16x1_4K2K60_LVDS_300to600MHz_VER006    NO.92
		//Address,Value,Mask
		{
		0x484304, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_24_10bit_16x1_4K2K60_LVDS_300to300MHz_VER006    NO.93
		//Address,Value,Mask
		{
		0x484304, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_24_10bit_4x2_4K2K60_LVDS_350to600MHz_VER006    NO.94
		//Address,Value,Mask
		{
		0x484304, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_24_10bit_4x2_4K2K60_LVDS_300to350MHz_VER006    NO.95
		//Address,Value,Mask
		{
		0x484304, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_24_10bit_4x2_4K2K60_LVDS_300to300MHz_VER006    NO.96
		//Address,Value,Mask
		{
		0x484304, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_24_10bit_12x1_4K2K60_LVDS_300to600MHz_VER006    NO.97
		//Address,Value,Mask
		{
		0x484304, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_24_10bit_12x1_4K2K60_LVDS_300to300MHz_VER006    NO.98
		//Address,Value,Mask
		{
		0x484304, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_24_10bit_12x1_4K1K144_LVDS_280to360MHz_VER006    NO.99
		//Address,Value,Mask
		{
		0x484304, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_24_10bit_12x1_4K1K144_LVDS_180to280MHz_VER006    NO.100
		//Address,Value,Mask
		{
		0x484304, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_24_10bit_12x1_4K1K144_LVDS_180to180MHz_VER006    NO.101
		//Address,Value,Mask
		{
		0x484304, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_CMPI_27_8bit_4x2_4K2K60_450to600MHz_VER006    NO.102
		//Address,Value,Mask
		{
		0x484304, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_CMPI_27_8bit_4x2_4K2K60_300to450MHz_VER006    NO.103
		//Address,Value,Mask
		{
		0x484304, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_CMPI_27_8bit_4x2_4K2K60_300to300MHz_VER006    NO.104
		//Address,Value,Mask
		{
		0x484304, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_CMPI_27_8bit_6x1_4K2K60_350to600MHz_VER006    NO.105
		//Address,Value,Mask
		{
		0x484304, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_CMPI_27_8bit_6x1_4K2K60_300to350MHz_VER006    NO.106
		//Address,Value,Mask
		{
		0x484304, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_CMPI_27_8bit_6x1_4K2K60_300to300MHz_VER006    NO.107
		//Address,Value,Mask
		{
		0x484304, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_CMPI_27_8bit_8x1_4K2K60_450to600MHz_VER006    NO.108
		//Address,Value,Mask
		{
		0x484304, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_CMPI_27_8bit_8x1_4K2K60_300to450MHz_VER006    NO.109
		//Address,Value,Mask
		{
		0x484304, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_CMPI_27_8bit_8x1_4K2K60_300to300MHz_VER006    NO.110
		//Address,Value,Mask
		{
		0x484304, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_CMPI_27_8bit_12x1_4K2K60_300to600MHz_VER006    NO.111
		//Address,Value,Mask
		{
		0x484304, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_CMPI_27_8bit_12x1_4K2K60_300to300MHz_VER006    NO.112
		//Address,Value,Mask
		{
		0x484304, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_CMPI_27_8bit_12x1_4K1K144_180to360MHz_VER006    NO.113
		//Address,Value,Mask
		{
		0x484304, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_CMPI_27_8bit_12x1_4K1K144_180to180MHz_VER006    NO.114
		//Address,Value,Mask
		{
		0x484304, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_CMPI_24_10bit_4x2_4K2K60_350to600MHz_VER006    NO.115
		//Address,Value,Mask
		{
		0x484304, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_CMPI_24_10bit_4x2_4K2K60_300to350MHz_VER006    NO.116
		//Address,Value,Mask
		{
		0x484304, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_CMPI_24_10bit_4x2_4K2K60_300to300MHz_VER006    NO.117
		//Address,Value,Mask
		{
		0x484304, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_CMPI_24_10bit_8x1_4K2K60_350to600MHz_VER006    NO.118
		//Address,Value,Mask
		{
		0x484304, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_CMPI_24_10bit_8x1_4K2K60_300to350MHz_VER006    NO.119
		//Address,Value,Mask
		{
		0x484304, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_CMPI_24_10bit_8x1_4K2K60_300to300MHz_VER006    NO.120
		//Address,Value,Mask
		{
		0x484304, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_CMPI_24_10bit_12x1_4K2K60_300to600MHz_VER006    NO.121
		//Address,Value,Mask
		{
		0x484304, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_CMPI_24_10bit_12x1_4K2K60_300to300MHz_VER006    NO.122
		//Address,Value,Mask
		{
		0x484304, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_CMPI_24_10bit_12x1_4K1K144_280to360MHz_VER006    NO.123
		//Address,Value,Mask
		{
		0x484304, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_CMPI_24_10bit_12x1_4K1K144_180to280MHz_VER006    NO.124
		//Address,Value,Mask
		{
		0x484304, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_CMPI_24_10bit_12x1_4K1K144_180to180MHz_VER006    NO.125
		//Address,Value,Mask
		{
		0x484304, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_ISP_8bit_4x2_4K2K60_450to600MHz_VER006    NO.126
		//Address,Value,Mask
		{
		0x484304, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_ISP_8bit_4x2_4K2K60_300to450MHz_VER006    NO.127
		//Address,Value,Mask
		{
		0x484304, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_ISP_8bit_4x2_4K2K60_300to300MHz_VER006    NO.128
		//Address,Value,Mask
		{
		0x484304, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_ISP_8bit_6x1_4K2K60_350to600MHz_VER006    NO.129
		//Address,Value,Mask
		{
		0x484304, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_ISP_8bit_6x1_4K2K60_300to350MHz_VER006    NO.130
		//Address,Value,Mask
		{
		0x484304, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_ISP_8bit_6x1_4K2K60_300to300MHz_VER006    NO.131
		//Address,Value,Mask
		{
		0x484304, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_ISP_8bit_6x2_4K2K60_300to600MHz_VER006    NO.132
		//Address,Value,Mask
		{
		0x484304, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_ISP_8bit_6x2_4K2K60_300to300MHz_VER006    NO.133
		//Address,Value,Mask
		{
		0x484304, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_ISP_8bit_8x1_4K2K60_450to600MHz_VER006    NO.134
		//Address,Value,Mask
		{
		0x484304, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_ISP_8bit_8x1_4K2K60_300to450MHz_VER006    NO.135
		//Address,Value,Mask
		{
		0x484304, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_ISP_8bit_8x1_4K2K60_300to300MHz_VER006    NO.136
		//Address,Value,Mask
		{
		0x484304, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_ISP_8bit_12x1_4K2K60_300to600MHz_VER006    NO.137
		//Address,Value,Mask
		{
		0x484304, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_ISP_8bit_12x1_4K2K60_300to300MHz_VER006    NO.138
		//Address,Value,Mask
		{
		0x484304, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_ISP_8bit_12x1_4K1K144_180to360MHz_VER006    NO.139
		//Address,Value,Mask
		{
		0x484304, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_ISP_8bit_12x1_4K1K144_180to180MHz_VER006    NO.140
		//Address,Value,Mask
		{
		0x484304, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_ISP_10bit_4x2_4K2K60_400to600MHz_VER006    NO.141
		//Address,Value,Mask
		{
		0x484304, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_ISP_10bit_4x2_4K2K60_300to400MHz_VER006    NO.142
		//Address,Value,Mask
		{
		0x484304, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_ISP_10bit_4x2_4K2K60_300to300MHz_VER006    NO.143
		//Address,Value,Mask
		{
		0x484304, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_ISP_10bit_6x2_4K2K60_300to600MHz_VER006    NO.144
		//Address,Value,Mask
		{
		0x484304, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_ISP_10bit_6x2_4K2K60_300to300MHz_VER006    NO.145
		//Address,Value,Mask
		{
		0x484304, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_ISP_10bit_8x1_4K2K60_400to600MHz_VER006    NO.146
		//Address,Value,Mask
		{
		0x484304, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_ISP_10bit_8x1_4K2K60_300to400MHz_VER006    NO.147
		//Address,Value,Mask
		{
		0x484304, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_ISP_10bit_8x1_4K2K60_300to300MHz_VER006    NO.148
		//Address,Value,Mask
		{
		0x484304, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_ISP_10bit_12x1_4K2K60_300to600MHz_VER006    NO.149
		//Address,Value,Mask
		{
		0x484304, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_ISP_10bit_12x1_4K2K60_300to300MHz_VER006    NO.150
		//Address,Value,Mask
		{
		0x484304, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_ISP_10bit_12x1_4K1K144_300to360MHz_VER006    NO.151
		//Address,Value,Mask
		{
		0x484304, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_ISP_10bit_12x1_4K1K144_180to300MHz_VER006    NO.152
		//Address,Value,Mask
		{
		0x484304, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_ISP_10bit_12x1_4K1K144_180to180MHz_VER006    NO.153
		//Address,Value,Mask
		{
		0x484304, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_CHPI_8bit_4x2_4K2K60_450to600MHz_VER006    NO.154
		//Address,Value,Mask
		{
		0x484304, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_CHPI_8bit_4x2_4K2K60_300to450MHz_VER006    NO.155
		//Address,Value,Mask
		{
		0x484304, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_CHPI_8bit_4x2_4K2K60_300to300MHz_VER006    NO.156
		//Address,Value,Mask
		{
		0x484304, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_CHPI_8bit_6x1_4K2K60_300to600MHz_VER006    NO.157
		//Address,Value,Mask
		{
		0x484304, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_CHPI_8bit_6x1_4K2K60_300to300MHz_VER006    NO.158
		//Address,Value,Mask
		{
		0x484304, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_CHPI_8bit_6x2_4K2K60_300to600MHz_VER006    NO.159
		//Address,Value,Mask
		{
		0x484304, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_CHPI_8bit_6x2_4K2K60_300to300MHz_VER006    NO.160
		//Address,Value,Mask
		{
		0x484304, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_CHPI_8bit_8x1_4K2K60_450to600MHz_VER006    NO.161
		//Address,Value,Mask
		{
		0x484304, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_CHPI_8bit_8x1_4K2K60_300to450MHz_VER006    NO.162
		//Address,Value,Mask
		{
		0x484304, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_CHPI_8bit_8x1_4K2K60_300to300MHz_VER006    NO.163
		//Address,Value,Mask
		{
		0x484304, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_CHPI_8bit_12x1_4K2K60_300to600MHz_VER006    NO.164
		//Address,Value,Mask
		{
		0x484304, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_CHPI_8bit_12x1_4K2K60_300to300MHz_VER006    NO.165
		//Address,Value,Mask
		{
		0x484304, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_CHPI_8bit_12x1_4K1K144_180to360MHz_VER006    NO.166
		//Address,Value,Mask
		{
		0x484304, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_CHPI_8bit_12x1_4K1K144_180to180MHz_VER006    NO.167
		//Address,Value,Mask
		{
		0x484304, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_CHPI_10bit_4x2_4K2K60_350to600MHz_VER006    NO.168
		//Address,Value,Mask
		{
		0x484304, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_CHPI_10bit_4x2_4K2K60_300to350MHz_VER006    NO.169
		//Address,Value,Mask
		{
		0x484304, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_CHPI_10bit_4x2_4K2K60_300to300MHz_VER006    NO.170
		//Address,Value,Mask
		{
		0x484304, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_CHPI_10bit_8x1_4K2K60_350to600MHz_VER006    NO.171
		//Address,Value,Mask
		{
		0x484304, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_CHPI_10bit_8x1_4K2K60_300to350MHz_VER006    NO.172
		//Address,Value,Mask
		{
		0x484304, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_CHPI_10bit_8x1_4K2K60_300to300MHz_VER006    NO.173
		//Address,Value,Mask
		{
		0x484304, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_CHPI_10bit_12x1_4K2K60_500to600MHz_VER006    NO.174
		//Address,Value,Mask
		{
		0x484304, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_CHPI_10bit_12x1_4K2K60_300to500MHz_VER006    NO.175
		//Address,Value,Mask
		{
		0x484304, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_CHPI_10bit_12x1_4K2K60_300to300MHz_VER006    NO.176
		//Address,Value,Mask
		{
		0x484304, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_CHPI_10bit_12x1_4K1K144_280to360MHz_VER006    NO.177
		//Address,Value,Mask
		{
		0x484304, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_CHPI_10bit_12x1_4K1K144_180to280MHz_VER006    NO.178
		//Address,Value,Mask
		{
		0x484304, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_CHPI_10bit_12x1_4K1K144_180to180MHz_VER006    NO.179
		//Address,Value,Mask
		{
		0x484304, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_Vx1_10bit_2_lane_2K60_150to300MHz_VER006    NO.180
		//Address,Value,Mask
		{
		0x484304, 0x1, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_Vx1_10bit_2_lane_2K60_150to150MHz_VER006    NO.181
		//Address,Value,Mask
		{
		0x484304, 0x1, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_Vx1_10bit_4_lane_4K2K30_300to600MHz_VER006    NO.182
		//Address,Value,Mask
		{
		0x484304, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_Vx1_10bit_4_lane_4K2K30_300to300MHz_VER006    NO.183
		//Address,Value,Mask
		{
		0x484304, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_Vx1_10bit_8_lane_4K2K60_300to600MHz_VER006    NO.184
		//Address,Value,Mask
		{
		0x484304, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_Vx1_10bit_8_lane_4K2K60_300to300MHz_VER006    NO.185
		//Address,Value,Mask
		{
		0x484304, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_Vx1_10bit_8_lane_4K1K144_180to360MHz_VER006    NO.186
		//Address,Value,Mask
		{
		0x484304, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_Vx1_10bit_8_lane_4K1K144_180to180MHz_VER006    NO.187
		//Address,Value,Mask
		{
		0x484304, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_Vx1_8bit_2_lane_2K60_225to300MHz_VER006    NO.188
		//Address,Value,Mask
		{
		0x484304, 0x1, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_Vx1_8bit_2_lane_2K60_150to225MHz_VER006    NO.189
		//Address,Value,Mask
		{
		0x484304, 0x1, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_Vx1_8bit_2_lane_2K60_150to150MHz_VER006    NO.190
		//Address,Value,Mask
		{
		0x484304, 0x1, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_Vx1_8bit_4_lane_4K2K30_450to600MHz_VER006    NO.191
		//Address,Value,Mask
		{
		0x484304, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_Vx1_8bit_4_lane_4K2K30_300to450MHz_VER006    NO.192
		//Address,Value,Mask
		{
		0x484304, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_Vx1_8bit_4_lane_4K2K30_300to300MHz_VER006    NO.193
		//Address,Value,Mask
		{
		0x484304, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_Vx1_8bit_8_lane_4K2K60_450to600MHz_VER006    NO.194
		//Address,Value,Mask
		{
		0x484304, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_Vx1_8bit_8_lane_4K2K60_300to450MHz_VER006    NO.195
		//Address,Value,Mask
		{
		0x484304, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_Vx1_8bit_8_lane_4K2K60_300to300MHz_VER006    NO.196
		//Address,Value,Mask
		{
		0x484304, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_Vx1_8bit_8_lane_4K1K144_225to360MHz_VER006    NO.197
		//Address,Value,Mask
		{
		0x484304, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_Vx1_8bit_8_lane_4K1K144_180to225MHz_VER006    NO.198
		//Address,Value,Mask
		{
		0x484304, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
	},   {		//E_PNL_SUPPORTED_LPLL_Vx1_8bit_8_lane_4K1K144_180to180MHz_VER006    NO.199
		//Address,Value,Mask
		{
		0x484304, 0x0, Fld(1, 0, AC_MSKW10)},	//reg_clk_shift
},  };

TBLStruct MPLLSettingTBL_VER006[PNL_NUM_VER006][MPLL_REG_NUM_VER006] =  {
	 {			//E_PNL_SUPPORTED_LPLL_LVDS_1ch_HD60_225to300MHz_VER006    NO.0
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_LVDS_1ch_HD60_150to225MHz_VER006    NO.1
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_LVDS_1ch_HD60_150to150MHz_VER006    NO.2
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_LVDS_2ch_2K60_225to300MHz_VER006    NO.3
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_LVDS_2ch_2K60_150to225MHz_VER006    NO.4
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_LVDS_2ch_2K60_150to150MHz_VER006    NO.5
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_HS_LVDS_1ch_2K60_225to300MHz_VER006    NO.6
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_HS_LVDS_1ch_2K60_150to225MHz_VER006    NO.7
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_HS_LVDS_1ch_2K60_150to150MHz_VER006    NO.8
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_HS_LVDS_2ch_4K2K30_225to300MHz_VER006    NO.9
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_HS_LVDS_2ch_4K2K30_150to225MHz_VER006    NO.10
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_HS_LVDS_2ch_4K2K30_150to150MHz_VER006    NO.11
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_MINILVDS_1bk_3pair_6bit_HD60_150to300MHz_VER006    NO.12
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_MINILVDS_1bk_3pair_6bit_HD60_150to150MHz_VER006    NO.13
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_MINILVDS_1bk_6pair_6bit_HD60_150to300MHz_VER006    NO.14
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_MINILVDS_1bk_6pair_6bit_HD60_150to150MHz_VER006    NO.15
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_MINILVDS_2bk_3pair_6bit_2K60_150to300MHz_VER006    NO.16
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_MINILVDS_2bk_3pair_6bit_2K60_150to150MHz_VER006    NO.17
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_MINILVDS_2bk_6pair_6bit_2K60_150to300MHz_VER006    NO.18
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_MINILVDS_2bk_6pair_6bit_2K60_150to150MHz_VER006    NO.19
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_MINILVDS_1bk_3pair_8bit_HD60_150to300MHz_VER006    NO.20
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_MINILVDS_1bk_3pair_8bit_HD60_150to150MHz_VER006    NO.21
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_MINILVDS_1bk_6pair_8bit_HD60_150to300MHz_VER006    NO.22
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_MINILVDS_1bk_6pair_8bit_HD60_150to150MHz_VER006    NO.23
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_MINILVDS_2bk_3pair_8bit_2K60_150to300MHz_VER006    NO.24
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_MINILVDS_2bk_3pair_8bit_2K60_150to150MHz_VER006    NO.25
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_MINILVDS_2bk_6pair_8bit_2K60_150to300MHz_VER006    NO.26
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_MINILVDS_2bk_6pair_8bit_2K60_150to150MHz_VER006    NO.27
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_2pair_2K60_CML_225to300MHz_VER006    NO.28
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_2pair_2K60_CML_150to225MHz_VER006    NO.29
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_2pair_2K60_CML_150to150MHz_VER006    NO.30
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_4pair_2K60_CML_225to300MHz_VER006    NO.31
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_4pair_2K60_CML_150to225MHz_VER006    NO.32
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_4pair_2K60_CML_150to150MHz_VER006    NO.33
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_6pair_2K120_CML_300to600MHz_VER006    NO.34
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_6pair_2K120_CML_300to300MHz_VER006    NO.35
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_8pair_2K120_CML_450to600MHz_VER006    NO.36
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_8pair_2K120_CML_300to450MHz_VER006    NO.37
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_8pair_2K120_CML_300to300MHz_VER006    NO.38
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_4x2_4K2K60_CML_450to600MHz_VER006    NO.39
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_4x2_4K2K60_CML_300to450MHz_VER006    NO.40
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_4x2_4K2K60_CML_300to300MHz_VER006    NO.41
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_6x1_4K2K60_CML_350to600MHz_VER006    NO.42
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_6x1_4K2K60_CML_300to350MHz_VER006    NO.43
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_6x1_4K2K60_CML_300to300MHz_VER006    NO.44
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_8x1_4K2K60_CML_450to600MHz_VER006    NO.45
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_8x1_4K2K60_CML_300to450MHz_VER006    NO.46
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_8x1_4K2K60_CML_300to300MHz_VER006    NO.47
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_12x1_4K2K60_CML_300to600MHz_VER006    NO.48
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_12x1_4K2K60_CML_300to300MHz_VER006    NO.49
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_12x1_4K1K144_CML_180to360MHz_VER006    NO.50
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_12x1_4K1K144_CML_180to180MHz_VER006    NO.51
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_16x1_4K2K60_CML_450to600MHz_VER006    NO.52
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_16x1_4K2K60_CML_300to450MHz_VER006    NO.53
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_16x1_4K2K60_CML_300to300MHz_VER006    NO.54
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_24_10bit_16x1_4K2K60_CML_300to600MHz_VER006    NO.55
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_24_10bit_16x1_4K2K60_CML_300to300MHz_VER006    NO.56
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_24_10bit_4x2_4K2K60_CML_350to600MHz_VER006    NO.57
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_24_10bit_4x2_4K2K60_CML_300to350MHz_VER006    NO.58
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_24_10bit_4x2_4K2K60_CML_300to300MHz_VER006    NO.59
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_24_10bit_12x1_4K2K60_CML_300to600MHz_VER006    NO.60
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_24_10bit_12x1_4K2K60_CML_300to300MHz_VER006    NO.61
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_24_10bit_12x1_4K1K144_CML_280to360MHz_VER006    NO.62
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_24_10bit_12x1_4K1K144_CML_180to280MHz_VER006    NO.63
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_24_10bit_12x1_4K1K144_CML_180to180MHz_VER006    NO.64
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_2pair_2K60_LVDS_225to300MHz_VER006    NO.65
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_2pair_2K60_LVDS_150to225MHz_VER006    NO.66
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_2pair_2K60_LVDS_150to150MHz_VER006    NO.67
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_4pair_2K60_LVDS_225to300MHz_VER006    NO.68
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_4pair_2K60_LVDS_150to225MHz_VER006    NO.69
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_4pair_2K60_LVDS_150to150MHz_VER006    NO.70
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_6pair_2K120_LVDS_300to600MHz_VER006    NO.71
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_6pair_2K120_LVDS_300to300MHz_VER006    NO.72
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_8pair_2K120_LVDS_450to600MHz_VER006    NO.73
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_8pair_2K120_LVDS_300to450MHz_VER006    NO.74
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_8pair_2K120_LVDS_300to300MHz_VER006    NO.75
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_4x2_4K2K60_LVDS_450to600MHz_VER006    NO.76
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_4x2_4K2K60_LVDS_300to450MHz_VER006    NO.77
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_4x2_4K2K60_LVDS_300to300MHz_VER006    NO.78
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_6x1_4K2K60_LVDS_350to600MHz_VER006    NO.79
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_6x1_4K2K60_LVDS_300to350MHz_VER006    NO.80
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_6x1_4K2K60_LVDS_300to300MHz_VER006    NO.81
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_8x1_4K2K60_LVDS_450to600MHz_VER006    NO.82
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_8x1_4K2K60_LVDS_300to450MHz_VER006    NO.83
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_8x1_4K2K60_LVDS_300to300MHz_VER006    NO.84
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_12x1_4K2K60_LVDS_300to600MHz_VER006    NO.85
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_12x1_4K2K60_LVDS_300to300MHz_VER006    NO.86
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_12x1_4K1K144_LVDS_180to360MHz_VER006    NO.87
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_12x1_4K1K144_LVDS_180to180MHz_VER006    NO.88
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_16x1_4K_450to600MHz_VER006    NO.89
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_16x1_4K_300to450MHz_VER006    NO.90
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_16x1_4K_300to300MHz_VER006    NO.91
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_24_10bit_16x1_4K2K60_LVDS_300to600MHz_VER006    NO.92
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_24_10bit_16x1_4K2K60_LVDS_300to300MHz_VER006    NO.93
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_24_10bit_4x2_4K2K60_LVDS_350to600MHz_VER006    NO.94
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_24_10bit_4x2_4K2K60_LVDS_300to350MHz_VER006    NO.95
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_24_10bit_4x2_4K2K60_LVDS_300to300MHz_VER006    NO.96
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_24_10bit_12x1_4K2K60_LVDS_300to600MHz_VER006    NO.97
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_24_10bit_12x1_4K2K60_LVDS_300to300MHz_VER006    NO.98
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_24_10bit_12x1_4K1K144_LVDS_280to360MHz_VER006    NO.99
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_24_10bit_12x1_4K1K144_LVDS_180to280MHz_VER006    NO.100
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_EPI_24_10bit_12x1_4K1K144_LVDS_180to180MHz_VER006    NO.101
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_CMPI_27_8bit_4x2_4K2K60_450to600MHz_VER006    NO.102
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_CMPI_27_8bit_4x2_4K2K60_300to450MHz_VER006    NO.103
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_CMPI_27_8bit_4x2_4K2K60_300to300MHz_VER006    NO.104
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_CMPI_27_8bit_6x1_4K2K60_350to600MHz_VER006    NO.105
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_CMPI_27_8bit_6x1_4K2K60_300to350MHz_VER006    NO.106
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_CMPI_27_8bit_6x1_4K2K60_300to300MHz_VER006    NO.107
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_CMPI_27_8bit_8x1_4K2K60_450to600MHz_VER006    NO.108
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_CMPI_27_8bit_8x1_4K2K60_300to450MHz_VER006    NO.109
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_CMPI_27_8bit_8x1_4K2K60_300to300MHz_VER006    NO.110
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_CMPI_27_8bit_12x1_4K2K60_300to600MHz_VER006    NO.111
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_CMPI_27_8bit_12x1_4K2K60_300to300MHz_VER006    NO.112
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_CMPI_27_8bit_12x1_4K1K144_180to360MHz_VER006    NO.113
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_CMPI_27_8bit_12x1_4K1K144_180to180MHz_VER006    NO.114
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_CMPI_24_10bit_4x2_4K2K60_350to600MHz_VER006    NO.115
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_CMPI_24_10bit_4x2_4K2K60_300to350MHz_VER006    NO.116
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_CMPI_24_10bit_4x2_4K2K60_300to300MHz_VER006    NO.117
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_CMPI_24_10bit_8x1_4K2K60_350to600MHz_VER006    NO.118
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_CMPI_24_10bit_8x1_4K2K60_300to350MHz_VER006    NO.119
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_CMPI_24_10bit_8x1_4K2K60_300to300MHz_VER006    NO.120
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_CMPI_24_10bit_12x1_4K2K60_300to600MHz_VER006    NO.121
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_CMPI_24_10bit_12x1_4K2K60_300to300MHz_VER006    NO.122
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_CMPI_24_10bit_12x1_4K1K144_280to360MHz_VER006    NO.123
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_CMPI_24_10bit_12x1_4K1K144_180to280MHz_VER006    NO.124
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_CMPI_24_10bit_12x1_4K1K144_180to180MHz_VER006    NO.125
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_ISP_8bit_4x2_4K2K60_450to600MHz_VER006    NO.126
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_ISP_8bit_4x2_4K2K60_300to450MHz_VER006    NO.127
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_ISP_8bit_4x2_4K2K60_300to300MHz_VER006    NO.128
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_ISP_8bit_6x1_4K2K60_350to600MHz_VER006    NO.129
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_ISP_8bit_6x1_4K2K60_300to350MHz_VER006    NO.130
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_ISP_8bit_6x1_4K2K60_300to300MHz_VER006    NO.131
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_ISP_8bit_6x2_4K2K60_300to600MHz_VER006    NO.132
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_ISP_8bit_6x2_4K2K60_300to300MHz_VER006    NO.133
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_ISP_8bit_8x1_4K2K60_450to600MHz_VER006    NO.134
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_ISP_8bit_8x1_4K2K60_300to450MHz_VER006    NO.135
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_ISP_8bit_8x1_4K2K60_300to300MHz_VER006    NO.136
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_ISP_8bit_12x1_4K2K60_300to600MHz_VER006    NO.137
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_ISP_8bit_12x1_4K2K60_300to300MHz_VER006    NO.138
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_ISP_8bit_12x1_4K1K144_180to360MHz_VER006    NO.139
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_ISP_8bit_12x1_4K1K144_180to180MHz_VER006    NO.140
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_ISP_10bit_4x2_4K2K60_400to600MHz_VER006    NO.141
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_ISP_10bit_4x2_4K2K60_300to400MHz_VER006    NO.142
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_ISP_10bit_4x2_4K2K60_300to300MHz_VER006    NO.143
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_ISP_10bit_6x2_4K2K60_300to600MHz_VER006    NO.144
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_ISP_10bit_6x2_4K2K60_300to300MHz_VER006    NO.145
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_ISP_10bit_8x1_4K2K60_400to600MHz_VER006    NO.146
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_ISP_10bit_8x1_4K2K60_300to400MHz_VER006    NO.147
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_ISP_10bit_8x1_4K2K60_300to300MHz_VER006    NO.148
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_ISP_10bit_12x1_4K2K60_300to600MHz_VER006    NO.149
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_ISP_10bit_12x1_4K2K60_300to300MHz_VER006    NO.150
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_ISP_10bit_12x1_4K1K144_300to360MHz_VER006    NO.151
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_ISP_10bit_12x1_4K1K144_180to300MHz_VER006    NO.152
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_ISP_10bit_12x1_4K1K144_180to180MHz_VER006    NO.153
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_CHPI_8bit_4x2_4K2K60_450to600MHz_VER006    NO.154
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_CHPI_8bit_4x2_4K2K60_300to450MHz_VER006    NO.155
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_CHPI_8bit_4x2_4K2K60_300to300MHz_VER006    NO.156
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_CHPI_8bit_6x1_4K2K60_300to600MHz_VER006    NO.157
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_CHPI_8bit_6x1_4K2K60_300to300MHz_VER006    NO.158
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_CHPI_8bit_6x2_4K2K60_300to600MHz_VER006    NO.159
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_CHPI_8bit_6x2_4K2K60_300to300MHz_VER006    NO.160
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_CHPI_8bit_8x1_4K2K60_450to600MHz_VER006    NO.161
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_CHPI_8bit_8x1_4K2K60_300to450MHz_VER006    NO.162
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_CHPI_8bit_8x1_4K2K60_300to300MHz_VER006    NO.163
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_CHPI_8bit_12x1_4K2K60_300to600MHz_VER006    NO.164
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_CHPI_8bit_12x1_4K2K60_300to300MHz_VER006    NO.165
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_CHPI_8bit_12x1_4K1K144_180to360MHz_VER006    NO.166
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_CHPI_8bit_12x1_4K1K144_180to180MHz_VER006    NO.167
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_CHPI_10bit_4x2_4K2K60_350to600MHz_VER006    NO.168
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_CHPI_10bit_4x2_4K2K60_300to350MHz_VER006    NO.169
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_CHPI_10bit_4x2_4K2K60_300to300MHz_VER006    NO.170
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_CHPI_10bit_8x1_4K2K60_350to600MHz_VER006    NO.171
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_CHPI_10bit_8x1_4K2K60_300to350MHz_VER006    NO.172
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_CHPI_10bit_8x1_4K2K60_300to300MHz_VER006    NO.173
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_CHPI_10bit_12x1_4K2K60_500to600MHz_VER006    NO.174
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_CHPI_10bit_12x1_4K2K60_300to500MHz_VER006    NO.175
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_CHPI_10bit_12x1_4K2K60_300to300MHz_VER006    NO.176
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_CHPI_10bit_12x1_4K1K144_280to360MHz_VER006    NO.177
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_CHPI_10bit_12x1_4K1K144_180to280MHz_VER006    NO.178
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_CHPI_10bit_12x1_4K1K144_180to180MHz_VER006    NO.179
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_Vx1_10bit_2_lane_2K60_150to300MHz_VER006    NO.180
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_Vx1_10bit_2_lane_2K60_150to150MHz_VER006    NO.181
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_Vx1_10bit_4_lane_4K2K30_300to600MHz_VER006    NO.182
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_Vx1_10bit_4_lane_4K2K30_300to300MHz_VER006    NO.183
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_Vx1_10bit_8_lane_4K2K60_300to600MHz_VER006    NO.184
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_Vx1_10bit_8_lane_4K2K60_300to300MHz_VER006    NO.185
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_Vx1_10bit_8_lane_4K1K144_180to360MHz_VER006    NO.186
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_Vx1_10bit_8_lane_4K1K144_180to180MHz_VER006    NO.187
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_Vx1_8bit_2_lane_2K60_225to300MHz_VER006    NO.188
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_Vx1_8bit_2_lane_2K60_150to225MHz_VER006    NO.189
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_Vx1_8bit_2_lane_2K60_150to150MHz_VER006    NO.190
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_Vx1_8bit_4_lane_4K2K30_450to600MHz_VER006    NO.191
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_Vx1_8bit_4_lane_4K2K30_300to450MHz_VER006    NO.192
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_Vx1_8bit_4_lane_4K2K30_300to300MHz_VER006    NO.193
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_Vx1_8bit_8_lane_4K2K60_450to600MHz_VER006    NO.194
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_Vx1_8bit_8_lane_4K2K60_300to450MHz_VER006    NO.195
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_Vx1_8bit_8_lane_4K2K60_300to300MHz_VER006    NO.196
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_Vx1_8bit_8_lane_4K1K144_225to360MHz_VER006    NO.197
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_Vx1_8bit_8_lane_4K1K144_180to225MHz_VER006    NO.198
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
	},   {		//E_PNL_SUPPORTED_LPLL_Vx1_8bit_8_lane_4K1K144_180to180MHz_VER006    NO.199
		//Address,Value,Mask
		{
		0x4861BC, 0x0, Fld(3, 12, AC_MSKW10)},	//reg_gcr_mpll_ictrl
		{
		0x4861B4, 0x0, Fld(2, 0, AC_MSKW10)},	//reg_gcr_mpll_input_div
		{
		0x4861B4, 0x2, Fld(2, 6, AC_MSKW10)},	//reg_gcr_mpll_loop_div_first
		{
		0x4861B4, 0x12, Fld(8, 8, AC_MSKW10)},	//reg_gcr_mpll_loop_div_second
		{
		0x4861BC, 0x4, Fld(3, 0, AC_MSKW10)},	//reg_gcr_mpll_output_div
		{
		0x4861B8, 0x31, Fld(16, 0, AC_MSKW10)},	//reg_mpll_test
},  };

uint16_t lpll_LoopGain_VER006[PNL_NUM_VER006] =  {
	16,			//E_PNL_SUPPORTED_LPLL_LVDS_1ch_HD60_225to300MHz_VER006    NO.0
	    8,			//E_PNL_SUPPORTED_LPLL_LVDS_1ch_HD60_150to225MHz_VER006    NO.1
	    8,			//E_PNL_SUPPORTED_LPLL_LVDS_1ch_HD60_150to150MHz_VER006    NO.2
	    16,			//E_PNL_SUPPORTED_LPLL_LVDS_2ch_2K60_225to300MHz_VER006    NO.3
	    8,			//E_PNL_SUPPORTED_LPLL_LVDS_2ch_2K60_150to225MHz_VER006    NO.4
	    8,			//E_PNL_SUPPORTED_LPLL_LVDS_2ch_2K60_150to150MHz_VER006    NO.5
	    16,			//E_PNL_SUPPORTED_LPLL_HS_LVDS_1ch_2K60_225to300MHz_VER006    NO.6
	    8,			//E_PNL_SUPPORTED_LPLL_HS_LVDS_1ch_2K60_150to225MHz_VER006    NO.7
	    8,			//E_PNL_SUPPORTED_LPLL_HS_LVDS_1ch_2K60_150to150MHz_VER006    NO.8
	    14,			//E_PNL_SUPPORTED_LPLL_HS_LVDS_2ch_4K2K30_225to300MHz_VER006    NO.9
	    14,			//E_PNL_SUPPORTED_LPLL_HS_LVDS_2ch_4K2K30_150to225MHz_VER006    NO.10
	    14,			//E_PNL_SUPPORTED_LPLL_HS_LVDS_2ch_4K2K30_150to150MHz_VER006    NO.11
	    8,			//E_PNL_SUPPORTED_LPLL_MINILVDS_1bk_3pair_6bit_HD60_150to300MHz_VER006    NO.12
	    8,			//E_PNL_SUPPORTED_LPLL_MINILVDS_1bk_3pair_6bit_HD60_150to150MHz_VER006    NO.13
	    8,			//E_PNL_SUPPORTED_LPLL_MINILVDS_1bk_6pair_6bit_HD60_150to300MHz_VER006    NO.14
	    8,			//E_PNL_SUPPORTED_LPLL_MINILVDS_1bk_6pair_6bit_HD60_150to150MHz_VER006    NO.15
	    8,			//E_PNL_SUPPORTED_LPLL_MINILVDS_2bk_3pair_6bit_2K60_150to300MHz_VER006    NO.16
	    8,			//E_PNL_SUPPORTED_LPLL_MINILVDS_2bk_3pair_6bit_2K60_150to150MHz_VER006    NO.17
	    8,			//E_PNL_SUPPORTED_LPLL_MINILVDS_2bk_6pair_6bit_2K60_150to300MHz_VER006    NO.18
	    8,			//E_PNL_SUPPORTED_LPLL_MINILVDS_2bk_6pair_6bit_2K60_150to150MHz_VER006    NO.19
	    8,			//E_PNL_SUPPORTED_LPLL_MINILVDS_1bk_3pair_8bit_HD60_150to300MHz_VER006    NO.20
	    8,			//E_PNL_SUPPORTED_LPLL_MINILVDS_1bk_3pair_8bit_HD60_150to150MHz_VER006    NO.21
	    8,			//E_PNL_SUPPORTED_LPLL_MINILVDS_1bk_6pair_8bit_HD60_150to300MHz_VER006    NO.22
	    8,			//E_PNL_SUPPORTED_LPLL_MINILVDS_1bk_6pair_8bit_HD60_150to150MHz_VER006    NO.23
	    8,			//E_PNL_SUPPORTED_LPLL_MINILVDS_2bk_3pair_8bit_2K60_150to300MHz_VER006    NO.24
	    8,			//E_PNL_SUPPORTED_LPLL_MINILVDS_2bk_3pair_8bit_2K60_150to150MHz_VER006    NO.25
	    8,			//E_PNL_SUPPORTED_LPLL_MINILVDS_2bk_6pair_8bit_2K60_150to300MHz_VER006    NO.26
	    8,			//E_PNL_SUPPORTED_LPLL_MINILVDS_2bk_6pair_8bit_2K60_150to150MHz_VER006    NO.27
	    6,			//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_2pair_2K60_CML_225to300MHz_VER006    NO.28
	    6,			//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_2pair_2K60_CML_150to225MHz_VER006    NO.29
	    6,			//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_2pair_2K60_CML_150to150MHz_VER006    NO.30
	    6,			//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_4pair_2K60_CML_225to300MHz_VER006    NO.31
	    6,			//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_4pair_2K60_CML_150to225MHz_VER006    NO.32
	    6,			//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_4pair_2K60_CML_150to150MHz_VER006    NO.33
	    6,			//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_6pair_2K120_CML_300to600MHz_VER006    NO.34
	    6,			//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_6pair_2K120_CML_300to300MHz_VER006    NO.35
	    6,			//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_8pair_2K120_CML_450to600MHz_VER006    NO.36
	    6,			//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_8pair_2K120_CML_300to450MHz_VER006    NO.37
	    6,			//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_8pair_2K120_CML_300to300MHz_VER006    NO.38
	    6,			//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_4x2_4K2K60_CML_450to600MHz_VER006    NO.39
	    6,			//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_4x2_4K2K60_CML_300to450MHz_VER006    NO.40
	    6,			//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_4x2_4K2K60_CML_300to300MHz_VER006    NO.41
	    6,			//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_6x1_4K2K60_CML_350to600MHz_VER006    NO.42
	    6,			//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_6x1_4K2K60_CML_300to350MHz_VER006    NO.43
	    6,			//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_6x1_4K2K60_CML_300to300MHz_VER006    NO.44
	    6,			//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_8x1_4K2K60_CML_450to600MHz_VER006    NO.45
	    6,			//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_8x1_4K2K60_CML_300to450MHz_VER006    NO.46
	    6,			//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_8x1_4K2K60_CML_300to300MHz_VER006    NO.47
	    6,			//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_12x1_4K2K60_CML_300to600MHz_VER006    NO.48
	    6,			//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_12x1_4K2K60_CML_300to300MHz_VER006    NO.49
	    6,			//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_12x1_4K1K144_CML_180to360MHz_VER006    NO.50
	    6,			//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_12x1_4K1K144_CML_180to180MHz_VER006    NO.51
	    6,			//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_16x1_4K2K60_CML_450to600MHz_VER006    NO.52
	    6,			//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_16x1_4K2K60_CML_300to450MHz_VER006    NO.53
	    6,			//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_16x1_4K2K60_CML_300to300MHz_VER006    NO.54
	    6,			//E_PNL_SUPPORTED_LPLL_EPI_24_10bit_16x1_4K2K60_CML_300to600MHz_VER006    NO.55
	    6,			//E_PNL_SUPPORTED_LPLL_EPI_24_10bit_16x1_4K2K60_CML_300to300MHz_VER006    NO.56
	    6,			//E_PNL_SUPPORTED_LPLL_EPI_24_10bit_4x2_4K2K60_CML_350to600MHz_VER006    NO.57
	    6,			//E_PNL_SUPPORTED_LPLL_EPI_24_10bit_4x2_4K2K60_CML_300to350MHz_VER006    NO.58
	    6,			//E_PNL_SUPPORTED_LPLL_EPI_24_10bit_4x2_4K2K60_CML_300to300MHz_VER006    NO.59
	    6,			//E_PNL_SUPPORTED_LPLL_EPI_24_10bit_12x1_4K2K60_CML_300to600MHz_VER006    NO.60
	    6,			//E_PNL_SUPPORTED_LPLL_EPI_24_10bit_12x1_4K2K60_CML_300to300MHz_VER006    NO.61
	    6,			//E_PNL_SUPPORTED_LPLL_EPI_24_10bit_12x1_4K1K144_CML_280to360MHz_VER006    NO.62
	    6,			//E_PNL_SUPPORTED_LPLL_EPI_24_10bit_12x1_4K1K144_CML_180to280MHz_VER006    NO.63
	    6,			//E_PNL_SUPPORTED_LPLL_EPI_24_10bit_12x1_4K1K144_CML_180to180MHz_VER006    NO.64
	    6,			//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_2pair_2K60_LVDS_225to300MHz_VER006    NO.65
	    6,			//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_2pair_2K60_LVDS_150to225MHz_VER006    NO.66
	    6,			//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_2pair_2K60_LVDS_150to150MHz_VER006    NO.67
	    6,			//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_4pair_2K60_LVDS_225to300MHz_VER006    NO.68
	    6,			//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_4pair_2K60_LVDS_150to225MHz_VER006    NO.69
	    6,			//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_4pair_2K60_LVDS_150to150MHz_VER006    NO.70
	    6,			//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_6pair_2K120_LVDS_300to600MHz_VER006    NO.71
	    6,			//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_6pair_2K120_LVDS_300to300MHz_VER006    NO.72
	    6,			//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_8pair_2K120_LVDS_450to600MHz_VER006    NO.73
	    6,			//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_8pair_2K120_LVDS_300to450MHz_VER006    NO.74
	    6,			//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_8pair_2K120_LVDS_300to300MHz_VER006    NO.75
	    6,			//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_4x2_4K2K60_LVDS_450to600MHz_VER006    NO.76
	    6,			//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_4x2_4K2K60_LVDS_300to450MHz_VER006    NO.77
	    6,			//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_4x2_4K2K60_LVDS_300to300MHz_VER006    NO.78
	    6,			//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_6x1_4K2K60_LVDS_350to600MHz_VER006    NO.79
	    6,			//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_6x1_4K2K60_LVDS_300to350MHz_VER006    NO.80
	    6,			//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_6x1_4K2K60_LVDS_300to300MHz_VER006    NO.81
	    6,			//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_8x1_4K2K60_LVDS_450to600MHz_VER006    NO.82
	    6,			//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_8x1_4K2K60_LVDS_300to450MHz_VER006    NO.83
	    6,			//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_8x1_4K2K60_LVDS_300to300MHz_VER006    NO.84
	    6,			//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_12x1_4K2K60_LVDS_300to600MHz_VER006    NO.85
	    6,			//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_12x1_4K2K60_LVDS_300to300MHz_VER006    NO.86
	    6,			//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_12x1_4K1K144_LVDS_180to360MHz_VER006    NO.87
	    6,			//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_12x1_4K1K144_LVDS_180to180MHz_VER006    NO.88
	    6,			//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_16x1_4K_450to600MHz_VER006    NO.89
	    6,			//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_16x1_4K_300to450MHz_VER006    NO.90
	    6,			//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_16x1_4K_300to300MHz_VER006    NO.91
	    6,			//E_PNL_SUPPORTED_LPLL_EPI_24_10bit_16x1_4K2K60_LVDS_300to600MHz_VER006    NO.92
	    6,			//E_PNL_SUPPORTED_LPLL_EPI_24_10bit_16x1_4K2K60_LVDS_300to300MHz_VER006    NO.93
	    6,			//E_PNL_SUPPORTED_LPLL_EPI_24_10bit_4x2_4K2K60_LVDS_350to600MHz_VER006    NO.94
	    6,			//E_PNL_SUPPORTED_LPLL_EPI_24_10bit_4x2_4K2K60_LVDS_300to350MHz_VER006    NO.95
	    6,			//E_PNL_SUPPORTED_LPLL_EPI_24_10bit_4x2_4K2K60_LVDS_300to300MHz_VER006    NO.96
	    6,			//E_PNL_SUPPORTED_LPLL_EPI_24_10bit_12x1_4K2K60_LVDS_300to600MHz_VER006    NO.97
	    6,			//E_PNL_SUPPORTED_LPLL_EPI_24_10bit_12x1_4K2K60_LVDS_300to300MHz_VER006    NO.98
	    6,			//E_PNL_SUPPORTED_LPLL_EPI_24_10bit_12x1_4K1K144_LVDS_280to360MHz_VER006    NO.99
	    6,			//E_PNL_SUPPORTED_LPLL_EPI_24_10bit_12x1_4K1K144_LVDS_180to280MHz_VER006    NO.100
	    6,			//E_PNL_SUPPORTED_LPLL_EPI_24_10bit_12x1_4K1K144_LVDS_180to180MHz_VER006    NO.101
	    12,			//E_PNL_SUPPORTED_LPLL_CMPI_27_8bit_4x2_4K2K60_450to600MHz_VER006    NO.102
	    6,			//E_PNL_SUPPORTED_LPLL_CMPI_27_8bit_4x2_4K2K60_300to450MHz_VER006    NO.103
	    6,			//E_PNL_SUPPORTED_LPLL_CMPI_27_8bit_4x2_4K2K60_300to300MHz_VER006    NO.104
	    6,			//E_PNL_SUPPORTED_LPLL_CMPI_27_8bit_6x1_4K2K60_350to600MHz_VER006    NO.105
	    6,			//E_PNL_SUPPORTED_LPLL_CMPI_27_8bit_6x1_4K2K60_300to350MHz_VER006    NO.106
	    6,			//E_PNL_SUPPORTED_LPLL_CMPI_27_8bit_6x1_4K2K60_300to300MHz_VER006    NO.107
	    12,			//E_PNL_SUPPORTED_LPLL_CMPI_27_8bit_8x1_4K2K60_450to600MHz_VER006    NO.108
	    6,			//E_PNL_SUPPORTED_LPLL_CMPI_27_8bit_8x1_4K2K60_300to450MHz_VER006    NO.109
	    6,			//E_PNL_SUPPORTED_LPLL_CMPI_27_8bit_8x1_4K2K60_300to300MHz_VER006    NO.110
	    6,			//E_PNL_SUPPORTED_LPLL_CMPI_27_8bit_12x1_4K2K60_300to600MHz_VER006    NO.111
	    6,			//E_PNL_SUPPORTED_LPLL_CMPI_27_8bit_12x1_4K2K60_300to300MHz_VER006    NO.112
	    6,			//E_PNL_SUPPORTED_LPLL_CMPI_27_8bit_12x1_4K1K144_180to360MHz_VER006    NO.113
	    6,			//E_PNL_SUPPORTED_LPLL_CMPI_27_8bit_12x1_4K1K144_180to180MHz_VER006    NO.114
	    6,			//E_PNL_SUPPORTED_LPLL_CMPI_24_10bit_4x2_4K2K60_350to600MHz_VER006    NO.115
	    6,			//E_PNL_SUPPORTED_LPLL_CMPI_24_10bit_4x2_4K2K60_300to350MHz_VER006    NO.116
	    6,			//E_PNL_SUPPORTED_LPLL_CMPI_24_10bit_4x2_4K2K60_300to300MHz_VER006    NO.117
	    6,			//E_PNL_SUPPORTED_LPLL_CMPI_24_10bit_8x1_4K2K60_350to600MHz_VER006    NO.118
	    6,			//E_PNL_SUPPORTED_LPLL_CMPI_24_10bit_8x1_4K2K60_300to350MHz_VER006    NO.119
	    6,			//E_PNL_SUPPORTED_LPLL_CMPI_24_10bit_8x1_4K2K60_300to300MHz_VER006    NO.120
	    6,			//E_PNL_SUPPORTED_LPLL_CMPI_24_10bit_12x1_4K2K60_300to600MHz_VER006    NO.121
	    6,			//E_PNL_SUPPORTED_LPLL_CMPI_24_10bit_12x1_4K2K60_300to300MHz_VER006    NO.122
	    6,			//E_PNL_SUPPORTED_LPLL_CMPI_24_10bit_12x1_4K1K144_280to360MHz_VER006    NO.123
	    6,			//E_PNL_SUPPORTED_LPLL_CMPI_24_10bit_12x1_4K1K144_180to280MHz_VER006    NO.124
	    6,			//E_PNL_SUPPORTED_LPLL_CMPI_24_10bit_12x1_4K1K144_180to180MHz_VER006    NO.125
	    12,			//E_PNL_SUPPORTED_LPLL_ISP_8bit_4x2_4K2K60_450to600MHz_VER006    NO.126
	    6,			//E_PNL_SUPPORTED_LPLL_ISP_8bit_4x2_4K2K60_300to450MHz_VER006    NO.127
	    6,			//E_PNL_SUPPORTED_LPLL_ISP_8bit_4x2_4K2K60_300to300MHz_VER006    NO.128
	    6,			//E_PNL_SUPPORTED_LPLL_ISP_8bit_6x1_4K2K60_350to600MHz_VER006    NO.129
	    6,			//E_PNL_SUPPORTED_LPLL_ISP_8bit_6x1_4K2K60_300to350MHz_VER006    NO.130
	    6,			//E_PNL_SUPPORTED_LPLL_ISP_8bit_6x1_4K2K60_300to300MHz_VER006    NO.131
	    6,			//E_PNL_SUPPORTED_LPLL_ISP_8bit_6x2_4K2K60_300to600MHz_VER006    NO.132
	    6,			//E_PNL_SUPPORTED_LPLL_ISP_8bit_6x2_4K2K60_300to300MHz_VER006    NO.133
	    12,			//E_PNL_SUPPORTED_LPLL_ISP_8bit_8x1_4K2K60_450to600MHz_VER006    NO.134
	    6,			//E_PNL_SUPPORTED_LPLL_ISP_8bit_8x1_4K2K60_300to450MHz_VER006    NO.135
	    6,			//E_PNL_SUPPORTED_LPLL_ISP_8bit_8x1_4K2K60_300to300MHz_VER006    NO.136
	    6,			//E_PNL_SUPPORTED_LPLL_ISP_8bit_12x1_4K2K60_300to600MHz_VER006    NO.137
	    6,			//E_PNL_SUPPORTED_LPLL_ISP_8bit_12x1_4K2K60_300to300MHz_VER006    NO.138
	    6,			//E_PNL_SUPPORTED_LPLL_ISP_8bit_12x1_4K1K144_180to360MHz_VER006    NO.139
	    6,			//E_PNL_SUPPORTED_LPLL_ISP_8bit_12x1_4K1K144_180to180MHz_VER006    NO.140
	    12,			//E_PNL_SUPPORTED_LPLL_ISP_10bit_4x2_4K2K60_400to600MHz_VER006    NO.141
	    12,			//E_PNL_SUPPORTED_LPLL_ISP_10bit_4x2_4K2K60_300to400MHz_VER006    NO.142
	    12,			//E_PNL_SUPPORTED_LPLL_ISP_10bit_4x2_4K2K60_300to300MHz_VER006    NO.143
	    24,			//E_PNL_SUPPORTED_LPLL_ISP_10bit_6x2_4K2K60_300to600MHz_VER006    NO.144
	    24,			//E_PNL_SUPPORTED_LPLL_ISP_10bit_6x2_4K2K60_300to300MHz_VER006    NO.145
	    12,			//E_PNL_SUPPORTED_LPLL_ISP_10bit_8x1_4K2K60_400to600MHz_VER006    NO.146
	    12,			//E_PNL_SUPPORTED_LPLL_ISP_10bit_8x1_4K2K60_300to400MHz_VER006    NO.147
	    12,			//E_PNL_SUPPORTED_LPLL_ISP_10bit_8x1_4K2K60_300to300MHz_VER006    NO.148
	    24,			//E_PNL_SUPPORTED_LPLL_ISP_10bit_12x1_4K2K60_300to600MHz_VER006    NO.149
	    24,			//E_PNL_SUPPORTED_LPLL_ISP_10bit_12x1_4K2K60_300to300MHz_VER006    NO.150
	    12,			//E_PNL_SUPPORTED_LPLL_ISP_10bit_12x1_4K1K144_300to360MHz_VER006    NO.151
	    12,			//E_PNL_SUPPORTED_LPLL_ISP_10bit_12x1_4K1K144_180to300MHz_VER006    NO.152
	    12,			//E_PNL_SUPPORTED_LPLL_ISP_10bit_12x1_4K1K144_180to180MHz_VER006    NO.153
	    24,			//E_PNL_SUPPORTED_LPLL_CHPI_8bit_4x2_4K2K60_450to600MHz_VER006    NO.154
	    12,			//E_PNL_SUPPORTED_LPLL_CHPI_8bit_4x2_4K2K60_300to450MHz_VER006    NO.155
	    12,			//E_PNL_SUPPORTED_LPLL_CHPI_8bit_4x2_4K2K60_300to300MHz_VER006    NO.156
	    6,			//E_PNL_SUPPORTED_LPLL_CHPI_8bit_6x1_4K2K60_300to600MHz_VER006    NO.157
	    6,			//E_PNL_SUPPORTED_LPLL_CHPI_8bit_6x1_4K2K60_300to300MHz_VER006    NO.158
	    6,			//E_PNL_SUPPORTED_LPLL_CHPI_8bit_6x2_4K2K60_300to600MHz_VER006    NO.159
	    6,			//E_PNL_SUPPORTED_LPLL_CHPI_8bit_6x2_4K2K60_300to300MHz_VER006    NO.160
	    24,			//E_PNL_SUPPORTED_LPLL_CHPI_8bit_8x1_4K2K60_450to600MHz_VER006    NO.161
	    12,			//E_PNL_SUPPORTED_LPLL_CHPI_8bit_8x1_4K2K60_300to450MHz_VER006    NO.162
	    12,			//E_PNL_SUPPORTED_LPLL_CHPI_8bit_8x1_4K2K60_300to300MHz_VER006    NO.163
	    6,			//E_PNL_SUPPORTED_LPLL_CHPI_8bit_12x1_4K2K60_300to600MHz_VER006    NO.164
	    6,			//E_PNL_SUPPORTED_LPLL_CHPI_8bit_12x1_4K2K60_300to300MHz_VER006    NO.165
	    6,			//E_PNL_SUPPORTED_LPLL_CHPI_8bit_12x1_4K1K144_180to360MHz_VER006    NO.166
	    6,			//E_PNL_SUPPORTED_LPLL_CHPI_8bit_12x1_4K1K144_180to180MHz_VER006    NO.167
	    12,			//E_PNL_SUPPORTED_LPLL_CHPI_10bit_4x2_4K2K60_350to600MHz_VER006    NO.168
	    12,			//E_PNL_SUPPORTED_LPLL_CHPI_10bit_4x2_4K2K60_300to350MHz_VER006    NO.169
	    12,			//E_PNL_SUPPORTED_LPLL_CHPI_10bit_4x2_4K2K60_300to300MHz_VER006    NO.170
	    12,			//E_PNL_SUPPORTED_LPLL_CHPI_10bit_8x1_4K2K60_350to600MHz_VER006    NO.171
	    12,			//E_PNL_SUPPORTED_LPLL_CHPI_10bit_8x1_4K2K60_300to350MHz_VER006    NO.172
	    12,			//E_PNL_SUPPORTED_LPLL_CHPI_10bit_8x1_4K2K60_300to300MHz_VER006    NO.173
	    24,			//E_PNL_SUPPORTED_LPLL_CHPI_10bit_12x1_4K2K60_500to600MHz_VER006    NO.174
	    12,			//E_PNL_SUPPORTED_LPLL_CHPI_10bit_12x1_4K2K60_300to500MHz_VER006    NO.175
	    12,			//E_PNL_SUPPORTED_LPLL_CHPI_10bit_12x1_4K2K60_300to300MHz_VER006    NO.176
	    12,			//E_PNL_SUPPORTED_LPLL_CHPI_10bit_12x1_4K1K144_280to360MHz_VER006    NO.177
	    6,			//E_PNL_SUPPORTED_LPLL_CHPI_10bit_12x1_4K1K144_180to280MHz_VER006    NO.178
	    6,			//E_PNL_SUPPORTED_LPLL_CHPI_10bit_12x1_4K1K144_180to180MHz_VER006    NO.179
	    6,			//E_PNL_SUPPORTED_LPLL_Vx1_10bit_2_lane_2K60_150to300MHz_VER006    NO.180
	    6,			//E_PNL_SUPPORTED_LPLL_Vx1_10bit_2_lane_2K60_150to150MHz_VER006    NO.181
	    6,			//E_PNL_SUPPORTED_LPLL_Vx1_10bit_4_lane_4K2K30_300to600MHz_VER006    NO.182
	    6,			//E_PNL_SUPPORTED_LPLL_Vx1_10bit_4_lane_4K2K30_300to300MHz_VER006    NO.183
	    6,			//E_PNL_SUPPORTED_LPLL_Vx1_10bit_8_lane_4K2K60_300to600MHz_VER006    NO.184
	    6,			//E_PNL_SUPPORTED_LPLL_Vx1_10bit_8_lane_4K2K60_300to300MHz_VER006    NO.185
	    6,			//E_PNL_SUPPORTED_LPLL_Vx1_10bit_8_lane_4K1K144_180to360MHz_VER006    NO.186
	    6,			//E_PNL_SUPPORTED_LPLL_Vx1_10bit_8_lane_4K1K144_180to180MHz_VER006    NO.187
	    12,			//E_PNL_SUPPORTED_LPLL_Vx1_8bit_2_lane_2K60_225to300MHz_VER006    NO.188
	    6,			//E_PNL_SUPPORTED_LPLL_Vx1_8bit_2_lane_2K60_150to225MHz_VER006    NO.189
	    6,			//E_PNL_SUPPORTED_LPLL_Vx1_8bit_2_lane_2K60_150to150MHz_VER006    NO.190
	    24,			//E_PNL_SUPPORTED_LPLL_Vx1_8bit_4_lane_4K2K30_450to600MHz_VER006    NO.191
	    12,			//E_PNL_SUPPORTED_LPLL_Vx1_8bit_4_lane_4K2K30_300to450MHz_VER006    NO.192
	    12,			//E_PNL_SUPPORTED_LPLL_Vx1_8bit_4_lane_4K2K30_300to300MHz_VER006    NO.193
	    24,			//E_PNL_SUPPORTED_LPLL_Vx1_8bit_8_lane_4K2K60_450to600MHz_VER006    NO.194
	    12,			//E_PNL_SUPPORTED_LPLL_Vx1_8bit_8_lane_4K2K60_300to450MHz_VER006    NO.195
	    12,			//E_PNL_SUPPORTED_LPLL_Vx1_8bit_8_lane_4K2K60_300to300MHz_VER006    NO.196
	    12,			//E_PNL_SUPPORTED_LPLL_Vx1_8bit_8_lane_4K1K144_225to360MHz_VER006    NO.197
	    6,			//E_PNL_SUPPORTED_LPLL_Vx1_8bit_8_lane_4K1K144_180to225MHz_VER006    NO.198
	    6,			//E_PNL_SUPPORTED_LPLL_Vx1_8bit_8_lane_4K1K144_180to180MHz_VER006    NO.199
};

uint16_t lpll_LoopDiv_VER006[PNL_NUM_VER006] =  {
	7,			//E_PNL_SUPPORTED_LPLL_LVDS_1ch_HD60_225to300MHz_VER006    NO.0
	    7,			//E_PNL_SUPPORTED_LPLL_LVDS_1ch_HD60_150to225MHz_VER006    NO.1
	    7,			//E_PNL_SUPPORTED_LPLL_LVDS_1ch_HD60_150to150MHz_VER006    NO.2
	    7,			//E_PNL_SUPPORTED_LPLL_LVDS_2ch_2K60_225to300MHz_VER006    NO.3
	    7,			//E_PNL_SUPPORTED_LPLL_LVDS_2ch_2K60_150to225MHz_VER006    NO.4
	    7,			//E_PNL_SUPPORTED_LPLL_LVDS_2ch_2K60_150to150MHz_VER006    NO.5
	    7,			//E_PNL_SUPPORTED_LPLL_HS_LVDS_1ch_2K60_225to300MHz_VER006    NO.6
	    7,			//E_PNL_SUPPORTED_LPLL_HS_LVDS_1ch_2K60_150to225MHz_VER006    NO.7
	    7,			//E_PNL_SUPPORTED_LPLL_HS_LVDS_1ch_2K60_150to150MHz_VER006    NO.8
	    7,			//E_PNL_SUPPORTED_LPLL_HS_LVDS_2ch_4K2K30_225to300MHz_VER006    NO.9
	    7,			//E_PNL_SUPPORTED_LPLL_HS_LVDS_2ch_4K2K30_150to225MHz_VER006    NO.10
	    7,			//E_PNL_SUPPORTED_LPLL_HS_LVDS_2ch_4K2K30_150to150MHz_VER006    NO.11
	    6,			//E_PNL_SUPPORTED_LPLL_MINILVDS_1bk_3pair_6bit_HD60_150to300MHz_VER006    NO.12
	    6,			//E_PNL_SUPPORTED_LPLL_MINILVDS_1bk_3pair_6bit_HD60_150to150MHz_VER006    NO.13
	    6,			//E_PNL_SUPPORTED_LPLL_MINILVDS_1bk_6pair_6bit_HD60_150to300MHz_VER006    NO.14
	    6,			//E_PNL_SUPPORTED_LPLL_MINILVDS_1bk_6pair_6bit_HD60_150to150MHz_VER006    NO.15
	    6,			//E_PNL_SUPPORTED_LPLL_MINILVDS_2bk_3pair_6bit_2K60_150to300MHz_VER006    NO.16
	    6,			//E_PNL_SUPPORTED_LPLL_MINILVDS_2bk_3pair_6bit_2K60_150to150MHz_VER006    NO.17
	    6,			//E_PNL_SUPPORTED_LPLL_MINILVDS_2bk_6pair_6bit_2K60_150to300MHz_VER006    NO.18
	    6,			//E_PNL_SUPPORTED_LPLL_MINILVDS_2bk_6pair_6bit_2K60_150to150MHz_VER006    NO.19
	    4,			//E_PNL_SUPPORTED_LPLL_MINILVDS_1bk_3pair_8bit_HD60_150to300MHz_VER006    NO.20
	    4,			//E_PNL_SUPPORTED_LPLL_MINILVDS_1bk_3pair_8bit_HD60_150to150MHz_VER006    NO.21
	    4,			//E_PNL_SUPPORTED_LPLL_MINILVDS_1bk_6pair_8bit_HD60_150to300MHz_VER006    NO.22
	    4,			//E_PNL_SUPPORTED_LPLL_MINILVDS_1bk_6pair_8bit_HD60_150to150MHz_VER006    NO.23
	    4,			//E_PNL_SUPPORTED_LPLL_MINILVDS_2bk_3pair_8bit_2K60_150to300MHz_VER006    NO.24
	    4,			//E_PNL_SUPPORTED_LPLL_MINILVDS_2bk_3pair_8bit_2K60_150to150MHz_VER006    NO.25
	    4,			//E_PNL_SUPPORTED_LPLL_MINILVDS_2bk_6pair_8bit_2K60_150to300MHz_VER006    NO.26
	    4,			//E_PNL_SUPPORTED_LPLL_MINILVDS_2bk_6pair_8bit_2K60_150to150MHz_VER006    NO.27
	    4,			//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_2pair_2K60_CML_225to300MHz_VER006    NO.28
	    4,			//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_2pair_2K60_CML_150to225MHz_VER006    NO.29
	    4,			//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_2pair_2K60_CML_150to150MHz_VER006    NO.30
	    4,			//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_4pair_2K60_CML_225to300MHz_VER006    NO.31
	    4,			//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_4pair_2K60_CML_150to225MHz_VER006    NO.32
	    4,			//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_4pair_2K60_CML_150to150MHz_VER006    NO.33
	    2,			//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_6pair_2K120_CML_300to600MHz_VER006    NO.34
	    2,			//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_6pair_2K120_CML_300to300MHz_VER006    NO.35
	    2,			//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_8pair_2K120_CML_450to600MHz_VER006    NO.36
	    2,			//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_8pair_2K120_CML_300to450MHz_VER006    NO.37
	    2,			//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_8pair_2K120_CML_300to300MHz_VER006    NO.38
	    2,			//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_4x2_4K2K60_CML_450to600MHz_VER006    NO.39
	    2,			//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_4x2_4K2K60_CML_300to450MHz_VER006    NO.40
	    2,			//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_4x2_4K2K60_CML_300to300MHz_VER006    NO.41
	    2,			//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_6x1_4K2K60_CML_350to600MHz_VER006    NO.42
	    2,			//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_6x1_4K2K60_CML_300to350MHz_VER006    NO.43
	    2,			//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_6x1_4K2K60_CML_300to300MHz_VER006    NO.44
	    2,			//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_8x1_4K2K60_CML_450to600MHz_VER006    NO.45
	    2,			//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_8x1_4K2K60_CML_300to450MHz_VER006    NO.46
	    2,			//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_8x1_4K2K60_CML_300to300MHz_VER006    NO.47
	    2,			//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_12x1_4K2K60_CML_300to600MHz_VER006    NO.48
	    2,			//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_12x1_4K2K60_CML_300to300MHz_VER006    NO.49
	    4,			//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_12x1_4K1K144_CML_180to360MHz_VER006    NO.50
	    4,			//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_12x1_4K1K144_CML_180to180MHz_VER006    NO.51
	    2,			//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_16x1_4K2K60_CML_450to600MHz_VER006    NO.52
	    2,			//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_16x1_4K2K60_CML_300to450MHz_VER006    NO.53
	    2,			//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_16x1_4K2K60_CML_300to300MHz_VER006    NO.54
	    2,			//E_PNL_SUPPORTED_LPLL_EPI_24_10bit_16x1_4K2K60_CML_300to600MHz_VER006    NO.55
	    2,			//E_PNL_SUPPORTED_LPLL_EPI_24_10bit_16x1_4K2K60_CML_300to300MHz_VER006    NO.56
	    2,			//E_PNL_SUPPORTED_LPLL_EPI_24_10bit_4x2_4K2K60_CML_350to600MHz_VER006    NO.57
	    2,			//E_PNL_SUPPORTED_LPLL_EPI_24_10bit_4x2_4K2K60_CML_300to350MHz_VER006    NO.58
	    2,			//E_PNL_SUPPORTED_LPLL_EPI_24_10bit_4x2_4K2K60_CML_300to300MHz_VER006    NO.59
	    2,			//E_PNL_SUPPORTED_LPLL_EPI_24_10bit_12x1_4K2K60_CML_300to600MHz_VER006    NO.60
	    2,			//E_PNL_SUPPORTED_LPLL_EPI_24_10bit_12x1_4K2K60_CML_300to300MHz_VER006    NO.61
	    4,			//E_PNL_SUPPORTED_LPLL_EPI_24_10bit_12x1_4K1K144_CML_280to360MHz_VER006    NO.62
	    4,			//E_PNL_SUPPORTED_LPLL_EPI_24_10bit_12x1_4K1K144_CML_180to280MHz_VER006    NO.63
	    4,			//E_PNL_SUPPORTED_LPLL_EPI_24_10bit_12x1_4K1K144_CML_180to180MHz_VER006    NO.64
	    4,			//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_2pair_2K60_LVDS_225to300MHz_VER006    NO.65
	    4,			//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_2pair_2K60_LVDS_150to225MHz_VER006    NO.66
	    4,			//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_2pair_2K60_LVDS_150to150MHz_VER006    NO.67
	    4,			//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_4pair_2K60_LVDS_225to300MHz_VER006    NO.68
	    4,			//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_4pair_2K60_LVDS_150to225MHz_VER006    NO.69
	    4,			//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_4pair_2K60_LVDS_150to150MHz_VER006    NO.70
	    2,			//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_6pair_2K120_LVDS_300to600MHz_VER006    NO.71
	    2,			//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_6pair_2K120_LVDS_300to300MHz_VER006    NO.72
	    2,			//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_8pair_2K120_LVDS_450to600MHz_VER006    NO.73
	    2,			//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_8pair_2K120_LVDS_300to450MHz_VER006    NO.74
	    2,			//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_8pair_2K120_LVDS_300to300MHz_VER006    NO.75
	    2,			//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_4x2_4K2K60_LVDS_450to600MHz_VER006    NO.76
	    2,			//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_4x2_4K2K60_LVDS_300to450MHz_VER006    NO.77
	    2,			//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_4x2_4K2K60_LVDS_300to300MHz_VER006    NO.78
	    2,			//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_6x1_4K2K60_LVDS_350to600MHz_VER006    NO.79
	    2,			//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_6x1_4K2K60_LVDS_300to350MHz_VER006    NO.80
	    2,			//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_6x1_4K2K60_LVDS_300to300MHz_VER006    NO.81
	    2,			//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_8x1_4K2K60_LVDS_450to600MHz_VER006    NO.82
	    2,			//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_8x1_4K2K60_LVDS_300to450MHz_VER006    NO.83
	    2,			//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_8x1_4K2K60_LVDS_300to300MHz_VER006    NO.84
	    2,			//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_12x1_4K2K60_LVDS_300to600MHz_VER006    NO.85
	    2,			//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_12x1_4K2K60_LVDS_300to300MHz_VER006    NO.86
	    4,			//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_12x1_4K1K144_LVDS_180to360MHz_VER006    NO.87
	    4,			//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_12x1_4K1K144_LVDS_180to180MHz_VER006    NO.88
	    2,			//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_16x1_4K_450to600MHz_VER006    NO.89
	    2,			//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_16x1_4K_300to450MHz_VER006    NO.90
	    2,			//E_PNL_SUPPORTED_LPLL_EPI_28_8bit_16x1_4K_300to300MHz_VER006    NO.91
	    2,			//E_PNL_SUPPORTED_LPLL_EPI_24_10bit_16x1_4K2K60_LVDS_300to600MHz_VER006    NO.92
	    2,			//E_PNL_SUPPORTED_LPLL_EPI_24_10bit_16x1_4K2K60_LVDS_300to300MHz_VER006    NO.93
	    2,			//E_PNL_SUPPORTED_LPLL_EPI_24_10bit_4x2_4K2K60_LVDS_350to600MHz_VER006    NO.94
	    2,			//E_PNL_SUPPORTED_LPLL_EPI_24_10bit_4x2_4K2K60_LVDS_300to350MHz_VER006    NO.95
	    2,			//E_PNL_SUPPORTED_LPLL_EPI_24_10bit_4x2_4K2K60_LVDS_300to300MHz_VER006    NO.96
	    2,			//E_PNL_SUPPORTED_LPLL_EPI_24_10bit_12x1_4K2K60_LVDS_300to600MHz_VER006    NO.97
	    2,			//E_PNL_SUPPORTED_LPLL_EPI_24_10bit_12x1_4K2K60_LVDS_300to300MHz_VER006    NO.98
	    4,			//E_PNL_SUPPORTED_LPLL_EPI_24_10bit_12x1_4K1K144_LVDS_280to360MHz_VER006    NO.99
	    4,			//E_PNL_SUPPORTED_LPLL_EPI_24_10bit_12x1_4K1K144_LVDS_180to280MHz_VER006    NO.100
	    4,			//E_PNL_SUPPORTED_LPLL_EPI_24_10bit_12x1_4K1K144_LVDS_180to180MHz_VER006    NO.101
	    3,			//E_PNL_SUPPORTED_LPLL_CMPI_27_8bit_4x2_4K2K60_450to600MHz_VER006    NO.102
	    3,			//E_PNL_SUPPORTED_LPLL_CMPI_27_8bit_4x2_4K2K60_300to450MHz_VER006    NO.103
	    3,			//E_PNL_SUPPORTED_LPLL_CMPI_27_8bit_4x2_4K2K60_300to300MHz_VER006    NO.104
	    2,			//E_PNL_SUPPORTED_LPLL_CMPI_27_8bit_6x1_4K2K60_350to600MHz_VER006    NO.105
	    2,			//E_PNL_SUPPORTED_LPLL_CMPI_27_8bit_6x1_4K2K60_300to350MHz_VER006    NO.106
	    2,			//E_PNL_SUPPORTED_LPLL_CMPI_27_8bit_6x1_4K2K60_300to300MHz_VER006    NO.107
	    3,			//E_PNL_SUPPORTED_LPLL_CMPI_27_8bit_8x1_4K2K60_450to600MHz_VER006    NO.108
	    3,			//E_PNL_SUPPORTED_LPLL_CMPI_27_8bit_8x1_4K2K60_300to450MHz_VER006    NO.109
	    3,			//E_PNL_SUPPORTED_LPLL_CMPI_27_8bit_8x1_4K2K60_300to300MHz_VER006    NO.110
	    2,			//E_PNL_SUPPORTED_LPLL_CMPI_27_8bit_12x1_4K2K60_300to600MHz_VER006    NO.111
	    2,			//E_PNL_SUPPORTED_LPLL_CMPI_27_8bit_12x1_4K2K60_300to300MHz_VER006    NO.112
	    4,			//E_PNL_SUPPORTED_LPLL_CMPI_27_8bit_12x1_4K1K144_180to360MHz_VER006    NO.113
	    4,			//E_PNL_SUPPORTED_LPLL_CMPI_27_8bit_12x1_4K1K144_180to180MHz_VER006    NO.114
	    2,			//E_PNL_SUPPORTED_LPLL_CMPI_24_10bit_4x2_4K2K60_350to600MHz_VER006    NO.115
	    2,			//E_PNL_SUPPORTED_LPLL_CMPI_24_10bit_4x2_4K2K60_300to350MHz_VER006    NO.116
	    2,			//E_PNL_SUPPORTED_LPLL_CMPI_24_10bit_4x2_4K2K60_300to300MHz_VER006    NO.117
	    2,			//E_PNL_SUPPORTED_LPLL_CMPI_24_10bit_8x1_4K2K60_350to600MHz_VER006    NO.118
	    2,			//E_PNL_SUPPORTED_LPLL_CMPI_24_10bit_8x1_4K2K60_300to350MHz_VER006    NO.119
	    2,			//E_PNL_SUPPORTED_LPLL_CMPI_24_10bit_8x1_4K2K60_300to300MHz_VER006    NO.120
	    2,			//E_PNL_SUPPORTED_LPLL_CMPI_24_10bit_12x1_4K2K60_300to600MHz_VER006    NO.121
	    2,			//E_PNL_SUPPORTED_LPLL_CMPI_24_10bit_12x1_4K2K60_300to300MHz_VER006    NO.122
	    4,			//E_PNL_SUPPORTED_LPLL_CMPI_24_10bit_12x1_4K1K144_280to360MHz_VER006    NO.123
	    4,			//E_PNL_SUPPORTED_LPLL_CMPI_24_10bit_12x1_4K1K144_180to280MHz_VER006    NO.124
	    4,			//E_PNL_SUPPORTED_LPLL_CMPI_24_10bit_12x1_4K1K144_180to180MHz_VER006    NO.125
	    3,			//E_PNL_SUPPORTED_LPLL_ISP_8bit_4x2_4K2K60_450to600MHz_VER006    NO.126
	    3,			//E_PNL_SUPPORTED_LPLL_ISP_8bit_4x2_4K2K60_300to450MHz_VER006    NO.127
	    3,			//E_PNL_SUPPORTED_LPLL_ISP_8bit_4x2_4K2K60_300to300MHz_VER006    NO.128
	    2,			//E_PNL_SUPPORTED_LPLL_ISP_8bit_6x1_4K2K60_350to600MHz_VER006    NO.129
	    2,			//E_PNL_SUPPORTED_LPLL_ISP_8bit_6x1_4K2K60_300to350MHz_VER006    NO.130
	    2,			//E_PNL_SUPPORTED_LPLL_ISP_8bit_6x1_4K2K60_300to300MHz_VER006    NO.131
	    2,			//E_PNL_SUPPORTED_LPLL_ISP_8bit_6x2_4K2K60_300to600MHz_VER006    NO.132
	    2,			//E_PNL_SUPPORTED_LPLL_ISP_8bit_6x2_4K2K60_300to300MHz_VER006    NO.133
	    3,			//E_PNL_SUPPORTED_LPLL_ISP_8bit_8x1_4K2K60_450to600MHz_VER006    NO.134
	    3,			//E_PNL_SUPPORTED_LPLL_ISP_8bit_8x1_4K2K60_300to450MHz_VER006    NO.135
	    3,			//E_PNL_SUPPORTED_LPLL_ISP_8bit_8x1_4K2K60_300to300MHz_VER006    NO.136
	    2,			//E_PNL_SUPPORTED_LPLL_ISP_8bit_12x1_4K2K60_300to600MHz_VER006    NO.137
	    2,			//E_PNL_SUPPORTED_LPLL_ISP_8bit_12x1_4K2K60_300to300MHz_VER006    NO.138
	    4,			//E_PNL_SUPPORTED_LPLL_ISP_8bit_12x1_4K1K144_180to360MHz_VER006    NO.139
	    4,			//E_PNL_SUPPORTED_LPLL_ISP_8bit_12x1_4K1K144_180to180MHz_VER006    NO.140
	    5,			//E_PNL_SUPPORTED_LPLL_ISP_10bit_4x2_4K2K60_400to600MHz_VER006    NO.141
	    5,			//E_PNL_SUPPORTED_LPLL_ISP_10bit_4x2_4K2K60_300to400MHz_VER006    NO.142
	    5,			//E_PNL_SUPPORTED_LPLL_ISP_10bit_4x2_4K2K60_300to300MHz_VER006    NO.143
	    5,			//E_PNL_SUPPORTED_LPLL_ISP_10bit_6x2_4K2K60_300to600MHz_VER006    NO.144
	    5,			//E_PNL_SUPPORTED_LPLL_ISP_10bit_6x2_4K2K60_300to300MHz_VER006    NO.145
	    5,			//E_PNL_SUPPORTED_LPLL_ISP_10bit_8x1_4K2K60_400to600MHz_VER006    NO.146
	    5,			//E_PNL_SUPPORTED_LPLL_ISP_10bit_8x1_4K2K60_300to400MHz_VER006    NO.147
	    5,			//E_PNL_SUPPORTED_LPLL_ISP_10bit_8x1_4K2K60_300to300MHz_VER006    NO.148
	    5,			//E_PNL_SUPPORTED_LPLL_ISP_10bit_12x1_4K2K60_300to600MHz_VER006    NO.149
	    5,			//E_PNL_SUPPORTED_LPLL_ISP_10bit_12x1_4K2K60_300to300MHz_VER006    NO.150
	    5,			//E_PNL_SUPPORTED_LPLL_ISP_10bit_12x1_4K1K144_300to360MHz_VER006    NO.151
	    5,			//E_PNL_SUPPORTED_LPLL_ISP_10bit_12x1_4K1K144_180to300MHz_VER006    NO.152
	    5,			//E_PNL_SUPPORTED_LPLL_ISP_10bit_12x1_4K1K144_180to180MHz_VER006    NO.153
	    5,			//E_PNL_SUPPORTED_LPLL_CHPI_8bit_4x2_4K2K60_450to600MHz_VER006    NO.154
	    5,			//E_PNL_SUPPORTED_LPLL_CHPI_8bit_4x2_4K2K60_300to450MHz_VER006    NO.155
	    5,			//E_PNL_SUPPORTED_LPLL_CHPI_8bit_4x2_4K2K60_300to300MHz_VER006    NO.156
	    2,			//E_PNL_SUPPORTED_LPLL_CHPI_8bit_6x1_4K2K60_300to600MHz_VER006    NO.157
	    2,			//E_PNL_SUPPORTED_LPLL_CHPI_8bit_6x1_4K2K60_300to300MHz_VER006    NO.158
	    2,			//E_PNL_SUPPORTED_LPLL_CHPI_8bit_6x2_4K2K60_300to600MHz_VER006    NO.159
	    2,			//E_PNL_SUPPORTED_LPLL_CHPI_8bit_6x2_4K2K60_300to300MHz_VER006    NO.160
	    5,			//E_PNL_SUPPORTED_LPLL_CHPI_8bit_8x1_4K2K60_450to600MHz_VER006    NO.161
	    5,			//E_PNL_SUPPORTED_LPLL_CHPI_8bit_8x1_4K2K60_300to450MHz_VER006    NO.162
	    5,			//E_PNL_SUPPORTED_LPLL_CHPI_8bit_8x1_4K2K60_300to300MHz_VER006    NO.163
	    2,			//E_PNL_SUPPORTED_LPLL_CHPI_8bit_12x1_4K2K60_300to600MHz_VER006    NO.164
	    2,			//E_PNL_SUPPORTED_LPLL_CHPI_8bit_12x1_4K2K60_300to300MHz_VER006    NO.165
	    4,			//E_PNL_SUPPORTED_LPLL_CHPI_8bit_12x1_4K1K144_180to360MHz_VER006    NO.166
	    4,			//E_PNL_SUPPORTED_LPLL_CHPI_8bit_12x1_4K1K144_180to180MHz_VER006    NO.167
	    5,			//E_PNL_SUPPORTED_LPLL_CHPI_10bit_4x2_4K2K60_350to600MHz_VER006    NO.168
	    5,			//E_PNL_SUPPORTED_LPLL_CHPI_10bit_4x2_4K2K60_300to350MHz_VER006    NO.169
	    5,			//E_PNL_SUPPORTED_LPLL_CHPI_10bit_4x2_4K2K60_300to300MHz_VER006    NO.170
	    5,			//E_PNL_SUPPORTED_LPLL_CHPI_10bit_8x1_4K2K60_350to600MHz_VER006    NO.171
	    5,			//E_PNL_SUPPORTED_LPLL_CHPI_10bit_8x1_4K2K60_300to350MHz_VER006    NO.172
	    5,			//E_PNL_SUPPORTED_LPLL_CHPI_10bit_8x1_4K2K60_300to300MHz_VER006    NO.173
	    5,			//E_PNL_SUPPORTED_LPLL_CHPI_10bit_12x1_4K2K60_500to600MHz_VER006    NO.174
	    5,			//E_PNL_SUPPORTED_LPLL_CHPI_10bit_12x1_4K2K60_300to500MHz_VER006    NO.175
	    5,			//E_PNL_SUPPORTED_LPLL_CHPI_10bit_12x1_4K2K60_300to300MHz_VER006    NO.176
	    5,			//E_PNL_SUPPORTED_LPLL_CHPI_10bit_12x1_4K1K144_280to360MHz_VER006    NO.177
	    5,			//E_PNL_SUPPORTED_LPLL_CHPI_10bit_12x1_4K1K144_180to280MHz_VER006    NO.178
	    5,			//E_PNL_SUPPORTED_LPLL_CHPI_10bit_12x1_4K1K144_180to180MHz_VER006    NO.179
	    4,			//E_PNL_SUPPORTED_LPLL_Vx1_10bit_2_lane_2K60_150to300MHz_VER006    NO.180
	    4,			//E_PNL_SUPPORTED_LPLL_Vx1_10bit_2_lane_2K60_150to150MHz_VER006    NO.181
	    2,			//E_PNL_SUPPORTED_LPLL_Vx1_10bit_4_lane_4K2K30_300to600MHz_VER006    NO.182
	    2,			//E_PNL_SUPPORTED_LPLL_Vx1_10bit_4_lane_4K2K30_300to300MHz_VER006    NO.183
	    2,			//E_PNL_SUPPORTED_LPLL_Vx1_10bit_8_lane_4K2K60_300to600MHz_VER006    NO.184
	    2,			//E_PNL_SUPPORTED_LPLL_Vx1_10bit_8_lane_4K2K60_300to300MHz_VER006    NO.185
	    4,			//E_PNL_SUPPORTED_LPLL_Vx1_10bit_8_lane_4K1K144_180to360MHz_VER006    NO.186
	    4,			//E_PNL_SUPPORTED_LPLL_Vx1_10bit_8_lane_4K1K144_180to180MHz_VER006    NO.187
	    5,			//E_PNL_SUPPORTED_LPLL_Vx1_8bit_2_lane_2K60_225to300MHz_VER006    NO.188
	    5,			//E_PNL_SUPPORTED_LPLL_Vx1_8bit_2_lane_2K60_150to225MHz_VER006    NO.189
	    5,			//E_PNL_SUPPORTED_LPLL_Vx1_8bit_2_lane_2K60_150to150MHz_VER006    NO.190
	    5,			//E_PNL_SUPPORTED_LPLL_Vx1_8bit_4_lane_4K2K30_450to600MHz_VER006    NO.191
	    5,			//E_PNL_SUPPORTED_LPLL_Vx1_8bit_4_lane_4K2K30_300to450MHz_VER006    NO.192
	    5,			//E_PNL_SUPPORTED_LPLL_Vx1_8bit_4_lane_4K2K30_300to300MHz_VER006    NO.193
	    5,			//E_PNL_SUPPORTED_LPLL_Vx1_8bit_8_lane_4K2K60_450to600MHz_VER006    NO.194
	    5,			//E_PNL_SUPPORTED_LPLL_Vx1_8bit_8_lane_4K2K60_300to450MHz_VER006    NO.195
	    5,			//E_PNL_SUPPORTED_LPLL_Vx1_8bit_8_lane_4K2K60_300to300MHz_VER006    NO.196
	    5,			//E_PNL_SUPPORTED_LPLL_Vx1_8bit_8_lane_4K1K144_225to360MHz_VER006    NO.197
	    5,			//E_PNL_SUPPORTED_LPLL_Vx1_8bit_8_lane_4K1K144_180to225MHz_VER006    NO.198
	    5,			//E_PNL_SUPPORTED_LPLL_Vx1_8bit_8_lane_4K1K144_180to180MHz_VER006    NO.199
};

 
//======version006 end======
      
#endif				//_MTK_TV_LPLL_TBL_H_
