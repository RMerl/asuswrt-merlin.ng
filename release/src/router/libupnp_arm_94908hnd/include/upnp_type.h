/*
 * Broadcom UPnP library type defintions
 *
 * Copyright (C) 2015, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: upnp_type.h 551899 2015-04-24 11:55:46Z $
 */

#ifndef __LIBUPNP_TYPE_H__
#define __LIBUPNP_TYPE_H__

#ifdef __cplusplus
extern "C" {
#endif	/* __cplusplus */

/*
 * Definitions
 */
#if !defined(OK) || !defined(ERROR)
#define OK     0
#define ERROR -1
#endif

#if !defined(TRUE) || !defined(FALSE)
#define TRUE   1
#define FALSE  0
#endif

/* Forward definition */
typedef struct upnp_tlv         UPNP_TLV;
typedef struct upnp_event       UPNP_EVENT;
typedef struct upnp_state_var   UPNP_STATE_VAR;
typedef struct action_argument  ACTION_ARGUMENT;
typedef struct upnp_action      UPNP_ACTION;
typedef struct upnp_service     UPNP_SERVICE;
typedef struct upnp_advertise   UPNP_ADVERTISE;
typedef struct upnp_description UPNP_DESCRIPTION;
typedef struct upnp_device      UPNP_DEVICE;
typedef struct upnp_subscriber  UPNP_SUBSCRIBER;
typedef struct upnp_scbrchain   UPNP_SCBRCHAIN;
typedef struct upnp_if          UPNP_INTERFACE;
typedef struct upnp_msg         UPNP_MSG;
typedef struct in_argument      IN_ARGUMENT;
typedef struct out_argument     OUT_ARGUMENT;
typedef struct upnp_context     UPNP_CONTEXT;

typedef int (*ACTION_FUNC)(UPNP_CONTEXT *, UPNP_SERVICE *, IN_ARGUMENT *, OUT_ARGUMENT *);
typedef int (*QUERY_FUNC)(UPNP_CONTEXT *, UPNP_SERVICE *, UPNP_TLV *);

/* UPNP type definition */
enum UPNP_DATA_TYPE_E {
	UPNP_TYPE_STR = 3,
	UPNP_TYPE_BOOL,
	UPNP_TYPE_UI1,
	UPNP_TYPE_UI2,
	UPNP_TYPE_UI4,
	UPNP_TYPE_I1,
	UPNP_TYPE_I2,
	UPNP_TYPE_I4,
	UPNP_TYPE_BIN_BASE64
};

struct upnp_tlv {
	int type;
	int len;
	union {
		long ival;
		unsigned long uval;
		char *str;		/* Same as value->text */
		char *bin;		/* Decoded bin data */
	} val;
	char buf[64];
	char *text;
	int text_strlen;
};

/* State variables */
struct upnp_event {
	UPNP_EVENT *next;
	UPNP_INTERFACE *ifp;

	int init;
	int notify;
	UPNP_TLV tlv;
};

struct upnp_state_var {
	UPNP_STATE_VAR *next;

	char *name;
	int type;
	QUERY_FUNC func;
	int eflag;
	UPNP_EVENT *event;
};

/* Action and arguments */
struct action_argument {
	char *name;
	int type;
	int related_id;
};

struct upnp_action {
	char *name;
	int in_num;
	ACTION_ARGUMENT *in_argument;
	int out_num;
	ACTION_ARGUMENT *out_argument;
	ACTION_FUNC action;
};

/* Service */
struct upnp_service {
	char *control_url;
	char *event_url;
	char *name;
	char *service_id;
	UPNP_ACTION *action_table;
	UPNP_STATE_VAR *statevar_table;

	int evented;
	UPNP_STATE_VAR *event_var_list;
	UPNP_SCBRCHAIN *scbrchain;
};

/* UPnP advertise */
struct upnp_advertise {
	char *name;
	char uuid[40];
	int type;
};

/* UPnP description */
struct upnp_description {
	char *name;
	char *mime_type;
	int len;
	char *data;
};

/* UPnP device */
#define	DEVICE_ATTACH_ALWAYS		0
#define	DEVICE_ATTACH_DYNAMICALLY	1

#define	DEVICE_NOTIFY_SUBSCRIBE		1
#define	DEVICE_NOTIFY_UNSUBSCRIBE	2
#define	DEVICE_NOTIFY_TIMEOUT		3

struct upnp_device {
	char *root_device_xml;
	UPNP_SERVICE *service_table;
	UPNP_ADVERTISE *advertise_table;
	UPNP_DESCRIPTION *description_table;
	int (*open)(UPNP_CONTEXT *);
	int (*close)(UPNP_CONTEXT *);
	int (*timeout)(UPNP_CONTEXT *, time_t);
	int (*notify)(UPNP_CONTEXT *, UPNP_SERVICE *, UPNP_SUBSCRIBER *, int);
	int (*scbrchk)(UPNP_CONTEXT *, UPNP_SERVICE *, UPNP_SUBSCRIBER *, struct in_addr,
	               unsigned short, char *);
	int gena_connect_retries;
	void *private;
};

/* UPNP GENA protocol */
#define UPNP_MAX_SID		48

struct upnp_subscriber {
	UPNP_SUBSCRIBER *next;
	UPNP_SUBSCRIBER *prev;

	struct in_addr ipaddr;
	unsigned short port;
	char *uri;
	char sid[UPNP_MAX_SID];
	unsigned int expire_time;
	unsigned int seq;
};

struct upnp_scbrchain {
	UPNP_SCBRCHAIN *next;

	UPNP_INTERFACE *ifp;
	UPNP_SUBSCRIBER *subscriberlist;
};

/* UPnP protocol suite */
/*
 * UPnP interface
 */
#define IFF_IPCHANGED		0x01    /* interface IP address changed */
#define IFF_MJOINED		0x02    /* SSDP multicast group joined */

struct upnp_if {
	UPNP_INTERFACE *next;	/* pointer to next if */

	char ifname[IFNAMSIZ];	/* interface name */
	int flag;		/* ip changed, multicast joined? */
	int http_sock;		/* upnp_http socket */
	struct in_addr ipaddr;
	struct in_addr netmask;
	char mac[6];

	UPNP_DEVICE *device;
	UPNP_ADVERTISE *advlist;
	void *devctrl;
};

/*
 * UPnP context
 */
/* UPNP SOAP protocol */
struct in_argument {
	char *name;
	UPNP_STATE_VAR *statevar;
	UPNP_TLV tlv;
};

/* Output argument defintions */
struct out_argument {
	char *name;
	UPNP_STATE_VAR *statevar;
	UPNP_TLV tlv;
};

struct upnp_msg {
	UPNP_MSG *next;
	char *name;
	char *value;
};

#define MAX_HEADER_LEN		4096
#define MAX_BUF_LEN		2048
#define UPNP_ERR_MSG_SIZE	128
#define SSDP_TRAINING_PERIOD	180
#define SSDP_TRAINING_TIME	30

struct upnp_context {
	UPNP_INTERFACE *iflist;
	UPNP_INTERFACE *focus_ifp;

	unsigned int http_port;
	unsigned int adv_time;
	unsigned int ssdp_trains;

	time_t adv_seconds;
	time_t gena_last_check;
	time_t upnp_last_time;

	int ssdp_sock;			/* Socket to recive ssdp multicast packets */
	int fd;

	/* client socket descriptor */
	int status;                     /* R_OK, R_ERROR, R_BAD_REQUEST... */
	char err_msg[UPNP_ERR_MSG_SIZE];

	/* Status */
	int method;                     /* M_GET, M_POST, ... */
	int method_id;                  /* method index */

	char buf[MAX_HEADER_LEN * 2];   /* upnp_http input buffer */
	int index;                      /* index to next unread char */
	int end;                        /* index to one char past last */

	char *url;
	UPNP_MSG *msglist;

	char *content;
	int  content_len;

	char head_buffer[MAX_HEADER_LEN];	/* output header buffer */
	char body_buffer[MAX_BUF_LEN];		/* outout body buffer */

	int in_arg_num;
	IN_ARGUMENT *in_args;

	int out_arg_num;
	OUT_ARGUMENT *out_args;
	char *baseurl_postfix;

	struct sockaddr_in dst_addr;
};

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __LIBUPNP_TYPE_H__ */
