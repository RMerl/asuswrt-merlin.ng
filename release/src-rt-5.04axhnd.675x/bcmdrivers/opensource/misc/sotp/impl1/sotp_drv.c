/*
<:copyright-BRCM:2018:DUAL/GPL:standard

   Copyright (c) 2018 Broadcom 
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
/***************************************************************************
* File Name  : sotp_drv.c
*
* Description: provides IOCTLS to provide sotp read/write to userspace
*
*
***************************************************************************/

/* Includes. */
#include <linux/version.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/module.h>
#include <linux/uaccess.h>
#include <linux/compat.h>
#include <linux/delay.h>
#include <linux/proc_fs.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/crc32.h>
#include <linux/of_device.h>
#include <linux/of_address.h>
#include <linux/miscdevice.h>
#include <bcmtypes.h>
#include <bcm_sotp.h>
#include <linux/bcm_log.h>
#include "sotp_base_defs.h"

/***************************************************************************
* Types and Defines
***************************************************************************/
#define SOTP_DBG_ENABLE     0
#if SOTP_DBG_ENABLE
#   define SOTP_DBG(fmt, args...) printk( KERN_DEBUG "SOTP_drv: " fmt, ## args)
#else
#   define SOTP_DBG(fmt, args...) /* not DBGging: nothing */
#endif

/***************************************************************************
* Prototypes
***************************************************************************/
static int sotp_ioctl(struct inode *inode, struct file *flip, unsigned int command, unsigned long arg);
static long unlocked_sotp_ioctl(struct file *filep, unsigned int cmd, unsigned long arg);

/***************************************************************************
* Local Variables
***************************************************************************/
static DEFINE_MUTEX(sotpIoctlMutex);

/***************************************************************************
* Driver API
***************************************************************************/
static struct file_operations sotp_fops =
{
    .unlocked_ioctl = unlocked_sotp_ioctl,
#if defined(CONFIG_COMPAT)
    .compat_ioctl   = unlocked_sotp_ioctl,
#endif
};

static int sotp_ioctl(struct inode *inode, struct file *flip, unsigned int command, unsigned long arg)
{
    int ret = -EINVAL;
    SOTP_IOCTL_PARMS ctrlParms;
    char * bufp = NULL;
    struct miscdevice *mdev = flip->private_data;
    struct device *dev = mdev->this_device;

    SOTP_DBG("%s: entry\n", __FUNCTION__); 


    if (copy_from_user((void*)&ctrlParms, (void*)arg, sizeof(ctrlParms)) != 0) 
    {
        dev_err(dev, "Failed to copy user ctrlParms!\n");
        return ret;
    }

#if defined(CONFIG_COMPAT)
    if (is_compat_task()) 
    {
        BCM_IOC_PTR_ZERO_EXT(ctrlParms.inout_data);
    }
#endif

    SOTP_DBG("%s: SOTP_IOCTL_%s Elem:%d bufp:%px\n", __FUNCTION__, (command==SOTP_IOCTL_GET?"GET":"SET"), ctrlParms.element, ctrlParms.inout_data);

    if (!ctrlParms.inout_data) 
    {
        dev_err(dev, "inout_data is null\n");
        return ret;
    }

    bufp = kmalloc(ctrlParms.data_len, GFP_KERNEL);

    if (!bufp) 
    {
        dev_err(dev, "Kmalloc failed\n");
        return -ENOMEM;
    }


    switch (command) {
    case SOTP_IOCTL_GET:
        switch (ctrlParms.element) {
        case SOTP_MAP:
            ret = sotp_dump_map( &ctrlParms.result );
            break;
        case SOTP_ROW:
            ret = sotp_get_row_data( ctrlParms.row_addr, bufp, ctrlParms.data_len, &ctrlParms.result, ctrlParms.raw_access );
            break;
        case SOTP_REGION_READLOCK:
            ret = sotp_get_region_readlock_status( ctrlParms.region_num, &ctrlParms.result );
            break;
        case SOTP_REGION_FUSELOCK:
            ret = sotp_get_region_fuselock_status( ctrlParms.region_num, &ctrlParms.result );
            break;
        case SOTP_KEYSLOT:
            ret = sotp_get_keyslot_data( ctrlParms.keyslot_section_num, bufp, ctrlParms.data_len, &ctrlParms.result );
            break;
        case SOTP_KEYSLOT_READLOCK:
            ret = sotp_get_keyslot_readlock_status( ctrlParms.keyslot_section_num, &ctrlParms.result );
	case SOTP_ROLLBACK_LVL:
	    ret = sotp_get_rollback_lvl( bufp, &ctrlParms.result );
            break;
        default:
            ret = -1;
        }

        /* Copy SOTP data */
        copy_to_user(ctrlParms.inout_data, bufp, ctrlParms.data_len);

        break;

    case SOTP_IOCTL_SET:
        /* Copy user data */
        if( ctrlParms.data_len )
        {
            if (copy_from_user(bufp, ctrlParms.inout_data, ctrlParms.data_len) != 0) 
            {
                dev_err(dev, "Failed to copy user data buf!\n");
                kfree(bufp);
                return ret;
            }
        }  

        switch (ctrlParms.element) {
        case SOTP_ROW:
            ret = sotp_set_row_data( ctrlParms.row_addr, bufp, ctrlParms.data_len, &ctrlParms.result, ctrlParms.raw_access);
            break;
        case SOTP_REGION_READLOCK:
            ret = sotp_set_region_readlock( ctrlParms.region_num, &ctrlParms.result );
            break;
        case SOTP_REGION_FUSELOCK:
            ret = sotp_set_region_fuselock( ctrlParms.region_num, &ctrlParms.result );
            break;
        case SOTP_KEYSLOT:
            ret = sotp_set_keyslot_data( ctrlParms.keyslot_section_num, bufp, ctrlParms.data_len, &ctrlParms.result );
            break;
        case SOTP_KEYSLOT_READLOCK:
            ret = sotp_set_keyslot_readlock( ctrlParms.keyslot_section_num, &ctrlParms.result );
            break;
	case SOTP_ROLLBACK_LVL:
	    ret = sotp_set_rollback_lvl( bufp, &ctrlParms.result );
	    break;
        default:
            ret = -1;
        }
        break;
    default:
        break;
    }

    copy_to_user((void *)arg, (void *) &ctrlParms, sizeof(ctrlParms));

    return (ret);
} 

static long unlocked_sotp_ioctl(struct file *filep, unsigned int cmd, unsigned long arg)
{
    struct inode *inode;
    long rt;

    inode = file_inode(filep);
    mutex_lock(&sotpIoctlMutex);
    rt = sotp_ioctl( inode, filep, cmd, arg );
    mutex_unlock(&sotpIoctlMutex);
    
    return rt;
}

static const struct of_device_id bcmbca_sotp_of_match[] = {
    { .compatible = "brcm,sotp" },
    { },
};

MODULE_DEVICE_TABLE(of, bcmbca_sotp_of_match);

static int bcmbca_sotp_probe(struct platform_device *pdev)
{
    struct device *dev = &pdev->dev;
    const struct of_device_id *match;
    struct miscdevice *mdev;
    struct resource *r_mem;
    SotpRegs *sotp = NULL;
    int ret;

    match = of_match_device(bcmbca_sotp_of_match, dev);
    if (!match)
    {
        dev_err(dev, "SOTP dev: Failed to match\n");
        return -ENODEV;
    }

    r_mem = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    if (!r_mem)
    {
        dev_err(dev, "SOTP dev unable to get register resource.\n");
        return -ENODEV;
    }
   
    sotp = devm_ioremap_resource(dev, r_mem);
    if (IS_ERR(sotp))
    {
        dev_err(dev, "SOTP dev unable to remap register resource.\n");
        return PTR_ERR(sotp);
    }
    
    /* Initialize SOTP core */
    ret = sotp_init(sotp);
    if (ret < 0)
    {
        dev_err(dev, "SOTP dev initialization failed. Not loading driver.\n");
        return ret;
     }

    if(sotp->sotp_otp_prog_ctrl == 0xDEADBEEF && sotp->sotp_otp_ctrl_0 == 0xDEADBEEF)
    {
        dev_err(dev, "SOTP dev access not permitted. Not loading driver.\n");
        return -EPERM;
    }

    mdev = devm_kzalloc(dev, sizeof(*mdev), GFP_KERNEL);
    if (!mdev)
    {
        dev_err(dev, "Failed to allocate memory for SOTP dev\n");
        return -ENOMEM;
    }

    platform_set_drvdata(pdev, mdev);

    mdev->minor  = MISC_DYNAMIC_MINOR;
    mdev->name   = "sotp";
    mdev->fops   = &sotp_fops;
    mdev->parent = NULL;

    ret = misc_register(mdev);
    if (ret) {
        dev_err(dev, "Failed to register SOTP dev\n");
        return ret;
    }

    dev_err(dev, "SOTP dev loaded succesfully!\n");

    return ret;
}

static int bcmbca_sotp_remove(struct platform_device *pdev)
{
    struct miscdevice *mdev = platform_get_drvdata(pdev);

    misc_deregister(mdev);
    dev_info(&pdev->dev, "SOTP dev unregistered\n");

    return 0;
}

static struct platform_driver bcmbca_sotp_drv = {
	.probe = bcmbca_sotp_probe,
	.remove = bcmbca_sotp_remove,
	.driver = {
		.name = "bcmbca-sotp",
		.of_match_table = bcmbca_sotp_of_match,
	},
};

module_platform_driver(bcmbca_sotp_drv);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("Broadcom SOTP driver");

