#if defined(CONFIG_BCM_KF_MTD_BCM963XX)
/*
<:copyright-BRCM:2013:DUAL/GPL:standard 

   Copyright (c) 2013 Broadcom 
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

/*
 * Flash mapping code for BCM963xx board SPI NOR flash memory
 *
 * Song Wang (songw@broadcom.com)
 */

#include <linux/module.h>
#include <linux/slab.h>
#include <linux/kernel.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/map.h>

#include <board.h>
#include <bcmTag.h>
#include <bcm_map_part.h>
#include <flash_api.h>

static void bcm63xx_noop(struct mtd_info *mtd);
static int bcm63xx_read(struct mtd_info *mtd, loff_t from, size_t len, size_t *retlen, u_char *buf);
static int bcm63xx_erase(struct mtd_info *mtd, struct erase_info *instr);
static int bcm63xx_write(struct mtd_info *mtd, loff_t to, size_t len, size_t *retlen, const u_char *buf);

static struct mtd_info *mtdRootFS;

#ifdef CONFIG_AUXFS_JFFS2
static struct mtd_info *mtdAuxFS;
#endif

static void bcm63xx_noop(struct mtd_info *mtd)
{
}

static int bcm63xx_read(struct mtd_info *mtd, loff_t from, size_t len, size_t *retlen, u_char *buf)
{
	unsigned long flash_base;

	flash_base = (unsigned long)mtd->priv;
	*retlen = kerSysReadFromFlash(buf, flash_base + from, len); 
	
	return 0;
}

static int bcm63xx_erase(struct mtd_info *mtd, struct erase_info *instr)
{
	unsigned long flash_base;

	if (instr->addr + instr->len > mtd->size) {
		printk("ERROR: bcm63xx_erase( mtd[%s]) invalid region\n", mtd->name);
		return (-EINVAL);
	}

	flash_base = (unsigned long)mtd->priv;

	if (kerSysEraseFlash( flash_base + instr->addr, instr->len))
		return (-EINVAL);
	
	instr->state = MTD_ERASE_DONE;
	mtd_erase_callback(instr);

	return 0;
}

static int bcm63xx_write(struct mtd_info *mtd, loff_t to, size_t len, size_t *retlen, const u_char *buf)
{
	unsigned long flash_base;
	int bytesRemaining;

	flash_base = (unsigned long)mtd->priv;

	bytesRemaining = kerSysWriteToFlash(flash_base + to, (char*)buf, len);
	*retlen = (len - bytesRemaining);

	return 0;
}

#ifdef CONFIG_AUXFS_JFFS2
static int __init create_aux_partition(void)
{
	FLASH_PARTITION_INFO fPartAuxFS;

	/* Read the flash memory partition map. */
	kerSysFlashPartInfoGet(&fPartAuxFS);

	if (fPartAuxFS.mem_length != 0) {

		if ((mtdAuxFS = kzalloc(sizeof(*mtdAuxFS), GFP_KERNEL)) == NULL)
			return -ENOMEM;

		/* Read/Write data (Aux) partition */
		mtdAuxFS->name = "data";
		mtdAuxFS->index = -1;
		mtdAuxFS->type = MTD_NORFLASH;
		mtdAuxFS->flags = MTD_CAP_NORFLASH;

		mtdAuxFS->erasesize = fPartAuxFS.sect_size;
		mtdAuxFS->writesize = 1;
		mtdAuxFS->numeraseregions = 0;
		mtdAuxFS->eraseregions	= NULL;
		mtdAuxFS->size = fPartAuxFS.mem_length;

		mtdAuxFS->_read = bcm63xx_read;
		mtdAuxFS->_erase = bcm63xx_erase;
		mtdAuxFS->_write = bcm63xx_write; 
		mtdAuxFS->_sync = bcm63xx_noop;
		mtdAuxFS->owner = THIS_MODULE;
		mtdAuxFS->priv = (void*)fPartAuxFS.mem_base;

		if (mtd_device_register(mtdAuxFS, NULL, 0 )) {
			printk("Failed to register device mtd:%s\n", mtdAuxFS->name);
			return -EIO;
		}	
	
		printk("Registered device mtd:%s dev%d Address=0x%08x Size=%llu\n",
			mtdAuxFS->name, mtdAuxFS->index, (int)mtdAuxFS->priv, mtdAuxFS->size);
	}

	return 0;
}
#endif

static int __init init_brcm_physmap(void)
{
	uintptr_t rootfs_addr, kernel_addr;
	PFILE_TAG pTag = (PFILE_TAG)NULL;
	int flash_type = flash_get_flash_type();

	if ((flash_type != FLASH_IFC_SPI) && (flash_type != FLASH_IFC_HS_SPI )) 
		return -EIO;

	printk("bcm963xx_mtd driver\n");

	if (!(pTag = kerSysImageTagGet())) {
		printk("Failed to read image tag from flash\n");
		return -EIO;
	}

	rootfs_addr = (uintptr_t)simple_strtoul(pTag->rootfsAddress, NULL, 10) + BOOT_OFFSET + IMAGE_OFFSET;
	kernel_addr = (uintptr_t)simple_strtoul(pTag->kernelAddress, NULL, 10) + BOOT_OFFSET + IMAGE_OFFSET;

	if ((mtdRootFS = kzalloc(sizeof(*mtdRootFS), GFP_KERNEL)) == NULL)
		return -ENOMEM;

	/* RootFS Read only partition */
	mtdRootFS->name = "rootfs";
	mtdRootFS->index = -1;
	mtdRootFS->type = MTD_NORFLASH;
	mtdRootFS->flags = MTD_CAP_ROM;

	mtdRootFS->erasesize = 0x10000;
	mtdRootFS->writesize = 0x10000;
	mtdRootFS->numeraseregions = 0;
	mtdRootFS->eraseregions	= NULL;

	mtdRootFS->_read = bcm63xx_read;
	mtdRootFS->_erase = bcm63xx_erase;
	mtdRootFS->_write = bcm63xx_write;
	mtdRootFS->_sync = bcm63xx_noop;
	mtdRootFS->owner = THIS_MODULE;

	if ((mtdRootFS->size = (kernel_addr - rootfs_addr)) <= 0) {
		printk("Invalid RootFs size\n");
		kfree(mtdRootFS);
		return -EIO;
	}

	mtdRootFS->priv = (void*)rootfs_addr;

	if (mtd_device_register(mtdRootFS , NULL, 0)) {
		printk("Failed to register device mtd:%s\n", mtdRootFS->name);
		return -EIO;
	}
	
	printk("Registered device mtd:%s dev%d Address=%p Size=%llu\n",
		mtdRootFS->name, mtdRootFS->index, (void *)mtdRootFS->priv, mtdRootFS->size);

	if (kerSysIsRootfsSet() == 0) {
		kerSysSetBootParm("root=", "/dev/mtdblock0");
		kerSysSetBootParm("rootfstype=", "squashfs");
	}

#ifdef CONFIG_AUXFS_JFFS2
	create_aux_partition();
#endif

	return 0;
}

static void __exit cleanup_brcm_physmap(void)
{
	if (mtdRootFS->index >= 0)
		mtd_device_unregister(mtdRootFS);

	kfree(mtdRootFS);

#ifdef CONFIG_AUXFS_JFFS2
	if (mtdAuxFS->index >= 0)
		mtd_device_unregister(mtdAuxFS);

	kfree(mtdAuxFS);
#endif
}

module_init(init_brcm_physmap);
module_exit(cleanup_brcm_physmap);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Song Wang songw@broadcom.com");
MODULE_DESCRIPTION("MTD Driver for Broadcom NOR Flash");
#endif
