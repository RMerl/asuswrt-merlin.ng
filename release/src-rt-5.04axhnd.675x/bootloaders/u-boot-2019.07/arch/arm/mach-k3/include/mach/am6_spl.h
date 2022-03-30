/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2018 Texas Instruments Incorporated - http://www.ti.com/
 *	Lokesh Vutla <lokeshvutla@ti.com>
 */
#ifndef _ASM_ARCH_AM6_SPL_H_
#define _ASM_ARCH_AM6_SPL_H_

#define BOOT_DEVICE_RAM			0x00
#define BOOT_DEVICE_OSPI		0x01
#define BOOT_DEVICE_QSPI		0x02
#define BOOT_DEVICE_HYPERFLASH		0x03
#define BOOT_DEVICE_SPI			0x04
#define BOOT_DEVICE_I2C			0x05
#define BOOT_DEVICE_MMC2		0x06
#define BOOT_DEVICE_ETHERNET		0x07
#define BOOT_DEVICE_USB			0x08
#define BOOT_DEVICE_PCIE		0x09
#define BOOT_DEVICE_UART		0x0a
#define BOOT_DEVICE_NAND		0x0c
#define BOOT_DEVICE_MMC1		0x0d
#define BOOT_DEVICE_MMC2_2		0x0e

#define BACKUP_BOOT_DEVICE_RAM		0x0
#define BACKUP_BOOT_DEVICE_USB		0x1
#define BACKUP_BOOT_DEVICE_UART		0x2
#define BACKUP_BOOT_DEVICE_ETHERNET	0x3
#define BACKUP_BOOT_DEVICE_MMC2		0x4
#define BACKUP_BOOT_DEVICE_SPI		0x5
#define BACKUP_BOOT_DEVICE_HYPERFLASH	0x6
#define BACKUP_BOOT_DEVICE_I2C		0x7

#define K3_PRIMARY_BOOTMODE		0x0
#define K3_BACKUP_BOOTMODE		0x1

#endif
