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
#include "opticaldet.h"
extern void trxbus_register_detect(void *(*insert)(void *dev, int is_pmd));
void *opticaldet_trx_register(void *dev, int is_pmd);

#define OPTICAL_DET_DEV_MAJOR 320
#define OPTICAL_DET_DEV_CLASS "opticaldetect"

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

                if (trx_get_full_info(0, &trx_info))
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

static int __init detect_init(void)
{
    int ret;

    trxbus_register_detect(opticaldet_trx_register);

    ret = register_chrdev(OPTICAL_DET_DEV_MAJOR, OPTICAL_DET_DEV_CLASS, &detect_file_ops);
    if (ret) {
        printk(KERN_ALERT "\nOptical detection module failed to load.\n");
    } else {
        printk(KERN_INFO "\nOptical detection module loaded.\n");
    }

    return ret;
}
module_init(detect_init);

static void __exit detect_exit(void)
{
    unregister_chrdev(OPTICAL_DET_DEV_MAJOR, OPTICAL_DET_DEV_CLASS);
    trxbus_register_detect(NULL);
    printk(KERN_INFO "\nOptical detection module unloaded.\n");
}
module_exit(detect_exit);

MODULE_AUTHOR("Broadcom");
MODULE_DESCRIPTION("Optical device detection driver");
MODULE_LICENSE("GPL");

