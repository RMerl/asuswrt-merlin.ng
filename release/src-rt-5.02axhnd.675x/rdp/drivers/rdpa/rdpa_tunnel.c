/*
* <:copyright-BRCM:2013-2015:proprietary:standard
* 
*    Copyright (c) 2013-2015 Broadcom 
*    All Rights Reserved
* 
*  This program is the proprietary software of Broadcom and/or its
*  licensors, and may only be used, duplicated, modified or distributed pursuant
*  to the terms and conditions of a separate, written license agreement executed
*  between you and Broadcom (an "Authorized License").  Except as set forth in
*  an Authorized License, Broadcom grants no license (express or implied), right
*  to use, or waiver of any kind with respect to the Software, and Broadcom
*  expressly reserves all rights in and to the Software and all intellectual
*  property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
*  NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
*  BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
* 
*  Except as expressly set forth in the Authorized License,
* 
*  1. This program, including its structure, sequence and organization,
*     constitutes the valuable trade secrets of Broadcom, and you shall use
*     all reasonable efforts to protect the confidentiality thereof, and to
*     use this information only in connection with your use of Broadcom
*     integrated circuit products.
* 
*  2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*     AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*     WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
*     RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
*     ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
*     FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
*     COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
*     TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
*     PERFORMANCE OF THE SOFTWARE.
* 
*  3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
*     ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
*     INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
*     WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
*     IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
*     OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
*     SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
*     SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
*     LIMITED REMEDY.
 :>
*/

/*
 * rdpa_tunnnel.c
 *
 *  Created on: Dec 12, 2018
 *      Author: Ariel Lior
 */


#include "bdmf_dev.h"
#include "rdpa_api.h"
#include "rdpa_common.h"
#include "rdd.h"
#include "rdd_tunnels_parsing.h"

uint32_t tunnel_flows_num;

/* Tunnel object private data */
typedef struct {
    bdmf_index index;           /**< tunnel index */
    uint32_t ref_cnt;           /**< Reference count */
    rdpa_tunnel_cfg_t cfg;      /**< Tunnel Configuration */
} tunnel_drv_priv_t;

static struct bdmf_object *tunnel_objects[RDPA_MAX_TUNNELS];
const bdmf_attr_enum_table_t rdpa_tunnel_type_enum_table =
{
    .type_name = "tunnel_type", .help = "GRE_L2 / GRE_L3 / DS_LITE",
    .values = {
        {"L2oGRE", rdpa_tunnel_l2gre},
        {"L3oGRE", rdpa_tunnel_l3gre},
        {"DS_LITE", rdpa_tunnel_dslite},
        {NULL, 0}
    }
};

static int tunnel_rdd_update(struct bdmf_object *mo, rdpa_tunnel_cfg_t *cfg)
{
    tunnel_drv_priv_t *tunnel = (tunnel_drv_priv_t *)bdmf_obj_data(mo);
    RDD_TUNNEL_ENTRY_DTS rdd_tunnel_entry;
    int rc;
#ifdef XRDP    
    uint32_t crc_ip;
#endif

    rdd_tunnel_entry.tunnel_type = cfg->tunnel_type;
    rdd_tunnel_entry.layer3_offset = cfg->layer3_offset;
    rdd_tunnel_entry.ip_family = cfg->local_ip.family;
    rdd_tunnel_entry.tunnel_header_length = cfg->tunnel_header_length;
#ifndef XRDP
    rdd_tunnel_entry.local_ip = cfg->local_ip.addr.ipv4;
    memcpy(rdd_tunnel_entry.tunnel_header, cfg->tunnel_header, cfg->tunnel_header_length);
    rc = rdd_tunnel_cfg_set(tunnel->index, &rdd_tunnel_entry);
#else
    rdd_tunnel_entry.gre_proto_offset = cfg->gre_proto_offset;
    if (cfg->tunnel_type == rdpa_tunnel_dslite)
        rc = rdd_tunnel_cfg_set(DS_LITE_TUNNEL_INDEX, &rdd_tunnel_entry, (psram_mem_memory_data *)cfg->tunnel_header);
    else
    {
        if (cfg->local_ip.family == bdmf_ip_family_ipv4)
            rdd_tunnel_entry.local_ip = cfg->local_ip.addr.ipv4;
        else
        {
            rdd_crc_ipv6_addr_calc(&(cfg->local_ip), &crc_ip);
            rdd_tunnel_entry.local_ip = crc_ip;
        }
        rc = rdd_tunnel_cfg_set(tunnel->index, &rdd_tunnel_entry, (psram_mem_memory_data *)cfg->tunnel_header);
    }
#endif    
    if (rc)
    {
        BDMF_TRACE_RET_OBJ(BDMF_ERR_PARM, mo,
            "Failed to configure RDD tunnel %d, error %d\n", (int)tunnel->index, rc);
    }
    return 0;
}

int  _rdpa_tunnel_ref_count_increase(struct bdmf_object *mo)
{
    tunnel_drv_priv_t *tunnel = (tunnel_drv_priv_t *)bdmf_obj_data(mo);

    tunnel->ref_cnt++;
    return 0;
}

int  _rdpa_tunnel_ref_count_decrease(struct bdmf_object *mo)
{
    tunnel_drv_priv_t *tunnel = (tunnel_drv_priv_t *)bdmf_obj_data(mo);

    tunnel->ref_cnt--;
    return 0;
}

/** This optional callback is called called at object init time
 *  before initial attributes are set.
 *  If function returns error code !=0, object creation is aborted
 */
static int tunnel_pre_init(struct bdmf_object *mo)
{
    tunnel_drv_priv_t *tunnel = (tunnel_drv_priv_t *)bdmf_obj_data(mo);

    tunnel->index = BDMF_INDEX_UNASSIGNED; /* unassigned */
    return 0;
}


/** This optional callback is called at object init time
 * after initial attributes are set.
 * Its work is:
 * - make sure that all necessary attributes are set and make sense
 * - allocate dynamic resources if any
 * - assign object name if not done in pre_init
 * - finalise object creation
 * If function returns error code !=0, object creation is aborted
 */
static int tunnel_post_init(struct bdmf_object *mo)
{
    int rc;
    tunnel_drv_priv_t *tunnel = (tunnel_drv_priv_t *)bdmf_obj_data(mo);

    /* If Tunnel index is set - make sure it is unique.
     * Otherwise, assign free
     */
    if (tunnel->index < 0)
    {
        int i;

        /* Find and assign free index */
        for (i = 0; i < RDPA_MAX_TUNNELS; i++)
        {
            if (!tunnel_objects[i])
            {
                tunnel->index = i;
                break;
            }
        }
    }
    if ((unsigned)tunnel->index >= RDPA_MAX_TUNNELS)
        BDMF_TRACE_RET(BDMF_ERR_PARM, "Too many tunnels or index %ld is out of range\n", tunnel->index);

    /* set object name */
    snprintf(mo->name, sizeof(mo->name), "tunnel, index=%ld", tunnel->index);
    if (tunnel_objects[tunnel->index])
        BDMF_TRACE_RET(BDMF_ERR_ALREADY, "%s already exists\n", mo->name);

    rc = tunnel_rdd_update(mo, &tunnel->cfg);
    if (rc < 0)
        return rc;

    tunnel->ref_cnt = 0;
    tunnel_objects[tunnel->index] = mo;

    return 0;
}


static void tunnel_destroy(struct bdmf_object *mo)
{
    tunnel_drv_priv_t *tunnel = (tunnel_drv_priv_t *)bdmf_obj_data(mo);
    rdpa_tunnel_cfg_t tunnel_cfg = {};

    if ((unsigned)tunnel->index >= RDPA_MAX_TUNNELS || tunnel_objects[tunnel->index] != mo)
    {
        bdmf_trace("Failed to remove tunnel, index %d\n", (int)tunnel->index);
        return;
    }

    if (tunnel->ref_cnt)
    {
        bdmf_trace("Failed to remove tunnel, index %d, reference counter %d\n", (int)tunnel->index, tunnel->ref_cnt);
        return;
    }

    tunnel_rdd_update(mo, &tunnel_cfg);

    tunnel_objects[tunnel->index] = NULL;
}

/* "cfg" attribute "write" callback */
static int tunnel_attr_tunnel_cfg_write(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index,
    const void *val, uint32_t size)
{
    tunnel_drv_priv_t *tunnel = (tunnel_drv_priv_t *)bdmf_obj_data(mo);
    rdpa_tunnel_cfg_t *cfg = (rdpa_tunnel_cfg_t *)val;
    int rc;

    if (mo->state == bdmf_state_active)
    {
        rc = tunnel_rdd_update(mo, cfg);
        if (rc)
            return rc;
    }
    memcpy(&tunnel->cfg, cfg, sizeof(*cfg));
    return 0;
}

/* tunnel attribute access */
static int tunnel_attr_cfg_read(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index, void *val,
    uint32_t size)
{
    tunnel_drv_priv_t *tunnel = (tunnel_drv_priv_t *)bdmf_obj_data(mo);
    rdpa_tunnel_cfg_t *tunnel_cfg = (rdpa_tunnel_cfg_t *)val;

    *tunnel_cfg = tunnel->cfg;
    return 0;
}


/*  tunnel_cfg aggregate type */
static struct bdmf_aggr_type tunnel_cfg_type =
{
    .name = "tunnel_cfg", .struct_name = "rdpa_tunnel_cfg_t",
    .help = "Tunnel Config",
    .fields = (struct bdmf_attr[])
    {
        { .name = "type", .help = "0 - L2_GRE / 1 - L3_GRE / 2 - DS_LITE", .type = bdmf_attr_enum,
            .ts.enum_table = &rdpa_tunnel_type_enum_table,
            .size = sizeof(rdpa_tunnel_type), .offset = offsetof(rdpa_tunnel_cfg_t, tunnel_type)
        },
        { .name = "header_size", .help = "Size of tunnel header in bytes", .size = sizeof(uint8_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_tunnel_cfg_t, tunnel_header_length),
            .flags = BDMF_ATTR_UNSIGNED
        },
        { .name = "local_ip", .help = "Ip source address", .size = sizeof(bdmf_ip_t),
            .type = bdmf_attr_ip_addr, .offset = offsetof(rdpa_tunnel_cfg_t, local_ip),
        },
        { .name = "l3_offset", .help = "l3 offset in tunnel header", .size = sizeof(uint8_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_tunnel_cfg_t, layer3_offset),
            .flags = BDMF_ATTR_UNSIGNED
        },
        { .name = "gre_proto_offset", .help = "GRE protocol offset in tunnel header", .size = sizeof(uint8_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_tunnel_cfg_t, gre_proto_offset),
            .flags = BDMF_ATTR_UNSIGNED
        },
        { .name = "header", .help = "Header in egress", .size = RDPA_MAX_TUNNEL_HEADER_LEN,
            .type = bdmf_attr_buffer, .offset = offsetof(rdpa_tunnel_cfg_t, tunnel_header)
        },
        BDMF_ATTR_LAST
    },
};
DECLARE_BDMF_AGGREGATE_TYPE(tunnel_cfg_type);


/* Object attribute descriptors */
static struct bdmf_attr tunnel_attrs[] = {
    { .name = "index", .help = "Tunnel Index", .type = bdmf_attr_number,
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE_INIT | BDMF_ATTR_CONFIG | BDMF_ATTR_KEY,
        .size = sizeof(bdmf_index), .offset = offsetof(tunnel_drv_priv_t, index)
    },
    { .name = "cfg", .help = "Tunnel Configuration",
        .type = bdmf_attr_aggregate, .ts.aggr_type_name = "tunnel_cfg",
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE | BDMF_ATTR_CONFIG,
        .offset = offsetof(tunnel_drv_priv_t, cfg),
        .read = tunnel_attr_cfg_read, .write = tunnel_attr_tunnel_cfg_write,
    },
    { .name = "ref_cnt", .help = "Reference count", .type = bdmf_attr_number,
      .flags = BDMF_ATTR_READ,
      .size = sizeof(uint32_t), .offset = offsetof(tunnel_drv_priv_t, ref_cnt)
    },
    BDMF_ATTR_LAST
};


static int tunnel_drv_init(struct bdmf_type *drv);
static void tunnel_drv_exit(struct bdmf_type *drv);

static struct bdmf_type tunnel_drv = {
    .name = "tunnel",
    .parent = "system",
    .description = "Tunnel",
    .drv_init = tunnel_drv_init,
    .drv_exit = tunnel_drv_exit,
    .pre_init = tunnel_pre_init,
    .post_init = tunnel_post_init,
    .destroy = tunnel_destroy,
    .extra_size = sizeof(tunnel_drv_priv_t),
    .aattr = tunnel_attrs,
    .max_objs = RDPA_MAX_TUNNELS,
};
DECLARE_BDMF_TYPE(rdpa_tunnel, tunnel_drv);

/* Init/exit module. Cater for GPL layer */
static int tunnel_drv_init(struct bdmf_type *drv)
{
#ifdef BDMF_DRIVER_GPL_LAYER
    f_rdpa_tunnel_drv = rdpa_tunnel_drv;
    f_rdpa_tunnel_get = rdpa_tunnel_get;
#endif
    return 0;
}

static void tunnel_drv_exit(struct bdmf_type *drv)
{
#ifdef BDMF_DRIVER_GPL_LAYER
    f_rdpa_tunnel_drv = NULL;
    f_rdpa_tunnel_get = NULL;
#endif
}

/***************************************************************************
 * Functions declared in auto-generated header
 **************************************************************************/

/** Get tunnel object by key
 * \return  Object handle or NULL if not found
 */
int rdpa_tunnel_get(bdmf_number index, bdmf_object_handle *_obj_)
{
    return rdpa_obj_get(tunnel_objects, RDPA_MAX_TUNNELS, index, _obj_);
}
