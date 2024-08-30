#include "bcmtypes.h"
#include "rdpa_types.h"
#include "tmctl_api.h"
#include "tmctl_api_plat.h"
#include "tmctl_bdmf_rdpa.h"
#include "tmctl_api_runner.h"
#include "rdpactl_api.h"

static int get_dscp_to_pbit_tables(void);

static BOOL is_obj_linked(bdmf_object_handle bdmf_obj, bdmf_object_handle dev_obj)
{
    bdmf_link_handle obj_link = BDMF_NULL;

    obj_link = bdmf_get_next_us_link(bdmf_obj, BDMF_NULL);
    if (obj_link)
    {
        do {
            if (bdmf_us_link_to_object(obj_link) == dev_obj)
            {
                return TRUE;
            }
            obj_link = bdmf_get_next_us_link(bdmf_obj, obj_link);
        } while(obj_link);
    }

    /* compare DS links obj*/
    obj_link = bdmf_get_next_ds_link(bdmf_obj, BDMF_NULL);
    if (obj_link)
    {
        do {
            if (bdmf_ds_link_to_object(obj_link) == dev_obj)
            {
                return TRUE;
            }
            obj_link = bdmf_get_next_ds_link(bdmf_obj, obj_link);
        } while(obj_link);
    }

    return FALSE;
}


static BOOL is_dscp_to_pbit_same_map(BOOL remark, bdmf_object_handle dscp_to_pbit_obj,
                                        uint32_t dscp_pbit_map[])
{
    int i = 0, rc = BDMF_ERR_OK;
    rdpa_pbit dscp_pbit;
    bdmf_boolean qos_map;

    rdpa_dscp_to_pbit_qos_mapping_get(dscp_to_pbit_obj, &qos_map);
    if (qos_map == remark)
    {
        return FALSE;
    }

    for (i = 0; i < TOTAL_DSCP_NUM; i++)
    {
        /* masked, ignore it */
        if (dscp_pbit_map[i] == U32_MAX)
            continue;

        rc = rdpa_dscp_to_pbit_dscp_map_get(dscp_to_pbit_obj, i, &dscp_pbit);
        if (0 != rc)
        {
            tmctl_error(
                "rdpa_dscp_to_pbit_dscp_map_get() failed:" \
                " pbit(%u) rc(%d)", i, rc);
            return FALSE;
        }
        if (dscp_pbit != dscp_pbit_map[i])
            break;
    }

    if (i == TOTAL_DSCP_NUM)
    {
        return TRUE;
    }

    return FALSE;
}

static int create_dscp_to_pbit_table(BOOL remark, bdmf_object_handle * d_to_p_obj)
{
    int rc = BDMF_ERR_OK;
    bdmf_mattr_handle mattr;

    mattr = bdmf_mattr_alloc(rdpa_dscp_to_pbit_drv());
    if (!mattr)
    {
        tmctl_error("create_dscp_to_pbit_table failed: cannot allocate context memory");
        return BDMF_ERR_NOMEM;
    }

    if (!remark)
    {
        rdpa_dscp_to_pbit_qos_mapping_set(mattr, TRUE);
        rdpa_dscp_to_pbit_table_set(mattr, 0);
    }

    rc = bdmf_new_and_set(rdpa_dscp_to_pbit_drv(), BDMF_NULL, mattr, d_to_p_obj);
    if (rc)
    {
        tmctl_error("create_dscp_to_pbit_table failed: rc(%d)", rc);
    }
    bdmf_mattr_free(mattr);
    return rc;
}

static int set_dscp_to_pbit_map(bdmf_object_handle d_to_p_obj,
                                        uint32_t dscp_pbit_map[])
{
    int i = 0, rc = BDMF_ERR_OK;

    for (i = 0; i < TOTAL_DSCP_NUM; i++)
    {
        if (dscp_pbit_map[i] == U32_MAX)
            continue;
        rc = rdpa_dscp_to_pbit_dscp_map_set(
            d_to_p_obj, i, dscp_pbit_map[i]);

        if (rc)
        {
            tmctl_error("rdpa_dscp_to_pbit_dscp_map_set()" \
                " failed: dscp(%u) pbit(%u) rc(%d)", \
                i, dscp_pbit_map[i], rc);
            break;
        }
    }

    return rc;
}

tmctl_ret_e tmctl_RdpaGetDscpToPbit(tmctl_devType_e devType,
                        tmctl_if_t *if_p, tmctl_dscpToPbitCfg_t* cfg_p)
{
    bdmf_object_handle d_to_p_obj = BDMF_NULL;
    bdmf_object_handle dev_obj = BDMF_NULL;
    int rc = BDMF_ERR_OK, i;
    uint8_t tbl_id;
    int tables_num;
    bdmf_boolean qos_map = FALSE, tbl_found = FALSE;
    rdpa_pbit dscp_map;

    tables_num = get_dscp_to_pbit_tables();
    if(tables_num == 0)
        return TMCTL_ERROR;

    for (tbl_id = 0; tbl_id < tables_num; tbl_id++)
    {
        rc = rdpa_dscp_to_pbit_get(tbl_id, &d_to_p_obj);
        if (rc)
            continue;

        rc = rdpa_dscp_to_pbit_qos_mapping_get(d_to_p_obj, &qos_map);
        if (!rc)
        {
          /* qos map only can use table 0 */
           if (qos_map && !cfg_p->remark && (tbl_id==0))
            {
                tbl_found = TRUE;
            }
            else if (cfg_p->remark && !qos_map)
            {
                if (cfg_p->devType == TMCTL_DEV_NONE)
                    tbl_found = TRUE;
                else
                {
                    if (!(get_tm_owner(devType, if_p, &dev_obj)))
                    {
                        if (is_obj_linked(d_to_p_obj, dev_obj))
                            tbl_found = TRUE;
                        if (dev_obj)
                            bdmf_put(dev_obj);
                    }
                }
            }
            if (tbl_found == TRUE)
                break;
        }
        else
        {
            tmctl_error("rdpa_dscp_to_pbit_qos_mapping_get()" \
                " failed: table(%u) rc(%d)", tbl_id, rc);
         }

        bdmf_put(d_to_p_obj);
        d_to_p_obj = BDMF_NULL;
    }

    if (tbl_found)
    {
        for (i = 0; i < TOTAL_DSCP_NUM; i++)
        {
            rc = rdpa_dscp_to_pbit_dscp_map_get(d_to_p_obj, i, &dscp_map);

            if (rc)
            {
                tmctl_error("rdpa_dscp_to_pbit_dscp_map_get()" \
                    " failed: table(%u) dscp(%u) rc(%d)", tbl_id, i, rc);
                break;
            }
            else
            {
                cfg_p->dscp[i]= dscp_map;
            }
        }
    }

    if (d_to_p_obj)
        bdmf_put(d_to_p_obj);

    if (rc == BDMF_ERR_NOENT)
    {
        tmctl_debug("no table found, ignore this error\n");
        rc  = BDMF_ERR_OK;
    }

    return (rc)?  TMCTL_ERROR : TMCTL_SUCCESS;

}


tmctl_ret_e tmctl_RdpaSetDscpToPbit(tmctl_devType_e devType,
                        tmctl_if_t *if_p, tmctl_dscpToPbitCfg_t* cfg_p)
{
    bdmf_object_handle d_to_p_obj = BDMF_NULL;
    bdmf_object_handle dev_obj = BDMF_NULL;
    bdmf_boolean qos_map;
    int rc = BDMF_ERR_OK;
    uint8_t tbl_id;
    int tables_num;

    if (cfg_p->remark == 0)
    {
        /* only table 0 is used as qos table */
        rc = rdpa_dscp_to_pbit_get(0, &d_to_p_obj);
        if (!rc)
        {
            rdpa_dscp_to_pbit_qos_mapping_get(d_to_p_obj, &qos_map);
            if (!qos_map)
            {
                bdmf_put(d_to_p_obj);
                bdmf_destroy(d_to_p_obj);
                rc = BDMF_ERR_NOENT;
            }
            else
            {
               rc = set_dscp_to_pbit_map(d_to_p_obj, cfg_p->dscp);
            }
        }

        if (rc == BDMF_ERR_NOENT)
        {
            rc = create_dscp_to_pbit_table(0, &d_to_p_obj);
            if (!rc)
            {
                set_dscp_to_pbit_map(d_to_p_obj, cfg_p->dscp);
                d_to_p_obj = BDMF_NULL;
            }
        }
    }
    else
    {
        bdmf_number tbl_id_avail = -1;
        BOOL link_check =  TRUE;
        tables_num = get_dscp_to_pbit_tables();
        if(tables_num == 0)
            return TMCTL_ERROR;

        for (tbl_id = 0; tbl_id < tables_num; tbl_id++)
        {
            rc = rdpa_dscp_to_pbit_get(tbl_id, &d_to_p_obj);
            if (rc)
                continue;

            rdpa_dscp_to_pbit_qos_mapping_get(d_to_p_obj, &qos_map);
            if (qos_map)
            {
                bdmf_put(d_to_p_obj);
                continue;
            }
            /* override all valid tables */
            if (cfg_p->devType == TMCTL_DEV_NONE)
            {
                set_dscp_to_pbit_map(d_to_p_obj, cfg_p->dscp);
                if (tbl_id_avail == -1)
                    tbl_id_avail = tbl_id;
            }
            else
            {
                /* XXX: BUG? dev_obj is set to NULL */
                /* unlink current table */
                if (link_check && is_obj_linked(d_to_p_obj, dev_obj))
                {
                    bdmf_unlink(d_to_p_obj, dev_obj);
                    link_check = FALSE;
                }

                if ((tbl_id_avail == -1) && is_dscp_to_pbit_same_map(1, d_to_p_obj,cfg_p->dscp))
                    tbl_id_avail = tbl_id;
            }

            bdmf_put(d_to_p_obj);
            d_to_p_obj = BDMF_NULL;
            rc = BDMF_ERR_OK;
        }

        if (rc == BDMF_ERR_NOENT)
            rc = BDMF_ERR_OK;

        if (tbl_id_avail == -1)
        {
            rc = create_dscp_to_pbit_table(1, &d_to_p_obj);
            if (!rc)
            {
                set_dscp_to_pbit_map(d_to_p_obj, cfg_p->dscp);
                rdpa_dscp_to_pbit_table_get(d_to_p_obj, &tbl_id_avail);
            }
            d_to_p_obj = BDMF_NULL;
        }

        if ((cfg_p->devType != TMCTL_DEV_NONE) && (tbl_id_avail != -1))
        {
            bdmf_object_handle tm_owner_obj = BDMF_NULL;
            rc = get_tm_owner(devType, if_p, &tm_owner_obj);
            if (!rc)
            {
                rc = rdpa_dscp_to_pbit_get(tbl_id_avail, &d_to_p_obj);
                if (!rc)
                    bdmf_link(d_to_p_obj, tm_owner_obj, BDMF_NULL);
                else
                    tmctl_error("rdpa_dscp_to_pbit_get() failed rc(%d)", rc);
                if (tm_owner_obj)
                    bdmf_put(tm_owner_obj);
            }
            else
            {
                tmctl_error("get_tm_owner() failed dev[%s], , rc(%d)",
                        ifp_to_name(devType, if_p), rc);
            }
        }
     }

    /* XXX: BUG? dev_obj is set to NULL */
    if (dev_obj)
        bdmf_put(dev_obj);
    if (d_to_p_obj)
        bdmf_put(d_to_p_obj);

    return (rc)?  TMCTL_ERROR : TMCTL_SUCCESS;
}

#if defined(CONFIG_BCM_PLATFORM_RDP_PRV)
tmctl_ret_e tmctl_RdpaSetTc2Queue(int table_index, tmctl_tcToQCfg_t* cfg_p)
{
    bdmf_object_handle tc_to_q_obj = BDMF_NULL;
    bdmf_mattr_handle mattr = BDMF_NULL;
    int rc, i;
    BOOL new_obj = FALSE;

    rc = rdpa_tc_to_queue_get(table_index, &tc_to_q_obj);
    if (rc == BDMF_ERR_NOENT)
    {
        /* Configure tc to q table */
        mattr = bdmf_mattr_alloc(rdpa_tc_to_queue_drv());
        if (!mattr)
        {
            tmctl_error("bdmf_mattr_alloc failed");
            goto error;
        }

        rdpa_tc_to_queue_table_set(mattr, table_index);
        rc = bdmf_new_and_set(rdpa_tc_to_queue_drv(), BDMF_NULL, mattr,
            &tc_to_q_obj);
        if (rc != 0)
        {
            tmctl_error("bdmf_new_and_set failed, rc(%d)", rc);
            goto error;
        }
        new_obj = TRUE;
    }
    else if (rc != 0)
    {
        tmctl_error("rdpa_tc_to_queue_get failed, rc(%d)", rc);
        goto error;
    }

    for (i = 0; i < TOTAL_TC_NUM; i++)
    {
        if (cfg_p->tc[i] == -1)
        {
            continue;
        }

        rc = rdpa_tc_to_queue_tc_map_set(tc_to_q_obj, i, cfg_p->tc[i]);
        if (rc != 0)
        {
            tmctl_error("rdpa_tc_to_queue_tc_map_set()"
              " failed: tc(%u), queue(%u), rc(%d)",
              i, cfg_p->tc[i], rc);
        }
    }

    if ((tc_to_q_obj != BDMF_NULL) && (new_obj == FALSE))
    {
        bdmf_put(tc_to_q_obj);
    }

    return TMCTL_SUCCESS;

error:
    if (mattr)
    {
        bdmf_mattr_free(mattr);
    }

    if (tc_to_q_obj)
    {
        bdmf_put(tc_to_q_obj);
    }

    return TMCTL_ERROR;
}
#endif
static bdmf_object_handle getPbitToQTableByOnwer(tmctl_devType_e devType, bdmf_object_handle owner)
{
    bdmf_object_handle tbl = BDMF_NULL;
    tmctl_ret_e ret;    
    switch(devType)
    {
        case  TMCTL_DEV_ETH:
            ret = rdpa_port_pbit_to_queue_get(owner, &tbl);
            break;
        case TMCTL_DEV_EPON:
            ret = rdpa_llid_pbit_to_queue_get(owner, &tbl);
            break;
        case TMCTL_DEV_GPON:
            ret = rdpa_llid_pbit_to_queue_get(owner, &tbl);
            break;
        default:
            ret = TMCTL_UNSUPPORTED;
    }

    if (ret)
    {
        tbl = BDMF_NULL;
        tmctl_error("Failed to get pbit to queue table ret[%d]", ret);
    }
    return tbl;
}

static bdmf_object_handle getPbitToQTableByIf(tmctl_devType_e devType, tmctl_if_t *if_p)
{
    bdmf_object_handle owner = BDMF_NULL;
    tmctl_ret_e ret;

    ret = get_tm_owner(devType, if_p, &owner);
    if (ret)
    {
        tmctl_error("Failed to get tm owner for %s, ret[%d]", ifp_to_name(devType, if_p), ret);
        return BDMF_NULL;
    }
    return getPbitToQTableByOnwer(devType, owner); 
}

tmctl_ret_e tmctlRdpa_getPbitToQ(tmctl_devType_e devType, tmctl_if_t *if_p, tmctl_pbitToQCfg_t* cfg_p)
{
    int ret;
    bdmf_object_handle tbl;
    const char *name = ifp_to_name(devType, if_p);
    int pbit, qid;
    
    tbl = getPbitToQTableByIf(devType, if_p);
    if (!tbl)
    {
        tmctl_error("There is no pbit to queue table linked to %s\n", name);
        return TMCTL_NOT_FOUND;
    }
    
    for (pbit = 0; pbit < TOTAL_PBIT_NUM; pbit++)
    {
        ret = rdpa_pbit_to_queue_pbit_map_get(tbl, pbit, (bdmf_number *)&qid);
        if (!ret)
            cfg_p->pbit[pbit] = qid;

        /* ignore not configured entries */
        if (ret == BDMF_ERR_NOENT)
            ret = 0;
    }
    return ret;
}

bdmf_object_handle allocateAndLinkTable(tmctl_devType_e devType, tmctl_if_t *if_p)
{
    bdmf_object_handle owner = BDMF_NULL;
    bdmf_object_handle tbl;
    const char *name = ifp_to_name(devType, if_p);
    int ret;

    ret = get_tm_owner(devType, if_p, &owner);
    if (ret)
    {
        tmctl_error("Failed to get tm owner for %s, ret[%d]", name , ret);
        return BDMF_NULL;
    }

    ret = bdmf_new_and_set(rdpa_pbit_to_queue_drv(), BDMF_NULL, BDMF_NULL, &tbl);
    if (ret)
    {
        tmctl_error("Failed to allocate pbit to queue table for %s, ret[%d]", name, ret);
        return BDMF_NULL;
    }

    
    ret = bdmf_link(tbl, owner, NULL);
    if (ret)
    {
        tmctl_error("Failed to link pbit to  queue table to %s, ret[%d]", name, ret);
        bdmf_destroy(tbl);
        return BDMF_NULL;
    }
    return tbl;
}

static int getTableLinkCount(bdmf_object_handle tbl)
{
    int count = 0;
    bdmf_link_handle obj_link = BDMF_NULL;
    
    obj_link = bdmf_get_next_us_link(tbl, BDMF_NULL);
    if (obj_link)
    {
        do {
            count++;
            obj_link = bdmf_get_next_us_link(tbl, obj_link);
        } while(obj_link);
    }

    obj_link = bdmf_get_next_ds_link(tbl, BDMF_NULL);
    if (obj_link)
    {
        do {
            count++;
            obj_link = bdmf_get_next_ds_link(tbl, obj_link);
        } while(obj_link);
    }
    return count;
}

/* This function will return not shared table linked to port/tcont/llid allocated one if needed  */
bdmf_object_handle getUniquePbitToQueueTable(tmctl_devType_e devType, tmctl_if_t *if_p)
{
    bdmf_object_handle tbl;
    const char *name = ifp_to_name(devType, if_p);
    tbl = getPbitToQTableByIf(devType, if_p);

    if (tbl)
    {   
        int count; 
        bdmf_object_handle owner = BDMF_NULL;

        /* make sure table is used exclusivly, unlink and reallocate if needed*/
        count = getTableLinkCount(tbl);
        if (count == 1)
            goto Exit;
        /* bdmf_unlink can handle null if no owner is found */
        (void)get_tm_owner(devType, if_p, &owner);
        bdmf_unlink(tbl, owner);
    }
    tbl =  allocateAndLinkTable(devType, if_p);

Exit:
    if (!tbl)
        tmctl_error("could not find/allocate unique pbit to queue table for %s\n", name);
    return tbl;
}

static int pbitToQMapSet(bdmf_object_handle tbl, tmctl_pbitToQCfg_t* cfg)
{
    int p_indx = 0;
    int  ret = BDMF_ERR_OK;

    for (p_indx = 0; p_indx < TOTAL_PBIT_NUM; p_indx++)
    {
        if (cfg->pbit[p_indx] == U32_MAX)
            continue;

        ret = rdpa_pbit_to_queue_pbit_map_set(tbl, p_indx, cfg->pbit[p_indx]);
        /*nothing to do with error report and contunue */
        if (ret)
            tmctl_error("failed to map pbit %d to queue %d\n", p_indx , cfg->pbit[p_indx]);
    }
    return ret;
}

tmctl_ret_e tmctlRdpa_setPbitToQ(tmctl_devType_e devType, tmctl_if_t *if_p, tmctl_pbitToQCfg_t* cfg)
{
    tmctl_ret_e ret = 0; 
    bdmf_object_handle tbl = BDMF_NULL;
   
    /*device not provided update all tables */
    if (devType ==  TMCTL_DEV_NONE)
    {
        while ((tbl = bdmf_get_next(rdpa_pbit_to_queue_drv(), tbl, NULL)))
        {
            ret = pbitToQMapSet(tbl, cfg);
            if (ret)    
                tmctl_error("Failed to configure pbit to queue during updating of all tables \n");
        }
    }
    else
    {
        tbl = getUniquePbitToQueueTable(devType, if_p);
        if (tbl)
                ret = pbitToQMapSet(tbl, cfg);
    }
    if (ret)    
    {    
        tmctl_error("Failed to configure pbit to queue table for %s\n",
                ifp_to_name(devType, if_p));
    }
    return ret;
}

tmctl_ret_e tmctlRdpa_getForceDscpToPbit(tmctl_dir_e dir, BOOL* enable_p)
{
    bdmf_object_handle system_obj;
    rdpa_system_cfg_t system_cfg;

    /* errors are ignored can't happen in normal operation */
    (void)rdpa_system_get(&system_obj);
    (void)rdpa_system_cfg_get(system_obj, &system_cfg);

    bdmf_put(system_obj);

    if (dir == TMCTL_DIR_DN)
        *enable_p = system_cfg.force_dscp_to_pbit_ds;
    else
        *enable_p = system_cfg.force_dscp_to_pbit_us;

    return 0;
}  

tmctl_ret_e tmctlRdpa_setForceDscpToPbit(tmctl_dir_e dir, BOOL* enable_p)
{
    bdmf_object_handle system_obj;
    rdpa_system_cfg_t system_cfg;
    tmctl_ret_e ret;

    if (rdpa_system_get(&system_obj))
    {
        tmctl_error("tmctlRdpa_setForceDscpToPbit() failed");
        return TMCTL_ERROR;
    }
    ret = rdpa_system_cfg_get(system_obj, &system_cfg);

    if (dir == TMCTL_DIR_DN)
        system_cfg.force_dscp_to_pbit_ds = *enable_p;
    else
        system_cfg.force_dscp_to_pbit_us = *enable_p;

    ret = rdpa_system_cfg_set(system_obj, &system_cfg);
    bdmf_put(system_obj);
    return ret;
}

tmctl_ret_e tmctlRdpa_getPktBasedQos(tmctl_dir_e dir,
                                  tmctl_qosType_e type,
                                  BOOL* enable_p)
{
    int rc;
    BOOL enable;
    tmctl_ret_e ret = TMCTL_SUCCESS;

    rc = rdpaCtl_PktBasedQosGet(dir, type, &enable);
    if (rc != 0)
    {
        tmctl_error("rdpaCtl_PktBasedQosGet failed, dir=%d type=%d rc=%d",
                dir, type, rc);
        return TMCTL_ERROR;
    }
    *enable_p = enable;

    return ret;

}

tmctl_ret_e tmctlRdpa_setPktBasedQos(tmctl_dir_e dir,
                                  tmctl_qosType_e type,
                                  BOOL* enable_p)
{
   int rc;
   tmctl_ret_e ret = TMCTL_SUCCESS;

   rc = rdpaCtl_PktBasedQosSet(dir, type, *enable_p);
   if (rc != 0)
   {
      tmctl_error(
        "rdpaCtl_PktBasedQosSet failed, dir=%d type=%d enable=%d rc=%d", \
                  dir, type, (*enable_p), rc);
      return TMCTL_ERROR;
   }

   return ret;
}


static int get_dscp_to_pbit_tables(void)
{
    bdmf_object_handle system = BDMF_NULL;
    rdpa_system_resources_t system_resources = {};
    int rc;

    if ((rc = rdpa_system_get(&system)) || (system == BDMF_NULL))
    {
        tmctl_error("rdpa_system_get() failed, rc=%d", rc);
        return 0;
    }

    if ((rc = rdpa_system_system_resources_get(system, &system_resources)))
    {
        tmctl_error("rdpa_system_system_resources_get() failed, rc=%d", rc);
        return 0; 
    }
    bdmf_put(system);
    /* Return the number of non_global tables (qos_mapping no) + 1 global table (qos_mapping yes) */
    return system_resources.num_dscp2pbit_tables + 1;
}

