/*
 * Broadcom UPnP library utilities include file
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
 * $Id: upnp_util.h 738255 2017-12-27 22:36:47Z $
 */

#ifndef __LIBUPNP_UTIL_H__
#define __LIBUPNP_UTIL_H__

#ifdef __cplusplus
extern "C" {
#endif	/* __cplusplus */

/* uuid read from this file is used as random postfix for baseurl
 * /proc/sys/kernel/random/uuid file gives a random uuid to every read from the file
 */
#define UPNP_RANDOM_STRING_FILE  "/proc/sys/kernel/random/uuid"

/* Functions */
void upnp_tlv_init(UPNP_TLV *tlv, int type);
void upnp_tlv_deinit(UPNP_TLV *tlv);
char *upnp_tlv_translate(UPNP_TLV *tlv);
int upnp_tlv_convert(UPNP_TLV *tlv, char *text_str);
int upnp_tlv_set_bin(UPNP_TLV *tlv, uintptr_t data, int len);
int upnp_tlv_set(UPNP_TLV *tlv, uintptr_t data);

/* Search functions */
UPNP_SERVICE *upnp_get_service_by_control_url(UPNP_CONTEXT *context, char *control_url);
UPNP_SERVICE *upnp_get_service_by_event_url(UPNP_CONTEXT *context, char *event_url);
UPNP_SERVICE *upnp_get_service_by_name(UPNP_CONTEXT *context, char *name);
UPNP_ADVERTISE *upnp_get_advertise_by_name(UPNP_CONTEXT *context, char *name);

UPNP_TLV *upnp_get_in_tlv(UPNP_CONTEXT *context, char *arg_name);
UPNP_TLV *upnp_get_out_tlv(UPNP_CONTEXT *context, char *arg_name);

#define	UPNP_IN_TLV(a)	upnp_get_in_tlv(context, a)
#define	UPNP_OUT_TLV(a)	upnp_get_out_tlv(context, a)
#define	HINT(a...)

/* Support function */
int upnp_gmt_time(char *time_buf);
void upnp_host_addr(unsigned char *host_addr, struct in_addr ipaddr, unsigned short port);
static inline char *upnp_buffer(UPNP_CONTEXT *context) {return context->head_buffer;}

int upnp_get_url_randomstring(char *randomstring);
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __LIBUPNP_UTIL_H__ */
