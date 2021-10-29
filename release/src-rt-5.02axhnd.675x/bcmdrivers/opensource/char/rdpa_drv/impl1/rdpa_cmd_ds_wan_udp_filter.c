
/*
* <:copyright-BRCM:2014:DUAL/GPL:standard
* 
*    Copyright (c) 2014 Broadcom 
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
 * File Name  : rdpa_cmd_ds_wan_udp_filter.c
 *
 * Description: This file contains the Runner DS WAN UDP Filter IOCTL API
 *
 *******************************************************************************
 */

#include <linux/module.h>
#include <linux/bcm_log.h>
#include "rdpa_types.h"
#include "rdpa_api.h"
#include "rdpa_drv.h"
#include "rdpa_cmd_ds_wan_udp_filter.h"

#if 1
#define DS_WAN_UDP_FILTER_LOG_ERROR(fmt, args...)                       \
    do {                                                                \
        if (bdmf_global_trace_level >= bdmf_trace_level_error)          \
            bdmf_trace("ERR: %s#%d: " fmt "\n", __FUNCTION__, __LINE__, ## args); \
    } while(0)
#define DS_WAN_UDP_FILTER_LOG_INFO(fmt, args...)                        \
    do {                                                                \
        if (bdmf_global_trace_level >= bdmf_trace_level_info)           \
            bdmf_trace("INF: %s#%d: " fmt "\n", __FUNCTION__, __LINE__, ## args); \
    } while(0)
#define DS_WAN_UDP_FILTER_LOG_DEBUG(fmt, args...)                       \
    do {                                                                \
        if (bdmf_global_trace_level >= bdmf_trace_level_debug)          \
            bdmf_trace("DBG: %s#%d: " fmt "\n", __FUNCTION__, __LINE__, ## args); \
    } while(0)
#endif

/*******************************************************************************/
/* global routines                                                             */
/*******************************************************************************/

/*******************************************************************************
 *
 * Function: rdpa_cmd_ds_wan_udp_filter_ioctl
 *
 * IOCTL interface to the RDPA DS_WAN_UDP_FILTER API.
 *
 *******************************************************************************/
int rdpa_cmd_ds_wan_udp_filter_ioctl(unsigned long arg)
{
    rdpa_drv_ioctl_ds_wan_udp_filter_t *user_ds_wan_udp_filter_p = (rdpa_drv_ioctl_ds_wan_udp_filter_t *)arg;
    rdpa_drv_ioctl_ds_wan_udp_filter_t ds_wan_udp_filter;
    int ret = 0;
    bdmf_object_handle ucast_class = NULL;

    copy_from_user(&ds_wan_udp_filter, user_ds_wan_udp_filter_p, sizeof(rdpa_drv_ioctl_ds_wan_udp_filter_t));

    ret = rdpa_ucast_get(&ucast_class);
    if (ret)
    {
        DS_WAN_UDP_FILTER_LOG_ERROR("ERROR : RDPA ucast object not initialized; failed cmd = %d", ds_wan_udp_filter.cmd);
        return -1;
    }

    DS_WAN_UDP_FILTER_LOG_DEBUG("RDPA DS_WAN_UDP_FILTER CMD: %d", ds_wan_udp_filter.cmd);

    bdmf_lock();

    switch(ds_wan_udp_filter.cmd)
    {
        case RDPA_IOCTL_DS_WAN_UDP_FILTER_CMD_ADD:
        {
            rdpa_ds_wan_udp_filter_t rdpa_ds_wan_udp_filter;
            bdmf_index index = (bdmf_index)ds_wan_udp_filter.filter.index;

            rdpa_ds_wan_udp_filter.offset = ds_wan_udp_filter.filter.offset;
            rdpa_ds_wan_udp_filter.value = ds_wan_udp_filter.filter.value;
            rdpa_ds_wan_udp_filter.mask = ds_wan_udp_filter.filter.mask;
            rdpa_ds_wan_udp_filter.hits = 0;

            ret = rdpa_ucast_ds_wan_udp_filter_add(ucast_class, &index, &rdpa_ds_wan_udp_filter);
            if(ret)
            {
                DS_WAN_UDP_FILTER_LOG_ERROR("Could not rdpa_ucast_ds_wan_udp_filter_add");
            }

            ds_wan_udp_filter.filter.index = (int32_t)index;

            copy_to_user(&user_ds_wan_udp_filter_p->filter.index, &ds_wan_udp_filter.filter.index,
                         sizeof(ds_wan_udp_filter.filter.index));
        }
        break;

        case RDPA_IOCTL_DS_WAN_UDP_FILTER_CMD_DELETE:
        {
            ret = rdpa_ucast_ds_wan_udp_filter_delete(ucast_class, ds_wan_udp_filter.filter.index);
            if(ret)
            {
                DS_WAN_UDP_FILTER_LOG_ERROR("Could not rdpa_ucast_ds_wan_udp_filter_delete");
            }
        }
        break;

        case RDPA_IOCTL_DS_WAN_UDP_FILTER_CMD_GET:
        {
            rdpa_ds_wan_udp_filter_t rdpa_ds_wan_udp_filter;

            ret = rdpa_ucast_ds_wan_udp_filter_get(ucast_class, ds_wan_udp_filter.filter.index, &rdpa_ds_wan_udp_filter);
            if(ret)
            {
                DS_WAN_UDP_FILTER_LOG_ERROR("Could not rdpa_ucast_ds_wan_udp_filter_get");
            }

            ds_wan_udp_filter.filter.offset = rdpa_ds_wan_udp_filter.offset;
            ds_wan_udp_filter.filter.value = rdpa_ds_wan_udp_filter.value;
            ds_wan_udp_filter.filter.mask = rdpa_ds_wan_udp_filter.mask;
            ds_wan_udp_filter.filter.hits = rdpa_ds_wan_udp_filter.hits;

            copy_to_user(user_ds_wan_udp_filter_p, &ds_wan_udp_filter, sizeof(rdpa_drv_ioctl_ds_wan_udp_filter_t));
        }
        break;

        default:
        {
            DS_WAN_UDP_FILTER_LOG_ERROR("Invalid Command: %d", ds_wan_udp_filter.cmd);

            ret = -1;
        }
    }

    bdmf_unlock();

    bdmf_put(ucast_class);
    ucast_class = NULL;

    return ret;
}
