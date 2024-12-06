// SPDX-License-Identifier: GPL-2.0+ OR MIT
/*
   Copyright (c) 2023 Broadcom 
   All Rights Reserved
   <:label-BRCM:2023:DUAL/GPL:standard
   
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
/*****************************************************************************
 *	Description:
 *		 Broadcom GPS Driver
 *****************************************************************************/
// #define DEBUG
#include <linux/device.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of_platform.h>
#include <linux/gpio/consumer.h>
#include <linux/stat.h>
#include <linux/sysfs.h>
#include <linux/slab.h>
#include <linux/miscdevice.h>
#include <linux/poll.h>
#include <linux/of_gpio.h>
#include <linux/version.h>
#include <bcm_bca_extintr.h>
#include <linux/spi/spi.h>

/****************************************************************************
 * PRIVATE SYMBOLIC CONSTANTS
 ****************************************************************************/
#define HOST_REQ_GPIO_OF_NAME "ext_irq"
#define NSTANDBY_GPIO_OF_NAME "reset"
#define HOST_REQ_GPIO_DEV_NAME "bcmgps_host_req"

/****************************************************************************
 * PRIVATE FUNCTION PROTOTYPES
 ****************************************************************************/
static ssize_t bcmgps_host_req_show(struct device *dev,
									struct device_attribute *attr, char *buf);
static ssize_t bcmgps_nstandby_show(struct device *dev,
									struct device_attribute *attr, char *buf);
static ssize_t bcmgps_nstandby_store(struct device *dev,
									 struct device_attribute *attr,
									 const char *buf, size_t count);
static ssize_t spidev_show(struct class *c, struct class_attribute *attr,
						   char *data);
static ssize_t raw_debug_show(struct class *c, struct class_attribute *attr,
							  char *data);
static ssize_t raw_debug_store(struct class *class,
							   struct class_attribute *attr, const char *buf,
							   size_t size);

/****************************************************************************
 * PRIVATE STATIC VARIABLES
 ****************************************************************************/
DECLARE_WAIT_QUEUE_HEAD(wait_queue_host_req);
static DEVICE_ATTR(host_req, S_IRUGO, bcmgps_host_req_show, NULL);
static DEVICE_ATTR(nstandby, (S_IRUGO | S_IWUSR), bcmgps_nstandby_show,
				   bcmgps_nstandby_store);
static CLASS_ATTR_RO(spidev);
static CLASS_ATTR_RW(raw_debug);

/****************************************************************************
 * PRIVATE TYPE DEFINITIONS
 ****************************************************************************/
struct bcmgps_priv {
	struct device *dev;
	struct class *sysfs_class;
	struct device *sysfs_dev;
	struct gpio_desc *gpiod_nstandby;
	struct gpio_desc *gpiod_host_req;
	int host_req_intr;
	bool is_host_req_rising;
	struct miscdevice misc_host_req;
	int raw_debug;
};
static struct bcmgps_priv *g_priv;

/* Declare attributes table (listing in fact sysfs files) */
static struct attribute *bcmgps_gpio_attrs[] = {
	&dev_attr_host_req.attr,
	&dev_attr_nstandby.attr,
	NULL};

/* Declare attribute group (specifying in fact sysfs directory) */
static struct attribute_group bcmgps_gpio_group = {
	.attrs = bcmgps_gpio_attrs,
};

static const struct of_device_id of_platform_bcmgps_table[] = {
	{ .compatible = "brcm, bcmgps", },
	{/* end of list */},
};
MODULE_DEVICE_TABLE(of, of_platform_bcmgps_table);

/****************************************************************************
 * HOST_REQ MISC DEVICE FILE OPERATION FUNCTIONS
 ****************************************************************************/
static int 
bcmgps_host_req_open(struct inode *inode, struct file *filp)
{
	struct bcmgps_priv *priv = container_of(filp->private_data,
					 struct bcmgps_priv, misc_host_req);

	filp->private_data = priv;

	return 0;
}

static int
bcmgps_host_req_release(struct inode *inode, struct file *filp)
{
	return 0;
}

static ssize_t
bcmgps_host_req_read(struct file *filp,
					 char __user *buf, size_t size, loff_t *ppos)
{
	struct bcmgps_priv *priv = filp->private_data;
	unsigned char value;

	value = gpiod_get_value_cansleep(priv->gpiod_host_req);
	size = sizeof(value);
	return copy_to_user(buf, &value, size) == 0 ? size : 0;
}

static unsigned int
bcmgps_host_req_poll(struct file *filp, poll_table *wait)
{
	struct bcmgps_priv *priv = filp->private_data;
	__poll_t mask = 0;

	dev_dbg(priv->dev, "%s(): poll_wait\n", __func__);

	poll_wait(filp, &wait_queue_host_req, wait);

	if (priv->is_host_req_rising == true) {
		priv->is_host_req_rising = false;
		mask |= (POLLIN | POLLRDNORM);
	}

	return mask;
}

static const struct file_operations bcmgps_host_req_fops = {
	.owner = THIS_MODULE,
	.open = bcmgps_host_req_open,
	.release = bcmgps_host_req_release,
	.read = bcmgps_host_req_read,
	.poll = bcmgps_host_req_poll,
};

/****************************************************************************
 * HOST_REQ ISR FUNCTION
 ****************************************************************************/
static irqreturn_t
bcmgps_host_req_isr(int irq, void *arg)
{
	struct bcmgps_priv *priv = arg;

	bcm_bca_extintr_mask(irq);

	dev_dbg(priv->dev, "%s(): interrupt called irq %d!\n", __func__, irq);

	priv->is_host_req_rising = true;

	/* wake up the waitqueue */
	wake_up(&wait_queue_host_req);

	bcm_bca_extintr_clear(irq);
	bcm_bca_extintr_unmask(irq);

	return IRQ_HANDLED;
}

/****************************************************************************
 * SYSFS FUNCTIONS
 ****************************************************************************/
static ssize_t
bcmgps_host_req_show(struct device *dev,
					 struct device_attribute *attr, char *buf)
{
	struct bcmgps_priv *priv = dev_get_drvdata(dev);
	int value = -1;

	value = gpiod_get_value_cansleep(priv->gpiod_host_req);

	return sprintf(buf, "%d\n", value);
}

static ssize_t
bcmgps_nstandby_show(struct device *dev,
					 struct device_attribute *attr, char *buf)
{
	struct bcmgps_priv *priv = dev_get_drvdata(dev);
	int value = -1;

	value = gpiod_get_value(priv->gpiod_nstandby);

	return sprintf(buf, "%d\n", value);
}

static ssize_t
bcmgps_nstandby_store(struct device *dev,
					  struct device_attribute *attr,
					  const char *buf,
					  size_t count)
{
	struct bcmgps_priv *priv = dev_get_drvdata(dev);
	long value = 0;
	int ret;

	ret = kstrtol(buf, 10, &value);
	if (ret != 0 || (value != 0 && value != 1)) {
		dev_err(dev, "failed to convert string(=%s) to long(=%ld). (ret=%d)\n",
				buf, value, ret);
		return -1;
	}

	dev_dbg(dev, "set nstandby to %ld\n", value);

	gpiod_set_value(priv->gpiod_nstandby, (int)value);

	return count;
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 15, 0))
static int
__spi_of_device_match(struct device *dev, const void *data)
#else
static int
__spi_of_device_match(struct device *dev, void *data)
#endif
{
	return (dev->of_node == data);
}

static ssize_t
spidev_show(struct class *class,
			struct class_attribute *attr, char *buf)
{
	struct bcmgps_priv *priv = g_priv;
	struct device *gdev, *sdev;
	struct device_node *snp;
	struct spi_device *spi_dev;

	if (!priv)
		return sprintf(buf, "none\n");

	gdev = priv->dev;

	/* get spi-bus device node from this module */
	snp = of_parse_phandle(gdev->of_node, "spi-bus", 0);
	if (!snp) {
		dev_err(gdev, "failed to get spi-bus node in bcmgps\n");
		return sprintf(buf, "none\n");
	}
	/* get device with spi_bus type */
	sdev = bus_find_device(&spi_bus_type, NULL, snp, __spi_of_device_match);
	if (!sdev) {
		dev_err(gdev, "failed to find spi-bus in spi-node of bcmgps\n");
		return sprintf(buf, "none\n");
	}
	spi_dev = to_spi_device(sdev);
	put_device(&spi_dev->dev);
	return sprintf(buf, "/dev/spidev%d.%d\n",
				   spi_dev->master->bus_num, spi_dev->chip_select);
}

static ssize_t
raw_debug_show(struct class *class,
			   struct class_attribute *attr, char *buf)
{
	struct bcmgps_priv *priv = g_priv;

	return sprintf(buf, "%lu\n", priv->raw_debug);
}

static ssize_t
raw_debug_store(struct class *class, struct class_attribute *attr,
				const char *buf, size_t size)
{
	struct bcmgps_priv *priv = g_priv;
	unsigned long raw_debug;
	ssize_t result;

	result = sscanf(buf, "%lu", &raw_debug);
	if (result != 1)
		return -EINVAL;

	priv->raw_debug = raw_debug;

	return size;
}

/****************************************************************************
 * PLATFORM DEVIDE FUNCTIONS
 ****************************************************************************/
static int
bcmgps_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct bcmgps_priv *priv;
	int status, is_group, is_misc_reg;
	int is_spidev_file, is_raw_debug_file;

	/* allocate private data */
	priv = kzalloc(sizeof(*priv), GFP_KERNEL);
	if (!priv)
		return -ENOMEM;
	memset(priv, 0, sizeof(*priv));

	priv->dev = dev;
	priv->host_req_intr = -1;
	priv->is_host_req_rising = false;
	priv->raw_debug = 0;
	g_priv = priv;

	/* sysfs: create class sysfs under /sys/class */
	priv->sysfs_class = class_create(THIS_MODULE, "bcmgps");
	if (IS_ERR(priv->sysfs_class)) {
		dev_err(dev, "failed to create class for \"bcmgps\".\n");
		status =  PTR_ERR(priv->sysfs_class);
		goto err_unlock_return;
	}

	/* sysfs: create device under /sys/class/bcmgps */
	priv->sysfs_dev = device_create(priv->sysfs_class, dev, 0, priv, "gpio");
	status = PTR_ERR_OR_ZERO(priv->sysfs_dev);
	if (status) {
		dev_err(dev, "failed to create device under class\n");
		goto err_unlock_return;
	}

	/* sysfs: create group under /sys/class/bcmgps/gpio */
	is_group = sysfs_create_group(&priv->sysfs_dev->kobj, &bcmgps_gpio_group);
	if (is_group) {
		dev_err(dev, "failed to create group under sysfs\n");
		status = is_group;
		goto err_unlock_return;
	}

	/* sysfs: create class file for reading spidev name */
	is_spidev_file = class_create_file(priv->sysfs_class, &class_attr_spidev);
	if (is_spidev_file) {
		dev_err(dev, "failed to create class file(spidev) under sysfs\n");
		status = is_spidev_file;
		goto err_unlock_return;
	}

	/* sysfs: create class file for raw debug */
	is_raw_debug_file = class_create_file(priv->sysfs_class, &class_attr_raw_debug);
	if (is_raw_debug_file) {
		dev_err(dev, "failed to create class file(raw_debug) under sysfs\n");
		status = is_raw_debug_file;
		goto err_unlock_return;
	}

	/* set private data to device */
	platform_set_drvdata(pdev, priv);

	/* nstandby: get gpio description and set low level */
	priv->gpiod_nstandby = devm_gpiod_get_optional(dev,
												   NSTANDBY_GPIO_OF_NAME,
												   GPIOD_ASIS);
	if (IS_ERR(priv->gpiod_nstandby)) {
		status = PTR_ERR(priv->gpiod_nstandby);
		dev_err(dev, "failed to get nstandby's gpio desc\n");
		goto err_unlock_return;
	}
	gpiod_direction_output(priv->gpiod_nstandby, 0);

	/* host_req: request external interrupt and get gpio description */
	priv->host_req_intr = bcm_bca_extintr_request(dev,
												  NULL,
												  HOST_REQ_GPIO_OF_NAME,
												  bcmgps_host_req_isr,
												  priv,
												  dev_name(dev),
												  NULL);
	if (priv->host_req_intr < 0) {
		dev_err(dev, "failed to request extintr for host_req (irq=%d)\n",
				priv->host_req_intr);
		status = priv->host_req_intr;
		goto err_unlock_return;
	}
	priv->gpiod_host_req = bcm_bca_extintr_get_gpiod(priv->host_req_intr);

	/* host_req: register misc device for host_req gpio */
	priv->misc_host_req.minor = MISC_DYNAMIC_MINOR;
	priv->misc_host_req.name = (const char *)HOST_REQ_GPIO_DEV_NAME;
	priv->misc_host_req.fops = &bcmgps_host_req_fops;
	is_misc_reg = misc_register(&priv->misc_host_req);
	if (is_misc_reg) {
		dev_err(dev, "failed to register host_req misc (err=%d)\n", status);
		status = is_misc_reg;
		goto err_unlock_return;
	}

	dev_info(dev,"Broadcom GPS GPIO driver is registered (irq=%d)\n", priv->host_req_intr);

	return 0;

err_unlock_return:
	if (!is_misc_reg)
		misc_deregister(&priv->misc_host_req);
	if (priv->host_req_intr > 0)
		bcm_bca_extintr_free(dev, priv->host_req_intr, priv);
	if (!is_spidev_file)
		class_remove_file(priv->sysfs_class, &class_attr_spidev);
	if (!is_raw_debug_file)
		class_remove_file(priv->sysfs_class, &class_attr_raw_debug);
	if (!is_group)
		sysfs_remove_group(&priv->sysfs_dev->kobj, &bcmgps_gpio_group);
	if (priv->sysfs_dev)
		device_destroy(priv->sysfs_class, 0);
	if (priv->sysfs_class)
		class_destroy(priv->sysfs_class);
	kfree(priv);
	g_priv = NULL;

	return status;
}

static int
bcmgps_remove(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct bcmgps_priv *priv = dev_get_drvdata(dev);

	misc_deregister(&priv->misc_host_req);
	if (priv->host_req_intr > 0)
		bcm_bca_extintr_free(dev, priv->host_req_intr, priv);
	class_remove_file(priv->sysfs_class, &class_attr_spidev);
	class_remove_file(priv->sysfs_class, &class_attr_raw_debug);
	sysfs_remove_group(&priv->sysfs_dev->kobj, &bcmgps_gpio_group);
	device_destroy(priv->sysfs_class, 0);
	class_destroy(priv->sysfs_class);

	kfree(priv);
	g_priv = NULL;

	return 0;
}

struct platform_driver of_platform_bcmgps_driver = {
	.driver = {
		.name = "brcmgps",
		.of_match_table = of_platform_bcmgps_table,
	},
	.probe = bcmgps_probe,
	.remove = bcmgps_remove,
};

static int __init bcmgps_init(void)
{
	pr_info("Broadcom GPS GPIO driver\n");
	return platform_driver_register(&of_platform_bcmgps_driver);
}

static void __exit bcmgps_exit(void)
{
	platform_driver_unregister(&of_platform_bcmgps_driver);
}

module_init(bcmgps_init);
module_exit(bcmgps_exit);
MODULE_LICENSE("GPL");
