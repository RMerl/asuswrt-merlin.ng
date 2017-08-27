/*
 * Broadcom UPnP library device specific include file
 *
 * Copyright (C) 2015, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: upnp_device.h 241182 2011-02-17 21:50:03Z $
 */

#ifndef __LIBUPNP_DEVICE_H__
#define __LIBUPNP_DEVICE_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

int  upnp_device_attach(UPNP_CONTEXT *context, UPNP_DEVICE *device);
void upnp_device_detach(UPNP_CONTEXT *context, UPNP_DEVICE *device);
int  upnp_device_renew_rootxml(UPNP_CONTEXT *context, char *databuf);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __LIBUPNP_H__ */
