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
#include <linux/nsproxy.h>
#include <linux/sched.h>


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
#include <net/netns/generic.h>

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,10,0)
#include <net/netfilter/ipv4/nf_defrag_ipv4.h>
#include <net/netfilter/ipv6/nf_defrag_ipv6.h>
#endif

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
int zero_csum_pass = 0;
int ip_tos_ignore = 0;
int frag_id_rewrite = 0;

module_param(debug, int, 0);
MODULE_PARM_DESC(debug, "debugging messages level (default=0)");

module_param(zero_csum_pass, int, 0);
MODULE_PARM_DESC(zero_csum_pass, "pass all-zero checksum unchanged (default=0)");

module_param(ip_tos_ignore, int, 0);
MODULE_PARM_DESC(ip_tos_ignore, "ignore IPv4 TOS and set IPv6 traffic class to zero (default=0)");

module_param(frag_id_rewrite, int, 0);
MODULE_PARM_DESC(frag_id_rewrite, "rewrite fragment id to a value inside allocated port set (default=0)");

static DEFINE_MUTEX(add_del_lock);

struct nat46_nsdata {
	struct proc_dir_entry *proc_entry;
	struct proc_dir_entry *proc_parent;
};

static unsigned int nat46_netid;


static struct net *nat46_get_net(void)
{
	struct task_struct *task = current;
	if (!task || !task->nsproxy || !task->nsproxy->net_ns)
		return NULL;

	return task->nsproxy->net_ns;
}

static int nat46_proc_show(struct seq_file *m, void *v)
{
	struct net *net;

	net = (struct net *)m->private;
	nat64_show_all_configs(net, m);
	return 0;
}

static int nat46_proc_open(struct inode *inode, struct file *file)
{
	struct net *net;

	net = nat46_get_net();
	if (!net)
		return -EFAULT;

	return single_open(file, nat46_proc_show, net);
}

static char *get_devname(char **ptail)
{
	const int maxlen = IFNAMSIZ-1;
	char *devname = get_next_arg(ptail);
	if(devname && (strlen(devname) > maxlen)) {
		printk(KERN_INFO "nat46: '%s' is "
			"longer than %d chars, truncating\n", devname, maxlen);
		devname[maxlen] = 0;
	}
	return devname;
}

static ssize_t nat46_proc_write(struct file *file, const char __user *buffer,
                              size_t count, loff_t *ppos)
{
	struct net *net;
	char *buf = NULL;
	char *tail = NULL;
	char *devname = NULL;
	char *arg_name = NULL;

	net = nat46_get_net();
	if (!net)
		return -EFAULT;

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
			mutex_lock(&add_del_lock);
			nat46_create(net, devname);
			mutex_unlock(&add_del_lock);
		} else if (0 == strcmp(arg_name, "del")) {
			devname = get_devname(&tail);
			printk(KERN_INFO "nat46: deleting device (%s)\n", devname);
			mutex_lock(&add_del_lock);
			nat46_destroy(net, devname);
			mutex_unlock(&add_del_lock);
		} else if (0 == strcmp(arg_name, "config")) {
			devname = get_devname(&tail);
			printk(KERN_INFO "nat46: configure device (%s) with '%s'\n", devname, tail);
			mutex_lock(&add_del_lock);
			nat46_configure(net, devname, tail);
			mutex_unlock(&add_del_lock);
		} else if (0 == strcmp(arg_name, "insert")) {
			devname = get_devname(&tail);
			printk(KERN_INFO "nat46: insert new rule into device (%s) with '%s'\n", devname, tail);
			mutex_lock(&add_del_lock);
			nat46_insert(net, devname, tail);
			mutex_unlock(&add_del_lock);
		} else if (0 == strcmp(arg_name, "remove")) {
			devname = get_devname(&tail);
			printk(KERN_INFO "nat46: remove a rule from the device (%s) with '%s'\n", devname, tail);
			mutex_lock(&add_del_lock);
			nat46_remove(net, devname, tail);
			mutex_unlock(&add_del_lock);
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


static int __net_init nat46_ns_init(struct net *net)
{
	struct nat46_nsdata *nsdata;

	nsdata = net_generic(net, nat46_netid);
	nsdata->proc_parent = proc_mkdir(NAT46_PROC_NAME, net->proc_net);
	if (nsdata->proc_parent) {
		nsdata->proc_entry = proc_create(NAT46_CONTROL_PROC_NAME, 0644, nsdata->proc_parent, &nat46_proc_fops);
		if(!nsdata->proc_entry) {
			remove_proc_entry(NAT46_PROC_NAME, net->proc_net);
			nsdata->proc_parent = NULL;
			printk(KERN_INFO "Error creating proc entry");
			return -ENOMEM;
		}
	}
	return 0;
}

static void __net_exit nat46_ns_exit(struct net *net)
{
	struct nat46_nsdata *nsdata;

	nat46_destroy_all(net);

	nsdata = net_generic(net, nat46_netid);
	if (nsdata->proc_parent) {
		if (nsdata->proc_entry) {
			remove_proc_entry(NAT46_CONTROL_PROC_NAME, nsdata->proc_parent);
		}
		remove_proc_entry(NAT46_PROC_NAME, net->proc_net);
	}
}

static struct pernet_operations nat46_net_ops = {
  .init = nat46_ns_init,
  .exit = nat46_ns_exit,
  .id = &nat46_netid,
  .size = sizeof(struct nat46_nsdata),
};

static int __init nat46_init(void)
{
	int ret = 0;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,10,0)
	struct net *net;

	net = nat46_get_net();
	if (!net) {
		return -EFAULT;
	}
#endif

	printk("nat46: module (version %s) loaded.\n", NAT46_VERSION);
	ret = register_pernet_subsys(&nat46_net_ops);
	if(ret) {
		goto error;
	}
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,10,0)
	ret = nf_defrag_ipv4_enable(net);
	if(ret) {
		goto error;
	}
	ret = nf_defrag_ipv6_enable(net);
	if(ret) {
		goto error;
	}
#endif
	init_frag_list(15);
	return 0;

error:
	return ret;
}

static void __exit nat46_exit(void)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,13,0)
	struct net *net;
#endif

	free_frag_list();
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,13,0)
	net = nat46_get_net();
	if (net) {
		nf_defrag_ipv6_disable(net);
		nf_defrag_ipv4_disable(net);
	}
#endif
	unregister_pernet_subsys(&nat46_net_ops);
	printk("nat46: module unloaded.\n");
}

module_init(nat46_init);
module_exit(nat46_exit);


