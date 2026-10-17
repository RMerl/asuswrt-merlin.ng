/* SPDX-License-Identifier: GPL-2.0+ */

#ifndef _MPC8XX_GPIO_H_
#define _MPC8XX_GPIO_H_

struct mpc8xx_gpio_plat {
	ulong addr;
	unsigned long size;
	uint ngpios;
};

#endif	/* MPC8XX_GPIO_H_ */
