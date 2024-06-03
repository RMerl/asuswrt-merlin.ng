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
#include <linux/version.h>

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

#include <linux/ppp_defs.h>

#include <linux/bcm_skb_defines.h>
#if IS_ENABLED(CONFIG_BCM_BPM)
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

static void skb_headerinit_fields(unsigned int headroom, unsigned int datalen,
		    struct sk_buff *skb, unsigned char *data,
		    RecycleFuncP recycle_hook, unsigned long recycle_context,
		    struct blog_t *blog_p)	/* defined(CONFIG_BLOG) */
{
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
	skb->recycle_hook = recycle_hook;
	skb->recycle_context = recycle_context;
	skb->recycle_flags = SKB_RECYCLE | SKB_DATA_RECYCLE;

#if IS_ENABLED(CONFIG_BCM_BPM)
	if (gbpm_is_buf_hw_recycle_capable(skb))
		skb->recycle_flags |= SKB_HW_RECYCLE_CAPABLE;

	if (gbpm_is_buf_hw_recycle_capable(skb->head))
		skb->recycle_flags |= SKB_DATA_HW_RECYCLE_CAPABLE;
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
		    struct blog_t *blog_p)
{
	_skb_headerreset(skb); /* memset to truesize */

	skb_headerinit_fields(headroom, datalen, skb, data, recycle_hook,
			      recycle_context, blog_p);

	_skb_shinforeset(skb_shinfo(skb));
	atomic_set(&(skb_shinfo(skb)->dataref), 1);
}
EXPORT_SYMBOL(skb_headerinit);

struct sk_buff *skb_header_alloc(void)
{
	return kmem_cache_alloc(skbuff_head_cache, GFP_ATOMIC);
}
EXPORT_SYMBOL(skb_header_alloc);


int bcm_kfree_skbmem(struct sk_buff *skb)
{
#if defined(CONFIG_BLOG)
	blog_free(skb, blog_free_reason_kfree);
#endif

#if IS_ENABLED(CONFIG_BCM_BPM)
	if (skb->recycle_flags & SKB_HW_RECYCLE_CAPABLE) {
		gbpm_recycle_hw_buf(skb, skb->recycle_context);
		return 1;
	}
#endif

	/* If the skb came from a preallocated pool, pass it to recycler hook */
	if (skb->recycle_hook && (skb->recycle_flags & SKB_RECYCLE)){
		/* TODO: resetting of flags & fileds here may  help in catching  
		 * double frees a bit early,(null pointer access)
		 */	
#if 0
		skb->head = NULL;
		skb->data = NULL;
		skb->recycle_flags &= ~(SKB_DATA_RECYCLE | SKB_DATA_HW_RECYCLE_CAPABLE);
#endif
		(*skb->recycle_hook)(skb, skb->recycle_context, SKB_RECYCLE);
		return 1;
	}
	return 0;
}

int bcm_skb_free_head(struct sk_buff *skb)
{

	/* If the data buffer came from a pre-allocated pool, recycle it.
	 * Recycling may only be performed when no references exist to it. */

	/* this check may be redundant, keep it just in case */
	if(skb->recycle_flags & SKB_DATA_HW_RECYCLE_DONE)
		return 1;

#if IS_ENABLED(CONFIG_BCM_BPM)
	if (skb->recycle_flags & SKB_DATA_HW_RECYCLE_CAPABLE) {
		gbpm_recycle_hw_buf(PHEAD_TO_PFKBUFF(skb->head), skb->recycle_context);
		skb->recycle_flags &= ~(SKB_DATA_RECYCLE | SKB_DATA_HW_RECYCLE_CAPABLE);
		return 1;
	}
#endif

	if (skb->recycle_hook && (skb->recycle_flags & SKB_DATA_RECYCLE)) {
		(*skb->recycle_hook)(skb, skb->recycle_context, SKB_DATA_RECYCLE);
		skb->recycle_flags &= ~(SKB_DATA_RECYCLE | SKB_DATA_HW_RECYCLE_CAPABLE);
		return 1;
	}

	return 0;/* buffer not freed, caller has to free*/ 
}


/*TODO remove this fucntion, redundant and incomplete, use kfree_skbmem instead */
void skb_header_free(struct sk_buff *skb)
{
	if (bcm_kfree_skbmem(skb))
		return;

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
	skb_p = kmem_cache_alloc(skbuff_head_cache, GFP_ATOMIC);
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

	if ((fkb_p->data_hw_recycle_capable))
		skb_p->recycle_flags |= SKB_DATA_HW_RECYCLE_CAPABLE;

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
struct sk_buff *skb_xlate(struct fkbuff * fkb_p)
{
	return (skb_xlate_dp(fkb_p, NULL));
}
EXPORT_SYMBOL(skb_xlate);

#ifdef CONFIG_BLOG
static inline netdev_tx_t __bcm_skb_direct_xmit_args(pNBuff_t nbuff, 
			struct net_device *dev, 
			BlogFcArgs_t *args)
{
	struct sk_buff *skb;
	netdev_tx_t ret = NETDEV_TX_BUSY;
	struct netdev_queue *txq;
	int cpu = smp_processor_id();

	skb = nbuff_xlate(nbuff);

	if (skb == NULL)
	{
		nbuff_free(nbuff);
		atomic_long_inc(&dev->tx_dropped);
		return ret;
	}

	skb->dev = dev;
	skb->bcm_ext.skb_fc_accel = 1;
	SKB_BPM_TAINTED(skb);

	/*TODO optimize this */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5,15,0))
	txq = netdev_pick_tx(dev, skb, NULL);
#else
	txq = netdev_core_pick_tx(dev, skb, NULL);
#endif
	local_bh_disable();
	HARD_TX_LOCK(dev, txq, cpu);

	if (!netif_xmit_frozen_or_drv_stopped(txq)) {
		/* It is an error if args is non-null and dev_xmit_args is null or vice-versa
		However, not throwing an error in this case since this check is done at the
		time of blog_emit()/fc_transmit() and the flow will not be accelerated
		if there is such an error */
		if (args && dev->bcm_nd_ext.dev_xmit_args)
			ret = ((HardStartXmitArgsFuncP)dev->bcm_nd_ext.dev_xmit_args)(skb, dev, args);
		else
			ret = dev->netdev_ops->ndo_start_xmit(skb, dev);
		if(ret == NETDEV_TX_OK) 
			txq_trans_update(txq);
	}

	HARD_TX_UNLOCK(dev, txq);
	local_bh_enable();

	if (!dev_xmit_complete(ret)) {
		dev_kfree_skb_any(skb);
		atomic_long_inc(&dev->tx_dropped);
	}
	return ret;
}

netdev_tx_t bcm_skb_direct_xmit(pNBuff_t nbuff, struct net_device *dev)
{
	return __bcm_skb_direct_xmit_args(nbuff, dev, NULL);
}
EXPORT_SYMBOL(bcm_skb_direct_xmit);

netdev_tx_t bcm_skb_direct_xmit_args(pNBuff_t nbuff, struct net_device *dev, BlogFcArgs_t *args)
{
	return __bcm_skb_direct_xmit_args(nbuff, dev, args);
}
EXPORT_SYMBOL(bcm_skb_direct_xmit_args);
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
int bcm_skbuff_handle_netif_rx_internal(struct sk_buff *skb, int *ret)
{
	SKB_BPM_TAINTED(skb);

#if defined(CONFIG_BLOG)
	if(blog_sinit_generic(skb, ret))
		return 1;
#endif
	bcm_skbuff_rx_mark_update(skb);
	
	return 0;
}


/**
 *
 *  bcm_skbuff_ext_handle_netif_receive_skb_core - update ingress skb mark
 *  @skb: skbuff pointer
 *  Note: Called in the beginning of __netif_receive_skb_core()
 * 
 */
int bcm_skbuff_handle_netif_receive_skb_core(struct sk_buff *skb, int *ret)
{
	SKB_BPM_TAINTED(skb);

#if defined(CONFIG_BLOG)
	if(blog_sinit_generic(skb, ret))
		return 1;
#endif

	if (!skbuff_bcm_ext_indev_get(skb)) 
		skbuff_bcm_ext_indev_set(skb, skb->dev);

	bcm_skbuff_rx_mark_update(skb);

	return 0;
}

/**
 * Zero out the skb's control buffer & bcm wlan ext 
 * @skb: buffer
 *
 */
void skb_cb_zero(struct sk_buff * skb)
{
	void *buf;
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
	new->bcm_ext.tunl = old->bcm_ext.tunl;
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
	new->bcm_ext.flags = old->bcm_ext.flags;

	/* Copy over map_id */
	skbuff_bcm_ext_map_get(new, map_id) = skbuff_bcm_ext_map_get(old, map_id);

#if defined(CONFIG_BCM_USBNET_ACCELERATION)
	if (old->bcm_ext.clone_fc_head) {
		/* here we expect old->data > old->clone_fc_head, if for some reason this is not
		 * true we still need to set new->clone_fc_head.
		 * skb_avail_headroom , will check for this error
		 */
		new->bcm_ext.clone_fc_head = new->data -  (int)(old->data - old->bcm_ext.clone_fc_head);
	}
#endif

	/* fkbInSkb field has garbage in new skb, initialize it to NULL, currently
	 * this field is just used as a placeholder in skb, so no need to copy
	*/
	new->fkbInSkb = NULL;
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
	n->recycle_flags = skb->recycle_flags & ~(SKB_RECYCLE |SKB_HW_RECYCLE_CAPABLE);
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
	n->bcm_ext.flags = skb->bcm_ext.flags;

	/* Copy over map_forward */
	skbuff_bcm_ext_map_get(n, map_forward) = skbuff_bcm_ext_map_get(skb, map_forward);

	/* Copy SPDT fields */
	skbuff_bcm_ext_spdt_set(n, so_mark, skbuff_bcm_ext_spdt_get(skb, so_mark));

	/* Copy fc_ctxt */
	n->bcm_ext.fc_ctxt = skb->bcm_ext.fc_ctxt;
	
#if defined(CONFIG_BLOG)
	if (n->blog_p) {
		blog_clone(skb, n->blog_p);
	}
#endif
}

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


