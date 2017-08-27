/*
 * libwebsockets - small server side websockets and web server implementation
 *
 * Copyright (C) 2010-2014 Andy Green <andy@warmcat.com>
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
 *
 * Some or all of this file is based on code from nghttp2, which has the
 * following license.  Since it's more liberal than lws license, you're also
 * at liberty to get the original code from
 * https://github.com/tatsuhiro-t/nghttp2 under his liberal terms alone.
 *
 * nghttp2 - HTTP/2.0 C Library
 *
 * Copyright (c) 2012 Tatsuhiro Tsujikawa
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "private-libwebsockets.h"

#ifndef LWS_NO_SERVER
#ifdef LWS_OPENSSL_SUPPORT

#if OPENSSL_VERSION_NUMBER >= 0x10002000L

struct alpn_ctx {
	unsigned char *data;
	unsigned short len;
};

static int
npn_cb(SSL *s, const unsigned char **data, unsigned int *len, void *arg)
{
	struct alpn_ctx *alpn_ctx = arg;

	lwsl_info("%s\n", __func__);
	*data = alpn_ctx->data;
	*len = alpn_ctx->len;

	return SSL_TLSEXT_ERR_OK;
}

static int
alpn_cb(SSL *s, const unsigned char **out, unsigned char *outlen,
	const unsigned char *in, unsigned int inlen, void *arg)
{
	struct alpn_ctx *alpn_ctx = arg;

	if (SSL_select_next_proto((unsigned char **)out, outlen, alpn_ctx->data,
				  alpn_ctx->len, in, inlen) !=
	    OPENSSL_NPN_NEGOTIATED)
		return SSL_TLSEXT_ERR_NOACK;

	return SSL_TLSEXT_ERR_OK;
}
#endif

LWS_VISIBLE void
lws_context_init_http2_ssl(struct lws_context *context)
{
#if OPENSSL_VERSION_NUMBER >= 0x10002000L
	static struct alpn_ctx protos = { (unsigned char *)"\x05h2-14"
					  "\x08http/1.1", 6 + 9 };

	SSL_CTX_set_next_protos_advertised_cb(context->ssl_ctx, npn_cb, &protos);

	// ALPN selection callback
	SSL_CTX_set_alpn_select_cb(context->ssl_ctx, alpn_cb, &protos);
	lwsl_notice(" HTTP2 / ALPN enabled\n");
#else
	lwsl_notice(
		" HTTP2 / ALPN configured but not supported by OpenSSL 0x%x\n",
		    OPENSSL_VERSION_NUMBER);
#endif // OPENSSL_VERSION_NUMBER >= 0x10002000L
}

void lws_http2_configure_if_upgraded(struct lws *wsi)
{
#if OPENSSL_VERSION_NUMBER >= 0x10002000L
	struct allocated_headers *ah;
	const char *method = "alpn";
	const unsigned char *name;
	unsigned len;

	SSL_get0_alpn_selected(wsi->ssl, &name, &len);

	if (!len) {
		SSL_get0_next_proto_negotiated(wsi->ssl, &name, &len);
		method = "npn";
	}

	if (!len) {
		lwsl_info("no npn/alpn upgrade\n");
		return;
	}

	lwsl_info("negotiated %s using %s\n", name, method);
	wsi->use_ssl = 1;
	if (strncmp((char *)name, "http/1.1", 8) == 0)
		return;

	/* http2 */

	/* adopt the header info */

	ah = wsi->u.hdr.ah;

	lws_union_transition(wsi, LWSCM_HTTP2_SERVING);
	wsi->state = LWSS_HTTP2_AWAIT_CLIENT_PREFACE;

	/* http2 union member has http union struct at start */
	wsi->u.http.ah = ah;

	lws_http2_init(&wsi->u.http2.peer_settings);
	lws_http2_init(&wsi->u.http2.my_settings);

	/* HTTP2 union */
#endif
}

#endif
#endif
