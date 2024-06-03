/* SPDX-License-Identifier: GPL-2.0 */
/*
 * From coreboot file of the same name
 *
 * Copyright (C) 2010 coresystems GmbH
 */

#ifndef __ASM_IOAPIC_H
#define __ASM_IOAPIC_H

#define IO_APIC_ADDR		0xfec00000

/* Direct addressed register */
#define IO_APIC_INDEX		(IO_APIC_ADDR + 0x00)
#define IO_APIC_DATA		(IO_APIC_ADDR + 0x10)

/* Indirect addressed register offset */
#define IO_APIC_ID		0x00
#define IO_APIC_VER		0x01

/**
 * io_apic_read() - Read I/O APIC register
 *
 * This routine reads I/O APIC indirect addressed register.
 *
 * @reg:	address of indirect addressed register
 * @return:	register value to read
 */
u32 io_apic_read(u32 reg);

/**
 * io_apic_write() - Write I/O APIC register
 *
 * This routine writes I/O APIC indirect addressed register.
 *
 * @reg:	address of indirect addressed register
 * @val:	register value to write
 */
void io_apic_write(u32 reg, u32 val);

void io_apic_set_id(int ioapic_id);

#endif
