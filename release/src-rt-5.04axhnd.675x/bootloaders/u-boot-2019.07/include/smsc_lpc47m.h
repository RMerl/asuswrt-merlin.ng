/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2014, Bin Meng <bmeng.cn@gmail.com>
 */

#ifndef _SMSC_LPC47M_H_
#define _SMSC_LPC47M_H_

/* I/O address of LPC47M */
#define LPC47M_IO_PORT	0x2e

/* Logical device number */
#define LPC47M_FDC	0	/* Floppy */
#define LPC47M_SP2	2	/* Serial Port 2 */
#define LPC47M_PP	3	/* Parallel Port */
#define LPC47M_SP1	4	/* Serial Port 1 */
#define LPC47M_KBC	7	/* Keyboard & Mouse */
#define LPC47M_PME	10	/* Power Control */

/**
 * Configure the base I/O port of the specified serial device and enable the
 * serial device.
 *
 * @dev: high 8 bits = super I/O port, low 8 bits = logical device number
 * @iobase: processor I/O port address to assign to this serial device
 * @irq: processor IRQ number to assign to this serial device
 */
void lpc47m_enable_serial(uint dev, uint iobase, uint irq);

/**
 * Configure the specified keyboard controller device and enable the keyboard
 * controller device.
 *
 * @dev: high 8 bits = Super I/O port, low 8 bits = logical device number
 * @irq0: processor IRQ number to assign to keyboard
 * @irq1: processor IRQ number to assign to mouse
 */
void lpc47m_enable_kbc(uint dev, uint irq0, uint irq1);

#endif /* _SMSC_LPC47M_H_ */
