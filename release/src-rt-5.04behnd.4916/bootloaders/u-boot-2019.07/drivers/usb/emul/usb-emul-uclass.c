// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2015 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <common.h>
#include <dm.h>
#include <usb.h>
#include <dm/device-internal.h>

static int copy_to_unicode(char *buff, int length, const char *str)
{
	int ptr;
	int i;

	if (length < 2)
		return 0;
	buff[1] = USB_DT_STRING;
	for (ptr = 2, i = 0; ptr + 1 < length && *str; i++, ptr += 2) {
		buff[ptr] = str[i];
		buff[ptr + 1] = 0;
	}
	buff[0] = ptr;

	return ptr;
}

static int usb_emul_get_string(struct usb_string *strings, int index,
			       char *buff, int length)
{
	if (index == 0) {
		char *desc = buff;

		desc[0] = 4;
		desc[1] = USB_DT_STRING;
		desc[2] = 0x09;
		desc[3] = 0x14;
		return 4;
	} else if (strings) {
		struct usb_string *ptr;

		for (ptr = strings; ptr->s; ptr++) {
			if (ptr->id == index)
				return copy_to_unicode(buff, length, ptr->s);
		}
	}

	return -EINVAL;
}

struct usb_generic_descriptor **usb_emul_find_descriptor(
		struct usb_generic_descriptor **ptr, int type, int index)
{
	debug("%s: type=%x, index=%d\n", __func__, type, index);
	for (; *ptr; ptr++) {
		if ((*ptr)->bDescriptorType != type)
			continue;
		switch (type) {
		case USB_DT_CONFIG: {
			struct usb_config_descriptor *cdesc;

			cdesc = (struct usb_config_descriptor *)*ptr;
			if (cdesc && cdesc->bConfigurationValue == index)
				return ptr;
			break;
		}
		default:
			return ptr;
		}
	}
	debug("%s: config ptr=%p\n", __func__, *ptr);

	return ptr;
}

static int usb_emul_get_descriptor(struct usb_dev_platdata *plat, int value,
				   void *buffer, int length)
{
	struct usb_generic_descriptor **ptr;
	int type = value >> 8;
	int index = value & 0xff;
	int upto, todo;

	debug("%s: type=%d, index=%d, plat=%p\n", __func__, type, index, plat);
	if (type == USB_DT_STRING) {
		return usb_emul_get_string(plat->strings, index, buffer,
					   length);
	}

	ptr = usb_emul_find_descriptor(plat->desc_list, type, index);
	if (!ptr) {
		debug("%s: Could not find descriptor type %d, index %d\n",
		      __func__, type, index);
		return -ENOENT;
	}
	for (upto = 0; *ptr && upto < length; ptr++, upto += todo) {
		todo = min(length - upto, (int)(*ptr)->bLength);

		memcpy(buffer + upto, *ptr, todo);
	}

	return upto ? upto : length ? -EIO : 0;
}

static int usb_emul_find_devnum(int devnum, int port1, struct udevice **emulp)
{
	struct udevice *dev;
	struct uclass *uc;
	int ret;

	*emulp = NULL;
	ret = uclass_get(UCLASS_USB_EMUL, &uc);
	if (ret)
		return ret;
	uclass_foreach_dev(dev, uc) {
		struct usb_dev_platdata *udev = dev_get_parent_platdata(dev);

		/*
		 * devnum is initialzied to zero at the beginning of the
		 * enumeration process in usb_setup_device(). At this
		 * point, udev->devnum has not been assigned to any valid
		 * USB address either, so we can't rely on the comparison
		 * result between udev->devnum and devnum to select an
		 * emulator device.
		 */
		if (!devnum) {
			struct usb_emul_platdata *plat;

			/*
			 * If the parent is sandbox USB controller, we are
			 * the root hub. And there is only one root hub
			 * in the system.
			 */
			if (device_get_uclass_id(dev->parent) == UCLASS_USB) {
				debug("%s: Found emulator '%s'\n",
				      __func__, dev->name);
				*emulp = dev;
				return 0;
			}

			plat = dev_get_uclass_platdata(dev);
			if (plat->port1 == port1) {
				debug("%s: Found emulator '%s', port %d\n",
				      __func__, dev->name, port1);
				*emulp = dev;
				return 0;
			}
		} else if (udev->devnum == devnum) {
			debug("%s: Found emulator '%s', addr %d\n", __func__,
			      dev->name, udev->devnum);
			*emulp = dev;
			return 0;
		}
	}

	debug("%s: No emulator found, addr %d\n", __func__, devnum);
	return -ENOENT;
}

int usb_emul_find(struct udevice *bus, ulong pipe, int port1,
		  struct udevice **emulp)
{
	int devnum = usb_pipedevice(pipe);

	return usb_emul_find_devnum(devnum, port1, emulp);
}

int usb_emul_find_for_dev(struct udevice *dev, struct udevice **emulp)
{
	struct usb_dev_platdata *udev = dev_get_parent_platdata(dev);

	return usb_emul_find_devnum(udev->devnum, 0, emulp);
}

int usb_emul_control(struct udevice *emul, struct usb_device *udev,
		     unsigned long pipe, void *buffer, int length,
		     struct devrequest *setup)
{
	struct dm_usb_ops *ops = usb_get_emul_ops(emul);
	struct usb_dev_platdata *plat;
	int ret;

	/* We permit getting the descriptor before we are probed */
	plat = dev_get_parent_platdata(emul);
	if (!ops->control)
		return -ENOSYS;
	debug("%s: dev=%s\n", __func__, emul->name);
	if (pipe == usb_rcvctrlpipe(udev, 0)) {
		switch (setup->request) {
		case USB_REQ_GET_DESCRIPTOR: {
			return usb_emul_get_descriptor(plat, setup->value,
						       buffer, length);
		}
		default:
			ret = device_probe(emul);
			if (ret)
				return ret;
			return ops->control(emul, udev, pipe, buffer, length,
					    setup);
		}
	} else if (pipe == usb_snddefctrl(udev)) {
		switch (setup->request) {
		case USB_REQ_SET_ADDRESS:
			debug("   ** set address %s %d\n", emul->name,
			      setup->value);
			plat->devnum = setup->value;
			return 0;
		default:
			debug("requestsend =%x\n", setup->request);
			break;
		}
	} else if (pipe == usb_sndctrlpipe(udev, 0)) {
		switch (setup->request) {
		case USB_REQ_SET_CONFIGURATION:
			plat->configno = setup->value;
			return 0;
		default:
			ret = device_probe(emul);
			if (ret)
				return ret;
			return ops->control(emul, udev, pipe, buffer, length,
					    setup);
		}
	}
	debug("pipe=%lx\n", pipe);

	return -EIO;
}

int usb_emul_bulk(struct udevice *emul, struct usb_device *udev,
		  unsigned long pipe, void *buffer, int length)
{
	struct dm_usb_ops *ops = usb_get_emul_ops(emul);
	int ret;

	/* We permit getting the descriptor before we are probed */
	if (!ops->bulk)
		return -ENOSYS;
	debug("%s: dev=%s\n", __func__, emul->name);
	ret = device_probe(emul);
	if (ret)
		return ret;
	return ops->bulk(emul, udev, pipe, buffer, length);
}

int usb_emul_int(struct udevice *emul, struct usb_device *udev,
		  unsigned long pipe, void *buffer, int length, int interval)
{
	struct dm_usb_ops *ops = usb_get_emul_ops(emul);

	if (!ops->interrupt)
		return -ENOSYS;
	debug("%s: dev=%s\n", __func__, emul->name);

	return ops->interrupt(emul, udev, pipe, buffer, length, interval);
}

int usb_emul_setup_device(struct udevice *dev, struct usb_string *strings,
			  void **desc_list)
{
	struct usb_dev_platdata *plat = dev_get_parent_platdata(dev);
	struct usb_generic_descriptor **ptr;
	struct usb_config_descriptor *cdesc;
	int upto;

	plat->strings = strings;
	plat->desc_list = (struct usb_generic_descriptor **)desc_list;

	/* Fill in wTotalLength for each configuration descriptor */
	ptr = plat->desc_list;
	for (cdesc = NULL, upto = 0; *ptr; upto += (*ptr)->bLength, ptr++) {
		debug("   - upto=%d, type=%d\n", upto, (*ptr)->bDescriptorType);
		if ((*ptr)->bDescriptorType == USB_DT_CONFIG) {
			if (cdesc) {
				cdesc->wTotalLength = upto;
				debug("%s: config %d length %d\n", __func__,
				      cdesc->bConfigurationValue,
				      cdesc->bLength);
			}
			cdesc = (struct usb_config_descriptor *)*ptr;
			upto = 0;
		}
	}
	if (cdesc) {
		cdesc->wTotalLength = upto;
		debug("%s: config %d length %d\n", __func__,
		      cdesc->bConfigurationValue, cdesc->wTotalLength);
	}

	return 0;
}

UCLASS_DRIVER(usb_emul) = {
	.id		= UCLASS_USB_EMUL,
	.name		= "usb_emul",
	.post_bind	= dm_scan_fdt_dev,
	.per_device_platdata_auto_alloc_size = sizeof(struct usb_emul_platdata),
	.per_child_auto_alloc_size = sizeof(struct usb_device),
	.per_child_platdata_auto_alloc_size = sizeof(struct usb_dev_platdata),
};
