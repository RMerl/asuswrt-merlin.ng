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

#ifdef CONFIG_SMC_BASED
/* short hand for reading/writing a part of OTP region */
otp_map_cmn_err_t bcm_otp_read_slice(otp_map_feat_t, u8 *buffer, u32 offset, u32 size);
otp_map_cmn_err_t bcm_otp_write_slice(otp_map_feat_t, u8 *buffer, u32 offset, u32 size);

/* Fuse all staged updates into OTO and SOTP */
otp_map_cmn_err_t bcm_otp_commit(void);
#endif /* #ifdef CONFIG_SMC_BASED */

#define bcm_otp_set(_feat, data) bcm_otp_write(_feat, data, sizeof(u32))

int bcm_otp_get_cpu_clk(unsigned int* val);
int bcm_otp_get_chipid(unsigned int* val);
int bcm_otp_get_nr_cpus(u32* val);

int bcm_otp_get_ldo_trim(unsigned int * val);

int bcm_otp_get_mfg_process(u32* val);
int bcm_otp_get_mfg_substrate(u32* val);
int bcm_otp_get_mfg_foundry(u32* val);
#endif
