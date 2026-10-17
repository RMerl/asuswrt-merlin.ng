/*
 * Linux Packet (skb) interface
 *
 * Copyright (C) 2025, Broadcom. All Rights Reserved.
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
 * $Id: linux_pkt.h 848131 2024-11-15 17:21:18Z $
 */

#ifndef _linux_pkt_h_
#define _linux_pkt_h_

#include <typedefs.h>

#ifdef __ARM_ARCH_7A__
#define PKT_HEADROOM_DEFAULT NET_SKB_PAD /**< NET_SKB_PAD is defined in a linux kernel header */
#else
#define PKT_HEADROOM_DEFAULT 16
#endif /* __ARM_ARCH_7A__ */

#ifdef BCMDRIVER
/*
 * BINOSL selects the slightly slower function-call-based binary compatible osl.
 * Macros expand to calls to functions defined in linux_osl.c .
 */
#ifndef BINOSL
/* Because the non BINOSL implemenation of the PKT OSL routines are macros (for
 * performance reasons),  we need the Linux headers.
 */
#include <linuxver.h>

#if defined(BCM_NBUFF_PKT)

#include <nbuff_pkt.h>

#define PKTSHAREDINFO(osh, skb)		\
	({BCM_REFERENCE(osh); skb_shinfo((struct sk_buff *)(skb));})

/*
 * With WLAN Single Packet Context (SPC), a single Linux skbuff represents a
 * 802.11 M-PDU, with N number of 802.11 M-SDU's data fragments carried via the
 * skb_shared_info's skb_frag frag table, indexed by the sequence of M-SDU's
 * databuffers. The number of segments (PKTSEGCNT) is the data of this SKB plus
 * the number of fragments.
 * The number of fragments (PKTFRAGCNT) can be used to iterate over just the
 * attached fragments to an SPC SKB.
 */
#define PKTSEGCNT(osh, skb)			\
	({BCM_REFERENCE(osh); (PKTSHAREDINFO((osh), (skb))->nr_frags + 1);})
#define PKTFRAGCNT(osh, skb)			\
	({BCM_REFERENCE(osh); (PKTSHAREDINFO((osh), (skb))->nr_frags);})
#define PKTFRAGCNTSET(osh, skb, cnt)		\
	({BCM_REFERENCE(osh); (PKTSHAREDINFO((osh), (skb))->nr_frags) = (cnt);})
#define PKTFRAG(osh, skb, ix)			\
	({BCM_REFERENCE(osh); (&PKTSHAREDINFO((osh), (skb))->frags[ix]);})
#define PKTFRAGLEN(osh, skb, ix)		\
	(skb_frag_size((const skb_frag_t *) PKTFRAG(osh, skb, ix)))
#define PKTFRAGLENSET(osh, skb, ix, l)	\
	(skb_frag_size_set((skb_frag_t *) PKTFRAG(osh, skb, ix), (l)))
#define PKTFRAGLENADD(osh, skb, ix, l)	\
	(skb_frag_size_add((skb_frag_t *) PKTFRAG(osh, skb, ix), (l)))
#define PKTFRAGLENSUB(osh, skb, ix, l)	\
	(skb_frag_size_sub((skb_frag_t *) PKTFRAG(osh, skb, ix), (l)))
#define PKTFRAGDATA(osh, skb, ix)		\
	(PKTSHAREDINFO((osh), (skb))->frags[ix].data)
#define PKTFRAGDATASET(osh, skb, ix, d)		\
	(PKTSHAREDINFO((osh), (skb))->frags[ix].data = (d))
#define PKTFRAGOFFSET(osh, skb, ix)		\
	(skb_frag_off(&PKTSHAREDINFO((osh), (skb))->frags[ix]))

#ifdef BCM_SPC_RX
#define PKTFRAGOFFSETSET(osh, skb, ix, off)		\
	({skb_frag_off_set(PKTFRAG(osh, skb, ix), off); })
#define PKTFRAGOFFSETADD(osh, skb, ix, offset)		\
	({skb_frag_off_set(PKTFRAG(osh, skb, ix), (PKTFRAGOFFSET(osh, skb, ix) + offset)); })
#define PKTFRAGOFFSETSUB(osh, skb, ix, offset)		\
	({skb_frag_off_set(PKTFRAG(osh, skb, ix), (PKTFRAGOFFSET(osh, skb, ix) - offset)); })

static inline void *
frag_push(void * osh, void *p, int ix, int len)
{
	PKTFRAGLENADD(osh, p, ix, len);
	PKTFRAGOFFSETSUB(osh, p, ix, len);
	PKTFRAGDATASET(osh, p, ix, ((uint8*)(PKTFRAGDATA(osh, p, ix)) - len));
	return PKTFRAGDATA(osh, p, ix);
}

static inline void *
frag_pull(void * osh, void *p, int ix, int len)
{
	PKTFRAGLENSUB(osh, p, ix, len);
	PKTFRAGOFFSETADD(osh, p, ix, len);
	PKTFRAGDATASET(osh, p, ix, ((uint8*)(PKTFRAGDATA(osh, p, ix)) + len));
	return PKTFRAGDATA(osh, p, ix);
}

#define PKTFRAGPULL(osh, skb, ix, offset)	(frag_pull(osh, skb, ix, offset))
#define PKTFRAGPUSH(osh, skb, ix, offset)	(frag_push(osh, skb, ix, offset))
#endif /* BCM_SPC_RX */

/*
 * SPC is not supported or required with dhd. These macros are only available to
 * wl and hnd modules. dhd gets the dummy versions of them in the else clause
 * when needed.
 */
#if defined(CONFIG_HAVE_SKBUFF_WL_SPC_V2) && !defined(BCM_ROUTER_DHD)
/*
 * The following macros require sk_buff to have the spc fields which is
 * defined after the 504L04P3 release. CONFIG_HAVE_SKBUFF_WL_SPC_V2 is our
 * compatibility test.
 * Additionally SKBs that DHD generates or uses have no use for the SPC feature
 * so the dhd driver is also excluded from using spc.
 */
#define PKTISSPC(osh, skb)		\
	({BCM_REFERENCE(osh); ((struct sk_buff*)(skb))->is_spc;})

#define PKTSETSPC(osh, skb)		\
	({BCM_REFERENCE(osh); ((struct sk_buff*)(skb))->is_spc = TRUE;})

#define PKTCLRSPC(osh, skb)		\
	({BCM_REFERENCE(osh); ((struct sk_buff*)(skb))->is_spc = FALSE;})

#if defined(BCM_PKTFWD_SPC)
#define PKTCLRSPCCANDIDATE(osh, skb)		\
	({BCM_REFERENCE(osh); ((struct sk_buff*)(skb))->is_spc_candidate = FALSE;})
#else /* CONFIG_BCM_PKTFWD_SPC */
#define PKTCLRSPCCANDIDATE(osh, skb)
#endif /* CONFIG_BCM_PKTFWD_SPC */

#define PKTTOTLEN(osh, skb)	\
	({BCM_REFERENCE(osh); ((struct sk_buff*)(skb))->spc_tot_len;})

#define PKTADDTOTLEN(osh, skb, l)	\
	({BCM_REFERENCE(osh); (((struct sk_buff*)(skb))->spc_tot_len += (l));})

#define PKTSUBTOTLEN(osh, skb, l)	\
	({BCM_REFERENCE(osh); (((struct sk_buff*)(skb))->spc_tot_len -= (l));})

#define PKTSETTOTLEN(osh, skb, l)	\
	({BCM_REFERENCE(osh); (((struct sk_buff*)(skb))->spc_tot_len = (l));})

#else /* CONFIG_HAVE_SKBUFF_WL_SPC_V2 && !BCM_ROUTER_DHD */
#define PKTISSPC(osh, skb)	FALSE
#define PKTSETSPC(osh, skb)
#define PKTCLRSPC(osh, skb)
#define PKTCLRSPCCANDIDATE(osh, skb)
#define PKTTOTLEN(osh, skb)	pkttotlen(osh, skb)
#endif /* CONFIG_HAVE_SKBUFF_WL_SPC_V2 && !BCM_ROUTER_DHD */

#ifndef	PKTDEV
#define PKTDEV(skb)			(((struct sk_buff *)(skb))->dev)
#define PKTSETDEV(skb, _dev)		(((struct sk_buff *)(skb))->dev = (_dev))
#define PKTBLOG(skb)			(((struct sk_buff *)(skb))->blog_p)
#endif

#else /* ================== ! BCM_NBUFF_PKT ================= */

#if !(defined(CMWIFI) && defined(CMWIFI_EROUTER))
#define PKTGET_DATA			PKTGET
#endif

/* packet primitives */
#ifdef BCMDBG_CTRACE
#if defined(CMWIFI) && defined(CMWIFI_EROUTER)
#define	PKTGET_DATA(osh, len, send)	osl_pktget_data((osh), (len), __LINE__, __FILE__)
#endif /* CMWIFI && CMWIFI_EROUTER */
#define	PKTGET(osh, len, send)		linux_pktget((osh), (len), __LINE__, __FILE__)
#define	PKTDUP(osh, skb)		osl_pktdup((osh), (skb), __LINE__, __FILE__)
#define PKTDUP_CPY(osh, skb)		osl_pktdup_cpy((osh), (skb), __LINE__, __FILE__)
#else
#ifdef BCM_OBJECT_TRACE
#if defined(CMWIFI) && defined(CMWIFI_EROUTER)
#define	PKTGET_DATA(osh, len, send)	osl_pktget_data((osh), (len), __LINE__, __FUNCTION__)
#endif /* CMWIFI && CMWIFI_EROUTER */
#define	PKTGET(osh, len, send)		linux_pktget((osh), (len), __LINE__, __FUNCTION__)
#define	PKTDUP(osh, skb)		osl_pktdup((osh), (skb), __LINE__, __FUNCTION__)
#define PKTDUP_CPY(osh, skb)		osl_pktdup_cpy((osh), (skb), __LINE__, __FUNCTION__)
#else
#if defined(CMWIFI) && defined(CMWIFI_EROUTER)
#define	PKTGET_DATA(osh, len, send)	osl_pktget_data((osh), (len))
#endif /* CMWIFI && CMWIFI_EROUTER */
#define	PKTGET(osh, len, send)		linux_pktget((osh), (len))
#define	PKTDUP(osh, skb)		osl_pktdup((osh), (skb))
#define PKTDUP_CPY(osh, skb)		osl_pktdup_cpy((osh), (skb))
#endif /* BCM_OBJECT_TRACE */
#endif /* BCMDBG_CTRACE */
#define PKTLIST_DUMP(osh, buf)		BCM_REFERENCE(osh)
#define PKTDBG_TRACE(osh, pkt, bit)	BCM_REFERENCE(osh)

#ifndef PKTFREE_NEW_API
#if defined(BCM_OBJECT_TRACE)
#define	PKTFREE(osh, skb, send)		linux_pktfree((osh), (skb), (send), \
						__LINE__, __FUNCTION__)
#else
#define	PKTFREE(osh, skb, send)		linux_pktfree((osh), (skb), (send))
#endif /* BCM_OBJECT_TRACE */
#else
#if defined(BCM_OBJECT_TRACE)
#define	PKTFREE(osh, skb, send)		linux_pktfree((osh), (skb), TRUE, (send), \
						__LINE__, __FUNCTION__)
#else
#define	PKTFREE(osh, skb, send)		linux_pktfree((osh), (skb), TRUE, (send))
#endif /* BCM_OBJECT_TRACE */
#endif /* PKTFREE_NEW_API */

#ifdef CONFIG_DHD_USE_STATIC_BUF
#define	PKTGET_STATIC(osh, len, send)		osl_pktget_static((osh), (len))
#define	PKTFREE_STATIC(osh, skb, send)		osl_pktfree_static((osh), (skb), (send))
#else
#define	PKTGET_STATIC	PKTGET
#define	PKTFREE_STATIC	PKTFREE
#endif /* CONFIG_DHD_USE_STATIC_BUF */
#define	PKTDATA(osh, skb)		({BCM_REFERENCE(osh); (((struct sk_buff*)(skb))->data);})
#define	PKTLEN(osh, skb)		({BCM_REFERENCE(osh); (((struct sk_buff*)(skb))->len);})
#define	PKTHEAD(osh, skb)		({BCM_REFERENCE(osh); (((struct sk_buff*)(skb))->head);})
#define PKTSETHEAD(osh, skb, h)		({BCM_REFERENCE(osh); \
					(((struct sk_buff *)(skb))->head = (h));})
#define PKTEXPHEADROOM(osh, skb, b)	\
	({ \
	 BCM_REFERENCE(osh); \
	 skb_realloc_headroom((struct sk_buff*)(skb), (b)); \
	 })
#define PKTHEADROOM(osh, skb)		\
	({ \
	 BCM_REFERENCE(osh); \
	 skb_headroom((struct sk_buff*)(skb)); \
	 })
#define PKTTAILROOM(osh, skb)		\
	({ \
	 BCM_REFERENCE(osh); \
	 skb_tailroom((struct sk_buff*)(skb)); \
	 })
#define PKTPADTAILROOM(osh, skb, padlen) \
	({ \
	 BCM_REFERENCE(osh); \
	 skb_pad((struct sk_buff*)(skb), (padlen)); \
	 })
#define	PKTNEXT(osh, skb)		({BCM_REFERENCE(osh); (((struct sk_buff*)(skb))->next);})
#define	PKTSETNEXT(osh, skb, x)		\
	({ \
	 BCM_REFERENCE(osh); \
	 (((struct sk_buff*)(skb))->next = (struct sk_buff*)(x)); \
	 })
#define	PKTSETLEN(osh, skb, len)	\
	({ \
	 BCM_REFERENCE(osh); \
	 __skb_trim((struct sk_buff*)(skb), (len)); \
	 })
#define	PKTPUSH(osh, skb, bytes)	\
	({ \
	 BCM_REFERENCE(osh); \
	 skb_push((struct sk_buff*)(skb), (bytes)); \
	 })
#define	PKTPULL(osh, skb, bytes)	\
	({ \
	 BCM_REFERENCE(osh); \
	 skb_pull((struct sk_buff*)(skb), (bytes)); \
	 })
#define	PKTTAG(skb)			((void*)(((struct sk_buff*)(skb))->cb))
#define PKTSETPOOL(osh, skb, x, y)	BCM_REFERENCE(osh)
#define	PKTPOOL(osh, skb)		({BCM_REFERENCE(osh); BCM_REFERENCE(skb); FALSE;})
#define PKTFREELIST(skb)		PKTLINK(skb)
#define PKTSETFREELIST(skb, x)		PKTSETLINK((skb), (x))
#define PKTPTR(skb)			(skb)
#define PKTID(skb)			({BCM_REFERENCE(skb); 0;})
#define PKTSETID(skb, id)		({BCM_REFERENCE(skb); BCM_REFERENCE(id);})
#define PKTDEV(skb)			(((struct sk_buff *)(skb))->dev)
#define PKTSETDEV(skb, _dev)		(((struct sk_buff *)(skb))->dev = (_dev))

#if defined(TSQ_MULTIPLIER)
#define PKTORPHAN(skb)          osl_pkt_orphan_partial(skb)
extern void osl_pkt_orphan_partial(struct sk_buff *skb);
#elif defined(PKT_FULL_ORPHAN)
#define PKTORPHAN(skb)          skb_orphan(skb)
#else /* !TSQ_MULTIPLIER && !PKT_FULL_ORPHAN */
#define PKTORPHAN(skb)          ({BCM_REFERENCE(skb); 0;})
#endif

#ifdef BCMDBG_CTRACE
#define	DEL_CTRACE(zosh, zskb) { \
	unsigned long zflags; \
	spin_lock_irqsave(&(zosh)->ctrace_lock, zflags); \
	list_del(&(zskb)->ctrace_list); \
	(zosh)->ctrace_num--; \
	(zskb)->ctrace_start = 0; \
	(zskb)->ctrace_count = 0; \
	spin_unlock_irqrestore(&(zosh)->ctrace_lock, zflags); \
}

#define	UPDATE_CTRACE(zskb, zfile, zline) { \
	struct sk_buff *_zskb = (struct sk_buff *)(zskb); \
	if (_zskb->ctrace_count < CTRACE_NUM) { \
		_zskb->func[_zskb->ctrace_count] = zfile; \
		_zskb->line[_zskb->ctrace_count] = zline; \
		_zskb->ctrace_count++; \
	} \
	else { \
		_zskb->func[_zskb->ctrace_start] = zfile; \
		_zskb->line[_zskb->ctrace_start] = zline; \
		_zskb->ctrace_start++; \
		if (_zskb->ctrace_start >= CTRACE_NUM) \
			_zskb->ctrace_start = 0; \
	} \
}

#define	ADD_CTRACE(zosh, zskb, zfile, zline) { \
	unsigned long zflags; \
	spin_lock_irqsave(&(zosh)->ctrace_lock, zflags); \
	list_add(&(zskb)->ctrace_list, &(zosh)->ctrace_list); \
	(zosh)->ctrace_num++; \
	UPDATE_CTRACE(zskb, zfile, zline); \
	spin_unlock_irqrestore(&(zosh)->ctrace_lock, zflags); \
}

#define PKTCALLER(zskb)	UPDATE_CTRACE((struct sk_buff *)zskb, (char *)__FUNCTION__, __LINE__)
#endif /* BCMDBG_CTRACE */

#define	PKTSETFAST(osh, skb)	({BCM_REFERENCE(osh); BCM_REFERENCE(skb);})
#define	PKTCLRFAST(osh, skb)	({BCM_REFERENCE(osh); BCM_REFERENCE(skb);})
#define	PKTISFAST(osh, skb)	({BCM_REFERENCE(osh); BCM_REFERENCE(skb); FALSE;})

#define	PKTSETCTF(osh, skb)	({BCM_REFERENCE(osh); BCM_REFERENCE(skb);})
#define	PKTCLRCTF(osh, skb)	({BCM_REFERENCE(osh); BCM_REFERENCE(skb);})
#define	PKTISCTF(osh, skb)	({BCM_REFERENCE(osh); BCM_REFERENCE(skb); FALSE;})

#if defined(PKTC) && defined(PKTC_TAG_IN_SKB_CB)
/* use local instead of platform specific constructs */
#define SKIPCT		(1 << 0)
#define CHAINED		(1 << 1)
#define PKTSETSKIPCT(osh, skb)	(PKTCFLAGS(skb) |= SKIPCT)
#define PKTCLRSKIPCT(osh, skb)	(PKTCFLAGS(skb) &= (~SKIPCT))
#define PKTSKIPCT(osh, skb)		(PKTCFLAGS(skb) & SKIPCT)
#define PKTSETCHAINED(osh, skb)	(PKTCFLAGS(skb) |= CHAINED)
#define PKTCLRCHAINED(osh, skb)	(PKTCFLAGS(skb) &= (~CHAINED))
#define PKTISCHAINED(skb)		(PKTCFLAGS(skb) & CHAINED)
#endif  /* (PKTC && PKTC_TAG_IN_SKB_CB) */

#if !(defined(CMWIFI) && defined(CMWIFI_EROUTER))
#if defined(PKTC) && !defined(PKTC_TAG_IN_SKB_CB)

#define SKIPCT  (1 << 2)
#define CHAINED (1 << 3)
#define PKTSETSKIPCT(osh, skb)  \
	({ \
	 BCM_REFERENCE(osh); \
	 (skbuff_bcm_ext_wlan_get((struct sk_buff*)(skb), pktc_flags) |= SKIPCT); \
	 })
#define PKTCLRSKIPCT(osh, skb)  \
	({ \
	 BCM_REFERENCE(osh); \
	 (skbuff_bcm_ext_wlan_get((struct sk_buff*)(skb), pktc_flags) &= (~SKIPCT)); \
	 })
#define PKTSKIPCT(osh, skb) \
	({ \
	 BCM_REFERENCE(osh); \
	 (skbuff_bcm_ext_wlan_get((struct sk_buff*)(skb), pktc_flags) & SKIPCT); \
	 })
#define PKTSETCHAINED(osh, skb) \
	({ \
	 BCM_REFERENCE(osh); \
	 (skbuff_bcm_ext_wlan_get((struct sk_buff*)(skb), pktc_flags) |= CHAINED); \
	 })
#define PKTCLRCHAINED(osh, skb) \
	({ \
	 BCM_REFERENCE(osh); \
	 (skbuff_bcm_ext_wlan_get((struct sk_buff*)(skb), pktc_flags) &= (~CHAINED)); \
	 })

#ifdef BCM_BLOG
#define	PKTISCHAINED(skb)	(IS_SKBUFF_PTR(skb) ? \
	 (skbuff_bcm_ext_wlan_get((struct sk_buff*)(skb), pktc_flags) & CHAINED) : FALSE)
#else
#define PKTISCHAINED(skb)   (skbuff_bcm_ext_wlan_get((struct sk_buff*)(skb), pktc_flags) & CHAINED)
#endif

#define CTF_MARK(m)				(m.value)

#else /* (PKTC && !PKTC_TAG_IN_SKB_CB) */

#define	PKTSETSKIPCT(osh, skb)	({BCM_REFERENCE(osh); BCM_REFERENCE(skb);})
#define	PKTCLRSKIPCT(osh, skb)	({BCM_REFERENCE(osh); BCM_REFERENCE(skb);})
#define	PKTSKIPCT(osh, skb)	({BCM_REFERENCE(osh); BCM_REFERENCE(skb);})
#define CTF_MARK(m)		({BCM_REFERENCE(m); 0;})
#endif /* (PKTC && !PKTC_TAG_IN_SKB_CB) */
#endif /* !(CMWIFI && CMWIFI_EROUTER) */

#define PKTFRAGLEN(osh, lb, ix)			(0)
#define PKTSETFRAGLEN(osh, lb, ix, len)		BCM_REFERENCE(osh)

#if !(defined(CMWIFI) && defined(CMWIFI_EROUTER))
/* For broadstream iqos */
#define	PKTSETTOBR(osh, skb)	({BCM_REFERENCE(osh); BCM_REFERENCE(skb);})
#define	PKTCLRTOBR(osh, skb)	({BCM_REFERENCE(osh); BCM_REFERENCE(skb);})
#define	PKTISTOBR(skb)	({BCM_REFERENCE(skb); FALSE;})
#endif /* !(CMWIFI && CMWIFI_EROUTER) */

#ifdef BCMFA
#ifdef BCMFA_HW_HASH
#define PKTSETFAHIDX(skb, idx)	(((struct sk_buff*)(skb))->napt_idx = idx)
#else
#define PKTSETFAHIDX(skb, idx)	({BCM_REFERENCE(skb); BCM_REFERENCE(idx);})
#endif /* BCMFA_SW_HASH */
#define PKTGETFAHIDX(skb)	(((struct sk_buff*)(skb))->napt_idx)
#define PKTSETFADEV(skb, imp)	(((struct sk_buff*)(skb))->dev = imp)
#define PKTSETRXDEV(skb)	(((struct sk_buff*)(skb))->rxdev = ((struct sk_buff*)(skb))->dev)

#define	AUX_TCP_FIN_RST	(1 << 0)
#define	AUX_FREED	(1 << 1)
#define PKTSETFAAUX(skb)	(((struct sk_buff*)(skb))->napt_flags |= AUX_TCP_FIN_RST)
#define	PKTCLRFAAUX(skb)	(((struct sk_buff*)(skb))->napt_flags &= (~AUX_TCP_FIN_RST))
#define	PKTISFAAUX(skb)		(((struct sk_buff*)(skb))->napt_flags & AUX_TCP_FIN_RST)
#define PKTSETFAFREED(skb)	(((struct sk_buff*)(skb))->napt_flags |= AUX_FREED)
#define	PKTCLRFAFREED(skb)	(((struct sk_buff*)(skb))->napt_flags &= (~AUX_FREED))
#define	PKTISFAFREED(skb)	(((struct sk_buff*)(skb))->napt_flags & AUX_FREED)
#define	PKTISFABRIDGED(skb)	PKTISFAAUX(skb)
#else
#define	PKTISFAAUX(skb)		({BCM_REFERENCE(skb); FALSE;})
#define	PKTISFABRIDGED(skb)	({BCM_REFERENCE(skb); FALSE;})
#define	PKTISFAFREED(skb)	({BCM_REFERENCE(skb); FALSE;})

#define	PKTCLRFAAUX(skb)	BCM_REFERENCE(skb)
#define PKTSETFAFREED(skb)	BCM_REFERENCE(skb)
#define	PKTCLRFAFREED(skb)	BCM_REFERENCE(skb)
#endif /* BCMFA */

#ifdef BCMDBG_CTRACE
#define PKTFRMNATIVE(osh, skb)  osl_pkt_frmnative(((osl_t *)osh), \
				(struct sk_buff*)(skb), __LINE__, __FILE__)
#define	PKTISFRMNATIVE(osh, skb) osl_pkt_is_frmnative((osl_t *)(osh), (struct sk_buff *)(skb))
#else
#define PKTFRMNATIVE(osh, skb)	osl_pkt_frmnative(((osl_t *)osh), (struct sk_buff*)(skb))
#endif /* BCMDBG_CTRACE */

#define PKTTONATIVE(osh, pkt)		osl_pkt_tonative((osl_t *)(osh), (pkt))
#define	PKTLINK(skb)			(((struct sk_buff*)(skb))->prev)
#define	PKTSETLINK(skb, x)		(((struct sk_buff*)(skb))->prev = (struct sk_buff*)(x))

#ifdef BCM_BLOG
extern uint osl_pktprio(void *skb);
extern void osl_pktsetprio(void *skb, uint x);
#define PKTPRIO(skb)                    osl_pktprio((skb))
#define PKTSETPRIO(skb, x)              osl_pktsetprio((skb), (x))
#define PKTBLOG(skb)			(((struct sk_buff *)(skb))->blog_p)
#else
#define	PKTPRIO(skb)			(((struct sk_buff*)(skb))->priority)
#define	PKTSETPRIO(skb, x)		(((struct sk_buff*)(skb))->priority = (x))
#endif /* BCM_BLOG */
#define PKTSUMNEEDED(skb)		(((struct sk_buff*)(skb))->ip_summed == CHECKSUM_HW)
#define PKTSETSUMGOOD(skb, x)		(((struct sk_buff*)(skb))->ip_summed = \
						((x) ? CHECKSUM_UNNECESSARY : CHECKSUM_NONE))
/* PKTSETSUMNEEDED and PKTSUMGOOD are not possible because skb->ip_summed is overloaded */
#define PKTSHARED(skb)                  (((struct sk_buff*)(skb))->cloned)

#ifdef CONFIG_NF_CONNTRACK_MARK
#define PKTMARK(p)                     (((struct sk_buff *)(p))->mark)
#define PKTSETMARK(p, m)               ((struct sk_buff *)(p))->mark = (m)
#else /* CONFIG_NF_CONNTRACK_MARK */
#define PKTMARK(p)                     0
#define PKTSETMARK(p, m)
#endif /* CONFIG_NF_CONNTRACK_MARK */
#define PKTISSPC(osh, skb)		FALSE

#endif /* ! BCM_NBUFF_PKT */

/* Account for packets entering or exiting a WLAN driver */
extern void osl_pkt_account(osl_t *osh, int skb_cnt, bool add);
#define PKTACCOUNT(osh, skb_cnt, add) \
	osl_pkt_account((osh), (skb_cnt), (add))

#if !defined(CMWIFI)
#define PKTDMAIDX(osh, skb)            ({BCM_REFERENCE(osh); \
	 (skbuff_bcm_ext_wlan_get((struct sk_buff*)(skb), dma_index));})
#define PKTSETDMAIDX(osh, skb, idx)    ({BCM_REFERENCE(osh); \
	 (skbuff_bcm_ext_wlan_get((struct sk_buff*)(skb), dma_index) = (idx));})
#endif /* !CMWIFI */

#if defined(BCM_OBJECT_TRACE)
extern void linux_pktfree(osl_t *osh, void *skb, bool docb, bool send,
	int line, const char *caller);
#ifdef BPM_BULK_FREE
extern void linux_pktfree_slow(osl_t *osh, void *skb, bool docb, bool send,
	int line, const char *caller);
#endif /* BPM_BULK_FREE */
#else
extern void linux_pktfree(osl_t *osh, void *skb, bool docb, bool send);
#ifdef BPM_BULK_FREE
extern void linux_pktfree_slow(osl_t *osh, void *skb, bool docb, bool send);
#endif /* BPM_BULK_FREE */
#endif /* BCM_OBJECT_TRACE */
extern void *osl_pktget_static(osl_t *osh, uint len);
extern void osl_pktfree_static(osl_t *osh, void *skb, bool send);
extern void osl_pktclone(osl_t *osh, void **pkt);

#ifdef BCMDBG_CTRACE
#define PKT_CTRACE_DUMP(osh, b)	osl_ctrace_dump((osh), (b))
extern void *linux_pktget(osl_t *osh, uint len, int line, char *file);
#if defined(CMWIFI) && defined(CMWIFI_EROUTER)
extern void *osl_pktget_data(osl_t *osh, uint len, int line, char *file);
#endif /* CMWIFI && CMWIFI_EROUTER */
extern void *osl_pkt_frmnative(osl_t *osh, void *skb, int line, char *file);
extern int osl_pkt_is_frmnative(osl_t *osh, struct sk_buff *pkt);
extern void *osl_pktdup(osl_t *osh, void *skb, int line, char *file);
extern void *osl_pktdup_cpy(osl_t *osh, void *skb, int line, char *file);
struct bcmstrbuf;
extern void osl_ctrace_dump(osl_t *osh, struct bcmstrbuf *b);
#else
#ifdef BCM_OBJECT_TRACE
extern void *linux_pktget(osl_t *osh, uint len, int line, const char *caller);
#if defined(CMWIFI) && defined(CMWIFI_EROUTER)
extern void *osl_pktget_data(osl_t *osh, uint len, int line, const char *caller);
#endif /* CMWIFI && CMWIFI_EROUTER */
extern void *osl_pktdup(osl_t *osh, void *skb, int line, const char *caller);
extern void *osl_pktdup_cpy(osl_t *osh, void *skb, int line, const char *caller);
#else
extern void *linux_pktget(osl_t *osh, uint len);
#if defined(CMWIFI) && defined(CMWIFI_EROUTER)
extern void *osl_pktget_data(osl_t *osh, uint len);
#endif /* CMWIFI && CMWIFI_EROUTER */
extern void *osl_pktdup(osl_t *osh, void *skb);
extern void *osl_pktdup_cpy(osl_t *osh, void *skb);
#endif /* BCM_OBJECT_TRACE */
extern void *osl_pkt_frmnative(osl_t *osh, void *skb);
#endif /* BCMDBG_CTRACE */
extern struct sk_buff *osl_pkt_tonative(osl_t *osh, void *pkt);

#ifdef BCM_SKB_FREE_OFFLOAD
#ifdef BPM_BULK_FREE
extern void dev_kfree_skb_thread_bulk(struct sk_buff *head, struct sk_buff *tail, uint32 len);
extern void linux_commit_skb_freelist(osl_t *osh);
#else
extern void dev_kfree_skb_thread_bulk(struct sk_buff *skb);
#endif /* BPM_BULK_FREE */
#endif /* BCM_SKB_FREE_OFFLOAD */

#else	/* ============== BINOSL ============= */

#define OSL_PREF_RANGE_LD(va, sz)
#define OSL_PREF_RANGE_ST(va, sz)

/* packet primitives */
#ifdef BCMDBG_CTRACE
#define	PKTGET(osh, len, send)		linux_pktget((osh), (len), __LINE__, __FILE__)
#define	PKTDUP(osh, skb)		osl_pktdup((osh), (skb), __LINE__, __FILE__)
#define	PKTDUP_CPY(osh, skb)		osl_pktdup_cpy((osh), (skb), __LINE__, __FILE__)
#define PKTFRMNATIVE(osh, skb)		osl_pkt_frmnative((osh), (skb), __LINE__, __FILE__)
#else
#ifdef BCM_OBJECT_TRACE
#define	PKTGET(osh, len, send)		linux_pktget((osh), (len), __LINE__, __FUNCTION__)
#define	PKTDUP(osh, skb)		osl_pktdup((osh), (skb), __LINE__, __FUNCTION__)
#define	PKTDUP_CPY(osh, skb)		osl_pktdup_cpy((osh), (skb), __LINE__, __FUNCTION__)
#else
#define	PKTGET(osh, len, send)		linux_pktget((osh), (len))
#define	PKTDUP(osh, skb)		osl_pktdup((osh), (skb))
#define	PKTDUP_CPY(osh, skb)		osl_pktdup_cpy((osh), (skb))
#endif /* BCM_OBJECT_TRACE */
#define PKTFRMNATIVE(osh, skb)		osl_pkt_frmnative((osh), (skb))
#endif /* BCMDBG_CTRACE */
#define PKTLIST_DUMP(osh, buf)		({BCM_REFERENCE(osh); BCM_REFERENCE(buf);})
#define PKTDBG_TRACE(osh, pkt, bit)	({BCM_REFERENCE(osh); BCM_REFERENCE(pkt);})
#if defined(BCM_OBJECT_TRACE)
#define	PKTFREE(osh, skb, send)		linux_pktfree((osh), (skb), TRUE, (send), \
						__LINE__, __FUNCTION__)
#else
#define	PKTFREE(osh, skb, send)		linux_pktfree((osh), (skb), TRUE, (send))
#endif /* BCM_OBJECT_TRACE */
#define	PKTDATA(osh, skb)		osl_pktdata((osh), (skb))
#define	PKTLEN(osh, skb)		osl_pktlen((osh), (skb))
#define PKTHEADROOM(osh, skb)		osl_pktheadroom((osh), (skb))
#define PKTTAILROOM(osh, skb)		osl_pkttailroom((osh), (skb))
#define	PKTNEXT(osh, skb)		osl_pktnext((osh), (skb))
#define	PKTSETNEXT(osh, skb, x)		({BCM_REFERENCE(osh); osl_pktsetnext((skb), (x));})
#define	PKTSETLEN(osh, skb, len)	osl_pktsetlen((osh), (skb), (len))
#define	PKTPUSH(osh, skb, bytes)	osl_pktpush((osh), (skb), (bytes))
#define	PKTPULL(osh, skb, bytes)	osl_pktpull((osh), (skb), (bytes))
#define PKTTAG(skb)			osl_pkttag((skb))
#define PKTTONATIVE(osh, pkt)		osl_pkt_tonative((osh), (pkt))
#define	PKTLINK(skb)			osl_pktlink((skb))
#define	PKTSETLINK(skb, x)		osl_pktsetlink((skb), (x))
#define	PKTPRIO(skb)			osl_pktprio((skb))
#define	PKTSETPRIO(skb, x)		osl_pktsetprio((skb), (x))
#define PKTSHARED(skb)                  osl_pktshared((skb))
#define PKTSETPOOL(osh, skb, x, y)	({BCM_REFERENCE(osh); BCM_REFERENCE(skb);})
#define	PKTPOOL(osh, skb)		({BCM_REFERENCE(osh); BCM_REFERENCE(skb); FALSE;})
#define PKTFREELIST(skb)		PKTLINK(skb)
#define PKTSETFREELIST(skb, x)		PKTSETLINK((skb), (x))
#define PKTPTR(skb)			(skb)
#define PKTID(skb)			({BCM_REFERENCE(skb); 0;})
#define PKTSETID(skb, id)		({BCM_REFERENCE(skb); BCM_REFERENCE(id);})
#define PKTDEV(skb)			osl_pktdev(skb)
#define PKTSETDEV(skb, _dev)		osl_pktsetdev(skb, _dev)

#ifdef BCM_OBJECT_TRACE
extern void *linux_pktget(osl_t *osh, uint len, int line, const char *caller);
#if defined(CMWIFI) && defined(CMWIFI_EROUTER)
extern void *osl_pktget_data(osl_t *osh, uint len, int line, const char *caller);
#endif /* CMWIFI && CMWIFI_EROUTER */
extern void *osl_pktdup(osl_t *osh, void *skb, int line, const char *caller);
extern void *osl_pktdup_cpy(osl_t *osh, void *skb, int line, const char *caller);
#else
extern void *linux_pktget(osl_t *osh, uint len);
#if defined(CMWIFI) && defined(CMWIFI_EROUTER)
extern void *osl_pktget_data(osl_t *osh, uint len);
#endif /* CMWIFI_EROUTER */
extern void *osl_pktdup(osl_t *osh, void *skb);
extern void *osl_pktdup_cpy(osl_t *osh, void *skb);
#endif /* BCM_OBJECT_TRACE */
extern void *osl_pkt_frmnative(osl_t *osh, void *skb);
#if defined(BCM_OBJECT_TRACE)
extern void linux_pktfree(osl_t *osh, void *skb, bool docb, bool send,
	int line, const char *caller);
#ifdef BPM_BULK_FREE
extern void linux_pktfree_slow(osl_t *osh, void *skb, bool docb, bool send,
	int line, const char *caller);
#endif /* BPM_BULK_FREE */
#else
extern void linux_pktfree(osl_t *osh, void *skb, bool docb, bool send);
#ifdef BPM_BULK_FREE
extern void linux_pktfree_slow(osl_t *osh, void *skb, bool docb, bool send);
#endif /* BPM_BULK_FREE */
#endif /* BCM_OBJECT_TRACE */
extern uchar *osl_pktdata(osl_t *osh, void *skb);
extern uint osl_pktlen(osl_t *osh, void *skb);
extern uint osl_pktheadroom(osl_t *osh, void *skb);
extern uint osl_pkttailroom(osl_t *osh, void *skb);
extern void *osl_pktnext(osl_t *osh, void *skb);
extern void osl_pktsetnext(void *skb, void *x);
extern void osl_pktsetlen(osl_t *osh, void *skb, uint len);
extern uchar *osl_pktpush(osl_t *osh, void *skb, int bytes);
extern uchar *osl_pktpull(osl_t *osh, void *skb, int bytes);
extern void *osl_pkttag(void *skb);
extern void *osl_pktlink(void *skb);
extern void osl_pktsetlink(void *skb, void *x);
extern uint osl_pktprio(void *skb);
extern void osl_pktsetprio(void *skb, uint x);
extern struct sk_buff *osl_pkt_tonative(osl_t *osh, void *pkt);
extern bool osl_pktshared(void *skb);

#endif	/* ============== BINOSL ============= */

/* defines for skb audit for WL */
#define OSL_SKB_CB			((struct sk_buff *)0)->cb
#if defined(PKTC) && !(defined(CMWIFI) && defined(CMWIFI_EROUTER))
#define OSL_SKB_PKTC_CB		(skbuff_bcm_ext_wlan_get(((struct sk_buff *)0), pktc_cb))
#define OSL_CHAIN_NODE_SZ	sizeof(struct chain_node)
#endif /* #if defined(PKTC) && !(defined(CMWIFI) && defined(CMWIFI_EROUTER)) */
#define OSL_SKB_CB_ALIGN_SZ	8 /* 8 byte alignment */
extern int osl_skb_audit(unsigned long pkttag_struct_size);

#define FOREACHPKT(skb, nskb, nskb1) \
for (nskb = (struct sk_buff *)skb; nskb; nskb = nskb->next) \
	for (nskb1 = nskb; nskb1 != NULL; nskb1 = PKTISCHAINED(nskb1) ? PKTCLINK(nskb1) : NULL)

#define PKTALLOCED(osh)		osl_pktalloced(osh)
extern uint osl_pktalloced(osl_t *osh);

/* Defines to update persistent packet allocations */
#define PKTALLOCED_PERSISTENTLY(osh)		osl_pktalloced_persistently(osh)
#define PKTALLOCED_PERSISTENTLY_INC(osh, cnt)	osl_pktalloced_persistently_inc(osh, cnt)
#define PKTALLOCED_PERSISTENTLY_DEC(osh, cnt)	osl_pktalloced_persistently_dec(osh, cnt)
extern uint osl_pktalloced_persistently(osl_t *osh);
extern void osl_pktalloced_persistently_inc(osl_t *osh, uint cnt);
extern void osl_pktalloced_persistently_dec(osl_t *osh, uint cnt);

#define PKTRESET(osh, skb, x)	osl_pkt_reset(osh, skb, x)
extern void osl_pkt_reset(osl_t *osh, void *p, int max_pkt_bytes);

#if !(defined(CMWIFI) && defined(CMWIFI_EROUTER) && !defined(PKTC_TAG_IN_SKB_CB))
#ifdef PKTC
#ifndef BCM_NBUFF_PKT

/* Use 8 or (if 64b pointer) 16 bytes of skb pktc_cb field to store below info */
struct chain_node {
	struct sk_buff	*link;
	unsigned int	flags:3, pkts:9, bytes:20;
#if defined(CONFIG_ARM64) && defined(PKTC_TAG_IN_SKB_CB)
	uint32		unused;
#endif /* CONFIG_ARM64 && PKTC_TAG_IN_SKB_CB */
};

#if defined(PKTC_TAG_IN_SKB_CB)
#define CHAIN_NODE(skb)((struct chain_node*)(&(((struct sk_buff*)skb)->cb[32])))
#else
#define CHAIN_NODE(skb)    \
	((struct chain_node*)(skbuff_bcm_ext_wlan_get((struct sk_buff*)skb, pktc_cb)))
#endif /* PKTC_TAG_IN_SKB_CB */

#define PKTCSETATTR(s, f, p, b) ({CHAIN_NODE(s)->flags = (f); CHAIN_NODE(s)->pkts = (p); \
		CHAIN_NODE(s)->bytes = (b);})
#define PKTCCLRATTR(s)      ({CHAIN_NODE(s)->flags = CHAIN_NODE(s)->pkts = \
		CHAIN_NODE(s)->bytes = 0;})
#define PKTCGETATTR(s)      (CHAIN_NODE(s)->flags << 29 | CHAIN_NODE(s)->pkts << 20 | \
		CHAIN_NODE(s)->bytes)
#define PKTCCNT(skb)        (CHAIN_NODE(skb)->pkts)
#define PKTCLEN(skb)        (CHAIN_NODE(skb)->bytes)
#define PKTCGETFLAGS(skb)   (CHAIN_NODE(skb)->flags)
#define PKTCSETFLAGS(skb, f)    (CHAIN_NODE(skb)->flags = (f))
#define PKTCCLRFLAGS(skb)   (CHAIN_NODE(skb)->flags = 0)
#define PKTCFLAGS(skb)      (CHAIN_NODE(skb)->flags)
#define PKTCSETCNT(skb, c)  (CHAIN_NODE(skb)->pkts = (c))
#define PKTCINCRCNT(skb)    (CHAIN_NODE(skb)->pkts++)
#define PKTCADDCNT(skb, c)  (CHAIN_NODE(skb)->pkts += (c))
#define PKTCSETLEN(skb, l)  (CHAIN_NODE(skb)->bytes = (l))
#define PKTCADDLEN(skb, l)  (CHAIN_NODE(skb)->bytes += (l))
#define PKTCSETFLAG(skb, fb)    (CHAIN_NODE(skb)->flags |= (fb))
#define PKTCCLRFLAG(skb, fb)    (CHAIN_NODE(skb)->flags &= ~(fb))
#define PKTCLINK(skb)       (CHAIN_NODE(skb)->link)
#define PKTSETCLINK(skb, x) (CHAIN_NODE(skb)->link = (struct sk_buff*)(x))
#define FOREACH_CHAINED_PKT(skb, nskb) \
for (; (skb) != NULL; (skb) = (nskb)) \
	if ((nskb) = (PKTISCHAINED(skb) ? PKTCLINK(skb) : NULL), \
		PKTSETCLINK((skb), NULL), 1)
#define PKTCFREE(osh, skb, send) \
do { \
	void *nskb; \
	ASSERT((skb) != NULL); \
	FOREACH_CHAINED_PKT((skb), nskb) { \
		PKTCLRCHAINED((osh), (skb)); \
		PKTCCLRFLAGS((skb)); \
		PKTFREE((osh), (skb), (send)); \
	} \
} while (0)
#define PKTCFREE_NOCB(osh, skb, send) \
do { \
	void *nskb; \
	ASSERT((skb) != NULL); \
	FOREACH_CHAINED_PKT((skb), nskb) { \
		PKTCLRCHAINED((osh), (skb)); \
		PKTCCLRFLAGS((skb)); \
		linux_pktfree((osh), (skb), FALSE, (send)); \
	} \
} while (0)
#define PKTCENQTAIL(h, t, p) \
do { \
	if ((t) == NULL) { \
		(h) = (t) = (p); \
	} else { \
		PKTSETCLINK((t), (p)); \
		(t) = (p); \
	} \
} while (0)

#endif /* !BCM_NBUFF_PKT */
#endif /* PKTC */
#endif /* !(CMWIFI && CMWIFI_EROUTER && !PKTC_TAG_IN_SKB_CB) */

/** For corerev >= 129, corrupted Rx frames (dma_rx()) should be dropped after linking
 * PhyRx Status buffer.
 * But PKTTAG flags can't be used across WLAN subsystems (hnddma and WL layer) so
 * repurposing the MSB of sk_buff::len to mark the packet as corrupted.
 */
#define RX_CORRUPTED	(1 << 31)

#ifdef BCM_SPC_RX
#define PKTSETRXCORRUPTED(osh, pnbuff) \
({ \
	ASSERT(PKTSPCRXDMAFMT(pnbuff) == FALSE); \
	STATIC_ASSERT(sizeof(((struct sk_buff *)(pnbuff))->len) >= sizeof(uint32)); \
	((struct sk_buff *)(pnbuff))->len |= RX_CORRUPTED; \
})
#define PKTRESETRXCORRUPTED(osh, pnbuff)  \
({ \
	ASSERT(PKTSPCRXDMAFMT(pnbuff) == FALSE); \
	(((struct sk_buff *)(pnbuff))->len &= (~RX_CORRUPTED)); \
})
#define PKTISRXCORRUPTED(osh, pnbuff) \
({ \
	(PKTSPCRXDMAFMT(pnbuff)) ? FALSE : \
	    (((struct sk_buff *)(pnbuff))->len & RX_CORRUPTED); \
})

#else /* !BCM_SPC_RX */
#define PKTSPCRXDMAFMT(pNBuff)	    FALSE
#define PKTSETRXCORRUPTED(osh, skb) \
({ \
	STATIC_ASSERT(sizeof(((struct sk_buff *)(skb))->len) >= sizeof(uint32)); \
	((struct sk_buff *)(skb))->len |= RX_CORRUPTED; \
})
#define PKTRESETRXCORRUPTED(osh, skb) (((struct sk_buff *)(skb))->len &= (~RX_CORRUPTED))
#define PKTISRXCORRUPTED(osh, skb) (((struct sk_buff *)(skb))->len & RX_CORRUPTED)
#endif /* !BCM_SPC_RX */

#ifndef PKTFREE_NEW_API
#define PKTFREE_OLDAPI(_fn)	_fn ## _oldapi
#if defined(BCM_OBJECT_TRACE)
static inline void PKTFREE_OLDAPI(linux_pktfree)(osl_t *osh, void *skb, bool send,
	int line, const char *caller)
{
	linux_pktfree(osh, skb, TRUE, send, line, caller);
}

#ifdef BPM_BULK_FREE
static inline void PKTFREE_OLDAPI(linux_pktfree_slow)(osl_t *osh, void *skb, bool send,
	int line, const char *caller)
{
	linux_pktfree_slow(osh, skb, TRUE, send, line, caller);
}
#endif /* BPM_BULK_FREE */
#else
static inline void PKTFREE_OLDAPI(linux_pktfree)(osl_t *osh, void *skb, bool send)
{
	linux_pktfree(osh, skb, TRUE, send);
}

#ifdef BPM_BULK_FREE
static inline void PKTFREE_OLDAPI(linux_pktfree_slow)(osl_t *osh, void *skb, bool send)
{
	linux_pktfree_slow(osh, skb, TRUE, send);
}
#endif /* BPM_BULK_FREE */
#endif /* BCM_OBJECT_TRACE */

#define linux_pktfree	PKTFREE_OLDAPI(linux_pktfree)
#endif /* PKTFREE_NEW_API */

#endif /* BCMDRIVER */

#endif	/* _linux_pkt_h_ */
