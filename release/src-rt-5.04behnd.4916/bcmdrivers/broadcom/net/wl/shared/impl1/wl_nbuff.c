/*
    Copyright (c) 2017 Broadcom
    All Rights Reserved

    <:label-BRCM:2017:DUAL/GPL:standard

    Unless you and Broadcom execute a separate written software license
    agreement governing use of this software, this software is licensed
    to you under the terms of the GNU General Public License version 2
    (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
    with the following added to such license:

       As a special exception, the copyright holders of this software give
       you permission to link this software with independent modules, and
       to copy and distribute the resulting executable under terms of your
       choice, provided that you also meet, for each linked independent
       module, the terms and conditions of the license of that module.
       An independent module is a module which is not derived from this
       software.  The special exception does not apply to any modifications
       of the software.

    Not withstanding the above, under no circumstances may you combine
    this software in any way with any other Broadcom software provided
    under a license other than the GPL, without Broadcom's express prior
    written consent.

    :>
*/

#if defined(BCM_NBUFF)

#include <linux/kernel.h>
#include <linux/nbuff.h>
#include <linux/blog.h>
#include <osl_decl.h>
#include <bcmutils.h>

#include <nbuff_pkt.h>

/**
 * ------------------------------------------------------------------------------------------------
 *  SECTION: GENERIC NBUFF (SKB|WKB|FKB) Packet Primitives - PKTxyz
 * ------------------------------------------------------------------------------------------------
 */

uint
nbuff_pktprio(void *pkt)
{
	uint32 prio = 0;

	if (IS_SKBUFF_PTR(pkt)) {
		prio = PNBUFF_2_SKBUFF(pkt)->mark >> PRIO_LOC_NFMARK & 0x7;
#if defined(BCM_WKB)
	} else if (IS_WKBUFF_PTR(pkt)) {
		prio = wkb_prio(PNBUFF_2_WKBUFF(pkt));
#endif /* BCM_WKB */
	} else {		/* manuplated in dhd, 3bit prio + 10bit flowid */
		FkBuff_t *fkb_p = PNBUFF_2_FKBUFF(pkt);
		prio = fkb_p->wl.ucast.dhd.wl_prio;	/* Same bit pos used for mcast */
	}

	if (prio > 7) {
		prio = 0;
	}

	return prio;
}

void
nbuff_pktsetprio(void *pkt, uint prio)
{
	if (IS_SKBUFF_PTR(pkt)) {
		struct sk_buff *skb = PNBUFF_2_SKBUFF(pkt);
		skb->mark &= ~(0x7 << PRIO_LOC_NFMARK);
		skb->mark |= (prio & 0x7) << PRIO_LOC_NFMARK;
		skb->wl.ucast.nic.wl_prio = prio;
#if defined(BCM_WKB)
	} else if (IS_WKBUFF_PTR(pkt)) {
		wkb_set_prio(PNBUFF_2_WKBUFF(pkt), prio);
#endif /* BCM_WKB */
	} else {
		FkBuff_t *fkb_p = PNBUFF_2_FKBUFF(pkt);
		fkb_p->wl.ucast.dhd.wl_prio = prio;	/* Same bit pos used for mcast */
	}
}

uchar *
nbuff_pktdata(osl_t *osh, void *pkt)
{
#if defined(BCM_WKB)
	if (IS_WKBUFF_PTR(pkt)) {
		return wkb_data(PNBUFF_2_WKBUFF(pkt));
	} else
#endif /* BCM_WKB */
	{
		return nbuff_get_data(pkt);
	}
}

void
nbuff_pktsetdata(osl_t *osh, void *pkt, uint8 *data)
{
#if defined(BCM_WKB)
	if (IS_WKBUFF_PTR(pkt)) {
		wkb_set_data(PNBUFF_2_WKBUFF(pkt), data);
	} else
#endif /* BCM_WKB */
	{
		printk("%s: no such operation for skb|fkb!\n", __FUNCTION__);
		ASSERT(0);
	}
}

void *
nbuff_pkthead(osl_t *osh, void *pkt)
{
	if (IS_SKBUFF_PTR(pkt)) {
		return PNBUFF_2_SKBUFF(pkt)->head;
#if defined(BCM_WKB)
	} else if (IS_WKBUFF_PTR(pkt)) {
		return wkb_head(PNBUFF_2_WKBUFF(pkt));
#endif /* BCM_WKB */
	} else {
		printk("%s: no such operation for fkb!\n", __FUNCTION__);
		return 0;
	}
}

uint
nbuff_pktlen(osl_t *osh, void *pkt)
{
#if defined(BCM_WKB)
	if (IS_WKBUFF_PTR(pkt)) {
		return (uint) wkb_len(PNBUFF_2_WKBUFF(pkt));
	} else
#endif /* BCM_WKB */
	{
		return nbuff_get_len(pkt);
	}
}

void
nbuff_pktsetlen(osl_t *osh, void *pkt, uint len)
{
	if (IS_SKBUFF_PTR(pkt)) {
		__pskb_trim(PNBUFF_2_SKBUFF(pkt), len);
#if defined(BCM_WKB)
	} else if (IS_WKBUFF_PTR(pkt)) {
		wkb_set_len(PNBUFF_2_WKBUFF(pkt), len);
#endif /* BCM_WKB */
	} else {
		nbuff_set_len(pkt, len);
	}
}

uint
nbuff_pktheadroom(osl_t *osh, void *pkt)
{
	if (IS_SKBUFF_PTR(pkt)) {
		return (uint) skb_headroom(PNBUFF_2_SKBUFF(pkt));
#if defined(BCM_WKB)
	} else if (IS_WKBUFF_PTR(pkt)) {
		return (uint) wkb_headroom(PNBUFF_2_WKBUFF(pkt));
#endif /* BCM_WKB */
	} else {
		return (uint) fkb_headroom(PNBUFF_2_FKBUFF(pkt));
	}
}

bool
nbuff_pktexpheadroom(osl_t *osh, void *pkt, int headroom)
{
#if defined(BCM_WKB)
	if (IS_WKBUFF_PTR(pkt)) {
		return wkb_realloc_headroom(PNBUFF_2_WKBUFF(pkt), headroom);
	} else
#endif /* BCM_WKB */
	{
		printk("%s: no such operation for skb|fkb!\n", __FUNCTION__);
		ASSERT(0);
		return FALSE;
	}
}

uint
nbuff_pkttailroom(osl_t *osh, void *pkt)
{
	if (IS_SKBUFF_PTR(pkt)) {
		return (uint) skb_tailroom(PNBUFF_2_SKBUFF(pkt));
#if defined(BCM_WKB)
	} else if (IS_WKBUFF_PTR(pkt)) {
		return wkb_tailroom(PNBUFF_2_WKBUFF(pkt));
#endif /* BCM_WKB */
	} else {
		printk("%s: no such operation for fkb!\n", __FUNCTION__);
		return 0;
	}
}

uchar *
nbuff_pktpush(osl_t *osh, void *pkt, int bytes)
{
#if defined(BCM_WKB)
	if (IS_WKBUFF_PTR(pkt)) {
		return wkb_push(PNBUFF_2_WKBUFF(pkt), bytes);
	} else
#endif /* BCM_WKB */
	{
		return nbuff_push(pkt, bytes);
	}
}

uchar *
nbuff_pktpull(osl_t *osh, void *pkt, int bytes)
{
#if defined(BCM_WKB)
	if (IS_WKBUFF_PTR(pkt)) {
		return wkb_pull(PNBUFF_2_WKBUFF(pkt), bytes);
	} else
#endif /* BCM_WKB */
	{
		return nbuff_pull(pkt, bytes);
	}
}

bool
nbuff_pktshared(void *pkt)
{
	if (IS_SKBUFF_PTR(pkt)) {
		return PNBUFF_2_SKBUFF(pkt)->cloned;
#if defined(BCM_WKB)
	} else if (IS_WKBUFF_PTR(pkt)) {
		printk("%s: no such operation for wkb!\n", __FUNCTION__);
		ASSERT(0);
		return FALSE;
#endif /* BCM_WKB */
	} else {
		printk("%s: no such operation for fkb!\n", __FUNCTION__);
		return FALSE;
	}
}

void *
nbuff_pktlink(void *pkt)
{
	if (IS_SKBUFF_PTR(pkt)) {
		return PNBUFF_2_SKBUFF(pkt)->prev;
#if defined(BCM_WKB)
	} else if (IS_WKBUFF_PTR(pkt)) {
		return wkb_link(PNBUFF_2_WKBUFF(pkt));
#endif /* BCM_WKB */
	} else {
		return nbuff_get_queue(pkt);
	}
}

void
nbuff_pktsetlink(void *pkt, void *link)
{
	if (IS_SKBUFF_PTR(pkt)) {
		PNBUFF_2_SKBUFF(pkt)->prev = PNBUFF_2_SKBUFF(link);
#if defined(BCM_WKB)
	} else if (IS_WKBUFF_PTR(pkt)) {
		wkb_set_link(PNBUFF_2_WKBUFF(pkt), link);
#endif /* BCM_WKB */
	} else {
		nbuff_set_queue(pkt, link);
	}
}

void
nbuff_pktsetnext(osl_t *osh, void *pkt, void *next)
{
	if (IS_SKBUFF_PTR(pkt)) {
		PNBUFF_2_SKBUFF(pkt)->next = PNBUFF_2_SKBUFF(next);
#if defined(BCM_WKB)
	} else if (IS_WKBUFF_PTR(pkt)) {
		wkb_set_next(PNBUFF_2_WKBUFF(pkt), next);
#endif /* BCM_WKB */
	} else {
		printk("%s: no such operation for fkb!\n", __FUNCTION__);
		ASSERT(0);
	}
}

void *
nbuff_pktnext(osl_t *osh, void *pkt)
{
	if (IS_SKBUFF_PTR(pkt)) {
		return PNBUFF_2_SKBUFF(pkt)->next;
#if defined(BCM_WKB)
	} else if (IS_WKBUFF_PTR(pkt)) {
		return wkb_next(PNBUFF_2_WKBUFF(pkt));
#endif /* BCM_WKB */
	} else {
		printk("%s: no such operation for fkb!\n", __FUNCTION__);
		return NULL;
	}
}

void *
nbuff_pktdup(osl_t *osh, void *pkt)
{
	void *pkt_dup;

#if defined(BCM_WKB)
	if (IS_WKBUFF_PTR(pkt)) {
#ifdef NOT_YET
		/* XXX: BCM_WKB_TEST: Review and fix it */
		pkt = wkb_pkt_ref(PNBUFF_2_WKBUFF(pkt));
		/* See if there is any back pointer */
		if (pkt == NULL) {
			printk("%s: no such operation for wkb!\n", __FUNCTION__);
			ASSERT(0);
			return NULL;
		}
#else /* ! NOT_YET */
		/* XXX: TODO: pkt cloning is not required and not supported with WKBs */
		ASSERT(0);
#endif /* ! NOT_YET */
	}
#endif /* BCM_WKB */

	ASSERT(IS_SKBUFF_PTR(pkt));

	SKB_BPM_TAINTED(PNBUFF_2_SKBUFF(pkt));		// clear SKB_BPM_PRISTINE, dirty
	pkt_dup = osl_pktdup(osh, pkt);

	return pkt_dup;
}

void *
nbuff_pktdup_cpy(osl_t *osh, void *pkt)
{
	void *pkt_dup;

	if (IS_SKBUFF_PTR(pkt)) {
		SKB_BPM_TAINTED(PNBUFF_2_SKBUFF(pkt));		// clear SKB_BPM_PRISTINE, dirty
		pkt_dup = osl_pktdup_cpy(osh, pkt);
#if defined(BCM_WKB)
	} else if (IS_WKBUFF_PTR(pkt)) {
		/* WKB should not have pNBuff linked */
		ASSERT(wkb_pkt_ref(PNBUFF_2_WKBUFF(pkt)) == NULL);
		pkt_dup = wkb_copy(PNBUFF_2_WKBUFF(pkt));
		PKTACCOUNT(osh, 1, TRUE);

#endif /* BCM_WKB */
	} else {
		printk("%s: no such operation for fkb!\n", __FUNCTION__);
		ASSERT(0);
		pkt_dup = NULL;
	}

	return pkt_dup;
}

void
nbuff_pktfree(osl_t *osh, void *pkt, bool docb, bool send)
{
	linux_pktfree(osh, pkt, docb, send);
	return;
}

void *
nbuff_pkt_tonative(osl_t *osh, void *pkt)
{
	struct sk_buff *skb;
#if defined(BCM_WKB)
	wkbuff_t *wkb = NULL;

	if (IS_WKBUFF_PTR(pkt)) {
		wkb = PNBUFF_2_WKBUFF(pkt);
		/* clear PKTTAG */
		wkb_pkt_tag_zero(wkb);

		pkt = wkb_pkt_ref(wkb);
		/* See if there is any back pointer */
		if (pkt == NULL) {
			PKTACCOUNT(osh, 1, FALSE);
			return WKBUFF_2_PNBUFF(wkb);
		}
	}
#endif /* BCM_WKB */

	if (IS_SKBUFF_PTR(pkt)) {
		skb = osl_pkt_tonative(osh, pkt);
	} else {
		skb = pkt;
	}

#if defined(BCM_WKB)
	if (wkb != NULL) {
		return (void *)WKBUFF_2_PNBUFF(wkb);
	}
#endif /* BCM_WKB */

	return (void *)skb;
}

void *
nbuff_pkt_frmnative(osl_t *osh, void *pkt)
{
#if defined(BCM_WKB)
	wkbuff_t *wkb = NULL;

	if (IS_WKBUFF_PTR(pkt)) {
		wkb = PNBUFF_2_WKBUFF(pkt);
		/* clear PKTTAG */
		wkb_pkt_tag_zero(wkb);

		pkt = wkb_pkt_ref(wkb);
		/* See if there is any back pointer */
		if (pkt == NULL) {
			PKTACCOUNT(osh, 1, TRUE);
			return NULL;
		}
	}
#endif /* BCM_WKB */

	if (IS_SKBUFF_PTR(pkt)) {
		pkt = osl_pkt_frmnative(osh, pkt);
	}

#if defined(BCM_WKB)
	if (wkb != NULL) {
		return WKBUFF_2_PNBUFF(wkb);
	}
#endif /* BCM_WKB */

	return pkt;
}

void inline *
nbuff_pkt_get_tag(void *pkt)
{
	if (IS_SKBUFF_PTR(pkt)) {
		return (void *)PNBUFF_2_SKBUFF(pkt)->cb;
#if defined(BCM_WKB)
	} else if (IS_WKBUFF_PTR(pkt)) {
		return (void *)wkb_pkt_tag(PNBUFF_2_WKBUFF(pkt));
#endif /* BCM_WKB */
	} else {
		FkBuff_t *fkb_p = PNBUFF_2_FKBUFF(pkt);
		return (void *)(BCM_PFKBUFF_TO_PHEAD(fkb_p));
	}
}

void inline
nbuff_pkt_clear_tag(void *pkt)
{
	void *tag;

#if defined(BCM_WKB)
	if (IS_WKBUFF_PTR(pkt)) {
		wkb_pkt_tag_zero(PNBUFF_2_WKBUFF(pkt));
		return;
	}
#endif /* BCM_WKB */

	tag = nbuff_pkt_get_tag(pkt);

	*(uint32 *) (tag) = 0;
	*(uint32 *) (tag + 4) = 0;
	*(uint32 *) (tag + 8) = 0;
	*(uint32 *) (tag + 12) = 0;
	*(uint32 *) (tag + 16) = 0;
	*(uint32 *) (tag + 20) = 0;
	*(uint32 *) (tag + 24) = 0;
	*(uint32 *) (tag + 28) = 0;
}

struct blog_t *
nbuff_pktblog(void *pkt)
{
#if defined(BCM_WKB)
	if (IS_WKBUFF_PTR(pkt)) {
		pkt = wkb_pkt_ref(PNBUFF_2_WKBUFF(pkt));
		/* See if there is any back pointer */
		if (pkt == NULL) {
			printk("%s: no such operation for wkb!\n", __FUNCTION__);
			ASSERT(0);
			return NULL;
		}
	}
#endif /* BCM_WKB */

	if (IS_SKBUFF_PTR(pkt)) {
		return PNBUFF_2_SKBUFF(pkt)->blog_p;
	} else {
		FkBuff_t *fkb_p = PNBUFF_2_FKBUFF(pkt);
		return fkb_p->blog_p;
	}
}

struct net_device *
nbuff_pktdev(void *pkt)
{
#if defined(BCM_WKB)
	if (IS_WKBUFF_PTR(pkt)) {
		pkt = wkb_pkt_ref(PNBUFF_2_WKBUFF(pkt));
		/* See if there is any back pointer */
		if (pkt == NULL) {
			printk("%s: no such operation for wkb!\n", __FUNCTION__);
			ASSERT(0);
			return NULL;
		}
	}
#endif /* BCM_WKB */

	if (IS_SKBUFF_PTR(pkt)) {
		return PNBUFF_2_SKBUFF(pkt)->dev;
	} else {
		printk("%s: no such operation for fkb!\n", __FUNCTION__);
		ASSERT(0);
		return NULL;
	}
}

void
nbuff_pktsetdev(void *pkt, struct net_device *dev)
{
#if defined(BCM_WKB)
	if (IS_WKBUFF_PTR(pkt)) {
		pkt = wkb_pkt_ref(PNBUFF_2_WKBUFF(pkt));
		/* See if there is any back pointer */
		if (pkt == NULL) {
			printk("%s: no such operation for wkb!\n", __FUNCTION__);
			ASSERT(0);
			return;
		}
	}
#endif /* BCM_WKB */

	if (IS_SKBUFF_PTR(pkt)) {
		PNBUFF_2_SKBUFF(pkt)->dev = dev;
	} else {
		printk("%s: no such operation for fkb!\n", __FUNCTION__);
		ASSERT(0);
	}
}

/* Handle skbuff::wlan_ext::wl_flag1 */
bool
nbuff_skb_wlan_ext_is_wl_flag(void *pkt, int flag)
{
#if defined(BCM_WKB)
	if (IS_WKBUFF_PTR(pkt)) {
		pkt = wkb_pkt_ref(PNBUFF_2_WKBUFF(pkt));
		/* See if there is any back pointer */
		if (pkt == NULL) {
			return FALSE;
		}
	}
#endif /* BCM_WKB */

	if (IS_SKBUFF_PTR(pkt) &&
		skbuff_bcm_ext_wlan_get(PNBUFF_2_SKBUFF(pkt), wl_flag1) & flag) {
		return TRUE;
	}

	return FALSE;
}

void
nbuff_skb_wlan_ext_set_wl_flag(void *pkt, int flag)
{
#if defined(BCM_WKB)
	if (IS_WKBUFF_PTR(pkt)) {
		pkt = wkb_pkt_ref(PNBUFF_2_WKBUFF(pkt));
		/* See if there is any back pointer */
		if (pkt == NULL) {
			printk("%s: no such operation for wkb!\n", __FUNCTION__);
			ASSERT(0);
			return;
		}
	}
#endif /* BCM_WKB */

	if (IS_SKBUFF_PTR(pkt)) {
		skbuff_bcm_ext_wlan_get(PNBUFF_2_SKBUFF(pkt), wl_flag1) |= flag;
	}
}

void
nbuff_skb_wlan_ext_clr_wl_flag(void *pkt, int flag)
{
#if defined(BCM_WKB)
	if (IS_WKBUFF_PTR(pkt)) {
		pkt = wkb_pkt_ref(PNBUFF_2_WKBUFF(pkt));
		/* See if there is any back pointer */
		if (pkt == NULL) {
			printk("%s: no such operation for wkb!\n", __FUNCTION__);
			ASSERT(0);
			return;
		}
	}
#endif /* BCM_WKB */

	if (IS_SKBUFF_PTR(pkt)) {
		skbuff_bcm_ext_wlan_get(PNBUFF_2_SKBUFF(pkt), wl_flag1) &= ~flag;
	}
}

#if defined(BCM_WLAN_MCAST_BLOG_SHARED_INFO) && defined(BCM_BLOG)
void
wl_mcast_blog_client_handler(void *sdu)
{
#if defined(BCM_WKB)
	if (IS_WKBUFF_PTR(sdu)) {
		sdu = wkb_pkt_ref(PNBUFF_2_WKBUFF(sdu));
		/* See if there is any back pointer */
		if (sdu == NULL) {
			printk("%s: no such operation for wkb!\n", __FUNCTION__);
			ASSERT(0);
			return;
		}
	}
#endif /* BCM_WKB */

	if (IS_SKBUFF_PTR(sdu) && PKTISBLOG_CLONED(sdu)) {
		blog_inc_wlan_mcast_client_count(sdu);
		PKTCLRBLOG_CLONED(sdu);
	}
}

void
wl_mcast_blog_clone_handler(void *sdu, void *sdu2)
{
#if defined(BCM_WKB)
	if (IS_WKBUFF_PTR(sdu)) {
		ASSERT(IS_WKBUFF_PTR(sdu2));

		sdu = wkb_pkt_ref(PNBUFF_2_WKBUFF(sdu));
		sdu2 = wkb_pkt_ref(PNBUFF_2_WKBUFF(sdu2));
		/* See if there is any back pointer */
		if ((sdu == NULL) || (sdu2 == NULL)) {
			printk("%s: no such operation for wkb! sdu %p; sdu2 %p\n",
			__FUNCTION__, sdu, sdu2);
			ASSERT(0);
			return;
		}
	}
#endif /* BCM_WKB */

	if (IS_SKBUFF_PTR(sdu) &&
		!blog_clone_wlan(PNBUFF_2_SKBUFF(sdu), PNBUFF_2_SKBUFF(sdu2))) {
		PKTSETBLOG_CLONED(sdu2);
	}
}
#endif /* BCM_WLAN_MCAST_BLOG_SHARED_INFO && BCM_BLOG */

/**
 * ------------------------------------------------------------------------------------------------
 *  SECTION: Packet alloc/free and Packet Pool
 * ------------------------------------------------------------------------------------------------
 */

/* Allocate a packet based on type (SKB|FKB|WKB) */
void *
nbuff_pktget_by_type(osl_t *osh, int len, bool send, int NBuffPtrType)
{
	void *pkt = NULL;

	switch (NBuffPtrType) {

#if defined(BCM_WKB)
	case WKBUFF_PTR:
	{
		wkbuff_t *wkb;

		wkb = wkb_alloc(len, WKB_SE_WL);
		if (wkb != NULL) {
			PKTACCOUNT(osh, 1, TRUE);
			pkt = WKBUFF_2_PNBUFF(wkb);
		}
		break;
	}
#endif /* BCM_WKB */

	case SKBUFF_PTR:
		pkt = linux_pktget(osh, len);
		break;
	default:
		printk("%s: Unsupported Packet Type %d\n", __FUNCTION__, NBuffPtrType);
		ASSERT(0);
		break;
	}

	return pkt;
}

#if defined(BCM_NBUFF_PKTPOOL_CACHE)
/* Allocate an SKB from WLAN local pool (cache) allocated from BPM */
void *
nbuff_pktpoolget(osl_t *osh, int len)
{
	struct sk_buff *skb;
	osl_pubinfo_t *osh_pub = (osl_pubinfo_t *)osh;

	if (osh_pub->skb_cache == NULL) {
		/* Refill the cache from BPM pool */
		if ((osh_pub->skb_cache = gbpm_alloc_mult_buf_skb_attach(
			BCM_NBUFF_PKTPOOL_CACHE_MAXPKTS, len, GBPM_LOW_PRIO_ALLOC)) == NULL) {
			return NULL;
		}

		PKTACCOUNT(osh, BCM_NBUFF_PKTPOOL_CACHE_MAXPKTS, TRUE);
	}

	/* Allocate a packet from cache */
	skb = osh_pub->skb_cache;
	osh_pub->skb_cache = skb->next;
	skb->next = NULL;
#if !defined(CONFIG_BCM_GLB_COHERENCY)
	/* XXX: TODO: Set PKTDIRTYP and perform NBUFF cache flush/invalidate
	 * (CC_NBUFF_FLUSH_OPTIMIZATION) in SW coherent platforms.
	 */
	SKB_BPM_TAINTED(skb);	// clear SKB_BPM_PRISTINE, dirty
#endif /* ! CONFIG_BCM_GLB_COHERENCY */

#ifdef BCMDBG_PKT
	osl_pktlist_add(osh, skb, __LINE__, __FILE__);
#endif /* BCMDBG_PKT */

	return skb;
}
#else /* ! BCM_NBUFF_PKTPOOL_CACHE */
void *
nbuff_pktpoolget(osl_t *osh, int len)
{
	struct sk_buff *skb;

	if ((skb = gbpm_alloc_buf_skb_attach(len)) == NULL) {
		return NULL;
	}

	PKTACCOUNT(osh, 1, TRUE);
#if !defined(CONFIG_BCM_GLB_COHERENCY)
	/* XXX: TODO: Set PKTDIRTYP and perform NBUFF cache flush/invalidate
	 * (CC_NBUFF_FLUSH_OPTIMIZATION) in SW coherent platforms.
	 */
	SKB_BPM_TAINTED(skb);	// clear SKB_BPM_PRISTINE, dirty
#endif /* ! CONFIG_BCM_GLB_COHERENCY */

#ifdef BCMDBG_PKT
	osl_pktlist_add(osh, skb, __LINE__, __FILE__);
#endif /* BCMDBG_PKT */

	return skb;
}
#endif /* ! BCM_NBUFF_PKTPOOL_CACHE */

/**
 * ------------------------------------------------------------------------------------------------
 *  SECTION: XPM - BPM/MPM based Packet Pool
 * ------------------------------------------------------------------------------------------------
 */
#ifdef PP_XPM
/* PP_XPM allocates SKBs and Data buffers from pool like MPM/BPM */

/* XXX: Current experiments shows that MPM buffers/SKBs have negative
 * impact on throughput. So using BPM buffers/SKBs but retaining the
 * SVN side cache optimization - deferring SKB linking to data bufs
 * during rx processing (originally SKB linking was done during rxfill)
 *
 * TODO: Replace BPM buffers with MPM
 */

#ifdef BCM_NBUFF_PKTPOOL_CACHE_MAXPKTS
#define PP_XPM_CACHE_MAXPKTS	    BCM_NBUFF_PKTPOOL_CACHE_MAXPKTS
#else
#define PP_XPM_CACHE_MAXPKTS	    64
#endif
/* Should be more or equal to PKTCBND/RXBND */
#define PP_XPM_MAX_XKB_CACHE	    128

typedef struct nbuff_pp_xpm {
	struct sk_buff	* skb_cache;
	/* Local cache of databufs allocated from BPM */
	void		* datapool[PP_XPM_CACHE_MAXPKTS];
	uint8		datapool_curidx;
#if defined(BCM_SPC_RX) || defined(BCM_WKB)
	xk_buff_t	xkb_cache[PP_XPM_MAX_XKB_CACHE];
	uint8		xkb_curidx;
#endif /* BCM_SPC_RX || BCM_WKB */
} nbuff_pp_xpm_t;

/* Initialize local XPM SKB/databuf pool */
int
nbuff_pp_xpm_init(osl_t *osh, bool *enab)
{
	osl_pubinfo_t	*osh_pub = (osl_pubinfo_t *)osh;
	nbuff_pp_xpm_t	*pp_xpm;
	char *var = NULL;
	int ret = BCME_OK;
	bool pp_xpm_enab = TRUE;

	/* NVRAM to enable/disable pp_xpm feature. Default enabled.
	 * Changing this configuration requires reboot
	 */
	var = getvar(NULL, "wl_pp_xpm");
	if (var != NULL) {
		pp_xpm_enab = bcm_strtoul(var, NULL, 0);
	}

	if (pp_xpm_enab == FALSE) {
		*enab = FALSE;
		return BCME_OK;
	}

	osh_pub->pp_xpm = MALLOCZ(osh, sizeof(nbuff_pp_xpm_t));
	if (osh_pub->pp_xpm == NULL) {
		ASSERT(osh_pub->pp_xpm == NULL);
		ret = BCME_NOMEM;
		goto done;
	}

	pp_xpm = (nbuff_pp_xpm_t *)osh_pub->pp_xpm;

	/* set to max so that skb/data is fetched from bpm */
	pp_xpm->datapool_curidx = PP_XPM_CACHE_MAXPKTS;

done:
	if (ret) {
		nbuff_pp_xpm_deinit(osh);
		*enab = FALSE;
	} else {
		*enab = TRUE;
	}

	printk("%s: USING BPM pktpool initialization %s\n",
		__FUNCTION__, (ret) ? "FAILED" : "SUCCESS");

	return ret;
} /* nbuff_pp_xpm_init */

/* Deinitialize local XPM skb/databuf pool */
int
nbuff_pp_xpm_deinit(osl_t *osh)
{
	osl_pubinfo_t	* osh_pub = (osl_pubinfo_t *)osh;
	nbuff_pp_xpm_t * pp_xpm = (nbuff_pp_xpm_t *)osh_pub->pp_xpm;
	struct sk_buff *skb;
	uint32 npkts;

	if (pp_xpm == NULL) {
		return BCME_OK;
	}

	/* Freeup data buffers from local cache */
	npkts = PP_XPM_CACHE_MAXPKTS - pp_xpm->datapool_curidx;
	if (npkts) {
		gbpm_free_mult_buf(npkts, &pp_xpm->datapool[pp_xpm->datapool_curidx]);
	}

	npkts = 0;
	/* Freeup SKBs from local cache */
	while (pp_xpm->skb_cache) {
		skb = pp_xpm->skb_cache;
		pp_xpm->skb_cache = skb->next;
		skb->next = NULL;
		npkts++;
		gbpm_free_skb(skb);
	}

	PKTACCOUNT(osh, npkts, FALSE);

	MFREE(osh, osh_pub->pp_xpm, sizeof(nbuff_pp_xpm_t));
	osh_pub->pp_xpm = NULL;

	printk("%s: deinit SUCCESS\n", __FUNCTION__);

	return BCME_OK;
} /* nbuff_pp_xpm_deinit */

/* Returns an SKB from WLAN local pool allocated from BPM/MPM */
struct sk_buff *
nbuff_pp_xpm_skb_get(osl_t *osh)
{
	osl_pubinfo_t *osh_pub = (osl_pubinfo_t *)osh;
	nbuff_pp_xpm_t *pp_xpm = (nbuff_pp_xpm_t *)osh_pub->pp_xpm;
	struct sk_buff *skb = NULL;

	ASSERT(pp_xpm != NULL);

	/* NOTE: Caching PP_XPM_CACHE_MAXPKTS skbs/databufs locally.
	 * This should be fine if MPM HW is used, but will have
	 * a toll on cache if we use BPM or other sw pool manager.
	 * At a given point the no of buffs used is based on the no of pkts
	 * available in dma ring. So fetching PP_XPM_CACHE_MAXPKTS
	 * no of bufs will evict other addresses from cache line causing
	 * additional overhead.
	 *
	 * TODO: May be we should check dma ring and fetch only those many
	 * buffers that are available in dma ring.
	 */

	if (pp_xpm->skb_cache == NULL) {
		pp_xpm->skb_cache = gbpm_alloc_mult_skb(PP_XPM_CACHE_MAXPKTS);
		if (pp_xpm->skb_cache == NULL) {
			goto done;
		}
		PKTACCOUNT(osh, BCM_NBUFF_PKTPOOL_CACHE_MAXPKTS, TRUE);
	}

	/* Allocate a packet from cache */
	skb = pp_xpm->skb_cache;
	pp_xpm->skb_cache = skb->next;
	skb->next = NULL;

	ASSERT(skb != NULL);

done:
	return skb;
} /* nbuff_pp_xpm_skb_get */

/* Freeup an SKB (No buffer attached) to XPM pool */
void
nbuff_pp_xpm_skb_free(osl_t *osh, struct sk_buff *skb)
{
	ASSERT(((osl_pubinfo_t *)osh)->pp_xpm != NULL);
	gbpm_free_skb(skb);
	PKTACCOUNT(osh, 1, FALSE);
}

/* return a databuf from WLAN local pool allocated by B/MPM */
void *
nbuff_pp_xpm_databuf_get(osl_t *osh, int len)
{
	osl_pubinfo_t *osh_pub = (osl_pubinfo_t *)osh;
	nbuff_pp_xpm_t *pp_xpm = (nbuff_pp_xpm_t *)osh_pub->pp_xpm;
	void *pdata = NULL;
	int ret = 0;

	ASSERT(pp_xpm != NULL);

	if (pp_xpm->datapool_curidx == PP_XPM_CACHE_MAXPKTS) {
		if (gbpm_alloc_mult_buf(PP_XPM_CACHE_MAXPKTS,
			(void **)&pp_xpm->datapool[0]) == GBPM_ERROR) {
			ret = BCME_ERROR;
		}

		if (ret < 0) {
			goto done;
		}

		pp_xpm->datapool_curidx = 0;
	}

	pdata = pp_xpm->datapool[pp_xpm->datapool_curidx];
	ASSERT(pdata != NULL);
	pp_xpm->datapool_curidx++;

done:
	return pdata;
} /* nbuff_pp_xpm_databuf_get */

/* Freeup databuffer to XPM pool */
void
nbuff_pp_xpm_databuf_free(osl_t *osh, void *pdata)
{
	ASSERT(((osl_pubinfo_t *)osh)->pp_xpm != NULL);
	gbpm_free_buf(pdata);
}

/* Freeup databuffer attached to an SKB to XPM pool */
void
nbuff_pp_xpm_spc_databuf_free(osl_t *osh, void *pkt, void *pdata)
{
	ASSERT(((osl_pubinfo_t *)osh)->pp_xpm != NULL);
	gbpm_free_buf_from_spc_skb(pkt, BCM_PDATA_TO_PHEAD(pdata));
}

/* Attach SKB to Data buffer */
/* NOTE:
 * When CONFIG_BCM_MPM_BIND_BPM defined, all buffer will be coming
 * from MPM and those are managed by BPM. BPM uses MPM_BUF_MODE_PDATA
 * for mpm alloc ring which does not reset shared info. So calling
 * gbpm_attach_skb() which explicitely resets shared_info region.
 *
 * For DL direction, Crossbow uses mpm ring of MPM_BUF_MODE_PDATA_SHINFO
 * types, so shared_info reset will be done twice. Let's take this hit
 * since this function will be called only detransform.
 *
 * data_offset: Offset of data pointer from pData (Might be -ve)
 */
int
nbuff_pp_xpm_attach_skb(osl_t *osh, struct sk_buff *skb, void *pdata,
	int32 data_offset, uint32 len)
{
#ifndef CONFIG_BCM_MPM_BIND_BPM
	void *pStart = BCM_PDATA_TO_PSTART(pdata);
	uint16 pStart_to_data_offset = (uint16)(uintptr)BCM_PSTART_TO_PDATA(NULL) + data_offset;

	/* In DL directrion we will be getting MPM buffrs here but for UL,
	 * we will be getting sw buffers from BPM. So for DL, we do not need
	 * shared info reset as crossbow uses MPM_BUF_MODE_PDATA_SHINFO rings.
	 */
	if (gbpm_is_buf_hw_recycle_capable(pStart)) {
		gbpm_attach_skb_hw_buf(skb, pStart, len, pStart_to_data_offset, true);
	} else
#endif /* !CONFIG_BCM_MPM_BIND_BPM */
	{
		gbpm_attach_skb(skb, pdata, (data_offset + len));

		if (data_offset) {
			if (data_offset > 0) {
				/* Pull out the data_offset */
				skb_pull(skb, data_offset);
			} else {
				skb_push(skb, -data_offset);
			}
		}
	}

	skb_cb_zero(skb);

	return BCME_OK;
} /* nbuff_pp_xpm_attach_skb */


/**
 * ------------------------------------------------------------------------------------------------
 *  SECTION: SPC - Single Packet Context Helper ADTs
 * ------------------------------------------------------------------------------------------------
 */
#if defined(BCM_SPC_RX) || defined(BCM_WKB)
/* Intialize XKB for Data buffer
 *
 * data_offset: Offset of data pointer from pData (Might be -ve)
 */
void *
nbuff_pp_xpm_attach_xkb(osl_t *osh, void *pdata, int data_offset, uint32 len)
{
	osl_pubinfo_t *osh_pub = (osl_pubinfo_t *)osh;
	nbuff_pp_xpm_t *pp_xpm = (nbuff_pp_xpm_t *)osh_pub->pp_xpm;
	xk_buff_t *xkb = &pp_xpm->xkb_cache[pp_xpm->xkb_curidx++];

	ASSERT(pp_xpm->xkb_curidx <= PP_XPM_MAX_XKB_CACHE);

	xkb->data = pdata;
	xkb->offset = BCM_PKT_HEADROOM;
	xkb->len = len;
	xkb->link = NULL;

	if (data_offset >= 0) {
		/* Pull out the data_offset */
		xkb_pull(xkb, data_offset);
	} else {
		xkb_push(xkb, -data_offset);
	}

	return XKBUFF_2_PNBUFF(xkb);
} /* nbuff_pp_xpm_attach_xkb */

void
nbuff_pp_xpm_release_xkb_cache(osl_t *osh)
{
	osl_pubinfo_t *osh_pub = (osl_pubinfo_t *)osh;
	nbuff_pp_xpm_t *pp_xpm = (nbuff_pp_xpm_t *)osh_pub->pp_xpm;

	pp_xpm->xkb_curidx = 0;
} /* nbuff_pp_xpm_release_xkb_cache */

/* Return skb for a given xk_buff_t */
void *
nbuff_pp_xpm_xkb_xlate(osl_t *osh, void *p)
{
	void	*pkt;
	xk_buff_t *xkb		= PNBUFF_2_XKBUFF(p);
	uint8_t *pData		= BCM_PHEAD_TO_PDATA(xkb->data - xkb->offset);
	int offset_from_pData	= (int)((uintptr_t)(xkb->data) - (uintptr_t)(pData));

#if defined(BCM_SPC_RX)
	{
		struct sk_buff *skb	= NULL;
		skb = nbuff_pp_xpm_skb_get(osh);
		if (skb == NULL) {
			return NULL;
		}

		nbuff_pp_xpm_attach_skb(osh, skb, pData, offset_from_pData, xkb->len);
		pkt = SKBUFF_2_PNBUFF(skb);
	}
#else /* BCM_WKB */
	pkt = nbuff_attach_wkb(osh, pData, offset_from_pData,
		(offset_from_pData + xkb->len), FALSE);
#endif /* BCM_WKB */

	ASSERT(PKTLEN(osh, pkt) == xkb->len);
	ASSERT(PKTDATA(osh, pkt) == xkb->data);

	PKTSETPRIO(pkt, 0);
	PKTSETNEXT(osh, pkt, NULL);

	return pkt;
} /* nbuff_pp_xpm_xkb_xlate */

#endif /* BCM_SPC_RX || BCM_WKB */

#if defined(BCM_WKB)
/**
 * ------------------------------------------------------------------------------------------------
 *  SECTION: WKB - Wireless Kernel Buffer
 * ------------------------------------------------------------------------------------------------
 */

/**
 * Prepares a WKB packet context for the data buffer.
 *
 * is_phead indicates the region to which data points to
 *
 * - With HW HDR CONV
 *   wkb::data		: Headoom region is used RxFIFO-1 (802.11)
 *   wkb::frag_data[0]	: Payload region is used RxFIFO-0 (802.3)
 * - Without HW HDR CONV
 *   wkb::data		: Payload region is used for unconverted frame RxFIFO-0
 *
 * data_offset: Offset of packt content payload (802.11) from passed data pointer (pData/pHead).
 *		Might be -ve
 */
void *
nbuff_attach_wkb(osl_t * osh, void * data, int data_offset, uint16 len, bool is_phead)
{
	wkbuff_t * wkb = NULL;
	void	* pStart;
	wkb_db_pool_t db_pool;

	if (is_phead) {
		pStart = BCM_PHEAD_TO_PSTART(data);
	} else {
		pStart = BCM_PDATA_TO_PSTART(data);
	}

	if (gbpm_is_buf_hw_recycle_capable(pStart))
		db_pool = WKB_DB_MPM;
	else
		db_pool = WKB_DB_BPM;

	wkb = wkb_init(pStart, data, len, 0, db_pool, WKB_SE_WL, TRUE);
	if (data_offset >= 0) {
		/* Pull out the data_offset */
		wkb_pull(wkb, data_offset);
	} else {
		wkb_push(wkb, -data_offset);
	}

	PKTACCOUNT(osh, 1, TRUE);

	return WKBUFF_2_PNBUFF(wkb);
} /* nbuff_attach_wkb() */

/* Fills the 802.3 databuffer at a given frag index
 * data_offset: Offset of data pointer from pData (Might be -ve)
 */
void
nbuff_wkb_frag_idx_fill_item(osl_t * osh, void * pkt, void * pData, uint16 frag_idx,
	int data_offset, uint16 frag_len)
{
	wkbuff_t * wkb = NULL;
	void	* pStart;
	uint16	db_pool;

	wkb = PNBUFF_2_WKBUFF(pkt);

	pStart = BCM_PDATA_TO_PSTART(pData);
	if (gbpm_is_buf_hw_recycle_capable(pStart))
		db_pool = WKB_DB_MPM;
	else
		db_pool = WKB_DB_BPM;

	frag_idx = WLIDX2WKBIDX(frag_idx);

	wkb_frag_idx_fill_item(wkb, pStart, pData, frag_idx, frag_len, db_pool);
	if (data_offset >= 0) {
		/* Pull out the data_offset */
		wkb_frag_pull(wkb, frag_idx, data_offset);
	} else {
		wkb_frag_push(wkb, frag_idx, -data_offset);
	}

} /* nbuff_wkb_frag_idx_fill_item() */

/* Add the MSDU fragment to head WKB */
int
nbuff_wkb_frag_fill_item(osl_t * osh, void * pkt, void * frag)
{
	void	* pStart;
	void	* msdu_data;
	uint16	msdu_len;
	uint16	db_pool;

	if (PKTSPCRXDMAFMT(frag)) {
		xk_buff_t *xkb = PNBUFF_2_XKBUFF(frag);

		pStart = BCM_PHEAD_TO_PSTART(xkb->data - xkb->offset);
		msdu_data = xkb->data;
		msdu_len = xkb->len;
		if (gbpm_is_buf_hw_recycle_capable(pStart))
			db_pool = WKB_DB_MPM;
		else
			db_pool = WKB_DB_BPM;
	} else {
		if (PKTISHDRCONVTD(osh, frag)) {
			msdu_data = wkb_frag_data(PNBUFF_2_WKBUFF(frag), WLIDX2WKBIDX(LB_FRAG1));
			msdu_len = wkb_frag_len(PNBUFF_2_WKBUFF(frag), WLIDX2WKBIDX(LB_FRAG1));
		} else {
			msdu_data = wkb_data(PNBUFF_2_WKBUFF(frag));
			msdu_len = wkb_len(PNBUFF_2_WKBUFF(frag));
		}

		pStart = wkb_db_addr(PNBUFF_2_WKBUFF(frag));
		db_pool = PNBUFF_2_WKBUFF(frag)->frag_key[0].db_pool;

		/* Reset WKB region of frag item */
		memset(PNBUFF_2_WKBUFF(frag), 0, WKB_CTXT_BYTES);
	}

	return wkb_frag_fill_item(PNBUFF_2_WKBUFF(pkt), pStart,
		msdu_data, msdu_len, db_pool, FALSE);

} /* nbuff_wkb_frag_fill_item */

/* Fixup WKB data parameters wrt to pNBuff linked to it */
void
nbuff_wkb_data_fixup(osl_t * osh, void * pkt)
{
	wkbuff_t	* wkb;
	struct sk_buff	* skb;

	wkb = PNBUFF_2_WKBUFF(pkt);
	skb = PNBUFF_2_SKBUFF(wkb_pkt_ref(wkb));

	WKB_PTRACE("wkb " WKB_ADDR_FMT " skb " WKB_ADDR_FMT " data " WKB_ADDR_FMT " len %d\n",
		WKB_ADDR_VAL(wkb), WKB_ADDR_VAL(skb), WKB_ADDR_VAL(skb->data), skb->len);

	/* Fix up WKB params */
	wkb_set_data(wkb, skb->data);
	wkb_set_len(wkb, skb->len);
	wkb_set_prio(wkb, nbuff_pktprio(skb));

	/* And Reset SKB data params */
	skb->data = NULL;
	__pskb_trim(skb, 0);
} /* nbuff_wkb_data_fixup */

void *
nbuff_xlate_to_wkb(osl_t * osh, void * pkt)
{
	struct sk_buff	* skb;
	wkbuff_t	* wkb;
	void		* pStart;
	void		* pData;
	wkb_db_pool_t	db_pool;

	/* Supports only SKB */
	ASSERT(IS_SKBUFF_PTR(pkt));

	skb = PNBUFF_2_SKBUFF(pkt);
	ASSERT(!skb->is_spc);

	WKB_PTRACE("SKB " WKB_ADDR_FMT " data " WKB_ADDR_FMT " len %d prio %d\n",
		WKB_ADDR_VAL(skb), WKB_ADDR_VAL(skb->data), skb->len, nbuff_pktprio(skb));

	pData = gbpm_alloc_buf();
	if (pData == NULL) {
		PKTFREE_NOCB(osh, pkt, TRUE);
		return NULL;
	}

	pStart = BCM_PDATA_TO_PSTART(pData);
	if (gbpm_is_buf_hw_recycle_capable(pStart))
		db_pool = WKB_DB_MPM;
	else
		db_pool = WKB_DB_BPM;

	wkb = wkb_init(pStart, pData, BCM_MAX_PKT_LEN, nbuff_pktprio(skb),
		db_pool, WKB_SE_LX, TRUE);

	/* Fill SKB buffer to Linux frag_idx.
	 * And only WKB should carry data buffer so reset frag data, len and SKB data, len.
	 */
	wkb_frag_fill_item(wkb, skb->head, NULL, 0, WKB_DB_LNX, FALSE);

	/* WKB should point to SKB data buffer */
	wkb_set_data(wkb, skb->data);
	wkb_set_len(wkb, skb->len);

	/* Reset SKB data params */
	skb->data = NULL;
	__pskb_trim(skb, 0);

	/* Save SKB pointer in WKB */
	wkb_set_pkt_ref(wkb, skb);

	WKB_PTRACE("WKB " WKB_ADDR_FMT " data " WKB_ADDR_FMT " len %d prio %d\n"
		"\tFrag: num %d flags %x tot_len %d wlFlowInf %x\n",
		WKB_ADDR_VAL(wkb), WKB_ADDR_VAL(wkb_data(wkb)), wkb_len(wkb), wkb_prio(wkb),
		wkb->frag_num, wkb->frag_flags, wkb_totlen(wkb), wkb->wl.u32);

	return WKBUFF_2_PNBUFF(wkb);
} /* nbuff_xlate_to_wkb */

void *
nbuff_xlate_to_skb(osl_t * osh, void * pkt)
{
	struct sk_buff	* skb;
	wkbuff_t	* wkb;
	uint8		* pData;
	uint8		* data;
	int		data_offset;
	uint16		len;
	uint16		prio;

	ASSERT(IS_WKBUFF_PTR(pkt));

	wkb	= (wkbuff_t *)PNBUFF_2_WKBUFF(pkt);

	WKB_PTRACE("WKB " WKB_ADDR_FMT " data " WKB_ADDR_FMT " len %d prio %d\n"
		"\tFrag: num %d flags %x tot_len %d wlFlowInf %x\n",
		WKB_ADDR_VAL(wkb), WKB_ADDR_VAL(wkb_data(wkb)), wkb_len(wkb), wkb_prio(wkb),
		wkb->frag_num, wkb->frag_flags, wkb_totlen(wkb), wkb->wl.u32);

	data	= wkb_data(wkb);
	len	= wkb_len(wkb);
	prio	= wkb_prio(wkb);
	skb	= wkb_pkt_ref(wkb);

	/* Check if an SKB is already associated with WKB */
	if (skb != NULL) {
		skb->data = data;
		__pskb_trim(skb, len);

		/* Reset SKB reference and free up WKB */
		nbuff_wkb_release_pkt_ref(osh, WKBUFF_2_PNBUFF(wkb));
		nbuff_free(WKBUFF_2_PNBUFF(wkb));

		goto done;
	}

	skb = gbpm_alloc_skb();
	if (skb == NULL) {
		PKTFREE_NOCB(osh, wkb, TRUE);
		return NULL;
	}

	pData = BCM_PWKBUFF_TO_PDATA(wkb);
	data_offset = (int)(data - pData);

	/* Reset WKB region */
	memset(wkb, 0, WKB_CTXT_BYTES);

	PP_XPM_ATTACH_SKB(osh, skb, pData, data_offset, len);

done:
	nbuff_pktsetprio(skb, prio);

	WKB_PTRACE("SKB " WKB_ADDR_FMT " head " WKB_ADDR_FMT " data " WKB_ADDR_FMT " len %d\n"
		"SPC %d nr_frags %d spc_tot_len %d\n",
		WKB_ADDR_VAL(skb), WKB_ADDR_VAL(skb->head), WKB_ADDR_VAL(skb->data), skb->len,
		skb->is_spc, skb_shinfo(skb)->nr_frags, skb->spc_tot_len);

	WKB_ASSERT(skb->data == data);
	WKB_ASSERT(skb->len == len);

	return skb;
} /* nbuff_xlate_to_skb */

/* Get pNBuff (SKB) linked to WKB and Fixup SKB data parameters */
void *
nbuff_wkb_get_pkt_ref(osl_t * osh, void * pkt)
{
	wkbuff_t	* wkb;
	struct sk_buff	* skb;

	wkb = PNBUFF_2_WKBUFF(pkt);
	skb = PNBUFF_2_SKBUFF(wkb_pkt_ref(wkb));

	WKB_PTRACE("wkb " WKB_ADDR_FMT " skb " WKB_ADDR_FMT " data " WKB_ADDR_FMT " len %d\n",
		WKB_ADDR_VAL(wkb), WKB_ADDR_VAL(skb), WKB_ADDR_VAL(wkb_data(wkb)), wkb_len(wkb));

	/* Get SKB attached to the WKB */
	if (skb == NULL) {
		return NULL;
	}

	/* Fix up SKB data params */
	skb->data = wkb_data(wkb);
	__pskb_trim(skb, wkb_len(wkb));
	nbuff_pktsetprio(skb, wkb_prio(wkb));

	return skb;
} /* nbuff_wkb_get_pkt_ref */

/* Reset all references to the pNBuff in WKB */
void *
nbuff_wkb_release_pkt_ref(osl_t * osh, void * pkt)
{
	return wkb_release_pkt_ref(PNBUFF_2_WKBUFF(pkt));
}

/*
 * Description : Iterate through wkb frags and prepare sdu (wkb_next) list
 * The API will not touch frag "0"
 */
int
nbuff_wkb_frags_release(osl_t * osh, void * pkt)
{
	int pktcnt;
	pktcnt = wkb_frags_release(PNBUFF_2_WKBUFF(pkt));

	/* Head packet is already accounted */
	PKTACCOUNT(osh, pktcnt - 1, TRUE);

	return pktcnt;
}
#endif /* BCM_WKB */
#endif /* PP_XPM */
#endif /* BCM_NBUFF */
