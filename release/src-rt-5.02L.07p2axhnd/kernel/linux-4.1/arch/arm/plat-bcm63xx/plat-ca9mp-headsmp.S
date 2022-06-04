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
#include <linux/linkage.h>
#include <linux/init.h>

	__CPUINIT

/*
 * Broadcom specific entry point for secondary CPUs.
 * This provides a "holding pen" into which all secondary cores are held
 * until we're ready for them to initialise.
 */
ENTRY(platform_secondary_startup)
	/*
	 * Get hardware CPU id of ours
	 */
	mrc	p15, 0, r0, c0, c0, 5
	and	r0, r0, #15
	adr	r4, 1f
	ldmia	r4, {r5, r6}
	sub	r4, r4, r5
	add	r6, r6, r4
pen:	ldr	r7, [r6]
	cmp	r7, r0
	bne	pen
	nop

	/* enable the cpu cycle counter on second core. adsl driver use that */
	mrc	p15, 0, r1, c9, c12, 0
	ldr	r2, =5
	orr	r1, r1, r2
	mcr	p15, 0, r1, c9, c12, 0
	ldr	r1, =0x80000000
	mcr	p15, 0, r1, c9, c12, 1

	bleq	v7_invalidate_l1

	/*
	 * we've been released from the holding pen: secondary_stack
	 * should now contain the SVC stack for this core
	 */
	b	secondary_startup

	.align
1:	.long	.
	.long	pen_release
ENDPROC(platform_secondary_startup)

#endif /* CONFIG_BCM_KF_ARM_BCM963XX */
