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


#ifndef _RDD_IPTV_H
#define _RDD_IPTV_H

#include "rdd.h"
#include "rdd_common.h"
#include "rdpa_types.h"
#include "rdpa_wlan_mcast.h"

typedef struct iptv_params
{
    uint32_t key_offset;
    uint32_t hash_tbl_idx;;
} iptv_params_t;

typedef struct
{
#if defined(BCM63158)
    RDD_IPTV_GPE_BASED_RESULT_DTS gpe;
    uint8_t replications;
#endif
    uint16_t wlan_mcast_index;
    uint64_t egress_port_vector;
    uint16_t ic_context;
    rdpa_iptv_channel_key_t key;
    rdpa_wlan_mcast_fwd_table_t wlan_mcast_fwd_table;
#define mc_key_vid key.vid
#define mc_key_mac key.mcast_group.mac
#define mc_key_gr_ip key.mcast_group.l3.gr_ip
#define mc_key_src_ip key.mcast_group.l3.src_ip
#define mc_wlan_idx wlan_mcast_index
#define mc_egress_port_vector egress_port_vector
#define mc_ic_context ic_context
#define mc_key_rx_if key.rx_if
} rdd_iptv_entry_t;

#define IPTV_DDR_CTX_TBL (RDD_IPTV_DDR_CONTEXT_TABLE_SIZE * sizeof(RDD_IPTV_DDR_CONTEXT_ENTRY_DTS))

#define IPTV_CHANNEL_INDEX_GET(key_idx, ctx_idx) ((key_idx << RDD_IPTV_DDR_CONTEXT_TABLE_LOG2_SIZE) | (ctx_idx & ((1 << RDD_IPTV_DDR_CONTEXT_TABLE_LOG2_SIZE) - 1)))
#define IPTV_KEY_INDEX_GET(channel_idx)          (channel_idx >> RDD_IPTV_DDR_CONTEXT_TABLE_LOG2_SIZE)
#define IPTV_CTX_INDEX_GET(channel_idx)          (channel_idx & ((1 << RDD_IPTV_DDR_CONTEXT_TABLE_LOG2_SIZE) - 1))

int rdd_iptv_module_init(const rdd_module_t *module);
void rdd_iptv_lkp_method_set(rdpa_iptv_lookup_method method);
int rdd_iptv_ctx_table_ddr_init(uint16_t fpm_min_pool_size);
void rdd_iptv_ctx_table_ddr_destroy(void);
void rdd_iptv_ddr_context_entry_get(RDD_IPTV_DDR_CONTEXT_ENTRY_DTS *ddr_ctx_entry, uint32_t entry_idx);

void rdd_iptv_hash_key_entry_compose(rdpa_iptv_channel_key_t *key, RDD_IPTV_HASH_LKP_ENTRY_DTS *hash_key_entry);
void rdd_iptv_hash_key_entry_decompose(RDD_IPTV_HASH_LKP_ENTRY_DTS hash_key_entry, rdpa_iptv_channel_key_t *key);
void rdd_iptv_hash_result_entry_compose(uint32_t ctx_idx, RDD_IPTV_HASH_RESULT_ENTRY_DTS *hash_result_entry);
void rdd_iptv_hash_result_entry_decompose(RDD_IPTV_HASH_RESULT_ENTRY_DTS hash_result_entry, uint32_t *ctx_idx);

/* TODO: move to Rdd_mcast_resolution*/
int rdd_iptv_result_entry_add(rdd_iptv_entry_t *iptv_entry, uint8_t *ic_ctx, uint32_t *head_idx, uint32_t ctr,
    uint32_t *entry_idx);
void rdd_iptv_result_entry_modify(rdd_iptv_entry_t *iptv_entry, uint8_t *ic_ctx, uint32_t entry_idx);
int rdd_iptv_result_entry_get(uint32_t ctx_idx, rdd_iptv_entry_t *rdd_iptv_entry, uint8_t *ic_ctx);
int rdd_iptv_result_entry_find(rdpa_iptv_channel_key_t *key, uint32_t head_idx, uint32_t *entry_idx);
int rdd_iptv_result_entry_delete(uint32_t entry_idx, uint32_t *head_idx, uint32_t *cntr_id);
int rdd_iptv_result_entry_next_idx_get(uint32_t entry_idx, uint32_t *next_idx);
int rdd_iptv_cntr_idx_get(uint32_t entry_idx, uint32_t *cntr_idx);
#endif /* _RDD_IPTV_H */
