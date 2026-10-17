/* SPDX-License-Identifier: GPL-2.0+ */

#ifndef __USB_URB_COMPAT_H__
#define __USB_URB_COMPAT_H__

#include <linux/compat.h>
#include <usb.h>

struct udevice;
struct urb;
struct usb_hcd;

struct usb_urb_ops {
	int (*urb_enqueue)(struct usb_hcd *hcd, struct urb *urb,
			   gfp_t mem_flags);
	int (*urb_dequeue)(struct usb_hcd *hcd, struct urb *urb, int status);
	int (*hub_control)(struct usb_hcd *hcd, struct usb_device *dev,
			   unsigned long pipe, void *buffer, int len,
			   struct devrequest *setup);
	irqreturn_t (*isr)(int irq, void *priv);
};

struct usb_hcd {
	void *hcd_priv;
	const struct usb_urb_ops *urb_ops;
};

struct usb_host_endpoint {
	struct usb_endpoint_descriptor desc;
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

/**
 * usb_dev_get_parent() - Get the parent of a USB device
 *
 * @udev: USB struct containing information about the device
 * Return: associated device for which udev == dev_get_parent_priv(dev)
 */
struct usb_device *usb_dev_get_parent(struct usb_device *udev);

int usb_urb_submit_control(struct usb_hcd *hcd, struct urb *urb,
			   struct usb_host_endpoint *hep,
			   struct usb_device *dev, unsigned long pipe,
			   void *buffer, int len, struct devrequest *setup,
			   int interval, enum usb_device_speed speed);

int usb_urb_submit_bulk(struct usb_hcd *hcd, struct urb *urb,
			struct usb_host_endpoint *hep, struct usb_device *dev,
			unsigned long pipe, void *buffer, int len);

int usb_urb_submit_irq(struct usb_hcd *hcd, struct urb *urb,
		       struct usb_host_endpoint *hep, struct usb_device *dev,
		       unsigned long pipe, void *buffer, int len, int interval);

void usb_urb_fill(struct urb *urb, struct usb_host_endpoint *hep,
		  struct usb_device *dev, int endpoint_type,
		  unsigned long pipe, void *buffer, int len,
		  struct devrequest *setup, int interval);

int usb_urb_submit(struct usb_hcd *hcd, struct urb *urb);

#endif /* __USB_COMPAT_H__ */
