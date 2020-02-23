/*
* <:copyright-BRCM:2016:DUAL/GPL:standard
* 
*    Copyright (c) 2016 Broadcom 
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
 * File Name  : bcm63xx_sec.c
 *
 * Description: 
 *    This file processes rootfs hash
 *
 ***************************************************************************/

/* Includes. */
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/capability.h>
#include <linux/slab.h>
#include <linux/errno.h>
#include <linux/module.h>
#include <linux/netdevice.h>
#include <asm/uaccess.h>
#include <linux/mtd/ubi.h>
#include <drivers/mtd/ubi/ubi.h>
#include <bcm_map_part.h>
#include <board.h>
#include <boardparms.h>
#include <shared_utils.h>
#include <bcmtypes.h>
#include <crypto/sha.h>
#include <crypto/sha256_base.h>
#include <linux/crypto.h>
#include "bcm_assert_locks.h"
#include "board_wd.h"
#include "bcm_mbox_map.h"

#if defined(_HASH_DBG_)
#define PRINT_HASH(PDGST)			do{   							\
							int j;                				\
							printk("%s:%d \nDigest: ",__func__,__LINE__); 	\
							for (j = 0; j < 32; j++ ) { 			\
								printk( "%02x ",PDGST[j]);		\
							}						\
							printk("\n");					\
						}while(0)
#else
#define PRINT_HASH(PDGST)
#endif
extern int bcm_get_root_propdata( const char * prop_name, char * data, int prop_size );

#define BRCM_ROOTFS_SHA256_PROP      "brcm_rootfs_sha256"
#define BRCM_ROOTFS_IMGLEN_PROP      "brcm_rootfs_imglen"

static int ubi_vol_notify(struct notifier_block *nb,
			 	unsigned long notification_type, 
				void *ns_ptr);
static struct notifier_block _ubi_vol_notifier = {
	.notifier_call = ubi_vol_notify,
};
struct root_ubifs_hash {
	struct shash_alg sha;	
	u8 calc_hash[SHA256_DIGEST_SIZE];
	u8 image_dgst[SHA256_DIGEST_SIZE];
	long image_size;
};

static struct root_ubifs_hash obj = {
	.sha = {
		.digestsize	=	SHA256_DIGEST_SIZE,
		.init           =       sha256_base_init,
		.update         =       crypto_sha256_update,
		.finup          =       crypto_sha256_finup,
		.descsize	=	sizeof(struct sha256_state),
		.base		=	{
			.cra_name	=	"sha256",
			.cra_driver_name=	"sha256-generic",
			.cra_flags	=	CRYPTO_ALG_TYPE_SHASH,
			.cra_blocksize	=	SHA256_BLOCK_SIZE,
		}
	},
};

typedef enum UBI_CALC_HASH_ERR {
	UBI_CALC_HASH_ERR_OK,
	UBI_CALC_HASH_ERR_FAIL,
	UBI_CALC_HASH_ERR_INVAL,
} ubi_calc_hash_err_t;

static ubi_calc_hash_err_t  ubi_calc_hash(struct root_ubifs_hash *_obj,
			struct ubi_device_info* di,
			struct ubi_volume_info* vi,
			u32 fs_magic)
{
	struct crypto_shash shash = {0};
	void *buf = NULL;
	struct ubi_device *ubi = ubi_get_device(di->ubi_num);
	struct ubi_volume *vol = ubi->volumes[vi->vol_id];
	struct shash_desc *desc = NULL;
	ubi_calc_hash_err_t  res = UBI_CALC_HASH_ERR_OK;
	int i;
	long image_size = _obj->image_size;

	printk(KERN_INFO "found %s %d\n", vol->name, vol->vol_id);
	desc = vmalloc(sizeof(struct shash_desc)+sizeof(struct sha256_state));
	if (!desc) {
		res =  UBI_CALC_HASH_ERR_FAIL;
		goto err;
	}
	desc->tfm = &shash;
	desc->tfm->base.__crt_alg = &_obj->sha.base;
	/* Note: All calls to sha256_base.h lib return 0 unconditionally*/
 	_obj->sha.init(desc);
	buf = vmalloc(vol->usable_leb_size);
	if (!buf) {
		res = UBI_CALC_HASH_ERR_FAIL;
		goto err;
	}
	for (i = 0; i < vol->used_ebs && image_size > 0 ; i++) {
	/* Original size of the volume is reflected via reserved_pebs
		Although, when volume gets resized the reserved pebs are
		equalized to size of the flash (instead of 144 becomes 2020 sector sizes) 
		The size of the squash or other images in this case needs to known in advance e.g.
                coming from dtb, as well as the has to verify against 
	*/
		int size;
		cond_resched();
		if (i == vol->used_ebs - 1)
			size = vol->last_eb_bytes;
		else
			size = vol->usable_leb_size;
		if (size > image_size) {
			size = image_size;
		}
		res = ubi_eba_read_leb(ubi, vol, i, buf, 0, size, 0);
		if (res) {
			res = UBI_CALC_HASH_ERR_FAIL;
			goto err;
		}
		if (i == 0) {
			u32 magic = 0;
			memcpy(&magic, buf, sizeof(magic));
			if (magic != fs_magic) {
				res = UBI_CALC_HASH_ERR_FAIL;
				goto err;
			}
		}
		/*Calc hash */
		_obj->sha.update(desc, (const u8*)buf, size);
		image_size -= size;
	}
	if (image_size) {
		res = UBI_CALC_HASH_ERR_INVAL;
		goto err;
	}
	_obj->sha.finup(desc, (const u8 *)buf, 0, _obj->calc_hash);
err:
	if (buf) {
		vfree(buf);
	}
	if (desc) {
		vfree(desc);
	}
	return res;
}

static int ubi_vol_notify(struct notifier_block *nb,
			 unsigned long notification_type, void *ns_ptr)
{
	struct ubi_notification* nt;
	struct ubi_volume_info* vi;
	struct ubi_device_info* di;
	int rc = NOTIFY_OK;
	if (notification_type != UBI_VOLUME_ADDED) {
		goto  done;
	}
	nt = (struct ubi_notification*)ns_ptr;
	vi = &nt->vi;
	di = &nt->di;
#if 0
	if (vi->vol_type == UBI_STATIC_VOLUME) {
		printk(KERN_INFO " -- UBI_TEST static volume\n");
	}
#endif
	if (!strncmp(vi->name,"rootfs_ubifs",12)) {
		ubi_calc_hash_err_t  res = ubi_calc_hash(&obj, di, vi, SQUASHFS_MAGIC); 
		switch (res) {
			case UBI_CALC_HASH_ERR_INVAL:
			case UBI_CALC_HASH_ERR_FAIL:
				printk(KERN_ERR "Failed to calculate hash\n");
				goto  _hlt;
			case UBI_CALC_HASH_ERR_OK:
				PRINT_HASH(obj.calc_hash);
				if (memcmp(obj.image_dgst, obj.calc_hash, SHA256_DIGEST_SIZE)) {
					goto _hlt;
				}
				printk(KERN_INFO "%s SHA256 digest - OK\n",vi->name);
				rc =  NOTIFY_STOP_MASK;
			default:
				break;
		}
	}
done:
	return rc;
_hlt:
	printk(KERN_CRIT "SHA256 Digest failed ... Resetting\n ");
#if !defined(CONFIG_BCM947189)
        BCM_MBOX_SOFT_RESET_SAFE_EN;
        BCM_MBOX_INACTIVE_IMAGE_SET(0x1);
#endif
#if defined(CONFIG_BCM_WATCHDOG_TIMER)
    	bcm_set_watchdog(0, 0, 0, 0);
#else
	kerSysSoftReset();	
#endif
	while(1);	
	return rc;
}

int __init bcm63xx_sec_init(void)
{
	/* Check if hash is requested in flat DT */
	if (!bcm_get_root_propdata(BRCM_ROOTFS_SHA256_PROP, (void*)(&obj.image_dgst),SHA256_DIGEST_SIZE) && 
		!bcm_get_root_propdata(BRCM_ROOTFS_IMGLEN_PROP,  (void*)(&obj.image_size), sizeof(unsigned int))) {
		printk(KERN_DEBUG "%s: Image size to hash %lu\n",__func__,obj.image_size);
		PRINT_HASH(obj.image_dgst);
		ubi_register_volume_notifier(&_ubi_vol_notifier, 1);
	} 
        return 0;
}

device_initcall(bcm63xx_sec_init);
