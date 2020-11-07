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

#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include "opticaldet.h"
#include "pmd.h"


extern int opticaldetect(pmd_calibration_parameters *calibration_parameters_from_json);
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

/* The frame size is larger than 1024 bytes [-Werror=frame-larger-than=] */
static pmd_wan_type_auto_detect_settings pmd_wan_auto_detect = {0};
static long _detect_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    unsigned long *val = (unsigned long *)arg;

    switch (cmd)
    {
        case OPTICALDET_IOCTL_DETECT:
            {
                *val = copy_from_user(&pmd_wan_auto_detect, (void *)arg, sizeof(pmd_wan_type_auto_detect_settings));
                if (*val) {
                    printk(KERN_ERR "\nError in Optical WAN type auto-detection module - OPTICALDET_IOCTL_DETECT.\n");
                }
                if (pmd_wan_auto_detect.is_calibration_file_valid) {
                    *val = opticaldetect(&(pmd_wan_auto_detect.calibration_parameters_from_json));
                } else {
                    *val = opticaldetect(NULL);
                }
            }
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


#define WAN_TYPE_DET_DEV_MAJOR 340
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
