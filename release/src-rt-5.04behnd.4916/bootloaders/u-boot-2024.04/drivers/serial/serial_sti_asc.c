// SPDX-License-Identifier: GPL-2.0+
/*
 * Support for Serial I/O using STMicroelectronics' on-chip ASC.
 *
 * Copyright (C) 2017, STMicroelectronics - All Rights Reserved
 * Author(s): Patrice Chotard, <patrice.chotard@foss.st.com> for STMicroelectronics.
 */

#include <common.h>
#include <dm.h>
#include <log.h>
#include <serial.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <linux/bitops.h>

DECLARE_GLOBAL_DATA_PTR;

#define BAUDMODE	0x00001000
#define RXENABLE	0x00000100
#define RUN		0x00000080
#define MODE		0x00000001
#define MODE_8BIT	0x0001
#define STOP_1BIT	0x0008
#define PARITYODD	0x0020

#define STA_TF		BIT(9)
#define STA_RBF		BIT(0)

struct sti_asc_uart {
	u32 baudrate;
	u32 txbuf;
	u32 rxbuf;
	u32 control;
	u32 inten;
	u32 status;
	u32 guardtime;
	u32 timeout;
	u32 txreset;
	u32 rxreset;
};

struct sti_asc_serial {
	/* address of registers in physical memory */
	struct sti_asc_uart *regs;
};

/* Values for the BAUDRATE Register */
#define PCLK			(200ul * 1000000ul)
#define BAUDRATE_VAL_M0(bps)	(PCLK / (16 * (bps)))
#define BAUDRATE_VAL_M1(bps)	((bps * (1 << 14)) + (1<<13)) / (PCLK/(1 << 6))

/*
 * MODE 0
 *                       ICCLK
 * ASCBaudRate =   ----------------
 *                   baudrate * 16
 *
 * MODE 1
 *                   baudrate * 16 * 2^16
 * ASCBaudRate =   ------------------------
 *                          ICCLK
 *
 * NOTE:
 * Mode 1 should be used for baudrates of 19200, and above, as it
 * has a lower deviation error than Mode 0 for higher frequencies.
 * Mode 0 should be used for all baudrates below 19200.
 */

static int sti_asc_pending(struct udevice *dev, bool input)
{
	struct sti_asc_serial *priv = dev_get_priv(dev);
	struct sti_asc_uart *const uart = priv->regs;
	unsigned long status;

	status = readl(&uart->status);
	if (input)
		return status & STA_RBF;
	else
		return status & STA_TF;
}

static int _sti_asc_serial_setbrg(struct sti_asc_uart *uart, int baudrate)
{
	unsigned long val;
	int t, mode = 1;

	switch (baudrate) {
	case 9600:
		t = BAUDRATE_VAL_M0(9600);
		mode = 0;
		break;
	case 19200:
		t = BAUDRATE_VAL_M1(19200);
		break;
	case 38400:
		t = BAUDRATE_VAL_M1(38400);
		break;
	case 57600:
		t = BAUDRATE_VAL_M1(57600);
		break;
	default:
		debug("ASC: unsupported baud rate: %d, using 115200 instead.\n",
		      baudrate);
	case 115200:
		t = BAUDRATE_VAL_M1(115200);
		break;
	}

	/* disable the baudrate generator */
	val = readl(&uart->control);
	writel(val & ~RUN, &uart->control);

	/* set baud generator reload value */
	writel(t, &uart->baudrate);
	/* reset the RX & TX buffers */
	writel(1, &uart->txreset);
	writel(1, &uart->rxreset);

	/* set baud generator mode */
	if (mode)
		val |= BAUDMODE;

	/* finally, write value and enable ASC */
	writel(val, &uart->control);

	return 0;
}

/* called to adjust baud-rate */
static int sti_asc_serial_setbrg(struct udevice *dev, int baudrate)
{
	struct sti_asc_serial *priv = dev_get_priv(dev);
	struct sti_asc_uart *const uart = priv->regs;

	return _sti_asc_serial_setbrg(uart, baudrate);
}

/* blocking function, that returns next char */
static int sti_asc_serial_getc(struct udevice *dev)
{
	struct sti_asc_serial *priv = dev_get_priv(dev);
	struct sti_asc_uart *const uart = priv->regs;

	/* polling wait: for a char to be read */
	if (!sti_asc_pending(dev, true))
		return -EAGAIN;

	return readl(&uart->rxbuf);
}

/* write write out a single char */
static int sti_asc_serial_putc(struct udevice *dev, const char c)
{
	struct sti_asc_serial *priv = dev_get_priv(dev);
	struct sti_asc_uart *const uart = priv->regs;

	/* wait till safe to write next char */
	if (sti_asc_pending(dev, false))
		return -EAGAIN;

	/* finally, write next char */
	writel(c, &uart->txbuf);

	return 0;
}

/* initialize the ASC */
static int sti_asc_serial_probe(struct udevice *dev)
{
	struct sti_asc_serial *priv = dev_get_priv(dev);
	unsigned long val;
	fdt_addr_t base;

	base = dev_read_addr(dev);
	if (base == FDT_ADDR_T_NONE)
		return -EINVAL;

	priv->regs = (struct sti_asc_uart *)base;
	sti_asc_serial_setbrg(dev, gd->baudrate);

	/*
	 * build up the value to be written to CONTROL
	 * set character length, bit stop number, odd parity
	 */
	val = RXENABLE | RUN | MODE_8BIT | STOP_1BIT | PARITYODD;
	writel(val, &priv->regs->control);

	return 0;
}

static const struct dm_serial_ops sti_asc_serial_ops = {
	.putc = sti_asc_serial_putc,
	.pending = sti_asc_pending,
	.getc = sti_asc_serial_getc,
	.setbrg = sti_asc_serial_setbrg,
};

static const struct udevice_id sti_serial_of_match[] = {
	{ .compatible = "st,asc" },
	{ }
};

U_BOOT_DRIVER(serial_sti_asc) = {
	.name = "serial_sti_asc",
	.id = UCLASS_SERIAL,
	.of_match = sti_serial_of_match,
	.ops = &sti_asc_serial_ops,
	.probe = sti_asc_serial_probe,
	.priv_auto	= sizeof(struct sti_asc_serial),
};
