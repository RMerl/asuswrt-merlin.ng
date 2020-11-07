/*
 * <:copyright-BRCM:2019:DUAL/GPL:standard
 * 
 *    Copyright (c) 2019 Broadcom 
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


#include <bdmf_dev.h>


/*
 * Structures underlying aggregate attributes
 */
typedef struct
{
    uint32_t min_bw;
    uint32_t max_bw;
    uint32_t assured;
    uint32_t guaranteed;
} allocid_sla_t;

typedef struct
{
    uint16_t onu_id;
    int status;
    allocid_sla_t sla;
} allocid_t;


#define MAX_ALLOCIDS 256
static allocid_t alloc_id_table[MAX_ALLOCIDS];


/* Private structure for driver use - allocated automatically for
 * each managed object
 */
struct allocid_drv_priv
{
    uint16_t alloc_id;
};

/** This optional callback is called at object init time
 *  before initial attributes are set.
 *  Its work is:
 *  - make sure that it is ok to create the object at this time
 *  - assign default values to internal structure(s)
 *  - allocate dynamic resources if any
 *  - set base addresses of memory areas if any
 * If function returns error code !=0, object creation is aborted
 */
static int allocid_pre_init(bdmf_session_handle session, struct bdmf_object *mo)
{
    struct allocid_drv_priv *priv = (struct allocid_drv_priv *)bdmf_obj_data(mo);
    /* Automatic attribute access uses offset from the specified
     * memory segment base. Set it here.
     */
    mo->mem_seg_base[0] = priv;
    return 0;
}

/** This optional callback is called at object init time
 * after initial attributes are set.
 *
 * Its work is:
 * - make sure that all necessary attributes are set and make sense
 * - allocate dynamic resources if any
 * - assign object name if not done in pre_init
 * - finalise object creation
 * If function returns error code !=0, object creation is aborted
 */
static int allocid_post_init(bdmf_session_handle session, struct bdmf_object *mo)
{
    struct allocid_drv_priv *priv = (struct allocid_drv_priv *)bdmf_obj_data(mo);
    snprintf(mo->name, sizeof(mo->name), "aim/alloc_id=%d", priv->alloc_id);
    alloc_id_table[priv->alloc_id].status = 1;
    return 0;
}


/* sla aggregate type */
static struct bdmf_aggr_type sla_type = {
    .name = "sla",
    .help = "Alloc Id SLA",
    .fields = (struct bdmf_attr[]) {
        { .name="min", .help="Min bw value", .size=sizeof(uint32_t),
        .type=bdmf_attr_number, .offset=offsetof(allocid_sla_t, min_bw)
        },
        { .name="max", .help="Min bw value", .size=sizeof(uint32_t),
        .type=bdmf_attr_number, .offset=offsetof(allocid_sla_t, max_bw)
        },
        { .name="assured", .help="Min bw value", .size=sizeof(uint32_t),
        .type=bdmf_attr_number, .offset=offsetof(allocid_sla_t, assured)
        },
        { .name="guaranteed", .help="Min bw value", .size=sizeof(uint32_t),
        .type=bdmf_attr_number, .offset=offsetof(allocid_sla_t, guaranteed),
        .flags=BDMF_ATTR_READ
        },
        BDMF_ATTR_LAST
    }
};
DECLARE_BDMF_AGGREGATE_TYPE(sla_type);


/** Optional "write" callback of "cfg1" attribute: value is in native format */
static int allocid_attr_write(bdmf_session_handle session, struct bdmf_object *mo,
                                 struct bdmf_attr *ad, uint32_t index, const void *val,
                                 uint32_t size)
{
    struct allocid_drv_priv *priv = (struct allocid_drv_priv *)bdmf_obj_data(mo);
    uint16_t id;
    allocid_t *entry;

    id = *(uint16_t *)val;
    if (id >= MAX_ALLOCIDS)
        return BDMF_ERR_RANGE;
    entry = &alloc_id_table[id];
    if (entry->status)
        return BDMF_ERR_ALREADY;
    /* Automatic attribute access uses offset from the specified
     * memory segment base. Set it here.
     */
    priv->alloc_id = id;
    mo->mem_seg_base[1] = entry;
    return sizeof(uint16_t);
}


/* Object attribute descriptors */
static struct bdmf_attr allocid_attrs[] = {
    { .name="alloc_id", .help="",.type=bdmf_attr_number,
      .flags=BDMF_ATTR_READ | BDMF_ATTR_KEY | BDMF_ATTR_MANDATORY |
           BDMF_ATTR_CONFIG | BDMF_ATTR_WRITE_INIT,
      .size=sizeof(uint16_t), .offset=offsetof(struct allocid_drv_priv, alloc_id),
      .write=allocid_attr_write
    },
    { .name="onu", .help="",.type=bdmf_attr_number,
      .flags=BDMF_ATTR_READ | BDMF_ATTR_MANDATORY |
           BDMF_ATTR_CONFIG | BDMF_ATTR_WRITE_INIT, .mem_seg=1,
      .size=sizeof(uint16_t), .offset=offsetof(allocid_t, onu_id)
    },
    { .name="sla", .help="SLA",
      .type=bdmf_attr_aggregate, .ts.aggr_type_name="sla",
      .flags=BDMF_ATTR_READ | BDMF_ATTR_WRITE | BDMF_ATTR_CONFIG | BDMF_ATTR_KEEP_FIELDS,
      .offset=offsetof(allocid_t, sla), .mem_seg=1,
    },
    { .name="status", .help="State",
      .flags=BDMF_ATTR_READ | BDMF_ATTR_CONFIG,
      .offset=offsetof(allocid_t, status), .mem_seg=1,
    },
    BDMF_ATTR_LAST
};

static struct bdmf_type allocid_drv = {
    .name = "aim",
    .description = "Alloc id manager",
    .pre_init = allocid_pre_init,
    .post_init = allocid_post_init,
    .extra_size = sizeof(struct allocid_drv_priv),
    .seg_type[0] = BDMF_MEM_CACHE,
    .seg_size[0] = sizeof(struct allocid_drv_priv),
    .seg_type[1] = BDMF_MEM_CACHE,
    .seg_size[1] = sizeof(allocid_t),
    .aattr = allocid_attrs,
};

DECLARE_BDMF_TYPE(allocid_drv);
