/*
	<:copyright-BRCM:2024:DUAL/GPL:standard

	Copyright (c) 2024 Broadcom
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
#ifndef __WKBUFF_H_INCLUDED__
#define __WKBUFF_H_INCLUDED__

/*
 * =============================================================================
 *
 * File Name  : wkbuff.h
 *
 * Description: Definition of a Wireless Kernel Buffer (WKB) packet context.
 *
 * The wkbuff_t layout has been adapted from the Dongle lbuf_t structure that
 * is embedded in the data-buffer.
 *
 * Refer bcm_pkt_lengths.h for BPM buffer layout
 *
 * wkbuff_t based packet context, a 208 Byte cache-line aligned region is embedded
 * in the data-buffer and is composed of sub-contexts,
 * namely
 *  - 48 Byte CONTROL
 *      - 32B SW: Maintains a packet databuffer in packet lists and
 *                tracks a packet's memory and priority attributes
 *      - 16B HW: Region accommodates 4 miscellaneous parameters per packet.
 *                Parameters are primarily for HW_SPC actions that may be
 *                performed, namely, SPC_RX or SPC_TX operations
 *  - 96 Byte FRAGMENT:
 *      - Carries the fragment information of 8 MSDUs like Buffer Memory
 *        attributes (2B frag_key), 2B Length and 8B Data pointer
 *  - 64 Byte EXTENSION
 *      - 48B WLAN  : WLAN UpperMAC Packet Tag and referenced as PKTTAG().
 *                    Used to cache computation results that need to be
 *                    persistent for the lifetime of the packet in the
 *                    WLAN device driver.
 *      - 16B Linux : a 8B back pointer pNbuff to SKB|FKB and a 8B pad
 *
 * Implementation Note:
 *
 *  - Common APIs provided: pushing, pulling, reading, writing, freeing
 *  - Frag '0' databuffer hosting wkbuff_t should always be a buffer with
 *    BPM layout
 *  - WKB packet context pointers are encoded using NBuff type WKBUFF_PTR
 *
 * ---------------------------------------------------------------------------
 * Naming Convention: MUST USE HW_SPC namespace and WKB prefix
 *  HWA2.x Series reserved for Dongle Mode
 *  HWA4.x Series reserved for NIC Mode, aka HW_SPC
 *
 *  wkbuff_t structure and definitions must be accessible
 *      to Network Processor,
 *      to HW_SPC device driver,
 *      to libraries like PKTFWD.
 *
 * All fields use Little Endian format, ARMv7 or ARMv8 Linux SDK.
 *
 * ---------------------------------------------------------------------------
 *
 * REFERENCE: hnd_pp_lbuf.h struct lbuf known to HWA Packet Pager as 3 sections
 *          hnd_pp_lbuf_control_t   control
 *          hnd_pp_lbuf_pkttag_t    pkttag
 *          hnd_pp_lbuf_fraginfo_t  fraginfo
 *
 * =============================================================================
 */

#if defined(CONFIG_BCM_WKB)
#include <linux/types.h>            /* include ISO C99 inttypes.h             */
#include <linux/kernel.h>
#include "bcm_assert.h"
#include <bcm_pkt_lengths.h>

/**
 * -----------------------------------------------------------------------------
 * Section : Macros used in WKB Namespace
 * -----------------------------------------------------------------------------
 */
#define WKB_NOOP                    do { /* no-op */ } while (0)

/** Error/Warn/Debug Control */
/* XXX: BCM_WKB_TEST: Change the value to "0" */
#if IS_ENABLED(CONFIG_BCM_WLAN_REMOVE_INTERNAL_DEBUG)
#define CC_WKB_DEBUG                (0)
#else /* ! CONFIG_BCM_WLAN_REMOVE_INTERNAL_DEBUG */
#define CC_WKB_DEBUG                (1)
#endif /* ! CONFIG_BCM_WLAN_REMOVE_INTERNAL_DEBUG */

/** Errors always enabled ... avoid using PRINT, use TRACE or PTRACE */
#define WKB_ERROR(fmt, arg...)      \
    printk(CLRr "%s: " fmt CLRnl, __FUNCTION__, ##arg)
#define WKB_PRINT(fmt, arg...)      \
    printk(CLRg "%s: " fmt CLRnl, __FUNCTION__, ##arg)

#define WKB_BUG(exp)                do { if (!(exp)) BUG(); } while (0)

/** Assert and Warnings enabled at level 1 and above */
#if (CC_WKB_DEBUG >= 1)
#define WKB_ASSERT(exp)             BCM_ASSERT_A(exp)
#define WKB_WARN(fmt, arg...)       \
    printk(CLRm "%s: " fmt CLRnl, __FUNCTION__, ##arg)
#else
#define WKB_ASSERT(exp)             WKB_NOOP
#define WKB_WARN(fmt, arg...)       WKB_NOOP
#endif /* CC_WKB_DEBUG < 1 */

/** Function entry and Trace enabled at level 2 and above */
#if (CC_WKB_DEBUG >= 2)
#define WKB_FUNC()                  printk(CLRb "%s" CLRnl, __FUNCTION__)
#define WKB_TRACE(fmt, arg...)      \
    printk(CLRb "%s: " fmt CLRnl, __FUNCTION__, ##arg)
#else
#define WKB_FUNC()                  WKB_NOOP
#define WKB_TRACE(fmt, arg...)      WKB_NOOP
#endif /* CC_WKB_DEBUG < 1 */

/** Per "P"acket Function entry and Trace enabled at level 3 and above */
#if (CC_WKB_DEBUG >= 3)
#define WKB_PFUNC()                 printk(CLRb "%s" CLRnl, __FUNCTION__)
#define WKB_PTRACE(fmt, arg...)     \
    printk(CLRc "%s: " fmt CLRnl, __FUNCTION__, ##arg)
#else
#define WKB_PFUNC()                 WKB_NOOP
#define WKB_PTRACE(fmt, arg...)     WKB_NOOP
#endif /* CC_WKB_DEBUG < 3 */

/** WKB return codes */
#define WKB_FAILURE                 (-1)
#define WKB_SUCCESS                 (0)

/* WKB address print format */
#define WKB_ADDR_FMT                "0x%px"
#define WKB_ADDR_VAL(ptr)           ((void *)(ptr))

/**
 * HW_SPC Defaults match these Data Buffer Configuration
 *
 * wkbuff_t is placed at the start of databuffer.
 *  Headroom starts at an offset of 512B from start of Databuffer
 *  512 Byte Headroom is large enough to carry a RxFIFO1 RxSts+UcSTs+D11_Hdr
 *  Data Payload room is 2KBytes [FIXME: Only Mgmt frames need 2KBytes]
 *  Optional tailroom of 64Bytes to make skb::end 64Byte cacheline aligned 
 *
 *  WKB offset should align with BPM buffer offsets defined in bcm_pkt_lengths.h
 */

                                        /* HW_SPC_DB_WKB_OFFSET_REG audit */
#define WKB_DB_CTXT_OFFSET      BCM_PKT_WKB_OFFSET
#define WKB_DB_CTXT_BYTES       BCM_PKT_WKB_BYTES

                                        /* HW_SPC_DB_TLM_OFFSET_REG audit */
#define WKB_DB_TLM_OFFSET       BCM_PKT_TLM_OFFSET
#define WKB_DB_TLM_BYTES        BCM_PKT_TLM_BYTES

                                        /* HW_SPC_DB_HDRM_OFFSET_REG audit */
#define WKB_DB_HDRM_OFFSET      BCM_PKT_HEADROOM_OFFSET
#define WKB_DB_HDRM_BYTES       BCM_PKT_HEADROOM    /* RxFIFO1 [RxS+UcS+D11] */

                                        /* HW_SPC_DB_PYLD_OFFSET_REG audit */
#define WKB_DB_PYLD_OFFSET      BCM_PKT_PYLD_OFFSET
#define WKB_DB_PYLD_BYTES       BCM_PKT_PYLD_BYTES  /* 2KB for 802.3, RxFIFO0 */

#define WKB_DB_BYTES            BCM_PKTBUF_SIZE

/**
 * HW_SPC defaults match these WKB Packet Context Configuration
 *
 * 208Byte Packet CONTEXT       :   CONTROL 48B + FRAGMENT 96B + EXTNSW 64B
 *
 *  CONTROL 48B         =   CtrlSW 32B + CtrlHW 16B
 *      ControlSW 32B pktparm   :   next link data len prio, frag control
 *      ControlHW 16B FIFOparm  :   wlFlowInf/rxfifo or txfifo parameters
 *
 *  FRAGMENT 96B        =   FRAGCNT_MAX=8 * FRAGITM_SIZE 12B
 *      Fragment_Item 12B       :   FragLen 2B + FragData64 8B + FragKey 2B
 *
 *  EXTENSION 64B       =   EXTENSION_WLAN 48B + EXTENSION_LINUX 16B
 *      Extension WLAN 48B      :   pkt_tag 48B
 *      Extension LINUX 16B     :   pkt_ref 8B + pkt_mgr 4B + pkt_pad 4B
 *      Extension INFO 0B       :   pkt_info 0B
 *
 */

#define WKB_CTRLSW_BYTES        32      /* HW_SPC_WKB_CTRLSW_BYTES_REG audit */
#define WKB_CTRLHW_BYTES        16      /* HW_SPC_WKB_CTRLHW_BYTES_REG audit */

                                        /* HW_SPC_WKB_FRAGMEM_BYTES_REG audit */
#define WKB_FRAGITM_BYTES       12      /* frag: len 2B + data 8B + key 2B */

#define WKB_FRAGCNT_MAX         8       /* HW_SPC_WKB_FRAGCNT_MAX_REG audit */
#define WKB_FRAGMEM_BYTES       (WKB_FRAGCNT_MAX * WKB_FRAGITM_BYTES)

/* 802.11 requirement to align all AMSDU subframes (except last)
 * length to 4Bytes
 */
#define WKB_FRAG_LEN_ALIGN      4

/**
 * A Extension SW Structure EXTNSW is composed of :
 *
 *  Extension for WLAN UMAC     EXTNWL 48Byte section
 *      pkt_tag 48B             :   WLAN UpperMAC Packet Tag storage
 *
 *  Extension for LINUX OS      EXTNLX 16Byte section
 *      pkt_ref 8B              :   Back pointer to NTK buffer pNbuff fkb|skb
 *      pkt_mgr 4B              :   OS field, e.g. SW Pool Manager
 *      pkt_pad 4B              :   Align pkt_tag on a 64B cacheline boundary
 *
 *
 *  Extension for Other INFO    EXTNIF 0Byte
 *      pkt_info 0B
 */
                                        /* HW_SPC_WKB_EXTNSW_BYTES_REG audit */
#define WKB_EXTNWL_BYTES        48      /* Extension WLAN UpperMAC */
#define WKB_EXTNLX_BYTES        16      /* Extension LINUX */

#define WKB_EXTNIF_BYTES        0       /* Extension INFO */

#define WKB_EXTNSW_BYTES        \
    (WKB_EXTNWL_BYTES + WKB_EXTNLX_BYTES + WKB_EXTNIF_BYTES)

/**
 * sizeof(wkbuff_t) = WKB_CTX 208B = CTRL 48B + FRAG 96B + EXTNSW 64B
 */
#define WKB_CTRL_BYTES          \
    (WKB_CTRLSW_BYTES + WKB_CTRLHW_BYTES)
#define WKB_FRAG_BYTES          WKB_FRAGMEM_BYTES
#define WKB_EXTN_BYTES          WKB_EXTNSW_BYTES

#define WKB_CTXT_BYTES          \
    (WKB_CTRL_BYTES + WKB_FRAG_BYTES + WKB_EXTN_BYTES)

/* To derive a HW_SPC_DB_CTXT_BYTES, include WKB_CTXT_PAD_BYTES
 *  HW_SPC bezeros the 48Bytes following the 208Byte sized wkbuff_t
 */
#define WKB_DB_CTXT_PAD_BYTES   48

/**
 * WKB fragment databuffer memory pool identification
 *
 *  Defines the source Pool Manager : either MPM, BPM or Linux Pool
 *
 *  If a fragment databuffer is-not MPM then
 *          wkb::ctrlsw::frag_flags bit#frag_ix is set
 */
typedef enum wkb_db_pool
{
    WKB_DB_MPM      = 0,            /* Hardware Memory Pool Manager */
    WKB_DB_BPM      = 1,            /* Software Buffer Pool Manager */
    WKB_DB_LNX      = 2,            /* Linux kmem cache */
    WKB_DB_MAX      = 3,            /* MAX number of pools supported */
} wkb_db_pool_t;

/* WKB can carry only one pNBuff (pkt_ref) and the corresponding data
 * buffer will be filled at frag_idx "1"
 */
#define WKB_LNX_FRAG_IDX        1

/**
 * WKB pointer address format
 *
 *  HW: DDR Physical address accessed
 *  SW: MMU mapped Virtual address accessed
 *
 *  HW_SPC is able to process virtual addresses pointing to MPM region
 */
typedef struct wkb_db_pa
{
    uint32_t        lo;             /* low 32 bits of a DB PA 64 bits */
    uint32_t        hi;             /* high 32 bits of a DB PA 64 bits */
} wkb_db_pa_t;                      /* WKB Data Buffer Physical Address */


/**
 * A wkbuff_t packet context includes 4 addresses into a databuffer, namely,
 *      wkb::CTRLSW::next       WKB SDU list, large AMSDU aggsf
 *      wkb::CTRLSW::link       WKB PDU list
 *      wkb::CTRLSW::data       WKB SDU data start address
 *      wkb::FRAGMEM::frag_data[]       WKB SDU data start address
 *
 * 1. HW_SPC READ ACCESS: Must perform
 *      - a V2P on 64bit values that fall in the HW_SPC_V2P Translation Window.
 *      - Reset the NBuff type code
 * 2. HW_SPC WRITE ACCESS: Must perform
 *      - a P2V on a 64bit value that fall in the HW_SPC_P2V Translation Window.
 *      - Encode NBuff type WKBUFF_PTR for WKB next and link pointers
 *
 * All MPM originating databuffers at HW_SPC Mappable and SW driver interprets
 * these values as 64bit C pointers. On a 32bit ARMv7, the upper 32bits are not
 * referenced on pointer accesses.
 *
 * See also, the back reference wkb::EXTNSW::pkt_ref .
 *      pkt_ref is used as a va_p to the top of the databuffer (pNbuff).
 * See also, wkb::FRAGMEM::frag_key[]::db_sob_lo12 databuffer
 */
typedef union wkb_db_addr
{
    wkb_db_pa_t     pa;             /* HW - DDR Physical Address */
    uint64_t        pa64;
    void          * va;             /* SW - MMU Virtual Address, C pointer */
    uint64_t        va64;
} wkb_db_addr_t;                    /* WKB address 64 bit format */


/**
 * Ability to identify the start of a databuffer, for freeing.
 *
 * When a fragment is added to a wkbuff_t, the lower bits of a databuffer's
 * start address are saved in the wkb::FRAGMEM::frag_key::db_sob_lo12
 *
 */

#define WKB_DB_SOB_LO12_NBITS       12      /* low12 bits of the databuf start */
#define WKB_DB_SOB_LO12_MASK        \
    ((1 << WKB_DB_SOB_LO12_NBITS) - 1)

/* Given a buffer address, returns lower "12" bits of */
#define WKB_DB_ADDR_TO_DB_SOB_L012(db_addr) \
    ((uint16_t)(uintptr_t)(db_addr) & WKB_DB_SOB_LO12_MASK)
/* Given a data pointer and sob_l012, return buffer start address */
#define WKB_DB_GET_SOB(data, db_lo)        \
    ((void *)((((uintptr_t)(data) - (db_lo)) & ~WKB_DB_SOB_LO12_MASK) | db_lo))

/**
 * A frag_key 16bits describes attributes of a databuffer
 *  db_pool             2 bits      Data Buffer Pool Manager
 *  db_is_pa            1 bit       indicate if ptrs are in VA or PA format
 *  db_has_telemetry    1 bit       indicate if buffer has telemetry
 *  db_sob_lo12         12 bits     databuffer start address 0:12 bits
 */
typedef union wkb_db_key
{
    uint16_t        key16;          /* key 16bit : databuffer attributes */
    struct {
        uint16_t        db_pool             : 2;    /* wkb_db_pool_t */
        uint16_t        db_is_pa            : 1;    /* VA or PA */
        uint16_t        db_has_telemetry    : 1;    /* Telemetry Timestamp */
        uint16_t        db_sob_lo12         : 12;   /* Lower 12 bits of SOB */
    };
} wkb_db_key_t;                     /* WKB databuffer key16bits */

#define WKB_DB_KEY_FMT          " KEY<%x:%x:%x:%x>"
#define WKB_DB_KEY_VAL(key)     \
        (key).db_pool, (key).is_pa, (key).db_has_telemetry, (key).db_lo12

/**
 * wkbuff_t::flags 8bits describes attributes of a Packet (WKB)
 */

#define WKB_FLAGS_SE_BITS       2   /* Source Entity (wkb_se_t) */
#define WKB_FLAGS_SE_MASK       ((1 << WKB_FLAGS_SE_BITS) - 1)
#define WKB_FLAGS_RSVD_BITS     6   /* Reserved Flags */

#define WKB_FLAGS_SET_SE(_se)  \
    (((_se) << WKB_FLAGS_SE_BITS) & WKB_FLAGS_SE_MASK)

#define WKB_FLAGS_GET_SE(_flags)  \
    (((_flags) & WKB_FLAGS_SE_MASK) >> WKB_FLAGS_SE_BITS)

/**
 * WKB Source Entity
 *  Defines the source of Packet : either NP, WL or LX
 */

typedef enum wkb_se
{
    WKB_SE_RSVD         = 0,    /* Intently RSVD for PKT cloning */
    WKB_SE_NP           = 1,    /* Network Processor (XBOW/RUNNER) */
    WKB_SE_WL           = 2,    /* WLAN D11MAC */
    WKB_SE_LX           = 3,    /* Linux Network Stack */
} wkb_se_t;


/**
 * ----------------------------------------------------------------------------
 *  WKB: Wireless Kernel Buffers placed at start of a Data Buffer
 * ----------------------------------------------------------------------------
 */
typedef struct wkbuff
{
    struct {                        /* CONTROL SECTION BGN - 48 Bytes*/
        union {                     /* CTRLSW SUBSECTION BGN - 32 Bytes */
            uint8_t         ctrlsw_u8[WKB_CTRLSW_BYTES];
            struct {
                wkb_db_addr_t   next;       /* >FRAGCNT_MAX MSDU list */
                wkb_db_addr_t   link;       /* MPDU list */
                wkb_db_addr_t   data;       /* Start of data */

                uint8_t         flags;      /* Packet flags */
                uint8_t         prio    : 3;       /* Packet priority */
                uint8_t         subunit : 5;       /* WLAN interface index */
                uint16_t        len;        /* Packet length */

                                            /* Fragment control word */
                uint8_t         frag_flags; /* has non-MPM fragments */
                uint8_t         frag_num;   /* No of frags in frags_array */
                uint16_t        frag_totlen; /* Total length of fragments */
            };                      /* ctrlsw_u8 */
        };                          /* CTRLSW SUBSECTION END */

        union {                     /* CTRLHW SUBSECTION BGN - 16 Bytes */
            uint8_t         ctrlhw_u8[WKB_CTRLHW_BYTES];
            uint32_t        hw_misc[4];     /* HW_SPC metadata, wlFlowInf */

            struct {

                struct {            /* Egress HW (WLAN HWA3b) hw_misc[0,1] */
                    uint8_t         num_desc;           /* Descs used for PDU */
                    uint8_t         LLCSNAP_ins_flags;  /* MAC TX LLC offload */
                    uint16_t        amsdu_total_len;    /* Total len of PDU */
                    uint32_t        txdma_flags;        /* TxFIFO dma flags */
                } txfifo;

                struct {            /* Ingress HW (CNP) hw_misc[2,3] */
                    wlFlowInf_t     wl;                 /* WLAN flow info */
                    uint32_t        hw_misc_3;          /* Not Used */
                };  /* txpost */
            };
        };                          /* CTRLHW SUBSECTION END */

    };                              /* CONTROL SECTION END */

    union {                         /* FRAGMEM SECTION BGN - 96 Bytes */
        /* FRAGITM 12 Bytes = frag_len 2B + frag_data 8B + frag_key 2B */
        uint8_t         fragmem_u8[WKB_FRAGMEM_BYTES];
        struct {
            uint16_t        frag_len[WKB_FRAGCNT_MAX];
            wkb_db_addr_t   frag_data[WKB_FRAGCNT_MAX];
            wkb_db_key_t    frag_key[WKB_FRAGCNT_MAX];
        };
    };                              /* FRAGMENT SECTION END */

    union {                         /* EXTNSW SECTION BGN - 64 Bytes */
        uint8_t         extnsw_u8[WKB_EXTNSW_BYTES];
        struct {
            /* Extension WLAN UpperMAC  : WKB_EXTNWL_BYTES */
            uint8_t         pkt_tag[WKB_EXTNWL_BYTES];  /* cb[] */

            /* Extension LINUX          : WKB_EXTNLX_BYTES */
            wkb_db_addr_t   pkt_ref;        /* Back va_p to pNBuff_t */
            uint32_t        pkt_mgr;        /* Pool Manager Recycling */
            uint32_t        pkt_pad;        /* UNSUSED: pads to 64B cacheline */
        };                          /* extnsw_u8 */
    };                              /* EXTNSW SECTION END */

} wkbuff_t;

/* Total size of the wkbuff should not exceed BCM_PKT_WKB_BYTES */
static_assert (sizeof(wkbuff_t) == WKB_CTXT_BYTES);
static_assert (sizeof(wkbuff_t) <= WKB_DB_CTXT_BYTES);
static_assert ((WKB_CTXT_BYTES + WKB_DB_CTXT_PAD_BYTES) == WKB_DB_CTXT_BYTES);


/*
 *------------------------------------------------------------------------------
 * WKB Functional Interfaces
 *------------------------------------------------------------------------------
 */

/* Initialize the WKB context for a data buffer */
extern wkbuff_t *   wkb_init(void * pStart, uint8_t * data, uint16_t len, uint8_t prio,
        wkb_db_pool_t db_pool, wkb_se_t wkb_se, bool need_reset);

/* Allocates a BPM buffer and prepares WKB */
extern wkbuff_t *   wkb_alloc(uint16_t len, wkb_se_t wkb_se);

/* Free a WKB and fragment data buffers to its respective preallocated pool. */
extern void         wkb_free(wkbuff_t * wkb);

/* Returns Data buffer Start address in which given WKB is embedded */
extern void *       wkb_db_addr(wkbuff_t * wkb);

/* Returns the summation of all WKB SDUs (PDU) linked using next */
extern uint16_t     wkb_totlen(wkbuff_t * wkb);

/* Move the data buffers for WKBs with pNBUFF (pkt_ref) linked */
extern bool         wkb_realloc_headroom(wkbuff_t * wkb, int headroom);

/* Create a copy of both wkbuff and its data buffer */
extern wkbuff_t *   wkb_copy(wkbuff_t * wkb);


/** WKB Control SW section */

/**
 * Function     : wkb_next
 * Description  : Fetch the next packet
 */
static inline pNBuff_t
wkb_next(wkbuff_t * wkb)
{
    return (pNBuff_t)(wkb->next.va);
}

/**
 * Function     : wkb_set_next
 * Description  : Set next pointer of packet
 */
static inline void
wkb_set_next(wkbuff_t * wkb, pNBuff_t next)
{
    WKB_ASSERT((next == NULL) || IS_WKBUFF_PTR(next));
    wkb->next.va = next;
}

/**
 * Function     : wkb_link
 * Description  : Fetch the link pointer of packet
 */
static inline pNBuff_t
wkb_link(wkbuff_t * wkb)
{
    return (pNBuff_t)(wkb->link.va);
}

/**
 * Function     : wkb_set_link
 * Description  : Set link pointer of packet
 */
static inline void
wkb_set_link(wkbuff_t * wkb, pNBuff_t link)
{
    /* XXX: BCM_WKB_TEST: WLAN modules might link packets with different
     * nbuff type
     * Eg. DHD will link FKB (flow-cache) and SKB (Network Stack)
     * This ASSERT might not be valid. Review it
     */
    WKB_ASSERT((link == NULL) || IS_WKBUFF_PTR(link));
    wkb->link.va = link;
}

/**
 * Function     : wkb_head
 * Description  : Fetch the head pointer of packet
 */
static inline void *
wkb_head(wkbuff_t * wkb)
{
    return (void *)BCM_PWKBUFF_TO_PHEAD(wkb);
}

/**
 * Function     : wkb_end
 * Description  : Fetch the end of databuffer
 *
 * NOTE:
 * - Shareinfo region is not included for tailroom
 * - Offsets are valid only for fixed size buffers from BPM SW pool
 */
static inline void *
wkb_end(wkbuff_t * wkb)
{
    return (void *)BCM_PWKBUFF_TO_PSHINFO(wkb);
}

/**
 * Function     : wkb_data
 * Description  : Fetch the data pointer of packet
 */
static inline uint8_t *
wkb_data(wkbuff_t * wkb)
{
    return (uint8_t *)wkb->data.va;
}

/**
 * Function     : wkb_set_data
 * Description  : Set data pointer of packet
 */
static inline void
wkb_set_data(wkbuff_t * wkb, uint8_t * data)
{
    wkb->data.va = (void *)data;
}

/**
 * Function     : wkb_len
 * Description  : Return the length of a WKB
 */
static inline uint16_t
wkb_len(wkbuff_t * wkb)
{
    return wkb->len;
}

/**
 * Function     : wkb_set_len
 * Description  : Sets the length of a WKB
 */
static inline void
wkb_set_len(wkbuff_t * wkb, uint16_t len)
{
    WKB_ASSERT(len <= (WKB_DB_HDRM_BYTES + WKB_DB_PYLD_BYTES));
    wkb->len = len;
}

/**
 * Function     : wkb_headroom
 * Description  : Return the number of bytes of free space at the head of wkb
 */
static inline uint16_t
wkb_headroom(wkbuff_t * wkb)
{
    return (uint16_t)((uintptr_t)wkb_data(wkb) - (uintptr_t)wkb_head(wkb));
}

/**
 * Function     : wkb_tailroom
 * Description  : Return the number of bytes of free space at the tail of wkb
 *
 * Note: Doesn't work for variable sized MPM buffer
 */
static inline uint16_t
wkb_tailroom(wkbuff_t * wkb)
{
    uint8_t * tail = wkb_data(wkb) + wkb_len(wkb);
    return (uint16_t)((uintptr_t)wkb_end(wkb) - (uintptr_t)tail);
}

/**
 * Function     : wkb_push
 * Description  : Prepare space for data at head of the packet buffer.
 */
static inline uint8_t *
wkb_push(wkbuff_t * wkb, uint16_t bytes)
{
    WKB_ASSERT((wkb->frag_key[WKB_LNX_FRAG_IDX].db_pool == WKB_DB_LNX) ||
        (wkb_headroom(wkb) >= bytes));

    wkb_set_len(wkb, wkb_len(wkb) + bytes);
    wkb_set_data(wkb, wkb_data(wkb) - bytes);
    return wkb_data(wkb);
}

/**
 * Function     : wkb_pull
 * Description  : Delete data from the head of packet buffer.
 */
static inline uint8_t *
wkb_pull(wkbuff_t * wkb, uint16_t bytes)
{
    WKB_ASSERT(wkb_len(wkb) >= bytes);

    wkb_set_len(wkb, wkb_len(wkb) - bytes);
    wkb_set_data(wkb, wkb_data(wkb) + bytes);
    return wkb_data(wkb);
}

/**
 * Function     : wkb_put
 * Description  : Extends the used data area of the buffer.
 * If this would exceed the total buffer size the kernel will panic.
 * A pointer to the first byte of the extra data is returned.
 */
static inline uint8_t *
wkb_put(wkbuff_t * wkb, uint16_t bytes)
{
    uint8_t * tail;
    uint8_t * end;

    tail    = wkb_data(wkb) + wkb->len;
    end     = wkb_end(wkb);

    if ((tail + bytes) > end) {
        WKB_ERROR(" Requested bytes %d is out range; "
            "wkb " WKB_ADDR_FMT " tail " WKB_ADDR_FMT " end " WKB_ADDR_FMT"\n",
            bytes, WKB_ADDR_VAL(wkb), WKB_ADDR_VAL(tail), WKB_ADDR_VAL(wkb));
        WKB_BUG(0);
        return tail;
    }

    wkb_set_len(wkb, wkb_len(wkb) + bytes);
    return tail;
}

/**
 * Function     : wkb_prio
 * Description  : Return the priority of packet
 */
static inline uint8_t
wkb_prio(wkbuff_t * wkb)
{
    return wkb->prio;
}

/**
 * Function     : wkb_set_prio
 * Description  : Sets the priority of packet
 */
static inline void
wkb_set_prio(wkbuff_t * wkb, uint8_t prio)
{
    wkb->prio = prio;
}

/**
 * -----------------------------------------------------------------------------
 * Section : WKB FRAGMEM
 * -----------------------------------------------------------------------------
 */

/* Fills the fragment at index wkb::frag_num in a WKB */
extern int      wkb_frag_fill_item(wkbuff_t * wkb, void * pStart, uint8_t * data,
    uint16_t len, wkb_db_pool_t db_pool, bool align);

/* Fills the fragment at a given frag index */
extern int      wkb_frag_idx_fill_item(wkbuff_t * wkb, void * pStart,
    uint8_t * data, uint16_t frag_idx, uint16_t len, wkb_db_pool_t db_pool);

/* Given a WKB frag index, returns frag data buffer address */
extern void *   wkb_frag_db_addr(wkbuff_t * wkb, uint16_t frag_idx);

/* Iterate through wkb frags and prepare sdu (wkb::next based) list */
extern int      wkb_frags_release(wkbuff_t * wkb);

/**
 * Function     : wkb_frag_len
 * Description  : Return the length of a WKB fragment
 */
static inline uint16_t
wkb_frag_len(wkbuff_t * wkb, uint16_t frag_idx)
{
    WKB_ASSERT(frag_idx < WKB_FRAGCNT_MAX);
    return wkb->frag_len[frag_idx];
}

/**
 * Function     : wkb_set_frag_len
 * Description  : Sets the length of a WKB fragment
 */
static inline void
wkb_set_frag_len(wkbuff_t * wkb, uint16_t frag_idx, uint16_t frag_len)
{
    WKB_ASSERT(frag_idx < WKB_FRAGCNT_MAX);
    wkb->frag_totlen        -= wkb_frag_len(wkb, frag_idx);
    wkb->frag_totlen        += frag_len;
    wkb->frag_len[frag_idx] = frag_len;
}

/**
 * Function     : wkb_frag_data
 * Description  : Return the frag data pointer of a WKB fragment
 */
static inline uint8_t *
wkb_frag_data(wkbuff_t * wkb, uint16_t frag_idx)
{
    WKB_ASSERT(frag_idx < WKB_FRAGCNT_MAX);
    return (uint8_t *)wkb->frag_data[frag_idx].va;
}

/**
 * Function     : wkb_set_frag_data
 * Description  : Set data pointer of WKB fragment
 */
static inline void
wkb_set_frag_data(wkbuff_t * wkb, uint16_t frag_idx, uint8_t * data)
{
    WKB_ASSERT(frag_idx < WKB_FRAGCNT_MAX);
    wkb->frag_data[frag_idx].va = (void *)data;
}

/**
 * Function     : wkb_frag_headroom
 * Description  : Return the number of bytes of free space at the frag head
 */
static inline uint16_t
wkb_frag_headroom(wkbuff_t * wkb, uint16_t frag_idx)
{
    uint8_t * head;

    head = wkb_frag_db_addr(wkb, frag_idx) + BCM_PKT_HEADROOM_OFFSET;
    return (uint16_t)((uintptr_t)wkb_frag_data(wkb, frag_idx) - (uintptr_t)head);
}


/**
 * Function     : wkb_frag_push
 * Description  : Prepare space for data at head of the fragment buffer.
 */
static inline uint8_t *
wkb_frag_push(wkbuff_t * wkb, uint16_t frag_idx, uint16_t bytes)
{
    wkb_set_frag_len(wkb, frag_idx, wkb_frag_len(wkb, frag_idx) + bytes);
    wkb_set_frag_data(wkb, frag_idx, wkb_frag_data(wkb, frag_idx) - bytes);
    return wkb_frag_data(wkb, frag_idx);
}

/**
 * Function     : wkb_frag_pull
 * Description  : Delete data from the head of fragment buffer.
 */
static inline uint8_t *
wkb_frag_pull(wkbuff_t * wkb, uint16_t frag_idx, uint16_t bytes)
{
    wkb_set_frag_len(wkb, frag_idx, wkb_frag_len(wkb, frag_idx) - bytes);
    wkb_set_frag_data(wkb, frag_idx, wkb_frag_data(wkb, frag_idx) + bytes);
    return wkb_frag_data(wkb, frag_idx);
}

/**
 * -----------------------------------------------------------------------------
 * Section : WKB Extension SW
 * -----------------------------------------------------------------------------
 */


/* Release the pNBuff linked to WKB */
extern void *   wkb_release_pkt_ref(wkbuff_t * wkb);

/**
 * Function     : wkb_extnsw
 * Description  : Return WKB's extension SW
 */
static inline uint8_t *
wkb_extnsw(wkbuff_t * wkb)
{
    return wkb->extnsw_u8;
}

/**
 * Function     : wkb_extnsw_zero
 * Description  : Zero out the WKB's extension SW
 */
static inline void
wkb_extnsw_zero(wkbuff_t * wkb)
{
    memset(wkb->extnsw_u8, 0, WKB_EXTNSW_BYTES);
}

/**
 * Function     : wkb_pkt_ref
 * Description  : Return the packet reference (back pointer to pNBuff_t)
 */
static inline void *
wkb_pkt_ref(wkbuff_t * wkb)
{
    return wkb->pkt_ref.va;
}

/**
 * Function     : wkb_set_pkt_ref
 * Description  : Sets the priority of packet
 */
static inline void
wkb_set_pkt_ref(wkbuff_t * wkb, void * pkt_ref)
{
    wkb->pkt_ref.va = pkt_ref;
}

/**
 * Function     : wkb_pkt_tag
 * Description  : Return WKB's pkt_tag (extension WLAN)
 */
static inline void *
wkb_pkt_tag(wkbuff_t * wkb)
{
    return (void *)wkb->pkt_tag;
}

/**
 * Function     : wkb_pkt_tag_zero
 * Description  : Zero out the WKB's pkt_tag (extension WLAN)
 */
static inline void
wkb_pkt_tag_zero(wkbuff_t * wkb)
{
    memset(wkb->pkt_tag, 0, WKB_EXTNWL_BYTES);
}


#if (CC_WKB_DEBUG >= 1)
/**
 * -----------------------------------------------------------------------------
 * Section : WKB Debug Helpers
 * -----------------------------------------------------------------------------
 */

/* Dumps WKB attributes and data content of every fragment */
extern void     wkb_dump(wkbuff_t * wkb, const char *func,
    int line, int dump_level);
#endif /* CC_WKB_DEBUG */

#else /* ! CONFIG_BCM_WKB */


/* Dummy definition to support WKB nbuff type compilation when CONFIG_BCM_WKB
 * is not enabled
 */
typedef struct wkbuff {
    /** NOTHING TO DEFINE */
    void * dummy;
} wkbuff_t;

#endif /* ! CONFIG_BCM_WKB */
#endif  /* defined(__NBUFF_H_INCLUDED__) */
