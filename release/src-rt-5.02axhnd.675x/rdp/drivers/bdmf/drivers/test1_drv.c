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


static int test1_id;

/* Private structure for driver use - allocated automatically for
 * each managed object
 */
struct test1_drv_priv
{
    uint32_t id;        /* Object id: uniquely identifies the object */
    uint32_t initcfg1;  /* Init-time configuration parameter */
    uint16_t cfg1;      /* Config parameter 1 */
    uint32_t stat1;     /* Statistic 1 */
#define TEST1_ARR1_SIZE             32
#define TEST1_ARR1_INVALID_VAL      -1
    int arr1[TEST1_ARR1_SIZE];  /* Array 1 */
};

/* A few attributes in a separate memory area
 */
struct test1_area1_priv
{
    uint32_t cfg2;      /* Config parameter 2 */
    uint32_t stat2;     /* Statistic 2 */
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
static int test1_pre_init(struct bdmf_object *mo)
{
    struct test1_drv_priv *priv = (struct test1_drv_priv *)bdmf_obj_data(mo);
    int i;
    
    /* Automatic attribute access uses offset from the specified
     * memory segment base. Set it here.
     */
    mo->mem_seg_base[0] = priv;

    /* Invalidate values in arr1. In this example negative value means invalid */
    for(i=0; i<TEST1_ARR1_SIZE; i++)
        priv->arr1[i] = TEST1_ARR1_INVALID_VAL;

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
static int test1_post_init(struct bdmf_object *mo)
{
    struct test1_drv_priv *priv = (struct test1_drv_priv *)bdmf_obj_data(mo);
    priv->id = ++test1_id;
    snprintf(mo->name, sizeof(mo->name), "test1/id1=%d", priv->id);
    return 0;
}

/* Optional "read" callback of "stat1" attribute */
static int test1_stat1_attr_read(struct bdmf_object *mo,
                                struct bdmf_attr *ad, bdmf_index index, void *val,
                                uint32_t size)
{
    struct test1_drv_priv *priv = (struct test1_drv_priv *)bdmf_obj_data(mo);
    *(uint32_t *)val = priv->stat1;
    ++priv->stat1; /* Increment statistic */
    return sizeof(uint32_t);
}

/* Optional "read" callback of "arr1" attribute */
static int test1_arr1_attr_read(struct bdmf_object *mo,
                                struct bdmf_attr *ad, bdmf_index index, void *val,
                                uint32_t size)
{
    struct test1_drv_priv *priv = (struct test1_drv_priv *)bdmf_obj_data(mo);
    if (priv->arr1[index] < 0)
        return BDMF_ERR_NOENT;
    *(int *)val = priv->arr1[index];
    return sizeof(int);
}
    
/** Optional "write" callback of "cfg1" attribute: value is in native format */
static int test1_cfg1_attr_write(struct bdmf_object *mo,
                                 struct bdmf_attr *ad, bdmf_index index, const void *val,
                                 uint32_t size)
{
    struct test1_drv_priv *priv = (struct test1_drv_priv *)bdmf_obj_data(mo);
    uint16_t cfg1 = *(uint16_t *)val;
    
    /* Lets say that odd numbers are bad and even numbers are good */
    if ((cfg1 & 1))
    {
        BDMF_TRACE_ERR("Attribute %s must be even. Got %u\n", ad->name, cfg1);
        return BDMF_ERR_PARM;
    }
    priv->cfg1 = cfg1;
    return sizeof(cfg1);
}

/*
 * Access to attributes in memory area #1
 */

/* Optional "read" callback of "stat2" attribute */
static int test1_stat2_attr_read(struct bdmf_object *mo,
                                struct bdmf_attr *ad, bdmf_index index, void *val,
                                uint32_t size)
{
    struct test1_area1_priv *priv = (struct test1_area1_priv *)mo->mem_seg_base[1];
    *(uint32_t *)val = priv->stat2;
    priv->stat2 = 1 - priv->stat2; /* Flip 1/0 */
    return sizeof(uint32_t);
}

/** Optional "write" callback of "cfg1" attribute: value is in native format */
static int test1_cfg2_attr_write(struct bdmf_object *mo,
                                 struct bdmf_attr *ad, bdmf_index index, const void *val,
                                 uint32_t size)
{
    struct test1_area1_priv *priv = (struct test1_area1_priv *)mo->mem_seg_base[1];
    uint32_t cfg2 = *(uint32_t *)val;

    /* Lets say that odd numbers are good and even numbers are bad */
    if (!(cfg2 & 1))
    {
        BDMF_TRACE_ERR("Attribute %s must be odd. Got %u\n", ad->name, cfg2);
        return BDMF_ERR_PARM;
    }
    priv->cfg2 = cfg2;
    return sizeof(cfg2);
}


/* Object attribute descriptors */
static struct bdmf_attr test1_attrs[] = {
    { .name="id1", .help="Automatically assigned unique object id",
      .type=bdmf_attr_number,
      .flags=BDMF_ATTR_READ | BDMF_ATTR_KEY | BDMF_ATTR_CONFIG,
      .size=sizeof(uint32_t), .offset=offsetof(struct test1_drv_priv, id)
    },
    { .name="initcfg1", .help="Init-time mandatory configuration",
      .type=bdmf_attr_number,
      .flags=BDMF_ATTR_READ | BDMF_ATTR_WRITE_INIT | BDMF_ATTR_MANDATORY | BDMF_ATTR_CONFIG,
      .size=sizeof(uint32_t), .offset=offsetof(struct test1_drv_priv, initcfg1)
    },
    { .name="cfg1", .help="Run-time configuration",
      .type=bdmf_attr_number,
      .flags=BDMF_ATTR_READ | BDMF_ATTR_WRITE | BDMF_ATTR_CONFIG,
      .size=sizeof(uint16_t), .offset=offsetof(struct test1_drv_priv, cfg1),
      .write=test1_cfg1_attr_write
    },
    { .name="stat1", .help="Statistic 1: increments after each display",
      .flags=BDMF_ATTR_READ | BDMF_ATTR_STAT, .size=sizeof(uint32_t),
      .read=test1_stat1_attr_read
    },
    { .name="arr1", .help="Array 1: only elements > 0 are displayed",
      .flags=BDMF_ATTR_READ | BDMF_ATTR_WRITE | BDMF_ATTR_CONFIG,
      .array_size=TEST1_ARR1_SIZE,
      .size=(sizeof(int)), .offset=offsetof(struct test1_drv_priv, arr1),
      .read=test1_arr1_attr_read
    },
    /* Attributes in a separate memory area */
    { .name="cfg2", .help="Config attribute in separate memory area",
      .type=bdmf_attr_number, .mem_seg=1,
      .flags=BDMF_ATTR_READ | BDMF_ATTR_WRITE | BDMF_ATTR_CONFIG,
      .size=sizeof(uint32_t), .offset=offsetof(struct test1_area1_priv, cfg2),
      .write=test1_cfg2_attr_write
    },
    { .name="stat2", .help="Statistic 2 in a separate mem area - flips 0/1",
      .type=bdmf_attr_number, .mem_seg=1,
      .flags=BDMF_ATTR_READ | BDMF_ATTR_STAT, .size=sizeof(uint32_t),
      .read=test1_stat2_attr_read
    },

    BDMF_ATTR_LAST
};

static struct bdmf_type test1_drv = {
    .name = "test1",
    .description = "Test1 BDMF plugin",
    .pre_init = test1_pre_init,
    .post_init = test1_post_init,
    .extra_size = sizeof(struct test1_drv_priv),
    .seg_type[0] = BDMF_MEM_CACHE,
    .seg_size[0] = sizeof(struct test1_drv_priv),
    .seg_type[1] = BDMF_MEM_CACHE,
    .seg_size[1] = sizeof(struct test1_area1_priv),
    .seg_auto_alloc[1] = 1,
    .aattr = test1_attrs
};

DECLARE_BDMF_TYPE(bdmf_test1, test1_drv);
