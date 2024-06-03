#ifndef __USB_COMPAT_H__
#define __USB_COMPAT_H__

#include <dm.h>
#include "usb.h"

struct usb_hcd {
	void *hcd_priv;
};

struct usb_host_endpoint {
	struct usb_endpoint_descriptor		desc;
	struct list_head urb_list;
	void *hcpriv;
};

/*
 * urb->transfer_flags:
 *
 * Note: URB_DIR_IN/OUT is automatically set in usb_submit_urb().
 */
#define URB_SHORT_NOT_OK	0x0001	/* report short reads as errors */
#define URB_ZERO_PACKET		0x0040	/* Finish bulk OUT with short packet */

struct urb;

typedef void (*usb_complete_t)(struct urb *);

struct urb {
	void *hcpriv;			/* private data for host controller */
	struct list_head urb_list;	/* list head for use by the urb's
					 * current owner */
	struct usb_device *dev;		/* (in) pointer to associated device */
	struct usb_host_endpoint *ep;	/* (internal) pointer to endpoint */
	unsigned int pipe;		/* (in) pipe information */
	int status;			/* (return) non-ISO status */
	unsigned int transfer_flags;	/* (in) URB_SHORT_NOT_OK | ...*/
	void *transfer_buffer;		/* (in) associated data buffer */
	dma_addr_t transfer_dma;	/* (in) dma addr for transfer_buffer */
	u32 transfer_buffer_length;	/* (in) data buffer length */
	u32 actual_length;		/* (return) actual transfer length */
	unsigned char *setup_packet;	/* (in) setup packet (control only) */
	int start_frame;		/* (modify) start frame (ISO) */
	usb_complete_t complete;	/* (in) completion routine */
};

#define usb_hcd_link_urb_to_ep(hcd, urb)	({		\
	int ret = 0;						\
	list_add_tail(&urb->urb_list, &urb->ep->urb_list);	\
	ret; })
#define usb_hcd_unlink_urb_from_ep(hcd, urb)	list_del_init(&urb->urb_list)
#define usb_hcd_check_unlink_urb(hdc, urb, status)	0

static inline void usb_hcd_giveback_urb(struct usb_hcd *hcd,
					struct urb *urb,
					int status)
{
	urb->status = status;
	if (urb->complete)
		urb->complete(urb);
}

static inline int usb_hcd_unmap_urb_for_dma(struct usb_hcd *hcd,
					struct urb *urb)
{
	/* TODO: add cache invalidation here */
	return 0;
}

#if CONFIG_IS_ENABLED(DM_USB)
static inline struct usb_device *usb_dev_get_parent(struct usb_device *udev)
{
	struct udevice *parent = udev->dev->parent;

	/*
	 * When called from usb-uclass.c: usb_scan_device() udev->dev points
	 * to the parent udevice, not the actual udevice belonging to the
	 * udev as the device is not instantiated yet.
	 *
	 * If dev is an usb-bus, then we are called from usb_scan_device() for
	 * an usb-device plugged directly into the root port, return NULL.
	 */
	if (device_get_uclass_id(udev->dev) == UCLASS_USB)
		return NULL;

	/*
	 * If these 2 are not the same we are being called from
	 * usb_scan_device() and udev itself is the parent.
	 */
	if (dev_get_parent_priv(udev->dev) != udev)
		return udev;

	/* We are being called normally, use the parent pointer */
	if (device_get_uclass_id(parent) == UCLASS_USB_HUB)
		return dev_get_parent_priv(parent);

	return NULL;
}
#else
static inline struct usb_device *usb_dev_get_parent(struct usb_device *dev)
{
	return dev->parent;
}
#endif

#endif /* __USB_COMPAT_H__ */
