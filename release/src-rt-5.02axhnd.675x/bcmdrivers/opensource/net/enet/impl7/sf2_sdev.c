/*
   <:copyright-BRCM:2015:DUAL/GPL:standard
   
      Copyright (c) 2015 Broadcom 
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
 *      Author: steven.hsieh@broadcom.com
 */

#if defined(CONFIG_NET_SWITCHDEV)

#include <bcmnet.h>
#include "port.h"
#include "enet.h"
#include "sf2.h"
#include "sf2_common.h"
#include "linux/if_bridge.h"
#include "linux/netdevice.h"


int sf2_switchdev_port_attr_get(struct net_device *dev,
                                struct switchdev_attr *attr)
{
    enetx_port_t *port = ((enetx_netdev *)netdev_priv(dev))->port;

    switch (attr->id) {
    case SWITCHDEV_ATTR_ID_PORT_PARENT_ID:
        attr->u.ppid.id_len = sizeof(port->obj_name) > MAX_PHYS_ITEM_ID_LEN ? MAX_PHYS_ITEM_ID_LEN : sizeof(port->obj_name);
        // if port is not hw fwd enabled, use its own name to differentiate from real switch name
        memcpy(&attr->u.ppid.id, PORT_IS_HW_FWD(port) ?port->p.parent_sw->obj_name : port->obj_name, attr->u.ppid.id_len);
        enet_dbgv("dev:%s attr->id:%d PORT_PARENT_ID parenet=%s\n", dev->name, attr->id, attr->u.ppid.id);
        break;
//    case SWITCHDEV_ATTR_ID_PORT_BRIDGE_FLAGS:
//        break;
    case SWITCHDEV_ATTR_ID_PORT_BRIDGE_FLAGS_SUPPORT:
        enet_dbgv("dev:%s attr->id:%d PORT_BRIDGE_FLAGS_SUPPORT\n", dev->name, attr->id);
        attr->u.brport_flags_support = BR_LEARNING;
        break;
    default:
        enet_dbgv("dev:%s attr->id:%d ret -EOPNOTSUPP\n", dev->name, attr->id);
        return -EOPNOTSUPP;
    }

    return 0;
}

int sf2_switchdev_port_attr_set(struct net_device *dev,
                                const struct switchdev_attr *attr,
                                struct switchdev_trans *trans)
{
    enetx_port_t *port = ((enetx_netdev *)netdev_priv(dev))->port;
    int err;

    switch (attr->id) {
    case SWITCHDEV_ATTR_ID_PORT_STP_STATE:
        enet_dbgv("dev:%s attr->id:%d %s PORT_STP_STATE=%d\n", dev->name, attr->id,  switchdev_trans_ph_commit(trans) ? "commit":"prepare", attr->u.stp_state);
        if (switchdev_trans_ph_prepare(trans)) return 0;
        err = port_sf2_port_stp_set(port, STP_MODE_UNCHANGED, attr->u.stp_state);
        break;
//    case SWITCHDEV_ATTR_ID_PORT_BRIDGE_FLAGS:
//        break;
    default:
        enet_dbgv("dev:%s attr->id:%d %s ret -EOPNOTSUPP\n", dev->name, attr->id,  switchdev_trans_ph_commit(trans) ? "commit":"prepare");
        err = -EOPNOTSUPP;
        break;
    }

    return err;
}

#endif //CONFIG_NET_SWITCHDEV