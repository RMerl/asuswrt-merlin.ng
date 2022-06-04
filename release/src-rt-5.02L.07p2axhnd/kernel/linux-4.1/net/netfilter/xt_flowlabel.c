#if defined(CONFIG_BCM_KF_NETFILTER)
/*
*    Copyright (c) 2003-2019 Broadcom
*    All Rights Reserved
*
<:label-BRCM:2019:DUAL/GPL:standard

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License, version 2, as published by
the Free Software Foundation (the "GPL").

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.


A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.

:>
*/


/* IP tables module for matching the value of the IPv6 flowlabel field
 *
 * BRCM, Feb, 1. 2019.
 */

#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/ipv6.h>
#include <net/ipv6.h>

#include <linux/netfilter/x_tables.h>
#include <linux/netfilter/xt_flowlabel.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Broadcom");
MODULE_DESCRIPTION("Xtables: IPv6 flowlabel field match");
MODULE_ALIAS("ip6t_flowlabel");


static bool
flowlabel_mt(const struct sk_buff *skb, struct xt_action_param *par)
{
	const struct xt_flowlabel_info *info = par->matchinfo;

	return (ip6_flowlabel(ipv6_hdr(skb)) == info->flowlabel) ^ !!info->invert;
}

static int flowlabel_mt_check(const struct xt_mtchk_param *par)
{
	const struct xt_flowlabel_info *info = par->matchinfo;

	if (info->flowlabel > XT_FLOWLABEL_MAX) {
		pr_err("flowlabel 0x%x out of range\n", info->flowlabel);
		return -EDOM;
	}

	return 0;
}


static struct xt_match flowlabel_mt_reg __read_mostly = {
	.name		= "flowlabel",
	.family		= NFPROTO_IPV6,
	.checkentry	= flowlabel_mt_check,
	.match		= flowlabel_mt,
	.matchsize	= sizeof(struct xt_flowlabel_info),
	.me		= THIS_MODULE,
};

static int __init flowlabel_mt_init(void)
{
	return xt_register_match(&flowlabel_mt_reg);
}

static void __exit flowlabel_mt_exit(void)
{
	xt_unregister_match(&flowlabel_mt_reg);
}

module_init(flowlabel_mt_init);
module_exit(flowlabel_mt_exit);

#endif // defined(CONFIG_BCM_KF_NETFILTER)