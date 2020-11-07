#if defined(CONFIG_BCM_KF_ARM_BCM963XX)
/*
<:copyright-BRCM:2013:DUAL/GPL:standard

   Copyright (c) 2013 Broadcom 
   All Rights Reserved

Unless you and Broadcom execute a separate written software license
agreement governing use of this software, this software is licensed
to you under the terms of the GNU General Public License version 2
(the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
with the following added to such license:

   As a special exception, the copyright holders of this software give
   you permission to link this software with independent modules, and
   to copy and distribute the resulting executable under terms of your
   choice, provided that you also meet, for each linked independent
   module, the terms and conditions of the license of that module.
   An independent module is a module which is not derived from this
   software.  The special exception does not apply to any modifications
   of the software.

Not withstanding the above, under no circumstances may you combine
this software in any way with any other Broadcom software provided
under a license other than the GPL, without Broadcom's express prior
written consent.

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
