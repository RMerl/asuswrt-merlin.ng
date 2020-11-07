/*
* <:copyright-BRCM:2013-2016:proprietary:standard
* 
*    Copyright (c) 2013-2016 Broadcom 
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
 :>
*/

#ifdef RDP_SIM

#include "bdmf_dev.h"
#include "rdpa_api.h"
#include "rdpa_int.h"
#include "rdd.h"
#include "rdd_common.h"
#include "rdpa_rdd_inline.h"
#include "rdpa_rdd_map.h"
#include "dhd_defs.h"

#define MAX_TX_POST_FLOW_RINGS                 8 /* per radio */
#define DHD_SIM_TX_POST_FLOW_RING_SIZE         16
#define DHD_SIM_TX_POST_PHY_RING_SIZE          2
#define DHD_SIM_CPU_TX_COMPLETE_FLOW_RING_SIZE 16
#define DHD_SIM_DONGLE_PROC_DELAY              2 /* simulating delay(in packets) in dongle processing */

/* Taken from bcmpcie.h */
#define BCMPCIE_H2D_COMMON_MSGRINGS            2
#define BCMPCIE_COMMON_MSGRINGS                5


#define DHD_TX_COMPLETE_RING            0
#define DHD_RX_COMPLETE_RING            1
#define DHD_RX_POST_RING                1

static int next_avail_cpu_queue = 4; /* For simulation, consider that queues #4,5,6 will be used for dhd offload.
                                        Alternatively, can create own cpu object. TBD. */
bdmf_session_handle dhd_sim_session = NULL; 
bdmf_session_handle g_wlan_rx_file_session = NULL;
FILE *g_wlan_rx_file = NULL;
uint8_t  dongle_proc_delay_cnt[RDPA_MAX_RADIOS]; 

extern uintptr_t xrdp_virt2phys(const ru_block_rec *ru_block, uint8_t addr_idx);
extern int flow_ring_format[RDPA_MAX_RADIOS];

typedef struct {
    bdmf_index index; /* Comes for radio_idx */
    bdmf_object_handle dhd_helper_obj;
    bdmf_object_handle cpu_obj;
    int cpu_queue_id;
    rdpa_dhd_init_cfg_t dhd_init_cfg;
    rdpa_dhd_wakeup_info_t wakeup_from_pci_info;
    void *tx_post_flow_ring_arr[MAX_TX_POST_FLOW_RINGS];
    bdmf_boolean enabled;
} dhd_sim_drv_priv_t;

struct bdmf_object *dhd_sim_object[RDPA_MAX_RADIOS] = {};

static int dhd_sim_pre_init(struct bdmf_object *mo)
{
    dhd_sim_drv_priv_t *dhd_sim_data = (dhd_sim_drv_priv_t *)bdmf_obj_data(mo);

    dhd_sim_data->index = BDMF_INDEX_UNASSIGNED;
    return 0;
}

static void dhd_sim_tx_post_descr_dump(bdmf_session_handle session, int radio_idx, int flow_ring_id,
    void *tx_post_descr)
{
    uint8_t  if_id;
    uint64_t data_buf_addr;
    uint32_t tx_eth_hdr_0, tx_eth_hdr_1, tx_eth_hdr_2; 
    uint16_t data_len, tx_eth_hdr_3; 
    uint32_t *pkt_ptr;
    uint32_t data_buf_addr_hi, data_buf_addr_low, request_id;    
    
    switch (flow_ring_format[radio_idx])
    {           
        case FR_FORMAT_WI_WI64: /* Legacy Work Item */
        {
            request_id = ((RDD_DHD_TX_POST_DESCRIPTOR_DTS *)tx_post_descr)->request_id;
            data_buf_addr_low = ((RDD_DHD_TX_POST_DESCRIPTOR_DTS *)tx_post_descr)->data_buf_addr_low;
            data_buf_addr_hi = ((RDD_DHD_TX_POST_DESCRIPTOR_DTS *)tx_post_descr)->data_buf_addr_hi;
            RDD_DHD_TX_POST_DESCRIPTOR_IF_ID_READ(if_id, tx_post_descr);
            RDD_DHD_TX_POST_DESCRIPTOR_DATA_LEN_READ(data_len, tx_post_descr);
            RDD_DHD_TX_POST_DESCRIPTOR_TX_ETH_HDR_0_READ(tx_eth_hdr_0, tx_post_descr);
            RDD_DHD_TX_POST_DESCRIPTOR_TX_ETH_HDR_1_READ(tx_eth_hdr_1, tx_post_descr);
            RDD_DHD_TX_POST_DESCRIPTOR_TX_ETH_HDR_2_READ(tx_eth_hdr_2, tx_post_descr);
            RDD_DHD_TX_POST_DESCRIPTOR_TX_ETH_HDR_3_READ(tx_eth_hdr_3, tx_post_descr);
            break;
        }

        case FR_FORMAT_WI_CWI32: /* Compact Work Item with 32b haddr */
        {               
            request_id = ((RDD_DHD_TX_POST_DESCRIPTOR_CWI32_DTS *)tx_post_descr)->request_id;
            data_buf_addr_low = ((RDD_DHD_TX_POST_DESCRIPTOR_CWI32_DTS *)tx_post_descr)->data_buf_addr_low;
            data_buf_addr_hi = 0;
            RDD_DHD_TX_POST_DESCRIPTOR_CWI32_IF_ID_READ(if_id, (RDD_DHD_TX_POST_DESCRIPTOR_CWI32_DTS *)tx_post_descr);
            RDD_DHD_TX_POST_DESCRIPTOR_CWI32_DATA_LEN_READ(data_len, (RDD_DHD_TX_POST_DESCRIPTOR_CWI32_DTS *)tx_post_descr);
            RDD_DHD_TX_POST_DESCRIPTOR_CWI32_TX_ETH_HDR_0_READ(tx_eth_hdr_0, (RDD_DHD_TX_POST_DESCRIPTOR_CWI32_DTS *)tx_post_descr);
            RDD_DHD_TX_POST_DESCRIPTOR_CWI32_TX_ETH_HDR_1_READ(tx_eth_hdr_1, (RDD_DHD_TX_POST_DESCRIPTOR_CWI32_DTS *)tx_post_descr);
            RDD_DHD_TX_POST_DESCRIPTOR_CWI32_TX_ETH_HDR_2_READ(tx_eth_hdr_2, (RDD_DHD_TX_POST_DESCRIPTOR_CWI32_DTS *)tx_post_descr);
            RDD_DHD_TX_POST_DESCRIPTOR_CWI32_TX_ETH_HDR_3_READ(tx_eth_hdr_3, (RDD_DHD_TX_POST_DESCRIPTOR_CWI32_DTS *)tx_post_descr);
 
            break;
        }
        
        default:
        {
            bdmf_print("ERROR: flow ring format: %d not supported\n", flow_ring_format[radio_idx]);
            
            BUG();                
        }
    } /* switch item_type */
        
    bdmf_session_print(session, "Tx packet arrived to dongle: Radio-%d Flow_Ring-%d SSID-%d Request_ID-%x Data_Buf_Low-%x, Data_Buf_Hi-%x, Len-%d ",
    radio_idx, flow_ring_id, if_id, __swap4bytes(request_id),
    data_buf_addr_low, data_buf_addr_hi, __swap2bytes(data_len));
    bdmf_session_print(session, "L2_header-%x%x%x%x\n", tx_eth_hdr_0, tx_eth_hdr_1, tx_eth_hdr_2, tx_eth_hdr_3);


    data_buf_addr = (uint64_t)((data_buf_addr_hi)) << 32 | (data_buf_addr_low);
    
    
    /* should be enough headroom in packet for L2 header */
    pkt_ptr = (uint32_t *)(((uint8_t *)RDD_PHYS_TO_VIRT(data_buf_addr)) - 14);
    
    pkt_ptr[0] = __swap4bytes(tx_eth_hdr_0);
    pkt_ptr[1] = __swap4bytes(tx_eth_hdr_1);
    pkt_ptr[2] = __swap4bytes(tx_eth_hdr_2);
    *(((uint16_t *)pkt_ptr) + 6) = __swap2bytes(tx_eth_hdr_3);
    
    bdmf_session_hexdump(session, (void *)pkt_ptr, 0, __swap2bytes(data_len) + 14);
}

static void rdd_tx_complete_descr_init(RDD_DHD_TX_COMPLETE_DESCRIPTOR_DTS *descr)
{
    RDD_DHD_TX_COMPLETE_DESCRIPTOR_MSG_TYPE_WRITE(0x12, descr);
    RDD_DHD_TX_COMPLETE_DESCRIPTOR_COMMON_HDR_FLAGS_WRITE(0, descr);
    RDD_DHD_TX_COMPLETE_DESCRIPTOR_EPOCH_WRITE(0, descr);
    RDD_DHD_TX_COMPLETE_DESCRIPTOR_STATUS_WRITE(__swap2bytes(0x103), descr);
    RDD_DHD_TX_COMPLETE_DESCRIPTOR_DMA_DONE_MARK_WRITE(1, descr);
}

/* From TX Post descriptor, get buffer type and data pointer, prepare TX descriptor and push it to TX Complete ring. */
static int dhd_sim_prepare_and_send_tx_complete_to_runner(dhd_sim_drv_priv_t *dhd_sim_data,
    void *tx_post_descr, uint32_t flow_ring_id)
{
    uint16_t *rd_ptr, *wr_ptr;
    uint16_t wr_idx, rd_idx;
    void *tx_complete_descr;
    
    rd_ptr = (uint16_t *)dhd_sim_data->dhd_init_cfg.d2r_rd_arr_base_addr + DHD_TX_COMPLETE_RING;
    wr_ptr = (uint16_t *)dhd_sim_data->dhd_init_cfg.d2r_wr_arr_base_addr + DHD_TX_COMPLETE_RING;

    wr_idx = *wr_ptr;
    rd_idx = *rd_ptr;
    
    if (((wr_idx + 1) & ~DHD_TX_COMPLETE_FLOW_RING_SIZE) == rd_idx)
    {
        BDMF_TRACE_RET(BDMF_ERR_NOMEM, "TX Complete ring is full, cannot continue reading from TX Post, "
            "rd_idx = %d, wr_idx = %d\n", rd_idx, wr_idx);
    }
    
    /* Prepare tx complete descriptor */            
    switch (flow_ring_format[dhd_sim_data->index])
    {                           
        case FR_FORMAT_WI_WI64: /* Legacy Work Item */
        {           
            tx_complete_descr = (RDD_DHD_TX_COMPLETE_DESCRIPTOR_DTS *)dhd_sim_data->dhd_init_cfg.tx_complete_flow_ring_base_addr + wr_idx;
            rdd_tx_complete_descr_init((RDD_DHD_TX_COMPLETE_DESCRIPTOR_DTS *)tx_complete_descr);
            RDD_DHD_TX_COMPLETE_DESCRIPTOR_IF_ID_WRITE(((RDD_DHD_TX_COMPLETE_DESCRIPTOR_DTS *)tx_post_descr)->if_id, (RDD_DHD_TX_COMPLETE_DESCRIPTOR_DTS *)tx_complete_descr);
            RDD_DHD_TX_COMPLETE_DESCRIPTOR_REQUEST_ID_WRITE(__swap4bytes(((RDD_DHD_TX_COMPLETE_DESCRIPTOR_DTS *)tx_post_descr)->request_id), 
                (RDD_DHD_TX_COMPLETE_DESCRIPTOR_DTS *)tx_complete_descr);
            RDD_DHD_TX_COMPLETE_DESCRIPTOR_FLOW_RING_ID_WRITE(__swap2bytes(flow_ring_id), (RDD_DHD_TX_COMPLETE_DESCRIPTOR_DTS *)tx_complete_descr);
            break;
        }

        case FR_FORMAT_WI_CWI32: /* Compact Work Item with 32b haddr */
        {               
            tx_complete_descr = (RDD_DHD_TX_COMPLETE_DESCRIPTOR_CWI32_DTS *)dhd_sim_data->dhd_init_cfg.tx_complete_flow_ring_base_addr + wr_idx;
            RDD_DHD_TX_COMPLETE_DESCRIPTOR_CWI32_REQUEST_ID_WRITE(__swap4bytes(((RDD_DHD_TX_POST_DESCRIPTOR_CWI32_DTS *)tx_post_descr)->request_id), 
                (RDD_DHD_TX_COMPLETE_DESCRIPTOR_CWI32_DTS *)tx_complete_descr);
            
            /* For now new complete format not supported
            RDD_DHD_TX_COMPLETE_DESCRIPTOR_CWI32_FLOW_RING_ID_WRITE(__swap2bytes(flow_ring_id), (RDD_DHD_TX_COMPLETE_DESCRIPTOR_DTS *)tx_complete_descr);
            RDD_DHD_TX_COMPLETE_DESCRIPTOR_CWI32_IF_ID_WRITE(((RDD_DHD_TX_COMPLETE_DESCRIPTOR_DTS *)tx_post_descr)->if_id, (RDD_DHD_TX_COMPLETE_DESCRIPTOR_DTS *)tx_complete_descr);
            */
            break;
        }
        
        default:
        {
            bdmf_print("ERROR: flow ring format: %d not supported\n", flow_ring_format[dhd_sim_data->index]);
            BUG();                
        }
    } /* switch item_type */
    
    
    wr_idx = (wr_idx + 1) & ~DHD_TX_COMPLETE_FLOW_RING_SIZE; /* Wrap around */
    *wr_ptr = wr_idx;
    
    /* Wakeup runner. Use wakeup from host as cannot simulate wakeup from PCI */
    rdpa_dhd_helper_complete_wakeup(dhd_sim_data->index, 1);
    return 0;
}

/* Since we don't implement interrupt handling inside the dongle, we will use the same mechanism for interrupts as we
 * use for CPU RX interface. Hence, we use the same registration format. This is good enough as raising interrupt
 * mechanism is same. */

static int dhd_sim_dongle_tx_post_isr(int irq, void *isr_priv)
{
    dhd_sim_drv_priv_t *dhd_sim_data = (dhd_sim_drv_priv_t *)isr_priv;
    void *tx_post_descr, *tx_post_ring_base;
    int fl_ring_id, i, rc;
    uint8_t  entry_size = 0;
    uint16_t *rd_ptr, *wr_ptr;
    uint16_t rd_idx, wr_idx;
    uint32_t addr_lo, addr_hi;
    
    /* simulating delay by number of interrupts incoming to the dongle */
    /* supposed to be a burst of FR=x packets, when some of them will be placed in BQ */
    /* after that packet from FR=y will cause the dongle to handle all packets from both x and y */
    if ((!dhd_sim_data->enabled) || (dongle_proc_delay_cnt[dhd_sim_data->index]))
    {
        dongle_proc_delay_cnt[dhd_sim_data->index]--;
        
        /* Radio is disabled or delayed, don't react on new packets in any TX Post ring */
        return BDMF_IRQ_HANDLED;
    }

    /* Poll on all flow rings (compare RD and WR pointers). If different, read and dump the packet, then invoke TX
     * Complete for it. */
    rd_ptr = ((uint16_t *)dhd_sim_data->dhd_init_cfg.r2d_rd_arr_base_addr) + BCMPCIE_H2D_COMMON_MSGRINGS;
    wr_ptr = ((uint16_t *)dhd_sim_data->dhd_init_cfg.r2d_wr_arr_base_addr) + BCMPCIE_H2D_COMMON_MSGRINGS;
  

    for (fl_ring_id = BCMPCIE_H2D_COMMON_MSGRINGS; fl_ring_id < MAX_TX_POST_FLOW_RINGS; fl_ring_id++, rd_ptr++, wr_ptr++)
    {
        GET_ADDR_HIGH_LOW(addr_hi, addr_lo, RDD_VIRT_TO_PHYS((void *)wr_ptr));
        bdmf_session_print(dhd_sim_session, "fl_ring_id = %d, Tx Post wr_addr = %x, rd_ptr = %d:  wr_ptr = %d\n", fl_ring_id, addr_lo, *rd_ptr, *wr_ptr);
        if (*rd_ptr == *wr_ptr) /* Nothing posted in this ring*/
            continue;

        rd_idx = (*rd_ptr);
        wr_idx = (*wr_ptr);
        
        tx_post_ring_base = (void *)dhd_sim_data->tx_post_flow_ring_arr[fl_ring_id];
        
        switch (flow_ring_format[dhd_sim_data->index])
        {           
            case FR_FORMAT_WI_WI64: /* Legacy Work Item */
            {
                entry_size = sizeof(RDD_DHD_TX_POST_DESCRIPTOR_DTS);
                break;
            }
    
            case FR_FORMAT_WI_CWI32: /* Compact Work Item with 32b haddr */
            {               
                entry_size = sizeof(RDD_DHD_TX_POST_DESCRIPTOR_CWI32_DTS);
                break;
            }
            
            default:
            {
                bdmf_print("ERROR: flow ring format: %d not supported\n", flow_ring_format[dhd_sim_data->index]);                
                BUG();                
            }
        } /* switch item_type */
            
        
        for (i = rd_idx; i != wr_idx;)
        {            
            tx_post_descr = (void *)((uint64_t)tx_post_ring_base + i*entry_size);
            GET_ADDR_HIGH_LOW(addr_hi, addr_lo, RDD_VIRT_TO_PHYS(tx_post_descr));            
            bdmf_session_print(dhd_sim_session, "Tx Post FR cur ptr = %x : %x:\n", addr_hi, addr_lo);

            dhd_sim_tx_post_descr_dump(g_wlan_rx_file_session, dhd_sim_data->index, fl_ring_id, tx_post_descr);
            rc = dhd_sim_prepare_and_send_tx_complete_to_runner(dhd_sim_data, tx_post_descr, fl_ring_id);
            if (rc)
                return rc;

            i = (i + 1) & ~DHD_SIM_TX_POST_PHY_RING_SIZE; /* Wrap around */
            *rd_ptr = i;
        }
    }
    return BDMF_IRQ_HANDLED;
}

static void dhd_sim_resources_free(dhd_sim_drv_priv_t *dhd_sim_data)
{
    int i;

    /* rdd_mm_aligned_free in simulator does nothing, so only need to reset pointers */
    dhd_sim_data->dhd_init_cfg.rx_post_flow_ring_base_addr = NULL;
    dhd_sim_data->dhd_init_cfg.rx_complete_flow_ring_base_addr = NULL;
    dhd_sim_data->dhd_init_cfg.tx_post_mgmt_arr_base_addr = NULL;
    dhd_sim_data->dhd_init_cfg.tx_post_mgmt_arr_base_phys_addr = 0;
    for (i = 0; i < MAX_TX_POST_FLOW_RINGS; i++)
        dhd_sim_data->tx_post_flow_ring_arr[i] = NULL;
    dhd_sim_data->dhd_init_cfg.tx_complete_flow_ring_base_addr = NULL;
}

#define _dhd_sim_resource_alloc(resource, size, phys, set_to_0) \
    __dhd_sim_resource_alloc(__FUNCTION__, __LINE__, resource, size, phys, set_to_0)
static void *__dhd_sim_resource_alloc(const char *func, int line,
    void **resource, int size, bdmf_phys_addr_t *phys, int set_to_0)
{
    bdmf_phys_addr_t _phys;

    *resource = (void *)rdp_mm_aligned_alloc(size, &_phys);
    if (*resource)
        memset(*resource, set_to_0 ? 0 : 0xff, size); /* May be better deadbeef? */
    else
        bdmf_trace("%s:%d rdp_mm_aligned_alloc failed\n", func, line);

    if (phys)
        *phys = _phys;
    return *resource;
}

static int dhd_sim_resources_alloc(dhd_sim_drv_priv_t *dhd_sim_data)
{
    int rc = BDMF_ERR_INTERNAL, i;
    bdmf_phys_addr_t phys;
    int rx_post_desc_size, rx_cmpl_desc_size, tx_post_desc_size, tx_cmpl_desc_size;

    switch (flow_ring_format[dhd_sim_data->index])
    {           
        case FR_FORMAT_WI_WI64: /* Legacy Work Item */
        {
            rx_post_desc_size = sizeof(RDD_DHD_RX_POST_DESCRIPTOR_DTS);
            rx_cmpl_desc_size = sizeof(RDD_DHD_RX_COMPLETE_DESCRIPTOR_DTS); 
            tx_post_desc_size = sizeof(RDD_DHD_TX_POST_DESCRIPTOR_DTS); 
            tx_cmpl_desc_size = sizeof(RDD_DHD_TX_COMPLETE_DESCRIPTOR_DTS); 
            break;
        }

        case FR_FORMAT_WI_CWI32: /* Compact Work Item with 32b haddr */
        {               
            rx_post_desc_size = sizeof(RDD_DHD_RX_POST_DESCRIPTOR_CWI32_DTS);
            rx_cmpl_desc_size = sizeof(RDD_DHD_RX_COMPLETE_DESCRIPTOR_CWI32_DTS); 
            tx_post_desc_size = sizeof(RDD_DHD_TX_POST_DESCRIPTOR_CWI32_DTS); 
            tx_cmpl_desc_size = sizeof(RDD_DHD_TX_COMPLETE_DESCRIPTOR_CWI32_DTS); 
            break;
        }
        
        default:
        {
            bdmf_print("ERROR: flow ring format: %d not supported\n", flow_ring_format[dhd_sim_data->index]);                
            BUG();                
        }
    } /* switch item_type */
        
    /* RX Post */
    _dhd_sim_resource_alloc(&dhd_sim_data->dhd_init_cfg.rx_post_flow_ring_base_addr,
        rx_post_desc_size * DHD_RX_POST_FLOW_RING_SIZE, NULL, 0);
    if (!dhd_sim_data->dhd_init_cfg.rx_post_flow_ring_base_addr)
        goto err;

    /* RX Complete */
    _dhd_sim_resource_alloc(&dhd_sim_data->dhd_init_cfg.rx_complete_flow_ring_base_addr,
        rx_cmpl_desc_size * DHD_RX_COMPLETE_FLOW_RING_SIZE, NULL, 0);
    if (!dhd_sim_data->dhd_init_cfg.rx_complete_flow_ring_base_addr)
        goto err;

    /* TX Post management array */
    _dhd_sim_resource_alloc(&dhd_sim_data->dhd_init_cfg.tx_post_mgmt_arr_base_addr,
        MAX_TX_POST_FLOW_RINGS * sizeof(rdpa_dhd_flring_cache_t), &phys, 0);
    if (!dhd_sim_data->dhd_init_cfg.tx_post_mgmt_arr_base_addr)
        goto err;
    /* Good enough for rdp simulation (32-bit). */
    dhd_sim_data->dhd_init_cfg.tx_post_mgmt_arr_base_phys_addr = (uint32_t)phys;

    /* TX Post rings */
    for (i = 0; i < MAX_TX_POST_FLOW_RINGS; i++)
    {
        rdpa_dhd_flring_cache_t *entry;
        uint32_t addr_lo, addr_hi;

        entry = (rdpa_dhd_flring_cache_t *)(dhd_sim_data->dhd_init_cfg.tx_post_mgmt_arr_base_addr) + i;

        _dhd_sim_resource_alloc(&dhd_sim_data->tx_post_flow_ring_arr[i],
            tx_post_desc_size * DHD_SIM_TX_POST_PHY_RING_SIZE, NULL, 0);
        if (!dhd_sim_data->tx_post_flow_ring_arr[i])
            goto err;

        GET_ADDR_HIGH_LOW(addr_hi, addr_lo, RDD_VIRT_TO_PHYS(dhd_sim_data->tx_post_flow_ring_arr[i]));

        entry->base_addr_low = __swap4bytes(addr_lo);
        entry->base_addr_high = __swap4bytes(addr_hi & 0xff);
        entry->items = __swap2bytes(DHD_SIM_TX_POST_FLOW_RING_SIZE);
        entry->flags = __swap2bytes((i % 8) << FLOW_RING_FLAG_SSID_SHIFT); /* Assume 8 rings per SSID and SSID-0 not in use */
        entry->backup_first_index = 0;
        entry->backup_last_index = 0;
        entry->backup_num_entries = 0;
        entry->phy_ring_size = __swap2bytes(DHD_SIM_TX_POST_PHY_RING_SIZE);
    }

    /* TX Complete */
    _dhd_sim_resource_alloc(&dhd_sim_data->dhd_init_cfg.tx_complete_flow_ring_base_addr,
        tx_cmpl_desc_size * DHD_TX_COMPLETE_FLOW_RING_SIZE, NULL, 0);
    if (!dhd_sim_data->dhd_init_cfg.tx_complete_flow_ring_base_addr)
        goto err;

    /* Runner to dongle (H2D) pointers. According to DHD = max_sub_queues per radio. As we hold only limited number of
     * TX Post rings, assume number of TX Post rings + RX post + Mcast/Bcast Ring. Size of index = 2 bytes */
    _dhd_sim_resource_alloc(&dhd_sim_data->dhd_init_cfg.r2d_wr_arr_base_addr,
        sizeof(uint16_t) * (MAX_TX_POST_FLOW_RINGS + BCMPCIE_H2D_COMMON_MSGRINGS), &phys, 1);
        
    /* Good enough for rdp simulation (32-bit). */
    dhd_sim_data->dhd_init_cfg.r2d_wr_arr_base_phys_addr = (uint32_t)phys;
    
        
    _dhd_sim_resource_alloc(&dhd_sim_data->dhd_init_cfg.r2d_rd_arr_base_addr,
        sizeof(uint16_t) * (MAX_TX_POST_FLOW_RINGS + BCMPCIE_H2D_COMMON_MSGRINGS), &phys, 1);
        
    /* Good enough for rdp simulation (32-bit). */
    dhd_sim_data->dhd_init_cfg.r2d_rd_arr_base_phys_addr = (uint32_t)phys;
    
    if (!dhd_sim_data->dhd_init_cfg.r2d_wr_arr_base_addr || !dhd_sim_data->dhd_init_cfg.r2d_rd_arr_base_addr)
        goto err;

    /* Dongle to Runner (D2H) pointers */
    _dhd_sim_resource_alloc(&dhd_sim_data->dhd_init_cfg.d2r_wr_arr_base_addr,
        sizeof(uint16_t) * BCMPCIE_COMMON_MSGRINGS, &phys, 1);
        
    /* Good enough for rdp simulation (32-bit). */
    dhd_sim_data->dhd_init_cfg.d2r_wr_arr_base_phys_addr = (uint32_t)phys;
        
    _dhd_sim_resource_alloc(&dhd_sim_data->dhd_init_cfg.d2r_rd_arr_base_addr,
        sizeof(uint16_t) * BCMPCIE_COMMON_MSGRINGS, &phys, 1);
        
    /* Good enough for rdp simulation (32-bit). */
    dhd_sim_data->dhd_init_cfg.d2r_rd_arr_base_phys_addr = (uint32_t)phys;    
    
    if (!dhd_sim_data->dhd_init_cfg.d2r_wr_arr_base_addr || !dhd_sim_data->dhd_init_cfg.d2r_rd_arr_base_addr)
        goto err;

    /* Allocate runner interrupt as we don't have interrupt to PCI support, and put in dongle_wakeup_register. We don't
     * register host-based doorbell_isr as we will expect to wake up dongle directly from Runner. If will change the
     * implementation to use host-based register, can change the assignment of dhd_sim_dongle_tx_post_isr to be
     * doorbell_isr. */
   
    dhd_sim_data->dhd_init_cfg.dongle_wakeup_register = xrdp_virt2phys(&RU_BLK(UBUS_SLV), 0) + RU_REG_OFFSET(UBUS_SLV, RNR_INTR_CTRL_ITR);
    dhd_sim_data->dhd_init_cfg.dongle_wakeup_register_2 = xrdp_virt2phys(&RU_BLK(UBUS_SLV), 0) + RU_REG_OFFSET(UBUS_SLV, RNR_INTR_CTRL_ITR);
    
    rc = bdmf_int_connect(DONGLE_WAKEUP_REGISTER_0 + dhd_sim_data->index,
        rdpa_cpu_host, 0, /* cpu index and flags are not in use in simulation environment */
        dhd_sim_dongle_tx_post_isr, "DHD_SIM", dhd_sim_data); 
    if (rc)
        goto err;

    return 0;

err:
    bdmf_trace("Resources allocation failed\n");
    dhd_sim_resources_free(dhd_sim_data);
    return rc;
}

static int dhd_helper_object_create(dhd_sim_drv_priv_t *dhd_sim_data)
{
    int rc;

    BDMF_MATTR(dhd_helper_attr, rdpa_dhd_helper_drv());

    rdpa_dhd_helper_radio_idx_set(dhd_helper_attr, dhd_sim_data->index);
    rdpa_dhd_helper_init_cfg_set(dhd_helper_attr, &dhd_sim_data->dhd_init_cfg);
    rc = bdmf_new_and_set(rdpa_dhd_helper_drv(), NULL, dhd_helper_attr, &dhd_sim_data->dhd_helper_obj);
    if (rc)
        return rc;

    return rdpa_dhd_helper_rx_post_init(dhd_sim_data->dhd_helper_obj);
}

/* ISR to read exception packets and packets sent via send_packet_to_dongle, for which TX complete is received by CPU.
 * Both use the same interrupt */
void dhd_sim_dongle_cpu_tx_complete_isr(long isr_priv)
{
    int rc, num_of_descrs = 0;
    dhd_sim_drv_priv_t *dhd_sim_data = (dhd_sim_drv_priv_t *)isr_priv;
    rdpa_dhd_complete_data_t dhd_complete_data = {dhd_sim_data->index, 0, 0, 0};
    
    /* Read exceptions packets. Will be dumped to cpu-sim.rx */
    while (1)
    {
        rdpa_cpu_rx_info_t cpu_rx_info = {};

        rc = rdpa_cpu_packet_get(rdpa_cpu_host, dhd_sim_data->cpu_queue_id, &cpu_rx_info);
        if (rc)
            break;
    }

    /* Handle CPU TX Completes: poll for rdpa_dhd_helper_dhd_complete_message_get, and for every descriptor free the
     * skb */ 
    while (1)
    {
        rc = rdpa_dhd_helper_dhd_complete_message_get(&dhd_complete_data);
        if (rc)
            break;
        /* XXX: Dump to file? */
        bdmf_trace("Releasing pointer of data invoked by send_packet_to_dongle debug API\n");
        bdmf_trace("Radio idx %d, request id 0x%x, flow_ring_id %d, buf_type %d, txp %p, status 0x%x\n",
            dhd_complete_data.radio_idx, dhd_complete_data.request_id, __swap2bytes(dhd_complete_data.flow_ring_id),
            dhd_complete_data.buf_type, dhd_complete_data.txp, __swap2bytes(dhd_complete_data.status));
        
        if (dhd_complete_data.txp)
            dev_kfree_skb(dhd_complete_data.txp);
        num_of_descrs++;
    }
    bdmf_trace("Got %d TX Complete descriptors in total\n", num_of_descrs);
}

static int wlan_sim_rx_file_write(bdmf_session_handle session, const char *buf, uint32_t size)
{
    if (!g_wlan_rx_file)
        return 0;
    return fwrite(buf, 1, size, g_wlan_rx_file);
}


static int dhd_sim_cpu_attach(dhd_sim_drv_priv_t *dhd_sim_data)
{
    int rc;
    rdpa_cpu_rxq_cfg_t rxq_cfg = {};
    rdpa_dhd_cpu_data_t cpu_data = {};
    bdmf_object_handle system_obj = NULL;
    bdmf_session_parm_t session_parm = {};
    
    /* Should be cpu/index=wlan0/1/2. Since we don't create them in UT environment, use rdpa_cpu_host with
     * un-used queue id */
    rc = rdpa_cpu_get(rdpa_cpu_host, &dhd_sim_data->cpu_obj);
    if (rc)
        return rc;
    bdmf_trace("dhd_sim_cpu_attach: next_avail_cpu_queue =  %d\n", next_avail_cpu_queue);
    dhd_sim_data->cpu_queue_id = next_avail_cpu_queue++;
    rxq_cfg.type = rdpa_ring_data;
    rxq_cfg.size = RDPA_DHD_HELPER_CPU_QUEUE_SIZE;
    rxq_cfg.isr_priv = (long)dhd_sim_data;
    rxq_cfg.rx_isr = dhd_sim_dongle_cpu_tx_complete_isr; 
    rxq_cfg.ic_cfg.ic_enable = 0;
    rxq_cfg.dump = 1;
    rc = rdpa_cpu_rxq_cfg_set(dhd_sim_data->cpu_obj, dhd_sim_data->cpu_queue_id, &rxq_cfg);
    if (rc)
        return rc;

    /* Since same vport will be used for both WDF and DHD offload interface, for exception we need to use different
     * reasons to map them to different cpu queues. For that purpose, we will use pci_ip_flow_miss reason. */
    rdpa_system_get(&system_obj);
    rc = rdpa_system_cpu_reason_to_tc_set(system_obj, rdpa_cpu_rx_reason_pci_ip_flow_miss_1 + dhd_sim_data->index,
        dhd_sim_data->cpu_queue_id);
    bdmf_put(system_obj);
    if (rc)
        return rc;

    /* Create CPU ring for TX Complete messages from runner to DHD. Will be handled by same ISR as exceptions CPU
     * ring */
    rc = rdpa_dhd_helper_dhd_complete_ring_create(dhd_sim_data->index, DHD_SIM_CPU_TX_COMPLETE_FLOW_RING_SIZE);
    if (rc)
        return rc;
    cpu_data.cpu_port = rdpa_cpu_host;
    cpu_data.exception_rxq = dhd_sim_data->cpu_queue_id;
    rc = rdpa_dhd_helper_cpu_data_set(dhd_sim_data->dhd_helper_obj, &cpu_data);
    
    if (!g_wlan_rx_file_session)
    {
        /* before handling messages, open cpu-rx file session */
        g_wlan_rx_file = fopen("wlan.rx", "w");
    
        memset(&session_parm, 0, sizeof(session_parm));
        /* TODO: can keep g_cpu_rx_file in session_parm private data? (instead of global) and use it from there in cpu_sim_rx_file_write */
        session_parm.name = "wlan.rx";
        session_parm.hex_dump_format = BDMF_HEX_DUMP_FORMAT_BYTE;
        session_parm.write = wlan_sim_rx_file_write;
        rc = bdmf_session_open(&session_parm, &g_wlan_rx_file_session);
    }
    
    return rc;
}

static void dhd_sim_cpu_detach(dhd_sim_drv_priv_t *dhd_sim_data)
{
    rdpa_cpu_rxq_cfg_t rxq_cfg;

    if (!dhd_sim_data->cpu_obj)
        return;

    /* Destroy CPU ring for TX Complete message from runner to DHD */
    rdpa_dhd_helper_dhd_complete_ring_destroy(dhd_sim_data->index, DHD_SIM_CPU_TX_COMPLETE_FLOW_RING_SIZE);

    rdpa_cpu_rxq_flush_set(dhd_sim_data->cpu_obj, dhd_sim_data->cpu_queue_id, 1);

    /* Un-configure the ring. We don't care about reason_to_tc map as no-one is going to handle these packets. */
    rxq_cfg.size = 0;
    rxq_cfg.ic_cfg.ic_enable = 0;
    rxq_cfg.rx_dump_data_cb = NULL;
    rdpa_cpu_rxq_cfg_set(dhd_sim_data->cpu_obj, dhd_sim_data->cpu_queue_id, &rxq_cfg);

    bdmf_put(dhd_sim_data->cpu_obj);
    dhd_sim_data->cpu_obj = NULL;
}

static void dhd_sim_obj_cleanup(dhd_sim_drv_priv_t *dhd_sim_data)
{
    if (dhd_sim_data->cpu_obj)
        dhd_sim_cpu_detach(dhd_sim_data);

    /* Release DHD object */
    if (dhd_sim_data->dhd_helper_obj)
    {
        rdpa_dhd_helper_rx_post_uninit(dhd_sim_data->dhd_helper_obj);
        bdmf_destroy(dhd_sim_data->dhd_helper_obj);
        dhd_sim_data->dhd_helper_obj = NULL;
    }
    
    /* Release allocated resources */
    dhd_sim_resources_free(dhd_sim_data);
}

/** allocate all resources required for dhd dongle and create an object for dhd_helper */
static int dhd_sim_post_init(struct bdmf_object *mo)
{
    dhd_sim_drv_priv_t *dhd_sim_data = (dhd_sim_drv_priv_t *)bdmf_obj_data(mo);
    int i, rc;

    if (dhd_sim_data->index == BDMF_INDEX_UNASSIGNED)
    {
        for (i = 0; i < RDPA_MAX_RADIOS; i++)
        {
            if (!dhd_sim_object[i])
            {
                dhd_sim_data->index = i;
                break;
            }
        }
    }
    if (dhd_sim_data->index >= RDPA_MAX_RADIOS)
    {
        BDMF_TRACE_RET(BDMF_ERR_PARM, "Too many dhd_sim objects or index %d is out of range\n",
            (int)dhd_sim_data->index);
    }
    /* set object name */
    if (dhd_sim_object[dhd_sim_data->index])
    {
        BDMF_TRACE_RET(BDMF_ERR_ALREADY,
            "WLAN Sim interface %d is already configured\n", (int)dhd_sim_data->index);
    }
        
    /* Simulation for backup queues - delay dongle response by dongle_proc_delay_cnt packets
       At least dongle_proc_delay_cnt packets should exist in simulation */
       
    dongle_proc_delay_cnt[dhd_sim_data->index] = DHD_SIM_DONGLE_PROC_DELAY;
    
    if (dhd_sim_data->index % 2)
        dhd_sim_data->dhd_init_cfg.flow_ring_format = FR_FORMAT_WI_WI64;
    else
        dhd_sim_data->dhd_init_cfg.flow_ring_format = FR_FORMAT_WI_CWI32;
    
    bdmf_trace("dhd_sim_post_init: FlowRing format for radio %d =  %d\n", (int)dhd_sim_data->index, dhd_sim_data->dhd_init_cfg.flow_ring_format);
    
    dhd_sim_data->dhd_init_cfg.dongle_wakeup_hwa = 0;
    
    /* Allocate resources */
    rc = dhd_sim_resources_alloc(dhd_sim_data);
    if (rc)
        goto exit;

    /* Create DHD object */
    rc = dhd_helper_object_create(dhd_sim_data);
    if (rc)
        goto exit;

    rdpa_dhd_helper_wakeup_information_get(&dhd_sim_data->wakeup_from_pci_info);

    rc = dhd_sim_cpu_attach(dhd_sim_data);
    if (rc)
        goto exit;

    dhd_sim_data->enabled = 1;
    snprintf(mo->name, sizeof(mo->name), "dhd_sim/index=%d", (int)dhd_sim_data->index);
    dhd_sim_object[dhd_sim_data->index] = mo;

exit:
    if (rc)
        dhd_sim_obj_cleanup(dhd_sim_data);
    return rc;
}

static void dhd_sim_destroy(struct bdmf_object *mo)
{
    dhd_sim_drv_priv_t *dhd_sim_data = (dhd_sim_drv_priv_t *)bdmf_obj_data(mo);

    if (mo != dhd_sim_object[dhd_sim_data->index])
        return;

    if (g_wlan_rx_file)
        fclose(g_wlan_rx_file);
 
    if (g_wlan_rx_file_session)
        bdmf_session_close(g_wlan_rx_file_session); 

    g_wlan_rx_file = NULL;
    g_wlan_rx_file_session = NULL;
    dhd_sim_data->enabled = 0;
    dhd_sim_obj_cleanup(dhd_sim_data);
    dhd_sim_object[dhd_sim_data->index] = NULL;   
}

/* Aggregate for "send_packet_to_dongle" attribute */
struct bdmf_aggr_type cpu_tx_post_info_type = {
   .name = "cpu_tx_post_info", .struct_name = "rdpa_dhd_tx_post_info_t",
    .help = "CPU TX Post info (debugging support)",
    .fields = (struct bdmf_attr[]) {
        { .name = "radio_idx", .help = "Radio index",
            .type = bdmf_attr_number, .size = sizeof(uint32_t),
            .offset = offsetof(rdpa_dhd_tx_post_info_t, radio_idx),
        },
        { .name = "flow_ring_id", .help = "Flow Ring ID",
            .type = bdmf_attr_number, .size = sizeof(uint32_t),
            .offset = offsetof(rdpa_dhd_tx_post_info_t, flow_ring_id)
        },
        { .name = "ssid", .help = "SSID index in radio",
            .type = bdmf_attr_number, .size = sizeof(uint32_t),
            .offset = offsetof(rdpa_dhd_tx_post_info_t, ssid_if_idx)
        },
        BDMF_ATTR_LAST
    }
};
DECLARE_BDMF_AGGREGATE_TYPE(cpu_tx_post_info_type);

static int dhd_sim_attr_send_packet_to_dongle(struct bdmf_object *mo,
    struct bdmf_attr *ad, bdmf_index index, const void *val, uint32_t size)
{
    rdpa_dhd_tx_post_info_t *tx_post_info = (rdpa_dhd_tx_post_info_t *)index;
    struct sk_buff *skb;
    
    if (!tx_post_info || (*(bdmf_index *)tx_post_info == BDMF_INDEX_UNASSIGNED) ||
        !val || !size)
    {
        return BDMF_ERR_PARM;
    }

    /* Map tx info, assuming host buffer, no speed service supported */
    tx_post_info->is_bpm = 0;
    tx_post_info->is_spdsvc_setup_packet = 0;

    skb = dev_alloc_skb(size + RDD_PACKET_HEADROOM_OFFSET);
    if (!skb)
        return BDMF_ERR_NOMEM;
    skb_put(skb, size);
    skb_reserve(skb, RDD_PACKET_HEADROOM_OFFSET);
    memcpy(skb->data, val, size);
    return rdpa_dhd_helper_send_packet_to_dongle(skb, size+4, tx_post_info);
}

static int dhd_sim_attr_receive_packet_from_dongle(struct bdmf_object *mo,
    struct bdmf_attr *ad, bdmf_index index, const void *val, uint32_t size)
{
    dhd_sim_drv_priv_t *dhd_sim_data = (dhd_sim_drv_priv_t *)bdmf_obj_data(mo);
    uint16_t *rx_post_rd_ptr, *rx_post_wr_ptr;
    uint16_t rx_post_wr_idx, rx_post_rd_idx;
    void *rx_post_descr, *rx_cmpl_descr;
    uint16_t *rx_cmpl_rd_ptr, *rx_cmpl_wr_ptr;
    uint16_t rx_cmpl_wr_idx, rx_cmpl_rd_idx;
    void  *data_buf_ptr;
    uint8_t data_offset = 0x5a;

    if (!val || !size)
        return BDMF_ERR_PARM;

        
    rx_post_rd_ptr = ((uint16_t *)dhd_sim_data->dhd_init_cfg.r2d_rd_arr_base_addr) + DHD_RX_POST_RING;
    rx_post_wr_ptr = ((uint16_t *)dhd_sim_data->dhd_init_cfg.r2d_wr_arr_base_addr) + DHD_RX_POST_RING;

    rx_post_wr_idx = *rx_post_wr_ptr;
    rx_post_rd_idx = *rx_post_rd_ptr;
    
    

    rx_cmpl_rd_ptr = ((uint16_t *)dhd_sim_data->dhd_init_cfg.d2r_rd_arr_base_addr) + DHD_RX_COMPLETE_RING;
    rx_cmpl_wr_ptr = ((uint16_t *)dhd_sim_data->dhd_init_cfg.d2r_wr_arr_base_addr) + DHD_RX_COMPLETE_RING;

    rx_cmpl_rd_idx = (*rx_cmpl_rd_ptr);
    rx_cmpl_wr_idx = (*rx_cmpl_wr_ptr);
    
    bdmf_trace("Receive packet from dongle\n");
    bdmf_trace("Rx post {wr_idx = %d, rd_idx = %d} | Rx complete {wr_idx = %d, rd_idx = %d}\n", rx_post_wr_idx, rx_post_rd_idx, rx_cmpl_wr_idx, rx_cmpl_rd_idx);

    if (rx_post_wr_idx == rx_post_rd_idx)
    {
        BDMF_TRACE_RET(BDMF_ERR_NOMEM, "RX Post ring is empty, rd_idx = %d, wr_idx = %d\n", rx_post_wr_idx,
            rx_post_rd_idx);
    }
    
    if (((rx_cmpl_wr_idx + 1) & ~DHD_RX_COMPLETE_FLOW_RING_SIZE) == rx_cmpl_rd_idx)
    {
        BDMF_TRACE_RET(BDMF_ERR_NOMEM, "RX Complete ring is full, cannot sent the buffer, "
            "rd_idx = %d, wr_idx = %d\n", rx_cmpl_rd_idx, rx_cmpl_wr_idx);
    }

    
    switch (flow_ring_format[dhd_sim_data->index])
    {           
        case FR_FORMAT_WI_WI64: /* Legacy Work Item */
        {
            /* Get RX Post descriptor and update read index in RX Post ring */
            rx_post_descr = (RDD_DHD_RX_POST_DESCRIPTOR_DTS *)dhd_sim_data->dhd_init_cfg.rx_post_flow_ring_base_addr + rx_post_rd_idx; 
            rx_post_rd_idx = (rx_post_rd_idx + 1) & ~DHD_RX_POST_FLOW_RING_SIZE;
            *rx_post_rd_ptr = (rx_post_rd_idx);
        
            /* Prepare RX Complete descriptor, copy data to FPM, request_id serves as FPM BN */
            data_buf_ptr = RDD_PHYS_TO_VIRT((((uint64_t)(((RDD_DHD_RX_POST_DESCRIPTOR_DTS *)rx_post_descr)->data_buf_addr_hi) << 32) | 
                (((RDD_DHD_RX_POST_DESCRIPTOR_DTS *)rx_post_descr)->data_buf_addr_low)) + data_offset);
            memcpy(data_buf_ptr, val, size); 
        
            rx_cmpl_descr = (RDD_DHD_RX_COMPLETE_DESCRIPTOR_DTS *)dhd_sim_data->dhd_init_cfg.rx_complete_flow_ring_base_addr +
                rx_cmpl_wr_idx; 
            memset(rx_cmpl_descr, 0, sizeof(RDD_DHD_RX_COMPLETE_DESCRIPTOR_DTS));
            RDD_DHD_RX_COMPLETE_DESCRIPTOR_MSG_TYPE_WRITE(0x13, rx_cmpl_descr);
            RDD_DHD_RX_COMPLETE_DESCRIPTOR_REQUEST_ID_WRITE(__swap4bytes(((RDD_DHD_RX_POST_DESCRIPTOR_DTS *)rx_post_descr)->request_id), rx_cmpl_descr);
            RDD_DHD_RX_COMPLETE_DESCRIPTOR_DATA_LEN_WRITE(__swap2bytes(size), rx_cmpl_descr);
            RDD_DHD_RX_COMPLETE_DESCRIPTOR_DATA_OFFSET_WRITE(__swap2bytes(data_offset), rx_cmpl_descr);
            RDD_DHD_RX_COMPLETE_DESCRIPTOR_IF_ID_WRITE(2, rx_cmpl_descr); /* XXX: Always send from SSID-2 */
            RDD_DHD_RX_COMPLETE_DESCRIPTOR_DMA_DONE_MARK_WRITE(1, rx_cmpl_descr);
            break;
        }

        case FR_FORMAT_WI_CWI32: /* Compact Work Item with 32b haddr */
        {                           
            /* Get RX Post descriptor and update read index in RX Post ring */
            rx_post_descr = (RDD_DHD_RX_POST_DESCRIPTOR_CWI32_DTS *)dhd_sim_data->dhd_init_cfg.rx_post_flow_ring_base_addr + rx_post_rd_idx; 
            rx_post_rd_idx = (rx_post_rd_idx + 1) & ~DHD_RX_POST_FLOW_RING_SIZE;
            *rx_post_rd_ptr = (rx_post_rd_idx);

            /* Prepare RX Complete descriptor, copy data to FPM, request_id serves as FPM BN */
            data_buf_ptr = RDD_PHYS_TO_VIRT((uint64_t) ((((RDD_DHD_RX_POST_DESCRIPTOR_CWI32_DTS *)rx_post_descr)->data_buf_addr_low)) + data_offset);
            memcpy(data_buf_ptr, val, size); 
        
            rx_cmpl_descr = (RDD_DHD_RX_COMPLETE_DESCRIPTOR_CWI32_DTS *)dhd_sim_data->dhd_init_cfg.rx_complete_flow_ring_base_addr +
                rx_cmpl_wr_idx; 
            memset(rx_cmpl_descr, 0, sizeof(RDD_DHD_RX_COMPLETE_DESCRIPTOR_CWI32_DTS));    
            RDD_DHD_RX_COMPLETE_DESCRIPTOR_CWI32_REQUEST_ID_WRITE(__swap4bytes(((RDD_DHD_RX_POST_DESCRIPTOR_CWI32_DTS *)rx_post_descr)->request_id), rx_cmpl_descr);
            RDD_DHD_RX_COMPLETE_DESCRIPTOR_CWI32_DATA_LEN_WRITE(__swap2bytes(size), rx_cmpl_descr);
            RDD_DHD_RX_COMPLETE_DESCRIPTOR_CWI32_DATA_OFFSET_WRITE((data_offset), rx_cmpl_descr);
            RDD_DHD_RX_COMPLETE_DESCRIPTOR_CWI32_IF_ID_WRITE(2, rx_cmpl_descr); /* XXX: Always send from SSID-2 */
            break;
        }
        
        default:
        {
            bdmf_print("ERROR: flow ring format: %d not supported\n", flow_ring_format[dhd_sim_data->index]);                
            BUG();                
        }
    } /* switch item_type */
        
    rx_cmpl_wr_idx = (rx_cmpl_wr_idx + 1) & ~DHD_RX_COMPLETE_FLOW_RING_SIZE;
    *rx_cmpl_wr_ptr = (rx_cmpl_wr_idx); 

    /* Wakeup runner. Use wakeup from host as cannot simulate wakeup from PCI */
    rdpa_dhd_helper_complete_wakeup(dhd_sim_data->index, 0);
    return 0;
}

/* DHD initial configuration */
struct bdmf_aggr_type wakeup_from_pci_info_type =
{
    .name = "wakeup_from_pci_info", .struct_name = "rdpa_dhd_wakeup_info_t",
    .help = "Wakeup info from PCI. Read only information, cannot be tested from simulator",
    .fields = (struct bdmf_attr[]) {
        { .name = "radio_idx", .help = "Radion index", .size = sizeof(uint32_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_dhd_wakeup_info_t, radio_idx)
        },
        { .name = "tx_complete_wakeup_register", .help = "TX Complete_wakeup register", .size = sizeof(uint32_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_dhd_wakeup_info_t, tx_complete_wakeup_register)
        },
        { .name = "tx_complete_wakeup_value", .help = "TX Complete_wakeup register value", .size = sizeof(uint32_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_dhd_wakeup_info_t, tx_complete_wakeup_value)
        },
        { .name = "rx_complete_wakeup_register", .help = "RX Complete_wakeup register", .size = sizeof(uint32_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_dhd_wakeup_info_t, rx_complete_wakeup_register)
        },
        { .name = "Rx_complete_wakeup_value", .help = "RX Complete_wakeup register value", .size = sizeof(uint32_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_dhd_wakeup_info_t, rx_complete_wakeup_value)
        },
        BDMF_ATTR_LAST
    },
};
DECLARE_BDMF_AGGREGATE_TYPE(wakeup_from_pci_info_type);

static int dhd_sim_attr_enable_ring(struct bdmf_object *mo,
    struct bdmf_attr *ad, bdmf_index index, const void *val, uint32_t size)
{
    dhd_sim_drv_priv_t *dhd_sim_data = (dhd_sim_drv_priv_t *)bdmf_obj_data(mo);
    uint32_t flow_ring_idx = *(uint32_t *)val;

    return rdpa_dhd_helper_flow_ring_enable_set(dhd_sim_data->dhd_helper_obj, flow_ring_idx, 1);
}

static int dhd_sim_attr_disable_ring(struct bdmf_object *mo,
    struct bdmf_attr *ad, bdmf_index index, const void *val, uint32_t size)
{
    dhd_sim_drv_priv_t *dhd_sim_data = (dhd_sim_drv_priv_t *)bdmf_obj_data(mo);
    uint32_t flow_ring_idx = *(uint32_t *)val;

    return rdpa_dhd_helper_flow_ring_enable_set(dhd_sim_data->dhd_helper_obj, flow_ring_idx, 0);
}

static int dhd_sim_attr_flush_ring(struct bdmf_object *mo,
    struct bdmf_attr *ad, bdmf_index index, const void *val, uint32_t size)
{
    dhd_sim_drv_priv_t *dhd_sim_data = (dhd_sim_drv_priv_t *)bdmf_obj_data(mo);
    uint32_t flow_ring_idx = *(uint32_t *)val;

    return rdpa_dhd_helper_flush_set(dhd_sim_data->dhd_helper_obj, flow_ring_idx);
}

static int dhd_sim_attr_aggr_timeout_read(struct bdmf_object *mo,
     struct bdmf_attr *ad, bdmf_index index, void *val, uint32_t size)
{
    dhd_sim_drv_priv_t *dhd_sim_data = (dhd_sim_drv_priv_t *)bdmf_obj_data(mo);
    uint32_t ac = (uint32_t)index;
    bdmf_number *th = (bdmf_number *)val;

    return rdpa_dhd_helper_aggregation_timeout_get(dhd_sim_data->dhd_helper_obj, ac, th);
}

static int dhd_sim_attr_aggr_timeout_write(struct bdmf_object *mo,
    struct bdmf_attr *ad, bdmf_index index, const void *val, uint32_t size)
{
    dhd_sim_drv_priv_t *dhd_sim_data = (dhd_sim_drv_priv_t *)bdmf_obj_data(mo);
    uint32_t ac = (uint32_t)index;
    uint8_t th = *(uint8_t *)val;

    return rdpa_dhd_helper_aggregation_timeout_set(dhd_sim_data->dhd_helper_obj, ac, th);
}

static int dhd_sim_attr_aggr_size_read(struct bdmf_object *mo,
     struct bdmf_attr *ad, bdmf_index index, void *val, uint32_t size)
{
    dhd_sim_drv_priv_t *dhd_sim_data = (dhd_sim_drv_priv_t *)bdmf_obj_data(mo);
    uint32_t ac = (uint32_t)index;
    bdmf_number *_size = (bdmf_number *)val;

    return rdpa_dhd_helper_aggregation_size_get(dhd_sim_data->dhd_helper_obj, ac, _size);
}

static int dhd_sim_attr_aggr_size_write(struct bdmf_object *mo,
    struct bdmf_attr *ad, bdmf_index index, const void *val, uint32_t size)
{
    dhd_sim_drv_priv_t *dhd_sim_data = (dhd_sim_drv_priv_t *)bdmf_obj_data(mo);
    uint32_t ac = (uint32_t)index;
    uint8_t _size = *(uint8_t *)val;

    return rdpa_dhd_helper_aggregation_size_set(dhd_sim_data->dhd_helper_obj, ac, _size);
}

static int dhd_sim_attr_aggr_bypass_cpu_tx_read(struct bdmf_object *mo,
     struct bdmf_attr *ad, bdmf_index index, void *val, uint32_t size)
{
    dhd_sim_drv_priv_t *dhd_sim_data = (dhd_sim_drv_priv_t *)bdmf_obj_data(mo);
    bdmf_boolean *enable = (bdmf_boolean *)val;

    return rdpa_dhd_helper_aggregation_bypass_cpu_tx_get(dhd_sim_data->dhd_helper_obj, enable);
}

static int dhd_sim_attr_aggr_bypass_cpu_tx_write(struct bdmf_object *mo,
    struct bdmf_attr *ad, bdmf_index index, const void *val, uint32_t size)
{
    dhd_sim_drv_priv_t *dhd_sim_data = (dhd_sim_drv_priv_t *)bdmf_obj_data(mo);
    bdmf_boolean enable = *(bdmf_boolean *)val;

    return rdpa_dhd_helper_aggregation_bypass_cpu_tx_set(dhd_sim_data->dhd_helper_obj, enable);
}

static int dhd_sim_attr_aggr_bypass_non_udp_tcp_read(struct bdmf_object *mo,
     struct bdmf_attr *ad, bdmf_index index, void *val, uint32_t size)
{
    dhd_sim_drv_priv_t *dhd_sim_data = (dhd_sim_drv_priv_t *)bdmf_obj_data(mo);
    bdmf_boolean *enable = (bdmf_boolean *)val;

    return rdpa_dhd_helper_aggregation_bypass_non_udp_tcp_get(dhd_sim_data->dhd_helper_obj, enable);
}

static int dhd_sim_attr_aggr_bypass_non_udp_tcp_write(struct bdmf_object *mo,
    struct bdmf_attr *ad, bdmf_index index, const void *val, uint32_t size)
{
    dhd_sim_drv_priv_t *dhd_sim_data = (dhd_sim_drv_priv_t *)bdmf_obj_data(mo);
    bdmf_boolean enable = *(bdmf_boolean *)val;

    return rdpa_dhd_helper_aggregation_bypass_non_udp_tcp_set(dhd_sim_data->dhd_helper_obj, enable);
}

static int dhd_sim_attr_aggr_bypass_tcp_pktlen_read(struct bdmf_object *mo,
     struct bdmf_attr *ad, bdmf_index index, void *val, uint32_t size)
{
    dhd_sim_drv_priv_t *dhd_sim_data = (dhd_sim_drv_priv_t *)bdmf_obj_data(mo);
    bdmf_number *pkt_len = (bdmf_number *)val;

    return rdpa_dhd_helper_aggregation_bypass_tcp_pktlen_get(dhd_sim_data->dhd_helper_obj, pkt_len);
}

static int dhd_sim_attr_aggr_bypass_tcp_pktlen_write(struct bdmf_object *mo,
    struct bdmf_attr *ad, bdmf_index index, const void *val, uint32_t size)
{
    dhd_sim_drv_priv_t *dhd_sim_data = (dhd_sim_drv_priv_t *)bdmf_obj_data(mo);
    uint8_t pkt_len = *(uint8_t *)val;

    return rdpa_dhd_helper_aggregation_bypass_tcp_pktlen_set(dhd_sim_data->dhd_helper_obj, pkt_len);
}

static int dhd_sim_attr_ssid_tx_dropped_packets_read(struct bdmf_object *mo, struct bdmf_attr *ad,
    bdmf_index index, void *val, uint32_t size)
{
    dhd_sim_drv_priv_t *dhd_sim_data = (dhd_sim_drv_priv_t *)bdmf_obj_data(mo);
    uint8_t ssid = (uint8_t)index;
    bdmf_number *rnr_drop_cnt = (bdmf_number *)val;

    return rdpa_dhd_helper_ssid_tx_dropped_packets_get(dhd_sim_data->dhd_helper_obj, ssid, rnr_drop_cnt);
}

/* Object attribute descriptors */
static struct bdmf_attr dhd_sim_attrs[] = {
    { .name = "index", .help = "WLAN Sim interface index", .type = bdmf_attr_number, .size = sizeof(bdmf_index),
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE_INIT | BDMF_ATTR_CONFIG | BDMF_ATTR_KEY | BDMF_ATTR_NO_AUTO_GEN,
        .offset = offsetof(dhd_sim_drv_priv_t, index)
    },
    { .name = "dhd_helper", .help = "DHD Helper object",
        .type = bdmf_attr_object, .size = sizeof(bdmf_object_handle), .ts.ref_type_name = "dhd_helper",
        .flags = BDMF_ATTR_READ | BDMF_ATTR_CONFIG | BDMF_ATTR_NO_AUTO_GEN,
        .offset = offsetof(dhd_sim_drv_priv_t, dhd_helper_obj)
    },
    { .name = "send_packet_to_dongle", .help = "Send packet to dongle (TX Post to CPU debug interface)",
        .type = bdmf_attr_buffer, .array_size = 1,
        .index_type = bdmf_attr_aggregate,
        .index_ts.aggr_type_name = "cpu_tx_post_info", .size = 256,
        .flags = BDMF_ATTR_WRITE | BDMF_ATTR_NO_AUTO_GEN,
        .write = dhd_sim_attr_send_packet_to_dongle
    },
    /* XXX: Change to aggregate, should have SSID in the key. Currently hardcoded to send from SSID-2 */
    { .name = "receive_from_dongle", .help = "Receive packet from dongle (RX Complete debug interface)",
        .type = bdmf_attr_buffer, .size = 256,
        .flags = BDMF_ATTR_WRITE | BDMF_ATTR_NO_AUTO_GEN,
        .write = dhd_sim_attr_receive_packet_from_dongle
    },
    { .name = "wakeup_info", .help = "Wakeup info from PCI",
        .type = bdmf_attr_aggregate, .ts.aggr_type_name = "wakeup_from_pci_info",
        .flags = BDMF_ATTR_READ | BDMF_ATTR_CONFIG,
        .offset = offsetof(dhd_sim_drv_priv_t, wakeup_from_pci_info)
    },
    { .name = "ring_enable", .help = "Enable FlowRing",
        .type = bdmf_attr_number, .size = sizeof(uint32_t),
        .flags = BDMF_ATTR_WRITE | BDMF_ATTR_CONFIG,
        .write = dhd_sim_attr_enable_ring
    },
    { .name = "ring_disable", .help = "Disable FlowRing",
        .type = bdmf_attr_number, .size = sizeof(uint32_t),
        .flags = BDMF_ATTR_WRITE | BDMF_ATTR_CONFIG,
        .write = dhd_sim_attr_disable_ring
    },
    { .name = "ring_flush", .help = "Flush FlowRing",
        .type = bdmf_attr_number, .size = sizeof(uint32_t),
        .flags = BDMF_ATTR_WRITE | BDMF_ATTR_CONFIG,
        .write = dhd_sim_attr_flush_ring
    },
    { .name = "aggregation_size", .help = "Number of packets to be aggregated on host side per AC (0-3)",
        .type = bdmf_attr_number, .size = sizeof(uint8_t), .array_size = RDPA_MAX_AC,
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE | BDMF_ATTR_CONFIG,
        .read = dhd_sim_attr_aggr_size_read, .write = dhd_sim_attr_aggr_size_write
    },
    { .name = "aggregation_timeout", .help = "Aggregation expiration timeout on host side per AC",
      .type = bdmf_attr_number, .size = sizeof(uint8_t), .array_size = RDPA_MAX_AC,
      .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE | BDMF_ATTR_CONFIG,
      .read = dhd_sim_attr_aggr_timeout_read, .write = dhd_sim_attr_aggr_timeout_write
    },
    { .name = "aggregation_bypass_cpu_tx", .help = "Bypass aggregation for CPU TX packet",
        .type = bdmf_attr_boolean, .size = sizeof(bdmf_boolean),
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE | BDMF_ATTR_CONFIG,
        .read = dhd_sim_attr_aggr_bypass_cpu_tx_read, .write = dhd_sim_attr_aggr_bypass_cpu_tx_write
    },
    { .name = "aggregation_bypass_non_udp_tcp", .help = "Bypass aggregation for non UDP/TCP forwarding packet",
        .type = bdmf_attr_boolean, .size = sizeof(bdmf_boolean),
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE | BDMF_ATTR_CONFIG,
        .read = dhd_sim_attr_aggr_bypass_non_udp_tcp_read, .write = dhd_sim_attr_aggr_bypass_non_udp_tcp_write
    },
    { .name = "aggregation_bypass_tcp_pktlen", .help = "Bypass aggregation for TCP forwarding less and euqal to this value. Enabled value range is 54 to 255",
        .type = bdmf_attr_number, .size = sizeof(uint8_t),
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE | BDMF_ATTR_CONFIG,
        .read = dhd_sim_attr_aggr_bypass_tcp_pktlen_read, .write = dhd_sim_attr_aggr_bypass_tcp_pktlen_write
    },
    { .name = "ssid_tx_dropped_packets", .help = "SSID Dropped Packets",
        .type = bdmf_attr_number, .size = sizeof(uint32_t), .array_size = 16,
        .flags = BDMF_ATTR_READ | BDMF_ATTR_STAT,
        .read = dhd_sim_attr_ssid_tx_dropped_packets_read
    },
    { .name = "radio_enable",
        .help = "Enable/Disable Radio (when disabled, packets from TX Post rings are not handled)",
        .type = bdmf_attr_boolean,
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE | BDMF_ATTR_CONFIG,
        .offset = offsetof(dhd_sim_drv_priv_t, enabled)
    },

    BDMF_ATTR_LAST
};

static int dhd_sim_drv_init(struct bdmf_type *drv);
static void dhd_sim_drv_exit(struct bdmf_type *drv);

struct bdmf_type dhd_sim_drv = {
    .name = "dhd_sim",
    .parent = "system",
    .description = "WLAN Simulator",
    .drv_init = dhd_sim_drv_init,
    .drv_exit = dhd_sim_drv_exit,
    .pre_init = dhd_sim_pre_init,
    .post_init = dhd_sim_post_init,
    .destroy = dhd_sim_destroy,
    .extra_size = sizeof(dhd_sim_drv_priv_t),
    .aattr = dhd_sim_attrs,
    .max_objs = RDPA_MAX_RADIOS,
};
DECLARE_BDMF_TYPE(rdpa_dhd_sim, dhd_sim_drv);

/* Init/exit module. Cater for GPL layer */
static int dhd_sim_drv_init(struct bdmf_type *drv)
{
    return 0;
}

static void dhd_sim_drv_exit(struct bdmf_type *drv)
{
}

#endif /* RDP_SIM */
