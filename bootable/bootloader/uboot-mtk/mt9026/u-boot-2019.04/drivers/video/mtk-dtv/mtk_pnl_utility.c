// SPDX-License-Identifier: (GPL-2.0 OR BSD-3-Clause)
/*
 * MediaTek Panel driver
 *
 * Copyright (c) 2020 MediaTek Inc.
 * Author Yu-Jen.Huang <yu-jen.huang@mediatek.com>
 */
#include <common.h>
#include <asm/io.h>
#include "coda/hwreg_common.h"
#include "mtk_pnl_utility.h"

uint32_t reg_base = 0x1C000000;

uint32_t __u4IO32AccessFld(uint8_t write, uint32_t tmp32, uint32_t val32, uint32_t fld)
{
    uint32_t t = 0;

    switch (Fld_ac(fld)) {
        case AC_FULLB0:
        case AC_FULLB1:
        case AC_FULLB2:
        case AC_FULLB3:
            if (write == 1)
                t = (tmp32&(~((uint32_t)0xFF<<(8*(Fld_ac(fld)-AC_FULLB0)))))|((val32&0xFF)<<(8*(Fld_ac(fld)-AC_FULLB0)));
            else
                t = (tmp32&((uint32_t)0xFF<<(8*(Fld_ac(fld)-AC_FULLB0)))) >> (8*(Fld_ac(fld)-AC_FULLB0));
            break;
        case AC_FULLW10:
        case AC_FULLW21:
        case AC_FULLW32:
            if (write == 1)
                t = (tmp32&(~((uint32_t)0xFFFF<<(8*(Fld_ac(fld)-AC_FULLW10))))) | ((val32&0xFFFF)<<(8*(Fld_ac(fld)-AC_FULLW10)));
            else
                t = (tmp32&(((uint32_t)0xFFFF<<(8*(Fld_ac(fld)-AC_FULLW10))))) >> (8*(Fld_ac(fld)-AC_FULLW10));
            break;
	case AC_FULLDW:
            t = val32;
            break;
	case AC_MSKB0:
	case AC_MSKB1:
	case AC_MSKB2:
	case AC_MSKB3:
            if (write == 1)
                t = (tmp32&(~(((((uint32_t)1<<Fld_wid(fld))-1)<<Fld_shift(fld)))))|(((val32&(((uint32_t)1<<Fld_wid(fld))-1))<< Fld_shift(fld)));
	    else
                t = (tmp32&(((((uint32_t)1<<Fld_wid(fld))-1)<<Fld_shift(fld))))>>Fld_shift(fld);
            break;
	case AC_MSKW10:
	case AC_MSKW21:
	case AC_MSKW32:
            if (write == 1)
                t = (tmp32&(~(((((uint32_t)1<<Fld_wid(fld))-1)<< Fld_shift(fld)))))|(((val32&(((uint32_t)1<<Fld_wid(fld))-1))<<Fld_shift(fld)));
            else
                t = (tmp32&(((((uint32_t)1<<Fld_wid(fld))-1)<<Fld_shift(fld)))) >>	Fld_shift(fld);
            break;
	case AC_MSKDW:
            if (write == 1)
                t = (tmp32&(~(((((uint32_t)1<<Fld_wid(fld))-1)<< Fld_shift(fld))))) | (((val32&(((uint32_t)1<<Fld_wid(fld))-1))<< Fld_shift(fld)));
            else
                t = (tmp32&(((((uint32_t)1<<Fld_wid(fld))-1)<< Fld_shift(fld)))) >> Fld_shift(fld);
            break;
        default:
            break;
        }
    return t;
}


void MTK_W2BYTEMSK(uint32_t addr, uint32_t value, uint32_t fld)
{
    uint32_t tmp = 0;
    unsigned long addrs;
    volatile void __iomem *reg;

    addrs = reg_base + addr;
    reg = (void *)addrs;
    tmp = __u4IO32AccessFld(1, readl(reg), value, fld);
    writel(tmp, reg);
}

uint32_t MTK_R2BYTEMSK(uint32_t addr, uint32_t fld)
{
    uint32_t tmp = 0;
    volatile void __iomem *reg;
    unsigned long addrs;

    addrs = reg_base + addr;
    reg = (void *)addrs;
    tmp = __u4IO32AccessFld(0, readl(reg), 0, fld);
    return tmp;
}
