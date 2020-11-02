
/*
* <:copyright-BRCM:2015:DUAL/GPL:standard
* 
*    Copyright (c) 2015 Broadcom 
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
 * File Name  : rdpa_cmd_dscp_to_pbit.c
 *
 * Description: This file contains the RDPA DSCP TO PBIT API.
 *
 *******************************************************************************
 */

#include <linux/module.h>
#include <linux/bcm_log.h>
#include "bcmtypes.h"
#include "rdpa_types.h"
#include "rdpa_api.h"
#include "rdpa_drv.h"
#include "rdpa_cmd_dscp_to_pbit.h"

#define __BDMF_LOG__

#define CMD_D_TO_P_LOG_ID_RDPA_CMD_DRV BCM_LOG_ID_RDPA_CMD_DRV

#if defined(__BDMF_LOG__)
#define CMD_D_TO_P_LOG_ERROR(fmt, args...) 										\
    do {                                                            				\
        if (bdmf_global_trace_level >= bdmf_trace_level_error)      				\
            bdmf_trace("ERR: %s#%d: " fmt "\n", __FUNCTION__, __LINE__, ## args);	\
    } while(0)
#define CMD_D_TO_P_LOG_INFO(fmt, args...) 										\
    do {                                                            				\
        if (bdmf_global_trace_level >= bdmf_trace_level_info)      					\
            bdmf_trace("INF: %s#%d: " fmt "\n", __FUNCTION__, __LINE__, ## args);	\
    } while(0)
#define CMD_D_TO_P_LOG_DEBUG(fmt, args...) 										\
    do {                                                            				\
        if (bdmf_global_trace_level >= bdmf_trace_level_debug)      					\
            bdmf_trace("DBG: %s#%d: " fmt "\n", __FUNCTION__, __LINE__, ## args);	\
    } while(0)
#else
#define CMD_D_TO_P_LOG_ERROR(fmt, arg...) BCM_LOG_ERROR(fmt, arg...)
#define CMD_D_TO_P_LOG_INFO(fmt, arg...) BCM_LOG_INFO(fmt, arg...)
#define CMD_D_TO_P_LOG_DEBUG(fmt, arg...) BCM_LOG_DEBUG(fmt, arg...)
#endif


/*******************************************************************************/
/* global routines                                                             */
/*******************************************************************************/


/*******************************************************************************
 *
 * Function: rdpa_cmd_dscp_to_pbit_ioctl
 *
 * IOCTL interface to the RDPA DSCP TO PBIT API.
 *
 *******************************************************************************/
int rdpa_cmd_dscp_to_pbit_ioctl(unsigned long arg)
{
    rdpa_drv_ioctl_dscp_to_pbit_t *userDtoP_p = (rdpa_drv_ioctl_dscp_to_pbit_t *)arg;
    rdpa_drv_ioctl_dscp_to_pbit_t dscp_to_pbit;
    bdmf_object_handle d_to_p_obj = NULL;
    int tbl_id;
    int ret = 0;
    int rc = BDMF_ERR_OK;

    copy_from_user(&dscp_to_pbit, userDtoP_p, sizeof(rdpa_drv_ioctl_dscp_to_pbit_t));

    CMD_D_TO_P_LOG_DEBUG("RDPA DSCP TO PBIT CMD(%d)", dscp_to_pbit.cmd);

    bdmf_lock();

    switch(dscp_to_pbit.cmd)
    {
        case RDPA_IOCTL_D_TO_P_CMD_GET:
        {
            int i;
            BOOL qos_mapping;
            uint8_t dscp_map;
 
            CMD_D_TO_P_LOG_DEBUG(
                            "RDPA_IOCTL_D_TO_P_CMD_GET");

            dscp_to_pbit.found = FALSE;
            
            for (tbl_id = 0; tbl_id < RDPA_DSCP_TO_PBIT_MAX_TABLES; tbl_id++)
            {
                qos_mapping = FALSE;

                rc = rdpa_dscp_to_pbit_get(tbl_id, &d_to_p_obj);
                if (rc)
                    continue;
                
                rc = 
                    rdpa_dscp_to_pbit_qos_mapping_get(d_to_p_obj, &qos_mapping);
                if (rc)
                {
                    CMD_D_TO_P_LOG_ERROR("rdpa_dscp_to_pbit_qos_mapping_get()" \
                        " failed: table(%u) rc(%d)", tbl_id, rc);
                    ret = RDPA_DRV_D_TO_P_QOS_MAP_GET;
                    bdmf_put(d_to_p_obj);
                    goto d_to_p_ioctl_exit;
                }

                if (qos_mapping)
                {
                    dscp_to_pbit.found = TRUE;
                    break;
                }
               
                bdmf_put(d_to_p_obj);
                d_to_p_obj = NULL;
            }

            if (!dscp_to_pbit.found)
                goto d_to_p_ioctl_exit;
            
            for (i = 0; i < 64; i++)
            {
                rc = rdpa_dscp_to_pbit_dscp_map_get(d_to_p_obj, i, &dscp_map);
                
                if (rc)
                {
                    CMD_D_TO_P_LOG_ERROR("rdpa_dscp_to_pbit_dscp_map_get()" \
                        " failed: table(%u) dscp(%u) rc(%d)", tbl_id, i, rc);
                    ret = RDPA_DRV_D_TO_P_QOS_MAP_GET;
                    break;
                }
                else
                {
                    dscp_to_pbit.dscp_pbit_map[i]= dscp_map;
                }
            }
              
            bdmf_put(d_to_p_obj);
         
            break;
        }
        
        case RDPA_IOCTL_D_TO_P_CMD_SET:
        {
            int i;
            BOOL qos_mapping;
 
            CMD_D_TO_P_LOG_DEBUG(
                            "RDPA_IOCTL_D_TO_P_CMD_SET");

            dscp_to_pbit.found = FALSE;
            
            for (tbl_id = 0; tbl_id < RDPA_DSCP_TO_PBIT_MAX_TABLES; tbl_id++)
            {
                qos_mapping = FALSE;

                rc = rdpa_dscp_to_pbit_get(tbl_id, &d_to_p_obj);
                if (rc)
                    continue;
                
                rc = 
                    rdpa_dscp_to_pbit_qos_mapping_get(d_to_p_obj, &qos_mapping);
                if (rc)
                {
                    CMD_D_TO_P_LOG_ERROR("rdpa_dscp_to_pbit_qos_mapping_get()" \
                        " failed: table(%u) rc(%d)", tbl_id, rc);
                    ret = RDPA_DRV_D_TO_P_QOS_MAP_GET;
                    bdmf_put(d_to_p_obj);
                    goto d_to_p_ioctl_exit;
                }

                if (qos_mapping)
                {
                    dscp_to_pbit.found = TRUE;
                    break;
                }
               
                bdmf_put(d_to_p_obj);
                d_to_p_obj = NULL;
            }

            if (!dscp_to_pbit.found)
            {
                BDMF_MATTR(dscp_to_pbit_attrs, rdpa_dscp_to_pbit_drv());
                rdpa_dscp_to_pbit_qos_mapping_set(dscp_to_pbit_attrs, TRUE);

                rc = bdmf_new_and_set(
                    rdpa_dscp_to_pbit_drv(), NULL, dscp_to_pbit_attrs, &d_to_p_obj);
                if (rc || (d_to_p_obj == NULL))
                {
                    CMD_D_TO_P_LOG_ERROR(
                        "bdmf_new_and_set() failed: rc(%d)", rc);
                    ret = RDPA_DRV_NEW_D_TO_P_ALLOC;
                    goto d_to_p_ioctl_exit;
                }
            }

            for (i = 0; i < 64; i++)
            {
                rc = rdpa_dscp_to_pbit_dscp_map_set(
                    d_to_p_obj, i, dscp_to_pbit.dscp_pbit_map[i]);
                
                if (rc)
                {
                    CMD_D_TO_P_LOG_ERROR("rdpa_dscp_to_pbit_dscp_map_set()" \
                        " failed: dscp(%u) pbit(%u) rc(%d)", \
                        i, dscp_to_pbit.dscp_pbit_map[i], rc);
                    ret = RDPA_DRV_D_TO_P_QOS_MAP_GET;
                    break;
                }
            }

            if (dscp_to_pbit.found)
                bdmf_put(d_to_p_obj);
            
            break;
        }

        default:
            CMD_D_TO_P_LOG_ERROR("Invalid IOCTL cmd %d", dscp_to_pbit.cmd);
            rc = RDPA_DRV_ERROR;
    }

    if (rc) {
        CMD_D_TO_P_LOG_ERROR("rdpa_cmd_dscp_to_pbit_ioctl() OUT: FAILED: rc(%d)", rc);
    }

d_to_p_ioctl_exit:

    copy_to_user(
        userDtoP_p, &dscp_to_pbit, sizeof(rdpa_drv_ioctl_dscp_to_pbit_t));
    
    if (ret) 
    {
        CMD_D_TO_P_LOG_ERROR(
            "rdpa_cmd_dscp_to_pbit_ioctl() OUT: FAILED: cmd(%u) rc(%d)", dscp_to_pbit.cmd, rc);
    }

    bdmf_unlock();
    return ret;
}

/*******************************************************************************
 *
 * Function: rdpa_cmd_dscp_to_pbit_init
 *
 * Initializes the RDPA DSCP TO PBIT API.
 *
 *******************************************************************************/
void rdpa_cmd_dscp_to_pbit_init(void)
{
    CMD_D_TO_P_LOG_DEBUG("RDPA DSCP TO PBIT INIT");
}

EXPORT_SYMBOL(rdpa_cmd_dscp_to_pbit_ioctl);
EXPORT_SYMBOL(rdpa_cmd_dscp_to_pbit_init);

