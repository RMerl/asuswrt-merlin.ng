// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2008-2012 Freescale Semiconductor, Inc.
 */

#include <common.h>
#include <command.h>
#include <pci.h>
#include <asm/processor.h>
#include <asm/mmu.h>
#include <asm/cache.h>
#include <asm/immap_85xx.h>
#include <asm/fsl_pci.h>
#include <fsl_ddr_sdram.h>
#include <asm/io.h>
#include <asm/fsl_serdes.h>
#include <spd.h>
#include <miiphy.h>
#include <linux/libfdt.h>
#include <spd_sdram.h>
#include <fdt_support.h>
#include <fsl_mdio.h>
#include <tsec.h>
#include <netdev.h>
#include <sata.h>

#include "../common/sgmii_riser.h"

int board_early_init_f (void)
{
#ifdef CONFIG_MMC
	volatile ccsr_gur_t *gur = (void *)(CONFIG_SYS_MPC85xx_GUTS_ADDR);

	setbits_be32(&gur->pmuxcr,
			(MPC85xx_PMUXCR_SDHC_CD |
			 MPC85xx_PMUXCR_SDHC_WP));

	/* The MPC8536DS board insert the SDHC_WP pin for erratum NMG_eSDHC118,
	 * however, this erratum only applies to MPC8536 Rev1.0.
	 * So set SDHC_WP to active-low when use MPC8536 Rev1.1 and greater.*/
	if ((((SVR_MAJ(get_svr()) & 0x7) == 0x1) &&
			(SVR_MIN(get_svr()) >= 0x1))
			|| (SVR_MAJ(get_svr() & 0x7) > 0x1))
		setbits_be32(&gur->gencfgr, MPC85xx_GENCFGR_SDHC_WP_INV);
#endif
	return 0;
}

int checkboard (void)
{
	u8 vboot;
	u8 *pixis_base = (u8 *)PIXIS_BASE;

	printf("Board: MPC8536DS Sys ID: 0x%02x, "
		"Sys Ver: 0x%02x, FPGA Ver: 0x%02x, ",
		in_8(pixis_base + PIXIS_ID), in_8(pixis_base + PIXIS_VER),
		in_8(pixis_base + PIXIS_PVER));

	vboot = in_8(pixis_base + PIXIS_VBOOT);
	switch ((vboot & PIXIS_VBOOT_LBMAP) >> 5) {
		case PIXIS_VBOOT_LBMAP_NOR0:
			puts ("vBank: 0\n");
			break;
		case PIXIS_VBOOT_LBMAP_NOR1:
			puts ("vBank: 1\n");
			break;
		case PIXIS_VBOOT_LBMAP_NOR2:
			puts ("vBank: 2\n");
			break;
		case PIXIS_VBOOT_LBMAP_NOR3:
			puts ("vBank: 3\n");
			break;
		case PIXIS_VBOOT_LBMAP_PJET:
			puts ("Promjet\n");
			break;
		case PIXIS_VBOOT_LBMAP_NAND:
			puts ("NAND\n");
			break;
	}

	return 0;
}

#if !defined(CONFIG_SPD_EEPROM)
/*
 * Fixed sdram init -- doesn't use serial presence detect.
 */

phys_size_t fixed_sdram (void)
{
	volatile immap_t *immap = (immap_t *)CONFIG_SYS_IMMR;
	struct ccsr_ddr __iomem *ddr = &immap->im_ddr;
	uint d_init;

	ddr->cs0_bnds = CONFIG_SYS_DDR_CS0_BNDS;
	ddr->cs0_config = CONFIG_SYS_DDR_CS0_CONFIG;

	ddr->timing_cfg_3 = CONFIG_SYS_DDR_TIMING_3;
	ddr->timing_cfg_0 = CONFIG_SYS_DDR_TIMING_0;
	ddr->timing_cfg_1 = CONFIG_SYS_DDR_TIMING_1;
	ddr->timing_cfg_2 = CONFIG_SYS_DDR_TIMING_2;
	ddr->sdram_mode = CONFIG_SYS_DDR_MODE_1;
	ddr->sdram_mode_2 = CONFIG_SYS_DDR_MODE_2;
	ddr->sdram_interval = CONFIG_SYS_DDR_INTERVAL;
	ddr->sdram_data_init = CONFIG_SYS_DDR_DATA_INIT;
	ddr->sdram_clk_cntl = CONFIG_SYS_DDR_CLK_CTRL;
	ddr->sdram_cfg_2 = CONFIG_SYS_DDR_CONTROL2;

#if defined (CONFIG_DDR_ECC)
	ddr->err_int_en = CONFIG_SYS_DDR_ERR_INT_EN;
	ddr->err_disable = CONFIG_SYS_DDR_ERR_DIS;
	ddr->err_sbe = CONFIG_SYS_DDR_SBE;
#endif
	asm("sync;isync");

	udelay(500);

	ddr->sdram_cfg = CONFIG_SYS_DDR_CONTROL;

#if defined(CONFIG_ECC_INIT_VIA_DDRCONTROLLER)
	d_init = 1;
	debug("DDR - 1st controller: memory initializing\n");
	/*
	 * Poll until memory is initialized.
	 * 512 Meg at 400 might hit this 200 times or so.
	 */
	while ((ddr->sdram_cfg_2 & (d_init << 4)) != 0) {
		udelay(1000);
	}
	debug("DDR: memory initialized\n\n");
	asm("sync; isync");
	udelay(500);
#endif

	return 512 * 1024 * 1024;
}

#endif

#ifdef CONFIG_PCI1
static struct pci_controller pci1_hose;
#endif

#ifdef CONFIG_PCI
void pci_init_board(void)
{
	ccsr_gur_t *gur = (void *)(CONFIG_SYS_MPC85xx_GUTS_ADDR);
	struct fsl_pci_info pci_info;
	u32 devdisr, pordevsr;
	u32 porpllsr, pci_agent, pci_speed, pci_32, pci_arb, pci_clk_sel;
	int first_free_busno;

	first_free_busno = fsl_pcie_init_board(0);

#ifdef CONFIG_PCI1
	devdisr = in_be32(&gur->devdisr);
	pordevsr = in_be32(&gur->pordevsr);
	porpllsr = in_be32(&gur->porpllsr);

	pci_speed = 66666000;
	pci_32 = 1;
	pci_arb = pordevsr & MPC85xx_PORDEVSR_PCI1_ARB;
	pci_clk_sel = porpllsr & MPC85xx_PORDEVSR_PCI1_SPD;

	if (!(devdisr & MPC85xx_DEVDISR_PCI1)) {
		SET_STD_PCI_INFO(pci_info, 1);
		set_next_law(pci_info.mem_phys,
			law_size_bits(pci_info.mem_size), pci_info.law);
		set_next_law(pci_info.io_phys,
			law_size_bits(pci_info.io_size), pci_info.law);

		pci_agent = fsl_setup_hose(&pci1_hose, pci_info.regs);
		printf("PCI: %d bit, %s MHz, %s, %s, %s (base address %lx)\n",
			(pci_32) ? 32 : 64,
			(pci_speed == 33333000) ? "33" :
			(pci_speed == 66666000) ? "66" : "unknown",
			pci_clk_sel ? "sync" : "async",
			pci_agent ? "agent" : "host",
			pci_arb ? "arbiter" : "external-arbiter",
			pci_info.regs);

		first_free_busno = fsl_pci_init_port(&pci_info,
					&pci1_hose, first_free_busno);
	} else {
		printf("PCI: disabled\n");
	}

	puts("\n");
#else
	setbits_be32(&gur->devdisr, MPC85xx_DEVDISR_PCI1); /* disable */
#endif
}
#endif

int board_early_init_r(void)
{
	const unsigned int flashbase = CONFIG_SYS_FLASH_BASE;
	int flash_esel = find_tlb_idx((void *)flashbase, 1);

	/*
	 * Remap Boot flash + PROMJET region to caching-inhibited
	 * so that flash can be erased properly.
	 */

	/* Flush d-cache and invalidate i-cache of any FLASH data */
	flush_dcache();
	invalidate_icache();

	if (flash_esel == -1) {
		/* very unlikely unless something is messed up */
		puts("Error: Could not find TLB for FLASH BASE\n");
		flash_esel = 1;	/* give our best effort to continue */
	} else {
		/* invalidate existing TLB entry for flash + promjet */
		disable_tlb(flash_esel);
	}

	set_tlb(1, flashbase, CONFIG_SYS_FLASH_BASE_PHYS,	/* tlb, epn, rpn */
		MAS3_SX|MAS3_SW|MAS3_SR, MAS2_I|MAS2_G,	/* perms, wimge */
		0, flash_esel, BOOKE_PAGESZ_256M, 1);	/* ts, esel, tsize, iprot */

	return 0;
}

int board_eth_init(bd_t *bis)
{
#ifdef CONFIG_TSEC_ENET
	struct fsl_pq_mdio_info mdio_info;
	struct tsec_info_struct tsec_info[2];
	int num = 0;

#ifdef CONFIG_TSEC1
	SET_STD_TSEC_INFO(tsec_info[num], 1);
	if (is_serdes_configured(SGMII_TSEC1)) {
		puts("eTSEC1 is in sgmii mode.\n");
		tsec_info[num].phyaddr = 0;
		tsec_info[num].flags |= TSEC_SGMII;
	}
	num++;
#endif
#ifdef CONFIG_TSEC3
	SET_STD_TSEC_INFO(tsec_info[num], 3);
	if (is_serdes_configured(SGMII_TSEC3)) {
		puts("eTSEC3 is in sgmii mode.\n");
		tsec_info[num].phyaddr = 1;
		tsec_info[num].flags |= TSEC_SGMII;
	}
	num++;
#endif

	if (!num) {
		printf("No TSECs initialized\n");
		return 0;
	}

#ifdef CONFIG_FSL_SGMII_RISER
	if (is_serdes_configured(SGMII_TSEC1) ||
	    is_serdes_configured(SGMII_TSEC3)) {
		fsl_sgmii_riser_init(tsec_info, num);
	}
#endif

	mdio_info.regs = (struct tsec_mii_mng *)CONFIG_SYS_MDIO_BASE_ADDR;
	mdio_info.name = DEFAULT_MII_NAME;
	fsl_pq_mdio_init(bis, &mdio_info);

	tsec_eth_init(bis, tsec_info, num);
#endif
	return pci_eth_init(bis);
}

#if defined(CONFIG_OF_BOARD_SETUP)
int ft_board_setup(void *blob, bd_t *bd)
{
	ft_cpu_setup(blob, bd);

	FT_FSL_PCI_SETUP;

#ifdef CONFIG_FSL_SGMII_RISER
	fsl_sgmii_riser_fdt_fixup(blob);
#endif

#ifdef CONFIG_HAS_FSL_MPH_USB
	fsl_fdt_fixup_dr_usb(blob, bd);
#endif

	return 0;
}
#endif
