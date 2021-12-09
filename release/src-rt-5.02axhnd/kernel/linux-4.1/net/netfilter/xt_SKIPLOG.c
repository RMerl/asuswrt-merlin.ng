#if defined(CONFIG_BCM_KF_NETFILTER)
/*
*    Copyright (c) 2003-2012 Broadcom Corporation
*    All Rights Reserved
*
<:label-BRCM:2012:DUAL/GPL:standard

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
#include <linux/netfilter/x_tables.h>
#if defined(CONFIG_BCM_KF_BLOG) && defined(CONFIG_BLOG)
#include <linux/blog.h>
#endif

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Broadcom");
MODULE_DESCRIPTION("iptables stop logging module");
MODULE_ALIAS("ipt_SKIPLOG");

static unsigned int
skiplog_tg(struct sk_buff *skb, const struct xt_action_param *par)
{
#if defined(CONFIG_BLOG_FEATURE)
	skb->ipt_check |= IPT_TARGET_SKIPLOG;
	if ( skb->ipt_check & IPT_TARGET_CHECK )
		return XT_CONTINUE;
#endif

#if defined(CONFIG_BCM_KF_BLOG) && defined(CONFIG_BLOG)
	blog_skip(skb, blog_skip_reason_nf_xt_skiplog);
#endif

	return XT_CONTINUE;
}

static struct xt_target skiplog_tg_reg __read_mostly = {
	.name		= "SKIPLOG",
	.revision   = 0,
	.family		= NFPROTO_UNSPEC,
	.target		= skiplog_tg,
	.me		= THIS_MODULE,
};

static int __init skiplog_tg_init(void)
{
	return xt_register_target(&skiplog_tg_reg);
}

static void __exit skiplog_tg_exit(void)
{
	xt_unregister_target(&skiplog_tg_reg);
}

module_init(skiplog_tg_init);
module_exit(skiplog_tg_exit);
#endif
