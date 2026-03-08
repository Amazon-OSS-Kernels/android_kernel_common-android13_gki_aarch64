// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2018 MediaTek Inc.
 */

#include <common.h>
#include <dm.h>
#include <wdt.h>
#include <dm/uclass-internal.h>
#include <asm/io.h>

int arch_misc_init(void)
{
    struct udevice *wdt;
    int ret;

    ret = uclass_first_device_err(UCLASS_WDT, &wdt);
    if (!ret)
        wdt_stop(wdt);

    return 0;
}

int arch_cpu_init(void)
{
    /*MTK temporary fix*/
    gd->cpu_clk = 0xB71B00;
    icache_enable();

    return 0;
}

/*TODO: need refactor for register base from ROM table*/
#if defined(CONFIG_TARGET_MT5888)
#define BASE_REG_ADDR   0x1F000000
#define RESET_REG_ADDR  0x2E5C
#elif defined(CONFIG_TARGET_MT5896)
#define BASE_REG_ADDR   0x1C000000
#define RESET_REG_ADDR  0x1095C
#endif

#define RIU_REG(addr) \
	((void __iomem *)(BASE_REG_ADDR) + (((addr) << 1) - ((addr) & 1)))

void reset_cpu(ulong addr)
{
    writeb(0x79,RIU_REG(RESET_REG_ADDR));
    return;
}

