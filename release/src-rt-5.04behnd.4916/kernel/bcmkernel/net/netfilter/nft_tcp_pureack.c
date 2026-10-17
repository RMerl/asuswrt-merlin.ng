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
#include <linux/ip.h>
#include <linux/ipv6.h>
#include <linux/tcp.h>

#if defined(CONFIG_BLOG)
#include <linux/blog.h>
#endif

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Broadcom");
MODULE_DESCRIPTION("nft blog tcp pureack match");
MODULE_ALIAS("nft_tcp_pureack");

#if 0
#define DEBUG_TCP_PUREACK(fmt, ...) printk("@@[%s.%d] "fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#else
#define DEBUG_TCP_PUREACK(fmt, ...) {}
#endif


struct nft_tcp_pureack {
};

static void nft_tcp_pureack_eval(const struct nft_expr *expr, struct nft_regs *regs, const struct nft_pktinfo *pkt) {
	struct sk_buff *skb = pkt->skb;
	const struct iphdr *iph;
	const struct ipv6hdr *ip6h;
	const struct tcphdr *th;
	unsigned int payload_len = 0xFFFFFFFF;	

#if defined(CONFIG_BLOG)
	if (skb->blog_p && skb->blog_p->key.tcp_pure_ack) {
		DEBUG_TCP_PUREACK("Pure ACK detected by BLOG.\n");
		regs->verdict.code = NFT_CONTINUE;
		return;
	}
#endif

	if ((nft_pf(pkt) != NFPROTO_IPV4) && (nft_pf(pkt) != NFPROTO_IPV6)) {
		/* NFPROTO_BRIDGE parsing is not supported yet. */
		DEBUG_TCP_PUREACK("Protocol Family [%d] is not supported.\n", nft_pf(pkt));
		regs->verdict.code = NFT_BREAK;
		return;
	}

	if (pkt->tprot != IPPROTO_TCP) {
		DEBUG_TCP_PUREACK("Non-TCP packet.\n");
		regs->verdict.code = NFT_BREAK;
		return;
	}

	th = (struct tcphdr *)skb_transport_header(skb);
	if (!th || !th->ack) {
		DEBUG_TCP_PUREACK("Invalid or non-ACK TCP header.\n");
		regs->verdict.code = NFT_BREAK;
		return;
	}

	if (nft_pf(pkt) == NFPROTO_IPV4) {
		iph = (struct iphdr *)skb_network_header(skb);
		if (!iph || iph->version != 4) {
			DEBUG_TCP_PUREACK("Invalid IPv4 header.\n");
			regs->verdict.code = NFT_BREAK;
			return;
		}
		payload_len = ntohs(iph->tot_len) - (iph->ihl << 2) - (th->doff << 2);
	} else { /* NFPROTO_IPV6 */
		ip6h = (struct ipv6hdr *)skb_network_header(skb);
		if (!ip6h || ip6h->version != 6) {
			DEBUG_TCP_PUREACK("Invalid IPv6 header.\n");
			regs->verdict.code = NFT_BREAK;
			return;
		}
		payload_len = ntohs(ip6h->payload_len) - (th->doff << 2);
	}

	if (payload_len == 0) {
		DEBUG_TCP_PUREACK("Pure ACK.\n");
		regs->verdict.code = NFT_CONTINUE;
	} else {
		DEBUG_TCP_PUREACK("ACK with payload.\n");
		regs->verdict.code = NFT_BREAK;
	}

}

static int nft_tcp_pureack_init(const struct nft_ctx *ctx, const struct nft_expr *expr, const struct nlattr * const tb[]) {
	//printk("--- tcp_pureack_init : \n");
	return 0;
}

static int nft_tcp_pureack_dump(struct sk_buff *skb, const struct nft_expr *expr) {
	//printk("--- tcp_pureack_dump : continue\n");
	return 0;
}

static struct nft_expr_type nft_tcp_pureack_type;
static const struct nft_expr_ops nft_tcp_pureack_op = {
	.eval = nft_tcp_pureack_eval,
	.size = NFT_EXPR_SIZE(sizeof(struct nft_tcp_pureack)),
	.init = nft_tcp_pureack_init,
	.dump = nft_tcp_pureack_dump,
	.type = &nft_tcp_pureack_type,
};
static struct nft_expr_type nft_tcp_pureack_type __read_mostly =  {
	.ops = &nft_tcp_pureack_op,
	.name = "tcp_pureack",
	.owner = THIS_MODULE,
};

static int __init nft_tcp_pureack_module_init(void)
{
	return nft_register_expr(&nft_tcp_pureack_type);
}

static void __exit nft_tcp_pureack_module_exit(void)
{
	nft_unregister_expr(&nft_tcp_pureack_type);
}

module_init(nft_tcp_pureack_module_init);
module_exit(nft_tcp_pureack_module_exit);

