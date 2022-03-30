// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2009 SAMSUNG Electronics
 * Minkyu Kang <mk7.kang@samsung.com>
 * Heungjun Kim <riverful.kim@samsung.com>
 *
 * based on drivers/serial/s3c64xx.c
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <fdtdec.h>
#include <linux/compiler.h>
#include <asm/io.h>
#include <asm/arch/clk.h>
#include <asm/arch/uart.h>
#include <serial.h>
#include <clk.h>

DECLARE_GLOBAL_DATA_PTR;

#define RX_FIFO_COUNT_SHIFT	0
#define RX_FIFO_COUNT_MASK	(0xff << RX_FIFO_COUNT_SHIFT)
#define RX_FIFO_FULL		(1 << 8)
#define TX_FIFO_COUNT_SHIFT	16
#define TX_FIFO_COUNT_MASK	(0xff << TX_FIFO_COUNT_SHIFT)
#define TX_FIFO_FULL		(1 << 24)

/* Information about a serial port */
struct s5p_serial_platdata {
	struct s5p_uart *reg;  /* address of registers in physical memory */
	u8 port_id;     /* uart port number */
};

/*
 * The coefficient, used to calculate the baudrate on S5P UARTs is
 * calculated as
 * C = UBRDIV * 16 + number_of_set_bits_in_UDIVSLOT
 * however, section 31.6.11 of the datasheet doesn't recomment using 1 for 1,
 * 3 for 2, ... (2^n - 1) for n, instead, they suggest using these constants:
 */
static const int udivslot[] = {
	0,
	0x0080,
	0x0808,
	0x0888,
	0x2222,
	0x4924,
	0x4a52,
	0x54aa,
	0x5555,
	0xd555,
	0xd5d5,
	0xddd5,
	0xdddd,
	0xdfdd,
	0xdfdf,
	0xffdf,
};

static void __maybe_unused s5p_serial_init(struct s5p_uart *uart)
{
	/* enable FIFOs, auto clear Rx FIFO */
	writel(0x3, &uart->ufcon);
	writel(0, &uart->umcon);
	/* 8N1 */
	writel(0x3, &uart->ulcon);
	/* No interrupts, no DMA, pure polling */
	writel(0x245, &uart->ucon);
}

static void __maybe_unused s5p_serial_baud(struct s5p_uart *uart, uint uclk,
					   int baudrate)
{
	u32 val;

	val = uclk / baudrate;

	writel(val / 16 - 1, &uart->ubrdiv);

	if (s5p_uart_divslot())
		writew(udivslot[val % 16], &uart->rest.slot);
	else
		writeb(val % 16, &uart->rest.value);
}

#ifndef CONFIG_SPL_BUILD
int s5p_serial_setbrg(struct udevice *dev, int baudrate)
{
	struct s5p_serial_platdata *plat = dev->platdata;
	struct s5p_uart *const uart = plat->reg;
	u32 uclk;

#ifdef CONFIG_CLK_EXYNOS
	struct clk clk;
	u32 ret;

	ret = clk_get_by_index(dev, 1, &clk);
	if (ret < 0)
		return ret;
	uclk = clk_get_rate(&clk);
#else
	uclk = get_uart_clk(plat->port_id);
#endif

	s5p_serial_baud(uart, uclk, baudrate);

	return 0;
}

static int s5p_serial_probe(struct udevice *dev)
{
	struct s5p_serial_platdata *plat = dev->platdata;
	struct s5p_uart *const uart = plat->reg;

	s5p_serial_init(uart);

	return 0;
}

static int serial_err_check(const struct s5p_uart *const uart, int op)
{
	unsigned int mask;

	/*
	 * UERSTAT
	 * Break Detect	[3]
	 * Frame Err	[2] : receive operation
	 * Parity Err	[1] : receive operation
	 * Overrun Err	[0] : receive operation
	 */
	if (op)
		mask = 0x8;
	else
		mask = 0xf;

	return readl(&uart->uerstat) & mask;
}

static int s5p_serial_getc(struct udevice *dev)
{
	struct s5p_serial_platdata *plat = dev->platdata;
	struct s5p_uart *const uart = plat->reg;

	if (!(readl(&uart->ufstat) & RX_FIFO_COUNT_MASK))
		return -EAGAIN;

	serial_err_check(uart, 0);
	return (int)(readb(&uart->urxh) & 0xff);
}

static int s5p_serial_putc(struct udevice *dev, const char ch)
{
	struct s5p_serial_platdata *plat = dev->platdata;
	struct s5p_uart *const uart = plat->reg;

	if (readl(&uart->ufstat) & TX_FIFO_FULL)
		return -EAGAIN;

	writeb(ch, &uart->utxh);
	serial_err_check(uart, 1);

	return 0;
}

static int s5p_serial_pending(struct udevice *dev, bool input)
{
	struct s5p_serial_platdata *plat = dev->platdata;
	struct s5p_uart *const uart = plat->reg;
	uint32_t ufstat = readl(&uart->ufstat);

	if (input)
		return (ufstat & RX_FIFO_COUNT_MASK) >> RX_FIFO_COUNT_SHIFT;
	else
		return (ufstat & TX_FIFO_COUNT_MASK) >> TX_FIFO_COUNT_SHIFT;
}

static int s5p_serial_ofdata_to_platdata(struct udevice *dev)
{
	struct s5p_serial_platdata *plat = dev->platdata;
	fdt_addr_t addr;

	addr = devfdt_get_addr(dev);
	if (addr == FDT_ADDR_T_NONE)
		return -EINVAL;

	plat->reg = (struct s5p_uart *)addr;
	plat->port_id = fdtdec_get_int(gd->fdt_blob, dev_of_offset(dev),
					"id", dev->seq);
	return 0;
}

static const struct dm_serial_ops s5p_serial_ops = {
	.putc = s5p_serial_putc,
	.pending = s5p_serial_pending,
	.getc = s5p_serial_getc,
	.setbrg = s5p_serial_setbrg,
};

static const struct udevice_id s5p_serial_ids[] = {
	{ .compatible = "samsung,exynos4210-uart" },
	{ }
};

U_BOOT_DRIVER(serial_s5p) = {
	.name	= "serial_s5p",
	.id	= UCLASS_SERIAL,
	.of_match = s5p_serial_ids,
	.ofdata_to_platdata = s5p_serial_ofdata_to_platdata,
	.platdata_auto_alloc_size = sizeof(struct s5p_serial_platdata),
	.probe = s5p_serial_probe,
	.ops	= &s5p_serial_ops,
};
#endif

#ifdef CONFIG_DEBUG_UART_S5P

#include <debug_uart.h>

static inline void _debug_uart_init(void)
{
	struct s5p_uart *uart = (struct s5p_uart *)CONFIG_DEBUG_UART_BASE;

	s5p_serial_init(uart);
	s5p_serial_baud(uart, CONFIG_DEBUG_UART_CLOCK, CONFIG_BAUDRATE);
}

static inline void _debug_uart_putc(int ch)
{
	struct s5p_uart *uart = (struct s5p_uart *)CONFIG_DEBUG_UART_BASE;

	while (readl(&uart->ufstat) & TX_FIFO_FULL);

	writeb(ch, &uart->utxh);
}

DEBUG_UART_FUNCS

#endif
