#if defined(CONFIG_BCM_KF_MTD_BCMNAND)
#include <linux/module.h>
#include <linux/mtd/mtd.h>

#include <linux/mtd/nand.h>


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
    struct mtd_info * mtd;
    struct nand_chip * nand;

    /* If SPI NAND FLASH is present then register the device. Otherwise do nothing */
    if (FLASH_IFC_SPINAND != flash_get_flash_type())
        return -ENODEV;

    if (((mtd = kmalloc(sizeof(struct mtd_info), GFP_KERNEL)) == NULL) ||
        ((nand = kmalloc(sizeof(struct nand_chip), GFP_KERNEL)) == NULL))
    {
        printk("Unable to allocate SPI NAND dev structure.\n");
        return -ENOMEM;
    }

    memset(mtd, 0, sizeof(struct mtd_info));
    memset(nand, 0, sizeof(struct nand_chip));

    mtd->priv = nand;

    bcmspinand_probe(mtd);

    /* Scan to check existence of the nand device */
    if(nand_scan(mtd, 1))
    {
        nand->init_size(mtd, nand, NULL); // override possibly incorrect values detected by Linux NAND driver

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
