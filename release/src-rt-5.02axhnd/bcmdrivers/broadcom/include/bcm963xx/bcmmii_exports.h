/*
<:copyright-BRCM:2008:DUAL/GPL:standard

   Copyright (c) 2008 Broadcom 
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
#ifndef _BCMMII_EXPORT_H_
#define _BCMMII_EXPORT_H_

void ethsw_switch_manage_port_power_mode(int portnumber, int power_mode);
int ethsw_switch_get_port_power_mode(int portnumber);
int ethsw_switch_manage_ports_leds(int led_mode);

int ethsw_iudmaq_to_egressq_map_get(int iudmaq, int *egressq);

#endif /* _BCMMII_EXPORT_H_ */
