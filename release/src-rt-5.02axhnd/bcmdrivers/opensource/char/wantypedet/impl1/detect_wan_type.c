/*
<:copyright-BRCM:2018:DUAL/GPL:standard

   Copyright (c) 2018 Broadcom 
   All Rights Reserved

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License, version 2, as published by
the Free Software Foundation (the "GPL").

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.


A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.

:>
*/

#include <linux/module.h>
#include <linux/fs.h>
#include "opticaldet.h"


extern int opticaldetect(void);
#if defined(CONFIG_BCM96838)
extern int signalDetect(void);
#endif


static int _file_open(struct inode *inode, struct file *file)
{
    return 0;
}

static int _file_release(struct inode *inode, struct file *file)
{
    return 0;
}

static long _detect_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    unsigned long *val = (unsigned long *)arg;

    switch (cmd)
    {
        case OPTICALDET_IOCTL_DETECT:
            *val = opticaldetect();
            break;
#if defined(CONFIG_BCM96838)
        case OPTICALDET_IOCTL_SD:
            *val = signalDetect();
            break;
#endif
        default:
            printk("%s: ERROR: No such IOCTL", __FILE__);
            return -1;
    }

    return 0;
}

static const struct file_operations detect_file_ops =
{
    .owner = THIS_MODULE,
    .open = _file_open,
    .release = _file_release,
    .unlocked_ioctl = _detect_ioctl,
#if defined(CONFIG_COMPAT)
    .compat_ioctl = _detect_ioctl,
#endif
};


#define WAN_TYPE_DET_DEV_MAJOR 3040
#define WAN_TYPE_DET_DEV_CLASS "wantypedetect"

int __init detect_init(void)
{
    int ret;

    ret = register_chrdev(WAN_TYPE_DET_DEV_MAJOR, WAN_TYPE_DET_DEV_CLASS, &detect_file_ops);
    if (ret) {
        printk(KERN_ALERT "\nOptical WAN type auto-detection module failed to load.\n");
    } else {
        printk(KERN_INFO "\nOptical WAN type auto-detection module loaded.\n");
    }

    return ret;
}
module_init(detect_init);

static void __exit detect_exit(void)
{
    unregister_chrdev(WAN_TYPE_DET_DEV_MAJOR, WAN_TYPE_DET_DEV_CLASS);
    printk(KERN_INFO "\nOptical WAN type auto-detection module unloaded.\n");
}
module_exit(detect_exit);

MODULE_AUTHOR("Broadcom");
MODULE_DESCRIPTION("Optical WAN type auto-detection driver");
MODULE_LICENSE("GPL");
