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
 * rdpa_ipsec.c
 *
 *  Created on: Mar 26, 2015
 */

#include "rdd.h"
#include "rdpa_api.h"
#include "rdpa_int.h"


/***************************************************************************
 * ipsec_object type
 **************************************************************************/

/* ipsec object private data */
typedef struct {
    uint32_t sa_table_ddr_addr;  /**< DDR SA table base address */
    uint16_t sa_entry_size;      /**< SA table entry size */
    rdpa_sa_desc_t sa_desc_ds[RDD_IPSEC_DS_SA_DESC_TABLE_SIZE];  /**< DS SA descriptors */
    rdpa_sa_desc_t sa_desc_us[RDD_IPSEC_US_SA_DESC_TABLE_SIZE];  /**< US SA descriptors */
    uint16_t sa_desc_cam_tbl_ds[RDD_IPSEC_DS_SA_DESC_CAM_TABLE_SIZE]; /**< DS SA descriptor cam table */
    uint16_t sa_desc_cam_tbl_us[RDD_IPSEC_US_SA_DESC_CAM_TABLE_SIZE]; /**< US SA descriptor cam table */
} ipsec_drv_priv_t;

static struct bdmf_object *ipsec_object;


/** This optional callback is called called at object init time
 *  before initial attributes are set.
 *  If function returns error code !=0, object creation is aborted
 */
static int ipsec_pre_init(struct bdmf_object *mo)
{
    ipsec_drv_priv_t *priv = (ipsec_drv_priv_t *)bdmf_obj_data(mo);

    priv->sa_table_ddr_addr = (uint32_t)RDPA_VALUE_UNASSIGNED;
    priv->sa_entry_size     = 0;
    memset((void *)&(priv->sa_desc_ds[0]), 0, sizeof(priv->sa_desc_ds));
    memset((void *)&(priv->sa_desc_us[0]), 0, sizeof(priv->sa_desc_us));
    memset((void *)&(priv->sa_desc_cam_tbl_ds[0]), 0xff, sizeof(priv->sa_desc_cam_tbl_ds));
    memset((void *)&(priv->sa_desc_cam_tbl_us[0]), 0xff, sizeof(priv->sa_desc_cam_tbl_us));

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
static int ipsec_post_init(struct bdmf_object *mo)
{
    ipsec_drv_priv_t *priv = (ipsec_drv_priv_t *)bdmf_obj_data(mo);

    ipsec_object = mo;

    snprintf(mo->name, sizeof(mo->name), "IPsec");

    if (priv->sa_table_ddr_addr != (uint32_t)RDPA_VALUE_UNASSIGNED &&
        priv->sa_entry_size > 0)
    {
        rdd_ipsec_sa_desc_table_address(priv->sa_table_ddr_addr, priv->sa_entry_size);
    }

    return 0;
}

static void ipsec_destroy(struct bdmf_object *mo)
{
    if (ipsec_object != mo)
        return;

    /* ToDo: do cleanups here */
    rdd_ipsec_sa_desc_table_address(0, 0);

    ipsec_object = NULL;
}


/*
 * enum tables
 */

/*
 * Attribute access functions
 */

/* "sa_desc" attribute "read" callback */
static int ipsec_attr_sa_desc_ds_read(struct bdmf_object *mo, struct bdmf_attr *ad,
                                      bdmf_index index, void *val, uint32_t size)
{
    ipsec_drv_priv_t *priv = (ipsec_drv_priv_t *)bdmf_obj_data(mo);

    rdd_ipsec_sa_desc_read(rdpa_dir_ds, (uint32_t)index, val);

    ((rdpa_sa_desc_t *)val)->spi           = ntohl(((rdpa_sa_desc_t *)val)->spi);
    ((rdpa_sa_desc_t *)val)->auth_config   = ntohl(((rdpa_sa_desc_t *)val)->auth_config);
    ((rdpa_sa_desc_t *)val)->crypt_config  = ntohl(((rdpa_sa_desc_t *)val)->crypt_config);
    ((rdpa_sa_desc_t *)val)->crypt_config2 = ntohl(((rdpa_sa_desc_t *)val)->crypt_config2);
 
    priv->sa_desc_ds[index] = *(rdpa_sa_desc_t *)val;

    if (priv->sa_desc_ds[index].spi == 0)
        return BDMF_ERR_NOENT;

    return 0;
}

static int ipsec_attr_sa_desc_us_read(struct bdmf_object *mo, struct bdmf_attr *ad,
                                      bdmf_index index, void *val, uint32_t size)
{
    ipsec_drv_priv_t *priv = (ipsec_drv_priv_t *)bdmf_obj_data(mo);

    rdd_ipsec_sa_desc_read(rdpa_dir_us, (uint32_t)index, val);

    ((rdpa_sa_desc_t *)val)->spi           = ntohl(((rdpa_sa_desc_t *)val)->spi);
    ((rdpa_sa_desc_t *)val)->auth_config   = ntohl(((rdpa_sa_desc_t *)val)->auth_config);
    ((rdpa_sa_desc_t *)val)->crypt_config  = ntohl(((rdpa_sa_desc_t *)val)->crypt_config);
    ((rdpa_sa_desc_t *)val)->crypt_config2 = ntohl(((rdpa_sa_desc_t *)val)->crypt_config2);
 
    priv->sa_desc_us[index] = *(rdpa_sa_desc_t *)val;

    if (priv->sa_desc_us[index].spi == 0)
        return BDMF_ERR_NOENT;

    return 0;
}

/* "sa_desc" attribute write callback */
static int ipsec_attr_sa_desc_ds_write(struct bdmf_object *mo, struct bdmf_attr *ad,
                                       bdmf_index index, const void *val, uint32_t size)
{
    ipsec_drv_priv_t *priv = (ipsec_drv_priv_t *)bdmf_obj_data(mo);

    if (((rdpa_sa_desc_t *)val)->spi && priv->sa_desc_ds[index].spi)
        return BDMF_ERR_ALREADY;

    priv->sa_desc_ds[index] = *(rdpa_sa_desc_t *)val;

    rdd_ipsec_sa_desc_write(rdpa_dir_ds, (uint32_t)index, val);

    return 0;
}

/* "sa_desc" attribute write callback */
static int ipsec_attr_sa_desc_us_write(struct bdmf_object *mo, struct bdmf_attr *ad,
                                       bdmf_index index, const void *val, uint32_t size)
{
    ipsec_drv_priv_t *priv = (ipsec_drv_priv_t *)bdmf_obj_data(mo);

    if (((rdpa_sa_desc_t *)val)->spi && priv->sa_desc_us[index].spi)
        return BDMF_ERR_ALREADY;

    priv->sa_desc_us[index] = *(rdpa_sa_desc_t *)val;

    rdd_ipsec_sa_desc_write(rdpa_dir_us, (uint32_t)index, val);

    return 0;
}

static int ipsec_attr_sa_desc_cam_tbl_ds_read(struct bdmf_object *mo, struct bdmf_attr *ad,
                                      bdmf_index index, void *val, uint32_t size)
{
    ipsec_drv_priv_t *priv = (ipsec_drv_priv_t *)bdmf_obj_data(mo);

    rdd_ipsec_sa_desc_cam_tbl_read(rdpa_dir_ds, (uint32_t)index, val);

    priv->sa_desc_cam_tbl_ds[index] = *(uint16_t *)val;

    return 0;
}

static int ipsec_attr_sa_desc_cam_tbl_us_read(struct bdmf_object *mo, struct bdmf_attr *ad,
                                      bdmf_index index, void *val, uint32_t size)
{
    ipsec_drv_priv_t *priv = (ipsec_drv_priv_t *)bdmf_obj_data(mo);

    rdd_ipsec_sa_desc_cam_tbl_read(rdpa_dir_us, (uint32_t)index, val);

    priv->sa_desc_cam_tbl_us[index] = *(uint16_t *)val;

    return 0;
}


/*
 * aggregate descriptors
 */
struct bdmf_aggr_type sa_desc_type = {
    .name = "sa_desc", .struct_name = "rdpa_sa_desc_t",
    .help = "SA Descriptor Configuration",
    .fields = (struct bdmf_attr[]) {
        { .name = "spi", .help = "Security Parameters Index",
          .flags = BDMF_ATTR_UNSIGNED | BDMF_ATTR_HEX_FORMAT,
          .type = bdmf_attr_number, .size = sizeof(uint32_t),
          .offset = offsetof(rdpa_sa_desc_t, spi)
        },
        { .name = "auth_config", .help = "Auth Config Register",
          .flags = BDMF_ATTR_UNSIGNED | BDMF_ATTR_HEX_FORMAT,
          .type = bdmf_attr_number, .size = sizeof(uint32_t),
          .offset = offsetof(rdpa_sa_desc_t, auth_config)
        },
        { .name = "crypt_config", .help = "Crypt Config Register",
          .flags = BDMF_ATTR_UNSIGNED | BDMF_ATTR_HEX_FORMAT,
          .type = bdmf_attr_number, .size = sizeof(uint32_t),
          .offset = offsetof(rdpa_sa_desc_t, crypt_config)
        },
        { .name = "crypt_config2", .help = "Crypt Config2 Register",
          .flags = BDMF_ATTR_UNSIGNED | BDMF_ATTR_HEX_FORMAT,
          .type = bdmf_attr_number, .size = sizeof(uint32_t),
          .offset = offsetof(rdpa_sa_desc_t, crypt_config2),
        },
        { .name = "auth_key", .help = "Auth Key",
          .flags = BDMF_ATTR_UNSIGNED,
          .type = bdmf_attr_buffer, .size = IPSEC_AUTH_KEY_SIZE_MAX,
          .offset = offsetof(rdpa_sa_desc_t, auth_key)
        },
        { .name = "crypt_key", .help = "Crypt Key",
          .flags = BDMF_ATTR_UNSIGNED,
          .type = bdmf_attr_buffer, .size = IPSEC_CRYPT_KEY_SIZE_MAX,
          .offset = offsetof(rdpa_sa_desc_t, crypt_key)
        },
        BDMF_ATTR_LAST
    }
};
DECLARE_BDMF_AGGREGATE_TYPE(sa_desc_type);


/*
 * Object attribute descriptors
 */
static struct bdmf_attr ipsec_attrs[] = {
    { .name = "sa_table_ddr_addr", .help = "ddr sa table base address",
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE_INIT | BDMF_ATTR_CONFIG | BDMF_ATTR_MANDATORY | BDMF_ATTR_UNSIGNED | BDMF_ATTR_HEX_FORMAT,
        .type = bdmf_attr_number, .size = sizeof(uint32_t),
        .offset = offsetof(ipsec_drv_priv_t, sa_table_ddr_addr)
    },
    { .name = "sa_entry_size", .help = "sa table entry size",
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE_INIT | BDMF_ATTR_CONFIG | BDMF_ATTR_MANDATORY,
        .type = bdmf_attr_number, .size = sizeof(uint16_t),
        .offset = offsetof(ipsec_drv_priv_t, sa_entry_size)
    },
    { .name = "sa_desc_ds", .help = "Runner Downstream SA Descriptor",
        .type = bdmf_attr_aggregate, .ts.aggr_type_name = "sa_desc", .array_size = RDD_IPSEC_DS_SA_DESC_TABLE_SIZE,
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE | BDMF_ATTR_CONFIG | BDMF_ATTR_NOLOCK,
        .read = ipsec_attr_sa_desc_ds_read, .write = ipsec_attr_sa_desc_ds_write
    },
    { .name = "sa_desc_us", .help = "Runner Upstream SA Descriptor",
        .type = bdmf_attr_aggregate, .ts.aggr_type_name = "sa_desc", .array_size = RDD_IPSEC_US_SA_DESC_TABLE_SIZE,
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE | BDMF_ATTR_CONFIG | BDMF_ATTR_NOLOCK,
        .read = ipsec_attr_sa_desc_us_read, .write = ipsec_attr_sa_desc_us_write
    },
    { .name = "sa_desc_cam_tbl_ds", .help = "Runner Downstream SA Descriptor CAM Table",
        .type = bdmf_attr_number, .size = sizeof(uint16_t), .array_size = RDD_IPSEC_DS_SA_DESC_CAM_TABLE_SIZE,
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE_INIT | BDMF_ATTR_CONFIG | BDMF_ATTR_MANDATORY,
        .read = ipsec_attr_sa_desc_cam_tbl_ds_read
    },
    { .name = "sa_desc_cam_tbl_us", .help = "Runner Upstream SA Descriptor CAM Table",
        .type = bdmf_attr_number, .size = sizeof(uint16_t), .array_size = RDD_IPSEC_US_SA_DESC_CAM_TABLE_SIZE,
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE_INIT | BDMF_ATTR_CONFIG | BDMF_ATTR_MANDATORY,
        .read = ipsec_attr_sa_desc_cam_tbl_us_read
    },
    BDMF_ATTR_LAST
};

static int ipsec_drv_init(struct bdmf_type *drv);
static void ipsec_drv_exit(struct bdmf_type *drv);

struct bdmf_type ipsec_drv = {
    .name = "ipsec",
    .parent = "system",
    .description = "IPsec Interface",
    .drv_init = ipsec_drv_init,
    .drv_exit = ipsec_drv_exit,
    .pre_init = ipsec_pre_init,
    .post_init = ipsec_post_init,
    .destroy = ipsec_destroy,
    .extra_size = sizeof(ipsec_drv_priv_t),
    .aattr = ipsec_attrs,
    .max_objs = 1,
};
DECLARE_BDMF_TYPE(rdpa_ipsec, ipsec_drv);


/***************************************************************************
 * Functions declared in auto-generated header
 **************************************************************************/

/** Get ipsec object by key
 * \param[out] ipsec_obj     Object handle
 * \return  0=OK or error <0
 */
int rdpa_ipsec_get(bdmf_object_handle *ipsec_obj)
{
    if (!ipsec_object || ipsec_object->state == bdmf_state_deleted)
        return BDMF_ERR_NOENT;
    bdmf_get(ipsec_object);
    *ipsec_obj = ipsec_object;
    return 0;
}


/***************************************************************************
 * Additional manually-written functions
 **************************************************************************/

/* Init/exit module. Cater for GPL layer */
static int ipsec_drv_init(struct bdmf_type *drv)
{
#ifdef BDMF_DRIVER_GPL_LAYER
    f_rdpa_ipsec_drv = rdpa_ipsec_drv;
    f_rdpa_ipsec_get = rdpa_ipsec_get;
#endif
    return 0;
}

static void ipsec_drv_exit(struct bdmf_type *drv)
{
#ifdef BDMF_DRIVER_GPL_LAYER
    f_rdpa_ipsec_drv = NULL;
    f_rdpa_ipsec_get = NULL;
#endif
}
