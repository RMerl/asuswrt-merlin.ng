// SPDX-License-Identifier: GPL-2.0+
/*
 * common reset-controller functions for B&R boards
 *
 * Copyright (C) 2019 Hannes Schmelzer <oe5hpm@oevsv.at>
 * B&R Industrial Automation GmbH - http://www.br-automation.com/ *
 */
#include <common.h>
#include <errno.h>
#include <i2c.h>
#include <dm/uclass.h>
#include "br_resetc.h"

/* I2C Address of controller */
#define	RSTCTRL_ADDR_PSOC	0x75
#define	RSTCTRL_ADDR_STM32	0x60

#define BMODE_DEFAULTAR		0
#define BMODE_SERVICE		2
#define BMODE_RUN		4
#define BMODE_PME		12
#define BMODE_DIAG		15

#ifdef CONFIG_LCD
#include <lcd.h>
#define LCD_SETCURSOR(x, y)	lcd_position_cursor(x, y)
#define LCD_PUTS(x)		lcd_puts(x)
#else
#define LCD_SETCURSOR(x, y)
#define LCD_PUTS(x)
#endif /* CONFIG_LCD */

static const char *bootmodeascii[16] = {
	"BOOT",		"reserved",	"reserved",	"reserved",
	"RUN",		"reserved",	"reserved",	"reserved",
	"reserved",	"reserved",	"reserved",	"reserved",
	"PME",		"reserved",	"reserved",	"DIAG",
};

struct br_reset_t {
	struct udevice *i2cdev;
	u8 is_psoc;
};

static struct br_reset_t resetc;

__weak int board_boot_key(void)
{
	return 0;
}

__weak void board_boot_led(unsigned int on)
{
}

static int resetc_init(void)
{
	struct udevice *i2cbus;
	int rc;

	rc = uclass_get_device_by_seq(UCLASS_I2C, 0, &i2cbus);
	if (rc) {
		printf("Cannot find I2C bus #0!\n");
		return -1;
	}

	resetc.is_psoc = 1;
	rc = dm_i2c_probe(i2cbus,
			  RSTCTRL_ADDR_PSOC, 0, &resetc.i2cdev);
	if (rc) {
		resetc.is_psoc = 0;
		rc = dm_i2c_probe(i2cbus,
				  RSTCTRL_ADDR_STM32, 0, &resetc.i2cdev);
	}

	if (rc)
		printf("Warning: cannot probe BuR resetcontroller!\n");

	return rc;
}

int br_resetc_regget(u8 reg, u8 *dst)
{
	int rc = 0;

	if (!resetc.i2cdev)
		rc = resetc_init();

	if (rc != 0)
		return rc;

	return dm_i2c_read(resetc.i2cdev, reg, dst, 1);
}

int br_resetc_regset(u8 reg, u8 val)
{
	int rc = 0;
	u16 regw = (val << 8) | val;

	if (!resetc.i2cdev)
		rc = resetc_init();

	if (rc != 0)
		return rc;

	if (resetc.is_psoc)
		return dm_i2c_write(resetc.i2cdev, reg, (u8 *)&regw, 2);

	return dm_i2c_write(resetc.i2cdev, reg, (u8 *)&regw, 1);
}

int br_resetc_bmode(void)
{
	int rc = 0;
	u16 regw;
	u8 regb, scr;
	int cnt;
	unsigned int bmode = 0;

	if (!resetc.i2cdev)
		rc = resetc_init();

	if (rc != 0)
		return rc;

	rc = dm_i2c_read(resetc.i2cdev, RSTCTRL_ENHSTATUS, &regb, 1);
	if (rc != 0) {
		printf("WARN: cannot read ENHSTATUS from resetcontroller!\n");
		return -1;
	}

	rc = dm_i2c_read(resetc.i2cdev, RSTCTRL_SCRATCHREG0, &scr, 1);
	if (rc != 0) {
		printf("WARN: cannot read SCRATCHREG from resetcontroller!\n");
		return -1;
	}

	board_boot_led(1);

	/* special bootmode from resetcontroller */
	if (regb & 0x4) {
		bmode = BMODE_DIAG;
	} else if (regb & 0x8) {
		bmode = BMODE_DEFAULTAR;
	} else if (board_boot_key() != 0) {
		cnt = 4;
		do {
			LCD_SETCURSOR(1, 8);
			switch (cnt) {
			case 4:
				LCD_PUTS
				("release KEY to enter SERVICE-mode.     ");
				break;
			case 3:
				LCD_PUTS
				("release KEY to enter DIAGNOSE-mode.    ");
				break;
			case 2:
				LCD_PUTS
				("release KEY to enter BOOT-mode.        ");
				break;
			}
			mdelay(1000);
			cnt--;
			if (board_boot_key() == 0)
				break;
		} while (cnt);

		switch (cnt) {
		case 0:
			bmode = BMODE_PME;
			break;
		case 1:
			bmode = BMODE_DEFAULTAR;
			break;
		case 2:
			bmode = BMODE_DIAG;
			break;
		case 3:
			bmode = BMODE_SERVICE;
			break;
		}
	} else if ((regb & 0x1) || scr == 0xCC) {
		bmode = BMODE_PME;
	} else {
		bmode = BMODE_RUN;
	}

	LCD_SETCURSOR(1, 8);

	switch (bmode) {
	case BMODE_PME:
		LCD_PUTS("entering PME-Mode (netscript).         ");
		regw = 0x0C0C;
		break;
	case BMODE_DEFAULTAR:
		LCD_PUTS("entering BOOT-mode.                    ");
		regw = 0x0000;
		break;
	case BMODE_DIAG:
		LCD_PUTS("entering DIAGNOSE-mode.                ");
		regw = 0x0F0F;
		break;
	case BMODE_SERVICE:
		LCD_PUTS("entering SERVICE mode.                 ");
		regw = 0xB4B4;
		break;
	case BMODE_RUN:
		LCD_PUTS("loading OS...                          ");
		regw = 0x0404;
		break;
	}

	board_boot_led(0);

	if (resetc.is_psoc)
		rc = dm_i2c_write(resetc.i2cdev, RSTCTRL_SCRATCHREG0,
				  (u8 *)&regw, 2);
	else
		rc = dm_i2c_write(resetc.i2cdev, RSTCTRL_SCRATCHREG0,
				  (u8 *)&regw, 1);

	if (rc != 0)
		printf("WARN: cannot write into resetcontroller!\n");

	if (resetc.is_psoc)
		printf("Reset: PSOC controller\n");
	else
		printf("Reset: STM32 controller\n");

	printf("Mode:  %s\n", bootmodeascii[regw & 0x0F]);
	env_set_ulong("b_mode", regw & 0x0F);

	return rc;
}
