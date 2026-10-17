/* SPDX-License-Identifier: GPL-2.0+
 *
 * Pinctrl driver for Nexell SoCs
 * (C) Copyright 2016 Nexell
 * Bongyu, KOO <freestyle@nexell.co.kr>
 */

#ifndef __PINCTRL_S5PXX18_H_
#define __PINCTRL_S5PXX18_H_

#include <linux/types.h>
#include <asm/io.h>

#define GPIOX_ALTFN0	0x20
#define GPIOX_ALTFN1	0x24
#define GPIOX_DRV1	0x48
#define GPIOX_DRV0	0x50
#define GPIOX_PULLSEL	0x58
#define GPIOX_PULLENB	0x60

#define GPIOX_SLEW_DISABLE_DEFAULT	0x44
#define GPIOX_DRV1_DISABLE_DEFAULT	0x4C
#define GPIOX_DRV0_DISABLE_DEFAULT	0x54
#define GPIOX_PULLSEL_DISABLE_DEFAULT	0x5C
#define GPIOX_PULLENB_DISABLE_DEFAULT	0x64

#define ALIVE_PWRGATE			0x0
#define ALIVE_PADPULLUPRST		0x80
#define ALIVE_PADPULLUPSET		0x84
#define ALIVE_PADPULLUPREAD		0x88

enum {
	nx_gpio_padfunc_0 = 0ul,
	nx_gpio_padfunc_1 = 1ul,
	nx_gpio_padfunc_2 = 2ul,
	nx_gpio_padfunc_3 = 3ul
};

enum {
	nx_gpio_drvstrength_0 = 0ul,
	nx_gpio_drvstrength_1 = 1ul,
	nx_gpio_drvstrength_2 = 2ul,
	nx_gpio_drvstrength_3 = 3ul
};

enum {
	nx_gpio_pull_down = 0ul,
	nx_gpio_pull_up = 1ul,
	nx_gpio_pull_off = 2ul
};

int s5pxx18_pinctrl_init(struct udevice *dev);
#endif /* __PINCTRL_S5PXX18_H_ */
