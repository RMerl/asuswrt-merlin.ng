/*
    Copyright (c) 2017 Broadcom
    All Rights Reserved

    <:label-BRCM:2017:DUAL/GPL:standard
    
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License, version 2, as published by
    the Free Software Foundation (the "GPL").
    
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    
    
    A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
    writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
    
    :>
*/

#if defined(BCM_NBUFF)

#include <linux/kernel.h>
#include <linux/nbuff.h>
#include <osl_decl.h>
#include <bcmutils.h>

#include <nbuff_pkt.h>

/* GENERIC NBUFF */
uint nbuff_pktprio(void *pkt)
{
	uint32 prio = 0;
	if (IS_SKBUFF_PTR(pkt)) {
		prio = ((struct sk_buff *)pkt)->mark >> PRIO_LOC_NFMARK & 0x7;
	} else {		/* manuplated in dhd, 3bit prio + 10bit flowid */
		FkBuff_t *fkb_p = PNBUFF_2_FKBUFF(pkt);
		prio = fkb_p->wl.ucast.dhd.wl_prio;	/* Same bit pos used for mcast */
	}

	if (prio > 7) {
		prio = 0;
	}

	return prio;
}

void nbuff_pktsetprio(void *pkt, uint x)
{
	if (IS_SKBUFF_PTR(pkt)) {
		((struct sk_buff *)pkt)->mark &= ~(0x7 << PRIO_LOC_NFMARK);
		((struct sk_buff *)pkt)->mark |= (x & 0x7) << PRIO_LOC_NFMARK;
	} else {
		FkBuff_t *fkb_p = PNBUFF_2_FKBUFF(pkt);
		fkb_p->wl.ucast.dhd.wl_prio = x;	/* Same bit pos used for mcast */
	}
}

uchar *nbuff_pktdata(osl_t * osh, void *pkt)
{
	BCM_REFERENCE(osh);
	return nbuff_get_data((pNBuff_t) pkt);
}

uint nbuff_pktlen(osl_t * osh, void *pkt)
{
	BCM_REFERENCE(osh);
	return nbuff_get_len((pNBuff_t) pkt);
}

void nbuff_pktsetlen(osl_t * osh, void *pkt, uint len)
{
	BCM_REFERENCE(osh);
	if (IS_SKBUFF_PTR((pNBuff_t) pkt))
		__pskb_trim((struct sk_buff *)pkt, len);
	/* else if IS_FPBUFF_PTR, else if IS_TGBUFF_PTR */
	else
		nbuff_set_len((pNBuff_t) pkt, len);
}

uint nbuff_pktheadroom(osl_t * osh, void *pkt)
{
	BCM_REFERENCE(osh);

	if (IS_FKBUFF_PTR(pkt))
		return (uint) fkb_headroom((FkBuff_t *) PNBUFF_2_PBUF(pkt));
	else
		return (uint) skb_headroom((struct sk_buff *)pkt);
}

uint nbuff_pkttailroom(osl_t * osh, void *pkt)
{
	BCM_REFERENCE(osh);

	if (IS_FKBUFF_PTR(pkt)) {
		printk("%s: no such operation for fkb!\n", __FUNCTION__);
		return 0;
	} else {
		return (uint) skb_tailroom((struct sk_buff *)pkt);
	}
}

uchar *nbuff_pktpush(osl_t * osh, void *pkt, int bytes)
{
	BCM_REFERENCE(osh);
	return (nbuff_push(pkt, bytes));
}

uchar *nbuff_pktpull(osl_t * osh, void *pkt, int bytes)
{
	BCM_REFERENCE(osh);
	return (nbuff_pull(pkt, bytes));
}

bool nbuff_pktshared(void *pkt)
{
	if (IS_SKBUFF_PTR(pkt)) {
		return (((struct sk_buff *)pkt)->cloned);
	} else {
		printk("%s: no such operation for fkb!\n", __FUNCTION__);
		return FALSE;
	}
}

void *nbuff_pktlink(void *pkt)
{
	if (IS_FKBUFF_PTR(pkt))
		return nbuff_get_queue(pkt);
	else
		return (((struct sk_buff *)(pkt))->prev);
}

void nbuff_pktsetlink(void *pkt, void *x)
{
	if (IS_FKBUFF_PTR(pkt))
		nbuff_set_queue(pkt, x);
	else
		(((struct sk_buff *)(pkt))->prev = (struct sk_buff *)(x));
}

void nbuff_pktsetnext(osl_t * osh, void *pkt, void *x)
{
	BCM_REFERENCE(osh);
	if (IS_FKBUFF_PTR(pkt))
		printk("%s: no such operation for fkb!\n", __FUNCTION__);
	else
		(((struct sk_buff *)(pkt))->next = (struct sk_buff *)(x));
}

void *nbuff_pktnext(osl_t * osh, void *pkt)
{
	BCM_REFERENCE(osh);
	if (IS_FKBUFF_PTR(pkt)) {
		printk("%s: no such operation for fkb!\n", __FUNCTION__);

		return NULL;
	} else {
		return (((struct sk_buff *)(pkt))->next);
	}
}

void *nbuff_pktdup(osl_t * osh, void *pkt)
{
	void *p = NULL;

	if (IS_SKBUFF_PTR(pkt)) {
		skb_bpm_tainted((struct sk_buff *)pkt);	// clear SKB_BPM_PRISTINE, dirty
		p = PKTDUP(osh, pkt);
	} else {
		printk("%s: no such operation for fkb!\n", __FUNCTION__);
	}
	return p;
}

void *nbuff_pktdup_cpy(osl_t * osh, void *pkt)
{
	void *p = NULL;

	if (IS_SKBUFF_PTR(pkt)) {
		skb_bpm_tainted((struct sk_buff *)pkt);	// clear SKB_BPM_PRISTINE, dirty
		p = PKTDUP_CPY(osh, pkt);
	} else {
		printk("%s: no such operation for fkb!\n", __FUNCTION__);
	}
	return p;
}

void BCMFASTPATH nbuff_pktfree(osl_t * osh, void *pkt, bool send)
{
	ASSERT(IS_SKBUFF_PTR(pkt));
#ifdef BCM_NBUFF_PKT
	linux_pktfree(osh, pkt, FALSE, send);
#else
	osl_pktfree(osh, pkt, send);
#endif
	return;
}

struct sk_buff *nbuff_pkt_tonative(osl_t * osh, void *pkt)
{
	struct sk_buff *p;

	if (IS_SKBUFF_PTR(pkt))
		p = osl_pkt_tonative(osh, pkt);
	else
		p = (struct sk_buff *)pkt;

	return p;
}

void *nbuff_pkt_frmnative(osl_t * osh, void *pkt)
{
	struct sk_buff *p;

	if (IS_SKBUFF_PTR(pkt))
		p = PKTFRMNATIVE(osh, pkt);
	else
		p = (struct sk_buff *)pkt;

	return p;
}

void inline *nbuff_pkt_get_tag(void *pkt)
{
	if (IS_SKBUFF_PTR(pkt)) {
		return ((void *)(((struct sk_buff *)(pkt))->cb));
	} else {
		FkBuff_t *fkb_p = PNBUFF_2_FKBUFF(pkt);
		return ((void *)(PFKBUFF_TO_PHEAD(fkb_p)));
	}
}

void inline nbuff_pkt_clear_tag(void *pkt)
{
	void *tag = nbuff_pkt_get_tag(pkt);

	*(uint32 *) (tag) = 0;
	*(uint32 *) (tag + 4) = 0;
	*(uint32 *) (tag + 8) = 0;
	*(uint32 *) (tag + 12) = 0;
	*(uint32 *) (tag + 16) = 0;
	*(uint32 *) (tag + 20) = 0;
	*(uint32 *) (tag + 24) = 0;
	*(uint32 *) (tag + 28) = 0;
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
	PKTTAINTED(osh, skb);
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
	PKTTAINTED(osh, skb);

#ifdef BCMDBG_PKT
	osl_pktlist_add(osh, skb, __LINE__, __FILE__);
#endif /* BCMDBG_PKT */

	return skb;
}
#endif /* ! BCM_NBUFF_PKTPOOL_CACHE */

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

typedef struct nbuff_pp_xpm {
	struct sk_buff  * skb_cache;
	/* Local cache of databufs allocated from BPM */
	void		* datapool[PP_XPM_CACHE_MAXPKTS];
	uint8		datapool_curidx;
} nbuff_pp_xpm_t;

/* Initialize local hw SKB/databuf pool */
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

	printk("%s: USING BPM pktpool  initialization %s\n",
		__FUNCTION__, (ret) ? "FAILED" : "SUCCESS");

	return ret;
} /* nbuff_pp_xpm_init */

/* Deinitialize local hw skb/databuf pool */
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

/* Freeup an SKB (No buffer attached) to HW pool */
void
nbuff_pp_xpm_skb_free(osl_t *osh, struct sk_buff *skb)
{
	ASSERT(((osl_pubinfo_t *)osh)->pp_xpm != NULL);
	gbpm_free_skb(skb);
	PKTACCOUNT(osh, 1, FALSE);
}

/* return a databuf from WLAN local pool allocated by HW (like MPM) */
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

/* Freeup databuffer to HW pool */
void
nbuff_pp_xpm_databuf_free(osl_t *osh, void *pdata)
{
	ASSERT(((osl_pubinfo_t *)osh)->pp_xpm != NULL);
	gbpm_free_buf(pdata);
}

/* Attach MPM SKB to MPM Data buffer */
int
nbuff_pp_xpm_attach_skb(osl_t *osh, struct sk_buff *skb, void *pdata,
	uint32 headroom, uint32 len)
{
	gbpm_attach_skb(skb, pdata, (headroom + len));
	skb_cb_zero(skb);

	if (headroom) {
		/* Pull out the headroom */
		skb_pull(skb, headroom);
	}

	return BCME_OK;
} /* nbuff_pp_xpm_attach_skb */
#endif /* PP_XPM */

#endif /* BCM_NBUFF */
