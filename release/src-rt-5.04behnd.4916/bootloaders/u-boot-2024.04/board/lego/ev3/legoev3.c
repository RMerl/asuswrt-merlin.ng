// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2016 David Lechner <david@lechnology.com>
 *
 * Based on da850evm.c
 *
 * Copyright (C) 2010 Texas Instruments Incorporated - https://www.ti.com/
 *
 * Based on da830evm.c. Original Copyrights follow:
 *
 * Copyright (C) 2009 Nick Thompson, GE Fanuc, Ltd. <nick.thompson@gefanuc.com>
 * Copyright (C) 2007 Sergey Kubushyn <ksi@koi8.net>
 */

#include <common.h>
#include <env.h>
#include <i2c.h>
#include <init.h>
#include <spi.h>
#include <spi_flash.h>
#include <asm/arch/hardware.h>
#include <asm/arch/pinmux_defs.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/arch/davinci_misc.h>
#include <linux/errno.h>
#include <hwconfig.h>
#include <asm/mach-types.h>
#include <asm/setup.h>
#include <dm/uclass.h>

DECLARE_GLOBAL_DATA_PTR;

#define EEPROM_I2C_ADDR		0x50
#define EEPROM_REV_OFFSET	0x3F00
#define EEPROM_BDADDR_OFFSET	0x3F06

const struct pinmux_resource pinmuxes[] = {
	PINMUX_ITEM(spi0_pins_base),
	PINMUX_ITEM(spi0_pins_scs0),
	PINMUX_ITEM(uart1_pins_txrx),
	PINMUX_ITEM(i2c0_pins),
	PINMUX_ITEM(mmc0_pins),
};

const int pinmuxes_size = ARRAY_SIZE(pinmuxes);

const struct lpsc_resource lpsc[] = {
	{ DAVINCI_LPSC_SPI0 },	/* Serial Flash */
	{ DAVINCI_LPSC_UART1 },	/* console */
	{ DAVINCI_LPSC_MMC_SD },
};

const int lpsc_size = ARRAY_SIZE(lpsc);

/*
 * The Bluetooth address serves as the board serial number.
 */
static void setup_serial_number(void)
{
	struct udevice *idev, *ibus;
	int ret;
	u32 offset;
	char serial_number[13];
	u8 buf[6];
	u8 eeprom_rev;

	if (env_get("serial#"))
		return;

	ret = uclass_get_device_by_seq(UCLASS_I2C, 0, &ibus);
	if (ret)
		return;

	ret = dm_i2c_probe(ibus, EEPROM_I2C_ADDR, 0, &idev);
	if (ret)
		return;

	if (dm_i2c_read(idev, EEPROM_REV_OFFSET, buf, 2)) {
		printf("\nEEPROM revision read failed!\n");
		return;
	}

	/*
	 * EEPROM rev 3 has Bluetooth address at EEPROM_REV_OFFSET.
	 * Other revisions have checksum at EEPROM_REV_OFFSET+1
	 * to detect this.
	 */
	if ((buf[0] ^ buf[1]) == 0xFF)
		eeprom_rev = buf[0];
	else
		eeprom_rev = 3;

	/* EEPROM rev 3 has Bluetooth address where rev should be */
	offset = (eeprom_rev == 3) ? EEPROM_REV_OFFSET : EEPROM_BDADDR_OFFSET;

	if (dm_i2c_read(idev, offset, buf, 6)) {
		printf("\nEEPROM serial read failed!\n");
		return;
	}

	sprintf(serial_number, "%02X%02X%02X%02X%02X%02X",
		buf[0], buf[1], buf[2], buf[3], buf[4], buf[5]);

	env_set("serial#", serial_number);
}

int board_early_init_f(void)
{
	/* enable the console UART */
	writel((DAVINCI_UART_PWREMU_MGMT_FREE | DAVINCI_UART_PWREMU_MGMT_URRST |
		DAVINCI_UART_PWREMU_MGMT_UTRST),
	       &davinci_uart1_ctrl_regs->pwremu_mgmt);

	/*
	 * Power on required peripherals
	 * ARM does not have access by default to PSC0 and PSC1
	 * assuming here that the DSP bootloader has set the IOPU
	 * such that PSC access is available to ARM
	 */
	if (da8xx_configure_lpsc_items(lpsc, ARRAY_SIZE(lpsc)))
		return 1;

	return 0;
}

int board_init(void)
{
	irq_init();

	/* address of boot parameters */
	gd->bd->bi_boot_params = LINUX_BOOT_PARAM_ADDR;

	/* setup the SUSPSRC for ARM to control emulation suspend */
	writel(readl(&davinci_syscfg_regs->suspsrc) &
	       ~(DAVINCI_SYSCFG_SUSPSRC_I2C |
		 DAVINCI_SYSCFG_SUSPSRC_SPI0 | DAVINCI_SYSCFG_SUSPSRC_TIMER0 |
		 DAVINCI_SYSCFG_SUSPSRC_UART1),
	       &davinci_syscfg_regs->suspsrc);

	/* configure pinmux settings */
	if (davinci_configure_pin_mux_items(pinmuxes, ARRAY_SIZE(pinmuxes)))
		return 1;

	return 0;
}

int board_late_init(void)
{
	setup_serial_number();

	return 0;
}
