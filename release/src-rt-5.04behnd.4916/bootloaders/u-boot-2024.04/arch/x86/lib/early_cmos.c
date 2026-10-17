// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2017, Bin Meng <bmeng.cn@gmail.com>
 */

/*
 * This library provides CMOS (inside RTC SRAM) access routines at a very
 * early stage when driver model is not available yet. Only read access is
 * provided. The 16-bit/32-bit read are compatible with driver model RTC
 * uclass write ops, that data is stored in little-endian mode.
 */

#include <common.h>
#include <asm/early_cmos.h>
#include <asm/io.h>

u8 cmos_read8(u8 addr)
{
	outb(addr, CMOS_IO_PORT);

	return inb(CMOS_IO_PORT + 1);
}

u16 cmos_read16(u8 addr)
{
	u16 value = 0;
	u16 data;
	int i;

	for (i = 0; i < sizeof(value); i++) {
		data = cmos_read8(addr + i);
		value |= data << (i << 3);
	}

	return value;
}

u32 cmos_read32(u8 addr)
{
	u32 value = 0;
	u32 data;
	int i;

	for (i = 0; i < sizeof(value); i++) {
		data = cmos_read8(addr + i);
		value |= data << (i << 3);
	}

	return value;
}
