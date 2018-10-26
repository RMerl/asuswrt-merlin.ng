/*
<:copyright-BRCM:2013:DUAL/GPL:standard

   Copyright (c) 2013 Broadcom 
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
