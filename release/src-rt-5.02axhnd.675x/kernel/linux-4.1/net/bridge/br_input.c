/*
 *	Handle incoming frames
 *	Linux ethernet bridge
 *
 *	Authors:
 *	Lennert Buytenhek		<buytenh@gnu.org>
 *
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License
 *	as published by the Free Software Foundation; either version
 *	2 of the License, or (at your option) any later version.
 */

#include <linux/slab.h>
#include <linux/kernel.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/netfilter_bridge.h>
#include <linux/neighbour.h>
#include <net/arp.h>
#include <linux/export.h>
#include <linux/rculist.h>
#include "br_private.h"
#if defined(CONFIG_BCM_KF_BLOG) && defined(CONFIG_BLOG)
#include <linux/blog.h>
#endif
#if defined(CONFIG_BCM_KF_WL)
#if defined(PKTC)
#include <osl.h>
#include <wl_pktc.h>
unsigned long (*wl_pktc_req_hook)(int req_id, unsigned long param0, unsigned long param1, unsigned long param2) = NULL;
EXPORT_SYMBOL(wl_pktc_req_hook);
unsigned long (*dhd_pktc_req_hook)(int req_id, unsigned long param0, unsigned long param1, unsigned long param2) = NULL;
EXPORT_SYMBOL(dhd_pktc_req_hook);
#endif /* PKTC */
#include <linux/bcm_skb_defines.h>
#endif

/* Hook for brouter */
br_should_route_hook_t __rcu *br_should_route_hook __read_mostly;
EXPORT_SYMBOL(br_should_route_hook);

static int br_pass_frame_up(struct sk_buff *skb)
{
	struct net_device *indev, *brdev = BR_INPUT_SKB_CB(skb)->brdev;
	struct net_bridge *br = netdev_priv(brdev);
	struct pcpu_sw_netstats *brstats = this_cpu_ptr(br->stats);
	struct net_port_vlans *pv;

#if defined(CONFIG_BCM_KF_BLOG) && defined(CONFIG_BLOG)
	blog_lock();
	blog_link(IF_DEVICE, blog_ptr(skb), (void*)br->dev, DIR_RX, skb->len);
	blog_unlock();
#endif

	u64_stats_update_begin(&brstats->syncp);
	brstats->rx_packets++;
	brstats->rx_bytes += skb->len;
	u64_stats_update_end(&brstats->syncp);

	/* Bridge is just like any other port.  Make sure the
	 * packet is allowed except in promisc modue when someone
	 * may be running packet capture.
	 */
	pv = br_get_vlan_info(br);
	if (!(brdev->flags & IFF_PROMISC) &&
	    !br_allowed_egress(br, pv, skb)) {
		kfree_skb(skb);
		return NET_RX_DROP;
	}

	indev = skb->dev;
	skb->dev = brdev;
	skb = br_handle_vlan(br, pv, skb);
	if (!skb)
		return NET_RX_DROP;

	return NF_HOOK(NFPROTO_BRIDGE, NF_BR_LOCAL_IN, NULL, skb,
		       indev, NULL,
		       netif_receive_skb_sk);
}

static void br_do_proxy_arp(struct sk_buff *skb, struct net_bridge *br,
			    u16 vid, struct net_bridge_port *p)
{
	struct net_device *dev = br->dev;
	struct neighbour *n;
	struct arphdr *parp;
	u8 *arpptr, *sha;
	__be32 sip, tip;

	BR_INPUT_SKB_CB(skb)->proxyarp_replied = false;

	if (dev->flags & IFF_NOARP)
		return;

	if (!pskb_may_pull(skb, arp_hdr_len(dev))) {
		dev->stats.tx_dropped++;
		return;
	}
	parp = arp_hdr(skb);

	if (parp->ar_pro != htons(ETH_P_IP) ||
	    parp->ar_op != htons(ARPOP_REQUEST) ||
	    parp->ar_hln != dev->addr_len ||
	    parp->ar_pln != 4)
		return;

	arpptr = (u8 *)parp + sizeof(struct arphdr);
	sha = arpptr;
	arpptr += dev->addr_len;	/* sha */
	memcpy(&sip, arpptr, sizeof(sip));
	arpptr += sizeof(sip);
	arpptr += dev->addr_len;	/* tha */
	memcpy(&tip, arpptr, sizeof(tip));

	if (ipv4_is_loopback(tip) ||
	    ipv4_is_multicast(tip))
		return;

	n = neigh_lookup(&arp_tbl, &tip, dev);
	if (n) {
		struct net_bridge_fdb_entry *f;

		if (!(n->nud_state & NUD_VALID)) {
			neigh_release(n);
			return;
		}

		f = __br_fdb_get(br, n->ha, vid);
		if (f && ((p->flags & BR_PROXYARP) ||
			  (f->dst && (f->dst->flags & BR_PROXYARP_WIFI)))) {
			arp_send(ARPOP_REPLY, ETH_P_ARP, sip, skb->dev, tip,
				 sha, n->ha, sha);
			BR_INPUT_SKB_CB(skb)->proxyarp_replied = true;
		}

		neigh_release(n);
	}
}

#if defined(CONFIG_BCM_KF_WL)
//  ETHER_TYPE_BRCM 0x886c, ETHER_TYPE_802_1X 0x888e, ETHER_TYPE_802_1X_PREAUTH 0x88c7
#define WL_AUTH_PROTOCOLS(proto)    ((proto)==htons(0x886c)||(proto)==htons(0x888e)||(proto)==htons(0x88c7))
#endif

/* note: already called with rcu_read_lock */
int br_handle_frame_finish(struct sock *sk, struct sk_buff *skb)
{
	const unsigned char *dest = eth_hdr(skb)->h_dest;
	struct net_bridge_port *p = br_port_get_rcu(skb->dev);
	struct net_bridge *br;
	struct net_bridge_fdb_entry *dst;
	struct net_bridge_mdb_entry *mdst;
	struct sk_buff *skb2;
	bool unicast = true;
	u16 vid = 0;

#if defined(CONFIG_BCM_KF_WL)
	if (!p || (p->state == BR_STATE_DISABLED && !WL_AUTH_PROTOCOLS(skb->protocol)))
#else
	if (!p || p->state == BR_STATE_DISABLED)
#endif
		goto drop;

	if (!br_allowed_ingress(p->br, nbp_get_vlan_info(p), skb, &vid))
		goto out;

#if defined(CONFIG_BCM_KF_VLAN_AGGREGATION) && defined(CONFIG_BCM_VLAN_AGGREGATION)
#if defined(CONFIG_BCM_KF_VLAN) && (defined(CONFIG_BCM_VLAN) || defined(CONFIG_BCM_VLAN_MODULE))
	if (skb->vlan_count)
 		vid = (skb->vlan_header[0] >> 16) & VLAN_VID_MASK;
	else
#endif /* CONFIG_BCM_VLAN) */
	/* 
	*  dev.c/__netif_receive_skb(): if proto == ETH_P_8021Q
	*  call vlan_untag() to remove tag and save vid in skb->vlan_tci
	*/
	if (skb_vlan_tag_present(skb))
		vid = skb->vlan_tci & VLAN_VID_MASK;
	else if ( vlan_eth_hdr(skb)->h_vlan_proto == htons(ETH_P_8021Q) )
		vid = ntohs(vlan_eth_hdr(skb)->h_vlan_TCI) & VLAN_VID_MASK;
#endif

	/* insert into forwarding database after filtering to avoid spoofing */
	br = p->br;
	if (p->flags & BR_LEARNING)
		br_fdb_update(br, p, eth_hdr(skb)->h_source, vid, false);

	if (!is_broadcast_ether_addr(dest) && is_multicast_ether_addr(dest) &&
	    br_multicast_rcv(br, p, skb, vid))
		goto drop;

	if (p->state == BR_STATE_LEARNING)
#if defined(CONFIG_BCM_KF_WL)
      if (!WL_AUTH_PROTOCOLS(skb->protocol))
#endif
		goto drop;

	BR_INPUT_SKB_CB(skb)->brdev = br->dev;

	/* The packet skb2 goes to the local host (NULL to skip). */
	skb2 = NULL;

	if (br->dev->flags & IFF_PROMISC)
		skb2 = skb;

	dst = NULL;

	if (IS_ENABLED(CONFIG_INET) && skb->protocol == htons(ETH_P_ARP))
		br_do_proxy_arp(skb, br, vid, p);

#if (defined(CONFIG_BCM_MCAST) || defined(CONFIG_BCM_MCAST_MODULE)) && defined(CONFIG_BCM_KF_MCAST)
	if ( br_bcm_mcast_receive != NULL )
	{
		int rv = br_bcm_mcast_receive(br->dev->ifindex, skb, 0);
		if ( rv < 0 )
		{
			/* there was an error with the packet */
			goto drop;
		}
		else if ( rv > 0 )
		{
			/* the packet was consumed */
			goto out;
		}
		/* continue */
	}
#endif

	if (is_broadcast_ether_addr(dest)) {
		skb2 = skb;
		unicast = false;
	} else if (is_multicast_ether_addr(dest)) {
		mdst = br_mdb_get(br, skb, vid);
		if ((mdst || BR_INPUT_SKB_CB_MROUTERS_ONLY(skb)) &&
		    br_multicast_querier_exists(br, eth_hdr(skb))) {
			if ((mdst && mdst->mglist) ||
			    br_multicast_is_router(br))
				skb2 = skb;
			br_multicast_forward(mdst, skb, skb2);
			skb = NULL;
			if (!skb2)
				goto out;
		} else
			skb2 = skb;

		unicast = false;
		br->dev->stats.multicast++;
#if !(defined(CONFIG_BCM_KF_BLOG) && defined(CONFIG_BLOG))
	} else if ((p->flags & BR_ISOLATE_MODE) ||
			((dst = __br_fdb_get(br, dest, vid)) && dst->is_local)) {
		skb2 = skb;
		/* Do not forward the packet since it's local. */
		skb = NULL;
	}
#else
	} else {
		struct net_bridge_fdb_entry *src;

		dst = __br_fdb_get(br, dest, vid);
		src = __br_fdb_get(br, eth_hdr(skb)->h_source, vid);
		blog_lock();
		if (src)
			blog_link(BRIDGEFDB, blog_ptr(skb), (void*)src, BLOG_PARAM1_SRCFDB, 0);

		if (dst)
			blog_link(BRIDGEFDB, blog_ptr(skb), (void*)dst, BLOG_PARAM1_DSTFDB, 0);

		blog_unlock();

#if defined(PKTC)
		/* wlan pktc */
		if ((dst != NULL) && (dst->dst != NULL) && (!dst->is_local)) {
#if defined(CONFIG_BCM_KF_WL)
			u8 from_wl_to_switch=0, from_switch_to_wl=0, from_wlan_to_wlan=0;
			BlogPhy_t srcPhyType, dstPhyType;
			uint32_t chainIdx;
			uint16_t dst_dev_has_vlan = 0;
			uint32_t pktc_tx_enabled = wl_pktc_req_hook ? 
						wl_pktc_req_hook(PKTC_TBL_GET_TX_MODE, 0, 0, 0) : 0;

			src = __br_fdb_get(br, eth_hdr(skb)->h_source, vid);
			if (unlikely(src == NULL) || unlikely(src->dst == NULL))
				goto next;

			srcPhyType = BLOG_GET_PHYTYPE(src->dst->dev->path.hw_port_type);
			dstPhyType = BLOG_GET_PHYTYPE(dst->dst->dev->path.hw_port_type);

			if ((srcPhyType == BLOG_WLANPHY) &&
			    (dstPhyType == BLOG_ENETPHY)) {
				from_wl_to_switch = 1;
			} else if ((srcPhyType == BLOG_ENETPHY || srcPhyType == BLOG_XTMPHY || srcPhyType == BLOG_EPONPHY || srcPhyType == BLOG_GPONPHY ||
				((src->dst->dev->rtnl_link_ops != NULL) && (strstr(src->dst->dev->rtnl_link_ops->kind, "gre") != NULL))) &&
 				(dstPhyType == BLOG_WLANPHY) && pktc_tx_enabled)
  			{ 
				from_switch_to_wl = 1;
   			}

#if defined (CONFIG_BCM_DHD_RUNNER) && defined (CONFIG_BCM_WIFI_FORWARDING_DRV_MODULE) || defined(CONFIG_BCM947189) || defined(CONFIG_BCM_ARCHER_WLAN)
			else if((srcPhyType == BLOG_WLANPHY) && (dstPhyType == BLOG_WLANPHY))
				from_wlan_to_wlan = 1;
#endif
#if defined(CONFIG_BCM_KF_WANDEV)

			if ((dst->dst->dev->priv_flags & IFF_BCM_VLAN))
			{
			    int len = strlen(dst->dst->dev->name);
			    dst_dev_has_vlan = 1;

			    /* WLAN Rx: Check dev is default LAN side VLAN (for ex:eth1.0 or eth2.0) 
                               that doesn't add/remove VLAN tag. So still create PKTC for it. 
			       WLAN Tx: Allow to create PKTC as VLAN tag modifications are performed by the accelerator(fcache/fap/runner).
			    */

			    if ((!(dst->dst->dev->priv_flags & IFF_WANDEV)) && (len > 3))
			    {
					if (( from_wl_to_switch && (dst->dst->dev->name[len-2] == '.') && (dst->dst->dev->name[len-1] == '0') )
						|| from_switch_to_wl )
					{
						dst_dev_has_vlan = 0;
					}
			    }
			}

			if ((from_wl_to_switch || from_switch_to_wl || from_wlan_to_wlan) &&
			    !(dst->dst->dev->priv_flags & IFF_WANDEV) &&
			    !(dst_dev_has_vlan)) {

			/* Also check for non-WAN cases.
			 * For the Rx direction, VLAN cases are allowed as long 
			 * as the packets are untagged.
			 *
			 * Tagged packets are not forwarded through the chaining 
			 * path by WLAN driver. Tagged packets go through the
			 * flowcache path.
			 * see wlc_sendup_chain() function for reference.
			 *
			 * For the Tx direction, there are no VLAN interfaces 
			 * created on wl device when LAN_VLAN flag is enabled 
			 * in the build.
			 *
			 * Get the root dest device and make sure that we 
			 * are always transmitting to a root device */

				struct net_device *root_dst_dev_p = dst->dst->dev;
				/* Get the root destination device */
				while (!netdev_path_is_root(root_dst_dev_p)) {
					root_dst_dev_p = netdev_path_next_dev(root_dst_dev_p);
				}
 
				/* Update chaining table for DHD on the wl to switch direction only */
				if ( (    from_wl_to_switch
#if defined(CONFIG_BCM947189)
  			   	      || from_wlan_to_wlan
#endif
				    )
				    && (dhd_pktc_req_hook != NULL)) 
				{
					dhd_pktc_req_hook(PKTC_TBL_UPDATE,
								     (unsigned long)&(dst->addr.addr[0]),
								     (unsigned long)root_dst_dev_p, 0);
				}
			 
			 	/* Update chaining table for WL (NIC driver) */
				chainIdx = wl_pktc_req_hook ? 
								wl_pktc_req_hook(PKTC_TBL_UPDATE,
								     (unsigned long)&(dst->addr.addr[0]),
								     (unsigned long)root_dst_dev_p, 0) : PKTC_INVALID_CHAIN_IDX;
				if (chainIdx != PKTC_INVALID_CHAIN_IDX) {
					/* Update chainIdx in blog
					 * chainEntry->tx_dev will always be NOT 
					 * NULL as we just added that above */
					if (skb->blog_p != NULL) 
					{
						if (from_switch_to_wl || from_wlan_to_wlan)
						{
							skb->blog_p->wfd.nic_ucast.is_tx_hw_acc_en = 1;

							/* in case of flow from WLAN to WLAN the flow will
							 *  be open in Runner only if is_rx_hw_acc_en in DHD
							 */
							if (from_wlan_to_wlan && (0 == skb->blog_p->rnr.is_rx_hw_acc_en))
								skb->blog_p->wfd.nic_ucast.is_tx_hw_acc_en = 0; 

							skb->blog_p->wfd.nic_ucast.is_chain = 1;
							skb->blog_p->wfd.nic_ucast.wfd_idx = ((chainIdx & PKTC_WFD_IDX_BITMASK) >> PKTC_WFD_IDX_BITPOS);
							skb->blog_p->wfd.nic_ucast.chain_idx = chainIdx;
						}
					}
				}
			}
#endif /* CONFIG_BCM_KF_WANDEV */
#endif
		}
next:
#endif /* PKTC */
		if ((p->flags & BR_ISOLATE_MODE) || ((dst != NULL) && dst->is_local)) {
			skb2 = skb;
			/* Do not forward the packet since it's local. */
			skb = NULL;
		}
	}
#endif
	if (skb) {
		if (dst) {
			dst->used = jiffies;
			br_forward(dst->dst, skb, skb2);
		} else
			br_flood_forward(br, skb, skb2, unicast);
	}

	if (skb2)
		return br_pass_frame_up(skb2);

out:
	return 0;
drop:
	kfree_skb(skb);
	goto out;
}
EXPORT_SYMBOL_GPL(br_handle_frame_finish);

/* note: already called with rcu_read_lock */
static int br_handle_local_finish(struct sock *sk, struct sk_buff *skb)
{
	struct net_bridge_port *p = br_port_get_rcu(skb->dev);
	u16 vid = 0;

	/* check if vlan is allowed, to avoid spoofing */
	if (p->flags & BR_LEARNING && br_should_learn(p, skb, &vid))
		br_fdb_update(p->br, p, eth_hdr(skb)->h_source, vid, false);
	return 0;	 /* process further */
}

/*
 * Return NULL if skb is handled
 * note: already called with rcu_read_lock
 */
rx_handler_result_t br_handle_frame(struct sk_buff **pskb)
{
	struct net_bridge_port *p;
	struct sk_buff *skb = *pskb;
	const unsigned char *dest = eth_hdr(skb)->h_dest;
	br_should_route_hook_t *rhook;

	if (unlikely(skb->pkt_type == PACKET_LOOPBACK))
		return RX_HANDLER_PASS;

	if (!is_valid_ether_addr(eth_hdr(skb)->h_source))
		goto drop;

	skb = skb_share_check(skb, GFP_ATOMIC);
	if (!skb)
		return RX_HANDLER_CONSUMED;

	p = br_port_get_rcu(skb->dev);

#if defined(CONFIG_BCM_KF_WANDEV)
        if (!p)
        {
            kfree_skb(skb);
            return RX_HANDLER_CONSUMED;
        }
#endif
	if (unlikely(is_link_local_ether_addr(dest))) {
		u16 fwd_mask = p->br->group_fwd_mask_required;

		/*
		 * See IEEE 802.1D Table 7-10 Reserved addresses
		 *
		 * Assignment		 		Value
		 * Bridge Group Address		01-80-C2-00-00-00
		 * (MAC Control) 802.3		01-80-C2-00-00-01
		 * (Link Aggregation) 802.3	01-80-C2-00-00-02
		 * 802.1X PAE address		01-80-C2-00-00-03
		 *
		 * 802.1AB LLDP 		01-80-C2-00-00-0E
		 *
		 * Others reserved for future standardization
		 */
		switch (dest[5]) {
		case 0x00:	/* Bridge Group Address */
			/* If STP is turned off,
			   then must forward to keep loop detection */
			if (p->br->stp_enabled == BR_NO_STP ||
			    fwd_mask & (1u << dest[5]))
				goto forward;
			break;

		case 0x01:	/* IEEE MAC (Pause) */
			goto drop;

		default:
			/* Allow selective forwarding for most other protocols */
			fwd_mask |= p->br->group_fwd_mask;
			if (fwd_mask & (1u << dest[5]))
				goto forward;
		}

		/* Deliver packet to local host only */
		if (NF_HOOK(NFPROTO_BRIDGE, NF_BR_LOCAL_IN, NULL, skb,
			    skb->dev, NULL, br_handle_local_finish)) {
			return RX_HANDLER_CONSUMED; /* consumed by filter */
		} else {
			*pskb = skb;
			return RX_HANDLER_PASS;	/* continue processing */
		}
	}

forward:
#if defined(CONFIG_BCM_KF_IEEE1905) && defined(CONFIG_BCM_IEEE1905)
	/* allow broute to forward packets to the stack in any STP state */
	rhook = rcu_dereference(br_should_route_hook);
	if (rhook) {
		if ((*rhook)(skb)) {
			*pskb = skb;
			if ((skb->protocol == htons(0x893a)) ||
			    (skb->protocol == htons(0x8912)) ||
			    (skb->protocol == htons(0x88e1)))
				br_handle_local_finish(NULL, skb);

			return RX_HANDLER_PASS;
		} else if (skb->protocol == htons(0x893a) &&
			   (skb->pkt_type == PACKET_MULTICAST))
			/* do not bridge multicast 1905 packets when 1905 is compiled */
			goto drop;

		dest = eth_hdr(skb)->h_dest;
	}
#endif

#if defined(CONFIG_BCM_KF_WL)
    if ((p->state != BR_STATE_FORWARDING) && WL_AUTH_PROTOCOLS(skb->protocol)) {
		/* force to forward brcm_type event packet */
		NF_HOOK(NFPROTO_BRIDGE, NF_BR_PRE_ROUTING, NULL, skb, skb->dev, NULL,
			br_handle_frame_finish);
		return RX_HANDLER_CONSUMED;
	}
#endif

	switch (p->state) {
	case BR_STATE_FORWARDING:
#if !defined(CONFIG_BCM_KF_IEEE1905) || !defined(CONFIG_BCM_IEEE1905)
		rhook = rcu_dereference(br_should_route_hook);
		if (rhook) {
			if ((*rhook)(skb)) {
				*pskb = skb;
				return RX_HANDLER_PASS;
			}
			dest = eth_hdr(skb)->h_dest;
		}
#endif
		/* fall through */
	case BR_STATE_LEARNING:
		if (ether_addr_equal(p->br->dev->dev_addr, dest))
			skb->pkt_type = PACKET_HOST;

		NF_HOOK(NFPROTO_BRIDGE, NF_BR_PRE_ROUTING, NULL, skb,
			skb->dev, NULL,
			br_handle_frame_finish);
		break;
	default:
drop:
		kfree_skb(skb);
	}
	return RX_HANDLER_CONSUMED;
}
