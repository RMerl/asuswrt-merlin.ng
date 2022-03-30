/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2020 Broadcom Ltd.
 */
#include <common.h>
#include <wdt.h>
#include <fdtdec.h>
#include <asm/types.h>
#include <asm/byteorder.h>
#include <linux/errno.h>
#include <asm/unaligned.h>
#include <malloc.h>
#include "tpl_params.h"
#include "bcm_secure.h"
#include "bcm_otp.h"
#include <asm/arch/rng.h>
#include "u-boot/rsa.h"
#include "u-boot/rsa-mod-exp.h"
#include <u-boot/sha256.h>
#include <uboot_aes.h>

void bcm_sec_clean_secmem(bcm_sec_t* sec)
{
	if ( sec->state & SEC_STATE_SECURE) {
		memset((void*)bcm_secbt_args(), 0, sizeof(bcm_secbt_args_t));
	}
}

/* prevent zerofying by bss loop since we are called early in before relocation*/

static int sec_otp_ctrl(bcm_sec_t* sec, bcm_sec_ctrl_t ctrl, void* arg)
{
	switch(ctrl) {
        case SEC_CTRL_SOTP_LOCK_ALL: 
/*  
 * disable sensitive blocks for sec slave 
 * and non-sec master*/
		{	
			if (bcm_sotp_ctl_perm(OTP_HW_CMN_CTL_LOCK,
				OTP_HW_CMN_CTL_LOCK_ALL, NULL) == OTP_MAP_CMN_ERR_UNSP) {
				printf("WARNING: lock is not supported\n");
			}
		}
		break;
	default:
		break;
	}
	return 0;
}

static int sec_key_ctrl(bcm_sec_t *sec, bcm_sec_ctrl_t ctrl, void* arg) 
{
	switch(ctrl) {
		case SEC_CTRL_KEY_GET:	
			if ( sec->state & SEC_STATE_SECURE) {
				bcm_sec_btrm_key_info(sec);
			}
			break;
        	case SEC_CTRL_KEY_CLEAN_ALL:
			bcm_sec_clean_secmem(sec);
			bcm_sec_clean_keys(sec);
			break;
		default:
			break;
	}
	return 0;
}

void bcm_sec_cb_init(bcm_sec_t* sec)
{
	sec->cb[SEC_CTRL_ARG_KEY].cb = sec_key_ctrl;
	sec->cb[SEC_CTRL_ARG_SOTP].cb = sec_otp_ctrl;
}
