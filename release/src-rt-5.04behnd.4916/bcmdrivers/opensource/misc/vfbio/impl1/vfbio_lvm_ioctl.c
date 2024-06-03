/*
<:copyright-BRCM:2019:DUAL/GPL:standard

   Copyright (c) 2019 Broadcom
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
/****************************************************************************
 * vFlash block IO driver - Logical Volume Manegement access from the user space
 *
 * Author: Igor Ternovsky <igor.ternovsky@broadcom.com>
*****************************************************************************/

#include <linux/version.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/dma-mapping.h>
#include <linux/cdev.h>
#include <vfbio_lvm_ioctl.h>
#include "vfbio_priv.h"

static int chrdev_major;

static long vfbio_lvm_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    int rc;
    vfbio_lvm_ioctl_param params;

    /* don't even decode wrong cmds: better returning  ENOTTY than EFAULT */
    if (_IOC_TYPE(cmd) != VFBIO_LVM_IOCTL_MAGIC)
        return -ENOTTY;

    rc = copy_from_user((char *)&params, (char *)arg, sizeof(params));
    if (rc < 0)
        return rc;

    switch (cmd)
    {
    case VFBIO_LVM_IOCTL_OP_CREATE:
        {
            params.rc = vfbio_lun_create(params.create.lun_name,
                params.create.size, &params.create.lun_id);
        }
        break;

    case VFBIO_LVM_IOCTL_OP_DELETE:
        {
            params.rc = vfbio_lun_delete(params.destroy.lun_id);
        }
        break;

    case VFBIO_LVM_IOCTL_OP_RESIZE:
        {
            params.rc = vfbio_lun_resize(params.resize.lun_id, params.resize.size);
        }
        break;

    case VFBIO_LVM_IOCTL_OP_RENAME:
        {
            params.rc = vfbio_lun_rename(params.rename.num_luns, params.rename.id_name_pairs);
        }
        break;

    case VFBIO_LVM_IOCTL_OP_CHMOD:
        {
            params.rc = vfbio_lun_chmod(params.chmod.lun_id, params.chmod.read_only);
        }
        break;

    case VFBIO_LVM_IOCTL_OP_VOLUME_INFO:
        {
            params.rc = vfbio_lun_get_info(params.lun_info.lun_id, &params.lun_info.info);
        }
        break;

    case VFBIO_LVM_IOCTL_OP_DEVICE_INFO:
        {
            params.rc = vfbio_device_get_info(&params.device_info.total_size, &params.device_info.free_size);
        }
        break;

    case VFBIO_LVM_IOCTL_OP_GET_ID:
        {
            params.rc = vfbio_lun_get_id(params.get_id.lun_name, &params.get_id.lun_id);
        }
        break;

    case VFBIO_LVM_IOCTL_OP_WRITE:
        {
            params.rc = vfbio_lun_write(params.write.lun_id, params.write.data, params.write.size);
        }
        break;
        
    case VFBIO_LVM_IOCTL_OP_GET_NEXT:
        {
            params.rc = vfbio_lun_get_next(params.get_next.lun_id, &params.get_next.lun_id);
        }
        break;

    default:
        params.rc = -EINVAL;
        break;
    }

    rc = copy_to_user((char *)arg, (char *)&params, sizeof(params));
    if (rc < 0)
        return rc;

    return 0;
}

static struct file_operations vfbio_lvm_ioctl_chrdev_fops =
{
    .owner = THIS_MODULE,
    .unlocked_ioctl = vfbio_lvm_ioctl,
    .compat_ioctl = vfbio_lvm_ioctl
};

static struct class *vfbio_cmd_class = NULL;
static struct cdev vfbio_cdev;
static struct device *vfbio_device;
static dev_t vfbio_dev_id;

int vfbio_lvm_ioctl_init(void)
{
    int rc;
    dev_t vfbio_dev_id;

    /*
     * Register your major, and accept a dynamic number.
     */
    vfbio_cmd_class = class_create(THIS_MODULE, VFBIO_CLASS_NAME);
    if (IS_ERR(vfbio_cmd_class)) {
        pr_err("Unable to class_create() for the class [%s]", VFBIO_CLASS_NAME);
        return PTR_ERR(vfbio_cmd_class);
    }

    rc = alloc_chrdev_region(&vfbio_dev_id, 0, 1, VFBIO_CLASS_NAME);
    chrdev_major = MAJOR(vfbio_dev_id);
    if (rc < 0) {
        pr_err("%s: can't alloc chrdev region\n", __FUNCTION__);
        vfbio_lvm_ioctl_exit();
        return rc;
    }

    cdev_init(&vfbio_cdev, &vfbio_lvm_ioctl_chrdev_fops);
    rc = cdev_add(&vfbio_cdev, vfbio_dev_id, 1);
    if (rc < 0) {
        pr_err("%s: can't register major %d\n", __FUNCTION__, vfbio_dev_id);
        vfbio_lvm_ioctl_exit();
        return rc;
    }

    /* not a big deal if we fail here :-) */
    vfbio_device = device_create(vfbio_cmd_class, NULL, MKDEV(chrdev_major, 0),
        NULL, VFBIO_DEV_NAME);
    if (IS_ERR(vfbio_device)) {
        pr_err("%s: can't register class device\n", __FUNCTION__);
        vfbio_device = NULL;
        vfbio_lvm_ioctl_exit();
        return PTR_ERR(vfbio_device);
    }
    printk("Registered character device %s with major %d\n", VFBIO_DEV_NAME, chrdev_major);

    return 0;
}

void vfbio_lvm_ioctl_exit(void)
{
    if (vfbio_device)
        device_destroy(vfbio_cmd_class, MKDEV(chrdev_major, 0));
    if (vfbio_cmd_class)
        class_destroy(vfbio_cmd_class);
    if (vfbio_dev_id)
        unregister_chrdev_region(vfbio_dev_id, 1);
}