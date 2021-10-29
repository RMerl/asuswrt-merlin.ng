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
/*
 * pmc_wan.h
 *
 *  Created on: Nov 30 2015
 *      Author: yonatani
 */


#ifndef _PMC_WAN_H_
#define _PMC_WAN_H_

typedef enum {
  WAN_INTF_XPON,
  WAN_INTF_XDSL,
  WAN_INTF_AETH
}WAN_INTF;

/* Power zone lists terminated by -1 */
#if defined (CONFIG_BCM963158) || defined(_BCM963158_)
#define XPON_POWER_ZONES   {1,3,-1}
#define XDSL_POWER_ZONES   {4,-1}
#define AETH_POWER_ZONES   {2,5,6,-1}
#else
#define XPON_POWER_ZONES   {-1}
#define XDSL_POWER_ZONES   {-1}
#define AETH_POWER_ZONES   {-1}
#endif

int pmc_wan_init(void);
int pmc_wan_power_down(void);
int pmc_wan_interface_power_control(WAN_INTF interface, int ctrl);
#if defined (CONFIG_BCM963158)
int pmc_wan_ae_reset(void);
#endif
#endif /* _PMC_WAN_H_ */
