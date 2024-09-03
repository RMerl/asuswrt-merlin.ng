/*
   <:copyright-BRCM:2019:DUAL/GPL:standard
   
      Copyright (c) 2019 Broadcom 
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
 *  Created on: Mar/2019
 *      Author: nikolai.iosifov@broadcom.com
 */


#include <linux/if_bridge.h>
#include <linux/netdevice.h>
#include <bcmnet.h>
#include "bdmf_interface.h"
#include <rdpa_api.h>
#include "port.h"
#include "enet.h"
#include "runner.h"
#include "runner_common.h"
#include <bdmf_dev.h>

#if defined(CONFIG_NET_SWITCHDEV) && (LINUX_VERSION_CODE < KERNEL_VERSION(5,1,0))

static int __find_upper_bridge(struct net_device *dev, void *data)
{
    if (dev->priv_flags & IFF_EBRIDGE)
    {
        struct net_device **br_dev = data;
        *br_dev = dev;
        return 1;
    }
	return 0;
}

int runner_port_attr_get(struct net_device *dev, struct switchdev_attr *attr)
{
    switch (attr->id) {
    case SWITCHDEV_ATTR_ID_PORT_PARENT_ID:
        {
            struct net_device *upper = NULL;
            char *name = dev->name;

            rcu_read_lock();
            netdev_walk_all_upper_dev_rcu(dev, __find_upper_bridge, &upper);

            if (upper)
                name = upper->name;

            rcu_read_unlock();

            attr->u.ppid.id_len = snprintf(attr->u.ppid.id, MAX_PHYS_ITEM_ID_LEN, "%s", name);
            enet_dbgv("dev:%s attr->id:%d PORT_PARENT_ID:%s\n", dev->name, attr->id, attr->u.ppid.id);
        }
        break;
    case SWITCHDEV_ATTR_ID_PORT_BRIDGE_FLAGS_SUPPORT:
        enet_dbgv("dev:%s attr->id:%d PORT_BRIDGE_FLAGS_SUPPORT\n", dev->name, attr->id);
        attr->u.brport_flags_support = BR_LEARNING;
        break;
    case SWITCHDEV_ATTR_ID_PORT_BRIDGE_FLAGS:
        enet_dbgv("dev:%s attr->id:%d PORT_BRIDGE_FLAGS\n", dev->name, attr->id);
        attr->u.brport_flags = BR_LEARNING;
        break;
    default:
        enet_dbgv("dev:%s attr->id:%d ret -EOPNOTSUPP\n", dev->name, attr->id);
        return -EOPNOTSUPP;
    }

    return 0;
}

int runner_port_attr_set(struct net_device *dev, const struct switchdev_attr *attr, struct switchdev_trans *trans)
{
    int err = 0;

    switch (attr->id) {
    case SWITCHDEV_ATTR_ID_PORT_BRIDGE_FLAGS:
        break;
    default:
        enet_dbgv("dev:%s attr->id:%d %s ret -EOPNOTSUPP\n", dev->name, attr->id,  switchdev_trans_ph_commit(trans) ? "commit" : "prepare");
        err = -EOPNOTSUPP;
        break;
    }

    return err;
}
#endif /* CONFIG_NET_SWITCHDEV */
