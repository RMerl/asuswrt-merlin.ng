// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2011
 * egnite GmbH <info@egnite.de>
 */

/*
 * Ethernut 5 power management support
 *
 * This board may be supplied via USB, IEEE 802.3af PoE or an
 * auxiliary DC input. An on-board ATmega168 microcontroller,
 * the so called power management controller or PMC, is used
 * to select the supply source and to switch on and off certain
 * energy consuming board components. This allows to reduce the
 * total stand-by consumption to less than 70mW.
 *
 * The main CPU communicates with the PMC via I2C. When
 * CONFIG_CMD_BSP is defined in the board configuration file,
 * then the board specific command 'pwrman' becomes available,
 * which allows to manually deal with the PMC.
 *
 * Two distinct registers are provided by the PMC for enabling
 * and disabling specific features. This avoids the often seen
 * read-modify-write cycle or shadow register requirement.
 * Additional registers are available to query the board
 * status and temperature, the auxiliary voltage and to control
 * the green user LED that is integrated in the reset switch.
 *
 * Note, that the AVR firmware of the PMC is released under BSDL.
 *
 * For additional information visit the project home page at
 * http://www.ethernut.de/
 */
#include <common.h>
#include <command.h>
#include <asm/arch/at91sam9260.h>
#include <asm/arch/at91_common.h>
#include <asm/arch/gpio.h>
#include <asm/io.h>
#include <i2c.h>
#include <linux/delay.h>

#include "ethernut5_pwrman.h"

/* PMC firmware version */
static int pwrman_major;
static int pwrman_minor;

/*
 * Enable Ethernut 5 power management.
 *
 * This function must be called during board initialization.
 * While we are using u-boot's I2C subsystem, it may be required
 * to enable the serial port before calling this function,
 * in particular when debugging is enabled.
 *
 * If board specific commands are not available, we will activate
 * all board components.
 */
void ethernut5_power_init(void)
{
	pwrman_minor = i2c_reg_read(PWRMAN_I2C_ADDR, PWRMAN_REG_VERS);
	pwrman_major = pwrman_minor >> 4;
	pwrman_minor &= 15;

#ifndef CONFIG_CMD_BSP
	/* Do not modify anything, if we do not have a known version. */
	if (pwrman_major == 2) {
		/* Without board specific commands we enable all features. */
		i2c_reg_write(PWRMAN_I2C_ADDR, PWRMAN_REG_ENA, ~PWRMAN_ETHRST);
		i2c_reg_write(PWRMAN_I2C_ADDR, PWRMAN_REG_DIS, PWRMAN_ETHRST);
	}
#endif
}

/*
 * Reset Ethernet PHY.
 *
 * This function allows the re-configure the PHY after
 * changing its strap pins.
 */
void ethernut5_phy_reset(void)
{
	/* Do not modify anything, if we do not have a known version. */
	if (pwrman_major != 2)
		return;

	/*
	 * Make sure that the Ethernet clock is enabled and the PHY reset
	 * is disabled for at least 100 us.
	 */
	i2c_reg_write(PWRMAN_I2C_ADDR, PWRMAN_REG_ENA, PWRMAN_ETHCLK);
	i2c_reg_write(PWRMAN_I2C_ADDR, PWRMAN_REG_DIS, PWRMAN_ETHRST);
	udelay(100);

	/*
	 * LAN8710 strap pins are
	 * PA14 => PHY MODE0
	 * PA15 => PHY MODE1
	 * PA17 => PHY MODE2 => 111b all capable
	 * PA18 => PHY ADDR0 => 0b
	 */
	at91_set_pio_input(AT91_PIO_PORTA, 14, 1);
	at91_set_pio_input(AT91_PIO_PORTA, 15, 1);
	at91_set_pio_input(AT91_PIO_PORTA, 17, 1);
	at91_set_pio_input(AT91_PIO_PORTA, 18, 0);

	/* Activate PHY reset for 100 us. */
	i2c_reg_write(PWRMAN_I2C_ADDR, PWRMAN_REG_ENA, PWRMAN_ETHRST);
	udelay(100);
	i2c_reg_write(PWRMAN_I2C_ADDR, PWRMAN_REG_DIS, PWRMAN_ETHRST);

	at91_set_pio_input(AT91_PIO_PORTA, 14, 1);
}

/*
 * Output the firmware version we got during initialization.
 */
void ethernut5_print_version(void)
{
	printf("%u.%u\n", pwrman_major, pwrman_minor);
}

/*
 * All code below this point is optional and implements
 * the 'pwrman' command.
 */
#ifdef CONFIG_CMD_BSP

/* Human readable names of PMC features */
char *pwrman_feat[8] = {
	"board", "vbin", "vbout", "mmc",
	"rs232", "ethclk", "ethrst", "wakeup"
};

/*
 * Print all feature names, that have its related flags enabled.
 */
static void print_flagged_features(u8 flags)
{
	int i;

	for (i = 0; i < 8; i++) {
		if (flags & (1 << i))
			printf("%s ", pwrman_feat[i]);
	}
}

/*
 * Return flags of a given list of feature names.
 *
 * The function stops at the first unknown list entry and
 * returns the number of detected names as a function result.
 */
static int feature_flags(char * const names[], int num, u8 *flags)
{
	int i, j;

	*flags = 0;
	for (i = 0; i < num; i++) {
		for (j = 0; j < 8; j++) {
			if (strcmp(pwrman_feat[j], names[i]) == 0) {
				*flags |= 1 << j;
				break;
			}
		}
		if (j > 7)
			break;
	}
	return i;
}

void ethernut5_print_power(void)
{
	u8 flags;
	int i;

	flags = i2c_reg_read(PWRMAN_I2C_ADDR, PWRMAN_REG_ENA);
	for (i = 0; i < 2; i++) {
		if (flags) {
			print_flagged_features(flags);
			printf("%s\n", i ? "off" : "on");
		}
		flags = ~flags;
	}
}

void ethernut5_print_celsius(void)
{
	int val;

	/* Read ADC value from LM50 and return Celsius degrees. */
	val = i2c_reg_read(PWRMAN_I2C_ADDR, PWRMAN_REG_TEMP);
	val *= 5000;	/* 100mV/degree with 5V reference */
	val += 128;	/* 8 bit resolution */
	val /= 256;
	val -= 450;	/* Celsius offset, still x10 */
	/* Output full degrees. */
	printf("%d\n", (val + 5) / 10);
}

void ethernut5_print_voltage(void)
{
	int val;

	/* Read ADC value from divider and return voltage. */
	val = i2c_reg_read(PWRMAN_I2C_ADDR, PWRMAN_REG_VAUX);
	/* Resistors are 100k and 12.1k */
	val += 5;
	val *= 180948;
	val /= 100000;
	val++;
	/* Calculation was done in 0.1V units. */
	printf("%d\n", (val + 5) / 10);
}

/*
 * Process the board specific 'pwrman' command.
 */
int do_pwrman(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	u8 val;
	int i;

	if (argc == 1) {
		ethernut5_print_power();
	} else if (argc == 2 && strcmp(argv[1], "reset") == 0) {
		at91_set_pio_output(AT91_PIO_PORTB, 8, 1);
		udelay(100);
		at91_set_pio_output(AT91_PIO_PORTB, 8, 0);
		udelay(100000);
	} else if (argc == 2 && strcmp(argv[1], "temp") == 0) {
		ethernut5_print_celsius();
	} else if (argc == 2 && strcmp(argv[1], "vaux") == 0) {
		ethernut5_print_voltage();
	} else if (argc == 2 && strcmp(argv[1], "version") == 0) {
		ethernut5_print_version();
	} else if (strcmp(argv[1], "led") == 0) {
		/* Control the green status LED. Blink frequency unit
		** is 0.1s, very roughly. */
		if (argc == 2) {
			/* No more arguments, output current settings. */
			val = i2c_reg_read(PWRMAN_I2C_ADDR, PWRMAN_REG_LEDCTL);
			printf("led %u %u\n", val >> 4, val & 15);
		} else {
			/* First argument specifies the on-time. */
			val = (u8) simple_strtoul(argv[2], NULL, 0);
			val <<= 4;
			if (argc > 3) {
				/* Second argument specifies the off-time. */
				val |= (u8) (simple_strtoul(argv[3], NULL, 0)
						& 15);
			}
			/* Update the LED control register. */
			i2c_reg_write(PWRMAN_I2C_ADDR, PWRMAN_REG_LEDCTL, val);
		}
	} else {
		/* We expect a list of features followed an optional status. */
		argc--;
		i = feature_flags(&argv[1], argc, &val);
		if (argc == i) {
			/* We got a list only, print status. */
			val &= i2c_reg_read(PWRMAN_I2C_ADDR, PWRMAN_REG_STA);
			if (val) {
				if (i > 1)
					print_flagged_features(val);
				printf("active\n");
			} else {
				printf("inactive\n");
			}
		} else {
			/* More arguments. */
			if (i == 0) {
				/* No given feature, use despensibles. */
				val = PWRMAN_DISPENSIBLE;
			}
			if (strcmp(argv[i + 1], "on") == 0) {
				/* Enable features. */
				i2c_reg_write(PWRMAN_I2C_ADDR, PWRMAN_REG_ENA,
						val);
			} else if (strcmp(argv[i + 1], "off") == 0) {
				/* Disable features. */
				i2c_reg_write(PWRMAN_I2C_ADDR, PWRMAN_REG_DIS,
						val);
			} else {
				printf("Bad parameter %s\n", argv[i + 1]);
				return 1;
			}
		}
	}
	return 0;
}

U_BOOT_CMD(
	pwrman,	CONFIG_SYS_MAXARGS, 1, do_pwrman,
	"power management",
		   "- print settings\n"
	"pwrman feature ...\n"
	"       - print status\n"
	"pwrman [feature ...] on|off\n"
	"       - enable/disable specified or all dispensible features\n"
	"pwrman led [on-time [off-time]]\n"
	"       - print or set led blink timer\n"
	"pwrman temp\n"
	"       - print board temperature (Celsius)\n"
	"pwrman vaux\n"
	"       - print auxiliary input voltage\n"
	"pwrman reset\n"
	"       - reset power management controller\n"
	"pwrman version\n"
	"       - print firmware version\n"
	"\n"
	"        features, (*)=dispensible:\n"
	"          board  - 1.8V and 3.3V supply\n"
	"          vbin   - supply via USB device connector\n"
	"          vbout  - USB host connector supply(*)\n"
	"          mmc    - MMC slot supply(*)\n"
	"          rs232  - RS232 driver\n"
	"          ethclk - Ethernet PHY clock(*)\n"
	"          ethrst - Ethernet PHY reset\n"
	"          wakeup - RTC alarm"
);
#endif /* CONFIG_CMD_BSP */
