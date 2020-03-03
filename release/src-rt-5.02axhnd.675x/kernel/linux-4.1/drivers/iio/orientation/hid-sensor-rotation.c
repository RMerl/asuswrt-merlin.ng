/*
 * HID Sensors Driver
 * Copyright (c) 2014, Intel Corporation.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 */

#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/slab.h>
#include <linux/hid-sensor-hub.h>
#include <linux/iio/iio.h>
#include <linux/iio/sysfs.h>
#include <linux/iio/buffer.h>
#include <linux/iio/trigger_consumer.h>
#include <linux/iio/triggered_buffer.h>
#include "../common/hid-sensors/hid-sensor-trigger.h"

struct dev_rot_state {
	struct hid_sensor_hub_callbacks callbacks;
	struct hid_sensor_common common_attributes;
	struct hid_sensor_hub_attribute_info quaternion;
	u32 sampled_vals[4];
};

/* Channel definitions */
static const struct iio_chan_spec dev_rot_channels[] = {
	{
		.type = IIO_ROT,
		.modified = 1,
		.channel2 = IIO_MOD_QUATERNION,
		.info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
		.info_mask_shared_by_type = BIT(IIO_CHAN_INFO_SAMP_FREQ) |
					BIT(IIO_CHAN_INFO_HYSTERESIS)
	}
};

/* Adjust channel real bits based on report descriptor */
static void dev_rot_adjust_channel_bit_mask(struct iio_chan_spec *chan,
						int size)
{
	chan->scan_type.sign = 's';
	/* Real storage bits will change based on the report desc. */
	chan->scan_type.realbits = size * 8;
	/* Maximum size of a sample to capture is u32 */
	chan->scan_type.storagebits = sizeof(u32) * 8;
	chan->scan_type.repeat = 4;
}

/* Channel read_raw handler */
static int dev_rot_read_raw(struct iio_dev *indio_dev,
				struct iio_chan_spec const *chan,
				int size, int *vals, int *val_len,
				long mask)
{
	struct dev_rot_state *rot_state = iio_priv(indio_dev);
	int ret_type;
	int i;

	vals[0] = 0;
	vals[1] = 0;

	switch (mask) {
	case IIO_CHAN_INFO_RAW:
		if (size >= 4) {
			for (i = 0; i < 4; ++i)
				vals[i] = rot_state->sampled_vals[i];
			ret_type = IIO_VAL_INT_MULTIPLE;
			*val_len =  4;
		} else
			ret_type = -EINVAL;
		break;
	case IIO_CHAN_INFO_SAMP_FREQ:
		ret_type = hid_sensor_read_samp_freq_value(
			&rot_state->common_attributes, &vals[0], &vals[1]);
		break;
	case IIO_CHAN_INFO_HYSTERESIS:
		ret_type = hid_sensor_read_raw_hyst_value(
			&rot_state->common_attributes, &vals[0], &vals[1]);
		break;
	default:
		ret_type = -EINVAL;
		break;
	}

	return ret_type;
}

/* Channel write_raw handler */
static int dev_rot_write_raw(struct iio_dev *indio_dev,
			       struct iio_chan_spec const *chan,
			       int val,
			       int val2,
			       long mask)
{
	struct dev_rot_state *rot_state = iio_priv(indio_dev);
	int ret;

	switch (mask) {
	case IIO_CHAN_INFO_SAMP_FREQ:
		ret = hid_sensor_write_samp_freq_value(
				&rot_state->common_attributes, val, val2);
		break;
	case IIO_CHAN_INFO_HYSTERESIS:
		ret = hid_sensor_write_raw_hyst_value(
				&rot_state->common_attributes, val, val2);
		break;
	default:
		ret = -EINVAL;
	}

	return ret;
}

static const struct iio_info dev_rot_info = {
	.driver_module = THIS_MODULE,
	.read_raw_multi = &dev_rot_read_raw,
	.write_raw = &dev_rot_write_raw,
};

/* Function to push data to buffer */
static void hid_sensor_push_data(struct iio_dev *indio_dev, u8 *data, int len)
{
	dev_dbg(&indio_dev->dev, "hid_sensor_push_data >>\n");
	iio_push_to_buffers(indio_dev, (u8 *)data);
	dev_dbg(&indio_dev->dev, "hid_sensor_push_data <<\n");

}

/* Callback handler to send event after all samples are received and captured */
static int dev_rot_proc_event(struct hid_sensor_hub_device *hsdev,
				unsigned usage_id,
				void *priv)
{
	struct iio_dev *indio_dev = platform_get_drvdata(priv);
	struct dev_rot_state *rot_state = iio_priv(indio_dev);

	dev_dbg(&indio_dev->dev, "dev_rot_proc_event\n");
	if (atomic_read(&rot_state->common_attributes.data_ready))
		hid_sensor_push_data(indio_dev,
				(u8 *)rot_state->sampled_vals,
				sizeof(rot_state->sampled_vals));

	return 0;
}

/* Capture samples in local storage */
static int dev_rot_capture_sample(struct hid_sensor_hub_device *hsdev,
				unsigned usage_id,
				size_t raw_len, char *raw_data,
				void *priv)
{
	struct iio_dev *indio_dev = platform_get_drvdata(priv);
	struct dev_rot_state *rot_state = iio_priv(indio_dev);

	if (usage_id == HID_USAGE_SENSOR_ORIENT_QUATERNION) {
		memcpy(rot_state->sampled_vals, raw_data,
					sizeof(rot_state->sampled_vals));
		dev_dbg(&indio_dev->dev, "Recd Quat len:%zu::%zu\n", raw_len,
					sizeof(rot_state->sampled_vals));
	}

	return 0;
}

/* Parse report which is specific to an usage id*/
static int dev_rot_parse_report(struct platform_device *pdev,
				struct hid_sensor_hub_device *hsdev,
				struct iio_chan_spec *channels,
				unsigned usage_id,
				struct dev_rot_state *st)
{
	int ret;

	ret = sensor_hub_input_get_attribute_info(hsdev,
				HID_INPUT_REPORT,
				usage_id,
				HID_USAGE_SENSOR_ORIENT_QUATERNION,
				&st->quaternion);
	if (ret)
		return ret;

	dev_rot_adjust_channel_bit_mask(&channels[0],
		st->quaternion.size / 4);

	dev_dbg(&pdev->dev, "dev_rot %x:%x\n", st->quaternion.index,
		st->quaternion.report_id);

	dev_dbg(&pdev->dev, "dev_rot: attrib size %d\n",
				st->quaternion.size);

	/* Set Sensitivity field ids, when there is no individual modifier */
	if (st->common_attributes.sensitivity.index < 0) {
		sensor_hub_input_get_attribute_info(hsdev,
			HID_FEATURE_REPORT, usage_id,
			HID_USAGE_SENSOR_DATA_MOD_CHANGE_SENSITIVITY_ABS |
			HID_USAGE_SENSOR_DATA_ORIENTATION,
			&st->common_attributes.sensitivity);
		dev_dbg(&pdev->dev, "Sensitivity index:report %d:%d\n",
			st->common_attributes.sensitivity.index,
			st->common_attributes.sensitivity.report_id);
	}

	return 0;
}

/* Function to initialize the processing for usage id */
static int hid_dev_rot_probe(struct platform_device *pdev)
{
	int ret;
	static char *name = "dev_rotation";
	struct iio_dev *indio_dev;
	struct dev_rot_state *rot_state;
	struct hid_sensor_hub_device *hsdev = pdev->dev.platform_data;
	struct iio_chan_spec *channels;

	indio_dev = devm_iio_device_alloc(&pdev->dev,
					  sizeof(struct dev_rot_state));
	if (indio_dev == NULL)
		return -ENOMEM;

	platform_set_drvdata(pdev, indio_dev);

	rot_state = iio_priv(indio_dev);
	rot_state->common_attributes.hsdev = hsdev;
	rot_state->common_attributes.pdev = pdev;

	ret = hid_sensor_parse_common_attributes(hsdev,
				HID_USAGE_SENSOR_DEVICE_ORIENTATION,
				&rot_state->common_attributes);
	if (ret) {
		dev_err(&pdev->dev, "failed to setup common attributes\n");
		return ret;
	}

	channels = devm_kmemdup(&pdev->dev, dev_rot_channels,
					sizeof(dev_rot_channels), GFP_KERNEL);
	if (!channels) {
		dev_err(&pdev->dev, "failed to duplicate channels\n");
		return -ENOMEM;
	}

	ret = dev_rot_parse_report(pdev, hsdev, channels,
			HID_USAGE_SENSOR_DEVICE_ORIENTATION, rot_state);
	if (ret) {
		dev_err(&pdev->dev, "failed to setup attributes\n");
		return ret;
	}

	indio_dev->channels = channels;
	indio_dev->num_channels = ARRAY_SIZE(dev_rot_channels);
	indio_dev->dev.parent = &pdev->dev;
	indio_dev->info = &dev_rot_info;
	indio_dev->name = name;
	indio_dev->modes = INDIO_DIRECT_MODE;

	ret = iio_triggered_buffer_setup(indio_dev, &iio_pollfunc_store_time,
		NULL, NULL);
	if (ret) {
		dev_err(&pdev->dev, "failed to initialize trigger buffer\n");
		return ret;
	}
	atomic_set(&rot_state->common_attributes.data_ready, 0);
	ret = hid_sensor_setup_trigger(indio_dev, name,
					&rot_state->common_attributes);
	if (ret) {
		dev_err(&pdev->dev, "trigger setup failed\n");
		goto error_unreg_buffer_funcs;
	}

	ret = iio_device_register(indio_dev);
	if (ret) {
		dev_err(&pdev->dev, "device register failed\n");
		goto error_remove_trigger;
	}

	rot_state->callbacks.send_event = dev_rot_proc_event;
	rot_state->callbacks.capture_sample = dev_rot_capture_sample;
	rot_state->callbacks.pdev = pdev;
	ret = sensor_hub_register_callback(hsdev,
					HID_USAGE_SENSOR_DEVICE_ORIENTATION,
					&rot_state->callbacks);
	if (ret) {
		dev_err(&pdev->dev, "callback reg failed\n");
		goto error_iio_unreg;
	}

	return 0;

error_iio_unreg:
	iio_device_unregister(indio_dev);
error_remove_trigger:
	hid_sensor_remove_trigger(&rot_state->common_attributes);
error_unreg_buffer_funcs:
	iio_triggered_buffer_cleanup(indio_dev);
	return ret;
}

/* Function to deinitialize the processing for usage id */
static int hid_dev_rot_remove(struct platform_device *pdev)
{
	struct hid_sensor_hub_device *hsdev = pdev->dev.platform_data;
	struct iio_dev *indio_dev = platform_get_drvdata(pdev);
	struct dev_rot_state *rot_state = iio_priv(indio_dev);

	sensor_hub_remove_callback(hsdev, HID_USAGE_SENSOR_DEVICE_ORIENTATION);
	iio_device_unregister(indio_dev);
	hid_sensor_remove_trigger(&rot_state->common_attributes);
	iio_triggered_buffer_cleanup(indio_dev);

	return 0;
}

static struct platform_device_id hid_dev_rot_ids[] = {
	{
		/* Format: HID-SENSOR-usage_id_in_hex_lowercase */
		.name = "HID-SENSOR-20008a",
	},
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(platform, hid_dev_rot_ids);

static struct platform_driver hid_dev_rot_platform_driver = {
	.id_table = hid_dev_rot_ids,
	.driver = {
		.name	= KBUILD_MODNAME,
	},
	.probe		= hid_dev_rot_probe,
	.remove		= hid_dev_rot_remove,
};
module_platform_driver(hid_dev_rot_platform_driver);

MODULE_DESCRIPTION("HID Sensor Device Rotation");
MODULE_AUTHOR("Srinivas Pandruvada <srinivas.pandruvada@linux.intel.com>");
MODULE_LICENSE("GPL");
