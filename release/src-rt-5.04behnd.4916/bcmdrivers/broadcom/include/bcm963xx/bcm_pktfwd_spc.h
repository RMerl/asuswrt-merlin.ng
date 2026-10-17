/*
   Copyright (c) 2024 Broadcom
   All Rights Reserved

   <:label-BRCM:2024:DUAL/GPL:standard

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

/* this file MUST be included after bcm_pktfwd.h */

#if !defined(__bcm_pktfwd_spc_h_included__)
#define      __bcm_pktfwd_spc_h_included__

#if defined(BCM_PKTFWD_SPC)

#define BCM_PKTFWD_SPC_CREDITS

#define PKTLIST_CTX_MAX_SFRAMES(ctx, prio, dest) \
	((pktlist_credits_t *)&(((pktlist_context_t *)(ctx))->fctable->max_sframes[prio][dest]))

#define PKTLIST_CTX_MAX_AGGBYTES(ctx, prio, dest) \
	((pktlist_credits_t *)&(((pktlist_context_t *)(ctx))->fctable->max_aggbytes[prio][dest]))
/**
 * =============================================================================
 * Section: PKTFWD_FLCTL Functional Interface SPC Extensions
 * =============================================================================
 */

/*
 * -----------------------------------------------------------------------------
 * Function : Set max_sframes of a pktlist identified by the prio and dest.
 * -----------------------------------------------------------------------------
 */

static inline void
__pktlist_fctable_set_max_sframes(pktlist_context_t * pktlist_context,
		uint32_t prio, uint32_t dest, uint32_t max_sframes)
{
	pktlist_credits_t * pktlist_max_sframes;

	/* Locate the pktlist credit */
	pktlist_max_sframes = PKTLIST_CTX_MAX_SFRAMES(pktlist_context, prio, dest);
	atomic_set(pktlist_max_sframes, max_sframes);
}   /* __pktlist_fctable_set_max_sframes() */

/*
 * -----------------------------------------------------------------------------
 * Function : Get max_sframes of a pktlist identified by the prio and dest.
 * -----------------------------------------------------------------------------
 */
static inline int32_t
__pktlist_fctable_get_max_sframes(pktlist_context_t * pktlist_context,
		uint32_t prio, uint32_t dest)
{
	pktlist_credits_t * pktlist_max_sframes;

	/* Locate the pktlist credit */
	pktlist_max_sframes = PKTLIST_CTX_MAX_SFRAMES(pktlist_context, prio, dest);
	return (atomic_read(pktlist_max_sframes));
}   /* __pktlist_fctable_get_max_sframes() */

/*
 * -----------------------------------------------------------------------------
 * Function : Set max_aggbytes of a pktlist identified by the prio and dest.
 * -----------------------------------------------------------------------------
 */
static inline void
__pktlist_fctable_set_max_aggbytes(pktlist_context_t * pktlist_context,
		uint32_t prio, uint32_t dest, uint32_t max_aggbytes)
{
	pktlist_credits_t * pktlist_max_aggbytes;

	/* Locate the pktlist credit */
	pktlist_max_aggbytes = PKTLIST_CTX_MAX_AGGBYTES(pktlist_context, prio, dest);
	atomic_set(pktlist_max_aggbytes, max_aggbytes);
}   /* __pktlist_fctable_set_max_aggbytes() */

/*
 * -----------------------------------------------------------------------------
 * Function : Get max_aggbytes of a pktlist identified by the prio and dest.
 * -----------------------------------------------------------------------------
 */
static inline int32_t
__pktlist_fctable_get_max_aggbytes(pktlist_context_t * pktlist_context,
		uint32_t prio, uint32_t dest)
{
	pktlist_credits_t * pktlist_max_aggbytes;

	/* Locate the pktlist credit */
	pktlist_max_aggbytes = PKTLIST_CTX_MAX_AGGBYTES(pktlist_context, prio, dest);
	return (atomic_read(pktlist_max_aggbytes));
}   /* __pktlist_fctable_get_max_aggbytes() */

/*
 * We can add a frag if the sum of: + padded total length of the frags
 * we have added so far + UNPADDED length of the candidate frag fit in
 * maxagg. The last one is unpadded because it is currently the very
 * last frag.
 */
static inline bool
__pktlist_spc_can_add(void *head, uint16_t len, uint16_t maxsf,
		uint16_t maxagg, const NBuffPtrType_t NBuffPtrType)
{
#if defined(CONFIG_BCM_WKB)
	if (NBuffPtrType == WKBUFF_PTR) {
		wkbuff_t * wkb = PNBUFF_2_WKBUFF(head);

		return ((wkb->frag_num < maxsf) &&
			((ALIGN(wkb_totlen(wkb), WKB_FRAG_LEN_ALIGN) + len) <= maxagg));
	} else
#endif /* CONFIG_BCM_WKB */
	{
		struct sk_buff *h = PNBUFF_2_SKBUFF(head);
		uint32_t spc_tot_len = h->spc_tot_len;
		return (((skb_shinfo(h)->nr_frags + 1) < maxsf) &&
			((ALIGN(max(h->len, spc_tot_len), 4) + len) <= maxagg));
	}
} /* __pktlist_spc_can_add() */

static inline void
__pktlist_skb_fill_data_desc(struct sk_buff *skb, int i,
		void *pHead, uint16_t len, uint16_t offset)
{
	skb_shinfo(skb)->frags[i].data = pHead + offset;
	skb_frag_off_set(&skb_shinfo(skb)->frags[i], offset);
	skb_frag_size_set(&skb_shinfo(skb)->frags[i], len);
}

static inline uint8_t
__pktlist_add_frag(void *head, void *pHead, uint16_t len, uint16_t offset)
{
	struct sk_buff *h = head;
	uint16_t i = skb_shinfo(h)->nr_frags;

	__pktlist_skb_fill_data_desc(h, i, pHead, len, offset);
	h->spc_tot_len += len;
	skb_shinfo(h)->nr_frags = i + 1;
	return i;
}

static inline uint
__pktlist_amsdu_subframe_needs_pad(uint16_t len)
{
	return !IS_ALIGNED(len, 4);
}

static inline void
__pktlist_pad_data(void *head)
{
	struct sk_buff *h = (struct sk_buff *) head;

	if (__pktlist_amsdu_subframe_needs_pad(h->len)) {
		h->len = ALIGN(h->len, 4);
	}
}

static inline void
__pktlist_pad_frag(struct sk_buff *h, uint8_t ix)
{
	uint16_t len = skb_frag_size(&skb_shinfo(h)->frags[ix]);

	if (__pktlist_amsdu_subframe_needs_pad(len)) {

		uint16_t pad = ALIGN(len, 4) - len;
		skb_frag_size_add(&skb_shinfo(h)->frags[ix], pad);

		h->spc_tot_len += pad;
	}
}

static inline void
__pktlist_commit_spc_pkt(pktlist_t * pktlist, NBuffPtrType_t NBuffPtrType)
{
	PKTLIST_PKT_SET_SLL(pktlist->tail, pktlist->spc_pkt, NBuffPtrType);
	pktlist->tail = pktlist->spc_pkt;
	pktlist->spc_pkt = PKTLIST_PKT_NULL;
	pktlist->len++;
}

static inline void
__pktlist_save_spc_pkt(pktlist_context_t *src_pktlist_context,
		pktlist_t * pktlist, NBuffPtrType_t NBuffPtrType)
{
	if (pktlist->spc_pkt != PKTLIST_PKT_NULL) {
		PKTLIST_PKT_SET_SLL(pktlist->spc_pkt, src_pktlist_context->spc_pkts,
				NBuffPtrType);
		src_pktlist_context->spc_pkts = pktlist->spc_pkt;
		pktlist->spc_pkt = PKTLIST_PKT_NULL;
	}
}

static inline void
__pktlist_restore_spc_pkts(pktlist_context_t * src_pktlist_context,
		NBuffPtrType_t NBuffPtrType)
{
	void * spc_pkt;
	uint16_t key;
	uint16_t dest;
	uint16_t prio;

	while (src_pktlist_context->spc_pkts != PKTLIST_PKT_NULL) {

		spc_pkt = src_pktlist_context->spc_pkts;
		src_pktlist_context->spc_pkts =
			PKTLIST_PKT_SLL(src_pktlist_context->spc_pkts, NBuffPtrType);
		PKTLIST_PKT_SET_SLL(spc_pkt, PKTLIST_PKT_NULL, NBuffPtrType);

		key = PKTLIST_PKT_KEY(spc_pkt, NBuffPtrType);
		dest = PKTLIST_DEST(key);

#if defined(CONFIG_BCM_WKB)
		if (NBuffPtrType == WKBUFF_PTR) {
			prio = GET_WLAN_PRIORITY(PNBUFF_2_WKBUFF(spc_pkt)->wl.ucast.nic.wl_prio);
		} else
#endif /* CONFIG_BCM_WKB */
		{
			prio = GET_WLAN_PRIORITY(PNBUFF_2_SKBUFF(spc_pkt)->wl.ucast.nic.wl_prio);
			PNBUFF_2_SKBUFF(spc_pkt)->is_spc_candidate = true;
		}

		__pktlist_add_pkt(src_pktlist_context,
				prio, dest, key, spc_pkt, NBuffPtrType);
	}
}

static inline void
__pktlist_commit_spc_pkts(
	pktlist_context_t * src_pktlist_context,    /* producer context */
	dll_t             * src_work_list,          /* producer's work list */
	NBuffPtrType_t      NBuffPtrType)
{
	pktlist_elem_t * src_elem;
	dll_t * item, * next; /* dll iterator */

	if (dll_empty(src_work_list)) {
		return; /* no pktlists have been accumulated for xfer */
	}

	for (item = dll_head_p(src_work_list); !dll_end(src_work_list, item);) {
		next = dll_next_p(item); /* iterator's next */
		src_elem = _envelope_of(item, pktlist_elem_t, node);

		PKTLIST_ASSERT(src_elem->pktlist.len != 0U);

		if (src_elem->pktlist.spc_pkt != PKTLIST_PKT_NULL) {
			__pktlist_commit_spc_pkt(&src_elem->pktlist, NBuffPtrType);
		}
		item = next; /* advance src work list iterator */
	}
}

static inline void
__pktlist_save_spc_pkts(
	pktlist_context_t * src_pktlist_context,    /* producer context */
	dll_t             * src_work_list,          /* producer's work list */
	NBuffPtrType_t      NBuffPtrType)
{
	pktlist_elem_t * src_elem;
	dll_t * item, * next; /* dll iterator */

	if (dll_empty(src_work_list)) {
		return; /* no pktlists have been accumulated for xfer */
	}

	for (item = dll_head_p(src_work_list); !dll_end(src_work_list, item);) {
		next = dll_next_p(item); /* iterator's next */
		src_elem = _envelope_of(item, pktlist_elem_t, node);

		PKTLIST_ASSERT(src_elem->pktlist.len != 0U);
		__pktlist_save_spc_pkt(src_pktlist_context,
			 &src_elem->pktlist, NBuffPtrType);
		item = next; /* advance src work list iterator */
	}
}

/*
 * -----------------------------------------------------------------------------
 *
 * Function   : __pktlist_append_data
 * Description: Add a data buffer to the tail container of a packet list.
 *              Packet list is identified by the prio and the destination.
 *              If the packet list was empty, or the tail cannot admit
 *              more data buffers, then return 0. Otherwise return 1 or
 *              the number of data buffers added (for future expansion
 *              perhaps).
 *
 * -----------------------------------------------------------------------------
 */

static inline uint8_t
__pktlist_append_data(pktlist_context_t * pktlist_context,
		uint32_t prio, uint32_t dest, uint16_t key, void * pHead,
		uint32_t len, uint32_t offset, const NBuffPtrType_t NBuffPtrType,
		bool qstat_telemetry)
{
	pktlist_t * pktlist;
	pktlist_elem_t * pktlist_elem;
	uint16_t max_sframes, max_aggbytes;
	bool using_spc_pkt = false;

	PKTLIST_ASSERT(pktlist_context != PKTLIST_CONTEXT_NULL);

	/* Locate the pktlist element and the contained pktlist */
	pktlist_elem = PKTLIST_CTX_ELEM(pktlist_context, prio, dest);
	pktlist = &pktlist_elem->pktlist;

	if (pktlist->len == 0U) {
		return 0;
	}

	max_sframes = __pktlist_fctable_get_max_sframes(pktlist_context,
			prio, dest);

	max_aggbytes = __pktlist_fctable_get_max_aggbytes(pktlist_context,
			prio, dest);

#if defined(CONFIG_BCM_WKB)
	if (NBuffPtrType == WKBUFF_PTR) {
		wkbuff_t	* wkb;
		void		* pStart = BCM_PHEAD_TO_PSTART(pHead);
		uint8_t	 	* data = (uint8_t *)pHead + offset;
		uint16_t	db_pool;

		if (max_sframes < 2) {
			/* cannot aggregate or aggregation is disabled */
			if (pktlist->spc_pkt) {
				__pktlist_commit_spc_pkt(pktlist, NBuffPtrType);
			}
			return 0;
		}

		if (pktlist->spc_pkt != PKTLIST_PKT_NULL) {
			wkb = PNBUFF_2_WKBUFF(pktlist->spc_pkt);
			using_spc_pkt = true;
		} else {
			wkb = PNBUFF_2_WKBUFF(pktlist->tail);
		}

		if (gbpm_is_buf_hw_recycle_capable(pStart))
			db_pool = WKB_DB_MPM;
		else
			db_pool = WKB_DB_BPM;

		/* Add new packet to the WKB fragments only if AMSDU aggregation
		 * conditions are met
		 * - Fragments <= Max Subframes
		 * - Total length of WKB (including PAD) fits in Max aggbytes
		 *
		 * XXX: BCM_WKB_TEST: TODO: WKB SDU chainging (WKB_FRAGCNT_MAX)
		 */
		if (!__pktlist_spc_can_add(WKBUFF_2_PNBUFF(wkb), len, max_sframes,
			max_aggbytes, WKBUFF_PTR)) {

			if (using_spc_pkt) {
				__pktlist_commit_spc_pkt(pktlist, NBuffPtrType);
			}

			WKB_TRACE("Agg limit reached: max_sframes %d max_aggbytes %d\n"
				"\twkb->frag_num %d wkb_totlen %d pkt len %d\n",
				max_sframes, max_aggbytes, wkb->frag_num, wkb_totlen(wkb), len);
			return 0;
		}

		/* Fill the MSDU fragment to WKB */
		wkb_frag_fill_item(wkb, pStart, data, len, db_pool, true);

		WKB_ASSERT(!qstat_telemetry);

		return 1;
	} else
#endif /* CONFIG_BCM_WKB */
	{
		struct sk_buff *spc_head;

		PKTFWD_ASSERT(NBuffPtrType == SKBUFF_PTR);

		if (pktlist->spc_pkt != PKTLIST_PKT_NULL) {
			spc_head = pktlist->spc_pkt;
			using_spc_pkt = true;
		} else {
			spc_head = pktlist->tail;
		}

		if (!spc_head->is_spc_candidate) {
			return 0;
		}

		if (max_sframes < 2) {
			spc_head->is_spc_candidate = false;
			/* cannot aggregate or aggregation is disabled */

			if (using_spc_pkt) {
				__pktlist_commit_spc_pkt(pktlist, NBuffPtrType);
			}
			return 0;
		}

		/*
		 * if adding this SKB to the fraglist does NOT put us
		 * over the limit, do it
		 */
		if (__pktlist_spc_can_add(spc_head, len, max_sframes, max_aggbytes,
			SKBUFF_PTR)) {
			uint8_t i = __pktlist_add_frag(spc_head, pHead, len, offset);

			if (i) {
				__pktlist_pad_frag(spc_head, i - 1);
			} else {
				__pktlist_pad_data(spc_head);
				spc_head->is_spc = true;
				spc_head->spc_tot_len = spc_head->len + len;
			}

			if (qstat_telemetry) {
				skb_spc_frag_set_qstat_telemetry(spc_head, i);
			}

			return 1;
		} else {
			spc_head->is_spc_candidate = false;

			if (using_spc_pkt) {
				__pktlist_commit_spc_pkt(pktlist, NBuffPtrType);
			}
		}
	}

	return 0;
}   /* __pktlist_append_data() */

#endif /* BCM_PKTFWD_SPC */

#endif /* __bcm_pktfwd_spc_h_included__ */
