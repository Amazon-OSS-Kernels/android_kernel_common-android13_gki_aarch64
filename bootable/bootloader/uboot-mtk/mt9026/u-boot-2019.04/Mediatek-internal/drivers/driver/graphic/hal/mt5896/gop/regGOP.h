/* SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause) */
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

#ifndef _REG_GOP_H_
#define _REG_GOP_H_

#include "GOP0_BKA4DE.h"
#include "GOP1_BKA4E4.h"
#include "GOP2_BKA4E8.h"
#include "GOP3_BKA4EC.h"
#include "GOPC_BKA4D3.h"
#include "GOPG_BKA4D9.h"
#include "GOP0_HVSP_BKA4E1.h"
#include "SCDISP_BKA4FA.h"
#include "ckgen01.h"
#include "modosd1.h"
#include "modosd2.h"
#include "OSD_ABF_LB_BKA4DD.h"

//-------------------------------------------------------------------------------------------------
//  Macro and Define
//-------------------------------------------------------------------------------------------------
/*********************************************
---------------- AC_FULL/MSK ----------------
  B3[31:24] | B2[23:16] | B1[15:8] | B0[7:0]
  W32[31:16] | W21[23:8] | W10 [15:0]
  DW[31:0]
---------------------------------------------
*********************************************/
#define AC_FULLB0           1
#define AC_FULLB1           2
#define AC_FULLB2           3
#define AC_FULLB3           4
#define AC_FULLW10          5
#define AC_FULLW21          6
#define AC_FULLW32          7
#define AC_FULLDW           8
#define AC_MSKB0            9
#define AC_MSKB1            10
#define AC_MSKB2            11
#define AC_MSKB3            12
#define AC_MSKW10           13
#define AC_MSKW21           14
#define AC_MSKW32           15
#define AC_MSKDW            16

/*********************************************
-------------------- Fld --------------------
    wid[31:16] | shift[15:8] | ac[7:0]
---------------------------------------------
*********************************************/
#define Fld(wid,shft,ac)    (((uint32_t)wid<<16)|(shft<<8)|ac)
#define Fld_wid(fld)        (uint8_t)((fld)>>16)
#define Fld_shift(fld)       (uint8_t)((fld)>>8)
#define Fld_ac(fld)         (uint8_t)((fld))

//----------------------------------------------------------------------------
// HW IP Reg Base Adr
//----------------------------------------------------------------------------
#define REG_BASE	(0x1C000000)
#define GOP0_BNK_ADDR_OFST	(0x0)
#define GOP1_BNK_ADDR_OFST	(0xC00)
#define GOP2_BNK_ADDR_OFST	(0x1400)
#define GOP3_BNK_ADDR_OFST	(0x1C00)
#define GOP4_BNK_ADDR_OFST	(-0x1600)

#define GOP_DST_OP	(0)
#define GOP_DST_VG	(1)
#define GOP_DST_IP	(2)

#define REG_GOP_HVSP1_RATIO_LSB_MSK	(0xFFFF)
#define REG_GOP_HVSP1_HRATIO_MSB_MSK	(0xFFF0000)
#define REG_GOP_HVSP1_VRATIO_MSB_MSK	(0xFF0000)
#define REG_GOP_HVSP2_RATIO_LSB_MSK	(0xFFFF)
#define REG_GOP_HVSP2_HRATIO_MSB_MSK	(0xFFF0000)
#define REG_GOP_HVSP2_VRATIO_MSB_MSK	(0xFF0000)

#define REG_GOP_NONTRANS_COUNT_SHITF	(16)

#define REG_GOPG_DB_TRIGGER	BIT(5)
#endif // _REG_GOP_H_
