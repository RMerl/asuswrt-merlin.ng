// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2007-2008
 * Stelian Pop <stelian@popies.net>
 * Lead Tech Design <www.leadtechdesign.com>
 *
 * (C) Copyright 2009-2015
 * Daniel Gorsulowski <daniel.gorsulowski@esd.eu>
 * esd electronic system design gmbh <www.esd.eu>
 */

#include <common.h>
#include <asm/io.h>
#include <asm/gpio.h>
#include <asm/mach-types.h>
#include <asm/setup.h>
#include <asm/arch/at91sam9_smc.h>
#include <asm/arch/at91_common.h>
#include <asm/arch/at91_pmc.h>
#include <asm/arch/at91_rstc.h>
#include <asm/arch/at91_matrix.h>
#include <asm/arch/at91_pio.h>
#include <asm/arch/clk.h>
#include <netdev.h>

DECLARE_GLOBAL_DATA_PTR;

/*
 * Miscelaneous platform dependent initialisations
 */

#ifdef CONFIG_REVISION_TAG
static int hw_rev = -1;	/* hardware revision */

int get_hw_rev(void)
{
	if (hw_rev >= 0)
		return hw_rev;

	hw_rev = at91_get_pio_value(AT91_PIO_PORTB, 19);
	hw_rev |= at91_get_pio_value(AT91_PIO_PORTB, 20) << 1;
	hw_rev |= at91_get_pio_value(AT91_PIO_PORTB, 21) << 2;
	hw_rev |= at91_get_pio_value(AT91_PIO_PORTB, 22) << 3;

	if (hw_rev == 15)
		hw_rev = 0;

	return hw_rev;
}
#endif /* CONFIG_REVISION_TAG */

#ifdef CONFIG_CMD_NAND
static void meesc_nand_hw_init(void)
{
	unsigned long csa;
	at91_smc_t	*smc	= (at91_smc_t *) ATMEL_BASE_SMC0;
	at91_matrix_t	*matrix = (at91_matrix_t *) ATMEL_BASE_MATRIX;

	/* Enable CS3 */
	csa = readl(&matrix->csa[0]) | AT91_MATRIX_CSA_EBI_CS3A;
	writel(csa, &matrix->csa[0]);

	/* Configure SMC CS3 for NAND/SmartMedia */
	writel(AT91_SMC_SETUP_NWE(1) | AT91_SMC_SETUP_NCS_WR(1) |
		AT91_SMC_SETUP_NRD(2) | AT91_SMC_SETUP_NCS_RD(2),
		&smc->cs[3].setup);

	writel(AT91_SMC_PULSE_NWE(3) | AT91_SMC_PULSE_NCS_WR(3) |
		AT91_SMC_PULSE_NRD(3) | AT91_SMC_PULSE_NCS_RD(3),
		&smc->cs[3].pulse);

	writel(AT91_SMC_CYCLE_NWE(6) | AT91_SMC_CYCLE_NRD(6),
		&smc->cs[3].cycle);
	writel(AT91_SMC_MODE_RM_NRD | AT91_SMC_MODE_WM_NWE |
		AT91_SMC_MODE_EXNW_DISABLE |
		AT91_SMC_MODE_DBW_8 |
		AT91_SMC_MODE_TDF_CYCLE(12),
		&smc->cs[3].mode);

	/* Configure RDY/BSY */
	gpio_direction_input(CONFIG_SYS_NAND_READY_PIN);

	/* Enable NandFlash */
	gpio_direction_output(CONFIG_SYS_NAND_ENABLE_PIN, 1);
}
#endif /* CONFIG_CMD_NAND */

#ifdef CONFIG_MACB
static void meesc_macb_hw_init(void)
{
	at91_periph_clk_enable(ATMEL_ID_EMAC);

	at91_macb_hw_init();
}
#endif

/*
 * Static memory controller initialization to enable Beckhoff ET1100 EtherCAT
 * controller debugging
 * The ET1100 is located at physical address 0x70000000
 * Its process memory is located at physical address 0x70001000
 */
static void meesc_ethercat_hw_init(void)
{
	at91_smc_t	*smc1	= (at91_smc_t *) ATMEL_BASE_SMC1;

	/* Configure SMC EBI1_CS0 for EtherCAT */
	writel(AT91_SMC_SETUP_NWE(0) | AT91_SMC_SETUP_NCS_WR(0) |
		AT91_SMC_SETUP_NRD(0) | AT91_SMC_SETUP_NCS_RD(0),
		&smc1->cs[0].setup);
	writel(AT91_SMC_PULSE_NWE(4) | AT91_SMC_PULSE_NCS_WR(9) |
		AT91_SMC_PULSE_NRD(5) | AT91_SMC_PULSE_NCS_RD(9),
		&smc1->cs[0].pulse);
	writel(AT91_SMC_CYCLE_NWE(10) | AT91_SMC_CYCLE_NRD(6),
		&smc1->cs[0].cycle);
	/*
	 * Configure behavior at external wait signal, byte-select mode, 16 bit
	 * data bus width, none data float wait states and TDF optimization
	 */
	writel(AT91_SMC_MODE_RM_NRD | AT91_SMC_MODE_EXNW_READY |
		AT91_SMC_MODE_DBW_16 | AT91_SMC_MODE_TDF_CYCLE(0) |
		AT91_SMC_MODE_TDF, &smc1->cs[0].mode);

	/* Configure RDY/BSY */
	at91_set_b_periph(AT91_PIO_PORTE, 20, 0);	/* EBI1_NWAIT */
}

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

int board_eth_init(bd_t *bis)
{
	int rc = 0;
#ifdef CONFIG_MACB
	rc = macb_eth_initialize(0, (void *)ATMEL_BASE_EMAC, 0x00);
#endif
	return rc;
}

#ifdef CONFIG_DISPLAY_BOARDINFO
int checkboard(void)
{
	char str[32];
	u_char hw_type;	/* hardware type */

	/* read the "Type" register of the ET1100 controller */
	hw_type = readb(CONFIG_ET1100_BASE);

	switch (hw_type) {
	case 0x11:
	case 0x3F:
		/* ET1100 present, arch number of MEESC-Board */
		gd->bd->bi_arch_number = MACH_TYPE_MEESC;
		puts("Board: CAN-EtherCAT Gateway");
		break;
	case 0xFF:
		/* no ET1100 present, arch number of EtherCAN/2-Board */
		gd->bd->bi_arch_number = MACH_TYPE_ETHERCAN2;
		puts("Board: EtherCAN/2 Gateway");
		/* switch on LED1D */
		at91_set_pio_output(AT91_PIO_PORTB, 12, 1);
		break;
	default:
		/* assume, no ET1100 present, arch number of EtherCAN/2-Board */
		gd->bd->bi_arch_number = MACH_TYPE_ETHERCAN2;
		printf("ERROR! Read invalid hw_type: %02X\n", hw_type);
		puts("Board: EtherCAN/2 Gateway");
		break;
	}
	if (env_get_f("serial#", str, sizeof(str)) > 0) {
		puts(", serial# ");
		puts(str);
	}
#ifdef CONFIG_REVISION_TAG
	printf("\nHardware-revision: 1.%d\n", get_hw_rev());
#endif
	printf("Mach-type: %lu\n", gd->bd->bi_arch_number);
	return 0;
}
#endif /* CONFIG_DISPLAY_BOARDINFO */

#ifdef CONFIG_SERIAL_TAG
void get_board_serial(struct tag_serialnr *serialnr)
{
	char *str;

	char *serial = env_get("serial#");
	if (serial) {
		str = strchr(serial, '_');
		if (str && (strlen(str) >= 4)) {
			serialnr->high = (*(str + 1) << 8) | *(str + 2);
			serialnr->low = simple_strtoul(str + 3, NULL, 16);
		}
	} else {
		serialnr->high = 0;
		serialnr->low = 0;
	}
}
#endif

#ifdef CONFIG_REVISION_TAG
u32 get_board_rev(void)
{
	return hw_rev | 0x100;
}
#endif

#ifdef CONFIG_MISC_INIT_R
int misc_init_r(void)
{
	char		*str;
	char		buf[32];
	at91_pmc_t	*pmc = (at91_pmc_t *) ATMEL_BASE_PMC;

	/*
	 * Normally the processor clock has a divisor of 2.
	 * In some cases this this needs to be set to 4.
	 * Check the user has set environment mdiv to 4 to change the divisor.
	 */
	str = env_get("mdiv");
	if (str && (strcmp(str, "4") == 0)) {
		writel((readl(&pmc->mckr) & ~AT91_PMC_MDIV) |
			AT91SAM9_PMC_MDIV_4, &pmc->mckr);
		at91_clock_init(CONFIG_SYS_AT91_MAIN_CLOCK);
		serial_setbrg();
		/* Notify the user that the clock is not default */
		printf("Setting master clock to %s MHz\n",
			strmhz(buf, get_mck_clk_rate()));
	}

	return 0;
}
#endif /* CONFIG_MISC_INIT_R */

int board_early_init_f(void)
{
	at91_periph_clk_enable(ATMEL_ID_UHP);

	return 0;
}

int board_init(void)
{
	/* initialize ET1100 Controller */
	meesc_ethercat_hw_init();

	/* adress of boot parameters */
	gd->bd->bi_boot_params = CONFIG_SYS_SDRAM_BASE + 0x100;

#ifdef CONFIG_CMD_NAND
	meesc_nand_hw_init();
#endif
#ifdef CONFIG_MACB
	meesc_macb_hw_init();
#endif
#ifdef CONFIG_AT91_CAN
	at91_can_hw_init();
#endif
#ifdef CONFIG_USB_OHCI_NEW
	at91_uhp_hw_init();
#endif
	return 0;
}
