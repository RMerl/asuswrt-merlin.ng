/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2015 Google, Inc
 */

#ifndef __S5M8767_H_
#define __S5M8767_H_

enum s5m8767_regnum {
	S5M8767_BUCK1 = 0,
	S5M8767_BUCK2,
	S5M8767_BUCK3,
	S5M8767_BUCK4,
	S5M8767_BUCK5,
	S5M8767_BUCK6,
	S5M8767_BUCK7,
	S5M8767_BUCK8,
	S5M8767_BUCK9,
	S5M8767_LDO1,
	S5M8767_LDO2,
	S5M8767_LDO3,
	S5M8767_LDO4,
	S5M8767_LDO5,
	S5M8767_LDO6,
	S5M8767_LDO7,
	S5M8767_LDO8,
	S5M8767_LDO9,
	S5M8767_LDO10,
	S5M8767_LDO11,
	S5M8767_LDO12,
	S5M8767_LDO13,
	S5M8767_LDO14,
	S5M8767_LDO15,
	S5M8767_LDO16,
	S5M8767_LDO17,
	S5M8767_LDO18,
	S5M8767_LDO19,
	S5M8767_LDO20,
	S5M8767_LDO21,
	S5M8767_LDO22,
	S5M8767_LDO23,
	S5M8767_LDO24,
	S5M8767_LDO25,
	S5M8767_LDO26,
	S5M8767_LDO27,
	S5M8767_LDO28,
	S5M8767_EN32KHZ_CP,

	S5M8767_NUM_OF_REGS,
};

struct sec_voltage_desc {
	int max;
	int min;
	int step;
};

/**
 * struct s5m8767_para - s5m8767 register parameters
 * @param vol_addr	i2c address of the given buck/ldo register
 * @param vol_bitpos	bit position to be set or clear within register
 * @param vol_bitmask	bit mask value
 * @param reg_enaddr	control register address, which enable the given
 *			given buck/ldo.
 * @param reg_enbiton	value to be written to buck/ldo to make it ON
 * @param vol		Voltage information
 */
struct s5m8767_para {
	enum s5m8767_regnum regnum;
	u8	vol_addr;
	u8	vol_bitpos;
	u8	vol_bitmask;
	u8	reg_enaddr;
	u8	reg_enbiton;
	const struct sec_voltage_desc *vol;
};

/* Drivers name */
#define S5M8767_LDO_DRIVER	"s5m8767_ldo"
#define S5M8767_BUCK_DRIVER	"s5m8767_buck"

int s5m8767_enable_32khz_cp(struct udevice *dev);

#endif /* __S5M8767_PMIC_H_ */
