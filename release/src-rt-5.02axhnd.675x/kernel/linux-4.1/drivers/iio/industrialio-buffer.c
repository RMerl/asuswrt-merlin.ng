/* The industrial I/O core
 *
 * Copyright (c) 2008 Jonathan Cameron
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published by
 * the Free Software Foundation.
 *
 * Handling of buffer allocation / resizing.
 *
 *
 * Things to look at here.
 * - Better memory allocation techniques?
 * - Alternative access techniques?
 */
#include <linux/kernel.h>
#include <linux/export.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/poll.h>
#include <linux/sched.h>

#include <linux/iio/iio.h>
#include "iio_core.h"
#include <linux/iio/sysfs.h>
#include <linux/iio/buffer.h>

static const char * const iio_endian_prefix[] = {
	[IIO_BE] = "be",
	[IIO_LE] = "le",
};

static bool iio_buffer_is_active(struct iio_buffer *buf)
{
	return !list_empty(&buf->buffer_list);
}

static size_t iio_buffer_data_available(struct iio_buffer *buf)
{
	return buf->access->data_available(buf);
}

static int iio_buffer_flush_hwfifo(struct iio_dev *indio_dev,
				   struct iio_buffer *buf, size_t required)
{
	if (!indio_dev->info->hwfifo_flush_to_buffer)
		return -ENODEV;

	return indio_dev->info->hwfifo_flush_to_buffer(indio_dev, required);
}

static bool iio_buffer_ready(struct iio_dev *indio_dev, struct iio_buffer *buf,
			     size_t to_wait, int to_flush)
{
	size_t avail;
	int flushed = 0;

	/* wakeup if the device was unregistered */
	if (!indio_dev->info)
		return true;

	/* drain the buffer if it was disabled */
	if (!iio_buffer_is_active(buf)) {
		to_wait = min_t(size_t, to_wait, 1);
		to_flush = 0;
	}

	avail = iio_buffer_data_available(buf);

	if (avail >= to_wait) {
		/* force a flush for non-blocking reads */
		if (!to_wait && !avail && to_flush)
			iio_buffer_flush_hwfifo(indio_dev, buf, to_flush);
		return true;
	}

	if (to_flush)
		flushed = iio_buffer_flush_hwfifo(indio_dev, buf,
						  to_wait - avail);
	if (flushed <= 0)
		return false;

	if (avail + flushed >= to_wait)
		return true;

	return false;
}

/**
 * iio_buffer_read_first_n_outer() - chrdev read for buffer access
 *
 * This function relies on all buffer implementations having an
 * iio_buffer as their first element.
 **/
ssize_t iio_buffer_read_first_n_outer(struct file *filp, char __user *buf,
				      size_t n, loff_t *f_ps)
{
	struct iio_dev *indio_dev = filp->private_data;
	struct iio_buffer *rb = indio_dev->buffer;
	size_t datum_size;
	size_t to_wait = 0;
	size_t to_read;
	int ret;

	if (!indio_dev->info)
		return -ENODEV;

	if (!rb || !rb->access->read_first_n)
		return -EINVAL;

	datum_size = rb->bytes_per_datum;

	/*
	 * If datum_size is 0 there will never be anything to read from the
	 * buffer, so signal end of file now.
	 */
	if (!datum_size)
		return 0;

	to_read = min_t(size_t, n / datum_size, rb->watermark);

	if (!(filp->f_flags & O_NONBLOCK))
		to_wait = to_read;

	do {
		ret = wait_event_interruptible(rb->pollq,
			iio_buffer_ready(indio_dev, rb, to_wait, to_read));
		if (ret)
			return ret;

		if (!indio_dev->info)
			return -ENODEV;

		ret = rb->access->read_first_n(rb, n, buf);
		if (ret == 0 && (filp->f_flags & O_NONBLOCK))
			ret = -EAGAIN;
	 } while (ret == 0);

	return ret;
}

/**
 * iio_buffer_poll() - poll the buffer to find out if it has data
 */
unsigned int iio_buffer_poll(struct file *filp,
			     struct poll_table_struct *wait)
{
	struct iio_dev *indio_dev = filp->private_data;
	struct iio_buffer *rb = indio_dev->buffer;

	if (!indio_dev->info || rb == NULL)
		return 0;

	poll_wait(filp, &rb->pollq, wait);
	if (iio_buffer_ready(indio_dev, rb, rb->watermark, 0))
		return POLLIN | POLLRDNORM;
	return 0;
}

/**
 * iio_buffer_wakeup_poll - Wakes up the buffer waitqueue
 * @indio_dev: The IIO device
 *
 * Wakes up the event waitqueue used for poll(). Should usually
 * be called when the device is unregistered.
 */
void iio_buffer_wakeup_poll(struct iio_dev *indio_dev)
{
	if (!indio_dev->buffer)
		return;

	wake_up(&indio_dev->buffer->pollq);
}

void iio_buffer_init(struct iio_buffer *buffer)
{
	INIT_LIST_HEAD(&buffer->demux_list);
	INIT_LIST_HEAD(&buffer->buffer_list);
	init_waitqueue_head(&buffer->pollq);
	kref_init(&buffer->ref);
	buffer->watermark = 1;
}
EXPORT_SYMBOL(iio_buffer_init);

static ssize_t iio_show_scan_index(struct device *dev,
				   struct device_attribute *attr,
				   char *buf)
{
	return sprintf(buf, "%u\n", to_iio_dev_attr(attr)->c->scan_index);
}

static ssize_t iio_show_fixed_type(struct device *dev,
				   struct device_attribute *attr,
				   char *buf)
{
	struct iio_dev_attr *this_attr = to_iio_dev_attr(attr);
	u8 type = this_attr->c->scan_type.endianness;

	if (type == IIO_CPU) {
#ifdef __LITTLE_ENDIAN
		type = IIO_LE;
#else
		type = IIO_BE;
#endif
	}
	if (this_attr->c->scan_type.repeat > 1)
		return sprintf(buf, "%s:%c%d/%dX%d>>%u\n",
		       iio_endian_prefix[type],
		       this_attr->c->scan_type.sign,
		       this_attr->c->scan_type.realbits,
		       this_attr->c->scan_type.storagebits,
		       this_attr->c->scan_type.repeat,
		       this_attr->c->scan_type.shift);
		else
			return sprintf(buf, "%s:%c%d/%d>>%u\n",
		       iio_endian_prefix[type],
		       this_attr->c->scan_type.sign,
		       this_attr->c->scan_type.realbits,
		       this_attr->c->scan_type.storagebits,
		       this_attr->c->scan_type.shift);
}

static ssize_t iio_scan_el_show(struct device *dev,
				struct device_attribute *attr,
				char *buf)
{
	int ret;
	struct iio_dev *indio_dev = dev_to_iio_dev(dev);

	/* Ensure ret is 0 or 1. */
	ret = !!test_bit(to_iio_dev_attr(attr)->address,
		       indio_dev->buffer->scan_mask);

	return sprintf(buf, "%d\n", ret);
}

/* Note NULL used as error indicator as it doesn't make sense. */
static const unsigned long *iio_scan_mask_match(const unsigned long *av_masks,
					  unsigned int masklength,
					  const unsigned long *mask)
{
	if (bitmap_empty(mask, masklength))
		return NULL;
	while (*av_masks) {
		if (bitmap_subset(mask, av_masks, masklength))
			return av_masks;
		av_masks += BITS_TO_LONGS(masklength);
	}
	return NULL;
}

static bool iio_validate_scan_mask(struct iio_dev *indio_dev,
	const unsigned long *mask)
{
	if (!indio_dev->setup_ops->validate_scan_mask)
		return true;

	return indio_dev->setup_ops->validate_scan_mask(indio_dev, mask);
}

/**
 * iio_scan_mask_set() - set particular bit in the scan mask
 * @indio_dev: the iio device
 * @buffer: the buffer whose scan mask we are interested in
 * @bit: the bit to be set.
 *
 * Note that at this point we have no way of knowing what other
 * buffers might request, hence this code only verifies that the
 * individual buffers request is plausible.
 */
static int iio_scan_mask_set(struct iio_dev *indio_dev,
		      struct iio_buffer *buffer, int bit)
{
	const unsigned long *mask;
	unsigned long *trialmask;

	trialmask = kmalloc(sizeof(*trialmask)*
			    BITS_TO_LONGS(indio_dev->masklength),
			    GFP_KERNEL);

	if (trialmask == NULL)
		return -ENOMEM;
	if (!indio_dev->masklength) {
		WARN_ON("Trying to set scanmask prior to registering buffer\n");
		goto err_invalid_mask;
	}
	bitmap_copy(trialmask, buffer->scan_mask, indio_dev->masklength);
	set_bit(bit, trialmask);

	if (!iio_validate_scan_mask(indio_dev, trialmask))
		goto err_invalid_mask;

	if (indio_dev->available_scan_masks) {
		mask = iio_scan_mask_match(indio_dev->available_scan_masks,
					   indio_dev->masklength,
					   trialmask);
		if (!mask)
			goto err_invalid_mask;
	}
	bitmap_copy(buffer->scan_mask, trialmask, indio_dev->masklength);

	kfree(trialmask);

	return 0;

err_invalid_mask:
	kfree(trialmask);
	return -EINVAL;
}

static int iio_scan_mask_clear(struct iio_buffer *buffer, int bit)
{
	clear_bit(bit, buffer->scan_mask);
	return 0;
}

static ssize_t iio_scan_el_store(struct device *dev,
				 struct device_attribute *attr,
				 const char *buf,
				 size_t len)
{
	int ret;
	bool state;
	struct iio_dev *indio_dev = dev_to_iio_dev(dev);
	struct iio_buffer *buffer = indio_dev->buffer;
	struct iio_dev_attr *this_attr = to_iio_dev_attr(attr);

	ret = strtobool(buf, &state);
	if (ret < 0)
		return ret;
	mutex_lock(&indio_dev->mlock);
	if (iio_buffer_is_active(indio_dev->buffer)) {
		ret = -EBUSY;
		goto error_ret;
	}
	ret = iio_scan_mask_query(indio_dev, buffer, this_attr->address);
	if (ret < 0)
		goto error_ret;
	if (!state && ret) {
		ret = iio_scan_mask_clear(buffer, this_attr->address);
		if (ret)
			goto error_ret;
	} else if (state && !ret) {
		ret = iio_scan_mask_set(indio_dev, buffer, this_attr->address);
		if (ret)
			goto error_ret;
	}

error_ret:
	mutex_unlock(&indio_dev->mlock);

	return ret < 0 ? ret : len;

}

static ssize_t iio_scan_el_ts_show(struct device *dev,
				   struct device_attribute *attr,
				   char *buf)
{
	struct iio_dev *indio_dev = dev_to_iio_dev(dev);
	return sprintf(buf, "%d\n", indio_dev->buffer->scan_timestamp);
}

static ssize_t iio_scan_el_ts_store(struct device *dev,
				    struct device_attribute *attr,
				    const char *buf,
				    size_t len)
{
	int ret;
	struct iio_dev *indio_dev = dev_to_iio_dev(dev);
	bool state;

	ret = strtobool(buf, &state);
	if (ret < 0)
		return ret;

	mutex_lock(&indio_dev->mlock);
	if (iio_buffer_is_active(indio_dev->buffer)) {
		ret = -EBUSY;
		goto error_ret;
	}
	indio_dev->buffer->scan_timestamp = state;
error_ret:
	mutex_unlock(&indio_dev->mlock);

	return ret ? ret : len;
}

static int iio_buffer_add_channel_sysfs(struct iio_dev *indio_dev,
					const struct iio_chan_spec *chan)
{
	int ret, attrcount = 0;
	struct iio_buffer *buffer = indio_dev->buffer;

	ret = __iio_add_chan_devattr("index",
				     chan,
				     &iio_show_scan_index,
				     NULL,
				     0,
				     IIO_SEPARATE,
				     &indio_dev->dev,
				     &buffer->scan_el_dev_attr_list);
	if (ret)
		return ret;
	attrcount++;
	ret = __iio_add_chan_devattr("type",
				     chan,
				     &iio_show_fixed_type,
				     NULL,
				     0,
				     0,
				     &indio_dev->dev,
				     &buffer->scan_el_dev_attr_list);
	if (ret)
		return ret;
	attrcount++;
	if (chan->type != IIO_TIMESTAMP)
		ret = __iio_add_chan_devattr("en",
					     chan,
					     &iio_scan_el_show,
					     &iio_scan_el_store,
					     chan->scan_index,
					     0,
					     &indio_dev->dev,
					     &buffer->scan_el_dev_attr_list);
	else
		ret = __iio_add_chan_devattr("en",
					     chan,
					     &iio_scan_el_ts_show,
					     &iio_scan_el_ts_store,
					     chan->scan_index,
					     0,
					     &indio_dev->dev,
					     &buffer->scan_el_dev_attr_list);
	if (ret)
		return ret;
	attrcount++;
	ret = attrcount;
	return ret;
}

static ssize_t iio_buffer_read_length(struct device *dev,
				      struct device_attribute *attr,
				      char *buf)
{
	struct iio_dev *indio_dev = dev_to_iio_dev(dev);
	struct iio_buffer *buffer = indio_dev->buffer;

	return sprintf(buf, "%d\n", buffer->length);
}

static ssize_t iio_buffer_write_length(struct device *dev,
				       struct device_attribute *attr,
				       const char *buf, size_t len)
{
	struct iio_dev *indio_dev = dev_to_iio_dev(dev);
	struct iio_buffer *buffer = indio_dev->buffer;
	unsigned int val;
	int ret;

	ret = kstrtouint(buf, 10, &val);
	if (ret)
		return ret;

	if (val == buffer->length)
		return len;

	mutex_lock(&indio_dev->mlock);
	if (iio_buffer_is_active(indio_dev->buffer)) {
		ret = -EBUSY;
	} else {
		buffer->access->set_length(buffer, val);
		ret = 0;
	}
	if (ret)
		goto out;
	if (buffer->length && buffer->length < buffer->watermark)
		buffer->watermark = buffer->length;
out:
	mutex_unlock(&indio_dev->mlock);

	return ret ? ret : len;
}

static ssize_t iio_buffer_show_enable(struct device *dev,
				      struct device_attribute *attr,
				      char *buf)
{
	struct iio_dev *indio_dev = dev_to_iio_dev(dev);
	return sprintf(buf, "%d\n", iio_buffer_is_active(indio_dev->buffer));
}

static int iio_compute_scan_bytes(struct iio_dev *indio_dev,
				const unsigned long *mask, bool timestamp)
{
	const struct iio_chan_spec *ch;
	unsigned bytes = 0;
	int length, i;

	/* How much space will the demuxed element take? */
	for_each_set_bit(i, mask,
			 indio_dev->masklength) {
		ch = iio_find_channel_from_si(indio_dev, i);
		if (ch->scan_type.repeat > 1)
			length = ch->scan_type.storagebits / 8 *
				ch->scan_type.repeat;
		else
			length = ch->scan_type.storagebits / 8;
		bytes = ALIGN(bytes, length);
		bytes += length;
	}
	if (timestamp) {
		ch = iio_find_channel_from_si(indio_dev,
					      indio_dev->scan_index_timestamp);
		if (ch->scan_type.repeat > 1)
			length = ch->scan_type.storagebits / 8 *
				ch->scan_type.repeat;
		else
			length = ch->scan_type.storagebits / 8;
		bytes = ALIGN(bytes, length);
		bytes += length;
	}
	return bytes;
}

static void iio_buffer_activate(struct iio_dev *indio_dev,
	struct iio_buffer *buffer)
{
	iio_buffer_get(buffer);
	list_add(&buffer->buffer_list, &indio_dev->buffer_list);
}

static void iio_buffer_deactivate(struct iio_buffer *buffer)
{
	list_del_init(&buffer->buffer_list);
	wake_up_interruptible(&buffer->pollq);
	iio_buffer_put(buffer);
}

void iio_disable_all_buffers(struct iio_dev *indio_dev)
{
	struct iio_buffer *buffer, *_buffer;

	if (list_empty(&indio_dev->buffer_list))
		return;

	if (indio_dev->setup_ops->predisable)
		indio_dev->setup_ops->predisable(indio_dev);

	list_for_each_entry_safe(buffer, _buffer,
			&indio_dev->buffer_list, buffer_list)
		iio_buffer_deactivate(buffer);

	indio_dev->currentmode = INDIO_DIRECT_MODE;
	if (indio_dev->setup_ops->postdisable)
		indio_dev->setup_ops->postdisable(indio_dev);

	if (indio_dev->available_scan_masks == NULL)
		kfree(indio_dev->active_scan_mask);
}

static void iio_buffer_update_bytes_per_datum(struct iio_dev *indio_dev,
	struct iio_buffer *buffer)
{
	unsigned int bytes;

	if (!buffer->access->set_bytes_per_datum)
		return;

	bytes = iio_compute_scan_bytes(indio_dev, buffer->scan_mask,
		buffer->scan_timestamp);

	buffer->access->set_bytes_per_datum(buffer, bytes);
}

static int __iio_update_buffers(struct iio_dev *indio_dev,
		       struct iio_buffer *insert_buffer,
		       struct iio_buffer *remove_buffer)
{
	int ret;
	int success = 0;
	struct iio_buffer *buffer;
	unsigned long *compound_mask;
	const unsigned long *old_mask;

	/* Wind down existing buffers - iff there are any */
	if (!list_empty(&indio_dev->buffer_list)) {
		if (indio_dev->setup_ops->predisable) {
			ret = indio_dev->setup_ops->predisable(indio_dev);
			if (ret)
				return ret;
		}
		indio_dev->currentmode = INDIO_DIRECT_MODE;
		if (indio_dev->setup_ops->postdisable) {
			ret = indio_dev->setup_ops->postdisable(indio_dev);
			if (ret)
				return ret;
		}
	}
	/* Keep a copy of current setup to allow roll back */
	old_mask = indio_dev->active_scan_mask;
	if (!indio_dev->available_scan_masks)
		indio_dev->active_scan_mask = NULL;

	if (remove_buffer)
		iio_buffer_deactivate(remove_buffer);
	if (insert_buffer)
		iio_buffer_activate(indio_dev, insert_buffer);

	/* If no buffers in list, we are done */
	if (list_empty(&indio_dev->buffer_list)) {
		indio_dev->currentmode = INDIO_DIRECT_MODE;
		if (indio_dev->available_scan_masks == NULL)
			kfree(old_mask);
		return 0;
	}

	/* What scan mask do we actually have? */
	compound_mask = kcalloc(BITS_TO_LONGS(indio_dev->masklength),
				sizeof(long), GFP_KERNEL);
	if (compound_mask == NULL) {
		if (indio_dev->available_scan_masks == NULL)
			kfree(old_mask);
		return -ENOMEM;
	}
	indio_dev->scan_timestamp = 0;

	list_for_each_entry(buffer, &indio_dev->buffer_list, buffer_list) {
		bitmap_or(compound_mask, compound_mask, buffer->scan_mask,
			  indio_dev->masklength);
		indio_dev->scan_timestamp |= buffer->scan_timestamp;
	}
	if (indio_dev->available_scan_masks) {
		indio_dev->active_scan_mask =
			iio_scan_mask_match(indio_dev->available_scan_masks,
					    indio_dev->masklength,
					    compound_mask);
		if (indio_dev->active_scan_mask == NULL) {
			/*
			 * Roll back.
			 * Note can only occur when adding a buffer.
			 */
			iio_buffer_deactivate(insert_buffer);
			if (old_mask) {
				indio_dev->active_scan_mask = old_mask;
				success = -EINVAL;
			}
			else {
				kfree(compound_mask);
				ret = -EINVAL;
				return ret;
			}
		}
	} else {
		indio_dev->active_scan_mask = compound_mask;
	}

	iio_update_demux(indio_dev);

	/* Wind up again */
	if (indio_dev->setup_ops->preenable) {
		ret = indio_dev->setup_ops->preenable(indio_dev);
		if (ret) {
			printk(KERN_ERR
			       "Buffer not started: buffer preenable failed (%d)\n", ret);
			goto error_remove_inserted;
		}
	}
	indio_dev->scan_bytes =
		iio_compute_scan_bytes(indio_dev,
				       indio_dev->active_scan_mask,
				       indio_dev->scan_timestamp);
	list_for_each_entry(buffer, &indio_dev->buffer_list, buffer_list) {
		iio_buffer_update_bytes_per_datum(indio_dev, buffer);
		if (buffer->access->request_update) {
			ret = buffer->access->request_update(buffer);
			if (ret) {
				printk(KERN_INFO
				       "Buffer not started: buffer parameter update failed (%d)\n", ret);
				goto error_run_postdisable;
			}
		}
	}
	if (indio_dev->info->update_scan_mode) {
		ret = indio_dev->info
			->update_scan_mode(indio_dev,
					   indio_dev->active_scan_mask);
		if (ret < 0) {
			printk(KERN_INFO "Buffer not started: update scan mode failed (%d)\n", ret);
			goto error_run_postdisable;
		}
	}
	/* Definitely possible for devices to support both of these. */
	if ((indio_dev->modes & INDIO_BUFFER_TRIGGERED) && indio_dev->trig) {
		indio_dev->currentmode = INDIO_BUFFER_TRIGGERED;
	} else if (indio_dev->modes & INDIO_BUFFER_HARDWARE) {
		indio_dev->currentmode = INDIO_BUFFER_HARDWARE;
	} else if (indio_dev->modes & INDIO_BUFFER_SOFTWARE) {
		indio_dev->currentmode = INDIO_BUFFER_SOFTWARE;
	} else { /* Should never be reached */
		/* Can only occur on first buffer */
		if (indio_dev->modes & INDIO_BUFFER_TRIGGERED)
			pr_info("Buffer not started: no trigger\n");
		ret = -EINVAL;
		goto error_run_postdisable;
	}

	if (indio_dev->setup_ops->postenable) {
		ret = indio_dev->setup_ops->postenable(indio_dev);
		if (ret) {
			printk(KERN_INFO
			       "Buffer not started: postenable failed (%d)\n", ret);
			indio_dev->currentmode = INDIO_DIRECT_MODE;
			if (indio_dev->setup_ops->postdisable)
				indio_dev->setup_ops->postdisable(indio_dev);
			goto error_disable_all_buffers;
		}
	}

	if (indio_dev->available_scan_masks)
		kfree(compound_mask);
	else
		kfree(old_mask);

	return success;

error_disable_all_buffers:
	indio_dev->currentmode = INDIO_DIRECT_MODE;
error_run_postdisable:
	if (indio_dev->setup_ops->postdisable)
		indio_dev->setup_ops->postdisable(indio_dev);
error_remove_inserted:
	if (insert_buffer)
		iio_buffer_deactivate(insert_buffer);
	indio_dev->active_scan_mask = old_mask;
	kfree(compound_mask);
	return ret;
}

int iio_update_buffers(struct iio_dev *indio_dev,
		       struct iio_buffer *insert_buffer,
		       struct iio_buffer *remove_buffer)
{
	int ret;

	if (insert_buffer == remove_buffer)
		return 0;

	mutex_lock(&indio_dev->info_exist_lock);
	mutex_lock(&indio_dev->mlock);

	if (insert_buffer && iio_buffer_is_active(insert_buffer))
		insert_buffer = NULL;

	if (remove_buffer && !iio_buffer_is_active(remove_buffer))
		remove_buffer = NULL;

	if (!insert_buffer && !remove_buffer) {
		ret = 0;
		goto out_unlock;
	}

	if (indio_dev->info == NULL) {
		ret = -ENODEV;
		goto out_unlock;
	}

	ret = __iio_update_buffers(indio_dev, insert_buffer, remove_buffer);

out_unlock:
	mutex_unlock(&indio_dev->mlock);
	mutex_unlock(&indio_dev->info_exist_lock);

	return ret;
}
EXPORT_SYMBOL_GPL(iio_update_buffers);

static ssize_t iio_buffer_store_enable(struct device *dev,
				       struct device_attribute *attr,
				       const char *buf,
				       size_t len)
{
	int ret;
	bool requested_state;
	struct iio_dev *indio_dev = dev_to_iio_dev(dev);
	bool inlist;

	ret = strtobool(buf, &requested_state);
	if (ret < 0)
		return ret;

	mutex_lock(&indio_dev->mlock);

	/* Find out if it is in the list */
	inlist = iio_buffer_is_active(indio_dev->buffer);
	/* Already in desired state */
	if (inlist == requested_state)
		goto done;

	if (requested_state)
		ret = __iio_update_buffers(indio_dev,
					 indio_dev->buffer, NULL);
	else
		ret = __iio_update_buffers(indio_dev,
					 NULL, indio_dev->buffer);

	if (ret < 0)
		goto done;
done:
	mutex_unlock(&indio_dev->mlock);
	return (ret < 0) ? ret : len;
}

static const char * const iio_scan_elements_group_name = "scan_elements";

static ssize_t iio_buffer_show_watermark(struct device *dev,
					 struct device_attribute *attr,
					 char *buf)
{
	struct iio_dev *indio_dev = dev_to_iio_dev(dev);
	struct iio_buffer *buffer = indio_dev->buffer;

	return sprintf(buf, "%u\n", buffer->watermark);
}

static ssize_t iio_buffer_store_watermark(struct device *dev,
					  struct device_attribute *attr,
					  const char *buf,
					  size_t len)
{
	struct iio_dev *indio_dev = dev_to_iio_dev(dev);
	struct iio_buffer *buffer = indio_dev->buffer;
	unsigned int val;
	int ret;

	ret = kstrtouint(buf, 10, &val);
	if (ret)
		return ret;
	if (!val)
		return -EINVAL;

	mutex_lock(&indio_dev->mlock);

	if (val > buffer->length) {
		ret = -EINVAL;
		goto out;
	}

	if (iio_buffer_is_active(indio_dev->buffer)) {
		ret = -EBUSY;
		goto out;
	}

	buffer->watermark = val;

	if (indio_dev->info->hwfifo_set_watermark)
		indio_dev->info->hwfifo_set_watermark(indio_dev, val);
out:
	mutex_unlock(&indio_dev->mlock);

	return ret ? ret : len;
}

static DEVICE_ATTR(length, S_IRUGO | S_IWUSR, iio_buffer_read_length,
		   iio_buffer_write_length);
static struct device_attribute dev_attr_length_ro = __ATTR(length,
	S_IRUGO, iio_buffer_read_length, NULL);
static DEVICE_ATTR(enable, S_IRUGO | S_IWUSR,
		   iio_buffer_show_enable, iio_buffer_store_enable);
static DEVICE_ATTR(watermark, S_IRUGO | S_IWUSR,
		   iio_buffer_show_watermark, iio_buffer_store_watermark);

static struct attribute *iio_buffer_attrs[] = {
	&dev_attr_length.attr,
	&dev_attr_enable.attr,
	&dev_attr_watermark.attr,
};

int iio_buffer_alloc_sysfs_and_mask(struct iio_dev *indio_dev)
{
	struct iio_dev_attr *p;
	struct attribute **attr;
	struct iio_buffer *buffer = indio_dev->buffer;
	int ret, i, attrn, attrcount, attrcount_orig = 0;
	const struct iio_chan_spec *channels;

	if (!buffer)
		return 0;

	attrcount = 0;
	if (buffer->attrs) {
		while (buffer->attrs[attrcount] != NULL)
			attrcount++;
	}

	attr = kcalloc(attrcount + ARRAY_SIZE(iio_buffer_attrs) + 1,
		       sizeof(struct attribute *), GFP_KERNEL);
	if (!attr)
		return -ENOMEM;

	memcpy(attr, iio_buffer_attrs, sizeof(iio_buffer_attrs));
	if (!buffer->access->set_length)
		attr[0] = &dev_attr_length_ro.attr;

	if (buffer->attrs)
		memcpy(&attr[ARRAY_SIZE(iio_buffer_attrs)], buffer->attrs,
		       sizeof(struct attribute *) * attrcount);

	attr[attrcount + ARRAY_SIZE(iio_buffer_attrs)] = NULL;

	buffer->buffer_group.name = "buffer";
	buffer->buffer_group.attrs = attr;

	indio_dev->groups[indio_dev->groupcounter++] = &buffer->buffer_group;

	if (buffer->scan_el_attrs != NULL) {
		attr = buffer->scan_el_attrs->attrs;
		while (*attr++ != NULL)
			attrcount_orig++;
	}
	attrcount = attrcount_orig;
	INIT_LIST_HEAD(&buffer->scan_el_dev_attr_list);
	channels = indio_dev->channels;
	if (channels) {
		/* new magic */
		for (i = 0; i < indio_dev->num_channels; i++) {
			if (channels[i].scan_index < 0)
				continue;

			/* Establish necessary mask length */
			if (channels[i].scan_index >
			    (int)indio_dev->masklength - 1)
				indio_dev->masklength
					= channels[i].scan_index + 1;

			ret = iio_buffer_add_channel_sysfs(indio_dev,
							 &channels[i]);
			if (ret < 0)
				goto error_cleanup_dynamic;
			attrcount += ret;
			if (channels[i].type == IIO_TIMESTAMP)
				indio_dev->scan_index_timestamp =
					channels[i].scan_index;
		}
		if (indio_dev->masklength && buffer->scan_mask == NULL) {
			buffer->scan_mask = kcalloc(BITS_TO_LONGS(indio_dev->masklength),
						    sizeof(*buffer->scan_mask),
						    GFP_KERNEL);
			if (buffer->scan_mask == NULL) {
				ret = -ENOMEM;
				goto error_cleanup_dynamic;
			}
		}
	}

	buffer->scan_el_group.name = iio_scan_elements_group_name;

	buffer->scan_el_group.attrs = kcalloc(attrcount + 1,
					      sizeof(buffer->scan_el_group.attrs[0]),
					      GFP_KERNEL);
	if (buffer->scan_el_group.attrs == NULL) {
		ret = -ENOMEM;
		goto error_free_scan_mask;
	}
	if (buffer->scan_el_attrs)
		memcpy(buffer->scan_el_group.attrs, buffer->scan_el_attrs,
		       sizeof(buffer->scan_el_group.attrs[0])*attrcount_orig);
	attrn = attrcount_orig;

	list_for_each_entry(p, &buffer->scan_el_dev_attr_list, l)
		buffer->scan_el_group.attrs[attrn++] = &p->dev_attr.attr;
	indio_dev->groups[indio_dev->groupcounter++] = &buffer->scan_el_group;

	return 0;

error_free_scan_mask:
	kfree(buffer->scan_mask);
error_cleanup_dynamic:
	iio_free_chan_devattr_list(&buffer->scan_el_dev_attr_list);
	kfree(indio_dev->buffer->buffer_group.attrs);

	return ret;
}

void iio_buffer_free_sysfs_and_mask(struct iio_dev *indio_dev)
{
	if (!indio_dev->buffer)
		return;

	kfree(indio_dev->buffer->scan_mask);
	kfree(indio_dev->buffer->buffer_group.attrs);
	kfree(indio_dev->buffer->scan_el_group.attrs);
	iio_free_chan_devattr_list(&indio_dev->buffer->scan_el_dev_attr_list);
}

/**
 * iio_validate_scan_mask_onehot() - Validates that exactly one channel is selected
 * @indio_dev: the iio device
 * @mask: scan mask to be checked
 *
 * Return true if exactly one bit is set in the scan mask, false otherwise. It
 * can be used for devices where only one channel can be active for sampling at
 * a time.
 */
bool iio_validate_scan_mask_onehot(struct iio_dev *indio_dev,
	const unsigned long *mask)
{
	return bitmap_weight(mask, indio_dev->masklength) == 1;
}
EXPORT_SYMBOL_GPL(iio_validate_scan_mask_onehot);

int iio_scan_mask_query(struct iio_dev *indio_dev,
			struct iio_buffer *buffer, int bit)
{
	if (bit > indio_dev->masklength)
		return -EINVAL;

	if (!buffer->scan_mask)
		return 0;

	/* Ensure return value is 0 or 1. */
	return !!test_bit(bit, buffer->scan_mask);
};
EXPORT_SYMBOL_GPL(iio_scan_mask_query);

/**
 * struct iio_demux_table() - table describing demux memcpy ops
 * @from:	index to copy from
 * @to:		index to copy to
 * @length:	how many bytes to copy
 * @l:		list head used for management
 */
struct iio_demux_table {
	unsigned from;
	unsigned to;
	unsigned length;
	struct list_head l;
};

static const void *iio_demux(struct iio_buffer *buffer,
				 const void *datain)
{
	struct iio_demux_table *t;

	if (list_empty(&buffer->demux_list))
		return datain;
	list_for_each_entry(t, &buffer->demux_list, l)
		memcpy(buffer->demux_bounce + t->to,
		       datain + t->from, t->length);

	return buffer->demux_bounce;
}

static int iio_push_to_buffer(struct iio_buffer *buffer, const void *data)
{
	const void *dataout = iio_demux(buffer, data);
	int ret;

	ret = buffer->access->store_to(buffer, dataout);
	if (ret)
		return ret;

	/*
	 * We can't just test for watermark to decide if we wake the poll queue
	 * because read may request less samples than the watermark.
	 */
	wake_up_interruptible_poll(&buffer->pollq, POLLIN | POLLRDNORM);
	return 0;
}

static void iio_buffer_demux_free(struct iio_buffer *buffer)
{
	struct iio_demux_table *p, *q;
	list_for_each_entry_safe(p, q, &buffer->demux_list, l) {
		list_del(&p->l);
		kfree(p);
	}
}


int iio_push_to_buffers(struct iio_dev *indio_dev, const void *data)
{
	int ret;
	struct iio_buffer *buf;

	list_for_each_entry(buf, &indio_dev->buffer_list, buffer_list) {
		ret = iio_push_to_buffer(buf, data);
		if (ret < 0)
			return ret;
	}

	return 0;
}
EXPORT_SYMBOL_GPL(iio_push_to_buffers);

static int iio_buffer_add_demux(struct iio_buffer *buffer,
	struct iio_demux_table **p, unsigned int in_loc, unsigned int out_loc,
	unsigned int length)
{

	if (*p && (*p)->from + (*p)->length == in_loc &&
		(*p)->to + (*p)->length == out_loc) {
		(*p)->length += length;
	} else {
		*p = kmalloc(sizeof(**p), GFP_KERNEL);
		if (*p == NULL)
			return -ENOMEM;
		(*p)->from = in_loc;
		(*p)->to = out_loc;
		(*p)->length = length;
		list_add_tail(&(*p)->l, &buffer->demux_list);
	}

	return 0;
}

static int iio_buffer_update_demux(struct iio_dev *indio_dev,
				   struct iio_buffer *buffer)
{
	const struct iio_chan_spec *ch;
	int ret, in_ind = -1, out_ind, length;
	unsigned in_loc = 0, out_loc = 0;
	struct iio_demux_table *p = NULL;

	/* Clear out any old demux */
	iio_buffer_demux_free(buffer);
	kfree(buffer->demux_bounce);
	buffer->demux_bounce = NULL;

	/* First work out which scan mode we will actually have */
	if (bitmap_equal(indio_dev->active_scan_mask,
			 buffer->scan_mask,
			 indio_dev->masklength))
		return 0;

	/* Now we have the two masks, work from least sig and build up sizes */
	for_each_set_bit(out_ind,
			 buffer->scan_mask,
			 indio_dev->masklength) {
		in_ind = find_next_bit(indio_dev->active_scan_mask,
				       indio_dev->masklength,
				       in_ind + 1);
		while (in_ind != out_ind) {
			in_ind = find_next_bit(indio_dev->active_scan_mask,
					       indio_dev->masklength,
					       in_ind + 1);
			ch = iio_find_channel_from_si(indio_dev, in_ind);
			if (ch->scan_type.repeat > 1)
				length = ch->scan_type.storagebits / 8 *
					ch->scan_type.repeat;
			else
				length = ch->scan_type.storagebits / 8;
			/* Make sure we are aligned */
			in_loc = roundup(in_loc, length) + length;
		}
		ch = iio_find_channel_from_si(indio_dev, in_ind);
		if (ch->scan_type.repeat > 1)
			length = ch->scan_type.storagebits / 8 *
				ch->scan_type.repeat;
		else
			length = ch->scan_type.storagebits / 8;
		out_loc = roundup(out_loc, length);
		in_loc = roundup(in_loc, length);
		ret = iio_buffer_add_demux(buffer, &p, in_loc, out_loc, length);
		if (ret)
			goto error_clear_mux_table;
		out_loc += length;
		in_loc += length;
	}
	/* Relies on scan_timestamp being last */
	if (buffer->scan_timestamp) {
		ch = iio_find_channel_from_si(indio_dev,
			indio_dev->scan_index_timestamp);
		if (ch->scan_type.repeat > 1)
			length = ch->scan_type.storagebits / 8 *
				ch->scan_type.repeat;
		else
			length = ch->scan_type.storagebits / 8;
		out_loc = roundup(out_loc, length);
		in_loc = roundup(in_loc, length);
		ret = iio_buffer_add_demux(buffer, &p, in_loc, out_loc, length);
		if (ret)
			goto error_clear_mux_table;
		out_loc += length;
		in_loc += length;
	}
	buffer->demux_bounce = kzalloc(out_loc, GFP_KERNEL);
	if (buffer->demux_bounce == NULL) {
		ret = -ENOMEM;
		goto error_clear_mux_table;
	}
	return 0;

error_clear_mux_table:
	iio_buffer_demux_free(buffer);

	return ret;
}

int iio_update_demux(struct iio_dev *indio_dev)
{
	struct iio_buffer *buffer;
	int ret;

	list_for_each_entry(buffer, &indio_dev->buffer_list, buffer_list) {
		ret = iio_buffer_update_demux(indio_dev, buffer);
		if (ret < 0)
			goto error_clear_mux_table;
	}
	return 0;

error_clear_mux_table:
	list_for_each_entry(buffer, &indio_dev->buffer_list, buffer_list)
		iio_buffer_demux_free(buffer);

	return ret;
}
EXPORT_SYMBOL_GPL(iio_update_demux);

/**
 * iio_buffer_release() - Free a buffer's resources
 * @ref: Pointer to the kref embedded in the iio_buffer struct
 *
 * This function is called when the last reference to the buffer has been
 * dropped. It will typically free all resources allocated by the buffer. Do not
 * call this function manually, always use iio_buffer_put() when done using a
 * buffer.
 */
static void iio_buffer_release(struct kref *ref)
{
	struct iio_buffer *buffer = container_of(ref, struct iio_buffer, ref);

	buffer->access->release(buffer);
}

/**
 * iio_buffer_get() - Grab a reference to the buffer
 * @buffer: The buffer to grab a reference for, may be NULL
 *
 * Returns the pointer to the buffer that was passed into the function.
 */
struct iio_buffer *iio_buffer_get(struct iio_buffer *buffer)
{
	if (buffer)
		kref_get(&buffer->ref);

	return buffer;
}
EXPORT_SYMBOL_GPL(iio_buffer_get);

/**
 * iio_buffer_put() - Release the reference to the buffer
 * @buffer: The buffer to release the reference for, may be NULL
 */
void iio_buffer_put(struct iio_buffer *buffer)
{
	if (buffer)
		kref_put(&buffer->ref, iio_buffer_release);
}
EXPORT_SYMBOL_GPL(iio_buffer_put);
