/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2022 Texas Instruments Incorporated - https://www.ti.com/
 */

#ifndef _ASM_ARCH_AM62A_SPL_H_
#define _ASM_ARCH_AM62A_SPL_H_

/* Primary BootMode devices */
#define BOOT_DEVICE_SPI_NAND		0x00
#define BOOT_DEVICE_RAM		0xFF
#define BOOT_DEVICE_OSPI		0x01
#define BOOT_DEVICE_QSPI		0x02
#define BOOT_DEVICE_SPI			0x03
#define BOOT_DEVICE_CPGMAC		0x04
#define BOOT_DEVICE_ETHERNET_RGMII	0x04
#define BOOT_DEVICE_ETHERNET_RMII	0x05
#define BOOT_DEVICE_I2C			0x06
#define BOOT_DEVICE_UART		0x07
#define BOOT_DEVICE_MMC			0x08
#define BOOT_DEVICE_EMMC		0x09

#define BOOT_DEVICE_USB			0x2A
#define BOOT_DEVICE_DFU			0x0A
#define BOOT_DEVICE_GPMC_NAND		0x0B
#define BOOT_DEVICE_GPMC_NOR		0x0C
#define BOOT_DEVICE_XSPI		0x0E
#define BOOT_DEVICE_NOBOOT		0x0F

/* U-Boot used aliases */
#define BOOT_DEVICE_ETHERNET		0x04
#define BOOT_DEVICE_SPINAND		0x10
#define BOOT_DEVICE_MMC2		0x08
#define BOOT_DEVICE_MMC1		0x09
/* Invalid */
#define BOOT_DEVICE_MMC2_2		0x1F

/* Backup BootMode devices */
#define BACKUP_BOOT_DEVICE_DFU		0x01
#define BACKUP_BOOT_DEVICE_UART		0x03
#define BACKUP_BOOT_DEVICE_ETHERNET	0x04
#define BACKUP_BOOT_DEVICE_MMC		0x05
#define BACKUP_BOOT_DEVICE_SPI		0x06
#define BACKUP_BOOT_DEVICE_I2C		0x07
#define BACKUP_BOOT_DEVICE_USB		0x09

#define K3_PRIMARY_BOOTMODE		0x0

#endif /* _ASM_ARCH_AM62A_SPL_H_ */
