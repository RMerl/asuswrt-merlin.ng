/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2015, Bin Meng <bmeng.cn@gmail.com>
 */

#ifndef _ARCH_QEMU_H_
#define _ARCH_QEMU_H_

/* Programmable Attribute Map (PAM) Registers */
#define I440FX_PAM		0x59
#define Q35_PAM			0x90
#define PAM_NUM			7
#define PAM_RW			0x33

/* X-Bus Chip Select Register */
#define XBCS			0x4e
#define APIC_EN			(1 << 8)

/* IDE Timing Register */
#define IDE0_TIM		0x40
#define IDE1_TIM		0x42
#define IDE_DECODE_EN		(1 << 15)

/* PCIe ECAM Base Address Register */
#define PCIEX_BAR		0x60
#define BAR_EN			(1 << 0)

/* I/O Ports */
#define CMOS_ADDR_PORT		0x70
#define CMOS_DATA_PORT		0x71

#define LOW_RAM_ADDR		0x34
#define HIGH_RAM_ADDR		0x35

/* PM registers */
#define PMBA		0x40
#define PMREGMISC	0x80
#define PMIOSE		(1 << 0)

#endif /* _ARCH_QEMU_H_ */
