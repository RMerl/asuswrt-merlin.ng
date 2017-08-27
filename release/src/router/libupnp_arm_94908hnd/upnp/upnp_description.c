/*
 * Broadcom UPnP library XML description protocol
 *
 * Copyright (C) 2015, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: upnp_description.c 569059 2015-07-07 04:47:51Z $
 */

#include <upnp.h>

/*
 * Change PresentationURL to specific interface IP address.
 */
static int
set_url(UPNP_CONTEXT *context, UPNP_DESCRIPTION *descr, char *data_buf, int *data_len)
{
	UPNP_INTERFACE	*ifp = context->focus_ifp;
	char *root_device_xml = ifp->device->root_device_xml;
	char *name = descr->name;

	/*
	 * Check if this is the root xml.
	 * If yes, replace the <presentationURL>,
	 * else return the data buffer length.
	 */
	if (strcmp(root_device_xml, name+1) != 0) {
		*data_len = strlen(data_buf);
		return 0;
	}

	/* Change the root device UUID dynamically. */
	upnp_device_renew_rootxml(context, data_buf);

	*data_len = strlen(data_buf);
	return 0;
}

/* Send description XML file */
static int
description_send(UPNP_CONTEXT *context, UPNP_DESCRIPTION *descr)
{
	char *p;
	int len;
	char *data_buf = descr->data;
	int data_len = descr->len;

	/* Binary type use data length directly */
	if (strcmp(descr->mime_type, "text/xml") == 0)
		set_url(context, descr, data_buf, &data_len);

	/* Send head out */
	len = snprintf(context->head_buffer, sizeof(context->head_buffer),
			"HTTP/1.1 200 OK\r\n"
			"Content-Type: %s\r\n"
			"Content-Length: %d\r\n"
			"Connection: close\r\n"
			"Pragma: no-cache\r\n"
			"\r\n",
			descr->mime_type,
			data_len);

	if (send(context->fd, context->head_buffer, len, 0) == -1) {
		upnp_syslog(LOG_ERR,
			"description_process() send failed! fd=%d, buf=%s\n",
			context->fd, context->head_buffer);
		return R_ERROR;
	}

	p = data_buf;

	while (data_len) {
		len = (data_len > UPNP_DESC_MAXLEN) ? UPNP_DESC_MAXLEN : data_len;
		if (send(context->fd, p, len, 0) == -1) {
			upnp_syslog(LOG_ERR, "description_send() failed");
			return -1;
		}

		p += len;
		data_len -= len;
	}

	return 0;
}

/* Description lookup and sending routine */
int
description_process(UPNP_CONTEXT *context)
{
	UPNP_DESCRIPTION *descr = 0;

	/* search the table for target url */
	for (descr = context->focus_ifp->device->description_table;
		descr && descr->data;
		descr++) {
		/* Matched, set the focus device chain */
		if (strcmp(context->url, descr->name) == 0) {
			goto find;
		}
	}

find:
	if (!descr || !descr->data) {
		return R_NOT_FOUND;
	}

	/* send description XML body */
	description_send(context, descr);
	return 0;
}
