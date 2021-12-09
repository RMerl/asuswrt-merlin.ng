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

#ifndef __DHD_NBUFF_H__
#define __DHD_NBUFF_H__
#if defined(BCM_NBUFF)
#ifdef mips
#undef ABS
#endif

#include <linux/nbuff.h>
#include <dhd_fkbpool.h>

#define IS_CLONED_FKB(pkt) (IS_FKBUFF_PTR(pkt) && (_is_fkb_cloned_pool_(PNBUFF_2_FKBUFF(pkt))))
#define IS_MASTER_FKB(pkt) (IS_FKBUFF_PTR(pkt) && !(_is_fkb_cloned_pool_(PNBUFF_2_FKBUFF(pkt))))
#define FKB_IS_MASTER(pkt) (!(_is_fkb_cloned_pool_(PNBUFF_2_FKBUFF(pkt))))
#define FKB_IS_CLONED(pkt) ((_is_fkb_cloned_pool_(PNBUFF_2_FKBUFF(pkt))))

#ifndef BCM_NBUFF_PKT

extern void *osl_pkt_get_tag(void *pkt);
extern int osl_pkttag_attach(void *osh, void *pkt);
extern void *osl_pktlink(void *skb);
extern void osl_pktsetlink(void *skb, void *x);
extern uint osl_pktprio(void *skb);
extern void osl_pktsetprio(void *skb, uint x);
extern uchar *osl_pktdata(osl_t *osh, void *skb);
extern uint osl_pktlen(osl_t *osh, void *skb);
extern uint nbuff_pktheadroom(osl_t *osh, void *skb);
extern uint osl_pktflowid(void *skb);
extern void osl_pktsetflowid(void *skb, uint x);
extern uchar *nbuff_pktpush(osl_t *osh, void *skb, int bytes);
extern uchar *nbuff_pktpull(osl_t *osh, void *skb, int bytes);
extern void osl_pktsetlen(osl_t *osh, void *skb, uint len);
extern void osl_pkt_set_dirtyp_len(osl_t *osh, void *p,int len);

#define PKTATTACHTAG(osh,skb)           osl_pkttag_attach((osh), (skb))
#ifdef PKTPUSH
#undef PKTPUSH
#endif
#define PKTPUSH(osh, skb, bytes)        nbuff_pktpush((osh), (skb), (bytes))
#ifdef PKTPULL
#undef PKTPULL
#endif
#define PKTPULL(osh, skb, bytes)        nbuff_pktpull((osh), (skb), (bytes))
#ifdef PKTSETLEN
#undef PKTSETLEN
#endif
#define PKTSETLEN(osh, skb, len)        osl_pktsetlen((osh), (skb), (len))
#ifdef PKTFLOWID
#undef PKTFLOWID
#endif
#define PKTFLOWID(skb)                  osl_pktflowid((skb))
#ifdef PKTSETFLOWID
#undef PKTSETFLOWID
#endif
#define PKTSETFLOWID(skb, x)    osl_pktsetflowid((skb), (x))
#ifdef PKTSETDIRTYPLEN
#undef PKTSETDIRTYPLEN
#endif
#define PKTSETDIRTYPLEN(osh, skb, len) osl_pkt_set_dirtyp_len(osh,skb,len)

#endif /* !BCM_NBUFF_PKT */

/*
 *  NBUFF (fkb) type packet does not have prev or next pointers,
 *  and can't be made into double linked list.
 *  We add prev and next pointers in the dhd_pkttag_fd
 *  to help make double linked list for packets
 */
typedef struct dll DHD_PKT_LIST;
#ifdef PKTLIST_INIT
#undef PKTLIST_INIT
#endif
#define PKTLIST_INIT(x)		dhd_pkt_queue_head_init((DHD_PKT_LIST *)(x))
#ifdef PKTLIST_ENQ
#undef PKTLIST_ENQ
#endif
#define PKTLIST_ENQ(x, y)	dhd_pkt_queue_head((DHD_PKT_LIST *)(x), (void *)(y))
#ifdef PKTLIST_DEQ
#undef PKTLIST_DEQ
#endif
#define PKTLIST_DEQ(x)		dhd_pkt_dequeue((DHD_PKT_LIST *)(x))
#ifdef PKTLIST_UNLINK
#undef PKTLIST_UNLINK
#endif
#define PKTLIST_UNLINK(x, y)	dhd_pkt_unlink((DHD_PKT_LIST *)(x), (void *)(y))
#ifdef PKTLIST_FINI
#undef PKTLIST_FINI
#endif
#define PKTLIST_FINI(osh, x)	dhd_pkt_queue_purge((osl_t *)(osh), (DHD_PKT_LIST *)(x))


#define DHD_PKTTAG_WMF_FKB_UCAST    0x0004  /* tag as wmf handled fkb unicast pkt */
#ifdef BCM_NBUFF
#define DHD_PKT_GET_WMF_FKB_UCAST(pkt)  ((DHD_PKTTAG_FD(pkt))->flags & DHD_PKTTAG_WMF_FKB_UCAST) ? TRUE : FALSE
#define DHD_PKT_SET_WMF_FKB_UCAST(pkt)  ((DHD_PKTTAG_FD(pkt))->flags |= DHD_PKTTAG_WMF_FKB_UCAST)
#define DHD_PKT_CLR_WMF_FKB_UCAST(pkt)  ((DHD_PKTTAG_FD(pkt))->flags &= ~DHD_PKTTAG_WMF_FKB_UCAST)
#else
#define DHD_PKT_GET_WMF_FKB_UCAST(pkt)  FALSE
#define DHD_PKT_SET_WMF_FKB_UCAST(pkt)  ({ BCM_REFERENCE(pkt); })
#define DHD_PKT_CLR_WMF_FKB_UCAST(pkt)  ({ BCM_REFERENCE(pkt); })
#endif /* BCM_NBUFF */

#define DHD_PKTTAG_FKBPOOL 0x0020  /* tag as FKB */
#ifdef WLCSM_DEBUG
void  dhd_pkt_set_fkbclone(void *pkt);
#define DHD_PKT_SET_FKBPOOL(pkt)  dhd_pkt_set_fkbclone(pkt)
#else
#define DHD_PKT_SET_FKBPOOL(pkt)   \
        ((DHD_PKTTAG_FD(pkt))->flags |= DHD_PKTTAG_FKBPOOL)
#endif

#define DHD_PKT_GET_FKBPOOL(pkt)   \
(IS_MASTER_FKB(pkt)?(((DHD_PKTTAG_FD(pkt))->flags & DHD_PKTTAG_FKBPOOL) ? TRUE : FALSE):FALSE)

#define DHD_PKT_CLR_FKBPOOL(pkt)   \
        ((DHD_PKTTAG_FD(pkt))->flags &= ~DHD_PKTTAG_FKBPOOL)


#define DHD_PKTTAG_DATA_DHDHDR  0x0040 /* tag as DHDHDR MOVED */

void dhd_set_fkb_dhdhdr_flag(void *pkt);
bool dhd_get_fkb_dhdhdr_flag(void *pkt);
void dhd_clr_fkb_dhdhdr_flag(void *pkt);
#define DHD_PKT_SET_DATA_DHDHDR(pkt)   (IS_FKBUFF_PTR(pkt)?dhd_set_fkb_dhdhdr_flag(pkt):({ ((void)(pkt)); }))
#define DHD_PKT_GET_DATA_DHDHDR(pkt)  (IS_FKBUFF_PTR(pkt)?dhd_get_fkb_dhdhdr_flag(pkt): FALSE)
#define DHD_PKT_CLR_DATA_DHDHDR(pkt)   (IS_FKBUFF_PTR(pkt)?dhd_clr_fkb_dhdhdr_flag(pkt):({ ((void)(pkt)); }))

#define  FKB_HAS_DHDHDR(pkt) DHD_PKT_GET_DATA_DHDHDR(pkt)

int dhd_pkttag_pool_init(void) ;
void dhd_pkttag_pool_free(void);
/* dhd pkttags pool size.
	Check FKBC clone pool(2080) to have more that this when you change it to bigger num.
*/
void osl_pkt_clear_tag(void *pkt);
void *dhd_pkttags_pool_get(void);
void dhd_pkttags_pool_put(void *tag);
#define DHD_PKTTAGS_POOL_GET() dhd_pkttags_pool_get()
#define DHD_PKTTAGS_POOL_PUT(t) dhd_pkttags_pool_put(t)

#ifndef BCM_NBUFF_WLMCAST
enum WLEMF_CMD {
    WLEMF_CMD_GETIGSC,         //get igsc instance
    WLEMF_CMD_SCBFIND,     //find scb by mac
    WLEMF_CMD_GETDEV,
    WLEMF_CMD_PKTDUP,
    WLEMF_CMD_ADD_STA_IP,
    WLEMF_CMD_STA_OFFLOAD_CHECK,   //to check if sta is using hw_ring for multicast
    WLEMF_CMD_PKTFREE
};
#endif


/* PKTLIST */
void dhd_pkt_queue_purge(osl_t *osh, DHD_PKT_LIST *list);
void *dhd_pkt_dequeue(DHD_PKT_LIST *list);
void dhd_pkt_unlink(DHD_PKT_LIST *list, void *pkt);
void dhd_pkt_queue_head(DHD_PKT_LIST *list,	void *newpkt);
void dhd_pkt_queue_head_init(DHD_PKT_LIST *list);
int dhd_nbuff_free(void *p);
void *osl_nbuff_dup(osl_t *osh,void *skb);
int dhd_nbuff_dup(osl_t *osl, void *p, void **ret_p);
extern void dhd_nbuff_dump(void *dhdp, void *strbuf);
#define DHD_PKT_GET_FLOWID(pkt)     PKTFLOWID(pkt)
#define DHD_PKT_SET_FLOWID(pkt, fid)    PKTSETFLOWID(pkt, fid)
#define DHD_PKT_SET_MAC(pkt, mac)   \
        (bcopy((mac), DHD_PKTTAG_FD((pkt))->mac_address, ETHER_ADDR_LEN))
#define DHD_PKT_GET_MAC(pkt)        ((DHD_PKTTAG_FD(pkt))->mac_address)

#ifndef BCM_NBUFF_PKT

#ifdef PKTDUP_CPY
#undef PKTDUP_CPY
#endif
#define PKTDUP_CPY(osh,pkt) (IS_SKBUFF_PTR(pkt)?osl_pktdup_cpy((osh),pkt):DHD_FKB_CLONE2UNICAST((osh),pkt,NULL))

#ifdef  PKTHEADROOM
#undef PKTHEADROOM
#endif
#define PKTHEADROOM(osh, skb)           nbuff_pktheadroom((osh), (skb))

#ifdef PKTDATA
#undef PKTDATA
#endif
#define PKTDATA(osh, skb)               osl_pktdata((osh), (skb))

#ifdef PKTLEN
#undef PKTLEN
#endif
#define PKTLEN(osh, skb)                osl_pktlen((osh), (skb))

#ifdef PKTFRMNATIVE
#undef PKTFRMNATIVE
#define PKTFRMNATIVE(osh, skb)  (IS_SKBUFF_PTR(skb)? (osl_pkt_frmnative(((osl_t *)osh), (struct sk_buff*)(skb))): skb)
#endif

#ifdef  PKTISFRMNATIVE
#undef  PKTISFRMNATIVE
#endif
#define PKTISFRMNATIVE(osh, skb) (IS_SKBUFF_PTR(skb) ?( osl_pkt_is_frmnative((osl_t *)(osh), (struct sk_buff *)(skb))): FALSE)

#ifdef  PKTTONATIVE
#undef  PKTTONATIVE
#endif
#define PKTTONATIVE(osh, pkt)    (IS_SKBUFF_PTR(skb) ?( osl_pkt_tonative((osl_t *)(osh), (pkt))): pkt)

#ifdef PKTLINK
#undef PKTLINK
#endif
#define PKTLINK(skb)                    osl_pktlink((skb))

#ifdef PKTSETLINK
#undef PKTSETLINK
#endif
#define PKTSETLINK(skb, x)              osl_pktsetlink((skb), (x))

#ifdef PKTPRIO
#undef PKTPRIO
#endif
#define PKTPRIO(skb)                    osl_pktprio((skb))

#ifdef PKTSETPRIO
#undef PKTSETPRIO
#endif

#define PKTSETPRIO(skb, x)              osl_pktsetprio((skb), (x))

#ifdef PKTDUP
#undef PKTDUP
#endif
#define PKTDUP(osh, pkt)    (IS_SKBUFF_PTR(pkt) ?( osl_pktdup((osl_t *)(osh), (pkt))):osl_nbuff_dup((osh),pkt))

#ifdef PKTFREE
#undef PKTFREE
#endif
#define  PKTFREE(osh,pkt,c)  (IS_SKBUFF_PTR(pkt)? (osl_pktfree((osl_t *)osh,(pkt),c)): dhd_nbuff_free((pkt)))

#ifdef PKTTAG
#undef PKTTAG
#endif
#define PKTTAG(pkt)                     ((dhd_pkttag_fd_t *)osl_pkt_get_tag(pkt))

#ifdef PKTC

#ifdef PKTCSETATTR
#undef PKTCSETATTR
#endif
#define	PKTCSETATTR(s, f, p, b)	(IS_SKBUFF_PTR(s) ? ({CHAIN_NODE(s)->flags = (f); \
			CHAIN_NODE(s)->pkts = (p); CHAIN_NODE(s)->bytes = (b);}) : BCM_REFERENCE(s))
#ifdef PKTCCLRATTR
#undef PKTCCLRATTR
#endif
#define	PKTCCLRATTR(skb)	(IS_SKBUFF_PTR(skb) ? ({CHAIN_NODE(skb)->flags = \
		   	CHAIN_NODE(skb)->pkts = CHAIN_NODE(skb)->bytes = 0;}) : BCM_REFERENCE(skb))
#ifdef PKTCGETATTR
#undef PKTCGETATTR
#endif
#define	PKTCGETATTR(skb)	(IS_SKBUFF_PTR(skb) ? (CHAIN_NODE(skb)->flags << 29 | \
			CHAIN_NODE(skb)->pkts << 20 | CHAIN_NODE(skb)->bytes) : BCM_REFERENCE(skb))
#ifdef PKTCCNT
#undef PKTCCNT
#endif
#define	PKTCCNT(skb)		(IS_SKBUFF_PTR(skb) ? (CHAIN_NODE(skb)->pkts) : 1)
#ifdef PKTCLEN
#undef PKTCLEN
#endif
#define	PKTCLEN(skb)		(IS_SKBUFF_PTR(skb) ? (CHAIN_NODE(skb)->bytes) : PKTLEN(skb))
#ifdef PKTCGETFLAGS
#undef PKTCGETFLAGS
#endif
#define	PKTCGETFLAGS(skb)	(IS_SKBUFF_PTR(skb) ? (CHAIN_NODE(skb)->flags) : BCM_REFERENCE(skb))
#ifdef PKTCSETFLAGS
#undef PKTCSETFLAGS
#endif
#define	PKTCSETFLAGS(skb, f)	(IS_SKBUFF_PTR(skb) ? (CHAIN_NODE(skb)->flags = (f)) : BCM_REFERENCE(skb))
#ifdef PKTCCLRFLAGS
#undef PKTCCLRFLAGS
#endif
#define	PKTCCLRFLAGS(skb)	(IS_SKBUFF_PTR(skb) ? (CHAIN_NODE(skb)->flags = 0) : BCM_REFERENCE(skb))
#ifdef PKTCFLAGS
#undef PKTCFLAGS
#endif
#define	PKTCFLAGS(skb)		(IS_SKBUFF_PTR(skb) ? (CHAIN_NODE(skb)->flags) : BCM_REFERENCE(skb))
#ifdef PKTCSETCNT
#undef PKTCSETCNT
#endif
#define	PKTCSETCNT(skb, c)	(IS_SKBUFF_PTR(skb) ? (CHAIN_NODE(skb)->pkts = (c)) : BCM_REFERENCE(skb))
#ifdef PKTCINCRCNT
#undef PKTCINCRCNT
#endif
#define	PKTCINCRCNT(skb)	(IS_SKBUFF_PTR(skb) ? (CHAIN_NODE(skb)->pkts++) : BCM_REFERENCE(skb))
#ifdef PKTCADDCNT
#undef PKTCADDCNT
#endif
#define	PKTCADDCNT(skb, c)	(IS_SKBUFF_PTR(skb) ? (CHAIN_NODE(skb)->pkts += (c)) : BCM_REFERENCE(skb))
#ifdef PKTCSETLEN
#undef PKTCSETLEN
#endif
#define	PKTCSETLEN(skb, l)	(IS_SKBUFF_PTR(skb) ? (CHAIN_NODE(skb)->bytes = (l)) : BCM_REFERENCE(skb))
#ifdef PKTCADDLEN
#undef PKTCADDLEN
#endif
#define	PKTCADDLEN(skb, l)	(IS_SKBUFF_PTR(skb) ? (CHAIN_NODE(skb)->bytes += (l)) : BCM_REFERENCE(skb))
#ifdef PKTCSETFLAG
#undef PKTCSETFLAG
#endif
#define	PKTCSETFLAG(skb, fb)	(IS_SKBUFF_PTR(skb) ? (CHAIN_NODE(skb)->flags |= (fb)) : BCM_REFERENCE(skb))
#ifdef PKTCCLRFLAG
#undef PKTCCLRFLAG
#endif
#define	PKTCCLRFLAG(skb, fb)	(IS_SKBUFF_PTR(skb) ? (CHAIN_NODE(skb)->flags &= ~(fb)) : BCM_REFERENCE(skb))
#ifdef PKTCLINK
#undef PKTCLINK
#endif
#define	PKTCLINK(skb) 		(IS_SKBUFF_PTR(skb) ? (CHAIN_NODE(skb)->link) : NULL)
#ifdef PKTSETCLINK
#undef PKTSETCLINK
#endif
#define	PKTSETCLINK(skb, x)	(IS_SKBUFF_PTR(skb) ? (CHAIN_NODE(skb)->link = (struct sk_buff*)(x)) : BCM_REFERENCE(skb))

#ifdef FOREACH_CHAINED_PKT
#undef FOREACH_CHAINED_PKT
#endif
#define FOREACH_CHAINED_PKT(skb, nskb)  \
	for (; (skb) != NULL; (skb) = (nskb)) \
		  if ((nskb) = (PKTISCHAINED(skb) ? PKTCLINK(skb) : NULL),\
				  PKTSETCLINK((skb), NULL), 1)

#ifdef PKTCFREE
#undef PKTCFREE
#endif
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

#ifdef PKTCENQTAIL
#undef PKTCENQTAIL
#endif
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

#endif /* !BCM_NBUFF_PKT */

#define ETHER_ADD_LEN (6)

#define IS_FKBWMFUCAST_NONFKBCLONE(pkt) ((DHD_PKT_GET_WMF_FKB_UCAST(pkt)) && \
    (!(DHD_PKT_GET_FKBPOOL(pkt))))
#define IS_FKB_WMF_UCAST(pkt) ((DHD_PKT_GET_WMF_FKB_UCAST(pkt)))


void dhd_nbuff_detach(void);
int dhd_nbuff_attach(void);

#define FKB_IS_FKBPOOL(pkt)  (DHD_PKT_GET_FKBPOOL(pkt))
#define FKB_IS_WMF_UCAST(pkt) (DHD_PKT_GET_WMF_FKB_UCAST(pkt))
#define PRIO_LOC_NFMARK 16
#include <dhd_nic_common.h>

#else

#define DHD_PKT_GET_FKBPOOL(pkt)   FALSE
#define DHD_PKT_SET_FKBPOOL(pkt)   ({ ((void)(pkt)); })
#define DHD_PKT_CLR_FKBPOOL(pkt)   ({ ((void)(pkt)); })

#endif /* BCM_NBUFF */

#if (defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE))
#if !defined(BCM_AWL)
#define BCM_NBUFF_PKT_BPM /* Packet data buffers, are from BPM pool */

#if defined(BCA_CPEROUTER) && defined(CONFIG_BCM947189)
#ifndef BCM_NBUFF_PKT_BPM_SKB
#define BCM_NBUFF_PKT_BPM_SKB /* Packet buffers are from BPM SKB pool */
#endif
#endif

#include <linux/gbpm.h>
#else /* BCM_AWL */
#include <bpm.h>
#if defined(BCM_NBUFF_PKT) && defined(CC_BPM_SKB_POOL_BUILD)
#define BCM_NBUFF_PKT_BPM_SKB /* Packet buffers are from BPM SKB pool */
#endif /* BCM_NBUFF_PKT && CC_BPM_SKB_POOL_BUILD */
#endif /* BCM_AWL */
#endif /* (CONFIG_BCM_BPM || CONFIG_BCM_BPM_MODULE) */

#if defined(BCM_NBUFF_PKT_BPM)

/* Forward declarations */
struct dhd_pub;
typedef struct dhd_pub dhd_pub_t;

#define DHD_DATABUF_CACHE_MAXBUFS       (128U) /* Multiple of RX_BUF_BURST */

extern int dhd_nbuff_bpm_init(dhd_pub_t * dhd_pub);
extern void dhd_nbuff_bpm_deinit(dhd_pub_t * dhd_pub);
extern void dhd_databuf_cache_clear(dhd_pub_t * dhdp);
extern void *dhd_databuf_alloc(dhd_pub_t * dhdp);
extern struct sk_buff * dhd_xlate_to_skb(dhd_pub_t * dhd_pub, pNBuff_t pNBuff);

#endif /* BCM_NBUFF_PKT_BPM */

#if defined(BCM_NBUFF_PKT_BPM_SKB)
/* Forward declarations */
struct dhd_pub;
typedef struct dhd_pub dhd_pub_t;

extern struct sk_buff *dhd_nbuff_bpm_skb_get(dhd_pub_t *dhdp, int len);
#endif /* BCM_NBUFF_PKT_BPM_SKB */


#endif /* __DHD_NBUFF_H__ */
