
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
 * File Name  : rdpa_cmd_port.c
 *
 * Description: This file contains the port configuration API.
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
#include "rdpa_ag_port.h"
#include "rdpa_drv.h"

#define __BDMF_LOG__

#define CMD_PORT_LOG_ID_RDPA_CMD_DRV BCM_LOG_ID_RDPA_CMD_DRV

#if defined(__BDMF_LOG__)
#define CMD_PORT_LOG_ERROR(fmt, args...) 										\
    do {                                                            				\
        if (bdmf_global_trace_level >= bdmf_trace_level_error)      				\
            bdmf_trace("ERR: %s#%d: " fmt "\n", __FUNCTION__, __LINE__, ## args);	\
    } while(0)
#define CMD_PORT_LOG_INFO(fmt, args...) 										\
    do {                                                            				\
        if (bdmf_global_trace_level >= bdmf_trace_level_info)      					\
            bdmf_trace("INF: %s#%d: " fmt "\n", __FUNCTION__, __LINE__, ## args);	\
    } while(0)
#define CMD_PORT_LOG_DEBUG(fmt, args...) 										\
    do {                                                            				\
        if (bdmf_global_trace_level >= bdmf_trace_level_debug)      					\
            bdmf_trace("DBG: %s#%d: " fmt "\n", __FUNCTION__, __LINE__, ## args);	\
    } while(0)
#else
#define CMD_PORT_LOG_ERROR(fmt, arg...) BCM_LOG_ERROR(fmt, arg...)
#define CMD_PORT_LOG_INFO(fmt, arg...) BCM_LOG_INFO(fmt, arg...)
#define CMD_SYS_LOG_DEBUG(fmt, arg...) BCM_LOG_DEBUG(fmt, arg...)
#endif


/*******************************************************************************/
/* global routines                                                             */
/*******************************************************************************/

/*******************************************************************************
 *
 * Function: rdpa_cmd_port_ioctl
 *
 * IOCTL interface to the RDPA PORT API.
 *
 *******************************************************************************/
int rdpa_cmd_port_ioctl(unsigned long arg)
{
    rdpa_drv_ioctl_port_t *userPort_p = (rdpa_drv_ioctl_port_t *)arg;
    rdpa_drv_ioctl_port_t port;
    bdmf_object_handle port_obj = NULL;
    bdmf_boolean is_wan;
    rdpa_port_dp_cfg_t cfg;
    int rc = BDMF_ERR_OK;

    if(copy_from_user(&port, userPort_p, sizeof(rdpa_drv_ioctl_port_t)))
    {
        CMD_PORT_LOG_ERROR("copy_from_user failed\n"); 
        return RDPA_DRV_ERROR;
    }

    CMD_PORT_LOG_DEBUG("RDPA PORT CMD(%d)", port.cmd);

    bdmf_lock();

    rc = rdpa_port_get(port.ifname, &port_obj);
    if (rc)
    {
        bdmf_unlock();
        return rc;
    }

    switch (port.cmd)
    {
        case RDPA_IOCTL_PORT_CMD_PARAM_GET:
        {
            rc = rdpa_port_cfg_get(port_obj, &cfg);
            if (!rc)
            {
                port.param.sal_miss_action = cfg.sal_miss_action;
                port.param.dal_miss_action = cfg.dal_miss_action;
                port.param.sal_enable = cfg.sal_enable;
                port.param.dal_enable = cfg.dal_enable;

                if(copy_to_user((rdpa_drv_ioctl_port_t *)arg, &port, sizeof(rdpa_drv_ioctl_port_t)))
                {
                    CMD_PORT_LOG_ERROR("copy_to_user failed\n"); 
                    rc = RDPA_DRV_ERROR;
                }
            }
            break;
        }

        case RDPA_IOCTL_PORT_CMD_PARAM_SET:
        {
            rc = rdpa_port_cfg_get(port_obj, &cfg);
            if (!rc)
            {
                cfg.dal_miss_action = port.param.dal_miss_action;
                cfg.sal_miss_action = port.param.sal_miss_action;
                cfg.sal_enable = port.param.sal_enable;
                cfg.dal_enable = port.param.dal_enable;
#ifdef XRDP
                if (rdpa_forward_action_forward == port.param.sal_miss_action)
                {
                    cfg.sal_enable = 0;
                }
                else
                {
                    cfg.sal_enable = 1;
                }
                rc = rdpa_port_is_wan_get(port_obj, &is_wan);
                if (!rc)
                {
                    if (is_wan)
                    {
                        if (rdpa_forward_action_forward == port.param.dal_miss_action)
                        {
                            cfg.dal_enable = 0;
                        }
                        else
                        {
                            cfg.dal_enable = 1;
                        }
                    }
                }
                else
                {
                    cfg.dal_enable = 1;
                }
#endif                
                rc = rdpa_port_cfg_set(port_obj, &cfg);
            }
            break;
        }

        default:
            CMD_PORT_LOG_ERROR("Invalid IOCTL cmd %d", port.cmd);
            rc = RDPA_DRV_ERROR;
    }

    if (port_obj)
    {
        bdmf_put(port_obj);
    }

    if (rc)
    {
        CMD_PORT_LOG_ERROR("rdpa_cmd_port_ioctl() OUT: FAILED: rc(%d)", rc);
    }

    bdmf_unlock();
    return rc;
}

/*******************************************************************************
 *
 * Function: rdpa_cmd_PORT_init
 *
 * Initializes the RDPA IC API.
 *
 *******************************************************************************/
void rdpa_cmd_port_init(void)
{
    CMD_PORT_LOG_DEBUG("RDPA PORT INIT");
}
