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

#define LWS_CPYAPP(ptr, str) { strcpy(ptr, str); ptr += strlen(str); }
#ifndef LWS_NO_EXTENSIONS
LWS_VISIBLE int
lws_extension_server_handshake(struct lws *wsi, char **p)
{
	int n;
	char *c;
	char ext_name[128];
	const struct lws_extension *ext;
	struct lws_context *context = wsi->context;
	int ext_count = 0;
	int more = 1;

	/*
	 * Figure out which extensions the client has that we want to
	 * enable on this connection, and give him back the list
	 */

	if (!lws_hdr_total_length(wsi, WSI_TOKEN_EXTENSIONS))
		return 0;

	/*
	 * break down the list of client extensions
	 * and go through them
	 */

	if (lws_hdr_copy(wsi, (char *)context->serv_buf,
			sizeof(context->serv_buf),
					      WSI_TOKEN_EXTENSIONS) < 0)
		return 1;

	c = (char *)context->serv_buf;
	lwsl_parser("WSI_TOKEN_EXTENSIONS = '%s'\n", c);
	wsi->count_active_extensions = 0;
	n = 0;
	while (more) {

		if (*c && (*c != ',' && *c != ' ' && *c != '\t')) {
			ext_name[n] = *c++;
			if (n < sizeof(ext_name) - 1)
				n++;
			continue;
		}
		ext_name[n] = '\0';
		if (!*c)
			more = 0;
		else {
			c++;
			if (!n)
				continue;
		}

		/* check a client's extension against our support */

		ext = lws_get_context(wsi)->extensions;

		while (ext && ext->callback) {

			if (strcmp(ext_name, ext->name)) {
				ext++;
				continue;
			}

			/*
			 * oh, we do support this one he asked for... but let's
			 * ask user code if it's OK to apply it on this
			 * particular connection + protocol
			 */

			n = lws_get_context(wsi)->protocols[0].callback(wsi,
				LWS_CALLBACK_CONFIRM_EXTENSION_OKAY,
				wsi->user_space, ext_name, 0);

			/*
			 * zero return from callback means go ahead and allow
			 * the extension, it's what we get if the callback is
			 * unhandled
			 */

			if (n) {
				ext++;
				continue;
			}

			/* apply it */

			if (ext_count)
				*(*p)++ = ',';
			else
				LWS_CPYAPP(*p,
				 "\x0d\x0aSec-WebSocket-Extensions: ");
			*p += sprintf(*p, "%s", ext_name);
			ext_count++;

			/* instantiate the extension on this conn */

			wsi->active_extensions_user[
				wsi->count_active_extensions] =
				     lws_zalloc(ext->per_session_data_size);
			if (wsi->active_extensions_user[
			     wsi->count_active_extensions] == NULL) {
				lwsl_err("Out of mem\n");
				return 1;
			}

			wsi->active_extensions[
				  wsi->count_active_extensions] = ext;

			/* allow him to construct his context */

			ext->callback(lws_get_context(wsi),
					ext, wsi,
					LWS_EXT_CALLBACK_CONSTRUCT,
					wsi->active_extensions_user[
				wsi->count_active_extensions], NULL, 0);

			wsi->count_active_extensions++;
			lwsl_parser("count_active_extensions <- %d\n",
					  wsi->count_active_extensions);

			ext++;
		}

		n = 0;
	}

	return 0;
}
#endif
int
handshake_0405(struct lws_context *context, struct lws *wsi)
{
	unsigned char hash[20];
	int n;
	char *response;
	char *p;
	int accept_len;

	if (!lws_hdr_total_length(wsi, WSI_TOKEN_HOST) ||
				!lws_hdr_total_length(wsi, WSI_TOKEN_KEY)) {
		lwsl_parser("handshake_04 missing pieces\n");
		/* completed header processing, but missing some bits */
		goto bail;
	}

	if (lws_hdr_total_length(wsi, WSI_TOKEN_KEY) >=
						     MAX_WEBSOCKET_04_KEY_LEN) {
		lwsl_warn("Client key too long %d\n", MAX_WEBSOCKET_04_KEY_LEN);
		goto bail;
	}

	/*
	 * since key length is restricted above (currently 128), cannot
	 * overflow
	 */
	n = sprintf((char *)context->serv_buf,
				"%s258EAFA5-E914-47DA-95CA-C5AB0DC85B11",
				lws_hdr_simple_ptr(wsi, WSI_TOKEN_KEY));

	lws_SHA1(context->serv_buf, n, hash);

	accept_len = lws_b64_encode_string((char *)hash, 20,
			(char *)context->serv_buf,
			sizeof(context->serv_buf));
	if (accept_len < 0) {
		lwsl_warn("Base64 encoded hash too long\n");
		goto bail;
	}

	/* allocate the per-connection user memory (if any) */
	if (lws_ensure_user_space(wsi))
		goto bail;

	/* create the response packet */

	/* make a buffer big enough for everything */

	response = (char *)context->serv_buf + MAX_WEBSOCKET_04_KEY_LEN + LWS_SEND_BUFFER_PRE_PADDING;
	p = response;
	LWS_CPYAPP(p, "HTTP/1.1 101 Switching Protocols\x0d\x0a"
		      "Upgrade: WebSocket\x0d\x0a"
		      "Connection: Upgrade\x0d\x0a"
		      "Sec-WebSocket-Accept: ");
	strcpy(p, (char *)context->serv_buf);
	p += accept_len;

	if (lws_hdr_total_length(wsi, WSI_TOKEN_PROTOCOL)) {
		LWS_CPYAPP(p, "\x0d\x0aSec-WebSocket-Protocol: ");
		n = lws_hdr_copy(wsi, p, 128, WSI_TOKEN_PROTOCOL);
		if (n < 0)
			goto bail;
		p += n;
	}

#ifndef LWS_NO_EXTENSIONS
	/*
	 * Figure out which extensions the client has that we want to
	 * enable on this connection, and give him back the list
	 */
	if (lws_extension_server_handshake(wsi, &p))
		goto bail;
#endif

	//LWS_CPYAPP(p, "\x0d\x0a""An-unknown-header: blah");

	/* end of response packet */

	LWS_CPYAPP(p, "\x0d\x0a\x0d\x0a");

	if (!lws_any_extension_handled(wsi, LWS_EXT_CALLBACK_HANDSHAKE_REPLY_TX,
				       response, p - response)) {

		/* okay send the handshake response accepting the connection */

		lwsl_parser("issuing resp pkt %d len\n", (int)(p - response));
#ifdef DEBUG
		fwrite(response, 1,  p - response, stderr);
#endif
		n = lws_write(wsi, (unsigned char *)response,
			      p - response, LWS_WRITE_HTTP_HEADERS);
		if (n != (p - response)) {
			lwsl_debug("handshake_0405: ERROR writing to socket\n");
			goto bail;
		}

	}

	/* alright clean up and set ourselves into established state */

	wsi->state = LWSS_ESTABLISHED;
	wsi->lws_rx_parse_state = LWS_RXPS_NEW;

	/* notify user code that we're ready to roll */

	if (wsi->protocol->callback)
		if (wsi->protocol->callback(wsi, LWS_CALLBACK_ESTABLISHED,
					    wsi->user_space,
#ifdef LWS_OPENSSL_SUPPORT
					    wsi->ssl,
#else
					    NULL,
#endif
					    0))
			goto bail;

	return 0;


bail:
	/* caller will free up his parsing allocations */
	return -1;
}

