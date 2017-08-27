/*
 * Broadcom UPnP library message include file
 *
 * Copyright (C) 2015, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: upnp_msg.h 241182 2011-02-17 21:50:03Z $
 */

#ifndef __LIBUPNP_MSG_H__
#define __LIBUPNP_MSG_H__

#ifdef __cplusplus
extern "C" {
#endif	/* __cplusplus */

/*
 * Functions
 */
char *upnp_msg_get(UPNP_CONTEXT *context, char *name);
int upnp_msg_save(UPNP_CONTEXT *context, char *name, char *value);
void upnp_msg_deinit(UPNP_CONTEXT *context);
int upnp_msg_init(UPNP_CONTEXT *context);

char *upnp_msg_tok(UPNP_CONTEXT *context);
int upnp_msg_parse(UPNP_CONTEXT *context);
int upnp_msg_head_read(UPNP_CONTEXT *context);

#ifdef __cplusplus
}
#endif

#endif /* __LIBUPNP_HTTP_H__ */
