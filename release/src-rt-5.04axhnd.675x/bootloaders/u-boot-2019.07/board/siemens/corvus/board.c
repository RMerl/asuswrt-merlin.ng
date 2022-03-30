// SPDX-License-Identifier: GPL-2.0+
/*
 * Board functions for Siemens CORVUS (AT91SAM9G45) based board
 * (C) Copyright 2013 Siemens AG
 *
 * Based on:
 * U-Boot file: board/atmel/at91sam9m10g45ek/at91sam9m10g45ek.c
 * (C) Copyright 2007-2008
 * Stelian Pop <stelian@popies.net>
 * Lead Tech Design <www.leadtechdesign.com>
 */

#include <common.h>
#include <dm.h>
#include <asm/io.h>
#include <asm/arch/at91sam9g45_matrix.h>
#include <asm/arch/at91sam9_smc.h>
#include <asm/arch/at91_common.h>
#include <asm/arch/at91_rstc.h>
#include <asm/arch/atmel_serial.h>
#include <asm/arch/gpio.h>
#include <asm/gpio.h>
#include <asm/arch/clk.h>
#if defined(CONFIG_RESET_PHY_R) && defined(CONFIG_MACB)
#include <net.h>
#endif
#ifndef CONFIG_DM_ETH
#include <netdev.h>
#endif
#include <spi.h>

#ifdef CONFIG_USB_GADGET_ATMEL_USBA
#include <asm/arch/atmel_usba_udc.h>
#endif

DECLARE_GLOBAL_DATA_PTR;

static void corvus_request_gpio(void)
{
	gpio_request(CONFIG_SYS_NAND_ENABLE_PIN, "nand ena");
	gpio_request(CONFIG_SYS_NAND_READY_PIN, "nand rdy");
	gpio_request(AT91_PIN_PD7, "d0");
	gpio_request(AT91_PIN_PD8, "d1");
	gpio_request(AT91_PIN_PA12, "d2");
	gpio_request(AT91_PIN_PA13, "d3");
	gpio_request(AT91_PIN_PA15, "d4");
	gpio_request(AT91_PIN_PB7, "recovery button");
	gpio_request(AT91_PIN_PD1, "USB0");
	gpio_request(AT91_PIN_PD3, "USB1");
	gpio_request(AT91_PIN_PB18, "SPICS1");
	gpio_request(AT91_PIN_PB3, "SPICS0");
	gpio_request(CONFIG_RED_LED, "red led");
	gpio_request(CONFIG_GREEN_LED, "green led");
}

static void corvus_nand_hw_init(void)
{
	struct at91_smc *smc = (struct at91_smc *)ATMEL_BASE_SMC;
	struct at91_matrix *matrix = (struct at91_matrix *)ATMEL_BASE_MATRIX;
	unsigned long csa;

	/* Enable CS3 */
	csa = readl(&matrix->ebicsa);
	csa |= AT91_MATRIX_EBI_CS3A_SMC_SMARTMEDIA;
	writel(csa, &matrix->ebicsa);

	/* Configure SMC CS3 for NAND/SmartMedia */
	writel(AT91_SMC_SETUP_NWE(2) | AT91_SMC_SETUP_NCS_WR(0) |
	       AT91_SMC_SETUP_NRD(2) | AT91_SMC_SETUP_NCS_RD(0),
	       &smc->cs[3].setup);
	writel(AT91_SMC_PULSE_NWE(4) | AT91_SMC_PULSE_NCS_WR(4) |
	       AT91_SMC_PULSE_NRD(4) | AT91_SMC_PULSE_NCS_RD(4),
	       &smc->cs[3].pulse);
	writel(AT91_SMC_CYCLE_NWE(7) | AT91_SMC_CYCLE_NRD(7),
	       &smc->cs[3].cycle);
	writel(AT91_SMC_MODE_RM_NRD | AT91_SMC_MODE_WM_NWE |
	       AT91_SMC_MODE_EXNW_DISABLE |
#ifdef CONFIG_SYS_NAND_DBW_16
	       AT91_SMC_MODE_DBW_16 |
#else /* CONFIG_SYS_NAND_DBW_8 */
	       AT91_SMC_MODE_DBW_8 |
#endif
	       AT91_SMC_MODE_TDF_CYCLE(3),
	       &smc->cs[3].mode);

	at91_periph_clk_enable(ATMEL_ID_PIOC);
	at91_periph_clk_enable(ATMEL_ID_PIOA);

	/* Enable NandFlash */
	at91_set_gpio_output(CONFIG_SYS_NAND_ENABLE_PIN, 1);
	at91_set_gpio_input(CONFIG_SYS_NAND_READY_PIN, 1);
}

#if defined(CONFIG_SPL_BUILD)
#include <spl.h>
#include <nand.h>

void spl_board_init(void)
{
	corvus_request_gpio();
	/*
	 * For on the sam9m10g45ek board, the chip wm9711 stay in the test
	 * mode, so it need do some action to exit mode.
	 */
	at91_set_gpio_output(AT91_PIN_PD7, 0);
	at91_set_gpio_output(AT91_PIN_PD8, 0);
	at91_set_pio_pullup(AT91_PIO_PORTD, 7, 1);
	at91_set_pio_pullup(AT91_PIO_PORTD, 8, 1);
	at91_set_pio_pullup(AT91_PIO_PORTA, 12, 1);
	at91_set_pio_pullup(AT91_PIO_PORTA, 13, 1);
	at91_set_pio_pullup(AT91_PIO_PORTA, 15, 1);

	corvus_nand_hw_init();

	/* Configure recovery button PINs */
	at91_set_gpio_input(AT91_PIN_PB7, 1);

	/* check if button is pressed */
	if (at91_get_gpio_value(AT91_PIN_PB7) == 0) {
		u32 boot_device;

		debug("Recovery button pressed\n");
		boot_device = spl_boot_device();
		switch (boot_device) {
#ifdef CONFIG_SPL_NAND_SUPPORT
		case BOOT_DEVICE_NAND:
			nand_init();
			spl_nand_erase_one(0, 0);
			break;
#endif
		}
	}
}

#include <asm/arch/atmel_mpddrc.h>
static void ddr2_conf(struct atmel_mpddrc_config *ddr2)
{
	ddr2->md = (ATMEL_MPDDRC_MD_DBW_16_BITS | ATMEL_MPDDRC_MD_DDR2_SDRAM);

	ddr2->cr = (ATMEL_MPDDRC_CR_NC_COL_10 |
		    ATMEL_MPDDRC_CR_NR_ROW_14 |
		    ATMEL_MPDDRC_CR_DIC_DS |
		    ATMEL_MPDDRC_CR_DQMS_SHARED |
		    ATMEL_MPDDRC_CR_CAS_DDR_CAS3);
	ddr2->rtr = 0x24b;

	ddr2->tpr0 = (6 << ATMEL_MPDDRC_TPR0_TRAS_OFFSET |/* 6*7.5 = 45 ns */
		      2 << ATMEL_MPDDRC_TPR0_TRCD_OFFSET |/* 2*7.5 = 15 ns */
		      2 << ATMEL_MPDDRC_TPR0_TWR_OFFSET | /* 2*7.5 = 15 ns */
		      8 << ATMEL_MPDDRC_TPR0_TRC_OFFSET | /* 8*7.5 = 75 ns */
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
	struct atmel_mpddrc_config ddr2;

	ddr2_conf(&ddr2);

	at91_system_clk_enable(AT91_PMC_DDR);

	/* DDRAM2 Controller initialize */
	ddr2_init(ATMEL_BASE_DDRSDRC0, ATMEL_BASE_CS6, &ddr2);
}
#endif

#ifdef CONFIG_CMD_USB
static void taurus_usb_hw_init(void)
{
	at91_periph_clk_enable(ATMEL_ID_PIODE);

	at91_set_gpio_output(AT91_PIN_PD1, 0);
	at91_set_gpio_output(AT91_PIN_PD3, 0);
}
#endif

#ifdef CONFIG_MACB
static void corvus_macb_hw_init(void)
{
	/* Enable clock */
	at91_periph_clk_enable(ATMEL_ID_EMAC);

	/*
	 * Disable pull-up on:
	 *      RXDV (PA15) => PHY normal mode (not Test mode)
	 *      ERX0 (PA12) => PHY ADDR0
	 *      ERX1 (PA13) => PHY ADDR1 => PHYADDR = 0x0
	 *
	 * PHY has internal pull-down
	 */
	at91_set_pio_pullup(AT91_PIO_PORTA, 15, 0);
	at91_set_pio_pullup(AT91_PIO_PORTA, 12, 0);
	at91_set_pio_pullup(AT91_PIO_PORTA, 13, 0);

	at91_phy_reset();

	/* Re-enable pull-up */
	at91_set_pio_pullup(AT91_PIO_PORTA, 15, 1);
	at91_set_pio_pullup(AT91_PIO_PORTA, 12, 1);
	at91_set_pio_pullup(AT91_PIO_PORTA, 13, 1);

	/* And the pins. */
	at91_macb_hw_init();
}
#endif

int board_early_init_f(void)
{
	at91_seriald_hw_init();
	corvus_request_gpio();
	return 0;
}

#ifdef CONFIG_USB_GADGET_ATMEL_USBA
/* from ./arch/arm/mach-at91/armv7/sama5d3_devices.c */
void at91_udp_hw_init(void)
{
	/* Enable UPLL clock */
	at91_upll_clk_enable();

	/* Enable UDPHS clock */
	at91_periph_clk_enable(ATMEL_ID_UDPHS);
}
#endif

int board_init(void)
{
	/* address of boot parameters */
	gd->bd->bi_boot_params = CONFIG_SYS_SDRAM_BASE + 0x100;

	/* we have to request the gpios again after relocation */
	corvus_request_gpio();
#ifdef CONFIG_CMD_NAND
	corvus_nand_hw_init();
#endif
#ifdef CONFIG_ATMEL_SPI
	at91_spi0_hw_init(1 << 4);
#endif
#ifdef CONFIG_MACB
	corvus_macb_hw_init();
#endif
#ifdef CONFIG_CMD_USB
	taurus_usb_hw_init();
#endif
#ifdef CONFIG_USB_GADGET_ATMEL_USBA
	at91_udp_hw_init();
	usba_udc_probe(&pdata);
#endif
	return 0;
}

int dram_init(void)
{
	gd->ram_size = get_ram_size((void *)CONFIG_SYS_SDRAM_BASE,
				    CONFIG_SYS_SDRAM_SIZE);
	return 0;
}

#ifndef CONFIG_DM_ETH
int board_eth_init(bd_t *bis)
{
	int rc = 0;
#ifdef CONFIG_MACB
	rc = macb_eth_initialize(0, (void *)ATMEL_BASE_EMAC, 0x00);
#endif
	return rc;
}
#endif

/* SPI chip select control */
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

static struct atmel_serial_platdata at91sam9260_serial_plat = {
	.base_addr = ATMEL_BASE_DBGU,
};

U_BOOT_DEVICE(at91sam9260_serial) = {
	.name	= "serial_atmel",
	.platdata = &at91sam9260_serial_plat,
};
