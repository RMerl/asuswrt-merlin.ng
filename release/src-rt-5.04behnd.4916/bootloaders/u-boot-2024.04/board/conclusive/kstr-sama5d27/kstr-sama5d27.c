// SPDX-License-Identifier: GPL-2.0+
/*
 * kstr-sama5d27.c - Board init file for Conclusive KSTR-SAMA5D27 board
 * Copyright (C) 2021-2023 Conclusive Engineering Sp. z o. o.
 */

#include <common.h>
#include <debug_uart.h>
#include <init.h>
#include <env.h>
#include <fdt_support.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/arch/at91_common.h>
#include <asm/arch/atmel_pio4.h>
#include <asm/arch/atmel_mpddrc.h>
#include <asm/arch/atmel_sdhci.h>
#include <asm/arch/clk.h>
#include <asm/arch/gpio.h>
#include <asm/arch/sama5d2.h>
#include <linux/delay.h>

#ifdef CONFIG_USB_GADGET_ATMEL_USBA
#include <asm/arch/atmel_usba_udc.h>
#endif

DECLARE_GLOBAL_DATA_PTR;

#ifdef CONFIG_DEBUG_UART_BOARD_INIT
static void board_uart1_hw_init(void)
{
	/* URXD1 */
	atmel_pio4_set_a_periph(AT91_PIO_PORTD, 2, ATMEL_PIO_PUEN_MASK);
	/* UTXD1 */
	atmel_pio4_set_a_periph(AT91_PIO_PORTD, 3, 0);
	at91_periph_clk_enable(ATMEL_ID_UART1);
}

void board_debug_uart_init(void)
{
	board_uart1_hw_init();
}
#endif

void board_lan8720a_init(void)
{
	/* LAN8720A_nRST */
	atmel_pio4_set_pio_output(AT91_PIO_PORTB, 12, 0);
	/*
	 * Force 0 on RXER/PHYAD0. LAN8720A chipset will latch with address 0 on
	 * MDIO bus.
	 */
	atmel_pio4_set_pio_output(AT91_PIO_PORTB, 17, 0);
	/* Minimal delay of reset signal is 25 ms */
	mdelay(30);
	/* LAN8720A_nRST */
	atmel_pio4_set_pio_output(AT91_PIO_PORTB, 12, 1);
}

void board_usba_init(void)
{
#ifdef CONFIG_USB_GADGET_ATMEL_USBA
	/* USB device peripheral initialization: sama5d2_devices.c */
	at91_udp_hw_init();
	/* USB device controller drivers/usb/gadget/atmel_usba_udc.c */
	usba_udc_probe(&pdata);
#endif
}

#ifdef CONFIG_BOARD_EARLY_INIT_F
int board_early_init_f(void)
{
#ifdef CONFIG_DEBUG_UART
	debug_uart_init();
#endif

	return 0;
}
#endif

int ft_board_setup(void *blob, struct bd_info *bd)
{
	char *wlanaddr = env_get("eth1addr");

	if (wlanaddr)
		do_fixup_by_compat(blob, "brcm,bcm4329-fmac", "local-mac-address",
				   wlanaddr, strlen(wlanaddr), 1);
	else
		printf("Not setting WIFI mac address. Check if EEPROM TLV is correctly set up.\n");

	return 0;
}

int board_init(void)
{
	/* address of boot parameters */
	gd->bd->bi_boot_params = CFG_SYS_SDRAM_BASE + 0x100;

	board_usba_init();
	board_lan8720a_init();

	return 0;
}

static int settings_r(void)
{
	mac_read_from_eeprom();
	serial_read_from_eeprom(0);

	return 0;
}
EVENT_SPY_SIMPLE(EVT_SETTINGS_R, settings_r);

#if defined(CONFIG_DISPLAY_BOARDINFO_LATE)
int checkboard(void)
{
	const char *serial_number;

	serial_number = env_get("serial#");
	if (!serial_number)
		printf("Warning: unknown serial number.\n");
	else
		printf("S/N:   %s\n", serial_number);

	return 0;
}
#endif

#ifdef CONFIG_MISC_INIT_R
int misc_init_r(void)
{
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
void spl_board_init(void)
{
}

static void ddrc_conf(struct atmel_mpddrc_config *ddrc)
{
	ddrc->md = (ATMEL_MPDDRC_MD_DBW_16_BITS | ATMEL_MPDDRC_MD_DDR2_SDRAM);

	ddrc->cr = (ATMEL_MPDDRC_CR_NC_COL_10 |
		    ATMEL_MPDDRC_CR_NR_ROW_13 |
		    ATMEL_MPDDRC_CR_CAS_DDR_CAS3 |
		    ATMEL_MPDDRC_CR_DIC_DS |
		    ATMEL_MPDDRC_CR_ZQ_LONG |
		    ATMEL_MPDDRC_CR_NB_8BANKS |
		    ATMEL_MPDDRC_CR_DECOD_INTERLEAVED |
		    ATMEL_MPDDRC_CR_UNAL_SUPPORTED);

	ddrc->rtr = 0x511;

	ddrc->tpr0 = ((7 << ATMEL_MPDDRC_TPR0_TRAS_OFFSET) |
		      (3 << ATMEL_MPDDRC_TPR0_TRCD_OFFSET) |
		      (3 << ATMEL_MPDDRC_TPR0_TWR_OFFSET) |
		      (9 << ATMEL_MPDDRC_TPR0_TRC_OFFSET) |
		      (3 << ATMEL_MPDDRC_TPR0_TRP_OFFSET) |
		      (4 << ATMEL_MPDDRC_TPR0_TRRD_OFFSET) |
		      (4 << ATMEL_MPDDRC_TPR0_TWTR_OFFSET) |
		      (2 << ATMEL_MPDDRC_TPR0_TMRD_OFFSET));

	ddrc->tpr1 = ((22 << ATMEL_MPDDRC_TPR1_TRFC_OFFSET) |
		      (23 << ATMEL_MPDDRC_TPR1_TXSNR_OFFSET) |
		      (200 << ATMEL_MPDDRC_TPR1_TXSRD_OFFSET) |
		      (3 << ATMEL_MPDDRC_TPR1_TXP_OFFSET));

	ddrc->tpr2 = ((2 << ATMEL_MPDDRC_TPR2_TXARD_OFFSET) |
		      (8 << ATMEL_MPDDRC_TPR2_TXARDS_OFFSET) |
		      (4 << ATMEL_MPDDRC_TPR2_TRPA_OFFSET) |
		      (4 << ATMEL_MPDDRC_TPR2_TRTP_OFFSET) |
		      (8 << ATMEL_MPDDRC_TPR2_TFAW_OFFSET));
}

void mem_init(void)
{
	struct at91_pmc *pmc = (struct at91_pmc *)ATMEL_BASE_PMC;
	struct atmel_mpddr *mpddrc = (struct atmel_mpddr *)ATMEL_BASE_MPDDRC;
	struct atmel_mpddrc_config ddrc_config;
	u32 reg;

	ddrc_conf(&ddrc_config);

	at91_periph_clk_enable(ATMEL_ID_MPDDRC);
	writel(AT91_PMC_DDR, &pmc->scer);

	reg = readl(&mpddrc->io_calibr);
	reg &= ~ATMEL_MPDDRC_IO_CALIBR_RDIV;
	reg |= ATMEL_MPDDRC_IO_CALIBR_DDR3_RZQ_55;
	reg &= ~ATMEL_MPDDRC_IO_CALIBR_TZQIO;
	reg |= ATMEL_MPDDRC_IO_CALIBR_TZQIO_(101);
	writel(reg, &mpddrc->io_calibr);

	writel(ATMEL_MPDDRC_RD_DATA_PATH_SHIFT_ONE_CYCLE,
	       &mpddrc->rd_data_path);

	ddr3_init(ATMEL_BASE_MPDDRC, ATMEL_BASE_DDRCS, &ddrc_config);

	writel(0x3, &mpddrc->cal_mr4);
	writel(64, &mpddrc->tim_cal);
}

void at91_pmc_init(void)
{
	u32 tmp;

	/*
	 * While coming from the ROM code, we run on PLLA @ 492 MHz / 164 MHz,
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
