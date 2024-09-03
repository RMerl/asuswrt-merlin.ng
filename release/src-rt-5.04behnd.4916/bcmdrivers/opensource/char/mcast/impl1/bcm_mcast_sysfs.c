/*
<:copyright-BRCM:2020:DUAL/GPL:standard

   Copyright (c) 2020 Broadcom 
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

#include <linux/kernel.h>
#include <linux/netdevice.h>
#include <linux/kobject.h>
#include <linux/sysfs.h>
#include "bcm_mcast_priv.h"

ssize_t bcm_mcastrouter_show(struct device *d, struct device_attribute *attr,
                             char *buf)
{
    struct net_device *dev_p = to_net_dev(d);

    return sprintf(buf, "%d\n", is_netdev_mcastrouter(dev_p) ? 1 : 0);
}

ssize_t bcm_mcastrouter_store(struct device *d, struct device_attribute *attr,
                              const char *buf, size_t count)
{
    struct net_device *dev_p = to_net_dev(d);
    int val;

    sscanf(buf, "%du", &val);

    if ( val )
    {
        netdev_mcastrouter_set(dev_p);
    }
    else
    {
        netdev_mcastrouter_unset(dev_p);
    }
    return count;
}

static struct device_attribute  bcm_mcastrouter_attribute = __ATTR(bcm_mcastrouter,
                                                                   0660,
                                                                   bcm_mcastrouter_show,
                                                                   bcm_mcastrouter_store);

void bcm_mcast_print_sysfs_file_path(struct net_device *dev)
{
    struct kobject *devkobj = &dev->dev.kobj;
    struct kernfs_node *parent;
    char *buf;

    parent = devkobj->parent->sd;

    buf = kzalloc(PATH_MAX, GFP_KERNEL);
    if (buf)
    {
        kernfs_path(parent, buf, PATH_MAX);

        bcm_printk("*** dev %s bcm_mcastrouter file path %s/%s ***\n", 
                   dev->name, buf, kobject_name(devkobj) );

        kfree(buf);
    }
}

void bcm_mcast_sysfs_create_file(struct net_device *dev)
{
    struct kobject *devkobj = &dev->dev.kobj;
    int error;

    /*  The bcm_mcastrouter file is normally located in one of the following paths. 
        /sys/devices/virtual/net/<dev>
        /sys/devices/platform/80040000.pcie/pci0000:00/0000:00:00.0/0000:01:00.0/net/<wl dev>
        Enable the following function to print the sysfs path for the device if the device
        cannot be located in one of the above paths */
    // bcm_mcast_print_sysfs_file_path(dev);

    error = sysfs_create_file(devkobj, &bcm_mcastrouter_attribute.attr);
    if (error) 
    {
        __logError("failed to create the bcm_mcastrouter file for dev", dev->name);
    }
}

void bcm_mcast_sysfs_remove_file(struct net_device *dev)
{
    struct kobject *devkobj = &dev->dev.kobj;

    sysfs_remove_file(devkobj, &bcm_mcastrouter_attribute.attr);
}
