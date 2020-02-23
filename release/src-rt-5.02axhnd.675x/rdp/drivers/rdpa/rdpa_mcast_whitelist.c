/*
* <:copyright-BRCM:2013-2018:proprietary:standard
* 
*    Copyright (c) 2013-2018 Broadcom 
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
 * rdpa_mcast_whitlist.c
 *
 *  Created on: September 28, 2018
 *      Author: wen.hsu
 */

#include <bdmf_dev.h>
#include <rdd.h>
#include <rdpa_api.h>
#include "rdpa_int.h"
#include "rdpa_mcast_whitelist.h"
#include "rdpa_mcast_whitelist_ex.h"

/***************************************************************************
 * mcast_whitelist object type
 **************************************************************************/

static struct bdmf_object *mcast_whitelist_object;

/*
 * mcast_whitelist object callback funtions
 */

static int mcast_whitelist_pre_init(struct bdmf_object *mo)
{
    return rdpa_mcast_whitelist_pre_init_ex();
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
static int mcast_whitelist_post_init(struct bdmf_object *mo)
{
    /* save pointer to the mcast_whitelist object */
    mcast_whitelist_object = mo;

    /* set object name */
    snprintf(mo->name, sizeof(mo->name), "mcast_whitelist");

    return rdpa_mcast_whitelist_post_init_ex(mo);
}

static void mcast_whitelist_destroy(struct bdmf_object *mo)
{
    rdpa_mcast_whitelist_destroy_ex();

    mcast_whitelist_object = NULL;
}

/** find mcast object */
static int mcast_whitelist_get(struct bdmf_type *drv, struct bdmf_object *owner,
                               const char *discr, struct bdmf_object **pmo)
{
    if (mcast_whitelist_object == NULL)
    {
        return BDMF_ERR_NOENT;
    }

    *pmo = mcast_whitelist_object;

    return 0;
}

/*  mcast_whitelist_info aggregate type */
struct bdmf_aggr_type mcast_whitelist_type = {
    .name = "mcast_whitelist", .struct_name = "rdpa_mcast_whitelist_t",
    .help = "Multicast Whitelist",
    .size = sizeof(rdpa_mcast_whitelist_t),
    .fields = (struct bdmf_attr[]) {
        { .name = "src_ip", .help = "Source IPv4/IPv6 address", .size = sizeof(bdmf_ip_t),
            .type = bdmf_attr_ip_addr, .offset = offsetof(rdpa_mcast_whitelist_t, src_ip)
        },
        { .name = "dst_ip", .help = "Destination IPv4/IPv6 address", .size = sizeof(bdmf_ip_t),
            .type = bdmf_attr_ip_addr, .offset = offsetof(rdpa_mcast_whitelist_t, dst_ip)
        },
        { .name = "num_vlan_tags", .help = "Number of VLAN Tags", .size = sizeof(uint8_t),
            .type = bdmf_attr_number, .min_val = 0, .max_val = 2,
            .offset = offsetof(rdpa_mcast_whitelist_t, num_vlan_tags),
            .flags = BDMF_ATTR_UNSIGNED
        },
        { .name = "outer_vlan_id", .help = "Outer VLAN VID (0xFFF == *)", .size = sizeof(uint16_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_mcast_whitelist_t, outer_vlan_id),
            .flags = BDMF_ATTR_UNSIGNED
        },
        { .name = "inner_vlan_id", .help = "Inner VLAN VID (0xFFF == *)", .size = sizeof(uint16_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_mcast_whitelist_t, inner_vlan_id),
            .flags = BDMF_ATTR_UNSIGNED
        },
        BDMF_ATTR_LAST
    }
};
DECLARE_BDMF_AGGREGATE_TYPE(mcast_whitelist_type);

/*  mcast_whitelist_stat aggregate type */
struct bdmf_aggr_type mcast_whitelist_stat_type =
{
    .name = "mcast_whitelist_stat", .struct_name = "rdpa_mcast_whitelist_stat_t",
    .help = "Mulicast Whitelist Overall Statistics",
    .fields = (struct bdmf_attr[])
    {
        { .name = "received_pkt", .help = "Received packets", .size = sizeof(uint32_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_mcast_whitelist_stat_t, rx_pkt)
        },
        { .name = "received_byte", .help = "Received bytes", .size = sizeof(uint32_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_mcast_whitelist_stat_t, rx_byte)
        },
        { .name = "dropped_pkt", .help = "dropped packets", .size = sizeof(uint32_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_mcast_whitelist_stat_t, dropped_pkt)
        },
        BDMF_ATTR_LAST
    }
};
DECLARE_BDMF_AGGREGATE_TYPE(mcast_whitelist_stat_type);

/* Object attribute descriptors */
static struct bdmf_attr mcast_whitelist_attrs[] = {
    { .name = "entry", .help = "Multicast whitelist entry",
      .type = bdmf_attr_aggregate, .ts.aggr_type_name = "mcast_whitelist", .array_size = RDPA_MCAST_MAX_WHITELIST,
      .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE | BDMF_ATTR_CONFIG,
      .read = mcast_attr_whitelist_read_ex, .find = mcast_attr_whitelist_find_ex,
      .add = mcast_attr_whitelist_add_ex, .del = mcast_attr_whitelist_delete_ex
    },
    {.name = "port_enable", .help = "Multicast whitelist port enable",
        .type = bdmf_attr_boolean, .size = sizeof(bdmf_boolean),
        .index_ts.enum_table = &rdpa_if_enum_table, .array_size = rdpa_if__number_of,
        .index_type = bdmf_attr_enum,
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE | BDMF_ATTR_CONFIG | BDMF_ATTR_NO_RANGE_CHECK,
        .read = mcast_attr_whitelist_enable_port_read_ex,
        .write = mcast_attr_whitelist_enable_port_write_ex,
    },
    { .name = "stat", .help = "Multicast whitelist statistics",
      .type = bdmf_attr_aggregate, .ts.aggr_type_name = "rdpa_stat", .array_size = RDPA_MCAST_MAX_WHITELIST,
      .flags = BDMF_ATTR_READ | BDMF_ATTR_STAT,
      .read = mcast_attr_whitelist_stats_read_ex
    },
    { .name = "global_stat", .help = "Multicast whitelist global statistics",
      .type = bdmf_attr_aggregate, .ts.aggr_type_name = "mcast_whitelist_stat",
      .flags = BDMF_ATTR_READ | BDMF_ATTR_STAT,
      .read = mcast_attr_whitelist_global_stat_read_ex
    },
    BDMF_ATTR_LAST
};


static int mcast_whitelist_drv_init(struct bdmf_type *drv);
static void mcast_whitelist_drv_exit(struct bdmf_type *drv);

struct bdmf_type mcast_whitelist_drv = {
    .name = "mcast_whitelist",
    .parent = "system",
    .description = "Multicast Whitelist Manager",
    .drv_init = mcast_whitelist_drv_init,
    .drv_exit = mcast_whitelist_drv_exit,
    .pre_init = mcast_whitelist_pre_init,
    .post_init = mcast_whitelist_post_init,
    .destroy = mcast_whitelist_destroy,
    .get = mcast_whitelist_get,
    .extra_size = sizeof(mcast_whitelist_drv_priv_t),
    .aattr = mcast_whitelist_attrs,
    .max_objs = 1,
};
DECLARE_BDMF_TYPE(rdpa_mcast_whitelist, mcast_whitelist_drv);

/* Init module. Cater for GPL layer */
static int mcast_whitelist_drv_init(struct bdmf_type *drv)
{
#ifdef BDMF_DRIVER_GPL_LAYER
    f_rdpa_mcast_whitelist_drv = rdpa_mcast_whitelist_drv;
    f_rdpa_mcast_whitelist_get = rdpa_mcast_whitelist_get;
#endif

    return 0;
}

/* Exit module. Cater for GPL layer */
static void mcast_whitelist_drv_exit(struct bdmf_type *drv)
{
#ifdef BDMF_DRIVER_GPL_LAYER
    f_rdpa_mcast_whitelist_drv = NULL;
    f_rdpa_mcast_whitelist_get = NULL;
#endif
}

/***************************************************************************
 * Functions declared in auto-generated header
 **************************************************************************/

/** Get mcast whitelist object by key
 * \return  Object handle or NULL if not found
 */
int rdpa_mcast_whitelist_get(bdmf_object_handle *_obj_)
{
    if (!mcast_whitelist_object || mcast_whitelist_object->state == bdmf_state_deleted)
        return BDMF_ERR_NOENT;

    bdmf_get(mcast_whitelist_object);

    *_obj_ = mcast_whitelist_object;

    return 0;
}

