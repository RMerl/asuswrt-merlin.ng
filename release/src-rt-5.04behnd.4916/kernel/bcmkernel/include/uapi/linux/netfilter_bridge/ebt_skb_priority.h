/* SPDX-License-Identifier: GPL-2.0 WITH Linux-syscall-note */
#ifndef __LINUX_BRIDGE_EBT_SKB_PRIORITY_H
#define __LINUX_BRIDGE_EBT_SKB_PRIORITY_H

struct ebt_skb_prioity_info {
	__u32 prio;
	int target;
};
#define EBT_SKB_PRIORITY_TARGET "skbpriority"

#endif
