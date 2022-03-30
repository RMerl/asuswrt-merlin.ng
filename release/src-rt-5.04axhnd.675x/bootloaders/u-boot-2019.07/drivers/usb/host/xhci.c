// SPDX-License-Identifier: GPL-2.0+
/*
 * USB HOST XHCI Controller stack
 *
 * Based on xHCI host controller driver in linux-kernel
 * by Sarah Sharp.
 *
 * Copyright (C) 2008 Intel Corp.
 * Author: Sarah Sharp
 *
 * Copyright (C) 2013 Samsung Electronics Co.Ltd
 * Authors: Vivek Gautam <gautam.vivek@samsung.com>
 *	    Vikas Sajjan <vikas.sajjan@samsung.com>
 */

/**
 * This file gives the xhci stack for usb3.0 looking into
 * xhci specification Rev1.0 (5/21/10).
 * The quirk devices support hasn't been given yet.
 */

#include <common.h>
#include <dm.h>
#include <asm/byteorder.h>
#include <usb.h>
#include <malloc.h>
#include <watchdog.h>
#include <asm/cache.h>
#include <asm/unaligned.h>
#include <linux/errno.h>
#include "xhci.h"

#ifndef CONFIG_USB_MAX_CONTROLLER_COUNT
#define CONFIG_USB_MAX_CONTROLLER_COUNT 1
#endif

static struct descriptor {
	struct usb_hub_descriptor hub;
	struct usb_device_descriptor device;
	struct usb_config_descriptor config;
	struct usb_interface_descriptor interface;
	struct usb_endpoint_descriptor endpoint;
	struct usb_ss_ep_comp_descriptor ep_companion;
} __attribute__ ((packed)) descriptor = {
	{
		0xc,		/* bDescLength */
		0x2a,		/* bDescriptorType: hub descriptor */
		2,		/* bNrPorts -- runtime modified */
		cpu_to_le16(0x8), /* wHubCharacteristics */
		10,		/* bPwrOn2PwrGood */
		0,		/* bHubCntrCurrent */
		{		/* Device removable */
		}		/* at most 7 ports! XXX */
	},
	{
		0x12,		/* bLength */
		1,		/* bDescriptorType: UDESC_DEVICE */
		cpu_to_le16(0x0300), /* bcdUSB: v3.0 */
		9,		/* bDeviceClass: UDCLASS_HUB */
		0,		/* bDeviceSubClass: UDSUBCLASS_HUB */
		3,		/* bDeviceProtocol: UDPROTO_SSHUBSTT */
		9,		/* bMaxPacketSize: 512 bytes  2^9 */
		0x0000,		/* idVendor */
		0x0000,		/* idProduct */
		cpu_to_le16(0x0100), /* bcdDevice */
		1,		/* iManufacturer */
		2,		/* iProduct */
		0,		/* iSerialNumber */
		1		/* bNumConfigurations: 1 */
	},
	{
		0x9,
		2,		/* bDescriptorType: UDESC_CONFIG */
		cpu_to_le16(0x1f), /* includes SS endpoint descriptor */
		1,		/* bNumInterface */
		1,		/* bConfigurationValue */
		0,		/* iConfiguration */
		0x40,		/* bmAttributes: UC_SELF_POWER */
		0		/* bMaxPower */
	},
	{
		0x9,		/* bLength */
		4,		/* bDescriptorType: UDESC_INTERFACE */
		0,		/* bInterfaceNumber */
		0,		/* bAlternateSetting */
		1,		/* bNumEndpoints */
		9,		/* bInterfaceClass: UICLASS_HUB */
		0,		/* bInterfaceSubClass: UISUBCLASS_HUB */
		0,		/* bInterfaceProtocol: UIPROTO_HSHUBSTT */
		0		/* iInterface */
	},
	{
		0x7,		/* bLength */
		5,		/* bDescriptorType: UDESC_ENDPOINT */
		0x81,		/* bEndpointAddress: IN endpoint 1 */
		3,		/* bmAttributes: UE_INTERRUPT */
		8,		/* wMaxPacketSize */
		255		/* bInterval */
	},
	{
		0x06,		/* ss_bLength */
		0x30,		/* ss_bDescriptorType: SS EP Companion */
		0x00,		/* ss_bMaxBurst: allows 1 TX between ACKs */
		/* ss_bmAttributes: 1 packet per service interval */
		0x00,
		/* ss_wBytesPerInterval: 15 bits for max 15 ports */
		cpu_to_le16(0x02),
	},
};

#if !CONFIG_IS_ENABLED(DM_USB)
static struct xhci_ctrl xhcic[CONFIG_USB_MAX_CONTROLLER_COUNT];
#endif

struct xhci_ctrl *xhci_get_ctrl(struct usb_device *udev)
{
#if CONFIG_IS_ENABLED(DM_USB)
	struct udevice *dev;

	/* Find the USB controller */
	for (dev = udev->dev;
	     device_get_uclass_id(dev) != UCLASS_USB;
	     dev = dev->parent)
		;
	return dev_get_priv(dev);
#else
	return udev->controller;
#endif
}

/**
 * Waits for as per specified amount of time
 * for the "result" to match with "done"
 *
 * @param ptr	pointer to the register to be read
 * @param mask	mask for the value read
 * @param done	value to be campared with result
 * @param usec	time to wait till
 * @return 0 if handshake is success else < 0 on failure
 */
static int handshake(uint32_t volatile *ptr, uint32_t mask,
					uint32_t done, int usec)
{
	uint32_t result;

	do {
		result = xhci_readl(ptr);
		if (result == ~(uint32_t)0)
			return -ENODEV;
		result &= mask;
		if (result == done)
			return 0;
		usec--;
		udelay(1);
	} while (usec > 0);

	return -ETIMEDOUT;
}

/**
 * Set the run bit and wait for the host to be running.
 *
 * @param hcor	pointer to host controller operation registers
 * @return status of the Handshake
 */
static int xhci_start(struct xhci_hcor *hcor)
{
	u32 temp;
	int ret;

	puts("Starting the controller\n");
	temp = xhci_readl(&hcor->or_usbcmd);
	temp |= (CMD_RUN);
	xhci_writel(&hcor->or_usbcmd, temp);

	/*
	 * Wait for the HCHalted Status bit to be 0 to indicate the host is
	 * running.
	 */
	ret = handshake(&hcor->or_usbsts, STS_HALT, 0, XHCI_MAX_HALT_USEC);
	if (ret)
		debug("Host took too long to start, "
				"waited %u microseconds.\n",
				XHCI_MAX_HALT_USEC);
	return ret;
}

/**
 * Resets the XHCI Controller
 *
 * @param hcor	pointer to host controller operation registers
 * @return -EBUSY if XHCI Controller is not halted else status of handshake
 */
static int xhci_reset(struct xhci_hcor *hcor)
{
	u32 cmd;
	u32 state;
	int ret;

	/* Halting the Host first */
	debug("// Halt the HC: %p\n", hcor);
	state = xhci_readl(&hcor->or_usbsts) & STS_HALT;
	if (!state) {
		cmd = xhci_readl(&hcor->or_usbcmd);
		cmd &= ~CMD_RUN;
		xhci_writel(&hcor->or_usbcmd, cmd);
	}

	ret = handshake(&hcor->or_usbsts,
			STS_HALT, STS_HALT, XHCI_MAX_HALT_USEC);
	if (ret) {
		printf("Host not halted after %u microseconds.\n",
				XHCI_MAX_HALT_USEC);
		return -EBUSY;
	}

	debug("// Reset the HC\n");
	cmd = xhci_readl(&hcor->or_usbcmd);
	cmd |= CMD_RESET;
	xhci_writel(&hcor->or_usbcmd, cmd);

	ret = handshake(&hcor->or_usbcmd, CMD_RESET, 0, XHCI_MAX_RESET_USEC);
	if (ret)
		return ret;

	/*
	 * xHCI cannot write to any doorbells or operational registers other
	 * than status until the "Controller Not Ready" flag is cleared.
	 */
	return handshake(&hcor->or_usbsts, STS_CNR, 0, XHCI_MAX_RESET_USEC);
}

/**
 * Used for passing endpoint bitmasks between the core and HCDs.
 * Find the index for an endpoint given its descriptor.
 * Use the return value to right shift 1 for the bitmask.
 *
 * Index  = (epnum * 2) + direction - 1,
 * where direction = 0 for OUT, 1 for IN.
 * For control endpoints, the IN index is used (OUT index is unused), so
 * index = (epnum * 2) + direction - 1 = (epnum * 2) + 1 - 1 = (epnum * 2)
 *
 * @param desc	USB enpdoint Descriptor
 * @return index of the Endpoint
 */
static unsigned int xhci_get_ep_index(struct usb_endpoint_descriptor *desc)
{
	unsigned int index;

	if (usb_endpoint_xfer_control(desc))
		index = (unsigned int)(usb_endpoint_num(desc) * 2);
	else
		index = (unsigned int)((usb_endpoint_num(desc) * 2) -
				(usb_endpoint_dir_in(desc) ? 0 : 1));

	return index;
}

/*
 * Convert bInterval expressed in microframes (in 1-255 range) to exponent of
 * microframes, rounded down to nearest power of 2.
 */
static unsigned int xhci_microframes_to_exponent(unsigned int desc_interval,
						 unsigned int min_exponent,
						 unsigned int max_exponent)
{
	unsigned int interval;

	interval = fls(desc_interval) - 1;
	interval = clamp_val(interval, min_exponent, max_exponent);
	if ((1 << interval) != desc_interval)
		debug("rounding interval to %d microframes, "\
		      "ep desc says %d microframes\n",
		      1 << interval, desc_interval);

	return interval;
}

static unsigned int xhci_parse_microframe_interval(struct usb_device *udev,
	struct usb_endpoint_descriptor *endpt_desc)
{
	if (endpt_desc->bInterval == 0)
		return 0;

	return xhci_microframes_to_exponent(endpt_desc->bInterval, 0, 15);
}

static unsigned int xhci_parse_frame_interval(struct usb_device *udev,
	struct usb_endpoint_descriptor *endpt_desc)
{
	return xhci_microframes_to_exponent(endpt_desc->bInterval * 8, 3, 10);
}

/*
 * Convert interval expressed as 2^(bInterval - 1) == interval into
 * straight exponent value 2^n == interval.
 */
static unsigned int xhci_parse_exponent_interval(struct usb_device *udev,
	struct usb_endpoint_descriptor *endpt_desc)
{
	unsigned int interval;

	interval = clamp_val(endpt_desc->bInterval, 1, 16) - 1;
	if (interval != endpt_desc->bInterval - 1)
		debug("ep %#x - rounding interval to %d %sframes\n",
		      endpt_desc->bEndpointAddress, 1 << interval,
		      udev->speed == USB_SPEED_FULL ? "" : "micro");

	if (udev->speed == USB_SPEED_FULL) {
		/*
		 * Full speed isoc endpoints specify interval in frames,
		 * not microframes. We are using microframes everywhere,
		 * so adjust accordingly.
		 */
		interval += 3;	/* 1 frame = 2^3 uframes */
	}

	return interval;
}

/*
 * Return the polling or NAK interval.
 *
 * The polling interval is expressed in "microframes". If xHCI's Interval field
 * is set to N, it will service the endpoint every 2^(Interval)*125us.
 *
 * The NAK interval is one NAK per 1 to 255 microframes, or no NAKs if interval
 * is set to 0.
 */
static unsigned int xhci_get_endpoint_interval(struct usb_device *udev,
	struct usb_endpoint_descriptor *endpt_desc)
{
	unsigned int interval = 0;

	switch (udev->speed) {
	case USB_SPEED_HIGH:
		/* Max NAK rate */
		if (usb_endpoint_xfer_control(endpt_desc) ||
		    usb_endpoint_xfer_bulk(endpt_desc)) {
			interval = xhci_parse_microframe_interval(udev,
								  endpt_desc);
			break;
		}
		/* Fall through - SS and HS isoc/int have same decoding */

	case USB_SPEED_SUPER:
		if (usb_endpoint_xfer_int(endpt_desc) ||
		    usb_endpoint_xfer_isoc(endpt_desc)) {
			interval = xhci_parse_exponent_interval(udev,
								endpt_desc);
		}
		break;

	case USB_SPEED_FULL:
		if (usb_endpoint_xfer_isoc(endpt_desc)) {
			interval = xhci_parse_exponent_interval(udev,
								endpt_desc);
			break;
		}
		/*
		 * Fall through for interrupt endpoint interval decoding
		 * since it uses the same rules as low speed interrupt
		 * endpoints.
		 */

	case USB_SPEED_LOW:
		if (usb_endpoint_xfer_int(endpt_desc) ||
		    usb_endpoint_xfer_isoc(endpt_desc)) {
			interval = xhci_parse_frame_interval(udev, endpt_desc);
		}
		break;

	default:
		BUG();
	}

	return interval;
}

/*
 * The "Mult" field in the endpoint context is only set for SuperSpeed isoc eps.
 * High speed endpoint descriptors can define "the number of additional
 * transaction opportunities per microframe", but that goes in the Max Burst
 * endpoint context field.
 */
static u32 xhci_get_endpoint_mult(struct usb_device *udev,
	struct usb_endpoint_descriptor *endpt_desc,
	struct usb_ss_ep_comp_descriptor *ss_ep_comp_desc)
{
	if (udev->speed < USB_SPEED_SUPER ||
	    !usb_endpoint_xfer_isoc(endpt_desc))
		return 0;

	return ss_ep_comp_desc->bmAttributes;
}

static u32 xhci_get_endpoint_max_burst(struct usb_device *udev,
	struct usb_endpoint_descriptor *endpt_desc,
	struct usb_ss_ep_comp_descriptor *ss_ep_comp_desc)
{
	/* Super speed and Plus have max burst in ep companion desc */
	if (udev->speed >= USB_SPEED_SUPER)
		return ss_ep_comp_desc->bMaxBurst;

	if (udev->speed == USB_SPEED_HIGH &&
	    (usb_endpoint_xfer_isoc(endpt_desc) ||
	     usb_endpoint_xfer_int(endpt_desc)))
		return usb_endpoint_maxp_mult(endpt_desc) - 1;

	return 0;
}

/*
 * Return the maximum endpoint service interval time (ESIT) payload.
 * Basically, this is the maxpacket size, multiplied by the burst size
 * and mult size.
 */
static u32 xhci_get_max_esit_payload(struct usb_device *udev,
	struct usb_endpoint_descriptor *endpt_desc,
	struct usb_ss_ep_comp_descriptor *ss_ep_comp_desc)
{
	int max_burst;
	int max_packet;

	/* Only applies for interrupt or isochronous endpoints */
	if (usb_endpoint_xfer_control(endpt_desc) ||
	    usb_endpoint_xfer_bulk(endpt_desc))
		return 0;

	/* SuperSpeed Isoc ep with less than 48k per esit */
	if (udev->speed >= USB_SPEED_SUPER)
		return le16_to_cpu(ss_ep_comp_desc->wBytesPerInterval);

	max_packet = usb_endpoint_maxp(endpt_desc);
	max_burst = usb_endpoint_maxp_mult(endpt_desc);

	/* A 0 in max burst means 1 transfer per ESIT */
	return max_packet * max_burst;
}

/**
 * Issue a configure endpoint command or evaluate context command
 * and wait for it to finish.
 *
 * @param udev	pointer to the Device Data Structure
 * @param ctx_change	flag to indicate the Context has changed or NOT
 * @return 0 on success, -1 on failure
 */
static int xhci_configure_endpoints(struct usb_device *udev, bool ctx_change)
{
	struct xhci_container_ctx *in_ctx;
	struct xhci_virt_device *virt_dev;
	struct xhci_ctrl *ctrl = xhci_get_ctrl(udev);
	union xhci_trb *event;

	virt_dev = ctrl->devs[udev->slot_id];
	in_ctx = virt_dev->in_ctx;

	xhci_flush_cache((uintptr_t)in_ctx->bytes, in_ctx->size);
	xhci_queue_command(ctrl, in_ctx->bytes, udev->slot_id, 0,
			   ctx_change ? TRB_EVAL_CONTEXT : TRB_CONFIG_EP);
	event = xhci_wait_for_event(ctrl, TRB_COMPLETION);
	BUG_ON(TRB_TO_SLOT_ID(le32_to_cpu(event->event_cmd.flags))
		!= udev->slot_id);

	switch (GET_COMP_CODE(le32_to_cpu(event->event_cmd.status))) {
	case COMP_SUCCESS:
		debug("Successful %s command\n",
			ctx_change ? "Evaluate Context" : "Configure Endpoint");
		break;
	default:
		printf("ERROR: %s command returned completion code %d.\n",
			ctx_change ? "Evaluate Context" : "Configure Endpoint",
			GET_COMP_CODE(le32_to_cpu(event->event_cmd.status)));
		return -EINVAL;
	}

	xhci_acknowledge_event(ctrl);

	return 0;
}

/**
 * Configure the endpoint, programming the device contexts.
 *
 * @param udev	pointer to the USB device structure
 * @return returns the status of the xhci_configure_endpoints
 */
static int xhci_set_configuration(struct usb_device *udev)
{
	struct xhci_container_ctx *in_ctx;
	struct xhci_container_ctx *out_ctx;
	struct xhci_input_control_ctx *ctrl_ctx;
	struct xhci_slot_ctx *slot_ctx;
	struct xhci_ep_ctx *ep_ctx[MAX_EP_CTX_NUM];
	int cur_ep;
	int max_ep_flag = 0;
	int ep_index;
	unsigned int dir;
	unsigned int ep_type;
	struct xhci_ctrl *ctrl = xhci_get_ctrl(udev);
	int num_of_ep;
	int ep_flag = 0;
	u64 trb_64 = 0;
	int slot_id = udev->slot_id;
	struct xhci_virt_device *virt_dev = ctrl->devs[slot_id];
	struct usb_interface *ifdesc;
	u32 max_esit_payload;
	unsigned int interval;
	unsigned int mult;
	unsigned int max_burst;
	unsigned int avg_trb_len;
	unsigned int err_count = 0;

	out_ctx = virt_dev->out_ctx;
	in_ctx = virt_dev->in_ctx;

	num_of_ep = udev->config.if_desc[0].no_of_ep;
	ifdesc = &udev->config.if_desc[0];

	ctrl_ctx = xhci_get_input_control_ctx(in_ctx);
	/* Initialize the input context control */
	ctrl_ctx->add_flags = cpu_to_le32(SLOT_FLAG);
	ctrl_ctx->drop_flags = 0;

	/* EP_FLAG gives values 1 & 4 for EP1OUT and EP2IN */
	for (cur_ep = 0; cur_ep < num_of_ep; cur_ep++) {
		ep_flag = xhci_get_ep_index(&ifdesc->ep_desc[cur_ep]);
		ctrl_ctx->add_flags |= cpu_to_le32(1 << (ep_flag + 1));
		if (max_ep_flag < ep_flag)
			max_ep_flag = ep_flag;
	}

	xhci_inval_cache((uintptr_t)out_ctx->bytes, out_ctx->size);

	/* slot context */
	xhci_slot_copy(ctrl, in_ctx, out_ctx);
	slot_ctx = xhci_get_slot_ctx(ctrl, in_ctx);
	slot_ctx->dev_info &= ~(cpu_to_le32(LAST_CTX_MASK));
	slot_ctx->dev_info |= cpu_to_le32(LAST_CTX(max_ep_flag + 1) | 0);

	xhci_endpoint_copy(ctrl, in_ctx, out_ctx, 0);

	/* filling up ep contexts */
	for (cur_ep = 0; cur_ep < num_of_ep; cur_ep++) {
		struct usb_endpoint_descriptor *endpt_desc = NULL;
		struct usb_ss_ep_comp_descriptor *ss_ep_comp_desc = NULL;

		endpt_desc = &ifdesc->ep_desc[cur_ep];
		ss_ep_comp_desc = &ifdesc->ss_ep_comp_desc[cur_ep];
		trb_64 = 0;

		/*
		 * Get values to fill the endpoint context, mostly from ep
		 * descriptor. The average TRB buffer lengt for bulk endpoints
		 * is unclear as we have no clue on scatter gather list entry
		 * size. For Isoc and Int, set it to max available.
		 * See xHCI 1.1 spec 4.14.1.1 for details.
		 */
		max_esit_payload = xhci_get_max_esit_payload(udev, endpt_desc,
							     ss_ep_comp_desc);
		interval = xhci_get_endpoint_interval(udev, endpt_desc);
		mult = xhci_get_endpoint_mult(udev, endpt_desc,
					      ss_ep_comp_desc);
		max_burst = xhci_get_endpoint_max_burst(udev, endpt_desc,
							ss_ep_comp_desc);
		avg_trb_len = max_esit_payload;

		ep_index = xhci_get_ep_index(endpt_desc);
		ep_ctx[ep_index] = xhci_get_ep_ctx(ctrl, in_ctx, ep_index);

		/* Allocate the ep rings */
		virt_dev->eps[ep_index].ring = xhci_ring_alloc(1, true);
		if (!virt_dev->eps[ep_index].ring)
			return -ENOMEM;

		/*NOTE: ep_desc[0] actually represents EP1 and so on */
		dir = (((endpt_desc->bEndpointAddress) & (0x80)) >> 7);
		ep_type = (((endpt_desc->bmAttributes) & (0x3)) | (dir << 2));

		ep_ctx[ep_index]->ep_info =
			cpu_to_le32(EP_MAX_ESIT_PAYLOAD_HI(max_esit_payload) |
			EP_INTERVAL(interval) | EP_MULT(mult));

		ep_ctx[ep_index]->ep_info2 =
			cpu_to_le32(ep_type << EP_TYPE_SHIFT);
		ep_ctx[ep_index]->ep_info2 |=
			cpu_to_le32(MAX_PACKET
			(get_unaligned(&endpt_desc->wMaxPacketSize)));

		/* Allow 3 retries for everything but isoc, set CErr = 3 */
		if (!usb_endpoint_xfer_isoc(endpt_desc))
			err_count = 3;
		ep_ctx[ep_index]->ep_info2 |=
			cpu_to_le32(MAX_BURST(max_burst) |
			ERROR_COUNT(err_count));

		trb_64 = (uintptr_t)
				virt_dev->eps[ep_index].ring->enqueue;
		ep_ctx[ep_index]->deq = cpu_to_le64(trb_64 |
				virt_dev->eps[ep_index].ring->cycle_state);

		/*
		 * xHCI spec 6.2.3:
		 * 'Average TRB Length' should be 8 for control endpoints.
		 */
		if (usb_endpoint_xfer_control(endpt_desc))
			avg_trb_len = 8;
		ep_ctx[ep_index]->tx_info =
			cpu_to_le32(EP_MAX_ESIT_PAYLOAD_LO(max_esit_payload) |
			EP_AVG_TRB_LENGTH(avg_trb_len));
	}

	return xhci_configure_endpoints(udev, false);
}

/**
 * Issue an Address Device command (which will issue a SetAddress request to
 * the device).
 *
 * @param udev pointer to the Device Data Structure
 * @return 0 if successful else error code on failure
 */
static int xhci_address_device(struct usb_device *udev, int root_portnr)
{
	int ret = 0;
	struct xhci_ctrl *ctrl = xhci_get_ctrl(udev);
	struct xhci_slot_ctx *slot_ctx;
	struct xhci_input_control_ctx *ctrl_ctx;
	struct xhci_virt_device *virt_dev;
	int slot_id = udev->slot_id;
	union xhci_trb *event;

	virt_dev = ctrl->devs[slot_id];

	/*
	 * This is the first Set Address since device plug-in
	 * so setting up the slot context.
	 */
	debug("Setting up addressable devices %p\n", ctrl->dcbaa);
	xhci_setup_addressable_virt_dev(ctrl, udev, root_portnr);

	ctrl_ctx = xhci_get_input_control_ctx(virt_dev->in_ctx);
	ctrl_ctx->add_flags = cpu_to_le32(SLOT_FLAG | EP0_FLAG);
	ctrl_ctx->drop_flags = 0;

	xhci_queue_command(ctrl, (void *)ctrl_ctx, slot_id, 0, TRB_ADDR_DEV);
	event = xhci_wait_for_event(ctrl, TRB_COMPLETION);
	BUG_ON(TRB_TO_SLOT_ID(le32_to_cpu(event->event_cmd.flags)) != slot_id);

	switch (GET_COMP_CODE(le32_to_cpu(event->event_cmd.status))) {
	case COMP_CTX_STATE:
	case COMP_EBADSLT:
		printf("Setup ERROR: address device command for slot %d.\n",
								slot_id);
		ret = -EINVAL;
		break;
	case COMP_TX_ERR:
		puts("Device not responding to set address.\n");
		ret = -EPROTO;
		break;
	case COMP_DEV_ERR:
		puts("ERROR: Incompatible device"
					"for address device command.\n");
		ret = -ENODEV;
		break;
	case COMP_SUCCESS:
		debug("Successful Address Device command\n");
		udev->status = 0;
		break;
	default:
		printf("ERROR: unexpected command completion code 0x%x.\n",
			GET_COMP_CODE(le32_to_cpu(event->event_cmd.status)));
		ret = -EINVAL;
		break;
	}

	xhci_acknowledge_event(ctrl);

	if (ret < 0)
		/*
		 * TODO: Unsuccessful Address Device command shall leave the
		 * slot in default state. So, issue Disable Slot command now.
		 */
		return ret;

	xhci_inval_cache((uintptr_t)virt_dev->out_ctx->bytes,
			 virt_dev->out_ctx->size);
	slot_ctx = xhci_get_slot_ctx(ctrl, virt_dev->out_ctx);

	debug("xHC internal address is: %d\n",
		le32_to_cpu(slot_ctx->dev_state) & DEV_ADDR_MASK);

	return 0;
}

/**
 * Issue Enable slot command to the controller to allocate
 * device slot and assign the slot id. It fails if the xHC
 * ran out of device slots, the Enable Slot command timed out,
 * or allocating memory failed.
 *
 * @param udev	pointer to the Device Data Structure
 * @return Returns 0 on succes else return error code on failure
 */
static int _xhci_alloc_device(struct usb_device *udev)
{
	struct xhci_ctrl *ctrl = xhci_get_ctrl(udev);
	union xhci_trb *event;
	int ret;

	/*
	 * Root hub will be first device to be initailized.
	 * If this device is root-hub, don't do any xHC related
	 * stuff.
	 */
	if (ctrl->rootdev == 0) {
		udev->speed = USB_SPEED_SUPER;
		return 0;
	}

	xhci_queue_command(ctrl, NULL, 0, 0, TRB_ENABLE_SLOT);
	event = xhci_wait_for_event(ctrl, TRB_COMPLETION);
	BUG_ON(GET_COMP_CODE(le32_to_cpu(event->event_cmd.status))
		!= COMP_SUCCESS);

	udev->slot_id = TRB_TO_SLOT_ID(le32_to_cpu(event->event_cmd.flags));

	xhci_acknowledge_event(ctrl);

	ret = xhci_alloc_virt_device(ctrl, udev->slot_id);
	if (ret < 0) {
		/*
		 * TODO: Unsuccessful Address Device command shall leave
		 * the slot in default. So, issue Disable Slot command now.
		 */
		puts("Could not allocate xHCI USB device data structures\n");
		return ret;
	}

	return 0;
}

#if !CONFIG_IS_ENABLED(DM_USB)
int usb_alloc_device(struct usb_device *udev)
{
	return _xhci_alloc_device(udev);
}
#endif

/*
 * Full speed devices may have a max packet size greater than 8 bytes, but the
 * USB core doesn't know that until it reads the first 8 bytes of the
 * descriptor.  If the usb_device's max packet size changes after that point,
 * we need to issue an evaluate context command and wait on it.
 *
 * @param udev	pointer to the Device Data Structure
 * @return returns the status of the xhci_configure_endpoints
 */
int xhci_check_maxpacket(struct usb_device *udev)
{
	struct xhci_ctrl *ctrl = xhci_get_ctrl(udev);
	unsigned int slot_id = udev->slot_id;
	int ep_index = 0;	/* control endpoint */
	struct xhci_container_ctx *in_ctx;
	struct xhci_container_ctx *out_ctx;
	struct xhci_input_control_ctx *ctrl_ctx;
	struct xhci_ep_ctx *ep_ctx;
	int max_packet_size;
	int hw_max_packet_size;
	int ret = 0;

	out_ctx = ctrl->devs[slot_id]->out_ctx;
	xhci_inval_cache((uintptr_t)out_ctx->bytes, out_ctx->size);

	ep_ctx = xhci_get_ep_ctx(ctrl, out_ctx, ep_index);
	hw_max_packet_size = MAX_PACKET_DECODED(le32_to_cpu(ep_ctx->ep_info2));
	max_packet_size = udev->epmaxpacketin[0];
	if (hw_max_packet_size != max_packet_size) {
		debug("Max Packet Size for ep 0 changed.\n");
		debug("Max packet size in usb_device = %d\n", max_packet_size);
		debug("Max packet size in xHCI HW = %d\n", hw_max_packet_size);
		debug("Issuing evaluate context command.\n");

		/* Set up the modified control endpoint 0 */
		xhci_endpoint_copy(ctrl, ctrl->devs[slot_id]->in_ctx,
				ctrl->devs[slot_id]->out_ctx, ep_index);
		in_ctx = ctrl->devs[slot_id]->in_ctx;
		ep_ctx = xhci_get_ep_ctx(ctrl, in_ctx, ep_index);
		ep_ctx->ep_info2 &= cpu_to_le32(~((0xffff & MAX_PACKET_MASK)
						<< MAX_PACKET_SHIFT));
		ep_ctx->ep_info2 |= cpu_to_le32(MAX_PACKET(max_packet_size));

		/*
		 * Set up the input context flags for the command
		 * FIXME: This won't work if a non-default control endpoint
		 * changes max packet sizes.
		 */
		ctrl_ctx = xhci_get_input_control_ctx(in_ctx);
		ctrl_ctx->add_flags = cpu_to_le32(EP0_FLAG);
		ctrl_ctx->drop_flags = 0;

		ret = xhci_configure_endpoints(udev, true);
	}
	return ret;
}

/**
 * Clears the Change bits of the Port Status Register
 *
 * @param wValue	request value
 * @param wIndex	request index
 * @param addr		address of posrt status register
 * @param port_status	state of port status register
 * @return none
 */
static void xhci_clear_port_change_bit(u16 wValue,
		u16 wIndex, volatile uint32_t *addr, u32 port_status)
{
	char *port_change_bit;
	u32 status;

	switch (wValue) {
	case USB_PORT_FEAT_C_RESET:
		status = PORT_RC;
		port_change_bit = "reset";
		break;
	case USB_PORT_FEAT_C_CONNECTION:
		status = PORT_CSC;
		port_change_bit = "connect";
		break;
	case USB_PORT_FEAT_C_OVER_CURRENT:
		status = PORT_OCC;
		port_change_bit = "over-current";
		break;
	case USB_PORT_FEAT_C_ENABLE:
		status = PORT_PEC;
		port_change_bit = "enable/disable";
		break;
	case USB_PORT_FEAT_C_SUSPEND:
		status = PORT_PLC;
		port_change_bit = "suspend/resume";
		break;
	default:
		/* Should never happen */
		return;
	}

	/* Change bits are all write 1 to clear */
	xhci_writel(addr, port_status | status);

	port_status = xhci_readl(addr);
	debug("clear port %s change, actual port %d status  = 0x%x\n",
			port_change_bit, wIndex, port_status);
}

/**
 * Save Read Only (RO) bits and save read/write bits where
 * writing a 0 clears the bit and writing a 1 sets the bit (RWS).
 * For all other types (RW1S, RW1CS, RW, and RZ), writing a '0' has no effect.
 *
 * @param state	state of the Port Status and Control Regsiter
 * @return a value that would result in the port being in the
 *	   same state, if the value was written to the port
 *	   status control register.
 */
static u32 xhci_port_state_to_neutral(u32 state)
{
	/* Save read-only status and port state */
	return (state & XHCI_PORT_RO) | (state & XHCI_PORT_RWS);
}

/**
 * Submits the Requests to the XHCI Host Controller
 *
 * @param udev pointer to the USB device structure
 * @param pipe contains the DIR_IN or OUT , devnum
 * @param buffer buffer to be read/written based on the request
 * @return returns 0 if successful else -1 on failure
 */
static int xhci_submit_root(struct usb_device *udev, unsigned long pipe,
			void *buffer, struct devrequest *req)
{
	uint8_t tmpbuf[4];
	u16 typeReq;
	void *srcptr = NULL;
	int len, srclen;
	uint32_t reg;
	volatile uint32_t *status_reg;
	struct xhci_ctrl *ctrl = xhci_get_ctrl(udev);
	struct xhci_hccr *hccr = ctrl->hccr;
	struct xhci_hcor *hcor = ctrl->hcor;
	int max_ports = HCS_MAX_PORTS(xhci_readl(&hccr->cr_hcsparams1));

	if ((req->requesttype & USB_RT_PORT) &&
	    le16_to_cpu(req->index) > max_ports) {
		printf("The request port(%d) exceeds maximum port number\n",
		       le16_to_cpu(req->index) - 1);
		return -EINVAL;
	}

	status_reg = (volatile uint32_t *)
		     (&hcor->portregs[le16_to_cpu(req->index) - 1].or_portsc);
	srclen = 0;

	typeReq = req->request | req->requesttype << 8;

	switch (typeReq) {
	case DeviceRequest | USB_REQ_GET_DESCRIPTOR:
		switch (le16_to_cpu(req->value) >> 8) {
		case USB_DT_DEVICE:
			debug("USB_DT_DEVICE request\n");
			srcptr = &descriptor.device;
			srclen = 0x12;
			break;
		case USB_DT_CONFIG:
			debug("USB_DT_CONFIG config\n");
			srcptr = &descriptor.config;
			srclen = 0x19;
			break;
		case USB_DT_STRING:
			debug("USB_DT_STRING config\n");
			switch (le16_to_cpu(req->value) & 0xff) {
			case 0:	/* Language */
				srcptr = "\4\3\11\4";
				srclen = 4;
				break;
			case 1:	/* Vendor String  */
				srcptr = "\16\3U\0-\0B\0o\0o\0t\0";
				srclen = 14;
				break;
			case 2:	/* Product Name */
				srcptr = "\52\3X\0H\0C\0I\0 "
					 "\0H\0o\0s\0t\0 "
					 "\0C\0o\0n\0t\0r\0o\0l\0l\0e\0r\0";
				srclen = 42;
				break;
			default:
				printf("unknown value DT_STRING %x\n",
					le16_to_cpu(req->value));
				goto unknown;
			}
			break;
		default:
			printf("unknown value %x\n", le16_to_cpu(req->value));
			goto unknown;
		}
		break;
	case USB_REQ_GET_DESCRIPTOR | ((USB_DIR_IN | USB_RT_HUB) << 8):
		switch (le16_to_cpu(req->value) >> 8) {
		case USB_DT_HUB:
		case USB_DT_SS_HUB:
			debug("USB_DT_HUB config\n");
			srcptr = &descriptor.hub;
			srclen = 0x8;
			break;
		default:
			printf("unknown value %x\n", le16_to_cpu(req->value));
			goto unknown;
		}
		break;
	case USB_REQ_SET_ADDRESS | (USB_RECIP_DEVICE << 8):
		debug("USB_REQ_SET_ADDRESS\n");
		ctrl->rootdev = le16_to_cpu(req->value);
		break;
	case DeviceOutRequest | USB_REQ_SET_CONFIGURATION:
		/* Do nothing */
		break;
	case USB_REQ_GET_STATUS | ((USB_DIR_IN | USB_RT_HUB) << 8):
		tmpbuf[0] = 1;	/* USB_STATUS_SELFPOWERED */
		tmpbuf[1] = 0;
		srcptr = tmpbuf;
		srclen = 2;
		break;
	case USB_REQ_GET_STATUS | ((USB_RT_PORT | USB_DIR_IN) << 8):
		memset(tmpbuf, 0, 4);
		reg = xhci_readl(status_reg);
		if (reg & PORT_CONNECT) {
			tmpbuf[0] |= USB_PORT_STAT_CONNECTION;
			switch (reg & DEV_SPEED_MASK) {
			case XDEV_FS:
				debug("SPEED = FULLSPEED\n");
				break;
			case XDEV_LS:
				debug("SPEED = LOWSPEED\n");
				tmpbuf[1] |= USB_PORT_STAT_LOW_SPEED >> 8;
				break;
			case XDEV_HS:
				debug("SPEED = HIGHSPEED\n");
				tmpbuf[1] |= USB_PORT_STAT_HIGH_SPEED >> 8;
				break;
			case XDEV_SS:
				debug("SPEED = SUPERSPEED\n");
				tmpbuf[1] |= USB_PORT_STAT_SUPER_SPEED >> 8;
				break;
			}
		}
		if (reg & PORT_PE)
			tmpbuf[0] |= USB_PORT_STAT_ENABLE;
		if ((reg & PORT_PLS_MASK) == XDEV_U3)
			tmpbuf[0] |= USB_PORT_STAT_SUSPEND;
		if (reg & PORT_OC)
			tmpbuf[0] |= USB_PORT_STAT_OVERCURRENT;
		if (reg & PORT_RESET)
			tmpbuf[0] |= USB_PORT_STAT_RESET;
		if (reg & PORT_POWER)
			/*
			 * XXX: This Port power bit (for USB 3.0 hub)
			 * we are faking in USB 2.0 hub port status;
			 * since there's a change in bit positions in
			 * two:
			 * USB 2.0 port status PP is at position[8]
			 * USB 3.0 port status PP is at position[9]
			 * So, we are still keeping it at position [8]
			 */
			tmpbuf[1] |= USB_PORT_STAT_POWER >> 8;
		if (reg & PORT_CSC)
			tmpbuf[2] |= USB_PORT_STAT_C_CONNECTION;
		if (reg & PORT_PEC)
			tmpbuf[2] |= USB_PORT_STAT_C_ENABLE;
		if (reg & PORT_OCC)
			tmpbuf[2] |= USB_PORT_STAT_C_OVERCURRENT;
		if (reg & PORT_RC)
			tmpbuf[2] |= USB_PORT_STAT_C_RESET;

		srcptr = tmpbuf;
		srclen = 4;
		break;
	case USB_REQ_SET_FEATURE | ((USB_DIR_OUT | USB_RT_PORT) << 8):
		reg = xhci_readl(status_reg);
		reg = xhci_port_state_to_neutral(reg);
		switch (le16_to_cpu(req->value)) {
		case USB_PORT_FEAT_ENABLE:
			reg |= PORT_PE;
			xhci_writel(status_reg, reg);
			break;
		case USB_PORT_FEAT_POWER:
			reg |= PORT_POWER;
			xhci_writel(status_reg, reg);
			break;
		case USB_PORT_FEAT_RESET:
			reg |= PORT_RESET;
			xhci_writel(status_reg, reg);
			break;
		default:
			printf("unknown feature %x\n", le16_to_cpu(req->value));
			goto unknown;
		}
		break;
	case USB_REQ_CLEAR_FEATURE | ((USB_DIR_OUT | USB_RT_PORT) << 8):
		reg = xhci_readl(status_reg);
		reg = xhci_port_state_to_neutral(reg);
		switch (le16_to_cpu(req->value)) {
		case USB_PORT_FEAT_ENABLE:
			reg &= ~PORT_PE;
			break;
		case USB_PORT_FEAT_POWER:
			reg &= ~PORT_POWER;
			break;
		case USB_PORT_FEAT_C_RESET:
		case USB_PORT_FEAT_C_CONNECTION:
		case USB_PORT_FEAT_C_OVER_CURRENT:
		case USB_PORT_FEAT_C_ENABLE:
			xhci_clear_port_change_bit((le16_to_cpu(req->value)),
							le16_to_cpu(req->index),
							status_reg, reg);
			break;
		default:
			printf("unknown feature %x\n", le16_to_cpu(req->value));
			goto unknown;
		}
		xhci_writel(status_reg, reg);
		break;
	default:
		puts("Unknown request\n");
		goto unknown;
	}

	debug("scrlen = %d\n req->length = %d\n",
		srclen, le16_to_cpu(req->length));

	len = min(srclen, (int)le16_to_cpu(req->length));

	if (srcptr != NULL && len > 0)
		memcpy(buffer, srcptr, len);
	else
		debug("Len is 0\n");

	udev->act_len = len;
	udev->status = 0;

	return 0;

unknown:
	udev->act_len = 0;
	udev->status = USB_ST_STALLED;

	return -ENODEV;
}

/**
 * Submits the INT request to XHCI Host cotroller
 *
 * @param udev	pointer to the USB device
 * @param pipe		contains the DIR_IN or OUT , devnum
 * @param buffer	buffer to be read/written based on the request
 * @param length	length of the buffer
 * @param interval	interval of the interrupt
 * @return 0
 */
static int _xhci_submit_int_msg(struct usb_device *udev, unsigned long pipe,
				void *buffer, int length, int interval)
{
	if (usb_pipetype(pipe) != PIPE_INTERRUPT) {
		printf("non-interrupt pipe (type=%lu)", usb_pipetype(pipe));
		return -EINVAL;
	}

	/*
	 * xHCI uses normal TRBs for both bulk and interrupt. When the
	 * interrupt endpoint is to be serviced, the xHC will consume
	 * (at most) one TD. A TD (comprised of sg list entries) can
	 * take several service intervals to transmit.
	 */
	return xhci_bulk_tx(udev, pipe, length, buffer);
}

/**
 * submit the BULK type of request to the USB Device
 *
 * @param udev	pointer to the USB device
 * @param pipe		contains the DIR_IN or OUT , devnum
 * @param buffer	buffer to be read/written based on the request
 * @param length	length of the buffer
 * @return returns 0 if successful else -1 on failure
 */
static int _xhci_submit_bulk_msg(struct usb_device *udev, unsigned long pipe,
				 void *buffer, int length)
{
	if (usb_pipetype(pipe) != PIPE_BULK) {
		printf("non-bulk pipe (type=%lu)", usb_pipetype(pipe));
		return -EINVAL;
	}

	return xhci_bulk_tx(udev, pipe, length, buffer);
}

/**
 * submit the control type of request to the Root hub/Device based on the devnum
 *
 * @param udev	pointer to the USB device
 * @param pipe		contains the DIR_IN or OUT , devnum
 * @param buffer	buffer to be read/written based on the request
 * @param length	length of the buffer
 * @param setup		Request type
 * @param root_portnr	Root port number that this device is on
 * @return returns 0 if successful else -1 on failure
 */
static int _xhci_submit_control_msg(struct usb_device *udev, unsigned long pipe,
				    void *buffer, int length,
				    struct devrequest *setup, int root_portnr)
{
	struct xhci_ctrl *ctrl = xhci_get_ctrl(udev);
	int ret = 0;

	if (usb_pipetype(pipe) != PIPE_CONTROL) {
		printf("non-control pipe (type=%lu)", usb_pipetype(pipe));
		return -EINVAL;
	}

	if (usb_pipedevice(pipe) == ctrl->rootdev)
		return xhci_submit_root(udev, pipe, buffer, setup);

	if (setup->request == USB_REQ_SET_ADDRESS &&
	   (setup->requesttype & USB_TYPE_MASK) == USB_TYPE_STANDARD)
		return xhci_address_device(udev, root_portnr);

	if (setup->request == USB_REQ_SET_CONFIGURATION &&
	   (setup->requesttype & USB_TYPE_MASK) == USB_TYPE_STANDARD) {
		ret = xhci_set_configuration(udev);
		if (ret) {
			puts("Failed to configure xHCI endpoint\n");
			return ret;
		}
	}

	return xhci_ctrl_tx(udev, pipe, setup, length, buffer);
}

static int xhci_lowlevel_init(struct xhci_ctrl *ctrl)
{
	struct xhci_hccr *hccr;
	struct xhci_hcor *hcor;
	uint32_t val;
	uint32_t val2;
	uint32_t reg;

	hccr = ctrl->hccr;
	hcor = ctrl->hcor;
	/*
	 * Program the Number of Device Slots Enabled field in the CONFIG
	 * register with the max value of slots the HC can handle.
	 */
	val = (xhci_readl(&hccr->cr_hcsparams1) & HCS_SLOTS_MASK);
	val2 = xhci_readl(&hcor->or_config);
	val |= (val2 & ~HCS_SLOTS_MASK);
	xhci_writel(&hcor->or_config, val);

	/* initializing xhci data structures */
	if (xhci_mem_init(ctrl, hccr, hcor) < 0)
		return -ENOMEM;

	reg = xhci_readl(&hccr->cr_hcsparams1);
	descriptor.hub.bNbrPorts = ((reg & HCS_MAX_PORTS_MASK) >>
						HCS_MAX_PORTS_SHIFT);
	printf("Register %x NbrPorts %d\n", reg, descriptor.hub.bNbrPorts);

	/* Port Indicators */
	reg = xhci_readl(&hccr->cr_hccparams);
	if (HCS_INDICATOR(reg))
		put_unaligned(get_unaligned(&descriptor.hub.wHubCharacteristics)
				| 0x80, &descriptor.hub.wHubCharacteristics);

	/* Port Power Control */
	if (HCC_PPC(reg))
		put_unaligned(get_unaligned(&descriptor.hub.wHubCharacteristics)
				| 0x01, &descriptor.hub.wHubCharacteristics);

	if (xhci_start(hcor)) {
		xhci_reset(hcor);
		return -ENODEV;
	}

	/* Zero'ing IRQ control register and IRQ pending register */
	xhci_writel(&ctrl->ir_set->irq_control, 0x0);
	xhci_writel(&ctrl->ir_set->irq_pending, 0x0);

	reg = HC_VERSION(xhci_readl(&hccr->cr_capbase));
	printf("USB XHCI %x.%02x\n", reg >> 8, reg & 0xff);

	return 0;
}

static int xhci_lowlevel_stop(struct xhci_ctrl *ctrl)
{
	u32 temp;

	xhci_reset(ctrl->hcor);

	debug("// Disabling event ring interrupts\n");
	temp = xhci_readl(&ctrl->hcor->or_usbsts);
	xhci_writel(&ctrl->hcor->or_usbsts, temp & ~STS_EINT);
	temp = xhci_readl(&ctrl->ir_set->irq_pending);
	xhci_writel(&ctrl->ir_set->irq_pending, ER_IRQ_DISABLE(temp));

	return 0;
}

#if !CONFIG_IS_ENABLED(DM_USB)
int submit_control_msg(struct usb_device *udev, unsigned long pipe,
		       void *buffer, int length, struct devrequest *setup)
{
	struct usb_device *hop = udev;

	if (hop->parent)
		while (hop->parent->parent)
			hop = hop->parent;

	return _xhci_submit_control_msg(udev, pipe, buffer, length, setup,
					hop->portnr);
}

int submit_bulk_msg(struct usb_device *udev, unsigned long pipe, void *buffer,
		    int length)
{
	return _xhci_submit_bulk_msg(udev, pipe, buffer, length);
}

int submit_int_msg(struct usb_device *udev, unsigned long pipe, void *buffer,
		   int length, int interval)
{
	return _xhci_submit_int_msg(udev, pipe, buffer, length, interval);
}

/**
 * Intialises the XHCI host controller
 * and allocates the necessary data structures
 *
 * @param index	index to the host controller data structure
 * @return pointer to the intialised controller
 */
int usb_lowlevel_init(int index, enum usb_init_type init, void **controller)
{
	struct xhci_hccr *hccr;
	struct xhci_hcor *hcor;
	struct xhci_ctrl *ctrl;
	int ret;

	*controller = NULL;

	if (xhci_hcd_init(index, &hccr, (struct xhci_hcor **)&hcor) != 0)
		return -ENODEV;

	if (xhci_reset(hcor) != 0)
		return -ENODEV;

	ctrl = &xhcic[index];

	ctrl->hccr = hccr;
	ctrl->hcor = hcor;

	ret = xhci_lowlevel_init(ctrl);

	if (ret) {
		ctrl->hccr = NULL;
		ctrl->hcor = NULL;
	} else {
		*controller = &xhcic[index];
	}

	return ret;
}

/**
 * Stops the XHCI host controller
 * and cleans up all the related data structures
 *
 * @param index	index to the host controller data structure
 * @return none
 */
int usb_lowlevel_stop(int index)
{
	struct xhci_ctrl *ctrl = (xhcic + index);

	if (ctrl->hcor) {
		xhci_lowlevel_stop(ctrl);
		xhci_hcd_stop(index);
		xhci_cleanup(ctrl);
	}

	return 0;
}
#endif /* CONFIG_IS_ENABLED(DM_USB) */

#if CONFIG_IS_ENABLED(DM_USB)

static int xhci_submit_control_msg(struct udevice *dev, struct usb_device *udev,
				   unsigned long pipe, void *buffer, int length,
				   struct devrequest *setup)
{
	struct usb_device *uhop;
	struct udevice *hub;
	int root_portnr = 0;

	debug("%s: dev='%s', udev=%p, udev->dev='%s', portnr=%d\n", __func__,
	      dev->name, udev, udev->dev->name, udev->portnr);
	hub = udev->dev;
	if (device_get_uclass_id(hub) == UCLASS_USB_HUB) {
		/* Figure out our port number on the root hub */
		if (usb_hub_is_root_hub(hub)) {
			root_portnr = udev->portnr;
		} else {
			while (!usb_hub_is_root_hub(hub->parent))
				hub = hub->parent;
			uhop = dev_get_parent_priv(hub);
			root_portnr = uhop->portnr;
		}
	}
/*
	struct usb_device *hop = udev;

	if (hop->parent)
		while (hop->parent->parent)
			hop = hop->parent;
*/
	return _xhci_submit_control_msg(udev, pipe, buffer, length, setup,
					root_portnr);
}

static int xhci_submit_bulk_msg(struct udevice *dev, struct usb_device *udev,
				unsigned long pipe, void *buffer, int length)
{
	debug("%s: dev='%s', udev=%p\n", __func__, dev->name, udev);
	return _xhci_submit_bulk_msg(udev, pipe, buffer, length);
}

static int xhci_submit_int_msg(struct udevice *dev, struct usb_device *udev,
			       unsigned long pipe, void *buffer, int length,
			       int interval)
{
	debug("%s: dev='%s', udev=%p\n", __func__, dev->name, udev);
	return _xhci_submit_int_msg(udev, pipe, buffer, length, interval);
}

static int xhci_alloc_device(struct udevice *dev, struct usb_device *udev)
{
	debug("%s: dev='%s', udev=%p\n", __func__, dev->name, udev);
	return _xhci_alloc_device(udev);
}

static int xhci_update_hub_device(struct udevice *dev, struct usb_device *udev)
{
	struct xhci_ctrl *ctrl = dev_get_priv(dev);
	struct usb_hub_device *hub = dev_get_uclass_priv(udev->dev);
	struct xhci_virt_device *virt_dev;
	struct xhci_input_control_ctx *ctrl_ctx;
	struct xhci_container_ctx *out_ctx;
	struct xhci_container_ctx *in_ctx;
	struct xhci_slot_ctx *slot_ctx;
	int slot_id = udev->slot_id;
	unsigned think_time;

	debug("%s: dev='%s', udev=%p\n", __func__, dev->name, udev);

	/* Ignore root hubs */
	if (usb_hub_is_root_hub(udev->dev))
		return 0;

	virt_dev = ctrl->devs[slot_id];
	BUG_ON(!virt_dev);

	out_ctx = virt_dev->out_ctx;
	in_ctx = virt_dev->in_ctx;

	ctrl_ctx = xhci_get_input_control_ctx(in_ctx);
	/* Initialize the input context control */
	ctrl_ctx->add_flags = cpu_to_le32(SLOT_FLAG);
	ctrl_ctx->drop_flags = 0;

	xhci_inval_cache((uintptr_t)out_ctx->bytes, out_ctx->size);

	/* slot context */
	xhci_slot_copy(ctrl, in_ctx, out_ctx);
	slot_ctx = xhci_get_slot_ctx(ctrl, in_ctx);

	/* Update hub related fields */
	slot_ctx->dev_info |= cpu_to_le32(DEV_HUB);
	/*
	 * refer to section 6.2.2: MTT should be 0 for full speed hub,
	 * but it may be already set to 1 when setup an xHCI virtual
	 * device, so clear it anyway.
	 */
	if (hub->tt.multi)
		slot_ctx->dev_info |= cpu_to_le32(DEV_MTT);
	else if (udev->speed == USB_SPEED_FULL)
		slot_ctx->dev_info &= cpu_to_le32(~DEV_MTT);
	slot_ctx->dev_info2 |= cpu_to_le32(XHCI_MAX_PORTS(udev->maxchild));
	/*
	 * Set TT think time - convert from ns to FS bit times.
	 * Note 8 FS bit times == (8 bits / 12000000 bps) ~= 666ns
	 *
	 * 0 =  8 FS bit times, 1 = 16 FS bit times,
	 * 2 = 24 FS bit times, 3 = 32 FS bit times.
	 *
	 * This field shall be 0 if the device is not a high-spped hub.
	 */
	think_time = hub->tt.think_time;
	if (think_time != 0)
		think_time = (think_time / 666) - 1;
	if (udev->speed == USB_SPEED_HIGH)
		slot_ctx->tt_info |= cpu_to_le32(TT_THINK_TIME(think_time));
	slot_ctx->dev_state = 0;

	return xhci_configure_endpoints(udev, false);
}

static int xhci_get_max_xfer_size(struct udevice *dev, size_t *size)
{
	/*
	 * xHCD allocates one segment which includes 64 TRBs for each endpoint
	 * and the last TRB in this segment is configured as a link TRB to form
	 * a TRB ring. Each TRB can transfer up to 64K bytes, however data
	 * buffers referenced by transfer TRBs shall not span 64KB boundaries.
	 * Hence the maximum number of TRBs we can use in one transfer is 62.
	 */
	*size = (TRBS_PER_SEGMENT - 2) * TRB_MAX_BUFF_SIZE;

	return 0;
}

int xhci_register(struct udevice *dev, struct xhci_hccr *hccr,
		  struct xhci_hcor *hcor)
{
	struct xhci_ctrl *ctrl = dev_get_priv(dev);
	struct usb_bus_priv *priv = dev_get_uclass_priv(dev);
	int ret;

	debug("%s: dev='%s', ctrl=%p, hccr=%p, hcor=%p\n", __func__, dev->name,
	      ctrl, hccr, hcor);

	ctrl->dev = dev;

	/*
	 * XHCI needs to issue a Address device command to setup
	 * proper device context structures, before it can interact
	 * with the device. So a get_descriptor will fail before any
	 * of that is done for XHCI unlike EHCI.
	 */
	priv->desc_before_addr = false;

	ret = xhci_reset(hcor);
	if (ret)
		goto err;

	ctrl->hccr = hccr;
	ctrl->hcor = hcor;
	ret = xhci_lowlevel_init(ctrl);
	if (ret)
		goto err;

	return 0;
err:
	free(ctrl);
	debug("%s: failed, ret=%d\n", __func__, ret);
	return ret;
}

int xhci_deregister(struct udevice *dev)
{
	struct xhci_ctrl *ctrl = dev_get_priv(dev);

	xhci_lowlevel_stop(ctrl);
	xhci_cleanup(ctrl);

	return 0;
}

struct dm_usb_ops xhci_usb_ops = {
	.control = xhci_submit_control_msg,
	.bulk = xhci_submit_bulk_msg,
	.interrupt = xhci_submit_int_msg,
	.alloc_device = xhci_alloc_device,
	.update_hub_device = xhci_update_hub_device,
	.get_max_xfer_size  = xhci_get_max_xfer_size,
};

#endif
