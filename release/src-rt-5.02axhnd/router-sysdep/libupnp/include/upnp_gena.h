/*
 * Broadcom UPnP library GENA include file
 *
 * Copyright 2019 Broadcom
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
 * $Id: upnp_gena.h 520342 2014-12-11 05:39:44Z $
 */
#ifndef  __LIBUPNP_GENA_H__
#define  __LIBUPNP_GENA_H__

#ifdef __cplusplus
extern "C" {
#endif	/* __cplusplus */

/* Constants */
#define GENA_TIMEOUT            30      /* GENA check interval */
#define GENA_SUBTIME            1800    /* default subscription time */
#define GENA_MAX_HEADER         512
#define GENA_MAX_BODY           4096
#define GENA_MAX_URL            256

/*
 * Functions
 */
UPNP_SCBRCHAIN *get_subscriber_chain(UPNP_CONTEXT *context, UPNP_SERVICE *service);
void delete_subscriber(UPNP_SCBRCHAIN *scbrchain, UPNP_SUBSCRIBER *subscriber);
UPNP_STATE_VAR *find_event_var(UPNP_CONTEXT *context, UPNP_SERVICE *service, char *name);
UPNP_EVENT *get_event(UPNP_CONTEXT *context, UPNP_STATE_VAR *state_var);
void gena_event_alarm(UPNP_CONTEXT *context, char *service_name,
	int num, char *headers[], char *ipaddr);

void gena_notify_complete(UPNP_CONTEXT *, UPNP_SERVICE *);
void gena_notify(UPNP_CONTEXT *, UPNP_SERVICE *, char *, char *);
int gena_process(UPNP_CONTEXT *);
void gena_timeout(UPNP_CONTEXT *);
int gena_init(UPNP_CONTEXT *);
int gena_shutdown(UPNP_CONTEXT *);

#ifdef __cplusplus
}
#endif // endif

#endif /* __LIBUPNP_GENA_H__ */
