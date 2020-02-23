/*************************************************************************
 *
 * ivi_xmit.h :
 *
 * This file is the header file for the 'ivi_xmit.c' file.
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


#ifndef IVI_XMIT_H
#define IVI_XMIT_H

#ifdef __KERNEL__

#include <linux/module.h>
#include <linux/version.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/etherdevice.h>
#include <linux/if_ether.h>
#include <linux/in.h>
#include <net/ip6_checksum.h>
#include <net/arp.h>
#include <net/ip.h>
#include <net/ipv6.h>
#include <net/ip6_route.h>
#include <linux/icmp.h>
#include <net/ndisc.h>
#include <net/route.h>

#include "ivi_config.h"
#include "ivi_rule.h"
#include "ivi_rule6.h"
#include "ivi_map.h"
#if 0
#include "ivi_map_tcp.h"
#endif
#include "ivi_nf.h"

extern __be32 v4address;
extern __be32 v4mask;
extern __be32 v4publicaddr;
extern __be32 v4publicmask;
extern __u8 v6prefix[16];
extern __be32 v6prefixlen;

extern u8 ivi_mode;

extern u8 hgw_fmt;
extern u8 hgw_transport;
extern u8 hgw_extension;

extern u16 mss_limit;

extern int ivi_v4v6_xmit(struct sk_buff *skb, unsigned int mtu, unsigned int _mtu);
extern int ivi_v6v4_xmit(struct sk_buff *skb);
extern int ivi_v4_dev(struct net_device *dev);
extern int ivi_v6_dev(struct net_device *dev);


#endif	/* __KERNEL__ */
#endif	/* IVI_XMIT_H */
