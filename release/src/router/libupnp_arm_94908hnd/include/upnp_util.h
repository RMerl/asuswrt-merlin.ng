/*
 * Broadcom UPnP library utilities include file
 *
 * Copyright (C) 2015, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: upnp_util.h 551899 2015-04-24 11:55:46Z $
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
int upnp_tlv_set_bin(UPNP_TLV *tlv, int data, int len);
int upnp_tlv_set(UPNP_TLV *tlv, int data);

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
