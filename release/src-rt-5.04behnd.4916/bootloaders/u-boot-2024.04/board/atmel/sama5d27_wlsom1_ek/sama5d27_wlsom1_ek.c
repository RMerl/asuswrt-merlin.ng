// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2019 Microchip Technology Inc. and its subsidiaries
 *
 * Author: Nicolas Ferre <nicolas.ferre@microcihp.com>
 */

#include <common.h>
#include <debug_uart.h>
#include <init.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/arch/at91_common.h>
#include <asm/arch/atmel_pio4.h>
#include <asm/arch/atmel_mpddrc.h>
#include <asm/arch/atmel_sdhci.h>
#include <asm/arch/clk.h>
#include <asm/arch/gpio.h>
#include <asm/arch/sama5d2.h>

extern void at91_pda_detect(void);

DECLARE_GLOBAL_DATA_PTR;

static void rgb_leds_init(void)
{
	atmel_pio4_set_pio_output(AT91_PIO_PORTA, 6, 0);	/* LED RED */
	atmel_pio4_set_pio_output(AT91_PIO_PORTA, 7, 0);	/* LED GREEN */
	atmel_pio4_set_pio_output(AT91_PIO_PORTA, 8, 1);	/* LED BLUE */
}

#ifdef CONFIG_BOARD_LATE_INIT
int board_late_init(void)
{
#ifdef CONFIG_VIDEO
	at91_video_show_board_info();
#endif
	at91_pda_detect();
	return 0;
}
#endif

#ifdef CONFIG_DEBUG_UART_BOARD_INIT
static void board_uart0_hw_init(void)
{
	atmel_pio4_set_c_periph(AT91_PIO_PORTB, 26, ATMEL_PIO_PUEN_MASK);	/* URXD0 */
	atmel_pio4_set_c_periph(AT91_PIO_PORTB, 27, 0);				/* UTXD0 */

	at91_periph_clk_enable(ATMEL_ID_UART0);
}

void board_debug_uart_init(void)
{
	board_uart0_hw_init();
}
#endif

#ifdef CONFIG_BOARD_EARLY_INIT_F
int board_early_init_f(void)
{
	return 0;
}
#endif

int board_init(void)
{
	/* address of boot parameters */
	gd->bd->bi_boot_params = CFG_SYS_SDRAM_BASE + 0x100;

	rgb_leds_init();

	return 0;
}

#ifdef CONFIG_MISC_INIT_R
int misc_init_r(void)
{
#ifdef CONFIG_SPI_FLASH_SFDP_SUPPORT
	at91_spi_nor_set_ethaddr();
#endif
	return 0;
}
#endif

int dram_init(void)
{
	gd->ram_size = get_ram_size((void *)CFG_SYS_SDRAM_BASE,
				    CFG_SYS_SDRAM_SIZE);
	return 0;
}

/* SPL */
#ifdef CONFIG_SPL_BUILD

static void board_leds_init(void)
{
	atmel_pio4_set_pio_output(AT91_PIO_PORTA, 6, 0); /* RED */
	atmel_pio4_set_pio_output(AT91_PIO_PORTA, 7, 1); /* GREEN */
	atmel_pio4_set_pio_output(AT91_PIO_PORTA, 8, 0); /* BLUE */
}

#ifdef CONFIG_SD_BOOT
void spl_mmc_init(void)
{
	atmel_pio4_set_a_periph(AT91_PIO_PORTA, 1, 0);	/* CMD */
	atmel_pio4_set_a_periph(AT91_PIO_PORTA, 2, 0);	/* DAT0 */
	atmel_pio4_set_a_periph(AT91_PIO_PORTA, 3, 0);	/* DAT1 */
	atmel_pio4_set_a_periph(AT91_PIO_PORTA, 4, 0);	/* DAT2 */
	atmel_pio4_set_a_periph(AT91_PIO_PORTA, 5, 0);	/* DAT3 */
	atmel_pio4_set_a_periph(AT91_PIO_PORTA, 0, 0);	/* CK */
	atmel_pio4_set_a_periph(AT91_PIO_PORTA, 13, 0);	/* CD */

	at91_periph_clk_enable(ATMEL_ID_SDMMC0);
}
#endif

#ifdef CONFIG_QSPI_BOOT
void spl_qspi_init(void)
{
	atmel_pio4_set_d_periph(AT91_PIO_PORTB, 5, 0);	/* SCK */
	atmel_pio4_set_d_periph(AT91_PIO_PORTB, 6, 0);	/* CS */
	atmel_pio4_set_d_periph(AT91_PIO_PORTB, 7, 0);	/* IO0 */
	atmel_pio4_set_d_periph(AT91_PIO_PORTB, 8, 0);	/* IO1 */
	atmel_pio4_set_d_periph(AT91_PIO_PORTB, 9, 0);	/* IO2 */
	atmel_pio4_set_d_periph(AT91_PIO_PORTB, 10, 0);	/* IO3 */

	at91_periph_clk_enable(ATMEL_ID_QSPI1);
}
#endif

void spl_board_init(void)
{
	board_leds_init();
#ifdef CONFIG_SD_BOOT
	spl_mmc_init();
#endif
#ifdef CONFIG_QSPI_BOOT
	spl_qspi_init();
#endif
}

void spl_display_print(void)
{
}

static void ddrc_conf(struct atmel_mpddrc_config *ddrc)
{
	ddrc->md = (ATMEL_MPDDRC_MD_DBW_32_BITS | ATMEL_MPDDRC_MD_LPDDR2_SDRAM);

	ddrc->cr = (ATMEL_MPDDRC_CR_NC_COL_9 |
		    ATMEL_MPDDRC_CR_NR_ROW_14 |
		    ATMEL_MPDDRC_CR_CAS_DDR_CAS3 |
		    ATMEL_MPDDRC_CR_ZQ_SHORT |
		    ATMEL_MPDDRC_CR_NB_8BANKS |
		    ATMEL_MPDDRC_CR_DECOD_INTERLEAVED |
		    ATMEL_MPDDRC_CR_UNAL_SUPPORTED);

	ddrc->lpddr23_lpr = ATMEL_MPDDRC_LPDDR23_LPR_DS(0x3);

	/*
	 * The AD220032D average time between REFRESH commands (Trefi): 3.9us
	 * 3.9us * 164MHz = 639.6 = 0x27F.
	 */
	ddrc->rtr = 0x27f;
	/* Enable Adjust Refresh Rate */
	ddrc->rtr |= ATMEL_MPDDRC_RTR_ADJ_REF;

	ddrc->tpr0 = ((7 << ATMEL_MPDDRC_TPR0_TRAS_OFFSET) |
		      (3 << ATMEL_MPDDRC_TPR0_TRCD_OFFSET) |
		      (4 << ATMEL_MPDDRC_TPR0_TWR_OFFSET) |
		      (11 << ATMEL_MPDDRC_TPR0_TRC_OFFSET) |
		      (4 << ATMEL_MPDDRC_TPR0_TRP_OFFSET) |
		      (2 << ATMEL_MPDDRC_TPR0_TRRD_OFFSET) |
		      (2 << ATMEL_MPDDRC_TPR0_TWTR_OFFSET) |
		      (5 << ATMEL_MPDDRC_TPR0_TMRD_OFFSET));

	ddrc->tpr1 = ((21 << ATMEL_MPDDRC_TPR1_TRFC_OFFSET) |
		      (0 << ATMEL_MPDDRC_TPR1_TXSNR_OFFSET) |
		      (23 << ATMEL_MPDDRC_TPR1_TXSRD_OFFSET) |
		      (2 << ATMEL_MPDDRC_TPR1_TXP_OFFSET));

	ddrc->tpr2 = ((0 << ATMEL_MPDDRC_TPR2_TXARD_OFFSET) |
		      (0 << ATMEL_MPDDRC_TPR2_TXARDS_OFFSET) |
		      (4 << ATMEL_MPDDRC_TPR2_TRPA_OFFSET) |
		      (2 << ATMEL_MPDDRC_TPR2_TRTP_OFFSET) |
		      (10 << ATMEL_MPDDRC_TPR2_TFAW_OFFSET));

	ddrc->tim_cal = ATMEL_MPDDRC_CALR_ZQCS(15);

	/*
	 * According to the sama5d2 datasheet and the following values:
	 * T Sens = 0.75%/C, V Sens = 0.2%/mV, T driftrate = 1C/sec and V driftrate = 15 mV/s
	 * Warning: note that the values T driftrate and V driftrate are dependent on
	 * the application environment.
	 * ZQCS period is 1.5 / ((0.75 x 1) + (0.2 x 15)) = 0.4s
	 * If Trefi is 3.9us, we have: 400000 / 3.9 = 102564: we can maximize
	 * this timer to 0xFFFE.
	 */
	ddrc->cal_mr4 = ATMEL_MPDDRC_CAL_MR4_COUNT_CAL(0xFFFE);

	/*
	 * MR4 Read interval is dependent on the application environment.
	 * Here, we want to maximize this value as temperature is supposed
	 * to vary slowly in the application chosen.
	 * If Trefi is 3.9us, we have:
	 * (0xFFFE) 65534 x 3.9 = 0.25s between MR4 reads.
	 */
	ddrc->cal_mr4 |= ATMEL_MPDDRC_CAL_MR4_MR4R(0xFFFE);
}

void mem_init(void)
{
	struct at91_pmc *pmc = (struct at91_pmc *)ATMEL_BASE_PMC;
	struct atmel_mpddr *mpddrc = (struct atmel_mpddr *)ATMEL_BASE_MPDDRC;
	struct atmel_mpddrc_config ddrc_config;
	u32 reg;

	at91_periph_clk_enable(ATMEL_ID_MPDDRC);
	writel(AT91_PMC_DDR, &pmc->scer);

	ddrc_conf(&ddrc_config);

	reg = readl(&mpddrc->io_calibr);
	reg &= ~ATMEL_MPDDRC_IO_CALIBR_RDIV;
	reg |= ATMEL_MPDDRC_IO_CALIBR_LPDDR2_RZQ_48;
	reg &= ~ATMEL_MPDDRC_IO_CALIBR_TZQIO;
	reg |= ATMEL_MPDDRC_IO_CALIBR_TZQIO_(100);
	writel(reg, &mpddrc->io_calibr);

	writel(ATMEL_MPDDRC_RD_DATA_PATH_SHIFT_ONE_CYCLE,
	       &mpddrc->rd_data_path);

	lpddr2_init(ATMEL_BASE_MPDDRC, ATMEL_BASE_DDRCS, &ddrc_config);
}

void at91_pmc_init(void)
{
	u32 tmp;

	/*
	 * while coming from the ROM code, we run on PLLA @ 492 MHz / 164 MHz
	 * so we need to slow down and configure MCKR accordingly.
	 * This is why we have a special flavor of the switching function.
	 */
	tmp = AT91_PMC_MCKR_PLLADIV_2 |
	      AT91_PMC_MCKR_MDIV_3 |
	      AT91_PMC_MCKR_CSS_MAIN;
	at91_mck_init_down(tmp);

	tmp = AT91_PMC_PLLAR_29 |
	      AT91_PMC_PLLXR_PLLCOUNT(0x3f) |
	      AT91_PMC_PLLXR_MUL(40) |
	      AT91_PMC_PLLXR_DIV(1);
	at91_plla_init(tmp);

	tmp = AT91_PMC_MCKR_H32MXDIV |
	      AT91_PMC_MCKR_PLLADIV_2 |
	      AT91_PMC_MCKR_MDIV_3 |
	      AT91_PMC_MCKR_CSS_PLLA;
	at91_mck_init(tmp);
}
#endif
