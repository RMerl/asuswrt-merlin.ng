// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2004-2006 Atmel Corporation
 *
 * Modified to support C structur SoC access by
 * Andreas Bießmann <biessmann@corscience.de>
 */
#include <common.h>
#include <clk.h>
#include <dm.h>
#include <errno.h>
#include <watchdog.h>
#include <serial.h>
#include <debug_uart.h>
#include <linux/compiler.h>

#include <asm/io.h>
#ifdef CONFIG_DM_SERIAL
#include <asm/arch/atmel_serial.h>
#endif
#include <asm/arch/clk.h>
#include <asm/arch/hardware.h>

#include "atmel_usart.h"

DECLARE_GLOBAL_DATA_PTR;

#ifndef CONFIG_DM_SERIAL
static void atmel_serial_setbrg_internal(atmel_usart3_t *usart, int id,
					 int baudrate)
{
	unsigned long divisor;
	unsigned long usart_hz;

	/*
	 *              Master Clock
	 * Baud Rate = --------------
	 *                16 * CD
	 */
	usart_hz = get_usart_clk_rate(id);
	divisor = (usart_hz / 16 + baudrate / 2) / baudrate;
	writel(USART3_BF(CD, divisor), &usart->brgr);
}

static void atmel_serial_init_internal(atmel_usart3_t *usart)
{
	/*
	 * Just in case: drain transmitter register
	 * 1000us is enough for baudrate >= 9600
	 */
	if (!(readl(&usart->csr) & USART3_BIT(TXEMPTY)))
		__udelay(1000);

	writel(USART3_BIT(RSTRX) | USART3_BIT(RSTTX), &usart->cr);
}

static void atmel_serial_activate(atmel_usart3_t *usart)
{
	writel((USART3_BF(USART_MODE, USART3_USART_MODE_NORMAL)
			   | USART3_BF(USCLKS, USART3_USCLKS_MCK)
			   | USART3_BF(CHRL, USART3_CHRL_8)
			   | USART3_BF(PAR, USART3_PAR_NONE)
			   | USART3_BF(NBSTOP, USART3_NBSTOP_1)),
			   &usart->mr);
	writel(USART3_BIT(RXEN) | USART3_BIT(TXEN), &usart->cr);
	/* 100us is enough for the new settings to be settled */
	__udelay(100);
}

static void atmel_serial_setbrg(void)
{
	atmel_serial_setbrg_internal((atmel_usart3_t *)CONFIG_USART_BASE,
				     CONFIG_USART_ID, gd->baudrate);
}

static int atmel_serial_init(void)
{
	atmel_usart3_t *usart = (atmel_usart3_t *)CONFIG_USART_BASE;

	atmel_serial_init_internal(usart);
	serial_setbrg();
	atmel_serial_activate(usart);

	return 0;
}

static void atmel_serial_putc(char c)
{
	atmel_usart3_t *usart = (atmel_usart3_t *)CONFIG_USART_BASE;

	if (c == '\n')
		serial_putc('\r');

	while (!(readl(&usart->csr) & USART3_BIT(TXRDY)));
	writel(c, &usart->thr);
}

static int atmel_serial_getc(void)
{
	atmel_usart3_t *usart = (atmel_usart3_t *)CONFIG_USART_BASE;

	while (!(readl(&usart->csr) & USART3_BIT(RXRDY)))
		 WATCHDOG_RESET();
	return readl(&usart->rhr);
}

static int atmel_serial_tstc(void)
{
	atmel_usart3_t *usart = (atmel_usart3_t *)CONFIG_USART_BASE;
	return (readl(&usart->csr) & USART3_BIT(RXRDY)) != 0;
}

static struct serial_device atmel_serial_drv = {
	.name	= "atmel_serial",
	.start	= atmel_serial_init,
	.stop	= NULL,
	.setbrg	= atmel_serial_setbrg,
	.putc	= atmel_serial_putc,
	.puts	= default_serial_puts,
	.getc	= atmel_serial_getc,
	.tstc	= atmel_serial_tstc,
};

void atmel_serial_initialize(void)
{
	serial_register(&atmel_serial_drv);
}

__weak struct serial_device *default_serial_console(void)
{
	return &atmel_serial_drv;
}
#endif

#ifdef CONFIG_DM_SERIAL
enum serial_clk_type {
	CLK_TYPE_NORMAL = 0,
	CLK_TYPE_DBGU,
};

struct atmel_serial_priv {
	atmel_usart3_t *usart;
	ulong usart_clk_rate;
};

static void _atmel_serial_set_brg(atmel_usart3_t *usart,
				  ulong usart_clk_rate, int baudrate)
{
	unsigned long divisor;

	divisor = (usart_clk_rate / 16 + baudrate / 2) / baudrate;
	writel(USART3_BF(CD, divisor), &usart->brgr);
}

void _atmel_serial_init(atmel_usart3_t *usart,
			ulong usart_clk_rate, int baudrate)
{
	writel(USART3_BIT(RXDIS) | USART3_BIT(TXDIS), &usart->cr);

	writel((USART3_BF(USART_MODE, USART3_USART_MODE_NORMAL) |
		USART3_BF(USCLKS, USART3_USCLKS_MCK) |
		USART3_BF(CHRL, USART3_CHRL_8) |
		USART3_BF(PAR, USART3_PAR_NONE) |
		USART3_BF(NBSTOP, USART3_NBSTOP_1)), &usart->mr);

	_atmel_serial_set_brg(usart, usart_clk_rate, baudrate);

	writel(USART3_BIT(RSTRX) | USART3_BIT(RSTTX), &usart->cr);
	writel(USART3_BIT(RXEN) | USART3_BIT(TXEN), &usart->cr);
}

int atmel_serial_setbrg(struct udevice *dev, int baudrate)
{
	struct atmel_serial_priv *priv = dev_get_priv(dev);

	_atmel_serial_set_brg(priv->usart, priv->usart_clk_rate, baudrate);

	return 0;
}

static int atmel_serial_getc(struct udevice *dev)
{
	struct atmel_serial_priv *priv = dev_get_priv(dev);

	if (!(readl(&priv->usart->csr) & USART3_BIT(RXRDY)))
		return -EAGAIN;

	return readl(&priv->usart->rhr);
}

static int atmel_serial_putc(struct udevice *dev, const char ch)
{
	struct atmel_serial_priv *priv = dev_get_priv(dev);

	if (!(readl(&priv->usart->csr) & USART3_BIT(TXRDY)))
		return -EAGAIN;

	writel(ch, &priv->usart->thr);

	return 0;
}

static int atmel_serial_pending(struct udevice *dev, bool input)
{
	struct atmel_serial_priv *priv = dev_get_priv(dev);
	uint32_t csr = readl(&priv->usart->csr);

	if (input)
		return csr & USART3_BIT(RXRDY) ? 1 : 0;
	else
		return csr & USART3_BIT(TXEMPTY) ? 0 : 1;
}

static const struct dm_serial_ops atmel_serial_ops = {
	.putc = atmel_serial_putc,
	.pending = atmel_serial_pending,
	.getc = atmel_serial_getc,
	.setbrg = atmel_serial_setbrg,
};

#if defined(CONFIG_SPL_BUILD) && !defined(CONFIG_SPL_CLK)
static int atmel_serial_enable_clk(struct udevice *dev)
{
	struct atmel_serial_priv *priv = dev_get_priv(dev);

	/* Use fixed clock value in SPL */
	priv->usart_clk_rate = CONFIG_SPL_UART_CLOCK;

	return 0;
}
#else
static int atmel_serial_enable_clk(struct udevice *dev)
{
	struct atmel_serial_priv *priv = dev_get_priv(dev);
	struct clk clk;
	ulong clk_rate;
	int ret;

	ret = clk_get_by_index(dev, 0, &clk);
	if (ret)
		return -EINVAL;

	if (dev_get_driver_data(dev) == CLK_TYPE_NORMAL) {
		ret = clk_enable(&clk);
		if (ret)
			return ret;
	}

	clk_rate = clk_get_rate(&clk);
	if (!clk_rate)
		return -EINVAL;

	priv->usart_clk_rate = clk_rate;

	clk_free(&clk);

	return 0;
}
#endif

static int atmel_serial_probe(struct udevice *dev)
{
	struct atmel_serial_platdata *plat = dev->platdata;
	struct atmel_serial_priv *priv = dev_get_priv(dev);
	int ret;
#if CONFIG_IS_ENABLED(OF_CONTROL)
	fdt_addr_t addr_base;

	addr_base = devfdt_get_addr(dev);
	if (addr_base == FDT_ADDR_T_NONE)
		return -ENODEV;

	plat->base_addr = (uint32_t)addr_base;
#endif
	priv->usart = (atmel_usart3_t *)plat->base_addr;

	ret = atmel_serial_enable_clk(dev);
	if (ret)
		return ret;

	_atmel_serial_init(priv->usart, priv->usart_clk_rate, gd->baudrate);

	return 0;
}

#if CONFIG_IS_ENABLED(OF_CONTROL)
static const struct udevice_id atmel_serial_ids[] = {
	{
		.compatible = "atmel,at91sam9260-dbgu",
		.data = CLK_TYPE_DBGU,
	},
	{
		.compatible = "atmel,at91sam9260-usart",
		.data = CLK_TYPE_NORMAL,
	},
	{ }
};
#endif

U_BOOT_DRIVER(serial_atmel) = {
	.name	= "serial_atmel",
	.id	= UCLASS_SERIAL,
#if CONFIG_IS_ENABLED(OF_CONTROL)
	.of_match = atmel_serial_ids,
	.platdata_auto_alloc_size = sizeof(struct atmel_serial_platdata),
#endif
	.probe = atmel_serial_probe,
	.ops	= &atmel_serial_ops,
#if !CONFIG_IS_ENABLED(OF_CONTROL)
	.flags = DM_FLAG_PRE_RELOC,
#endif
	.priv_auto_alloc_size	= sizeof(struct atmel_serial_priv),
};
#endif

#ifdef CONFIG_DEBUG_UART_ATMEL
static inline void _debug_uart_init(void)
{
	atmel_usart3_t *usart = (atmel_usart3_t *)CONFIG_DEBUG_UART_BASE;

	_atmel_serial_init(usart, CONFIG_DEBUG_UART_CLOCK, CONFIG_BAUDRATE);
}

static inline void _debug_uart_putc(int ch)
{
	atmel_usart3_t *usart = (atmel_usart3_t *)CONFIG_DEBUG_UART_BASE;

	while (!(readl(&usart->csr) & USART3_BIT(TXRDY)))
		;

	writel(ch, &usart->thr);
}

DEBUG_UART_FUNCS
#endif
