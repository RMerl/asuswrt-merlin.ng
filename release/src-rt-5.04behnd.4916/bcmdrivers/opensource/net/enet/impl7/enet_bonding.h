/*
   <:copyright-BRCM:2022:DUAL/GPL:standard
   
      Copyright (c) 2022 Broadcom 
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

#ifndef _ENET_BONDING_H_
#define _ENET_BONDING_H_

#if defined(CONFIG_BCM_KERNEL_BONDING)
void bonding_init(void);
void bonding_uninit(void);
int bonding_change_event(struct net_device *slave, struct net_device *master, bool linking);
int bonding_update_br_pbvlan(enetx_port_t *sw, struct net_device *dev, uint32_t *portMap);
int bonding_is_lan_wan_port(void *ctxt);
#else
static inline void bonding_init(void) {}
static inline void bonding_uninit(void) {}
int bonding_change_event(struct net_device *slave, struct net_device *master, bool linking) { return 0; }
static inline int bonding_update_br_pbvlan(enetx_port_t *sw, struct net_device *dev, uint32_t *portMap) { return 0; }
static inline int bonding_is_lan_wan_port(void *ctxt) { return 0; }
#endif

#endif

