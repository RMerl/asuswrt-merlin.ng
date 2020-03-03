/*
 *  Copyright (C) 2007
 *
 *  Author: Eric Biederman <ebiederm@xmision.com>
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License as
 *  published by the Free Software Foundation, version 2 of the
 *  License.
 */

#include <linux/module.h>
#include <linux/ipc.h>
#include <linux/nsproxy.h>
#include <linux/sysctl.h>
#include <linux/uaccess.h>
#include <linux/ipc_namespace.h>
#include <linux/msg.h>
#include "util.h"

static void *get_ipc(struct ctl_table *table)
{
	char *which = table->data;
	struct ipc_namespace *ipc_ns = current->nsproxy->ipc_ns;
	which = (which - (char *)&init_ipc_ns) + (char *)ipc_ns;
	return which;
}

#ifdef CONFIG_PROC_SYSCTL
static int proc_ipc_dointvec(struct ctl_table *table, int write,
	void __user *buffer, size_t *lenp, loff_t *ppos)
{
	struct ctl_table ipc_table;

	memcpy(&ipc_table, table, sizeof(ipc_table));
	ipc_table.data = get_ipc(table);

	return proc_dointvec(&ipc_table, write, buffer, lenp, ppos);
}

static int proc_ipc_dointvec_minmax(struct ctl_table *table, int write,
	void __user *buffer, size_t *lenp, loff_t *ppos)
{
	struct ctl_table ipc_table;

	memcpy(&ipc_table, table, sizeof(ipc_table));
	ipc_table.data = get_ipc(table);

	return proc_dointvec_minmax(&ipc_table, write, buffer, lenp, ppos);
}

static int proc_ipc_dointvec_minmax_orphans(struct ctl_table *table, int write,
	void __user *buffer, size_t *lenp, loff_t *ppos)
{
	struct ipc_namespace *ns = current->nsproxy->ipc_ns;
	int err = proc_ipc_dointvec_minmax(table, write, buffer, lenp, ppos);

	if (err < 0)
		return err;
	if (ns->shm_rmid_forced)
		shm_destroy_orphaned(ns);
	return err;
}

static int proc_ipc_doulongvec_minmax(struct ctl_table *table, int write,
	void __user *buffer, size_t *lenp, loff_t *ppos)
{
	struct ctl_table ipc_table;
	memcpy(&ipc_table, table, sizeof(ipc_table));
	ipc_table.data = get_ipc(table);

	return proc_doulongvec_minmax(&ipc_table, write, buffer,
					lenp, ppos);
}

static int proc_ipc_auto_msgmni(struct ctl_table *table, int write,
	void __user *buffer, size_t *lenp, loff_t *ppos)
{
	struct ctl_table ipc_table;
	int dummy = 0;

	memcpy(&ipc_table, table, sizeof(ipc_table));
	ipc_table.data = &dummy;

	if (write)
		pr_info_once("writing to auto_msgmni has no effect");

	return proc_dointvec_minmax(&ipc_table, write, buffer, lenp, ppos);
}

#else
#define proc_ipc_doulongvec_minmax NULL
#define proc_ipc_dointvec	   NULL
#define proc_ipc_dointvec_minmax   NULL
#define proc_ipc_dointvec_minmax_orphans   NULL
#define proc_ipc_auto_msgmni	   NULL
#endif

static int zero;
static int one = 1;
static int int_max = INT_MAX;

static struct ctl_table ipc_kern_table[] = {
	{
		.procname	= "shmmax",
		.data		= &init_ipc_ns.shm_ctlmax,
		.maxlen		= sizeof(init_ipc_ns.shm_ctlmax),
		.mode		= 0644,
		.proc_handler	= proc_ipc_doulongvec_minmax,
	},
	{
		.procname	= "shmall",
		.data		= &init_ipc_ns.shm_ctlall,
		.maxlen		= sizeof(init_ipc_ns.shm_ctlall),
		.mode		= 0644,
		.proc_handler	= proc_ipc_doulongvec_minmax,
	},
	{
		.procname	= "shmmni",
		.data		= &init_ipc_ns.shm_ctlmni,
		.maxlen		= sizeof(init_ipc_ns.shm_ctlmni),
		.mode		= 0644,
		.proc_handler	= proc_ipc_dointvec,
	},
	{
		.procname	= "shm_rmid_forced",
		.data		= &init_ipc_ns.shm_rmid_forced,
		.maxlen		= sizeof(init_ipc_ns.shm_rmid_forced),
		.mode		= 0644,
		.proc_handler	= proc_ipc_dointvec_minmax_orphans,
		.extra1		= &zero,
		.extra2		= &one,
	},
	{
		.procname	= "msgmax",
		.data		= &init_ipc_ns.msg_ctlmax,
		.maxlen		= sizeof(init_ipc_ns.msg_ctlmax),
		.mode		= 0644,
		.proc_handler	= proc_ipc_dointvec_minmax,
		.extra1		= &zero,
		.extra2		= &int_max,
	},
	{
		.procname	= "msgmni",
		.data		= &init_ipc_ns.msg_ctlmni,
		.maxlen		= sizeof(init_ipc_ns.msg_ctlmni),
		.mode		= 0644,
		.proc_handler	= proc_ipc_dointvec_minmax,
		.extra1		= &zero,
		.extra2		= &int_max,
	},
	{
		.procname	= "auto_msgmni",
		.data		= NULL,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= proc_ipc_auto_msgmni,
		.extra1		= &zero,
		.extra2		= &one,
	},
	{
		.procname	=  "msgmnb",
		.data		= &init_ipc_ns.msg_ctlmnb,
		.maxlen		= sizeof(init_ipc_ns.msg_ctlmnb),
		.mode		= 0644,
		.proc_handler	= proc_ipc_dointvec_minmax,
		.extra1		= &zero,
		.extra2		= &int_max,
	},
	{
		.procname	= "sem",
		.data		= &init_ipc_ns.sem_ctls,
		.maxlen		= 4*sizeof(int),
		.mode		= 0644,
		.proc_handler	= proc_ipc_dointvec,
	},
#ifdef CONFIG_CHECKPOINT_RESTORE
	{
		.procname	= "sem_next_id",
		.data		= &init_ipc_ns.ids[IPC_SEM_IDS].next_id,
		.maxlen		= sizeof(init_ipc_ns.ids[IPC_SEM_IDS].next_id),
		.mode		= 0644,
		.proc_handler	= proc_ipc_dointvec_minmax,
		.extra1		= &zero,
		.extra2		= &int_max,
	},
	{
		.procname	= "msg_next_id",
		.data		= &init_ipc_ns.ids[IPC_MSG_IDS].next_id,
		.maxlen		= sizeof(init_ipc_ns.ids[IPC_MSG_IDS].next_id),
		.mode		= 0644,
		.proc_handler	= proc_ipc_dointvec_minmax,
		.extra1		= &zero,
		.extra2		= &int_max,
	},
	{
		.procname	= "shm_next_id",
		.data		= &init_ipc_ns.ids[IPC_SHM_IDS].next_id,
		.maxlen		= sizeof(init_ipc_ns.ids[IPC_SHM_IDS].next_id),
		.mode		= 0644,
		.proc_handler	= proc_ipc_dointvec_minmax,
		.extra1		= &zero,
		.extra2		= &int_max,
	},
#endif
	{}
};

static struct ctl_table ipc_root_table[] = {
	{
		.procname	= "kernel",
		.mode		= 0555,
		.child		= ipc_kern_table,
	},
	{}
};

static int __init ipc_sysctl_init(void)
{
	register_sysctl_table(ipc_root_table);
	return 0;
}

device_initcall(ipc_sysctl_init);
