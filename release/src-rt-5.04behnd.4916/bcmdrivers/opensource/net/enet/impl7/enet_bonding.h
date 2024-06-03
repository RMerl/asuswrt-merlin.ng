/*
   <:copyright-BRCM:2022:DUAL/GPL:standard

      Copyright (c) 2022 Broadcom 
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

#ifndef _ENET_BONDING_H_
#define _ENET_BONDING_H_

#if defined(CONFIG_BCM_KERNEL_BONDING)
void bonding_init(void);
void bonding_uninit(void);
int bonding_netdev_event(unsigned long event, struct net_device *dev);
int bonding_update_br_pbvlan(enetx_port_t *sw, struct net_device *dev, uint32_t *portMap);
int bonding_is_lan_wan_port(void *ctxt);
#else
static inline void bonding_init(void) {}
static inline void bonding_uninit(void) {}
static inline int bonding_netdev_event(unsigned long event, struct net_device *dev) { return 0; }
static inline int bonding_update_br_pbvlan(enetx_port_t *sw, struct net_device *dev, uint32_t *portMap) { return 0; }
static inline int bonding_is_lan_wan_port(void *ctxt) { return 0; }
#endif

#endif

