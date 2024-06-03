// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2007-2011 Freescale Semiconductor, Inc.
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
#include <miiphy.h>
#include <linux/libfdt.h>
#include <fdt_support.h>
#include <tsec.h>
#include <fsl_mdio.h>
#include <netdev.h>

#include "../common/sgmii_riser.h"

int checkboard (void)
{
	u8 vboot;
	u8 *pixis_base = (u8 *)PIXIS_BASE;

	printf("Board: MPC8572DS Sys ID: 0x%02x, "
		"Sys Ver: 0x%02x, FPGA Ver: 0x%02x, ",
		in_8(pixis_base + PIXIS_ID), in_8(pixis_base + PIXIS_VER),
		in_8(pixis_base + PIXIS_PVER));

	vboot = in_8(pixis_base + PIXIS_VBOOT);
	switch ((vboot & PIXIS_VBOOT_LBMAP) >> 6) {
		case PIXIS_VBOOT_LBMAP_NOR0:
			puts ("vBank: 0\n");
			break;
		case PIXIS_VBOOT_LBMAP_PJET:
			puts ("Promjet\n");
			break;
		case PIXIS_VBOOT_LBMAP_NAND:
			puts ("NAND\n");
			break;
		case PIXIS_VBOOT_LBMAP_NOR1:
			puts ("vBank: 1\n");
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

#ifdef CONFIG_PCI
void pci_init_board(void)
{
	struct pci_controller *hose;

	fsl_pcie_init_board(0);

	hose = find_hose_by_cfg_addr((void *)(CONFIG_SYS_PCIE3_ADDR));

	if (hose) {
		u32 temp32;
		u8 uli_busno = hose->first_busno + 2;

		/*
		 * Activate ULI1575 legacy chip by performing a fake
		 * memory access.  Needed to make ULI RTC work.
		 * Device 1d has the first on-board memory BAR.
		 */
		pci_hose_read_config_dword(hose, PCI_BDF(uli_busno, 0x1d, 0),
				PCI_BASE_ADDRESS_1, &temp32);

		if (temp32 >= CONFIG_SYS_PCIE3_MEM_BUS) {
			void *p = pci_mem_to_virt(PCI_BDF(uli_busno, 0x1d, 0),
					temp32, 4, 0);
			debug(" uli1572 read to %p\n", p);
			in_be32(p);
		}
	}
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
		flash_esel = 2; /* give our best effort to continue */
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
	struct tsec_info_struct tsec_info[4];
	int num = 0;

#ifdef CONFIG_TSEC1
	SET_STD_TSEC_INFO(tsec_info[num], 1);
	if (is_serdes_configured(SGMII_TSEC1)) {
		puts("eTSEC1 is in sgmii mode.\n");
		tsec_info[num].flags |= TSEC_SGMII;
	}
	num++;
#endif
#ifdef CONFIG_TSEC2
	SET_STD_TSEC_INFO(tsec_info[num], 2);
	if (is_serdes_configured(SGMII_TSEC2)) {
		puts("eTSEC2 is in sgmii mode.\n");
		tsec_info[num].flags |= TSEC_SGMII;
	}
	num++;
#endif
#ifdef CONFIG_TSEC3
	SET_STD_TSEC_INFO(tsec_info[num], 3);
	if (is_serdes_configured(SGMII_TSEC3)) {
		puts("eTSEC3 is in sgmii mode.\n");
		tsec_info[num].flags |= TSEC_SGMII;
	}
	num++;
#endif
#ifdef CONFIG_TSEC4
	SET_STD_TSEC_INFO(tsec_info[num], 4);
	if (is_serdes_configured(SGMII_TSEC4)) {
		puts("eTSEC4 is in sgmii mode.\n");
		tsec_info[num].flags |= TSEC_SGMII;
	}
	num++;
#endif

	if (!num) {
		printf("No TSECs initialized\n");

		return 0;
	}

#ifdef CONFIG_FSL_SGMII_RISER
	fsl_sgmii_riser_init(tsec_info, num);
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
	phys_addr_t base;
	phys_size_t size;

	ft_cpu_setup(blob, bd);

	base = env_get_bootm_low();
	size = env_get_bootm_size();

	fdt_fixup_memory(blob, (u64)base, (u64)size);

	FT_FSL_PCI_SETUP;

#ifdef CONFIG_FSL_SGMII_RISER
	fsl_sgmii_riser_fdt_fixup(blob);
#endif

	return 0;
}
#endif
