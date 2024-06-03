// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2013 Freescale Semiconductor, Inc.
 * Author: Shaveta Leekha <shaveta@freescale.com>
 */

#include "idt8t49n222a_serdes_clk.h"

#define DEVICE_ID_REG		0x00

static int check_pll_status(u8 idt_addr)
{
	u8 val = 0;
	int ret;

	ret = i2c_read(idt_addr, 0x17, 1, &val, 1);
	if (ret < 0) {
		printf("IDT:0x%x could not read status register from device.\n",
			idt_addr);
		return ret;
	}

	if (val & 0x04) {
		debug("idt8t49n222a PLL is LOCKED: %x\n", val);
	} else {
		printf("idt8t49n222a PLL is not LOCKED: %x\n", val);
		return -1;
	}

	return 0;
}

int set_serdes_refclk(u8 idt_addr, u8 serdes_num,
			enum serdes_refclk refclk1,
			enum serdes_refclk refclk2, u8 feedback)
{
	u8 dev_id = 0;
	int i, ret;

	debug("IDT:Configuring idt8t49n222a device at I2C address: 0x%2x\n",
		idt_addr);

	ret = i2c_read(idt_addr, DEVICE_ID_REG, 1, &dev_id, 1);
	if (ret < 0) {
		debug("IDT:0x%x could not read DEV_ID from device.\n",
			idt_addr);
		return ret;
	}

	if ((dev_id != 0x00) && (dev_id != 0x24) && (dev_id != 0x2a)) {
		debug("IDT: device at address 0x%x is not idt8t49n222a.\n",
			idt_addr);
	}

	if (serdes_num != 1 && serdes_num != 2) {
		debug("serdes_num should be 1 for SerDes1 and"
			" 2 for SerDes2.\n");
		return -1;
	}

	if ((refclk1 == SERDES_REFCLK_122_88 && refclk2 != SERDES_REFCLK_122_88)
		|| (refclk1 != SERDES_REFCLK_122_88
			&& refclk2 == SERDES_REFCLK_122_88)) {
		debug("Only one refclk at 122.88MHz is not supported."
			" Please set both refclk1 & refclk2 to 122.88MHz"
			" or both not to 122.88MHz.\n");
		return -1;
	}

	if (refclk1 != SERDES_REFCLK_100 && refclk1 != SERDES_REFCLK_122_88
					&& refclk1 != SERDES_REFCLK_125
					&& refclk1 != SERDES_REFCLK_156_25) {
		debug("refclk1 should be 100MHZ, 122.88MHz, 125MHz"
			" or 156.25MHz.\n");
		return -1;
	}

	if (refclk2 != SERDES_REFCLK_100 && refclk2 != SERDES_REFCLK_122_88
					&& refclk2 != SERDES_REFCLK_125
					&& refclk2 != SERDES_REFCLK_156_25) {
		debug("refclk2 should be 100MHZ, 122.88MHz, 125MHz"
			" or 156.25MHz.\n");
		return -1;
	}

	if (feedback != 0 && feedback != 1) {
		debug("valid values for feedback are 0(default) or 1.\n");
		return -1;
	}

	/* Configuring IDT for output refclks as
	 * Refclk1 = 122.88MHz  Refclk2 = 122.88MHz
	 */
	if (refclk1 == SERDES_REFCLK_122_88 &&
			refclk2 == SERDES_REFCLK_122_88) {
		printf("Setting refclk1:122.88 and refclk2:122.88\n");
		for (i = 0; i < NUM_IDT_REGS; i++)
			i2c_reg_write(idt_addr, idt_conf_122_88[i][0],
						idt_conf_122_88[i][1]);

		if (feedback) {
			for (i = 0; i < NUM_IDT_REGS_FEEDBACK; i++)
				i2c_reg_write(idt_addr,
					idt_conf_122_88_feedback[i][0],
					idt_conf_122_88_feedback[i][1]);
		}
	}

	if (refclk1 != SERDES_REFCLK_122_88 &&
			refclk2 != SERDES_REFCLK_122_88) {
		for (i = 0; i < NUM_IDT_REGS; i++)
			i2c_reg_write(idt_addr, idt_conf_not_122_88[i][0],
						idt_conf_not_122_88[i][1]);
	}

	/* Configuring IDT for output refclks as
	 * Refclk1 = 100MHz  Refclk2 = 125MHz
	 */
	if (refclk1 == SERDES_REFCLK_100 && refclk2 == SERDES_REFCLK_125) {
		printf("Setting refclk1:100 and refclk2:125\n");
		i2c_reg_write(idt_addr, 0x11, 0x10);
	}

	/* Configuring IDT for output refclks as
	 * Refclk1 = 125MHz  Refclk2 = 125MHz
	 */
	if (refclk1 == SERDES_REFCLK_125 && refclk2 == SERDES_REFCLK_125) {
		printf("Setting refclk1:125 and refclk2:125\n");
		i2c_reg_write(idt_addr, 0x10, 0x10);
		i2c_reg_write(idt_addr, 0x11, 0x10);
	}

	/* Configuring IDT for output refclks as
	 * Refclk1 = 125MHz  Refclk2 = 100MHz
	 */
	if (refclk1 == SERDES_REFCLK_125 && refclk2 == SERDES_REFCLK_100) {
		printf("Setting refclk1:125 and refclk2:100\n");
		i2c_reg_write(idt_addr, 0x10, 0x10);
	}

	/* Configuring IDT for output refclks as
	 * Refclk1 = 156.25MHz  Refclk2 = 156.25MHz
	 */
	if (refclk1 == SERDES_REFCLK_156_25 &&
			refclk2 == SERDES_REFCLK_156_25) {
		printf("Setting refclk1:156.25 and refclk2:156.25\n");
		for (i = 0; i < NUM_IDT_REGS_156_25; i++)
			i2c_reg_write(idt_addr, idt_conf_156_25[i][0],
						idt_conf_156_25[i][1]);
	}

	/* Configuring IDT for output refclks as
	 * Refclk1 = 100MHz  Refclk2 = 156.25MHz
	 */
	if (refclk1 == SERDES_REFCLK_100 &&
			refclk2 == SERDES_REFCLK_156_25) {
		printf("Setting refclk1:100 and refclk2:156.25\n");
		for (i = 0; i < NUM_IDT_REGS_156_25; i++)
			i2c_reg_write(idt_addr, idt_conf_100_156_25[i][0],
						idt_conf_100_156_25[i][1]);
	}

	/* Configuring IDT for output refclks as
	 * Refclk1 = 125MHz  Refclk2 = 156.25MHz
	 */
	if (refclk1 == SERDES_REFCLK_125 &&
			refclk2 == SERDES_REFCLK_156_25) {
		printf("Setting refclk1:125 and refclk2:156.25\n");
		for (i = 0; i < NUM_IDT_REGS_156_25; i++)
			i2c_reg_write(idt_addr, idt_conf_125_156_25[i][0],
						idt_conf_125_156_25[i][1]);
	}

	/* Configuring IDT for output refclks as
	 * Refclk1 = 156.25MHz  Refclk2 = 100MHz
	 */
	if (refclk1 == SERDES_REFCLK_156_25 &&
			refclk2 == SERDES_REFCLK_100) {
		printf("Setting refclk1:156.25 and refclk2:100\n");
		for (i = 0; i < NUM_IDT_REGS_156_25; i++)
			i2c_reg_write(idt_addr, idt_conf_156_25_100[i][0],
						idt_conf_156_25_100[i][1]);
	}

	/* Configuring IDT for output refclks as
	 * Refclk1 = 156.25MHz  Refclk2 = 125MHz
	 */
	if (refclk1 == SERDES_REFCLK_156_25 &&
			refclk2 == SERDES_REFCLK_125) {
		printf("Setting refclk1:156.25 and refclk2:125\n");
		for (i = 0; i < NUM_IDT_REGS_156_25; i++)
			i2c_reg_write(idt_addr, idt_conf_156_25_125[i][0],
						idt_conf_156_25_125[i][1]);
	}

	/* waiting for maximum of 1 second if PLL doesn'r get locked
	 * initially. then check the status again.
	 */
	if (check_pll_status(idt_addr)) {
		mdelay(1000);
		if (check_pll_status(idt_addr))
			return -1;
	}

	return 0;
}
