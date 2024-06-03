/* SPDX-License-Identifier: GPL-2.0+
 *
 * Copyright 2020 Broadcom Ltd.
 */
#include <common.h>
#include <linux/ctype.h>
#include <linux/io.h>
#include "linux/printk.h"
#include <asm/arch/misc.h>
#include <malloc.h>
#include "itc_rpc.h"
#include "bcm_secure.h"
#include "bcm_otp.h"

//#define OTP_DEBUG

#ifdef OTP_DEBUG
#include <hexdump.h>
#define OTP_DBG(fmt, args...)   printf("%s#%d: " fmt, __FUNCTION__, __LINE__, ##args)
#define OTP_HEXDUMP(prefix, data, size)   \
    print_hex_dump_bytes(prefix, DUMP_PREFIX_ADDRESS, data, size)
#else
#define OTP_DBG(fmt, args...)
#define OTP_HEXDUMP(prefix, data, size)
#endif
#define OTP_ERR(fmt, args...)   printf("%s#%d: error: " fmt, __FUNCTION__, __LINE__, ##args)

#define SOTP_EXCHANGE_AREA_ADDR_START  0x00640000
#define SOTP_EXCHANGE_AREA_ADDR_END    0x0067FFFF

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
#define k_FIELD_STR_JU_MODE             "JU_MODE"
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
#define k_FIELD_STR_PROCESS             "PROCESS"
#define k_FIELD_STR_SUBSTRATE           "SUBSTRAT"
#define k_FIELD_STR_FOUNDARY            "FOUNDARY"

#define OTP_MFG_SECURE_MASK             0x1
#define OTP_FLD_SECURE_MASK             0x2

/*
 * OTP feat map
 */

typedef struct bcm_otp_feat_info bcm_otp_feat_info;
typedef otp_map_cmn_err_t (*otp_read_cb)(bcm_otp_feat_info *feat_info);
typedef otp_map_cmn_err_t (*otp_write_cb)(bcm_otp_feat_info *feat_info);

typedef struct bcm_otp_feat_info
{
    otp_map_feat_t feat;
    const char *fld_name;   /* OTP field name - used by bcm_otp_default_read_cb, bcm_olt_default_write_cb */
    u32 fld_size;           /* OTP field size */
    int is_sotp;            /* 1=field is in SOTP */
    otp_read_cb read_cb;    /* Field read callback. bcm_otp_default_read_cb is used by default */
    otp_read_cb write_cb;   /* Field write callback. bcm_otp_default_write_cb is used by default */
    u32 *cache;             /* Cached data */
    u32 size;               /* Size of data in cache */
} bcm_otp_feat_info;

static otp_map_cmn_err_t bcm_otp_default_read_cb(bcm_otp_feat_info *feat_info);
static otp_map_cmn_err_t bcm_otp_default_write_cb(bcm_otp_feat_info *feat_info);
static otp_map_cmn_err_t otp_fld_roe_read_cb(bcm_otp_feat_info *feat_info);
static otp_map_cmn_err_t otp_fld_roe_write_cb(bcm_otp_feat_info *feat_info);
static otp_map_cmn_err_t otp_mfg_mode_read_cb(bcm_otp_feat_info *feat_info);
static otp_map_cmn_err_t otp_mfg_mode_write_cb(bcm_otp_feat_info *feat_info);
static otp_map_cmn_err_t otp_fld_mode_read_cb(bcm_otp_feat_info *feat_info);
static otp_map_cmn_err_t otp_fld_mode_write_cb(bcm_otp_feat_info *feat_info);
static otp_map_cmn_err_t bcm_otp_swap_write_cb(bcm_otp_feat_info *feat_info);
static otp_map_cmn_err_t bcm_otp_swap_read_cb(bcm_otp_feat_info *feat_info);
static otp_map_cmn_err_t bcm_otp_empty_read_cb(bcm_otp_feat_info *feat_info);

static bcm_otp_feat_info otp_feat_info[] = {
    { .feat = OTP_MAP_CUST_MFG_MRKTID, .fld_name = k_FIELD_STR_MRKT_ID, .fld_size = 3 },
    { .feat = OTP_MAP_CSEC_CHIPID, .fld_name = k_FIELD_STR_CHIPID, .fld_size = 4 },
    { .feat = SOTP_MAP_FLD_ROE, .fld_size = 48,
      .read_cb = otp_fld_roe_read_cb, .write_cb = otp_fld_roe_write_cb },
    { .feat = SOTP_MAP_FLD_HMID, .fld_name = k_FIELD_STR_FLD_HMID, .fld_size = 32, .is_sotp=1 },
    { .feat = OTP_MAP_BRCM_BTRM_BOOT_ENABLE, .fld_size = sizeof(u32),
      .read_cb = otp_mfg_mode_read_cb, .write_cb = otp_mfg_mode_write_cb },
    { .feat = OTP_MAP_CUST_BTRM_BOOT_ENABLE, .fld_size = sizeof(u32),
      .read_cb = otp_fld_mode_read_cb, .write_cb = otp_fld_mode_write_cb },
    { .feat = SOTP_MAP_CUST_AREA, .fld_name = k_FIELD_STR_CUST_KEYS, .fld_size = 2320, .is_sotp=1 },
    { .feat = SOTP_MAP_CUST_NECC0, .fld_name = k_FIELD_STR_GEN_ROW0, .fld_size = 9, .is_sotp=1 },
    { .feat = SOTP_MAP_CUST_NECC1, .fld_name = k_FIELD_STR_GEN_ROW1, .fld_size = 9, .is_sotp=1 },
    { .feat = SOTP_MAP_CUST_NECC2, .fld_name = k_FIELD_STR_GEN_ROW2, .fld_size = 9, .is_sotp=1 },
    { .feat = SOTP_MAP_CUST_NECC3, .fld_name = k_FIELD_STR_GEN_ROW3, .fld_size = 9, .is_sotp=1 },
    { .feat = OTP_MAP_JTAG_PWD, .fld_name = k_FIELD_STR_JTAGPWD, .fld_size = 8, .read_cb = bcm_otp_empty_read_cb},
    { .feat = OTP_MAP_DBG_MODE, .fld_name = k_FIELD_STR_DBG_MODE, .fld_size = 4 },
    { .feat = OTP_MAP_JU_MODE, .fld_name = k_FIELD_STR_JU_MODE, .fld_size = 4 },
    { .feat = OTP_MAP_LEDS, .fld_name = k_FIELD_STR_LEDS, .fld_size = 5, 
      .write_cb = bcm_otp_swap_write_cb, .read_cb = bcm_otp_swap_read_cb},
    { .feat = OTP_MAP_MFG_PROCESS,   .fld_name = k_FIELD_STR_PROCESS,   .fld_size = sizeof(u32)},
    { .feat = OTP_MAP_MFG_SUBSTRATE, .fld_name = k_FIELD_STR_SUBSTRATE, .fld_size = sizeof(u32)}, 
    { .feat = OTP_MAP_MFG_FOUNDRY,   .fld_name = k_FIELD_STR_FOUNDARY,  .fld_size = sizeof(u32)},   
};
static u32 zero_buf[16];

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
        rc = rpc_send_request_timeout(RPC_TUNNEL_ARM_SMC_NS, &msg, RPC_TIMEOUT);
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

    rc = rpc_send_request_timeout(RPC_TUNNEL_ARM_SMC_NS, &msg, RPC_TIMEOUT);
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

/* OTP read helper */
static otp_map_cmn_err_t bcm_otp_field_read(const char *fld_name, int is_sotp, u32 *data, u32 fld_size)
{
    u8 *read_buf = is_sotp ? (u8 *)SOTP_EXCHANGE_AREA_ADDR_START : (u8 *)data;
    otp_map_cmn_err_t rc;

    invalidate_dcache_range((unsigned long)read_buf, (unsigned long)read_buf + fld_size - 1);
    rc = _otp_rpc_field_get(fld_name, read_buf);
    if (!rc && is_sotp)
        memcpy(data, read_buf, fld_size);
    OTP_DBG("fld=%s size=%u sotp=%d rc=%d\n", fld_name, fld_size, is_sotp, rc);
    OTP_HEXDUMP("data:", data, fld_size);
    return rc;
}

/* OTP write helper */
static otp_map_cmn_err_t bcm_otp_field_write(const char *fld_name, int is_sotp, u32 *data, u32 fld_size)
{
    u8 *write_buf = is_sotp ? (u8 *)SOTP_EXCHANGE_AREA_ADDR_START : (u8 *)data;
    otp_map_cmn_err_t rc;

    if (is_sotp)
        memcpy(write_buf, data, fld_size);
    flush_dcache_range((unsigned long)write_buf, (unsigned long)write_buf + fld_size - 1);
    rc = _otp_rpc_field_set(fld_name, write_buf, fld_size);
    OTP_DBG("fld=%s size=%u sotp=%d rc=%d\n", fld_name, fld_size, is_sotp, rc);
    OTP_HEXDUMP("data:", write_buf, fld_size);
    return rc;
}

/* Default read callback */
static otp_map_cmn_err_t bcm_otp_default_read_cb(bcm_otp_feat_info *feat_info)
{
    otp_map_cmn_err_t rc;
    if (feat_info->fld_name == NULL || !feat_info->fld_size)
        return OTP_MAP_CMN_ERR_INVAL;
    rc = bcm_otp_field_read(feat_info->fld_name, feat_info->is_sotp, 
         feat_info->cache, feat_info->fld_size);
    feat_info->size = feat_info->fld_size;
    return rc;
}

static otp_map_cmn_err_t bcm_otp_default_write_cb(bcm_otp_feat_info *feat_info)
{
    otp_map_cmn_err_t rc;
    if (feat_info->fld_name == NULL || !feat_info->fld_size)
        return OTP_MAP_CMN_ERR_INVAL;
    rc = bcm_otp_field_write(feat_info->fld_name, feat_info->is_sotp, 
        feat_info->cache, feat_info->size);
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

static otp_map_cmn_err_t bcm_otp_empty_read_cb(bcm_otp_feat_info *feat_info)
{
	printf("Otp field %s read not supported\n", feat_info->fld_name);
	return -1;
}

/* Read FLD ROE
   The function returns 16/32 bytes of AES EK followed by 16 bytes of AES IV.
*/
static otp_map_cmn_err_t otp_fld_roe_read_cb(bcm_otp_feat_info *feat_info)
{
    otp_map_cmn_err_t rc;

    rc = bcm_otp_field_read(k_FIELD_STR_FLD_EK_256, OTP_TYPE_SOTP, feat_info->cache, 32);
    if (rc)
        return rc;
    /* Check whether it is AES-128 or AES-256 */
    feat_info->size = 32;
    rc = bcm_otp_field_read(k_FIELD_STR_FLD_IV, OTP_TYPE_SOTP, 
        feat_info->cache + ((feat_info->size==32) ? 16 : 32)/sizeof(u32), 16);
    if (rc)
        return rc;
    return OTP_MAP_CMN_OK;
}

static otp_map_cmn_err_t otp_fld_roe_write_cb(bcm_otp_feat_info *feat_info)
{
    otp_map_cmn_err_t rc;

    if (feat_info->size != 32 && feat_info->size !=48)
    {
        OTP_ERR("%s: write size is invalid. Expected 32 or 48\n", __FUNCTION__);
        return OTP_MAP_CMN_ERR_INVAL;
    }
    rc = bcm_otp_field_write(
        (feat_info->size == 32) ? k_FIELD_STR_FLD_EK: k_FIELD_STR_FLD_EK_256, 
        OTP_TYPE_SOTP, feat_info->cache, feat_info->size - 16);
    rc = rc ? rc : bcm_otp_field_write(k_FIELD_STR_FLD_IV, OTP_TYPE_SOTP, 
        feat_info->cache + ((feat_info->size == 32) ? 16 : 32)/sizeof(u32), 16);
    return rc;
}

static otp_map_cmn_err_t otp_mfg_mode_read_cb(bcm_otp_feat_info *feat_info)
{
    otp_map_cmn_err_t rc;

    rc = bcm_otp_field_read(k_FIELD_STR_SECBOOT, OTP_TYPE_OTP, feat_info->cache, feat_info->fld_size);
    if (rc)
        return rc;
    feat_info->cache[0] &= OTP_MFG_SECURE_MASK;
    feat_info->size = feat_info->fld_size;
    return OTP_MAP_CMN_OK;
}

static otp_map_cmn_err_t otp_mfg_mode_write_cb(bcm_otp_feat_info *feat_info)
{
    otp_map_cmn_err_t rc;

    feat_info->cache[0] = OTP_MFG_SECURE_MASK;
    rc = bcm_otp_field_write(k_FIELD_STR_SECBOOT, OTP_TYPE_OTP, feat_info->cache, feat_info->fld_size);
    return rc;
}

static otp_map_cmn_err_t otp_fld_mode_read_cb(bcm_otp_feat_info *feat_info)
{
    otp_map_cmn_err_t rc;

    rc = bcm_otp_field_read(k_FIELD_STR_SECBOOT, OTP_TYPE_OTP, feat_info->cache, feat_info->fld_size);
    if (rc)
        return rc;
    feat_info->cache[0] &= OTP_FLD_SECURE_MASK;
    feat_info->size = feat_info->fld_size;
    return OTP_MAP_CMN_OK;
}

static otp_map_cmn_err_t otp_fld_mode_write_cb(bcm_otp_feat_info *feat_info)
{
    otp_map_cmn_err_t rc;

    feat_info->cache[0] = OTP_MFG_SECURE_MASK | OTP_FLD_SECURE_MASK;
    rc = bcm_otp_field_write(k_FIELD_STR_SECBOOT, OTP_TYPE_OTP, feat_info->cache, feat_info->fld_size);
    return rc;
}

//
// BCM OTP service API
//

int bcm_otp_init()
{
    int i;
    for (i = 0; i < sizeof(otp_feat_info)/sizeof(otp_feat_info[0]); i++)
    {
        if (!otp_feat_info[i].fld_size)
            return OTP_MAP_CMN_ERR_INVAL;
        otp_feat_info[i].cache = calloc(1, otp_feat_info[i].fld_size);
        if (otp_feat_info[i].cache == NULL)
            return OTP_MAP_CMN_ERR_FAIL;
    }
    memset((void *)SOTP_EXCHANGE_AREA_ADDR_START,
        0,
        SOTP_EXCHANGE_AREA_ADDR_END-SOTP_EXCHANGE_AREA_ADDR_START+1);
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
        return rc;
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

/* Temporary implementation of [S]OTP field status query.
   The long term implementation should be done by adding a new RPC
*/
otp_map_cmn_err_t bcm_otp_ctl(otp_map_t id,     
        otp_hw_cmn_ctl_cmd_t *cmd, u32 *res)
{
    otp_hw_ctl_data_t *ctl_data = (otp_hw_ctl_data_t *)cmd->data;
    otp_map_feat_t feat = ctl_data->addr;
    otp_map_cmn_err_t rc = OTP_MAP_CMN_ERR_UNSP;
    u32 *read_buf = NULL;
    u32 read_size = 0;

    *res = 0;
    if (cmd->ctl != OTP_HW_CMN_CTL_STATUS)
    {
        OTP_ERR("Command %d is not supported\n", cmd->ctl);
        return OTP_MAP_CMN_ERR_UNSP;
    }

    /* Try to read the field */
    rc = bcm_otp_read(feat, &read_buf, &read_size);
    if (rc || read_buf==NULL)
    {
        OTP_ERR("Feat %d is not supported or read failed\n", feat);
        return rc;
    }

    if (read_size <= sizeof(zero_buf) && memcmp(read_buf, zero_buf, read_size))
    {
        *res = OTP_HW_CMN_STATUS_ROW_DATA_VALID | OTP_HW_CMN_STATUS_ROW_RD_LOCKED;
    }
    return OTP_MAP_CMN_OK;
}

/* an SOTP helper*/
otp_map_cmn_err_t bcm_sotp_ctl_perm( otp_hw_cmn_ctl_t ctl, 
            u32 data, u32* res)
{
    otp_map_cmn_err_t rc = OTP_MAP_CMN_ERR_UNSP;
    if (ctl == OTP_HW_CMN_CTL_LOCK && data == OTP_HW_CMN_CTL_LOCK_ALL)
    {
        rc = _otp_rpc_commit();
        printf("OTP: all transactions committed, rc=%d\n", rc);
    }
    else {
        OTP_ERR("ctl=%d data=%u is not supported\n", ctl, data);
        rc = OTP_MAP_CMN_ERR_UNSP;
    }
    return rc;
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

int bcm_otp_get_mfg_process(u32* val)
{
    return bcm_otp_get(OTP_MAP_MFG_PROCESS, val);
}

int bcm_otp_get_mfg_foundry(u32* val)
{
    return bcm_otp_get(OTP_MAP_MFG_FOUNDRY, val);
}

int bcm_otp_get_mfg_substrate(u32* val)
{
    return bcm_otp_get(OTP_MAP_MFG_SUBSTRATE, val);
}
