/*
 *
 * module-wide functions, mostly boilerplate
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

#include <linux/version.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/ip.h>
#include <linux/icmp.h>
#include <linux/inet.h>
#include <linux/icmpv6.h>
#include <linux/inetdevice.h>
#include <linux/types.h>
#include <linux/netfilter_ipv4.h>


#include <linux/fs.h>           // for basic filesystem
#include <linux/proc_fs.h>      // for the proc filesystem
#include <linux/seq_file.h>     // for sequence files

#include <net/ip.h>
#include <net/tcp.h>
#include <net/udp.h>
#include <net/icmp.h>
#include <net/route.h>
#include <net/ip6_route.h>

#include <net/ipv6.h>

#include "nat46-core.h"
#include "nat46-netdev.h"

#define NAT46_PROC_NAME	"nat46"
#define NAT46_CONTROL_PROC_NAME "control"

#ifndef NAT46_VERSION
#define NAT46_VERSION __DATE__ " " __TIME__
#endif

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Andrew Yourtchenko <ayourtch@gmail.com>");
MODULE_DESCRIPTION("NAT46 stateless translation");

int debug = 0;
module_param(debug, int, 0);
MODULE_PARM_DESC(debug, "debugging messages level (default=1)");

bool nullcheck = 0;
module_param(nullcheck, bool, 0);
MODULE_PARM_DESC(nullcheck, "retaining UDP checksum 0 value when translating to IPv4 (default=0)");

static struct proc_dir_entry *nat46_proc_entry;
static struct proc_dir_entry *nat46_proc_parent;


static int nat46_proc_show(struct seq_file *m, void *v)
{
	nat64_show_all_configs(m);
	return 0;
}


static int nat46_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, nat46_proc_show, NULL);
}

static char *get_devname(char **ptail)
{
	const int maxlen = IFNAMSIZ-1;
	char *devname = get_next_arg(ptail);
	if(strlen(devname) > maxlen) {
		printk(KERN_INFO "nat46: '%s' is "
			"longer than %d chars, truncating\n", devname, maxlen);
		devname[maxlen] = 0;
	}
	return devname;
}

static ssize_t nat46_proc_write(struct file *file, const char __user *buffer,
                              size_t count, loff_t *ppos)
{
	char *buf = NULL;
	char *tail = NULL;
	char *devname = NULL;
	char *arg_name = NULL;

	buf = kmalloc(sizeof(char) * (count + 1), GFP_KERNEL);
	if (!buf)
		return -ENOMEM;

	if (copy_from_user(buf, buffer, count)) {
		kfree(buf);
		return -EFAULT;
	}
	tail = buf;
	buf[count] = '\0';
	if( (count > 0) && (buf[count-1] == '\n') ) {
		buf[count-1] = '\0';
	}

	while (NULL != (arg_name = get_next_arg(&tail))) {
		if (0 == strcmp(arg_name, "add")) {
			devname = get_devname(&tail);
			printk(KERN_INFO "nat46: adding device (%s)\n", devname);
			nat46_create(devname);
		} else if (0 == strcmp(arg_name, "del")) {
			devname = get_devname(&tail);
			printk(KERN_INFO "nat46: deleting device (%s)\n", devname);
			nat46_destroy(devname);
		} else if (0 == strcmp(arg_name, "config")) {
			devname = get_devname(&tail);
			printk(KERN_INFO "nat46: configure device (%s) with '%s'\n", devname, tail);
			nat46_configure(devname, tail);
		} else if (0 == strcmp(arg_name, "insert")) {
			devname = get_devname(&tail);
			printk(KERN_INFO "nat46: insert new rule into device (%s) with '%s'\n", devname, tail);
			nat46_insert(devname, tail);
		} else if (0 == strcmp(arg_name, "remove")) {
			devname = get_devname(&tail);
			printk(KERN_INFO "nat46: remove a rule from the device (%s) with '%s'\n", devname, tail);
			nat46_remove(devname, tail);
		}
	}

	kfree(buf);
	return count;
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(5,6,0)
static const struct file_operations nat46_proc_fops = {
	.owner		= THIS_MODULE,
	.open		= nat46_proc_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
	.write		= nat46_proc_write,
};
#else
static const struct proc_ops nat46_proc_fops = {
	.proc_open	= nat46_proc_open,
	.proc_read	= seq_read,
	.proc_lseek	= seq_lseek,
	.proc_release	= single_release,
	.proc_write	= nat46_proc_write,
};
#endif


int create_nat46_proc_entry(void) {
	nat46_proc_parent = proc_mkdir(NAT46_PROC_NAME, init_net.proc_net);
	if (nat46_proc_parent) {
		nat46_proc_entry = proc_create(NAT46_CONTROL_PROC_NAME, 0644, nat46_proc_parent, &nat46_proc_fops );
		if(!nat46_proc_entry) {
			printk(KERN_INFO "Error creating proc entry");
			return -ENOMEM;
		}
	}
	return 0;
}


static int __init nat46_init(void)
{
	int ret = 0;

	printk("nat46: module (version %s) loaded.\n", NAT46_VERSION);
	ret = create_nat46_proc_entry();
	if(ret) {
		goto error;
	}
	return 0;

error:
	return ret;
}

static void __exit nat46_exit(void)
{
	nat46_destroy_all();
	if (nat46_proc_parent) {
		if (nat46_proc_entry) {
			remove_proc_entry(NAT46_CONTROL_PROC_NAME, nat46_proc_parent);
		}
		remove_proc_entry(NAT46_PROC_NAME, init_net.proc_net);
	}
	printk("nat46: module unloaded.\n");
}

module_init(nat46_init);
module_exit(nat46_exit);


