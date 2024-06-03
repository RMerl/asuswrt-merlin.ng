// SPDX-License-Identifier: GPL-2.0+
/*
 * Bluewater Systems Snapper 9260/9G20 modules
 *
 * (C) Copyright 2011 Bluewater Systems
 *   Author: Andre Renaud <andre@bluewatersys.com>
 *   Author: Ryan Mallon <ryan@bluewatersys.com>
 */

#include <common.h>
#include <atmel_lcd.h>
#include <atmel_lcdc.h>
#include <atmel_mci.h>
#include <dm.h>
#include <lcd.h>
#include <net.h>
#ifndef CONFIG_DM_ETH
#include <netdev.h>
#endif
#include <asm/gpio.h>
#include <asm/io.h>
#include <asm/mach-types.h>
#include <asm/arch/at91sam9g45_matrix.h>
#include <asm/arch/at91sam9_smc.h>
#include <asm/arch/at91_common.h>
#include <asm/arch/at91_emac.h>
#include <asm/arch/at91_rstc.h>
#include <asm/arch/at91_rtc.h>
#include <asm/arch/at91_sck.h>
#include <asm/arch/atmel_serial.h>
#include <asm/arch/clk.h>
#include <asm/arch/gpio.h>
#include <dm/uclass-internal.h>

#ifdef CONFIG_GURNARD_SPLASH
#include "splash_logo.h"
#endif

DECLARE_GLOBAL_DATA_PTR;

/* IO Expander pins */
#define IO_EXP_ETH_RESET	(0 << 1)
#define IO_EXP_ETH_POWER	(1 << 1)

#ifdef CONFIG_MACB
static void gurnard_macb_hw_init(void)
{
	struct at91_port *pioa = (struct at91_port *)ATMEL_BASE_PIOA;

	at91_periph_clk_enable(ATMEL_ID_EMAC);

	/*
	 * Enable pull-up on:
	 *	RXDV (PA12) => MODE0 - PHY also has pull-up
	 *	ERX0 (PA13) => MODE1 - PHY also has pull-up
	 *	ERX1 (PA15) => MODE2 - PHY also has pull-up
	 */
	writel(pin_to_mask(AT91_PIN_PA15) |
	       pin_to_mask(AT91_PIN_PA12) |
	       pin_to_mask(AT91_PIN_PA13),
	       &pioa->puer);

	at91_phy_reset();

	at91_macb_hw_init();
}
#endif

#ifdef CONFIG_CMD_NAND
static int gurnard_nand_hw_init(void)
{
	struct at91_matrix *matrix = (struct at91_matrix *)ATMEL_BASE_MATRIX;
	struct at91_smc *smc = (struct at91_smc *)ATMEL_BASE_SMC;
	ulong flags;
	int ret;

	/* Enable CS3 as NAND/SmartMedia */
	setbits_le32(&matrix->ebicsa, AT91_MATRIX_EBI_CS3A_SMC_SMARTMEDIA);

	/* Configure SMC CS3 for NAND/SmartMedia */
	writel(AT91_SMC_SETUP_NWE(2) | AT91_SMC_SETUP_NCS_WR(0) |
	       AT91_SMC_SETUP_NRD(2) | AT91_SMC_SETUP_NCS_RD(0),
	       &smc->cs[3].setup);
	writel(AT91_SMC_PULSE_NWE(4) | AT91_SMC_PULSE_NCS_WR(4) |
	       AT91_SMC_PULSE_NRD(4) | AT91_SMC_PULSE_NCS_RD(4),
	       &smc->cs[3].pulse);
	writel(AT91_SMC_CYCLE_NWE(7) | AT91_SMC_CYCLE_NRD(7),
	       &smc->cs[3].cycle);
#ifdef CONFIG_SYS_NAND_DBW_16
	flags = AT91_SMC_MODE_DBW_16;
#else
	flags = AT91_SMC_MODE_DBW_8;
#endif
	writel(AT91_SMC_MODE_RM_NRD | AT91_SMC_MODE_WM_NWE |
	       AT91_SMC_MODE_EXNW_DISABLE |
	       flags |
	       AT91_SMC_MODE_TDF_CYCLE(3),
	       &smc->cs[3].mode);

	ret = gpio_request(CONFIG_SYS_NAND_READY_PIN, "nand_rdy");
	if (ret)
		return ret;
	gpio_direction_input(CONFIG_SYS_NAND_READY_PIN);

	/* Enable NandFlash */
	ret = gpio_request(CONFIG_SYS_NAND_ENABLE_PIN, "nand_ce");
	if (ret)
		return ret;
	gpio_direction_output(CONFIG_SYS_NAND_ENABLE_PIN, 1);

	return 0;
}
#endif

#ifdef CONFIG_GURNARD_SPLASH
static void lcd_splash(int width, int height)
{
	u16 colour;
	int x, y;
	u16 *base_addr = (u16 *)gd->video_bottom;

	memset(base_addr, 0xff, width * height * 2);
	/*
	 * Blit the logo to the center of the screen
	 */
	for (y = 0; y < BMP_LOGO_HEIGHT; y++) {
		for (x = 0; x < BMP_LOGO_WIDTH; x++) {
			int posx, posy;
			colour = bmp_logo_palette[bmp_logo_bitmap[
			    y * BMP_LOGO_WIDTH + x]];
			posx = x + (width - BMP_LOGO_WIDTH) / 2;
			posy = y;
			base_addr[posy * width + posx] = colour;
		}
	}
}
#endif

#ifdef CONFIG_DM_VIDEO
static void at91sam9g45_lcd_hw_init(void)
{
	at91_set_A_periph(AT91_PIN_PE0, 0);	/* LCDDPWR */
	at91_set_A_periph(AT91_PIN_PE2, 0);	/* LCDCC */
	at91_set_A_periph(AT91_PIN_PE3, 0);	/* LCDVSYNC */
	at91_set_A_periph(AT91_PIN_PE4, 0);	/* LCDHSYNC */
	at91_set_A_periph(AT91_PIN_PE5, 0);	/* LCDDOTCK */

	at91_set_A_periph(AT91_PIN_PE7, 0);	/* LCDD0 */
	at91_set_A_periph(AT91_PIN_PE8, 0);	/* LCDD1 */
	at91_set_A_periph(AT91_PIN_PE9, 0);	/* LCDD2 */
	at91_set_A_periph(AT91_PIN_PE10, 0);	/* LCDD3 */
	at91_set_A_periph(AT91_PIN_PE11, 0);	/* LCDD4 */
	at91_set_A_periph(AT91_PIN_PE12, 0);	/* LCDD5 */
	at91_set_A_periph(AT91_PIN_PE13, 0);	/* LCDD6 */
	at91_set_A_periph(AT91_PIN_PE14, 0);	/* LCDD7 */
	at91_set_A_periph(AT91_PIN_PE15, 0);	/* LCDD8 */
	at91_set_A_periph(AT91_PIN_PE16, 0);	/* LCDD9 */
	at91_set_A_periph(AT91_PIN_PE17, 0);	/* LCDD10 */
	at91_set_A_periph(AT91_PIN_PE18, 0);	/* LCDD11 */
	at91_set_A_periph(AT91_PIN_PE19, 0);	/* LCDD12 */
	at91_set_B_periph(AT91_PIN_PE20, 0);	/* LCDD13 */
	at91_set_A_periph(AT91_PIN_PE21, 0);	/* LCDD14 */
	at91_set_A_periph(AT91_PIN_PE22, 0);	/* LCDD15 */
	at91_set_A_periph(AT91_PIN_PE23, 0);	/* LCDD16 */
	at91_set_A_periph(AT91_PIN_PE24, 0);	/* LCDD17 */
	at91_set_A_periph(AT91_PIN_PE25, 0);	/* LCDD18 */
	at91_set_A_periph(AT91_PIN_PE26, 0);	/* LCDD19 */
	at91_set_A_periph(AT91_PIN_PE27, 0);	/* LCDD20 */
	at91_set_B_periph(AT91_PIN_PE28, 0);	/* LCDD21 */
	at91_set_A_periph(AT91_PIN_PE29, 0);	/* LCDD22 */
	at91_set_A_periph(AT91_PIN_PE30, 0);	/* LCDD23 */

	at91_periph_clk_enable(ATMEL_ID_LCDC);
}
#endif

#ifdef CONFIG_GURNARD_FPGA
/**
 * Initialise the memory bus settings so that we can talk to the
 * memory mapped FPGA
 */
static int fpga_hw_init(void)
{
	struct at91_matrix *matrix = (struct at91_matrix *)ATMEL_BASE_MATRIX;
	struct at91_smc *smc = (struct at91_smc *)ATMEL_BASE_SMC;
	int i;

	setbits_le32(&matrix->ebicsa, AT91_MATRIX_EBI_CS1A_SDRAMC);

	at91_set_a_periph(2, 4, 0); /* EBIA21 */
	at91_set_a_periph(2, 5, 0); /* EBIA22 */
	at91_set_a_periph(2, 6, 0); /* EBIA23 */
	at91_set_a_periph(2, 7, 0); /* EBIA24 */
	at91_set_a_periph(2, 12, 0); /* EBIA25 */
	for (i = 15; i <= 31; i++) /* EBINWAIT & EBID16 - 31 */
		at91_set_a_periph(2, i, 0);

	/* configure SMC cs0 for FPGA access timing */
	writel(AT91_SMC_SETUP_NWE(1) | AT91_SMC_SETUP_NCS_WR(2) |
	       AT91_SMC_SETUP_NRD(0) | AT91_SMC_SETUP_NCS_RD(2),
	       &smc->cs[0].setup);
	writel(AT91_SMC_PULSE_NWE(5) | AT91_SMC_PULSE_NCS_WR(4) |
	       AT91_SMC_PULSE_NRD(6) | AT91_SMC_PULSE_NCS_RD(4),
	       &smc->cs[0].pulse);
	writel(AT91_SMC_CYCLE_NWE(6) | AT91_SMC_CYCLE_NRD(6),
	       &smc->cs[0].cycle);
	writel(AT91_SMC_MODE_BAT |
	       AT91_SMC_MODE_EXNW_DISABLE |
	       AT91_SMC_MODE_DBW_32 |
	       AT91_SMC_MODE_TDF |
	       AT91_SMC_MODE_TDF_CYCLE(2),
	       &smc->cs[0].mode);

	/* Do a write to within EBI_CS1 to enable the SDCK */
	writel(0, ATMEL_BASE_CS1);

	return 0;
}
#endif

#ifdef CONFIG_CMD_USB

#define USB0_ENABLE_PIN		AT91_PIN_PB22
#define USB1_ENABLE_PIN		AT91_PIN_PB23

void gurnard_usb_init(void)
{
	at91_set_gpio_output(USB0_ENABLE_PIN, 1);
	at91_set_gpio_value(USB0_ENABLE_PIN, 0);
	at91_set_gpio_output(USB1_ENABLE_PIN, 1);
	at91_set_gpio_value(USB1_ENABLE_PIN, 0);
}
#endif

#ifdef CONFIG_GENERIC_ATMEL_MCI
int cpu_mmc_init(bd_t *bis)
{
	return atmel_mci_init((void *)ATMEL_BASE_MCI0);
}
#endif

static void gurnard_enable_console(int enable)
{
	at91_set_gpio_output(AT91_PIN_PB14, 1);
	at91_set_gpio_value(AT91_PIN_PB14, enable ? 0 : 1);
}

void at91sam9g45_slowclock_init(void)
{
	/*
	 * On AT91SAM9G45 revC CPUs, the slow clock can be based on an
	 * internal impreciseRC oscillator or an external 32kHz oscillator.
	 * Switch to the latter.
	 */
	unsigned i, tmp;
	ulong *reg = (ulong *)ATMEL_BASE_SCKCR;

	tmp = readl(reg);
	if ((tmp & AT91SAM9G45_SCKCR_OSCSEL) == AT91SAM9G45_SCKCR_OSCSEL_RC) {
		timer_init();
		tmp |= AT91SAM9G45_SCKCR_OSC32EN;
		writel(tmp, reg);
		for (i = 0; i < 1200; i++)
			udelay(1000);
		tmp |= AT91SAM9G45_SCKCR_OSCSEL_32;
		writel(tmp, reg);
		udelay(200);
		tmp &= ~AT91SAM9G45_SCKCR_RCEN;
		writel(tmp, reg);
	}
}

int board_early_init_f(void)
{
	at91_seriald_hw_init();
	gurnard_enable_console(1);

	return 0;
}

int board_init(void)
{
	const char *rev_str;
#ifdef CONFIG_CMD_NAND
	int ret;
#endif

	at91_periph_clk_enable(ATMEL_ID_PIOA);
	at91_periph_clk_enable(ATMEL_ID_PIOB);
	at91_periph_clk_enable(ATMEL_ID_PIOC);
	at91_periph_clk_enable(ATMEL_ID_PIODE);

	at91sam9g45_slowclock_init();

	/*
	 * Clear the RTC IDR to disable all IRQs. Avoid issues when Linux
	 * boots with spurious IRQs.
	 */
	writel(0xffffffff, AT91_RTC_IDR);

	/* Make sure that the reset signal is attached properly */
	setbits_le32(AT91_ASM_RSTC_MR, AT91_RSTC_KEY | AT91_RSTC_MR_URSTEN);

	gd->bd->bi_arch_number = MACH_TYPE_SNAPPER_9260;

	/* Address of boot parameters */
	gd->bd->bi_boot_params = CONFIG_SYS_SDRAM_BASE + 0x100;

#ifdef CONFIG_CMD_NAND
	ret = gurnard_nand_hw_init();
	if (ret)
		return ret;
#endif
#ifdef CONFIG_ATMEL_SPI
	at91_spi0_hw_init(1 << 4);
#endif

#ifdef CONFIG_MACB
	gurnard_macb_hw_init();
#endif

#ifdef CONFIG_GURNARD_FPGA
	fpga_hw_init();
#endif

#ifdef CONFIG_CMD_USB
	gurnard_usb_init();
#endif

#ifdef CONFIG_CMD_MMC
	at91_set_A_periph(AT91_PIN_PA12, 0);
	at91_set_gpio_output(AT91_PIN_PA8, 1);
	at91_set_gpio_value(AT91_PIN_PA8, 0);
	at91_mci_hw_init();
#endif

#ifdef CONFIG_DM_VIDEO
	at91sam9g45_lcd_hw_init();
	at91_set_A_periph(AT91_PIN_PE6, 1);	/* power up */

	/* Select the second timing index for board rev 2 */
	rev_str = env_get("board_rev");
	if (rev_str && !strncmp(rev_str, "2", 1)) {
		struct udevice *dev;

		uclass_find_first_device(UCLASS_VIDEO, &dev);
		if (dev) {
			struct atmel_lcd_platdata *plat = dev_get_platdata(dev);

			plat->timing_index = 1;
		}
	}
#endif

	return 0;
}

int board_late_init(void)
{
	u_int8_t env_enetaddr[8];
	char *env_str;
	char *end;
	int i;

	/*
	 * Set MAC address so we do not need to init Ethernet before Linux
	 * boot
	 */
	env_str = env_get("ethaddr");
	if (env_str) {
		struct at91_emac *emac = (struct at91_emac *)ATMEL_BASE_EMAC;
		/* Parse MAC address */
		for (i = 0; i < 6; i++) {
			env_enetaddr[i] = env_str ?
				simple_strtoul(env_str, &end, 16) : 0;
			if (env_str)
				env_str = (*end) ? end+1 : end;
		}

		/* Set hardware address */
		writel(env_enetaddr[0] | env_enetaddr[1] << 8 |
		       env_enetaddr[2] << 16 | env_enetaddr[3] << 24,
		       &emac->sa2l);
		writel((env_enetaddr[4] | env_enetaddr[5] << 8), &emac->sa2h);

		printf("MAC:   %s\n", env_get("ethaddr"));
	} else {
		/* Not set in environment */
		printf("MAC:   not set\n");
	}
#ifdef CONFIG_GURNARD_SPLASH
	lcd_splash(480, 272);
#endif

	return 0;
}

#ifndef CONFIG_DM_ETH
int board_eth_init(bd_t *bis)
{
	return macb_eth_initialize(0, (void *)ATMEL_BASE_EMAC, 0);
}
#endif

int dram_init(void)
{
	gd->ram_size = get_ram_size((void *)CONFIG_SYS_SDRAM_BASE,
				    CONFIG_SYS_SDRAM_SIZE);
	return 0;
}

void reset_phy(void)
{
}

static struct atmel_serial_platdata at91sam9260_serial_plat = {
	.base_addr = ATMEL_BASE_DBGU,
};

U_BOOT_DEVICE(at91sam9260_serial) = {
	.name	= "serial_atmel",
	.platdata = &at91sam9260_serial_plat,
};
