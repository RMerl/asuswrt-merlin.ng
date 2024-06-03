// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2017 Álvaro Fernández Rojas <noltari@gmail.com>
 *
 * Derived from linux/drivers/tty/serial/bcm63xx_uart.c:
 *	Copyright (C) 2008 Maxime Bizon <mbizon@freebox.fr>
 */

#include <clk.h>
#include <dm.h>
#include <debug_uart.h>
#include <errno.h>
#include <serial.h>
#include <asm/io.h>
#include <asm/types.h>

/* UART Control register */
#define UART_CTL_REG			0x0
#define UART_CTL_RXTIMEOUT_MASK		0x1f
#define UART_CTL_RXTIMEOUT_5		0x5
#define UART_CTL_RSTRXFIFO_SHIFT	6
#define UART_CTL_RSTRXFIFO_MASK		(1 << UART_CTL_RSTRXFIFO_SHIFT)
#define UART_CTL_RSTTXFIFO_SHIFT	7
#define UART_CTL_RSTTXFIFO_MASK		(1 << UART_CTL_RSTTXFIFO_SHIFT)
#define UART_CTL_STOPBITS_SHIFT		8
#define UART_CTL_STOPBITS_MASK		(0xf << UART_CTL_STOPBITS_SHIFT)
#define UART_CTL_STOPBITS_1		(0x7 << UART_CTL_STOPBITS_SHIFT)
#define UART_CTL_BITSPERSYM_SHIFT	12
#define UART_CTL_BITSPERSYM_MASK	(0x3 << UART_CTL_BITSPERSYM_SHIFT)
#define UART_CTL_BITSPERSYM_8		(0x3 << UART_CTL_BITSPERSYM_SHIFT)
#define UART_CTL_XMITBRK_SHIFT		14
#define UART_CTL_XMITBRK_MASK		(1 << UART_CTL_XMITBRK_SHIFT)
#define UART_CTL_RSVD_SHIFT		15
#define UART_CTL_RSVD_MASK		(1 << UART_CTL_RSVD_SHIFT)
#define UART_CTL_RXPAREVEN_SHIFT	16
#define UART_CTL_RXPAREVEN_MASK		(1 << UART_CTL_RXPAREVEN_SHIFT)
#define UART_CTL_RXPAREN_SHIFT		17
#define UART_CTL_RXPAREN_MASK		(1 << UART_CTL_RXPAREN_SHIFT)
#define UART_CTL_TXPAREVEN_SHIFT	18
#define UART_CTL_TXPAREVEN_MASK		(1 << UART_CTL_TXPAREVEN_SHIFT)
#define UART_CTL_TXPAREN_SHIFT		19
#define UART_CTL_TXPAREN_MASK		(1 << UART_CTL_TXPAREN_SHIFT)
#define UART_CTL_LOOPBACK_SHIFT		20
#define UART_CTL_LOOPBACK_MASK		(1 << UART_CTL_LOOPBACK_SHIFT)
#define UART_CTL_RXEN_SHIFT		21
#define UART_CTL_RXEN_MASK		(1 << UART_CTL_RXEN_SHIFT)
#define UART_CTL_TXEN_SHIFT		22
#define UART_CTL_TXEN_MASK		(1 << UART_CTL_TXEN_SHIFT)
#define UART_CTL_BRGEN_SHIFT		23
#define UART_CTL_BRGEN_MASK		(1 << UART_CTL_BRGEN_SHIFT)

/* UART Baudword register */
#define UART_BAUD_REG			0x4

/* UART FIFO Config register */
#define UART_FIFO_CFG_REG		0x8
#define UART_FIFO_CFG_RX_SHIFT		8
#define UART_FIFO_CFG_RX_MASK		(0xf << UART_FIFO_CFG_RX_SHIFT)
#define UART_FIFO_CFG_RX_4		(0x4 << UART_FIFO_CFG_RX_SHIFT)
#define UART_FIFO_CFG_TX_SHIFT		12
#define UART_FIFO_CFG_TX_MASK		(0xf << UART_FIFO_CFG_TX_SHIFT)
#define UART_FIFO_CFG_TX_4		(0x4 << UART_FIFO_CFG_TX_SHIFT)

/* UART Interrupt register */
#define UART_IR_REG			0x10
#define UART_IR_STAT(x)			(1 << (x))
#define UART_IR_TXEMPTY			5
#define UART_IR_RXOVER			7
#define UART_IR_RXNOTEMPTY		11

/* UART FIFO register */
#define UART_FIFO_REG			0x14
#define UART_FIFO_VALID_MASK		0xff
#define UART_FIFO_FRAMEERR_SHIFT	8
#define UART_FIFO_FRAMEERR_MASK		(1 << UART_FIFO_FRAMEERR_SHIFT)
#define UART_FIFO_PARERR_SHIFT		9
#define UART_FIFO_PARERR_MASK		(1 << UART_FIFO_PARERR_SHIFT)
#define UART_FIFO_BRKDET_SHIFT		10
#define UART_FIFO_BRKDET_MASK		(1 << UART_FIFO_BRKDET_SHIFT)
#define UART_FIFO_ANYERR_MASK		(UART_FIFO_FRAMEERR_MASK |	\
					UART_FIFO_PARERR_MASK |		\
					UART_FIFO_BRKDET_MASK)

struct bcm6345_serial_priv {
	void __iomem *base;
	ulong uartclk;
};

/* enable rx & tx operation on uart */
static void bcm6345_serial_enable(void __iomem *base)
{
	setbits_32(base + UART_CTL_REG, UART_CTL_BRGEN_MASK |
		   UART_CTL_TXEN_MASK | UART_CTL_RXEN_MASK);
}

/* disable rx & tx operation on uart */
static void bcm6345_serial_disable(void __iomem *base)
{
	clrbits_32(base + UART_CTL_REG, UART_CTL_BRGEN_MASK |
		   UART_CTL_TXEN_MASK | UART_CTL_RXEN_MASK);
}

/* clear all unread data in rx fifo and unsent data in tx fifo */
static void bcm6345_serial_flush(void __iomem *base)
{
	/* empty rx and tx fifo */
	setbits_32(base + UART_CTL_REG, UART_CTL_RSTRXFIFO_MASK |
		   UART_CTL_RSTTXFIFO_MASK);

	/* read any pending char to make sure all irq status are cleared */
	readl(base + UART_FIFO_REG);
}

static int bcm6345_serial_init(void __iomem *base, ulong clk, u32 baudrate)
{
	u32 val;

	/* mask all irq and flush port */
	bcm6345_serial_disable(base);
	bcm6345_serial_flush(base);

	/* set uart control config */
	clrsetbits_32(base + UART_CTL_REG,
		      /* clear rx timeout */
		      UART_CTL_RXTIMEOUT_MASK |
		      /* clear stop bits */
		      UART_CTL_STOPBITS_MASK |
		      /* clear bits per symbol */
		      UART_CTL_BITSPERSYM_MASK |
		      /* clear xmit break */
		      UART_CTL_XMITBRK_MASK |
		      /* clear reserved bit */
		      UART_CTL_RSVD_MASK |
		      /* disable parity */
		      UART_CTL_RXPAREN_MASK |
		      UART_CTL_TXPAREN_MASK |
		      /* disable loopback */
		      UART_CTL_LOOPBACK_MASK,
		      /* set timeout to 5 */
		      UART_CTL_RXTIMEOUT_5 |
		      /* set 8 bits/symbol */
		      UART_CTL_BITSPERSYM_8 |
		      /* set 1 stop bit */
		      UART_CTL_STOPBITS_1 |
		      /* set parity to even */
		      UART_CTL_RXPAREVEN_MASK |
		      UART_CTL_TXPAREVEN_MASK);

	/* set uart fifo config */
	clrsetbits_32(base + UART_FIFO_CFG_REG,
		      /* clear fifo config */
		      UART_FIFO_CFG_RX_MASK |
		      UART_FIFO_CFG_TX_MASK,
		      /* set fifo config to 4 */
		      UART_FIFO_CFG_RX_4 |
		      UART_FIFO_CFG_TX_4);

	/* set baud rate */
	val = ((clk / baudrate) >> 4);
	if (val & 0x1)
		val = (val >> 1);
	else
		val = (val >> 1) - 1;
	writel(val, base + UART_BAUD_REG);

	/* clear interrupts */
	writel(0, base + UART_IR_REG);

	/* enable uart */
	bcm6345_serial_enable(base);

	return 0;
}

static int bcm6345_serial_pending(struct udevice *dev, bool input)
{
	struct bcm6345_serial_priv *priv = dev_get_priv(dev);
	u32 val = readl(priv->base + UART_IR_REG);

	if (input)
		return !!(val & UART_IR_STAT(UART_IR_RXNOTEMPTY));
	else
		return !(val & UART_IR_STAT(UART_IR_TXEMPTY));
}

static int bcm6345_serial_setbrg(struct udevice *dev, int baudrate)
{
	struct bcm6345_serial_priv *priv = dev_get_priv(dev);

	return bcm6345_serial_init(priv->base, priv->uartclk, baudrate);
}

static int bcm6345_serial_putc(struct udevice *dev, const char ch)
{
	struct bcm6345_serial_priv *priv = dev_get_priv(dev);
	u32 val;

	val = readl(priv->base + UART_IR_REG);
	if (!(val & UART_IR_STAT(UART_IR_TXEMPTY)))
		return -EAGAIN;

	writel(ch, priv->base + UART_FIFO_REG);

	return 0;
}

static int bcm6345_serial_getc(struct udevice *dev)
{
	struct bcm6345_serial_priv *priv = dev_get_priv(dev);
	u32 val;

	val = readl(priv->base + UART_IR_REG);
	if (val & UART_IR_STAT(UART_IR_RXOVER))
		setbits_32(priv->base + UART_CTL_REG, UART_CTL_RSTRXFIFO_MASK);
	if (!(val & UART_IR_STAT(UART_IR_RXNOTEMPTY)))
		return -EAGAIN;

	val = readl(priv->base + UART_FIFO_REG);
	if (val & UART_FIFO_ANYERR_MASK)
		return -EAGAIN;

	return val & UART_FIFO_VALID_MASK;
}

static int bcm6345_serial_probe(struct udevice *dev)
{
	struct bcm6345_serial_priv *priv = dev_get_priv(dev);
	struct clk clk;
	int ret;

	/* get address */
	priv->base = dev_remap_addr(dev);
	if (!priv->base)
		return -EINVAL;

	/* get clock rate */
	ret = clk_get_by_index(dev, 0, &clk);
	if (ret < 0)
		return ret;
	priv->uartclk = clk_get_rate(&clk);
	clk_free(&clk);

	/* initialize serial */
	return bcm6345_serial_init(priv->base, priv->uartclk, CONFIG_BAUDRATE);
}

static const struct dm_serial_ops bcm6345_serial_ops = {
	.putc = bcm6345_serial_putc,
	.pending = bcm6345_serial_pending,
	.getc = bcm6345_serial_getc,
	.setbrg = bcm6345_serial_setbrg,
};

static const struct udevice_id bcm6345_serial_ids[] = {
	{ .compatible = "brcm,bcm6345-uart" },
	{ /* sentinel */ }
};

U_BOOT_DRIVER(bcm6345_serial) = {
	.name = "bcm6345-uart",
	.id = UCLASS_SERIAL,
	.of_match = bcm6345_serial_ids,
	.probe = bcm6345_serial_probe,
	.priv_auto_alloc_size = sizeof(struct bcm6345_serial_priv),
	.ops = &bcm6345_serial_ops,
};

#ifdef CONFIG_DEBUG_UART_BCM6345
static inline void _debug_uart_init(void)
{
	void __iomem *base = (void __iomem *)CONFIG_DEBUG_UART_BASE;

	bcm6345_serial_init(base, CONFIG_DEBUG_UART_CLOCK, CONFIG_BAUDRATE);
}

static inline void wait_xfered(void __iomem *base)
{
	do {
		u32 val = readl(base + UART_IR_REG);
		if (val & UART_IR_STAT(UART_IR_TXEMPTY))
			break;
	} while (1);
}

static inline void _debug_uart_putc(int ch)
{
	void __iomem *base = (void __iomem *)CONFIG_DEBUG_UART_BASE;

	wait_xfered(base);
	writel(ch, base + UART_FIFO_REG);
	wait_xfered(base);
}

DEBUG_UART_FUNCS
#endif
