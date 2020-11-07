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


#include "rdp_subsystem_common.h"
#include "rdp_common.h"
#include "rdd_runner_proj_defs.h"
#include "rdd_defs.h"
#include "rdp_drv_natc.h"
#include "rdp_drv_natc_counters.h"
#include "data_path_init.h"


int drv_natc_counters_get_counter_values(uint32_t sub_table_id, uint32_t counter_key, uint64_t *packet, uint64_t *bytes)
{
    uint32_t entry_index;
    uint32_t hash_index;
    RDD_NAT_CACHE_COUNTER_LKP_ENTRY_DTS key_entry;
    key_entry.counter_key = counter_key;
    key_entry.valid = 1;
    key_entry.reserved0 = 0;
    key_entry.reserved1 = 0;
    key_entry.reserved2 = 0;
    key_entry.reserved3 = 0;
    key_entry.sub_table_id = sub_table_id;

#ifdef FIRMWARE_LITTLE_ENDIAN
    SWAPBYTES(((uint8_t *)&key_entry), sizeof(RDD_NAT_CACHE_L2_LKP_ENTRY_DTS));
#endif

    drv_natc_key_idx_get(NATC_TBL_IDX_COUNTERS, (uint8_t*)&key_entry, &hash_index, &entry_index);
    return drv_natc_entry_counters_get(NATC_TBL_IDX_COUNTERS, entry_index, packet, bytes);
}

int drv_natc_counters_add_new_counter(uint32_t sub_table_id, uint32_t *entry_index, uint32_t counter_key)
{
    int rc;
    uint8_t context[64];
    RDD_NAT_CACHE_COUNTER_LKP_ENTRY_DTS key_entry;
    uint8_t keyword[NATC_MAX_ENTRY_LEN] = {};

    memset(context, 0, 64);
    key_entry.valid = 1;
    key_entry.counter_key = counter_key;
    key_entry.reserved0 = 0;
    key_entry.reserved1 = 0;
    key_entry.reserved2 = 0;
    key_entry.reserved3 = 0;
    key_entry.sub_table_id = sub_table_id;

#ifdef FIRMWARE_LITTLE_ENDIAN
    SWAPBYTES(((uint8_t *)&key_entry), sizeof(RDD_NAT_CACHE_COUNTER_LKP_ENTRY_DTS));
#endif

    memcpy(keyword, (uint8_t *)&key_entry, sizeof(RDD_NAT_CACHE_COUNTER_LKP_ENTRY_DTS));
    rc = drv_natc_key_result_entry_add(NATC_TBL_IDX_COUNTERS, keyword, (uint8_t *)&context[0], entry_index);
    BDMF_TRACE_DBG("drv_natc_counters_add_new_counter return value = %d entry index is %d\n", rc, (int)*entry_index);
    return rc;
}

int drv_natc_counters_remove_counter(uint32_t sub_table_id, uint32_t counter_key)
{
    int rc;
    uint32_t entry_index;
    uint32_t hash_index;
    RDD_NAT_CACHE_COUNTER_LKP_ENTRY_DTS key_entry;
    key_entry.counter_key = counter_key;
    key_entry.valid = 1;
    key_entry.reserved0 = 0;
    key_entry.reserved1 = 0;
    key_entry.reserved2 = 0;
    key_entry.reserved3 = 0;
    key_entry.sub_table_id = sub_table_id;

#ifdef FIRMWARE_LITTLE_ENDIAN
    SWAPBYTES(((uint8_t *)&key_entry), sizeof(RDD_NAT_CACHE_L2_LKP_ENTRY_DTS));
#endif
    drv_natc_key_idx_get(NATC_TBL_IDX_COUNTERS, (uint8_t*)&key_entry, &hash_index, &entry_index);
    rc = drv_natc_entry_delete(NATC_TBL_IDX_COUNTERS, entry_index, 1, 1);
    return rc;
}

