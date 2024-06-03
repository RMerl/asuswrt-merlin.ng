// SPDX-License-Identifier: GPL-2.0
/**
 * udc-core.c - Core UDC Framework
 *
 * Copyright (C) 2015 Texas Instruments Incorporated - http://www.ti.com
 *
 * Author: Felipe Balbi <balbi@ti.com>
 *
 * Taken from Linux Kernel v3.19-rc1 (drivers/usb/gadget/udc-core.c) and ported
 * to uboot.
 *
 * commit 02e8c96627 : usb: gadget: udc: core: prepend udc_attach_driver with
 *		       usb_
 */

#include <linux/compat.h>
#include <malloc.h>
#include <asm/cache.h>
#include <asm/dma-mapping.h>
#include <common.h>
#include <dm.h>
#include <dm/device-internal.h>
#include <linux/usb/ch9.h>
#include <linux/usb/gadget.h>

/**
 * struct usb_udc - describes one usb device controller
 * @driver - the gadget driver pointer. For use by the class code
 * @dev - the child device to the actual controller
 * @gadget - the gadget. For use by the class code
 * @list - for use by the udc class driver
 *
 * This represents the internal data structure which is used by the UDC-class
 * to hold information about udc driver and gadget together.
 */
struct usb_udc {
	struct usb_gadget_driver	*driver;
	struct usb_gadget		*gadget;
	struct device			dev;
	struct list_head		list;
};

static struct class *udc_class;
static LIST_HEAD(udc_list);
DEFINE_MUTEX(udc_lock);

/* ------------------------------------------------------------------------- */

int usb_gadget_map_request(struct usb_gadget *gadget,
		struct usb_request *req, int is_in)
{
	if (req->length == 0)
		return 0;

	req->dma = dma_map_single(req->buf, req->length,
				  is_in ? DMA_TO_DEVICE : DMA_FROM_DEVICE);

	return 0;
}
EXPORT_SYMBOL_GPL(usb_gadget_map_request);

void usb_gadget_unmap_request(struct usb_gadget *gadget,
		struct usb_request *req, int is_in)
{
	if (req->length == 0)
		return;

	dma_unmap_single((void *)(uintptr_t)req->dma, req->length,
			 is_in ? DMA_TO_DEVICE : DMA_FROM_DEVICE);
}
EXPORT_SYMBOL_GPL(usb_gadget_unmap_request);

/* ------------------------------------------------------------------------- */

/**
 * usb_gadget_giveback_request - give the request back to the gadget layer
 * Context: in_interrupt()
 *
 * This is called by device controller drivers in order to return the
 * completed request back to the gadget layer.
 */
void usb_gadget_giveback_request(struct usb_ep *ep,
		struct usb_request *req)
{
	req->complete(ep, req);
}
EXPORT_SYMBOL_GPL(usb_gadget_giveback_request);

/* ------------------------------------------------------------------------- */

void usb_gadget_set_state(struct usb_gadget *gadget,
		enum usb_device_state state)
{
	gadget->state = state;
}
EXPORT_SYMBOL_GPL(usb_gadget_set_state);

/* ------------------------------------------------------------------------- */

/**
 * usb_gadget_udc_reset - notifies the udc core that bus reset occurs
 * @gadget: The gadget which bus reset occurs
 * @driver: The gadget driver we want to notify
 *
 * If the udc driver has bus reset handler, it needs to call this when the bus
 * reset occurs, it notifies the gadget driver that the bus reset occurs as
 * well as updates gadget state.
 */
void usb_gadget_udc_reset(struct usb_gadget *gadget,
		struct usb_gadget_driver *driver)
{
	driver->reset(gadget);
	usb_gadget_set_state(gadget, USB_STATE_DEFAULT);
}
EXPORT_SYMBOL_GPL(usb_gadget_udc_reset);

/**
 * usb_gadget_udc_start - tells usb device controller to start up
 * @udc: The UDC to be started
 *
 * This call is issued by the UDC Class driver when it's about
 * to register a gadget driver to the device controller, before
 * calling gadget driver's bind() method.
 *
 * It allows the controller to be powered off until strictly
 * necessary to have it powered on.
 *
 * Returns zero on success, else negative errno.
 */
static inline int usb_gadget_udc_start(struct usb_udc *udc)
{
	return udc->gadget->ops->udc_start(udc->gadget, udc->driver);
}

/**
 * usb_gadget_udc_stop - tells usb device controller we don't need it anymore
 * @gadget: The device we want to stop activity
 * @driver: The driver to unbind from @gadget
 *
 * This call is issued by the UDC Class driver after calling
 * gadget driver's unbind() method.
 *
 * The details are implementation specific, but it can go as
 * far as powering off UDC completely and disable its data
 * line pullups.
 */
static inline void usb_gadget_udc_stop(struct usb_udc *udc)
{
	udc->gadget->ops->udc_stop(udc->gadget);
}

/**
 * usb_udc_release - release the usb_udc struct
 * @dev: the dev member within usb_udc
 *
 * This is called by driver's core in order to free memory once the last
 * reference is released.
 */
static void usb_udc_release(struct device *dev)
{
	struct usb_udc *udc;

	udc = container_of(dev, struct usb_udc, dev);
	kfree(udc);
}

/**
 * usb_add_gadget_udc_release - adds a new gadget to the udc class driver list
 * @parent: the parent device to this udc. Usually the controller driver's
 * device.
 * @gadget: the gadget to be added to the list.
 * @release: a gadget release function.
 *
 * Returns zero on success, negative errno otherwise.
 */
int usb_add_gadget_udc_release(struct device *parent, struct usb_gadget *gadget,
		void (*release)(struct device *dev))
{
	struct usb_udc		*udc;
	int			ret = -ENOMEM;

	udc = kzalloc(sizeof(*udc), GFP_KERNEL);
	if (!udc)
		goto err1;

	dev_set_name(&gadget->dev, "gadget");
	gadget->dev.parent = parent;

	udc->dev.release = usb_udc_release;
	udc->dev.class = udc_class;
	udc->dev.parent = parent;

	udc->gadget = gadget;

	mutex_lock(&udc_lock);
	list_add_tail(&udc->list, &udc_list);

	usb_gadget_set_state(gadget, USB_STATE_NOTATTACHED);

	mutex_unlock(&udc_lock);

	return 0;

err1:
	return ret;
}
EXPORT_SYMBOL_GPL(usb_add_gadget_udc_release);

/**
 * usb_add_gadget_udc - adds a new gadget to the udc class driver list
 * @parent: the parent device to this udc. Usually the controller
 * driver's device.
 * @gadget: the gadget to be added to the list
 *
 * Returns zero on success, negative errno otherwise.
 */
int usb_add_gadget_udc(struct device *parent, struct usb_gadget *gadget)
{
	return usb_add_gadget_udc_release(parent, gadget, NULL);
}
EXPORT_SYMBOL_GPL(usb_add_gadget_udc);

static void usb_gadget_remove_driver(struct usb_udc *udc)
{
	dev_dbg(&udc->dev, "unregistering UDC driver [%s]\n",
			udc->driver->function);

	usb_gadget_disconnect(udc->gadget);
	udc->driver->disconnect(udc->gadget);
	udc->driver->unbind(udc->gadget);
	usb_gadget_udc_stop(udc);

	udc->driver = NULL;
}

/**
 * usb_del_gadget_udc - deletes @udc from udc_list
 * @gadget: the gadget to be removed.
 *
 * This, will call usb_gadget_unregister_driver() if
 * the @udc is still busy.
 */
void usb_del_gadget_udc(struct usb_gadget *gadget)
{
	struct usb_udc		*udc = NULL;

	mutex_lock(&udc_lock);
	list_for_each_entry(udc, &udc_list, list)
		if (udc->gadget == gadget)
			goto found;

	dev_err(gadget->dev.parent, "gadget not registered.\n");
	mutex_unlock(&udc_lock);

	return;

found:
	dev_vdbg(gadget->dev.parent, "unregistering gadget\n");

	list_del(&udc->list);
	mutex_unlock(&udc_lock);

	if (udc->driver)
		usb_gadget_remove_driver(udc);
}
EXPORT_SYMBOL_GPL(usb_del_gadget_udc);

/* ------------------------------------------------------------------------- */

static int udc_bind_to_driver(struct usb_udc *udc, struct usb_gadget_driver *driver)
{
	int ret;

	dev_dbg(&udc->dev, "registering UDC driver [%s]\n",
			driver->function);

	udc->driver = driver;

	ret = driver->bind(udc->gadget);
	if (ret)
		goto err1;
	ret = usb_gadget_udc_start(udc);
	if (ret) {
		driver->unbind(udc->gadget);
		goto err1;
	}
	usb_gadget_connect(udc->gadget);

	return 0;
err1:
	if (ret != -EISNAM)
		dev_err(&udc->dev, "failed to start %s: %d\n",
			udc->driver->function, ret);
	udc->driver = NULL;
	return ret;
}

int usb_gadget_probe_driver(struct usb_gadget_driver *driver)
{
	struct usb_udc		*udc = NULL;
	int			ret;

	if (!driver || !driver->bind || !driver->setup)
		return -EINVAL;

	mutex_lock(&udc_lock);
	list_for_each_entry(udc, &udc_list, list) {
		/* For now we take the first one */
		if (!udc->driver)
			goto found;
	}

	printf("couldn't find an available UDC\n");
	mutex_unlock(&udc_lock);
	return -ENODEV;
found:
	ret = udc_bind_to_driver(udc, driver);
	mutex_unlock(&udc_lock);
	return ret;
}
EXPORT_SYMBOL_GPL(usb_gadget_probe_driver);

int usb_gadget_register_driver(struct usb_gadget_driver *driver)
{
	return usb_gadget_probe_driver(driver);
}
EXPORT_SYMBOL_GPL(usb_gadget_register_driver);

int usb_gadget_unregister_driver(struct usb_gadget_driver *driver)
{
	struct usb_udc		*udc = NULL;
	int			ret = -ENODEV;

	if (!driver || !driver->unbind)
		return -EINVAL;

	mutex_lock(&udc_lock);
	list_for_each_entry(udc, &udc_list, list)
		if (udc->driver == driver) {
			usb_gadget_remove_driver(udc);
			usb_gadget_set_state(udc->gadget,
					USB_STATE_NOTATTACHED);
			ret = 0;
			break;
		}

	mutex_unlock(&udc_lock);
	return ret;
}
EXPORT_SYMBOL_GPL(usb_gadget_unregister_driver);

MODULE_DESCRIPTION("UDC Framework");
MODULE_AUTHOR("Felipe Balbi <balbi@ti.com>");
MODULE_LICENSE("GPL v2");
