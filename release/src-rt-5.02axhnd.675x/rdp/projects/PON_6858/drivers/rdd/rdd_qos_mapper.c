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


#include "rdd.h"
#include "rdd_common.h"
#include "rdd_qos_mapper.h"
#include "rdpa_qos_mapper.h"
#include "rdd_defs.h"
#include "rdp_drv_qm.h"

#if !defined(_CFE_)
#include "rdpa_tcont.h"
#else
#define RDPA_MAX_TCONT 32
#endif

#define QOS_MAPPER_TABLE_UNASSIGNED RDPA_TC_TO_QUEUE_ID_MAX_TABLES
#define RDD_QOS_MAPPER_INDEX_UNASSIGNED RDD_QOS_MAPPER_ID_MAX_TABLES
#define RDD_QOS_MAPPER_TABLE_FREE 0
#define RDD_QOS_MAPPER_TABLE_ALLOCATED 1
#define QM_QUEUE_TO_BYTE_QUEUE(qm_queue) ( qm_queue < drv_qm_get_ds_start() ? qm_queue : (qm_queue - drv_qm_get_ds_start()))

typedef struct
{
    bdmf_boolean tc2q_linked;
    bdmf_boolean pbit2q_linked;
    uint8_t table_index;
} qos_mapper_manager_t;

static qos_mapper_manager_t qos_mapper_tables_manager[RDD_TX_FLOW_TABLE_SIZE] = {};
static int32_t qos_table_index_reference_count[RDD_QOS_MAPPER_ID_MAX_TABLES];
static uint8_t g_tcont_ptr_to_rdd_table_index_mapping[RDPA_MAX_TCONT] = {};

int g_drop_q;

static void print_buf_dbg( void )
{
    int i;

    BDMF_TRACE_DBG("___       dbg print_____________\n");
    for (i = 0; i < RDPA_MAX_TCONT; i++)
    {
        if (g_tcont_ptr_to_rdd_table_index_mapping[i] != RDD_QOS_MAPPER_INDEX_UNASSIGNED)
            BDMF_TRACE_DBG("tcont_mapping[%d] = %d,", i, g_tcont_ptr_to_rdd_table_index_mapping[i]);
    }
    BDMF_TRACE_DBG("\n");
    for (i = 0; i < RDD_TX_FLOW_TABLE_SIZE; i++)
    {
        if (qos_mapper_tables_manager[i].tc2q_linked != 0 || qos_mapper_tables_manager[i].pbit2q_linked != 0 ||
            qos_mapper_tables_manager[i].table_index != RDD_QOS_MAPPER_INDEX_UNASSIGNED)
            BDMF_TRACE_DBG("tables_manager[%d] = %d%d, %d", i, qos_mapper_tables_manager[i].tc2q_linked, qos_mapper_tables_manager[i].pbit2q_linked,
                qos_mapper_tables_manager[i].table_index);
    }
    BDMF_TRACE_DBG("\n");
    for (i = 0; i < RDD_QOS_MAPPER_ID_MAX_TABLES; i++)
    {
        if (qos_table_index_reference_count[i])
            RDD_BTRACE("reference_count[%d] = %d,", i, qos_table_index_reference_count[i]);
    }
    BDMF_TRACE_DBG("\n");
}

static void _clean_tcont_ptr_to_rdd_table(uint8_t rdd_index)
{
    int32_t i;
    for (i = 0; i < RDPA_MAX_TCONT; i++)
    {
        if (g_tcont_ptr_to_rdd_table_index_mapping[i] == rdd_index)
            g_tcont_ptr_to_rdd_table_index_mapping[i] = RDD_QOS_MAPPER_INDEX_UNASSIGNED;
    }
}

void rdd_qos_mapper_init()
{
    int i;

    g_drop_q = QM_QUEUE_TO_BYTE_QUEUE(QM_QUEUE_DROP);
    memset(qos_table_index_reference_count, 0, sizeof(qos_table_index_reference_count));
    for (i = 0; i < RDPA_MAX_TCONT; i++)
        g_tcont_ptr_to_rdd_table_index_mapping[i] = RDD_QOS_MAPPER_INDEX_UNASSIGNED;
    
    for (i = 0; i < RDD_TX_FLOW_TABLE_SIZE; i++)
    {
        qos_mapper_tables_manager[i].tc2q_linked = 0;
        qos_mapper_tables_manager[i].pbit2q_linked = 0;
        qos_mapper_tables_manager[i].table_index = RDD_QOS_MAPPER_INDEX_UNASSIGNED;
    }
    /* set all DS/US tc to queue table entries to DS drop queue */
    for (i = 0; i < RDD_QOS_MAPPER_ID_MAX_TABLES * 8; i++)
    {
        RDD_TC_TO_QUEUE_8_QUEUE_OFFSET_WRITE_G(QM_QUEUE_DROP, RDD_TC_TO_QUEUE_TABLE_ADDRESS_ARR, i / 8, i % 8);
        RDD_PBIT_TO_QUEUE_8_QUEUE_OFFSET_WRITE_G(QM_QUEUE_DROP, RDD_PBIT_TO_QUEUE_TABLE_ADDRESS_ARR, i / 8, i % 8);
    }
    /* Set drop table to to DS drop queue */
    for (i = (RDD_TC_TO_QUEUE_TABLE_SIZE - 1) * 8; i < RDD_TC_TO_QUEUE_TABLE_SIZE * 8; i++)
    {
        RDD_TC_TO_QUEUE_8_QUEUE_OFFSET_WRITE_G(QM_QUEUE_DROP, RDD_TC_TO_QUEUE_TABLE_ADDRESS_ARR, i / 8, i % 8);
        RDD_PBIT_TO_QUEUE_8_QUEUE_OFFSET_WRITE_G(QM_QUEUE_DROP, RDD_PBIT_TO_QUEUE_TABLE_ADDRESS_ARR, i / 8, i % 8);
    }
    /* Set all TC_TO_QUEUE table pointers to invalid table ptr*/
#if defined(CONFIG_MULTI_WAN_SUPPORT) && !defined(BCM_PON_XRDP)
    for (i = 0; i < RDD_PON_TX_FLOW_TABLE_SIZE; i++)
        RDD_TX_FLOW_ENTRY_QOS_TABLE_PTR_WRITE_G(RDD_QOS_MAPPER_INDEX_UNASSIGNED, RDD_PON_TX_FLOW_TABLE_ADDRESS_ARR, i);
    for (i = 0; i < RDD_DSL_TX_FLOW_TABLE_SIZE; i++)
        RDD_TX_FLOW_ENTRY_QOS_TABLE_PTR_WRITE_G(RDD_QOS_MAPPER_INDEX_UNASSIGNED, RDD_DSL_TX_FLOW_TABLE_ADDRESS_ARR, i);
    for (i = 0; i < RDD_VPORT_TX_FLOW_TABLE_SIZE; i++)
        RDD_TX_FLOW_ENTRY_QOS_TABLE_PTR_WRITE_G(RDD_QOS_MAPPER_INDEX_UNASSIGNED, RDD_VPORT_TX_FLOW_TABLE_ADDRESS_ARR, i);
#else
    for (i = 0; i < RDD_TX_FLOW_TABLE_SIZE; i++)
        RDD_TX_FLOW_ENTRY_QOS_TABLE_PTR_WRITE_G(RDD_QOS_MAPPER_INDEX_UNASSIGNED, RDD_TX_FLOW_TABLE_ADDRESS_ARR, i);
#endif
    return;
}

static void _rdd_tc_to_queue_set_q(uint8_t rdd_index, uint8_t tc, uint16_t qm_queue_index)
{
    uint16_t entry_offset = rdd_index * RDD_TC_TO_QUEUE_8_QUEUE_OFFSET_NUMBER + tc;

    RDD_BTRACE("Setting TC %d to qm_queue %d at table number rdd_index %d\n", tc, qm_queue_index, rdd_index);
    if (qm_queue_index == QM_QUEUE_DROP)
        GROUP_MWRITE_8(RDD_TC_TO_QUEUE_TABLE_ADDRESS_ARR, entry_offset, qm_queue_index);
    else
        GROUP_MWRITE_8(RDD_TC_TO_QUEUE_TABLE_ADDRESS_ARR, entry_offset, QM_QUEUE_TO_BYTE_QUEUE(qm_queue_index));
}

static void _rdd_pbit_to_queue_set_q(uint8_t rdd_index, uint8_t pbit, uint16_t qm_queue_index)
{
    uint16_t entry_offset = rdd_index * RDD_TC_TO_QUEUE_8_QUEUE_OFFSET_NUMBER + pbit;
    RDD_BTRACE("Setting PBIT %d to qm_queue %d at table number rdd_index %d\n", pbit, qm_queue_index, rdd_index);
    if (qm_queue_index == QM_QUEUE_DROP)
        GROUP_MWRITE_8(RDD_PBIT_TO_QUEUE_TABLE_ADDRESS_ARR, entry_offset, qm_queue_index);
    else
        GROUP_MWRITE_8(RDD_PBIT_TO_QUEUE_TABLE_ADDRESS_ARR, entry_offset, QM_QUEUE_TO_BYTE_QUEUE(qm_queue_index));
}

/* TODO: implement
static void  _rdd_to_to_queue_copy_table(uint8_t old_idx, uint8_t alloc_idx, uint8_t vport_or_gem,
            rdpa_traffic_dir dir);
*/

static bdmf_error_t _rdd_alloc_tc_to_queue_table(rdpa_traffic_dir dir, uint16_t port_or_wan_flow, uint8_t *alloc_idx,
    bdmf_boolean is_pbit, bdmf_boolean is_tcont, bdmf_number tcont_idx)
{
    uint8_t start_idx, search_depth;
    int i;
    uint16_t tx_flow = PORT_OR_WAN_FLOW_TO_TX_FLOW(port_or_wan_flow, dir);
#if defined(CONFIG_MULTI_WAN_SUPPORT) && !defined(BCM_PON_XRDP)
    int tx_flow_per_type;
    uint32_t *table_addr_arr;
#endif

    if (dir == rdpa_dir_ds)
    {
        RDD_BTRACE("Allocating a DS TC to Queue table for port %d\n", port_or_wan_flow);
        start_idx = 0;
        search_depth = RDD_DS_QOS_MAPPER_ID_MAX_TABLES;
    }
    else
    {
        RDD_BTRACE("Allocating an US TC to Queue table for wan_flow %d\n", port_or_wan_flow);
        start_idx = RDD_DS_QOS_MAPPER_ID_MAX_TABLES;
        search_depth = RDD_US_QOS_MAPPER_ID_MAX_TABLES;
    }
    for (i = start_idx; i < start_idx + search_depth; i++)
    {
        BDMF_TRACE_DBG("TC DBG i = %d, (%d)\n", i, qos_table_index_reference_count[i]);

        if (qos_table_index_reference_count[i] == 0)
        {
            /* rdd table index is aligned to the required table size and there are enough free table entries */
            qos_table_index_reference_count[i]++;
            *alloc_idx = i;
            goto exit;
        }
    }
    RDD_TRACE("TC to Queue table memory depleted, cannot allocate table");

    return BDMF_ERR_NOMEM;

exit:

#if defined(CONFIG_MULTI_WAN_SUPPORT) && !defined(BCM_PON_XRDP)
    tx_flow_per_type = rdd_port_or_wan_flow_to_tx_flow_and_table_ptr(port_or_wan_flow, dir, &table_addr_arr);
    if (tx_flow_per_type < 0)
    {
        RDD_TRACE("Invalid port/wan_flow: %d ; tx_flow: %d, dir: %d\n",
                  port_or_wan_flow, tx_flow, dir);
        return BDMF_ERR_PARM;
    }
    /*Reload tx_flow*/
    tx_flow = tx_flow_per_type;
#endif
    /* allocate the new table */
    if (is_pbit)
        qos_mapper_tables_manager[tx_flow].pbit2q_linked = 1;
    else
        qos_mapper_tables_manager[tx_flow].tc2q_linked = 1;

    qos_mapper_tables_manager[tx_flow].table_index = *alloc_idx;

    if (is_tcont)
        g_tcont_ptr_to_rdd_table_index_mapping[tcont_idx] = *alloc_idx;

#if defined(CONFIG_MULTI_WAN_SUPPORT) && !defined(BCM_PON_XRDP)
    RDD_TX_FLOW_ENTRY_QOS_TABLE_PTR_WRITE_G(*alloc_idx, table_addr_arr, tx_flow_per_type);
#else
    RDD_TX_FLOW_ENTRY_QOS_TABLE_PTR_WRITE_G(*alloc_idx, RDD_TX_FLOW_TABLE_ADDRESS_ARR, tx_flow);
#endif
    return BDMF_ERR_OK;
}

/* used for EPON CTC, point all tx flows to the same table */
void rdd_qos_mapper_set_table_id_to_tx_flow(uint16_t src_tx_flow, uint16_t dst_tx_flow)
{
#if !defined(BCM63158)
    uint8_t table_id;
    RDD_TX_FLOW_ENTRY_QOS_TABLE_PTR_READ_G(table_id, RDD_TX_FLOW_TABLE_ADDRESS_ARR, src_tx_flow);
    RDD_TX_FLOW_ENTRY_QOS_TABLE_PTR_WRITE_G(table_id, RDD_TX_FLOW_TABLE_ADDRESS_ARR, dst_tx_flow);
#endif
}

void rdd_qos_mapper_invalidate_table(uint16_t port_or_wan_flow, rdpa_traffic_dir dir, uint8_t size, bdmf_boolean is_pbit)
{
    uint16_t tx_flow = PORT_OR_WAN_FLOW_TO_TX_FLOW(port_or_wan_flow, dir);
    uint8_t rdd_index;
    bdmf_boolean do_unlink;

#if defined(CONFIG_MULTI_WAN_SUPPORT) && !defined(BCM_PON_XRDP)
    int tx_flow_per_type;
    uint32_t *table_addr_arr;

    tx_flow_per_type = rdd_port_or_wan_flow_to_tx_flow_and_table_ptr(port_or_wan_flow, dir, &table_addr_arr);
    if (tx_flow_per_type < 0)
    {
        RDD_TRACE("Invalid port/wan_flow: %d ; tx_flow: %d, dir: %d\n",
            port_or_wan_flow, tx_flow, dir);
        return;
    }
    /* Reload tx_flow*/
    tx_flow = tx_flow_per_type;
#endif

    RDD_BTRACE("DBG Invalidating RDD TC to size %d and port/wan_flow %d, tx_flow %d\n",
        size, port_or_wan_flow, tx_flow);

    if (tx_flow >= RDD_TX_FLOW_TABLE_SIZE)
    {
        RDD_TRACE("tx_flow out of range %d\n", tx_flow);
        return;
    }

    rdd_index = qos_mapper_tables_manager[tx_flow].table_index;
    if (rdd_index >= RDD_QOS_MAPPER_ID_MAX_TABLES)
    {
        RDD_TRACE("rdd_index out of range %d\n", rdd_index);
        return;
    }

    RDD_BTRACE("Invalidating RDD TC to Queue table %d of size %d and port/wan_flow %d\n",
        size, rdd_index, port_or_wan_flow);

    if (is_pbit)
    {
        qos_mapper_tables_manager[tx_flow].pbit2q_linked = 0;
        do_unlink = (qos_mapper_tables_manager[tx_flow].tc2q_linked == 0);
    }
    else
    {
        qos_mapper_tables_manager[tx_flow].tc2q_linked = 0;    
        do_unlink = (qos_mapper_tables_manager[tx_flow].pbit2q_linked == 0);
    }


    BDMF_TRACE_DBG("Invalidating is_pbit %d, tc2q_linked %d, pbit2q_linked %d, do_unlink %d, ref_count %d\n",
        is_pbit, qos_mapper_tables_manager[tx_flow].tc2q_linked, 
        qos_mapper_tables_manager[tx_flow].pbit2q_linked, do_unlink,
        qos_table_index_reference_count[rdd_index]);

    if (do_unlink)
    {
#if defined(CONFIG_MULTI_WAN_SUPPORT) && !defined(BCM_PON_XRDP)
        RDD_TX_FLOW_ENTRY_QOS_TABLE_PTR_WRITE_G(RDD_QOS_MAPPER_INDEX_UNASSIGNED, table_addr_arr, tx_flow_per_type);
#else
        RDD_TX_FLOW_ENTRY_QOS_TABLE_PTR_WRITE_G(RDD_QOS_MAPPER_INDEX_UNASSIGNED, RDD_TX_FLOW_TABLE_ADDRESS_ARR, tx_flow);
#endif
        qos_mapper_tables_manager[tx_flow].table_index = RDD_QOS_MAPPER_INDEX_UNASSIGNED;

        if (qos_table_index_reference_count[rdd_index] > 0)
        {
            qos_table_index_reference_count[rdd_index]--;
            if (qos_table_index_reference_count[rdd_index] == 0)
                _clean_tcont_ptr_to_rdd_table(rdd_index);
        }
    }
}

bdmf_error_t rdd_realloc_tc_to_queue_table(uint16_t port_or_wan_flow,
    rdpa_traffic_dir dir, uint8_t *size)
{
    /*bdmf_error_t rc;
    uint8_t temp_rdd_idx, alloc_idx;*/

    /* TODO:Implement */
    return BDMF_ERR_OK;
}

/**************************************************************************************************
 * _rdd_qos_mapping_entry_set set entry to tc or pbit table
 *   PARAMS: port_or_wan_flow - port or wan id
 *           dir - traffic dir - up/ds
 *           is_pbit is_pbit (if 1 - pbit table, 0 tc table
 *           is_tcont - 1 tcont 
 *           tcont    - tcont_id 
**************************************************************************************************/
bdmf_error_t _rdd_qos_mapping_entry_set(uint16_t port_or_wan_flow, rdpa_traffic_dir dir,
    bdmf_boolean is_pbit, uint8_t *rdd_index_out, bdmf_boolean is_tcont, bdmf_number tcont_idx)
{
    uint16_t tx_flow = PORT_OR_WAN_FLOW_TO_TX_FLOW(port_or_wan_flow, dir);
    uint8_t rdd_index = qos_mapper_tables_manager[tx_flow].table_index;
    bdmf_error_t rc = BDMF_ERR_OK;

    /* tc to queue table not allocated -  allocate new table entry */
    if (rdd_index == RDD_QOS_MAPPER_INDEX_UNASSIGNED)
    {
        print_buf_dbg();

        if (is_tcont)
        {
            rdd_index = g_tcont_ptr_to_rdd_table_index_mapping[tcont_idx];
            if (rdd_index != RDD_QOS_MAPPER_INDEX_UNASSIGNED)
            {
#if defined(CONFIG_MULTI_WAN_SUPPORT) && !defined(BCM_PON_XRDP)
                int tx_flow_per_type;
                uint32_t *table_addr_arr;
                tx_flow_per_type = rdd_port_or_wan_flow_to_tx_flow_and_table_ptr(port_or_wan_flow, dir,
                    &table_addr_arr);
                if (tx_flow_per_type < 0)
                {
                    RDD_TRACE("Invalid port/wan_flow: %d ; tx_flow: %d, dir: %d\n",
                        port_or_wan_flow, tx_flow, dir);
                    return BDMF_ERR_PARM;
                }
                /* Reload tx_flow*/
                tx_flow = tx_flow_per_type;
                RDD_TX_FLOW_ENTRY_QOS_TABLE_PTR_WRITE_G(rdd_index, table_addr_arr, tx_flow_per_type);
#else
                RDD_TX_FLOW_ENTRY_QOS_TABLE_PTR_WRITE_G(rdd_index, RDD_TX_FLOW_TABLE_ADDRESS_ARR, tx_flow);
#endif
                qos_mapper_tables_manager[tx_flow].table_index = rdd_index;
                if (is_pbit)
                    qos_mapper_tables_manager[tx_flow].pbit2q_linked = 1;
                else
                    qos_mapper_tables_manager[tx_flow].tc2q_linked = 1;
                qos_table_index_reference_count[rdd_index]++;
                goto exit;
            }     
        }
        rc = _rdd_alloc_tc_to_queue_table(dir, port_or_wan_flow, &rdd_index, is_pbit, is_tcont, tcont_idx);
    }
exit:
    if (rdd_index_out)
        *rdd_index_out = rdd_index;
    else
       return BDMF_ERR_PARM;
    return rc;
}

bdmf_error_t rdd_tc_to_queue_entry_set(uint16_t port_or_wan_flow, rdpa_traffic_dir dir, uint8_t tc, uint16_t qm_queue_index, bdmf_boolean is_tcont, bdmf_number tcont_idx)
{
    uint8_t rdd_index;
    bdmf_error_t rc;

    rc = _rdd_qos_mapping_entry_set(port_or_wan_flow, dir, 0, &rdd_index, is_tcont, tcont_idx);
    if (rc)
    {
        RDD_BTRACE("Failed to allocate new TC_TO_QUEUE table, port_or_wan_flow %d, ERR %d\n",  port_or_wan_flow, rc);
        return rc;
    }

    BDMF_TRACE_DBG("DBG TC_TO_QUEUE wan_flow %d, rdd_index %d, qm_queue_index %d, tc %d\n", port_or_wan_flow, rdd_index, qm_queue_index, tc);

    if (dir == rdpa_dir_ds)
        RDD_BTRACE("Setting TC %d to Queue %d of RDD table number %d and port %d\n",
                tc, qm_queue_index, rdd_index, port_or_wan_flow);
    else
        RDD_BTRACE("Setting TC %d to Queue %d of RDD table number %d and wan_flow %d\n",
                tc, qm_queue_index, rdd_index, port_or_wan_flow);

    _rdd_tc_to_queue_set_q(rdd_index, tc, qm_queue_index);
    return BDMF_ERR_OK;
}

bdmf_error_t rdd_pbit_to_queue_entry_set(uint16_t port_or_wan_flow, rdpa_traffic_dir dir, uint8_t pbit, 
    uint16_t qm_queue_index, bdmf_boolean is_tcont, bdmf_number tcont_idx)
{
    uint8_t rdd_index;
    bdmf_error_t rc;

    rc = _rdd_qos_mapping_entry_set(port_or_wan_flow, dir, 1, &rdd_index, is_tcont, tcont_idx);
    if (rc)
    {
        RDD_BTRACE("Failed to allocate new PBIT_TO_QUEUE table, port_or_wan_flow %d, ERR %d\n",
            port_or_wan_flow, rc);
        return rc;
    }

    if (dir == rdpa_dir_ds)
        RDD_BTRACE("Setting PBIT %d to Queue %d of RDD table number %d and port %d\n",
                pbit, qm_queue_index, rdd_index, port_or_wan_flow);
    else
        RDD_BTRACE("Setting PBIT %d to Queue %d of RDD table number %d and wan_flow %d\n",
                pbit, qm_queue_index, rdd_index, port_or_wan_flow);

    BDMF_TRACE_DBG("DBG PBIT_TO_QUEUE wan_flow %d, rdd_index %d, qm_queue_index %d, pbit %d\n", port_or_wan_flow, rdd_index, qm_queue_index, pbit);

    _rdd_pbit_to_queue_set_q(rdd_index, pbit, qm_queue_index);
    return BDMF_ERR_OK;
}

bdmf_error_t rdd_us_pbits_to_wan_flow_entry_cfg(uint8_t gem_mapping_table, uint8_t pbit, uint8_t gem)
{
    GROUP_MWRITE_8(RDD_PBIT_TO_GEM_TABLE_ADDRESS_ARR, (gem_mapping_table * RDD_PBIT_TO_GEM_TABLE_SIZE2) + pbit, gem);
    return BDMF_ERR_OK;
}
