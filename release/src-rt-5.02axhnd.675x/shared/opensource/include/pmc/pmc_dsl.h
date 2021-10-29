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

#ifndef PMC_DSL_H
#define PMC_DSL_H

int pmc_dsl_power_up(void);
int pmc_dsl_power_down(void);
int pmc_dsl_clock_set(int flag);	/* flag = 0 => turn off, turn on otherwise */
int pmc_dsl_mips_enable(int flag);	/* flag = 0 => turn off, turn on otherwise */
int pmc_dsl_mipscore_enable(int flag, int core);
int pmc_dsl_core_reset(void);

#endif //#ifndef PMC_DSL_H
