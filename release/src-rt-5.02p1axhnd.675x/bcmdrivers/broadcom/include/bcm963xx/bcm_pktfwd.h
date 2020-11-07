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

/**
 * =============================================================================
 *
 * Section: Abstract Data Types (ADTs) used in BCM Packet Forwarding Library
 *
 * Shared Library to support the forwarding of 802.3 packets between two
 * networking devices. A cache of the native operating stack is maintained using
 * a dictionary abstract data type. Packets are forwarded between an ingress and
 * egress driver using a single linked list of packets, accumulated using a
 * PKTLIST ADT.
 *
 * A 802.3 Mac Address Lookup Table is abstracted in D3LUT using a <sym,key>
 * dictionary.
 *
 * A D3LUT entry identifies an object that describes a LAN forwarding device or
 * a WLAN forwarding device (D3FWD_WLIF).
 *
 * =============================================================================
 */

#if !defined(__bcm_pktfwd_h_included__)
#define      __bcm_pktfwd_h_included__

#if defined(BCM_PKTFWD)

#include <linux/kernel.h>
#include <linux/brcm_dll.h>
#if defined(CONFIG_BLOG)
#include <linux/blog.h>
#endif /* CONFIG_BLOG */
#include <linux/nbuff.h>

#include "bcm_prefetch.h"
#include "bcm_assert.h"

/**
 * -----------------------------------------------------------------------------
 * Section : Macros used in BCM_PKTFWD Namespace
 * -----------------------------------------------------------------------------
 */
#define PKTFWD_NOOP                 do { /* no-op */ } while (0)

/** Do not include wlan bcmdefs.h for BCM_BIT_MANIP_MACROS */
#define PKTFWD_GBF(val, NAME)       (((val) & NAME##_MASK) >> NAME##_SHIFT)
#define PKTFWD_CBF(val, NAME)       ((val) & ~NAME##_MASK)
#define PKTFWD_SBF(val, NAME)       (((val) << NAME##_SHIFT) & NAME##_MASK)
#define PKTFWD_GBIT(val, NAME)      (((val) & NAME##_MASK) >> NAME##_SHIFT)
#define PKTFWD_CBIT(val, NAME)      ((val) & ~NAME##_MASK)
#define PKTFWD_SBIT(NAME)           (1 << NAME##_SHIFT)

/** Error/Warn/Debug Control */
#define CC_PKTFWD_DEBUG             (0)

/** Errors always enabled ... avoid using PRINT, use TRACE or PTRACE */
#define PKTFWD_ERROR(fmt, arg...) \
    printk(CLRr "%s: " fmt CLRnl, __FUNCTION__, ##arg)
#define PKTFWD_PRINT(fmt, arg...) \
    printk(CLRg "%s: " fmt CLRnl, __FUNCTION__, ##arg)

/** Assert and Warnings enabled at level 1 and above */
#if (CC_PKTFWD_DEBUG >= 1)
#define PKTFWD_ASSERT(exp)          BCM_ASSERT_A(exp)
#define PKTFWD_WARN(fmt, arg...) \
    printk(CLRm "%s: " fmt CLRnl, __FUNCTION__, ##arg)
#else
#define PKTFWD_ASSERT(exp)          PKTFWD_NOOP
#define PKTFWD_WARN(fmt, arg...)    PKTFWD_NOOP
#endif /* CC_PKTFWD_DEBUG < 1 */

/** Function entry and Trace enabled at level 2 and above */
#if (CC_PKTFWD_DEBUG >= 2)
#define PKTFWD_FUNC()               printk(CLRb "%s" CLRnl, __FUNCTION__)
#define PKTFWD_TRACE(fmt, arg...) \
    printk(CLRb "%s: " fmt CLRnl, __FUNCTION__, ##arg)
#else
#define PKTFWD_FUNC()               PKTFWD_NOOP
#define PKTFWD_TRACE(fmt, arg...)   PKTFWD_NOOP
#endif /* CC_PKTFWD_DEBUG < 1 */

/** Per "P"acket Function entry and Trace enabled at level 3 and above */
#if (CC_PKTFWD_DEBUG >= 3)
#define PKTFWD_PFUNC()              printk(CLRb "%s" CLRnl, __FUNCTION__)
#define PKTFWD_PTRACE(fmt, arg...) \
    printk(CLRc "%s: " fmt CLRnl, __FUNCTION__, ##arg)
#else
#define PKTFWD_PFUNC()              PKTFWD_NOOP
#define PKTFWD_PTRACE(fmt, arg...)  PKTFWD_NOOP
#endif /* CC_PKTFWD_DEBUG < 3 */


/**
 * -----------------------------------------------------------------------------
 *
 * Section: Packet Forwarding Domains
 *
 * PKTFWD_DOMAINS_WLAN defines number of WLAN radio instances.
 * PKTFWD_DOMAINS_TOT = N WLAN Radio Domain(s) and 1 LAN|WAN Domain
 * Domain indexes [0..PKTFWD_DOMAINS_WLAN) are WLAN radio domains
 * Last domain index PKTFWD_XDOMAIN_IDX is a non-WLAN domain (LAN or WAN)
 *
 * Within a domain, the space [0 .. PKTFWD_ENDPOINTS_MAX) lists the individual
 * endpoints (being WLAN stations or LAN devices) represented by a 802.3 Addr.
 *
 * Use Global-space to per-Domain-space conversion macros.
 *
 * Nomenclature: XYZ_TOT refers to global space, XYZ_MAX refers to domain space
 *
 * -----------------------------------------------------------------------------
 */

/** WLAN Radio Domains supports a max of 3 Radios */
#define PKTFWD_DOMAINS_WLAN         (3)

/** Non-WLAN Radio domain is last domain: (e.g. domain of all LAN endpoints) */
#define PKTFWD_DOMAINS_TOT          (PKTFWD_DOMAINS_WLAN + 1)
#define PKTFWD_XDOMAIN_IDX          (PKTFWD_DOMAINS_TOT - 1) /* last */

/** WLAN Interfaces (Network Devices) per Radio (Domain) */
#define PKTFWD_DEVICES_MAX          (16)
#define PKTFWD_DEVICES_TOT          (PKTFWD_DOMAINS_WLAN * PKTFWD_DEVICES_MAX)


/**
 * Total number of 802.3 Endpoints within a Domain : MUST BE A POWER OF 2
 *
 * In WLAN domain, an endpoint is a WLAN station.
 * The last domain (XDOMAIN) will likewise carry PKTFWD_ENDPOINTS_MAX
 * endpoints for LAN/WAN.
 *
 * A Global endpoint (12b key index) may be converted to per domain endpoint
 *
 * All global endpoints may be managed in a system pool necessitating the
 * domain index and per endpoint index to be contiguous, for indexed access.
 *
 * System global endpoint index uses a 12b key::index
 * See pktlist_elem_t::key and d3lut_key_t
 */
#if defined(CONFIG_BCM_HND_EAP)

#define PKTFWD_ENDPOINTS_MAX        (512) /* per domain */
#define PKTFWD_ENDPOINTS_MASK       (0x01FF)
#define PKTFWD_ENDPOINTS_SHIFT      (0)

#define PKTFWD_DOMAIN_MASK          (0x0600)
#define PKTFWD_DOMAIN_SHIFT         (9)

#else

#define PKTFWD_ENDPOINTS_MAX        (128)
#define PKTFWD_ENDPOINTS_MASK       (0x007F)
#define PKTFWD_ENDPOINTS_SHIFT      (0)

#define PKTFWD_DOMAIN_MASK          (0x0180)
#define PKTFWD_DOMAIN_SHIFT         (7)

#endif /* ! CONFIG_BCM_HND_EAP */

/** Used to extract per domain endpoint index, compose global endpoint index */

/** Total number of WLAN endpoints across all WLAN radios */
#define PKTFWD_ENDPOINTS_WLAN       (PKTFWD_DOMAINS_WLAN * PKTFWD_ENDPOINTS_MAX)

/** Total number of endpoints across all domains (global space) */
#define PKTFWD_ENDPOINTS_TOT        (PKTFWD_DOMAINS_TOT * PKTFWD_ENDPOINTS_MAX)


/** Global space to/from per domain-space conversion macros */

/** Given a global space endpoint index, fetch the per domain endpoint index */
#define PKTFWD_DOMAIN_ENDPOINT_IDX(gbl_endpoint_idx) \
    PKTFWD_GBF((gbl_endpoint_idx), PKTFWD_ENDPOINTS)

/** Given a global space endpoint index, fetch the domain index */
#define PKTFWD_ENDPOINT_DOMAIN_IDX(gbl_endpoint_idx) \
    PKTFWD_GBF((gbl_endpoint_idx), PKTFWD_DOMAIN)

/** Given a per domain endpoint index, convert to global enpoint index */
#define PKTFWD_GBL_ENDPOINT_IDX(domain_idx, endpoint_idx) \
    (PKTFWD_SBF((endpoint_idx), PKTFWD_ENDPOINTS) + PKTFWD_SBF((domain_idx), PKTFWD_DOMAIN))

/**
 * -----------------------------------------------------------------------------
 *
 * pktfwd_key_t
 *
 * A 16b key is saved within a D3LUT. The 16b key may be passed to a network
 * processor. Network processor will tag the 16b key for matched/processed
 * packets delivered to SW. A packet's 16b tagged key will be used by PKTLIST
 * for binning and transferring pktlists to a destination WLAN radio/interface.
 *
 * See: pktlist_t::key and d3lut_elem_t::key
 *
 * - index  : global scoped endpoint index. 12b restricts to 4K endpoints
 * - incarn : incarnation of a endpoint index allocation (stale key)
 * - domain : domain is also encoded in the global endpoint index.
 *            domain may be used to manage pools of endpoints per domain, wlan
 *            radio unit, or even WLAN Forward Device identification.
 *
 * -----------------------------------------------------------------------------
 */

#if !defined(CONFIG_BCM_WLAN_16BIT_STATION_CHAIN_IDX_SUPPORT)
#error "BCM_PKTFWD requires 16b WLAN flow support"
#endif

typedef union pktfwd_key                /* pktfwd_key_t */
{
    uint16_t        v16;                /* 16 bit key associated with Address */
    struct {
        LE_DECL(
            uint16_t index   : 12;      /* global scoped limit to 4K elements */
            uint16_t incarn  : 2;       /* incarnation incremented on free */
            uint16_t domain  : 2;       /* endpoint domain */
        )
        BE_DECL(
            uint16_t domain  : 2;       /* used as wfd_id - bits 15,14 */
            uint16_t incarn  : 2;
            uint16_t index   : 12;
        )
    };
} pktfwd_key_t;

#define PKTFWD_KEY_INDEX_MASK       (0xFFF)
#define PKTFWD_KEY_INDEX_SHIFT      (0)

#define PKTFWD_KEY_INCARN_MASK      (0x3)
#define PKTFWD_KEY_INCARN_SHIFT     (12)

#define PKTFWD_KEY_DOMAIN_MASK      (0x3)
#define PKTFWD_KEY_DOMAIN_SHIFT     (14)

#define PKTFWD_KEY_MATCH_MASK \
    ( (PKTFWD_KEY_INDEX_MASK  << PKTFWD_KEY_INDEX_SHIFT) \
    | (PKTFWD_KEY_INCARN_MASK << PKTFWD_KEY_INCARN_SHIFT) )

#define PKTFWD_KEY_INVALID          (0xFFFF) /* 16b */
#define PKTFWD_KEY_INDEX_INVALID    (PKTFWD_KEY_INDEX_MASK)


/** Maximum number of packet priorities */
#define PKTFWD_PRIO_MAX             (8)


/**
 * -----------------------------------------------------------------------------
 * Section: BCM_PKTFWD Compatability Control
 * BCM Packet Forwarding Abstract Data Type <version, release, patch>
 * -----------------------------------------------------------------------------
 */

/** 32 bit [ <16 bit version> : <8 bit release> : <8 bit patch> ] */
#define PKTFWD_V(VRP)               ((uint16_t) ((VRP) >> 16))
#define PKTFWD_R(VRP)               ((uint8_t)  ((VRP) >>  8))
#define PKTFWD_P(VRP)               ((uint8_t)  ((VRP) >>  0))

#define PKTFWD_VERSION(version, release, patch) \
    (((version) << 16) + ((release) << 8) + ((patch) << 0))

#define PKTFWD_VERSIONCODE(ADT) \
    PKTFWD_VERSION(ADT ##_VERSION, ADT ##_RELEASE, ADT ##_PATCH)

#define PKTFWD_VERSION_NONE         (0)

/** Version Release Patch: Dot Notation Form */
#define PKTFWD_VRP_FMT              " \033[1m\033[34m%s[%u.%u.%u]\033[0m"
#define PKTFWD_VRP_VAL(NAME, VRP) \
    #NAME, PKTFWD_V(VRP), PKTFWD_R(VRP), PKTFWD_P(VRP)


// -------------------------------- Packet List Transfer ADT
#define BCM_PKTLIST

#if defined(BCM_PKTLIST)
#define PKTLIST_VERSION             (1)
#define PKTLIST_RELEASE             (1)
#define PKTLIST_PATCH               (0)
#define PKTLIST_VERSIONCODE         PKTFWD_VERSIONCODE(PKTLIST)
#else
#define PKTLIST_VERSIONCODE         PKTFWD_VERSION_NONE
#endif  /* ! BCM_PKTLIST */

// -------------------------------- Packet Queue Transfer ADT
#define BCM_PKTQUEUE

#if defined(BCM_PKTQUEUE)
#define PKTQUEUE_VERSION            (1)
#define PKTQUEUE_RELEASE            (0)
#define PKTQUEUE_PATCH              (0)
#define PKTQUEUE_VERSIONCODE        PKTFWD_VERSIONCODE(PKTQUEUE)
#else
#define PKTQUEUE_VERSIONCODE        PKTFWD_VERSION_NONE
#endif  /* ! BCM_PKTQUEUE */

// -------------------------------- WLAN Station/Interface Connection Database
#define BCM_D3FWD

#if defined(BCM_D3FWD)
#define D3FWD_VERSION               (1)
#define D3FWD_RELEASE               (0)
#define D3FWD_PATCH                 (0)
#define D3FWD_VERSIONCODE           PKTFWD_VERSIONCODE(D3FWD)
#else
#define D3FWD_VERSIONCODE           PKTFWD_VERSION_NONE
#endif  /* ! BCM_D3FWD */

// -------------------------------- 802.3 <MacAddress,key> Lookup Table ADT
#define BCM_D3LUT

#if defined(BCM_D3LUT)
#define D3LUT_VERSION               (1)
#define D3LUT_RELEASE               (0)
#define D3LUT_PATCH                 (0)
#define D3LUT_VERSIONCODE           PKTFWD_VERSIONCODE(D3LUT)
#else
#define D3LUT_VERSIONCODE           PKTFWD_VERSION_NONE
#endif  /* ! BCM_D3LUT */


/**
 * BCM_PKTFWD implementation relies on following BCM ADTs.
 * Alternate ADTs may be developed and below compile error relaxed, accordingly.
 */
#if !defined(BCM_PKTLIST) || !defined(BCM_PKTQUEUE) || \
    !defined(BCM_D3FWD) || !defined(BCM_D3LUT)
#error "BCM_PKTFWD requires BCM_PKTLIST BCM_PKTQUEUE BCM_D3FWD and BCM_D3LUT"
#endif /* !BCM_PKTLIST || !BCM_PKTQUEUE || !BCM_D3FWD || !BCM_D3LUT */


/** BCM_PKTFWD return codes */
#define BCM_PKTFWD_FAILURE          (-1)
#define BCM_PKTFWD_SUCCESS          (0)

/** BCM_PKTFWD global subsystem construction/destruction */
extern int  bcm_pktfwd_sys_init(void);
extern void bcm_pktfwd_sys_fini(void);

/*
 * -----------------------------------------------------------------------------
 *  Function : Helper router to fetch and reset first-set interface index from
 *             ssid_vector bitmap.
 * -----------------------------------------------------------------------------
 */
static inline uint16_t
pktfwd_map_ssid_vector_to_ssid(uint16_t * ssid_vector)
{
    uint32_t ssid_index = __ffs(* ssid_vector);

    /* Clear the bit we found */
    (* ssid_vector) &= ~(1 << ssid_index);

    return ssid_index;
}   /* pktfwd_map_ssid_vector_to_ssid() */

/*
 * =============================================================================
 * Section:  pNBuff_t Functional Interface
 *
 * Impl Caveat: Inlined functions and constant NBuffPtrType_t allows for compile
 *              time resolution of SKB vs FKB type.
 *              Default case will never be compiled.
 * =============================================================================
 */

/**
 * -----------------------------------------------------------------------------
 * Function : Helper routine to get packet link pointer based on Buffer type
 * CAUTION: pktfwd_pktsll and PKTLINK (used by CFP) must match
 * -----------------------------------------------------------------------------
 */

static inline void *
pktfwd_pktsll(void * pkt, const NBuffPtrType_t NBuffPtrType)
{
    void * pktsll;

    switch (NBuffPtrType)
    {
        case SKBUFF_PTR:
             pktsll = ((struct sk_buff *)pkt)->prev;
             break;

        case FKBUFF_PTR:
            pktsll = ((FkBuff_t *)PNBUFF_2_PBUF(pkt))->queue;
            break;

        default:
            if (IS_SKBUFF_PTR(pkt))
                pktsll = ((struct sk_buff *)pkt)->prev;
            else
                pktsll = ((FkBuff_t *)PNBUFF_2_PBUF(pkt))->queue;
            break;
    }

    return pktsll;
}   /* pktfwd_pktsll() */

/**
 * -----------------------------------------------------------------------------
 * Function : Helper routine to set packet link pointer based on Buffer type
 * -----------------------------------------------------------------------------
 */
static inline void
pktfwd_pktsetsll(void * pkt, void * pktsll, const NBuffPtrType_t NBuffPtrType)
{
    switch (NBuffPtrType)
    {
        case SKBUFF_PTR:
             ((struct sk_buff *) pkt)->prev = pktsll;
             break;

        case FKBUFF_PTR:
            ((FkBuff_t *) PNBUFF_2_PBUF(pkt))->queue = pktsll;
            break;

        default:
            if (IS_SKBUFF_PTR(pkt))
                ((struct sk_buff *) pkt)->prev = pktsll;
            else
                ((FkBuff_t *) PNBUFF_2_PBUF(pkt))->queue = pktsll;
            break;
    }
}   /* pktfwd_pktsetsll() */

/**
 * -----------------------------------------------------------------------------
 * Function : Helper routine to free packet
 * -----------------------------------------------------------------------------
 */
static inline void
pktfwd_pktfree(void * pkt)
{
    nbuff_free(pkt);
}   /* pktfwd_pktfree() */

/**
 * -----------------------------------------------------------------------------
 * Function : Helper routine to get packet key
 * -----------------------------------------------------------------------------
 */

static inline uint16_t
pktfwd_pktlist_pktkey(void * pkt, const NBuffPtrType_t NBuffPtrType)
{
    uint16_t pktkey;

    switch (NBuffPtrType)
    {
        case SKBUFF_PTR:
            pktkey = ((struct sk_buff *) pkt)->wl.ucast.nic.wl_chainidx;
            break;

        case FKBUFF_PTR:
            pktkey = ((FkBuff_t *) PNBUFF_2_PBUF(pkt))->wl.ucast.dhd.flowring_idx;
            break;

        default:
            if (IS_SKBUFF_PTR(pkt))
                pktkey = ((struct sk_buff *) pkt)->wl.ucast.nic.wl_chainidx;
            else
                pktkey = ((FkBuff_t *) PNBUFF_2_PBUF(pkt))->wl.ucast.dhd.flowring_idx;
            break;
    }

    return pktkey;
}   /* pktfwd_pktlist_pktkey() */

/**
 * -----------------------------------------------------------------------------
 * Function : Helper routine to get packet pktfwd_key
 * -----------------------------------------------------------------------------
 */

static inline uint16_t
pktfwd_pktqueue_pktkey(void * pkt, const NBuffPtrType_t NBuffPtrType)
{
    uint16_t pktkey;

    switch (NBuffPtrType)
    {
        case SKBUFF_PTR:
            pktkey = ((struct sk_buff *) pkt)->wl.pktfwd.pktfwd_key;
            break;

        case FKBUFF_PTR:
            pktkey = ((FkBuff_t *) PNBUFF_2_PBUF(pkt))->wl.pktfwd.pktfwd_key;
            break;

        default:
            if (IS_SKBUFF_PTR(pkt))
                pktkey = ((struct sk_buff *) pkt)->wl.pktfwd.pktfwd_key;
            else
                pktkey = ((FkBuff_t *) PNBUFF_2_PBUF(pkt))->wl.pktfwd.pktfwd_key;
            break;
    }

    return pktkey;
}   /* pktfwd_pktqueue_pktkey() */


#if defined(BCM_PKTLIST)

/**
 * =============================================================================
 *
 * Section: PKTLIST Abstract Data Type
 *
 * Packet List management library between an ingress and egress network device.
 * Used in the WFD (WLAN Forwarding Driver) to WLAN device driver transfer.
 *
 * WFD driver manages the Rx Queues serviced by the embedded network processor.
 * Rx Queues (implemented as Rx Descriptor rings) are populated with empty
 * buffers from either a shared buffer pool manager, or from a ring that feeds
 * buffers to the network processor. Packets arriving into the Rx Queues, are
 * specifically for a WLAN radio hosted on the CPU core to which the Rx Queue
 * interrupt is mapped (Radio per CPU core affinity model, in a multi radio
 * multi CPU core configuration). Every received packet will include a unique
 * ID that identifies a bridged connection to a destination WLAN station. In
 * a multi WLAN station configuration, packets arriving in the Rx Queue may be
 * interleaved with respect to the destination station.
 *
 * Pktlist infrastructure is used by WFD to sort packets by destination station
 * and priority, into single linked lists of packets. Pktlists are handed by
 * the WFD thread to the WLAN thread, with Pktlists joining to form longer
 * trains of packets in the destination, if a previous pktlist is pending a
 * dispatch through the WLAN protocol stack, in the WLAN thread.
 *
 * Pktlist abstraction is agnostic of the logic related to allocation and the
 * assignment of bridged ID to identify a destination. A multicast packet will
 * use a unique ID assigned by the packet list abstraction.
 *
 * Theory of Operation:
 * --------------------
 *
 * A table of packet lists are maintained in the ingress and egress driver.
 * Packets arriving at ingress are directly added to a packet list identified
 * by the packet priority and a destination ID. The packet list is then placed
 * onto a pending work list. Work lists are maintained for a mcast and by ucast.
 *
 * After all (or bounded by a budget) incoming packets are added to packet
 * lists, the pending packet lists queued in the work list may be handed off to
 * the peer egress packet list.
 *
 * Packet list transfer to a peer egress driver's packet list context must
 * ensure that a lock is taken. The pending work list of pktlists in ingress
 * driver are transferred to the peer egress driver, using a fast list join.
 * The peer's pktlist is placed on the corresponding peer's work list.
 * A peer's packet list need not be in the peer's pktlist context's free or work
 * list and may reside in the peer interface's work list awaiting dispatch.
 *
 * As new packets arrive, and get dispatched to the peer, the peer's list will
 * grow, until the peer egress driver is ready to process a pktlist. Peer
 * dispatches an entire packet list into its datapath.
 *
 * Implementation Caveats:
 *
 * Packet List context maintained in source (ingress) and destination (egress)
 * driver. Ingress addition of pkts to packet lists is not protected by lock
 * limiting daisy chaining of a peer to another downstream peer.
 *
 * A pktlist_context is instantiated per pair of <ingress, egress> driver.
 * Ingress driver accumulates pktlists into local pktlist_context and then
 * hands off to the peer pktlist_context using the attached "xfer" handler.
 *
 * Egress pktlist_context need only have a "xfer" handler and a peer to effect
 * a daisy chaining to yet a third driver. Daisy chaining is not supported, as
 * ingress driver does not take explicit spinlock on local pktlist_context.
 *
 * CC_PKTLIST_PRIO: Conditional compile to maintain unicast packet lists pending
 *                  for dispatch by priority.
 *
 * Version History
 *      1.0.0   :   Initial Version
 *
 * =============================================================================
 */


/** PKTLIST Conditional compiles */
#define CC_PKTLIST_DEBUG            (0)
// #define CC_PKTLIST_STATS

/** PKTLIST return codes */
#define PKTLIST_FAILURE             (-1)
#define PKTLIST_SUCCESS             (0)


/**
 * -----------------------------------------------------------------------------
 * Section: PKTLIST Dimensioning
 * Per Radio Unicast MacAddress space (i.e. WLAN destination station
 * -----------------------------------------------------------------------------
 */

#define PKTLIST_UCAST_MAX           (PKTFWD_ENDPOINTS_MAX)
#define PKTLIST_MCAST_MAX           (1)

#define PKTLIST_MCAST_ELEM          (PKTLIST_UCAST_MAX)
#define PKTLIST_DEST_MAX            (PKTLIST_UCAST_MAX + PKTLIST_MCAST_MAX)

#define PKTLIST_PRIO_MAX            (PKTFWD_PRIO_MAX)
#define PKTLIST_NODES_MAX           (PKTLIST_PRIO_MAX * PKTLIST_DEST_MAX)

/** Given a 12b (or 16b) key index, fetch the per pool element index */
#define PKTLIST_DEST(key)           PKTFWD_DOMAIN_ENDPOINT_IDX(key)

/**
 * -----------------------------------------------------------------------------
 * Section: PKTLIST Debug Support
 * -----------------------------------------------------------------------------
 */

#define PKTLIST_NOOP                PKTFWD_NOOP

/** Errors always enabled ... avoid using PRINT, use TRACE or PTRACE */
#define PKTLIST_ERROR(fmt, arg...) \
    printk(CLRr "%s: " fmt CLRnl, __FUNCTION__, ##arg)
#define PKTLIST_PRINT(fmt, arg...) \
    printk(CLRg "%s: " fmt CLRnl, __FUNCTION__, ##arg)

/** Assert and Warnings enabled at level 1 and above */
#if (CC_PKTLIST_DEBUG >= 1)
#define PKTLIST_ASSERT(exp)         BCM_ASSERT_A(exp)
#define PKTLIST_WARN(fmt, arg...) \
    printk(CLRm "%s: " fmt CLRnl, __FUNCTION__, ##arg)
#else
#define PKTLIST_ASSERT(exp)         PKTLIST_NOOP
#define PKTLIST_WARN(fmt, arg...)   PKTLIST_NOOP
#endif  /* CC_PKTLIST_DEBUG < 1 */

/** Function entry and Trace enabled at level 2 and above */
#if (CC_PKTLIST_DEBUG >= 2)
#define PKTLIST_FUNC()              printk(CLRb "%s" CLRnl, __FUNCTION__)
#define PKTLIST_TRACE(fmt, arg...) \
    printk(CLRb "%s: " fmt CLRnl, __FUNCTION__, ##arg)
#else
#define PKTLIST_FUNC()              PKTLIST_NOOP
#define PKTLIST_TRACE(fmt, arg...)  PKTLIST_NOOP
#endif  /* CC_PKTLIST_DEBUG < 2 */

/** Per "P"acket Function entry and Trace enabled at level 3 and above */
#if (CC_PKTLIST_DEBUG >= 3)
#define PKTLIST_PFUNC()             printk(CLRb "%s" CLRnl, __FUNCTION__)
#define PKTLIST_PTRACE(fmt, arg...) \
    printk(CLRb "%s: " fmt CLRnl, __FUNCTION__, ##arg)
#else
#define PKTLIST_PFUNC()             PKTLIST_NOOP
#define PKTLIST_PTRACE(fmt, arg...) PKTLIST_NOOP
#endif /* CC_PKTLIST_DEBUG < 3 */

#define PKTLIST_CTX_NAME_SZ         (8)

/** Formating */
#define PKTLIST_FMT                 " %4u %4u %04x %7u"
#define PKTLIST_VAL(p)              (p)->dest, (p)->prio, (p)->key.v16, (p)->len

/** Design optimization, override during debug. */
#if defined(CC_PKTLIST_DEBUG)
#define PKTLIST_RESET(pktlist_p) \
({  \
    (pktlist_p)->head    = PKTLIST_PKT_NULL; \
    (pktlist_p)->tail    = PKTLIST_PKT_NULL; \
    (pktlist_p)->len     = 0U; \
    (pktlist_p)->key.v16 = ~0; \
})

#else

#define PKTLIST_RESET(pktlist_p) \
({ \
    (pktlist_p)->len     = 0U; \
    (pktlist_p)->key.v16 = ~0; \
}) /* Do not need to reset <head,tail,key> just len. */
#endif /* ! CC_PKTLIST_DEBUG */


/**
 * -----------------------------------------------------------------------------
 * Section: PKTLIST Statistics collection for design
 * -----------------------------------------------------------------------------
 */

#if defined(CC_PKTLIST_STATS)
#define PKTLIST_STATS_ADD(v32, val) ((v32) += (val))
#else
#define PKTLIST_STATS_ADD(v32, val) PKTFWD_NOOP
#endif /* ! CC_PKTLIST_STATS */


/**
 * -----------------------------------------------------------------------------
 * Section: PKTLIST Context level Locking/Unlocking
 * Locking/Unlocking is done within driver, and not in the pktlist primitives
 * -----------------------------------------------------------------------------
 */

#if defined(CONFIG_SMP) || defined(CONFIG_PREEMPT)
#define PKTLIST_LOCK(_c)            spin_lock_bh(&((_c)->lock))
#define PKTLIST_UNLK(_c)            spin_unlock_bh(&((_c)->lock))
#else
#define PKTLIST_LOCK(_c)            local_irq_disable()
#define PKTLIST_UNLK(_c)            local_irq_enable()
#endif  /* ! (CONFIG_SMP || CONFIG_PREEMPT) */


/**
 * -----------------------------------------------------------------------------
 * Section: PKTLIST Namespaces: Objects forward declarations and definitions
 * -----------------------------------------------------------------------------
 */

struct  pktlist;                    /* single linked list of packets */
struct  pktlist_elem;               /* pktlist with a dll for xfers */
struct  pktlist_table;              /* 2-d array of pktlist_elem */
struct  pktlist_context;            /* binning and xfer of pktlist */

/* Same as Nbuff_t of struct sk_buff or struct fkbuff */
typedef void                        pktlist_pkt_t;

typedef struct  pktlist             pktlist_t;
typedef struct  pktlist_elem        pktlist_elem_t;
typedef struct  pktlist_table       pktlist_table_t;
typedef struct  pktlist_context     pktlist_context_t;

#define PKTLIST_NULL                ((pktlist_t *)         NULL)
#define PKTLIST_PKT_NULL            ((pktlist_pkt_t *)     NULL)
#define PKTLIST_ELEM_NULL           ((pktlist_elem_t *)    NULL)
#define PKTLIST_TABLE_NULL          ((pktlist_table_t *)   NULL)
#define PKTLIST_CONTEXT_NULL        ((pktlist_context_t *) NULL)


/**
 * -----------------------------------------------------------------------------
 * Section: PKTLIST Conversion and Table Accessor Macros
 * -----------------------------------------------------------------------------
 */

/** Network buff's are linked into a single linked list using nbuff::queue or
 *  sk_buff::prev field based on buf type.
 */
/** CAUTION: PKTLIST_PKT_SLL and PKTLINK (used by CFP) must match */
#define PKTLIST_PKT_SLL(pkt, NBuffPtrType)              \
                pktfwd_pktsll((pkt), (NBuffPtrType))

#define PKTLIST_PKT_SET_SLL(pkt, link, NBuffPtrType)    \
                pktfwd_pktsetsll((pkt), (link), (NBuffPtrType))

#define PKTLIST_PKT_KEY(pkt, NBuffPtrType)              \
                pktfwd_pktlist_pktkey((pkt), (NBuffPtrType))

#define PKTLIST_PKT_FREE(pkt)               pktfwd_pktfree((pkt))

/** PKTLIST conversion/accessor macros to locate a pktlist_elem_t */
#define PKTLIST_CTX_ELEM(ctx, prio, dest) \
    ((pktlist_elem_t *)&(((pktlist_context_t *)(ctx))->table->elem[prio][dest]))

#define PKTLIST_TBL_ELEM(tbl, prio, dest) \
    ((pktlist_elem_t *)&(((pktlist_table_t *)(tbl))->elem[prio][dest]))


#if defined(BCM_PKTFWD_FLCTL)

/**
 * -----------------------------------------------------------------------------
 * Section: PKTFWD_FLCTL Namespaces: Objects forward declarations and definitions
 * -----------------------------------------------------------------------------
 */

struct  pktlist_fctable_t;          /* 2-d array of packet admission credits */

typedef atomic_t                    pktlist_credits_t;
typedef struct  pktlist_fctable     pktlist_fctable_t;

#define PKTLIST_FCTABLE_NULL        ((pktlist_fctable_t *) NULL)


/** PKTFWD_FLCTL conversion/accessor macros to locate a pktlist_credits_t */
#define PKTLIST_CTX_CREDITS(ctx, prio, dest) \
    ((pktlist_credits_t *)&(((pktlist_context_t *)(ctx))->fctable->credits[prio][dest]))

#define PKTLIST_FCTBL_CREDITS(fctbl, prio, dest) \
    ((pktlist_credits_t *)&(((pktlist_table_t *)(fctbl))->credits[prio][dest]))


/**
 * -----------------------------------------------------------------------------
 * pktlist_fctable_t
 * Table of station credits maintained as a two-dimensional array for indexed
 * access to a pktlist_credits_t (given a <prio,dest>)
 *
 * During init, consumer (WLAN) driver instantiate a pktlist_fctable in its
 * pktlist_context_t and the fctable is shared with the producer (WFD) driver.
 * When registering a station with PKTFWD, WLAN configures the corresponding
 * station credits with the station queue length value (SCB prec queue length).
 * Based on credit availability, WFD driver will either admit the packet to
 * pktlist and decrement credit or drop the packet.
 * WLAN driver will increment the credits when a packet is released from
 * precedence queues.
 *
 * NOTE: Used only for WLAN NIC radio
 * -----------------------------------------------------------------------------
 */

struct pktlist_fctable               /* pktlist_fctable_t */
{
    pktlist_credits_t   credits[PKTLIST_PRIO_MAX][PKTLIST_DEST_MAX];
    uint32_t            pkt_prio_favor; /* favor pkt prio; defalut VI, VO */
};

#endif /* BCM_PKTFWD_FLCTL */


/**
 * -----------------------------------------------------------------------------
 * Section: PKTLIST Modules and registration of exported functions
 * -----------------------------------------------------------------------------
 */

typedef enum pktlist_context_module
{
    PKTLIST_CONTEXT_MODULE_WFD  = 0,
    PKTLIST_CONTEXT_MODULE_WLAN = 1,
    PKTLIST_CONTEXT_MODULE_MAX
} pktlist_context_module_t;


/**
 * -----------------------------------------------------------------------------
 * Transfer all pktlists (maintained by prio, dest) to the peer driver's
 * pktlist_context using the pktlist_context_xfer_fn_t handler in the source
 * driver.
 * On Init, the peer's pktlist transfer handler and peer's pktlist context will
 * be saved in the producer's (source driver's) pktlist context.
 *
 * This allows a daisy chaining of transfers.
 * -----------------------------------------------------------------------------
 */

#define PKTFWD_KEYMAP_K2F       (1)     /* PKTFWD key to flowid map */
#define PKTFWD_KEYMAP_F2K       (0)     /* flowid to PKTFWD key map */

typedef void (* pktlist_context_xfer_fn_t)(pktlist_context_t * pktlist_context);
typedef int  (* pktlist_context_keymap_fn_t)(uint32_t radio_idx,
    uint16_t * key, uint16_t * flowid, uint16_t prio, bool k2f);

#define PKTLIST_CONTEXT_PEER_NULL   ((pktlist_context_t *)       NULL)
#define PKTLIST_CONTEXT_XFER_NULL   ((pktlist_context_xfer_fn_t) NULL)
#define PKTLIST_CONTEXT_KEYMAP_NULL ((pktlist_context_keymap_fn_t) NULL)

extern  pktlist_context_t *
               pktlist_context_init(
                    pktlist_context_t *pktlist_context_peer,
                    pktlist_context_xfer_fn_t pktlist_context_xfer_fn,
                    pktlist_context_keymap_fn_t pktlist_context_keymap_fn,
                    void * driver, const char * driver_name, uint32_t unit);

extern  pktlist_context_t *
               pktlist_context_fini(pktlist_context_t * pktlist_context);

extern  void   pktlist_context_dump(pktlist_context_t * context,
                    bool dump_peer, bool dump_verbose);

/** Private to bcm_pktfwd.c and hosting module, to dump all instances */
extern  void   pktlist_context_dump_all(void);


/**
 * =============================================================================
 * Section: PKTLIST Core Structures
 * =============================================================================
 */


/**
 * -----------------------------------------------------------------------------
 * pktlist_t
 *
 * Linked list of packets. PKTLIST_PKT_SLL(pkt) uses the sk_buff::prev pointer
 * or nbuff::queue buff_queue to form a single linked list of packets having a
 * common attribute <prio,dest>
 *
 * A D3LUT element's 16bit key is transparently passed to each packet, and is
 * composed of a 2b domain (radio unit), a 2b incarnation, and a 12b global
 * scoped endpoint index.
 *
 * Packets in a pktlist_t will be processed after ensuring that the sll of
 * packets have the same incarnation (i.e. no stale packets).
 *
 * -----------------------------------------------------------------------------
 */

struct pktlist                      /* pktlist_t */
{
    pktlist_pkt_t * head;           /* head packet in pktlist */
    pktlist_pkt_t * tail;           /* tail packet in pktlist */
    uint32_t        len;            /* number of packets in pktlist */
    struct {
      LE_DECL(
        uint16_t    prio : 4;       /* packet priority */
        uint16_t    dest : 12;      /* packet destination (domain scope) */
      )
      BE_DECL(
        uint16_t    dest : 12;
        uint16_t    prio : 4;
      )
    };
    pktfwd_key_t    key;            /* 2b domain, 2b incarn, 12b index */
};


/**
 * -----------------------------------------------------------------------------
 * pktlist_elem_t
 *
 * Each pktlist is encapsulated in a pktlist_elem, that includes a dll_t for
 * maintaining pktlist(s) in free lists ot pending transfer lists.
 * -----------------------------------------------------------------------------
 */

struct pktlist_elem                 /* pktlist_elem_t */
{
    dll_t           node;           /* use _envelope_of() */
    pktlist_t       pktlist;
};


/**
 * -----------------------------------------------------------------------------
 * pktlist_table_t
 *
 * Table of pktlist_elem_t maintained as a two dimensional array for indexed
 * access to a pktlist_elem_t (given a <prio,dest>)
 * -----------------------------------------------------------------------------
 */

struct pktlist_table                /* pktlist_table_t */
{
    /* do not add elements above the 2-dimensional array */
    pktlist_elem_t  elem[PKTLIST_PRIO_MAX][PKTLIST_DEST_MAX];
};


/**
 *------------------------------------------------------------------------------
 *
 * pktlist_context
 *
 * State:
 * - Table of pktlist elements
 * - Non empty pktlist elements are managed into mcast and by ucast[prio]
 *   dll, awaiting a transfer to the peer
 * - Empty pktlist elements are managed in a free dll
 * - Transfer object is described by a
 *        peer's pktlist_context ('this') pointer
 *        and a "xfer" handler function pointer ('member func')
 * - Owning driver context and name
 *
 *------------------------------------------------------------------------------
 */

struct pktlist_context              /* pktlist_context_t */
{
    /* Runtime State */
    spinlock_t          lock;       /* spinlock : not used in ingress* */

    uint32_t            dispatches; /* total dispatch handler invocations */

    pktlist_table_t   * table;      /* table of pktlist_elem_t */
    dll_t               free;       /* list of free pktlist_elem::nodes */
    dll_t               mcast;      /* list of mcast pktlist_elem::nodes */
    dll_t               ucast[PKTLIST_PRIO_MAX]; /* ucast lists by prio */
#if defined(BCM_PKTFWD_FLCTL)
    pktlist_fctable_t * fctable;    /* table of per pktlist credits */
#else /* ! BCM_PKTFWD_FLCTL */
    void              * fctable;
#endif /* ! BCM_PKTFWD_FLCTL */
    /* Peer consumer's pktlist context for daisy chaining (xfer lists) */
    pktlist_context_t * peer;       /* peer driver pktlist_context */
    pktlist_context_xfer_fn_t xfer_fn;  /* Transfer to peer hander */
    pktlist_context_keymap_fn_t keymap_fn;  /* DHD: pktfwd key and flowid map */

    /* Statistics and Debug */
    uint32_t            list_stats; /* total pktlists dispatched */
    uint32_t            pkts_stats; /* total pkts dispatched */
    void              * driver;     /* pointer to driver context */
    char                driver_name[PKTLIST_CTX_NAME_SZ];
    uint32_t            unit;       /* WLAN radio unit */

    /* System Wide Debug: list of all initialized instances */
    dll_t               instance;   /* use _envelope_of() */

} ____cacheline_aligned;


/**
 * =============================================================================
 * Section: PKTLIST Functional Interface
 * =============================================================================
 */


/**
 * -----------------------------------------------------------------------------
 *
 * Function   : __pktlist_xfer_pkts
 * Description: Extract a list of packets from a source packet list node and
 *              append them to the tail of the destination packet list node.
 *              The destination packet list node is moved to an active work list
 *              in the destination packet list context.
 *
 *              This helper function is invoked by __pktlist_xfer_work.
 *
 *              Destination driver managing the pktlist_context may move
 *              pktlist_elem_t from pending work lists (mcast or by ucast) to
 *              other driver "private" lists.
 *
 * -----------------------------------------------------------------------------
 */

static inline uint32_t
__pktlist_xfer_pkts(pktlist_elem_t * src_elem, pktlist_elem_t * dst_elem,
                    dll_t * dst_work_list, const NBuffPtrType_t NBuffPtrType)
{
    /* Terminate the pktlist, now */
    PKTLIST_PKT_SET_SLL(src_elem->pktlist.tail, PKTLIST_PKT_NULL, NBuffPtrType);

    PKTLIST_ASSERT((dst_elem->pktlist.prio == src_elem->pktlist.prio) &&
        (dst_elem->pktlist.dest == src_elem->pktlist.dest));

    /* Move destination element to work list and transfer packets to peer */
    if (dst_elem->pktlist.len == 0U)
    {
        dst_elem->pktlist.head    = src_elem->pktlist.head;
        dst_elem->pktlist.tail    = src_elem->pktlist.tail;
        dst_elem->pktlist.key.v16 = src_elem->pktlist.key.v16; 

        /* Delete node from current dll and move to work list */
        dll_delete(&dst_elem->node);
        dll_append(dst_work_list, &dst_elem->node);
    }
    else
    {
        /* CAUTION: dst_elem_t need not be in dst_work_list */
        PKTLIST_PKT_SET_SLL(dst_elem->pktlist.tail, src_elem->pktlist.head,
                            NBuffPtrType);
        dst_elem->pktlist.tail = src_elem->pktlist.tail;
    }

    dst_elem->pktlist.len += src_elem->pktlist.len;

    return src_elem->pktlist.len;

}   /* __pktlist_xfer_pkts() */


/**
 * -----------------------------------------------------------------------------
 *
 * Function   : __pktlist_xfer_work
 * Description: Traverse all pending pktlists placed in a source work list
 *              (mcast or by ucast), and flush each pktlist to the peer.
 *              All pktlists in the src work list are moved to the src free list
 *              after all the packets were handed off to the destination.
 *
 *              Uses helper function __pktlist_xfer_pkts which is responsible
 *              for retrieving a pktlist from a src pktlist_elem_t and joining
 *              it with the peer's corresponding pktlist_elem_t. The helper
 *              function is responsible for managing the peer pktlist_elem_t,
 *              i.e. moving it to the peer's corresponding work list, if the
 *              peer's pktlist_elem_t was empty prior to the join.
 *
 * __pktlist_xfer_work() needs to be invoked with peer pktlist_context LOCKED.
 *
 * -----------------------------------------------------------------------------
 */

static inline void
__pktlist_xfer_work(    /* Xfer pktlist(s) from producer to consumer worklist */
    pktlist_context_t * src_pktlist_context,    /* producer context */
    pktlist_context_t * dst_pktlist_context,    /* consumer context */
    dll_t             * src_work_list,          /* producer's work list */
    dll_t             * dst_work_list,          /* consumer's work list */
    const char        * name,
    const NBuffPtrType_t    NBuffPtrType)       /* Buffer type SKB|FKB */
{
    uint32_t list_stats, pkts_stats;
    pktlist_elem_t * src_elem, * dst_elem;
    dll_t * item, * next; /* dll iterator */

    if (dll_empty(src_work_list))
        return; /* no pktlists have been accumulated for xfer */

    list_stats = 0U;
    pkts_stats = 0U;

    /* For each pktlist in an active work list */
    for (item = dll_head_p(src_work_list); ! dll_end(src_work_list, item); )
    {
        next = dll_next_p(item); /* iterator's next */
        src_elem = _envelope_of(item, pktlist_elem_t, node);

        PKTLIST_ASSERT(src_elem->pktlist.len != 0U);

        PKTLIST_PTRACE("%s prio<%u> dest<%3u> len<%3u>", name,
           src_elem->pktlist.prio, src_elem->pktlist.dest,
           src_elem->pktlist.len);

        /* Locate peer element in peer's pktlist_context table */
        dst_elem = PKTLIST_CTX_ELEM(dst_pktlist_context,
                        src_elem->pktlist.prio, src_elem->pktlist.dest);

        /* Extract pkts from src_elem and append to dst_elem */
        pkts_stats += __pktlist_xfer_pkts(src_elem, dst_elem, dst_work_list,
                                          NBuffPtrType);
        ++list_stats;

        PKTLIST_RESET(&src_elem->pktlist);

        /* Later entire work list of pktlist(s) will be moved to free list */

        item = next; /* advance src work list iterator */
    }

    if (list_stats) /* if any lists were handed off */
    {
        /* Now move entire work list to free list */
        dll_t *src_free_list;
        src_free_list = &src_pktlist_context->free;
        dll_join(src_work_list, src_free_list);

        /* Update cummulative stats in dst and src pktlist_context */
        dst_pktlist_context->pkts_stats += pkts_stats;
        src_pktlist_context->pkts_stats += pkts_stats; /* packets handed off */

        src_pktlist_context->list_stats += list_stats; /* lists handed off */
    }

}   /* __pktlist_xfer_work() */


/**
 * -----------------------------------------------------------------------------
 *
 * Function   : __pktlist_xfer_pktlist
 * Description: Extract packets from all pktlist(s) in a work list and place
 *              them into a pktlist, independent of the pktlist_key.
 *
 * This function may be used to free packets in pending pktlist(s) in a worklist
 *
 * -----------------------------------------------------------------------------
 */

static inline void
__pktlist_xfer_pktlist(dll_t * work_list, pktlist_t * pktlist,
                       const NBuffPtrType_t NBuffPtrType)
{
    dll_t * item, * next; /* dll iterator */
    pktlist_elem_t * work_elem;

    if (dll_empty(work_list))
        return;

    /* For each pktlist in work list */
    for (item = dll_head_p(work_list); ! dll_end(work_list, item); )
    {
        next = dll_next_p(item); /* iterator's next */
        work_elem = _envelope_of(item, pktlist_elem_t, node);

        PKTLIST_ASSERT(work_elem->pktlist.len != 0U);

        /* xfer all packets from work list elem to pktlist */
        if (pktlist->len == 0U)
        {
            pktlist->head                  = work_elem->pktlist.head;
            pktlist->tail                  = work_elem->pktlist.tail;
        }
        else
        {
            PKTLIST_PKT_SET_SLL(pktlist->tail, work_elem->pktlist.head,
                                NBuffPtrType);
            pktlist->tail                  = work_elem->pktlist.tail;
        }

        pktlist->len += work_elem->pktlist.len;

        PKTLIST_RESET( &work_elem->pktlist );

    } /* for each work list elem */

}   /* __pktlist_xfer_pktlist() */


/**
 * -----------------------------------------------------------------------------
 *
 * Function   : __pktlist_head
 * Description: Get the head packet for a pktlist identified by prio and dest.
 *
 * -----------------------------------------------------------------------------
 */

static inline pktlist_pkt_t *
__pktlist_head(pktlist_context_t * pktlist_context,
    uint32_t prio, uint32_t dest)
{
    pktlist_t * pktlist;
    pktlist_elem_t * pktlist_elem;

    PKTLIST_ASSERT(pktlist_context != PKTLIST_CONTEXT_NULL);

    pktlist_elem = PKTLIST_CTX_ELEM(pktlist_context, prio, dest);
    pktlist = &pktlist_elem->pktlist;

    PKTLIST_ASSERT((pktlist->prio == prio) && (pktlist->dest == dest));

    if ( likely(pktlist->len != 0U) )
        return pktlist->head;
    else
        return PKTLIST_PKT_NULL;

}  /* __pktlist_head() */


/*
 * -----------------------------------------------------------------------------
 *
 * Function   : __pktlist_add_pkt
 * Description: Add a packet to a packet list.
 *              Packet list is identified by the prio and the destination.
 *              If the packet list was empty, then the packet list is placed
 *              onto a work list.
 *
 *              A pktlist adopts the first added packet's 16b key.
 *
 * -----------------------------------------------------------------------------
 */

static inline void
__pktlist_add_pkt(pktlist_context_t * pktlist_context,
    uint32_t prio, uint32_t dest, uint16_t key, pktlist_pkt_t * pkt,
    const NBuffPtrType_t NBuffPtrType)
{
    pktlist_t * pktlist;
    pktlist_elem_t * pktlist_elem;

    PKTLIST_ASSERT(pktlist_context != PKTLIST_CONTEXT_NULL);

    /* Locate the pktlist element and the contained pktlist */
    pktlist_elem = PKTLIST_CTX_ELEM(pktlist_context, prio, dest);
    pktlist = &pktlist_elem->pktlist;

    PKTLIST_ASSERT((pktlist->prio == prio) && (pktlist->dest == dest));

    if ( likely(pktlist->len != 0U) ) /* Do not use <head,tail> PKTLIST_RESET */
    {
        /* Append to tail - without auditing new packet's key (incarn) match */
        PKTLIST_PKT_SET_SLL(pktlist->tail, pkt, NBuffPtrType);
        pktlist->tail                  = pkt;
    }
    else /* Move pktlist from free list to appropriate ucast/mcast work list */
    {
        dll_delete(&pktlist_elem->node);

        if ( likely(dest != PKTLIST_MCAST_ELEM) ) /* ucast destination */
        {
            PKTLIST_PTRACE("prio %d dest %d key 0x%04x", prio, dest, key);
            pktlist->key.v16 = key; /* pktlist adopts head pkt's key::incarn */
            dll_append(&pktlist_context->ucast[prio], &pktlist_elem->node);
        }
        else                                      /* mcast packet */
        {
            /* key has no relevance, for multicast */
            dll_append(&pktlist_context->mcast, &pktlist_elem->node);
        }

        /* Add packet to head */
        pktlist->head = pktlist->tail = pkt;
    }

    /* CAUTION: PKTLIST_PKT_SET_SLL(pkt) is NOT terminated by PKTLIST_PKT_NULL ! */

    ++pktlist->len;

}   /* __pktlist_add_pkt() */


#if defined(BCM_PKTFWD_FLCTL)

/**
 * =============================================================================
 * Section: PKTFWD_FLCTL Functional Interface
 * =============================================================================
 */


/*
 * -----------------------------------------------------------------------------
 * Function : Set credits of a pktlist identified by the prio and dest.
 * -----------------------------------------------------------------------------
 */

static inline void
__pktlist_fctable_set_credits(pktlist_context_t * pktlist_context,
                              uint32_t prio, uint32_t dest, uint32_t credits)
{
    pktlist_credits_t * pktlist_credits;

    /* Locate the pktlist credit */
    pktlist_credits = PKTLIST_CTX_CREDITS(pktlist_context, prio, dest);

    atomic_set(pktlist_credits, credits);

}   /* __pktlist_fctable_set_credits() */


/*
 * -----------------------------------------------------------------------------
 * Function : Get credits of a pktlist identified by the prio and dest.
 * -----------------------------------------------------------------------------
 */

static inline int32_t
__pktlist_fctable_get_credits(pktlist_context_t * pktlist_context,
                              uint32_t prio, uint32_t dest)
{
    pktlist_credits_t * pktlist_credits;

    /* Locate the pktlist credit */
    pktlist_credits = PKTLIST_CTX_CREDITS(pktlist_context, prio, dest);

    return (atomic_read(pktlist_credits));

}   /* __pktlist_fctable_get_credits() */


/*
 * -----------------------------------------------------------------------------
 * Function : Increament credits of a pktlist identified by the prio and dest.
 * -----------------------------------------------------------------------------
 */

static inline void
__pktlist_fctable_inc_credits(pktlist_context_t * pktlist_context,
                              uint32_t prio, uint32_t dest)
{
    pktlist_credits_t * pktlist_credits;

    /* Dont update credits for favored prio */
    if (prio >= pktlist_context->fctable->pkt_prio_favor)
        return;

    /* Locate the pktlist credit */
    pktlist_credits = PKTLIST_CTX_CREDITS(pktlist_context, prio, dest);

    atomic_inc(pktlist_credits);

    PKTLIST_ASSERT(atomic_read(pktlist_credits) > 8192);

}   /* __pktlist_fctable_inc_credits() */


/*
 * -----------------------------------------------------------------------------
 * Function : Decreament credits of a pktlist identified by the prio and dest.
 * -----------------------------------------------------------------------------
 */

static inline void
__pktlist_fctable_dec_credits(pktlist_context_t * pktlist_context,
                              uint32_t prio, uint32_t dest)
{
    pktlist_credits_t * pktlist_credits;

    /* Dont update credits for favored prio */
    if (prio >= pktlist_context->fctable->pkt_prio_favor)
        return;

    /* Locate the pktlist credit */
    pktlist_credits = PKTLIST_CTX_CREDITS(pktlist_context, prio, dest);

    atomic_dec(pktlist_credits);

}   /* __pktlist_fctable_dec_credits() */


/*
 * -----------------------------------------------------------------------------
 * Function : Add credits to a pktlist identified by the prio and dest.
 * -----------------------------------------------------------------------------
 */

static inline void
__pktlist_fctable_add_credits(pktlist_context_t * pktlist_context,
                              uint32_t prio, uint32_t dest, uint32_t credits)
{
    pktlist_credits_t * pktlist_credits;

    /* Dont update credits for favored prio */
    if (prio >= pktlist_context->fctable->pkt_prio_favor)
        return;

    /* Locate the pktlist credit */
    pktlist_credits = PKTLIST_CTX_CREDITS(pktlist_context, prio, dest);

    atomic_add(credits, pktlist_credits);

}   /* __pktlist_fctable_add_credits() */


/*
 * -----------------------------------------------------------------------------
 * Function : Subtract credits of a pktlist identified by the prio and dest.
 * -----------------------------------------------------------------------------
 */

static inline void
__pktlist_fctable_sub_credits(pktlist_context_t * pktlist_context,
                              uint32_t prio, uint32_t dest, uint32_t credits)
{
    pktlist_credits_t * pktlist_credits;

    /* Dont update credits for favored prio */
    if (prio >= pktlist_context->fctable->pkt_prio_favor)
        return;

    /* Locate the pktlist credit */
    pktlist_credits = PKTLIST_CTX_CREDITS(pktlist_context, prio, dest);

    atomic_sub(credits, pktlist_credits);

}   /* __pktlist_fctable_sub_crediti() */

#endif /* BCM_PKTFWD_FLCTL */


#endif /* BCM_PKTLIST */


#if defined(BCM_PKTQUEUE)

/**
 * =============================================================================
 *
 * Section: PKTQUEUE Abstract Data Type
 *
 * Packet Queue management library between an ingress and egress network device.
 * Used in the WLAN driver to WFD/LAN transfer.
 *
 * WFD/LAN driver manages the SW Queues serviced by the ingress WLAN network
 * device. Packets arriving into the SW Queues are from a WLAN radio.
 * Every received packet will include a unique ID that identifies a bridged
 * connection to a destination LAN/WLAN station. Packets arriving in the SW
 * Queue may be interleaved with respect to the destination station.
 *
 * Egress network device (WFD/LAN) will sort packtets using Unique ID and
 * forward packets to the destination station.
 * Eg., WFD uses Pktlist infrastructure for sorting and forwarding packets.
 *
 * Theory of Operation:
 * --------------------
 *
 * A table of packet queues are maintained in the ingress driver. Packets
 * arriving at ingress are directly added to the queue identified by the
 * destination domain (domain is derived using d3lut lookup).
 *
 * After all (or bounded by a budget) incoming packets are added to packet
 * queues, packet queues may be handed off to egress packet queue and inform
 * Egress driver arrival of new packets using reigstered callbacks.
 *
 * Packet queue transfer to egress driver's SW queue must ensure that a
 * lock is taken. The pending pktqueues in ingress driver are transferred to
 * the egress driver SW queue, using a fast queue join.
 *
 * As new packets arrive, and get flushed to the Egress driver SW queue, the
 * peer queue will grow, until the egress driver is ready to process SW queue.
 * Egress driver will dispatch packets in SW queue (bounded by a budget)
 * into its datapath.
 *
 * Implementation Caveats:
 *
 * Packet Queues are maintained in source (ingress) and destination (egress)
 * driver. Ingress addition of pkts to packet queue is not protected by lock.
 *
 * A pktqueue is instantiated per pair of <ingress, egress> driver.
 * Ingress driver accumulates packets into local packet queue and then
 * hands off to the peer packet queue using the attached "flush" handler.
 *
 * Version History
 *      1.0.0   :   Initial Version
 *
 * =============================================================================
 */

/** PKTQUEUE Conditional compiles */
#define CC_PKTQUEUE_DEBUG           (0)
// #define CC_PKTQUEUE_STATS

/** PKTQUEUE return codes */
#define PKTQUEUE_FAILURE            (-1)
#define PKTQUEUE_SUCCESS            (0)

/**
 * -----------------------------------------------------------------------------
 * Section: PKTQUEUE Dimensioning
 * -----------------------------------------------------------------------------
 */

/** Total number of domains managed */
#define PKTQUEUE_DOMAINS_TOT            (PKTFWD_DOMAINS_TOT)
#define PKTQUEUE_XDOMAIN_IDX            (PKTFWD_XDOMAIN_IDX)

/** Total number of queues managed (3 WLAN + 1 LAN + 1 network stack) */
#define PKTQUEUE_QUEUES_MAX             (PKTFWD_DOMAINS_TOT + 1)
#define PKTQUEUE_NTKSTK_QUEUE_IDX       (PKTQUEUE_QUEUES_MAX - 1) /* last */

/**
 * -----------------------------------------------------------------------------
 * Section: PKTQUEUE Debug Support
 * -----------------------------------------------------------------------------
 */

#define PKTQUEUE_NOOP                PKTFWD_NOOP

/** Errors always enabled ... avoid using PRINT, use TRACE or PTRACE */
#define PKTQUEUE_ERROR(fmt, arg...) \
    printk(CLRr "%s: " fmt CLRnl, __FUNCTION__, ##arg)
#define PKTQUEUE_PRINT(fmt, arg...) \
    printk(CLRg "%s: " fmt CLRnl, __FUNCTION__, ##arg)

/** Assert and Warnings enabled at level 1 and above */
#if (CC_PKTQUEUE_DEBUG >= 1)
#define PKTQUEUE_ASSERT(exp)         BCM_ASSERT_A(exp)
#define PKTQUEUE_WARN(fmt, arg...) \
    printk(CLRm "%s: " fmt CLRnl, __FUNCTION__, ##arg)
#else
#define PKTQUEUE_ASSERT(exp)         PKTQUEUE_NOOP
#define PKTQUEUE_WARN(fmt, arg...)   PKTQUEUE_NOOP
#endif  /* CC_PKTQUEUE_DEBUG < 1 */

/** Function entry and Trace enabled at level 2 and above */
#if (CC_PKTQUEUE_DEBUG >= 2)
#define PKTQUEUE_FUNC()              printk(CLRb "%s" CLRnl, __FUNCTION__)
#define PKTQUEUE_TRACE(fmt, arg...) \
    printk(CLRb "%s: " fmt CLRnl, __FUNCTION__, ##arg)
#else
#define PKTQUEUE_FUNC()              PKTQUEUE_NOOP
#define PKTQUEUE_TRACE(fmt, arg...)  PKTQUEUE_NOOP
#endif  /* CC_PKTQUEUE_DEBUG < 2 */

/** Per "P"acket Function entry and Trace enabled at level 3 and above */
#if (CC_PKTQUEUE_DEBUG >= 3)
#define PKTQUEUE_PFUNC()             printk(CLRb "%s" CLRnl, __FUNCTION__)
#define PKTQUEUE_PTRACE(fmt, arg...) \
    printk(CLRb "%s: " fmt CLRnl, __FUNCTION__, ##arg)
#else
#define PKTQUEUE_PFUNC()             PKTQUEUE_NOOP
#define PKTQUEUE_PTRACE(fmt, arg...) PKTQUEUE_NOOP
#endif /* CC_PKTQUEUE_DEBUG < 3 */

/** Design optimization, override during debug. */
#if defined(CC_PKTQUEUE_DEBUG)
#define PKTQUEUE_RESET(pktqueue_p) \
({  \
    (pktqueue_p)->head      = PKTQUEUE_PKT_NULL; \
    (pktqueue_p)->tail      = PKTQUEUE_PKT_NULL; \
    (pktqueue_p)->len       = 0U; \
})

#else

#define PKTQUEUE_RESET(pktqueue_p) \
({ \
    (pktqueue_p)->len       = 0U; \
}) /* Do not need to reset <head,tail> just len. */
#endif /* ! CC_PKTQUEUE_DEBUG */

/**
 * -----------------------------------------------------------------------------
 * Section: PKTQUEUE Statistics collection for design
 * -----------------------------------------------------------------------------
 */

#if defined(CC_PKTQUEUE_STATS)
#define PKTQUEUE_STATS_ADD(v32, val)    ((v32) += (val))
#else
#define PKTQUEUE_STATS_ADD(v32, val)    PKTFWD_NOOP
#endif /* ! CC_PKTQUEUE_STATS */


/**
 * -----------------------------------------------------------------------------
 * Section: PKTQUEUE Namespaces: Objects forward declarations and definitions
 * -----------------------------------------------------------------------------
 */

struct  pktqueue;                   /* single linked list of packets */
struct  pktqueue_table;             /* array of pktqueues */
struct  pktqueue_context;           /* binning and xfer of pktqueue */

/* Same as Nbuff_t of struct sk_buff or struct fkbuff */
typedef void                        pktqueue_pkt_t;

typedef struct  pktqueue            pktqueue_t;
typedef struct  pktqueue_table      pktqueue_table_t;
typedef struct  pktqueue_context    pktqueue_context_t;

#define PKTQUEUE_NULL               ((pktqueue_t *)         NULL)
#define PKTQUEUE_PKT_NULL           ((pktqueue_pkt_t *)     NULL)
#define PKTQUEUE_TABLE_NULL         ((pktqueue_table_t *)   NULL)
#define PKTQUEUE_CONTEXT_NULL       ((pktqueue_context_t *) NULL)

/**
 * -----------------------------------------------------------------------------
 * Section: PKTQUEUE Conversion and Table Accessor Macros
 * -----------------------------------------------------------------------------
 */

/** Network buff's are linked into a single linked list using nbuff::queue or
 *  sk_buff::prev field based on buf type.
 */
#define PKTQUEUE_PKT_SLL(pkt, NBuffPtrType)             \
                pktfwd_pktsll((pkt), (NBuffPtrType))

#define PKTQUEUE_PKT_SET_SLL(pkt, link, NBuffPtrType)   \
                pktfwd_pktsetsll((pkt), (link), (NBuffPtrType))

#define PKTQUEUE_PKT_KEY(pkt, NBuffPtrType)             \
                pktfwd_pktqueue_pktkey((pkt), (NBuffPtrType))

#define PKTQUEUE_PKT_FREE(pkt)              pktfwd_pktfree((pkt))

/** PKTQUEUE accessor macro to locate a pktqueue_t */
#define PKTQUEUE_TBL_QUEUE(tbl, queue)                  \
            ((pktqueue_t *)&(((pktqueue_table_t *)(tbl))->pktqueue[queue]))

/**
 * -----------------------------------------------------------------------------
 * Section: PKTQUEUE Modules and registration of exported functions
 * -----------------------------------------------------------------------------
 */

/**
 * -----------------------------------------------------------------------------
 * Flush all packets in pktqueues (maintained by dest domian) to the egress
 * driver's pktqueue using the pktqueue_flush_pkts_fn_t handler in the source
 * driver.
 *
 * After flushing packtes, inform Egress driver about arrival of new packets
 * using the pktqueue_flush_complete_fn_t handler.
 *
 * On Init, the Egress driver pktqueue flush and flush complete handler will
 * be saved in the common bcm_pktfwd pktqueue context accessed by multiple
 * producers (NIC & DHD WLAN drivers).
 *
 * This allows a daisy chaining of transfers.
 * -----------------------------------------------------------------------------
 */

typedef bool (* pktqueue_flush_pkts_fn_t)(void * driver, pktqueue_t * pktqueue);
typedef void (* pktqueue_flush_complete_fn_t)(void * driver);

#define PKTQUEUE_FLUSH_PKTS_NULL        ((pktqueue_flush_pkts_fn_t) NULL)
#define PKTQUEUE_FLUSH_COMPLETE_NULL    ((pktqueue_flush_complete_fn_t) NULL)

extern  pktqueue_context_t *
            pktqueue_context_register(
                    pktqueue_flush_pkts_fn_t        pktqueue_flush_pkts_fn,
                    pktqueue_flush_complete_fn_t    pktqueue_flush_complete_fn,
                    void * driver, uint32_t domain);

extern  void   pktqueue_context_reset(pktqueue_context_t * pktqueue_context);

extern  pktqueue_context_t *
            pktqueue_context_unregister(pktqueue_context_t * pktqueue_context);

extern  pktqueue_context_t *
            pktqueue_get_domain_pktqueue_context(uint32_t domain);

extern  void   pktqueue_context_dump(pktqueue_context_t * context);

/** Private to bcm_pktfwd.c and hosting module, to dump all instances */
extern  void   pktqueue_context_dump_all(void);


/**
 * =============================================================================
 * Section: PKTQUEUE Core Structures
 * =============================================================================
 */


/**
 * -----------------------------------------------------------------------------
 * pktqueue_t
 *
 * Linked list of packets. PKTQUEUE_PKT_SLL(pkt) uses the sk_buff::prev pointer
 * or nbuff::queue buff_queue to form a single linked list of packets having a
 * common attribute <dest domain>
 *
 * A D3LUT element's 16bit key is transparently passed to each packet, and is
 * composed of a 2b domain (radio unit), a 2b incarnation, and a 12b global
 * scoped endpoint index.
 *
 * -----------------------------------------------------------------------------
 */

struct pktqueue                     /* pktqueue_t */
{
    pktqueue_pkt_t    * head;           /* head packet in pktqueue */
    pktqueue_pkt_t    * tail;           /* tail packet in pktqueue */
    uint32_t            len;            /* number of packets in pktqueue */
    NBuffPtrType_t      NBuffPtrType;   /* SKB|FPB|TGB|FKB */
};

/**
 * -----------------------------------------------------------------------------
 * pktqueue_table_t
 *
 * Table of pktqueue_elem_t maintained as an array for indexed access to a
 * pktqueue_t (given a <domain>)
 * -----------------------------------------------------------------------------
 */

struct pktqueue_table                /* pktqueue_table_t */
{
    pktqueue_t  pktqueue[PKTQUEUE_QUEUES_MAX];
};


/**
 *------------------------------------------------------------------------------
 *
 * pktqueue_context
 *
 * State:
 * - Egress driver's
 *   "flush" handler
 *   "flush complete" handler
 * - Owning driver context and domain
 * - Egress driver context initialization state
 *
 *------------------------------------------------------------------------------
 */

struct pktqueue_context         /* pktqueue_context_t */
{
    /* Flush packets to egress driver pktqueue */
    pktqueue_flush_pkts_fn_t        flush_pkts_fn;
    /* Flush complete callback to egress driver */
    pktqueue_flush_complete_fn_t    flush_complete_fn;

    /* Statistics and Debug */
    uint32_t        queue_stats;    /* total queues flushed */
    uint32_t        pkts_stats;     /* total packets flushed */
    void          * driver;         /* pointer to driver context */
    uint32_t        domain;         /* Egress driver domain */
    bool            initialized;    /* Egress driver context registered */
};

/**
 * =============================================================================
 * Section: PKTQUEUE Functional Interface
 * =============================================================================
 */

/**
 * -----------------------------------------------------------------------------
 *
 * Function   : __pktqueue_xfer_pkts
 * Description: Extract a queue of packets from a source packet queue and append
 *              them to the tail of the destination packet queue.
 *
 *              This helper function is invoked by egress network device "flush"
 *              handler "pktqueue_flush_pkts_fn_t".
 * -----------------------------------------------------------------------------
 */

static inline uint32_t
__pktqueue_xfer_pkts(pktqueue_t * src_pktqueue, pktqueue_t * dst_pktqueue,
                     const NBuffPtrType_t NBuffPtrType)
{

    PKTQUEUE_PTRACE("len<%u> pkt_type<%3s>", src_pktqueue->len,
        src_pktqueue->NBuffPtrType == SKBUFF_PTR ? "SKB" : "FKB");

    /* Terminate the pktqueue, now */
    PKTQUEUE_PKT_SET_SLL(src_pktqueue->tail, PKTQUEUE_PKT_NULL, NBuffPtrType);

    /* Transfer packets to peer */
    if (dst_pktqueue->len == 0U)
        dst_pktqueue->head = src_pktqueue->head;
    else
        PKTQUEUE_PKT_SET_SLL(dst_pktqueue->tail, src_pktqueue->head,
                             NBuffPtrType);

    dst_pktqueue->tail = src_pktqueue->tail;
    dst_pktqueue->len += src_pktqueue->len;

    return src_pktqueue->len;

}   /* __pktqueue_xfer_pkts */

/**
 * -----------------------------------------------------------------------------
 * Function : __pktqueue_free_pkts
 *            Helper routine to free all packet in a pktqueue.
 * CAUTION: No osh accounting
 * -----------------------------------------------------------------------------
 */

static inline void
__pktqueue_free_pkts(pktqueue_t * pktqueue_free,
                     const NBuffPtrType_t NBuffPtrType)
{
    pktqueue_pkt_t * pkt;

    PKTFWD_FUNC();

    while (pktqueue_free->len)
    {
        pkt = pktqueue_free->head;
        pktqueue_free->head = PKTQUEUE_PKT_SLL(pkt, NBuffPtrType);
        PKTQUEUE_PKT_SET_SLL(pkt, PKTQUEUE_PKT_NULL, NBuffPtrType);
        pktqueue_free->len--;

        PKTQUEUE_PKT_FREE(pkt);
    }

    return;

}   /* __pktqueue_free_pkts() */


#endif /* BCM_PKTQUEUE */


#if defined(BCM_D3FWD)

/**
 * =============================================================================
 * Section: Forwarding Context Management.
 *
 * D3FWD serves as a proxy cache of the native network stack's bridge layer.
 *
 * In Linux, the bridge fdb (see br_device.c and br_input.c) is used to populate
 * entries in the D3FWD database, with network device extensions to WLAN facing
 * interfaces and stations withing such interfaces, and the 802.3 LAN facing
 * network devices.
 *
 * TODO: This is not a true Abstract Data Type and may be moved into wl_pktfwd.c
 *       Retained here for shared access between dhd.ko and wl.ko
 *
 * =============================================================================
 */

#define CC_D3FWD_DEBUG              (0)
// #define CC_D3FWD_STATS

#define D3FWD_NONE
#define D3FWD_NOOP                  PKTFWD_NOOP

/** D3LUT: API return values */
#define D3FWD_FAILURE               (-1)
#define D3FWD_SUCCESS               (0)

#if defined(CC_D3FWD_STATS)
#define D3FWD_STATS_EXPR(expr)      expr
#define D3FWD_STATS_ADD(v32, val)   ((v32) += (val))
#else
#define D3FWD_STATS_EXPR(expr)      D3FWD_NONE
#define D3FWD_STATS_ADD(v32, val)   D3FWD_NOOP
#endif /* CC_D3FWD_STATS */


/** Errors always enabled ... avoid using PRINT, use TRACE or PTRACE */
#define D3FWD_ERROR(fmt, arg...) \
    printk(CLRr "%s: " fmt CLRnl, __FUNCTION__, ##arg)
#define D3FWD_PRINT(fmt, arg...) \
    printk(CLRg "%s: " fmt CLRnl, __FUNCTION__, ##arg)

/** Assert and Warnings enabled at level 1 and above */
#if (CC_D3FWD_DEBUG >= 1)
#define D3FWD_ASSERT(exp)           BCM_ASSERT_A(exp)
#define D3FWD_WARN(fmt, arg...) \
    printk(CLRm "%s: " fmt CLRnl, __FUNCTION__, ##arg)
#else
#define D3FWD_ASSERT(exp)           D3FWD_NOOP
#define D3FWD_WARN(fmt, arg...)     D3FWD_NOOP
#endif /* CC_D3FWD_DEBUG < 1 */

/** Function entry and Trace enabled at level 2 and above */
#if (CC_D3FWD_DEBUG >= 2)
#define D3FWD_FUNC()                printk(CLRb "%s" CLRnl, __FUNCTION__)
#define D3FWD_TRACE(fmt, arg...) \
    printk(CLRb "%s: " fmt CLRnl, __FUNCTION__, ##arg)
#else
#define D3FWD_FUNC()                D3FWD_NOOP
#define D3FWD_TRACE(fmt, arg...)    D3FWD_NOOP
#endif /* CC_D3FWD_DEBUG < 2 */

/** Per "P"acket Function entry and Trace enabled at level 3 and above */
#if (CC_D3FWD_DEBUG >= 3)
#define D3FWD_PFUNC()               printk(CLRb "%s" CLRnl, __FUNCTION__)
#define D3FWD_PTRACE(fmt, arg...) \
    printk(CLRc "%s: " fmt CLRnl, __FUNCTION__, ##arg)
#else
#define D3FWD_PFUNC()               D3FWD_NOOP
#define D3FWD_PTRACE(fmt, arg)      D3FWD_NOOP
#endif /* CC_D3LUT_DEBUG < 3 */

/**
 * -----------------------------------------------------------------------------
 * Section: D3FWD Namespace: Objects forward declarations and definitions
 * -----------------------------------------------------------------------------
 */
struct net_device;              /* linux net_device: LAN or WLAN virtual if */
struct osl_info;                /* wlan OS abstraction layer handle */
struct wl_info;                 /* WLAN radio instance */
struct wl_if;                   /* extends a virtual interface's net_device */

struct  d3fwd_wlif;             /* d3fwd extension to a WLAN virtual if */
struct  d3fwd_ext;              /* d3fwd extension of a D3 hash table entry */

typedef struct d3fwd_wlif           d3fwd_wlif_t;
typedef struct d3fwd_ext            d3fwd_ext_t;

#define D3FWD_WLIF_NULL             ((d3fwd_wlif_t *) NULL)


#define D3FWD_PRIO_MAX              (PKTFWD_PRIO_MAX)

#if (D3FWD_PRIO_MAX != (8))
#error "ucast_pktlist supports 8 unicast packet priorities"
#endif

/* each station structure */
typedef struct d3lut_sta
{
    dll_t       node;
    uint8_t     mac[6]; /*ETHER_ADDR_LEN*/
    uint16_t    flag;                   /* flag for this sta */
    struct d3lut_elem *d3lut_elem;      /* associated d3lut_elem */
} d3lut_sta_t;

/**
 * -----------------------------------------------------------------------------
 * d3fwd_wlif_stats is a private structure. (data-hiding: move to wl_pktfwd.c)
 * Per WLAN Interface Statistics, maintained per ucast priority.
 * -----------------------------------------------------------------------------
 */

typedef struct d3fwd_wlif_stats
{
    /* WLAN transmit (downstream) stats */
    uint32_t   tot_pkts;    /* total packets */
    uint32_t   cfp_pkts;    /* pkts count - bypass via WLCFP */
    uint32_t   cfp_fwds;    /* xfer count - bypass via WLCFP */
    uint32_t   chn_pkts;    /* pkts count - WLCFP miss use native chain_node */
    uint32_t   chn_fwds;    /* xfer count - WLCFP miss use native chain_node */
    uint32_t   slow_pkts;   /* xfer count - WLCFP miss, no PKTC, fwd one pkt at
			     * a time */
    uint32_t   tx_drops;    /* dropped packets */
    uint32_t   schedule;    /* wlan thread: wlif net device dispatch requests */
    uint32_t   complete;    /* wlan thread: wlif net device scheduled */

    /* WLAN receive (upstream) stats */
    uint32_t   rx_tot_pkts;     /* total recv packets */
    uint32_t   rx_fast_pkts;    /* LUT HIT, fast path packets */
    uint32_t   rx_slow_pkts;    /* LUT MISS, slow path packets */
    uint32_t   rx_drops;        /* dropped recv packets */

} d3fwd_wlif_stats_t;


/**
 * -----------------------------------------------------------------------------
 * d3fwd_wlif is a private structure.
 *
 * Extension of network stack's (e.g. Linux net_device) with a WLAN interface.
 * Structure holds pending packet work lists for dispatch into a wl_if.
 * Pending packet lists per destination station, accumulated in the ucast
 * work list may be transposed into a PKTC chain_node construct for delivery to
 * the WLAN interface, or alternatively leverage a Cached Flow Processing bypass
 * datapath, if the CFP gate-keeper deems candidate for bypass aceeleration.
 *
 * 1:1 extension:
 *     "d3fwd_wlif_t" extends "struct wl_if" extends base "struct net_device"
 *     Extensions are via pointers as opposed to one contiguous object.
 *
 * TODO: Should we move struct d3fwd_wlif to wl_pktfwd.c to enforce data-hiding.
 * -----------------------------------------------------------------------------
 */

struct d3fwd_wlif                       /* d3fwd_wlif_t */
{
    dll_t               node;           /* use _envelope_of() */
    struct osl_info   * osh;
    struct wl_if      * wlif;           /* 1:1 wl_if extension */
    struct net_device * net_device;     /* net_device of wl_if */
    struct d3lut_elem * wds_d3lut_elem; /* d3lut_elem used for WDS */

    uint8_t             wl_schedule;    /* wlan thread schedule state */
    struct {
        uint8_t         unit    : 4;    /* radio unit */
        uint8_t         wfd_idx : 4;;   /* wfd_idx servicing the wl_if */
    };
    uint16_t            stations;
    uint32_t            ucast_bmap;     /* bmap of pending ucast work */

    /* mcast and ucast worklists are protected by radio's PKTLIST lock */
    dll_t               mcast;          /* mcast pktlist work */
    dll_t               ucast[D3FWD_PRIO_MAX]; /* ucast pktlist work by prio */

    D3FWD_STATS_EXPR(
    d3fwd_wlif_stats_t  stats[D3FWD_PRIO_MAX]; /* wlif stats */
    )

    /* flags for each netdevice/wlif */
    uint32_t            flags;

    /* sta free list per intf */
    dll_t               sta_free_list;
    d3lut_sta_t         *sta_pool;

};

extern void d3fwd_wlif_stats_dump(d3fwd_wlif_stats_t * d3fwd_wlif_stats,
                bool clear_on_read);
extern void d3fwd_wlif_dump(d3fwd_wlif_t * d3fwd_wlif);
extern void d3fwd_wlif_clr(d3fwd_wlif_t * d3fwd_wlif);


/**
 * -----------------------------------------------------------------------------
 * Extension of a D3LUT entry per WLAN station endpoint or LAN endpoint
 *
 * d3lut_elem_t is-a d3fwd_ext_t "by inheritance",(as opposed to a has-a
 * composition. This avoids the necessity for independently managing the
 * d3lut_elem_t and d3fwd_ext_t, which are used in a 1:1 is-a relationship.
 * -----------------------------------------------------------------------------
 */

struct d3fwd_ext                        /* d3fwd_ext_t */
{
    union {
        void              * if_handle;  /* opaque interface handler */
        d3fwd_wlif_t      * d3fwd_wlif; /* wlif interface hosting station */
        struct net_device * net_device; /* lan interface's net_device */
    };
    union {
        pktfwd_key_t    flow_key;
        uint16_t        flowid;         /* sta_id or cfp's flow_Id */
    };
    uint8_t             ucast_bmap;     /* bit map of pending ucast pktlist */
    struct {
        uint8_t         wlan            :  1; /* 802.3 address in WLAN domain */
        uint8_t         assoc           :  1; /* 0:disassoc, 1:assoc */
        uint8_t         inuse           :  1; /* d3fwd_ext entry in use */
        uint8_t         hit             :  1; /* packet passed through d3fwd */
        uint8_t         flags           :  4; /* future use ... */
    };
    uint16_t            ssid;           /* WLAN interface index */
    uint16_t            rsvd16;         /* pad a D3LUT element to 16 B */
};

void d3fwd_ext_dump(d3fwd_ext_t *d3fwd_ext);


/**
 * -----------------------------------------------------------------------------
 * D3FWD Extension Management
 * When a D3LUT element is free, the extension will be cloberred, implying that
 * it may not be used. On element allocation, the cloberred state is asserted.
 * Optionally assert that d3fwd_ext::inuse is indeed 0 on d3lut_del -> elem put
 * -----------------------------------------------------------------------------
 */

/** D3FWD EXT: Test and Set: used by Get and Put allocation */
#define D3FWD_EXT_SET(__ext, __inuse) \
    ({  D3FWD_ASSERT(__ext.inuse ^ (__inuse)); \
        memset(&(__ext), 0, sizeof(d3fwd_ext_t)); \
        __ext.inuse = (__inuse); \
    })

/** D3FWD EXT: Get invoked on allocation, marking inuse = 1 */
#define D3FWD_EXT_GET(__ext)        D3FWD_EXT_SET(__ext, 1)

/** D3FWD EXT: Put invoked on de-allocation, marking inuse = 0 */
#define D3FWD_EXT_PUT(__ext)        D3FWD_EXT_SET(__ext, 0)

#define D3FWD_EXT_CMP(__ext, hndl)  ((__ext).if_handle == (hndl))


#else  /* ! BCM_D3FWD */

/**
 * Permit the D3LUT to be used as a standalone Dictionary ADT with has-a data
 * hiding extension, or N:M elem to extension binding ...
 *
 * NOTE: ! BCM_D3FWD is Untested.
 */
#define D3FWD_ASSERT(exp)           D3FWD_NOOP
#define D3FWD_ERROR(fmt, arg...)    D3FWD_NOOP
#define D3FWD_PRINT(fmt, arg...)    D3FWD_NOOP
#define D3FWD_WARN(fmt, arg...)     D3FWD_NOOP
#define D3FWD_FUNC()                D3FWD_NOOP
#define D3FWD_TRACE(fmt, arg...)    D3FWD_NOOP
#define D3FWD_PFUNC()               D3FWD_NOOP
#define D3FWD_PTRACE(fmt, arg...)   D3FWD_NOOP

typedef void * d3fwd_ext_t;

#define D3FWD_EXT_SET(__ext, inuse) ({ __ext = (void *) NULL; })
#define D3FWD_EXT_GET(__ext)        ({ __ext = (void *) NULL; })
#define D3FWD_EXT_PUT(__ext)        ({ __ext = (void *)((uintptr_t)(~0)); })
#define D3FWD_EXT_CMP(__ext, hndl)  ( (__ext) == (hndl) )

#endif /* ! BCM_D3FWD */


#if defined(BCM_D3LUT)

/**
 * =============================================================================
 * Section: 802.3 MacAddress LookUp Table
 * =============================================================================
 */


/**
 * -----------------------------------------------------------------------------
 * Section: D3LUT Conditional Compiles
 * -----------------------------------------------------------------------------
 */

// #define CC_D3LUT_BFLT            /* D3LUT Bloom Filter Support */
#define CC_D3LUT_XORH               /* 16 bit XOR Hash algorithm (or crc16) */

#define CC_D3LUT_DEBUG              (0)
// #define CC_D3LUT_STATS


/**
 * -----------------------------------------------------------------------------
 * Section: D3LUT Namespace:  Forward Declaration and Definitions
 * -----------------------------------------------------------------------------
 */

union   d3lut_sym;                  /* symbol (6B D3 MacAddr) */
struct  d3lut_elem;                 /* element of <Sym,Key> and extension */
struct  d3lut_bins;                 /* hash table (cols) collision bins */
struct  d3lut_bkts;                 /* hash table (rows) bukets (of bins) */
struct  d3lut_dict;                 /* dictionary - hash table abstraction */
union   d3lut_last;                 /* last hit cached lookup */
struct  d3lut_pool;                 /* pre-allocated pool of d3lut_elem(s) */
struct  d3lut;                      /* 802.3 MAC addr lookup table */

typedef union  d3lut_sym            d3lut_sym_t;
typedef union  pktfwd_key           d3lut_key_t;
typedef struct d3lut_elem           d3lut_elem_t;
typedef struct d3lut_bins           d3lut_bins_t;
typedef struct d3lut_bkts           d3lut_bkts_t;
typedef struct d3lut_dict           d3lut_dict_t;
typedef union  d3lut_last           d3lut_last_t;
typedef struct d3lut_pool           d3lut_pool_t;
typedef struct d3lut                d3lut_t;

#define D3LUT_SYM_NULL              ((d3lut_sym_t *)  NULL)
#define D3LUT_KEY_NULL              ((d3lut_key_t *)  NULL)
#define D3LUT_ELEM_NULL             ((d3lut_elem_t *) NULL)
#define D3LUT_BINS_NULL             ((d3lut_bins_t *) NULL)
#define D3LUT_BKTS_NULL             ((d3lut_bkts_t *) NULL)
#define D3LUT_DICT_NULL             ((d3lut_dict_t *) NULL)
#define D3LUT_LAST_NULL             ((d3lut_last_t *) NULL)
#define D3LUT_NULL                  ((d3lut_t *)      NULL)

/**
 * -----------------------------------------------------------------------------
 * Section: D3LUT Generic Macros
 * -----------------------------------------------------------------------------
 */
#define D3LUT_NONE
#define D3LUT_NOOP                  PKTFWD_NOOP

/** D3LUT: API return values */
#define D3LUT_FAILURE               (-1)
#define D3LUT_SUCCESS               (0)

#define D3LUT_KEY_INDEX_MASK        (PKTFWD_KEY_INDEX_MASK)
#define D3LUT_KEY_INDEX_SHIFT       (PKTFWD_KEY_INDEX_SHIFT)

#define D3LUT_KEY_INCARN_MASK       (PKTFWD_KEY_INCARN_MASK)
#define D3LUT_KEY_INCARN_SHIFT      (PKTFWD_KEY_INCARN_SHIFT)

#define D3LUT_KEY_DOMAIN_MASK       (PKTFWD_KEY_DOMAIN_MASK)
#define D3LUT_KEY_DOMAIN_SHIFT      (PKTFWD_KEY_DOMAIN_SHIFT

#define D3LUT_KEY_MATCH_MASK        (PKTFWD_KEY_MATCH_MASK) /* skip domain */

/** D3LUT: Invalid Values */
#define D3LUT_KEY_INVALID           (PKTFWD_KEY_INVALID)
#define D3LUT_KEY_INDEX_INVALID     (PKTFWD_KEY_INDEX_INVALID)

#define D3LUT_LAST_INVALID          (0U) /* D3LUT_BFLT_HASH_INVALID */


/**
 * -----------------------------------------------------------------------------
 * Section: D3LUT Helper macros
 * -----------------------------------------------------------------------------
 */

/**  Check for 32b aligned sym */
#define D3LUT_ELEM_IS_ALIGN(elem) ((((uintptr_t)(elem)) & 1) == 0)

/** Compute an element index, given a table base and an element pointer */
#define D3LUT_TABLE_IDX(base, elem) \
    ((uint32_t)(((d3lut_elem_t *)(elem)) - ((d3lut_elem_t *)(base))))

/** Compute an element pointer, given a table base and the element's index */
#define D3LUT_TABLE_ELEM(base, idx) \
    (((d3lut_elem_t *)(base)) + (idx))

/** Compute an element address, given a table base and the element's index */
#define D3LUT_TABLE_ADDR(base, idx) \
    ((base) + ((idx) * sizeof(d3lut_elem_t)))


/**
 * -----------------------------------------------------------------------------
 * Section: D3LUT Dimensioning
 * -----------------------------------------------------------------------------
 */

/** Total number of pools (domains) managed */
#define D3LUT_POOL_TOT              (PKTFWD_DOMAINS_TOT)

/** Maximum number of elements across all hash tables */
#define D3LUT_ELEM_TOT              (PKTFWD_ENDPOINTS_TOT)

/** Maximum number of elements per pool */
#define D3LUT_ELEM_MAX              (D3LUT_ELEM_TOT / D3LUT_POOL_TOT)

#if (D3LUT_POOL_TOT > 4)
#error "d3lut_key_t 2b key index limits to 4 domains"
#endif

#if (D3LUT_ELEM_TOT > 4096)
#error "d3lut_key_t 12b key index limits to 4K element indices"
#endif


/** Given a key index, fetch the per pool element index */
#define D3LUT_ELEM_IDX(key_index)   PKTFWD_DOMAIN_ENDPOINT_IDX(key_index)

/** Given a global space endpoint index, fetch the domain index */
#define D3LUT_POOL_IDX(key_index)   PKTFWD_ENDPOINT_DOMAIN_IDX(key_index)

/** Given a per domain endpoint index, convert to global enpoint index */
#define D3LUT_KEY_IDX(pool_idx, elem_idx) \
    PKTFWD_GBL_ENDPOINT_IDX((pool_idx), (elem_idx))


/** D3LUT element lookup: Find symbol in global pool  */
#define D3LUT_LKUP_GLOBAL_POOL      ((uint32_t)(~0U))

/** Pool Allocation Policy: Dynamic Allocation or Explicit ID Selection */
#define D3LUT_POLICY_POOL_BY_INDEX  (0U)
#define D3LUT_POLICY_POOL_FREELIST  ((uint32_t)(~0U))

/** D3LUT cached recent lookup using sym's last byte as slot index in cache */
#define D3LUT_LAST_SYM_BYTE_IX      (5)     /* 6'th byte in symbol */
#define D3LUT_LAST_SLOTS_MAX        (256)   /* slot for each 8-bit index */

/** D3LUT Dictionary and Element symbol dimensioning */
#define D3LUT_DICT_SYM_SIZE         (6)     /* size of a Mac Address */
#define D3LUT_DICT_BKTS_MAX         (1024)  /* 1024 buckets */
#if defined(CONFIG_BCM_HND_EAP)
#define D3LUT_DICT_BINS_MAX         (16)    /* collision list */
#else
#define D3LUT_DICT_BINS_MAX         (4)     /* collision list */
#endif
#define D3LUT_DICT_SYMS_MAX         (D3LUT_DICT_BKTS_MAX * D3LUT_DICT_BINS_MAX)

#define D3LUT_DICT_BKT_MASK         (0x3FF) /* 1024 buckets */
#define D3LUT_DICT_BKT_POS          (6)     /* MSB bits (15 - 6) of hash */

#define D3LUT_DICT_BKT_IDX(hash16)  \
    ((hash16 >> D3LUT_DICT_BKT_POS) & D3LUT_DICT_BKT_MASK)


/** D3LUT Bloom Filter sizing. */
#define D3LUT_BFLT_BITS             (64 * 1024) /* 16bit hash index */
#define D3LUT_BFLT_WORDS            (D3LUT_BFLT_BITS / 32)


/**
 * -----------------------------------------------------------------------------
 * Section: D3LUT Statistics
 * -----------------------------------------------------------------------------
 */

#if defined(CC_D3LUT_STATS)
#define D3LUT_STATS_EXPR(expr)      expr
#else
#define D3LUT_STATS_EXPR(expr)      D3LUT_NONE
#endif


/**
 * -----------------------------------------------------------------------------
 * Section: D3LUT Debug
 * -----------------------------------------------------------------------------
 */

/** Errors always enabled ... avoid using PRINT, use TRACE or PTRACE */
#define D3LUT_ERROR(fmt, arg...) \
    printk(CLRr "%s: " fmt CLRnl, __FUNCTION__, ##arg)
#define D3LUT_PRINT(fmt, arg...) \
    printk(CLRg "%s: " fmt CLRnl, __FUNCTION__, ##arg)

/** Assert and Warnings enabled at level 1 and above */
#if (CC_D3LUT_DEBUG >= 1)
#define D3LUT_ASSERT(exp)           BCM_ASSERT_A(exp)
#define D3LUT_WARN(fmt, arg...) \
    printk(CLRm "%s: " fmt CLRnl, __FUNCTION__, ##arg)
#else
#define D3LUT_ASSERT(exp)           D3LUT_NOOP
#define D3LUT_WARN(fmt, arg...)     D3LUT_NOOP
#endif /* CC_D3LUT_DEBUG < 1 */

/** Function entry and Trace enabled at level 2 and above */
#if (CC_D3LUT_DEBUG >= 2)
#define D3LUT_FUNC()                printk(CLRb "%s" CLRnl, __FUNCTION__)
#define D3LUT_TRACE(fmt, arg...) \
    printk(CLRb "%s: " fmt CLRnl, __FUNCTION__, ##arg)
#else
#define D3LUT_FUNC()                D3LUT_NOOP
#define D3LUT_TRACE(fmt, arg...)    D3LUT_NOOP
#endif /* CC_D3LUT_DEBUG < 2 */

#if (CC_D3LUT_DEBUG >= 3)
#define D3LUT_PFUNC()               printk(CLRb "%s" CLRnl, __FUNCTION__)
#define D3LUT_PTRACE(fmt, arg...) \
    printk(CLRc "%s: " fmt CLRnl, __FUNCTION__, ##arg)
#else
#define D3LUT_PFUNC()               D3LUT_NOOP
#define D3LUT_PTRACE(fmt, arg...)   D3LUT_NOOP
#endif /* CC_D3LUT_DEBUG < 3 */


/** Formatted Ethernet Mac Address display */
#define D3LUT_SYM_FMT               "SYM<%02X:%02X:%02X:%02X:%02X:%02X> "
#define D3LUT_SYM_VAL(sym) \
    *((sym) + 0), *((sym) + 1), *((sym) + 2), \
    *((sym) + 3), *((sym) + 4), *((sym) + 5)

#define D3LUT_KEY_FMT               "KEY<%1u:%01u:%04u::%3u> "
#define D3LUT_KEY_VAL(key) \
    (key).domain, (key).incarn, (key).index, D3LUT_ELEM_IDX((key).index)

#define D3LUT_ELEM_FMT              D3LUT_SYM_FMT D3LUT_KEY_FMT
#define D3LUT_ELEM_VAL(elem)        D3LUT_SYM_VAL((elem)->sym.v8), \
                                    D3LUT_KEY_VAL((elem)->key)

#define D3LUT_ELEM_PRINT(elem) \
    D3LUT_PRINT("\t"D3LUT_ELEM_FMT "\n", D3LUT_ELEM_VAL(elem))


/** Ensure that 802.3 mac address is 2 Byte aligned */
#define D3LUT_ASSERT_SYM(sym) \
    D3LUT_ASSERT(((sym) != NULL) && ((((uintptr_t)(sym)) & 1) == 0))


#if defined(CONFIG_SMP) || defined(CONFIG_PREEMPT)
#define D3LUT_LOCK(lut)             spin_lock_bh(&((lut)->lock))
#define D3LUT_UNLK(lut)             spin_unlock_bh(&((lut)->lock))
#else
#define D3LUT_LOCK(lut)             local_irq_disable()
#define D3LUT_UNLK(lut)             local_irq_enable()
#endif  /* ! (CONFIG_SMP || CONFIG_PREEMPT) */

/**
 * =============================================================================
 * Section: D3LUT Bloom Filter
 *
 * Single <uint16_t> Hash bloom filter made of 64 Kbits.
 * hash = D3LUT_BFLT_HASH_INVALID is reserved.
 * =============================================================================
 */

#if defined(CC_D3LUT_BFLT)

#define D3LUT_BFLT_EXIST_NO         (0U)
#define D3LUT_BFLT_EXIST_MAYBE      (~0U)

#define D3LUT_BFLT_HASH_INVALID     ((uint16_t)0x0)

#define D3LUT_BFLT_EXPR(expr)       expr

typedef struct d3lut_bflt
{
    uint32_t        elem_total;             /* total number of elements */
    uint32_t        b64K[D3LUT_BFLT_WORDS]; /* bloom filter 64 Kbits bitmap */
    uint16_t        hash[D3LUT_ELEM_TOT];   /* per element 16 bit hash */
} d3lut_bflt_t;

#define D3LUT_BFLT_NULL             ((d3lut_bflt_t *) NULL)

uint32_t d3lut_bflt_exist(d3lut_bflt_t * d3lut_bflt, uint16_t hash);

#else  /* ! CC_D3LUT_BFLT */

#define D3LUT_BFLT_EXPR(expr)       D3LUT_NONE
static inline uint32_t d3lut_bflt_exist(void * d3lut_bflt, uint16_t hash)
{ return ~0; }

#endif /* ! CC_D3LUT_BFLT */


/**
 * =============================================================================
 * Section: D3LUT Core Structures
 * =============================================================================
 */


/**
 * -----------------------------------------------------------------------------
 * Section: D3LUT Element in a Dictionary.
 *
 * Dictionary is like an associative array, accessed using a "symbol" to
 * retrieve a 16 bit key made of a <12 bit index, 2 bit incarn, 2 bit domain>
 *
 * Domain serves as a pool index. As each WLAN domain has a 1:1 pool, begining
 * with pool index 0, a key's domain also refers to the WLAN radio unit and
 * correspondingly the WFD index.
 *
 * The D3LUT element's key::index may be used in a 1:1 algorithm for locating
 * a user context.
 * -----------------------------------------------------------------------------
 */

union d3lut_sym                         /* d3lut_sym_t */
{
    uint8_t          v8[D3LUT_DICT_SYM_SIZE]; /* 802.3 (D3) MacAddress */
    uint16_t        v16[D3LUT_DICT_SYM_SIZE / 2];
};

/** d3lut_key_t is union pktfwd_key_t */

/**
 * -----------------------------------------------------------------------------
 * D3LUT table's (hash) bins point to a d3lut_elem_t.
 * -----------------------------------------------------------------------------
 */

struct d3lut_elem                   /* d3lut_elem_t */
{
    d3lut_elem_t  * next;           /* free list, (collision sll?) */
                                    /* D3LUT Dictionary <sym,key> element */
    d3lut_sym_t     sym;            /* 16b aligned 6 Byte 802.3 MAC Address */
    d3lut_key_t     key;            /* 16 bit Key <domain, incarn, index> */

    /* d3fwd_ext_t is a d3lut_elem_t "by inheritance", as opposed to a has-a
     * composition. d3fwd_ext_t extends d3lut_elem_t in a 1:1 relationship and
     * memory management of d3lut_elem_t is leveraged for d3fwd_ext_t.
     *
     * See d3lut_elem_init() and Element Free Pool Management in d3lut_t.
     */
    d3fwd_ext_t     ext;

    /* station list associated with this d3lut_elem */
    dll_t           sta_list;

} __attribute__ ((packed));


void d3fwd_elem_dump(d3lut_elem_t *d3lut_elem);


/**
 * -----------------------------------------------------------------------------
 *
 * Section: D3LUT Dictionary as a hash table with last hit cached entry
 * Implementation uses a two dimensional bkts of bins, with 4 collision bins.
 *
 * Optionally, fixed size bins may be eliminated by linking d3lut_elem in a
 * single linked list to form a collision list. If using a sll collision list
 * then need to hold the d3lut::lock on collision list walk. (wl_pktfwd_match)
 *
 * -----------------------------------------------------------------------------
 */

/** D3LUT Hash table bin */
struct d3lut_bins                   /* d3lut_bins_t */
{
    d3lut_elem_t  * bins[D3LUT_DICT_BINS_MAX];
};

/** D3LUT Hash table bucket of bins */
struct d3lut_bkts                   /* d3lut_bkts_t */
{
    d3lut_bins_t    bkts[D3LUT_DICT_BKTS_MAX];
};


/**
 * -----------------------------------------------------------------------------
 * D3LUT Fast lookup of cached last hit hash
 * -----------------------------------------------------------------------------
 */

union d3lut_last                    /* d3lut_last_t */
{
    uint32_t       v32;
    struct {
        uint16_t   hash;            /* Last hit hash value */
        uint16_t   elem;            /* Last hit element index */
    };
};

/**
 * -----------------------------------------------------------------------------
 * D3LUT Dictionary using a last hit lkup and hash table of buckets of bins
 * -----------------------------------------------------------------------------
 */

struct d3lut_dict                   /* d3lut_dict_t */
{
    uint32_t        lookups;        /* count of number of lookups */
    uint32_t        entries;        /* count of entries in dictionary */
    uint32_t        failures;       /* count of failures */

    d3lut_last_t    last[D3LUT_LAST_SLOTS_MAX]; /* last hit cache */
    d3lut_bkts_t    htable;         /* Hash table: buckets and bins */

    D3LUT_STATS_EXPR(               /* statistics */
        struct d3lut_dict_stats {
            uint32_t    last;       /* matches using last cached lkup */
            uint32_t    hits;       /* matches using dict lkup */
            uint32_t    miss;       /* matches using dict lkup */
            uint32_t    coll;       /* count of collisions */
            uint32_t    inss;       /* entries inserted into hash table */
            uint32_t    dels;       /* entries deleted from hash table */
            uint32_t    dups;       /* duplicate elements in hash table */
            uint32_t    errs;       /* errors in dict management */
        } stats;
    )
};


/**
 * -----------------------------------------------------------------------------
 * Section: Element Free Pool Management
 * BCM_PKTFWD defines number of WLAN radio instances. Free list of d3lut_elem_t
 * are maintained per WLAN radio and one for all LAN/WAN.
 *
 * Pool manager used for dynamic allocation and free.
 * Alternatively, if an index is allocated externally, then the pool elem_base
 * may be used to select an element from the pool. See d3lut_ins(), where index
 * may be explicitly specified.
 * -----------------------------------------------------------------------------
 */

typedef union d3lut_policy
{
    uint16_t        v16;
    uint32_t        v32;
    uint32_t        pool_by_index;  /* user selects an element in pool */
    uint32_t        pool_freelist;  /* allocate using free pool value (~0U) */
} d3lut_policy_t;

struct d3lut_pool
{
    d3lut_elem_t  * elem_head;      /* free list of d3lut elements */
    d3lut_elem_t  * elem_tail;      /* free list of d3lut elements */
    uint16_t        elem_free;      /* num d3lut elements in free list */
    uint16_t        elem_max;       /* maximum number of elements */
    d3lut_policy_t  policy;         /* allocation policy */
    d3lut_elem_t  * elem_base;      /* memory block of all d3lut elements */
};


/**
 * =============================================================================
 * Section: D3LUT System
 * =============================================================================
 */

struct d3lut                        /* d3lut_t */
{
    spinlock_t      lock;           /* spinlock for exclusive access */

    d3lut_dict_t  * dict;

    D3LUT_BFLT_EXPR(d3lut_bflt_t * bflt;)

    d3lut_pool_t    pool[D3LUT_POOL_TOT];
    d3lut_elem_t  * elem_base;      /* memory block of all d3lut elements */

    D3LUT_STATS_EXPR(               /* statistics */
        struct d3lut_stats {
            uint32_t    elem_gets;      /* count of d3lut element allocations */
            uint32_t    elem_puts;      /* count of d3lut element frees */
            uint32_t    elem_errs;      /* errors due to depleted pool */
        } stats;
    )

};

extern d3lut_t * d3lut_gp;

/**
 * =============================================================================
 * Section: D3LUT Functional Interface
 * =============================================================================
 */


/** D3LUT apis below do not take a lock. Caller needs to lock appropriately */

/** D3LUT System ------------------------------------------------------------ */
/** D3LUT Initialize */
extern d3lut_t        * d3lut_init(uint32_t d3lut_elem_tot);
/** D3LUT Finalize (delete) */
extern void             d3lut_fini(d3lut_t * d3lut);
/** D3LUT Debug Dump */
extern void             d3lut_dump(d3lut_t * d3lut);

/** D3LUT private functions for conversion */
extern d3lut_elem_t   * d3lut_k2e(d3lut_t * d3lut, d3lut_key_t key);
extern d3lut_key_t      d3lut_e2k(d3lut_t * d3lut, d3lut_elem_t * elem);


/** D3LUT Pool Management --------------------------------------------------- */
/** D3LUT Pool configuration */
extern void             d3lut_policy_set(d3lut_t * d3lut,
                                  uint32_t pool, uint32_t d3lut_policy);
/** D3LUT element allocation. Not for general use. Used by dictionary  */
extern d3lut_elem_t   * d3lut_get(d3lut_t * d3lut,
                                  uint32_t pool, d3lut_policy_t d3lut_policy);
/** D3LUT element deallocation. Not for general use. Used by dictionary  */
extern void             d3lut_put(d3lut_t * d3lut,
                                  d3lut_elem_t * d3lut_elem); /* dealloc */


/** D3LUT Dictionary -------------------------------------------------------- */
/** D3LUT Dictionary hash function */
extern uint16_t         d3lut_hash(const uint8_t * sym);
/** D3LUT Dictionary - insert an element, invokes d3lut_get() */
extern d3lut_elem_t   * d3lut_ins(d3lut_t * d3lut, uint8_t * sym, uint32_t pool,
                                  d3lut_policy_t d3lut_policy);
/** D3LUT Dictionary - delete an element, invoked d3lut_put().
 *                     May only access the returned d3lut_elem::key::index
 */
extern d3lut_elem_t   * d3lut_del(d3lut_t * d3lut, uint8_t * sym,
                                  uint32_t pool);
/** D3LUT Dictionary - lookup a symbol in dictionary and return key */
extern d3lut_elem_t   * d3lut_lkup(d3lut_t * d3lut, uint8_t * sym,
                                   uint32_t pool);
/** D3LUT Dictionary - clear all entries with a matching d3fwd_wlif */
extern void             d3lut_clr(d3lut_t * d3lut, void * ext,
                                  bool ignore_ext_match);

extern void             d3lut_stats_clr(d3lut_t * d3lut);

#endif /* BCM_D3LUT */

#endif /* BCM_PKTFWD */

#endif /* __bcm_pktfwd_h_included__ */
