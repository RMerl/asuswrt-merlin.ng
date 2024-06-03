// SPDX-License-Identifier: GPL-2.0+
/*
 * composite.c - infrastructure for Composite USB Gadgets
 *
 * Copyright (C) 2006-2008 David Brownell
 * U-Boot porting: Lukasz Majewski <l.majewski@samsung.com>
 */
#undef DEBUG

#include <linux/bitops.h>
#include <linux/usb/composite.h>

#define USB_BUFSIZ	4096

static struct usb_composite_driver *composite;

/**
 * usb_add_function() - add a function to a configuration
 * @config: the configuration
 * @function: the function being added
 * Context: single threaded during gadget setup
 *
 * After initialization, each configuration must have one or more
 * functions added to it.  Adding a function involves calling its @bind()
 * method to allocate resources such as interface and string identifiers
 * and endpoints.
 *
 * This function returns the value of the function's bind(), which is
 * zero for success else a negative errno value.
 */
int usb_add_function(struct usb_configuration *config,
		struct usb_function *function)
{
	int	value = -EINVAL;

	debug("adding '%s'/%p to config '%s'/%p\n",
			function->name, function,
			config->label, config);

	if (!function->set_alt || !function->disable)
		goto done;

	function->config = config;
	list_add_tail(&function->list, &config->functions);

	if (function->bind) {
		value = function->bind(config, function);
		if (value < 0) {
			list_del(&function->list);
			function->config = NULL;
		}
	} else
		value = 0;

	if (!config->fullspeed && function->descriptors)
		config->fullspeed = 1;
	if (!config->highspeed && function->hs_descriptors)
		config->highspeed = 1;

done:
	if (value)
		debug("adding '%s'/%p --> %d\n",
				function->name, function, value);
	return value;
}

/**
 * usb_function_deactivate - prevent function and gadget enumeration
 * @function: the function that isn't yet ready to respond
 *
 * Blocks response of the gadget driver to host enumeration by
 * preventing the data line pullup from being activated.  This is
 * normally called during @bind() processing to change from the
 * initial "ready to respond" state, or when a required resource
 * becomes available.
 *
 * For example, drivers that serve as a passthrough to a userspace
 * daemon can block enumeration unless that daemon (such as an OBEX,
 * MTP, or print server) is ready to handle host requests.
 *
 * Not all systems support software control of their USB peripheral
 * data pullups.
 *
 * Returns zero on success, else negative errno.
 */
int usb_function_deactivate(struct usb_function *function)
{
	struct usb_composite_dev	*cdev = function->config->cdev;
	int				status = 0;

	if (cdev->deactivations == 0)
		status = usb_gadget_disconnect(cdev->gadget);
	if (status == 0)
		cdev->deactivations++;

	return status;
}

/**
 * usb_function_activate - allow function and gadget enumeration
 * @function: function on which usb_function_activate() was called
 *
 * Reverses effect of usb_function_deactivate().  If no more functions
 * are delaying their activation, the gadget driver will respond to
 * host enumeration procedures.
 *
 * Returns zero on success, else negative errno.
 */
int usb_function_activate(struct usb_function *function)
{
	struct usb_composite_dev	*cdev = function->config->cdev;
	int				status = 0;

	if (cdev->deactivations == 0)
		status = -EINVAL;
	else {
		cdev->deactivations--;
		if (cdev->deactivations == 0)
			status = usb_gadget_connect(cdev->gadget);
	}

	return status;
}

/**
 * usb_interface_id() - allocate an unused interface ID
 * @config: configuration associated with the interface
 * @function: function handling the interface
 * Context: single threaded during gadget setup
 *
 * usb_interface_id() is called from usb_function.bind() callbacks to
 * allocate new interface IDs.  The function driver will then store that
 * ID in interface, association, CDC union, and other descriptors.  It
 * will also handle any control requests targetted at that interface,
 * particularly changing its altsetting via set_alt().  There may
 * also be class-specific or vendor-specific requests to handle.
 *
 * All interface identifier should be allocated using this routine, to
 * ensure that for example different functions don't wrongly assign
 * different meanings to the same identifier.  Note that since interface
 * identifers are configuration-specific, functions used in more than
 * one configuration (or more than once in a given configuration) need
 * multiple versions of the relevant descriptors.
 *
 * Returns the interface ID which was allocated; or -ENODEV if no
 * more interface IDs can be allocated.
 */
int usb_interface_id(struct usb_configuration *config,
		struct usb_function *function)
{
	unsigned char id = config->next_interface_id;

	if (id < MAX_CONFIG_INTERFACES) {
		config->interface[id] = function;
		config->next_interface_id = id + 1;
		return id;
	}
	return -ENODEV;
}

static int config_buf(struct usb_configuration *config,
		enum usb_device_speed speed, void *buf, u8 type)
{
	int				len = USB_BUFSIZ - USB_DT_CONFIG_SIZE;
	void				*next = buf + USB_DT_CONFIG_SIZE;
	struct usb_descriptor_header    **descriptors;
	struct usb_config_descriptor	*c;
	int				status;
	struct usb_function		*f;

	/* write the config descriptor */
	c = buf;
	c->bLength = USB_DT_CONFIG_SIZE;
	c->bDescriptorType = type;

	c->bNumInterfaces = config->next_interface_id;
	c->bConfigurationValue = config->bConfigurationValue;
	c->iConfiguration = config->iConfiguration;
	c->bmAttributes = USB_CONFIG_ATT_ONE | config->bmAttributes;
	c->bMaxPower = config->bMaxPower ? : (CONFIG_USB_GADGET_VBUS_DRAW / 2);

	/* There may be e.g. OTG descriptors */
	if (config->descriptors) {
		status = usb_descriptor_fillbuf(next, len,
				config->descriptors);
		if (status < 0)
			return status;
		len -= status;
		next += status;
	}

	/* add each function's descriptors */
	list_for_each_entry(f, &config->functions, list) {
		if (speed == USB_SPEED_HIGH)
			descriptors = f->hs_descriptors;
		else
			descriptors = f->descriptors;
		if (!descriptors)
			continue;
		status = usb_descriptor_fillbuf(next, len,
			(const struct usb_descriptor_header **) descriptors);
		if (status < 0)
			return status;
		len -= status;
		next += status;
	}

	len = next - buf;
	c->wTotalLength = cpu_to_le16(len);
	return len;
}

static int config_desc(struct usb_composite_dev *cdev, unsigned w_value)
{
	enum usb_device_speed		speed = USB_SPEED_UNKNOWN;
	struct usb_gadget		*gadget = cdev->gadget;
	u8				type = w_value >> 8;
	int                             hs = 0;
	struct usb_configuration	*c;

	if (gadget_is_dualspeed(gadget)) {
		if (gadget->speed == USB_SPEED_HIGH)
			hs = 1;
		if (type == USB_DT_OTHER_SPEED_CONFIG)
			hs = !hs;
		if (hs)
			speed = USB_SPEED_HIGH;
	}

	w_value &= 0xff;
	list_for_each_entry(c, &cdev->configs, list) {
		if (speed == USB_SPEED_HIGH) {
			if (!c->highspeed)
				continue;
		} else {
			if (!c->fullspeed)
				continue;
		}
		if (w_value == 0)
			return config_buf(c, speed, cdev->req->buf, type);
		w_value--;
	}
	return -EINVAL;
}

static int count_configs(struct usb_composite_dev *cdev, unsigned type)
{
	struct usb_gadget		*gadget = cdev->gadget;
	unsigned			count = 0;
	int				hs = 0;
	struct usb_configuration	*c;

	if (gadget_is_dualspeed(gadget)) {
		if (gadget->speed == USB_SPEED_HIGH)
			hs = 1;
		if (type == USB_DT_DEVICE_QUALIFIER)
			hs = !hs;
	}
	list_for_each_entry(c, &cdev->configs, list) {
		/* ignore configs that won't work at this speed */
		if (hs) {
			if (!c->highspeed)
				continue;
		} else {
			if (!c->fullspeed)
				continue;
		}
		count++;
	}
	return count;
}

static void device_qual(struct usb_composite_dev *cdev)
{
	struct usb_qualifier_descriptor	*qual = cdev->req->buf;

	qual->bLength = sizeof(*qual);
	qual->bDescriptorType = USB_DT_DEVICE_QUALIFIER;
	/* POLICY: same bcdUSB and device type info at both speeds */
	qual->bcdUSB = cdev->desc.bcdUSB;
	qual->bDeviceClass = cdev->desc.bDeviceClass;
	qual->bDeviceSubClass = cdev->desc.bDeviceSubClass;
	qual->bDeviceProtocol = cdev->desc.bDeviceProtocol;
	/* ASSUME same EP0 fifo size at both speeds */
	qual->bMaxPacketSize0 = cdev->gadget->ep0->maxpacket;
	qual->bNumConfigurations = count_configs(cdev, USB_DT_DEVICE_QUALIFIER);
	qual->bRESERVED = 0;
}

static void reset_config(struct usb_composite_dev *cdev)
{
	struct usb_function		*f;

	debug("%s:\n", __func__);

	list_for_each_entry(f, &cdev->config->functions, list) {
		if (f->disable)
			f->disable(f);

		bitmap_zero(f->endpoints, 32);
	}
	cdev->config = NULL;
}

static int set_config(struct usb_composite_dev *cdev,
		const struct usb_ctrlrequest *ctrl, unsigned number)
{
	struct usb_gadget	*gadget = cdev->gadget;
	unsigned		power = gadget_is_otg(gadget) ? 8 : 100;
	struct usb_descriptor_header **descriptors;
	int			result = -EINVAL;
	struct usb_endpoint_descriptor *ep;
	struct usb_configuration *c = NULL;
	int                     addr;
	int			tmp;
	struct usb_function	*f;

	if (cdev->config)
		reset_config(cdev);

	if (number) {
		list_for_each_entry(c, &cdev->configs, list) {
			if (c->bConfigurationValue == number) {
				result = 0;
				break;
			}
		}
		if (result < 0)
			goto done;
	} else
		result = 0;

	debug("%s: %s speed config #%d: %s\n", __func__,
	     ({ char *speed;
		     switch (gadget->speed) {
		     case USB_SPEED_LOW:
			     speed = "low";
			     break;
		     case USB_SPEED_FULL:
			     speed = "full";
			     break;
		     case USB_SPEED_HIGH:
			     speed = "high";
			     break;
		     default:
			     speed = "?";
			     break;
		     };
		     speed;
	     }), number, c ? c->label : "unconfigured");

	if (!c)
		goto done;

	cdev->config = c;

	/* Initialize all interfaces by setting them to altsetting zero. */
	for (tmp = 0; tmp < MAX_CONFIG_INTERFACES; tmp++) {
		f = c->interface[tmp];
		if (!f)
			break;

		/*
		 * Record which endpoints are used by the function. This is used
		 * to dispatch control requests targeted at that endpoint to the
		 * function's setup callback instead of the current
		 * configuration's setup callback.
		 */
		if (gadget->speed == USB_SPEED_HIGH)
			descriptors = f->hs_descriptors;
		else
			descriptors = f->descriptors;

		for (; *descriptors; ++descriptors) {
			if ((*descriptors)->bDescriptorType != USB_DT_ENDPOINT)
				continue;

			ep = (struct usb_endpoint_descriptor *)*descriptors;
			addr = ((ep->bEndpointAddress & 0x80) >> 3)
			     |	(ep->bEndpointAddress & 0x0f);
			generic_set_bit(addr, f->endpoints);
		}

		result = f->set_alt(f, tmp, 0);
		if (result < 0) {
			debug("interface %d (%s/%p) alt 0 --> %d\n",
					tmp, f->name, f, result);

			reset_config(cdev);
			goto done;
		}
	}

	/* when we return, be sure our power usage is valid */
	power = c->bMaxPower ? (2 * c->bMaxPower) : CONFIG_USB_GADGET_VBUS_DRAW;
done:
	usb_gadget_vbus_draw(gadget, power);
	return result;
}

/**
 * usb_add_config() - add a configuration to a device.
 * @cdev: wraps the USB gadget
 * @config: the configuration, with bConfigurationValue assigned
 * Context: single threaded during gadget setup
 *
 * One of the main tasks of a composite driver's bind() routine is to
 * add each of the configurations it supports, using this routine.
 *
 * This function returns the value of the configuration's bind(), which
 * is zero for success else a negative errno value.  Binding configurations
 * assigns global resources including string IDs, and per-configuration
 * resources such as interface IDs and endpoints.
 */
int usb_add_config(struct usb_composite_dev *cdev,
		struct usb_configuration *config)
{
	int				status = -EINVAL;
	struct usb_configuration	*c;
	struct usb_function		*f;
	unsigned int			i;

	debug("%s: adding config #%u '%s'/%p\n", __func__,
			config->bConfigurationValue,
			config->label, config);

	if (!config->bConfigurationValue || !config->bind)
		goto done;

	/* Prevent duplicate configuration identifiers */
	list_for_each_entry(c, &cdev->configs, list) {
		if (c->bConfigurationValue == config->bConfigurationValue) {
			status = -EBUSY;
			goto done;
		}
	}

	config->cdev = cdev;
	list_add_tail(&config->list, &cdev->configs);

	INIT_LIST_HEAD(&config->functions);
	config->next_interface_id = 0;

	status = config->bind(config);
	if (status < 0) {
		list_del(&config->list);
		config->cdev = NULL;
	} else {
		debug("cfg %d/%p speeds:%s%s\n",
			config->bConfigurationValue, config,
			config->highspeed ? " high" : "",
			config->fullspeed
				? (gadget_is_dualspeed(cdev->gadget)
					? " full"
					: " full/low")
				: "");

		for (i = 0; i < MAX_CONFIG_INTERFACES; i++) {
			f = config->interface[i];
			if (!f)
				continue;
			debug("%s: interface %d = %s/%p\n",
			      __func__, i, f->name, f);
		}
	}

	usb_ep_autoconfig_reset(cdev->gadget);

done:
	if (status)
		debug("added config '%s'/%u --> %d\n", config->label,
				config->bConfigurationValue, status);
	return status;
}

/*
 * We support strings in multiple languages ... string descriptor zero
 * says which languages are supported.	The typical case will be that
 * only one language (probably English) is used, with I18N handled on
 * the host side.
 */

static void collect_langs(struct usb_gadget_strings **sp, __le16 *buf)
{
	const struct usb_gadget_strings	*s;
	u16				language;
	__le16				*tmp;

	while (*sp) {
		s = *sp;
		language = cpu_to_le16(s->language);
		for (tmp = buf; *tmp && tmp < &buf[126]; tmp++) {
			if (*tmp == language)
				goto repeat;
		}
		*tmp++ = language;
repeat:
		sp++;
	}
}

static int lookup_string(
	struct usb_gadget_strings	**sp,
	void				*buf,
	u16				language,
	int				id
)
{
	int				value;
	struct usb_gadget_strings	*s;

	while (*sp) {
		s = *sp++;
		if (s->language != language)
			continue;
		value = usb_gadget_get_string(s, id, buf);
		if (value > 0)
			return value;
	}
	return -EINVAL;
}

static int get_string(struct usb_composite_dev *cdev,
		void *buf, u16 language, int id)
{
	struct usb_string_descriptor	*s = buf;
	struct usb_gadget_strings	**sp;
	int				len;
	struct usb_configuration	*c;
	struct usb_function		*f;

	/*
	 * Yes, not only is USB's I18N support probably more than most
	 * folk will ever care about ... also, it's all supported here.
	 * (Except for UTF8 support for Unicode's "Astral Planes".)
	 */

	/* 0 == report all available language codes */
	if (id == 0) {
		memset(s, 0, 256);
		s->bDescriptorType = USB_DT_STRING;

		sp = composite->strings;
		if (sp)
			collect_langs(sp, s->wData);

		list_for_each_entry(c, &cdev->configs, list) {
			sp = c->strings;
			if (sp)
				collect_langs(sp, s->wData);

			list_for_each_entry(f, &c->functions, list) {
				sp = f->strings;
				if (sp)
					collect_langs(sp, s->wData);
			}
		}

		for (len = 0; len <= 126 && s->wData[len]; len++)
			continue;
		if (!len)
			return -EINVAL;

		s->bLength = 2 * (len + 1);
		return s->bLength;
	}

	/*
	 * Otherwise, look up and return a specified string.  String IDs
	 * are device-scoped, so we look up each string table we're told
	 * about.  These lookups are infrequent; simpler-is-better here.
	 */
	if (composite->strings) {
		len = lookup_string(composite->strings, buf, language, id);
		if (len > 0)
			return len;
	}
	list_for_each_entry(c, &cdev->configs, list) {
		if (c->strings) {
			len = lookup_string(c->strings, buf, language, id);
			if (len > 0)
				return len;
		}
		list_for_each_entry(f, &c->functions, list) {
			if (!f->strings)
				continue;
			len = lookup_string(f->strings, buf, language, id);
			if (len > 0)
				return len;
		}
	}
	return -EINVAL;
}

/**
 * usb_string_id() - allocate an unused string ID
 * @cdev: the device whose string descriptor IDs are being allocated
 * Context: single threaded during gadget setup
 *
 * @usb_string_id() is called from bind() callbacks to allocate
 * string IDs.	Drivers for functions, configurations, or gadgets will
 * then store that ID in the appropriate descriptors and string table.
 *
 * All string identifier should be allocated using this,
 * @usb_string_ids_tab() or @usb_string_ids_n() routine, to ensure
 * that for example different functions don't wrongly assign different
 * meanings to the same identifier.
 */
int usb_string_id(struct usb_composite_dev *cdev)
{
	if (cdev->next_string_id < 254) {
		/*
		 * string id 0 is reserved by USB spec for list of
		 * supported languages
		 * 255 reserved as well? -- mina86
		 */
		cdev->next_string_id++;
		return cdev->next_string_id;
	}
	return -ENODEV;
}

/**
 * usb_string_ids() - allocate unused string IDs in batch
 * @cdev: the device whose string descriptor IDs are being allocated
 * @str: an array of usb_string objects to assign numbers to
 * Context: single threaded during gadget setup
 *
 * @usb_string_ids() is called from bind() callbacks to allocate
 * string IDs.	Drivers for functions, configurations, or gadgets will
 * then copy IDs from the string table to the appropriate descriptors
 * and string table for other languages.
 *
 * All string identifier should be allocated using this,
 * @usb_string_id() or @usb_string_ids_n() routine, to ensure that for
 * example different functions don't wrongly assign different meanings
 * to the same identifier.
 */
int usb_string_ids_tab(struct usb_composite_dev *cdev, struct usb_string *str)
{
	u8 next = cdev->next_string_id;

	for (; str->s; ++str) {
		if (next >= 254)
			return -ENODEV;
		str->id = ++next;
	}

	cdev->next_string_id = next;

	return 0;
}

/**
 * usb_string_ids_n() - allocate unused string IDs in batch
 * @c: the device whose string descriptor IDs are being allocated
 * @n: number of string IDs to allocate
 * Context: single threaded during gadget setup
 *
 * Returns the first requested ID.  This ID and next @n-1 IDs are now
 * valid IDs.  At least provided that @n is non-zero because if it
 * is, returns last requested ID which is now very useful information.
 *
 * @usb_string_ids_n() is called from bind() callbacks to allocate
 * string IDs.	Drivers for functions, configurations, or gadgets will
 * then store that ID in the appropriate descriptors and string table.
 *
 * All string identifier should be allocated using this,
 * @usb_string_id() or @usb_string_ids_n() routine, to ensure that for
 * example different functions don't wrongly assign different meanings
 * to the same identifier.
 */
int usb_string_ids_n(struct usb_composite_dev *c, unsigned n)
{
	u8 next = c->next_string_id;

	if (n > 254 || next + n > 254)
		return -ENODEV;

	c->next_string_id += n;
	return next + 1;
}

static void composite_setup_complete(struct usb_ep *ep, struct usb_request *req)
{
	if (req->status || req->actual != req->length)
		debug("%s: setup complete --> %d, %d/%d\n", __func__,
				req->status, req->actual, req->length);
}

/*
 * The setup() callback implements all the ep0 functionality that's
 * not handled lower down, in hardware or the hardware driver(like
 * device and endpoint feature flags, and their status).  It's all
 * housekeeping for the gadget function we're implementing.  Most of
 * the work is in config and function specific setup.
 */
static int
composite_setup(struct usb_gadget *gadget, const struct usb_ctrlrequest *ctrl)
{
	u16				w_length = le16_to_cpu(ctrl->wLength);
	u16				w_index = le16_to_cpu(ctrl->wIndex);
	u16				w_value = le16_to_cpu(ctrl->wValue);
	struct usb_composite_dev	*cdev = get_gadget_data(gadget);
	u8				intf = w_index & 0xFF;
	int				value = -EOPNOTSUPP;
	struct usb_request		*req = cdev->req;
	struct usb_function		*f = NULL;
	int				standard;
	u8				endp;
	struct usb_configuration	*c;

	/*
	 * partial re-init of the response message; the function or the
	 * gadget might need to intercept e.g. a control-OUT completion
	 * when we delegate to it.
	 */
	req->zero = 0;
	req->complete = composite_setup_complete;
	req->length = USB_BUFSIZ;
	gadget->ep0->driver_data = cdev;
	standard = (ctrl->bRequestType & USB_TYPE_MASK)
						== USB_TYPE_STANDARD;
	if (!standard)
		goto unknown;

	switch (ctrl->bRequest) {

	/* we handle all standard USB descriptors */
	case USB_REQ_GET_DESCRIPTOR:
		if (ctrl->bRequestType != USB_DIR_IN)
			goto unknown;
		switch (w_value >> 8) {

		case USB_DT_DEVICE:
			cdev->desc.bNumConfigurations =
				count_configs(cdev, USB_DT_DEVICE);

			/*
			 * If the speed is Super speed, then the supported
			 * max packet size is 512 and it should be sent as
			 * exponent of 2. So, 9(2^9=512) should be filled in
			 * bMaxPacketSize0. Also fill USB version as 3.0
			 * if speed is Super speed.
			 */
			if (cdev->gadget->speed == USB_SPEED_SUPER) {
				cdev->desc.bMaxPacketSize0 = 9;
				cdev->desc.bcdUSB = cpu_to_le16(0x0300);
			} else {
				cdev->desc.bMaxPacketSize0 =
					cdev->gadget->ep0->maxpacket;
			}
			value = min(w_length, (u16) sizeof cdev->desc);
			memcpy(req->buf, &cdev->desc, value);
			break;
		case USB_DT_DEVICE_QUALIFIER:
			if (!gadget_is_dualspeed(gadget))
				break;
			device_qual(cdev);
			value = min_t(int, w_length,
				      sizeof(struct usb_qualifier_descriptor));
			break;
		case USB_DT_OTHER_SPEED_CONFIG:
			if (!gadget_is_dualspeed(gadget))
				break;

		case USB_DT_CONFIG:
			value = config_desc(cdev, w_value);
			if (value >= 0)
				value = min(w_length, (u16) value);
			break;
		case USB_DT_STRING:
			value = get_string(cdev, req->buf,
					w_index, w_value & 0xff);
			if (value >= 0)
				value = min(w_length, (u16) value);
			break;
		case USB_DT_BOS:
			/*
			 * The USB compliance test (USB 2.0 Command Verifier)
			 * issues this request. We should not run into the
			 * default path here. But return for now until
			 * the superspeed support is added.
			 */
			break;
		default:
			goto unknown;
		}
		break;

	/* any number of configs can work */
	case USB_REQ_SET_CONFIGURATION:
		if (ctrl->bRequestType != 0)
			goto unknown;
		if (gadget_is_otg(gadget)) {
			if (gadget->a_hnp_support)
				debug("HNP available\n");
			else if (gadget->a_alt_hnp_support)
				debug("HNP on another port\n");
			else
				debug("HNP inactive\n");
		}

		value = set_config(cdev, ctrl, w_value);
		break;
	case USB_REQ_GET_CONFIGURATION:
		if (ctrl->bRequestType != USB_DIR_IN)
			goto unknown;
		if (cdev->config)
			*(u8 *)req->buf = cdev->config->bConfigurationValue;
		else
			*(u8 *)req->buf = 0;
		value = min(w_length, (u16) 1);
		break;

	/*
	 * function drivers must handle get/set altsetting; if there's
	 * no get() method, we know only altsetting zero works.
	 */
	case USB_REQ_SET_INTERFACE:
		if (ctrl->bRequestType != USB_RECIP_INTERFACE)
			goto unknown;
		if (!cdev->config || w_index >= MAX_CONFIG_INTERFACES)
			break;
		f = cdev->config->interface[intf];
		if (!f)
			break;
		if (w_value && !f->set_alt)
			break;
		value = f->set_alt(f, w_index, w_value);
		break;
	case USB_REQ_GET_INTERFACE:
		if (ctrl->bRequestType != (USB_DIR_IN|USB_RECIP_INTERFACE))
			goto unknown;
		if (!cdev->config || w_index >= MAX_CONFIG_INTERFACES)
			break;
		f = cdev->config->interface[intf];
		if (!f)
			break;
		/* lots of interfaces only need altsetting zero... */
		value = f->get_alt ? f->get_alt(f, w_index) : 0;
		if (value < 0)
			break;
		*((u8 *)req->buf) = value;
		value = min(w_length, (u16) 1);
		break;
	default:
unknown:
		debug("non-core control req%02x.%02x v%04x i%04x l%d\n",
			ctrl->bRequestType, ctrl->bRequest,
			w_value, w_index, w_length);

		if (!cdev->config)
			goto done;

		/*
		 * functions always handle their interfaces and endpoints...
		 * punt other recipients (other, WUSB, ...) to the current
		 * configuration code.
		 */
		switch (ctrl->bRequestType & USB_RECIP_MASK) {
		case USB_RECIP_INTERFACE:
			f = cdev->config->interface[intf];
			break;

		case USB_RECIP_ENDPOINT:
			endp = ((w_index & 0x80) >> 3) | (w_index & 0x0f);
			list_for_each_entry(f, &cdev->config->functions, list) {
				if (test_bit(endp, f->endpoints))
					break;
			}
			if (&f->list == &cdev->config->functions)
				f = NULL;
			break;
		/*
		 * dfu-util (version 0.5) sets bmRequestType.Receipent = Device
		 * for non-standard request (w_value = 0x21,
		 * bRequest = GET_DESCRIPTOR in this case).
		 * When only one interface is registered (as it is done now),
		 * then this request shall be handled as it was requested for
		 * interface.
		 *
		 * In the below code it is checked if only one interface is
		 * present and proper function for it is extracted. Due to that
		 * function's setup (f->setup) is called to handle this
		 * special non-standard request.
		 */
		case USB_RECIP_DEVICE:
			debug("cdev->config->next_interface_id: %d intf: %d\n",
			       cdev->config->next_interface_id, intf);
			if (cdev->config->next_interface_id == 1)
				f = cdev->config->interface[intf];
			break;
		}

		if (f && f->setup)
			value = f->setup(f, ctrl);
		else {
			c = cdev->config;
			if (c->setup)
				value = c->setup(c, ctrl);
		}

		goto done;
	}

	/* respond with data transfer before status phase? */
	if (value >= 0) {
		req->length = value;
		req->zero = value < w_length;
		value = usb_ep_queue(gadget->ep0, req, GFP_KERNEL);
		if (value < 0) {
			debug("ep_queue --> %d\n", value);
			req->status = 0;
			composite_setup_complete(gadget->ep0, req);
		}
	}

done:
	/* device either stalls (value < 0) or reports success */
	return value;
}

static void composite_disconnect(struct usb_gadget *gadget)
{
	struct usb_composite_dev	*cdev = get_gadget_data(gadget);

	if (cdev->config)
		reset_config(cdev);
	if (composite->disconnect)
		composite->disconnect(cdev);
}

static void composite_unbind(struct usb_gadget *gadget)
{
	struct usb_composite_dev	*cdev = get_gadget_data(gadget);
	struct usb_configuration	*c;
	struct usb_function		*f;

	/*
	 * composite_disconnect() must already have been called
	 * by the underlying peripheral controller driver!
	 * so there's no i/o concurrency that could affect the
	 * state protected by cdev->lock.
	 */
	BUG_ON(cdev->config);

	while (!list_empty(&cdev->configs)) {
		c = list_first_entry(&cdev->configs,
				struct usb_configuration, list);
		while (!list_empty(&c->functions)) {
			f = list_first_entry(&c->functions,
					struct usb_function, list);
			list_del(&f->list);
			if (f->unbind) {
				debug("unbind function '%s'/%p\n",
						f->name, f);
				f->unbind(c, f);
			}
		}
		list_del(&c->list);
		if (c->unbind) {
			debug("unbind config '%s'/%p\n", c->label, c);
			c->unbind(c);
		}
		free(c);
	}
	if (composite->unbind)
		composite->unbind(cdev);

	if (cdev->req) {
		kfree(cdev->req->buf);
		usb_ep_free_request(gadget->ep0, cdev->req);
	}
	kfree(cdev);
	set_gadget_data(gadget, NULL);

	composite = NULL;
}

static int composite_bind(struct usb_gadget *gadget)
{
	int				status = -ENOMEM;
	struct usb_composite_dev	*cdev;

	cdev = calloc(sizeof *cdev, 1);
	if (!cdev)
		return status;

	cdev->gadget = gadget;
	set_gadget_data(gadget, cdev);
	INIT_LIST_HEAD(&cdev->configs);

	/* preallocate control response and buffer */
	cdev->req = usb_ep_alloc_request(gadget->ep0, GFP_KERNEL);
	if (!cdev->req)
		goto fail;
	cdev->req->buf = memalign(CONFIG_SYS_CACHELINE_SIZE, USB_BUFSIZ);
	if (!cdev->req->buf)
		goto fail;
	cdev->req->complete = composite_setup_complete;
	gadget->ep0->driver_data = cdev;

	cdev->bufsiz = USB_BUFSIZ;
	cdev->driver = composite;

	usb_gadget_set_selfpowered(gadget);
	usb_ep_autoconfig_reset(cdev->gadget);

	status = composite->bind(cdev);
	if (status < 0)
		goto fail;

	memcpy(&cdev->desc, composite->dev,
	       sizeof(struct usb_device_descriptor));
	cdev->desc.bMaxPacketSize0 = gadget->ep0->maxpacket;

	debug("%s: ready\n", composite->name);
	return 0;

fail:
	composite_unbind(gadget);
	return status;
}

static void
composite_suspend(struct usb_gadget *gadget)
{
	struct usb_composite_dev	*cdev = get_gadget_data(gadget);
	struct usb_function		*f;

	debug("%s: suspend\n", __func__);
	if (cdev->config) {
		list_for_each_entry(f, &cdev->config->functions, list) {
			if (f->suspend)
				f->suspend(f);
		}
	}
	if (composite->suspend)
		composite->suspend(cdev);

	cdev->suspended = 1;
}

static void
composite_resume(struct usb_gadget *gadget)
{
	struct usb_composite_dev	*cdev = get_gadget_data(gadget);
	struct usb_function		*f;

	debug("%s: resume\n", __func__);
	if (composite->resume)
		composite->resume(cdev);
	if (cdev->config) {
		list_for_each_entry(f, &cdev->config->functions, list) {
			if (f->resume)
				f->resume(f);
		}
	}

	cdev->suspended = 0;
}

static struct usb_gadget_driver composite_driver = {
	.speed		= USB_SPEED_HIGH,

	.bind		= composite_bind,
	.unbind         = composite_unbind,

	.setup		= composite_setup,
	.reset          = composite_disconnect,
	.disconnect	= composite_disconnect,

	.suspend        = composite_suspend,
	.resume         = composite_resume,
};

/**
 * usb_composite_register() - register a composite driver
 * @driver: the driver to register
 * Context: single threaded during gadget setup
 *
 * This function is used to register drivers using the composite driver
 * framework.  The return value is zero, or a negative errno value.
 * Those values normally come from the driver's @bind method, which does
 * all the work of setting up the driver to match the hardware.
 *
 * On successful return, the gadget is ready to respond to requests from
 * the host, unless one of its components invokes usb_gadget_disconnect()
 * while it was binding.  That would usually be done in order to wait for
 * some userspace participation.
 */
int usb_composite_register(struct usb_composite_driver *driver)
{
	int res;

	if (!driver || !driver->dev || !driver->bind || composite)
		return -EINVAL;

	if (!driver->name)
		driver->name = "composite";
	composite = driver;

	res = usb_gadget_register_driver(&composite_driver);
	if (res != 0)
		composite = NULL;

	return res;
}

/**
 * usb_composite_unregister() - unregister a composite driver
 * @driver: the driver to unregister
 *
 * This function is used to unregister drivers using the composite
 * driver framework.
 */
void usb_composite_unregister(struct usb_composite_driver *driver)
{
	if (composite != driver)
		return;
	usb_gadget_unregister_driver(&composite_driver);
	composite = NULL;
}
