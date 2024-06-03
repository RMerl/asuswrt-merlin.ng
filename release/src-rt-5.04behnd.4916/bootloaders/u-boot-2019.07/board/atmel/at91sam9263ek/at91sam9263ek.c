// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2007-2008
 * Stelian Pop <stelian@popies.net>
 * Lead Tech Design <www.leadtechdesign.com>
 */

#include <common.h>
#include <debug_uart.h>
#include <linux/sizes.h>
#include <asm/arch/at91sam9263.h>
#include <asm/arch/at91sam9_smc.h>
#include <asm/arch/at91_common.h>
#include <asm/arch/at91_matrix.h>
#include <asm/arch/at91_pio.h>
#include <asm/arch/clk.h>
#include <asm/io.h>
#include <asm/arch/gpio.h>
#include <asm/arch/hardware.h>
#include <lcd.h>
#include <atmel_lcdc.h>
#include <asm/mach-types.h>

DECLARE_GLOBAL_DATA_PTR;

/* ------------------------------------------------------------------------- */
/*
 * Miscelaneous platform dependent initialisations
 */

#ifdef CONFIG_CMD_NAND
static void at91sam9263ek_nand_hw_init(void)
{
	unsigned long csa;
	at91_smc_t    *smc    = (at91_smc_t *) ATMEL_BASE_SMC0;
	at91_matrix_t *matrix = (at91_matrix_t *) ATMEL_BASE_MATRIX;

	/* Enable CS3 */
	csa = readl(&matrix->csa[0]) | AT91_MATRIX_CSA_EBI_CS3A;
	writel(csa, &matrix->csa[0]);

	/* Enable CS3 */

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
	at91_periph_clk_enable(ATMEL_ID_PIOCDE);

	/* Configure RDY/BSY */
	at91_set_gpio_input(CONFIG_SYS_NAND_READY_PIN, 1);

	/* Enable NandFlash */
	at91_set_gpio_output(CONFIG_SYS_NAND_ENABLE_PIN, 1);
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
	at91_set_pio_value(AT91_PIO_PORTA, 30, 1);  /* power up */
}

void lcd_disable(void)
{
	at91_set_pio_value(AT91_PIO_PORTA, 30, 0);  /* power down */
}

static void at91sam9263ek_lcd_hw_init(void)
{
	at91_set_a_periph(AT91_PIO_PORTC, 1, 0);	/* LCDHSYNC */
	at91_set_a_periph(AT91_PIO_PORTC, 2, 0);	/* LCDDOTCK */
	at91_set_a_periph(AT91_PIO_PORTC, 3, 0);	/* LCDDEN */
	at91_set_b_periph(AT91_PIO_PORTB, 9, 0);	/* LCDCC */
	at91_set_a_periph(AT91_PIO_PORTC, 6, 0);	/* LCDD2 */
	at91_set_a_periph(AT91_PIO_PORTC, 7, 0);	/* LCDD3 */
	at91_set_a_periph(AT91_PIO_PORTC, 8, 0);	/* LCDD4 */
	at91_set_a_periph(AT91_PIO_PORTC, 9, 0);	/* LCDD5 */
	at91_set_a_periph(AT91_PIO_PORTC, 10, 0);	/* LCDD6 */
	at91_set_a_periph(AT91_PIO_PORTC, 11, 0);	/* LCDD7 */
	at91_set_a_periph(AT91_PIO_PORTC, 14, 0);	/* LCDD10 */
	at91_set_a_periph(AT91_PIO_PORTC, 15, 0);	/* LCDD11 */
	at91_set_a_periph(AT91_PIO_PORTC, 16, 0);	/* LCDD12 */
	at91_set_b_periph(AT91_PIO_PORTC, 12, 0);	/* LCDD13 */
	at91_set_a_periph(AT91_PIO_PORTC, 18, 0);	/* LCDD14 */
	at91_set_a_periph(AT91_PIO_PORTC, 19, 0);	/* LCDD15 */
	at91_set_a_periph(AT91_PIO_PORTC, 22, 0);	/* LCDD18 */
	at91_set_a_periph(AT91_PIO_PORTC, 23, 0);	/* LCDD19 */
	at91_set_a_periph(AT91_PIO_PORTC, 24, 0);	/* LCDD20 */
	at91_set_b_periph(AT91_PIO_PORTC, 17, 0);	/* LCDD21 */
	at91_set_a_periph(AT91_PIO_PORTC, 26, 0);	/* LCDD22 */
	at91_set_a_periph(AT91_PIO_PORTC, 27, 0);	/* LCDD23 */

	at91_periph_clk_enable(ATMEL_ID_LCDC);
	gd->fb_base = ATMEL_BASE_SRAM0;
}

#ifdef CONFIG_LCD_INFO
#include <nand.h>
#include <version.h>

#ifdef CONFIG_MTD_NOR_FLASH
extern flash_info_t flash_info[];
#endif

void lcd_show_board_info(void)
{
	ulong dram_size, nand_size;
#ifdef CONFIG_MTD_NOR_FLASH
	ulong flash_size;
#endif
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
#ifdef CONFIG_MTD_NOR_FLASH
	flash_size = 0;
	for (i = 0; i < CONFIG_SYS_MAX_FLASH_BANKS; i++)
		flash_size += flash_info[i].size;
#endif
	lcd_printf ("  %ld MB SDRAM, %ld MB NAND",
		dram_size >> 20,
		nand_size >> 20 );
#ifdef CONFIG_MTD_NOR_FLASH
	lcd_printf (",\n  %ld MB NOR",
		flash_size >> 20);
#endif
	lcd_puts ("\n");
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
	/* arch number of AT91SAM9263EK-Board */
	gd->bd->bi_arch_number = MACH_TYPE_AT91SAM9263EK;
	/* adress of boot parameters */
	gd->bd->bi_boot_params = CONFIG_SYS_SDRAM_BASE + 0x100;

#ifdef CONFIG_CMD_NAND
	at91sam9263ek_nand_hw_init();
#endif
#ifdef CONFIG_USB_OHCI_NEW
	at91_uhp_hw_init();
#endif
#ifdef CONFIG_LCD
	at91sam9263ek_lcd_hw_init();
#endif
	return 0;
}

int dram_init(void)
{
	gd->ram_size = get_ram_size((void *)CONFIG_SYS_SDRAM_BASE,
		CONFIG_SYS_SDRAM_SIZE);

	return 0;
}

#ifdef CONFIG_RESET_PHY_R
void reset_phy(void)
{
}
#endif
