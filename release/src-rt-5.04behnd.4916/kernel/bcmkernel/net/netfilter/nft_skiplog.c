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
#include <net/netfilter/nf_tables.h>
#include <linux/tcp.h>

#if defined(CONFIG_BLOG)
#include <linux/blog.h>
#endif

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Broadcom");
MODULE_DESCRIPTION("nft stop logging module");
MODULE_ALIAS("nft_skiplog");

struct nft_skiplog {
};

static void nft_skiplog_eval(const struct nft_expr *expr, struct nft_regs *regs, const struct nft_pktinfo *pkt) {
#if defined(CONFIG_BLOG)
	struct sk_buff *skb = pkt->skb;

	blog_skip(skb, blog_skip_reason_nf_xt_skiplog);
#endif
	//printk("--- nft_skiplog_eval : \n");
	regs->verdict.code = NFT_CONTINUE;
}

static int nft_skiplog_init(const struct nft_ctx *ctx, const struct nft_expr *expr, const struct nlattr * const tb[]) {
	//printk("--- nft_skiplog_init : \n");
	return 0;
}

static int nft_skiplog_dump(struct sk_buff *skb, const struct nft_expr *expr) {
	//printk("--- nft_skiplog_dump : \n");
	return 0;
}

static struct nft_expr_type nft_skiplog_type;
static const struct nft_expr_ops nft_skiplog_op = {
	.eval = nft_skiplog_eval,
	.size = NFT_EXPR_SIZE(sizeof(struct nft_skiplog)),
	.init = nft_skiplog_init,
	.dump = nft_skiplog_dump,
	.type = &nft_skiplog_type,
};
static struct nft_expr_type nft_skiplog_type __read_mostly =  {
	.ops = &nft_skiplog_op,
	.name = "skiplog",
	.owner = THIS_MODULE,
};

static int __init nft_skiplog_module_init(void)
{       
	return nft_register_expr(&nft_skiplog_type);
}

static void __exit nft_skiplog_module_exit(void)
{
	nft_unregister_expr(&nft_skiplog_type);
}

module_init(nft_skiplog_module_init);
module_exit(nft_skiplog_module_exit);

