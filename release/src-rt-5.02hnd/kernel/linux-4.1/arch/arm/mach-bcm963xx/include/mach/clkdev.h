#if defined(CONFIG_BCM_KF_ARM_BCM963XX)
/*
<:copyright-BRCM:2013:DUAL/GPL:standard

   Copyright (c) 2013 Broadcom 
   All Rights Reserved

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License, version 2, as published by
the Free Software Foundation (the "GPL").

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.


A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.

:>
*/
#ifndef __ASM_MACH_CLKDEV_H
#define __ASM_MACH_CLKDEV_H	__FILE__

#include <plat/clock.h>
#include <asm/atomic.h>

/* FIXME! the following is based on bcm5301x, and it will need to
 * be modified based on the clk implementation */
struct clk {
	const struct clk_ops	*ops;
	const char		*name;
	atomic_t		ena_cnt;
	atomic_t		use_cnt;
	unsigned long		rate;
	unsigned		gated :1;
	unsigned		fixed :1;
	unsigned		chan  :6;
	void __iomem		*regs_base;
	struct clk		*parent;
	/* TBD: could it have multiple parents to select from ? */
	enum {
		CLK_XTAL, CLK_GATE, CLK_PLL, CLK_DIV, CLK_PHA, CLK_UART, CLK_DMAC
	} type;
};

int __clk_get(struct clk *clk);
void __clk_put(struct clk *clk);

#endif /* __ASM_MACH_CLKDEV_H */
#endif /* CONFIG_BCM_KF_ARM_BCM963XX */
