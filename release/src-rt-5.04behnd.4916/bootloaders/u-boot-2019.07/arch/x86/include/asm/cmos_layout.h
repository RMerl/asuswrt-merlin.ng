/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2017, Bin Meng <bmeng.cn@gmail.com>
 */

#ifndef __CMOS_LAYOUT_H
#define __CMOS_LAYOUT_H

/*
 * The RTC internal registers and RAM is organized as two banks of 128 bytes
 * each, called the standard and extended banks. The first 14 bytes of the
 * standard bank contain the RTC time and date information along with four
 * registers, A - D, that are used for configuration of the RTC. The extended
 * bank contains a full 128 bytes of battery backed SRAM.
 *
 * For simplicity in U-Boot we only support CMOS in the standard bank, and
 * its base address starts from offset 0x10, which leaves us 112 bytes space.
 */
#define CMOS_BASE		0x10

/*
 * The file records all offsets off CMOS_BASE that is currently used by
 * U-Boot for various reasons. It is put in such a unified place in order
 * to be consistent across platforms.
 */

/* stack address for S3 boot in a FSP configuration, 4 bytes */
#define CMOS_FSP_STACK_ADDR	CMOS_BASE

#endif /* __CMOS_LAYOUT_H */
