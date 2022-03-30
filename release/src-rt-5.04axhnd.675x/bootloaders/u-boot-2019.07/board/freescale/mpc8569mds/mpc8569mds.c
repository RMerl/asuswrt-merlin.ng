// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2009-2010 Freescale Semiconductor.
 *
 * (C) Copyright 2002 Scott McNutt <smcnutt@artesyncp.com>
 */

#include <common.h>
#include <console.h>
#include <hwconfig.h>
#include <pci.h>
#include <asm/processor.h>
#include <asm/mmu.h>
#include <asm/cache.h>
#include <asm/immap_85xx.h>
#include <asm/fsl_pci.h>
#include <fsl_ddr_sdram.h>
#include <asm/fsl_serdes.h>
#include <asm/io.h>
#include <spd_sdram.h>
#include <i2c.h>
#include <ioports.h>
#include <linux/libfdt.h>
#include <fdt_support.h>
#include <fsl_esdhc.h>
#include <phy.h>

#include "bcsr.h"
#if defined(CONFIG_PQ_MDS_PIB)
#include "../common/pq-mds-pib.h"
#endif

const qe_iop_conf_t qe_iop_conf_tab[] = {
	/* QE_MUX_MDC */
	{2,  31, 1, 0, 1}, /* QE_MUX_MDC               */

	/* QE_MUX_MDIO */
	{2,  30, 3, 0, 2}, /* QE_MUX_MDIO              */

#if defined(CONFIG_SYS_UCC_RGMII_MODE)
	/* UCC_1_RGMII */
	{2, 11, 2, 0, 1}, /* CLK12 */
	{0,  0, 1, 0, 3}, /* ENET1_TXD0_SER1_TXD0      */
	{0,  1, 1, 0, 3}, /* ENET1_TXD1_SER1_TXD1      */
	{0,  2, 1, 0, 1}, /* ENET1_TXD2_SER1_TXD2      */
	{0,  3, 1, 0, 2}, /* ENET1_TXD3_SER1_TXD3      */
	{0,  6, 2, 0, 3}, /* ENET1_RXD0_SER1_RXD0      */
	{0,  7, 2, 0, 1}, /* ENET1_RXD1_SER1_RXD1      */
	{0,  8, 2, 0, 2}, /* ENET1_RXD2_SER1_RXD2      */
	{0,  9, 2, 0, 2}, /* ENET1_RXD3_SER1_RXD3      */
	{0,  4, 1, 0, 2}, /* ENET1_TX_EN_SER1_RTS_B    */
	{0, 12, 2, 0, 3}, /* ENET1_RX_DV_SER1_CTS_B    */
	{2,  8, 2, 0, 1}, /* ENET1_GRXCLK              */
	{2, 20, 1, 0, 2}, /* ENET1_GTXCLK              */

	/* UCC_2_RGMII */
	{2, 16, 2, 0, 3}, /* CLK17 */
	{0, 14, 1, 0, 2}, /* ENET2_TXD0_SER2_TXD0      */
	{0, 15, 1, 0, 2}, /* ENET2_TXD1_SER2_TXD1      */
	{0, 16, 1, 0, 1}, /* ENET2_TXD2_SER2_TXD2      */
	{0, 17, 1, 0, 1}, /* ENET2_TXD3_SER2_TXD3      */
	{0, 20, 2, 0, 2}, /* ENET2_RXD0_SER2_RXD0      */
	{0, 21, 2, 0, 1}, /* ENET2_RXD1_SER2_RXD1      */
	{0, 22, 2, 0, 1}, /* ENET2_RXD2_SER2_RXD2      */
	{0, 23, 2, 0, 1}, /* ENET2_RXD3_SER2_RXD3      */
	{0, 18, 1, 0, 2}, /* ENET2_TX_EN_SER2_RTS_B    */
	{0, 26, 2, 0, 3}, /* ENET2_RX_DV_SER2_CTS_B    */
	{2,  3, 2, 0, 1}, /* ENET2_GRXCLK              */
	{2,  2, 1, 0, 2}, /* ENET2_GTXCLK              */

	/* UCC_3_RGMII */
	{2, 11, 2, 0, 1}, /* CLK12 */
	{0, 29, 1, 0, 2}, /* ENET3_TXD0_SER3_TXD0      */
	{0, 30, 1, 0, 3}, /* ENET3_TXD1_SER3_TXD1      */
	{0, 31, 1, 0, 2}, /* ENET3_TXD2_SER3_TXD2      */
	{1,  0, 1, 0, 3}, /* ENET3_TXD3_SER3_TXD3      */
	{1,  3, 2, 0, 3}, /* ENET3_RXD0_SER3_RXD0      */
	{1,  4, 2, 0, 1}, /* ENET3_RXD1_SER3_RXD1      */
	{1,  5, 2, 0, 2}, /* ENET3_RXD2_SER3_RXD2      */
	{1,  6, 2, 0, 3}, /* ENET3_RXD3_SER3_RXD3      */
	{1,  1, 1, 0, 1}, /* ENET3_TX_EN_SER3_RTS_B    */
	{1,  9, 2, 0, 3}, /* ENET3_RX_DV_SER3_CTS_B    */
	{2,  9, 2, 0, 2}, /* ENET3_GRXCLK              */
	{2, 25, 1, 0, 2}, /* ENET3_GTXCLK              */

	/* UCC_4_RGMII */
	{2, 16, 2, 0, 3}, /* CLK17 */
	{1, 12, 1, 0, 2}, /* ENET4_TXD0_SER4_TXD0      */
	{1, 13, 1, 0, 2}, /* ENET4_TXD1_SER4_TXD1      */
	{1, 14, 1, 0, 1}, /* ENET4_TXD2_SER4_TXD2      */
	{1, 15, 1, 0, 2}, /* ENET4_TXD3_SER4_TXD3      */
	{1, 18, 2, 0, 2}, /* ENET4_RXD0_SER4_RXD0      */
	{1, 19, 2, 0, 1}, /* ENET4_RXD1_SER4_RXD1      */
	{1, 20, 2, 0, 1}, /* ENET4_RXD2_SER4_RXD2      */
	{1, 21, 2, 0, 2}, /* ENET4_RXD3_SER4_RXD3      */
	{1, 16, 1, 0, 2}, /* ENET4_TX_EN_SER4_RTS_B    */
	{1, 24, 2, 0, 3}, /* ENET4_RX_DV_SER4_CTS_B    */
	{2, 17, 2, 0, 2}, /* ENET4_GRXCLK              */
	{2, 24, 1, 0, 2}, /* ENET4_GTXCLK              */

#elif defined(CONFIG_SYS_UCC_RMII_MODE)
	/* UCC_1_RMII */
	{2, 15, 2, 0, 1}, /* CLK16 */
	{0,  0, 1, 0, 3}, /* ENET1_TXD0_SER1_TXD0      */
	{0,  1, 1, 0, 3}, /* ENET1_TXD1_SER1_TXD1      */
	{0,  6, 2, 0, 3}, /* ENET1_RXD0_SER1_RXD0      */
	{0,  7, 2, 0, 1}, /* ENET1_RXD1_SER1_RXD1      */
	{0,  4, 1, 0, 2}, /* ENET1_TX_EN_SER1_RTS_B    */
	{0, 12, 2, 0, 3}, /* ENET1_RX_DV_SER1_CTS_B    */

	/* UCC_2_RMII */
	{2, 15, 2, 0, 1}, /* CLK16 */
	{0, 14, 1, 0, 2}, /* ENET2_TXD0_SER2_TXD0      */
	{0, 15, 1, 0, 2}, /* ENET2_TXD1_SER2_TXD1      */
	{0, 20, 2, 0, 2}, /* ENET2_RXD0_SER2_RXD0      */
	{0, 21, 2, 0, 1}, /* ENET2_RXD1_SER2_RXD1      */
	{0, 18, 1, 0, 2}, /* ENET2_TX_EN_SER2_RTS_B    */
	{0, 26, 2, 0, 3}, /* ENET2_RX_DV_SER2_CTS_B    */

	/* UCC_3_RMII */
	{2, 15, 2, 0, 1}, /* CLK16 */
	{0, 29, 1, 0, 2}, /* ENET3_TXD0_SER3_TXD0      */
	{0, 30, 1, 0, 3}, /* ENET3_TXD1_SER3_TXD1      */
	{1,  3, 2, 0, 3}, /* ENET3_RXD0_SER3_RXD0      */
	{1,  4, 2, 0, 1}, /* ENET3_RXD1_SER3_RXD1      */
	{1,  1, 1, 0, 1}, /* ENET3_TX_EN_SER3_RTS_B    */
	{1,  9, 2, 0, 3}, /* ENET3_RX_DV_SER3_CTS_B    */

	/* UCC_4_RMII */
	{2, 15, 2, 0, 1}, /* CLK16 */
	{1, 12, 1, 0, 2}, /* ENET4_TXD0_SER4_TXD0      */
	{1, 13, 1, 0, 2}, /* ENET4_TXD1_SER4_TXD1      */
	{1, 18, 2, 0, 2}, /* ENET4_RXD0_SER4_RXD0      */
	{1, 19, 2, 0, 1}, /* ENET4_RXD1_SER4_RXD1      */
	{1, 16, 1, 0, 2}, /* ENET4_TX_EN_SER4_RTS_B    */
	{1, 24, 2, 0, 3}, /* ENET4_RX_DV_SER4_CTS_B    */
#endif

	/* UART1 is muxed with QE PortF bit [9-12].*/
	{5, 12, 2, 0, 3}, /* UART1_SIN */
	{5, 9,  1, 0, 3}, /* UART1_SOUT */
	{5, 10, 2, 0, 3}, /* UART1_CTS_B */
	{5, 11, 1, 0, 2}, /* UART1_RTS_B */

	/* QE UART                                     */
	{0, 19, 1, 0, 2}, /* QEUART_TX                 */
	{1, 17, 2, 0, 3}, /* QEUART_RX                 */
	{0, 25, 1, 0, 1}, /* QEUART_RTS                */
	{1, 23, 2, 0, 1}, /* QEUART_CTS                */

	/* QE USB                                      */
	{5,  3, 1, 0, 1}, /* USB_OE                    */
	{5,  4, 1, 0, 2}, /* USB_TP                    */
	{5,  5, 1, 0, 2}, /* USB_TN                    */
	{5,  6, 2, 0, 2}, /* USB_RP                    */
	{5,  7, 2, 0, 1}, /* USB_RX                    */
	{5,  8, 2, 0, 1}, /* USB_RN                    */
	{2,  4, 2, 0, 2}, /* CLK5                      */

	/* SPI Flash, M25P40                           */
	{4, 27, 3, 0, 1}, /* SPI_MOSI                  */
	{4, 28, 3, 0, 1}, /* SPI_MISO                  */
	{4, 29, 3, 0, 1}, /* SPI_CLK                   */
	{4, 30, 1, 0, 0}, /* SPI_SEL, GPIO             */

	{0,  0, 0, 0, QE_IOP_TAB_END} /* END of table */
};

void local_bus_init(void);

int board_early_init_f (void)
{
	/*
	 * Initialize local bus.
	 */
	local_bus_init ();

	enable_8569mds_flash_write();

#ifdef CONFIG_QE
	enable_8569mds_qe_uec();
#endif

#if CONFIG_SYS_I2C2_OFFSET
	/* Enable I2C2 signals instead of SD signals */
	volatile struct ccsr_gur *gur;
	gur = (struct ccsr_gur *)(CONFIG_SYS_IMMR + 0xe0000);
	gur->plppar1 &= ~PLPPAR1_I2C_BIT_MASK;
	gur->plppar1 |= PLPPAR1_I2C2_VAL;
	gur->plpdir1 &= ~PLPDIR1_I2C_BIT_MASK;
	gur->plpdir1 |= PLPDIR1_I2C2_VAL;

	disable_8569mds_brd_eeprom_write_protect();
#endif

	return 0;
}

int board_early_init_r(void)
{
	const unsigned int flashbase = CONFIG_SYS_NAND_BASE;
	const u8 flash_esel = 0;

	/*
	 * Remap Boot flash to caching-inhibited
	 * so that flash can be erased properly.
	 */

	/* Flush d-cache and invalidate i-cache of any FLASH data */
	flush_dcache();
	invalidate_icache();

	/* invalidate existing TLB entry for flash */
	disable_tlb(flash_esel);

	set_tlb(1, flashbase, CONFIG_SYS_NAND_BASE,	/* tlb, epn, rpn */
		MAS3_SX|MAS3_SW|MAS3_SR, MAS2_I|MAS2_G,	/* perms, wimge */
		0, flash_esel,				/* ts, esel */
		BOOKE_PAGESZ_64M, 1);			/* tsize, iprot */

	return 0;
}

int checkboard (void)
{
	printf ("Board: 8569 MDS\n");

	return 0;
}

#if !defined(CONFIG_SPD_EEPROM)
phys_size_t fixed_sdram(void)
{
	struct ccsr_ddr __iomem *ddr =
		(struct ccsr_ddr __iomem *)CONFIG_SYS_FSL_DDR_ADDR;
	uint d_init;

	out_be32(&ddr->cs0_bnds, CONFIG_SYS_DDR_CS0_BNDS);
	out_be32(&ddr->cs0_config, CONFIG_SYS_DDR_CS0_CONFIG);
	out_be32(&ddr->timing_cfg_3, CONFIG_SYS_DDR_TIMING_3);
	out_be32(&ddr->timing_cfg_0, CONFIG_SYS_DDR_TIMING_0);
	out_be32(&ddr->timing_cfg_1, CONFIG_SYS_DDR_TIMING_1);
	out_be32(&ddr->timing_cfg_2, CONFIG_SYS_DDR_TIMING_2);
	out_be32(&ddr->sdram_cfg, CONFIG_SYS_DDR_SDRAM_CFG);
	out_be32(&ddr->sdram_cfg_2, CONFIG_SYS_DDR_SDRAM_CFG_2);
	out_be32(&ddr->sdram_mode, CONFIG_SYS_DDR_SDRAM_MODE);
	out_be32(&ddr->sdram_mode_2, CONFIG_SYS_DDR_SDRAM_MODE_2);
	out_be32(&ddr->sdram_interval, CONFIG_SYS_DDR_SDRAM_INTERVAL);
	out_be32(&ddr->sdram_data_init, CONFIG_SYS_DDR_DATA_INIT);
	out_be32(&ddr->sdram_clk_cntl, CONFIG_SYS_DDR_SDRAM_CLK_CNTL);
	out_be32(&ddr->timing_cfg_4, CONFIG_SYS_DDR_TIMING_4);
	out_be32(&ddr->timing_cfg_5, CONFIG_SYS_DDR_TIMING_5);
	out_be32(&ddr->ddr_zq_cntl, CONFIG_SYS_DDR_ZQ_CNTL);
	out_be32(&ddr->ddr_wrlvl_cntl, CONFIG_SYS_DDR_WRLVL_CNTL);
	out_be32(&ddr->sdram_cfg_2, CONFIG_SYS_DDR_SDRAM_CFG_2);
#if defined (CONFIG_DDR_ECC)
	out_be32(&ddr->err_int_en, CONFIG_SYS_DDR_ERR_INT_EN);
	out_be32(&ddr->err_disable, CONFIG_SYS_DDR_ERR_DIS);
	out_be32(&ddr->err_sbe, CONFIG_SYS_DDR_SBE);
#endif
	udelay(500);

	out_be32(&ddr->sdram_cfg, CONFIG_SYS_DDR_CONTROL);
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
	udelay(500);
#endif
	return CONFIG_SYS_SDRAM_SIZE * 1024 * 1024;
}
#endif

/*
 * Initialize Local Bus
 */
void
local_bus_init(void)
{
	volatile ccsr_gur_t *gur = (void *)(CONFIG_SYS_MPC85xx_GUTS_ADDR);
	volatile fsl_lbc_t *lbc = LBC_BASE_ADDR;

	uint clkdiv;
	sys_info_t sysinfo;

	get_sys_info(&sysinfo);
	clkdiv = (lbc->lcrr & LCRR_CLKDIV) * 2;

	out_be32(&gur->lbiuiplldcr1, 0x00078080);
	if (clkdiv == 16)
		out_be32(&gur->lbiuiplldcr0, 0x7c0f1bf0);
	else if (clkdiv == 8)
		out_be32(&gur->lbiuiplldcr0, 0x6c0f1bf0);
	else if (clkdiv == 4)
		out_be32(&gur->lbiuiplldcr0, 0x5c0f1bf0);

	out_be32(&lbc->lcrr, (u32)in_be32(&lbc->lcrr)| 0x00030000);
}

static void fdt_board_disable_serial(void *blob, bd_t *bd, const char *alias)
{
	const char *status = "disabled";
	int off;
	int err;

	off = fdt_path_offset(blob, alias);
	if (off < 0) {
		printf("WARNING: could not find %s alias: %s.\n", alias,
			fdt_strerror(off));
		return;
	}

	err = fdt_setprop(blob, off, "status", status, strlen(status) + 1);
	if (err) {
		printf("WARNING: could not set status for serial0: %s.\n",
			fdt_strerror(err));
		return;
	}
}

/*
 * Because of an erratum in prototype boards it is impossible to use eSDHC
 * without disabling UART0 (which makes it quite easy to 'brick' the board
 * by simply issung 'setenv hwconfig esdhc', and not able to interact with
 * U-Boot anylonger).
 *
 * So, but default we assume that the board is a prototype, which is a most
 * safe assumption. There is no way to determine board revision from a
 * register, so we use hwconfig.
 */

static int prototype_board(void)
{
	if (hwconfig_subarg("board", "rev", NULL))
		return hwconfig_subarg_cmp("board", "rev", "prototype");
	return 1;
}

static int esdhc_disables_uart0(void)
{
	return prototype_board() ||
	       hwconfig_subarg_cmp("esdhc", "mode", "4-bits");
}

static void fdt_board_fixup_qe_uart(void *blob, bd_t *bd)
{
	u8 *bcsr = (u8 *)CONFIG_SYS_BCSR_BASE;
	const char *devtype = "serial";
	const char *compat = "ucc_uart";
	const char *clk = "brg9";
	u32 portnum = 0;
	int off = -1;

	if (!hwconfig("qe_uart"))
		return;

	if (hwconfig("esdhc") && esdhc_disables_uart0()) {
		printf("QE UART: won't enable with esdhc.\n");
		return;
	}

	fdt_board_disable_serial(blob, bd, "serial1");

	while (1) {
		const u32 *idx;
		int len;

		off = fdt_node_offset_by_compatible(blob, off, "ucc_geth");
		if (off < 0) {
			printf("WARNING: unable to fixup device tree for "
				"QE UART\n");
			return;
		}

		idx = fdt_getprop(blob, off, "cell-index", &len);
		if (!idx || len != sizeof(*idx) || *idx != fdt32_to_cpu(2))
			continue;
		break;
	}

	fdt_setprop(blob, off, "device_type", devtype, strlen(devtype) + 1);
	fdt_setprop(blob, off, "compatible", compat, strlen(compat) + 1);
	fdt_setprop(blob, off, "tx-clock-name", clk, strlen(clk) + 1);
	fdt_setprop(blob, off, "rx-clock-name", clk, strlen(clk) + 1);
	fdt_setprop(blob, off, "port-number", &portnum, sizeof(portnum));

	setbits_8(&bcsr[15], BCSR15_QEUART_EN);
}

#ifdef CONFIG_FSL_ESDHC

int board_mmc_init(bd_t *bd)
{
	struct ccsr_gur *gur = (struct ccsr_gur *)CONFIG_SYS_MPC85xx_GUTS_ADDR;
	u8 *bcsr = (u8 *)CONFIG_SYS_BCSR_BASE;
	u8 bcsr6 = BCSR6_SD_CARD_1BIT;

	if (!hwconfig("esdhc"))
		return 0;

	printf("Enabling eSDHC...\n"
	       "  For eSDHC to function, I2C2 ");
	if (esdhc_disables_uart0()) {
		printf("and UART0 should be disabled.\n");
		printf("  Redirecting stderr, stdout and stdin to UART1...\n");
		console_assign(stderr, "eserial1");
		console_assign(stdout, "eserial1");
		console_assign(stdin, "eserial1");
		printf("Switched to UART1 (initial log has been printed to "
		       "UART0).\n");

		clrsetbits_be32(&gur->plppar1, PLPPAR1_UART0_BIT_MASK,
					       PLPPAR1_ESDHC_4BITS_VAL);
		clrsetbits_be32(&gur->plpdir1, PLPDIR1_UART0_BIT_MASK,
					       PLPDIR1_ESDHC_4BITS_VAL);
		bcsr6 |= BCSR6_SD_CARD_4BITS;
	} else {
		printf("should be disabled.\n");
	}

	/* Assign I2C2 signals to eSDHC. */
	clrsetbits_be32(&gur->plppar1, PLPPAR1_I2C_BIT_MASK,
				       PLPPAR1_ESDHC_VAL);
	clrsetbits_be32(&gur->plpdir1, PLPDIR1_I2C_BIT_MASK,
				       PLPDIR1_ESDHC_VAL);

	/* Mux I2C2 (and optionally UART0) signals to eSDHC. */
	setbits_8(&bcsr[6], bcsr6);

	return fsl_esdhc_mmc_init(bd);
}

static void fdt_board_fixup_esdhc(void *blob, bd_t *bd)
{
	const char *status = "disabled";
	int off = -1;

	if (!hwconfig("esdhc"))
		return;

	if (esdhc_disables_uart0())
		fdt_board_disable_serial(blob, bd, "serial0");

	while (1) {
		const u32 *idx;
		int len;

		off = fdt_node_offset_by_compatible(blob, off, "fsl-i2c");
		if (off < 0)
			break;

		idx = fdt_getprop(blob, off, "cell-index", &len);
		if (!idx || len != sizeof(*idx))
			continue;

		if (*idx == 1) {
			fdt_setprop(blob, off, "status", status,
				    strlen(status) + 1);
			break;
		}
	}

	if (hwconfig_subarg_cmp("esdhc", "mode", "4-bits")) {
		off = fdt_node_offset_by_compatible(blob, -1, "fsl,esdhc");
		if (off < 0) {
			printf("WARNING: could not find esdhc node\n");
			return;
		}
		fdt_delprop(blob, off, "sdhci,1-bit-only");
	}
}
#else
static inline void fdt_board_fixup_esdhc(void *blob, bd_t *bd) {}
#endif

static void fdt_board_fixup_qe_usb(void *blob, bd_t *bd)
{
	u8 *bcsr = (u8 *)CONFIG_SYS_BCSR_BASE;

	if (hwconfig_subarg_cmp("qe_usb", "speed", "low"))
		clrbits_8(&bcsr[17], BCSR17_nUSBLOWSPD);
	else
		setbits_8(&bcsr[17], BCSR17_nUSBLOWSPD);

	if (hwconfig_subarg_cmp("qe_usb", "mode", "peripheral")) {
		clrbits_8(&bcsr[17], BCSR17_USBVCC);
		clrbits_8(&bcsr[17], BCSR17_USBMODE);
		do_fixup_by_compat(blob, "fsl,mpc8569-qe-usb", "mode",
				   "peripheral", sizeof("peripheral"), 1);
	} else {
		setbits_8(&bcsr[17], BCSR17_USBVCC);
		setbits_8(&bcsr[17], BCSR17_USBMODE);
	}

	clrbits_8(&bcsr[17], BCSR17_nUSBEN);
}

#ifdef CONFIG_PCI
void pci_init_board(void)
{
#if defined(CONFIG_PQ_MDS_PIB)
	pib_init();
#endif

	fsl_pcie_init_board(0);
}
#endif /* CONFIG_PCI */

#if defined(CONFIG_OF_BOARD_SETUP)
int ft_board_setup(void *blob, bd_t *bd)
{
#if defined(CONFIG_SYS_UCC_RMII_MODE)
	int nodeoff, off, err;
	unsigned int val;
	const u32 *ph;
	const u32 *index;

	/* fixup device tree for supporting rmii mode */
	nodeoff = -1;
	while ((nodeoff = fdt_node_offset_by_compatible(blob, nodeoff,
				"ucc_geth")) >= 0) {
		err = fdt_setprop_string(blob, nodeoff, "tx-clock-name",
						"clk16");
		if (err < 0) {
			printf("WARNING: could not set tx-clock-name %s.\n",
				fdt_strerror(err));
			break;
		}

		err = fdt_fixup_phy_connection(blob, nodeoff,
				PHY_INTERFACE_MODE_RMII);

		if (err < 0) {
			printf("WARNING: could not set phy-connection-type "
				"%s.\n", fdt_strerror(err));
			break;
		}

		index = fdt_getprop(blob, nodeoff, "cell-index", 0);
		if (index == NULL) {
			printf("WARNING: could not get cell-index of ucc\n");
			break;
		}

		ph = fdt_getprop(blob, nodeoff, "phy-handle", 0);
		if (ph == NULL) {
			printf("WARNING: could not get phy-handle of ucc\n");
			break;
		}

		off = fdt_node_offset_by_phandle(blob, *ph);
		if (off < 0) {
			printf("WARNING: could not get phy node	%s.\n",
				fdt_strerror(err));
			break;
		}

		val = 0x7 + *index; /* RMII phy address starts from 0x8 */

		err = fdt_setprop(blob, off, "reg", &val, sizeof(u32));
		if (err < 0) {
			printf("WARNING: could not set reg for phy-handle "
				"%s.\n", fdt_strerror(err));
			break;
		}
	}
#endif
	ft_cpu_setup(blob, bd);

	FT_FSL_PCI_SETUP;

	fdt_board_fixup_esdhc(blob, bd);
	fdt_board_fixup_qe_uart(blob, bd);
	fdt_board_fixup_qe_usb(blob, bd);

	return 0;
}
#endif
