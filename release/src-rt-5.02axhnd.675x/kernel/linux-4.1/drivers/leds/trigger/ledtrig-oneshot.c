/*
 * One-shot LED Trigger
 *
 * Copyright 2012, Fabio Baltieri <fabio.baltieri@gmail.com>
 *
 * Based on ledtrig-timer.c by Richard Purdie <rpurdie@openedhand.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/ctype.h>
#include <linux/slab.h>
#include <linux/leds.h>
#include "../leds.h"

#define DEFAULT_DELAY 100

struct oneshot_trig_data {
	unsigned int invert;
};

static ssize_t led_shot(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	struct led_classdev *led_cdev = dev_get_drvdata(dev);
	struct oneshot_trig_data *oneshot_data = led_cdev->trigger_data;

	led_blink_set_oneshot(led_cdev,
			&led_cdev->blink_delay_on, &led_cdev->blink_delay_off,
			oneshot_data->invert);

	/* content is ignored */
	return size;
}
static ssize_t led_invert_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct led_classdev *led_cdev = dev_get_drvdata(dev);
	struct oneshot_trig_data *oneshot_data = led_cdev->trigger_data;

	return sprintf(buf, "%u\n", oneshot_data->invert);
}

static ssize_t led_invert_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	struct led_classdev *led_cdev = dev_get_drvdata(dev);
	struct oneshot_trig_data *oneshot_data = led_cdev->trigger_data;
	unsigned long state;
	int ret;

	ret = kstrtoul(buf, 0, &state);
	if (ret)
		return ret;

	oneshot_data->invert = !!state;

	if (oneshot_data->invert)
		led_set_brightness_async(led_cdev, LED_FULL);
	else
		led_set_brightness_async(led_cdev, LED_OFF);

	return size;
}

static ssize_t led_delay_on_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct led_classdev *led_cdev = dev_get_drvdata(dev);

	return sprintf(buf, "%lu\n", led_cdev->blink_delay_on);
}

static ssize_t led_delay_on_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	struct led_classdev *led_cdev = dev_get_drvdata(dev);
	unsigned long state;
	int ret;

	ret = kstrtoul(buf, 0, &state);
	if (ret)
		return ret;

	led_cdev->blink_delay_on = state;

	return size;
}
static ssize_t led_delay_off_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct led_classdev *led_cdev = dev_get_drvdata(dev);

	return sprintf(buf, "%lu\n", led_cdev->blink_delay_off);
}

static ssize_t led_delay_off_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	struct led_classdev *led_cdev = dev_get_drvdata(dev);
	unsigned long state;
	int ret;

	ret = kstrtoul(buf, 0, &state);
	if (ret)
		return ret;

	led_cdev->blink_delay_off = state;

	return size;
}

static DEVICE_ATTR(delay_on, 0644, led_delay_on_show, led_delay_on_store);
static DEVICE_ATTR(delay_off, 0644, led_delay_off_show, led_delay_off_store);
static DEVICE_ATTR(invert, 0644, led_invert_show, led_invert_store);
static DEVICE_ATTR(shot, 0200, NULL, led_shot);

static void oneshot_trig_activate(struct led_classdev *led_cdev)
{
	struct oneshot_trig_data *oneshot_data;
	int rc;

	oneshot_data = kzalloc(sizeof(*oneshot_data), GFP_KERNEL);
	if (!oneshot_data)
		return;

	led_cdev->trigger_data = oneshot_data;

	rc = device_create_file(led_cdev->dev, &dev_attr_delay_on);
	if (rc)
		goto err_out_trig_data;
	rc = device_create_file(led_cdev->dev, &dev_attr_delay_off);
	if (rc)
		goto err_out_delayon;
	rc = device_create_file(led_cdev->dev, &dev_attr_invert);
	if (rc)
		goto err_out_delayoff;
	rc = device_create_file(led_cdev->dev, &dev_attr_shot);
	if (rc)
		goto err_out_invert;

	led_cdev->blink_delay_on = DEFAULT_DELAY;
	led_cdev->blink_delay_off = DEFAULT_DELAY;

	led_cdev->activated = true;

	return;

err_out_invert:
	device_remove_file(led_cdev->dev, &dev_attr_invert);
err_out_delayoff:
	device_remove_file(led_cdev->dev, &dev_attr_delay_off);
err_out_delayon:
	device_remove_file(led_cdev->dev, &dev_attr_delay_on);
err_out_trig_data:
	kfree(led_cdev->trigger_data);
}

static void oneshot_trig_deactivate(struct led_classdev *led_cdev)
{
	struct oneshot_trig_data *oneshot_data = led_cdev->trigger_data;

	if (led_cdev->activated) {
		device_remove_file(led_cdev->dev, &dev_attr_delay_on);
		device_remove_file(led_cdev->dev, &dev_attr_delay_off);
		device_remove_file(led_cdev->dev, &dev_attr_invert);
		device_remove_file(led_cdev->dev, &dev_attr_shot);
		kfree(oneshot_data);
		led_cdev->activated = false;
	}

	/* Stop blinking */
	led_set_brightness(led_cdev, LED_OFF);
}

static struct led_trigger oneshot_led_trigger = {
	.name     = "oneshot",
	.activate = oneshot_trig_activate,
	.deactivate = oneshot_trig_deactivate,
};

static int __init oneshot_trig_init(void)
{
	return led_trigger_register(&oneshot_led_trigger);
}

static void __exit oneshot_trig_exit(void)
{
	led_trigger_unregister(&oneshot_led_trigger);
}

module_init(oneshot_trig_init);
module_exit(oneshot_trig_exit);

MODULE_AUTHOR("Fabio Baltieri <fabio.baltieri@gmail.com>");
MODULE_DESCRIPTION("One-shot LED trigger");
MODULE_LICENSE("GPL");
