/* SPDX-License-Identifier: GPL-2.0+
 *
 * Copyright 2020 Broadcom Ltd.
 */
#include <common.h>
#include <linux/ctype.h>
#include <linux/io.h>
#include "linux/printk.h"
#include <asm/arch/otp.h>
#include <asm/arch/misc.h>
#include <malloc.h>
#include "bcm_secure.h"
#include "bcm_otp.h"

/* global and static reserved in data section will not be nuked by bss clean loop*/
static bcm_otp_t *s_bcm_otp __attribute__((section(".data"))) = NULL;

__weak otp_hw_cmn_err_t otp_hw_cmn_init(otp_hw_cmn_t* dev)
{
	return OTP_HW_CMN_ERR_UNSP;
}
__weak otp_hw_cmn_err_t sotp_hw_cmn_init(otp_hw_cmn_t* dev)
{
	return OTP_HW_CMN_ERR_UNSP;
}

static otp_map_cmn_t* _get_map(otp_map_feat_t feat)
{
	if ((int)feat  <= OTP_MAP_INVALID || 
		(int) feat == SOTP_MAP_INVALID ||
		(int) feat >= OTP_MAP_MAX) {
		return NULL;
	}
	return  bcm_otp( feat < SOTP_MAP_INVALID? BCM_OTP_MAP : BCM_SOTP_MAP); 
}

int bcm_otp_init()
{
	otp_map_cmn_err_t rc = OTP_HW_CMN_ERR_FAIL;
	static otp_map_cmn_t s_map_obj[BCM_MAP_MAX] __attribute__((section(".data")));
	static bcm_otp_t s_otp_obj __attribute__((section(".data")));
	if (s_bcm_otp) {
		printf ("%s ERROR: BCM_OTP - not reentrant\n", __FUNCTION__);
		goto err;
	}
/*
 * malloc if desired here
 * */
	memset(s_map_obj, 0, sizeof(s_map_obj));
	memset(&s_otp_obj, 0, sizeof(s_otp_obj));

	rc = otp_map_cmn_init(&s_map_obj[BCM_OTP_MAP], otp_hw_cmn_init, NULL);
	if (rc) {
		printf ("%s ERROR: BCM_OTP unable to initialize OTP\n", __FUNCTION__);
		goto err;
	}

	s_otp_obj.map[BCM_OTP_MAP] = &s_map_obj[BCM_OTP_MAP];

	rc = otp_map_cmn_init(&s_map_obj[BCM_SOTP_MAP], 
				sotp_hw_cmn_init, &s_otp_obj.map[BCM_OTP_MAP]->dev);
	if (rc) {
		if (rc != OTP_MAP_CMN_ERR_UNSP) {
			printf ("%s ERROR: BCM_OTP unable to initialize SOTP\n", __FUNCTION__);
			goto err;
		}
		s_otp_obj.map[BCM_SOTP_MAP] = NULL;
	} else {
		s_otp_obj.map[BCM_SOTP_MAP] = &s_map_obj[BCM_SOTP_MAP];
	}

	s_bcm_otp = &s_otp_obj;
	rc = OTP_MAP_CMN_OK;
err:
	return rc;
}

otp_map_cmn_t* bcm_otp(otp_map_t id)
{
	return (id < BCM_OTP_MAP || id >= BCM_MAP_MAX || !s_bcm_otp)?  NULL : s_bcm_otp->map[id];
}

otp_map_cmn_err_t bcm_otp_read(otp_map_feat_t otp_feat, u32** data, u32* size)
{
	otp_map_cmn_t* obj = _get_map(otp_feat);
	if (!obj ) {
		return OTP_MAP_CMN_ERR_UNSP;
	}
	return obj->read(obj, otp_feat, data, size);
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
	otp_map_cmn_t* obj = _get_map(otp_feat);
	if (!obj ) {
		return OTP_MAP_CMN_ERR_UNSP;
	}
	return obj->write(obj, otp_feat, data, size);
}

otp_map_cmn_err_t bcm_otp_ctl(otp_map_t id, 	
		otp_hw_cmn_ctl_cmd_t *cmd, u32 *res)
{
	otp_map_cmn_t* obj = bcm_otp(id);
	if (!obj) {
		return OTP_MAP_CMN_ERR_FAIL;
	}
	return obj->ctl(obj, cmd, res);
}

/* an SOTP helper*/
otp_map_cmn_err_t bcm_sotp_ctl_perm( otp_hw_cmn_ctl_t ctl, 
			u32 data, u32* res)
{
	otp_map_cmn_err_t rc = OTP_MAP_CMN_ERR_INVAL;
	u32 stat = 0;
	otp_hw_ctl_data_t ctl_data = {0} ;
	otp_hw_cmn_ctl_cmd_t cmd = {
		.ctl = ctl,
		.data = (uintptr_t)&ctl_data, 
		.size = sizeof(ctl_data)
	};
	otp_map_cmn_t* obj = bcm_otp(BCM_SOTP_MAP);
	if (!obj) {
		return rc;
	}
	switch(cmd.ctl) {
   		case OTP_HW_CMN_CTL_STATUS: {
			if (!res) {
				goto err; 
			}
			ctl_data.status = (otp_hw_cmn_status_t) data; 
			break;
		}
   		case OTP_HW_CMN_CTL_LOCK:
   		case OTP_HW_CMN_CTL_UNLOCK:
			ctl_data.perm = (otp_hw_cmn_perm_t) data; 
			break;
		default:
			rc = OTP_MAP_CMN_ERR_UNSP;
			goto err;		
	}
	rc = obj->ctl(obj, &cmd, &stat);
	if (rc) {
		goto err;	
	}	
	if (res) {
		*res = stat;
	}
	
err:
	return rc;	
}

int bcm_otp_get_ldo_trim(unsigned int* val)
{
    return bcm_otp_get(OTP_MAP_LDO_TRIM, val);
}

int bcm_otp_get_cpu_clk(unsigned int* val)
{
    return bcm_otp_get(OTP_MAP_CPU_CLOCK_FREQ, val);
}

int bcm_otp_get_chipid(unsigned int* val)
{
    return bcm_otp_get(OTP_MAP_SEC_CHIPVAR, val);
}

int bcm_otp_get_nr_cpus(u32* val)
{
	return bcm_otp_get(OTP_MAP_CPU_CORE_CFG, val);
}




