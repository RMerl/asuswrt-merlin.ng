#if defined(CONFIG_BCM_KF_NETFILTER)
/*
*    Copyright (c) 2003-2019 Broadcom
*    All Rights Reserved
*
<:label-BRCM:2019:DUAL/GPL:standard

Unless you and Broadcom execute a separate written software license
agreement governing use of this software, this software is licensed
to you under the terms of the GNU General Public License version 2
(the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
with the following added to such license:

   As a special exception, the copyright holders of this software give
   you permission to link this software with independent modules, and
   to copy and distribute the resulting executable under terms of your
   choice, provided that you also meet, for each linked independent
   module, the terms and conditions of the license of that module.
   An independent module is a module which is not derived from this
   software.  The special exception does not apply to any modifications
   of the software.

Not withstanding the above, under no circumstances may you combine
this software in any way with any other Broadcom software provided
under a license other than the GPL, without Broadcom's express prior
written consent.

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