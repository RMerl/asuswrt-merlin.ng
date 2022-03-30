
#include <linux/nbuff.h>
#include <net/sock.h>


static inline struct sk_buff *bcm_find_skb_by_flow_id(uint32_t flowid)
{
	/* TODO add this function later,needed for coalescing */
	return NULL;
}

static inline void set_skb_fields(FkBuff_t *fkb, struct sk_buff *skb
	, struct net_device *dev)
{
	/*TODO check if we can use skb_dst_set_noref as blog holds reference*/

	if(fkb->dst_entry){
		dst_hold(fkb->dst_entry);
		skb_dst_set(skb, fkb->dst_entry);
	}
	skb->dev = dev;
	skb->skb_iif = dev->ifindex;
	return;
}

static inline void position_skb_ptrs_to_transport(struct sk_buff *skb, BlogFcArgs_t *fc_args)
{

	/*initialize ip & tcp header related fields in skb */
	skb_set_mac_header(skb, 0); 
	skb_set_network_header(skb, fc_args->tx_l3_offset);
	skb_set_transport_header(skb, fc_args->tx_l4_offset);

    /*position data pointer to start of TCP hdr */
	skb_pull(skb,fc_args->tx_l4_offset);
	skb->pkt_type = PACKET_HOST;
	return;
}

extern int tcp_v4_rcv(struct sk_buff *skb);

/* inject the packet into ipv4_tcp_stack  directly from the network driver */
int bcm_tcp_v4_recv(pNBuff_t pNBuff, BlogFcArgs_t *fc_args)
{
	struct sk_buff *skb;
	FkBuff_t *fkb;

	if(IS_FKBUFF_PTR(pNBuff))
	{
		fkb = PNBUFF_2_FKBUFF(pNBuff);
		/* Translate the fkb to skb */
		/* find the skb for flowid or allocate a new skb */
		skb = bcm_find_skb_by_flow_id(fkb->flowid);

		if(!skb)
		{
			skb = skb_xlate_dp(fkb, NULL);

			if(!skb)
			{
				nbuff_free(fkb);
				return 0;
			}

		}
		skb->mark=0;
		skb->priority=0;
	}
	else
	{
		skb = PNBUFF_2_SKBUFF(pNBuff);
		fkb = (FkBuff_t *)&skb->fkbInSkb;
	}

	set_skb_fields(fkb, skb, fc_args->txdev_p);
	position_skb_ptrs_to_transport(skb, fc_args);

	 /*
	 * bh_disable is needed to prevent deadlock on sock_lock when TCP timers
	 * are executed
	 */
	if (skb) {
		local_bh_disable();
		tcp_v4_rcv(skb);
		local_bh_enable();
	}
      
	return 0;
}
EXPORT_SYMBOL(bcm_tcp_v4_recv);

static const struct net_device_ops bcm_tcp4_netdev_ops = {
	.ndo_open	= NULL,
	.ndo_stop	= NULL,
	.ndo_start_xmit	= (HardStartXmitFuncP)bcm_tcp_v4_recv,
	.ndo_set_mac_address = NULL,
	.ndo_do_ioctl	= NULL,
	.ndo_tx_timeout	= NULL,
	.ndo_get_stats	= NULL,
	.ndo_change_mtu	= NULL 
};

struct net_device  bcm_tcp4_netdev = {
	.name		= "tcp4_netdev",
	/* set it to 64K incase we aggregate pkts in HW in future */
	.mtu		= 64 * 1024,
	.netdev_ops	= &bcm_tcp4_netdev_ops
};

inline static int ethernet_offset_heuristic(struct sk_buff *skb)
{
        uint16_t offset = BLOG_ETH_HDR_LEN ;
        uint8_t *ether_type_loc = 
            (skb_transport_header(skb)-BLOG_IPV4_HDR_LEN-BLOG_ETH_TYPE_LEN);
        
        if ( ntohs(*(uint16_t*)ether_type_loc) == BLOG_ETH_P_IPV4 )
        {
            offset += BLOG_IPV4_HDR_LEN;
            goto parse_success;
        }
        else
        {
            ether_type_loc -= BLOG_PPPOE_HDR_LEN;
            
            if ( ntohs(*(uint16_t*)ether_type_loc) == BLOG_ETH_P_PPP_SES )
            {
                offset += (BLOG_PPPOE_HDR_LEN + BLOG_IPV4_HDR_LEN);
                goto parse_success;
            }
            goto parse_fail;
        }
        goto parse_fail;

parse_success:
        return offset;
parse_fail:
        return 0;

}

int bcm_tcp_v4_blog_emit(struct sk_buff *skb, struct sock *sk)
{

   if(skb->blog_p && skb->blog_p->l2_mode)
   {
      blog_skip(skb,blog_skip_reason_l2_local_termination);
   }
   else if( (sk && sk->sk_state == TCP_ESTABLISHED) && skb->blog_p &&
		( skb->blog_p->rx.info.phyHdrType == BLOG_ENETPHY
		|| skb->blog_p->rx.info.phyHdrType == BLOG_EPONPHY
		|| skb->blog_p->rx.info.phyHdrType == BLOG_GPONPHY
      || ((skb->blog_p->rx.info.phyHdrType == BLOG_XTMPHY)
         && (skb->blog_p->rx.info.bmap.ETH_802x == 1))) )
	{
		struct net_device *tmpdev;
        int offset = ethernet_offset_heuristic(skb);
        skb_push(skb,offset);
		tmpdev = skb->dev;
		skb->dev = &bcm_tcp4_netdev;
		blog_emit(skb, tmpdev, TYPE_ETH, 0, BLOG_TCP4_LOCALPHY);
		skb->dev = tmpdev;
        skb_pull(skb,offset);
	}
	else{
		/*unsupported local tcp */
      blog_skip(skb, blog_skip_reason_local_tcp_termination);
	}
	
   return 0;
}

