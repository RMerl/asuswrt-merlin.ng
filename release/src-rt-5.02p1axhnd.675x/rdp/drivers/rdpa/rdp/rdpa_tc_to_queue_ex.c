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
#include "rdp_drv_ih.h"
#include "rdpa_int.h"
#include "rdpa_egress_tm_inline.h"
#include "rdpa_rdd_inline.h"
#include "rdpa_qos_mapper_ex.h"
#include "rdpa_tc_to_queue_ex.h"
#if !defined(LEGACY_RDP) && !defined(WL4908)
#include "rdd_multicast_processing.h"
#endif

bdmf_object_handle ds_tc_to_queue_objects[RDPA_DS_TC_TO_QUEUE_ID_MAX_TABLES];
bdmf_object_handle us_tc_to_queue_objects[RDPA_US_TC_TO_QUEUE_ID_MAX_TABLES];

void rdpa_tc_to_queue_obj_init_ex(bdmf_object_handle **container, int *max_tables, uint8_t *table_size, rdpa_traffic_dir dir)
{
    if (!*table_size)
    {
        *table_size = (dir == rdpa_dir_ds) ?
                RDPA_DS_TC_TO_QUEUE_ID_TABLE_SIZE : RDPA_US_TC_TO_QUEUE_ID_TABLE_SIZE;
    }
    *max_tables = (dir == rdpa_dir_ds) ? RDPA_DS_TC_TO_QUEUE_ID_MAX_TABLES
                : RDPA_US_TC_TO_QUEUE_ID_MAX_TABLES;
    *container = (dir == rdpa_dir_ds) ?  ds_tc_to_queue_objects : us_tc_to_queue_objects;
}

int rdpa_tc_to_queue_set_single_entry_ex(struct bdmf_object *mo, bdmf_boolean set_to_rdd,
    bdmf_object_handle port_obj, bdmf_index tc, bdmf_index queue, bdmf_boolean link)
{
    tc_to_queue_drv_priv_t *tbl = (tc_to_queue_drv_priv_t *)bdmf_obj_data(mo);

    int rc_id = 0, prty = 0;
    int rc = 0;

    if (tbl->dir == rdpa_dir_ds)
    {
        port_drv_priv_t *port = (port_drv_priv_t *)bdmf_obj_data(port_obj);
        if (port->index == rdpa_if_switch)
            rc = set_switch_port(mo, set_to_rdd, link, tc, queue, 0, rdpa_tc_to_queue);
        else
        {
            if (link)
            {
                if (!rdpa_if_is_wifi(port->index))
                    rc = _rdpa_egress_tm_lan_port_queue_to_rdd(port->index, queue, &rc_id, &prty);
                else 
                    prty = queue * 2;
            }

            if (set_to_rdd)
            {
                rdd_bridge_port_t rdd_src_port = rdpa_port_rdpa_if_to_vport(port->index);

                rc = rc ? rc : rdd_ds_tc_to_queue_entry_cfg(rdd_src_port, (uint8_t) tc, prty);
            }
        }
    }
    else /*handle upstream case */
    {
        bdmf_object_handle gem = NULL;
        bdmf_object_handle tcont = port_obj;
        bdmf_boolean mgmt;
        bdmf_number channel_idx;
        uint8_t pbit_table = 0;
        rdpa_gem_flow_us_cfg_t gem_us_cfg;

        /* handle Gbe or car mode */
        if (port_obj->drv == rdpa_port_drv())
        {
            rdpa_if port_idx;
            bdmf_number pbit_tbl;
            bdmf_link_handle us_link = NULL;

            while ((us_link = bdmf_get_next_us_link(port_obj, us_link)))
            {
                if (bdmf_us_link_to_object(us_link)->drv == rdpa_pbit_to_queue_drv())
                {
                    rdpa_pbit_to_queue_table_get(bdmf_us_link_to_object(us_link), &pbit_tbl);
                    pbit_table = (uint8_t)pbit_tbl;
                    break;
                }
            }

            rdpa_port_index_get(port_obj, &port_idx);

            if (rdpa_is_gbe_mode())
            {
                if (link)
                    rc = _rdpa_egress_tm_channel_queue_to_rdd(rdpa_dir_us, port_idx , queue, &rc_id, &prty);
                if (set_to_rdd)
                {
                    rc = rc ? rc : rdd_us_tc_to_queue_entry_cfg(tbl->index, tc, prty, rc_id);
                    if (!rc)
                        rdd_us_wan_flow_cfg(0, 0, 0, 0, 1,  0, pbit_table, tbl->index);
                }
            }
            else if (rdpa_is_car_mode())
            {
                /* walk over all configured gems and assign the table */
                while ((gem = bdmf_get_next(rdpa_gem_drv(), gem, NULL)))
                {
                    rdpa_gem_us_cfg_get(gem, &gem_us_cfg);

                    rdpa_tcont_management_get(gem_us_cfg.tcont, &mgmt);
                    if (mgmt)
                        continue;

                    rdpa_tcont_channel_get(gem_us_cfg.tcont, &channel_idx);

                    if (link)
                        rc = _rdpa_egress_tm_channel_queue_to_rdd(rdpa_dir_us, channel_idx , queue, &rc_id, &prty);

                    if (set_to_rdd)
                    {
                        rc = rc ? rc : rdd_us_tc_to_queue_entry_cfg(tbl->index, tc, prty, rc_id);
                        if (!rc)
                            us_wan_flow_rdd_cfg(mo, gem, channel_idx, pbit_table, tbl->index);
                    }

                    if (rc)
                    {
                        bdmf_put(gem);
                        break;
                    }

                    BDMF_TRACE_DBG_OBJ(mo, "Configure US traffic_class %d to queue_id %d " 
                        "(q,rc: %d %d) pbit_table %d tc_table %d in RDD", (int)tc, (int)queue, 
                        prty, rc_id, pbit_table, (int)tbl->index);
                }
            }
        }
        else if (port_obj->drv == rdpa_tcont_drv())
        {
            /* OMCI management tcont is not a typical tcont*/
            rdpa_tcont_management_get(tcont, &mgmt);
            if (mgmt)
                BDMF_TRACE_RET(BDMF_ERR_PERM, "Can't configure tc_to_queue on management tcont");

            rdpa_tcont_channel_get(tcont, &channel_idx);

            if (link)
                rc = _rdpa_egress_tm_channel_queue_to_rdd(rdpa_dir_us, channel_idx , queue, &rc_id, &prty);

            if (set_to_rdd)
            {
                bdmf_object_handle gem = NULL;

                rc = rc ? rc : rdd_us_tc_to_queue_entry_cfg(tbl->index, tc, prty, rc_id);

                if (!rc)
                {
                    while ((gem = bdmf_get_next(rdpa_gem_drv(), gem, NULL)))
                    {
                        rdpa_gem_us_cfg_get(gem, &gem_us_cfg);

                        if (gem_us_cfg.tcont == tcont)
                        {
#ifdef CONFIG_BCM_TCONT
                            pbit_table = tcont_pbit_table_get(gem_us_cfg.tcont);
#endif
                            /* Set the gem to point on the tc table */
                            us_wan_flow_rdd_cfg(mo, gem, channel_idx, pbit_table, tbl->index);
                        }
                    }
                }
            }
        }
    }

    if (rc)
    {
        BDMF_TRACE_RET_OBJ(BDMF_ERR_INTERNAL, mo, "Can't set table index %d dir %s tc %d to queue %d. err %d\n",
            (int)tbl->index, (tbl->dir == rdpa_dir_ds) ? "ds" : "us", (int)tc, (int)queue, rc);
    }

    return 0;
}

bdmf_error_t rdpa_tc_to_queue_realloc_table_ex(int *tc_to_queue_linked_tcont_llid, tc_to_queue_drv_priv_t *tbl)
{
    return BDMF_ERR_OK;
}

void rdpa_tc_to_queue_unlink_port_ex(struct bdmf_object *mo, struct bdmf_object *other, bdmf_number port)
{
    return;
}
