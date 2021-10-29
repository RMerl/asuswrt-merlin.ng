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
/*
 * This file exists to satisfy compile-time dependency from:
 * arch/arm/include/asm/timex.h
 * It must be a value known at compile time for <linux/jiffies.h>
 * but its value is never used in the resulting code.
 * If "get_cycles()" inline function in <asm/timex.h> is rewritten,
 * then in combination with this constant it could be used to measure
 * microsecond elapsed time using the global timer clock-source.
 * -LR
 */

/* FIXME!! when knowing the real clock tick rate */
#define CLOCK_TICK_RATE		(1000000)

#endif /* CONFIG_BCM_KF_ARM_BCM963XX */
