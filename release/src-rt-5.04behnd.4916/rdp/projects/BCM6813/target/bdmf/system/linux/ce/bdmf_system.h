/*
 * <:copyright-BRCM:2019:DUAL/GPL:standard
 * 
 *    Copyright (c) 2019 Broadcom 
 *    All Rights Reserved
 * 
 * Unless you and Broadcom execute a separate written software license
 * agreement governing use of this software, this software is licensed
 * to you under the terms of the GNU General Public License version 2
 * (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
 * with the following added to such license:
 * 
 *    As a special exception, the copyright holders of this software give
 *    you permission to link this software with independent modules, and
 *    to copy and distribute the resulting executable under terms of your
 *    choice, provided that you also meet, for each linked independent
 *    module, the terms and conditions of the license of that module.
 *    An independent module is a module which is not derived from this
 *    software.  The special exception does not apply to any modifications
 *    of the software.
 * 
 * Not withstanding the above, under no circumstances may you combine
 * this software in any way with any other Broadcom software provided
 * under a license other than the GPL, without Broadcom's express prior
 * written consent.
 * 
 * :>
 */

#ifndef _BDMF_SYSTEM_H_
#define _BDMF_SYSTEM_H_


#include "bdmf_system_common.h"
#include <linux/nbuff.h>
#if (defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE))
#include <linux/gbpm.h>
#define BDMF_GBPM_TRACK_BUF(buf, value, info)         GBPM_TRACK_BUF(buf, GBPM_DRV_BDMF, value, info)
#define BDMF_GBPM_TRACK_SKB(skb, value, info)         GBPM_TRACK_SKB(skb, GBPM_DRV_BDMF, value, info)
#define BDMF_GBPM_TRACK_FKB(fkb, value, info)         GBPM_TRACK_FKB(fkb, GBPM_DRV_BDMF, value, info)
#else
#define BDMF_GBPM_TRACK_BUF(buf, value, info)         do{}while(0)
#define BDMF_GBPM_TRACK_SKB(skb, value, info)         do{}while(0)
#define BDMF_GBPM_TRACK_FKB(fkb, value, info)         do{}while(0)
#endif
#ifdef CONFIG_MIPS
#include <asm/r4kcache.h>
#endif
#include <bcm_prefetch.h>
#include <rdpa_types.h>
#include <linux/types.h>
#include <net/sock.h>

#define BDMF_RX_CSUM_VERIFIED_MASK   0x01
#define BDMF_RX_CSUM_VERIFIED_SHIFT  0

static inline int sim_mem_init(void) {return 1;}
static inline void sim_mem_destroy(void) {}

/*
*** RUNNER_MAX_GSO_FRAGS should match RDD_GSO_DESC_ENTRY_FRAG_DATA_NUMBER
*/
#define RUNNER_MAX_GSO_FRAGS 18

/** Invalidate dcache range
 * \param[in]   addr    start address
 * \param[in]   size    size
 */
static inline void bdmf_dcache_inv(unsigned long addr, unsigned long size)
{
    //blast_inv_dcache_range(addr, addr+size);
    cache_invalidate_len((void*)addr, size);
}

/** Flush dcache range
 * \param[in]   addr    start address
 * \param[in]   size    size
 */
static inline void bdmf_dcache_flush(unsigned long addr, unsigned long size)
{
    //blast_dcache_range(addr, addr+size);
    cache_flush_len((void*)addr, size);
}

/** Flush a range in FPM pool memory
 * \param[in]   addr    start address
 * \param[in]   size    size
 */
static inline void bdmf_fpm_dcache_flush(unsigned long addr, unsigned long size)
{
    fpm_cache_flush_len((void*)addr, size);
}

static inline bdmf_sysb_type bdmf_sysb_typeof(bdmf_sysb sysb)
{
    if ( IS_FKBUFF_PTR(sysb) )
    {
        return bdmf_sysb_fkb;
    }
    else
    {
        return bdmf_sysb_skb;
    }
}

/** Set headroom size for system buffer
 * \param[in]   sysb_type   System buffer type
 * \param[in]   headroom    Headroom size
 */
void bdmf_sysb_headroom_size_set(bdmf_sysb_type sysb_type, uint32_t headroom);

/** convert sysb to skb or fkb
 * \param[in]   sysb        System buffer
 * \return skb or fkb
 */
static inline void *bdmf_sysb_2_fkb_or_skb( bdmf_sysb sysb )
{
    return PNBUFF_2_PBUF(sysb);
}

extern struct sk_buff * skb_header_alloc(void);

/** Allocate one or more sk_buffs from the BPM SKB pool or Linux kmem cache.
 *  \param[in]  num_skbs    Number of sk_buff headers to allocate
 * \return Pointer to a list of sk_buffs, list is skb:next null terminated.
 */
static inline struct sk_buff *
bdmf_skb_header_alloc(uint32_t num_skbs)
{
    struct sk_buff *skb;

#if (defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE))
    skb = gbpm_alloc_mult_skb(num_skbs);
#else /* ! (CONFIG_BCM_BPM || CONFIG_BCM_BPM_MODULE) */
    skb = skb_header_alloc();
    if (skb != (struct sk_buff*)NULL)
    {
        skb->next = (struct sk_buff *)NULL;
    }
#endif /* ! (CONFIG_BCM_BPM || CONFIG_BCM_BPM_MODULE) */

    return skb;
}

static inline void
bdmf_skb_headerinit(struct sk_buff *skb, void *data, uint32_t len)
{
#if (defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE))

    gbpm_attach_skb(skb, data, len);

#else /* ! (CONFIG_BCM_BPM || CONFIG_BCM_BPM_MODULE) */

    skb_headerinit(BCM_PKT_HEADROOM,
#if defined(CC_NBUFF_FLUSH_OPTIMIZATION)
        SKB_DATA_ALIGN(len + BCM_SKB_TAILROOM),
#else
        BCM_MAX_PKT_LEN,
#endif
        skb, data, bdmf_sysb_recycle, 0, NULL);

    skb_trim(skb, len);
    skb->recycle_flags &= SKB_NO_RECYCLE;

#endif /* ! (CONFIG_BCM_BPM || CONFIG_BCM_BPM_MODULE) */

#if defined(CC_NBUFF_FLUSH_OPTIMIZATION)
    skb_shinfo(skb)->dirty_p = skb->data + BCM_DCACHE_LINE_LEN;
#endif

    bcm_prefetch(data); /* 1 BCM_DCACHE_LINE_LEN of data in cache */
}

#if (defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE))
static inline struct sk_buff *
bdmf_attach_skb_bpm(uint8_t *data, int data_offset_len)
{
    struct sk_buff *skb;

    skb = gbpm_alloc_skb();

    if (likely(skb)) { /* attach BPM skb and override gbpm_recycle_skb hook */
        gbpm_attach_skb(skb, data, data_offset_len);
        skb->recycle_hook = (RecycleFuncP) bdmf_sysb_recycle_skb;
    }

    return skb;
}
#endif /* CONFIG_BCM_BPM */

#define BDMF_SYSB_HEADER_ALLOC_5PARM

/** Allocate system buffer header i.e skb or fkb structure
 *  and initilize it with the provided len & data buffer
 * \param[in]   sysb_type   System buffer type
 * \param[in]   len         Data length
 * \return system buffer pointer.
 * If the function returns NULL, caller is responsible for "data" deallocation
 */
static inline bdmf_sysb 
bdmf_sysb_header_alloc(   bdmf_sysb_type      sysb_type, 
                          void*               datap,
                          uint32_t            data_offset,
                          uint32_t            len, 
                          uint32_t            context, 
                          int                 flags)
{
    if( sysb_type == bdmf_sysb_fkb )
    {
        FkBuff_t *fkb;

        fkb = fkb_init(datap, BCM_PKT_HEADROOM, datap, len);

        /*set the recyle hook */
        fkb->recycle_hook = bdmf_sysb_recycle;
        fkb->recycle_context = context;

        fkb->rx_csum_verified=!!(flags & BDMF_RX_CSUM_VERIFIED_MASK);

        return (bdmf_sysb)FKBUFF_2_PNBUFF(fkb);
    }
    else if (sysb_type == bdmf_sysb_skb)
    {
        struct sk_buff *skb;
        /* allocate skb structure*/
        skb = skb_header_alloc();
        if(!skb)
        {
            return NULL;
        }

        /* initialize the skb */

        skb_headerinit(BCM_PKT_HEADROOM + data_offset,
#if defined(CC_NBUFF_FLUSH_OPTIMIZATION)
            SKB_DATA_ALIGN(len + BCM_SKB_TAILROOM + data_offset),
#else
            BCM_MAX_PKT_LEN - data_offset,
#endif
            skb, datap + data_offset, bdmf_sysb_recycle,
            context, NULL);

        skb_trim(skb, len);
        skb->recycle_flags &= SKB_NO_RECYCLE;/* no skb recycle,just do data recyle */
        skb->ip_summed = flags & BDMF_RX_CSUM_VERIFIED_MASK ? CHECKSUM_UNNECESSARY : CHECKSUM_NONE;

        return (bdmf_sysb)SKBUFF_2_PNBUFF(skb);
    }
    printk("%s: sysbtype=%d not supported\n", __FUNCTION__, sysb_type);
    return NULL;
}


/** Mark skb orphan partial
 * \param[in]   sysb        System buffer
 * \return void
 */
static inline void bdmf_sysb_orphan_partial(const bdmf_sysb sysb)
{
    /* sysb could be NULL for raw packet send */
    if(sysb && IS_SKBUFF_PTR(sysb) && PNBUFF_2_SKBUFF(sysb)->destructor && PNBUFF_2_SKBUFF(sysb)->sk)
    {
        skb_orphan_partial(PNBUFF_2_SKBUFF(sysb));
    }
}

/** Allocate data buffers.
 * \param[out]  bufp        Array to hold allocated data buffers
 * \param[in]   num_buffs   number of buffers to allocate
 *\ param[in]   prio        ring prio
 * \param[in]   context     currently unused
 * \returns     number of buffers allocated.
 */
static inline uint32_t bdmf_sysb_databuf_alloc( void **bufp, uint32_t num_buffs, uint32_t prio, unsigned long context)
{
#if (defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE))
    if(gbpm_alloc_mult_buf_ex(num_buffs, (void**)bufp, prio) == GBPM_ERROR)
    {
        /* BPM returns either all the buffers requested or none */
        return 0;
    }

    /* No cache invalidation of buffers is needed for buffers coming from BPM */

    /* BPM would have reserved space for FkBuff_t and BCM_PKT_HEADROOM */

    return num_buffs;
#else
    uint32_t *datap;
       /* allocate from kernel directly */
    datap = kmalloc(BCM_PKTBUF_SIZE, GFP_ATOMIC);

    if(!datap)
    {
        return 0;
    }

    /* reserve space for headroom & FKB */
    bufp[0] = (void *)PFKBUFF_TO_PDATA((void *)(datap), BCM_PKT_HEADROOM);

    /* do a cache invalidate of the DMA-seen data area */
    bdmf_dcache_inv((unsigned long)bufp[0], BCM_MAX_PKT_LEN);

    return 1; /* always return only one buffer when BPM is not enabled */
#endif
}

#ifdef XRDP
extern void (*sysb_recycle_function_cb)(void *datap);
#endif
/** Recycle system buffer.
 * \param[in]   sysb        System buffer
 * \param[in]   context     unused
 */
static inline void __bdmf_sysb_databuf_recycle(void *pFkb)
{
#ifdef XRDP
    BDMF_GBPM_TRACK_BUF( pFkb, GBPM_VAL_RECYCLE, 0 );
    (sysb_recycle_function_cb)(pFkb);
#else
#if (defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE))
    BDMF_GBPM_TRACK_BUF( pFkb, GBPM_VAL_FREE, 0 );
    /* BPM expects data buffer ptr offseted by FkBuff_t, BCM_PKT_HEADROOM */
    gbpm_free_buf((void*)PFKBUFF_TO_PDATA(pFkb, BCM_PKT_HEADROOM));
#else
    /* pFkb points to start of buffer, i.e. pFkBuff */
    kfree(pFkb);
#endif
#endif
}

/** Free the datap poniter actual pointer allocated(before headroom) and
 then recyle
 * \param[in]   sysb        System buffer
 * \param[in]   context     unused
 */
static inline void bdmf_sysb_databuf_free(void *pFkb, unsigned long context)
{
     /*do cache invalidate */
      bdmf_dcache_inv((unsigned long)pFkb, BCM_MAX_PKT_LEN);
    __bdmf_sysb_databuf_recycle((void *)PDATA_TO_PFKBUFF(pFkb, BCM_PKT_HEADROOM));
}

/** Release system buffer.
 * \param[in]   sysb        System buffer
 */
static inline void bdmf_sysb_free(bdmf_sysb sysb)
{
        nbuff_free(sysb);
}

/** Get sysb data pointer
 * \param[in]   sysb        System buffer
 * \return data pointer
 */
static inline void *bdmf_sysb_data(const bdmf_sysb sysb)
{
    if(IS_FKBUFF_PTR(sysb))
    {
        return (PNBUFF_2_FKBUFF(sysb))->data;
    }
    else
    {
        return (PNBUFF_2_SKBUFF(sysb))->data;
    }
}

/** Get sysb head pointer
 * \param[in]   sysb        System buffer
 * \return head pointer
 */
static inline void *bdmf_sysb_head(const bdmf_sysb sysb)
{
    if(IS_FKBUFF_PTR(sysb))
    {
        return (void *)PFKBUFF_TO_PHEAD(PNBUFF_2_FKBUFF(sysb));
    }
    else
    {
        return (PNBUFF_2_SKBUFF(sysb))->head;
    }
}

static inline void *bdmf_sysb_to_pktbuf(const bdmf_sysb sysb)
{
    if(IS_FKBUFF_PTR(sysb))
    {
        return (void *)(PNBUFF_2_FKBUFF(sysb));
    }
    else
    {
        return (void *)PDATA_TO_PFKBUFF((PNBUFF_2_SKBUFF(sysb))->head, 0);
    }
}

/** Invalidate headroom and Flush sysb data
 * \param[in]   sysb        System buffer
 * \param[in]   data        valid data start location
 * \param[in]   len         valid data length
 * \return data pointer
 * sysb structure has pointer to a linear memory containing metadata and scrathpad
 * followd by the packet starting at "data". Before handing the packet to Runner,
 * it is mandatory to flush the dcache lines containing the packet so they will
 * reside in DDR. This is  because Runner reads the packet from DDR. 
 * In some use cases, Runner may write to areas before "data". For example,
 * when adding protocol layers.
 * Although the area before "data" does not contain valid packet data, it is 
 * important to invalidate the dcache lines containing data of this area because
 * if not, this CPU may write back these dirty dcache line upon their evacuation,
 * overwriting Runner changes.
 */
static inline void bdmf_sysb_inv_headroom_data_flush(const bdmf_sysb sysb, 
    void *data, uint32_t len)
{
    /* Override len if dirty pointer is set */
    if(IS_FKBUFF_PTR(sysb) && is_dptr_tag_(PNBUFF_2_FKBUFF(sysb)->dirty_p))
        len = _to_kptr_from_dptr_(PNBUFF_2_FKBUFF(sysb)->dirty_p) - (uint8_t *)data;

    nbuff_flush(sysb, data, len);
    nbuff_invalidate_headroom(sysb, data);
}

/** Flush sysb data
 * \param[in]   sysb        System buffer
 * \return data pointer
 */
static inline void bdmf_sysb_data_flush(const bdmf_sysb sysb, void *data, uint32_t len)
{
    nbuff_flush(sysb, data, len);
}


/** Get sysb data length
 * \param[in]   sysb        System buffer
 * \return data length
 */
static inline uint32_t bdmf_sysb_length(const bdmf_sysb sysb)
{
    if(IS_FKBUFF_PTR(sysb))
    {
        return (PNBUFF_2_FKBUFF(sysb))->len;
    }
    else
    {
        return (PNBUFF_2_SKBUFF(sysb))->len;
    }
}

/** Get FPM buffer number. Only if system buffer was allocated from FPM pool
 * \param[in]   sysb        System buffer
 * \return FPM buffer number
 */
static inline uint32_t bdmf_sysb_fpm_num(const bdmf_sysb sysb)
{
    if(IS_FKBUFF_PTR(sysb))
    {
        return (PNBUFF_2_FKBUFF(sysb))->fpm_num;
    }
    else
    {
        return (PNBUFF_2_SKBUFF(sysb))->fpm_num;
    }
}

/** Set sysb data length
 * \param[in]   sysb        System buffer
 * \param[in]   len         data length
 * \return void
 */
static inline void bdmf_sysb_length_set(const bdmf_sysb sysb, uint32_t len)
{
    if(IS_FKBUFF_PTR(sysb))
    {
        (PNBUFF_2_FKBUFF(sysb))->len = len;
    }
    else
    {
        (PNBUFF_2_SKBUFF(sysb))->len = len;
    }
}

/** Get sysb linear data length
 * \param[in]   sysb        System buffer
 * \return data length
 */
static inline uint32_t bdmf_sysb_data_length(const bdmf_sysb sysb)
{
    if(IS_FKBUFF_PTR(sysb))
    {
        return (PNBUFF_2_FKBUFF(sysb))->len;
    }
    else
    {
        /*length of skb->data only, does not include data in skb->frags or fraglist*/
        return skb_headlen(PNBUFF_2_SKBUFF(sysb));
    }
}

/** Check whether the sysb is shared (users count > 1)
 * \param[in]   sysb        System buffer
 * \return 1 is shared, 0 otherwise
 */
static inline int bdmf_sysb_shared(const bdmf_sysb sysb)
{
    return nbuff_is_shared(sysb);
}

/** Check whether the sysb->data can be recyled by HW
 *  and if yes free the sysb strcuture 
 * \param[in]   sysb        System buffer
 * \return 1 sysb is freed and sysb->data will be freed by HW
 * \flase :0 both sysb & data willb e freed later
 */
static inline int bdmf_sysb_data_hw_recycle_prep(bdmf_sysb sysb)
{
    return nbuff_data_hw_recycle_prep(sysb);
}

/** Check whether the sysb mark field
 * \param[in]   sysb        System buffer
 * \return mark field
 */
static inline unsigned long bdmf_sysb_mark(const bdmf_sysb sysb)
{
    if (IS_FKBUFF_PTR(sysb))
    {
        return (PNBUFF_2_FKBUFF(sysb))->mark;
    }
    else
    {
        return (PNBUFF_2_SKBUFF(sysb))->mark;
    }
}

/** Return sysb mark field 
 * \param[in]   sysb        System buffer
 * \return mark field
 */
static inline unsigned long bdmf_sysb_mark_svcq_restore_hack(const bdmf_sysb sysb)
{
    if (IS_FKBUFF_PTR(sysb))
    {
        /* for fkb the mark field is overloaded with queue and when WLAN drivers
         * queue packets mark is lost, but when service queues are enabled, the
         * svc queue id embeded in mark is needed. As a temporary hack svc_q info
         * is  stored in seperate fields, and it's restored here
         */
        struct fkbuff *fkb = PNBUFF_2_FKBUFF(sysb);
        /*clear svcq & dpi feilds in mark */
        fkb->mark &= ~(SKBMARK_SQ_MARK_M |SKBMARK_DPIQ_MARK_M);
        /*reterive & and set svc en & qid from fkb */
        fkb->mark |= ((fkb->svc_q_en << SKBMARK_SQ_MARK_S) | (fkb->svc_q_id << SKBMARK_DPIQ_MARK_S));
        return fkb->mark;
    }
    else
    {
        return (PNBUFF_2_SKBUFF(sysb))->mark;
    }
}

/** Return service queue bit from mark value
 */
static inline unsigned long bdmf_sysb_sq_mark_get(unsigned long mark)
{
    return SKBMARK_GET_SQ_MARK(mark);
}

/** Return service queue id bits from mark value
 */
static inline unsigned long bdmf_sysb_dpiq_mark_get(unsigned long mark)
{
    return SKBMARK_GET_DPIQ_MARK(mark);
}

static inline int buf_wifi_priority_get(void *buffer)
{
    int wifi_priority;

    if (IS_SKBUFF_PTR(buffer))
        wifi_priority = LINUX_GET_PRIO_MARK(((struct sk_buff *)buffer)->mark);
    else
        wifi_priority = ((struct fkbuff *)((uintptr_t)buffer & (uintptr_t)NBUFF_PTR_MASK))->wl.ucast.dhd.wl_prio;

    return wifi_priority;
}

static inline void *bdmf_ioremap(bdmf_phys_addr_t phys_addr, size_t size)
{
    return ioremap(phys_addr, size);
}

#if defined(CONFIG_BCM_PKTRUNNER_GSO)

#define RUNNER_MAX_GSO_DESC 512

extern void* bdmf_sysb_data_to_gsodesc(const bdmf_sysb sysb, uint32_t *is_gso_pkt_p);
extern int bdmf_gso_desc_pool_create( uint32_t num_desc);
extern void bdmf_gso_desc_pool_destroy(void);


#ifdef _BYTE_ORDER_LITTLE_ENDIAN_
typedef struct {
    union{
        uint32_t word0;
        uint32_t data;
    };
    union{
        uint32_t word1;
        struct{
            uint16_t len;
            uint16_t linear_len; /*TODO change to total_len */
        };
    };

    union{
        uint32_t word2;
        struct{
            uint16_t mss;
            struct {
                uint8_t isAllocated:1;
                uint8_t reserved0:7;
            };
            uint8_t nr_frags;
        };
    };

    union{
        uint32_t word3;
        uint32_t reserved1;
    };

    uint32_t frag_data[RUNNER_MAX_GSO_FRAGS];
    uint16_t frag_len[RUNNER_MAX_GSO_FRAGS];
    uint32_t reserved2;
}runner_gso_desc_t;

#else
typedef struct {
    union{
        uint32_t word0;
        uint32_t data;
    };

    union{
        uint32_t word1;
        struct{
            uint16_t len; /*TODO change to total_len */
            uint16_t linear_len;
        };
    };

    union{
        uint32_t word2;
        struct{
            uint16_t mss;
            struct {
                uint8_t isAllocated:1;
                uint8_t reserved0:7;
            };
            uint8_t nr_frags;
        };
    };

    union{
        uint32_t word3;
        uint32_t reserved1;
    };
    uint32_t frag_data[RUNNER_MAX_GSO_FRAGS];
    uint16_t frag_len[RUNNER_MAX_GSO_FRAGS];
    uint32_t reserved2;
}runner_gso_desc_t;

#endif

/** Checks if a packet needs GSO processing and convert skb to GSO Descriptor
 * \param[in]   sysb  system buffer
 * \param[out]  is_gso_pkt_p indicates to caller if sysb is a GSO packet
 * \returns for Non-GSO: sysb->data GSO: GSO Desciptor or NULL
 */
static inline void *bdmf_sysb_gso_data(const bdmf_sysb sysb, uint32_t *is_gso_pkt_p)
{
    if(IS_FKBUFF_PTR(sysb))
    {
        *is_gso_pkt_p = 0;
        return (PNBUFF_2_FKBUFF(sysb))->data;
    }
    else
    {
        return bdmf_sysb_data_to_gsodesc(sysb, is_gso_pkt_p);
    }
}

#endif


#if defined(CONFIG_RUNNER_CPU_TX_FRAG_GATHER)

// #define CC_CONFIG_BCM_SG_FRAG_GATHER_DEBUG  1


/* The SG_DESC ring size should no be less than cpu_tx_ring size */
/* Note there are two cpu_tx_ring, high/low priority, each are 2048 now */
/* #define RUNNER_MAX_SG_DESC    (RDPA_CPU_TX_RING_HIGH_PRIO_SIZE + RDPA_CPU_TX_RING_LOW_PRIO_SIZE) */
#define RUNNER_MAX_SG_DESC  4096

/* The SG template header buffer size */
#define RUNNER_SG_HDR_SIZE  128

/*
 * RUNNER_MAX_SG_FRAGS should match RDD_SG_DESC_ENTRY_FRAG_DATA/LEN_NUMBER
 * Note Frag_data address is in 64bit, so needs two 32bit words to store in runner sram
 */
#define RUNNER_MAX_SG_FRAGS 4

typedef int (*rnr_cpu_tx_func) (pbuf_t *pbuf, const void *info, unsigned long mark);

#ifdef _BYTE_ORDER_LITTLE_ENDIAN_
typedef struct {
    union{
        uint32_t word0;
        struct{
            uint8_t  reserve:5;
            uint8_t  is_udp:1;
            uint8_t  is_csum_offload:1;
            uint8_t  is_allocated:1;
            uint8_t  nr_frags;
            uint16_t total_len;
        };
    };

    union{
        uint32_t word1;
        struct{
            uint16_t pseudo_hdr_csum;
            uint8_t  csum_sop;
            uint8_t  csum_offset;            
        };
    };
    
    uint64_t frag_data[RUNNER_MAX_SG_FRAGS];
    uint16_t frag_len[RUNNER_MAX_SG_FRAGS];
    /* 16Byte alignment marker */

    uint8_t *gso_template_hdr;  /* Pointer to gso template header buffer */
    uint32_t len;               /* gso template header length */
} runner_sg_desc_t;

#else
typedef struct {
    union{
        uint32_t word0;
        struct{
            uint8_t  is_allocated:1;
            uint8_t  is_csum_offload:1;
            uint8_t  is_udp:1;
            uint8_t  reserve:5;
            uint8_t  nr_frags;
            uint16_t total_len;
        };
    };

    union{
        uint32_t word1;
        struct{
            uint16_t pseudo_hdr_csum;
            uint8_t  csum_sop;
            uint8_t  csum_offset;            
        };
    };
    
    uint64_t frag_data[RUNNER_MAX_SG_FRAGS];
    uint16_t frag_len[RUNNER_MAX_SG_FRAGS];
    /* 16Byte alignment marker */

    uint8_t *gso_template_hdr;  /* Pointer to gso template header buffer */
    uint32_t len;               /* gso template header length */
} runner_sg_desc_t;
#endif

extern void* bdmf_sysb_data_to_sgdesc(const bdmf_sysb sysb, uint32_t *is_sg_pkt_p, rnr_cpu_tx_func xmit_fn, pbuf_t *pbuf, const void *info, uint8_t recycle_bit);
extern int bdmf_sg_desc_pool_create(uint32_t num_desc);
extern void bdmf_sg_desc_pool_destroy(void);
extern runner_sg_desc_t *bdmf_runner_sg_desc_alloc(void);
extern void bdmf_runner_sg_desc_free(runner_sg_desc_t *sg_desc_p);
extern void *bdmf_kmap_skb_frag(const skb_frag_t *frag);
extern void bdmf_kunmap_skb_frag(void *vaddr);
extern int bdmf_runner_check_sg_desc_pool(void);
extern int bdmf_sg_send_desc(runner_sg_desc_t *sg_desc_p, rnr_cpu_tx_func xmit_fn, pbuf_t *pbuf, const void *info);

/** Checks if a packet needs frag data processing and convert skb to SG Descriptor
 * \param[in]   sysb  system buffer
 * \param[out]  is_sg_pkt_p indicates to caller if sysb is a SG packet
 * \returns for Non-SG: sysb->data SG: SG Desciptor or NULL
 */
static inline void *bdmf_sysb_sg_data(const bdmf_sysb sysb, uint32_t *is_sg_pkt_p, rnr_cpu_tx_func xmit_fn, pbuf_t *pbuf, const void *info, uint8_t recycle_bit)
{
    if(IS_FKBUFF_PTR(sysb))
    {
        *is_sg_pkt_p = 0;
        return (PNBUFF_2_FKBUFF(sysb))->data;
    }
    else
    {
        return bdmf_sysb_data_to_sgdesc(sysb, is_sg_pkt_p, xmit_fn, pbuf, info, recycle_bit);
    }
}

#endif

/*we put the common include at last line of this file*/


#endif

