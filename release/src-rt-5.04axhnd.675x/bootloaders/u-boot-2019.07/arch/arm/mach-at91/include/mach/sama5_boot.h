/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Boot mode definitions for the SAMA5Dx SoC
 *
 * Copyright (C) 2016 Marek Vasut <marex@denx.de>
 */

#ifndef __SAMA5_BOOT_H
#define __SAMA5_BOOT_H

/* Boot modes stored by BootROM in r4 */
#define ATMEL_SAMA5_BOOT_FROM_OFF	0
#define ATMEL_SAMA5_BOOT_FROM_MASK	0xf
#define ATMEL_SAMA5_BOOT_FROM_SPI	(0 << 0)
#define ATMEL_SAMA5_BOOT_FROM_MCI	(1 << 0)
#define ATMEL_SAMA5_BOOT_FROM_SMC	(2 << 0)
#define ATMEL_SAMA5_BOOT_FROM_TWI	(3 << 0)
#define ATMEL_SAMA5_BOOT_FROM_QSPI	(4 << 0)
#define ATMEL_SAMA5_BOOT_FROM_SAMBA	(7 << 0)

#define ATMEL_SAMA5_BOOT_DEV_ID_OFF	4
#define ATMEL_SAMA5_BOOT_DEV_ID_MASK	0xf

#endif /* __SAMA5_BOOT_H */
