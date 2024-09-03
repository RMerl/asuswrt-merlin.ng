/*
*    Copyright (c) 2003-2020 Broadcom
*    All Rights Reserved
*
<:label-BRCM:2020:DUAL/GPL:standard

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

#ifndef _BR_BCM_MAC_LIMIT_H
#define _BR_BCM_MAC_LIMIT_H

#include <linux/netdevice.h>
#include "br_private.h"
typedef unsigned int (* mac_limit_rcv_hook_t)(struct net_device *dev);
typedef unsigned int (* mac_limit_rcv_check_hook_t)(struct net_device *dev);
typedef void (* mac_learning_notify_hook_t)(struct net_device *dev, int type);
typedef void (* mac_learning_update_hook_t)(struct net_device *dev, int add);

extern void bcm_mac_limit_enable(uint32_t enable);

extern void bcm_mac_limit_learning_notify(struct net_bridge *br, const struct net_bridge_fdb_entry *fdb, int type);
extern void bcm_mac_limit_learning_update(struct net_bridge_fdb_entry *fdb,struct net_bridge_port *source);

#endif /* _BR_BCM_MAC_LIMIT_H */
