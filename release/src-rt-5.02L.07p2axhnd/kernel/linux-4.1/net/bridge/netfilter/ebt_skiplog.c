#if defined(CONFIG_BCM_KF_NETFILTER)
/*
*    Copyright (c) 2003-2012 Broadcom Corporation
*    All Rights Reserved
*
<:label-BRCM:2012:DUAL/GPL:standard

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

/*
 *  ebt_skiplog
 */
#include <linux/module.h>
#include <linux/netfilter/x_tables.h>
#include <linux/netfilter_bridge/ebtables.h>
#if defined(CONFIG_BCM_KF_BLOG) && defined(CONFIG_BLOG)
#include <linux/blog.h>
#endif

static unsigned int
ebt_skiplog_tg(struct sk_buff *skb, const struct xt_action_param *par)
{
#if defined(CONFIG_BCM_KF_BLOG) && defined(CONFIG_BLOG)
	blog_skip(skb, blog_skip_reason_nf_ebt_skiplog);
#endif

	return EBT_CONTINUE;
}

static struct xt_target ebt_skiplog_tg_reg __read_mostly = {
	.name		= "SKIPLOG",
	.revision	= 0,
	.family		= NFPROTO_BRIDGE,
	.target		= ebt_skiplog_tg,
	.me		= THIS_MODULE,
};

static int __init ebt_skiplog_init(void)
{
	return xt_register_target(&ebt_skiplog_tg_reg);
}

static void __exit ebt_skiplog_fini(void)
{
	xt_unregister_target(&ebt_skiplog_tg_reg);
}

module_init(ebt_skiplog_init);
module_exit(ebt_skiplog_fini);
MODULE_DESCRIPTION("Ebtables: SKIPLOG target");
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Broadcom");
#endif
