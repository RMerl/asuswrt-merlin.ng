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

#ifndef _RDD_NATC_H
#define _RDD_NATC_H

#include "rdd.h"


void rdd_natc_tbl_cfg(uint8_t tbl_idx, uint32_t key_addr_hi, uint32_t key_addr_lo, uint32_t res_addr_hi, uint32_t res_addr_lo);

void rdd_natc_l2_fc_enable(int enable);
int rdd_natc_l2_fc_get(void);

typedef union {
    struct {
#if defined(_BYTE_ORDER_LITTLE_ENDIAN_)
        uint32_t natc_idx:20;
        uint32_t tbl_idx:4;
        uint32_t rdpa_use_only:8;
#else
        uint32_t rdpa_use_only:8;
        uint32_t tbl_idx:4;
        uint32_t natc_idx:20;
#endif
    };
    uint32_t word;
} natc_flow_id_t;

#define natc_tbls_num  (NATC_TBL_IDX_LAST+1)

static inline rdpa_traffic_dir rdd_natc_tbl_idx_to_dir(rdd_natc_tbl_idx tbl_idx)
{
#ifndef G9991
    return (tbl_idx == NATC_TBL_IDX_DS ? rdpa_dir_ds : rdpa_dir_us);
#else
    /* For G9991 - always downstream */
    return rdpa_dir_ds;
#endif
}

static inline rdd_natc_tbl_idx rdd_natc_l2l3_ucast_dir_to_tbl_idx(rdpa_traffic_dir dir)
{
#ifndef G9991
    return (dir == rdpa_dir_ds ? NATC_TBL_IDX_DS : NATC_TBL_IDX_US);
#else
    /* For G9991 - always downstream */
    return NATC_TBL_IDX_DS;
#endif
}

static inline uint32_t build_rdd_flow_id_for_natc(uint32_t natc_idx, rdd_natc_tbl_idx tbl_idx)
{
    natc_flow_id_t flow_id = {.word = 0};
    flow_id.tbl_idx = tbl_idx;
    flow_id.natc_idx = natc_idx;
    return flow_id.word;
}

#endif /* _RDD_NATC_H */
