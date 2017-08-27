/*
 * Broadcom UPnP library SSDP include file
 *
 * Copyright (C) 2015, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: upnp_ssdp.h 241182 2011-02-17 21:50:03Z $
 */

#ifndef __LIBUPNP_SSDP_H__
#define __LIBUPNP_SSDP_H__

#ifdef __cplusplus
extern "C" {
#endif	/* __cplusplus */

/*
 * Constants & Definitions
 */
#define SSDP_ADVTIME    1800
#define SSDP_ADDR       "239.255.255.250"
#define SSDP_PORT       1900
#define SSDP_MAXLEN     2048    /* SSDP response and advertise buffer size */

enum SSDP_TYPE_E {
	SSDP_BYEBYE,
	SSDP_ALIVE
};

enum MSEARCH_TYPE_E {
	MSEARCH_NONE,
	MSEARCH_ALL,
	MSEARCH_ROOTDEVICE,
	MSEARCH_UUID,
	MSEARCH_SERVICE,
	MSEARCH_DEVICE,
	MSEARCH_WFA_SERVICE,
	MSEARCH_WFA_DEVICE,
	MSEARCH_OTHER
};

enum ADVERTISE_TYPE_E {
	ADVERTISE_DEVICE,
	ADVERTISE_SERVICE,
	ADVERTISE_UUID,
	ADVERTISE_ROOTDEVICE
};

/*
 * Functions
 */
void ssdp_process(UPNP_CONTEXT *);
void ssdp_timeout(UPNP_CONTEXT *);
void ssdp_shutdown(UPNP_CONTEXT *);
int ssdp_init(UPNP_CONTEXT *);
int ssdp_add_multi(UPNP_CONTEXT *);
void ssdp_del_multi(UPNP_CONTEXT *);
void ssdp_alive(UPNP_CONTEXT *);
void ssdp_byebye(UPNP_CONTEXT *);

#ifdef __cplusplus
}
#endif

#endif /* __LIBUPNP_SSDP_H__ */
