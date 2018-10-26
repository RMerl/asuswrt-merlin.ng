/*
* <:copyright-BRCM:2013-2015:GPL/GPL:standard
* 
*    Copyright (c) 2013-2015 Broadcom 
*    All Rights Reserved
* 
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License, version 2, as published by
* the Free Software Foundation (the "GPL").
* 
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
* 
* 
* A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
* writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
* Boston, MA 02111-1307, USA.
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


#define BDMF_RX_CSUM_VERIFIED_MASK   0x01
#define BDMF_RX_CSUM_VERIFIED_SHIFT  0


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

        skb_headerinit(BCM_PKT_HEADROOM,
#if defined(ENET_CACHE_SMARTFLUSH)
            SKB_DATA_ALIGN(len+BCM_SKB_TAILROOM),
#else
            BCM_MAX_PKT_LEN,
#endif
            skb, datap, bdmf_sysb_recycle,
            context, NULL);

        skb_trim(skb, len);
        skb->recycle_flags &= SKB_NO_RECYCLE;/* no skb recycle,just do data recyle */
        skb->ip_summed = flags & BDMF_RX_CSUM_VERIFIED_MASK ? CHECKSUM_UNNECESSARY : CHECKSUM_NONE;

        return (bdmf_sysb)SKBUFF_2_PNBUFF(skb);
    }
    printk("%s: sysbtype=%d not supported\n", __FUNCTION__, sysb_type);
    return NULL;
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
extern void (*sysb_recycle_to_feed_cb)(void *datap);
#endif
/** Recycle system buffer.
 * \param[in]   sysb        System buffer
 * \param[in]   context     unused
 */
static inline void __bdmf_sysb_databuf_recycle(void *datap, unsigned long context)
{
#ifdef XRDP
    BDMF_GBPM_TRACK_BUF( datap, GBPM_VAL_RECYCLE, 0 );
    (sysb_recycle_to_feed_cb)(datap);
#else
#if (defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE))
    BDMF_GBPM_TRACK_BUF( datap, GBPM_VAL_FREE, 0 );
    /* BPM expects data buffer ptr offseted by FkBuff_t, BCM_PKT_HEADROOM */
    gbpm_free_buf((void*)PFKBUFF_TO_PDATA(datap, BCM_PKT_HEADROOM));
#else
    /* datap points to start of buffer, i.e. pFkBuff */
    kfree(datap);
#endif
#endif
}

/** Free the datap poniter actual pointer allocated(before headroom) and
 then recyle
 * \param[in]   sysb        System buffer
 * \param[in]   context     unused
 */
static inline void bdmf_sysb_databuf_free(void *datap, unsigned long context)
{
     /*do cache invalidate */
      bdmf_dcache_inv((unsigned long)datap, BCM_MAX_PKT_LEN);
    __bdmf_sysb_databuf_recycle((void *)PDATA_TO_PFKBUFF(datap, BCM_PKT_HEADROOM), context);
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

static inline void *bdmf_ioremap(bdmf_phys_addr_t phys_addr, size_t size)
{
    return ioremap(phys_addr, size);
}

#if defined(CONFIG_BCM_PKTRUNNER_GSO)

#define RUNNER_MAX_GSO_DESC 512

/*
*** RUNNER_MAX_GSO_FRAGS should match RDD_GSO_DESC_ENTRY_FRAG_DATA_NUMBER
*/
#define RUNNER_MAX_GSO_FRAGS 18


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

/*we put the common include at last line of this file*/


#endif

