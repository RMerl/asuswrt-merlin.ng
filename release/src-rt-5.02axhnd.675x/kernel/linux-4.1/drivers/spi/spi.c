/*
 * SPI init/core code
 *
 * Copyright (C) 2005 David Brownell
 * Copyright (C) 2008 Secret Lab Technologies Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/init.h>
#include <linux/cache.h>
#include <linux/dma-mapping.h>
#include <linux/dmaengine.h>
#include <linux/mutex.h>
#include <linux/of_device.h>
#include <linux/of_irq.h>
#include <linux/clk/clk-conf.h>
#include <linux/slab.h>
#include <linux/mod_devicetable.h>
#include <linux/spi/spi.h>
#include <linux/of_gpio.h>
#include <linux/pm_runtime.h>
#include <linux/pm_domain.h>
#include <linux/export.h>
#include <linux/sched/rt.h>
#include <linux/delay.h>
#include <linux/kthread.h>
#include <linux/ioport.h>
#include <linux/acpi.h>

#define CREATE_TRACE_POINTS
#include <trace/events/spi.h>

static void spidev_release(struct device *dev)
{
	struct spi_device	*spi = to_spi_device(dev);

	/* spi masters may cleanup for released devices */
	if (spi->master->cleanup)
		spi->master->cleanup(spi);

	spi_master_put(spi->master);
	kfree(spi);
}

static ssize_t
modalias_show(struct device *dev, struct device_attribute *a, char *buf)
{
	const struct spi_device	*spi = to_spi_device(dev);
	int len;

	len = acpi_device_modalias(dev, buf, PAGE_SIZE - 1);
	if (len != -ENODEV)
		return len;

	return sprintf(buf, "%s%s\n", SPI_MODULE_PREFIX, spi->modalias);
}
static DEVICE_ATTR_RO(modalias);

static struct attribute *spi_dev_attrs[] = {
	&dev_attr_modalias.attr,
	NULL,
};
ATTRIBUTE_GROUPS(spi_dev);

/* modalias support makes "modprobe $MODALIAS" new-style hotplug work,
 * and the sysfs version makes coldplug work too.
 */

static const struct spi_device_id *spi_match_id(const struct spi_device_id *id,
						const struct spi_device *sdev)
{
	while (id->name[0]) {
		if (!strcmp(sdev->modalias, id->name))
			return id;
		id++;
	}
	return NULL;
}

const struct spi_device_id *spi_get_device_id(const struct spi_device *sdev)
{
	const struct spi_driver *sdrv = to_spi_driver(sdev->dev.driver);

	return spi_match_id(sdrv->id_table, sdev);
}
EXPORT_SYMBOL_GPL(spi_get_device_id);

static int spi_match_device(struct device *dev, struct device_driver *drv)
{
	const struct spi_device	*spi = to_spi_device(dev);
	const struct spi_driver	*sdrv = to_spi_driver(drv);

	/* Attempt an OF style match */
	if (of_driver_match_device(dev, drv))
		return 1;

	/* Then try ACPI */
	if (acpi_driver_match_device(dev, drv))
		return 1;

	if (sdrv->id_table)
		return !!spi_match_id(sdrv->id_table, spi);

	return strcmp(spi->modalias, drv->name) == 0;
}

static int spi_uevent(struct device *dev, struct kobj_uevent_env *env)
{
	const struct spi_device		*spi = to_spi_device(dev);
	int rc;

	rc = acpi_device_uevent_modalias(dev, env);
	if (rc != -ENODEV)
		return rc;

	add_uevent_var(env, "MODALIAS=%s%s", SPI_MODULE_PREFIX, spi->modalias);
	return 0;
}

struct bus_type spi_bus_type = {
	.name		= "spi",
	.dev_groups	= spi_dev_groups,
	.match		= spi_match_device,
	.uevent		= spi_uevent,
};
EXPORT_SYMBOL_GPL(spi_bus_type);


static int spi_drv_probe(struct device *dev)
{
	const struct spi_driver		*sdrv = to_spi_driver(dev->driver);
	int ret;

	ret = of_clk_set_defaults(dev->of_node, false);
	if (ret)
		return ret;

	ret = dev_pm_domain_attach(dev, true);
	if (ret != -EPROBE_DEFER) {
		ret = sdrv->probe(to_spi_device(dev));
		if (ret)
			dev_pm_domain_detach(dev, true);
	}

	return ret;
}

static int spi_drv_remove(struct device *dev)
{
	const struct spi_driver		*sdrv = to_spi_driver(dev->driver);
	int ret;

	ret = sdrv->remove(to_spi_device(dev));
	dev_pm_domain_detach(dev, true);

	return ret;
}

static void spi_drv_shutdown(struct device *dev)
{
	const struct spi_driver		*sdrv = to_spi_driver(dev->driver);

	sdrv->shutdown(to_spi_device(dev));
}

/**
 * spi_register_driver - register a SPI driver
 * @sdrv: the driver to register
 * Context: can sleep
 */
int spi_register_driver(struct spi_driver *sdrv)
{
	sdrv->driver.bus = &spi_bus_type;
	if (sdrv->probe)
		sdrv->driver.probe = spi_drv_probe;
	if (sdrv->remove)
		sdrv->driver.remove = spi_drv_remove;
	if (sdrv->shutdown)
		sdrv->driver.shutdown = spi_drv_shutdown;
	return driver_register(&sdrv->driver);
}
EXPORT_SYMBOL_GPL(spi_register_driver);

/*-------------------------------------------------------------------------*/

/* SPI devices should normally not be created by SPI device drivers; that
 * would make them board-specific.  Similarly with SPI master drivers.
 * Device registration normally goes into like arch/.../mach.../board-YYY.c
 * with other readonly (flashable) information about mainboard devices.
 */

struct boardinfo {
	struct list_head	list;
	struct spi_board_info	board_info;
};

static LIST_HEAD(board_list);
static LIST_HEAD(spi_master_list);

/*
 * Used to protect add/del opertion for board_info list and
 * spi_master list, and their matching process
 */
static DEFINE_MUTEX(board_lock);

/**
 * spi_alloc_device - Allocate a new SPI device
 * @master: Controller to which device is connected
 * Context: can sleep
 *
 * Allows a driver to allocate and initialize a spi_device without
 * registering it immediately.  This allows a driver to directly
 * fill the spi_device with device parameters before calling
 * spi_add_device() on it.
 *
 * Caller is responsible to call spi_add_device() on the returned
 * spi_device structure to add it to the SPI master.  If the caller
 * needs to discard the spi_device without adding it, then it should
 * call spi_dev_put() on it.
 *
 * Returns a pointer to the new device, or NULL.
 */
struct spi_device *spi_alloc_device(struct spi_master *master)
{
	struct spi_device	*spi;

	if (!spi_master_get(master))
		return NULL;

	spi = kzalloc(sizeof(*spi), GFP_KERNEL);
	if (!spi) {
		spi_master_put(master);
		return NULL;
	}

	spi->master = master;
	spi->dev.parent = &master->dev;
	spi->dev.bus = &spi_bus_type;
	spi->dev.release = spidev_release;
	spi->cs_gpio = -ENOENT;
	device_initialize(&spi->dev);
	return spi;
}
EXPORT_SYMBOL_GPL(spi_alloc_device);

static void spi_dev_set_name(struct spi_device *spi)
{
	struct acpi_device *adev = ACPI_COMPANION(&spi->dev);

	if (adev) {
		dev_set_name(&spi->dev, "spi-%s", acpi_dev_name(adev));
		return;
	}

	dev_set_name(&spi->dev, "%s.%u", dev_name(&spi->master->dev),
		     spi->chip_select);
}

static int spi_dev_check(struct device *dev, void *data)
{
	struct spi_device *spi = to_spi_device(dev);
	struct spi_device *new_spi = data;

	if (spi->master == new_spi->master &&
	    spi->chip_select == new_spi->chip_select)
		return -EBUSY;
	return 0;
}

/**
 * spi_add_device - Add spi_device allocated with spi_alloc_device
 * @spi: spi_device to register
 *
 * Companion function to spi_alloc_device.  Devices allocated with
 * spi_alloc_device can be added onto the spi bus with this function.
 *
 * Returns 0 on success; negative errno on failure
 */
int spi_add_device(struct spi_device *spi)
{
	static DEFINE_MUTEX(spi_add_lock);
	struct spi_master *master = spi->master;
	struct device *dev = master->dev.parent;
	int status;

	/* Chipselects are numbered 0..max; validate. */
	if (spi->chip_select >= master->num_chipselect) {
		dev_err(dev, "cs%d >= max %d\n",
			spi->chip_select,
			master->num_chipselect);
		return -EINVAL;
	}

	/* Set the bus ID string */
	spi_dev_set_name(spi);

	/* We need to make sure there's no other device with this
	 * chipselect **BEFORE** we call setup(), else we'll trash
	 * its configuration.  Lock against concurrent add() calls.
	 */
	mutex_lock(&spi_add_lock);

	status = bus_for_each_dev(&spi_bus_type, NULL, spi, spi_dev_check);
	if (status) {
		dev_err(dev, "chipselect %d already in use\n",
				spi->chip_select);
		goto done;
	}

	if (master->cs_gpios)
		spi->cs_gpio = master->cs_gpios[spi->chip_select];

	/* Drivers may modify this initial i/o setup, but will
	 * normally rely on the device being setup.  Devices
	 * using SPI_CS_HIGH can't coexist well otherwise...
	 */
	status = spi_setup(spi);
	if (status < 0) {
		dev_err(dev, "can't setup %s, status %d\n",
				dev_name(&spi->dev), status);
		goto done;
	}

	/* Device may be bound to an active driver when this returns */
	status = device_add(&spi->dev);
	if (status < 0)
		dev_err(dev, "can't add %s, status %d\n",
				dev_name(&spi->dev), status);
	else
		dev_dbg(dev, "registered child %s\n", dev_name(&spi->dev));

done:
	mutex_unlock(&spi_add_lock);
	return status;
}
EXPORT_SYMBOL_GPL(spi_add_device);

/**
 * spi_new_device - instantiate one new SPI device
 * @master: Controller to which device is connected
 * @chip: Describes the SPI device
 * Context: can sleep
 *
 * On typical mainboards, this is purely internal; and it's not needed
 * after board init creates the hard-wired devices.  Some development
 * platforms may not be able to use spi_register_board_info though, and
 * this is exported so that for example a USB or parport based adapter
 * driver could add devices (which it would learn about out-of-band).
 *
 * Returns the new device, or NULL.
 */
struct spi_device *spi_new_device(struct spi_master *master,
				  struct spi_board_info *chip)
{
	struct spi_device	*proxy;
	int			status;

	/* NOTE:  caller did any chip->bus_num checks necessary.
	 *
	 * Also, unless we change the return value convention to use
	 * error-or-pointer (not NULL-or-pointer), troubleshootability
	 * suggests syslogged diagnostics are best here (ugh).
	 */

	proxy = spi_alloc_device(master);
	if (!proxy)
		return NULL;

	WARN_ON(strlen(chip->modalias) >= sizeof(proxy->modalias));

	proxy->chip_select = chip->chip_select;
	proxy->max_speed_hz = chip->max_speed_hz;
	proxy->mode = chip->mode;
	proxy->irq = chip->irq;
	strlcpy(proxy->modalias, chip->modalias, sizeof(proxy->modalias));
	proxy->dev.platform_data = (void *) chip->platform_data;
	proxy->controller_data = chip->controller_data;
	proxy->controller_state = NULL;

	status = spi_add_device(proxy);
	if (status < 0) {
		spi_dev_put(proxy);
		return NULL;
	}

	return proxy;
}
EXPORT_SYMBOL_GPL(spi_new_device);

static void spi_match_master_to_boardinfo(struct spi_master *master,
				struct spi_board_info *bi)
{
	struct spi_device *dev;

	if (master->bus_num != bi->bus_num)
		return;

	dev = spi_new_device(master, bi);
	if (!dev)
		dev_err(master->dev.parent, "can't create new device for %s\n",
			bi->modalias);
}

/**
 * spi_register_board_info - register SPI devices for a given board
 * @info: array of chip descriptors
 * @n: how many descriptors are provided
 * Context: can sleep
 *
 * Board-specific early init code calls this (probably during arch_initcall)
 * with segments of the SPI device table.  Any device nodes are created later,
 * after the relevant parent SPI controller (bus_num) is defined.  We keep
 * this table of devices forever, so that reloading a controller driver will
 * not make Linux forget about these hard-wired devices.
 *
 * Other code can also call this, e.g. a particular add-on board might provide
 * SPI devices through its expansion connector, so code initializing that board
 * would naturally declare its SPI devices.
 *
 * The board info passed can safely be __initdata ... but be careful of
 * any embedded pointers (platform_data, etc), they're copied as-is.
 */
int spi_register_board_info(struct spi_board_info const *info, unsigned n)
{
	struct boardinfo *bi;
	int i;

	if (!n)
		return -EINVAL;

	bi = kzalloc(n * sizeof(*bi), GFP_KERNEL);
	if (!bi)
		return -ENOMEM;

	for (i = 0; i < n; i++, bi++, info++) {
		struct spi_master *master;

		memcpy(&bi->board_info, info, sizeof(*info));
		mutex_lock(&board_lock);
		list_add_tail(&bi->list, &board_list);
		list_for_each_entry(master, &spi_master_list, list)
			spi_match_master_to_boardinfo(master, &bi->board_info);
		mutex_unlock(&board_lock);
	}

	return 0;
}

/*-------------------------------------------------------------------------*/

static void spi_set_cs(struct spi_device *spi, bool enable)
{
	if (spi->mode & SPI_CS_HIGH)
		enable = !enable;

	if (spi->cs_gpio >= 0)
		gpio_set_value(spi->cs_gpio, !enable);
	else if (spi->master->set_cs)
		spi->master->set_cs(spi, !enable);
}

#ifdef CONFIG_HAS_DMA
static int spi_map_buf(struct spi_master *master, struct device *dev,
		       struct sg_table *sgt, void *buf, size_t len,
		       enum dma_data_direction dir)
{
	const bool vmalloced_buf = is_vmalloc_addr(buf);
	const int desc_len = vmalloced_buf ? PAGE_SIZE : master->max_dma_len;
	const int sgs = DIV_ROUND_UP(len, desc_len);
	struct page *vm_page;
	void *sg_buf;
	size_t min;
	int i, ret;

	ret = sg_alloc_table(sgt, sgs, GFP_KERNEL);
	if (ret != 0)
		return ret;

	for (i = 0; i < sgs; i++) {
		min = min_t(size_t, len, desc_len);

		if (vmalloced_buf) {
			vm_page = vmalloc_to_page(buf);
			if (!vm_page) {
				sg_free_table(sgt);
				return -ENOMEM;
			}
			sg_set_page(&sgt->sgl[i], vm_page,
				    min, offset_in_page(buf));
		} else {
			sg_buf = buf;
			sg_set_buf(&sgt->sgl[i], sg_buf, min);
		}


		buf += min;
		len -= min;
	}

	ret = dma_map_sg(dev, sgt->sgl, sgt->nents, dir);
	if (!ret)
		ret = -ENOMEM;
	if (ret < 0) {
		sg_free_table(sgt);
		return ret;
	}

	sgt->nents = ret;

	return 0;
}

static void spi_unmap_buf(struct spi_master *master, struct device *dev,
			  struct sg_table *sgt, enum dma_data_direction dir)
{
	if (sgt->orig_nents) {
		dma_unmap_sg(dev, sgt->sgl, sgt->orig_nents, dir);
		sg_free_table(sgt);
	}
}

static int __spi_map_msg(struct spi_master *master, struct spi_message *msg)
{
	struct device *tx_dev, *rx_dev;
	struct spi_transfer *xfer;
	int ret;

	if (!master->can_dma)
		return 0;

	tx_dev = master->dma_tx->device->dev;
	rx_dev = master->dma_rx->device->dev;

	list_for_each_entry(xfer, &msg->transfers, transfer_list) {
		if (!master->can_dma(master, msg->spi, xfer))
			continue;

		if (xfer->tx_buf != NULL) {
			ret = spi_map_buf(master, tx_dev, &xfer->tx_sg,
					  (void *)xfer->tx_buf, xfer->len,
					  DMA_TO_DEVICE);
			if (ret != 0)
				return ret;
		}

		if (xfer->rx_buf != NULL) {
			ret = spi_map_buf(master, rx_dev, &xfer->rx_sg,
					  xfer->rx_buf, xfer->len,
					  DMA_FROM_DEVICE);
			if (ret != 0) {
				spi_unmap_buf(master, tx_dev, &xfer->tx_sg,
					      DMA_TO_DEVICE);
				return ret;
			}
		}
	}

	master->cur_msg_mapped = true;

	return 0;
}

static int spi_unmap_msg(struct spi_master *master, struct spi_message *msg)
{
	struct spi_transfer *xfer;
	struct device *tx_dev, *rx_dev;

	if (!master->cur_msg_mapped || !master->can_dma)
		return 0;

	tx_dev = master->dma_tx->device->dev;
	rx_dev = master->dma_rx->device->dev;

	list_for_each_entry(xfer, &msg->transfers, transfer_list) {
		/*
		 * Restore the original value of tx_buf or rx_buf if they are
		 * NULL.
		 */
		if (xfer->tx_buf == master->dummy_tx)
			xfer->tx_buf = NULL;
		if (xfer->rx_buf == master->dummy_rx)
			xfer->rx_buf = NULL;

		if (!master->can_dma(master, msg->spi, xfer))
			continue;

		spi_unmap_buf(master, rx_dev, &xfer->rx_sg, DMA_FROM_DEVICE);
		spi_unmap_buf(master, tx_dev, &xfer->tx_sg, DMA_TO_DEVICE);
	}

	return 0;
}
#else /* !CONFIG_HAS_DMA */
static inline int __spi_map_msg(struct spi_master *master,
				struct spi_message *msg)
{
	return 0;
}

static inline int spi_unmap_msg(struct spi_master *master,
				struct spi_message *msg)
{
	return 0;
}
#endif /* !CONFIG_HAS_DMA */

static int spi_map_msg(struct spi_master *master, struct spi_message *msg)
{
	struct spi_transfer *xfer;
	void *tmp;
	unsigned int max_tx, max_rx;

	if (master->flags & (SPI_MASTER_MUST_RX | SPI_MASTER_MUST_TX)) {
		max_tx = 0;
		max_rx = 0;

		list_for_each_entry(xfer, &msg->transfers, transfer_list) {
			if ((master->flags & SPI_MASTER_MUST_TX) &&
			    !xfer->tx_buf)
				max_tx = max(xfer->len, max_tx);
			if ((master->flags & SPI_MASTER_MUST_RX) &&
			    !xfer->rx_buf)
				max_rx = max(xfer->len, max_rx);
		}

		if (max_tx) {
			tmp = krealloc(master->dummy_tx, max_tx,
				       GFP_KERNEL | GFP_DMA);
			if (!tmp)
				return -ENOMEM;
			master->dummy_tx = tmp;
			memset(tmp, 0, max_tx);
		}

		if (max_rx) {
			tmp = krealloc(master->dummy_rx, max_rx,
				       GFP_KERNEL | GFP_DMA);
			if (!tmp)
				return -ENOMEM;
			master->dummy_rx = tmp;
		}

		if (max_tx || max_rx) {
			list_for_each_entry(xfer, &msg->transfers,
					    transfer_list) {
				if (!xfer->tx_buf)
					xfer->tx_buf = master->dummy_tx;
				if (!xfer->rx_buf)
					xfer->rx_buf = master->dummy_rx;
			}
		}
	}

	return __spi_map_msg(master, msg);
}

/*
 * spi_transfer_one_message - Default implementation of transfer_one_message()
 *
 * This is a standard implementation of transfer_one_message() for
 * drivers which impelment a transfer_one() operation.  It provides
 * standard handling of delays and chip select management.
 */
static int spi_transfer_one_message(struct spi_master *master,
				    struct spi_message *msg)
{
	struct spi_transfer *xfer;
	bool keep_cs = false;
	int ret = 0;
	unsigned long ms = 1;

	spi_set_cs(msg->spi, true);

	list_for_each_entry(xfer, &msg->transfers, transfer_list) {
		trace_spi_transfer_start(msg, xfer);

		if (xfer->tx_buf || xfer->rx_buf) {
			reinit_completion(&master->xfer_completion);

			ret = master->transfer_one(master, msg->spi, xfer);
			if (ret < 0) {
				dev_err(&msg->spi->dev,
					"SPI transfer failed: %d\n", ret);
				goto out;
			}

			if (ret > 0) {
				ret = 0;
				ms = xfer->len * 8 * 1000 / xfer->speed_hz;
				ms += ms + 100; /* some tolerance */

				ms = wait_for_completion_timeout(&master->xfer_completion,
								 msecs_to_jiffies(ms));
			}

			if (ms == 0) {
				dev_err(&msg->spi->dev,
					"SPI transfer timed out\n");
				msg->status = -ETIMEDOUT;
			}
		} else {
			if (xfer->len)
				dev_err(&msg->spi->dev,
					"Bufferless transfer has length %u\n",
					xfer->len);
		}

		trace_spi_transfer_stop(msg, xfer);

		if (msg->status != -EINPROGRESS)
			goto out;

		if (xfer->delay_usecs)
			udelay(xfer->delay_usecs);

		if (xfer->cs_change) {
			if (list_is_last(&xfer->transfer_list,
					 &msg->transfers)) {
				keep_cs = true;
			} else {
				spi_set_cs(msg->spi, false);
				udelay(10);
				spi_set_cs(msg->spi, true);
			}
		}

		msg->actual_length += xfer->len;
	}

out:
	if (ret != 0 || !keep_cs)
		spi_set_cs(msg->spi, false);

	if (msg->status == -EINPROGRESS)
		msg->status = ret;

	if (msg->status && master->handle_err)
		master->handle_err(master, msg);

	spi_finalize_current_message(master);

	return ret;
}

/**
 * spi_finalize_current_transfer - report completion of a transfer
 * @master: the master reporting completion
 *
 * Called by SPI drivers using the core transfer_one_message()
 * implementation to notify it that the current interrupt driven
 * transfer has finished and the next one may be scheduled.
 */
void spi_finalize_current_transfer(struct spi_master *master)
{
	complete(&master->xfer_completion);
}
EXPORT_SYMBOL_GPL(spi_finalize_current_transfer);

/**
 * __spi_pump_messages - function which processes spi message queue
 * @master: master to process queue for
 * @in_kthread: true if we are in the context of the message pump thread
 *
 * This function checks if there is any spi message in the queue that
 * needs processing and if so call out to the driver to initialize hardware
 * and transfer each message.
 *
 * Note that it is called both from the kthread itself and also from
 * inside spi_sync(); the queue extraction handling at the top of the
 * function should deal with this safely.
 */
static void __spi_pump_messages(struct spi_master *master, bool in_kthread)
{
	unsigned long flags;
	bool was_busy = false;
	int ret;

	/* Lock queue */
	spin_lock_irqsave(&master->queue_lock, flags);

	/* Make sure we are not already running a message */
	if (master->cur_msg) {
		spin_unlock_irqrestore(&master->queue_lock, flags);
		return;
	}

	/* If another context is idling the device then defer */
	if (master->idling) {
		queue_kthread_work(&master->kworker, &master->pump_messages);
		spin_unlock_irqrestore(&master->queue_lock, flags);
		return;
	}

	/* Check if the queue is idle */
	if (list_empty(&master->queue) || !master->running) {
		if (!master->busy) {
			spin_unlock_irqrestore(&master->queue_lock, flags);
			return;
		}

		/* Only do teardown in the thread */
		if (!in_kthread) {
			queue_kthread_work(&master->kworker,
					   &master->pump_messages);
			spin_unlock_irqrestore(&master->queue_lock, flags);
			return;
		}

		master->busy = false;
		master->idling = true;
		spin_unlock_irqrestore(&master->queue_lock, flags);

		kfree(master->dummy_rx);
		master->dummy_rx = NULL;
		kfree(master->dummy_tx);
		master->dummy_tx = NULL;
		if (master->unprepare_transfer_hardware &&
		    master->unprepare_transfer_hardware(master))
			dev_err(&master->dev,
				"failed to unprepare transfer hardware\n");
		if (master->auto_runtime_pm) {
			pm_runtime_mark_last_busy(master->dev.parent);
			pm_runtime_put_autosuspend(master->dev.parent);
		}
		trace_spi_master_idle(master);

		spin_lock_irqsave(&master->queue_lock, flags);
		master->idling = false;
		spin_unlock_irqrestore(&master->queue_lock, flags);
		return;
	}

	/* Extract head of queue */
	master->cur_msg =
		list_first_entry(&master->queue, struct spi_message, queue);

	list_del_init(&master->cur_msg->queue);
	if (master->busy)
		was_busy = true;
	else
		master->busy = true;
	spin_unlock_irqrestore(&master->queue_lock, flags);

	if (!was_busy && master->auto_runtime_pm) {
		ret = pm_runtime_get_sync(master->dev.parent);
		if (ret < 0) {
			dev_err(&master->dev, "Failed to power device: %d\n",
				ret);
			return;
		}
	}

	if (!was_busy)
		trace_spi_master_busy(master);

	if (!was_busy && master->prepare_transfer_hardware) {
		ret = master->prepare_transfer_hardware(master);
		if (ret) {
			dev_err(&master->dev,
				"failed to prepare transfer hardware\n");

			if (master->auto_runtime_pm)
				pm_runtime_put(master->dev.parent);
			return;
		}
	}

	trace_spi_message_start(master->cur_msg);

	if (master->prepare_message) {
		ret = master->prepare_message(master, master->cur_msg);
		if (ret) {
			dev_err(&master->dev,
				"failed to prepare message: %d\n", ret);
			master->cur_msg->status = ret;
			spi_finalize_current_message(master);
			return;
		}
		master->cur_msg_prepared = true;
	}

	ret = spi_map_msg(master, master->cur_msg);
	if (ret) {
		master->cur_msg->status = ret;
		spi_finalize_current_message(master);
		return;
	}

	ret = master->transfer_one_message(master, master->cur_msg);
	if (ret) {
		dev_err(&master->dev,
			"failed to transfer one message from queue\n");
		return;
	}
}

/**
 * spi_pump_messages - kthread work function which processes spi message queue
 * @work: pointer to kthread work struct contained in the master struct
 */
static void spi_pump_messages(struct kthread_work *work)
{
	struct spi_master *master =
		container_of(work, struct spi_master, pump_messages);

	__spi_pump_messages(master, true);
}

static int spi_init_queue(struct spi_master *master)
{
	struct sched_param param = { .sched_priority = MAX_RT_PRIO - 1 };

	master->running = false;
	master->busy = false;

	init_kthread_worker(&master->kworker);
	master->kworker_task = kthread_run(kthread_worker_fn,
					   &master->kworker, "%s",
					   dev_name(&master->dev));
	if (IS_ERR(master->kworker_task)) {
		dev_err(&master->dev, "failed to create message pump task\n");
		return PTR_ERR(master->kworker_task);
	}
	init_kthread_work(&master->pump_messages, spi_pump_messages);

	/*
	 * Master config will indicate if this controller should run the
	 * message pump with high (realtime) priority to reduce the transfer
	 * latency on the bus by minimising the delay between a transfer
	 * request and the scheduling of the message pump thread. Without this
	 * setting the message pump thread will remain at default priority.
	 */
	if (master->rt) {
		dev_info(&master->dev,
			"will run message pump with realtime priority\n");
		sched_setscheduler(master->kworker_task, SCHED_FIFO, &param);
	}

	return 0;
}

/**
 * spi_get_next_queued_message() - called by driver to check for queued
 * messages
 * @master: the master to check for queued messages
 *
 * If there are more messages in the queue, the next message is returned from
 * this call.
 */
struct spi_message *spi_get_next_queued_message(struct spi_master *master)
{
	struct spi_message *next;
	unsigned long flags;

	/* get a pointer to the next message, if any */
	spin_lock_irqsave(&master->queue_lock, flags);
	next = list_first_entry_or_null(&master->queue, struct spi_message,
					queue);
	spin_unlock_irqrestore(&master->queue_lock, flags);

	return next;
}
EXPORT_SYMBOL_GPL(spi_get_next_queued_message);

/**
 * spi_finalize_current_message() - the current message is complete
 * @master: the master to return the message to
 *
 * Called by the driver to notify the core that the message in the front of the
 * queue is complete and can be removed from the queue.
 */
void spi_finalize_current_message(struct spi_master *master)
{
	struct spi_message *mesg;
	unsigned long flags;
	int ret;

	spin_lock_irqsave(&master->queue_lock, flags);
	mesg = master->cur_msg;
	spin_unlock_irqrestore(&master->queue_lock, flags);

	spi_unmap_msg(master, mesg);

	if (master->cur_msg_prepared && master->unprepare_message) {
		ret = master->unprepare_message(master, mesg);
		if (ret) {
			dev_err(&master->dev,
				"failed to unprepare message: %d\n", ret);
		}
	}

	spin_lock_irqsave(&master->queue_lock, flags);
	master->cur_msg = NULL;
	master->cur_msg_prepared = false;
	queue_kthread_work(&master->kworker, &master->pump_messages);
	spin_unlock_irqrestore(&master->queue_lock, flags);

	trace_spi_message_done(mesg);

	mesg->state = NULL;
	if (mesg->complete)
		mesg->complete(mesg->context);
}
EXPORT_SYMBOL_GPL(spi_finalize_current_message);

static int spi_start_queue(struct spi_master *master)
{
	unsigned long flags;

	spin_lock_irqsave(&master->queue_lock, flags);

	if (master->running || master->busy) {
		spin_unlock_irqrestore(&master->queue_lock, flags);
		return -EBUSY;
	}

	master->running = true;
	master->cur_msg = NULL;
	spin_unlock_irqrestore(&master->queue_lock, flags);

	queue_kthread_work(&master->kworker, &master->pump_messages);

	return 0;
}

static int spi_stop_queue(struct spi_master *master)
{
	unsigned long flags;
	unsigned limit = 500;
	int ret = 0;

	spin_lock_irqsave(&master->queue_lock, flags);

	/*
	 * This is a bit lame, but is optimized for the common execution path.
	 * A wait_queue on the master->busy could be used, but then the common
	 * execution path (pump_messages) would be required to call wake_up or
	 * friends on every SPI message. Do this instead.
	 */
	while ((!list_empty(&master->queue) || master->busy) && limit--) {
		spin_unlock_irqrestore(&master->queue_lock, flags);
		usleep_range(10000, 11000);
		spin_lock_irqsave(&master->queue_lock, flags);
	}

	if (!list_empty(&master->queue) || master->busy)
		ret = -EBUSY;
	else
		master->running = false;

	spin_unlock_irqrestore(&master->queue_lock, flags);

	if (ret) {
		dev_warn(&master->dev,
			 "could not stop message queue\n");
		return ret;
	}
	return ret;
}

static int spi_destroy_queue(struct spi_master *master)
{
	int ret;

	ret = spi_stop_queue(master);

	/*
	 * flush_kthread_worker will block until all work is done.
	 * If the reason that stop_queue timed out is that the work will never
	 * finish, then it does no good to call flush/stop thread, so
	 * return anyway.
	 */
	if (ret) {
		dev_err(&master->dev, "problem destroying queue\n");
		return ret;
	}

	flush_kthread_worker(&master->kworker);
	kthread_stop(master->kworker_task);

	return 0;
}

static int __spi_queued_transfer(struct spi_device *spi,
				 struct spi_message *msg,
				 bool need_pump)
{
	struct spi_master *master = spi->master;
	unsigned long flags;

	spin_lock_irqsave(&master->queue_lock, flags);

	if (!master->running) {
		spin_unlock_irqrestore(&master->queue_lock, flags);
		return -ESHUTDOWN;
	}
	msg->actual_length = 0;
	msg->status = -EINPROGRESS;

	list_add_tail(&msg->queue, &master->queue);
	if (!master->busy && need_pump)
		queue_kthread_work(&master->kworker, &master->pump_messages);

	spin_unlock_irqrestore(&master->queue_lock, flags);
	return 0;
}

/**
 * spi_queued_transfer - transfer function for queued transfers
 * @spi: spi device which is requesting transfer
 * @msg: spi message which is to handled is queued to driver queue
 */
static int spi_queued_transfer(struct spi_device *spi, struct spi_message *msg)
{
	return __spi_queued_transfer(spi, msg, true);
}

static int spi_master_initialize_queue(struct spi_master *master)
{
	int ret;

	master->transfer = spi_queued_transfer;
	if (!master->transfer_one_message)
		master->transfer_one_message = spi_transfer_one_message;

	/* Initialize and start queue */
	ret = spi_init_queue(master);
	if (ret) {
		dev_err(&master->dev, "problem initializing queue\n");
		goto err_init_queue;
	}
	master->queued = true;
	ret = spi_start_queue(master);
	if (ret) {
		dev_err(&master->dev, "problem starting queue\n");
		goto err_start_queue;
	}

	return 0;

err_start_queue:
	spi_destroy_queue(master);
err_init_queue:
	return ret;
}

/*-------------------------------------------------------------------------*/

#if defined(CONFIG_OF)
static struct spi_device *
of_register_spi_device(struct spi_master *master, struct device_node *nc)
{
	struct spi_device *spi;
	int rc;
	u32 value;

	/* Alloc an spi_device */
	spi = spi_alloc_device(master);
	if (!spi) {
		dev_err(&master->dev, "spi_device alloc error for %s\n",
			nc->full_name);
		rc = -ENOMEM;
		goto err_out;
	}

	/* Select device driver */
	rc = of_modalias_node(nc, spi->modalias,
				sizeof(spi->modalias));
	if (rc < 0) {
		dev_err(&master->dev, "cannot find modalias for %s\n",
			nc->full_name);
		goto err_out;
	}

	/* Device address */
	rc = of_property_read_u32(nc, "reg", &value);
	if (rc) {
		dev_err(&master->dev, "%s has no valid 'reg' property (%d)\n",
			nc->full_name, rc);
		goto err_out;
	}
	spi->chip_select = value;

	/* Mode (clock phase/polarity/etc.) */
	if (of_find_property(nc, "spi-cpha", NULL))
		spi->mode |= SPI_CPHA;
	if (of_find_property(nc, "spi-cpol", NULL))
		spi->mode |= SPI_CPOL;
	if (of_find_property(nc, "spi-cs-high", NULL))
		spi->mode |= SPI_CS_HIGH;
	if (of_find_property(nc, "spi-3wire", NULL))
		spi->mode |= SPI_3WIRE;
	if (of_find_property(nc, "spi-lsb-first", NULL))
		spi->mode |= SPI_LSB_FIRST;

	/* Device DUAL/QUAD mode */
	if (!of_property_read_u32(nc, "spi-tx-bus-width", &value)) {
		switch (value) {
		case 1:
			break;
		case 2:
			spi->mode |= SPI_TX_DUAL;
			break;
		case 4:
			spi->mode |= SPI_TX_QUAD;
			break;
		default:
			dev_warn(&master->dev,
				"spi-tx-bus-width %d not supported\n",
				value);
			break;
		}
	}

	if (!of_property_read_u32(nc, "spi-rx-bus-width", &value)) {
		switch (value) {
		case 1:
			break;
		case 2:
			spi->mode |= SPI_RX_DUAL;
			break;
		case 4:
			spi->mode |= SPI_RX_QUAD;
			break;
		default:
			dev_warn(&master->dev,
				"spi-rx-bus-width %d not supported\n",
				value);
			break;
		}
	}

	/* Device speed */
	rc = of_property_read_u32(nc, "spi-max-frequency", &value);
	if (rc) {
		dev_err(&master->dev, "%s has no valid 'spi-max-frequency' property (%d)\n",
			nc->full_name, rc);
		goto err_out;
	}
	spi->max_speed_hz = value;

	/* IRQ */
	spi->irq = irq_of_parse_and_map(nc, 0);

	/* Store a pointer to the node in the device structure */
	of_node_get(nc);
	spi->dev.of_node = nc;

	/* Register the new device */
	rc = spi_add_device(spi);
	if (rc) {
		dev_err(&master->dev, "spi_device register error %s\n",
			nc->full_name);
		goto err_out;
	}

	return spi;

err_out:
	spi_dev_put(spi);
	return ERR_PTR(rc);
}

/**
 * of_register_spi_devices() - Register child devices onto the SPI bus
 * @master:	Pointer to spi_master device
 *
 * Registers an spi_device for each child node of master node which has a 'reg'
 * property.
 */
static void of_register_spi_devices(struct spi_master *master)
{
	struct spi_device *spi;
	struct device_node *nc;

	if (!master->dev.of_node)
		return;

	for_each_available_child_of_node(master->dev.of_node, nc) {
		spi = of_register_spi_device(master, nc);
		if (IS_ERR(spi))
			dev_warn(&master->dev, "Failed to create SPI device for %s\n",
				nc->full_name);
	}
}
#else
static void of_register_spi_devices(struct spi_master *master) { }
#endif

#ifdef CONFIG_ACPI
static int acpi_spi_add_resource(struct acpi_resource *ares, void *data)
{
	struct spi_device *spi = data;

	if (ares->type == ACPI_RESOURCE_TYPE_SERIAL_BUS) {
		struct acpi_resource_spi_serialbus *sb;

		sb = &ares->data.spi_serial_bus;
		if (sb->type == ACPI_RESOURCE_SERIAL_TYPE_SPI) {
			spi->chip_select = sb->device_selection;
			spi->max_speed_hz = sb->connection_speed;

			if (sb->clock_phase == ACPI_SPI_SECOND_PHASE)
				spi->mode |= SPI_CPHA;
			if (sb->clock_polarity == ACPI_SPI_START_HIGH)
				spi->mode |= SPI_CPOL;
			if (sb->device_polarity == ACPI_SPI_ACTIVE_HIGH)
				spi->mode |= SPI_CS_HIGH;
		}
	} else if (spi->irq < 0) {
		struct resource r;

		if (acpi_dev_resource_interrupt(ares, 0, &r))
			spi->irq = r.start;
	}

	/* Always tell the ACPI core to skip this resource */
	return 1;
}

static acpi_status acpi_spi_add_device(acpi_handle handle, u32 level,
				       void *data, void **return_value)
{
	struct spi_master *master = data;
	struct list_head resource_list;
	struct acpi_device *adev;
	struct spi_device *spi;
	int ret;

	if (acpi_bus_get_device(handle, &adev))
		return AE_OK;
	if (acpi_bus_get_status(adev) || !adev->status.present)
		return AE_OK;

	spi = spi_alloc_device(master);
	if (!spi) {
		dev_err(&master->dev, "failed to allocate SPI device for %s\n",
			dev_name(&adev->dev));
		return AE_NO_MEMORY;
	}

	ACPI_COMPANION_SET(&spi->dev, adev);
	spi->irq = -1;

	INIT_LIST_HEAD(&resource_list);
	ret = acpi_dev_get_resources(adev, &resource_list,
				     acpi_spi_add_resource, spi);
	acpi_dev_free_resource_list(&resource_list);

	if (ret < 0 || !spi->max_speed_hz) {
		spi_dev_put(spi);
		return AE_OK;
	}

	adev->power.flags.ignore_parent = true;
	strlcpy(spi->modalias, acpi_device_hid(adev), sizeof(spi->modalias));
	if (spi_add_device(spi)) {
		adev->power.flags.ignore_parent = false;
		dev_err(&master->dev, "failed to add SPI device %s from ACPI\n",
			dev_name(&adev->dev));
		spi_dev_put(spi);
	}

	return AE_OK;
}

static void acpi_register_spi_devices(struct spi_master *master)
{
	acpi_status status;
	acpi_handle handle;

	handle = ACPI_HANDLE(master->dev.parent);
	if (!handle)
		return;

	status = acpi_walk_namespace(ACPI_TYPE_DEVICE, handle, 1,
				     acpi_spi_add_device, NULL,
				     master, NULL);
	if (ACPI_FAILURE(status))
		dev_warn(&master->dev, "failed to enumerate SPI slaves\n");
}
#else
static inline void acpi_register_spi_devices(struct spi_master *master) {}
#endif /* CONFIG_ACPI */

static void spi_master_release(struct device *dev)
{
	struct spi_master *master;

	master = container_of(dev, struct spi_master, dev);
	kfree(master);
}

static struct class spi_master_class = {
	.name		= "spi_master",
	.owner		= THIS_MODULE,
	.dev_release	= spi_master_release,
};



/**
 * spi_alloc_master - allocate SPI master controller
 * @dev: the controller, possibly using the platform_bus
 * @size: how much zeroed driver-private data to allocate; the pointer to this
 *	memory is in the driver_data field of the returned device,
 *	accessible with spi_master_get_devdata().
 * Context: can sleep
 *
 * This call is used only by SPI master controller drivers, which are the
 * only ones directly touching chip registers.  It's how they allocate
 * an spi_master structure, prior to calling spi_register_master().
 *
 * This must be called from context that can sleep.  It returns the SPI
 * master structure on success, else NULL.
 *
 * The caller is responsible for assigning the bus number and initializing
 * the master's methods before calling spi_register_master(); and (after errors
 * adding the device) calling spi_master_put() to prevent a memory leak.
 */
struct spi_master *spi_alloc_master(struct device *dev, unsigned size)
{
	struct spi_master	*master;

	if (!dev)
		return NULL;

	master = kzalloc(size + sizeof(*master), GFP_KERNEL);
	if (!master)
		return NULL;

	device_initialize(&master->dev);
	master->bus_num = -1;
	master->num_chipselect = 1;
	master->dev.class = &spi_master_class;
	master->dev.parent = get_device(dev);
	spi_master_set_devdata(master, &master[1]);

	return master;
}
EXPORT_SYMBOL_GPL(spi_alloc_master);

#ifdef CONFIG_OF
static int of_spi_register_master(struct spi_master *master)
{
	int nb, i, *cs;
	struct device_node *np = master->dev.of_node;

	if (!np)
		return 0;

	nb = of_gpio_named_count(np, "cs-gpios");
	master->num_chipselect = max_t(int, nb, master->num_chipselect);

	/* Return error only for an incorrectly formed cs-gpios property */
	if (nb == 0 || nb == -ENOENT)
		return 0;
	else if (nb < 0)
		return nb;

	cs = devm_kzalloc(&master->dev,
			  sizeof(int) * master->num_chipselect,
			  GFP_KERNEL);
	master->cs_gpios = cs;

	if (!master->cs_gpios)
		return -ENOMEM;

	for (i = 0; i < master->num_chipselect; i++)
		cs[i] = -ENOENT;

	for (i = 0; i < nb; i++)
		cs[i] = of_get_named_gpio(np, "cs-gpios", i);

	return 0;
}
#else
static int of_spi_register_master(struct spi_master *master)
{
	return 0;
}
#endif

/**
 * spi_register_master - register SPI master controller
 * @master: initialized master, originally from spi_alloc_master()
 * Context: can sleep
 *
 * SPI master controllers connect to their drivers using some non-SPI bus,
 * such as the platform bus.  The final stage of probe() in that code
 * includes calling spi_register_master() to hook up to this SPI bus glue.
 *
 * SPI controllers use board specific (often SOC specific) bus numbers,
 * and board-specific addressing for SPI devices combines those numbers
 * with chip select numbers.  Since SPI does not directly support dynamic
 * device identification, boards need configuration tables telling which
 * chip is at which address.
 *
 * This must be called from context that can sleep.  It returns zero on
 * success, else a negative error code (dropping the master's refcount).
 * After a successful return, the caller is responsible for calling
 * spi_unregister_master().
 */
int spi_register_master(struct spi_master *master)
{
	static atomic_t		dyn_bus_id = ATOMIC_INIT((1<<15) - 1);
	struct device		*dev = master->dev.parent;
	struct boardinfo	*bi;
	int			status = -ENODEV;
	int			dynamic = 0;

	if (!dev)
		return -ENODEV;

	status = of_spi_register_master(master);
	if (status)
		return status;

	/* even if it's just one always-selected device, there must
	 * be at least one chipselect
	 */
	if (master->num_chipselect == 0)
		return -EINVAL;

	if ((master->bus_num < 0) && master->dev.of_node)
		master->bus_num = of_alias_get_id(master->dev.of_node, "spi");

	/* convention:  dynamically assigned bus IDs count down from the max */
	if (master->bus_num < 0) {
		/* FIXME switch to an IDR based scheme, something like
		 * I2C now uses, so we can't run out of "dynamic" IDs
		 */
		master->bus_num = atomic_dec_return(&dyn_bus_id);
		dynamic = 1;
	}

	INIT_LIST_HEAD(&master->queue);
	spin_lock_init(&master->queue_lock);
	spin_lock_init(&master->bus_lock_spinlock);
	mutex_init(&master->bus_lock_mutex);
	master->bus_lock_flag = 0;
	init_completion(&master->xfer_completion);
	if (!master->max_dma_len)
		master->max_dma_len = INT_MAX;

	/* register the device, then userspace will see it.
	 * registration fails if the bus ID is in use.
	 */
	dev_set_name(&master->dev, "spi%u", master->bus_num);
	status = device_add(&master->dev);
	if (status < 0)
		goto done;
	dev_dbg(dev, "registered master %s%s\n", dev_name(&master->dev),
			dynamic ? " (dynamic)" : "");

	/* If we're using a queued driver, start the queue */
	if (master->transfer)
		dev_info(dev, "master is unqueued, this is deprecated\n");
	else {
		status = spi_master_initialize_queue(master);
		if (status) {
			device_del(&master->dev);
			goto done;
		}
	}

	mutex_lock(&board_lock);
	list_add_tail(&master->list, &spi_master_list);
	list_for_each_entry(bi, &board_list, list)
		spi_match_master_to_boardinfo(master, &bi->board_info);
	mutex_unlock(&board_lock);

	/* Register devices from the device tree and ACPI */
	of_register_spi_devices(master);
	acpi_register_spi_devices(master);
done:
	return status;
}
EXPORT_SYMBOL_GPL(spi_register_master);

static void devm_spi_unregister(struct device *dev, void *res)
{
	spi_unregister_master(*(struct spi_master **)res);
}

/**
 * dev_spi_register_master - register managed SPI master controller
 * @dev:    device managing SPI master
 * @master: initialized master, originally from spi_alloc_master()
 * Context: can sleep
 *
 * Register a SPI device as with spi_register_master() which will
 * automatically be unregister
 */
int devm_spi_register_master(struct device *dev, struct spi_master *master)
{
	struct spi_master **ptr;
	int ret;

	ptr = devres_alloc(devm_spi_unregister, sizeof(*ptr), GFP_KERNEL);
	if (!ptr)
		return -ENOMEM;

	ret = spi_register_master(master);
	if (!ret) {
		*ptr = master;
		devres_add(dev, ptr);
	} else {
		devres_free(ptr);
	}

	return ret;
}
EXPORT_SYMBOL_GPL(devm_spi_register_master);

static int __unregister(struct device *dev, void *null)
{
	spi_unregister_device(to_spi_device(dev));
	return 0;
}

/**
 * spi_unregister_master - unregister SPI master controller
 * @master: the master being unregistered
 * Context: can sleep
 *
 * This call is used only by SPI master controller drivers, which are the
 * only ones directly touching chip registers.
 *
 * This must be called from context that can sleep.
 */
void spi_unregister_master(struct spi_master *master)
{
	int dummy;

	if (master->queued) {
		if (spi_destroy_queue(master))
			dev_err(&master->dev, "queue remove failed\n");
	}

	mutex_lock(&board_lock);
	list_del(&master->list);
	mutex_unlock(&board_lock);

	dummy = device_for_each_child(&master->dev, NULL, __unregister);
	device_unregister(&master->dev);
}
EXPORT_SYMBOL_GPL(spi_unregister_master);

int spi_master_suspend(struct spi_master *master)
{
	int ret;

	/* Basically no-ops for non-queued masters */
	if (!master->queued)
		return 0;

	ret = spi_stop_queue(master);
	if (ret)
		dev_err(&master->dev, "queue stop failed\n");

	return ret;
}
EXPORT_SYMBOL_GPL(spi_master_suspend);

int spi_master_resume(struct spi_master *master)
{
	int ret;

	if (!master->queued)
		return 0;

	ret = spi_start_queue(master);
	if (ret)
		dev_err(&master->dev, "queue restart failed\n");

	return ret;
}
EXPORT_SYMBOL_GPL(spi_master_resume);

static int __spi_master_match(struct device *dev, const void *data)
{
	struct spi_master *m;
	const u16 *bus_num = data;

	m = container_of(dev, struct spi_master, dev);
	return m->bus_num == *bus_num;
}

/**
 * spi_busnum_to_master - look up master associated with bus_num
 * @bus_num: the master's bus number
 * Context: can sleep
 *
 * This call may be used with devices that are registered after
 * arch init time.  It returns a refcounted pointer to the relevant
 * spi_master (which the caller must release), or NULL if there is
 * no such master registered.
 */
struct spi_master *spi_busnum_to_master(u16 bus_num)
{
	struct device		*dev;
	struct spi_master	*master = NULL;

	dev = class_find_device(&spi_master_class, NULL, &bus_num,
				__spi_master_match);
	if (dev)
		master = container_of(dev, struct spi_master, dev);
	/* reference got in class_find_device */
	return master;
}
EXPORT_SYMBOL_GPL(spi_busnum_to_master);


/*-------------------------------------------------------------------------*/

/* Core methods for SPI master protocol drivers.  Some of the
 * other core methods are currently defined as inline functions.
 */

/**
 * spi_setup - setup SPI mode and clock rate
 * @spi: the device whose settings are being modified
 * Context: can sleep, and no requests are queued to the device
 *
 * SPI protocol drivers may need to update the transfer mode if the
 * device doesn't work with its default.  They may likewise need
 * to update clock rates or word sizes from initial values.  This function
 * changes those settings, and must be called from a context that can sleep.
 * Except for SPI_CS_HIGH, which takes effect immediately, the changes take
 * effect the next time the device is selected and data is transferred to
 * or from it.  When this function returns, the spi device is deselected.
 *
 * Note that this call will fail if the protocol driver specifies an option
 * that the underlying controller or its driver does not support.  For
 * example, not all hardware supports wire transfers using nine bit words,
 * LSB-first wire encoding, or active-high chipselects.
 */
int spi_setup(struct spi_device *spi)
{
	unsigned	bad_bits, ugly_bits;
	int		status = 0;

	/* check mode to prevent that DUAL and QUAD set at the same time
	 */
	if (((spi->mode & SPI_TX_DUAL) && (spi->mode & SPI_TX_QUAD)) ||
		((spi->mode & SPI_RX_DUAL) && (spi->mode & SPI_RX_QUAD))) {
		dev_err(&spi->dev,
		"setup: can not select dual and quad at the same time\n");
		return -EINVAL;
	}
	/* if it is SPI_3WIRE mode, DUAL and QUAD should be forbidden
	 */
	if ((spi->mode & SPI_3WIRE) && (spi->mode &
		(SPI_TX_DUAL | SPI_TX_QUAD | SPI_RX_DUAL | SPI_RX_QUAD)))
		return -EINVAL;
	/* help drivers fail *cleanly* when they need options
	 * that aren't supported with their current master
	 */
	bad_bits = spi->mode & ~spi->master->mode_bits;
	ugly_bits = bad_bits &
		    (SPI_TX_DUAL | SPI_TX_QUAD | SPI_RX_DUAL | SPI_RX_QUAD);
	if (ugly_bits) {
		dev_warn(&spi->dev,
			 "setup: ignoring unsupported mode bits %x\n",
			 ugly_bits);
		spi->mode &= ~ugly_bits;
		bad_bits &= ~ugly_bits;
	}
	if (bad_bits) {
		dev_err(&spi->dev, "setup: unsupported mode bits %x\n",
			bad_bits);
		return -EINVAL;
	}

	if (!spi->bits_per_word)
		spi->bits_per_word = 8;

	if (!spi->max_speed_hz)
		spi->max_speed_hz = spi->master->max_speed_hz;

	spi_set_cs(spi, false);

	if (spi->master->setup)
		status = spi->master->setup(spi);

	dev_dbg(&spi->dev, "setup mode %d, %s%s%s%s%u bits/w, %u Hz max --> %d\n",
			(int) (spi->mode & (SPI_CPOL | SPI_CPHA)),
			(spi->mode & SPI_CS_HIGH) ? "cs_high, " : "",
			(spi->mode & SPI_LSB_FIRST) ? "lsb, " : "",
			(spi->mode & SPI_3WIRE) ? "3wire, " : "",
			(spi->mode & SPI_LOOP) ? "loopback, " : "",
			spi->bits_per_word, spi->max_speed_hz,
			status);

	return status;
}
EXPORT_SYMBOL_GPL(spi_setup);

static int __spi_validate(struct spi_device *spi, struct spi_message *message)
{
	struct spi_master *master = spi->master;
	struct spi_transfer *xfer;
	int w_size;

	if (list_empty(&message->transfers))
		return -EINVAL;

	/* Half-duplex links include original MicroWire, and ones with
	 * only one data pin like SPI_3WIRE (switches direction) or where
	 * either MOSI or MISO is missing.  They can also be caused by
	 * software limitations.
	 */
	if ((master->flags & SPI_MASTER_HALF_DUPLEX)
			|| (spi->mode & SPI_3WIRE)) {
		unsigned flags = master->flags;

		list_for_each_entry(xfer, &message->transfers, transfer_list) {
			if (xfer->rx_buf && xfer->tx_buf)
				return -EINVAL;
			if ((flags & SPI_MASTER_NO_TX) && xfer->tx_buf)
				return -EINVAL;
			if ((flags & SPI_MASTER_NO_RX) && xfer->rx_buf)
				return -EINVAL;
		}
	}

	/**
	 * Set transfer bits_per_word and max speed as spi device default if
	 * it is not set for this transfer.
	 * Set transfer tx_nbits and rx_nbits as single transfer default
	 * (SPI_NBITS_SINGLE) if it is not set for this transfer.
	 */
	list_for_each_entry(xfer, &message->transfers, transfer_list) {
		message->frame_length += xfer->len;
		if (!xfer->bits_per_word)
			xfer->bits_per_word = spi->bits_per_word;

		if (!xfer->speed_hz)
			xfer->speed_hz = spi->max_speed_hz;

		if (master->max_speed_hz &&
		    xfer->speed_hz > master->max_speed_hz)
			xfer->speed_hz = master->max_speed_hz;

		if (master->bits_per_word_mask) {
			/* Only 32 bits fit in the mask */
			if (xfer->bits_per_word > 32)
				return -EINVAL;
			if (!(master->bits_per_word_mask &
					BIT(xfer->bits_per_word - 1)))
				return -EINVAL;
		}

		/*
		 * SPI transfer length should be multiple of SPI word size
		 * where SPI word size should be power-of-two multiple
		 */
		if (xfer->bits_per_word <= 8)
			w_size = 1;
		else if (xfer->bits_per_word <= 16)
			w_size = 2;
		else
			w_size = 4;

		/* No partial transfers accepted */
		if (xfer->len % w_size)
			return -EINVAL;

		if (xfer->speed_hz && master->min_speed_hz &&
		    xfer->speed_hz < master->min_speed_hz)
			return -EINVAL;

		if (xfer->tx_buf && !xfer->tx_nbits)
			xfer->tx_nbits = SPI_NBITS_SINGLE;
		if (xfer->rx_buf && !xfer->rx_nbits)
			xfer->rx_nbits = SPI_NBITS_SINGLE;
		/* check transfer tx/rx_nbits:
		 * 1. check the value matches one of single, dual and quad
		 * 2. check tx/rx_nbits match the mode in spi_device
		 */
		if (xfer->tx_buf) {
			if (xfer->tx_nbits != SPI_NBITS_SINGLE &&
				xfer->tx_nbits != SPI_NBITS_DUAL &&
				xfer->tx_nbits != SPI_NBITS_QUAD)
				return -EINVAL;
			if ((xfer->tx_nbits == SPI_NBITS_DUAL) &&
				!(spi->mode & (SPI_TX_DUAL | SPI_TX_QUAD)))
				return -EINVAL;
			if ((xfer->tx_nbits == SPI_NBITS_QUAD) &&
				!(spi->mode & SPI_TX_QUAD))
				return -EINVAL;
		}
		/* check transfer rx_nbits */
		if (xfer->rx_buf) {
			if (xfer->rx_nbits != SPI_NBITS_SINGLE &&
				xfer->rx_nbits != SPI_NBITS_DUAL &&
				xfer->rx_nbits != SPI_NBITS_QUAD)
				return -EINVAL;
			if ((xfer->rx_nbits == SPI_NBITS_DUAL) &&
				!(spi->mode & (SPI_RX_DUAL | SPI_RX_QUAD)))
				return -EINVAL;
			if ((xfer->rx_nbits == SPI_NBITS_QUAD) &&
				!(spi->mode & SPI_RX_QUAD))
				return -EINVAL;
		}
	}

	message->status = -EINPROGRESS;

	return 0;
}

static int __spi_async(struct spi_device *spi, struct spi_message *message)
{
	struct spi_master *master = spi->master;

	message->spi = spi;

	trace_spi_message_submit(message);

	return master->transfer(spi, message);
}

/**
 * spi_async - asynchronous SPI transfer
 * @spi: device with which data will be exchanged
 * @message: describes the data transfers, including completion callback
 * Context: any (irqs may be blocked, etc)
 *
 * This call may be used in_irq and other contexts which can't sleep,
 * as well as from task contexts which can sleep.
 *
 * The completion callback is invoked in a context which can't sleep.
 * Before that invocation, the value of message->status is undefined.
 * When the callback is issued, message->status holds either zero (to
 * indicate complete success) or a negative error code.  After that
 * callback returns, the driver which issued the transfer request may
 * deallocate the associated memory; it's no longer in use by any SPI
 * core or controller driver code.
 *
 * Note that although all messages to a spi_device are handled in
 * FIFO order, messages may go to different devices in other orders.
 * Some device might be higher priority, or have various "hard" access
 * time requirements, for example.
 *
 * On detection of any fault during the transfer, processing of
 * the entire message is aborted, and the device is deselected.
 * Until returning from the associated message completion callback,
 * no other spi_message queued to that device will be processed.
 * (This rule applies equally to all the synchronous transfer calls,
 * which are wrappers around this core asynchronous primitive.)
 */
int spi_async(struct spi_device *spi, struct spi_message *message)
#if defined(CONFIG_BCM_KF_SPI) && !defined(CONFIG_SPI_BCM63XX_HSSPI)
{
	struct spi_master *master = spi->master;
	unsigned long flags;

	/* holding the spinlock and disabling irqs for the duration of the transfer is problematic
	 ��the controller driver manages the locking so call __spi_async without the lock */

	spin_lock_irqsave(&master->bus_lock_spinlock, flags);
	if (master->bus_lock_flag){
		spin_unlock_irqrestore(&master->bus_lock_spinlock, flags);
		return -EBUSY;
	}
	spin_unlock_irqrestore(&master->bus_lock_spinlock, flags);

	return __spi_async(spi, message);
}
#else
{
	struct spi_master *master = spi->master;
	int ret;
	unsigned long flags;

	ret = __spi_validate(spi, message);
	if (ret != 0)
		return ret;

	spin_lock_irqsave(&master->bus_lock_spinlock, flags);

	if (master->bus_lock_flag)
		ret = -EBUSY;
	else
		ret = __spi_async(spi, message);

	spin_unlock_irqrestore(&master->bus_lock_spinlock, flags);

	return ret;
}
#endif
EXPORT_SYMBOL_GPL(spi_async);

/**
 * spi_async_locked - version of spi_async with exclusive bus usage
 * @spi: device with which data will be exchanged
 * @message: describes the data transfers, including completion callback
 * Context: any (irqs may be blocked, etc)
 *
 * This call may be used in_irq and other contexts which can't sleep,
 * as well as from task contexts which can sleep.
 *
 * The completion callback is invoked in a context which can't sleep.
 * Before that invocation, the value of message->status is undefined.
 * When the callback is issued, message->status holds either zero (to
 * indicate complete success) or a negative error code.  After that
 * callback returns, the driver which issued the transfer request may
 * deallocate the associated memory; it's no longer in use by any SPI
 * core or controller driver code.
 *
 * Note that although all messages to a spi_device are handled in
 * FIFO order, messages may go to different devices in other orders.
 * Some device might be higher priority, or have various "hard" access
 * time requirements, for example.
 *
 * On detection of any fault during the transfer, processing of
 * the entire message is aborted, and the device is deselected.
 * Until returning from the associated message completion callback,
 * no other spi_message queued to that device will be processed.
 * (This rule applies equally to all the synchronous transfer calls,
 * which are wrappers around this core asynchronous primitive.)
 */
int spi_async_locked(struct spi_device *spi, struct spi_message *message)
{
	struct spi_master *master = spi->master;
	int ret;
	unsigned long flags;

	ret = __spi_validate(spi, message);
	if (ret != 0)
		return ret;

	spin_lock_irqsave(&master->bus_lock_spinlock, flags);

	ret = __spi_async(spi, message);

	spin_unlock_irqrestore(&master->bus_lock_spinlock, flags);

	return ret;

}
EXPORT_SYMBOL_GPL(spi_async_locked);


/*-------------------------------------------------------------------------*/

/* Utility methods for SPI master protocol drivers, layered on
 * top of the core.  Some other utility methods are defined as
 * inline functions.
 */

static void spi_complete(void *arg)
{
	complete(arg);
}

static int __spi_sync(struct spi_device *spi, struct spi_message *message,
		      int bus_locked)
{
	DECLARE_COMPLETION_ONSTACK(done);
	int status;
	struct spi_master *master = spi->master;
	unsigned long flags;

	status = __spi_validate(spi, message);
	if (status != 0)
		return status;

	message->complete = spi_complete;
	message->context = &done;
	message->spi = spi;

	if (!bus_locked)
		mutex_lock(&master->bus_lock_mutex);

	/* If we're not using the legacy transfer method then we will
	 * try to transfer in the calling context so special case.
	 * This code would be less tricky if we could remove the
	 * support for driver implemented message queues.
	 */
	if (master->transfer == spi_queued_transfer) {
		spin_lock_irqsave(&master->bus_lock_spinlock, flags);

		trace_spi_message_submit(message);

		status = __spi_queued_transfer(spi, message, false);

		spin_unlock_irqrestore(&master->bus_lock_spinlock, flags);
	} else {
		status = spi_async_locked(spi, message);
	}

	if (!bus_locked)
		mutex_unlock(&master->bus_lock_mutex);

	if (status == 0) {
		/* Push out the messages in the calling context if we
		 * can.
		 */
		if (master->transfer == spi_queued_transfer)
			__spi_pump_messages(master, false);

		wait_for_completion(&done);
		status = message->status;
	}
	message->context = NULL;
	return status;
}

/**
 * spi_sync - blocking/synchronous SPI data transfers
 * @spi: device with which data will be exchanged
 * @message: describes the data transfers
 * Context: can sleep
 *
 * This call may only be used from a context that may sleep.  The sleep
 * is non-interruptible, and has no timeout.  Low-overhead controller
 * drivers may DMA directly into and out of the message buffers.
 *
 * Note that the SPI device's chip select is active during the message,
 * and then is normally disabled between messages.  Drivers for some
 * frequently-used devices may want to minimize costs of selecting a chip,
 * by leaving it selected in anticipation that the next message will go
 * to the same chip.  (That may increase power usage.)
 *
 * Also, the caller is guaranteeing that the memory associated with the
 * message will not be freed before this call returns.
 *
 * It returns zero on success, else a negative error code.
 */
int spi_sync(struct spi_device *spi, struct spi_message *message)
{
	return __spi_sync(spi, message, 0);
}
EXPORT_SYMBOL_GPL(spi_sync);

/**
 * spi_sync_locked - version of spi_sync with exclusive bus usage
 * @spi: device with which data will be exchanged
 * @message: describes the data transfers
 * Context: can sleep
 *
 * This call may only be used from a context that may sleep.  The sleep
 * is non-interruptible, and has no timeout.  Low-overhead controller
 * drivers may DMA directly into and out of the message buffers.
 *
 * This call should be used by drivers that require exclusive access to the
 * SPI bus. It has to be preceded by a spi_bus_lock call. The SPI bus must
 * be released by a spi_bus_unlock call when the exclusive access is over.
 *
 * It returns zero on success, else a negative error code.
 */
int spi_sync_locked(struct spi_device *spi, struct spi_message *message)
{
	return __spi_sync(spi, message, 1);
}
EXPORT_SYMBOL_GPL(spi_sync_locked);

/**
 * spi_bus_lock - obtain a lock for exclusive SPI bus usage
 * @master: SPI bus master that should be locked for exclusive bus access
 * Context: can sleep
 *
 * This call may only be used from a context that may sleep.  The sleep
 * is non-interruptible, and has no timeout.
 *
 * This call should be used by drivers that require exclusive access to the
 * SPI bus. The SPI bus must be released by a spi_bus_unlock call when the
 * exclusive access is over. Data transfer must be done by spi_sync_locked
 * and spi_async_locked calls when the SPI bus lock is held.
 *
 * It returns zero on success, else a negative error code.
 */
int spi_bus_lock(struct spi_master *master)
{
	unsigned long flags;

	mutex_lock(&master->bus_lock_mutex);

	spin_lock_irqsave(&master->bus_lock_spinlock, flags);
	master->bus_lock_flag = 1;
	spin_unlock_irqrestore(&master->bus_lock_spinlock, flags);

	/* mutex remains locked until spi_bus_unlock is called */

	return 0;
}
EXPORT_SYMBOL_GPL(spi_bus_lock);

/**
 * spi_bus_unlock - release the lock for exclusive SPI bus usage
 * @master: SPI bus master that was locked for exclusive bus access
 * Context: can sleep
 *
 * This call may only be used from a context that may sleep.  The sleep
 * is non-interruptible, and has no timeout.
 *
 * This call releases an SPI bus lock previously obtained by an spi_bus_lock
 * call.
 *
 * It returns zero on success, else a negative error code.
 */
int spi_bus_unlock(struct spi_master *master)
{
	master->bus_lock_flag = 0;

	mutex_unlock(&master->bus_lock_mutex);

	return 0;
}
EXPORT_SYMBOL_GPL(spi_bus_unlock);

/* portable code must never pass more than 32 bytes */
#define	SPI_BUFSIZ	max(32, SMP_CACHE_BYTES)

static u8	*buf;

/**
 * spi_write_then_read - SPI synchronous write followed by read
 * @spi: device with which data will be exchanged
 * @txbuf: data to be written (need not be dma-safe)
 * @n_tx: size of txbuf, in bytes
 * @rxbuf: buffer into which data will be read (need not be dma-safe)
 * @n_rx: size of rxbuf, in bytes
 * Context: can sleep
 *
 * This performs a half duplex MicroWire style transaction with the
 * device, sending txbuf and then reading rxbuf.  The return value
 * is zero for success, else a negative errno status code.
 * This call may only be used from a context that may sleep.
 *
 * Parameters to this routine are always copied using a small buffer;
 * portable code should never use this for more than 32 bytes.
 * Performance-sensitive or bulk transfer code should instead use
 * spi_{async,sync}() calls with dma-safe buffers.
 */
int spi_write_then_read(struct spi_device *spi,
		const void *txbuf, unsigned n_tx,
		void *rxbuf, unsigned n_rx)
{
	static DEFINE_MUTEX(lock);

	int			status;
	struct spi_message	message;
	struct spi_transfer	x[2];
	u8			*local_buf;

	/* Use preallocated DMA-safe buffer if we can.  We can't avoid
	 * copying here, (as a pure convenience thing), but we can
	 * keep heap costs out of the hot path unless someone else is
	 * using the pre-allocated buffer or the transfer is too large.
	 */
	if ((n_tx + n_rx) > SPI_BUFSIZ || !mutex_trylock(&lock)) {
		local_buf = kmalloc(max((unsigned)SPI_BUFSIZ, n_tx + n_rx),
				    GFP_KERNEL | GFP_DMA);
		if (!local_buf)
			return -ENOMEM;
	} else {
		local_buf = buf;
	}

	spi_message_init(&message);
	memset(x, 0, sizeof(x));
	if (n_tx) {
		x[0].len = n_tx;
		spi_message_add_tail(&x[0], &message);
	}
	if (n_rx) {
		x[1].len = n_rx;
		spi_message_add_tail(&x[1], &message);
	}

	memcpy(local_buf, txbuf, n_tx);
	x[0].tx_buf = local_buf;
	x[1].rx_buf = local_buf + n_tx;

	/* do the i/o */
	status = spi_sync(spi, &message);
	if (status == 0)
		memcpy(rxbuf, x[1].rx_buf, n_rx);

	if (x[0].tx_buf == buf)
		mutex_unlock(&lock);
	else
		kfree(local_buf);

	return status;
}
EXPORT_SYMBOL_GPL(spi_write_then_read);

/*-------------------------------------------------------------------------*/

#if IS_ENABLED(CONFIG_OF_DYNAMIC)
static int __spi_of_device_match(struct device *dev, void *data)
{
	return dev->of_node == data;
}

/* must call put_device() when done with returned spi_device device */
static struct spi_device *of_find_spi_device_by_node(struct device_node *node)
{
	struct device *dev = bus_find_device(&spi_bus_type, NULL, node,
						__spi_of_device_match);
	return dev ? to_spi_device(dev) : NULL;
}

static int __spi_of_master_match(struct device *dev, const void *data)
{
	return dev->of_node == data;
}

/* the spi masters are not using spi_bus, so we find it with another way */
static struct spi_master *of_find_spi_master_by_node(struct device_node *node)
{
	struct device *dev;

	dev = class_find_device(&spi_master_class, NULL, node,
				__spi_of_master_match);
	if (!dev)
		return NULL;

	/* reference got in class_find_device */
	return container_of(dev, struct spi_master, dev);
}

static int of_spi_notify(struct notifier_block *nb, unsigned long action,
			 void *arg)
{
	struct of_reconfig_data *rd = arg;
	struct spi_master *master;
	struct spi_device *spi;

	switch (of_reconfig_get_state_change(action, arg)) {
	case OF_RECONFIG_CHANGE_ADD:
		master = of_find_spi_master_by_node(rd->dn->parent);
		if (master == NULL)
			return NOTIFY_OK;	/* not for us */

		spi = of_register_spi_device(master, rd->dn);
		put_device(&master->dev);

		if (IS_ERR(spi)) {
			pr_err("%s: failed to create for '%s'\n",
					__func__, rd->dn->full_name);
			return notifier_from_errno(PTR_ERR(spi));
		}
		break;

	case OF_RECONFIG_CHANGE_REMOVE:
		/* find our device by node */
		spi = of_find_spi_device_by_node(rd->dn);
		if (spi == NULL)
			return NOTIFY_OK;	/* no? not meant for us */

		/* unregister takes one ref away */
		spi_unregister_device(spi);

		/* and put the reference of the find */
		put_device(&spi->dev);
		break;
	}

	return NOTIFY_OK;
}

static struct notifier_block spi_of_notifier = {
	.notifier_call = of_spi_notify,
};
#else /* IS_ENABLED(CONFIG_OF_DYNAMIC) */
extern struct notifier_block spi_of_notifier;
#endif /* IS_ENABLED(CONFIG_OF_DYNAMIC) */

static int __init spi_init(void)
{
	int	status;

	buf = kmalloc(SPI_BUFSIZ, GFP_KERNEL);
	if (!buf) {
		status = -ENOMEM;
		goto err0;
	}

	status = bus_register(&spi_bus_type);
	if (status < 0)
		goto err1;

	status = class_register(&spi_master_class);
	if (status < 0)
		goto err2;

	if (IS_ENABLED(CONFIG_OF_DYNAMIC))
		WARN_ON(of_reconfig_notifier_register(&spi_of_notifier));

	return 0;

err2:
	bus_unregister(&spi_bus_type);
err1:
	kfree(buf);
	buf = NULL;
err0:
	return status;
}

/* board_info is normally registered in arch_initcall(),
 * but even essential drivers wait till later
 *
 * REVISIT only boardinfo really needs static linking. the rest (device and
 * driver registration) _could_ be dynamically linked (modular) ... costs
 * include needing to have boardinfo data structures be much more public.
 */
postcore_initcall(spi_init);

