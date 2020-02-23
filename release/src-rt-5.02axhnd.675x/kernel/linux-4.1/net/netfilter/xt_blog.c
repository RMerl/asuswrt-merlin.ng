#if defined(CONFIG_BCM_KF_NETFILTER) && defined(CONFIG_BCM_KF_BLOG)
/*
*    Copyright (c) 2003-2018 Broadcom Corporation
*    All Rights Reserved
*
<:label-BRCM:2018:DUAL/GPL:standard

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

#include <linux/module.h>
#include <linux/skbuff.h>

#include <linux/netfilter/xt_blog.h>
#include <linux/netfilter/x_tables.h>

#include <linux/blog.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Broadcom");
MODULE_DESCRIPTION("iptables blog match");
MODULE_ALIAS("ipt_blog");
MODULE_ALIAS("ip6t_blog");

static bool
blog_mt(const struct sk_buff *skb, struct xt_action_param *par)
{
	const struct xt_blog *info = par->matchinfo;

	if (skb->blog_p) {
		if (info->tcp_pure_ack)
		{
			return (skb->blog_p->ack_done || skb->blog_p->key.tcp_pure_ack) ^
				info->invert;
		}
	}

	return false;
}

static struct xt_match blog_mt_reg __read_mostly = {
	.name		= "blog",
	.revision   = 0,
	.family	= NFPROTO_UNSPEC,
	.match		= blog_mt,
	.matchsize	= sizeof(struct xt_blog),
	.me		= THIS_MODULE,
};

static int __init blog_mt_init(void)
{
	return xt_register_match(&blog_mt_reg);
}

static void __exit blog_mt_exit(void)
{
	xt_unregister_match(&blog_mt_reg);
}

module_init(blog_mt_init);
module_exit(blog_mt_exit);
#endif
