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

#include <bdmf_dev.h>
#include <rdpa_api.h>
#include "rdpa_common.h"
#include "rdd.h"
#include "rdpa_int.h"
#include "rdpa_egress_tm_inline.h"
#include "rdpa_rdd_inline.h"
#include "rdd_qos_mapper.h"
#include "rdpa_tc_to_queue_ex.h"
#include "rdpa_tcont_ex.h"
#include "rdpa_egress_tm_ex.h"
#include "rdpa_qos_mapper_ex.h"

bdmf_object_handle ds_tc_to_queue_objects[RDPA_TC_TO_QUEUE_ID_MAX_TABLES / 2];
bdmf_object_handle us_tc_to_queue_objects[RDPA_TC_TO_QUEUE_ID_MAX_TABLES / 2];

void rdpa_tc_to_queue_obj_init_ex(bdmf_object_handle **container, int *max_tables, uint8_t *table_size, rdpa_traffic_dir dir)
{
    if (!*table_size)
        *table_size = RDPA_BS_TC_TO_QUEUE_TABLE_SIZE;
    *max_tables = RDPA_TC_TO_QUEUE_ID_MAX_TABLES / 2;
    *container = (dir == rdpa_dir_ds) ? ds_tc_to_queue_objects : us_tc_to_queue_objects;
}

int rdpa_tc_to_queue_set_single_entry_ex(struct bdmf_object *mo, bdmf_boolean set_to_rdd,
    bdmf_object_handle other, bdmf_index tc, bdmf_index queue, bdmf_boolean link)
{
    tc_to_queue_drv_priv_t *tbl = (tc_to_queue_drv_priv_t *)bdmf_obj_data(mo);
    int rc = 0;

    rc = rdpa_qos_to_queue_set_single_entry_ex(mo, set_to_rdd, other, tc, queue, link,
        &rdd_tc_to_queue_entry_set, (int)tbl->index);

    return rc;
}

void rdpa_tc_to_queue_unlink_port_ex(struct bdmf_object *mo, struct bdmf_object *other, bdmf_number port)
{
    rdpa_qos_to_queue_unlink_other_ex(other, RDPA_BS_TC_TO_QUEUE_TABLE_SIZE, 0);
}

bdmf_error_t rdpa_tc_to_queue_realloc_table_ex(int *tc_to_queue_linked_tcont_llid, tc_to_queue_drv_priv_t *tbl)
{
    /* check if table size should be increased , if so - allocate new, larger tables for all TCONTS/LLIDS */
    /* TODO : move to a function in ex files */
    /* TODO - for each TCONT - find wan_flow (gem) */
#if 0
    if (index >= tbl->size)
    {
        for (inst = 0; inst < RDPA_MAX_TCONT; inst++)
        {
            if (tc_to_queue_linked_tcont_llid[inst] == tbl->index)
            {
                rc = rdd_realloc_tc_to_queue_table(tbl->index, inst, tbl->dir, &(tbl->size));
                if (rc)
                    return rc;
            }
        }
    }
#endif
    return BDMF_ERR_OK;
}


