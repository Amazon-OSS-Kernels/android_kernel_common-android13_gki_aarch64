// SPDX-License-Identifier: (GPL-2.0 OR BSD-3-Clause)
/*
 * MediaTek UART driver
 *
 * Copyright (c) 2020 MediaTek Inc.
 * Author jfy <jfy@mediatek.com>
 */

#include <clk.h>
#include <common.h>
#include <div64.h>
#include <dm.h>
#include <errno.h>
#include <serial.h>
#include <watchdog.h>
#include <asm/io.h>
#include <asm/types.h>
#include <debug_uart.h>

#if defined(CONFIG_TARGET_MT5896)
#define REG_RIU_BASE			0x1C000000
#define REG_PM_BANK			0x10100
#define REG_HK51_UART0_EN		(0x09)
#define REG_HK51_UART0_ENABLE		(0x1000)
#define REG_UART_CM4_EN			(0x7E)
#define REG_UART_CM4_ENABLE		(0x0004)
#else
#define REG_PM_BASE			0x1f000000
#define REG_PM_BANK			0xE00
#define REG_PM_OFFSET			0x9
#define REG_PM_UART_ENABLE_MSK		0x0C00
#endif

DECLARE_GLOBAL_DATA_PTR;

/**
 * struct mtk_tv_ns16550_platdata - information about a NS16550 port
 *
 * @base:		Base register address
 * @reg_width:		IO accesses size of registers (in bytes)
 * @reg_shift:		Shift size of registers (0=byte, 1=16bit, 2=32bit...)
 * @clock:		UART base clock speed in Hz
 */
struct mtk_tv_ns16550_platdata {
	unsigned long base;
	int reg_width;
	int reg_shift;
	int reg_offset;
	int clock;
	u32 fcr;
};

/*TODO: need to use reg-shift*/
#define serial_reg(iobase, addr) *((volatile unsigned short*)(iobase + ((addr)<< 2)))

void serial_write(void *iobase, u8 addr, u8 data)
{
	serial_reg((char *)iobase, addr) = data;
}

static u32 serial_read(void *iobase, u8 addr)
{
	u16 data;
	data = serial_reg((char *)iobase, addr);
	return (data);
}

#define io_serial_out(reg,val) \
          serial_write((char *)priv->regs,(char*)reg-(char *)priv->regs,val)

#define io_serial_in(reg) \
          serial_read((char *)priv->regs,(char*)reg-(char *)priv->regs)

#ifdef CONFIG_DEBUG_UART

#define io_serial_dout(reg,val) \
          serial_write((char *)priv.regs,(char*)reg-(char*)priv.regs,val)

#define io_serial_din(reg) \
          serial_read((char *)priv.regs,(char*)reg-(char*)priv.regs)

#define serial_out(reg, value) \
        serial_out_shift((char *)com_port + \
                ((char *)reg - (char *)com_port) * \
                        (1 << CONFIG_DEBUG_UART_SHIFT), \
                CONFIG_DEBUG_UART_SHIFT, value)
#define serial_in(reg) \
        serial_in_shift((char *)com_port + \
                ((char *)reg - (char *)com_port) * \
                        (1 << CONFIG_DEBUG_UART_SHIFT), \
                CONFIG_DEBUG_UART_SHIFT)
#endif

/**
 * mtk_tv_serial_regs : each register is 2 bytes width
 * rbr 0x0
 * ier 0x2
 * fcr 0x4
 * lcr 0x6
 * mcr 0x8
 * lsr 0xa
 * msr 0xc
 * usr 0xe
 * tfl 0x10
 */
struct mtk_tv_serial_regs {
	u16 rbr;
	u16 ier;
	u16 fcr;
	u16 lcr;
	u16 mcr;
	u16 lsr;
	u16 msr;
	u16 usr;
	u16 tfl;
};

#define thr rbr
#define iir fcr
#define dll rbr
#define dlm ier

#define UART_LCR_WLS_8	0x03	/* 8 bit character length */
#define UART_LCR_DLAB	0x80	/* Divisor latch access bit */

#define UART_LSR_DR	0x01		/* Data ready */
#define UART_LSR_THRE	0x20	/* Xmit holding register empty */

/*TODO the data is correct if the real baud is within 3%. */
#define MTK_TV_BAUD_ALLOW_MAX(baud)	((baud) + (baud) * 3 / 100)
#define MTK_TV_BAUD_ALLOW_MIX(baud)	((baud) - (baud) * 3 / 100)
/*TODO MTK UART SYNTH*/
#ifdef CONFIG_DEBUG_UART
static inline void _debug_uart_init(void);
#endif
struct mtk_tv_serial_priv {
	struct mtk_tv_serial_regs __iomem *regs;
	u32 clock;
	bool is_console;
};

#define MTK_TV_CONSOLE_IDX_PM_UART0	(1)
#define MTK_TV_CONSOLE_IDX_FUART0	(2)

static int mtk_tv_console_du_idx;

int _ns16550_calc_divisor(int clock, int baudrate)
{
	const unsigned int mode_x_div = 16;

	return DIV_ROUND_CLOSEST(clock, mode_x_div * baudrate);
}

static void _mtk_tv_serial_setbrg(struct mtk_tv_serial_priv *priv, int baud)
{
	u32 quot;

#if defined(CONFIG_TARGET_MT5888)
	volatile unsigned short *u16Ch;
#endif
	quot = _ns16550_calc_divisor(priv->clock,baud);

	/* set divisor */
	io_serial_out(&priv->regs->lcr,UART_LCR_WLS_8 | UART_LCR_DLAB);
	io_serial_out(&priv->regs->dll,quot & U8_MAX);
	io_serial_out(&priv->regs->dlm,(quot >> BITS_PER_BYTE) & U8_MAX);
	io_serial_out(&priv->regs->lcr,UART_LCR_WLS_8);

#if defined(CONFIG_TARGET_MT5888)
	u16Ch = (unsigned int *)(REG_PM_BASE + ((REG_PM_BANK + (REG_PM_OFFSET << 1)) << 1));
	*u16Ch |= REG_PM_UART_ENABLE_MSK;
#endif
}

static int mtk_tv_serial_setbrg(struct udevice *dev, int baudrate)
{
	struct mtk_tv_serial_priv *priv = dev_get_priv(dev);

	_mtk_tv_serial_setbrg(priv, baudrate);

	return 0;
}

static int mtk_tv_serial_putc(struct udevice *dev, const char ch)
{
	struct mtk_tv_serial_priv *priv = dev_get_priv(dev);

	if (priv->is_console) {
		// serial port is console, check console enable/disable
		if ((gd->flags & GD_FLG_DISABLE_CONSOLE)) {
			// console disabled by debug command, check register to resume console
			volatile unsigned short *u16Ch;

			if (mtk_tv_console_du_idx == MTK_TV_CONSOLE_IDX_PM_UART0) {
				// console UART IP is PM_UART0
				u16Ch = (unsigned short *)(REG_RIU_BASE +
							   ((REG_PM_BANK + (REG_UART_CM4_EN << 1)) << 1));
				if ((*u16Ch & REG_UART_CM4_ENABLE))
					// sboot console is PM_UART0 and register setting resumed
					gd->flags &= ~GD_FLG_DISABLE_CONSOLE;
				else
					// console disable by debug command and not resumed by register setting
					return 0;
			} else if (mtk_tv_console_du_idx == MTK_TV_CONSOLE_IDX_FUART0) {
				// console UART IP is FUART0
				u16Ch = (unsigned short *)(REG_RIU_BASE +
							   ((REG_PM_BANK + (REG_HK51_UART0_EN << 1)) << 1));
				if (!(*u16Ch & REG_HK51_UART0_ENABLE))
					// sboot console is PM_UART0 and register setting resumed
					gd->flags &= ~GD_FLG_DISABLE_CONSOLE;
				else
					// console disable by debug command and not resumed by register setting
					return 0;
			} else
				// console UART IP unknown
				return 0;
		}

		if ((gd->flags & GD_FLG_DISABLE_CONSOLE)) {
			// console disable by debug command and not resumed by register setting
			if (ch == '\n')
				WATCHDOG_RESET();
			return 0;
		}

		if (gd->flags & GD_FLG_SILENT) {
			// console disable by parsing .ini, never resume console
			if (ch == '\n')
				WATCHDOG_RESET();
			return 0;
		}
	}

	if (!(io_serial_in(&priv->regs->lsr) & UART_LSR_THRE))
		return -EAGAIN;

	io_serial_out(&priv->regs->thr, ch);

	if (ch == '\n')
		WATCHDOG_RESET();

	return 0;
}

#define CON_RESCUE_KEY		"00112233"
#define CON_RESCUE_KEY_LEN	(sizeof(CON_RESCUE_KEY) - 1) // eliminate null terminal
static unsigned char con_rescue_keyword[CON_RESCUE_KEY_LEN] = {0};
static unsigned char con_rescue_keypool[CON_RESCUE_KEY_LEN] = {0};
static u32 con_rescue_keyidx;

static int mtk_tv_serial_getc(struct udevice *dev)
{
	struct mtk_tv_serial_priv *priv = dev_get_priv(dev);

	if (priv->is_console) {
		// serial port is console, check console enable/disable
		if ((gd->flags & GD_FLG_DISABLE_CONSOLE)) {
			// console disabled by debug command, check register to resume console
			volatile unsigned short *u16Ch;

			if (mtk_tv_console_du_idx == MTK_TV_CONSOLE_IDX_PM_UART0) {
				// console UART IP is PM_UART0
				u16Ch = (unsigned short *)(REG_RIU_BASE +
							   ((REG_PM_BANK + (REG_UART_CM4_EN << 1)) << 1));
				if ((*u16Ch & REG_UART_CM4_ENABLE))
					// sboot console is PM_UART0 and register setting resumed
					gd->flags &= ~GD_FLG_DISABLE_CONSOLE;
				else
					// console disable by debug command and not resumed by register setting
					return 0;
			} else if (mtk_tv_console_du_idx == MTK_TV_CONSOLE_IDX_FUART0) {
				// console UART IP is FUART0
				u16Ch = (unsigned short *)(REG_RIU_BASE +
							   ((REG_PM_BANK + (REG_HK51_UART0_EN << 1)) << 1));
				if (!(*u16Ch & REG_HK51_UART0_ENABLE))
					// sboot console is PM_UART0 and register setting resumed
					gd->flags &= ~GD_FLG_DISABLE_CONSOLE;
				else
					// console disable by debug command and not resumed by register setting
					return 0;
			} else
				// console UART IP unknown
				return 0;
		}

		if ((gd->flags & GD_FLG_DISABLE_CONSOLE))
			// console disable by debug command and not resumed by register setting
			return 0;

		if (gd->flags & GD_FLG_SILENT) {
			// console disable by parsing .ini, only check password for console resume
			int i, ch;

			if (!(io_serial_in(&priv->regs->lsr) & UART_LSR_DR))
				return -EAGAIN;

			// password string by reading io_serial_in(&priv->regs->rbr);
			ch = io_serial_in(&priv->regs->rbr);

			con_rescue_keypool[con_rescue_keyidx++] = (u8)(ch & U8_MAX);
			con_rescue_keyidx %= CON_RESCUE_KEY_LEN;

			// password string checking
			for (i = 0; i < CON_RESCUE_KEY_LEN; i++)
				if (con_rescue_keypool[(con_rescue_keyidx + i) %
						       CON_RESCUE_KEY_LEN] !=
				    con_rescue_keyword[i])
					break;

			if (i >= CON_RESCUE_KEY_LEN) {
				gd->flags &= ~GD_FLG_SILENT;
				printf("console restore, press enter to continue...\n");
			}

			// only return enter key for abort autoboot in console silent
			if (ch == '\r' || ch == '\n')
				return ch;

			return 0;
		}
	}

	if (!(io_serial_in(&priv->regs->lsr) & UART_LSR_DR))
		return -EAGAIN;

	return io_serial_in(&priv->regs->rbr);
}

static int mtk_tv_serial_pending(struct udevice *dev, bool input)
{
	struct mtk_tv_serial_priv *priv = dev_get_priv(dev);

	if (priv->is_console) {
		// serial port is console, check console enable/disable
		if ((gd->flags & GD_FLG_DISABLE_CONSOLE)) {
			// console disabled by debug command, check register to resume console
			volatile unsigned short *u16Ch;

			if (mtk_tv_console_du_idx == MTK_TV_CONSOLE_IDX_PM_UART0) {
				// console UART IP is PM_UART0
				u16Ch = (unsigned short *)(REG_RIU_BASE +
							   ((REG_PM_BANK + (REG_UART_CM4_EN << 1)) << 1));
				if ((*u16Ch & REG_UART_CM4_ENABLE))
					// sboot console is PM_UART0 and register setting resumed
					gd->flags &= ~GD_FLG_DISABLE_CONSOLE;
				else
					// console disable by debug command and not resumed by register setting
					return 0;
			} else if (mtk_tv_console_du_idx == MTK_TV_CONSOLE_IDX_FUART0) {
				// console UART IP is FUART0
				u16Ch = (unsigned short *)(REG_RIU_BASE +
							   ((REG_PM_BANK + (REG_HK51_UART0_EN << 1)) << 1));
				if (!(*u16Ch & REG_HK51_UART0_ENABLE))
					// sboot console is PM_UART0 and register setting resumed
					gd->flags &= ~GD_FLG_DISABLE_CONSOLE;
				else
					// console disable by debug command and not resumed by register setting
					return 0;
			} else
				// console UART IP unknown
				return 0;
		}
	}

	if (input)
		return (io_serial_in(&priv->regs->lsr) & UART_LSR_DR) ? 1 : 0;
	else
		return (io_serial_in(&priv->regs->lsr) & UART_LSR_THRE) ? 0 : 1;
}

static int mtk_tv_serial_probe(struct udevice *dev)
{
	struct mtk_tv_serial_priv *priv = dev_get_priv(dev);

	/* Disable interrupt */
	io_serial_out(&priv->regs->ier,0);

	if ((gd->flags & GD_FLG_RELOC)) {
		/* Rescue console onoff, cannot execute before relocate */
		memcpy(con_rescue_keyword, CON_RESCUE_KEY, sizeof(con_rescue_keyword));
		memset(con_rescue_keypool, U8_MAX, sizeof(con_rescue_keypool));
		con_rescue_keyidx = 0;
	}

	return 0;
}

static int mtk_tv_serial_ofdata_to_platdata(struct udevice *dev)
{
	struct mtk_tv_serial_priv *priv = dev_get_priv(dev);
	fdt_addr_t addr;

	/*
	 * Due to console may different in chip family, check console UART by previous booting stage
	 * register setting. Only console UART needs this checking.
	 */
	if (dev_read_bool(dev, "console-uart")) {
#define MULTIPLE_ADDR_PROP_SIZE		(8)
		fdt_addr_t addr1, addr2;
		int addr_prop_size;
		/*
		 * There will be two addresses(values in property "reg") before DTB overlay,
		 * UART address decision by checking sboot console selection.
		 * No dicision making after DTB overlay since there will be only one address.
		 */
		addr1 = dev_read_addr_index(dev, 0);
		addr_prop_size = dev_read_size(dev, "reg") / sizeof(u32);

		if (addr_prop_size < MULTIPLE_ADDR_PROP_SIZE)
			// only one address
			addr = addr1;
		else {
			// two addresses, use address by checking sboot setting
			volatile unsigned short *reg;

			addr2 = dev_read_addr_index(dev, 1);

			reg = (unsigned short *)(REG_RIU_BASE +
						 ((REG_PM_BANK + (REG_UART_CM4_EN << 1)) << 1));
			if ((*reg & REG_UART_CM4_ENABLE))
				// sboot console is PM_UART0, use address index 1
				addr = addr2;
			else
				// sboot console is FUART0, use address index 0
				addr = addr1;
		}

		priv->is_console = true;
	} else {
		addr = dev_read_addr(dev);
		priv->is_console = false;
	}

	if (addr == FDT_ADDR_T_NONE)
		return -EINVAL;

	priv->regs = map_physmem(addr, 0, MAP_NOCACHE);

	if (!priv->clock)
		priv->clock = dev_read_u32_default(dev, "clock-frequency", 0);

	if (!priv->clock) {
		return -EINVAL;
	}

	return 0;
}

static const struct dm_serial_ops mtk_tv_serial_ops = {
	.putc = mtk_tv_serial_putc,
	.pending = mtk_tv_serial_pending,
	.getc = mtk_tv_serial_getc,
	.setbrg = mtk_tv_serial_setbrg,
};

static const struct udevice_id mtk_tv_serial_ids[] = {
	{ .compatible = "mediatek_tv,ns16550" },
	{ .compatible = "mediatek_tv,uart" },
	{ }
};

U_BOOT_DRIVER(serial) = {
	.name = "serial",
	.id = UCLASS_SERIAL,
	.of_match = mtk_tv_serial_ids,
	.ofdata_to_platdata = mtk_tv_serial_ofdata_to_platdata,
	.platdata_auto_alloc_size = sizeof(struct mtk_tv_ns16550_platdata),
	.priv_auto_alloc_size = sizeof(struct mtk_tv_serial_priv),
	.probe = mtk_tv_serial_probe,
	.ops = &mtk_tv_serial_ops,
#if !CONFIG_IS_ENABLED(OF_CONTROL)
	.flags = DM_FLAG_PRE_RELOC,
#endif
};

int serial_disbale (void)
{
#ifdef CONFIG_TARGET_MT5896
	volatile unsigned short *u16Ch;

	u16Ch = (unsigned short *)(REG_RIU_BASE +
				   ((REG_PM_BANK + (REG_UART_CM4_EN << 1)) << 1));
	if ((*u16Ch & REG_UART_CM4_ENABLE)) {
		// sboot console is PM_UART0
		dev_err(NULL, "Resume console by reg 0x%X 0x%X 0x%X\n", REG_PM_BANK,
			REG_UART_CM4_EN, REG_UART_CM4_ENABLE);
		u16Ch = (unsigned short *)(REG_RIU_BASE +
					   ((REG_PM_BANK + (REG_UART_CM4_EN << 1)) << 1));
		*u16Ch &= ~REG_UART_CM4_ENABLE;

		mtk_tv_console_du_idx = MTK_TV_CONSOLE_IDX_PM_UART0;
	} else {
		// sboot console is FUART0
		dev_err(NULL, "Resume console by reg 0x%X 0x%X 0x%X\n", REG_PM_BANK,
			REG_HK51_UART0_EN, REG_HK51_UART0_ENABLE);
		u16Ch = (unsigned short *)(REG_RIU_BASE +
					   ((REG_PM_BANK + (REG_HK51_UART0_EN << 1)) << 1));
		*u16Ch |= REG_HK51_UART0_ENABLE;

		mtk_tv_console_du_idx = MTK_TV_CONSOLE_IDX_FUART0;
	}

	gd->flags |= GD_FLG_DISABLE_CONSOLE;
#endif

#ifdef CONFIG_TARGET_MT5888
	volatile unsigned int *u32Ch;
	u32Ch = (unsigned int *)(REG_PM_BASE + ((REG_PM_BANK + (REG_PM_OFFSET << 1)) << 1));
	*u32Ch &= ~REG_PM_UART_ENABLE_MSK;
#endif
    return 0;
}

#ifdef CONFIG_DEBUG_UART

#include <debug_uart.h>

static inline void _debug_uart_init(void)
{
	struct mtk_tv_serial_priv priv;

	priv.regs = (void *) CONFIG_DEBUG_UART_BASE;
	priv.clock = CONFIG_DEBUG_UART_CLOCK;

	io_serial_dout(&priv.regs->ier,0);

	_mtk_tv_serial_setbrg(&priv, CONFIG_BAUDRATE);
}

static inline void _debug_uart_putc(int ch)
{
	struct mtk_tv_serial_priv priv;

	priv.regs = (void *) CONFIG_DEBUG_UART_BASE;
	priv.clock = CONFIG_DEBUG_UART_CLOCK;

	while (!(io_serial_din(&priv.regs->lsr)& UART_LSR_THRE)) ///*UART_REG(0x1f201300,0xa)
		;

	io_serial_dout(&priv.regs->thr,ch);
}

DEBUG_UART_FUNCS

#endif
