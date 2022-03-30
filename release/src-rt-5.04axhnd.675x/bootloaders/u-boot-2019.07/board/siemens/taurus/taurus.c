// SPDX-License-Identifier: GPL-2.0+
/*
 * Board functions for Siemens TAURUS (AT91SAM9G20) based boards
 * (C) Copyright Siemens AG
 *
 * Based on:
 * U-Boot file: board/atmel/at91sam9260ek/at91sam9260ek.c
 *
 * (C) Copyright 2007-2008
 * Stelian Pop <stelian@popies.net>
 * Lead Tech Design <www.leadtechdesign.com>
 */

#include <command.h>
#include <common.h>
#include <dm.h>
#include <environment.h>
#include <asm/io.h>
#include <asm/arch/at91sam9260_matrix.h>
#include <asm/arch/at91sam9_smc.h>
#include <asm/arch/at91_common.h>
#include <asm/arch/at91_rstc.h>
#include <asm/arch/gpio.h>
#include <asm/arch/at91sam9_sdramc.h>
#include <asm/arch/atmel_serial.h>
#include <asm/arch/clk.h>
#include <asm/gpio.h>
#include <linux/mtd/rawnand.h>
#include <atmel_mci.h>
#include <asm/arch/at91_spi.h>
#include <spi.h>

#include <net.h>
#ifndef CONFIG_DM_ETH
#include <netdev.h>
#endif

DECLARE_GLOBAL_DATA_PTR;

static void taurus_request_gpio(void)
{
	gpio_request(CONFIG_SYS_NAND_ENABLE_PIN, "nand ena");
	gpio_request(CONFIG_SYS_NAND_READY_PIN, "nand rdy");
	gpio_request(AT91_PIN_PA25, "ena PHY");
}

static void taurus_nand_hw_init(void)
{
	struct at91_smc *smc = (struct at91_smc *)ATMEL_BASE_SMC;
	struct at91_matrix *matrix = (struct at91_matrix *)ATMEL_BASE_MATRIX;
	unsigned long csa;

	/* Assign CS3 to NAND/SmartMedia Interface */
	csa = readl(&matrix->ebicsa);
	csa |= AT91_MATRIX_CS3A_SMC_SMARTMEDIA;
	writel(csa, &matrix->ebicsa);

	/* Configure SMC CS3 for NAND/SmartMedia */
	writel(AT91_SMC_SETUP_NWE(2) | AT91_SMC_SETUP_NCS_WR(0) |
	       AT91_SMC_SETUP_NRD(2) | AT91_SMC_SETUP_NCS_RD(0),
	       &smc->cs[3].setup);
	writel(AT91_SMC_PULSE_NWE(4) | AT91_SMC_PULSE_NCS_WR(3) |
	       AT91_SMC_PULSE_NRD(4) | AT91_SMC_PULSE_NCS_RD(3),
	       &smc->cs[3].pulse);
	writel(AT91_SMC_CYCLE_NWE(7) | AT91_SMC_CYCLE_NRD(7),
	       &smc->cs[3].cycle);
	writel(AT91_SMC_MODE_RM_NRD | AT91_SMC_MODE_WM_NWE |
	       AT91_SMC_MODE_EXNW_DISABLE |
	       AT91_SMC_MODE_DBW_8 |
	       AT91_SMC_MODE_TDF_CYCLE(3),
	       &smc->cs[3].mode);

	/* Configure RDY/BSY */
	at91_set_gpio_input(CONFIG_SYS_NAND_READY_PIN, 1);

	/* Enable NandFlash */
	at91_set_gpio_output(CONFIG_SYS_NAND_ENABLE_PIN, 1);
}

#if defined(CONFIG_SPL_BUILD)
#include <spl.h>
#include <nand.h>
#include <spi_flash.h>

void matrix_init(void)
{
	struct at91_matrix *mat = (struct at91_matrix *)ATMEL_BASE_MATRIX;

	writel((readl(&mat->scfg[3]) & (~AT91_MATRIX_SLOT_CYCLE))
			| AT91_MATRIX_SLOT_CYCLE_(0x40),
			&mat->scfg[3]);
}

#if defined(CONFIG_BOARD_AXM)
static int at91_is_recovery(void)
{
	if ((at91_get_gpio_value(AT91_PIN_PA26) == 0) &&
	    (at91_get_gpio_value(AT91_PIN_PA27) == 0))
		return 1;

	return 0;
}
#elif defined(CONFIG_BOARD_TAURUS)
static int at91_is_recovery(void)
{
	if (at91_get_gpio_value(AT91_PIN_PA31) == 0)
		return 1;

	return 0;
}
#endif

void spl_board_init(void)
{
	taurus_nand_hw_init();
	at91_spi0_hw_init(TAURUS_SPI_MASK);

#if defined(CONFIG_BOARD_AXM)
	/* Configure LED PINs */
	at91_set_gpio_output(AT91_PIN_PA6, 0);
	at91_set_gpio_output(AT91_PIN_PA8, 0);
	at91_set_gpio_output(AT91_PIN_PA9, 0);
	at91_set_gpio_output(AT91_PIN_PA10, 0);
	at91_set_gpio_output(AT91_PIN_PA11, 0);
	at91_set_gpio_output(AT91_PIN_PA12, 0);

	/* Configure recovery button PINs */
	at91_set_gpio_input(AT91_PIN_PA26, 1);
	at91_set_gpio_input(AT91_PIN_PA27, 1);
#elif defined(CONFIG_BOARD_TAURUS)
	at91_set_gpio_input(AT91_PIN_PA31, 1);
#endif

	/* check for recovery mode */
	if (at91_is_recovery() == 1) {
		struct spi_flash *flash;

		puts("Recovery button pressed\n");
		nand_init();
		spl_nand_erase_one(0, 0);
		flash = spi_flash_probe(CONFIG_SF_DEFAULT_BUS,
					0,
					CONFIG_SF_DEFAULT_SPEED,
					CONFIG_SF_DEFAULT_MODE);
		if (!flash) {
			puts("no flash\n");
		} else {
			puts("erase spi flash sector 0\n");
			spi_flash_erase(flash, 0,
					CONFIG_SYS_NAND_U_BOOT_SIZE);
		}
	}
}

#define SDRAM_BASE_CONF	(AT91_SDRAMC_NR_13 | AT91_SDRAMC_CAS_3 \
			 |AT91_SDRAMC_NB_4 | AT91_SDRAMC_DBW_32 \
			 | AT91_SDRAMC_TWR_VAL(3) | AT91_SDRAMC_TRC_VAL(9) \
			 | AT91_SDRAMC_TRP_VAL(3) | AT91_SDRAMC_TRCD_VAL(3) \
			 | AT91_SDRAMC_TRAS_VAL(6) | AT91_SDRAMC_TXSR_VAL(10))

void sdramc_configure(unsigned int mask)
{
	struct at91_matrix *ma = (struct at91_matrix *)ATMEL_BASE_MATRIX;
	struct sdramc_reg setting;

	at91_sdram_hw_init();
	setting.cr = SDRAM_BASE_CONF | mask;
	setting.mdr = AT91_SDRAMC_MD_SDRAM;
	setting.tr = (CONFIG_SYS_MASTER_CLOCK * 7) / 1000000;

	writel(readl(&ma->ebicsa) | AT91_MATRIX_CS1A_SDRAMC |
		AT91_MATRIX_VDDIOMSEL_3_3V | AT91_MATRIX_EBI_IOSR_SEL,
		&ma->ebicsa);

	sdramc_initialize(ATMEL_BASE_CS1, &setting);
}

void mem_init(void)
{
	unsigned int ram_size = 0;

	/* Configure SDRAM for 128MB */
	sdramc_configure(AT91_SDRAMC_NC_10);

	/* Do memtest for 128MB */
	ram_size = get_ram_size((void *)CONFIG_SYS_SDRAM_BASE,
				CONFIG_SYS_SDRAM_SIZE);

	/*
	 * If 32MB or 16MB should be supported check also for
	 * expected mirroring at A16 and A17
	 * To find mirror addresses depends how the collumns are connected
	 * at RAM (internaly or externaly)
	 * If the collumns are not in inverted order the mirror size effect
	 * behaves like normal SRAM with A0,A1,A2,etc. connected incremantal
	 */

	/* Mirrors at A15 on ATMEL G20 SDRAM Controller with 64MB*/
	if (ram_size == 0x800) {
		printf("\n\r 64MB\n");
		sdramc_configure(AT91_SDRAMC_NC_9);
	} else {
		/* Size already initialized */
		printf("\n\r 128MB\n");
	}
}
#endif

#ifdef CONFIG_MACB
static void siemens_phy_reset(void)
{
	/*
	 * we need to reset PHY for 200us
	 * because of bug in ATMEL G20 CPU (undefined initial state of GPIO)
	 */
	if ((readl(AT91_ASM_RSTC_SR) & AT91_RSTC_RSTTYP) ==
	    AT91_RSTC_RSTTYP_GENERAL)
		at91_set_gpio_value(AT91_PIN_PA25, 0); /* reset eth switch */
}

static void taurus_macb_hw_init(void)
{
	/* Enable EMAC clock */
	at91_periph_clk_enable(ATMEL_ID_EMAC0);

	/*
	 * Disable pull-up on:
	 *	RXDV (PA17) => PHY normal mode (not Test mode)
	 *	ERX0 (PA14) => PHY ADDR0
	 *	ERX1 (PA15) => PHY ADDR1
	 *	ERX2 (PA25) => PHY ADDR2
	 *	ERX3 (PA26) => PHY ADDR3
	 *	ECRS (PA28) => PHY ADDR4  => PHYADDR = 0x0
	 *
	 * PHY has internal pull-down
	 */
	at91_set_pio_pullup(AT91_PIO_PORTA, 14, 0);
	at91_set_pio_pullup(AT91_PIO_PORTA, 15, 0);
	at91_set_pio_pullup(AT91_PIO_PORTA, 17, 0);
	at91_set_pio_pullup(AT91_PIO_PORTA, 25, 0);
	at91_set_pio_pullup(AT91_PIO_PORTA, 26, 0);
	at91_set_pio_pullup(AT91_PIO_PORTA, 28, 0);

	siemens_phy_reset();

	at91_phy_reset();

	at91_set_gpio_input(AT91_PIN_PA25, 1);   /* ERST tri-state */

	/* Re-enable pull-up */
	at91_set_pio_pullup(AT91_PIO_PORTA, 14, 1);
	at91_set_pio_pullup(AT91_PIO_PORTA, 15, 1);
	at91_set_pio_pullup(AT91_PIO_PORTA, 17, 1);
	at91_set_pio_pullup(AT91_PIO_PORTA, 25, 1);
	at91_set_pio_pullup(AT91_PIO_PORTA, 26, 1);
	at91_set_pio_pullup(AT91_PIO_PORTA, 28, 1);

	/* Initialize EMAC=MACB hardware */
	at91_macb_hw_init();
}
#endif

#ifdef CONFIG_GENERIC_ATMEL_MCI
int board_mmc_init(bd_t *bd)
{
	at91_mci_hw_init();

	return atmel_mci_init((void *)ATMEL_BASE_MCI);
}
#endif

int board_early_init_f(void)
{
	/* Enable clocks for all PIOs */
	at91_periph_clk_enable(ATMEL_ID_PIOA);
	at91_periph_clk_enable(ATMEL_ID_PIOB);
	at91_periph_clk_enable(ATMEL_ID_PIOC);

	at91_seriald_hw_init();
	taurus_request_gpio();

	return 0;
}

#ifdef CONFIG_USB_GADGET_AT91
#include <linux/usb/at91_udc.h>

void at91_udp_hw_init(void)
{
	/* Enable PLLB */
	at91_pllb_clk_enable(get_pllb_init());

	/* Enable UDPCK clock, MCK is enabled in at91_clock_init() */
	at91_periph_clk_enable(ATMEL_ID_UDP);

	at91_system_clk_enable(AT91SAM926x_PMC_UDP);
}

struct at91_udc_data board_udc_data  = {
	.baseaddr = ATMEL_BASE_UDP0,
};
#endif

int board_init(void)
{
	/* adress of boot parameters */
	gd->bd->bi_boot_params = CONFIG_SYS_SDRAM_BASE + 0x100;

	taurus_request_gpio();
#ifdef CONFIG_CMD_NAND
	taurus_nand_hw_init();
#endif
#ifdef CONFIG_MACB
	taurus_macb_hw_init();
#endif
	at91_spi0_hw_init(TAURUS_SPI_MASK);
#ifdef CONFIG_USB_GADGET_AT91
	at91_udp_hw_init();
	at91_udc_probe(&board_udc_data);
#endif

	return 0;
}

int dram_init(void)
{
	gd->ram_size = get_ram_size((void *)CONFIG_SYS_SDRAM_BASE,
				    CONFIG_SYS_SDRAM_SIZE);
	return 0;
}

#if !defined(CONFIG_SPL_BUILD)
#if defined(CONFIG_BOARD_AXM)
/*
 * Booting the Fallback Image.
 *
 *  The function is used to provide and
 *  boot the image with the fallback
 *  parameters, incase if the faulty image
 *  in upgraded over the base firmware.
 *
 */
static int upgrade_failure_fallback(void)
{
	char *partitionset_active = NULL;
	char *rootfs = NULL;
	char *rootfs_fallback = NULL;
	char *kern_off;
	char *kern_off_fb;
	char *kern_size;
	char *kern_size_fb;

	partitionset_active = env_get("partitionset_active");
	if (partitionset_active) {
		if (partitionset_active[0] == 'A')
			env_set("partitionset_active", "B");
		else
			env_set("partitionset_active", "A");
	} else {
		printf("partitionset_active missing.\n");
		return -ENOENT;
	}

	rootfs = env_get("rootfs");
	rootfs_fallback = env_get("rootfs_fallback");
	env_set("rootfs", rootfs_fallback);
	env_set("rootfs_fallback", rootfs);

	kern_size = env_get("kernel_size");
	kern_size_fb = env_get("kernel_size_fallback");
	env_set("kernel_size", kern_size_fb);
	env_set("kernel_size_fallback", kern_size);

	kern_off = env_get("kernel_Off");
	kern_off_fb = env_get("kernel_Off_fallback");
	env_set("kernel_Off", kern_off_fb);
	env_set("kernel_Off_fallback", kern_off);

	env_set("bootargs", '\0');
	env_set("upgrade_available", '\0');
	env_set("boot_retries", '\0');
	env_save();

	return 0;
}

static int do_upgrade_available(cmd_tbl_t *cmdtp, int flag, int argc,
			char * const argv[])
{
	unsigned long upgrade_available = 0;
	unsigned long boot_retry = 0;
	char boot_buf[10];

	upgrade_available = simple_strtoul(env_get("upgrade_available"), NULL,
					   10);
	if (upgrade_available) {
		boot_retry = simple_strtoul(env_get("boot_retries"), NULL, 10);
		boot_retry++;
		sprintf(boot_buf, "%lx", boot_retry);
		env_set("boot_retries", boot_buf);
		env_save();

		/*
		 * Here the boot_retries count is checked, and if the
		 * count becomes greater than 2 switch back to the
		 * fallback, and reset the board.
		 */

		if (boot_retry > 2) {
			if (upgrade_failure_fallback() == 0)
				do_reset(NULL, 0, 0, NULL);
			return -1;
		}
	}
	return 0;
}

U_BOOT_CMD(
	upgrade_available,	1,	1,	do_upgrade_available,
	"check Siemens update",
	"no parameters"
);
#endif
#endif
