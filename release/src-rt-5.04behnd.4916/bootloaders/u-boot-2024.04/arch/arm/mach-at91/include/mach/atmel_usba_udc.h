/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2005-2013 Atmel Corporation
 *			   Bo Shen <voice.shen@atmel.com>
 */

#ifndef __ATMEL_USBA_UDC_H__
#define __ATMEL_USBA_UDC_H__

#include <linux/usb/atmel_usba_udc.h>

#define EP(nam, idx, maxpkt, maxbk, dma, isoc)		\
	[idx] = {					\
		.name	= nam,				\
		.index	= idx,				\
		.fifo_size	= maxpkt,		\
		.nr_banks	= maxbk,		\
		.can_dma	= dma,			\
		.can_isoc	= isoc,			\
	}

#if defined(CONFIG_AT91SAM9G45) || defined(CONFIG_AT91SAM9M10G45) || \
	defined(CONFIG_AT91SAM9X5)
static struct usba_ep_data usba_udc_ep[] = {
	EP("ep0", 0, 64, 1, 0, 0),
	EP("ep1", 1, 1024, 2, 1, 1),
	EP("ep2", 2, 1024, 2, 1, 1),
	EP("ep3", 3, 1024, 3, 1, 0),
	EP("ep4", 4, 1024, 3, 1, 0),
	EP("ep5", 5, 1024, 3, 1, 1),
	EP("ep6", 6, 1024, 3, 1, 1),
};
#elif defined(CONFIG_SAMA5D2) || defined(CONFIG_SAMA5D3) || \
	defined(CONFIG_SAMA5D4)
static struct usba_ep_data usba_udc_ep[] = {
	EP("ep0", 0, 64, 1, 0, 0),
	EP("ep1", 1, 1024, 3, 1, 0),
	EP("ep2", 2, 1024, 3, 1, 0),
	EP("ep3", 3, 1024, 2, 1, 0),
	EP("ep4", 4, 1024, 2, 1, 0),
	EP("ep5", 5, 1024, 2, 1, 0),
	EP("ep6", 6, 1024, 2, 1, 0),
	EP("ep7", 7, 1024, 2, 1, 0),
	EP("ep8", 8, 1024, 2, 0, 0),
	EP("ep9", 9, 1024, 2, 0, 0),
	EP("ep10", 10, 1024, 2, 0, 0),
	EP("ep11", 11, 1024, 2, 0, 0),
	EP("ep12", 12, 1024, 2, 0, 0),
	EP("ep13", 13, 1024, 2, 0, 0),
	EP("ep14", 14, 1024, 2, 0, 0),
	EP("ep15", 15, 1024, 2, 0, 0),
};
#else
# error "NO usba_udc_ep defined"
#endif

#undef EP

struct usba_platform_data pdata = {
	.num_ep	= ARRAY_SIZE(usba_udc_ep),
	.ep	= usba_udc_ep,
};

#endif
