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


#define BDMF_RX_CSUM_VERIFIED_MASK   0x01
#define BDMF_RX_CSUM_VERIFIED_SHIFT  0

/** Invalidate dcache range
 * \param[in]   addr    start address
 * \param[in]   size    size
 */
static inline void bdmf_dcache_inv(unsigned long addr, unsigned long size)
{
    /* call OS cache invalidate function */
}

/** Flush dcache range
 * \param[in]   addr    start address
 * \param[in]   size    size
 */
static inline void bdmf_dcache_flush(unsigned long addr, unsigned long size)
{
    /* call OS cache flush function */
}

static inline bdmf_sysb_type bdmf_sysb_typeof(bdmf_sysb sysb)
{
    return bdmf_sysb_skb;
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
    return NULL;
}

extern struct sk_buff * skb_header_alloc(void);

/** Allocate system buffer header i.e skb or fkb structure
 *  and initialize it with the provided len & data buffer
 * \param[in]   sysb_type   System buffer type
 * \param[in]   len         Data length
 * \return system buffer pointer.
 * If the function returns NULL, caller is responsible for "data" deallocation
 */
static inline bdmf_sysb 
bdmf_sysb_header_alloc(   bdmf_sysb_type sysb_type, 
                          void*           datap,
                          uint32_t        len, 
                          unsigned long   context, 
                          uint32_t        flags)
{
    /* Not implemented for CM */
    return NULL;
}

/** Allocate data buffers.
 * \param[out]  bufp        Array to hold allocated data buffers
 * \param[in]   num_buffs   number of buffers to allocate
 * \param[in]   context     currently unused
 * \returns     number of buffers allocated.
 */
static inline uint32_t bdmf_sysb_databuf_alloc(uint32_t *bufp, uint32_t num_buffs, uint32_t prio, unsigned long context)
{
    /* Not implemented for CM */
    return 0;
}


/** Recycle system buffer.
 * \param[in]   sysb        System buffer
 * \param[in]   context     unused
 */
static inline void __bdmf_sysb_databuf_recycle(void *datap, unsigned long context)
{
    /* Not implemented for CM */
    return ;
}

/** Free the datap poniter actual pointer allocated(before headroom) and
 then recyle
 * \param[in]   sysb        System buffer
 * \param[in]   context     unused
 */
static inline void bdmf_sysb_databuf_free(void *datap, unsigned long context)
{
    /* Not implemented for CM */
    return ;
}

/** Release system buffer.
 * \param[in]   sysb        System buffer
 */
static inline void bdmf_sysb_free(bdmf_sysb sysb)
{
    /* Not implemented for CM */
    return ;
}

/** Get sysb data pointer
 * \param[in]   sysb        System buffer
 * \return data pointer
 */
static inline void *bdmf_sysb_data(const bdmf_sysb sysb)
{
    /* Not implemented for CM */
    return 0;
}

/** Invalidate headroom and Flush sysb data
 * \param[in]   sysb        System buffer
 * \param[in]   data        valid data start location
 * \param[in]   len         valid data length
 * \return data pointer
 */ 
static inline void bdmf_sysb_inv_headroom_data_flush(const bdmf_sysb sysb, 
    void *data, uint32_t len)
{
    /*not implemented for CM */
    return;  
}

/** Flush sysb data
 * \param[in]   sysb        System buffer
 * \return data pointer
 */
static inline void bdmf_sysb_data_flush(const bdmf_sysb sysb, void *data, uint32_t len)
{
    /* Not implemented for CM */
    return ;
}


/** Get sysb data length
 * \param[in]   sysb        System buffer
 * \return data length
 */
static inline uint32_t bdmf_sysb_length(const bdmf_sysb sysb)
{
    /* Not implemented for CM */
    return 0;
}

/** Set sysb data length
 * \param[in]   sysb        System buffer
 * \param[in]   len         data length
 * \return void
 */
static inline void bdmf_sysb_length_set(const bdmf_sysb sysb, uint32_t len)
{
    /* Not implemented for CM */
    return ;
}

/** Get sysb linear data length
 * \param[in]   sysb        System buffer
 * \return data length
 */
static inline uint32_t bdmf_sysb_data_length(const bdmf_sysb sysb)
{
    /* Not implemented for CM */
    return 0;
}

static inline void *bdmf_ioremap(bdmf_phys_addr_t phys_addr, size_t size)
{
    return ioremap(phys_addr, size);
}

#endif

