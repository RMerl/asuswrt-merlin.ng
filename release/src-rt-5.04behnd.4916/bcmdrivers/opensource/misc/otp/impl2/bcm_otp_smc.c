/* 
   <:copyright-BRCM:2011:DUAL/GPL:standard

   Copyright (c) 2011 Broadcom 
   All Rights Reserved

   Unless you and Broadcom execute a separate written software license
   agreement governing use of this software, this software is licensed
   to you under the terms of the GNU General Public License version 2
   (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
   with the following added to such license:

   As a special exception, the copyright holders of this software give
   you permission to link this software with independent modules, and
   to copy and distribute the resulting executable under terms of your
   choice, provided that you also meet, for each linked independent
   module, the terms and conditions of the license of that module.
   An independent module is a module which is not derived from this
   software.  The special exception does not apply to any modifications
   of the software.

   Not withstanding the above, under no circumstances may you combine
   this software in any way with any other Broadcom software provided
   under a license other than the GPL, without Broadcom's express prior
   written consent.

   :>
 */

#include <linux/io.h>
#include <linux/string.h>
#include <linux/spinlock.h>
//#include <bcm_otp_map.h>
#include <bcm_strap_drv.h>
#include <linux/module.h>
#include <linux/of_fdt.h>
#include <itc_rpc.h>
#include <linux/dma-mapping.h>
#include <linux/slab.h>
#include "otp_map.h"
#include "bcm_otp.h"
#include "shared_utils.h"

extern struct device *otp_dev;

//#define OTP_DEBUG

#ifdef OTP_DEBUG
#define OTP_DBG(fmt, args...)   printk("%s#%d: " fmt, __FUNCTION__, __LINE__, ##args)
#define OTP_HEXDUMP(prefix, data, size) print_hex_dump(KERN_CONT, prefix, DUMP_PREFIX_OFFSET, 16, 1, data, size, false)
#else
#define OTP_DBG(fmt, args...)
#define OTP_HEXDUMP(prefix, data, size)
#endif
#define OTP_ERR(fmt, args...)   printk("%s#%d: error: " fmt, __FUNCTION__, __LINE__, ##args)

enum
{
    OTP_TYPE_OTP = 0,
    OTP_TYPE_SOTP = 1
};

/* SMC OTP field names */
/* TODO Igor trim the field name list */
#define k_FIELD_STR_DBG_MODE            "DBG_MODE"
#define k_FIELD_STR_FU_DIS              "FU_DIS"
#define k_FIELD_STR_AUTH_EN             "AUTH_EN"
#define k_FIELD_STR_ENC_EN              "ENC_EN"
#define k_FIELD_STR_JU_MODE				"JU_MODE"
#define k_FIELD_STR_MRKT_ID             "MRKT_ID"
#define k_FIELD_STR_FAIL_CFG            "FAIL_CFG"
#define k_FIELD_STR_FAIL_DLY            "FAIL_DLY"
#define k_FIELD_STR_LED_TYPE            "LED_TYPE"
#define k_FIELD_STR_LED_MODE            "LED_MODE"
#define k_FIELD_STR_LED_TP              "LED_TP"
#define k_FIELD_STR_LED_CLR             "LED_CLR"
#define k_FIELD_STR_LED_INT             "LED_INT"
#define k_FIELD_STR_LED_CURR            "LED_CURR"
#define k_FIELD_STR_CUL_NONCE_DIS       "NONC_DIS"
#define k_FIELD_STR_FM_CMPLT            "FM_CMPLT"
#define k_FIELD_STR_CHIPID              "CHIPID"
#define k_FIELD_STR_JTAGPWD             "JTAGPWD"
#define k_FIELD_STR_RSA_KEY             "RSA_KEY"
#define k_FIELD_STR_RSA_KEYB            "RSA_KEYB"
#define k_FIELD_STR_EC_KEY              "EC_KEY"
#define k_FIELD_STR_GEN_ROW0            "GEN_ROW0"
#define k_FIELD_STR_GEN_ROW1            "GEN_ROW1"
#define k_FIELD_STR_GEN_ROW2            "GEN_ROW2"
#define k_FIELD_STR_GEN_ROW3            "GEN_ROW3"
#define k_FIELD_STR_UNQ_REENC_CMPLT     "RENCDONE"
#define k_FIELD_STR_BP3_INIT            "BP3_INIT"
#define k_FIELD_STR_FLD_HMID            "FLD_HMID"
#define k_FIELD_STR_FLD_EK              "FLDEK128"
#define k_FIELD_STR_FLD_EK_256          "FLDEK256"
#define k_FIELD_STR_FLD_IV              "FLDIV"
#define k_FIELD_STR_SECBOOT             "SECBOOT"
#define k_FIELD_STR_CUST_KEYS           "CUSTKEYS"
#define k_FIELD_STR_LEDS                "LEDS"

#define OTP_MFG_SECURE_MASK             0x1
#define OTP_FLD_SECURE_MASK             0x2

/*
 * OTP feat map
 */

typedef struct bcm_otp_feat_info bcm_otp_feat_info;
typedef otp_map_cmn_err_t (*otp_read_cb)(bcm_otp_feat_info *feat_info);
typedef otp_map_cmn_err_t (*otp_write_cb)(bcm_otp_feat_info *feat_info);

extern int otp_rpc_tunnel_id;

typedef struct bcm_otp_feat_info
{
    otp_map_feat_t feat;
    const char *fld_name;   /* OTP field name - used by bcm_otp_default_read_cb, bcm_olt_default_write_cb */
    u32 fld_size;           /* OTP field size */
    otp_read_cb read_cb;    /* Field read callback. bcm_otp_default_read_cb is used by default */
    otp_read_cb write_cb;   /* Field write callback. bcm_otp_default_write_cb is used by default */
    u32 *cache;             /* Cached data */
    u32 size;               /* Size of data in cache */
} bcm_otp_feat_info;

static otp_map_cmn_err_t bcm_otp_default_read_cb(bcm_otp_feat_info *feat_info);
static otp_map_cmn_err_t bcm_otp_default_write_cb(bcm_otp_feat_info *feat_info);
static otp_map_cmn_err_t bcm_otp_swap_read_cb(bcm_otp_feat_info *feat_info); 
static otp_map_cmn_err_t bcm_otp_swap_write_cb(bcm_otp_feat_info *feat_info);

static bcm_otp_feat_info otp_feat_info[] = {
	{ .feat = OTP_MAP_CUST_MFG_MRKTID, .fld_name = k_FIELD_STR_MRKT_ID, .fld_size = 2, .read_cb = bcm_otp_swap_read_cb, .write_cb = bcm_otp_swap_write_cb},
	{ .feat = OTP_MAP_CSEC_CHIPID, .fld_name = k_FIELD_STR_CHIPID, .fld_size = 4 , .read_cb = bcm_otp_swap_read_cb, .write_cb = bcm_otp_swap_write_cb},
	{ .feat = OTP_MAP_JTAG_PWD, .fld_name = k_FIELD_STR_JTAGPWD, .fld_size = 8, .read_cb = bcm_otp_swap_read_cb, .write_cb = bcm_otp_swap_write_cb },
	{ .feat = OTP_MAP_DBG_MODE, .fld_name = k_FIELD_STR_DBG_MODE, .fld_size = 4 },
	{ .feat = OTP_MAP_JU_MODE, .fld_name = k_FIELD_STR_JU_MODE, .fld_size = 4 },
	{ .feat = OTP_MAP_LEDS, .fld_name = k_FIELD_STR_LEDS, .fld_size = 5},
};
//
// OTP RPC access helpers
//

#define RPC_TIMEOUT  5     /* secs */

//OTP Service RPC commands
typedef enum
{
    k_OTP_FIELD_ID,
    k_OTP_FIELD_NAME,
    k_OTP_FIELD_GET,
    k_OTP_FIELD_SET,
    k_OTP_COMMIT,
    k_OTP_LOCK_REGION,
    k_OTP_SVC_FUNC__NUM_OF
} otp_svc_func_t;

static char *otp_rpc_svc_func_nume(otp_svc_func_t otp_svc_func)
{
    static char *svc_func_name[k_OTP_SVC_FUNC__NUM_OF] = {
        [k_OTP_FIELD_ID]    = "field-id",
        [k_OTP_FIELD_NAME]  = "field-name",
        [k_OTP_FIELD_GET]   = "field-get",
        [k_OTP_FIELD_SET]   = "field-set",
        [k_OTP_COMMIT]      = "commit",
        [k_OTP_LOCK_REGION] = "lock-region"
    };
    if (otp_svc_func >= k_OTP_SVC_FUNC__NUM_OF)
        return "invalid-func";
    return svc_func_name[otp_svc_func];
}

static inline int8_t otp_rpc_rc(const rpc_msg *msg)
{
    return (int8_t)(msg->data[0] >> 24);
}

//-------------------------------------------------------
//The following functions allow the caller to get or set
//an OTP field managed by the OTP Service application
//without needing to know the details (row# nor bits) of
//the field itself -- just like how the host accesses
//customer OTP fields managed by SMC.
//-------------------------------------------------------
static int8_t _otp_access(otp_svc_func_t otp_svc_func, const char *p_field_name,
        u8 *p_buf, u32 buf_sz)
{
    rpc_msg msg;
    u8 field_id = 0xff;
    int8_t rc = -1;

    if (p_field_name)
    {
        rpc_msg_init(&msg, RPC_SERVICE_OTP, k_OTP_FIELD_ID, 0, 0, 0, 0);
        strncpy((char*)&(msg.data[1]), p_field_name, 8);
        rc = rpc_send_request_timeout(otp_rpc_tunnel_id, &msg, RPC_TIMEOUT);
        if (rc) 
        {
            OTP_ERR("otp: rpc_send_request failure (%d)\n", rc);
            return rc;
        }
        if (otp_rpc_rc(&msg) != 0)
        {
            OTP_ERR("otp: Failed to get field \"%s\" info\n",  p_field_name);
            return -1;
        }
        field_id = msg.data[0] & 0xff;
    }

    rpc_msg_init(&msg, RPC_SERVICE_OTP, (u32)otp_svc_func, 0, 0, 0, 0);

    if ((otp_svc_func == k_OTP_FIELD_SET) || (otp_svc_func == k_OTP_FIELD_GET))
    {
        if (!p_buf || field_id == 0xff)
        {
            OTP_ERR("%s: error in parameters\n", __FUNCTION__);
            return -1;
        }
        msg.data[0] = (u32)((((long)p_buf) >> 32) << 24) | field_id;
        msg.data[1] = (u32)(long)p_buf;
        if (otp_svc_func == k_OTP_FIELD_SET)
            msg.data[0] |= ((buf_sz * 8) & 0xffff) << 8;
    }
    else if (otp_svc_func != k_OTP_COMMIT)
    {
        OTP_ERR("otp: service function '%s' is not supported\n", otp_rpc_svc_func_nume(otp_svc_func));
        return -1;
    }

    rc = rpc_send_request_timeout(otp_rpc_tunnel_id, &msg, RPC_TIMEOUT);
    if (!rc)
        rc = otp_rpc_rc(&msg);
    if (rc) 
    {
        OTP_ERR("otp: %s request failure\n", otp_rpc_svc_func_nume(otp_svc_func));
        return rc;
    }
    return 0;
}

static int8_t _otp_rpc_field_get(const char *p_field_name, u8 *p_buf)
{
    return _otp_access(k_OTP_FIELD_GET, p_field_name, p_buf, 0);
}

static int8_t _otp_rpc_field_set(const char *p_field_name, u8 *p_data, u32 data_len)
{
    return _otp_access(k_OTP_FIELD_SET, p_field_name, p_data, data_len);
}

static int8_t _otp_rpc_commit(void)
{
    int rc;
    rc = _otp_access(k_OTP_COMMIT, NULL, NULL, 0);
    OTP_DBG("rc=%d\n", rc);
    return rc;
}

/*
 * OTP Read/write callbacks
 */

static bcm_otp_feat_info *otp_feat_info_get(otp_map_feat_t otp_feat)
{
    int i;
    for (i = 0; i < sizeof(otp_feat_info)/sizeof(otp_feat_info[0]); i++)
    {
        if (otp_feat_info[i].feat == otp_feat)
            return &otp_feat_info[i];
    }
    return NULL;
}

u32 otp_feat_info_get_size(otp_map_feat_t otp_feat)
{
    int i;
    for (i = 0; i < sizeof(otp_feat_info)/sizeof(otp_feat_info[0]); i++)
    {
        if (otp_feat_info[i].feat == otp_feat)
            return otp_feat_info[i].fld_size;
    }
	return 0;
}

/* OTP read helper */
static otp_map_cmn_err_t bcm_otp_field_read(const char *fld_name, u32 *data, u32 fld_size)
{
    otp_map_cmn_err_t rc;
	dma_addr_t dma_addr = 0;

    // Prepare Request
	dma_addr = dma_map_single(otp_dev, data, fld_size, DMA_FROM_DEVICE);
	if (dma_mapping_error(otp_dev, dma_addr)) {
		pr_err("Failed to map buffer for DMA use");
		rc = (-ENOMEM);
		return rc;
	}

    rc = _otp_rpc_field_get(fld_name, (unsigned char*)dma_addr);

	OTP_DBG("fld=%s size=%u rc=%d\n", fld_name, fld_size,  rc);
    OTP_HEXDUMP("data:", data, fld_size);

    dma_unmap_single(otp_dev, dma_addr, fld_size, DMA_FROM_DEVICE);

	return rc;
}

/* OTP write helper */
static otp_map_cmn_err_t bcm_otp_field_write(const char *fld_name, u32 *data, u32 fld_size)
{
    otp_map_cmn_err_t rc;
	dma_addr_t dma_addr = 0;

	// Prepare Request
	dma_addr = dma_map_single(otp_dev, data, fld_size, DMA_TO_DEVICE);
	if (dma_mapping_error(otp_dev, dma_addr)) {
		pr_err("Failed to map buffer for DMA use");
		rc = (-ENOMEM);
		return rc;
	}

    rc = _otp_rpc_field_set(fld_name, (unsigned char*)dma_addr, fld_size);
    OTP_DBG("fld=%s size=%u rc=%d\n", fld_name, fld_size, rc);
    OTP_HEXDUMP("data:", data, fld_size);

	dma_unmap_single(otp_dev, dma_addr, fld_size, DMA_TO_DEVICE);

    return rc;
}

/* Default read callback */
static otp_map_cmn_err_t bcm_otp_default_read_cb(bcm_otp_feat_info *feat_info)
{
    otp_map_cmn_err_t rc;
    if (feat_info->fld_name == NULL || !feat_info->fld_size)
        return OTP_MAP_CMN_ERR_INVAL;
    rc = bcm_otp_field_read(feat_info->fld_name, feat_info->cache, feat_info->fld_size);
    feat_info->size = feat_info->fld_size;
    return rc;
}

static otp_map_cmn_err_t bcm_otp_default_write_cb(bcm_otp_feat_info *feat_info)
{
    otp_map_cmn_err_t rc;
    if (feat_info->fld_name == NULL || !feat_info->fld_size)
        return OTP_MAP_CMN_ERR_INVAL;
    rc = bcm_otp_field_write(feat_info->fld_name, feat_info->cache, feat_info->size);
    return rc;
}


static void swap_buf( char* buf, int size)
{
	int i;
    for ( i=0; i< size/2; i++ ) 
    {
        char t = buf[i];
        buf[i] = buf[size-i-1];
        buf[size-i-1] = t;
    }
}

/* swap read callback */
static otp_map_cmn_err_t bcm_otp_swap_write_cb(bcm_otp_feat_info *feat_info)
{
	swap_buf((char*)feat_info->cache, feat_info->size);

    return bcm_otp_default_write_cb(feat_info);
}

static otp_map_cmn_err_t bcm_otp_swap_read_cb(bcm_otp_feat_info *feat_info)
{
    otp_map_cmn_err_t rc;
	rc = bcm_otp_default_read_cb(feat_info);
	if (!rc)
	{
        swap_buf((char*)feat_info->cache, feat_info->size);
	}

	return rc;
}

int bcm_otp_init()
{
    int i;
    for (i = 0; i < sizeof(otp_feat_info)/sizeof(otp_feat_info[0]); i++)
    {
        if (!otp_feat_info[i].fld_size)
            return OTP_MAP_CMN_ERR_INVAL;
        otp_feat_info[i].cache = kmalloc(otp_feat_info[i].fld_size, GFP_KERNEL);
        if (otp_feat_info[i].cache == NULL)
            return OTP_MAP_CMN_ERR_FAIL;
		memset(otp_feat_info[i].cache, 0, otp_feat_info[i].fld_size);
    }
    OTP_DBG("Init completed\n");
    return OTP_MAP_CMN_OK;
}

otp_map_cmn_err_t bcm_otp_read(otp_map_feat_t otp_feat, u32** data, u32* size)
{
    bcm_otp_feat_info *feat_info = otp_feat_info_get(otp_feat);
    otp_read_cb read_cb;
    otp_map_cmn_err_t rc;
    
    if (feat_info == NULL)
    {
        OTP_ERR("%s: feat %d is not supported\n", __FUNCTION__, otp_feat);
        return OTP_MAP_CMN_ERR_UNSP;
    }
    read_cb = (feat_info->read_cb != NULL) ? feat_info->read_cb : bcm_otp_default_read_cb;
    rc = read_cb(feat_info);
    if (rc)
	{
	    return rc;
	}
	*data = feat_info->cache;
    *size = feat_info->size;
    return OTP_MAP_CMN_OK;
}

otp_map_cmn_err_t bcm_otp_get(otp_map_feat_t otp_feat, u32* data)
{
    otp_map_cmn_err_t  rc = OTP_MAP_CMN_ERR_FAIL;
    u32 size = 0;
    u32* p = NULL;
    rc = bcm_otp_read(otp_feat, &p, &size);
    if (rc) {
        goto err;
    }
    if (size > sizeof(u32)) {
        rc =  OTP_MAP_CMN_ERR_INVAL;
        goto err;
    }
    memcpy(data, p ,sizeof(u32));
    rc = OTP_MAP_CMN_OK;
err:
    return rc;
}

otp_map_cmn_err_t bcm_otp_write(otp_map_feat_t otp_feat, const u32* data, u32 size)
{
    bcm_otp_feat_info *feat_info = otp_feat_info_get(otp_feat);
    otp_write_cb write_cb;
    
    if (feat_info == NULL)
    {
        OTP_ERR("%s: feat %d is not supported\n", __FUNCTION__, otp_feat);
        return OTP_MAP_CMN_ERR_UNSP;
    }
    if (size > feat_info->fld_size)
    {
        OTP_ERR("%s: size %u is too large for feat %d\n", __FUNCTION__, size, otp_feat);
        return OTP_MAP_CMN_ERR_UNSP;
    }
    if (!size)
        size = feat_info->fld_size;
    write_cb = (feat_info->write_cb != NULL) ? feat_info->write_cb : bcm_otp_default_write_cb;
    memcpy(feat_info->cache, data, size);
    feat_info->size = size;
    return write_cb(feat_info);
}

/* Fuse all staged updates */
otp_map_cmn_err_t bcm_otp_commit(void)
{
    return _otp_rpc_commit();
}

otp_map_cmn_err_t bcm_otp_read_slice(otp_map_feat_t otp_feat, u8 *buffer, u32 offset, u32 size)
{
    u32 *field_cache;
    u32 field_size;
    otp_map_cmn_err_t rc;

    if (!buffer)
    {
        OTP_ERR("%s: buffer is NULL\n", __FUNCTION__);
        return OTP_MAP_CMN_ERR_INVAL;
    }
    rc = bcm_otp_read(otp_feat, &field_cache, &field_size);
    if (rc != OTP_MAP_CMN_OK)
        return rc;
    if (offset + size > field_size)
    {
        OTP_ERR("%s: offset+size %u is too large for feat %d\n", 
            __FUNCTION__, offset+size, otp_feat);
        return OTP_MAP_CMN_ERR_INVAL;
    }

    memcpy(buffer, ((u8 *)field_cache) + offset, size);
    return OTP_MAP_CMN_OK;
}

otp_map_cmn_err_t bcm_otp_write_slice(otp_map_feat_t otp_feat, u8 *buffer, u32 offset, u32 size)
{
    u32 *field_cache;
    u32 field_size;
    otp_map_cmn_err_t rc;

    if (!buffer)
    {
        OTP_ERR("%s: buffer is NULL\n", __FUNCTION__);
        return OTP_MAP_CMN_ERR_INVAL;
    }
    rc = bcm_otp_read(otp_feat, &field_cache, &field_size);
    if (rc != OTP_MAP_CMN_OK)
        return rc;
    if (offset + size > field_size)
    {
        OTP_ERR("%s: offset+size %u is too large for feat %d\n", 
            __FUNCTION__, offset+size, otp_feat);
        return OTP_MAP_CMN_ERR_INVAL;
    }

    memcpy(((u8 *)field_cache) + offset, buffer, size);
    rc = bcm_otp_write(otp_feat, field_cache, field_size);
    return rc;
}
