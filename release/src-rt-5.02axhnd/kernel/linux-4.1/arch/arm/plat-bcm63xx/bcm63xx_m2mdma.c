/*
<:copyright-BRCM:2015:DUAL/GPL:standard

   Copyright (c) 2015 Broadcom 
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
#include <linux/types.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/io.h>
#include <linux/module.h>
#include <linux/dma-mapping.h>
#include <linux/delay.h>
#include <linux/bug.h>
#include <linux/nbuff.h>
#include "bcm_map_part.h"


#define M2M_DMA_REG_BASE BCM_IO_ADDR(M2M_DMA_PHYS_BASE)

typedef struct {

    uint32_t ch0_desc_status;
    uint32_t ch1_desc_status;
    uint32_t ch2_desc_status;
    uint32_t ch3_desc_status;

    uint32_t ch0_src_addr;
    uint32_t ch0_dest_addr;
    uint32_t ch0_desc_id;
    uint32_t ch0_dma_config;

    uint32_t ch1_src_addr;
    uint32_t ch1_dest_addr;
    uint32_t ch1_desc_id;
    uint32_t ch1_dma_config;

    uint32_t ch2_src_addr;
    uint32_t ch2_dest_addr;
    uint32_t ch2_desc_id;
    uint32_t ch2_dma_config;

    uint32_t ch3_src_addr;
    uint32_t ch3_dest_addr;
    uint32_t ch3_desc_id;
    uint32_t ch3_dma_config;

    uint32_t int_clear;
    uint32_t control;
    uint32_t dma_status;
    uint32_t ch_stop;
    uint32_t desc_clear;

    uint32_t ch0_ubus_err_debug0;
    uint32_t ch0_ubus_err_debug1;

    uint32_t ch1_ubus_err_debug0;
    uint32_t ch1_ubus_err_debug1;

    uint32_t ch2_ubus_err_debug0;
    uint32_t ch2_ubus_err_debug1;

    uint32_t ch3_ubus_err_debug0;
    uint32_t ch3_ubus_err_debug1;

    uint32_t ch0_stop_src_addr;
    uint32_t ch0_stop_dest_addr;
    uint32_t ch0_stop_addr_msb;

    uint32_t ch1_stop_src_addr;
    uint32_t ch1_stop_dest_addr;
    uint32_t ch1_stop_addr_msb;

    uint32_t ch2_stop_src_addr;
    uint32_t ch2_stop_dest_addr;
    uint32_t ch2_stop_addr_msb;

    uint32_t ch3_stop_src_addr;
    uint32_t ch3_stop_dest_addr;
    uint32_t ch3_stop_addr_msb;

    uint32_t ch0_status_id_fifo;
    uint32_t ch1_status_id_fifo;
    uint32_t ch2_status_id_fifo;
    uint32_t ch3_status_id_fifo;

    uint32_t spare0;
    uint32_t spare1;
    uint32_t spare2;
} bcm_m2m_dma_reg_t;

#define M2M_DMA_REG ((volatile bcm_m2m_dma_reg_t * const) M2M_DMA_REG_BASE)


#define MAX_ASYNC_DMA_CHNLS 4
#define MAX_M2M_CHNL_QUEUE_DEPTH 8

typedef struct {
    volatile uint32_t src_addr;
    volatile uint32_t dest_addr;
    volatile uint32_t desc_id;
    volatile uint32_t dma_config;
}m2m_dma_desc_t;


typedef struct {
    m2m_dma_desc_t *dma_desc;
    volatile uint32_t *desc_status;
    uint32_t enable_mask;
    uint16_t desc_id;
    uint8_t chnl_idx;
    uint8_t avail_desc;
}m2m_dma_chanl_t;

typedef struct {
    m2m_dma_chanl_t async_chnls[MAX_ASYNC_DMA_CHNLS];
    spinlock_t  async_chnl_lock;
    uint8_t cur_async_chnl_idx;
}bcm_m2m_dma_t;

static bcm_m2m_dma_t bcm_m2m_dma;


#define M2M_UBUS_BURST_SIZE_128 0x100000  /*128 bytes*/
#define M2M_DMA_LEN_MASK        0x0FFFFF

/* DMA channels enable mask with 1 oustanding UBUS request */

#define DMA_CHANL0_ENABLE_MASK 0x01
#define DMA_CHANL1_ENABLE_MASK 0x02
#define DMA_CHANL2_ENABLE_MASK 0x004
#define DMA_CHANL3_ENABLE_MASK 0x008

#define M2M_ASYNC_LOCK()    spin_lock(&bcm_m2m_dma.async_chnl_lock)
#define M2M_ASYNC_UNLOCK()  spin_unlock(&bcm_m2m_dma.async_chnl_lock)



/* 
 * Find a DMA channel with a free descriptor slot 
 * caller should acquire lock 
*/
static inline m2m_dma_chanl_t * get_free_dma_channel_async(void)
{
    uint8_t chnl = bcm_m2m_dma.cur_async_chnl_idx;
    m2m_dma_chanl_t *m2m_dma_chnl;
    int i;

    /* here <= is needed to check the starting channel twice before returning NULL */
    for(i=0; i<=MAX_ASYNC_DMA_CHNLS; i++)
    {
        chnl = (chnl+1) % MAX_ASYNC_DMA_CHNLS;
        m2m_dma_chnl = &bcm_m2m_dma.async_chnls[chnl];
        if(m2m_dma_chnl->avail_desc)
        {
            //printk("channel=%d, avail_desc=%d\n",chnl, m2m_dma_chnl->avail_desc); 
            m2m_dma_chnl->avail_desc--;
            bcm_m2m_dma.cur_async_chnl_idx = chnl; 
            return m2m_dma_chnl;
        }

        /*read num of free descriptors from HW and update the avil_desc*/
        m2m_dma_chnl->avail_desc = *m2m_dma_chnl->desc_status & 0xFF;
    }
    return NULL;
}

/* 
 * check if a given transcation and all the transactions before it are completed
 *
 * id: bits 0-15 desc_id
 * id: 16-17 channel num 
 */
static inline int bcm_m2m_is_async_dma_done(uint32_t id)
{
    volatile uint32_t busy;
    int i;
    uint16_t cur_desc_id;
    uint16_t desc_id;
    uint8_t chnl = (id >> 16) & (MAX_ASYNC_DMA_CHNLS-1);

    /* first check if M2M is idle */
    busy = M2M_DMA_REG->dma_status & 0xf;
    if(!busy)
        return 1;


    /* here given an id we need to find out if the corresponding transaction and
     * all the transcations before it in other channels are completed
     * 
     * each channel maintains it's own desc_id, but since transactions are
     * scheduled in round robin fashion among channels,
     * for the channels before a given chnl we expect cur_desc_id >= desc_id 
     * and for the channels after a given chnl we expect cur_desc_id >=desc_id-1
     *
     * any holes will be catched by M2M idle check 
     */

    busy=0;
    for(i=0; i<=MAX_ASYNC_DMA_CHNLS; i++)
    {
        cur_desc_id = *bcm_m2m_dma.async_chnls[i].desc_status >> 16;
        
        desc_id = id & 0xFFFF; /* 16 bits */
        if(i > chnl)
        {
            desc_id--;
        }

        if(cur_desc_id < desc_id)
        {
            busy=1;
            break;
        }
        else if((cur_desc_id + MAX_M2M_CHNL_QUEUE_DEPTH) >= desc_id)
        {
            /*no rollover */
            busy=1;
            break;
        }
    }

    return (!busy);
}

#define MAX_WAIT_LOOP_COUNT 50000


/* bcm_m2m_wait_for_complete:
 *
 * given a transcation this function checks if the corresponding DMA transaction 
 * and all the transactions before it are completed 
 *
 * desc_id - DMA transaction to check
 *
 * returns non-zero value if DMA is complete, zero if DMA is still pending
 */
int bcm_m2m_wait_for_complete(uint32_t desc_id)
{
    int i = MAX_WAIT_LOOP_COUNT +1;

    /*dont wait indefinitely */
    while(--i && !bcm_m2m_is_async_dma_done(desc_id));

    if(i == 0)
    {
        printk(KERN_WARNING"%s: M2M transaction %x has not yet completed\n", __func__, desc_id);
    }

    return i;
}
EXPORT_SYMBOL(bcm_m2m_wait_for_complete);

static inline void queue_m2m_transfer(m2m_dma_desc_t *dma_desc, uint32_t phys_dest,
        uint32_t phys_src, uint32_t desc_id, uint16_t len)
{
    dma_desc->src_addr = phys_src;
    dma_desc->dest_addr = phys_dest;
    dma_desc->desc_id = desc_id;
    dma_desc->dma_config = M2M_UBUS_BURST_SIZE_128 | (len & M2M_DMA_LEN_MASK);
}

/* caller must ensure len is maximum of 16 bits only */
/* caller must ensure src & dest are in contiguos physical memory */
static inline uint32_t __bcm_m2m_dma_memcpy_async(void *dest, void *src, uint16_t len)
{
    m2m_dma_chanl_t *m2m_dma_chnl;
    uint32_t phys_src;
    uint32_t phys_dest;
    uint32_t desc_id;

    phys_src =  virt_to_phys(src);
    phys_dest = virt_to_phys(dest);

    M2M_ASYNC_LOCK();
    
    do{

        m2m_dma_chnl = get_free_dma_channel_async();

        if(m2m_dma_chnl)
        {
            desc_id = m2m_dma_chnl->desc_id++;

            queue_m2m_transfer(m2m_dma_chnl->dma_desc, phys_dest, phys_src,
                    desc_id<<16, len);

            M2M_ASYNC_UNLOCK();
        }
        else
        {
            /* Instead of waiting fallback to memcpy if cache lines are
             * not shared by dest. This check is needed to avoid corruption
             * when both DMA & CPU try to use same cache line 
             */
            if(!(((uint32_t)dest & (L2_CACHE_LINE_SIZE - 1)) || (len % L2_CACHE_LINE_SIZE)))
            {
                /*get a channel pointer -needed just for a desc_id*/
                m2m_dma_chnl = &bcm_m2m_dma.async_chnls[bcm_m2m_dma.cur_async_chnl_idx];
                desc_id = m2m_dma_chnl->desc_id -1;

                M2M_ASYNC_UNLOCK();
        

                memcpy(dest, src, len);
                /*flush dest to make it look like DMA copy to caller */
                dma_map_single(NULL, dest, len, DMA_TO_DEVICE);
            }
        }
    } while(!m2m_dma_chnl);

    return ((m2m_dma_chnl->chnl_idx << 16) | desc_id );
}

/* bcm_m2m_dma_memcpy_async:
 * use this function with cached memory
 * here we flush src & invalidate dest before scheduling the transfer 
 *
 *
 * dest - virtual address of destination
 * src  - virtual address of destination
 * len  - length of data to be copied
 *
 * this function expects src & dest to be in contiguos physcial memory 
 *
 * returns a transaction id for the DMA operation,
 * copy is not complete on return, caller has to explicitly check if
 * transaction is completed
 */ 
uint32_t bcm_m2m_dma_memcpy_async(void *dest, void *src, uint16_t len)
{
    /* TODO do we need to call dma_unmap_single for NULL device */
    dma_map_single(NULL, src,  len, DMA_TO_DEVICE);
    dma_map_single(NULL, dest, len, DMA_FROM_DEVICE);
    
    return __bcm_m2m_dma_memcpy_async(dest, src, len);
}
EXPORT_SYMBOL(bcm_m2m_dma_memcpy_async);


/* bcm_m2m_dma_memcpy_async_no_flush:
 * use this function with cached memory
 * here there is no cache flush of src, use this function
 * when you are sure that src is not dirty in cache
 *
 * dest - virtual address of destination
 * src  - virtual address of destination
 * len  - length of data to be copied
 *
 * this function expects src & dest to be in contiguos physcial memory 
 *
 * returns a transaction id for the DMA operation,
 * copy is not complete on return, caller has to explicitly check if
 * transaction is completed
 */ 
uint32_t bcm_m2m_dma_memcpy_async_no_flush(void *dest, void *src, uint16_t len)
{
    /* TODO do we need to call dma_unmap_single for NULL device */
    dma_map_single(NULL, src,  len, DMA_TO_DEVICE);
    
    return __bcm_m2m_dma_memcpy_async(dest, src, len);
}
EXPORT_SYMBOL(bcm_m2m_dma_memcpy_async_no_flush);

/* bcm_m2m_dma_memcpy_async_no_flush_inv:
 * Here there is no cache flush of src,and also there is no invalidate on dest 
 * use this when you are that src is not dirty in cache & dest is not in cache
 *
 * dest - virtual address of destination
 * src  - virtual address of destination
 * len  - length of data to be copied
 *
 * this function expects src & dest to be in contiguos physcial memory 
 *
 * returns a transaction id for the DMA operation,
 * copy is not complete on return, caller has to explicitly check if
 * transaction is completed
 */ 
uint32_t bcm_m2m_dma_memcpy_async_no_flush_inv(void *dest, void *src, uint16_t len)
{
    return __bcm_m2m_dma_memcpy_async(dest, src, len);
}
EXPORT_SYMBOL(bcm_m2m_dma_memcpy_async_no_flush_inv);


/* bcm_m2m_dma_memcpy_async_uncached:
 * use with uncached memory, caller has to pass physical addresses
 *
 * phys_dest - physical address of destination
 * phys_src  - virtual address of destination
 * len       - length of data to be copied
 *
 *
 * returns a transaction id for the DMA operation,
 * copy is not complete on return, caller has to explicitly check if
 * transaction is completed
 */
uint32_t bcm_m2m_dma_memcpy_async_uncached(uint32_t phys_dest, uint32_t phys_src, uint16_t len)
{
    m2m_dma_chanl_t *m2m_dma_chnl;
    uint32_t desc_id;

    M2M_ASYNC_LOCK();

    do
    {
        m2m_dma_chnl = get_free_dma_channel_async();

        if(m2m_dma_chnl)
        {
            desc_id = m2m_dma_chnl->desc_id++;

            queue_m2m_transfer(m2m_dma_chnl->dma_desc, phys_dest, phys_src,
                    desc_id<<16, len);

            M2M_ASYNC_UNLOCK();
        }

    } while(!m2m_dma_chnl);

    return ((m2m_dma_chnl->chnl_idx << 16) | desc_id );
}
EXPORT_SYMBOL(bcm_m2m_dma_memcpy_async_uncached);
 
static __init int bcm_m2m_dma_init(void)
{
    spin_lock_init(&bcm_m2m_dma.async_chnl_lock);

    bcm_m2m_dma.async_chnls[0].dma_desc = (m2m_dma_desc_t *)&M2M_DMA_REG->ch0_src_addr;
    bcm_m2m_dma.async_chnls[0].desc_status = &M2M_DMA_REG->ch0_desc_status;
    bcm_m2m_dma.async_chnls[0].chnl_idx = 0;
    bcm_m2m_dma.async_chnls[0].desc_id = 0;
    bcm_m2m_dma.async_chnls[0].avail_desc = M2M_DMA_REG->ch0_desc_status & 0xFF;
    bcm_m2m_dma.async_chnls[0].enable_mask = DMA_CHANL0_ENABLE_MASK;

    bcm_m2m_dma.async_chnls[1].dma_desc = (m2m_dma_desc_t *)&M2M_DMA_REG->ch1_src_addr;
    bcm_m2m_dma.async_chnls[1].desc_status = &M2M_DMA_REG->ch1_desc_status;
    bcm_m2m_dma.async_chnls[1].chnl_idx = 1;
    bcm_m2m_dma.async_chnls[1].desc_id = 0;
    bcm_m2m_dma.async_chnls[1].avail_desc = M2M_DMA_REG->ch1_desc_status & 0xFF;
    bcm_m2m_dma.async_chnls[1].enable_mask = DMA_CHANL1_ENABLE_MASK;

    bcm_m2m_dma.async_chnls[2].dma_desc = (m2m_dma_desc_t *)&M2M_DMA_REG->ch2_src_addr;
    bcm_m2m_dma.async_chnls[2].desc_status = &M2M_DMA_REG->ch2_desc_status;
    bcm_m2m_dma.async_chnls[2].chnl_idx = 2;
    bcm_m2m_dma.async_chnls[2].desc_id = 0;
    bcm_m2m_dma.async_chnls[2].avail_desc = M2M_DMA_REG->ch2_desc_status & 0xFF;
    bcm_m2m_dma.async_chnls[2].enable_mask = DMA_CHANL2_ENABLE_MASK;

    bcm_m2m_dma.async_chnls[3].dma_desc = (m2m_dma_desc_t *)&M2M_DMA_REG->ch3_src_addr;
    bcm_m2m_dma.async_chnls[3].desc_status = &M2M_DMA_REG->ch3_desc_status;
    bcm_m2m_dma.async_chnls[3].chnl_idx = 3;
    bcm_m2m_dma.async_chnls[3].desc_id = 0;
    bcm_m2m_dma.async_chnls[3].avail_desc = M2M_DMA_REG->ch3_desc_status & 0xFF;
    bcm_m2m_dma.async_chnls[3].enable_mask = DMA_CHANL3_ENABLE_MASK;

    bcm_m2m_dma.cur_async_chnl_idx=0;

    M2M_DMA_REG->control = DMA_CHANL0_ENABLE_MASK | DMA_CHANL1_ENABLE_MASK
        | DMA_CHANL2_ENABLE_MASK | DMA_CHANL3_ENABLE_MASK;

    printk(KERN_DEBUG "+++ Successfully registered M2M DMA\n");
    return 0;
}

arch_initcall(bcm_m2m_dma_init);
