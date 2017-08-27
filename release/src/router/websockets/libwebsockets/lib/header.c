/*
 * libwebsockets - small server side websockets and web server implementation
 *
 * Copyright (C) 2010-2013 Andy Green <andy@warmcat.com>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation:
 *  version 2.1 of the License.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 *  MA  02110-1301  USA
 */

#include "private-libwebsockets.h"
#include "lextable-strings.h"

const unsigned char *lws_token_to_string(enum lws_token_indexes token)
{
	if ((unsigned int)token >= ARRAY_SIZE(set))
		return NULL;

	return (unsigned char *)set[token];
}

int
lws_add_http_header_by_name(struct lws *wsi, const unsigned char *name,
			    const unsigned char *value, int length,
			    unsigned char **p, unsigned char *end)
{
#ifdef LWS_USE_HTTP2
	if (wsi->mode == LWSCM_HTTP2_SERVING)
		return lws_add_http2_header_by_name(wsi, name,
						    value, length, p, end);
#else
	(void)wsi;
#endif
	if (name) {
		while (*p < end && *name)
			*((*p)++) = *name++;
		if (*p == end)
			return 1;
		*((*p)++) = ' ';
	}
	if (*p + length + 3 >= end)
		return 1;

	memcpy(*p, value, length);
	*p += length;
	*((*p)++) = '\x0d';
	*((*p)++) = '\x0a';

	return 0;
}

int lws_finalize_http_header(struct lws *wsi, unsigned char **p,
			     unsigned char *end)
{
#ifdef LWS_USE_HTTP2
	if (wsi->mode == LWSCM_HTTP2_SERVING)
		return 0;
#else
	(void)wsi;
#endif
	if ((long)(end - *p) < 3)
		return 1;
	*((*p)++) = '\x0d';
	*((*p)++) = '\x0a';

	return 0;
}

int
lws_add_http_header_by_token(struct lws *wsi, enum lws_token_indexes token,
			     const unsigned char *value, int length,
			     unsigned char **p, unsigned char *end)
{
	const unsigned char *name;
#ifdef LWS_USE_HTTP2
	if (wsi->mode == LWSCM_HTTP2_SERVING)
		return lws_add_http2_header_by_token(wsi, token, value, length, p, end);
#endif
	name = lws_token_to_string(token);
	if (!name)
		return 1;
	return lws_add_http_header_by_name(wsi, name, value, length, p, end);
}

int lws_add_http_header_content_length(struct lws *wsi,
				       unsigned long content_length,
				       unsigned char **p, unsigned char *end)
{
	char b[24];
	int n;

	n = sprintf(b, "%lu", content_length);
	if (lws_add_http_header_by_token(wsi, WSI_TOKEN_HTTP_CONTENT_LENGTH,
					 (unsigned char *)b, n, p, end))
		return 1;
	wsi->u.http.content_length = content_length;
	wsi->u.http.content_remain = content_length;

	return 0;
}

static const char *err400[] = {
	"Bad Request",
	"Unauthorized",
	"Payment Required",
	"Forbidden",
	"Not Found",
	"Method Not Allowed",
	"Not Acceptable",
	"Proxy Auth Required",
	"Request Timeout",
	"Conflict",
	"Gone",
	"Length Required",
	"Precondition Failed",
	"Request Entity Too Large",
	"Request URI too Long",
	"Unsupported Media Type",
	"Requested Range Not Satisfiable",
	"Expectation Failed"
};

static const char *err500[] = {
	"Internal Server Error",
	"Not Implemented",
	"Bad Gateway",
	"Service Unavailable",
	"Gateway Timeout",
	"HTTP Version Not Supported"
};

int
lws_add_http_header_status(struct lws *wsi, unsigned int code,
			   unsigned char **p, unsigned char *end)
{
	unsigned char code_and_desc[60];
	const char *description = "";
	int n;

#ifdef LWS_USE_HTTP2
	if (wsi->mode == LWSCM_HTTP2_SERVING)
		return lws_add_http2_header_status(wsi, code, p, end);
#endif
	if (code >= 400 && code < (400 + ARRAY_SIZE(err400)))
		description = err400[code - 400];
	if (code >= 500 && code < (500 + ARRAY_SIZE(err500)))
		description = err500[code - 500];

	n = sprintf((char *)code_and_desc, "HTTP/1.0 %u %s", code, description);

	return lws_add_http_header_by_name(wsi, NULL, code_and_desc,
					   n, p, end);
}

/**
 * lws_return_http_status() - Return simple http status
 * @wsi:		Websocket instance (available from user callback)
 * @code:		Status index, eg, 404
 * @html_body:		User-readable HTML description < 1KB, or NULL
 *
 *	Helper to report HTTP errors back to the client cleanly and
 *	consistently
 */
LWS_VISIBLE int
lws_return_http_status(struct lws *wsi, unsigned int code, const char *html_body)
{
	int n, m;
	struct lws_context *context = lws_get_context(wsi);
	unsigned char *p = context->serv_buf +
			   LWS_SEND_BUFFER_PRE_PADDING;
	unsigned char *start = p;
	unsigned char *end = p + sizeof(context->serv_buf) -
			     LWS_SEND_BUFFER_PRE_PADDING;

	if (!html_body)
		html_body = "";

	if (lws_add_http_header_status(wsi, code, &p, end))
		return 1;
	if (lws_add_http_header_by_token(wsi, WSI_TOKEN_HTTP_SERVER,
					 (unsigned char *)"libwebsockets", 13,
					 &p, end))
		return 1;
	if (lws_add_http_header_by_token(wsi, WSI_TOKEN_HTTP_CONTENT_TYPE,
					 (unsigned char *)"text/html", 9,
					 &p, end))
		return 1;
	if (lws_finalize_http_header(wsi, &p, end))
		return 1;

	m = lws_write(wsi, start, p - start, LWS_WRITE_HTTP_HEADERS);
	if (m != (int)(p - start))
		return 1;

	n = sprintf((char *)start, "<html><body><h1>%u</h1>%s</body></html>",
		    code, html_body);
	m = lws_write(wsi, start, n, LWS_WRITE_HTTP);

	return m != n;
}
