/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2016 Marvell International Ltd.
 * https://spdx.org/licenses
 */

 #ifndef __PINCTRL_MVEBU_H_
 #define __PINCTRL_MVEBU_H_

 #define MVEBU_MAX_PINCTL_BANKS		4
 #define MVEBU_MAX_PINS_PER_BANK	100
 #define MVEBU_MAX_FUNC			0xF

/*
 * struct mvebu_pin_bank_data: mvebu-pinctrl bank data
 * @base_reg: controller base address for this bank
 * @pin_cnt:  number of pins included in this bank
 * @max_func: maximum configurable function value for pins in this bank
 * @reg_direction:
 * @bank_name: the pin's bank name
 */
struct mvebu_pinctrl_priv {
	void		*base_reg;
	uint		pin_cnt;
	uint		max_func;
	int		reg_direction;
	const char	*bank_name;
};

#endif /* __PINCTRL_MVEBU_H_ */
