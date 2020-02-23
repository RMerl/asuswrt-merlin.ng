/*************************************************************************
 *
 * ivi_config.h :
 *   
 * MAP-T/MAP-E Configuration Header File
 *
 * Copyright (C) 2013 CERNET Network Center
 * All rights reserved.
 * 
 * Design and coding: 
 *   Xing Li <xing@cernet.edu.cn> 
 *	 Congxiao Bao <congxiao@cernet.edu.cn>
 * 	 Yuncheng Zhu <haoyu@cernet.edu.cn>
 * 	 Wentao Shang <wentaoshang@gmail.com>
 * 	 Guoliang Han <bupthgl@gmail.com>
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

#ifndef IVI_CONFIG_H
#define IVI_CONFIG_H

#include <linux/types.h>
#ifdef __KERNEL__
#include <net/ipv6.h>
#endif

#define ADDR_FMT_NONE       0   // DMR format
#define ADDR_FMT_MAPT       1   // BMR/FMR format
#define ADDR_FMT_MAPX_CPE   2   // MAP CPE format with no eabits

#define MAP_T   0  // Header translation
#define MAP_E	1  // Header encapsulation mode 1: BR address is specified as a /128

#define TCP_MAX_LOOP_NUM 20
#define UDP_MAX_LOOP_NUM 6

#ifndef IFNAMSIZ
#define IFNAMSIZ 16
#endif

// comment this line out if you don't want to compile this code for the Linksys environment
//#define LINKSYS_COMPILE
//#define TP_LINK_COMPILE

#ifdef LINKSYS_COMPILE
static inline u32 get_unaligned_be32(const void *p) {
	u32 tmp;
	memcpy(&tmp, p, 4);
	return tmp;
}
#endif

#ifndef NIP4

#define NIP4_FMT "%u.%u.%u.%u"

#ifdef TP_LINK_COMPILE
#define NIP4(addr) \
	((unsigned char *)&addr)[0], \
	((unsigned char *)&addr)[1], \
	((unsigned char *)&addr)[2], \
	((unsigned char *)&addr)[3]
#else
#define NIP4(addr) \
	((unsigned char *)&addr)[3], \
	((unsigned char *)&addr)[2], \
	((unsigned char *)&addr)[1], \
	((unsigned char *)&addr)[0]
#endif

#endif

#ifndef NIP6
#define NIP6(addr) \
	ntohs((addr).s6_addr16[0]), \
	ntohs((addr).s6_addr16[1]), \
	ntohs((addr).s6_addr16[2]), \
	ntohs((addr).s6_addr16[3]), \
	ntohs((addr).s6_addr16[4]), \
	ntohs((addr).s6_addr16[5]), \
	ntohs((addr).s6_addr16[6]), \
	ntohs((addr).s6_addr16[7])
#define NIP6_FMT "%04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x"
#endif

struct rule_info {
	__u32 prefix4;
	int plen4;
	struct in6_addr prefix6;
	int plen6;
	__u16 ratio;
	__u16 adjacent;
	__u8 format;
	__u8 transport;
	__u8 extension;
};

#define MAPT_PORTMAP_TCP	0
#define MAPT_PORTMAP_UDP	1
#define MAPT_PORTMAP_ICMP	2

struct portmap_info {
	__u32 lanAddr;
	__u32 wanAddr;
	__u16 port;
	__u16 intPort;
	__u16 proto;
};

#ifdef __KERNEL__

// comment this line out if you don't want to track any debug information
//#define IVI_DEBUG

// comment this line out if you don't want to track any debug information of tcp connection state
//#define IVI_DEBUG_TCP

// comment this line out if you don't want to track any debug information of rule mapping
//#define IVI_DEBUG_RULE

// comment this line out if you don't want to track any debug information of port mapping
//#define IVI_DEBUG_MAP
//#define IVI_DEBUG_MAP_TCP

enum {
	IVI_MODE_HGW = 0,		// Home gateway
	IVI_MODE_HGW_NAT44,	    // Home gateway with NAT44
};

#define IVI_HTABLE_SIZE		32
#define IVI_GOLDEN_RATIO_16		0x9e37
#define IVI_GOLDEN_RATIO_32		0x9e370001

// Generic hash function for a 16 bit value, see 'Introduction to Algorithms, 2nd Edition' Section 11.3.2
static inline int port_hashfn(__be16 port)
{
	unsigned int m = port * IVI_GOLDEN_RATIO_16;
	return ((m & 0xf800) >> 11);  // extract highest 6 bits as hash result
}

// Generic hash function for a 32 bit value, see 'Introduction to Algorithms, 2nd Edition' Section 11.3.2
static inline int v4addr_port_hashfn(__be32 addr, __be16 port)
{
	__be32 m = addr + port;
	m *= IVI_GOLDEN_RATIO_32;
	return ((m & 0xf8000000) >> 27);
}

#endif /* __KERNEL__ */

#endif /* IVI_CONFIG_H */
