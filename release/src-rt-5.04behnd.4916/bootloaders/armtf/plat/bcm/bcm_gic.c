/*
<:copyright-BRCM:2012:DUAL/GPL:standard 

   Copyright (c) 2012 Broadcom 
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

