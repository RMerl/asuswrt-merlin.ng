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

#ifndef _RDPA_XTM_EX_H
#define _RDPA_XTM_EX_H

#include "rdpa_xtm.h"

#define RDPA_XTM_FLOW_PACKET_OFFSET     0
#define RDPA_XTM_FLOW_BYTES_OFFSET      1
#define RDPA_XTM_FLOW_DISCARDS_OFFSET   2

/***************************************************************************
 * xtmflow object type
 **************************************************************************/
/* xtmflow object private data */
typedef struct {
    bdmf_index index;               /**< Gem index */
    uint16_t hdr_type;              /**< ATM/PTM L2 header type per WAN Flow) */
    uint16_t fstat;              /**< DMA buffer descriptor frame status word (external number) */
    rdpa_xtmflow_us_cfg_t us_cfg;  /**< xtmflow US configuration */
    int  ptm_bonding;
} xtmflow_drv_priv_t;

int rdpa_us_wan_flow_config(uint32_t      wan_flow,
                            int           wan_channel,
                            uint32_t      wan_hdr_type,
                            uint32_t      wan_port_or_fstat,
                            bdmf_boolean  crc_calc,
                            int           ptm_bonding,
                            uint8_t       pbits_to_queue_table_index,
                            uint8_t       traffic_class_to_queue_table_index,
                            bdmf_boolean  enable);


int rdpa_wan_dsl_channel_base(void);

int rdpa_flow_pm_counters_get(int index, rdpa_xtmflow_stat_t *stat);

int rdpa_xtm_xtmchannel_id_to_tm_channel_id(int xtmchannel_id);

int rdpa_xtm_txdrop_stats_get(rdpa_stat_t *tx_drop_stats);
int rdpa_xtmflow_attr_stat_write_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index,
    const void *val, uint32_t size);
#endif
