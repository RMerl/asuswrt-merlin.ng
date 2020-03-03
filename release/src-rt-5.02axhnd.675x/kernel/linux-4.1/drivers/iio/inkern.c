/* The industrial I/O core in kernel channel mapping
 *
 * Copyright (c) 2011 Jonathan Cameron
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published by
 * the Free Software Foundation.
 */
#include <linux/err.h>
#include <linux/export.h>
#include <linux/slab.h>
#include <linux/mutex.h>
#include <linux/of.h>

#include <linux/iio/iio.h>
#include "iio_core.h"
#include <linux/iio/machine.h>
#include <linux/iio/driver.h>
#include <linux/iio/consumer.h>

struct iio_map_internal {
	struct iio_dev *indio_dev;
	struct iio_map *map;
	struct list_head l;
};

static LIST_HEAD(iio_map_list);
static DEFINE_MUTEX(iio_map_list_lock);

int iio_map_array_register(struct iio_dev *indio_dev, struct iio_map *maps)
{
	int i = 0, ret = 0;
	struct iio_map_internal *mapi;

	if (maps == NULL)
		return 0;

	mutex_lock(&iio_map_list_lock);
	while (maps[i].consumer_dev_name != NULL) {
		mapi = kzalloc(sizeof(*mapi), GFP_KERNEL);
		if (mapi == NULL) {
			ret = -ENOMEM;
			goto error_ret;
		}
		mapi->map = &maps[i];
		mapi->indio_dev = indio_dev;
		list_add(&mapi->l, &iio_map_list);
		i++;
	}
error_ret:
	mutex_unlock(&iio_map_list_lock);

	return ret;
}
EXPORT_SYMBOL_GPL(iio_map_array_register);


/*
 * Remove all map entries associated with the given iio device
 */
int iio_map_array_unregister(struct iio_dev *indio_dev)
{
	int ret = -ENODEV;
	struct iio_map_internal *mapi;
	struct list_head *pos, *tmp;

	mutex_lock(&iio_map_list_lock);
	list_for_each_safe(pos, tmp, &iio_map_list) {
		mapi = list_entry(pos, struct iio_map_internal, l);
		if (indio_dev == mapi->indio_dev) {
			list_del(&mapi->l);
			kfree(mapi);
			ret = 0;
		}
	}
	mutex_unlock(&iio_map_list_lock);
	return ret;
}
EXPORT_SYMBOL_GPL(iio_map_array_unregister);

static const struct iio_chan_spec
*iio_chan_spec_from_name(const struct iio_dev *indio_dev, const char *name)
{
	int i;
	const struct iio_chan_spec *chan = NULL;

	for (i = 0; i < indio_dev->num_channels; i++)
		if (indio_dev->channels[i].datasheet_name &&
		    strcmp(name, indio_dev->channels[i].datasheet_name) == 0) {
			chan = &indio_dev->channels[i];
			break;
		}
	return chan;
}

#ifdef CONFIG_OF

static int iio_dev_node_match(struct device *dev, void *data)
{
	return dev->of_node == data && dev->type == &iio_device_type;
}

/**
 * __of_iio_simple_xlate - translate iiospec to the IIO channel index
 * @indio_dev:	pointer to the iio_dev structure
 * @iiospec:	IIO specifier as found in the device tree
 *
 * This is simple translation function, suitable for the most 1:1 mapped
 * channels in IIO chips. This function performs only one sanity check:
 * whether IIO index is less than num_channels (that is specified in the
 * iio_dev).
 */
static int __of_iio_simple_xlate(struct iio_dev *indio_dev,
				const struct of_phandle_args *iiospec)
{
	if (!iiospec->args_count)
		return 0;

	if (iiospec->args[0] >= indio_dev->num_channels) {
		dev_err(&indio_dev->dev, "invalid channel index %u\n",
			iiospec->args[0]);
		return -EINVAL;
	}

	return iiospec->args[0];
}

static int __of_iio_channel_get(struct iio_channel *channel,
				struct device_node *np, int index)
{
	struct device *idev;
	struct iio_dev *indio_dev;
	int err;
	struct of_phandle_args iiospec;

	err = of_parse_phandle_with_args(np, "io-channels",
					 "#io-channel-cells",
					 index, &iiospec);
	if (err)
		return err;

	idev = bus_find_device(&iio_bus_type, NULL, iiospec.np,
			       iio_dev_node_match);
	of_node_put(iiospec.np);
	if (idev == NULL)
		return -EPROBE_DEFER;

	indio_dev = dev_to_iio_dev(idev);
	channel->indio_dev = indio_dev;
	if (indio_dev->info->of_xlate)
		index = indio_dev->info->of_xlate(indio_dev, &iiospec);
	else
		index = __of_iio_simple_xlate(indio_dev, &iiospec);
	if (index < 0)
		goto err_put;
	channel->channel = &indio_dev->channels[index];

	return 0;

err_put:
	iio_device_put(indio_dev);
	return index;
}

static struct iio_channel *of_iio_channel_get(struct device_node *np, int index)
{
	struct iio_channel *channel;
	int err;

	if (index < 0)
		return ERR_PTR(-EINVAL);

	channel = kzalloc(sizeof(*channel), GFP_KERNEL);
	if (channel == NULL)
		return ERR_PTR(-ENOMEM);

	err = __of_iio_channel_get(channel, np, index);
	if (err)
		goto err_free_channel;

	return channel;

err_free_channel:
	kfree(channel);
	return ERR_PTR(err);
}

static struct iio_channel *of_iio_channel_get_by_name(struct device_node *np,
						      const char *name)
{
	struct iio_channel *chan = NULL;

	/* Walk up the tree of devices looking for a matching iio channel */
	while (np) {
		int index = 0;

		/*
		 * For named iio channels, first look up the name in the
		 * "io-channel-names" property.  If it cannot be found, the
		 * index will be an error code, and of_iio_channel_get()
		 * will fail.
		 */
		if (name)
			index = of_property_match_string(np, "io-channel-names",
							 name);
		chan = of_iio_channel_get(np, index);
		if (!IS_ERR(chan) || PTR_ERR(chan) == -EPROBE_DEFER)
			break;
		else if (name && index >= 0) {
			pr_err("ERROR: could not get IIO channel %s:%s(%i)\n",
				np->full_name, name ? name : "", index);
			return NULL;
		}

		/*
		 * No matching IIO channel found on this node.
		 * If the parent node has a "io-channel-ranges" property,
		 * then we can try one of its channels.
		 */
		np = np->parent;
		if (np && !of_get_property(np, "io-channel-ranges", NULL))
			return NULL;
	}

	return chan;
}

static struct iio_channel *of_iio_channel_get_all(struct device *dev)
{
	struct iio_channel *chans;
	int i, mapind, nummaps = 0;
	int ret;

	do {
		ret = of_parse_phandle_with_args(dev->of_node,
						 "io-channels",
						 "#io-channel-cells",
						 nummaps, NULL);
		if (ret < 0)
			break;
	} while (++nummaps);

	if (nummaps == 0)	/* no error, return NULL to search map table */
		return NULL;

	/* NULL terminated array to save passing size */
	chans = kcalloc(nummaps + 1, sizeof(*chans), GFP_KERNEL);
	if (chans == NULL)
		return ERR_PTR(-ENOMEM);

	/* Search for OF matches */
	for (mapind = 0; mapind < nummaps; mapind++) {
		ret = __of_iio_channel_get(&chans[mapind], dev->of_node,
					   mapind);
		if (ret)
			goto error_free_chans;
	}
	return chans;

error_free_chans:
	for (i = 0; i < mapind; i++)
		iio_device_put(chans[i].indio_dev);
	kfree(chans);
	return ERR_PTR(ret);
}

#else /* CONFIG_OF */

static inline struct iio_channel *
of_iio_channel_get_by_name(struct device_node *np, const char *name)
{
	return NULL;
}

static inline struct iio_channel *of_iio_channel_get_all(struct device *dev)
{
	return NULL;
}

#endif /* CONFIG_OF */

static struct iio_channel *iio_channel_get_sys(const char *name,
					       const char *channel_name)
{
	struct iio_map_internal *c_i = NULL, *c = NULL;
	struct iio_channel *channel;
	int err;

	if (name == NULL && channel_name == NULL)
		return ERR_PTR(-ENODEV);

	/* first find matching entry the channel map */
	mutex_lock(&iio_map_list_lock);
	list_for_each_entry(c_i, &iio_map_list, l) {
		if ((name && strcmp(name, c_i->map->consumer_dev_name) != 0) ||
		    (channel_name &&
		     strcmp(channel_name, c_i->map->consumer_channel) != 0))
			continue;
		c = c_i;
		iio_device_get(c->indio_dev);
		break;
	}
	mutex_unlock(&iio_map_list_lock);
	if (c == NULL)
		return ERR_PTR(-ENODEV);

	channel = kzalloc(sizeof(*channel), GFP_KERNEL);
	if (channel == NULL) {
		err = -ENOMEM;
		goto error_no_mem;
	}

	channel->indio_dev = c->indio_dev;

	if (c->map->adc_channel_label) {
		channel->channel =
			iio_chan_spec_from_name(channel->indio_dev,
						c->map->adc_channel_label);

		if (channel->channel == NULL) {
			err = -EINVAL;
			goto error_no_chan;
		}
	}

	return channel;

error_no_chan:
	kfree(channel);
error_no_mem:
	iio_device_put(c->indio_dev);
	return ERR_PTR(err);
}

struct iio_channel *iio_channel_get(struct device *dev,
				    const char *channel_name)
{
	const char *name = dev ? dev_name(dev) : NULL;
	struct iio_channel *channel;

	if (dev) {
		channel = of_iio_channel_get_by_name(dev->of_node,
						     channel_name);
		if (channel != NULL)
			return channel;
	}

	return iio_channel_get_sys(name, channel_name);
}
EXPORT_SYMBOL_GPL(iio_channel_get);

void iio_channel_release(struct iio_channel *channel)
{
	iio_device_put(channel->indio_dev);
	kfree(channel);
}
EXPORT_SYMBOL_GPL(iio_channel_release);

struct iio_channel *iio_channel_get_all(struct device *dev)
{
	const char *name;
	struct iio_channel *chans;
	struct iio_map_internal *c = NULL;
	int nummaps = 0;
	int mapind = 0;
	int i, ret;

	if (dev == NULL)
		return ERR_PTR(-EINVAL);

	chans = of_iio_channel_get_all(dev);
	if (chans)
		return chans;

	name = dev_name(dev);

	mutex_lock(&iio_map_list_lock);
	/* first count the matching maps */
	list_for_each_entry(c, &iio_map_list, l)
		if (name && strcmp(name, c->map->consumer_dev_name) != 0)
			continue;
		else
			nummaps++;

	if (nummaps == 0) {
		ret = -ENODEV;
		goto error_ret;
	}

	/* NULL terminated array to save passing size */
	chans = kzalloc(sizeof(*chans)*(nummaps + 1), GFP_KERNEL);
	if (chans == NULL) {
		ret = -ENOMEM;
		goto error_ret;
	}

	/* for each map fill in the chans element */
	list_for_each_entry(c, &iio_map_list, l) {
		if (name && strcmp(name, c->map->consumer_dev_name) != 0)
			continue;
		chans[mapind].indio_dev = c->indio_dev;
		chans[mapind].data = c->map->consumer_data;
		chans[mapind].channel =
			iio_chan_spec_from_name(chans[mapind].indio_dev,
						c->map->adc_channel_label);
		if (chans[mapind].channel == NULL) {
			ret = -EINVAL;
			goto error_free_chans;
		}
		iio_device_get(chans[mapind].indio_dev);
		mapind++;
	}
	if (mapind == 0) {
		ret = -ENODEV;
		goto error_free_chans;
	}
	mutex_unlock(&iio_map_list_lock);

	return chans;

error_free_chans:
	for (i = 0; i < nummaps; i++)
		iio_device_put(chans[i].indio_dev);
	kfree(chans);
error_ret:
	mutex_unlock(&iio_map_list_lock);

	return ERR_PTR(ret);
}
EXPORT_SYMBOL_GPL(iio_channel_get_all);

void iio_channel_release_all(struct iio_channel *channels)
{
	struct iio_channel *chan = &channels[0];

	while (chan->indio_dev) {
		iio_device_put(chan->indio_dev);
		chan++;
	}
	kfree(channels);
}
EXPORT_SYMBOL_GPL(iio_channel_release_all);

static int iio_channel_read(struct iio_channel *chan, int *val, int *val2,
	enum iio_chan_info_enum info)
{
	int unused;
	int vals[INDIO_MAX_RAW_ELEMENTS];
	int ret;
	int val_len = 2;

	if (val2 == NULL)
		val2 = &unused;

	if(!iio_channel_has_info(chan->channel, info))
		return -EINVAL;

	if (chan->indio_dev->info->read_raw_multi) {
		ret = chan->indio_dev->info->read_raw_multi(chan->indio_dev,
					chan->channel, INDIO_MAX_RAW_ELEMENTS,
					vals, &val_len, info);
		*val = vals[0];
		*val2 = vals[1];
	} else
		ret = chan->indio_dev->info->read_raw(chan->indio_dev,
					chan->channel, val, val2, info);

	return ret;
}

int iio_read_channel_raw(struct iio_channel *chan, int *val)
{
	int ret;

	mutex_lock(&chan->indio_dev->info_exist_lock);
	if (chan->indio_dev->info == NULL) {
		ret = -ENODEV;
		goto err_unlock;
	}

	ret = iio_channel_read(chan, val, NULL, IIO_CHAN_INFO_RAW);
err_unlock:
	mutex_unlock(&chan->indio_dev->info_exist_lock);

	return ret;
}
EXPORT_SYMBOL_GPL(iio_read_channel_raw);

int iio_read_channel_average_raw(struct iio_channel *chan, int *val)
{
	int ret;

	mutex_lock(&chan->indio_dev->info_exist_lock);
	if (chan->indio_dev->info == NULL) {
		ret = -ENODEV;
		goto err_unlock;
	}

	ret = iio_channel_read(chan, val, NULL, IIO_CHAN_INFO_AVERAGE_RAW);
err_unlock:
	mutex_unlock(&chan->indio_dev->info_exist_lock);

	return ret;
}
EXPORT_SYMBOL_GPL(iio_read_channel_average_raw);

static int iio_convert_raw_to_processed_unlocked(struct iio_channel *chan,
	int raw, int *processed, unsigned int scale)
{
	int scale_type, scale_val, scale_val2, offset;
	s64 raw64 = raw;
	int ret;

	ret = iio_channel_read(chan, &offset, NULL, IIO_CHAN_INFO_OFFSET);
	if (ret >= 0)
		raw64 += offset;

	scale_type = iio_channel_read(chan, &scale_val, &scale_val2,
					IIO_CHAN_INFO_SCALE);
	if (scale_type < 0)
		return scale_type;

	switch (scale_type) {
	case IIO_VAL_INT:
		*processed = raw64 * scale_val;
		break;
	case IIO_VAL_INT_PLUS_MICRO:
		if (scale_val2 < 0)
			*processed = -raw64 * scale_val;
		else
			*processed = raw64 * scale_val;
		*processed += div_s64(raw64 * (s64)scale_val2 * scale,
				      1000000LL);
		break;
	case IIO_VAL_INT_PLUS_NANO:
		if (scale_val2 < 0)
			*processed = -raw64 * scale_val;
		else
			*processed = raw64 * scale_val;
		*processed += div_s64(raw64 * (s64)scale_val2 * scale,
				      1000000000LL);
		break;
	case IIO_VAL_FRACTIONAL:
		*processed = div_s64(raw64 * (s64)scale_val * scale,
				     scale_val2);
		break;
	case IIO_VAL_FRACTIONAL_LOG2:
		*processed = (raw64 * (s64)scale_val * scale) >> scale_val2;
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

int iio_convert_raw_to_processed(struct iio_channel *chan, int raw,
	int *processed, unsigned int scale)
{
	int ret;

	mutex_lock(&chan->indio_dev->info_exist_lock);
	if (chan->indio_dev->info == NULL) {
		ret = -ENODEV;
		goto err_unlock;
	}

	ret = iio_convert_raw_to_processed_unlocked(chan, raw, processed,
							scale);
err_unlock:
	mutex_unlock(&chan->indio_dev->info_exist_lock);

	return ret;
}
EXPORT_SYMBOL_GPL(iio_convert_raw_to_processed);

int iio_read_channel_processed(struct iio_channel *chan, int *val)
{
	int ret;

	mutex_lock(&chan->indio_dev->info_exist_lock);
	if (chan->indio_dev->info == NULL) {
		ret = -ENODEV;
		goto err_unlock;
	}

	if (iio_channel_has_info(chan->channel, IIO_CHAN_INFO_PROCESSED)) {
		ret = iio_channel_read(chan, val, NULL,
				       IIO_CHAN_INFO_PROCESSED);
	} else {
		ret = iio_channel_read(chan, val, NULL, IIO_CHAN_INFO_RAW);
		if (ret < 0)
			goto err_unlock;
		ret = iio_convert_raw_to_processed_unlocked(chan, *val, val, 1);
	}

err_unlock:
	mutex_unlock(&chan->indio_dev->info_exist_lock);

	return ret;
}
EXPORT_SYMBOL_GPL(iio_read_channel_processed);

int iio_read_channel_scale(struct iio_channel *chan, int *val, int *val2)
{
	int ret;

	mutex_lock(&chan->indio_dev->info_exist_lock);
	if (chan->indio_dev->info == NULL) {
		ret = -ENODEV;
		goto err_unlock;
	}

	ret = iio_channel_read(chan, val, val2, IIO_CHAN_INFO_SCALE);
err_unlock:
	mutex_unlock(&chan->indio_dev->info_exist_lock);

	return ret;
}
EXPORT_SYMBOL_GPL(iio_read_channel_scale);

int iio_get_channel_type(struct iio_channel *chan, enum iio_chan_type *type)
{
	int ret = 0;
	/* Need to verify underlying driver has not gone away */

	mutex_lock(&chan->indio_dev->info_exist_lock);
	if (chan->indio_dev->info == NULL) {
		ret = -ENODEV;
		goto err_unlock;
	}

	*type = chan->channel->type;
err_unlock:
	mutex_unlock(&chan->indio_dev->info_exist_lock);

	return ret;
}
EXPORT_SYMBOL_GPL(iio_get_channel_type);

static int iio_channel_write(struct iio_channel *chan, int val, int val2,
			     enum iio_chan_info_enum info)
{
	return chan->indio_dev->info->write_raw(chan->indio_dev,
						chan->channel, val, val2, info);
}

int iio_write_channel_raw(struct iio_channel *chan, int val)
{
	int ret;

	mutex_lock(&chan->indio_dev->info_exist_lock);
	if (chan->indio_dev->info == NULL) {
		ret = -ENODEV;
		goto err_unlock;
	}

	ret = iio_channel_write(chan, val, 0, IIO_CHAN_INFO_RAW);
err_unlock:
	mutex_unlock(&chan->indio_dev->info_exist_lock);

	return ret;
}
EXPORT_SYMBOL_GPL(iio_write_channel_raw);
