/* SPDX-License-Identifier: GPL-2.0+ */

#ifndef _GPIO_DA8XX_DEFS_H_
#define _GPIO_DA8XX_DEFS_H_

struct davinci_gpio {
	unsigned int dir;
	unsigned int out_data;
	unsigned int set_data;
	unsigned int clr_data;
	unsigned int in_data;
	unsigned int set_rising;
	unsigned int clr_rising;
	unsigned int set_falling;
	unsigned int clr_falling;
	unsigned int intstat;
};

struct davinci_gpio_bank {
	int num_gpio;
	unsigned int irq_num;
	unsigned int irq_mask;
	unsigned long *in_use;
	struct davinci_gpio *base;
};

#define GPIO_NAME_SIZE		20
#define MAX_NUM_GPIOS		144
#define GPIO_BIT(gp)		((gp) & 0x1F)

#ifdef CONFIG_DM_GPIO

/* Information about a GPIO bank */
struct davinci_gpio_platdata {
	int bank_index;
	ulong base;	/* address of registers in physical memory */
	const char *port_name;
};
#endif

#endif
