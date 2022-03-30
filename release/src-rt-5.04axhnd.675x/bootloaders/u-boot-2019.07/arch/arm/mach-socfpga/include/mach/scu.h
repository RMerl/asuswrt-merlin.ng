/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2014 Marek Vasut <marex@denx.de>
 */

#ifndef __SOCFPGA_SCU_H__
#define __SOCFPGA_SCU_H__

struct scu_registers {
	u32	ctrl;			/* 0x00 */
	u32	cfg;
	u32	cpsr;
	u32	iassr;
	u32	_pad_0x10_0x3c[12];	/* 0x10 */
	u32	fsar;			/* 0x40 */
	u32	fear;
	u32	_pad_0x48_0x4c[2];
	u32	acr;			/* 0x50 */
	u32	sacr;
};

#endif	/* __SOCFPGA_SCU_H__ */
