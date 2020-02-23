/*
   <:copyright-BRCM:2015:DUAL/GPL:standard

      Copyright (c) 2015 Broadcom
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