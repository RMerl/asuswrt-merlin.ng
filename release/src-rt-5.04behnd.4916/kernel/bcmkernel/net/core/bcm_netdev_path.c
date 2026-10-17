#include <linux/module.h>
#include <linux/netdevice.h>
#include <linux/rtnetlink.h>

/* Get the lower device of dev.
   Return NULL if dev is NULL, or there is no lower device for dev.
*/
struct net_device *netdev_path_next_dev(struct net_device *dev)
{
	struct net_device *next_dev;
	struct list_head *iter;

	if (!dev) 
		return NULL;

	rcu_read_lock();
	iter = &dev->adj_list.lower;
	next_dev = netdev_next_lower_dev_rcu(dev, &iter);
	rcu_read_unlock();
	return next_dev;
}
EXPORT_SYMBOL(netdev_path_next_dev);

/* Get the root device of dev.
   Return NULL if dev is NULL.
   Return dev if dev has no lower device.
*/
struct net_device *netdev_path_get_root(struct net_device *dev)
{
	int i, nest_lvl;

	if (!dev) 
		return NULL;

	rcu_read_lock();
	nest_lvl = dev->lower_level;
	for (i = 0; i < nest_lvl; i++) {
		struct list_head *lower = &dev->adj_list.lower;
		if (list_empty(lower))
			break;
		
		dev = netdev_next_lower_dev_rcu(dev, &lower);
	}
	rcu_read_unlock();
	return dev;
}
EXPORT_SYMBOL(netdev_path_get_root);

int netdev_path_set_hw_subport_mcast_idx(struct net_device *dev,
					 unsigned int subport_idx)
{
#if NETDEV_PATH_HW_SUBPORTS_MAX > 0
	if (subport_idx >= NETDEV_PATH_HW_SUBPORTS_MAX) {
		printk(KERN_ERR "%s : Invalid subport <%u>, max <%u>",
		       __func__, subport_idx, NETDEV_PATH_HW_SUBPORTS_MAX);
		return -1;
	}
#endif

	dev->bcm_nd_ext.path.hw_subport_mcast_idx = subport_idx;

	return 0;
}

EXPORT_SYMBOL(netdev_path_set_hw_subport_mcast_idx);

/* Adds a NON-ROOT device to a path.
   A Root device is indirectly added to a path once another device points to it.
   This function must be called under rtnl lock.
*/
int netdev_path_add(struct net_device *new_dev, struct net_device *next_dev)
{
	int err = 0;

	/* new device already in a path, fail */
	if (netdev_path_is_linked(new_dev)) {
		err = -EBUSY;
		goto exit;
	}
	err = netdev_upper_dev_link(next_dev, new_dev, NULL);
	if (err) {
		printk(KERN_ERR "%s : netdev_upper_dev_link(%s, %s) error (%d)\n", 
			__func__, next_dev->name, new_dev->name, err);
		goto exit;
    }

	next_dev->bcm_nd_ext.path.refcount++;

#if defined(CONFIG_BCM_SW_GSO)
	if (new_dev->dev.type && new_dev->dev.type->name && !strcmp(new_dev->dev.type->name, "ppp")) {
		/* Enable GSO/TSO capability on PPP interface, but disable SG by default to keep the same performance as original Linux */
		new_dev->features |= next_dev->vlan_features & (~(NETIF_F_SG|NETIF_F_HIGHDMA));
		new_dev->hw_features |= next_dev->vlan_features;
		new_dev->wanted_features = new_dev->features & new_dev->hw_features;    
		__netdev_update_features(new_dev);
	}
#endif

exit:
	return err;
}
EXPORT_SYMBOL(netdev_path_add);

/* Removes a device from a path.
   This function must be called under rtnl lock.
*/
int netdev_path_remove(struct net_device *dev)
{
	struct net_device *next_dev;

	/* device referenced by one or more interfaces, fail */
	if (!netdev_path_is_leaf(dev))
		return -EBUSY;

	next_dev = netdev_path_next_dev(dev);
	/* device has no lower device, nothing to do */
	if (!next_dev)
		goto exit;

	next_dev->bcm_nd_ext.path.refcount--;
	netdev_upper_dev_unlink(next_dev, dev);

exit:
	return 0;
}
EXPORT_SYMBOL(netdev_path_remove);

/* Prints all devices in a path */
void netdev_path_dump(struct net_device *dev)
{
	printk("netdev path : ");
	while (1) {
		pr_cont("%s", dev->name);
		dev = netdev_path_next_dev(dev);
		if (!dev)
			break;
		pr_cont(" -> ");
	}
	pr_cont("\n");
}
EXPORT_SYMBOL(netdev_path_dump);

