// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2011 Marek Vasut <marek.vasut@gmail.com>
 *
 * (C) Copyright 2002
 * Wolfgang Denk, DENX Software Engineering, <wd@denx.de>
 *
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 *
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Alex Zuepke <azu@sysgo.de>
 *
 * Copyright (C) 1999 2000 2001 Erik Mouw (J.A.K.Mouw@its.tudelft.nl)
 *
 * Modified to add driver model (DM) support
 * (C) Copyright 2016 Marcel Ziswiler <marcel.ziswiler@toradex.com>
 */

#include <common.h>
#include <asm/arch/pxa-regs.h>
#include <asm/arch/regs-uart.h>
#include <asm/io.h>
#include <dm.h>
#include <dm/platform_data/serial_pxa.h>
#include <linux/compiler.h>
#include <serial.h>
#include <watchdog.h>

DECLARE_GLOBAL_DATA_PTR;

static uint32_t pxa_uart_get_baud_divider(int baudrate)
{
	return 921600 / baudrate;
}

static void pxa_uart_toggle_clock(uint32_t uart_index, int enable)
{
	uint32_t clk_reg, clk_offset, reg;

	clk_reg = UART_CLK_REG;
	clk_offset = UART_CLK_BASE << uart_index;

	reg = readl(clk_reg);

	if (enable)
		reg |= clk_offset;
	else
		reg &= ~clk_offset;

	writel(reg, clk_reg);
}

/*
 * Enable clock and set baud rate, parity etc.
 */
void pxa_setbrg_common(struct pxa_uart_regs *uart_regs, int port, int baudrate)
{
	uint32_t divider = pxa_uart_get_baud_divider(baudrate);
	if (!divider)
		hang();


	pxa_uart_toggle_clock(port, 1);

	/* Disable interrupts and FIFOs */
	writel(0, &uart_regs->ier);
	writel(0, &uart_regs->fcr);

	/* Set baud rate */
	writel(LCR_WLS0 | LCR_WLS1 | LCR_DLAB, &uart_regs->lcr);
	writel(divider & 0xff, &uart_regs->dll);
	writel(divider >> 8, &uart_regs->dlh);
	writel(LCR_WLS0 | LCR_WLS1, &uart_regs->lcr);

	/* Enable UART */
	writel(IER_UUE, &uart_regs->ier);
}

#ifndef CONFIG_DM_SERIAL
static struct pxa_uart_regs *pxa_uart_index_to_regs(uint32_t uart_index)
{
	switch (uart_index) {
	case FFUART_INDEX: return (struct pxa_uart_regs *)FFUART_BASE;
	case BTUART_INDEX: return (struct pxa_uart_regs *)BTUART_BASE;
	case STUART_INDEX: return (struct pxa_uart_regs *)STUART_BASE;
	case HWUART_INDEX: return (struct pxa_uart_regs *)HWUART_BASE;
	default:
		return NULL;
	}
}

/*
 * Enable clock and set baud rate, parity etc.
 */
void pxa_setbrg_dev(uint32_t uart_index)
{
	struct pxa_uart_regs *uart_regs = pxa_uart_index_to_regs(uart_index);
	if (!uart_regs)
		panic("Failed getting UART registers\n");

	pxa_setbrg_common(uart_regs, uart_index, gd->baudrate);
}

/*
 * Initialise the serial port with the given baudrate. The settings
 * are always 8 data bits, no parity, 1 stop bit, no start bits.
 */
int pxa_init_dev(unsigned int uart_index)
{
	pxa_setbrg_dev(uart_index);
	return 0;
}

/*
 * Output a single byte to the serial port.
 */
void pxa_putc_dev(unsigned int uart_index, const char c)
{
	struct pxa_uart_regs *uart_regs;

	/* If \n, also do \r */
	if (c == '\n')
		pxa_putc_dev(uart_index, '\r');

	uart_regs = pxa_uart_index_to_regs(uart_index);
	if (!uart_regs)
		hang();

	while (!(readl(&uart_regs->lsr) & LSR_TEMT))
		WATCHDOG_RESET();
	writel(c, &uart_regs->thr);
}

/*
 * Read a single byte from the serial port. Returns 1 on success, 0
 * otherwise. When the function is succesfull, the character read is
 * written into its argument c.
 */
int pxa_tstc_dev(unsigned int uart_index)
{
	struct pxa_uart_regs *uart_regs;

	uart_regs = pxa_uart_index_to_regs(uart_index);
	if (!uart_regs)
		return -1;

	return readl(&uart_regs->lsr) & LSR_DR;
}

/*
 * Read a single byte from the serial port. Returns 1 on success, 0
 * otherwise. When the function is succesfull, the character read is
 * written into its argument c.
 */
int pxa_getc_dev(unsigned int uart_index)
{
	struct pxa_uart_regs *uart_regs;

	uart_regs = pxa_uart_index_to_regs(uart_index);
	if (!uart_regs)
		return -1;

	while (!(readl(&uart_regs->lsr) & LSR_DR))
		WATCHDOG_RESET();
	return readl(&uart_regs->rbr) & 0xff;
}

void pxa_puts_dev(unsigned int uart_index, const char *s)
{
	while (*s)
		pxa_putc_dev(uart_index, *s++);
}

#define	pxa_uart(uart, UART)						\
	int uart##_init(void)						\
	{								\
		return pxa_init_dev(UART##_INDEX);			\
	}								\
									\
	void uart##_setbrg(void)					\
	{								\
		return pxa_setbrg_dev(UART##_INDEX);			\
	}								\
									\
	void uart##_putc(const char c)					\
	{								\
		return pxa_putc_dev(UART##_INDEX, c);			\
	}								\
									\
	void uart##_puts(const char *s)					\
	{								\
		return pxa_puts_dev(UART##_INDEX, s);			\
	}								\
									\
	int uart##_getc(void)						\
	{								\
		return pxa_getc_dev(UART##_INDEX);			\
	}								\
									\
	int uart##_tstc(void)						\
	{								\
		return pxa_tstc_dev(UART##_INDEX);			\
	}								\

#define	pxa_uart_desc(uart)						\
	struct serial_device serial_##uart##_device =			\
	{								\
		.name	= "serial_"#uart,				\
		.start	= uart##_init,					\
		.stop	= NULL,						\
		.setbrg	= uart##_setbrg,				\
		.getc	= uart##_getc,					\
		.tstc	= uart##_tstc,					\
		.putc	= uart##_putc,					\
		.puts	= uart##_puts,					\
	};

#define	pxa_uart_multi(uart, UART)					\
	pxa_uart(uart, UART)						\
	pxa_uart_desc(uart)

#if defined(CONFIG_HWUART)
	pxa_uart_multi(hwuart, HWUART)
#endif
#if defined(CONFIG_STUART)
	pxa_uart_multi(stuart, STUART)
#endif
#if defined(CONFIG_FFUART)
	pxa_uart_multi(ffuart, FFUART)
#endif
#if defined(CONFIG_BTUART)
	pxa_uart_multi(btuart, BTUART)
#endif

__weak struct serial_device *default_serial_console(void)
{
#if CONFIG_CONS_INDEX == 1
	return &serial_hwuart_device;
#elif CONFIG_CONS_INDEX == 2
	return &serial_stuart_device;
#elif CONFIG_CONS_INDEX == 3
	return &serial_ffuart_device;
#elif CONFIG_CONS_INDEX == 4
	return &serial_btuart_device;
#else
#error "Bad CONFIG_CONS_INDEX."
#endif
}

void pxa_serial_initialize(void)
{
#if defined(CONFIG_FFUART)
	serial_register(&serial_ffuart_device);
#endif
#if defined(CONFIG_BTUART)
	serial_register(&serial_btuart_device);
#endif
#if defined(CONFIG_STUART)
	serial_register(&serial_stuart_device);
#endif
}
#endif /* CONFIG_DM_SERIAL */

#ifdef CONFIG_DM_SERIAL
static int pxa_serial_probe(struct udevice *dev)
{
	struct pxa_serial_platdata *plat = dev->platdata;

	pxa_setbrg_common((struct pxa_uart_regs *)plat->base, plat->port,
			  plat->baudrate);
	return 0;
}

static int pxa_serial_putc(struct udevice *dev, const char ch)
{
	struct pxa_serial_platdata *plat = dev->platdata;
	struct pxa_uart_regs *uart_regs = (struct pxa_uart_regs *)plat->base;

	/* Wait for last character to go. */
	if (!(readl(&uart_regs->lsr) & LSR_TEMT))
		return -EAGAIN;

	writel(ch, &uart_regs->thr);

	return 0;
}

static int pxa_serial_getc(struct udevice *dev)
{
	struct pxa_serial_platdata *plat = dev->platdata;
	struct pxa_uart_regs *uart_regs = (struct pxa_uart_regs *)plat->base;

	/* Wait for a character to arrive. */
	if (!(readl(&uart_regs->lsr) & LSR_DR))
		return -EAGAIN;

	return readl(&uart_regs->rbr) & 0xff;
}

int pxa_serial_setbrg(struct udevice *dev, int baudrate)
{
	struct pxa_serial_platdata *plat = dev->platdata;
	struct pxa_uart_regs *uart_regs = (struct pxa_uart_regs *)plat->base;
	int port = plat->port;

	pxa_setbrg_common(uart_regs, port, baudrate);

	return 0;
}

static int pxa_serial_pending(struct udevice *dev, bool input)
{
	struct pxa_serial_platdata *plat = dev->platdata;
	struct pxa_uart_regs *uart_regs = (struct pxa_uart_regs *)plat->base;

	if (input)
		return readl(&uart_regs->lsr) & LSR_DR ? 1 : 0;
	else
		return readl(&uart_regs->lsr) & LSR_TEMT ? 0 : 1;

	return 0;
}

static const struct dm_serial_ops pxa_serial_ops = {
	.putc		= pxa_serial_putc,
	.pending	= pxa_serial_pending,
	.getc		= pxa_serial_getc,
	.setbrg		= pxa_serial_setbrg,
};

U_BOOT_DRIVER(serial_pxa) = {
	.name	= "serial_pxa",
	.id	= UCLASS_SERIAL,
	.probe	= pxa_serial_probe,
	.ops	= &pxa_serial_ops,
	.flags	= DM_FLAG_PRE_RELOC,
};
#endif /* CONFIG_DM_SERIAL */
