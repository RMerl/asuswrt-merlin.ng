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
#ifndef __ASM_PLAT_CLOCK_H
#define __ASM_PLAT_CLOCK_H	__FILE__

#define FREQ_MHZ(x)	((x)*1000*1000)

struct clk;

/*
 * Operations on clocks -
 * See <linux/clk.h> for description
 */
struct clk_ops {
	int	(* enable)(struct clk *);
	void	(* disable)(struct clk *);
	long	(* round)(struct clk *, unsigned long);
	int	(* setrate)(struct clk *, unsigned long);
	/* Update current rate and return running status */
	int	(* status)(struct clk *);
};

#endif /* __ASM_PLAT_CLOCK_H */
#endif /* CONFIG_BCM_KF_ARM_BCM963XX */
