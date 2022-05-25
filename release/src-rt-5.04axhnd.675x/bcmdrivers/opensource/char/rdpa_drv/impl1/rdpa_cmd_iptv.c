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
 * File Name  : rdpa_cmd_iptv.c
 *
 * Description: This file contains the runner iptv configuration API.
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
#include "rdpa_cmd_tm.h"

#define __BDMF_LOG__

#define CMD_IPTV_LOG_ID_RDPA_CMD_DRV BCM_LOG_ID_RDPA_CMD_DRV

#if defined(__BDMF_LOG__)
#define CMD_IPTV_LOG_ERROR(fmt, args...) 										\
    do {                                                            				\
        if (bdmf_global_trace_level >= bdmf_trace_level_error)      				\
            bdmf_trace("ERR: %s#%d: " fmt "\n", __FUNCTION__, __LINE__, ## args);	\
    } while(0)
#define CMD_IPTV_LOG_INFO(fmt, args...) 										\
    do {                                                            				\
        if (bdmf_global_trace_level >= bdmf_trace_level_info)      					\
            bdmf_trace("INF: %s#%d: " fmt "\n", __FUNCTION__, __LINE__, ## args);	\
    } while(0)
#define CMD_IPTV_LOG_DEBUG(fmt, args...) 										\
    do {                                                            				\
        if (bdmf_global_trace_level >= bdmf_trace_level_debug)      					\
            bdmf_trace("DBG: %s#%d: " fmt "\n", __FUNCTION__, __LINE__, ## args);	\
    } while(0)
#else
#define CMD_IPTV_LOG_ERROR(fmt, arg...) BCM_LOG_ERROR(fmt, arg...)
#define CMD_IPTV_LOG_INFO(fmt, arg...) BCM_LOG_INFO(fmt, arg...)
#define CMD_IPTV_LOG_DEBUG(fmt, arg...) BCM_LOG_DEBUG(fmt, arg...)
#endif

#define DUMP_IPV4_ADDR_FMT      "<%03u.%03u.%03u.%03u>"
#define DUMP_IPV6_ADDR_FMT      "<%04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x>"


#define DUMP_IPV4_ADDR(ip)      ((uint8_t*)&ip)[0], ((uint8_t*)&ip)[1],     \
                                ((uint8_t*)&ip)[2], ((uint8_t*)&ip)[3]

#define DUMP_IPV6_ADDR(ip)      ((uint16_t*)&ip)[0], ((uint16_t*)&ip)[1],   \
                                ((uint16_t*)&ip)[2], ((uint16_t*)&ip)[3],   \
                                ((uint16_t*)&ip)[4], ((uint16_t*)&ip)[5],   \
                                ((uint16_t*)&ip)[6], ((uint16_t*)&ip)[7]

/*******************************************************************************/
/* static routines Functions                                                   */
/*******************************************************************************/
static void dump_iptv_entry(rdpa_drv_ioctl_iptv_entry_t *entry, 
                                 rdpa_iptv_lookup_method method)
{
    CMD_IPTV_LOG_INFO("    filter\n");
    switch(method)
    {
        case iptv_lookup_method_mac_vid:
            CMD_IPTV_LOG_INFO("                     vid: %d", entry->key.vid);
        case iptv_lookup_method_mac: 
            CMD_IPTV_LOG_INFO("             mac address: %02X:%02X:%02X:%02X:%02X:%02X",
                entry->key.group.mac[0], entry->key.group.mac[1],
                entry->key.group.mac[2], entry->key.group.mac[3],
                entry->key.group.mac[4], entry->key.group.mac[5]);
            break;
        case iptv_lookup_method_group_ip_src_ip_vid:
            CMD_IPTV_LOG_INFO("                     vid: %d", entry->key.vid);
        case iptv_lookup_method_group_ip_src_ip:
            if (entry->key.ip_family == bdmf_ip_family_ipv4)
            {
                CMD_IPTV_LOG_INFO("               source ip:" DUMP_IPV4_ADDR_FMT,
                    DUMP_IPV4_ADDR(entry->key.src_ip.ipv4));
            } 
            else 
            {
                CMD_IPTV_LOG_INFO("               source ip:" DUMP_IPV6_ADDR_FMT,
                    DUMP_IPV6_ADDR(entry->key.src_ip.ipv6[0]));
            } 
        case iptv_lookup_method_group_ip:
            if (entry->key.ip_family == bdmf_ip_family_ipv4)
            {
                CMD_IPTV_LOG_INFO("                group ip:" DUMP_IPV4_ADDR_FMT,
                    DUMP_IPV4_ADDR(entry->key.group.ipv4));
            } 
            else 
            {
                CMD_IPTV_LOG_INFO("                group ip:" DUMP_IPV6_ADDR_FMT,
                    DUMP_IPV6_ADDR(entry->key.group.ipv6[0]));
            } 
            break;
    }

    CMD_IPTV_LOG_INFO("    vlan         acvtion: %d", entry->vlan.action);
    CMD_IPTV_LOG_INFO("                     vid: %d", entry->vlan.vid);
}

static void vlan_action_find(rdpa_vlan_action_cfg_t *action, 
                                rdpa_traffic_dir dir, 
                                bdmf_object_handle *vlan_action_obj)
{
    bdmf_object_handle obj = NULL;
    rdpa_vlan_action_cfg_t action_tmp;
    rdpa_traffic_dir dir_tmp;

    *vlan_action_obj = NULL;

    while ((obj = bdmf_get_next(rdpa_vlan_action_drv(), obj, NULL)))
    {
        rdpa_vlan_action_dir_get(obj, &dir_tmp);
        if (dir != dir_tmp)
            continue;

        rdpa_vlan_action_action_get(obj, &action_tmp);
        if (!memcmp(action, &action_tmp, sizeof(action_tmp)))
        {
            *vlan_action_obj = obj;
            return;
        }
    }
}

static int vlan_action_add(rdpa_vlan_action_cfg_t *action, rdpa_traffic_dir dir, bdmf_object_handle *vlan_action_obj)
{
    int rc;
    BDMF_MATTR(vlan_action_attr, rdpa_vlan_action_drv());

    rc = rdpa_vlan_action_dir_set(vlan_action_attr, dir);
    rc = rc ? : rdpa_vlan_action_action_set(vlan_action_attr, action);
    /* index will be picked automatically by rdpa */
    rc = rc ? : bdmf_new_and_set(rdpa_vlan_action_drv(), NULL, vlan_action_attr, vlan_action_obj);
    if (rc < 0)
    {
        CMD_IPTV_LOG_ERROR("Failed to create vlan action object rc(%d)", rc);
    }
    else 
    {
        CMD_IPTV_LOG_DEBUG("Created new vlan_action");
    }

    return rc; 
}

static int build_iptv_request(rdpa_drv_ioctl_iptv_t *iptv, 
                                  rdpa_iptv_channel_request_t *req,
                                  rdpa_iptv_lookup_method method)
{
    bdmf_object_handle vlan_action_obj;
    rdpa_vlan_action_cfg_t vlan_action; 
    int rc = 0;

    /* reset req */
    memset(req, 0, sizeof(rdpa_iptv_channel_request_t));
    switch(method)
    {
        case iptv_lookup_method_mac_vid:
            req->key.vid = iptv->entry.key.vid;
        case iptv_lookup_method_mac: 
            /* copy mac address */
            memcpy(req->key.mcast_group.mac.b, iptv->entry.key.group.mac, 6);
            break;
        case iptv_lookup_method_group_ip_src_ip_vid:
            req->key.vid = iptv->entry.key.vid;
        case iptv_lookup_method_group_ip_src_ip:
            if (iptv->entry.key.ip_family == bdmf_ip_family_ipv4)
            {
                req->key.mcast_group.l3.src_ip.family = bdmf_ip_family_ipv4;
                req->key.mcast_group.l3.src_ip.addr.ipv4 = iptv->entry.key.src_ip.ipv4;
            } 
            else 
            {
                req->key.mcast_group.l3.src_ip.family = bdmf_ip_family_ipv6;
                memcpy(req->key.mcast_group.l3.src_ip.addr.ipv6.data, 
                       iptv->entry.key.src_ip.ipv6,
                       sizeof(req->key.mcast_group.l3.src_ip.addr.ipv6.data));
            } 
        case iptv_lookup_method_group_ip:
            if (iptv->entry.key.ip_family == bdmf_ip_family_ipv4)
            {
                req->key.mcast_group.l3.gr_ip.family = bdmf_ip_family_ipv4;
                req->key.mcast_group.l3.gr_ip.addr.ipv4 = iptv->entry.key.group.ipv4;
            } 
            else 
            {
                req->key.mcast_group.l3.gr_ip.family = bdmf_ip_family_ipv6;
                memcpy(req->key.mcast_group.l3.gr_ip.addr.ipv6.data, 
                       iptv->entry.key.group.ipv6,
                       sizeof(req->key.mcast_group.l3.gr_ip.addr.ipv6.data));
            } 
            break;
    }

    req->mcast_result.egress_port = iptv->egress_port + rdpa_if_lan0;

    memset(&vlan_action, 0, sizeof(rdpa_vlan_action_cfg_t));
    
    vlan_action.cmd = iptv->entry.vlan.action;
    if (vlan_action.cmd == RDPA_VLAN_CMD_REPLACE)
    {
        vlan_action.parm[0].vid = iptv->entry.vlan.vid;
    }
    vlan_action_find(&vlan_action, rdpa_dir_ds, &vlan_action_obj);
    if (!vlan_action_obj)
        rc = vlan_action_add(&vlan_action, rdpa_dir_ds, &vlan_action_obj);
    else
    {
        bdmf_number idx;

        rdpa_vlan_action_index_get(vlan_action_obj, &idx);
        //bdmf_put(vlan_action_obj);
        CMD_IPTV_LOG_DEBUG("Reusing existing vlan_object %d", (int)idx);
    }
    if (!rc)
    {
        req->mcast_result.vlan_action = vlan_action_obj;
    }
    return rc;
     
}


/*******************************************************************************/
/* global routines                                                             */
/*******************************************************************************/

/*******************************************************************************
 *
 * Function: rdpa_cmd_iptv_ioctl
 *
 * IOCTL interface to the RDPA IPTV API.
 *
 *******************************************************************************/
int rdpa_cmd_iptv_ioctl(unsigned long arg)
{
	rdpa_drv_ioctl_iptv_t *userIptv_p = (rdpa_drv_ioctl_iptv_t *)arg;
	rdpa_drv_ioctl_iptv_t iptv;
	bdmf_object_handle iptv_obj = NULL;
    rdpa_iptv_lookup_method method;
    rdpa_mcast_filter_method filter_method;
    rdpa_iptv_channel_request_t request;
    rdpa_channel_req_key_t request_key;
    int ret = 0;
	bdmf_error_t rc = BDMF_ERR_OK;

    copy_from_user(&iptv, userIptv_p, sizeof(rdpa_drv_ioctl_iptv_t));

    CMD_IPTV_LOG_DEBUG("RDPA IPTV CMD(%d)", iptv.cmd);

    bdmf_lock();

    rc = rdpa_iptv_get(&iptv_obj);
    if (rc)
    {
		CMD_IPTV_LOG_ERROR("rdpa_iptv_get() failed: rc(%d)", rc);
	    ret = RDPA_DRV_ERROR;
	    goto ioctl_exit;
    }

    switch(iptv.cmd)
    {
        case RDPA_IOCTL_IPTV_CMD_LOOKUP_METHOD_SET: {
            CMD_IPTV_LOG_INFO("RDPA_IOCTL_IPTV_CMD_LOOKUP_METHOD_SET: method(%d)", iptv.method);
            
            rc = rdpa_iptv_lookup_method_set(iptv_obj, iptv.method);
            if (rc)
            {
    		    CMD_IPTV_LOG_ERROR("rdpa_iptv_lookup_method_set() failed: rc(%d)", rc);
                ret = RDPA_DRV_ERROR;
                goto ioctl_exit;
            }
            break;
        }
        case RDPA_IOCTL_IPTV_CMD_LOOKUP_METHOD_GET: {
            CMD_IPTV_LOG_INFO("RDPA_IOCTL_IPTV_CMD_LOOKUP_METHOD_GET");
            rc = rdpa_iptv_lookup_method_get(iptv_obj, &method);
            if (rc)
            {
    		    CMD_IPTV_LOG_ERROR("rdpa_iptv_lookup_method_get() failed: rc(%d)", rc);
                ret = RDPA_DRV_ERROR;
                goto ioctl_exit;
            }

            iptv.method = method;
            
            break;
        }
        case RDPA_IOCTL_IPTV_CMD_ENTRY_ADD: {
            CMD_IPTV_LOG_INFO("RDPA_IOCTL_IPTV_CMD_ENTRY_ADD: egress port(%d)", iptv.egress_port);
            rc = rdpa_iptv_lookup_method_get(iptv_obj, &method);
            if (rc)
            {
    		    CMD_IPTV_LOG_ERROR("rdpa_iptv_lookup_method_get() failed: rc(%d)", rc);
                ret = RDPA_DRV_ERROR;
                goto ioctl_exit;
            }

            dump_iptv_entry(&iptv.entry, method);
            rc = build_iptv_request(&iptv, &request, method);
            if (rc)
            {
    		    CMD_IPTV_LOG_ERROR("build_iptv_request() failed: rc(%d)", rc);
                ret = RDPA_DRV_ERROR;
                goto ioctl_exit;
            }
            rc = rdpa_iptv_channel_request_add(iptv_obj, &request_key, &request);
            if (rc < 0)
            {
                if (rc != BDMF_ERR_ALREADY)
                {
                    ret = RDPA_DRV_ERROR;
                    goto ioctl_exit;
                }
            }
            else
                iptv.index = (uint32_t)request_key.channel_index;
            break;
        }
        case RDPA_IOCTL_IPTV_CMD_ENTRY_REMOVE: {
            CMD_IPTV_LOG_INFO("RDPA_IOCTL_IPTV_CMD_ENTRY_REMOVE: index(%d) egress port(%d)", 
                iptv.index, iptv.egress_port);

            request_key.port = iptv.egress_port + rdpa_if_lan0;
            request_key.channel_index = iptv.index;

            rc = rdpa_iptv_channel_request_delete(iptv_obj, &request_key);
            if (rc < 0)
            {
                if (rc != BDMF_ERR_NOENT)                
                {
                    ret = RDPA_DRV_ERROR;
                    goto ioctl_exit;
                }
            }
            break;
        }
        case RDPA_IOCTL_IPTV_CMD_ENTRY_FLUSH: {
            bdmf_boolean flush = TRUE;
            rc = rdpa_iptv_flush_set(iptv_obj, flush);
            if (rc)
            {
    		    CMD_IPTV_LOG_ERROR("rdpa_iptv_flush_set() failed: rc(%d)", rc);
                ret = RDPA_DRV_ERROR;
                goto ioctl_exit;
            }
            
            break;
        }
        case RDPA_IOCTL_IPTV_CMD_PREFIX_FILTER_SET: {
            CMD_IPTV_LOG_INFO("RDPA_IOCTL_IPTV_CMD_PREFIX_FILTER_SET: filter method(%d)", iptv.filter_method);
 
            rc = rdpa_iptv_mcast_prefix_filter_set(iptv_obj, iptv.filter_method);
            if (rc)
            {
    		    CMD_IPTV_LOG_ERROR("rdpa_iptv_mcast_prefix_filter_set() failed: rc(%d)", rc);
                ret = RDPA_DRV_ERROR;
                goto ioctl_exit;
            }
            break;
        }
        case RDPA_IOCTL_IPTV_CMD_PREFIX_FILTER_GET: {
            CMD_IPTV_LOG_INFO("RDPA_IOCTL_IPTV_CMD_PREFIX_FILTER_GET");
            rc = rdpa_iptv_mcast_prefix_filter_get(iptv_obj, &filter_method);
            if (rc)
            {
    		    CMD_IPTV_LOG_ERROR("rdpa_iptv_mcast_prefix_filter_get() failed: rc(%d)", rc);
                ret = RDPA_DRV_ERROR;
                goto ioctl_exit;
            }
            
            iptv.filter_method = filter_method;
           
            CMD_IPTV_LOG_INFO("RDPA_IOCTL_IPTV_CMD_PREFIX_FILTER_GET: filter method(%d)", iptv.filter_method);
            break;
        }
        default:
            CMD_IPTV_LOG_ERROR("Invalid IOCTL cmd %d", iptv.cmd);
            ret = RDPA_DRV_ERROR;
    }

ioctl_exit:
	if (ret) {
		CMD_IPTV_LOG_ERROR("rdpa_cmd_iptv_ioctl() OUT: FAILED: rc(%d)", rc);
	}

    if (iptv_obj)
	    bdmf_put(iptv_obj);

	bdmf_unlock();

	copy_to_user(userIptv_p, &iptv, sizeof(rdpa_drv_ioctl_iptv_t));

    return ret;
}


