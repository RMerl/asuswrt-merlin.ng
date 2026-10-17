/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2020 Broadcom Ltd.
 */

#include <linux/types.h>
#include <wdt.h>
#include <fdtdec.h>
#include <asm/types.h>
#include <asm/byteorder.h>
#include <linux/errno.h>
#include <asm/unaligned.h>
#include <malloc.h>
#include <asm/global_data.h>
#include "tpl_params.h"
#include "spl_env.h"
#include "bcm_secure.h"
#include "bcm_otp.h"
#include "u-boot/rsa.h"
#include "u-boot/rsa-mod-exp.h"
#include <u-boot/sha256.h>
#include <uboot_aes.h>
#include <watchdog.h>

DECLARE_GLOBAL_DATA_PTR;

#define NODE_NAME_LEN	128
int bcm_sec_boot_state(bcm_sec_state_t* sec_state)
{
	void* fdt = (void*)gd->fdt_blob;
	int rc = 0, len = 0 ;
	u32* state = NULL;
	int off = fdt_path_offset(fdt, "/chosen");
	if (off < 0) {
		printf("ERROR: Can't find /chosen node in boot DTB!\n");
	}
	state = (u32*)(fdt_getprop(fdt, off, "brom_sec_state", &len));

	if (!state || (int)len < sizeof(u32)) {
		rc = -1;
		*sec_state = SEC_STATE_UNSEC;
		goto err;
	}
	*sec_state = fdt32_to_cpu(*state);

err:
	return rc; 
}

static int sec_key_ctrl(bcm_sec_t *sec, bcm_sec_ctrl_t ctrl, void* arg)
{
	int rc = 0;
	switch(ctrl) {
	case SEC_CTRL_KEY_GET:
		{
			if ( !(sec->state & SEC_STATE_SECURE)) {
				u8* env_key = bcm_util_env_var2bin("brcm_pub_testkey", RSA2048_BYTES);
				/* We've got test key; try to authenticate with it
 				* Only works in non-secure mode
 				* */
				if (!env_key) {
					debug("No Key in environment \n");
				} else {
					memcpy(sec->key.rsa_pub, env_key, RSA2048_BYTES);
					sec->key.pub = sec->key.rsa_pub;
					free(env_key);
				} 
			}
		}
		break;
        case SEC_CTRL_KEY_CHAIN_RSA:
		if(sec->state & SEC_STATE_SECURE) {
			int  len = 0;	
			u8 * k;
			int off;
			off = fdt_path_offset(arg, "/trust/brcm_pub_key");
			if (off < 0) {
				printf("ERROR: Can't find /trust/brcm_pub_key node in boot DTB!\n");
			}
			k = (u8*)(fdt_getprop(arg, off, "value", &len));
			if (!k || (len != RSA2048_BYTES && len != RSA3072_BYTES && 
					len != RSA4096_BYTES)) {
				rc = -1;
				printf("ERROR: length  %d \n", len);
				break ;
			} else {
				memcpy(sec->key.rsa_pub, k, len);
				sec->key.pub = sec->key.rsa_pub;
				sec->key.pub_len = len ;
			}

		} 
		break;
        case SEC_CTRL_KEY_CLEAN_ALL:
		bcm_sec_clean_keys(sec);
		break;
	default:
		break;
	}
	return rc;
}

void bcm_sec_cb_init(bcm_sec_t* sec)
{
	sec->cb[SEC_CTRL_ARG_KEY].cb = sec_key_ctrl;
}
