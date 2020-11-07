/*
 * Broadcom UPnP library HTTP protocol
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
 * $Id: upnp_http.c 569048 2015-07-07 02:40:50Z $
 */

#include <upnp.h>

/* Functions */
static int upnp_http_fsm_init(UPNP_CONTEXT *);
static int upnp_http_parse_method(UPNP_CONTEXT *);
static int upnp_http_parse_uri(UPNP_CONTEXT *);
static int upnp_http_read_body(UPNP_CONTEXT *);
static int upnp_http_dispatch(UPNP_CONTEXT *);
static int upnp_http_fsm_deinit(UPNP_CONTEXT *);

/*
 * method parsing lookup table
 */
struct upnp_method {
	char *str;
	int method;
	int (*dispatch)(UPNP_CONTEXT *);
};

struct upnp_method upnp_http_methods[] =
{
	{"GET",         METHOD_GET,             description_process},
	{"POST",        METHOD_POST,            soap_process},
	{"SUBSCRIBE",   METHOD_SUBSCRIBE,       gena_process},
	{"UNSUBSCRIBE", METHOD_UNSUBSCRIBE,     gena_process},
	{0, 0, 0}
};

static struct upnp_state upnp_http_fsm[] =
{
	{upnp_http_fsm_init},
	{upnp_msg_head_read},
	{upnp_http_parse_method},
	{upnp_http_parse_uri},
	{upnp_msg_parse},
	{upnp_http_read_body},
	{upnp_http_dispatch},
	{upnp_http_fsm_deinit},
	{0}
};

/* Send error reply messages to clients */
static int
send_error_reply(UPNP_CONTEXT *req)
{
	int len;
	char *buf = req->head_buffer;
	char *err_msg;

	/* get relevant error message */
	switch (req->status) {
	case R_BAD_REQUEST:
		err_msg = "400 Bad Request";
		break;
	case R_FORBIDDEN:
		err_msg = "403 Forbidden";
		break;
	case R_PRECONDITION_FAIL:
		err_msg = "412 Precondition Fail";
		break;
	case R_METHOD_NA:
		err_msg = "405 Method Not Allowed";
		break;
	case R_NONE_ACC:
		err_msg = "406 Not Acceptable";
		break;
	case R_NOT_FOUND:
		err_msg = "404 Not Found";
		break;
	case R_ERROR:
		err_msg = "500 Internal Server Error";
		break;
	default:
		return 0;
	}

	/* Generate header */
	len = snprintf(buf, sizeof(req->head_buffer), "HTTP1.1 %s\r\n", err_msg);

	/*
	 * Only POST need to send Content-Type and title
	 */
	if (req->method == METHOD_POST ||
	    req->method == METHOD_MPOST) {
		if (len >= sizeof(req->head_buffer))
			return -1;

		len += snprintf(buf+len, sizeof(req->head_buffer) - len,
			"Content-Type: text/xml\r\n\r\n"
			"<title>%s</title>"
			"<body>%s</body>\r\n",
			err_msg,
			err_msg);
	}

	if (send(req->fd, buf, len, 0) == -1)
		return (-1);

	return (0);
}

/* Read body data from socket */
static int
upnp_http_read_body(UPNP_CONTEXT *context)
{
	char *content_len_p;
	int len;
	int diff;
	int n;

	/* NULL ended the body */
	context->content[context->content_len] = '\0';

	/* FIXME: is there any change CONTENT-LENGTH is not set,
	 * but with content?  Post action?
	 */
	content_len_p = upnp_msg_get(context, "CONTENT-LENGTH");
	if (content_len_p == 0) {
		/* body could be empty */
		return 0;
	}
	else {
		len = atoi(content_len_p);
		if (len == 0) {
			return 0;
		}
		else if (len < 0) {
			context->status = R_BAD_REQUEST;
			return -1;
		}
		else if (context->content + len >=
			context->buf + sizeof(context->buf)) {
			context->status = R_ERROR;
			return -1;
		}
	}

	/* Receive remainder */
	while ((diff = len - context->content_len) > 0) {
		n = upnp_fd_read(context->fd,
			&context->content[context->content_len], diff, 0);
		if (n <= 0) {
			context->status = R_ERROR;
			return -2;
		}

		context->content_len += n;
	}

	/* NULL ended the body */
	context->content[context->content_len] = '\0';
	return 0;
}

/* Parse the URI */
static int
upnp_http_parse_uri(UPNP_CONTEXT *context)
{
	int len;
	int gap;
	char *p = context->url;

	/* Locate URI */
	len = strcspn(p, " \t");
	gap = strspn(p + len, " \t");
	p[len] = 0;
	if (context->baseurl_postfix != NULL) {
		if (strncmp(context->baseurl_postfix, p+1, UPNP_URL_UUID_LEN)) {
			context->status = R_NOT_FOUND;
			return -1;
		}
		context->url = p + UPNP_URL_UUID_LEN + 1;
	} else	{
		context->url = p;
	}
	/* skip URL and white spaces */
	p += len + gap;

	/* Check HTTP version */
	if (strcmp(p, "HTTP/1.0") != 0 &&
	    strcmp(p, "HTTP/1.1") != 0) {
		return -1;
	}

	return 0;
}

/*
 * Parse the http method,
 * the followings are allowed,
 * (1) GET
 * (2) POST
 * (3) M-POST
 * (4) SUBSCRIBE
 * (5) UNSUBSCRIBE
 */
static int
upnp_http_parse_method(UPNP_CONTEXT *context)
{
	int len;
	int gap;
	char *p = upnp_msg_tok(context);
	int i;

	/* Locate method */
	len = strcspn(p, " \t");
	gap = strspn(p + len, " \t");
	p[len] = 0;

	/* Find method */
	for (i = 0; upnp_http_methods[i].str; i++) {
		/* For a matched method, save the information */
		if (strcmp(p, upnp_http_methods[i].str) == 0) {
			context->method_id = i;
			context->method = upnp_http_methods[i].method;
			context->url = p + len + gap;
			break;
		}

	}
	if (upnp_http_methods[i].str == 0) {
		context->status = R_METHOD_NA;
		return -1;
	}

	return 0;
}

/* Invoke method handling function */
static int
upnp_http_dispatch(UPNP_CONTEXT *context)
{
	struct upnp_method *methods = upnp_http_methods;

	context->status = (*methods[context->method_id].dispatch)(context);
	if (context->status != 0)
		return -1;

	return 0;
}

/* Initialize the upnp_http context */
int
upnp_http_fsm_init(UPNP_CONTEXT *context)
{
	context->status = 0;
	context->err_msg[0] = 0;
	context->index  = 0;
	context->end    = 0;

	upnp_msg_init(context);
	return 0;
}

static int
upnp_http_fsm_deinit(UPNP_CONTEXT *context)
{
	upnp_msg_deinit(context);
	return 0;
}

/* Process client (control point) requests */
void
upnp_http_process(UPNP_CONTEXT *context)
{
	struct sockaddr_in addr;
	socklen_t addr_len;
	int s;
	int i;
	int ret = 0;

	/* accept new upnp_http socket */
	addr_len = sizeof(struct sockaddr_in);
	s = accept(context->focus_ifp->http_sock, (struct sockaddr *)&addr, &addr_len);
	if (s == -1) {
		upnp_syslog(LOG_ERR, "accept failed");
		return;
	}

	context->dst_addr = addr;
	context->fd = s;

	/* Do state machine */
	for (i = 0; upnp_http_fsm[i].func; i++) {
		ret = (*upnp_http_fsm[i].func)(context);
		if (ret < 0) {
			upnp_http_fsm_deinit(context);
			break;
		}
	}

	/* Send error reply if neccessary */
	if (ret == -1)
		send_error_reply(context);

	close(s);
	return;
}

/* Close the upnp_http socket */
void
upnp_http_shutdown(UPNP_CONTEXT *context)
{
	UPNP_INTERFACE	*ifp = context->focus_ifp;

	if (ifp->http_sock != -1) {
		close(ifp->http_sock);
		ifp->http_sock = -1;
	}

	return;
}

/* Open the upnp_http socket */
int
upnp_http_init(UPNP_CONTEXT *context)
{
	UPNP_INTERFACE	*ifp = context->focus_ifp;

	ifp->http_sock = upnp_open_tcp_socket(ifp->ipaddr, context->http_port);
	if (ifp->http_sock == -1)
		return -1;

	return 0;
}
