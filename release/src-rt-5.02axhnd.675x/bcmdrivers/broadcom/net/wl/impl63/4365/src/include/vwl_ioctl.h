/*
 * NETVWL interface definitions
 * Broadcom 802.11abg Networking Device Driver
 *
 * Copyright 2020 Broadcom
 *
 * This program is the proprietary software of Broadcom and/or
 * its licensors, and may only be used, duplicated, modified or distributed
 * pursuant to the terms and conditions of a separate, written license
 * agreement executed between you and Broadcom (an "Authorized License").
 * Except as set forth in an Authorized License, Broadcom grants no license
 * (express or implied), right to use, or waiver of any kind with respect to
 * the Software, and Broadcom expressly reserves all rights in and to the
 * Software and all intellectual property rights therein.  IF YOU HAVE NO
 * AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY
 * WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF
 * THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use
 * all reasonable efforts to protect the confidentiality thereof, and to
 * use this information only in connection with your use of Broadcom
 * integrated circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES,
 * REPRESENTATIONS OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR
 * OTHERWISE, WITH RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY
 * DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY,
 * NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES,
 * ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR
 * CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING
 * OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL
 * BROADCOM OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL,
 * SPECIAL, INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR
 * IN ANY WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
 * IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii)
 * ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF
 * OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY
 * NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *
 * $Id: vwl_ioctl.h 708017 2017-06-29 14:11:45Z $
 */

#ifndef __vwl_ioctl_h__
#define __vwl_ioctl_h__

#ifndef DD_TYPE_NETVWL
#define DD_TYPE_NETVWL  FILE_DEVICE_BUS_EXTENDER
#endif // endif

#define IOCTL_NETVWL_BASE          0x100

#define	NETVWL_IOCTL_COOKIE	0x12349678

typedef struct {
	ULONG cookie;
	ULONG oid;
} netvwl_ioctl_hdr_t;

#define NETVWL_IOCTL_BCM_OID		CTL_CODE(FILE_DEVICE_BUS_EXTENDER, \
		IOCTL_NETVWL_BASE+0x30, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define NETVWL_IOCTL_DEVICE_CONNECT	CTL_CODE(FILE_DEVICE_BUS_EXTENDER, \
		IOCTL_NETVWL_BASE+0x31, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define NETVWL_IOCTL_DEVICE_DISCONNECT	CTL_CODE(FILE_DEVICE_BUS_EXTENDER, \
		IOCTL_NETVWL_BASE+0x32, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define NETVWL_IOCTL_DEVICE_SEND_UP	CTL_CODE(FILE_DEVICE_BUS_EXTENDER, \
		IOCTL_NETVWL_BASE+0x33, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define NETVWL_IOCTL_DEVICE_LINK	CTL_CODE(FILE_DEVICE_BUS_EXTENDER, \
		IOCTL_NETVWL_BASE+0x34, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define NETVWL_IOCTL_DEVICE_TX_COMPLETE	CTL_CODE(FILE_DEVICE_BUS_EXTENDER, \
		IOCTL_NETVWL_BASE+0x35, METHOD_BUFFERED, FILE_ANY_ACCESS)

#ifndef ETHER_ADDR_LEN
#define ETHER_ADDR_LEN	6
#endif // endif

typedef void (*vwl_ioctl_complete_cb_t)(void *handle, int status);

typedef int (*wl_tx_cb_t)(void *handle, void *txc_handle, void *buf, unsigned int size);
typedef int (*wl_ioctl_cb_t)(void *handle, int oid, void *buf, unsigned int size,
	vwl_ioctl_complete_cb_t fn);
typedef void (*wl_halt_cb_t)(void *handle);

typedef struct _wl_device_pars
{
	int version;  /* version of this type (currently 1) */
	void *handle; /* handle for the wl device (wl_info_t *) */
	int instance;
	wl_tx_cb_t tx_cb;
	wl_ioctl_cb_t  ioctl_cb;
	wl_halt_cb_t  halt_cb;
	UCHAR macaddr[ETHER_ADDR_LEN];
} wl_device_pars;

#endif /* __vwl_ioctl_h__ */
