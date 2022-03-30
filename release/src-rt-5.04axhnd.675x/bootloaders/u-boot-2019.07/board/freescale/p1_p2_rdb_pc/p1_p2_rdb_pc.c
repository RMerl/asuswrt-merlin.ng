// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2010-2011, 2013 Freescale Semiconductor, Inc.
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
#include <vsc7385.h>
#include <ioports.h>
#include <asm/fsl_serdes.h>
#include <netdev.h>

#ifdef CONFIG_QE

#define GPIO_GETH_SW_PORT	1
#define GPIO_GETH_SW_PIN	29
#define GPIO_GETH_SW_DATA	(1 << (31 - GPIO_GETH_SW_PIN))

#define GPIO_SLIC_PORT		1
#define GPIO_SLIC_PIN		30
#define GPIO_SLIC_DATA		(1 << (31 - GPIO_SLIC_PIN))

#if defined(CONFIG_TARGET_P1021RDB) && !defined(CONFIG_SYS_RAMBOOT)
#define GPIO_DDR_RST_PORT	1
#define GPIO_DDR_RST_PIN	8
#define GPIO_DDR_RST_DATA	(1 << (31 - GPIO_DDR_RST_PIN))

#define GPIO_2BIT_MASK		(0x3 << (32 - (GPIO_DDR_RST_PIN + 1) * 2))
#endif

#if defined(CONFIG_TARGET_P1025RDB) || defined(CONFIG_TARGET_P1021RDB)
#define PCA_IOPORT_I2C_ADDR		0x23
#define PCA_IOPORT_OUTPUT_CMD		0x2
#define PCA_IOPORT_CFG_CMD		0x6
#define PCA_IOPORT_QE_PIN_ENABLE	0xf8
#define PCA_IOPORT_QE_TDM_ENABLE	0xf6
#endif

const qe_iop_conf_t qe_iop_conf_tab[] = {
	/* GPIO */
	{1,   1, 2, 0, 0}, /* GPIO7/PB1   - LOAD_DEFAULT_N */
#if defined(CONFIG_TARGET_P1021RDB) && !defined(CONFIG_SYS_RAMBOOT)
	{1,   8, 1, 1, 0}, /* GPIO10/PB8  - DDR_RST */
#endif
	{0,  15, 1, 0, 0}, /* GPIO11/A15  - WDI */
	{GPIO_GETH_SW_PORT, GPIO_GETH_SW_PIN, 1, 0, 0},	/* RST_GETH_SW_N */
	{GPIO_SLIC_PORT, GPIO_SLIC_PIN, 1, 0, 0},	/* RST_SLIC_N */

#ifdef CONFIG_TARGET_P1025RDB
	/* QE_MUX_MDC */
	{1,  19, 1, 0, 1}, /* QE_MUX_MDC               */

	/* QE_MUX_MDIO */
	{1,  20, 3, 0, 1}, /* QE_MUX_MDIO              */

	/* UCC_1_MII */
	{0, 23, 2, 0, 2}, /* CLK12 */
	{0, 24, 2, 0, 1}, /* CLK9 */
	{0,  7, 1, 0, 2}, /* ENET1_TXD0_SER1_TXD0      */
	{0,  9, 1, 0, 2}, /* ENET1_TXD1_SER1_TXD1      */
	{0, 11, 1, 0, 2}, /* ENET1_TXD2_SER1_TXD2      */
	{0, 12, 1, 0, 2}, /* ENET1_TXD3_SER1_TXD3      */
	{0,  6, 2, 0, 2}, /* ENET1_RXD0_SER1_RXD0      */
	{0, 10, 2, 0, 2}, /* ENET1_RXD1_SER1_RXD1      */
	{0, 14, 2, 0, 2}, /* ENET1_RXD2_SER1_RXD2      */
	{0, 15, 2, 0, 2}, /* ENET1_RXD3_SER1_RXD3      */
	{0,  5, 1, 0, 2}, /* ENET1_TX_EN_SER1_RTS_B    */
	{0, 13, 1, 0, 2}, /* ENET1_TX_ER               */
	{0,  4, 2, 0, 2}, /* ENET1_RX_DV_SER1_CTS_B    */
	{0,  8, 2, 0, 2}, /* ENET1_RX_ER_SER1_CD_B    */
	{0, 17, 2, 0, 2}, /* ENET1_CRS    */
	{0, 16, 2, 0, 2}, /* ENET1_COL    */

	/* UCC_5_RMII */
	{1, 11, 2, 0, 1}, /* CLK13 */
	{1, 7,  1, 0, 2}, /* ENET5_TXD0_SER5_TXD0      */
	{1, 10, 1, 0, 2}, /* ENET5_TXD1_SER5_TXD1      */
	{1, 6, 2, 0, 2}, /* ENET5_RXD0_SER5_RXD0      */
	{1, 9, 2, 0, 2}, /* ENET5_RXD1_SER5_RXD1      */
	{1, 5, 1, 0, 2}, /* ENET5_TX_EN_SER5_RTS_B    */
	{1, 4, 2, 0, 2}, /* ENET5_RX_DV_SER5_CTS_B    */
	{1, 8, 2, 0, 2}, /* ENET5_RX_ER_SER5_CD_B    */
#endif

	{0,  0, 0, 0, QE_IOP_TAB_END} /* END of table */
};
#endif

struct cpld_data {
	u8 cpld_rev_major;
	u8 pcba_rev;
	u8 wd_cfg;
	u8 rst_bps_sw;
	u8 load_default_n;
	u8 rst_bps_wd;
	u8 bypass_enable;
	u8 bps_led;
	u8 status_led;                  /* offset: 0x8 */
	u8 fxo_led;                     /* offset: 0x9 */
	u8 fxs_led;                     /* offset: 0xa */
	u8 rev4[2];
	u8 system_rst;                  /* offset: 0xd */
	u8 bps_out;
	u8 rev5[3];
	u8 cpld_rev_minor;
};

#define CPLD_WD_CFG	0x03
#define CPLD_RST_BSW	0x00
#define CPLD_RST_BWD	0x00
#define CPLD_BYPASS_EN	0x03
#define CPLD_STATUS_LED	0x01
#define CPLD_FXO_LED	0x01
#define CPLD_FXS_LED	0x0F
#define CPLD_SYS_RST	0x00

void board_cpld_init(void)
{
	struct cpld_data *cpld_data = (void *)(CONFIG_SYS_CPLD_BASE);

	out_8(&cpld_data->wd_cfg, CPLD_WD_CFG);
	out_8(&cpld_data->status_led, CPLD_STATUS_LED);
	out_8(&cpld_data->fxo_led, CPLD_FXO_LED);
	out_8(&cpld_data->fxs_led, CPLD_FXS_LED);
	out_8(&cpld_data->system_rst, CPLD_SYS_RST);
}

void board_gpio_init(void)
{
#ifdef CONFIG_QE
	ccsr_gur_t *gur = (void *)(CONFIG_SYS_MPC85xx_GUTS_ADDR);
	par_io_t *par_io = (par_io_t *) &(gur->qe_par_io);

#if defined(CONFIG_TARGET_P1021RDB) && !defined(CONFIG_SYS_RAMBOOT)
	/* reset DDR3 */
	setbits_be32(&par_io[GPIO_DDR_RST_PORT].cpdat, GPIO_DDR_RST_DATA);
	udelay(1000);
	clrbits_be32(&par_io[GPIO_DDR_RST_PORT].cpdat, GPIO_DDR_RST_DATA);
	udelay(1000);
	setbits_be32(&par_io[GPIO_DDR_RST_PORT].cpdat, GPIO_DDR_RST_DATA);
	/* disable CE_PB8 */
	clrbits_be32(&par_io[GPIO_DDR_RST_PORT].cpdir1, GPIO_2BIT_MASK);
#endif
	/* Enable VSC7385 switch */
	setbits_be32(&par_io[GPIO_GETH_SW_PORT].cpdat, GPIO_GETH_SW_DATA);

	/* Enable SLIC */
	setbits_be32(&par_io[GPIO_SLIC_PORT].cpdat, GPIO_SLIC_DATA);
#else

	ccsr_gpio_t *pgpio = (void *)(CONFIG_SYS_MPC85xx_GPIO_ADDR);

	/*
	 * GPIO10 DDR Reset, open drain
	 * GPIO7  LOAD_DEFAULT_N          Input
	 * GPIO11  WDI (watchdog input)
	 * GPIO12  Ethernet Switch Reset
	 * GPIO13  SLIC Reset
	 */

	setbits_be32(&pgpio->gpdir, 0x02130000);
#if !defined(CONFIG_SYS_RAMBOOT) && !defined(CONFIG_SPL)
	/* init DDR3 reset signal */
	setbits_be32(&pgpio->gpdir, 0x00200000);
	setbits_be32(&pgpio->gpodr, 0x00200000);
	clrbits_be32(&pgpio->gpdat, 0x00200000);
	udelay(1000);
	setbits_be32(&pgpio->gpdat, 0x00200000);
	udelay(1000);
	clrbits_be32(&pgpio->gpdir, 0x00200000);
#endif

#ifdef CONFIG_VSC7385_ENET
	/* reset VSC7385 Switch */
	setbits_be32(&pgpio->gpdir, 0x00080000);
	setbits_be32(&pgpio->gpdat, 0x00080000);
#endif

#ifdef CONFIG_SLIC
	/* reset SLIC */
	setbits_be32(&pgpio->gpdir, 0x00040000);
	setbits_be32(&pgpio->gpdat, 0x00040000);
#endif
#endif
}

int board_early_init_f(void)
{
	ccsr_gur_t *gur = (void *)(CONFIG_SYS_MPC85xx_GUTS_ADDR);

	setbits_be32(&gur->pmuxcr,
			(MPC85xx_PMUXCR_SDHC_CD | MPC85xx_PMUXCR_SDHC_WP));
	clrbits_be32(&gur->sdhcdcr, SDHCDCR_CD_INV);

	clrbits_be32(&gur->pmuxcr, MPC85xx_PMUXCR_SD_DATA);
	setbits_be32(&gur->pmuxcr, MPC85xx_PMUXCR_TDM_ENA);

	board_gpio_init();
	board_cpld_init();

	return 0;
}

int checkboard(void)
{
	struct cpld_data *cpld_data = (void *)(CONFIG_SYS_CPLD_BASE);
	ccsr_gur_t *gur = (void *)(CONFIG_SYS_MPC85xx_GUTS_ADDR);
	u8 in, out, io_config, val;

	printf("Board: %s CPLD: V%d.%d PCBA: V%d.0\n", CONFIG_BOARDNAME,
		in_8(&cpld_data->cpld_rev_major) & 0x0F,
		in_8(&cpld_data->cpld_rev_minor) & 0x0F,
		in_8(&cpld_data->pcba_rev) & 0x0F);

	/* Initialize i2c early for rom_loc and flash bank information */
	i2c_set_bus_num(CONFIG_SYS_SPD_BUS_NUM);

	if (i2c_read(CONFIG_SYS_I2C_PCA9557_ADDR, 0, 1, &in, 1) < 0 ||
	    i2c_read(CONFIG_SYS_I2C_PCA9557_ADDR, 1, 1, &out, 1) < 0 ||
	    i2c_read(CONFIG_SYS_I2C_PCA9557_ADDR, 3, 1, &io_config, 1) < 0) {
		printf("Error reading i2c boot information!\n");
		return 0; /* Don't want to hang() on this error */
	}

	val = (in & io_config) | (out & (~io_config));

	puts("rom_loc: ");
	if ((val & (~__SW_BOOT_MASK)) == __SW_BOOT_SD) {
		puts("sd");
#ifdef __SW_BOOT_SPI
	} else if ((val & (~__SW_BOOT_MASK)) == __SW_BOOT_SPI) {
		puts("spi");
#endif
#ifdef __SW_BOOT_NAND
	} else if ((val & (~__SW_BOOT_MASK)) == __SW_BOOT_NAND) {
		puts("nand");
#endif
#ifdef __SW_BOOT_PCIE
	} else if ((val & (~__SW_BOOT_MASK)) == __SW_BOOT_PCIE) {
		puts("pcie");
#endif
	} else {
		if (val & 0x2)
			puts("nor lower bank");
		else
			puts("nor upper bank");
	}
	puts("\n");

	if (val & 0x1) {
		setbits_be32(&gur->pmuxcr, MPC85xx_PMUXCR_SD_DATA);
		puts("SD/MMC : 8-bit Mode\n");
		puts("eSPI : Disabled\n");
	} else {
		puts("SD/MMC : 4-bit Mode\n");
		puts("eSPI : Enabled\n");
	}

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
		MAS3_SX|MAS3_SW|MAS3_SR, MAS2_I|MAS2_G,/* perms, wimge */
		0, flash_esel, BOOKE_PAGESZ_64M, 1);/* ts, esel, tsize, iprot */
	return 0;
}

int board_eth_init(bd_t *bis)
{
	struct fsl_pq_mdio_info mdio_info;
	struct tsec_info_struct tsec_info[4];
	ccsr_gur_t *gur __attribute__((unused)) =
		(void *)(CONFIG_SYS_MPC85xx_GUTS_ADDR);
	int num = 0;
#ifdef CONFIG_VSC7385_ENET
	char *tmp;
	unsigned int vscfw_addr;
#endif

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

#ifdef CONFIG_VSC7385_ENET
	/* If a VSC7385 microcode image is present, then upload it. */
	tmp = env_get("vscfw_addr");
	if (tmp) {
		vscfw_addr = simple_strtoul(tmp, NULL, 16);
		printf("uploading VSC7385 microcode from %x\n", vscfw_addr);
		if (vsc7385_upload_firmware((void *) vscfw_addr,
					CONFIG_VSC7385_IMAGE_SIZE))
			puts("Failure uploading VSC7385 microcode.\n");
	} else
		puts("No address specified for VSC7385 microcode.\n");
#endif

	mdio_info.regs = TSEC_GET_MDIO_REGS_BASE(1);
	mdio_info.name = DEFAULT_MII_NAME;

	fsl_pq_mdio_init(bis, &mdio_info);

	tsec_eth_init(bis, tsec_info, num);

#if defined(CONFIG_UEC_ETH)
	/*  QE0 and QE3 need to be exposed for UCC1 and UCC5 Eth mode */
	setbits_be32(&gur->pmuxcr, MPC85xx_PMUXCR_QE0);
	setbits_be32(&gur->pmuxcr, MPC85xx_PMUXCR_QE3);

	uec_standard_init(bis);
#endif

	return pci_eth_init(bis);
}

#if defined(CONFIG_QE) && \
	(defined(CONFIG_TARGET_P1025RDB) || defined(CONFIG_TARGET_P1021RDB))
static void fdt_board_fixup_qe_pins(void *blob)
{
	unsigned int oldbus;
	u8 val8;
	int node;
	fsl_lbc_t *lbc = LBC_BASE_ADDR;

	if (hwconfig("qe")) {
		/* For QE and eLBC pins multiplexing,
		 * there is a PCA9555 device on P1025RDB.
		 * It control the multiplex pins' functions,
		 * and setting the PCA9555 can switch the
		 * function between QE and eLBC.
		 */
		oldbus = i2c_get_bus_num();
		i2c_set_bus_num(0);
		if (hwconfig("tdm"))
			val8 = PCA_IOPORT_QE_TDM_ENABLE;
		else
			val8 = PCA_IOPORT_QE_PIN_ENABLE;
		i2c_write(PCA_IOPORT_I2C_ADDR, PCA_IOPORT_CFG_CMD,
				1, &val8, 1);
		i2c_write(PCA_IOPORT_I2C_ADDR, PCA_IOPORT_OUTPUT_CMD,
				1, &val8, 1);
		i2c_set_bus_num(oldbus);
		/* if run QE TDM, Set ABSWP to implement
		 * conversion of addresses in the eLBC.
		 */
		if (hwconfig("tdm")) {
			set_lbc_or(2, CONFIG_PMC_OR_PRELIM);
			set_lbc_br(2, CONFIG_PMC_BR_PRELIM);
			setbits_be32(&lbc->lbcr, CONFIG_SYS_LBC_LBCR);
		}
	} else {
		node = fdt_path_offset(blob, "/qe");
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
#if defined(CONFIG_TARGET_P1020RDB_PD) || defined(CONFIG_TARGET_P1020RDB_PC)
	const char *soc_usb_compat = "fsl-usb2-dr";
	int usb_err, usb1_off, usb2_off;
#endif
#if defined(CONFIG_SDCARD) || defined(CONFIG_SPIFLASH)
	int err;
#endif

	ft_cpu_setup(blob, bd);

	base = env_get_bootm_low();
	size = env_get_bootm_size();

	fdt_fixup_memory(blob, (u64)base, (u64)size);

	FT_FSL_PCI_SETUP;

#ifdef CONFIG_QE
	do_fixup_by_compat(blob, "fsl,qe", "status", "okay",
			sizeof("okay"), 0);
#if defined(CONFIG_TARGET_P1025RDB) || defined(CONFIG_TARGET_P1021RDB)
	fdt_board_fixup_qe_pins(blob);
#endif
#endif

#if defined(CONFIG_HAS_FSL_DR_USB)
	fsl_fdt_fixup_dr_usb(blob, bd);
#endif

#if defined(CONFIG_SDCARD) || defined(CONFIG_SPIFLASH)
	/* Delete eLBC node as it is muxed with USB2 controller */
	if (hwconfig("usb2")) {
		const char *soc_elbc_compat = "fsl,p1020-elbc";
		int off = fdt_node_offset_by_compatible(blob, -1,
				soc_elbc_compat);
		if (off < 0) {
			printf("WARNING: could not find compatible node %s\n",
			       soc_elbc_compat);
			return off;
		}
		err = fdt_del_node(blob, off);
		if (err < 0) {
			printf("WARNING: could not remove %s\n",
			       soc_elbc_compat);
			return err;
		}
		return 0;
	}
#endif

#if defined(CONFIG_TARGET_P1020RDB_PD) || defined(CONFIG_TARGET_P1020RDB_PC)
/* Delete USB2 node as it is muxed with eLBC */
	usb1_off = fdt_node_offset_by_compatible(blob, -1,
		soc_usb_compat);
	if (usb1_off < 0) {
		printf("WARNING: could not find compatible node %s\n",
		       soc_usb_compat);
		return usb1_off;
	}
	usb2_off = fdt_node_offset_by_compatible(blob, usb1_off,
			soc_usb_compat);
	if (usb2_off < 0) {
		printf("WARNING: could not find compatible node %s\n",
		       soc_usb_compat);
		return usb2_off;
	}
	usb_err = fdt_del_node(blob, usb2_off);
	if (usb_err < 0) {
		printf("WARNING: could not remove %s\n", soc_usb_compat);
		return usb_err;
	}
#endif

	return 0;
}
#endif
