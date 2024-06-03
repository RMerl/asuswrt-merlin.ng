/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2013 Google Inc.
 * Copyright (C) 2016 Bin Meng <bmeng.cn@gmail.com>
 *
 * Modified from coreboot src/soc/intel/baytrail/include/soc/iomap.h
 */

#ifndef _BAYTRAIL_IOMAP_H_
#define _BAYTRAIL_IOMAP_H_

/* Memory Mapped IO bases */

/* PCI Configuration Space */
#define MCFG_BASE_ADDRESS		CONFIG_PCIE_ECAM_BASE
#define MCFG_BASE_SIZE			0x10000000

/* Temporary Base Address */
#define TEMP_BASE_ADDRESS		0xfd000000

/* Transactions in this range will abort */
#define ABORT_BASE_ADDRESS		0xfeb00000
#define ABORT_BASE_SIZE			0x00100000

/* High Performance Event Timer */
#define HPET_BASE_ADDRESS		0xfed00000
#define HPET_BASE_SIZE			0x400

/* SPI Bus */
#define SPI_BASE_ADDRESS		0xfed01000
#define SPI_BASE_SIZE			0x400

/* Power Management Controller */
#define PMC_BASE_ADDRESS		0xfed03000
#define PMC_BASE_SIZE			0x400

#define GEN_PMCON1			0x20
#define  UART_EN			(1 << 24)
#define  DISB				(1 << 23)
#define  MEM_SR				(1 << 21)
#define  SRS				(1 << 20)
#define  CTS				(1 << 19)
#define  MS4V				(1 << 18)
#define  PWR_FLR			(1 << 16)
#define  PME_B0_S5_DIS			(1 << 15)
#define  SUS_PWR_FLR			(1 << 14)
#define  WOL_EN_OVRD			(1 << 13)
#define  DIS_SLP_X_STRCH_SUS_UP		(1 << 12)
#define  GEN_RST_STS			(1 <<  9)
#define  RPS				(1 <<  2)
#define  AFTERG3_EN			(1 <<  0)
#define GEN_PMCON2			0x24
#define  SLPSX_STR_POL_LOCK		(1 << 18)
#define  BIOS_PCI_EXP_EN		(1 << 10)
#define  PWRBTN_LVL			(1 <<  9)
#define  SMI_LOCK			(1 <<  4)

/* Power Management Unit */
#define PUNIT_BASE_ADDRESS		0xfed05000
#define PUNIT_BASE_SIZE			0x800

/* Intel Legacy Block */
#define ILB_BASE_ADDRESS		0xfed08000
#define ILB_BASE_SIZE			0x400

/* IO Memory */
#define IO_BASE_ADDRESS			0xfed0c000
#define  IO_BASE_OFFSET_GPSCORE		0x0000
#define  IO_BASE_OFFSET_GPNCORE		0x1000
#define  IO_BASE_OFFSET_GPSSUS		0x2000
#define IO_BASE_SIZE			0x4000

/* Root Complex Base Address */
#define RCBA_BASE_ADDRESS		0xfed1c000
#define RCBA_BASE_SIZE			0x400

/* MODPHY */
#define MPHY_BASE_ADDRESS		0xfef00000
#define MPHY_BASE_SIZE			0x100000

/* IO Port bases */
#define ACPI_BASE_ADDRESS		0x0400
#define ACPI_BASE_SIZE			0x80

#define PM1_STS				0x00
#define PM1_CNT				0x04

#define GPIO_BASE_ADDRESS		0x0500
#define GPIO_BASE_SIZE			0x100

#define SMBUS_BASE_ADDRESS		0xefa0

#endif /* _BAYTRAIL_IOMAP_H_ */
