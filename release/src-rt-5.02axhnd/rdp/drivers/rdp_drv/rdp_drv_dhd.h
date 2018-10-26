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

#ifndef _RDP_DRV_DHD_H_
#define _RDP_DRV_DHD_H_

#include "rdd_dhd_helper.h"


void rdp_drv_dhd_skb_fifo_tbl_init(void);
int rdp_drv_dhd_helper_flow_ring_flush(uint32_t radio_idx, uint32_t flow_ring_idx);
int rdp_drv_dhd_helper_flow_ring_disable(uint32_t radio_idx, uint32_t flow_ring_idx);
void rdp_drv_dhd_helper_wakeup_information_get(rdpa_dhd_wakeup_info_t *wakeup_info);
int rdp_drv_dhd_helper_dhd_complete_ring_create(uint32_t radio_idx, uint32_t ring_size);
int rdp_drv_dhd_helper_dhd_complete_ring_destroy(uint32_t radio_idx, uint32_t ring_size);
uint32_t rdp_drv_dhd_helper_ssid_tx_dropped_packets_get(uint32_t radio_idx, uint32_t ssid);
int rdp_drv_dhd_cpu_tx(const rdpa_dhd_tx_post_info_t *info, void *buffer, uint32_t  pkt_length);
int rdp_drv_dhd_helper_dhd_complete_message_get(rdpa_dhd_complete_data_t *dhd_complete_info);
int rdp_drv_rx_post_init(uint32_t radio_idx, rdpa_dhd_init_cfg_t *init_cfg, uint32_t num_items);
int rdp_drv_dhd_rx_post_uninit(uint32_t radio_idx, rdpa_dhd_init_cfg_t *init_cfg, uint32_t *num_items);
int rdp_drv_dhd_rx_post_reinit(uint32_t radio_idx, rdpa_dhd_init_cfg_t *init_cfg);

#endif
