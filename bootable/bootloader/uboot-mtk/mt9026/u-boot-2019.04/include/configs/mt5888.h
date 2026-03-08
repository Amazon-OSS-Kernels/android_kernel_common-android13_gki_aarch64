/*
 * (C) Copyright 2010
 * Texas Instruments Incorporated.
 * Steve Sakoman  <steve@sakoman.com>
 *
 * Configuration settings for the TI OMAP4 Panda board.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include <linux/sizes.h>

#define CONFIG_SYS_NONCACHED_MEMORY     SZ_1M

/* Display CPU and Board Info */
#define CONFIG_DISPLAY_CPUINFO          1
#define CONFIG_DISPLAY_BOARDINFO        1

/* Generic Interrupt Controller */
#define CONFIG_GICV2
#define GICD_BASE                   0x16001000
#define GICC_BASE                   0x16002000

#define CONFIG_CMDLINE_TAG              1   /* enable passing of ATAGs */
#define CONFIG_SETUP_MEMORY_TAGS        1
#define CONFIG_INITRD_TAG               1
#define CONFIG_REVISION_TAG             1

/*
 * Size of malloc() pool
 * Total Size Environment - 64k
 */
#define UBOOT_HEAP_SIZE                 (0x1000000)
#define CONFIG_SYS_MALLOC_LEN           (CONFIG_ENV_SIZE + UBOOT_HEAP_SIZE) // 64K + 16M

/*
 * serial port - NS16550 compatible
 */
#define V_NS16550_CLK                   48000000
#define CONFIG_SYS_NS16550_REG_SIZE     (-4)
#define CONFIG_SYS_NS16550_CLK          V_NS16550_CLK
#define CONFIG_CONS_INDEX               1
#define CONFIG_SYS_NS16550_COM3         0x1F201300

#define CONFIG_ENV_SIZE                 0x10000
#define CONFIG_ENV_SECT_SIZE            0x20000 // 128KB
#define CONFIG_ENV_OFFSET               0x500000

/* Environment */
#define CONFIG_SYS_MMC_ENV_DEV          0

#define CONFIG_SYS_BAUDRATE_TABLE       {4800, 9600, 19200, 38400, 57600, 115200}

#ifdef CONFIG_CMD_NAND
#define MTDIDS_DEFAULT      "nand0=edb64M-nand"
#define MTDPARTS_DEFAULT    "mtdparts=nand:0x40000(NPT),0x40000(KL_BP),0x500000(KL),0x7500000(UBI)"
#else
#define MTDPARTS_DEFAULT    ""
#endif

#define CONFIG_ENV_OVERWRITE
#define CONFIG_EXTRA_ENV_SETTINGS \
        "console=ttyS2,115200\0" \
        "fdt_high=0x23000000\0" \
        "initrd_high=0x24000000\0" \
        "dtboaddr=0x27000000\0" \
        "ENTER_STANDBY=0\0" \
        "QHB_FLAG=0\0" \
        "loglevel=3\0" \

#define CONFIG_AUTO_COMPLETE            1
#define CONFIG_CMDLINE_EDITING          1

/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_LONGHELP             /* undef to save memory */
#define CONFIG_SYS_CBSIZE               2048
/* Print Buffer Size */
#define CONFIG_SYS_PBSIZE               (CONFIG_SYS_CBSIZE + \
                                        sizeof(CONFIG_SYS_PROMPT) + 16)
#define CONFIG_SYS_MAXARGS              64
/* Boot Argument Buffer Size */
#define CONFIG_SYS_BARGSIZE             (CONFIG_SYS_CBSIZE)

/* Default load address */
#define CONFIG_SYS_LOAD_ADDR            0x20000000
#define CONFIG_LOADADDR                 0x25000000

/* DTB pending size */
#define CONFIG_SYS_FDT_PAD              0x20000

/* Use General purpose timer 1 */
#define CONFIG_SYS_TIMERBASE            0x16000600
#define CONFIG_SYS_PTV                  2    /* Divisor: 2^(PTV+1) => 8 */
#define CONFIG_SYS_BOOTPARAMS_LEN       (64*1024)

/*
 * SDRAM Memory Map
 * Even though we use two CS all the memory
 * is mapped to one contiguous block
 */
#define CONFIG_SYS_SDRAM_BASE           0x20000000
#define CONFIG_SYS_INIT_RAM_SIZE        (CONFIG_DRAM_SIZE_MB_RELOCATE * 0x100000)
#define CONFIG_SYS_INIT_SP_ADDR         (CONFIG_SYS_SDRAM_BASE + \
                                         CONFIG_SYS_INIT_RAM_SIZE - \
                                         GENERATED_GBL_DATA_SIZE)

#define CONFIG_SUPPORT_EMMC_BOOT	/* eMMC specific */

#endif /* __CONFIG_H */

