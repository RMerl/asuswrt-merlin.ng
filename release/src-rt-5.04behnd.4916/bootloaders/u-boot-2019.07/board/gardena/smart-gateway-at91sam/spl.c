// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2012 Atmel Corporation
 * Copyright (C) 2019 Stefan Roese <sr@denx.de>
 */

#include <common.h>
#include <nand.h>
#include <spl.h>
#include <asm/arch/at91sam9x5_matrix.h>
#include <asm/arch/at91sam9_smc.h>
#include <asm/arch/atmel_mpddrc.h>
#include <asm/arch/clk.h>
#include <asm/arch/gpio.h>

static void at91sam9x5ek_nand_hw_init(void)
{
	struct at91_smc *smc = (struct at91_smc *)ATMEL_BASE_SMC;
	struct at91_matrix *matrix = (struct at91_matrix *)ATMEL_BASE_MATRIX;
	unsigned long csa;

	/* Enable CS3 */
	csa = readl(&matrix->ebicsa);
	csa |= AT91_MATRIX_EBI_CS3A_SMC_SMARTMEDIA;

	/* NAND flash on D16 */
	csa |= AT91_MATRIX_NFD0_ON_D16;

	/* Configure IO drive */
	csa &= ~AT91_MATRIX_EBI_EBI_IOSR_NORMAL;

	writel(csa, &matrix->ebicsa);

	/* Configure SMC CS3 for NAND/SmartMedia */
	writel(AT91_SMC_SETUP_NWE(1) | AT91_SMC_SETUP_NCS_WR(0) |
	       AT91_SMC_SETUP_NRD(1) | AT91_SMC_SETUP_NCS_RD(0),
	       &smc->cs[3].setup);
	writel(AT91_SMC_PULSE_NWE(3) | AT91_SMC_PULSE_NCS_WR(5) |
	       AT91_SMC_PULSE_NRD(4) | AT91_SMC_PULSE_NCS_RD(6),
	       &smc->cs[3].pulse);
	writel(AT91_SMC_CYCLE_NWE(5) | AT91_SMC_CYCLE_NRD(6),
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

	at91_periph_clk_enable(ATMEL_ID_PIOCD);

	/* Configure RDY/BSY */
	at91_set_gpio_input(CONFIG_SYS_NAND_READY_PIN, 1);

	/* Enable NandFlash */
	at91_set_gpio_output(CONFIG_SYS_NAND_ENABLE_PIN, 1);

	at91_pio3_set_a_periph(AT91_PIO_PORTD, 0, 1);	/* NAND OE */
	at91_pio3_set_a_periph(AT91_PIO_PORTD, 1, 1);	/* NAND WE */
	at91_pio3_set_a_periph(AT91_PIO_PORTD, 2, 1);	/* NAND ALE */
	at91_pio3_set_a_periph(AT91_PIO_PORTD, 3, 1);	/* NAND CLE */
	at91_pio3_set_a_periph(AT91_PIO_PORTD, 6, 1);
	at91_pio3_set_a_periph(AT91_PIO_PORTD, 7, 1);
	at91_pio3_set_a_periph(AT91_PIO_PORTD, 8, 1);
	at91_pio3_set_a_periph(AT91_PIO_PORTD, 9, 1);
	at91_pio3_set_a_periph(AT91_PIO_PORTD, 10, 1);
	at91_pio3_set_a_periph(AT91_PIO_PORTD, 11, 1);
	at91_pio3_set_a_periph(AT91_PIO_PORTD, 12, 1);
	at91_pio3_set_a_periph(AT91_PIO_PORTD, 13, 1);
}

void at91_spl_board_init(void)
{
	at91sam9x5ek_nand_hw_init();
}

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

	ddr2->tpr2 = (7 << ATMEL_MPDDRC_TPR2_TFAW_OFFSET |
		      2 << ATMEL_MPDDRC_TPR2_TRTP_OFFSET |
		      3 << ATMEL_MPDDRC_TPR2_TRPA_OFFSET |
		      7 << ATMEL_MPDDRC_TPR2_TXARDS_OFFSET |
		      2 << ATMEL_MPDDRC_TPR2_TXARD_OFFSET);
}

void mem_init(void)
{
	struct at91_matrix *matrix = (struct at91_matrix *)ATMEL_BASE_MATRIX;
	struct at91_pmc *pmc = (struct at91_pmc *)ATMEL_BASE_PMC;
	struct atmel_mpddrc_config ddr2;
	unsigned long csa;

	ddr2_conf(&ddr2);

	/* Enable DDR2 clock */
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
