/*
* <:copyright-BRCM:2012:DUAL/GPL:standard
* 
*    Copyright (c) 2012 Broadcom 
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

#include <linux/module.h>
#include <linux/types.h>
#include <linux/kernel.h>

#include <linux/in.h>
#include <linux/inet.h>
#include <linux/tcp.h>
#include <linux/udp.h>

#include <net/protocol.h>
#include <net/dst.h>
#include <net/sock.h>
#include <net/checksum.h>
#include <net/ip6_checksum.h>
#include <net/xfrm.h>

#include <linux/skbuff.h>
#include <linux/bcm_skbuff.h>
#include <linux/nbuff.h>

#include <linux/bcm_skb_defines.h>
#if defined(CONFIG_BCM_BPM_BUF_TRACKING)
#include <linux/gbpm.h>
#endif

/* Returns size of struct sk_buff */
size_t skb_size(void)
{
	return sizeof(struct sk_buff);
}
EXPORT_SYMBOL(skb_size);

size_t skb_aligned_size(void)
{
	/*TODO this is broken. always assumes 16 byte cacheline, used CACHLINE macro instead */

	return ((sizeof(struct sk_buff) + 0x0f) & ~0x0f);
}
EXPORT_SYMBOL(skb_aligned_size);

int skb_layout_test(int head_offset, int tail_offset, int end_offset)
{
#define SKBOFFSETOF(member)	((size_t)&((struct sk_buff*)0)->member)
	if ((SKBOFFSETOF(head) == head_offset) &&
	    (SKBOFFSETOF(tail) == tail_offset) &&
	    (SKBOFFSETOF(end) == end_offset))
		return 1;
	return 0;
}
EXPORT_SYMBOL(skb_layout_test);

int skb_avail_headroom(const struct sk_buff *skb)
{
#if defined(CONFIG_BCM_USBNET_ACCELERATION)
	if (skb->bcm_ext.clone_fc_head) {
		/* In this case  it's unlikely but possible for
		 * the value of skb->data - skb->clone_fc_head to be negative
		 * the caller should check for negative value
		 */
		return skb->data - skb->bcm_ext.clone_fc_head;
	} else
#endif
		return skb_headroom(skb);
}
EXPORT_SYMBOL(skb_avail_headroom);

void skb_bpm_tainted(struct sk_buff *skb)
{
	/* recycle_flags &= ~SKB_BPM_PRISTINE, dirty_p = NULL */
	SKB_BPM_TAINTED(skb);
}
EXPORT_SYMBOL(skb_bpm_tainted);

void skb_data_pristine(struct sk_buff *skb)
{
	SKB_DATA_PRISTINE(skb); /* skb->dirty = skb->head */
}
EXPORT_SYMBOL(skb_data_pristine);

static inline void _skb_headerreset(struct sk_buff *skb)
{
	memset(skb, 0, offsetof(struct sk_buff, truesize));
}
void skb_headerreset(struct sk_buff *skb)
{
	_skb_headerreset(skb);
	refcount_set(&skb->users, 1);
}
EXPORT_SYMBOL(skb_headerreset);

static inline void _skb_shinforeset(struct skb_shared_info *skb_shinfo)
{
	memset(skb_shinfo, 0, offsetof(struct skb_shared_info, dataref));
}
void skb_shinforeset(struct skb_shared_info *skb_shinfo)
{
	_skb_shinforeset(skb_shinfo);
	atomic_set(&(skb_shinfo->dataref), 1);
}
EXPORT_SYMBOL(skb_shinforeset);

static inline void bcm_skb_set_end_pointer(struct sk_buff *skb, const int end_offset)
{
#ifdef NET_SKBUFF_DATA_USES_OFFSET
	skb->end = end_offset; 
#else
	skb->end = skb->head + end_offset;
#endif
}

/**
 *
 *	skb_headerinit  -   initialize a socket buffer header
 *	@headroom: reserved headroom size
 *	@datalen: data buffer size, data buffer is allocated by caller
 *	@skb: skb allocated by caller
 *	@data: data buffer allocated by caller
 *	@recycle_hook: callback function to free data buffer and skb
 *	@recycle_context: context value passed to recycle_hook, param1
 *	@blog_p: pass a blog to a skb for logging
 *
 *	Initializes the socket buffer and assigns the data buffer to it.
 *	Both the sk_buff and the pointed data buffer are pre-allocated.
 *
 */
void skb_headerinit(unsigned int headroom, unsigned int datalen,
		    struct sk_buff *skb, unsigned char *data,
		    RecycleFuncP recycle_hook, unsigned long recycle_context,
		    struct blog_t *blog_p)	/* defined(CONFIG_BLOG) */
{
	_skb_headerreset(skb); /* memset to truesize */

	refcount_set(&skb->users, 1);
	skb->head = data - headroom;
	skb->data = data;
	skb_set_tail_pointer(skb, datalen);
	skb->len = datalen;


	WARN_ONCE(((uintptr_t)skb->head & (SMP_CACHE_BYTES -1)),
				"skb->head pointer not cache aligned, skb=%px skb->head=%px ",
				skb, skb->head);

	/*TODO fix this function to receive len & tailroom separately 
	 * and set end = SKB_DATA_ALIGN(headroom + len + tailroom)
	 * and with the assumption skb->head is cachealigned
	 *
	 * with current implementation if skb->data is not cache aligned 
	 * there is a possibilty of buffer overflow if pktlen is equal 
	 * to BCM_MAX_PKTLEN, as SKB_DATA_ALIGN will increase headroom
	 */
#if 0 /* Mask the error print until JIRA SWBCACPE-40964 is understood */
	WARN_ONCE(((uintptr_t)skb->data & (SMP_CACHE_BYTES -1)),
				"skb->data pointer not cache aligned, skb=%px skb->data=%px ",
				skb, skb->data);
#endif
	bcm_skb_set_end_pointer(skb, SKB_DATA_ALIGN(headroom + datalen));
	skb->truesize = SKB_TRUESIZE(skb_end_offset(skb));
	
#if defined(CONFIG_BCM_BPM_BUF_TRACKING)
	GBPM_INC_REF(data);
	GBPM_TRACK_SKB(skb, GBPM_DRV_KERN, GBPM_VAL_INIT, 0);
#endif
#if defined(CONFIG_BLOG)
	skb->blog_p = blog_p;
	if (blog_p)
		blog_p->skb_p = skb;
	// skb->tunl = NULL; memset in _skb_headerreset
#endif
#if (defined(CONFIG_BCM_VLAN) || defined(CONFIG_BCM_VLAN_MODULE))
	// skb->vlan_count = 0; memset in _skb_headerreset
#endif
#if IS_ENABLED(CONFIG_BCM_MAP)
	// skb->map_id = 0; memset in _skb_headerreset
	// skb->map_offset = 0; memset in skb_headerreset
#endif
	skb->fc_ctxt = 0;
	skb->recycle_hook = recycle_hook;
	skb->recycle_context = recycle_context;
	skb->recycle_flags = SKB_RECYCLE | SKB_DATA_RECYCLE;

	_skb_shinforeset(skb_shinfo(skb));
	atomic_set(&(skb_shinfo(skb)->dataref), 1);
}
EXPORT_SYMBOL(skb_headerinit);

struct sk_buff *skb_header_alloc(void)
{
	return kmem_cache_alloc(skbuff_head_cache, GFP_ATOMIC);
}
EXPORT_SYMBOL(skb_header_alloc);

void skb_header_free(struct sk_buff *skb)
{
	/* If the skb came from a preallocated pool, pass it to recycler hook */
	if (skb->recycle_hook && (skb->recycle_flags & SKB_RECYCLE)) {
		/* Detach data buffer */
		/* Caller should take care of databuf */
		skb->head = NULL;
		skb->data = NULL;
		skb->recycle_flags &= ~ SKB_DATA_RECYCLE;
		(*skb->recycle_hook)(skb, skb->recycle_context, SKB_RECYCLE);
	} else
		kmem_cache_free(skbuff_head_cache, skb);
}
EXPORT_SYMBOL(skb_header_free);

/*
 * Translate a fkb to a skb, by allocating a skb from the skbuff_head_cache.
 * PS. skb->dev is not set during initialization.
 *
 * Caller verifies whether the fkb is unshared:
 *  if fkb_p==NULL||IS_FKB_CLONE(fkb_p)||fkb_p->users>1 and return NULL skb.
 */
struct sk_buff *skb_xlate_dp(struct fkbuff * fkb_p, uint8_t *dirty_p)
{
	struct sk_buff * skb_p;


	/* Optimization: use preallocated pool of skb with SKB_POOL_RECYCLE flag */
#if defined(CONFIG_BCM947189)
	/* For low memory system like 47189 we should not use GFP_ATOMIC as memory
	 * may not be readily available when the offered rate is very high
	 */
	skb_p = kmem_cache_alloc(skbuff_head_cache, GFP_KERNEL);
#else
	skb_p = kmem_cache_alloc(skbuff_head_cache, GFP_ATOMIC);
#endif
	if (!skb_p)
		return skb_p;
	skb_p->fclone = SKB_FCLONE_UNAVAILABLE;

	memset(skb_p, 0, offsetof(struct sk_buff, truesize));


	skb_p->data = fkb_p->data;
	skb_p->head = (unsigned char *)(fkb_p + 1 );
	skb_set_tail_pointer(skb_p, fkb_p->len);

	/* here assumption is head is cachealigned and  data has not expanded
	 * at tail yet 
	 */
	WARN_ONCE(((uintptr_t)skb_p->head & (SMP_CACHE_BYTES -1)),
				"skb->head pointer not cache aligned, skb=%px skb->head=%px ",
				skb_p, skb_p->head);
	bcm_skb_set_end_pointer(skb_p, SKB_DATA_ALIGN((skb_p->data -skb_p->head) +
				fkb_p->len + BCM_SKB_TAILROOM));
	skb_p->truesize = SKB_TRUESIZE(skb_end_offset(skb_p));

#if defined (CONFIG_BCM_BPM_BUF_TRACKING)
	GBPM_INC_REF(skb_p->data);
	GBPM_TRACK_SKB(skb_p, GBPM_DRV_KERN, GBPM_VAL_XLATE, 0);
#endif

#define F2S(x) skb_p->x = fkb_p->x
	F2S(len);
	F2S(queue);
	F2S(priority);

#if defined(CONFIG_BLOG)
	if (_IS_BPTR_(fkb_p->blog_p)) {	/* should not happen */
		F2S(blog_p);
		fkb_p->blog_p->skb_p = skb_p;
	}
#endif
	F2S(recycle_hook);
	F2S(recycle_context);
	skb_p->recycle_flags = SKB_DATA_RECYCLE;

	/* redundant: fkb_p must not be used henceforth */
	fkb_dec_ref(fkb_p);

	refcount_set(&skb_p->users, 1);

#if defined(CONFIG_BCM_CSO)
	if (fkb_p->rx_csum_verified)
		skb_p->ip_summed = CHECKSUM_UNNECESSARY;
#endif
	/*
	 * When fkb is xlated to skb, preserve the dirty_p info.
	 * This allows receiving driver to shorten its cache flush and also
	 * can shorten the cache flush when the buffer is recycled.  Improves
	 * wlan perf by 10%.
	 */
	skb_shinfo(skb_p)->dirty_p = dirty_p;
	/* any change to skb_shinfo initialization in __alloc_skb must be ported
	 * to this block. */
	skb_shinfo(skb_p)->meta_len = 0;
	skb_shinfo(skb_p)->nr_frags = 0;
	skb_shinfo(skb_p)->tx_flags = 0;
	skb_shinfo(skb_p)->gso_size = 0;
	skb_shinfo(skb_p)->gso_segs = 0;
	skb_shinfo(skb_p)->frag_list = NULL;
	memset(&(skb_shinfo(skb_p)->hwtstamps), 0,
	       sizeof(skb_shinfo(skb_p)->hwtstamps));
	skb_shinfo(skb_p)->gso_type = 0;
	skb_shinfo(skb_p)->tskey = 0;
	atomic_set(&(skb_shinfo(skb_p)->dataref), 1);
	skb_shinfo(skb_p)->destructor_arg = NULL;

	return skb_p;
#undef F2S
}
EXPORT_SYMBOL(skb_xlate_dp);

 /* skb_xlate is deprecated.  New code should call skb_xlate_dp directly.
 */
struct sk_buff * skb_xlate(struct fkbuff * fkb_p)
{
	return (skb_xlate_dp(fkb_p, NULL));
}
EXPORT_SYMBOL(skb_xlate);

#define NETDEV_XMIT(_dev, _buff)	\
		_dev->netdev_ops->ndo_start_xmit(_buff, _dev)

/*
 * This fucntion fragments the skb into multiple skbs and xmits them
 * this fucntion is a substitue for ip_fragment when Ip stack is skipped
 * for packet acceleartion(fcache,CMF)
 *
 * Currently only IPv4 is supported
 */
void skb_frag_xmit4(struct sk_buff *origskb, struct net_device *txdev,
		    uint32_t is_pppoe, uint32_t minMtu, void *ipp)
{

#define DEBUG_SKBFRAG(args)

#define IP_DF		0x4000		/* Flag: "Don't Fragment"	*/
#define IP_MF		0x2000		/* Flag: "More Fragments"	*/
#define IP_OFFSET	0x1FFF		/* "Fragment Offset" part	*/

	struct iphdr *iph;
	int datapos, offset;
	unsigned int max_dlen, hlen, hdrslen, left, len;
	uint16_t not_last_frag;
	struct sk_buff *fraglisthead;
	struct sk_buff *fraglisttail;
	struct sk_buff *skb2;

	DEBUG_SKBFRAG(("skb_frag_xmit4:enter origskb=%px,netdev=%px,is_pppoe=%d,\
				minMtu=%d ipp=%px\n",origskb, txdev, is_pppoe,
				minMtu, ipp));

	if (likely(origskb->len <= minMtu)) {
		/* xmit packet */
		NETDEV_XMIT(txdev, (void *)CAST_REAL_TO_VIRT_PNBUFF(origskb,
					SKBUFF_PTR));
		return;
	}

	fraglisthead = NULL;
	fraglisttail = NULL;
	skb2 = NULL;

	DEBUG_SKBFRAG(("skb_frag_xmit4: checking for DF\n"));
	iph = (struct iphdr *)ipp;
	/* DROP the packet if DF flag is set */
	if (unlikely((iph->frag_off & htons(IP_DF)) && !(origskb->ignore_df))) {
		/*----TODO: update error stats, send icmp error message ?--- */
		kfree_skb(origskb);
		return;
	}

	hlen = iph->ihl * 4;

	DEBUG_SKBFRAG(("skb_frag_xmit4: calculating hdrs len\n"));
	/* calculate space for data,(ip payload) */
	hdrslen = ((uintptr_t)ipp - (uintptr_t)(origskb->data)) + hlen;

	left = origskb->len - hdrslen;	/* Size of ip payload */
	datapos = hdrslen;/* Where to start from */
	max_dlen =  minMtu - hdrslen;	/* ip payload per frame */

	DEBUG_SKBFRAG(("skb_frag_xmit4: computed hdrslen=%d, left=%d\n",
			hdrslen, left));

	/* frag_offset is represented in 8 byte blocks */
	offset = (ntohs(iph->frag_off) & IP_OFFSET) << 3;
	not_last_frag = iph->frag_off & htons(IP_MF);

	/* copy the excess data (>MTU size) from orig fkb to new fkb's */
	fraglisthead = origskb;

	while (left > 0) {
		DEBUG_SKBFRAG(("skb_frag_xmit4: making fragments\n"));
		len = left;
		/* IF: it doesn't fit, use 'max_dlen' - the data space left */
		if (len > max_dlen)
			len = max_dlen;
		/* IF: we are not sending upto and including the packet end
			then align the next start on an eight byte boundary */
		if (len < left)
			len &= ~7;

		if (datapos == hdrslen) {
			/*reuse the orig skb for 1st fragment */
			skb2 = origskb;
			DEBUG_SKBFRAG(("skb_frag_xmit4: reusing skb\n"));
			skb2->next = NULL;
			fraglisttail = skb2;
			skb2->len = hdrslen+len;
			skb_set_tail_pointer(skb2, hdrslen+len);
		} else {

			DEBUG_SKBFRAG(("skb_frag_xmit4: genrating new skb\n"));
			/* Allocate a new skb */
			if ((skb2 = alloc_skb(len+hdrslen, GFP_ATOMIC)) == NULL) {
				printk(KERN_INFO "no memory for new fragment!\n");
				goto fail;
			}

			/* copy skb metadata */
			skb2->queue = origskb->queue;
			skb2->priority = origskb->priority;
			skb2->dev = origskb->dev;

			dst_release(skb_dst(skb2));
			skb_dst_set(skb2, dst_clone(skb_dst(origskb)));
#ifdef CONFIG_NET_SCHED
			skb2->tc_index = origskb->tc_index;
#endif

			skb_put(skb2, len + hdrslen);

			DEBUG_SKBFRAG(("skb_frag_xmit4: copying headerto new skb\n"));

			/* copy the l2 header &l3 header to new fkb from orig fkb */
			memcpy(skb2->data, origskb->data, hdrslen);

			DEBUG_SKBFRAG(("skb_frag_xmit4: copying data to new skb\n"));
			/*
			 *	Copy a block of the IP datagram.
			 */
			memcpy(skb2->data + hdrslen, origskb->data + datapos,
					len);

			skb2->next = NULL;
			fraglisttail->next = skb2;
			fraglisttail = skb2;
		}

		/* Fill in the new header fields. */
		DEBUG_SKBFRAG(("skb_frag_xmit4: adjusting ipheader\n"));
		iph = (struct iphdr *)(skb2->data + (hdrslen- hlen));
		iph->frag_off = htons((offset >> 3));
		iph->tot_len = htons(len + hlen);

		/* fix pppoelen */
		if (is_pppoe)
			*((uint16_t*)iph - 2) = htons(len + hlen +
						sizeof(uint16_t));

		left -= len;
		datapos += len;
		offset += len;

		/* If we are fragmenting a fragment that's not the
		 * last fragment then keep MF on each fragment */
		if (left > 0 || not_last_frag)
			iph->frag_off |= htons(IP_MF);
		//else
		//iph->frag_off &= ~htons(IP_MF);/*make sure MF is cleared */

		DEBUG_SKBFRAG(("skb_frag_xmit4: computing ipcsum\n"));
		/* fix ip checksum */
		iph->check = 0;
		/* TODO replace with our own csum_calc */
		iph->check = ip_fast_csum((unsigned char *)iph, iph->ihl);

		DEBUG_SKBFRAG(("skb_frag_xmit4: loop done\n"));
	}

	/* xmit skb's */
	while (fraglisthead) {
		DEBUG_SKBFRAG(("skb_frag_xmit4: sending skb fragment \n"));
		skb2 = fraglisthead;
		fraglisthead = fraglisthead->next;
		skb2->next = NULL;
		NETDEV_XMIT(txdev, (void *)CAST_REAL_TO_VIRT_PNBUFF(skb2,
					SKBUFF_PTR));
	}
	return;

fail:
	DEBUG_SKBFRAG(("skb_frag_xmit4: ENTERED FAIL CASE\n"));
	while (fraglisthead) {
		skb2 = fraglisthead;
		fraglisthead = fraglisthead->next;
		kfree_skb(skb2);
	}
	return;

}
EXPORT_SYMBOL(skb_frag_xmit4);

#ifdef CONFIG_IPV6
static void ipv6_generate_ident(struct frag_hdr *fhdr)
{
	static atomic_t ipv6_fragmentation_id;
	int old, new;

	do {
		old = atomic_read(&ipv6_fragmentation_id);
		new = old + 1;
		if (!new)
			new = 1;
	} while (atomic_cmpxchg(&ipv6_fragmentation_id, old, new) != old);
	fhdr->identification = htonl(new);
}
#endif

/*
 * This fucntion fragments the skb into multiple skbs and xmits them
 * this fucntion is a substitue for ip6_fragment when IPv6 stack is skipped
 * for packet acceleartion
 *
 * Assumption: there should be no extension header in IPv6 header while
 *	       learning the tunnel traffic
 *
 * Currently only IPv6 is supported
 */
void skb_frag_xmit6(struct sk_buff *origskb, struct net_device *txdev,
		    uint32_t is_pppoe, uint32_t minMtu, void *ipp)
{
#ifdef CONFIG_IPV6
	struct ipv6hdr *iph;
	int datapos, offset;
	struct frag_hdr *fh;
	__be32 frag_id = 0;
	u8 nexthdr;
	unsigned int max_dlen, hlen, hdrslen, left, len, frag_hdrs_len;
	struct sk_buff *fraglisthead;
	struct sk_buff *fraglisttail;
	struct sk_buff *skb2;

	DEBUG_SKBFRAG(("skb_frag_xmit6:enter origskb=%px,netdev=%px,is_pppoe=%d,\
			minMtu=%d ipp=%px\n",origskb, txdev, is_pppoe, minMtu, ipp));

	if (likely(origskb->len <= minMtu)) {
		/* xmit packet */
		NETDEV_XMIT(txdev, (void *)CAST_REAL_TO_VIRT_PNBUFF(origskb,
					SKBUFF_PTR));
		return;
	}

	fraglisthead = NULL;
	fraglisttail = NULL;
	skb2 = NULL;

	iph = (struct ipv6hdr *)ipp;
	hlen = sizeof(struct ipv6hdr);

	DEBUG_SKBFRAG(("skb_frag_xmit6: calculating hdrs len\n"));
	/* calculate space for data,(ip payload) */
	hdrslen = ((uintptr_t)ipp - (uintptr_t)(origskb->data)) + hlen;

	left = origskb->len - hdrslen;	/* Size of remaining ip payload */
	datapos = hdrslen;/* Where to start from */
	/* hdrlens including frag_hdr of packets after fragmented */
	frag_hdrs_len = hdrslen + sizeof(struct frag_hdr);
	/* calculate max ip payload len per frame */
	max_dlen =  minMtu - frag_hdrs_len;
	nexthdr = iph->nexthdr;

	DEBUG_SKBFRAG(("skb_frag_xmit6: computed hdrslen=%d, left=%d, max=%d\n",
			hdrslen, left, max_dlen));

	offset = 0;
	/* copy the excess data (>MTU size) from orig fkb to new fkb's */
	fraglisthead = origskb;

	/* len represents length of payload! */
	while (left > 0) {
		DEBUG_SKBFRAG(("skb_frag_xmit6: making fragments\n"));
		len = left;
		/* IF: it doesn't fit, use 'max_dlen' - the data space left */
		if (len > max_dlen)
			len = max_dlen;
		/* IF: we are not sending upto and including the packet end
			then align the next start on an eight byte boundary */
		if (len < left)
			len &= ~7;

		/*
		 * Create new skbs to fragment the packet. Instead of reusing the
		 * orignal skb, a new skb is allocated to insert frag header
		 */
		DEBUG_SKBFRAG(("skb_frag_xmit6: genrating new skb\n"));
		/* Allocate a new skb */
		if ((skb2 = alloc_skb(len+frag_hdrs_len, GFP_ATOMIC)) == NULL) {
				printk(KERN_INFO "no memory for new fragment!\n");
				goto fail;
		}

		/* copy skb metadata */
		skb2->queue = origskb->queue;
		skb2->priority = origskb->priority;
		skb2->dev = origskb->dev;

		dst_release(skb_dst(skb2));
		skb_dst_set(skb2, dst_clone(skb_dst(origskb)));
#ifdef CONFIG_NET_SCHED
		skb2->tc_index = origskb->tc_index;
#endif
		skb_put(skb2, len + frag_hdrs_len);

		DEBUG_SKBFRAG(("skb_frag_xmit6: copying headerto new skb\n"));

		/* copy the l2 header & l3 header to new fkb from orig fkb */
		memcpy(skb2->data, origskb->data, hdrslen);

		DEBUG_SKBFRAG(("skb_frag_xmit6: copying data to new skb\n"));
		/* Copy a block of the IP datagram. */
		memcpy(skb2->data+frag_hdrs_len, origskb->data+datapos, len);

		skb2->next = NULL;

		/* first fragment, setup fraglist */
		if (datapos == hdrslen) {
			fraglisthead = skb2;
			fraglisttail = skb2;
		} else {
			fraglisttail->next = skb2;
			fraglisttail = skb2;
		}

		/* Fill in the new header fields. */
		DEBUG_SKBFRAG(("skb_frag_xmit6: adjusting IPv6 header\n"));
		iph = (struct ipv6hdr *)(skb2->data + (hdrslen - hlen));
		iph->payload_len = htons(len + sizeof(struct frag_hdr));
		iph->nexthdr = NEXTHDR_FRAGMENT;

		/* insert fragmentation header */
		fh = (struct frag_hdr *)(iph + 1);
		fh->nexthdr = nexthdr;
		fh->reserved = 0;
		if (!frag_id) {
			ipv6_generate_ident(fh);
			frag_id = fh->identification;
		} else
			fh->identification = frag_id;
		fh->frag_off = htons(offset);

		/* fix pppoelen */
		if (is_pppoe)
			*((uint16_t*)iph - 2) = htons(len +
					sizeof(struct frag_hdr) +
					sizeof(struct ipv6hdr) +
					sizeof(uint16_t));
		left -= len;
		datapos += len;
		offset += len;

		/* If we are fragmenting a fragment that's not the
		 * last fragment then keep MF on each fragment */
		if (left > 0)
			fh->frag_off |= htons(IP6_MF);

		DEBUG_SKBFRAG(("skb_frag_xmit6: loop done\n"));
	}

	/* xmit skb's */
	while (fraglisthead) {
		DEBUG_SKBFRAG(("skb_frag_xmit6: sending skb fragment \n"));
		skb2 = fraglisthead;
		fraglisthead = fraglisthead->next;
		skb2->next = NULL;
		NETDEV_XMIT(txdev, (void *)CAST_REAL_TO_VIRT_PNBUFF(skb2,
					SKBUFF_PTR));
	}

	/* free the orignal skb */
	kfree_skb(origskb);

	return;

fail:
	DEBUG_SKBFRAG(("skb_frag_xmit6: ENTERED FAIL CASE\n"));
	while (fraglisthead) {
		skb2 = fraglisthead;
		fraglisthead = fraglisthead->next;
		kfree_skb(skb2);
	}

	/* free the orignal skb */
	kfree_skb(origskb);

	return;

#else  /* !CONFIG_IPV6 */
	DEBUG_SKBFRAG(("skb_frag_xmit6: called while IPv6 is disabled in kernel?\n"));
	kfree_skb(origskb);
	return;
#endif
}
EXPORT_SYMBOL(skb_frag_xmit6);

/**
 * Zero out the skb's control buffer & bcm wlan ext 
 * @skb: buffer
 *
 */
void skb_cb_zero(struct sk_buff * skb)
{
	void  *buf;
	memset(skb->cb, 0, sizeof(skb->cb));

	buf = skbuff_bcm_ext_wlan_get(skb, wl_cb);
	memset(buf, 0, sizeof(struct wlan_ext));
}
EXPORT_SYMBOL(skb_cb_zero);

/**
 *
 *  bcm_skbuff_copy_skb_header - copy BCM skb header fields
 *	@new: new skbuff pointer
 *	@old: old skbuff pointer
 *  Note: Called towards the end of __copy_skb_header()
 * 
 */
void bcm_skbuff_copy_skb_header(struct sk_buff *new, const struct sk_buff *old)
{

#if defined(CONFIG_BLOG)
	blog_xfer(new, old);	/* CONFIG_BLOG: transfers blog ownership */
	new->bcm_ext.tunl		= old->bcm_ext.tunl;
#endif

	/* Copy over WLAN related fields */
	memset(skbuff_bcm_ext_wlan_get(new, pktc_cb), 0, sizeof(skbuff_bcm_ext_wlan_get(new, pktc_cb)));
	skbuff_bcm_ext_wlan_get(new, pktc_flags) = skbuff_bcm_ext_wlan_get(old, pktc_flags);
	
	/* Copy VLAN field */
	skbuff_bcm_ext_vlan_get(new, bcm_flags) = skbuff_bcm_ext_vlan_get(old, bcm_flags);
	skbuff_bcm_ext_vlan_get(new, vlan_count) = skbuff_bcm_ext_vlan_get(old, vlan_count);
	skbuff_bcm_ext_vlan_get(new, vlan_tpid) = skbuff_bcm_ext_vlan_get(old, vlan_tpid);
	skbuff_bcm_ext_vlan_get(new, cfi_save) = skbuff_bcm_ext_vlan_get(old, cfi_save);
	skbuff_bcm_ext_vlan_get(new, rxdev) = skbuff_bcm_ext_vlan_get(old, rxdev);
	memcpy(skbuff_bcm_ext_vlan_get(new, vlan_header), skbuff_bcm_ext_vlan_get(old, vlan_header), sizeof(skbuff_bcm_ext_vlan_get(old, vlan_header)));

	/* Copy SPDT fields */
	skbuff_bcm_ext_spdt_set(new, so_mark, skbuff_bcm_ext_spdt_get(old, so_mark));

	/* Copy over NBUFF related fields */
	new->queue = old->queue; /* union with mark */
	new->priority = old->priority;
	new->recycle_and_rnr_flags |= old->recycle_and_rnr_flags & SKB_RNR_FLAGS;

	/* Copy over in_dev */
	skbuff_bcm_ext_indev_get(new) = skbuff_bcm_ext_indev_get(old);

	/* Copy flags */
	new->bcm_ext.skb_fc_accel = old->bcm_ext.skb_fc_accel;

#if defined(CONFIG_BCM_USBNET_ACCELERATION)
	if (old->bcm_ext.clone_fc_head) {
		/* here we expect old->data > old->clone_fc_head, if for some reason this is not
		 * true we still need to set new->clone_fc_head.
		 * skb_avail_headroom , will check for this error
		 */
		new->bcm_ext.clone_fc_head = new->data -  (int)(old->data - old->bcm_ext.clone_fc_head);
	}
#endif
}

/**
 *
 *  bcm_skbuff_skb_clone - clone BCM skb header fields
 *	@n: cloned skbuff pointer
 *  @skb: original skbuff pointer
 *  Note: Called in the middle of __skb_clone()
 * 
 */
void bcm_skbuff_skb_clone(struct sk_buff *n, struct sk_buff *skb)
{
	n->recycle_hook = skb->recycle_hook;
	n->recycle_context = skb->recycle_context;
	n->recycle_flags = skb->recycle_flags & SKB_NO_RECYCLE;
#if defined(CONFIG_BCM_USBNET_ACCELERATION)
	n->bcm_ext.clone_wr_head = NULL;
	skb->bcm_ext.clone_wr_head = NULL;
	n->bcm_ext.clone_fc_head = skb->bcm_ext.clone_fc_head;
#endif
#if (defined(CONFIG_BCM_WLAN) || defined(CONFIG_BCM_WLAN_MODULE))
	n->bcm_ext.wlan = skb->bcm_ext.wlan;
#endif

	/* Copy over in_dev */
	skbuff_bcm_ext_indev_get(n) = skbuff_bcm_ext_indev_get(skb);

	/* Copy flags */
	n->bcm_ext.skb_fc_accel = skb->bcm_ext.skb_fc_accel;

	/* Copy over map_forward */
	skbuff_bcm_ext_map_get(n, map_forward) = skbuff_bcm_ext_map_get(skb, map_forward);

	/* Copy SPDT fields */
	skbuff_bcm_ext_spdt_set(n, so_mark, skbuff_bcm_ext_spdt_get(skb, so_mark));
}

#if defined(CONFIG_BCM_USBNET_ACCELERATION)
/**
 *	skb_clone_headers_set - set the clone_fc_head and clone_wr_head in
 *  an aggregated skb(ex: used in USBNET RX packet aggregation)
 *	@skb: buffer to operate on
 *  @len: lenghth of writable clone headroom
 *
 *  when this pointer is set you can still modify the cloned packet and also
 *  expand the packet till clone_wr_head. This is used in cases on packet aggregation.
 */
void skb_clone_headers_set(struct sk_buff *skb, unsigned int len)
{
	skb->bcm_ext.clone_fc_head = skb->data - len;
	if (skb_cloned(skb))
		skb->bcm_ext.clone_wr_head = skb->data - len;
	else
		skb->bcm_ext.clone_wr_head = NULL;
}
EXPORT_SYMBOL(skb_clone_headers_set);

/**
 *	skb_writable_headroom - bytes preceding skb->data that are writable(even on some
 *  cloned skb's);
 *	@skb: buffer to check
 *
 *	Return the number of bytes of writable free space preceding the skb->data of an &sk_buff.
 *  note:skb->cloned_wr_head is used to indicate the padding between 2 packets when multiple packets
 *  are present in buffer pointed by skb->head(ex: used in USBNET RX packet aggregation)
 *
 */
unsigned int skb_writable_headroom(const struct sk_buff *skb)
{
	if (skb_cloned(skb)) {
		if (skb->bcm_ext.clone_wr_head)
			return skb->data - skb->bcm_ext.clone_wr_head;
		else if (skb->bcm_ext.clone_fc_head)
			return 0;
	}

	return skb_headroom(skb);
}
EXPORT_SYMBOL(skb_writable_headroom);
#endif

static void bcm_skbuff_rx_mark_update(struct sk_buff *skb)
{
	/* mark IFFWAN flag in skb based on dev->priv_flags */
	if (skb->dev) {
		unsigned int mark = skb->mark;
		skb->mark |= SKBMARK_SET_IFFWAN_MARK(mark, (is_netdev_wan(skb->dev) ? 1:0));
#if defined(CONFIG_BLOG)
		if (skb->blog_p)
			skb->blog_p->isWan = is_netdev_wan(skb->dev) ? 1 : 0;
#endif
	}
}

/**
 *
 *  bcm_skbuff_ext_handle_netif_rx_internal - update ingress skb mark
 *  @skb: skbuff pointer
 *  Note: Called in the beginning of netif_rx_internal()
 * 
 */
void bcm_skbuff_handle_netif_rx_internal(struct sk_buff *skb)
{
	bcm_skbuff_rx_mark_update(skb);
}

/**
 *
 *  bcm_skbuff_ext_handle_netif_receive_skb_core - update ingress skb mark
 *  @skb: skbuff pointer
 *  Note: Called in the beginning of __netif_receive_skb_core()
 * 
 */
void bcm_skbuff_handle_netif_receive_skb_core(struct sk_buff *skb)
{
	if (!skbuff_bcm_ext_indev_get(skb)) 
		skbuff_bcm_ext_indev_set(skb, skb->dev);
	bcm_skbuff_rx_mark_update(skb);
}

#if (defined(CONFIG_BCM_WLAN) || defined(CONFIG_BCM_WLAN_MODULE))
#include <linux/if_bridge.h>
//  ETHER_TYPE_BRCM 0x886c, ETHER_TYPE_802_1X 0x888e, ETHER_TYPE_802_1X_PREAUTH 0x88c7
#define WL_AUTH_PROTOCOLS(proto) ((proto)==htons(0x886c)||(proto)==htons(0x888e)||(proto)==htons(0x88c7))

int bcm_hook_br_handle_frame_finish(struct sk_buff *skb, int state)
{
	return ((state != BR_STATE_FORWARDING) && WL_AUTH_PROTOCOLS(skb->protocol));
}

int bcm_hook_br_should_deliver(struct sk_buff *skb, int state)
{
	if (skb->pkt_type == PACKET_BROADCAST || skb->pkt_type == PACKET_MULTICAST)
		return 0;
	return ((state != BR_STATE_FORWARDING) && WL_AUTH_PROTOCOLS(skb->protocol));
}

#endif // CONFIG_BCM_WLAN
