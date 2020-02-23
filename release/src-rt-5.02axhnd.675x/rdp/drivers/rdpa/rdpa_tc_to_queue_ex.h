/*
 * <:copyright-BRCM:2015:proprietary:standard
 * 
 *    Copyright (c) 2015 Broadcom 
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

#ifndef _RDPA_TC_TO_QUEUE_EX_H_
#define _RDPA_TC_TO_QUEUE_EX_H_

#include "rdpa_egress_tm_inline.h"
#include "rdpa_rdd_inline.h"
#ifdef XRDP
#include "rdd_qos_mapper.h"
#endif

#ifndef XRDP
#define RDPA_US_TC_TO_QUEUE_ID_TABLE_SIZE 32
#define RDPA_DS_TC_TO_QUEUE_ID_TABLE_SIZE 8
#endif
#define RDPA_CS_TC_TO_QUEUE_TABLE_SIZE 32
#define RDPA_BS_TC_TO_QUEUE_TABLE_SIZE 8

typedef struct tc_queue
{
    DLIST_ENTRY(tc_queue) list;
    bdmf_index tc;
    bdmf_index queue;
} tc_to_queue_t;

DLIST_HEAD(tc_to_queue_list, tc_queue);

/* tc_to_queue object private data */
typedef struct {
    bdmf_index index;
    rdpa_traffic_dir dir;
    struct tc_to_queue_list mapping_list;  /**< Holds list of mapping tc to queue */
    bdmf_index entries;  /**< Holds number of list entries */
    uint8_t size; /**< Holds the maximal size of the table */
} tc_to_queue_drv_priv_t;


int rdpa_tc_to_queue_set_single_entry_ex(struct bdmf_object *mo, bdmf_boolean set_to_rdd,
    bdmf_object_handle port_obj, bdmf_index tc, bdmf_index queue, bdmf_boolean link);
void rdpa_tc_to_queue_obj_init_ex(bdmf_object_handle **container, int *max_tables, uint8_t *table_size, rdpa_traffic_dir dir);
bdmf_error_t rdpa_tc_to_queue_realloc_table_ex(int *tc_to_queue_linked_tcont_llid, tc_to_queue_drv_priv_t *tbl);
void rdpa_tc_to_queue_unlink_port_ex(struct bdmf_object *mo, struct bdmf_object *other, bdmf_number port);

#endif
