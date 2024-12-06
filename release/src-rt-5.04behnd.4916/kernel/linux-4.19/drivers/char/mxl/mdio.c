/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 *
 * Copyright 2020, ASUSTeK Inc.
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND ASUS GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 */

#include <linux/module.h>

#include <linux/types.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/fcntl.h>
#include <linux/init.h>
#include <linux/mutex.h>
#include <asm/uaccess.h>
#include "bcm_OS_Deps.h"
#include <board.h>
#include "phy/mdio_drv_sf2.h"

MODULE_DESCRIPTION("MaxLinear MxL86252C support");
MODULE_AUTHOR("ASUS");
MODULE_LICENSE("GPL");

typedef unsigned long long	mxl_uint64;
typedef long long		mxl_int64;
typedef unsigned int		mxl_uint32;
typedef int			mxl_int32;
typedef unsigned short		mxl_uint16;
typedef short			mxl_int16;
typedef unsigned char		mxl_uint8;
typedef char			mxl_int8;

typedef struct {
	uint16_t addr;
	uint16_t dev;
	uint16_t reg;
	uint16_t val;
} mdio_data;

/* MDC/MDIO, redefine/implement the following Macro */
#define MDC_MDIO_WRITE_C22(phyID, regID, data) mdio_write_c22_register(phyID, regID, data)
#define MDC_MDIO_READ_C22(phyID, regID, pData) mdio_read_c22_register(phyID, regID, pData)
#define MDC_MDIO_WRITE_C45(phyID, devID, regID, data) mdio_write_c45_register(phyID, devID, regID, data)
#define MDC_MDIO_READ_C45(phyID, devID, regID, pData) mdio_read_c45_register(phyID, devID, regID, pData)

int mxl_ext_swctl_init(void)
{
	return 0;
}

static int mxl_ioctl(struct inode *inode, struct file *flip, unsigned int command, unsigned long arg)
{
	mdio_data data;
	int ret;

	switch(command) {
	case 0:	// c45 read
		raw_copy_from_user(&data, (mdio_data __user *)arg, sizeof(data));
#if 0
		printk("kernel addr: 0x%x ", data.addr);
		printk("kernel dev: 0x%x ", data.dev);
		printk("kernel reg: 0x%x ", data.reg);
		printk("\n");
#endif
		ret = mdio_read_c45_register(data.addr, data.dev, data.reg, &data.val);
#if 0
		printk("value: 0x%x\n", data.val);
#endif
		raw_copy_to_user((mdio_data __user *)arg, &data, sizeof(data));

		break;
	case 1:	// c45 write
		raw_copy_from_user(&data, (mdio_data __user *)arg, sizeof(data));
#if 0
		printk("addr: 0x%x ", data.addr);
		printk("dev: 0x%x ", data.dev);
		printk("reg: 0x%x ", data.reg);
		printk("val: 0x%x ", data.val);
		printk("\n");
#endif
		ret = mdio_write_c45_register(data.addr, data.dev, data.reg, data.val);
#if 0
		if (!ret)
			printk("mdio_write_c45_register successfully\n");
#endif
		raw_copy_to_user((mdio_data __user *)arg, &data, sizeof(data));

		break;
	case 4: // c22 read
		raw_copy_from_user(&data, (mdio_data __user *)arg, sizeof(data));
#if 0
		printk("addr: 0x%x ", data.addr);
		printk("reg: 0x%x ", data.reg);
		printk("\n");
#endif
		ret = mdio_read_c22_register(data.addr, data.reg, &data.val);
#if 0
		printk("value: 0x%x\n", data.val);
#endif
		raw_copy_to_user((mdio_data __user *)arg, &data, sizeof(data));

		break;
	case 3: // c22 write
		raw_copy_from_user(&data, (mdio_data __user *)arg, sizeof(data));
#if 0
		printk("addr: 0x%x ", data.addr);
		printk("reg: 0x%x ", data.reg);
		printk("val: 0x%x ", data.val);
		printk("\n");
#endif
		ret = mdio_write_c22_register(data.addr, data.reg, data.val);
#if 0
		if (!ret)
			printk("mdio_write_c22_register successfully\n");
#endif
		raw_copy_to_user((mdio_data __user *)arg, &data, sizeof(data));

		break;
	default:
		return -EINVAL;
	}

	return 0;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 33)
static DEFINE_MUTEX(mxlswitch_mutex);

static long unlocked_mxl_ioctl(struct file *filep, unsigned int cmd, unsigned long arg)
{
	struct inode *inode;
	long rt;

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 0, 0)
	inode = filep->f_dentry->d_inode;
#else
	inode = file_inode(filep);
#endif
	mutex_lock(&mxlswitch_mutex);
	rt = mxl_ioctl( inode, filep, cmd, arg );
	mutex_unlock(&mxlswitch_mutex);

	return rt;
}
#endif

static int mxl_open(struct inode *inode, struct file *filp)
{
	return 0;
}

static int mxl_release(struct inode *inode, struct file *filp)
{
	return 0;
}

static struct file_operations mxl_fops =
{
	.owner		= THIS_MODULE,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 33)
	.unlocked_ioctl	= unlocked_mxl_ioctl,
#if defined(CONFIG_COMPAT)
	.compat_ioctl	= unlocked_mxl_ioctl,
#endif
#else
	.ioctl		= mxl_ioctl,
#endif
	.open		= mxl_open,
	.release	= mxl_release,
};

static int __init mxl_init_asus(void)
{
	int ret;

	ret = register_chrdev(207, "mxlswitch", &mxl_fops );
	if (ret < 0)
		printk("mxl_init(major 207): fail to register device.\n");
	else
		printk("mxl: mxl_init entry\n");

	printk("init mxl switch %s:%d\n", __FUNCTION__, __LINE__);

	mxl_ext_swctl_init();

	return ret;
}

static void __exit mxl_exit(void)
{
	unregister_chrdev(207, "mxlswitch");

	printk("mxl driver exited\n");
}

module_init(mxl_init_asus);
module_exit(mxl_exit);

MODULE_LICENSE("GPL");
