/*
* <:copyright-BRCM:2014-2015:proprietary:standard
* 
*    Copyright (c) 2014-2015 Broadcom 
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

#ifndef _RDPA_DHD_HELPER_EX_H_
#define _RDPA_DHD_HELPER_EX_H_

/* Amount BPM buffers reserved per radio*/
#define DHD_RX_RESERVED_BUFFERS 1024

/* dhd_helper object private data */
typedef struct
{
    bdmf_index radio_idx;
    rdpa_dhd_init_cfg_t init_cfg;
    bdmf_object_handle cpu_port_obj;
    rdpa_dhd_cpu_data_t cpu_data;
} dhd_helper_drv_priv_t;

extern bdmf_object_handle dhd_helper_obj[RDPA_MAX_RADIOS];
extern bdmf_boolean llcsnap_enable[RDPA_MAX_RADIOS];
extern int flow_ring_format[RDPA_MAX_RADIOS];

int dhd_helper_post_init_ex(struct bdmf_object *mo);
void dhd_helper_destroy_ex(struct bdmf_object *mo);
int rdpa_dhd_helper_send_packet_to_dongle_ex(void *data, uint32_t length, const rdpa_dhd_tx_post_info_t *info);

int dhd_helper_attr_flush_write_ex(struct bdmf_object *mo, struct bdmf_attr *ad, 
    bdmf_index index, const void *val, uint32_t size);

int dhd_helper_attr_flow_ring_enable_write_ex(struct bdmf_object *mo, struct bdmf_attr *ad,
    bdmf_index index, const void *val, uint32_t size);

int dhd_helper_tx_complete_host_send2dhd_write_ex(struct bdmf_object *mo, struct bdmf_attr *ad,
    bdmf_index index, const void *val, uint32_t size);

int dhd_helper_tx_complete_host_send2dhd_read_ex(struct bdmf_object *mo,
     struct bdmf_attr *ad, bdmf_index index, void *val, uint32_t size);

int dhd_helper_dhd_stat_read_ex(struct bdmf_object *mo, struct bdmf_attr *ad,
    bdmf_index index, void *val, uint32_t size);
int dhd_helper_dhd_stat_write_ex(struct bdmf_object *mo, struct bdmf_attr *ad,
    bdmf_index index, const void *val, uint32_t size);

int dhd_helper_int_connect_write_ex(struct bdmf_object *mo,
    struct bdmf_attr *ad, bdmf_index index, const void *val, uint32_t size);

int rdpa_dhd_helper_dhd_complete_message_get_ex(rdpa_dhd_complete_data_t *dhd_complete_info);

int rdpa_dhd_helper_dhd_complete_ring_destroy_ex(uint32_t ring_idx, uint32_t ring_size);

int rdpa_dhd_helper_dhd_complete_ring_create_ex(uint32_t ring_idx, uint32_t ring_size);

int dhd_helper_attr_rx_post_init_write_ex(struct bdmf_object *mo, struct bdmf_attr *ad,
    bdmf_index index, const void *val, uint32_t size);

int dhd_helper_attr_rx_post_uninit_write_ex(struct bdmf_object *mo, struct bdmf_attr *ad,
    bdmf_index index, const void *val, uint32_t size);

int dhd_helper_attr_rx_post_reinit_write_ex(struct bdmf_object *mo, struct bdmf_attr *ad,
    bdmf_index index, const void *val, uint32_t size);
    
int dhd_helper_ssid_tx_dropped_packets_read_ex(struct bdmf_object *mo, struct bdmf_attr *ad,
    bdmf_index index, void *val, uint32_t size);
    
void rdpa_dhd_helper_wakeup_information_get_ex(rdpa_dhd_wakeup_info_t *wakeup_info);
    
void rdpa_dhd_helper_doorbell_interrupt_clear_ex(uint32_t radio_idx);

static inline void rdd_dhd_helper_packet_dump(void *data, uint32_t length, const rdpa_dhd_tx_post_info_t *info)
{
    if (likely(bdmf_global_trace_level < bdmf_trace_level_debug))
        return;
    bdmf_session_print(NULL, "SSID: %d, FlowRing ID: %d, Radio; %d\n", info->ssid_if_idx, info->flow_ring_id, info->radio_idx);
    bdmf_session_hexdump(NULL, data, 0, length);
}
   
int dhd_helper_port_obj_write_ex(struct bdmf_object *mo, struct bdmf_attr *ad,
    bdmf_index index, const void *val, uint32_t size);

void dhd_helper_cpu_exception_rxq_set_ex(dhd_helper_drv_priv_t *dhd_helper, uint8_t rdd_cpu_rqx);

#endif /* _RDPA_DHD_HELPER_EX_H_ */
