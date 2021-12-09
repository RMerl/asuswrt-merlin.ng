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
