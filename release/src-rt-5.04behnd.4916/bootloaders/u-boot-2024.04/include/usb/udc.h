/* SPDX-License-Identifier: GPL-2.0+ */
#ifndef USB_UDC_H
#define USB_UDC_H

#ifndef EP0_MAX_PACKET_SIZE
#define EP0_MAX_PACKET_SIZE     64
#endif

#ifndef EP_MAX_PACKET_SIZE
#define EP_MAX_PACKET_SIZE	64
#endif

#if !defined(CONFIG_PPC)
/* mpc8xx_udc.h will set these values */
#define UDC_OUT_PACKET_SIZE     EP_MAX_PACKET_SIZE
#define UDC_IN_PACKET_SIZE      EP_MAX_PACKET_SIZE
#define UDC_INT_PACKET_SIZE     EP_MAX_PACKET_SIZE
#define UDC_BULK_PACKET_SIZE    EP_MAX_PACKET_SIZE
#endif

#define UDC_BULK_HS_PACKET_SIZE	512

#ifndef UDC_INT_ENDPOINT
#define UDC_INT_ENDPOINT	1
#endif

#ifndef UDC_OUT_ENDPOINT
#define UDC_OUT_ENDPOINT	2
#endif

#ifndef UDC_IN_ENDPOINT
#define UDC_IN_ENDPOINT		3
#endif

/* function declarations */
int udc_init(void);
void udc_irq(void);
int udc_endpoint_write(struct usb_endpoint_instance *endpoint);
void udc_setup_ep(struct usb_device_instance *device, unsigned int ep,
		  struct usb_endpoint_instance *endpoint);
void udc_connect(void);
void udc_disconnect(void);
void udc_enable(struct usb_device_instance *device);
void udc_disable(void);
void udc_startup_events(struct usb_device_instance *device);

/* Flow control */
void udc_set_nak(int epid);
void udc_unset_nak(int epid);

#endif
