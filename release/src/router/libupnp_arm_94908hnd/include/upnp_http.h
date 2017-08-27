/*
 * Broadcom UPnP library HTTP protocol include file
 *
 * Copyright (C) 2015, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: upnp_http.h 241182 2011-02-17 21:50:03Z $
 */

#ifndef __LIBUPNP_HTTP_H__
#define __LIBUPNP_HTTP_H__

#ifdef __cplusplus
extern "C" {
#endif	/* __cplusplus */

/* 
 * Constants
 */
#define UPNP_HTTP_DEFPORT           1980
#define MAX_WAITS                   10      /* http socket maximum connection queue */
#define MAX_FIELD_LEN	            256

/* request status */
#define R_SILENT                    999

#define R_REQUEST_OK                200
#define R_CREATED                   201
#define R_ACCEPTED                  202
#define R_NON_AUTHOR_INFORMATION    203
#define R_NO_CONTENT                204
#define R_RESET_CONTENT             205
#define R_PARTIAL_CONTENT           206

#define R_MULTIPLE                  300
#define R_MOVED_PERM                301
#define R_MOVED_FOUND               302
#define R_SEE_OTHER                 303
#define R_NOT_MODIFIED              304
#define R_USE_PROXY                 305
#define R_TEMPORARY_REDIRECT        307

#define R_BAD_REQUEST               400
#define R_UNAUTHORIZED              401
#define R_PAYMENT                   402
#define R_FORBIDDEN                 403
#define R_NOT_FOUND                 404
#define R_METHOD_NA                 405
#define R_NONE_ACC                  406
#define R_PROXY                     407
#define R_REQUEST_TO                408
#define R_CONFLICT                  409
#define R_GONE                      410
#define R_LENGTH_REQUIRED           411
#define R_PRECONDITION_FAIL         412
#define R_REQUEST_ENTITY_LARGE      413
#define R_REQUEST_URI_LONG          414
#define R_UNSUPPORTED_MEDIA_TYPE    415
#define R_REQUEST_RANGE_NOT_SATIS   416
#define R_EXPECTATION_FAIL          417

#define R_ERROR                     500
#define	R_NOT_IMP                   501
#define	R_BAD_GATEWAY               502
#define R_SERVICE_UNAV              503
#define	R_GATEWAY_TO                504
#define	R_HTTP_VERSION_NOT_SUPPORT  505

/* request methods */
enum UPNP_HTTP_REQUEST_METHOD_E
{
	METHOD_GET = 0,
	METHOD_SUBSCRIBE,
	METHOD_UNSUBSCRIBE,
	METHOD_POST,
	METHOD_MPOST,
	METHOD_MSEARCH
};

struct upnp_state {
	int (*func)(UPNP_CONTEXT *);
};

/*
 * Functions
 */
void upnp_http_process(UPNP_CONTEXT *);
void upnp_http_shutdown(UPNP_CONTEXT *);
int upnp_http_init(UPNP_CONTEXT *);

#ifdef __cplusplus
}
#endif

#endif /* __LIBUPNP_HTTP_H__ */
