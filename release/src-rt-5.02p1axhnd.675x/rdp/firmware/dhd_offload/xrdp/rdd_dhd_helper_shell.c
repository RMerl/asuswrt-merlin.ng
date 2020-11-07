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



#include "bdmf_shell.h"
#include "rdd.h"
#include "dhd_defs.h"
#include "rdpa_dhd_helper_basic.h"
#include "rdp_drv_proj_cntr.h"
#include "rdd_dhd_helper.h"

extern void *g_dhd_tx_post_mgmt_fr_base_ptr[RDPA_MAX_RADIOS];
extern uint32_t g_dhd_tx_post_mgmt_arr_entry_count[RDPA_MAX_RADIOS];

#define MAKE_BDMF_SHELL_CMD_NOPARM(dir, cmd, help, cb) \
    bdmfmon_cmd_add(dir, cmd, cb, help, BDMF_ACCESS_ADMIN, NULL, NULL)

#define MAKE_BDMF_SHELL_CMD(dir, cmd, help, cb, parms...)   \
{                                                           \
    static bdmfmon_cmd_parm_t cmd_parms[]={                 \
        parms,                                              \
        BDMFMON_PARM_LIST_TERMINATOR                        \
    };                                                      \
    bdmfmon_cmd_add(dir, cmd, cb, help, BDMF_ACCESS_ADMIN, NULL, cmd_parms); \
}

extern void rdpa_common_update_cntr_results_uint32(void *stat_buf, void *accumulated_buf, uint32_t stat_offset_in_bytes, uint32_t cntr_result);
rdpa_dhd_data_stat_t *rdpa_get_accumulative_dhd_data_stat(uint32_t radio_idx);

static RDD_DHD_COMPLETE_COMMON_RADIO_ENTRY_DTS* get_dhd_complete_radio_entry(uint32_t radio_idx)
{
    RDD_DHD_COMPLETE_COMMON_RADIO_DATA_DTS *radio_instance_table_ptr;   
    
    radio_instance_table_ptr = (RDD_DHD_COMPLETE_COMMON_RADIO_DATA_DTS *) RDD_DHD_COMPLETE_COMMON_RADIO_DATA_PTR(get_runner_idx(dhd_complete_runner_image));
    
#if (RDPA_MAX_RADIOS == 1)
    return &radio_instance_table_ptr->entry;
#else
    return &radio_instance_table_ptr->entry[radio_idx];
#endif
}

static int _rdd_print_dhd_tx_post_desc(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{    
    void *desc_ptr;

    uint32_t radio_idx, ring_id, desc_num;
    uint32_t val;
    rdpa_dhd_flring_cache_t *entry_ptr;

    radio_idx = (uint32_t)parm[0].value.unumber;
    ring_id = (uint32_t)parm[1].value.unumber;
    desc_num = (uint32_t)parm[2].value.unumber;

    entry_ptr = (rdpa_dhd_flring_cache_t *)g_dhd_tx_post_mgmt_fr_base_ptr[radio_idx] + ring_id;
    if (!entry_ptr)
    {
        bdmf_session_print(session, "ERROR: Radio ring pointer is uninitialized.\n");
        return -1;
    }

    bdmf_session_print(session, "TX POST descriptor\n");
    
    switch (flow_ring_format[radio_idx])
    {           
        case FR_FORMAT_WI_WI64: /* Legacy Work Item */
        {
#ifdef PHYS_ADDR_64BIT
            desc_ptr = (RDD_DHD_TX_POST_DESCRIPTOR_DTS *)RDD_PHYS_TO_VIRT(((uint64_t)__swap4bytes(entry_ptr->base_addr_high) << 32) + __swap4bytes(entry_ptr->base_addr_low)) + desc_num;
#else
            desc_ptr = (RDD_DHD_TX_POST_DESCRIPTOR_DTS *)RDD_PHYS_TO_VIRT(__swap4bytes(entry_ptr->base_addr_low)) + desc_num;
#endif
            RDD_DHD_TX_POST_DESCRIPTOR_MSG_TYPE_READ(val, desc_ptr);
            bdmf_session_print(session, "msg_type                 = 0x%4.4x\n", val);
            RDD_DHD_TX_POST_DESCRIPTOR_IF_ID_READ(val, desc_ptr);
            bdmf_session_print(session, "if_id                    = 0x%4.4x\n", val);
            RDD_DHD_TX_POST_DESCRIPTOR_REQUEST_ID_READ(val, desc_ptr);
            bdmf_session_print(session, "request_id               = 0x%8.8x\n", val);
            RDD_DHD_TX_POST_DESCRIPTOR_TX_ETH_HDR_0_READ(val, desc_ptr);
            bdmf_session_print(session, "tx_eth_hdr_0             = 0x%8.8x\n", val);
            RDD_DHD_TX_POST_DESCRIPTOR_TX_ETH_HDR_1_READ(val, desc_ptr);
            bdmf_session_print(session, "tx_eth_hdr_1             = 0x%8.8x\n", val);
            RDD_DHD_TX_POST_DESCRIPTOR_TX_ETH_HDR_2_READ(val, desc_ptr);
            bdmf_session_print(session, "tx_eth_hdr_2             = 0x%8.8x\n", val);
            RDD_DHD_TX_POST_DESCRIPTOR_TX_ETH_HDR_3_READ(val, desc_ptr);
            bdmf_session_print(session, "tx_eth_hdr_3             = 0x%4.4x\n", val);
            RDD_DHD_TX_POST_DESCRIPTOR_FLAGS_READ(val, desc_ptr);
            bdmf_session_print(session, "flags                    = 0x%4.4x\n", val);
            RDD_DHD_TX_POST_DESCRIPTOR_DATA_BUF_ADDR_LOW_READ(val, desc_ptr);
            bdmf_session_print(session, "data_buf_addr_low    	  = 0x%8.8x\n", __swap4bytes(val));
            RDD_DHD_TX_POST_DESCRIPTOR_DATA_BUF_ADDR_HI_READ(val, desc_ptr);
            bdmf_session_print(session, "data_buf_addr_hi         = 0x%8.8x\n", __swap4bytes(val));    
            RDD_DHD_TX_POST_DESCRIPTOR_DATA_LEN_READ(val, desc_ptr);
            bdmf_session_print(session, "data_len                 = 0x%4.4x\n", __swap2bytes(val));

            break;
        }

        case FR_FORMAT_WI_CWI32: /* Compact Work Item with 32b haddr */
        {               
#ifdef PHYS_ADDR_64BIT
            desc_ptr = (RDD_DHD_TX_POST_DESCRIPTOR_CWI32_DTS *)RDD_PHYS_TO_VIRT(((uint64_t)__swap4bytes(entry_ptr->base_addr_high) << 32) + __swap4bytes(entry_ptr->base_addr_low)) + desc_num;
#else
            desc_ptr = (RDD_DHD_TX_POST_DESCRIPTOR_CWI32_DTS *)RDD_PHYS_TO_VIRT(__swap4bytes(entry_ptr->base_addr_low)) + desc_num;
#endif
            RDD_DHD_TX_POST_DESCRIPTOR_CWI32_IF_ID_READ(val, desc_ptr);
            bdmf_session_print(session, "if_id                    = 0x%4.4x\n", val);
            RDD_DHD_TX_POST_DESCRIPTOR_CWI32_REQUEST_ID_READ(val, desc_ptr);
            bdmf_session_print(session, "request_id               = 0x%8.8x\n", val);
            RDD_DHD_TX_POST_DESCRIPTOR_CWI32_TX_ETH_HDR_0_READ(val, desc_ptr);
            bdmf_session_print(session, "tx_eth_hdr_0             = 0x%8.8x\n", val);
            RDD_DHD_TX_POST_DESCRIPTOR_CWI32_TX_ETH_HDR_1_READ(val, desc_ptr);
            bdmf_session_print(session, "tx_eth_hdr_1             = 0x%8.8x\n", val);
            RDD_DHD_TX_POST_DESCRIPTOR_CWI32_TX_ETH_HDR_2_READ(val, desc_ptr);
            bdmf_session_print(session, "tx_eth_hdr_2             = 0x%8.8x\n", val);
            RDD_DHD_TX_POST_DESCRIPTOR_CWI32_TX_ETH_HDR_3_READ(val, desc_ptr);
            bdmf_session_print(session, "tx_eth_hdr_3             = 0x%4.4x\n", val);
            RDD_DHD_TX_POST_DESCRIPTOR_CWI32_FLAGS_READ(val, desc_ptr);
            bdmf_session_print(session, "flags                    = 0x%4.4x\n", val);
            RDD_DHD_TX_POST_DESCRIPTOR_CWI32_DATA_BUF_ADDR_LOW_READ(val, desc_ptr);
            bdmf_session_print(session, "data_buf_addr        	  = 0x%8.8x\n", __swap4bytes(val));            
            RDD_DHD_TX_POST_DESCRIPTOR_CWI32_DATA_LEN_READ(val, desc_ptr);
            bdmf_session_print(session, "data_len                 = 0x%4.4x\n", __swap2bytes(val));

            break;
        }        
    } /* switch item_type */
        
    return 0;
}

static int _rdd_print_dhd_tx_complete_desc(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    RDD_DHD_COMPLETE_COMMON_RADIO_ENTRY_DTS     *radio_instance_entry_ptr;
    void                                        *desc_ptr;
    uint32_t                                    desc_num, radio_idx; 
    bdmf_phys_addr_t                            base_ptr;
    uint32_t                                    addr_lo,addr_hi,val;

    radio_idx = (uint32_t)parm[0].value.unumber;
    desc_num = (uint32_t)parm[1].value.unumber;

    radio_instance_entry_ptr = get_dhd_complete_radio_entry(radio_idx);

    RDD_DHD_COMPLETE_COMMON_RADIO_ENTRY_TX_COMPLETE_FR_BASE_PTR_LOW_READ(addr_lo, radio_instance_entry_ptr);
    RDD_DHD_COMPLETE_COMMON_RADIO_ENTRY_TX_COMPLETE_FR_BASE_PTR_HIGH_READ(addr_hi, radio_instance_entry_ptr);

#ifdef PHYS_ADDR_64BIT
    base_ptr = ((uint64_t)addr_hi << 32) + addr_lo;
#else
    addr_hi = 0;
    base_ptr = addr_lo | addr_hi;            
#endif      
    
    bdmf_session_print(session, "TX COMPLETE descriptor\n");
    
    switch (flow_ring_format[radio_idx])
    {           
        case FR_FORMAT_WI_WI64: /* Legacy Work Item */
        {
            desc_ptr = (RDD_DHD_TX_COMPLETE_DESCRIPTOR_DTS *)RDD_PHYS_TO_VIRT(base_ptr) + desc_num;
            
            RDD_DHD_TX_COMPLETE_DESCRIPTOR_MSG_TYPE_READ(val, desc_ptr);
            bdmf_session_print(session, "msg_type                 = 0x%4.4x\n", val);
            RDD_DHD_TX_COMPLETE_DESCRIPTOR_IF_ID_READ(val, desc_ptr);
            bdmf_session_print(session, "if_id                    = 0x%4.4x\n", val);
            RDD_DHD_TX_COMPLETE_DESCRIPTOR_COMMON_HDR_FLAGS_READ(val, desc_ptr);
            bdmf_session_print(session, "common_hdr_flags         = 0x%4.4x\n", val);
            RDD_DHD_TX_COMPLETE_DESCRIPTOR_EPOCH_READ(val, desc_ptr);
            bdmf_session_print(session, "epoch                    = 0x%4.4x\n", val);
            RDD_DHD_TX_COMPLETE_DESCRIPTOR_REQUEST_ID_READ(val, desc_ptr);
            bdmf_session_print(session, "request_id               = 0x%8.8x\n", val);
            RDD_DHD_TX_COMPLETE_DESCRIPTOR_STATUS_READ(val, desc_ptr);
            bdmf_session_print(session, "status                   = 0x%4.4x\n", val);
            RDD_DHD_TX_COMPLETE_DESCRIPTOR_FLOW_RING_ID_READ(val, desc_ptr);
            bdmf_session_print(session, "flow_ring_id             = 0x%4.4x\n", __swap2bytes(val));
            RDD_DHD_TX_COMPLETE_DESCRIPTOR_DMA_DONE_MARK_READ(val, desc_ptr);
            bdmf_session_print(session, "dma_done_mark 	          = 0x%8.8x\n", val);
            break;
        }

        case FR_FORMAT_WI_CWI32: /* Compact Work Item with 32b haddr */
        {               
            desc_ptr = (RDD_DHD_TX_COMPLETE_DESCRIPTOR_CWI32_DTS *)RDD_PHYS_TO_VIRT(base_ptr) + desc_num;                        
            
            RDD_DHD_TX_COMPLETE_DESCRIPTOR_CWI32_REQUEST_ID_READ(val, desc_ptr);
            bdmf_session_print(session, "request_id               = 0x%8.8x\n", val);
            break;
        }        
    } /* switch item_type */
    
    return 0;
}

static int _rdd_print_dhd_rx_complete_desc(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    RDD_DHD_COMPLETE_COMMON_RADIO_ENTRY_DTS  *radio_instance_entry_ptr;
    void                                     *desc_ptr;    
    uint32_t                                 desc_num, radio_idx;
    bdmf_phys_addr_t                         base_ptr;
    uint32_t                                 addr_lo,addr_hi, val;

    radio_idx = (uint32_t)parm[0].value.unumber;
    desc_num = (uint32_t)parm[1].value.unumber;
    
    radio_instance_entry_ptr = get_dhd_complete_radio_entry(radio_idx);

    RDD_DHD_COMPLETE_COMMON_RADIO_ENTRY_RX_COMPLETE_FR_BASE_PTR_LOW_READ(addr_lo, radio_instance_entry_ptr);
    RDD_DHD_COMPLETE_COMMON_RADIO_ENTRY_RX_COMPLETE_FR_BASE_PTR_HIGH_READ(addr_hi, radio_instance_entry_ptr);
    
#ifdef PHYS_ADDR_64BIT
    base_ptr = ((uint64_t)addr_hi << 32) + addr_lo;
#else
    addr_hi = 0;
    base_ptr = addr_lo | addr_hi;            
#endif
       
    switch (flow_ring_format[radio_idx])
    {           
        case FR_FORMAT_WI_WI64: /* Legacy Work Item */
        {        
            desc_ptr = (RDD_DHD_RX_COMPLETE_DESCRIPTOR_DTS *)RDD_PHYS_TO_VIRT(base_ptr) + desc_num;
            
            bdmf_session_print(session, "RX COMPLETE descriptor\n");
            RDD_DHD_RX_COMPLETE_DESCRIPTOR_MSG_TYPE_READ(val, desc_ptr);
            bdmf_session_print(session, "msg_type                 = 0x%4.4x\n", val);
            RDD_DHD_RX_COMPLETE_DESCRIPTOR_IF_ID_READ(val, desc_ptr);
            bdmf_session_print(session, "if_id                    = 0x%4.4x\n", val);
            RDD_DHD_RX_COMPLETE_DESCRIPTOR_REQUEST_ID_READ(val, desc_ptr);
            bdmf_session_print(session, "request_id               = 0x%8.8x\n", val);
            RDD_DHD_RX_COMPLETE_DESCRIPTOR_COMMON_HDR_FLAGS_READ(val, desc_ptr);
            bdmf_session_print(session, "compl_msg_hdr_status     = 0x%4.4x\n", val);
            RDD_DHD_RX_COMPLETE_DESCRIPTOR_FLOW_RING_ID_READ(val, desc_ptr);
            bdmf_session_print(session, "flow_ring_id             = 0x%4.4x\n", __swap2bytes(val));
            RDD_DHD_RX_COMPLETE_DESCRIPTOR_DATA_LEN_READ(val, desc_ptr);
            bdmf_session_print(session, "data_len                 = 0x%4.4x\n", __swap2bytes(val));
            RDD_DHD_RX_COMPLETE_DESCRIPTOR_DATA_OFFSET_READ(val, desc_ptr);
            bdmf_session_print(session, "data_offset              = 0x%4.4x\n", __swap2bytes(val));
            RDD_DHD_RX_COMPLETE_DESCRIPTOR_FLAGS_READ(val, desc_ptr);
            bdmf_session_print(session, "flags                    = 0x%4.4x\n", val);
            RDD_DHD_RX_COMPLETE_DESCRIPTOR_RX_STATUS_0_READ(val, desc_ptr);
            bdmf_session_print(session, "rx_status_0              = 0x%8.8x\n", val);
            RDD_DHD_RX_COMPLETE_DESCRIPTOR_RX_STATUS_1_READ(val, desc_ptr);
            bdmf_session_print(session, "rx_status_1	          = 0x%8.8x\n", val);
            RDD_DHD_RX_COMPLETE_DESCRIPTOR_DMA_DONE_MARK_READ(val, desc_ptr);
            bdmf_session_print(session, "dma_done_mark 	          = 0x%8.8x\n", val);        
            break;
        }

        case FR_FORMAT_WI_CWI32: /* Compact Work Item with 32b haddr */
        {                           
            desc_ptr = (RDD_DHD_RX_COMPLETE_DESCRIPTOR_CWI32_DTS *)RDD_PHYS_TO_VIRT(base_ptr) + desc_num;
            bdmf_session_print(session, "RX COMPLETE descriptor\n");
            RDD_DHD_RX_COMPLETE_DESCRIPTOR_CWI32_IF_ID_READ(val, desc_ptr);
            bdmf_session_print(session, "if_id                    = 0x%4.4x\n", val);
            RDD_DHD_RX_COMPLETE_DESCRIPTOR_CWI32_REQUEST_ID_READ(val, desc_ptr);
            bdmf_session_print(session, "request_id               = 0x%8.8x\n", val);
            RDD_DHD_RX_COMPLETE_DESCRIPTOR_CWI32_DATA_LEN_READ(val, desc_ptr);
            bdmf_session_print(session, "data_len                 = 0x%4.4x\n", __swap2bytes(val));
            RDD_DHD_RX_COMPLETE_DESCRIPTOR_CWI32_DATA_OFFSET_READ(val, desc_ptr);
            bdmf_session_print(session, "data_offset              = 0x%4.4x\n", __swap2bytes(val));
            RDD_DHD_RX_COMPLETE_DESCRIPTOR_CWI32_FLAGS_READ(val, desc_ptr);
            bdmf_session_print(session, "flags                    = 0x%4.4x\n", val);      
            break;
        }        
    } /* switch item_type */
    
    return 0;
}

static int _rdd_print_dhd_rx_post_desc(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    RDD_DHD_COMPLETE_COMMON_RADIO_ENTRY_DTS  *radio_instance_entry_ptr;
    void                                     *desc_ptr;
    uint32_t                                 desc_num, radio_idx;
    bdmf_phys_addr_t                         base_ptr;
    uint32_t                                 addr_lo,addr_hi, val;
    
    
    radio_idx = (uint32_t)parm[0].value.unumber;
    desc_num = (uint32_t)parm[1].value.unumber;
   
    radio_instance_entry_ptr = get_dhd_complete_radio_entry(radio_idx);

    RDD_DHD_COMPLETE_COMMON_RADIO_ENTRY_RX_POST_FR_BASE_PTR_LOW_READ(addr_lo, radio_instance_entry_ptr);
    RDD_DHD_COMPLETE_COMMON_RADIO_ENTRY_RX_POST_FR_BASE_PTR_HIGH_READ(addr_hi, radio_instance_entry_ptr);
    
#ifdef PHYS_ADDR_64BIT
    base_ptr = ((uint64_t)addr_hi << 32) + addr_lo;
#else
    addr_hi = 0;
    base_ptr = addr_lo | addr_hi;            
#endif
            
    switch (flow_ring_format[radio_idx])
    {           
        case FR_FORMAT_WI_WI64: /* Legacy Work Item */
        {        
            desc_ptr = (RDD_DHD_RX_POST_DESCRIPTOR_DTS *)RDD_PHYS_TO_VIRT(base_ptr) + desc_num;
            
            bdmf_session_print(session, "RX POST descriptor\n");
            RDD_DHD_RX_POST_DESCRIPTOR_MSG_TYPE_READ(val, desc_ptr);
            bdmf_session_print(session, "msg_type                 = 0x%4.4x\n", val);
            RDD_DHD_RX_POST_DESCRIPTOR_IF_ID_READ(val, desc_ptr);
            bdmf_session_print(session, "if_id                    = 0x%4.4x\n", val);
            RDD_DHD_RX_POST_DESCRIPTOR_COMMON_HDR_FLAGS_READ(val, desc_ptr);
            bdmf_session_print(session, "common_hdr_flags         = 0x%4.4x\n", val);
            RDD_DHD_RX_POST_DESCRIPTOR_EPOCH_READ(val, desc_ptr);
            bdmf_session_print(session, "epoch                    = 0x%4.4x\n", val);    
            RDD_DHD_RX_POST_DESCRIPTOR_REQUEST_ID_READ(val, desc_ptr);
            bdmf_session_print(session, "request_id               = 0x%8.8x\n", val);    
            RDD_DHD_RX_POST_DESCRIPTOR_META_BUF_LEN_READ(val, desc_ptr);
            bdmf_session_print(session, "meta_buf_len             = 0x%4.4x\n", val);
            RDD_DHD_RX_POST_DESCRIPTOR_DATA_LEN_READ(val, desc_ptr);
            bdmf_session_print(session, "data_len                 = 0x%4.4x\n", __swap2bytes(val));
            RDD_DHD_RX_POST_DESCRIPTOR_METADATA_BUF_ADDR_LOW_READ(val, desc_ptr);
            bdmf_session_print(session, "metadata_buf_addr_low    = 0x%8.8x\n", val);
            RDD_DHD_RX_POST_DESCRIPTOR_METADATA_BUF_ADDR_HI_READ(val, desc_ptr);
            bdmf_session_print(session, "metadata_buf_addr_hi     = 0x%8.8x\n", val);    
            RDD_DHD_RX_POST_DESCRIPTOR_DATA_BUF_ADDR_LOW_READ(val, desc_ptr);
            bdmf_session_print(session, "data_buf_addr_low        = 0x%8.8x\n", __swap4bytes(val));    
            RDD_DHD_RX_POST_DESCRIPTOR_DATA_BUF_ADDR_HI_READ(val, desc_ptr);
            bdmf_session_print(session, "data_buf_addr_hi         = 0x%8.8x\n", __swap4bytes(val));        
            break;
        }

        case FR_FORMAT_WI_CWI32: /* Compact Work Item with 32b haddr */
        {                  
            desc_ptr = (RDD_DHD_RX_POST_DESCRIPTOR_CWI32_DTS *)RDD_PHYS_TO_VIRT(base_ptr) + desc_num;
            RDD_DHD_RX_POST_DESCRIPTOR_CWI32_REQUEST_ID_READ(val, desc_ptr);
            bdmf_session_print(session, "request_id               = 0x%8.8x\n", val);       
            RDD_DHD_RX_POST_DESCRIPTOR_CWI32_DATA_BUF_ADDR_LOW_READ(val, desc_ptr);
            bdmf_session_print(session, "data_buf_addr            = 0x%8.8x\n", __swap4bytes(val));          
            break;
        }        
    } /* switch item_type */
    
    

    return 0;
}


static int _rdd_print_dhd_general_counters(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    uint32_t radio_idx;
    uint32_t cntr_arr[MAX_NUM_OF_COUNTERS_PER_READ] = {};
    rdpa_dhd_data_stat_t result_to_print;
    uint32_t offset_in_struct;
    rdpa_dhd_data_stat_t *accumulative_buf;
    
    radio_idx = (uint32_t)parm[0].value.unumber;
    
    bdmf_session_print(session, "General DHD counters\n");

    bdmf_session_print(session, "Radio %d\n", radio_idx);

    accumulative_buf = rdpa_get_accumulative_dhd_data_stat(radio_idx);
    if (!accumulative_buf)
        return BDMF_ERR_PARM;

    drv_cntr_counter_read(CNTR_GROUP_GENERAL, GENERAL_COUNTER_DHD_TX_POST_PKTS_0 + radio_idx, cntr_arr);
    offset_in_struct = offsetof(rdpa_dhd_data_stat_t, dhd_tx_post_packets);
    rdpa_common_update_cntr_results_uint32(&result_to_print, accumulative_buf, offset_in_struct, cntr_arr[0]);

    bdmf_session_print(session, "\tTX_POST packets = %d\n", result_to_print.dhd_tx_post_packets);
    
    drv_cntr_counter_read(CNTR_GROUP_GENERAL, GENERAL_COUNTER_DHD_TX_COMPLETE_PKTS_0 + radio_idx, cntr_arr);
    offset_in_struct = offsetof(rdpa_dhd_data_stat_t, dhd_tx_complete_packets);
    rdpa_common_update_cntr_results_uint32(&result_to_print, accumulative_buf, offset_in_struct, cntr_arr[0]);

    bdmf_session_print(session, "\tTX_COMPLETE packets = %d\n", result_to_print.dhd_tx_complete_packets);
    
    drv_cntr_counter_read(CNTR_GROUP_GENERAL, GENERAL_COUNTER_DHD_RX_COMPLETE_PKTS_0 + radio_idx, cntr_arr);
    offset_in_struct = offsetof(rdpa_dhd_data_stat_t, dhd_rx_complete_packets);
    rdpa_common_update_cntr_results_uint32(&result_to_print, accumulative_buf, offset_in_struct, cntr_arr[0]);

    bdmf_session_print(session, "\tRX_COMPLETE packets = %d\n", result_to_print.dhd_rx_complete_packets);

    drv_cntr_counter_read(CNTR_GROUP_DHD_CTR, DHD_CTR_GROUP_DHD_TX_FPM_USED_0 + radio_idx, cntr_arr);
    result_to_print.dhd_tx_fpm_used = cntr_arr[0];

    bdmf_session_print(session, "\tFPM in use for radio = %d\n", result_to_print.dhd_tx_fpm_used);
    
    drv_cntr_counter_read(CNTR_GROUP_DHD_CTR, DHD_CTR_GROUP_DHD_TX_FPM_USED_TOTAL, cntr_arr);
    result_to_print.dhd_tx_total_fpm_used = cntr_arr[0];

    bdmf_session_print(session, "\tTotal FPM in use = %d\n", result_to_print.dhd_tx_total_fpm_used);
    
    return 0;
}


static int _rdd_print_dhd_drop_counters(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    uint32_t radio_idx, i, cntr_id;
    uint32_t cntr_arr[MAX_NUM_OF_COUNTERS_PER_READ] = {};
    rdpa_dhd_ssid_tx_dropped_t result_ssid;
    rdpa_dhd_data_stat_t result_to_print;
    rdpa_dhd_data_stat_t *accumulative_buf;
    uint32_t dhd_tx_fr_full_0_ac_offset[] =
    {
        offsetof(rdpa_dhd_data_stat_t, dhd_tx_fr_ac_bk_full),
        offsetof(rdpa_dhd_data_stat_t, dhd_tx_fr_ac_be_full),
        offsetof(rdpa_dhd_data_stat_t, dhd_tx_fr_ac_vi_full),
        offsetof(rdpa_dhd_data_stat_t, dhd_tx_fr_ac_vo_full),
        offsetof(rdpa_dhd_data_stat_t, dhd_tx_fr_ac_bc_mc_full)
    };
    
    radio_idx = (uint32_t)parm[0].value.unumber;
    
    bdmf_session_print(session, "SSID drop counters\n");

    bdmf_session_print(session, "Radio %d\n", radio_idx);

    accumulative_buf = rdpa_get_accumulative_dhd_data_stat(radio_idx);

    if (!accumulative_buf)
        return BDMF_ERR_PARM;

    for (i = 0; i < DHD_MAX_SSID_NUM; i++)
    {
        cntr_id = DHD_CNTR_DHD_TX_DROP_0_SSID_0 + radio_idx*DHD_MAX_SSID_NUM + i; /*here should be 16 to keep hw/sw counters allignment*/
        drv_cntr_counter_read(CNTR_GROUP_DHD_CNTRS, cntr_id, cntr_arr);
        result_ssid[i] = cntr_arr[0]; /*not supported accumulation for this counter*/
        bdmf_session_print(session, "\tSSID%d = %d\n", i, result_ssid[i]);
    }

    bdmf_session_print(session, "AC Flow Ring Full counters\n");


    bdmf_session_print(session, "Radio %d\n", radio_idx);

    for (i = 0; i < 5; i++)
    {
    	uint32_t cntr_val;
        cntr_id = DHD_CNTR_DHD_TX_FR_FULL_0_AC_0 + radio_idx*5 + i;
        drv_cntr_counter_read(CNTR_GROUP_DHD_CNTRS, cntr_id, cntr_arr);
        rdpa_common_update_cntr_results_uint32(&result_to_print, accumulative_buf, dhd_tx_fr_full_0_ac_offset[i], cntr_arr[0]);
        cntr_val = *(uint32_t *)((uint8_t *)&result_to_print + dhd_tx_fr_full_0_ac_offset[i]);
        bdmf_session_print(session, "\tAC%d = %d\n", i, cntr_val);
    }

    drv_cntr_counter_read(CNTR_GROUP_DHD_CNTRS, DHD_CNTR_DHD_RX_DROP_0 + radio_idx, cntr_arr);
    rdpa_common_update_cntr_results_uint32(&result_to_print, accumulative_buf, offsetof(rdpa_dhd_data_stat_t, dhd_rx_drop), cntr_arr[0]);
    
    bdmf_session_print(session, "FPM alloc failed for radio %d = %d\n", radio_idx, result_to_print.dhd_rx_drop);
    
    drv_cntr_counter_read(CNTR_GROUP_DHD_CNTRS, DHD_CNTR_DHD_TX_FPM_DROP_0 + radio_idx, cntr_arr);
    rdpa_common_update_cntr_results_uint32(&result_to_print, accumulative_buf, offsetof(rdpa_dhd_data_stat_t, dhd_tx_fpm_drop), cntr_arr[0]);

    bdmf_session_print(session, "FPM congestion normal priority drop for radio %d = %d\n", radio_idx, result_to_print.dhd_tx_fpm_drop);
    
    drv_cntr_counter_read(CNTR_GROUP_DHD_CNTRS, DHD_CNTR_DHD_TX_HIGH_PRIO_FPM_DROP_0 + radio_idx, cntr_arr);
    rdpa_common_update_cntr_results_uint32(&result_to_print, accumulative_buf, offsetof(rdpa_dhd_data_stat_t, dhd_tx_high_prio_fpm_drop), cntr_arr[0]);

    bdmf_session_print(session, "FPM congestion high priority and mcast drop for radio %d = %d\n", radio_idx, result_to_print.dhd_tx_high_prio_fpm_drop);
    
    drv_cntr_counter_read(CNTR_GROUP_DHD_CNTRS, DHD_CNTR_DHD_MCAST_SBPM_DROP_0 + radio_idx, cntr_arr);
    rdpa_common_update_cntr_results_uint32(&result_to_print, accumulative_buf, offsetof(rdpa_dhd_data_stat_t, dhd_mcast_sbpm_drop), cntr_arr[0]);

    bdmf_session_print(session, "Mcast sbpm drop for radio %d = %d\n", radio_idx, result_to_print.dhd_mcast_sbpm_drop);

    return 0;
}

static int _rdd_print_dhd_flow_rings_cache(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    uint32_t radio_idx,i;
    RDD_DHD_FLOW_RING_CACHE_LKP_ENTRY_DTS *cache_lkp_ptr, cache_lkp;
    RDD_DHD_FLOW_RING_CACHE_CTX_ENTRY_DTS *cache_ctx_ptr, ctx_entry;
    RDD_DHD_BACKUP_INFO_CACHE_ENTRY_DTS *backup_info_ptr, backup_info;

    radio_idx = (uint32_t)parm[0].value.unumber;
    
    bdmf_session_print(session, "FlowRings Cache\n");

    cache_lkp_ptr = (RDD_DHD_FLOW_RING_CACHE_LKP_ENTRY_DTS *)RDD_DHD_FLOW_RING_CACHE_LKP_TABLE_PTR(get_runner_idx(dhd_tx_post_runner_image));
    cache_ctx_ptr = (RDD_DHD_FLOW_RING_CACHE_CTX_ENTRY_DTS *)RDD_DHD_FLOW_RING_CACHE_CTX_TABLE_PTR(get_runner_idx(dhd_tx_post_runner_image));
    backup_info_ptr = (RDD_DHD_BACKUP_INFO_CACHE_ENTRY_DTS *)RDD_DHD_BACKUP_INFO_CACHE_TABLE_PTR(get_runner_idx(dhd_tx_post_runner_image));

    bdmf_session_print(session, "Radio   Ring ID  Inv  Base        PhySize  TotalSize   Flags       SSID  RD_Idx  WR_Idx  FirstIdx  LastIdx  BackupEntries\n");
    bdmf_session_print(session, "=====================================================================================================================\n");

    cache_lkp_ptr += radio_idx*16;
    cache_ctx_ptr += radio_idx*16; 
    backup_info_ptr += radio_idx*16;
    
    for (i = 0; i < 16; i++, cache_lkp_ptr++, cache_ctx_ptr++, backup_info_ptr++)
    {
        /* Read Flowring cache entry */
        RDD_DHD_FLOW_RING_CACHE_LKP_ENTRY_FLOW_RING_ID_READ(cache_lkp.flow_ring_id, cache_lkp_ptr);        
        RDD_DHD_FLOW_RING_CACHE_LKP_ENTRY_INVALID_READ(cache_lkp.invalid, cache_lkp_ptr);
        bdmf_session_print(session, "%-5d   %-7d  %-3d  ", (i >> 4), cache_lkp.flow_ring_id, cache_lkp.invalid);

        /* Read FlowRing context entry */
        RDD_DHD_FLOW_RING_CACHE_CTX_ENTRY_FLOW_RING_BASE_LOW_READ(ctx_entry.flow_ring_base_low, cache_ctx_ptr);
        RDD_DHD_FLOW_RING_CACHE_CTX_ENTRY_SIZE_READ(ctx_entry.size, cache_ctx_ptr);
        RDD_DHD_FLOW_RING_CACHE_CTX_ENTRY_FLAGS_READ(ctx_entry.flags, cache_ctx_ptr);
        RDD_DHD_FLOW_RING_CACHE_CTX_ENTRY_SSID_READ(ctx_entry.ssid, cache_ctx_ptr);
        RDD_DHD_FLOW_RING_CACHE_CTX_ENTRY_RD_IDX_READ(ctx_entry.rd_idx, cache_ctx_ptr);
        RDD_DHD_FLOW_RING_CACHE_CTX_ENTRY_WR_IDX_READ(ctx_entry.wr_idx, cache_ctx_ptr);
        
        RDD_DHD_BACKUP_INFO_CACHE_ENTRY_BACKUP_FIRST_INDEX_READ(backup_info.backup_first_index, backup_info_ptr);
        RDD_DHD_BACKUP_INFO_CACHE_ENTRY_BACKUP_LAST_INDEX_READ(backup_info.backup_last_index, backup_info_ptr);
        RDD_DHD_BACKUP_INFO_CACHE_ENTRY_BACKUP_NUM_ENTRIES_READ(backup_info.backup_num_entries, backup_info_ptr);
        RDD_DHD_BACKUP_INFO_CACHE_ENTRY_PHY_SIZE_READ(backup_info.phy_size, backup_info_ptr);
        
        bdmf_session_print(session, "0x%-8x  %-7d  %-9d   0x%-8x  %-4d  %-6d  %-6d  %-8d  %-7d  %-6d\n",
            ctx_entry.flow_ring_base_low, backup_info.phy_size, ctx_entry.size, ctx_entry.flags, ctx_entry.ssid, ctx_entry.rd_idx, ctx_entry.wr_idx,
            backup_info.backup_first_index, backup_info.backup_last_index, backup_info.backup_num_entries);
    }

    return 0;
}

static int _rdd_print_dhd_flow_rings_ddr(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    int i;
    uint32_t radio_idx;
    rdpa_dhd_flring_cache_t *entry_ptr;

    radio_idx = (uint32_t)parm[0].value.unumber;

    entry_ptr = (rdpa_dhd_flring_cache_t *)g_dhd_tx_post_mgmt_fr_base_ptr[radio_idx];
    if (!entry_ptr)
    {
        bdmf_session_print(session, "ERROR: Radio ring pointer is uninitialized.\n");
        return -1;
    }

    bdmf_session_print(session, "Total of %d rings allocated.  Displaying enabled rings only.\n", g_dhd_tx_post_mgmt_arr_entry_count[radio_idx]);
    bdmf_session_print(session, "Ring ID  Radio  Base        PhySize  TotalSize   Flags       SSID  FirstIdx  LastIdx  BackupEntries\n");
    bdmf_session_print(session, "===================================================================================================\n");

    for (i = 2; i < g_dhd_tx_post_mgmt_arr_entry_count[radio_idx]; i++)
    {
        // Short circuit if we come across an uninitialized ring descriptor
        if (!entry_ptr[i].base_addr_low) 
        {
            bdmf_session_print(session, "Invalid base address found, exitting.");
            break;
        }
        
        if (!(__swap2bytes(entry_ptr[i].flags) & FLOW_RING_FLAG_DISABLED))
        {
            bdmf_session_print(session, "%7d  %5d  0x%08x   %-7d  %-8d   0x%-10x  %-4d  %-8d  %-7d  %-13d\n", 
                i, 
                radio_idx,
                __swap4bytes(entry_ptr[i].base_addr_low),
                __swap2bytes(entry_ptr[i].phy_ring_size),
                __swap2bytes(entry_ptr[i].items),
                __swap2bytes(entry_ptr[i].flags) & ~FLOW_RING_FLAG_SSID_MASK,
                (__swap2bytes(entry_ptr[i].flags) & FLOW_RING_FLAG_SSID_MASK) >> FLOW_RING_FLAG_SSID_SHIFT,
                __swap2bytes(entry_ptr[i].backup_first_index),
                __swap2bytes(entry_ptr[i].backup_last_index),
                __swap2bytes(entry_ptr[i].backup_num_entries));
        }
    }

    return 0;
}

static int _rdd_set_dhd_fpm_th(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    uint16_t  low_th, high_th, excl_th;

    low_th = (uint32_t)parm[0].value.unumber;
    high_th = (uint32_t)parm[1].value.unumber;
    excl_th = (uint32_t)parm[2].value.unumber;

    return rdd_dhd_helper_fpm_thresholds_set(low_th, high_th, excl_th);
}

static int _rdd_get_dhd_fpm_th(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    uint16_t low_th, high_th, excl_th;
    uint32_t used;

    rdd_dhd_helper_fpm_thresholds_get(&low_th, &high_th, &excl_th);
    rdd_dhd_helper_tx_total_fpm_used_get(&used);

    bdmf_session_print(session, "DHD FPM low threshold = %d, high threshold = %d, excl threshold = %d, used = %d\n", low_th, high_th, excl_th, used);

    return 0;
}

static int _rdd_set_dhd_coalescing_params(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    uint16_t  radio_idx, coal_thresh, coal_timeout;

    radio_idx = (uint32_t)parm[0].value.unumber;
    coal_thresh = (uint32_t)parm[1].value.unumber;
    coal_timeout = (uint32_t)parm[2].value.unumber;

    RDD_DHD_POST_COMMON_RADIO_ENTRY_COALESCING_MAX_COUNT_WRITE_G(coal_thresh, RDD_DHD_POST_COMMON_RADIO_DATA_ADDRESS_ARR, radio_idx);
    RDD_DHD_POST_COMMON_RADIO_ENTRY_COALESCING_TIMEOUT_WRITE_G(coal_timeout, RDD_DHD_POST_COMMON_RADIO_DATA_ADDRESS_ARR, radio_idx);

    return 0;
}

void rdd_dhd_helper_shell_cmds_init(bdmfmon_handle_t rdd_dir)
{
    MAKE_BDMF_SHELL_CMD(rdd_dir, "pdtxp", "print DHD TX POST descriptor", _rdd_print_dhd_tx_post_desc,
        BDMFMON_MAKE_PARM("radio_idx", "radio index", BDMFMON_PARM_NUMBER, 0),
        BDMFMON_MAKE_PARM("ring_id", "ring id", BDMFMON_PARM_NUMBER, 0),
        BDMFMON_MAKE_PARM("desc_id", "desc id", BDMFMON_PARM_NUMBER, 0));
    MAKE_BDMF_SHELL_CMD(rdd_dir, "pdrxp",   "print DHD RX POST descriptor", _rdd_print_dhd_rx_post_desc,
        BDMFMON_MAKE_PARM("radio_idx", "radio index", BDMFMON_PARM_NUMBER, 0),
        BDMFMON_MAKE_PARM("desc_id", "desc id", BDMFMON_PARM_NUMBER, 0));
    MAKE_BDMF_SHELL_CMD(rdd_dir, "pdtxc",   "print DHD TX COMPLETE descriptor", _rdd_print_dhd_tx_complete_desc,
        BDMFMON_MAKE_PARM("radio_idx", "radio index", BDMFMON_PARM_NUMBER, 0),
        BDMFMON_MAKE_PARM("desc_id", "desc id", BDMFMON_PARM_NUMBER, 0));
    MAKE_BDMF_SHELL_CMD(rdd_dir, "pdrxc",   "print DHD RX COMPLETE descriptor", _rdd_print_dhd_rx_complete_desc,
        BDMFMON_MAKE_PARM("radio_idx", "radio index", BDMFMON_PARM_NUMBER, 0),
        BDMFMON_MAKE_PARM("desc_id", "desc id", BDMFMON_PARM_NUMBER, 0));
    MAKE_BDMF_SHELL_CMD(rdd_dir, "pddc",   "print DHD drop counters", _rdd_print_dhd_drop_counters,
        BDMFMON_MAKE_PARM("radio_idx", "radio index", BDMFMON_PARM_NUMBER, 0));
    MAKE_BDMF_SHELL_CMD(rdd_dir, "pdfrc",   "print DHD Flow Rings cache", _rdd_print_dhd_flow_rings_cache,
        BDMFMON_MAKE_PARM("radio_idx", "radio index", BDMFMON_PARM_NUMBER, 0));
    MAKE_BDMF_SHELL_CMD(rdd_dir, "pdfrd",  "print DHD Flow Rings from ddr", _rdd_print_dhd_flow_rings_ddr,
        BDMFMON_MAKE_PARM("radio_idx", "radio index", BDMFMON_PARM_NUMBER, 0));
    MAKE_BDMF_SHELL_CMD(rdd_dir, "pdgc",   "print DHD general counters", _rdd_print_dhd_general_counters,
        BDMFMON_MAKE_PARM("radio_idx", "radio index", BDMFMON_PARM_NUMBER, 0));
    MAKE_BDMF_SHELL_CMD(rdd_dir, "sdfl",   "set DHD FPM thresholds", _rdd_set_dhd_fpm_th,
        BDMFMON_MAKE_PARM("low_prio_limit", "low_prio_limit", BDMFMON_PARM_NUMBER, 0),
        BDMFMON_MAKE_PARM("high_prio_limit", "high_prio_limit", BDMFMON_PARM_NUMBER, 0),
        BDMFMON_MAKE_PARM("excl_prio_limit", "excl_prio_limit", BDMFMON_PARM_NUMBER, 0));
    MAKE_BDMF_SHELL_CMD_NOPARM(rdd_dir, "gdfl",  "get DHD FPM thresholds", _rdd_get_dhd_fpm_th);
    MAKE_BDMF_SHELL_CMD(rdd_dir, "sdcp",   "set DHD coalescing parameters", _rdd_set_dhd_coalescing_params,
        BDMFMON_MAKE_PARM("radio_idx", "radio index", BDMFMON_PARM_NUMBER, 0),
        BDMFMON_MAKE_PARM("coal_thresh", "number of doorbells to coalesce", BDMFMON_PARM_NUMBER, 0),
        BDMFMON_MAKE_PARM("coal_timeout", "coalescing timeout (msec)", BDMFMON_PARM_NUMBER, 0));
}

