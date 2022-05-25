/*
   <:copyright-BRCM:2019:DUAL/GPL:standard

      Copyright (c) 2019 Broadcom
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
 *  Created on: Jul/2019
 *      Author: nikolai.iosifov@broadcom.com
 */

#if defined(CONFIG_NET_SWITCHDEV)

#include <linux/if_bridge.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <net/switchdev.h>
#include "br_private.h"

int rdpa_add_or_modify_mac_tbl_entry(struct net_bridge_port *br_port, const uint8_t *addr, uint16_t vid, int is_static, int is_add);
int rdpa_remove_mac_tbl_entry(struct net_bridge_port *br_port, const uint8_t *addr, uint16_t vid, int remove_dups);

/* switchdev notifier */
struct runner_switchdev_event_work {
    struct work_struct work;
    struct switchdev_notifier_fdb_info fdb_info;
    unsigned long event;
};

/* called under rtnl lock */
static inline void runner_switchdev_fdb_add(struct net_bridge_port *port, struct switchdev_notifier_fdb_info *fdb_info)
{
    rdpa_add_or_modify_mac_tbl_entry(port, fdb_info->addr, fdb_info->vid, 0, 0);
}

/* called under rtnl lock */
static inline void runner_switchdev_fdb_del(struct net_bridge_port *port, struct switchdev_notifier_fdb_info *fdb_info)
{
    rdpa_remove_mac_tbl_entry(port, fdb_info->addr, fdb_info->vid, 0);
}

int runner_switchdev_learning_enabled(struct net_device *dev)
{
    struct switchdev_attr attr = {
        .orig_dev = dev,
        .id = SWITCHDEV_ATTR_ID_PORT_BRIDGE_FLAGS,
    };
    int err = switchdev_port_attr_get(dev, &attr);

    return !(err || !(attr.u.brport_flags & BR_LEARNING));
}

/* Called under rcu_read_lock() and RTNL() */
static int runner_switchdev_event(struct notifier_block *unused, unsigned long event, void *ptr)
{
    struct net_device *dev = switchdev_notifier_info_to_dev(ptr);
    struct switchdev_notifier_fdb_info *fdb_info = ptr;
    struct net_bridge_port *port;

    if (!runner_switchdev_learning_enabled(dev) || !(port = br_port_get_rtnl(dev)))
        return NOTIFY_DONE;

    switch (event) {
    case SWITCHDEV_FDB_ADD_TO_DEVICE:
        runner_switchdev_fdb_add(br_port_get_rtnl(dev), fdb_info);
        break;
    case SWITCHDEV_FDB_DEL_TO_DEVICE:
        runner_switchdev_fdb_del(br_port_get_rtnl(dev), fdb_info);
        break;
    default:
        break;
    }

    return NOTIFY_DONE;
}

static struct notifier_block runner_switchdev_notifier = {
    .notifier_call = runner_switchdev_event,
};

int runner_switchdev_init(void)
{
    int err;

    err = register_switchdev_notifier(&runner_switchdev_notifier);
    if (err) {
        printk(KERN_ERR "Failed to register switchdev notifier (%d)\n", err);
    }
    return err;
}

int runner_switchdev_cleanup(void)
{
    int err;

    err = unregister_switchdev_notifier(&runner_switchdev_notifier);
    if (err) {
        printk(KERN_ERR "Failed to unregister switchdev notifier (%d)\n", err);
    }
    return err;
}

#endif /* CONFIG_NET_SWITCHDEV */
