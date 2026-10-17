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

/**
 * ------------------------------------------------------------------------------------------------
 *  SECTION: PKTLIST
 * ------------------------------------------------------------------------------------------------
 */

/*
 *  NBUFF (fkb) type packet does not have prev or next pointers,
 *  and can't be made into double linked list.
 *  We add prev and next pointers in the dhd_pkttag_fd
 *  to help make double linked list for packets
 */
typedef struct dll DHD_PKT_LIST;

void dhd_pkt_queue_purge(osl_t *osh, DHD_PKT_LIST *list);
void *dhd_pkt_dequeue(DHD_PKT_LIST *list);
void dhd_pkt_unlink(DHD_PKT_LIST *list, void *pkt);
void dhd_pkt_queue_head(DHD_PKT_LIST *list,	void *newpkt);
void dhd_pkt_queue_head_init(DHD_PKT_LIST *list);

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

/**
 * ------------------------------------------------------------------------------------------------
 *  SECTION: PKTTAG POOL
 * ------------------------------------------------------------------------------------------------
 */

#define DHD_PKTTAG_WMF_FKB_UCAST    0x0004  /* tag as wmf handled fkb unicast pkt */

#define DHD_PKT_GET_WMF_FKB_UCAST(pkt) \
	((DHD_PKTTAG_FD(pkt))->flags & DHD_PKTTAG_WMF_FKB_UCAST) ? TRUE : FALSE
#define DHD_PKT_SET_WMF_FKB_UCAST(pkt)  ((DHD_PKTTAG_FD(pkt))->flags |= DHD_PKTTAG_WMF_FKB_UCAST)
#define DHD_PKT_CLR_WMF_FKB_UCAST(pkt)  ((DHD_PKTTAG_FD(pkt))->flags &= ~DHD_PKTTAG_WMF_FKB_UCAST)

#define DHD_PKTTAG_FKBPOOL 0x0020  /* tag as FKB */

int dhd_pkttag_pool_init(void);
void dhd_pkttag_pool_free(void);
/* dhd pkttags pool size.
	Check FKBC clone pool(2080) to have more that this when you change it to bigger num.
*/
void *dhd_pkttags_pool_get(void);
void dhd_pkttags_pool_put(void *tag);
#define DHD_PKTTAGS_POOL_GET() dhd_pkttags_pool_get()
#define DHD_PKTTAGS_POOL_PUT(t) dhd_pkttags_pool_put(t)

/**
 * ------------------------------------------------------------------------------------------------
 *  SECTION: FKBPOOL
 * ------------------------------------------------------------------------------------------------
 */

#ifdef WLCSM_DEBUG
void dhd_pkt_set_fkbclone(void *pkt);
#define DHD_PKT_SET_FKBPOOL(pkt)  dhd_pkt_set_fkbclone(pkt)
#else
#define DHD_PKT_SET_FKBPOOL(pkt)  ((DHD_PKTTAG_FD(pkt))->flags |= DHD_PKTTAG_FKBPOOL)
#endif

#define DHD_PKT_GET_FKBPOOL(pkt) \
	(IS_MASTER_FKB(pkt) ? (((DHD_PKTTAG_FD(pkt))->flags & DHD_PKTTAG_FKBPOOL) ? TRUE : FALSE) \
		: FALSE)

#define DHD_PKT_CLR_FKBPOOL(pkt)  ((DHD_PKTTAG_FD(pkt))->flags &= ~DHD_PKTTAG_FKBPOOL)

#define FKB_IS_FKBPOOL(pkt)  (DHD_PKT_GET_FKBPOOL(pkt))

#define DHD_PKTTAG_DATA_DHDHDR  0x0040 /* tag as DHDHDR MOVED */

void dhd_set_fkb_dhdhdr_flag(void *pkt);
bool dhd_get_fkb_dhdhdr_flag(void *pkt);
void dhd_clr_fkb_dhdhdr_flag(void *pkt);
#define DHD_PKT_SET_DATA_DHDHDR(pkt) \
	(IS_FKBUFF_PTR(pkt) ? dhd_set_fkb_dhdhdr_flag(pkt) : ({ ((void)(pkt)); }))
#define DHD_PKT_GET_DATA_DHDHDR(pkt)  (IS_FKBUFF_PTR(pkt) ? dhd_get_fkb_dhdhdr_flag(pkt) : FALSE)
#define DHD_PKT_CLR_DATA_DHDHDR(pkt) \
	(IS_FKBUFF_PTR(pkt) ? dhd_clr_fkb_dhdhdr_flag(pkt) : ({ ((void)(pkt)); }))

#define  FKB_HAS_DHDHDR(pkt) DHD_PKT_GET_DATA_DHDHDR(pkt)

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

#define ETHER_ADD_LEN (6)

#define IS_FKBWMFUCAST_NONFKBCLONE(pkt) \
		((DHD_PKT_GET_WMF_FKB_UCAST(pkt)) && (!(DHD_PKT_GET_FKBPOOL(pkt))))
#define IS_FKB_WMF_UCAST(pkt) ((DHD_PKT_GET_WMF_FKB_UCAST(pkt)))

#define FKB_IS_WMF_UCAST(pkt) (DHD_PKT_GET_WMF_FKB_UCAST(pkt))
#include <dhd_nic_common.h>


/**
 * ------------------------------------------------------------------------------------------------
 *  SECTION: DHD specific NBUFF (SKB|FKB) Packet Primitives - PKTxyz
 * ------------------------------------------------------------------------------------------------
 */
extern int	nbuff_pkttag_attach(void * osh, void * pkt);
extern uint	nbuff_pktflowid(void * pkt);
extern void	nbuff_pktsetflowid(void * pkt, uint flowid);
extern void *	nbuff_pkt_get_tag(void *pkt);

#define PKTATTACHTAG(osh, pkt)		nbuff_pkttag_attach((osh), (pkt))
#define PKTFLOWID(pkt)			nbuff_pktflowid((pkt))
#define PKTSETFLOWID(pkt, flowid)	nbuff_pktsetflowid((pkt), (flowid))

#define DHD_PKT_GET_FLOWID(pkt)		PKTFLOWID(pkt)
#define DHD_PKT_SET_FLOWID(pkt, fid)	PKTSETFLOWID(pkt, fid)
#define DHD_PKT_SET_MAC(pkt, mac)	\
	(bcopy((mac), DHD_PKTTAG_FD((pkt))->mac_address, ETHER_ADDR_LEN))
#define DHD_PKT_GET_MAC(pkt)		((DHD_PKTTAG_FD(pkt))->mac_address)


extern int	dhd_nbuff_attach(void);
extern void	dhd_nbuff_detach(void);
#if defined(DSLCPE) || defined(BCM_NBUFF_WLMCAST)
/* Add nbuff dump output to a buffer */
extern void	dhd_nbuff_dump(void *dhd, void *strb);
#endif /* DSLCPE || BCM_NBUFF_WLMCAST */

/**
 * ------------------------------------------------------------------------------------------------
 * Section: BPM based data buffers
 * ------------------------------------------------------------------------------------------------
 */
#if (defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE))
#if !(defined(BCM_AWL) || defined(DHD_SKB))
#define BCM_NBUFF_PKT_BPM /* Packet data buffers, are from BPM pool */
#include <linux/gbpm.h>
#else /* (defined(BCM_AWL) || defined(DHD_SKB)) */
#include <bpm.h>

#if defined(CC_BPM_SKB_POOL_BUILD)
#define BCM_NBUFF_PKT_BPM_SKB /* Packet buffers are from BPM SKB pool */
#endif /* CC_BPM_SKB_POOL_BUILD */
#endif /* !(defined(BCM_AWL) || defined(DHD_SKB)) */
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

extern struct dhd_pub *g_dhd_info[];
#if defined(BCM_NBUFF_PKT_BPM)
#define BCM_NBUFF_DHD_RECYCLE_HOOK 1
void dhd_nbuff_recycle(pNBuff_t pNBuff, unsigned long context, uint32_t flags);
#endif

#if defined(DHD_SKB)
#define DHD_NBUFF_PTR_TYPE      SKBUFF_PTR
#else
#define DHD_NBUFF_PTR_TYPE      FKBUFF_PTR
#endif /* defined(DHD_SKB) */

#endif /* BCM_NBUFF */

#endif /* __DHD_NBUFF_H__ */
