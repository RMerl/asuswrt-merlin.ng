// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2015-2016 Wills Wang <wills.wang@live.com>
 */

#include <common.h>
#include <dm.h>
#include <div64.h>
#include <errno.h>
#include <serial.h>
#include <asm/io.h>
#include <asm/addrspace.h>
#include <asm/types.h>
#include <dm/pinctrl.h>
#include <mach/ar71xx_regs.h>

#define AR933X_UART_DATA_REG            0x00
#define AR933X_UART_CS_REG              0x04
#define AR933X_UART_CLK_REG             0x08

#define AR933X_UART_DATA_TX_RX_MASK     0xff
#define AR933X_UART_DATA_RX_CSR         BIT(8)
#define AR933X_UART_DATA_TX_CSR         BIT(9)
#define AR933X_UART_CS_IF_MODE_S        2
#define AR933X_UART_CS_IF_MODE_M        0x3
#define AR933X_UART_CS_IF_MODE_DTE      1
#define AR933X_UART_CS_IF_MODE_DCE      2
#define AR933X_UART_CS_TX_RDY_ORIDE     BIT(7)
#define AR933X_UART_CS_RX_RDY_ORIDE     BIT(8)
#define AR933X_UART_CLK_STEP_M          0xffff
#define AR933X_UART_CLK_SCALE_M         0xfff
#define AR933X_UART_CLK_SCALE_S         16
#define AR933X_UART_CLK_STEP_S          0

struct ar933x_serial_priv {
	void __iomem *regs;
};

/*
 * Baudrate algorithm come from Linux/drivers/tty/serial/ar933x_uart.c
 * baudrate = (clk / (scale + 1)) * (step * (1 / 2^17))
 */
static u32 ar933x_serial_get_baud(u32 clk, u32 scale, u32 step)
{
	u64 t;
	u32 div;

	div = (2 << 16) * (scale + 1);
	t = clk;
	t *= step;
	t += (div / 2);
	do_div(t, div);

	return t;
}

static void ar933x_serial_get_scale_step(u32 clk, u32 baud,
					 u32 *scale, u32 *step)
{
	u32 tscale, baudrate;
	long min_diff;

	*scale = 0;
	*step = 0;

	min_diff = baud;
	for (tscale = 0; tscale < AR933X_UART_CLK_SCALE_M; tscale++) {
		u64 tstep;
		int diff;

		tstep = baud * (tscale + 1);
		tstep *= (2 << 16);
		do_div(tstep, clk);

		if (tstep > AR933X_UART_CLK_STEP_M)
			break;

		baudrate = ar933x_serial_get_baud(clk, tscale, tstep);
		diff = abs(baudrate - baud);
		if (diff < min_diff) {
			min_diff = diff;
			*scale = tscale;
			*step = tstep;
		}
	}
}

static int ar933x_serial_setbrg(struct udevice *dev, int baudrate)
{
	struct ar933x_serial_priv *priv = dev_get_priv(dev);
	u32 val, scale, step;

	val = get_serial_clock();
	ar933x_serial_get_scale_step(val, baudrate, &scale, &step);

	val  = (scale & AR933X_UART_CLK_SCALE_M)
			<< AR933X_UART_CLK_SCALE_S;
	val |= (step & AR933X_UART_CLK_STEP_M)
			<< AR933X_UART_CLK_STEP_S;
	writel(val, priv->regs + AR933X_UART_CLK_REG);

	return 0;
}

static int ar933x_serial_putc(struct udevice *dev, const char c)
{
	struct ar933x_serial_priv *priv = dev_get_priv(dev);
	u32 data;

	data = readl(priv->regs + AR933X_UART_DATA_REG);
	if (!(data & AR933X_UART_DATA_TX_CSR))
		return -EAGAIN;

	data  = (u32)c | AR933X_UART_DATA_TX_CSR;
	writel(data, priv->regs + AR933X_UART_DATA_REG);

	return 0;
}

static int ar933x_serial_getc(struct udevice *dev)
{
	struct ar933x_serial_priv *priv = dev_get_priv(dev);
	u32 data;

	data = readl(priv->regs + AR933X_UART_DATA_REG);
	if (!(data & AR933X_UART_DATA_RX_CSR))
		return -EAGAIN;

	writel(AR933X_UART_DATA_RX_CSR, priv->regs + AR933X_UART_DATA_REG);
	return data & AR933X_UART_DATA_TX_RX_MASK;
}

static int ar933x_serial_pending(struct udevice *dev, bool input)
{
	struct ar933x_serial_priv *priv = dev_get_priv(dev);
	u32 data;

	data = readl(priv->regs + AR933X_UART_DATA_REG);
	if (input)
		return (data & AR933X_UART_DATA_RX_CSR) ? 1 : 0;
	else
		return (data & AR933X_UART_DATA_TX_CSR) ? 0 : 1;
}

static int ar933x_serial_probe(struct udevice *dev)
{
	struct ar933x_serial_priv *priv = dev_get_priv(dev);
	fdt_addr_t addr;
	u32 val;

	addr = devfdt_get_addr(dev);
	if (addr == FDT_ADDR_T_NONE)
		return -EINVAL;

	priv->regs = map_physmem(addr, AR933X_UART_SIZE,
				 MAP_NOCACHE);

	/*
	 * UART controller configuration:
	 * - no DMA
	 * - no interrupt
	 * - DCE mode
	 * - no flow control
	 * - set RX ready oride
	 * - set TX ready oride
	 */
	val = (AR933X_UART_CS_IF_MODE_DCE << AR933X_UART_CS_IF_MODE_S) |
	      AR933X_UART_CS_TX_RDY_ORIDE | AR933X_UART_CS_RX_RDY_ORIDE;
	writel(val, priv->regs + AR933X_UART_CS_REG);
	return 0;
}

static const struct dm_serial_ops ar933x_serial_ops = {
	.putc = ar933x_serial_putc,
	.pending = ar933x_serial_pending,
	.getc = ar933x_serial_getc,
	.setbrg = ar933x_serial_setbrg,
};

static const struct udevice_id ar933x_serial_ids[] = {
	{ .compatible = "qca,ar9330-uart" },
	{ }
};

U_BOOT_DRIVER(serial_ar933x) = {
	.name   = "serial_ar933x",
	.id = UCLASS_SERIAL,
	.of_match = ar933x_serial_ids,
	.priv_auto_alloc_size = sizeof(struct ar933x_serial_priv),
	.probe = ar933x_serial_probe,
	.ops    = &ar933x_serial_ops,
};

#ifdef CONFIG_DEBUG_UART_AR933X

#include <debug_uart.h>

static inline void _debug_uart_init(void)
{
	void __iomem *regs = (void *)CONFIG_DEBUG_UART_BASE;
	u32 val, scale, step;

	/*
	 * UART controller configuration:
	 * - no DMA
	 * - no interrupt
	 * - DCE mode
	 * - no flow control
	 * - set RX ready oride
	 * - set TX ready oride
	 */
	val = (AR933X_UART_CS_IF_MODE_DCE << AR933X_UART_CS_IF_MODE_S) |
	      AR933X_UART_CS_TX_RDY_ORIDE | AR933X_UART_CS_RX_RDY_ORIDE;
	writel(val, regs + AR933X_UART_CS_REG);

	ar933x_serial_get_scale_step(CONFIG_DEBUG_UART_CLOCK,
				     CONFIG_BAUDRATE, &scale, &step);

	val  = (scale & AR933X_UART_CLK_SCALE_M)
			<< AR933X_UART_CLK_SCALE_S;
	val |= (step & AR933X_UART_CLK_STEP_M)
			<< AR933X_UART_CLK_STEP_S;
	writel(val, regs + AR933X_UART_CLK_REG);
}

static inline void _debug_uart_putc(int c)
{
	void __iomem *regs = (void *)CONFIG_DEBUG_UART_BASE;
	u32 data;

	do {
		data = readl(regs + AR933X_UART_DATA_REG);
	} while (!(data & AR933X_UART_DATA_TX_CSR));

	data  = (u32)c | AR933X_UART_DATA_TX_CSR;
	writel(data, regs + AR933X_UART_DATA_REG);
}

DEBUG_UART_FUNCS

#endif
