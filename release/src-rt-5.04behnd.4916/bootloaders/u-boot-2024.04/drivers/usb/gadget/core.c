// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2003
 * Gerry Hamel, geh@ti.com, Texas Instruments
 *
 * Based on
 * linux/drivers/usbd/usbd.c.c - USB Device Core Layer
 *
 * Copyright (c) 2000, 2001, 2002 Lineo
 * Copyright (c) 2001 Hewlett Packard
 *
 * By:
 *	Stuart Lynne <sl@lineo.com>,
 *	Tom Rushworth <tbr@lineo.com>,
 *	Bruce Balden <balden@lineo.com>
 */

#include <log.h>
#include <malloc.h>
#include <serial.h>
#include <usbdevice.h>

#define MAX_INTERFACES 2


int maxstrings = 20;

/* Global variables ************************************************************************** */

struct usb_string_descriptor **usb_strings;

int usb_devices;

extern struct usb_function_driver ep0_driver;

int registered_functions;
int registered_devices;

__maybe_unused static char *usbd_device_events[] = {
	"DEVICE_UNKNOWN",
	"DEVICE_INIT",
	"DEVICE_CREATE",
	"DEVICE_HUB_CONFIGURED",
	"DEVICE_RESET",
	"DEVICE_ADDRESS_ASSIGNED",
	"DEVICE_CONFIGURED",
	"DEVICE_SET_INTERFACE",
	"DEVICE_SET_FEATURE",
	"DEVICE_CLEAR_FEATURE",
	"DEVICE_DE_CONFIGURED",
	"DEVICE_BUS_INACTIVE",
	"DEVICE_BUS_ACTIVITY",
	"DEVICE_POWER_INTERRUPTION",
	"DEVICE_HUB_RESET",
	"DEVICE_DESTROY",
	"DEVICE_FUNCTION_PRIVATE",
};

__maybe_unused static char *usbd_device_status[] = {
	"USBD_OPENING",
	"USBD_OK",
	"USBD_SUSPENDED",
	"USBD_CLOSING",
};

#define USBD_DEVICE_STATUS(x) (((unsigned int)x <= USBD_CLOSING) ? usbd_device_status[x] : "UNKNOWN")


/* Descriptor support functions ************************************************************** */


/**
 * usbd_get_string - find and return a string descriptor
 * @index: string index to return
 *
 * Find an indexed string and return a pointer to a it.
 */
struct usb_string_descriptor *usbd_get_string (__u8 index)
{
	if (index >= maxstrings) {
		return NULL;
	}
	return usb_strings[index];
}


/* Access to device descriptor functions ***************************************************** */


/* *
 * usbd_device_configuration_instance - find a configuration instance for this device
 * @device:
 * @configuration: index to configuration, 0 - N-1
 *
 * Get specifed device configuration. Index should be bConfigurationValue-1.
 */
static struct usb_configuration_instance *usbd_device_configuration_instance (struct usb_device_instance *device,
		unsigned int port, unsigned int configuration)
{
	if (configuration >= device->configurations)
		return NULL;

	return device->configuration_instance_array + configuration;
}


/* *
 * usbd_device_interface_instance
 * @device:
 * @configuration: index to configuration, 0 - N-1
 * @interface: index to interface
 *
 * Return the specified interface descriptor for the specified device.
 */
struct usb_interface_instance *usbd_device_interface_instance (struct usb_device_instance *device, int port, int configuration, int interface)
{
	struct usb_configuration_instance *configuration_instance;

	if ((configuration_instance = usbd_device_configuration_instance (device, port, configuration)) == NULL) {
		return NULL;
	}
	if (interface >= configuration_instance->interfaces) {
		return NULL;
	}
	return configuration_instance->interface_instance_array + interface;
}

/* *
 * usbd_device_alternate_descriptor_list
 * @device:
 * @configuration: index to configuration, 0 - N-1
 * @interface: index to interface
 * @alternate: alternate setting
 *
 * Return the specified alternate descriptor for the specified device.
 */
struct usb_alternate_instance *usbd_device_alternate_instance (struct usb_device_instance *device, int port, int configuration, int interface, int alternate)
{
	struct usb_interface_instance *interface_instance;

	if ((interface_instance = usbd_device_interface_instance (device, port, configuration, interface)) == NULL) {
		return NULL;
	}

	if (alternate >= interface_instance->alternates) {
		return NULL;
	}

	return interface_instance->alternates_instance_array + alternate;
}


/* *
 * usbd_device_device_descriptor
 * @device: which device
 * @configuration: index to configuration, 0 - N-1
 * @port: which port
 *
 * Return the specified configuration descriptor for the specified device.
 */
struct usb_device_descriptor *usbd_device_device_descriptor (struct usb_device_instance *device, int port)
{
	return (device->device_descriptor);
}

/**
 * usbd_device_configuration_descriptor
 * @device: which device
 * @port: which port
 * @configuration: index to configuration, 0 - N-1
 *
 * Return the specified configuration descriptor for the specified device.
 */
struct usb_configuration_descriptor *usbd_device_configuration_descriptor (struct
									   usb_device_instance
									   *device, int port, int configuration)
{
	struct usb_configuration_instance *configuration_instance;
	if (!(configuration_instance = usbd_device_configuration_instance (device, port, configuration))) {
		return NULL;
	}
	return (configuration_instance->configuration_descriptor);
}


/**
 * usbd_device_interface_descriptor
 * @device: which device
 * @port: which port
 * @configuration: index to configuration, 0 - N-1
 * @interface: index to interface
 * @alternate: alternate setting
 *
 * Return the specified interface descriptor for the specified device.
 */
struct usb_interface_descriptor *usbd_device_interface_descriptor (struct usb_device_instance
								   *device, int port, int configuration, int interface, int alternate)
{
	struct usb_interface_instance *interface_instance;
	if (!(interface_instance = usbd_device_interface_instance (device, port, configuration, interface))) {
		return NULL;
	}
	if ((alternate < 0) || (alternate >= interface_instance->alternates)) {
		return NULL;
	}
	return (interface_instance->alternates_instance_array[alternate].interface_descriptor);
}

/**
 * usbd_device_endpoint_descriptor_index
 * @device: which device
 * @port: which port
 * @configuration: index to configuration, 0 - N-1
 * @interface: index to interface
 * @alternate: index setting
 * @index: which index
 *
 * Return the specified endpoint descriptor for the specified device.
 */
struct usb_endpoint_descriptor *usbd_device_endpoint_descriptor_index (struct usb_device_instance
								       *device, int port, int configuration, int interface, int alternate, int index)
{
	struct usb_alternate_instance *alternate_instance;

	if (!(alternate_instance = usbd_device_alternate_instance (device, port, configuration, interface, alternate))) {
		return NULL;
	}
	if (index >= alternate_instance->endpoints) {
		return NULL;
	}
	return *(alternate_instance->endpoints_descriptor_array + index);
}


/**
 * usbd_device_endpoint_transfersize
 * @device: which device
 * @port: which port
 * @configuration: index to configuration, 0 - N-1
 * @interface: index to interface
 * @index: which index
 *
 * Return the specified endpoint transfer size;
 */
int usbd_device_endpoint_transfersize (struct usb_device_instance *device, int port, int configuration, int interface, int alternate, int index)
{
	struct usb_alternate_instance *alternate_instance;

	if (!(alternate_instance = usbd_device_alternate_instance (device, port, configuration, interface, alternate))) {
		return 0;
	}
	if (index >= alternate_instance->endpoints) {
		return 0;
	}
	return *(alternate_instance->endpoint_transfersize_array + index);
}


/**
 * usbd_device_endpoint_descriptor
 * @device: which device
 * @port: which port
 * @configuration: index to configuration, 0 - N-1
 * @interface: index to interface
 * @alternate: alternate setting
 * @endpoint: which endpoint
 *
 * Return the specified endpoint descriptor for the specified device.
 */
struct usb_endpoint_descriptor *usbd_device_endpoint_descriptor (struct usb_device_instance *device, int port, int configuration, int interface, int alternate, int endpoint)
{
	struct usb_endpoint_descriptor *endpoint_descriptor;
	int i;

	for (i = 0; !(endpoint_descriptor = usbd_device_endpoint_descriptor_index (device, port, configuration, interface, alternate, i)); i++) {
		if (endpoint_descriptor->bEndpointAddress == endpoint) {
			return endpoint_descriptor;
		}
	}
	return NULL;
}

/**
 * usbd_endpoint_halted
 * @device: point to struct usb_device_instance
 * @endpoint: endpoint to check
 *
 * Return non-zero if endpoint is halted.
 */
int usbd_endpoint_halted (struct usb_device_instance *device, int endpoint)
{
	return (device->status == USB_STATUS_HALT);
}


/**
 * usbd_rcv_complete - complete a receive
 * @endpoint:
 * @len:
 * @urb_bad:
 *
 * Called from rcv interrupt to complete.
 */
void usbd_rcv_complete(struct usb_endpoint_instance *endpoint, int len, int urb_bad)
{
	if (endpoint) {
		struct urb *rcv_urb;

		/*usbdbg("len: %d urb: %p\n", len, endpoint->rcv_urb); */

		/* if we had an urb then update actual_length, dispatch if neccessary */
		if ((rcv_urb = endpoint->rcv_urb)) {

			/*usbdbg("actual: %d buffer: %d\n", */
			/*rcv_urb->actual_length, rcv_urb->buffer_length); */

			/* check the urb is ok, are we adding data less than the packetsize */
			if (!urb_bad && (len <= endpoint->rcv_packetSize)) {
			  /*usbdbg("updating actual_length by %d\n",len); */

				/* increment the received data size */
				rcv_urb->actual_length += len;

			} else {
				usberr(" RECV_ERROR actual: %d buffer: %d urb_bad: %d\n",
				       rcv_urb->actual_length, rcv_urb->buffer_length, urb_bad);

				rcv_urb->actual_length = 0;
				rcv_urb->status = RECV_ERROR;
			}
		} else {
			usberr("no rcv_urb!");
		}
	} else {
		usberr("no endpoint!");
	}

}

/**
 * usbd_tx_complete - complete a transmit
 * @endpoint:
 * @resetart:
 *
 * Called from tx interrupt to complete.
 */
void usbd_tx_complete (struct usb_endpoint_instance *endpoint)
{
	if (endpoint) {
		struct urb *tx_urb;

		/* if we have a tx_urb advance or reset, finish if complete */
		if ((tx_urb = endpoint->tx_urb)) {
			int sent = endpoint->last;
			endpoint->sent += sent;
			endpoint->last -= sent;

			if( (endpoint->tx_urb->actual_length - endpoint->sent) <= 0 ) {
				tx_urb->actual_length = 0;
				endpoint->sent = 0;
				endpoint->last = 0;

				/* Remove from active, save for re-use */
				urb_detach(tx_urb);
				urb_append(&endpoint->done, tx_urb);
				/*usbdbg("done->next %p, tx_urb %p, done %p", */
				/*	 endpoint->done.next, tx_urb, &endpoint->done); */

				endpoint->tx_urb = first_urb_detached(&endpoint->tx);
				if( endpoint->tx_urb ) {
					endpoint->tx_queue--;
					usbdbg("got urb from tx list");
				}
				if( !endpoint->tx_urb ) {
					/*usbdbg("taking urb from done list"); */
					endpoint->tx_urb = first_urb_detached(&endpoint->done);
				}
				if( !endpoint->tx_urb ) {
					usbdbg("allocating new urb for tx_urb");
					endpoint->tx_urb = usbd_alloc_urb(tx_urb->device, endpoint);
				}
			}
		}
	}
}

/* URB linked list functions ***************************************************** */

/*
 * Initialize an urb_link to be a single element list.
 * If the urb_link is being used as a distinguished list head
 * the list is empty when the head is the only link in the list.
 */
void urb_link_init (urb_link * ul)
{
	if (ul) {
		ul->prev = ul->next = ul;
	}
}

/*
 * Detach an urb_link from a list, and set it
 * up as a single element list, so no dangling
 * pointers can be followed, and so it can be
 * joined to another list if so desired.
 */
void urb_detach (struct urb *urb)
{
	if (urb) {
		urb_link *ul = &urb->link;
		ul->next->prev = ul->prev;
		ul->prev->next = ul->next;
		urb_link_init (ul);
	}
}

/*
 * Return the first urb_link in a list with a distinguished
 * head "hd", or NULL if the list is empty.  This will also
 * work as a predicate, returning NULL if empty, and non-NULL
 * otherwise.
 */
urb_link *first_urb_link (urb_link * hd)
{
	urb_link *nx;
	if (NULL != hd && NULL != (nx = hd->next) && nx != hd) {
		/* There is at least one element in the list */
		/* (besides the distinguished head). */
		return (nx);
	}
	/* The list is empty */
	return (NULL);
}

/*
 * Return the first urb in a list with a distinguished
 * head "hd", or NULL if the list is empty.
 */
struct urb *first_urb (urb_link * hd)
{
	urb_link *nx;
	if (NULL == (nx = first_urb_link (hd))) {
		/* The list is empty */
		return (NULL);
	}
	return (p2surround (struct urb, link, nx));
}

/*
 * Detach and return the first urb in a list with a distinguished
 * head "hd", or NULL if the list is empty.
 *
 */
struct urb *first_urb_detached (urb_link * hd)
{
	struct urb *urb;
	if ((urb = first_urb (hd))) {
		urb_detach (urb);
	}
	return urb;
}


/*
 * Append an urb_link (or a whole list of
 * urb_links) to the tail of another list
 * of urb_links.
 */
void urb_append (urb_link * hd, struct urb *urb)
{
	if (hd && urb) {
		urb_link *new = &urb->link;

		/* This allows the new urb to be a list of urbs, */
		/* with new pointing at the first, but the link */
		/* must be initialized. */
		/* Order is important here... */
		urb_link *pul = hd->prev;
		new->prev->next = hd;
		hd->prev = new->prev;
		new->prev = pul;
		pul->next = new;
	}
}

/* URB create/destroy functions ***************************************************** */

/**
 * usbd_alloc_urb - allocate an URB appropriate for specified endpoint
 * @device: device instance
 * @endpoint: endpoint
 *
 * Allocate an urb structure. The usb device urb structure is used to
 * contain all data associated with a transfer, including a setup packet for
 * control transfers.
 *
 * NOTE: endpoint_address MUST contain a direction flag.
 */
struct urb *usbd_alloc_urb (struct usb_device_instance *device,
			    struct usb_endpoint_instance *endpoint)
{
	struct urb *urb;

	if (!(urb = (struct urb *) malloc (sizeof (struct urb)))) {
		usberr (" F A T A L:  malloc(%zu) FAILED!!!!",
			sizeof (struct urb));
		return NULL;
	}

	/* Fill in known fields */
	memset (urb, 0, sizeof (struct urb));
	urb->endpoint = endpoint;
	urb->device = device;
	urb->buffer = (u8 *) urb->buffer_data;
	urb->buffer_length = sizeof (urb->buffer_data);

	urb_link_init (&urb->link);

	return urb;
}

/**
 * usbd_dealloc_urb - deallocate an URB and associated buffer
 * @urb: pointer to an urb structure
 *
 * Deallocate an urb structure and associated data.
 */
void usbd_dealloc_urb (struct urb *urb)
{
	if (urb) {
		free (urb);
	}
}

/* Event signaling functions ***************************************************** */

/**
 * usbd_device_event - called to respond to various usb events
 * @device: pointer to struct device
 * @event: event to respond to
 *
 * Used by a Bus driver to indicate an event.
 */
void usbd_device_event_irq (struct usb_device_instance *device, usb_device_event_t event, int data)
{
	usb_device_state_t state;

	if (!device || !device->bus) {
		usberr("(%p,%d) NULL device or device->bus", device, event);
		return;
	}

	state = device->device_state;

	usbinfo("%s", usbd_device_events[event]);

	switch (event) {
	case DEVICE_UNKNOWN:
		break;
	case DEVICE_INIT:
		device->device_state = STATE_INIT;
		break;

	case DEVICE_CREATE:
		device->device_state = STATE_ATTACHED;
		break;

	case DEVICE_HUB_CONFIGURED:
		device->device_state = STATE_POWERED;
		break;

	case DEVICE_RESET:
		device->device_state = STATE_DEFAULT;
		device->address = 0;
		break;

	case DEVICE_ADDRESS_ASSIGNED:
		device->device_state = STATE_ADDRESSED;
		break;

	case DEVICE_CONFIGURED:
		device->device_state = STATE_CONFIGURED;
		break;

	case DEVICE_DE_CONFIGURED:
		device->device_state = STATE_ADDRESSED;
		break;

	case DEVICE_BUS_INACTIVE:
		if (device->status != USBD_CLOSING) {
			device->status = USBD_SUSPENDED;
		}
		break;
	case DEVICE_BUS_ACTIVITY:
		if (device->status != USBD_CLOSING) {
			device->status = USBD_OK;
		}
		break;

	case DEVICE_SET_INTERFACE:
		break;
	case DEVICE_SET_FEATURE:
		break;
	case DEVICE_CLEAR_FEATURE:
		break;

	case DEVICE_POWER_INTERRUPTION:
		device->device_state = STATE_POWERED;
		break;
	case DEVICE_HUB_RESET:
		device->device_state = STATE_ATTACHED;
		break;
	case DEVICE_DESTROY:
		device->device_state = STATE_UNKNOWN;
		break;

	case DEVICE_FUNCTION_PRIVATE:
		break;

	default:
		usbdbg("event %d - not handled",event);
		break;
	}
	debug("%s event: %d oldstate: %d newstate: %d status: %d address: %d",
		device->name, event, state,
		device->device_state, device->status, device->address);

	/* tell the bus interface driver */
	if( device->event ) {
		/* usbdbg("calling device->event"); */
		device->event(device, event, data);
	}
}
