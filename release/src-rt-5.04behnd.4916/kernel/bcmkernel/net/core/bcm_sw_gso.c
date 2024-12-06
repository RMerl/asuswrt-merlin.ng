/*
* <:copyright-BRCM:2018:DUAL/GPL:standard
* 
*    Copyright (c) 2018 Broadcom 
*    All Rights Reserved
* 
* Unless you and Broadcom execute a separate written software license
* agreement governing use of this software, this software is licensed
* to you under the terms of the GNU General Public License version 2
* (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
* with the following added to such license:
* 
*    As a special exception, the copyright holders of this software give
*    you permission to link this software with independent modules, and
*    to copy and distribute the resulting executable under terms of your
*    choice, provided that you also meet, for each linked independent
*    module, the terms and conditions of the license of that module.
*    An independent module is a module which is not derived from this
*    software.  The special exception does not apply to any modifications
*    of the software.
* 
* Not withstanding the above, under no circumstances may you combine
* this software in any way with any other Broadcom software provided
* under a license other than the GPL, without Broadcom's express prior
* written consent.
* 
:>
*/

#include <linux/nbuff.h>
#include <net/ip6_checksum.h>
#include <linux/ppp_defs.h>
#if IS_ENABLED(CONFIG_BCM_BPM)
#include <linux/gbpm.h>
#endif

#include <net/ip_tunnels.h>
#include <net/gre.h>

#include <linux/version.h>
#include <net/bcm_gso.h>
#include "sw_gso.h"

#if !IS_ENABLED(CONFIG_BCM_BPM)
static struct kmem_cache *pktBufferCache;
#endif

static sw_gso_enqueue_cb gso_thread_enq_cb = NULL;

void bcm_sw_gso_reg_enq_cb(sw_gso_enqueue_cb cb)
{
	gso_thread_enq_cb = cb;
}
EXPORT_SYMBOL(bcm_sw_gso_reg_enq_cb);

void bcm_sw_gso_dereg_enq_cb(void)
{
	gso_thread_enq_cb = NULL;
}
EXPORT_SYMBOL(bcm_sw_gso_dereg_enq_cb);

static inline int bcm_add_l2hdr(FkBuff_t *fkb, struct gso_hdrs *hdrs)
{
	unsigned int data_len = fkb->len;

	bcm_gso_add_l2hdr(&fkb->data, &data_len, hdrs);
	fkb->len = data_len;
	return 0;
}

static inline int bcm_add_l3hdr(FkBuff_t *fkb, struct gso_hdrs *hdrs)
{
	int ret = 0;
	unsigned int data_len = fkb->len;

	ret = bcm_gso_add_l3hdr(&fkb->data, &data_len, hdrs);
	fkb->len = data_len;
	return ret;
}

static inline int bcm_add_gre_hdr(FkBuff_t *fkb, struct gso_hdrs *hdrs)
{
	unsigned int data_len = fkb->len;

	bcm_gso_add_gre_hdr(&fkb->data, &data_len, hdrs);
	fkb->len = data_len;
	return 0;
}

static inline int bcm_add_outer_l2hdr(FkBuff_t *fkb, struct gso_hdrs *hdrs)
{
	unsigned int data_len = fkb->len;

	bcm_gso_add_outer_l2hdr(&fkb->data, &data_len, hdrs);
	fkb->len = data_len;
	return 0;
}

static inline int bcm_add_outer_l3hdr(FkBuff_t *fkb, struct gso_hdrs *hdrs)
{
	int ret = 0;
	unsigned int data_len = fkb->len;

	ret = bcm_gso_add_outer_l3hdr(&fkb->data, &data_len, hdrs);
	fkb->len = data_len;
	return ret;
}

static inline void bcm_add_tcphdr(FkBuff_t *fkb, struct gso_hdrs *hdrs)
{
	struct tcphdr *th;
	__wsum base = 0x00;
	unsigned int data_len = fkb->len;

	bcm_gso_add_tcphdr(&fkb->data, &data_len, hdrs);
	fkb->len = data_len;

	th = (struct tcphdr *)fkb->data;
	th->check = 0;
	if(hdrs->payload_csum) //apply payload csum that pre-calc in skb_copy_and_csum_bits()
		base = csum_partial(fkb->data, hdrs->l4hdrlen, hdrs->payload_csum);
	else
		base = csum_partial(fkb->data, fkb->len, 0);

    th->check = bcm_gso_l4_csum(hdrs, fkb->len, IPPROTO_TCP,base);
}

static void bcm_sw_gso_recycle_func(void *pNBuff, unsigned long context, uint32_t flags)
{
#if IS_ENABLED(CONFIG_BCM_BPM)
	gbpm_free_buf(PFKBUFF_TO_PDATA(PNBUFF_2_PBUF(pNBuff), BCM_PKT_HEADROOM));
#else
	kmem_cache_free(pktBufferCache, PNBUFF_2_PBUF(pNBuff));
#endif
}

static inline int bcm_sw_gso_free_pool(void **buffer_pool,unsigned int cur_pkt,unsigned int npkts)
{
	if(cur_pkt < npkts){
		int i;
		for(i=cur_pkt; i < npkts; i++)
		    bcm_sw_gso_recycle_func(PDATA_TO_PFKBUFF(buffer_pool[i], BCM_PKT_HEADROOM), 0, 0 );
	}
	return 0;
}

static inline int bcm_sw_alloc_pkt_buffers(int npkts, void **buffer_pool)
{
#if IS_ENABLED(CONFIG_BCM_BPM)
	/* allocate buffers from BPM */
	if( gbpm_alloc_mult_buf(npkts, buffer_pool) == GBPM_ERROR ){
		return -1;
	}
	return 0;
#else
	int i;
	void *buf;

	for(i=0; i < npkts; i++){
		buf = kmem_cache_alloc(pktBufferCache, GFP_ATOMIC);
		if(buf == NULL)
			break;
		buffer_pool[i] = (void *)PFKBUFF_TO_PDATA((void *)(buf), BCM_PKT_HEADROOM);
	}

	/* check if we allocated all the buffers requested */
	if( i && i != npkts){
		printk("%s %s: Not enough memory for packet buffer allocation\n", __FILE__, __FUNCTION__);
		npkts =i;
		for(i=0; i<npkts; i++)
			kmem_cache_free(pktBufferCache, PDATA_TO_PFKBUFF(buffer_pool[i], BCM_PKT_HEADROOM));

		return -1;
	}
	return 0;
#endif
}

static inline int bcm_sw_gso_tcp_segment(struct sk_buff *skb, struct gso_hdrs *hdrs)
{
	int npkts;
	FkBuff_t *fkb;
	void *pbuf;
	unsigned short bytesleft;
	unsigned short offset;
	unsigned short mss;
	unsigned short payloadlen;
	int cur_pkt=0;
	/*TODO do kmalloc if stack size is an issue*/
	void *buffer_pool[BCM_SW_GSO_MAX_SEGS];

	/* calculate the number of packets needed to transmit this skb */

	bytesleft = skb->len - hdrs->totlen;
	offset = hdrs->totlen;

	mss = skb_shinfo(skb)->gso_size;

	if(mss == 0)
	{
		mss = bytesleft;
		npkts =1;
	}
	else
		npkts = DIV_ROUND_UP(bytesleft, mss);

	if(npkts > BCM_SW_GSO_MAX_SEGS){
		printk(KERN_ERR "%s: npkts=%d greater than max segs(%d) \n",
				__func__, npkts, BCM_SW_GSO_MAX_SEGS);
		return -1;
	}

	if( bcm_sw_alloc_pkt_buffers(npkts, buffer_pool) != 0 )
		return -1;

	hdrs->tcp_segtype = BCM_SW_GSO_SEG_TYPE_FIRST;

	do {

		if(cur_pkt > npkts){
			/* we shoud never be here,if we are then most likely there is
			 * some thing wrong with mss, allocated packets not xmit yet,
			 * need to free them here
			 */
			printk(KERN_ERR "%s:error pktcount =%d > allocated buffers=%d mss=%d\n",
					__func__, cur_pkt, npkts, mss);

			bcm_sw_gso_free_pool(buffer_pool,cur_pkt,npkts);

			return -1;
		}
		/* initialize fkb */
		pbuf = buffer_pool[cur_pkt++];

		fkb = fkb_init(pbuf, BCM_PKT_HEADROOM, pbuf + hdrs->totlen, 0);
		fkb->recycle_hook = bcm_sw_gso_recycle_func;
		fkb->recycle_context = 0;
		fkb->queue = skb->queue; //queue overlap with mark ,so imply fkb->mark = skb->mark
		fkb->priority = skb->priority;

		payloadlen = min(mss, bytesleft);

		/* copy data from original skb to new packet */

		/* calc tcp payload csum from skb->frag page and copy tcp payload to fkb->data */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5,15,0))
 		hdrs->payload_csum = skb_copy_and_csum_bits(skb, offset, fkb->data, payloadlen,0x00);
#else
 		hdrs->payload_csum = skb_copy_and_csum_bits(skb, offset, fkb->data, payloadlen);
#endif
		fkb->len += payloadlen;

		offset +=payloadlen;
		bytesleft -=payloadlen;

		if(bytesleft == 0){
			if(likely(hdrs->tcp_segtype != BCM_SW_GSO_SEG_TYPE_FIRST))
				hdrs->tcp_segtype = BCM_SW_GSO_SEG_TYPE_LAST;
			else
				hdrs->tcp_segtype = BCM_SW_GSO_SEG_TYPE_SINGLE;
		}

		/* copy tcp hdr & update fields */
		bcm_add_tcphdr(fkb, hdrs);

		hdrs->tcpseq +=payloadlen;
		hdrs->tcp_segtype = BCM_SW_GSO_SEG_TYPE_MIDDLE;

		/* copy l3 hdr  */
		bcm_add_l3hdr(fkb, hdrs);

		/* copy l2 hdr  */
		bcm_add_l2hdr(fkb,hdrs);

		if(hdrs->grehdr)
		{
			/* add gre hdr  */
			bcm_add_gre_hdr(fkb, hdrs);
            
			/* copy outer l3 hdr  */
			if(hdrs->outer_ipv4hdr)
				bcm_add_outer_l3hdr(fkb, hdrs);

			/* copy outer l2 hdr  */
			if(hdrs->outer_ethhdr)
				bcm_add_outer_l2hdr(fkb,hdrs);
		}

		fkb->dirty_p = _to_dptr_from_kptr_(fkb->data + fkb->len);

	} while (bytesleft);

	if(cur_pkt>0)
	{
		/* Postpone xmit_fn() after all frangment done then send in a burst */
		int i;
		/* xmit packet */
		for(i=0; i < cur_pkt; i++)
			hdrs->xmit_fn(FKBUFF_2_PNBUFF(PDATA_TO_PFKBUFF(buffer_pool[i], BCM_PKT_HEADROOM)), hdrs->txdev);
	}

	if(cur_pkt < npkts){
		printk(KERN_ERR "%s:error pktcount =%d < allocated buffers=%d\n", __func__, cur_pkt, npkts);
		bcm_sw_gso_free_pool(buffer_pool,cur_pkt,npkts);
	}

	return 0;
}

static uint32_t bcm_sw_gso_gen_ipv6id(void)
{
	static atomic_t ipv6_fragmentation_id;
	uint32_t old, new;

	do {
		old = atomic_read(&ipv6_fragmentation_id);
		new = old + 1;
		if (!new)
			new = 1;
	} while (atomic_cmpxchg(&ipv6_fragmentation_id, old, new) != old);
	return new;
}

static int bcm_sw_gso_ip_fragment(struct sk_buff *skb, struct gso_hdrs *hdrs)
{
	int npkts;
	FkBuff_t *fkb;
	void *pbuf;
	unsigned short bytesleft;
	unsigned short offset;
	unsigned short mss;
	unsigned short payloadlen;
	int cur_pkt=0;
	/*TODO do kmalloc if stack size is an issue*/
	void *buffer_pool[BCM_SW_GSO_MAX_SEGS];


	/* calculate the number of packets needed to transmit this skb */
	bytesleft = skb->len - (hdrs->totlen - hdrs->l4hdrlen);
	offset = hdrs->totlen - hdrs->l4hdrlen;

	mss = skb_shinfo(skb)->gso_size;

	if(mss == 0){
		mss = bytesleft;
		npkts =1;
	}
	else{
		mss = mss & ~7; /* make it multiple of 8 */
		npkts = DIV_ROUND_UP(bytesleft, mss);
	}

	if(npkts > BCM_SW_GSO_MAX_SEGS){
		printk(KERN_ERR "%s: npkts=%d greater than max segs(%d) \n",
				__func__, npkts, BCM_SW_GSO_MAX_SEGS);
		return -1;
	}

	if( bcm_sw_alloc_pkt_buffers(npkts, buffer_pool) != 0 )
		return -1;

	hdrs->ip_fragoff = 0;
	hdrs->ip_lastfrag = 0;
	if(bytesleft - mss){
		/* perform IP fragmentation */
		hdrs->ip_fragneeded = 1;
	}

	if(hdrs->l3proto == BCM_L3_PROTO_IPV6){
		hdrs->ipv6_fragid = htonl(bcm_sw_gso_gen_ipv6id());
	}

	/* perform UDPchecksum */
	if( (hdrs->l4proto == IPPROTO_UDP) && (skb->ip_summed == CHECKSUM_PARTIAL))
	{
		struct udphdr *uh= hdrs->l4hdr;
		int l4hdr_offset = 0;
		uh->check = 0;

		l4hdr_offset = hdrs->l4hdr - ((hdrs->outer_ethhdr != NULL) ? hdrs->outer_ethhdr : hdrs->ethhdr );

		uh->check = bcm_gso_l4_csum(hdrs, skb->len-l4hdr_offset, IPPROTO_UDP,
				skb_checksum(skb, l4hdr_offset, skb->len-l4hdr_offset, 0));
		if(uh->check == 0x00)
			uh->check = CSUM_MANGLED_0;
	}

	do{

		if(cur_pkt > npkts){
			/* we shoud never be here,if we are then most likely there is
			 * some thing wrong with mss, allocated packets not xmit yet,
			 * need to free them here
			 */
			printk(KERN_ERR "%s:error pktcount =%d > allocated buffers=%d mss=%d\n",
					__func__, cur_pkt, npkts, mss);

			bcm_sw_gso_free_pool(buffer_pool,cur_pkt,npkts);

			return -1;
		}
		/* initialize fkb */
		pbuf = buffer_pool[cur_pkt++];

		fkb = fkb_init(pbuf, BCM_PKT_HEADROOM, pbuf + hdrs->totlen, 0);
		fkb->recycle_hook = bcm_sw_gso_recycle_func;
		fkb->recycle_context = 0;
		fkb->mark = skb->mark;
		fkb->priority = skb->priority;

		payloadlen = min(mss, bytesleft);

		/* copy data from original skb to new packet */

		/* copy tcp payload */
		skb_copy_bits(skb, offset, fkb->data, payloadlen);
		fkb->len += payloadlen;


		offset +=payloadlen;
		bytesleft -=payloadlen;

		if(bytesleft == 0)
			hdrs->ip_lastfrag = 1;

		/* copy l3 hdr  */
		bcm_add_l3hdr(fkb, hdrs);

		hdrs->ip_fragoff +=payloadlen;

		/* copy l2 hdr  */
		bcm_add_l2hdr(fkb,hdrs);

		if(hdrs->grehdr)
		{
			/* add gre hdr  */
			bcm_add_gre_hdr(fkb, hdrs);
            
			/* copy outer l3 hdr  */
			if(hdrs->outer_ipv4hdr)
			bcm_add_outer_l3hdr(fkb, hdrs);

			/* copy outer l2 hdr  */
			if(hdrs->outer_ethhdr)
			bcm_add_outer_l2hdr(fkb,hdrs);
		}

		fkb->dirty_p = _to_dptr_from_kptr_(fkb->data + fkb->len);

	}while (bytesleft);

	if(cur_pkt>0)
	{
		/* Postpone xmit_fn() after all frangment done then send in a burst */
		int i;
		/* xmit packet */
		for(i=0; i < cur_pkt; i++)
			hdrs->xmit_fn(FKBUFF_2_PNBUFF(PDATA_TO_PFKBUFF(buffer_pool[i], BCM_PKT_HEADROOM)), hdrs->txdev);
	}

	if(cur_pkt < npkts){
		printk(KERN_ERR "%s:error pktcount =%d < allocated buffers=%d\n", __func__, cur_pkt, npkts);
		bcm_sw_gso_free_pool(buffer_pool,cur_pkt,npkts);
	}

	return 0;
}



static inline int bcm_sw_gso(struct sk_buff *skb, struct net_device *txdev, HardStartXmitFuncP xmit_fn)
{
	struct gso_hdrs hdrs;
	int ret=-1;
	
	if(unlikely(skb_is_nonlinear(skb) && (skb->ip_summed != CHECKSUM_PARTIAL)))
	{
		/* forwarded pkts which are received as frags need to be linearized,
		 * just check for checksum not needed and frags
		 *
		 * Note:we don't expect a packet with GSO skb without
		 * CHECKSUM_PARTIAL even for UDP pkts
		 */
		if(__skb_linearize(skb)){
			printk(KERN_ERR "%s: skb_linearize failed \n",__func__);
			ret= -1;
			goto done;
		}
	}

	/*check if this skb needs GSO processing*/
	if(skb_is_gso(skb) || skb_shinfo(skb)->nr_frags)
	{
		/* pre release skb header state earlier (ex: release TCP windows)
	   	   to reduce queue delay impact TCP throughput
		*/
		skb_orphan_partial(skb);

		memset(&hdrs,0, sizeof(struct gso_hdrs));
		/* extract l2 & l3 headers and l4 proto */
		if( bcm_parse_gso_hdrs(skb, &hdrs) < 0){
			ret = -1;
			goto done;
		};

		hdrs.txdev = txdev;
		hdrs.xmit_fn =xmit_fn;

		/*perform GSO and transmit packets*/

		switch(hdrs.l4proto) {
			case IPPROTO_TCP:
				ret = bcm_sw_gso_tcp_segment(skb, &hdrs);
				break;

			case IPPROTO_UDP:
				ret = bcm_sw_gso_ip_fragment(skb, &hdrs);
				break;

			default:
				printk(KERN_ERR "%s:SW GSO not supported for protocol=%d \n",__func__, hdrs.l4proto);
				ret= -1;
				goto done;
		}
	} else if((skb->ip_summed == CHECKSUM_PARTIAL)){

		/* skb may shared with someone.
		   unshare for update csum on private copy skb->data only.
		*/
		skb = skb_unshare(skb, GFP_ATOMIC);
		if (!skb)
		{
			ret = -1;
			return ret;
		}

		/*only csum is needed */
		memset(&hdrs,0, sizeof(struct gso_hdrs));
		if( bcm_parse_gso_hdrs(skb, &hdrs) < 0){
			ret = -1;
			goto done;
		};

		switch(hdrs.l4proto) {
			case IPPROTO_TCP:
			{
			 	struct tcphdr *th= hdrs.l4hdr;
			 	int l4hdr_offset = 0;

				th->check = 0;
				/*TODO calulate l4 len based on l3 len */

				l4hdr_offset = hdrs.l4hdr - ((hdrs.outer_ethhdr != NULL) ? hdrs.outer_ethhdr : hdrs.ethhdr );
				th->check = bcm_gso_l4_csum(&hdrs, ((skb->len)-l4hdr_offset),
					IPPROTO_TCP, csum_partial(hdrs.l4hdr, ((skb->len)-l4hdr_offset), 0));
				break;
			}

			case IPPROTO_UDP:
			{
			 	struct udphdr *uh= hdrs.l4hdr;
				int l4hdr_offset = 0;

				uh->check = 0;
				/*TODO calulate l4 len based on l3 len */
				l4hdr_offset = hdrs.l4hdr - ((hdrs.outer_ethhdr != NULL) ? hdrs.outer_ethhdr : hdrs.ethhdr );
				uh->check = bcm_gso_l4_csum(&hdrs, ((skb->len)-l4hdr_offset),
				        IPPROTO_UDP, csum_partial(hdrs.l4hdr, ((skb->len)-l4hdr_offset), 0));
				if(uh->check == 0x00)
					uh->check = CSUM_MANGLED_0;
				break;
			}

			default:
				printk(KERN_ERR "%s:SW GSO not supported for protocol=%d \n",__func__, hdrs.l4proto);
				ret= -1;
				goto done;
		}

		skb->ip_summed = CHECKSUM_NONE;
		xmit_fn(SKBUFF_2_PNBUFF(skb), txdev);
		return 0;
	}else{
		/* transmit skb as is */
		xmit_fn(SKBUFF_2_PNBUFF(skb), txdev);
		return 0;
	}
done:
	/*free the original skb & return */
	dev_kfree_skb_any(skb);
	return ret;
}

int bcm_sw_gso_xmit(struct sk_buff *skb, struct net_device *txdev, HardStartXmitFuncP xmit_fn)
{
	int ret = NETDEV_TX_BUSY;

	if(gso_thread_enq_cb)
		ret = (gso_thread_enq_cb)( skb
	                            ,txdev
	                            ,NULL
	                            ,xmit_fn);

	if(ret != NETDEV_TX_OK)
        ret = bcm_sw_gso( skb
                         ,txdev
                         ,xmit_fn);

	return ret;
}
EXPORT_SYMBOL(bcm_sw_gso_xmit);

int bcm_sw_gso_skb_prehandle(  struct sk_buff       *skb_p
                              ,struct net_device    *dev
                              ,gso_skb_prehandle_cb skb_pre_handle
                              ,HardStartXmitFuncP   fkb_post_handle)
{
    int handle_ret = NETDEV_TX_BUSY;

    if (skb_p)
    {
        if (!skb_pre_handle)
        { /* No skb pre handle, call bcm_sw_gso directly */
            bcm_sw_gso(skb_p, dev, fkb_post_handle);
            handle_ret = NETDEV_TX_OK;
        }else
        {
            handle_ret = (skb_pre_handle)(skb_p,dev);
        }

        if(handle_ret != NETDEV_TX_OK)
                dev_kfree_skb_any(skb_p);
    }

    return handle_ret;
}
EXPORT_SYMBOL(bcm_sw_gso_skb_prehandle);

int bcm_gso_enqueue(struct sk_buff *skb, struct net_device *dev,HardStartXmitFuncP skb_pre_handle_fn)
{
	int ret = NETDEV_TX_BUSY;

	if(gso_thread_enq_cb)
		ret = (gso_thread_enq_cb)( skb
					,dev
					,skb_pre_handle_fn
					,NULL);

	if(ret != NETDEV_TX_OK)
		ret = bcm_sw_gso_skb_prehandle( skb
						,dev
						,skb_pre_handle_fn
						,NULL);

	return ret;
}
EXPORT_SYMBOL(bcm_gso_enqueue);

int bcm_sw_gso_xmit_classic(struct sk_buff *skb, struct net_device *txdev, HardStartXmitFuncP xmit_fn)
{
	return bcm_sw_gso(skb, txdev, xmit_fn);
}
EXPORT_SYMBOL(bcm_sw_gso_xmit_classic);

static int __init bcm_sw_gso_init(void)
{
#if !IS_ENABLED(CONFIG_BCM_BPM)
    /* create a slab cache for GSO buffers */
    pktBufferCache = kmem_cache_create("pktBufferCache",
                                       BCM_PKTBUF_SIZE,
                                       0, /* align */
                                       SLAB_HWCACHE_ALIGN, /* flags */
                                       NULL); /* ctor */
    if(pktBufferCache == NULL)
    {
        printk("%s %s: Failed to create packet buffer cache\n", __FILE__, __FUNCTION__);
		BUG();
    }
#endif

	return 0;
}

subsys_initcall(bcm_sw_gso_init);


__be16 bcm_sw_gso_skb_network_protocol(struct sk_buff *skb, int offset, __be16 type)
{
	__be16 protocol_type = type;

	/* BRCM SW supports PPP session */
	if (type == htons(ETH_P_PPP_SES)) {
		if (offset)
			protocol_type = *((uint16_t *)(skb->data + offset + PPP_HDRLEN + 2));
		else /* Untag frame didn't set mac_len in skb */
			protocol_type = *((uint16_t *)(skb->data + ETH_HLEN + PPP_HDRLEN + 2));
		if (protocol_type == htons(PPP_IP))
			protocol_type = htons(ETH_P_IP);
		else if (protocol_type == htons(PPP_IPV6))
			protocol_type = htons(ETH_P_IPV6);
	}

	return protocol_type;
}

