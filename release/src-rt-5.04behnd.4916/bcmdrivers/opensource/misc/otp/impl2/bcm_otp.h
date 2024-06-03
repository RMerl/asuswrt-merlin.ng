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
