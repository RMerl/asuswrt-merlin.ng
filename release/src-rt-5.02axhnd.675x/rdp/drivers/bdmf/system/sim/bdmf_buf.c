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


/*******************************************************************
 * bdmf-buf.c
 *
 * bdmf - user-space sk_buff emulation
 *
 *******************************************************************/

#include "bdmf_dev.h"

/*
 * Pieces of buffer emulation
 */
struct sk_buff_head free_skb_list;
static bdmf_fastlock skb_lock;
static uint32_t skb_alloc_count;
static uint32_t skb_free_count;


struct sk_buff *dev_alloc_skb(uint32_t length)
{
    struct sk_buff *skb;

    if (length > SKB_ALLOC_LEN-sizeof(struct sk_buff))
    {
            printf("%s: attempt to allocate skb longer than %d\n",
                   __FUNCTION__, (uint32_t)(SKB_ALLOC_LEN-sizeof(struct sk_buff)));
            return NULL;
    }
    bdmf_fastlock_lock(&skb_lock);
    skb = __skb_dequeue(&free_skb_list);
    if (!skb)
    {
            bdmf_fastlock_unlock(&skb_lock);
#ifdef XRDP
            skb = bdmf_alloc_rsv(SKB_ALLOC_LEN, NULL);
#else
            skb = bdmf_mem_alloc(NULL, BDMF_MEM_CACHE, SKB_ALLOC_LEN, SKB_ALLOC_ALIGN);
#endif
            if (!skb)
                return NULL;
            bdmf_fastlock_lock(&skb_lock);
    }
    else
            --skb_free_count;
    ++skb_alloc_count;
    bdmf_fastlock_unlock(&skb_lock);

    memset(skb, 0, sizeof(struct sk_buff));
    skb->magic = SKB_MAGIC;
    skb->data = (uint8_t *)skb + sizeof(struct sk_buff) + SKB_RESERVE;
    skb->tail = skb->data;
    skb->len = 0;
    skb->end = (uint8_t *)skb + SKB_ALLOC_LEN;
    return skb;
}


void dev_kfree_skb(struct sk_buff *skb)
{
    bdmf_fastlock_lock(&skb_lock);
    __skb_queue_tail(&free_skb_list, skb);
    ++skb_free_count;
    --skb_alloc_count;
    bdmf_fastlock_unlock(&skb_lock);
}


/**
*    skb_make - fill in skb header given data pointer and length
*    the data pointer should not be farer than 256 bytes from
*    the beginning of skb buffer it is part of
*    @data: buffer pointer. originally skb_data(skb)
*           where skb is allocated by dev_alloc_skb
*           possibly modified later, but not more that 64 bytes in either direction
*/
struct sk_buff *skb_make(uint8_t *data, uint32_t len)
{
    struct sk_buff *skb=(struct sk_buff *)((unsigned long)data & ~(SKB_ALLOC_ALIGN-1));
    assert(skb->magic==SKB_MAGIC);
    assert(data+len<skb->end);
    skb->data = data;
    skb->len = len;
    return skb;
}

/* returns skb pointer data belongs to or NULL if data doesn't belong to skb */
struct sk_buff *data_to_skb(void *data)
{
    struct sk_buff *skb=(struct sk_buff *)((unsigned long)data & ~(SKB_ALLOC_ALIGN-1));
    if (skb->magic != SKB_MAGIC)
        return NULL;
    return skb;
}


/**
*    dev_kfree_skb_data - release skb given the data pointer
*    the data pointer should not be farer than 256 bytes from
*    the beginning of skb buffer it is part of
*    @data: buffer pointer. originally skb_data(skb)
*           where skb is allocated by dev_alloc_skb
*           possibly modified later, but not more that 64 bytes in either direction
*/
void dev_kfree_skb_data(uint8_t *data)
{
    struct sk_buff *skb=skb_make(data, 1);
    dev_kfree_skb(skb);
}


/**
*    skb_stat - get skb pool statistics
*    @alloc_count: number of outstanding skb allocations
*    @free_count: number of buffers on skb free list
*/
void skb_stat(uint32_t *alloc_count, uint32_t *free_count)
{
    *alloc_count = skb_alloc_count;
    *free_count = skb_free_count;
}
