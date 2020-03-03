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
#include "bdmf_dev.h"
#include "rdpa_api.h"
#include "rdpa_int.h"
#include "rdd_ih_defs.h"
#include "rdd.h"
#include "rdpa_egress_tm_inline.h"
#include "rdpa_rdd_inline.h"
#include "rdpa_dscp_to_pbit_ex.h"
#include "rdpa_qos_mapper_ex.h"
#include "rdp_drv_ih.h"

#define IH_DSCP_TO_PBIT_TABLE_INDEX 0

static int dscp_pbit_linked_port[rdpa_if__number_of]; /* table_id+1 if linked, 0 if not linked */
static uint8_t dscp_pbit_map[RDPA_DSCP_TO_PBIT_MAX_TABLES][64];

/* Init data structures */
bdmf_error_t rdpa_rdd_dscp_to_pbit_init(void)
{
    int i;
    for (i = 0; i < rdpa_if__number_of; i++)
        dscp_pbit_linked_port[i] = RDPA_RDD_DSCP_MAPPING_TABLE_UNDEFINED;
    return BDMF_ERR_OK;
}

/* Set DSCP to TCP mapping.
 */
bdmf_error_t rdpa_rdd_vlan_dscp_pbit_mapping_set(struct bdmf_object *mo, uint8_t table, uint8_t dscp, uint8_t pbit)
{
    rdd_bridge_port_t rdd_src_port;
    rdpa_if port;
    int rc = 0;

    if (table >= RDPA_DSCP_TO_PBIT_MAX_TABLES)
        return BDMF_ERR_RANGE;
    for (port = 0; port < rdpa_if__number_of; port++)
    {
        /* For switch port we handle its LAG interfaces */
        if (port == rdpa_if_switch)
            continue;

        if (dscp_pbit_linked_port[port] == table)
        {
            rdd_src_port = rdpa_port_rdpa_if_to_vport(port);

#ifdef LEGACY_RDP
            rc = rdd_dscp_to_pbits_cfg(rdd_src_port, dscp, pbit);
#else
            if (rdd_src_port == BL_LILAC_RDD_WAN_BRIDGE_PORT)
                rdd_dscp_to_pbits_cfg(rdpa_dir_ds, rdd_src_port, dscp, pbit);
            else
                rdd_dscp_to_pbits_cfg(rdpa_dir_us, rdd_src_port, dscp, pbit);
#endif
            if (rc)
            {
                BDMF_TRACE_RET_OBJ(BDMF_ERR_INTERNAL, mo,
                    "Can't link dscp %d pbit %d to port %s, rdd_error = %d\n",
                    (int)dscp, (int)pbit, bdmf_attr_get_enum_text_hlp(&rdpa_if_enum_table, port), rc);
            }
        }
    }

    dscp_pbit_map[table][dscp] = pbit;

    return BDMF_ERR_OK;
}

/* Set DSCP to TCP mapping.
 */
bdmf_error_t rdpa_rdd_qos_dscp_pbit_mapping_set(struct bdmf_object *mo, uint8_t dscp, uint8_t pbit)
{
    int rc = 0;

    rc = fi_bl_drv_ih_set_dscp_to_tci_table_entry(IH_DSCP_TO_PBIT_TABLE_INDEX, dscp, pbit);
    if (rc)
        BDMF_TRACE_RET_OBJ(BDMF_ERR_INTERNAL, mo, "Can't set dscp %u pbit %u. ih error = %d\n", dscp, pbit, rc);

    rc = rdd_dscp_to_pbits_global_cfg(dscp, pbit);
    if (rc)
        BDMF_TRACE_RET_OBJ(BDMF_ERR_INTERNAL, mo, "Can't set dscp %u pbit %u. rdd error = %d\n", dscp, pbit, rc);

    return BDMF_ERR_OK;
}

/* Set DSCP to TCP+DEI mapping.
 */
bdmf_error_t rdpa_rdd_qos_dscp_pbit_dei_mapping_set(struct bdmf_object *mo, uint8_t dscp, uint8_t pbit, uint8_t dei)
{
    int rc;
    rc = rdpa_rdd_qos_dscp_pbit_mapping_set(mo, dscp, pbit);
    rc = rc ? rc : rdd_dscp_to_pbits_dei_global_cfg(dscp, pbit, dei);
    if (rc)
    {
        BDMF_TRACE_RET_OBJ(BDMF_ERR_INTERNAL, mo, "Can't set dscp %u pbit %u  dei %u. rdd error = %d\n",
            dscp, pbit, dei, rc);
    }
    return BDMF_ERR_OK;
}

/* Set vport to DSCP-To-PBIT table mapping.
 * Use table_id=RDPA_RDD_DSCP_TO_PBIT_TABLE_UNDEFINED to remove the existing mapping
 */
bdmf_error_t rdpa_rdd_port_to_dscp_to_pbit_table_set(struct bdmf_object *mo, rdpa_if port, uint8_t table)
{
    rdd_bridge_port_t rdd_src_port;
    uint8_t pbit;
    int i;
    int rc = 0;


    if (port == rdpa_if_switch)
    {
        rc = set_switch_port_to_dscp_pbit_table(mo, table);
    }
    else
    {
        rdd_src_port = rdpa_port_rdpa_if_to_vport(port);
        for (i = 0; i < 64 && !rc; i++)
        {
            pbit = (table == RDPA_RDD_DSCP_MAPPING_TABLE_UNDEFINED) ? 0 : dscp_pbit_map[table][i];
#ifdef LEGACY_RDP
            rc = rdd_dscp_to_pbits_cfg(rdd_src_port, i, pbit);
#else
            if (rdd_src_port == BL_LILAC_RDD_WAN_BRIDGE_PORT)
                rdd_dscp_to_pbits_cfg(rdpa_dir_ds, rdd_src_port, i, pbit);
            else
                rdd_dscp_to_pbits_cfg(rdpa_dir_us, rdd_src_port, i, pbit);
#endif
        }
    }
    if (rc)
        return rc;

    dscp_pbit_linked_port[port] = table;

    return rc;
}
