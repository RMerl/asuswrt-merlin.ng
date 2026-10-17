// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2015, Bin Meng <bmeng.cn@gmail.com>
 */

#include <common.h>
#include <log.h>
#include <asm/io.h>
#include <asm/ioapic.h>
#include <asm/lapic.h>

u32 io_apic_read(u32 reg)
{
	writel(reg, IO_APIC_INDEX);
	return readl(IO_APIC_DATA);
}

void io_apic_write(u32 reg, u32 val)
{
	writel(reg, IO_APIC_INDEX);
	writel(val, IO_APIC_DATA);
}

void io_apic_set_id(int ioapic_id)
{
	int bsp_lapicid = lapicid();

	debug("IOAPIC: Initialising IOAPIC at %08x\n", IO_APIC_ADDR);
	debug("IOAPIC: Bootstrap Processor Local APIC = %#02x\n", bsp_lapicid);

	if (ioapic_id) {
		debug("IOAPIC: ID = 0x%02x\n", ioapic_id);
		/* Set IOAPIC ID if it has been specified */
		io_apic_write(0x00, (io_apic_read(0x00) & 0xf0ffffff) |
			      (ioapic_id << 24));
	}
}
