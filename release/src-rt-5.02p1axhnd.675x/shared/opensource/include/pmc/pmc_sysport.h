/*
<:copyright-BRCM:2018:DUAL/GPL:standard 

   Copyright (c) 2018 Broadcom 
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

#ifndef PMC_SYSPORT_H
#define PMC_SYSPORT_H

void pmc_sysport_reset_system_port(int port);
int pmc_sysport_power_up(void);
int pmc_sysport_power_down(void);

#endif //#ifndef PMC_SYSPORT_H
