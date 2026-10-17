/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2017, Bin Meng <bmeng.cn@gmail.com>
 */

#ifndef _BRASWELL_IOMAP_H_
#define _BRASWELL_IOMAP_H_

/* Memory Mapped IO bases */

/* Power Management Controller */
#define PMC_BASE_ADDRESS	0xfed03000
#define PMC_BASE_SIZE		0x400

/* Power Management Unit */
#define PUNIT_BASE_ADDRESS	0xfed05000
#define PUNIT_BASE_SIZE		0x800

/* Intel Legacy Block */
#define ILB_BASE_ADDRESS	0xfed08000
#define ILB_BASE_SIZE		0x400

/* SPI Bus */
#define SPI_BASE_ADDRESS	0xfed01000
#define SPI_BASE_SIZE		0x400

/* Root Complex Base Address */
#define RCBA_BASE_ADDRESS	0xfed1c000
#define RCBA_BASE_SIZE		0x400

/* IO Memory */
#define IO_BASE_ADDRESS		0xfed80000
#define IO_BASE_SIZE		0x4000

/* MODPHY */
#define MPHY_BASE_ADDRESS	0xfef00000
#define MPHY_BASE_SIZE		0x100000

/* IO Port bases */

#define ACPI_BASE_ADDRESS	0x400
#define ACPI_BASE_SIZE		0x80

#define GPIO_BASE_ADDRESS	0x500
#define GPIO_BASE_SIZE		0x100

#define SMBUS_BASE_ADDRESS	0xefa0

#endif /* _BRASWELL_IOMAP_H_ */
