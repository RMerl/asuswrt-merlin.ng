/*
 * <:copyright-BRCM:2020:DUAL/GPL:standard 
 * 
 *    Copyright (c) 2020 Broadcom 
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
#ifndef __NDI_LOCAL_H
#define __NDI_LOCAL_H

#include <linux/types.h>
#include <linux/ip.h>
#include <linux/ipv6.h>
#include <linux/in6.h>
#include <linux/if_ether.h>
#include <net/ipv6.h>
#include <net/xfrm.h>
#include <linux/ndi.h>

#undef pr_fmt
#define pr_fmt(fmt)		"NDI " fmt

struct sk_buff;
struct nf_conn;
struct ndi_dev;

enum {
	IGN_NONE = 0,
	IGN_NO_DEV = 1,
	IGN_INVAL_L3_PROTO = 2,
	IGN_NO_MAC = 3,
	IGN_IP4_MULTICAST = 4,
	IGN_IP6_MULTICAST = 5,
};

struct ndi_classify {
	struct sk_buff		*skb;
	struct nf_conn		*ct;
	struct ndi_dev		*dev;
	unsigned int		hook;
	unsigned long		flags;
	unsigned int		ignore_reason;
};

/* flags */
enum {
	UPDATED_BIT,
	DEV_CLASSIFY_BIT,
};

/* enum for device heartbeat state */
enum {
	DEV_ONLINE,
	DEV_MONITOR,
	DEV_PROBING,
	DEV_INACTIVE,
	DEV_OFFLINE,
};

struct ndi_ip {
	struct in_addr		ip4;
	struct in6_addr		ip6;
	u8			l3proto;
	u8			mac[ETH_ALEN];
	struct hlist_node	node;
};

struct ndi_sa {
	struct xfrm_state	*x;
	struct ndi_dev		*dev;
	struct hlist_node	node;
};

struct nl_bcast_entry {
	struct sock		*socket;
	struct net		*net;
	struct list_head	node;
};

/* core */
extern spinlock_t lock;
extern struct list_head nl_bcast_list;
extern struct proc_dir_entry *ndi_dir;
void probe_set_online(struct ndi_classify *p);

/* dhcp */
void ndi_parse_dhcp(struct ndi_classify *p);

/* arp */
void ndi_parse_arp(struct ndi_classify *p);

/* tables */
struct ndi_dev *dev_find_by_mac(u8 *mac);
struct ndi_dev *dev_find_by_id(u32 id);
struct ndi_dev *dev_find_or_new(u8 *mac, struct sk_buff *skb);
struct ndi_dev *dev_find_or_new_ignored(u8 *mac, struct sk_buff *skb);
struct ndi_dev *dev_find_or_new_for_secpath(struct sec_path *sp,
					    struct ndi_dev *ct_dev);
int dev_ip_update(struct ndi_dev *dev, struct sk_buff *skb);
int devs_bucket_count(void);
void dev_sa_get(struct ndi_dev *dev);
void dev_sa_put(struct ndi_dev *dev);
struct hlist_head *devs_get_bucket(int i);
struct ndi_ip *ip_find(void *ip, int l3proto);
struct ndi_ip *ip_find_by_skb(struct sk_buff *skb);
void ip_free(struct ndi_ip *entry);
struct ndi_ip *ip_find_or_new(void *ip, int l3proto, u8 *mac);
struct ndi_sa *sa_find(struct xfrm_state *x);
struct ndi_sa *sa_find_first(struct sec_path *sp);
struct ndi_sa *sa_find_or_new(struct xfrm_state *x);
void sa_delete(struct ndi_sa *sa);
int __init ndi_tables_init(void);
void __exit ndi_tables_exit(void);

/* netlink */
void ndi_nl_rcv(struct sk_buff *skb);
int ndi_dev_nl_event_locked(struct ndi_dev *dev, int type);
int ndi_dev_nl_event(struct ndi_dev *dev, int type);
int ndi_dev_nl_init(void);
void ndi_dev_nl_exit(void);

/* helpers */
#define ndi_dev_name(__dev) \
	({ \
		int __ip4, __ip6; \
		char __name[256]; \
	\
		__ip4 = !ipv4_ignore(__dev->ip4.s_addr); \
		__ip6 = !ipv6_ignore(&__dev->ip6); \
		if (__ip4 && __ip6) \
			sprintf(__name, "[%pM (%pI4) (%pI6c)]", __dev->mac, \
				&__dev->ip4, &__dev->ip6); \
		else if (__ip6) \
			sprintf(__name, "[%pM (%pI6c)]", __dev->mac, \
				&__dev->ip6); \
		else if (__ip4) \
			sprintf(__name, "[%pM (%pI4)]", __dev->mac, \
				&__dev->ip4); \
		else \
			sprintf(__name, "[%pM]", __dev->mac); \
	\
		__name; \
	})

#define ndi_devs_for_each_entry(dev, bkt) \
	for (bkt = 0; bkt < devs_bucket_count(); bkt++) \
		hlist_for_each_entry(dev, devs_get_bucket(bkt), node)

#define ndi_devs_for_each_entry_safe(dev, tmp, bkt) \
	for (bkt = 0; bkt < devs_bucket_count(); bkt++) \
		hlist_for_each_entry_safe(dev, tmp, devs_get_bucket(bkt), node)

static inline int should_ignore_ndi_dev(struct ndi_dev *dev)
{
	return test_bit(NDI_DEV_IGNORE_BIT, &dev->flags) ||
	       test_bit(NDI_DEV_STALE_BIT, &dev->flags);
}

static inline int
cf_l3(struct sk_buff *skb, int l3proto, int l4proto, int srcport, int dstport)
{
	int l4src = 0, l4dst = 0;

	if (skb->protocol != htons(l3proto))
		return 0;
	if (l3proto == ETH_P_IP && ip_hdr(skb)->protocol != l4proto)
		return 0;
	if (l3proto == ETH_P_IPV6 && ipv6_hdr(skb)->nexthdr != l4proto)
		return 0 ;
	if (l4proto == IPPROTO_UDP) {
		l4src = udp_hdr(skb)->source;
		l4dst = udp_hdr(skb)->dest;
	} else if (l4proto == IPPROTO_TCP) {
		l4src = tcp_hdr(skb)->source;
		l4dst = tcp_hdr(skb)->dest;
	}
	if ((srcport && l4src != htons(srcport)) ||
	    (dstport && l4dst != htons(dstport)))
		return 0;

	return 1;
}
static inline int
cf_l3v4(struct sk_buff *skb, int l4proto, int srcport, int dstport)
{
	return cf_l3(skb, ETH_P_IP, l4proto, srcport, dstport);
}
static inline int
cf_l3v6(struct sk_buff *skb, int l4proto, int srcport, int dstport)
{
	return cf_l3(skb, ETH_P_IPV6, l4proto, srcport, dstport);
}
static inline int is_dhcp(struct sk_buff *skb)
{
	return cf_l3v4(skb, IPPROTO_UDP, 68, 67) ||
	       cf_l3v4(skb, IPPROTO_UDP, 67, 68);
}
static inline int is_dhcp6(struct sk_buff *skb)
{
	return cf_l3v6(skb, IPPROTO_UDP, 546, 547) ||
	       cf_l3v6(skb, IPPROTO_UDP, 547, 546);
}
static inline int is_arp(struct sk_buff *skb)
{
	return skb->protocol == htons(ETH_P_ARP);
}
static inline struct in_addr *ndi_ipv4_get_addr(struct sk_buff *skb)
{
	if (is_netdev_wan(skb->dev))
		return (void *)&ip_hdr(skb)->daddr;
	else
		return (void *)&ip_hdr(skb)->saddr;
}
static inline struct in6_addr *ndi_ipv6_get_addr(struct sk_buff *skb)
{
	if (is_netdev_wan(skb->dev))
		return &ipv6_hdr(skb)->daddr;
	else
		return &ipv6_hdr(skb)->saddr;
}
static inline int ipv4_addr_equal(struct in_addr *a, struct in_addr *b)
{
	return a->s_addr == b->s_addr;
}
static inline int ipv4_ignore(u32 ip)
{
	return ipv4_is_loopback(ip) ||
	       ipv4_is_multicast(ip) ||
	       ipv4_is_lbcast(ip) ||
	       ipv4_is_zeronet(ip);
}
static inline int ipv6_ignore(struct in6_addr *ip)
{
	return ipv6_addr_any(ip) ||
	       ipv6_addr_loopback(ip) ||
	       ipv6_addr_is_multicast(ip);
}

#endif /* __NDI_LOCAL_H */
