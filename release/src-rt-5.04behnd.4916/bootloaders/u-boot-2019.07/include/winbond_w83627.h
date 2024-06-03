/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2016 Stefan Roese <sr@denx.de>
 */

#ifndef _WINBOND_W83627_H_
#define _WINBOND_W83627_H_

/* I/O address of Winbond Super IO chip */
#define WINBOND_IO_PORT		0x2e

/* Logical device number */
#define W83627DHG_FDC		0	/* Floppy */
#define W83627DHG_PP		1	/* Parallel port */
#define W83627DHG_SP1		2	/* Com1 */
#define W83627DHG_SP2		3	/* Com2 */
#define W83627DHG_KBC		5	/* PS/2 keyboard & mouse */
#define W83627DHG_SPI		6	/* Serial peripheral interface */
#define W83627DHG_WDTO_PLED	8	/* WDTO#, PLED */
#define W83627DHG_ACPI		10	/* ACPI */
#define W83627DHG_HWM		11	/* Hardware monitor */
#define W83627DHG_PECI_SST	12	/* PECI, SST */

/**
 * Configure the base I/O port of the specified serial device and enable the
 * serial device.
 *
 * @dev: high 8 bits = super I/O port, low 8 bits = logical device number
 * @iobase: processor I/O port address to assign to this serial device
 * @irq: processor IRQ number to assign to this serial device
 */
void winbond_enable_serial(uint dev, uint iobase, uint irq);

#endif /* _WINBOND_W83627_H_ */
