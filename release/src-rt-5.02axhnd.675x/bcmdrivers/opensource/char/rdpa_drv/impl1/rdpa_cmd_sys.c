
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
 * File Name  : rdpa_cmd_sys.c
 *
 * Description: This file contains the FAP Traffic Manager configuration API.
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
#include "rdpa_epon.h"
#include "rdpa_ag_epon.h"
#include "rdpa_ag_port.h"
#include "rdpa_drv.h"
#include "rdpa_cmd_sys.h"
#include "rdpa_cmd_misc.h"
#if (defined(CONFIG_BCM_INGQOS) || defined(CONFIG_BCM_INGQOS_MODULE))
#include <linux/iqos.h>
#include <ingqos.h>
#endif

#define __BDMF_LOG__

#define CMD_SYS_LOG_ID_RDPA_CMD_DRV BCM_LOG_ID_RDPA_CMD_DRV

#if defined(__BDMF_LOG__)
#define CMD_SYS_LOG_ERROR(fmt, args...) 										\
    do {                                                            				\
        if (bdmf_global_trace_level >= bdmf_trace_level_error)      				\
            bdmf_trace("ERR: %s#%d: " fmt "\n", __FUNCTION__, __LINE__, ## args);	\
    } while(0)
#define CMD_SYS_LOG_INFO(fmt, args...) 										\
    do {                                                            				\
        if (bdmf_global_trace_level >= bdmf_trace_level_info)      					\
            bdmf_trace("INF: %s#%d: " fmt "\n", __FUNCTION__, __LINE__, ## args);	\
    } while(0)
#define CMD_SYS_LOG_DEBUG(fmt, args...) 										\
    do {                                                            				\
        if (bdmf_global_trace_level >= bdmf_trace_level_debug)      					\
            bdmf_trace("DBG: %s#%d: " fmt "\n", __FUNCTION__, __LINE__, ## args);	\
    } while(0)
#else
#define CMD_SYS_LOG_ERROR(fmt, arg...) BCM_LOG_ERROR(fmt, arg...)
#define CMD_SYS_LOG_INFO(fmt, arg...) BCM_LOG_INFO(fmt, arg...)
#define CMD_SYS_LOG_DEBUG(fmt, arg...) BCM_LOG_DEBUG(fmt, arg...)
#endif


/*******************************************************************************/
/* global routines                                                             */
/*******************************************************************************/

static bdmf_object_handle system_obj = NULL;

#if (defined(CONFIG_BCM_INGQOS) || defined(CONFIG_BCM_INGQOS_MODULE))
int rdpa_iq_sys_set_cong_ctrl(void *iq_param)
{
    int rc = BDMF_ERR_OK;
#if defined(CONFIG_BCM_DSL_RDP)
    bdmf_number threshold_num;
#elif defined(CONFIG_BCM_DSL_XRDP)
    bdmf_boolean enable;
#endif

#if defined(CONFIG_BCM_DSL_RDP)
    if (((iq_param_t *)iq_param)->status > 0)
        threshold_num = RDPA_IQ_IH_CONG_THRESHOLD_NUM;
    else 
        threshold_num = 0;

    rdpa_system_get(&system_obj);
    rc = rdpa_system_ih_cong_threshold_set(system_obj, threshold_num);
    bdmf_put(system_obj);
#elif defined(CONFIG_BCM_DSL_XRDP)
    if (((iq_param_t *)iq_param)->status > 0)
        enable = 1;
    else 
        enable = 0;

    rdpa_system_get(&system_obj);
    rc = rdpa_system_ingress_congestion_ctrl_set(system_obj, enable);
    bdmf_put(system_obj);
#else
    rc = BDMF_ERR_NOT_SUPPORTED;
#endif

    return rc;
}

int rdpa_iq_sys_get_cong_ctrl(void *iq_param)
{
    int rc = BDMF_ERR_OK;
#if defined(CONFIG_BCM_DSL_RDP)
    bdmf_number threshold_num;
#elif defined(CONFIG_BCM_DSL_XRDP)
    bdmf_boolean enable;
#endif

    ((iq_param_t *)iq_param)->status = 0;
#if defined(CONFIG_BCM_DSL_RDP)
    rdpa_system_get(&system_obj);
    rc = rdpa_system_ih_cong_threshold_get(system_obj, &threshold_num);
    bdmf_put(system_obj);
    if ((rc == BDMF_ERR_OK) && (threshold_num != 0))
        ((iq_param_t *)iq_param)->status = 1;
#elif defined(CONFIG_BCM_DSL_XRDP)
    rdpa_system_get(&system_obj);
    rc = rdpa_system_ingress_congestion_ctrl_get(system_obj, &enable);
    bdmf_put(system_obj);
    if ((rc == BDMF_ERR_OK) && (enable != 0))
        ((iq_param_t *)iq_param)->status = 1;
#elif defined(CONFIG_BCM_PON_XRDP)
    ((iq_param_t *)iq_param)->status = 1;
#endif

    return rc;
}
#endif

/*******************************************************************************
 *
 * Function: rdpa_cmd_sys_ioctl
 *
 * IOCTL interface to the RDPA INGRESS CLASSIFIER API.
 *
 *******************************************************************************/
int rdpa_cmd_sys_ioctl(unsigned long arg)
{
    rdpa_drv_ioctl_sys_t *userSys_p = (rdpa_drv_ioctl_sys_t *)arg;
    rdpa_drv_ioctl_sys_t sys;
    int rc = BDMF_ERR_OK;

    copy_from_user(&sys, userSys_p, sizeof(rdpa_drv_ioctl_sys_t));

    CMD_SYS_LOG_DEBUG("RDPA SYS CMD(%d)", sys.cmd);

    bdmf_lock();
	
    switch(sys.cmd)
    {
        case RDPA_IOCTL_SYS_CMD_WANTYPE_GET:
        {
            rdpa_wan_type wan_type = rdpa_wan_none;

            get_rdpa_wan_type(sys.param.rdpa_if, &wan_type);
            sys.param.wan_type = wan_type;
            copy_to_user((rdpa_drv_ioctl_sys_t *)arg, &sys, sizeof(rdpa_drv_ioctl_sys_t));
            break;
        }
		
        case RDPA_IOCTL_SYS_CMD_IN_TPID_GET:
        {
            rdpa_system_cfg_t system_cfg;
                
            rdpa_system_get(&system_obj);
            if ((rc = rdpa_system_cfg_get(system_obj, &system_cfg) == BDMF_ERR_OK))
            {
                sys.param.inner_tpid = system_cfg.inner_tpid;
            }
            bdmf_put(system_obj);
            break;
        }
        
    	case RDPA_IOCTL_SYS_CMD_IN_TPID_SET:
        {
            rdpa_system_cfg_t system_cfg;
            
            rdpa_system_get(&system_obj);
            if ((rc = rdpa_system_cfg_get(system_obj, &system_cfg) == BDMF_ERR_OK))
            {
                system_cfg.inner_tpid = sys.param.inner_tpid;
                rc = rdpa_system_cfg_set(system_obj, &system_cfg);
            }
            bdmf_put(system_obj);
            break;
        }
		
        case RDPA_IOCTL_SYS_CMD_OUT_TPID_GET:
        {
            rdpa_system_cfg_t system_cfg;
                    
            rdpa_system_get(&system_obj);
            if ((rc = rdpa_system_cfg_get(system_obj, &system_cfg) == BDMF_ERR_OK))
            {
                sys.param.outer_tpid = system_cfg.outer_tpid;
            }
            bdmf_put(system_obj);
            break;
        }       
		
        case RDPA_IOCTL_SYS_CMD_OUT_TPID_SET:
        {
            rdpa_system_cfg_t system_cfg;
                
            rdpa_system_get(&system_obj);
            if ((rc = rdpa_system_cfg_get(system_obj, &system_cfg) == BDMF_ERR_OK))
            {
                system_cfg.outer_tpid = sys.param.outer_tpid;
                rc = rdpa_system_cfg_set(system_obj, &system_cfg);
            }
            bdmf_put(system_obj);
            break;
        }

        case RDPA_IOCTL_SYS_CMD_DETECT_TPID_SET:
        {
            rdpa_tpid_detect_cfg_t entry = {.val_udef=0, .otag_en=0, .itag_en=0};
            rdpa_tpid_detect_t tpid = rdpa_tpid_detect__num_of;
            int is_found = 0;
            
            rdpa_system_get(&system_obj);
            
            for (tpid = rdpa_tpid_detect_0x8100; tpid < rdpa_tpid_detect__num_of; tpid ++)
            {
                if (rdpa_system_tpid_detect_get(system_obj, tpid, &entry) == BDMF_ERR_OK)
                {
                    if (entry.val_udef == sys.param.inner_tpid)
                    {
                        is_found = 1;
                        break;
                    }
                }
            }
            if (!is_found)
            {
                entry.val_udef = sys.param.detect_tpid.tpid;
                entry.otag_en = 1;
                entry.itag_en = 1;
                switch(entry.val_udef)
                {
                    case 0x8100:
                        tpid = rdpa_tpid_detect_0x8100;
                        break;
                    case 0x88A8:
                        tpid = rdpa_tpid_detect_0x88A8;
                        break;
                    case 0x9100:
                        tpid = rdpa_tpid_detect_0x9100;
                        break;
                    case 0x9200:
                        tpid = rdpa_tpid_detect_0x9200;
                        break;
                    default:
                        tpid = sys.param.detect_tpid.is_inner?rdpa_tpid_detect_udef_2:rdpa_tpid_detect_udef_1;
                        break;
                }
                rc = rdpa_system_tpid_detect_set(system_obj, tpid, &entry);
            }
                
            bdmf_put(system_obj);
            break;
        }

        case RDPA_IOCTL_SYS_CMD_EPON_MODE_SET:
        {
            /* temporary. this will move to the rdpa_cmd_epon
            */
            rdpa_epon_mode epon_mode = sys.param.epon_mode;
            if (!rdpa_epon_get(&system_obj))
            {
                rc = rdpa_epon_mode_set(system_obj, epon_mode);
                bdmf_put(system_obj);
            }
            break;
        }

        case RDPA_IOCTL_SYS_CMD_EPON_MODE_GET:
        {
            /* temporary. this will move to the rdpa_cmd_epon 
            */
            rdpa_epon_mode epon_mode = rdpa_epon_none;
            if (!rdpa_epon_get(&system_obj))
            {
                rc = rdpa_epon_mode_get(system_obj, &epon_mode);
                bdmf_put(system_obj);
            }
            sys.param.epon_mode = epon_mode;
            copy_to_user((rdpa_drv_ioctl_sys_t *)arg, &sys, sizeof(rdpa_drv_ioctl_sys_t));
            break;
        }

        case RDPA_IOCTL_SYS_CMD_EPON_STATUS_GET:
        {
            bdmf_object_handle port_obj = NULL;
            rdpa_port_dp_cfg_t port_cfg;
            rdpa_if port = rdpa_wan_type_to_if(rdpa_wan_epon);

            if (rdpa_port_get(port, &port_obj))
            {
                CMD_SYS_LOG_ERROR("WAN port is not initialized\n");
                sys.param.epon_enable = 0;
                break;
            }
            
            if ((rc = rdpa_port_cfg_get(port_obj, &port_cfg)))
            {
                CMD_SYS_LOG_ERROR("Failed to get port configuration for WAN port\n");
                bdmf_put(port_obj);
                break;
            }
            else
            {
                sys.param.epon_enable = !port_cfg.ae_enable;
                bdmf_put(port_obj);
            }


            copy_to_user((rdpa_drv_ioctl_sys_t *)arg, &sys, sizeof(rdpa_drv_ioctl_sys_t));
            break;
        }

        case RDPA_IOCTL_SYS_CMD_ALWAYS_TPID_SET:
        {
            rdpa_system_cfg_t system_cfg;
            rdpa_system_get(&system_obj);
            rc = rdpa_system_cfg_get(system_obj, &system_cfg);
            system_cfg.add_always_tpid = sys.param.always_tpid;
            rc = rdpa_system_cfg_set(system_obj, &system_cfg);
            bdmf_put(system_obj);
            break;
        }

        case RDPA_IOCTL_SYS_CMD_FORCE_DSCP_GET:
        {
            rdpa_system_cfg_t system_cfg;
            rdpa_system_get(&system_obj);
            rdpa_system_cfg_get(system_obj, &system_cfg);
            bdmf_put(system_obj);

            if (sys.param.force_dscp.dir == rdpa_dir_ds)
                sys.param.force_dscp.enable = system_cfg.force_dscp_to_pbit_ds;
            else
                sys.param.force_dscp.enable = system_cfg.force_dscp_to_pbit_us;

            copy_to_user((rdpa_drv_ioctl_sys_t *)arg,
                &sys, sizeof(rdpa_drv_ioctl_sys_t));

            break;
        }

        case RDPA_IOCTL_SYS_CMD_FORCE_DSCP_SET:
        {
            rdpa_system_cfg_t system_cfg;
            rdpa_system_get(&system_obj);
            rc = rdpa_system_cfg_get(system_obj, &system_cfg);

            if (sys.param.force_dscp.dir == rdpa_dir_ds)
                system_cfg.force_dscp_to_pbit_ds = sys.param.force_dscp.enable;
            else
                system_cfg.force_dscp_to_pbit_us = sys.param.force_dscp.enable;

            rc = rdpa_system_cfg_set(system_obj, &system_cfg);
            bdmf_put(system_obj);
            break;
        }

        case RDPA_IOCTL_SYS_CMD_CAR_MODE_SET:
        {
            rdpa_system_cfg_t system_cfg;
            rdpa_system_get(&system_obj);
            rc = rdpa_system_cfg_get(system_obj, &system_cfg);

            system_cfg.car_mode = sys.param.car_mode;

            rc = rdpa_system_cfg_set(system_obj, &system_cfg);
            bdmf_put(system_obj);
            break;
        }

        default:
            CMD_SYS_LOG_ERROR("Invalid IOCTL cmd %d", sys.cmd);
            rc = RDPA_DRV_ERROR;
    }

    if (rc) {
    CMD_SYS_LOG_ERROR("rdpa_cmd_sys_ioctl() OUT: FAILED: rc(%d)", rc);
    }

    bdmf_unlock();
    return rc;
}

/*******************************************************************************
 *
 * Function: rdpa_cmd_SYS_init
 *
 * Initializes the RDPA IC API.
 *
 *******************************************************************************/
void rdpa_cmd_sys_init(void)
{
    CMD_SYS_LOG_DEBUG("RDPA SYS INIT");
}
