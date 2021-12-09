
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
#include "bcmenet.h"
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
    rdpa_port_dp_cfg_t cfg;
    rdpa_port_sa_limit_t sa_limit;
    int rc = BDMF_ERR_OK;

    copy_from_user(&port, userPort_p, sizeof(rdpa_drv_ioctl_port_t));

    CMD_PORT_LOG_DEBUG("RDPA PORT CMD(%d)", port.cmd);

    bdmf_lock();

    rc = rdpa_port_get(port.port_idx, &port_obj);
    if (rc)
    {
        bdmf_unlock();
        return rc;
    }

    switch (port.cmd)
    {
        case RDPA_IOCTL_PORT_CMD_SA_LIMIT_GET:
        {
            rc = rdpa_port_sa_limit_get(port_obj, &sa_limit);
            if (!rc)
            {
                port.param.sa_limit.max_sa = sa_limit.max_sa;
                port.param.sa_limit.num_sa = sa_limit.num_sa;
                copy_to_user((rdpa_drv_ioctl_port_t *)arg, &port, sizeof(rdpa_drv_ioctl_port_t));
            }
            break;
        }

        case RDPA_IOCTL_PORT_CMD_SA_LIMIT_SET:
        {
            rc = rdpa_port_sa_limit_get(port_obj, &sa_limit);
            if (!rc)
            {
                sa_limit.max_sa = port.param.sa_limit.max_sa;
                rc = rdpa_port_sa_limit_set(port_obj, &sa_limit);
            }
            break;
        }

        case RDPA_IOCTL_PORT_CMD_PARAM_GET:
        {
            rc = rdpa_port_cfg_get(port_obj, &cfg);
            if (!rc)
            {
                port.param.sal_miss_action = cfg.sal_miss_action;
                port.param.dal_miss_action = cfg.dal_miss_action;
                port.param.sal_enable = cfg.sal_enable;
                port.param.dal_enable = cfg.dal_enable;

                copy_to_user((rdpa_drv_ioctl_port_t *)arg, &port, sizeof(rdpa_drv_ioctl_port_t));
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

                if (rdpa_if_is_wan(port.port_idx))
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
