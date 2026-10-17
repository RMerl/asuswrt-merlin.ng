/*
* <:copyright-BRCM:2024:DUAL/GPL:standard
* 
*    Copyright (c) 2024 Broadcom 
*    All Rights Reserved
* 
* Unless you and Broadcom execute a separate written software license
* agreement governing use of this software, this software is licensed
* to you under the terms of the GNU General Public License version 2
* (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
* with the following added to such license:
* 
*    As a special exception, the copyright holders of this software give
*    you permission to link this software with independent modules, and
*    to copy and distribute the resulting executable under terms of your
*    choice, provided that you also meet, for each linked independent
*    module, the terms and conditions of the license of that module.
*    An independent module is a module which is not derived from this
*    software.  The special exception does not apply to any modifications
*    of the software.
* 
* Not withstanding the above, under no circumstances may you combine
* this software in any way with any other Broadcom software provided
* under a license other than the GPL, without Broadcom's express prior
* written consent.
* 
* :> 
*/


/***************************************************************************
 * File Name  : bcm63xx_keyctl.c
 *
 * Description: 
 *   BCM key control driver
 *
 ***************************************************************************/

/* Includes. */
#include <linux/version.h>
#include <linux/bug.h>
#include <linux/init.h>
#include <linux/errno.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/string.h>
#include <asm/page.h>
#include <asm/cacheflush.h>
#include <asm/uaccess.h>
#include <linux/capability.h>
#include <linux/platform_device.h>
#include <linux/string.h>
#include <linux/string_helpers.h>
#include <linux/crypto.h>
#include <linux/key.h>
#include <linux/magic.h>
#include <security/keys/internal.h>

#include <board.h>
#include <shared_utils.h>
#include <bcmtypes.h>

#include <linux/of.h>

//#define BCM_KEYCTL_DBG 

#ifdef  BCM_KEYCTL_DBG 
#define BCM_PRT_DBG(...) printk(KERN_INFO DRV_NAME ":"  __VA_ARGS__);
#else
#define BCM_PRT_DBG(...) 
#endif
#define BCM_PRT_INFO(...) printk(KERN_INFO DRV_NAME ":"  __VA_ARGS__);
#define BCM_PRT_ERR(...) printk(KERN_ERR DRV_NAME  ": ERROR " __VA_ARGS__);
#define PRT_ARR(___dat,___sz)			do{   							\
							int j;                				\
							for (j = 0; j < ___sz; j++ ) { 			\
								if(!(j%32)){printk("\n");}		\
								printk( "%02x ",((u8*)___dat)[j]);		\
							}						\
							printk("\n");					\
						}while(0)

#define DRV_NAME "bcm_keyctl"

typedef enum bcm_key_type_len_e {
	BCM_KEY_TYPE_LEN_NONE = 0, 
	BCM_KEY_TYPE_LEN_AES_256 = 32,	
	BCM_KEY_TYPE_LEN_AES_512 = 64,	
} bcm_key_type_len_t;

#define BCM_KEYCTL_MAX_STR_SIZE 64
#define BCM_KEYCTL_KEY_MAX_N	1
#define BCM_KEYCTL_ID_MIN_LEN	4
#define BCM_IMAGE_KEY_PATH	"/"
#define BCM_IMAGE_KEY_SZ	sizeof(BCM_KEYCTL_MAX_STR_SIZE) 

#define DT_ID_LEN 128
#define BCM_KEY_DESC_LEN	(DT_ID_LEN*2 + 1) 
typedef struct bcm_key {
	u8			key[BCM_KEY_TYPE_LEN_AES_512];
	key_serial_t		id;
	char			dt_id[DT_ID_LEN+1];
	bcm_key_type_len_t	type_len;
} bcm_key_t;
 
typedef struct bcm_keyctl {
	bcm_key_t keys[BCM_KEYCTL_KEY_MAX_N];
	u32 cnt;
} bcm_keyctl_t;

static char *key_ids;
static bcm_keyctl_t keyctl_obj = {0};
char *node_match[BCM_KEYCTL_KEY_MAX_N] = {0};

static int key_type(u32 pp_len,
			bcm_key_type_len_t* type_len)
{
	int rc = 0;
	if ( pp_len == BCM_KEY_TYPE_LEN_AES_256) {
		*type_len = BCM_KEY_TYPE_LEN_AES_256;
	} else if (pp_len == BCM_KEY_TYPE_LEN_AES_512) {
		*type_len = BCM_KEY_TYPE_LEN_AES_512;
	} else {
		rc = -1; 
	}
	return 	rc;
}

static int add_key(int ringid, 
		char type[32], 
		char* description, 
		void* payload,
		u32 plen,
		key_serial_t* ref)
{
	key_ref_t keyring_ref, key_ref;
        int rc = -1;

        /* find the target keyring (which must be writable) */
        keyring_ref = lookup_user_key(ringid, KEY_LOOKUP_CREATE, KEY_NEED_WRITE);
        if (IS_ERR(keyring_ref)) {
                *ref = PTR_ERR(keyring_ref);
		BCM_PRT_ERR( "can't obtain ref for ringid:0x%x \n ", ringid);
                goto err;
        }

        /* create or update the requested key and add it to the target
 	** keyring */
	BCM_PRT_DBG(" adding master key 0x%016llx sz %d session type %s\n",
			*((unsigned long long*)payload), 
			plen, type);

        key_ref = key_create_or_update(keyring_ref, type, 
				description,
                                payload, plen, 
				KEY_PERM_UNDEF/*|KEY_ALLOC_BYPASS_RESTRICTION*/,
                                KEY_ALLOC_IN_QUOTA);
        if (!IS_ERR(key_ref)) {
                *ref = key_ref_to_ptr(key_ref)->serial;
                key_ref_put(key_ref);
		rc = 0;
        }
        else {
                *ref = PTR_ERR(key_ref);
		BCM_PRT_ERR( " can't add key to a keyring \n ");
        }
	printk(KERN_INFO DRV_NAME ":OK");
err:
	return rc;
}

static int add_keys(void)
{
	int i, rc = -1;
	long key_ring_id = 0; 
	const char *sess_type = "logon";
	char tag_desc[BCM_KEY_DESC_LEN + 1] = {0} ;
	key_serial_t new_key_id  = 0;
	bcm_key_t* keys =  keyctl_obj.keys;

	key_ring_id = keyctl_get_keyring_ID(KEY_SPEC_SESSION_KEYRING, 0);
	if (IS_ERR(ERR_PTR(key_ring_id))) {
		BCM_PRT_ERR("unable to get key ring %lu\n", key_ring_id);
		goto err;
	}
	BCM_PRT_DBG(" added key_ring %lu session type %s\n",key_ring_id, sess_type);
	for (i = 0 ; i < keyctl_obj.cnt ; i++) {
		if (keys[i].type_len == BCM_KEY_TYPE_LEN_NONE)  {
			continue;
		}  
		snprintf(tag_desc, BCM_KEY_DESC_LEN,"%s:", keys[i].dt_id);
		BCM_PRT_DBG("descr: %s type: %s\n", tag_desc, sess_type);
		if (add_key(key_ring_id, (char*)sess_type, 
				tag_desc, keys[i].key,	
				keys[i].type_len, &new_key_id)) {
			BCM_PRT_ERR("unable to add key %u\n",new_key_id);
			goto err; 
		}
		memset(keys[i].key, 0, keys[i].type_len);
		memcpy(&keys[i].id, &new_key_id, sizeof(key_serial_t));
		BCM_PRT_DBG( "new_key_id: 0x%x\n", (unsigned int)new_key_id);
	}
	rc = 0;
err:
	return rc;	
}

static int parse_bootarg(char *src, char *dst[BCM_KEYCTL_KEY_MAX_N])
{
	char* s = src, *p = src ;
	int i  = 0;
	if (!strlen(p) || strlen(p) < BCM_KEYCTL_ID_MIN_LEN) {
		return -EINVAL;
	}
	while(p && i < BCM_KEYCTL_KEY_MAX_N) {
		p = strchr(s, ',');
		if (p)
			*p++ = '\0';
		dst[i++] = strim(s);
		s =  p;
	}
	return (i > 0? 0 : -EINVAL);
}

static int bcm63xx_keyctl_early_init(void)
{
	int rc  = -1, j;
	struct property* pp;
	struct device_node* nd = NULL;
	struct {
		struct property* pp;
		struct device_node* nd;
	} prop_addr[BCM_KEYCTL_KEY_MAX_N] = {0};
	char *ids = NULL; 
	if (!key_ids) { 
		return 0;
	}

	if (strlen(key_ids) >= BCM_KEYCTL_MAX_STR_SIZE) {
		printk(KERN_ERR "Argument is too big. Limit is %d", BCM_KEYCTL_MAX_STR_SIZE);
		rc = -EINVAL;
		goto err;
	}
	ids = kstrndup(key_ids, BCM_KEYCTL_MAX_STR_SIZE, GFP_KERNEL);
	if (!ids) {
		rc = -ENOMEM;
		goto err;
	}
	rc = parse_bootarg(ids, node_match);
	if (rc) {
		BCM_PRT_ERR("Invalid module argument\n");
		goto err;
	}
	BCM_PRT_DBG("booarg param %s\n",ids);
	if (!of_have_populated_dt()) {
		BCM_PRT_ERR(" FDT is not populated\n");
		goto err;
	}
	keyctl_obj.cnt = 0;
	BCM_PRT_DBG("- in early_init for \n");
	nd = of_find_node_by_path(BCM_IMAGE_KEY_PATH);	
	for (j = 0; j <  BCM_KEYCTL_KEY_MAX_N; j++) {
		bcm_key_type_len_t type_len;
		for_each_property_of_node(nd, pp) {
			if (!strncmp(pp->name, node_match[j], DT_ID_LEN)) { 
				prop_addr[j].pp = pp;
				prop_addr[j].nd = nd;
				break;
			}
		}
		if (!prop_addr[j].pp || key_type(prop_addr[j].pp->length, &type_len)) {
			continue;
		}
	
		BCM_PRT_DBG("found node %s prop %s\n", node_match[j], prop_addr[j].pp->name);
		if (prop_addr[j].pp->length > BCM_KEY_TYPE_LEN_AES_512) {
			BCM_PRT_ERR( "key size execeeded maximum of %d \n", BCM_KEY_TYPE_LEN_AES_512);
			goto err;		
		}
	
		/* printk("%s->%s\n", of_node_full_name(n), pp->name); */
		BCM_PRT_DBG("key data 0x%016llx %d\n", 
				*((unsigned long long*)prop_addr[j].pp->value),
				prop_addr[j].pp->length);

	
		keyctl_obj.keys[j].type_len = type_len;
		strncpy(keyctl_obj.keys[j].dt_id, node_match[j], DT_ID_LEN);
		memcpy(keyctl_obj.keys[j].key, prop_addr[j].pp->value, prop_addr[j].pp->length);
		keyctl_obj.cnt++;
		/* memset_kern_ro((unsigned long)pp->value, 0 ,pp->length);*/
		/*BCM_PRT_DBG( "cleared key data 0x%016llx\n",*((unsigned long long*)pp->value));*/
	}
	while( j > 0 )  {
		if (prop_addr[--j].pp) {
			of_remove_property(prop_addr[j].nd, (void*)(prop_addr[j].pp));
		}
	}
	add_keys();

	rc = 0;
err:
	if (ids) {
		kfree(ids);
	}
        return rc;
}
module_param(key_ids, charp, 0);
MODULE_PARM_DESC(key_ids, "comma delimited list of image key nodes from dtb");
early_initcall(bcm63xx_keyctl_early_init);


