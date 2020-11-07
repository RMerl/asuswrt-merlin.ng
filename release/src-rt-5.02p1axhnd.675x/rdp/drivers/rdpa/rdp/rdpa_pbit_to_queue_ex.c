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
#include "rdpa_int.h"
#include "rdd.h"
#include "rdpa_egress_tm_inline.h"
#include "rdpa_rdd_inline.h"
#include "rdpa_pbit_to_queue_ex.h"
#include "rdpa_qos_mapper_ex.h"
#if !defined(LEGACY_RDP) && !defined(WL4908)
#include "rdd_multicast_processing.h"
#endif

int pbit_to_queue_set_single_entry_ex(struct bdmf_object *mo, bdmf_boolean set_to_rdd,
    bdmf_object_handle obj, bdmf_index pbit, bdmf_index queue_id, bdmf_boolean link)
{
    pbit_to_queue_drv_priv_t *tbl = (pbit_to_queue_drv_priv_t *)bdmf_obj_data(mo);
    rdd_bridge_port_t rdd_src_port;
    bdmf_number channel_idx;
    rdpa_if port;
    int rc_id = 0, queue = 0;
    int rc = 0;
    bdmf_boolean mgmt;
    uint8_t tc_table = 0;
    bdmf_object_handle gem = NULL;
    rdpa_gem_flow_us_cfg_t gem_us_cfg;

    if (obj->drv == rdpa_port_drv())
    {
        rc = rdpa_port_index_get(obj, &port);
        if (rdpa_if_is_lan_or_wifi(port) || port == rdpa_if_switch)
        {
            if (port == rdpa_if_switch)
                rc = set_switch_port(mo, set_to_rdd, link, pbit, queue_id, 0, rdpa_pbit_to_queue);
            else
            {
                rdd_src_port = rdpa_port_rdpa_if_to_vport(port);
                if (link)
                    rc = _rdpa_egress_tm_lan_port_queue_to_rdd(port, queue_id, &rc_id, &queue);

                if (set_to_rdd)
                    rc = rc ? rc : rdd_ds_pbits_to_qos_entry_cfg(rdd_src_port, pbit, queue);
            }
        }
        else if (rdpa_if_is_wan(port))
        {
            bdmf_link_handle us_link = NULL;
            bdmf_number tc_tbl;

            if (!rdpa_is_gbe_mode() && !rdpa_is_car_mode() && !rdpa_is_epon_ae_mode())
            {
                BDMF_TRACE_RET_OBJ(BDMF_ERR_PARM, mo,
                    "upstream table can be linked to port object only in GBE or CAR mode or active ethernet\n");
            }

            while ((us_link = bdmf_get_next_us_link(obj, us_link)))
            {
                if (bdmf_us_link_to_object(us_link)->drv == rdpa_tc_to_queue_drv())
                {
                    rdpa_tc_to_queue_table_get(bdmf_us_link_to_object(us_link), &tc_tbl);
                    tc_table = (uint8_t)tc_tbl;
                    break;
                }
            }

            if (rdpa_is_gbe_mode())
            {
                channel_idx = port;

                if (link)
                    rc = _rdpa_egress_tm_channel_queue_to_rdd(rdpa_dir_us, channel_idx, queue_id, &rc_id, &queue);

                if (set_to_rdd)
                {
                    rc = rc ? rc : rdd_us_pbits_to_qos_entry_cfg(tbl->index, pbit, queue, (uint8_t)rc_id);
                    if (!rc)
                        rdd_us_wan_flow_cfg(0, 0, 0, 0, 1, 0, tbl->index, tc_table);
                }
            }
            else if (rdpa_is_car_mode())
            {
                /* Traverse configured gems and assign the table */
                while ((gem = bdmf_get_next(rdpa_gem_drv(), gem, NULL)))
                {
                    rdpa_gem_us_cfg_get(gem, &gem_us_cfg);
                    if (!gem_us_cfg.tcont)
                        continue;

                    rdpa_tcont_management_get(gem_us_cfg.tcont, &mgmt);
                    if (mgmt)
                        continue;

                    rdpa_tcont_channel_get(gem_us_cfg.tcont, &channel_idx);
                    if (link)
                    {
                        rc = _rdpa_egress_tm_channel_queue_to_rdd(rdpa_dir_us, channel_idx,
                            queue_id, &rc_id, &queue);
                    }

                    if (set_to_rdd)
                    {
                        rc = rc ? rc : rdd_us_pbits_to_qos_entry_cfg(tbl->index, pbit, queue, (uint8_t)rc_id);

                        BDMF_TRACE_DBG_OBJ(mo, "Configure US pbit %d to queue_id %d"
                            " (q,rc: %d %d) pbit_table %d tc_table %d in RDD", (int)pbit, (int)queue_id, 
                            queue, rc_id, (int)tbl->index, tc_table);

                        if (!rc)
                            us_wan_flow_rdd_cfg(mo, gem, channel_idx, tbl->index, tc_table);
                    }

                    if (rc)
                    {
                        bdmf_put(gem);
                        break;
                    }
                }
            }
        }
    }
    else if (obj->drv == rdpa_tcont_drv())
    {
        bdmf_boolean mgmt;
        uint8_t tc_table = 0;
        bdmf_object_handle gem = NULL;

        /* OMCI management tcont is not a typical tcont*/
        rdpa_tcont_management_get(obj, &mgmt);
        if (mgmt)
            BDMF_TRACE_RET(BDMF_ERR_PERM, "Can't link pbit to queue to management tcont");

        rdpa_tcont_channel_get(obj, &channel_idx);
        if (link)
            rc = _rdpa_egress_tm_channel_queue_to_rdd(rdpa_dir_us, channel_idx , queue_id, &rc_id, &queue);

        if (set_to_rdd)
        {
            rc = rc ? rc : rdd_us_pbits_to_qos_entry_cfg(tbl->index, pbit, queue, (uint8_t)rc_id);

            /* Walk over all gems that points on that tcont */
            if (!rc)
            {
                while ((gem = bdmf_get_next(rdpa_gem_drv(), gem, NULL)))
                {
                    rdpa_gem_us_cfg_get(gem, &gem_us_cfg);

                    if (gem_us_cfg.tcont == obj)
                    {
#ifdef CONFIG_BCM_TCONT
                        tc_table = tcont_tc_table_get(gem_us_cfg.tcont);
#endif
                        /* Set the gem to point on the tc table */
                        us_wan_flow_rdd_cfg(mo, gem, channel_idx, tbl->index, tc_table);
                    }
                }
            }
        }
    }
    else if (obj->drv == rdpa_llid_drv())
    {
        int channel;
        queue_info_t queue_id_info = {};
        llid_drv_priv_t *llid = (llid_drv_priv_t *)bdmf_obj_data(obj);

        if (!llid)
            BDMF_TRACE_RET_OBJ(BDMF_ERR_INTERNAL, mo, "failed to get llid object");

        if (link)
        {
            rc = egress_tm_queue_id_info_get(rdpa_dir_us, llid->index , queue_id, &queue_id_info);
            BDMF_TRACE_INFO("llid->index=%d, queue_id=%d : [rc_id=%d, queue=%d, channel=%d]\n", 
                (int)llid->index, (int)queue_id, (int)queue_id_info.rc_id, (int)queue_id_info.queue,
                (int)queue_id_info.channel);
        }

        if (rc)
            BDMF_TRACE_RET_OBJ(rc, mo, "failed to get llid egress_tm for llid->index=%d", (int)llid->index);

        if (set_to_rdd)
        {
            if (is_rdpa_epon_ctc_or_cuc_mode())
            {
                BDMF_TRACE_INFO("llid->index=%d, queue_id=%d : [rc_id=%d, queue=%d, channel=%d]\n", 
                    (int)llid->index, (int)queue_id, (int)queue_id_info.rc_id, (int)queue_id_info.queue,
                    (int)queue_id_info.channel);
                rc = rdd_us_pbits_to_wan_flow_entry_cfg(tbl->index, pbit, queue_id_info.channel);
            }
            if (!rc) 
                rc = rdd_us_pbits_to_qos_entry_cfg(tbl->index, pbit, queue_id_info.queue, (uint8_t)queue_id_info.rc_id);

            if (!rc) 
            {
                /* data channels starts from 1 */
                for (channel = 1; channel < llid->num_channels; channel++)
                {
                    rdd_us_wan_flow_cfg(llid->channels[channel], llid->channels[channel], 0,
                        0, 1, 0, tbl->index, tc_table);
                }
            }
        }
    }

    if (rc)
    {
        BDMF_TRACE_RET_OBJ(BDMF_ERR_INTERNAL, mo,
            "Can't set table index %d pbit %d to queue %d. err %d\n",
            (int)tbl->index, (int)pbit, (int)queue_id, rc);
    }

    return 0;
}

void rdpa_pbit_to_queue_unlink_other_ex(struct bdmf_object *mo, struct bdmf_object *other)
{
    return; /*not supported in RDP*/
}

