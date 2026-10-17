/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2023 StarFive Technology Co., Ltd.
 * Author:	yanhong <yanhong.wang@starfivetech.com>
 *
 */

#ifndef _GPIO_STARFIVE_H_
#define _GPIO_STARFIVE_H_

#include <asm/arch/regs.h>

#define GPIO_NUM_SHIFT		2 /*one dword include 4 gpios*/
#define GPIO_BYTE_SHIFT		3

#define GPIO_INDEX_MASK		0x3

#define GPIO_DOEN_MASK		0x3f
#define GPIO_DOUT_MASK		0x7f
#define GPIO_DIN_MASK		0x7f
#define GPIO_DS_MASK		0x06
#define GPIO_DS_SHIFT		1
#define GPIO_SLEW_MASK		BIT(5)
#define GPIO_SLEW_SHIFT		5
#define GPIO_PULL_MASK		0x18
#define GPIO_PULL_SHIFT		3
#define GPIO_PULL_UP		1
#define GPIO_PULL_DOWN		2

#define NR_GPIOS		64

#define GPIO_OFFSET(gpio)	\
	(((gpio) >> GPIO_NUM_SHIFT) << GPIO_NUM_SHIFT)

#define GPIO_SHIFT(gpio) \
	(((gpio) & GPIO_INDEX_MASK) << GPIO_BYTE_SHIFT)

enum gpio_state {
	LOW,
	HIGH
};

#define GPIO_DOEN		0x0
#define GPIO_DOUT		0x40
#define GPIO_DIN		0x80
#define GPIO_EN			0xdc
#define GPIO_LOW_IE		0x100
#define GPIO_HIGH_IE		0x104
#define GPIO_CONFIG		0x120

#define SYS_IOMUX_DOEN(gpio, oen) \
	clrsetbits_le32(JH7110_SYS_IOMUX + GPIO_OFFSET(gpio), \
		GPIO_DOEN_MASK << GPIO_SHIFT(gpio), \
		(oen) << GPIO_SHIFT(gpio))

#define SYS_IOMUX_DOUT(gpio, gpo) \
	clrsetbits_le32(JH7110_SYS_IOMUX + GPIO_DOUT + GPIO_OFFSET(gpio), \
			GPIO_DOUT_MASK << GPIO_SHIFT(gpio), \
			((gpo) & GPIO_DOUT_MASK) << GPIO_SHIFT(gpio))

#define SYS_IOMUX_DIN(gpio, gpi)\
	clrsetbits_le32(JH7110_SYS_IOMUX + GPIO_DIN + GPIO_OFFSET(gpi), \
			GPIO_DIN_MASK << GPIO_SHIFT(gpi), \
			((gpio + 2) & GPIO_DIN_MASK) << GPIO_SHIFT(gpi))

#define SYS_IOMUX_SET_DS(gpio, ds) \
	clrsetbits_le32(JH7110_SYS_IOMUX + GPIO_CONFIG + gpio * 4, \
			GPIO_DS_MASK, (ds) << GPIO_DS_SHIFT)

#define SYS_IOMUX_SET_SLEW(gpio, slew) \
	clrsetbits_le32(JH7110_SYS_IOMUX + GPIO_CONFIG + gpio * 4, \
			GPIO_SLEW_MASK, (slew) << GPIO_SLEW_SHIFT)

#define SYS_IOMUX_SET_PULL(gpio, pull) \
	clrsetbits_le32(JH7110_SYS_IOMUX + GPIO_CONFIG + gpio * 4, \
			GPIO_PULL_MASK, (pull) << GPIO_PULL_SHIFT)

#define SYS_IOMUX_COMPLEX(gpio, gpi, gpo, oen) \
	do { \
		SYS_IOMUX_DOEN(gpio, oen); \
		SYS_IOMUX_DOUT(gpio, gpo); \
		SYS_IOMUX_DIN(gpio, gpi); \
	} while (0)

#endif /* _GPIO_STARFIVE_H_ */
