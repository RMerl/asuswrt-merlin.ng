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
#include "rdpa_qos_mapper_ex.h"
#include "rdpa_dscp_to_pbit_ex.h"
 
#include "rdd_qos_mapper.h"
#include "rdpa_tcont_ex.h"
#include "rdpa_egress_tm_ex.h"

/*******************************************************************************/ 
/* _rdpa_qos_link_tcont_to_qos                                                 */
/*                                                                             */
/* Abstruct:                                                                   */
/* This function set single entry in TC or PBIT to TCONT object                */ 
/*                                                                             */
/* Input:                                                                      */
/*   mo  - tc_to_queue or pbit_to_queue driver handler                         */
/*   set_to_rdd  - flag if store data to FW                                    */
/*   obj - pointer to link object - (tcont,port)                               */
/*   link - link /unlink flag                                                  */
/*   qos - what index in pbit or tc table to change                           */
/*   queue_id - queue_id to set in index pbit of tc/pbit to queue table        */
/*   f_rdd_qos_to_queue_set - pointer to rdd access function for tc_to_queue/pbit_to_queue */ 
/*   tbl_idx - index of tc_to_queue/pbit to queue                              */
/* Output:                                                                     */
/*   rc - error code                                                           */
/*        0 - no error                                                         */
/*        other error,                                                         */  
/*******************************************************************************/
static int _rdpa_qos_link_tcont_to_qos(struct bdmf_object *mo, bdmf_boolean set_to_rdd, bdmf_object_handle obj,
    bdmf_index qos, bdmf_index queue_id, bdmf_boolean link, f_rdd_qos_to_queue_set_t f_rdd_qos_to_queue_set, int tbl_idx)
{
    bdmf_object_handle gem = NULL;
    bdmf_boolean mgmt;
    bdmf_number channel_id, flow_idx;
    int rc = BDMF_ERR_OK, queue = QM_QUEUE_DROP, rc_id;
    rdpa_gem_flow_us_cfg_t gem_us_cfg;

    /* OMCI management tcont is not a typical tcont*/
    if (!rdpa_is_car_mode())
    {
        rdpa_tcont_management_get(obj, &mgmt);
        if (mgmt)
            BDMF_TRACE_RET(BDMF_ERR_PERM, "can't link qos to queue to management tcont");

        rdpa_tcont_index_get(obj, &channel_id);
        channel_id = (int)rdpa_tcont_tcont_id_to_channel_id(channel_id);

        if (link)
            rc = _rdpa_egress_tm_channel_queue_to_rdd(rdpa_dir_us, (int)(channel_id) , queue_id, &rc_id, &queue);

        if (set_to_rdd)
        {
            /* Walk over all gems that points on that tcont */
            if (!rc)
            {
                while ((gem = bdmf_get_next(rdpa_gem_drv(), gem, NULL)))
                {
                    rdpa_gem_us_cfg_get(gem, &gem_us_cfg);
                    if (gem_us_cfg.tcont == obj)
                    {
                        rdpa_gem_index_get(gem, &flow_idx);
                        rc = rc ? rc : f_rdd_qos_to_queue_set(flow_idx, rdpa_dir_us, qos, queue, 1, channel_id);
                    }
                }
            }
        }
    }
    else /* car_mode*/
    { 	
        while ((gem = bdmf_get_next(rdpa_gem_drv(), gem, NULL)))
        {
            rdpa_gem_us_cfg_get(gem, &gem_us_cfg);
            if (!gem_us_cfg.tcont)
                continue;

            rdpa_tcont_management_get(gem_us_cfg.tcont, &mgmt); 
            if (mgmt)
                continue;

            rdpa_tcont_index_get(gem_us_cfg.tcont, &channel_id); 
            channel_id = (int)rdpa_tcont_tcont_id_to_channel_id(channel_id);

            if (link) 
                rc = _rdpa_egress_tm_channel_queue_to_rdd(rdpa_dir_us, (int)(channel_id), queue_id, &rc_id, &queue);

            if (set_to_rdd)
            {
                rdpa_gem_index_get(gem, &flow_idx); 
                rc = rc ? rc : f_rdd_qos_to_queue_set(flow_idx, rdpa_dir_us, qos, queue, 1,  channel_id);
                    BDMF_TRACE_DBG_OBJ(mo, "Configure US qos %d to queue_id %d"
                        " (q,rc: %d %d) qos_table %d in RDD", (int)qos, (int)queue_id, 
                        queue, rc_id, tbl_idx);
            }
            if (rc)
            {
                bdmf_put(gem);
                break;
            }
        }
    }

    return rc;
}

/*******************************************************************************/ 
/* _rdpa_qos_link_single_llid_to_qos                                           */
/*                                                                             */
/* Abstruct:                                                                   */
/* This function set single entry in TC or PBIT to LLID object                */ 
/*                                                                             */
/* Input:                                                                      */
/*   mo  - tc_to_queue or pbit_to_queue driver handler                         */
/*   set_to_rdd  - flag if store data to FW                                    */
/*   obj - pointer to link object - (tcont,port)                               */
/*   link - link /unlink flag                                                  */
/*   qos - what index in pbit or tc table to change                           */
/*   queue_id - queue_id to set in index pbit of tc/pbit to queue table        */
/*   tbl_idx - index of tc_to_queue/pbit to queue                              */
/* Output:                                                                     */
/*   rc - error code                                                           */
/*        0 - no error                                                         */
/*        other error,                                                         */
/*******************************************************************************/
static int _rdpa_qos_link_single_llid_to_qos(struct bdmf_object *mo, bdmf_boolean set_to_rdd,
    bdmf_object_handle llid_obj, bdmf_index qos, bdmf_index queue_id, bdmf_boolean link, 
    f_rdd_qos_to_queue_set_t f_rdd_qos_to_queue_set, int tbl_idx)
{
    llid_drv_priv_t *llid_priv = (llid_drv_priv_t *)bdmf_obj_data(llid_obj);
    queue_info_t queue_id_info = {.queue = QM_QUEUE_DROP};
    bdmf_number channel_id;
    int rc = BDMF_ERR_OK;

    if (link)
    {
        rc = rdpa_egress_tm_queue_info_get(llid_priv->data_egress_tm, queue_id, &queue_id_info);
        BDMF_TRACE_INFO("llid_priv->index=%d, queue_id=%d : [rc_id=%d, queue=%d, channel=%d]\n",
            (int)llid_priv->index, (int)queue_id, (int)queue_id_info.rc_id, (int)queue_id_info.queue,
            (int)queue_id_info.channel);
    }
    if (rc)
    {
        BDMF_TRACE_RET_OBJ(rc, mo, "failed to get llid egress_tm for llid->index=%d",
            (int)llid_priv->index);
    }
    if (set_to_rdd)
    {
        rc = f_rdd_qos_to_queue_set((uint8_t)llid_priv->index, rdpa_dir_us, qos, queue_id_info.queue, 0, 0);
        if (!rc && is_rdpa_epon_ctc_or_cuc_mode())
        {
            for (channel_id = 0; channel_id < (llid_priv->num_channels + 1); channel_id++)
                rdd_qos_mapper_set_table_id_to_tx_flow(llid_priv->index, llid_priv->channels[channel_id]);
        }
        if (rc)
        {
            BDMF_TRACE_RET_OBJ(BDMF_ERR_INTERNAL, mo, "can't set table index %d qos %d to queue %d for llid->index=%d. err %d\n",
                tbl_idx, (int)qos, (int)queue_id, (int)llid_priv->index, rc);
        }
    }
    return rc;
}

/*******************************************************************************/ 
/* rdpa_qos_to_queue_set_single_entry                                          */
/*                                                                             */
/* Abstruct:                                                                   */
/* This function set single entry in TC or PBIT to queue table                 */ 
/*                                                                             */
/* Input:                                                                      */
/*   mo  - tc_to_queue or pbit_to_queue driver handler                         */
/*   set_to_rdd  - flag if store data to FW                                    */
/*   obj - pointer to link object - (tcont,port)                               */
/*   link - link /unlink flag                                                  */
/*   qos - what index in pbit or tc table to change                           */
/*   queue_id - queue_id to set in index pbit of tc/pbit to queue table        */
/*   f_rdd_qos_to_queue_set - pointer to rdd access function for tc_to_queue/pbit_to_queue */ 
/*   tbl_idx - index of tc_to_queue/pbit to queue                              */
/* Output:                                                                     */
/*   rc - error code                                                           */
/*        0 - no error                                                         */
/*        other error,                                                         */  
/*******************************************************************************/
int rdpa_qos_to_queue_set_single_entry_ex(struct bdmf_object *mo, bdmf_boolean set_to_rdd,
    bdmf_object_handle obj, bdmf_index qos, bdmf_index queue_id, bdmf_boolean link, f_rdd_qos_to_queue_set_t f_rdd_qos_to_queue_set, int tbl_idx)
{
    rdd_vport_id_t vport;
    rdpa_if port;
    int rc = 0, rc_id = 0, queue = QM_QUEUE_DROP;

    bdmf_object_handle llid_obj = NULL;

    if (obj->drv == rdpa_port_drv())
    { 
        rc = rdpa_port_index_get(obj, &port);
        if (rdpa_if_is_cpu_port(port))
        {
            BDMF_TRACE_RET_OBJ(BDMF_ERR_NOT_SUPPORTED, mo,
                "Cannot link QOS table to WIFI or CPU port\n");	
        }
        if (rdpa_if_is_lan(port))
        {
            vport = rdpa_port_rdpa_if_to_vport(port);
            if (link)
                rc = _rdpa_egress_tm_lan_port_queue_to_rdd(port, queue_id, &rc_id, &queue);

            if (set_to_rdd)
                rc = rc ? rc : f_rdd_qos_to_queue_set((uint8_t)vport, rdpa_dir_ds, qos, queue, 0, 0);
        }
        else if (rdpa_if_is_wan(port))
        {
#if defined(BCM_PON_XRDP)
            bdmf_link_handle us_link = NULL;
            bdmf_number tc_tbl;
#endif
            if ((rdpa_wan_if_to_wan_type(port) != rdpa_wan_gbe) && !rdpa_is_car_mode() && !rdpa_is_epon_ae_mode())
            {
                BDMF_TRACE_RET_OBJ(BDMF_ERR_PARM, mo,
                    "upstream table can be linked to port object only in GBE or CAR mode or AE mode\n");
            }
#if defined(BCM_PON_XRDP)
            while ((us_link = bdmf_get_next_us_link(obj, us_link)))
            {
                if (bdmf_us_link_to_object(us_link)->drv == rdpa_tc_to_queue_drv())
                {
                    rdpa_tc_to_queue_table_get(bdmf_us_link_to_object(us_link), &tc_tbl);
                    break;
                }
            }
#endif
            if ((rdpa_wan_if_to_wan_type(port) == rdpa_wan_gbe) || rdpa_is_epon_ae_mode())
            {
                if (link)
                    rc = _rdpa_egress_tm_channel_queue_to_rdd(rdpa_dir_us, port, queue_id, &rc_id, &queue);

                if (set_to_rdd) 
                {
                    vport = rdpa_port_rdpa_if_to_vport(port);
#if defined(CONFIG_MULTI_WAN_SUPPORT) && defined(BCM_PON_XRDP)
                    /* for DUAL one gbe wan1 port cannot use flow_id 0 used by pon so it "takes" entry from DS instead*/
                    if (rdpa_wan_if_to_wan_type(port) == rdpa_wan_gbe)
                        rc = rc ? rc : f_rdd_qos_to_queue_set((uint8_t)vport, rdpa_dir_ds, (uint8_t)qos, queue, 0, 0);
                    else  
                        rc = rc ? rc : f_rdd_qos_to_queue_set((uint8_t)vport, rdpa_dir_us, (uint8_t)qos, queue, 0, 0);
#else
                    rc = rc ? rc : f_rdd_qos_to_queue_set((uint8_t)vport, rdpa_dir_us, qos, queue, 0, 0);
#endif
                    BDMF_TRACE_INFO("vport=%d, queue_id=%d : [rc_id=%d, queue=%d]\n",
                        (int)vport, (int)queue_id, rc_id, (int)queue);
                }
            }
            else if (rdpa_is_car_mode())
            {
                if (rdpa_is_gpon_or_xgpon_mode())
                {
                    /* Traverse configured gems and assign the table */ 
                    rc = _rdpa_qos_link_tcont_to_qos(mo, set_to_rdd, obj, qos, queue_id, link, f_rdd_qos_to_queue_set, tbl_idx);
                }
                else if (rdpa_is_epon_or_xepon_mode())
                {
                    while ((llid_obj = bdmf_get_next(rdpa_llid_drv(), llid_obj, NULL)) && !rc)
                    {
                        rc = _rdpa_qos_link_single_llid_to_qos(mo, set_to_rdd, llid_obj, qos, queue_id, link, f_rdd_qos_to_queue_set, tbl_idx);
                        if (rc)
                        {
                            /*release llid ref counter*/
                            bdmf_put(llid_obj);
                            break;
                        }
                    }
                }
            }
        }
    }
    else if (obj->drv == rdpa_tcont_drv())
    {
        rc = _rdpa_qos_link_tcont_to_qos(mo, set_to_rdd, obj, qos, queue_id, link, f_rdd_qos_to_queue_set, tbl_idx);
    }
    else if (obj->drv == rdpa_llid_drv())
    {
        rc = _rdpa_qos_link_single_llid_to_qos(mo, set_to_rdd, obj, qos, queue_id, link, f_rdd_qos_to_queue_set, tbl_idx);
    }

    if (rc)
    {
        BDMF_TRACE_RET_OBJ(BDMF_ERR_INTERNAL, mo, "can't set table index %d qos %d to queue %d. err %d\n",
            tbl_idx, (int)qos, (int)queue_id, rc);
    }

    return BDMF_ERR_OK; 
}

/*******************************************************************************/
/* rdpa_qos_to_queue_unlink_port_ex                                            */
/*                                                                             */
/* Abstruct:                                                                   */
/* This function is for unlinking TC or PBIT to queue table in rdd             */
/*                                                                             */
/* Input:                                                                      */
/*   port  - number of port/tcont                                              */
/*   other - pointer to link object - (tcont,port,llid)                        */
/*   is_pbit - 0 tc_to_queue, 1 pbit_to_queue                                  */ 
/*   tbl_size - table size to unlink - ussually 8                              */
/* Output:                                                                     */
/*   none                                                                      */  
/*******************************************************************************/
 
void rdpa_qos_to_queue_unlink_other_ex(struct bdmf_object *other, uint8_t tbl_size, bdmf_boolean is_pbit)
{ 
    bdmf_object_handle gem = NULL, llid_obj = NULL;
    rdpa_gem_flow_us_cfg_t gem_us_cfg;
    bdmf_number flow_idx;
    rdpa_if port_priv;
    rdd_vport_id_t vport;

    rdpa_port_index_get(other, &port_priv);

    if (other->drv == rdpa_tcont_drv() || (rdpa_is_car_mode() && rdpa_is_gpon_or_xgpon_mode() && rdpa_if_is_wan(port_priv)))
    {
        while ((gem = bdmf_get_next(rdpa_gem_drv(), gem, NULL)))
        {
            rdpa_gem_us_cfg_get(gem, &gem_us_cfg);
            /* if not car mode invalidate gem for specific tcont if car mode invalidate all gem flows */
            if ((gem_us_cfg.tcont == other && !rdpa_is_car_mode()) || rdpa_is_car_mode())
            {
                rdpa_gem_index_get(gem, &flow_idx);
                rdd_qos_mapper_invalidate_table((uint16_t)flow_idx, rdpa_dir_us, tbl_size, is_pbit);
            }
        }
    }
    else if (other->drv == rdpa_llid_drv() || (rdpa_is_car_mode() && rdpa_is_epon_or_xepon_mode() && rdpa_if_is_wan(port_priv)))
    { 
        llid_drv_priv_t *llid;
        if (!rdpa_is_car_mode())
        {
            llid = (llid_drv_priv_t *)bdmf_obj_data(other);
            rdd_qos_mapper_invalidate_table((uint8_t)llid->index, rdpa_dir_us, tbl_size, is_pbit);
        }
        else
        {
            while ((llid_obj = bdmf_get_next(rdpa_llid_drv(), llid_obj, NULL)))
            {
                llid = (llid_drv_priv_t *)bdmf_obj_data(llid_obj);
                rdd_qos_mapper_invalidate_table((uint8_t)llid->index, rdpa_dir_us, tbl_size, is_pbit);
            }
        }
    }
    else
    {
        if (rdpa_if_is_cpu_port(port_priv))
        {
            BDMF_TRACE_INFO("Cannot link QOS table to WIFI or CPU port %d\n", (int)port_priv);	
            return;
        }
        if (rdpa_if_is_lan(port_priv))
        {
            vport = rdpa_port_rdpa_if_to_vport(port_priv);
            rdd_qos_mapper_invalidate_table(vport, rdpa_dir_ds, tbl_size, is_pbit);
        }
        else if (rdpa_if_is_wan(port_priv)) 
        {
            if (rdpa_is_gbe_mode() || rdpa_is_epon_ae_mode()) 
            {
               vport = rdpa_port_rdpa_if_to_vport(port_priv);
               rdd_qos_mapper_invalidate_table(vport, rdpa_dir_us, tbl_size, is_pbit);
            }
        }
    }
}
