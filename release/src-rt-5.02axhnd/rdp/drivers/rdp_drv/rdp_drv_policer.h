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

#ifndef _RDP_DRV_POLICER_H_
#define _RDP_DRV_POLICER_H_

#include "rdp_drv_cnpl.h"
#include "rdpa_policer.h"

typedef struct {
    uint8_t index;                    /* policer index */
    uint8_t is_dual;                  /* 0 - single bucket ; 1 - dual bucket */
    uint64_t commited_rate;           /* Committed Information Rate (CIR) - bps */
    uint64_t peak_rate;               /* PEAK Information Rate (PIR) - bps */
    uint64_t committed_burst_size;    /* Committed Burst Size (CBS) - bytes */
    uint64_t peak_burst_size;         /* PEAK Burst Size (PBS) - bytes */
    uint8_t overflow;                 /* dual bucket mode */
    uint8_t dei_mode;                 /* 0 - disable ; 1 - enable */
} policer_cfg_t;

/* Policer factor to be added for every packet */
typedef enum {
    rdp_policer_factor_bytes_neg_8,   /* subtract 8B to policing */
    rdp_policer_factor_bytes_neg_4,   /* subtract 4B to policing */
    rdp_policer_factor_bytes_0,       /* don't add bytes to policing */
    rdp_policer_factor_bytes_4,       /* add 4B to policing */
    rdp_policer_factor_bytes_8,       /* add 8B to policing */
    rdp_policer_factor_num_of,        /* Number of possible types */
} rdp_policer_factor_bytes;

bdmf_error_t drv_policer_group_init(void);
bdmf_error_t drv_cnpl_policer_set(policer_cfg_t* policer_cfg);
bdmf_error_t drv_cnpl_policer_clr(policer_cfg_t* policer_cfg);

static inline void drv_cnpl_policer_max_cbs_get(policer_rate_size_t commited_rate, uint32_t* max_committed_burst_byte_size)
{
    *max_committed_burst_byte_size = (((commited_rate / POLICER_TIMER_PERIOD) * BUCKET_SIZE_RATE_MULT_MAX) / 8);
}

static inline void drv_cnpl_policer_min_cbs_get(policer_rate_size_t  commited_rate, uint32_t* min_committed_burst_byte_size)
{
    *min_committed_burst_byte_size = (commited_rate / POLICER_TIMER_PERIOD) / 8;
}

#endif
