
/*
* <:copyright-BRCM:2013:DUAL/GPL:standard
* 
*    Copyright (c) 2013 Broadcom 
*    All Rights Reserved
* 
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License, version 2, as published by
* the Free Software Foundation (the "GPL").
* 
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
* 
* 
* A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
* writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
* Boston, MA 02111-1307, USA.
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
         (key_data->l4_dst_port == 546) || (key_data->l4_dst_port == 547)) &&
        (key_data->ip_proto == IPPROTO_UDP))
    {
        /* DHCP, ip_proto must be UDP */
        filter_idx = RDPA_FILTER_DHCP;
    }
#if defined(XRDP_RGEN6)
    else if (((key_mask ^ (IQ_KEY_MASK_IP_PROTO | IQ_KEY_MASK_DST_PORT)) == 0) &&
        (key_data->l4_dst_port == 53) && 
        ((key_data->ip_proto == IPPROTO_UDP) || (key_data->ip_proto == IPPROTO_TCP)))
    {
        /* DNS, dst_port=53, ip_proto could be UDP or TCP */
        filter_idx = RDPA_FILTER_DNS;
    }    
    else if (((key_mask ^ (IQ_KEY_MASK_IP_PROTO | IQ_KEY_MASK_SRC_PORT)) == 0) &&
        (key_data->l4_src_port == 53) && 
        ((key_data->ip_proto == IPPROTO_UDP) || (key_data->ip_proto == IPPROTO_TCP)))
    {
        /* DNS, src_port=53, ip_proto could be UDP or TCP */
        filter_idx = RDPA_FILTER_DNS;
    }
#endif
    else
        return -EINVAL;

    return filter_idx;
}

int rdpa_iq_filter_add(void *iq_param)
{
    iq_action_t *action = &((iq_param_t *)iq_param)->action;
    bdmf_object_handle port_obj = NULL;
    rdpa_filter_ctrl_t filter_ctrl;
    int filter_idx, rc;

    filter_idx = rdpa_iq_filter_get_idx((iq_param_t *)iq_param);
    if (filter_idx < 0)
        return filter_idx;

    filter_ctrl.enabled = TRUE;
    if (action->type == IQ_ACTION_TYPE_TRAP)
        filter_ctrl.action = rdpa_forward_action_host;
    else /* if (action->type == IQ_ACTION_TYPE_DROP) */
        filter_ctrl.action = rdpa_forward_action_drop;

    /* the current implementation will add the filter on all the WAN/LAN port */
    while ((port_obj = bdmf_get_next(rdpa_port_drv(), port_obj, NULL)))
    {
            rc = rdpa_port_ingress_filter_set(port_obj, filter_idx, &filter_ctrl);
            CMD_FILTER_LOG_DEBUG("Adding filter '%u', rc=%d\n", filter_idx, rc);
    }
    return 0;
}

int rdpa_iq_filter_rem(void *iq_param)
{
    bdmf_object_handle port_obj = NULL;
    rdpa_filter_ctrl_t filter_ctrl;
    int filter_idx, rc;

    filter_idx = rdpa_iq_filter_get_idx((iq_param_t *)iq_param);
    if (filter_idx < 0)
        return filter_idx;

    filter_ctrl.enabled = FALSE;

    /* the current implementation will configure the filter on all the WAN/LAN port */
    while ((port_obj = bdmf_get_next(rdpa_port_drv(), port_obj, NULL)))
    {
        rc = rdpa_port_ingress_filter_set(port_obj, filter_idx, &filter_ctrl);
        CMD_FILTER_LOG_DEBUG("Adding filter '%u', rc=%d\n", filter_idx, rc);
    }
    return 0;
}
#endif

