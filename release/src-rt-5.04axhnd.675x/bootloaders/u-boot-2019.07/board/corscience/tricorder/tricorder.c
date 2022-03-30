// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2012
 * Corscience GmbH & Co. KG, <www.corscience.de>
 * Thomas Weber <weber@corscience.de>
 * Sunil Kumar <sunilsaini05@gmail.com>
 * Shashi Ranjan <shashiranjanmca05@gmail.com>
 *
 * Derived from Devkit8000 code by
 * Frederik Kriewitz <frederik@kriewitz.eu>
 */
#include <common.h>
#include <twl4030.h>
#include <asm/io.h>
#include <asm/gpio.h>
#include <asm/arch/mmc_host_def.h>
#include <asm/arch/mux.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/mem.h>
#include "tricorder.h"
#include "tricorder-eeprom.h"

DECLARE_GLOBAL_DATA_PTR;

/*
 * Routine: board_init
 * Description: Early hardware init.
 */
int board_init(void)
{
	gpmc_init(); /* in SRAM or SDRAM, finish GPMC */
	/* boot param addr */
	gd->bd->bi_boot_params = (OMAP34XX_SDRC_CS0 + 0x100);

	return 0;
}

/**
 * get_eeprom - read the eeprom
 *
 * @eeprom - pointer to a eeprom struct to fill
 *
 * This function will panic() on wrong EEPROM content
 */
static void get_eeprom(struct tricorder_eeprom *eeprom)
{
	int ret;

	if (!eeprom)
		panic("No eeprom given!\n");

	ret = gpio_request(7, "BMS");
	if (ret)
		panic("gpio: requesting BMS pin failed\n");

	ret = gpio_direction_input(7);
	if (ret)
		panic("gpio: set BMS as input failed\n");

	ret = gpio_get_value(7);
	if (ret < 0)
		panic("gpio: get BMS pin state failed\n");

	gpio_free(7);

	if (ret == 0) {
		/* BMS is _not_ set, do the EEPROM check */
		ret = tricorder_get_eeprom(0x51, eeprom);
		if (!ret) {
			if (strncmp(eeprom->board_name, "CS10411", 7) != 0)
				panic("Wrong board name '%.*s'\n",
				      sizeof(eeprom->board_name),
						eeprom->board_name);
			if (eeprom->board_version[0] < 'D')
				panic("Wrong board version '%.*s'\n",
				      sizeof(eeprom->board_version),
						eeprom->board_version);
		} else {
			panic("Could not get board revision\n");
		}
	} else {
		memset(eeprom, 0, TRICORDER_EEPROM_SIZE);
	}
}

/**
 * print_hwversion - print out a HW version string
 *
 * @eeprom - pointer to the eeprom
 */
static void print_hwversion(struct tricorder_eeprom *eeprom)
{
	size_t len;
	if (!eeprom)
		panic("No eeprom given!");

	printf("Board %.*s:%.*s serial %.*s",
	       sizeof(eeprom->board_name), eeprom->board_name,
	       sizeof(eeprom->board_version), eeprom->board_version,
	       sizeof(eeprom->board_serial), eeprom->board_serial);

	len = strnlen(eeprom->interface_version,
		      sizeof(eeprom->interface_version));
	if (len > 0)
		printf(" HW interface version %.*s",
		       sizeof(eeprom->interface_version),
		       eeprom->interface_version);
	puts("\n");
}

/*
 * Routine: misc_init_r
 * Description: Configure board specific parts
 */
int misc_init_r(void)
{
	struct tricorder_eeprom eeprom;
	get_eeprom(&eeprom);
	print_hwversion(&eeprom);

	twl4030_power_init();
	status_led_set(0, CONFIG_LED_STATUS_ON);
	status_led_set(1, CONFIG_LED_STATUS_ON);
	status_led_set(2, CONFIG_LED_STATUS_ON);

	omap_die_id_display();

	return 0;
}

/*
 * Routine: set_muxconf_regs
 * Description: Setting up the configuration Mux registers specific to the
 *		hardware. Many pins need to be moved from protect to primary
 *		mode.
 */
void set_muxconf_regs(void)
{
	MUX_TRICORDER();
}

#if defined(CONFIG_MMC)
int board_mmc_init(bd_t *bis)
{
	return omap_mmc_init(0, 0, 0, -1, -1);
}
#endif

#if defined(CONFIG_MMC)
void board_mmc_power_init(void)
{
	twl4030_power_mmc_init(0);
}
#endif

/*
 * Routine: get_board_mem_timings
 * Description: If we use SPL then there is no x-loader nor config header
 * so we have to setup the DDR timings ourself on the first bank.  This
 * provides the timing values back to the function that configures
 * the memory.  We have either one or two banks of 128MB DDR.
 */
void get_board_mem_timings(struct board_sdrc_timings *timings)
{
	struct tricorder_eeprom eeprom;
	get_eeprom(&eeprom);

	/* General SDRC config */
	if (eeprom.board_version[0] > 'D') {
		/* use optimized timings for our SDRAM device */
		timings->mcfg = MCFG((256 << 20), 14);
#define MT46H64M32_TDAL  6	/* Twr/Tck + Trp/tck		*/
				/* 15/6 + 18/6 = 5.5 -> 6	*/
#define MT46H64M32_TDPL  3	/* 15/6 = 2.5 -> 3 (Twr)	*/
#define MT46H64M32_TRRD  2	/* 12/6 = 2			*/
#define MT46H64M32_TRCD  3	/* 18/6 = 3			*/
#define MT46H64M32_TRP   3	/* 18/6 = 3			*/
#define MT46H64M32_TRAS  7	/* 42/6 = 7			*/
#define MT46H64M32_TRC  10	/* 60/6 = 10			*/
#define MT46H64M32_TRFC 12	/* 72/6 = 12			*/
		timings->ctrla = ACTIM_CTRLA(MT46H64M32_TRFC, MT46H64M32_TRC,
					     MT46H64M32_TRAS, MT46H64M32_TRP,
					     MT46H64M32_TRCD, MT46H64M32_TRRD,
					     MT46H64M32_TDPL,
					     MT46H64M32_TDAL);

#define MT46H64M32_TWTR 1
#define MT46H64M32_TCKE 1
#define MT46H64M32_XSR 19	/* 112.5/6 = 18.75 => ~19	*/
#define MT46H64M32_TXP 1
		timings->ctrlb = ACTIM_CTRLB(MT46H64M32_TWTR, MT46H64M32_TCKE,
					     MT46H64M32_TXP, MT46H64M32_XSR);

		timings->mr = MICRON_V_MR_165;
		timings->rfr_ctrl = SDP_3430_SDRC_RFR_CTRL_165MHz;
	} else {
		/* use conservative beagleboard timings as default */
		timings->mcfg = MICRON_V_MCFG_165(128 << 20);
		timings->ctrla = MICRON_V_ACTIMA_165;
		timings->ctrlb = MICRON_V_ACTIMB_165;
		timings->mr = MICRON_V_MR_165;
		timings->rfr_ctrl = SDP_3430_SDRC_RFR_CTRL_165MHz;
	}
}
