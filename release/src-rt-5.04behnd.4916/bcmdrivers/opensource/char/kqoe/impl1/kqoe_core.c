/*
 * <:copyright-BRCM:2024:DUAL/GPL:standard
 *
 *    Copyright (c) 2024 Broadcom
 *    All Rights Reserved
 *
 * Unless you and Broadcom execute a separate written software license
 * agreement governing use of this software, this software is licensed
 * to you under the terms of the GNU General Public License version 2
 * (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
 * with the following added to such license:
 *
 *    As a special exception, the copyright holders of this software give
 *    you permission to link this software with independent modules, and
 *    to copy and distribute the resulting executable under terms of your
 *    choice, provided that you also meet, for each linked independent
 *    module, the terms and conditions of the license of that module.
 *    An independent module is a module which is not derived from this
 *    software.  The special exception does not apply to any modifications
 *    of the software.
 *
 * Not withstanding the above, under no circumstances may you combine
 * this software in any way with any other Broadcom software provided
 * under a license other than the GPL, without Broadcom's express prior
 * written consent.
 *
 * :>
 */
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/etherdevice.h>
#include <linux/netlink.h>
#include <net/netlink.h>
#include <net/netfilter/nf_conntrack.h>
#include <linux/kqoe.h>
#include "kqoe_local.h"

/* --- variables --- */
struct proc_dir_entry *kq_dir;
struct kq_config kqcfg;

/* --- functions --- */
static int kq_info_seq_show(struct seq_file *s, void *v)
{
	seq_printf(s, "DS num_queues: %u\n", kqcfg.ds.num_queues);
	seq_printf(s, "US num_queues: %u\n", kqcfg.us.num_queues);
	return 0;
}
static int kq_info_open(struct inode *inode, struct file *file)
{
	return single_open(file, kq_info_seq_show, NULL);
};
static const struct proc_ops kq_info_fops = {
	.proc_open	= kq_info_open,
	.proc_read	= seq_read,
	.proc_lseek	= seq_lseek,
	.proc_release	= single_release
};

static int __init kqoe_init(void)
{
	struct proc_dir_entry *pde;
	int ret;

	ret = kq_hw_init();
	if (ret)
		goto err;

	/* create proc entries */
	kq_dir = proc_mkdir("kqoe", NULL);
	if (!kq_dir) {
		pr_err("couldn't create kqoe proc directory\n");
		ret = -ENOMEM;
		goto err_deinit_hw;
	}

	pde = proc_create("info", 0440, kq_dir, &kq_info_fops);
	if (!pde) {
		pr_err("couldn't create proc entry 'info'\n");
		goto err_free_kq_dir;
	}

	ret = kq_dscp_init();
	if (ret)
		goto err_free_proc_info;

	ret = kqnl_init();
	if (ret)
		goto err_deinit_dscp;

	return 0;

err_deinit_dscp:
	kq_dscp_exit();
err_free_proc_info:
	remove_proc_entry("info", kq_dir);
err_free_kq_dir:
	proc_remove(kq_dir);
err_deinit_hw:
	kq_hw_exit();
err:
	return ret;
}

static void __exit kqoe_exit(void)
{
	kqnl_exit();
	kq_dscp_exit();

	remove_proc_entry("info", kq_dir);
	proc_remove(kq_dir);

	kq_hw_exit();

	flush_scheduled_work();
}

module_init(kqoe_init);
module_exit(kqoe_exit);
MODULE_LICENSE("GPL");
