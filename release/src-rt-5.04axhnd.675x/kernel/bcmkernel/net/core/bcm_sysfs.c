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
 *  Created on: Nov/2019
 *      Author: nikolai.iosifov@broadcom.com
 */

#if defined(CONFIG_BRIDGE)
/* Bridge related SYSFS */
#include <linux/if_bridge.h>
#include <linux/br_fp.h>

static ssize_t bridge_type_store(struct device *d,
    struct device_attribute *attr, const char *buf, size_t len)
{
    struct net_device *br_dev = to_net_dev(d);
    char *endp;
    unsigned long val;

    if (!ns_capable(dev_net(br_dev)->user_ns, CAP_NET_ADMIN))
        return -EPERM;

    val = simple_strtoul(buf, &endp, 0);
    if (endp == buf)
        return -EINVAL;

#if defined(CONFIG_BCM_RDPA_BRIDGE) || defined(CONFIG_BCM_RDPA_BRIDGE_MODULE)
    br_fp_hook(br_dev, BR_FP_BRIDGE_TYPE, br_dev, (void *)&val);
#endif

    return len;
}

static DEVICE_ATTR_WO(bridge_type);

static ssize_t mac_entry_discard_counter_show(struct device *d,
    struct device_attribute *attr, char *buf)
{
    struct net_device *br_dev = to_net_dev(d);

    return sprintf(buf, "%u\n", bridge_mac_entry_discard_counter_get(br_dev));
}

static ssize_t mac_entry_discard_counter_store(struct device *d,
    struct device_attribute *attr, const char *buf, size_t len)
{
    struct net_device *br_dev = to_net_dev(d);
    char *endp;
    unsigned long val;

    if (!ns_capable(dev_net(br_dev)->user_ns, CAP_NET_ADMIN))
        return -EPERM;

    val = simple_strtoul(buf, &endp, 0);
    if (endp == buf)
        return -EINVAL;

    bridge_mac_entry_discard_counter_set(br_dev, val);

    return len;
}

static DEVICE_ATTR_RW(mac_entry_discard_counter);

static ssize_t local_switching_disable_show(struct device *d,
    struct device_attribute *attr, char *buf)
{
    struct net_device *br_dev = to_net_dev(d);

    return sprintf(buf, "%u\n", bridge_local_switching_disable_get(br_dev));
}

static ssize_t local_switching_disable_store(struct device *d,
    struct device_attribute *attr, const char *buf, size_t len)
{
    struct net_device *br_dev = to_net_dev(d);
    char *endp;
    unsigned long val;

    if (!ns_capable(dev_net(br_dev)->user_ns, CAP_NET_ADMIN))
        return -EPERM;

    val = simple_strtoul(buf, &endp, 0);
    if (endp == buf)
        return -EINVAL;

    bridge_local_switching_disable_set(br_dev, val);
#if defined(CONFIG_BCM_RDPA_BRIDGE) || defined(CONFIG_BCM_RDPA_BRIDGE_MODULE)
    br_fp_hook(br_dev, BR_FP_LOCAL_SWITCHING_DISABLE, br_dev, NULL);
#endif

    return len;
}

static DEVICE_ATTR_RW(local_switching_disable);

static struct attribute *bcm_br_attrs[] = {
    &dev_attr_bridge_type.attr,
    &dev_attr_mac_entry_discard_counter.attr,
    &dev_attr_local_switching_disable.attr,
    NULL
};

static struct attribute_group bcm_sysfs_br = {
    .name = SYSFS_BRIDGE_ATTR,
    .attrs = bcm_br_attrs,
};
#endif /* CONFIG BRIDGE*/

/* Generic registration/unregistration functions */

static void bcm_sysfs_add(struct net_device *dev)
{
#if defined (CONFIG_BRIDGE)
    if (dev->priv_flags & IFF_EBRIDGE)
        sysfs_merge_group(&dev->dev.kobj, &bcm_sysfs_br);
#endif /* CONFIG_BRIDGE */
}

static int netdev_event_add(struct notifier_block *this, unsigned long event, void *ptr)
{
    struct net_device *dev = netdev_notifier_info_to_dev(ptr);

    if (event == NETDEV_REGISTER)
        bcm_sysfs_add(dev);

    return NOTIFY_DONE;
}

static struct notifier_block netdev_notifier_add = {
    .notifier_call = netdev_event_add,
    .priority = INT_MIN, /* Must run last */
};

static int __init bcm_sysfs_init(void)
{
    register_netdevice_notifier(&netdev_notifier_add);
    /* we do not need to run: 
     *
     * sysfs_unmerge_group(&dev->dev.kobj, &bcm_sysfs_br);
     *
     * on unregister, because the sysfs directory is going to be removed by the
     * bridge itself before the notifiers are being called. See br_dev_delete.
     */
    return 0;
}

subsys_initcall(bcm_sysfs_init);
