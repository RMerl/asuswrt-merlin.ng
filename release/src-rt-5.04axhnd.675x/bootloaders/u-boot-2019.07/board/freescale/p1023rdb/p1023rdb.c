// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2013 Freescale Semiconductor, Inc.
 *
 * Authors:  Roy Zang <tie-fei.zang@freescale.com>
 *           Chunhe Lan <Chunhe.Lan@freescale.com>
 */

#include <common.h>
#include <command.h>
#include <pci.h>
#include <asm/io.h>
#include <asm/cache.h>
#include <asm/processor.h>
#include <asm/mmu.h>
#include <asm/immap_85xx.h>
#include <asm/fsl_pci.h>
#include <fsl_ddr_sdram.h>
#include <asm/fsl_portals.h>
#include <fsl_qbman.h>
#include <linux/libfdt.h>
#include <fdt_support.h>
#include <netdev.h>
#include <malloc.h>
#include <fm_eth.h>
#include <fsl_mdio.h>
#include <miiphy.h>
#include <phy.h>
#include <fsl_dtsec.h>

DECLARE_GLOBAL_DATA_PTR;

int board_early_init_f(void)
{
	fsl_lbc_t *lbc = LBC_BASE_ADDR;

	/* Set ABSWP to implement conversion of addresses in the LBC */
	setbits_be32(&lbc->lbcr, CONFIG_SYS_LBC_LBCR);

	return 0;
}

int checkboard(void)
{
	printf("Board: P1023 RDB\n");

	return 0;
}

#ifdef CONFIG_PCI
void pci_init_board(void)
{
	fsl_pcie_init_board(0);
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
		flash_esel = 2;	/* give our best effort to continue */
	} else {
		/* invalidate existing TLB entry for flash + promjet */
		disable_tlb(flash_esel);
	}

	set_tlb(1, flashbase, CONFIG_SYS_FLASH_BASE_PHYS,
		MAS3_SW|MAS3_SR, MAS2_I|MAS2_G,
		0, flash_esel, BOOKE_PAGESZ_256M, 1);

	setup_qbman_portals();

	return 0;
}

unsigned long get_board_sys_clk(ulong dummy)
{
	return gd->bus_clk;
}

unsigned long get_board_ddr_clk(ulong dummy)
{
	return gd->mem_clk;
}

int board_eth_init(bd_t *bis)
{
	ccsr_gur_t *gur = (ccsr_gur_t *)CONFIG_SYS_MPC85xx_GUTS_ADDR;
	struct fsl_pq_mdio_info dtsec_mdio_info;

	/*
	 * Need to set dTSEC 1 pin multiplexing to TSEC. The default setting
	 * is not correct.
	 */
	setbits_be32(&gur->pmuxcr, MPC85xx_PMUXCR_TSEC1_1);

	dtsec_mdio_info.regs =
		(struct tsec_mii_mng *)CONFIG_SYS_FM1_DTSEC1_MDIO_ADDR;
	dtsec_mdio_info.name = DEFAULT_FM_MDIO_NAME;

	/* Register the 1G MDIO bus */
	fsl_pq_mdio_init(bis, &dtsec_mdio_info);

	fm_info_set_phy_address(FM1_DTSEC1, CONFIG_SYS_FM1_DTSEC1_PHY_ADDR);
	fm_info_set_phy_address(FM1_DTSEC2, CONFIG_SYS_FM1_DTSEC2_PHY_ADDR);

	fm_info_set_mdio(FM1_DTSEC1,
			 miiphy_get_dev_by_name(DEFAULT_FM_MDIO_NAME));
	fm_info_set_mdio(FM1_DTSEC2,
			 miiphy_get_dev_by_name(DEFAULT_FM_MDIO_NAME));

#ifdef CONFIG_FMAN_ENET
	cpu_eth_init(bis);
#endif

	return pci_eth_init(bis);
}

#if defined(CONFIG_OF_BOARD_SETUP)
int ft_board_setup(void *blob, bd_t *bd)
{
	phys_addr_t base;
	phys_size_t size;

	ft_cpu_setup(blob, bd);

	base = env_get_bootm_low();
	size = env_get_bootm_size();

	fdt_fixup_memory(blob, (u64)base, (u64)size);

#ifdef CONFIG_HAS_FSL_DR_USB
	fsl_fdt_fixup_dr_usb(blob, bd);
#endif

	fdt_fixup_fman_ethernet(blob);

	return 0;
}
#endif
