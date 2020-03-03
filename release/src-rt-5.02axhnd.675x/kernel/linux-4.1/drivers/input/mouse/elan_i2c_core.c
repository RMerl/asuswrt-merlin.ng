/*
 * Elan I2C/SMBus Touchpad driver
 *
 * Copyright (c) 2013 ELAN Microelectronics Corp.
 *
 * Author: 林政維 (Duson Lin) <dusonlin@emc.com.tw>
 * Version: 1.5.7
 *
 * Based on cyapa driver:
 * copyright (c) 2011-2012 Cypress Semiconductor, Inc.
 * copyright (c) 2011-2012 Google, Inc.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation.
 *
 * Trademarks are the property of their respective owners.
 */

#include <linux/acpi.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/firmware.h>
#include <linux/i2c.h>
#include <linux/init.h>
#include <linux/input/mt.h>
#include <linux/interrupt.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/input.h>
#include <linux/uaccess.h>
#include <linux/jiffies.h>
#include <linux/completion.h>
#include <linux/of.h>
#include <linux/regulator/consumer.h>
#include <asm/unaligned.h>

#include "elan_i2c.h"

#define DRIVER_NAME		"elan_i2c"
#define ELAN_DRIVER_VERSION	"1.5.7"
#define ETP_MAX_PRESSURE	255
#define ETP_FWIDTH_REDUCE	90
#define ETP_FINGER_WIDTH	15
#define ETP_RETRY_COUNT		3

#define ETP_MAX_FINGERS		5
#define ETP_FINGER_DATA_LEN	5
#define ETP_REPORT_ID		0x5D
#define ETP_REPORT_ID_OFFSET	2
#define ETP_TOUCH_INFO_OFFSET	3
#define ETP_FINGER_DATA_OFFSET	4
#define ETP_HOVER_INFO_OFFSET	30
#define ETP_MAX_REPORT_LEN	34

/* The main device structure */
struct elan_tp_data {
	struct i2c_client	*client;
	struct input_dev	*input;
	struct regulator	*vcc;

	const struct elan_transport_ops *ops;

	/* for fw update */
	struct completion	fw_completion;
	bool			in_fw_update;

	struct mutex		sysfs_mutex;

	unsigned int		max_x;
	unsigned int		max_y;
	unsigned int		width_x;
	unsigned int		width_y;
	unsigned int		x_res;
	unsigned int		y_res;

	u8			product_id;
	u8			fw_version;
	u8			sm_version;
	u8			iap_version;
	u16			fw_checksum;
	int			pressure_adjustment;
	u8			mode;

	bool			irq_wake;

	u8			min_baseline;
	u8			max_baseline;
	bool			baseline_ready;
};

static int elan_enable_power(struct elan_tp_data *data)
{
	int repeat = ETP_RETRY_COUNT;
	int error;

	error = regulator_enable(data->vcc);
	if (error) {
		dev_err(&data->client->dev,
			"failed to enable regulator: %d\n", error);
		return error;
	}

	do {
		error = data->ops->power_control(data->client, true);
		if (error >= 0)
			return 0;

		msleep(30);
	} while (--repeat > 0);

	dev_err(&data->client->dev, "failed to enable power: %d\n", error);
	return error;
}

static int elan_disable_power(struct elan_tp_data *data)
{
	int repeat = ETP_RETRY_COUNT;
	int error;

	do {
		error = data->ops->power_control(data->client, false);
		if (!error) {
			error = regulator_disable(data->vcc);
			if (error) {
				dev_err(&data->client->dev,
					"failed to disable regulator: %d\n",
					error);
				/* Attempt to power the chip back up */
				data->ops->power_control(data->client, true);
				break;
			}

			return 0;
		}

		msleep(30);
	} while (--repeat > 0);

	dev_err(&data->client->dev, "failed to disable power: %d\n", error);
	return error;
}

static int elan_sleep(struct elan_tp_data *data)
{
	int repeat = ETP_RETRY_COUNT;
	int error;

	do {
		error = data->ops->sleep_control(data->client, true);
		if (!error)
			return 0;

		msleep(30);
	} while (--repeat > 0);

	return error;
}

static int __elan_initialize(struct elan_tp_data *data)
{
	struct i2c_client *client = data->client;
	int error;

	error = data->ops->initialize(client);
	if (error) {
		dev_err(&client->dev, "device initialize failed: %d\n", error);
		return error;
	}

	data->mode |= ETP_ENABLE_ABS;
	error = data->ops->set_mode(client, data->mode);
	if (error) {
		dev_err(&client->dev,
			"failed to switch to absolute mode: %d\n", error);
		return error;
	}

	error = data->ops->sleep_control(client, false);
	if (error) {
		dev_err(&client->dev,
			"failed to wake device up: %d\n", error);
		return error;
	}

	return 0;
}

static int elan_initialize(struct elan_tp_data *data)
{
	int repeat = ETP_RETRY_COUNT;
	int error;

	do {
		error = __elan_initialize(data);
		if (!error)
			return 0;

		msleep(30);
	} while (--repeat > 0);

	return error;
}

static int elan_query_device_info(struct elan_tp_data *data)
{
	int error;

	error = data->ops->get_product_id(data->client, &data->product_id);
	if (error)
		return error;

	error = data->ops->get_version(data->client, false, &data->fw_version);
	if (error)
		return error;

	error = data->ops->get_checksum(data->client, false,
					&data->fw_checksum);
	if (error)
		return error;

	error = data->ops->get_sm_version(data->client, &data->sm_version);
	if (error)
		return error;

	error = data->ops->get_version(data->client, true, &data->iap_version);
	if (error)
		return error;

	error = data->ops->get_pressure_adjustment(data->client,
						   &data->pressure_adjustment);
	if (error)
		return error;

	return 0;
}

static unsigned int elan_convert_resolution(u8 val)
{
	/*
	 * (value from firmware) * 10 + 790 = dpi
	 *
	 * We also have to convert dpi to dots/mm (*10/254 to avoid floating
	 * point).
	 */

	return ((int)(char)val * 10 + 790) * 10 / 254;
}

static int elan_query_device_parameters(struct elan_tp_data *data)
{
	unsigned int x_traces, y_traces;
	u8 hw_x_res, hw_y_res;
	int error;

	error = data->ops->get_max(data->client, &data->max_x, &data->max_y);
	if (error)
		return error;

	error = data->ops->get_num_traces(data->client, &x_traces, &y_traces);
	if (error)
		return error;

	data->width_x = data->max_x / x_traces;
	data->width_y = data->max_y / y_traces;

	error = data->ops->get_resolution(data->client, &hw_x_res, &hw_y_res);
	if (error)
		return error;

	data->x_res = elan_convert_resolution(hw_x_res);
	data->y_res = elan_convert_resolution(hw_y_res);

	return 0;
}

/*
 **********************************************************
 * IAP firmware updater related routines
 **********************************************************
 */
static int elan_write_fw_block(struct elan_tp_data *data,
			       const u8 *page, u16 checksum, int idx)
{
	int retry = ETP_RETRY_COUNT;
	int error;

	do {
		error = data->ops->write_fw_block(data->client,
						  page, checksum, idx);
		if (!error)
			return 0;

		dev_dbg(&data->client->dev,
			"IAP retrying page %d (error: %d)\n", idx, error);
	} while (--retry > 0);

	return error;
}

static int __elan_update_firmware(struct elan_tp_data *data,
				  const struct firmware *fw)
{
	struct i2c_client *client = data->client;
	struct device *dev = &client->dev;
	int i, j;
	int error;
	u16 iap_start_addr;
	u16 boot_page_count;
	u16 sw_checksum = 0, fw_checksum = 0;

	error = data->ops->prepare_fw_update(client);
	if (error)
		return error;

	iap_start_addr = get_unaligned_le16(&fw->data[ETP_IAP_START_ADDR * 2]);

	boot_page_count = (iap_start_addr * 2) / ETP_FW_PAGE_SIZE;
	for (i = boot_page_count; i < ETP_FW_VAILDPAGE_COUNT; i++) {
		u16 checksum = 0;
		const u8 *page = &fw->data[i * ETP_FW_PAGE_SIZE];

		for (j = 0; j < ETP_FW_PAGE_SIZE; j += 2)
			checksum += ((page[j + 1] << 8) | page[j]);

		error = elan_write_fw_block(data, page, checksum, i);
		if (error) {
			dev_err(dev, "write page %d fail: %d\n", i, error);
			return error;
		}

		sw_checksum += checksum;
	}

	/* Wait WDT reset and power on reset */
	msleep(600);

	error = data->ops->finish_fw_update(client, &data->fw_completion);
	if (error)
		return error;

	error = data->ops->get_checksum(client, true, &fw_checksum);
	if (error)
		return error;

	if (sw_checksum != fw_checksum) {
		dev_err(dev, "checksum diff sw=[%04X], fw=[%04X]\n",
			sw_checksum, fw_checksum);
		return -EIO;
	}

	return 0;
}

static int elan_update_firmware(struct elan_tp_data *data,
				const struct firmware *fw)
{
	struct i2c_client *client = data->client;
	int retval;

	dev_dbg(&client->dev, "Starting firmware update....\n");

	disable_irq(client->irq);
	data->in_fw_update = true;

	retval = __elan_update_firmware(data, fw);
	if (retval) {
		dev_err(&client->dev, "firmware update failed: %d\n", retval);
		data->ops->iap_reset(client);
	} else {
		/* Reinitialize TP after fw is updated */
		elan_initialize(data);
		elan_query_device_info(data);
	}

	data->in_fw_update = false;
	enable_irq(client->irq);

	return retval;
}

/*
 *******************************************************************
 * SYSFS attributes
 *******************************************************************
 */
static ssize_t elan_sysfs_read_fw_checksum(struct device *dev,
					   struct device_attribute *attr,
					   char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct elan_tp_data *data = i2c_get_clientdata(client);

	return sprintf(buf, "0x%04x\n", data->fw_checksum);
}

static ssize_t elan_sysfs_read_product_id(struct device *dev,
					 struct device_attribute *attr,
					 char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct elan_tp_data *data = i2c_get_clientdata(client);

	return sprintf(buf, "%d.0\n", data->product_id);
}

static ssize_t elan_sysfs_read_fw_ver(struct device *dev,
				      struct device_attribute *attr,
				      char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct elan_tp_data *data = i2c_get_clientdata(client);

	return sprintf(buf, "%d.0\n", data->fw_version);
}

static ssize_t elan_sysfs_read_sm_ver(struct device *dev,
				      struct device_attribute *attr,
				      char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct elan_tp_data *data = i2c_get_clientdata(client);

	return sprintf(buf, "%d.0\n", data->sm_version);
}

static ssize_t elan_sysfs_read_iap_ver(struct device *dev,
				       struct device_attribute *attr,
				       char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct elan_tp_data *data = i2c_get_clientdata(client);

	return sprintf(buf, "%d.0\n", data->iap_version);
}

static ssize_t elan_sysfs_update_fw(struct device *dev,
				    struct device_attribute *attr,
				    const char *buf, size_t count)
{
	struct elan_tp_data *data = dev_get_drvdata(dev);
	const struct firmware *fw;
	int error;
	const u8 *fw_signature;
	static const u8 signature[] = {0xAA, 0x55, 0xCC, 0x33, 0xFF, 0xFF};

	error = request_firmware(&fw, ETP_FW_NAME, dev);
	if (error) {
		dev_err(dev, "cannot load firmware %s: %d\n",
			ETP_FW_NAME, error);
		return error;
	}

	/* Firmware file must match signature data */
	fw_signature = &fw->data[ETP_FW_SIGNATURE_ADDRESS];
	if (memcmp(fw_signature, signature, sizeof(signature)) != 0) {
		dev_err(dev, "signature mismatch (expected %*ph, got %*ph)\n",
			(int)sizeof(signature), signature,
			(int)sizeof(signature), fw_signature);
		error = -EBADF;
		goto out_release_fw;
	}

	error = mutex_lock_interruptible(&data->sysfs_mutex);
	if (error)
		goto out_release_fw;

	error = elan_update_firmware(data, fw);

	mutex_unlock(&data->sysfs_mutex);

out_release_fw:
	release_firmware(fw);
	return error ?: count;
}

static ssize_t calibrate_store(struct device *dev,
			       struct device_attribute *attr,
			       const char *buf, size_t count)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct elan_tp_data *data = i2c_get_clientdata(client);
	int tries = 20;
	int retval;
	int error;
	u8 val[3];

	retval = mutex_lock_interruptible(&data->sysfs_mutex);
	if (retval)
		return retval;

	disable_irq(client->irq);

	data->mode |= ETP_ENABLE_CALIBRATE;
	retval = data->ops->set_mode(client, data->mode);
	if (retval) {
		dev_err(dev, "failed to enable calibration mode: %d\n",
			retval);
		goto out;
	}

	retval = data->ops->calibrate(client);
	if (retval) {
		dev_err(dev, "failed to start calibration: %d\n",
			retval);
		goto out_disable_calibrate;
	}

	val[0] = 0xff;
	do {
		/* Wait 250ms before checking if calibration has completed. */
		msleep(250);

		retval = data->ops->calibrate_result(client, val);
		if (retval)
			dev_err(dev, "failed to check calibration result: %d\n",
				retval);
		else if (val[0] == 0)
			break; /* calibration done */

	} while (--tries);

	if (tries == 0) {
		dev_err(dev, "failed to calibrate. Timeout.\n");
		retval = -ETIMEDOUT;
	}

out_disable_calibrate:
	data->mode &= ~ETP_ENABLE_CALIBRATE;
	error = data->ops->set_mode(data->client, data->mode);
	if (error) {
		dev_err(dev, "failed to disable calibration mode: %d\n",
			error);
		if (!retval)
			retval = error;
	}
out:
	enable_irq(client->irq);
	mutex_unlock(&data->sysfs_mutex);
	return retval ?: count;
}

static ssize_t elan_sysfs_read_mode(struct device *dev,
				    struct device_attribute *attr,
				    char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct elan_tp_data *data = i2c_get_clientdata(client);
	int error;
	enum tp_mode mode;

	error = mutex_lock_interruptible(&data->sysfs_mutex);
	if (error)
		return error;

	error = data->ops->iap_get_mode(data->client, &mode);

	mutex_unlock(&data->sysfs_mutex);

	if (error)
		return error;

	return sprintf(buf, "%d\n", (int)mode);
}

static DEVICE_ATTR(product_id, S_IRUGO, elan_sysfs_read_product_id, NULL);
static DEVICE_ATTR(firmware_version, S_IRUGO, elan_sysfs_read_fw_ver, NULL);
static DEVICE_ATTR(sample_version, S_IRUGO, elan_sysfs_read_sm_ver, NULL);
static DEVICE_ATTR(iap_version, S_IRUGO, elan_sysfs_read_iap_ver, NULL);
static DEVICE_ATTR(fw_checksum, S_IRUGO, elan_sysfs_read_fw_checksum, NULL);
static DEVICE_ATTR(mode, S_IRUGO, elan_sysfs_read_mode, NULL);
static DEVICE_ATTR(update_fw, S_IWUSR, NULL, elan_sysfs_update_fw);

static DEVICE_ATTR_WO(calibrate);

static struct attribute *elan_sysfs_entries[] = {
	&dev_attr_product_id.attr,
	&dev_attr_firmware_version.attr,
	&dev_attr_sample_version.attr,
	&dev_attr_iap_version.attr,
	&dev_attr_fw_checksum.attr,
	&dev_attr_calibrate.attr,
	&dev_attr_mode.attr,
	&dev_attr_update_fw.attr,
	NULL,
};

static const struct attribute_group elan_sysfs_group = {
	.attrs = elan_sysfs_entries,
};

static ssize_t acquire_store(struct device *dev, struct device_attribute *attr,
			     const char *buf, size_t count)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct elan_tp_data *data = i2c_get_clientdata(client);
	int error;
	int retval;

	retval = mutex_lock_interruptible(&data->sysfs_mutex);
	if (retval)
		return retval;

	disable_irq(client->irq);

	data->baseline_ready = false;

	data->mode |= ETP_ENABLE_CALIBRATE;
	retval = data->ops->set_mode(data->client, data->mode);
	if (retval) {
		dev_err(dev, "Failed to enable calibration mode to get baseline: %d\n",
			retval);
		goto out;
	}

	msleep(250);

	retval = data->ops->get_baseline_data(data->client, true,
					      &data->max_baseline);
	if (retval) {
		dev_err(dev, "Failed to read max baseline form device: %d\n",
			retval);
		goto out_disable_calibrate;
	}

	retval = data->ops->get_baseline_data(data->client, false,
					      &data->min_baseline);
	if (retval) {
		dev_err(dev, "Failed to read min baseline form device: %d\n",
			retval);
		goto out_disable_calibrate;
	}

	data->baseline_ready = true;

out_disable_calibrate:
	data->mode &= ~ETP_ENABLE_CALIBRATE;
	error = data->ops->set_mode(data->client, data->mode);
	if (error) {
		dev_err(dev, "Failed to disable calibration mode after acquiring baseline: %d\n",
			error);
		if (!retval)
			retval = error;
	}
out:
	enable_irq(client->irq);
	mutex_unlock(&data->sysfs_mutex);
	return retval ?: count;
}

static ssize_t min_show(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct elan_tp_data *data = i2c_get_clientdata(client);
	int retval;

	retval = mutex_lock_interruptible(&data->sysfs_mutex);
	if (retval)
		return retval;

	if (!data->baseline_ready) {
		retval = -ENODATA;
		goto out;
	}

	retval = snprintf(buf, PAGE_SIZE, "%d", data->min_baseline);

out:
	mutex_unlock(&data->sysfs_mutex);
	return retval;
}

static ssize_t max_show(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct elan_tp_data *data = i2c_get_clientdata(client);
	int retval;

	retval = mutex_lock_interruptible(&data->sysfs_mutex);
	if (retval)
		return retval;

	if (!data->baseline_ready) {
		retval = -ENODATA;
		goto out;
	}

	retval = snprintf(buf, PAGE_SIZE, "%d", data->max_baseline);

out:
	mutex_unlock(&data->sysfs_mutex);
	return retval;
}


static DEVICE_ATTR_WO(acquire);
static DEVICE_ATTR_RO(min);
static DEVICE_ATTR_RO(max);

static struct attribute *elan_baseline_sysfs_entries[] = {
	&dev_attr_acquire.attr,
	&dev_attr_min.attr,
	&dev_attr_max.attr,
	NULL,
};

static const struct attribute_group elan_baseline_sysfs_group = {
	.name = "baseline",
	.attrs = elan_baseline_sysfs_entries,
};

static const struct attribute_group *elan_sysfs_groups[] = {
	&elan_sysfs_group,
	&elan_baseline_sysfs_group,
	NULL
};

/*
 ******************************************************************
 * Elan isr functions
 ******************************************************************
 */
static void elan_report_contact(struct elan_tp_data *data,
				int contact_num, bool contact_valid,
				bool hover_event, u8 *finger_data)
{
	struct input_dev *input = data->input;
	unsigned int pos_x, pos_y;
	unsigned int pressure, mk_x, mk_y;
	unsigned int area_x, area_y, major, minor;
	unsigned int scaled_pressure;

	if (contact_valid) {
		pos_x = ((finger_data[0] & 0xf0) << 4) |
						finger_data[1];
		pos_y = ((finger_data[0] & 0x0f) << 8) |
						finger_data[2];
		mk_x = (finger_data[3] & 0x0f);
		mk_y = (finger_data[3] >> 4);
		pressure = finger_data[4];

		if (pos_x > data->max_x || pos_y > data->max_y) {
			dev_dbg(input->dev.parent,
				"[%d] x=%d y=%d over max (%d, %d)",
				contact_num, pos_x, pos_y,
				data->max_x, data->max_y);
			return;
		}

		/*
		 * To avoid treating large finger as palm, let's reduce the
		 * width x and y per trace.
		 */
		area_x = mk_x * (data->width_x - ETP_FWIDTH_REDUCE);
		area_y = mk_y * (data->width_y - ETP_FWIDTH_REDUCE);

		major = max(area_x, area_y);
		minor = min(area_x, area_y);

		scaled_pressure = pressure + data->pressure_adjustment;

		if (scaled_pressure > ETP_MAX_PRESSURE)
			scaled_pressure = ETP_MAX_PRESSURE;

		input_mt_slot(input, contact_num);
		input_mt_report_slot_state(input, MT_TOOL_FINGER, true);
		input_report_abs(input, ABS_MT_POSITION_X, pos_x);
		input_report_abs(input, ABS_MT_POSITION_Y, data->max_y - pos_y);
		input_report_abs(input, ABS_MT_DISTANCE, hover_event);
		input_report_abs(input, ABS_MT_PRESSURE,
				 hover_event ? 0 : scaled_pressure);
		input_report_abs(input, ABS_TOOL_WIDTH, mk_x);
		input_report_abs(input, ABS_MT_TOUCH_MAJOR, major);
		input_report_abs(input, ABS_MT_TOUCH_MINOR, minor);
	} else {
		input_mt_slot(input, contact_num);
		input_mt_report_slot_state(input, MT_TOOL_FINGER, false);
	}
}

static void elan_report_absolute(struct elan_tp_data *data, u8 *packet)
{
	struct input_dev *input = data->input;
	u8 *finger_data = &packet[ETP_FINGER_DATA_OFFSET];
	int i;
	u8 tp_info = packet[ETP_TOUCH_INFO_OFFSET];
	u8 hover_info = packet[ETP_HOVER_INFO_OFFSET];
	bool contact_valid, hover_event;

	hover_event = hover_info & 0x40;
	for (i = 0; i < ETP_MAX_FINGERS; i++) {
		contact_valid = tp_info & (1U << (3 + i));
		elan_report_contact(data, i, contact_valid, hover_event,
				    finger_data);

		if (contact_valid)
			finger_data += ETP_FINGER_DATA_LEN;
	}

	input_report_key(input, BTN_LEFT, tp_info & 0x01);
	input_mt_report_pointer_emulation(input, true);
	input_sync(input);
}

static irqreturn_t elan_isr(int irq, void *dev_id)
{
	struct elan_tp_data *data = dev_id;
	struct device *dev = &data->client->dev;
	int error;
	u8 report[ETP_MAX_REPORT_LEN];

	/*
	 * When device is connected to i2c bus, when all IAP page writes
	 * complete, the driver will receive interrupt and must read
	 * 0000 to confirm that IAP is finished.
	*/
	if (data->in_fw_update) {
		complete(&data->fw_completion);
		goto out;
	}

	error = data->ops->get_report(data->client, report);
	if (error)
		goto out;

	if (report[ETP_REPORT_ID_OFFSET] != ETP_REPORT_ID)
		dev_err(dev, "invalid report id data (%x)\n",
			report[ETP_REPORT_ID_OFFSET]);
	else
		elan_report_absolute(data, report);

out:
	return IRQ_HANDLED;
}

/*
 ******************************************************************
 * Elan initialization functions
 ******************************************************************
 */
static int elan_setup_input_device(struct elan_tp_data *data)
{
	struct device *dev = &data->client->dev;
	struct input_dev *input;
	unsigned int max_width = max(data->width_x, data->width_y);
	unsigned int min_width = min(data->width_x, data->width_y);
	int error;

	input = devm_input_allocate_device(dev);
	if (!input)
		return -ENOMEM;

	input->name = "Elan Touchpad";
	input->id.bustype = BUS_I2C;
	input_set_drvdata(input, data);

	error = input_mt_init_slots(input, ETP_MAX_FINGERS,
				    INPUT_MT_POINTER | INPUT_MT_DROP_UNUSED);
	if (error) {
		dev_err(dev, "failed to initialize MT slots: %d\n", error);
		return error;
	}

	__set_bit(EV_ABS, input->evbit);
	__set_bit(INPUT_PROP_POINTER, input->propbit);
	__set_bit(INPUT_PROP_BUTTONPAD, input->propbit);
	__set_bit(BTN_LEFT, input->keybit);

	/* Set up ST parameters */
	input_set_abs_params(input, ABS_X, 0, data->max_x, 0, 0);
	input_set_abs_params(input, ABS_Y, 0, data->max_y, 0, 0);
	input_abs_set_res(input, ABS_X, data->x_res);
	input_abs_set_res(input, ABS_Y, data->y_res);
	input_set_abs_params(input, ABS_PRESSURE, 0, ETP_MAX_PRESSURE, 0, 0);
	input_set_abs_params(input, ABS_TOOL_WIDTH, 0, ETP_FINGER_WIDTH, 0, 0);

	/* And MT parameters */
	input_set_abs_params(input, ABS_MT_POSITION_X, 0, data->max_x, 0, 0);
	input_set_abs_params(input, ABS_MT_POSITION_Y, 0, data->max_y, 0, 0);
	input_abs_set_res(input, ABS_MT_POSITION_X, data->x_res);
	input_abs_set_res(input, ABS_MT_POSITION_Y, data->y_res);
	input_set_abs_params(input, ABS_MT_PRESSURE, 0,
			     ETP_MAX_PRESSURE, 0, 0);
	input_set_abs_params(input, ABS_MT_TOUCH_MAJOR, 0,
			     ETP_FINGER_WIDTH * max_width, 0, 0);
	input_set_abs_params(input, ABS_MT_TOUCH_MINOR, 0,
			     ETP_FINGER_WIDTH * min_width, 0, 0);
	input_set_abs_params(input, ABS_MT_DISTANCE, 0, 1, 0, 0);

	data->input = input;

	return 0;
}

static void elan_disable_regulator(void *_data)
{
	struct elan_tp_data *data = _data;

	regulator_disable(data->vcc);
}

static void elan_remove_sysfs_groups(void *_data)
{
	struct elan_tp_data *data = _data;

	sysfs_remove_groups(&data->client->dev.kobj, elan_sysfs_groups);
}

static int elan_probe(struct i2c_client *client,
		      const struct i2c_device_id *dev_id)
{
	const struct elan_transport_ops *transport_ops;
	struct device *dev = &client->dev;
	struct elan_tp_data *data;
	unsigned long irqflags;
	int error;

	if (IS_ENABLED(CONFIG_MOUSE_ELAN_I2C_I2C) &&
	    i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		transport_ops = &elan_i2c_ops;
	} else if (IS_ENABLED(CONFIG_MOUSE_ELAN_I2C_SMBUS) &&
		   i2c_check_functionality(client->adapter,
					   I2C_FUNC_SMBUS_BYTE_DATA |
						I2C_FUNC_SMBUS_BLOCK_DATA |
						I2C_FUNC_SMBUS_I2C_BLOCK)) {
		transport_ops = &elan_smbus_ops;
	} else {
		dev_err(dev, "not a supported I2C/SMBus adapter\n");
		return -EIO;
	}

	data = devm_kzalloc(&client->dev, sizeof(struct elan_tp_data),
			    GFP_KERNEL);
	if (!data)
		return -ENOMEM;

	i2c_set_clientdata(client, data);

	data->ops = transport_ops;
	data->client = client;
	init_completion(&data->fw_completion);
	mutex_init(&data->sysfs_mutex);

	data->vcc = devm_regulator_get(&client->dev, "vcc");
	if (IS_ERR(data->vcc)) {
		error = PTR_ERR(data->vcc);
		if (error != -EPROBE_DEFER)
			dev_err(&client->dev,
				"Failed to get 'vcc' regulator: %d\n",
				error);
		return error;
	}

	error = regulator_enable(data->vcc);
	if (error) {
		dev_err(&client->dev,
			"Failed to enable regulator: %d\n", error);
		return error;
	}

	error = devm_add_action(&client->dev,
				elan_disable_regulator, data);
	if (error) {
		regulator_disable(data->vcc);
		dev_err(&client->dev,
			"Failed to add disable regulator action: %d\n",
			error);
		return error;
	}

	/* Make sure there is something at this address */
	error = i2c_smbus_read_byte(client);
	if (error < 0) {
		dev_dbg(&client->dev, "nothing at this address: %d\n", error);
		return -ENXIO;
	}

	/* Initialize the touchpad. */
	error = elan_initialize(data);
	if (error)
		return error;

	error = elan_query_device_info(data);
	if (error)
		return error;

	error = elan_query_device_parameters(data);
	if (error)
		return error;

	dev_dbg(&client->dev,
		"Elan Touchpad Information:\n"
		"    Module product ID:  0x%04x\n"
		"    Firmware Version:  0x%04x\n"
		"    Sample Version:  0x%04x\n"
		"    IAP Version:  0x%04x\n"
		"    Max ABS X,Y:   %d,%d\n"
		"    Width X,Y:   %d,%d\n"
		"    Resolution X,Y:   %d,%d (dots/mm)\n",
		data->product_id,
		data->fw_version,
		data->sm_version,
		data->iap_version,
		data->max_x, data->max_y,
		data->width_x, data->width_y,
		data->x_res, data->y_res);

	/* Set up input device properties based on queried parameters. */
	error = elan_setup_input_device(data);
	if (error)
		return error;

	/*
	 * Systems using device tree should set up interrupt via DTS,
	 * the rest will use the default falling edge interrupts.
	 */
	irqflags = client->dev.of_node ? 0 : IRQF_TRIGGER_FALLING;

	error = devm_request_threaded_irq(&client->dev, client->irq,
					  NULL, elan_isr,
					  irqflags | IRQF_ONESHOT,
					  client->name, data);
	if (error) {
		dev_err(&client->dev, "cannot register irq=%d\n", client->irq);
		return error;
	}

	error = sysfs_create_groups(&client->dev.kobj, elan_sysfs_groups);
	if (error) {
		dev_err(&client->dev, "failed to create sysfs attributes: %d\n",
			error);
		return error;
	}

	error = devm_add_action(&client->dev,
				elan_remove_sysfs_groups, data);
	if (error) {
		elan_remove_sysfs_groups(data);
		dev_err(&client->dev,
			"Failed to add sysfs cleanup action: %d\n",
			error);
		return error;
	}

	error = input_register_device(data->input);
	if (error) {
		dev_err(&client->dev, "failed to register input device: %d\n",
			error);
		return error;
	}

	/*
	 * Systems using device tree should set up wakeup via DTS,
	 * the rest will configure device as wakeup source by default.
	 */
	if (!client->dev.of_node)
		device_init_wakeup(&client->dev, true);

	return 0;
}

static int __maybe_unused elan_suspend(struct device *dev)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct elan_tp_data *data = i2c_get_clientdata(client);
	int ret;

	/*
	 * We are taking the mutex to make sure sysfs operations are
	 * complete before we attempt to bring the device into low[er]
	 * power mode.
	 */
	ret = mutex_lock_interruptible(&data->sysfs_mutex);
	if (ret)
		return ret;

	disable_irq(client->irq);

	if (device_may_wakeup(dev)) {
		ret = elan_sleep(data);
		/* Enable wake from IRQ */
		data->irq_wake = (enable_irq_wake(client->irq) == 0);
	} else {
		ret = elan_disable_power(data);
	}

	mutex_unlock(&data->sysfs_mutex);
	return ret;
}

static int __maybe_unused elan_resume(struct device *dev)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct elan_tp_data *data = i2c_get_clientdata(client);
	int error;

	if (device_may_wakeup(dev) && data->irq_wake) {
		disable_irq_wake(client->irq);
		data->irq_wake = false;
	}

	error = elan_enable_power(data);
	if (error) {
		dev_err(dev, "power up when resuming failed: %d\n", error);
		goto err;
	}

	error = elan_initialize(data);
	if (error)
		dev_err(dev, "initialize when resuming failed: %d\n", error);

err:
	enable_irq(data->client->irq);
	return error;
}

static SIMPLE_DEV_PM_OPS(elan_pm_ops, elan_suspend, elan_resume);

static const struct i2c_device_id elan_id[] = {
	{ DRIVER_NAME, 0 },
	{ },
};
MODULE_DEVICE_TABLE(i2c, elan_id);

#ifdef CONFIG_ACPI
static const struct acpi_device_id elan_acpi_id[] = {
	{ "ELAN0000", 0 },
	{ }
};
MODULE_DEVICE_TABLE(acpi, elan_acpi_id);
#endif

#ifdef CONFIG_OF
static const struct of_device_id elan_of_match[] = {
	{ .compatible = "elan,ekth3000" },
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, elan_of_match);
#endif

static struct i2c_driver elan_driver = {
	.driver = {
		.name	= DRIVER_NAME,
		.owner	= THIS_MODULE,
		.pm	= &elan_pm_ops,
		.acpi_match_table = ACPI_PTR(elan_acpi_id),
		.of_match_table = of_match_ptr(elan_of_match),
	},
	.probe		= elan_probe,
	.id_table	= elan_id,
};

module_i2c_driver(elan_driver);

MODULE_AUTHOR("Duson Lin <dusonlin@emc.com.tw>");
MODULE_DESCRIPTION("Elan I2C/SMBus Touchpad driver");
MODULE_LICENSE("GPL");
MODULE_VERSION(ELAN_DRIVER_VERSION);
