/*
 * Copyright (c) 2022 CoretronicMEMS
 *
 * Driver for CMC CMP201 digital pressure sensor.
 *
 */

#include <linux/completion.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/gpio/consumer.h>
#include <linux/iio/iio.h>
#include <linux/iio/sysfs.h>
#include <linux/interrupt.h>
#include <linux/module.h>
#include <linux/pm_runtime.h>
#include <linux/random.h>
#include <linux/regmap.h>
#include <linux/regulator/consumer.h>

#include "cmp201.h"

struct cmp201_data {
  struct device* dev;
  struct mutex lock;
  struct regmap* regmap;
  struct completion done;
  const struct cmp201_chip_info* chip_info;
  struct regulator* vddd;
  struct regulator* vdda;
  unsigned int start_up_time; /* in microseconds */
  /* log of base 2 of oversampling rate */
  u8 oversampling_press;
  u8 oversampling_temp;

  /*
   * Carryover value from temperature conversion, used in pressure
   * calculation.
   */
  s32 t_fine;
};

struct cmp201_chip_info {
  const int* oversampling_temp_avail;
  int num_oversampling_temp_avail;

  const int* oversampling_press_avail;
  int num_oversampling_press_avail;

  int (*chip_config)(struct cmp201_data*);
  int (*read_temp)(struct cmp201_data*, int*);
  int (*read_press)(struct cmp201_data*, int*, int*);
};

static const struct iio_chan_spec cmp201_channels[] = {
    {
        .type = IIO_PRESSURE,
        .info_mask_separate = BIT(IIO_CHAN_INFO_PROCESSED) |
                              BIT(IIO_CHAN_INFO_OVERSAMPLING_RATIO),
    },
    {
        .type = IIO_TEMP,
        .info_mask_separate = BIT(IIO_CHAN_INFO_PROCESSED) |
                              BIT(IIO_CHAN_INFO_OVERSAMPLING_RATIO),
    },
};

static int cmp201_read_temp(struct cmp201_data* data, int* val) {
  int ret;
  u8 buf[3] = {0};
  s32 adc_temp;

  ret = regmap_bulk_read(data->regmap, CMP201_REG_TEMP_XLSB, buf, 3);
  if (ret < 0) {
    dev_err(data->dev, "failed to read temperature\n");
    return ret;
  }

  // adc_temp = be32_to_cpu(tmp) >> 12;
  adc_temp = (s32)((buf[2] << 16) | (buf[1] << 8) | buf[0]);

  printk(KERN_DEBUG
        "CMP201 debug:\n"
        "  adc_temp     = dec:%d  hex:0x%08X\n",
        adc_temp, (u32)adc_temp);

  /*
   * val might be NULL if we're called by the read_press routine,
   * who only cares about the carry over t_fine value.
   */
  if (val) {
    *val = adc_temp * 10;
    return IIO_VAL_INT;
  }

  // final result needs to divide 655360 into Celsius
  return 0;
}

static int cmp201_read_press(struct cmp201_data* data, int* val, int* val2) {
  int ret;
  u8 buf[3] = {0};
  s32 adc_press;

  ret = regmap_bulk_read(data->regmap, CMP201_REG_PRESS_XLSB, buf, 3);
  if (ret < 0) {
    dev_err(data->dev, "failed to read pressure\n");
    return ret;
  }

  // adc_press = be32_to_cpu(tmp) >> 12;
  adc_press = (s32)((buf[2] << 16) | (buf[1] << 8) | buf[0]);

  printk(KERN_DEBUG
        "CMP201 debug:\n"
        "  adc_press     = dec:%d  hex:0x%08X\n",
        adc_press, (u32)adc_press);

  *val = adc_press;
  *val2 = 64;

  // final result is Pa, if we want Kpa, need to divide 1000
  return IIO_VAL_FRACTIONAL;
}

static int cmp201_read_raw(struct iio_dev* indio_dev,
                           struct iio_chan_spec const* chan, int* val,
                           int* val2, long mask) {
  int ret;
  struct cmp201_data* data = iio_priv(indio_dev);

  pm_runtime_get_sync(data->dev);
  mutex_lock(&data->lock);

  switch (mask) {
    case IIO_CHAN_INFO_PROCESSED:
      switch (chan->type) {
        case IIO_PRESSURE:
          ret = data->chip_info->read_press(data, val, val2);
          break;
        case IIO_TEMP:
          ret = data->chip_info->read_temp(data, val);
          break;
        default:
          ret = -EINVAL;
          break;
      }
      break;
    case IIO_CHAN_INFO_OVERSAMPLING_RATIO:
      switch (chan->type) {
        case IIO_PRESSURE:
          *val = 1 << data->oversampling_press;
          ret = IIO_VAL_INT;
          break;
        case IIO_TEMP:
          *val = 1 << data->oversampling_temp;
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

  mutex_unlock(&data->lock);
  pm_runtime_mark_last_busy(data->dev);
  pm_runtime_put_autosuspend(data->dev);

  return ret;
}

static int cmp201_write_oversampling_ratio_temp(struct cmp201_data* data,
                                                int val) {
  int i;
  const int* avail = data->chip_info->oversampling_temp_avail;
  const int n = data->chip_info->num_oversampling_temp_avail;

  for (i = 0; i < n; i++) {
    if (avail[i] == val) {
      data->oversampling_temp = ilog2(val);

      return data->chip_info->chip_config(data);
    }
  }
  return -EINVAL;
}

static int cmp201_write_oversampling_ratio_press(struct cmp201_data* data,
                                                 int val) {
  int i;
  const int* avail = data->chip_info->oversampling_press_avail;
  const int n = data->chip_info->num_oversampling_press_avail;

  for (i = 0; i < n; i++) {
    if (avail[i] == val) {
      data->oversampling_press = ilog2(val);

      return data->chip_info->chip_config(data);
    }
  }
  return -EINVAL;
}

static int cmp201_write_raw(struct iio_dev* indio_dev,
                            struct iio_chan_spec const* chan, int val, int val2,
                            long mask) {
  int ret = 0;
  struct cmp201_data* data = iio_priv(indio_dev);

  switch (mask) {
    case IIO_CHAN_INFO_OVERSAMPLING_RATIO:
      pm_runtime_get_sync(data->dev);
      mutex_lock(&data->lock);
      switch (chan->type) {
        case IIO_PRESSURE:
          ret = cmp201_write_oversampling_ratio_press(data, val);
          break;
        case IIO_TEMP:
          ret = cmp201_write_oversampling_ratio_temp(data, val);
          break;
        default:
          ret = -EINVAL;
          break;
      }
      mutex_unlock(&data->lock);
      pm_runtime_mark_last_busy(data->dev);
      pm_runtime_put_autosuspend(data->dev);
      break;
    default:
      return -EINVAL;
  }

  return ret;
}

static ssize_t cmp201_show_avail(char* buf, const int* vals, const int n) {
  size_t len = 0;
  int i;

  for (i = 0; i < n; i++)
    len += scnprintf(buf + len, PAGE_SIZE - len, "%d ", vals[i]);

  buf[len - 1] = '\n';

  return len;
}

static ssize_t cmp201_show_temp_oversampling_avail(
    struct device* dev, struct device_attribute* attr, char* buf) {
  struct cmp201_data* data = iio_priv(dev_to_iio_dev(dev));

  return cmp201_show_avail(buf, data->chip_info->oversampling_temp_avail,
                           data->chip_info->num_oversampling_temp_avail);
}

static ssize_t cmp201_show_press_oversampling_avail(
    struct device* dev, struct device_attribute* attr, char* buf) {
  struct cmp201_data* data = iio_priv(dev_to_iio_dev(dev));

  return cmp201_show_avail(buf, data->chip_info->oversampling_press_avail,
                           data->chip_info->num_oversampling_press_avail);
}

static IIO_DEVICE_ATTR(in_temp_oversampling_ratio_available, S_IRUGO,
                       cmp201_show_temp_oversampling_avail, NULL, 0);

static IIO_DEVICE_ATTR(in_pressure_oversampling_ratio_available, S_IRUGO,
                       cmp201_show_press_oversampling_avail, NULL, 0);

static struct attribute* cmp201_attributes[] = {
    &iio_dev_attr_in_temp_oversampling_ratio_available.dev_attr.attr,
    &iio_dev_attr_in_pressure_oversampling_ratio_available.dev_attr.attr,
    NULL,
};

static const struct attribute_group cmp201_attrs_group = {
    .attrs = cmp201_attributes,
};

static const struct iio_info cmp201_info = {
    .read_raw = &cmp201_read_raw,
    .write_raw = &cmp201_write_raw,
    .attrs = &cmp201_attrs_group,
};

static int cmp201_chip_config(struct cmp201_data* data) {
  int ret;

  ret = regmap_write(data->regmap, CMP201_REG_RESET, 0xB6);

  /* Wait to make sure we started up properly */
  usleep_range(data->start_up_time, data->start_up_time + 100);

  ret = regmap_write(data->regmap, CMP201_REG_FIFO_CONFIG_1, 0x00);
  if (ret) return ret;
  ret = regmap_write(data->regmap, CMP201_REG_INT_CTRL, 0x40);
  if (ret) return ret;
  ret = regmap_write(data->regmap, CMP201_REG_OSR, 0x77);
  if (ret) return ret;
  ret = regmap_write(data->regmap, CMP201_REG_ODR, 0x16);
  if (ret) return ret;
  ret = regmap_write(data->regmap, CMP201_REG_FILTER, 0x00);
  if (ret) return ret;
  ret = regmap_write(data->regmap, CMP201_REG_PWR_CTRL, 0x33);
  if (ret) return ret;
  return ret;
}

static const int cmp201_oversampling_avail[] = {1, 2, 4, 8, 16, 32, 64, 128};

static const struct cmp201_chip_info cmp201_chip_info = {
    .oversampling_temp_avail = cmp201_oversampling_avail,
    .num_oversampling_temp_avail = ARRAY_SIZE(cmp201_oversampling_avail),

    .oversampling_press_avail = cmp201_oversampling_avail,
    .num_oversampling_press_avail = ARRAY_SIZE(cmp201_oversampling_avail),

    .chip_config = cmp201_chip_config,
    .read_temp = cmp201_read_temp,
    .read_press = cmp201_read_press,
};

int cmp201_common_probe(struct device* dev, struct regmap* regmap,
                        unsigned int chip, const char* name, int irq) {
  int ret;
  struct iio_dev* indio_dev;
  struct cmp201_data* data;
  unsigned int chip_id;
  struct gpio_desc* gpiod;

  indio_dev = devm_iio_device_alloc(dev, sizeof(*data));
  if (!indio_dev) return -ENOMEM;

  data = iio_priv(indio_dev);
  mutex_init(&data->lock);
  data->dev = dev;

  indio_dev->dev.parent = dev;
  indio_dev->name = name;
  indio_dev->channels = cmp201_channels;
  indio_dev->info = &cmp201_info;
  indio_dev->modes = INDIO_DIRECT_MODE;

  switch (chip) {
    case CMP201_CHIP_ID:
      indio_dev->num_channels = 2;
      data->chip_info = &cmp201_chip_info;
      data->oversampling_press = ilog2(128);
      data->oversampling_temp = ilog2(128);
      data->start_up_time = 2000;
      break;
    default:
      return -EINVAL;
  }

  dev_info(dev, "Initializing regulators...\n");
  /* Bring up regulators */
  data->vddd = devm_regulator_get(dev, "vddd");
  if (IS_ERR(data->vddd)) {
    dev_err(dev, "failed to get VDDD regulator\n");
    return PTR_ERR(data->vddd);
  }
  dev_info(dev, "VDDD regulator acquired\n");
  ret = regulator_enable(data->vddd);
  if (ret) {
    dev_err(dev, "failed to enable VDDD regulator\n");
    return ret;
  }
  dev_info(dev, "VDDD regulator enabled\n");

  data->vdda = devm_regulator_get(dev, "vdda");
  if (IS_ERR(data->vdda)) {
    dev_err(dev, "failed to get VDDA regulator\n");
    ret = PTR_ERR(data->vdda);
    goto out_disable_vddd;
  }
  dev_info(dev, "VDDA regulator acquired\n");
  ret = regulator_enable(data->vdda);
  if (ret) {
    dev_err(dev, "failed to enable VDDA regulator\n");
    goto out_disable_vddd;
  }
  dev_info(dev, "VDDA regulator enabled\n");

  /* Wait to make sure we started up properly */
  usleep_range(data->start_up_time, data->start_up_time + 100);

  /* Bring chip out of reset if there is an assigned GPIO line */
  gpiod = devm_gpiod_get(dev, "reset", GPIOD_OUT_HIGH);

  /* Deassert the signal */
  if (!IS_ERR(gpiod)) {
    dev_info(dev, "release reset\n");
    gpiod_set_value(gpiod, 0);
  }

  data->regmap = regmap;
  ret = regmap_read(regmap, CMP201_REG_CHIP_ID, &chip_id);
  if (ret < 0) goto out_disable_vdda;
  dev_info(dev, "chip is %x, chip_id is %x\n", chip, chip_id);
  if (chip_id != chip) {
    dev_err(dev, "bad chip id: expected %x got %x\n", chip, chip_id);
    return -EINVAL;
    goto out_disable_vdda;
  }

  /* cmp201 initial process */
  ret = data->chip_info->chip_config(data);
  if (ret < 0) goto out_disable_vdda;

  dev_set_drvdata(dev, indio_dev);

  /* Enable runtime PM */
  pm_runtime_get_noresume(dev);
  pm_runtime_set_active(dev);
  pm_runtime_enable(dev);
  /*
   * Set autosuspend to two orders of magnitude larger than the
   * start-up time.
   */
  pm_runtime_set_autosuspend_delay(dev, data->start_up_time / 10);
  pm_runtime_use_autosuspend(dev);
  pm_runtime_put(dev);

  ret = iio_device_register(indio_dev);
  if (ret) goto out_runtime_pm_disable;

  return 0;

out_runtime_pm_disable:
  pm_runtime_get_sync(data->dev);
  pm_runtime_put_noidle(data->dev);
  pm_runtime_disable(data->dev);
out_disable_vdda:
  regulator_disable(data->vdda);
out_disable_vddd:
  regulator_disable(data->vddd);
  return ret;
}
EXPORT_SYMBOL(cmp201_common_probe);

int cmp201_common_remove(struct device* dev) {
  struct iio_dev* indio_dev = dev_get_drvdata(dev);
  struct cmp201_data* data = iio_priv(indio_dev);

  iio_device_unregister(indio_dev);
  pm_runtime_get_sync(data->dev);
  pm_runtime_put_noidle(data->dev);
  pm_runtime_disable(data->dev);
  regulator_disable(data->vdda);
  regulator_disable(data->vddd);
  return 0;
}
EXPORT_SYMBOL(cmp201_common_remove);

#ifdef CONFIG_PM
static int cmp201_runtime_suspend(struct device* dev) {
  struct iio_dev* indio_dev = dev_get_drvdata(dev);
  struct cmp201_data* data = iio_priv(indio_dev);
  int ret;

  ret = regulator_disable(data->vdda);
  if (ret) return ret;
  return regulator_disable(data->vddd);
}

static int cmp201_runtime_resume(struct device* dev) {
  struct iio_dev* indio_dev = dev_get_drvdata(dev);
  struct cmp201_data* data = iio_priv(indio_dev);
  int ret;

  ret = regulator_enable(data->vddd);
  if (ret) return ret;
  ret = regulator_enable(data->vdda);
  if (ret) return ret;
  usleep_range(data->start_up_time, data->start_up_time + 100);
  return data->chip_info->chip_config(data);  //  cmp201 initial process
}
#endif /* CONFIG_PM */

const struct dev_pm_ops cmp201_dev_pm_ops = {
    SET_SYSTEM_SLEEP_PM_OPS(pm_runtime_force_suspend, pm_runtime_force_resume)
        SET_RUNTIME_PM_OPS(cmp201_runtime_suspend, cmp201_runtime_resume,
                           NULL)};
EXPORT_SYMBOL(cmp201_dev_pm_ops);

MODULE_AUTHOR("CoretronicMEMS");
MODULE_DESCRIPTION("Driver for CMC CMP201 pressure and temperature sensor");
MODULE_LICENSE("GPL v2");
