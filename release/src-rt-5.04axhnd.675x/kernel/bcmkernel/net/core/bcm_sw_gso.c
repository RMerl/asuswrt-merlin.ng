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
#include <linux/gbpm.h>
#include <net/ip6_checksum.h>

#if !IS_ENABLED(CONFIG_BCM_BPM)
static struct kmem_cache *pktBufferCache;
#endif

#define BCM_SW_GSO_MAX_HEADERS  16
#define BCM_SW_GSO_MAX_SEGS  64
#define BCM_L3_PROTO_IPV4 	1
#define BCM_L3_PROTO_IPV6 	2

enum {
	BCM_SW_GSO_SEG_TYPE_SINGLE=0,
	BCM_SW_GSO_SEG_TYPE_FIRST,
	BCM_SW_GSO_SEG_TYPE_MIDDLE,
	BCM_SW_GSO_SEG_TYPE_LAST,
	BCM_SW_GSO_SEG_TYPE_MAX
};

struct gso_hdrs{
	void *ethhdr; /*including VLAN's */
	void *ppphdr;
	void *ipv4hdr;
	void *ipv6hdr;
	void *l4hdr;
	union {
		struct {
			uint8_t ethhdrlen;/*including VLAN's */
			uint8_t ppphdrlen;
			uint8_t ipv4hdrlen;
			uint8_t ipv6hdrlen;
			uint8_t l4hdrlen;
			uint8_t totlen;
			uint8_t l3proto;
			uint8_t l4proto;
		};
		uint64_t lenproto;
	};
	void *txdev;
	HardStartXmitFuncP xmit_fn;
	uint32_t ipv6_flowlbl;
	uint32_t tcpseq;
	uint32_t ipv6_fragid;
	uint16_t ipv4_id;
 	uint16_t ip_fragoff;
	uint8_t ip_fragneeded;
	uint8_t ip_lastfrag;
	uint8_t tcp_segtype;
};

static inline void bcm_push_hdr(FkBuff_t *fkb, void *src, unsigned int len)
{
	fkb->data -= len;
	fkb->len  +=len;
	memcpy(fkb->data, src, len);
}

static inline int bcm_add_l2hdr(FkBuff_t *fkb, struct gso_hdrs *hdrs)
{
	if(hdrs->ppphdrlen)
	{
		bcm_push_hdr(fkb, hdrs->ppphdr, hdrs->ppphdrlen);
		/*TODO adjust ppphdr len ip +2 */
	}
	bcm_push_hdr(fkb, hdrs->ethhdr, hdrs->ethhdrlen);
	return 0;
}

static inline int bcm_add_l3hdr(FkBuff_t *fkb, struct gso_hdrs *hdrs)
{
	if(hdrs->l3proto == BCM_L3_PROTO_IPV4){
		struct iphdr *ipv4;
		bcm_push_hdr(fkb, hdrs->ipv4hdr, hdrs->ipv4hdrlen);
		ipv4 = (struct iphdr *)fkb->data;
		/* update len   */
		 ipv4->tot_len = htons(fkb->len);

		 if(hdrs->ip_fragneeded){
			 /* update offset */
			 ipv4->frag_off = htons((hdrs->ip_fragoff >> 3));
			 /* update flags */
			 if( !hdrs->ip_lastfrag)
				 ipv4->frag_off |= htons(IP_MF);
		 }

		/* update IPID  */

		/* update csum  */
		ipv4->check = 0;
		ipv4->check = ip_fast_csum((unsigned char *)ipv4, ipv4->ihl);
	}
	else if(hdrs->l3proto == BCM_L3_PROTO_IPV6){
		struct ipv6hdr *ipv6;

		if(hdrs->ip_fragneeded){
			struct frag_hdr fh;

			fh.nexthdr = ((struct ipv6hdr *)hdrs->ipv6hdr)->nexthdr;
			fh.reserved = 0;
			fh.identification = hdrs->ipv6_fragid;
			fh.frag_off = htons(hdrs->ip_fragoff);
			bcm_push_hdr(fkb, &fh, sizeof(struct frag_hdr));
		}

		bcm_push_hdr(fkb, hdrs->ipv6hdr, hdrs->ipv6hdrlen);
		ipv6 = (struct ipv6hdr *)fkb->data;

		/* update len   */
		ipv6->payload_len = htons( fkb->len - hdrs->ipv6hdrlen);

		if(hdrs->ip_fragneeded){
			ipv6->nexthdr =NEXTHDR_FRAGMENT;
		}

		/* update IPID  */
	    /* update flags */
	}
	else{
		printk(KERN_ERR "%s:Unsuppoerted L3 protocol %d\n", __func__, hdrs->l3proto);
		return -1;
	}
	return 0;
}

static inline __sum16 bcm_sw_gso_l4_csum(struct gso_hdrs *hdrs, uint32_t len, uint16_t proto, __wsum base)
{
	if(hdrs->l3proto == BCM_L3_PROTO_IPV4 ){
		struct iphdr *ipv4= hdrs->ipv4hdr;
		return csum_tcpudp_magic(ipv4->saddr, ipv4->daddr, len, proto, base);
	} else {
		struct ipv6hdr *ipv6 = hdrs->ipv6hdr;
		return csum_ipv6_magic(&ipv6->saddr, &ipv6->daddr, len, proto, base);
	}
}

static inline void bcm_add_tcphdr(FkBuff_t *fkb, struct gso_hdrs *hdrs)
{
	struct tcphdr *th;
	bcm_push_hdr(fkb, hdrs->l4hdr, hdrs->l4hdrlen);
	/* update seq */
	th = (struct tcphdr *)fkb->data;
	th->seq = htonl(hdrs->tcpseq);

	if(hdrs->tcp_segtype != BCM_SW_GSO_SEG_TYPE_SINGLE){

		/* clear cwr in all packets except first */
		if(hdrs->tcp_segtype != BCM_SW_GSO_SEG_TYPE_FIRST)
			th->cwr = 0;

		/* clear fin, psh in all packets except last */
		if((hdrs->tcp_segtype != BCM_SW_GSO_SEG_TYPE_LAST))
			th->fin = th->psh = 0;
	}

	th->check = 0;
	th->check = bcm_sw_gso_l4_csum(hdrs, fkb->len, IPPROTO_TCP, csum_partial(fkb->data, fkb->len, 0));
}

void bcm_sw_gso_recycle_func(void *pNBuff, unsigned long context, uint32_t flags)
{
#if IS_ENABLED(CONFIG_BCM_BPM)
	gbpm_free_buf(PFKBUFF_TO_PDATA(PNBUFF_2_PBUF(pNBuff), BCM_PKT_HEADROOM));
#else
	kmem_cache_free(pktBufferCache, PNBUFF_2_PBUF(pNBuff));
#endif
}
EXPORT_SYMBOL(bcm_sw_gso_recycle_func);

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
			 * some thing wrong with mss, allocated packets are already xmitted,
			 * no need to free them here
			 */ 
			printk(KERN_ERR "%s:error pktcount =%d > allocated buffers=%d mss=%d\n",
					__func__, cur_pkt, npkts, mss);
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


		fkb->dirty_p = _to_dptr_from_kptr_(fkb->data + fkb->len);
		/* xmit packet */
		hdrs->xmit_fn(FKBUFF_2_PNBUFF(fkb), hdrs->txdev);

	} while (bytesleft);

	if(cur_pkt < npkts){
		int i;
		printk(KERN_ERR "%s:error pktcount =%d < allocated buffers=%d\n", __func__, cur_pkt, npkts);
		for(i=cur_pkt; i < npkts; i++)
			bcm_sw_gso_recycle_func(PDATA_TO_PFKBUFF(buffer_pool[i], BCM_PKT_HEADROOM), 0, 0 );
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
		uh->check = 0;
		int csum_offset = hdrs->l4hdr - hdrs->ethhdr;

		uh->check = bcm_sw_gso_l4_csum(hdrs, skb->len-csum_offset, IPPROTO_UDP,
				skb_checksum(skb, csum_offset, skb->len-csum_offset, 0));
	}

	do{

		if(cur_pkt > npkts){
			/* we shoud never be here,if we are then most likely there is
			 * some thing wrong with mss, allocated packets are already xmitted,
			 * no need to free them here
			 */ 
			printk(KERN_ERR "%s:error pktcount =%d > allocated buffers=%d mss=%d\n",
					__func__, cur_pkt, npkts, mss);
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

		fkb->dirty_p = _to_dptr_from_kptr_(fkb->data + fkb->len);
		/* xmit packet */
		hdrs->xmit_fn(FKBUFF_2_PNBUFF(fkb), hdrs->txdev);

	}while (bytesleft);

	if(cur_pkt < npkts){
		int i;
		printk(KERN_ERR "%s:error pktcount =%d < allocated buffers=%d\n", __func__, cur_pkt, npkts);
		for(i=cur_pkt; i< npkts; i++)
			bcm_sw_gso_recycle_func(PDATA_TO_PFKBUFF(buffer_pool[i], BCM_PKT_HEADROOM), 0, 0 );
	}

	return 0;
}


static inline int bcm_parse_gso_hdrs(struct sk_buff *skb, struct gso_hdrs *hdrs)
{
	struct iphdr *ipv4;
	struct ipv6hdr *ipv6;
	struct tcphdr *tcp_hdr;
    int header = -1;
	void *data = skb->data;
	unsigned int skbheadlen = skb_headlen(skb);
	/*assumes first header is ETH always */
	uint16_t proto = ETH_P_802_3;

    while(1)
    {
        header++;

		if(hdrs->totlen > skbheadlen){
			printk(KERN_ERR "%s: headers not present in linear data hdrslen=%u skbheadlen=%d \n",
				 __func__, hdrs->totlen, skbheadlen);
			return -1;
		}

        if (header > BCM_SW_GSO_MAX_HEADERS){
            printk(KERN_ERR "%s:Too many headers <%d>\n", __func__, header);
			return -1;
        }

        switch(proto)
        {
            case ETH_P_802_3:  /* first encap: XYZoE */

                if(header != 0)
                {
                    return -1;
                }
				hdrs->ethhdr = data;

				proto = ntohs(((struct ethhdr *)data)->h_proto);
				hdrs->ethhdrlen = ETH_HLEN;
				hdrs->totlen += ETH_HLEN;
				data += ETH_HLEN;
                break;

            case ETH_P_8021Q: 
            case ETH_P_8021AD: 

				proto = ntohs(*((uint16_t *)(data+2)));
				hdrs->ethhdrlen += 4;
				hdrs->totlen += 4;
				data += 4;
                break;

            case ETH_P_IP:
				 
				ipv4 = data;

				hdrs->l3proto = BCM_L3_PROTO_IPV4;
				proto = ipv4->protocol;

                if ((proto == IPPROTO_TCP) || (proto == IPPROTO_UDP)){

					hdrs->ipv4hdr = data;
					hdrs->ipv4_id = ntohs(ipv4->id);
					hdrs->ipv4hdrlen = ipv4->ihl<<2;
					hdrs->totlen += ipv4->ihl<<2;
					data += ipv4->ihl<<2;

                } else {
                    printk(KERN_ERR "%s Unsupported L4 type=%u \n", __func__, proto);
                    return -1;
                }
				break;

            case ETH_P_IPV6:

				ipv6 = data;

				hdrs->l3proto = BCM_L3_PROTO_IPV6;
				proto = ipv6->nexthdr;

                if ((proto == IPPROTO_TCP) || (proto == IPPROTO_UDP)){

					hdrs->ipv6hdr = data;
					hdrs->ipv6_flowlbl = ntohl(ip6_flowlabel(ipv6));
					hdrs->ipv6hdrlen = sizeof(struct ipv6hdr);
					hdrs->totlen += sizeof(struct ipv6hdr);
					data += sizeof(struct ipv6hdr);

                } else {
                    printk(KERN_ERR "%s Unsupported L4 type=%u \n", __func__, proto);
                    return -1;
                }
				break;

			case IPPROTO_TCP :
				tcp_hdr = data;

				hdrs->l4hdr = data;
				hdrs->l4proto = IPPROTO_TCP;
				hdrs->l4hdrlen = tcp_hdr->doff <<2;
				hdrs->totlen += tcp_hdr->doff <<2;
				hdrs->tcpseq = ntohl(tcp_hdr->seq);
				return  0 ;

			case IPPROTO_UDP :
				hdrs->l4proto = IPPROTO_UDP;
				hdrs->l4hdr = data;
				hdrs->l4hdrlen = sizeof(struct udphdr) ;
				hdrs->totlen += sizeof(struct udphdr);
				return  0 ;

			/*TODO add PPP */

            default :
                printk(KERN_ERR "%s:UNSUPPORTED protocol %u ", __func__, proto);
                return -1;
        } /* switch (headerType) */
    }
	return -1;
}


static inline int bcm_sw_gso(struct sk_buff *skb, struct net_device *txdev, HardStartXmitFuncP xmit_fn)
{
	struct gso_hdrs hdrs;
	int ret=-1;

	/*check if this skb needs GSO processing*/
	if(skb_is_gso(skb) || skb_shinfo(skb)->nr_frags)
	{

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
				th->check = 0;
				/*TODO calulate l4 len based on l3 len */
				th->check = bcm_sw_gso_l4_csum(&hdrs, skb->len-(hdrs.l4hdr - hdrs.ethhdr),
						IPPROTO_TCP, csum_partial(hdrs.l4hdr, skb->len-(hdrs.l4hdr - hdrs.ethhdr), 0));
				break;
			}

			case IPPROTO_UDP:
			{
			 	struct udphdr *uh= hdrs.l4hdr;
				uh->check = 0;
				/*TODO calulate l4 len based on l3 len */
				uh->check = bcm_sw_gso_l4_csum(&hdrs, skb->len-(hdrs.l4hdr - hdrs.ethhdr),
						IPPROTO_UDP, csum_partial(hdrs.l4hdr, skb->len-(hdrs.l4hdr - hdrs.ethhdr), 0));
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
	return bcm_sw_gso(skb, txdev, xmit_fn);
}
EXPORT_SYMBOL(bcm_sw_gso_xmit);


static int __init bcm_sw_gso_init(void)
{
#if !IS_ENABLED(CONFIG_BCM_BPM)
    /* create a slab cache for GSO buffers */
#if defined(CONFIG_BCM96855) && defined(CONFIG_BCM_JUMBO_FRAME)
	pktBufferCache = kmem_cache_create("pktBufferCache",
                                       bcm_pktbuf_size(),
                                       0, /* align */
                                       SLAB_HWCACHE_ALIGN, /* flags */
                                       NULL); /* ctor */
#else
	pktBufferCache = kmem_cache_create("pktBufferCache",
                                       BCM_PKTBUF_SIZE,
                                       0, /* align */
                                       SLAB_HWCACHE_ALIGN, /* flags */
                                       NULL); /* ctor */
#endif
	if(pktBufferCache == NULL)
    {
        printk("%s %s: Failed to create packet buffer cache\n", __FILE__, __FUNCTION__);
		BUG();
    }
#endif

	return 0;
}

subsys_initcall(bcm_sw_gso_init);
