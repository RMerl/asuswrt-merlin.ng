/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2015 Google, Inc
 */

#ifndef _ASM_ARCH_GPIO_H
#define _ASM_ARCH_GPIO_H

struct rockchip_gpio_regs {
	u32 swport_dr;
	u32 swport_ddr;
	u32 reserved0[(0x30 - 0x08) / 4];
	u32 inten;
	u32 intmask;
	u32 inttype_level;
	u32 int_polarity;
	u32 int_status;
	u32 int_rawstatus;
	u32 debounce;
	u32 porta_eoi;
	u32 ext_port;
	u32 reserved1[(0x60 - 0x54) / 4];
	u32 ls_sync;
};
check_member(rockchip_gpio_regs, ls_sync, 0x60);

enum gpio_pu_pd {
	GPIO_PULL_NORMAL = 0,
	GPIO_PULL_UP,
	GPIO_PULL_DOWN,
	GPIO_PULL_REPEAT,
};

/* These defines are only used by spl_gpio.h */
enum {
	/* Banks have 8 GPIOs, so 3 bits, and there are 4 banks, so 2 bits */
	GPIO_BANK_SHIFT		= 3,
	GPIO_BANK_MASK		= 3 << GPIO_BANK_SHIFT,

	GPIO_OFFSET_MASK	= 0x1f,
};

#define GPIO(bank, offset)	((bank) << GPIO_BANK_SHIFT | (offset))

enum gpio_bank_t {
	BANK_A = 0,
	BANK_B,
	BANK_C,
	BANK_D,
};

enum gpio_dir_t {
	GPIO_INPUT = 0,
	GPIO_OUTPUT,
};

#endif
