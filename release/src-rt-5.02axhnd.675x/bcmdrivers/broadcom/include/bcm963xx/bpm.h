
#ifndef __BPM_H_INCLUDED__
#define __BPM_H_INCLUDED__

/*
<:copyright-BRCM:2007:DUAL/GPL:standard

   Copyright (c) 2007 Broadcom 
   All Rights Reserved

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

/*
 *******************************************************************************
 * File Name  : bpm.h
 *
 *******************************************************************************
 */
#define BPM_VERSION             "v0.1"
#define BPM_VER_STR             BPM_VERSION
#define BPM_MODNAME             "Broadcom BPM Module"

/* BPM Character Device */
#define BPM_DRV_MAJOR           304

#define BPM_ERROR               (-1)
#define BPM_SUCCESS             0

// #define CC_BPM_DBG

#if defined(CC_BPM_DBG)
#define BPM_DBG_ASSERT(cond)    BCM_ASSERT(cond)
#else
#define BPM_DBG_ASSERT(cond)    do { /* noop */ } while (0)
#endif

#define BPM_HIGH_PRIO_ALLOC     (1)
#define BPM_LOW_PRIO_ALLOC      (0)
/*
 * CAUTION!!! 
 * It is highly recommended NOT to change the tuning parameters
 * in this file from their default values. Any change may badly affect
 * the performance of the system.
 */


/* The memory used for buffers is defined in terms of percent of 
 * total SDRAM memory on the system */
#define BPM_MAX_MEMSIZE         (64 * 1024) // each memory chunk of 64KB
#define BPM_MAX_MEM_POOL_IX     (16*1024)   // how many memory chunks of 64KB

#define BPM_MAX_BUF_POOL_IX       \
    ((BPM_MAX_MEMSIZE * BPM_MAX_MEM_POOL_IX)/BCM_PKTBUF_SIZE)


#define MB (1024*1024)


/*
 *------------------------------------------------------------------------------
 * Layout of a BPM buffer
 *
 * FkBuff_t|BCM_PKT_HEADROOM|BCM_MAX_PKT_LEN|BCM_SKB_TAILROOM|BCM_SKB_SHAREDINFO
 *
 * ^       ^                ^               ^                ^
 * |       |                |               |                |
 * pFkb    |                pData           |                |
 *         |                |               |                |
 *         |                |               |                |
 *      skb:head         skb:data        skb:tail         skb:end
 *                          |
 *                          |
 *                        "pBuf"
 *
 * CAUTION: BPM buf_pool[] maintains pBuf pointers (and not FkBuff_t pointers).
 *
 * Note: The data pointer may travel, between head and end.
 *
 * DMA transfer to a buffer may only occur between pBuf of BCM_MAX_PKT_LEN.
 *
 * #include nbuff.h
 * #include bcm_pkt_lengths.h
 *
 * Assumptions:
 * - sizeof FkBuff_t is multiple of cacheline
 * - BCM_PKT_HEADROOM is multiple of cacheline
 *
 * Actual tailroom in a DMAed received packet is runtime defined.
 *------------------------------------------------------------------------------
 */


/* Constant offset of skb_shared_info from pData in a BPM managed buffer */
#define BPM_BUF_TO_END_OFFSET \
    BCM_DCACHE_ALIGN(BCM_MAX_PKT_LEN + BCM_SKB_TAILROOM)

#define BPM_PFKBUFF_TO_BUF(pFkb)   ( PFKBUFF_TO_PDATA(pFkb, BCM_PKT_HEADROOM) )
#define BPM_BUF_TO_PFKBUFF(buf)    ( PDATA_TO_PFKBUFF((buf), BCM_PKT_HEADROOM) )

#define BPM_BUF_TO_PHEAD(buf)      ( ((uint8_t*)(buf)) - BCM_PKT_HEADROOM )
#define BPM_PHEAD_TO_BUF(head)     ( ((uint8_t*)(head)) + BCM_PKT_HEADROOM )

#ifdef NET_SKBUFF_DATA_USES_OFFSET

/* skb:tail and skb:end are specified as offsets from skb:head */
#define BPM_BUF_TO_TAIL(buf, datalen) \
    ( (sk_buff_data_t) (BCM_PKT_HEADROOM + (datalen)) )

#define BPM_BUF_TO_END(buf) \
   ( (sk_buff_data_t) (BCM_PKT_HEADROOM + BPM_BUF_TO_END_OFFSET) )

#else  /* ! NET_SKBUFF_DATA_USES_OFFSET */

/* skb:tail and skb:end are specified as pointers */
#define BPM_BUF_TO_TAIL(buf, datalen) \
    ( (sk_buff_data_t) ( ((sk_buff_data_t)(buf)) + (datalen) ) )

#define BPM_BUF_TO_END(buf) \
    ( (sk_buff_data_t) ( (uint8_t*)(buf) + BPM_BUF_TO_END_OFFSET) )

#endif /* ! NET_SKBUFF_DATA_USES_OFFSET */

#define BPM_BUF_TO_PSHINFO(buf) \
    (struct skb_shared_info *)( (uint8_t*)(buf) + BPM_BUF_TO_END_OFFSET)


/*
 * An interface can request additional buffers from global BPM, when a 
 * percentage of its RX ring buffers are used (not yet replenished).
 * An interace will requst only when the number of used buffers has crossed the
 * BPM_XXX_ALLOC_TRIG_PCT threshold.
 */
#define BPM_ENET_ALLOC_TRIG_PCT         15

#if defined(CONFIG_BCM_XTMCFG) || defined(CONFIG_BCM_XTMCFG_MODULE)
#define BPM_XTM_ALLOC_TRIG_PCT          15
#endif


/* 
 * bulk alloc count: how many buffers are requested, after trigger threshold
 * has been hit.
 */
#define BPM_ENET_BULK_ALLOC_COUNT       128

#if defined(CONFIG_BCM_XTMCFG) || defined(CONFIG_BCM_XTMCFG_MODULE)
#define BPM_XTM_BULK_ALLOC_COUNT        64
#endif




/* From the global buffer pool, some buffers are allocated to the RX ring of
 * a channel and are considered as reserved.
 *
 * BPM_PCT_RESV_BUF_RXRING indicates what percentage of the reserved
 * buffers should contribute to reserving of buffers from global buffer pool.
 */
#define BPM_PCT_RESV_BUF_RXRING         100


/* The number of dynamic buffers is obtained by subtracting the reserved
 * buffers from the total buffers in the global buffer pool.
 *
 * Dynamic buffer low threshold is used to decide whether to use the 
 * low or high threshold of a TX queue to make decision to enqueue or drop
 * a packet. If the TX queue depth is less than the used threshold the 
 * packet is enqueued else the packet is dropped. 
 *
 * dynamic buffer low threshold is calculated by using
 * BPM_PCT_DYN_BUF_LO_THRESH percentage of total dynamic buffers.
 */
#define BPM_PCT_DYN_BUF_LO_THRESH       10



/* BPM TxQ Low and High Thresholds for XTM */
#if defined(CONFIG_BCM_XTMCFG) || defined(CONFIG_BCM_XTMCFG_MODULE)
/* Dynamically assign the threshold based on the US link speed */
/* Min # of additional buffers assigned */
#define XTM_BPM_TXQ_HI_BUF_MIN          300
#define XTM_BPM_TXQ_LO_BUF_MIN          200

/* Low Threshold is set in percent of High Threshold */
#define XTM_BPM_PCT_BUF_LO_THRESH       50

/* Low and High Thresholds for 120Mbps US link */
#define XTM_BPM_TXQ_HI_BUF_120MBPS      HOST_XTM_NR_TXBDS
#define XTM_BPM_TXQ_LO_BUF_120MBPS  \
    (XTM_BPM_TXQ_HI_BUF_120MBPS * XTM_BPM_PCT_BUF_LO_THRESH/100)

/* Low and high threshold for any US speed is assigned proportionate 
 * to 120MBPS US link
 */
#define XTM_BPM_CALC_TXQ_LO_THRESH(s)                                           \
    ((((s) * XTM_BPM_TXQ_LO_BUF_120MBPS) / 120) + XTM_BPM_TXQ_LO_BUF_MIN)
#define XTM_BPM_CALC_TXQ_HI_THRESH(s)                                           \
    ((((s) * XTM_BPM_TXQ_HI_BUF_120MBPS) / 120) + XTM_BPM_TXQ_HI_BUF_MIN)

#define XTM_BPM_TXQ_LO_THRESH(s)                                            \
    ((XTM_BPM_CALC_TXQ_LO_THRESH((s))>XTM_BPM_TXQ_HI_BUF_120MBPS)?XTM_BPM_TXQ_HI_BUF_120MBPS: \
     XTM_BPM_CALC_TXQ_LO_THRESH((s)))

#define XTM_BPM_TXQ_HI_THRESH(s)                                           \
    ((XTM_BPM_CALC_TXQ_HI_THRESH((s))>XTM_BPM_TXQ_HI_BUF_120MBPS)?XTM_BPM_TXQ_HI_BUF_120MBPS: \
     XTM_BPM_CALC_TXQ_HI_THRESH((s)))
#endif



/* BPM TxQ Low and High Thresholds for MoCA */


typedef struct {
    uint32_t q_lo_thresh;
    uint32_t q_hi_thresh;
    uint32_t q_dropped;
} bpm_thresh_t;


/* 
 * Eth TxQ drop thresholds
 * -----------------------
 * An outgoing packet's priority is mapped to one of the four 
 * egress queues (0-3). Although there are 4 queues but Q0 and Q1 are 
 * mapped to the same priority level.
 * 
 * FAP and MIPS use Tx IuDMA channel to transmit packet to Ethernet switch.
 * The transmitted packet can have any priority between 0-3. Tx IuDMA ring
 * can become full because the incoming packet rate is higher than outgoing
 * pakcet rate. When Tx IuDMA ring becomes full we need to give preference to
 * higher priority packets. 
 *
 * We donot know in advance the mix of high, medium, and low priority packet
 * rates. But we can reserve some bandwidth for the high and medium priority
 * packets using the drop thresholds. High priority will have highest 
 * threshold, and low priority will have lowest threshold and the medium 
 * will be in between. The drop thresholds are specified in terms of 
 * percentage of Tx IuDMA ring size. 
 *
 * When the outgoing packet has a priority p, and the current instantaneous
 * queue depth is greater or equal to priority drop threshold, the packet 
 * will be dropped, in all other cases packet will be queued for transmission.
 *
 * The configured drop threshold and the number of packets dropped for a 
 * priority/queue can be dumped using the "bpm thresh" CLI command
 *
 * CAUTION: 
 * 1. A user may fine tune the threshold values based on the requirements. 
 * But lowering the threshold too much may affect the packet rates achieved for
 * lower priorities.
 */
#define ENET_BPM_PCT_TXQ0_DROP_THRESH      75
#define ENET_BPM_PCT_TXQ1_DROP_THRESH      ENET_BPM_PCT_TXQ0_DROP_THRESH
#define ENET_BPM_PCT_TXQ2_DROP_THRESH      85
#define ENET_BPM_PCT_TXQ3_DROP_THRESH      100

#if (ENET_BPM_PCT_TXQ1_DROP_THRESH != ENET_BPM_PCT_TXQ0_DROP_THRESH)
#error "ERROR -(ENET_BPM_PCT_TXQ1_DROP_THRESH != ENET_BPM_PCT_TXQ0_DROP_THRESH)"
#endif

/**
 * Pre-allocated shared pool of skbuff(s), managed by BPM.
 * skbuff(s) are returned to BPM in a pristined state (pre-initialized).
 */

/* BPM skb pool conditional compile */
/* BPM is populated with a pre-allocated pool at init time. */
#define CC_BPM_SKB_POOL_BUILD

/* On depletion the skbpool may self grow to the max BPM size. */
// #define CC_BPM_SKB_POOL_GROWS /* experimental */
// #define CC_BPM_SKB_POOL_STATS

/* Used to compute the number of skbuff(s) as a percent of total BPM buffers */
#define BPM_SKB_POOL_PCT_INIT             80

/* Extend BPM SKB pool by 16 skbuffs, on depleted BPM Pool */
#define BPM_SKB_POOL_EXTEND               16

#endif /*  __BPM_H_INCLUDED__ */
