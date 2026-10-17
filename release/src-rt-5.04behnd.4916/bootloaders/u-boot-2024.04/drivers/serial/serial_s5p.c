// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2009 SAMSUNG Electronics
 * Minkyu Kang <mk7.kang@samsung.com>
 * Heungjun Kim <riverful.kim@samsung.com>
 *
 * based on drivers/serial/s3c64xx.c
 */

#include <dm.h>
#include <errno.h>
#include <fdtdec.h>
#include <asm/global_data.h>
#include <linux/compiler.h>
#include <asm/io.h>
#if !IS_ENABLED(CONFIG_ARCH_APPLE)
#include <asm/arch/clk.h>
#endif
#include <asm/arch/uart.h>
#include <serial.h>
#include <clk.h>

enum {
	PORT_S5P = 0,
	PORT_S5L
};

#define UFCON_FIFO_EN		BIT(0)
#define UFCON_RX_FIFO_RESET	BIT(1)
#define UMCON_RESET_VAL		0x0
#define ULCON_WORD_8_BIT	0x3
#define UCON_RX_IRQ_OR_POLLING	BIT(0)
#define UCON_TX_IRQ_OR_POLLING	BIT(2)
#define UCON_RX_ERR_IRQ_EN	BIT(6)
#define UCON_TX_IRQ_LEVEL	BIT(9)

#define S5L_RX_FIFO_COUNT_SHIFT	0
#define S5L_RX_FIFO_COUNT_MASK	(0xf << S5L_RX_FIFO_COUNT_SHIFT)
#define S5L_RX_FIFO_FULL	BIT(8)
#define S5L_TX_FIFO_COUNT_SHIFT	4
#define S5L_TX_FIFO_COUNT_MASK	(0xf << S5L_TX_FIFO_COUNT_SHIFT)
#define S5L_TX_FIFO_FULL	BIT(9)

#define S5P_RX_FIFO_COUNT_SHIFT	0
#define S5P_RX_FIFO_COUNT_MASK	(0xff << S5P_RX_FIFO_COUNT_SHIFT)
#define S5P_RX_FIFO_FULL	BIT(8)
#define S5P_TX_FIFO_COUNT_SHIFT	16
#define S5P_TX_FIFO_COUNT_MASK	(0xff << S5P_TX_FIFO_COUNT_SHIFT)
#define S5P_TX_FIFO_FULL	BIT(24)

/* Information about a serial port */
struct s5p_serial_plat {
	struct s5p_uart *reg;	/* address of registers in physical memory */
	u8 reg_width;		/* register width */
	u8 port_id;		/* uart port number */
	u8 rx_fifo_count_shift;
	u8 tx_fifo_count_shift;
	u32 rx_fifo_count_mask;
	u32 tx_fifo_count_mask;
	u32 rx_fifo_full;
	u32 tx_fifo_full;
};

/*
 * The coefficient, used to calculate the baudrate on S5P UARTs is
 * calculated as
 * C = UBRDIV * 16 + number_of_set_bits_in_UDIVSLOT
 * however, section 31.6.11 of the datasheet doesn't recommend using 1 for 1,
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
	/* Enable FIFOs, auto clear Rx FIFO */
	writel(UFCON_FIFO_EN | UFCON_RX_FIFO_RESET, &uart->ufcon);
	/* No auto flow control, disable nRTS signal */
	writel(UMCON_RESET_VAL, &uart->umcon);
	/* 8N1, no parity bit */
	writel(ULCON_WORD_8_BIT, &uart->ulcon);
	/* No interrupts, no DMA, pure polling */
	writel(UCON_RX_IRQ_OR_POLLING | UCON_TX_IRQ_OR_POLLING |
	       UCON_RX_ERR_IRQ_EN | UCON_TX_IRQ_LEVEL, &uart->ucon);
}

static void __maybe_unused s5p_serial_baud(struct s5p_uart *uart, u8 reg_width,
					   uint uclk, int baudrate)
{
	u32 val;

	val = uclk / baudrate;

	writel(val / 16 - 1, &uart->ubrdiv);

	if (s5p_uart_divslot())
		writew(udivslot[val % 16], &uart->rest.slot);
	else if (reg_width == 4)
		writel(val % 16, &uart->rest.value);
	else
		writeb(val % 16, &uart->rest.value);
}

#ifndef CONFIG_SPL_BUILD
int s5p_serial_setbrg(struct udevice *dev, int baudrate)
{
	struct s5p_serial_plat *plat = dev_get_plat(dev);
	struct s5p_uart *const uart = plat->reg;
	u32 uclk;

#if IS_ENABLED(CONFIG_CLK_EXYNOS) || IS_ENABLED(CONFIG_ARCH_APPLE)
	struct clk clk;
	int ret;

	ret = clk_get_by_index(dev, 1, &clk);
	if (ret < 0)
		return ret;
	uclk = clk_get_rate(&clk);
#else
	uclk = get_uart_clk(plat->port_id);
#endif

	s5p_serial_baud(uart, plat->reg_width, uclk, baudrate);

	return 0;
}

static int s5p_serial_probe(struct udevice *dev)
{
	struct s5p_serial_plat *plat = dev_get_plat(dev);
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
	struct s5p_serial_plat *plat = dev_get_plat(dev);
	struct s5p_uart *const uart = plat->reg;

	if (!(readl(&uart->ufstat) & plat->rx_fifo_count_mask))
		return -EAGAIN;

	serial_err_check(uart, 0);
	if (plat->reg_width == 4)
		return (int)(readl(&uart->urxh) & 0xff);
	else
		return (int)(readb(&uart->urxh) & 0xff);
}

static int s5p_serial_putc(struct udevice *dev, const char ch)
{
	struct s5p_serial_plat *plat = dev_get_plat(dev);
	struct s5p_uart *const uart = plat->reg;

	if (readl(&uart->ufstat) & plat->tx_fifo_full)
		return -EAGAIN;

	if (plat->reg_width == 4)
		writel(ch, &uart->utxh);
	else
		writeb(ch, &uart->utxh);
	serial_err_check(uart, 1);

	return 0;
}

static int s5p_serial_pending(struct udevice *dev, bool input)
{
	struct s5p_serial_plat *plat = dev_get_plat(dev);
	struct s5p_uart *const uart = plat->reg;
	uint32_t ufstat = readl(&uart->ufstat);

	if (input) {
		return (ufstat & plat->rx_fifo_count_mask) >>
			plat->rx_fifo_count_shift;
	} else {
		return (ufstat & plat->tx_fifo_count_mask) >>
			plat->tx_fifo_count_shift;
	}
}

static int s5p_serial_of_to_plat(struct udevice *dev)
{
	struct s5p_serial_plat *plat = dev_get_plat(dev);
	const ulong port_type = dev_get_driver_data(dev);

	plat->reg = dev_read_addr_ptr(dev);
	if (!plat->reg)
		return -EINVAL;

	plat->reg_width = dev_read_u32_default(dev, "reg-io-width", 1);
	plat->port_id = dev_read_u8_default(dev, "id", dev_seq(dev));

	if (port_type == PORT_S5L) {
		plat->rx_fifo_count_shift = S5L_RX_FIFO_COUNT_SHIFT;
		plat->rx_fifo_count_mask = S5L_RX_FIFO_COUNT_MASK;
		plat->rx_fifo_full = S5L_RX_FIFO_FULL;
		plat->tx_fifo_count_shift = S5L_TX_FIFO_COUNT_SHIFT;
		plat->tx_fifo_count_mask = S5L_TX_FIFO_COUNT_MASK;
		plat->tx_fifo_full = S5L_TX_FIFO_FULL;
	} else {
		plat->rx_fifo_count_shift = S5P_RX_FIFO_COUNT_SHIFT;
		plat->rx_fifo_count_mask = S5P_RX_FIFO_COUNT_MASK;
		plat->rx_fifo_full = S5P_RX_FIFO_FULL;
		plat->tx_fifo_count_shift = S5P_TX_FIFO_COUNT_SHIFT;
		plat->tx_fifo_count_mask = S5P_TX_FIFO_COUNT_MASK;
		plat->tx_fifo_full = S5P_TX_FIFO_FULL;
	}

	return 0;
}

static const struct dm_serial_ops s5p_serial_ops = {
	.putc		= s5p_serial_putc,
	.pending	= s5p_serial_pending,
	.getc		= s5p_serial_getc,
	.setbrg		= s5p_serial_setbrg,
};

static const struct udevice_id s5p_serial_ids[] = {
	{ .compatible = "samsung,exynos4210-uart",	.data = PORT_S5P },
	{ .compatible = "samsung,exynos850-uart",	.data = PORT_S5P },
	{ .compatible = "apple,s5l-uart",		.data = PORT_S5L },
	{ }
};

U_BOOT_DRIVER(serial_s5p) = {
	.name		= "serial_s5p",
	.id		= UCLASS_SERIAL,
	.of_match	= s5p_serial_ids,
	.of_to_plat	= s5p_serial_of_to_plat,
	.plat_auto	= sizeof(struct s5p_serial_plat),
	.probe		= s5p_serial_probe,
	.ops		= &s5p_serial_ops,
};
#endif

#ifdef CONFIG_DEBUG_UART_S5P

#include <debug_uart.h>

static inline void _debug_uart_init(void)
{
	if (IS_ENABLED(CONFIG_DEBUG_UART_SKIP_INIT))
		return;

	struct s5p_uart *uart = (struct s5p_uart *)CONFIG_VAL(DEBUG_UART_BASE);

	s5p_serial_init(uart);
#if IS_ENABLED(CONFIG_ARCH_APPLE)
	s5p_serial_baud(uart, 4, CONFIG_DEBUG_UART_CLOCK, CONFIG_BAUDRATE);
#else
	s5p_serial_baud(uart, 1, CONFIG_DEBUG_UART_CLOCK, CONFIG_BAUDRATE);
#endif
}

static inline void _debug_uart_putc(int ch)
{
	struct s5p_uart *uart = (struct s5p_uart *)CONFIG_VAL(DEBUG_UART_BASE);

#if IS_ENABLED(CONFIG_ARCH_APPLE)
	while (readl(&uart->ufstat) & S5L_TX_FIFO_FULL)
		;
	writel(ch, &uart->utxh);
#else
	while (readl(&uart->ufstat) & S5P_TX_FIFO_FULL)
		;
	writeb(ch, &uart->utxh);
#endif
}

DEBUG_UART_FUNCS

#endif
