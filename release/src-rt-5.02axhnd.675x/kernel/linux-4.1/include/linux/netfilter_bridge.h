#ifndef __LINUX_BRIDGE_NETFILTER_H
#define __LINUX_BRIDGE_NETFILTER_H

#include <uapi/linux/netfilter_bridge.h>
#include <linux/skbuff.h>

enum nf_br_hook_priorities {
	NF_BR_PRI_FIRST = INT_MIN,
	NF_BR_PRI_NAT_DST_BRIDGED = -300,
	NF_BR_PRI_FILTER_BRIDGED = -200,
	NF_BR_PRI_BRNF = 0,
	NF_BR_PRI_NAT_DST_OTHER = 100,
	NF_BR_PRI_FILTER_OTHER = 200,
	NF_BR_PRI_NAT_SRC = 300,
	NF_BR_PRI_LAST = INT_MAX,
};

#if IS_ENABLED(CONFIG_BRIDGE_NETFILTER)

#define BRNF_BRIDGED_DNAT		0x02
#define BRNF_NF_BRIDGE_PREROUTING	0x08

static inline unsigned int nf_bridge_mtu_reduction(const struct sk_buff *skb)
{
	if (skb->nf_bridge->orig_proto == BRNF_PROTO_PPPOE)
		return PPPOE_SES_HLEN;
	return 0;
}

int br_handle_frame_finish(struct sock *sk, struct sk_buff *skb);

static inline void br_drop_fake_rtable(struct sk_buff *skb)
{
	struct dst_entry *dst = skb_dst(skb);

	if (dst && (dst->flags & DST_FAKE_RTABLE))
		skb_dst_drop(skb);
}

static inline int nf_bridge_get_physinif(const struct sk_buff *skb)
{
	struct nf_bridge_info *nf_bridge;

	if (skb->nf_bridge == NULL)
		return 0;

	nf_bridge = skb->nf_bridge;
	return nf_bridge->physindev ? nf_bridge->physindev->ifindex : 0;
}

static inline int nf_bridge_get_physoutif(const struct sk_buff *skb)
{
	struct nf_bridge_info *nf_bridge;

	if (skb->nf_bridge == NULL)
		return 0;

	nf_bridge = skb->nf_bridge;
	return nf_bridge->physoutdev ? nf_bridge->physoutdev->ifindex : 0;
}

static inline struct net_device *
nf_bridge_get_physindev(const struct sk_buff *skb)
{
	return skb->nf_bridge ? skb->nf_bridge->physindev : NULL;
}

static inline struct net_device *
nf_bridge_get_physoutdev(const struct sk_buff *skb)
{
	return skb->nf_bridge ? skb->nf_bridge->physoutdev : NULL;
}
#else
#define br_drop_fake_rtable(skb)	        do { } while (0)
#endif /* CONFIG_BRIDGE_NETFILTER */

#endif
