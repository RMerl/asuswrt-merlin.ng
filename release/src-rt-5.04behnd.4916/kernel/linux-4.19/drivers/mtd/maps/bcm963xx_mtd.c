#if defined(CONFIG_BCM_KF_MTD_BCMNAND)
#include <linux/module.h>
#include <linux/mtd/mtd.h>

#include <linux/mtd/rawnand.h>

#include <linux/spi/spi.h>
#include <linux/spi/flash.h>

#include <board.h>

#include <flash_api.h>

#define PRINTK(...)
//#define PRINTK printk

extern int setup_mtd_parts(struct mtd_info * mtd);
extern int bcmspinand_probe(struct mtd_info * mtd);

static int __init mtd_init(void)
{
    static struct mtd_info * mtd;
    static struct nand_chip * nand;

    /* If SPI NAND FLASH is present then register the device. Otherwise do nothing */
    if (FLASH_IFC_SPINAND != flash_get_flash_type())
        return -ENODEV;

    if ((nand = kmalloc(sizeof(struct nand_chip), GFP_KERNEL)) == NULL)
    {
        printk("Unable to allocate SPI NAND dev structure.\n");
        return -ENOMEM;
    }

    memset(nand, 0, sizeof(struct nand_chip));

    mtd = nand_to_mtd(nand); // mtd is now part of the nand structure
    static struct nand_controller * controller;
    static struct nand_controller_ops * cops;
    static struct nand_manufacturer * desc;

    if ( ((controller = kmalloc(sizeof(struct nand_controller), GFP_KERNEL)) == NULL) ||
         ((cops = kmalloc(sizeof(struct nand_controller_ops), GFP_KERNEL)) == NULL) ||
         ((desc = kmalloc(sizeof(struct nand_manufacturer), GFP_KERNEL)) == NULL) /* ||
         ((mops = kmalloc(sizeof(struct nand_manufacturer_ops), GFP_KERNEL)) == NULL)
         ((wq = kmalloc(sizeof(struct wait_queue_head), GFP_KERNEL)) == NULL) */ )
    {
        printk("Unable to allocate SPI NAND dev structure.\n");
        if (controller)
            kfree(controller);
        if (cops)
            kfree(cops);
        if(desc)
            kfree(desc);
        return -ENOMEM;
    }

    controller->lock = (spinlock_t){0};
    spin_lock_init(&controller->lock);
    init_waitqueue_head(&controller->wq);

    memset(cops, 0, sizeof(struct nand_controller_ops));
    memset(desc, 0, sizeof(struct nand_manufacturer));

    nand->controller = controller;
    nand->controller->active = nand;
    nand->controller->ops = cops;

    nand->state == FL_READY;
    nand->manufacturer.desc = desc;

    bcmspinand_probe(mtd);

    /* Scan to check existence of the nand device */
    nand_scan(nand, 0);
    {
        // nand->init_size(mtd, nand, NULL); // override possibly incorrect values detected by Linux NAND driver anandg

        kerSysNvRamLoad(mtd);

        setup_mtd_parts(mtd);
    }

    return 0;
}

module_init(mtd_init);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("David Regan");
MODULE_DESCRIPTION("MTD map and partitions SPI NAND");

#endif /* CONFIG_BCM_KF_MTD_BCMNAND */
