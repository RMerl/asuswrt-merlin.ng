// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2016 Freescale Semiconductor, Inc.
 * Author: Hongbo Zhang <hongbo.zhang@nxp.com>
 * This file implements LS102X platform PSCI SYSTEM-SUSPEND function
 */

#include <config.h>
#include <asm/io.h>
#include <asm/psci.h>
#include <asm/arch/immap_ls102xa.h>
#include <fsl_immap.h>
#include "fsl_epu.h"

#define __secure __attribute__((section("._secure.text")))

#define CCSR_GICD_CTLR			0x1000
#define CCSR_GICC_CTLR			0x2000
#define DCSR_RCPM_CG1CR0		0x31c
#define DCSR_RCPM_CSTTACR0		0xb00
#define DCFG_CRSTSR_WDRFR		0x8
#define DDR_RESV_LEN			128

#ifdef CONFIG_LS1_DEEP_SLEEP
/*
 * DDR controller initialization training breaks the first 128 bytes of DDR,
 * save them so that the bootloader can restore them while resuming.
 */
static void __secure ls1_save_ddr_head(void)
{
	const char *src = (const char *)CONFIG_SYS_SDRAM_BASE;
	char *dest = (char *)(OCRAM_BASE_S_ADDR + OCRAM_S_SIZE - DDR_RESV_LEN);
	struct ccsr_scfg __iomem *scfg = (void *)CONFIG_SYS_FSL_SCFG_ADDR;
	int i;

	out_le32(&scfg->sparecr[2], dest);

	for (i = 0; i < DDR_RESV_LEN; i++)
		*dest++ = *src++;
}

static void __secure ls1_fsm_setup(void)
{
	void *dcsr_epu_base = (void *)(CONFIG_SYS_DCSRBAR + EPU_BLOCK_OFFSET);
	void *dcsr_rcpm_base = (void *)SYS_FSL_DCSR_RCPM_ADDR;

	out_be32(dcsr_rcpm_base + DCSR_RCPM_CSTTACR0, 0x00001001);
	out_be32(dcsr_rcpm_base + DCSR_RCPM_CG1CR0, 0x00000001);

	fsl_epu_setup((void *)dcsr_epu_base);

	/* Pull MCKE signal low before enabling deep sleep signal in FPGA */
	out_be32(dcsr_epu_base + EPECR0, 0x5);
	out_be32(dcsr_epu_base + EPSMCR15, 0x76300000);
}

static void __secure ls1_deepsleep_irq_cfg(void)
{
	struct ccsr_scfg __iomem *scfg = (void *)CONFIG_SYS_FSL_SCFG_ADDR;
	struct ccsr_rcpm __iomem *rcpm = (void *)CONFIG_SYS_FSL_RCPM_ADDR;
	u32 ippdexpcr0, ippdexpcr1, pmcintecr = 0;

	/* Mask interrupts from GIC */
	out_be32(&rcpm->nfiqoutr, 0x0ffffffff);
	out_be32(&rcpm->nirqoutr, 0x0ffffffff);
	/* Mask deep sleep wake-up interrupts while entering deep sleep */
	out_be32(&rcpm->dsimskr, 0x0ffffffff);

	ippdexpcr0 = in_be32(&rcpm->ippdexpcr0);
	/*
	 * Workaround: There is bug of register ippdexpcr1, when read it always
	 * returns zero, so its value is saved to a scrachpad register to be
	 * read, that is why we don't read it from register ippdexpcr1 itself.
	 */
	ippdexpcr1 = in_le32(&scfg->sparecr[7]);
	out_be32(&rcpm->ippdexpcr1, ippdexpcr1);

	if (ippdexpcr0 & RCPM_IPPDEXPCR0_ETSEC)
		pmcintecr |= SCFG_PMCINTECR_ETSECRXG0 |
			     SCFG_PMCINTECR_ETSECRXG1 |
			     SCFG_PMCINTECR_ETSECERRG0 |
			     SCFG_PMCINTECR_ETSECERRG1;

	if (ippdexpcr0 & RCPM_IPPDEXPCR0_GPIO)
		pmcintecr |= SCFG_PMCINTECR_GPIO;

	if (ippdexpcr1 & RCPM_IPPDEXPCR1_LPUART)
		pmcintecr |= SCFG_PMCINTECR_LPUART;

	if (ippdexpcr1 & RCPM_IPPDEXPCR1_FLEXTIMER)
		pmcintecr |= SCFG_PMCINTECR_FTM;

	/* Always set external IRQ pins as wakeup source */
	pmcintecr |= SCFG_PMCINTECR_IRQ0 | SCFG_PMCINTECR_IRQ1;

	out_be32(&scfg->pmcintlecr, 0);
	/* Clear PMC interrupt status */
	out_be32(&scfg->pmcintsr, 0xffffffff);
	/* Enable wakeup interrupt during deep sleep */
	out_be32(&scfg->pmcintecr, pmcintecr);
}

static void __secure ls1_delay(unsigned int loop)
{
	while (loop--) {
		int i = 1000;
		while (i--)
			;
	}
}

static void __secure ls1_start_fsm(void)
{
	void *dcsr_epu_base = (void *)(CONFIG_SYS_DCSRBAR + EPU_BLOCK_OFFSET);
	void *ccsr_gic_base = (void *)SYS_FSL_GIC_ADDR;
	struct ccsr_scfg __iomem *scfg = (void *)CONFIG_SYS_FSL_SCFG_ADDR;
	struct ccsr_ddr __iomem *ddr = (void *)CONFIG_SYS_FSL_DDR_ADDR;

	/* Set HRSTCR */
	setbits_be32(&scfg->hrstcr, 0x80000000);

	/* Place DDR controller in self refresh mode */
	setbits_be32(&ddr->sdram_cfg_2, 0x80000000);

	ls1_delay(2000);

	/* Set EVT4_B to lock the signal MCKE down */
	out_be32(dcsr_epu_base + EPECR0, 0x0);

	ls1_delay(2000);

	out_be32(ccsr_gic_base + CCSR_GICD_CTLR, 0x0);
	out_be32(ccsr_gic_base + CCSR_GICC_CTLR, 0x0);

	/* Enable all EPU Counters */
	setbits_be32(dcsr_epu_base + EPGCR, 0x80000000);

	/* Enable SCU15 */
	setbits_be32(dcsr_epu_base + EPECR15, 0x90000004);

	/* Enter WFI mode, and EPU FSM will start */
	__asm__ __volatile__ ("wfi" : : : "memory");

	/* NEVER ENTER HERE */
	while (1)
		;
}

static void __secure ls1_deep_sleep(u32 entry_point)
{
	struct ccsr_scfg __iomem *scfg = (void *)CONFIG_SYS_FSL_SCFG_ADDR;
	struct ccsr_gur __iomem *gur = (void *)CONFIG_SYS_FSL_GUTS_ADDR;
	struct ccsr_rcpm __iomem *rcpm = (void *)CONFIG_SYS_FSL_RCPM_ADDR;
#ifdef QIXIS_BASE
	u32 tmp;
	void *qixis_base = (void *)QIXIS_BASE;
#endif

	/* Enable cluster to enter the PCL10 state */
	out_be32(&scfg->clusterpmcr, SCFG_CLUSTERPMCR_WFIL2EN);

	/* Save the first 128 bytes of DDR data */
	ls1_save_ddr_head();

	/* Save the kernel resume entry */
	out_le32(&scfg->sparecr[3], entry_point);

	/* Request to put cluster 0 in PCL10 state */
	setbits_be32(&rcpm->clpcl10setr, RCPM_CLPCL10SETR_C0);

	/* Setup the registers of the EPU FSM for deep sleep */
	ls1_fsm_setup();

#ifdef QIXIS_BASE
	/* Connect the EVENT button to IRQ in FPGA */
	tmp = in_8(qixis_base + QIXIS_CTL_SYS);
	tmp &= ~QIXIS_CTL_SYS_EVTSW_MASK;
	tmp |= QIXIS_CTL_SYS_EVTSW_IRQ;
	out_8(qixis_base + QIXIS_CTL_SYS, tmp);

	/* Enable deep sleep signals in FPGA */
	tmp = in_8(qixis_base + QIXIS_PWR_CTL2);
	tmp |= QIXIS_PWR_CTL2_PCTL;
	out_8(qixis_base + QIXIS_PWR_CTL2, tmp);

	/* Pull down PCIe RST# */
	tmp = in_8(qixis_base + QIXIS_RST_FORCE_3);
	tmp |= QIXIS_RST_FORCE_3_PCIESLOT1;
	out_8(qixis_base + QIXIS_RST_FORCE_3, tmp);
#endif

	/* Enable Warm Device Reset */
	setbits_be32(&scfg->dpslpcr, SCFG_DPSLPCR_WDRR_EN);
	setbits_be32(&gur->crstsr, DCFG_CRSTSR_WDRFR);

	/* Disable QE */
	setbits_be32(&gur->devdisr, CCSR_DEVDISR1_QE);

	ls1_deepsleep_irq_cfg();

	psci_v7_flush_dcache_all();

	ls1_start_fsm();
}

#else
static void __secure ls1_sleep(void)
{
	struct ccsr_scfg __iomem *scfg = (void *)CONFIG_SYS_FSL_SCFG_ADDR;
	struct ccsr_rcpm __iomem *rcpm = (void *)CONFIG_SYS_FSL_RCPM_ADDR;

#ifdef QIXIS_BASE
	u32 tmp;
	void *qixis_base = (void *)QIXIS_BASE;

	/* Connect the EVENT button to IRQ in FPGA */
	tmp = in_8(qixis_base + QIXIS_CTL_SYS);
	tmp &= ~QIXIS_CTL_SYS_EVTSW_MASK;
	tmp |= QIXIS_CTL_SYS_EVTSW_IRQ;
	out_8(qixis_base + QIXIS_CTL_SYS, tmp);
#endif

	/* Enable cluster to enter the PCL10 state */
	out_be32(&scfg->clusterpmcr, SCFG_CLUSTERPMCR_WFIL2EN);

	setbits_be32(&rcpm->powmgtcsr, RCPM_POWMGTCSR_LPM20_REQ);

	__asm__ __volatile__ ("wfi" : : : "memory");
}
#endif

void __secure ls1_system_suspend(u32 fn, u32 entry_point, u32 context_id)
{
#ifdef CONFIG_LS1_DEEP_SLEEP
	ls1_deep_sleep(entry_point);
#else
	ls1_sleep();
#endif
}
