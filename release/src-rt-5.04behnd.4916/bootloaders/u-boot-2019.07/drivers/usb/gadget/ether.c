// SPDX-License-Identifier: GPL-2.0+
/*
 * ether.c -- Ethernet gadget driver, with CDC and non-CDC options
 *
 * Copyright (C) 2003-2005,2008 David Brownell
 * Copyright (C) 2003-2004 Robert Schwebel, Benedikt Spranger
 * Copyright (C) 2008 Nokia Corporation
 */

#include <common.h>
#include <console.h>
#include <environment.h>
#include <linux/errno.h>
#include <linux/netdevice.h>
#include <linux/usb/ch9.h>
#include <linux/usb/cdc.h>
#include <linux/usb/gadget.h>
#include <net.h>
#include <usb.h>
#include <malloc.h>
#include <memalign.h>
#include <linux/ctype.h>

#include "gadget_chips.h"
#include "rndis.h"

#include <dm.h>
#include <dm/lists.h>
#include <dm/uclass-internal.h>
#include <dm/device-internal.h>

#define USB_NET_NAME "usb_ether"

#define atomic_read
extern struct platform_data brd;


unsigned packet_received, packet_sent;

/*
 * Ethernet gadget driver -- with CDC and non-CDC options
 * Builds on hardware support for a full duplex link.
 *
 * CDC Ethernet is the standard USB solution for sending Ethernet frames
 * using USB.  Real hardware tends to use the same framing protocol but look
 * different for control features.  This driver strongly prefers to use
 * this USB-IF standard as its open-systems interoperability solution;
 * most host side USB stacks (except from Microsoft) support it.
 *
 * This is sometimes called "CDC ECM" (Ethernet Control Model) to support
 * TLA-soup.  "CDC ACM" (Abstract Control Model) is for modems, and a new
 * "CDC EEM" (Ethernet Emulation Model) is starting to spread.
 *
 * There's some hardware that can't talk CDC ECM.  We make that hardware
 * implement a "minimalist" vendor-agnostic CDC core:  same framing, but
 * link-level setup only requires activating the configuration.  Only the
 * endpoint descriptors, and product/vendor IDs, are relevant; no control
 * operations are available.  Linux supports it, but other host operating
 * systems may not.  (This is a subset of CDC Ethernet.)
 *
 * It turns out that if you add a few descriptors to that "CDC Subset",
 * (Windows) host side drivers from MCCI can treat it as one submode of
 * a proprietary scheme called "SAFE" ... without needing to know about
 * specific product/vendor IDs.  So we do that, making it easier to use
 * those MS-Windows drivers.  Those added descriptors make it resemble a
 * CDC MDLM device, but they don't change device behavior at all.  (See
 * MCCI Engineering report 950198 "SAFE Networking Functions".)
 *
 * A third option is also in use.  Rather than CDC Ethernet, or something
 * simpler, Microsoft pushes their own approach: RNDIS.  The published
 * RNDIS specs are ambiguous and appear to be incomplete, and are also
 * needlessly complex.  They borrow more from CDC ACM than CDC ECM.
 */

#define DRIVER_DESC		"Ethernet Gadget"
/* Based on linux 2.6.27 version */
#define DRIVER_VERSION		"May Day 2005"

static const char driver_desc[] = DRIVER_DESC;

#define RX_EXTRA	20		/* guard against rx overflows */

#ifndef	CONFIG_USB_ETH_RNDIS
#define rndis_uninit(x)		do {} while (0)
#define rndis_deregister(c)	do {} while (0)
#define rndis_exit()		do {} while (0)
#endif

/* CDC and RNDIS support the same host-chosen outgoing packet filters. */
#define	DEFAULT_FILTER	(USB_CDC_PACKET_TYPE_BROADCAST \
			|USB_CDC_PACKET_TYPE_ALL_MULTICAST \
			|USB_CDC_PACKET_TYPE_PROMISCUOUS \
			|USB_CDC_PACKET_TYPE_DIRECTED)

#define USB_CONNECT_TIMEOUT (3 * CONFIG_SYS_HZ)

/*-------------------------------------------------------------------------*/

struct eth_dev {
	struct usb_gadget	*gadget;
	struct usb_request	*req;		/* for control responses */
	struct usb_request	*stat_req;	/* for cdc & rndis status */

	u8			config;
	struct usb_ep		*in_ep, *out_ep, *status_ep;
	const struct usb_endpoint_descriptor
				*in, *out, *status;

	struct usb_request	*tx_req, *rx_req;

#ifndef CONFIG_DM_ETH
	struct eth_device	*net;
#else
	struct udevice		*net;
#endif
	struct net_device_stats	stats;
	unsigned int		tx_qlen;

	unsigned		zlp:1;
	unsigned		cdc:1;
	unsigned		rndis:1;
	unsigned		suspended:1;
	unsigned		network_started:1;
	u16			cdc_filter;
	unsigned long		todo;
	int			mtu;
#define	WORK_RX_MEMORY		0
	int			rndis_config;
	u8			host_mac[ETH_ALEN];
};

/*
 * This version autoconfigures as much as possible at run-time.
 *
 * It also ASSUMES a self-powered device, without remote wakeup,
 * although remote wakeup support would make sense.
 */

/*-------------------------------------------------------------------------*/
struct ether_priv {
	struct eth_dev ethdev;
#ifndef CONFIG_DM_ETH
	struct eth_device netdev;
#else
	struct udevice *netdev;
#endif
	struct usb_gadget_driver eth_driver;
};

struct ether_priv eth_priv;
struct ether_priv *l_priv = &eth_priv;

/*-------------------------------------------------------------------------*/

/* "main" config is either CDC, or its simple subset */
static inline int is_cdc(struct eth_dev *dev)
{
#if	!defined(CONFIG_USB_ETH_SUBSET)
	return 1;		/* only cdc possible */
#elif	!defined(CONFIG_USB_ETH_CDC)
	return 0;		/* only subset possible */
#else
	return dev->cdc;	/* depends on what hardware we found */
#endif
}

/* "secondary" RNDIS config may sometimes be activated */
static inline int rndis_active(struct eth_dev *dev)
{
#ifdef	CONFIG_USB_ETH_RNDIS
	return dev->rndis;
#else
	return 0;
#endif
}

#define	subset_active(dev)	(!is_cdc(dev) && !rndis_active(dev))
#define	cdc_active(dev)		(is_cdc(dev) && !rndis_active(dev))

#define DEFAULT_QLEN	2	/* double buffering by default */

/* peak bulk transfer bits-per-second */
#define	HS_BPS		(13 * 512 * 8 * 1000 * 8)
#define	FS_BPS		(19 *  64 * 1 * 1000 * 8)

#ifdef CONFIG_USB_GADGET_DUALSPEED
#define	DEVSPEED	USB_SPEED_HIGH

#ifdef CONFIG_USB_ETH_QMULT
#define qmult CONFIG_USB_ETH_QMULT
#else
#define qmult 5
#endif

/* for dual-speed hardware, use deeper queues at highspeed */
#define qlen(gadget) \
	(DEFAULT_QLEN*((gadget->speed == USB_SPEED_HIGH) ? qmult : 1))

static inline int BITRATE(struct usb_gadget *g)
{
	return (g->speed == USB_SPEED_HIGH) ? HS_BPS : FS_BPS;
}

#else	/* full speed (low speed doesn't do bulk) */

#define qmult		1

#define	DEVSPEED	USB_SPEED_FULL

#define qlen(gadget) DEFAULT_QLEN

static inline int BITRATE(struct usb_gadget *g)
{
	return FS_BPS;
}
#endif

/*-------------------------------------------------------------------------*/

/*
 * DO NOT REUSE THESE IDs with a protocol-incompatible driver!!  Ever!!
 * Instead:  allocate your own, using normal USB-IF procedures.
 */

/*
 * Thanks to NetChip Technologies for donating this product ID.
 * It's for devices with only CDC Ethernet configurations.
 */
#define CDC_VENDOR_NUM		0x0525	/* NetChip */
#define CDC_PRODUCT_NUM		0xa4a1	/* Linux-USB Ethernet Gadget */

/*
 * For hardware that can't talk CDC, we use the same vendor ID that
 * ARM Linux has used for ethernet-over-usb, both with sa1100 and
 * with pxa250.  We're protocol-compatible, if the host-side drivers
 * use the endpoint descriptors.  bcdDevice (version) is nonzero, so
 * drivers that need to hard-wire endpoint numbers have a hook.
 *
 * The protocol is a minimal subset of CDC Ether, which works on any bulk
 * hardware that's not deeply broken ... even on hardware that can't talk
 * RNDIS (like SA-1100, with no interrupt endpoint, or anything that
 * doesn't handle control-OUT).
 */
#define	SIMPLE_VENDOR_NUM	0x049f	/* Compaq Computer Corp. */
#define	SIMPLE_PRODUCT_NUM	0x505a	/* Linux-USB "CDC Subset" Device */

/*
 * For hardware that can talk RNDIS and either of the above protocols,
 * use this ID ... the windows INF files will know it.  Unless it's
 * used with CDC Ethernet, Linux 2.4 hosts will need updates to choose
 * the non-RNDIS configuration.
 */
#define RNDIS_VENDOR_NUM	0x0525	/* NetChip */
#define RNDIS_PRODUCT_NUM	0xa4a2	/* Ethernet/RNDIS Gadget */

/*
 * Some systems will want different product identifers published in the
 * device descriptor, either numbers or strings or both.  These string
 * parameters are in UTF-8 (superset of ASCII's 7 bit characters).
 */

/*
 * Emulating them in eth_bind:
 * static ushort idVendor;
 * static ushort idProduct;
 */

#if defined(CONFIG_USB_GADGET_MANUFACTURER)
static char *iManufacturer = CONFIG_USB_GADGET_MANUFACTURER;
#else
static char *iManufacturer = "U-Boot";
#endif

/* These probably need to be configurable. */
static ushort bcdDevice;
static char *iProduct;
static char *iSerialNumber;

static char dev_addr[18];

static char host_addr[18];


/*-------------------------------------------------------------------------*/

/*
 * USB DRIVER HOOKUP (to the hardware driver, below us), mostly
 * ep0 implementation:  descriptors, config management, setup().
 * also optional class-specific notification interrupt transfer.
 */

/*
 * DESCRIPTORS ... most are static, but strings and (full) configuration
 * descriptors are built on demand.  For now we do either full CDC, or
 * our simple subset, with RNDIS as an optional second configuration.
 *
 * RNDIS includes some CDC ACM descriptors ... like CDC Ethernet.  But
 * the class descriptors match a modem (they're ignored; it's really just
 * Ethernet functionality), they don't need the NOP altsetting, and the
 * status transfer endpoint isn't optional.
 */

#define STRING_MANUFACTURER		1
#define STRING_PRODUCT			2
#define STRING_ETHADDR			3
#define STRING_DATA			4
#define STRING_CONTROL			5
#define STRING_RNDIS_CONTROL		6
#define STRING_CDC			7
#define STRING_SUBSET			8
#define STRING_RNDIS			9
#define STRING_SERIALNUMBER		10

/* holds our biggest descriptor (or RNDIS response) */
#define USB_BUFSIZ	256

/*
 * This device advertises one configuration, eth_config, unless RNDIS
 * is enabled (rndis_config) on hardware supporting at least two configs.
 *
 * NOTE:  Controllers like superh_udc should probably be able to use
 * an RNDIS-only configuration.
 *
 * FIXME define some higher-powered configurations to make it easier
 * to recharge batteries ...
 */

#define DEV_CONFIG_VALUE	1	/* cdc or subset */
#define DEV_RNDIS_CONFIG_VALUE	2	/* rndis; optional */

static struct usb_device_descriptor
device_desc = {
	.bLength =		sizeof device_desc,
	.bDescriptorType =	USB_DT_DEVICE,

	.bcdUSB =		__constant_cpu_to_le16(0x0200),

	.bDeviceClass =		USB_CLASS_COMM,
	.bDeviceSubClass =	0,
	.bDeviceProtocol =	0,

	.idVendor =		__constant_cpu_to_le16(CDC_VENDOR_NUM),
	.idProduct =		__constant_cpu_to_le16(CDC_PRODUCT_NUM),
	.iManufacturer =	STRING_MANUFACTURER,
	.iProduct =		STRING_PRODUCT,
	.bNumConfigurations =	1,
};

static struct usb_otg_descriptor
otg_descriptor = {
	.bLength =		sizeof otg_descriptor,
	.bDescriptorType =	USB_DT_OTG,

	.bmAttributes =		USB_OTG_SRP,
};

static struct usb_config_descriptor
eth_config = {
	.bLength =		sizeof eth_config,
	.bDescriptorType =	USB_DT_CONFIG,

	/* compute wTotalLength on the fly */
	.bNumInterfaces =	2,
	.bConfigurationValue =	DEV_CONFIG_VALUE,
	.iConfiguration =	STRING_CDC,
	.bmAttributes =		USB_CONFIG_ATT_ONE | USB_CONFIG_ATT_SELFPOWER,
	.bMaxPower =		1,
};

#ifdef	CONFIG_USB_ETH_RNDIS
static struct usb_config_descriptor
rndis_config = {
	.bLength =              sizeof rndis_config,
	.bDescriptorType =      USB_DT_CONFIG,

	/* compute wTotalLength on the fly */
	.bNumInterfaces =       2,
	.bConfigurationValue =  DEV_RNDIS_CONFIG_VALUE,
	.iConfiguration =       STRING_RNDIS,
	.bmAttributes =		USB_CONFIG_ATT_ONE | USB_CONFIG_ATT_SELFPOWER,
	.bMaxPower =            1,
};
#endif

/*
 * Compared to the simple CDC subset, the full CDC Ethernet model adds
 * three class descriptors, two interface descriptors, optional status
 * endpoint.  Both have a "data" interface and two bulk endpoints.
 * There are also differences in how control requests are handled.
 *
 * RNDIS shares a lot with CDC-Ethernet, since it's a variant of the
 * CDC-ACM (modem) spec.  Unfortunately MSFT's RNDIS driver is buggy; it
 * may hang or oops.  Since bugfixes (or accurate specs, letting Linux
 * work around those bugs) are unlikely to ever come from MSFT, you may
 * wish to avoid using RNDIS.
 *
 * MCCI offers an alternative to RNDIS if you need to connect to Windows
 * but have hardware that can't support CDC Ethernet.   We add descriptors
 * to present the CDC Subset as a (nonconformant) CDC MDLM variant called
 * "SAFE".  That borrows from both CDC Ethernet and CDC MDLM.  You can
 * get those drivers from MCCI, or bundled with various products.
 */

#ifdef	CONFIG_USB_ETH_CDC
static struct usb_interface_descriptor
control_intf = {
	.bLength =		sizeof control_intf,
	.bDescriptorType =	USB_DT_INTERFACE,

	.bInterfaceNumber =	0,
	/* status endpoint is optional; this may be patched later */
	.bNumEndpoints =	1,
	.bInterfaceClass =	USB_CLASS_COMM,
	.bInterfaceSubClass =	USB_CDC_SUBCLASS_ETHERNET,
	.bInterfaceProtocol =	USB_CDC_PROTO_NONE,
	.iInterface =		STRING_CONTROL,
};
#endif

#ifdef	CONFIG_USB_ETH_RNDIS
static const struct usb_interface_descriptor
rndis_control_intf = {
	.bLength =              sizeof rndis_control_intf,
	.bDescriptorType =      USB_DT_INTERFACE,

	.bInterfaceNumber =     0,
	.bNumEndpoints =        1,
	.bInterfaceClass =      USB_CLASS_COMM,
	.bInterfaceSubClass =   USB_CDC_SUBCLASS_ACM,
	.bInterfaceProtocol =   USB_CDC_ACM_PROTO_VENDOR,
	.iInterface =           STRING_RNDIS_CONTROL,
};
#endif

static const struct usb_cdc_header_desc header_desc = {
	.bLength =		sizeof header_desc,
	.bDescriptorType =	USB_DT_CS_INTERFACE,
	.bDescriptorSubType =	USB_CDC_HEADER_TYPE,

	.bcdCDC =		__constant_cpu_to_le16(0x0110),
};

#if defined(CONFIG_USB_ETH_CDC) || defined(CONFIG_USB_ETH_RNDIS)

static const struct usb_cdc_union_desc union_desc = {
	.bLength =		sizeof union_desc,
	.bDescriptorType =	USB_DT_CS_INTERFACE,
	.bDescriptorSubType =	USB_CDC_UNION_TYPE,

	.bMasterInterface0 =	0,	/* index of control interface */
	.bSlaveInterface0 =	1,	/* index of DATA interface */
};

#endif	/* CDC || RNDIS */

#ifdef	CONFIG_USB_ETH_RNDIS

static const struct usb_cdc_call_mgmt_descriptor call_mgmt_descriptor = {
	.bLength =		sizeof call_mgmt_descriptor,
	.bDescriptorType =	USB_DT_CS_INTERFACE,
	.bDescriptorSubType =	USB_CDC_CALL_MANAGEMENT_TYPE,

	.bmCapabilities =	0x00,
	.bDataInterface =	0x01,
};

static const struct usb_cdc_acm_descriptor acm_descriptor = {
	.bLength =		sizeof acm_descriptor,
	.bDescriptorType =	USB_DT_CS_INTERFACE,
	.bDescriptorSubType =	USB_CDC_ACM_TYPE,

	.bmCapabilities =	0x00,
};

#endif

#ifndef CONFIG_USB_ETH_CDC

/*
 * "SAFE" loosely follows CDC WMC MDLM, violating the spec in various
 * ways:  data endpoints live in the control interface, there's no data
 * interface, and it's not used to talk to a cell phone radio.
 */

static const struct usb_cdc_mdlm_desc mdlm_desc = {
	.bLength =		sizeof mdlm_desc,
	.bDescriptorType =	USB_DT_CS_INTERFACE,
	.bDescriptorSubType =	USB_CDC_MDLM_TYPE,

	.bcdVersion =		__constant_cpu_to_le16(0x0100),
	.bGUID = {
		0x5d, 0x34, 0xcf, 0x66, 0x11, 0x18, 0x11, 0xd6,
		0xa2, 0x1a, 0x00, 0x01, 0x02, 0xca, 0x9a, 0x7f,
	},
};

/*
 * since "usb_cdc_mdlm_detail_desc" is a variable length structure, we
 * can't really use its struct.  All we do here is say that we're using
 * the submode of "SAFE" which directly matches the CDC Subset.
 */
#ifdef CONFIG_USB_ETH_SUBSET
static const u8 mdlm_detail_desc[] = {
	6,
	USB_DT_CS_INTERFACE,
	USB_CDC_MDLM_DETAIL_TYPE,

	0,	/* "SAFE" */
	0,	/* network control capabilities (none) */
	0,	/* network data capabilities ("raw" encapsulation) */
};
#endif

#endif

static const struct usb_cdc_ether_desc ether_desc = {
	.bLength =		sizeof(ether_desc),
	.bDescriptorType =	USB_DT_CS_INTERFACE,
	.bDescriptorSubType =	USB_CDC_ETHERNET_TYPE,

	/* this descriptor actually adds value, surprise! */
	.iMACAddress =		STRING_ETHADDR,
	.bmEthernetStatistics = __constant_cpu_to_le32(0), /* no statistics */
	.wMaxSegmentSize =	__constant_cpu_to_le16(PKTSIZE_ALIGN),
	.wNumberMCFilters =	__constant_cpu_to_le16(0),
	.bNumberPowerFilters =	0,
};

#if defined(CONFIG_USB_ETH_CDC) || defined(CONFIG_USB_ETH_RNDIS)

/*
 * include the status endpoint if we can, even where it's optional.
 * use wMaxPacketSize big enough to fit CDC_NOTIFY_SPEED_CHANGE in one
 * packet, to simplify cancellation; and a big transfer interval, to
 * waste less bandwidth.
 *
 * some drivers (like Linux 2.4 cdc-ether!) "need" it to exist even
 * if they ignore the connect/disconnect notifications that real aether
 * can provide.  more advanced cdc configurations might want to support
 * encapsulated commands (vendor-specific, using control-OUT).
 *
 * RNDIS requires the status endpoint, since it uses that encapsulation
 * mechanism for its funky RPC scheme.
 */

#define LOG2_STATUS_INTERVAL_MSEC	5	/* 1 << 5 == 32 msec */
#define STATUS_BYTECOUNT		16	/* 8 byte header + data */

static struct usb_endpoint_descriptor
fs_status_desc = {
	.bLength =		USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =	USB_DT_ENDPOINT,

	.bEndpointAddress =	USB_DIR_IN,
	.bmAttributes =		USB_ENDPOINT_XFER_INT,
	.wMaxPacketSize =	__constant_cpu_to_le16(STATUS_BYTECOUNT),
	.bInterval =		1 << LOG2_STATUS_INTERVAL_MSEC,
};
#endif

#ifdef	CONFIG_USB_ETH_CDC

/* the default data interface has no endpoints ... */

static const struct usb_interface_descriptor
data_nop_intf = {
	.bLength =		sizeof data_nop_intf,
	.bDescriptorType =	USB_DT_INTERFACE,

	.bInterfaceNumber =	1,
	.bAlternateSetting =	0,
	.bNumEndpoints =	0,
	.bInterfaceClass =	USB_CLASS_CDC_DATA,
	.bInterfaceSubClass =	0,
	.bInterfaceProtocol =	0,
};

/* ... but the "real" data interface has two bulk endpoints */

static const struct usb_interface_descriptor
data_intf = {
	.bLength =		sizeof data_intf,
	.bDescriptorType =	USB_DT_INTERFACE,

	.bInterfaceNumber =	1,
	.bAlternateSetting =	1,
	.bNumEndpoints =	2,
	.bInterfaceClass =	USB_CLASS_CDC_DATA,
	.bInterfaceSubClass =	0,
	.bInterfaceProtocol =	0,
	.iInterface =		STRING_DATA,
};

#endif

#ifdef	CONFIG_USB_ETH_RNDIS

/* RNDIS doesn't activate by changing to the "real" altsetting */

static const struct usb_interface_descriptor
rndis_data_intf = {
	.bLength =		sizeof rndis_data_intf,
	.bDescriptorType =	USB_DT_INTERFACE,

	.bInterfaceNumber =	1,
	.bAlternateSetting =	0,
	.bNumEndpoints =	2,
	.bInterfaceClass =	USB_CLASS_CDC_DATA,
	.bInterfaceSubClass =	0,
	.bInterfaceProtocol =	0,
	.iInterface =		STRING_DATA,
};

#endif

#ifdef CONFIG_USB_ETH_SUBSET

/*
 * "Simple" CDC-subset option is a simple vendor-neutral model that most
 * full speed controllers can handle:  one interface, two bulk endpoints.
 *
 * To assist host side drivers, we fancy it up a bit, and add descriptors
 * so some host side drivers will understand it as a "SAFE" variant.
 */

static const struct usb_interface_descriptor
subset_data_intf = {
	.bLength =		sizeof subset_data_intf,
	.bDescriptorType =	USB_DT_INTERFACE,

	.bInterfaceNumber =	0,
	.bAlternateSetting =	0,
	.bNumEndpoints =	2,
	.bInterfaceClass =      USB_CLASS_COMM,
	.bInterfaceSubClass =	USB_CDC_SUBCLASS_MDLM,
	.bInterfaceProtocol =	0,
	.iInterface =		STRING_DATA,
};

#endif	/* SUBSET */

static struct usb_endpoint_descriptor
fs_source_desc = {
	.bLength =		USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =	USB_DT_ENDPOINT,

	.bEndpointAddress =	USB_DIR_IN,
	.bmAttributes =		USB_ENDPOINT_XFER_BULK,
	.wMaxPacketSize =	__constant_cpu_to_le16(64),
};

static struct usb_endpoint_descriptor
fs_sink_desc = {
	.bLength =		USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =	USB_DT_ENDPOINT,

	.bEndpointAddress =	USB_DIR_OUT,
	.bmAttributes =		USB_ENDPOINT_XFER_BULK,
	.wMaxPacketSize =	__constant_cpu_to_le16(64),
};

static const struct usb_descriptor_header *fs_eth_function[11] = {
	(struct usb_descriptor_header *) &otg_descriptor,
#ifdef CONFIG_USB_ETH_CDC
	/* "cdc" mode descriptors */
	(struct usb_descriptor_header *) &control_intf,
	(struct usb_descriptor_header *) &header_desc,
	(struct usb_descriptor_header *) &union_desc,
	(struct usb_descriptor_header *) &ether_desc,
	/* NOTE: status endpoint may need to be removed */
	(struct usb_descriptor_header *) &fs_status_desc,
	/* data interface, with altsetting */
	(struct usb_descriptor_header *) &data_nop_intf,
	(struct usb_descriptor_header *) &data_intf,
	(struct usb_descriptor_header *) &fs_source_desc,
	(struct usb_descriptor_header *) &fs_sink_desc,
	NULL,
#endif /* CONFIG_USB_ETH_CDC */
};

static inline void fs_subset_descriptors(void)
{
#ifdef CONFIG_USB_ETH_SUBSET
	/* behavior is "CDC Subset"; extra descriptors say "SAFE" */
	fs_eth_function[1] = (struct usb_descriptor_header *) &subset_data_intf;
	fs_eth_function[2] = (struct usb_descriptor_header *) &header_desc;
	fs_eth_function[3] = (struct usb_descriptor_header *) &mdlm_desc;
	fs_eth_function[4] = (struct usb_descriptor_header *) &mdlm_detail_desc;
	fs_eth_function[5] = (struct usb_descriptor_header *) &ether_desc;
	fs_eth_function[6] = (struct usb_descriptor_header *) &fs_source_desc;
	fs_eth_function[7] = (struct usb_descriptor_header *) &fs_sink_desc;
	fs_eth_function[8] = NULL;
#else
	fs_eth_function[1] = NULL;
#endif
}

#ifdef	CONFIG_USB_ETH_RNDIS
static const struct usb_descriptor_header *fs_rndis_function[] = {
	(struct usb_descriptor_header *) &otg_descriptor,
	/* control interface matches ACM, not Ethernet */
	(struct usb_descriptor_header *) &rndis_control_intf,
	(struct usb_descriptor_header *) &header_desc,
	(struct usb_descriptor_header *) &call_mgmt_descriptor,
	(struct usb_descriptor_header *) &acm_descriptor,
	(struct usb_descriptor_header *) &union_desc,
	(struct usb_descriptor_header *) &fs_status_desc,
	/* data interface has no altsetting */
	(struct usb_descriptor_header *) &rndis_data_intf,
	(struct usb_descriptor_header *) &fs_source_desc,
	(struct usb_descriptor_header *) &fs_sink_desc,
	NULL,
};
#endif

/*
 * usb 2.0 devices need to expose both high speed and full speed
 * descriptors, unless they only run at full speed.
 */

#if defined(CONFIG_USB_ETH_CDC) || defined(CONFIG_USB_ETH_RNDIS)
static struct usb_endpoint_descriptor
hs_status_desc = {
	.bLength =		USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =	USB_DT_ENDPOINT,

	.bmAttributes =		USB_ENDPOINT_XFER_INT,
	.wMaxPacketSize =	__constant_cpu_to_le16(STATUS_BYTECOUNT),
	.bInterval =		LOG2_STATUS_INTERVAL_MSEC + 4,
};
#endif /* CONFIG_USB_ETH_CDC */

static struct usb_endpoint_descriptor
hs_source_desc = {
	.bLength =		USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =	USB_DT_ENDPOINT,

	.bmAttributes =		USB_ENDPOINT_XFER_BULK,
	.wMaxPacketSize =	__constant_cpu_to_le16(512),
};

static struct usb_endpoint_descriptor
hs_sink_desc = {
	.bLength =		USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =	USB_DT_ENDPOINT,

	.bmAttributes =		USB_ENDPOINT_XFER_BULK,
	.wMaxPacketSize =	__constant_cpu_to_le16(512),
};

static struct usb_qualifier_descriptor
dev_qualifier = {
	.bLength =		sizeof dev_qualifier,
	.bDescriptorType =	USB_DT_DEVICE_QUALIFIER,

	.bcdUSB =		__constant_cpu_to_le16(0x0200),
	.bDeviceClass =		USB_CLASS_COMM,

	.bNumConfigurations =	1,
};

static const struct usb_descriptor_header *hs_eth_function[11] = {
	(struct usb_descriptor_header *) &otg_descriptor,
#ifdef CONFIG_USB_ETH_CDC
	/* "cdc" mode descriptors */
	(struct usb_descriptor_header *) &control_intf,
	(struct usb_descriptor_header *) &header_desc,
	(struct usb_descriptor_header *) &union_desc,
	(struct usb_descriptor_header *) &ether_desc,
	/* NOTE: status endpoint may need to be removed */
	(struct usb_descriptor_header *) &hs_status_desc,
	/* data interface, with altsetting */
	(struct usb_descriptor_header *) &data_nop_intf,
	(struct usb_descriptor_header *) &data_intf,
	(struct usb_descriptor_header *) &hs_source_desc,
	(struct usb_descriptor_header *) &hs_sink_desc,
	NULL,
#endif /* CONFIG_USB_ETH_CDC */
};

static inline void hs_subset_descriptors(void)
{
#ifdef CONFIG_USB_ETH_SUBSET
	/* behavior is "CDC Subset"; extra descriptors say "SAFE" */
	hs_eth_function[1] = (struct usb_descriptor_header *) &subset_data_intf;
	hs_eth_function[2] = (struct usb_descriptor_header *) &header_desc;
	hs_eth_function[3] = (struct usb_descriptor_header *) &mdlm_desc;
	hs_eth_function[4] = (struct usb_descriptor_header *) &mdlm_detail_desc;
	hs_eth_function[5] = (struct usb_descriptor_header *) &ether_desc;
	hs_eth_function[6] = (struct usb_descriptor_header *) &hs_source_desc;
	hs_eth_function[7] = (struct usb_descriptor_header *) &hs_sink_desc;
	hs_eth_function[8] = NULL;
#else
	hs_eth_function[1] = NULL;
#endif
}

#ifdef	CONFIG_USB_ETH_RNDIS
static const struct usb_descriptor_header *hs_rndis_function[] = {
	(struct usb_descriptor_header *) &otg_descriptor,
	/* control interface matches ACM, not Ethernet */
	(struct usb_descriptor_header *) &rndis_control_intf,
	(struct usb_descriptor_header *) &header_desc,
	(struct usb_descriptor_header *) &call_mgmt_descriptor,
	(struct usb_descriptor_header *) &acm_descriptor,
	(struct usb_descriptor_header *) &union_desc,
	(struct usb_descriptor_header *) &hs_status_desc,
	/* data interface has no altsetting */
	(struct usb_descriptor_header *) &rndis_data_intf,
	(struct usb_descriptor_header *) &hs_source_desc,
	(struct usb_descriptor_header *) &hs_sink_desc,
	NULL,
};
#endif


/* maxpacket and other transfer characteristics vary by speed. */
static inline struct usb_endpoint_descriptor *
ep_desc(struct usb_gadget *g, struct usb_endpoint_descriptor *hs,
		struct usb_endpoint_descriptor *fs)
{
	if (gadget_is_dualspeed(g) && g->speed == USB_SPEED_HIGH)
		return hs;
	return fs;
}

/*-------------------------------------------------------------------------*/

/* descriptors that are built on-demand */

static char manufacturer[50];
static char product_desc[40] = DRIVER_DESC;
static char serial_number[20];

/* address that the host will use ... usually assigned at random */
static char ethaddr[2 * ETH_ALEN + 1];

/* static strings, in UTF-8 */
static struct usb_string		strings[] = {
	{ STRING_MANUFACTURER,	manufacturer, },
	{ STRING_PRODUCT,	product_desc, },
	{ STRING_SERIALNUMBER,	serial_number, },
	{ STRING_DATA,		"Ethernet Data", },
	{ STRING_ETHADDR,	ethaddr, },
#ifdef	CONFIG_USB_ETH_CDC
	{ STRING_CDC,		"CDC Ethernet", },
	{ STRING_CONTROL,	"CDC Communications Control", },
#endif
#ifdef	CONFIG_USB_ETH_SUBSET
	{ STRING_SUBSET,	"CDC Ethernet Subset", },
#endif
#ifdef	CONFIG_USB_ETH_RNDIS
	{ STRING_RNDIS,		"RNDIS", },
	{ STRING_RNDIS_CONTROL,	"RNDIS Communications Control", },
#endif
	{  }		/* end of list */
};

static struct usb_gadget_strings	stringtab = {
	.language	= 0x0409,	/* en-us */
	.strings	= strings,
};

/*============================================================================*/
DEFINE_CACHE_ALIGN_BUFFER(u8, control_req, USB_BUFSIZ);

#if defined(CONFIG_USB_ETH_CDC) || defined(CONFIG_USB_ETH_RNDIS)
DEFINE_CACHE_ALIGN_BUFFER(u8, status_req, STATUS_BYTECOUNT);
#endif

/*============================================================================*/

/*
 * one config, two interfaces:  control, data.
 * complications: class descriptors, and an altsetting.
 */
static int
config_buf(struct usb_gadget *g, u8 *buf, u8 type, unsigned index, int is_otg)
{
	int					len;
	const struct usb_config_descriptor	*config;
	const struct usb_descriptor_header	**function;
	int					hs = 0;

	if (gadget_is_dualspeed(g)) {
		hs = (g->speed == USB_SPEED_HIGH);
		if (type == USB_DT_OTHER_SPEED_CONFIG)
			hs = !hs;
	}
#define which_fn(t)	(hs ? hs_ ## t ## _function : fs_ ## t ## _function)

	if (index >= device_desc.bNumConfigurations)
		return -EINVAL;

#ifdef	CONFIG_USB_ETH_RNDIS
	/*
	 * list the RNDIS config first, to make Microsoft's drivers
	 * happy. DOCSIS 1.0 needs this too.
	 */
	if (device_desc.bNumConfigurations == 2 && index == 0) {
		config = &rndis_config;
		function = which_fn(rndis);
	} else
#endif
	{
		config = &eth_config;
		function = which_fn(eth);
	}

	/* for now, don't advertise srp-only devices */
	if (!is_otg)
		function++;

	len = usb_gadget_config_buf(config, buf, USB_BUFSIZ, function);
	if (len < 0)
		return len;
	((struct usb_config_descriptor *) buf)->bDescriptorType = type;
	return len;
}

/*-------------------------------------------------------------------------*/

static void eth_start(struct eth_dev *dev, gfp_t gfp_flags);
static int alloc_requests(struct eth_dev *dev, unsigned n, gfp_t gfp_flags);

static int
set_ether_config(struct eth_dev *dev, gfp_t gfp_flags)
{
	int					result = 0;
	struct usb_gadget			*gadget = dev->gadget;

#if defined(CONFIG_USB_ETH_CDC) || defined(CONFIG_USB_ETH_RNDIS)
	/* status endpoint used for RNDIS and (optionally) CDC */
	if (!subset_active(dev) && dev->status_ep) {
		dev->status = ep_desc(gadget, &hs_status_desc,
						&fs_status_desc);
		dev->status_ep->driver_data = dev;

		result = usb_ep_enable(dev->status_ep, dev->status);
		if (result != 0) {
			debug("enable %s --> %d\n",
				dev->status_ep->name, result);
			goto done;
		}
	}
#endif

	dev->in = ep_desc(gadget, &hs_source_desc, &fs_source_desc);
	dev->in_ep->driver_data = dev;

	dev->out = ep_desc(gadget, &hs_sink_desc, &fs_sink_desc);
	dev->out_ep->driver_data = dev;

	/*
	 * With CDC,  the host isn't allowed to use these two data
	 * endpoints in the default altsetting for the interface.
	 * so we don't activate them yet.  Reset from SET_INTERFACE.
	 *
	 * Strictly speaking RNDIS should work the same: activation is
	 * a side effect of setting a packet filter.  Deactivation is
	 * from REMOTE_NDIS_HALT_MSG, reset from REMOTE_NDIS_RESET_MSG.
	 */
	if (!cdc_active(dev)) {
		result = usb_ep_enable(dev->in_ep, dev->in);
		if (result != 0) {
			debug("enable %s --> %d\n",
				dev->in_ep->name, result);
			goto done;
		}

		result = usb_ep_enable(dev->out_ep, dev->out);
		if (result != 0) {
			debug("enable %s --> %d\n",
				dev->out_ep->name, result);
			goto done;
		}
	}

done:
	if (result == 0)
		result = alloc_requests(dev, qlen(gadget), gfp_flags);

	/* on error, disable any endpoints  */
	if (result < 0) {
		if (!subset_active(dev) && dev->status_ep)
			(void) usb_ep_disable(dev->status_ep);
		dev->status = NULL;
		(void) usb_ep_disable(dev->in_ep);
		(void) usb_ep_disable(dev->out_ep);
		dev->in = NULL;
		dev->out = NULL;
	} else if (!cdc_active(dev)) {
		/*
		 * activate non-CDC configs right away
		 * this isn't strictly according to the RNDIS spec
		 */
		eth_start(dev, GFP_ATOMIC);
	}

	/* caller is responsible for cleanup on error */
	return result;
}

static void eth_reset_config(struct eth_dev *dev)
{
	if (dev->config == 0)
		return;

	debug("%s\n", __func__);

	rndis_uninit(dev->rndis_config);

	/*
	 * disable endpoints, forcing (synchronous) completion of
	 * pending i/o.  then free the requests.
	 */

	if (dev->in) {
		usb_ep_disable(dev->in_ep);
		if (dev->tx_req) {
			usb_ep_free_request(dev->in_ep, dev->tx_req);
			dev->tx_req = NULL;
		}
	}
	if (dev->out) {
		usb_ep_disable(dev->out_ep);
		if (dev->rx_req) {
			usb_ep_free_request(dev->out_ep, dev->rx_req);
			dev->rx_req = NULL;
		}
	}
	if (dev->status)
		usb_ep_disable(dev->status_ep);

	dev->rndis = 0;
	dev->cdc_filter = 0;
	dev->config = 0;
}

/*
 * change our operational config.  must agree with the code
 * that returns config descriptors, and altsetting code.
 */
static int eth_set_config(struct eth_dev *dev, unsigned number,
				gfp_t gfp_flags)
{
	int			result = 0;
	struct usb_gadget	*gadget = dev->gadget;

	if (gadget_is_sa1100(gadget)
			&& dev->config
			&& dev->tx_qlen != 0) {
		/* tx fifo is full, but we can't clear it...*/
		pr_err("can't change configurations");
		return -ESPIPE;
	}
	eth_reset_config(dev);

	switch (number) {
	case DEV_CONFIG_VALUE:
		result = set_ether_config(dev, gfp_flags);
		break;
#ifdef	CONFIG_USB_ETH_RNDIS
	case DEV_RNDIS_CONFIG_VALUE:
		dev->rndis = 1;
		result = set_ether_config(dev, gfp_flags);
		break;
#endif
	default:
		result = -EINVAL;
		/* FALL THROUGH */
	case 0:
		break;
	}

	if (result) {
		if (number)
			eth_reset_config(dev);
		usb_gadget_vbus_draw(dev->gadget,
				gadget_is_otg(dev->gadget) ? 8 : 100);
	} else {
		char *speed;
		unsigned power;

		power = 2 * eth_config.bMaxPower;
		usb_gadget_vbus_draw(dev->gadget, power);

		switch (gadget->speed) {
		case USB_SPEED_FULL:
			speed = "full"; break;
#ifdef CONFIG_USB_GADGET_DUALSPEED
		case USB_SPEED_HIGH:
			speed = "high"; break;
#endif
		default:
			speed = "?"; break;
		}

		dev->config = number;
		printf("%s speed config #%d: %d mA, %s, using %s\n",
				speed, number, power, driver_desc,
				rndis_active(dev)
					? "RNDIS"
					: (cdc_active(dev)
						? "CDC Ethernet"
						: "CDC Ethernet Subset"));
	}
	return result;
}

/*-------------------------------------------------------------------------*/

#ifdef	CONFIG_USB_ETH_CDC

/*
 * The interrupt endpoint is used in CDC networking models (Ethernet, ATM)
 * only to notify the host about link status changes (which we support) or
 * report completion of some encapsulated command (as used in RNDIS).  Since
 * we want this CDC Ethernet code to be vendor-neutral, we don't use that
 * command mechanism; and only one status request is ever queued.
 */
static void eth_status_complete(struct usb_ep *ep, struct usb_request *req)
{
	struct usb_cdc_notification	*event = req->buf;
	int				value = req->status;
	struct eth_dev			*dev = ep->driver_data;

	/* issue the second notification if host reads the first */
	if (event->bNotificationType == USB_CDC_NOTIFY_NETWORK_CONNECTION
			&& value == 0) {
		__le32	*data = req->buf + sizeof *event;

		event->bmRequestType = 0xA1;
		event->bNotificationType = USB_CDC_NOTIFY_SPEED_CHANGE;
		event->wValue = __constant_cpu_to_le16(0);
		event->wIndex = __constant_cpu_to_le16(1);
		event->wLength = __constant_cpu_to_le16(8);

		/* SPEED_CHANGE data is up/down speeds in bits/sec */
		data[0] = data[1] = cpu_to_le32(BITRATE(dev->gadget));

		req->length = STATUS_BYTECOUNT;
		value = usb_ep_queue(ep, req, GFP_ATOMIC);
		debug("send SPEED_CHANGE --> %d\n", value);
		if (value == 0)
			return;
	} else if (value != -ECONNRESET) {
		debug("event %02x --> %d\n",
			event->bNotificationType, value);
		if (event->bNotificationType ==
				USB_CDC_NOTIFY_SPEED_CHANGE) {
			dev->network_started = 1;
			printf("USB network up!\n");
		}
	}
	req->context = NULL;
}

static void issue_start_status(struct eth_dev *dev)
{
	struct usb_request		*req = dev->stat_req;
	struct usb_cdc_notification	*event;
	int				value;

	/*
	 * flush old status
	 *
	 * FIXME ugly idiom, maybe we'd be better with just
	 * a "cancel the whole queue" primitive since any
	 * unlink-one primitive has way too many error modes.
	 * here, we "know" toggle is already clear...
	 *
	 * FIXME iff req->context != null just dequeue it
	 */
	usb_ep_disable(dev->status_ep);
	usb_ep_enable(dev->status_ep, dev->status);

	/*
	 * 3.8.1 says to issue first NETWORK_CONNECTION, then
	 * a SPEED_CHANGE.  could be useful in some configs.
	 */
	event = req->buf;
	event->bmRequestType = 0xA1;
	event->bNotificationType = USB_CDC_NOTIFY_NETWORK_CONNECTION;
	event->wValue = __constant_cpu_to_le16(1);	/* connected */
	event->wIndex = __constant_cpu_to_le16(1);
	event->wLength = 0;

	req->length = sizeof *event;
	req->complete = eth_status_complete;
	req->context = dev;

	value = usb_ep_queue(dev->status_ep, req, GFP_ATOMIC);
	if (value < 0)
		debug("status buf queue --> %d\n", value);
}

#endif

/*-------------------------------------------------------------------------*/

static void eth_setup_complete(struct usb_ep *ep, struct usb_request *req)
{
	if (req->status || req->actual != req->length)
		debug("setup complete --> %d, %d/%d\n",
				req->status, req->actual, req->length);
}

#ifdef CONFIG_USB_ETH_RNDIS

static void rndis_response_complete(struct usb_ep *ep, struct usb_request *req)
{
	if (req->status || req->actual != req->length)
		debug("rndis response complete --> %d, %d/%d\n",
			req->status, req->actual, req->length);

	/* done sending after USB_CDC_GET_ENCAPSULATED_RESPONSE */
}

static void rndis_command_complete(struct usb_ep *ep, struct usb_request *req)
{
	struct eth_dev          *dev = ep->driver_data;
	int			status;

	/* received RNDIS command from USB_CDC_SEND_ENCAPSULATED_COMMAND */
	status = rndis_msg_parser(dev->rndis_config, (u8 *) req->buf);
	if (status < 0)
		pr_err("%s: rndis parse error %d", __func__, status);
}

#endif	/* RNDIS */

/*
 * The setup() callback implements all the ep0 functionality that's not
 * handled lower down.  CDC has a number of less-common features:
 *
 *  - two interfaces:  control, and ethernet data
 *  - Ethernet data interface has two altsettings:  default, and active
 *  - class-specific descriptors for the control interface
 *  - class-specific control requests
 */
static int
eth_setup(struct usb_gadget *gadget, const struct usb_ctrlrequest *ctrl)
{
	struct eth_dev		*dev = get_gadget_data(gadget);
	struct usb_request	*req = dev->req;
	int			value = -EOPNOTSUPP;
	u16			wIndex = le16_to_cpu(ctrl->wIndex);
	u16			wValue = le16_to_cpu(ctrl->wValue);
	u16			wLength = le16_to_cpu(ctrl->wLength);

	/*
	 * descriptors just go into the pre-allocated ep0 buffer,
	 * while config change events may enable network traffic.
	 */

	debug("%s\n", __func__);

	req->complete = eth_setup_complete;
	switch (ctrl->bRequest) {

	case USB_REQ_GET_DESCRIPTOR:
		if (ctrl->bRequestType != USB_DIR_IN)
			break;
		switch (wValue >> 8) {

		case USB_DT_DEVICE:
			device_desc.bMaxPacketSize0 = gadget->ep0->maxpacket;
			value = min(wLength, (u16) sizeof device_desc);
			memcpy(req->buf, &device_desc, value);
			break;
		case USB_DT_DEVICE_QUALIFIER:
			if (!gadget_is_dualspeed(gadget))
				break;
			value = min(wLength, (u16) sizeof dev_qualifier);
			memcpy(req->buf, &dev_qualifier, value);
			break;

		case USB_DT_OTHER_SPEED_CONFIG:
			if (!gadget_is_dualspeed(gadget))
				break;
			/* FALLTHROUGH */
		case USB_DT_CONFIG:
			value = config_buf(gadget, req->buf,
					wValue >> 8,
					wValue & 0xff,
					gadget_is_otg(gadget));
			if (value >= 0)
				value = min(wLength, (u16) value);
			break;

		case USB_DT_STRING:
			value = usb_gadget_get_string(&stringtab,
					wValue & 0xff, req->buf);

			if (value >= 0)
				value = min(wLength, (u16) value);

			break;
		}
		break;

	case USB_REQ_SET_CONFIGURATION:
		if (ctrl->bRequestType != 0)
			break;
		if (gadget->a_hnp_support)
			debug("HNP available\n");
		else if (gadget->a_alt_hnp_support)
			debug("HNP needs a different root port\n");
		value = eth_set_config(dev, wValue, GFP_ATOMIC);
		break;
	case USB_REQ_GET_CONFIGURATION:
		if (ctrl->bRequestType != USB_DIR_IN)
			break;
		*(u8 *)req->buf = dev->config;
		value = min(wLength, (u16) 1);
		break;

	case USB_REQ_SET_INTERFACE:
		if (ctrl->bRequestType != USB_RECIP_INTERFACE
				|| !dev->config
				|| wIndex > 1)
			break;
		if (!cdc_active(dev) && wIndex != 0)
			break;

		/*
		 * PXA hardware partially handles SET_INTERFACE;
		 * we need to kluge around that interference.
		 */
		if (gadget_is_pxa(gadget)) {
			value = eth_set_config(dev, DEV_CONFIG_VALUE,
						GFP_ATOMIC);
			/*
			 * PXA25x driver use non-CDC ethernet gadget.
			 * But only _CDC and _RNDIS code can signalize
			 * that network is working. So we signalize it
			 * here.
			 */
			dev->network_started = 1;
			debug("USB network up!\n");
			goto done_set_intf;
		}

#ifdef CONFIG_USB_ETH_CDC
		switch (wIndex) {
		case 0:		/* control/master intf */
			if (wValue != 0)
				break;
			if (dev->status) {
				usb_ep_disable(dev->status_ep);
				usb_ep_enable(dev->status_ep, dev->status);
			}

			value = 0;
			break;
		case 1:		/* data intf */
			if (wValue > 1)
				break;
			usb_ep_disable(dev->in_ep);
			usb_ep_disable(dev->out_ep);

			/*
			 * CDC requires the data transfers not be done from
			 * the default interface setting ... also, setting
			 * the non-default interface resets filters etc.
			 */
			if (wValue == 1) {
				if (!cdc_active(dev))
					break;
				usb_ep_enable(dev->in_ep, dev->in);
				usb_ep_enable(dev->out_ep, dev->out);
				dev->cdc_filter = DEFAULT_FILTER;
				if (dev->status)
					issue_start_status(dev);
				eth_start(dev, GFP_ATOMIC);
			}
			value = 0;
			break;
		}
#else
		/*
		 * FIXME this is wrong, as is the assumption that
		 * all non-PXA hardware talks real CDC ...
		 */
		debug("set_interface ignored!\n");
#endif /* CONFIG_USB_ETH_CDC */

done_set_intf:
		break;
	case USB_REQ_GET_INTERFACE:
		if (ctrl->bRequestType != (USB_DIR_IN|USB_RECIP_INTERFACE)
				|| !dev->config
				|| wIndex > 1)
			break;
		if (!(cdc_active(dev) || rndis_active(dev)) && wIndex != 0)
			break;

		/* for CDC, iff carrier is on, data interface is active. */
		if (rndis_active(dev) || wIndex != 1)
			*(u8 *)req->buf = 0;
		else {
			/* *(u8 *)req->buf = netif_carrier_ok (dev->net) ? 1 : 0; */
			/* carrier always ok ...*/
			*(u8 *)req->buf = 1 ;
		}
		value = min(wLength, (u16) 1);
		break;

#ifdef CONFIG_USB_ETH_CDC
	case USB_CDC_SET_ETHERNET_PACKET_FILTER:
		/*
		 * see 6.2.30: no data, wIndex = interface,
		 * wValue = packet filter bitmap
		 */
		if (ctrl->bRequestType != (USB_TYPE_CLASS|USB_RECIP_INTERFACE)
				|| !cdc_active(dev)
				|| wLength != 0
				|| wIndex > 1)
			break;
		debug("packet filter %02x\n", wValue);
		dev->cdc_filter = wValue;
		value = 0;
		break;

	/*
	 * and potentially:
	 * case USB_CDC_SET_ETHERNET_MULTICAST_FILTERS:
	 * case USB_CDC_SET_ETHERNET_PM_PATTERN_FILTER:
	 * case USB_CDC_GET_ETHERNET_PM_PATTERN_FILTER:
	 * case USB_CDC_GET_ETHERNET_STATISTIC:
	 */

#endif /* CONFIG_USB_ETH_CDC */

#ifdef CONFIG_USB_ETH_RNDIS
	/*
	 * RNDIS uses the CDC command encapsulation mechanism to implement
	 * an RPC scheme, with much getting/setting of attributes by OID.
	 */
	case USB_CDC_SEND_ENCAPSULATED_COMMAND:
		if (ctrl->bRequestType != (USB_TYPE_CLASS|USB_RECIP_INTERFACE)
				|| !rndis_active(dev)
				|| wLength > USB_BUFSIZ
				|| wValue
				|| rndis_control_intf.bInterfaceNumber
					!= wIndex)
			break;
		/* read the request, then process it */
		value = wLength;
		req->complete = rndis_command_complete;
		/* later, rndis_control_ack () sends a notification */
		break;

	case USB_CDC_GET_ENCAPSULATED_RESPONSE:
		if ((USB_DIR_IN|USB_TYPE_CLASS|USB_RECIP_INTERFACE)
					== ctrl->bRequestType
				&& rndis_active(dev)
				/* && wLength >= 0x0400 */
				&& !wValue
				&& rndis_control_intf.bInterfaceNumber
					== wIndex) {
			u8 *buf;
			u32 n;

			/* return the result */
			buf = rndis_get_next_response(dev->rndis_config, &n);
			if (buf) {
				memcpy(req->buf, buf, n);
				req->complete = rndis_response_complete;
				rndis_free_response(dev->rndis_config, buf);
				value = n;
			}
			/* else stalls ... spec says to avoid that */
		}
		break;
#endif	/* RNDIS */

	default:
		debug("unknown control req%02x.%02x v%04x i%04x l%d\n",
			ctrl->bRequestType, ctrl->bRequest,
			wValue, wIndex, wLength);
	}

	/* respond with data transfer before status phase? */
	if (value >= 0) {
		debug("respond with data transfer before status phase\n");
		req->length = value;
		req->zero = value < wLength
				&& (value % gadget->ep0->maxpacket) == 0;
		value = usb_ep_queue(gadget->ep0, req, GFP_ATOMIC);
		if (value < 0) {
			debug("ep_queue --> %d\n", value);
			req->status = 0;
			eth_setup_complete(gadget->ep0, req);
		}
	}

	/* host either stalls (value < 0) or reports success */
	return value;
}

/*-------------------------------------------------------------------------*/

static void rx_complete(struct usb_ep *ep, struct usb_request *req);

static int rx_submit(struct eth_dev *dev, struct usb_request *req,
				gfp_t gfp_flags)
{
	int			retval = -ENOMEM;
	size_t			size;

	/*
	 * Padding up to RX_EXTRA handles minor disagreements with host.
	 * Normally we use the USB "terminate on short read" convention;
	 * so allow up to (N*maxpacket), since that memory is normally
	 * already allocated.  Some hardware doesn't deal well with short
	 * reads (e.g. DMA must be N*maxpacket), so for now don't trim a
	 * byte off the end (to force hardware errors on overflow).
	 *
	 * RNDIS uses internal framing, and explicitly allows senders to
	 * pad to end-of-packet.  That's potentially nice for speed,
	 * but means receivers can't recover synch on their own.
	 */

	debug("%s\n", __func__);
	if (!req)
		return -EINVAL;

	size = (ETHER_HDR_SIZE + dev->mtu + RX_EXTRA);
	size += dev->out_ep->maxpacket - 1;
	if (rndis_active(dev))
		size += sizeof(struct rndis_packet_msg_type);
	size -= size % dev->out_ep->maxpacket;

	/*
	 * Some platforms perform better when IP packets are aligned,
	 * but on at least one, checksumming fails otherwise.  Note:
	 * RNDIS headers involve variable numbers of LE32 values.
	 */

	req->buf = (u8 *)net_rx_packets[0];
	req->length = size;
	req->complete = rx_complete;

	retval = usb_ep_queue(dev->out_ep, req, gfp_flags);

	if (retval)
		pr_err("rx submit --> %d", retval);

	return retval;
}

static void rx_complete(struct usb_ep *ep, struct usb_request *req)
{
	struct eth_dev	*dev = ep->driver_data;

	debug("%s: status %d\n", __func__, req->status);
	switch (req->status) {
	/* normal completion */
	case 0:
		if (rndis_active(dev)) {
			/* we know MaxPacketsPerTransfer == 1 here */
			int length = rndis_rm_hdr(req->buf, req->actual);
			if (length < 0)
				goto length_err;
			req->length -= length;
			req->actual -= length;
		}
		if (req->actual < ETH_HLEN || PKTSIZE_ALIGN < req->actual) {
length_err:
			dev->stats.rx_errors++;
			dev->stats.rx_length_errors++;
			debug("rx length %d\n", req->length);
			break;
		}

		dev->stats.rx_packets++;
		dev->stats.rx_bytes += req->length;
		break;

	/* software-driven interface shutdown */
	case -ECONNRESET:		/* unlink */
	case -ESHUTDOWN:		/* disconnect etc */
	/* for hardware automagic (such as pxa) */
	case -ECONNABORTED:		/* endpoint reset */
		break;

	/* data overrun */
	case -EOVERFLOW:
		dev->stats.rx_over_errors++;
		/* FALLTHROUGH */
	default:
		dev->stats.rx_errors++;
		break;
	}

	packet_received = 1;
}

static int alloc_requests(struct eth_dev *dev, unsigned n, gfp_t gfp_flags)
{

	dev->tx_req = usb_ep_alloc_request(dev->in_ep, 0);

	if (!dev->tx_req)
		goto fail1;

	dev->rx_req = usb_ep_alloc_request(dev->out_ep, 0);

	if (!dev->rx_req)
		goto fail2;

	return 0;

fail2:
	usb_ep_free_request(dev->in_ep, dev->tx_req);
fail1:
	pr_err("can't alloc requests");
	return -1;
}

static void tx_complete(struct usb_ep *ep, struct usb_request *req)
{
	struct eth_dev	*dev = ep->driver_data;

	debug("%s: status %s\n", __func__, (req->status) ? "failed" : "ok");
	switch (req->status) {
	default:
		dev->stats.tx_errors++;
		debug("tx err %d\n", req->status);
		/* FALLTHROUGH */
	case -ECONNRESET:		/* unlink */
	case -ESHUTDOWN:		/* disconnect etc */
		break;
	case 0:
		dev->stats.tx_bytes += req->length;
	}
	dev->stats.tx_packets++;

	packet_sent = 1;
}

static inline int eth_is_promisc(struct eth_dev *dev)
{
	/* no filters for the CDC subset; always promisc */
	if (subset_active(dev))
		return 1;
	return dev->cdc_filter & USB_CDC_PACKET_TYPE_PROMISCUOUS;
}

#if 0
static int eth_start_xmit (struct sk_buff *skb, struct net_device *net)
{
	struct eth_dev		*dev = netdev_priv(net);
	int			length = skb->len;
	int			retval;
	struct usb_request	*req = NULL;
	unsigned long		flags;

	/* apply outgoing CDC or RNDIS filters */
	if (!eth_is_promisc (dev)) {
		u8		*dest = skb->data;

		if (is_multicast_ethaddr(dest)) {
			u16	type;

			/* ignores USB_CDC_PACKET_TYPE_MULTICAST and host
			 * SET_ETHERNET_MULTICAST_FILTERS requests
			 */
			if (is_broadcast_ethaddr(dest))
				type = USB_CDC_PACKET_TYPE_BROADCAST;
			else
				type = USB_CDC_PACKET_TYPE_ALL_MULTICAST;
			if (!(dev->cdc_filter & type)) {
				dev_kfree_skb_any (skb);
				return 0;
			}
		}
		/* ignores USB_CDC_PACKET_TYPE_DIRECTED */
	}

	spin_lock_irqsave(&dev->req_lock, flags);
	/*
	 * this freelist can be empty if an interrupt triggered disconnect()
	 * and reconfigured the gadget (shutting down this queue) after the
	 * network stack decided to xmit but before we got the spinlock.
	 */
	if (list_empty(&dev->tx_reqs)) {
		spin_unlock_irqrestore(&dev->req_lock, flags);
		return 1;
	}

	req = container_of (dev->tx_reqs.next, struct usb_request, list);
	list_del (&req->list);

	/* temporarily stop TX queue when the freelist empties */
	if (list_empty (&dev->tx_reqs))
		netif_stop_queue (net);
	spin_unlock_irqrestore(&dev->req_lock, flags);

	/* no buffer copies needed, unless the network stack did it
	 * or the hardware can't use skb buffers.
	 * or there's not enough space for any RNDIS headers we need
	 */
	if (rndis_active(dev)) {
		struct sk_buff	*skb_rndis;

		skb_rndis = skb_realloc_headroom (skb,
				sizeof (struct rndis_packet_msg_type));
		if (!skb_rndis)
			goto drop;

		dev_kfree_skb_any (skb);
		skb = skb_rndis;
		rndis_add_hdr (skb);
		length = skb->len;
	}
	req->buf = skb->data;
	req->context = skb;
	req->complete = tx_complete;

	/* use zlp framing on tx for strict CDC-Ether conformance,
	 * though any robust network rx path ignores extra padding.
	 * and some hardware doesn't like to write zlps.
	 */
	req->zero = 1;
	if (!dev->zlp && (length % dev->in_ep->maxpacket) == 0)
		length++;

	req->length = length;

	/* throttle highspeed IRQ rate back slightly */
	if (gadget_is_dualspeed(dev->gadget))
		req->no_interrupt = (dev->gadget->speed == USB_SPEED_HIGH)
			? ((atomic_read(&dev->tx_qlen) % qmult) != 0)
			: 0;

	retval = usb_ep_queue (dev->in_ep, req, GFP_ATOMIC);
	switch (retval) {
	default:
		DEBUG (dev, "tx queue err %d\n", retval);
		break;
	case 0:
		net->trans_start = jiffies;
		atomic_inc (&dev->tx_qlen);
	}

	if (retval) {
drop:
		dev->stats.tx_dropped++;
		dev_kfree_skb_any (skb);
		spin_lock_irqsave(&dev->req_lock, flags);
		if (list_empty (&dev->tx_reqs))
			netif_start_queue (net);
		list_add (&req->list, &dev->tx_reqs);
		spin_unlock_irqrestore(&dev->req_lock, flags);
	}
	return 0;
}

/*-------------------------------------------------------------------------*/
#endif

static void eth_unbind(struct usb_gadget *gadget)
{
	struct eth_dev		*dev = get_gadget_data(gadget);

	debug("%s...\n", __func__);
	rndis_deregister(dev->rndis_config);
	rndis_exit();

	/* we've already been disconnected ... no i/o is active */
	if (dev->req) {
		usb_ep_free_request(gadget->ep0, dev->req);
		dev->req = NULL;
	}
	if (dev->stat_req) {
		usb_ep_free_request(dev->status_ep, dev->stat_req);
		dev->stat_req = NULL;
	}

	if (dev->tx_req) {
		usb_ep_free_request(dev->in_ep, dev->tx_req);
		dev->tx_req = NULL;
	}

	if (dev->rx_req) {
		usb_ep_free_request(dev->out_ep, dev->rx_req);
		dev->rx_req = NULL;
	}

/*	unregister_netdev (dev->net);*/
/*	free_netdev(dev->net);*/

	dev->gadget = NULL;
	set_gadget_data(gadget, NULL);
}

static void eth_disconnect(struct usb_gadget *gadget)
{
	eth_reset_config(get_gadget_data(gadget));
	/* FIXME RNDIS should enter RNDIS_UNINITIALIZED */
}

static void eth_suspend(struct usb_gadget *gadget)
{
	/* Not used */
}

static void eth_resume(struct usb_gadget *gadget)
{
	/* Not used */
}

/*-------------------------------------------------------------------------*/

#ifdef CONFIG_USB_ETH_RNDIS

/*
 * The interrupt endpoint is used in RNDIS to notify the host when messages
 * other than data packets are available ... notably the REMOTE_NDIS_*_CMPLT
 * messages, but also REMOTE_NDIS_INDICATE_STATUS_MSG and potentially even
 * REMOTE_NDIS_KEEPALIVE_MSG.
 *
 * The RNDIS control queue is processed by GET_ENCAPSULATED_RESPONSE, and
 * normally just one notification will be queued.
 */

static void rndis_control_ack_complete(struct usb_ep *ep,
					struct usb_request *req)
{
	struct eth_dev          *dev = ep->driver_data;

	debug("%s...\n", __func__);
	if (req->status || req->actual != req->length)
		debug("rndis control ack complete --> %d, %d/%d\n",
			req->status, req->actual, req->length);

	if (!dev->network_started) {
		if (rndis_get_state(dev->rndis_config)
				== RNDIS_DATA_INITIALIZED) {
			dev->network_started = 1;
			printf("USB RNDIS network up!\n");
		}
	}

	req->context = NULL;

	if (req != dev->stat_req)
		usb_ep_free_request(ep, req);
}

static char rndis_resp_buf[8] __attribute__((aligned(sizeof(__le32))));

#ifndef CONFIG_DM_ETH
static int rndis_control_ack(struct eth_device *net)
#else
static int rndis_control_ack(struct udevice *net)
#endif
{
	struct ether_priv	*priv = (struct ether_priv *)net->priv;
	struct eth_dev		*dev = &priv->ethdev;
	int                     length;
	struct usb_request      *resp = dev->stat_req;

	/* in case RNDIS calls this after disconnect */
	if (!dev->status) {
		debug("status ENODEV\n");
		return -ENODEV;
	}

	/* in case queue length > 1 */
	if (resp->context) {
		resp = usb_ep_alloc_request(dev->status_ep, GFP_ATOMIC);
		if (!resp)
			return -ENOMEM;
		resp->buf = rndis_resp_buf;
	}

	/*
	 * Send RNDIS RESPONSE_AVAILABLE notification;
	 * USB_CDC_NOTIFY_RESPONSE_AVAILABLE should work too
	 */
	resp->length = 8;
	resp->complete = rndis_control_ack_complete;
	resp->context = dev;

	*((__le32 *) resp->buf) = __constant_cpu_to_le32(1);
	*((__le32 *) (resp->buf + 4)) = __constant_cpu_to_le32(0);

	length = usb_ep_queue(dev->status_ep, resp, GFP_ATOMIC);
	if (length < 0) {
		resp->status = 0;
		rndis_control_ack_complete(dev->status_ep, resp);
	}

	return 0;
}

#else

#define	rndis_control_ack	NULL

#endif	/* RNDIS */

static void eth_start(struct eth_dev *dev, gfp_t gfp_flags)
{
	if (rndis_active(dev)) {
		rndis_set_param_medium(dev->rndis_config,
					NDIS_MEDIUM_802_3,
					BITRATE(dev->gadget)/100);
		rndis_signal_connect(dev->rndis_config);
	}
}

static int eth_stop(struct eth_dev *dev)
{
#ifdef RNDIS_COMPLETE_SIGNAL_DISCONNECT
	unsigned long ts;
	unsigned long timeout = CONFIG_SYS_HZ; /* 1 sec to stop RNDIS */
#endif

	if (rndis_active(dev)) {
		rndis_set_param_medium(dev->rndis_config, NDIS_MEDIUM_802_3, 0);
		rndis_signal_disconnect(dev->rndis_config);

#ifdef RNDIS_COMPLETE_SIGNAL_DISCONNECT
		/* Wait until host receives OID_GEN_MEDIA_CONNECT_STATUS */
		ts = get_timer(0);
		while (get_timer(ts) < timeout)
			usb_gadget_handle_interrupts(0);
#endif

		rndis_uninit(dev->rndis_config);
		dev->rndis = 0;
	}

	return 0;
}

/*-------------------------------------------------------------------------*/

static int is_eth_addr_valid(char *str)
{
	if (strlen(str) == 17) {
		int i;
		char *p, *q;
		uchar ea[6];

		/* see if it looks like an ethernet address */

		p = str;

		for (i = 0; i < 6; i++) {
			char term = (i == 5 ? '\0' : ':');

			ea[i] = simple_strtol(p, &q, 16);

			if ((q - p) != 2 || *q++ != term)
				break;

			p = q;
		}

		/* Now check the contents. */
		return is_valid_ethaddr(ea);
	}
	return 0;
}

static u8 nibble(unsigned char c)
{
	if (likely(isdigit(c)))
		return c - '0';
	c = toupper(c);
	if (likely(isxdigit(c)))
		return 10 + c - 'A';
	return 0;
}

static int get_ether_addr(const char *str, u8 *dev_addr)
{
	if (str) {
		unsigned	i;

		for (i = 0; i < 6; i++) {
			unsigned char num;

			if ((*str == '.') || (*str == ':'))
				str++;
			num = nibble(*str++) << 4;
			num |= (nibble(*str++));
			dev_addr[i] = num;
		}
		if (is_valid_ethaddr(dev_addr))
			return 0;
	}
	return 1;
}

static int eth_bind(struct usb_gadget *gadget)
{
	struct eth_dev		*dev = &l_priv->ethdev;
	u8			cdc = 1, zlp = 1, rndis = 1;
	struct usb_ep		*in_ep, *out_ep, *status_ep = NULL;
	int			status = -ENOMEM;
	int			gcnum;
	u8			tmp[7];
#ifdef CONFIG_DM_ETH
	struct eth_pdata	*pdata = dev_get_platdata(l_priv->netdev);
#endif

	/* these flags are only ever cleared; compiler take note */
#ifndef	CONFIG_USB_ETH_CDC
	cdc = 0;
#endif
#ifndef	CONFIG_USB_ETH_RNDIS
	rndis = 0;
#endif
	/*
	 * Because most host side USB stacks handle CDC Ethernet, that
	 * standard protocol is _strongly_ preferred for interop purposes.
	 * (By everyone except Microsoft.)
	 */
	if (gadget_is_pxa(gadget)) {
		/* pxa doesn't support altsettings */
		cdc = 0;
	} else if (gadget_is_musbhdrc(gadget)) {
		/* reduce tx dma overhead by avoiding special cases */
		zlp = 0;
	} else if (gadget_is_sh(gadget)) {
		/* sh doesn't support multiple interfaces or configs */
		cdc = 0;
		rndis = 0;
	} else if (gadget_is_sa1100(gadget)) {
		/* hardware can't write zlps */
		zlp = 0;
		/*
		 * sa1100 CAN do CDC, without status endpoint ... we use
		 * non-CDC to be compatible with ARM Linux-2.4 "usb-eth".
		 */
		cdc = 0;
	}

	gcnum = usb_gadget_controller_number(gadget);
	if (gcnum >= 0)
		device_desc.bcdDevice = cpu_to_le16(0x0300 + gcnum);
	else {
		/*
		 * can't assume CDC works.  don't want to default to
		 * anything less functional on CDC-capable hardware,
		 * so we fail in this case.
		 */
		pr_err("controller '%s' not recognized",
			gadget->name);
		return -ENODEV;
	}

	/*
	 * If there's an RNDIS configuration, that's what Windows wants to
	 * be using ... so use these product IDs here and in the "linux.inf"
	 * needed to install MSFT drivers.  Current Linux kernels will use
	 * the second configuration if it's CDC Ethernet, and need some help
	 * to choose the right configuration otherwise.
	 */
	if (rndis) {
#if defined(CONFIG_USB_GADGET_VENDOR_NUM) && defined(CONFIG_USB_GADGET_PRODUCT_NUM)
		device_desc.idVendor =
			__constant_cpu_to_le16(CONFIG_USB_GADGET_VENDOR_NUM);
		device_desc.idProduct =
			__constant_cpu_to_le16(CONFIG_USB_GADGET_PRODUCT_NUM);
#else
		device_desc.idVendor =
			__constant_cpu_to_le16(RNDIS_VENDOR_NUM);
		device_desc.idProduct =
			__constant_cpu_to_le16(RNDIS_PRODUCT_NUM);
#endif
		sprintf(product_desc, "RNDIS/%s", driver_desc);

	/*
	 * CDC subset ... recognized by Linux since 2.4.10, but Windows
	 * drivers aren't widely available.  (That may be improved by
	 * supporting one submode of the "SAFE" variant of MDLM.)
	 */
	} else {
#if defined(CONFIG_USB_GADGET_VENDOR_NUM) && defined(CONFIG_USB_GADGET_PRODUCT_NUM)
		device_desc.idVendor = cpu_to_le16(CONFIG_USB_GADGET_VENDOR_NUM);
		device_desc.idProduct = cpu_to_le16(CONFIG_USB_GADGET_PRODUCT_NUM);
#else
		if (!cdc) {
			device_desc.idVendor =
				__constant_cpu_to_le16(SIMPLE_VENDOR_NUM);
			device_desc.idProduct =
				__constant_cpu_to_le16(SIMPLE_PRODUCT_NUM);
		}
#endif
	}
	/* support optional vendor/distro customization */
	if (bcdDevice)
		device_desc.bcdDevice = cpu_to_le16(bcdDevice);
	if (iManufacturer)
		strlcpy(manufacturer, iManufacturer, sizeof manufacturer);
	if (iProduct)
		strlcpy(product_desc, iProduct, sizeof product_desc);
	if (iSerialNumber) {
		device_desc.iSerialNumber = STRING_SERIALNUMBER,
		strlcpy(serial_number, iSerialNumber, sizeof serial_number);
	}

	/* all we really need is bulk IN/OUT */
	usb_ep_autoconfig_reset(gadget);
	in_ep = usb_ep_autoconfig(gadget, &fs_source_desc);
	if (!in_ep) {
autoconf_fail:
		pr_err("can't autoconfigure on %s\n",
			gadget->name);
		return -ENODEV;
	}
	in_ep->driver_data = in_ep;	/* claim */

	out_ep = usb_ep_autoconfig(gadget, &fs_sink_desc);
	if (!out_ep)
		goto autoconf_fail;
	out_ep->driver_data = out_ep;	/* claim */

#if defined(CONFIG_USB_ETH_CDC) || defined(CONFIG_USB_ETH_RNDIS)
	/*
	 * CDC Ethernet control interface doesn't require a status endpoint.
	 * Since some hosts expect one, try to allocate one anyway.
	 */
	if (cdc || rndis) {
		status_ep = usb_ep_autoconfig(gadget, &fs_status_desc);
		if (status_ep) {
			status_ep->driver_data = status_ep;	/* claim */
		} else if (rndis) {
			pr_err("can't run RNDIS on %s", gadget->name);
			return -ENODEV;
#ifdef CONFIG_USB_ETH_CDC
		} else if (cdc) {
			control_intf.bNumEndpoints = 0;
			/* FIXME remove endpoint from descriptor list */
#endif
		}
	}
#endif

	/* one config:  cdc, else minimal subset */
	if (!cdc) {
		eth_config.bNumInterfaces = 1;
		eth_config.iConfiguration = STRING_SUBSET;

		/*
		 * use functions to set these up, in case we're built to work
		 * with multiple controllers and must override CDC Ethernet.
		 */
		fs_subset_descriptors();
		hs_subset_descriptors();
	}

	usb_gadget_set_selfpowered(gadget);

	/* For now RNDIS is always a second config */
	if (rndis)
		device_desc.bNumConfigurations = 2;

	if (gadget_is_dualspeed(gadget)) {
		if (rndis)
			dev_qualifier.bNumConfigurations = 2;
		else if (!cdc)
			dev_qualifier.bDeviceClass = USB_CLASS_VENDOR_SPEC;

		/* assumes ep0 uses the same value for both speeds ... */
		dev_qualifier.bMaxPacketSize0 = device_desc.bMaxPacketSize0;

		/* and that all endpoints are dual-speed */
		hs_source_desc.bEndpointAddress =
				fs_source_desc.bEndpointAddress;
		hs_sink_desc.bEndpointAddress =
				fs_sink_desc.bEndpointAddress;
#if defined(CONFIG_USB_ETH_CDC) || defined(CONFIG_USB_ETH_RNDIS)
		if (status_ep)
			hs_status_desc.bEndpointAddress =
					fs_status_desc.bEndpointAddress;
#endif
	}

	if (gadget_is_otg(gadget)) {
		otg_descriptor.bmAttributes |= USB_OTG_HNP,
		eth_config.bmAttributes |= USB_CONFIG_ATT_WAKEUP;
		eth_config.bMaxPower = 4;
#ifdef	CONFIG_USB_ETH_RNDIS
		rndis_config.bmAttributes |= USB_CONFIG_ATT_WAKEUP;
		rndis_config.bMaxPower = 4;
#endif
	}


	/* network device setup */
#ifndef CONFIG_DM_ETH
	dev->net = &l_priv->netdev;
#else
	dev->net = l_priv->netdev;
#endif

	dev->cdc = cdc;
	dev->zlp = zlp;

	dev->in_ep = in_ep;
	dev->out_ep = out_ep;
	dev->status_ep = status_ep;

	memset(tmp, 0, sizeof(tmp));
	/*
	 * Module params for these addresses should come from ID proms.
	 * The host side address is used with CDC and RNDIS, and commonly
	 * ends up in a persistent config database.  It's not clear if
	 * host side code for the SAFE thing cares -- its original BLAN
	 * thing didn't, Sharp never assigned those addresses on Zaurii.
	 */
#ifndef CONFIG_DM_ETH
	get_ether_addr(dev_addr, dev->net->enetaddr);
	memcpy(tmp, dev->net->enetaddr, sizeof(dev->net->enetaddr));
#else
	get_ether_addr(dev_addr, pdata->enetaddr);
	memcpy(tmp, pdata->enetaddr, sizeof(pdata->enetaddr));
#endif

	get_ether_addr(host_addr, dev->host_mac);

	sprintf(ethaddr, "%02X%02X%02X%02X%02X%02X",
		dev->host_mac[0], dev->host_mac[1],
			dev->host_mac[2], dev->host_mac[3],
			dev->host_mac[4], dev->host_mac[5]);

	if (rndis) {
		status = rndis_init();
		if (status < 0) {
			pr_err("can't init RNDIS, %d", status);
			goto fail;
		}
	}

	/*
	 * use PKTSIZE (or aligned... from u-boot) and set
	 * wMaxSegmentSize accordingly
	 */
	dev->mtu = PKTSIZE_ALIGN; /* RNDIS does not like this, only 1514, TODO*/

	/* preallocate control message data and buffer */
	dev->req = usb_ep_alloc_request(gadget->ep0, GFP_KERNEL);
	if (!dev->req)
		goto fail;
	dev->req->buf = control_req;
	dev->req->complete = eth_setup_complete;

	/* ... and maybe likewise for status transfer */
#if defined(CONFIG_USB_ETH_CDC) || defined(CONFIG_USB_ETH_RNDIS)
	if (dev->status_ep) {
		dev->stat_req = usb_ep_alloc_request(dev->status_ep,
							GFP_KERNEL);
		if (!dev->stat_req) {
			usb_ep_free_request(dev->status_ep, dev->req);

			goto fail;
		}
		dev->stat_req->buf = status_req;
		dev->stat_req->context = NULL;
	}
#endif

	/* finish hookup to lower layer ... */
	dev->gadget = gadget;
	set_gadget_data(gadget, dev);
	gadget->ep0->driver_data = dev;

	/*
	 * two kinds of host-initiated state changes:
	 *  - iff DATA transfer is active, carrier is "on"
	 *  - tx queueing enabled if open *and* carrier is "on"
	 */

	printf("using %s, OUT %s IN %s%s%s\n", gadget->name,
		out_ep->name, in_ep->name,
		status_ep ? " STATUS " : "",
		status_ep ? status_ep->name : ""
		);
#ifndef CONFIG_DM_ETH
	printf("MAC %pM\n", dev->net->enetaddr);
#else
	printf("MAC %pM\n", pdata->enetaddr);
#endif

	if (cdc || rndis)
		printf("HOST MAC %02x:%02x:%02x:%02x:%02x:%02x\n",
			dev->host_mac[0], dev->host_mac[1],
			dev->host_mac[2], dev->host_mac[3],
			dev->host_mac[4], dev->host_mac[5]);

	if (rndis) {
		u32	vendorID = 0;

		/* FIXME RNDIS vendor id == "vendor NIC code" == ? */

		dev->rndis_config = rndis_register(rndis_control_ack);
		if (dev->rndis_config < 0) {
fail0:
			eth_unbind(gadget);
			debug("RNDIS setup failed\n");
			status = -ENODEV;
			goto fail;
		}

		/* these set up a lot of the OIDs that RNDIS needs */
		rndis_set_host_mac(dev->rndis_config, dev->host_mac);
		if (rndis_set_param_dev(dev->rndis_config, dev->net, dev->mtu,
					&dev->stats, &dev->cdc_filter))
			goto fail0;
		if (rndis_set_param_vendor(dev->rndis_config, vendorID,
					manufacturer))
			goto fail0;
		if (rndis_set_param_medium(dev->rndis_config,
					NDIS_MEDIUM_802_3, 0))
			goto fail0;
		printf("RNDIS ready\n");
	}
	return 0;

fail:
	pr_err("%s failed, status = %d", __func__, status);
	eth_unbind(gadget);
	return status;
}

/*-------------------------------------------------------------------------*/
static void _usb_eth_halt(struct ether_priv *priv);

static int _usb_eth_init(struct ether_priv *priv)
{
	struct eth_dev *dev = &priv->ethdev;
	struct usb_gadget *gadget;
	unsigned long ts;
	int ret;
	unsigned long timeout = USB_CONNECT_TIMEOUT;

	ret = usb_gadget_initialize(0);
	if (ret)
		return ret;

	/* Configure default mac-addresses for the USB ethernet device */
#ifdef CONFIG_USBNET_DEV_ADDR
	strlcpy(dev_addr, CONFIG_USBNET_DEV_ADDR, sizeof(dev_addr));
#endif
#ifdef CONFIG_USBNET_HOST_ADDR
	strlcpy(host_addr, CONFIG_USBNET_HOST_ADDR, sizeof(host_addr));
#endif
	/* Check if the user overruled the MAC addresses */
	if (env_get("usbnet_devaddr"))
		strlcpy(dev_addr, env_get("usbnet_devaddr"),
			sizeof(dev_addr));

	if (env_get("usbnet_hostaddr"))
		strlcpy(host_addr, env_get("usbnet_hostaddr"),
			sizeof(host_addr));

	if (!is_eth_addr_valid(dev_addr)) {
		pr_err("Need valid 'usbnet_devaddr' to be set");
		goto fail;
	}
	if (!is_eth_addr_valid(host_addr)) {
		pr_err("Need valid 'usbnet_hostaddr' to be set");
		goto fail;
	}

	priv->eth_driver.speed		= DEVSPEED;
	priv->eth_driver.bind		= eth_bind;
	priv->eth_driver.unbind		= eth_unbind;
	priv->eth_driver.setup		= eth_setup;
	priv->eth_driver.reset		= eth_disconnect;
	priv->eth_driver.disconnect	= eth_disconnect;
	priv->eth_driver.suspend	= eth_suspend;
	priv->eth_driver.resume		= eth_resume;
	if (usb_gadget_register_driver(&priv->eth_driver) < 0)
		goto fail;

	dev->network_started = 0;

	packet_received = 0;
	packet_sent = 0;

	gadget = dev->gadget;
	usb_gadget_connect(gadget);

	if (env_get("cdc_connect_timeout"))
		timeout = simple_strtoul(env_get("cdc_connect_timeout"),
						NULL, 10) * CONFIG_SYS_HZ;
	ts = get_timer(0);
	while (!dev->network_started) {
		/* Handle control-c and timeouts */
		if (ctrlc() || (get_timer(ts) > timeout)) {
			pr_err("The remote end did not respond in time.");
			goto fail;
		}
		usb_gadget_handle_interrupts(0);
	}

	packet_received = 0;
	rx_submit(dev, dev->rx_req, 0);
	return 0;
fail:
	_usb_eth_halt(priv);
	return -1;
}

static int _usb_eth_send(struct ether_priv *priv, void *packet, int length)
{
	int			retval;
	void			*rndis_pkt = NULL;
	struct eth_dev		*dev = &priv->ethdev;
	struct usb_request	*req = dev->tx_req;
	unsigned long ts;
	unsigned long timeout = USB_CONNECT_TIMEOUT;

	debug("%s:...\n", __func__);

	/* new buffer is needed to include RNDIS header */
	if (rndis_active(dev)) {
		rndis_pkt = malloc(length +
					sizeof(struct rndis_packet_msg_type));
		if (!rndis_pkt) {
			pr_err("No memory to alloc RNDIS packet");
			goto drop;
		}
		rndis_add_hdr(rndis_pkt, length);
		memcpy(rndis_pkt + sizeof(struct rndis_packet_msg_type),
				packet, length);
		packet = rndis_pkt;
		length += sizeof(struct rndis_packet_msg_type);
	}
	req->buf = packet;
	req->context = NULL;
	req->complete = tx_complete;

	/*
	 * use zlp framing on tx for strict CDC-Ether conformance,
	 * though any robust network rx path ignores extra padding.
	 * and some hardware doesn't like to write zlps.
	 */
	req->zero = 1;
	if (!dev->zlp && (length % dev->in_ep->maxpacket) == 0)
		length++;

	req->length = length;
#if 0
	/* throttle highspeed IRQ rate back slightly */
	if (gadget_is_dualspeed(dev->gadget))
		req->no_interrupt = (dev->gadget->speed == USB_SPEED_HIGH)
			? ((dev->tx_qlen % qmult) != 0) : 0;
#endif
	dev->tx_qlen = 1;
	ts = get_timer(0);
	packet_sent = 0;

	retval = usb_ep_queue(dev->in_ep, req, GFP_ATOMIC);

	if (!retval)
		debug("%s: packet queued\n", __func__);
	while (!packet_sent) {
		if (get_timer(ts) > timeout) {
			printf("timeout sending packets to usb ethernet\n");
			return -1;
		}
		usb_gadget_handle_interrupts(0);
	}
	if (rndis_pkt)
		free(rndis_pkt);

	return 0;
drop:
	dev->stats.tx_dropped++;
	return -ENOMEM;
}

static int _usb_eth_recv(struct ether_priv *priv)
{
	usb_gadget_handle_interrupts(0);

	return 0;
}

static void _usb_eth_halt(struct ether_priv *priv)
{
	struct eth_dev *dev = &priv->ethdev;

	/* If the gadget not registered, simple return */
	if (!dev->gadget)
		return;

	/*
	 * Some USB controllers may need additional deinitialization here
	 * before dropping pull-up (also due to hardware issues).
	 * For example: unhandled interrupt with status stage started may
	 * bring the controller to fully broken state (until board reset).
	 * There are some variants to debug and fix such cases:
	 * 1) In the case of RNDIS connection eth_stop can perform additional
	 * interrupt handling. See RNDIS_COMPLETE_SIGNAL_DISCONNECT definition.
	 * 2) 'pullup' callback in your UDC driver can be improved to perform
	 * this deinitialization.
	 */
	eth_stop(dev);

	usb_gadget_disconnect(dev->gadget);

	/* Clear pending interrupt */
	if (dev->network_started) {
		usb_gadget_handle_interrupts(0);
		dev->network_started = 0;
	}

	usb_gadget_unregister_driver(&priv->eth_driver);
	usb_gadget_release(0);
}

#ifndef CONFIG_DM_ETH
static int usb_eth_init(struct eth_device *netdev, bd_t *bd)
{
	struct ether_priv *priv = (struct ether_priv *)netdev->priv;

	return _usb_eth_init(priv);
}

static int usb_eth_send(struct eth_device *netdev, void *packet, int length)
{
	struct ether_priv	*priv = (struct ether_priv *)netdev->priv;

	return _usb_eth_send(priv, packet, length);
}

static int usb_eth_recv(struct eth_device *netdev)
{
	struct ether_priv *priv = (struct ether_priv *)netdev->priv;
	struct eth_dev *dev = &priv->ethdev;
	int ret;

	ret = _usb_eth_recv(priv);
	if (ret) {
		pr_err("error packet receive\n");
		return ret;
	}

	if (!packet_received)
		return 0;

	if (dev->rx_req) {
		net_process_received_packet(net_rx_packets[0],
					    dev->rx_req->length);
	} else {
		pr_err("dev->rx_req invalid");
	}
	packet_received = 0;
	rx_submit(dev, dev->rx_req, 0);

	return 0;
}

void usb_eth_halt(struct eth_device *netdev)
{
	struct ether_priv *priv = (struct ether_priv *)netdev->priv;

	_usb_eth_halt(priv);
}

int usb_eth_initialize(bd_t *bi)
{
	struct eth_device *netdev = &l_priv->netdev;

	strlcpy(netdev->name, USB_NET_NAME, sizeof(netdev->name));

	netdev->init = usb_eth_init;
	netdev->send = usb_eth_send;
	netdev->recv = usb_eth_recv;
	netdev->halt = usb_eth_halt;
	netdev->priv = l_priv;

	eth_register(netdev);
	return 0;
}
#else
static int usb_eth_start(struct udevice *dev)
{
	struct ether_priv *priv = dev_get_priv(dev);

	return _usb_eth_init(priv);
}

static int usb_eth_send(struct udevice *dev, void *packet, int length)
{
	struct ether_priv *priv = dev_get_priv(dev);

	return _usb_eth_send(priv, packet, length);
}

static int usb_eth_recv(struct udevice *dev, int flags, uchar **packetp)
{
	struct ether_priv *priv = dev_get_priv(dev);
	struct eth_dev *ethdev = &priv->ethdev;
	int ret;

	ret = _usb_eth_recv(priv);
	if (ret) {
		pr_err("error packet receive\n");
		return ret;
	}

	if (packet_received) {
		if (ethdev->rx_req) {
			*packetp = (uchar *)net_rx_packets[0];
			return ethdev->rx_req->length;
		} else {
			pr_err("dev->rx_req invalid");
			return -EFAULT;
		}
	}

	return -EAGAIN;
}

static int usb_eth_free_pkt(struct udevice *dev, uchar *packet,
				   int length)
{
	struct ether_priv *priv = dev_get_priv(dev);
	struct eth_dev *ethdev = &priv->ethdev;

	packet_received = 0;

	return rx_submit(ethdev, ethdev->rx_req, 0);
}

static void usb_eth_stop(struct udevice *dev)
{
	struct ether_priv *priv = dev_get_priv(dev);

	_usb_eth_halt(priv);
}

static int usb_eth_probe(struct udevice *dev)
{
	struct ether_priv *priv = dev_get_priv(dev);
	struct eth_pdata *pdata = dev_get_platdata(dev);

	priv->netdev = dev;
	l_priv = priv;

	get_ether_addr(CONFIG_USBNET_DEVADDR, pdata->enetaddr);
	eth_env_set_enetaddr("usbnet_devaddr", pdata->enetaddr);

	return 0;
}

static const struct eth_ops usb_eth_ops = {
	.start		= usb_eth_start,
	.send		= usb_eth_send,
	.recv		= usb_eth_recv,
	.free_pkt	= usb_eth_free_pkt,
	.stop		= usb_eth_stop,
};

int usb_ether_init(void)
{
	struct udevice *dev;
	struct udevice *usb_dev;
	int ret;

	ret = uclass_first_device(UCLASS_USB_GADGET_GENERIC, &usb_dev);
	if (!usb_dev || ret) {
		pr_err("No USB device found\n");
		return ret;
	}

	ret = device_bind_driver(usb_dev, "usb_ether", "usb_ether", &dev);
	if (!dev || ret) {
		pr_err("usb - not able to bind usb_ether device\n");
		return ret;
	}

	return 0;
}

U_BOOT_DRIVER(eth_usb) = {
	.name	= "usb_ether",
	.id	= UCLASS_ETH,
	.probe	= usb_eth_probe,
	.ops	= &usb_eth_ops,
	.priv_auto_alloc_size = sizeof(struct ether_priv),
	.platdata_auto_alloc_size = sizeof(struct eth_pdata),
	.flags = DM_FLAG_ALLOC_PRIV_DMA,
};
#endif /* CONFIG_DM_ETH */
