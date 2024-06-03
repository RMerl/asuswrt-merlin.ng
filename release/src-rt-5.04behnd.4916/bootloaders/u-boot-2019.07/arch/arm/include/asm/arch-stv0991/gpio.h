/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2014, STMicroelectronics - All Rights Reserved
 * Author(s): Vikas Manocha, <vikas.manocha@st.com> for STMicroelectronics.
 */

#ifndef __ASM_ARCH_STV0991_GPIO_H
#define __ASM_ARCH_STV0991_GPIO_H

enum gpio_direction {
	GPIO_DIRECTION_IN,
	GPIO_DIRECTION_OUT,
};

struct gpio_regs {
	u32 data;		/* offset 0x0 */
	u32 reserved[0xff];	/* 0x4--0x3fc */
	u32 dir;		/* offset 0x400 */
};

#endif	/* __ASM_ARCH_STV0991_GPIO_H */
