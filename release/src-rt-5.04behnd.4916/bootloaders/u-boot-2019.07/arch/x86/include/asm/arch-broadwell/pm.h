/* SPDX-License-Identifier: GPL-2.0 */
/*
 * From coreboot src/soc/intel/broadwell/include/soc/pm.h
 *
 * Copyright (C) 2016 Google, Inc.
 */

#ifndef __ASM_ARCH_PM_H
#define __ASM_ARCH_PM_H

#define PM1_STS			0x00
#define  WAK_STS		(1 << 15)
#define  PCIEXPWAK_STS		(1 << 14)
#define  PRBTNOR_STS		(1 << 11)
#define  RTC_STS		(1 << 10)
#define  PWRBTN_STS		(1 << 8)
#define  GBL_STS		(1 << 5)
#define  BM_STS			(1 << 4)
#define  TMROF_STS		(1 << 0)
#define PM1_EN			0x02
#define  PCIEXPWAK_DIS		(1 << 14)
#define  RTC_EN			(1 << 10)
#define  PWRBTN_EN		(1 << 8)
#define  GBL_EN			(1 << 5)
#define  TMROF_EN		(1 << 0)
#define PM1_CNT			0x04
#define  SLP_EN			(1 << 13)
#define  SLP_TYP		(7 << 10)
#define   SLP_TYP_SHIFT         10
#define   SLP_TYP_S0		0
#define   SLP_TYP_S1		1
#define   SLP_TYP_S3		5
#define   SLP_TYP_S4		6
#define   SLP_TYP_S5		7
#define  GBL_RLS		(1 << 2)
#define  BM_RLD			(1 << 1)
#define  SCI_EN			(1 << 0)
#define PM1_TMR			0x08
#define SMI_EN			0x30
#define  XHCI_SMI_EN		(1 << 31)
#define  ME_SMI_EN		(1 << 30)
#define  GPIO_UNLOCK_SMI_EN	(1 << 27)
#define  INTEL_USB2_EN		(1 << 18)
#define  LEGACY_USB2_EN		(1 << 17)
#define  PERIODIC_EN		(1 << 14)
#define  TCO_EN			(1 << 13)
#define  MCSMI_EN		(1 << 11)
#define  BIOS_RLS		(1 <<  7)
#define  SWSMI_TMR_EN		(1 <<  6)
#define  APMC_EN		(1 <<  5)
#define  SLP_SMI_EN		(1 <<  4)
#define  LEGACY_USB_EN		(1 <<  3)
#define  BIOS_EN		(1 <<  2)
#define  EOS			(1 <<  1)
#define  GBL_SMI_EN		(1 <<  0)
#define SMI_STS			0x34
#define UPWRC			0x3c
#define  UPWRC_WS		(1 << 8)
#define  UPWRC_WE		(1 << 1)
#define  UPWRC_SMI		(1 << 0)
#define GPE_CNTL		0x42
#define  SWGPE_CTRL		(1 << 1)
#define DEVACT_STS		0x44
#define PM2_CNT			0x50
#define TCO1_CNT		0x60
#define  TCO_TMR_HLT		(1 << 11)
#define TCO1_STS		0x64
#define  DMISCI_STS		(1 << 9)
#define TCO2_STS		0x66
#define  TCO2_STS_SECOND_TO	(1 << 1)

#define GPE0_REG_MAX		4
#define GPE0_REG_SIZE		32
#define GPE0_STS(x)		(0x80 + (x * 4))
#define  GPE_31_0		0	/* 0x80/0x90 = GPE[31:0] */
#define  GPE_63_32		1	/* 0x84/0x94 = GPE[63:32] */
#define  GPE_94_64		2	/* 0x88/0x98 = GPE[94:64] */
#define  GPE_STD		3	/* 0x8c/0x9c = Standard GPE */
#define   WADT_STS		(1 << 18)
#define   GP27_STS		(1 << 16)
#define   PME_B0_STS		(1 << 13)
#define   ME_SCI_STS		(1 << 12)
#define   PME_STS		(1 << 11)
#define   BATLOW_STS		(1 << 10)
#define   PCI_EXP_STS		(1 << 9)
#define   SMB_WAK_STS		(1 << 7)
#define   TCOSCI_STS		(1 << 6)
#define   SWGPE_STS		(1 << 2)
#define   HOT_PLUG_STS		(1 << 1)
#define GPE0_EN(x)		(0x90 + (x * 4))
#define   WADT_en		(1 << 18)
#define   GP27_EN		(1 << 16)
#define   PME_B0_EN		(1 << 13)
#define   ME_SCI_EN		(1 << 12)
#define   PME_EN		(1 << 11)
#define   BATLOW_EN		(1 << 10)
#define   PCI_EXP_EN		(1 << 9)
#define   TCOSCI_EN		(1 << 6)
#define   SWGPE_EN		(1 << 2)
#define   HOT_PLUG_EN		(1 << 1)

#define MAINBOARD_POWER_OFF	0
#define MAINBOARD_POWER_ON	1
#define MAINBOARD_POWER_KEEP	2

#define SLEEP_STATE_S0		0
#define SLEEP_STATE_S3		3
#define SLEEP_STATE_S5		5

struct chipset_power_state {
	uint16_t pm1_sts;
	uint16_t pm1_en;
	uint32_t pm1_cnt;
	uint16_t tco1_sts;
	uint16_t tco2_sts;
	uint32_t gpe0_sts[4];
	uint32_t gpe0_en[4];
	uint16_t gen_pmcon1;
	uint16_t gen_pmcon2;
	uint16_t gen_pmcon3;
	int prev_sleep_state;
	uint16_t hsio_version;
	uint16_t hsio_checksum;
};

void power_state_get(struct udevice *pch_dev, struct chipset_power_state *ps);

#endif
