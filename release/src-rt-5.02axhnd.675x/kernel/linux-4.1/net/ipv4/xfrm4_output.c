/*
 * xfrm4_output.c - Common IPsec encapsulation code for IPv4.
 * Copyright (c) 2004 Herbert Xu <herbert@gondor.apana.org.au>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version
 * 2 of the License, or (at your option) any later version.
 */

#include <linux/if_ether.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/netfilter_ipv4.h>
#include <net/dst.h>
#include <net/ip.h>
#include <net/xfrm.h>
#include <net/icmp.h>

static int xfrm4_tunnel_check_size(struct sk_buff *skb)
{
	int mtu, ret = 0;

	if (IPCB(skb)->flags & IPSKB_XFRM_TUNNEL_SIZE)
		goto out;

	if (!(ip_hdr(skb)->frag_off & htons(IP_DF)) || skb->ignore_df)
		goto out;

	mtu = dst_mtu(skb_dst(skb));
	if (skb->len > mtu) {
		if (skb->sk)
			xfrm_local_error(skb, mtu);
		else
			icmp_send(skb, ICMP_DEST_UNREACH,
				  ICMP_FRAG_NEEDED, htonl(mtu));
		ret = -EMSGSIZE;
	}
out:
	return ret;
}

int xfrm4_extract_output(struct xfrm_state *x, struct sk_buff *skb)
{
	int err;

	err = xfrm4_tunnel_check_size(skb);
	if (err)
		return err;

	XFRM_MODE_SKB_CB(skb)->protocol = ip_hdr(skb)->protocol;

	return xfrm4_extract_header(skb);
}

int xfrm4_prepare_output(struct xfrm_state *x, struct sk_buff *skb)
{
	int err;

	err = xfrm_inner_extract_output(x, skb);
	if (err)
		return err;

	IPCB(skb)->flags |= IPSKB_XFRM_TUNNEL_SIZE;
	skb->protocol = htons(ETH_P_IP);

	return x->outer_mode->output2(x, skb);
}
EXPORT_SYMBOL(xfrm4_prepare_output);

int xfrm4_output_finish(struct sock *sk, struct sk_buff *skb)
{
	memset(IPCB(skb), 0, sizeof(*IPCB(skb)));

#ifdef CONFIG_NETFILTER
	IPCB(skb)->flags |= IPSKB_XFRM_TRANSFORMED;
#endif

	return xfrm_output(sk, skb);
}

static int __xfrm4_output(struct sock *sk, struct sk_buff *skb)
{
	struct xfrm_state *x = skb_dst(skb)->xfrm;

#ifdef CONFIG_NETFILTER
	if (!x) {
		IPCB(skb)->flags |= IPSKB_REROUTED;
		return dst_output_sk(sk, skb);
	}
#endif

	return x->outer_mode->afinfo->output_finish(sk, skb);
}

int xfrm4_output(struct sock *sk, struct sk_buff *skb)
{
	return NF_HOOK_COND(NFPROTO_IPV4, NF_INET_POST_ROUTING, sk, skb,
			    NULL, skb_dst(skb)->dev, __xfrm4_output,
			    !(IPCB(skb)->flags & IPSKB_REROUTED));
}

void xfrm4_local_error(struct sk_buff *skb, u32 mtu)
{
	struct iphdr *hdr;

	hdr = skb->encapsulation ? inner_ip_hdr(skb) : ip_hdr(skb);
	ip_local_error(skb->sk, EMSGSIZE, hdr->daddr,
		       inet_sk(skb->sk)->inet_dport, mtu);
}
