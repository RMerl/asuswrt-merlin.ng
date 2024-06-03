// SPDX-License-Identifier: GPL-2.0+
/*
 * Common board functions for siemens AM335X based boards
 * (C) Copyright 2013 Siemens Schweiz AG
 * (C) Heiko Schocher, DENX Software Engineering, hs@denx.de.
 *
 * Based on:
 * U-Boot file:/board/ti/am335x/board.c
 * Copyright (C) 2011, Texas Instruments, Incorporated - http://www.ti.com/
 */

#include <common.h>
#include <errno.h>
#include <spl.h>
#include <asm/arch/cpu.h>
#include <asm/arch/hardware.h>
#include <asm/arch/omap.h>
#include <asm/arch/ddr_defs.h>
#include <asm/arch/clock.h>
#include <asm/arch/gpio.h>
#include <asm/arch/mmc_host_def.h>
#include <asm/arch/sys_proto.h>
#include <asm/io.h>
#include <asm/emif.h>
#include <asm/gpio.h>
#include <i2c.h>
#include <miiphy.h>
#include <cpsw.h>
#include <watchdog.h>
#include <asm/mach-types.h>
#include "../common/factoryset.h"

DECLARE_GLOBAL_DATA_PTR;

#ifdef CONFIG_SPL_BUILD
void set_uart_mux_conf(void)
{
	enable_uart0_pin_mux();
}

void set_mux_conf_regs(void)
{
	/* Initalize the board header */
	enable_i2c0_pin_mux();
	i2c_set_bus_num(0);

	/* enable early the console */
	gd->baudrate = CONFIG_BAUDRATE;
	serial_init();
	gd->have_console = 1;
	if (read_eeprom() < 0)
		puts("Could not get board ID.\n");

	enable_board_pin_mux();
}

void sdram_init(void)
{
	spl_siemens_board_init();
	board_init_ddr();

	return;
}
#endif /* #ifdef CONFIG_SPL_BUILD */

#ifndef CONFIG_SPL_BUILD
/*
 * Basic board specific setup.  Pinmux has been handled already.
 */
int board_init(void)
{
#if defined(CONFIG_HW_WATCHDOG)
	hw_watchdog_init();
#endif /* defined(CONFIG_HW_WATCHDOG) */
	i2c_set_bus_num(0);
	if (read_eeprom() < 0)
		puts("Could not get board ID.\n");
#ifdef CONFIG_MACH_TYPE
	gd->bd->bi_arch_number = CONFIG_MACH_TYPE;
#endif
	gd->bd->bi_boot_params = CONFIG_SYS_SDRAM_BASE + 0x100;

#ifdef CONFIG_FACTORYSET
	factoryset_read_eeprom(CONFIG_SYS_I2C_EEPROM_ADDR);
#endif

	gpmc_init();

#ifdef CONFIG_NAND_CS_INIT
	board_nand_cs_init();
#endif
#ifdef CONFIG_VIDEO
	board_video_init();
#endif

	return 0;
}
#endif /* #ifndef CONFIG_SPL_BUILD */

#define OSC	(V_OSCK/1000000)
const struct dpll_params dpll_ddr = {
		DDR_PLL_FREQ, OSC-1, 1, -1, -1, -1, -1};

const struct dpll_params *get_dpll_ddr_params(void)
{
	return &dpll_ddr;
}

#ifndef CONFIG_SPL_BUILD

#define MAX_NR_LEDS	10
#define MAX_PIN_NUMBER	128
#define STARTUP	0

#if defined(BOARD_DFU_BUTTON_GPIO)
unsigned char get_button_state(char * const envname, unsigned char def)
{
	int button = 0;
	int gpio;
	char *ptr_env;

	/* If button is not found we take default */
	ptr_env = env_get(envname);
	if (NULL == ptr_env) {
		gpio = def;
	} else {
		gpio = (unsigned char)simple_strtoul(ptr_env, NULL, 0);
		if (gpio > MAX_PIN_NUMBER)
			gpio = def;
	}

	gpio_request(gpio, "");
	gpio_direction_input(gpio);
	if (gpio_get_value(gpio))
		button = 1;
	else
		button = 0;

	gpio_free(gpio);

	return button;
}
/**
 * This command returns the status of the user button on
 * Input - none
 * Returns -	1 if button is held down
 *		0 if button is not held down
 */
static int
do_userbutton(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int button = 0;
	button = get_button_state("button_dfu0", BOARD_DFU_BUTTON_GPIO);
	button |= get_button_state("button_dfu1", BOARD_DFU_BUTTON_GPIO);
	return button;
}

U_BOOT_CMD(
	dfubutton, CONFIG_SYS_MAXARGS, 1, do_userbutton,
	"Return the status of the DFU button",
	""
);
#endif

static int
do_usertestwdt(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	printf("\n\n\n Go into infinite loop\n\n\n");
	while (1)
		;
	return 0;
};

U_BOOT_CMD(
	testwdt, CONFIG_SYS_MAXARGS, 1,	do_usertestwdt,
	"Sends U-Boot into infinite loop",
	""
);

/**
 * Get led gpios from env and set them.
 * The led define in environment need to need to be of the form ledN=NN,S0,S1
 * where N is an unsigned integer from 0 to 9 and S0 and S1 is 0 or 1. S0
 * defines the startup state of the led, S1 the special state of the led when
 * it enters e.g. dfu mode.
 */
void set_env_gpios(unsigned char state)
{
	char *ptr_env;
	char str_tmp[5];	/* must contain "ledX"*/
	char num[1];
	unsigned char i, idx, pos1, pos2, ccount;
	unsigned char gpio_n, gpio_s0, gpio_s1;

	for (i = 0; i < MAX_NR_LEDS; i++) {
		strcpy(str_tmp, "led");
		sprintf(num, "%d", i);
		strcat(str_tmp, num);

		/* If env var is not found we stop */
		ptr_env = env_get(str_tmp);
		if (NULL == ptr_env)
			break;

		/* Find sperators position */
		pos1 = 0;
		pos2 = 0;
		ccount = 0;
		for (idx = 0; ptr_env[idx] != '\0'; idx++) {
			if (ptr_env[idx] == ',') {
				if (ccount++ < 1)
					pos1 = idx;
				else
					pos2 = idx;
			}
		}
		/* Bad led description skip this definition */
		if (pos2 <= pos1 || ccount > 2)
			continue;

		/* Get pin number and request gpio */
		memset(str_tmp, 0, sizeof(str_tmp));
		strncpy(str_tmp, ptr_env, pos1*sizeof(char));
		gpio_n = (unsigned char)simple_strtoul(str_tmp, NULL, 0);

		/* Invalid gpio number skip definition */
		if (gpio_n > MAX_PIN_NUMBER)
			continue;

		gpio_request(gpio_n, "");

		if (state == STARTUP) {
			/* get pin state 0 and set */
			memset(str_tmp, 0, sizeof(str_tmp));
			strncpy(str_tmp, ptr_env+pos1+1,
				(pos2-pos1-1)*sizeof(char));
			gpio_s0 = (unsigned char)simple_strtoul(str_tmp, NULL,
								0);

			gpio_direction_output(gpio_n, gpio_s0);

		} else {
			/* get pin state 1 and set */
			memset(str_tmp, 0, sizeof(str_tmp));
			strcpy(str_tmp, ptr_env+pos2+1);
			gpio_s1 = (unsigned char)simple_strtoul(str_tmp, NULL,
								0);
			gpio_direction_output(gpio_n, gpio_s1);
		}
	} /* loop through defined led in environment */
}

static int do_board_led(cmd_tbl_t *cmdtp, int flag, int argc,
			   char *const argv[])
{
	if (argc != 2)
		return CMD_RET_USAGE;
	if ((unsigned char)simple_strtoul(argv[1], NULL, 0) == STARTUP)
		set_env_gpios(0);
	else
		set_env_gpios(1);
	return 0;
};

U_BOOT_CMD(
	draco_led, CONFIG_SYS_MAXARGS, 2,	do_board_led,
	"Set LEDs defined in environment",
	"<0|1>"
);
#endif /* !CONFIG_SPL_BUILD */
