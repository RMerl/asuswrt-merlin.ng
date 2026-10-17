// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2016 Stefan Roese <sr@denx.de>
 * Copyright (C) 2021 Pali Rohár <pali@kernel.org>
 */

#include <common.h>
#include <clk.h>
#include <dm.h>
#include <serial.h>
#include <asm/io.h>
#include <asm/arch/cpu.h>
#include <mach/soc.h>

struct mvebu_plat {
	void __iomem *base;
	ulong tbg_rate;
	u8 tbg_idx;
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
#define UART_STATUS_TX_EMPTY	0x40
#define UART_STATUS_TXFIFO_FULL	0x800

#define UART_CTRL_RXFIFO_RESET	0x4000
#define UART_CTRL_TXFIFO_RESET	0x8000

static int mvebu_serial_putc(struct udevice *dev, const char ch)
{
	struct mvebu_plat *plat = dev_get_plat(dev);
	void __iomem *base = plat->base;

	if (readl(base + UART_STATUS_REG) & UART_STATUS_TXFIFO_FULL)
		return -EAGAIN;

	writel(ch, base + UART_TX_REG);

	return 0;
}

static int mvebu_serial_getc(struct udevice *dev)
{
	struct mvebu_plat *plat = dev_get_plat(dev);
	void __iomem *base = plat->base;

	if (!(readl(base + UART_STATUS_REG) & UART_STATUS_RX_RDY))
		return -EAGAIN;

	return readl(base + UART_RX_REG) & 0xff;
}

static int mvebu_serial_pending(struct udevice *dev, bool input)
{
	struct mvebu_plat *plat = dev_get_plat(dev);
	void __iomem *base = plat->base;

	if (input) {
		if (readl(base + UART_STATUS_REG) & UART_STATUS_RX_RDY)
			return 1;
	} else {
		if (!(readl(base + UART_STATUS_REG) & UART_STATUS_TX_EMPTY))
			return 1;
	}

	return 0;
}

static int mvebu_serial_setbrg(struct udevice *dev, int baudrate)
{
	struct mvebu_plat *plat = dev_get_plat(dev);
	void __iomem *base = plat->base;
	u32 divider, d1, d2;
	u32 oversampling;

	/*
	 * Calculate divider
	 * baudrate = clock / 16 / divider
	 */
	d1 = d2 = 1;
	divider = DIV_ROUND_CLOSEST(plat->tbg_rate, baudrate * 16 * d1 * d2);

	/*
	 * Set Programmable Oversampling Stack to 0,
	 * UART defaults to 16x scheme
	 */
	oversampling = 0;

	if (divider < 1)
		divider = 1;
	else if (divider > 1023) {
		/*
		 * If divider is too high for selected baudrate then set
		 * divider d1 to the maximal value 6.
		 */
		d1 = 6;
		divider = DIV_ROUND_CLOSEST(plat->tbg_rate,
					    baudrate * 16 * d1 * d2);
		if (divider < 1)
			divider = 1;
		else if (divider > 1023) {
			/*
			 * If divider is still too high then set also divider
			 * d2 to the maximal value 6.
			 */
			d2 = 6;
			divider = DIV_ROUND_CLOSEST(plat->tbg_rate,
						    baudrate * 16 * d1 * d2);
			if (divider < 1)
				divider = 1;
			else if (divider > 1023) {
				/*
				 * And if divider is still to high then
				 * use oversampling with maximal factor 63.
				 */
				oversampling = (63 << 0) | (63 << 8) |
					      (63 << 16) | (63 << 24);
				divider = DIV_ROUND_CLOSEST(plat->tbg_rate,
						baudrate * 63 * d1 * d2);
				if (divider < 1)
					divider = 1;
				else if (divider > 1023)
					divider = 1023;
			}
		}
	}

	divider |= BIT(19); /* Do not use XTAL as a base clock */
	divider |= d1 << 15; /* Set d1 divider */
	divider |= d2 << 12; /* Set d2 divider */
	divider |= plat->tbg_idx << 10; /* Use selected TBG as a base clock */

	while (!(readl(base + UART_STATUS_REG) & UART_STATUS_TX_EMPTY))
		;
	writel(divider, base + UART_BAUD_REG);
	writel(oversampling, base + UART_POSSR_REG);

	return 0;
}

static int mvebu_serial_probe(struct udevice *dev)
{
	struct mvebu_plat *plat = dev_get_plat(dev);
	void __iomem *base = plat->base;
	struct udevice *nb_clk;
	ofnode nb_clk_node;
	int i, res;

	nb_clk_node = ofnode_by_compatible(ofnode_null(),
					   "marvell,armada-3700-periph-clock-nb");
	if (!ofnode_valid(nb_clk_node)) {
		printf("%s: NB periph clock node not available\n", __func__);
		return -ENODEV;
	}

	res = device_get_global_by_ofnode(nb_clk_node, &nb_clk);
	if (res) {
		printf("%s: Cannot get NB periph clock\n", __func__);
		return res;
	}

	/*
	 * Choose the TBG clock with lowest frequency which allows to configure
	 * UART also at lower baudrates.
	 */
	for (i = 0; i < 4; i++) {
		struct clk clk;
		ulong rate;

		res = clk_get_by_index_nodev(nb_clk_node, i, &clk);
		if (res) {
			printf("%s: Cannot get TBG clock %i: %i\n", __func__,
			       i, res);
			return -ENODEV;
		}

		rate = clk_get_rate(&clk);
		if (!rate || IS_ERR_VALUE(rate)) {
			printf("%s: Cannot get rate for TBG clock %i\n",
			       __func__, i);
			return -EINVAL;
		}

		if (!i || plat->tbg_rate > rate) {
			plat->tbg_rate = rate;
			plat->tbg_idx = i;
		}
	}

	/* reset FIFOs */
	writel(UART_CTRL_RXFIFO_RESET | UART_CTRL_TXFIFO_RESET,
	       base + UART_CTRL_REG);

	/* No Parity, 1 Stop */
	writel(0, base + UART_CTRL_REG);

	return 0;
}

static int mvebu_serial_remove(struct udevice *dev)
{
	struct mvebu_plat *plat = dev_get_plat(dev);
	void __iomem *base = plat->base;
	ulong new_parent_rate, parent_rate;
	u32 new_divider, divider;
	u32 new_oversampling;
	u32 oversampling;
	u32 d1, d2;
	u32 nb_rst;

	/*
	 * Switch UART base clock back to XTAL because older Linux kernel
	 * expects it. Otherwise it does not calculate UART divisor correctly
	 * and therefore UART does not work in kernel.
	 */
	divider = readl(base + UART_BAUD_REG);
	if (!(divider & BIT(19))) /* UART already uses XTAL */
		return 0;

	/* Read current divisors settings */
	d1 = (divider >> 15) & 7;
	d2 = (divider >> 12) & 7;
	parent_rate = plat->tbg_rate;
	divider &= 1023;
	oversampling = readl(base + UART_POSSR_REG) & 63;
	if (!oversampling)
		oversampling = 16;

	/* Calculate new divisor against XTAL clock without changing baudrate */
	new_oversampling = 0;
	new_parent_rate = get_ref_clk() * 1000000;
	new_divider = DIV_ROUND_CLOSEST(new_parent_rate * divider * d1 * d2 *
					oversampling, parent_rate * 16);

	/*
	 * UART does not work reliably when XTAL divisor is smaller than 4.
	 * In this case we do not switch UART parent to XTAL. User either
	 * configured unsupported settings or has newer kernel with patches
	 * which allow usage of non-XTAL clock as a parent clock.
	 */
	if (new_divider < 4)
		return 0;

	/*
	 * If new divisor is larger than maximal supported, try to switch
	 * from default x16 scheme to oversampling with maximal factor 63.
	 */
	if (new_divider > 1023) {
		new_oversampling = 63;
		new_divider = DIV_ROUND_CLOSEST(new_parent_rate * divider * d1 *
						d2 * oversampling,
						parent_rate * new_oversampling);
		if (new_divider < 4 || new_divider > 1023)
			return 0;
	}

	/* wait until TX empty */
	while (!(readl(base + UART_STATUS_REG) & UART_STATUS_TX_EMPTY))
		;

	/* external reset of UART via North Bridge Peripheral */
	nb_rst = readl(MVEBU_REGISTER(0x12400));
	writel(nb_rst & ~BIT(3), MVEBU_REGISTER(0x12400));
	writel(nb_rst | BIT(3), MVEBU_REGISTER(0x12400));

	/* set baudrate and oversampling */
	writel(new_divider, base + UART_BAUD_REG);
	writel(new_oversampling, base + UART_POSSR_REG);

	/* No Parity, 1 Stop */
	writel(0, base + UART_CTRL_REG);

	return 0;
}

static int mvebu_serial_of_to_plat(struct udevice *dev)
{
	struct mvebu_plat *plat = dev_get_plat(dev);

	plat->base = dev_read_addr_ptr(dev);

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
	.of_to_plat = mvebu_serial_of_to_plat,
	.plat_auto	= sizeof(struct mvebu_plat),
	.probe	= mvebu_serial_probe,
	.remove	= mvebu_serial_remove,
	.flags	= DM_FLAG_OS_PREPARE,
	.ops	= &mvebu_serial_ops,
};

#ifdef CONFIG_DEBUG_MVEBU_A3700_UART

#include <debug_uart.h>

static inline void _debug_uart_init(void)
{
	void __iomem *base = (void __iomem *)CONFIG_VAL(DEBUG_UART_BASE);
	u32 parent_rate, divider;

	/* reset FIFOs */
	writel(UART_CTRL_RXFIFO_RESET | UART_CTRL_TXFIFO_RESET,
	       base + UART_CTRL_REG);

	/* No Parity, 1 Stop */
	writel(0, base + UART_CTRL_REG);

	/*
	 * Calculate divider
	 * baudrate = clock / 16 / divider
	 */
	parent_rate = (readl(MVEBU_REGISTER(0x13808)) & BIT(9)) ?
		      40000000 : 25000000;
	divider = DIV_ROUND_CLOSEST(parent_rate, CONFIG_BAUDRATE * 16);
	writel(divider, base + UART_BAUD_REG);

	/*
	 * Set Programmable Oversampling Stack to 0,
	 * UART defaults to 16x scheme
	 */
	writel(0, base + UART_POSSR_REG);
}

static inline void _debug_uart_putc(int ch)
{
	void __iomem *base = (void __iomem *)CONFIG_VAL(DEBUG_UART_BASE);

	while (readl(base + UART_STATUS_REG) & UART_STATUS_TXFIFO_FULL)
		;

	writel(ch, base + UART_TX_REG);
}

DEBUG_UART_FUNCS
#endif
