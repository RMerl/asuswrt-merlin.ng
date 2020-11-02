
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
 * File Name  : rdpa_cmd_pbit_to_q.c
 *
 * Description: This file contains the RDPA PBIT TO Q API.
 *
 *******************************************************************************
 */

#include <linux/module.h>
#include <linux/bcm_log.h>
#include "bcmtypes.h"
#include "rdpa_types.h"
#include "rdpa_api.h"
#include "rdpa_drv.h"
#include "rdpa_cmd_pbit_to_q.h"

#define __BDMF_LOG__

#define CMD_P_TO_Q_LOG_ID_RDPA_CMD_DRV BCM_LOG_ID_RDPA_CMD_DRV

#if defined(__BDMF_LOG__)
#define CMD_P_TO_Q_LOG_ERROR(fmt, args...) 										\
    do {                                                            				\
        if (bdmf_global_trace_level >= bdmf_trace_level_error)      				\
            bdmf_trace("ERR: %s#%d: " fmt "\n", __FUNCTION__, __LINE__, ## args);	\
    } while(0)
#define CMD_P_TO_Q_LOG_INFO(fmt, args...) 										\
    do {                                                            				\
        if (bdmf_global_trace_level >= bdmf_trace_level_info)      					\
            bdmf_trace("INF: %s#%d: " fmt "\n", __FUNCTION__, __LINE__, ## args);	\
    } while(0)
#define CMD_P_TO_Q_LOG_DEBUG(fmt, args...) 										\
    do {                                                            				\
        if (bdmf_global_trace_level >= bdmf_trace_level_debug)      					\
            bdmf_trace("DBG: %s#%d: " fmt "\n", __FUNCTION__, __LINE__, ## args);	\
    } while(0)
#else
#define CMD_P_TO_Q_LOG_ERROR(fmt, arg...) BCM_LOG_ERROR(fmt, arg...)
#define CMD_P_TO_Q_LOG_INFO(fmt, arg...) BCM_LOG_INFO(fmt, arg...)
#define CMD_P_TO_Q_LOG_DEBUG(fmt, arg...) BCM_LOG_DEBUG(fmt, arg...)
#endif

#define TOTAL_PBIT_NUM   8
/*******************************************************************************/
/* static routines Functions                                                   */
/*******************************************************************************/

/*******************************************************************************
 *
 * Function: dev_link_pbit_to_q_check
 *
 *   check if dev is linked to pbit to queue 
 *
 ******************************************************************************/
static BOOL pbit_to_q_dev_link_check(bdmf_object_handle dev_obj, 
                               bdmf_object_handle p_to_q_obj)
{
    bdmf_link_handle obj_link = NULL;
    
    obj_link = bdmf_get_next_us_link(p_to_q_obj, NULL);
    if (obj_link)
    {
        do {
            if (bdmf_us_link_to_object(obj_link) == dev_obj)
            {
                return TRUE;
            }
            obj_link = bdmf_get_next_us_link(p_to_q_obj, obj_link);
        } while(obj_link);
    }

    /* compare DS links obj*/
    obj_link = bdmf_get_next_ds_link(p_to_q_obj, NULL);
    if (obj_link)
    {
        do {
            if (bdmf_us_link_to_object(obj_link) == dev_obj)
            {
                return true;
            }
            obj_link = bdmf_get_next_ds_link(p_to_q_obj, obj_link);
        } while(obj_link);
    }

    return FALSE;
}


static int pbit_to_q_link_dev_num_get(bdmf_object_handle p_to_q_obj)
{
    int link_num = 0;
    bdmf_link_handle obj_link = NULL;
    
    obj_link = bdmf_get_next_us_link(p_to_q_obj, NULL);
    if (obj_link)
    {
        do {
            link_num++;
            obj_link = bdmf_get_next_us_link(p_to_q_obj, obj_link);
        } while(obj_link);
    }

    obj_link = bdmf_get_next_ds_link(p_to_q_obj, NULL);
    if (obj_link)
    {
        do {
            link_num++;
            obj_link = bdmf_get_next_ds_link(p_to_q_obj, obj_link);
        } while(obj_link);
    }

    return link_num;
}

static int pbit_to_q_map_set(bdmf_object_handle p_to_q_obj, 
                                        rdpa_drv_ioctl_pbit_to_q_t pbit_to_q)
{
    int i = 0, rc = BDMF_ERR_OK;

    for (i = 0; i < TOTAL_PBIT_NUM; i++)
    {
        if (pbit_to_q.pbit_q_map[i] == U32_MAX)
            continue;

        rc = rdpa_pbit_to_queue_pbit_map_set(
            p_to_q_obj, i, pbit_to_q.pbit_q_map[i]);
        if (0 != rc)
        {
            CMD_P_TO_Q_LOG_ERROR(
                "rdpa_pbit_to_queue_pbit_map_set() failed:" \
                "pbit(%u) qid(%u) rc(%d)", \
                i, pbit_to_q.pbit_q_map[i], rc);
            return RDPA_DRV_P_TO_Q_QOS_MAP_SET;
        }
    }

    return 0;
}

static BOOL pbit_to_q_same_config_check(bdmf_object_handle p_to_q_obj, 
                                        rdpa_drv_ioctl_pbit_to_q_t pbit_to_q)
{
    int i = 0, rc = 0;
    bdmf_number pbit_qid;
    
    for (i = 0; i < TOTAL_PBIT_NUM; i++)
    {
        if (pbit_to_q.pbit_q_map[i] == U32_MAX)
            continue;
        
        rc = rdpa_pbit_to_queue_pbit_map_get(p_to_q_obj, i, &pbit_qid);
        if (0 != rc)
        {
            CMD_P_TO_Q_LOG_ERROR(
                "rdpa_pbit_to_queue_pbit_map_get() failed:" \
                "pbit(%u) rc(%d)", i, rc);
            return FALSE;
        }

        if (pbit_qid != pbit_to_q.pbit_q_map[i])
            break;
    }

    if (i == TOTAL_PBIT_NUM)
    {
        return TRUE;
    }

    return FALSE;
}


/*******************************************************************************/
/* global routines                                                             */
/*******************************************************************************/


/*******************************************************************************
 *
 * Function: rdpa_cmd_pbit_to_q_ioctl
 *
 * IOCTL interface to the RDPA PBIT TO Q API.
 *
 *******************************************************************************/
int rdpa_cmd_pbit_to_q_ioctl(unsigned long arg)
{
    rdpa_drv_ioctl_pbit_to_q_t *userPtoQ_p = (rdpa_drv_ioctl_pbit_to_q_t *)arg;
    rdpa_drv_ioctl_pbit_to_q_t pbit_to_q;
    int ret = 0;
    int rc = BDMF_ERR_OK;
    bdmf_object_handle p_to_q_obj = NULL;
    bdmf_object_handle dev_obj = NULL;
    
    copy_from_user(&pbit_to_q, userPtoQ_p, sizeof(rdpa_drv_ioctl_pbit_to_q_t));

    CMD_P_TO_Q_LOG_DEBUG("RDPA PBIT TO Q CMD(%d)", pbit_to_q.cmd);

    bdmf_lock();

    switch (pbit_to_q.dev_type)
    {
        case RDPA_IOCTL_DEV_PORT:
            rc = rdpa_port_get(pbit_to_q.dev_id, &dev_obj);
            if (0 != rc)
            {
                CMD_P_TO_Q_LOG_DEBUG("rdpa_port_get() failed:" \
                    " port(%u) rc(%d)", pbit_to_q.dev_id, rc);
                ret = RDPA_DRV_PORT_GET;
                goto p_to_q_ioctl_exit;
            }
            break;
            
        case RDPA_IOCTL_DEV_TCONT:
            rc = rdpa_tcont_get(pbit_to_q.dev_id, &dev_obj);
            if (0 != rc)
            {
                CMD_P_TO_Q_LOG_DEBUG("rdpa_tcont_get() failed:" \
                    " tcont(%u) rc(%d)", pbit_to_q.dev_id, rc);
                ret = RDPA_DRV_TCONT_GET;
                goto p_to_q_ioctl_exit;
            }
            break;
            
        case RDPA_IOCTL_DEV_LLID:
            rc = rdpa_llid_get(pbit_to_q.dev_id, &dev_obj);
            if (0 != rc)
            {
                CMD_P_TO_Q_LOG_DEBUG("rdpa_llid_get() failed:" \
                    " llid(%u) rc(%d)", pbit_to_q.dev_id, rc);
                ret = RDPA_DRV_LLID_GET;
                goto p_to_q_ioctl_exit;
            }
            break;

        case RDPA_IOCTL_DEV_NONE:
            CMD_P_TO_Q_LOG_DEBUG(
                "dev none for table level operation");
            break;

        default:
            CMD_P_TO_Q_LOG_DEBUG(
                "Invalid IOCTL dev type %d", pbit_to_q.dev_type);
            ret = RDPA_DRV_ERROR;
            goto p_to_q_ioctl_exit;
    }

    switch (pbit_to_q.cmd)
    {
        case RDPA_IOCTL_P_TO_Q_CMD_GET:
        {
            int i;
            int tbl_idx;
            bdmf_number pbit_qid;
 
            CMD_P_TO_Q_LOG_DEBUG("RDPA_IOCTL_P_TO_Q_CMD_GET dev=%d if=%d", \
                pbit_to_q.dev_type, pbit_to_q.dev_id);

            pbit_to_q.found = FALSE;
            
            for (tbl_idx = 0; tbl_idx < RDPA_PBIT_TO_PRTY_MAX_TABLES; tbl_idx++)
            {
                p_to_q_obj = NULL;
                rc = rdpa_pbit_to_queue_get(tbl_idx, &p_to_q_obj);
                if (0 != rc)
                    continue;

                if ((dev_obj==NULL) || pbit_to_q_dev_link_check(dev_obj, p_to_q_obj))
                {
                    pbit_to_q.found = TRUE;
                    break;
                }
                else
                {
                    bdmf_put(p_to_q_obj);
                }
            }

            if (!pbit_to_q.found)
                goto p_to_q_ioctl_exit;
           
            for (i = 0; i < TOTAL_PBIT_NUM; i++)
            {
                rc = rdpa_pbit_to_queue_pbit_map_get(p_to_q_obj, i, &pbit_qid);
                if (0 != rc)
                {
                    CMD_P_TO_Q_LOG_DEBUG(
                        "rdpa_pbit_to_queue_pbit_map_get() failed:" \
                        " table(%u) pbit(%u) rc(%d)", tbl_idx, i, rc);
                    ret = RDPA_DRV_P_TO_Q_QOS_MAP_GET;
                    break;
                }
                pbit_to_q.pbit_q_map[i] = pbit_qid;
            }
            
            bdmf_put(p_to_q_obj);
            break;
        }

        case RDPA_IOCTL_P_TO_Q_CMD_SET:
        {
            int tbl_idx;
            BOOL has_same_cfg = FALSE;
            BOOL dev_has_table = FALSE;

            /* dev's current linked table obj */
            bdmf_object_handle cur_p_to_q_obj = NULL;
            
            BDMF_MATTR(pbit_to_q_attrs, rdpa_pbit_to_queue_drv());

            CMD_P_TO_Q_LOG_DEBUG("RDPA_IOCTL_P_TO_Q_CMD_SET");

            /* check if the dev itself has table linked already*/
            for (tbl_idx = 0; tbl_idx < RDPA_PBIT_TO_PRTY_MAX_TABLES; tbl_idx++)
            {
                cur_p_to_q_obj = NULL;
                rc = rdpa_pbit_to_queue_get(tbl_idx, &cur_p_to_q_obj);
                if (0 != rc)
                    continue;

                /* no dev is assgined, modify all tables */
                if (dev_obj == NULL)
                {
                    ret = pbit_to_q_map_set(cur_p_to_q_obj, pbit_to_q);
                    dev_has_table = TRUE;
                }
                else if (pbit_to_q_dev_link_check(dev_obj, cur_p_to_q_obj))
                {
                    dev_has_table = TRUE;
                    break;
                }

                bdmf_put(cur_p_to_q_obj);
            }
            
            if (rc == BDMF_ERR_NOENT)
                rc = BDMF_ERR_OK;
            
            /* create table if no table exists for non dev mode */
            if (dev_obj == NULL)
            {
                if (!dev_has_table)
                {
                    cur_p_to_q_obj = NULL;
                    rc = bdmf_new_and_set(
                        rdpa_pbit_to_queue_drv(), NULL, pbit_to_q_attrs, &p_to_q_obj);
                    if (rc || (p_to_q_obj == NULL))
                    {
                        CMD_P_TO_Q_LOG_ERROR("bdmf_new_and_set() failed: pbit_to_q rc(%d)", rc);
                        ret = RDPA_DRV_NEW_D_TO_P_ALLOC;
                        goto p_to_q_ioctl_exit;
                    }
                    rc = pbit_to_q_map_set(p_to_q_obj, pbit_to_q);
                }
            }
            else
            {
                if (dev_has_table && 1 == pbit_to_q_link_dev_num_get(cur_p_to_q_obj))
                {
                    /* if the dev has table already and the table is only used by the dev,
                                  we just use this table to do the new cfg later. */
                    p_to_q_obj = cur_p_to_q_obj;
                    ret = pbit_to_q_map_set(p_to_q_obj, pbit_to_q);
                    bdmf_put(p_to_q_obj);
                }
                else 
                {
                    /* check if there's existing table the matches the new cfg */    
                    for (tbl_idx = 0; tbl_idx < RDPA_PBIT_TO_PRTY_MAX_TABLES; tbl_idx++)
                    {
                        p_to_q_obj = NULL;
                        rc = rdpa_pbit_to_queue_get(tbl_idx, &p_to_q_obj);
                        if (0 != rc)
                            continue;

                        has_same_cfg = pbit_to_q_same_config_check(p_to_q_obj, pbit_to_q);
                        if (has_same_cfg)
                            break;
                        bdmf_put(p_to_q_obj);
                    }

                    /* start the process when there's existing table that matches the new config.
                       Just link this table to the dev. */
                    if (has_same_cfg)
                    {
                        /* check if this table has linked to the dev already.
                                      if yes, do nothing.
                                      if no, link it to the dev. */
                        if (!pbit_to_q_dev_link_check(dev_obj, p_to_q_obj))
                        {
                            /* before link the new table, 
                                            unlink the current table of the dev first. */
                            if (dev_has_table)
                                bdmf_unlink(cur_p_to_q_obj, dev_obj);

                            /* link the table */
                            rc = bdmf_link(p_to_q_obj, dev_obj, NULL);
                        }

                        if (dev_has_table)
                            bdmf_put(cur_p_to_q_obj);

                        bdmf_unlink(p_to_q_obj, dev_obj);
                        bdmf_link(p_to_q_obj, dev_obj, NULL);
                        
                        if (cur_p_to_q_obj != p_to_q_obj)
                            bdmf_put(p_to_q_obj);
                    }
                    else
                    {
                    /* if the dev has table already, but the table is used by other dev also,
                                  we unlink the current table from the dev here. */
                    if (dev_has_table)
                    {
                        bdmf_unlink(cur_p_to_q_obj, dev_obj);
                        bdmf_put(cur_p_to_q_obj);
                    }

                    /* create a new table */
                    p_to_q_obj = NULL;
                    rc = bdmf_new_and_set(
                        rdpa_pbit_to_queue_drv(), NULL, pbit_to_q_attrs, &p_to_q_obj);
                    if (rc || (p_to_q_obj == NULL))
                    {
                        CMD_P_TO_Q_LOG_ERROR(
                            "bdmf_new_and_set() failed: pbit_to_q rc(%d)", rc);
                        ret = RDPA_DRV_NEW_D_TO_P_ALLOC;
                        goto p_to_q_ioctl_exit;
                    }

                    rc = bdmf_link(p_to_q_obj, dev_obj, NULL);
                    ret = pbit_to_q_map_set(p_to_q_obj, pbit_to_q);
                    }

                }
            }
            break;
        }
        default:
            CMD_P_TO_Q_LOG_ERROR("Invalid IOCTL cmd %d", pbit_to_q.cmd);
            rc = RDPA_DRV_ERROR;
    }

    if (rc) {
        CMD_P_TO_Q_LOG_ERROR(
            "rdpa_cmd_pbit_to_q_ioctl() OUT: FAILED: rc(%d)", rc);
    }

p_to_q_ioctl_exit:

    copy_to_user(
        userPtoQ_p, &pbit_to_q, sizeof(rdpa_drv_ioctl_pbit_to_q_t));
    
    if (dev_obj)
        bdmf_put(dev_obj);
    
    if (ret) 
    {
        CMD_P_TO_Q_LOG_ERROR(
            "rdpa_cmd_pbit_to_q_ioctl() OUT: FAILED: cmd(%u) ret(%d)", \
            pbit_to_q.cmd, ret);
    }

    bdmf_unlock();
    return ret;
}

/*******************************************************************************
 *
 * Function: rdpa_cmd_pbit_to_q_init
 *
 * Initializes the RDPA PBIT TO Q API.
 *
 *******************************************************************************/
void rdpa_cmd_pbit_to_q_init(void)
{
    CMD_P_TO_Q_LOG_DEBUG("RDPA PBIT TO Q INIT");
}

EXPORT_SYMBOL(rdpa_cmd_pbit_to_q_ioctl);
EXPORT_SYMBOL(rdpa_cmd_pbit_to_q_init);

