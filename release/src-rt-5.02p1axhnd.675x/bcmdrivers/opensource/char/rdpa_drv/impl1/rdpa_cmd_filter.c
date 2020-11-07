
/*
* <:copyright-BRCM:2013:DUAL/GPL:standard
* 
*    Copyright (c) 2013 Broadcom 
*    All Rights Reserved
* 
* Unless you and Broadcom execute a separate written software license
* agreement governing use of this software, this software is licensed
* to you under the terms of the GNU General Public License version 2
* (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
* with the following added to such license:
* 
*    As a special exception, the copyright holders of this software give
*    you permission to link this software with independent modules, and
*    to copy and distribute the resulting executable under terms of your
*    choice, provided that you also meet, for each linked independent
*    module, the terms and conditions of the license of that module.
*    An independent module is a module which is not derived from this
*    software.  The special exception does not apply to any modifications
*    of the software.
* 
* Not withstanding the above, under no circumstances may you combine
* this software in any way with any other Broadcom software provided
* under a license other than the GPL, without Broadcom's express prior
* written consent.
* 
* :>
*/

/*
 *******************************************************************************
 * File Name  : rdpa_cmd_filter.c
 *
 * Description: This file contains the filter configuration API.
 *
 *******************************************************************************
 */

#include <linux/module.h>
#include <linux/jiffies.h>
#include <linux/delay.h>
#include <linux/bcm_log.h>
#include "bcmenet.h"
#include "bcmtypes.h"
#include "bcmnet.h"
#include "rdpa_types.h"
#include "rdpa_api.h"
#include "rdpa_ag_filter.h"
#include "rdpa_drv.h"
#if (defined(CONFIG_BCM_INGQOS) || defined(CONFIG_BCM_INGQOS_MODULE))
#include <linux/iqos.h>
#include "ingqos.h"
#endif

#define __BDMF_LOG__

#define CMD_FILTER_LOG_ID_RDPA_CMD_DRV BCM_LOG_ID_RDPA_CMD_DRV

#if defined(__BDMF_LOG__)
#define CMD_FILTER_LOG_ERROR(fmt, args...) 										\
    do {                                                            				\
        if (bdmf_global_trace_level >= bdmf_trace_level_error)      				\
            bdmf_trace("ERR: %s#%d: " fmt "\n", __FUNCTION__, __LINE__, ## args);	\
    } while(0)
#define CMD_FILTER_LOG_INFO(fmt, args...) 										\
    do {                                                            				\
        if (bdmf_global_trace_level >= bdmf_trace_level_info)      					\
            bdmf_trace("INF: %s#%d: " fmt "\n", __FUNCTION__, __LINE__, ## args);	\
    } while(0)
#define CMD_FILTER_LOG_DEBUG(fmt, args...) 										\
    do {                                                            				\
        if (bdmf_global_trace_level >= bdmf_trace_level_debug)      					\
            bdmf_trace("DBG: %s#%d: " fmt "\n", __FUNCTION__, __LINE__, ## args);	\
    } while(0)
#else
#define CMD_FILTER_LOG_ERROR(fmt, arg...) BCM_LOG_ERROR(fmt, arg...)
#define CMD_FILTER_LOG_INFO(fmt, arg...) BCM_LOG_INFO(fmt, arg...)
#define CMD_FILTER_LOG_DEBUG(fmt, arg...) BCM_LOG_DEBUG(fmt, arg...)
#endif


/*******************************************************************************/
/* global routines                                                             */
/*******************************************************************************/

/*******************************************************************************
 *
 * Function: rdpa_cmd_filter_ioctl
 *
 * IOCTL interface to the RDPA FILTER API.
 *
 *******************************************************************************/
int rdpa_cmd_filter_ioctl(unsigned long arg)
{
    rdpa_drv_ioctl_filter_t *userFilter_p = (rdpa_drv_ioctl_filter_t *)arg;
    rdpa_drv_ioctl_filter_t filter;
    rdpa_filter_tpid_vals_t tpid_curr_vals = { };
    bdmf_object_handle filter_obj = NULL;
    int rc = BDMF_ERR_OK;

    copy_from_user(&filter, userFilter_p, sizeof(rdpa_drv_ioctl_filter_t));
    CMD_FILTER_LOG_DEBUG("RDPA FILTER CMD(%d)", filter.cmd);

    bdmf_lock();
    rc = rdpa_filter_get( &filter_obj );

    if (rc)
    {
        bdmf_unlock();
        return rc;
    }

    switch(filter.cmd)
    {
        case RDPA_IOCTL_FILTER_CMD_ADD_ENTRY: 
        {
            rdpa_ports ports = filter.param.key.ports;
            while (1)
            {
                rdpa_if port_idx = rdpa_port_get_next(&ports);
                bdmf_object_handle port_obj;

                if (port_idx == rdpa_if_none)
                    break;

                rc = rdpa_port_get(port_idx, &port_obj);
                if (rc)
                {
                    CMD_FILTER_LOG_DEBUG("Attemp to add filter '%u' for port '%d' failed, port not found\n",
                        filter.param.key.filter, port_idx);
                    continue;
                }
                rc = rdpa_port_ingress_filter_set(port_obj, filter.param.key.filter, &filter.param.ctrl);
                CMD_FILTER_LOG_DEBUG("Adding filter '%u' for port '%d', rc=%d\n",
                    filter.param.key.filter, port_idx, rc);
                bdmf_put(port_obj);
                if (rc)
                    break;
            }
            break;
        }

        case RDPA_IOCTL_FILTER_CMD_GLOBAL_CFG: 
        {
            rc = rdpa_filter_global_cfg_set(filter_obj, &filter.param.global_cfg);
            break;
        }

        case RDPA_IOCTL_FILTER_CMD_ETYPE_UDEF_CFG: 
        {            
            rc = rdpa_filter_etype_udef_set(filter_obj, filter.param.udef_inx, filter.param.udef_val);
            break;
        }

        case RDPA_IOCTL_FILTER_CMD_TPID_VALS_CFG: 
        {
            /* Change only the related value, read modify write */
            rc = rdpa_filter_tpid_vals_get(filter_obj, &tpid_curr_vals);
            if (rc)
            {
                /* If was not configured before configure with the default dont care value */
                tpid_curr_vals.val_ds = 0xffff;
                tpid_curr_vals.val_us = 0xffff;
            }

            /* US direction = 1 */
            if (filter.param.tpid_direction)
                filter.param.tpid_vals.val_ds = tpid_curr_vals.val_ds;
            else
                filter.param.tpid_vals.val_us = tpid_curr_vals.val_us;

            rc = rdpa_filter_tpid_vals_set(filter_obj, &filter.param.tpid_vals);
            break;
        }

        case RDPA_IOCTL_FILTER_CMD_OUI_CFG: 
        {
            rc = rdpa_filter_oui_val_set(filter_obj, &filter.param.oui_val_key, filter.param.oui_val);
            break;
        }

        case RDPA_IOCTL_FILTER_CMD_GET_STAT: 
        {
            rc = rdpa_filter_stats_get(filter_obj, &filter.param.stats_params, &filter.param.stats_val);
            if (!rc)
            {
                copy_to_user((rdpa_drv_ioctl_filter_t *)arg, &filter, sizeof(rdpa_drv_ioctl_filter_t));
            }
            break;
        }

        default:
            CMD_FILTER_LOG_ERROR("Invalid IOCTL cmd %d", filter.cmd);
            rc = RDPA_DRV_ERROR;
    }

    bdmf_put(filter_obj);
    if (rc) {
        CMD_FILTER_LOG_ERROR("rdpa_cmd_filter_ioctl() OUT: FAILED: rc(%d)", rc);
    }
    bdmf_unlock();

    return rc;
}

#if (defined(CONFIG_BCM_INGQOS) || defined(CONFIG_BCM_INGQOS_MODULE))
static int rdpa_iq_filter_get_idx(iq_param_t *iq_param)
{
    uint32_t key_mask = iq_param->key_mask;
    iq_key_data_t *key_data = &iq_param->key_data;
    int filter_idx = 0;

    if ((key_mask ^ IQ_KEY_MASK_ETHER_TYPE) == 0)
    {
        switch (key_data->eth_type) {
        case ETH_P_ARP:
            filter_idx = RDPA_FILTER_ETYPE_ARP;
            break;
        case ETH_P_PPP_DISC:
            filter_idx = RDPA_FILTER_ETYPE_PPPOE_D;
            break;
        case ETH_P_PPP_SES:
            filter_idx = RDPA_FILTER_ETYPE_PPPOE_S;
            break;
        case ETH_P_8021AG:
            filter_idx = RDPA_FILTER_ETYPE_802_1AG_CFM;
            break;
        case ETH_P_1588:
            filter_idx = RDPA_FILTER_ETYPE_PTP_1588;
            break;
        case ETH_P_PAE:
            filter_idx = RDPA_FILTER_ETYPE_802_1X;
            break;
        default:
            /* Only Pre-defined EthType Filters otherwise handled by IC */
            CMD_FILTER_LOG_DEBUG("unsupported EtherType = 0x%04x",key_data->eth_type);
            return -EINVAL;
        }
    }
    else if (((key_mask ^ (IQ_KEY_MASK_IP_PROTO | IQ_KEY_MASK_DST_PORT)) == 0) &&
        ((key_data->l4_dst_port == 67) || (key_data->l4_dst_port == 68) ||
         (key_data->l4_dst_port == 546) || (key_data->l4_dst_port == 547)))
    {
        /* DHCP */
        filter_idx = RDPA_FILTER_DHCP;
    }
    else
        return -EINVAL;

    return filter_idx;
}

int rdpa_iq_filter_add(void *iq_param)
{
    iq_action_t *action = &((iq_param_t *)iq_param)->action;
    bdmf_object_handle port_obj;
    rdpa_filter_ctrl_t filter_ctrl;
    int i, filter_idx, rc;

    filter_idx = rdpa_iq_filter_get_idx((iq_param_t *)iq_param);
    if (filter_idx < 0)
        return filter_idx;

    filter_ctrl.enabled = TRUE;
    if (action->type == IQ_ACTION_TYPE_TRAP)
        filter_ctrl.action = rdpa_forward_action_host;
    else /* if (action->type == IQ_ACTION_TYPE_DROP) */
        filter_ctrl.action = rdpa_forward_action_drop;

    /* the current implementation will add the filter on all the WAN/LAN port */
    for (i = rdpa_if_first; i <= rdpa_if_lan_max; i++)
    {
        rc = rdpa_port_get(i, &port_obj);
        if (!rc)
        {
            rc = rdpa_port_ingress_filter_set(port_obj, filter_idx, &filter_ctrl);
            CMD_FILTER_LOG_DEBUG("Adding filter '%u' for port '%d', rc=%d\n",
                filter_idx, i, rc);
            bdmf_put(port_obj);
        }
    }

    return 0;
}

int rdpa_iq_filter_rem(void *iq_param)
{
    bdmf_object_handle port_obj;
    rdpa_filter_ctrl_t filter_ctrl;
    int i, filter_idx, rc;

    filter_idx = rdpa_iq_filter_get_idx((iq_param_t *)iq_param);
    if (filter_idx < 0)
        return filter_idx;

    filter_ctrl.enabled = FALSE;

    /* the current implementation will configure the filter on all the WAN/LAN port */
    for (i = rdpa_if_first; i <= rdpa_if_lan_max; i++)
    {
        rc = rdpa_port_get(i, &port_obj);
        if (!rc)
        {
            rc = rdpa_port_ingress_filter_set(port_obj, filter_idx, &filter_ctrl);
            CMD_FILTER_LOG_DEBUG("Adding filter '%u' for port '%d', rc=%d\n",
                filter_idx, i, rc);
            bdmf_put(port_obj);
        }
    }

    return 0;
}
#endif

