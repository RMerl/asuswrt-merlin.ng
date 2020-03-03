/*
 * Intel SST generic IPC Support
 *
 * Copyright (C) 2015, Intel Corporation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License version
 * 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#ifndef __SST_GENERIC_IPC_H
#define __SST_GENERIC_IPC_H

#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/wait.h>
#include <linux/list.h>
#include <linux/workqueue.h>
#include <linux/sched.h>
#include <linux/kthread.h>

#define IPC_MAX_MAILBOX_BYTES	256

struct ipc_message {
	struct list_head list;
	u64 header;

	/* direction wrt host CPU */
	char tx_data[IPC_MAX_MAILBOX_BYTES];
	size_t tx_size;
	char rx_data[IPC_MAX_MAILBOX_BYTES];
	size_t rx_size;

	wait_queue_head_t waitq;
	bool pending;
	bool complete;
	bool wait;
	int errno;
};

struct sst_generic_ipc;

struct sst_plat_ipc_ops {
	void (*tx_msg)(struct sst_generic_ipc *, struct ipc_message *);
	void (*shim_dbg)(struct sst_generic_ipc *, const char *);
	void (*tx_data_copy)(struct ipc_message *, char *, size_t);
	u64  (*reply_msg_match)(u64 header, u64 *mask);
};

/* SST generic IPC data */
struct sst_generic_ipc {
	struct device *dev;
	struct sst_dsp *dsp;

	/* IPC messaging */
	struct list_head tx_list;
	struct list_head rx_list;
	struct list_head empty_list;
	wait_queue_head_t wait_txq;
	struct task_struct *tx_thread;
	struct kthread_worker kworker;
	struct kthread_work kwork;
	bool pending;
	struct ipc_message *msg;

	struct sst_plat_ipc_ops ops;
};

int sst_ipc_tx_message_wait(struct sst_generic_ipc *ipc, u64 header,
	void *tx_data, size_t tx_bytes, void *rx_data, size_t rx_bytes);

int sst_ipc_tx_message_nowait(struct sst_generic_ipc *ipc, u64 header,
	void *tx_data, size_t tx_bytes);

struct ipc_message *sst_ipc_reply_find_msg(struct sst_generic_ipc *ipc,
	u64 header);

void sst_ipc_tx_msg_reply_complete(struct sst_generic_ipc *ipc,
	struct ipc_message *msg);

void sst_ipc_drop_all(struct sst_generic_ipc *ipc);
int sst_ipc_init(struct sst_generic_ipc *ipc);
void sst_ipc_fini(struct sst_generic_ipc *ipc);

#endif
