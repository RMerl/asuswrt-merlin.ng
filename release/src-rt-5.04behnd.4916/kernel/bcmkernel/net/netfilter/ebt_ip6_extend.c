/*
 *  ebt_ip6
 *
 *	Authors:
 *	Manohar Castelino <manohar.r.castelino@intel.com>
 *	Kuo-Lang Tseng <kuo-lang.tseng@intel.com>
 *	Jan Engelhardt <jengelh@medozas.de>
 *
 * Summary:
 * This is just a modification of the IPv4 code written by
 * Bart De Schuymer <bdschuym@pandora.be>
 * with the changes required to support IPv6
 *
 *  Jan, 2008
 *
 *  Extend by Broadcom at Jan 31, 2019
 */
#include <linux/ipv6.h>
#include <net/ipv6.h>
#include <linux/in.h>
#include <linux/module.h>
#include <net/dsfield.h>
#include <linux/netfilter/x_tables.h>
#include <linux/netfilter_bridge/ebtables.h>
#include "linux/netfilter_bridge/ebt_ip6_extend.h"

static bool
ip6_less_than(const struct in6_addr *a, const struct in6_addr *b)
{
	unsigned int i;

	for (i = 0; i < 4; ++i) {
		if (a->s6_addr32[i] != b->s6_addr32[i])
			return ntohl(a->s6_addr32[i]) < ntohl(b->s6_addr32[i]);
	}

	return 0;
}

static bool
ebt_ip6_mt(const struct sk_buff *skb, struct xt_action_param *par)
{
	const struct ebt_ip6_extend_info *info = par->matchinfo;
	const struct ipv6hdr *ih6;
	struct ipv6hdr _ip6h;

	ih6 = skb_header_pointer(skb, 0, sizeof(_ip6h), &_ip6h);
	if (ih6 == NULL)
		return false;
	if (info->bitmask & EBT_IP6_TCLASS_EXTEND) {
		u8 tclass = ipv6_get_dsfield(ih6);
		if (NF_INVF(info, EBT_IP6_TCLASS_EXTEND, 
			((tclass & info->tclassmsk) < (info->tclass[0] & info->tclassmsk))
			|| ((tclass & info->tclassmsk) > (info->tclass[1] & info->tclassmsk))))
			return false;
    }
	if (info->bitmask & EBT_IP6_FLOWLABEL_EXTEND) {
		if (NF_INVF(info, EBT_IP6_FLOWLABEL_EXTEND,
			(ih6->flow_lbl[0] & 0xF) != (info->flow_lbl[0] & 0xF)
			|| ih6->flow_lbl[1] != info->flow_lbl[1]
			|| ih6->flow_lbl[2] != info->flow_lbl[2]))
			return false;
	}

	if (info->bitmask & EBT_IP6_RANGE_SRC &&
		NF_INVF(info, EBT_IP6_RANGE_SRC,
			ip6_less_than(&ih6->saddr, &info->range_src.ip_min) || 
			ip6_less_than(&info->range_src.ip_max, &ih6->saddr)))
		return false;

	if (info->bitmask & EBT_IP6_RANGE_DST &&
		NF_INVF(info, EBT_IP6_RANGE_DST,
			ip6_less_than(&ih6->daddr, &info->range_dst.ip_min) || 
			ip6_less_than(&info->range_dst.ip_max, &ih6->daddr)))
		return false;

	return true;
}

static int ebt_ip6_mt_check(const struct xt_mtchk_param *par)
{
	const struct ebt_entry *e = par->entryinfo;
	struct ebt_ip6_extend_info *info = par->matchinfo;

	if (e->ethproto != htons(ETH_P_IPV6))
		return -EINVAL;
	if (info->bitmask & ~EBT_IP6_MASK_EXTEND || info->invflags & ~EBT_IP6_MASK_EXTEND)
		return -EINVAL;
	return 0;
}

static struct xt_match ebt_ip6_mt_reg __read_mostly = {
	.name		= EBT_IP6_MATCH_EXTEND,
	.revision	= 0,
	.family		= NFPROTO_BRIDGE,
	.match		= ebt_ip6_mt,
	.checkentry	= ebt_ip6_mt_check,
	.matchsize	= sizeof(struct ebt_ip6_extend_info),
	.me		= THIS_MODULE,
};

static int __init ebt_ip6_init(void)
{
	return xt_register_match(&ebt_ip6_mt_reg);
}

static void __exit ebt_ip6_fini(void)
{
	xt_unregister_match(&ebt_ip6_mt_reg);
}

module_init(ebt_ip6_init);
module_exit(ebt_ip6_fini);
MODULE_DESCRIPTION("Ebtables: IPv6 protocol packet match extend");
MODULE_AUTHOR("Kuo-Lang Tseng <kuo-lang.tseng@intel.com>, Broadcom");
MODULE_LICENSE("GPL");
