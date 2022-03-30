/*
*    Copyright (c) 2003-2020 Broadcom
*    All Rights Reserved
*
<:label-BRCM:2020:DUAL/GPL:standard

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

#ifndef _BR_BCM_MAC_LIMIT_H
#define _BR_BCM_MAC_LIMIT_H

#include <linux/netdevice.h>
#include "br_private.h"
typedef unsigned int (* mac_limit_rcv_hook_t)(struct net_device *dev);
typedef void (* mac_learning_notify_hook_t)(struct net_device *dev, int type);
typedef void (* mac_learning_update_hook_t)(struct net_device *dev, int add);

extern void bcm_mac_limit_learning_notify(struct net_bridge *br, const struct net_bridge_fdb_entry *fdb, int type);
extern void bcm_mac_limit_learning_update(struct net_bridge_fdb_entry *fdb,struct net_bridge_port *source);
#endif /* _BR_BCM_MAC_LIMIT_H */