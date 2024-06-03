/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2003
 * Gerry Hamel, geh@ti.com, Texas Instruments
 *
 * Based on linux/drivers/usbd/usbd.h
 *
 * Copyright (c) 2000, 2001, 2002 Lineo
 * Copyright (c) 2001 Hewlett Packard
 *
 * By:
 *	Stuart Lynne <sl@lineo.com>,
 *	Tom Rushworth <tbr@lineo.com>,
 *	Bruce Balden <balden@lineo.com>
 */

#ifndef __USBDCORE_H__
#define __USBDCORE_H__

#include <common.h>
#include "usbdescriptors.h"


#define MAX_URBS_QUEUED 5


#if 1
#define usberr(fmt,args...) serial_printf("ERROR: %s(), %d: "fmt"\n",__FUNCTION__,__LINE__,##args)
#else
#define usberr(fmt,args...) do{}while(0)
#endif

#if 0
#define usbdbg(fmt,args...) serial_printf("debug: %s(), %d: "fmt"\n",__FUNCTION__,__LINE__,##args)
#else
#define usbdbg(fmt,args...) do{}while(0)
#endif

#if 0
#define usbinfo(fmt,args...) serial_printf("info: %s(), %d: "fmt"\n",__FUNCTION__,__LINE__,##args)
#else
#define usbinfo(fmt,args...) do{}while(0)
#endif

#ifndef le16_to_cpu
#define le16_to_cpu(x)	(x)
#endif

#ifndef inb
#define inb(p)	     (*(volatile u8*)(p))
#endif

#ifndef outb
#define outb(val,p)  (*(volatile u8*)(p) = (val))
#endif

#ifndef inw
#define inw(p)	     (*(volatile u16*)(p))
#endif

#ifndef outw
#define outw(val,p)  (*(volatile u16*)(p) = (val))
#endif

#ifndef inl
#define inl(p)	     (*(volatile u32*)(p))
#endif

#ifndef outl
#define outl(val,p)  (*(volatile u32*)(p) = (val))
#endif

#ifndef insw
#define insw(p,to,len)	   mmio_insw(p,to,len)
#endif

#ifndef outsw
#define outsw(p,from,len)  mmio_outsw(p,from,len)
#endif

#ifndef insb
#define insb(p,to,len)	   mmio_insb(p,to,len)
#endif

#ifndef mmio_insw
#define mmio_insw(r,b,l)	({	int __i ;  \
					u16 *__b2;  \
					__b2 = (u16 *) b;  \
					for (__i = 0; __i < l; __i++) {	 \
					  *(__b2 + __i) = inw(r);  \
					};  \
				})
#endif

#ifndef mmio_outsw
#define mmio_outsw(r,b,l)	({	int __i; \
					u16 *__b2; \
					__b2 = (u16 *) b; \
					for (__i = 0; __i < l; __i++) { \
					    outw( *(__b2 + __i), r); \
					} \
				})
#endif

#ifndef mmio_insb
#define mmio_insb(r,b,l)	({	int __i ;  \
					u8 *__b2;  \
					__b2 = (u8 *) b;  \
					for (__i = 0; __i < l; __i++) {	 \
					  *(__b2 + __i) = inb(r);  \
					};  \
				})
#endif

/*
 * Structure member address manipulation macros.
 * These are used by client code (code using the urb_link routines), since
 * the urb_link structure is embedded in the client data structures.
 *
 * Note: a macro offsetof equivalent to member_offset is defined in stddef.h
 *	 but this is kept here for the sake of portability.
 *
 * p2surround returns a pointer to the surrounding structure given
 * type of the surrounding structure, the name memb of the structure
 * member pointed at by ptr.  For example, if you have:
 *
 *	struct foo {
 *	    int x;
 *	    float y;
 *	    char z;
 *	} thingy;
 *
 *	char *cp = &thingy.z;
 *
 * then
 *
 *	&thingy == p2surround(struct foo, z, cp)
 *
 * Clear?
 */
#define _cv_(ptr)		  ((char*)(void*)(ptr))
#define member_offset(type,memb)  (_cv_(&(((type*)0)->memb))-(char*)0)
#define p2surround(type,memb,ptr) ((type*)(void*)(_cv_(ptr)-member_offset(type,memb)))

struct urb;

struct usb_endpoint_instance;
struct usb_interface_instance;
struct usb_configuration_instance;
struct usb_device_instance;
struct usb_bus_instance;

/*
 * Device and/or Interface Class codes
 */
#define USB_CLASS_PER_INTERFACE		0	/* for DeviceClass */
#define USB_CLASS_AUDIO			1
#define USB_CLASS_COMM			2
#define USB_CLASS_HID			3
#define USB_CLASS_PHYSICAL		5
#define USB_CLASS_PRINTER		7
#define USB_CLASS_MASS_STORAGE		8
#define USB_CLASS_HUB			9
#define USB_CLASS_DATA			10
#define USB_CLASS_APP_SPEC		0xfe
#define USB_CLASS_VENDOR_SPEC		0xff

/*
 * USB types
 */
#define USB_TYPE_STANDARD		(0x00 << 5)
#define USB_TYPE_CLASS			(0x01 << 5)
#define USB_TYPE_VENDOR			(0x02 << 5)
#define USB_TYPE_RESERVED		(0x03 << 5)

/*
 * USB recipients
 */
#define USB_RECIP_DEVICE		0x00
#define USB_RECIP_INTERFACE		0x01
#define USB_RECIP_ENDPOINT		0x02
#define USB_RECIP_OTHER			0x03

/*
 * USB directions
 */
#define USB_DIR_OUT			0
#define USB_DIR_IN			0x80

/*
 * Descriptor types
 */
#define USB_DT_DEVICE			0x01
#define USB_DT_CONFIG			0x02
#define USB_DT_STRING			0x03
#define USB_DT_INTERFACE		0x04
#define USB_DT_ENDPOINT			0x05

#if defined(CONFIG_USBD_HS)
#define USB_DT_QUAL			0x06
#endif

#define USB_DT_HID			(USB_TYPE_CLASS | 0x01)
#define USB_DT_REPORT			(USB_TYPE_CLASS | 0x02)
#define USB_DT_PHYSICAL			(USB_TYPE_CLASS | 0x03)
#define USB_DT_HUB			(USB_TYPE_CLASS | 0x09)

/*
 * Descriptor sizes per descriptor type
 */
#define USB_DT_DEVICE_SIZE		18
#define USB_DT_CONFIG_SIZE		9
#define USB_DT_INTERFACE_SIZE		9
#define USB_DT_ENDPOINT_SIZE		7
#define USB_DT_ENDPOINT_AUDIO_SIZE	9	/* Audio extension */
#define USB_DT_HUB_NONVAR_SIZE		7
#define USB_DT_HID_SIZE			9

/*
 * Endpoints
 */
#define USB_ENDPOINT_NUMBER_MASK	0x0f	/* in bEndpointAddress */
#define USB_ENDPOINT_DIR_MASK		0x80

#define USB_ENDPOINT_XFERTYPE_MASK	0x03	/* in bmAttributes */
#define USB_ENDPOINT_XFER_CONTROL	0
#define USB_ENDPOINT_XFER_ISOC		1
#define USB_ENDPOINT_XFER_BULK		2
#define USB_ENDPOINT_XFER_INT		3

/*
 * USB Packet IDs (PIDs)
 */
#define USB_PID_UNDEF_0			       0xf0
#define USB_PID_OUT			       0xe1
#define USB_PID_ACK			       0xd2
#define USB_PID_DATA0			       0xc3
#define USB_PID_PING			       0xb4	/* USB 2.0 */
#define USB_PID_SOF			       0xa5
#define USB_PID_NYET			       0x96	/* USB 2.0 */
#define USB_PID_DATA2			       0x87	/* USB 2.0 */
#define USB_PID_SPLIT			       0x78	/* USB 2.0 */
#define USB_PID_IN			       0x69
#define USB_PID_NAK			       0x5a
#define USB_PID_DATA1			       0x4b
#define USB_PID_PREAMBLE		       0x3c	/* Token mode */
#define USB_PID_ERR			       0x3c	/* USB 2.0: handshake mode */
#define USB_PID_SETUP			       0x2d
#define USB_PID_STALL			       0x1e
#define USB_PID_MDATA			       0x0f	/* USB 2.0 */

/*
 * Standard requests
 */
#define USB_REQ_GET_STATUS		0x00
#define USB_REQ_CLEAR_FEATURE		0x01
#define USB_REQ_SET_FEATURE		0x03
#define USB_REQ_SET_ADDRESS		0x05
#define USB_REQ_GET_DESCRIPTOR		0x06
#define USB_REQ_SET_DESCRIPTOR		0x07
#define USB_REQ_GET_CONFIGURATION	0x08
#define USB_REQ_SET_CONFIGURATION	0x09
#define USB_REQ_GET_INTERFACE		0x0A
#define USB_REQ_SET_INTERFACE		0x0B
#define USB_REQ_SYNCH_FRAME		0x0C

#define USBD_DEVICE_REQUESTS(x) (((unsigned int)x <= USB_REQ_SYNCH_FRAME) ? usbd_device_requests[x] : "UNKNOWN")

/*
 * HID requests
 */
#define USB_REQ_GET_REPORT		0x01
#define USB_REQ_GET_IDLE		0x02
#define USB_REQ_GET_PROTOCOL		0x03
#define USB_REQ_SET_REPORT		0x09
#define USB_REQ_SET_IDLE		0x0A
#define USB_REQ_SET_PROTOCOL		0x0B


/*
 * USB Spec Release number
 */

#if defined(CONFIG_USBD_HS)
#define USB_BCD_VERSION			0x0200
#else
#define USB_BCD_VERSION			0x0110
#endif


/*
 * Device Requests	(c.f Table 9-2)
 */

#define USB_REQ_DIRECTION_MASK		0x80
#define USB_REQ_TYPE_MASK		0x60
#define USB_REQ_RECIPIENT_MASK		0x1f

#define USB_REQ_DEVICE2HOST		0x80
#define USB_REQ_HOST2DEVICE		0x00

#define USB_REQ_TYPE_STANDARD		0x00
#define USB_REQ_TYPE_CLASS		0x20
#define USB_REQ_TYPE_VENDOR		0x40

#define USB_REQ_RECIPIENT_DEVICE	0x00
#define USB_REQ_RECIPIENT_INTERFACE	0x01
#define USB_REQ_RECIPIENT_ENDPOINT	0x02
#define USB_REQ_RECIPIENT_OTHER		0x03

/*
 * get status bits
 */

#define USB_STATUS_SELFPOWERED		0x01
#define USB_STATUS_REMOTEWAKEUP		0x02

#define USB_STATUS_HALT			0x01

/*
 * descriptor types
 */

#define USB_DESCRIPTOR_TYPE_DEVICE			0x01
#define USB_DESCRIPTOR_TYPE_CONFIGURATION		0x02
#define USB_DESCRIPTOR_TYPE_STRING			0x03
#define USB_DESCRIPTOR_TYPE_INTERFACE			0x04
#define USB_DESCRIPTOR_TYPE_ENDPOINT			0x05
#define USB_DESCRIPTOR_TYPE_DEVICE_QUALIFIER		0x06
#define USB_DESCRIPTOR_TYPE_OTHER_SPEED_CONFIGURATION	0x07
#define USB_DESCRIPTOR_TYPE_INTERFACE_POWER		0x08
#define USB_DESCRIPTOR_TYPE_HID				0x21
#define USB_DESCRIPTOR_TYPE_REPORT			0x22

#define USBD_DEVICE_DESCRIPTORS(x) (((unsigned int)x <= USB_DESCRIPTOR_TYPE_INTERFACE_POWER) ? \
		usbd_device_descriptors[x] : "UNKNOWN")

/*
 * standard feature selectors
 */
#define USB_ENDPOINT_HALT		0x00
#define USB_DEVICE_REMOTE_WAKEUP	0x01
#define USB_TEST_MODE			0x02


/* USB Requests
 *
 */

struct usb_device_request {
	u8 bmRequestType;
	u8 bRequest;
	u16 wValue;
	u16 wIndex;
	u16 wLength;
} __attribute__ ((packed));


/* USB Status
 *
 */
typedef enum urb_send_status {
	SEND_IN_PROGRESS,
	SEND_FINISHED_OK,
	SEND_FINISHED_ERROR,
	RECV_READY,
	RECV_OK,
	RECV_ERROR
} urb_send_status_t;

/*
 * Device State (c.f USB Spec 2.0 Figure 9-1)
 *
 * What state the usb device is in.
 *
 * Note the state does not change if the device is suspended, we simply set a
 * flag to show that it is suspended.
 *
 */
typedef enum usb_device_state {
	STATE_INIT,		/* just initialized */
	STATE_CREATED,		/* just created */
	STATE_ATTACHED,		/* we are attached */
	STATE_POWERED,		/* we have seen power indication (electrical bus signal) */
	STATE_DEFAULT,		/* we been reset */
	STATE_ADDRESSED,	/* we have been addressed (in default configuration) */
	STATE_CONFIGURED,	/* we have seen a set configuration device command */
	STATE_UNKNOWN,		/* destroyed */
} usb_device_state_t;

#define USBD_DEVICE_STATE(x) (((unsigned int)x <= STATE_UNKNOWN) ? usbd_device_states[x] : "UNKNOWN")

/*
 * Device status
 *
 * Overall state
 */
typedef enum usb_device_status {
	USBD_OPENING,		/* we are currently opening */
	USBD_OK,		/* ok to use */
	USBD_SUSPENDED,		/* we are currently suspended */
	USBD_CLOSING,		/* we are currently closing */
} usb_device_status_t;

#define USBD_DEVICE_STATUS(x) (((unsigned int)x <= USBD_CLOSING) ? usbd_device_status[x] : "UNKNOWN")

/*
 * Device Events
 *
 * These are defined in the USB Spec (c.f USB Spec 2.0 Figure 9-1).
 *
 * There are additional events defined to handle some extra actions we need
 * to have handled.
 *
 */
typedef enum usb_device_event {

	DEVICE_UNKNOWN,		/* bi - unknown event */
	DEVICE_INIT,		/* bi  - initialize */
	DEVICE_CREATE,		/* bi  - */
	DEVICE_HUB_CONFIGURED,	/* bi  - bus has been plugged int */
	DEVICE_RESET,		/* bi  - hub has powered our port */

	DEVICE_ADDRESS_ASSIGNED,	/* ep0 - set address setup received */
	DEVICE_CONFIGURED,	/* ep0 - set configure setup received */
	DEVICE_SET_INTERFACE,	/* ep0 - set interface setup received */

	DEVICE_SET_FEATURE,	/* ep0 - set feature setup received */
	DEVICE_CLEAR_FEATURE,	/* ep0 - clear feature setup received */

	DEVICE_DE_CONFIGURED,	/* ep0 - set configure setup received for ?? */

	DEVICE_BUS_INACTIVE,	/* bi  - bus in inactive (no SOF packets) */
	DEVICE_BUS_ACTIVITY,	/* bi  - bus is active again */

	DEVICE_POWER_INTERRUPTION,	/* bi  - hub has depowered our port */
	DEVICE_HUB_RESET,	/* bi  - bus has been unplugged */
	DEVICE_DESTROY,		/* bi  - device instance should be destroyed */

	DEVICE_HOTPLUG,		/* bi  - a hotplug event has occurred */

	DEVICE_FUNCTION_PRIVATE,	/* function - private */

} usb_device_event_t;


typedef struct urb_link {
	struct urb_link *next;
	struct urb_link *prev;
} urb_link;

/* USB Data structure - for passing data around.
 *
 * This is used for both sending and receiving data.
 *
 * The callback function is used to let the function driver know when
 * transmitted data has been sent.
 *
 * The callback function is set by the alloc_recv function when an urb is
 * allocated for receiving data for an endpoint and used to call the
 * function driver to inform it that data has arrived.
 */

/* in linux we'd malloc this, but in u-boot we prefer static data */
#define URB_BUF_SIZE 512

struct urb {

	struct usb_endpoint_instance *endpoint;
	struct usb_device_instance *device;

	struct usb_device_request device_request;	/* contents of received SETUP packet */

	struct urb_link link;	/* embedded struct for circular doubly linked list of urbs */

	u8* buffer;
	unsigned int buffer_length;
	unsigned int actual_length;

	urb_send_status_t status;
	int data;

	u16 buffer_data[URB_BUF_SIZE];	/* data received (OUT) or being sent (IN) */
};

/* Endpoint configuration
 *
 * Per endpoint configuration data. Used to track which function driver owns
 * an endpoint.
 *
 */
struct usb_endpoint_instance {
	int endpoint_address;	/* logical endpoint address */

	/* control */
	int status;		/* halted */
	int state;		/* available for use by bus interface driver */

	/* receive side */
	struct urb_link rcv;	/* received urbs */
	struct urb_link rdy;	/* empty urbs ready to receive */
	struct urb *rcv_urb;	/* active urb */
	int rcv_attributes;	/* copy of bmAttributes from endpoint descriptor */
	int rcv_packetSize;	/* maximum packet size from endpoint descriptor */
	int rcv_transferSize;	/* maximum transfer size from function driver */
	int rcv_queue;

	/* transmit side */
	struct urb_link tx;	/* urbs ready to transmit */
	struct urb_link done;	/* transmitted urbs */
	struct urb *tx_urb;	/* active urb */
	int tx_attributes;	/* copy of bmAttributes from endpoint descriptor */
	int tx_packetSize;	/* maximum packet size from endpoint descriptor */
	int tx_transferSize;	/* maximum transfer size from function driver */
	int tx_queue;

	int sent;		/* data already sent */
	int last;		/* data sent in last packet XXX do we need this */
};

struct usb_alternate_instance {
	struct usb_interface_descriptor *interface_descriptor;

	int endpoints;
	int *endpoint_transfersize_array;
	struct usb_endpoint_descriptor **endpoints_descriptor_array;
};

struct usb_interface_instance {
	int alternates;
	struct usb_alternate_instance *alternates_instance_array;
};

struct usb_configuration_instance {
	int interfaces;
	struct usb_configuration_descriptor *configuration_descriptor;
	struct usb_interface_instance *interface_instance_array;
};


/* USB Device Instance
 *
 * For each physical bus interface we create a logical device structure. This
 * tracks all of the required state to track the USB HOST's view of the device.
 *
 * Keep track of the device configuration for a real physical bus interface,
 * this includes the bus interface, multiple function drivers, the current
 * configuration and the current state.
 *
 * This will show:
 *	the specific bus interface driver
 *	the default endpoint 0 driver
 *	the configured function driver
 *	device state
 *	device status
 *	endpoint list
 */

struct usb_device_instance {

	/* generic */
	char *name;
	struct usb_device_descriptor *device_descriptor;	/* per device descriptor */
#if defined(CONFIG_USBD_HS)
	struct usb_qualifier_descriptor *qualifier_descriptor;
#endif

	void (*event) (struct usb_device_instance *device, usb_device_event_t event, int data);

	/* Do cdc device specific control requests */
	int (*cdc_recv_setup)(struct usb_device_request *request, struct urb *urb);

	/* bus interface */
	struct usb_bus_instance *bus;	/* which bus interface driver */

	/* configuration descriptors */
	int configurations;
	struct usb_configuration_instance *configuration_instance_array;

	/* device state */
	usb_device_state_t device_state;	/* current USB Device state */
	usb_device_state_t device_previous_state;	/* current USB Device state */

	u8 address;		/* current address (zero is default) */
	u8 configuration;	/* current show configuration (zero is default) */
	u8 interface;		/* current interface (zero is default) */
	u8 alternate;		/* alternate flag */

	usb_device_status_t status;	/* device status */

	int urbs_queued;	/* number of submitted urbs */

	/* Shouldn't need to make this atomic, all we need is a change indicator */
	unsigned long usbd_rxtx_timestamp;
	unsigned long usbd_last_rxtx_timestamp;

};

/* Bus Interface configuration structure
 *
 * This is allocated for each configured instance of a bus interface driver.
 *
 * The privdata pointer may be used by the bus interface driver to store private
 * per instance state information.
 */
struct usb_bus_instance {

	struct usb_device_instance *device;
	struct usb_endpoint_instance *endpoint_array;	/* array of available configured endpoints */

	int max_endpoints;	/* maximimum number of rx enpoints */
	unsigned char			maxpacketsize;

	unsigned int serial_number;
	char *serial_number_str;
	void *privdata;		/* private data for the bus interface */

};

extern char *usbd_device_events[];
extern char *usbd_device_states[];
extern char *usbd_device_status[];
extern char *usbd_device_requests[];
extern char *usbd_device_descriptors[];

void urb_link_init (urb_link * ul);
void urb_detach (struct urb *urb);
urb_link *first_urb_link (urb_link * hd);
struct urb *first_urb (urb_link * hd);
struct urb *first_urb_detached (urb_link * hd);
void urb_append (urb_link * hd, struct urb *urb);

struct urb *usbd_alloc_urb (struct usb_device_instance *device, struct usb_endpoint_instance *endpoint);
void	    usbd_dealloc_urb (struct urb *urb);

/*
 * usbd_device_event is used by bus interface drivers to tell the higher layers that
 * certain events have taken place.
 */
void usbd_device_event_irq (struct usb_device_instance *conf, usb_device_event_t, int);
void usbd_device_event (struct usb_device_instance *conf, usb_device_event_t, int);

/* descriptors
 *
 * Various ways of finding descriptors based on the current device and any
 * possible configuration / interface / endpoint for it.
 */
struct usb_configuration_descriptor *usbd_device_configuration_descriptor (struct usb_device_instance *, int, int);
struct usb_function_instance *usbd_device_function_instance (struct usb_device_instance *, unsigned int);
struct usb_interface_instance *usbd_device_interface_instance (struct usb_device_instance *, int, int, int);
struct usb_alternate_instance *usbd_device_alternate_instance (struct usb_device_instance *, int, int, int, int);
struct usb_interface_descriptor *usbd_device_interface_descriptor (struct usb_device_instance *, int, int, int, int);
struct usb_endpoint_descriptor *usbd_device_endpoint_descriptor_index (struct usb_device_instance *, int, int, int, int, int);
struct usb_class_descriptor *usbd_device_class_descriptor_index (struct usb_device_instance *, int, int, int, int, int);
struct usb_class_report_descriptor *usbd_device_class_report_descriptor_index( struct usb_device_instance *, int , int , int , int , int );
struct usb_endpoint_descriptor *usbd_device_endpoint_descriptor (struct usb_device_instance *, int, int, int, int, int);
int				usbd_device_endpoint_transfersize (struct usb_device_instance *, int, int, int, int, int);
struct usb_string_descriptor *usbd_get_string (u8);
struct usb_device_descriptor *usbd_device_device_descriptor(struct
		usb_device_instance *, int);

#if defined(CONFIG_USBD_HS)
/*
 * is_usbd_high_speed routine needs to be defined by specific gadget driver
 * It returns true if device enumerates at High speed
 * Retuns false otherwise
 */
int is_usbd_high_speed(void);
#endif
int usbd_endpoint_halted (struct usb_device_instance *device, int endpoint);
void usbd_rcv_complete(struct usb_endpoint_instance *endpoint, int len, int urb_bad);
void usbd_tx_complete (struct usb_endpoint_instance *endpoint);

/* These are macros used in debugging */
#ifdef DEBUG
static inline void print_urb(struct urb *u)
{
	serial_printf("urb %p\n", (u));
	serial_printf("\tendpoint %p\n", u->endpoint);
	serial_printf("\tdevice %p\n", u->device);
	serial_printf("\tbuffer %p\n", u->buffer);
	serial_printf("\tbuffer_length %d\n", u->buffer_length);
	serial_printf("\tactual_length %d\n", u->actual_length);
	serial_printf("\tstatus %d\n", u->status);
	serial_printf("\tdata %d\n", u->data);
}

static inline void print_usb_device_request(struct usb_device_request *r)
{
	serial_printf("usb request\n");
	serial_printf("\tbmRequestType 0x%2.2x\n", r->bmRequestType);
	if ((r->bmRequestType & USB_REQ_DIRECTION_MASK) == 0)
		serial_printf("\t\tDirection : To device\n");
	else
		serial_printf("\t\tDirection : To host\n");
	if ((r->bmRequestType & USB_TYPE_STANDARD) == USB_TYPE_STANDARD)
		serial_printf("\t\tType      : Standard\n");
	if ((r->bmRequestType & USB_TYPE_CLASS) == USB_TYPE_CLASS)
		serial_printf("\t\tType      : Standard\n");
	if ((r->bmRequestType & USB_TYPE_VENDOR) == USB_TYPE_VENDOR)
		serial_printf("\t\tType      : Standard\n");
	if ((r->bmRequestType & USB_TYPE_RESERVED) == USB_TYPE_RESERVED)
		serial_printf("\t\tType      : Standard\n");
	if ((r->bmRequestType & USB_REQ_RECIPIENT_MASK) ==
	    USB_REQ_RECIPIENT_DEVICE)
		serial_printf("\t\tRecipient : Device\n");
	if ((r->bmRequestType & USB_REQ_RECIPIENT_MASK) ==
	    USB_REQ_RECIPIENT_INTERFACE)
		serial_printf("\t\tRecipient : Interface\n");
	if ((r->bmRequestType & USB_REQ_RECIPIENT_MASK) ==
	    USB_REQ_RECIPIENT_ENDPOINT)
		serial_printf("\t\tRecipient : Endpoint\n");
	if ((r->bmRequestType & USB_REQ_RECIPIENT_MASK) ==
	    USB_REQ_RECIPIENT_OTHER)
		serial_printf("\t\tRecipient : Other\n");
	serial_printf("\tbRequest      0x%2.2x\n", r->bRequest);
	if (r->bRequest == USB_REQ_GET_STATUS)
		serial_printf("\t\tGET_STATUS\n");
	else if (r->bRequest == USB_REQ_SET_ADDRESS)
		serial_printf("\t\tSET_ADDRESS\n");
	else if (r->bRequest == USB_REQ_SET_FEATURE)
		serial_printf("\t\tSET_FEATURE\n");
	else if (r->bRequest == USB_REQ_GET_DESCRIPTOR)
		serial_printf("\t\tGET_DESCRIPTOR\n");
	else if (r->bRequest == USB_REQ_SET_CONFIGURATION)
		serial_printf("\t\tSET_CONFIGURATION\n");
	else if (r->bRequest == USB_REQ_SET_INTERFACE)
		serial_printf("\t\tUSB_REQ_SET_INTERFACE\n");
	else
		serial_printf("\tUNKNOWN %d\n", r->bRequest);
	serial_printf("\twValue        0x%4.4x\n", r->wValue);
	if (r->bRequest == USB_REQ_GET_DESCRIPTOR) {
		switch (r->wValue >> 8) {
		case USB_DESCRIPTOR_TYPE_DEVICE:
			serial_printf("\tDEVICE\n");
			break;
		case USB_DESCRIPTOR_TYPE_CONFIGURATION:
			serial_printf("\tCONFIGURATION\n");
			break;
		case USB_DESCRIPTOR_TYPE_STRING:
			serial_printf("\tSTRING\n");
			break;
		case USB_DESCRIPTOR_TYPE_INTERFACE:
			serial_printf("\tINTERFACE\n");
			break;
		case USB_DESCRIPTOR_TYPE_ENDPOINT:
			serial_printf("\tENDPOINT\n");
			break;
		case USB_DESCRIPTOR_TYPE_DEVICE_QUALIFIER:
			serial_printf("\tDEVICE_QUALIFIER\n");
			break;
		case USB_DESCRIPTOR_TYPE_OTHER_SPEED_CONFIGURATION:
			serial_printf("\tOTHER_SPEED_CONFIGURATION\n");
			break;
		case USB_DESCRIPTOR_TYPE_INTERFACE_POWER:
			serial_printf("\tINTERFACE_POWER\n");
			break;
		case USB_DESCRIPTOR_TYPE_HID:
			serial_printf("\tHID\n");
			break;
		case USB_DESCRIPTOR_TYPE_REPORT:
			serial_printf("\tREPORT\n");
			break;
		default:
			serial_printf("\tUNKNOWN TYPE\n");
			break;
		}
	}
	serial_printf("\twIndex        0x%4.4x\n", r->wIndex);
	serial_printf("\twLength       0x%4.4x\n", r->wLength);
}
#else
/* stubs */
#define print_urb(u)
#define print_usb_device_request(r)
#endif /* DEBUG */
#endif
