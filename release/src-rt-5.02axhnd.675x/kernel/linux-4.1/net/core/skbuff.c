/*
 *	Routines having to do with the 'struct sk_buff' memory handlers.
 *
 *	Authors:	Alan Cox <alan@lxorguk.ukuu.org.uk>
 *			Florian La Roche <rzsfl@rz.uni-sb.de>
 *
 *	Fixes:
 *		Alan Cox	:	Fixed the worst of the load
 *					balancer bugs.
 *		Dave Platt	:	Interrupt stacking fix.
 *	Richard Kooijman	:	Timestamp fixes.
 *		Alan Cox	:	Changed buffer format.
 *		Alan Cox	:	destructor hook for AF_UNIX etc.
 *		Linus Torvalds	:	Better skb_clone.
 *		Alan Cox	:	Added skb_copy.
 *		Alan Cox	:	Added all the changed routines Linus
 *					only put in the headers
 *		Ray VanTassle	:	Fixed --skb->lock in free
 *		Alan Cox	:	skb_copy copy arp field
 *		Andi Kleen	:	slabified it.
 *		Robert Olsson	:	Removed skb_head_pool
 *
 *	NOTE:
 *		The __skb_ routines should be called with interrupts
 *	disabled, or you better be *real* sure that the operation is atomic
 *	with respect to whatever list is being frobbed (e.g. via lock_sock()
 *	or via disabling bottom half handlers, etc).
 *
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License
 *	as published by the Free Software Foundation; either version
 *	2 of the License, or (at your option) any later version.
 */

/*
 *	The functions in this file will not compile correctly with gcc 2.4.x
 */

#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/module.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/kmemcheck.h>
#include <linux/mm.h>
#include <linux/interrupt.h>
#include <linux/in.h>
#include <linux/inet.h>
#include <linux/slab.h>
#include <linux/tcp.h>
#include <linux/udp.h>
#include <linux/netdevice.h>
#ifdef CONFIG_NET_CLS_ACT
#include <net/pkt_sched.h>
#endif
#include <linux/string.h>
#include <linux/skbuff.h>
#include <linux/splice.h>
#include <linux/cache.h>
#include <linux/rtnetlink.h>
#include <linux/init.h>
#include <linux/scatterlist.h>
#include <linux/errqueue.h>
#include <linux/prefetch.h>
#include <linux/if_vlan.h>

#include <net/protocol.h>
#include <net/dst.h>
#include <net/sock.h>
#include <net/checksum.h>
#include <net/ip6_checksum.h>
#include <net/xfrm.h>

#include <asm/uaccess.h>
#include <trace/events/skb.h>
#include <linux/highmem.h>
#include <linux/capability.h>
#include <linux/user_namespace.h>


struct kmem_cache *skbuff_head_cache __read_mostly;
static struct kmem_cache *skbuff_fclone_cache __read_mostly;
#if defined(CONFIG_IMQ) || defined(CONFIG_IMQ_MODULE)
static struct kmem_cache *skbuff_cb_store_cache __read_mostly;
#endif

#if defined(CONFIG_IMQ) || defined(CONFIG_IMQ_MODULE)
/* Control buffer save/restore for IMQ devices */
struct skb_cb_table {
#if defined(CONFIG_BCM_KF_NBUFF)
	char			cb[64] ____cacheline_aligned;
#else
	char			cb[48] __aligned(8);
#endif
	void			*cb_next;
	atomic_t		refcnt;
};

static DEFINE_SPINLOCK(skb_cb_store_lock);

int skb_save_cb(struct sk_buff *skb)
{
	struct skb_cb_table *next;

	next = kmem_cache_alloc(skbuff_cb_store_cache, GFP_ATOMIC);
	if (!next)
		return -ENOMEM;

	BUILD_BUG_ON(sizeof(skb->cb) != sizeof(next->cb));

	memcpy(next->cb, skb->cb, sizeof(skb->cb));
	next->cb_next = skb->cb_next;

	atomic_set(&next->refcnt, 1);

	skb->cb_next = next;
	return 0;
}
EXPORT_SYMBOL(skb_save_cb);

int skb_restore_cb(struct sk_buff *skb)
{
	struct skb_cb_table *next;

	if (!skb->cb_next)
		return 0;

	next = skb->cb_next;

	BUILD_BUG_ON(sizeof(skb->cb) != sizeof(next->cb));

	memcpy(skb->cb, next->cb, sizeof(skb->cb));
	skb->cb_next = next->cb_next;

	spin_lock(&skb_cb_store_lock);

	if (atomic_dec_and_test(&next->refcnt))
		kmem_cache_free(skbuff_cb_store_cache, next);

	spin_unlock(&skb_cb_store_lock);

	return 0;
}
EXPORT_SYMBOL(skb_restore_cb);

static void skb_copy_stored_cb(struct sk_buff *   , const struct sk_buff *     ) __attribute__ ((unused));
static void skb_copy_stored_cb(struct sk_buff *new, const struct sk_buff *__old)
{
	struct skb_cb_table *next;
	struct sk_buff *old;

	if (!__old->cb_next) {
		new->cb_next = NULL;
		return;
	}

	spin_lock(&skb_cb_store_lock);

	old = (struct sk_buff *)__old;

	next = old->cb_next;
	atomic_inc(&next->refcnt);
	new->cb_next = next;

	spin_unlock(&skb_cb_store_lock);
}
#endif
int sysctl_max_skb_frags __read_mostly = MAX_SKB_FRAGS;
EXPORT_SYMBOL(sysctl_max_skb_frags);

#if defined(CONFIG_BCM_KF_NBUFF)
#include <linux/nbuff.h>
#include <linux/blog.h>

/* Returns size of struct sk_buff */
size_t skb_size(void)
{
	return sizeof(struct sk_buff);
}
EXPORT_SYMBOL(skb_size);

size_t skb_aligned_size(void)
{
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
	if (skb->clone_fc_head) {
		/* In this case  it's unlikely but possible for
		 * the value of skb->data - skb->clone_fc_head to be negative
		 * the caller should check for negative value
		 */
		return skb->data - skb->clone_fc_head;
	} else
#endif
		return skb->data - skb->head;
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
	atomic_set(&skb->users, 1);
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

	skb->truesize = datalen + sizeof(struct sk_buff);
	atomic_set(&skb->users, 1);
	skb->head = data - headroom;
	skb->data = data;
	skb_set_tail_pointer(skb, datalen);
	/* FIXME!! check if this alignment is to ensure cache line aligned?
	 * make sure skb buf ends at 16 bytes boudary */
	skb->end = skb->tail + (0x10 - (((uintptr_t)skb_tail_pointer(skb)) & 0xf));
	skb->len = datalen;

#if defined (CONFIG_BCM_KF_BPM_BUF_TRACKING)
	GBPM_INC_REF(data);
	KERN_GBPM_TRACK_SKB(skb, GBPM_VAL_INIT, 0);
#endif
#if defined(CONFIG_BCM_KF_BLOG) && defined(CONFIG_BLOG)
	skb->blog_p = blog_p;
	if (blog_p)
		blog_p->skb_p = skb;
	// skb->tunl = NULL; memset in _skb_headerreset
#endif
#if defined(CONFIG_BCM_KF_VLAN) && (defined(CONFIG_BCM_VLAN) || defined(CONFIG_BCM_VLAN_MODULE))
	// skb->vlan_count = 0; memset in _skb_headerreset
#endif
#if defined(CONFIG_BCM_KF_MAP) && (defined(CONFIG_BCM_MAP) || defined(CONFIG_BCM_MAP_MODULE))
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

/*
 * -----------------------------------------------------------------------------
 *  Function: Free or recycle an sk_buff memory
 *
 *  CAUTION : API ignores any databuf attached to it.
 *            Caller of the function should handle databuf.
 * -----------------------------------------------------------------------------
 */
void skb_header_free(struct sk_buff *skb)
{
	/* If the skb came from a preallocated pool, pass it to recycler hook */
	if (skb->recycle_hook && (skb->recycle_flags & SKB_RECYCLE))
    {
        /* Detach data buffer */
        /* Caller should take care of databuf */
        skb->head = NULL;
        skb->data = NULL;
        skb->recycle_flags &= ~ SKB_DATA_RECYCLE;
        (*skb->recycle_hook)(skb, skb->recycle_context, SKB_RECYCLE);
    }
	else
    {
        kmem_cache_free(skbuff_head_cache, skb);
    }
}
EXPORT_SYMBOL(skb_header_free);
#endif  /* CONFIG_BCM_KF_NBUFF */

/**
 *	skb_panic - private function for out-of-line support
 *	@skb:	buffer
 *	@sz:	size
 *	@addr:	address
 *	@msg:	skb_over_panic or skb_under_panic
 *
 *	Out-of-line support for skb_put() and skb_push().
 *	Called via the wrapper skb_over_panic() or skb_under_panic().
 *	Keep out of line to prevent kernel bloat.
 *	__builtin_return_address is not used because it is not always reliable.
 */
static void skb_panic(struct sk_buff *skb, unsigned int sz, void *addr,
		      const char msg[])
{
	pr_emerg("%s: text:%p len:%d put:%d head:%p data:%p tail:%#lx end:%#lx dev:%s\n",
		 msg, addr, skb->len, sz, skb->head, skb->data,
		 (unsigned long)skb->tail, (unsigned long)skb->end,
		 skb->dev ? skb->dev->name : "<NULL>");
	BUG();
}

static void skb_over_panic(struct sk_buff *skb, unsigned int sz, void *addr)
{
	skb_panic(skb, sz, addr, __func__);
}

static void skb_under_panic(struct sk_buff *skb, unsigned int sz, void *addr)
{
	skb_panic(skb, sz, addr, __func__);
}

/*
 * kmalloc_reserve is a wrapper around kmalloc_node_track_caller that tells
 * the caller if emergency pfmemalloc reserves are being used. If it is and
 * the socket is later found to be SOCK_MEMALLOC then PFMEMALLOC reserves
 * may be used. Otherwise, the packet data may be discarded until enough
 * memory is free
 */
#define kmalloc_reserve(size, gfp, node, pfmemalloc) \
	 __kmalloc_reserve(size, gfp, node, _RET_IP_, pfmemalloc)

static void *__kmalloc_reserve(size_t size, gfp_t flags, int node,
			       unsigned long ip, bool *pfmemalloc)
{
	void *obj;
	bool ret_pfmemalloc = false;

	/*
	 * Try a regular allocation, when that fails and we're not entitled
	 * to the reserves, fail.
	 */
	obj = kmalloc_node_track_caller(size,
					flags | __GFP_NOMEMALLOC | __GFP_NOWARN,
					node);
	if (obj || !(gfp_pfmemalloc_allowed(flags)))
		goto out;

	/* Try again but now we are using pfmemalloc reserves */
	ret_pfmemalloc = true;
	obj = kmalloc_node_track_caller(size, flags, node);

out:
	if (pfmemalloc)
		*pfmemalloc = ret_pfmemalloc;

	return obj;
}

/* 	Allocate a new skbuff. We do this ourselves so we can fill in a few
 *	'private' fields and also do memory statistics to find all the
 *	[BEEP] leaks.
 *
 */

struct sk_buff *__alloc_skb_head(gfp_t gfp_mask, int node)
{
	struct sk_buff *skb;

	/* Get the HEAD */
	skb = kmem_cache_alloc_node(skbuff_head_cache,
				    gfp_mask & ~__GFP_DMA, node);
	if (!skb)
		goto out;

	/*
	 * Only clear those fields we need to clear, not those that we will
	 * actually initialise below. Hence, don't put any more fields after
	 * the tail pointer in struct sk_buff!
	 */
	memset(skb, 0, offsetof(struct sk_buff, tail));
	skb->head = NULL;
	skb->truesize = sizeof(struct sk_buff);
	atomic_set(&skb->users, 1);

	skb->mac_header = (typeof(skb->mac_header))~0U;
out:
	return skb;
}

/**
 *	__alloc_skb	-	allocate a network buffer
 *	@size: size to allocate
 *	@gfp_mask: allocation mask
 *	@flags: If SKB_ALLOC_FCLONE is set, allocate from fclone cache
 *		instead of head cache and allocate a cloned (child) skb.
 *		If SKB_ALLOC_RX is set, __GFP_MEMALLOC will be used for
 *		allocations in case the data is required for writeback
 *	@node: numa node to allocate memory on
 *
 *	Allocate a new &sk_buff. The returned buffer has no headroom and a
 *	tail room of at least size bytes. The object has a reference count
 *	of one. The return is the buffer. On a failure the return is %NULL.
 *
 *	Buffers may only be allocated from interrupts using a @gfp_mask of
 *	%GFP_ATOMIC.
 */
struct sk_buff *__alloc_skb(unsigned int size, gfp_t gfp_mask,
			    int flags, int node)
{
	struct kmem_cache *cache;
	struct skb_shared_info *shinfo;
	struct sk_buff *skb;
	u8 *data;
	bool pfmemalloc;

#if defined(CONFIG_BCM_KF_ARM64_BCM963XX) && defined(CONFIG_BCM94908) && defined(CONFIG_BCM_HND_EAP)
	/*
	 * runner on 94908 supports only 30 bit addressing, which limits
	 * its operation to the lower 1 Gig memory space.  Otherwise an
	 * extra packet copy is performed which slows its operation.
	 * Therefore, for this platform, force the buffer to always be
	 * dma'able, hence ensuring it always comes from the low 1 Gig
	 * address space.
	 */
	if (likely(!(gfp_mask & GFP_HIGHUSER)))
		gfp_mask |= GFP_DMA;

#endif /* defined(CONFIG_BCM_KF_ARM64_BCM963XX) && defined(CONFIG_BCM94908) && defined(CONFIG_BCM_HND_EAP) */

	cache = (flags & SKB_ALLOC_FCLONE)
		? skbuff_fclone_cache : skbuff_head_cache;

	if (sk_memalloc_socks() && (flags & SKB_ALLOC_RX))
		gfp_mask |= __GFP_MEMALLOC;

	/* Get the HEAD */
	skb = kmem_cache_alloc_node(cache, gfp_mask & ~__GFP_DMA, node);
	if (!skb)
		goto out;
	prefetchw(skb);

#if defined(CONFIG_BCM_KF_NBUFF)
	if (size < ENET_MIN_MTU_SIZE_EXT_SWITCH) {
		/* Add enough tailroom so that small packets can be padded
		 * with 0s to meet the Ethernet minimum pkt size of 60 bytes +
		 * EXT_SW TAG Total: 64 bytes)
		 * This will help improve performance with NAS test scenarios
		 * where the TCP ACK is usually less than 60 bytes
		 * WARNING: Note that the macro SKB_WITH_OVERHEAD() does not
		 * take into account this additional tailroom overhead. If
		 * the original size passed to this function uses the
		 * SKB_WITH_OVERHEAD macro to calculate the alloc length then
		 * this function will allocate more than what is expected and
		 * this can cause length (calculated using SKB_WITH_OVERHEAD)
		 * based operations to fail. We found few instances of skb
		 * allocations using the macro SKB_WITH_OVERHEAD (for ex:
		 * allocations using NLMSG_DEFAULT_SIZE in netlink.h).
		 * However, all those allocations allocate large skbs
		 * (page size 4k/8k) and will not enter this additional tailroom
		 * logic */
		size = ENET_MIN_MTU_SIZE_EXT_SWITCH;
	}
#endif

	/* We do our best to align skb_shared_info on a separate cache
	 * line. It usually works because kmalloc(X > SMP_CACHE_BYTES) gives
	 * aligned memory blocks, unless SLUB/SLAB debug is enabled.
	 * Both skb->head and skb_shared_info are cache line aligned.
	 */
	size = SKB_DATA_ALIGN(size);
	size += SKB_DATA_ALIGN(sizeof(struct skb_shared_info));
	data = kmalloc_reserve(size, gfp_mask, node, &pfmemalloc);
	if (!data)
		goto nodata;
	/* kmalloc(size) might give us more room than requested.
	 * Put skb_shared_info exactly at the end of allocated zone,
	 * to allow max possible filling before reallocation.
	 */
	size = SKB_WITH_OVERHEAD(ksize(data));
	prefetchw(data + size);

#if defined(CONFIG_BCM_KF_NBUFF)
	/*
	 * Clearing all fields -- fields that were not cleared before
	 * were moved to earlier locations in the structure, so just
	 * zeroing them out (OK, since we overwrite them shortly:
	 */
	memset(skb, 0, offsetof(struct sk_buff, truesize));
#else
	/*
	 * Only clear those fields we need to clear, not those that we will
	 * actually initialise below. Hence, don't put any more fields after
	 * the tail pointer in struct sk_buff!
	 */
	memset(skb, 0, offsetof(struct sk_buff, tail));
#endif
	/* Account for allocated memory : skb + skb->head */
	skb->truesize = SKB_TRUESIZE(size);
	skb->pfmemalloc = pfmemalloc;
	atomic_set(&skb->users, 1);
	skb->head = data;
	skb->data = data;
	skb_reset_tail_pointer(skb);
	skb->end = skb->tail + size;
	skb->mac_header = (typeof(skb->mac_header))~0U;
	skb->transport_header = (typeof(skb->transport_header))~0U;

	/* make sure we initialize shinfo sequentially */
	shinfo = skb_shinfo(skb);
	memset(shinfo, 0, offsetof(struct skb_shared_info, dataref));
#if defined(CONFIG_BCM_KF_NBUFF)
	shinfo->dirty_p = NULL;
#endif

	atomic_set(&shinfo->dataref, 1);
	kmemcheck_annotate_variable(shinfo->destructor_arg);

	if (flags & SKB_ALLOC_FCLONE) {
		struct sk_buff_fclones *fclones;

		fclones = container_of(skb, struct sk_buff_fclones, skb1);

		kmemcheck_annotate_bitfield(&fclones->skb2, flags1);
		skb->fclone = SKB_FCLONE_ORIG;
		atomic_set(&fclones->fclone_ref, 1);

		fclones->skb2.fclone = SKB_FCLONE_CLONE;
		fclones->skb2.pfmemalloc = pfmemalloc;
	}
out:
	return skb;
nodata:
	kmem_cache_free(cache, skb);
	skb = NULL;
	goto out;
}
EXPORT_SYMBOL(__alloc_skb);

/**
 * __build_skb - build a network buffer
 * @data: data buffer provided by caller
 * @frag_size: size of data, or 0 if head was kmalloced
 *
 * Allocate a new &sk_buff. Caller provides space holding head and
 * skb_shared_info. @data must have been allocated by kmalloc() only if
 * @frag_size is 0, otherwise data should come from the page allocator
 *  or vmalloc()
 * The return is the new skb buffer.
 * On a failure the return is %NULL, and @data is not freed.
 * Notes :
 *  Before IO, driver allocates only data buffer where NIC put incoming frame
 *  Driver should add room at head (NET_SKB_PAD) and
 *  MUST add room at tail (SKB_DATA_ALIGN(skb_shared_info))
 *  After IO, driver calls build_skb(), to allocate sk_buff and populate it
 *  before giving packet to stack.
 *  RX rings only contains data buffers, not full skbs.
 */
struct sk_buff *__build_skb(void *data, unsigned int frag_size)
{
	struct skb_shared_info *shinfo;
	struct sk_buff *skb;
	unsigned int size = frag_size ? : ksize(data);

	skb = kmem_cache_alloc(skbuff_head_cache, GFP_ATOMIC);
	if (!skb)
		return NULL;

	size -= SKB_DATA_ALIGN(sizeof(struct skb_shared_info));

	memset(skb, 0, offsetof(struct sk_buff, tail));
	skb->truesize = SKB_TRUESIZE(size);
	atomic_set(&skb->users, 1);
	skb->head = data;
	skb->data = data;
	skb_reset_tail_pointer(skb);
	skb->end = skb->tail + size;
	skb->mac_header = (typeof(skb->mac_header))~0U;
	skb->transport_header = (typeof(skb->transport_header))~0U;

	/* make sure we initialize shinfo sequentially */
	shinfo = skb_shinfo(skb);
	memset(shinfo, 0, offsetof(struct skb_shared_info, dataref));
	atomic_set(&shinfo->dataref, 1);
	kmemcheck_annotate_variable(shinfo->destructor_arg);

	return skb;
}

/* build_skb() is wrapper over __build_skb(), that specifically
 * takes care of skb->head and skb->pfmemalloc
 * This means that if @frag_size is not zero, then @data must be backed
 * by a page fragment, not kmalloc() or vmalloc()
 */
struct sk_buff *build_skb(void *data, unsigned int frag_size)
{
	struct sk_buff *skb = __build_skb(data, frag_size);

	if (skb && frag_size) {
		skb->head_frag = 1;
		if (page_is_pfmemalloc(virt_to_head_page(data)))
			skb->pfmemalloc = 1;
	}
	return skb;
}
EXPORT_SYMBOL(build_skb);

struct netdev_alloc_cache {
	struct page_frag	frag;
	/* we maintain a pagecount bias, so that we dont dirty cache line
	 * containing page->_count every time we allocate a fragment.
	 */
	unsigned int		pagecnt_bias;
};
static DEFINE_PER_CPU(struct netdev_alloc_cache, netdev_alloc_cache);
static DEFINE_PER_CPU(struct netdev_alloc_cache, napi_alloc_cache);

static struct page *__page_frag_refill(struct netdev_alloc_cache *nc,
				       gfp_t gfp_mask)
{
	const unsigned int order = NETDEV_FRAG_PAGE_MAX_ORDER;
	struct page *page = NULL;
	gfp_t gfp = gfp_mask;

	if (order) {
		gfp_mask |= __GFP_COMP | __GFP_NOWARN | __GFP_NORETRY |
			    __GFP_NOMEMALLOC;
		page = alloc_pages_node(NUMA_NO_NODE, gfp_mask, order);
		nc->frag.size = PAGE_SIZE << (page ? order : 0);
	}

	if (unlikely(!page))
		page = alloc_pages_node(NUMA_NO_NODE, gfp, 0);

	nc->frag.page = page;

	return page;
}

static void *__alloc_page_frag(struct netdev_alloc_cache __percpu *cache,
			       unsigned int fragsz, gfp_t gfp_mask)
{
	struct netdev_alloc_cache *nc = this_cpu_ptr(cache);
	struct page *page = nc->frag.page;
	unsigned int size;
	int offset;

	if (unlikely(!page)) {
refill:
		page = __page_frag_refill(nc, gfp_mask);
		if (!page)
			return NULL;

		/* if size can vary use frag.size else just use PAGE_SIZE */
		size = NETDEV_FRAG_PAGE_MAX_ORDER ? nc->frag.size : PAGE_SIZE;

		/* Even if we own the page, we do not use atomic_set().
		 * This would break get_page_unless_zero() users.
		 */
		atomic_add(size - 1, &page->_count);

		/* reset page count bias and offset to start of new frag */
		nc->pagecnt_bias = size;
		nc->frag.offset = size;
	}

	offset = nc->frag.offset - fragsz;
	if (unlikely(offset < 0)) {
		if (!atomic_sub_and_test(nc->pagecnt_bias, &page->_count))
			goto refill;

		/* if size can vary use frag.size else just use PAGE_SIZE */
		size = NETDEV_FRAG_PAGE_MAX_ORDER ? nc->frag.size : PAGE_SIZE;

		/* OK, page count is 0, we can safely set it */
		atomic_set(&page->_count, size);

		/* reset page count bias and offset to start of new frag */
		nc->pagecnt_bias = size;
		offset = size - fragsz;
	}

	nc->pagecnt_bias--;
	nc->frag.offset = offset;

	return page_address(page) + offset;
}

static void *__netdev_alloc_frag(unsigned int fragsz, gfp_t gfp_mask)
{
	unsigned long flags;
	void *data;

	local_irq_save(flags);
	data = __alloc_page_frag(&netdev_alloc_cache, fragsz, gfp_mask);
	local_irq_restore(flags);
	return data;
}

/**
 * netdev_alloc_frag - allocate a page fragment
 * @fragsz: fragment size
 *
 * Allocates a frag from a page for receive buffer.
 * Uses GFP_ATOMIC allocations.
 */
void *netdev_alloc_frag(unsigned int fragsz)
{
	return __netdev_alloc_frag(fragsz, GFP_ATOMIC | __GFP_COLD);
}
EXPORT_SYMBOL(netdev_alloc_frag);

static void *__napi_alloc_frag(unsigned int fragsz, gfp_t gfp_mask)
{
	return __alloc_page_frag(&napi_alloc_cache, fragsz, gfp_mask);
}

void *napi_alloc_frag(unsigned int fragsz)
{
	return __napi_alloc_frag(fragsz, GFP_ATOMIC | __GFP_COLD);
}
EXPORT_SYMBOL(napi_alloc_frag);

/**
 *	__alloc_rx_skb - allocate an skbuff for rx
 *	@length: length to allocate
 *	@gfp_mask: get_free_pages mask, passed to alloc_skb
 *	@flags:	If SKB_ALLOC_RX is set, __GFP_MEMALLOC will be used for
 *		allocations in case we have to fallback to __alloc_skb()
 *		If SKB_ALLOC_NAPI is set, page fragment will be allocated
 *		from napi_cache instead of netdev_cache.
 *
 *	Allocate a new &sk_buff and assign it a usage count of one. The
 *	buffer has unspecified headroom built in. Users should allocate
 *	the headroom they think they need without accounting for the
 *	built in space. The built in space is used for optimisations.
 *
 *	%NULL is returned if there is no free memory.
 */
static struct sk_buff *__alloc_rx_skb(unsigned int length, gfp_t gfp_mask,
				      int flags)
{
	struct sk_buff *skb = NULL;
	unsigned int fragsz = SKB_DATA_ALIGN(length) +
			      SKB_DATA_ALIGN(sizeof(struct skb_shared_info));

	if (fragsz <= PAGE_SIZE && !(gfp_mask & (__GFP_WAIT | GFP_DMA))) {
		void *data;

		if (sk_memalloc_socks())
			gfp_mask |= __GFP_MEMALLOC;

		data = (flags & SKB_ALLOC_NAPI) ?
			__napi_alloc_frag(fragsz, gfp_mask) :
			__netdev_alloc_frag(fragsz, gfp_mask);

		if (likely(data)) {
			skb = build_skb(data, fragsz);
			if (unlikely(!skb))
				put_page(virt_to_head_page(data));
		}
	} else {
		skb = __alloc_skb(length, gfp_mask,
				  SKB_ALLOC_RX, NUMA_NO_NODE);
	}
	return skb;
}

/**
 *	__netdev_alloc_skb - allocate an skbuff for rx on a specific device
 *	@dev: network device to receive on
 *	@length: length to allocate
 *	@gfp_mask: get_free_pages mask, passed to alloc_skb
 *
 *	Allocate a new &sk_buff and assign it a usage count of one. The
 *	buffer has NET_SKB_PAD headroom built in. Users should allocate
 *	the headroom they think they need without accounting for the
 *	built in space. The built in space is used for optimisations.
 *
 *	%NULL is returned if there is no free memory.
 */
struct sk_buff *__netdev_alloc_skb(struct net_device *dev,
				   unsigned int length, gfp_t gfp_mask)
{
	struct sk_buff *skb;

	length += NET_SKB_PAD;
	skb = __alloc_rx_skb(length, gfp_mask, 0);

	if (likely(skb)) {
		skb_reserve(skb, NET_SKB_PAD);
		skb->dev = dev;
	}

	return skb;
}
EXPORT_SYMBOL(__netdev_alloc_skb);

/**
 *	__napi_alloc_skb - allocate skbuff for rx in a specific NAPI instance
 *	@napi: napi instance this buffer was allocated for
 *	@length: length to allocate
 *	@gfp_mask: get_free_pages mask, passed to alloc_skb and alloc_pages
 *
 *	Allocate a new sk_buff for use in NAPI receive.  This buffer will
 *	attempt to allocate the head from a special reserved region used
 *	only for NAPI Rx allocation.  By doing this we can save several
 *	CPU cycles by avoiding having to disable and re-enable IRQs.
 *
 *	%NULL is returned if there is no free memory.
 */
struct sk_buff *__napi_alloc_skb(struct napi_struct *napi,
				 unsigned int length, gfp_t gfp_mask)
{
	struct sk_buff *skb;

	length += NET_SKB_PAD + NET_IP_ALIGN;
	skb = __alloc_rx_skb(length, gfp_mask, SKB_ALLOC_NAPI);

	if (likely(skb)) {
		skb_reserve(skb, NET_SKB_PAD + NET_IP_ALIGN);
		skb->dev = napi->dev;
	}

	return skb;
}
EXPORT_SYMBOL(__napi_alloc_skb);

void skb_add_rx_frag(struct sk_buff *skb, int i, struct page *page, int off,
		     int size, unsigned int truesize)
{
	skb_fill_page_desc(skb, i, page, off, size);
	skb->len += size;
	skb->data_len += size;
	skb->truesize += truesize;
}
EXPORT_SYMBOL(skb_add_rx_frag);

void skb_coalesce_rx_frag(struct sk_buff *skb, int i, int size,
			  unsigned int truesize)
{
	skb_frag_t *frag = &skb_shinfo(skb)->frags[i];

	skb_frag_size_add(frag, size);
	skb->len += size;
	skb->data_len += size;
	skb->truesize += truesize;
}
EXPORT_SYMBOL(skb_coalesce_rx_frag);

static void skb_drop_list(struct sk_buff **listp)
{
	kfree_skb_list(*listp);
	*listp = NULL;
}

static inline void skb_drop_fraglist(struct sk_buff *skb)
{
	skb_drop_list(&skb_shinfo(skb)->frag_list);
}

#if !defined(CONFIG_BCM_MPTCP) || !defined(CONFIG_BCM_KF_MPTCP)
static void skb_clone_fraglist(struct sk_buff *skb)
#else
void skb_clone_fraglist(struct sk_buff *skb)
#endif
{
	struct sk_buff *list;

	skb_walk_frags(skb, list)
		skb_get(list);
}

static void skb_free_head(struct sk_buff *skb)
{
#if defined(CONFIG_BCM_KF_NBUFF)
	/* If the data buffer came from a pre-allocated pool, recycle it.
	 * Recycling may only be performed when no references exist to it. */
	if (skb->recycle_hook && (skb->recycle_flags & SKB_DATA_RECYCLE)) {
		(*skb->recycle_hook)(skb, skb->recycle_context, SKB_DATA_RECYCLE);
		skb->recycle_flags &= SKB_DATA_NO_RECYCLE;	/* mask out */
		return;
	}
#endif
	if (skb->head_frag)
		put_page(virt_to_head_page(skb->head));
	else
		kfree(skb->head);
}

static void skb_release_data(struct sk_buff *skb)
{
	struct skb_shared_info *shinfo = skb_shinfo(skb);
	int i;

	if (skb->cloned &&
	    atomic_sub_return(skb->nohdr ? (1 << SKB_DATAREF_SHIFT) + 1 : 1,
			      &shinfo->dataref))
		return;

	for (i = 0; i < shinfo->nr_frags; i++)
		__skb_frag_unref(&shinfo->frags[i]);

	/*
	 * If skb buf is from userspace, we need to notify the caller
	 * the lower device DMA has done;
	 */
	if (shinfo->tx_flags & SKBTX_DEV_ZEROCOPY) {
		struct ubuf_info *uarg;

		uarg = shinfo->destructor_arg;
		if (uarg->callback)
			uarg->callback(uarg, true);
	}

	if (shinfo->frag_list)
		kfree_skb_list(shinfo->frag_list);

	skb_free_head(skb);
}

/*
 *	Free an skbuff by memory without cleaning the state.
 */
static void kfree_skbmem(struct sk_buff *skb)
{
	struct sk_buff_fclones *fclones;

#if defined(CONFIG_BCM_KF_NBUFF)
#if defined(CONFIG_BCM_KF_BLOG) && defined(CONFIG_BLOG)
    blog_free(skb, blog_free_reason_kfree);
#endif

	/* If the skb came from a preallocated pool, pass it to recycler hook */
	if (skb->recycle_hook && (skb->recycle_flags & SKB_RECYCLE))
		(*skb->recycle_hook)(skb, skb->recycle_context, SKB_RECYCLE);
	else {
#endif /* CONFIG_BCM_KF_NBUFF */

	switch (skb->fclone) {
	case SKB_FCLONE_UNAVAILABLE:
		kmem_cache_free(skbuff_head_cache, skb);
		return;

	case SKB_FCLONE_ORIG:
		fclones = container_of(skb, struct sk_buff_fclones, skb1);

		/* We usually free the clone (TX completion) before original skb
		 * This test would have no chance to be true for the clone,
		 * while here, branch prediction will be good.
		 */
		if (atomic_read(&fclones->fclone_ref) == 1)
			goto fastpath;
		break;

	default: /* SKB_FCLONE_CLONE */
		fclones = container_of(skb, struct sk_buff_fclones, skb2);
		break;
	}
	if (!atomic_dec_and_test(&fclones->fclone_ref))
		return;
fastpath:
	kmem_cache_free(skbuff_fclone_cache, fclones);
#if defined(CONFIG_BCM_KF_NBUFF)
	}
#endif	/* CONFIG_BCM_KF_NBUFF */
}

static void skb_release_head_state(struct sk_buff *skb)
{
	skb_dst_drop(skb);
#ifdef CONFIG_XFRM
	secpath_put(skb->sp);
#endif
	if (skb->destructor) {
		WARN_ON(in_irq());
		skb->destructor(skb);
	}
#if defined(CONFIG_IMQ) || defined(CONFIG_IMQ_MODULE)
	/*
	 * This should not happen. When it does, avoid memleak by restoring
	 * the chain of cb-backups.
	 */
	while (skb->cb_next != NULL) {
		if (net_ratelimit())
			pr_warn("IMQ: kfree_skb: skb->cb_next: %08lx\n",
				(unsigned long)skb->cb_next);

		skb_restore_cb(skb);
	}
	/*
	 * This should not happen either, nf_queue_entry is nullified in
	 * imq_dev_xmit(). If we have non-NULL nf_queue_entry then we are
	 * leaking entry pointers, maybe memory. We don't know if this is
	 * pointer to already freed memory, or should this be freed.
	 * If this happens we need to add refcounting, etc for nf_queue_entry.
	 */
	if (skb->nf_queue_entry && net_ratelimit())
		pr_warn("%s\n", "IMQ: kfree_skb: skb->nf_queue_entry != NULL");
#endif
#if IS_ENABLED(CONFIG_NF_CONNTRACK)
	nf_conntrack_put(skb->nfct);
#endif
#if IS_ENABLED(CONFIG_BRIDGE_NETFILTER)
	nf_bridge_put(skb->nf_bridge);
#endif
#if defined(CONFIG_BCM_KF_NBUFF)
	skb->tc_word = 0;
#endif	/* CONFIG_BCM_KF_NBUFF */
}

/* Free everything but the sk_buff shell. */
static void skb_release_all(struct sk_buff *skb)
{
	skb_release_head_state(skb);
	if (likely(skb->head))
		skb_release_data(skb);
}

/**
 *	__kfree_skb - private function
 *	@skb: buffer
 *
 *	Free an sk_buff. Release anything attached to the buffer.
 *	Clean the state. This is an internal helper function. Users should
 *	always call kfree_skb
 */

void __kfree_skb(struct sk_buff *skb)
{
#if defined (CONFIG_BCM_KF_BPM_BUF_TRACKING)
	GBPM_DEC_REF(skb->data);
	KERN_GBPM_TRACK_SKB(skb, GBPM_VAL_FREE, 0);
#endif
#if defined(CONFIG_BCM_KF_NBUFF)
	if (skb->recycle_hook && (skb->recycle_flags & SKB_RECYCLE_NOFREE)) {
		(*skb->recycle_hook)(skb, skb->recycle_context, SKB_RECYCLE_NOFREE);
		return;
	}
#endif
	skb_release_all(skb);
	kfree_skbmem(skb);
}
EXPORT_SYMBOL(__kfree_skb);

/**
 *	kfree_skb - free an sk_buff
 *	@skb: buffer to free
 *
 *	Drop a reference to the buffer and free it if the usage count has
 *	hit zero.
 */
void kfree_skb(struct sk_buff *skb)
{
	if (unlikely(!skb))
		return;
	if (likely(atomic_read(&skb->users) == 1))
		smp_rmb();
	else if (likely(!atomic_dec_and_test(&skb->users)))
		return;
	trace_kfree_skb(skb, __builtin_return_address(0));
	__kfree_skb(skb);
}
EXPORT_SYMBOL(kfree_skb);

void kfree_skb_list(struct sk_buff *segs)
{
	while (segs) {
		struct sk_buff *next = segs->next;

		kfree_skb(segs);
		segs = next;
	}
}
EXPORT_SYMBOL(kfree_skb_list);

#if defined(CONFIG_BCM_KF_NBUFF)
/*
 * Translate a fkb to a skb, by allocating a skb from the skbuff_head_cache.
 * PS. skb->dev is not set during initialization.
 *
 * Caller verifies whether the fkb is unshared:
 *  if fkb_p==NULL||IS_FKB_CLONE(fkb_p)||fkb_p->users>1 and return NULL skb.
 *
 * skb_xlate is deprecated.  New code should call skb_xlate_dp directly.
 */
struct sk_buff * skb_xlate(struct fkbuff * fkb_p)
{
	return (skb_xlate_dp(fkb_p, NULL));
}
EXPORT_SYMBOL(skb_xlate);

struct sk_buff *skb_xlate_dp(struct fkbuff * fkb_p, uint8_t *dirty_p)
{
	struct sk_buff * skb_p;
	unsigned int datalen;

	/* Optimization: use preallocated pool of skb with SKB_POOL_RECYCLE flag */
	skb_p = kmem_cache_alloc(skbuff_head_cache, GFP_ATOMIC);
	if (!skb_p)
		return skb_p;
	skb_p->fclone = SKB_FCLONE_UNAVAILABLE;

	memset(skb_p, 0, offsetof(struct sk_buff, truesize));

	datalen = SKB_DATA_ALIGN(fkb_p->len + BCM_SKB_TAILROOM);

	skb_p->data = fkb_p->data;
	skb_p->head = (unsigned char *)(fkb_p + 1 );
	skb_set_tail_pointer(skb_p, fkb_p->len);
	/* FIXME!! check whether this has to do with the cache line size
	 * make sure skb buf ends at 16 bytes boudary */
#ifdef NET_SKBUFF_DATA_USES_OFFSET
	skb_p->end = (skb_p->data - skb_p->head) + datalen;
#else
	skb_p->end = skb_p->data + datalen;
#endif

#if defined (CONFIG_BCM_KF_BPM_BUF_TRACKING)
	GBPM_INC_REF(skb_p->data);
	KERN_GBPM_TRACK_SKB(skb_p, GBPM_VAL_XLATE, 0);
#endif

#define F2S(x) skb_p->x = fkb_p->x
	F2S(len);
	F2S(queue);
	F2S(priority);

#if defined(CONFIG_BCM_KF_BLOG) && defined(CONFIG_BLOG)
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

	atomic_set(&skb_p->users, 1);
	skb_p->truesize = datalen + sizeof(struct sk_buff);

	/* any change to skb_shinfo initialization in __alloc_skb must be ported
	 * to this block. */
	atomic_set(&(skb_shinfo(skb_p)->dataref), 1);
	skb_shinfo(skb_p)->nr_frags = 0;
	skb_shinfo(skb_p)->gso_size = 0;
	skb_shinfo(skb_p)->gso_segs = 0;
	skb_shinfo(skb_p)->gso_type = 0;
	skb_shinfo(skb_p)->ip6_frag_id = 0;
	skb_shinfo(skb_p)->tx_flags = 0;
	skb_shinfo(skb_p)->frag_list = NULL;
	memset(&(skb_shinfo(skb_p)->hwtstamps), 0,
	       sizeof(skb_shinfo(skb_p)->hwtstamps));
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

	return skb_p;
#undef F2S
}
EXPORT_SYMBOL(skb_xlate_dp);

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

	DEBUG_SKBFRAG(("skb_frag_xmit4:enter origskb=%p,netdev=%p,is_pppoe=%d,\
				minMtu=%d ipp=%p\n",origskb, txdev, is_pppoe,
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
 *             learning the tunnel traffic
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

	DEBUG_SKBFRAG(("skb_frag_xmit6:enter origskb=%p,netdev=%p,is_pppoe=%d,\
			minMtu=%d ipp=%p\n",origskb, txdev, is_pppoe, minMtu, ipp));

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
#endif  /* defined(CONFIG_BCM_KF_NBUFF) */

/**
 *	skb_tx_error - report an sk_buff xmit error
 *	@skb: buffer that triggered an error
 *
 *	Report xmit error if a device callback is tracking this skb.
 *	skb must be freed afterwards.
 */
void skb_tx_error(struct sk_buff *skb)
{
	if (skb_shinfo(skb)->tx_flags & SKBTX_DEV_ZEROCOPY) {
		struct ubuf_info *uarg;

		uarg = skb_shinfo(skb)->destructor_arg;
		if (uarg->callback)
			uarg->callback(uarg, false);
		skb_shinfo(skb)->tx_flags &= ~SKBTX_DEV_ZEROCOPY;
	}
}
EXPORT_SYMBOL(skb_tx_error);

/**
 *	consume_skb - free an skbuff
 *	@skb: buffer to free
 *
 *	Drop a ref to the buffer and free it if the usage count has hit zero
 *	Functions identically to kfree_skb, but kfree_skb assumes that the frame
 *	is being dropped after a failure and notes that
 */
void consume_skb(struct sk_buff *skb)
{
	if (unlikely(!skb))
		return;
	if (likely(atomic_read(&skb->users) == 1))
		smp_rmb();
	else if (likely(!atomic_dec_and_test(&skb->users)))
		return;
	trace_consume_skb(skb);
	__kfree_skb(skb);
}
EXPORT_SYMBOL(consume_skb);

/* Make sure a field is enclosed inside headers_start/headers_end section */
#define CHECK_SKB_FIELD(field) \
	BUILD_BUG_ON(offsetof(struct sk_buff, field) <		\
		     offsetof(struct sk_buff, headers_start));	\
	BUILD_BUG_ON(offsetof(struct sk_buff, field) >		\
		     offsetof(struct sk_buff, headers_end));	\

static void __copy_skb_header(struct sk_buff *new, const struct sk_buff *old)
{
#if defined(CONFIG_BCM_KF_VLAN) && (defined(CONFIG_BCM_VLAN) || defined(CONFIG_BCM_VLAN_MODULE))
	int i;
#endif

#if defined(CONFIG_BCM_KF_WL)
	memset(new->pktc_cb, 0, sizeof(new->pktc_cb));
#endif

	new->tstamp		= old->tstamp;
	/* We do not copy old->sk */
	new->dev		= old->dev;
	memcpy(new->cb, old->cb, sizeof(old->cb));
	skb_dst_copy(new, old);
#ifdef CONFIG_XFRM
	new->sp			= secpath_get(old->sp);
#endif
	__nf_copy(new, old, false);
#if defined(CONFIG_IMQ) || defined(CONFIG_IMQ_MODULE)
	new->cb_next = NULL;
	/*skb_copy_stored_cb(new, old);*/
#endif

	/* Note : this field could be in headers_start/headers_end section
	 * It is not yet because we do not want to have a 16 bit hole
	 */
	new->queue_mapping = old->queue_mapping;
#if defined(CONFIG_BCM_KF_NBUFF)
	new->queue = old->queue;
	new->priority = old->priority;
#endif

	memcpy(&new->headers_start, &old->headers_start,
	       offsetof(struct sk_buff, headers_end) -
	       offsetof(struct sk_buff, headers_start));
	CHECK_SKB_FIELD(protocol);
	CHECK_SKB_FIELD(csum);
	CHECK_SKB_FIELD(hash);
#if defined(CONFIG_BCM_KF_NBUFF)
#else
	CHECK_SKB_FIELD(priority);
#endif
	CHECK_SKB_FIELD(skb_iif);
	CHECK_SKB_FIELD(vlan_proto);
	CHECK_SKB_FIELD(vlan_tci);
	CHECK_SKB_FIELD(transport_header);
	CHECK_SKB_FIELD(network_header);
	CHECK_SKB_FIELD(mac_header);

	CHECK_SKB_FIELD(inner_protocol);
	CHECK_SKB_FIELD(inner_transport_header);
	CHECK_SKB_FIELD(inner_network_header);
	CHECK_SKB_FIELD(inner_mac_header);
#if defined(CONFIG_BCM_KF_NBUFF)
#else
	CHECK_SKB_FIELD(mark);
#endif
#ifdef CONFIG_NETWORK_SECMARK
	CHECK_SKB_FIELD(secmark);
#endif
#ifdef CONFIG_NET_RX_BUSY_POLL
	CHECK_SKB_FIELD(napi_id);
#endif
#ifdef CONFIG_XPS
	CHECK_SKB_FIELD(sender_cpu);
#endif
#if defined(CONFIG_BCM_KF_BLOG) && defined(CONFIG_BLOG)
	new->tunl		= old->tunl;
#endif
#if defined(CONFIG_BCM_KF_BLOG) && defined(CONFIG_BLOG)
	blog_xfer(new, old);	/* CONFIG_BLOG: transfers blog ownership */
#endif

#if (defined(CONFIG_BCM_KF_USBNET) && defined(CONFIG_BCM_USBNET_ACCELERATION))
	if (old->clone_fc_head) {
		/* here we expect old->data > old->clone_fc_head, if for some reason this is not
		 * true we still need to set new->clone_fc_head.
		 * skb_avail_headroom , will check for this error
		 */
		new->clone_fc_head = new->data -  (int)(old->data - old->clone_fc_head);
	}
#endif

#if defined(CONFIG_BCM_KF_NBUFF)
	new->vtag_word = old->vtag_word;
	new->tc_word = old->tc_word;
#if defined(CONFIG_BCM_KF_VLAN) && (defined(CONFIG_BCM_VLAN) || defined(CONFIG_BCM_VLAN_MODULE))
	new->vlan_count = old->vlan_count;
	new->vlan_tpid = old->vlan_tpid;
	for (i = 0; i < SKB_VLAN_MAX_TAGS; i++) {
		new->vlan_header[i] = old->vlan_header[i];
	}
	new->rxdev = old->rxdev;
#endif /* BCM_VLAN */
#else  /* CONFIG_BCM_KF_NBUFF */

#ifdef CONFIG_NET_SCHED
	CHECK_SKB_FIELD(tc_index);
#ifdef CONFIG_NET_CLS_ACT
	CHECK_SKB_FIELD(tc_verd);
#endif
#endif
#endif /* CONFIG_BCM_KF_NBUFF */

#if defined(CONFIG_BCM_KF_WL)
	new->pktc_flags		= old->pktc_flags;
#endif

}

/*
 * You should not add any new code to this function.  Add it to
 * __copy_skb_header above instead.
 */
static struct sk_buff *__skb_clone(struct sk_buff *n, struct sk_buff *skb)
{
#if defined(CONFIG_BCM_KF_VLAN) && (defined(CONFIG_BCM_VLAN) || defined(CONFIG_BCM_VLAN_MODULE))
	int i;
#endif
#define C(x) n->x = skb->x

	n->next = n->prev = NULL;
	n->sk = NULL;
	__copy_skb_header(n, skb);

	C(len);
	C(data_len);
	C(mac_len);
	n->hdr_len = skb->nohdr ? skb_headroom(skb) : skb->hdr_len;
	n->cloned = 1;
	n->nohdr = 0;
	n->peeked = 0;
	n->destructor = NULL;
	C(tail);
	C(end);
	C(head);
	C(head_frag);
	C(data);
	C(truesize);

#if (defined(CONFIG_BCM_KF_USBNET) && defined(CONFIG_BCM_USBNET_ACCELERATION))
	n->clone_wr_head = NULL;
	skb->clone_wr_head = NULL;
	C(clone_fc_head);
#endif

#if defined (CONFIG_BCM_KF_BPM_BUF_TRACKING)
	GBPM_INC_REF(skb->data);
	KERN_GBPM_TRACK_SKB(skb, GBPM_VAL_COPY_SRC, 0);
	KERN_GBPM_TRACK_SKB(skb, GBPM_VAL_COPY_DST, 0);
#endif

#if defined(CONFIG_BCM_KF_NBUFF)
	C(recycle_hook);
	C(recycle_context);
	n->recycle_flags = skb->recycle_flags & SKB_NO_RECYCLE;
#if defined(CONFIG_BCM_KF_VLAN) && (defined(CONFIG_BCM_VLAN) || defined(CONFIG_BCM_VLAN_MODULE))
	C(vlan_count);
	C(vlan_tpid);
	C(cfi_save);
	C(vlan_tci);
	C(vlan_proto);
	for (i = 0; i<SKB_VLAN_MAX_TAGS; i++) {
		C(vlan_header[i]);
	}
	C(rxdev);
#endif /* BCM_VLAN */
    C(bcm_flags.bcm_flags_word);
#endif /* CONFIG_BCM_KF_NBUFF */

#if defined(CONFIG_BCM_KF_80211) && (defined(CONFIG_MAC80211) || defined(CONFIG_MAC80211_MODULE))
	C(do_not_encrypt);
	C(requeue);
#endif
	atomic_set(&n->users, 1);

	atomic_inc(&(skb_shinfo(skb)->dataref));
	skb->cloned = 1;

	return n;
#undef C
}

/**
 *	skb_morph	-	morph one skb into another
 *	@dst: the skb to receive the contents
 *	@src: the skb to supply the contents
 *
 *	This is identical to skb_clone except that the target skb is
 *	supplied by the user.
 *
 *	The target skb is returned upon exit.
 */
struct sk_buff *skb_morph(struct sk_buff *dst, struct sk_buff *src)
{
#if defined(CONFIG_BCM_KF_NBUFF)
	struct sk_buff *skb;
	unsigned int recycle_flags;
	unsigned long recycle_context;
	RecycleFuncP recycle_hook;

	skb_release_all(dst);

	/* Need to retain the recycle flags, context & hook of dst to free it
	 * into proper pool. */
	recycle_flags = dst->recycle_flags & SKB_RECYCLE;
	recycle_hook = dst->recycle_hook;
	recycle_context = dst->recycle_context;

	skb = __skb_clone(dst, src);

	dst->recycle_flags |= recycle_flags;
	dst->recycle_hook = recycle_hook;
	dst->recycle_context = recycle_context;
	return skb;
#else /* CONFIG_BCM_KF_NBUFF */
	skb_release_all(dst);
	return __skb_clone(dst, src);
#endif /* CONFIG_BCM_KF_NBUFF */
}
EXPORT_SYMBOL_GPL(skb_morph);

/**
 *	skb_copy_ubufs	-	copy userspace skb frags buffers to kernel
 *	@skb: the skb to modify
 *	@gfp_mask: allocation priority
 *
 *	This must be called on SKBTX_DEV_ZEROCOPY skb.
 *	It will copy all frags into kernel and drop the reference
 *	to userspace pages.
 *
 *	If this function is called from an interrupt gfp_mask() must be
 *	%GFP_ATOMIC.
 *
 *	Returns 0 on success or a negative error code on failure
 *	to allocate kernel memory to copy to.
 */
int skb_copy_ubufs(struct sk_buff *skb, gfp_t gfp_mask)
{
	int i;
	int num_frags = skb_shinfo(skb)->nr_frags;
	struct page *page, *head = NULL;
	struct ubuf_info *uarg = skb_shinfo(skb)->destructor_arg;

	for (i = 0; i < num_frags; i++) {
		u8 *vaddr;
		skb_frag_t *f = &skb_shinfo(skb)->frags[i];

		page = alloc_page(gfp_mask);
		if (!page) {
			while (head) {
				struct page *next = (struct page *)page_private(head);
				put_page(head);
				head = next;
			}
			return -ENOMEM;
		}
		vaddr = kmap_atomic(skb_frag_page(f));
		memcpy(page_address(page),
		       vaddr + f->page_offset, skb_frag_size(f));
		kunmap_atomic(vaddr);
		set_page_private(page, (unsigned long)head);
		head = page;
	}

	/* skb frags release userspace buffers */
	for (i = 0; i < num_frags; i++)
		skb_frag_unref(skb, i);

	uarg->callback(uarg, false);

	/* skb frags point to kernel buffers */
	for (i = num_frags - 1; i >= 0; i--) {
		__skb_fill_page_desc(skb, i, head, 0,
				     skb_shinfo(skb)->frags[i].size);
		head = (struct page *)page_private(head);
	}

	skb_shinfo(skb)->tx_flags &= ~SKBTX_DEV_ZEROCOPY;
	return 0;
}
EXPORT_SYMBOL_GPL(skb_copy_ubufs);

/**
 *	skb_clone	-	duplicate an sk_buff
 *	@skb: buffer to clone
 *	@gfp_mask: allocation priority
 *
 *	Duplicate an &sk_buff. The new one is not owned by a socket. Both
 *	copies share the same packet data but not structure. The new
 *	buffer has a reference count of 1. If the allocation fails the
 *	function returns %NULL otherwise the new buffer is returned.
 *
 *	If this function is called from an interrupt gfp_mask() must be
 *	%GFP_ATOMIC.
 */

struct sk_buff *skb_clone(struct sk_buff *skb, gfp_t gfp_mask)
{
	struct sk_buff_fclones *fclones = container_of(skb,
						       struct sk_buff_fclones,
						       skb1);
	struct sk_buff *n;

	if (skb_orphan_frags(skb, gfp_mask))
		return NULL;

	if (skb->fclone == SKB_FCLONE_ORIG &&
	    atomic_read(&fclones->fclone_ref) == 1) {
		n = &fclones->skb2;
		atomic_set(&fclones->fclone_ref, 2);
	} else {
		if (skb_pfmemalloc(skb))
			gfp_mask |= __GFP_MEMALLOC;

		n = kmem_cache_alloc(skbuff_head_cache, gfp_mask);
		if (!n)
			return NULL;

		kmemcheck_annotate_bitfield(n, flags1);
		n->fclone = SKB_FCLONE_UNAVAILABLE;
	}

	return __skb_clone(n, skb);
}
EXPORT_SYMBOL(skb_clone);

static void skb_headers_offset_update(struct sk_buff *skb, int off)
{
	/* Only adjust this if it actually is csum_start rather than csum */
	if (skb->ip_summed == CHECKSUM_PARTIAL)
		skb->csum_start += off;
	/* {transport,network,mac}_header and tail are relative to skb->head */
	skb->transport_header += off;
	skb->network_header   += off;
	if (skb_mac_header_was_set(skb))
		skb->mac_header += off;
	skb->inner_transport_header += off;
	skb->inner_network_header += off;
	skb->inner_mac_header += off;
}

#if !defined(CONFIG_BCM_MPTCP) || !defined(CONFIG_BCM_KF_MPTCP)
static void copy_skb_header(struct sk_buff *new, const struct sk_buff *old)
#else
void copy_skb_header(struct sk_buff *new, const struct sk_buff *old)
#endif
{
	__copy_skb_header(new, old);

	skb_shinfo(new)->gso_size = skb_shinfo(old)->gso_size;
	skb_shinfo(new)->gso_segs = skb_shinfo(old)->gso_segs;
	skb_shinfo(new)->gso_type = skb_shinfo(old)->gso_type;
}

static inline int skb_alloc_rx_flag(const struct sk_buff *skb)
{
	if (skb_pfmemalloc(skb))
		return SKB_ALLOC_RX;
	return 0;
}

/**
 *	skb_copy	-	create private copy of an sk_buff
 *	@skb: buffer to copy
 *	@gfp_mask: allocation priority
 *
 *	Make a copy of both an &sk_buff and its data. This is used when the
 *	caller wishes to modify the data and needs a private copy of the
 *	data to alter. Returns %NULL on failure or the pointer to the buffer
 *	on success. The returned buffer has a reference count of 1.
 *
 *	As by-product this function converts non-linear &sk_buff to linear
 *	one, so that &sk_buff becomes completely private and caller is allowed
 *	to modify all the data of returned buffer. This means that this
 *	function is not recommended for use in circumstances when only
 *	header is going to be modified. Use pskb_copy() instead.
 */

struct sk_buff *skb_copy(const struct sk_buff *skb, gfp_t gfp_mask)
{
	int headerlen = skb_headroom(skb);
	unsigned int size = skb_end_offset(skb) + skb->data_len;
	struct sk_buff *n = __alloc_skb(size, gfp_mask,
					skb_alloc_rx_flag(skb), NUMA_NO_NODE);

	if (!n)
		return NULL;

	/* Set the data pointer */
	skb_reserve(n, headerlen);
	/* Set the tail pointer and length */
	skb_put(n, skb->len);

	if (skb_copy_bits(skb, -headerlen, n->head, headerlen + skb->len))
		BUG();

	copy_skb_header(n, skb);

#if defined (CONFIG_BCM_KF_BPM_BUF_TRACKING)
	KERN_GBPM_TRACK_SKB(skb, GBPM_VAL_COPY_SRC, 1);
#endif
	return n;
}
EXPORT_SYMBOL(skb_copy);

/**
 *	__pskb_copy_fclone	-  create copy of an sk_buff with private head.
 *	@skb: buffer to copy
 *	@headroom: headroom of new skb
 *	@gfp_mask: allocation priority
 *	@fclone: if true allocate the copy of the skb from the fclone
 *	cache instead of the head cache; it is recommended to set this
 *	to true for the cases where the copy will likely be cloned
 *
 *	Make a copy of both an &sk_buff and part of its data, located
 *	in header. Fragmented data remain shared. This is used when
 *	the caller wishes to modify only header of &sk_buff and needs
 *	private copy of the header to alter. Returns %NULL on failure
 *	or the pointer to the buffer on success.
 *	The returned buffer has a reference count of 1.
 */

struct sk_buff *__pskb_copy_fclone(struct sk_buff *skb, int headroom,
				   gfp_t gfp_mask, bool fclone)
{
	unsigned int size = skb_headlen(skb) + headroom;
	int flags = skb_alloc_rx_flag(skb) | (fclone ? SKB_ALLOC_FCLONE : 0);
	struct sk_buff *n = __alloc_skb(size, gfp_mask, flags, NUMA_NO_NODE);

	if (!n)
		goto out;

	/* Set the data pointer */
	skb_reserve(n, headroom);
	/* Set the tail pointer and length */
	skb_put(n, skb_headlen(skb));
	/* Copy the bytes */
	skb_copy_from_linear_data(skb, n->data, n->len);

	n->truesize += skb->data_len;
	n->data_len  = skb->data_len;
	n->len	     = skb->len;

	if (skb_shinfo(skb)->nr_frags) {
		int i;

		if (skb_orphan_frags(skb, gfp_mask)) {
			kfree_skb(n);
			n = NULL;
			goto out;
		}
		for (i = 0; i < skb_shinfo(skb)->nr_frags; i++) {
			skb_shinfo(n)->frags[i] = skb_shinfo(skb)->frags[i];
			skb_frag_ref(skb, i);
		}
		skb_shinfo(n)->nr_frags = i;
	}

	if (skb_has_frag_list(skb)) {
		skb_shinfo(n)->frag_list = skb_shinfo(skb)->frag_list;
		skb_clone_fraglist(n);
	}

	copy_skb_header(n, skb);

#if defined (CONFIG_BCM_KF_BPM_BUF_TRACKING)
	KERN_GBPM_TRACK_SKB(skb, GBPM_VAL_COPY_SRC, 2);
#endif

out:
	return n;
}
EXPORT_SYMBOL(__pskb_copy_fclone);

/**
 *	pskb_expand_head - reallocate header of &sk_buff
 *	@skb: buffer to reallocate
 *	@nhead: room to add at head
 *	@ntail: room to add at tail
 *	@gfp_mask: allocation priority
 *
 *	Expands (or creates identical copy, if @nhead and @ntail are zero)
 *	header of @skb. &sk_buff itself is not changed. &sk_buff MUST have
 *	reference count of 1. Returns zero in the case of success or error,
 *	if expansion failed. In the last case, &sk_buff is not changed.
 *
 *	All the pointers pointing into skb header may change and must be
 *	reloaded after call to this function.
 */

int pskb_expand_head(struct sk_buff *skb, int nhead, int ntail,
		     gfp_t gfp_mask)
{
	int i;
	u8 *data;
	int size = nhead + skb_end_offset(skb) + ntail;
	long off;
#if (defined(CONFIG_BCM_KF_USBNET) && defined(CONFIG_BCM_USBNET_ACCELERATION))
	int clone_fc_len = 0;
#endif

	BUG_ON(nhead < 0);

	if (skb_shared(skb))
		BUG();

	size = SKB_DATA_ALIGN(size);

	if (skb_pfmemalloc(skb))
		gfp_mask |= __GFP_MEMALLOC;
	data = kmalloc_reserve(size + SKB_DATA_ALIGN(sizeof(struct skb_shared_info)),
			       gfp_mask, NUMA_NO_NODE, NULL);
	if (!data)
		goto nodata;
	size = SKB_WITH_OVERHEAD(ksize(data));

	/* Copy only real data... and, alas, header. This should be
	 * optimized for the cases when header is void.
	 */
	memcpy(data + nhead, skb->head, skb_tail_pointer(skb) - skb->head);

	memcpy((struct skb_shared_info *)(data + size),
	       skb_shinfo(skb),
	       offsetof(struct skb_shared_info, frags[skb_shinfo(skb)->nr_frags]));

	/*
	 * if shinfo is shared we must drop the old head gracefully, but if it
	 * is not we can just drop the old head and let the existing refcount
	 * be since all we did is relocate the values
	 */
	if (skb_cloned(skb)) {
		/* copy this zero copy skb frags */
		if (skb_orphan_frags(skb, gfp_mask))
			goto nofrags;
		for (i = 0; i < skb_shinfo(skb)->nr_frags; i++)
			skb_frag_ref(skb, i);

		if (skb_has_frag_list(skb))
			skb_clone_fraglist(skb);

		skb_release_data(skb);
	} else {
		skb_free_head(skb);
	}
	off = (data + nhead) - skb->head;

	skb->head     = data;
	skb->head_frag = 0;
#if (defined(CONFIG_BCM_KF_USBNET) && defined(CONFIG_BCM_USBNET_ACCELERATION))
	if (skb->clone_fc_head)
		clone_fc_len = skb->data - skb->clone_fc_head;
#endif

	skb->data    += off;

#if (defined(CONFIG_BCM_KF_USBNET) && defined(CONFIG_BCM_USBNET_ACCELERATION))
	if (skb->clone_fc_head)
		skb->clone_fc_head = skb->data - clone_fc_len;
#endif

#ifdef NET_SKBUFF_DATA_USES_OFFSET
	skb->end      = size;
	off           = nhead;
#else
	skb->end      = skb->head + size;
#endif
	skb->tail	      += off;
	skb_headers_offset_update(skb, nhead);
	skb->cloned   = 0;
	skb->hdr_len  = 0;
	skb->nohdr    = 0;

#if defined(CONFIG_BCM_KF_NBUFF)
	/* Clear Data recycle as this buffer was allocated via kmalloc.
	 * Note that skb_release_data might have already cleared it but it is
	 * not guaranteed. If the buffer is cloned, then skb_release_data
	 * does not clear the buffer. The original data buffer will be freed
	 * when the cloned skb is freed */
	skb->recycle_flags &= SKB_DATA_NO_RECYCLE;
	/* The data buffer of this skb is not pre-allocated any more
	 * even though the skb itself is pre-allocated,
	 * dirty_p pertains to previous buffer so clear it */
	skb_shinfo(skb)->dirty_p = NULL;
#endif

	atomic_set(&skb_shinfo(skb)->dataref, 1);
	return 0;

nofrags:
	kfree(data);
nodata:
	return -ENOMEM;
}
EXPORT_SYMBOL(pskb_expand_head);

/* Make private copy of skb with writable head and some headroom */

struct sk_buff *skb_realloc_headroom(struct sk_buff *skb, unsigned int headroom)
{
	struct sk_buff *skb2;
	int delta = headroom - skb_headroom(skb);

	if (delta <= 0)
		skb2 = pskb_copy(skb, GFP_ATOMIC);
	else {
		skb2 = skb_clone(skb, GFP_ATOMIC);
		if (skb2 && pskb_expand_head(skb2, SKB_DATA_ALIGN(delta), 0,
					     GFP_ATOMIC)) {
			kfree_skb(skb2);
			skb2 = NULL;
		}
	}
	return skb2;
}
EXPORT_SYMBOL(skb_realloc_headroom);

/**
 *	skb_copy_expand	-	copy and expand sk_buff
 *	@skb: buffer to copy
 *	@newheadroom: new free bytes at head
 *	@newtailroom: new free bytes at tail
 *	@gfp_mask: allocation priority
 *
 *	Make a copy of both an &sk_buff and its data and while doing so
 *	allocate additional space.
 *
 *	This is used when the caller wishes to modify the data and needs a
 *	private copy of the data to alter as well as more space for new fields.
 *	Returns %NULL on failure or the pointer to the buffer
 *	on success. The returned buffer has a reference count of 1.
 *
 *	You must pass %GFP_ATOMIC as the allocation priority if this function
 *	is called from an interrupt.
 */
struct sk_buff *skb_copy_expand(const struct sk_buff *skb,
				int newheadroom, int newtailroom,
				gfp_t gfp_mask)
{
	/*
	 *	Allocate the copy buffer
	 */
	struct sk_buff *n = __alloc_skb(newheadroom + skb->len + newtailroom,
					gfp_mask, skb_alloc_rx_flag(skb),
					NUMA_NO_NODE);
	int oldheadroom = skb_headroom(skb);
	int head_copy_len, head_copy_off;

	if (!n)
		return NULL;

	skb_reserve(n, newheadroom);

	/* Set the tail pointer and length */
	skb_put(n, skb->len);

	head_copy_len = oldheadroom;
	head_copy_off = 0;
	if (newheadroom <= head_copy_len)
		head_copy_len = newheadroom;
	else
		head_copy_off = newheadroom - head_copy_len;

	/* Copy the linear header and data. */
	if (skb_copy_bits(skb, -head_copy_len, n->head + head_copy_off,
			  skb->len + head_copy_len))
		BUG();

	copy_skb_header(n, skb);

	skb_headers_offset_update(n, newheadroom - oldheadroom);

#if defined (CONFIG_BCM_KF_BPM_BUF_TRACKING)
	KERN_GBPM_TRACK_SKB(skb, GBPM_VAL_COPY_SRC, 3);
#endif

	return n;
}
EXPORT_SYMBOL(skb_copy_expand);

/**
 *	skb_pad			-	zero pad the tail of an skb
 *	@skb: buffer to pad
 *	@pad: space to pad
 *
 *	Ensure that a buffer is followed by a padding area that is zero
 *	filled. Used by network drivers which may DMA or transfer data
 *	beyond the buffer end onto the wire.
 *
 *	May return error in out of memory cases. The skb is freed on error.
 */

int skb_pad(struct sk_buff *skb, int pad)
{
	int err;
	int ntail;

	/* If the skbuff is non linear tailroom is always zero.. */
	if (!skb_cloned(skb) && skb_tailroom(skb) >= pad) {
		memset(skb->data+skb->len, 0, pad);
		return 0;
	}

	ntail = skb->data_len + pad - (skb->end - skb->tail);
	if (likely(skb_cloned(skb) || ntail > 0)) {
		err = pskb_expand_head(skb, 0, ntail, GFP_ATOMIC);
		if (unlikely(err))
			goto free_skb;
	}

	/* FIXME: The use of this function with non-linear skb's really needs
	 * to be audited.
	 */
	err = skb_linearize(skb);
	if (unlikely(err))
		goto free_skb;

	memset(skb->data + skb->len, 0, pad);
	return 0;

free_skb:
	kfree_skb(skb);
	return err;
}
EXPORT_SYMBOL(skb_pad);

/**
 *	pskb_put - add data to the tail of a potentially fragmented buffer
 *	@skb: start of the buffer to use
 *	@tail: tail fragment of the buffer to use
 *	@len: amount of data to add
 *
 *	This function extends the used data area of the potentially
 *	fragmented buffer. @tail must be the last fragment of @skb -- or
 *	@skb itself. If this would exceed the total buffer size the kernel
 *	will panic. A pointer to the first byte of the extra data is
 *	returned.
 */

unsigned char *pskb_put(struct sk_buff *skb, struct sk_buff *tail, int len)
{
	if (tail != skb) {
		skb->data_len += len;
		skb->len += len;
	}
	return skb_put(tail, len);
}
EXPORT_SYMBOL_GPL(pskb_put);

/**
 *	skb_put - add data to a buffer
 *	@skb: buffer to use
 *	@len: amount of data to add
 *
 *	This function extends the used data area of the buffer. If this would
 *	exceed the total buffer size the kernel will panic. A pointer to the
 *	first byte of the extra data is returned.
 */
unsigned char *skb_put(struct sk_buff *skb, unsigned int len)
{
	unsigned char *tmp = skb_tail_pointer(skb);
	SKB_LINEAR_ASSERT(skb);
	skb->tail += len;
	skb->len  += len;
	if (unlikely(skb->tail > skb->end))
		skb_over_panic(skb, len, __builtin_return_address(0));
	return tmp;
}
EXPORT_SYMBOL(skb_put);

/**
 *	skb_push - add data to the start of a buffer
 *	@skb: buffer to use
 *	@len: amount of data to add
 *
 *	This function extends the used data area of the buffer at the buffer
 *	start. If this would exceed the total buffer headroom the kernel will
 *	panic. A pointer to the first byte of the extra data is returned.
 */
unsigned char *skb_push(struct sk_buff *skb, unsigned int len)
{
	skb->data -= len;
	skb->len  += len;
	if (unlikely(skb->data<skb->head))
		skb_under_panic(skb, len, __builtin_return_address(0));
	return skb->data;
}
EXPORT_SYMBOL(skb_push);

/**
 *	skb_pull - remove data from the start of a buffer
 *	@skb: buffer to use
 *	@len: amount of data to remove
 *
 *	This function removes data from the start of a buffer, returning
 *	the memory to the headroom. A pointer to the next data in the buffer
 *	is returned. Once the data has been pulled future pushes will overwrite
 *	the old data.
 */
unsigned char *skb_pull(struct sk_buff *skb, unsigned int len)
{
	return skb_pull_inline(skb, len);
}
EXPORT_SYMBOL(skb_pull);

/**
 *	skb_trim - remove end from a buffer
 *	@skb: buffer to alter
 *	@len: new length
 *
 *	Cut the length of a buffer down by removing data from the tail. If
 *	the buffer is already under the length specified it is not modified.
 *	The skb must be linear.
 */
void skb_trim(struct sk_buff *skb, unsigned int len)
{
	if (skb->len > len)
		__skb_trim(skb, len);
}
EXPORT_SYMBOL(skb_trim);

/* Trims skb to length len. It can change skb pointers.
 */

int ___pskb_trim(struct sk_buff *skb, unsigned int len)
{
	struct sk_buff **fragp;
	struct sk_buff *frag;
	int offset = skb_headlen(skb);
	int nfrags = skb_shinfo(skb)->nr_frags;
	int i;
	int err;

	if (skb_cloned(skb) &&
	    unlikely((err = pskb_expand_head(skb, 0, 0, GFP_ATOMIC))))
		return err;

	i = 0;
	if (offset >= len)
		goto drop_pages;

	for (; i < nfrags; i++) {
		int end = offset + skb_frag_size(&skb_shinfo(skb)->frags[i]);

		if (end < len) {
			offset = end;
			continue;
		}

		skb_frag_size_set(&skb_shinfo(skb)->frags[i++], len - offset);

drop_pages:
		skb_shinfo(skb)->nr_frags = i;

		for (; i < nfrags; i++)
			skb_frag_unref(skb, i);

		if (skb_has_frag_list(skb))
			skb_drop_fraglist(skb);
		goto done;
	}

	for (fragp = &skb_shinfo(skb)->frag_list; (frag = *fragp);
	     fragp = &frag->next) {
		int end = offset + frag->len;

		if (skb_shared(frag)) {
			struct sk_buff *nfrag;

			nfrag = skb_clone(frag, GFP_ATOMIC);
			if (unlikely(!nfrag))
				return -ENOMEM;

			nfrag->next = frag->next;
			consume_skb(frag);
			frag = nfrag;
			*fragp = frag;
		}

		if (end < len) {
			offset = end;
			continue;
		}

		if (end > len &&
		    unlikely((err = pskb_trim(frag, len - offset))))
			return err;

		if (frag->next)
			skb_drop_list(&frag->next);
		break;
	}

done:
	if (len > skb_headlen(skb)) {
		skb->data_len -= skb->len - len;
		skb->len       = len;
	} else {
		skb->len       = len;
		skb->data_len  = 0;
		skb_set_tail_pointer(skb, len);
	}

	return 0;
}
EXPORT_SYMBOL(___pskb_trim);

/**
 *	__pskb_pull_tail - advance tail of skb header
 *	@skb: buffer to reallocate
 *	@delta: number of bytes to advance tail
 *
 *	The function makes a sense only on a fragmented &sk_buff,
 *	it expands header moving its tail forward and copying necessary
 *	data from fragmented part.
 *
 *	&sk_buff MUST have reference count of 1.
 *
 *	Returns %NULL (and &sk_buff does not change) if pull failed
 *	or value of new tail of skb in the case of success.
 *
 *	All the pointers pointing into skb header may change and must be
 *	reloaded after call to this function.
 */

/* Moves tail of skb head forward, copying data from fragmented part,
 * when it is necessary.
 * 1. It may fail due to malloc failure.
 * 2. It may change skb pointers.
 *
 * It is pretty complicated. Luckily, it is called only in exceptional cases.
 */
unsigned char *__pskb_pull_tail(struct sk_buff *skb, int delta)
{
	/* If skb has not enough free space at tail, get new one
	 * plus 128 bytes for future expansions. If we have enough
	 * room at tail, reallocate without expansion only if skb is cloned.
	 */
	int i, k, eat = (skb->tail + delta) - skb->end;

	if (eat > 0 || skb_cloned(skb)) {
		if (pskb_expand_head(skb, 0, eat > 0 ? eat + 128 : 0,
				     GFP_ATOMIC))
			return NULL;
	}

	if (skb_copy_bits(skb, skb_headlen(skb), skb_tail_pointer(skb), delta))
		BUG();

	/* Optimization: no fragments, no reasons to preestimate
	 * size of pulled pages. Superb.
	 */
	if (!skb_has_frag_list(skb))
		goto pull_pages;

	/* Estimate size of pulled pages. */
	eat = delta;
	for (i = 0; i < skb_shinfo(skb)->nr_frags; i++) {
		int size = skb_frag_size(&skb_shinfo(skb)->frags[i]);

		if (size >= eat)
			goto pull_pages;
		eat -= size;
	}

	/* If we need update frag list, we are in troubles.
	 * Certainly, it possible to add an offset to skb data,
	 * but taking into account that pulling is expected to
	 * be very rare operation, it is worth to fight against
	 * further bloating skb head and crucify ourselves here instead.
	 * Pure masohism, indeed. 8)8)
	 */
	if (eat) {
		struct sk_buff *list = skb_shinfo(skb)->frag_list;
		struct sk_buff *clone = NULL;
		struct sk_buff *insp = NULL;

		do {
			BUG_ON(!list);

			if (list->len <= eat) {
				/* Eaten as whole. */
				eat -= list->len;
				list = list->next;
				insp = list;
			} else {
				/* Eaten partially. */

				if (skb_shared(list)) {
					/* Sucks! We need to fork list. :-( */
					clone = skb_clone(list, GFP_ATOMIC);
					if (!clone)
						return NULL;
					insp = list->next;
					list = clone;
				} else {
					/* This may be pulled without
					 * problems. */
					insp = list;
				}
				if (!pskb_pull(list, eat)) {
					kfree_skb(clone);
					return NULL;
				}
				break;
			}
		} while (eat);

		/* Free pulled out fragments. */
		while ((list = skb_shinfo(skb)->frag_list) != insp) {
			skb_shinfo(skb)->frag_list = list->next;
			kfree_skb(list);
		}
		/* And insert new clone at head. */
		if (clone) {
			clone->next = list;
			skb_shinfo(skb)->frag_list = clone;
		}
	}
	/* Success! Now we may commit changes to skb data. */

pull_pages:
	eat = delta;
	k = 0;
	for (i = 0; i < skb_shinfo(skb)->nr_frags; i++) {
		int size = skb_frag_size(&skb_shinfo(skb)->frags[i]);

		if (size <= eat) {
			skb_frag_unref(skb, i);
			eat -= size;
		} else {
			skb_shinfo(skb)->frags[k] = skb_shinfo(skb)->frags[i];
			if (eat) {
				skb_shinfo(skb)->frags[k].page_offset += eat;
				skb_frag_size_sub(&skb_shinfo(skb)->frags[k], eat);
				eat = 0;
			}
			k++;
		}
	}
	skb_shinfo(skb)->nr_frags = k;

	skb->tail     += delta;
	skb->data_len -= delta;

	return skb_tail_pointer(skb);
}
EXPORT_SYMBOL(__pskb_pull_tail);

/**
 *	skb_copy_bits - copy bits from skb to kernel buffer
 *	@skb: source skb
 *	@offset: offset in source
 *	@to: destination buffer
 *	@len: number of bytes to copy
 *
 *	Copy the specified number of bytes from the source skb to the
 *	destination buffer.
 *
 *	CAUTION ! :
 *		If its prototype is ever changed,
 *		check arch/{*}/net/{*}.S files,
 *		since it is called from BPF assembly code.
 */
int skb_copy_bits(const struct sk_buff *skb, int offset, void *to, int len)
{
	int start = skb_headlen(skb);
	struct sk_buff *frag_iter;
	int i, copy;

	if (offset > (int)skb->len - len)
		goto fault;

	/* Copy header. */
	if ((copy = start - offset) > 0) {
		if (copy > len)
			copy = len;
		skb_copy_from_linear_data_offset(skb, offset, to, copy);
		if ((len -= copy) == 0)
			return 0;
		offset += copy;
		to     += copy;
	}

	for (i = 0; i < skb_shinfo(skb)->nr_frags; i++) {
		int end;
		skb_frag_t *f = &skb_shinfo(skb)->frags[i];

		WARN_ON(start > offset + len);

		end = start + skb_frag_size(f);
		if ((copy = end - offset) > 0) {
			u8 *vaddr;

			if (copy > len)
				copy = len;

			vaddr = kmap_atomic(skb_frag_page(f));
			memcpy(to,
			       vaddr + f->page_offset + offset - start,
			       copy);
			kunmap_atomic(vaddr);

			if ((len -= copy) == 0)
				return 0;
			offset += copy;
			to     += copy;
		}
		start = end;
	}

	skb_walk_frags(skb, frag_iter) {
		int end;

		WARN_ON(start > offset + len);

		end = start + frag_iter->len;
		if ((copy = end - offset) > 0) {
			if (copy > len)
				copy = len;
			if (skb_copy_bits(frag_iter, offset - start, to, copy))
				goto fault;
			if ((len -= copy) == 0)
				return 0;
			offset += copy;
			to     += copy;
		}
		start = end;
	}

	if (!len)
		return 0;

fault:
	return -EFAULT;
}
EXPORT_SYMBOL(skb_copy_bits);

/*
 * Callback from splice_to_pipe(), if we need to release some pages
 * at the end of the spd in case we error'ed out in filling the pipe.
 */
static void sock_spd_release(struct splice_pipe_desc *spd, unsigned int i)
{
	put_page(spd->pages[i]);
}

static struct page *linear_to_page(struct page *page, unsigned int *len,
				   unsigned int *offset,
				   struct sock *sk)
{
	struct page_frag *pfrag = sk_page_frag(sk);

	if (!sk_page_frag_refill(sk, pfrag))
		return NULL;

	*len = min_t(unsigned int, *len, pfrag->size - pfrag->offset);

	memcpy(page_address(pfrag->page) + pfrag->offset,
	       page_address(page) + *offset, *len);
	*offset = pfrag->offset;
	pfrag->offset += *len;

	return pfrag->page;
}

static bool spd_can_coalesce(const struct splice_pipe_desc *spd,
			     struct page *page,
			     unsigned int offset)
{
	return	spd->nr_pages &&
		spd->pages[spd->nr_pages - 1] == page &&
		(spd->partial[spd->nr_pages - 1].offset +
		 spd->partial[spd->nr_pages - 1].len == offset);
}

/*
 * Fill page/offset/length into spd, if it can hold more pages.
 */
static bool spd_fill_page(struct splice_pipe_desc *spd,
			  struct pipe_inode_info *pipe, struct page *page,
			  unsigned int *len, unsigned int offset,
			  bool linear,
			  struct sock *sk)
{
	if (unlikely(spd->nr_pages == MAX_SKB_FRAGS))
		return true;

	if (linear) {
		page = linear_to_page(page, len, &offset, sk);
		if (!page)
			return true;
	}
	if (spd_can_coalesce(spd, page, offset)) {
		spd->partial[spd->nr_pages - 1].len += *len;
		return false;
	}
	get_page(page);
	spd->pages[spd->nr_pages] = page;
	spd->partial[spd->nr_pages].len = *len;
	spd->partial[spd->nr_pages].offset = offset;
	spd->nr_pages++;

	return false;
}

static bool __splice_segment(struct page *page, unsigned int poff,
			     unsigned int plen, unsigned int *off,
			     unsigned int *len,
			     struct splice_pipe_desc *spd, bool linear,
			     struct sock *sk,
			     struct pipe_inode_info *pipe)
{
	if (!*len)
		return true;

	/* skip this segment if already processed */
	if (*off >= plen) {
		*off -= plen;
		return false;
	}

	/* ignore any bits we already processed */
	poff += *off;
	plen -= *off;
	*off = 0;

	do {
		unsigned int flen = min(*len, plen);

		if (spd_fill_page(spd, pipe, page, &flen, poff,
				  linear, sk))
			return true;
		poff += flen;
		plen -= flen;
		*len -= flen;
	} while (*len && plen);

	return false;
}

/*
 * Map linear and fragment data from the skb to spd. It reports true if the
 * pipe is full or if we already spliced the requested length.
 */
static bool __skb_splice_bits(struct sk_buff *skb, struct pipe_inode_info *pipe,
			      unsigned int *offset, unsigned int *len,
			      struct splice_pipe_desc *spd, struct sock *sk)
{
	int seg;

	/* map the linear part :
	 * If skb->head_frag is set, this 'linear' part is backed by a
	 * fragment, and if the head is not shared with any clones then
	 * we can avoid a copy since we own the head portion of this page.
	 */
	if (__splice_segment(virt_to_page(skb->data),
			     (unsigned long) skb->data & (PAGE_SIZE - 1),
			     skb_headlen(skb),
			     offset, len, spd,
			     skb_head_is_locked(skb),
			     sk, pipe))
		return true;

	/*
	 * then map the fragments
	 */
	for (seg = 0; seg < skb_shinfo(skb)->nr_frags; seg++) {
		const skb_frag_t *f = &skb_shinfo(skb)->frags[seg];

		if (__splice_segment(skb_frag_page(f),
				     f->page_offset, skb_frag_size(f),
				     offset, len, spd, false, sk, pipe))
			return true;
	}

	return false;
}

/*
 * Map data from the skb to a pipe. Should handle both the linear part,
 * the fragments, and the frag list. It does NOT handle frag lists within
 * the frag list, if such a thing exists. We'd probably need to recurse to
 * handle that cleanly.
 */
int skb_splice_bits(struct sk_buff *skb, unsigned int offset,
		    struct pipe_inode_info *pipe, unsigned int tlen,
		    unsigned int flags)
{
	struct partial_page partial[MAX_SKB_FRAGS];
	struct page *pages[MAX_SKB_FRAGS];
	struct splice_pipe_desc spd = {
		.pages = pages,
		.partial = partial,
		.nr_pages_max = MAX_SKB_FRAGS,
		.flags = flags,
		.ops = &nosteal_pipe_buf_ops,
		.spd_release = sock_spd_release,
	};
	struct sk_buff *frag_iter;
	struct sock *sk = skb->sk;
	int ret = 0;

	/*
	 * __skb_splice_bits() only fails if the output has no room left,
	 * so no point in going over the frag_list for the error case.
	 */
	if (__skb_splice_bits(skb, pipe, &offset, &tlen, &spd, sk))
		goto done;
	else if (!tlen)
		goto done;

	/*
	 * now see if we have a frag_list to map
	 */
	skb_walk_frags(skb, frag_iter) {
		if (!tlen)
			break;
		if (__skb_splice_bits(frag_iter, pipe, &offset, &tlen, &spd, sk))
			break;
	}

done:
	if (spd.nr_pages) {
		/*
		 * Drop the socket lock, otherwise we have reverse
		 * locking dependencies between sk_lock and i_mutex
		 * here as compared to sendfile(). We enter here
		 * with the socket lock held, and splice_to_pipe() will
		 * grab the pipe inode lock. For sendfile() emulation,
		 * we call into ->sendpage() with the i_mutex lock held
		 * and networking will grab the socket lock.
		 */
		release_sock(sk);
		ret = splice_to_pipe(pipe, &spd);
		lock_sock(sk);
	}

	return ret;
}

/**
 *	skb_store_bits - store bits from kernel buffer to skb
 *	@skb: destination buffer
 *	@offset: offset in destination
 *	@from: source buffer
 *	@len: number of bytes to copy
 *
 *	Copy the specified number of bytes from the source buffer to the
 *	destination skb.  This function handles all the messy bits of
 *	traversing fragment lists and such.
 */

int skb_store_bits(struct sk_buff *skb, int offset, const void *from, int len)
{
	int start = skb_headlen(skb);
	struct sk_buff *frag_iter;
	int i, copy;

	if (offset > (int)skb->len - len)
		goto fault;

	if ((copy = start - offset) > 0) {
		if (copy > len)
			copy = len;
		skb_copy_to_linear_data_offset(skb, offset, from, copy);
		if ((len -= copy) == 0)
			return 0;
		offset += copy;
		from += copy;
	}

	for (i = 0; i < skb_shinfo(skb)->nr_frags; i++) {
		skb_frag_t *frag = &skb_shinfo(skb)->frags[i];
		int end;

		WARN_ON(start > offset + len);

		end = start + skb_frag_size(frag);
		if ((copy = end - offset) > 0) {
			u8 *vaddr;

			if (copy > len)
				copy = len;

			vaddr = kmap_atomic(skb_frag_page(frag));
			memcpy(vaddr + frag->page_offset + offset - start,
			       from, copy);
			kunmap_atomic(vaddr);

			if ((len -= copy) == 0)
				return 0;
			offset += copy;
			from += copy;
		}
		start = end;
	}

	skb_walk_frags(skb, frag_iter) {
		int end;

		WARN_ON(start > offset + len);

		end = start + frag_iter->len;
		if ((copy = end - offset) > 0) {
			if (copy > len)
				copy = len;
			if (skb_store_bits(frag_iter, offset - start,
					   from, copy))
				goto fault;
			if ((len -= copy) == 0)
				return 0;
			offset += copy;
			from += copy;
		}
		start = end;
	}
	if (!len)
		return 0;

fault:
	return -EFAULT;
}
EXPORT_SYMBOL(skb_store_bits);

/* Checksum skb data. */
__wsum __skb_checksum(const struct sk_buff *skb, int offset, int len,
		      __wsum csum, const struct skb_checksum_ops *ops)
{
	int start = skb_headlen(skb);
	int i, copy = start - offset;
	struct sk_buff *frag_iter;
	int pos = 0;

	/* Checksum header. */
	if (copy > 0) {
		if (copy > len)
			copy = len;
		csum = ops->update(skb->data + offset, copy, csum);
		if ((len -= copy) == 0)
			return csum;
		offset += copy;
		pos	= copy;
	}

	for (i = 0; i < skb_shinfo(skb)->nr_frags; i++) {
		int end;
		skb_frag_t *frag = &skb_shinfo(skb)->frags[i];

		WARN_ON(start > offset + len);

		end = start + skb_frag_size(frag);
		if ((copy = end - offset) > 0) {
			__wsum csum2;
			u8 *vaddr;

			if (copy > len)
				copy = len;
			vaddr = kmap_atomic(skb_frag_page(frag));
			csum2 = ops->update(vaddr + frag->page_offset +
					    offset - start, copy, 0);
			kunmap_atomic(vaddr);
			csum = ops->combine(csum, csum2, pos, copy);
			if (!(len -= copy))
				return csum;
			offset += copy;
			pos    += copy;
		}
		start = end;
	}

	skb_walk_frags(skb, frag_iter) {
		int end;

		WARN_ON(start > offset + len);

		end = start + frag_iter->len;
		if ((copy = end - offset) > 0) {
			__wsum csum2;
			if (copy > len)
				copy = len;
			csum2 = __skb_checksum(frag_iter, offset - start,
					       copy, 0, ops);
			csum = ops->combine(csum, csum2, pos, copy);
			if ((len -= copy) == 0)
				return csum;
			offset += copy;
			pos    += copy;
		}
		start = end;
	}
	BUG_ON(len);

	return csum;
}
EXPORT_SYMBOL(__skb_checksum);

__wsum skb_checksum(const struct sk_buff *skb, int offset,
		    int len, __wsum csum)
{
	const struct skb_checksum_ops ops = {
		.update  = csum_partial_ext,
		.combine = csum_block_add_ext,
	};

	return __skb_checksum(skb, offset, len, csum, &ops);
}
EXPORT_SYMBOL(skb_checksum);

/* Both of above in one bottle. */

__wsum skb_copy_and_csum_bits(const struct sk_buff *skb, int offset,
				    u8 *to, int len, __wsum csum)
{
	int start = skb_headlen(skb);
	int i, copy = start - offset;
	struct sk_buff *frag_iter;
	int pos = 0;

	/* Copy header. */
	if (copy > 0) {
		if (copy > len)
			copy = len;
		csum = csum_partial_copy_nocheck(skb->data + offset, to,
						 copy, csum);
		if ((len -= copy) == 0)
			return csum;
		offset += copy;
		to     += copy;
		pos	= copy;
	}

	for (i = 0; i < skb_shinfo(skb)->nr_frags; i++) {
		int end;

		WARN_ON(start > offset + len);

		end = start + skb_frag_size(&skb_shinfo(skb)->frags[i]);
		if ((copy = end - offset) > 0) {
			__wsum csum2;
			u8 *vaddr;
			skb_frag_t *frag = &skb_shinfo(skb)->frags[i];

			if (copy > len)
				copy = len;
			vaddr = kmap_atomic(skb_frag_page(frag));
			csum2 = csum_partial_copy_nocheck(vaddr +
							  frag->page_offset +
							  offset - start, to,
							  copy, 0);
			kunmap_atomic(vaddr);
			csum = csum_block_add(csum, csum2, pos);
			if (!(len -= copy))
				return csum;
			offset += copy;
			to     += copy;
			pos    += copy;
		}
		start = end;
	}

	skb_walk_frags(skb, frag_iter) {
		__wsum csum2;
		int end;

		WARN_ON(start > offset + len);

		end = start + frag_iter->len;
		if ((copy = end - offset) > 0) {
			if (copy > len)
				copy = len;
			csum2 = skb_copy_and_csum_bits(frag_iter,
						       offset - start,
						       to, copy, 0);
			csum = csum_block_add(csum, csum2, pos);
			if ((len -= copy) == 0)
				return csum;
			offset += copy;
			to     += copy;
			pos    += copy;
		}
		start = end;
	}
	BUG_ON(len);
	return csum;
}
EXPORT_SYMBOL(skb_copy_and_csum_bits);

 /**
 *	skb_zerocopy_headlen - Calculate headroom needed for skb_zerocopy()
 *	@from: source buffer
 *
 *	Calculates the amount of linear headroom needed in the 'to' skb passed
 *	into skb_zerocopy().
 */
unsigned int
skb_zerocopy_headlen(const struct sk_buff *from)
{
	unsigned int hlen = 0;

	if (!from->head_frag ||
	    skb_headlen(from) < L1_CACHE_BYTES ||
	    skb_shinfo(from)->nr_frags >= MAX_SKB_FRAGS)
		hlen = skb_headlen(from);

	if (skb_has_frag_list(from))
		hlen = from->len;

	return hlen;
}
EXPORT_SYMBOL_GPL(skb_zerocopy_headlen);

/**
 *	skb_zerocopy - Zero copy skb to skb
 *	@to: destination buffer
 *	@from: source buffer
 *	@len: number of bytes to copy from source buffer
 *	@hlen: size of linear headroom in destination buffer
 *
 *	Copies up to `len` bytes from `from` to `to` by creating references
 *	to the frags in the source buffer.
 *
 *	The `hlen` as calculated by skb_zerocopy_headlen() specifies the
 *	headroom in the `to` buffer.
 *
 *	Return value:
 *	0: everything is OK
 *	-ENOMEM: couldn't orphan frags of @from due to lack of memory
 *	-EFAULT: skb_copy_bits() found some problem with skb geometry
 */
int
skb_zerocopy(struct sk_buff *to, struct sk_buff *from, int len, int hlen)
{
	int i, j = 0;
	int plen = 0; /* length of skb->head fragment */
	int ret;
	struct page *page;
	unsigned int offset;

	BUG_ON(!from->head_frag && !hlen);

	/* dont bother with small payloads */
	if (len <= skb_tailroom(to))
		return skb_copy_bits(from, 0, skb_put(to, len), len);

	if (hlen) {
		ret = skb_copy_bits(from, 0, skb_put(to, hlen), hlen);
		if (unlikely(ret))
			return ret;
		len -= hlen;
	} else {
		plen = min_t(int, skb_headlen(from), len);
		if (plen) {
			page = virt_to_head_page(from->head);
			offset = from->data - (unsigned char *)page_address(page);
			__skb_fill_page_desc(to, 0, page, offset, plen);
			get_page(page);
			j = 1;
			len -= plen;
		}
	}

	to->truesize += len + plen;
	to->len += len + plen;
	to->data_len += len + plen;

	if (unlikely(skb_orphan_frags(from, GFP_ATOMIC))) {
		skb_tx_error(from);
		return -ENOMEM;
	}

	for (i = 0; i < skb_shinfo(from)->nr_frags; i++) {
		if (!len)
			break;
		skb_shinfo(to)->frags[j] = skb_shinfo(from)->frags[i];
		skb_shinfo(to)->frags[j].size = min_t(int, skb_shinfo(to)->frags[j].size, len);
		len -= skb_shinfo(to)->frags[j].size;
		skb_frag_ref(to, j);
		j++;
	}
	skb_shinfo(to)->nr_frags = j;

	return 0;
}
EXPORT_SYMBOL_GPL(skb_zerocopy);

void skb_copy_and_csum_dev(const struct sk_buff *skb, u8 *to)
{
	__wsum csum;
	long csstart;

	if (skb->ip_summed == CHECKSUM_PARTIAL)
		csstart = skb_checksum_start_offset(skb);
	else
		csstart = skb_headlen(skb);

	BUG_ON(csstart > skb_headlen(skb));

	skb_copy_from_linear_data(skb, to, csstart);

	csum = 0;
	if (csstart != skb->len)
		csum = skb_copy_and_csum_bits(skb, csstart, to + csstart,
					      skb->len - csstart, 0);

	if (skb->ip_summed == CHECKSUM_PARTIAL) {
		long csstuff = csstart + skb->csum_offset;

		*((__sum16 *)(to + csstuff)) = csum_fold(csum);
	}
}
EXPORT_SYMBOL(skb_copy_and_csum_dev);

/**
 *	skb_dequeue - remove from the head of the queue
 *	@list: list to dequeue from
 *
 *	Remove the head of the list. The list lock is taken so the function
 *	may be used safely with other locking list functions. The head item is
 *	returned or %NULL if the list is empty.
 */

struct sk_buff *skb_dequeue(struct sk_buff_head *list)
{
	unsigned long flags;
	struct sk_buff *result;

	spin_lock_irqsave(&list->lock, flags);
	result = __skb_dequeue(list);
	spin_unlock_irqrestore(&list->lock, flags);
	return result;
}
EXPORT_SYMBOL(skb_dequeue);

/**
 *	skb_dequeue_tail - remove from the tail of the queue
 *	@list: list to dequeue from
 *
 *	Remove the tail of the list. The list lock is taken so the function
 *	may be used safely with other locking list functions. The tail item is
 *	returned or %NULL if the list is empty.
 */
struct sk_buff *skb_dequeue_tail(struct sk_buff_head *list)
{
	unsigned long flags;
	struct sk_buff *result;

	spin_lock_irqsave(&list->lock, flags);
	result = __skb_dequeue_tail(list);
	spin_unlock_irqrestore(&list->lock, flags);
	return result;
}
EXPORT_SYMBOL(skb_dequeue_tail);

/**
 *	skb_queue_purge - empty a list
 *	@list: list to empty
 *
 *	Delete all buffers on an &sk_buff list. Each buffer is removed from
 *	the list and one reference dropped. This function takes the list
 *	lock and is atomic with respect to other list locking functions.
 */
void skb_queue_purge(struct sk_buff_head *list)
{
	struct sk_buff *skb;
	while ((skb = skb_dequeue(list)) != NULL)
		kfree_skb(skb);
}
EXPORT_SYMBOL(skb_queue_purge);

/**
 *	skb_queue_head - queue a buffer at the list head
 *	@list: list to use
 *	@newsk: buffer to queue
 *
 *	Queue a buffer at the start of the list. This function takes the
 *	list lock and can be used safely with other locking &sk_buff functions
 *	safely.
 *
 *	A buffer cannot be placed on two lists at the same time.
 */
void skb_queue_head(struct sk_buff_head *list, struct sk_buff *newsk)
{
	unsigned long flags;

	spin_lock_irqsave(&list->lock, flags);
	__skb_queue_head(list, newsk);
	spin_unlock_irqrestore(&list->lock, flags);
}
EXPORT_SYMBOL(skb_queue_head);

/**
 *	skb_queue_tail - queue a buffer at the list tail
 *	@list: list to use
 *	@newsk: buffer to queue
 *
 *	Queue a buffer at the tail of the list. This function takes the
 *	list lock and can be used safely with other locking &sk_buff functions
 *	safely.
 *
 *	A buffer cannot be placed on two lists at the same time.
 */
void skb_queue_tail(struct sk_buff_head *list, struct sk_buff *newsk)
{
	unsigned long flags;

	spin_lock_irqsave(&list->lock, flags);
	__skb_queue_tail(list, newsk);
	spin_unlock_irqrestore(&list->lock, flags);
}
EXPORT_SYMBOL(skb_queue_tail);

/**
 *	skb_unlink	-	remove a buffer from a list
 *	@skb: buffer to remove
 *	@list: list to use
 *
 *	Remove a packet from a list. The list locks are taken and this
 *	function is atomic with respect to other list locked calls
 *
 *	You must know what list the SKB is on.
 */
void skb_unlink(struct sk_buff *skb, struct sk_buff_head *list)
{
	unsigned long flags;

	spin_lock_irqsave(&list->lock, flags);
	__skb_unlink(skb, list);
	spin_unlock_irqrestore(&list->lock, flags);
}
EXPORT_SYMBOL(skb_unlink);

/**
 *	skb_append	-	append a buffer
 *	@old: buffer to insert after
 *	@newsk: buffer to insert
 *	@list: list to use
 *
 *	Place a packet after a given packet in a list. The list locks are taken
 *	and this function is atomic with respect to other list locked calls.
 *	A buffer cannot be placed on two lists at the same time.
 */
void skb_append(struct sk_buff *old, struct sk_buff *newsk, struct sk_buff_head *list)
{
	unsigned long flags;

	spin_lock_irqsave(&list->lock, flags);
	__skb_queue_after(list, old, newsk);
	spin_unlock_irqrestore(&list->lock, flags);
}
EXPORT_SYMBOL(skb_append);

/**
 *	skb_insert	-	insert a buffer
 *	@old: buffer to insert before
 *	@newsk: buffer to insert
 *	@list: list to use
 *
 *	Place a packet before a given packet in a list. The list locks are
 * 	taken and this function is atomic with respect to other list locked
 *	calls.
 *
 *	A buffer cannot be placed on two lists at the same time.
 */
void skb_insert(struct sk_buff *old, struct sk_buff *newsk, struct sk_buff_head *list)
{
	unsigned long flags;

	spin_lock_irqsave(&list->lock, flags);
	__skb_insert(newsk, old->prev, old, list);
	spin_unlock_irqrestore(&list->lock, flags);
}
EXPORT_SYMBOL(skb_insert);

static inline void skb_split_inside_header(struct sk_buff *skb,
					   struct sk_buff* skb1,
					   const u32 len, const int pos)
{
	int i;

	skb_copy_from_linear_data_offset(skb, len, skb_put(skb1, pos - len),
					 pos - len);
	/* And move data appendix as is. */
	for (i = 0; i < skb_shinfo(skb)->nr_frags; i++)
		skb_shinfo(skb1)->frags[i] = skb_shinfo(skb)->frags[i];

	skb_shinfo(skb1)->nr_frags = skb_shinfo(skb)->nr_frags;
	skb_shinfo(skb)->nr_frags  = 0;
	skb1->data_len		   = skb->data_len;
	skb1->len		   += skb1->data_len;
	skb->data_len		   = 0;
	skb->len		   = len;
	skb_set_tail_pointer(skb, len);
}

static inline void skb_split_no_header(struct sk_buff *skb,
				       struct sk_buff* skb1,
				       const u32 len, int pos)
{
	int i, k = 0;
	const int nfrags = skb_shinfo(skb)->nr_frags;

	skb_shinfo(skb)->nr_frags = 0;
	skb1->len		  = skb1->data_len = skb->len - len;
	skb->len		  = len;
	skb->data_len		  = len - pos;

	for (i = 0; i < nfrags; i++) {
		int size = skb_frag_size(&skb_shinfo(skb)->frags[i]);

		if (pos + size > len) {
			skb_shinfo(skb1)->frags[k] = skb_shinfo(skb)->frags[i];

			if (pos < len) {
				/* Split frag.
				 * We have two variants in this case:
				 * 1. Move all the frag to the second
				 *    part, if it is possible. F.e.
				 *    this approach is mandatory for TUX,
				 *    where splitting is expensive.
				 * 2. Split is accurately. We make this.
				 */
				skb_frag_ref(skb, i);
				skb_shinfo(skb1)->frags[0].page_offset += len - pos;
				skb_frag_size_sub(&skb_shinfo(skb1)->frags[0], len - pos);
				skb_frag_size_set(&skb_shinfo(skb)->frags[i], len - pos);
				skb_shinfo(skb)->nr_frags++;
			}
			k++;
		} else
			skb_shinfo(skb)->nr_frags++;
		pos += size;
	}
	skb_shinfo(skb1)->nr_frags = k;
}

/**
 * skb_split - Split fragmented skb to two parts at length len.
 * @skb: the buffer to split
 * @skb1: the buffer to receive the second part
 * @len: new length for skb
 */
void skb_split(struct sk_buff *skb, struct sk_buff *skb1, const u32 len)
{
	int pos = skb_headlen(skb);

	skb_shinfo(skb1)->tx_flags |= skb_shinfo(skb)->tx_flags &
				      SKBTX_SHARED_FRAG;
	if (len < pos)	/* Split line is inside header. */
		skb_split_inside_header(skb, skb1, len, pos);
	else		/* Second chunk has no header, nothing to copy. */
		skb_split_no_header(skb, skb1, len, pos);
}
EXPORT_SYMBOL(skb_split);

/* Shifting from/to a cloned skb is a no-go.
 *
 * Caller cannot keep skb_shinfo related pointers past calling here!
 */
static int skb_prepare_for_shift(struct sk_buff *skb)
{
	return skb_cloned(skb) && pskb_expand_head(skb, 0, 0, GFP_ATOMIC);
}

/**
 * skb_shift - Shifts paged data partially from skb to another
 * @tgt: buffer into which tail data gets added
 * @skb: buffer from which the paged data comes from
 * @shiftlen: shift up to this many bytes
 *
 * Attempts to shift up to shiftlen worth of bytes, which may be less than
 * the length of the skb, from skb to tgt. Returns number bytes shifted.
 * It's up to caller to free skb if everything was shifted.
 *
 * If @tgt runs out of frags, the whole operation is aborted.
 *
 * Skb cannot include anything else but paged data while tgt is allowed
 * to have non-paged data as well.
 *
 * TODO: full sized shift could be optimized but that would need
 * specialized skb free'er to handle frags without up-to-date nr_frags.
 */
int skb_shift(struct sk_buff *tgt, struct sk_buff *skb, int shiftlen)
{
	int from, to, merge, todo;
	struct skb_frag_struct *fragfrom, *fragto;

	BUG_ON(shiftlen > skb->len);
	BUG_ON(skb_headlen(skb));	/* Would corrupt stream */

	todo = shiftlen;
	from = 0;
	to = skb_shinfo(tgt)->nr_frags;
	fragfrom = &skb_shinfo(skb)->frags[from];

	/* Actual merge is delayed until the point when we know we can
	 * commit all, so that we don't have to undo partial changes
	 */
	if (!to ||
	    !skb_can_coalesce(tgt, to, skb_frag_page(fragfrom),
			      fragfrom->page_offset)) {
		merge = -1;
	} else {
		merge = to - 1;

		todo -= skb_frag_size(fragfrom);
		if (todo < 0) {
			if (skb_prepare_for_shift(skb) ||
			    skb_prepare_for_shift(tgt))
				return 0;

			/* All previous frag pointers might be stale! */
			fragfrom = &skb_shinfo(skb)->frags[from];
			fragto = &skb_shinfo(tgt)->frags[merge];

			skb_frag_size_add(fragto, shiftlen);
			skb_frag_size_sub(fragfrom, shiftlen);
			fragfrom->page_offset += shiftlen;

			goto onlymerged;
		}

		from++;
	}

	/* Skip full, not-fitting skb to avoid expensive operations */
	if ((shiftlen == skb->len) &&
	    (skb_shinfo(skb)->nr_frags - from) > (MAX_SKB_FRAGS - to))
		return 0;

	if (skb_prepare_for_shift(skb) || skb_prepare_for_shift(tgt))
		return 0;

	while ((todo > 0) && (from < skb_shinfo(skb)->nr_frags)) {
		if (to == MAX_SKB_FRAGS)
			return 0;

		fragfrom = &skb_shinfo(skb)->frags[from];
		fragto = &skb_shinfo(tgt)->frags[to];

		if (todo >= skb_frag_size(fragfrom)) {
			*fragto = *fragfrom;
			todo -= skb_frag_size(fragfrom);
			from++;
			to++;

		} else {
			__skb_frag_ref(fragfrom);
			fragto->page = fragfrom->page;
			fragto->page_offset = fragfrom->page_offset;
			skb_frag_size_set(fragto, todo);

			fragfrom->page_offset += todo;
			skb_frag_size_sub(fragfrom, todo);
			todo = 0;

			to++;
			break;
		}
	}

	/* Ready to "commit" this state change to tgt */
	skb_shinfo(tgt)->nr_frags = to;

	if (merge >= 0) {
		fragfrom = &skb_shinfo(skb)->frags[0];
		fragto = &skb_shinfo(tgt)->frags[merge];

		skb_frag_size_add(fragto, skb_frag_size(fragfrom));
		__skb_frag_unref(fragfrom);
	}

	/* Reposition in the original skb */
	to = 0;
	while (from < skb_shinfo(skb)->nr_frags)
		skb_shinfo(skb)->frags[to++] = skb_shinfo(skb)->frags[from++];
	skb_shinfo(skb)->nr_frags = to;

	BUG_ON(todo > 0 && !skb_shinfo(skb)->nr_frags);

onlymerged:
	/* Most likely the tgt won't ever need its checksum anymore, skb on
	 * the other hand might need it if it needs to be resent
	 */
	tgt->ip_summed = CHECKSUM_PARTIAL;
	skb->ip_summed = CHECKSUM_PARTIAL;

	/* Yak, is it really working this way? Some helper please? */
	skb->len -= shiftlen;
	skb->data_len -= shiftlen;
	skb->truesize -= shiftlen;
	tgt->len += shiftlen;
	tgt->data_len += shiftlen;
	tgt->truesize += shiftlen;

	return shiftlen;
}

/**
 * skb_prepare_seq_read - Prepare a sequential read of skb data
 * @skb: the buffer to read
 * @from: lower offset of data to be read
 * @to: upper offset of data to be read
 * @st: state variable
 *
 * Initializes the specified state variable. Must be called before
 * invoking skb_seq_read() for the first time.
 */
void skb_prepare_seq_read(struct sk_buff *skb, unsigned int from,
			  unsigned int to, struct skb_seq_state *st)
{
	st->lower_offset = from;
	st->upper_offset = to;
	st->root_skb = st->cur_skb = skb;
	st->frag_idx = st->stepped_offset = 0;
	st->frag_data = NULL;
}
EXPORT_SYMBOL(skb_prepare_seq_read);

/**
 * skb_seq_read - Sequentially read skb data
 * @consumed: number of bytes consumed by the caller so far
 * @data: destination pointer for data to be returned
 * @st: state variable
 *
 * Reads a block of skb data at @consumed relative to the
 * lower offset specified to skb_prepare_seq_read(). Assigns
 * the head of the data block to @data and returns the length
 * of the block or 0 if the end of the skb data or the upper
 * offset has been reached.
 *
 * The caller is not required to consume all of the data
 * returned, i.e. @consumed is typically set to the number
 * of bytes already consumed and the next call to
 * skb_seq_read() will return the remaining part of the block.
 *
 * Note 1: The size of each block of data returned can be arbitrary,
 *       this limitation is the cost for zerocopy sequential
 *       reads of potentially non linear data.
 *
 * Note 2: Fragment lists within fragments are not implemented
 *       at the moment, state->root_skb could be replaced with
 *       a stack for this purpose.
 */
unsigned int skb_seq_read(unsigned int consumed, const u8 **data,
			  struct skb_seq_state *st)
{
	unsigned int block_limit, abs_offset = consumed + st->lower_offset;
	skb_frag_t *frag;

	if (unlikely(abs_offset >= st->upper_offset)) {
		if (st->frag_data) {
			kunmap_atomic(st->frag_data);
			st->frag_data = NULL;
		}
		return 0;
	}

next_skb:
	block_limit = skb_headlen(st->cur_skb) + st->stepped_offset;

	if (abs_offset < block_limit && !st->frag_data) {
		*data = st->cur_skb->data + (abs_offset - st->stepped_offset);
		return block_limit - abs_offset;
	}

	if (st->frag_idx == 0 && !st->frag_data)
		st->stepped_offset += skb_headlen(st->cur_skb);

	while (st->frag_idx < skb_shinfo(st->cur_skb)->nr_frags) {
		frag = &skb_shinfo(st->cur_skb)->frags[st->frag_idx];
		block_limit = skb_frag_size(frag) + st->stepped_offset;

		if (abs_offset < block_limit) {
			if (!st->frag_data)
				st->frag_data = kmap_atomic(skb_frag_page(frag));

			*data = (u8 *) st->frag_data + frag->page_offset +
				(abs_offset - st->stepped_offset);

			return block_limit - abs_offset;
		}

		if (st->frag_data) {
			kunmap_atomic(st->frag_data);
			st->frag_data = NULL;
		}

		st->frag_idx++;
		st->stepped_offset += skb_frag_size(frag);
	}

	if (st->frag_data) {
		kunmap_atomic(st->frag_data);
		st->frag_data = NULL;
	}

	if (st->root_skb == st->cur_skb && skb_has_frag_list(st->root_skb)) {
		st->cur_skb = skb_shinfo(st->root_skb)->frag_list;
		st->frag_idx = 0;
		goto next_skb;
	} else if (st->cur_skb->next) {
		st->cur_skb = st->cur_skb->next;
		st->frag_idx = 0;
		goto next_skb;
	}

	return 0;
}
EXPORT_SYMBOL(skb_seq_read);

/**
 * skb_abort_seq_read - Abort a sequential read of skb data
 * @st: state variable
 *
 * Must be called if skb_seq_read() was not called until it
 * returned 0.
 */
void skb_abort_seq_read(struct skb_seq_state *st)
{
	if (st->frag_data)
		kunmap_atomic(st->frag_data);
}
EXPORT_SYMBOL(skb_abort_seq_read);

#define TS_SKB_CB(state)	((struct skb_seq_state *) &((state)->cb))

static unsigned int skb_ts_get_next_block(unsigned int offset, const u8 **text,
					  struct ts_config *conf,
					  struct ts_state *state)
{
	return skb_seq_read(offset, text, TS_SKB_CB(state));
}

static void skb_ts_finish(struct ts_config *conf, struct ts_state *state)
{
	skb_abort_seq_read(TS_SKB_CB(state));
}

/**
 * skb_find_text - Find a text pattern in skb data
 * @skb: the buffer to look in
 * @from: search offset
 * @to: search limit
 * @config: textsearch configuration
 *
 * Finds a pattern in the skb data according to the specified
 * textsearch configuration. Use textsearch_next() to retrieve
 * subsequent occurrences of the pattern. Returns the offset
 * to the first occurrence or UINT_MAX if no match was found.
 */
unsigned int skb_find_text(struct sk_buff *skb, unsigned int from,
			   unsigned int to, struct ts_config *config)
{
	struct ts_state state;
	unsigned int ret;

	config->get_next_block = skb_ts_get_next_block;
	config->finish = skb_ts_finish;

	skb_prepare_seq_read(skb, from, to, TS_SKB_CB(&state));

	ret = textsearch_find(config, &state);
	return (ret <= to - from ? ret : UINT_MAX);
}
EXPORT_SYMBOL(skb_find_text);

/**
 * skb_append_datato_frags - append the user data to a skb
 * @sk: sock  structure
 * @skb: skb structure to be appended with user data.
 * @getfrag: call back function to be used for getting the user data
 * @from: pointer to user message iov
 * @length: length of the iov message
 *
 * Description: This procedure append the user data in the fragment part
 * of the skb if any page alloc fails user this procedure returns  -ENOMEM
 */
int skb_append_datato_frags(struct sock *sk, struct sk_buff *skb,
			int (*getfrag)(void *from, char *to, int offset,
					int len, int odd, struct sk_buff *skb),
			void *from, int length)
{
	int frg_cnt = skb_shinfo(skb)->nr_frags;
	int copy;
	int offset = 0;
	int ret;
	struct page_frag *pfrag = &current->task_frag;

	do {
		/* Return error if we don't have space for new frag */
		if (frg_cnt >= MAX_SKB_FRAGS)
			return -EMSGSIZE;

		if (!sk_page_frag_refill(sk, pfrag))
			return -ENOMEM;

		/* copy the user data to page */
		copy = min_t(int, length, pfrag->size - pfrag->offset);

		ret = getfrag(from, page_address(pfrag->page) + pfrag->offset,
			      offset, copy, 0, skb);
		if (ret < 0)
			return -EFAULT;

		/* copy was successful so update the size parameters */
		skb_fill_page_desc(skb, frg_cnt, pfrag->page, pfrag->offset,
				   copy);
		frg_cnt++;
		pfrag->offset += copy;
		get_page(pfrag->page);

		skb->truesize += copy;
		atomic_add(copy, &sk->sk_wmem_alloc);
		skb->len += copy;
		skb->data_len += copy;
		offset += copy;
		length -= copy;

	} while (length > 0);

	return 0;
}
EXPORT_SYMBOL(skb_append_datato_frags);

/**
 *	skb_pull_rcsum - pull skb and update receive checksum
 *	@skb: buffer to update
 *	@len: length of data pulled
 *
 *	This function performs an skb_pull on the packet and updates
 *	the CHECKSUM_COMPLETE checksum.  It should be used on
 *	receive path processing instead of skb_pull unless you know
 *	that the checksum difference is zero (e.g., a valid IP header)
 *	or you are setting ip_summed to CHECKSUM_NONE.
 */
unsigned char *skb_pull_rcsum(struct sk_buff *skb, unsigned int len)
{
	unsigned char *data = skb->data;

	BUG_ON(len > skb->len);
	__skb_pull(skb, len);
	skb_postpull_rcsum(skb, data, len);
	return skb->data;
}
EXPORT_SYMBOL_GPL(skb_pull_rcsum);

/**
 *	skb_segment - Perform protocol segmentation on skb.
 *	@head_skb: buffer to segment
 *	@features: features for the output path (see dev->features)
 *
 *	This function performs segmentation on the given skb.  It returns
 *	a pointer to the first in a list of new skbs for the segments.
 *	In case of error it returns ERR_PTR(err).
 */
struct sk_buff *skb_segment(struct sk_buff *head_skb,
			    netdev_features_t features)
{
	struct sk_buff *segs = NULL;
	struct sk_buff *tail = NULL;
	struct sk_buff *list_skb = skb_shinfo(head_skb)->frag_list;
	skb_frag_t *frag = skb_shinfo(head_skb)->frags;
	unsigned int mss = skb_shinfo(head_skb)->gso_size;
	unsigned int doffset = head_skb->data - skb_mac_header(head_skb);
	struct sk_buff *frag_skb = head_skb;
	unsigned int offset = doffset;
	unsigned int tnl_hlen = skb_tnl_header_len(head_skb);
	unsigned int headroom;
	unsigned int len;
	__be16 proto;
	bool csum;
	int sg = !!(features & NETIF_F_SG);
	int nfrags = skb_shinfo(head_skb)->nr_frags;
	int err = -ENOMEM;
	int i = 0;
	int pos;
	int dummy;

	__skb_push(head_skb, doffset);
	proto = skb_network_protocol(head_skb, &dummy);
	if (unlikely(!proto))
		return ERR_PTR(-EINVAL);

	csum = !head_skb->encap_hdr_csum &&
	    !!can_checksum_protocol(features, proto);

	headroom = skb_headroom(head_skb);
	pos = skb_headlen(head_skb);

	do {
		struct sk_buff *nskb;
		skb_frag_t *nskb_frag;
		int hsize;
		int size;

		len = head_skb->len - offset;
		if (len > mss)
			len = mss;

		hsize = skb_headlen(head_skb) - offset;
		if (hsize < 0)
			hsize = 0;
		if (hsize > len || !sg)
			hsize = len;

		if (!hsize && i >= nfrags && skb_headlen(list_skb) &&
		    (skb_headlen(list_skb) == len || sg)) {
			BUG_ON(skb_headlen(list_skb) > len);

			i = 0;
			nfrags = skb_shinfo(list_skb)->nr_frags;
			frag = skb_shinfo(list_skb)->frags;
			frag_skb = list_skb;
			pos += skb_headlen(list_skb);

			while (pos < offset + len) {
				BUG_ON(i >= nfrags);

				size = skb_frag_size(frag);
				if (pos + size > offset + len)
					break;

				i++;
				pos += size;
				frag++;
			}

			nskb = skb_clone(list_skb, GFP_ATOMIC);
			list_skb = list_skb->next;

			if (unlikely(!nskb))
				goto err;

			if (unlikely(pskb_trim(nskb, len))) {
				kfree_skb(nskb);
				goto err;
			}

			hsize = skb_end_offset(nskb);
			if (skb_cow_head(nskb, doffset + headroom)) {
				kfree_skb(nskb);
				goto err;
			}

			nskb->truesize += skb_end_offset(nskb) - hsize;
			skb_release_head_state(nskb);
			__skb_push(nskb, doffset);
		} else {
			nskb = __alloc_skb(hsize + doffset + headroom,
					   GFP_ATOMIC, skb_alloc_rx_flag(head_skb),
					   NUMA_NO_NODE);

			if (unlikely(!nskb))
				goto err;

			skb_reserve(nskb, headroom);
			__skb_put(nskb, doffset);
		}

		if (segs)
			tail->next = nskb;
		else
			segs = nskb;
		tail = nskb;

		__copy_skb_header(nskb, head_skb);

		skb_headers_offset_update(nskb, skb_headroom(nskb) - headroom);
		skb_reset_mac_len(nskb);

		skb_copy_from_linear_data_offset(head_skb, -tnl_hlen,
						 nskb->data - tnl_hlen,
						 doffset + tnl_hlen);

		if (nskb->len == len + doffset)
			goto perform_csum_check;

		if (!sg && !nskb->remcsum_offload) {
			nskb->ip_summed = CHECKSUM_NONE;
			nskb->csum = skb_copy_and_csum_bits(head_skb, offset,
							    skb_put(nskb, len),
							    len, 0);
			SKB_GSO_CB(nskb)->csum_start =
			    skb_headroom(nskb) + doffset;
			continue;
		}

		nskb_frag = skb_shinfo(nskb)->frags;

		skb_copy_from_linear_data_offset(head_skb, offset,
						 skb_put(nskb, hsize), hsize);

		skb_shinfo(nskb)->tx_flags |= skb_shinfo(head_skb)->tx_flags &
					      SKBTX_SHARED_FRAG;

		while (pos < offset + len) {
			if (i >= nfrags) {
				BUG_ON(skb_headlen(list_skb));

				i = 0;
				nfrags = skb_shinfo(list_skb)->nr_frags;
				frag = skb_shinfo(list_skb)->frags;
				frag_skb = list_skb;

				BUG_ON(!nfrags);

				list_skb = list_skb->next;
			}

			if (unlikely(skb_shinfo(nskb)->nr_frags >=
				     MAX_SKB_FRAGS)) {
				net_warn_ratelimited(
					"skb_segment: too many frags: %u %u\n",
					pos, mss);
				goto err;
			}

			if (unlikely(skb_orphan_frags(frag_skb, GFP_ATOMIC)))
				goto err;

			*nskb_frag = *frag;
			__skb_frag_ref(nskb_frag);
			size = skb_frag_size(nskb_frag);

			if (pos < offset) {
				nskb_frag->page_offset += offset - pos;
				skb_frag_size_sub(nskb_frag, offset - pos);
			}

			skb_shinfo(nskb)->nr_frags++;

			if (pos + size <= offset + len) {
				i++;
				frag++;
				pos += size;
			} else {
				skb_frag_size_sub(nskb_frag, pos + size - (offset + len));
				goto skip_fraglist;
			}

			nskb_frag++;
		}

skip_fraglist:
		nskb->data_len = len - hsize;
		nskb->len += nskb->data_len;
		nskb->truesize += nskb->data_len;

perform_csum_check:
		if (!csum && !nskb->remcsum_offload) {
			nskb->csum = skb_checksum(nskb, doffset,
						  nskb->len - doffset, 0);
			nskb->ip_summed = CHECKSUM_NONE;
			SKB_GSO_CB(nskb)->csum_start =
			    skb_headroom(nskb) + doffset;
		}
	} while ((offset += len) < head_skb->len);

	/* Some callers want to get the end of the list.
	 * Put it in segs->prev to avoid walking the list.
	 * (see validate_xmit_skb_list() for example)
	 */
	segs->prev = tail;

	/* Following permits correct backpressure, for protocols
	 * using skb_set_owner_w().
	 * Idea is to tranfert ownership from head_skb to last segment.
	 */
	if (head_skb->destructor == sock_wfree) {
		swap(tail->truesize, head_skb->truesize);
		swap(tail->destructor, head_skb->destructor);
		swap(tail->sk, head_skb->sk);
	}
	return segs;

err:
	kfree_skb_list(segs);
	return ERR_PTR(err);
}
EXPORT_SYMBOL_GPL(skb_segment);

int skb_gro_receive(struct sk_buff **head, struct sk_buff *skb)
{
	struct skb_shared_info *pinfo, *skbinfo = skb_shinfo(skb);
	unsigned int offset = skb_gro_offset(skb);
	unsigned int headlen = skb_headlen(skb);
	unsigned int len = skb_gro_len(skb);
	struct sk_buff *lp, *p = *head;
	unsigned int delta_truesize;

	if (unlikely(p->len + len >= 65536))
		return -E2BIG;

	lp = NAPI_GRO_CB(p)->last;
	pinfo = skb_shinfo(lp);

	if (headlen <= offset) {
		skb_frag_t *frag;
		skb_frag_t *frag2;
		int i = skbinfo->nr_frags;
		int nr_frags = pinfo->nr_frags + i;

		if (nr_frags > MAX_SKB_FRAGS)
			goto merge;

		offset -= headlen;
		pinfo->nr_frags = nr_frags;
		skbinfo->nr_frags = 0;

		frag = pinfo->frags + nr_frags;
		frag2 = skbinfo->frags + i;
		do {
			*--frag = *--frag2;
		} while (--i);

		frag->page_offset += offset;
		skb_frag_size_sub(frag, offset);

		/* all fragments truesize : remove (head size + sk_buff) */
		delta_truesize = skb->truesize -
				 SKB_TRUESIZE(skb_end_offset(skb));

		skb->truesize -= skb->data_len;
		skb->len -= skb->data_len;
		skb->data_len = 0;

		NAPI_GRO_CB(skb)->free = NAPI_GRO_FREE;
		goto done;
	} else if (skb->head_frag) {
		int nr_frags = pinfo->nr_frags;
		skb_frag_t *frag = pinfo->frags + nr_frags;
		struct page *page = virt_to_head_page(skb->head);
		unsigned int first_size = headlen - offset;
		unsigned int first_offset;

		if (nr_frags + 1 + skbinfo->nr_frags > MAX_SKB_FRAGS)
			goto merge;

		first_offset = skb->data -
			       (unsigned char *)page_address(page) +
			       offset;

		pinfo->nr_frags = nr_frags + 1 + skbinfo->nr_frags;

		frag->page.p	  = page;
		frag->page_offset = first_offset;
		skb_frag_size_set(frag, first_size);

		memcpy(frag + 1, skbinfo->frags, sizeof(*frag) * skbinfo->nr_frags);
		/* We dont need to clear skbinfo->nr_frags here */

		delta_truesize = skb->truesize - SKB_DATA_ALIGN(sizeof(struct sk_buff));
		NAPI_GRO_CB(skb)->free = NAPI_GRO_FREE_STOLEN_HEAD;
		goto done;
	}

merge:
	delta_truesize = skb->truesize;
	if (offset > headlen) {
		unsigned int eat = offset - headlen;

		skbinfo->frags[0].page_offset += eat;
		skb_frag_size_sub(&skbinfo->frags[0], eat);
		skb->data_len -= eat;
		skb->len -= eat;
		offset = headlen;
	}

	__skb_pull(skb, offset);

	if (NAPI_GRO_CB(p)->last == p)
		skb_shinfo(p)->frag_list = skb;
	else
		NAPI_GRO_CB(p)->last->next = skb;
	NAPI_GRO_CB(p)->last = skb;
	__skb_header_release(skb);
	lp = p;

done:
	NAPI_GRO_CB(p)->count++;
	p->data_len += len;
	p->truesize += delta_truesize;
	p->len += len;
	if (lp != p) {
		lp->data_len += len;
		lp->truesize += delta_truesize;
		lp->len += len;
	}
	NAPI_GRO_CB(skb)->same_flow = 1;
	return 0;
}

void __init skb_init(void)
{
	skbuff_head_cache = kmem_cache_create("skbuff_head_cache",
					      sizeof(struct sk_buff),
					      0,
					      SLAB_HWCACHE_ALIGN|SLAB_PANIC,
					      NULL);
	skbuff_fclone_cache = kmem_cache_create("skbuff_fclone_cache",
						sizeof(struct sk_buff_fclones),
						0,
						SLAB_HWCACHE_ALIGN|SLAB_PANIC,
						NULL);
#if defined(CONFIG_IMQ) || defined(CONFIG_IMQ_MODULE)
	skbuff_cb_store_cache = kmem_cache_create("skbuff_cb_store_cache",
						  sizeof(struct skb_cb_table),
						  0,
						  SLAB_HWCACHE_ALIGN|SLAB_PANIC,
						  NULL);
#endif
}

static int
__skb_to_sgvec(struct sk_buff *skb, struct scatterlist *sg, int offset, int len,
	       unsigned int recursion_level)
{
	int start = skb_headlen(skb);
	int i, copy = start - offset;
	struct sk_buff *frag_iter;
	int elt = 0;

	if (unlikely(recursion_level >= 24))
		return -EMSGSIZE;

	if (copy > 0) {
		if (copy > len)
			copy = len;
		sg_set_buf(sg, skb->data + offset, copy);
		elt++;
		if ((len -= copy) == 0)
			return elt;
		offset += copy;
	}

	for (i = 0; i < skb_shinfo(skb)->nr_frags; i++) {
		int end;

		WARN_ON(start > offset + len);

		end = start + skb_frag_size(&skb_shinfo(skb)->frags[i]);
		if ((copy = end - offset) > 0) {
			skb_frag_t *frag = &skb_shinfo(skb)->frags[i];
			if (unlikely(elt && sg_is_last(&sg[elt - 1])))
				return -EMSGSIZE;

			if (copy > len)
				copy = len;
			sg_set_page(&sg[elt], skb_frag_page(frag), copy,
					frag->page_offset+offset-start);
			elt++;
			if (!(len -= copy))
				return elt;
			offset += copy;
		}
		start = end;
	}

	skb_walk_frags(skb, frag_iter) {
		int end, ret;

		WARN_ON(start > offset + len);

		end = start + frag_iter->len;
		if ((copy = end - offset) > 0) {
			if (unlikely(elt && sg_is_last(&sg[elt - 1])))
				return -EMSGSIZE;

			if (copy > len)
				copy = len;
			ret = __skb_to_sgvec(frag_iter, sg+elt, offset - start,
					      copy, recursion_level + 1);
			if (unlikely(ret < 0))
				return ret;
			elt += ret;
			if ((len -= copy) == 0)
				return elt;
			offset += copy;
		}
		start = end;
	}
	BUG_ON(len);
	return elt;
}

/**
 *	skb_to_sgvec - Fill a scatter-gather list from a socket buffer
 *	@skb: Socket buffer containing the buffers to be mapped
 *	@sg: The scatter-gather list to map into
 *	@offset: The offset into the buffer's contents to start mapping
 *	@len: Length of buffer space to be mapped
 *
 *	Fill the specified scatter-gather list with mappings/pointers into a
 *	region of the buffer space attached to a socket buffer. Returns either
 *	the number of scatterlist items used, or -EMSGSIZE if the contents
 *	could not fit.
 */
int skb_to_sgvec(struct sk_buff *skb, struct scatterlist *sg, int offset, int len)
{
	int nsg = __skb_to_sgvec(skb, sg, offset, len, 0);

	if (nsg <= 0)
		return nsg;

	sg_mark_end(&sg[nsg - 1]);

	return nsg;
}
EXPORT_SYMBOL_GPL(skb_to_sgvec);

/* As compared with skb_to_sgvec, skb_to_sgvec_nomark only map skb to given
 * sglist without mark the sg which contain last skb data as the end.
 * So the caller can mannipulate sg list as will when padding new data after
 * the first call without calling sg_unmark_end to expend sg list.
 *
 * Scenario to use skb_to_sgvec_nomark:
 * 1. sg_init_table
 * 2. skb_to_sgvec_nomark(payload1)
 * 3. skb_to_sgvec_nomark(payload2)
 *
 * This is equivalent to:
 * 1. sg_init_table
 * 2. skb_to_sgvec(payload1)
 * 3. sg_unmark_end
 * 4. skb_to_sgvec(payload2)
 *
 * When mapping mutilple payload conditionally, skb_to_sgvec_nomark
 * is more preferable.
 */
int skb_to_sgvec_nomark(struct sk_buff *skb, struct scatterlist *sg,
			int offset, int len)
{
	return __skb_to_sgvec(skb, sg, offset, len, 0);
}
EXPORT_SYMBOL_GPL(skb_to_sgvec_nomark);



/**
 *	skb_cow_data - Check that a socket buffer's data buffers are writable
 *	@skb: The socket buffer to check.
 *	@tailbits: Amount of trailing space to be added
 *	@trailer: Returned pointer to the skb where the @tailbits space begins
 *
 *	Make sure that the data buffers attached to a socket buffer are
 *	writable. If they are not, private copies are made of the data buffers
 *	and the socket buffer is set to use these instead.
 *
 *	If @tailbits is given, make sure that there is space to write @tailbits
 *	bytes of data beyond current end of socket buffer.  @trailer will be
 *	set to point to the skb in which this space begins.
 *
 *	The number of scatterlist elements required to completely map the
 *	COW'd and extended socket buffer will be returned.
 */
int skb_cow_data(struct sk_buff *skb, int tailbits, struct sk_buff **trailer)
{
	int copyflag;
	int elt;
	struct sk_buff *skb1, **skb_p;

	/* If skb is cloned or its head is paged, reallocate
	 * head pulling out all the pages (pages are considered not writable
	 * at the moment even if they are anonymous).
	 */
	if ((skb_cloned(skb) || skb_shinfo(skb)->nr_frags) &&
	    __pskb_pull_tail(skb, skb_pagelen(skb)-skb_headlen(skb)) == NULL)
		return -ENOMEM;

	/* Easy case. Most of packets will go this way. */
	if (!skb_has_frag_list(skb)) {
		/* A little of trouble, not enough of space for trailer.
		 * This should not happen, when stack is tuned to generate
		 * good frames. OK, on miss we reallocate and reserve even more
		 * space, 128 bytes is fair. */

		if (skb_tailroom(skb) < tailbits &&
		    pskb_expand_head(skb, 0, tailbits-skb_tailroom(skb)+128, GFP_ATOMIC))
			return -ENOMEM;

		/* Voila! */
		*trailer = skb;
		return 1;
	}

	/* Misery. We are in troubles, going to mincer fragments... */

	elt = 1;
	skb_p = &skb_shinfo(skb)->frag_list;
	copyflag = 0;

	while ((skb1 = *skb_p) != NULL) {
		int ntail = 0;

		/* The fragment is partially pulled by someone,
		 * this can happen on input. Copy it and everything
		 * after it. */

		if (skb_shared(skb1))
			copyflag = 1;

		/* If the skb is the last, worry about trailer. */

		if (skb1->next == NULL && tailbits) {
			if (skb_shinfo(skb1)->nr_frags ||
			    skb_has_frag_list(skb1) ||
			    skb_tailroom(skb1) < tailbits)
				ntail = tailbits + 128;
		}

		if (copyflag ||
		    skb_cloned(skb1) ||
		    ntail ||
		    skb_shinfo(skb1)->nr_frags ||
		    skb_has_frag_list(skb1)) {
			struct sk_buff *skb2;

			/* Fuck, we are miserable poor guys... */
			if (ntail == 0)
				skb2 = skb_copy(skb1, GFP_ATOMIC);
			else
				skb2 = skb_copy_expand(skb1,
						       skb_headroom(skb1),
						       ntail,
						       GFP_ATOMIC);
			if (unlikely(skb2 == NULL))
				return -ENOMEM;

			if (skb1->sk)
				skb_set_owner_w(skb2, skb1->sk);

			/* Looking around. Are we still alive?
			 * OK, link new skb, drop old one */

			skb2->next = skb1->next;
			*skb_p = skb2;
			kfree_skb(skb1);
			skb1 = skb2;
		}
		elt++;
		*trailer = skb1;
		skb_p = &skb1->next;
	}

	return elt;
}
EXPORT_SYMBOL_GPL(skb_cow_data);

static void sock_rmem_free(struct sk_buff *skb)
{
	struct sock *sk = skb->sk;

	atomic_sub(skb->truesize, &sk->sk_rmem_alloc);
}

/*
 * Note: We dont mem charge error packets (no sk_forward_alloc changes)
 */
int sock_queue_err_skb(struct sock *sk, struct sk_buff *skb)
{
	if (atomic_read(&sk->sk_rmem_alloc) + skb->truesize >=
	    (unsigned int)sk->sk_rcvbuf)
		return -ENOMEM;

	skb_orphan(skb);
	skb->sk = sk;
	skb->destructor = sock_rmem_free;
	atomic_add(skb->truesize, &sk->sk_rmem_alloc);

	/* before exiting rcu section, make sure dst is refcounted */
	skb_dst_force(skb);

	skb_queue_tail(&sk->sk_error_queue, skb);
	if (!sock_flag(sk, SOCK_DEAD))
		sk->sk_error_report(sk);
	return 0;
}
EXPORT_SYMBOL(sock_queue_err_skb);

struct sk_buff *sock_dequeue_err_skb(struct sock *sk)
{
	struct sk_buff_head *q = &sk->sk_error_queue;
	struct sk_buff *skb, *skb_next;
	unsigned long flags;
	int err = 0;

	spin_lock_irqsave(&q->lock, flags);
	skb = __skb_dequeue(q);
	if (skb && (skb_next = skb_peek(q)))
		err = SKB_EXT_ERR(skb_next)->ee.ee_errno;
	spin_unlock_irqrestore(&q->lock, flags);

	sk->sk_err = err;
	if (err)
		sk->sk_error_report(sk);

	return skb;
}
EXPORT_SYMBOL(sock_dequeue_err_skb);

/**
 * skb_clone_sk - create clone of skb, and take reference to socket
 * @skb: the skb to clone
 *
 * This function creates a clone of a buffer that holds a reference on
 * sk_refcnt.  Buffers created via this function are meant to be
 * returned using sock_queue_err_skb, or free via kfree_skb.
 *
 * When passing buffers allocated with this function to sock_queue_err_skb
 * it is necessary to wrap the call with sock_hold/sock_put in order to
 * prevent the socket from being released prior to being enqueued on
 * the sk_error_queue.
 */
struct sk_buff *skb_clone_sk(struct sk_buff *skb)
{
	struct sock *sk = skb->sk;
	struct sk_buff *clone;

	if (!sk || !atomic_inc_not_zero(&sk->sk_refcnt))
		return NULL;

	clone = skb_clone(skb, GFP_ATOMIC);
	if (!clone) {
		sock_put(sk);
		return NULL;
	}

	clone->sk = sk;
	clone->destructor = sock_efree;

	return clone;
}
EXPORT_SYMBOL(skb_clone_sk);

static void __skb_complete_tx_timestamp(struct sk_buff *skb,
					struct sock *sk,
					int tstype)
{
	struct sock_exterr_skb *serr;
	int err;

	serr = SKB_EXT_ERR(skb);
	memset(serr, 0, sizeof(*serr));
	serr->ee.ee_errno = ENOMSG;
	serr->ee.ee_origin = SO_EE_ORIGIN_TIMESTAMPING;
	serr->ee.ee_info = tstype;
	if (sk->sk_tsflags & SOF_TIMESTAMPING_OPT_ID) {
		serr->ee.ee_data = skb_shinfo(skb)->tskey;
		if (sk->sk_protocol == IPPROTO_TCP &&
		    sk->sk_type == SOCK_STREAM)
			serr->ee.ee_data -= sk->sk_tskey;
	}

	err = sock_queue_err_skb(sk, skb);

	if (err)
		kfree_skb(skb);
}

static bool skb_may_tx_timestamp(struct sock *sk, bool tsonly)
{
	bool ret;

	if (likely(sysctl_tstamp_allow_data || tsonly))
		return true;

	read_lock_bh(&sk->sk_callback_lock);
	ret = sk->sk_socket && sk->sk_socket->file &&
	      file_ns_capable(sk->sk_socket->file, &init_user_ns, CAP_NET_RAW);
	read_unlock_bh(&sk->sk_callback_lock);
	return ret;
}

void skb_complete_tx_timestamp(struct sk_buff *skb,
			       struct skb_shared_hwtstamps *hwtstamps)
{
	struct sock *sk = skb->sk;

	if (!skb_may_tx_timestamp(sk, false))
		goto err;

	/* Take a reference to prevent skb_orphan() from freeing the socket,
	 * but only if the socket refcount is not zero.
	 */
	if (likely(atomic_inc_not_zero(&sk->sk_refcnt))) {
		*skb_hwtstamps(skb) = *hwtstamps;
		__skb_complete_tx_timestamp(skb, sk, SCM_TSTAMP_SND);
		sock_put(sk);
		return;
	}

err:
	kfree_skb(skb);
}
EXPORT_SYMBOL_GPL(skb_complete_tx_timestamp);

void __skb_tstamp_tx(struct sk_buff *orig_skb,
		     struct skb_shared_hwtstamps *hwtstamps,
		     struct sock *sk, int tstype)
{
	struct sk_buff *skb;
	bool tsonly;

	if (!sk)
		return;

	tsonly = sk->sk_tsflags & SOF_TIMESTAMPING_OPT_TSONLY;
	if (!skb_may_tx_timestamp(sk, tsonly))
		return;

	if (tsonly)
		skb = alloc_skb(0, GFP_ATOMIC);
	else
		skb = skb_clone(orig_skb, GFP_ATOMIC);
	if (!skb)
		return;

	if (tsonly) {
		skb_shinfo(skb)->tx_flags |= skb_shinfo(orig_skb)->tx_flags &
					     SKBTX_ANY_TSTAMP;
		skb_shinfo(skb)->tskey = skb_shinfo(orig_skb)->tskey;
	}

	if (hwtstamps)
		*skb_hwtstamps(skb) = *hwtstamps;
	else
		skb->tstamp = ktime_get_real();

	__skb_complete_tx_timestamp(skb, sk, tstype);
}
EXPORT_SYMBOL_GPL(__skb_tstamp_tx);

void skb_tstamp_tx(struct sk_buff *orig_skb,
		   struct skb_shared_hwtstamps *hwtstamps)
{
	return __skb_tstamp_tx(orig_skb, hwtstamps, orig_skb->sk,
			       SCM_TSTAMP_SND);
}
EXPORT_SYMBOL_GPL(skb_tstamp_tx);

void skb_complete_wifi_ack(struct sk_buff *skb, bool acked)
{
	struct sock *sk = skb->sk;
	struct sock_exterr_skb *serr;
	int err = 1;

	skb->wifi_acked_valid = 1;
	skb->wifi_acked = acked;

	serr = SKB_EXT_ERR(skb);
	memset(serr, 0, sizeof(*serr));
	serr->ee.ee_errno = ENOMSG;
	serr->ee.ee_origin = SO_EE_ORIGIN_TXSTATUS;

	/* Take a reference to prevent skb_orphan() from freeing the socket,
	 * but only if the socket refcount is not zero.
	 */
	if (likely(atomic_inc_not_zero(&sk->sk_refcnt))) {
		err = sock_queue_err_skb(sk, skb);
		sock_put(sk);
	}
	if (err)
		kfree_skb(skb);
}
EXPORT_SYMBOL_GPL(skb_complete_wifi_ack);

/**
 * skb_partial_csum_set - set up and verify partial csum values for packet
 * @skb: the skb to set
 * @start: the number of bytes after skb->data to start checksumming.
 * @off: the offset from start to place the checksum.
 *
 * For untrusted partially-checksummed packets, we need to make sure the values
 * for skb->csum_start and skb->csum_offset are valid so we don't oops.
 *
 * This function checks and sets those values and skb->ip_summed: if this
 * returns false you should drop the packet.
 */
bool skb_partial_csum_set(struct sk_buff *skb, u16 start, u16 off)
{
	if (unlikely(start > skb_headlen(skb)) ||
	    unlikely((int)start + off > skb_headlen(skb) - 2)) {
		net_warn_ratelimited("bad partial csum: csum=%u/%u len=%u\n",
				     start, off, skb_headlen(skb));
		return false;
	}
	skb->ip_summed = CHECKSUM_PARTIAL;
	skb->csum_start = skb_headroom(skb) + start;
	skb->csum_offset = off;
	skb_set_transport_header(skb, start);
	return true;
}
EXPORT_SYMBOL_GPL(skb_partial_csum_set);

static int skb_maybe_pull_tail(struct sk_buff *skb, unsigned int len,
			       unsigned int max)
{
	if (skb_headlen(skb) >= len)
		return 0;

	/* If we need to pullup then pullup to the max, so we
	 * won't need to do it again.
	 */
	if (max > skb->len)
		max = skb->len;

	if (__pskb_pull_tail(skb, max - skb_headlen(skb)) == NULL)
		return -ENOMEM;

	if (skb_headlen(skb) < len)
		return -EPROTO;

	return 0;
}

#define MAX_TCP_HDR_LEN (15 * 4)

static __sum16 *skb_checksum_setup_ip(struct sk_buff *skb,
				      typeof(IPPROTO_IP) proto,
				      unsigned int off)
{
	switch (proto) {
		int err;

	case IPPROTO_TCP:
		err = skb_maybe_pull_tail(skb, off + sizeof(struct tcphdr),
					  off + MAX_TCP_HDR_LEN);
		if (!err && !skb_partial_csum_set(skb, off,
						  offsetof(struct tcphdr,
							   check)))
			err = -EPROTO;
		return err ? ERR_PTR(err) : &tcp_hdr(skb)->check;

	case IPPROTO_UDP:
		err = skb_maybe_pull_tail(skb, off + sizeof(struct udphdr),
					  off + sizeof(struct udphdr));
		if (!err && !skb_partial_csum_set(skb, off,
						  offsetof(struct udphdr,
							   check)))
			err = -EPROTO;
		return err ? ERR_PTR(err) : &udp_hdr(skb)->check;
	}

	return ERR_PTR(-EPROTO);
}

/* This value should be large enough to cover a tagged ethernet header plus
 * maximally sized IP and TCP or UDP headers.
 */
#define MAX_IP_HDR_LEN 128

static int skb_checksum_setup_ipv4(struct sk_buff *skb, bool recalculate)
{
	unsigned int off;
	bool fragment;
	__sum16 *csum;
	int err;

	fragment = false;

	err = skb_maybe_pull_tail(skb,
				  sizeof(struct iphdr),
				  MAX_IP_HDR_LEN);
	if (err < 0)
		goto out;

	if (ip_hdr(skb)->frag_off & htons(IP_OFFSET | IP_MF))
		fragment = true;

	off = ip_hdrlen(skb);

	err = -EPROTO;

	if (fragment)
		goto out;

	csum = skb_checksum_setup_ip(skb, ip_hdr(skb)->protocol, off);
	if (IS_ERR(csum))
		return PTR_ERR(csum);

	if (recalculate)
		*csum = ~csum_tcpudp_magic(ip_hdr(skb)->saddr,
					   ip_hdr(skb)->daddr,
					   skb->len - off,
					   ip_hdr(skb)->protocol, 0);
	err = 0;

out:
	return err;
}

/* This value should be large enough to cover a tagged ethernet header plus
 * an IPv6 header, all options, and a maximal TCP or UDP header.
 */
#define MAX_IPV6_HDR_LEN 256

#define OPT_HDR(type, skb, off) \
	(type *)(skb_network_header(skb) + (off))

static int skb_checksum_setup_ipv6(struct sk_buff *skb, bool recalculate)
{
	int err;
	u8 nexthdr;
	unsigned int off;
	unsigned int len;
	bool fragment;
	bool done;
	__sum16 *csum;

	fragment = false;
	done = false;

	off = sizeof(struct ipv6hdr);

	err = skb_maybe_pull_tail(skb, off, MAX_IPV6_HDR_LEN);
	if (err < 0)
		goto out;

	nexthdr = ipv6_hdr(skb)->nexthdr;

	len = sizeof(struct ipv6hdr) + ntohs(ipv6_hdr(skb)->payload_len);
	while (off <= len && !done) {
		switch (nexthdr) {
		case IPPROTO_DSTOPTS:
		case IPPROTO_HOPOPTS:
		case IPPROTO_ROUTING: {
			struct ipv6_opt_hdr *hp;

			err = skb_maybe_pull_tail(skb,
						  off +
						  sizeof(struct ipv6_opt_hdr),
						  MAX_IPV6_HDR_LEN);
			if (err < 0)
				goto out;

			hp = OPT_HDR(struct ipv6_opt_hdr, skb, off);
			nexthdr = hp->nexthdr;
			off += ipv6_optlen(hp);
			break;
		}
		case IPPROTO_AH: {
			struct ip_auth_hdr *hp;

			err = skb_maybe_pull_tail(skb,
						  off +
						  sizeof(struct ip_auth_hdr),
						  MAX_IPV6_HDR_LEN);
			if (err < 0)
				goto out;

			hp = OPT_HDR(struct ip_auth_hdr, skb, off);
			nexthdr = hp->nexthdr;
			off += ipv6_authlen(hp);
			break;
		}
		case IPPROTO_FRAGMENT: {
			struct frag_hdr *hp;

			err = skb_maybe_pull_tail(skb,
						  off +
						  sizeof(struct frag_hdr),
						  MAX_IPV6_HDR_LEN);
			if (err < 0)
				goto out;

			hp = OPT_HDR(struct frag_hdr, skb, off);

			if (hp->frag_off & htons(IP6_OFFSET | IP6_MF))
				fragment = true;

			nexthdr = hp->nexthdr;
			off += sizeof(struct frag_hdr);
			break;
		}
		default:
			done = true;
			break;
		}
	}

	err = -EPROTO;

	if (!done || fragment)
		goto out;

	csum = skb_checksum_setup_ip(skb, nexthdr, off);
	if (IS_ERR(csum))
		return PTR_ERR(csum);

	if (recalculate)
		*csum = ~csum_ipv6_magic(&ipv6_hdr(skb)->saddr,
					 &ipv6_hdr(skb)->daddr,
					 skb->len - off, nexthdr, 0);
	err = 0;

out:
	return err;
}

/**
 * skb_checksum_setup - set up partial checksum offset
 * @skb: the skb to set up
 * @recalculate: if true the pseudo-header checksum will be recalculated
 */
int skb_checksum_setup(struct sk_buff *skb, bool recalculate)
{
	int err;

	switch (skb->protocol) {
	case htons(ETH_P_IP):
		err = skb_checksum_setup_ipv4(skb, recalculate);
		break;

	case htons(ETH_P_IPV6):
		err = skb_checksum_setup_ipv6(skb, recalculate);
		break;

	default:
		err = -EPROTO;
		break;
	}

	return err;
}
EXPORT_SYMBOL(skb_checksum_setup);

void __skb_warn_lro_forwarding(const struct sk_buff *skb)
{
	net_warn_ratelimited("%s: received packets cannot be forwarded while LRO is enabled\n",
			     skb->dev->name);
}
EXPORT_SYMBOL(__skb_warn_lro_forwarding);

void kfree_skb_partial(struct sk_buff *skb, bool head_stolen)
{
	if (head_stolen) {
		skb_release_head_state(skb);
		kmem_cache_free(skbuff_head_cache, skb);
	} else {
		__kfree_skb(skb);
	}
}
EXPORT_SYMBOL(kfree_skb_partial);

/**
 * skb_try_coalesce - try to merge skb to prior one
 * @to: prior buffer
 * @from: buffer to add
 * @fragstolen: pointer to boolean
 * @delta_truesize: how much more was allocated than was requested
 */
bool skb_try_coalesce(struct sk_buff *to, struct sk_buff *from,
		      bool *fragstolen, int *delta_truesize)
{
	int i, delta, len = from->len;

	*fragstolen = false;

	if (skb_cloned(to))
		return false;

	if (len <= skb_tailroom(to)) {
		if (len)
			BUG_ON(skb_copy_bits(from, 0, skb_put(to, len), len));
		*delta_truesize = 0;
		return true;
	}

	if (skb_has_frag_list(to) || skb_has_frag_list(from))
		return false;

	if (skb_headlen(from) != 0) {
		struct page *page;
		unsigned int offset;

		if (skb_shinfo(to)->nr_frags +
		    skb_shinfo(from)->nr_frags >= MAX_SKB_FRAGS)
			return false;

		if (skb_head_is_locked(from))
			return false;

		delta = from->truesize - SKB_DATA_ALIGN(sizeof(struct sk_buff));

		page = virt_to_head_page(from->head);
		offset = from->data - (unsigned char *)page_address(page);

		skb_fill_page_desc(to, skb_shinfo(to)->nr_frags,
				   page, offset, skb_headlen(from));
		*fragstolen = true;
	} else {
		if (skb_shinfo(to)->nr_frags +
		    skb_shinfo(from)->nr_frags > MAX_SKB_FRAGS)
			return false;

		delta = from->truesize - SKB_TRUESIZE(skb_end_offset(from));
	}

	WARN_ON_ONCE(delta < len);

	memcpy(skb_shinfo(to)->frags + skb_shinfo(to)->nr_frags,
	       skb_shinfo(from)->frags,
	       skb_shinfo(from)->nr_frags * sizeof(skb_frag_t));
	skb_shinfo(to)->nr_frags += skb_shinfo(from)->nr_frags;

	if (!skb_cloned(from))
		skb_shinfo(from)->nr_frags = 0;

	/* if the skb is not cloned this does nothing
	 * since we set nr_frags to 0.
	 */
	for (i = 0; i < skb_shinfo(from)->nr_frags; i++)
		skb_frag_ref(from, i);

	to->truesize += delta;
	to->len += len;
	to->data_len += len;

	*delta_truesize = delta;
	return true;
}
EXPORT_SYMBOL(skb_try_coalesce);

/**
 * skb_scrub_packet - scrub an skb
 *
 * @skb: buffer to clean
 * @xnet: packet is crossing netns
 *
 * skb_scrub_packet can be used after encapsulating or decapsulting a packet
 * into/from a tunnel. Some information have to be cleared during these
 * operations.
 * skb_scrub_packet can also be used to clean a skb before injecting it in
 * another namespace (@xnet == true). We have to clear all information in the
 * skb that could impact namespace isolation.
 */
void skb_scrub_packet(struct sk_buff *skb, bool xnet)
{
	skb->tstamp.tv64 = 0;
	skb->pkt_type = PACKET_HOST;
	skb->skb_iif = 0;
	skb->ignore_df = 0;
	skb_dst_drop(skb);
	skb_sender_cpu_clear(skb);
	secpath_reset(skb);
	nf_reset(skb);
	nf_reset_trace(skb);

	if (!xnet)
		return;

	ipvs_reset(skb);
	skb_orphan(skb);

#if defined(CONFIG_BCM_KF_BLOG) && defined(CONFIG_BLOG)
/*
 *  when using containers in some network configurations, the packets
 *  will be treated as L2 flows while learning. Local aceleration
 *  is not supported for L2 flows so skip it.
 *
 *  note: we try to add containers MAC address to Host MAC table (using
 *  netdev_notfier) to treat flows terminating in containers as L3 flows,
 *  but this check helps to catch any unknown configurations
*/
    if(skb->blog_p && skb->blog_p->l2_mode)
    {
		blog_skip(skb, blog_skip_reason_scrub_pkt); /* No local accel in l2 mode */
    }
#endif
	skb->mark = 0;
}
EXPORT_SYMBOL_GPL(skb_scrub_packet);

/**
 * skb_gso_transport_seglen - Return length of individual segments of a gso packet
 *
 * @skb: GSO skb
 *
 * skb_gso_transport_seglen is used to determine the real size of the
 * individual segments, including Layer4 headers (TCP/UDP).
 *
 * The MAC/L2 or network (IP, IPv6) headers are not accounted for.
 */
unsigned int skb_gso_transport_seglen(const struct sk_buff *skb)
{
	const struct skb_shared_info *shinfo = skb_shinfo(skb);
	unsigned int thlen = 0;

	if (skb->encapsulation) {
		thlen = skb_inner_transport_header(skb) -
			skb_transport_header(skb);

		if (likely(shinfo->gso_type & (SKB_GSO_TCPV4 | SKB_GSO_TCPV6)))
			thlen += inner_tcp_hdrlen(skb);
	} else if (likely(shinfo->gso_type & (SKB_GSO_TCPV4 | SKB_GSO_TCPV6))) {
		thlen = tcp_hdrlen(skb);
	}
	/* UFO sets gso_size to the size of the fragmentation
	 * payload, i.e. the size of the L4 (UDP) header is already
	 * accounted for.
	 */
	return thlen + shinfo->gso_size;
}
EXPORT_SYMBOL_GPL(skb_gso_transport_seglen);

static struct sk_buff *skb_reorder_vlan_header(struct sk_buff *skb)
{
	if (skb_cow(skb, skb_headroom(skb)) < 0) {
		kfree_skb(skb);
		return NULL;
	}

	memmove(skb->data - ETH_HLEN, skb->data - skb->mac_len - VLAN_HLEN,
		2 * ETH_ALEN);
	skb->mac_header += VLAN_HLEN;
	return skb;
}

struct sk_buff *skb_vlan_untag(struct sk_buff *skb)
{
	struct vlan_hdr *vhdr;
	u16 vlan_tci;

	if (unlikely(skb_vlan_tag_present(skb))) {
		/* vlan_tci is already set-up so leave this for another time */
		return skb;
	}

	skb = skb_share_check(skb, GFP_ATOMIC);
	if (unlikely(!skb))
		goto err_free;

	if (unlikely(!pskb_may_pull(skb, VLAN_HLEN)))
		goto err_free;

	vhdr = (struct vlan_hdr *)skb->data;
	vlan_tci = ntohs(vhdr->h_vlan_TCI);
#if defined(CONFIG_BCM_KF_NBUFF)
#if defined(CONFIG_BCM_KF_VLAN) && (defined(CONFIG_BCM_VLAN) || defined(CONFIG_BCM_VLAN_MODULE))
	skb->cfi_save = vlan_tci & VLAN_CFI_MASK;
#endif
#endif
	__vlan_hwaccel_put_tag(skb, skb->protocol, vlan_tci);

	skb_pull_rcsum(skb, VLAN_HLEN);
	vlan_set_encap_proto(skb, vhdr);

	skb = skb_reorder_vlan_header(skb);
	if (unlikely(!skb))
		goto err_free;

	skb_reset_network_header(skb);
	skb_reset_transport_header(skb);
	skb_reset_mac_len(skb);

	return skb;

err_free:
	kfree_skb(skb);
	return NULL;
}
EXPORT_SYMBOL(skb_vlan_untag);

int skb_ensure_writable(struct sk_buff *skb, int write_len)
{
	if (!pskb_may_pull(skb, write_len))
		return -ENOMEM;

	if (!skb_cloned(skb) || skb_clone_writable(skb, write_len))
		return 0;

	return pskb_expand_head(skb, 0, 0, GFP_ATOMIC);
}
EXPORT_SYMBOL(skb_ensure_writable);

/* remove VLAN header from packet and update csum accordingly. */
static int __skb_vlan_pop(struct sk_buff *skb, u16 *vlan_tci)
{
	struct vlan_hdr *vhdr;
	unsigned int offset = skb->data - skb_mac_header(skb);
	int err;

	__skb_push(skb, offset);
	err = skb_ensure_writable(skb, VLAN_ETH_HLEN);
	if (unlikely(err))
		goto pull;

	skb_postpull_rcsum(skb, skb->data + (2 * ETH_ALEN), VLAN_HLEN);

	vhdr = (struct vlan_hdr *)(skb->data + ETH_HLEN);
	*vlan_tci = ntohs(vhdr->h_vlan_TCI);

	memmove(skb->data + VLAN_HLEN, skb->data, 2 * ETH_ALEN);
	__skb_pull(skb, VLAN_HLEN);

	vlan_set_encap_proto(skb, vhdr);
	skb->mac_header += VLAN_HLEN;

	if (skb_network_offset(skb) < ETH_HLEN)
		skb_set_network_header(skb, ETH_HLEN);

	skb_reset_mac_len(skb);
pull:
	__skb_pull(skb, offset);

	return err;
}

int skb_vlan_pop(struct sk_buff *skb)
{
	u16 vlan_tci;
	__be16 vlan_proto;
	int err;

	if (likely(skb_vlan_tag_present(skb))) {
		skb->vlan_tci = 0;
	} else {
		if (unlikely((skb->protocol != htons(ETH_P_8021Q) &&
			      skb->protocol != htons(ETH_P_8021AD)) ||
			     skb->len < VLAN_ETH_HLEN))
			return 0;

		err = __skb_vlan_pop(skb, &vlan_tci);
		if (err)
			return err;
	}
	/* move next vlan tag to hw accel tag */
	if (likely((skb->protocol != htons(ETH_P_8021Q) &&
		    skb->protocol != htons(ETH_P_8021AD)) ||
		   skb->len < VLAN_ETH_HLEN))
		return 0;

	vlan_proto = skb->protocol;
	err = __skb_vlan_pop(skb, &vlan_tci);
	if (unlikely(err))
		return err;

	__vlan_hwaccel_put_tag(skb, vlan_proto, vlan_tci);
	return 0;
}
EXPORT_SYMBOL(skb_vlan_pop);

int skb_vlan_push(struct sk_buff *skb, __be16 vlan_proto, u16 vlan_tci)
{
	if (skb_vlan_tag_present(skb)) {
		unsigned int offset = skb->data - skb_mac_header(skb);
		int err;

		/* __vlan_insert_tag expect skb->data pointing to mac header.
		 * So change skb->data before calling it and change back to
		 * original position later
		 */
		__skb_push(skb, offset);
		err = __vlan_insert_tag(skb, skb->vlan_proto,
					skb_vlan_tag_get(skb));
		if (err)
			return err;
		skb->protocol = skb->vlan_proto;
		skb->mac_len += VLAN_HLEN;
		__skb_pull(skb, offset);

		if (skb->ip_summed == CHECKSUM_COMPLETE)
			skb->csum = csum_add(skb->csum, csum_partial(skb->data
					+ (2 * ETH_ALEN), VLAN_HLEN, 0));
	}
	__vlan_hwaccel_put_tag(skb, vlan_proto, vlan_tci);
	return 0;
}
EXPORT_SYMBOL(skb_vlan_push);

/**
 * alloc_skb_with_frags - allocate skb with page frags
 *
 * @header_len: size of linear part
 * @data_len: needed length in frags
 * @max_page_order: max page order desired.
 * @errcode: pointer to error code if any
 * @gfp_mask: allocation mask
 *
 * This can be used to allocate a paged skb, given a maximal order for frags.
 */
struct sk_buff *alloc_skb_with_frags(unsigned long header_len,
				     unsigned long data_len,
				     int max_page_order,
				     int *errcode,
				     gfp_t gfp_mask)
{
	int npages = (data_len + (PAGE_SIZE - 1)) >> PAGE_SHIFT;
	unsigned long chunk;
	struct sk_buff *skb;
	struct page *page;
	gfp_t gfp_head;
	int i;

	*errcode = -EMSGSIZE;
	/* Note this test could be relaxed, if we succeed to allocate
	 * high order pages...
	 */
	if (npages > MAX_SKB_FRAGS)
		return NULL;

	gfp_head = gfp_mask;
	if (gfp_head & __GFP_WAIT)
		gfp_head |= __GFP_REPEAT;

	*errcode = -ENOBUFS;
	skb = alloc_skb(header_len, gfp_head);
	if (!skb)
		return NULL;

	skb->truesize += npages << PAGE_SHIFT;

	for (i = 0; npages > 0; i++) {
		int order = max_page_order;

		while (order) {
			if (npages >= 1 << order) {
				page = alloc_pages((gfp_mask & ~__GFP_WAIT) |
						   __GFP_COMP |
						   __GFP_NOWARN |
						   __GFP_NORETRY,
						   order);
				if (page)
					goto fill_page;
				/* Do not retry other high order allocations */
				order = 1;
				max_page_order = 0;
			}
			order--;
		}
		page = alloc_page(gfp_mask);
		if (!page)
			goto failure;
fill_page:
		chunk = min_t(unsigned long, data_len,
			      PAGE_SIZE << order);
		skb_fill_page_desc(skb, i, page, 0, chunk);
		data_len -= chunk;
		npages -= 1 << order;
	}
	return skb;

failure:
	kfree_skb(skb);
	return NULL;
}
EXPORT_SYMBOL(alloc_skb_with_frags);
