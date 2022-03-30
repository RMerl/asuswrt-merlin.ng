/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2014 Google, Inc
 *
 * From Coreboot file of the same name
 *
 * Copyright (C) 2007-2008 coresystems GmbH
 * Copyright (C) 2011 Google Inc.
 */

#ifndef _ACH_ASM_SANDYBRIDGE_H
#define _ACH_ASM_SANDYBRIDGE_H

/* Chipset types */
#define SANDYBRIDGE_MOBILE	0
#define SANDYBRIDGE_DESKTOP	1
#define SANDYBRIDGE_SERVER	2

/* Device ID for SandyBridge and IvyBridge */
#define BASE_REV_SNB	0x00
#define BASE_REV_IVB	0x50
#define BASE_REV_MASK	0x50

/* SandyBridge CPU stepping */
#define SNB_STEP_D0	(BASE_REV_SNB + 5) /* Also J0 */
#define SNB_STEP_D1	(BASE_REV_SNB + 6)
#define SNB_STEP_D2	(BASE_REV_SNB + 7) /* Also J1/Q0 */

/* IvyBridge CPU stepping */
#define IVB_STEP_A0	(BASE_REV_IVB + 0)
#define IVB_STEP_B0	(BASE_REV_IVB + 2)
#define IVB_STEP_C0	(BASE_REV_IVB + 4)
#define IVB_STEP_K0	(BASE_REV_IVB + 5)
#define IVB_STEP_D0	(BASE_REV_IVB + 6)

/* Intel Enhanced Debug region must be 4MB */
#define IED_SIZE	0x400000

/* Northbridge BARs */
#define DEFAULT_DMIBAR		0xfed18000	/* 4 KB */
#define DEFAULT_EPBAR		0xfed19000	/* 4 KB */
#define DEFAULT_RCBABASE	0xfed1c000
/* 4 KB per PCIe device */
#define DEFAULT_PCIEXBAR	CONFIG_PCIE_ECAM_BASE

#define IOMMU_BASE1		0xfed90000ULL
#define IOMMU_BASE2		0xfed91000ULL

/* Device 0:0.0 PCI configuration space (Host Bridge) */
#define EPBAR		0x40
#define MCHBAR		0x48
#define PCIEXBAR	0x60
#define DMIBAR		0x68
#define X60BAR		0x60

#define GGC		0x50			/* GMCH Graphics Control */

#define DEVEN		0x54			/* Device Enable */
#define  DEVEN_PEG60	(1 << 13)
#define  DEVEN_IGD	(1 << 4)
#define  DEVEN_PEG10	(1 << 3)
#define  DEVEN_PEG11	(1 << 2)
#define  DEVEN_PEG12	(1 << 1)
#define  DEVEN_HOST	(1 << 0)

#define PAM0		0x80
#define PAM1		0x81
#define PAM2		0x82
#define PAM3		0x83
#define PAM4		0x84
#define PAM5		0x85
#define PAM6		0x86

#define LAC		0x87	/* Legacy Access Control */
#define SMRAM		0x88	/* System Management RAM Control */
#define  D_OPEN		(1 << 6)
#define  D_CLS		(1 << 5)
#define  D_LCK		(1 << 4)
#define  G_SMRAME	(1 << 3)
#define  C_BASE_SEG	((0 << 2) | (1 << 1) | (0 << 0))

#define TOM		0xa0
#define TOUUD		0xa8	/* Top of Upper Usable DRAM */
#define TSEG		0xb8	/* TSEG base */
#define TOLUD		0xbc	/* Top of Low Used Memory */

#define SKPAD		0xdc	/* Scratchpad Data */

/* Device 0:1.0 PCI configuration space (PCI Express) */
#define BCTRL1		0x3e	/* 16bit */

/* Device 0:2.0 PCI configuration space (Graphics Device) */

#define MSAC		0x62	/* Multi Size Aperture Control */
#define SWSCI		0xe8	/* SWSCI  enable */
#define ASLS		0xfc	/* OpRegion Base */

/*
 * MCHBAR
 */
#define SSKPD		0x5d14	/* 16bit (scratchpad) */
#define BIOS_RESET_CPL	0x5da8	/* 8bit */

/*
 * DMIBAR
 */

#define DMIBAR_REG(x)	(DEFAULT_DMIBAR + x)

/**
 * bridge_silicon_revision() - Get the Northbridge revision
 *
 * @dev:	Northbridge device
 * @return revision ID (bits 3:0) and bridge ID (bits 7:4)
 */
int bridge_silicon_revision(struct udevice *dev);

#endif
