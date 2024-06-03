/*
 * Linux Packet (skb) interface
 *
 * Copyright (C) 2023, Broadcom. All Rights Reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 *
 * <<Broadcom-WL-IPTag/Open:>>
 *
 * $Id: linux_pkt.c 831681 2023-10-24 17:21:12Z $
 */

#define PKTFREE_NEW_API

#include <typedefs.h>
#include <bcmendian.h>
#include <linuxver.h>
#include <bcmdefs.h>

#ifdef mips
#include <asm/paccess.h>
#endif /* mips */
#include <linux/random.h>

#include <osl.h>
#include <bcmutils.h>
#include <pcicfg.h>

#include <linux/fs.h>
#include "linux_osl_priv.h"

#if defined(BCM_BLOG)
#include <linux/nbuff.h>
#include <wl_blog.h>
#endif

#if defined(PKTC_TBL)
#include <wl_pktc.h>
#endif

#if defined(CMWIFI) && defined(CMWIFI_EROUTER)
#include "wl_erouter.h"
#endif /* CMWIFI & CMWIFI_EROUTER */

#ifdef CONFIG_DHD_USE_STATIC_BUF

bcm_static_buf_t *bcm_static_buf = 0;
bcm_static_pkt_t *bcm_static_skb = 0;

void* wifi_platform_prealloc(void *adapter, int section, unsigned long size);
#endif /* CONFIG_DHD_USE_STATIC_BUF */

/* Audit the size and alignment of different control blocks in skb structure,
 * used by WL Driver
 *
 * @pkttag_struct_size size of structure wlc_pkttag_t
 * @return 1 for success, 0 for failure
 */
int
osl_skb_audit(unsigned long pkttag_struct_size)
{
	/* Audit skb->cb */

	if (pkttag_struct_size > sizeof(OSL_SKB_CB)) {
		printk("%s:%d - SKB Audit Failed \n", __FUNCTION__, __LINE__);
		return 0;
	}
	if (!ISALIGNED(OSL_SKB_CB, OSL_SKB_CB_ALIGN_SZ)) {
		printk("%s:%d - SKB Audit Failed \n", __FUNCTION__, __LINE__);
		return 0;
	}

#if defined(PKTC) && !(defined(CMWIFI) && defined(CMWIFI_EROUTER))
	/* Audit pktc->cb */

	if (OSL_CHAIN_NODE_SZ > sizeof(OSL_SKB_PKTC_CB)) {
		printk("%s:%d - SKB Audit Failed \n", __FUNCTION__, __LINE__);
		return 0;
	}

	if (!ISALIGNED(OSL_SKB_PKTC_CB, OSL_SKB_CB_ALIGN_SZ)) {
		printk("%s:%d - SKB Audit Failed \n", __FUNCTION__, __LINE__);
		return 0;
	}
#endif /* #if defined(PKTC) && !(defined(CMWIFI) && defined(CMWIFI_EROUTER)) */

	printk("%s: PASS \n", __FUNCTION__);

	return 1;
}

#if defined(CONFIG_BCM_KF_WL) && defined(BCM_PKTFWD)
#define OSL_PKTTAG_CLEAR(p) \
{ \
	struct sk_buff *s = (struct sk_buff *)(p); \
	unsigned long long * wl_cb = (unsigned long long *)&s->wl_cb[0]; \
	__OSL_PKTTAG_CLEAR(p); \
	ASSERT((sizeof(s->wl_cb) == 24)); \
	*(wl_cb + 0) = 0ULL; *(wl_cb + 1) = 0ULL; \
	*(wl_cb + 2) = 0ULL; \
}
#else
#define OSL_PKTTAG_CLEAR  __OSL_PKTTAG_CLEAR
#endif /* CONFIG_BCM_KF_WL */

/* BCM_OBJECT_TRACE - don't clear the first 4 byte that is the pkt sn */
#ifdef BCM_OBJECT_TRACE
#define __OSL_PKTTAG_CLEAR(p) \
do { \
	struct sk_buff *s = (struct sk_buff *)(p); \
	ASSERT(OSL_PKTTAG_SZ == 48); \
	*(uint32 *)(&s->cb[4]) = 0; \
	*(uint32 *)(&s->cb[8]) = 0; *(uint32 *)(&s->cb[12]) = 0; \
	*(uint32 *)(&s->cb[16]) = 0; *(uint32 *)(&s->cb[20]) = 0; \
	*(uint32 *)(&s->cb[24]) = 0; *(uint32 *)(&s->cb[28]) = 0; \
	*(uint32 *)(&s->cb[32]) = 0; *(uint32 *)(&s->cb[36]) = 0; \
	*(uint32 *)(&s->cb[40]) = 0; *(uint32 *)(&s->cb[44]) = 0; \
} while (0)
#elif defined(BCM_OBJECT_TRACE) && defined(WL_EAP_PKTTAG_EXT)
#define __OSL_PKTTAG_CLEAR(p) \
do { \
	struct sk_buff *s = (struct sk_buff *)(p); \
	ASSERT(OSL_PKTTAG_SZ == 56); \
	*(uint32 *)(&s->cb[4]) = 0; \
	*(uint32 *)(&s->cb[8]) = 0; *(uint32 *)(&s->cb[12]) = 0; \
	*(uint32 *)(&s->cb[16]) = 0; *(uint32 *)(&s->cb[20]) = 0; \
	*(uint32 *)(&s->cb[24]) = 0; *(uint32 *)(&s->cb[28]) = 0; \
	*(uint32 *)(&s->cb[32]) = 0; *(uint32 *)(&s->cb[36]) = 0; \
	*(uint32 *)(&s->cb[40]) = 0; *(uint32 *)(&s->cb[44]) = 0; \
	*(uint32 *)(&s->cb[48]) = 0; *(uint32 *)(&s->cb[52]) = 0; \
} while (0)

#elif defined(WL_EAP_PKTTAG_EXT)
#define __OSL_PKTTAG_CLEAR(p) \
do { \
	struct sk_buff *s = (struct sk_buff *)(p); \
	ASSERT(OSL_PKTTAG_SZ == 56); \
	*(uint32 *)(&s->cb[0]) = 0; *(uint32 *)(&s->cb[4]) = 0; \
	*(uint32 *)(&s->cb[8]) = 0; *(uint32 *)(&s->cb[12]) = 0; \
	*(uint32 *)(&s->cb[16]) = 0; *(uint32 *)(&s->cb[20]) = 0; \
	*(uint32 *)(&s->cb[24]) = 0; *(uint32 *)(&s->cb[28]) = 0; \
	*(uint32 *)(&s->cb[32]) = 0; *(uint32 *)(&s->cb[36]) = 0; \
	*(uint32 *)(&s->cb[40]) = 0; *(uint32 *)(&s->cb[44]) = 0; \
	*(uint32 *)(&s->cb[48]) = 0; *(uint32 *)(&s->cb[52]) = 0; \
} while (0)
#else
#define __OSL_PKTTAG_CLEAR(p) \
do { \
	struct sk_buff *s = (struct sk_buff *)(p); \
	ASSERT(OSL_PKTTAG_SZ == 48); \
	*(uint32 *)(&s->cb[0]) = 0; *(uint32 *)(&s->cb[4]) = 0; \
	*(uint32 *)(&s->cb[8]) = 0; *(uint32 *)(&s->cb[12]) = 0; \
	*(uint32 *)(&s->cb[16]) = 0; *(uint32 *)(&s->cb[20]) = 0; \
	*(uint32 *)(&s->cb[24]) = 0; *(uint32 *)(&s->cb[28]) = 0; \
	*(uint32 *)(&s->cb[32]) = 0; *(uint32 *)(&s->cb[36]) = 0; \
	*(uint32 *)(&s->cb[40]) = 0; *(uint32 *)(&s->cb[44]) = 0; \
} while (0)
#endif /* BCM_OBJECT_TRACE */

int osl_static_mem_init(osl_t *osh, void *adapter)
{
#ifdef CONFIG_DHD_USE_STATIC_BUF
		if (!bcm_static_buf && adapter) {
			if (!(bcm_static_buf = (bcm_static_buf_t *)wifi_platform_prealloc(adapter,
				3, STATIC_BUF_SIZE + STATIC_BUF_TOTAL_LEN))) {
				printk("can not alloc static buf!\n");
				bcm_static_skb = NULL;
				ASSERT(osh->magic == OS_HANDLE_MAGIC);
				return -ENOMEM;
			} else {
				printk("alloc static buf at %p!\n", bcm_static_buf);
			}

			spin_lock_init(&bcm_static_buf->static_lock);

			bcm_static_buf->buf_ptr = (unsigned char *)bcm_static_buf + STATIC_BUF_SIZE;
		}

#if defined(DHD_USE_STATIC_CTRLBUF)
		if (!bcm_static_skb && adapter) {
			int i;
			void *skb_buff_ptr = 0;
			bcm_static_skb = (bcm_static_pkt_t *)((char *)bcm_static_buf + 2048);
			skb_buff_ptr = wifi_platform_prealloc(adapter, 4, 0);
			if (!skb_buff_ptr) {
				printk("cannot alloc static buf!\n");
				bcm_static_buf = NULL;
				bcm_static_skb = NULL;
				ASSERT(osh->magic == OS_HANDLE_MAGIC);
				return -ENOMEM;
			}

			bcopy(skb_buff_ptr, bcm_static_skb, sizeof(struct sk_buff *) *
				(STATIC_PKT_MAX_NUM));
			for (i = 0; i < STATIC_PKT_MAX_NUM; i++) {
				bcm_static_skb->pkt_use[i] = 0;
			}

#ifdef DHD_USE_STATIC_CTRLBUF
			spin_lock_init(&bcm_static_skb->osl_pkt_lock);
			bcm_static_skb->last_allocated_index = 0;
#else
			sema_init(&bcm_static_skb->osl_pkt_sem, 1);
#endif /* DHD_USE_STATIC_CTRLBUF */
		}
#endif
#endif /* CONFIG_DHD_USE_STATIC_BUF */

	return 0;
}

int osl_static_mem_deinit(osl_t *osh, void *adapter)
{
#ifdef CONFIG_DHD_USE_STATIC_BUF
	if (bcm_static_buf) {
		bcm_static_buf = 0;
	}
#endif /* CONFIG_DHD_USE_STATIC_BUF */
	return 0;
}

static struct sk_buff * BCMFASTPATH
osl_alloc_skb(osl_t *osh, unsigned int len)
{
	struct sk_buff *skb;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 25)
	gfp_t flags = (in_atomic() || irqs_disabled()) ? GFP_ATOMIC : GFP_KERNEL;
#ifdef DHD_USE_ATOMIC_PKTGET
	flags = GFP_ATOMIC;
#endif /* DHD_USE_ATOMIC_PKTGET */
	skb = __dev_alloc_skb(len, flags);
#else
	skb = dev_alloc_skb(len);
#endif /* LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 25) */

	return skb;
}

/* Reset the SKB header for packets which will be
 * consumed in the wl driver
 */
void
osl_pkt_reset(osl_t *osh, void *p, int max_pkt_bytes)
{
	struct sk_buff *skb = (struct sk_buff *)p;

	ASSERT(skb != NULL);

	/* SKB header recycle */
	skb->data = skb->head + PKT_HEADROOM_DEFAULT;
#ifdef NET_SKBUFF_DATA_USES_OFFSET
	skb_set_tail_pointer(skb, (PKT_HEADROOM_DEFAULT + max_pkt_bytes));
#else
	skb->tail = skb->data + (PKT_HEADROOM_DEFAULT + max_pkt_bytes);
#endif /* NET_SKBUFF_DATA_USES_OFFSET */
	skb->len  = max_pkt_bytes;

	/* clear pkttag */
	if (OSH_PUB(osh).pkttag) {
		OSL_PKTTAG_CLEAR(skb);
	}
}

/* Convert a driver packet to native(OS) packet
 * In the process, packettag is zeroed out before sending up
 * IP code depends on skb->cb to be setup correctly with various options
 * In our case, that means it should be 0
 */
struct sk_buff * BCMFASTPATH
osl_pkt_tonative(osl_t *osh, void *pkt)
{
	struct sk_buff *nskb;
	struct sk_buff *nskb1, *nskb2;
	uint32	pktcnt = 0;

	/* Decrement the packet counter */
	for (nskb = (struct sk_buff *)pkt; nskb; nskb = nskb->next) {

		for (nskb1 = nskb; nskb1 != NULL; nskb1 = nskb2) {
			if (PKTISCHAINED(nskb1)) {
				nskb2 = PKTCLINK(nskb1);
			} else {
				nskb2 = NULL;
			}

			pktcnt++;
			/* clear pkttag inside pkt list */
			if (OSH_PUB(osh).pkttag)
				OSL_PKTTAG_CLEAR(nskb1);

#ifdef BCMDBG_CTRACE
			DEL_CTRACE(osh, nskb1);
#endif /* BCMDBG_CTRACE */
		}
	}

	atomic_sub(pktcnt, &osh->cmn->pktalloced);
	return (struct sk_buff *)pkt;
}

/* Convert a native(OS) packet to driver packet.
 * In the process, native packet is destroyed, there is no copying
 * Also, a packettag is zeroed out
 */
#ifdef BCMDBG_CTRACE
void * BCMFASTPATH
osl_pkt_frmnative(osl_t *osh, void *pkt, int line, char *file)
#else
void * BCMFASTPATH
osl_pkt_frmnative(osl_t *osh, void *pkt)
#endif /* BCMDBG_CTRACE */
{
	struct sk_buff *cskb;
	struct sk_buff *nskb;
	unsigned long pktalloced = 0;

	/* walk the PKTCLINK() list */
	for (cskb = (struct sk_buff *)pkt;
	     cskb != NULL;
	     cskb = PKTISCHAINED(cskb) ? PKTCLINK(cskb) : NULL) {

		/* walk the pkt buffer list */
		for (nskb = cskb; nskb; nskb = nskb->next) {

			/* Increment the packet counter */
			pktalloced++;

			/* clear pkttag inside pkt list */
			if (OSH_PUB(osh).pkttag)
				OSL_PKTTAG_CLEAR(nskb);

			/* clean the 'prev' pointer
			 * Kernel 3.18 is leaving skb->prev pointer set to skb
			 * to indicate a non-fragmented skb
			 */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 18, 0))
			nskb->prev = NULL;
#endif /* LINUX_VERSION_CODE >= KERNEL_VERSION(3, 18, 0) */

#ifdef BCMDBG_CTRACE
			ADD_CTRACE(osh, nskb, file, line);
#endif /* BCMDBG_CTRACE */
		}
	}

	/* Increment the packet counter */
	atomic_add(pktalloced, &osh->cmn->pktalloced);

	return (void *)pkt;
}

/* Return a new packet. zero out pkttag */
#ifdef BCMDBG_CTRACE
void * BCMFASTPATH
linux_pktget(osl_t *osh, uint len, int line, char *file)
#else
#ifdef BCM_OBJECT_TRACE
void * BCMFASTPATH
linux_pktget(osl_t *osh, uint len, int line, const char *caller)
#else
void * BCMFASTPATH
linux_pktget(osl_t *osh, uint len)
#endif /* BCM_OBJECT_TRACE */
#endif /* BCMDBG_CTRACE */
{
	struct sk_buff *skb;
	uchar num = 0;
	if (lmtest != FALSE) {
		get_random_bytes(&num, sizeof(uchar));
		if ((num + 1) <= (256 * lmtest / 100))
			return NULL;
	}

#if defined(CMWIFI) && defined(CMWIFI_EROUTER) && !defined(BCMDONGLEHOST)
	if ((skb = wlerouter_alloc_skb(osh->pdev, len, false))) {
#else
	if ((skb = osl_alloc_skb(osh, len))) {
#endif /* CMWIFI && CMWIFI_EROUTER && !BCMDONGLEHOST */
#ifdef BCMDBG
		skb_put(skb, len);
#else
		skb->tail += len;
		skb->len  += len;
#endif
		skb->priority = 0;

#ifdef BCMDBG_CTRACE
		ADD_CTRACE(osh, skb, file, line);
#endif
		atomic_inc(&osh->cmn->pktalloced);
#ifdef BCM_OBJECT_TRACE
		bcm_object_trace_opr(skb, BCM_OBJDBG_ADD_PKT, caller, line);
#endif /* BCM_OBJECT_TRACE */
	}

	return ((void*) skb);
}

#if defined(CMWIFI) && defined(CMWIFI_EROUTER)
/* Return a new packet. zero out pkttag */
#if defined(BCMDBG_CTRACE)
void * BCMFASTPATH
osl_pktget_data(osl_t *osh, uint len, int line, char *file)
#elif defined(BCM_OBJECT_TRACE)
void * BCMFASTPATH
osl_pktget_data(osl_t *osh, uint len, int line, const char *caller)
#else
void * BCMFASTPATH
osl_pktget_data(osl_t *osh, uint len)
#endif
{
	struct sk_buff *skb;
#if defined(BCMDONGLEHOST)
	if (wler_is_accelerated(0, OSL_PCIE_BUS(osh), false)) {
#endif /* BCMDONGLEHOST */
		if ((skb = wlerouter_alloc_skb(osh->pdev, len, false))) {
			skb->tail += len;
			skb->len  += len;
			skb->priority = 0;

#ifdef BCMDBG_CTRACE
			ADD_CTRACE(osh, skb, file, line);
#endif
			atomic_inc(&osh->cmn->pktalloced);
		}
		return ((void*) skb);
#if defined(BCMDONGLEHOST)
	}

	return ((void*)linux_pktget(osh, len));
#endif /* BCMDONGLEHOST */
}
#endif /* CMWIFI && CMWIFI_EROUTER */

static void BCMFASTPATH
linux_pktfree_callbacks(osl_t *osh, struct sk_buff *skb, bool send)
{
	struct sk_buff *nskb;

	BCM_REFERENCE(nskb);
	if (send) {
		if (OSH_PUB(osh).tx_fn) {
			OSH_PUB(osh).tx_fn(OSH_PUB(osh).tx_ctx, skb, 0);
		}
	} else {
		if (OSH_PUB(osh).rx_fn) {
			OSH_PUB(osh).rx_fn(OSH_PUB(osh).rx_ctx, skb);
		}
	}

	PKTDBG_TRACE(osh, (void *) skb, PKTLIST_PKTFREE);

#if defined(CONFIG_DHD_USE_STATIC_BUF) && defined(DHD_USE_STATIC_CTRLBUF)
	if (skb && (skb->mac_len == PREALLOC_USED_MAGIC)) {
		printk("%s: pkt %p is from static pool\n",
			__FUNCTION__, skb);
		dump_stack();
		return;
	}

	if (skb && (skb->mac_len == PREALLOC_FREE_MAGIC)) {
		printk("%s: pkt %p is from static pool and not in used\n",
			__FUNCTION__, skb);
		dump_stack();
		return;
	}
#endif /* CONFIG_DHD_USE_STATIC_BUF && DHD_USE_STATIC_CTRLBUF */
}

#ifdef BPM_BULK_FREE
/* Free the driver packet. Free the tag if present */
#ifdef BCM_OBJECT_TRACE
void BCMFASTPATH
linux_pktfree_slow(osl_t *osh, void *p, bool docb, bool send, int line, const char *caller)
#else
void BCMFASTPATH
linux_pktfree_slow(osl_t *osh, void *p, bool docb, bool send)
#endif /* BCM_OBJECT_TRACE */
{
	struct sk_buff *skb, *nskb;
	uint32	pktcnt = 0;

	if (osh == NULL)
		return;

	skb = (struct sk_buff*) p;

	if (docb) {
		linux_pktfree_callbacks(osh, skb, send);
	}

	/* perversion: we use skb->next to chain multi-skb packets */
	while (skb) {

		pktcnt++;
		nskb = skb->next;
		skb->next = NULL;

#ifdef BCMDBG_CTRACE
		DEL_CTRACE(osh, skb);
#endif

#ifdef BCM_OBJECT_TRACE
		bcm_object_trace_opr(skb, BCM_OBJDBG_REMOVE, caller, line);
#endif /* BCM_OBJECT_TRACE */

#if defined(CMWIFI) && defined(CMWIFI_EROUTER)
		if (wlerouter_free_skb(skb) < 0)
#endif
		{
			if (skb->destructor) {
				/* cannot kfree_skb() on hard IRQ (net/core/skbuff.c) if
				 * destructor exists
				 */
				dev_kfree_skb_any(skb);
			} else {
				/* can free immediately (even in_irq()) if destructor
				 * does not exist
				 */
				dev_kfree_skb(skb);
			}
		}

		skb = nskb;
	}

	atomic_sub(pktcnt, &osh->cmn->pktalloced);
}

void
linux_commit_skb_freelist(osl_t *osh)
{
	/* commit pending skbs (if any) to skbfree thread */
	if (BCM_SKB_FREE_OFFLOAD_ENAB(osh) && OSH_PUB(osh).skbfreelist.len) {

		atomic_sub(OSH_PUB(osh).skbfreelist.len, &osh->cmn->pktalloced);
		dev_kfree_skb_thread_bulk(
			OSH_PUB(osh).skbfreelist.head,
			OSH_PUB(osh).skbfreelist.tail,
			OSH_PUB(osh).skbfreelist.len);
		OSH_PUB(osh).skbfreelist.head = NULL;
		OSH_PUB(osh).skbfreelist.tail = NULL;
		OSH_PUB(osh).skbfreelist.len = 0;
	}
}
#endif /* BPM_BULK_FREE */

/*
 * BCM_OBJECT_TRACE requires a caller site to be an argument. Instead of a bunch
 * of ifdefs and elses around the function definition and call sites, let's just
 * use a macro for the optional argument.
 */
#ifdef BCM_OBJECT_TRACE
#define BCM_OBJECT_TRACE_ARGS line, caller
#else
#define BCM_OBJECT_TRACE_ARGS 0, NULL
#endif

static inline void BCMFASTPATH
linux_pktfree_do_traces(osl_t *osh, struct sk_buff *skb, int line, const char *caller)
{

	BCM_REFERENCE(line);
	BCM_REFERENCE(caller);
#ifdef BCMDBG_CTRACE
	DEL_CTRACE(osh, skb);
#endif

#ifdef BCM_OBJECT_TRACE
	bcm_object_trace_opr(skb, BCM_OBJDBG_REMOVE, caller, line);
#endif /* BCM_OBJECT_TRACE */
}

#ifdef BCM_SKB_FREE_OFFLOAD
#ifdef BPM_BULK_FREE
static inline void BCMFASTPATH
linux_pktfree_offload_bulk(osl_t *osh, struct sk_buff *first, struct sk_buff *last,
		uint32 pktcnt)
{
	OSH_PUB(osh).skbfreelist.len += pktcnt;

	if (OSH_PUB(osh).skbfreelist.tail) {
		OSH_PUB(osh).skbfreelist.tail->next = first;
	} else {
		OSH_PUB(osh).skbfreelist.head = first;
	}
	OSH_PUB(osh).skbfreelist.tail = last;

	/* Commit to skbfree thread only after budget
	 * no of skb or wl down is triggered
	 */
	if (OSH_PUB(osh).wl_going_down ||
		(OSH_PUB(osh).skbfreelist.len >= BPM_SKB_FREE_BUDGET)) {

		atomic_sub(OSH_PUB(osh).skbfreelist.len, &osh->cmn->pktalloced);
	        dev_kfree_skb_thread_bulk(
			OSH_PUB(osh).skbfreelist.head,
			OSH_PUB(osh).skbfreelist.tail,
			OSH_PUB(osh).skbfreelist.len);
		OSH_PUB(osh).skbfreelist.head = NULL;
		OSH_PUB(osh).skbfreelist.tail = NULL;
		OSH_PUB(osh).skbfreelist.len = 0;
	}
}
#endif /* BPM_BULK_FREE */
static inline void BCMFASTPATH
linux_pktfree_offload(osl_t *osh, struct sk_buff *skb, int line, const char *caller)
{
	struct sk_buff *fskb, *pskb;
	uint32	pktcnt = 0;

	BCM_REFERENCE(line);
	BCM_REFERENCE(caller);
	fskb = skb;

	/* perversion: we use skb->next to chain multi-skb packets */
	while (skb) {
		pskb = skb;
		pktcnt++;
		linux_pktfree_do_traces(osh, skb, BCM_OBJECT_TRACE_ARGS);
		skb = skb->next;
	}

#ifdef BPM_BULK_FREE
	linux_pktfree_offload_bulk(osh, fskb, pskb, pktcnt);
#else
	atomic_sub(pktcnt, &osh->cmn->pktalloced);
	dev_kfree_skb_thread_bulk(fskb);
#endif /* BPM_BULK_FREE */
}
#endif /* BCM_SKB_FREE_OFFLOAD */

/* Free the driver packet. Free the tag if present */
#ifdef BCM_OBJECT_TRACE
void BCMFASTPATH
linux_pktfree(osl_t *osh, void *p, bool docb, bool send, int line, const char *caller)
#else
void BCMFASTPATH
linux_pktfree(osl_t *osh, void *p, bool docb, bool send)
#endif /* BCM_OBJECT_TRACE */
{
	struct sk_buff *skb, *nskb;
	uint32	pktcnt = 0;

#ifdef BCM_BLOG
	if (IS_FKBUFF_PTR(p)) {
		nbuff_free((pNBuff_t)p);
		return;
	}
#endif

	if (osh == NULL)
		return;

	ASSERT(p);
	skb = (struct sk_buff *) p;

	if (docb) {
		linux_pktfree_callbacks(osh, skb, send);
	}

#ifdef BCM_SKB_FREE_OFFLOAD
	if (BCM_SKB_FREE_OFFLOAD_ENAB(osh)) {
		linux_pktfree_offload(osh, skb, BCM_OBJECT_TRACE_ARGS);
		return;
	}
#endif

	/* perversion: we use skb->next to chain multi-skb packets */
	while (skb) {
		pktcnt++;
		nskb = skb->next;

		linux_pktfree_do_traces(osh, skb, BCM_OBJECT_TRACE_ARGS);

#if defined(CMWIFI) && defined(CMWIFI_EROUTER)
		if (wlerouter_free_skb(skb) < 0)
#endif
		{
			if (skb->destructor) {
				/* cannot kfree_skb() on hard irq
				 * (net/core/skbuff.c) if
				 * destructor exists
				 */
				dev_kfree_skb_any(skb);
			} else {
				/* can free immediately (even in_irq())
				 * if destructor does not exist
				 */
				dev_kfree_skb(skb);
			}
		}

		skb = nskb;
	}
	atomic_sub(pktcnt, &osh->cmn->pktalloced);
}

#ifdef CONFIG_DHD_USE_STATIC_BUF
void*
osl_pktget_static(osl_t *osh, uint len)
{
	int i = 0;
	struct sk_buff *skb;
#ifdef DHD_USE_STATIC_CTRLBUF
	unsigned long flags;
#endif /* DHD_USE_STATIC_CTRLBUF */

	if (!bcm_static_skb)
		return linux_pktget(osh, len);

	if (len > DHD_SKB_MAX_BUFSIZE) {
		printk("%s: attempt to allocate huge packet (0x%x)\n", __FUNCTION__, len);
		return linux_pktget(osh, len);
	}

#ifdef DHD_USE_STATIC_CTRLBUF
	spin_lock_irqsave(&bcm_static_skb->osl_pkt_lock, flags);

	if (len <= DHD_SKB_2PAGE_BUFSIZE) {
		uint32 index;
		for (i = 0; i < STATIC_PKT_2PAGE_NUM; i++) {
			index = bcm_static_skb->last_allocated_index % STATIC_PKT_2PAGE_NUM;
			bcm_static_skb->last_allocated_index++;
			if (bcm_static_skb->skb_8k[index] &&
				bcm_static_skb->pkt_use[index] == 0) {
				break;
			}
		}

		if ((i != STATIC_PKT_2PAGE_NUM) &&
			(index >= 0) && (index < STATIC_PKT_2PAGE_NUM)) {
			bcm_static_skb->pkt_use[index] = 1;
			skb = bcm_static_skb->skb_8k[index];
			skb->data = skb->head;
#ifdef NET_SKBUFF_DATA_USES_OFFSET
			skb_set_tail_pointer(skb, PKT_HEADROOM_DEFAULT);
#else
			skb->tail = skb->data + PKT_HEADROOM_DEFAULT;
#endif /* NET_SKBUFF_DATA_USES_OFFSET */
			skb->data += PKT_HEADROOM_DEFAULT;
			skb->cloned = 0;
			skb->priority = 0;
#ifdef NET_SKBUFF_DATA_USES_OFFSET
			skb_set_tail_pointer(skb, len);
#else
			skb->tail = skb->data + len;
#endif /* NET_SKBUFF_DATA_USES_OFFSET */
			skb->len = len;
			skb->mac_len = PREALLOC_USED_MAGIC;
			spin_unlock_irqrestore(&bcm_static_skb->osl_pkt_lock, flags);
			return skb;
		}
	}

	spin_unlock_irqrestore(&bcm_static_skb->osl_pkt_lock, flags);
	printk("%s: all static pkt in use!\n", __FUNCTION__);
	return NULL;
#else
	down(&bcm_static_skb->osl_pkt_sem);

	if (len <= DHD_SKB_1PAGE_BUFSIZE) {
		for (i = 0; i < STATIC_PKT_1PAGE_NUM; i++) {
			if (bcm_static_skb->skb_4k[i] &&
				bcm_static_skb->pkt_use[i] == 0) {
				break;
			}
		}

		if (i != STATIC_PKT_1PAGE_NUM) {
			bcm_static_skb->pkt_use[i] = 1;

			skb = bcm_static_skb->skb_4k[i];
#ifdef NET_SKBUFF_DATA_USES_OFFSET
			skb_set_tail_pointer(skb, len);
#else
			skb->tail = skb->data + len;
#endif /* NET_SKBUFF_DATA_USES_OFFSET */
			skb->len = len;

			up(&bcm_static_skb->osl_pkt_sem);
			return skb;
		}
	}

	if (len <= DHD_SKB_2PAGE_BUFSIZE) {
		for (i = STATIC_PKT_1PAGE_NUM; i < STATIC_PKT_1_2PAGE_NUM; i++) {
			if (bcm_static_skb->skb_8k[i - STATIC_PKT_1PAGE_NUM] &&
				bcm_static_skb->pkt_use[i] == 0) {
				break;
			}
		}

		if ((i >= STATIC_PKT_1PAGE_NUM) && (i < STATIC_PKT_1_2PAGE_NUM)) {
			bcm_static_skb->pkt_use[i] = 1;
			skb = bcm_static_skb->skb_8k[i - STATIC_PKT_1PAGE_NUM];
#ifdef NET_SKBUFF_DATA_USES_OFFSET
			skb_set_tail_pointer(skb, len);
#else
			skb->tail = skb->data + len;
#endif /* NET_SKBUFF_DATA_USES_OFFSET */
			skb->len = len;

			up(&bcm_static_skb->osl_pkt_sem);
			return skb;
		}
	}

#if defined(ENHANCED_STATIC_BUF)
	if (bcm_static_skb->skb_16k &&
		bcm_static_skb->pkt_use[STATIC_PKT_MAX_NUM - 1] == 0) {
		bcm_static_skb->pkt_use[STATIC_PKT_MAX_NUM - 1] = 1;

		skb = bcm_static_skb->skb_16k;
#ifdef NET_SKBUFF_DATA_USES_OFFSET
		skb_set_tail_pointer(skb, len);
#else
		skb->tail = skb->data + len;
#endif /* NET_SKBUFF_DATA_USES_OFFSET */
		skb->len = len;

		up(&bcm_static_skb->osl_pkt_sem);
		return skb;
	}
#endif /* ENHANCED_STATIC_BUF */

	up(&bcm_static_skb->osl_pkt_sem);
	printk("%s: all static pkt in use!\n", __FUNCTION__);
	return linux_pktget(osh, len);
#endif /* DHD_USE_STATIC_CTRLBUF */
}

void
osl_pktfree_static(osl_t *osh, void *p, bool send)
{
	int i;
#ifdef DHD_USE_STATIC_CTRLBUF
	struct sk_buff *skb = (struct sk_buff *)p;
	unsigned long flags;
#endif /* DHD_USE_STATIC_CTRLBUF */

	if (!p) {
		return;
	}

	if (!bcm_static_skb) {
		linux_pktfree(osh, p, send);
		return;
	}

#ifdef DHD_USE_STATIC_CTRLBUF
	spin_lock_irqsave(&bcm_static_skb->osl_pkt_lock, flags);

	for (i = 0; i < STATIC_PKT_2PAGE_NUM; i++) {
		if (p == bcm_static_skb->skb_8k[i]) {
			if (bcm_static_skb->pkt_use[i] == 0) {
				printk("%s: static pkt idx %d(%p) is double free\n",
					__FUNCTION__, i, p);
			} else {
				bcm_static_skb->pkt_use[i] = 0;
			}

			if (skb->mac_len != PREALLOC_USED_MAGIC) {
				printk("%s: static pkt idx %d(%p) is not in used\n",
					__FUNCTION__, i, p);
			}

			skb->mac_len = PREALLOC_FREE_MAGIC;
			spin_unlock_irqrestore(&bcm_static_skb->osl_pkt_lock, flags);
			return;
		}
	}

	spin_unlock_irqrestore(&bcm_static_skb->osl_pkt_lock, flags);
	printk("%s: packet %p does not exist in the pool\n", __FUNCTION__, p);
#else
	down(&bcm_static_skb->osl_pkt_sem);
	for (i = 0; i < STATIC_PKT_1PAGE_NUM; i++) {
		if (p == bcm_static_skb->skb_4k[i]) {
			bcm_static_skb->pkt_use[i] = 0;
			up(&bcm_static_skb->osl_pkt_sem);
			return;
		}
	}

	for (i = STATIC_PKT_1PAGE_NUM; i < STATIC_PKT_1_2PAGE_NUM; i++) {
		if (p == bcm_static_skb->skb_8k[i - STATIC_PKT_1PAGE_NUM]) {
			bcm_static_skb->pkt_use[i] = 0;
			up(&bcm_static_skb->osl_pkt_sem);
			return;
		}
	}
#ifdef ENHANCED_STATIC_BUF
	if (p == bcm_static_skb->skb_16k) {
		bcm_static_skb->pkt_use[STATIC_PKT_MAX_NUM - 1] = 0;
		up(&bcm_static_skb->osl_pkt_sem);
		return;
	}
#endif
	up(&bcm_static_skb->osl_pkt_sem);
#endif /* DHD_USE_STATIC_CTRLBUF */
	linux_pktfree(osh, p, send);
}
#endif /* CONFIG_DHD_USE_STATIC_BUF */

/* Clone a packet.
 * The pkttag contents are NOT cloned.
 */
#ifdef BCMDBG_CTRACE
void *
osl_pktdup(osl_t *osh, void *skb, int line, char *file)
#else
#ifdef BCM_OBJECT_TRACE
void *
osl_pktdup(osl_t *osh, void *skb, int line, const char *caller)
#else
void *
osl_pktdup(osl_t *osh, void *skb)
#endif /* BCM_OBJECT_TRACE */
#endif /* BCMDBG_CTRACE */
{
	void * p;

	ASSERT(!PKTISCHAINED(skb));

	/* clear the CTFBUF flag if set and map the rest of the buffer
	 * before cloning.
	 */
	PKTCTFMAP(osh, skb);

#if defined(CMWIFI) && defined(CMWIFI_EROUTER) && !defined(CMWIFI_33940)
	if (wler_is_native_buffer((u32)skb)) {
		if ((p = wler_clone_skbuf((void *)skb)) == NULL)
			return NULL;
	} else {
#endif /* CMWIFI && CMWIFI_EROUTER && !CMWIFI_33940 */

	if ((p = skb_clone((struct sk_buff *)skb, GFP_ATOMIC)) == NULL)
		return NULL;

#if defined(CMWIFI) && defined(CMWIFI_EROUTER) && !defined(CMWIFI_33940)
	}
#endif /* CMWIFI && CMWIFI_EROUTER && !CMWIFI_33940 */

	/* skb_clone copies skb->cb.. we don't want that */
	if (OSH_PUB(osh).pkttag)
		OSL_PKTTAG_CLEAR(p);

	/* Increment the packet counter */
	atomic_inc(&osh->cmn->pktalloced);
#ifdef BCM_OBJECT_TRACE
	bcm_object_trace_opr(p, BCM_OBJDBG_ADD_PKT, caller, line);
#endif /* BCM_OBJECT_TRACE */

#ifdef BCMDBG_CTRACE
	ADD_CTRACE(osh, (struct sk_buff *)p, file, line);
#endif
	return (p);
}

/* Copy a packet.
 * The pkttag contents are NOT cloned.
 */
#ifdef BCMDBG_CTRACE
void *
osl_pktdup_cpy(osl_t *osh, void *skb, int line, char *file)
#else
#ifdef BCM_OBJECT_TRACE
void *
osl_pktdup_cpy(osl_t *osh, void *skb, int line, const char *caller)
#else
void *
osl_pktdup_cpy(osl_t *osh, void *skb)
#endif /* BCM_OBJECT_TRACE */
#endif /* BCMDBG_CTRACE */
{
	void * p;
#if defined(BCM_BLOG) && defined(BCM_WLAN_PER_CLIENT_FLOW_LEARNING)
	struct blog_t *blog_p = ((struct sk_buff *)skb)->blog_p;
#endif

	ASSERT(!PKTISCHAINED(skb));

	/* clear the CTFBUF flag if set and map the rest of the buffer
	 * before cloning.
	 */
	PKTCTFMAP(osh, skb);

	if ((p = pskb_copy((struct sk_buff *)skb, GFP_ATOMIC)) == NULL)
		return NULL;

	/* Clear PKTC  context */
	PKTSETCLINK(p, NULL);
	PKTCCLRFLAGS(p);
	PKTCSETCNT(p, 1);
	PKTCSETLEN(p, PKTLEN(osh, skb));

	/* skb_clone copies skb->cb.. we don't want that */
	if (OSH_PUB(osh).pkttag)
		OSL_PKTTAG_CLEAR(p);

	/* Increment the packet counter */
	atomic_inc(&osh->cmn->pktalloced);
#ifdef BCM_OBJECT_TRACE
	bcm_object_trace_opr(p, BCM_OBJDBG_ADD_PKT, caller, line);
#endif /* BCM_OBJECT_TRACE */

#ifdef BCMDBG_CTRACE
	ADD_CTRACE(osh, (struct sk_buff *)p, file, line);
#endif
#if defined(BCM_BLOG) && defined(BCM_WLAN_PER_CLIENT_FLOW_LEARNING)
	if (blog_p) {
		/* pskb_cpy transfer blog to coped skb, to allocate
		 * a new blog and assigned to original skb
		 */
#ifndef BCM_WLAN_MCAST_BLOG_SHARED_INFO
		if (blog_skb((struct sk_buff *)skb) != NULL)
			blog_copy(((struct sk_buff *)skb)->blog_p, ((struct sk_buff *)p)->blog_p);
		else
			ASSERT(((struct sk_buff *)skb)->blog_p != NULL);
#else
		wl_mcast_blog_clone_handler(skb, p);
#endif
	}
#endif  /* BCM_BLOG && BCM_WLAN_PER_CLIENT_FLOW_LEARNING */
	return (p);
}

#ifdef BCMDBG_CTRACE
int osl_pkt_is_frmnative(osl_t *osh, struct sk_buff *pkt)
{
	unsigned long flags;
	struct sk_buff *skb;
	int ck = FALSE;

	spin_lock_irqsave(&osh->ctrace_lock, flags);

	list_for_each_entry(skb, &osh->ctrace_list, ctrace_list) {
		if (pkt == skb) {
			ck = TRUE;
			break;
		}
	}

	spin_unlock_irqrestore(&osh->ctrace_lock, flags);
	return ck;
}

void osl_ctrace_dump(osl_t *osh, struct bcmstrbuf *b)
{
	unsigned long flags;
	struct sk_buff *skb;
	int idx = 0;
	int i, j;

	spin_lock_irqsave(&osh->ctrace_lock, flags);

	if (b != NULL)
		bcm_bprintf(b, " Total %d sbk not free\n", osh->ctrace_num);
	else
		printk(" Total %d sbk not free\n", osh->ctrace_num);

	list_for_each_entry(skb, &osh->ctrace_list, ctrace_list) {
		if (b != NULL)
			bcm_bprintf(b, "[%d] skb %p:\n", ++idx, skb);
		else
			printk("[%d] skb %p:\n", ++idx, skb);

		for (i = 0; i < skb->ctrace_count; i++) {
			j = (skb->ctrace_start + i) % CTRACE_NUM;
			if (b != NULL)
				bcm_bprintf(b, "    [%s(%d)]\n", skb->func[j], skb->line[j]);
			else
				printk("    [%s(%d)]\n", skb->func[j], skb->line[j]);
		}
		if (b != NULL)
			bcm_bprintf(b, "\n");
		else
			printk("\n");
	}

	spin_unlock_irqrestore(&osh->ctrace_lock, flags);

	return;
}
#endif /* BCMDBG_CTRACE */

/*
 * BINOSL selects the slightly slower function-call-based binary compatible osl.
 */
#ifdef BINOSL
bool
osl_pktshared(void *skb)
{
	return (((struct sk_buff*)skb)->cloned);
}

uchar*
osl_pktdata(osl_t *osh, void *skb)
{
	return (((struct sk_buff*)skb)->data);
}

uint
osl_pktlen(osl_t *osh, void *skb)
{
	return (((struct sk_buff*)skb)->len);
}

uint
osl_pktheadroom(osl_t *osh, void *skb)
{
	return (uint) skb_headroom((struct sk_buff *) skb);
}

uint
osl_pkttailroom(osl_t *osh, void *skb)
{
	return (uint) skb_tailroom((struct sk_buff *) skb);
}

void*
osl_pktnext(osl_t *osh, void *skb)
{
	return (((struct sk_buff*)skb)->next);
}

void
osl_pktsetnext(void *skb, void *x)
{
	((struct sk_buff*)skb)->next = (struct sk_buff*)x;
}

void
osl_pktsetlen(osl_t *osh, void *skb, uint len)
{
	__skb_trim((struct sk_buff*)skb, len);
}

uchar*
osl_pktpush(osl_t *osh, void *skb, int bytes)
{
	return (skb_push((struct sk_buff*)skb, bytes));
}

uchar*
osl_pktpull(osl_t *osh, void *skb, int bytes)
{
	return (skb_pull((struct sk_buff*)skb, bytes));
}

void*
osl_pkttag(void *skb)
{
	return ((void*)(((struct sk_buff*)skb)->cb));
}

void*
osl_pktlink(void *skb)
{
	return (((struct sk_buff*)skb)->prev);
}

void
osl_pktsetlink(void *skb, void *x)
{
	((struct sk_buff*)skb)->prev = (struct sk_buff*)x;
}

uint
osl_pktprio(void *skb)
{
	return (((struct sk_buff*)skb)->priority);
}

void
osl_pktsetprio(void *skb, uint x)
{
	((struct sk_buff*)skb)->priority = x;
}
#endif	/* BINOSL */

uint
osl_pktalloced(osl_t *osh)
{
	return (atomic_read(&osh->cmn->pktalloced));
}

uint
osl_pktalloced_persistently(osl_t *osh)
{
	return (atomic_read(&osh->cmn->persistently));
}

void
osl_pktalloced_persistently_inc(osl_t *osh, uint cnt)
{
	atomic_add(cnt, &osh->cmn->persistently);
}

void
osl_pktalloced_persistently_dec(osl_t *osh, uint cnt)
{
	atomic_sub(cnt, &osh->cmn->persistently);
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 6, 0) && defined(TSQ_MULTIPLIER)
#include <linux/kallsyms.h>
#include <net/sock.h>
void
osl_pkt_orphan_partial(struct sk_buff *skb)
{
	uint32 fraction;
	static void *p_tcp_wfree = NULL;

	if (!skb->destructor || skb->destructor == sock_wfree)
		return;

	if (unlikely(!p_tcp_wfree)) {
		char sym[KSYM_SYMBOL_LEN];
		sprint_symbol(sym, (unsigned long)skb->destructor);
		sym[9] = 0;
		if (!strcmp(sym, "tcp_wfree"))
			p_tcp_wfree = skb->destructor;
		else
			return;
	}

	if (unlikely(skb->destructor != p_tcp_wfree || !skb->sk))
		return;

	/* abstract a certain portion of skb truesize from the socket
	 * sk_wmem_alloc to allow more skb can be allocated for this
	 * socket for better cusion meeting WiFi device requirement
	 */
	fraction = skb->truesize * (TSQ_MULTIPLIER - 1) / TSQ_MULTIPLIER;
	skb->truesize -= fraction;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 85)
	atomic_sub(fraction, (atomic_t *)&skb->sk->sk_wmem_alloc);
#else
	atomic_sub(fraction, &skb->sk->sk_wmem_alloc);
#endif /* LINUX_VERSION >= 5.4.85 */
}
#endif /* LINUX_VERSION >= 3.6.0 && TSQ_MULTIPLIER */

#ifdef BCM_BLOG
uint
osl_pktprio(void *p)
{
	uint32 prio;
	prio = ((struct sk_buff*)p)->mark>>PRIO_LOC_NFMARK & 0x7;

	if (prio > 7)
		printk("osl_pktprio: wrong prio (0x%x) !!!\n", prio);
	return prio;
}

void
osl_pktsetprio(void *p, uint x)
{
	((struct sk_buff*)p)->mark &= ~(0x7 << PRIO_LOC_NFMARK);
	((struct sk_buff*)p)->mark |= (x & 0x7) << PRIO_LOC_NFMARK;
}
#endif /* BCM_BLOG */

void
osl_pkt_account(osl_t *osh, int skb_cnt, bool add)
{
	if (add)
		atomic_add(skb_cnt, &osh->cmn->pktalloced);
	else
		atomic_sub(skb_cnt, &osh->cmn->pktalloced);
}

/* use platform specific packet free to skb and non-skb data packet on list
 * (e.g. fpm buffers for cmwifi platform)
 */
void
osl_pkt_queue_purge(osl_t *osh, struct sk_buff_head * list)
{
#if defined(CMWIFI) && defined(CMWIFI_EROUTER)
	struct sk_buff *skb;
	while ((skb = skb_dequeue(list)) != NULL) {
		(void)wlerouter_free_skb(skb);
	}
#else
	skb_queue_purge(list);
#endif /* CMWIFI */
}
