/* SPDX-License-Identifier: GPL-2.0+ */

#ifndef _LINUX_DELAY_H
#define _LINUX_DELAY_H

#include <linux/kernel.h>

void __udelay(unsigned long usec);
void udelay(unsigned long usec);

static inline void mdelay(unsigned long msec)
{
	udelay(1000 * msec);
}

static inline void ndelay(unsigned long nsec)
{
	udelay(DIV_ROUND_UP(nsec, 1000));
}

#endif /* defined(_LINUX_DELAY_H) */
