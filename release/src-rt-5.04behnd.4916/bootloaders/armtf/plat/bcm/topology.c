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

#include <arch.h>
#include <platform_def.h>
#include "bcm_private.h"

/* The power domain tree descriptor */
static unsigned char power_domain_tree_desc[] = {
	/* Number of root nodes */
	PLATFORM_CLUSTER_COUNT,
	/* Number of children for the first node */
	PLATFORM_CLUSTER0_CORE_COUNT,
	/* Number of children for the second node */
	PLATFORM_CLUSTER1_CORE_COUNT,
};

/*******************************************************************************
 * This function returns the ARM default topology tree information.
 ******************************************************************************/
const unsigned char *plat_get_power_domain_tree_desc(void)
{
	return power_domain_tree_desc;
}

/*******************************************************************************
 * This function implements a part of the critical interface between the psci
 * generic layer and the platform that allows the former to query the platform
 * to convert an MPIDR to a unique linear index. An error code (-1) is returned
 * in case the MPIDR is invalid.
 ******************************************************************************/
int plat_core_pos_by_mpidr(u_register_t mpidr)
{
#if defined (PLATFORM_FLAVOR_68880) || defined (PLATFORM_FLAVOR_6837)
	return (mpidr >> MPIDR_AFF1_SHIFT) & 0xFF;
#else
	unsigned int cluster_id, cpu_id;

	mpidr &= MPIDR_AFFINITY_MASK;
	if (mpidr & ~(MPIDR_CLUSTER_MASK | MPIDR_CPU_MASK))
		return -1;

	cluster_id = (mpidr >> MPIDR_AFF1_SHIFT) & MPIDR_AFFLVL_MASK;
	cpu_id = (mpidr >> MPIDR_AFF0_SHIFT) & MPIDR_AFFLVL_MASK;

	if (cluster_id >= PLATFORM_CLUSTER_COUNT)
		return -1;

	if (cpu_id >= PLATFORM_MAX_CPUS_PER_CLUSTER)
		return -1;

	return plat_bcm_calc_core_pos(mpidr);
#endif
}

/* Needed for floating point lib */
void raise (void) { asm("b ."); }

