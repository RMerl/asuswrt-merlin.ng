// SPDX-License-Identifier: GPL-2.0+
/*
 * common.c
 *
 * common board functions for B&R boards
 *
 * Copyright (C) 2013 Hannes Schmelzer <oe5hpm@oevsv.at>
 * Bernecker & Rainer Industrieelektronik GmbH - http://www.br-automation.com
 *
 */
#include <log.h>
#include <version.h>
#include <common.h>
#include <env.h>
#include <fdtdec.h>
#include <i2c.h>
#include <asm/global_data.h>
#include <linux/delay.h>
#include "bur_common.h"

DECLARE_GLOBAL_DATA_PTR;

/* --------------------------------------------------------------------------*/

int ft_board_setup(void *blob, struct bd_info *bd)
{
	int nodeoffset;

	nodeoffset = fdt_path_offset(blob, "/factory-settings");
	if (nodeoffset < 0) {
		printf("%s: cannot find /factory-settings, trying /fset\n",
		       __func__);
		nodeoffset = fdt_path_offset(blob, "/fset");
		if (nodeoffset < 0) {
			printf("%s: cannot find /fset.\n", __func__);
			return 0;
		}
	}

	if (fdt_setprop(blob, nodeoffset, "bl-version",
			PLAIN_VERSION, strlen(PLAIN_VERSION)) != 0) {
		printf("%s: no 'bl-version' prop in fdt!\n", __func__);
		return 0;
	}
	return 0;
}

int brdefaultip_setup(int bus, int chip)
{
	int rc;
	struct udevice *i2cdev;
	u8 u8buf = 0;
	char defip[256] = { 0 };

	rc = i2c_get_chip_for_busnum(bus, chip, 2, &i2cdev);
	if (rc != 0) {
		printf("WARN: cannot probe baseboard EEPROM!\n");
		return -1;
	}

	rc = dm_i2c_read(i2cdev, 0, &u8buf, 1);
	if (rc != 0) {
		printf("WARN: cannot read baseboard EEPROM!\n");
		return -1;
	}

	if (u8buf != 0xFF)
		snprintf(defip, sizeof(defip),
			 "if test -r ${ipaddr}; then; else setenv ipaddr 192.168.60.%d; setenv serverip 192.168.60.254; setenv gatewayip 192.168.60.254; setenv netmask 255.255.255.0; fi;",
			 u8buf);
	else
		strncpy(defip,
			"if test -r ${ipaddr}; then; else setenv ipaddr 192.168.60.1; setenv serverip 192.168.60.254; setenv gatewayip 192.168.60.254; setenv netmask 255.255.255.0; fi;",
			sizeof(defip));

	env_set("brdefaultip", defip);
	env_set_hex("board_id", u8buf);

	return 0;
}

int overwrite_console(void)
{
	return 1;
}

#if defined(CONFIG_SPL_BUILD) && defined(CONFIG_AM33XX)
#include <asm/arch/hardware.h>
#include <asm/arch/omap.h>
#include <asm/arch/clock.h>
#include <asm/arch/sys_proto.h>
#include <power/tps65217.h>

static struct ctrl_dev *cdev = (struct ctrl_dev *)CTRL_DEVICE_BASE;

void pmicsetup(u32 mpupll, unsigned int bus)
{
	int mpu_vdd;
	int usb_cur_lim;

	if (power_tps65217_init(bus)) {
		printf("WARN: cannot setup PMIC 0x24 @ bus #%d, not found!.\n",
		       bus);
		return;
	}

	/* Get the frequency which is defined by device fuses */
	dpll_mpu_opp100.m = am335x_get_efuse_mpu_max_freq(cdev);
	printf("detected max. frequency: %d - ", dpll_mpu_opp100.m);

	if (0 != mpupll) {
		dpll_mpu_opp100.m = mpupll;
		printf("retuning MPU-PLL to: %d MHz.\n", dpll_mpu_opp100.m);
	} else {
		puts("ok.\n");
	}
	/*
	 * Increase USB current limit to 1300mA or 1800mA and set
	 * the MPU voltage controller as needed.
	 */
	if (dpll_mpu_opp100.m == MPUPLL_M_1000) {
		usb_cur_lim = TPS65217_USB_INPUT_CUR_LIMIT_1800MA;
		mpu_vdd = TPS65217_DCDC_VOLT_SEL_1325MV;
	} else {
		usb_cur_lim = TPS65217_USB_INPUT_CUR_LIMIT_1300MA;
		mpu_vdd = TPS65217_DCDC_VOLT_SEL_1275MV;
	}

	if (tps65217_reg_write(TPS65217_PROT_LEVEL_NONE, TPS65217_POWER_PATH,
			       usb_cur_lim, TPS65217_USB_INPUT_CUR_LIMIT_MASK))
		puts("tps65217_reg_write failure\n");

	/* Set DCDC3 (CORE) voltage to 1.125V */
	if (tps65217_voltage_update(TPS65217_DEFDCDC3,
				    TPS65217_DCDC_VOLT_SEL_1125MV)) {
		puts("tps65217_voltage_update failure\n");
		return;
	}

	/* Set CORE Frequencies to OPP100 */
	do_setup_dpll(&dpll_core_regs, &dpll_core_opp100);

	/* Set DCDC2 (MPU) voltage */
	if (tps65217_voltage_update(TPS65217_DEFDCDC2, mpu_vdd)) {
		puts("tps65217_voltage_update failure\n");
		return;
	}

	/* Set LDO3 to 1.8V */
	if (tps65217_reg_write(TPS65217_PROT_LEVEL_2,
			       TPS65217_DEFLS1,
			       TPS65217_LDO_VOLTAGE_OUT_1_8,
			       TPS65217_LDO_MASK))
		puts("tps65217_reg_write failure\n");
	/* Set LDO4 to 3.3V */
	if (tps65217_reg_write(TPS65217_PROT_LEVEL_2,
			       TPS65217_DEFLS2,
			       TPS65217_LDO_VOLTAGE_OUT_3_3,
			       TPS65217_LDO_MASK))
		puts("tps65217_reg_write failure\n");

	/* Set MPU Frequency to what we detected now that voltages are set */
	do_setup_dpll(&dpll_mpu_regs, &dpll_mpu_opp100);
	/* Set PWR_EN bit in Status Register */
	tps65217_reg_write(TPS65217_PROT_LEVEL_NONE,
			   TPS65217_STATUS, TPS65217_PWR_OFF, TPS65217_PWR_OFF);
}

void set_uart_mux_conf(void)
{
	enable_uart0_pin_mux();
}

void set_mux_conf_regs(void)
{
	enable_board_pin_mux();
}

#endif /* CONFIG_SPL_BUILD && CONFIG_AM33XX */
