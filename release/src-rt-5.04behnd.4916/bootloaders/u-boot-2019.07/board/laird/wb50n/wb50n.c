// SPDX-License-Identifier: GPL-2.0+
/*
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/sama5_sfr.h>
#include <asm/arch/sama5d3_smc.h>
#include <asm/arch/at91_common.h>
#include <asm/arch/at91_pmc.h>
#include <asm/arch/at91_rstc.h>
#include <asm/arch/gpio.h>
#include <asm/arch/clk.h>
#include <micrel.h>
#include <net.h>
#include <netdev.h>
#include <spl.h>
#include <asm/arch/atmel_mpddrc.h>
#include <asm/arch/at91_wdt.h>

DECLARE_GLOBAL_DATA_PTR;

/* ------------------------------------------------------------------------- */
/*
 * Miscelaneous platform dependent initialisations
 */

void wb50n_nand_hw_init(void)
{
	struct at91_smc *smc = (struct at91_smc *)ATMEL_BASE_SMC;

	at91_periph_clk_enable(ATMEL_ID_SMC);

	/* Configure SMC CS3 for NAND/SmartMedia */
	writel(AT91_SMC_SETUP_NWE(2) | AT91_SMC_SETUP_NCS_WR(1) |
	       AT91_SMC_SETUP_NRD(2) | AT91_SMC_SETUP_NCS_RD(1),
	       &smc->cs[3].setup);
	writel(AT91_SMC_PULSE_NWE(3) | AT91_SMC_PULSE_NCS_WR(5) |
	       AT91_SMC_PULSE_NRD(3) | AT91_SMC_PULSE_NCS_RD(5),
	       &smc->cs[3].pulse);
	writel(AT91_SMC_CYCLE_NWE(8) | AT91_SMC_CYCLE_NRD(8),
	       &smc->cs[3].cycle);
	writel(AT91_SMC_TIMINGS_TCLR(3) | AT91_SMC_TIMINGS_TADL(10) |
	       AT91_SMC_TIMINGS_TAR(3) | AT91_SMC_TIMINGS_TRR(4) |
	       AT91_SMC_TIMINGS_TWB(5) | AT91_SMC_TIMINGS_RBNSEL(3) |
	       AT91_SMC_TIMINGS_NFSEL(1), &smc->cs[3].timings);
	writel(AT91_SMC_MODE_RM_NRD | AT91_SMC_MODE_WM_NWE |
	       AT91_SMC_MODE_EXNW_DISABLE |
	       AT91_SMC_MODE_DBW_8 |
	       AT91_SMC_MODE_TDF_CYCLE(3), &smc->cs[3].mode);

	/* Disable Flash Write Protect Line */
	at91_set_pio_output(AT91_PIO_PORTE, 14, 1);
}

int board_early_init_f(void)
{
	at91_periph_clk_enable(ATMEL_ID_PIOA);
	at91_periph_clk_enable(ATMEL_ID_PIOB);
	at91_periph_clk_enable(ATMEL_ID_PIOC);
	at91_periph_clk_enable(ATMEL_ID_PIOD);
	at91_periph_clk_enable(ATMEL_ID_PIOE);

	at91_seriald_hw_init();

	return 0;
}

int board_init(void)
{
	/* adress of boot parameters */
	gd->bd->bi_boot_params = CONFIG_SYS_SDRAM_BASE + 0x100;

	wb50n_nand_hw_init();

	at91_macb_hw_init();

	return 0;
}

int dram_init(void)
{
	gd->ram_size = get_ram_size((void *)CONFIG_SYS_SDRAM_BASE,
	                            CONFIG_SYS_SDRAM_SIZE);
	return 0;
}

int board_phy_config(struct phy_device *phydev)
{
	/* rx data delay */
	ksz9021_phy_extended_write(phydev,
	                           MII_KSZ9021_EXT_RGMII_RX_DATA_SKEW, 0x2222);
	/* tx data delay */
	ksz9021_phy_extended_write(phydev,
	                           MII_KSZ9021_EXT_RGMII_TX_DATA_SKEW, 0x2222);
	/* rx/tx clock delay */
	ksz9021_phy_extended_write(phydev,
	                           MII_KSZ9021_EXT_RGMII_CLOCK_SKEW, 0xf2f4);

	return 0;
}

int board_eth_init(bd_t *bis)
{
	int rc = 0;

	rc = macb_eth_initialize(0, (void *)ATMEL_BASE_EMAC, 0x00);

	return rc;
}

#ifdef CONFIG_BOARD_LATE_INIT
#include <linux/ctype.h>
int board_late_init(void)
{
#ifdef CONFIG_ENV_VARS_UBOOT_RUNTIME_CONFIG
	const char *LAIRD_NAME = "lrd_name";
	char name[32], *p;

	strcpy(name, get_cpu_name());
	for (p = name; *p != '\0'; *p = tolower(*p), p++)
		;
	strcat(name, "-wb50n");
	env_set(LAIRD_NAME, name);

#endif

	return 0;
}
#endif

/* SPL */
#ifdef CONFIG_SPL_BUILD
void spl_board_init(void)
{
	wb50n_nand_hw_init();
}

static void ddr2_conf(struct atmel_mpddrc_config *ddr2)
{
	ddr2->md = (ATMEL_MPDDRC_MD_DBW_32_BITS | ATMEL_MPDDRC_MD_LPDDR_SDRAM);

	ddr2->cr = (ATMEL_MPDDRC_CR_NC_COL_9 |
	            ATMEL_MPDDRC_CR_NR_ROW_13 |
	            ATMEL_MPDDRC_CR_CAS_DDR_CAS3 |
	            ATMEL_MPDDRC_CR_NDQS_DISABLED |
	            ATMEL_MPDDRC_CR_DECOD_INTERLEAVED |
	            ATMEL_MPDDRC_CR_UNAL_SUPPORTED);

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
	struct atmel_sfr *sfr = (struct atmel_sfr *)ATMEL_BASE_SFR;
	struct atmel_mpddrc_config ddr2;

	ddr2_conf(&ddr2);

	writel(ATMEL_SFR_DDRCFG_FDQIEN | ATMEL_SFR_DDRCFG_FDQSIEN,
	       &sfr->ddrcfg);

	/* enable MPDDR clock */
	at91_periph_clk_enable(ATMEL_ID_MPDDRC);
	at91_system_clk_enable(AT91_PMC_DDR);

	/* DDRAM2 Controller initialize */
	ddr2_init(ATMEL_BASE_MPDDRC, ATMEL_BASE_DDRCS, &ddr2);
}

void at91_pmc_init(void)
{
	u32 tmp;

	tmp = AT91_PMC_PLLAR_29 |
	      AT91_PMC_PLLXR_PLLCOUNT(0x3f) |
	      AT91_PMC_PLLXR_MUL(43) | AT91_PMC_PLLXR_DIV(1);
	at91_plla_init(tmp);

	at91_pllicpr_init(AT91_PMC_IPLL_PLLA(0x3));

	tmp = AT91_PMC_MCKR_MDIV_4 | AT91_PMC_MCKR_CSS_PLLA;
	at91_mck_init(tmp);
}
#endif
