/*
<:copyright-BRCM:2020:DUAL/GPL:standard

   Copyright (c) 2020 Broadcom 
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
#include <linux/fs.h>
#include <linux/types.h>
#include <linux/uaccess.h>
#include <linux/cdev.h>
#include "opticaldet.h"
#include "trxbus.h"

void *opticaldet_trx_register(struct device *dev, int is_pmd);

#define OPTICAL_DET_DEV_CLASS "opticaldetect"

struct detect_io_info {
    struct class *class;
    struct device *device;
    struct cdev cdev;
    int major;
};
static struct detect_io_info detect_info;

static int _file_open(struct inode *inode, struct file *file)
{
    return 0;
}

static int _file_release(struct inode *inode, struct file *file)
{
    return 0;
}

static long _file_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    switch (cmd)
    {
        case OPTDETECT_IOCTL_GET_TRX_INFO:
            {
                TRX_INFOMATION trx_info;

                memset(&trx_info, 0, sizeof(trx_info));
                if (trx_get_full_info(wan_i2c_bus_get(), &trx_info))
                {
                    return -1;
                }

                if (copy_to_user((void *)arg, (void *)&trx_info, sizeof(TRX_INFOMATION)))
                {
                    printk(KERN_ERR "%s: failed copy data to user!\n", __FUNCTION__);
                    return -1;
                }
            }
            break;

        default:
            printk(KERN_ERR "%s: ERR! unknown ioctl cmd\n", __FUNCTION__);
            return -1;
    }
    return 0;
}

static const struct file_operations detect_file_ops =
{
    .owner = THIS_MODULE,
    .open = _file_open,
    .release = _file_release,
    .unlocked_ioctl = _file_ioctl,
#if defined(CONFIG_COMPAT)
    .compat_ioctl = _file_ioctl,
#endif
};

static void detect_deinit(void)
{
    if (!IS_ERR(detect_info.device)) {
        device_destroy(detect_info.class, MKDEV(detect_info.major, 0));
        cdev_del(&detect_info.cdev);
    }

    if (!IS_ERR(detect_info.class))
        class_destroy(detect_info.class);

    if (detect_info.major)
        unregister_chrdev_region(MKDEV(detect_info.major, 0), 1);
}

static int __init detect_init(void)
{
    dev_t dev = 0;
    dev_t devno;
    int ret;

    memset(&detect_info, 0, sizeof(detect_info));

    trxbus_register_detect(opticaldet_trx_register);

    ret = alloc_chrdev_region(&dev, 0, 1, OPTICAL_DET_DEV_CLASS);
    if (ret < 0) {
        pr_err("%s:alloc_chrdev_region() failed\n", __func__);
        return -ENODEV;
    }
    detect_info.major = MAJOR(dev);

    /* create device and class */
    detect_info.class = class_create(THIS_MODULE, OPTICAL_DET_DEV_CLASS);
    if (IS_ERR(detect_info.class)) {
        ret = PTR_ERR(detect_info.class);
        pr_err("%s:Fail to create class %s, ret = %d\n", __func__,
               OPTICAL_DET_DEV_CLASS, ret);
        goto fail;
    }

    devno = MKDEV(detect_info.major, 0);
    cdev_init(&detect_info.cdev, &detect_file_ops);
    detect_info.cdev.owner = THIS_MODULE;

    ret = cdev_add(&detect_info.cdev, devno, 1);
    if (ret) {
        pr_err("%s:Fail to add cdev %s, ret = %d\n", __func__,
               OPTICAL_DET_DEV_CLASS, ret);
        goto fail;
    }

    detect_info.device = device_create(detect_info.class, NULL, devno, NULL,
                      OPTICAL_DET_DEV_CLASS);
    if (IS_ERR(detect_info.device)) {
        ret = PTR_ERR(detect_info.device);
        pr_err("%s:Fail to create device %s, ret = %d\n", __func__,
               OPTICAL_DET_DEV_CLASS, ret);
        goto fail;
    }

    pr_info("\nOptical detection module loaded.\n");

    return ret;

fail:
    detect_deinit();
    pr_alert("\nOptical detection module failed to load.\n");
    return ret;
}
module_init(detect_init);

static void __exit detect_exit(void)
{
    detect_deinit();
    trxbus_register_detect(NULL);
    pr_info("\nOptical detection module unloaded.\n");
}
module_exit(detect_exit);

MODULE_AUTHOR("Broadcom");
MODULE_DESCRIPTION("Optical device detection driver");
MODULE_LICENSE("GPL");

