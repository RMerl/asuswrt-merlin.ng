/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2001
 * Denis Peter, MPL AG Switzerland
 *
 * Adapted for U-Boot driver model
 * (C) Copyright 2015 Google, Inc
 * Note: Part of this code has been derived from linux
 *
 */
#ifndef _USB_H_
#define _USB_H_

#include <fdtdec.h>
#include <usb_defs.h>
#include <linux/usb/ch9.h>
#include <asm/cache.h>
#include <part.h>

/*
 * The EHCI spec says that we must align to at least 32 bytes.  However,
 * some platforms require larger alignment.
 */
#if ARCH_DMA_MINALIGN > 32
#define USB_DMA_MINALIGN	ARCH_DMA_MINALIGN
#else
#define USB_DMA_MINALIGN	32
#endif

/* Everything is aribtrary */
#define USB_ALTSETTINGALLOC		4
#define USB_MAXALTSETTING		128	/* Hard limit */

#define USB_MAX_DEVICE			32
#define USB_MAXCONFIG			8
#define USB_MAXINTERFACES		8
#define USB_MAXENDPOINTS		16
#define USB_MAXCHILDREN			8	/* This is arbitrary */
#define USB_MAX_HUB			16

#define USB_CNTL_TIMEOUT 100 /* 100ms timeout */

/*
 * This is the timeout to allow for submitting an urb in ms. We allow more
 * time for a BULK device to react - some are slow.
 */
#define USB_TIMEOUT_MS(pipe) (usb_pipebulk(pipe) ? 5000 : 1000)

/* device request (setup) */
struct devrequest {
	__u8	requesttype;
	__u8	request;
	__le16	value;
	__le16	index;
	__le16	length;
} __attribute__ ((packed));

/* Interface */
struct usb_interface {
	struct usb_interface_descriptor desc;

	__u8	no_of_ep;
	__u8	num_altsetting;
	__u8	act_altsetting;

	struct usb_endpoint_descriptor ep_desc[USB_MAXENDPOINTS];
	/*
	 * Super Speed Device will have Super Speed Endpoint
	 * Companion Descriptor  (section 9.6.7 of usb 3.0 spec)
	 * Revision 1.0 June 6th 2011
	 */
	struct usb_ss_ep_comp_descriptor ss_ep_comp_desc[USB_MAXENDPOINTS];
} __attribute__ ((packed));

/* Configuration information.. */
struct usb_config {
	struct usb_config_descriptor desc;

	__u8	no_of_if;	/* number of interfaces */
	struct usb_interface if_desc[USB_MAXINTERFACES];
} __attribute__ ((packed));

enum {
	/* Maximum packet size; encoded as 0,1,2,3 = 8,16,32,64 */
	PACKET_SIZE_8   = 0,
	PACKET_SIZE_16  = 1,
	PACKET_SIZE_32  = 2,
	PACKET_SIZE_64  = 3,
};

/**
 * struct usb_device - information about a USB device
 *
 * With driver model both UCLASS_USB (the USB controllers) and UCLASS_USB_HUB
 * (the hubs) have this as parent data. Hubs are children of controllers or
 * other hubs and there is always a single root hub for each controller.
 * Therefore struct usb_device can always be accessed with
 * dev_get_parent_priv(dev), where dev is a USB device.
 *
 * Pointers exist for obtaining both the device (could be any uclass) and
 * controller (UCLASS_USB) from this structure. The controller does not have
 * a struct usb_device since it is not a device.
 */
struct usb_device {
	int	devnum;			/* Device number on USB bus */
	int	speed;			/* full/low/high */
	char	mf[32];			/* manufacturer */
	char	prod[32];		/* product */
	char	serial[32];		/* serial number */

	/* Maximum packet size; one of: PACKET_SIZE_* */
	int maxpacketsize;
	/* one bit for each endpoint ([0] = IN, [1] = OUT) */
	unsigned int toggle[2];
	/* endpoint halts; one bit per endpoint # & direction;
	 * [0] = IN, [1] = OUT
	 */
	unsigned int halted[2];
	int epmaxpacketin[16];		/* INput endpoint specific maximums */
	int epmaxpacketout[16];		/* OUTput endpoint specific maximums */

	int configno;			/* selected config number */
	/* Device Descriptor */
	struct usb_device_descriptor descriptor
		__attribute__((aligned(ARCH_DMA_MINALIGN)));
	struct usb_config config; /* config descriptor */

	int have_langid;		/* whether string_langid is valid yet */
	int string_langid;		/* language ID for strings */
	int (*irq_handle)(struct usb_device *dev);
	unsigned long irq_status;
	int irq_act_len;		/* transferred bytes */
	void *privptr;
	/*
	 * Child devices -  if this is a hub device
	 * Each instance needs its own set of data structures.
	 */
	unsigned long status;
	unsigned long int_pending;	/* 1 bit per ep, used by int_queue */
	int act_len;			/* transferred bytes */
	int maxchild;			/* Number of ports if hub */
	int portnr;			/* Port number, 1=first */
#if !CONFIG_IS_ENABLED(DM_USB)
	/* parent hub, or NULL if this is the root hub */
	struct usb_device *parent;
	struct usb_device *children[USB_MAXCHILDREN];
	void *controller;		/* hardware controller private data */
#endif
	/* slot_id - for xHCI enabled devices */
	unsigned int slot_id;
#if CONFIG_IS_ENABLED(DM_USB)
	struct udevice *dev;		/* Pointer to associated device */
	struct udevice *controller_dev;	/* Pointer to associated controller */
#endif
};

struct int_queue;

/*
 * You can initialize platform's USB host or device
 * ports by passing this enum as an argument to
 * board_usb_init().
 */
enum usb_init_type {
	USB_INIT_HOST,
	USB_INIT_DEVICE
};

/**********************************************************************
 * this is how the lowlevel part communicate with the outer world
 */

int usb_lowlevel_init(int index, enum usb_init_type init, void **controller);
int usb_lowlevel_stop(int index);

#if defined(CONFIG_USB_MUSB_HOST) || CONFIG_IS_ENABLED(DM_USB)
int usb_reset_root_port(struct usb_device *dev);
#else
#define usb_reset_root_port(dev)
#endif

int submit_bulk_msg(struct usb_device *dev, unsigned long pipe,
			void *buffer, int transfer_len);
int submit_control_msg(struct usb_device *dev, unsigned long pipe, void *buffer,
			int transfer_len, struct devrequest *setup);
int submit_int_msg(struct usb_device *dev, unsigned long pipe, void *buffer,
			int transfer_len, int interval);

#if defined CONFIG_USB_EHCI_HCD || defined CONFIG_USB_MUSB_HOST \
	|| CONFIG_IS_ENABLED(DM_USB)
struct int_queue *create_int_queue(struct usb_device *dev, unsigned long pipe,
	int queuesize, int elementsize, void *buffer, int interval);
int destroy_int_queue(struct usb_device *dev, struct int_queue *queue);
void *poll_int_queue(struct usb_device *dev, struct int_queue *queue);
#endif

/* Defines */
#define USB_UHCI_VEND_ID	0x8086
#define USB_UHCI_DEV_ID		0x7112

/*
 * PXA25x can only act as USB device. There are drivers
 * which works with USB CDC gadgets implementations.
 * Some of them have common routines which can be used
 * in boards init functions e.g. udc_disconnect() used for
 * forced device disconnection from host.
 */
extern void udc_disconnect(void);

/*
 * board-specific hardware initialization, called by
 * usb drivers and u-boot commands
 *
 * @param index USB controller number
 * @param init initializes controller as USB host or device
 */
int board_usb_init(int index, enum usb_init_type init);

/*
 * can be used to clean up after failed USB initialization attempt
 * vide: board_usb_init()
 *
 * @param index USB controller number for selective cleanup
 * @param init usb_init_type passed to board_usb_init()
 */
int board_usb_cleanup(int index, enum usb_init_type init);

#ifdef CONFIG_USB_STORAGE

#define USB_MAX_STOR_DEV 7
int usb_stor_scan(int mode);
int usb_stor_info(void);

#endif

#ifdef CONFIG_USB_HOST_ETHER

#define USB_MAX_ETH_DEV 5
int usb_host_eth_scan(int mode);

#endif

#ifdef CONFIG_USB_KEYBOARD

int drv_usb_kbd_init(void);
int usb_kbd_deregister(int force);

#endif
/* routines */
int usb_init(void); /* initialize the USB Controller */
int usb_stop(void); /* stop the USB Controller */
int usb_detect_change(void); /* detect if a USB device has been (un)plugged */


int usb_set_protocol(struct usb_device *dev, int ifnum, int protocol);
int usb_set_idle(struct usb_device *dev, int ifnum, int duration,
			int report_id);
int usb_control_msg(struct usb_device *dev, unsigned int pipe,
			unsigned char request, unsigned char requesttype,
			unsigned short value, unsigned short index,
			void *data, unsigned short size, int timeout);
int usb_bulk_msg(struct usb_device *dev, unsigned int pipe,
			void *data, int len, int *actual_length, int timeout);
int usb_submit_int_msg(struct usb_device *dev, unsigned long pipe,
			void *buffer, int transfer_len, int interval);
int usb_disable_asynch(int disable);
int usb_maxpacket(struct usb_device *dev, unsigned long pipe);
int usb_get_configuration_no(struct usb_device *dev, int cfgno,
			unsigned char *buffer, int length);
int usb_get_configuration_len(struct usb_device *dev, int cfgno);
int usb_get_report(struct usb_device *dev, int ifnum, unsigned char type,
			unsigned char id, void *buf, int size);
int usb_get_class_descriptor(struct usb_device *dev, int ifnum,
			unsigned char type, unsigned char id, void *buf,
			int size);
int usb_clear_halt(struct usb_device *dev, int pipe);
int usb_string(struct usb_device *dev, int index, char *buf, size_t size);
int usb_set_interface(struct usb_device *dev, int interface, int alternate);
int usb_get_port_status(struct usb_device *dev, int port, void *data);

/* big endian -> little endian conversion */
/* some CPUs are already little endian e.g. the ARM920T */
#define __swap_16(x) \
	({ unsigned short x_ = (unsigned short)x; \
	 (unsigned short)( \
		((x_ & 0x00FFU) << 8) | ((x_ & 0xFF00U) >> 8)); \
	})
#define __swap_32(x) \
	({ unsigned long x_ = (unsigned long)x; \
	 (unsigned long)( \
		((x_ & 0x000000FFUL) << 24) | \
		((x_ & 0x0000FF00UL) <<	 8) | \
		((x_ & 0x00FF0000UL) >>	 8) | \
		((x_ & 0xFF000000UL) >> 24)); \
	})

#ifdef __LITTLE_ENDIAN
# define swap_16(x) (x)
# define swap_32(x) (x)
#else
# define swap_16(x) __swap_16(x)
# define swap_32(x) __swap_32(x)
#endif

/*
 * Calling this entity a "pipe" is glorifying it. A USB pipe
 * is something embarrassingly simple: it basically consists
 * of the following information:
 *  - device number (7 bits)
 *  - endpoint number (4 bits)
 *  - current Data0/1 state (1 bit)
 *  - direction (1 bit)
 *  - speed (2 bits)
 *  - max packet size (2 bits: 8, 16, 32 or 64)
 *  - pipe type (2 bits: control, interrupt, bulk, isochronous)
 *
 * That's 18 bits. Really. Nothing more. And the USB people have
 * documented these eighteen bits as some kind of glorious
 * virtual data structure.
 *
 * Let's not fall in that trap. We'll just encode it as a simple
 * unsigned int. The encoding is:
 *
 *  - max size:		bits 0-1	(00 = 8, 01 = 16, 10 = 32, 11 = 64)
 *  - direction:	bit 7		(0 = Host-to-Device [Out],
 *					(1 = Device-to-Host [In])
 *  - device:		bits 8-14
 *  - endpoint:		bits 15-18
 *  - Data0/1:		bit 19
 *  - pipe type:	bits 30-31	(00 = isochronous, 01 = interrupt,
 *					 10 = control, 11 = bulk)
 *
 * Why? Because it's arbitrary, and whatever encoding we select is really
 * up to us. This one happens to share a lot of bit positions with the UHCI
 * specification, so that much of the uhci driver can just mask the bits
 * appropriately.
 */
/* Create various pipes... */
#define create_pipe(dev,endpoint) \
		(((dev)->devnum << 8) | ((endpoint) << 15) | \
		(dev)->maxpacketsize)
#define default_pipe(dev) ((dev)->speed << 26)

#define usb_sndctrlpipe(dev, endpoint)	((PIPE_CONTROL << 30) | \
					 create_pipe(dev, endpoint))
#define usb_rcvctrlpipe(dev, endpoint)	((PIPE_CONTROL << 30) | \
					 create_pipe(dev, endpoint) | \
					 USB_DIR_IN)
#define usb_sndisocpipe(dev, endpoint)	((PIPE_ISOCHRONOUS << 30) | \
					 create_pipe(dev, endpoint))
#define usb_rcvisocpipe(dev, endpoint)	((PIPE_ISOCHRONOUS << 30) | \
					 create_pipe(dev, endpoint) | \
					 USB_DIR_IN)
#define usb_sndbulkpipe(dev, endpoint)	((PIPE_BULK << 30) | \
					 create_pipe(dev, endpoint))
#define usb_rcvbulkpipe(dev, endpoint)	((PIPE_BULK << 30) | \
					 create_pipe(dev, endpoint) | \
					 USB_DIR_IN)
#define usb_sndintpipe(dev, endpoint)	((PIPE_INTERRUPT << 30) | \
					 create_pipe(dev, endpoint))
#define usb_rcvintpipe(dev, endpoint)	((PIPE_INTERRUPT << 30) | \
					 create_pipe(dev, endpoint) | \
					 USB_DIR_IN)
#define usb_snddefctrl(dev)		((PIPE_CONTROL << 30) | \
					 default_pipe(dev))
#define usb_rcvdefctrl(dev)		((PIPE_CONTROL << 30) | \
					 default_pipe(dev) | \
					 USB_DIR_IN)

/* The D0/D1 toggle bits */
#define usb_gettoggle(dev, ep, out) (((dev)->toggle[out] >> ep) & 1)
#define usb_dotoggle(dev, ep, out)  ((dev)->toggle[out] ^= (1 << ep))
#define usb_settoggle(dev, ep, out, bit) ((dev)->toggle[out] = \
						((dev)->toggle[out] & \
						 ~(1 << ep)) | ((bit) << ep))

/* Endpoint halt control/status */
#define usb_endpoint_out(ep_dir)	(((ep_dir >> 7) & 1) ^ 1)
#define usb_endpoint_halt(dev, ep, out) ((dev)->halted[out] |= (1 << (ep)))
#define usb_endpoint_running(dev, ep, out) ((dev)->halted[out] &= ~(1 << (ep)))
#define usb_endpoint_halted(dev, ep, out) ((dev)->halted[out] & (1 << (ep)))

#define usb_packetid(pipe)	(((pipe) & USB_DIR_IN) ? USB_PID_IN : \
				 USB_PID_OUT)

#define usb_pipeout(pipe)	((((pipe) >> 7) & 1) ^ 1)
#define usb_pipein(pipe)	(((pipe) >> 7) & 1)
#define usb_pipedevice(pipe)	(((pipe) >> 8) & 0x7f)
#define usb_pipe_endpdev(pipe)	(((pipe) >> 8) & 0x7ff)
#define usb_pipeendpoint(pipe)	(((pipe) >> 15) & 0xf)
#define usb_pipedata(pipe)	(((pipe) >> 19) & 1)
#define usb_pipetype(pipe)	(((pipe) >> 30) & 3)
#define usb_pipeisoc(pipe)	(usb_pipetype((pipe)) == PIPE_ISOCHRONOUS)
#define usb_pipeint(pipe)	(usb_pipetype((pipe)) == PIPE_INTERRUPT)
#define usb_pipecontrol(pipe)	(usb_pipetype((pipe)) == PIPE_CONTROL)
#define usb_pipebulk(pipe)	(usb_pipetype((pipe)) == PIPE_BULK)

#define usb_pipe_ep_index(pipe)	\
		usb_pipecontrol(pipe) ? (usb_pipeendpoint(pipe) * 2) : \
				((usb_pipeendpoint(pipe) * 2) - \
				 (usb_pipein(pipe) ? 0 : 1))

/**
 * struct usb_device_id - identifies USB devices for probing and hotplugging
 * @match_flags: Bit mask controlling which of the other fields are used to
 *	match against new devices. Any field except for driver_info may be
 *	used, although some only make sense in conjunction with other fields.
 *	This is usually set by a USB_DEVICE_*() macro, which sets all
 *	other fields in this structure except for driver_info.
 * @idVendor: USB vendor ID for a device; numbers are assigned
 *	by the USB forum to its members.
 * @idProduct: Vendor-assigned product ID.
 * @bcdDevice_lo: Low end of range of vendor-assigned product version numbers.
 *	This is also used to identify individual product versions, for
 *	a range consisting of a single device.
 * @bcdDevice_hi: High end of version number range.  The range of product
 *	versions is inclusive.
 * @bDeviceClass: Class of device; numbers are assigned
 *	by the USB forum.  Products may choose to implement classes,
 *	or be vendor-specific.  Device classes specify behavior of all
 *	the interfaces on a device.
 * @bDeviceSubClass: Subclass of device; associated with bDeviceClass.
 * @bDeviceProtocol: Protocol of device; associated with bDeviceClass.
 * @bInterfaceClass: Class of interface; numbers are assigned
 *	by the USB forum.  Products may choose to implement classes,
 *	or be vendor-specific.  Interface classes specify behavior only
 *	of a given interface; other interfaces may support other classes.
 * @bInterfaceSubClass: Subclass of interface; associated with bInterfaceClass.
 * @bInterfaceProtocol: Protocol of interface; associated with bInterfaceClass.
 * @bInterfaceNumber: Number of interface; composite devices may use
 *	fixed interface numbers to differentiate between vendor-specific
 *	interfaces.
 * @driver_info: Holds information used by the driver.  Usually it holds
 *	a pointer to a descriptor understood by the driver, or perhaps
 *	device flags.
 *
 * In most cases, drivers will create a table of device IDs by using
 * USB_DEVICE(), or similar macros designed for that purpose.
 * They will then export it to userspace using MODULE_DEVICE_TABLE(),
 * and provide it to the USB core through their usb_driver structure.
 *
 * See the usb_match_id() function for information about how matches are
 * performed.  Briefly, you will normally use one of several macros to help
 * construct these entries.  Each entry you provide will either identify
 * one or more specific products, or will identify a class of products
 * which have agreed to behave the same.  You should put the more specific
 * matches towards the beginning of your table, so that driver_info can
 * record quirks of specific products.
 */
struct usb_device_id {
	/* which fields to match against? */
	u16 match_flags;

	/* Used for product specific matches; range is inclusive */
	u16 idVendor;
	u16 idProduct;
	u16 bcdDevice_lo;
	u16 bcdDevice_hi;

	/* Used for device class matches */
	u8 bDeviceClass;
	u8 bDeviceSubClass;
	u8 bDeviceProtocol;

	/* Used for interface class matches */
	u8 bInterfaceClass;
	u8 bInterfaceSubClass;
	u8 bInterfaceProtocol;

	/* Used for vendor-specific interface matches */
	u8 bInterfaceNumber;

	/* not matched against */
	ulong driver_info;
};

/* Some useful macros to use to create struct usb_device_id */
#define USB_DEVICE_ID_MATCH_VENDOR		0x0001
#define USB_DEVICE_ID_MATCH_PRODUCT		0x0002
#define USB_DEVICE_ID_MATCH_DEV_LO		0x0004
#define USB_DEVICE_ID_MATCH_DEV_HI		0x0008
#define USB_DEVICE_ID_MATCH_DEV_CLASS		0x0010
#define USB_DEVICE_ID_MATCH_DEV_SUBCLASS	0x0020
#define USB_DEVICE_ID_MATCH_DEV_PROTOCOL	0x0040
#define USB_DEVICE_ID_MATCH_INT_CLASS		0x0080
#define USB_DEVICE_ID_MATCH_INT_SUBCLASS	0x0100
#define USB_DEVICE_ID_MATCH_INT_PROTOCOL	0x0200
#define USB_DEVICE_ID_MATCH_INT_NUMBER		0x0400

/* Match anything, indicates this is a valid entry even if everything is 0 */
#define USB_DEVICE_ID_MATCH_NONE		0x0800
#define USB_DEVICE_ID_MATCH_ALL			0x07ff

/**
 * struct usb_driver_entry - Matches a driver to its usb_device_ids
 * @driver: Driver to use
 * @match: List of match records for this driver, terminated by {}
 */
struct usb_driver_entry {
	struct driver *driver;
	const struct usb_device_id *match;
};

#define USB_DEVICE_ID_MATCH_DEVICE \
		(USB_DEVICE_ID_MATCH_VENDOR | USB_DEVICE_ID_MATCH_PRODUCT)

/**
 * USB_DEVICE - macro used to describe a specific usb device
 * @vend: the 16 bit USB Vendor ID
 * @prod: the 16 bit USB Product ID
 *
 * This macro is used to create a struct usb_device_id that matches a
 * specific device.
 */
#define USB_DEVICE(vend, prod) \
	.match_flags = USB_DEVICE_ID_MATCH_DEVICE, \
	.idVendor = (vend), \
	.idProduct = (prod)

#define U_BOOT_USB_DEVICE(__name, __match) \
	ll_entry_declare(struct usb_driver_entry, __name, usb_driver_entry) = {\
		.driver = llsym(struct driver, __name, driver), \
		.match = __match, \
		}

/*************************************************************************
 * Hub Stuff
 */
struct usb_port_status {
	unsigned short wPortStatus;
	unsigned short wPortChange;
} __attribute__ ((packed));

struct usb_hub_status {
	unsigned short wHubStatus;
	unsigned short wHubChange;
} __attribute__ ((packed));

/*
 * Hub Device descriptor
 * USB Hub class device protocols
 */
#define USB_HUB_PR_FS		0 /* Full speed hub */
#define USB_HUB_PR_HS_NO_TT	0 /* Hi-speed hub without TT */
#define USB_HUB_PR_HS_SINGLE_TT	1 /* Hi-speed hub with single TT */
#define USB_HUB_PR_HS_MULTI_TT	2 /* Hi-speed hub with multiple TT */
#define USB_HUB_PR_SS		3 /* Super speed hub */

/* Transaction Translator Think Times, in bits */
#define HUB_TTTT_8_BITS		0x00
#define HUB_TTTT_16_BITS	0x20
#define HUB_TTTT_24_BITS	0x40
#define HUB_TTTT_32_BITS	0x60

/* Hub descriptor */
struct usb_hub_descriptor {
	unsigned char  bLength;
	unsigned char  bDescriptorType;
	unsigned char  bNbrPorts;
	unsigned short wHubCharacteristics;
	unsigned char  bPwrOn2PwrGood;
	unsigned char  bHubContrCurrent;
	/* 2.0 and 3.0 hubs differ here */
	union {
		struct {
			/* add 1 bit for hub status change; round to bytes */
			__u8 DeviceRemovable[(USB_MAXCHILDREN + 1 + 7) / 8];
			__u8 PortPowerCtrlMask[(USB_MAXCHILDREN + 1 + 7) / 8];
		} __attribute__ ((packed)) hs;

		struct {
			__u8 bHubHdrDecLat;
			__le16 wHubDelay;
			__le16 DeviceRemovable;
		} __attribute__ ((packed)) ss;
	} u;
} __attribute__ ((packed));


struct usb_hub_device {
	struct usb_device *pusb_dev;
	struct usb_hub_descriptor desc;

	ulong connect_timeout;		/* Device connection timeout in ms */
	ulong query_delay;		/* Device query delay in ms */
	int overcurrent_count[USB_MAXCHILDREN];	/* Over-current counter */
	int hub_depth;			/* USB 3.0 hub depth */
	struct usb_tt tt;		/* Transaction Translator */
};

#if CONFIG_IS_ENABLED(DM_USB)
/**
 * struct usb_platdata - Platform data about a USB controller
 *
 * Given a USB controller (UCLASS_USB) dev this is dev_get_platdata(dev)
 */
struct usb_platdata {
	enum usb_init_type init_type;
};

/**
 * struct usb_dev_platdata - Platform data about a USB device
 *
 * Given a USB device dev this structure is dev_get_parent_platdata(dev).
 * This is used by sandbox to provide emulation data also.
 *
 * @id:		ID used to match this device
 * @devnum:	Device address on the USB bus
 * @udev:	usb-uclass internal use only do NOT use
 * @strings:	List of descriptor strings (for sandbox emulation purposes)
 * @desc_list:	List of descriptors (for sandbox emulation purposes)
 */
struct usb_dev_platdata {
	struct usb_device_id id;
	int devnum;
	/*
	 * This pointer is used to pass the usb_device used in usb_scan_device,
	 * to get the usb descriptors before the driver is known, to the
	 * actual udevice once the driver is known and the udevice is created.
	 * This will be NULL except during probe, do NOT use.
	 *
	 * This should eventually go away.
	 */
	struct usb_device *udev;
#ifdef CONFIG_SANDBOX
	struct usb_string *strings;
	/* NULL-terminated list of descriptor pointers */
	struct usb_generic_descriptor **desc_list;
#endif
	int configno;
};

/**
 * struct usb_bus_priv - information about the USB controller
 *
 * Given a USB controller (UCLASS_USB) 'dev', this is
 * dev_get_uclass_priv(dev).
 *
 * @next_addr:	Next device address to allocate minus 1. Incremented by 1
 *		each time a new device address is set, so this holds the
 *		number of devices on the bus
 * @desc_before_addr:	true if we can read a device descriptor before it
 *		has been assigned an address. For XHCI this is not possible
 *		so this will be false.
 * @companion:  True if this is a companion controller to another USB
 *		controller
 */
struct usb_bus_priv {
	int next_addr;
	bool desc_before_addr;
	bool companion;
};

/**
 * struct usb_emul_platdata - platform data about the USB emulator
 *
 * Given a USB emulator (UCLASS_USB_EMUL) 'dev', this is
 * dev_get_uclass_platdata(dev).
 *
 * @port1:	USB emulator device port number on the parent hub
 */
struct usb_emul_platdata {
	int port1;	/* Port number (numbered from 1) */
};

/**
 * struct dm_usb_ops - USB controller operations
 *
 * This defines the operations supoorted on a USB controller. Common
 * arguments are:
 *
 * @bus:	USB bus (i.e. controller), which is in UCLASS_USB.
 * @udev:	USB device parent data. Controllers are not expected to need
 *		this, since the device address on the bus is encoded in @pipe.
 *		It is used for sandbox, and can be handy for debugging and
 *		logging.
 * @pipe:	An assortment of bitfields which provide address and packet
 *		type information. See create_pipe() above for encoding
 *		details
 * @buffer:	A buffer to use for sending/receiving. This should be
 *		DMA-aligned.
 * @length:	Buffer length in bytes
 */
struct dm_usb_ops {
	/**
	 * control() - Send a control message
	 *
	 * Most parameters are as above.
	 *
	 * @setup: Additional setup information required by the message
	 */
	int (*control)(struct udevice *bus, struct usb_device *udev,
		       unsigned long pipe, void *buffer, int length,
		       struct devrequest *setup);
	/**
	 * bulk() - Send a bulk message
	 *
	 * Parameters are as above.
	 */
	int (*bulk)(struct udevice *bus, struct usb_device *udev,
		    unsigned long pipe, void *buffer, int length);
	/**
	 * interrupt() - Send an interrupt message
	 *
	 * Most parameters are as above.
	 *
	 * @interval: Interrupt interval
	 */
	int (*interrupt)(struct udevice *bus, struct usb_device *udev,
			 unsigned long pipe, void *buffer, int length,
			 int interval);

	/**
	 * create_int_queue() - Create and queue interrupt packets
	 *
	 * Create and queue @queuesize number of interrupt usb packets of
	 * @elementsize bytes each. @buffer must be atleast @queuesize *
	 * @elementsize bytes.
	 *
	 * Note some controllers only support a queuesize of 1.
	 *
	 * @interval: Interrupt interval
	 *
	 * @return A pointer to the created interrupt queue or NULL on error
	 */
	struct int_queue * (*create_int_queue)(struct udevice *bus,
				struct usb_device *udev, unsigned long pipe,
				int queuesize, int elementsize, void *buffer,
				int interval);

	/**
	 * poll_int_queue() - Poll an interrupt queue for completed packets
	 *
	 * Poll an interrupt queue for completed packets. The return value
	 * points to the part of the buffer passed to create_int_queue()
	 * corresponding to the completed packet.
	 *
	 * @queue: queue to poll
	 *
	 * @return Pointer to the data of the first completed packet, or
	 *         NULL if no packets are ready
	 */
	void * (*poll_int_queue)(struct udevice *bus, struct usb_device *udev,
				 struct int_queue *queue);

	/**
	 * destroy_int_queue() - Destroy an interrupt queue
	 *
	 * Destroy an interrupt queue created by create_int_queue().
	 *
	 * @queue: queue to poll
	 *
	 * @return 0 if OK, -ve on error
	 */
	int (*destroy_int_queue)(struct udevice *bus, struct usb_device *udev,
				 struct int_queue *queue);

	/**
	 * alloc_device() - Allocate a new device context (XHCI)
	 *
	 * Before sending packets to a new device on an XHCI bus, a device
	 * context must be created. If this method is not NULL it will be
	 * called before the device is enumerated (even before its descriptor
	 * is read). This should be NULL for EHCI, which does not need this.
	 */
	int (*alloc_device)(struct udevice *bus, struct usb_device *udev);

	/**
	 * reset_root_port() - Reset usb root port
	 */
	int (*reset_root_port)(struct udevice *bus, struct usb_device *udev);

	/**
	 * update_hub_device() - Update HCD's internal representation of hub
	 *
	 * After a hub descriptor is fetched, notify HCD so that its internal
	 * representation of this hub can be updated (xHCI)
	 */
	int (*update_hub_device)(struct udevice *bus, struct usb_device *udev);

	/**
	 * get_max_xfer_size() - Get HCD's maximum transfer bytes
	 *
	 * The HCD may have limitation on the maximum bytes to be transferred
	 * in a USB transfer. USB class driver needs to be aware of this.
	 */
	int (*get_max_xfer_size)(struct udevice *bus, size_t *size);
};

#define usb_get_ops(dev)	((struct dm_usb_ops *)(dev)->driver->ops)
#define usb_get_emul_ops(dev)	((struct dm_usb_ops *)(dev)->driver->ops)

/**
 * usb_get_dev_index() - look up a device index number
 *
 * Look up devices using their index number (starting at 0). This works since
 * in U-Boot device addresses are allocated starting at 1 with no gaps.
 *
 * TODO(sjg@chromium.org): Remove this function when usb_ether.c is modified
 * to work better with driver model.
 *
 * @bus:	USB bus to check
 * @index:	Index number of device to find (0=first). This is just the
 *		device address less 1.
 */
struct usb_device *usb_get_dev_index(struct udevice *bus, int index);

/**
 * usb_setup_device() - set up a device ready for use
 *
 * @dev:	USB device pointer. This need not be a real device - it is
 *		common for it to just be a local variable with its ->dev
 *		member (i.e. @dev->dev) set to the parent device and
 *		dev->portnr set to the port number on the hub (1=first)
 * @do_read:	true to read the device descriptor before an address is set
 *		(should be false for XHCI buses, true otherwise)
 * @parent:	Parent device (either UCLASS_USB or UCLASS_USB_HUB)
 * @return 0 if OK, -ve on error */
int usb_setup_device(struct usb_device *dev, bool do_read,
		     struct usb_device *parent);

/**
 * usb_hub_is_root_hub() - Test whether a hub device is root hub or not
 *
 * @hub:	USB hub device to test
 * @return:	true if the hub device is root hub, false otherwise.
 */
bool usb_hub_is_root_hub(struct udevice *hub);

/**
 * usb_hub_scan() - Scan a hub and find its devices
 *
 * @hub:	Hub device to scan
 */
int usb_hub_scan(struct udevice *hub);

/**
 * usb_scan_device() - Scan a device on a bus
 *
 * Scan a device on a bus. It has already been detected and is ready to
 * be enumerated. This may be either the root hub (@parent is a bus) or a
 * normal device (@parent is a hub)
 *
 * @parent:	Parent device
 * @port:	Hub port number (numbered from 1)
 * @speed:	USB speed to use for this device
 * @devp:	Returns pointer to device if all is well
 * @return 0 if OK, -ve on error
 */
int usb_scan_device(struct udevice *parent, int port,
		    enum usb_device_speed speed, struct udevice **devp);

/**
 * usb_get_bus() - Find the bus for a device
 *
 * Search up through parents to find the bus this device is connected to. This
 * will be a device with uclass UCLASS_USB.
 *
 * @dev:	Device to check
 * @return The bus, or NULL if not found (this indicates a critical error in
 *	the USB stack
 */
struct udevice *usb_get_bus(struct udevice *dev);

/**
 * usb_select_config() - Set up a device ready for use
 *
 * This function assumes that the device already has an address and a driver
 * bound, and is ready to be set up.
 *
 * This re-reads the device and configuration descriptors and sets the
 * configuration
 *
 * @dev:	Device to set up
 */
int usb_select_config(struct usb_device *dev);

/**
 * usb_child_pre_probe() - Pre-probe function for USB devices
 *
 * This is called on all children of hubs and USB controllers (i.e. UCLASS_USB
 * and UCLASS_USB_HUB) when a new device is about to be probed. It sets up the
 * device from the saved platform data and calls usb_select_config() to
 * finish set up.
 *
 * Once this is done, the device's normal driver can take over, knowing the
 * device is accessible on the USB bus.
 *
 * This function is for use only by the internal USB stack.
 *
 * @dev:	Device to set up
 */
int usb_child_pre_probe(struct udevice *dev);

struct ehci_ctrl;

/**
 * usb_setup_ehci_gadget() - Set up a USB device as a gadget
 *
 * TODO(sjg@chromium.org): Tidy this up when USB gadgets can use driver model
 *
 * This provides a way to tell a controller to start up as a USB device
 * instead of as a host. It is untested.
 */
int usb_setup_ehci_gadget(struct ehci_ctrl **ctlrp);

/**
 * usb_stor_reset() - Prepare to scan USB storage devices
 *
 * Empty the list of USB storage devices in preparation for scanning them.
 * This must be called before a USB scan.
 */
void usb_stor_reset(void);

#else /* !CONFIG_IS_ENABLED(DM_USB) */

struct usb_device *usb_get_dev_index(int index);

#endif

bool usb_device_has_child_on_port(struct usb_device *parent, int port);

int usb_hub_probe(struct usb_device *dev, int ifnum);
void usb_hub_reset(void);

/*
 * usb_find_usb2_hub_address_port() - Get hub address and port for TT setting
 *
 * Searches for the first HS hub above the given device. If a
 * HS hub is found, the hub address and the port the device is
 * connected to is return, as required for SPLIT transactions
 *
 * @param: udev full speed or low speed device
 */
void usb_find_usb2_hub_address_port(struct usb_device *udev,
				    uint8_t *hub_address, uint8_t *hub_port);

/**
 * usb_alloc_new_device() - Allocate a new device
 *
 * @devp: returns a pointer of a new device structure. With driver model this
 *		is a device pointer, but with legacy USB this pointer is
 *		driver-specific.
 * @return 0 if OK, -ENOSPC if we have found out of room for new devices
 */
int usb_alloc_new_device(struct udevice *controller, struct usb_device **devp);

/**
 * usb_free_device() - Free a partially-inited device
 *
 * This is an internal function. It is used to reverse the action of
 * usb_alloc_new_device() when we hit a problem during init.
 */
void usb_free_device(struct udevice *controller);

int usb_new_device(struct usb_device *dev);

int usb_alloc_device(struct usb_device *dev);

/**
 * usb_update_hub_device() - Update HCD's internal representation of hub
 *
 * After a hub descriptor is fetched, notify HCD so that its internal
 * representation of this hub can be updated.
 *
 * @dev:		Hub device
 * @return 0 if OK, -ve on error
 */
int usb_update_hub_device(struct usb_device *dev);

/**
 * usb_get_max_xfer_size() - Get HCD's maximum transfer bytes
 *
 * The HCD may have limitation on the maximum bytes to be transferred
 * in a USB transfer. USB class driver needs to be aware of this.
 *
 * @dev:		USB device
 * @size:		maximum transfer bytes
 * @return 0 if OK, -ve on error
 */
int usb_get_max_xfer_size(struct usb_device *dev, size_t *size);

/**
 * usb_emul_setup_device() - Set up a new USB device emulation
 *
 * This is normally called when a new emulation device is bound. It tells
 * the USB emulation uclass about the features of the emulator.
 *
 * @dev:		Emulation device
 * @strings:		List of USB string descriptors, terminated by a NULL
 *			entry
 * @desc_list:		List of points or USB descriptors, terminated by NULL.
 *			The first entry must be struct usb_device_descriptor,
 *			and others follow on after that.
 * @return 0 if OK, -ENOSYS if not implemented, other -ve on error
 */
int usb_emul_setup_device(struct udevice *dev, struct usb_string *strings,
			  void **desc_list);

/**
 * usb_emul_control() - Send a control packet to an emulator
 *
 * @emul:	Emulator device
 * @udev:	USB device (which the emulator is causing to appear)
 * See struct dm_usb_ops for details on other parameters
 * @return 0 if OK, -ve on error
 */
int usb_emul_control(struct udevice *emul, struct usb_device *udev,
		     unsigned long pipe, void *buffer, int length,
		     struct devrequest *setup);

/**
 * usb_emul_bulk() - Send a bulk packet to an emulator
 *
 * @emul:	Emulator device
 * @udev:	USB device (which the emulator is causing to appear)
 * See struct dm_usb_ops for details on other parameters
 * @return 0 if OK, -ve on error
 */
int usb_emul_bulk(struct udevice *emul, struct usb_device *udev,
		  unsigned long pipe, void *buffer, int length);

/**
 * usb_emul_int() - Send an interrupt packet to an emulator
 *
 * @emul:	Emulator device
 * @udev:	USB device (which the emulator is causing to appear)
 * See struct dm_usb_ops for details on other parameters
 * @return 0 if OK, -ve on error
 */
int usb_emul_int(struct udevice *emul, struct usb_device *udev,
		  unsigned long pipe, void *buffer, int length, int interval);

/**
 * usb_emul_find() - Find an emulator for a particular device
 *
 * Check @pipe and @port1 to find a device number on bus @bus and return it.
 *
 * @bus:	USB bus (controller)
 * @pipe:	Describes pipe being used, and includes the device number
 * @port1:	Describes port number on the parent hub
 * @emulp:	Returns pointer to emulator, or NULL if not found
 * @return 0 if found, -ve on error
 */
int usb_emul_find(struct udevice *bus, ulong pipe, int port1,
		  struct udevice **emulp);

/**
 * usb_emul_find_for_dev() - Find an emulator for a particular device
 *
 * @dev:	USB device to check
 * @emulp:	Returns pointer to emulator, or NULL if not found
 * @return 0 if found, -ve on error
 */
int usb_emul_find_for_dev(struct udevice *dev, struct udevice **emulp);

/**
 * usb_emul_find_descriptor() - Find a USB descriptor of a particular device
 *
 * @ptr:	a pointer to a list of USB descriptor pointers
 * @type:	type of USB descriptor to find
 * @index:	if @type is USB_DT_CONFIG, this is the configuration value
 * @return a pointer to the USB descriptor found, NULL if not found
 */
struct usb_generic_descriptor **usb_emul_find_descriptor(
		struct usb_generic_descriptor **ptr, int type, int index);

/**
 * usb_show_tree() - show the USB device tree
 *
 * This shows a list of active USB devices along with basic information about
 * each.
 */
void usb_show_tree(void);

#endif /*_USB_H_ */
