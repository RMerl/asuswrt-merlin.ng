/*
 * Early serial console for 8250/16550 devices
 *
 * (c) Copyright 2004 Hewlett-Packard Development Company, L.P.
 *	Bjorn Helgaas <bjorn.helgaas@hp.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Based on the 8250.c serial driver, Copyright (C) 2001 Russell King,
 * and on early_printk.c by Andi Kleen.
 *
 * This is for use before the serial driver has initialized, in
 * particular, before the UARTs have been discovered and named.
 * Instead of specifying the console device as, e.g., "ttyS0",
 * we locate the device directly by its MMIO or I/O port address.
 *
 * The user can specify the device directly, e.g.,
 *	earlycon=uart8250,io,0x3f8,9600n8
 *	earlycon=uart8250,mmio,0xff5e0000,115200n8
 *	earlycon=uart8250,mmio32,0xff5e0000,115200n8
 * or
 *	console=uart8250,io,0x3f8,9600n8
 *	console=uart8250,mmio,0xff5e0000,115200n8
 *	console=uart8250,mmio32,0xff5e0000,115200n8
 */

#include <linux/tty.h>
#include <linux/init.h>
#include <linux/console.h>
#include <linux/serial_reg.h>
#include <linux/serial.h>
#include <linux/serial_8250.h>
#include <asm/io.h>
#include <asm/serial.h>

unsigned int __weak __init serial8250_early_in(struct uart_port *port, int offset)
{
	switch (port->iotype) {
	case UPIO_MEM:
		return readb(port->membase + offset);
	case UPIO_MEM32:
		return readl(port->membase + (offset << 2));
	case UPIO_MEM32BE:
		return ioread32be(port->membase + (offset << 2));
	case UPIO_PORT:
		return inb(port->iobase + offset);
	default:
		return 0;
	}
}

void __weak __init serial8250_early_out(struct uart_port *port, int offset, int value)
{
	switch (port->iotype) {
	case UPIO_MEM:
		writeb(value, port->membase + offset);
		break;
	case UPIO_MEM32:
		writel(value, port->membase + (offset << 2));
		break;
	case UPIO_MEM32BE:
		iowrite32be(value, port->membase + (offset << 2));
		break;
	case UPIO_PORT:
		outb(value, port->iobase + offset);
		break;
	}
}

#define BOTH_EMPTY (UART_LSR_TEMT | UART_LSR_THRE)

static void __init wait_for_xmitr(struct uart_port *port)
{
	unsigned int status;

	for (;;) {
		status = serial8250_early_in(port, UART_LSR);
		if ((status & BOTH_EMPTY) == BOTH_EMPTY)
			return;
		cpu_relax();
	}
}

static void __init serial_putc(struct uart_port *port, int c)
{
	wait_for_xmitr(port);
	serial8250_early_out(port, UART_TX, c);
}

static void __init early_serial8250_write(struct console *console,
					const char *s, unsigned int count)
{
	struct earlycon_device *device = console->data;
	struct uart_port *port = &device->port;
	unsigned int ier;

	/* Save the IER and disable interrupts preserving the UUE bit */
	ier = serial8250_early_in(port, UART_IER);
	if (ier)
		serial8250_early_out(port, UART_IER, ier & UART_IER_UUE);

	uart_console_write(port, s, count, serial_putc);

	/* Wait for transmitter to become empty and restore the IER */
	wait_for_xmitr(port);

	if (ier)
		serial8250_early_out(port, UART_IER, ier);
}

static void __init init_port(struct earlycon_device *device)
{
	struct uart_port *port = &device->port;
	unsigned int divisor;
	unsigned char c;
	unsigned int ier;

	serial8250_early_out(port, UART_LCR, 0x3);	/* 8n1 */
	ier = serial8250_early_in(port, UART_IER);
	serial8250_early_out(port, UART_IER, ier & UART_IER_UUE); /* no interrupt */
	serial8250_early_out(port, UART_FCR, 0);	/* no fifo */
	serial8250_early_out(port, UART_MCR, 0x3);	/* DTR + RTS */

	divisor = DIV_ROUND_CLOSEST(port->uartclk, 16 * device->baud);
	c = serial8250_early_in(port, UART_LCR);
	serial8250_early_out(port, UART_LCR, c | UART_LCR_DLAB);
	serial8250_early_out(port, UART_DLL, divisor & 0xff);
	serial8250_early_out(port, UART_DLM, (divisor >> 8) & 0xff);
	serial8250_early_out(port, UART_LCR, c & ~UART_LCR_DLAB);
}

static int __init early_serial8250_setup(struct earlycon_device *device,
					 const char *options)
{
	if (!(device->port.membase || device->port.iobase))
		return -ENODEV;

	if (!device->baud) {
		struct uart_port *port = &device->port;
		unsigned int ier;

		/* assume the device was initialized, only mask interrupts */
		ier = serial8250_early_in(port, UART_IER);
		serial8250_early_out(port, UART_IER, ier & UART_IER_UUE);
	} else
		init_port(device);

	device->con->write = early_serial8250_write;
	return 0;
}
EARLYCON_DECLARE(uart8250, early_serial8250_setup);
EARLYCON_DECLARE(uart, early_serial8250_setup);
