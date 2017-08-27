/*
 * Broadcom UPnP library include file
 *
 * Copyright (C) 2015, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: upnp.h 569059 2015-07-07 04:47:51Z $
 */

#ifndef __LIBUPNP_H__
#define __LIBUPNP_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <upnp_osl.h>

#include <upnp_type.h>
#include <upnp_soap.h>
#include <upnp_gena.h>
#include <upnp_ssdp.h>
#include <upnp_http.h>
#include <upnp_description.h>
#include <upnp_device.h>
#include <upnp_util.h>
#include <upnp_msg.h>
#include <upnp_version.h>


#define UPNP_URL_UUID_LEN 36
#define TIME_BUF_LEN 64

/*
 * Functions
 */
UPNP_CONTEXT *upnp_init(int http_port, int adv_time, char *ifname_list[],
	UPNP_DEVICE *device, char *randomstring);
void upnp_deinit(UPNP_CONTEXT *context);
int upnp_fd_read(int s, char *buf, int len, int flags);
int upnp_fdset(UPNP_CONTEXT *context, fd_set *fds);
int upnp_fd_isset(UPNP_CONTEXT *context, fd_set *fds);
void upnp_dispatch(UPNP_CONTEXT *context, fd_set *fds);
void upnp_timeout(UPNP_CONTEXT *context);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __LIBUPNP_H__ */
