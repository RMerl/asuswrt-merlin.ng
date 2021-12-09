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
