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

#ifndef __BCM_PRIVATE_H
#define __BCM_PRIVATE_H

void bcm_configure_mmu_el3(unsigned long total_base, unsigned long total_size,
			unsigned long ro_start, unsigned long ro_limit,
			unsigned long coh_start, unsigned long coh_limit);

void plat_bcm_io_setup(void);
unsigned int plat_bcm_calc_core_pos(u_register_t mpidr);

int dt_add_psci_node(void *fdt);
int dt_add_psci_cpu_enable_methods(void *fdt);

#endif /*__BCM_PRIVATE_H*/
