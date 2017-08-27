/*
 * Broadcom UPnP library XML description protocol include file
 *
 * Copyright (C) 2015, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: upnp_description.h 241182 2011-02-17 21:50:03Z $
 */

#ifndef __LIBUPNP_DESCRIPTION_H__
#define __LIBUPNP_DESCRIPTION_H__

#ifdef __cplusplus
extern "C" {
#endif	/* __cplusplus */

/* Constants */
#define UPNP_DESC_MAXLEN 2048    /* maximum length per send */

/*
 * Functions
 */
int description_process(UPNP_CONTEXT *context);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __LIBUPNP_DESCRIPTION_H__ */
