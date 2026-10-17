/*
   <:copyright-BRCM:2024:DUAL/GPL:standard
   
      Copyright (c) 2024 Broadcom 
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
*******************************************************************************
*
* File Name  : tc.c
*
* Description: TC GPL Driver
*
*******************************************************************************
*/

#include <linux/module.h>
#include "tc_int.h"
#include "bcm_tc.h"

static struct list_head dev_list;

struct dev_entry {
    struct list_head dev_base_head;
    struct net_device *dev;
    struct net_device_ops netdev_ops;
    const struct net_device_ops *orig_ops;
    int has_tc;
};

static struct dev_entry *tc_drv_find_dev_entry(struct net_device *dev)
{
    struct dev_entry *e;

    list_for_each_entry(e, &dev_list, dev_base_head)
    {
        if (e->dev == dev)
            return e;
    }

    return NULL;
}

int tc_drv_setup_tc(struct net_device *dev, enum tc_setup_type type, void *type_data)
{
    int rc;
    struct dev_entry *e;

    if (type == TC_SETUP_BLOCK &&
        ((struct flow_block_offload *)type_data)->binder_type == FLOW_BLOCK_BINDER_TYPE_CLSACT_INGRESS)
    {
        rc = tc_ingress_setup_block(dev, type_data);
        if (rc != -EOPNOTSUPP)
            return rc;
    }

    e = tc_drv_find_dev_entry(dev);
    if (e)
    {
        if (e->orig_ops->ndo_setup_tc)
            return e->orig_ops->ndo_setup_tc(dev, type, type_data);
    }

    return -EOPNOTSUPP;
}

static int tc_netdev_event(struct notifier_block *nb, unsigned long event, void *ptr)
{
    struct net_device *dev = netdev_notifier_info_to_dev(ptr);
    struct dev_entry *e;

    if (!is_netdev_wlan(dev))
        return NOTIFY_DONE;

    printk("%s:%d dev %s event %d\n", __FUNCTION__, __LINE__, dev->name, (int)event);

    switch (event)
    {
        case NETDEV_REGISTER:
            e = kmalloc(sizeof(*e), GFP_KERNEL);
            if (!e)
                break;

            e->dev = dev;
            e->orig_ops = dev->netdev_ops;
            e->netdev_ops = *dev->netdev_ops;
            e->netdev_ops.ndo_setup_tc = tc_drv_setup_tc;
            dev->netdev_ops = &e->netdev_ops;
            e->has_tc = (dev->features & NETIF_F_HW_TC) ? 1 : 0;
            dev->features |= NETIF_F_HW_TC;
            INIT_LIST_HEAD(&e->dev_base_head);
            list_add(&e->dev_base_head, &dev_list);
            break;

        case NETDEV_UNREGISTER: 
            e = tc_drv_find_dev_entry(dev);
            if (e)
            {
                list_del(&e->dev_base_head);
                if (!e->has_tc)
                    dev->features |= ~NETIF_F_HW_TC;
                dev->netdev_ops = e->orig_ops;
                kfree(e);
            }
            break;

        default:
            break;
    }

    return NOTIFY_DONE;
}

static struct notifier_block tc_netdev_notifier = {
    .notifier_call = tc_netdev_event
};

static int __init tc_init(void)
{
    int rc;

    rc = tc_ingress_init();
    if (rc)
    {
        __logError(TC_MODNAME " Char Driver " TC_VER_STR " failed to register (%d)\n", rc);
        return rc;
    }

    INIT_LIST_HEAD(&dev_list);
    rc = register_netdevice_notifier(&tc_netdev_notifier);
    if (rc)
    {
        tc_ingress_exit();
        __logError(TC_MODNAME " Char Driver " TC_VER_STR " failed to register (%d)\n", rc);
        return rc;
    }
    __logNotice(TC_MODNAME " Char Driver " TC_VER_STR " Registered\n");

    bcmLog_setLogLevel(BCM_LOG_ID_TC, BCM_LOG_LEVEL_ERROR);
    return 0;
}

static void __exit tc_exit(void)
{
    unregister_netdevice_notifier(&tc_netdev_notifier);
    tc_ingress_exit();
    __logNotice(TC_MODNAME " Char Driver " TC_VER_STR " Unregistered");
}

module_init(tc_init);
module_exit(tc_exit);

MODULE_DESCRIPTION(TC_MODNAME " GPL");
MODULE_VERSION(TC_VERSION);
MODULE_LICENSE("GPL");

MODULE_SOFTDEP("post: wfd");
