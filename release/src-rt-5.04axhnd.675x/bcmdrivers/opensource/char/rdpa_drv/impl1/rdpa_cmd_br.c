
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
 * File Name  : rdpa_cmd_br.c
 *
 * Description: This file contains bridge configuration API.
 *
 *******************************************************************************
 */

#include <linux/module.h>
#include <linux/bcm_log.h>
#include "rdpa_types.h"
#include "rdpa_api.h"
#include "rdpa_ag_bridge.h"
#include "rdpa_drv.h"
#include "rdpa_cmd_br.h"

#define __BDMF_LOG__

#define CMD_BR_LOG_ID_RDPA_CMD_DRV BCM_LOG_ID_RDPA_CMD_DRV

#if defined(__BDMF_LOG__)
#define CMD_BR_LOG_ERROR(fmt, args...) 										\
    do {                                                            				\
        if (bdmf_global_trace_level >= bdmf_trace_level_error)      				\
            bdmf_trace("ERR: %s#%d: " fmt "\n", __FUNCTION__, __LINE__, ## args);	\
    } while(0)
#define CMD_BR_LOG_INFO(fmt, args...) 										\
    do {                                                            				\
        if (bdmf_global_trace_level >= bdmf_trace_level_info)      					\
            bdmf_trace("INF: %s#%d: " fmt "\n", __FUNCTION__, __LINE__, ## args);	\
    } while(0)
#define CMD_BR_LOG_DEBUG(fmt, args...) 										\
    do {                                                            				\
        if (bdmf_global_trace_level >= bdmf_trace_level_debug)      					\
            bdmf_trace("DBG: %s#%d: " fmt "\n", __FUNCTION__, __LINE__, ## args);	\
    } while(0)
#else
#define CMD_BR_LOG_ERROR(fmt, arg...) BCM_LOG_ERROR(fmt, arg...)
#define CMD_BR_LOG_INFO(fmt, arg...) BCM_LOG_INFO(fmt, arg...)
#define CMD_BR_LOG_DEBUG(fmt, arg...) BCM_LOG_DEBUG(fmt, arg...)
#endif


/*******************************************************************************/
/* global routines                                                             */
/*******************************************************************************/

/*******************************************************************************
 *
 * Function: rdpa_cmd_br_ioctl
 *
 * IOCTL interface to the RDPA BRIDGE API.
 *
 *******************************************************************************/
int rdpa_cmd_br_ioctl(unsigned long arg)
{
    rdpa_drv_ioctl_br_t *userBr_p = (rdpa_drv_ioctl_br_t *)arg;
    rdpa_drv_ioctl_br_t br_para;
    int ret = 0;
    bdmf_error_t rc = BDMF_ERR_OK;

    copy_from_user(&br_para, userBr_p, sizeof(rdpa_drv_ioctl_br_t));

    CMD_BR_LOG_DEBUG("RDPA BRIDGE CMD(%d)", br_para.cmd);

    bdmf_lock();

    switch (br_para.cmd)
    {
        case RDPA_IOCTL_BR_CMD_FIND_OBJ: {
            bdmf_object_handle br_obj = NULL;
 
            CMD_BR_LOG_DEBUG(
                "RDPA_IOCTL_BR_CMD_FIND_OBJ: br(%u)", br_para.br_index);

            br_para.found = FALSE;
            rc = rdpa_bridge_get(br_para.br_index, &br_obj);
            if (!rc)
            {
                br_para.found = TRUE;
                bdmf_put(br_obj);
                CMD_BR_LOG_INFO("BR(%u) exist", br_para.br_index);
            }
            else
            {
                CMD_BR_LOG_DEBUG("BR(%d) does not exist", br_para.br_index);
            }
            break;
        }
        case RDPA_IOCTL_BR_CMD_LOCAL_SWITCH_SET: {
            bdmf_object_handle br_obj = NULL;

            CMD_BR_LOG_DEBUG(
                "RDPA_IOCTL_BR_CMD_LOCAL_SWITCH_SET: br(%u) local_switch(%u)", 
                 br_para.br_index, (int)br_para.local_switch);
  
            if ((rc = rdpa_bridge_get(br_para.br_index, &br_obj)))
            {
                CMD_BR_LOG_ERROR("rdpa_bridge_get() failed: br(%u) rc(%d)", 
                    br_para.br_index, rc);
                ret = RDPA_DRV_BR_GET;
                goto ioctl_exit;
            }

            CMD_BR_LOG_DEBUG("Got BR = %p", br_obj);

            if ((rc = rdpa_bridge_local_switch_enable_set(br_obj, br_para.local_switch)))
            {
                bdmf_put(br_obj);
                CMD_BR_LOG_ERROR(
                    "rdpa_bridge_local_switch_set() failed: rc(%d)", rc);
                ret = RDPA_DRV_BR_LOCAL_SWITCH_SET;
                goto ioctl_exit;
            }

            bdmf_put(br_obj);

            CMD_BR_LOG_INFO("set hw local switch success,br(%u) mode(%u)", 
                br_para.br_index, br_para.local_switch);
            break;
        }
        default: {
            CMD_BR_LOG_ERROR("Invalid IOCTL cmd %d", br_para.cmd);
            ret = RDPA_DRV_ERROR;
        }
    }

ioctl_exit:
    if (ret) 
    {
        CMD_BR_LOG_ERROR(
            "rdpa_cmd_br_ioctl() OUT: FAILED: cmd(%u) rc(%d)", br_para.cmd, rc);
    }

    bdmf_unlock();
    copy_to_user(userBr_p, &br_para, sizeof(rdpa_drv_ioctl_br_t));

    return ret;
}


/*******************************************************************************
 *
 * Function: rdpa_cmd_br_init
 *
 * Initializes the RDPA BR API.
 *
 *******************************************************************************/
void rdpa_cmd_br_init(void)
{
    CMD_BR_LOG_DEBUG("RDPA BR INIT");
}
