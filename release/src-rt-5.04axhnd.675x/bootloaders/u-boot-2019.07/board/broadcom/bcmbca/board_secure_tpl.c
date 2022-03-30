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
#include <spl.h>
#include "tpl_params.h"
#include "spl_env.h"
#include "bcm_secure.h"
#include "bcm_otp.h"
#include "bcm_rng.h"
#include <asm/arch/rng.h>
#include "u-boot/rsa.h"
#include "u-boot/rsa-mod-exp.h"
#include <u-boot/sha256.h>
#include <uboot_aes.h>
#include <watchdog.h>
#include "mini-gmp/mini-gmp.h"
#include "mini-gmp/mini-mpq.h"

#define NODE_NAME_LEN			128
#define BCM_SEC_MAX_ENCODED_KEYS	10
#define BCM_SEC_MAX_EXPORTED_ITEMS	5
#define DELG_SEC_FIT_NODE_PATH 		"/security"
#define DELG_SEC_FIT_NODE_SIG		"signature"
#define DELG_SEC_POL_PATH		"/security_policy"
#define DELG_SEC_POL_DELGID		"delegate_id"
#define DELG_SEC_POL_MINTPLV		"min_tpl_compatibility"
#define DELG_SEC_POL_AES_KEY_PATH	"/security_policy/key-aes"
#define DELG_SEC_POL_AES_KEY_ENC_ALGO	"algo"
#define DELG_SEC_POL_AES_KEY_DATA	"data"
#define DELG_SEC_POL_ANTI_ROLLBACK_PATH "/security_policy/anti-rollback"
#define DELG_SEC_POL_SEC_RESTRICT_PATH  "/security_policy/sec_restriction"
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
#define DELG_HW_STATE_PATH		"/trust/hw_state"
#define DELG_ANTI_ROLLBACK_PATH 	"/trust/anti-rollback"
#define DELG_ENC_KEYS_PATH		"/trust/encoded_keys"
#define DELG_ENC_KEYS_NAME		"key_name"
#define DELG_ENC_KEYS_PERM		"permission"
#define DELG_ENC_KEYS_SIZE		"size"
#define DELG_ENC_KEYS_DATA		DELG_SEC_POL_AES_KEY_DATA
#define DELG_ENC_KEYS_LDADDR		"load_addr"
#define DELG_ENC_KEYS_ALGO		DELG_SEC_POL_AES_KEY_ENC_ALGO
#define DELG_ENC_KEYS_PERMSEC		"secure"
#define DELG_ENC_KEYS_PERMNONSEC	"nonsecure"
#define DELG_TRUST_NODE_PATH		"/trust"
#define DELG_TRUST_NODE			"trust"
#define DELG_SEC_POL_SEC_EXPORT_PATH	"/security_policy/sec_restriction/sec_exports"
#define DELG_SEC_ALLOWED_EXPORTS        "allowed_exports"
#define DELG_SEC_EXPORT_PATH            "/trust/sec_exports"
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

static int sec_rng_ctrl(bcm_sec_t* sec, bcm_sec_ctrl_t ctrl, void* arg)
{
	switch(ctrl) {
	
        case SEC_CTRL_RNG_LOCK_ALL: 
		rng_pac_lock(RNG_PERM_DISABLE_ALL);
		break;
	
        case SEC_CTRL_RNG_UNLOCK_RNG_UNSEC: 
		rng_pac_lock(RNG_PERM_NSEC_ENABLE);
		break;
        case SEC_CTRL_RNG_UNLOCK_RNG_SEC: 
		rng_pac_lock(RNG_PERM_SEC_ENABLE);
		break;
	default:
		break;
	}
	return 0;
}

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

	while( exp_item_otp_map[i].item_id ) {
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

	return rc;
}

static int sec_salt_hash_item( u8 * item_src, int size_src, 
		u8* item_dest, int size_dst,
		char * algo, int salt)
{
	char * temp_buff;
	char * buf_ptr;
	char * dst_ptr;

	/* Allocate temp mem of src size + size of 32-bit salt */
	temp_buff = malloc(size_src + sizeof(int));
	if (!temp_buff) {
		printf("ERROR: Cannot allocate memory tempbuff\n");
		return -1;
	} 

	/* Init buffer pointers */
	buf_ptr = (char*)temp_buff;
	dst_ptr = (char*)item_dest;

	//TODO: Support other algorithms rather than just sha256
	if( (size_dst % SHA256_SUM_LEN) || size_dst > 2*SHA256_SUM_LEN ) {
		printf("ERROR: Unsupported final item size %d for hashing!\n", size_dst);
		return -1;
	}
	
	/* First pass, get first 256bits of obfuscated item data */
	if( salt ) {
		memcpy(buf_ptr, &salt, sizeof(int));
		buf_ptr += sizeof(int);
	}
	memcpy(buf_ptr, item_src, size_src);

	//TODO: Handle other algorithms, hardcoding to sha256
	bcm_sec_digest( (u8*)temp_buff, size_src + sizeof(int), (u8*)dst_ptr, "sha256");
	dst_ptr += SHA256_SUM_LEN;

	/* 2nd pass, get 2nd 256bits of obfuscated item data, reverse order of salting */
	if( size_dst > SHA256_SUM_LEN ) {
		buf_ptr = temp_buff;
		memcpy(buf_ptr, item_src, size_src);
		buf_ptr += size_src;
		if( salt ) {
			memcpy(buf_ptr, &salt, sizeof(int));
			buf_ptr += sizeof(u32);
		}
		//TODO: Handle other algorithms, hardcoding to sha256
		bcm_sec_digest((u8*)temp_buff, size_src + sizeof(int), (u8*)dst_ptr, "sha256");
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
			printf("INFO: Cannot retrieve export item %s! Skipping!\n", item->name);
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
			ret = sec_salt_hash_item( (u8*) pdata, data_size, temp_buff, item->len, item->algo, item->salt );
			if( ret ) {
				printf("INFO: Cannot salt/hash item %s! Skipping!\n", item->name);
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
err:
	if( temp_buff )
		free(temp_buff);
	
	return ret;
}

static int sec_add_decoded_key_fdt( u8 * fdt, char* key_name, u8 * key_val, int key_len)
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
	snprintf(key_node_name,NODE_NAME_LEN,"key_%s", key_name);
	node = sec_add_fdt_node( fdt, DELG_TRUST_NODE_PATH, key_node_name);
	if( node < 0 ) {
		printf("ERROR: Could not add %s node to %s!\n",
			key_node_name, DELG_TRUST_NODE_PATH);
		return -1;
	}

	printf("INFO: Adding exported decoded key node %s to dtb, size:%d\n",key_node_name, key_len);
	ret = fdt_setprop(fdt, node, "value", key_val, key_len); 
	if( ret )
		printf("ERROR: Could net set value for %s/%s\n",DELG_TRUST_NODE_PATH,key_node_name);

	return ret;
}

static int sec_key_ctrl(bcm_sec_t *sec, bcm_sec_ctrl_t ctrl, void * arg)
{
	int rc = 0;
	u8* ek_iv = NULL;
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
			item.salt = 0;
			item.algo = NULL;
			item.exp_flag = 1;

			bcm_sec_get_antirollback_lvl(&lvl);
			lvl = cpu_to_be32(lvl);
			item.name = "antirollback_lvl";
			item.len = sizeof(u32);
			item.value = (u8*)&lvl;
			rc = sec_add_export_item_fdt(arg, &item);

			if( rc == 0 ) {
				item.name = "brcm_pub_key";
				item.len = RSA2048_BYTES;
				item.value = sec->key.pub;
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
			bcm_sec_key_arg_t* sec_arg = arg;
			for (i = 0; i<sec_arg->len; i++) { 
				/* Decrypt encoded key */
				bcm_sec_get_active_aes_key(&ek_iv);
				/* FIXME: Compare un-enc and enc sizes to figure out malloc length ( we are assuming enc > un-enc ) */
				key_data = malloc(sec_arg->enc_key[i].size_enc);
				if (!key_data) {
					printf("ERROR: Cannot allocate memory for decoded key size %d!\n", sec_arg->enc_key[i].size_enc);
					return -1;
				}
				memcpy(key_data, sec_arg->enc_key[i].data, sec_arg->enc_key[i].size_enc);
				bcm_sec_aes_cbc128((u8*)ek_iv, (u8*)ek_iv + AES128_KEY_LENGTH, key_data, sec_arg->enc_key[i].size_enc,0);
				debug("KEYDECRYPT: encr:0x%08x key:0x%08x decr:0x%08x\n", *(u32*)sec_arg->enc_key[i].data, *(u32*)ek_iv, *(u32*)key_data);

				/* If no permissions are set, assume secure key */
				if( !sec_arg->enc_key[i].perm ||
					(strcasecmp(sec_arg->enc_key[i].perm, DELG_ENC_KEYS_PERMSEC) == 0) ) {
					/* For secure keys see if there is a load address */
					if( sec_arg->enc_key[i].load_addr ) {
						printf("INFO: copying key %s to %p\n", 
							sec_arg->enc_key[i].name,(void*)sec_arg->enc_key[i].load_addr); 
						printf("INFO: copying to load addr disabled for now!\n");
						//memcpy((void*)sec_arg->enc_key[i].load_addr,	key_data, sec_arg->enc_key[i].size);
					} else {
						printf("INFO: No load address specified for secure key %s! Ignoring\n", sec_arg->enc_key[i].name);
					}
				} else {
					/* For non-secure keys add them to dtb */
					sec_add_decoded_key_fdt(sec_arg->arg, 
						sec_arg->enc_key[i].name, 
						key_data,
						sec_arg->enc_key[i].size);
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
			item.salt = 0;
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
	sec->cb[SEC_CTRL_ARG_RNG].cb = sec_rng_ctrl;
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

bcm_sec_export_item_t bcm_sec_exported_items[BCM_SEC_MAX_EXPORTED_ITEMS];
static int bcm_sec_process_exports(u8 * fit_hdr, bcm_sec_cb_arg_t* key_args)
{
	u8 * fit_ptr = NULL;
	bcm_sec_delg_cfg* delg_cfg = bcm_sec_get_delg_cfg();
	char * allowed_exports = NULL;
	bcm_sec_key_arg_t * item_list;
	int sec_export_offset = -1;
	int exp_item_offset = -1;
	int count = 0;
	int ndepth = 0;
	int * value = NULL;

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
			allowed_exports = (char*)fdt_getprop((void *)fit_ptr, sec_export_offset, DELG_SEC_ALLOWED_EXPORTS, NULL);
			if( !allowed_exports || strlen((const char*)allowed_exports) == 0 ) {
				/* No exports allowed */
				sec_export_offset = -1;
			}
		}
	}
	/* If no policy export node  or if policy export  is disabled, simply return */
	if( sec_export_offset < 0 ) {
		return 0;
	}

	/* Check delegate's requested exports */
	if( fit_hdr ) {
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
	}

	if (sec_export_offset) {
		/* Process all image subnodes */
		for (ndepth = 0, count = 0,
		     exp_item_offset = fdt_next_node(fit_hdr, sec_export_offset, &ndepth);
				(exp_item_offset >= 0) && (ndepth > 0);
				exp_item_offset = fdt_next_node(fit_hdr, exp_item_offset, &ndepth)) {
			if (ndepth == 1) {
				/*
				 * Direct child node of the sec_export parent node,
				 * i.e. item node.
				 */

				/* Get key name */
				bcm_sec_exported_items[count].value = NULL;
				bcm_sec_exported_items[count].name = (char*)fdt_getprop((void *)fit_hdr, exp_item_offset, DELG_EXP_ITEM_NAME, NULL);
				bcm_sec_exported_items[count].id = (char*)fdt_getprop((void *)fit_hdr, exp_item_offset, DELG_EXP_ITEM_ID, NULL);
				value = (int*)fdt_getprop((void *)fit_hdr, exp_item_offset, DELG_EXP_ITEM_LENGTH, NULL);

				if( !bcm_sec_exported_items[count].name || !bcm_sec_exported_items[count].id || !value )
					continue;

				printf("INFO: Found item:%s id:%s in fit\n", bcm_sec_exported_items[count].name,  bcm_sec_exported_items[count].id);
				/* Check if requested item is allowed to be exported */
				if( !strstr(allowed_exports, bcm_sec_exported_items[count].id))
					continue;

				bcm_sec_exported_items[count].len = be32_to_cpu(*value);
				if( !bcm_sec_exported_items[count].len)
					continue;

				/* Get optional 32-bit salt */
				value = (int*)fdt_getprop((void *)fit_hdr, exp_item_offset, DELG_EXP_ITEM_SALT, NULL);
				if( value )
					bcm_sec_exported_items[count].salt = be32_to_cpu(*value);

				/* Get optional hashing algorithm */
				bcm_sec_exported_items[count].algo = (char*)fdt_getprop((void *)fit_hdr, exp_item_offset, DELG_EXP_ITEM_ALGO, NULL);
				bcm_sec_exported_items[count].exp_flag = 1;

				count++;

				if( count >= BCM_SEC_MAX_EXPORTED_ITEMS ) {
					printf("INFO: Max secure export item limit of %d items reached! Ignoring other items!\n", BCM_SEC_MAX_EXPORTED_ITEMS);
					break;
				}
			}
		}
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
		/* commit the antirollback */
		printf("INFO: Committing antirollback level:%d\n", commit_antirollback);
		ret = bcm_sec_set_antirollback_lvl(commit_antirollback);
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

static int bcm_sec_delg_process_sw_restrictions( u8 * fit_hdr, int verify_loaded_img)
{
	int * value = 0;
	u8 * sec_pol_fit_ptr = NULL;
	bcm_sec_delg_cfg* delg_cfg = bcm_sec_get_delg_cfg();
	int node = -1;
	int count=0;
	int ndepth;
	int img_desc_offset, req_imgs_offset, fit_img_offset, fit_img_hash_offset;

	/* IF delegations are active then hw_state in security  */
	debug("INFO: Looking for %s node in security policy\n", DELG_SEC_POL_SW_RESTRICT_PATH);
	if( delg_cfg && delg_cfg->delg_id ) 
	{
		sec_pol_fit_ptr = delg_cfg->sec_policy_fit;

		/* Get security sw restrictions node offset */
		node =  fdt_path_offset (sec_pol_fit_ptr, DELG_SEC_POL_SW_RESTRICT_PATH);
		if (node < 0) {
			debug("INFO: Could not find %s node in security policy\n", DELG_SEC_POL_SW_RESTRICT_PATH);
			return 0;
		} else {
			/* Check if node is disabled */
			value = (int*)fdt_getprop((void *)sec_pol_fit_ptr, node, DELG_SEC_POL_STATUS, NULL);
			if (value && (strcasecmp((const char*)value, DELG_SEC_POL_STATUS_DISABLED) == 0)){
				debug("INFO: Found DISABLED  %s node in security policy\n", DELG_SEC_POL_SW_RESTRICT_PATH);
				return 0;
			} else {
				printf("INFO: Found %s node in security policy\n", DELG_SEC_POL_SW_RESTRICT_PATH);

				/* Get required images node offset */
				req_imgs_offset =  fdt_subnode_offset (sec_pol_fit_ptr, node,  DELG_SEC_POL_REQ_IMGS);
				if (req_imgs_offset < 0) {
					printf("ERROR: Could not find %s/%s node in security policy\n", 
						DELG_SEC_POL_SW_RESTRICT_PATH, DELG_SEC_POL_REQ_IMGS);
					return -EINVAL;
				}
			}
		}

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

				/* TODO: Check if image will overflow to sensitive memory areas */
				if( !verify_loaded_img ) {
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
	}
	return 0;
}

int bcm_sec_delg_post_process_sw_restrictions( u8 * fit_hdr )
{
	return( bcm_sec_delg_process_sw_restrictions(fit_hdr, 1));
}
int bcm_sec_delg_pre_process_sw_restrictions( u8 * fit_hdr )
{
	return( bcm_sec_delg_process_sw_restrictions(fit_hdr, 0));
}

bcm_sec_enc_key_arg_t bcm_sec_encoded_keys[BCM_SEC_MAX_ENCODED_KEYS];

static int bcm_sec_process_encoded_keys(u8 * fit_hdr, bcm_sec_cb_arg_t* key_args)
{
	/* TODO: Encoded keys could be present in security node or outside security node.
	 * Keys inside security node will be encrypted via the root aes key. Keys outside
	 * the security node will be encrypted via the active (delegated) aes key.
	 * These keys will have  security permissions. These keys need to be added to the 
	 * key_chain so that they can be exported to the next level of software in 
	 * spl_perform_fixups */
	int enc_keys_offset = -1;
	int key_offset = -1;
	int count = 0;
	int ndepth = 0;
	int * value = NULL;
	bcm_sec_key_arg_t *enc_keys;
	bcm_sec_delg_cfg* delg_cfg = bcm_sec_get_delg_cfg();

	/* Process key-owners encoded keys in security node if present
	 * These keys are encrypted using ROE */
	if( delg_cfg && delg_cfg->delg_id ) 
	{
		/* TODO: Handle key-owners encoded keys */
	}

	/* Process delegates encoded keys in FIT header
	 * These keys are encrypted using the active AES key */
	enc_keys_offset =  fdt_path_offset (fit_hdr, DELG_ENC_KEYS_PATH);
	if (enc_keys_offset < 0) {
		debug("INFO: Could not find %s node in fit\n", DELG_ENC_KEYS_PATH);
		return 0;
	} else {
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
				bcm_sec_encoded_keys[count].name = (char*)fdt_getprop((void *)fit_hdr, key_offset, DELG_ENC_KEYS_NAME, NULL);
				if (bcm_sec_encoded_keys[count].name == NULL)
				{
					printf("INFO: Can't find %s parameter in key node under %s \n", DELG_ENC_KEYS_NAME,
						DELG_ENC_KEYS_PATH);
					continue;
				}
				printf("INFO: Found key %s in under %s\n", bcm_sec_encoded_keys[count].name, DELG_ENC_KEYS_PATH);

				/* Get unencrypted size */
				value = (int*)fdt_getprop((void *)fit_hdr, key_offset, DELG_ENC_KEYS_SIZE, NULL);
				if( value ) {
					bcm_sec_encoded_keys[count].size  =  be32_to_cpu(*value); 
				} else {
					printf("INFO: Unencrypted size for key %s is not specified! Skipping Key!\n", 
						bcm_sec_encoded_keys[count].name);
					continue;
				}
				
				/* Get keydata */
				value = (int*)fdt_getprop((void *)fit_hdr, key_offset, DELG_ENC_KEYS_DATA, (int*)&bcm_sec_encoded_keys[count].size_enc);
				if( value && bcm_sec_encoded_keys[count].size_enc ) {
					bcm_sec_encoded_keys[count].data = (u8*)value;
				} else {
					printf("INFO: Key data for key %s not found! Skipping Key!\n", 
						bcm_sec_encoded_keys[count].name);
					continue;
				}

				/* Get load address (optional) */
				value = (int*)fdt_getprop((void *)fit_hdr, key_offset, DELG_ENC_KEYS_LDADDR, NULL);
				if( value )
					bcm_sec_encoded_keys[count].load_addr  =  be64_to_cpu(*value); 

				/* Get load permissions and enc algorithm (optional) */
				bcm_sec_encoded_keys[count].perm = (char*)fdt_getprop((void *)fit_hdr, key_offset, DELG_ENC_KEYS_PERM, NULL);
				bcm_sec_encoded_keys[count].algo = (char*)fdt_getprop((void *)fit_hdr, key_offset, DELG_ENC_KEYS_ALGO, NULL);
				count++;

				if( count >= BCM_SEC_MAX_ENCODED_KEYS ) {
					printf("INFO: Max encoded key limit of %d keys reached! Ignoring other keys!\n", BCM_SEC_MAX_ENCODED_KEYS);
					break;
				}
			}
		}
	}

	/* Add detected keys */
	if( count ) {
		enc_keys = malloc(sizeof(bcm_sec_key_arg_t));
		if (!enc_keys) {
			printf("ERROR: Cannot allocate memory for encoded key ring!\n");
			return -1;
		}
		enc_keys->len = count;
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
	u32 min_tpl_version __attribute__((unused));
	char * key_enc_alg __attribute__((unused));
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
	nodeoffset = fdt_path_offset((void *)policy_fdt, DELG_SEC_POL_PATH);
	if( nodeoffset < 0 ) {
		printf("ERROR: %s node not found in security policy!\n", DELG_SEC_POL_PATH);
		return ret;
	}

	/* Get delegate id */
	value = (u32*)fdt_getprop((void *)policy_fdt, nodeoffset, DELG_SEC_POL_DELGID, NULL);
	if (value == NULL)
	{
		printf("Can't find %s parameter in %s\n", DELG_SEC_POL_DELGID,
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
		printf("Can't find %s parameter in %s\n", DELG_SEC_POL_MINTPLV,
			DELG_SEC_POL_PATH);
		return 0;
	}

	//TODO: Compare TPL version to something
	min_tpl_version =  be32_to_cpu(*value); 

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
		printf("Can't find %s parameter in %s\n", DELG_SEC_POL_AES_KEY_ENC_ALGO,
			DELG_SEC_POL_AES_KEY_PATH);
		return ret;
	}

	// TODO: Support other encryption algorithms
	key_enc_alg = (char*)value;

	/* Get key data */
	value = (u32*)fdt_getprop((void *)policy_fdt, nodeoffset, DELG_SEC_POL_AES_KEY_DATA, &key_len);
	if (value == NULL)
	{
		printf("Can't find %s parameter in %s\n", DELG_SEC_POL_AES_KEY_DATA,
			DELG_SEC_POL_AES_KEY_PATH);
		return ret;
	}
	printf("Found potential Encrypted AES Key: KAES:0x%08x len:%d\n",
		*(u32*)value, key_len); 

	/* Decypt delegates aes key using ROE */
	u8* ek_iv = NULL;
	bcm_sec_get_root_aes_key(&ek_iv);
	key_data = malloc(key_len);
	if (!key_data) {
		printf("ERROR: Cannot allocate memory for decoded delegated key!\n");
		return -1;
	}
	memcpy(key_data, (u8*)value, key_len); 
	debug("Decrypting del_enc_key: ROE:0x%08x, KAES:0x%08x len:%d\n",
	                *(u32*)ek_iv, *(u32*)value, key_len);
	bcm_sec_aes_cbc128((u8*)ek_iv, (u8*)ek_iv + AES128_KEY_LENGTH, key_data, key_len,0);

	/* Copy key to our delg config */
	memcpy(delg_cfg->aes_ek, (void*)key_data, 
			BCM_SECBT_AES_CBC128_EK_LEN); 
	memcpy(delg_cfg->aes_ek + BCM_SECBT_AES_CBC128_EK_LEN,  
		(void*)(key_data+BCM_SECBT_AES_CBC128_EK_LEN), BCM_SECBT_AES_CBC128_EK_LEN);
	free(key_data);
	key_data = NULL;

#if 0	
	/* ### WARNING: ENABLING THIS DEBUG WILL EXPOSE DECRYPTED KEYS ### */
	debug("Decrypted AES_EK: 0x%08x AES_IV: 0x%08x\n",
		*(u32*)delg_cfg->aes_ek, 
		*(u32*)(delg_cfg->aes_ek + BCM_SECBT_AES_CBC128_EK_LEN));
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
	fit_image_get_data_and_size((void *)fit, nodeoffset, (const void**)&sec_policy_ptr, &size);
	if( !size ) {
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
	printf("Found potential Security Node: Policy:0x%08x Sig:0x%08x Size:%d\n",
		*(u32*)sec_policy_ptr, *(u32*)sig_sec_fit_delg, (u32)size);

	ret = bcm_sec_rsa_verify((u8*)sec_policy_ptr, 
				size, 
				sig_sec_fit_delg,  
				RSA2048_BYTES, krot_pub, &im );

	if( ret == 0 ) {
		printf("Security Node Authentication Successfull!\n");
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

	/* parse sdr fields */
	psdr += sizeof(u32);
	delegateId = *(u32*)psdr;
	psdr += sizeof(u32);
	max_anti_rollback_lvl = *(u32*)psdr;
	psdr += sizeof(u32);
	krsa_delg_pub = psdr;
	psdr += RSA2048_BYTES;
	sig_sec_rec_delg = psdr;
	sdr_size = (unsigned long long )(psdr - sdr_start); // Check this
	*sdr_plus_sig_size = sdr_size + RSA2048_BYTES;

	printf("Found potential SDR: delg_id:%d maxrollbck:%d size:%llu\n", 
		delegateId, max_anti_rollback_lvl, sdr_size);

	printf("Found potential SDR: PubKey:0x%08x Sig:0x%08x\n",
		*(u32*)krsa_delg_pub, *(u32*)sig_sec_rec_delg);
	
	/* Check if SDR + signature have crossed our search boundary */
	if ( ((u8*)psdr + *sdr_plus_sig_size-1) > hdr_end ) {
		printf("ERROR: Corrupted SDR, search exceeds boundary 0x%p > 0x%p\n",
			(u8*)psdr + RSA2048_BYTES-1, hdr_end);
		goto err;
	}
	
	/* Check if delegateID is valid */
	if( !delegateId ) {
		printf("ERROR: Invalid Delegate ID %d\n", delegateId);
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
		printf("SDR Authentication Successfull!\n");
		/* Commit all fields to our structures */
		delg_cfg = malloc(sizeof(bcm_sec_delg_cfg)); //TODO: Where to free this?
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
	memset(_aes,    0, sizeof(bcm_sec_key_arg_t)*2);
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
			memset(_aes[1].key,  AES128_KEY_LENGTH*2, 0);
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
