/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2012
 * Texas Instruments, <www.ti.com>
 */
#ifndef	_ASM_ARCH_SPL_H_
#define	_ASM_ARCH_SPL_H_

#define BOOT_DEVICE_NONE	0x00
#define BOOT_DEVICE_XIP		0x01
#define BOOT_DEVICE_XIPWAIT	0x02
#define BOOT_DEVICE_NAND	0x03
#define BOOT_DEVICE_ONENAND	0x04
#define BOOT_DEVICE_MMC1	0x05
#define BOOT_DEVICE_MMC2	0x06
#define BOOT_DEVICE_MMC2_2	0x07
#define BOOT_DEVICE_UART	0x43
#define BOOT_DEVICE_USB		0x45

#define MMC_BOOT_DEVICES_START	BOOT_DEVICE_MMC1
#define MMC_BOOT_DEVICES_END	BOOT_DEVICE_MMC2_2
#endif
