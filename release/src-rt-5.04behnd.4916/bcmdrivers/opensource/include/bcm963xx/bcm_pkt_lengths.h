#ifndef __BCM_PKT_LENGTHS_H__
#define __BCM_PKT_LENGTHS_H__

/*TODO: makesure this file compiles when not for linux kernel also */


/* this file is used to define the packet length for All platforms and
   features at a common place */

#define BCM_DCACHE_LINE_LEN     64ul
#define BCM_DCACHE_ALIGN_LEN    63ul

#if defined(__KERNEL__)
#define BCM_DCACHE_ALIGN(len)   ((len + BCM_DCACHE_ALIGN_LEN) & ~BCM_DCACHE_ALIGN_LEN)
#endif /* __KERNEL__ */

#define GREATER(x, y)           (((x) > (y)) ? (x) : (y))

#if defined(L1_CACHE_BYTES)
#if (BCM_DCACHE_LINE_LEN != L1_CACHE_BYTES)
#error "BCM_DCACHE_LINE_LEN != L1_CACHE_BYTES"
#endif
#endif /* L1_CACHE_BYTES */

 /* ############ MAX Packet payload size ############ */

/* define chip dependent jumbo mtu payload max */
#if defined(CONFIG_BCM963158)
#define MAX_JUMBO_MTU_PAYLOAD_SIZE  (4000)  /* restrict by FPM_buf_size 512*8=4k, set to 1k for 8k MTU or 2k for 9k MTU*/
#elif defined(CONFIG_BCM94912) || defined(CONFIG_BCM96813)
#define MAX_JUMBO_MTU_PAYLOAD_SIZE  (10240) /* set rdp1 token size from 40 to 320 */
#elif defined(CONFIG_BCM947622)
#define MAX_JUMBO_MTU_PAYLOAD_SIZE  (9216)
#elif defined(CONFIG_BCM96764)
#define MAX_JUMBO_MTU_PAYLOAD_SIZE  (9216)
#elif defined(CONFIG_BCM963146)
/* 63146 Runner FW flows don't support 2048 MTU */
#define MAX_JUMBO_MTU_PAYLOAD_SIZE  (2044)
#elif defined(CONFIG_BCM963178)
#define MAX_JUMBO_MTU_PAYLOAD_SIZE  (2048)
#elif defined(CONFIG_BCM96756)
#define MAX_JUMBO_MTU_PAYLOAD_SIZE  (4014)
#elif defined(CONFIG_BCM96765) || defined(CONFIG_BCM96766)
#if defined(CONFIG_BCM_CROSSBOW_FULL_OFFLOAD)
#define MAX_JUMBO_MTU_PAYLOAD_SIZE  (9216)
#else
#define MAX_JUMBO_MTU_PAYLOAD_SIZE  (4014)
#endif
#endif /* CONFIG_BCM963158 */

/* Ethernet */
/* Not chip specific but feature specific */
#if defined(CONFIG_BCM_JUMBO_FRAME) && defined(CONFIG_BCM_MAX_MTU_SIZE)
#if defined(MAX_JUMBO_MTU_PAYLOAD_SIZE)
    #if (CONFIG_BCM_MAX_MTU_SIZE > MAX_JUMBO_MTU_PAYLOAD_SIZE)
    #error "ERROR - CONFIG_BCM_MAX_MTU_SIZE > MAX_JUMBO_MTU_PAYLOAD_SIZE"
    #endif
#endif
#define ENET_MAX_MTU_PAYLOAD_SIZE  CONFIG_BCM_MAX_MTU_SIZE
#define ENET_NON_JUMBO_MAX_MTU_PAYLOAD_SIZE  (1500)
#else
#define ENET_MAX_MTU_PAYLOAD_SIZE  (1500)  /* Ethernet Max Payload Size */
#endif

/* XTM */
#if defined(CONFIG_BCM_PON_XRDP)

#define XTM_MAX_MTU_PAYLOAD_SIZE    0

#else /* ! CONFIG_BCM_PON_XRDP */

#if defined(CONFIG_BCM_JUMBO_FRAME)
#if defined(CONFIG_BCM963158)
/* 63158 Runner FW flows don't support 2048 MTU */
#define XTM_MAX_MTU_PAYLOAD_SIZE    2044  /* Ethernet Max Payload Size - mini jumbo */
#else
#define XTM_MAX_MTU_PAYLOAD_SIZE    2048  /* Ethernet Max Payload Size - mini jumbo */
#endif /* 63158 */
#else
#define XTM_MAX_MTU_PAYLOAD_SIZE    1500
#endif
#endif /* ! CONFIG_BCM_PON_XRDP */


/* WLAN */

#define WLAN_MAX_MTU_PAYLOAD_SIZE   (1500)

#if defined(CONFIG_BCM_USER_DEFINED_DEFAULT_MTU)
    #if (CONFIG_BCM_USER_DEFINED_DEFAULT_MTU_SIZE > ENET_MAX_MTU_PAYLOAD_SIZE)
    #error "ERROR - CONFIG_BCM_USER_DEFINED_DEFAULT_MTU_SIZE > ENET_MAX_MTU_PAYLOAD_SIZE"
    #endif
    #if (CONFIG_BCM_USER_DEFINED_DEFAULT_MTU_SIZE > XTM_MAX_MTU_PAYLOAD_SIZE)
    #error "ERROR - CONFIG_BCM_USER_DEFINED_DEFAULT_MTU_SIZE > XTM_MAX_MTU_PAYLOAD_SIZE"
    #endif

    #define BCM_ENET_DEFAULT_MTU_SIZE CONFIG_BCM_USER_DEFINED_DEFAULT_MTU_SIZE
    #define BCM_XTM_DEFAULT_MTU_SIZE  CONFIG_BCM_USER_DEFINED_DEFAULT_MTU_SIZE

#else /* !CONFIG_BCM_USER_DEFINED_DEFAULT_MTU */

    #define BCM_ENET_DEFAULT_MTU_SIZE ENET_MAX_MTU_PAYLOAD_SIZE
    #define BCM_XTM_DEFAULT_MTU_SIZE  XTM_MAX_MTU_PAYLOAD_SIZE

#endif /* !CONFIG_BCM_USER_DEFINED_DEFAULT_MTU */

/* select greater of XTM, WLAN, ENET and use it as MAX payload in system
 * for buffer allocation purpose
 */

/*TODO check if compiler is replacing these checks with a final value, if not
 select the MAX payload manually */

#define BCM_MAX_MTU_PAYLOAD_SIZE    \
    GREATER(GREATER(ENET_MAX_MTU_PAYLOAD_SIZE, XTM_MAX_MTU_PAYLOAD_SIZE), WLAN_MAX_MTU_PAYLOAD_SIZE)


/* ############ space needed for L2 header ############ */
#ifndef ENET_MAX_MTU_EXTRA_SIZE
#define ENET_MAX_MTU_EXTRA_SIZE     (32)
/* L2 header can include DA(6), SA(6), ethtype(2), multiple tags(n*4) (vlan, broadcom), FCS(4),
   or PPP header,  reserve 32 bytes */
#endif

/*select greater value*/
#define BCM_MAX_MTU_EXTRA_SIZE    ENET_MAX_MTU_EXTRA_SIZE

/* ############ Total length used for packets ############ */
/* Align this to 64 bytes as Iudma may overwite, some bytes if not 64 byte aligned , bug in iudma*/
/* This ignores the headroom in the BPM buffer otherwise it could be reduced.
 It could be set to 1856 so that the headroom in the BPM buffer would be used. 
 But to keep things simple and safe, max pkt len is assigned to PKTBUFSZ from
 linux_osl.h */
#define BCM_MAX_PKT_LEN             \
    GREATER(2048, BCM_DCACHE_ALIGN(BCM_MAX_MTU_EXTRA_SIZE + BCM_MAX_MTU_PAYLOAD_SIZE))

#if defined(CONFIG_BCM963158)
#define BCM_MAX_PKT_LEN_MTU_BASED   \
    (BCM_MAX_MTU_EXTRA_SIZE + BCM_MAX_MTU_PAYLOAD_SIZE + 4)  // runner + SF2 has 4 byte BRCM Tag
#else
#define BCM_MAX_PKT_LEN_MTU_BASED   \
    (BCM_MAX_MTU_EXTRA_SIZE + BCM_MAX_MTU_PAYLOAD_SIZE)
#endif


#if defined(__KERNEL__)
/**
 * ----------------------------------------------------------------------------
 *  SECTION: BPM Layout buffer offsets and lengths
 *
 *  BPM/MPM buffer Layout:
 *
 *  |BCM_PKT_WKB_BYTES|BCM_PKT_TLM_BYTES|BCM_PKT_FKB_BYTES|BCM_PKT_HEADROOM|BCM_MAX_PKT_LEN|BCM_SKB_TAILROOM|BCM_SKB_SHAREDINFO
 *
 *  ^                 ^                 ^                 ^                ^               ^                ^
 *  |                 |                 |                 |                |               |                |
 *  |                 |                 |                 |                |               |                |
 * pWkb              pTlm              pFkb              pHead            pData           pTail            pShinfo
 *  |                                                     |                |               |                |
 *  |                                                     |                |               |                |
 *  |                                                  skb:head         skb:data        skb:tail         skb:end
 *  |                                                                      |
 *  |                                                                      |
 * pStart                                                                "pBuf"
 *
 * Note:
 * - BPM (SW) pools maintains pBuf pointers (not pStart)
 * ----------------------------------------------------------------------------
 */

#if defined(CONFIG_BCM_WKB)
/* Reserve WKB and Telemetry regions only if WKB is supported */
#define CC_CONFIG_BCM_WKB_COMPAT
#endif /* CONFIG_BCM_WKB */

/* ############ space needed for WKB and Telemetry ############ */
#if defined(CC_CONFIG_BCM_WKB_COMPAT)

/* -------------------------- WKB ----------------------------- */
#define BCM_PKT_WKB_OFFSET          0
#define BCM_PKT_WKB_BYTES           256

/* -------------------------- Telemetry ----------------------- */
#define BCM_PKT_TLM_OFFSET          (BCM_PKT_WKB_OFFSET + BCM_PKT_WKB_BYTES)
#define BCM_PKT_TLM_BYTES           64

#else /* ! CC_CONFIG_BCM_WKB_COMPAT */

/* -------------------------- WKB ----------------------------- */
#define BCM_PKT_WKB_OFFSET          0
#define BCM_PKT_WKB_BYTES           0

/* -------------------------- Telemetry ----------------------- */
#define BCM_PKT_TLM_OFFSET          (BCM_PKT_WKB_OFFSET + BCM_PKT_WKB_BYTES)
#define BCM_PKT_TLM_BYTES           0

#endif /* ! CC_CONFIG_BCM_WKB_COMPAT */


/* ############ space needed for FKB ############ */
#define BCM_PKT_FKB_OFFSET          (BCM_PKT_TLM_OFFSET + BCM_PKT_TLM_BYTES)
#define BCM_PKT_FKB_BYTES           64


/* ############ space needed for headroom in the packet ############ */
#define BCM_PKT_HEADROOM_OFFSET     \
    (BCM_PKT_FKB_OFFSET + BCM_PKT_FKB_BYTES)

/* this headroom is needed for WLAN header for  ENET,XTM,XPON ==> WLAN traffic */
#define WLAN_TX_HEADROOM            208
#define XTM_BONDING_HEADROOM        48
#define GRE_HDR_LEN                 16

/* Headroom is a multiple of cacheline */
#define BCM_PKT_HEADROOM            BCM_DCACHE_ALIGN(WLAN_TX_HEADROOM + GRE_HDR_LEN)

/* Total Headroom Reserved from buffer Start to Payload */
#define BCM_PKT_HEADROOM_RSV        (BCM_PKT_HEADROOM_OFFSET + BCM_PKT_HEADROOM)


/* ############ space needed for Packet Payload ############ */
#define BCM_PKT_PYLD_OFFSET         (BCM_PKT_HEADROOM_OFFSET + BCM_PKT_HEADROOM)
#define BCM_PKT_PYLD_BYTES          BCM_MAX_PKT_LEN


/* ############ space needed for Tailroom in the packet ############ */
#define BCM_PKT_TAILROOM_OFFSET     (BCM_PKT_PYLD_OFFSET + BCM_PKT_PYLD_BYTES)
/* ----------- space needed for skb tailroom ---------- */
#define BCM_SKB_TAILROOM            BCM_DCACHE_ALIGN(32)

/* ----------- space needed for skb_shared_info ---------- */
#define BCM_SKB_END_OFFSET          (BCM_PKT_TAILROOM_OFFSET + BCM_SKB_TAILROOM)
#define BCM_SKB_SHAREDINFO          BCM_DCACHE_ALIGN(sizeof(struct skb_shared_info))

#define BCM_PKT_TAILROOM            (BCM_SKB_TAILROOM + BCM_SKB_SHAREDINFO)


/* ### Toatal buf size i.e BCM_PKT_PYLD_BYTES + metadata(fkb,skb_sharedinfo etc..) ### */
#define BCM_PKTBUF_SIZE             (BCM_PKT_WKB_BYTES +        \
                                    BCM_PKT_FKB_BYTES +         \
                                    BCM_PKT_TLM_BYTES +         \
                                    BCM_PKT_HEADROOM +          \
                                    BCM_PKT_PYLD_BYTES +        \
                                    BCM_PKT_TAILROOM)

/* pStart to pXyz conversion */
#define BCM_PSTART_TO_PWKBUFF(start)    ((wkbuff_t *)((uint8_t *)(start) + BCM_PKT_WKB_OFFSET))
#define BCM_PWKBUFF_TO_PSTART(pWkb)     ((uint8_t *)(pWkb) - BCM_PKT_WKB_OFFSET)

#define BCM_PSTART_TO_PTLM(start)       ((uint8_t *)(start) + BCM_PKT_TLM_OFFSET)
#define BCM_PTLM_TO_PSTART(pTlm)        ((uint8_t *)(pTlm) - BCM_PKT_TLM_OFFSET)

#define BCM_PSTART_TO_PFKBUFF(start)    ((FkBuff_t *)((uint8_t *)(start) + BCM_PKT_FKB_OFFSET))
#define BCM_PFKBUFF_TO_PSTART(pFkb)     ((uint8_t *)(pFkb) - BCM_PKT_FKB_OFFSET)

#define BCM_PSTART_TO_PHEAD(start)      ((uint8_t *)(start) + BCM_PKT_HEADROOM_OFFSET)
#define BCM_PHEAD_TO_PSTART(head)       ((uint8_t *)(head) - BCM_PKT_HEADROOM_OFFSET)

#define BCM_PSTART_TO_PDATA(start)      ((uint8_t *)(start) + BCM_PKT_PYLD_OFFSET)
#define BCM_PDATA_TO_PSTART(data)       ((uint8_t *)(data) - BCM_PKT_PYLD_OFFSET)

/* SHINFO/skb::end offset is only valid for fixed size buffer allocated from SW BPM.
 * HW (MPM) buffer can have variable payload
 */
#define BCM_PSTART_TO_PSHINFO(start)    \
        ((struct skb_shared_info *)((uint8_t *)(start) + BCM_SKB_END_OFFSET))
#define BCM_PSHINFO_TO_PSTART(shinfo)   ((uint8_t *)(shinfo) - BCM_SKB_END_OFFSET)

/* All other sub-regions offsets will be adjusted wrt pStart */
/* Eg. pHead to pData = pHead to pStart to pData */

#define BCM_PWKBUFF_TO_PHEAD(pWkb)      BCM_PSTART_TO_PHEAD(BCM_PWKBUFF_TO_PSTART(pWkb))
#define BCM_PHEAD_TO_PWKBUFF(head)      BCM_PSTART_TO_PWKBUFF(BCM_PHEAD_TO_PSTART(head))

#define BCM_PWKBUFF_TO_PDATA(pWkb)      BCM_PSTART_TO_PDATA(BCM_PWKBUFF_TO_PSTART(pWkb))
#define BCM_PDATA_TO_PWKBUFF(data)      BCM_PSTART_TO_PWKBUFF(BCM_PDATA_TO_PSTART(data))

#define BCM_PWKBUFF_TO_PSHINFO(pWkb)    BCM_PSTART_TO_PSHINFO(BCM_PWKBUFF_TO_PSTART(pWkb))
#define BCM_PSHINFO_TO_PWKBUFF(shinfo)  BCM_PSTART_TO_PWKBUFF(BCM_PSHINFO_TO_PSTART(shinfo))

#define BCM_PFKBUFF_TO_PHEAD(pFkb)      BCM_PSTART_TO_PHEAD(BCM_PFKBUFF_TO_PSTART(pFkb))
#define BCM_PHEAD_TO_PFKBUFF(data)      BCM_PSTART_TO_PFKBUFF(BCM_PHEAD_TO_PSTART(data))

#define BCM_PFKBUFF_TO_PDATA(pFkb)      BCM_PSTART_TO_PDATA(BCM_PFKBUFF_TO_PSTART(pFkb))
#define BCM_PDATA_TO_PFKBUFF(data)      BCM_PSTART_TO_PFKBUFF(BCM_PDATA_TO_PSTART(data))

#define BCM_PHEAD_TO_PDATA(head)        BCM_PSTART_TO_PDATA(BCM_PHEAD_TO_PSTART(head))
#define BCM_PDATA_TO_PHEAD(data)        BCM_PSTART_TO_PHEAD(BCM_PDATA_TO_PSTART(data))

#define BCM_PDATA_TO_PSHINFO(data)      BCM_PSTART_TO_PSHINFO(BCM_PDATA_TO_PSTART(data))
#define BCM_PSHINFO_TO_PDATA(shinfo)    BCM_PSTART_TO_PDATA(BCM_PSHINFO_TO_PSTART(shinfo))

#endif /* __KERNEL__ */

/* XXX: TODO : Consolidate all offset MACROS from bpm.h and replace them with BCM_Pxxx */

/* Constant offset of skb_shared_info from pData in a BPM managed buffer */
#define BPM_BUF_TO_END_OFFSET           \
    BCM_DCACHE_ALIGN(BCM_MAX_PKT_LEN + BCM_SKB_TAILROOM)


/* ############ other common defines ############ */

#define ENET_MIN_MTU_SIZE               60            /* Without FCS */
#define ENET_MIN_MTU_SIZE_EXT_SWITCH    64            /* Without FCS */
#define ENET_MAX_MTU_SIZE               (ENET_MAX_MTU_PAYLOAD_SIZE + ENET_MAX_MTU_EXTRA_SIZE)

#if defined(__KERNEL__)
#define BCM_SKB_ALIGNED_SIZE            ((sizeof(struct sk_buff) + 0x0f) & ~0x0f)
#endif

#endif /* __BCM_PKT_LENGTHS_H__ */
