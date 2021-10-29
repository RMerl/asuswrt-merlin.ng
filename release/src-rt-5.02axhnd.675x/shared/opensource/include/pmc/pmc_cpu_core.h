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

#ifndef PMC_CPU_CORE_H
#define PMC_CPU_CORE_H

int pmc_cpu_core_power_up(unsigned cpu);
int pmc_cpu_core_power_down(unsigned cpu);
int pmc_cpu_l2cache_power_up(void);
int pmc_cpu_l2cache_power_down(void);

#endif //#ifndef PMC_CPU_CORE_H
