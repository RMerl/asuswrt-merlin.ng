/*
 *  ebt_ip
 *
 *	Authors:
 *	Bart De Schuymer <bdschuym@pandora.be>
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
#include <linux/ip.h>
#include <net/ip.h>
#include <linux/in.h>
#include <linux/module.h>
#include <linux/netfilter/x_tables.h>
#include <linux/netfilter_bridge/ebtables.h>
#include <linux/netfilter_bridge/ebt_ip_extend.h>

static bool
ebt_ip_mt(const struct sk_buff *skb, struct xt_action_param *par)
{
	const struct ebt_ip_extend_info *info = par->matchinfo;
	const struct iphdr *ih;
	struct iphdr _iph;

	ih = skb_header_pointer(skb, 0, sizeof(_iph), &_iph);
	if (ih == NULL)
		return false;
	if (info->bitmask & EBT_IP_TOS_EXTEND&&
	   FWINV(((ih->tos & info->tosmask) < (info->tos[0] & info->tosmask)) || 
	   ((ih->tos & info->tosmask) > (info->tos[1] & info->tosmask)), EBT_IP_TOS_EXTEND))
		return false;
	return true;
}

static int ebt_ip_mt_check(const struct xt_mtchk_param *par)
{
	const struct ebt_ip_extend_info *info = par->matchinfo;
	const struct ebt_entry *e = par->entryinfo;

	if (e->ethproto != htons(ETH_P_IP))
		return -EINVAL;
	if (info->bitmask & ~EBT_IP_MASK_EXTEND|| info->invflags & ~EBT_IP_MASK_EXTEND)
		return -EINVAL;
	return 0;
}

static struct xt_match ebt_ip_mt_reg __read_mostly = {
	.name		= EBT_IP_MATCH_EXTEND,
	.revision	= 0,
	.family		= NFPROTO_BRIDGE,
	.match		= ebt_ip_mt,
	.checkentry	= ebt_ip_mt_check,
	.matchsize	= sizeof(struct ebt_ip_extend_info),
	.me		= THIS_MODULE,
};

static int __init ebt_ip_init(void)
{
	return xt_register_match(&ebt_ip_mt_reg);
}

static void __exit ebt_ip_fini(void)
{
	xt_unregister_match(&ebt_ip_mt_reg);
}

module_init(ebt_ip_init);
module_exit(ebt_ip_fini);
MODULE_DESCRIPTION("Ebtables: IPv4 protocol packet match extend");
MODULE_LICENSE("GPL");
