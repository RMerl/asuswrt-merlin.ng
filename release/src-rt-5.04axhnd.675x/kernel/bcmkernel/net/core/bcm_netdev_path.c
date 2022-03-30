#include <linux/module.h>
#include <linux/netdevice.h>
#include <linux/rtnetlink.h>

struct net_device *netdev_path_next_dev(struct net_device *dev)
{
    struct net_device *next_dev;
    struct list_head *iter;

    if (!dev) 
        return NULL;
    netdev_for_each_lower_dev(dev, next_dev, iter) {
        if (next_dev->bcm_nd_ext.path.refcount) {
            return next_dev;
        }
    }
    
    return NULL;
}
EXPORT_SYMBOL(netdev_path_next_dev);

struct net_device *netdev_path_get_root(struct net_device *dev)
{
    for (; dev && !netdev_path_is_root(dev); dev = netdev_path_next_dev(dev));
    return dev;
}

EXPORT_SYMBOL(netdev_path_get_root);

int netdev_path_set_hw_subport_mcast_idx(struct net_device *dev,
					 unsigned int subport_idx)
{
	if (subport_idx >= NETDEV_PATH_HW_SUBPORTS_MAX) {
		printk(KERN_ERR "%s : Invalid subport <%u>, max <%u>",
		       __func__, subport_idx, NETDEV_PATH_HW_SUBPORTS_MAX);
		return -1;
	}

	dev->bcm_nd_ext.path.hw_subport_mcast_idx = subport_idx;

	return 0;
}

EXPORT_SYMBOL(netdev_path_set_hw_subport_mcast_idx);

/* Adds a NON-ROOT device to a path. A Root device is indirectly
   added to a path once another device points to it */
int netdev_path_add(struct net_device *new_dev, struct net_device *next_dev)
{
    int err;
    int need_lock = !rtnl_is_locked();

	/* new device already in a path, fail */
	if (netdev_path_is_linked(new_dev))
		return -EBUSY;

    if (need_lock) rtnl_lock();
	err = netdev_upper_dev_link(next_dev, new_dev, NULL);
    if (err) {
        printk(KERN_ERR "%s : netdev_upper_dev_link(%s, %s) error (%d)\n", 
                __func__, next_dev->name, new_dev->name, err);
        if (need_lock) rtnl_unlock();
        return err;
    }

	next_dev->bcm_nd_ext.path.refcount++;

    if (need_lock) rtnl_unlock();
	return 0;
}
EXPORT_SYMBOL(netdev_path_add);

/* Removes a device from a path */
int netdev_path_remove(struct net_device *dev)
{
    struct net_device *next_dev;
    int need_lock = !rtnl_is_locked();

	/* device referenced by one or more interfaces, fail */
	if (!netdev_path_is_leaf(dev))
		return -EBUSY;

	/* device is the first in the list */
	/* Nothing to do */
	if (netdev_path_is_root(dev))
		return 0;

    if (need_lock) rtnl_lock();
    next_dev = netdev_path_next_dev(dev);
	next_dev->bcm_nd_ext.path.refcount--;

    netdev_upper_dev_unlink(next_dev, dev);

    if (need_lock) rtnl_unlock();
	return 0;
}
EXPORT_SYMBOL(netdev_path_remove);

/* Prints all devices in a path */
void netdev_path_dump(struct net_device *dev)
{
	printk("netdev path : ");

	while (1) {
		pr_cont("%s", dev->name);

		if (netdev_path_is_root(dev))
			break;

		pr_cont(" -> ");

		dev = netdev_path_next_dev(dev);
	}

	pr_cont("\n");
}
EXPORT_SYMBOL(netdev_path_dump);

