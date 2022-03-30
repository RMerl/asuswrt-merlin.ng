/* SPDX-License-Identifier: GPL-2.0+
 *
 * Copyright 2020 Broadcom Ltd.
 */
/*
 * 
 */

#ifndef __BCM_OTP_H__
#define __BCM_OTP_H__

#include "otp_map.h"
#include "otp_map_cmn.h"

typedef enum {
	BCM_OTP_MAP,
	BCM_SOTP_MAP,
	BCM_MAP_MAX,
} otp_map_t;

typedef struct bcm_otp {
	otp_map_cmn_t* map[BCM_MAP_MAX]; 
} bcm_otp_t;


otp_map_cmn_t*  bcm_otp(otp_map_t);

otp_map_cmn_err_t bcm_otp_read(otp_map_feat_t, u32**, u32*);
otp_map_cmn_err_t bcm_otp_write(otp_map_feat_t, const u32*, u32);
otp_map_cmn_err_t bcm_otp_ctl(otp_map_t id, otp_hw_cmn_ctl_cmd_t *cmd, u32 *res);
int bcm_otp_init(void);

/* helper functions */
otp_map_cmn_err_t bcm_sotp_ctl_perm( otp_hw_cmn_ctl_t ctl, u32 data, u32* res);

/* short hand for reading/writing just a u32 val*/
otp_map_cmn_err_t bcm_otp_get(otp_map_feat_t, u32*);

#define bcm_otp_set(_feat, data) bcm_otp_write(_feat, data, sizeof(u32)

int bcm_otp_get_cpu_clk(unsigned int* val);
int bcm_otp_get_chipid(unsigned int* val);
int bcm_otp_get_nr_cpus(u32* val);
#endif
