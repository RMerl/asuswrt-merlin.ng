/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2016 Bin Meng <bmeng.cn@gmail.com>
 */

#ifndef _QUARK_IOMAP_H_
#define _QUARK_IOMAP_H_

/* Memory Mapped IO bases */

/* ESRAM */
#define ESRAM_BASE_ADDRESS		CONFIG_ESRAM_BASE
#define ESRAM_BASE_SIZE			ESRAM_SIZE

/* PCI Configuration Space */
#define MCFG_BASE_ADDRESS		CONFIG_PCIE_ECAM_BASE
#define MCFG_BASE_SIZE			0x10000000

/* High Performance Event Timer */
#define HPET_BASE_ADDRESS		0xfed00000
#define HPET_BASE_SIZE			0x400

/* Root Complex Base Address */
#define RCBA_BASE_ADDRESS		CONFIG_RCBA_BASE
#define RCBA_BASE_SIZE			0x4000

/* IO Port bases */
#define ACPI_PM1_BASE_ADDRESS		CONFIG_ACPI_PM1_BASE
#define ACPI_PM1_BASE_SIZE		0x10

#define ACPI_PBLK_BASE_ADDRESS		CONFIG_ACPI_PBLK_BASE
#define ACPI_PBLK_BASE_SIZE		0x10

#define SPI_DMA_BASE_ADDRESS		CONFIG_SPI_DMA_BASE
#define SPI_DMA_BASE_SIZE		0x10

#define GPIO_BASE_ADDRESS		CONFIG_GPIO_BASE
#define GPIO_BASE_SIZE			0x80

#define ACPI_GPE0_BASE_ADDRESS		CONFIG_ACPI_GPE0_BASE
#define ACPI_GPE0_BASE_SIZE		0x40

#define WDT_BASE_ADDRESS		CONFIG_WDT_BASE
#define WDT_BASE_SIZE			0x40

#endif /* _QUARK_IOMAP_H_ */
