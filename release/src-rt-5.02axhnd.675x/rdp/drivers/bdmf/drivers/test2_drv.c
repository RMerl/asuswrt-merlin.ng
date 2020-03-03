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


static int test2_id;


/*
 * Structures underlying aggregate attributes
 */
struct test2_aggr1_type
{
    uint32_t f1;
    uint32_t f2;
};

struct test2_aggr2_type
{
    uint32_t f1;
    uint32_t ip1;
    uint8_t  mac1[6];
    struct test2_aggr1_type ag1;
};

/* Private structure for driver use - allocated automatically for
 * each managed object
 */
struct test2_drv_priv
{
    uint32_t id;        /* Object id: uniquely identifies the object */
    uint32_t stat;
    struct test2_aggr1_type ag1; /* Aggregate that itself contains aggregates */
    struct test2_aggr2_type ag2; /* Aggregate that itself contains aggregates */
};

/** This optional callback is called called at object init time
 *  before initial attributes are set.
 *  Its work is:
 *  - make sure that it is ok to create the object at this time
 *  - assign default values to internal structure(s)
 *  - allocate dynamic resources if any
 *  - set base addresses of memory areas if any
 * If function returns error code !=0, object creation is aborted
 */
static int test2_pre_init(struct bdmf_object *mo)
{
    struct test2_drv_priv *priv = (struct test2_drv_priv *)bdmf_obj_data(mo);
    
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
static int test2_post_init(struct bdmf_object *mo)
{
    struct test2_drv_priv *priv = (struct test2_drv_priv *)bdmf_obj_data(mo);
    priv->id = ++test2_id;
    snprintf(mo->name, sizeof(mo->name), "test2/id2=%d", priv->id);
    return 0;
}

/** Optional write callback for ag1 aggregate attribute: value is in native format */
static int test2_ag1_attr_write(struct bdmf_object *mo,
                                struct bdmf_attr *ad, bdmf_index index, const void *val,
                                uint32_t size)
{
    struct test2_drv_priv *priv = (struct test2_drv_priv *)bdmf_obj_data(mo);
    const struct test2_aggr1_type *ag1=(const struct test2_aggr1_type *)val;
    char sbuf[128];
    priv->ag1 = *ag1;
    ad->val_to_s(mo, ad, ag1, sbuf, sizeof(sbuf));
    bdmf_print("Aggregate attribute %s: new value: %s\n", ad->name, sbuf);
    return sizeof(struct test2_aggr1_type);
}

/** Optional write callback for ag2 aggregate attribute: value is in native format */
static int test2_ag2_attr_write(struct bdmf_object *mo,
                                struct bdmf_attr *ad, bdmf_index index, const void *val,
                                uint32_t size)
{
    struct test2_drv_priv *priv = (struct test2_drv_priv *)bdmf_obj_data(mo);
    const struct test2_aggr2_type *ag2=(const struct test2_aggr2_type *)val;
    char sbuf[128];
    priv->ag2 = *ag2;
    ad->val_to_s(mo, ad, ag2, sbuf, sizeof(sbuf));
    bdmf_print("Aggregate attribute %s: new value: %s\n", ad->name, sbuf);
    return sizeof(struct test2_aggr2_type);
}

/* Read stat2 callback */
static int test2_stat2_attr_read(struct bdmf_object *mo,
                                struct bdmf_attr *ad, bdmf_index index, void *val,
                                uint32_t size)
{
    struct test2_drv_priv *priv = (struct test2_drv_priv *)bdmf_obj_data(mo);
    *(uint32_t *)val = priv->stat;
    priv->stat = 1 - priv->stat; /* alternate 0/1 */
    return sizeof(uint32_t);
}

/* test2_aggr1 aggregate type */
static struct bdmf_aggr_type test2_aggr1_type = {
    .name = "test2_aggr1",
    .help = "Aggregate type example 1",
    .fields = (struct bdmf_attr[]) {
        { .name="f1", .help="field1", .size=sizeof(uint32_t),
        .type=bdmf_attr_number, .offset=offsetof(struct test2_aggr1_type, f1)
        },
        { .name="f2", .help="field1", .size=sizeof(uint32_t),
        .type=bdmf_attr_number, .offset=offsetof(struct test2_aggr1_type, f2)
        },
        BDMF_ATTR_LAST
    }
};
DECLARE_BDMF_AGGREGATE_TYPE(test2_aggr1_type);


/* test2 aggregate type */
static struct bdmf_aggr_type test2_aggr2_type = {
    .name = "test2_aggr2",
    .help = "Aggregate type example 2",
    .fields = (struct bdmf_attr[]) {
        { .name="fnum", .help="Mandatory numeric field", .size=sizeof(uint32_t),
        .type=bdmf_attr_number, .offset=offsetof(struct test2_aggr2_type, f1),
        .flags=BDMF_ATTR_MANDATORY
        },
        { .name="fip", .help="IP field",
        .type=bdmf_attr_ipv4_addr, .offset=offsetof(struct test2_aggr2_type, ip1)
        },
        { .name="fmac", .help="MAC field",
        .type=bdmf_attr_ether_addr, .offset=offsetof(struct test2_aggr2_type, mac1)
        },
        { .name="fag", .help="Field that itself is an aggregate",
        .type=bdmf_attr_aggregate, .offset=offsetof(struct test2_aggr2_type, ag1),
        .ts.aggr_type_name="test2_aggr1"
        },
        BDMF_ATTR_LAST
    }
};
DECLARE_BDMF_AGGREGATE_TYPE(test2_aggr2_type);


/* Object attribute descriptors */
static struct bdmf_attr test2_attrs[] = {
    { .name="id2", .help="Automatically assigned unique object id",
      .type=bdmf_attr_number,
      .flags=BDMF_ATTR_READ | BDMF_ATTR_KEY | BDMF_ATTR_CONFIG,
      .size=sizeof(uint32_t), .offset=offsetof(struct test2_drv_priv, id)
    },
    { .name="ag1", .help="Aggregate attribute",
      .type=bdmf_attr_aggregate, .ts.aggr_type_name="test2_aggr1",
      .flags=BDMF_ATTR_READ | BDMF_ATTR_WRITE | BDMF_ATTR_CONFIG | BDMF_ATTR_CLEAR_FIELDS,
      .offset=offsetof(struct test2_drv_priv, ag1),
      .write=test2_ag1_attr_write
    },
    { .name="ag2", .help="Aggregate attribute containing another aggregate",
      .type=bdmf_attr_aggregate, .ts.aggr_type_name="test2_aggr2",
      .flags=BDMF_ATTR_READ | BDMF_ATTR_WRITE | BDMF_ATTR_CONFIG,
      .offset=offsetof(struct test2_drv_priv, ag2),
      .write=test2_ag2_attr_write
    },
    { .name="stat2", .help="Statistic 2 - alternates 0 and 1",
      .flags=BDMF_ATTR_READ | BDMF_ATTR_STAT, .size=sizeof(uint32_t),
      .read=test2_stat2_attr_read
    },
    BDMF_ATTR_LAST
};

static struct bdmf_type test2_drv = {
    .name = "test2",
    .description = "Test2 BDMF plugin",
    .pre_init = test2_pre_init,
    .post_init = test2_post_init,
    .extra_size = sizeof(struct test2_drv_priv),
    .seg_type[0] = BDMF_MEM_CACHE,
    .seg_size[0] = sizeof(struct test2_drv_priv),
    .aattr = test2_attrs,
    .parent = "test1"
};

DECLARE_BDMF_TYPE(bdmf_test2, test2_drv);
