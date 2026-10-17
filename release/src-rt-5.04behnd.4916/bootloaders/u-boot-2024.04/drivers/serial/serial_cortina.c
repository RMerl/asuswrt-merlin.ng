// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2020 Cortina-Access Ltd.
 * Common UART Driver for Cortina Access CAxxxx line of SoCs
 *
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <watchdog.h>
#include <asm/io.h>
#include <serial.h>
#include <linux/bitops.h>
#include <linux/compiler.h>

/* Register definitions */
#define UCFG			0x00	/* UART config register */
#define UFC			0x04	/* Flow Control */
#define URX_SAMPLE		0x08	/* UART RX Sample register */
#define URT_TUNE		0x0C	/* Fine tune of UART clk */
#define UTX_DATA		0x10	/* UART TX Character data */
#define URX_DATA		0x14	/* UART RX Character data */
#define UINFO			0x18	/* UART Info */
#define UINT_EN0		0x1C	/* UART Interrupt enable 0 */
#define UINT_EN1		0x20	/* UART Interrupt enable 1 */
#define UINT0			0x24	/* UART Interrupt 0 setting/clearing */
#define UINT1			0x28	/* UART Interrupt 1 setting/clearing */
#define UINT_STAT		0x2C	/* UART Interrupt Status */

/* UART Control Register Bit Fields */
#define UCFG_BAUD_COUNT_MASK    0xFFFFFF00
#define UCFG_BAUD_COUNT(x)	((x << 8) & UCFG_BAUD_COUNT_MASK)
#define UCFG_EN			BIT(7)
#define UCFG_RX_EN		BIT(6)
#define UCFG_TX_EN		BIT(5)
#define UCFG_PARITY_EN		BIT(4)
#define UCFG_PARITY_SEL		BIT(3)
#define UCFG_2STOP_BIT		BIT(2)
#define UCFG_CNT1		BIT(1)
#define UCFG_CNT0		BIT(0)
#define UCFG_CHAR_5		0
#define UCFG_CHAR_6		1
#define UCFG_CHAR_7		2
#define UCFG_CHAR_8		3

#define UINFO_TX_FIFO_EMPTY	BIT(3)
#define UINFO_TX_FIFO_FULL	BIT(2)
#define UINFO_RX_FIFO_EMPTY	BIT(1)
#define UINFO_RX_FIFO_FULL	BIT(0)

#define UINT_RX_NON_EMPTY	BIT(6)
#define UINT_TX_EMPTY		BIT(5)
#define UINT_RX_UNDERRUN	BIT(4)
#define UINT_RX_OVERRUN		BIT(3)
#define UINT_RX_PARITY_ERR	BIT(2)
#define UINT_RX_STOP_ERR	BIT(1)
#define UINT_TX_OVERRUN		BIT(0)
#define UINT_MASK_ALL		0x7F

struct ca_uart_priv {
	void __iomem *base;
};

int ca_serial_setbrg(struct udevice *dev, int baudrate)
{
	struct ca_uart_priv *priv = dev_get_priv(dev);
	unsigned int uart_ctrl, baud, sample;

	baud = CORTINA_UART_CLOCK / baudrate;

	uart_ctrl = readl(priv->base + UCFG);
	uart_ctrl &= ~UCFG_BAUD_COUNT_MASK;
	uart_ctrl |= UCFG_BAUD_COUNT(baud);
	writel(uart_ctrl, priv->base + UCFG);

	sample = baud / 2;
	sample = (sample < 7) ? 7 : sample;
	writel(sample, priv->base + URX_SAMPLE);

	return 0;
}

static int ca_serial_getc(struct udevice *dev)
{
	struct ca_uart_priv *priv = dev_get_priv(dev);
	int ch;

	ch = readl(priv->base + URX_DATA) & 0xFF;

	return (int)ch;
}

static int ca_serial_putc(struct udevice *dev, const char ch)
{
	struct ca_uart_priv *priv = dev_get_priv(dev);
	unsigned int status;

	/* Retry if TX FIFO full */
	status = readl(priv->base + UINFO);
	if (status & UINFO_TX_FIFO_FULL)
		return -EAGAIN;

	writel(ch, priv->base + UTX_DATA);

	return 0;
}

static int ca_serial_pending(struct udevice *dev, bool input)
{
	struct ca_uart_priv *priv = dev_get_priv(dev);
	unsigned int status;

	status = readl(priv->base + UINFO);

	if (input)
		return (status & UINFO_RX_FIFO_EMPTY) ? 0 : 1;
	else
		return (status & UINFO_TX_FIFO_FULL) ? 1 : 0;
}

static int ca_serial_probe(struct udevice *dev)
{
	struct ca_uart_priv *priv = dev_get_priv(dev);
	u32 uart_ctrl;

	/* Set data, parity and stop bits */
	uart_ctrl = UCFG_EN | UCFG_TX_EN | UCFG_RX_EN | UCFG_CHAR_8;
	writel(uart_ctrl, priv->base + UCFG);

	return 0;
}

static int ca_serial_of_to_plat(struct udevice *dev)
{
	struct ca_uart_priv *priv = dev_get_priv(dev);

	priv->base = dev_remap_addr_index(dev, 0);
	if (!priv->base)
		return -ENOENT;

	return 0;
}

static const struct dm_serial_ops ca_serial_ops = {
	.putc = ca_serial_putc,
	.pending = ca_serial_pending,
	.getc = ca_serial_getc,
	.setbrg = ca_serial_setbrg,
};

static const struct udevice_id ca_serial_ids[] = {
	{.compatible = "cortina,ca-uart"},
	{}
};

U_BOOT_DRIVER(serial_cortina) = {
	.name = "serial_cortina",
	.id = UCLASS_SERIAL,
	.of_match = ca_serial_ids,
	.of_to_plat = ca_serial_of_to_plat,
	.priv_auto	= sizeof(struct ca_uart_priv),
	.probe = ca_serial_probe,
	.ops = &ca_serial_ops
};
