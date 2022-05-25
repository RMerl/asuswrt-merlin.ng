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
#include <linux/version.h>
#include <linux/bug.h>
#include <linux/init.h>
#include <linux/errno.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/capability.h>
#include <linux/platform_device.h>
#include <linux/netdevice.h>
#include <linux/string.h>
#include <linux/string_helpers.h>
#include <asm/uaccess.h>
#include <linux/mtd/ubi.h>
#include <drivers/mtd/ubi/ubi.h>
#include <linux/magic.h>
#include <bcm_map_part.h>
#include <board.h>
#include <boardparms.h>
#include <shared_utils.h>
#include <bcmtypes.h>
#include <crypto/sha.h>
#include <crypto/sha256_base.h>
#include <linux/crypto.h>
#include "bcm_assert_locks.h"
#include "bcm_mbox_map.h"
#include <linux/lsm_hooks.h>

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

#define DRV_NAME "brcm_rfshash"

#define SQUASHFS_ENCRYPTED (~SQUASHFS_MAGIC)

extern int bcm_get_root_propdata( const char * prop_name, char * data, int prop_size );

#define BRCM_ROOTFS_SHA256_PROP      "brcm_rootfs_sha256"
#define BRCM_ROOTFS_IMGLEN_PROP      "brcm_rootfs_imglen"

static int ubi_vol_notify(struct notifier_block *nb,
			 	unsigned long notification_type, 
				void *ns_ptr);
static struct notifier_block ubi_vol_notifier = {
	.notifier_call = ubi_vol_notify,
};
struct rootfs_ubifs_hash {
	struct shash_alg sha;	
	u8 calc_hash[SHA256_DIGEST_SIZE];
	u8 image_dgst[SHA256_DIGEST_SIZE];
	long image_size;
	int vol_id;
	int ubi_num;
	char* dev_vol_nm;
	int trusted;
	u32 fs_magic; 
	int dm_cnt; 
};

static struct rootfs_ubifs_hash obj = {
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

typedef enum UBI_ROOTFS_CALC_HASH_ERR {
	UBI_CALC_HASH_ERR_OK,
	UBI_CALC_HASH_ERR_FAIL,
	UBI_CALC_HASH_ERR_INVAL,
} ubi_rootfs_calc_hash_err_t;

static ubi_rootfs_calc_hash_err_t  ubi_rootfs_calc_hash(struct rootfs_ubifs_hash *obj_rfs,
			struct ubi_device_info* di,
			struct ubi_volume_info* vi,
			u32 fs_magic)
{
	struct crypto_shash shash = {0};
	void *buf = NULL;
	struct ubi_device *ubi = ubi_get_device(di->ubi_num);
	struct ubi_volume *vol = ubi->volumes[vi->vol_id];
	struct shash_desc *desc = NULL;
	ubi_rootfs_calc_hash_err_t  res = UBI_CALC_HASH_ERR_OK;
	int i;
	long image_size = obj_rfs->image_size;

	printk(KERN_DEBUG "found %s %d\n", vol->name, vol->vol_id);
	desc = vmalloc(sizeof(struct shash_desc)+sizeof(struct sha256_state));
	if (!desc) {
		res =  UBI_CALC_HASH_ERR_FAIL;
		goto err;
	}
	desc->tfm = &shash;
	desc->tfm->base.__crt_alg = &obj_rfs->sha.base;
	/* Note: All calls to sha256_base.h lib return 0 unconditionally*/
 	obj_rfs->sha.init(desc);
	buf = vmalloc(vol->usable_leb_size);
	if (!buf) {
		res = UBI_CALC_HASH_ERR_FAIL;
		goto err;
	}
	for (i = 0; i < vol->used_ebs && image_size > 0 ; i++) {
	/* Original size of the volume is reflected via reserved_pebs
		Although, when a volume gets resized the reserved pebs are
		equalized to the size of the flash (instead of 144 becomes 2020 sector sizes) 
		The size of the squash or other images in this case must be known in advance e.g.
                expected to coming from dtb or kerenel bootcmd line, as well as it has to verified against it 
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
		if (i == 0 && fs_magic != SQUASHFS_ENCRYPTED) {
			u32 magic = 0;
			memcpy(&magic, buf, sizeof(magic));
			if (magic != fs_magic) {
				res = UBI_CALC_HASH_ERR_FAIL;
				goto err;
			}
		}
		/*Calc hash */
		obj_rfs->sha.update(desc, (const u8*)buf, size);
		image_size -= size;
	}
	if (image_size) {
		res = UBI_CALC_HASH_ERR_INVAL;
		goto err;
	}
	obj_rfs->sha.finup(desc, (const u8 *)buf, 0, obj_rfs->calc_hash);
err:
	if (buf) {
		vfree(buf);
	}
	if (desc) {
		vfree(desc);
	}
	return res;
}

static int ubi_vol_args_rootfs_cb(char *param, char *val, 
		const char *doing, void *arg)
{
	char* tmp = NULL;
	
	if ( (param && val) && 
		((!strcmp("root", param) && !strncmp(val,"/dev/dm",sizeof(char)*7)) || 
		 !strcmp(param,"dm-mod.create") || 
		(!strcmp(param, "rootfstype") && !strcmp(val, "squashfs")))) {
		printk(KERN_DEBUG "%s=%s",param,val);
		obj.dm_cnt++;
	}
	if (val) {
		tmp = strstr(val,"/dev/ubiblock");
		if (tmp && !obj.dev_vol_nm) {
			obj.dev_vol_nm = kstrdup(tmp, GFP_KERNEL);
			sscanf(obj.dev_vol_nm,"/dev/ubiblock%d_%d", &obj.ubi_num, &obj.vol_id);
			printk(KERN_INFO "listening for %s %d %d\n", 
						obj.dev_vol_nm, obj.vol_id, obj.ubi_num);
		}
	}
	return 0;
}

static void __reset(void )
{
        BCM_MBOX_SOFT_RESET_SAFE_EN;
        BCM_MBOX_INACTIVE_IMAGE_SET(0x1);
	kerSysSoftReset();	
	while(1);
}

static int ubi_vol_notify(struct notifier_block *nb,
			 unsigned long notification_type, void *ns_ptr)
{
	struct ubi_notification* nt;
	struct ubi_volume_info* vi;
	struct ubi_device_info* di;
	int rc = NOTIFY_OK;
	if (notification_type != UBI_VOLUME_ADDED) {
		return rc;	
	}
	nt = (struct ubi_notification*)ns_ptr;
	vi = &nt->vi;
	di = &nt->di;
#if 0
	if (vi->vol_type == ubi_static_volume) {
		printk(kern_info " -- ubi_test static volume\n");
	}
#endif
	printk(KERN_INFO "%s: %s %d :%d \n ",__func__,
			vi->name, vi->vol_id, vi->ubi_num);

	if (vi->vol_id != obj.vol_id || 
		vi->ubi_num != obj.ubi_num) {
		goto done;
	}
	switch(ubi_rootfs_calc_hash(&obj, di, vi, obj.fs_magic)) {
		case UBI_CALC_HASH_ERR_INVAL:
		case UBI_CALC_HASH_ERR_FAIL:
			printk(KERN_ERR "Failed to calculate hash\n");
			goto  _hlt;
		case UBI_CALC_HASH_ERR_OK:
			PRINT_HASH(obj.calc_hash);
			if (memcmp(obj.image_dgst, obj.calc_hash, SHA256_DIGEST_SIZE)) {
				goto _hlt;
			}
			printk(KERN_INFO "%s : SHA256 DIGEST - OK\n",vi->name);
			obj.trusted = 1;
			rc =  NOTIFY_STOP_MASK;
			
		default:
			break;
	}
done:
	return rc;
_hlt:
	printk(KERN_CRIT "ROOTFS SEC: SHA256 Digest failed ... Resetting\n ");
	__reset();
	return -1;
}

#ifdef CONFIG_SECURITY
/* This is what I have in mind - to be removed; unfortunately it will 
 * drag bunch of security hook in the expense of using sb_kern_mount
 * But wouild be very robust to use  
 * */
static int bcm_sec_sb_kern_mount(struct super_block *sb, int flags, void *data)
{
	if (!obj.trusted) {
		printk(KERN_CRIT "ROOTFS SEC: ROOTFS can not be verified  ... Resetting\n ");
		__reset();
	}
	return 0;
}

static struct security_hook_list bcm_sec_hook[] = {
	LSM_HOOK_INIT(sb_kern_mount, bcm_sec_sb_kern_mount)
};

#endif

static int bcm63xx_sec_setup(void)
{
	if (!bcm_get_root_propdata(BRCM_ROOTFS_SHA256_PROP, (void*)(&obj.image_dgst),SHA256_DIGEST_SIZE) && 
		!bcm_get_root_propdata(BRCM_ROOTFS_IMGLEN_PROP,  (void*)(&obj.image_size), sizeof(unsigned int))) {
		char* tmp_cmd = kstrdup(saved_command_line,GFP_KERNEL);

		/* Not supported for eMMC */
		if(strstr(tmp_cmd,"/dev/mmcblk"))
			return 0;

		parse_args("RFS SHA-2 CHECK ARGS", tmp_cmd, NULL,
		   			0, 0, 0, NULL, &ubi_vol_args_rootfs_cb);
		kfree(tmp_cmd);
		if (!obj.dev_vol_nm) {
			printk(KERN_CRIT "ROOTFS SEC: Unable to detect the signed rootfs device\n ");
			__reset();
		}
		obj.fs_magic = obj.dm_cnt < 3? SQUASHFS_MAGIC : SQUASHFS_ENCRYPTED;
#ifdef CONFIG_SECURITY
		security_add_hooks(bcm_sec_hook, ARRAY_SIZE(bcm_sec_hook), "bcm_sec_hooks");
#endif
		printk(KERN_DEBUG "%s: Image size to hash %lu\n",__func__,obj.image_size);
		PRINT_HASH(obj.image_dgst);
			ubi_register_volume_notifier(&ubi_vol_notifier, 1);
	}
	return 0;
}

static struct platform_device *bcm63xx_sec_plat_dev;

static int bcm63xx_sec_probe(struct platform_device *pdev)
{
	/* TBD for EMMC Check if hash is requested in flat DT */
        return 0;
}

static struct platform_driver bcm63xx_sec_driver = {
	.probe          = bcm63xx_sec_probe,
	.driver         = {
		.name   = DRV_NAME,
	},
};

int __init  bcm63xx_sec_init(void)
{
	int err = 0;
	if (bcm63xx_sec_setup()) {
		return err;
	}
	err = platform_driver_register(&bcm63xx_sec_driver);
	if (err) {
		return err;
	}
	bcm63xx_sec_plat_dev = platform_device_register_simple(DRV_NAME,
								  -1, NULL, 0);
	if (IS_ERR(bcm63xx_sec_plat_dev)) {
		err = PTR_ERR(bcm63xx_sec_plat_dev);
		platform_driver_unregister(&bcm63xx_sec_driver);
	}
	return 0;
}

device_initcall(bcm63xx_sec_init);

