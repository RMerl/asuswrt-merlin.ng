/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Exynos pinctrl driver header.
 * Copyright (C) 2016 Samsung Electronics
 * Thomas Abraham <thomas.ab@samsung.com>
 */

#ifndef __PINCTRL_EXYNOS_H_
#define __PINCTRL_EXYNOS_H_

/**
 * enum pincfg_type - possible pin configuration types supported.
 * @PINCFG_TYPE_FUNC: Function configuration.
 * @PINCFG_TYPE_DAT: Pin value configuration.
 * @PINCFG_TYPE_PUD: Pull up/down configuration.
 * @PINCFG_TYPE_DRV: Drive strength configuration.
 */
enum pincfg_type {
	PINCFG_TYPE_FUNC,
	PINCFG_TYPE_DAT,
	PINCFG_TYPE_PUD,
	PINCFG_TYPE_DRV,

	PINCFG_TYPE_NUM
};

/**
 * struct samsung_pin_bank_type: pin bank type description
 * @fld_width: widths of configuration bitfields (0 if unavailable)
 * @reg_offset: offsets of configuration registers (don't care of width is 0)
 */
struct samsung_pin_bank_type {
	u8 fld_width[PINCFG_TYPE_NUM];
	u8 reg_offset[PINCFG_TYPE_NUM];
};

/**
 * struct samsung_pin_bank_data: represent a controller pin-bank data.
 * @type: type of the bank (register offsets and bitfield widths)
 * @offset: starting offset of the pin-bank registers.
 * @nr_pins: number of pins included in this bank.
 * @name: name to be prefixed for each pin in this pin bank.
 */
struct samsung_pin_bank_data {
	const struct samsung_pin_bank_type *type;
	u32		offset;
	u8		nr_pins;
	const char	*name;
};

extern const struct samsung_pin_bank_type bank_type_alive;

#define EXYNOS_PIN_BANK(pins, reg, id)			\
	{						\
		.type		= &bank_type_alive,	\
		.offset		= reg,			\
		.nr_pins	= pins,			\
		.name		= id			\
	}

/**
 * struct samsung_pin_ctrl: represent a pin controller.
 * @pin_banks: list of pin banks included in this controller.
 * @nr_banks: number of pin banks.
 */
struct samsung_pin_ctrl {
	const struct samsung_pin_bank_data *pin_banks;
	u32 nr_banks;
};

/**
 * struct exynos_pinctrl_priv: exynos pin controller driver private data
 * @pin_ctrl: pin controller bank information.
 * @base: base address of the pin controller instance.
 * @num_banks: number of pin banks included in the pin controller.
 */
struct exynos_pinctrl_priv {
	const struct samsung_pin_ctrl *pin_ctrl;
	unsigned long base;
	int num_banks;
};

/**
 * struct exynos_pinctrl_config_data: configuration for a peripheral.
 * @offset: offset of the config registers in the controller.
 * @mask: value of the register to be masked with.
 * @value: new value to be programmed.
 */
struct exynos_pinctrl_config_data {
	const unsigned int	offset;
	const unsigned int	mask;
	const unsigned int	value;
};


void exynos_pinctrl_setup_peri(struct exynos_pinctrl_config_data *conf,
		unsigned int num_conf, unsigned long base);
int exynos_pinctrl_set_state(struct udevice *dev,
		struct udevice *config);
int exynos_pinctrl_probe(struct udevice *dev);

#endif /* __PINCTRL_EXYNOS_H_ */
