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

/*
 * rdpa_qos_mapping.c
 *
 * QoS mapping tables
 *
 *  Created on: Aug 17, 2012
 *      Author: igort
 */

#include <bdmf_dev.h>
#include <rdpa_api.h>
#include "rdpa_common.h"
#include "rdpa_int.h"
#include "rdd.h"
#include "rdpa_egress_tm_inline.h"
#include "rdpa_rdd_inline.h"
#include "rdpa_qos_mapper_ex.h"
#include "rdpa_dscp_to_pbit_ex.h"

#include "rdp_drv_ih.h"
#if !defined(LEGACY_RDP) && !defined(WL4908)
#include "rdd_multicast_processing.h"
#endif

static int set_switch_port_single_entry(struct bdmf_object *mo, bdmf_number port,
    bdmf_boolean set_to_rdd, bdmf_boolean link, bdmf_index index, bdmf_index queue,
    rdpa_pbit pbit, rdpa_qos_map_table_type table)
{
    rdd_bridge_port_t rdd_src_port;
    int rc_id = 0, prty = 0;
    int queue_id = 0;
    int rc = 0;

    rdd_src_port = rdpa_port_rdpa_if_to_vport(port);
    switch (table)
    {
    case rdpa_pbit_to_queue:
        if (rdpa_is_fttdp_mode())
            rdd_src_port = 0;
        if (link)
            rc = _rdpa_egress_tm_lan_port_queue_to_rdd(port, queue, &rc_id, &queue_id);
        if (set_to_rdd)
            rc = rc ? rc : rdd_ds_pbits_to_qos_entry_cfg(rdd_src_port, index, queue_id);

        BDMF_TRACE_DBG_OBJ(mo, "pbit_to_queue port %s pbit %d queue %d set to rdd %d\n",
            bdmf_attr_get_enum_text_hlp(&rdpa_if_enum_table, port), (int)index, (int)queue, set_to_rdd);
        break;
    case rdpa_tc_to_queue:
        if (rdpa_is_fttdp_mode())
            rdd_src_port = 0;
        if (link)
            rc = _rdpa_egress_tm_lan_port_queue_to_rdd(port, queue, &rc_id, &prty);
        if (set_to_rdd)
            rc = rc ? rc : rdd_ds_tc_to_queue_entry_cfg(rdd_src_port, (uint8_t)index, prty);

        BDMF_TRACE_DBG_OBJ(mo, "tc_to_queue port %s tc %d queue %d set to rdd %d\n",
            bdmf_attr_get_enum_text_hlp(&rdpa_if_enum_table, port), (int)index, (int)queue, set_to_rdd);
        break;
    default:
        return BDMF_ERR_NOT_SUPPORTED; /* we shouldn't get here */
    }
    return rc;
}

/* configure the switch port.
 * index is the first table parameter dscp/pbit/tc.
 * second table parameter is either queue or pbit */
int set_switch_port(struct bdmf_object *mo, bdmf_boolean set_to_rdd,
    bdmf_boolean is_link, bdmf_index index, bdmf_index queue, rdpa_pbit pbit, rdpa_qos_map_table_type table)
{
    rdpa_ports lag_ports = rdpa_get_switch_lag_port_mask();
    int port = 0, rc = 0;

    /* FTTdp we configure a fixed port which is expected by FW (set in set_switch_port_single_entry) */
    if (rdpa_is_fttdp_mode())
        return set_switch_port_single_entry(mo, port, set_to_rdd, is_link,  index,  queue, pbit, table);

    for (port = rdpa_if_lag0; port < rdpa_if_lag4; port++)
    {
        if (rdpa_if_id(port) & lag_ports)
        {
            rc = set_switch_port_single_entry(mo, port, set_to_rdd, is_link,  index,  queue, pbit, table);
            if (rc)
                return rc;
        }
    }

    return 0;
}

/*
 * pbit_to_pbit attribute access
 */
void us_wan_flow_rdd_cfg(struct bdmf_object *mo, bdmf_object_handle gem, bdmf_number channel_idx, int tbl_idx,
    uint8_t tc_table)
{
    bdmf_number gem_index;
    bdmf_number gem_port;
    rdpa_gem_flow_type flow_type;

    rdpa_gem_index_get(gem, &gem_index);
    rdpa_gem_gem_port_get(gem, &gem_port);
    rdpa_gem_flow_type_get(gem, &flow_type);

    BDMF_TRACE_DBG_OBJ(mo, "Configure GEN index %d port %d tcont/port %d. "
        "pbit_table %d tc_table %d in RDD\n", (int)gem_index, (int)gem_port,
        (int)channel_idx, tbl_idx, (int)tbl_idx);

    rdd_us_wan_flow_cfg(gem_index, channel_idx, 0,
        gem_port, flow_type == rdpa_gem_flow_ethernet ? 1 : 0, 0, tbl_idx, tc_table);
}

int rdpa_qos_to_queue_set_single_entry_ex(struct bdmf_object *mo, bdmf_boolean set_to_rdd,
    bdmf_object_handle obj, bdmf_index qos, bdmf_index queue_id, bdmf_boolean link,
    f_rdd_qos_to_queue_set_t f_rdd_qos_to_queue_set, int tbl_idx) 
{
    return BDMF_ERR_OK; /* not supported*/
}

void rdpa_qos_to_queue_unlink_other_ex(struct bdmf_object *other, uint8_t tbl_size, bdmf_boolean is_pbit)
{
    return; /* not supported*/
}
