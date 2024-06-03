/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * PXA27x register declarations and HCD data structures
 *
 * Copyright (C) 2007 Rodolfo Giometti <giometti@linux.it>
 * Copyright (C) 2007 Eurotech S.p.A. <info@eurotech.it>
 */


#ifndef __PXA270X_UDC_H__
#define __PXA270X_UDC_H__

#include <asm/byteorder.h>

/* Endpoint 0 states */
#define EP0_IDLE		0
#define EP0_IN_DATA		1
#define EP0_OUT_DATA		2
#define EP0_XFER_COMPLETE	3


/* Endpoint parameters */
#define MAX_ENDPOINTS		4

#define EP0_MAX_PACKET_SIZE     16

#define UDC_OUT_ENDPOINT        0x02
#define UDC_IN_ENDPOINT         0x01
#define UDC_INT_ENDPOINT        0x05

#endif
