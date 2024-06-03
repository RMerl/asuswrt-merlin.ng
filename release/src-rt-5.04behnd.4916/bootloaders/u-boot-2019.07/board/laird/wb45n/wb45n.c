// SPDX-License-Identifier: GPL-2.0+
/*
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/at91sam9x5_matrix.h>
#include <asm/arch/at91sam9_smc.h>
#include <asm/arch/at91_common.h>
#include <asm/arch/at91_rstc.h>
#include <asm/arch/clk.h>
#include <asm/arch/gpio.h>
#include <net.h>
#include <netdev.h>

DECLARE_GLOBAL_DATA_PTR;

/* ------------------------------------------------------------------------- */
/*
 * Miscelaneous platform dependent initialisations
 */
static void wb45n_nand_hw_init(void)
{
	struct at91_smc *smc = (struct at91_smc *)ATMEL_BASE_SMC;
	struct at91_matrix *matrix = (struct at91_matrix *)ATMEL_BASE_MATRIX;
	unsigned long csa;

	csa = readl(&matrix->ebicsa);
	/* Enable CS3 */
	csa |= AT91_MATRIX_EBI_CS3A_SMC_SMARTMEDIA;
	/* NAND flash on D0 */
	csa &= ~AT91_MATRIX_NFD0_ON_D16;
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
	       AT91_SMC_MODE_DBW_8 |
	       AT91_SMC_MODE_TDF_CYCLE(1), &smc->cs[3].mode);

	at91_periph_clk_enable(ATMEL_ID_PIOCD);

	/* Configure RDY/BSY */
	at91_set_gpio_input(CONFIG_SYS_NAND_READY_PIN, 1);
	/* Enable NandFlash */
	at91_set_gpio_output(CONFIG_SYS_NAND_ENABLE_PIN, 1);
	/* Disable Flash Write Protect Line */
	at91_set_gpio_output(AT91_PIN_PD10, 1);

	at91_set_a_periph(AT91_PIO_PORTD, 0, 1);	/* NAND OE */
	at91_set_a_periph(AT91_PIO_PORTD, 1, 1);	/* NAND WE */
	at91_set_a_periph(AT91_PIO_PORTD, 2, 1);	/* NAND ALE */
	at91_set_a_periph(AT91_PIO_PORTD, 3, 1);	/* NAND CLE */
}

static void wb45n_gpio_hw_init(void)
{

	/* Configure wifi gpio CHIP_PWD_L */
	at91_set_gpio_output(AT91_PIN_PA28, 0);

	/* Setup USB pins */
	at91_set_gpio_input(AT91_PIN_PB11, 0);
	at91_set_gpio_output(AT91_PIN_PB12, 0);

	/* IRQ pin, pullup, deglitch */
	at91_set_gpio_input(AT91_PIN_PB18, 1);
	at91_set_gpio_deglitch(AT91_PIN_PB18, 1);
}

int board_eth_init(bd_t *bis)
{
	int rc = 0;

	if (has_emac0())
		rc = macb_eth_initialize(0, (void *)ATMEL_BASE_EMAC0, 0x00);

	return rc;
}

int board_early_init_f(void)
{
	at91_seriald_hw_init();
	return 0;
}

int board_init(void)
{
	/* address of boot parameters */
	gd->bd->bi_boot_params = CONFIG_SYS_SDRAM_BASE + 0x100;

	wb45n_gpio_hw_init();

	wb45n_nand_hw_init();

	at91_macb_hw_init();

	return 0;
}

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
	/* Setup GPIO first */
	wb45n_gpio_hw_init();

	/* Bring up NAND */
	wb45n_nand_hw_init();
}

void matrix_init(void)
{
	struct at91_matrix *matrix = (struct at91_matrix *)ATMEL_BASE_MATRIX;
	unsigned long csa;

	csa = readl(&matrix->ebicsa);
	/* Pull ups on D0 - D16 */
	csa &= ~AT91_MATRIX_EBI_DBPU_OFF;
	csa |= AT91_MATRIX_EBI_DBPD_OFF;
	/* Normal drive strength */
	csa |= AT91_MATRIX_EBI_EBI_IOSR_NORMAL;
	/* Multi-port off */
	csa &= ~AT91_MATRIX_MP_ON;
	writel(csa, &matrix->ebicsa);
}

#include <asm/arch/atmel_mpddrc.h>
static void ddr2_conf(struct atmel_mpddrc_config *ddr2)
{
	ddr2->md = (ATMEL_MPDDRC_MD_DBW_16_BITS | ATMEL_MPDDRC_MD_LPDDR_SDRAM);

	ddr2->cr = (ATMEL_MPDDRC_CR_NC_COL_10 |
	            ATMEL_MPDDRC_CR_NR_ROW_13 |
	            ATMEL_MPDDRC_CR_CAS_DDR_CAS3 |
	            ATMEL_MPDDRC_CR_DECOD_INTERLEAVED |
	            ATMEL_MPDDRC_CR_DQMS_SHARED);

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
	struct atmel_mpddrc_config ddr2;
	unsigned long csa;

	ddr2_conf(&ddr2);

	/* enable DDR2 clock */
	at91_system_clk_enable(AT91_PMC_DDR);

	/* Chip select 1 is for DDR2/SDRAM */
	csa = readl(&matrix->ebicsa);
	csa |= AT91_MATRIX_EBI_CS1A_SDRAMC;
	writel(csa, &matrix->ebicsa);

	/* DDRAM2 Controller initialize */
	ddr2_init(ATMEL_BASE_DDRSDRC, ATMEL_BASE_CS1, &ddr2);
}
#endif
