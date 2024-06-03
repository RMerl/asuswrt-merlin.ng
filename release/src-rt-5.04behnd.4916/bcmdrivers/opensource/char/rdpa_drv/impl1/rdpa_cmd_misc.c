
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
 * File Name  : rdpa_cmd_misc.c
 *
 * Description: This file contains the RDPA MISCELLANEOUS API.
 *
 *******************************************************************************
 */

#include <linux/module.h>
#include <linux/bcm_log.h>
#include "bcmtypes.h"
#include "rdpa_types.h"
#include "rdpa_api.h"
#include "rdpa_drv.h"
#include "rdpa_cmd_misc.h"
#include "rdpa_mw_qos.h"
#include "bcm_rsvmem.h"


#define __BDMF_LOG__

#define CMD_MISC_LOG_ID_RDPA_CMD_DRV BCM_LOG_ID_RDPA_CMD_DRV

#if defined(__BDMF_LOG__)
#define CMD_MISC_LOG_ERROR(fmt, args...) 										\
    do {                                                            				\
        if (bdmf_global_trace_level >= bdmf_trace_level_error)      				\
            bdmf_trace("ERR: %s#%d: " fmt "\n", __FUNCTION__, __LINE__, ## args);	\
    } while(0)
#define CMD_MISC_LOG_INFO(fmt, args...) 										\
    do {                                                            				\
        if (bdmf_global_trace_level >= bdmf_trace_level_info)      					\
            bdmf_trace("INF: %s#%d: " fmt "\n", __FUNCTION__, __LINE__, ## args);	\
    } while(0)
#define CMD_MISC_LOG_DEBUG(fmt, args...) 										\
    do {                                                            				\
        if (bdmf_global_trace_level >= bdmf_trace_level_debug)      					\
            bdmf_trace("DBG: %s#%d: " fmt "\n", __FUNCTION__, __LINE__, ## args);	\
    } while(0)
#else
#define CMD_MISC_LOG_ERROR(fmt, arg...) BCM_LOG_ERROR(fmt, arg...)
#define CMD_MISC_LOG_INFO(fmt, arg...) BCM_LOG_INFO(fmt, arg...)
#define CMD_MISC_LOG_DEBUG(fmt, arg...) BCM_LOG_DEBUG(fmt, arg...)
#endif

#define ONE_MB (1024 * 1024)
extern int BcmMemReserveGetByName(char *name, void **virt_addr, phys_addr_t* phys_addr, unsigned int *size);
static int get_tm_memory_info(rdpa_drv_ioctl_misc_t *misc)
{
    void *fpm_pool_addr;
    uint32_t fpm_pool_size;
    phys_addr_t fpm_pool_phys_addr;
    /* fetch the reserved-memory from device tree */
    if (BcmMemReserveGetByName(FPMPOOL_BASE_ADDR_STR, &fpm_pool_addr, &fpm_pool_phys_addr, &fpm_pool_size))
    {
        return RDPA_DRV_ERROR;
    }
    misc->fpm_pool_memory_size = fpm_pool_size / ONE_MB;
    return 0;
}

/*******************************************************************************/
/* global routines                                                             */
/*******************************************************************************/


/*******************************************************************************
 *
 * Function: rdpa_cmd_misc_ioctl
 *
 * IOCTL interface to the RDPA MISCELLANEOUS API.
 *
 *******************************************************************************/
int rdpa_cmd_misc_ioctl(unsigned long arg)
{
    rdpa_drv_ioctl_misc_t *userMisc_p = (rdpa_drv_ioctl_misc_t *)arg;
    rdpa_drv_ioctl_misc_t misc_cfg;
    int ret = 0;
    int rc = BDMF_ERR_OK;
    
    if(copy_from_user(&misc_cfg, userMisc_p, sizeof(rdpa_drv_ioctl_misc_t)))
    {
        pr_debug("%s:%d: copy_from_user failed", __FUNCTION__, __LINE__ );
        return RDPA_DRV_ERROR;
    }

    CMD_MISC_LOG_DEBUG("RDPA MISC CMD(%d)", misc_cfg.cmd);

    switch(misc_cfg.cmd)
    {
        case RDPA_IOCTL_MISC_CMD_PKT_BASED_QOS_GET:
        {
            BOOL enable;
            
            CMD_MISC_LOG_DEBUG(
                "RDPA_IOCTL_MISC_CMD_PKT_BASED_QOS_GET dir=%d type=%d", \
                misc_cfg.dir, misc_cfg.type);

            bdmf_lock();
            rc = rdpa_mw_pkt_based_qos_get(
                misc_cfg.dir, misc_cfg.type, &enable);
            bdmf_unlock();

            if (rc != 0)
            {
                CMD_MISC_LOG_ERROR("rdpa_mw_pkt_based_qos_get() failed:" \
                    " dir(%u) type(%d) rc(%d)", \
                    misc_cfg.dir, misc_cfg.type, rc);
                ret = RDPA_DRV_PKT_BASED_QOS_GET;
            }
            else
            {
                misc_cfg.enable = enable;
                if(copy_to_user(
                    userMisc_p, &misc_cfg, sizeof(rdpa_drv_ioctl_misc_t)))
                {
                    pr_debug("%s:%d: copy_to_user failed", __FUNCTION__, __LINE__ );
                    return RDPA_DRV_ERROR;
                }
            }

            break;
        }
        
        case RDPA_IOCTL_MISC_CMD_PKT_BASED_QOS_SET:
        {
            CMD_MISC_LOG_DEBUG(
                "RDPA_IOCTL_MISC_CMD_PKT_BASED_QOS_SET dir=%d type=%d en=%d", \
                misc_cfg.dir, misc_cfg.type, misc_cfg.enable);

            bdmf_lock();            
            rc = rdpa_mw_pkt_based_qos_set(
                misc_cfg.dir, misc_cfg.type, &misc_cfg.enable);
            bdmf_unlock();
            
            if (rc != 0)
            {
                CMD_MISC_LOG_ERROR("rdpa_mw_pkt_based_qos_set() failed:" \
                    " dir(%u) type(%d) en(%d) rc(%d)", \
                    misc_cfg.dir, misc_cfg.type, misc_cfg.enable, rc);
                ret = RDPA_DRV_PKT_BASED_QOS_SET;
            }
            
            break;
        }

        case RDPA_IOCTL_MISC_CMD_GET_TM_MEMORY_INFO: 
        {
            bdmf_lock();
            ret = get_tm_memory_info(&misc_cfg);
            bdmf_unlock();

            if (ret)
            {
                CMD_MISC_LOG_ERROR("get_tm_memory_info() failed: rc(%d)", ret);
            }
            if(copy_to_user(
                    userMisc_p, &misc_cfg, sizeof(rdpa_drv_ioctl_misc_t)))
            {
                pr_debug("%s:%d: copy_to_user failed", __FUNCTION__, __LINE__ );
                rc = RDPA_DRV_ERROR;
            }
            break;
        }
 
        default:
            CMD_MISC_LOG_ERROR("Invalid IOCTL cmd %d", misc_cfg.cmd);
            rc = RDPA_DRV_ERROR;
    }
    
    if (ret) 
    {
        CMD_MISC_LOG_ERROR(
            "rdpa_cmd_misc_ioctl() OUT: FAILED: cmd(%u) ret(%d)", \
            misc_cfg.cmd, ret);
    }

    return ret;
}

/*******************************************************************************
 *
 * Function: rdpa_cmd_misc_init
 *
 * Initializes the RDPA MISCELLANEOUS API.
 *
 *******************************************************************************/
void rdpa_cmd_misc_init(void)
{
    CMD_MISC_LOG_DEBUG("RDPA MISC INIT");
}

/*******************************************************************************
 *
 * Function: get_rdpa_epon_port_obj
 *
 *
 * Return epon port object
 *
 *******************************************************************************/
bdmf_object_handle get_rdpa_epon_port_obj(void)
{
    bdmf_object_handle port_obj = NULL;
    rdpa_port_type port_type = rdpa_port_type_none;
    int rc;

    while (1)
    {
        port_obj = bdmf_get_next(rdpa_port_drv(), port_obj, NULL);
        if (!port_obj)
            break;
       
        rc = rdpa_port_type_get(port_obj, &port_type);
        if (rc)
            continue;
        if ((port_type == rdpa_port_epon) || (port_type == rdpa_port_xepon))
            return port_obj;
    }
    return NULL;
}

/*******************************************************************************
 *
 * Function: is_ae_enable 
 * 
 *
 * Return true if active ethrnet is enable
 *
 *******************************************************************************/

int is_ae_enable(void)
{
    bdmf_object_handle port_obj = NULL;
    rdpa_port_type port_type = rdpa_port_type_none;    
    bdmf_boolean is_wan = FALSE;
    int rc = 0;

    while (1)
    {
        port_obj = bdmf_get_next(rdpa_port_drv(), port_obj, NULL);
        if (!port_obj)
            break;
                
        rc = rdpa_port_type_get(port_obj, &port_type);
        rdpa_port_is_wan_get(port_obj, &is_wan);        
        if (rc)
            continue;

        if ((port_type == rdpa_port_epon_ae) && is_wan)
        {
            bdmf_put(port_obj);
            return 1;
        }
    }

    return 0;
}

EXPORT_SYMBOL(rdpa_cmd_misc_ioctl);
EXPORT_SYMBOL(rdpa_cmd_misc_init);

