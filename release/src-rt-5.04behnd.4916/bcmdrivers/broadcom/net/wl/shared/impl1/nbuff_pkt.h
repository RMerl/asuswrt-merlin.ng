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

#ifndef _nbuff_pkt_h_
#define _nbuff_pkt_h_

#if defined(BCM_NBUFF_PKT)

#define PKTFREE_NEW_API

#include <typedefs.h>
#include <linux/bcm_skb_defines.h>
#include <linux/blog.h>
#include <linux/bcm_log_mod.h>
#if defined(BCM_WKB)
#include <linux/nbuff.h>
#endif /* BCM_WKB */
#if (defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE))
#include <linux/gbpm.h>
#endif

#define NO_OSH(osh, exp)            ({ BCM_REFERENCE(osh); exp; })

/**
 * ------------------------------------------------------------------------------------------------
 *  SECTION: Packet alloc/free and Packet Pool
 * ------------------------------------------------------------------------------------------------
 */
extern void *	nbuff_pktget_by_type(osl_t * osh, int len, bool send, int NBuffPtrType);
extern void *	nbuff_pktpoolget(osl_t * osh, int len);
extern void	nbuff_pktfree(osl_t * osh, void * pkt, bool docb, bool send);
extern void *	nbuff_pktdup(osl_t * osh, void * pkt);
extern void *	nbuff_pktdup_cpy(osl_t * osh, void * pkt);
extern void *	nbuff_pkt_tonative(osl_t * osh, void * pkt);
extern void *	nbuff_pkt_frmnative(osl_t * osh, void * pkt);

#define PKTGET_BY_TYPE(osh, len, send, type)	nbuff_pktget_by_type((osh), (len), (send), (type))
/* Get a packet buffer from BPM local pool cache */
#define PKTPOOLGET(osh, len)		nbuff_pktpoolget((osh), (len))
#if defined(BCM_SANITIZE)
#include <bcm_asan.h>
#define dasan_unregister_kb(pkt)	dasan_unregister((uptr)(pkt))
#if defined(BCM_WKB)
#define dasan_unregister_shinfo(pkt)
#else /* BCM_WKB */
#define dasan_unregister_shinfo(pkt)	dasan_unregister((uptr)skb_shinfo((struct sk_buff *)pkt))
#endif /* BCM_WKB */


#define PKTFREE(osh, pkt, send)	({ dasan_unregister_kb((pkt));  \
				dasan_unregister_shinfo(pkt);    \
				nbuff_pktfree((osh), (pkt), TRUE, (send)); })
#else /* BCM_SANITIZE */
#define PKTFREE(osh, pkt, send)		nbuff_pktfree((osh), (pkt), TRUE, (send))
#endif /* BCM_SANITIZE */
#define PKTFREE_NOCB(osh, pkt, send)	nbuff_pktfree((osh), (pkt), FALSE, (send))
#define PKTDUP(osh, pkt)		nbuff_pktdup((osh), (pkt))
#define PKTDUP_CPY(osh, pkt)		nbuff_pktdup_cpy((osh), (pkt))
#define PKTTONATIVE(osh, pkt)		nbuff_pkt_tonative((osh), (pkt))
#define PKTFRMNATIVE(osh, pkt)		nbuff_pkt_frmnative((osh), (pkt))

/**
 * ------------------------------------------------------------------------------------------------
 *  SECTION: NBUFF (SKB|WKB|FKB) Packet Primitives - PKTxyz
 * ------------------------------------------------------------------------------------------------
 */

#if defined(BCM_NBUFF_IS_WKB)
#define NBUFF_IS_WKBUFF			/* Use wkbuff_t directly */
#else /* ! BCM_NBUFF_IS_WKB */
/* DHD_SKB is defined when DHD is required to support handling of only SKBUFFs
 * and FKBUFFs are not supported.
 * HND should always support all NBUFF types
 */
#if (defined(BCMDONGLEHOST) && !defined(DHD_SKB)) || defined(HND_LINUX)
#define NBUFF_IS_ANY_BUFF		/* SKB | FKB | WKB */
#else /* ! NBUFF_IS_ANY_BUFF */
#define NBUFF_IS_SKBUFF			/* Use sk_buff directly */
#endif /* ! NBUFF_IS_ANY_BUFF */
#endif /* ! BCM_NBUFF_IS_WKB */


#if defined(NBUFF_IS_ANY_BUFF)
/**
 * ------------------------------------------------------------------------------------------------
 *  SECTION: PKT primitives used by modules (HND, DHD) which supportes
 *           different nbuff_types (SKB, FKB, WKB)
 * ------------------------------------------------------------------------------------------------
 */
extern uchar *	nbuff_pktdata(osl_t * osh, void * pkt);
extern void	nbuff_pktsetdata(osl_t * osh, void * pkt, uint8 * data);
extern void *	nbuff_pkthead(osl_t * osh, void * pkt);
extern uint	nbuff_pktlen(osl_t * osh, void * pkt);
extern uint	nbuff_pktheadroom(osl_t * osh, void * pkt);
extern bool	nbuff_pktexpheadroom(osl_t * osh, void * pkt, int headroom);
extern uint	nbuff_pkttailroom(osl_t * osh, void * pkt);
extern void	nbuff_pktsetnext(osl_t * osh, void * pkt, void * next);
extern void *	nbuff_pktnext(osl_t * osh, void * pkt);
extern uchar *	nbuff_pktpush(osl_t * osh, void * pkt, int bytes);
extern uchar *	nbuff_pktpull(osl_t * osh, void * pkt, int bytes);
extern void	nbuff_pktsetlen(osl_t * osh, void * pkt, uint len);
extern bool	nbuff_pktshared(void * pkt);
extern void *	nbuff_pktlink(void * pkt);
extern void	nbuff_pktsetlink(void * pkt, void * link);
extern uint	nbuff_pktprio(void * pkt);
extern void	nbuff_pktsetprio(void *pkt, uint prio);
extern void *	nbuff_pkt_get_tag(void * pkt);
extern struct blog_t *	nbuff_pktblog(void * pkt);
extern struct net_device * nbuff_pktdev(void * pkt);
extern void	nbuff_pktsetdev(void * pkt, struct net_device * dev);

#if defined(HND_LINUX) && defined(BCM_WKB)
/* XXX: BCM_WKB_TEST: TODO
 * HND module allocates SKB based databuffer and it might pass the SKB to WLAN NIC driver
 * So, review the path when WKB is enabled
 * Eg., BCMRX_PCN_PKTPOOL
 */
#define PKTGET(osh, len, send)	\
({ \
	printf("%s: ERROR: HND module allocating an SKB\n", __FUNCTION__); \
	ASSERT(0); \
	PKTGET_BY_TYPE(osh, len, send, SKBUFF_PTR); \
})
#else /* ! HND_LINUX */
#define PKTGET(osh, len, send)      PKTGET_BY_TYPE(osh, len, send, SKBUFF_PTR)
#endif /* ! HND_LINUX */
#define PKTDATA(osh, pkt)       nbuff_pktdata((osh), (pkt))
#define PKTSETDATA(osh, pkt, data)  nbuff_pktsetdata((osh), (pkt), (data))
#define PKTHEAD(osh, pkt)           nbuff_pkthead((osh), (pkt))
#define PKTLEN(osh, pkt)        nbuff_pktlen((osh), (pkt))
#define PKTHEADROOM(osh, pkt)   nbuff_pktheadroom((osh), (pkt))
#define PKTEXPHEADROOM(osh, pkt, headroom)      nbuff_pktexpheadroom((osh), (pkt), (headroom))
#define PKTTAILROOM(osh, pkt)   nbuff_pkttailroom((osh), (pkt))
#define PKTNEXT(osh, pkt)       nbuff_pktnext((osh), (pkt))
#define PKTSETNEXT(osh, pkt, x)     nbuff_pktsetnext((osh), (pkt), (x))
#define PKTPUSH(osh, pkt, bytes)    nbuff_pktpush((osh), (pkt), (bytes))
#define PKTPULL(osh, pkt, bytes)    nbuff_pktpull((osh), (pkt), (bytes))
#define PKTSETLEN(osh, pkt, len)    nbuff_pktsetlen((osh), (pkt), (len))
#define PKTTAG(pkt)             nbuff_pkt_get_tag((pkt))
#define PKTLINK(pkt)            nbuff_pktlink((pkt))
#define PKTSETLINK(pkt, x)      nbuff_pktsetlink((pkt), (x))
#define PKTPRIO(pkt)            nbuff_pktprio((pkt))
#define PKTSETPRIO(pkt, x)      nbuff_pktsetprio((pkt), (x))
#define PKTSHARED(pkt)          nbuff_pktshared((pkt))
#define PKTBLOG(pkt)            nbuff_pktblog(pkt)
#define PKTDEV(pkt)             nbuff_pktdev(pkt)
#define PKTSETDEV(pkt, x)       nbuff_pktsetdev(pkt, x)

#define PKTDIRTYP(osh, pkt)		\
	NO_OSH(osh, IS_SKBUFF_PTR(pkt) ? skb_shinfo(pkt)->dirty_p : NULL)
#define PKTTAINTED(osh, pkt)		\
	NO_OSH(osh, IS_SKBUFF_PTR(pkt) ? SKB_BPM_TAINTED(pkt) : BCM_REFERENCE(pkt))
#define PKTDATAPRISTINE(osh, pkt)	\
	NO_OSH(osh, IS_SKBUFF_PTR(pkt) ? SKB_DATA_PRISTINE(pkt) : BCM_REFERENCE(pkt))
#define PKTDATATAINTED(osh, pkt)	PKTTAINTED(osh, pkt)


#elif defined(NBUFF_IS_SKBUFF)
/**
 * ------------------------------------------------------------------------------------------------
 *  SECTION: PKT primitives used by WLAN modules which supports only SKB (NIC, EMF, IGS)
 * ------------------------------------------------------------------------------------------------
 */

#define PKTGET(osh, len, send)				PKTGET_BY_TYPE(osh, len, send, SKBUFF_PTR)

#define PKTDATA(osh, skb)				({ BCM_REFERENCE(osh); \
	(((struct sk_buff*)(skb))->data);		})

#define PKTLEN(osh, skb)				({ BCM_REFERENCE(osh); \
	(((struct sk_buff*)(skb))->len);		})

#define PKTHEADROOM(osh, skb)				({ BCM_REFERENCE(osh); \
	skb_headroom((struct sk_buff*)(skb));		})

#define PKTTAILROOM(osh, skb)				({ BCM_REFERENCE(osh); \
	skb_tailroom((struct sk_buff*)(skb));		})

#define PKTNEXT(osh, skb)				({ BCM_REFERENCE(osh); \
	(((struct sk_buff*)(skb))->next);		})

#define PKTSETNEXT(osh, skb, skb_next)			({ BCM_REFERENCE(osh); \
	(((struct sk_buff*)(skb))->next = (struct sk_buff*)(skb_next)); })

#define PKTPUSH(osh, skb, bytes)			({ BCM_REFERENCE(osh); \
	skb_push((struct sk_buff*)(skb), (bytes));	})

#define PKTPULL(osh, skb, bytes)			({ BCM_REFERENCE(osh); \
	skb_pull((struct sk_buff*)(skb), (bytes));	})

#define PKTSETLEN(osh, skb, len)			({ BCM_REFERENCE(osh); \
	__pskb_trim((struct sk_buff*)(skb), (len));	})

#define PKTTAG(skb) \
	((void *)(((struct sk_buff*)(skb))->cb))

#define PKTLINK(skb)							       \
	(((struct sk_buff*)(skb))->prev)

#define PKTSETLINK(skb, x)						       \
	(((struct sk_buff*)(skb))->prev = (struct sk_buff*)(x))

#define PKTPRIO(skb)					({		       \
	uint32 prio = ((struct sk_buff*)(skb))->mark >> PRIO_LOC_NFMARK & 0x7; \
	if (prio > 7) prio = 0;						       \
	prio;						})

#define PKTSETPRIO(skb, x)				({		       \
	((struct sk_buff*)(skb))->mark &= ~(0x7 << PRIO_LOC_NFMARK);	       \
	((struct sk_buff*)(skb))->mark |= ((x) & 0x7) << PRIO_LOC_NFMARK;      \
	((struct sk_buff*)(skb))->wl.ucast.nic.wl_prio = x;		       \
							})
#define PKTSHARED(skb)							       \
	(((struct sk_buff*)(skb))->cloned)

#define PKTDIRTYP(osh, skb)				({		       \
	BCM_REFERENCE(osh); skb_shinfo(skb)->dirty_p;	})

#define PKTTAINTED(osh, skb)				({		       \
	BCM_REFERENCE(osh); SKB_BPM_TAINTED(skb);	})

#define PKTDATAPRISTINE(osh, skb)			({		       \
	BCM_REFERENCE(osh); SKB_DATA_PRISTINE(skb);	})

#define PKTDATATAINTED(osh, skb)			PKTTAINTED((osh), (skb))

#define PKTBLOG(pkt)								\
	((struct sk_buff *)pkt)->blog_p

#define PKTDEV(pkt)								\
	((struct sk_buff *)pkt)->dev

#define PKTSETDEV(pkt, _dev)							\
	((struct sk_buff *)pkt)->dev = _dev

/*
 * qstat skb access macros are defined unconditionally so these macros
 * work unconditionally too. It is left up to the user to decide
 * when to use the macros using a build or runtime flag.
 */
#define PKT_HAS_QSTAT_TELEMETRY(p)						\
	skb_has_qstat_telemetry((struct sk_buff *)p)
#define PKT_DATA_HAS_QSTAT_TELEMETRY(p)						\
	skb_data_has_qstat_telemetry((struct sk_buff *) (p))
#define PKT_FRAG_HAS_QSTAT_TELEMETRY(p, i)					\
	skb_spc_frag_has_qstat_telemetry((struct sk_buff *) (p), (i))
#define PKT_QSTAT_TIMESTAMP_DATA(p)						\
	bcm_score_qstat_wlan_nic_set_egress_ts(((struct sk_buff *) p)->head)
#define PKT_QSTAT_TIMESTAMP_FRAG(p, i)						\
	bcm_score_qstat_wlan_nic_set_egress_ts(skb_shinfo(p)->frags[i].data -		\
			skb_frag_off(&skb_shinfo(p)->frags[i]))

#else /* NBUFF_IS_WKBUFF */
/**
 * ------------------------------------------------------------------------------------------------
 *  SECTION: PKT primitives used by WLAN modules supports only WKB
 * ------------------------------------------------------------------------------------------------
 */
extern bool	nbuff_pktshared(void * pkt);
extern struct blog_t * nbuff_pktblog(void * pkt);
extern struct net_device * nbuff_pktdev(void * pkt);
extern void	nbuff_pktsetdev(void * pkt, struct net_device * dev);

#define PKTGET(osh, len, send)		PKTGET_BY_TYPE(osh, len, send, WKBUFF_PTR)

#define PKTDATA(osh, pkt)		NO_OSH(osh, wkb_data(PNBUFF_2_WKBUFF(pkt)))

#define PKTSETDATA(osh, pkt, data)	NO_OSH(osh, wkb_set_data(PNBUFF_2_WKBUFF(pkt), data))

#define PKTHEAD(osh, pkt)		NO_OSH(osh, wkb_head(PNBUFF_2_WKBUFF(pkt)))

#define PKTLEN(osh, pkt)		NO_OSH(osh, wkb_len(PNBUFF_2_WKBUFF(pkt)))

#define PKTSETLEN(osh, pkt, len)	NO_OSH(osh, wkb_set_len(PNBUFF_2_WKBUFF(pkt), len))

#define PKTHEADROOM(osh, pkt)		NO_OSH(osh, wkb_headroom(PNBUFF_2_WKBUFF(pkt)))

#define PKTEXPHEADROOM(osh, pkt, headroom)	\
	NO_OSH(osh, wkb_realloc_headroom(PNBUFF_2_WKBUFF(pkt), headroom))

#define PKTTAILROOM(osh, pkt)		NO_OSH(osh, wkb_tailroom(PNBUFF_2_WKBUFF(pkt)))

#define PKTPUSH(osh, pkt, bytes)	NO_OSH(osh, wkb_push(PNBUFF_2_WKBUFF(pkt), bytes))

#define PKTPULL(osh, pkt, bytes)	NO_OSH(osh, wkb_pull(PNBUFF_2_WKBUFF(pkt), bytes))

#define PKTNEXT(osh, pkt)		NO_OSH(osh, wkb_next(PNBUFF_2_WKBUFF(pkt)))

#define PKTSETNEXT(osh, pkt, next)	NO_OSH(osh, wkb_set_next(PNBUFF_2_WKBUFF(pkt), next))

#define PKTLINK(pkt)			wkb_link(PNBUFF_2_WKBUFF(pkt))

#define PKTSETLINK(pkt, link)		wkb_set_link(PNBUFF_2_WKBUFF(pkt), link)

#define PKTPRIO(pkt)			wkb_prio(PNBUFF_2_WKBUFF(pkt))

#define PKTSETPRIO(pkt, prio)		wkb_set_prio(PNBUFF_2_WKBUFF(pkt), prio)

#define PKTTAG(pkt)			wkb_pkt_tag(PNBUFF_2_WKBUFF(pkt))

#define PKTSHARED(pkt)			nbuff_pktshared((pkt))
#define PKTBLOG(pkt)			nbuff_pktblog(pkt)
#define PKTDEV(pkt)			nbuff_pktdev(pkt)
#define PKTSETDEV(pkt, dev)		nbuff_pktsetdev(pkt, dev)

#define PKTDIRTYP(osh, pkt)		NO_OSH(osh, BCM_REFERENCE(pkt); (NULL))
#define PKTTAINTED(osh, pkt)		NO_OSH(osh, BCM_REFERENCE(pkt))
#define PKTDATAPRISTINE(osh, pkt)	NO_OSH(osh, BCM_REFERENCE(pkt))
#define PKTDATATAINTED(osh, pkt)	PKTTAINTED(osh, pkt)

/*
 * XXX: BCM_WKB_TEST: TODO: Define these MACROs with WKB
 * qstat WKB access macros are defined unconditionally so these macros
 * work unconditionally too. It is left up to the user to decide
 * when to use the macros using a build or runtime flag.
 */
#define PKT_HAS_QSTAT_TELEMETRY(p)			FALSE
#define PKT_DATA_HAS_QSTAT_TELEMETRY(p)			FALSE
#define PKT_FRAG_HAS_QSTAT_TELEMETRY(p, i)		FALSE
#define PKT_QSTAT_TIMESTAMP_DATA(p)			BCM_REFERENCE(p);
#define PKT_QSTAT_TIMESTAMP_FRAG(p, i)			BCM_REFERENCE(p);

#endif /* NBUFF_IS_WKBUFF */


/**
 * ------------------------------------------------------------------------------------------------
 *  SECTION: PKT primitives common to all modules (HND, DHD, NIC, EMF, IGS)
 *           but only supports SKB
 * ------------------------------------------------------------------------------------------------
 */

/* wlan_ext::wl_flag1 */
#define SKB_BLOG_TOHANDLE_PKT		(1 << 1)
#define SKB_WMF_HANDLED_PKT		(1 << 2)
#define SKB_BLOG_CLONED_PKT		(1 << 5)

extern bool nbuff_skb_wlan_ext_is_wl_flag(void *pkt, int flag);
extern void nbuff_skb_wlan_ext_set_wl_flag(void *pkt, int flag);
extern void nbuff_skb_wlan_ext_clr_wl_flag(void *pkt, int flag);

/**  IDENTIFY IF SKB need to be handled with blog_emit */
#define PKTISBLOG_TOHANDLE(pkt)		nbuff_skb_wlan_ext_is_wl_flag(pkt, SKB_BLOG_TOHANDLE_PKT)
#define PKTSETBLOG_TOHANDLE(pkt)	nbuff_skb_wlan_ext_set_wl_flag(pkt, SKB_BLOG_TOHANDLE_PKT)
#define PKTCLRBLOG_TOHANDLE(pkt)	nbuff_skb_wlan_ext_clr_wl_flag(pkt, SKB_BLOG_TOHANDLE_PKT)

/**  IDENTIFY IF SKB is WFM handled */
#define PKTISWMF_HANDLED(pkt)		nbuff_skb_wlan_ext_is_wl_flag(pkt, SKB_WMF_HANDLED_PKT)
#define PKTSETWMF_HANDLED(pkt)		nbuff_skb_wlan_ext_set_wl_flag(pkt, SKB_WMF_HANDLED_PKT)
#define PKTCLRWMF_HANDLED(pkt)		nbuff_skb_wlan_ext_clr_wl_flag(pkt, SKB_WMF_HANDLED_PKT)

/**  IDENTIFY IF SKB has blog cloned */
#define PKTISBLOG_CLONED(pkt)		nbuff_skb_wlan_ext_is_wl_flag(pkt, SKB_BLOG_CLONED_PKT)
#define PKTSETBLOG_CLONED(pkt)		nbuff_skb_wlan_ext_set_wl_flag(pkt, SKB_BLOG_CLONED_PKT)
#define PKTCLRBLOG_CLONED(pkt)		nbuff_skb_wlan_ext_clr_wl_flag(pkt, SKB_BLOG_CLONED_PKT);

#ifdef PKTC
#define CHAINED (1 << 3)
#define PKTSETCHAINED(osh, skb) \
	(IS_SKBUFF_PTR(skb) ? \
		(skbuff_bcm_ext_wlan_get((struct sk_buff*)(skb), pktc_flags) |= CHAINED) : \
		BCM_REFERENCE(skb))
#define PKTCLRCHAINED(osh, skb) \
	(IS_SKBUFF_PTR(skb) ? \
		(skbuff_bcm_ext_wlan_get((struct sk_buff*)(skb), pktc_flags) &= (~CHAINED)) : \
		BCM_REFERENCE(skb))
#define PKTISCHAINED(skb) \
	(IS_SKBUFF_PTR(skb) ? \
		(skbuff_bcm_ext_wlan_get((struct sk_buff*)(skb), pktc_flags) & CHAINED) : \
		FALSE)

/* Use 8 bytes of skb tstamp field to store below info */
struct chain_node {
	struct sk_buff *link;
	unsigned int flags:3, pkts:9, bytes:20;
};

#define CHAIN_NODE(skb) \
	((struct chain_node*)(skbuff_bcm_ext_wlan_get((struct sk_buff*)skb, pktc_cb)))
#define PKTCSETATTR(skb, f, p, b) \
	(IS_SKBUFF_PTR(skb) ? ({CHAIN_NODE(skb)->flags = (f); CHAIN_NODE(skb)->pkts = (p); \
		CHAIN_NODE(skb)->bytes = (b);}) : BCM_REFERENCE(skb))
#define PKTCCLRATTR(skb) \
	(IS_SKBUFF_PTR(skb) ? ({CHAIN_NODE(skb)->flags = CHAIN_NODE(skb)->pkts = \
		CHAIN_NODE(skb)->bytes = 0;}) : BCM_REFERENCE(skb))
#define PKTCGETATTR(skb) \
	(IS_SKBUFF_PTR(skb) ? (CHAIN_NODE(skb)->flags << 29 | CHAIN_NODE(skb)->pkts << 20 | \
		CHAIN_NODE(skb)->bytes) : ({BCM_REFERENCE(skb); 0;}))
#define PKTCCNT(skb)		(IS_SKBUFF_PTR(skb) ? (CHAIN_NODE(skb)->pkts) : 1)
#define PKTCLEN(skb)		(IS_SKBUFF_PTR(skb) ? (CHAIN_NODE(skb)->bytes) : PKTLEN(skb))
#define PKTCGETFLAGS(skb) \
	(IS_SKBUFF_PTR(skb) ? (CHAIN_NODE(skb)->flags) : ({BCM_REFERENCE(skb); 0;}))
#define PKTCSETFLAGS(skb, f) \
	(IS_SKBUFF_PTR(skb) ? (CHAIN_NODE(skb)->flags = (f)) : BCM_REFERENCE(skb))
#define PKTCCLRFLAGS(skb) \
	(IS_SKBUFF_PTR(skb) ? (CHAIN_NODE(skb)->flags = 0) : BCM_REFERENCE(skb))
#define PKTCFLAGS(skb) \
	(IS_SKBUFF_PTR(skb) ? (CHAIN_NODE(skb)->flags) : ({BCM_REFERENCE(skb); 0;}))
#define PKTCSETCNT(skb, c) \
	(IS_SKBUFF_PTR(skb) ? (CHAIN_NODE(skb)->pkts = (c)) : BCM_REFERENCE(skb))
#define PKTCINCRCNT(skb) \
	(IS_SKBUFF_PTR(skb) ? (CHAIN_NODE(skb)->pkts++) : BCM_REFERENCE(skb))
#define PKTCADDCNT(skb, c) \
	(IS_SKBUFF_PTR(skb) ? (CHAIN_NODE(skb)->pkts += (c)) : BCM_REFERENCE(skb))
#define PKTCSETLEN(skb, l) \
	(IS_SKBUFF_PTR(skb) ? (CHAIN_NODE(skb)->bytes = (l)) : BCM_REFERENCE(skb))
#define PKTCADDLEN(skb, l) \
	(IS_SKBUFF_PTR(skb) ? (CHAIN_NODE(skb)->bytes += (l)) : BCM_REFERENCE(skb))
#define PKTCSETFLAG(skb, fb) \
	(IS_SKBUFF_PTR(skb) ? (CHAIN_NODE(skb)->flags |= (fb)) : BCM_REFERENCE(skb))
#define PKTCCLRFLAG(skb, fb) \
	(IS_SKBUFF_PTR(skb) ? (CHAIN_NODE(skb)->flags &= ~(fb)) : BCM_REFERENCE(skb))
#define PKTCLINK(skb)		(IS_SKBUFF_PTR(skb) ? (CHAIN_NODE(skb)->link) : NULL)
#define PKTSETCLINK(skb, x) \
	(IS_SKBUFF_PTR(skb) ? (CHAIN_NODE(skb)->link = (struct sk_buff*)(x)) : BCM_REFERENCE(skb))
#define FOREACH_CHAINED_PKT(skb, nskb) \
	for (; (skb) != NULL; (skb) = (nskb)) \
		if ((nskb) = (PKTISCHAINED(skb) ? PKTCLINK(skb) : NULL), \
			PKTSETCLINK((skb), NULL), 1)

#define PKTCFREE(osh, skb, send) \
	(IS_SKBUFF_PTR(skb) ? \
		({do { \
			void *nskb; \
			ASSERT((skb) != NULL); \
			FOREACH_CHAINED_PKT((skb), nskb) { \
				PKTCLRCHAINED((osh), (skb)); \
				PKTCCLRFLAGS((skb)); \
				PKTFREE((osh), (skb), (send)); \
			} \
		} while (0); }) : \
		PKTFREE(osh, skb, send))
#define PKTCFREE_NOCB(osh, skb, send) \
	(IS_SKBUFF_PTR(skb) ? \
		({do { \
			void *nskb; \
			ASSERT((skb) != NULL); \
			FOREACH_CHAINED_PKT((skb), nskb) { \
				PKTCLRCHAINED((osh), (skb)); \
				PKTCCLRFLAGS((skb)); \
				PKTFREE_NOCB((osh), (skb), (send)); \
			} \
		} while (0); }) : \
		PKTFREE_NOCB(osh, skb, send))

#define PKTCENQTAIL(h, t, p) \
	(IS_SKBUFF_PTR(p) ? \
		({do { \
			if ((t) == NULL) { \
				(h) = (t) = (p); \
			} else { \
				PKTSETCLINK((t), (p)); \
				(t) = (p); \
			} \
		} while (0); }) : BCM_REFERENCE(p))

#endif /* PKTC */

#if !defined(BCM_WKB)
#if defined(WLCFP)

/** Cache Flow Processing Packet Macros */
/* wlan_ext::pktc_flags */
#define SKB_CFP_PKT     (1 << 6)

#define PKTISCFP(pkt)   (skbuff_bcm_ext_wlan_get((struct sk_buff*)(pkt), pktc_flags) & SKB_CFP_PKT)
#define PKTSETCFP(pkt) \
	({ skbuff_bcm_ext_wlan_get((struct sk_buff*)(pkt), pktc_flags) |= SKB_CFP_PKT; })
#define PKTCLRCFP(pkt) \
	({ skbuff_bcm_ext_wlan_get((struct sk_buff*)(pkt), pktc_flags) &= ~SKB_CFP_PKT; })

/* wlan_ext::wl_flowid */
#define PKTGETCFPFLOWID(pkt) \
	({  skbuff_bcm_ext_wlan_get((struct sk_buff*)(pkt), wl_flowid); })
#define PKTSETCFPFLOWID(pkt, cfp_flowid) \
	({  PKTSETCFP(pkt); skbuff_bcm_ext_wlan_get((struct sk_buff*)(pkt), wl_flowid) = \
	(cfp_flowid); })
#define PKTCLRCFPFLOWID(pkt, cfp_flowid) \
	({  PKTCLRCFP(pkt); skbuff_bcm_ext_wlan_get((struct sk_buff*)(pkt), wl_flowid) = \
	(cfp_flowid); })

#endif /* WLCFP */

#else /* BCM_WKB */

/**
 * ------------------------------------------------------------------------------------------------
 *  SECTION: PKT primitives common to all modules (HND, DHD, NIC, EMF, IGS)
 *           but only supports WKB
 * ------------------------------------------------------------------------------------------------
 */
/* Largest reasonable packet buffer used by device driver for WLAN/Ethernet MTU in bytes */
#define PKTBUFSZ			BCM_PKT_PYLD_BYTES	/* RxFIFO 0: 802.3 */
#define PKTBUFSZ_HDR			BCM_PKT_HEADROOM	/* RxFIFO 1: 802.11 HDR */

extern void *	nbuff_attach_wkb(osl_t * osh, void * pdata, int data_offset,
	uint16 len, bool is_phead);
extern void	nbuff_wkb_frag_idx_fill_item(osl_t * osh, void * pkt, void * frag_data,
	uint16 frag_idx, int data_offset, uint16 frag_len);
extern int	nbuff_wkb_frag_fill_item(osl_t * osh, void * pkt, void * frag);
extern void	nbuff_wkb_data_fixup(osl_t * osh, void *pkt);
extern void *	nbuff_xlate_to_wkb(osl_t * osh, void * pkt);
extern void *	nbuff_xlate_to_skb(osl_t * osh, void * pkt);
extern void *	nbuff_wkb_get_pkt_ref(osl_t * osh, void * pkt);
extern void *	nbuff_wkb_release_pkt_ref(osl_t * osh, void * pkt);
extern int	nbuff_wkb_frags_release(osl_t * osh, void * pkt);

/* With WKB, AMSDU SF"0" wil be filled into frag "0" but WLAN SPC uses frag "0" for AMSDU SF "1"
 * so translate fragcnt and frag_idx accordingly
 * WLAN SPC frag_idx = WKB frag_idx - 1
 * WLAN SPC frag_num = WKB frag_num - 1
 *
 * LB_FRAG1 is used to get 802.3 data of AMSDU Subframe "0" when HWHDR conv is enabled
 */
#define LB_FRAG1					(-1)
#define WLIDX2WKBIDX(_frag_idx)				(_frag_idx + 1)
#define WL_WKB_LNX_FRAG_IDX				(WKB_LNX_FRAG_IDX - 1)
#define WL_WKB_FRAGCNT_MAX				(WKB_FRAGCNT_MAX - 1)

#define PKTFRAGHEAD(osh, pkt, idx)			\
	NO_OSH(osh, wkb_frag_db_addr(PNBUFF_2_WKBUFF(pkt), WLIDX2WKBIDX(idx)) + WKB_DB_HDRM_OFFSET)

#define PKTFRAGDATA(osh, pkt, idx)			\
	NO_OSH(osh, wkb_frag_data(PNBUFF_2_WKBUFF(pkt), WLIDX2WKBIDX(idx)))

#define PKTSETFRAGDATA(osh, pkt, idx, data)		\
	NO_OSH(osh, wkb_set_frag_data(PNBUFF_2_WKBUFF(pkt), WLIDX2WKBIDX(idx), data))

#define PKTFRAGLEN(osh, pkt, idx)			\
	NO_OSH(osh, wkb_frag_len(PNBUFF_2_WKBUFF(pkt), WLIDX2WKBIDX(idx)))

#define PKTSETFRAGLEN(osh, pkt, idx, len)		\
	NO_OSH(osh, wkb_set_frag_len(PNBUFF_2_WKBUFF(pkt), WLIDX2WKBIDX(idx), len))

#define PKTFRAGPUSH(osh, pkt, idx, bytes)		\
	NO_OSH(osh, wkb_frag_push(PNBUFF_2_WKBUFF(pkt), WLIDX2WKBIDX(idx), bytes))

#define PKTFRAGPULL(osh, pkt, idx, bytes)		\
	NO_OSH(osh, wkb_frag_pull(PNBUFF_2_WKBUFF(pkt), WLIDX2WKBIDX(idx), bytes))

#define PKTFRAGTOTLEN(osh, pkt)				\
	NO_OSH(osh, PNBUFF_2_WKBUFF(pkt)->frag_totlen)

#define PKTFRAGCNT(osh, pkt)				\
	NO_OSH(osh, (PNBUFF_2_WKBUFF(pkt)->frag_num - 1))

#define PKTFRAGTOTNUM(osh, pkt)				PKTFRAGCNT(osh, pkt)
#define PKTSEGCNT(osh, pkt)				(PKTFRAGCNT(osh, pkt) + 1)

#define PKTFRAGUSEDLEN(osh, pkt) 			PKTFRAGLEN(osh, pkt, LB_FRAG1)
#define PKTSETFRAGUSEDLEN(osh, pkt, len)		PKTSETFRAGLEN(osh, pkt, LB_FRAG1, len)

/**
 * SECTION: WLAN PKTTAG
 */

/* WKB equivalent of sk_buff::bcm_skb_ext::wlan_ext */
#define NBUFF_WLAN_EXT_BYTES		8
#define NBUFF_PKTTAG_BYTES		(WKB_EXTNWL_BYTES - NBUFF_WLAN_EXT_BYTES)

/* WLAN (DHD/WL) PKTTTAG; WKB equivalent of sk_buff::cb[]  */
#define DHD_PKTTAG_BYTES		NBUFF_PKTTAG_BYTES
#define WL_PKTTAG_BYTES			NBUFF_PKTTAG_BYTES

typedef union nbuff_pkttag {
	uint32	u32[WKB_EXTNWL_BYTES];	/* 48 Bytes */

	struct {
		/* WKB equivalent of sk_buff::cb[] */
		union {	/* 40 Bytes */
			/** Private to DHD layer - dhd_pkttag_fd_t */
			uint8	dhd_pkttag[DHD_PKTTAG_BYTES];
			/* Private to WLC Layer - wlc_pkttag_t */
			uint8	wl_pkttag[WL_PKTTAG_BYTES];
		};
		/* Common to all modules HNDDMA, PKTFWD, AWL, BLOG, WLC, DHD.
		 * WKB equivalent of sk_buff::bcm_skb_ext::wlan_ext
		 */
		struct { /* NBUFF_WLAN_EXT_BYTES: 8 Bytes */
			uint16	wl_flowid;
			uint16	dma_index;
			uint16	flags;
			uint8	PAD[2];
		};
	};
} nbuff_pkttag_t;

/* nbuff_pkkttag_t::flags Used only by NIC (wl.ko) driver */
#define NBUFF_WKB_SPC			(1 << 0)
#define NBUFF_WKB_CFP			(1 << 1)
#define NBUFF_WKB_RX_CORRUPTED		(1 << 2)
/* RxPath - SplitRxMode */
#define NBUFF_WKB_RX_FRAG 		(1 << 3)
#define NBUFF_WKB_HDR_CONVERTED 	(1 << 4)

/* XXX: BCM_WKB_TEST: Placed ASSERT for WKB validation.
 * TODO: Remove the assert once WKB is stabilized
 */

#define PKTISSPC(osh, pkt)				NO_OSH(osh, \
	ASSERT(IS_WKBUFF_PTR(pkt));			\
	((nbuff_pkttag_t *)PKTTAG(pkt))->flags & NBUFF_WKB_SPC)
#define PKTSETSPC(osh, pkt)				NO_OSH(osh, \
	ASSERT(IS_WKBUFF_PTR(pkt));			\
	((nbuff_pkttag_t *)PKTTAG(pkt))->flags |= NBUFF_WKB_SPC)
#define PKTCLRSPC(osh, pkt)				NO_OSH(osh, \
	ASSERT(IS_WKBUFF_PTR(pkt));			\
	((nbuff_pkttag_t *)PKTTAG(pkt))->flags &= ~NBUFF_WKB_SPC)

#define PKTISCFP(pkt)					({ \
	ASSERT(IS_WKBUFF_PTR(pkt));			\
	((nbuff_pkttag_t *)PKTTAG(pkt))->flags & NBUFF_WKB_CFP; })
#define PKTSETCFP(pkt)					({ \
	ASSERT(IS_WKBUFF_PTR(pkt));			\
	((nbuff_pkttag_t *)PKTTAG(pkt))->flags |= NBUFF_WKB_CFP; })
#define PKTCLRCFP(pkt)					({ \
	ASSERT(IS_WKBUFF_PTR(pkt));			\
	((nbuff_pkttag_t *)PKTTAG(pkt))->flags &= ~NBUFF_WKB_CFP; })

#define PKTISRXCORRUPTED(osh, pkt)			NO_OSH(osh, \
	IS_WKBUFF_PTR(pkt) ?						\
	((nbuff_pkttag_t *)PKTTAG(pkt))->flags & NBUFF_WKB_RX_CORRUPTED : FALSE)
#define PKTSETRXCORRUPTED(osh, pkt)			NO_OSH(osh, \
	ASSERT(IS_WKBUFF_PTR(pkt));			\
	((nbuff_pkttag_t *)PKTTAG(pkt))->flags |= NBUFF_WKB_RX_CORRUPTED)
#define PKTCLRRXCORRUPTED(osh, pkt)			NO_OSH(osh, \
	ASSERT(IS_WKBUFF_PTR(pkt));			\
	((nbuff_pkttag_t *)PKTTAG(pkt))->flags &= ~NBUFF_WKB_RX_CORRUPTED)

#define PKTISRXFRAG(osh, pkt)				NO_OSH(osh, \
	ASSERT(IS_WKBUFF_PTR(pkt));			\
	((nbuff_pkttag_t *)PKTTAG(pkt))->flags & NBUFF_WKB_RX_FRAG)
#define PKTSETRXFRAG(osh, pkt)				NO_OSH(osh, \
	ASSERT(IS_WKBUFF_PTR(pkt));			\
	((nbuff_pkttag_t *)PKTTAG(pkt))->flags |= NBUFF_WKB_RX_FRAG)
#define PKTCLRRXFRAG(osh, pkt)				NO_OSH(osh, \
	ASSERT(IS_WKBUFF_PTR(pkt));			\
	((nbuff_pkttag_t *)PKTTAG(pkt))->flags &= ~NBUFF_WKB_RX_FRAG)

#define PKTISFRAG(osh, pkt)				PKTISRXFRAG(osh, pkt)

#define PKTISHDRCONVTD(osh, pkt)			NO_OSH(osh, \
	ASSERT(IS_WKBUFF_PTR(pkt));			\
	((nbuff_pkttag_t *)PKTTAG(pkt))->flags & NBUFF_WKB_HDR_CONVERTED)
#define PKTSETHDRCONVTD(osh, pkt)			NO_OSH(osh, \
	ASSERT(IS_WKBUFF_PTR(pkt));			\
	((nbuff_pkttag_t *)PKTTAG(pkt))->flags |= NBUFF_WKB_HDR_CONVERTED)
#define PKTCLRHDRCONVTD(osh, pkt)			NO_OSH(osh, \
	ASSERT(IS_WKBUFF_PTR(pkt));			\
	((nbuff_pkttag_t *)PKTTAG(pkt))->flags &= ~NBUFF_WKB_HDR_CONVERTED)

#define PKTGETCFPFLOWID(pkt)				({ \
	ASSERT(IS_WKBUFF_PTR(pkt));			\
	((nbuff_pkttag_t *)PKTTAG(pkt))->cfp_flowid; })
#define PKTSETCFPFLOWID(pkt, cfp_flowid)		({ \
	ASSERT(IS_WKBUFF_PTR(pkt));			\
	PKTSETCFP(pkt); ((nbuff_pkttag_t *)PKTTAG(pkt))->wl_flowid = (cfp_flowid); })
#define PKTCLRCFPFLOWID(pkt, cfp_flowid)		({ \
	ASSERT(IS_WKBUFF_PTR(pkt));			\
	PKTCLRCFP(pkt); ((nbuff_pkttag_t *)PKTTAG(pkt))->wl_flowid = (cfp_flowid); })

/* wlan_ext::dma_index */
#define PKTDMAIDX(osh, pkt)				NO_OSH(osh, \
	ASSERT(IS_WKBUFF_PTR(pkt));			\
	((nbuff_pkttag_t *)PKTTAG(pkt))->dma_index)
#define PKTSETDMAIDX(osh, pkt, idx)			NO_OSH(osh, \
	ASSERT(IS_WKBUFF_PTR(pkt));			\
	((nbuff_pkttag_t *)PKTTAG(pkt))->dma_index = (idx))

#endif /* BCM_WKB */

/** MACROS COMMON TO SKB, WKB and FKB */

/**  IDENTIFY IF PKT is to be forward back to BSS */
#define PKTISINTRABSS_FWD(pkt)		FALSE
#define PKTSETINTRABSS_FWD(pkt)		BCM_REFERENCE(pkt);
#define PKTCLRINTRABSS_FWD(pkt)		BCM_REFERENCE(pkt);

#ifdef BCMDBG_CTRACE
#define PKTCALLER(zskb)			({ BCM_REFERENCE(zskb); })
#endif /* BCMDBG_CTRACE */

#define PKTSETFWDERBUF(osh, pkt)	NO_OSH(osh, BCM_REFERENCE(pkt))
#define PKTCLRFWDERBUF(osh, pkt)	NO_OSH(osh, BCM_REFERENCE(pkt))
#define PKTISFWDERBUF(osh, pkt)		NO_OSH(osh, BCM_REFERENCE(pkt); FALSE)
#define PKTORPHAN(pkt)			BCM_REFERENCE(pkt)
#define PKTSETPOOL(osh, pkt, x, y)	BCM_REFERENCE(osh)
#define PKTPOOL(osh, pkt)		NO_OSH(osh, BCM_REFERENCE(pkt); FALSE)
#ifndef PKTLIST_DUMP
#define PKTLIST_DUMP(osh, buf)		BCM_REFERENCE(osh)
#endif /* ! PKTLIST_DUMP */

#define PKTFREELIST(pkt)		PKTLINK(pkt)
#define PKTSETFREELIST(pkt, x)		PKTSETLINK((pkt), (x))

/**
 * ------------------------------------------------------------------------------------------------
 *  SECTION: XPM - BPM/MPM based Packet Pool
 * ------------------------------------------------------------------------------------------------
 */
#ifdef PP_XPM
extern int nbuff_pp_xpm_init(osl_t *osh, bool *enab);
extern int nbuff_pp_xpm_deinit(osl_t *osh);
extern struct sk_buff *nbuff_pp_xpm_skb_get(osl_t *osh);
extern void nbuff_pp_xpm_skb_free(osl_t *osh, struct sk_buff *skb);
extern void *nbuff_pp_xpm_databuf_get(osl_t *osh, int len);
extern void nbuff_pp_xpm_databuf_free(osl_t *osh, void *pdata);
extern void nbuff_pp_xpm_spc_databuf_free(osl_t *osh, void *skb, void *pdata);
extern int nbuff_pp_xpm_attach_skb(osl_t *osh, struct sk_buff *skb, void *pdata,
	int32 data_offset, uint32 len);

#define PP_XPM_INIT(osh, enab)			nbuff_pp_xpm_init((osh), (enab))
#define PP_XPM_DEINIT(osh)			nbuff_pp_xpm_deinit((osh))
#define PP_XPM_SKB_GET(osh)			nbuff_pp_xpm_skb_get((osh))
#define PP_XPM_SKB_FREE(osh, skb)		nbuff_pp_xpm_skb_free((osh), (skb))
#define PP_XPM_DATABUF_GET(osh, len)		nbuff_pp_xpm_databuf_get((osh), (len))
#define PP_XPM_DATABUF_FREE(osh, pdata) 	nbuff_pp_xpm_databuf_free((osh), (pdata))
#define PP_XPM_SPC_DATABUF_FREE(osh, skb, pdata)	\
	nbuff_pp_xpm_spc_databuf_free((osh), (skb), (pdata))
#define PP_XPM_ATTACH_SKB(osh, skb, pdata, h, l)	\
	nbuff_pp_xpm_attach_skb((osh), (skb), (pdata), (h), (l))

/**
 * ------------------------------------------------------------------------------------------------
 *  SECTION: SPC - Single Packet Context Helper ADTs
 * ------------------------------------------------------------------------------------------------
 */
#if defined(BCM_SPC_RX) || defined(BCM_WKB)

/* Tiny structure used at HNDDMA to facilitate PKTPUSH/PULL
 * and BULK transfer when SPC is enabled.
 * Lifetime of this structure is until we form the SPC formatted
 * AMSDU frames (CFP Path) or convert to SKB/WKB (slow path).
 */
typedef struct xk_buff {
	void 	*link;		/* for PKTLINK */
	uint8	*data;		/* Equivalent to PKTDATA */
	uint16 	offset;		/* Offset between pHead and data */
	uint16 	len;		/* Equivalent to PKTLEN */
} xk_buff_t;

/* Reusing TGBUFF_PTR for XKB since WLAN driver does not use this type */
#define XKBUFF_TYPE				TGBUFF_PTR
#define XKBUFF_MASK				(~XKBUFF_TYPE)
#define IS_XKBUFF_PTR(p)			(((uintptr_t)(p) & XKBUFF_TYPE) == XKBUFF_TYPE)
#define XKBUFF_2_PNBUFF(pNBuff)			\
	((void*) ((uintptr_t)(pNBuff) | (uintptr_t)(XKBUFF_TYPE)))
#define PNBUFF_2_XKBUFF(pNBuff)			\
	((xk_buff_t*) ((uintptr_t)(pNBuff) & (uintptr_t)(XKBUFF_MASK)))

#define PKTSPCRXDMAFMT(p)			IS_XKBUFF_PTR((p))
#define PKTSPCRXLINK(p)				(((xk_buff_t *)PNBUFF_2_XKBUFF(p))->link)
#define PKTSPCRXSETLINK(p, x) 			(((xk_buff_t *)PNBUFF_2_XKBUFF(p))->link = x)

#define PKTSPCRXDATA(osh, p)			\
	({ BCM_REFERENCE(osh); ((xk_buff_t *)PNBUFF_2_XKBUFF(p))->data; })
#define PKTSPCRXLEN(osh, p)			\
	({ BCM_REFERENCE(osh); ((xk_buff_t *)PNBUFF_2_XKBUFF(p))->len; })
#define PKTSPCRXSETLEN(osh, p, _len)		\
	({ BCM_REFERENCE(osh); ((xk_buff_t *)PNBUFF_2_XKBUFF(p))->len = _len; })
#define PKTSPCRXOFFSET(osh, p)			\
	({ BCM_REFERENCE(osh); ((xk_buff_t *)PNBUFF_2_XKBUFF(p))->offset; })
#define PKTSPCRXSETOFFSET(osh, p, _offset)	\
	({ BCM_REFERENCE(osh); ((xk_buff_t *)PNBUFF_2_XKBUFF(p))->offset = _offset; })

static inline void *
xkb_push(void *p, int len)
{
	xk_buff_t *xkb = (xk_buff_t *)PNBUFF_2_XKBUFF(p);
	xkb->len += len;
	xkb->offset -= len;
	xkb->data -= len;
	return xkb->data;
}

static inline void *
xkb_pull(void *p, int len)
{
	xk_buff_t *xkb = (xk_buff_t *)PNBUFF_2_XKBUFF(p);
	xkb->len -= len;
	xkb->offset += len;
	xkb->data += len;
	return xkb->data;
}

#define PKTSPCRXPUSH(osh, p, len)		\
	({ BCM_REFERENCE(osh); xkb_push(p, len); })
#define PKTSPCRXPULL(osh, pNBuff, len)		\
	({ BCM_REFERENCE(osh); xkb_pull(p, len); })

extern void *	nbuff_pp_xpm_attach_xkb(osl_t *osh, void *pdata, int data_offset, uint32 len);
extern void	nbuff_pp_xpm_release_xkb_cache(osl_t *osh);
extern void *	nbuff_pp_xpm_xkb_xlate(osl_t *osh, void *p);

/* Initializes xkb structure for databuf */
#define PP_XPM_ATTACH_XKB(osh, pdata, h, l)	\
	nbuff_pp_xpm_attach_xkb((osh), (pdata), (h), (l))
#define PP_XPM_RELEASE_XKB_CACHE(osh)		nbuff_pp_xpm_release_xkb_cache((osh))
/* For single xkb -> skb translate */
#define PKTSPCRXXLATE(osh, p)			nbuff_pp_xpm_xkb_xlate((osh), (p))
#endif /* BCM_SPC_RX || BCM_WKB */
#endif /* PP_XPM */

#endif /* BCM_NBUFF_PKT */

#endif /* _nbuff_pkt_h_ */
