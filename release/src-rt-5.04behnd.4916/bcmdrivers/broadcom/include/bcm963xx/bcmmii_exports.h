/*
<:copyright-BRCM:2008:DUAL/GPL:standard

   Copyright (c) 2008 Broadcom 
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
#ifndef _BCMMII_EXPORT_H_
#define _BCMMII_EXPORT_H_

void ethsw_switch_manage_port_power_mode(int portnumber, int power_mode);
int ethsw_switch_get_port_power_mode(int portnumber);
int ethsw_switch_manage_ports_leds(int led_mode);

int ethsw_iudmaq_to_egressq_map_get(int iudmaq, int *egressq);

#endif /* _BCMMII_EXPORT_H_ */
