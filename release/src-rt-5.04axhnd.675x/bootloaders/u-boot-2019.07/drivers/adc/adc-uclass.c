// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2015 Samsung Electronics
 * Przemyslaw Marczak <p.marczak@samsung.com>
 */

#include <common.h>
#include <errno.h>
#include <div64.h>
#include <dm.h>
#include <dm/lists.h>
#include <dm/device-internal.h>
#include <dm/uclass-internal.h>
#include <adc.h>
#include <power/regulator.h>

#define ADC_UCLASS_PLATDATA_SIZE	sizeof(struct adc_uclass_platdata)
#define CHECK_NUMBER			true
#define CHECK_MASK			(!CHECK_NUMBER)

/* TODO: add support for timer uclass (for early calls) */
#ifdef CONFIG_SANDBOX_ARCH
#define sdelay(x)	udelay(x)
#else
extern void sdelay(unsigned long loops);
#endif

static int check_channel(struct udevice *dev, int value, bool number_or_mask,
			 const char *caller_function)
{
	struct adc_uclass_platdata *uc_pdata = dev_get_uclass_platdata(dev);
	unsigned mask = number_or_mask ? (1 << value) : value;

	/* For the real ADC hardware, some ADC channels can be inactive.
	 * For example if device has 4 analog channels, and only channels
	 * 1-st and 3-rd are valid, then channel mask is: 0b1010, so request
	 * with mask 0b1110 should return an error.
	*/
	if ((uc_pdata->channel_mask >= mask) && (uc_pdata->channel_mask & mask))
		return 0;

	printf("Error in %s/%s().\nWrong channel selection for device: %s\n",
	       __FILE__, caller_function, dev->name);

	return -EINVAL;
}

static int adc_supply_enable(struct udevice *dev)
{
	struct adc_uclass_platdata *uc_pdata = dev_get_uclass_platdata(dev);
	const char *supply_type;
	int ret = 0;

	if (uc_pdata->vdd_supply) {
		supply_type = "vdd";
		ret = regulator_set_enable(uc_pdata->vdd_supply, true);
	}

	if (!ret && uc_pdata->vss_supply) {
		supply_type = "vss";
		ret = regulator_set_enable(uc_pdata->vss_supply, true);
	}

	if (ret)
		pr_err("%s: can't enable %s-supply!", dev->name, supply_type);

	return ret;
}

int adc_data_mask(struct udevice *dev, unsigned int *data_mask)
{
	struct adc_uclass_platdata *uc_pdata = dev_get_uclass_platdata(dev);

	if (!uc_pdata)
		return -ENOSYS;

	*data_mask = uc_pdata->data_mask;
	return 0;
}

int adc_channel_mask(struct udevice *dev, unsigned int *channel_mask)
{
	struct adc_uclass_platdata *uc_pdata = dev_get_uclass_platdata(dev);

	if (!uc_pdata)
		return -ENOSYS;

	*channel_mask = uc_pdata->channel_mask;

	return 0;
}

int adc_stop(struct udevice *dev)
{
	const struct adc_ops *ops = dev_get_driver_ops(dev);

	if (!ops->stop)
		return -ENOSYS;

	return ops->stop(dev);
}

int adc_start_channel(struct udevice *dev, int channel)
{
	const struct adc_ops *ops = dev_get_driver_ops(dev);
	int ret;

	if (!ops->start_channel)
		return -ENOSYS;

	ret = check_channel(dev, channel, CHECK_NUMBER, __func__);
	if (ret)
		return ret;

	ret = adc_supply_enable(dev);
	if (ret)
		return ret;

	return ops->start_channel(dev, channel);
}

int adc_start_channels(struct udevice *dev, unsigned int channel_mask)
{
	const struct adc_ops *ops = dev_get_driver_ops(dev);
	int ret;

	if (!ops->start_channels)
		return -ENOSYS;

	ret = check_channel(dev, channel_mask, CHECK_MASK, __func__);
	if (ret)
		return ret;

	ret = adc_supply_enable(dev);
	if (ret)
		return ret;

	return ops->start_channels(dev, channel_mask);
}

int adc_channel_data(struct udevice *dev, int channel, unsigned int *data)
{
	struct adc_uclass_platdata *uc_pdata = dev_get_uclass_platdata(dev);
	const struct adc_ops *ops = dev_get_driver_ops(dev);
	unsigned int timeout_us = uc_pdata->data_timeout_us;
	int ret;

	if (!ops->channel_data)
		return -ENOSYS;

	ret = check_channel(dev, channel, CHECK_NUMBER, __func__);
	if (ret)
		return ret;

	do {
		ret = ops->channel_data(dev, channel, data);
		if (!ret || ret != -EBUSY)
			break;

		/* TODO: use timer uclass (for early calls). */
		sdelay(5);
	} while (timeout_us--);

	return ret;
}

int adc_channels_data(struct udevice *dev, unsigned int channel_mask,
		      struct adc_channel *channels)
{
	struct adc_uclass_platdata *uc_pdata = dev_get_uclass_platdata(dev);
	unsigned int timeout_us = uc_pdata->multidata_timeout_us;
	const struct adc_ops *ops = dev_get_driver_ops(dev);
	int ret;

	if (!ops->channels_data)
		return -ENOSYS;

	ret = check_channel(dev, channel_mask, CHECK_MASK, __func__);
	if (ret)
		return ret;

	do {
		ret = ops->channels_data(dev, channel_mask, channels);
		if (!ret || ret != -EBUSY)
			break;

		/* TODO: use timer uclass (for early calls). */
		sdelay(5);
	} while (timeout_us--);

	return ret;
}

int adc_channel_single_shot(const char *name, int channel, unsigned int *data)
{
	struct udevice *dev;
	int ret;

	ret = uclass_get_device_by_name(UCLASS_ADC, name, &dev);
	if (ret)
		return ret;

	ret = adc_start_channel(dev, channel);
	if (ret)
		return ret;

	ret = adc_channel_data(dev, channel, data);
	if (ret)
		return ret;

	return 0;
}

static int _adc_channels_single_shot(struct udevice *dev,
				     unsigned int channel_mask,
				     struct adc_channel *channels)
{
	unsigned int data;
	int channel, ret;

	for (channel = 0; channel <= ADC_MAX_CHANNEL; channel++) {
		/* Check channel bit. */
		if (!((channel_mask >> channel) & 0x1))
			continue;

		ret = adc_start_channel(dev, channel);
		if (ret)
			return ret;

		ret = adc_channel_data(dev, channel, &data);
		if (ret)
			return ret;

		channels->id = channel;
		channels->data = data;
		channels++;
	}

	return 0;
}

int adc_channels_single_shot(const char *name, unsigned int channel_mask,
			     struct adc_channel *channels)
{
	struct udevice *dev;
	int ret;

	ret = uclass_get_device_by_name(UCLASS_ADC, name, &dev);
	if (ret)
		return ret;

	ret = adc_start_channels(dev, channel_mask);
	if (ret)
		goto try_manual;

	ret = adc_channels_data(dev, channel_mask, channels);
	if (ret)
		return ret;

	return 0;

try_manual:
	if (ret != -ENOSYS)
		return ret;

	return _adc_channels_single_shot(dev, channel_mask, channels);
}

static int adc_vdd_platdata_update(struct udevice *dev)
{
	struct adc_uclass_platdata *uc_pdata = dev_get_uclass_platdata(dev);
	int ret;

	/* Warning!
	 * This function can't return supply device before its bind.
	 * Please pay attention to proper fdt scan sequence. If ADC device
	 * will bind before its supply regulator device, then the below 'get'
	 * will return an error.
	 */
	if (!uc_pdata->vdd_supply)
		return 0;

	ret = regulator_get_value(uc_pdata->vdd_supply);
	if (ret < 0)
		return ret;

	uc_pdata->vdd_microvolts = ret;

	return 0;
}

static int adc_vss_platdata_update(struct udevice *dev)
{
	struct adc_uclass_platdata *uc_pdata = dev_get_uclass_platdata(dev);
	int ret;

	if (!uc_pdata->vss_supply)
		return 0;

	ret = regulator_get_value(uc_pdata->vss_supply);
	if (ret < 0)
		return ret;

	uc_pdata->vss_microvolts = ret;

	return 0;
}

int adc_vdd_value(struct udevice *dev, int *uV)
{
	struct adc_uclass_platdata *uc_pdata = dev_get_uclass_platdata(dev);
	int ret, value_sign = uc_pdata->vdd_polarity_negative ? -1 : 1;

	/* Update the regulator Value. */
	ret = adc_vdd_platdata_update(dev);
	if (ret)
		return ret;

	if (uc_pdata->vdd_microvolts == -ENODATA)
		return -ENODATA;

	*uV = uc_pdata->vdd_microvolts * value_sign;

	return 0;
}

int adc_vss_value(struct udevice *dev, int *uV)
{
	struct adc_uclass_platdata *uc_pdata = dev_get_uclass_platdata(dev);
	int ret, value_sign = uc_pdata->vss_polarity_negative ? -1 : 1;

	/* Update the regulator Value. */
	ret = adc_vss_platdata_update(dev);
	if (ret)
		return ret;

	if (uc_pdata->vss_microvolts == -ENODATA)
		return -ENODATA;

	*uV = uc_pdata->vss_microvolts * value_sign;

	return 0;
}

int adc_raw_to_uV(struct udevice *dev, unsigned int raw, int *uV)
{
	unsigned int data_mask;
	int ret, val, vref;
	u64 raw64 = raw;

	ret = adc_vdd_value(dev, &vref);
	if (ret)
		return ret;

	if (!adc_vss_value(dev, &val))
		vref -= val;

	ret = adc_data_mask(dev, &data_mask);
	if (ret)
		return ret;

	raw64 *= vref;
	do_div(raw64, data_mask);
	*uV = raw64;

	return 0;
}

static int adc_vdd_platdata_set(struct udevice *dev)
{
	struct adc_uclass_platdata *uc_pdata = dev_get_uclass_platdata(dev);
	int ret;
	char *prop;

	prop = "vdd-polarity-negative";
	uc_pdata->vdd_polarity_negative = dev_read_bool(dev, prop);

	/* Optionally get regulators */
	ret = device_get_supply_regulator(dev, "vdd-supply",
					  &uc_pdata->vdd_supply);
	if (!ret)
		return adc_vdd_platdata_update(dev);

	if (ret != -ENOENT)
		return ret;

	/* No vdd-supply phandle. */
	prop  = "vdd-microvolts";
	uc_pdata->vdd_microvolts = dev_read_u32_default(dev, prop, -ENODATA);

	return 0;
}

static int adc_vss_platdata_set(struct udevice *dev)
{
	struct adc_uclass_platdata *uc_pdata = dev_get_uclass_platdata(dev);
	int ret;
	char *prop;

	prop = "vss-polarity-negative";
	uc_pdata->vss_polarity_negative = dev_read_bool(dev, prop);

	ret = device_get_supply_regulator(dev, "vss-supply",
					  &uc_pdata->vss_supply);
	if (!ret)
		return adc_vss_platdata_update(dev);

	if (ret != -ENOENT)
		return ret;

	/* No vss-supply phandle. */
	prop = "vss-microvolts";
	uc_pdata->vss_microvolts = dev_read_u32_default(dev, prop, -ENODATA);

	return 0;
}

static int adc_pre_probe(struct udevice *dev)
{
	int ret;

	/* Set ADC VDD platdata: polarity, uV, regulator (phandle). */
	ret = adc_vdd_platdata_set(dev);
	if (ret)
		pr_err("%s: Can't update Vdd. Error: %d", dev->name, ret);

	/* Set ADC VSS platdata: polarity, uV, regulator (phandle). */
	ret = adc_vss_platdata_set(dev);
	if (ret)
		pr_err("%s: Can't update Vss. Error: %d", dev->name, ret);

	return 0;
}

UCLASS_DRIVER(adc) = {
	.id	= UCLASS_ADC,
	.name	= "adc",
	.pre_probe =  adc_pre_probe,
	.per_device_platdata_auto_alloc_size = ADC_UCLASS_PLATDATA_SIZE,
};
