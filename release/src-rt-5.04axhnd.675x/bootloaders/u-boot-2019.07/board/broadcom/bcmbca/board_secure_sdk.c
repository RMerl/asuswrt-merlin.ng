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
#include "spl_env.h"
#include "bcm_secure.h"
#include "bcm_otp.h"
#include "u-boot/rsa.h"
#include "u-boot/rsa-mod-exp.h"
#include <u-boot/sha256.h>
#include <uboot_aes.h>
#include <watchdog.h>

#define NODE_NAME_LEN	128

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
			k = (char*)(fdt_getprop(arg, off, "value", &len));
			if (!k || len != RSA2048_BYTES) {
				rc = -1;
				printf("ERROR: length  %d \n", len);
				break ;
			} else {
				memcpy(sec->key.rsa_pub, k, RSA2048_BYTES);
				sec->key.pub = sec->key.rsa_pub;
			}

		} 
		break;
       	case SEC_CTRL_KEY_CHAIN_AES:
		if(sec->state & SEC_STATE_SECURE) {
			int  len = 0, alloc_len = 0;
			char *_keys_prop[2] = {FIT_AES1, FIT_AES2};
			bcm_sec_key_arg_t *aes_arg;
			int off;
			u8* aes1,aes2;
			char key_node[NODE_NAME_LEN];

			/* Get aes1 */
			snprintf(key_node,NODE_NAME_LEN, "/trust/%s", _keys_prop[0]);
			off = fdt_path_offset(arg, key_node);
			if (off < 0) {
				printf("INFO: Can't find %s node in boot DTB!\n", key_node);
				break;
			}
			aes1 = (char*)(fdt_getprop(arg, off, "value", &len));
			if (aes1 && len != BCM_SECBT_AES_CBC128_EK_LEN*2) {
				break;
			}

			/* Get aes2 */
			snprintf(key_node,NODE_NAME_LEN, "/trust/%s", _keys_prop[1]);
			off = fdt_path_offset(arg, key_node);
			if (off < 0) {
				printf("INFO: Can't find %s node in boot DTB!\n", key_node);
				break;
			}
			aes2 = (char*)(fdt_getprop(arg, off, "value", &len));
			if (aes2 && len != BCM_SECBT_AES_CBC128_EK_LEN*2) {
				break;
			}

			alloc_len = sizeof(bcm_sec_key_arg_t) + 
				(aes1 && aes2) ? sizeof(bcm_sec_key_aes_arg_t)*2 : sizeof(bcm_sec_key_aes_arg_t);
			aes_arg = malloc(alloc_len);
			if (!aes_arg) {
		 		break;
			}
			memset(aes_arg, 0, alloc_len);
			if (aes1) {
				//printf("Got %s 0x%x\n" , _keys_prop[0], *(u32*)aes1);
				strcpy(aes_arg->aes[aes_arg->len].id,_keys_prop[0]);
				memcpy(aes_arg->aes[aes_arg->len].key, aes1, BCM_SECBT_AES_CBC128_EK_LEN*2);
				aes_arg->len++;
			}
			if (aes2) {
				strcpy(aes_arg->aes[aes_arg->len].id, _keys_prop[1]);
				memcpy(aes_arg->aes[aes_arg->len].key, aes2, BCM_SECBT_AES_CBC128_EK_LEN*2);
				//printf("Got %s 0x%x\n" , _keys_prop[1], *(u32*)aes2);	
				aes_arg->len++;
			}
			sec->key.ch_ek = aes_arg;
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
