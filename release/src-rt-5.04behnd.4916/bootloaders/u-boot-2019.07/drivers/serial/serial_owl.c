// SPDX-License-Identifier: GPL-2.0+
/*
 * Actions Semi OWL SoCs UART driver
 *
 * Copyright (C) 2015 Actions Semi Co., Ltd.
 * Copyright (C) 2018 Manivannan Sadhasivam <manivannan.sadhasivam@linaro.org>
 */

#include <common.h>
#include <clk.h>
#include <dm.h>
#include <errno.h>
#include <fdtdec.h>
#include <serial.h>
#include <asm/io.h>
#include <asm/types.h>

/* UART Registers */
#define	OWL_UART_CTL			(0x0000)
#define	OWL_UART_RXDAT			(0x0004)
#define	OWL_UART_TXDAT			(0x0008)
#define	OWL_UART_STAT			(0x000C)

/* UART_CTL Register Definitions */
#define	OWL_UART_CTL_PRS_NONE		GENMASK(6, 4)
#define	OWL_UART_CTL_STPS		BIT(2)
#define	OWL_UART_CTL_DWLS		3

/* UART_STAT Register Definitions */
#define	OWL_UART_STAT_TFES		BIT(10)	/* TX FIFO Empty Status	*/
#define	OWL_UART_STAT_RFFS		BIT(9)	/* RX FIFO full	Status */
#define	OWL_UART_STAT_TFFU		BIT(6)	/* TX FIFO full	Status */
#define	OWL_UART_STAT_RFEM		BIT(5)	/* RX FIFO Empty Status	*/

struct owl_serial_priv {
	phys_addr_t base;
};

int owl_serial_setbrg(struct udevice *dev, int baudrate)
{
	/* Driver supports only fixed baudrate */
	return 0;
}

static int owl_serial_getc(struct udevice *dev)
{
	struct owl_serial_priv *priv = dev_get_priv(dev);

	if (readl(priv->base + OWL_UART_STAT) & OWL_UART_STAT_RFEM)
		return -EAGAIN;

	return (int)(readl(priv->base +	OWL_UART_RXDAT));
}

static int owl_serial_putc(struct udevice *dev,	const char ch)
{
	struct owl_serial_priv *priv = dev_get_priv(dev);

	if (readl(priv->base + OWL_UART_STAT) & OWL_UART_STAT_TFFU)
		return -EAGAIN;

	writel(ch, priv->base +	OWL_UART_TXDAT);

	return 0;
}

static int owl_serial_pending(struct udevice *dev, bool	input)
{
	struct owl_serial_priv *priv = dev_get_priv(dev);
	unsigned int stat = readl(priv->base + OWL_UART_STAT);

	if (input)
		return !(stat &	OWL_UART_STAT_RFEM);
	else
		return !(stat &	OWL_UART_STAT_TFES);
}

static int owl_serial_probe(struct udevice *dev)
{
	struct owl_serial_priv *priv = dev_get_priv(dev);
	struct clk clk;
	u32 uart_ctl;
	int ret;

	/* Set data, parity and stop bits */
	uart_ctl = readl(priv->base + OWL_UART_CTL);
	uart_ctl &= ~(OWL_UART_CTL_PRS_NONE);
	uart_ctl &= ~(OWL_UART_CTL_STPS);
	uart_ctl |= OWL_UART_CTL_DWLS;
	writel(uart_ctl, priv->base + OWL_UART_CTL);

	/* Enable UART clock */
	ret = clk_get_by_index(dev, 0, &clk);
	if (ret < 0)
		return ret;

	ret = clk_enable(&clk);
	if (ret < 0)
		return ret;

	return 0;
}

static int owl_serial_ofdata_to_platdata(struct	udevice	*dev)
{
	struct owl_serial_priv *priv = dev_get_priv(dev);

	priv->base = dev_read_addr(dev);
	if (priv->base == FDT_ADDR_T_NONE)
		return -EINVAL;

	return 0;
}

static const struct dm_serial_ops owl_serial_ops = {
	.putc =	owl_serial_putc,
	.pending = owl_serial_pending,
	.getc =	owl_serial_getc,
	.setbrg	= owl_serial_setbrg,
};

static const struct udevice_id owl_serial_ids[] = {
	{ .compatible =	"actions,s900-serial" },
	{ }
};

U_BOOT_DRIVER(serial_owl) = {
	.name = "serial_owl",
	.id = UCLASS_SERIAL,
	.of_match = owl_serial_ids,
	.ofdata_to_platdata = owl_serial_ofdata_to_platdata,
	.priv_auto_alloc_size =	sizeof(struct owl_serial_priv),
	.probe = owl_serial_probe,
	.ops = &owl_serial_ops,
};
