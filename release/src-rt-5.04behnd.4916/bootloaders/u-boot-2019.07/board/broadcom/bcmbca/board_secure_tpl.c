/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2020 Broadcom Ltd.
 */
#include <common.h>
#include <nand.h>
#include <wdt.h>
#include <fdtdec.h>
#include <asm/types.h>
#include <asm/byteorder.h>
#include <linux/errno.h>
#include <asm/unaligned.h>
#include <malloc.h>
#include <spl.h>
#include "tpl_params.h"
#include "spl_env.h"
#include "bcm_bootstate.h"
#include "bcm_secure.h"
#include "bcm_otp.h"
#include "bcm_rng.h"
#include "u-boot/rsa.h"
#include "u-boot/rsa-mod-exp.h"
#include <u-boot/sha256.h>
#include <uboot_aes.h>
#include <watchdog.h>
#include <mmc.h>
#include <ext4fs.h>
#include "mini-gmp/mini-gmp.h"
#include "mini-gmp/mini-mpq.h"
#include "tpl_common.h"

/* This corresponds to the min_tpl_compatibility parameter in the security policy */
#define THIS_TPL_COMPAT_VERSION		0x3

#define NODE_NAME_LEN			128
#define BCM_SEC_MAX_ENCODED_KEYS	10
#define BCM_SEC_MAX_EXPORTED_ITEMS	10
#define DELG_SEC_FIT_NODE_PATH 		"/security"
#define DELG_SEC_FIT_NODE_SIG		"signature"
#define DELG_SEC_POL_PATH		"/security_policy"
#define DELG_SEC_POL_COMPAT_PATH	"/security_policy/compatible"
#define DELG_SEC_POL_DELGID		"delegate_id"
#define DELG_SEC_POL_MINTPLV		"min_tpl_compatibility"
#define DELG_SEC_POL_AES_KEY_PATH	"/security_policy/key-aes"
#define DELG_SEC_POL_AES_KEY_ENC_ALGO	"algo"
#define DELG_SEC_POL_AES_KEY_DATA	"data"
#define DELG_SEC_POL_ANTI_ROLLBACK_PATH	"/security_policy/anti-rollback"
#define DELG_SEC_POL_SEC_RESTRICT_PATH	"/security_policy/sec_restriction"
#define DELG_SEC_POL_ENC_KEYS_PATH	"/security_policy/encoded_keys"
#define DELG_SEC_POL_STATUS		"status"
#define DELG_SEC_POL_STATUS_DISABLED	"disabled"
#define DELG_SEC_POL_NEW_ANTIROLLBCK	"new_antirollback"
#define DELG_SEC_POL_ANTIROLLBCK_LIM	"antirollback_limit"
#define DELG_SEC_POL_HW_STATE_PATH	"/security_policy/sec_restriction/hw_state"
#define DELG_SEC_POL_SW_RESTRICT_PATH	"/security_policy/sec_restriction/sw_restriction"
#define DELG_SEC_POL_REQ_IMGS		"required_images"
#define DELG_REQ_IMG_NODE_NAME		"node_name"
#define DELG_REQ_IMG_SIZE		"size"
#define DELG_REQ_IMG_LOAD_ADDR		"load_addr"
#define DELG_REQ_IMG_START_ADDR		"start_addr"
#define DELG_REQ_IMG_SHA256		"sha256"
#define DELG_SEC_POL_DBG_GRPS		"debug_groups"
#define DELG_DBG_GRP_NAME		"group_name"
#define DELG_DBG_GRP_CERT		"group_cert"
#define DELG_DBG_GRP_KEY		"group_pubkey"
#define DELG_DBG_GRP_SERNUM		"serial_no"
#define DELG_DBG_CERT_NODE_NAME		"/debug_group"
#define DELG_HW_STATE_PATH		"/trust/hw_state"
#define DELG_ANTI_ROLLBACK_PATH 	"/trust/anti-rollback"
#define DELG_OS_EXPORT_FLAG		"os_export"
#define DELG_EXPORT_PERM		"permission"
#define DELG_ENC_KEYS_PATH		"/trust/encoded_keys"
#define DELG_ENC_KEYS_NAME		"key_name"
#define DELG_ENC_KEYS_SIZE		"size"
#define DELG_ENC_KEYS_DATA		DELG_SEC_POL_AES_KEY_DATA
#define DELG_ENC_KEYS_LDADDR		"load_addr"
#define DELG_ENC_KEYS_ALGO		DELG_SEC_POL_AES_KEY_ENC_ALGO
#define DELG_EXPORT_PERMSEC		"secure"
#define DELG_EXPORT_PERMNONSEC		"nonsecure"
#define DELG_ENC_KEYS_EXP_PREFIX	"key_"
#define DELG_TRUST_NODE_PATH		"/trust"
#define DELG_TRUST_NODE			"trust"
#define DELG_SEC_POL_SEC_EXPORT_PATH	"/security_policy/sec_restriction/sec_exports"
#define DELG_SEC_ALLOWED_EXPORTS	"allowed_exports"
#define DELG_SEC_EXPORT_PATH		"/trust/sec_exports"
#define DELG_EXP_ITEM_NAME		"item_name"
#define DELG_EXP_ITEM_ID		"item_sec_id"
#define DELG_EXP_ITEM_LENGTH		"length"
#define DELG_EXP_ITEM_SALT		"salt"
#define DELG_EXP_ITEM_ALGO		DELG_SEC_POL_AES_KEY_ENC_ALGO

static int sec_add_export_item_fdt( u8 * fdt, bcm_sec_export_item_t* item );
static int bcm_sec_set_sotp_hw_state(u8* fit, int node, bcm_sec_cb_arg_t* sotp_st_arg);
static int bcm_sec_set_rng_hw_state(u8* fit, int node, bcm_sec_cb_arg_t* rng_st_arg);

typedef struct sec_exp_item_otp_map {
	char item_id[NODE_NAME_LEN];
	otp_map_feat_t otp_feat;
} sec_exp_item_otp_map_t;

sec_exp_item_otp_map_t exp_item_otp_map[] = { 
	{ "SEC_ITEM_KEY_DEV_SPECIFIC"	, SOTP_MAP_KEY_DEV_SPECIFIC},
	{ "SEC_ITEM_SER_NUM"		, SOTP_MAP_SER_NUM},
	{ ""				, OTP_MAP_MAX}
};

static char default_allowed_exports[] = "SEC_ITEM_KEY_DEV_SPECIFIC,SEC_ITEM_SER_NUM";
static bcm_sec_export_item_t bcm_sec_exported_items[BCM_SEC_MAX_EXPORTED_ITEMS];
static bcm_sec_enc_key_arg_t bcm_sec_encoded_keys[BCM_SEC_MAX_ENCODED_KEYS];
static int tee_loaded = 0;
static int armtf_loaded = 0;
static u8* fit_load_addr = NULL;

void bcm_sec_clean_secmem(bcm_sec_t* sec)
{
	if ( sec->state & SEC_STATE_SECURE) {
		memset((void*)bcm_secbt_args(), 0, sizeof(bcm_secbt_args_t));
	}
}

static int sec_otp_ctrl(bcm_sec_t* sec, bcm_sec_ctrl_t ctrl, void* arg)
{
	switch(ctrl) {
        case SEC_CTRL_SOTP_LOCK_ALL: 
		/* Lock access to all otp keycontent including register access to the controller for both secure/non-secure masters*/
		{
			if (bcm_sotp_ctl_perm(OTP_HW_CMN_CTL_LOCK,
				OTP_HW_CMN_CTL_LOCK_ALL, NULL) == OTP_MAP_CMN_ERR_UNSP) {
				printf("WARNING: lock is not supported\n");
			}
		}
		break;
	
        case SEC_CTRL_SOTP_UNLOCK_SOTP_UNSEC_PROV: {
		/*  
 		* unlock non secure masters in provisioning mode --> readlock all nonzero rows
		*/
			if (bcm_sotp_ctl_perm(OTP_HW_CMN_CTL_UNLOCK,
				OTP_HW_CMN_CTL_LOCK_NS_PROV, NULL) == OTP_MAP_CMN_ERR_UNSP) {
				printf("WARNING: unlock is not supported\n");
			}
			break;
		}
		break;

        case SEC_CTRL_SOTP_UNLOCK_SOTP_UNSEC: {
		/*  
 		* unlock non secure masters
		*/
			if (bcm_sotp_ctl_perm(OTP_HW_CMN_CTL_UNLOCK,
				OTP_HW_CMN_CTL_LOCK_NS, NULL) == OTP_MAP_CMN_ERR_UNSP) {
				printf("WARNING: unlock is not supported\n");
			}
			break;
		}
		break;
        case SEC_CTRL_SOTP_UNLOCK_SOTP_SEC: {
 		/* unlock secure masters*/
			if (bcm_sotp_ctl_perm(OTP_HW_CMN_CTL_UNLOCK,
				OTP_HW_CMN_CTL_LOCK_S, NULL) == OTP_MAP_CMN_ERR_UNSP) {
				printf("WARNING: unlock is not supported\n");
			}
		}
		break;
	default:
		break;
	}
	return 0;
}

#ifndef CONFIG_SMC_BASED
static int sec_rng_ctrl(bcm_sec_t* sec, bcm_sec_ctrl_t ctrl, void* arg)
{
	switch(ctrl) {
	
        case SEC_CTRL_RNG_LOCK_ALL: 
		rng_pac_lock(RNG_PERM_ALL_ACCESS_DISABLE);
		break;
	
        case SEC_CTRL_RNG_UNLOCK_RNG_UNSEC: 
		rng_pac_lock(RNG_PERM_NSEC_ACCESS_ENABLE);
		break;
        case SEC_CTRL_RNG_UNLOCK_RNG_SEC: 
		rng_pac_lock(RNG_PERM_SEC_ACCESS_ENABLE);
		break;
	default:
		break;
	}
	return 0;
}
#endif

static int sec_add_fdt_node( u8 * fdt, char * path, char * nodename)
{
	int node = -1;
	printf("INFO: Creating %s/%s\n", path, nodename);
        /*
         * Parameters: Node path, new node to be appended to the path.
         */
        node = fdt_path_offset (fdt, path);
        if (node < 0) {
                /*
                 * Not found or something else bad happened.
                 */
                printf ("libfdt fdt_path_offset() returned %s\n",
                        fdt_strerror(node));
                return -1;
        }
        node = fdt_add_subnode(fdt, node, nodename);
        if (node < 0) {
                printf ("libfdt fdt_add_subnode(): %s\n",
                        fdt_strerror(node));
                return -1;
        }
	return node;
}

static int sec_get_export_item( char * item_id, u8 ** data, u32 * size )
{
	int i = 0;
	int rc = -1;

	while( exp_item_otp_map[i].otp_feat != OTP_MAP_MAX ) {
		if( (strncasecmp(exp_item_otp_map[i].item_id, item_id, NODE_NAME_LEN) == 0)) 
		{
			printf("INFO: Found matching item for %s = %d\n", item_id, 
				exp_item_otp_map[i].otp_feat);
			break;
		}
		i++;
	}
	if( exp_item_otp_map[i].otp_feat != OTP_MAP_MAX )
		rc = bcm_otp_read(exp_item_otp_map[i].otp_feat, (u32**)data, size);

	/* If slot was empty */
	if( rc == OTP_HW_CMN_ERR_KEY_EMPTY )
		rc = OTP_HW_CMN_OK;

	return rc;
}

static int sec_salt_hash_item( u8 * fdt, u8 * item_src, int size_src, 
		u8* item_dest, bcm_sec_export_item_t* item)
{
	char * temp_buff;
	char * buf_ptr;
	char * dst_ptr;
	char salt_key_node[NODE_NAME_LEN];
	int salt_key_offset = -1;
	char * salt_key = NULL;
	int salt_int = 0;
	char * salt_bytes_ptr = NULL;
	int salt_bytes_size = 0;
	int hash_data_size = 0;
	int req_hash_size = item->len;

	/* verify salt */
	if( item->salt ) {	
		/* Determine if salt is a 32-bit number or a encoded key */
		if ( item->salt_len < sizeof(int) ) {
			/* Invalid Salt */
			printf("ERROR: Invalid integer Salt!, Size < 4bytes \n");
			return -1;
		} else if( item->salt_len == sizeof(int) ) {
			/* 32-bit integer Salt */
			salt_int =  be32_to_cpu(*(int*)(item->salt));
			salt_bytes_ptr = (char*)&salt_int;
			salt_bytes_size = sizeof(int);
			debug("INFO: Valid integer salt 0x%08x found in dtb\n", (uint32_t)salt_int);
		} else if(strncmp((const char*)(item->salt), DELG_ENC_KEYS_EXP_PREFIX,
		   		strlen(DELG_ENC_KEYS_EXP_PREFIX)) == 0) {
			/* Salt points to encoded key */
			snprintf(salt_key_node,NODE_NAME_LEN,"%s/%s", DELG_TRUST_NODE_PATH, item->salt);
			salt_key_offset = fdt_path_offset(fdt, salt_key_node);
			if( salt_key_offset < 0 ) 
				printf("INFO: Salt key %s not found in dtb!\n", salt_key_node);
			else
				salt_key = (char*)fdt_getprop(fdt, salt_key_offset, "value", &salt_bytes_size);

			if( !salt_bytes_size ) {
				if( salt_key_offset >= 0 ) {
					printf("ERROR: Invalid Salt key %s size in dtb!\n", 
						salt_key_node);
				}
				return -1;
			}
			debug("INFO: Valid salt_key %s found in dtb\n", salt_key_node);
			salt_bytes_ptr = salt_key;
		} else {
			/* Invalid Salt */
			printf("ERROR: Invalid integer Salt!, Size > 4bytes \n");
			return -1;
		}
	}

	/* Calculate amount of data we are hashing */
	hash_data_size = size_src+salt_bytes_size;

	/* Allocate temp mem of src size + size of salt */
	temp_buff = malloc(hash_data_size);
	if (!temp_buff) {
		printf("ERROR: Cannot allocate memory hashing tempbuff\n");
		return -1;
	} 

	/* Init buffer pointers */
	buf_ptr = (char*)temp_buff;
	dst_ptr = (char*)item_dest;

	if( (req_hash_size % SHA256_SUM_LEN) || req_hash_size > 2*SHA256_SUM_LEN ) {
		printf("ERROR: Unsupported final item size %d for hashing!\n", req_hash_size);
		return -1;
	}
	
	/* First pass, get first 256bits of obfuscated item data */
	if( salt_bytes_ptr ) {
		memcpy(buf_ptr, salt_bytes_ptr, salt_bytes_size);
		buf_ptr += salt_bytes_size;
	}
	memcpy(buf_ptr, item_src, size_src);

	bcm_sec_digest( (u8*)temp_buff, hash_data_size, (u8*)dst_ptr, "sha256");
	dst_ptr += SHA256_SUM_LEN;

	/* 2nd pass, get 2nd 256bits of obfuscated item data, reverse order of salting */
	if( req_hash_size > SHA256_SUM_LEN ) {
		buf_ptr = temp_buff;
		memcpy(buf_ptr, item_src, size_src);
		buf_ptr += size_src;
		if( salt_bytes_ptr ) {
			memcpy(buf_ptr, salt_bytes_ptr, salt_bytes_size);
		}
		bcm_sec_digest((u8*)temp_buff, hash_data_size, (u8*)dst_ptr, "sha256");
	}
	free(temp_buff);
	return 0;
}

static int sec_add_export_item_fdt( u8 * fdt, bcm_sec_export_item_t* item )
{
	int node = -1;
	char node_name[NODE_NAME_LEN];
	int ret = -1;
	u8 * temp_buff = NULL;
	u8 * pdata = NULL;
	u32 data_size = 0;

	/* Default permission is non-secure for backwards compatibility */
	char * perm = DELG_EXPORT_PERMNONSEC;

	/* Get the exported item */
	if( !item->value ) {
		/* Get items */
		if( !item->id || !item->len ) {
			printf("INFO: Skipping exported item %s %s %d\n", item->name, item->id, item->len);
			ret = 0;
			goto err;
		}

		/* Get the item */
		ret = sec_get_export_item( item->id, &pdata, &data_size );
		if( ret ) {
			printf("INFO: Cannot retrieve export item %s! Skipping!\n", item->id);
			ret = 0;
			goto err;
		}
		
		/* If item is empty, skip */
		if( !pdata || !data_size ) {
			printf("INFO: Src data for item %s is not provisioned! Skipping!\n", item->name);
			ret = 0;
			goto err;
		}

		/* Allocate memory for value */
		temp_buff = malloc(item->len);
		if (!temp_buff) {
			printf("ERROR: Cannot allocate memory for export item\n");
			ret = -1;
			goto err;
		} 
		memset(temp_buff,0,item->len);

		/* Salt and/or hash the item */
		if( item->algo ) {
			ret = sec_salt_hash_item( fdt, (u8*) pdata, data_size, temp_buff, item);
			if( ret ) {
				printf("INFO: Cannot salt/hash for item %s! Skipping!\n", item->name);
				ret = 0;
				goto err;
			}
		} else {
			memcpy(temp_buff, pdata, (data_size>item->len?item->len:data_size));
		}

		/* Set value */
		item->value = temp_buff;
	}

	/* Create /trust node if it doesnt exits */	
	node = fdt_path_offset (fdt, DELG_TRUST_NODE_PATH);
	if(node < 0) {
		node = sec_add_fdt_node( fdt, "/", DELG_TRUST_NODE);
		if( node < 0) {
			printf("ERROR: Could not create %s node!\n", DELG_TRUST_NODE_PATH);
			ret = -1;
			goto err;
		}
	} 

	/* Add item node */
	snprintf(node_name,NODE_NAME_LEN,item->name);
	node = sec_add_fdt_node( fdt, DELG_TRUST_NODE_PATH, node_name);
	if( node < 0 ) {
		printf("ERROR: Could not add %s node to %s!\n",
			node_name, DELG_TRUST_NODE_PATH);
		ret = -1;
		goto err;
	}
	printf("INFO: Adding exported item node %s to dtb, size:%d\n", node_name, item->len);

	/* Add the item value */
	ret = fdt_setprop(fdt, node, "value", item->value, item->len);
	if( ret ) {
		printf("ERROR: Could net set value for %s/%s\n",DELG_TRUST_NODE_PATH,node_name);
		goto err;
	}
	
	/* Set the export flag */
	if( !ret && item->exp_flag ) {
		ret = fdt_setprop(fdt, node, "export", "yes", strlen("yes")+1); 
		if( ret ) {
			printf("ERROR: Could not set %s/%s/export\n",DELG_TRUST_NODE_PATH,node_name);
			goto err;
		}
	}

	/* Set the permission flag */
	if( !ret && item->perm 
		&& (strcasecmp(item->perm, DELG_EXPORT_PERMSEC) == 0)  ) {
		perm = item->perm;
	} 
	ret = fdt_setprop(fdt, node, "permission", perm, strlen(perm)+1); 
	if( ret ) {
		printf("ERROR: Could not set %s/%s/permission\n",DELG_TRUST_NODE_PATH,node_name);
		goto err;
	}

err:
	if( temp_buff )
		free(temp_buff);
	
	return ret;
}

static int sec_add_decoded_key_fdt( u8 * fdt, char* key_name, u8 * key_val, int key_len, char* perm, u8 exp_flag)
{
	int node = -1;
	char key_node_name[NODE_NAME_LEN];
	int ret = -1;

	node = fdt_path_offset (fdt, DELG_TRUST_NODE_PATH);
	/* Create /trust node if it doesnt exits */	
	if(node < 0) {
		node = sec_add_fdt_node( fdt, "/", DELG_TRUST_NODE);
		if( node < 0) {
			printf("ERROR: Could not create %s node!\n", DELG_TRUST_NODE_PATH);
			return -1;
		}
	} 

	/* Add key node */
	snprintf(key_node_name,NODE_NAME_LEN,"%s%s", DELG_ENC_KEYS_EXP_PREFIX, key_name);
	node = sec_add_fdt_node( fdt, DELG_TRUST_NODE_PATH, key_node_name);
	if( node < 0 ) {
		printf("ERROR: Could not add %s node to %s!\n",
			key_node_name, DELG_TRUST_NODE_PATH);
		return -1;
	}

	/* Set decoded key */
	printf("INFO: Adding exported decoded key node %s to dtb, size:%d\n",key_node_name, key_len);
	ret = fdt_setprop(fdt, node, "value", key_val, key_len); 
	if( ret )
		printf("ERROR: Could net set value for %s/%s\n",DELG_TRUST_NODE_PATH,key_node_name);

	/* Set permission flag */
	if( !ret ) {
		ret = fdt_setprop(fdt, node, "permission", perm, strlen(perm)+1);
		if( ret )
			printf("ERROR: Could net set permission for %s/%s\n",DELG_TRUST_NODE_PATH,key_node_name);
	}

	/* Set the export flag */
	if( !ret && exp_flag ) {
		ret = fdt_setprop(fdt, node, "export", "yes", strlen("yes")+1); 
		if( ret ) {
			printf("ERROR: Could not set %s/%s/export\n",DELG_TRUST_NODE_PATH,key_node_name);
		}
	}

	return ret;
}

static int sec_key_ctrl(bcm_sec_t *sec, bcm_sec_ctrl_t ctrl, void * arg)
{
	int rc = 0;
	u8* dec_aes_key = NULL;
	u8* key_data = NULL;
	switch(ctrl) {
	case SEC_CTRL_KEY_GET:
		{
			if ( sec->state & SEC_STATE_SECURE) {
				bcm_sec_btrm_key_info(sec);
			} else {
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
		if (sec->state&SEC_STATE_SECURE) {
			bcm_sec_export_item_t item;
			u32 lvl = 0;
			item.salt = NULL;
			item.algo = NULL;
			item.exp_flag = 1;

			bcm_sec_get_antirollback_lvl(&lvl);
			lvl = cpu_to_be32(lvl);
			item.name = "antirollback_lvl";
			item.len = sizeof(u32);
			item.value = (u8*)&lvl;
			item.perm = DELG_EXPORT_PERMNONSEC;
			rc = sec_add_export_item_fdt(arg, &item);

			if( rc == 0 ) {
				item.name = "brcm_pub_key";
				item.len = sec->key.pub_len;
				item.value = sec->key.pub;
				item.perm = DELG_EXPORT_PERMNONSEC;
				rc = sec_add_export_item_fdt(arg, &item);
				if (!sec->key.pub || rc ) {
					rc = -1;
					printf("ERROR: unable to chain pub_key\n");
				}
			}
		} 
		break;
	case SEC_CTRL_KEY_EXPORT_ITEM: 
		if (sec->state&SEC_STATE_SECURE) {
			int i;
			bcm_sec_key_arg_t* sec_arg = arg;
			for (i = 0; i<sec_arg->len; i++) { 
				sec_add_export_item_fdt(sec_arg->arg, 
					&sec_arg->item[i]);
			}
		}
		break;

       	case SEC_CTRL_KEY_CHAIN_ENCKEY:
		if (sec->state&SEC_STATE_SECURE) {
			int i;
			u8 * iv;
			bcm_sec_key_arg_t* sec_arg = arg;
			for (i = 0; i<sec_arg->len; i++) { 
				/* Get decryption key */
				if( sec_arg->enc_key[i].dec_key == DEC_KEY_TYPE_AES_ROOT )
					bcm_sec_get_root_aes_key(&dec_aes_key);
				else
					bcm_sec_get_active_aes_key(&dec_aes_key);

				iv = dec_aes_key + AES128_KEY_LENGTH;
				key_data = malloc(sec_arg->enc_key[i].size_enc > sec_arg->enc_key[i].size ? sec_arg->enc_key[i].size_enc : sec_arg->enc_key[i].size);
				if (!key_data) {
					printf("ERROR: Cannot allocate memory for decoded key size %d (enc:%d)!\n", sec_arg->enc_key[i].size_enc, sec_arg->enc_key[i].size_enc);
					return -1;
				}
				memcpy(key_data, sec_arg->enc_key[i].data, sec_arg->enc_key[i].size_enc);

				/* Decrypt encoded key */
				bcm_sec_aes_cbc128(dec_aes_key, iv, key_data, sec_arg->enc_key[i].size_enc,0);

#ifdef BCM_SEC_EXPOSE_SECRET_KEYS				
				/* ### WARNING: ENABLING THIS DEBUG WILL EXPOSE DECRYPTED ENCODED KEYS ### */
				{
					int j=0;
					printf("%s: ciphertxt:", sec_arg->enc_key[i].name);
					for( j=0; j<sec_arg->enc_key[i].size_enc; j+=1)
					{
						printf("%02x", *((u8*)sec_arg->enc_key[i].data+j));
					}
					printf("\n%s:  plaintxt:", sec_arg->enc_key[i].name);
					for( j=0; j<sec_arg->enc_key[i].size; j+=1)
					{
						printf("%02x", *((u8*)key_data+j));
					}
					printf("\n%s:       key:", sec_arg->enc_key[i].name);
					for( j=0; j<BCM_SECBT_AES_CBC128_EK_LEN*2; j+=1)
					{
						printf("%02x",  *((u8*)dec_aes_key+j));
					}
					printf("\n");
				}
#endif				

				/* If no permissions are set, DO NOT export key */
				if( sec_arg->enc_key[i].perm ) {
					if ( strcasecmp(sec_arg->enc_key[i].perm, DELG_EXPORT_PERMSEC) == 0 ) {
						/* For secure keys see if there is a load address */
						if( sec_arg->enc_key[i].load_addr ) {
							printf("INFO: copying key %s to %llx\n", 
								sec_arg->enc_key[i].name,sec_arg->enc_key[i].load_addr); 
							memcpy((void*)(uintptr_t)sec_arg->enc_key[i].load_addr, key_data, sec_arg->enc_key[i].size); 
						} else {
							/* Add secure key to dtb */
							sec_add_decoded_key_fdt(sec_arg->arg,
								sec_arg->enc_key[i].name,
								key_data,
							        sec_arg->enc_key[i].size,
								DELG_EXPORT_PERMSEC, 0);
						}
					} else {
						/* For non-secure keys add them to dtb */
						sec_add_decoded_key_fdt(sec_arg->arg,
							sec_arg->enc_key[i].name,
							key_data,
							sec_arg->enc_key[i].size,
							DELG_EXPORT_PERMNONSEC, sec_arg->enc_key[i].exp_flag);
					}
				}
				free(key_data);
				key_data = NULL;
			}
		}
		break;
       	case SEC_CTRL_KEY_CHAIN_AES:
		if (sec->state&SEC_STATE_SECURE) {
			int i;
			bcm_sec_export_item_t item;
			bcm_sec_key_arg_t* sec_arg = arg;
			item.salt = NULL;
			item.algo = NULL;
			item.exp_flag = 0;
			for (i = 0; i< sec_arg->len; i++) {
				item.name =  sec_arg->aes[i].id;
				item.len = BCM_SECBT_AES_CBC128_EK_LEN*2;
				item.value = sec_arg->aes[i].key;
				if (sec_add_export_item_fdt(sec_arg->arg, &item)) {
					rc = -1;
					printf("ERROR: unable to chain aes_key\n");
				}
			}
		}
		break;
        case SEC_CTRL_KEY_CLEAN_SEC_MEM:
		memset((void*)bcm_secbt_args(),0, sizeof(bcm_secbt_args_t));
		break;
        case SEC_CTRL_KEY_CLEAN_ALL:
		bcm_sec_clean_secmem(sec);
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
	sec->cb[SEC_CTRL_ARG_SOTP].cb = sec_otp_ctrl;
#ifndef CONFIG_SMC_BASED
	sec->cb[SEC_CTRL_ARG_RNG].cb = sec_rng_ctrl;
#endif
}

void bcm_sec_get_root_aes_key(u8** key)
{
	*key = bcm_sec()->key.aes_ek;
}

u8* bcm_sec_get_root_pub_key(void)
{
	return bcm_sec()->key.rsa_pub;
}

static void bcm_sec_set_delg_cfg( bcm_sec_delg_cfg * cfg )
{
	bcm_sec()->delg_cfg_obj = cfg;
}

bcm_sec_delg_cfg * bcm_sec_get_delg_cfg(void)
{
	return bcm_sec()->delg_cfg_obj;
}

static int bcm_sec_retrieve_export_items( u8 * fit_hdr, int sec_export_offset, int * count , char * allowed_exports)
{
	int exp_item_offset = -1;
	int ndepth = 0;
	int * value = NULL;

	if (sec_export_offset) {
		/* Process all image subnodes */
		for (ndepth = 0,
		     exp_item_offset = fdt_next_node(fit_hdr, sec_export_offset, &ndepth);
				(exp_item_offset >= 0) && (ndepth > 0);
				exp_item_offset = fdt_next_node(fit_hdr, exp_item_offset, &ndepth)) {
			if (ndepth == 1) {
				/*
				 * Direct child node of the sec_export parent node,
				 * i.e. item node.
				 */

				/* Get key name */
				bcm_sec_exported_items[*count].value = NULL;
				bcm_sec_exported_items[*count].name = (char*)fdt_getprop((void *)fit_hdr, exp_item_offset, DELG_EXP_ITEM_NAME, NULL);
				bcm_sec_exported_items[*count].id = (char*)fdt_getprop((void *)fit_hdr, exp_item_offset, DELG_EXP_ITEM_ID, NULL);
				value = (int*)fdt_getprop((void *)fit_hdr, exp_item_offset, DELG_EXP_ITEM_LENGTH, NULL);

				if( !bcm_sec_exported_items[*count].name || !bcm_sec_exported_items[*count].id || !value )
					continue;

				printf("INFO: Found item:%s id:%s in fit\n", bcm_sec_exported_items[*count].name,  bcm_sec_exported_items[*count].id);

				/* Check if requested item is allowed to be exported */
				if( allowed_exports ) {
					if( !strstr(allowed_exports, bcm_sec_exported_items[*count].id))
						continue;
				}

				bcm_sec_exported_items[*count].len = be32_to_cpu(*value);
				if( !bcm_sec_exported_items[*count].len)
					continue;

				/* Get optional 32-bit salt or name of salt-key */
				bcm_sec_exported_items[*count].salt = (char*)fdt_getprop((void *)fit_hdr, exp_item_offset, DELG_EXP_ITEM_SALT, 
									&(bcm_sec_exported_items[*count].salt_len));

				/* Get optional hashing algorithm */
				bcm_sec_exported_items[*count].algo = (char*)fdt_getprop((void *)fit_hdr, exp_item_offset, DELG_EXP_ITEM_ALGO, NULL);
				if( bcm_sec_exported_items[*count].algo ) {
					if(strncmp((const char*)(bcm_sec_exported_items[*count].algo), "sha256", strlen("sha256")) != 0) {
						printf("INFO: Unsupported salt/hash algorithm %s! Skipping Item!\n", bcm_sec_exported_items[*count].algo);
						continue;
					}
				}

				/* Get optional os_export flag. If flag is not present, export all items to Linux by default */
				value = (int*)fdt_getprop((void *)fit_hdr, exp_item_offset, DELG_OS_EXPORT_FLAG, NULL);
				if (value && (strcasecmp((const char*)value, "no") == 0))
					bcm_sec_exported_items[*count].exp_flag = 0;
				else
					bcm_sec_exported_items[*count].exp_flag = 1;

				/* Get optional permissions flag. If flag is not set, non-secure is assumed for backwards compatibility */
				bcm_sec_exported_items[*count].perm = (char*)fdt_getprop((void *)fit_hdr, exp_item_offset, DELG_EXPORT_PERM, NULL);

				*count = *count + 1;

				if( *count >= BCM_SEC_MAX_EXPORTED_ITEMS ) {
					printf("INFO: Max secure export item limit of %d items reached! Ignoring other items!\n", BCM_SEC_MAX_EXPORTED_ITEMS);
					break;
				}
			}
		}
	}
	return *count;
}
static int bcm_sec_process_exports(u8 * fit_hdr, bcm_sec_cb_arg_t* key_args)
{
	u8 * fit_ptr = NULL;
	bcm_sec_delg_cfg* delg_cfg = bcm_sec_get_delg_cfg();
	char * allowed_exports = NULL;
	bcm_sec_key_arg_t * item_list;
	int sec_export_offset = -1;
	int count = 0;
	int * value = NULL;

	memset(bcm_sec_exported_items, 0, sizeof(bcm_sec_export_item_t)*BCM_SEC_MAX_EXPORTED_ITEMS);

	/* IF delegations are active then sec_exports in security 
	 * policy control delegate requested exports */
	if( delg_cfg && delg_cfg->delg_id ) 
	{
		fit_ptr = delg_cfg->sec_policy_fit;
		sec_export_offset =  fdt_path_offset (fit_ptr, DELG_SEC_POL_SEC_EXPORT_PATH);
		if (sec_export_offset < 0) {
			debug("INFO: Could not find %s in security policy\n", DELG_SEC_POL_SEC_EXPORT_PATH);
		} else {
			/* Check if sec_export_offset is disabled */
			value = (int*)fdt_getprop((void *)fit_ptr, sec_export_offset, DELG_SEC_POL_STATUS, NULL);
			if (value && (strcasecmp((const char*)value, DELG_SEC_POL_STATUS_DISABLED) == 0)){
				printf("INFO: Found DISABLED  %s  in security policy\n", DELG_SEC_POL_SEC_EXPORT_PATH);
				sec_export_offset = -1;	
			} else {
				printf("INFO: Found %s  in security policy\n", DELG_SEC_POL_SEC_EXPORT_PATH);
			}

			/* Retrieve key-owner specified export items - all exports are allowed */
			bcm_sec_retrieve_export_items(fit_ptr, sec_export_offset, &count, NULL);

			/* Retrieve allowed exports for delegates */
			allowed_exports = (char*)fdt_getprop((void *)fit_ptr, sec_export_offset, DELG_SEC_ALLOWED_EXPORTS, NULL);
			if( allowed_exports && strlen((const char*)allowed_exports) == 0 ) {
				/* No delegate exports allowed */
				allowed_exports = NULL;
			}
		}
	} else {
		/* If delegation is not active, allow all default exports */
		allowed_exports = &default_allowed_exports[0];
	}

	/* Check delegate's requested exports */
	if( fit_hdr && allowed_exports ) {
		delg_cfg = NULL;
		fit_ptr = fit_hdr;
		sec_export_offset =  fdt_path_offset (fit_ptr, DELG_SEC_EXPORT_PATH);
		if (sec_export_offset < 0) {
			debug("INFO: Could not find %s  in fit\n", DELG_SEC_EXPORT_PATH);
			return 0;
		} else {
			/* Check if sec_export_offset is disabled */
			value = (int*)fdt_getprop((void *)fit_ptr, sec_export_offset, DELG_SEC_POL_STATUS, NULL);
			if (value && (strcasecmp((const char*)value, DELG_SEC_POL_STATUS_DISABLED) == 0)){
				printf("INFO: Found DISABLED  %s  in fit\n", DELG_SEC_EXPORT_PATH);
				sec_export_offset = -1;
			} else {
				printf("INFO: Found %s  in fit\n", DELG_SEC_EXPORT_PATH);
			}
		}

		/* Retrieve delegate specified export items */
		if (sec_export_offset) 
			bcm_sec_retrieve_export_items(fit_hdr, sec_export_offset, &count, allowed_exports);
	}

	if( count ) {
		item_list = malloc(sizeof(bcm_sec_key_arg_t));
		if (!item_list) {
			printf("ERROR: Cannot allocate memory for exported item list!\n");
			return -1;
		}
		item_list->len = count;
		item_list->item = &bcm_sec_exported_items[0];
		key_args->arg[2].ctrl = SEC_CTRL_KEY_EXPORT_ITEM;
		key_args->arg[2].ctrl_arg = item_list;
	}
	return 0;
}

static int bcm_sec_get_antirollback_node( u8 * fit, char * path, int * req_antirollback, int * antirollback_limit)
{
	/* Get key-owners antirollback update node from policy */
	int * value = NULL;
	int nodeoffset = fdt_path_offset((void *)fit, path);
	if( nodeoffset < 0 ) {
		printf("INFO: %s node NOT found!\n", path);
	} else  {
		/* Check if node is disabled */
		value = (int*)fdt_getprop((void *)fit, nodeoffset, DELG_SEC_POL_STATUS, NULL);
		if (value && (strcasecmp((const char*)value, DELG_SEC_POL_STATUS_DISABLED) == 0)) {
			nodeoffset = -1;
			printf("INFO: Found disabled %s node!\n", path);
		} else {
			printf("INFO: Found %s node!\n", path);
			/* retrieve anti=rollback settings */
			value = (int*)fdt_getprop((void *)fit, nodeoffset, DELG_SEC_POL_NEW_ANTIROLLBCK, NULL);
			if (value && req_antirollback )
				*req_antirollback =  be32_to_cpu(*value); 

			value = (int*)fdt_getprop((void *)fit, nodeoffset, DELG_SEC_POL_ANTIROLLBCK_LIM, NULL);
			if (value && antirollback_limit )
				*antirollback_limit =  be32_to_cpu(*value); 
		}
	}
	return nodeoffset;
}

static int bcm_sec_process_antirollback( u8 * fit_hdr)
{
	/* Process rollback update node for both key-owner and delegate.
	 * key-owner's rollback update node can limit the rollback levels that 
	 * can be updated by delegate */
	u32 pol_req_antirollback = 0;
	u32 pol_antirollback_limit = 0;
	u32 delg_req_antirollback = 0;
	u32 commit_antirollback = 0;
	u32 current_antirollback = 0;
	u32 delg_maximum_antirollback = 0;
	int nodeoffset_pol = -1;
	int nodeoffset_fit = -1;
	bcm_sec_delg_cfg * delg_cfg = NULL;
	int ret = -1;

	/* Get current anti-rollback level */
	bcm_sec_get_antirollback_lvl(&current_antirollback);

	/* Check if delegations are active, get key-owners antirollback settings */
	delg_cfg = bcm_sec_get_delg_cfg();
	if( delg_cfg && delg_cfg->delg_id ) {
		/* Get maximum rollback valid for this delegation */
		delg_maximum_antirollback = delg_cfg->max_antirollback;

		/* Get key-owners antirollback update node from policy */
		nodeoffset_pol = bcm_sec_get_antirollback_node((void *)delg_cfg->sec_policy_fit, DELG_SEC_POL_ANTI_ROLLBACK_PATH,
								(int*)&pol_req_antirollback, (int*)&pol_antirollback_limit);
	}

	/* Get delegates antirollback update node from main FIT hdr */
	if( fit_hdr ) {
		nodeoffset_fit = bcm_sec_get_antirollback_node((void *)fit_hdr, DELG_ANTI_ROLLBACK_PATH,
								(int*)&delg_req_antirollback, NULL);
	}

	/* Early return if no node found in security policy or FIT hdr */
	if ((nodeoffset_pol < 0) && (nodeoffset_fit < 0)) {
		return 0;
	}

	printf("INFO: pol_req:%d pol_lim:%d delg_req:%d max:%d curr:%d\n",
		pol_req_antirollback, pol_antirollback_limit, delg_req_antirollback, 
		delg_maximum_antirollback, current_antirollback);

	/* Verify key-owners requested antirollback */
	if( pol_req_antirollback ) {
		if( pol_req_antirollback > delg_maximum_antirollback ) {
			printf("ERROR: Invalid security policy requested antirollback:%d, Max:%d\n",
				pol_req_antirollback, delg_maximum_antirollback);
			return ret;
		}
	} 

	/* Ensure that key-owner's anti-rollback limit for delegate's falls within the max allowable 
	 * Also, set the antirollback_limit for the delegate to max allowable if antirollback node
	 * in policy is not enabled */
	if( (nodeoffset_pol < 0) || (pol_antirollback_limit > delg_maximum_antirollback) )
		pol_antirollback_limit = delg_maximum_antirollback;
	
	/* Verify delegates requested antirollback */
	if( delg_req_antirollback ) {
		if( delg_req_antirollback > pol_antirollback_limit) {
			printf("INFO: Invalid delegate requested antirollback:%d, Max:%d. Ignoring update request\n",
				delg_req_antirollback, pol_antirollback_limit);
			delg_req_antirollback = 0;
		}
	} 
	
	/* Figure out the requested antirollback level to commit */
	if (delg_req_antirollback > pol_req_antirollback)
		commit_antirollback = delg_req_antirollback;
	else
		commit_antirollback = pol_req_antirollback;

	/* Do boundary checks */
	if( !commit_antirollback || (commit_antirollback <= current_antirollback) ) {
		debug("INFO: Not committing requested anti_rollback level %d! Min:%d\n", 
			commit_antirollback,current_antirollback);
		ret = 0;
	} else {
#if defined(CONFIG_BCM_BOOTSTATE)
		if (0 == ((bcmbca_get_boot_reason() >> BCM_RESET_REASON_BITS) & BCM_BOOT_REASON_ACTIVATE)) {
			/* commit the antirollback */
			printf("INFO: Committing antirollback level:%d\n", commit_antirollback);
			ret = bcm_sec_set_antirollback_lvl(commit_antirollback);
		} else {
			printf("INFO: Will set antirollback level:%d once image is committed\n",
				 commit_antirollback);
		}
#endif
	}

	return ret;
}

static int bcm_sec_process_hw_state(u8 * fit_hdr, bcm_sec_cb_arg_t* sotp_st_arg, bcm_sec_cb_arg_t* rng_st_arg)
{
	int * value = 0;
	u8 * fit_ptr = NULL;
	bcm_sec_state_t st = bcm_sec_state();
	bcm_sec_delg_cfg* delg_cfg = bcm_sec_get_delg_cfg();
	int node = -1;

	/* IF delegations are active then hw_state in security 
	 * policy overrides delegate specified state */
	if( delg_cfg && delg_cfg->delg_id ) 
	{
		fit_ptr = delg_cfg->sec_policy_fit;
		node =  fdt_path_offset (fit_ptr, DELG_SEC_POL_HW_STATE_PATH);
		if (node < 0) {
			debug("INFO: Could not find %s node in security policy\n", DELG_SEC_POL_HW_STATE_PATH);
		} else {
			/* Check if node is disabled */
			value = (int*)fdt_getprop((void *)fit_ptr, node, DELG_SEC_POL_STATUS, NULL);
			if (value && (strcasecmp((const char*)value, DELG_SEC_POL_STATUS_DISABLED) == 0)){
				printf("INFO: Found DISABLED  %s node in security policy\n", DELG_SEC_POL_HW_STATE_PATH);
				node = -1;	
			} else {
				printf("INFO: Found %s node in security policy\n", DELG_SEC_POL_HW_STATE_PATH);
			}
		}
	}

	/* Check delegate's specified hw_state */
	if( (node < 0) && fit_hdr ) {
		delg_cfg = NULL;
		fit_ptr = fit_hdr;
		node =  fdt_path_offset (fit_ptr, DELG_HW_STATE_PATH);
		if (node < 0) {
			debug("INFO: Could not find %s node in fit\n", DELG_HW_STATE_PATH);
			node = -1;
		} else {
			/* Check if node is disabled */
			value = (int*)fdt_getprop((void *)fit_ptr, node, DELG_SEC_POL_STATUS, NULL);
			if (value && (strcasecmp((const char*)value, DELG_SEC_POL_STATUS_DISABLED) == 0)){
				printf("INFO: Found DISABLED  %s node in fit\n", DELG_HW_STATE_PATH);
				node = -1;
			} else {
				printf("INFO: Found %s node in fit\n", DELG_HW_STATE_PATH);
			}
		}
	}

	/* Set the default hw state */
	/* If in secure mode and sotp-lock not specified     --> LOCK by default 
	 * If in NON-secure mode and sotp-lock not specified --> UNLOCK by default */
	/* If in secure mode and rng-lock not specified     --> LOCK by default 
	 * If in NON-secure mode and rng-lock not specified --> UNLOCK by default */
	if(st == SEC_STATE_SECURE) {
		sotp_st_arg->arg[0].ctrl = SEC_CTRL_SOTP_LOCK_ALL;
		rng_st_arg->arg[0].ctrl = SEC_CTRL_RNG_LOCK_ALL;
	} else {
		sotp_st_arg->arg[0].ctrl = SEC_CTRL_SOTP_UNLOCK_SOTP_UNSEC;
		rng_st_arg->arg[0].ctrl = SEC_CTRL_RNG_UNLOCK_RNG_UNSEC;
	}

	if( node >= 0 ) {
		/* set hw state from node */
		bcm_sec_set_sotp_hw_state(fit_ptr, node, sotp_st_arg); 
		bcm_sec_set_rng_hw_state(fit_ptr, node, rng_st_arg); 
	}
	return 0;
}

static int get_loadable_image_name(const void *fit, int index,  char **outname)
{
	const char *name, *str;
	int conf_node;
	int len, i;

	conf_node = fit_find_config_node(fit);
	if (conf_node < 0) {
		printf("No matching DT out of these options:\n");
	}

	name = fdt_getprop(fit, conf_node, "loadables", &len);
	if (!name) {
		debug("cannot find property '%s': %d\n", "loadables", len);
		return -EINVAL;
	}

	str = name;
	for (i = 0; i < index; i++) {
		str = strchr(str, '\0') + 1;
		if (!str || (str - name >= len)) {
			debug("no string for index %d\n", index);
			return -E2BIG;
		}
	}

	*outname = (char *)str;
	return 0;
}

static int get_fit_value( void * fit_ptr, int node_offset, char * name , int * return_val) 
{
	int ret = 0;
	int * value = (int*)fdt_getprop((void *)fit_ptr, node_offset, name, NULL);
	if ( value ) 
		*return_val = be32_to_cpu(*value); 
	else
		ret = 1;

	return ret;
}

static int bcm_sec_get_sec_restriction_subnode( u8* sec_pol_fit_ptr, char * required_subnode )
{
	int ret = -EINVAL;
	int * value = 0;
	int node = -1;

	/* Get security sw restrictions node offset */
	node =  fdt_path_offset (sec_pol_fit_ptr, DELG_SEC_POL_SW_RESTRICT_PATH);
	if (node < 0) {
		debug("INFO: Could not find %s node in security policy\n", DELG_SEC_POL_SW_RESTRICT_PATH);
	} else {
		/* Check if node is disabled */
		value = (int*)fdt_getprop((void *)sec_pol_fit_ptr, node, DELG_SEC_POL_STATUS, NULL);
		if (value && (strcasecmp((const char*)value, DELG_SEC_POL_STATUS_DISABLED) == 0)){
			debug("INFO: Found DISABLED %s node in security policy\n", DELG_SEC_POL_SW_RESTRICT_PATH);
		} else {
			debug("INFO: Found %s node in security policy\n", DELG_SEC_POL_SW_RESTRICT_PATH);

			/* Get required subnode offset */
			node = fdt_subnode_offset (sec_pol_fit_ptr, node,  required_subnode);
			if( node < 0 ) {
				debug("INFO: Could not find %s/%s node in security policy\n", 
					DELG_SEC_POL_SW_RESTRICT_PATH, required_subnode);
			} else {
				/* Check if node is disabled */
				value = (int*)fdt_getprop((void *)sec_pol_fit_ptr, node, DELG_SEC_POL_STATUS, NULL);
				if (value && (strcasecmp((const char*)value, DELG_SEC_POL_STATUS_DISABLED) == 0)){
					debug("INFO: Found DISABLED %s/%s node in security policy\n", 
						DELG_SEC_POL_SW_RESTRICT_PATH, required_subnode);
				} else {
					debug("INFO: Found %s/%s node in security policy\n", 
						DELG_SEC_POL_SW_RESTRICT_PATH, required_subnode);
					ret = node;
				}
			}
		}
	}
	return ret;
}

static int bcm_sec_auth_dbg_cert( char * certp, uint8_t* auth_key )
{
	int rc = -EINVAL;
	struct image_sign_info im;
	int cert_size = 0;
	char * cert_sig = NULL;
 

	/* Get size - adjust size to align with 4byte boundaries */
	if( !fdt_check_header(certp) ) {
		cert_size = fdt_totalsize(certp);
	} else {
		printf("ERROR: Invalid (or missing) dbg_cert!\n");
		return rc;
	}

	
	/* Get sig pointer - Sig is aligned to 4 byte boundary */
	cert_sig = certp + ((cert_size + 3)&(~3));

#ifdef CERT_DBG
	printf(" cert_size:%d\n", cert_size);
	printf(" %02x %02x %02x %02x\n", *certp, *(certp+1), *(certp+2), *(certp+3));
	printf(" %02x %02x %02x %02x\n", *(cert_sig-4), *(cert_sig-3), *(cert_sig-2), *(cert_sig-1));
	printf(" %02x %02x %02x %02x\n", *cert_sig, *(cert_sig+1), *(cert_sig+2), *(cert_sig+3));
	printf(" %02x %02x %02x %02x\n", *(cert_sig+RSA2048_BYTES-4), *(cert_sig+RSA2048_BYTES-3), *(cert_sig+RSA2048_BYTES-2), *(cert_sig+RSA2048_BYTES-1));
	printf(" %02x %02x %02x %02x\n", *auth_key, *(auth_key+1), *(auth_key+2), *(auth_key+3));
	printf(" %02x %02x %02x %02x\n", *(auth_key+RSA2048_BYTES-4), *(auth_key+RSA2048_BYTES-3), *(auth_key+RSA2048_BYTES-2), *(auth_key+RSA2048_BYTES-1));
#endif	
	
	/* Setup signing structures */
	im.checksum = image_get_checksum_algo("sha256,");
	if (!im.checksum) {
		printf("ERROR: couldn't get checksum algo\n"); 
		return rc;
	}
	rc = bcm_sec_rsa_verify((const u8*)certp, cert_size, 
				(const u8*)cert_sig, RSA2048_BYTES, 
				(const u8*)auth_key, &im );
	if( rc == 0 ) {
		debug("INFO: dbg_cert Authenticated Successfully!\n");
	
	} else {
		printf("ERROR: dbg_cert Authentication Failed!\n");
	}

	return rc;
}

#if defined(CONFIG_MMC)
#define DBG_GRP_CERT_GPT_PREFIX "gpt:"
static int bcm_sec_get_mmc_dbg_cert( char * group_cert, char ** certp, uint32_t * cert_size)
{
	int ret = -1;
	__maybe_unused loff_t filelen, actlen;
	disk_partition_t part_info = {};
	struct mmc *mmc;
	struct blk_desc *block_dev;
	char * file = NULL;
	char * gpt_part = NULL;

	gpt_part = strstr(group_cert, DBG_GRP_CERT_GPT_PREFIX) ;
	if(gpt_part && 
		(gpt_part+strlen(DBG_GRP_CERT_GPT_PREFIX) < group_cert+strlen(group_cert))) {

		/* Get GPT partition name */
		gpt_part = gpt_part + strlen(DBG_GRP_CERT_GPT_PREFIX);
		gpt_part = strstr(gpt_part, "/");
		if( gpt_part ) {
			gpt_part++;
		} else {
			printf("ERROR: Invalid dbg_cert partition name!\n");
			return ret;
		}

		/* Get filename */
		file = strstr(gpt_part, "/");
		if( file ) {
			*file = '\0';
			file++;
			if(!strlen(file)) {
				printf("ERROR: Invalid debug cert filename!\n");
				return ret;
			}
		} else {
			printf("ERROR: Cannot find debug cert filename!\n");
			return ret;
		}
	} else {
		printf("ERROR: Cannot find GPT partition for dbg_cert %s\n", group_cert);
		return ret;
	}
	printf("INFO: Looking for dbg_cert in /%s/%s\n", gpt_part,file);

	mmc = find_mmc_device(0);
	if( !mmc ) {
		printf("ERROR: cannot get mmc device!\n");
		return (-1);
	}
	block_dev = mmc_get_blk_desc(mmc);
	if (part_get_info_by_name(block_dev, gpt_part, &part_info) < 0) {
		printf("ERROR: Cannot find gpt partition %s!\n", gpt_part);
		return -1;
	}

	/* Read file from ext4 partition */
	ext4fs_set_blk_dev(block_dev, &part_info);
	ret = ext4fs_mount(0);
	if (ret < 0) {
		printf("ERROR: ext4fs_mount of gpt partition %s failed!\n", gpt_part);
		return ret;
	}
	ret = ext4fs_open(file, &filelen);
	if (ret < 0) {
		printf("ERROR: ext4fs_open of file /%s/%s failed!\n", gpt_part, file);
		return ret;
	}
	*certp = malloc(filelen);
	if(!(*certp)) {
		printf("ERROR: cannot alloc %llu bytes for dbg_cert!\n", filelen);
		return -1;
	}
	ret = ext4fs_read((void *)*certp, 0, filelen, &actlen);
	if (ret < 0) {
		printf("ERROR: reading file /%s/%s, ret - %d\n", gpt_part, file, ret);
		return ret;
	}
	
	return 0;
}
#endif

#if defined(CONFIG_NAND)
#define DBG_GRP_CERT_UBI_PREFIX "ubi:"
static int bcm_sec_get_nand_dbg_cert( char * group_cert, char ** certp, uint32_t * cert_size)
{
	char * ubivol = NULL;
	int ret = -EINVAL;
	bcaspl_part_info info;
	struct ubispl_load volume;

	/* parse dgb cert volid */
	ubivol = strstr(group_cert, DBG_GRP_CERT_UBI_PREFIX) ;
	if( ubivol && (ubivol+strlen(DBG_GRP_CERT_UBI_PREFIX) < group_cert+strlen(group_cert)) ) {
		ubivol = ubivol + strlen(DBG_GRP_CERT_UBI_PREFIX);
	} else {
		printf("ERROR: Cannot find ubi volume for dbg_cert %s\n", group_cert);
		return ret;
	}

	/* Assign buffer */
	//*certp = (char*)spl_get_load_buffer(0, 0);
	volume.load_addr = *certp;
	volume.vol_id = simple_strtoul(ubivol, NULL, 0);
	debug("INFO: ubi vol %d found for dbg_cert\n", volume.vol_id);

	/* Get current ubi info */
	tpl_get_ubi_info(&info);

	/* Load volume */
	ret = ubispl_load_volumes(&(info.ubi_info), &volume, 1);

	return ret;
}

#define DBG_CERT_PAD 0x100
static u32 tpl_get_total_fit_size( u8* fit_hdr )
{
	int images_noffset;
	int noffset;
	int ndepth;
	unsigned int data_size, data_offset, fit_size;
	const fdt32_t *val;
	int ret = 0;
	
	/* Find images parent node offset */
	ret = fdt_check_header(fit_hdr);
	if(ret)
	{
		printf("Invalid FDT hdr check failed:%d! Aborting!\n", ret);
	}
	
	images_noffset = fdt_path_offset(fit_hdr, "/images");
	if (images_noffset < 0) {
		printf("Can't find images parent node \n");
		return -1;
	}

	/* Process all image subnodes */
	for (ndepth = 0, fit_size = 0,
	 noffset = fdt_next_node(fit_hdr, images_noffset, &ndepth);
	(noffset >= 0) && (ndepth > 0); noffset = fdt_next_node(fit_hdr, noffset, &ndepth)) 
	{
		if (ndepth == 1) 
		{
			/* Offset - Check for both relative and absolute offsets */
			val = fdt_getprop(fit_hdr, noffset, "data-offset", NULL);
			if( val ) 
			{
				/* Relative offset */
				data_offset = fdt32_to_cpu(*val);
				data_offset += ((fdt_totalsize(fit_hdr) + 3) & ~3);
			}
			else
			{
				val = fdt_getprop(fit_hdr, noffset, "data-position", NULL);
				if (!val)
					continue;

				/* Absolute offset */
				data_offset = fdt32_to_cpu(*val);
			}
			val = fdt_getprop(fit_hdr, noffset, "data-size", NULL);
			if (!val)
			    continue;
			
			data_size = fdt32_to_cpu(*val);

			if( data_offset + data_size > fit_size )
				fit_size = data_offset + data_size;
		}
	}
	return fit_size;
}
#endif

#if defined(CONFIG_TPL_SPI_FLASH_SUPPORT)
static int bcm_sec_get_spinor_dbg_cert( char * group_cert, char ** certp, uint32_t * cert_size)
{
	debug("INFO: %s not implemented!\n", __FUNCTION__);
	return 0;
}
#endif


static int bcm_sec_get_dbg_cert( u8* loaded_fit_hdr, char * group_cert, char ** certp, uint32_t * cert_size)
{
	char * imgdev_name = tpl_get_imgdev_name();
	int ret = -EINVAL;
	if( imgdev_name ) {
#if defined(CONFIG_NAND)
		if (strcmp(imgdev_name,"NAND") == 0)
		{
			/* For NAND, we can only load entire volumes, we have no way of
			 * knowing how big the volumes are due to lack of existing UBI API
			 * in SPL/TPL. Therefore only place we can safely load certificate is
			 * at the end of the loaded FIT image */
			*certp = (char*)(loaded_fit_hdr + tpl_get_total_fit_size(loaded_fit_hdr) + DBG_CERT_PAD);
			ret = bcm_sec_get_nand_dbg_cert( group_cert, certp, cert_size);
		}
#endif
#if defined(CONFIG_MMC)
		if (strcmp(imgdev_name,"EMMC") == 0)
			ret = bcm_sec_get_mmc_dbg_cert( group_cert, certp, cert_size);
#endif
#if defined(CONFIG_TPL_SPI_FLASH_SUPPORT)
		if (strcmp(imgdev_name,"SPINOR") == 0)
			ret = bcm_sec_get_spinor_dbg_cert( group_cert, certp, cert_size);
#else
		(void) imgdev_name;
#endif
	}
	return ret;
}

static int bcm_sec_process_dbg_cert(char * certp, char * group_name, u8* sec_pol_fit_ptr, 
				int dbg_grp_offset)
{
	int ret = -EINVAL;
	char * value = NULL;
	int size = 0;
	int i;
	int node = fdt_path_offset(certp, DELG_DBG_CERT_NODE_NAME);
	char sec_ser_num[32] = {0};
	value = (char*)fdt_getprop((void *)certp, node, DELG_DBG_GRP_NAME, NULL);
	if (strncmp(group_name, value, strlen(group_name)) != 0) {
		printf("ERROR: cert_grp_name:%s sec_pol_grp_name:%s DOESNT MATCH!\n", value,
			group_name);
	} else {
		debug("INFO: cert_grp_name:%s sec_pol_grp_name:%s Match!\n", value,
			group_name);
		value = (char*)fdt_getprop((void *)certp, node, DELG_DBG_GRP_SERNUM, &size);
		if( !value || !size || (size % 32 )) {
			printf("ERROR: Invalid serial number(s): size:%d\n", size);
		} else {
			debug("INFO: Detected serial number(s): size:%d\n", size);
			if( bcm_sec_get_sec_ser_num((char*)sec_ser_num, sizeof(u32)*8) )
			{
				printf("ERROR: serial number retrieval failed!\n");
				return ret;
			}

			/* Compare local serial number to one in debug_cert */
			for( i=0; i<size; i+=32 )
			{
				/* check if any of the 32byte serial numbers match local */
				if( memcmp(&value[i],&sec_ser_num[0], 32) == 0 )
				{
					debug("INFO: dbg_grp Serial[%d] Matches!\n", i/32);
					return 0;
				}
			}
		}
	}

	/* Cleanup memory as eneded */
#if defined(CONFIG_MMC)
	{
		char * imgdev_name = tpl_get_imgdev_name();
		if (imgdev_name && (strcmp(imgdev_name,"EMMC") == 0))
			free(certp);
	}
#endif
	return ret;
}

static int bcm_sec_delg_process_debug_groups( u8 * fit_hdr ) 
{
	int ret = -EINVAL;
	u8 * sec_pol_fit_ptr = NULL;
	int count=0;
	int ndepth;
	int dbg_grp_desc_offset, dbg_grps_offset;
	bcm_sec_delg_cfg* delg_cfg = bcm_sec_get_delg_cfg();


	if( delg_cfg && delg_cfg->delg_id ) {
		sec_pol_fit_ptr = delg_cfg->sec_policy_fit;
	} else {
		debug("INFO: Delegation not active, skipping debug_groups\n"); 
		return 0;
	}

	/* Get debug groups node offset */
	dbg_grps_offset = bcm_sec_get_sec_restriction_subnode(sec_pol_fit_ptr, 
				DELG_SEC_POL_DBG_GRPS);

	/* If No debug groups detected, return success */
	if (dbg_grps_offset < 0) 
		return 0;

	/* Process all required image subnodes in order */
	for (ndepth = 0, count = 0, dbg_grp_desc_offset = fdt_next_node(sec_pol_fit_ptr, dbg_grps_offset, &ndepth);
		(dbg_grp_desc_offset >= 0) && (ndepth > 0);
		dbg_grp_desc_offset = fdt_next_node(sec_pol_fit_ptr, dbg_grp_desc_offset, &ndepth)) {
		if (ndepth == 1) {
			char * group_name = NULL;
			char * group_cert = NULL;
			uint8_t* key_ptr = NULL;
			char* certp = NULL;
			uint32_t cert_size;

			/*
			 * Direct child node of the debug_groups parent node
			 */

			/* Get debug group details from security policy */
			group_name  = (char*)fdt_getprop((void *)sec_pol_fit_ptr, 
						dbg_grp_desc_offset,    DELG_DBG_GRP_NAME, NULL);
			group_cert  = (char*)fdt_getprop((void *)sec_pol_fit_ptr, 
						dbg_grp_desc_offset,    DELG_DBG_GRP_CERT, NULL);
			key_ptr     = (uint8_t*)fdt_getprop((void *)sec_pol_fit_ptr, 
						dbg_grp_desc_offset, DELG_DBG_GRP_KEY, NULL);

			if( group_name && group_cert ) {
				printf("INFO: Found dbg_grp:%s dbg_cert:%s\n", group_name, 
					group_cert);
			} else {
				debug("INFO: Invalid dbg_grp at index:%d\n", count);
				count++;
				continue;
			}

			/* If no key is specified, use FLD ROT public key for authentication */
			if( !key_ptr ) {
				printf("INFO: No dbg_cert auth key found, using ROT!\n"); 
				key_ptr = bcm_sec_get_root_pub_key();
			}


			/* Increment */
			count++;

			/* 1 - Get certificate from flash */
			ret = bcm_sec_get_dbg_cert( fit_hdr, group_cert, &certp, &cert_size);
			if( ret ) {
				printf("ERROR: Could not find dbg_cert for dbg_group:%s\n", 
					group_name);
				return ret;
			}

			/* 2 - Authenticate dbg_cert */
			ret = bcm_sec_auth_dbg_cert( certp, key_ptr );
			if( ret ) {
				printf("ERROR: dbg_cert authentication failed for dbg_group:%s\n", 
					group_name);
				return ret;
			} 

			/* 3 - Process dbg_cert */
			ret = bcm_sec_process_dbg_cert(certp, group_name, sec_pol_fit_ptr,
							dbg_grp_desc_offset); 
			if( ret ) {
				printf("ERROR: dbg_cert NOT VALID for dbg_group:%s!\n",
					group_name);
				return ret;
			} else {
				printf("INFO: dbg_cert is VALID for dbg_group:%s!\n",
					group_name);
			}
		}
	}
	/* Error out if we didnt find any debug groups images */
	if( !count ) {
		printf("ERROR: Could not find any valid debug groups under %s/%s in security policy\n", 
				DELG_SEC_POL_SW_RESTRICT_PATH, DELG_SEC_POL_DBG_GRPS);
		return -EINVAL;
	}
	return 0;
}

static int bcm_sec_delg_process_required_images( u8 * fit_hdr, int verify_loaded_img)
{
	u8 * sec_pol_fit_ptr = NULL;
	int count=0;
	int ndepth;
	int img_desc_offset, req_imgs_offset, fit_img_offset, fit_img_hash_offset;
	bcm_sec_delg_cfg* delg_cfg = bcm_sec_get_delg_cfg();

	if( delg_cfg && delg_cfg->delg_id ) {
		sec_pol_fit_ptr = delg_cfg->sec_policy_fit;
	} else {
		printf("INFO: Delegation not active, skipping required_images\n"); 
		return 0;
	}

	/* Get required images node offset */
	req_imgs_offset = bcm_sec_get_sec_restriction_subnode(sec_pol_fit_ptr, DELG_SEC_POL_REQ_IMGS);

	/* If No required images detected, return success */
	if (req_imgs_offset < 0) 
		return 0;

	/* Process all required image subnodes in order */
	for (ndepth = 0, count = 0, img_desc_offset = fdt_next_node(sec_pol_fit_ptr, req_imgs_offset, &ndepth);
		(img_desc_offset >= 0) && (ndepth > 0);
		img_desc_offset = fdt_next_node(sec_pol_fit_ptr, img_desc_offset, &ndepth)) {
		if (ndepth == 1) {
			char * node_name;
			int  size = 0;
			int  load_addr = 0;
			int  start_addr = 0;
			uint8_t* sha256_ptr;
			char * img_node_name;
			int fit_img_size = 0;      
			int fit_img_load_addr = 0; 
			int fit_img_start_addr = 0;
			uint8_t* fit_img_sha256_ptr;
			int i;

			/*
			 * Direct child node of the required_images parent node
			 */

			/* Get required image details from security policy */
			node_name  = (char*)fdt_getprop((void *)sec_pol_fit_ptr, img_desc_offset, DELG_REQ_IMG_NODE_NAME, NULL);
			get_fit_value((void *)sec_pol_fit_ptr, img_desc_offset, DELG_REQ_IMG_SIZE, &size);
			get_fit_value((void *)sec_pol_fit_ptr, img_desc_offset, DELG_REQ_IMG_LOAD_ADDR, &load_addr);
			get_fit_value((void *)sec_pol_fit_ptr, img_desc_offset, DELG_REQ_IMG_START_ADDR, &start_addr);
			sha256_ptr = (uint8_t*)fdt_getprop((void *)sec_pol_fit_ptr, img_desc_offset, DELG_REQ_IMG_SHA256, NULL);

			if( node_name && size && sha256_ptr ) {
				printf("INFO: Checking Reqd item:%s, load:0x%08x, start:0x%08x, size:0x%08x in %s\n",
					node_name, load_addr, start_addr, size,(verify_loaded_img?"memory":"FIT") );
			} else {
				printf("INFO: Invalid required image at index %d\n", count);
				count++;
				continue;
			}

			/* Get loadable image from FIT header */
			if( get_loadable_image_name(fit_hdr, count,  &img_node_name) ) {
				printf("ERROR: Cannot find required image %s in loadables list index %d!\n", 
					node_name, count);
				return -EINVAL;
			} 

			/* Compare loadable image name to required image name (load order matters) */
			if( strcasecmp(img_node_name, node_name) ) {
				printf("ERROR:     Required image order mismatch! Req[%d]=%s Actual[%d]=%s\n",
					count, node_name, count, img_node_name);
				return -EINVAL;
			}
			
			/* Get image node from FIT header */
			fit_img_offset = fdt_subnode_offset(fit_hdr, fdt_path_offset(fit_hdr, "/images"), img_node_name);
			if (fit_img_offset < 0) {
				printf("ERROR:     cannot find required image node '%s' in FIT: %d\n", img_node_name, fit_img_offset);
				return -EINVAL;
			}

			/* Get image details from FIT */
			get_fit_value((void *)fit_hdr, fit_img_offset, "data-size", &fit_img_size);
			get_fit_value((void *)fit_hdr, fit_img_offset, "load", &fit_img_load_addr);
			get_fit_value((void *)fit_hdr, fit_img_offset, "entry", &fit_img_start_addr);

			/* Check load addr, size, entry points */
			if( !verify_loaded_img ) {
				printf("INFO:     Found loadable[%d]:%s, load:0x%08x, start:0x%08x, size:0x%08x\n",
					count, img_node_name, fit_img_load_addr, fit_img_start_addr, fit_img_size);	

				if( fit_img_size != size || fit_img_load_addr != load_addr || fit_img_start_addr != start_addr ) {
					printf("ERROR:    Required image mismatch! \n");
					printf("          Req[%d]=%s size:0x%08x load:0x%08x start:0x%08x \n",
						count, node_name, size, load_addr, start_addr);
					printf("          FIT[%d]=%s size:0x%08x load:0x%08x start:0x%08x \n",
						count, img_node_name, fit_img_size, fit_img_load_addr, fit_img_start_addr);
					return -EINVAL;
				}

				/* Cache the first entry point we encounter. This is the address TPL will be handing off to */
				if( !delg_cfg->post_loader_entry_point )
					delg_cfg->post_loader_entry_point = start_addr;

			}

			/* Get hash value */
			if( verify_loaded_img ) {
				/* Get hash of loaded image in memory */
				printf("INFO:     Checking hash of loaded %s @ 0x%08x ... ", img_node_name, fit_img_load_addr);
				fit_img_sha256_ptr = malloc(SHA256_SUM_LEN);
				if( fit_img_sha256_ptr )
					bcm_sec_digest( (u8*)(uintptr_t)fit_img_load_addr, fit_img_size, (u8*)(uintptr_t)fit_img_sha256_ptr, "sha256");
			} else {
				/* Get image hash node from FIT header */
				printf("INFO:     Checking hash of %s in FIT ... ", img_node_name);
				fit_img_hash_offset = fdt_subnode_offset(fit_hdr, fit_img_offset, "hash-1");
				if (fit_img_hash_offset < 0) {
					printf("ERROR: cannot find required hash-1 node for image '%s' in FIT: %d\n", 
						img_node_name, fit_img_hash_offset);
					return -EINVAL;
				}
				fit_img_sha256_ptr = (uint8_t*)fdt_getprop((void *)fit_hdr, fit_img_hash_offset, "value", NULL);
			}

			/* Compare hashes */
			if( fit_img_sha256_ptr ) {
				for( i=0; i<SHA256_SUM_LEN; i++ ) {
					if( fit_img_sha256_ptr[i] != sha256_ptr[i] ) {
						printf("ERROR: required image '%s' hash mismatch!\n", img_node_name);
						return -EINVAL;
					}
				}
				printf("OK\n");
			} else {
				printf("ERROR: cannot get required sha256 value for image '%s' in %s\n", 
					img_node_name, (verify_loaded_img?"memory":"FIT"));
				return -EINVAL;
			} 

			if( !verify_loaded_img ) {
				/* Add checks to detect potential memory corruption */
			}

			/* Increment */
			count++;

			/* Free temp mem */
			if( verify_loaded_img )
				free(fit_img_sha256_ptr);
			
		}
	}
	/* Error out if we didnt find any required images */
	if( !count ) {
		printf("ERROR: Could not find any required images under %s/%s in security policy\n", 
		DELG_SEC_POL_SW_RESTRICT_PATH, DELG_SEC_POL_REQ_IMGS);
		return -EINVAL;
	}
	return 0;
}

int bcm_sec_delg_post_process_sw_restrictions( u8 * fit_hdr )
{
	/* Process required images */
	return( bcm_sec_delg_process_required_images(fit_hdr, 1));
}

int bcm_sec_delg_pre_process_sw_restrictions( u8 * fit_hdr )
{
	int ret = -EINVAL;

	/* Process debug groups */
	ret = bcm_sec_delg_process_debug_groups(fit_hdr);

	/* Process required images */
	if( !ret )
		ret = bcm_sec_delg_process_required_images(fit_hdr,0);
	return( ret );
}

static int bcm_sec_get_encoded_keys(u8* fit_hdr, int enc_keys_offset, 
				    bcm_sec_dec_key_type_t dec_key, int * total_key_count)
{
	int key_offset = -1;
	int count;
	int ndepth = 0;
	int * value = NULL;
	/* Process all image subnodes */
	for (ndepth = 0, count = 0,
	     key_offset = fdt_next_node(fit_hdr, enc_keys_offset, &ndepth);
			(key_offset >= 0) && (ndepth > 0);
			key_offset = fdt_next_node(fit_hdr, key_offset, &ndepth)) {
		if (ndepth == 1) {
			/*
			 * Direct child node of the encoded_keys parent node,
			 * i.e. key node.
			 */

			/* Get key name */
			bcm_sec_encoded_keys[*total_key_count].name = (char*)fdt_getprop((void *)fit_hdr, key_offset, DELG_ENC_KEYS_NAME, NULL);
			if (bcm_sec_encoded_keys[*total_key_count].name == NULL)
			{
				printf("INFO: Can't find %s parameter in key node under %s \n", DELG_ENC_KEYS_NAME,
					(dec_key==DEC_KEY_TYPE_AES_ROOT)?DELG_SEC_POL_ENC_KEYS_PATH:DELG_ENC_KEYS_PATH);
				continue;
			}
			printf("INFO: Found key %s in under %s\n", bcm_sec_encoded_keys[*total_key_count].name, 
				(dec_key==DEC_KEY_TYPE_AES_ROOT)?DELG_SEC_POL_ENC_KEYS_PATH:DELG_ENC_KEYS_PATH);

			/* Get unencrypted size */
			value = (int*)fdt_getprop((void *)fit_hdr, key_offset, DELG_ENC_KEYS_SIZE, NULL);
			if( value ) {
				bcm_sec_encoded_keys[*total_key_count].size  =  be32_to_cpu(*value); 
			} else {
				printf("INFO: Unencrypted size for key %s is not specified! Skipping Key!\n", 
					bcm_sec_encoded_keys[*total_key_count].name);
				continue;
			}
			
			/* Get keydata */
			value = (int*)fdt_getprop((void *)fit_hdr, key_offset, DELG_ENC_KEYS_DATA, (int*)&bcm_sec_encoded_keys[*total_key_count].size_enc);
			if( value && bcm_sec_encoded_keys[*total_key_count].size_enc ) {
				bcm_sec_encoded_keys[*total_key_count].data = (u8*)value;
			} else {
				printf("INFO: Key data for key %s not found! Skipping Key!\n", 
					bcm_sec_encoded_keys[*total_key_count].name);
				continue;
			}

			/* Get load address (optional) */
			value = (int*)fdt_getprop((void *)fit_hdr, key_offset, DELG_ENC_KEYS_LDADDR, NULL);
			if( value )
				bcm_sec_encoded_keys[*total_key_count].load_addr  =  be64_to_cpu(*value); 

			/* Get load permissions and enc algorithm (optional) */
			bcm_sec_encoded_keys[*total_key_count].perm = (char*)fdt_getprop((void *)fit_hdr, key_offset, DELG_EXPORT_PERM, NULL);
			bcm_sec_encoded_keys[*total_key_count].algo = (char*)fdt_getprop((void *)fit_hdr, key_offset, DELG_ENC_KEYS_ALGO, NULL);
			if( bcm_sec_encoded_keys[*total_key_count].algo ) {
				if(strncmp((const char*)(bcm_sec_encoded_keys[*total_key_count].algo), "aes-cbc-128", strlen("aes-cbc-128")) != 0) {
					printf("INFO: Unsupported encryption algorithm %s! Skipping Key!\n", bcm_sec_encoded_keys[*total_key_count].algo);
					continue;
				}
			}

			/* Get optional os_export flag. If flag is not present, decoded keys are NOT exported to Linux */
			value = (int*)fdt_getprop((void *)fit_hdr, key_offset, DELG_OS_EXPORT_FLAG, NULL);
			if (value && (strcasecmp((const char*)value, "yes") == 0))
				bcm_sec_encoded_keys[*total_key_count].exp_flag = 1;
			else
				bcm_sec_encoded_keys[*total_key_count].exp_flag = 0;

			/* Set decryption key */
			bcm_sec_encoded_keys[*total_key_count].dec_key = dec_key;

			/* Update local count */
			count++;

			/* Update total count */
			*total_key_count = *total_key_count + 1;

			if( *total_key_count >= BCM_SEC_MAX_ENCODED_KEYS ) {
				printf("INFO: Max encoded key limit of %d keys reached! Ignoring other keys!\n", BCM_SEC_MAX_ENCODED_KEYS);
				break;
			}
		}
	}
	/* Retun number of keys found in this invocation */
	return count;
}

static int bcm_sec_process_encoded_keys(u8 * fit_hdr, bcm_sec_cb_arg_t* key_args)
{
	int enc_keys_offset = -1;
	int num_keys_found = 0;
	u8* fit_ptr;
	bcm_sec_key_arg_t *enc_keys;
	bcm_sec_delg_cfg* delg_cfg = bcm_sec_get_delg_cfg();

	/* Process key-owners encoded keys in security node if present
	 * These keys are encrypted using ROE */
	if( delg_cfg && delg_cfg->delg_id ) 
	{
		fit_ptr = delg_cfg->sec_policy_fit;
		enc_keys_offset =  fdt_path_offset (fit_ptr, DELG_SEC_POL_ENC_KEYS_PATH);
		if (enc_keys_offset < 0) {
			debug("INFO: Could not find %s in security policy\n", DELG_SEC_POL_ENC_KEYS_PATH);
		} else {
			bcm_sec_get_encoded_keys(fit_ptr, enc_keys_offset, DEC_KEY_TYPE_AES_ROOT, &num_keys_found);
		}
	}

	/* Process delegates encoded keys in FIT header
	 * These keys are encrypted using the active AES key */
	enc_keys_offset =  fdt_path_offset (fit_hdr, DELG_ENC_KEYS_PATH);
	if (enc_keys_offset < 0) {
		debug("INFO: Could not find %s node in fit\n", DELG_ENC_KEYS_PATH);
	} else {
		bcm_sec_get_encoded_keys(fit_hdr, enc_keys_offset, DEC_KEY_TYPE_AES_DELG, &num_keys_found);
	}

	/* Add detected keys */
	if( num_keys_found ) {
		enc_keys = malloc(sizeof(bcm_sec_key_arg_t));
		if (!enc_keys) {
			printf("ERROR: Cannot allocate memory for encoded key ring!\n");
			return -1;
		}
		enc_keys->len = num_keys_found;
		enc_keys->enc_key = &bcm_sec_encoded_keys[0];
		key_args->arg[1].ctrl = SEC_CTRL_KEY_CHAIN_ENCKEY;
		key_args->arg[1].ctrl_arg = enc_keys;
	}
	return 0;
}

static int bcm_sec_delg_process_policy(u8 * policy_fdt, u8 * fit_hdr )
{
	u32 * value;
	u32 pol_delg_id;
	u32 min_tpl_version; 
	char * key_enc_alg;
	u8 * key_data;
	int key_len;
	int nodeoffset = 0;
	bcm_sec_delg_cfg * delg_cfg = NULL;
	int ret = -1;

	/* Get delegation config */ 
	delg_cfg = bcm_sec_get_delg_cfg();
	if( !delg_cfg ) {
		printf("ERROR: Delegation configuration not found!\n");
		return ret;
	}

	/* Get security policy node */
	nodeoffset = fdt_path_offset((void *)policy_fdt, DELG_SEC_POL_COMPAT_PATH);
	if( nodeoffset < 0 ) {
		nodeoffset = fdt_path_offset((void *)policy_fdt, DELG_SEC_POL_PATH);
		if( nodeoffset < 0 ) {
			printf("ERROR: %s & %s node not found in security policy!\n", DELG_SEC_POL_COMPAT_PATH,
				DELG_SEC_POL_PATH);
			return ret;
		} else {
			printf("INFO: Legacy Security Policy Format detected. Consider upgrading to latest Security Policy Format\n");
		}
	}

	/* Get delegate id */
	value = (u32*)fdt_getprop((void *)policy_fdt, nodeoffset, DELG_SEC_POL_DELGID, NULL);
	if (value == NULL)
	{
		printf("ERROR: Can't find %s parameter in %s\n", DELG_SEC_POL_DELGID,
			DELG_SEC_POL_PATH);
		return ret;
	}
	pol_delg_id =  be32_to_cpu(*value); 
	if( pol_delg_id != delg_cfg->delg_id ) {
		printf("ERROR: delegate ID mismatch! %d != %d !\n", delg_cfg->delg_id, pol_delg_id);
		return ret;
	}
	
	/* get min tpl version */
	value = (u32*)fdt_getprop((void *)policy_fdt, nodeoffset, DELG_SEC_POL_MINTPLV, NULL);
	if (value == NULL)
	{
		printf("ERROR: Can't find %s parameter in %s\n", DELG_SEC_POL_MINTPLV,
			DELG_SEC_POL_PATH);
		return ret;
	}

	min_tpl_version =  be32_to_cpu(*value); 
	if (min_tpl_version > THIS_TPL_COMPAT_VERSION)
	{
		printf("ERROR: Delegation requires min_tpl_compatibility %x versus our compatibility %x\n",
			min_tpl_version, THIS_TPL_COMPAT_VERSION);
		return ret;
	}

	/* 1 - Get delegated AES key (MANDATORY) */
	nodeoffset = fdt_path_offset((void *)policy_fdt, DELG_SEC_POL_AES_KEY_PATH);
	if( nodeoffset < 0 ) {
		printf("ERROR: %s node not found in bundle!\n", DELG_SEC_POL_AES_KEY_PATH);
		return ret;
	}

	/* Get encryption algorithm */
	value = (u32*)fdt_getprop((void *)policy_fdt, nodeoffset, DELG_SEC_POL_AES_KEY_ENC_ALGO, NULL);
	if (value == NULL)
	{
		printf("ERROR: Can't find %s parameter in %s\n", DELG_SEC_POL_AES_KEY_ENC_ALGO,
			DELG_SEC_POL_AES_KEY_PATH);
		return ret;
	}

	/* Check encryption algorithm */
	key_enc_alg = (char*)value;
	if(strncmp((const char*)(key_enc_alg), "aes-cbc-128", strlen("aes-cbc-128")) != 0) {
		printf("ERROR: Unsupported encryption algorithm %s\n", key_enc_alg);
		return ret;
	}

	/* Get key data */
	value = (u32*)fdt_getprop((void *)policy_fdt, nodeoffset, DELG_SEC_POL_AES_KEY_DATA, &key_len);
	if (value == NULL)
	{
		printf("ERROR: Can't find %s parameter in %s\n", DELG_SEC_POL_AES_KEY_DATA,
			DELG_SEC_POL_AES_KEY_PATH);
		return ret;
	}
	printf("INFO: Found potential Encrypted AES Key: KAES:0x%08x len:%d\n",
		*(u32*)value, key_len); 

	/* Decypt delegates aes key using ROE */
	u8* root_aes_key = NULL;
	u8* iv;

	/* retrieve root key */
	bcm_sec_get_root_aes_key(&root_aes_key);
	if(!root_aes_key) {
		printf("ERROR: Could not retrieve root aes key!\n");
		return -1;
	}
	iv = (u8*)root_aes_key + AES128_KEY_LENGTH;

	/* Allocate space for decrypted delegated encryption key */
	key_data = malloc(key_len);
	if (!key_data) {
		printf("ERROR: Cannot allocate memory for decoded delegated key!\n");
		return -1;
	}
	memcpy(key_data, (u8*)value, key_len); 

	/* Decrypt delegated encryption key */
	bcm_sec_aes_cbc128(root_aes_key, iv, key_data, key_len,0);

	/* Copy key to our delg config */
	memcpy(delg_cfg->aes_ek, (void*)key_data, 
			BCM_SECBT_AES_CBC128_EK_LEN); 
	memcpy(delg_cfg->aes_ek + BCM_SECBT_AES_CBC128_EK_LEN,  
		(void*)(key_data+BCM_SECBT_AES_CBC128_EK_LEN), BCM_SECBT_AES_CBC128_EK_LEN);
	free(key_data);
	key_data = NULL;

#if BCM_SEC_EXPOSE_SECRET_KEYS	
	/* ### WARNING: ENABLING THIS DEBUG WILL EXPOSE DECRYPTED DELG AES KEYS ### */
	printf("INFO: Decrypted AES_EK: 0x%08x AES_IV: 0x%08x\n",
		*(u32*)delg_cfg->aes_ek, 
		*(u32*)(delg_cfg->aes_ek + BCM_SECBT_AES_CBC128_EK_LEN));
	printf("%s: ciphertxt:", "delg_aes");
	{ 
		int j;
		for( j=0; j<key_len; j+=1)
		{
			printf("%02x",  *((u8*)value+j));
		}
	}
	printf("\n%s:  plaintxt:", "delg_aes");
	{ 
		int j;
		for( j=0; j<BCM_SECBT_AES_CBC128_EK_LEN*2; j+=1)
		{
			printf("%02x",  *((u8*)delg_cfg->aes_ek+j));
		}
	}
	printf("\n");
#endif	

	bcm_sec_set_active_aes_key(delg_cfg->aes_ek);

	/* Save our pointer to our policy for later use */
	delg_cfg->sec_policy_fit = policy_fdt;

	/* Handle sw restrictions */
	ret = bcm_sec_delg_pre_process_sw_restrictions( fit_hdr);
	if( ret ) {
		printf("ERROR: Pre FIT load sw restrictions check failed!\n");
	}
	return ret;

}

int bcm_sec_delg_process_sec_node(u8 * fit)
{
	size_t  size = 0;
	int nodeoffset = 0;
	ulong * sec_policy_ptr = NULL;
	u8 * sig_sec_fit_delg = NULL;
	int sig_len = 0;
	int ret = -1;
	u8* krot_pub = bcm_sec_get_root_pub_key();
	struct image_sign_info im;
	
	/* Setup signing structures */
	im.checksum = image_get_checksum_algo("sha256,");
	if (!im.checksum) {
		printf("ERROR: couldn't get checksum algo\n"); 
		return ret;
	}

	nodeoffset = fdt_path_offset((void *)fit, DELG_SEC_FIT_NODE_PATH);
	if( nodeoffset < 0 ) {
		printf("ERROR: %s node not found in FIT!\n", DELG_SEC_FIT_NODE_PATH);
		return ret;
	}
	
	/* Get security policy dtb */
	ret = fit_image_get_data_and_size((void *)fit, nodeoffset, (const void**)&sec_policy_ptr, &size);
	if( ret || !size ) {
		printf("ERROR: sec policy data not found!\n");
		return ret;
	}

	/* Get signature */
	sig_sec_fit_delg = (u8*)fdt_getprop( (void *)fit, nodeoffset, DELG_SEC_FIT_NODE_SIG, &sig_len);
	if( !sig_sec_fit_delg) {
		printf("ERROR: %s not found in %s !\n", DELG_SEC_FIT_NODE_SIG,
			 DELG_SEC_FIT_NODE_PATH);
		return ret;
	}

	/* Verify signature */
	printf("INFO: Found potential Security Node: Policy:0x%08x Sig:0x%08x Size:%d\n",
		*(u32*)sec_policy_ptr, *(u32*)sig_sec_fit_delg, (u32)size);

	ret = bcm_sec_rsa_verify((u8*)sec_policy_ptr, 
				size, 
				sig_sec_fit_delg,  
				RSA2048_BYTES, krot_pub, &im );

	if( ret == 0 ) {
		printf("INFO: Security Node Authentication Successfull!\n");
		/* Process policy */
		ret = bcm_sec_delg_process_policy((u8*)sec_policy_ptr, fit);
	} else {
		printf("ERROR: Security Node Authentication FAILS!");
		ret = -1;
	}
	return ret;

}

int bcm_sec_delg_process_sdr( u8 * psdr, u8 * hdr_end, u32 * sdr_plus_sig_size)
{
	int rc = -1;
	u32 lvl = 0;
	u32 delegateId = 0;
	u32 max_anti_rollback_lvl = 0;
	u8* sdr_start = psdr;
	unsigned long long sdr_size = 0;
	u8* sig_sec_rec_delg = NULL;
	u8* krsa_delg_pub = NULL;
	u8* krot_pub = bcm_sec_get_root_pub_key();
	struct image_sign_info im;
	bcm_sec_delg_cfg * delg_cfg = NULL;
	
	/* Setup signing structures */
	im.checksum = image_get_checksum_algo("sha256,");
	if (!im.checksum) {
		printf("ERROR: couldn't get checksum algo\n"); 
		goto err;
	}

	/* Retrieve Delegate ID. Note that legacy SDR has delegate ID
	 * as the first word after the SDR tag. Non-legacy SDR have 
	 * a '0x0' first word, followed by the delegate id */
	psdr += sizeof(u32);
	delegateId = *(u32*)psdr;
	if( !delegateId ) {
		psdr += sizeof(u32);
		delegateId = *(u32*)psdr;
		if( !delegateId ) {
			printf("ERROR: Invalid Delegate ID %d\n", delegateId);
			goto err;
		}
	} else {
		printf("INFO: Legacy SDR detected. Consider upgrading to latest SDR format\n");
	}


	/* Parse remaining fields */
	psdr += sizeof(u32);
	max_anti_rollback_lvl = *(u32*)psdr;
	psdr += sizeof(u32);
	krsa_delg_pub = psdr;
	psdr += RSA2048_BYTES;
	sig_sec_rec_delg = psdr;
	sdr_size = (unsigned long long )(psdr - sdr_start); // Check this
	*sdr_plus_sig_size = sdr_size + RSA2048_BYTES;

	printf("INFO: Found potential SDR: delg_id:%d maxrollbck:%d size:%llu\n", 
		delegateId, max_anti_rollback_lvl, sdr_size);

	printf("INFO: Found potential SDR: PubKey:0x%08x Sig:0x%08x\n",
		*(u32*)krsa_delg_pub, *(u32*)sig_sec_rec_delg);
	
	/* Check if SDR + signature have crossed our search boundary */
	if ( ((u8*)psdr + *sdr_plus_sig_size-1) > hdr_end ) {
		printf("ERROR: Corrupted SDR, search exceeds boundary 0x%p > 0x%p\n",
			(u8*)psdr + RSA2048_BYTES-1, hdr_end);
		goto err;
	}
	
	/* Check if anti-rollback is valid */
	rc = bcm_sec_get_antirollback_lvl(&lvl);
	if(  (lvl > max_anti_rollback_lvl) || rc ) {
		printf("ERROR: Expired delegated credentials detected curr(%d) > max(%d) rc:%d\n", 
			lvl, max_anti_rollback_lvl, rc);
		rc = -1;
		goto err;
	}
	
	
	/* Authenticate sdr signature */
	rc = bcm_sec_rsa_verify(sdr_start, 
				sdr_size, 
				sig_sec_rec_delg,  
				RSA2048_BYTES, krot_pub, &im );
	if( rc == 0 ) {
		printf("INFO: SDR Authentication Successfull!\n");
		/* Commit all fields to our structures */
		delg_cfg = malloc(sizeof(bcm_sec_delg_cfg)); 
		if (!delg_cfg) {
			printf("ERROR: Cannot allocate memory for delegation config!\n");
			rc = -1;
			goto err;
		}
		memset(delg_cfg, 0, sizeof(bcm_sec_delg_cfg));
		delg_cfg->delg_id = delegateId;
		delg_cfg->max_antirollback = max_anti_rollback_lvl;
		memcpy(delg_cfg->rsa_pub, krsa_delg_pub, BCM_SECBT_RSA2048_MOD_LEN);

		/* Commit delegation config */
		bcm_sec_set_delg_cfg( delg_cfg );

		/* Set active public key */
		bcm_sec_set_active_pub_key( (u8*)delg_cfg->rsa_pub );

	} else {
		printf("ERROR: SDR Authentication FAILS!\n");
		rc = -1;
	}

err:
	return rc;
}

/* returns heap allocated key*/
static int fdt_key_decrypt(void* fit, int node, const char* prop, 
			const u8* ek_iv,  u8* aes)
{
	int rc = -1, len = 0;
	assert(BCM_SECBT_AES_CBC128_EK_LEN == AES128_KEY_LENGTH);
	const u8* key = fdt_getprop(fit, node, prop, &len);
	if (!key || ((len-1)%(AES128_KEY_LENGTH*2))) {
		if (key) {
			printf("Error: Invalid size for %s must be at least %d including iv \n",prop ,AES128_KEY_LENGTH*2);
		}
		goto err;
	}
	if (bcm_util_hex2u32((const char*)key, aes) < 0) {
		goto err;
	}
	bcm_sec_aes_cbc128((u8*)ek_iv, (u8*)ek_iv + AES128_KEY_LENGTH, aes, (len-1)/2, 0);
	rc  = 0;
err: 
	return rc;
}

static int bcm_sec_fit_key_chain(void* fit, int node, bcm_sec_key_arg_t** aes )
{
	int rc = -1, len = 0;
	u8* ek_iv = NULL;
	bcm_sec_key_aes_arg_t* _aes = NULL;
	bcm_sec_key_arg_t* aes_arg = NULL;
	char *_keys_prop[2] = {FIT_AES1, FIT_AES2};
	bcm_sec_get_active_aes_key(&ek_iv);
	if (!ek_iv)   {
		goto err;
	}
	aes_arg = malloc(sizeof(bcm_sec_key_arg_t));
	_aes    = malloc(sizeof(bcm_sec_key_aes_arg_t)*2);
	if (!aes_arg || !_aes) {
		goto err;
	}
	memset(aes_arg, 0, sizeof(bcm_sec_key_arg_t));
	memset(_aes,    0, sizeof(bcm_sec_key_aes_arg_t)*2);
	aes_arg->aes = _aes;

	if (fdt_key_decrypt(fit, node, _keys_prop[0], ek_iv,  _aes[0].key)) {
		free(_aes);
		free(aes_arg);
		goto err;
	}
	strcpy(_aes[0].id, _keys_prop[0]);
	aes_arg->len++;
	if (!fdt_key_decrypt(fit, node, _keys_prop[1], _aes[0].key,  _aes[1].key)) {
		if (fdt_getprop(fit, node, "dont-flush-aes1", &len)) {
		/* will chain aes1-key along with dev key */
			strcpy(_aes[1].id, _keys_prop[1]);
			aes_arg->len++;
		} else {
			strcpy(_aes[0].id, _keys_prop[1]);
			memcpy(_aes[0].key, _aes[1].key, AES128_KEY_LENGTH*2);	
			memset(_aes[1].key,  0, AES128_KEY_LENGTH*2);
		}
	}
	*aes = aes_arg;
	rc  = 0;
err:
	return rc;
}

static int bcm_sec_set_sotp_hw_state(u8* fit, int node, bcm_sec_cb_arg_t* sotp_st_arg) {

	int * value = NULL;
	int len;

	/* Set sotp hardware state */
	value = (int*)fdt_getprop(fit, node, "sotp-lock", &len);
	if( value ) {
		if (strncmp((const char*)value, "all", 3) == 0) {
			/* Lock access for all masters */
			sotp_st_arg->arg[0].ctrl = SEC_CTRL_SOTP_LOCK_ALL;
		} else if( strncmp((const char*)value, "none", 4) == 0 ) {
			/* Lock access for none ( unlocked for all masters ) */
			sotp_st_arg->arg[0].ctrl = SEC_CTRL_SOTP_UNLOCK_SOTP_UNSEC;
		} else if( strncmp((const char*)value, "prov", 4) == 0 ) {
			/* Provisioning mode, Lock access for none, lock non-zero rows */
			sotp_st_arg->arg[0].ctrl = SEC_CTRL_SOTP_UNLOCK_SOTP_UNSEC_PROV;
		} else {
			/* Lock access for non secure masters only ( unlocked for secure masters only )*/
			sotp_st_arg->arg[0].ctrl = SEC_CTRL_SOTP_UNLOCK_SOTP_SEC;
		}
	}
	return 0;
}

static int bcm_sec_set_rng_hw_state(u8* fit, int node, bcm_sec_cb_arg_t* rng_st_arg) {

	int * value = NULL;
	int len;

	/* Set rng hardware state */
	value = (int*)fdt_getprop(fit, node, "rng-lock", &len);
	if( value ) {
		if(strncmp((const char*)value, "all", 3) == 0) {
			/* Lock access for all masters */
			rng_st_arg->arg[0].ctrl = SEC_CTRL_RNG_LOCK_ALL;
		} else if( strncmp((const char*)value, "none", 4) == 0 ) {
			/* Lock access for none ( unlocked for all masters ) */
			rng_st_arg->arg[0].ctrl = SEC_CTRL_RNG_UNLOCK_RNG_UNSEC;
		} else {
			/* Lock access for non secure masters only ( unlocked for secure masters only )*/
			rng_st_arg->arg[0].ctrl = SEC_CTRL_RNG_UNLOCK_RNG_SEC;
		}
	}
	return 0;
}

static int bcm_sec_chk_loaded_bin(u8* fit_addr, char* bin_name)
{
	const char *name;
	int conf_node;
	int len;
	char * str;
	char *str_end;
	int found=0;

	conf_node = fit_find_config_node(fit_addr);
	if (conf_node < 0) {
		printf("No matching DT out of these options:\n");
	}

	name = fdt_getprop(fit_addr, conf_node, "loadables", &len);
	if (!name) {
		debug("cannot find property '%s': %d\n", "loadables", len);
	}

	/* Check if bin is part of the 'loadables' */
	str = (char*)name;
	str_end = (char*)((uintptr_t)name + len);
	while( str ) {
		if (strcasecmp((const char*)str, bin_name) != 0) {
			/* loadables is \0 separated list of strings */
			str = memchr(str, '\0', (int)((uintptr_t)str_end - (uintptr_t)str));
			if(str) {
				str++;
			} else {
				break;
			}
		} else {
			found=1;
			break;
		}
	}

	return found;
}

void bcm_sec_delg_on_post_load(u8* load_addr)
{

	int ret = -1;
	bcm_sec_delg_cfg * delg_cfg = NULL;
	/* Get delegation status */
	delg_cfg = bcm_sec_get_delg_cfg();

	/* Check if loaded images violate any sw restrictions in security policy */
	if( delg_cfg && delg_cfg->delg_id ) {
		ret = bcm_sec_delg_post_process_sw_restrictions( (u8*)load_addr );
		if( ret ) {
			printf("ERROR: Post FIT load sw restrictions check failed!\n");
			bcm_sec_abort();
		}
	}

	/* Cache loaded binaries */
	tee_loaded = bcm_sec_chk_loaded_bin(load_addr, "optee");
	armtf_loaded = bcm_sec_chk_loaded_bin(load_addr, "atf");
	
	/* Store final fit load address */
	fit_load_addr = load_addr;
}

static int bcm_sec_delg_scrub_secure_items( void *fdt_addr )
{
        int ret=0;
        int len=0;
        char * value;
        int trust_offset, trust_item_offset;
        char * node_name;
        int ndepth = 0;

        trust_offset = fdt_path_offset(fdt_addr, "/trust");
        if (trust_offset < 0) {
                debug("INFO: Didnt find /trust node in DTB\n");
                return ret;
        }

        for (trust_item_offset = fdt_next_node(fdt_addr, trust_offset, &ndepth); (trust_item_offset >= 0) && (ndepth > 0);)  {
                if (ndepth == 1) {
                                /*
                                 * Direct child node of the trust parent node,
                                 * i.e. item node.
                                 */
                        node_name = (char*)fdt_get_name(fdt_addr, trust_item_offset, &len);
                        value = (char*)(fdt_getprop(fdt_addr, trust_item_offset, "permission", &len));
			if (value && (strcasecmp((const char*)value, "secure") == 0)){
				/* secure item --> scrub it */
				printf("INFO: Scrubbing secure node /trust/%s\n", node_name);
				fdt_del_node(fdt_addr, trust_item_offset);

				/* Search from start of /trust node as offsets have changed */
				ndepth = 0;
				trust_item_offset = fdt_next_node(fdt_addr, trust_offset, &ndepth);
			} else {
				trust_item_offset = fdt_next_node(fdt_addr, trust_item_offset, &ndepth);
			}
                }
        }
        return ret;
}

void bcm_sec_delg_pre_launch_fixup(void* spl_im)
{
	u32 compat_ver;
	int ofs;
	struct spl_image_info* spl_image = (struct spl_image_info*)spl_im;
	bcm_sec_delg_cfg * delg_cfg = NULL;
	bcm_sec_ctrl_arg_t arg = {.ctrl = SEC_CTRL_KEY_CHAIN_RSA, 
				.ctrl_arg = spl_image->fdt_addr} ;
	bcm_sec_update_ctrl_arg(&arg, SEC_CTRL_ARG_KEY);

	/* Get delegation status */
	delg_cfg = bcm_sec_get_delg_cfg();

	/* Check if delegation is active inorder to pass encoded keys */
	if( delg_cfg && delg_cfg->delg_id ) {
		arg.ctrl = SEC_CTRL_KEY_CHAIN_ENCKEY;
	} else {
		arg.ctrl = SEC_CTRL_KEY_CHAIN_AES;
	}
	bcm_sec_update_ctrl_arg(&arg, SEC_CTRL_ARG_KEY);

	/* Pass export items */
	arg.ctrl = SEC_CTRL_KEY_EXPORT_ITEM;
	bcm_sec_update_ctrl_arg(&arg, SEC_CTRL_ARG_KEY);
	bcm_sec_do(SEC_SET, NULL);

	/* Compare entrypoint to security policy sw restrictions entrypoint */
	if( delg_cfg && delg_cfg->delg_id ) {
		if( delg_cfg->post_loader_entry_point ) {
			if( spl_image->entry_point != delg_cfg->post_loader_entry_point ) {
				printf("ERROR: Invalid entrypoint:0x%lx! reqd entrypoint from sec policy:0x%08x\n",
					spl_image->entry_point, delg_cfg->post_loader_entry_point);
				bcm_sec_abort();
			} else {
				printf("INFO: Post loader entry point 0x%08lx is valid!\n", spl_image->entry_point);
			}
		}

		/* Check for a TEE and scrub secure items if no TEE is found */
		if( !tee_loaded ) {
			printf("INFO: No TEE detected, scrubbing secure items!\n");
			bcm_sec_delg_scrub_secure_items(spl_image->fdt_addr);
		}
	}
	
	/* Add min tpl compat version to loader_info */
 	ofs = fdt_path_offset (spl_image->fdt_addr, "/chosen/loader_info");
	if(ofs >= 0) {
		compat_ver = cpu_to_be32(THIS_TPL_COMPAT_VERSION);
		if(fdt_setprop(spl_image->fdt_addr, ofs, "tpl_min_compat", (u8*)&compat_ver,sizeof(u32)))
			printf("ERROR: Could not set tpl compatibility level info\n");
	} else {
		printf("ERROR: Could not find loader_info node!\n");
	}
}

void bcm_sec_state_to_fdt(void* fdt)
{
 	int ofs = fdt_path_offset (fdt, "/chosen");
	if(ofs >= 0) {
		if(fdt_setprop_u32(fdt, ofs, "brom_sec_state", bcm_sec()->state)) {
			printf("Could not set sec_state\n");
		}
	}
}

int bcm_sec_fit(void* fit)
{
	bcm_sec_key_arg_t *aes;
	int node = -1;
/*trust/dont-lock-sotp ==>  otherwise, lock SOTP before passing control

trust/dont-flush-aes1 ==> otherwise, wipe out the aes key before passing control
trust/aes1-key = hex-string ==> use original AES key to decrypt this string, wipe the original key,*/ 
	bcm_sec_cb_arg_t cb_args[SEC_CTRL_ARG_MAX] = {0}; 
	bcm_sec_cb_arg_t* karg = &cb_args[SEC_CTRL_ARG_KEY];
	bcm_sec_cb_arg_t* sarg = &cb_args[SEC_CTRL_ARG_SOTP];
	bcm_sec_cb_arg_t* rarg = &cb_args[SEC_CTRL_ARG_RNG];
	bcm_sec_delg_cfg* delg_cfg = bcm_sec_get_delg_cfg();

	/* Chain keys if required */
	/* when enumerated later 0 - will be run first
 	* 1- second and so on.for both s_ctrl and k_ctrl 		
 	*/
	karg->arg[0].ctrl = SEC_CTRL_KEY_CHAIN_RSA;
	karg->arg[3].ctrl = SEC_CTRL_KEY_CLEAN_ALL;
	
	if( delg_cfg && delg_cfg->delg_id ) {
		/* If delegated credentials are in play then do not pass aes1/aes2 
		 * and instead process encoded keys in FIT and security node */
		bcm_sec_process_encoded_keys(fit, karg);
	} else  {
		node =  fdt_path_offset (fit, "/trust");
		if (node < 0) {
			/* No trust node */
			goto done;
		}
		/* aes1  aka aes1 
		*  aes2 gets decrypted by aes1, aes1 gets decrypted by ROE aka truste bootrom key 
		* 	*/ 
		if (!bcm_sec_fit_key_chain(fit, node, &aes)) {
			karg->arg[1].ctrl = SEC_CTRL_KEY_CHAIN_AES;
			karg->arg[1].ctrl_arg = aes;
		}
	}

	/* Configure anti-rollback levels */
	bcm_sec_process_antirollback(fit);

	/* Export secure items */
	bcm_sec_process_exports(fit, karg);

done:
	/* Set hw states */
	bcm_sec_process_hw_state(fit, sarg, rarg);

	bcm_sec_do(SEC_SET_SCHED, cb_args);
	return 0;

}
