/*
 * Network device related boilerplate functions
 *
 * Copyright (c) 2013-2014 Andrew Yourtchenko <ayourtch@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2
 * as published by the Free Software Foundation
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */



#include <linux/if_arp.h>
#include <linux/netdevice.h>
#include <linux/route.h>
#include <linux/skbuff.h>
#include <net/ip6_fib.h>
#include <net/ip6_route.h>
#include <net/ipv6.h>
#include <linux/version.h>
#include "nat46-core.h"
#include "nat46-module.h"

#define NETDEV_DEFAULT_NAME "nat46."

#if defined(CONFIG_BCM_KF_BLOG) && defined(CONFIG_BLOG)
#include <linux/nbuff.h>
#include <linux/blog.h>
#endif
#if defined(CONFIG_BCM_KF_NETDEV_EXT)
#include <linux/bcm_netdevice.h>
#endif

typedef struct {
  u32 sig;
  nat46_instance_t *nat46;
} nat46_netdev_priv_t;

static u8 netdev_count = 0;

static int nat46_netdev_up(struct net_device *dev);
static int nat46_netdev_down(struct net_device *dev);

static netdev_tx_t nat46_netdev_xmit(struct sk_buff *skb, struct net_device *dev);


static const struct net_device_ops nat46_netdev_ops = {
	.ndo_open       = nat46_netdev_up,      /* Called at ifconfig nat46 up */
	.ndo_stop       = nat46_netdev_down,    /* Called at ifconfig nat46 down */
	.ndo_start_xmit = nat46_netdev_xmit,    /* REQUIRED, must return NETDEV_TX_OK */
};

static int nat46_netdev_up(struct net_device *dev)
{
	netif_start_queue(dev);
	return 0;
}

static int nat46_netdev_down(struct net_device *dev)
{
	netif_stop_queue(dev);
	return 0;
}

static netdev_tx_t nat46_netdev_xmit(struct sk_buff *skb, struct net_device *dev)
{
	dev->stats.rx_packets++;
	dev->stats.rx_bytes += skb->len;

#if defined(CONFIG_BCM_KF_BLOG) && defined(CONFIG_BLOG)
    blog_lock();
    blog_link(IF_DEVICE, blog_ptr(skb), (void*)dev, DIR_RX, skb->len);
    blog_unlock();
#endif   

	if(ETH_P_IP == ntohs(skb->protocol)) {
		nat46_ipv4_input(skb);
	}
	if(ETH_P_IPV6 == ntohs(skb->protocol)) {
		nat46_ipv6_input(skb);
	}
	kfree_skb(skb);
	return NETDEV_TX_OK;
}

void nat46_netdev_count_xmit(struct sk_buff *skb, struct net_device *dev) {
	dev->stats.tx_packets++;
	dev->stats.tx_bytes += skb->len;

#if defined(CONFIG_BCM_KF_BLOG) && defined(CONFIG_BLOG)
    blog_lock();
    blog_link(IF_DEVICE, blog_ptr(skb), (void*)dev, DIR_TX, skb->len);
    blog_unlock();
#endif   
}

void *netdev_nat46_instance(struct net_device *dev) {
	nat46_netdev_priv_t *priv = netdev_priv(dev);
	return priv->nat46;
}

static void netdev_nat46_set_instance(struct net_device *dev, nat46_instance_t *new_nat46) {
	nat46_netdev_priv_t *priv = netdev_priv(dev);
	if(priv->nat46) {
		release_nat46_instance(priv->nat46);
	}
	priv->nat46 = new_nat46;
}

static void nat46_netdev_setup(struct net_device *dev)
{
	nat46_netdev_priv_t *priv = netdev_priv(dev);
	nat46_instance_t *nat46 = alloc_nat46_instance(1, NULL, -1, -1, -1);

	memset(priv, 0, sizeof(*priv));
	priv->sig = NAT46_DEVICE_SIGNATURE;
	priv->nat46 = nat46;

	dev->netdev_ops = &nat46_netdev_ops;
	dev->type = ARPHRD_NONE;
	dev->hard_header_len = 0;
	dev->addr_len = 0;
	dev->mtu = 16384; /* iptables does reassembly. Rather than using ETH_DATA_LEN, let's try to get as much mileage as we can with the Linux stack */
	dev->features = NETIF_F_NETNS_LOCAL;
	dev->flags = IFF_NOARP | IFF_POINTOPOINT;
}

int nat46_netdev_create(char *basename, struct net_device **dev)
{
	int ret = 0;
	char *devname = NULL;
	int automatic_name = 0;

	if (basename && strcmp("", basename)) {
		devname = kmalloc(strlen(basename)+1, GFP_KERNEL);
	} else {
		devname = kmalloc(strlen(NETDEV_DEFAULT_NAME)+3+1, GFP_KERNEL);
		automatic_name = 1;
	}
	if (!devname) {
		printk("nat46: can not allocate memory to store device name.\n");
		ret = -ENOMEM;
		goto err;
	}
	if (automatic_name) {
		snprintf(devname, strlen(NETDEV_DEFAULT_NAME)+3, "%s%d", NETDEV_DEFAULT_NAME, netdev_count);
		netdev_count++;
	} else {
		strcpy(devname, basename);
	}

#if LINUX_VERSION_CODE <= KERNEL_VERSION(3,17,0)
	*dev = alloc_netdev(sizeof(nat46_instance_t), devname, nat46_netdev_setup);
#else
	*dev = alloc_netdev(sizeof(nat46_instance_t), devname, NET_NAME_UNKNOWN, nat46_netdev_setup);
#endif
	if (!*dev) {
		printk("nat46: Unable to allocate nat46 device '%s'.\n", devname);
		ret = -ENOMEM;
		goto err_alloc_dev;
	}

	ret = register_netdev(*dev);
	if(ret) {
		printk("nat46: Unable to register nat46 device.\n");
		ret = -ENOMEM;
		goto err_register_dev;
	}

	printk("nat46: netdevice nat46 '%s' created successfully.\n", devname);
	kfree(devname);

	return 0;

err_register_dev:
	free_netdev(*dev);
err_alloc_dev:
	kfree(devname);
err:
	return ret;
}

void nat46_netdev_destroy(struct net_device *dev)
{
	netdev_nat46_set_instance(dev, NULL);
	unregister_netdev(dev);
	printk("nat46: Destroying nat46 device.\n");
}

static int is_nat46(struct net_device *dev) {
	nat46_netdev_priv_t *priv = netdev_priv(dev);
	return (priv && (NAT46_DEVICE_SIGNATURE == priv->sig));
}


static struct net_device *find_dev(char *name) {
	struct net_device *dev;
	struct net_device *out = NULL;

	if(!name) {
		return NULL;
	}

	read_lock(&dev_base_lock);
	dev = first_net_device(&init_net);
	while (dev) {
		if((0 == strcmp(dev->name, name)) && is_nat46(dev)) {
			if(debug) {
				printk(KERN_INFO "found [%s]\n", dev->name);
			}
			out = dev;
			break;
		}
		dev = next_net_device(dev);
	}
	read_unlock(&dev_base_lock);
	return out;
}

int nat46_create(char *devname) {
	int ret = 0;
	struct net_device *dev = find_dev(devname);
	if (dev) {
		printk("Can not add: device '%s' already exists!\n", devname);
		return -1;
	}
	ret = nat46_netdev_create(devname, &dev);
	return ret;
}

int nat46_destroy(char *devname) {
	struct net_device *dev = find_dev(devname);
	if(dev) {
		printk("Destroying '%s'\n", devname);
		nat46_netdev_destroy(dev);
		return 0;
	} else {
		printk("Could not find device '%s'\n", devname);
		return -1;
	}
}

int nat46_insert(char *devname, char *buf) {
	struct net_device *dev = find_dev(devname);
	int ret = -1;
	if(dev) {
		nat46_instance_t *nat46 = netdev_nat46_instance(dev);
		nat46_instance_t *nat46_new = alloc_nat46_instance(nat46->npairs+1, nat46, 0, 1, -1);
		if(nat46_new) {
			netdev_nat46_set_instance(dev, nat46_new);
			ret = nat46_set_ipair_config(nat46_new, 0, buf, strlen(buf));
		} else {
			printk("Could not insert a new rule on device %s\n", devname);
		}
	}
	return ret;
}

int nat46_configure(char *devname, char *buf) {
	struct net_device *dev = find_dev(devname);
	if(dev) {
		nat46_instance_t *nat46 = netdev_nat46_instance(dev);
		return nat46_set_config(nat46, buf, strlen(buf));
	} else {
		return -1;
	}
}

int nat46_remove(char *devname, char *buf) {
	int ret = -1;
	int buflen = 1024;
	char config_remove[buflen];
	struct net_device *dev;
	nat46_instance_t *nat46;
	nat46_instance_t *nat46_remove;
	int result_rem;
	int i;

	if((dev = find_dev(devname)) == NULL ||
	   (nat46 = netdev_nat46_instance(dev)) == NULL ||
	   (nat46_remove = alloc_nat46_instance(1, NULL, -1, -1, -1)) == NULL) {
		return ret;
	}

	if(nat46_set_ipair_config(nat46_remove, 0, buf, buflen) < 0) {
		release_nat46_instance(nat46_remove);
		return ret;
	}

	result_rem = nat46_get_ipair_config(nat46_remove, 0, config_remove, buflen);
	for(i = 0; i < nat46->npairs; i++) {
		char config[buflen];
		int result = nat46_get_ipair_config(nat46, i, config, buflen);

		if (result_rem == result && strncmp(config_remove, config, result_rem) == 0) {
			nat46_instance_t *nat46_new = alloc_nat46_instance(nat46->npairs-1, nat46, 0, 0, i);
			if(nat46_new) {
				netdev_nat46_set_instance(dev, nat46_new);
				ret = 0;
			} else {
				printk("Could not remove the rule from device %s\n", devname);
			}
			break;
		}
	}
	release_nat46_instance(nat46_remove);
	return ret;
}

void nat64_show_all_configs(struct seq_file *m) {
        struct net_device *dev;
	read_lock(&dev_base_lock);
	dev = first_net_device(&init_net);
	while (dev) {
		if(is_nat46(dev)) {
			nat46_instance_t *nat46 = netdev_nat46_instance(dev);
			int buflen = 1024;
			int ipair = -1;
			char *buf = kmalloc(buflen+1, GFP_KERNEL);
			seq_printf(m, "add %s\n", dev->name);
			if(buf) {
				for(ipair = 0; ipair < nat46->npairs; ipair++) {
					nat46_get_ipair_config(nat46, ipair, buf, buflen);
					if(ipair < nat46->npairs-1) {
						seq_printf(m,"insert %s %s\n", dev->name, buf);
					} else {
						seq_printf(m,"config %s %s\n", dev->name, buf);
					}
				}
				seq_printf(m,"\n");
				kfree(buf);
			}
		}
		dev = next_net_device(dev);
	}
	read_unlock(&dev_base_lock);

}

void nat46_destroy_all(void) {
        struct net_device *dev;
        struct net_device *nat46dev;
	do {
		read_lock(&dev_base_lock);
		nat46dev = NULL;
		dev = first_net_device(&init_net);
		while (dev) {
			if(is_nat46(dev)) {
				nat46dev = dev;
			}
			dev = next_net_device(dev);
		}
		read_unlock(&dev_base_lock);
		if(nat46dev) {
			nat46_netdev_destroy(nat46dev);
		}
	} while (nat46dev);

}
