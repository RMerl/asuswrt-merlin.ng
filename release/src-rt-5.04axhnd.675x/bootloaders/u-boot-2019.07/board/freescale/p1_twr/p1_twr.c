// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2013 Freescale Semiconductor, Inc.
 */

#include <common.h>
#include <command.h>
#include <hwconfig.h>
#include <pci.h>
#include <i2c.h>
#include <asm/processor.h>
#include <asm/mmu.h>
#include <asm/cache.h>
#include <asm/immap_85xx.h>
#include <asm/fsl_pci.h>
#include <fsl_ddr_sdram.h>
#include <asm/io.h>
#include <asm/fsl_law.h>
#include <asm/fsl_lbc.h>
#include <asm/mp.h>
#include <miiphy.h>
#include <linux/libfdt.h>
#include <fdt_support.h>
#include <fsl_mdio.h>
#include <tsec.h>
#include <ioports.h>
#include <asm/fsl_serdes.h>
#include <netdev.h>

#define SYSCLK_64	64000000
#define SYSCLK_66	66666666

unsigned long get_board_sys_clk(ulong dummy)
{
	ccsr_gur_t *gur = (void __iomem *)(CONFIG_SYS_MPC85xx_GUTS_ADDR);
	par_io_t *par_io = (par_io_t *) &(gur->qe_par_io);
	unsigned int cpdat_val = 0;

	/* Set-up up pin muxing based on board switch settings */
	cpdat_val = par_io[1].cpdat;

	/* Check switch setting for SYSCLK select (PB3)  */
	if (cpdat_val & 0x10000000)
		return SYSCLK_64;
	else
		return SYSCLK_66;

	return 0;
}

#ifdef CONFIG_QE

#define PCA_IOPORT_I2C_ADDR		0x23
#define PCA_IOPORT_OUTPUT_CMD		0x2
#define PCA_IOPORT_CFG_CMD		0x6

const qe_iop_conf_t qe_iop_conf_tab[] = {

#ifdef CONFIG_TWR_P1025
	/* GPIO */
	{1,  0, 1, 0, 0},
	{1,  18, 1, 0, 0},

	/* GPIO for switch options */
	{1,  2, 2, 0, 0}, /* PROFIBUS_MODE_SEL */
	{1,  3, 2, 0, 0}, /* SYS_CLK_SELECT */
	{1,  29, 2, 0, 0}, /* LOCALBUS_QE_MUXSEL */
	{1,  30, 2, 0, 0}, /* ETH_TDM_SEL */

	/* QE_MUX_MDC */
	{1,  19, 1, 0, 1}, /* QE_MUX_MDC */

	/* QE_MUX_MDIO */
	{1,  20, 3, 0, 1}, /* QE_MUX_MDIO */

	/* UCC_1_MII */
	{0, 23, 2, 0, 2}, /* CLK12 */
	{0, 24, 2, 0, 1}, /* CLK9 */
	{0,  7, 1, 0, 2}, /* ENET1_TXD0_SER1_TXD0 */
	{0,  9, 1, 0, 2}, /* ENET1_TXD1_SER1_TXD1 */
	{0, 11, 1, 0, 2}, /* ENET1_TXD2_SER1_TXD2 */
	{0, 12, 1, 0, 2}, /* ENET1_TXD3_SER1_TXD3 */
	{0,  6, 2, 0, 2}, /* ENET1_RXD0_SER1_RXD0 */
	{0, 10, 2, 0, 2}, /* ENET1_RXD1_SER1_RXD1 */
	{0, 14, 2, 0, 2}, /* ENET1_RXD2_SER1_RXD2 */
	{0, 15, 2, 0, 2}, /* ENET1_RXD3_SER1_RXD3 */
	{0,  5, 1, 0, 2}, /* ENET1_TX_EN_SER1_RTS_B */
	{0, 13, 1, 0, 2}, /* ENET1_TX_ER */
	{0,  4, 2, 0, 2}, /* ENET1_RX_DV_SER1_CTS_B */
	{0,  8, 2, 0, 2}, /* ENET1_RX_ER_SER1_CD_B */
	{0, 17, 2, 0, 2}, /* ENET1_CRS */
	{0, 16, 2, 0, 2}, /* ENET1_COL */

	/* UCC_5_RMII */
	{1, 11, 2, 0, 1}, /* CLK13 */
	{1, 7,  1, 0, 2}, /* ENET5_TXD0_SER5_TXD0 */
	{1, 10, 1, 0, 2}, /* ENET5_TXD1_SER5_TXD1 */
	{1, 6, 2, 0, 2}, /* ENET5_RXD0_SER5_RXD0 */
	{1, 9, 2, 0, 2}, /* ENET5_RXD1_SER5_RXD1 */
	{1, 5, 1, 0, 2}, /* ENET5_TX_EN_SER5_RTS_B */
	{1, 4, 2, 0, 2}, /* ENET5_RX_DV_SER5_CTS_B */
	{1, 8, 2, 0, 2}, /* ENET5_RX_ER_SER5_CD_B */

	/* TDMA - clock option is configured in OS based on board setting */
	{1, 23, 2, 0, 2}, /* TDMA_TXD */
	{1, 25, 2, 0, 2}, /* TDMA_RXD */
	{1, 26, 1, 0, 2}, /* TDMA_SYNC */
#endif

	{0,  0, 0, 0, QE_IOP_TAB_END} /* END of table */
};
#endif

int board_early_init_f(void)
{
	ccsr_gur_t *gur = (void __iomem *)(CONFIG_SYS_MPC85xx_GUTS_ADDR);

	setbits_be32(&gur->pmuxcr,
			(MPC85xx_PMUXCR_SDHC_CD | MPC85xx_PMUXCR_SDHC_WP));

	/* SDHC_DAT[4:7] not exposed to pins (use as SPI) */
	clrbits_be32(&gur->pmuxcr, MPC85xx_PMUXCR_SD_DATA);

	return 0;
}

int checkboard(void)
{
	ccsr_gur_t *gur = (void __iomem *)(CONFIG_SYS_MPC85xx_GUTS_ADDR);
	u8 boot_status;

	printf("Board: %s\n", CONFIG_BOARDNAME);

	boot_status = ((gur->porbmsr) >> MPC85xx_PORBMSR_ROMLOC_SHIFT) & 0xf;
	puts("rom_loc: ");
	if (boot_status == PORBMSR_ROMLOC_NOR)
		puts("nor flash");
	else if (boot_status == PORBMSR_ROMLOC_SDHC)
		puts("sd");
	else
		puts("unknown");
	puts("\n");

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
	 * Remap Boot flash region to caching-inhibited
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
		/* invalidate existing TLB entry for flash */
		disable_tlb(flash_esel);
	}

	set_tlb(1, flashbase, CONFIG_SYS_FLASH_BASE_PHYS, /* tlb, epn, rpn */
		MAS3_SW|MAS3_SR, MAS2_I|MAS2_G,           /* perms, wimge */
		0, flash_esel, BOOKE_PAGESZ_64M, 1);/* ts, esel, tsize, iprot */
	return 0;
}

int board_eth_init(bd_t *bis)
{
	struct fsl_pq_mdio_info mdio_info;
	struct tsec_info_struct tsec_info[4];
	ccsr_gur_t *gur __attribute__((unused)) =
		(void __iomem *)(CONFIG_SYS_MPC85xx_GUTS_ADDR);
	int num = 0;

#ifdef CONFIG_TSEC1
	SET_STD_TSEC_INFO(tsec_info[num], 1);
	num++;
#endif
#ifdef CONFIG_TSEC2
	SET_STD_TSEC_INFO(tsec_info[num], 2);
	if (is_serdes_configured(SGMII_TSEC2)) {
		printf("eTSEC2 is in sgmii mode.\n");
		tsec_info[num].flags |= TSEC_SGMII;
	}
	num++;
#endif
#ifdef CONFIG_TSEC3
	SET_STD_TSEC_INFO(tsec_info[num], 3);
	num++;
#endif

	if (!num) {
		printf("No TSECs initialized\n");
		return 0;
	}

	mdio_info.regs = (struct tsec_mii_mng *)CONFIG_SYS_MDIO_BASE_ADDR;
	mdio_info.name = DEFAULT_MII_NAME;

	fsl_pq_mdio_init(bis, &mdio_info);

	tsec_eth_init(bis, tsec_info, num);

#if defined(CONFIG_UEC_ETH)
	/* QE0 and QE3 need to be exposed for UCC1
	 * and UCC5 Eth mode (in PMUXCR register).
	 * Currently QE/LBC muxed pins assumed to be
	 * LBC for U-Boot and PMUXCR updated by OS if required */

	uec_standard_init(bis);
#endif

	return pci_eth_init(bis);
}

#if defined(CONFIG_QE)
static void fdt_board_fixup_qe_pins(void *blob)
{
	int node;

	if (!hwconfig("qe")) {
		/* For QE and eLBC pins multiplexing,
		 * When don't use QE function, remove
		 * qe node from dt blob.
		 */
		node = fdt_path_offset(blob, "/qe");
		if (node >= 0)
			fdt_del_node(blob, node);
	} else {
		/* For TWR Peripheral Modules - TWR-SER2
		 * board only can support Signal Port MII,
		 * so delete one UEC node when use MII port.
		 */
		if (hwconfig("mii"))
			node = fdt_path_offset(blob, "/qe/ucc@2400");
		else
			node = fdt_path_offset(blob, "/qe/ucc@2000");
		if (node >= 0)
			fdt_del_node(blob, node);
	}

	return;
}
#endif

#ifdef CONFIG_OF_BOARD_SETUP
int ft_board_setup(void *blob, bd_t *bd)
{
	phys_addr_t base;
	phys_size_t size;

	ft_cpu_setup(blob, bd);

	base = env_get_bootm_low();
	size = env_get_bootm_size();

	fdt_fixup_memory(blob, (u64)base, (u64)size);

	FT_FSL_PCI_SETUP;

#ifdef CONFIG_QE
	do_fixup_by_compat(blob, "fsl,qe", "status", "okay",
			sizeof("okay"), 0);
#endif
#if defined(CONFIG_TWR_P1025)
	fdt_board_fixup_qe_pins(blob);
#endif
	fsl_fdt_fixup_dr_usb(blob, bd);

	return 0;
}
#endif
