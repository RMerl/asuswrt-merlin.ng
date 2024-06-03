/*
 *  ebt_ip6
 *
 *	Authors:
 * Kuo-Lang Tseng <kuo-lang.tseng@intel.com>
 * Manohar Castelino <manohar.r.castelino@intel.com>
 *
 *  Jan 11, 2008
 *
 *  Extend by Broadcom at Jan 31, 2019
 */
#ifndef __LINUX_BRIDGE_EBT_IP6_EXTEND_H
#define __LINUX_BRIDGE_EBT_IP6_EXTEND_H

#include <linux/types.h>

#define EBT_IP6_TCLASS_EXTEND 0x01
#define EBT_IP6_FLOWLABEL_EXTEND 0x02
#define EBT_IP6_RANGE_SRC     0x04
#define EBT_IP6_RANGE_DST     0x08

#define EBT_IP6_MASK_EXTEND (EBT_IP6_TCLASS_EXTEND | EBT_IP6_FLOWLABEL_EXTEND  | \
	                         EBT_IP6_RANGE_SRC | EBT_IP6_RANGE_DST)
#define EBT_IP6_MATCH_EXTEND "ip6-extend"


struct ebt_ip6range {
	struct in6_addr ip_min, ip_max;
};

/* the same values are used for the invflags */
struct ebt_ip6_extend_info {
	__u8  flow_lbl[3];
	__u8  tclass[2];
	__u8  tclassmsk;
	struct ebt_ip6range range_src;
	struct ebt_ip6range range_dst;
	__u8  bitmask;
	__u8  invflags;
};

#endif
