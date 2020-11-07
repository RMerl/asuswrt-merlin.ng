/*
* <:copyright-BRCM:2014:proprietary:standard
* 
*    Copyright (c) 2014 Broadcom 
*    All Rights Reserved
* 
*  This program is the proprietary software of Broadcom and/or its
*  licensors, and may only be used, duplicated, modified or distributed pursuant
*  to the terms and conditions of a separate, written license agreement executed
*  between you and Broadcom (an "Authorized License").  Except as set forth in
*  an Authorized License, Broadcom grants no license (express or implied), right
*  to use, or waiver of any kind with respect to the Software, and Broadcom
*  expressly reserves all rights in and to the Software and all intellectual
*  property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
*  NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
*  BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
* 
*  Except as expressly set forth in the Authorized License,
* 
*  1. This program, including its structure, sequence and organization,
*     constitutes the valuable trade secrets of Broadcom, and you shall use
*     all reasonable efforts to protect the confidentiality thereof, and to
*     use this information only in connection with your use of Broadcom
*     integrated circuit products.
* 
*  2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*     AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*     WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
*     RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
*     ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
*     FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
*     COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
*     TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
*     PERFORMANCE OF THE SOFTWARE.
* 
*  3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
*     ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
*     INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
*     WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
*     IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
*     OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
*     SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
*     SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
*     LIMITED REMEDY.
* :>
*/


#ifndef _CFE_

#include "rdp_drv_dhd.h"
#include "rdd_init.h"

extern RDD_FPM_GLOBAL_CFG_DTS g_fpm_hw_cfg;
extern bdmf_boolean llcsnap_enable[RDPA_MAX_RADIOS];
extern int flow_ring_format[RDPA_MAX_RADIOS];
extern void* dongle_wakeup_register[2];

DEFINE_BDMF_FASTLOCK(wlan_ucast_lock);

typedef struct {  
    uint16_t     free_skb_index;          
    void         *cpu_tx_skb_pointer;
} rdp_drv_dhd_index_fifo_t;

extern rdd_dhd_rx_post_ring_t g_dhd_rx_post_ring_priv[RDPA_MAX_RADIOS];

static uint32_t rdp_drv_dhd_skb_free_indexes_release_ptr = 0;
static uint32_t rdp_drv_dhd_skb_free_indexes_head_ptr = 0;
static uint32_t rdp_drv_dhd_skb_free_indexes_cntr = 0;
static uint32_t rdp_drv_dhd_no_free_skb_counter = 0;
static rdp_drv_dhd_index_fifo_t *rdp_drv_dhd_skb_fifo_table = NULL;
RING_DESCTIPTOR g_dhd_complete_ring_desc[RDPA_MAX_RADIOS] = {};

#define DOT11_LLC_SNAP_HDR_LEN 8
#define SNAP_HDR_LEN 6

/* Copy 14B ethernet header: 32bit aligned source and destination. */
#define edasacopy32(s, d) \
do { \
    ((uint32_t *)(d))[0] = ((const uint32_t *)(s))[0]; \
    ((uint32_t *)(d))[1] = ((const uint32_t *)(s))[1]; \
    ((uint32_t *)(d))[2] = ((const uint32_t *)(s))[2]; \
} while (0)

static const union {
    uint32_t u32;
    uint16_t u16[2];
    uint8_t u8[4];
} _ctl_oui3 = { .u8 = {0x00, 0x00, 0x00, 0x03} };
    
#define ETHER_TYPE_WORD         6
/** LLCSNAP: OUI[2] setting for Bridge Tunnel (Apple ARP and Novell IPX) */
#define ETHER_TYPE_APPLE_ARP    0x80f3 /* Apple Address Resolution Protocol */
#define ETHER_TYPE_NOVELL_IPX   0x8137 /* Novel IPX Protocol */

#define BRIDGE_TUNNEL_OUI2      0xf8 /* OUI[2] value for Bridge Tunnel */

#define IS_BRIDGE_TUNNEL(et) \
    (((et) == ETHER_TYPE_APPLE_ARP) || ((et) == ETHER_TYPE_NOVELL_IPX))
    
#define MIN_PACKET_LENGTH_WITHOUT_CRC  60

#if (defined(CONFIG_BCM_SPDSVC) || defined(CONFIG_BCM_SPDSVC_MODULE))
#include "rdd_spdsvc.h"
int spdsvc_free_index = -1;
#endif


/* Initialize skb index databases and variables */
void rdp_drv_dhd_skb_fifo_tbl_init(void)
{
    uint16_t i;
   
    /* allocate Free Indexes table */  
    rdp_drv_dhd_skb_fifo_table = (rdp_drv_dhd_index_fifo_t *)bdmf_alloc(sizeof(rdp_drv_dhd_index_fifo_t) * RDD_CPU_TX_SKB_LIMIT_DEFAULT);
    
    /* Fill free indexes FIFO */
    for (i = 0; i < RDD_CPU_TX_SKB_LIMIT_DEFAULT; i++)
    {        
        /* put invalid released index for debug */
        rdp_drv_dhd_skb_fifo_table[i].free_skb_index = i;
        rdp_drv_dhd_skb_fifo_table[i].cpu_tx_skb_pointer = NULL;
    }

    /* update rdd free indexes counter */
    rdp_drv_dhd_skb_free_indexes_cntr = RDD_CPU_TX_SKB_LIMIT_DEFAULT;
    rdp_drv_dhd_skb_free_indexes_head_ptr = 0;
    rdp_drv_dhd_skb_free_indexes_release_ptr = 0;
    
    /* counts problems , when no free skb index was available */
    rdp_drv_dhd_no_free_skb_counter = 0;
}

/* Allocate FPM tokens and fills RxPost FlowRing */
int rdp_drv_rx_post_init(uint32_t radio_idx, rdpa_dhd_init_cfg_t *init_cfg, uint32_t num_items)
{
    rdd_dhd_rx_post_ring_t *ring_info = &g_dhd_rx_post_ring_priv[radio_idx];
    uint32_t fpm_buffer_number;
    uintptr_t write_ptr;
    int i, rc; 
#ifndef RDP_SIM                  
    fpm_pool_stat fpm_stat;
    uint16_t xon_thr, xoff_thr;
#endif

    RDD_BTRACE("radio_idx = %d, init_cfg = %p\n", radio_idx, init_cfg);
    
    for (i = 0, write_ptr = (uintptr_t)init_cfg->rx_post_flow_ring_base_addr; i < num_items; i++)
    {

#ifndef RDP_SIM
        rc = ag_drv_fpm_pool_stat_get(&fpm_stat);
        rc = rc ? rc : ag_drv_fpm_pool1_xon_xoff_cfg_get(&xon_thr, &xoff_thr);
        
        if (rc || (fpm_stat.num_of_tokens_available <= xoff_thr))
            return BDMF_ERR_NORES;
#endif
        rc = drv_fpm_alloc_buffer(DHD_DATA_LEN, &fpm_buffer_number);        
        
        if (rc)
            return rc;
        
        /* Fill entry in RXPost ring with new fpm token , ignore pool_number in fpm_buffer_number*/              
        rdd_rx_post_descr_init(radio_idx, (uint8_t *)write_ptr, (fpm_buffer_number & 0xFFFF));
        
        /* Suport for various RxPost Work Item Formats */
        switch (flow_ring_format[radio_idx])
        {           
            case FR_FORMAT_WI_WI64: /* Legacy Work Item */
            {
                bdmf_dcache_flush((unsigned long)write_ptr, sizeof(RDD_DHD_RX_POST_DESCRIPTOR_DTS));
        
                write_ptr += sizeof(RDD_DHD_RX_POST_DESCRIPTOR_DTS);
                
                break;
            }

            case FR_FORMAT_WI_CWI32: /* Compact Work Item with 32b haddr */
            {               
                bdmf_dcache_flush((unsigned long)write_ptr, sizeof(RDD_DHD_RX_POST_DESCRIPTOR_CWI32_DTS));
        
                write_ptr += sizeof(RDD_DHD_RX_POST_DESCRIPTOR_CWI32_DTS);
                
                break;
            }
            
            default :
            {
                bdmf_print("ERROR: flow ring format: %d not supported\n", flow_ring_format[radio_idx]);
                
                BUG();                
            }
        } /* switch item_type */
                
    }
    
    /* Update RX_post WR/RD index both in SRAM and DDR */
    *ring_info->rd_idx_addr = 0;
    *ring_info->wr_idx_addr = num_items;
    
    RDD_DHD_COMPLETE_COMMON_RADIO_ENTRY_RX_POST_WR_IDX_WRITE_G(__swap2bytes(num_items), RDD_DHD_COMPLETE_COMMON_RADIO_DATA_ADDRESS_ARR, radio_idx);
    
    WMB();
    
    /* send a doorbell to HWA to update wr index */
    /* do not send for now - maybe HWA not up yet*/
    if (init_cfg->dongle_wakeup_hwa)
    {
      /* *((uint32_t*) dongle_wakeup_register[1]) = (((*ring_info->wr_idx_addr) << INDEX_VAL_SHIFT) | (DMA_TYPE_HWA_RXPOST << DMA_TYPE_SHIFT));  */
    }
    
    return 0;
}


/* Frees FPM tokens from RxPost FlowRing */
int rdp_drv_dhd_rx_post_uninit(uint32_t radio_idx, rdpa_dhd_init_cfg_t *init_cfg, uint32_t *num_items)
{   
    uint32_t fpm_buffer_number;
    uint8_t *descr_ptr;
    int rc = 0;
    uint16_t start, end;
    rdd_dhd_rx_post_ring_t *ring_info = &g_dhd_rx_post_ring_priv[radio_idx];
    
    RDD_BTRACE("radio_idx = %d, init_cfg = %p\n", radio_idx, init_cfg);
    
    /* the logic behind this is, there should always be
     * (DHD_RX_POST_FLOW_RING_SIZE - 1) buffers in RxPost ring, because Runner
     * allocates 1 back when it receives 1 in RxComplete.  (Wr_idx - 1) should
     * represent the last refilled buffer, and WRAP(wr_idx + 1) should be
     * the oldest refilled buffer in RxPost.  Therefore, we will free by
     * going from wr_idx + 1, toward wr_idx + 2, and on until it wraps
     * around and gets to (wr_idx - 1) */
    end = *ring_info->wr_idx_addr;
    *num_items = 0;
    
    RMB();
    
    start = (end + 1) & (DHD_RX_POST_FLOW_RING_SIZE - 1);

    do {
        descr_ptr = init_cfg->rx_post_flow_ring_base_addr;
        
        /* Suport for various RxPost Work Item Formats */
        switch (flow_ring_format[radio_idx])
        {           
            case FR_FORMAT_WI_WI64: /* Legacy Work Item */
            {
                descr_ptr += (sizeof(RDD_DHD_RX_POST_DESCRIPTOR_DTS) * start);
                RDD_DHD_RX_POST_DESCRIPTOR_REQUEST_ID_READ(fpm_buffer_number, descr_ptr);
                break;
            }

            case FR_FORMAT_WI_CWI32: /* Compact Work Item with 32b haddr */
            {               
                descr_ptr += (sizeof(RDD_DHD_RX_POST_DESCRIPTOR_CWI32_DTS) * start);
                RDD_DHD_RX_POST_DESCRIPTOR_CWI32_REQUEST_ID_READ(fpm_buffer_number, descr_ptr);
                break;
            }
            
            default :
            {
                bdmf_print("ERROR: flow ring format: %d not supported\n", flow_ring_format[radio_idx]);
                
                BUG();                
            }
        } /* switch item_type */
        

        fpm_buffer_number &= DHD_RX_POST_VALID_REQ_ID_MASK;
        rc = drv_fpm_free_buffer(DHD_DATA_LEN, fpm_buffer_number);
        
        if (rc)
            bdmf_trace("Error releasing FPM num %d, rc = %d\n", fpm_buffer_number, rc);

        (*num_items)++;
        
        start++;
        if (unlikely(start == DHD_RX_POST_FLOW_RING_SIZE))
            start = 0;

    } while (start != end);
    
    return 0;
}

/* Refills FPM tokens from RxPost FlowRing */
int rdp_drv_dhd_rx_post_reinit(uint32_t radio_idx, rdpa_dhd_init_cfg_t *init_cfg)
{ 
  uint32_t num_items;
  int rc = 0;
  
  /* First phase - just empty and refill the ring */
  rc = rdp_drv_dhd_rx_post_uninit(radio_idx, init_cfg, &num_items);
  
  rc = rc ? rc: rdp_drv_rx_post_init(radio_idx, init_cfg, num_items);
  
  return 0;
}

/* Sends message to runner via QM */
static int rdp_drv_dhd_cpu_tx_send_message(uint32_t message_type, uint32_t radio_idx, uint32_t read_idx_flow_ring_idx)
{
    int rc = 0;
    uint8_t  cpu_msg_done = 0;
    int32_t  timeout = 100000;
    rdpa_dhd_ffd_data_t  params = (rdpa_dhd_ffd_data_t)read_idx_flow_ring_idx;
    
    RDD_DHD_CPU_QM_DESCRIPTOR_DTS cpu_tx_descriptor = {};
    
    RDD_BTRACE("message_type=%d, radio_idx=%d, flow_ring_idx=%d, read_idx=%d, read_idx_valid=%d\n", message_type, radio_idx, params.flowring_idx, params.read_idx, params.read_idx_valid);
    
    memset(&cpu_tx_descriptor, 0, sizeof(cpu_tx_descriptor));
    
    cpu_tx_descriptor.first_level_q = QM_QUEUE_DHD_CPU_TX_POST_0 +  radio_idx*2;
    cpu_tx_descriptor.cpu_msg = 1;
    cpu_tx_descriptor.cpu_pd =  1;  
    cpu_tx_descriptor.cpu_msg_type = message_type;
    cpu_tx_descriptor.flow_ring_id = params.flowring_idx;
    cpu_tx_descriptor.valid = 1;
    cpu_tx_descriptor.abs = 1;
    cpu_tx_descriptor.pkt_id_or_read_idx = params.read_idx | (params.read_idx_valid << 13);
    
    /* reset msg done responce before sending message to runner */
    RDD_DHD_POST_COMMON_RADIO_ENTRY_CPU_MSG_DONE_WRITE_G(0, RDD_DHD_POST_COMMON_RADIO_DATA_ADDRESS_ARR, radio_idx);
    
    rc = drv_qm_cpu_tx((uint32_t *)&cpu_tx_descriptor, QM_QUEUE_DHD_CPU_TX_POST_0 +  radio_idx*2, 0, 0);
    
    if (rc)
      return rc;
    
    do
    {
        RDD_DHD_POST_COMMON_RADIO_ENTRY_CPU_MSG_DONE_READ_G(cpu_msg_done, RDD_DHD_POST_COMMON_RADIO_DATA_ADDRESS_ARR, radio_idx);
    } while ((cpu_msg_done == 0) && (timeout-- > 0));
    
    if (timeout <= 0) 
    {
      bdmf_print("ERROR: rdp_drv_dhd_cpu_tx_send_message failed: message_type=%d, radio_idx=%d, flow_ring_idx=%d\n", message_type, radio_idx, params.flowring_idx);
    }
    
    return 0;
}

/* sending "flush" rings message */
int rdp_drv_dhd_helper_flow_ring_flush(uint32_t radio_idx, uint32_t read_idx_flow_ring_id)
{
    rdpa_dhd_ffd_data_t  params = (rdpa_dhd_ffd_data_t)read_idx_flow_ring_id;
    
    RDD_BTRACE("radio_idx=%d, flow_ring_idx=%d, read_idx=%d, read_idx_valid=%d\n", radio_idx, params.flowring_idx, params.read_idx, params.read_idx_valid);
     
    return rdp_drv_dhd_cpu_tx_send_message(DHD_MSG_TYPE_FLOW_RING_FLUSH, radio_idx, read_idx_flow_ring_id);
}

/* sending "ring disable" rings message */
int rdp_drv_dhd_helper_flow_ring_disable(uint32_t radio_idx, uint32_t flow_ring_idx)
{
    RDD_BTRACE("radio_idx=%d, flow_ring_idx=%d\n", radio_idx, flow_ring_idx);
    
    return rdp_drv_dhd_cpu_tx_send_message(DHD_MSG_TYPE_FLOW_RING_SET_DISABLED, radio_idx, flow_ring_idx);
}

extern uintptr_t xrdp_virt2phys(const ru_block_rec *ru_block, uint8_t addr_idx);

/* Returns task information to be propagated to the dongle to wakeup runner directly */
void rdp_drv_dhd_helper_wakeup_information_get(rdpa_dhd_wakeup_info_t *wakeup_info)
{
    uint64_t complete_wakeup_register;  
    
    RDD_BTRACE("wakeup_info = %p\n", wakeup_info);
      
    complete_wakeup_register = xrdp_virt2phys(&RU_BLK(RNR_REGS), get_runner_idx(dhd_complete_runner_image)) + RU_REG_OFFSET(RNR_REGS, CFG_CPU_WAKEUP) ;
    
    wakeup_info->tx_complete_wakeup_register = (uint64_t)(complete_wakeup_register); 
    wakeup_info->tx_complete_wakeup_value = DHD_TX_COMPLETE_0_THREAD_NUMBER + wakeup_info->radio_idx; 
  
    wakeup_info->rx_complete_wakeup_register = (uint64_t)(complete_wakeup_register);
    wakeup_info->rx_complete_wakeup_value = DHD_RX_COMPLETE_0_THREAD_NUMBER + wakeup_info->radio_idx; 
}

/* Creates CPU TX complete ring and propagates ring descriptor to runnner */
int rdp_drv_dhd_helper_dhd_complete_ring_create(uint32_t radio_idx, uint32_t ring_size)
{
    RING_DESCTIPTOR  *descriptor = &g_dhd_complete_ring_desc[radio_idx];
    bdmf_phys_addr_t phy_addr;
    
    if (!ring_size)
    {
        bdmf_trace("ERROR: can't create ring with 0 size\n");
        return BDMF_ERR_ALREADY;
    }
    
    bdmf_trace("Creating DHD complete ring for radio: %d with %d entries descriptor=%p\n " , radio_idx, ring_size, descriptor);

    /*set ring parameters*/

    descriptor->ring_id = radio_idx;
    descriptor->num_of_entries = ring_size;
    descriptor->num_of_entries_mask = descriptor->num_of_entries - 1; /* used for fast modulus operation */
    descriptor->size_of_entry   = sizeof(RDD_DHD_COMPLETE_RING_ENTRY_DTS);
    
    /*allocate ring descriptors - must be non-cacheable memory*/
    descriptor->base = (void *)rdp_mm_aligned_alloc(descriptor->size_of_entry * descriptor->num_of_entries, &phy_addr);
    if (descriptor->base == NULL)
    {
        bdmf_trace("failed to allocate memory for ring descriptor\n");
        return BDMF_ERR_ALREADY;
    }
    
    bdmf_trace("Done initializing Ring %d Base=%p num of entries= %d RDD Base=0x%lx descriptor=%p\n",
        descriptor->ring_id, descriptor->base, descriptor->num_of_entries, (unsigned long)phy_addr, descriptor);
        
    /* reset interface counters */
    drv_cntr_counter_clr(CNTR_GROUP_DHD_CTR, DHD_CTR_GROUP_WRITE_PTR_DHD_0+radio_idx);
    drv_cntr_counter_clr(CNTR_GROUP_DHD_CTR, DHD_CTR_GROUP_SPINLOCK_DHD_0+radio_idx);
    descriptor->shadow_read_idx = 0;
    descriptor->shadow_write_idx = 0;
    
    rdd_complete_ring_init(radio_idx, descriptor, phy_addr);    
                  
    return BDMF_ERR_OK;
}

/* frees allocated ring and resets descriptor in SRAM */
int rdp_drv_dhd_helper_dhd_complete_ring_destroy(uint32_t radio_idx, uint32_t ring_size)
{ 
    RING_DESCTIPTOR *descriptor = &g_dhd_complete_ring_desc[radio_idx];
    
    RDD_BTRACE("radio_idx = %d, ring_size = %d\n", radio_idx, ring_size);
    
    /* create an array of ring elements */
    if (!descriptor->base)
        return BDMF_ERR_ALREADY;
        
    
    rdp_mm_aligned_free((void *)descriptor->base, descriptor->size_of_entry * ring_size);
       
    descriptor->shadow_read_idx = 0;
    descriptor->shadow_write_idx = 0;
    descriptor->ring_id = 0;
    descriptor->num_of_entries = 0;
    descriptor->num_of_entries_mask = 0;
    descriptor->size_of_entry   = 0;
        
    rdd_complete_ring_init(radio_idx, descriptor, 0);       
    
    descriptor->base = 0;

    return 0;
}

/* Reads number of dropped packets for given ssid/radio pair */
uint32_t rdp_drv_dhd_helper_ssid_tx_dropped_packets_get(uint32_t radio_idx, uint32_t ssid)
{    
    uint32_t cntr_arr[MAX_NUM_OF_COUNTERS_PER_READ] = {};
    uint32_t cntr_id;
    
    RDD_BTRACE("radio_idx = %d, ssid = %d\n", radio_idx, ssid);
    
    cntr_id = DHD_CNTR_DHD_TX_DROP_0_SSID_0 + radio_idx*16 + ssid;
    drv_cntr_counter_read(CNTR_GROUP_DHD_CNTRS, cntr_id, cntr_arr);
    return cntr_arr[0];
}

/* Prepares PD to send to QM */
static void rdp_drv_dhd_cpu_tx_set_packet_descriptor(const rdpa_dhd_tx_post_info_t *info, void *buffer, void* data, RDD_DHD_CPU_QM_DESCRIPTOR_DTS *cpu_tx_descriptor,
    int16_t free_index, uint16_t pkt_length)
{
    bdmf_phys_addr_t phy_addr;
    uint32_t addr_hi, addr_lo; 
    int wifi_priority = 0;
    
    phy_addr = RDD_VIRT_TO_PHYS(data);    
    GET_ADDR_HIGH_LOW(addr_hi, addr_lo, phy_addr);

#ifndef RDP_SIM
    if (IS_SKBUFF_PTR(buffer))
        wifi_priority = ((((struct sk_buff *)buffer)->mark) >> 16) & 0x7;
    else
        wifi_priority = ((struct fkbuff *)((uintptr_t)buffer & (uintptr_t)NBUFF_PTR_MASK))->wl.ucast.dhd.wl_prio;
#endif

    cpu_tx_descriptor->first_level_q = QM_QUEUE_DHD_CPU_TX_POST_0 + info->radio_idx*2;
    cpu_tx_descriptor->cpu_msg = 0;
    cpu_tx_descriptor->cpu_pd =  1; 
    cpu_tx_descriptor->cpu_msg_type = 0;
    cpu_tx_descriptor->flow_ring_id = info->flow_ring_id;
    cpu_tx_descriptor->valid = 1;
    cpu_tx_descriptor->packet_length = pkt_length;
    cpu_tx_descriptor->pkt_id_or_read_idx = free_index;
    cpu_tx_descriptor->abs_ptr2 = ((addr_hi & 0xff) << (32 - DHD_CPU_QM_DESCRIPTOR_ABS_PTR1_F_WIDTH)) | (addr_lo >> (DHD_CPU_QM_DESCRIPTOR_ABS_PTR1_F_WIDTH));
    cpu_tx_descriptor->wifi_priority = wifi_priority;
    cpu_tx_descriptor->radio_idx = info->radio_idx;
    cpu_tx_descriptor->abs_ptr1 = addr_lo;
    cpu_tx_descriptor->abs = 1;
    cpu_tx_descriptor->target_mem_0 = 0;
    cpu_tx_descriptor->agg_pd = 0;    
#if (defined(CONFIG_BCM_SPDSVC) || defined(CONFIG_BCM_SPDSVC_MODULE))
    cpu_tx_descriptor->spdsvc = info->is_spdsvc_setup_packet;
#endif
}

/* Send packet to dongler callback 
*  assigns skb_index from the pull and sends message to runner
*  with data_ptr , pkt_id via QM
*/
int rdp_drv_dhd_cpu_tx(const rdpa_dhd_tx_post_info_t *info, void *buffer, uint32_t  pkt_length)
{    
    RDD_DHD_CPU_QM_DESCRIPTOR_DTS cpu_tx_descriptor = {};
    
    uint16_t free_index, eth_type;
    uint8_t *source, *dest;
    bdmf_error_t rdd_rc = 0;

    RDD_BTRACE("info = %p (radio_idx = %d, flow_ring_id = %d, ssid_if_idx = %d), buffer = %p, pkt_length = %d, rdp_drv_dhd_skb_free_indexes_cntr = %d\n", info,
        info->radio_idx, info->flow_ring_id, info->ssid_if_idx, buffer, pkt_length, rdp_drv_dhd_skb_free_indexes_cntr);
   
    /* pkt padding */
    if (pkt_length <  MIN_PACKET_LENGTH_WITHOUT_CRC)
    {
        pkt_length = MIN_PACKET_LENGTH_WITHOUT_CRC;
    }
    
    if (llcsnap_enable[info->radio_idx])
    {
        /* LLCSNAP insertion */
        source = (uint8_t *) bdmf_sysb_data(buffer);
        dest = source - DOT11_LLC_SNAP_HDR_LEN;
        
        pkt_length += DOT11_LLC_SNAP_HDR_LEN;
        eth_type = ((uint16_t *)(source))[ETHER_TYPE_WORD];
        
        edasacopy32(source, dest);
        
        /* default oui */
        ((uint32_t *)(dest))[4] = __swap4bytes(_ctl_oui3.u32);
        
        /* ethernet payload length: 2B */
        ((uint16_t *)(dest))[6] = __swap2bytes(pkt_length - RDD_LAYER2_HEADER_MINIMUM_LENGTH);
        
        /* dsap = 0xaa ssap = 0xaa: 2B copy */
        ((uint16_t *)(dest))[7] = (uint16_t)0xAAAA; /* no need for __swap2bytes */
             
        /* Set OUI[2] for Bridge Tunnel */
        if (IS_BRIDGE_TUNNEL(eth_type)) 
        {
            ((uint8_t *)(dest))[19] = BRIDGE_TUNNEL_OUI2;
        }       
    }
    else
    {
        dest = (uint8_t *) bdmf_sysb_data(buffer);
    }

    bdmf_dcache_flush((unsigned long)dest, pkt_length);
    
    bdmf_fastlock_lock(&wlan_ucast_lock);
        
    if  (unlikely(rdp_drv_dhd_skb_free_indexes_cntr == 0)) 
    {
        rdp_drv_dhd_no_free_skb_counter++;
        bdmf_fastlock_unlock(&wlan_ucast_lock);
        
        RDD_BTRACE("index overflow, total_no_free_indx_counter = %d\n", rdp_drv_dhd_no_free_skb_counter);
        return BDMF_ERR_OVERFLOW;
    }
    
    /* get skb pointer list free index */
    free_index = rdp_drv_dhd_skb_fifo_table[rdp_drv_dhd_skb_free_indexes_head_ptr].free_skb_index;

    if (unlikely(free_index >= RDD_CPU_TX_SKB_LIMIT_DEFAULT))
    {
        bdmf_print("ERROR: RDD_cpu_tx  allocated SKB: idx=0x%x, head_ptr=0x%x\n", free_index,
              rdp_drv_dhd_skb_free_indexes_head_ptr);
        bdmf_fastlock_unlock(&wlan_ucast_lock);
        
        BUG();
    }

    rdp_drv_dhd_skb_free_indexes_head_ptr = (rdp_drv_dhd_skb_free_indexes_head_ptr + 1) &
        (RDD_CPU_TX_SKB_LIMIT_DEFAULT - 1);
        
    rdp_drv_dhd_skb_free_indexes_cntr--;

    /* save buffer ptr and data ptr */
    rdp_drv_dhd_skb_fifo_table[free_index].cpu_tx_skb_pointer = (void *)buffer;   
    
    rdp_drv_dhd_cpu_tx_set_packet_descriptor(info, buffer, (void *)dest, &cpu_tx_descriptor, free_index, pkt_length);

#if (defined(CONFIG_BCM_SPDSVC) || defined(CONFIG_BCM_SPDSVC_MODULE))
    if (info->is_spdsvc_setup_packet)
    {
        bdmf_trace("spdsvc_setup packet in DHD, radio idx %d length %d, Q %d dest %p free_index = %d\n", 
               info->radio_idx, bdmf_sysb_length(buffer), cpu_tx_descriptor.first_level_q, dest, free_index );
        spdsvc_free_index = free_index;

        // change PD to use the processing queue for speed service packets
        cpu_tx_descriptor.first_level_q = QM_QUEUE_DHD_TX_POST_0 + info->radio_idx*2;

        rdd_wlan_spdsvc_gen_start (NULL, info, &cpu_tx_descriptor);

        bdmf_fastlock_unlock(&wlan_ucast_lock);
        return 0;
    }
#endif  

    rdd_rc = drv_qm_cpu_tx((uint32_t *)&cpu_tx_descriptor, QM_QUEUE_DHD_CPU_TX_POST_0 +  info->radio_idx*2, 0, 0);
       
    if (rdd_rc)
    {        
        bdmf_sysb_free(buffer);
            
        bdmf_trace("rdd error %d\n", (int)rdd_rc);
        
        rdp_drv_dhd_skb_fifo_table[rdp_drv_dhd_skb_free_indexes_release_ptr].free_skb_index  = free_index; 
        rdp_drv_dhd_skb_free_indexes_release_ptr++;
        rdp_drv_dhd_skb_free_indexes_release_ptr &= (RDD_CPU_TX_SKB_LIMIT_DEFAULT - 1); 
        
        bdmf_fastlock_unlock(&wlan_ucast_lock);
        
        return BDMF_ERR_IO;
    }
    
    bdmf_fastlock_unlock(&wlan_ucast_lock);
    return 0;
}

/* RDPA callback to return information about 
*  next complete from Tx complete message
*  function pulls next complete message from CPU ring,
*  returns index to pool, free skb in case packet was dropped and
*  returns skb pointer with buffer type otherwise
*/
int rdp_drv_dhd_helper_dhd_complete_message_get(rdpa_dhd_complete_data_t *dhd_complete_info)
{
    /* Read entry from DHD tx complete ring and fill dhd_complete_info structure */
    int rc = BDMF_ERR_OK;    
    RING_DESCTIPTOR *descriptor = &g_dhd_complete_ring_desc[dhd_complete_info->radio_idx];        
    RDD_DHD_COMPLETE_RING_ENTRY_DTS *entry_ptr;
    uint32_t request_id_buffer_type, drop_ind = 1;
    uint16_t index_to_free = 0;
    uint8_t buf_type = 0; 
    void *txp = 0;
    uint32_t cntr_arr[MAX_NUM_OF_COUNTERS_PER_READ] = {};

    bdmf_fastlock_lock(&wlan_ucast_lock);

    if (descriptor->shadow_read_idx == descriptor->shadow_write_idx)
    {      
        /* Update write index from counter */
        drv_cntr_counter_read(CNTR_GROUP_DHD_CTR, DHD_CTR_GROUP_WRITE_PTR_DHD_0 + dhd_complete_info->radio_idx, cntr_arr);
        descriptor->shadow_write_idx = cntr_arr[0];
    }

    RDD_BTRACE("read_ptr=%d, write_ptr=%d\n", descriptor->shadow_read_idx,
        descriptor->shadow_write_idx);

    while ((descriptor->shadow_read_idx != descriptor->shadow_write_idx) && (drop_ind))
    {
        entry_ptr = (RDD_DHD_COMPLETE_RING_ENTRY_DTS *)(descriptor->base + descriptor->shadow_read_idx*descriptor->size_of_entry);
        RDD_DHD_COMPLETE_RING_ENTRY_RING_VALUE_READ(request_id_buffer_type, entry_ptr);
        buf_type = RDD_DHD_COMPLETE_RING_ENTRY_BUFFER_TYPE_L_READ(request_id_buffer_type);      
       
        if (buf_type == DHD_TX_POST_HOST_BUFFER_VALUE)
        {
            /* It is a buffer from offloaded ring - release an index and pass the ptr to DHD */
            index_to_free = request_id_buffer_type & (RDD_CPU_TX_SKB_LIMIT_DEFAULT - 1);

#if (defined(CONFIG_BCM_SPDSVC) || defined(CONFIG_BCM_SPDSVC_MODULE))
            if (index_to_free == spdsvc_free_index)
            {
                bdmf_trace("spdsvc packet returns idx = %d\n", index_to_free);
                spdsvc_free_index = -1;
                rdd_wlan_spdsvc_gen_complete();
            }
#endif

            RDD_DHD_COMPLETE_RING_ENTRY_DROP_READ(drop_ind, entry_ptr);
            
            /* Debug feature - pointer validity check */
            if (likely(rdp_drv_dhd_skb_fifo_table[index_to_free].cpu_tx_skb_pointer != NULL))
            {
                /* free index in SW ring*/
                rdp_drv_dhd_skb_fifo_table[rdp_drv_dhd_skb_free_indexes_release_ptr].free_skb_index  = index_to_free;
            
                /* set ptr to SKB buffer ptr + indication of HOST_BUFFER value */
                txp = (void *)(rdp_drv_dhd_skb_fifo_table[index_to_free].cpu_tx_skb_pointer);
                rdp_drv_dhd_skb_fifo_table[index_to_free].cpu_tx_skb_pointer = NULL;
                
                /* increment counters */
                rdp_drv_dhd_skb_free_indexes_cntr++;
                rdp_drv_dhd_skb_free_indexes_release_ptr++;
                rdp_drv_dhd_skb_free_indexes_release_ptr &= (RDD_CPU_TX_SKB_LIMIT_DEFAULT - 1);
            }
            else
            {
                txp = 0;
                bdmf_trace("ERROR: rdd dhd helper: release of not allocated SKB: idx=%d, ptr=%d\n", index_to_free,
                    rdp_drv_dhd_skb_free_indexes_release_ptr);
                rc = BDMF_ERR_ALREADY;                
            }
            
            if ((drop_ind) && (txp))
            {
               //RDD_BTRACE("Packet dropped, ptr=%p\n", txp);
               bdmf_sysb_free((bdmf_sysb)txp);
            }            
        }     
        else
        {
            txp = 0;
            drop_ind = 0;
        }
       
        descriptor->shadow_read_idx++;
        /* num_of_entries should be power of 2 */
        descriptor->shadow_read_idx &= descriptor->num_of_entries_mask;
    }           
     
    if (drop_ind)
    {
        rc = BDMF_ERR_ALREADY;
    }
    else
    {
        /* Set the return parameters. */
        dhd_complete_info->request_id = request_id_buffer_type;
        dhd_complete_info->buf_type = buf_type;
        dhd_complete_info->txp = txp;
        RDD_DHD_COMPLETE_RING_ENTRY_STATUS_READ(dhd_complete_info->status, entry_ptr);
        RDD_DHD_COMPLETE_RING_ENTRY_FLOW_RING_ID_READ(dhd_complete_info->flow_ring_id, entry_ptr);                             
    }

    /* Update read index for FW */
    RDD_CPU_RING_DESCRIPTOR_READ_IDX_WRITE_G(descriptor->shadow_read_idx, RDD_DHD_CPU_RECYCLE_RING_DESCRIPTOR_TABLE_ADDRESS_ARR,
        dhd_complete_info->radio_idx);
    
    bdmf_fastlock_unlock(&wlan_ucast_lock);
    
    return rc;
}

#endif
