// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2007-2008
 * Stelian Pop <stelian@popies.net>
 * Lead Tech Design <www.leadtechdesign.com>
 * Copyright (C) 2008 Ronetix Ilko Iliev (www.ronetix.at)
 * Copyright (C) 2009 Jean-Christopher PLAGNIOL-VILLARD <plagnioj@jcrosoft.com>
 */

#include <common.h>
#include <linux/sizes.h>
#include <asm/io.h>
#include <asm/gpio.h>
#include <asm/arch/at91sam9_smc.h>
#include <asm/arch/at91_common.h>
#include <asm/arch/at91_rstc.h>
#include <asm/arch/at91_matrix.h>
#include <asm/arch/clk.h>
#include <asm/arch/gpio.h>

#include <lcd.h>
#include <atmel_lcdc.h>
#if defined(CONFIG_RESET_PHY_R) && defined(CONFIG_DRIVER_DM9000)
#include <net.h>
#endif
#include <netdev.h>
#include <asm/mach-types.h>

DECLARE_GLOBAL_DATA_PTR;

/* ------------------------------------------------------------------------- */
/*
 * Miscelaneous platform dependent initialisations
 */

#ifdef CONFIG_CMD_NAND
static void pm9261_nand_hw_init(void)
{
	unsigned long csa;
	struct at91_smc *smc = (struct at91_smc *)ATMEL_BASE_SMC;
	struct at91_matrix *matrix = (struct at91_matrix *)ATMEL_BASE_MATRIX;

	/* Enable CS3 */
	csa = readl(&matrix->csa) | AT91_MATRIX_CSA_EBI_CS3A;
	writel(csa, &matrix->csa);

	/* Configure SMC CS3 for NAND/SmartMedia */
	writel(AT91_SMC_SETUP_NWE(1) | AT91_SMC_SETUP_NCS_WR(0) |
		AT91_SMC_SETUP_NRD(1) | AT91_SMC_SETUP_NCS_RD(0),
		&smc->cs[3].setup);

	writel(AT91_SMC_PULSE_NWE(3) | AT91_SMC_PULSE_NCS_WR(3) |
		AT91_SMC_PULSE_NRD(3) | AT91_SMC_PULSE_NCS_RD(3),
		&smc->cs[3].pulse);

	writel(AT91_SMC_CYCLE_NWE(5) | AT91_SMC_CYCLE_NRD(5),
		&smc->cs[3].cycle);

	writel(AT91_SMC_MODE_RM_NRD | AT91_SMC_MODE_WM_NWE |
		AT91_SMC_MODE_EXNW_DISABLE |
#ifdef CONFIG_SYS_NAND_DBW_16
		AT91_SMC_MODE_DBW_16 |
#else /* CONFIG_SYS_NAND_DBW_8 */
		AT91_SMC_MODE_DBW_8 |
#endif
		AT91_SMC_MODE_TDF_CYCLE(2),
		&smc->cs[3].mode);

	at91_periph_clk_enable(ATMEL_ID_PIOA);
	at91_periph_clk_enable(ATMEL_ID_PIOC);

	/* Configure RDY/BSY */
	gpio_direction_input(CONFIG_SYS_NAND_READY_PIN);

	/* Enable NandFlash */
	gpio_direction_output(CONFIG_SYS_NAND_ENABLE_PIN, 1);

	at91_set_a_periph(AT91_PIO_PORTC, 0, 0);	/* NANDOE */
	at91_set_a_periph(AT91_PIO_PORTC, 1, 0);	/* NANDWE */
}
#endif


#ifdef CONFIG_DRIVER_DM9000
static void pm9261_dm9000_hw_init(void)
{
	struct at91_smc *smc = (struct at91_smc *)ATMEL_BASE_SMC;

	/* Configure SMC CS2 for DM9000 */
	writel(AT91_SMC_SETUP_NWE(2) | AT91_SMC_SETUP_NCS_WR(0) |
		AT91_SMC_SETUP_NRD(2) | AT91_SMC_SETUP_NCS_RD(0),
		&smc->cs[2].setup);

	writel(AT91_SMC_PULSE_NWE(4) | AT91_SMC_PULSE_NCS_WR(8) |
		AT91_SMC_PULSE_NRD(4) | AT91_SMC_PULSE_NCS_RD(8),
		&smc->cs[2].pulse);

	writel(AT91_SMC_CYCLE_NWE(16) | AT91_SMC_CYCLE_NRD(16),
		&smc->cs[2].cycle);

	writel(AT91_SMC_MODE_RM_NRD | AT91_SMC_MODE_WM_NWE |
		AT91_SMC_MODE_EXNW_DISABLE |
		AT91_SMC_MODE_BAT | AT91_SMC_MODE_DBW_16 |
		AT91_SMC_MODE_TDF_CYCLE(1),
		&smc->cs[2].mode);

	/* Configure Interrupt pin as input, no pull-up */
	at91_periph_clk_enable(ATMEL_ID_PIOA);
	at91_set_pio_input(AT91_PIO_PORTA, 24, 0);
}
#endif

#ifdef CONFIG_LCD
vidinfo_t panel_info = {
	.vl_col =		240,
	.vl_row =		320,
	.vl_clk =		4965000,
	.vl_sync =		ATMEL_LCDC_INVLINE_INVERTED |
				ATMEL_LCDC_INVFRAME_INVERTED,
	.vl_bpix =		3,
	.vl_tft =		1,
	.vl_hsync_len =		5,
	.vl_left_margin =	1,
	.vl_right_margin =	33,
	.vl_vsync_len =		1,
	.vl_upper_margin =	1,
	.vl_lower_margin =	0,
	.mmio =			ATMEL_BASE_LCDC,
};

void lcd_enable(void)
{
	at91_set_pio_value(AT91_PIO_PORTA, 22, 0);  /* power up */
}

void lcd_disable(void)
{
	at91_set_pio_value(AT91_PIO_PORTA, 22, 1);  /* power down */
}

static void pm9261_lcd_hw_init(void)
{
	at91_set_a_periph(AT91_PIO_PORTB, 1, 0);	/* LCDHSYNC */
	at91_set_a_periph(AT91_PIO_PORTB, 2, 0);	/* LCDDOTCK */
	at91_set_a_periph(AT91_PIO_PORTB, 3, 0);	/* LCDDEN */
	at91_set_a_periph(AT91_PIO_PORTB, 4, 0);	/* LCDCC */
	at91_set_a_periph(AT91_PIO_PORTB, 7, 0);	/* LCDD2 */
	at91_set_a_periph(AT91_PIO_PORTB, 8, 0);	/* LCDD3 */
	at91_set_a_periph(AT91_PIO_PORTB, 9, 0);	/* LCDD4 */
	at91_set_a_periph(AT91_PIO_PORTB, 10, 0);	/* LCDD5 */
	at91_set_a_periph(AT91_PIO_PORTB, 11, 0);	/* LCDD6 */
	at91_set_a_periph(AT91_PIO_PORTB, 12, 0);	/* LCDD7 */
	at91_set_a_periph(AT91_PIO_PORTB, 15, 0);	/* LCDD10 */
	at91_set_a_periph(AT91_PIO_PORTB, 16, 0);	/* LCDD11 */
	at91_set_a_periph(AT91_PIO_PORTB, 17, 0);	/* LCDD12 */
	at91_set_a_periph(AT91_PIO_PORTB, 18, 0);	/* LCDD13 */
	at91_set_a_periph(AT91_PIO_PORTB, 19, 0);	/* LCDD14 */
	at91_set_a_periph(AT91_PIO_PORTB, 20, 0);	/* LCDD15 */
	at91_set_b_periph(AT91_PIO_PORTB, 23, 0);	/* LCDD18 */
	at91_set_b_periph(AT91_PIO_PORTB, 24, 0);	/* LCDD19 */
	at91_set_b_periph(AT91_PIO_PORTB, 25, 0);	/* LCDD20 */
	at91_set_b_periph(AT91_PIO_PORTB, 26, 0);	/* LCDD21 */
	at91_set_b_periph(AT91_PIO_PORTB, 27, 0);	/* LCDD22 */
	at91_set_b_periph(AT91_PIO_PORTB, 28, 0);	/* LCDD23 */

	at91_system_clk_enable(AT91_PMC_HCK1);

	gd->fb_base = ATMEL_BASE_SRAM;
}

#ifdef CONFIG_LCD_INFO
#include <nand.h>
#include <version.h>

extern flash_info_t flash_info[];

void lcd_show_board_info(void)
{
	ulong dram_size, nand_size, flash_size;
	int i;
	char temp[32];

	lcd_printf ("%s\n", U_BOOT_VERSION);
	lcd_printf ("(C) 2009 Ronetix GmbH\n");
	lcd_printf ("support@ronetix.at\n");
	lcd_printf ("%s CPU at %s MHz",
		CONFIG_SYS_AT91_CPU_NAME,
		strmhz(temp, get_cpu_clk_rate()));

	dram_size = 0;
	for (i = 0; i < CONFIG_NR_DRAM_BANKS; i++)
		dram_size += gd->bd->bi_dram[i].size;

	nand_size = 0;
	for (i = 0; i < CONFIG_SYS_MAX_NAND_DEVICE; i++)
		nand_size += get_nand_dev_by_index(i)->size;

	flash_size = 0;
	for (i = 0; i < CONFIG_SYS_MAX_FLASH_BANKS; i++)
		flash_size += flash_info[i].size;

	lcd_printf ("%ld MB SDRAM, %ld MB NAND\n%ld MB NOR Flash\n"
			"%ld MB DataFlash\n",
		dram_size >> 20,
		nand_size >> 20,
		flash_size >> 20);
}
#endif /* CONFIG_LCD_INFO */

#endif /* CONFIG_LCD */

int board_early_init_f(void)
{
	return 0;
}

int board_init(void)
{
	/* arch number of PM9261-Board */
	gd->bd->bi_arch_number = MACH_TYPE_PM9261;

	/* adress of boot parameters */
	gd->bd->bi_boot_params = PHYS_SDRAM + 0x100;

#ifdef CONFIG_CMD_NAND
	pm9261_nand_hw_init();
#endif
#ifdef CONFIG_DRIVER_DM9000
	pm9261_dm9000_hw_init();
#endif
#ifdef CONFIG_LCD
	pm9261_lcd_hw_init();
#endif
	return 0;
}

#ifdef CONFIG_DRIVER_DM9000
int board_eth_init(bd_t *bis)
{
	return dm9000_initialize(bis);
}
#endif

int dram_init(void)
{
	/* dram_init must store complete ramsize in gd->ram_size */
	gd->ram_size = get_ram_size((void *)PHYS_SDRAM,
				PHYS_SDRAM_SIZE);
	return 0;
}

int dram_init_banksize(void)
{
	gd->bd->bi_dram[0].start = PHYS_SDRAM;
	gd->bd->bi_dram[0].size = PHYS_SDRAM_SIZE;

	return 0;
}

#ifdef CONFIG_RESET_PHY_R
void reset_phy(void)
{
#ifdef CONFIG_DRIVER_DM9000
	/*
	 * Initialize ethernet HW addr prior to starting Linux,
	 * needed for nfsroot
	 */
	eth_init();
#endif
}
#endif

#ifdef CONFIG_DISPLAY_BOARDINFO
int checkboard (void)
{
	char buf[32];

	printf ("Board : Ronetix PM9261\n");
	printf ("Crystal frequency: %8s MHz\n",
					strmhz(buf, get_main_clk_rate()));
	printf ("CPU clock        : %8s MHz\n",
					strmhz(buf, get_cpu_clk_rate()));
	printf ("Master clock     : %8s MHz\n",
					strmhz(buf, get_mck_clk_rate()));

	return 0;
}
#endif
