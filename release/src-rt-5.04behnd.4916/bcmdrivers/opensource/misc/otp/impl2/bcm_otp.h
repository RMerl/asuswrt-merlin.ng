/* 
   <:copyright-BRCM:2011:DUAL/GPL:standard
   
      Copyright (c) 2011 Broadcom 
      All Rights Reserved
   
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License, version 2, as published by
   the Free Software Foundation (the "GPL").
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   
   A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
   writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
   
   :>
 */

#ifndef __BCM_OTP_H__
#define __BCM_OTP_H__

#include "otp_map.h"

typedef enum {
	BCM_OTP_MAP,
	BCM_SOTP_MAP,
	BCM_MAP_MAX,
} otp_map_t;

otp_map_cmn_err_t bcm_otp_read(otp_map_feat_t, unsigned int**, unsigned int*);
otp_map_cmn_err_t bcm_otp_write(otp_map_feat_t, const unsigned int*, unsigned int);
int bcm_otp_init(void);

/* short hand for reading/writing just a unsigned int val*/
otp_map_cmn_err_t bcm_otp_get(otp_map_feat_t, unsigned int*);

#ifdef CONFIG_SMC_BASED
/* short hand for reading/writing a part of OTP region */
otp_map_cmn_err_t bcm_otp_read_slice(otp_map_feat_t, u8 *buffer, unsigned int offset, unsigned int size);
otp_map_cmn_err_t bcm_otp_write_slice(otp_map_feat_t, u8 *buffer, unsigned int offset, unsigned int size);

/* Fuse all staged updates into OTO and SOTP */
otp_map_cmn_err_t bcm_otp_commit(void);
#endif /* #ifdef CONFIG_SMC_BASED */

#define bcm_otp_set(_feat, data) bcm_otp_write(_feat, data, sizeof(unsigned int))

int bcm_otp_get_cpu_clk(unsigned int* val);
int bcm_otp_get_chipid(unsigned int* val);
int bcm_otp_get_nr_cpus(unsigned int* val);

int bcm_otp_get_ldo_trim(unsigned int * val);
u32 otp_feat_info_get_size(otp_map_feat_t otp_feat);
#endif
