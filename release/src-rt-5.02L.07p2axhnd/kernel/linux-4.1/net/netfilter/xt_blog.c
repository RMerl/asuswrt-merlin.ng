#if defined(CONFIG_BCM_KF_NETFILTER) && defined(CONFIG_BCM_KF_BLOG)
/*
*    Copyright (c) 2003-2018 Broadcom Corporation
*    All Rights Reserved
*
<:label-BRCM:2018:DUAL/GPL:standard

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
