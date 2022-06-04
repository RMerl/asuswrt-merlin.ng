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

#include <linux/io.h>
#include <bcm_map_part.h>
#include <pmc_drv.h>
#include <BPCM.h>
#include <asm/cp15.h>
#include <pmc_neon.h>
#include <mach/smp.h>
#include <linux/smp.h>
#include <pmc_neon.h>

static void __init neon_enable(void *data)
{
	u32 access;
	(void)data;

	access = get_copro_access();

	/*
	 * Enable full access to VFP (cp10 and cp11)
	 */
	set_copro_access(access | CPACC_FULL(10) | CPACC_FULL(11));

	/* mov r0, 0x40000000; vmsr fpexc, r0 */
	asm volatile ("mov r0, #0x40000000; .word 0xeee80a10" : : : "r0" );
}

static int __init bcm6846_neon_fixup(void)
{
	smp_call_function_single(0, neon_enable, NULL, 1);
	return 0;
}

late_initcall(bcm6846_neon_fixup);

#endif   // CONFIG_BCM_KF_ARM_BCM963XX
