/*
 *
 * Intel Management Engine Interface (Intel MEI) Linux driver
 * Copyright (c) 2003-2012, Intel Corporation.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 */
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/fcntl.h>
#include <linux/poll.h>
#include <linux/init.h>
#include <linux/ioctl.h>
#include <linux/cdev.h>
#include <linux/sched.h>
#include <linux/uuid.h>
#include <linux/compat.h>
#include <linux/jiffies.h>
#include <linux/interrupt.h>

#include <linux/mei.h>

#include "mei_dev.h"
#include "client.h"

/**
 * mei_open - the open function
 *
 * @inode: pointer to inode structure
 * @file: pointer to file structure
 *
 * Return: 0 on success, <0 on error
 */
static int mei_open(struct inode *inode, struct file *file)
{
	struct mei_device *dev;
	struct mei_cl *cl;

	int err;

	dev = container_of(inode->i_cdev, struct mei_device, cdev);
	if (!dev)
		return -ENODEV;

	mutex_lock(&dev->device_lock);

	if (dev->dev_state != MEI_DEV_ENABLED) {
		dev_dbg(dev->dev, "dev_state != MEI_ENABLED  dev_state = %s\n",
		    mei_dev_state_str(dev->dev_state));
		err = -ENODEV;
		goto err_unlock;
	}

	cl = mei_cl_alloc_linked(dev, MEI_HOST_CLIENT_ID_ANY);
	if (IS_ERR(cl)) {
		err = PTR_ERR(cl);
		goto err_unlock;
	}

	file->private_data = cl;

	mutex_unlock(&dev->device_lock);

	return nonseekable_open(inode, file);

err_unlock:
	mutex_unlock(&dev->device_lock);
	return err;
}

/**
 * mei_release - the release function
 *
 * @inode: pointer to inode structure
 * @file: pointer to file structure
 *
 * Return: 0 on success, <0 on error
 */
static int mei_release(struct inode *inode, struct file *file)
{
	struct mei_cl *cl = file->private_data;
	struct mei_device *dev;
	int rets = 0;

	if (WARN_ON(!cl || !cl->dev))
		return -ENODEV;

	dev = cl->dev;

	mutex_lock(&dev->device_lock);
	if (cl == &dev->iamthif_cl) {
		rets = mei_amthif_release(dev, file);
		goto out;
	}
	if (mei_cl_is_connected(cl)) {
		cl->state = MEI_FILE_DISCONNECTING;
		cl_dbg(dev, cl, "disconnecting\n");
		rets = mei_cl_disconnect(cl);
	}
	mei_cl_flush_queues(cl, file);
	cl_dbg(dev, cl, "removing\n");

	mei_cl_unlink(cl);

	file->private_data = NULL;

	kfree(cl);
out:
	mutex_unlock(&dev->device_lock);
	return rets;
}


/**
 * mei_read - the read function.
 *
 * @file: pointer to file structure
 * @ubuf: pointer to user buffer
 * @length: buffer length
 * @offset: data offset in buffer
 *
 * Return: >=0 data length on success , <0 on error
 */
static ssize_t mei_read(struct file *file, char __user *ubuf,
			size_t length, loff_t *offset)
{
	struct mei_cl *cl = file->private_data;
	struct mei_device *dev;
	struct mei_cl_cb *cb = NULL;
	int rets;
	int err;


	if (WARN_ON(!cl || !cl->dev))
		return -ENODEV;

	dev = cl->dev;


	mutex_lock(&dev->device_lock);
	if (dev->dev_state != MEI_DEV_ENABLED) {
		rets = -ENODEV;
		goto out;
	}

	if (length == 0) {
		rets = 0;
		goto out;
	}

	if (cl == &dev->iamthif_cl) {
		rets = mei_amthif_read(dev, file, ubuf, length, offset);
		goto out;
	}

	cb = mei_cl_read_cb(cl, file);
	if (cb) {
		/* read what left */
		if (cb->buf_idx > *offset)
			goto copy_buffer;
		/* offset is beyond buf_idx we have no more data return 0 */
		if (cb->buf_idx > 0 && cb->buf_idx <= *offset) {
			rets = 0;
			goto free;
		}
		/* Offset needs to be cleaned for contiguous reads*/
		if (cb->buf_idx == 0 && *offset > 0)
			*offset = 0;
	} else if (*offset > 0) {
		*offset = 0;
	}

	err = mei_cl_read_start(cl, length, file);
	if (err && err != -EBUSY) {
		dev_dbg(dev->dev,
			"mei start read failure with status = %d\n", err);
		rets = err;
		goto out;
	}

	if (list_empty(&cl->rd_completed) && !waitqueue_active(&cl->rx_wait)) {
		if (file->f_flags & O_NONBLOCK) {
			rets = -EAGAIN;
			goto out;
		}

		mutex_unlock(&dev->device_lock);

		if (wait_event_interruptible(cl->rx_wait,
				(!list_empty(&cl->rd_completed)) ||
				(!mei_cl_is_connected(cl)))) {

			if (signal_pending(current))
				return -EINTR;
			return -ERESTARTSYS;
		}

		mutex_lock(&dev->device_lock);
		if (!mei_cl_is_connected(cl)) {
			rets = -EBUSY;
			goto out;
		}
	}

	cb = mei_cl_read_cb(cl, file);
	if (!cb) {
		rets = 0;
		goto out;
	}

copy_buffer:
	/* now copy the data to user space */
	if (cb->status) {
		rets = cb->status;
		dev_dbg(dev->dev, "read operation failed %d\n", rets);
		goto free;
	}

	dev_dbg(dev->dev, "buf.size = %d buf.idx= %ld\n",
	    cb->buf.size, cb->buf_idx);
	if (length == 0 || ubuf == NULL || *offset > cb->buf_idx) {
		rets = -EMSGSIZE;
		goto free;
	}

	/* length is being truncated to PAGE_SIZE,
	 * however buf_idx may point beyond that */
	length = min_t(size_t, length, cb->buf_idx - *offset);

	if (copy_to_user(ubuf, cb->buf.data + *offset, length)) {
		dev_dbg(dev->dev, "failed to copy data to userland\n");
		rets = -EFAULT;
		goto free;
	}

	rets = length;
	*offset += length;
	if ((unsigned long)*offset < cb->buf_idx)
		goto out;

free:
	mei_io_cb_free(cb);

out:
	dev_dbg(dev->dev, "end mei read rets= %d\n", rets);
	mutex_unlock(&dev->device_lock);
	return rets;
}
/**
 * mei_write - the write function.
 *
 * @file: pointer to file structure
 * @ubuf: pointer to user buffer
 * @length: buffer length
 * @offset: data offset in buffer
 *
 * Return: >=0 data length on success , <0 on error
 */
static ssize_t mei_write(struct file *file, const char __user *ubuf,
			 size_t length, loff_t *offset)
{
	struct mei_cl *cl = file->private_data;
	struct mei_me_client *me_cl = NULL;
	struct mei_cl_cb *write_cb = NULL;
	struct mei_device *dev;
	unsigned long timeout = 0;
	int rets;

	if (WARN_ON(!cl || !cl->dev))
		return -ENODEV;

	dev = cl->dev;

	mutex_lock(&dev->device_lock);

	if (dev->dev_state != MEI_DEV_ENABLED) {
		rets = -ENODEV;
		goto out;
	}

	me_cl = mei_me_cl_by_uuid_id(dev, &cl->cl_uuid, cl->me_client_id);
	if (!me_cl) {
		rets = -ENOTTY;
		goto out;
	}

	if (length == 0) {
		rets = 0;
		goto out;
	}

	if (length > me_cl->props.max_msg_length) {
		rets = -EFBIG;
		goto out;
	}

	if (!mei_cl_is_connected(cl)) {
		cl_err(dev, cl, "is not connected");
		rets = -ENODEV;
		goto out;
	}
	if (cl == &dev->iamthif_cl) {
		write_cb = mei_amthif_find_read_list_entry(dev, file);

		if (write_cb) {
			timeout = write_cb->read_time +
				mei_secs_to_jiffies(MEI_IAMTHIF_READ_TIMER);

			if (time_after(jiffies, timeout)) {
				*offset = 0;
				mei_io_cb_free(write_cb);
				write_cb = NULL;
			}
		}
	}

	*offset = 0;
	write_cb = mei_cl_alloc_cb(cl, length, MEI_FOP_WRITE, file);
	if (!write_cb) {
		rets = -ENOMEM;
		goto out;
	}

	rets = copy_from_user(write_cb->buf.data, ubuf, length);
	if (rets) {
		dev_dbg(dev->dev, "failed to copy data from userland\n");
		rets = -EFAULT;
		goto out;
	}

	if (cl == &dev->iamthif_cl) {
		rets = mei_amthif_write(cl, write_cb);

		if (rets) {
			dev_err(dev->dev,
				"amthif write failed with status = %d\n", rets);
			goto out;
		}
		mei_me_cl_put(me_cl);
		mutex_unlock(&dev->device_lock);
		return length;
	}

	rets = mei_cl_write(cl, write_cb, false);
out:
	mei_me_cl_put(me_cl);
	mutex_unlock(&dev->device_lock);
	if (rets < 0)
		mei_io_cb_free(write_cb);
	return rets;
}

/**
 * mei_ioctl_connect_client - the connect to fw client IOCTL function
 *
 * @file: private data of the file object
 * @data: IOCTL connect data, input and output parameters
 *
 * Locking: called under "dev->device_lock" lock
 *
 * Return: 0 on success, <0 on failure.
 */
static int mei_ioctl_connect_client(struct file *file,
			struct mei_connect_client_data *data)
{
	struct mei_device *dev;
	struct mei_client *client;
	struct mei_me_client *me_cl;
	struct mei_cl *cl;
	int rets;

	cl = file->private_data;
	dev = cl->dev;

	if (dev->dev_state != MEI_DEV_ENABLED)
		return -ENODEV;

	if (cl->state != MEI_FILE_INITIALIZING &&
	    cl->state != MEI_FILE_DISCONNECTED)
		return  -EBUSY;

	/* find ME client we're trying to connect to */
	me_cl = mei_me_cl_by_uuid(dev, &data->in_client_uuid);
	if (!me_cl || me_cl->props.fixed_address) {
		dev_dbg(dev->dev, "Cannot connect to FW Client UUID = %pUl\n",
				&data->in_client_uuid);
		return  -ENOTTY;
	}

	cl->me_client_id = me_cl->client_id;
	cl->cl_uuid = me_cl->props.protocol_name;

	dev_dbg(dev->dev, "Connect to FW Client ID = %d\n",
			cl->me_client_id);
	dev_dbg(dev->dev, "FW Client - Protocol Version = %d\n",
			me_cl->props.protocol_version);
	dev_dbg(dev->dev, "FW Client - Max Msg Len = %d\n",
			me_cl->props.max_msg_length);

	/* if we're connecting to amthif client then we will use the
	 * existing connection
	 */
	if (uuid_le_cmp(data->in_client_uuid, mei_amthif_guid) == 0) {
		dev_dbg(dev->dev, "FW Client is amthi\n");
		if (!mei_cl_is_connected(&dev->iamthif_cl)) {
			rets = -ENODEV;
			goto end;
		}
		mei_cl_unlink(cl);

		kfree(cl);
		cl = NULL;
		dev->iamthif_open_count++;
		file->private_data = &dev->iamthif_cl;

		client = &data->out_client_properties;
		client->max_msg_length = me_cl->props.max_msg_length;
		client->protocol_version = me_cl->props.protocol_version;
		rets = dev->iamthif_cl.status;

		goto end;
	}

	/* prepare the output buffer */
	client = &data->out_client_properties;
	client->max_msg_length = me_cl->props.max_msg_length;
	client->protocol_version = me_cl->props.protocol_version;
	dev_dbg(dev->dev, "Can connect?\n");

	rets = mei_cl_connect(cl, file);

end:
	mei_me_cl_put(me_cl);
	return rets;
}

/**
 * mei_ioctl - the IOCTL function
 *
 * @file: pointer to file structure
 * @cmd: ioctl command
 * @data: pointer to mei message structure
 *
 * Return: 0 on success , <0 on error
 */
static long mei_ioctl(struct file *file, unsigned int cmd, unsigned long data)
{
	struct mei_device *dev;
	struct mei_cl *cl = file->private_data;
	struct mei_connect_client_data connect_data;
	int rets;


	if (WARN_ON(!cl || !cl->dev))
		return -ENODEV;

	dev = cl->dev;

	dev_dbg(dev->dev, "IOCTL cmd = 0x%x", cmd);

	mutex_lock(&dev->device_lock);
	if (dev->dev_state != MEI_DEV_ENABLED) {
		rets = -ENODEV;
		goto out;
	}

	switch (cmd) {
	case IOCTL_MEI_CONNECT_CLIENT:
		dev_dbg(dev->dev, ": IOCTL_MEI_CONNECT_CLIENT.\n");
		if (copy_from_user(&connect_data, (char __user *)data,
				sizeof(struct mei_connect_client_data))) {
			dev_dbg(dev->dev, "failed to copy data from userland\n");
			rets = -EFAULT;
			goto out;
		}

		rets = mei_ioctl_connect_client(file, &connect_data);
		if (rets)
			goto out;

		/* if all is ok, copying the data back to user. */
		if (copy_to_user((char __user *)data, &connect_data,
				sizeof(struct mei_connect_client_data))) {
			dev_dbg(dev->dev, "failed to copy data to userland\n");
			rets = -EFAULT;
			goto out;
		}

		break;

	default:
		rets = -ENOIOCTLCMD;
	}

out:
	mutex_unlock(&dev->device_lock);
	return rets;
}

/**
 * mei_compat_ioctl - the compat IOCTL function
 *
 * @file: pointer to file structure
 * @cmd: ioctl command
 * @data: pointer to mei message structure
 *
 * Return: 0 on success , <0 on error
 */
#ifdef CONFIG_COMPAT
static long mei_compat_ioctl(struct file *file,
			unsigned int cmd, unsigned long data)
{
	return mei_ioctl(file, cmd, (unsigned long)compat_ptr(data));
}
#endif


/**
 * mei_poll - the poll function
 *
 * @file: pointer to file structure
 * @wait: pointer to poll_table structure
 *
 * Return: poll mask
 */
static unsigned int mei_poll(struct file *file, poll_table *wait)
{
	unsigned long req_events = poll_requested_events(wait);
	struct mei_cl *cl = file->private_data;
	struct mei_device *dev;
	unsigned int mask = 0;

	if (WARN_ON(!cl || !cl->dev))
		return POLLERR;

	dev = cl->dev;

	mutex_lock(&dev->device_lock);


	if (dev->dev_state != MEI_DEV_ENABLED ||
	    !mei_cl_is_connected(cl)) {
		mask = POLLERR;
		goto out;
	}

	if (cl == &dev->iamthif_cl) {
		mask = mei_amthif_poll(dev, file, wait);
		goto out;
	}

	if (req_events & (POLLIN | POLLRDNORM)) {
		poll_wait(file, &cl->rx_wait, wait);

		if (!list_empty(&cl->rd_completed))
			mask |= POLLIN | POLLRDNORM;
		else
			mei_cl_read_start(cl, 0, file);
	}

out:
	mutex_unlock(&dev->device_lock);
	return mask;
}

/**
 * fw_status_show - mei device attribute show method
 *
 * @device: device pointer
 * @attr: attribute pointer
 * @buf:  char out buffer
 *
 * Return: number of the bytes printed into buf or error
 */
static ssize_t fw_status_show(struct device *device,
		struct device_attribute *attr, char *buf)
{
	struct mei_device *dev = dev_get_drvdata(device);
	struct mei_fw_status fw_status;
	int err, i;
	ssize_t cnt = 0;

	mutex_lock(&dev->device_lock);
	err = mei_fw_status(dev, &fw_status);
	mutex_unlock(&dev->device_lock);
	if (err) {
		dev_err(device, "read fw_status error = %d\n", err);
		return err;
	}

	for (i = 0; i < fw_status.count; i++)
		cnt += scnprintf(buf + cnt, PAGE_SIZE - cnt, "%08X\n",
				fw_status.status[i]);
	return cnt;
}
static DEVICE_ATTR_RO(fw_status);

static struct attribute *mei_attrs[] = {
	&dev_attr_fw_status.attr,
	NULL
};
ATTRIBUTE_GROUPS(mei);

/*
 * file operations structure will be used for mei char device.
 */
static const struct file_operations mei_fops = {
	.owner = THIS_MODULE,
	.read = mei_read,
	.unlocked_ioctl = mei_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl = mei_compat_ioctl,
#endif
	.open = mei_open,
	.release = mei_release,
	.write = mei_write,
	.poll = mei_poll,
	.llseek = no_llseek
};

static struct class *mei_class;
static dev_t mei_devt;
#define MEI_MAX_DEVS  MINORMASK
static DEFINE_MUTEX(mei_minor_lock);
static DEFINE_IDR(mei_idr);

/**
 * mei_minor_get - obtain next free device minor number
 *
 * @dev:  device pointer
 *
 * Return: allocated minor, or -ENOSPC if no free minor left
 */
static int mei_minor_get(struct mei_device *dev)
{
	int ret;

	mutex_lock(&mei_minor_lock);
	ret = idr_alloc(&mei_idr, dev, 0, MEI_MAX_DEVS, GFP_KERNEL);
	if (ret >= 0)
		dev->minor = ret;
	else if (ret == -ENOSPC)
		dev_err(dev->dev, "too many mei devices\n");

	mutex_unlock(&mei_minor_lock);
	return ret;
}

/**
 * mei_minor_free - mark device minor number as free
 *
 * @dev:  device pointer
 */
static void mei_minor_free(struct mei_device *dev)
{
	mutex_lock(&mei_minor_lock);
	idr_remove(&mei_idr, dev->minor);
	mutex_unlock(&mei_minor_lock);
}

int mei_register(struct mei_device *dev, struct device *parent)
{
	struct device *clsdev; /* class device */
	int ret, devno;

	ret = mei_minor_get(dev);
	if (ret < 0)
		return ret;

	/* Fill in the data structures */
	devno = MKDEV(MAJOR(mei_devt), dev->minor);
	cdev_init(&dev->cdev, &mei_fops);
	dev->cdev.owner = parent->driver->owner;

	/* Add the device */
	ret = cdev_add(&dev->cdev, devno, 1);
	if (ret) {
		dev_err(parent, "unable to add device %d:%d\n",
			MAJOR(mei_devt), dev->minor);
		goto err_dev_add;
	}

	clsdev = device_create_with_groups(mei_class, parent, devno,
					   dev, mei_groups,
					   "mei%d", dev->minor);

	if (IS_ERR(clsdev)) {
		dev_err(parent, "unable to create device %d:%d\n",
			MAJOR(mei_devt), dev->minor);
		ret = PTR_ERR(clsdev);
		goto err_dev_create;
	}

	ret = mei_dbgfs_register(dev, dev_name(clsdev));
	if (ret) {
		dev_err(clsdev, "cannot register debugfs ret = %d\n", ret);
		goto err_dev_dbgfs;
	}

	return 0;

err_dev_dbgfs:
	device_destroy(mei_class, devno);
err_dev_create:
	cdev_del(&dev->cdev);
err_dev_add:
	mei_minor_free(dev);
	return ret;
}
EXPORT_SYMBOL_GPL(mei_register);

void mei_deregister(struct mei_device *dev)
{
	int devno;

	devno = dev->cdev.dev;
	cdev_del(&dev->cdev);

	mei_dbgfs_deregister(dev);

	device_destroy(mei_class, devno);

	mei_minor_free(dev);
}
EXPORT_SYMBOL_GPL(mei_deregister);

static int __init mei_init(void)
{
	int ret;

	mei_class = class_create(THIS_MODULE, "mei");
	if (IS_ERR(mei_class)) {
		pr_err("couldn't create class\n");
		ret = PTR_ERR(mei_class);
		goto err;
	}

	ret = alloc_chrdev_region(&mei_devt, 0, MEI_MAX_DEVS, "mei");
	if (ret < 0) {
		pr_err("unable to allocate char dev region\n");
		goto err_class;
	}

	ret = mei_cl_bus_init();
	if (ret < 0) {
		pr_err("unable to initialize bus\n");
		goto err_chrdev;
	}

	return 0;

err_chrdev:
	unregister_chrdev_region(mei_devt, MEI_MAX_DEVS);
err_class:
	class_destroy(mei_class);
err:
	return ret;
}

static void __exit mei_exit(void)
{
	unregister_chrdev_region(mei_devt, MEI_MAX_DEVS);
	class_destroy(mei_class);
	mei_cl_bus_exit();
}

module_init(mei_init);
module_exit(mei_exit);

MODULE_AUTHOR("Intel Corporation");
MODULE_DESCRIPTION("Intel(R) Management Engine Interface");
MODULE_LICENSE("GPL v2");

