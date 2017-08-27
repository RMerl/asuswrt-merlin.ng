/*
 * Broadcom UPnP library SOAP include file
 *
 * Copyright (C) 2015, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: upnp_soap.h 241182 2011-02-17 21:50:03Z $
 */

#ifndef __LIBUPNP_SOAP_H__
#define __LIBUPNP_SOAP_H__

#ifdef __cplusplus
extern "C" {
#endif	/* __cplusplus */

#include <upnp_type.h>

#define SOAP_MAX_ERRMSG		256
#define SOAP_MAX_BUF		2048

enum SOAP_ERROR_E {
	SOAP_INVALID_ACTION = 401,
	SOAP_INVALID_ARGS,
	SOAP_ACTION_FAILED = 501,
	SOAP_ARGUMENT_VALUE_INVALID = 600,
	SOAP_ARGUMENT_VALUE_OUT_OF_RANGE,
	SOAP_OPTIONAL_ACTION_NOT_IMPLEMENTED,
	SOAP_OUT_OF_MEMORY,
	SOAP_HUMAN_INTERVENTION_REQUIRED,
	SOAP_STRING_ARGUMENT_TOO_LONG,
	SOAP_ACTION_NOT_AUTHORIZED,
	SOAP_SIGNATURE_FAILURE,
	SOAP_SIGNATURE_MISSING,
	SOAP_NOT_ENCRYPTED,
	SOAP_INVALID_SEQUENCE,
	SOAP_INVALID_CONTROL_URL,
	SOAP_NO_SUCH_SESSION
};

/*
 * Functions
 */
int soap_process(UPNP_CONTEXT *context);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __LIBUPNP_SOAP_H__ */
