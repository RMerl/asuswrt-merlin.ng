/*
 * <:copyright-BRCM:2017:proprietary:standard
 * 
 *    Copyright (c) 2017 Broadcom 
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
#include <rdd.h>
#include <rdd_ih_defs.h>
#include <rdpa_api.h>
#include "rdpa_int.h"
#include "rdpa_rdd_inline.h"
#include "rdpa_mcast_ex.h"
#include "rdpa_mcast.h"
#include "rdpa_port_int.h"

#if (RDPA_CMD_LIST_MCAST_L2_LIST_SIZE != RDD_FC_MCAST_PORT_HEADER_BUFFER_SIZE2)
#error "RDPA_CMD_LIST_MCAST_L2_LIST_SIZE != RDD_FC_MCAST_PORT_HEADER_BUFFER_SIZE2"
#endif

#if (RDPA_CMD_LIST_MCAST_L3_LIST_SIZE != RDD_FC_MCAST_FLOW_CONTEXT_ENTRY_L3_COMMAND_LIST_NUMBER)
#error "RDPA_CMD_LIST_MCAST_L3_LIST_SIZE != RDD_FC_MCAST_FLOW_CONTEXT_ENTRY_L3_COMMAND_LIST_NUMBER"
#endif

int rdpa_mcast_pre_init_ex(void)
{
    return BDMF_ERR_OK;
}

int rdpa_mcast_post_init_ex(void)
{
    return BDMF_ERR_OK;
}

void rdpa_mcast_destroy_ex(void)
{
}

int rdpa_mcast_rdpa_if_to_rdd_vport_ex(rdpa_if rdpa_port, rdd_vport_id_t *rdd_vport)
{
    rdd_emac_id_t *rdd_emac = (rdd_emac_id_t *)rdd_vport;
    uint8_t wifi_ssid;

    if (rdpa_if_to_rdd_lan_mac(rdpa_port, rdd_emac, &wifi_ssid) == 0)
    {
        if (*rdd_emac >= RDD_EMAC_ID_0 && *rdd_emac <= RDD_EMAC_ID_7)
        {
            *rdd_emac -= 1; /* Notice the -1 here */

            return 0;
        }
    }

    return -1;
}

int rdpa_mcast_rdd_vport_to_rdpa_if_ex(rdd_vport_id_t rdd_vport, rdpa_if *rdpa_port)
{
    *rdpa_port = rdd_lan_mac_to_rdpa_if((rdd_emac_id_t)(rdd_vport+1)); /* Notice the +1 here */

    if (*rdpa_port == rdpa_if_none)
    {
        return -1;
    }

    return 0;
}

int rdpa_mcast_rdd_context_get_ex(bdmf_index index, rdd_mcast_flow_t *rdd_mcast_flow)
{
    rdd_mcast_flow_context_t *rdd_mcast_context =
        RDD_MCAST_FLOW_CONTEXT_GET_EX(rdd_mcast_flow);
    int rc;
    uint32_t rdd_flow_id = index;

    rc = rdd_context_entry_get(rdd_flow_id, &rdd_mcast_flow->context);
    if (rc)
    {
        return BDMF_ERR_NOENT;
    }

    if (!rdd_mcast_context->multicast_flag)
    {
        return BDMF_ERR_NOENT;
    }

    return rc;
}

int rdpa_mcast_rdd_context_modify_ex(bdmf_index index, rdd_mcast_flow_t *rdd_mcast_flow)
{
    int rc;
    uint32_t rdd_flow_id = index;

    rc = rdd_context_entry_modify(&rdd_mcast_flow->context, rdd_flow_id);
    if (rc)
    {
        BDMF_TRACE_ERR("Multicast flow modification failed, error %d\n", rc);

        return BDMF_ERR_INVALID_OP;
    }

    return rc;
}

int rdpa_mcast_rdd_key_get_ex(bdmf_index index, rdpa_mcast_flow_t *rdpa_mcast_flow,
                              rdd_mcast_flow_t *rdd_mcast_flow)
{
    rdd_mcast_flow_context_t *rdd_mcast_context =
        RDD_MCAST_FLOW_CONTEXT_GET_EX(rdd_mcast_flow);
    int rc;
    uint32_t rdd_flow_id = index;

    /* Get the connection, so we can retrieve the Key */
    rc = rdd_fc_mcast_connection_entry_get(rdd_mcast_context->connection_table_index, rdd_flow_id,
                                           &rdpa_mcast_flow->key);
    if (rc)
    {
        return BDMF_ERR_NOENT;
    }

    rdpa_mcast_flow->key.rx_if = rdpa_port_vport_to_rdpa_if((rdd_vport_id_t)rdpa_mcast_flow->key.rx_if);

    return rc;
}

void rdpa_mcast_rdd_key_create_ex(rdpa_mcast_flow_t *rdpa_mcast_flow, rdd_mcast_flow_t *rdd_mcast_flow)
{
    memcpy(&rdd_mcast_flow->key, &rdpa_mcast_flow->key, sizeof(rdpa_mcast_flow_key_t));

    /* change rx_if to src_bridge_port */
    rdd_mcast_flow->key.rx_if = rdpa_port_rdpa_if_to_vport(rdpa_mcast_flow->key.rx_if);
}

int rdpa_mcast_rdd_flow_add_ex(bdmf_index *index, rdd_mcast_flow_t *rdd_mcast_flow)
{
    int rc;

    rc = rdd_fc_mcast_connection_entry_add(rdd_mcast_flow);
    if (rc)
    {
#ifdef WL4908
        if (rc != BDMF_ERR_IGNORE) 
        {
            /* Do not print errors for hash collisions */
            BDMF_TRACE_ERR("Multicast flow could not be added, error=%d\n", rc);
        }
        return rc;
#else
        if (rc == BL_LILAC_RDD_ERROR_ADD_LOOKUP_NO_EMPTY_ENTRY) 
        {
            /* Ignore hash collisions and do not print errors */
            return BDMF_ERR_IGNORE;
        } 

        BDMF_TRACE_ERR("Multicast flow could not be added, error=%d\n", rc);

        if (rc == BL_LILAC_RDD_ERROR_LOOKUP_ENTRY_EXISTS) 
        {
            return BDMF_ERR_ALREADY; 
        }
        else if (rc == BL_LILAC_RDD_ERROR_ADD_LOOKUP_ENTRY ||
                rc == BL_LILAC_RDD_ERROR_ADD_CONTEXT_ENTRY)
        {
            return BDMF_ERR_NORES;
        }
        else
        {
            return BDMF_ERR_INTERNAL;
        }
#endif
    }

    /* set the created flow index, to return */
    *index = rdd_mcast_flow->xo_entry_index;

    return rc;
}

int rdpa_mcast_rdd_flow_delete_ex(bdmf_index index)
{
    int rc;

    rc = rdd_fc_mcast_connection_entry_delete(index);
    if (rc)
    {
        BDMF_TRACE_ERR("Multicast flow deletion failed, error=%d\n", rc);

        return BDMF_ERR_INTERNAL;
    }

    return rc;
}

int rdpa_mcast_rdd_flow_find_ex(bdmf_index *index, rdd_mcast_flow_t *rdd_mcast_flow)
{
    int rc;

    rc = rdd_fc_mcast_connection_entry_search(rdd_mcast_flow, index);
    if (rc)
    {
        return BDMF_ERR_NOENT;
    }

    return rc;
}

int rdpa_mcast_rdd_flow_stats_get_ex(bdmf_index index, rdd_mcast_flow_t *rdd_mcast_flow)
{
    rdd_mcast_flow_context_t *rdd_mcast_context =
        RDD_MCAST_FLOW_CONTEXT_GET_EX(rdd_mcast_flow);
    int rc;

    rc = rdd_context_entry_flwstat_get(index, &rdd_mcast_flow->context);
    if (rc)
    {
        return BDMF_ERR_NOENT;
    }

    if (!rdd_mcast_context->multicast_flag)
    {
        return BDMF_ERR_NOENT;
    }

    return rc;
}

int rdpa_mcast_rdd_port_header_buffer_get_ex(rdd_vport_id_t rdd_vport,
                                             rdd_mcast_port_header_buffer_t *rdd_port_header_buffer,
                                             rdpa_mcast_port_context_t *rdpa_port_context,
                                             rdd_mcast_port_context_t *rdd_port_context)
{
    return rdd_fc_mcast_port_header_buffer_get(rdd_vport,
                                               rdd_port_header_buffer,
                                               rdpa_port_context->port_header.l2_header);
}

int rdpa_mcast_rdd_port_header_buffer_set_ex(rdd_vport_id_t rdd_vport,
                                             rdpa_mcast_port_context_t *rdpa_port_context,
                                             rdd_mcast_port_header_buffer_t *rdd_port_header_buffer,
                                             rdd_mcast_port_context_t *rdd_port_context)
{
    return rdd_fc_mcast_port_header_buffer_put(rdd_vport,
                                               rdpa_port_context->port_header.l2_header,
                                               rdd_port_header_buffer);
}
