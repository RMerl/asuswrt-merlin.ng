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
