/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2016 Google, Inc
 */

#ifndef __ASM_ARCH_PCH_H
#define __ASM_ARCH_PCH_H

#define PMBASE			0x40
#define ACPI_CNTL		0x44
#define  ACPI_EN		(1 << 7)

#define GPIO_BASE		0x48 /* LPC GPIO Base Address Register */
#define GPIO_CNTL		0x4C /* LPC GPIO Control Register */
#define  GPIO_EN		(1 << 4)

#define PCIEXBAR	0x60

#define  PCH_DEV_LPC		PCI_BDF(0, 0x1f, 0)

/* RCB registers */
#define OIC		0x31fe	/* 16bit */
#define HPTC		0x3404	/* 32bit */
#define FD		0x3418	/* 32bit */

/* Function Disable 1 RCBA 0x3418 */
#define PCH_DISABLE_ALWAYS	(1 << 0)

/* PM registers */
#define TCO1_CNT		0x60
#define  TCO_TMR_HLT		(1 << 11)


/* Device 0:0.0 PCI configuration space */

#define EPBAR		0x40
#define MCHBAR		0x48
#define PCIEXBAR	0x60
#define DMIBAR		0x68
#define GGC		0x50	/* GMCH Graphics Control */
#define DEVEN		0x54	/* Device Enable */
#define  DEVEN_D7EN	(1 << 14)
#define  DEVEN_D4EN	(1 << 7)
#define  DEVEN_D3EN	(1 << 5)
#define  DEVEN_D2EN	(1 << 4)
#define  DEVEN_D1F0EN	(1 << 3)
#define  DEVEN_D1F1EN	(1 << 2)
#define  DEVEN_D1F2EN	(1 << 1)
#define  DEVEN_D0EN	(1 << 0)
#define DPR		0x5c
#define  DPR_EPM	(1 << 2)
#define  DPR_PRS	(1 << 1)
#define  DPR_SIZE_MASK	0xff0

#define MCHBAR_PEI_VERSION	0x5034
#define BIOS_RESET_CPL		0x5da8
#define EDRAMBAR		0x5408
#define MCH_PAIR		0x5418
#define GDXCBAR			0x5420

#define PAM0		0x80
#define PAM1		0x81
#define PAM2		0x82
#define PAM3		0x83
#define PAM4		0x84
#define PAM5		0x85
#define PAM6		0x86

/* PCODE MMIO communications live in the MCHBAR. */
#define BIOS_MAILBOX_INTERFACE			0x5da4
#define  MAILBOX_RUN_BUSY			(1 << 31)
#define  MAILBOX_BIOS_CMD_READ_PCS		1
#define  MAILBOX_BIOS_CMD_WRITE_PCS		2
#define  MAILBOX_BIOS_CMD_READ_CALIBRATION	0x509
#define  MAILBOX_BIOS_CMD_FSM_MEASURE_INTVL	0x909
#define  MAILBOX_BIOS_CMD_READ_PCH_POWER	0xa
#define  MAILBOX_BIOS_CMD_READ_PCH_POWER_EXT	0xb
#define  MAILBOX_BIOS_CMD_READ_C9C10_VOLTAGE	0x26
#define  MAILBOX_BIOS_CMD_WRITE_C9C10_VOLTAGE	0x27
/* Errors are returned back in bits 7:0. */
#define  MAILBOX_BIOS_ERROR_NONE		0
#define  MAILBOX_BIOS_ERROR_INVALID_COMMAND	1
#define  MAILBOX_BIOS_ERROR_TIMEOUT		2
#define  MAILBOX_BIOS_ERROR_ILLEGAL_DATA	3
#define  MAILBOX_BIOS_ERROR_RESERVED		4
#define  MAILBOX_BIOS_ERROR_ILLEGAL_VR_ID	5
#define  MAILBOX_BIOS_ERROR_VR_INTERFACE_LOCKED	6
#define  MAILBOX_BIOS_ERROR_VR_ERROR		7
/* Data is passed through bits 31:0 of the data register. */
#define BIOS_MAILBOX_DATA			0x5da0

/* SATA IOBP Registers */
#define SATA_IOBP_SP0_SECRT88	0xea002688
#define SATA_IOBP_SP1_SECRT88	0xea002488

#define SATA_SECRT88_VADJ_MASK	0xff
#define SATA_SECRT88_VADJ_SHIFT	16

#define SATA_IOBP_SP0DTLE_DATA	0xea002550
#define SATA_IOBP_SP0DTLE_EDGE	0xea002554
#define SATA_IOBP_SP1DTLE_DATA	0xea002750
#define SATA_IOBP_SP1DTLE_EDGE	0xea002754

#define SATA_DTLE_MASK		0xF
#define SATA_DTLE_DATA_SHIFT	24
#define SATA_DTLE_EDGE_SHIFT	16

/* Power Management */
#define PCH_PCS			0x84
#define  PCH_PCS_PS_D3HOT	3

#define GEN_PMCON_1		0xa0
#define  SMI_LOCK		(1 << 4)
#define GEN_PMCON_2		0xa2
#define  SYSTEM_RESET_STS	(1 << 4)
#define  THERMTRIP_STS		(1 << 3)
#define  SYSPWR_FLR		(1 << 1)
#define  PWROK_FLR		(1 << 0)
#define GEN_PMCON_3		0xa4
#define  SUS_PWR_FLR		(1 << 14)
#define  GEN_RST_STS		(1 << 9)
#define  RTC_BATTERY_DEAD	(1 << 2)
#define  PWR_FLR		(1 << 1)
#define  SLEEP_AFTER_POWER_FAIL	(1 << 0)
#define GEN_PMCON_LOCK		0xa6
#define  SLP_STR_POL_LOCK	(1 << 2)
#define  ACPI_BASE_LOCK		(1 << 1)
#define PMIR			0xac
#define  PMIR_CF9LOCK		(1 << 31)
#define  PMIR_CF9GR		(1 << 20)

/* Broadwell PCH (Wildcat Point) */
#define PCH_WPT_HSW_U_SAMPLE	0x9cc1
#define PCH_WPT_BDW_U_SAMPLE	0x9cc2
#define PCH_WPT_BDW_U_PREMIUM	0x9cc3
#define PCH_WPT_BDW_U_BASE	0x9cc5
#define PCH_WPT_BDW_Y_SAMPLE	0x9cc6
#define PCH_WPT_BDW_Y_PREMIUM	0x9cc7
#define PCH_WPT_BDW_Y_BASE	0x9cc9
#define PCH_WPT_BDW_H		0x9ccb

#define SA_IGD_OPROM_VENDEV	0x80860406

/* Dynamically determine if the part is ULT */
bool cpu_is_ult(void);

u32 pch_iobp_read(u32 address);
int pch_iobp_write(u32 address, u32 data);
int pch_iobp_update(u32 address, u32 andvalue, u32 orvalue);
int  pch_iobp_exec(u32 addr, u16 op_dcode, u8 route_id, u32 *data, u8 *resp);

#endif
