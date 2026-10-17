/*********************************************************************
 * voice slic power control driver.c
 *
 * Copyright (c) 2024 Broadcom Corporation
 * All Rights Reserved
 *
 * <:label-BRCM:2024:DUAL/GPL:standard
 * 
 * Unless you and Broadcom execute a separate written software license
 * agreement governing use of this software, this software is licensed
 * to you under the terms of the GNU General Public License version 2
 * (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
 * with the following added to such license:
 * 
 *    As a special exception, the copyright holders of this software give
 *    you permission to link this software with independent modules, and
 *    to copy and distribute the resulting executable under terms of your
 *    choice, provided that you also meet, for each linked independent
 *    module, the terms and conditions of the license of that module.
 *    An independent module is a module which is not derived from this
 *    software.  The special exception does not apply to any modifications
 *    of the software.
 * 
 * Not withstanding the above, under no circumstances may you combine
 * this software in any way with any other Broadcom software provided
 * under a license other than the GPL, without Broadcom's express prior
 * written consent.
 * 
 * :>
 **********************************************************************/

#include <linux/of.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/platform_device.h>
#include <linux/of_gpio.h>
#include <linux/proc_fs.h>
#include <linux/delay.h>
#include <linux/cdev.h>

#include "bcm_voice_dt_bindings.h"
#include <bcm_slic_pwrctl.h>

#define DRV_NAME "brcm-slic-power-ctrl"

extern void bcm_voice_slicpwr_pinctrl_select_state(int);

#define BUF_STR_LEN 256

#define SLICPWR_DEVNAME "slic_pwrctrl"
#define SLICPWR_MAX_MINOR 8

struct slic_pwrctrl {
	int ctrl_gpio_status;
	struct device *dev;
	struct gpio_desc *ctrl_gpio;

};

static dev_t slic_pwrctrl_devid;
static struct class *slic_pwrctrl_class;
struct device *slic_pwrctrl_dev;
struct cdev slic_pwrctrl_cdev;
static struct slic_pwrctrl *priv;

#if ENABLE_PROC_SLIC_PWR
static struct proc_dir_entry *slic_pwrctrl_proc_dir;
#endif

static int slic_power_set(uint32_t status)
{
	int ret;
	if (!priv)
		return -ENOMEM;

	status = status ? 1:0;

	dev_notice(priv->dev,"%sabling power by GPIO_%d\n", status ? "en" : "dis",
					desc_to_gpio(priv->ctrl_gpio));

	if (status == SLIC_POWER_OFF)
		bcm_voice_slicpwr_pinctrl_select_state(status);		

	ret = gpiod_direction_output(priv->ctrl_gpio, status);
	if (ret) {
		dev_err(priv->dev, "set gpio_%d output %s failed.\n",desc_to_gpio(priv->ctrl_gpio),status ? "high":"low");
		return ret;
	}
	priv->ctrl_gpio_status = status;

	if (status == SLIC_POWER_ON)
		bcm_voice_slicpwr_pinctrl_select_state(status);

	return 0;
}

static ssize_t slic_pwrctrl_write(struct file *file, const char __user *buffer,
				  size_t count, loff_t *pos)
{
	uint32_t cmd;
	int converted,ret;
	
	converted = kstrtouint_from_user(buffer, count, 0, &cmd);
	if (converted < 0)
		return converted;

	ret = slic_power_set(cmd);
	if (ret)
		return ret;
		
	return count;
}

static ssize_t slic_pwrctrl_read(struct file *file, char __user *buffer,
				  size_t count, loff_t *pos)
{
	int size;
	char buf[BUF_STR_LEN];

	if (!priv)
		return -ENOMEM;

	/* for power control framework, SLIC no power = VOIP Off is on */
	size = scnprintf(buf, sizeof(buf), "%s",priv->ctrl_gpio_status == SLIC_POWER_OFF ? "1":"0");
	return simple_read_from_buffer(buffer, count, pos, buf, size);
}

static const struct file_operations slic_pwrctrl_fops = 
{
	.write	= slic_pwrctrl_write,
	.read 	= slic_pwrctrl_read,
};

#if ENABLE_PROC_SLIC_PWR
static ssize_t  slic_proc_power_read(struct file *filp,
				char __user *user_buf,
				size_t count, loff_t *f_pos)
{
	char buf[BUF_STR_LEN];
	size_t size;
	size_t uncopied = 0;
	ssize_t ret = 0;
	if(*f_pos) 
		return 0;
	if (!priv)
		return -ENOMEM;
	size = scnprintf(buf, sizeof(buf), "The SLIC power status = %s\n",priv->ctrl_gpio_status == SLIC_POWER_ON? "On":"Off");
	uncopied = copy_to_user(&user_buf[ret], buf, size);
	ret += (size - uncopied);
	*f_pos += ret;

	return ret;
}

static const struct proc_ops pwrctl_proc_fops = {
	.proc_read  = slic_proc_power_read,
	.proc_write = slic_pwrctrl_write,
};
#endif

static int slic_pwrctrl_remove(struct platform_device *pdev)
{
#if ENABLE_PROC_SLIC_PWR
	remove_proc_entry(PROC_SLIC_POWER_STATUS, slic_pwrctrl_proc_dir);
	remove_proc_entry(PROC_SLIC_POWER_CTL_DIR, NULL);
#endif
	return 0;
}

static int slic_pwrctrl_probe(struct platform_device *pdev)
{
	int ret;
	struct device *dev = &pdev->dev;
	struct device_node *np = pdev->dev.of_node;
#if ENABLE_PROC_SLIC_PWR
	struct proc_dir_entry *entry;
#endif

	if (!np)
		return -ENODEV;

	priv = (struct slic_pwrctrl *)devm_kzalloc(&pdev->dev, sizeof(*priv), GFP_KERNEL);
	if (!priv)
		return -ENOMEM;

	priv->ctrl_gpio = devm_gpiod_get_optional(&pdev->dev, "slicpwrctl", GPIOD_OUT_HIGH);
	if (IS_ERR_OR_NULL(priv->ctrl_gpio)) {
		dev_err(dev, "can't get SLIC power control GPIO.\n");
		return -EINVAL;
	}
	priv->ctrl_gpio_status = SLIC_POWER_ON;
	priv->dev = &pdev->dev;

	bcm_voice_slicpwr_pinctrl_select_state(SLIC_POWER_ON);

#if ENABLE_PROC_SLIC_PWR
	slic_pwrctrl_proc_dir = proc_mkdir(PROC_SLIC_POWER_CTL_DIR, NULL); 
	if (!slic_pwrctrl_proc_dir)
		return -EIO;
	entry = proc_create(PROC_SLIC_POWER_STATUS, S_IRUGO|S_IWUSR, slic_pwrctrl_proc_dir, &pwrctl_proc_fops);
	if (!entry) {
		remove_proc_entry(PROC_SLIC_POWER_CTL_DIR, NULL);
		return -ENOMEM;
	}
#endif

    slic_pwrctrl_class = class_create(THIS_MODULE, "slic_pwrctrl_class");
	if (IS_ERR(slic_pwrctrl_class)) {
		dev_err(dev,"%s: couldn't create class\n", __func__);
		ret = PTR_ERR(slic_pwrctrl_class);
		goto err_class;
	}

	ret = alloc_chrdev_region(&slic_pwrctrl_devid,0,SLICPWR_MAX_MINOR,SLICPWR_DEVNAME);
	if (ret < 0) 
		goto err_dev;
	dev_info(dev,"MAJOR Number is %d,MINOR Number is %d\n",MAJOR(slic_pwrctrl_devid),MINOR(slic_pwrctrl_devid));

	cdev_init(&slic_pwrctrl_cdev,&slic_pwrctrl_fops);

	ret = cdev_add(&slic_pwrctrl_cdev,slic_pwrctrl_devid,SLICPWR_MAX_MINOR);
	if (ret) {
		dev_err(dev, "cdev_add err %d.\n", ret);
		goto err_add;
	}

	slic_pwrctrl_dev = device_create(slic_pwrctrl_class, NULL, slic_pwrctrl_devid, NULL, DEV_SLIC_PWR_FILE);
	if (IS_ERR(slic_pwrctrl_dev)) {
		dev_err(dev,"failed to create device\n");
		ret = PTR_ERR(slic_pwrctrl_dev);
		goto err_create;
	}	

	return 0;

err_create:
	cdev_del(&slic_pwrctrl_cdev);
err_add:
	unregister_chrdev_region(slic_pwrctrl_devid, SLICPWR_MAX_MINOR);
err_dev:
	class_destroy(slic_pwrctrl_class);
err_class:
	return ret;
}

static const struct of_device_id bcm_slic_pwrctl_match[] = 
{
	{.compatible = "brcm,voice-slic-power" },
	{ }
	
};
MODULE_DEVICE_TABLE(of, bcm_slic_pwrctl_match);

static struct platform_driver slic_pwrctl_driver = {
	.probe = slic_pwrctrl_probe,
	.remove = slic_pwrctrl_remove,
	.driver = {
		.name = "slic-power-ctrl",
		.of_match_table = of_match_ptr(bcm_slic_pwrctl_match),
	},
};

module_platform_driver(slic_pwrctl_driver);

MODULE_AUTHOR("Kevin Li <kevin-ke.li@broadcom.com>");
MODULE_DESCRIPTION("BCM slic power control driver");
MODULE_LICENSE("GPL v2");
MODULE_ALIAS("platform:" DRV_NAME);
