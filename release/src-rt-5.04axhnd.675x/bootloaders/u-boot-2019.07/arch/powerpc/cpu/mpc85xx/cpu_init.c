// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2007-2011 Freescale Semiconductor, Inc.
 *
 * (C) Copyright 2003 Motorola Inc.
 * Modified by Xianghua Xiao, X.Xiao@motorola.com
 *
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#include <common.h>
#include <watchdog.h>
#include <asm/processor.h>
#include <ioports.h>
#include <sata.h>
#include <fm_eth.h>
#include <asm/io.h>
#include <asm/cache.h>
#include <asm/mmu.h>
#include <fsl_errata.h>
#include <asm/fsl_law.h>
#include <asm/fsl_serdes.h>
#include <asm/fsl_srio.h>
#ifdef CONFIG_FSL_CORENET
#include <asm/fsl_portals.h>
#include <asm/fsl_liodn.h>
#include <fsl_qbman.h>
#endif
#include <fsl_usb.h>
#include <hwconfig.h>
#include <linux/compiler.h>
#include "mp.h"
#ifdef CONFIG_CHAIN_OF_TRUST
#include <fsl_validate.h>
#endif
#ifdef CONFIG_FSL_CAAM
#include <fsl_sec.h>
#endif
#if defined(CONFIG_SECURE_BOOT) && defined(CONFIG_FSL_CORENET)
#include <asm/fsl_pamu.h>
#include <fsl_secboot_err.h>
#endif
#ifdef CONFIG_SYS_QE_FMAN_FW_IN_NAND
#include <nand.h>
#include <errno.h>
#endif
#ifndef CONFIG_ARCH_QEMU_E500
#include <fsl_ddr.h>
#endif
#include "../../../../drivers/ata/fsl_sata.h"
#ifdef CONFIG_U_QE
#include <fsl_qe.h>
#endif

#ifdef CONFIG_SYS_FSL_SINGLE_SOURCE_CLK
/*
 * For deriving usb clock from 100MHz sysclk, reference divisor is set
 * to a value of 5, which gives an intermediate value 20(100/5). The
 * multiplication factor integer is set to 24, which when multiplied to
 * above intermediate value provides clock for usb ip.
 */
void usb_single_source_clk_configure(struct ccsr_usb_phy *usb_phy)
{
	sys_info_t sysinfo;

	get_sys_info(&sysinfo);
	if (sysinfo.diff_sysclk == 1) {
		clrbits_be32(&usb_phy->pllprg[1],
			     CONFIG_SYS_FSL_USB_PLLPRG2_MFI);
		setbits_be32(&usb_phy->pllprg[1],
			     CONFIG_SYS_FSL_USB_PLLPRG2_REF_DIV_INTERNAL_CLK |
			     CONFIG_SYS_FSL_USB_PLLPRG2_MFI_INTERNAL_CLK |
			     CONFIG_SYS_FSL_USB_INTERNAL_SOC_CLK_EN);
		}
}
#endif

#ifdef CONFIG_SYS_FSL_ERRATUM_A006261
void fsl_erratum_a006261_workaround(struct ccsr_usb_phy __iomem *usb_phy)
{
#ifdef CONFIG_SYS_FSL_USB_DUAL_PHY_ENABLE
	u32 xcvrprg = in_be32(&usb_phy->port1.xcvrprg);

	/* Increase Disconnect Threshold by 50mV */
	xcvrprg &= ~CONFIG_SYS_FSL_USB_XCVRPRG_HS_DCNT_PROG_MASK |
						INC_DCNT_THRESHOLD_50MV;
	/* Enable programming of USB High speed Disconnect threshold */
	xcvrprg |= CONFIG_SYS_FSL_USB_XCVRPRG_HS_DCNT_PROG_EN;
	out_be32(&usb_phy->port1.xcvrprg, xcvrprg);

	xcvrprg = in_be32(&usb_phy->port2.xcvrprg);
	/* Increase Disconnect Threshold by 50mV */
	xcvrprg &= ~CONFIG_SYS_FSL_USB_XCVRPRG_HS_DCNT_PROG_MASK |
						INC_DCNT_THRESHOLD_50MV;
	/* Enable programming of USB High speed Disconnect threshold */
	xcvrprg |= CONFIG_SYS_FSL_USB_XCVRPRG_HS_DCNT_PROG_EN;
	out_be32(&usb_phy->port2.xcvrprg, xcvrprg);
#else

	u32 temp = 0;
	u32 status = in_be32(&usb_phy->status1);

	u32 squelch_prog_rd_0_2 =
		(status >> CONFIG_SYS_FSL_USB_SQUELCH_PROG_RD_0)
			& CONFIG_SYS_FSL_USB_SQUELCH_PROG_MASK;

	u32 squelch_prog_rd_3_5 =
		(status >> CONFIG_SYS_FSL_USB_SQUELCH_PROG_RD_3)
			& CONFIG_SYS_FSL_USB_SQUELCH_PROG_MASK;

	setbits_be32(&usb_phy->config1,
		     CONFIG_SYS_FSL_USB_HS_DISCNCT_INC);
	setbits_be32(&usb_phy->config2,
		     CONFIG_SYS_FSL_USB_RX_AUTO_CAL_RD_WR_SEL);

	temp = squelch_prog_rd_0_2 << CONFIG_SYS_FSL_USB_SQUELCH_PROG_WR_3;
	out_be32(&usb_phy->config2, in_be32(&usb_phy->config2) | temp);

	temp = squelch_prog_rd_3_5 << CONFIG_SYS_FSL_USB_SQUELCH_PROG_WR_0;
	out_be32(&usb_phy->config2, in_be32(&usb_phy->config2) | temp);
#endif
}
#endif


#if defined(CONFIG_QE) && !defined(CONFIG_U_QE)
extern qe_iop_conf_t qe_iop_conf_tab[];
extern void qe_config_iopin(u8 port, u8 pin, int dir,
				int open_drain, int assign);
extern void qe_init(uint qe_base);
extern void qe_reset(void);

static void config_qe_ioports(void)
{
	u8      port, pin;
	int     dir, open_drain, assign;
	int     i;

	for (i = 0; qe_iop_conf_tab[i].assign != QE_IOP_TAB_END; i++) {
		port		= qe_iop_conf_tab[i].port;
		pin		= qe_iop_conf_tab[i].pin;
		dir		= qe_iop_conf_tab[i].dir;
		open_drain	= qe_iop_conf_tab[i].open_drain;
		assign		= qe_iop_conf_tab[i].assign;
		qe_config_iopin(port, pin, dir, open_drain, assign);
	}
}
#endif

#ifdef CONFIG_CPM2
void config_8560_ioports (volatile ccsr_cpm_t * cpm)
{
	int portnum;

	for (portnum = 0; portnum < 4; portnum++) {
		uint pmsk = 0,
		     ppar = 0,
		     psor = 0,
		     pdir = 0,
		     podr = 0,
		     pdat = 0;
		iop_conf_t *iopc = (iop_conf_t *) & iop_conf_tab[portnum][0];
		iop_conf_t *eiopc = iopc + 32;
		uint msk = 1;

		/*
		 * NOTE:
		 * index 0 refers to pin 31,
		 * index 31 refers to pin 0
		 */
		while (iopc < eiopc) {
			if (iopc->conf) {
				pmsk |= msk;
				if (iopc->ppar)
					ppar |= msk;
				if (iopc->psor)
					psor |= msk;
				if (iopc->pdir)
					pdir |= msk;
				if (iopc->podr)
					podr |= msk;
				if (iopc->pdat)
					pdat |= msk;
			}

			msk <<= 1;
			iopc++;
		}

		if (pmsk != 0) {
			volatile ioport_t *iop = ioport_addr (cpm, portnum);
			uint tpmsk = ~pmsk;

			/*
			 * the (somewhat confused) paragraph at the
			 * bottom of page 35-5 warns that there might
			 * be "unknown behaviour" when programming
			 * PSORx and PDIRx, if PPARx = 1, so I
			 * decided this meant I had to disable the
			 * dedicated function first, and enable it
			 * last.
			 */
			iop->ppar &= tpmsk;
			iop->psor = (iop->psor & tpmsk) | psor;
			iop->podr = (iop->podr & tpmsk) | podr;
			iop->pdat = (iop->pdat & tpmsk) | pdat;
			iop->pdir = (iop->pdir & tpmsk) | pdir;
			iop->ppar |= ppar;
		}
	}
}
#endif

#ifdef CONFIG_SYS_FSL_CPC
#if defined(CONFIG_RAMBOOT_PBL) || defined(CONFIG_SYS_CPC_REINIT_F)
void disable_cpc_sram(void)
{
	int i;

	cpc_corenet_t *cpc = (cpc_corenet_t *)CONFIG_SYS_FSL_CPC_ADDR;

	for (i = 0; i < CONFIG_SYS_NUM_CPC; i++, cpc++) {
		if (in_be32(&cpc->cpcsrcr0) & CPC_SRCR0_SRAMEN) {
			/* find and disable LAW of SRAM */
			struct law_entry law = find_law(CONFIG_SYS_INIT_L3_ADDR);

			if (law.index == -1) {
				printf("\nFatal error happened\n");
				return;
			}
			disable_law(law.index);

			clrbits_be32(&cpc->cpchdbcr0, CPC_HDBCR0_CDQ_SPEC_DIS);
			out_be32(&cpc->cpccsr0, 0);
			out_be32(&cpc->cpcsrcr0, 0);
		}
	}
}
#endif

#if defined(T1040_TDM_QUIRK_CCSR_BASE)
#ifdef CONFIG_POST
#error POST memory test cannot be enabled with TDM
#endif
static void enable_tdm_law(void)
{
	int ret;
	char buffer[HWCONFIG_BUFFER_SIZE] = {0};
	int tdm_hwconfig_enabled = 0;

	/*
	 * Extract hwconfig from environment since environment
	 * is not setup properly yet. Search for tdm entry in
	 * hwconfig.
	 */
	ret = env_get_f("hwconfig", buffer, sizeof(buffer));
	if (ret > 0) {
		tdm_hwconfig_enabled = hwconfig_f("tdm", buffer);
		/* If tdm is defined in hwconfig, set law for tdm workaround */
		if (tdm_hwconfig_enabled)
			set_next_law(T1040_TDM_QUIRK_CCSR_BASE, LAW_SIZE_16M,
				     LAW_TRGT_IF_CCSR);
	}
}
#endif

void enable_cpc(void)
{
	int i;
	int ret;
	u32 size = 0;
	u32 cpccfg0;
	char buffer[HWCONFIG_BUFFER_SIZE];
	char cpc_subarg[16];
	bool have_hwconfig = false;
	int cpc_args = 0;
	cpc_corenet_t *cpc = (cpc_corenet_t *)CONFIG_SYS_FSL_CPC_ADDR;

	/* Extract hwconfig from environment */
	ret = env_get_f("hwconfig", buffer, sizeof(buffer));
	if (ret > 0) {
		/*
		 * If "en_cpc" is not defined in hwconfig then by default all
		 * cpcs are enable. If this config is defined then individual
		 * cpcs which have to be enabled should also be defined.
		 * e.g en_cpc:cpc1,cpc2;
		 */
		if (hwconfig_f("en_cpc", buffer))
			have_hwconfig = true;
	}

	for (i = 0; i < CONFIG_SYS_NUM_CPC; i++, cpc++) {
		if (have_hwconfig) {
			sprintf(cpc_subarg, "cpc%u", i + 1);
			cpc_args = hwconfig_sub_f("en_cpc", cpc_subarg, buffer);
			if (cpc_args == 0)
				continue;
		}
		cpccfg0 = in_be32(&cpc->cpccfg0);
		size += CPC_CFG0_SZ_K(cpccfg0);

#ifdef CONFIG_SYS_FSL_ERRATUM_CPC_A002
		setbits_be32(&cpc->cpchdbcr0, CPC_HDBCR0_TAG_ECC_SCRUB_DIS);
#endif
#ifdef CONFIG_SYS_FSL_ERRATUM_CPC_A003
		setbits_be32(&cpc->cpchdbcr0, CPC_HDBCR0_DATA_ECC_SCRUB_DIS);
#endif
#ifdef CONFIG_SYS_FSL_ERRATUM_A006593
		setbits_be32(&cpc->cpchdbcr0, 1 << (31 - 21));
#endif
#ifdef CONFIG_SYS_FSL_ERRATUM_A006379
		if (has_erratum_a006379()) {
			setbits_be32(&cpc->cpchdbcr0,
				     CPC_HDBCR0_SPLRU_LEVEL_EN);
		}
#endif

		out_be32(&cpc->cpccsr0, CPC_CSR0_CE | CPC_CSR0_PE);
		/* Read back to sync write */
		in_be32(&cpc->cpccsr0);

	}

	puts("Corenet Platform Cache: ");
	print_size(size * 1024, " enabled\n");
}

static void invalidate_cpc(void)
{
	int i;
	cpc_corenet_t *cpc = (cpc_corenet_t *)CONFIG_SYS_FSL_CPC_ADDR;

	for (i = 0; i < CONFIG_SYS_NUM_CPC; i++, cpc++) {
		/* skip CPC when it used as all SRAM */
		if (in_be32(&cpc->cpcsrcr0) & CPC_SRCR0_SRAMEN)
			continue;
		/* Flash invalidate the CPC and clear all the locks */
		out_be32(&cpc->cpccsr0, CPC_CSR0_FI | CPC_CSR0_LFC);
		while (in_be32(&cpc->cpccsr0) & (CPC_CSR0_FI | CPC_CSR0_LFC))
			;
	}
}
#else
#define enable_cpc()
#define invalidate_cpc()
#define disable_cpc_sram()
#endif /* CONFIG_SYS_FSL_CPC */

/*
 * Breathe some life into the CPU...
 *
 * Set up the memory map
 * initialize a bunch of registers
 */

#ifdef CONFIG_FSL_CORENET
static void corenet_tb_init(void)
{
	volatile ccsr_rcpm_t *rcpm =
		(void *)(CONFIG_SYS_FSL_CORENET_RCPM_ADDR);
	volatile ccsr_pic_t *pic =
		(void *)(CONFIG_SYS_MPC8xxx_PIC_ADDR);
	u32 whoami = in_be32(&pic->whoami);

	/* Enable the timebase register for this core */
	out_be32(&rcpm->ctbenrl, (1 << whoami));
}
#endif

#ifdef CONFIG_SYS_FSL_ERRATUM_A007212
void fsl_erratum_a007212_workaround(void)
{
	ccsr_gur_t __iomem *gur = (void *)(CONFIG_SYS_MPC85xx_GUTS_ADDR);
	u32 ddr_pll_ratio;
	u32 __iomem *plldgdcr1 = (void *)(CONFIG_SYS_DCSRBAR + 0x21c20);
	u32 __iomem *plldadcr1 = (void *)(CONFIG_SYS_DCSRBAR + 0x21c28);
	u32 __iomem *dpdovrcr4 = (void *)(CONFIG_SYS_DCSRBAR + 0x21e80);
#if (CONFIG_SYS_NUM_DDR_CTLRS >= 2)
	u32 __iomem *plldgdcr2 = (void *)(CONFIG_SYS_DCSRBAR + 0x21c40);
	u32 __iomem *plldadcr2 = (void *)(CONFIG_SYS_DCSRBAR + 0x21c48);
#if (CONFIG_SYS_NUM_DDR_CTLRS >= 3)
	u32 __iomem *plldgdcr3 = (void *)(CONFIG_SYS_DCSRBAR + 0x21c60);
	u32 __iomem *plldadcr3 = (void *)(CONFIG_SYS_DCSRBAR + 0x21c68);
#endif
#endif
	/*
	 * Even this workaround applies to selected version of SoCs, it is
	 * safe to apply to all versions, with the limitation of odd ratios.
	 * If RCW has disabled DDR PLL, we have to apply this workaround,
	 * otherwise DDR will not work.
	 */
	ddr_pll_ratio = (in_be32(&gur->rcwsr[0]) >>
		FSL_CORENET_RCWSR0_MEM_PLL_RAT_SHIFT) &
		FSL_CORENET_RCWSR0_MEM_PLL_RAT_MASK;
	/* check if RCW sets ratio to 0, required by this workaround */
	if (ddr_pll_ratio != 0)
		return;
	ddr_pll_ratio = (in_be32(&gur->rcwsr[0]) >>
		FSL_CORENET_RCWSR0_MEM_PLL_RAT_RESV_SHIFT) &
		FSL_CORENET_RCWSR0_MEM_PLL_RAT_MASK;
	/* check if reserved bits have the desired ratio */
	if (ddr_pll_ratio == 0) {
		printf("Error: Unknown DDR PLL ratio!\n");
		return;
	}
	ddr_pll_ratio >>= 1;

	setbits_be32(plldadcr1, 0x02000001);
#if (CONFIG_SYS_NUM_DDR_CTLRS >= 2)
	setbits_be32(plldadcr2, 0x02000001);
#if (CONFIG_SYS_NUM_DDR_CTLRS >= 3)
	setbits_be32(plldadcr3, 0x02000001);
#endif
#endif
	setbits_be32(dpdovrcr4, 0xe0000000);
	out_be32(plldgdcr1, 0x08000001 | (ddr_pll_ratio << 1));
#if (CONFIG_SYS_NUM_DDR_CTLRS >= 2)
	out_be32(plldgdcr2, 0x08000001 | (ddr_pll_ratio << 1));
#if (CONFIG_SYS_NUM_DDR_CTLRS >= 3)
	out_be32(plldgdcr3, 0x08000001 | (ddr_pll_ratio << 1));
#endif
#endif
	udelay(100);
	clrbits_be32(plldadcr1, 0x02000001);
#if (CONFIG_SYS_NUM_DDR_CTLRS >= 2)
	clrbits_be32(plldadcr2, 0x02000001);
#if (CONFIG_SYS_NUM_DDR_CTLRS >= 3)
	clrbits_be32(plldadcr3, 0x02000001);
#endif
#endif
	clrbits_be32(dpdovrcr4, 0xe0000000);
}
#endif

ulong cpu_init_f(void)
{
	extern void m8560_cpm_reset (void);
#ifdef CONFIG_SYS_DCSRBAR_PHYS
	ccsr_gur_t *gur = (void *)(CONFIG_SYS_MPC85xx_GUTS_ADDR);
#endif
#if defined(CONFIG_SECURE_BOOT) && !defined(CONFIG_SYS_RAMBOOT)
	struct law_entry law;
#endif
#ifdef CONFIG_ARCH_MPC8548
	ccsr_local_ecm_t *ecm = (void *)(CONFIG_SYS_MPC85xx_ECM_ADDR);
	uint svr = get_svr();

	/*
	 * CPU2 errata workaround: A core hang possible while executing
	 * a msync instruction and a snoopable transaction from an I/O
	 * master tagged to make quick forward progress is present.
	 * Fixed in silicon rev 2.1.
	 */
	if ((SVR_MAJ(svr) == 1) || ((SVR_MAJ(svr) == 2 && SVR_MIN(svr) == 0x0)))
		out_be32(&ecm->eebpcr, in_be32(&ecm->eebpcr) | (1 << 16));
#endif

	disable_tlb(14);
	disable_tlb(15);

#if defined(CONFIG_SECURE_BOOT) && !defined(CONFIG_SYS_RAMBOOT)
	/* Disable the LAW created for NOR flash by the PBI commands */
	law = find_law(CONFIG_SYS_PBI_FLASH_BASE);
	if (law.index != -1)
		disable_law(law.index);

#if defined(CONFIG_SYS_CPC_REINIT_F)
	disable_cpc_sram();
#endif
#endif

#ifdef CONFIG_CPM2
	config_8560_ioports((ccsr_cpm_t *)CONFIG_SYS_MPC85xx_CPM_ADDR);
#endif

       init_early_memctl_regs();

#if defined(CONFIG_CPM2)
	m8560_cpm_reset();
#endif

#if defined(CONFIG_QE) && !defined(CONFIG_U_QE)
	/* Config QE ioports */
	config_qe_ioports();
#endif

#if defined(CONFIG_FSL_DMA)
	dma_init();
#endif
#ifdef CONFIG_FSL_CORENET
	corenet_tb_init();
#endif
	init_used_tlb_cams();

	/* Invalidate the CPC before DDR gets enabled */
	invalidate_cpc();

 #ifdef CONFIG_SYS_DCSRBAR_PHYS
	/* set DCSRCR so that DCSR space is 1G */
	setbits_be32(&gur->dcsrcr, FSL_CORENET_DCSR_SZ_1G);
	in_be32(&gur->dcsrcr);
#endif

#ifdef CONFIG_SYS_FSL_ERRATUM_A007212
	fsl_erratum_a007212_workaround();
#endif

	return 0;
}

/* Implement a dummy function for those platforms w/o SERDES */
static void __fsl_serdes__init(void)
{
	return ;
}
__attribute__((weak, alias("__fsl_serdes__init"))) void fsl_serdes_init(void);

#if defined(CONFIG_SYS_FSL_QORIQ_CHASSIS2) && defined(CONFIG_E6500)
int enable_cluster_l2(void)
{
	int i = 0;
	u32 cluster, svr = get_svr();
	ccsr_gur_t *gur = (void __iomem *)(CONFIG_SYS_MPC85xx_GUTS_ADDR);
	struct ccsr_cluster_l2 __iomem *l2cache;

	/* only the L2 of first cluster should be enabled as expected on T4080,
	 * but there is no EOC in the first cluster as HW sake, so return here
	 * to skip enabling L2 cache of the 2nd cluster.
	 */
	if (SVR_SOC_VER(svr) == SVR_T4080)
		return 0;

	cluster = in_be32(&gur->tp_cluster[i].lower);
	if (cluster & TP_CLUSTER_EOC)
		return 0;

	/* The first cache has already been set up, so skip it */
	i++;

	/* Look through the remaining clusters, and set up their caches */
	do {
		int j, cluster_valid = 0;

		l2cache = (void __iomem *)(CONFIG_SYS_FSL_CLUSTER_1_L2 + i * 0x40000);

		cluster = in_be32(&gur->tp_cluster[i].lower);

		/* check that at least one core/accel is enabled in cluster */
		for (j = 0; j < 4; j++) {
			u32 idx = (cluster >> (j*8)) & TP_CLUSTER_INIT_MASK;
			u32 type = in_be32(&gur->tp_ityp[idx]);

			if ((type & TP_ITYP_AV) &&
			    TP_ITYP_TYPE(type) == TP_ITYP_TYPE_PPC)
				cluster_valid = 1;
		}

		if (cluster_valid) {
			/* set stash ID to (cluster) * 2 + 32 + 1 */
			clrsetbits_be32(&l2cache->l2csr1, 0xff, 32 + i * 2 + 1);

			printf("enable l2 for cluster %d %p\n", i, l2cache);

			out_be32(&l2cache->l2csr0, L2CSR0_L2FI|L2CSR0_L2LFC);
			while ((in_be32(&l2cache->l2csr0)
				& (L2CSR0_L2FI|L2CSR0_L2LFC)) != 0)
					;
			out_be32(&l2cache->l2csr0, L2CSR0_L2E|L2CSR0_L2PE|L2CSR0_L2REP_MODE);
		}
		i++;
	} while (!(cluster & TP_CLUSTER_EOC));

	return 0;
}
#endif

/*
 * Initialize L2 as cache.
 */
int l2cache_init(void)
{
	__maybe_unused u32 svr = get_svr();
#ifdef CONFIG_L2_CACHE
	ccsr_l2cache_t *l2cache = (void __iomem *)CONFIG_SYS_MPC85xx_L2_ADDR;
#elif defined(CONFIG_SYS_FSL_QORIQ_CHASSIS2) && defined(CONFIG_E6500)
	struct ccsr_cluster_l2 * l2cache = (void __iomem *)CONFIG_SYS_FSL_CLUSTER_1_L2;
#endif

	puts ("L2:    ");

#if defined(CONFIG_L2_CACHE)
	volatile uint cache_ctl;
	uint ver;
	u32 l2siz_field;

	ver = SVR_SOC_VER(svr);

	asm("msync;isync");
	cache_ctl = l2cache->l2ctl;

#if defined(CONFIG_SYS_RAMBOOT) && defined(CONFIG_SYS_INIT_L2_ADDR)
	if (cache_ctl & MPC85xx_L2CTL_L2E) {
		/* Clear L2 SRAM memory-mapped base address */
		out_be32(&l2cache->l2srbar0, 0x0);
		out_be32(&l2cache->l2srbar1, 0x0);

		/* set MBECCDIS=0, SBECCDIS=0 */
		clrbits_be32(&l2cache->l2errdis,
				(MPC85xx_L2ERRDIS_MBECC |
				 MPC85xx_L2ERRDIS_SBECC));

		/* set L2E=0, L2SRAM=0 */
		clrbits_be32(&l2cache->l2ctl,
				(MPC85xx_L2CTL_L2E |
				 MPC85xx_L2CTL_L2SRAM_ENTIRE));
	}
#endif

	l2siz_field = (cache_ctl >> 28) & 0x3;

	switch (l2siz_field) {
	case 0x0:
		printf(" unknown size (0x%08x)\n", cache_ctl);
		return -1;
		break;
	case 0x1:
		if (ver == SVR_8540 || ver == SVR_8560   ||
		    ver == SVR_8541 || ver == SVR_8555) {
			puts("128 KiB ");
			/* set L2E=1, L2I=1, & L2BLKSZ=1 (128 KiBibyte) */
			cache_ctl = 0xc4000000;
		} else {
			puts("256 KiB ");
			cache_ctl = 0xc0000000; /* set L2E=1, L2I=1, & L2SRAM=0 */
		}
		break;
	case 0x2:
		if (ver == SVR_8540 || ver == SVR_8560   ||
		    ver == SVR_8541 || ver == SVR_8555) {
			puts("256 KiB ");
			/* set L2E=1, L2I=1, & L2BLKSZ=2 (256 KiBibyte) */
			cache_ctl = 0xc8000000;
		} else {
			puts("512 KiB ");
			/* set L2E=1, L2I=1, & L2SRAM=0 */
			cache_ctl = 0xc0000000;
		}
		break;
	case 0x3:
		puts("1024 KiB ");
		/* set L2E=1, L2I=1, & L2SRAM=0 */
		cache_ctl = 0xc0000000;
		break;
	}

	if (l2cache->l2ctl & MPC85xx_L2CTL_L2E) {
		puts("already enabled");
#if defined(CONFIG_SYS_INIT_L2_ADDR) && defined(CONFIG_SYS_FLASH_BASE)
		u32 l2srbar = l2cache->l2srbar0;
		if (l2cache->l2ctl & MPC85xx_L2CTL_L2SRAM_ENTIRE
				&& l2srbar >= CONFIG_SYS_FLASH_BASE) {
			l2srbar = CONFIG_SYS_INIT_L2_ADDR;
			l2cache->l2srbar0 = l2srbar;
			printf(", moving to 0x%08x", CONFIG_SYS_INIT_L2_ADDR);
		}
#endif /* CONFIG_SYS_INIT_L2_ADDR */
		puts("\n");
	} else {
		asm("msync;isync");
		l2cache->l2ctl = cache_ctl; /* invalidate & enable */
		asm("msync;isync");
		puts("enabled\n");
	}
#elif defined(CONFIG_BACKSIDE_L2_CACHE)
	if (SVR_SOC_VER(svr) == SVR_P2040) {
		puts("N/A\n");
		goto skip_l2;
	}

	u32 l2cfg0 = mfspr(SPRN_L2CFG0);

	/* invalidate the L2 cache */
	mtspr(SPRN_L2CSR0, (L2CSR0_L2FI|L2CSR0_L2LFC));
	while (mfspr(SPRN_L2CSR0) & (L2CSR0_L2FI|L2CSR0_L2LFC))
		;

#ifdef CONFIG_SYS_CACHE_STASHING
	/* set stash id to (coreID) * 2 + 32 + L2 (1) */
	mtspr(SPRN_L2CSR1, (32 + 1));
#endif

	/* enable the cache */
	mtspr(SPRN_L2CSR0, CONFIG_SYS_INIT_L2CSR0);

	if (CONFIG_SYS_INIT_L2CSR0 & L2CSR0_L2E) {
		while (!(mfspr(SPRN_L2CSR0) & L2CSR0_L2E))
			;
		print_size((l2cfg0 & 0x3fff) * 64 * 1024, " enabled\n");
	}

skip_l2:
#elif defined(CONFIG_SYS_FSL_QORIQ_CHASSIS2) && defined(CONFIG_E6500)
	if (l2cache->l2csr0 & L2CSR0_L2E)
		print_size((l2cache->l2cfg0 & 0x3fff) * 64 * 1024,
			   " enabled\n");

	enable_cluster_l2();
#else
	puts("disabled\n");
#endif

	return 0;
}

/*
 *
 * The newer 8548, etc, parts have twice as much cache, but
 * use the same bit-encoding as the older 8555, etc, parts.
 *
 */
int cpu_init_r(void)
{
	__maybe_unused u32 svr = get_svr();
#ifdef CONFIG_SYS_LBC_LCRR
	fsl_lbc_t *lbc = (void __iomem *)LBC_BASE_ADDR;
#endif
#if defined(CONFIG_PPC_SPINTABLE_COMPATIBLE) && defined(CONFIG_MP)
	extern int spin_table_compat;
	const char *spin;
#endif
#ifdef CONFIG_SYS_FSL_ERRATUM_SEC_A003571
	ccsr_sec_t __iomem *sec = (void *)CONFIG_SYS_FSL_SEC_ADDR;
#endif
#if defined(CONFIG_SYS_P4080_ERRATUM_CPU22) || \
	defined(CONFIG_SYS_FSL_ERRATUM_NMG_CPU_A011)
	/*
	 * CPU22 and NMG_CPU_A011 share the same workaround.
	 * CPU22 applies to P4080 rev 1.0, 2.0, fixed in 3.0
	 * NMG_CPU_A011 applies to P4080 rev 1.0, 2.0, fixed in 3.0
	 * also applies to P3041 rev 1.0, 1.1, P2041 rev 1.0, 1.1, both
	 * fixed in 2.0. NMG_CPU_A011 is activated by default and can
	 * be disabled by hwconfig with syntax:
	 *
	 * fsl_cpu_a011:disable
	 */
	extern int enable_cpu_a011_workaround;
#ifdef CONFIG_SYS_P4080_ERRATUM_CPU22
	enable_cpu_a011_workaround = (SVR_MAJ(svr) < 3);
#else
	char buffer[HWCONFIG_BUFFER_SIZE];
	char *buf = NULL;
	int n, res;

	n = env_get_f("hwconfig", buffer, sizeof(buffer));
	if (n > 0)
		buf = buffer;

	res = hwconfig_arg_cmp_f("fsl_cpu_a011", "disable", buf);
	if (res > 0) {
		enable_cpu_a011_workaround = 0;
	} else {
		if (n >= HWCONFIG_BUFFER_SIZE) {
			printf("fsl_cpu_a011 was not found. hwconfig variable "
				"may be too long\n");
		}
		enable_cpu_a011_workaround =
			(SVR_SOC_VER(svr) == SVR_P4080 && SVR_MAJ(svr) < 3) ||
			(SVR_SOC_VER(svr) != SVR_P4080 && SVR_MAJ(svr) < 2);
	}
#endif
	if (enable_cpu_a011_workaround) {
		flush_dcache();
		mtspr(L1CSR2, (mfspr(L1CSR2) | L1CSR2_DCWS));
		sync();
	}
#endif

#ifdef CONFIG_SYS_FSL_ERRATUM_A007907
	flush_dcache();
	mtspr(L1CSR2, (mfspr(L1CSR2) & ~L1CSR2_DCSTASHID));
	sync();
#endif

#ifdef CONFIG_SYS_FSL_ERRATUM_A005812
	/*
	 * A-005812 workaround sets bit 32 of SPR 976 for SoCs running
	 * in write shadow mode. Checking DCWS before setting SPR 976.
	 */
	if (mfspr(L1CSR2) & L1CSR2_DCWS)
		mtspr(SPRN_HDBCR0, (mfspr(SPRN_HDBCR0) | 0x80000000));
#endif

#if defined(CONFIG_PPC_SPINTABLE_COMPATIBLE) && defined(CONFIG_MP)
	spin = env_get("spin_table_compat");
	if (spin && (*spin == 'n'))
		spin_table_compat = 0;
	else
		spin_table_compat = 1;
#endif

#ifdef CONFIG_FSL_CORENET
	set_liodns();
#ifdef CONFIG_SYS_DPAA_QBMAN
	setup_qbman_portals();
#endif
#endif

	l2cache_init();
#if defined(CONFIG_RAMBOOT_PBL)
	disable_cpc_sram();
#endif
	enable_cpc();
#if defined(T1040_TDM_QUIRK_CCSR_BASE)
	enable_tdm_law();
#endif

#ifndef CONFIG_SYS_FSL_NO_SERDES
	/* needs to be in ram since code uses global static vars */
	fsl_serdes_init();
#endif

#ifdef CONFIG_SYS_FSL_ERRATUM_SEC_A003571
#define MCFGR_AXIPIPE 0x000000f0
	if (IS_SVR_REV(svr, 1, 0))
		sec_clrbits32(&sec->mcfgr, MCFGR_AXIPIPE);
#endif

#ifdef CONFIG_SYS_FSL_ERRATUM_A005871
	if (IS_SVR_REV(svr, 1, 0)) {
		int i;
		__be32 *p = (void __iomem *)CONFIG_SYS_DCSRBAR + 0xb004c;

		for (i = 0; i < 12; i++) {
			p += i + (i > 5 ? 11 : 0);
			out_be32(p, 0x2);
		}
		p = (void __iomem *)CONFIG_SYS_DCSRBAR + 0xb0108;
		out_be32(p, 0x34);
	}
#endif

#ifdef CONFIG_SYS_SRIO
	srio_init();
#ifdef CONFIG_SRIO_PCIE_BOOT_MASTER
	char *s = env_get("bootmaster");
	if (s) {
		if (!strcmp(s, "SRIO1")) {
			srio_boot_master(1);
			srio_boot_master_release_slave(1);
		}
		if (!strcmp(s, "SRIO2")) {
			srio_boot_master(2);
			srio_boot_master_release_slave(2);
		}
	}
#endif
#endif

#if defined(CONFIG_MP)
	setup_mp();
#endif

#ifdef CONFIG_SYS_FSL_ERRATUM_ESDHC13
	{
		if (SVR_MAJ(svr) < 3) {
			void *p;
			p = (void *)CONFIG_SYS_DCSRBAR + 0x20520;
			setbits_be32(p, 1 << (31 - 14));
		}
	}
#endif

#ifdef CONFIG_SYS_LBC_LCRR
	/*
	 * Modify the CLKDIV field of LCRR register to improve the writing
	 * speed for NOR flash.
	 */
	clrsetbits_be32(&lbc->lcrr, LCRR_CLKDIV, CONFIG_SYS_LBC_LCRR);
	__raw_readl(&lbc->lcrr);
	isync();
#ifdef CONFIG_SYS_FSL_ERRATUM_NMG_LBC103
	udelay(100);
#endif
#endif

#ifdef CONFIG_SYS_FSL_USB1_PHY_ENABLE
	{
		struct ccsr_usb_phy __iomem *usb_phy1 =
			(void *)CONFIG_SYS_MPC85xx_USB1_PHY_ADDR;
#ifdef CONFIG_SYS_FSL_ERRATUM_A006261
		if (has_erratum_a006261())
			fsl_erratum_a006261_workaround(usb_phy1);
#endif
		out_be32(&usb_phy1->usb_enable_override,
				CONFIG_SYS_FSL_USB_ENABLE_OVERRIDE);
	}
#endif
#ifdef CONFIG_SYS_FSL_USB2_PHY_ENABLE
	{
		struct ccsr_usb_phy __iomem *usb_phy2 =
			(void *)CONFIG_SYS_MPC85xx_USB2_PHY_ADDR;
#ifdef CONFIG_SYS_FSL_ERRATUM_A006261
		if (has_erratum_a006261())
			fsl_erratum_a006261_workaround(usb_phy2);
#endif
		out_be32(&usb_phy2->usb_enable_override,
				CONFIG_SYS_FSL_USB_ENABLE_OVERRIDE);
	}
#endif

#ifdef CONFIG_SYS_FSL_ERRATUM_USB14
	/* On P204x/P304x/P50x0 Rev1.0, USB transmit will result internal
	 * multi-bit ECC errors which has impact on performance, so software
	 * should disable all ECC reporting from USB1 and USB2.
	 */
	if (IS_SVR_REV(get_svr(), 1, 0)) {
		struct dcsr_dcfg_regs *dcfg = (struct dcsr_dcfg_regs *)
			(CONFIG_SYS_DCSRBAR + CONFIG_SYS_DCSR_DCFG_OFFSET);
		setbits_be32(&dcfg->ecccr1,
				(DCSR_DCFG_ECC_DISABLE_USB1 |
				 DCSR_DCFG_ECC_DISABLE_USB2));
	}
#endif

#if defined(CONFIG_SYS_FSL_USB_DUAL_PHY_ENABLE)
		struct ccsr_usb_phy __iomem *usb_phy =
			(void *)CONFIG_SYS_MPC85xx_USB1_PHY_ADDR;
		setbits_be32(&usb_phy->pllprg[1],
			     CONFIG_SYS_FSL_USB_PLLPRG2_PHY2_CLK_EN |
			     CONFIG_SYS_FSL_USB_PLLPRG2_PHY1_CLK_EN |
			     CONFIG_SYS_FSL_USB_PLLPRG2_MFI |
			     CONFIG_SYS_FSL_USB_PLLPRG2_PLL_EN);
#ifdef CONFIG_SYS_FSL_SINGLE_SOURCE_CLK
		usb_single_source_clk_configure(usb_phy);
#endif
		setbits_be32(&usb_phy->port1.ctrl,
			     CONFIG_SYS_FSL_USB_CTRL_PHY_EN);
		setbits_be32(&usb_phy->port1.drvvbuscfg,
			     CONFIG_SYS_FSL_USB_DRVVBUS_CR_EN);
		setbits_be32(&usb_phy->port1.pwrfltcfg,
			     CONFIG_SYS_FSL_USB_PWRFLT_CR_EN);
		setbits_be32(&usb_phy->port2.ctrl,
			     CONFIG_SYS_FSL_USB_CTRL_PHY_EN);
		setbits_be32(&usb_phy->port2.drvvbuscfg,
			     CONFIG_SYS_FSL_USB_DRVVBUS_CR_EN);
		setbits_be32(&usb_phy->port2.pwrfltcfg,
			     CONFIG_SYS_FSL_USB_PWRFLT_CR_EN);

#ifdef CONFIG_SYS_FSL_ERRATUM_A006261
		if (has_erratum_a006261())
			fsl_erratum_a006261_workaround(usb_phy);
#endif

#endif /* CONFIG_SYS_FSL_USB_DUAL_PHY_ENABLE */

#ifdef CONFIG_SYS_FSL_ERRATUM_A009942
	erratum_a009942_check_cpo();
#endif

#ifdef CONFIG_FMAN_ENET
	fman_enet_init();
#endif

#if defined(CONFIG_SECURE_BOOT) && defined(CONFIG_FSL_CORENET)
	if (pamu_init() < 0)
		fsl_secboot_handle_error(ERROR_ESBC_PAMU_INIT);
#endif

#ifdef CONFIG_FSL_CAAM
	sec_init();

#if defined(CONFIG_ARCH_C29X)
	if ((SVR_SOC_VER(svr) == SVR_C292) ||
	    (SVR_SOC_VER(svr) == SVR_C293))
		sec_init_idx(1);

	if (SVR_SOC_VER(svr) == SVR_C293)
		sec_init_idx(2);
#endif
#endif

#if defined(CONFIG_FSL_SATA_V2) && defined(CONFIG_SYS_FSL_ERRATUM_SATA_A001)
	/*
	 * For P1022/1013 Rev1.0 silicon, after power on SATA host
	 * controller is configured in legacy mode instead of the
	 * expected enterprise mode. Software needs to clear bit[28]
	 * of HControl register to change to enterprise mode from
	 * legacy mode.  We assume that the controller is offline.
	 */
	if (IS_SVR_REV(svr, 1, 0) &&
	    ((SVR_SOC_VER(svr) == SVR_P1022) ||
	     (SVR_SOC_VER(svr) == SVR_P1013))) {
		fsl_sata_reg_t *reg;

		/* first SATA controller */
		reg = (void *)CONFIG_SYS_MPC85xx_SATA1_ADDR;
		clrbits_le32(&reg->hcontrol, HCONTROL_ENTERPRISE_EN);

		/* second SATA controller */
		reg = (void *)CONFIG_SYS_MPC85xx_SATA2_ADDR;
		clrbits_le32(&reg->hcontrol, HCONTROL_ENTERPRISE_EN);
	}
#endif

	init_used_tlb_cams();

	return 0;
}

void arch_preboot_os(void)
{
	u32 msr;

	/*
	 * We are changing interrupt offsets and are about to boot the OS so
	 * we need to make sure we disable all async interrupts. EE is already
	 * disabled by the time we get called.
	 */
	msr = mfmsr();
	msr &= ~(MSR_ME|MSR_CE);
	mtmsr(msr);
}

#if defined(CONFIG_SATA) && defined(CONFIG_FSL_SATA)
int sata_initialize(void)
{
	if (is_serdes_configured(SATA1) || is_serdes_configured(SATA2))
		return __sata_initialize();

	return 1;
}
#endif

void cpu_secondary_init_r(void)
{
#ifdef CONFIG_U_QE
	uint qe_base = CONFIG_SYS_IMMR + 0x00140000; /* QE immr base */
#elif defined CONFIG_QE
	uint qe_base = CONFIG_SYS_IMMR + 0x00080000; /* QE immr base */
#endif

#ifdef CONFIG_QE
	qe_init(qe_base);
	qe_reset();
#endif
}

#ifdef CONFIG_BOARD_LATE_INIT
int board_late_init(void)
{
#ifdef CONFIG_CHAIN_OF_TRUST
	fsl_setenv_chain_of_trust();
#endif

	return 0;
}
#endif
