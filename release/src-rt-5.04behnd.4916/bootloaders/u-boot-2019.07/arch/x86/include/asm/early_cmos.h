/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2017, Bin Meng <bmeng.cn@gmail.com>
 */

#ifndef __EARLY_CMOS_H
#define __EARLY_CMOS_H

/* CMOS actually resides in the RTC SRAM */
#define CMOS_IO_PORT	0x70

/**
 * cmos_read8() - Get 8-bit data stored at the given address
 *
 * This reads from CMOS for the 8-bit data stored at the given address.
 *
 * @addr:	RTC SRAM address
 * @return:	8-bit data stored at the given address
 */
u8 cmos_read8(u8 addr);

/**
 * cmos_read16() - Get 16-bit data stored at the given address
 *
 * This reads from CMOS for the 16-bit data stored at the given address.
 *
 * @addr:	RTC SRAM address
 * @return:	16-bit data stored at the given address
 */
u16 cmos_read16(u8 addr);

/**
 * cmos_read32() - Get 32-bit data stored at the given address
 *
 * This reads from CMOS for the 32-bit data stored at the given address.
 *
 * @addr:	RTC SRAM address
 * @return:	32-bit data stored at the given address
 */
u32 cmos_read32(u8 addr);

#endif /* __EARLY_CMOS_H */
