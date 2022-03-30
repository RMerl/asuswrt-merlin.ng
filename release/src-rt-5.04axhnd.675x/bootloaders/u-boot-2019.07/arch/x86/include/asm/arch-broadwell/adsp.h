/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Support for Intel Application Digital Signal Processor
 *
 * Copyright 2019 Google LLC
 *
 * Modified from coreboot file of the same name
 */

#ifndef __ASM_ARCH_BROADWELL_ADSP_H
#define __ASM_ARCH_BROADWELL_ADSP_H

#define ADSP_PCI_IRQ			23
#define ADSP_ACPI_IRQ			3
#define  ADSP_ACPI_IRQEN		BIT(3)

#define ADSP_SHIM_BASE_LPT		0xe7000
#define ADSP_SHIM_BASE_WPT		0xfb000
#define  ADSP_SHIM_LTRC			0xe0
#define   ADSP_SHIM_LTRC_VALUE		0x3003
#define  ADSP_SHIM_IMC			0x28
#define  ADSP_SHIM_IPCD			0x40

#define ADSP_PCI_VDRTCTL0		0xa0
#define  ADSP_VDRTCTL0_D3PGD_LPT	BIT(1)
#define  ADSP_VDRTCTL0_D3PGD_WPT	BIT(0)
#define  ADSP_VDRTCTL0_D3SRAMPGD_LPT	BIT(2)
#define  ADSP_VDRTCTL0_D3SRAMPGD_WPT	BIT(1)
#define ADSP_PCI_VDRTCTL1		0xa4
#define ADSP_PCI_VDRTCTL2		0xa8
#define  ADSP_VDRTCTL2_VALUE		0x00000fff

#define ADSP_IOBP_VDLDAT1		0xd7000624
#define  ADSP_VDLDAT1_VALUE		0x00040100
#define ADSP_IOBP_VDLDAT2		0xd7000628
#define  ADSP_IOBP_ACPI_IRQ3		0xd9d8
#define  ADSP_IOBP_ACPI_IRQ3I		0xd8d9
#define  ADSP_IOBP_ACPI_IRQ4		0xdbda
#define ADSP_IOBP_PMCTL			0xd70001e0
#define  ADSP_PMCTL_VALUE		0x3f
#define ADSP_IOBP_PCICFGCTL		0xd7000500
#define  ADSP_PCICFGCTL_PCICD		BIT(0)
#define  ADSP_PCICFGCTL_ACPIIE		BIT(1)
#define  ADSP_PCICFGCTL_SPCBAD		BIT(7)

#endif /* __ASM_ARCH_BROADWELL_ADSP_H */
