/*
 * Broadcom UPnP library XML description protocol
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
 * $Id: upnp_description.c 569048 2015-07-07 02:40:50Z $
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
