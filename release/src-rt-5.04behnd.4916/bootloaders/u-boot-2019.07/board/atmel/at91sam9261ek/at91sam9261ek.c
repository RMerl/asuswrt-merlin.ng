// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2007-2008
 * Stelian Pop <stelian@popies.net>
 * Lead Tech Design <www.leadtechdesign.com>
 */

#include <common.h>
#include <debug_uart.h>
#include <asm/io.h>
#include <asm/arch/at91sam9261.h>
#include <asm/arch/at91sam9261_matrix.h>
#include <asm/arch/at91sam9_smc.h>
#include <asm/arch/at91_common.h>
#include <asm/arch/at91_rstc.h>
#include <asm/arch/clk.h>
#include <asm/arch/gpio.h>
#include <lcd.h>
#include <atmel_lcdc.h>
#if defined(CONFIG_RESET_PHY_R) && defined(CONFIG_DRIVER_DM9000)
#include <net.h>
#include <netdev.h>
#endif
#include <asm/mach-types.h>

DECLARE_GLOBAL_DATA_PTR;

/* ------------------------------------------------------------------------- */
/*
 * Miscelaneous platform dependent initialisations
 */

#ifdef CONFIG_CMD_NAND
static void at91sam9261ek_nand_hw_init(void)
{
	struct at91_smc *smc = (struct at91_smc *)ATMEL_BASE_SMC;
	struct at91_matrix *matrix = (struct at91_matrix *)ATMEL_BASE_MATRIX;
	unsigned long csa;

	/* Enable CS3 */
	csa = readl(&matrix->ebicsa);
	csa |= AT91_MATRIX_CS3A_SMC_SMARTMEDIA;

	writel(csa, &matrix->ebicsa);

	/* Configure SMC CS3 for NAND/SmartMedia */
#ifdef CONFIG_AT91SAM9G10EK
	writel(AT91_SMC_SETUP_NWE(2) | AT91_SMC_SETUP_NCS_WR(0) |
		AT91_SMC_SETUP_NRD(2) | AT91_SMC_SETUP_NCS_RD(0),
		&smc->cs[3].setup);
	writel(AT91_SMC_PULSE_NWE(3) | AT91_SMC_PULSE_NCS_WR(7) |
		AT91_SMC_PULSE_NRD(3) | AT91_SMC_PULSE_NCS_RD(7),
		&smc->cs[3].pulse);
	writel(AT91_SMC_CYCLE_NWE(7) | AT91_SMC_CYCLE_NRD(7),
		&smc->cs[3].cycle);
#else
	writel(AT91_SMC_SETUP_NWE(1) | AT91_SMC_SETUP_NCS_WR(0) |
		AT91_SMC_SETUP_NRD(1) | AT91_SMC_SETUP_NCS_RD(0),
		&smc->cs[3].setup);
	writel(AT91_SMC_PULSE_NWE(3) | AT91_SMC_PULSE_NCS_WR(3) |
		AT91_SMC_PULSE_NRD(3) | AT91_SMC_PULSE_NCS_RD(3),
		&smc->cs[3].pulse);
	writel(AT91_SMC_CYCLE_NWE(5) | AT91_SMC_CYCLE_NRD(5),
		&smc->cs[3].cycle);
#endif
	writel(AT91_SMC_MODE_RM_NRD | AT91_SMC_MODE_WM_NWE |
		       AT91_SMC_MODE_EXNW_DISABLE |
#ifdef CONFIG_SYS_NAND_DBW_16
		       AT91_SMC_MODE_DBW_16 |
#else /* CONFIG_SYS_NAND_DBW_8 */
		       AT91_SMC_MODE_DBW_8 |
#endif
		       AT91_SMC_MODE_TDF_CYCLE(2),
		       &smc->cs[3].mode);

	at91_periph_clk_enable(ATMEL_ID_PIOC);

	/* Configure RDY/BSY */
	at91_set_gpio_input(CONFIG_SYS_NAND_READY_PIN, 1);

	/* Enable NandFlash */
	at91_set_gpio_output(CONFIG_SYS_NAND_ENABLE_PIN, 1);

	at91_set_A_periph(AT91_PIN_PC0, 0);	/* NANDOE */
	at91_set_A_periph(AT91_PIN_PC1, 0);	/* NANDWE */
}
#endif

#ifdef CONFIG_DRIVER_DM9000
static void at91sam9261ek_dm9000_hw_init(void)
{
	struct at91_smc *smc = (struct at91_smc *)ATMEL_BASE_SMC;

	/* Configure SMC CS2 for DM9000 */
#ifdef CONFIG_AT91SAM9G10EK
	writel(AT91_SMC_SETUP_NWE(3) | AT91_SMC_SETUP_NCS_WR(0) |
		AT91_SMC_SETUP_NRD(3) | AT91_SMC_SETUP_NCS_RD(0),
		&smc->cs[2].setup);
	writel(AT91_SMC_PULSE_NWE(6) | AT91_SMC_PULSE_NCS_WR(8) |
		AT91_SMC_PULSE_NRD(6) | AT91_SMC_PULSE_NCS_RD(8),
		&smc->cs[2].pulse);
	writel(AT91_SMC_CYCLE_NWE(20) | AT91_SMC_CYCLE_NRD(20),
		&smc->cs[2].cycle);
	writel(AT91_SMC_MODE_RM_NRD | AT91_SMC_MODE_WM_NWE |
		       AT91_SMC_MODE_EXNW_DISABLE |
		       AT91_SMC_MODE_BAT | AT91_SMC_MODE_DBW_16 |
		       AT91_SMC_MODE_TDF_CYCLE(1),
		       &smc->cs[2].mode);
#else
	writel(AT91_SMC_SETUP_NWE(3) | AT91_SMC_SETUP_NCS_WR(0) |
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
#endif

	/* Configure Reset signal as output */
	at91_set_gpio_output(AT91_PIN_PC10, 0);

	/* Configure Interrupt pin as input, no pull-up */
	at91_set_gpio_input(AT91_PIN_PC11, 0);
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
	at91_set_gpio_value(AT91_PIN_PA12, 0);  /* power up */
}

void lcd_disable(void)
{
	at91_set_gpio_value(AT91_PIN_PA12, 1);  /* power down */
}

static void at91sam9261ek_lcd_hw_init(void)
{
	at91_set_A_periph(AT91_PIN_PB1, 0);	/* LCDHSYNC */
	at91_set_A_periph(AT91_PIN_PB2, 0);	/* LCDDOTCK */
	at91_set_A_periph(AT91_PIN_PB3, 0);	/* LCDDEN */
	at91_set_A_periph(AT91_PIN_PB4, 0);	/* LCDCC */
	at91_set_A_periph(AT91_PIN_PB7, 0);	/* LCDD2 */
	at91_set_A_periph(AT91_PIN_PB8, 0);	/* LCDD3 */
	at91_set_A_periph(AT91_PIN_PB9, 0);	/* LCDD4 */
	at91_set_A_periph(AT91_PIN_PB10, 0);	/* LCDD5 */
	at91_set_A_periph(AT91_PIN_PB11, 0);	/* LCDD6 */
	at91_set_A_periph(AT91_PIN_PB12, 0);	/* LCDD7 */
	at91_set_A_periph(AT91_PIN_PB15, 0);	/* LCDD10 */
	at91_set_A_periph(AT91_PIN_PB16, 0);	/* LCDD11 */
	at91_set_A_periph(AT91_PIN_PB17, 0);	/* LCDD12 */
	at91_set_A_periph(AT91_PIN_PB18, 0);	/* LCDD13 */
	at91_set_A_periph(AT91_PIN_PB19, 0);	/* LCDD14 */
	at91_set_A_periph(AT91_PIN_PB20, 0);	/* LCDD15 */
	at91_set_B_periph(AT91_PIN_PB23, 0);	/* LCDD18 */
	at91_set_B_periph(AT91_PIN_PB24, 0);	/* LCDD19 */
	at91_set_B_periph(AT91_PIN_PB25, 0);	/* LCDD20 */
	at91_set_B_periph(AT91_PIN_PB26, 0);	/* LCDD21 */
	at91_set_B_periph(AT91_PIN_PB27, 0);	/* LCDD22 */
	at91_set_B_periph(AT91_PIN_PB28, 0);	/* LCDD23 */

	at91_system_clk_enable(AT91_PMC_HCK1);

	/* For 9G10EK, let U-Boot allocate the framebuffer in SDRAM */
#ifdef CONFIG_AT91SAM9261EK
	gd->fb_base = ATMEL_BASE_SRAM;
#endif
}

#ifdef CONFIG_LCD_INFO
#include <nand.h>
#include <version.h>

void lcd_show_board_info(void)
{
	ulong dram_size, nand_size;
	int i;
	char temp[32];

	lcd_printf ("%s\n", U_BOOT_VERSION);
	lcd_printf ("(C) 2008 ATMEL Corp\n");
	lcd_printf ("at91support@atmel.com\n");
	lcd_printf ("%s CPU at %s MHz\n",
		ATMEL_CPU_NAME,
		strmhz(temp, get_cpu_clk_rate()));

	dram_size = 0;
	for (i = 0; i < CONFIG_NR_DRAM_BANKS; i++)
		dram_size += gd->bd->bi_dram[i].size;
	nand_size = 0;
	for (i = 0; i < CONFIG_SYS_MAX_NAND_DEVICE; i++)
		nand_size += get_nand_dev_by_index(i)->size;
	lcd_printf ("  %ld MB SDRAM, %ld MB NAND\n",
		dram_size >> 20,
		nand_size >> 20 );
}
#endif /* CONFIG_LCD_INFO */
#endif

#ifdef CONFIG_DEBUG_UART_BOARD_INIT
void board_debug_uart_init(void)
{
	at91_seriald_hw_init();
}
#endif

#ifdef CONFIG_BOARD_EARLY_INIT_F
int board_early_init_f(void)
{
#ifdef CONFIG_DEBUG_UART
	debug_uart_init();
#endif
	return 0;
}
#endif

int board_init(void)
{
#ifdef CONFIG_AT91SAM9G10EK
	/* arch number of AT91SAM9G10EK-Board */
	gd->bd->bi_arch_number = MACH_TYPE_AT91SAM9G10EK;
#else
	/* arch number of AT91SAM9261EK-Board */
	gd->bd->bi_arch_number = MACH_TYPE_AT91SAM9261EK;
#endif
	/* adress of boot parameters */
	gd->bd->bi_boot_params = CONFIG_SYS_SDRAM_BASE + 0x100;

#ifdef CONFIG_CMD_NAND
	at91sam9261ek_nand_hw_init();
#endif
#ifdef CONFIG_DRIVER_DM9000
	at91sam9261ek_dm9000_hw_init();
#endif
#ifdef CONFIG_LCD
	at91sam9261ek_lcd_hw_init();
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
	gd->ram_size = get_ram_size((void *)CONFIG_SYS_SDRAM_BASE,
		CONFIG_SYS_SDRAM_SIZE);

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
