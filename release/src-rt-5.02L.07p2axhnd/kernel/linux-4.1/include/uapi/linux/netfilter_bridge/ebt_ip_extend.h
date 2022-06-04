/*
 *  ebt_ip
 *
 *	Authors:
 *	Bart De Schuymer <bart.de.schuymer@pandora.be>
 *
 *  April, 2002
 *
 *  Changes:
 *    added ip-sport and ip-dport
 *    Innominate Security Technologies AG <mhopf@innominate.com>
 *    September, 2002
 *
 *  Extend by Broadcom at Jan 31, 2019
 */
#ifndef __LINUX_BRIDGE_EBT_IP_EXTEND_H
#define __LINUX_BRIDGE_EBT_IP_EXTEND_H

#include <linux/types.h>


#define EBT_IP_TOS_EXTEND 0x01

#define EBT_IP_MASK_EXTEND (EBT_IP_TOS_EXTEND)
#define EBT_IP_MATCH_EXTEND "ip-extend"

/* the same values are used for the invflags */
struct ebt_ip_extend_info {
	__u8  tos[2];
	__u8  tosmask;
	__u8  bitmask;
	__u8  invflags;
};

#endif
