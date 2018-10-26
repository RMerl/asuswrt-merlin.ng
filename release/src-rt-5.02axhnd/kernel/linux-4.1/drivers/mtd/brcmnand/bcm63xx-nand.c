#if defined(CONFIG_BCM_KF_MTD_BCMNAND)
/*
 *
 *  drivers/mtd/brcmnand/bcm7xxx-nand.c
 *
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

#include <linux/module.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/err.h>
#include <linux/mtd/mtd.h>
#include <asm/io.h>
#include <bcm_map_part.h>
#include <board.h>
#include "brcmnand_priv.h"
#include <linux/slab.h>
#include <flash_api.h>

#define PRINTK(...)
//#define PRINTK printk

#define DRIVER_NAME     "brcmnand"
#define DRIVER_INFO     "Broadcom NAND controller"

extern int setup_mtd_parts(struct mtd_info* mtd);

static int brcmnanddrv_probe(struct platform_device *pdev);
static int brcmnanddrv_remove(struct platform_device *pdev);

static struct platform_driver brcmnand_platform_driver =
{
	.probe		= brcmnanddrv_probe,
	.remove		= brcmnanddrv_remove,
	.driver		=
	{
		.name	= DRIVER_NAME,
	},
};

static struct resource brcmnand_resources[] =
{
	[0] = {
		.name	= DRIVER_NAME,
		.flags	= IORESOURCE_MEM,
	},
};

static struct brcmnand_info {
	struct mtd_info mtd;
	struct brcmnand_chip brcmnand;
	int nr_parts;
	struct mtd_partition* parts;
} *gNandInfo[NUM_NAND_CS];

int gNandCS[NAND_MAX_CS];
/* Number of NAND chips, only applicable to v1.0+ NAND controller */
int gNumNand   = 0;
int gClearBBT  = 0;
char gClearCET = 0;
uint32_t gNandTiming1[NAND_MAX_CS], gNandTiming2[NAND_MAX_CS];
uint32_t gAccControl[NAND_MAX_CS],  gNandConfig[NAND_MAX_CS];

static unsigned long t1[NAND_MAX_CS] = { 0 };
static int nt1 = 0;
static unsigned long t2[NAND_MAX_CS] = { 0 };
static int nt2 = 0;
static unsigned long acc[NAND_MAX_CS] = { 0 };
static int nacc = 0;
static unsigned long nandcfg[NAND_MAX_CS] = { 0 };
static int ncfg = 0;
static void* gPageBuffer = NULL;

static int brcmnanddrv_probe(struct platform_device *pdev)
{
	static int csi = 0;     // Index into dev/nandInfo array
	int cs = 0;             // Chip Select
	int err = 0;
	struct brcmnand_info* info = NULL;
	static struct brcmnand_ctrl* ctrl = (struct brcmnand_ctrl*)0;

	if (!gPageBuffer && (gPageBuffer = kmalloc(sizeof(struct brcmnand_buffers), GFP_KERNEL)) == NULL) {
		return -ENOMEM;
	}

	if ( (ctrl = kmalloc(sizeof(struct brcmnand_ctrl), GFP_KERNEL)) != NULL) {
		memset(ctrl, 0, sizeof(struct brcmnand_ctrl));
		ctrl->state = FL_READY;
		init_waitqueue_head(&ctrl->wq);
		spin_lock_init(&ctrl->chip_lock);

		if ((info = kmalloc(sizeof(struct brcmnand_info), GFP_KERNEL)) != NULL) {
			gNandInfo[csi] = info;
			memset(info, 0, sizeof(struct brcmnand_info));
			info->brcmnand.ctrl = ctrl;
			info->brcmnand.ctrl->numchips = gNumNand = 1;
			info->brcmnand.csi = csi;

			/* For now all devices share the same buffer */
			info->brcmnand.ctrl->buffers = (struct brcmnand_buffers*)gPageBuffer;

			info->brcmnand.ctrl->numchips = gNumNand;
			info->brcmnand.chip_shift = 0; // Only 1 chip
			info->brcmnand.priv = &info->mtd;
			info->mtd.name = dev_name(&pdev->dev);
			info->mtd.priv = &info->brcmnand;
			info->mtd.owner = THIS_MODULE;

			/* Enable the following for a flash based bad block table */
			info->brcmnand.options |= NAND_BBT_USE_FLASH;

			/* Each chip now will have its own BBT (per mtd handle) */
			if (brcmnand_scan(&info->mtd, cs, gNumNand) == 0) {
				PRINTK("Master size=%08llx\n", info->mtd.size);
				setup_mtd_parts(&info->mtd);
				dev_set_drvdata(&pdev->dev, info);
			}else
				err = -ENXIO;
		}else
			err = -ENOMEM;
	}else
		err = -ENOMEM;

	if (err) {
		if (gPageBuffer) {
			kfree(gPageBuffer);
			gPageBuffer = NULL;
		}

		if (ctrl) {
			kfree(ctrl);
			ctrl = NULL;
		}

		if (info) {
			kfree(info);
			info = NULL;
		}
	}

	return err;
}

static int brcmnanddrv_remove(struct platform_device *pdev)
{
	struct brcmnand_info *info = dev_get_drvdata(&pdev->dev);

	dev_set_drvdata(&pdev->dev, NULL);

	if (info) {
		mtd_device_unregister(&info->mtd);

		brcmnand_release(&info->mtd);
		kfree(gPageBuffer);
		kfree(info);
	}

	return 0;
}

static int __init brcmnanddrv_init(void)
{
	int ret = 0;
	int csi;
	int ncsi;
	char cmd[32] = "\0";
	struct platform_device *pdev;

	if (flash_get_flash_type() != FLASH_IFC_NAND)
		return -ENODEV;

	kerSysBlParmsGetStr(NAND_COMMAND_NAME, cmd, sizeof(cmd));
	PRINTK("%s: brcmnanddrv_init - NANDCMD='%s'\n", __FUNCTION__, cmd);

	if (cmd[0]) {
		if (strcmp(cmd, "rescan") == 0)
			gClearBBT = 1;
		else if (strcmp(cmd, "showbbt") == 0)
			gClearBBT = 2;
		else if (strcmp(cmd, "eraseall") == 0)
			gClearBBT = 8;
		else if (strcmp(cmd, "erase") == 0)
			gClearBBT = 7;
		else if (strcmp(cmd, "clearbbt") == 0)
			gClearBBT = 9;
		else if (strcmp(cmd, "showcet") == 0)
			gClearCET = 1;
		else if (strcmp(cmd, "resetcet") == 0)
			gClearCET = 2;
		else if (strcmp(cmd, "disablecet") == 0)
			gClearCET = 3;
		else
			printk(KERN_WARNING "%s: unknown command '%s'\n",
			       __FUNCTION__, cmd);
	}

	for (csi = 0; csi < NAND_MAX_CS; csi++) {
		gNandTiming1[csi] = 0;
		gNandTiming2[csi] = 0;
		gAccControl[csi]  = 0;
		gNandConfig[csi]  = 0;
	}

	if (nacc == 1)
		PRINTK("%s: nacc=%d, gAccControl[0]=%08lx, gNandConfig[0]=%08lx\n",
		       __FUNCTION__, nacc, acc[0], nandcfg[0]);

	if (nacc > 1)
		PRINTK("%s: nacc=%d, gAccControl[1]=%08lx, gNandConfig[1]=%08lx\n",
		       __FUNCTION__, nacc, acc[1], nandcfg[1]);

	for (csi = 0; csi < nacc; csi++)
		gAccControl[csi] = acc[csi];

	for (csi = 0; csi < ncfg; csi++)
		gNandConfig[csi] = nandcfg[csi];

	ncsi = max(nt1, nt2);

	for (csi = 0; csi < ncsi; csi++) {
		if (nt1 && csi < nt1)
			gNandTiming1[csi] = t1[csi];

		if (nt2 && csi < nt2)
			gNandTiming2[csi] = t2[csi];

	}

	printk(KERN_INFO DRIVER_INFO " (BrcmNand Controller)\n");

	if ( (pdev = platform_device_alloc(DRIVER_NAME, 0)) != NULL ) {
		platform_device_add(pdev);
		platform_device_put(pdev);
		ret = platform_driver_register(&brcmnand_platform_driver);

		brcmnand_resources[0].start = BPHYSADDR(BCHP_NAND_REG_START);
		brcmnand_resources[0].end = BPHYSADDR(BCHP_NAND_REG_END) + 3;

		if (ret >= 0)
			request_resource(&iomem_resource, &brcmnand_resources[0]);
		else
			printk("brcmnanddrv_init: driver_register failed, err=%d\n", ret);
	}else
		ret = -ENODEV;
	return ret;
}

static void __exit brcmnanddrv_exit(void)
{
	release_resource(&brcmnand_resources[0]);
	platform_driver_unregister(&brcmnand_platform_driver);
}

module_init(brcmnanddrv_init);
module_exit(brcmnanddrv_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Ton Truong <ttruong@broadcom.com>");
MODULE_DESCRIPTION("Broadcom NAND flash driver");

#endif //CONFIG_BCM_KF_MTD_BCMNAND
