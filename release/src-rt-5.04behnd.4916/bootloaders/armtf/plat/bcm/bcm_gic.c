/*
<:copyright-BRCM:2012:DUAL/GPL:standard 

   Copyright (c) 2012 Broadcom 
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

#include <assert.h>
#include <bl_common.h>
#include <gicv2.h>
#include <interrupt_mgmt.h>

uint32_t plat_ic_get_pending_interrupt_id(void)
{
	return gicv2_get_pending_interrupt_id();
}

uint32_t plat_ic_get_pending_interrupt_type(void)
{
	return gicv2_get_pending_interrupt_type();
}

uint32_t plat_ic_acknowledge_interrupt(void)
{
	return gicv2_acknowledge_interrupt();
}

uint32_t plat_ic_get_interrupt_type(uint32_t id)
{
	uint32_t group;

	group = gicv2_get_interrupt_group(id);

	/* Assume that all secure interrupts are S-EL1 interrupts */
	if (!group)
		return INTR_TYPE_S_EL1;
	else
		return INTR_TYPE_NS;

}

void plat_ic_end_of_interrupt(uint32_t id)
{
	gicv2_end_of_interrupt(id);
}

uint32_t plat_interrupt_type_to_line(uint32_t type,
				uint32_t security_state)
{
	assert(type == INTR_TYPE_S_EL1 ||
	       type == INTR_TYPE_EL3 ||
	       type == INTR_TYPE_NS);

	assert(sec_state_is_valid(security_state));

	/* Non-secure interrupts are signalled on the IRQ line always */
	if (type == INTR_TYPE_NS)
		return __builtin_ctz(SCR_IRQ_BIT);

	/*
	 * Secure interrupts are signalled using the IRQ line if the FIQ_EN
	 * bit is not set else they are signalled using the FIQ line.
	 */
	if (gicv2_is_fiq_enabled())
		return __builtin_ctz(SCR_FIQ_BIT);
	else
		return __builtin_ctz(SCR_IRQ_BIT);
}

