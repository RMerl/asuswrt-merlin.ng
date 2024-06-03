// SPDX-License-Identifier: GPL-2.0+
/*
 * Board functions for mini-box PICOSAM9G45 (AT91SAM9G45) based board
 * (C) Copyright 2015 Inter Act B.V.
 *
 * Based on:
 * U-Boot file: board/atmel/at91sam9m10g45ek/at91sam9m10g45ek.c
 * (C) Copyright 2007-2008
 * Stelian Pop <stelian@popies.net>
 * Lead Tech Design <www.leadtechdesign.com>
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/clk.h>
#include <asm/arch/at91sam9g45_matrix.h>
#include <asm/arch/at91sam9_smc.h>
#include <asm/arch/at91_common.h>
#include <asm/arch/gpio.h>
#include <asm/arch/clk.h>
#include <lcd.h>
#include <linux/mtd/rawnand.h>
#include <atmel_lcdc.h>
#include <atmel_mci.h>
#if defined(CONFIG_RESET_PHY_R) && defined(CONFIG_MACB)
#include <net.h>
#endif
#include <netdev.h>
#include <asm/mach-types.h>

DECLARE_GLOBAL_DATA_PTR;

/* ------------------------------------------------------------------------- */
/*
 * Miscelaneous platform dependent initialisations
 */

#if defined(CONFIG_SPL_BUILD)
#include <spl.h>

void at91_spl_board_init(void)
{
#ifdef CONFIG_SYS_USE_MMC
	at91_mci_hw_init();
#endif
}

#include <asm/arch/atmel_mpddrc.h>
static void ddr2_conf(struct atmel_mpddrc_config *ddr2)
{
	ddr2->md = (ATMEL_MPDDRC_MD_DBW_16_BITS | ATMEL_MPDDRC_MD_DDR2_SDRAM);

	ddr2->cr = (ATMEL_MPDDRC_CR_NC_COL_10 |
		    ATMEL_MPDDRC_CR_NR_ROW_14 |
		    ATMEL_MPDDRC_CR_DQMS_SHARED |
		    ATMEL_MPDDRC_CR_CAS_DDR_CAS3);

	ddr2->rtr = 0x24b;

	ddr2->tpr0 = (6 << ATMEL_MPDDRC_TPR0_TRAS_OFFSET |/* 6*7.5 = 45 ns */
		      2 << ATMEL_MPDDRC_TPR0_TRCD_OFFSET |/* 2*7.5 = 15 ns */
		      2 << ATMEL_MPDDRC_TPR0_TWR_OFFSET | /* 2*7.5 = 15 ns */
		      8 << ATMEL_MPDDRC_TPR0_TRC_OFFSET | /* 8*7.5 = 60 ns */
		      2 << ATMEL_MPDDRC_TPR0_TRP_OFFSET | /* 2*7.5 = 15 ns */
		      1 << ATMEL_MPDDRC_TPR0_TRRD_OFFSET | /* 1*7.5= 7.5 ns*/
		      1 << ATMEL_MPDDRC_TPR0_TWTR_OFFSET | /* 1 clk cycle */
		      2 << ATMEL_MPDDRC_TPR0_TMRD_OFFSET); /* 2 clk cycles */

	ddr2->tpr1 = (2 << ATMEL_MPDDRC_TPR1_TXP_OFFSET | /* 2*7.5 = 15 ns */
		      200 << ATMEL_MPDDRC_TPR1_TXSRD_OFFSET |
		      16 << ATMEL_MPDDRC_TPR1_TXSNR_OFFSET |
		      14 << ATMEL_MPDDRC_TPR1_TRFC_OFFSET);

	ddr2->tpr2 = (1 << ATMEL_MPDDRC_TPR2_TRTP_OFFSET |
		      0 << ATMEL_MPDDRC_TPR2_TRPA_OFFSET |
		      7 << ATMEL_MPDDRC_TPR2_TXARDS_OFFSET |
		      2 << ATMEL_MPDDRC_TPR2_TXARD_OFFSET);
}

void mem_init(void)
{
	struct at91_matrix *mat = (struct at91_matrix *)ATMEL_BASE_MATRIX;
	struct atmel_mpddrc_config ddr2;
	unsigned long csa;

	ddr2_conf(&ddr2);

	at91_system_clk_enable(AT91_PMC_DDR);

	/* Chip select 1 is for DDR2/SDRAM */
	csa = readl(&mat->ebicsa);
	csa |= AT91_MATRIX_EBI_CS1A_SDRAMC;
	csa &= ~AT91_MATRIX_EBI_VDDIOMSEL_3_3V;
	writel(csa, &mat->ebicsa);

	/* DDRAM2 Controller initialize */
	ddr2_init(ATMEL_BASE_DDRSDRC0, ATMEL_BASE_CS6, &ddr2);
	ddr2_init(ATMEL_BASE_DDRSDRC1, ATMEL_BASE_CS1, &ddr2);
}
#endif

#ifdef CONFIG_CMD_USB
static void picosam9g45_usb_hw_init(void)
{
	at91_periph_clk_enable(ATMEL_ID_PIODE);

	at91_set_gpio_output(AT91_PIN_PD1, 0);
	at91_set_gpio_output(AT91_PIN_PD3, 0);
}
#endif

#ifdef CONFIG_MACB
static void picosam9g45_macb_hw_init(void)
{
	struct at91_port *pioa = (struct at91_port *)ATMEL_BASE_PIOA;

	at91_periph_clk_enable(ATMEL_ID_EMAC);

	/*
	 * Disable pull-up on:
	 *      RXDV (PA15) => PHY normal mode (not Test mode)
	 *      ERX0 (PA12) => PHY ADDR0
	 *      ERX1 (PA13) => PHY ADDR1 => PHYADDR = 0x0
	 *
	 * PHY has internal pull-down
	 */
	writel(pin_to_mask(AT91_PIN_PA15) |
	       pin_to_mask(AT91_PIN_PA12) |
	       pin_to_mask(AT91_PIN_PA13),
	       &pioa->pudr);

	at91_phy_reset();

	/* Re-enable pull-up */
	writel(pin_to_mask(AT91_PIN_PA15) |
	       pin_to_mask(AT91_PIN_PA12) |
	       pin_to_mask(AT91_PIN_PA13),
	       &pioa->puer);

	/* And the pins. */
	at91_macb_hw_init();
}
#endif

#ifdef CONFIG_LCD

vidinfo_t panel_info = {
	.vl_col =		480,
	.vl_row =		272,
	.vl_clk =		9000000,
	.vl_sync =		ATMEL_LCDC_INVLINE_NORMAL |
				ATMEL_LCDC_INVFRAME_NORMAL,
	.vl_bpix =		3,
	.vl_tft =		1,
	.vl_hsync_len =		45,
	.vl_left_margin =	1,
	.vl_right_margin =	1,
	.vl_vsync_len =		1,
	.vl_upper_margin =	40,
	.vl_lower_margin =	1,
	.mmio =			ATMEL_BASE_LCDC,
};


void lcd_enable(void)
{
	at91_set_A_periph(AT91_PIN_PE6, 1);	/* power up */
}

void lcd_disable(void)
{
	at91_set_A_periph(AT91_PIN_PE6, 0);	/* power down */
}

static void picosam9g45_lcd_hw_init(void)
{
	at91_set_A_periph(AT91_PIN_PE0, 0);	/* LCDDPWR */
	at91_set_A_periph(AT91_PIN_PE2, 0);	/* LCDCC */
	at91_set_A_periph(AT91_PIN_PE3, 0);	/* LCDVSYNC */
	at91_set_A_periph(AT91_PIN_PE4, 0);	/* LCDHSYNC */
	at91_set_A_periph(AT91_PIN_PE5, 0);	/* LCDDOTCK */

	at91_set_A_periph(AT91_PIN_PE7, 0);	/* LCDD0 */
	at91_set_A_periph(AT91_PIN_PE8, 0);	/* LCDD1 */
	at91_set_A_periph(AT91_PIN_PE9, 0);	/* LCDD2 */
	at91_set_A_periph(AT91_PIN_PE10, 0);	/* LCDD3 */
	at91_set_A_periph(AT91_PIN_PE11, 0);	/* LCDD4 */
	at91_set_A_periph(AT91_PIN_PE12, 0);	/* LCDD5 */
	at91_set_A_periph(AT91_PIN_PE13, 0);	/* LCDD6 */
	at91_set_A_periph(AT91_PIN_PE14, 0);	/* LCDD7 */
	at91_set_A_periph(AT91_PIN_PE15, 0);	/* LCDD8 */
	at91_set_A_periph(AT91_PIN_PE16, 0);	/* LCDD9 */
	at91_set_A_periph(AT91_PIN_PE17, 0);	/* LCDD10 */
	at91_set_A_periph(AT91_PIN_PE18, 0);	/* LCDD11 */
	at91_set_A_periph(AT91_PIN_PE19, 0);	/* LCDD12 */
	at91_set_B_periph(AT91_PIN_PE20, 0);	/* LCDD13 */
	at91_set_A_periph(AT91_PIN_PE21, 0);	/* LCDD14 */
	at91_set_A_periph(AT91_PIN_PE22, 0);	/* LCDD15 */
	at91_set_A_periph(AT91_PIN_PE23, 0);	/* LCDD16 */
	at91_set_A_periph(AT91_PIN_PE24, 0);	/* LCDD17 */
	at91_set_A_periph(AT91_PIN_PE25, 0);	/* LCDD18 */
	at91_set_A_periph(AT91_PIN_PE26, 0);	/* LCDD19 */
	at91_set_A_periph(AT91_PIN_PE27, 0);	/* LCDD20 */
	at91_set_B_periph(AT91_PIN_PE28, 0);	/* LCDD21 */
	at91_set_A_periph(AT91_PIN_PE29, 0);	/* LCDD22 */
	at91_set_A_periph(AT91_PIN_PE30, 0);	/* LCDD23 */

	at91_periph_clk_enable(ATMEL_ID_LCDC);

	gd->fb_base = CONFIG_AT91SAM9G45_LCD_BASE;
}

#ifdef CONFIG_LCD_INFO
#include <nand.h>
#include <version.h>

void lcd_show_board_info(void)
{
	ulong dram_size;
	int i;
	char temp[32];

	lcd_printf("%s\n", U_BOOT_VERSION);
	lcd_printf("(C) 2015 Inter Act B.V.\n");
	lcd_printf("support@interact.nl\n");
	lcd_printf("%s CPU at %s MHz\n",
		   ATMEL_CPU_NAME,
		   strmhz(temp, get_cpu_clk_rate()));

	dram_size = 0;
	for (i = 0; i < CONFIG_NR_DRAM_BANKS; i++)
		dram_size += gd->bd->bi_dram[i].size;
	lcd_printf("  %ld MB SDRAM\n", dram_size >> 20);
}
#endif /* CONFIG_LCD_INFO */
#endif

#ifdef CONFIG_GENERIC_ATMEL_MCI
int board_mmc_init(bd_t *bis)
{
	at91_mci_hw_init();

	return atmel_mci_init((void *)ATMEL_BASE_MCI0);
}
#endif

int board_early_init_f(void)
{
	at91_seriald_hw_init();
	return 0;
}

int board_init(void)
{
	gd->bd->bi_arch_number = MACH_TYPE_PICOSAM9G45;

	/* adress of boot parameters */
	gd->bd->bi_boot_params = CONFIG_SYS_SDRAM_BASE + 0x100;

#ifdef CONFIG_CMD_USB
	picosam9g45_usb_hw_init();
#endif
#ifdef CONFIG_ATMEL_SPI
	at91_spi0_hw_init(1 << 4);
#endif
#ifdef CONFIG_MACB
	picosam9g45_macb_hw_init();
#endif
#ifdef CONFIG_LCD
	picosam9g45_lcd_hw_init();
#endif
	return 0;
}

int dram_init(void)
{
	gd->ram_size    = get_ram_size((long *)PHYS_SDRAM_1, PHYS_SDRAM_1_SIZE)
			+ get_ram_size((long *)PHYS_SDRAM_2, PHYS_SDRAM_2_SIZE);

	return 0;
}

int dram_init_banksize(void)
{
	gd->bd->bi_dram[0].start = PHYS_SDRAM_1;
	gd->bd->bi_dram[0].size = get_ram_size((long *)PHYS_SDRAM_1,
							PHYS_SDRAM_1_SIZE);
	gd->bd->bi_dram[1].start = PHYS_SDRAM_2;
	gd->bd->bi_dram[1].size = get_ram_size((long *)PHYS_SDRAM_2,
							PHYS_SDRAM_2_SIZE);

	return 0;
}

#ifdef CONFIG_RESET_PHY_R
void reset_phy(void)
{
}
#endif

int board_eth_init(bd_t *bis)
{
	int rc = 0;
#ifdef CONFIG_MACB
	rc = macb_eth_initialize(0, (void *)ATMEL_BASE_EMAC, 0x00);
#endif
	return rc;
}

/* SPI chip select control */
#ifdef CONFIG_ATMEL_SPI
#include <spi.h>

int spi_cs_is_valid(unsigned int bus, unsigned int cs)
{
	return bus == 0 && cs < 2;
}

void spi_cs_activate(struct spi_slave *slave)
{
	switch (slave->cs) {
	case 1:
		at91_set_gpio_output(AT91_PIN_PB18, 0);
		break;
	case 0:
	default:
		at91_set_gpio_output(AT91_PIN_PB3, 0);
		break;
	}
}

void spi_cs_deactivate(struct spi_slave *slave)
{
	switch (slave->cs) {
	case 1:
		at91_set_gpio_output(AT91_PIN_PB18, 1);
		break;
	case 0:
	default:
		at91_set_gpio_output(AT91_PIN_PB3, 1);
	break;
	}
}
#endif /* CONFIG_ATMEL_SPI */
