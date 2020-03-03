/* 
* <:copyright-BRCM:2013-2015:proprietary:standard
* 
*    Copyright (c) 2013-2015 Broadcom 
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

#ifndef _RDPA_MCAST_EX_H_
#define _RDPA_MCAST_EX_H_

#ifdef XRDP

#define RDD_MCAST_MAX_PORT_CONTEXT_ENTRIES RDD_IPTV_DDR_PORT_BUFFER_TABLE_SIZE

#define rdd_mcast_port_context_t RDD_IPTV_PORT_CONTEXT_ENTRY_DTS
#define rdd_mcast_port_header_buffer_t RDD_IPTV_DDR_PORT_BUFFER_TABLE_DTS

typedef struct {
    uint32_t flow_hits;
    uint32_t flow_bytes;
    uint32_t multicast_flag;
    uint32_t is_routed;
    uint32_t mtu;
    uint32_t is_tos_mangle;
    uint32_t tos;
    uint32_t number_of_ports;
    uint32_t port_mask;
    uint32_t wlan_mcast_clients;
    uint32_t wlan_mcast_index;
    uint64_t mcast_port_header_buffer_ptr;
    uint32_t command_list_length_64;
    union {
        rdd_mcast_port_context_t port_context[RDD_MCAST_MAX_PORT_CONTEXT_ENTRIES];
        uint32_t port_context_u32[RDD_MCAST_MAX_PORT_CONTEXT_ENTRIES];
    };
    uint8_t l3_command_list[RDPA_CMD_LIST_MCAST_L3_LIST_SIZE];
} rdd_mcast_flow_context_t;

typedef struct {
    rdpa_mcast_flow_key_t key;
    rdd_mcast_flow_context_t context;
} rdd_mcast_flow_t;

#define RDD_MCAST_FLOW_CONTEXT_GET_EX(rdd_mcast_flow)  \
    (&(rdd_mcast_flow)->context)

#else /* RDP */

#define RDD_MCAST_MAX_PORT_CONTEXT_ENTRIES RDD_FC_MCAST_FLOW_CONTEXT_ENTRY_PORT_CONTEXT_NUMBER

#define rdd_mcast_port_context_t RDD_FC_MCAST_PORT_CONTEXT_ENTRY_DTS
#define rdd_mcast_port_header_buffer_t RDD_FC_MCAST_PORT_HEADER_BUFFER_DTS
#define rdd_mcast_flow_context_t RDD_FC_MCAST_FLOW_CONTEXT_ENTRY_DTS

#define RDD_MCAST_FLOW_CONTEXT_GET_EX(rdd_mcast_flow)                   \
    (&(rdd_mcast_flow)->context.fc_mcast_flow_context_entry)

#endif /* RDP */

int rdpa_mcast_pre_init_ex(void);
int rdpa_mcast_post_init_ex(void);
void rdpa_mcast_destroy_ex(void);
int rdpa_mcast_rdpa_if_to_rdd_vport_ex(rdpa_if rdpa_port, rdd_vport_id_t *rdd_vport);
int rdpa_mcast_rdd_vport_to_rdpa_if_ex(rdd_vport_id_t rdd_vport, rdpa_if *rdpa_port);
int rdpa_mcast_rdd_context_get_ex(bdmf_index index, rdd_mcast_flow_t *rdd_mcast_flow);
int rdpa_mcast_rdd_context_modify_ex(bdmf_index index, rdd_mcast_flow_t *rdd_mcast_flow);
int rdpa_mcast_rdd_key_get_ex(bdmf_index index, rdpa_mcast_flow_t *rdpa_mcast_flow,
                              rdd_mcast_flow_t *rdd_mcast_flow);
void rdpa_mcast_rdd_key_create_ex(rdpa_mcast_flow_t *rdpa_mcast_flow, rdd_mcast_flow_t *rdd_mcast_flow);
int rdpa_mcast_rdd_flow_add_ex(bdmf_index *index, rdd_mcast_flow_t *rdd_mcast_flow);
int rdpa_mcast_rdd_flow_delete_ex(bdmf_index index);
int rdpa_mcast_rdd_flow_find_ex(bdmf_index *index, rdd_mcast_flow_t *rdd_mcast_flow);
int rdpa_mcast_rdd_flow_stats_get_ex(bdmf_index index, rdd_mcast_flow_t *rdd_mcast_flow);
int rdpa_mcast_rdd_port_header_buffer_get_ex(rdd_vport_id_t rdd_vport,
                                             rdd_mcast_port_header_buffer_t *rdd_port_header_buffer,
                                             rdpa_mcast_port_context_t *rdpa_port_context,
                                             rdd_mcast_port_context_t *rdd_port_context);
int rdpa_mcast_rdd_port_header_buffer_set_ex(rdd_vport_id_t rdd_vport,
                                             rdpa_mcast_port_context_t *rdpa_port_context,
                                             rdd_mcast_port_header_buffer_t *rdd_port_header_buffer,
                                             rdd_mcast_port_context_t *rdd_port_context);
#endif /* _RDPA_MCAST_EX_H_ */
