/*
 *	IPV6 GSO/GRO offload support
 *	Linux INET6 implementation
 *
 *	This program is free software; you can redistribute it and/or
 *      modify it under the terms of the GNU General Public License
 *      as published by the Free Software Foundation; either version
 *      2 of the License, or (at your option) any later version.
 */

#include <linux/kernel.h>
#include <linux/socket.h>
#include <linux/netdevice.h>
#include <linux/skbuff.h>
#include <linux/printk.h>

#include <net/protocol.h>
#include <net/ipv6.h>

#include "ip6_offload.h"

static int ipv6_gso_pull_exthdrs(struct sk_buff *skb, int proto)
{
	const struct net_offload *ops = NULL;

	for (;;) {
		struct ipv6_opt_hdr *opth;
		int len;

		if (proto != NEXTHDR_HOP) {
			ops = rcu_dereference(inet6_offloads[proto]);

			if (unlikely(!ops))
				break;

			if (!(ops->flags & INET6_PROTO_GSO_EXTHDR))
				break;
		}

		if (unlikely(!pskb_may_pull(skb, 8)))
			break;

		opth = (void *)skb->data;
		len = ipv6_optlen(opth);

		if (unlikely(!pskb_may_pull(skb, len)))
			break;

		opth = (void *)skb->data;
		proto = opth->nexthdr;
		__skb_pull(skb, len);
	}

	return proto;
}

static struct sk_buff *ipv6_gso_segment(struct sk_buff *skb,
	netdev_features_t features)
{
	struct sk_buff *segs = ERR_PTR(-EINVAL);
	struct ipv6hdr *ipv6h;
	const struct net_offload *ops;
	int proto;
	struct frag_hdr *fptr;
	u8 *prevhdr;
	int offset = 0;
	bool encap, udpfrag;
	int nhoff;

	if (unlikely(skb_shinfo(skb)->gso_type &
		     ~(SKB_GSO_TCPV4 |
		       SKB_GSO_UDP |
		       SKB_GSO_DODGY |
		       SKB_GSO_TCP_ECN |
		       SKB_GSO_GRE |
		       SKB_GSO_GRE_CSUM |
		       SKB_GSO_IPIP |
		       SKB_GSO_SIT |
		       SKB_GSO_UDP_TUNNEL |
		       SKB_GSO_UDP_TUNNEL_CSUM |
		       SKB_GSO_TUNNEL_REMCSUM |
		       SKB_GSO_TCPV6 |
		       0)))
		goto out;

	skb_reset_network_header(skb);
	nhoff = skb_network_header(skb) - skb_mac_header(skb);
	if (unlikely(!pskb_may_pull(skb, sizeof(*ipv6h))))
		goto out;

	encap = SKB_GSO_CB(skb)->encap_level > 0;
	if (encap)
		features &= skb->dev->hw_enc_features;
	SKB_GSO_CB(skb)->encap_level += sizeof(*ipv6h);

	ipv6h = ipv6_hdr(skb);
	__skb_pull(skb, sizeof(*ipv6h));
	segs = ERR_PTR(-EPROTONOSUPPORT);

	proto = ipv6_gso_pull_exthdrs(skb, ipv6h->nexthdr);

	if (skb->encapsulation &&
	    skb_shinfo(skb)->gso_type & (SKB_GSO_SIT|SKB_GSO_IPIP))
		udpfrag = proto == IPPROTO_UDP && encap;
	else
		udpfrag = proto == IPPROTO_UDP && !skb->encapsulation;

	ops = rcu_dereference(inet6_offloads[proto]);
	if (likely(ops && ops->callbacks.gso_segment)) {
		skb_reset_transport_header(skb);
		segs = ops->callbacks.gso_segment(skb, features);
	}

	if (IS_ERR(segs))
		goto out;

	for (skb = segs; skb; skb = skb->next) {
		ipv6h = (struct ipv6hdr *)(skb_mac_header(skb) + nhoff);
		ipv6h->payload_len = htons(skb->len - nhoff - sizeof(*ipv6h));
		skb->network_header = (u8 *)ipv6h - skb->head;

		if (udpfrag) {
			int err = ip6_find_1stfragopt(skb, &prevhdr);
			if (err < 0) {
				kfree_skb_list(segs);
				return ERR_PTR(err);
			}
			fptr = (struct frag_hdr *)((u8 *)ipv6h + err);
			fptr->frag_off = htons(offset);
			if (skb->next)
				fptr->frag_off |= htons(IP6_MF);
			offset += (ntohs(ipv6h->payload_len) -
				   sizeof(struct frag_hdr));
		}
		if (encap)
			skb_reset_inner_headers(skb);
	}

out:
	return segs;
}

/* Return the total length of all the extension hdrs, following the same
 * logic in ipv6_gso_pull_exthdrs() when parsing ext-hdrs.
 */
static int ipv6_exthdrs_len(struct ipv6hdr *iph,
			    const struct net_offload **opps)
{
	struct ipv6_opt_hdr *opth = (void *)iph;
	int len = 0, proto, optlen = sizeof(*iph);

	proto = iph->nexthdr;
	for (;;) {
		if (proto != NEXTHDR_HOP) {
			*opps = rcu_dereference(inet6_offloads[proto]);
			if (unlikely(!(*opps)))
				break;
			if (!((*opps)->flags & INET6_PROTO_GSO_EXTHDR))
				break;
		}
		opth = (void *)opth + optlen;
		optlen = ipv6_optlen(opth);
		len += optlen;
		proto = opth->nexthdr;
	}
	return len;
}

static struct sk_buff **ipv6_gro_receive(struct sk_buff **head,
					 struct sk_buff *skb)
{
	const struct net_offload *ops;
	struct sk_buff **pp = NULL;
	struct sk_buff *p;
	struct ipv6hdr *iph;
	unsigned int nlen;
	unsigned int hlen;
	unsigned int off;
	u16 flush = 1;
	int proto;

	off = skb_gro_offset(skb);
	hlen = off + sizeof(*iph);
	iph = skb_gro_header_fast(skb, off);
	if (skb_gro_header_hard(skb, hlen)) {
		iph = skb_gro_header_slow(skb, hlen, off);
		if (unlikely(!iph))
			goto out;
	}

	skb_set_network_header(skb, off);
	skb_gro_pull(skb, sizeof(*iph));
	skb_set_transport_header(skb, skb_gro_offset(skb));

	flush += ntohs(iph->payload_len) != skb_gro_len(skb);

	rcu_read_lock();
	proto = iph->nexthdr;
	ops = rcu_dereference(inet6_offloads[proto]);
	if (!ops || !ops->callbacks.gro_receive) {
		__pskb_pull(skb, skb_gro_offset(skb));
		proto = ipv6_gso_pull_exthdrs(skb, proto);
		skb_gro_pull(skb, -skb_transport_offset(skb));
		skb_reset_transport_header(skb);
		__skb_push(skb, skb_gro_offset(skb));

		ops = rcu_dereference(inet6_offloads[proto]);
		if (!ops || !ops->callbacks.gro_receive)
			goto out_unlock;

		iph = ipv6_hdr(skb);
	}

	NAPI_GRO_CB(skb)->proto = proto;

	flush--;
	nlen = skb_network_header_len(skb);

	for (p = *head; p; p = p->next) {
		const struct ipv6hdr *iph2;
		__be32 first_word; /* <Version:4><Traffic_Class:8><Flow_Label:20> */

		if (!NAPI_GRO_CB(p)->same_flow)
			continue;

		iph2 = (struct ipv6hdr *)(p->data + off);
		first_word = *(__be32 *)iph ^ *(__be32 *)iph2;

		/* All fields must match except length and Traffic Class.
		 * XXX skbs on the gro_list have all been parsed and pulled
		 * already so we don't need to compare nlen
		 * (nlen != (sizeof(*iph2) + ipv6_exthdrs_len(iph2, &ops)))
		 * memcmp() alone below is suffcient, right?
		 */
		 if ((first_word & htonl(0xF00FFFFF)) ||
		    memcmp(&iph->nexthdr, &iph2->nexthdr,
			   nlen - offsetof(struct ipv6hdr, nexthdr))) {
			NAPI_GRO_CB(p)->same_flow = 0;
			continue;
		}
		/* flush if Traffic Class fields are different */
		NAPI_GRO_CB(p)->flush |= !!(first_word & htonl(0x0FF00000));
		NAPI_GRO_CB(p)->flush |= flush;

		/* Clear flush_id, there's really no concept of ID in IPv6. */
		NAPI_GRO_CB(p)->flush_id = 0;
	}

	NAPI_GRO_CB(skb)->flush |= flush;

	skb_gro_postpull_rcsum(skb, iph, nlen);

	pp = call_gro_receive(ops->callbacks.gro_receive, head, skb);

out_unlock:
	rcu_read_unlock();

out:
	NAPI_GRO_CB(skb)->flush |= flush;

	return pp;
}

#if defined(CONFIG_BCM_KF_MISC_BACKPORTS)
static struct sk_buff **sit_gro_receive(struct sk_buff **head,
					struct sk_buff *skb)
{
	if (NAPI_GRO_CB(skb)->encap_mark) {
		NAPI_GRO_CB(skb)->flush = 1;
		return NULL;
	}

	NAPI_GRO_CB(skb)->encap_mark = 1;

	return ipv6_gro_receive(head, skb);
}
#endif

static int ipv6_gro_complete(struct sk_buff *skb, int nhoff)
{
	const struct net_offload *ops;
	struct ipv6hdr *iph = (struct ipv6hdr *)(skb->data + nhoff);
	int err = -ENOSYS;

	iph->payload_len = htons(skb->len - nhoff - sizeof(*iph));

	rcu_read_lock();

	nhoff += sizeof(*iph) + ipv6_exthdrs_len(iph, &ops);
	if (WARN_ON(!ops || !ops->callbacks.gro_complete))
		goto out_unlock;

	err = ops->callbacks.gro_complete(skb, nhoff);

out_unlock:
	rcu_read_unlock();

	return err;
}

static struct packet_offload ipv6_packet_offload __read_mostly = {
	.type = cpu_to_be16(ETH_P_IPV6),
	.callbacks = {
		.gso_segment = ipv6_gso_segment,
		.gro_receive = ipv6_gro_receive,
		.gro_complete = ipv6_gro_complete,
	},
};

static const struct net_offload sit_offload = {
	.callbacks = {
		.gso_segment	= ipv6_gso_segment,
#if defined(CONFIG_BCM_KF_MISC_BACKPORTS)
		.gro_receive    = sit_gro_receive,
#endif
	},
};

static int __init ipv6_offload_init(void)
{

	if (tcpv6_offload_init() < 0)
		pr_crit("%s: Cannot add TCP protocol offload\n", __func__);
	if (udp_offload_init() < 0)
		pr_crit("%s: Cannot add UDP protocol offload\n", __func__);
	if (ipv6_exthdrs_offload_init() < 0)
		pr_crit("%s: Cannot add EXTHDRS protocol offload\n", __func__);

	dev_add_offload(&ipv6_packet_offload);

	inet_add_offload(&sit_offload, IPPROTO_IPV6);

	return 0;
}

fs_initcall(ipv6_offload_init);
