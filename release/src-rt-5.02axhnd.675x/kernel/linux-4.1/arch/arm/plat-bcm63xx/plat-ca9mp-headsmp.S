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
