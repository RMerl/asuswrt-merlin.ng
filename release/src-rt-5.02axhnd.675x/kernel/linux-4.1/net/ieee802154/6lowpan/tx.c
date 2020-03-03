/* This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <net/6lowpan.h>
#include <net/ieee802154_netdev.h>

#include "6lowpan_i.h"

/* don't save pan id, it's intra pan */
struct lowpan_addr {
	u8 mode;
	union {
		/* IPv6 needs big endian here */
		__be64 extended_addr;
		__be16 short_addr;
	} u;
};

struct lowpan_addr_info {
	struct lowpan_addr daddr;
	struct lowpan_addr saddr;
};

static inline struct
lowpan_addr_info *lowpan_skb_priv(const struct sk_buff *skb)
{
	WARN_ON_ONCE(skb_headroom(skb) < sizeof(struct lowpan_addr_info));
	return (struct lowpan_addr_info *)(skb->data -
			sizeof(struct lowpan_addr_info));
}

int lowpan_header_create(struct sk_buff *skb, struct net_device *dev,
			 unsigned short type, const void *_daddr,
			 const void *_saddr, unsigned int len)
{
	const u8 *saddr = _saddr;
	const u8 *daddr = _daddr;
	struct lowpan_addr_info *info;

	/* TODO:
	 * if this package isn't ipv6 one, where should it be routed?
	 */
	if (type != ETH_P_IPV6)
		return 0;

	if (!saddr)
		saddr = dev->dev_addr;

	raw_dump_inline(__func__, "saddr", (unsigned char *)saddr, 8);
	raw_dump_inline(__func__, "daddr", (unsigned char *)daddr, 8);

	info = lowpan_skb_priv(skb);

	/* TODO: Currently we only support extended_addr */
	info->daddr.mode = IEEE802154_ADDR_LONG;
	memcpy(&info->daddr.u.extended_addr, daddr,
	       sizeof(info->daddr.u.extended_addr));
	info->saddr.mode = IEEE802154_ADDR_LONG;
	memcpy(&info->saddr.u.extended_addr, saddr,
	       sizeof(info->daddr.u.extended_addr));

	return 0;
}

static struct sk_buff*
lowpan_alloc_frag(struct sk_buff *skb, int size,
		  const struct ieee802154_hdr *master_hdr)
{
	struct net_device *real_dev = lowpan_dev_info(skb->dev)->real_dev;
	struct sk_buff *frag;
	int rc;

	frag = alloc_skb(real_dev->hard_header_len +
			 real_dev->needed_tailroom + size,
			 GFP_ATOMIC);

	if (likely(frag)) {
		frag->dev = real_dev;
		frag->priority = skb->priority;
		skb_reserve(frag, real_dev->hard_header_len);
		skb_reset_network_header(frag);
		*mac_cb(frag) = *mac_cb(skb);

		rc = dev_hard_header(frag, real_dev, 0, &master_hdr->dest,
				     &master_hdr->source, size);
		if (rc < 0) {
			kfree_skb(frag);
			return ERR_PTR(rc);
		}
	} else {
		frag = ERR_PTR(-ENOMEM);
	}

	return frag;
}

static int
lowpan_xmit_fragment(struct sk_buff *skb, const struct ieee802154_hdr *wpan_hdr,
		     u8 *frag_hdr, int frag_hdrlen,
		     int offset, int len)
{
	struct sk_buff *frag;

	raw_dump_inline(__func__, " fragment header", frag_hdr, frag_hdrlen);

	frag = lowpan_alloc_frag(skb, frag_hdrlen + len, wpan_hdr);
	if (IS_ERR(frag))
		return -PTR_ERR(frag);

	memcpy(skb_put(frag, frag_hdrlen), frag_hdr, frag_hdrlen);
	memcpy(skb_put(frag, len), skb_network_header(skb) + offset, len);

	raw_dump_table(__func__, " fragment dump", frag->data, frag->len);

	return dev_queue_xmit(frag);
}

static int
lowpan_xmit_fragmented(struct sk_buff *skb, struct net_device *dev,
		       const struct ieee802154_hdr *wpan_hdr)
{
	u16 dgram_size, dgram_offset;
	__be16 frag_tag;
	u8 frag_hdr[5];
	int frag_cap, frag_len, payload_cap, rc;
	int skb_unprocessed, skb_offset;

	dgram_size = lowpan_uncompress_size(skb, &dgram_offset) -
		     skb->mac_len;
	frag_tag = htons(lowpan_dev_info(dev)->fragment_tag);
	lowpan_dev_info(dev)->fragment_tag++;

	frag_hdr[0] = LOWPAN_DISPATCH_FRAG1 | ((dgram_size >> 8) & 0x07);
	frag_hdr[1] = dgram_size & 0xff;
	memcpy(frag_hdr + 2, &frag_tag, sizeof(frag_tag));

	payload_cap = ieee802154_max_payload(wpan_hdr);

	frag_len = round_down(payload_cap - LOWPAN_FRAG1_HEAD_SIZE -
			      skb_network_header_len(skb), 8);

	skb_offset = skb_network_header_len(skb);
	skb_unprocessed = skb->len - skb->mac_len - skb_offset;

	rc = lowpan_xmit_fragment(skb, wpan_hdr, frag_hdr,
				  LOWPAN_FRAG1_HEAD_SIZE, 0,
				  frag_len + skb_network_header_len(skb));
	if (rc) {
		pr_debug("%s unable to send FRAG1 packet (tag: %d)",
			 __func__, ntohs(frag_tag));
		goto err;
	}

	frag_hdr[0] &= ~LOWPAN_DISPATCH_FRAG1;
	frag_hdr[0] |= LOWPAN_DISPATCH_FRAGN;
	frag_cap = round_down(payload_cap - LOWPAN_FRAGN_HEAD_SIZE, 8);

	do {
		dgram_offset += frag_len;
		skb_offset += frag_len;
		skb_unprocessed -= frag_len;
		frag_len = min(frag_cap, skb_unprocessed);

		frag_hdr[4] = dgram_offset >> 3;

		rc = lowpan_xmit_fragment(skb, wpan_hdr, frag_hdr,
					  LOWPAN_FRAGN_HEAD_SIZE, skb_offset,
					  frag_len);
		if (rc) {
			pr_debug("%s unable to send a FRAGN packet. (tag: %d, offset: %d)\n",
				 __func__, ntohs(frag_tag), skb_offset);
			goto err;
		}
	} while (skb_unprocessed > frag_cap);

	consume_skb(skb);
	return NET_XMIT_SUCCESS;

err:
	kfree_skb(skb);
	return rc;
}

static int lowpan_header(struct sk_buff *skb, struct net_device *dev)
{
	struct ieee802154_addr sa, da;
	struct ieee802154_mac_cb *cb = mac_cb_init(skb);
	struct lowpan_addr_info info;
	void *daddr, *saddr;

	memcpy(&info, lowpan_skb_priv(skb), sizeof(info));

	/* TODO: Currently we only support extended_addr */
	daddr = &info.daddr.u.extended_addr;
	saddr = &info.saddr.u.extended_addr;

	lowpan_header_compress(skb, dev, ETH_P_IPV6, daddr, saddr, skb->len);

	cb->type = IEEE802154_FC_TYPE_DATA;

	/* prepare wpan address data */
	sa.mode = IEEE802154_ADDR_LONG;
	sa.pan_id = ieee802154_mlme_ops(dev)->get_pan_id(dev);
	sa.extended_addr = ieee802154_devaddr_from_raw(saddr);

	/* intra-PAN communications */
	da.pan_id = sa.pan_id;

	/* if the destination address is the broadcast address, use the
	 * corresponding short address
	 */
	if (lowpan_is_addr_broadcast((const u8 *)daddr)) {
		da.mode = IEEE802154_ADDR_SHORT;
		da.short_addr = cpu_to_le16(IEEE802154_ADDR_BROADCAST);
		cb->ackreq = false;
	} else {
		da.mode = IEEE802154_ADDR_LONG;
		da.extended_addr = ieee802154_devaddr_from_raw(daddr);
		cb->ackreq = true;
	}

	return dev_hard_header(skb, lowpan_dev_info(dev)->real_dev,
			ETH_P_IPV6, (void *)&da, (void *)&sa, 0);
}

netdev_tx_t lowpan_xmit(struct sk_buff *skb, struct net_device *dev)
{
	struct ieee802154_hdr wpan_hdr;
	int max_single, ret;

	pr_debug("package xmit\n");

	/* We must take a copy of the skb before we modify/replace the ipv6
	 * header as the header could be used elsewhere
	 */
	skb = skb_unshare(skb, GFP_ATOMIC);
	if (!skb)
		return NET_XMIT_DROP;

	ret = lowpan_header(skb, dev);
	if (ret < 0) {
		kfree_skb(skb);
		return NET_XMIT_DROP;
	}

	if (ieee802154_hdr_peek(skb, &wpan_hdr) < 0) {
		kfree_skb(skb);
		return NET_XMIT_DROP;
	}

	max_single = ieee802154_max_payload(&wpan_hdr);

	if (skb_tail_pointer(skb) - skb_network_header(skb) <= max_single) {
		skb->dev = lowpan_dev_info(dev)->real_dev;
		return dev_queue_xmit(skb);
	} else {
		netdev_tx_t rc;

		pr_debug("frame is too big, fragmentation is needed\n");
		rc = lowpan_xmit_fragmented(skb, dev, &wpan_hdr);

		return rc < 0 ? NET_XMIT_DROP : rc;
	}
}
