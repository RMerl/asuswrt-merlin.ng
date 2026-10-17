/*
 * Linux Broadcom BCM47xx GPIO char driver
 *
 * Copyright (C) 2025, Broadcom. All Rights Reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 *
 * <<Broadcom-WL-IPTag/Open:>>
 *
 * $Id: linux_gpio.c 834183 2023-12-13 12:11:09Z $
 *
 */
#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/spinlock.h>
#include <asm/uaccess.h>

#include <typedefs.h>
#include <bcmutils.h>
#include <siutils.h>
#include <bcmdevs.h>
#include <linux/gpio.h>
#include <linux_gpio.h>

/* handle to the sb */
static si_t *gpio_sih;

/* major number assigned to the device and device handles */
static int gpio_major;
static struct class *gpiodev_class = NULL;

#ifndef GPIO_COMMON_SPINLOCK_NAME
#define GPIO_COMMON_SPINLOCK_NAME lock
static DEFINE_SPINLOCK(GPIO_COMMON_SPINLOCK_NAME);
#else
extern spinlock_t GPIO_COMMON_SPINLOCK_NAME;
#endif

static int
gpio_open(struct inode *inode, struct file * file)
{
	MOD_INC_USE_COUNT;
	return 0;
}

static int
gpio_release(struct inode *inode, struct file * file)
{
	MOD_DEC_USE_COUNT;
	return 0;
}

static long
gpio_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	struct gpio_ioctl gpioioc;
	unsigned long flags;

	if (copy_from_user(&gpioioc, (struct gpio_ioctl *)arg, sizeof(struct gpio_ioctl)))
		return -EFAULT;

	spin_lock_irqsave(&GPIO_COMMON_SPINLOCK_NAME, flags);
	switch (cmd) {
		case GPIO_IOC_RESERVE:
			gpioioc.val = si_gpioreserve(gpio_sih, gpioioc.mask, GPIO_APP_PRIORITY);
			break;
		case GPIO_IOC_RELEASE:
			/*
			 * releasing the gpio doesn't change the current
			 * value on the GPIO last write value
			 * persists till some one overwrites it
			 */
			gpioioc.val = si_gpiorelease(gpio_sih, gpioioc.mask, GPIO_APP_PRIORITY);
			break;
		case GPIO_IOC_OUT:
			gpioioc.val = si_gpioout(gpio_sih, gpioioc.mask, gpioioc.val,
			                         GPIO_APP_PRIORITY);
			break;
		case GPIO_IOC_OUTEN:
			gpioioc.val = si_gpioouten(gpio_sih, gpioioc.mask, gpioioc.val,
			                           GPIO_APP_PRIORITY);
			break;
		case GPIO_IOC_IN:
			gpioioc.val = si_gpioin(gpio_sih);
			break;
		default:
			break;
	}
	spin_unlock_irqrestore(&GPIO_COMMON_SPINLOCK_NAME, flags);
	if (copy_to_user((struct gpio_ioctl *)arg, &gpioioc, sizeof(struct gpio_ioctl)))
		return -EFAULT;

	return 0;

}

static struct file_operations gpio_fops = {
	owner:		THIS_MODULE,
	open:		gpio_open,
	release:	gpio_release,
	unlocked_ioctl:	gpio_ioctl
};

static int __init
gpio_init(void)
{
	if (!(gpio_sih = si_kattach(SI_OSH)))
		return -ENODEV;

	if ((gpio_major = register_chrdev(0, "gpio", &gpio_fops)) < 0)
		return gpio_major;

	gpiodev_class = class_create(THIS_MODULE, "gpio");
	if (IS_ERR(gpiodev_class)) {
		printk("Error creating gpio class\n");
		return -1;
	}

	/* Add the device gpio0 */
	device_create(gpiodev_class, NULL, MKDEV(gpio_major, 0), NULL, "gpio", 0);
	return 0;
}

static void __exit
gpio_exit(void)
{
	if (gpiodev_class != NULL) {
		device_destroy(gpiodev_class, MKDEV(gpio_major, 0));
		class_destroy(gpiodev_class);
	}

	gpiodev_class = NULL;
	if (gpio_major >= 0)
		unregister_chrdev(gpio_major, "gpio");
}

module_init(gpio_init);
module_exit(gpio_exit);
