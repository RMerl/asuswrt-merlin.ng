/*
 * Broadcom UPnP library HTTP protocol include file
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
 *
 * <<Broadcom-WL-IPTag/Proprietary:>>
 *
 * $Id: upnp_http.h 520342 2014-12-11 05:39:44Z $
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
#endif // endif

#endif /* __LIBUPNP_HTTP_H__ */
