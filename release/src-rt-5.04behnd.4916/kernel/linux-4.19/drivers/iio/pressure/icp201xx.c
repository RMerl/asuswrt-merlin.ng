/*
 * Copyright (C) 2017-2020 InvenSense, Inc.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/pm.h>
#include <linux/crc8.h>
#include <linux/atomic.h>
#include <linux/spinlock.h>
#include <linux/delay.h>
#include <linux/hrtimer.h>
#include <linux/ktime.h>
#ifdef CONFIG_RTC_INTF_ALARM
#include <linux/android_alarm.h>
#endif
#include <linux/irqreturn.h>
#include <linux/math64.h>
#include <linux/iio/iio.h>
#include <linux/iio/buffer.h>
#include <linux/iio/trigger.h>
#include <linux/iio/trigger_consumer.h>
#include <linux/iio/triggered_buffer.h>
#include <linux/iio/sysfs.h>
#include <linux/version.h>

#include "icp201xx.h"
#include "icp201xx_core.h"
#include "icp201xx_i2c.h"

#define ICP201XX_VERSION "0.0.2_test1"

enum icp201xx_attributes {
        ICP201XX_ATTR_SAMP_AVAIL_FREQ,
        ICP201XX_ATTR_MODE,
        ICP201XX_ATTR_CALIBDATA,
};

enum icp201xx_channel {
        ICP201XX_CHANNEL_PRESSURE,
        ICP201XX_CHANNEL_TEMPERATURE,
        ICP201XX_CHANNEL_TIMESTAMP,
};

static const unsigned icp201xx_min_freq = 1;
static const unsigned icp201xx_max_freq[] = {
        [ICP201XX_MODE_0] = 25,
        [ICP201XX_MODE_1] = 120,
        [ICP201XX_MODE_2] = 40,
        [ICP201XX_MODE_3] = 2,
        [ICP201XX_MODE_4] = 40,
};


static const char *icp201xx_mode_strings[] = {
        [ICP201XX_MODE_0] = "0",
        [ICP201XX_MODE_1] = "1",
        [ICP201XX_MODE_2] = "2",
        [ICP201XX_MODE_3] = "3",
        [ICP201XX_MODE_4] = "4",
};


#ifdef CONFIG_RTC_INTF_ALARM
static inline u64 icp201xx_get_time(void)
{
	ktime_t ts;

	/* Workaround for some platform on which monotonic clock and
	 * Android SystemClock has a gap.
	 * Use ktime_to_timespec(alarm_get_elapsed_realtime()) instead of
	 * get_monotonic_boottime() for these platform
	 */
	ts = ktime_to_timespec(alarm_get_elapsed_realtime());

	return timespec_to_ns(ts);
}
#else
static inline u64 icp201xx_get_time(void)
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 19, 0)
	/* kernel ~4.18 */
	struct timespec ts;
	get_monotonic_boottime(&ts);

	return timespec_to_ns(&ts);
#elif LINUX_VERSION_CODE < KERNEL_VERSION(5, 3, 0)
	/* kernel 4.19~5.2 */
	return ktime_get_boot_ns();
#else
	/* kernel 5.3~ */
	return ktime_get_boottime_ns();
#endif
}
#endif

#define ICP201XX_CRC8_POLYNOMIAL		0x31

DECLARE_CRC8_TABLE(icp201xx_crc8_table);

static int icp201xx_init_chip(struct icp201xx_state *st)
{
	int ret;
	u8 buf;

	do {
		ret = icp201xx_reg_write(st, 0xEE, 0xF0);
		if (!ret)
			break;

		udelay(5);
	}while(1);

	ret = icp201xx_reg_read(st, ICP201XX_REG_VERSION, &buf);
	if (ret)
		return ret;

	pr_info("ICP201XX(version %s) VERSION : %X", ICP201XX_VERSION, buf);

	if (buf == 0xB2) {
		st->version = ICP201XX_VERSION_B;
		return 0;
	}
	else if (buf == 0x00)
		st->version = ICP201XX_VERSION_A;
	else
		return -EINVAL;

	ret = icp201xx_reg_read(st, ICP201XX_REG_DEVICE_ID, &buf);
	if (ret)
		return ret;

	pr_info("ICP201XX(version %s) DEVICE_ID : %X", ICP201XX_VERSION, buf);

	ret = icp201xx_reg_read(st, ICP201XX_REG_OTP_STATUS2, &buf);
	if (ret)
		return ret;

	if (buf == 0x01) {
		pr_info("ICP201XX boot sequence was already done.");
		return 0;
	}

	icp201xx_OTP_bootup_cfg(st);

	return 0;
}

static int icp201xx_process_raw_data(struct icp201xx_state *st, uint8_t packet_cnt, uint8_t *data,int32_t * pressure, int32_t * temperature)
{
	uint8_t i, offset = 0;

	for ( i = 0 ; i < packet_cnt ; i++ ) {
		if ( st->fifo_readout_mode == ICP201XX_FIFO_READOUT_MODE_PRES_TEMP) {
			pressure[i] = (int32_t)((( data[offset+2] & 0x0f) << 16) | (data[offset+1] << 8) | data[offset]) ;
			offset += 3;
			temperature[i] = (int32_t)(((data[offset+2] & 0x0f) << 16) | (data[offset+1] << 8) | data[offset]) ;
			offset += 3;
		} else if ( st->fifo_readout_mode == ICP201XX_FIFO_READOUT_MODE_TEMP_ONLY) {
			temperature[i] = (int32_t)(((data[offset+2] & 0x0f) << 16) | (data[offset+1] << 8) | data[offset]) ;
			offset += 3;
		} else if( st->fifo_readout_mode == ICP201XX_FIFO_READOUT_MODE_TEMP_PRES) {
			temperature[i] = (int32_t)(((data[offset+2] & 0x0f) << 16) | (data[offset+1] << 8) | data[offset]) ;
			offset += 3;
			pressure[i] = (int32_t)((( data[offset+2] & 0x0f) << 16) | (data[offset+1] << 8) | data[offset]) ;
			offset += 3;
		} else if( st->fifo_readout_mode == ICP201XX_FIFO_READOUT_MODE_PRES_ONLY) {
			pressure[i] = (int32_t)((( data[offset+2] & 0x0f) << 16) | (data[offset+1] << 8) | data[offset]) ;
			offset += 3;
		}
	}
	return 0;
}

static void display_press_temp(uint8_t fifo_packets, int32_t *data_temp, int32_t *data_press)
{
	uint8_t i;
	for ( i = 0 ; i < fifo_packets ; i++) {
		if (data_press[i] & 0x080000 )
			data_press[i] |= 0xFFF00000;

		if (data_temp[i] & 0x080000 )
			data_temp[i] |= 0xFFF00000;
	}
}

static irqreturn_t icp201xx_read_measurement(int irq, void *p)
{
	struct iio_poll_func *pf = p;
	struct iio_dev *indio_dev = pf->indio_dev;
	struct icp201xx_state *st = iio_priv(indio_dev);

	struct {
		uint32_t pressure;
		uint32_t temp;
	} __packed data;
	uint64_t ts;
	uint8_t ts_buf[10] = {0,};

	uint8_t fifo_cnt = 0;
	uint8_t fifo_data[96] = {0,};
	static int32_t data_temp[20], data_press[20];

	int ret;

	ts = icp201xx_get_time();
	memcpy(ts_buf, &ts, 8);

	icp201xx_get_fifo_count(st, &fifo_cnt);

	if(fifo_cnt > 0) {
		icp201xx_get_fifo_data(st, fifo_cnt, fifo_data);
		icp201xx_process_raw_data(st, fifo_cnt > 1 ? 2 : fifo_cnt, fifo_data, data_press, data_temp);
		display_press_temp(fifo_cnt, data_temp, data_press);
		data.pressure = data_press[0];
		data.temp = data_temp[0];
	} else {
		if (data_press[1] != 0)
			data.pressure = data_press[1];
		else
			data.pressure = data_press[0];

		if (data_temp[1] != 0)
			data.temp = data_temp[1];
		else
			data.temp = data_temp[0];

	}

	icp201xx_read_dummy_data(st);

	ret = iio_push_to_buffers(indio_dev, (uint8_t *)&data);
	if (ret)
		dev_err(&st->client->dev, "iio push error %d\n", ret);

	ret = iio_push_to_buffers(indio_dev, ts_buf);
	if (ret)
		dev_err(&st->client->dev, "iio push error %d\n", ret);

	iio_trigger_notify_done(indio_dev->trig);
	return IRQ_HANDLED;
}

static enum hrtimer_restart icp201xx_timer_handler(struct hrtimer *timer)
{
	struct icp201xx_state *st = container_of(timer, struct icp201xx_state,
						timer);
	ktime_t period;
	unsigned long flags;

	spin_lock_irqsave(&st->period_lock, flags);
	period = st->period;
	spin_unlock_irqrestore(&st->period_lock, flags);
	hrtimer_forward_now(timer, period);

	iio_trigger_poll(st->trig);

	return HRTIMER_RESTART;
}

static int icp201xx_trig_set_state(struct iio_trigger *trig, bool state)
{
	struct iio_dev *indio_dev = iio_trigger_get_drvdata(trig);
	struct icp201xx_state *st = iio_priv(indio_dev);
	ktime_t period;
	unsigned long flags;

	if (state) {
		inv_run_icp201xx_in_polling(st, atomic_read(&st->mode), st->frequency);
		spin_lock_irqsave(&st->period_lock, flags);
		period = st->period;
		spin_unlock_irqrestore(&st->period_lock, flags);
		hrtimer_start(&st->timer, period, HRTIMER_MODE_REL);
	} else {
		icp201xx_soft_reset(st);
		hrtimer_cancel(&st->timer);
	}

	return 0;
}

static const struct iio_trigger_ops icp201xx_trigger_ops = {
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 15, 0)
	.owner = THIS_MODULE,
#endif
	.set_trigger_state = icp201xx_trig_set_state,
};


static int icp201xx_read_raw(struct iio_dev *indio_dev,
			    struct iio_chan_spec const *chan,
			    int *val, int *val2, long mask)
{
	struct icp201xx_state *st = iio_priv(indio_dev);
	uint8_t raw_buf[6] = {0,};
	int ret;

	switch (mask) {
	case IIO_CHAN_INFO_RAW:
		mutex_lock(&indio_dev->mlock);
		if (iio_buffer_enabled(indio_dev)) {
			ret = -EBUSY;
			goto out_info_raw;
		}
		ret = 0;
		if (ret)
			goto out_info_raw;

		switch (chan->type) {
		case IIO_PRESSURE:
			icp201xx_reg_read_n(st, ICP201XX_REG_PRESS_DATA_0, 3, raw_buf);
			*val = (int)(((raw_buf[2] & 0x0f) << 16) | (raw_buf[1] << 8) | raw_buf[0]);
			if (*val & 0x080000 )
				*val |= 0xFFF00000;

			ret = IIO_VAL_INT;
			break;
		case IIO_TEMP:
			icp201xx_reg_read_n(st, ICP201XX_REG_TEMP_DATA_0, 3, raw_buf);
			*val = (int)(((raw_buf[2] & 0x0f) << 16) | (raw_buf[1] << 8) | raw_buf[0]);
			if (*val & 0x080000 )
				*val |= 0xFFF00000;

			ret = IIO_VAL_INT;
			break;
		default:
			ret = -EINVAL;
			break;
		};
out_info_raw:
		mutex_unlock(&indio_dev->mlock);
		break;
	case IIO_CHAN_INFO_SCALE:
		switch (chan->type) {
		case IIO_PRESSURE:
			*val = 1;
			ret = IIO_VAL_INT;
			break;
		case IIO_TEMP:
			*val = 1;
			ret = IIO_VAL_INT;
			break;
		default:
			ret = -EINVAL;
			break;
		}
		break;
	case IIO_CHAN_INFO_OFFSET:
		switch (chan->type) {
		case IIO_PRESSURE:
			*val = 0;
			ret = IIO_VAL_INT;
			break;
		case IIO_TEMP:
			*val = 0;
			ret = IIO_VAL_INT;
			break;
		default:
			ret = -EINVAL;
			break;
		}
		break;
	default:
		ret = -EINVAL;
		break;
	}

	return ret;
}

static int icp201xx_validate_trigger(struct iio_dev *indio_dev,
				    struct iio_trigger *trig)
{
	struct icp201xx_state *st = iio_priv(indio_dev);

	if (st->trig != trig)
		return -EINVAL;

	return 0;
}

static ssize_t icp201xx_attr_show(struct device *dev,
				 struct device_attribute *attr, char *buf)
{
	struct icp201xx_state *st = iio_priv(dev_to_iio_dev(dev));
	struct iio_dev_attr *this_attr = to_iio_dev_attr(attr);

	switch (this_attr->address) {
	case ICP201XX_ATTR_SAMP_AVAIL_FREQ:
		return scnprintf(buf, PAGE_SIZE, "%u-%u\n", icp201xx_min_freq,
				 icp201xx_max_freq[atomic_read(&st->mode)]);
	case ICP201XX_ATTR_MODE:
		return scnprintf(buf, PAGE_SIZE, "%d\n", atomic_read(&st->mode));
	case ICP201XX_ATTR_CALIBDATA:
		return scnprintf(buf, PAGE_SIZE, "0\n");
	default:
		return -EINVAL;
	}
}

static ssize_t icp201xx_attr_store(struct device *dev,
				  struct device_attribute *attr,
				  const char *buf, size_t count)
{
	struct iio_dev *indio_dev = dev_to_iio_dev(dev);
	struct icp201xx_state *st = iio_priv(indio_dev);
	struct iio_dev_attr *this_attr = to_iio_dev_attr(attr);
	unsigned val;
	int64_t min_period;
	int ret;
	unsigned long flags;

	switch (this_attr->address) {
	case ICP201XX_ATTR_MODE:
		for (val = 0; val < ICP201XX_MODE_NB; ++val) {
			if (sysfs_streq(buf, icp201xx_mode_strings[val]))
				break;
		}
		if (val >= ICP201XX_MODE_NB)
			return -EINVAL;

		min_period = div_u64(NSEC_PER_SEC, icp201xx_max_freq[val]);

		mutex_lock(&indio_dev->mlock);
		if (!iio_buffer_enabled(indio_dev)) {
			/* first ensure period is aligned with new mode */
			spin_lock_irqsave(&st->period_lock, flags);
			st->period = ns_to_ktime(min_period);
			st->frequency = icp201xx_max_freq[val];
			spin_unlock_irqrestore(&st->period_lock, flags);
			/* switch mode */
			atomic_set(&st->mode, val);
			ret = 0;
		} else {
			ret = -EBUSY;
		}
		mutex_unlock(&indio_dev->mlock);
		if (ret)
			return ret;
		break;
	default:
		return -EINVAL;
	}

	return count;
}

static ssize_t icp201xx_period_show(struct device *dev,
				   struct device_attribute *attr, char *buf)
{
	struct icp201xx_state *st = iio_priv(dev_to_iio_dev(dev));
	ktime_t period;
	uint32_t period_us;
	unsigned long flags;

	spin_lock_irqsave(&st->period_lock, flags);
	period = st->period;
	spin_unlock_irqrestore(&st->period_lock, flags);
	period_us = div_u64(ktime_to_ns(period), 1000UL);

	return scnprintf(buf, PAGE_SIZE, "%lu\n", USEC_PER_SEC / period_us);
}

static ssize_t icp201xx_period_store(struct device *dev,
				    struct device_attribute *attr,
				    const char *buf, size_t count)
{
	struct icp201xx_state *st = iio_priv(dev_to_iio_dev(dev));
	int32_t sampling_frequency;
	ktime_t period;
	unsigned long flags;

	if (kstrtoint(buf, 10, &sampling_frequency))
		return -EINVAL;
	if (sampling_frequency < icp201xx_min_freq ||
			sampling_frequency > icp201xx_max_freq[atomic_read(&st->mode)])
		return -EINVAL;

	period = ns_to_ktime(div_u64(NSEC_PER_SEC, sampling_frequency));
	spin_lock_irqsave(&st->period_lock, flags);
	st->period = period;
	st->frequency = sampling_frequency;
	spin_unlock_irqrestore(&st->period_lock, flags);

	if (hrtimer_active(&st->timer)) {
		hrtimer_cancel(&st->timer);
		hrtimer_start(&st->timer, period, HRTIMER_MODE_REL);
	}

	return count;
}

static IIO_DEV_ATTR_SAMP_FREQ(S_IRUGO | S_IWUSR, icp201xx_period_show,
			      icp201xx_period_store);
static IIO_DEVICE_ATTR(sampling_frequency_available, S_IRUGO, icp201xx_attr_show,
		       NULL, ICP201XX_ATTR_SAMP_AVAIL_FREQ);
static IIO_DEVICE_ATTR(mode, S_IRUGO | S_IWUSR, icp201xx_attr_show,
		       icp201xx_attr_store, ICP201XX_ATTR_MODE);
static IIO_DEVICE_ATTR(calibdata, S_IRUGO, icp201xx_attr_show, NULL,
		       ICP201XX_ATTR_CALIBDATA);

static struct attribute *icp201xx_attributes[] = {
	&iio_dev_attr_sampling_frequency.dev_attr.attr,
	&iio_dev_attr_sampling_frequency_available.dev_attr.attr,
	&iio_dev_attr_mode.dev_attr.attr,
	&iio_dev_attr_calibdata.dev_attr.attr,
	NULL,
};

static const struct attribute_group icp201xx_attribute_group = {
	.attrs = icp201xx_attributes,
};

static const struct iio_info icp201xx_info = {
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 15, 0)
	.driver_module = THIS_MODULE,
#endif
	.read_raw = icp201xx_read_raw,
	.attrs = &icp201xx_attribute_group,
	.validate_trigger = icp201xx_validate_trigger,
};

static const struct iio_chan_spec icp201xx_channels[] = {
	{
		.type = IIO_PRESSURE,
		.info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
		.info_mask_shared_by_type = BIT(IIO_CHAN_INFO_OFFSET) |
				BIT(IIO_CHAN_INFO_SCALE),
		.scan_index = ICP201XX_CHANNEL_PRESSURE,
		.scan_type = {
			.sign = 'u',
			.realbits = 24,
			.storagebits = 32,
			.shift = 8,
			.endianness = IIO_BE,
		},
	}, {
		.type = IIO_TEMP,
		.info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
		.info_mask_shared_by_type = BIT(IIO_CHAN_INFO_OFFSET) |
				BIT(IIO_CHAN_INFO_SCALE),
		.scan_index = ICP201XX_CHANNEL_TEMPERATURE,
		.scan_type = {
			.sign = 'u',
			.realbits = 16,
			.storagebits = 32,
			.shift = 16,
			.endianness = IIO_BE,
		},
	},
	IIO_CHAN_SOFT_TIMESTAMP(ICP201XX_CHANNEL_TIMESTAMP),
};

static int icp201xx_probe(struct i2c_client *client,
			 const struct i2c_device_id *id)
{
	struct iio_dev *indio_dev;
	struct icp201xx_state *st;
	int ret;

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		dev_err(&client->dev, "plain i2c transactions not supported\n");
		return -ENODEV;
	}
	/* has to be done before the first I2C communication */
	crc8_populate_msb(icp201xx_crc8_table, ICP201XX_CRC8_POLYNOMIAL);

	indio_dev = iio_device_alloc(sizeof(*st));
	if (indio_dev == NULL) {
		dev_err(&client->dev, "memory allocation failed\n");
		return -ENOMEM;
	}
	i2c_set_clientdata(client, indio_dev);
	indio_dev->dev.parent = &client->dev;
	indio_dev->name = id->name;
	indio_dev->channels = icp201xx_channels;
	indio_dev->num_channels = ARRAY_SIZE(icp201xx_channels);
	indio_dev->info = &icp201xx_info;

	st = iio_priv(indio_dev);
	st->client = client;
	st->chip = id->driver_data;
	hrtimer_init(&st->timer, CLOCK_BOOTTIME, HRTIMER_MODE_REL);
	st->timer.function = icp201xx_timer_handler;
	spin_lock_init(&st->period_lock);
	atomic_set(&st->mode, ICP201XX_MODE_2);
	st->period = ns_to_ktime(NSEC_PER_SEC / 40UL);
	st->frequency = 40;

	ret = icp201xx_init_chip(st);
	if (ret) {
		dev_err(&client->dev, "init chip error %d\n", ret);
		pr_info("init chip error %d\n", ret);
		goto error_free_device;
	}

	ret = iio_triggered_buffer_setup(indio_dev, NULL,
					 icp201xx_read_measurement, NULL);
	if (ret) {
		dev_err(&client->dev, "iio triggered buffer error %d\n", ret);
		goto error_free_device;
	}

	st->trig = iio_trigger_alloc("%s-dev%d", indio_dev->name,
				     indio_dev->id);
	if (st->trig == NULL) {
		ret = -ENOMEM;
		dev_err(&client->dev, "iio trigger alloc error\n");
		goto error_free_buffer;
	}
	st->trig->dev.parent = &client->dev;
	st->trig->ops = &icp201xx_trigger_ops;
	iio_trigger_set_drvdata(st->trig, indio_dev);

	ret = iio_trigger_register(st->trig);
	if (ret) {
		dev_err(&client->dev, "iio trigger register error %d\n", ret);
		goto error_free_trigger;
	}
	iio_trigger_get(st->trig);
	indio_dev->trig = st->trig;

	ret = iio_device_register(indio_dev);
	if (ret) {
		dev_err(&client->dev, "iio device register error %d\n", ret);
		goto error_unregister_trigger;
	}

	pr_info("ICP201xx probe done");
	return 0;

error_unregister_trigger:
	iio_trigger_unregister(st->trig);
error_free_trigger:
	iio_trigger_free(st->trig);
error_free_buffer:
	iio_triggered_buffer_cleanup(indio_dev);
error_free_device:
	iio_device_free(indio_dev);
	return ret;
}

static int icp201xx_remove(struct i2c_client *client)
{
	struct iio_dev *indio_dev = i2c_get_clientdata(client);
	struct icp201xx_state *st = iio_priv(indio_dev);

	iio_device_unregister(indio_dev);
	iio_trigger_unregister(st->trig);
	iio_trigger_free(st->trig);
	iio_triggered_buffer_cleanup(indio_dev);
	iio_device_free(indio_dev);

	return 0;
}

static int icp201xx_suspend(struct device *dev)
{
	struct iio_dev *indio_dev = i2c_get_clientdata(to_i2c_client(dev));
	struct icp201xx_state *st = iio_priv(indio_dev);

	mutex_lock(&indio_dev->mlock);
	if (iio_buffer_enabled(indio_dev))
		hrtimer_cancel(&st->timer);
	mutex_unlock(&indio_dev->mlock);

	return 0;
}

static int icp201xx_resume(struct device *dev)
{
	struct iio_dev *indio_dev = i2c_get_clientdata(to_i2c_client(dev));
	struct icp201xx_state *st = iio_priv(indio_dev);

	mutex_lock(&indio_dev->mlock);
	if (iio_buffer_enabled(indio_dev))
		hrtimer_start(&st->timer, ns_to_ktime(0), HRTIMER_MODE_REL);
	mutex_unlock(&indio_dev->mlock);

	return 0;
}

static UNIVERSAL_DEV_PM_OPS(icp201xx_pm, icp201xx_suspend, icp201xx_resume, NULL);

static const struct of_device_id icp201xx_of_match[] = {
	{
		.compatible = "invensense,icp201xx",
		.data = (void *)ICP201XX,
	},
	{ }
};
MODULE_DEVICE_TABLE(of, icp201xx_of_match);

static const struct i2c_device_id icp201xx_id[] = {
	{ "icp201xx", ICP201XX },
	{ }
};
MODULE_DEVICE_TABLE(i2c, icp201xx_id);

static struct i2c_driver icp201xx_driver = {
	.driver = {
		.owner = THIS_MODULE,
		.name = "icp201xx",
		.pm = &icp201xx_pm,
		.of_match_table = of_match_ptr(icp201xx_of_match),
	},
	.probe = icp201xx_probe,
	.remove = icp201xx_remove,
	.id_table = icp201xx_id,
};
module_i2c_driver(icp201xx_driver);

MODULE_AUTHOR("Invensense Corporation");
MODULE_DESCRIPTION("Invensense ICP201XX driver");
MODULE_LICENSE("GPL");
