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


#ifndef _RDD_COMPLEX_SCHEDULER_H
#define _RDD_COMPLEX_SCHEDULER_H

#include "rdd.h"

#define COMPLEX_SCHEDULER

#define COMPLEX_SCHEDULER_NUM_OF_QUEUES       32
#define COMPLEX_SCHEDULER_FULL_BUDGET_VECTOR  0xffffffff

typedef enum
{
    complex_scheduler_full_dwrr = 0,
    complex_scheduler_2sp_30dwrr = 1,
    complex_scheduler_4sp_28dwrr = 2,
    complex_scheduler_8sp_24dwrr = 3,
    complex_scheduler_16sp_16dwrr = 4,
    complex_scheduler_full_sp = 5,
    complex_scheduler_num_of_dwrr_offset
} complex_scheduler_dwrr_offset_t;

typedef enum
{
    complex_scheduler_block_queue = 0,
    complex_scheduler_block_bs = 1,
    complex_scheduler_block_cs = 2,
    complex_scheduler_num_of_block_type
} complex_scheduler_block_type_t;

typedef struct
{
    complex_scheduler_dwrr_offset_t dwrr_offset_sir;
    complex_scheduler_dwrr_offset_t dwrr_offset_pir;
    uint8_t bbh_queue_index;
    uint8_t hw_bbh_qid;
    bdmf_boolean parent_exists;
} complex_scheduler_cfg_t;
typedef struct
{
    uint8_t block_index; /* queue or basic scheduler index */
    uint8_t scheduler_slot_index; /* internal index for complex scheduler */
    uint8_t bs_dwrr_offset;
    quantum_number_t quantum_number;
    complex_scheduler_block_type_t block_type;
} complex_scheduler_block_t;

/* API to RDPA level */
bdmf_error_t rdd_complex_scheduler_cfg(rdpa_traffic_dir dir, uint8_t complex_scheduler_index, complex_scheduler_cfg_t *cfg);
bdmf_error_t rdd_complex_scheduler_block_cfg(rdpa_traffic_dir dir, uint8_t complex_scheduler_index, complex_scheduler_block_t *block);
bdmf_error_t rdd_complex_scheduler_block_remove(rdpa_traffic_dir dir, uint8_t complex_scheduler_index, uint8_t scheduler_slot_index);

/* API to block */
bdmf_error_t rdd_complex_scheduler_rate_set(rdpa_traffic_dir dir, uint8_t complex_scheduler_index, uint32_t block_bit_mask);

#endif
