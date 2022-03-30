/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Intel PXA25x on-chip full speed USB device controller
 *
 * Copyright (C) 2003 Robert Schwebel <r.schwebel@pengutronix.de>, Pengutronix
 * Copyright (C) 2003 David Brownell
 * Copyright (C) 2012 Lukasz Dalek <luk0104@gmail.com>
 */

#ifndef __LINUX_USB_GADGET_PXA25X_H
#define __LINUX_USB_GADGET_PXA25X_H

#include <linux/types.h>
#include <asm/arch/regs-usb.h>

/*
 * Prefetching support - only ARMv5.
 */

#ifdef ARCH_HAS_PREFETCH
static inline void prefetch(const void *ptr)
{
	__asm__ __volatile__(
		"pld\t%a0"
		:
		: "p" (ptr)
		: "cc");
}

#define prefetchw(ptr)	prefetch(ptr)
#endif /* ARCH_HAS_PREFETCH */

/*-------------------------------------------------------------------------*/

#define UDC_REGS	((struct pxa25x_udc_regs *)PXA25X_UDC_BASE)

/*-------------------------------------------------------------------------*/

struct pxa2xx_udc_mach_info {
	int  (*udc_is_connected)(void);		/* do we see host? */
	void (*udc_command)(int cmd);
#define	PXA2XX_UDC_CMD_CONNECT		0	/* let host see us */
#define	PXA2XX_UDC_CMD_DISCONNECT	1	/* so host won't see us */
};

struct pxa25x_udc;

struct pxa25x_ep {
	struct usb_ep				ep;
	struct pxa25x_udc			*dev;

	const struct usb_endpoint_descriptor	*desc;
	struct list_head			queue;
	unsigned long				pio_irqs;

	unsigned short				fifo_size;
	u8					bEndpointAddress;
	u8					bmAttributes;

	unsigned				stopped:1;

	/* UDCCS = UDC Control/Status for this EP
	 * UBCR = UDC Byte Count Remaining (contents of OUT fifo)
	 * UDDR = UDC Endpoint Data Register (the fifo)
	 * DRCM = DMA Request Channel Map
	 */
	u32					*reg_udccs;
	u32					*reg_ubcr;
	u32					*reg_uddr;
};

struct pxa25x_request {
	struct usb_request			req;
	struct list_head			queue;
};

enum ep0_state {
	EP0_IDLE,
	EP0_IN_DATA_PHASE,
	EP0_OUT_DATA_PHASE,
	EP0_END_XFER,
	EP0_STALL,
};

#define EP0_FIFO_SIZE	16U
#define BULK_FIFO_SIZE	64U
#define ISO_FIFO_SIZE	256U
#define INT_FIFO_SIZE	8U

struct udc_stats {
	struct ep0stats {
		unsigned long		ops;
		unsigned long		bytes;
	} read, write;
	unsigned long			irqs;
};

#ifdef CONFIG_USB_PXA25X_SMALL
/* when memory's tight, SMALL config saves code+data.  */
#define	PXA_UDC_NUM_ENDPOINTS	3
#endif

#ifndef	PXA_UDC_NUM_ENDPOINTS
#define	PXA_UDC_NUM_ENDPOINTS	16
#endif

struct pxa25x_watchdog {
	unsigned				running:1;
	ulong					period;
	ulong					base;
	struct pxa25x_udc			*udc;

	void (*function)(struct pxa25x_udc *udc);
};

struct pxa25x_udc {
	struct usb_gadget			gadget;
	struct usb_gadget_driver		*driver;
	struct pxa25x_udc_regs			*regs;

	enum ep0_state				ep0state;
	struct udc_stats			stats;
	unsigned				got_irq:1,
						pullup:1,
						has_cfr:1,
						req_pending:1,
						req_std:1,
						req_config:1,
						active:1;

	struct clk				*clk;
	struct pxa2xx_udc_mach_info		*mach;
	u64					dma_mask;
	struct pxa25x_ep			ep[PXA_UDC_NUM_ENDPOINTS];

	struct pxa25x_watchdog			watchdog;
};

/*-------------------------------------------------------------------------*/

static struct pxa25x_udc *the_controller;

/*-------------------------------------------------------------------------*/

#ifndef DEBUG
# define NOISY 0
#endif

#endif /* __LINUX_USB_GADGET_PXA25X_H */
