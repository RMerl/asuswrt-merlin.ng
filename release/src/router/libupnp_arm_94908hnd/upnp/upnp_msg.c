/*
 * Broadcom UPnP library HTTP protocol
 *
 * Copyright (C) 2015, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: upnp_msg.c 241182 2011-02-17 21:50:03Z $
 */

#include <upnp.h>

char *
upnp_msg_get(UPNP_CONTEXT *context, char *name)
{
	UPNP_MSG *msg;

	for (msg = context->msglist; msg; msg = msg->next) {
		if (strcmp(name, msg->name) == 0)
			return msg->value;
	}

	return NULL;
}

int
upnp_msg_save(UPNP_CONTEXT *context, char *name, char *value)
{
	UPNP_MSG *msg;

	/* FIXEME: Duplicate name, value to other buffer
	 * seems safer.
	 */

	/* Find in list */
	for (msg = context->msglist; msg; msg = msg->next) {
		if (strcmp(name, msg->name) == 0) {
			msg->value = value;
			return 0;
		}
	}

	/* Create a new one */
	msg = (UPNP_MSG *)calloc(1, sizeof(*msg));
	if (msg == NULL) {
		upnp_syslog(LOG_ERR, "Out of memory!");
		return -1;
	}

	msg->name = name;
	msg->value = value;

	/* Do prepend */
	msg->next = context->msglist;
	context->msglist = msg;

	return 0;
}

void
upnp_msg_deinit(UPNP_CONTEXT *context)
{
	UPNP_MSG *msg;

	while ((msg = context->msglist) != NULL) {
		context->msglist = context->msglist->next;
		free(msg);
	}
}

int
upnp_msg_init(UPNP_CONTEXT *context)
{
	context->msglist = NULL;
	return 0;
}

char *
upnp_msg_tok(UPNP_CONTEXT *context)
{
	char *token;
	int len;
	int gap;

	token = context->buf + context->index;

	len = strcspn(token, "\r\n");
	gap = strspn(token + len, "\r\n");
	token[len] = 0;

	/* Advanced to next head messge */
	context->index += len + gap;

	/* Skip leading white space if any */
	token += strspn(token, " \t");
	return token;
}

/* Parse head and set appropriate env variables */
int
upnp_msg_parse(UPNP_CONTEXT *context)
{
	int len;
	int gap;
	char *token, *value;
	char *p;

	while (1) {
		token = upnp_msg_tok(context);
		if (*token == 0)
			break;

		/* Locate ' :\t' to seperate token and value */
		len = strcspn(token, ": \t");
		gap = strspn(token + len, ": \t");
		token[len] = 0;

		value = token + len + gap;

		/* capitalize token */
		for (p = token; *p; p++)
			*p = toupper(*p);

		if (upnp_msg_save(context, token, value) != 0) {
			context->status = R_ERROR;
			return -1;
		}
	}

	return 0;
}

/* Find end of the message header */
static int
upnp_msg_head_check(UPNP_CONTEXT *context)
{
	int i = context->index;
	int j;
	int end = context->end;
	int eoh = 0;
	int eol = 0;
	char c;

	/* start parsing */
	while (i < end) {
		/* end of line */
		c = context->buf[i++];
		if (c == '\n') {
			if (i < end &&
			    context->buf[i] != '\r' &&
			    context->buf[i] != '\n') {
				/*
				* PC style, the end of line is
				* \r\n
				*/
				if (i > 1 && context->buf[i-2] == '\r')
					eol = 2;  /* \r\n */
				else
					eol = 1;  /* \n */
			}
			else if (i < end && context->buf[i] == '\n') {
				/* end of headers: \n\n */
				eol = 1;
				eoh = 1;
			}
			else if (i > 1 && i+1 < end &&
				context->buf[i-2] == '\r' &&
				context->buf[i] == '\r' && context->buf[i+1] == '\n') {
				/* end of headers: \r\n\r\n */
				eol = 2;
				eoh = 2;
			}

			/* make header */
			if (eol)
				context->index = i;

			if (eoh) {
				/* Null end message head */
				for (j = 0; j < eoh; j++)
					context->buf[context->index + j] = 0;

				context->index += eoh;
				return 0;
			}
		}
	}

	return -1;
}

/* Read data from socket until end of head or error occurs */
int
upnp_msg_head_read(UPNP_CONTEXT *context)
{
	int buf_size;
	int n;

	while (upnp_msg_head_check(context)) {
		/* check buffer size */
		buf_size = sizeof(context->buf) - context->end;
		if (buf_size <= 0) {
			context->status = R_BAD_REQUEST;
			break;
		}

		/* read socket into the buffer */
		n = upnp_fd_read(context->fd, &context->buf[context->end], buf_size, 0);
		if (n <= 0) {
			context->status = R_ERROR;
			return -2;
		}

		/* Move end */
		context->end += n;
	}

	if (context->status != 0)
		return -1;

	/* Set content start */
	context->content = &context->buf[context->index];
	context->content_len = context->end - context->index;

	/* Set index to start for head processing */
	context->index = 0;
	return 0;
}
