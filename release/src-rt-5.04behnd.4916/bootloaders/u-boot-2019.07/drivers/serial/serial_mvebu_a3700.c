// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2016 Stefan Roese <sr@denx.de>
 */

#include <common.h>
#include <dm.h>
#include <serial.h>
#include <asm/io.h>

struct mvebu_platdata {
	void __iomem *base;
};

/*
 * Register offset
 */
#define UART_RX_REG		0x00
#define UART_TX_REG		0x04
#define UART_CTRL_REG		0x08
#define UART_STATUS_REG		0x0c
#define UART_BAUD_REG		0x10
#define UART_POSSR_REG		0x14

#define UART_STATUS_RX_RDY	0x10
#define UART_STATUS_TXFIFO_FULL	0x800

#define UART_CTRL_RXFIFO_RESET	0x4000
#define UART_CTRL_TXFIFO_RESET	0x8000

#define CONFIG_UART_BASE_CLOCK	25804800

static int mvebu_serial_putc(struct udevice *dev, const char ch)
{
	struct mvebu_platdata *plat = dev_get_platdata(dev);
	void __iomem *base = plat->base;

	while (readl(base + UART_STATUS_REG) & UART_STATUS_TXFIFO_FULL)
		;

	writel(ch, base + UART_TX_REG);

	return 0;
}

static int mvebu_serial_getc(struct udevice *dev)
{
	struct mvebu_platdata *plat = dev_get_platdata(dev);
	void __iomem *base = plat->base;

	while (!(readl(base + UART_STATUS_REG) & UART_STATUS_RX_RDY))
		;

	return readl(base + UART_RX_REG) & 0xff;
}

static int mvebu_serial_pending(struct udevice *dev, bool input)
{
	struct mvebu_platdata *plat = dev_get_platdata(dev);
	void __iomem *base = plat->base;

	if (readl(base + UART_STATUS_REG) & UART_STATUS_RX_RDY)
		return 1;

	return 0;
}

static int mvebu_serial_setbrg(struct udevice *dev, int baudrate)
{
	struct mvebu_platdata *plat = dev_get_platdata(dev);
	void __iomem *base = plat->base;

	/*
	 * Calculate divider
	 * baudrate = clock / 16 / divider
	 */
	writel(CONFIG_UART_BASE_CLOCK / baudrate / 16, base + UART_BAUD_REG);

	/*
	 * Set Programmable Oversampling Stack to 0,
	 * UART defaults to 16x scheme
	 */
	writel(0, base + UART_POSSR_REG);

	return 0;
}

static int mvebu_serial_probe(struct udevice *dev)
{
	struct mvebu_platdata *plat = dev_get_platdata(dev);
	void __iomem *base = plat->base;

	/* reset FIFOs */
	writel(UART_CTRL_RXFIFO_RESET | UART_CTRL_TXFIFO_RESET,
	       base + UART_CTRL_REG);

	/* No Parity, 1 Stop */
	writel(0, base + UART_CTRL_REG);

	return 0;
}

static int mvebu_serial_ofdata_to_platdata(struct udevice *dev)
{
	struct mvebu_platdata *plat = dev_get_platdata(dev);

	plat->base = devfdt_get_addr_ptr(dev);

	return 0;
}

static const struct dm_serial_ops mvebu_serial_ops = {
	.putc = mvebu_serial_putc,
	.pending = mvebu_serial_pending,
	.getc = mvebu_serial_getc,
	.setbrg = mvebu_serial_setbrg,
};

static const struct udevice_id mvebu_serial_ids[] = {
	{ .compatible = "marvell,armada-3700-uart" },
	{ }
};

U_BOOT_DRIVER(serial_mvebu) = {
	.name	= "serial_mvebu",
	.id	= UCLASS_SERIAL,
	.of_match = mvebu_serial_ids,
	.ofdata_to_platdata = mvebu_serial_ofdata_to_platdata,
	.platdata_auto_alloc_size = sizeof(struct mvebu_platdata),
	.probe	= mvebu_serial_probe,
	.ops	= &mvebu_serial_ops,
};

#ifdef CONFIG_DEBUG_MVEBU_A3700_UART

#include <debug_uart.h>

static inline void _debug_uart_init(void)
{
	void __iomem *base = (void __iomem *)CONFIG_DEBUG_UART_BASE;

	/* reset FIFOs */
	writel(UART_CTRL_RXFIFO_RESET | UART_CTRL_TXFIFO_RESET,
	       base + UART_CTRL_REG);

	/* No Parity, 1 Stop */
	writel(0, base + UART_CTRL_REG);

	/*
	 * Calculate divider
	 * baudrate = clock / 16 / divider
	 */
	writel(CONFIG_UART_BASE_CLOCK / 115200 / 16, base + UART_BAUD_REG);

	/*
	 * Set Programmable Oversampling Stack to 0,
	 * UART defaults to 16x scheme
	 */
	writel(0, base + UART_POSSR_REG);
}

static inline void _debug_uart_putc(int ch)
{
	void __iomem *base = (void __iomem *)CONFIG_DEBUG_UART_BASE;

	while (readl(base + UART_STATUS_REG) & UART_STATUS_TXFIFO_FULL)
		;

	writel(ch, base + UART_TX_REG);
}

DEBUG_UART_FUNCS
#endif
