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
#ifndef _KQOE_LOCAL_H
#define _KQOE_LOCAL_H

#include <linux/init.h>
#include <linux/types.h>

struct kq_bw {
	u64	min;
	u64	max;
};

struct kq_cfg_dir {
	u8			num_queues;
	u16			bypass;
	struct kq_bw		bw;
};

struct kq_config {
	struct kq_cfg_dir	us;
	struct kq_cfg_dir	ds;
};

struct kq_queue {
	u16		id;
	struct kq_bw	bw;
	u32		status;
};

struct proc_dir_entry;

extern struct kq_config		kqcfg;
extern struct proc_dir_entry	*kq_dir;

struct nf_conn;

/* dscp functions */
int kq_dscp_rewrite(struct nf_conn *ct, u8 dscp);
int __init kq_dscp_init(void);
void kq_dscp_exit(void);

/* netlink functions */
int __init kqnl_init(void);
void kqnl_exit(void);

/* hardware functions */
int __init kq_hw_init(void);
void kq_hw_exit(void);
int kq_hw_queue_fill(struct kq_queue *q, u8 dir);
int kq_hw_queue_set(struct kq_queue *q, u8 dir);
int kq_hw_config_fill(struct kq_cfg_dir *c, u8 dir);
int kq_hw_config_set(struct kq_cfg_dir *c, u8 dir);

#endif /* _KQOE_LOCAL_H */
