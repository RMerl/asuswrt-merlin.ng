/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2014 Google, Inc
 *
 * From Coreboot src/southbridge/intel/bd82x6x/pch.h
 *
 * Copyright (C) 2008-2009 coresystems GmbH
 * Copyright (C) 2012 The Chromium OS Authors.  All rights reserved.
 */

#ifndef _ASM_ARCH_PCH_H
#define _ASM_ARCH_PCH_H

#include <pci.h>

/* PCH types */
#define PCH_TYPE_CPT	0x1c /* CougarPoint */
#define PCH_TYPE_PPT	0x1e /* IvyBridge */

/* PCH stepping values for LPC device */
#define PCH_STEP_A0	0
#define PCH_STEP_A1	1
#define PCH_STEP_B0	2
#define PCH_STEP_B1	3
#define PCH_STEP_B2	4
#define PCH_STEP_B3	5
#define DEFAULT_GPIOBASE	0x0480
#define DEFAULT_PMBASE		0x0500

#define SMBUS_IO_BASE		0x0400

#define MAINBOARD_POWER_OFF	0
#define MAINBOARD_POWER_ON	1
#define MAINBOARD_POWER_KEEP	2

/* PCI Configuration Space (D30:F0): PCI2PCI */
#define PSTS	0x06
#define SMLT	0x1b
#define SECSTS	0x1e
#define INTR	0x3c
#define BCTRL	0x3e
#define   SBR	(1 << 6)
#define   SEE	(1 << 1)
#define   PERE	(1 << 0)

#define PCH_EHCI1_DEV		PCI_BDF(0, 0x1d, 0)
#define PCH_EHCI2_DEV		PCI_BDF(0, 0x1a, 0)
#define PCH_XHCI_DEV		PCI_BDF(0, 0x14, 0)
#define PCH_ME_DEV		PCI_BDF(0, 0x16, 0)
#define PCH_PCIE_DEV_SLOT	28

#define PCH_DEV			PCI_BDF(0, 0, 0)
#define PCH_VIDEO_DEV		PCI_BDF(0, 2, 0)

/* PCI Configuration Space (D31:F0): LPC */
#define PCH_LPC_DEV		PCI_BDF(0, 0x1f, 0)
#define SERIRQ_CNTL		0x64

#define GEN_PMCON_1		0xa0
#define GEN_PMCON_2		0xa2
#define GEN_PMCON_3		0xa4
#define ETR3			0xac
#define  ETR3_CWORWRE		(1 << 18)
#define  ETR3_CF9GR		(1 << 20)

/* GEN_PMCON_3 bits */
#define RTC_BATTERY_DEAD	(1 << 2)
#define RTC_POWER_FAILED	(1 << 1)
#define SLEEP_AFTER_POWER_FAIL	(1 << 0)

#define BIOS_CNTL		0xDC
#define GPIO_BASE		0x48 /* LPC GPIO Base Address Register */
#define GPIO_CNTL		0x4C /* LPC GPIO Control Register */
#define GPIO_ROUT		0xb8

#define PIRQA_ROUT		0x60
#define PIRQB_ROUT		0x61
#define PIRQC_ROUT		0x62
#define PIRQD_ROUT		0x63
#define PIRQE_ROUT		0x68
#define PIRQF_ROUT		0x69
#define PIRQG_ROUT		0x6A
#define PIRQH_ROUT		0x6B

#define GEN_PMCON_1		0xa0
#define GEN_PMCON_2		0xa2
#define GEN_PMCON_3		0xa4
#define ETR3			0xac
#define  ETR3_CWORWRE		(1 << 18)
#define  ETR3_CF9GR		(1 << 20)

#define PMBASE			0x40
#define ACPI_CNTL		0x44
#define BIOS_CNTL		0xDC
#define GPIO_BASE		0x48 /* LPC GPIO Base Address Register */
#define GPIO_CNTL		0x4C /* LPC GPIO Control Register */
#define GPIO_ROUT		0xb8

/* PCI Configuration Space (D31:F1): IDE */
#define PCH_IDE_DEV		PCI_BDF(0, 0x1f, 1)
#define PCH_SATA_DEV		PCI_BDF(0, 0x1f, 2)
#define PCH_SATA2_DEV		PCI_BDF(0, 0x1f, 5)

#define IDE_SDMA_CNT		0x48	/* Synchronous DMA control */
#define   IDE_SSDE1		(1 <<  3)
#define   IDE_SSDE0		(1 <<  2)
#define   IDE_PSDE1		(1 <<  1)
#define   IDE_PSDE0		(1 <<  0)

#define IDE_SDMA_TIM		0x4a

#define IDE_CONFIG		0x54	/* IDE I/O Configuration Register */
#define   SIG_MODE_SEC_NORMAL	(0 << 18)
#define   SIG_MODE_SEC_TRISTATE	(1 << 18)
#define   SIG_MODE_SEC_DRIVELOW	(2 << 18)
#define   SIG_MODE_PRI_NORMAL	(0 << 16)
#define   SIG_MODE_PRI_TRISTATE	(1 << 16)
#define   SIG_MODE_PRI_DRIVELOW	(2 << 16)
#define   FAST_SCB1		(1 << 15)
#define   FAST_SCB0		(1 << 14)
#define   FAST_PCB1		(1 << 13)
#define   FAST_PCB0		(1 << 12)
#define   SCB1			(1 <<  3)
#define   SCB0			(1 <<  2)
#define   PCB1			(1 <<  1)
#define   PCB0			(1 <<  0)

#define SATA_SIRI		0xa0 /* SATA Indexed Register Index */
#define SATA_SIRD		0xa4 /* SATA Indexed Register Data */
#define SATA_SP			0xd0 /* Scratchpad */

/* SATA IOBP Registers */
#define SATA_IOBP_SP0G3IR	0xea000151
#define SATA_IOBP_SP1G3IR	0xea000051

#define VCH		0x0000	/* 32bit */
#define VCAP1		0x0004	/* 32bit */
#define VCAP2		0x0008	/* 32bit */
#define PVC		0x000c	/* 16bit */
#define PVS		0x000e	/* 16bit */

#define V0CAP		0x0010	/* 32bit */
#define V0CTL		0x0014	/* 32bit */
#define V0STS		0x001a	/* 16bit */

#define V1CAP		0x001c	/* 32bit */
#define V1CTL		0x0020	/* 32bit */
#define V1STS		0x0026	/* 16bit */

#define RCTCL		0x0100	/* 32bit */
#define ESD		0x0104	/* 32bit */
#define ULD		0x0110	/* 32bit */
#define ULBA		0x0118	/* 64bit */

#define RP1D		0x0120	/* 32bit */
#define RP1BA		0x0128	/* 64bit */
#define RP2D		0x0130	/* 32bit */
#define RP2BA		0x0138	/* 64bit */
#define RP3D		0x0140	/* 32bit */
#define RP3BA		0x0148	/* 64bit */
#define RP4D		0x0150	/* 32bit */
#define RP4BA		0x0158	/* 64bit */
#define HDD		0x0160	/* 32bit */
#define HDBA		0x0168	/* 64bit */
#define RP5D		0x0170	/* 32bit */
#define RP5BA		0x0178	/* 64bit */
#define RP6D		0x0180	/* 32bit */
#define RP6BA		0x0188	/* 64bit */

#define RPC		0x0400	/* 32bit */
#define RPFN		0x0404	/* 32bit */

#define TRSR		0x1e00	/*  8bit */
#define TRCR		0x1e10	/* 64bit */
#define TWDR		0x1e18	/* 64bit */

#define IOTR0		0x1e80	/* 64bit */
#define IOTR1		0x1e88	/* 64bit */
#define IOTR2		0x1e90	/* 64bit */
#define IOTR3		0x1e98	/* 64bit */

#define TCTL		0x3000	/*  8bit */

#define NOINT		0
#define INTA		1
#define INTB		2
#define INTC		3
#define INTD		4

#define DIR_IDR		12	/* Interrupt D Pin Offset */
#define DIR_ICR		8	/* Interrupt C Pin Offset */
#define DIR_IBR		4	/* Interrupt B Pin Offset */
#define DIR_IAR		0	/* Interrupt A Pin Offset */

#define PIRQA		0
#define PIRQB		1
#define PIRQC		2
#define PIRQD		3
#define PIRQE		4
#define PIRQF		5
#define PIRQG		6
#define PIRQH		7

/* IO Buffer Programming */
#define IOBPIRI		0x2330
#define IOBPD		0x2334
#define IOBPS		0x2338
#define  IOBPS_RW_BX    ((1 << 9)|(1 << 10))
#define  IOBPS_WRITE_AX	((1 << 9)|(1 << 10))
#define  IOBPS_READ_AX	((1 << 8)|(1 << 9)|(1 << 10))

#define D31IP		0x3100	/* 32bit */
#define D31IP_TTIP	24	/* Thermal Throttle Pin */
#define D31IP_SIP2	20	/* SATA Pin 2 */
#define D31IP_SMIP	12	/* SMBUS Pin */
#define D31IP_SIP	8	/* SATA Pin */
#define D30IP		0x3104	/* 32bit */
#define D30IP_PIP	0	/* PCI Bridge Pin */
#define D29IP		0x3108	/* 32bit */
#define D29IP_E1P	0	/* EHCI #1 Pin */
#define D28IP		0x310c	/* 32bit */
#define D28IP_P8IP	28	/* PCI Express Port 8 */
#define D28IP_P7IP	24	/* PCI Express Port 7 */
#define D28IP_P6IP	20	/* PCI Express Port 6 */
#define D28IP_P5IP	16	/* PCI Express Port 5 */
#define D28IP_P4IP	12	/* PCI Express Port 4 */
#define D28IP_P3IP	8	/* PCI Express Port 3 */
#define D28IP_P2IP	4	/* PCI Express Port 2 */
#define D28IP_P1IP	0	/* PCI Express Port 1 */
#define D27IP		0x3110	/* 32bit */
#define D27IP_ZIP	0	/* HD Audio Pin */
#define D26IP		0x3114	/* 32bit */
#define D26IP_E2P	0	/* EHCI #2 Pin */
#define D25IP		0x3118	/* 32bit */
#define D25IP_LIP	0	/* GbE LAN Pin */
#define D22IP		0x3124	/* 32bit */
#define D22IP_KTIP	12	/* KT Pin */
#define D22IP_IDERIP	8	/* IDE-R Pin */
#define D22IP_MEI2IP	4	/* MEI #2 Pin */
#define D22IP_MEI1IP	0	/* MEI #1 Pin */
#define D20IP		0x3128  /* 32bit */
#define D20IP_XHCIIP	0
#define D31IR		0x3140	/* 16bit */
#define D30IR		0x3142	/* 16bit */
#define D29IR		0x3144	/* 16bit */
#define D28IR		0x3146	/* 16bit */
#define D27IR		0x3148	/* 16bit */
#define D26IR		0x314c	/* 16bit */
#define D25IR		0x3150	/* 16bit */
#define D22IR		0x315c	/* 16bit */
#define D20IR		0x3160	/* 16bit */
#define OIC		0x31fe	/* 16bit */

#define SPI_FREQ_SWSEQ	0x3893
#define SPI_DESC_COMP0	0x38b0
#define SPI_FREQ_WR_ERA	0x38b4

#define DIR_ROUTE(a, b, c, d) \
		(((d) << DIR_IDR) | ((c) << DIR_ICR) | \
			((b) << DIR_IBR) | ((a) << DIR_IAR))

#define HPTC		0x3404	/* 32bit */
#define BUC		0x3414	/* 32bit */
#define PCH_DISABLE_GBE		(1 << 5)
#define FD		0x3418	/* 32bit */
#define DISPBDF		0x3424  /* 16bit */
#define FD2		0x3428	/* 32bit */
#define CG		0x341c	/* 32bit */

/* Function Disable 1 RCBA 0x3418 */
#define PCH_DISABLE_ALWAYS	((1 << 0)|(1 << 26))
#define PCH_DISABLE_P2P		(1 << 1)
#define PCH_DISABLE_SATA1	(1 << 2)
#define PCH_DISABLE_SMBUS	(1 << 3)
#define PCH_DISABLE_HD_AUDIO	(1 << 4)
#define PCH_DISABLE_EHCI2	(1 << 13)
#define PCH_DISABLE_LPC		(1 << 14)
#define PCH_DISABLE_EHCI1	(1 << 15)
#define PCH_DISABLE_PCIE(x)	(1 << (16 + x))
#define PCH_DISABLE_THERMAL	(1 << 24)
#define PCH_DISABLE_SATA2	(1 << 25)
#define PCH_DISABLE_XHCI	(1 << 27)

/* Function Disable 2 RCBA 0x3428 */
#define PCH_DISABLE_KT		(1 << 4)
#define PCH_DISABLE_IDER	(1 << 3)
#define PCH_DISABLE_MEI2	(1 << 2)
#define PCH_DISABLE_MEI1	(1 << 1)
#define PCH_ENABLE_DBDF		(1 << 0)

/* ICH7 GPIOBASE */
#define GPIO_USE_SEL	0x00
#define GP_IO_SEL	0x04
#define GP_LVL		0x0c
#define GPO_BLINK	0x18
#define GPI_INV		0x2c
#define GPIO_USE_SEL2	0x30
#define GP_IO_SEL2	0x34
#define GP_LVL2		0x38
#define GPIO_USE_SEL3	0x40
#define GP_IO_SEL3	0x44
#define GP_LVL3		0x48
#define GP_RST_SEL1	0x60
#define GP_RST_SEL2	0x64
#define GP_RST_SEL3	0x68

/* ICH7 PMBASE */
#define PM1_STS		0x00
#define   WAK_STS	(1 << 15)
#define   PCIEXPWAK_STS	(1 << 14)
#define   PRBTNOR_STS	(1 << 11)
#define   RTC_STS	(1 << 10)
#define   PWRBTN_STS	(1 << 8)
#define   GBL_STS	(1 << 5)
#define   BM_STS	(1 << 4)
#define   TMROF_STS	(1 << 0)
#define PM1_EN		0x02
#define   PCIEXPWAK_DIS	(1 << 14)
#define   RTC_EN	(1 << 10)
#define   PWRBTN_EN	(1 << 8)
#define   GBL_EN	(1 << 5)
#define   TMROF_EN	(1 << 0)
#define PM1_CNT		0x04
#define   SLP_EN	(1 << 13)
#define   SLP_TYP	(7 << 10)
#define    SLP_TYP_S0	0
#define    SLP_TYP_S1	1
#define    SLP_TYP_S3	5
#define    SLP_TYP_S4	6
#define    SLP_TYP_S5	7
#define   GBL_RLS	(1 << 2)
#define   BM_RLD	(1 << 1)
#define   SCI_EN	(1 << 0)
#define PM1_TMR		0x08
#define PROC_CNT	0x10
#define LV2		0x14
#define LV3		0x15
#define LV4		0x16
#define PM2_CNT		0x50 /* mobile only */
#define GPE0_STS	0x20
#define   PME_B0_STS	(1 << 13)
#define   PME_STS	(1 << 11)
#define   BATLOW_STS	(1 << 10)
#define   PCI_EXP_STS	(1 << 9)
#define   RI_STS	(1 << 8)
#define   SMB_WAK_STS	(1 << 7)
#define   TCOSCI_STS	(1 << 6)
#define   SWGPE_STS	(1 << 2)
#define   HOT_PLUG_STS	(1 << 1)
#define GPE0_EN		0x28
#define   PME_B0_EN	(1 << 13)
#define   PME_EN	(1 << 11)
#define   TCOSCI_EN	(1 << 6)
#define SMI_EN		0x30
#define   INTEL_USB2_EN	 (1 << 18) /* Intel-Specific USB2 SMI logic */
#define   LEGACY_USB2_EN (1 << 17) /* Legacy USB2 SMI logic */
#define   PERIODIC_EN	 (1 << 14) /* SMI on PERIODIC_STS in SMI_STS */
#define   TCO_EN	 (1 << 13) /* Enable TCO Logic (BIOSWE et al) */
#define   MCSMI_EN	 (1 << 11) /* Trap microcontroller range access */
#define   BIOS_RLS	 (1 <<  7) /* asserts SCI on bit set */
#define   SWSMI_TMR_EN	 (1 <<  6) /* start software smi timer on bit set */
#define   APMC_EN	 (1 <<  5) /* Writes to APM_CNT cause SMI# */
#define   SLP_SMI_EN	 (1 <<  4) /* Write SLP_EN in PM1_CNT asserts SMI# */
#define   LEGACY_USB_EN  (1 <<  3) /* Legacy USB circuit SMI logic */
#define   BIOS_EN	 (1 <<  2) /* Assert SMI# on setting GBL_RLS bit */
#define   EOS		 (1 <<  1) /* End of SMI (deassert SMI#) */
#define   GBL_SMI_EN	 (1 <<  0) /* SMI# generation at all? */
#define SMI_STS		0x34
#define ALT_GP_SMI_EN	0x38
#define ALT_GP_SMI_STS	0x3a
#define GPE_CNTL	0x42
#define DEVACT_STS	0x44
#define SS_CNT		0x50
#define C3_RES		0x54
#define TCO1_STS	0x64
#define   DMISCI_STS	(1 << 9)
#define TCO2_STS	0x66

/**
 * pch_silicon_revision() - Read silicon device ID from the PCH
 *
 * @dev:	PCH device
 * Return: silicon device ID
 */
int pch_silicon_type(struct udevice *dev);

/**
 * pch_pch_iobp_update() - Update a pch register
 *
 * @dev:	PCH device
 * @address:	Address to update
 * @andvalue:	Value to AND with existing value
 * @orvalue:	Value to OR with existing value
 */
void pch_iobp_update(struct udevice *dev, u32 address, u32 andvalue,
			     u32 orvalue);

#endif
