/*************************************************************************
 *
 * ivi_ioctl.c :
 *   
 * MAP-T/MAP-E Configuration Interface Kernel Module
 *
 * Copyright (C) 2013 CERNET Network Center
 * All rights reserved.
 * 
 * Design and coding: 
 *   Xing Li <xing@cernet.edu.cn> 
 *	 Congxiao Bao <congxiao@cernet.edu.cn>
 *   Guoliang Han <bupthgl@gmail.com>
 * 	 Yuncheng Zhu <haoyu@cernet.edu.cn>
 * 	 Wentao Shang <wentaoshang@gmail.com>
 * 	 
 * 
 * Contributions:
 *
 * This file is part of MAP-T/MAP-E Kernel Module.
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * You should have received a copy of the GNU General Public License 
 * along with MAP-T/MAP-E Kernel Module. If not, see 
 * <http://www.gnu.org/licenses/>.
 *
 * For more versions, please send an email to <bupthgl@gmail.com> to
 * obtain an password to access the svn server.
 *
 * LIC: GPLv2
 *
 ************************************************************************/

#include <linux/module.h>
#include <linux/ioctl.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/netdevice.h>
#include <linux/uaccess.h>

#include "ivi_nf.h"
#include "ivi_xmit.h"
#include "ivi_ioctl.h"
#include "ivi_rule.h"
#include "ivi_rule6.h"
#include "ivi_config.h"
#include "ivi_portmap.h"

static long ivi_ioctl(struct file *file, unsigned int cmd, unsigned long arg) {
	int retval = 0;
	struct net_device *dev;
	char temp[IVI_IOCTL_LEN];
	struct rule_info rule;
	struct portmap_info portmap;
	
	switch (cmd) {
		case IVI_IOC_V4DEV:
			if (copy_from_user(temp, (char *)arg, IVI_IOCTL_LEN) > 0) {
				return -EACCES;
			}
			temp[IVI_IOCTL_LEN - 1] = 0;
			dev = dev_get_by_name(&init_net, temp);
			if (dev == NULL) {
				return -ENODEV;
			}
			retval = nf_getv4dev(dev);
			printk(KERN_INFO "ivi_ioctl: v4 device set to %s.\n", temp);
			break;
		
		case IVI_IOC_V6DEV:
			if (copy_from_user(temp, (char *)arg, IVI_IOCTL_LEN) > 0) {
				return -EACCES;
			}
			temp[IVI_IOCTL_LEN - 1] = 0;
			dev = dev_get_by_name(&init_net, temp);
			if (dev == NULL) {
				return -ENODEV;
			}
			retval = nf_getv6dev(dev);
			printk(KERN_INFO "ivi_ioctl: v6 device set to %s.\n", temp);
			break;
		
		case IVI_IOC_START:
			retval = nf_running(1);
			break;
		
		case IVI_IOC_STOP:
			retval = nf_running(0);
			break;
		
		case IVI_IOC_V4NET:
			if (copy_from_user(&v4address, (__be32 *)arg, sizeof(__be32)) > 0) {
				return -EACCES;
			}
			printk(KERN_INFO "ivi_ioctl: v4 address set to %08x.\n", v4address);
			break;
		
		case IVI_IOC_V4MASK:
			if (copy_from_user(&v4mask, (__be32 *)arg, sizeof(__be32)) > 0) {
				return -EACCES;
			}
			printk(KERN_INFO "ivi_ioctl: v4 address mask set to %08x.\n", v4mask);
			break;
		
		case IVI_IOC_V6NET:
			if (copy_from_user(v6prefix, (__u8 *)arg, 16) > 0) {
				return -EACCES;
			}
			printk(KERN_INFO "ivi_ioctl: v6 prefix set to %04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x.\n", 
				ntohs(((__be16 *)v6prefix)[0]), ntohs(((__be16 *)v6prefix)[1]), ntohs(((__be16 *)v6prefix)[2]), ntohs(((__be16 *)v6prefix)[3]), 
				ntohs(((__be16 *)v6prefix)[4]), ntohs(((__be16 *)v6prefix)[5]), ntohs(((__be16 *)v6prefix)[6]), ntohs(((__be16 *)v6prefix)[7]));
			break;
		
		case IVI_IOC_V6MASK:
			if (copy_from_user(&v6prefixlen, (__be32 *)arg, sizeof(__be32)) > 0) {
				return -EACCES;
			}
			printk(KERN_INFO "ivi_ioctl: v6 prefix length set to %d.\n", v6prefixlen);
			break;
		
		case IVI_IOC_V4PUB:
			if (copy_from_user(&v4publicaddr, (__be32 *)arg, sizeof(__be32)) > 0) {
				return -EACCES;
			}
			printk(KERN_INFO "ivi_ioctl: v4 public address set to %08x.\n", v4publicaddr);
			break;
			
		case IVI_IOC_V4PUBMASK:
			if (copy_from_user(&v4publicmask, (__be32 *)arg, sizeof(__be32)) > 0) {
				return -EACCES;
			}
			printk(KERN_INFO "ivi_ioctl: v4 public address mask set to %08x.\n", v4publicmask);
			break;
			
		case IVI_IOC_NAT:
			ivi_mode = IVI_MODE_HGW_NAT44;
			printk(KERN_INFO "ivi_ioctl: ivi_mode set to hgw with nat44 enabled.\n");
			break;
		
		case IVI_IOC_NONAT:
			ivi_mode = IVI_MODE_HGW;
			printk(KERN_INFO "ivi_ioctl: ivi_mode set to hgw with nat44 disabled.\n");
			break;

		case IVI_IOC_HGW_MAPX:
			hgw_fmt = ADDR_FMT_MAPX_CPE;
			printk(KERN_INFO "ivi_ioctl: addr_fmt set to %d.\n", hgw_fmt);
			break;

		case IVI_IOC_ADJACENT:
			if (copy_from_user(&hgw_adjacent, (u16 *)arg, sizeof(u16)) > 0) {
				return -EACCES;
			}
			printk(KERN_INFO "ivi_ioctl: adjacent set to %d.\n", hgw_adjacent);
			break;

		case IVI_IOC_MAPT:
			if (copy_from_user(&hgw_ratio, (u16 *)arg, sizeof(u16)) > 0) {
				return -EACCES;
			}
			printk(KERN_INFO "ivi_ioctl: ratio set to %d.\n", hgw_ratio);
			if (copy_from_user(&hgw_offset, ((u16 *)arg) + 1, sizeof(u16)) > 0) {
				return -EACCES;
			}
			printk(KERN_INFO "ivi_ioctl: offset set to %d.\n", hgw_offset);
			
			hgw_suffix = hgw_offset;
			printk(KERN_INFO "ivi_ioctl: suffix set to %04x.\n", hgw_suffix);
			hgw_fmt = ADDR_FMT_MAPT;
			printk(KERN_INFO "ivi_ioctl: addr_fmt set to %d.\n", hgw_fmt);
			break;
		
		case IVI_IOC_MSS_LIMIT:
			if (copy_from_user(&mss_limit, (u16 *)arg, sizeof(u16)) > 0) {
				return -EACCES;
			}
			printk(KERN_INFO "ivi_ioctl: mss limit set to %d.\n", mss_limit);
			break;

		case IVI_IOC_ADD_RULE:
			if (copy_from_user(&rule, (void *)arg, sizeof(struct rule_info)) > 0) {
				return -EACCES;
			}
			if (ivi_rule_insert(&rule) != 0) {
				printk(KERN_DEBUG "ivi_ioctl: fail to insert " NIP4_FMT "/%d -> " NIP6_FMT "/%d\n", 
						NIP4(rule.prefix4), rule.plen4, NIP6(rule.prefix6), rule.plen6);
				return -EINVAL;
			}
			if (ivi_rule6_insert(&rule) != 0) {
				printk(KERN_DEBUG "ivi_ioctl: fail to insert " NIP6_FMT " -> %d, address format %d\n", 
					NIP6(rule.prefix6), rule.plen6, rule.format);
				return -EINVAL;
			}
			break;
			
		case IVI_IOC_TRANSPT:
			if (copy_from_user(&hgw_transport, (u8 *)arg, sizeof(u8)) > 0) {
				return -EACCES;
			}
			printk(KERN_INFO "ivi_ioctl: transport set to %d.\n", hgw_transport);
			if (copy_from_user(&hgw_extension, (u8 *)arg + 1, sizeof(u8)) > 0) {
				return -EACCES;
			}
			printk(KERN_INFO "ivi_ioctl: extension set to %d.\n", hgw_extension);
			break;
				
		case IVI_IOC_ADD_PORTMAP:
			{
			u32 idx, intPort;

			if (copy_from_user(&portmap, (void *)arg, sizeof(struct portmap_info)) > 0)
				return -EACCES;

			intPort = portmap.intPort;
			portmap.lanAddr = htonl(portmap.lanAddr);
			portmap.wanAddr = htonl(portmap.wanAddr);
			idx = mapportmap_lookup(&(portmap.lanAddr), portmap.wanAddr, portmap.port, &intPort, 
					portmap.proto, MAPPORTMAP_MODE_ADD);
			if (idx == MAPPORTMAP_IX_INVALID) {
				printk(KERN_DEBUG "ivi_ioctl: fail to insert portmap " NIP4_FMT " " NIP4_FMT " %d %d %d\n", 
						NIP4(portmap.lanAddr), NIP4(portmap.wanAddr), portmap.port, portmap.intPort, portmap.proto);
				return -EINVAL;
			}
			break;
			}
			
		case IVI_IOC_DEL_PORTMAP:
			{
			u32 idx, intPort;

			if (copy_from_user(&portmap, (void *)arg, sizeof(struct portmap_info)) > 0)
				return -EACCES;

			intPort = portmap.intPort;
			portmap.lanAddr = htonl(portmap.lanAddr);
			portmap.wanAddr = htonl(portmap.wanAddr);
			idx = mapportmap_lookup(&(portmap.lanAddr), portmap.wanAddr, portmap.port, &intPort, 
					portmap.proto, MAPPORTMAP_MODE_DEL);
			if (idx == MAPPORTMAP_IX_INVALID) {
				printk(KERN_DEBUG "ivi_ioctl: fail to delete portmap, does not exist! " NIP4_FMT " " NIP4_FMT " %d %d %d\n", 
						NIP4(portmap.lanAddr), NIP4(portmap.wanAddr), portmap.port, portmap.intPort, portmap.proto);
				return -EINVAL;
			}
			else
				mapportmap_delete(idx, portmap.proto);

			break;
			}
			
		default:
			retval = -ENOTTY;
	}
	return retval;
}

static int ivi_open(struct inode *inode, struct file *file) {
#ifdef IVI_DEBUG
	printk(KERN_DEBUG "IVI: virtual device is opened for ioctl.\n");
#endif
	return 0;
}

static int ivi_release(struct inode *inode, struct file *file) {
#ifdef IVI_DEBUG
	printk(KERN_DEBUG "IVI: virtual device is closed.\n");
#endif
	return 0;
}

static dev_t devno;
static struct cdev ivi_cdev;
static struct class *ivi_class = NULL;
static struct device *ivi_device = NULL;

struct file_operations ivi_ops = {
	.owner		=	THIS_MODULE,
	.unlocked_ioctl = ivi_ioctl,
#if defined(CONFIG_COMPAT)
    .compat_ioctl = ivi_ioctl,
#endif
	.open		=	ivi_open,
	.release	=	ivi_release,
};

int ivi_ioctl_init(void) {
	int retval;
	if ((retval = alloc_chrdev_region(&devno, 0, 1, IVI_DEVNAME)) < 0) {
		printk(KERN_ERR "IVI: failed to register ioctl as character device, code %d.\n", retval);
	}
	if (IS_ERR(ivi_class = class_create(THIS_MODULE, IVI_DEVNAME))) {
		retval = PTR_ERR(ivi_class);
		printk(KERN_ERR "IVI: failed to create device class, code %d.\n", retval);
		goto out_chrdev;
	}
	if (IS_ERR(ivi_device = device_create(ivi_class, NULL, devno, NULL, IVI_DEVNAME))) {
		retval = PTR_ERR(ivi_device);
		printk(KERN_ERR "IVI: failed to create device, code %d.\n", retval);
		goto out_class;
	}
	cdev_init(&ivi_cdev, &ivi_ops);
	ivi_cdev.owner = THIS_MODULE;
	if ((retval = cdev_add(&ivi_cdev, devno, 1)) < 0) {
		printk(KERN_ERR "IVI: failed to add driver, code %d.\n", retval);
		goto out_device;
	}
#ifdef IVI_DEBUG
	printk(KERN_DEBUG "IVI: ivi_ioctl loaded with return value %d.\n", retval);
#endif
	goto out;
out_device:
	device_destroy(ivi_class, devno);
out_class:
	class_destroy(ivi_class);
out_chrdev:
	unregister_chrdev_region(devno, 1);
out:
	return retval;
}

void ivi_ioctl_exit(void) {
	cdev_del(&ivi_cdev);
	device_destroy(ivi_class, devno);
	class_destroy(ivi_class);
	unregister_chrdev_region(devno, 1);
#ifdef IVI_DEBUG
	printk(KERN_DEBUG "IVI: ivi_ioctl unloaded.\n");
#endif
}
