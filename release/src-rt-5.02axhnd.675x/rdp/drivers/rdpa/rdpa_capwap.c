/*
* <:copyright-BRCM:2012-2015:proprietary:standard
* 
*    Copyright (c) 2012-2015 Broadcom 
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
 * rdpa_capwap.c
 */

#include <bdmf_dev.h>
#include "rdpa_api.h"
#include "rdpa_common.h"
#include "rdd.h"
#include "rdpa_capwap.h"
#include "rdd_capwap.h"


/***************************************************************************
 * capwap_object type
 **************************************************************************/

/* capwap object private data */
typedef struct {
    bdmf_boolean clear;
    rdpa_capwap_cfg_t cfg;
} capwap_drv_priv_t;

static struct bdmf_object *capwap_object;

static int capwap_post_init(struct bdmf_object *mo)
{
    capwap_object = mo;

    snprintf(mo->name, sizeof(mo->name), "capwap");

    return 0;
}

static void capwap_destroy(struct bdmf_object *mo)
{
    if (capwap_object != mo)
        return;

    /* ToDo: do cleanups here */
    capwap_object = NULL;
}

static int capwap_get(struct bdmf_type *drv,
           struct bdmf_object *owner, const char *discr,
           struct bdmf_object **pmo)
{
    if (!capwap_object)
        return BDMF_ERR_NOENT;
    *pmo = capwap_object;
    return 0;
}

/* "cfg" attribute "write" callback */
static int capwap_attr_cfg_write(struct bdmf_object *mo,
    struct bdmf_attr *ad, bdmf_index index, const void *val, uint32_t size)
{
    int rc;
    capwap_drv_priv_t *capwap = (capwap_drv_priv_t *)bdmf_obj_data(mo);
    rdpa_capwap_cfg_t *cfg = (rdpa_capwap_cfg_t *)val;

    capwap->cfg = *cfg;
    rc = rdd_capwap_cfg_set(cfg);

    return rc;
}

/* "cfg" attribute "read" callback */
static int capwap_attr_cfg_read(struct bdmf_object *mo,
    struct bdmf_attr *ad, bdmf_index index, void *val, uint32_t size)
{
    capwap_drv_priv_t *capwap = (capwap_drv_priv_t *)bdmf_obj_data(mo);
    rdpa_capwap_cfg_t *cfg = (rdpa_capwap_cfg_t *)val;

    *cfg = capwap->cfg;

    return 0;
}

/* "clear_stats" attribute "write" callback */
static int capwap_attr_clear_stats_write(struct bdmf_object *mo,
    struct bdmf_attr *ad, bdmf_index index, const void *val, uint32_t size)
{
    int rc;

    rc = rdd_capwap_stats_clear();

    return rc;
}

struct bdmf_aggr_type capwap_cfg_type =
{
    .name = "capwap_configuration",
    .struct_name = "rdpa_capwap_cfg_t",
    .help = "CAPWAP configuration",
    .fields = (struct bdmf_attr[])
    {
        { .name = "ac_port", .help = "CAPWAP AC UDP port",
          .size = sizeof(uint16_t), .type = bdmf_attr_number,
          .offset = offsetof(rdpa_capwap_cfg_t, ac_port),
          .flags = BDMF_ATTR_UNSIGNED 
        },
        { .name = "ac_ip", .help = "CAPWAP AC IP address", .size = sizeof(bdmf_ip_t),
          .type = bdmf_attr_ip_addr,
          .offset = offsetof(rdpa_capwap_cfg_t, ac_ip)
        },
        { .name = "ap_ip", .help = "CAPWAP AP IP address", .size = sizeof(bdmf_ip_t),
          .type = bdmf_attr_ip_addr,
          .offset = offsetof(rdpa_capwap_cfg_t, ap_ip)
        },
        { .name = "ap_mac_address", .help = "CAPWAP AP MAC address", .size = sizeof(bdmf_mac_t),
          .type = bdmf_attr_ether_addr,
          .offset = offsetof(rdpa_capwap_cfg_t, ap_mac_address)
        },
        BDMF_ATTR_LAST
    }
};
DECLARE_BDMF_AGGREGATE_TYPE(capwap_cfg_type);

/* Object attribute descriptors */
static struct bdmf_attr capwap_attrs[] =
{
    { .name = "clear_stats", .help = "CAPWAP clear statistics",
      .type = bdmf_attr_boolean, .flags = BDMF_ATTR_WRITE,
      .size = sizeof(bdmf_boolean),
      .offset = offsetof(capwap_drv_priv_t, clear),
      .write = capwap_attr_clear_stats_write
    },
    { .name = "cfg", .help = "CAPWAP configuration",
      .type = bdmf_attr_aggregate, .ts.aggr_type_name = "capwap_configuration",
      .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE | BDMF_ATTR_CONFIG,
      .size = sizeof(rdpa_capwap_cfg_t),
      .offset = offsetof(capwap_drv_priv_t, cfg),
      .read = capwap_attr_cfg_read,
      .write = capwap_attr_cfg_write
    },
    BDMF_ATTR_LAST
};

static int capwap_drv_init(struct bdmf_type *drv);
static void capwap_drv_exit(struct bdmf_type *drv);

struct bdmf_type capwap_drv = {
    .name = "capwap",
    .parent = "system",
    .description = "CAPWAP",
    .drv_init = capwap_drv_init,
    .drv_exit = capwap_drv_exit,
    .post_init = capwap_post_init,
    .destroy = capwap_destroy,
    .get = capwap_get,
    .extra_size = sizeof(capwap_drv_priv_t),
    .aattr = capwap_attrs,
    .max_objs = 1,
};
DECLARE_BDMF_TYPE(rdpa_capwap, capwap_drv);

/***************************************************************************
 * capwap_reassembly_object type
 **************************************************************************/

/* capwap reassembly object private data */
typedef struct {
    bdmf_boolean enable;
    rdpa_capwap_reassembly_cfg_t cfg;
    rdpa_capwap_reassembly_stats_t stats;
    rdpa_capwap_reassembly_contexts_t active_contexts;
} capwap_reassembly_drv_priv_t;

static struct bdmf_object *capwap_reassembly_object;

static int capwap_reassembly_post_init(struct bdmf_object *mo)
{
    capwap_reassembly_object = mo;

    snprintf(mo->name, sizeof(mo->name), "capwap_reassembly");

    return 0;
}

static void capwap_reassembly_destroy(struct bdmf_object *mo)
{
    if (capwap_reassembly_object != mo)
        return;

    /* ToDo: do cleanups here */
    capwap_reassembly_object = NULL;
}

/* "enable" attribute "write" callback */
static int capwap_reassembly_attr_enable_write(struct bdmf_object *mo,
    struct bdmf_attr *ad, bdmf_index index, const void *val, uint32_t size)
{
    int rc;

    capwap_reassembly_drv_priv_t *capwap = (capwap_reassembly_drv_priv_t *)bdmf_obj_data(mo);
    bdmf_boolean enable = *(bdmf_boolean *)val;

    capwap->enable = enable;
    rc = rdd_capwap_reassembly_enable(enable);

    return rc;
}

/* "enable" attribute "read" callback */
static int capwap_reassembly_attr_enable_read(struct bdmf_object *mo,
    struct bdmf_attr *ad, bdmf_index index, void *val, uint32_t size)
{
    capwap_reassembly_drv_priv_t *capwap = (capwap_reassembly_drv_priv_t *)bdmf_obj_data(mo);
    bdmf_boolean *enablep = (bdmf_boolean *)val;

    *enablep = capwap->enable;

    return 0;
}

/* "cfg" attribute "write" callback */
static int capwap_reassembly_attr_cfg_write(struct bdmf_object *mo,
    struct bdmf_attr *ad, bdmf_index index, const void *val, uint32_t size)
{
    int rc;
    capwap_reassembly_drv_priv_t *capwap_reassembly = (capwap_reassembly_drv_priv_t *)bdmf_obj_data(mo);
    rdpa_capwap_reassembly_cfg_t *cfg = (rdpa_capwap_reassembly_cfg_t *)val;

    capwap_reassembly->cfg = *cfg;
    rc = rdd_capwap_reassembly_cfg_set(cfg);

    return rc;
}

/* "cfg" attribute "read" callback */
static int capwap_reassembly_attr_cfg_read(struct bdmf_object *mo,
    struct bdmf_attr *ad, bdmf_index index, void *val, uint32_t size)
{
    capwap_reassembly_drv_priv_t *capwap_reassembly = (capwap_reassembly_drv_priv_t *)bdmf_obj_data(mo);
    rdpa_capwap_reassembly_cfg_t *cfg = (rdpa_capwap_reassembly_cfg_t *)val;

    *cfg = capwap_reassembly->cfg;

    return 0;
}

/* "stats" attribute "read" callback */
static int capwap_reassembly_attr_stats_read(struct bdmf_object *mo,
    struct bdmf_attr *ad, bdmf_index index, void *val, uint32_t size)
{
    int rc;
    rdpa_capwap_reassembly_stats_t *stats = (rdpa_capwap_reassembly_stats_t *)val;

    rc = rdd_capwap_reassembly_stats_get(stats);

    return rc;
}

/* "active_contexts" attribute "read" callback */
static int capwap_reassembly_attr_active_contexts_read(struct bdmf_object *mo,
    struct bdmf_attr *ad, bdmf_index index, void *val, uint32_t size)
{
    int rc;
    rdpa_capwap_reassembly_contexts_t *active_contexts = (rdpa_capwap_reassembly_contexts_t *)val;

    rc = rdd_capwap_reassembly_active_contexts_get(active_contexts);

    return rc;
}

struct bdmf_aggr_type capwap_reassembly_cfg_type =
{
    .name = "capwap_reassembly_configuration",
    .struct_name = "rdpa_capwap_reassembly_cfg_t",
    .help = "CAPWAP reassembly configuration",
    .fields = (struct bdmf_attr[])
    {
        { .name = "ip_v4_window_check", .help = "verify IPv4 packet id",
          .type = bdmf_attr_boolean, .size = sizeof(bdmf_boolean),
          .offset = offsetof(rdpa_capwap_reassembly_cfg_t, ip_v4_window_check)
        },
        { .name = "receive_frame_buffer_size", .help = "size to accomodate largest packet",
          .size = sizeof(uint16_t), .type = bdmf_attr_number,
          .offset = offsetof(rdpa_capwap_reassembly_cfg_t, receive_frame_buffer_size),
          .flags = BDMF_ATTR_UNSIGNED 
        },
        BDMF_ATTR_LAST
    }
};
DECLARE_BDMF_AGGREGATE_TYPE(capwap_reassembly_cfg_type);

struct bdmf_aggr_type capwap_reassembly_stats_type =
{
    .name = "capwap_reassembly_statistics", .struct_name = "rdpa_capwap_reassembly_stats_t",
    .help = "CAPWAP statistics",
    .fields = (struct bdmf_attr[])
    {
        { .name = "invalid_headers", .help = "invalid headers",
          .size = sizeof(uint32_t),
          .type = bdmf_attr_number,
          .offset = offsetof(rdpa_capwap_reassembly_stats_t, invalid_headers),
          .flags = BDMF_ATTR_UNSIGNED 
        },
        { .name = "aborts", .help = "aborts",
          .size = sizeof(uint32_t),
          .type = bdmf_attr_number,
          .offset = offsetof(rdpa_capwap_reassembly_stats_t, aborts),
          .flags = BDMF_ATTR_UNSIGNED 
        },
        { .name = "fragments_received", .help = "fragments received",
          .size = sizeof(uint32_t),
          .type = bdmf_attr_number,
          .offset = offsetof(rdpa_capwap_reassembly_stats_t, fragments_received),
          .flags = BDMF_ATTR_UNSIGNED 
        },
        { .name = "fragments_evicted", .help = "fragments evicted",
          .size = sizeof(uint32_t),
          .type = bdmf_attr_number,
          .offset = offsetof(rdpa_capwap_reassembly_stats_t, fragments_evicted),
          .flags = BDMF_ATTR_UNSIGNED 
        },
        { .name = "unfragmented_packets", .help = "unfragmented packets",
          .size = sizeof(uint32_t),
          .type = bdmf_attr_number,
          .offset = offsetof(rdpa_capwap_reassembly_stats_t, unfragmented_packets),
          .flags = BDMF_ATTR_UNSIGNED 
        },
        { .name = "middle_fragments", .help = "middle fragments",
          .size = sizeof(uint32_t),
          .type = bdmf_attr_number,
          .offset = offsetof(rdpa_capwap_reassembly_stats_t, middle_fragments),
          .flags = BDMF_ATTR_UNSIGNED 
        },
        { .name = "first_fragments", .help = "first fragments",
          .size = sizeof(uint32_t),
          .type = bdmf_attr_number,
          .offset = offsetof(rdpa_capwap_reassembly_stats_t, first_fragments),
          .flags = BDMF_ATTR_UNSIGNED 
        },
        { .name = "last_fragments", .help = "last fragments",
          .size = sizeof(uint32_t),
          .type = bdmf_attr_number,
          .offset = offsetof(rdpa_capwap_reassembly_stats_t, last_fragments),
          .flags = BDMF_ATTR_UNSIGNED 
        },
        { .name = "packets_not_in_window", .help = "packets not in window",
          .size = sizeof(uint32_t),
          .type = bdmf_attr_number,
          .offset = offsetof(rdpa_capwap_reassembly_stats_t, packets_not_in_window),
          .flags = BDMF_ATTR_UNSIGNED 
        },
        { .name = "packets_reassembled", .help = "packets reassembled",
          .size = sizeof(uint32_t),
          .type = bdmf_attr_number,
          .offset = offsetof(rdpa_capwap_reassembly_stats_t, packets_reassembled),
          .flags = BDMF_ATTR_UNSIGNED 
        },
        { .name = "invalid_fragment", .help = "invalid fragments",
          .size = sizeof(uint32_t),
          .type = bdmf_attr_number,
          .offset = offsetof(rdpa_capwap_reassembly_stats_t, invalid_fragment),
          .flags = BDMF_ATTR_UNSIGNED 
        },
        { .name = "reassembled_packet_too_big", .help = "reassembled packet too big",
          .size = sizeof(uint32_t),
          .type = bdmf_attr_number,
          .offset = offsetof(rdpa_capwap_reassembly_stats_t, reassembled_packet_too_big),
          .flags = BDMF_ATTR_UNSIGNED 
        },
        BDMF_ATTR_LAST
    }
};
DECLARE_BDMF_AGGREGATE_TYPE(capwap_reassembly_stats_type);

struct bdmf_aggr_type capwap_reassembly_active_contexts_entries_type =
{
    .name = "capwap_reassembly_active_contexts",
    .struct_name = "rdpa_capwap_reassembly_contexts_t",
    .help = "CAPWAP reassembly active context entries",
    .fields = (struct bdmf_attr[])
    {
        { .name = "entry0", .help = "entry 0",
          .type = bdmf_attr_number, .size = sizeof(uint16_t),
          .offset = offsetof(rdpa_capwap_reassembly_contexts_t, entry0)
        },
        { .name = "entry1", .help = "entry 1",
          .type = bdmf_attr_number, .size = sizeof(uint16_t),
          .offset = offsetof(rdpa_capwap_reassembly_contexts_t, entry1)
        },
        { .name = "entry2", .help = "entry 2",
          .type = bdmf_attr_number, .size = sizeof(uint16_t),
          .offset = offsetof(rdpa_capwap_reassembly_contexts_t, entry2)
        },
        { .name = "entry3", .help = "entry 3",
          .type = bdmf_attr_number, .size = sizeof(uint16_t),
          .offset = offsetof(rdpa_capwap_reassembly_contexts_t, entry3)
        },
        BDMF_ATTR_LAST
    }
};
DECLARE_BDMF_AGGREGATE_TYPE(capwap_reassembly_active_contexts_entries_type);

/* Object attribute descriptors */
static struct bdmf_attr capwap_reassembly_attrs[] =
{
    { .name = "enable", .help = "CAPWAP enable/disable processing",
      .type = bdmf_attr_boolean, .size = sizeof(bdmf_boolean),
      .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE | BDMF_ATTR_CONFIG,
      .offset = offsetof(capwap_reassembly_drv_priv_t, enable),
      .read = capwap_reassembly_attr_enable_read,
      .write = capwap_reassembly_attr_enable_write
    },
    { .name = "cfg", .help = "CAPWAP reassembly configuration",
      .type = bdmf_attr_aggregate, .ts.aggr_type_name = "capwap_reassembly_configuration",
      .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE | BDMF_ATTR_CONFIG,
      .size = sizeof(rdpa_capwap_reassembly_cfg_t),
      .offset = offsetof(capwap_reassembly_drv_priv_t, cfg),
      .read = capwap_reassembly_attr_cfg_read,
      .write = capwap_reassembly_attr_cfg_write
    },
    { .name = "stats", .help = "CAPWAP reassembly statistics",
      .type = bdmf_attr_aggregate, .ts.aggr_type_name = "capwap_reassembly_statistics",
      .flags = BDMF_ATTR_READ | BDMF_ATTR_STAT | BDMF_ATTR_NOLOCK,
      .size = sizeof(rdpa_capwap_reassembly_stats_t),
      .offset = offsetof(capwap_reassembly_drv_priv_t, stats),
      .read = capwap_reassembly_attr_stats_read
    },
    { .name = "active_contexts", .help = "Active CAPWAP context entries",
      .type = bdmf_attr_aggregate, .ts.aggr_type_name = "capwap_reassembly_active_contexts",
      .size = sizeof(rdpa_capwap_reassembly_contexts_t),
      .flags = BDMF_ATTR_READ | BDMF_ATTR_CONFIG,
      .offset = offsetof(capwap_reassembly_drv_priv_t, active_contexts),
      .read = capwap_reassembly_attr_active_contexts_read
    },
    BDMF_ATTR_LAST
};

static int capwap_drv_init(struct bdmf_type *drv);
static void capwap_drv_exit(struct bdmf_type *drv);

struct bdmf_type capwap_reassembly_drv = {
    .name = "capwap_reassembly",
    .parent = "capwap",
    .description = "CAPWAP reassembly",
    .post_init = capwap_reassembly_post_init,
    .destroy = capwap_reassembly_destroy,
    .extra_size = sizeof(capwap_reassembly_drv_priv_t),
    .aattr = capwap_reassembly_attrs,
    .max_objs = 1,
};
DECLARE_BDMF_TYPE(rdpa_capwap_reassembly, capwap_reassembly_drv);

/***************************************************************************
 * capwap_fragmentation_object type
 **************************************************************************/

/* capwap fragmentation object private data */
typedef struct {
    bdmf_boolean enable;
    rdpa_capwap_fragmentation_cfg_t cfg;
    rdpa_capwap_fragmentation_stats_t stats;
} capwap_fragmentation_drv_priv_t;

static struct bdmf_object *capwap_fragmentation_object;

static int capwap_fragmentation_post_init(struct bdmf_object *mo)
{
    capwap_fragmentation_object = mo;

    snprintf(mo->name, sizeof(mo->name), "capwap_fragmentation");

    return 0;
}

static void capwap_fragmentation_destroy(struct bdmf_object *mo)
{
    if (capwap_fragmentation_object != mo)
        return;

    /* ToDo: do cleanups here */
    capwap_fragmentation_object = NULL;
}

/* "enable" attribute "write" callback */
static int capwap_fragmentation_attr_enable_write(struct bdmf_object *mo,
    struct bdmf_attr *ad, bdmf_index index, const void *val, uint32_t size)
{
    int rc;

    capwap_fragmentation_drv_priv_t *capwap = (capwap_fragmentation_drv_priv_t *)bdmf_obj_data(mo);
    bdmf_boolean enable = *(bdmf_boolean *)val;

    capwap->enable = enable;
    rc = rdd_capwap_fragmentation_enable(enable);

    return rc;
}

/* "enable" attribute "read" callback */
static int capwap_fragmentation_attr_enable_read(struct bdmf_object *mo,
    struct bdmf_attr *ad, bdmf_index index, void *val, uint32_t size)
{
    capwap_fragmentation_drv_priv_t *capwap = (capwap_fragmentation_drv_priv_t *)bdmf_obj_data(mo);
    bdmf_boolean *enablep = (bdmf_boolean *)val;

    *enablep = capwap->enable;

    return 0;
}

/* "cfg" attribute "write" callback */
static int capwap_fragmentation_attr_cfg_write(struct bdmf_object *mo,
    struct bdmf_attr *ad, bdmf_index index, const void *val, uint32_t size)
{
    int rc;
    capwap_fragmentation_drv_priv_t *capwap_fragmentation = (capwap_fragmentation_drv_priv_t *)bdmf_obj_data(mo);
    rdpa_capwap_fragmentation_cfg_t *cfg = (rdpa_capwap_fragmentation_cfg_t *)val;

    capwap_fragmentation->cfg = *cfg;
    rc = rdd_capwap_fragmentation_cfg_set(cfg);

    return rc;
}

/* "cfg" attribute "read" callback */
static int capwap_fragmentation_attr_cfg_read(struct bdmf_object *mo,
    struct bdmf_attr *ad, bdmf_index index, void *val, uint32_t size)
{
    capwap_fragmentation_drv_priv_t *capwap_fragmentation = (capwap_fragmentation_drv_priv_t *)bdmf_obj_data(mo);
    rdpa_capwap_fragmentation_cfg_t *cfg = (rdpa_capwap_fragmentation_cfg_t *)val;

    *cfg = capwap_fragmentation->cfg;

    return 0;
}

/* "stats" attribute "read" callback */
static int capwap_fragmentation_attr_stats_read(struct bdmf_object *mo,
    struct bdmf_attr *ad, bdmf_index index, void *val, uint32_t size)
{
    int rc;
    rdpa_capwap_fragmentation_stats_t *stats = (rdpa_capwap_fragmentation_stats_t *)val;

    rc = rdd_capwap_fragmentation_stats_get(stats);

    return rc;
}

struct bdmf_aggr_type capwap_fragmentation_cfg_type =
{
    .name = "capwap_fragmentation_configuration",
    .struct_name = "rdpa_capwap_fragmentation_cfg_t",
    .help = "CAPWAP fragmentation configuration",
    .fields = (struct bdmf_attr[])
    {
        { .name = "max_frame_size", .help = "max size that can be sent to egress port",
          .size = sizeof(uint16_t), .type = bdmf_attr_number,
          .offset = offsetof(rdpa_capwap_fragmentation_cfg_t, max_frame_size),
          .flags = BDMF_ATTR_UNSIGNED 
        },
        BDMF_ATTR_LAST
    }
};
DECLARE_BDMF_AGGREGATE_TYPE(capwap_fragmentation_cfg_type);

struct bdmf_aggr_type capwap_fragmentation_stats_type =
{
    .name = "capwap_fragmentation_statistics", .struct_name = "rdpa_capwap_fragmentation_stats_t",
    .help = "CAPWAP statistics",
    .fields = (struct bdmf_attr[])
    {
        { .name = "upstream_packets", .help = "upstream packets",
          .size = sizeof(uint32_t),
          .type = bdmf_attr_number,
          .offset = offsetof(rdpa_capwap_fragmentation_stats_t, upstream_packets),
          .flags = BDMF_ATTR_UNSIGNED 
        },
        { .name = "invalid_ethtype_or_ip_header", .help = "invalid Ethernet Type or IP header",
          .size = sizeof(uint32_t),
          .type = bdmf_attr_number,
          .offset = offsetof(rdpa_capwap_fragmentation_stats_t, invalid_ethtype_or_ip_header),
          .flags = BDMF_ATTR_UNSIGNED 
        },
        { .name = "invalid_protocol", .help = "invalid protocol",
          .size = sizeof(uint32_t),
          .type = bdmf_attr_number,
          .offset = offsetof(rdpa_capwap_fragmentation_stats_t, invalid_protocol),
          .flags = BDMF_ATTR_UNSIGNED 
        },
        { .name = "congestion", .help = "congestion packets",
          .size = sizeof(uint32_t),
          .type = bdmf_attr_number,
          .offset = offsetof(rdpa_capwap_fragmentation_stats_t, congestion),
          .flags = BDMF_ATTR_UNSIGNED 
        },
        { .name = "middle_fragments", .help = "middle fragments",
          .size = sizeof(uint32_t),
          .type = bdmf_attr_number,
          .offset = offsetof(rdpa_capwap_fragmentation_stats_t, middle_fragments),
          .flags = BDMF_ATTR_UNSIGNED 
        },
        { .name = "first_fragments", .help = "first fragments",
          .size = sizeof(uint32_t),
          .type = bdmf_attr_number,
          .offset = offsetof(rdpa_capwap_fragmentation_stats_t, first_fragments),
          .flags = BDMF_ATTR_UNSIGNED 
        },
        { .name = "last_fragments", .help = "last fragments",
          .size = sizeof(uint32_t),
          .type = bdmf_attr_number,
          .offset = offsetof(rdpa_capwap_fragmentation_stats_t, last_fragments),
          .flags = BDMF_ATTR_UNSIGNED 
        },
        BDMF_ATTR_LAST
    }
};
DECLARE_BDMF_AGGREGATE_TYPE(capwap_fragmentation_stats_type);

/* Object attribute descriptors */
static struct bdmf_attr capwap_fragmentation_attrs[] =
{
    { .name = "enable", .help = "CAPWAP enable/disable processing",
      .type = bdmf_attr_boolean, .size = sizeof(bdmf_boolean),
      .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE | BDMF_ATTR_CONFIG,
      .offset = offsetof(capwap_fragmentation_drv_priv_t, enable),
      .read = capwap_fragmentation_attr_enable_read,
      .write = capwap_fragmentation_attr_enable_write
    },
    { .name = "cfg", .help = "CAPWAP fragmentation configuration",
      .type = bdmf_attr_aggregate, .ts.aggr_type_name = "capwap_fragmentation_configuration",
      .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE | BDMF_ATTR_CONFIG,
      .size = sizeof(rdpa_capwap_fragmentation_cfg_t),
      .offset = offsetof(capwap_fragmentation_drv_priv_t, cfg),
      .read = capwap_fragmentation_attr_cfg_read,
      .write = capwap_fragmentation_attr_cfg_write
    },
    { .name = "stats", .help = "CAPWAP fragmentation statistics",
      .type = bdmf_attr_aggregate, .ts.aggr_type_name = "capwap_fragmentation_statistics",
      .flags = BDMF_ATTR_READ | BDMF_ATTR_STAT | BDMF_ATTR_NOLOCK,
      .size = sizeof(rdpa_capwap_fragmentation_stats_t),
      .offset = offsetof(capwap_fragmentation_drv_priv_t, stats),
      .read = capwap_fragmentation_attr_stats_read
    },
    BDMF_ATTR_LAST
};

static int capwap_drv_init(struct bdmf_type *drv);
static void capwap_drv_exit(struct bdmf_type *drv);

struct bdmf_type capwap_fragmentation_drv = {
    .name = "capwap_fragmentation",
    .parent = "capwap",
    .description = "CAPWAP fragmentation",
    .post_init = capwap_fragmentation_post_init,
    .destroy = capwap_fragmentation_destroy,
    .extra_size = sizeof(capwap_fragmentation_drv_priv_t),
    .aattr = capwap_fragmentation_attrs,
    .max_objs = 1,
};
DECLARE_BDMF_TYPE(rdpa_capwap_fragmentation, capwap_fragmentation_drv);

/***************************************************************************
 * capwap driver entry / exit functions
 **************************************************************************/

/* Init/exit module. Cater for GPL layer */
static int capwap_drv_init(struct bdmf_type *drv)
{
#ifdef BDMF_DRIVER_GPL_LAYER
    f_rdpa_capwap_drv = rdpa_capwap_drv;
    f_rdpa_capwap_reassembly_drv = rdpa_capwap_reassembly_drv;
    f_rdpa_capwap_fragmentation_drv = rdpa_capwap_fragmentation_drv;
    f_rdpa_capwap_get = rdpa_capwap_get;
    f_rdpa_capwap_reassembly_get = rdpa_capwap_reassembly_get;
    f_rdpa_capwap_fragmentation_get = rdpa_capwap_fragmentation_get;
#endif
    return 0;
}

static void capwap_drv_exit(struct bdmf_type *drv)
{
#ifdef BDMF_DRIVER_GPL_LAYER
    f_rdpa_capwap_drv = NULL;
    f_rdpa_capwap_reassembly_drv = NULL;
    f_rdpa_capwap_fragmentation_drv = NULL;
    f_rdpa_capwap_get = NULL;
    f_rdpa_capwap_reassembly_get = NULL;
    f_rdpa_capwap_fragmentation_get = NULL;
#endif
}

/***************************************************************************
 * Functions declared in auto-generated header
 **************************************************************************/

int rdpa_capwap_get(bdmf_object_handle *capwap_obj)
{
    if (!capwap_object || capwap_object->state == bdmf_state_deleted)
        return BDMF_ERR_NOENT;
    bdmf_get(capwap_object);
    *capwap_obj = capwap_object;
    return 0;
}

int rdpa_capwap_reassembly_get(bdmf_object_handle *capwap_reassembly_obj)
{
    if (!capwap_reassembly_object || capwap_reassembly_object->state == bdmf_state_deleted)
        return BDMF_ERR_NOENT;
    bdmf_get(capwap_reassembly_object);
    *capwap_reassembly_obj = capwap_reassembly_object;
    return 0;
}

int rdpa_capwap_fragmentation_get(bdmf_object_handle *capwap_fragmentation_obj)
{
    if (!capwap_fragmentation_object || capwap_fragmentation_object->state == bdmf_state_deleted)
        return BDMF_ERR_NOENT;
    bdmf_get(capwap_fragmentation_object);
    *capwap_fragmentation_obj = capwap_fragmentation_object;
    return 0;
}

