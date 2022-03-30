// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2017 Microchip Corporation
 *		      Wenyou Yang <wenyou.yang@microchip.com>
 */

#include <common.h>
#include <debug_uart.h>
#include <dm.h>
#include <i2c.h>
#include <nand.h>
#include <version.h>
#include <asm/io.h>
#include <asm/arch/at91_common.h>
#include <asm/arch/atmel_pio4.h>
#include <asm/arch/atmel_mpddrc.h>
#include <asm/arch/atmel_sdhci.h>
#include <asm/arch/clk.h>
#include <asm/arch/gpio.h>
#include <asm/arch/sama5d2.h>
#include <asm/arch/sama5d2_smc.h>

extern void at91_pda_detect(void);

DECLARE_GLOBAL_DATA_PTR;

#ifdef CONFIG_NAND_ATMEL
static void board_nand_hw_init(void)
{
	struct at91_smc *smc = (struct at91_smc *)ATMEL_BASE_SMC;

	at91_periph_clk_enable(ATMEL_ID_HSMC);

	/* Configure SMC CS3 for NAND */
	writel(AT91_SMC_SETUP_NWE(1) | AT91_SMC_SETUP_NCS_WR(1) |
	       AT91_SMC_SETUP_NRD(1) | AT91_SMC_SETUP_NCS_RD(1),
	       &smc->cs[3].setup);
	writel(AT91_SMC_PULSE_NWE(2) | AT91_SMC_PULSE_NCS_WR(4) |
	       AT91_SMC_PULSE_NRD(2) | AT91_SMC_PULSE_NCS_RD(3),
	       &smc->cs[3].pulse);
	writel(AT91_SMC_CYCLE_NWE(6) | AT91_SMC_CYCLE_NRD(5),
	       &smc->cs[3].cycle);
	writel(AT91_SMC_TIMINGS_TCLR(2) | AT91_SMC_TIMINGS_TADL(7) |
	       AT91_SMC_TIMINGS_TAR(2)  | AT91_SMC_TIMINGS_TRR(3)   |
	       AT91_SMC_TIMINGS_TWB(7)  | AT91_SMC_TIMINGS_RBNSEL(3) |
	       AT91_SMC_TIMINGS_NFSEL(1), &smc->cs[3].timings);
	writel(AT91_SMC_MODE_RM_NRD | AT91_SMC_MODE_WM_NWE |
	       AT91_SMC_MODE_EXNW_DISABLE |
	       AT91_SMC_MODE_DBW_8 |
	       AT91_SMC_MODE_TDF_CYCLE(3),
	       &smc->cs[3].mode);

	atmel_pio4_set_b_periph(AT91_PIO_PORTA, 22, ATMEL_PIO_DRVSTR_ME);	/* D0 */
	atmel_pio4_set_b_periph(AT91_PIO_PORTA, 23, ATMEL_PIO_DRVSTR_ME);	/* D1 */
	atmel_pio4_set_b_periph(AT91_PIO_PORTA, 24, ATMEL_PIO_DRVSTR_ME);	/* D2 */
	atmel_pio4_set_b_periph(AT91_PIO_PORTA, 25, ATMEL_PIO_DRVSTR_ME);	/* D3 */
	atmel_pio4_set_b_periph(AT91_PIO_PORTA, 26, ATMEL_PIO_DRVSTR_ME);	/* D4 */
	atmel_pio4_set_b_periph(AT91_PIO_PORTA, 27, ATMEL_PIO_DRVSTR_ME);	/* D5 */
	atmel_pio4_set_b_periph(AT91_PIO_PORTA, 28, ATMEL_PIO_DRVSTR_ME);	/* D6 */
	atmel_pio4_set_b_periph(AT91_PIO_PORTA, 29, ATMEL_PIO_DRVSTR_ME);	/* D7 */
	atmel_pio4_set_b_periph(AT91_PIO_PORTB, 2, 0);	/* RE */
	atmel_pio4_set_b_periph(AT91_PIO_PORTA, 30, 0);	/* WE */
	atmel_pio4_set_b_periph(AT91_PIO_PORTA, 31, ATMEL_PIO_PUEN_MASK);	/* NCS */
	atmel_pio4_set_b_periph(AT91_PIO_PORTC, 8, ATMEL_PIO_PUEN_MASK);	/* RDY */
	atmel_pio4_set_b_periph(AT91_PIO_PORTB, 0, ATMEL_PIO_PUEN_MASK);	/* ALE */
	atmel_pio4_set_b_periph(AT91_PIO_PORTB, 1, ATMEL_PIO_PUEN_MASK);	/* CLE */
}
#endif

#ifdef CONFIG_BOARD_LATE_INIT
int board_late_init(void)
{
	at91_pda_detect();
	return 0;
}
#endif

static void board_usb_hw_init(void)
{
	atmel_pio4_set_pio_output(AT91_PIO_PORTB, 12, ATMEL_PIO_PUEN_MASK);
}

#ifdef CONFIG_DEBUG_UART_BOARD_INIT
static void board_uart0_hw_init(void)
{
	atmel_pio4_set_c_periph(AT91_PIO_PORTB, 26, ATMEL_PIO_PUEN_MASK);	/* URXD0 */
	atmel_pio4_set_c_periph(AT91_PIO_PORTB, 27, 0);	/* UTXD0 */

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
#ifdef CONFIG_DEBUG_UART
	debug_uart_init();
#endif
	return 0;
}
#endif

int board_init(void)
{
	/* address of boot parameters */
	gd->bd->bi_boot_params = CONFIG_SYS_SDRAM_BASE + 0x100;

#ifdef CONFIG_NAND_ATMEL
	board_nand_hw_init();
#endif
#ifdef CONFIG_CMD_USB
	board_usb_hw_init();
#endif
	return 0;
}

int dram_init(void)
{
	gd->ram_size = get_ram_size((void *)CONFIG_SYS_SDRAM_BASE,
				    CONFIG_SYS_SDRAM_SIZE);
	return 0;
}

#define AT24MAC_MAC_OFFSET	0xfa

#ifdef CONFIG_MISC_INIT_R
int misc_init_r(void)
{
#ifdef CONFIG_I2C_EEPROM
	at91_set_ethaddr(AT24MAC_MAC_OFFSET);
#endif
	return 0;
}
#endif
