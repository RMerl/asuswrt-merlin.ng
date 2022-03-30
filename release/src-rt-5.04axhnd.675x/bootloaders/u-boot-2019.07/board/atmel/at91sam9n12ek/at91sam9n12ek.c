// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2013 Atmel Corporation
 * Josh Wu <josh.wu@atmel.com>
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/at91sam9x5_matrix.h>
#include <asm/arch/at91sam9_smc.h>
#include <asm/arch/at91_common.h>
#include <asm/arch/at91_rstc.h>
#include <asm/arch/at91_pio.h>
#include <asm/arch/clk.h>
#include <debug_uart.h>
#include <lcd.h>
#include <atmel_hlcdc.h>
#include <netdev.h>

#ifdef CONFIG_LCD_INFO
#include <nand.h>
#include <version.h>
#endif

DECLARE_GLOBAL_DATA_PTR;

/* ------------------------------------------------------------------------- */
/*
 * Miscelaneous platform dependent initialisations
 */
#ifdef CONFIG_NAND_ATMEL
static void at91sam9n12ek_nand_hw_init(void)
{
	struct at91_smc *smc = (struct at91_smc *)ATMEL_BASE_SMC;
	struct at91_matrix *matrix = (struct at91_matrix *)ATMEL_BASE_MATRIX;
	unsigned long csa;

	/* Assign CS3 to NAND/SmartMedia Interface */
	csa = readl(&matrix->ebicsa);
	csa |= AT91_MATRIX_EBI_CS3A_SMC_SMARTMEDIA;
	/* Configure databus */
	csa &= ~AT91_MATRIX_NFD0_ON_D16; /* nandflash connect to D0~D15 */
	/* Configure IO drive */
	csa |= AT91_MATRIX_EBI_EBI_IOSR_NORMAL;

	writel(csa, &matrix->ebicsa);

	/* Configure SMC CS3 for NAND/SmartMedia */
	writel(AT91_SMC_SETUP_NWE(1) | AT91_SMC_SETUP_NCS_WR(0) |
		AT91_SMC_SETUP_NRD(2) | AT91_SMC_SETUP_NCS_RD(0),
		&smc->cs[3].setup);
	writel(AT91_SMC_PULSE_NWE(3) | AT91_SMC_PULSE_NCS_WR(5) |
		AT91_SMC_PULSE_NRD(4) | AT91_SMC_PULSE_NCS_RD(6),
		&smc->cs[3].pulse);
	writel(AT91_SMC_CYCLE_NWE(5) | AT91_SMC_CYCLE_NRD(7),
		&smc->cs[3].cycle);
	writel(AT91_SMC_MODE_RM_NRD | AT91_SMC_MODE_WM_NWE |
		AT91_SMC_MODE_EXNW_DISABLE |
#ifdef CONFIG_SYS_NAND_DBW_16
		AT91_SMC_MODE_DBW_16 |
#else /* CONFIG_SYS_NAND_DBW_8 */
		AT91_SMC_MODE_DBW_8 |
#endif
		AT91_SMC_MODE_TDF_CYCLE(1),
		&smc->cs[3].mode);

	/* Configure RDY/BSY pin */
	at91_set_pio_input(AT91_PIO_PORTD, 5, 1);

	/* Configure ENABLE pin for NandFlash */
	at91_set_pio_output(AT91_PIO_PORTD, 4, 1);

	at91_pio3_set_a_periph(AT91_PIO_PORTD, 0, 1);    /* NAND OE */
	at91_pio3_set_a_periph(AT91_PIO_PORTD, 1, 1);    /* NAND WE */
	at91_pio3_set_a_periph(AT91_PIO_PORTD, 2, 1);    /* ALE */
	at91_pio3_set_a_periph(AT91_PIO_PORTD, 3, 1);    /* CLE */
}
#endif

#ifdef CONFIG_LCD
vidinfo_t panel_info = {
	.vl_col = 480,
	.vl_row = 272,
	.vl_clk = 9000000,
	.vl_bpix = LCD_BPP,
	.vl_sync = 0,
	.vl_tft = 1,
	.vl_hsync_len = 5,
	.vl_left_margin = 8,
	.vl_right_margin = 43,
	.vl_vsync_len = 10,
	.vl_upper_margin = 4,
	.vl_lower_margin = 12,
	.mmio = ATMEL_BASE_LCDC,
};

void lcd_enable(void)
{
	at91_set_pio_output(AT91_PIO_PORTC, 25, 0);	/* power up */
}

void lcd_disable(void)
{
	at91_set_pio_output(AT91_PIO_PORTC, 25, 1);	/* power down */
}

#ifdef CONFIG_LCD_INFO
void lcd_show_board_info(void)
{
	ulong dram_size, nand_size;
	int i;
	char temp[32];

	lcd_printf("%s\n", U_BOOT_VERSION);
	lcd_printf("ATMEL Corp\n");
	lcd_printf("at91@atmel.com\n");
	lcd_printf("%s CPU at %s MHz\n",
		ATMEL_CPU_NAME,
		strmhz(temp, get_cpu_clk_rate()));

	dram_size = 0;
	for (i = 0; i < CONFIG_NR_DRAM_BANKS; i++)
		dram_size += gd->bd->bi_dram[i].size;
	nand_size = 0;
	for (i = 0; i < CONFIG_SYS_MAX_NAND_DEVICE; i++)
		nand_size += get_nand_dev_by_index(i)->size;
	lcd_printf("  %ld MB SDRAM, %ld MB NAND\n",
		dram_size >> 20,
		nand_size >> 20);
}
#endif /* CONFIG_LCD_INFO */
#endif /* CONFIG_LCD */

#ifdef CONFIG_KS8851_MLL
void at91sam9n12ek_ks8851_hw_init(void)
{
	struct at91_smc *smc = (struct at91_smc *)ATMEL_BASE_SMC;

	writel(AT91_SMC_SETUP_NWE(2) | AT91_SMC_SETUP_NCS_WR(0) |
	       AT91_SMC_SETUP_NRD(1) | AT91_SMC_SETUP_NCS_RD(0),
	       &smc->cs[2].setup);
	writel(AT91_SMC_PULSE_NWE(7) | AT91_SMC_PULSE_NCS_WR(7) |
	       AT91_SMC_PULSE_NRD(7) | AT91_SMC_PULSE_NCS_RD(7),
	       &smc->cs[2].pulse);
	writel(AT91_SMC_CYCLE_NWE(9) | AT91_SMC_CYCLE_NRD(9),
	       &smc->cs[2].cycle);
	writel(AT91_SMC_MODE_RM_NRD | AT91_SMC_MODE_WM_NWE |
	       AT91_SMC_MODE_EXNW_DISABLE |
	       AT91_SMC_MODE_BAT | AT91_SMC_MODE_DBW_16 |
	       AT91_SMC_MODE_TDF_CYCLE(1),
	       &smc->cs[2].mode);

	/* Configure NCS2 PIN */
	at91_pio3_set_b_periph(AT91_PIO_PORTD, 19, 0);
}
#endif

#ifdef CONFIG_USB_ATMEL
void at91sam9n12ek_usb_hw_init(void)
{
	at91_set_pio_output(AT91_PIO_PORTB, 7, 0);
}
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
	/* adress of boot parameters */
	gd->bd->bi_boot_params = CONFIG_SYS_SDRAM_BASE + 0x100;

#ifdef CONFIG_NAND_ATMEL
	at91sam9n12ek_nand_hw_init();
#endif

#ifdef CONFIG_LCD
	at91_lcd_hw_init();
#endif

#ifdef CONFIG_KS8851_MLL
	at91sam9n12ek_ks8851_hw_init();
#endif

#ifdef CONFIG_USB_ATMEL
	at91sam9n12ek_usb_hw_init();
#endif

	return 0;
}

#ifdef CONFIG_KS8851_MLL
int board_eth_init(bd_t *bis)
{
	return ks8851_mll_initialize(0, CONFIG_KS8851_MLL_BASEADDR);
}
#endif

int dram_init(void)
{
	gd->ram_size = get_ram_size((void *)CONFIG_SYS_SDRAM_BASE,
					CONFIG_SYS_SDRAM_SIZE);
	return 0;
}

#if defined(CONFIG_SPL_BUILD)
#include <spl.h>
#include <nand.h>

void at91_spl_board_init(void)
{
#ifdef CONFIG_SD_BOOT
	at91_mci_hw_init();
#elif CONFIG_NAND_BOOT
	at91sam9n12ek_nand_hw_init();
#elif CONFIG_SPI_BOOT
	at91_spi0_hw_init(1 << 4);
#endif
}

#include <asm/arch/atmel_mpddrc.h>
static void ddr2_conf(struct atmel_mpddrc_config *ddr2)
{
	ddr2->md = (ATMEL_MPDDRC_MD_DBW_16_BITS | ATMEL_MPDDRC_MD_DDR2_SDRAM);

	ddr2->cr = (ATMEL_MPDDRC_CR_NC_COL_10 |
		    ATMEL_MPDDRC_CR_NR_ROW_13 |
		    ATMEL_MPDDRC_CR_CAS_DDR_CAS3 |
		    ATMEL_MPDDRC_CR_NB_8BANKS |
		    ATMEL_MPDDRC_CR_DECOD_INTERLEAVED);

	ddr2->rtr = 0x411;

	ddr2->tpr0 = (6 << ATMEL_MPDDRC_TPR0_TRAS_OFFSET |
		      2 << ATMEL_MPDDRC_TPR0_TRCD_OFFSET |
		      2 << ATMEL_MPDDRC_TPR0_TWR_OFFSET |
		      8 << ATMEL_MPDDRC_TPR0_TRC_OFFSET |
		      2 << ATMEL_MPDDRC_TPR0_TRP_OFFSET |
		      2 << ATMEL_MPDDRC_TPR0_TRRD_OFFSET |
		      2 << ATMEL_MPDDRC_TPR0_TWTR_OFFSET |
		      2 << ATMEL_MPDDRC_TPR0_TMRD_OFFSET);

	ddr2->tpr1 = (2 << ATMEL_MPDDRC_TPR1_TXP_OFFSET |
		      200 << ATMEL_MPDDRC_TPR1_TXSRD_OFFSET |
		      19 << ATMEL_MPDDRC_TPR1_TXSNR_OFFSET |
		      18 << ATMEL_MPDDRC_TPR1_TRFC_OFFSET);

	ddr2->tpr2 = (2 << ATMEL_MPDDRC_TPR2_TRTP_OFFSET |
		      3 << ATMEL_MPDDRC_TPR2_TRPA_OFFSET |
		      7 << ATMEL_MPDDRC_TPR2_TXARDS_OFFSET |
		      2 << ATMEL_MPDDRC_TPR2_TXARD_OFFSET);
}

void mem_init(void)
{
	struct at91_pmc *pmc = (struct at91_pmc *)ATMEL_BASE_PMC;
	struct at91_matrix *matrix = (struct at91_matrix *)ATMEL_BASE_MATRIX;
	struct atmel_mpddrc_config ddr2;
	unsigned long csa;

	ddr2_conf(&ddr2);

	/* enable DDR2 clock */
	writel(AT91_PMC_DDR, &pmc->scer);

	/* Chip select 1 is for DDR2/SDRAM */
	csa = readl(&matrix->ebicsa);
	csa |= AT91_MATRIX_EBI_CS1A_SDRAMC;
	csa &= ~AT91_MATRIX_EBI_DBPU_OFF;
	csa |= AT91_MATRIX_EBI_DBPD_OFF;
	csa |= AT91_MATRIX_EBI_EBI_IOSR_NORMAL;
	writel(csa, &matrix->ebicsa);

	/* DDRAM2 Controller initialize */
	ddr2_init(ATMEL_BASE_DDRSDRC, ATMEL_BASE_CS1, &ddr2);
}
#endif
