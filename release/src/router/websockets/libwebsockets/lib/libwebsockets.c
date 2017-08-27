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
 */

#include "private-libwebsockets.h"

int log_level = LLL_ERR | LLL_WARN | LLL_NOTICE;
static void (*lwsl_emit)(int level, const char *line) = lwsl_emit_stderr;

static const char * const log_level_names[] = {
	"ERR",
	"WARN",
	"NOTICE",
	"INFO",
	"DEBUG",
	"PARSER",
	"HEADER",
	"EXTENSION",
	"CLIENT",
	"LATENCY",
};

void
lws_free_wsi(struct lws *wsi)
{
	if (!wsi)
		return;

	/* Protocol user data may be allocated either internally by lws
	 * or by specified the user.
	 * We should only free what we allocated. */
	if (wsi->protocol && wsi->protocol->per_session_data_size &&
	    wsi->user_space && !wsi->user_space_externally_allocated)
		lws_free(wsi->user_space);

	lws_free_set_NULL(wsi->rxflow_buffer);
	lws_free_set_NULL(wsi->trunc_alloc);
	lws_free_header_table(wsi);
	lws_free(wsi);
}

void
lws_close_free_wsi(struct lws *wsi, enum lws_close_status reason)
{
	struct lws_context *context = wsi->context;
	int n, m, ret, old_state;
	struct lws_tokens eff_buf;
	unsigned char buf[LWS_SEND_BUFFER_PRE_PADDING + 2 +
			  LWS_SEND_BUFFER_POST_PADDING];

	if (!wsi)
		return;

	old_state = wsi->state;

	if (wsi->mode == LWSCM_HTTP_SERVING_ACCEPTED &&
	    wsi->u.http.fd != LWS_INVALID_FILE) {
		lwsl_debug("closing http file\n");
		lws_plat_file_close(wsi, wsi->u.http.fd);
		wsi->u.http.fd = LWS_INVALID_FILE;
		context->protocols[0].callback(wsi, LWS_CALLBACK_CLOSED_HTTP,
					       wsi->user_space, NULL, 0);
	}
	if (wsi->socket_is_permanently_unusable ||
	    reason == LWS_CLOSE_STATUS_NOSTATUS_CONTEXT_DESTROY)
		goto just_kill_connection;

	switch (old_state) {
	case LWSS_DEAD_SOCKET:
		return;

	/* we tried the polite way... */
	case LWSS_AWAITING_CLOSE_ACK:
		goto just_kill_connection;

	case LWSS_FLUSHING_STORED_SEND_BEFORE_CLOSE:
		if (wsi->trunc_len) {
			lws_callback_on_writable(wsi);
			return;
		}
		lwsl_info("wsi %p completed LWSS_FLUSHING_STORED_SEND_BEFORE_CLOSE\n", wsi);
		goto just_kill_connection;
	default:
		if (wsi->trunc_len) {
			lwsl_info("wsi %p entering LWSS_FLUSHING_STORED_SEND_BEFORE_CLOSE\n", wsi);
			wsi->state = LWSS_FLUSHING_STORED_SEND_BEFORE_CLOSE;
			lws_set_timeout(wsi, PENDING_FLUSH_STORED_SEND_BEFORE_CLOSE, 5);
			return;
		}
		break;
	}

	wsi->u.ws.close_reason = reason;

	if (wsi->mode == LWSCM_WSCL_WAITING_CONNECT ||
	    wsi->mode == LWSCM_WSCL_ISSUE_HANDSHAKE)
		goto just_kill_connection;

	if (wsi->mode == LWSCM_HTTP_SERVING)
		context->protocols[0].callback(wsi, LWS_CALLBACK_CLOSED_HTTP,
					       wsi->user_space, NULL, 0);

	/*
	 * are his extensions okay with him closing?  Eg he might be a mux
	 * parent and just his ch1 aspect is closing?
	 */

	if (lws_ext_cb_wsi_active_exts(wsi,
		      LWS_EXT_CALLBACK_CHECK_OK_TO_REALLY_CLOSE, NULL, 0) > 0) {
		lwsl_ext("extension vetoed close\n");
		return;
	}

	/*
	 * flush any tx pending from extensions, since we may send close packet
	 * if there are problems with send, just nuke the connection
	 */

	do {
		ret = 0;
		eff_buf.token = NULL;
		eff_buf.token_len = 0;

		/* show every extension the new incoming data */

		m = lws_ext_cb_wsi_active_exts(wsi,
			  LWS_EXT_CALLBACK_FLUSH_PENDING_TX, &eff_buf, 0);
		if (m < 0) {
			lwsl_ext("Extension reports fatal error\n");
			goto just_kill_connection;
		}
		if (m)
			/*
			 * at least one extension told us he has more
			 * to spill, so we will go around again after
			 */
			ret = 1;

		/* assuming they left us something to send, send it */

		if (eff_buf.token_len)
			if (lws_issue_raw(wsi, (unsigned char *)eff_buf.token,
					  eff_buf.token_len) !=
			    eff_buf.token_len) {
				lwsl_debug("close: ext spill failed\n");
				goto just_kill_connection;
			}
	} while (ret);

	/*
	 * signal we are closing, lws_write will
	 * add any necessary version-specific stuff.  If the write fails,
	 * no worries we are closing anyway.  If we didn't initiate this
	 * close, then our state has been changed to
	 * LWSS_RETURNED_CLOSE_ALREADY and we will skip this.
	 *
	 * Likewise if it's a second call to close this connection after we
	 * sent the close indication to the peer already, we are in state
	 * LWSS_AWAITING_CLOSE_ACK and will skip doing this a second time.
	 */

	if (old_state == LWSS_ESTABLISHED &&
	    reason != LWS_CLOSE_STATUS_NOSTATUS &&
	    reason != LWS_CLOSE_STATUS_NOSTATUS_CONTEXT_DESTROY) {
		lwsl_debug("sending close indication...\n");
		/* make valgrind happy */
		memset(buf, 0, sizeof(buf));
		n = lws_write(wsi, &buf[LWS_SEND_BUFFER_PRE_PADDING + 2],
			      0, LWS_WRITE_CLOSE);
		if (n >= 0) {
			/*
			 * we have sent a nice protocol level indication we
			 * now wish to close, we should not send anything more
			 */
			wsi->state = LWSS_AWAITING_CLOSE_ACK;

			/*
			 * ...and we should wait for a reply for a bit
			 * out of politeness
			 */
			lws_set_timeout(wsi, PENDING_TIMEOUT_CLOSE_ACK, 1);
			lwsl_debug("sent close indication, awaiting ack\n");

			return;
		}

		lwsl_info("close: sending close packet failed, hanging up\n");

		/* else, the send failed and we should just hang up */
	}

just_kill_connection:

	lwsl_debug("close: just_kill_connection\n");

	/*
	 * we won't be servicing or receiving anything further from this guy
	 * delete socket from the internal poll list if still present
	 */
	lws_ssl_remove_wsi_from_buffered_list(wsi);

	/* checking return redundant since we anyway close */
	remove_wsi_socket_from_fds(wsi);

	wsi->state = LWSS_DEAD_SOCKET;

	lws_free_set_NULL(wsi->rxflow_buffer);
	lws_free_header_table(wsi);

	if (old_state == LWSS_ESTABLISHED ||
	    wsi->mode == LWSCM_WS_SERVING ||
	    wsi->mode == LWSCM_WS_CLIENT) {

		lws_free_set_NULL(wsi->u.ws.rx_user_buffer);

		if (wsi->trunc_alloc)
			/* not going to be completed... nuke it */
			lws_free_set_NULL(wsi->trunc_alloc);

		if (wsi->u.ws.ping_payload_buf) {
			lws_free_set_NULL(wsi->u.ws.ping_payload_buf);
			wsi->u.ws.ping_payload_alloc = 0;
			wsi->u.ws.ping_payload_len = 0;
			wsi->u.ws.ping_pending_flag = 0;
		}
	}

	/* tell the user it's all over for this guy */

	if (wsi->protocol && wsi->protocol->callback &&
	    ((old_state == LWSS_ESTABLISHED) ||
	    (old_state == LWSS_RETURNED_CLOSE_ALREADY) ||
	    (old_state == LWSS_AWAITING_CLOSE_ACK) ||
	    (old_state == LWSS_FLUSHING_STORED_SEND_BEFORE_CLOSE))) {
		lwsl_debug("calling back CLOSED\n");
		wsi->protocol->callback(wsi, LWS_CALLBACK_CLOSED,
					wsi->user_space, NULL, 0);
	} else if (wsi->mode == LWSCM_HTTP_SERVING_ACCEPTED) {
		lwsl_debug("calling back CLOSED_HTTP\n");
		context->protocols[0].callback(wsi, LWS_CALLBACK_CLOSED_HTTP,
					       wsi->user_space, NULL, 0 );
	} else if (wsi->mode == LWSCM_WSCL_WAITING_SERVER_REPLY ||
		   wsi->mode == LWSCM_WSCL_WAITING_CONNECT) {
		lwsl_debug("Connection closed before server reply\n");
		context->protocols[0].callback(wsi,
					LWS_CALLBACK_CLIENT_CONNECTION_ERROR,
					wsi->user_space, NULL, 0);
	} else
		lwsl_debug("not calling back closed mode=%d state=%d\n",
			   wsi->mode, old_state);

	/* deallocate any active extension contexts */

	if (lws_ext_cb_wsi_active_exts(wsi, LWS_EXT_CALLBACK_DESTROY, NULL, 0) < 0)
		lwsl_warn("extension destruction failed\n");
#ifndef LWS_NO_EXTENSIONS
	for (n = 0; n < wsi->count_active_extensions; n++)
		lws_free(wsi->active_extensions_user[n]);
#endif
	/*
	 * inform all extensions in case they tracked this guy out of band
	 * even though not active on him specifically
	 */
	if (lws_ext_cb_all_exts(context, wsi,
		       LWS_EXT_CALLBACK_DESTROY_ANY_WSI_CLOSING, NULL, 0) < 0)
		lwsl_warn("ext destroy wsi failed\n");

	if (!lws_ssl_close(wsi) && lws_socket_is_valid(wsi->sock)) {
#if LWS_POSIX
		n = shutdown(wsi->sock, SHUT_RDWR);
		if (n)
			lwsl_debug("closing: shutdown ret %d\n", LWS_ERRNO);

		n = compatible_close(wsi->sock);
		if (n)
			lwsl_debug("closing: close ret %d\n", LWS_ERRNO);

#else
		compatible_close(wsi->sock);
#endif
		wsi->sock = LWS_SOCK_INVALID;
	}

	/* outermost destroy notification for wsi (user_space still intact) */
	context->protocols[0].callback(wsi, LWS_CALLBACK_WSI_DESTROY,
				       wsi->user_space, NULL, 0);

	lws_free_wsi(wsi);
}

LWS_VISIBLE int
lws_get_addresses(struct lws_context *context, void *ads, char *name,
		  int name_len, char *rip, int rip_len)
{
#if LWS_POSIX
	struct addrinfo ai, *res;
	struct sockaddr_in addr4;

	if (rip)
		rip[0] = '\0';
	name[0] = '\0';
	addr4.sin_family = AF_UNSPEC;

#ifdef LWS_USE_IPV6
	if (LWS_IPV6_ENABLED(context)) {
		if (!lws_plat_inet_ntop(AF_INET6, &((struct sockaddr_in6 *)ads)->sin6_addr, rip, rip_len)) {
			lwsl_err("inet_ntop", strerror(LWS_ERRNO));
			return -1;
		}

		// Strip off the IPv4 to IPv6 header if one exists
		if (strncmp(rip, "::ffff:", 7) == 0)
			memmove(rip, rip + 7, strlen(rip) - 6);

		getnameinfo((struct sockaddr *)ads,
				sizeof(struct sockaddr_in6), name,
							name_len, NULL, 0, 0);

		return 0;
	} else
#endif
	{
		struct addrinfo *result;

		memset(&ai, 0, sizeof ai);
		ai.ai_family = PF_UNSPEC;
		ai.ai_socktype = SOCK_STREAM;
		ai.ai_flags = AI_CANONNAME;

		if (getnameinfo((struct sockaddr *)ads,
				sizeof(struct sockaddr_in),
				name, name_len, NULL, 0, 0))
			return -1;

		if (!rip)
			return 0;

		if (getaddrinfo(name, NULL, &ai, &result))
			return -1;

		res = result;
		while (addr4.sin_family == AF_UNSPEC && res) {
			switch (res->ai_family) {
			case AF_INET:
				addr4.sin_addr = ((struct sockaddr_in *)res->ai_addr)->sin_addr;
				addr4.sin_family = AF_INET;
				break;
			}

			res = res->ai_next;
		}
		freeaddrinfo(result);
	}

	if (addr4.sin_family == AF_UNSPEC)
		return -1;

	if (lws_plat_inet_ntop(AF_INET, &addr4.sin_addr, rip, rip_len) == NULL)
		return -1;

	return 0;
#else
	(void)context;
	(void)ads;
	(void)name;
	(void)name_len;
	(void)rip;
	(void)rip_len;

	return -1;
#endif
}

/**
 * lws_get_peer_addresses() - Get client address information
 * @wsi:	Local struct lws associated with
 * @fd:		Connection socket descriptor
 * @name:	Buffer to take client address name
 * @name_len:	Length of client address name buffer
 * @rip:	Buffer to take client address IP dotted quad
 * @rip_len:	Length of client address IP buffer
 *
 *	This function fills in @name and @rip with the name and IP of
 *	the client connected with socket descriptor @fd.  Names may be
 *	truncated if there is not enough room.  If either cannot be
 *	determined, they will be returned as valid zero-length strings.
 */

LWS_VISIBLE void
lws_get_peer_addresses(struct lws *wsi, lws_sockfd_type fd, char *name,
		       int name_len, char *rip, int rip_len)
{
#if LWS_POSIX
	socklen_t len;
#ifdef LWS_USE_IPV6
	struct sockaddr_in6 sin6;
#endif
	struct sockaddr_in sin4;
	struct lws_context *context = wsi->context;
	int ret = -1;
	void *p;

	rip[0] = '\0';
	name[0] = '\0';

	lws_latency_pre(context, wsi);

#ifdef LWS_USE_IPV6
	if (LWS_IPV6_ENABLED(context)) {
		len = sizeof(sin6);
		p = &sin6;
	} else
#endif
	{
		len = sizeof(sin4);
		p = &sin4;
	}

	if (getpeername(fd, p, &len) < 0) {
		lwsl_warn("getpeername: %s\n", strerror(LWS_ERRNO));
		goto bail;
	}

	ret = lws_get_addresses(context, p, name, name_len, rip, rip_len);

bail:
	lws_latency(context, wsi, "lws_get_peer_addresses", ret, 1);
#else
	(void)wsi;
	(void)fd;
	(void)name;
	(void)name_len;
	(void)rip;
	(void)rip_len;
#endif
}

/**
 * lws_context_user() - get the user data associated with the context
 * @context: Websocket context
 *
 *	This returns the optional user allocation that can be attached to
 *	the context the sockets live in at context_create time.  It's a way
 *	to let all sockets serviced in the same context share data without
 *	using globals statics in the user code.
 */
LWS_EXTERN void *
lws_context_user(struct lws_context *context)
{
	return context->user_space;
}


/**
 * lws_callback_all_protocol() - Callback all connections using
 *				the given protocol with the given reason
 *
 * @protocol:	Protocol whose connections will get callbacks
 * @reason:	Callback reason index
 */

LWS_VISIBLE int
lws_callback_all_protocol(struct lws_context *context,
			  const struct lws_protocols *protocol, int reason)
{
	struct lws *wsi;
	int n;

	for (n = 0; n < context->fds_count; n++) {
		wsi = wsi_from_fd(context, context->fds[n].fd);
		if (!wsi)
			continue;
		if (wsi->protocol == protocol)
			protocol->callback(wsi, reason, wsi->user_space, NULL, 0);
	}

	return 0;
}

/**
 * lws_set_timeout() - marks the wsi as subject to a timeout
 *
 * You will not need this unless you are doing something special
 *
 * @wsi:	Websocket connection instance
 * @reason:	timeout reason
 * @secs:	how many seconds
 */

LWS_VISIBLE void
lws_set_timeout(struct lws *wsi, enum pending_timeout reason, int secs)
{
	time_t now;

	time(&now);

	wsi->pending_timeout_limit = now + secs;
	wsi->pending_timeout = reason;
}


#if LWS_POSIX

/**
 * lws_get_socket_fd() - returns the socket file descriptor
 *
 * You will not need this unless you are doing something special
 *
 * @wsi:	Websocket connection instance
 */

LWS_VISIBLE int
lws_get_socket_fd(struct lws *wsi)
{
	return wsi->sock;
}

#endif

#ifdef LWS_LATENCY
void
lws_latency(struct lws_context *context, struct lws *wsi, const char *action,
	    int ret, int completed)
{
	unsigned long long u;
	char buf[256];

	u = time_in_microseconds();

	if (!action) {
		wsi->latency_start = u;
		if (!wsi->action_start)
			wsi->action_start = u;
		return;
	}
	if (completed) {
		if (wsi->action_start == wsi->latency_start)
			sprintf(buf,
			  "Completion first try lat %lluus: %p: ret %d: %s\n",
					u - wsi->latency_start,
						      (void *)wsi, ret, action);
		else
			sprintf(buf,
			  "Completion %lluus: lat %lluus: %p: ret %d: %s\n",
				u - wsi->action_start,
					u - wsi->latency_start,
						      (void *)wsi, ret, action);
		wsi->action_start = 0;
	} else
		sprintf(buf, "lat %lluus: %p: ret %d: %s\n",
			      u - wsi->latency_start, (void *)wsi, ret, action);

	if (u - wsi->latency_start > context->worst_latency) {
		context->worst_latency = u - wsi->latency_start;
		strcpy(context->worst_latency_info, buf);
	}
	lwsl_latency("%s", buf);
}
#endif



/**
 * lws_rx_flow_control() - Enable and disable socket servicing for
 *				received packets.
 *
 * If the output side of a server process becomes choked, this allows flow
 * control for the input side.
 *
 * @wsi:	Websocket connection instance to get callback for
 * @enable:	0 = disable read servicing for this connection, 1 = enable
 */

LWS_VISIBLE int
lws_rx_flow_control(struct lws *wsi, int enable)
{
	if (enable == (wsi->rxflow_change_to & LWS_RXFLOW_ALLOW))
		return 0;

	lwsl_info("%s: (0x%p, %d)\n", __func__, wsi, enable);
	wsi->rxflow_change_to = LWS_RXFLOW_PENDING_CHANGE | !!enable;

	return 0;
}

/**
 * lws_rx_flow_allow_all_protocol() - Allow all connections with this protocol to receive
 *
 * When the user server code realizes it can accept more input, it can
 * call this to have the RX flow restriction removed from all connections using
 * the given protocol.
 *
 * @protocol:	all connections using this protocol will be allowed to receive
 */

LWS_VISIBLE void
lws_rx_flow_allow_all_protocol(const struct lws_context *context,
			       const struct lws_protocols *protocol)
{
	int n;
	struct lws *wsi;

	for (n = 0; n < context->fds_count; n++) {
		wsi = wsi_from_fd(context, context->fds[n].fd);
		if (!wsi)
			continue;
		if (wsi->protocol == protocol)
			lws_rx_flow_control(wsi, LWS_RXFLOW_ALLOW);
	}
}


/**
 * lws_canonical_hostname() - returns this host's hostname
 *
 * This is typically used by client code to fill in the host parameter
 * when making a client connection.  You can only call it after the context
 * has been created.
 *
 * @context:	Websocket context
 */
LWS_VISIBLE extern const char *
lws_canonical_hostname(struct lws_context *context)
{
	return (const char *)context->canonical_hostname;
}

int user_callback_handle_rxflow(callback_function callback_function,
				struct lws *wsi,
				enum lws_callback_reasons reason, void *user,
				void *in, size_t len)
{
	int n;

	n = callback_function(wsi, reason, user, in, len);
	if (!n)
		n = _lws_rx_flow_control(wsi);

	return n;
}


/**
 * lws_set_proxy() - Setups proxy to lws_context.
 * @context:	pointer to struct lws_context you want set proxy to
 * @proxy: pointer to c string containing proxy in format address:port
 *
 * Returns 0 if proxy string was parsed and proxy was setup.
 * Returns -1 if @proxy is NULL or has incorrect format.
 *
 * This is only required if your OS does not provide the http_proxy
 * environment variable (eg, OSX)
 *
 *   IMPORTANT! You should call this function right after creation of the
 *   lws_context and before call to connect. If you call this
 *   function after connect behavior is undefined.
 *   This function will override proxy settings made on lws_context
 *   creation with genenv() call.
 */

LWS_VISIBLE int
lws_set_proxy(struct lws_context *context, const char *proxy)
{
	char *p;
	char authstring[96];

	if (!proxy)
		return -1;

	p = strchr(proxy, '@');
	if (p) { /* auth is around */

		if ((unsigned int)(p - proxy) > sizeof(authstring) - 1)
			goto auth_too_long;

		strncpy(authstring, proxy, p - proxy);
		// null termination not needed on input
		if (lws_b64_encode_string(authstring, (p - proxy),
		    context->proxy_basic_auth_token,
		    sizeof context->proxy_basic_auth_token) < 0)
			goto auth_too_long;

		lwsl_notice(" Proxy auth in use\n");

		proxy = p + 1;
	} else
		context->proxy_basic_auth_token[0] = '\0';

	strncpy(context->http_proxy_address, proxy,
				sizeof(context->http_proxy_address) - 1);
	context->http_proxy_address[
				sizeof(context->http_proxy_address) - 1] = '\0';

	p = strchr(context->http_proxy_address, ':');
	if (!p && !context->http_proxy_port) {
		lwsl_err("http_proxy needs to be ads:port\n");

		return -1;
	} else {
		if (p) {
			*p = '\0';
			context->http_proxy_port = atoi(p + 1);
		}
	}

	lwsl_notice(" Proxy %s:%u\n", context->http_proxy_address,
						context->http_proxy_port);

	return 0;

auth_too_long:
	lwsl_err("proxy auth too long\n");

	return -1;
}

/**
 * lws_get_protocol() - Returns a protocol pointer from a websocket
 *				  connection.
 * @wsi:	pointer to struct websocket you want to know the protocol of
 *
 *
 *	Some apis can act on all live connections of a given protocol,
 *	this is how you can get a pointer to the active protocol if needed.
 */

LWS_VISIBLE const struct lws_protocols *
lws_get_protocol(struct lws *wsi)
{
	return wsi->protocol;
}

LWS_VISIBLE int
lws_is_final_fragment(struct lws *wsi)
{
	return wsi->u.ws.final;
}

LWS_VISIBLE unsigned char
lws_get_reserved_bits(struct lws *wsi)
{
	return wsi->u.ws.rsv;
}

int
lws_ensure_user_space(struct lws *wsi)
{
	lwsl_info("%s: %p protocol %p\n", __func__, wsi, wsi->protocol);
	if (!wsi->protocol)
		return 1;

	/* allocate the per-connection user memory (if any) */

	if (wsi->protocol->per_session_data_size && !wsi->user_space) {
		wsi->user_space = lws_zalloc(wsi->protocol->per_session_data_size);
		if (wsi->user_space  == NULL) {
			lwsl_err("Out of memory for conn user space\n");
			return 1;
		}
	} else
		lwsl_info("%s: %p protocol pss %u, user_space=%d\n",
			  __func__, wsi, wsi->protocol->per_session_data_size,
			  wsi->user_space);
	return 0;
}

LWS_VISIBLE void lwsl_emit_stderr(int level, const char *line)
{
	unsigned long long now;
	char buf[300];
	int n;

	buf[0] = '\0';
	for (n = 0; n < LLL_COUNT; n++) {
		if (level != (1 << n))
			continue;
		now = time_in_microseconds() / 100;
		sprintf(buf, "[%llu:%04d] %s: ",
			(unsigned long long) now / 10000,
			(int)(now % 10000), log_level_names[n]);
		break;
	}

	fprintf(stderr, "%s%s", buf, line);
}

LWS_VISIBLE void _lws_logv(int filter, const char *format, va_list vl)
{
	char buf[256];

	if (!(log_level & filter))
		return;

	vsnprintf(buf, sizeof(buf), format, vl);
	buf[sizeof(buf) - 1] = '\0';

	lwsl_emit(filter, buf);
}

LWS_VISIBLE void _lws_log(int filter, const char *format, ...)
{
	va_list ap;

	va_start(ap, format);
	_lws_logv(filter, format, ap);
	va_end(ap);
}

/**
 * lws_set_log_level() - Set the logging bitfield
 * @level:	OR together the LLL_ debug contexts you want output from
 * @log_emit_function:	NULL to leave it as it is, or a user-supplied
 *			function to perform log string emission instead of
 *			the default stderr one.
 *
 *	log level defaults to "err", "warn" and "notice" contexts enabled and
 *	emission on stderr.
 */

LWS_VISIBLE void lws_set_log_level(int level,
				   void (*func)(int level, const char *line))
{
	log_level = level;
	if (func)
		lwsl_emit = func;
}

/**
 * lws_use_ssl() - Find out if connection is using SSL
 * @wsi:	websocket connection to check
 *
 *	Returns 0 if the connection is not using SSL, 1 if using SSL and
 *	using verified cert, and 2 if using SSL but the cert was not
 *	checked (appears for client wsi told to skip check on connection)
 */
LWS_VISIBLE int
lws_is_ssl(struct lws *wsi)
{
#ifdef LWS_OPENSSL_SUPPORT
	return wsi->use_ssl;
#else
	(void)wsi;
	return 0;
#endif
}

/**
 * lws_partial_buffered() - find out if lws buffered the last write
 * @wsi:	websocket connection to check
 *
 * Returns 1 if you cannot use lws_write because the last
 * write on this connection is still buffered, and can't be cleared without
 * returning to the service loop and waiting for the connection to be
 * writeable again.
 *
 * If you will try to do >1 lws_write call inside a single
 * WRITEABLE callback, you must check this after every write and bail if
 * set, ask for a new writeable callback and continue writing from there.
 *
 * This is never set at the start of a writeable callback, but any write
 * may set it.
 */

LWS_VISIBLE int
lws_partial_buffered(struct lws *wsi)
{
	return !!wsi->trunc_len;
}

void lws_set_protocol_write_pending(struct lws *wsi,
				    enum lws_pending_protocol_send pend)
{
	lwsl_info("setting pps %d\n", pend);

	if (wsi->pps)
		lwsl_err("pps overwrite\n");
	wsi->pps = pend;
	lws_rx_flow_control(wsi, 0);
	lws_callback_on_writable(wsi);
}

LWS_VISIBLE size_t
lws_get_peer_write_allowance(struct lws *wsi)
{
#ifdef LWS_USE_HTTP2
	/* only if we are using HTTP2 on this connection */
	if (wsi->mode != LWSCM_HTTP2_SERVING)
		return -1;
	/* user is only interested in how much he can send, or that he can't  */
	if (wsi->u.http2.tx_credit <= 0)
		return 0;

	return wsi->u.http2.tx_credit;
#else
	(void)wsi;
	return -1;
#endif
}

LWS_VISIBLE void
lws_union_transition(struct lws *wsi, enum connection_mode mode)
{
	memset(&wsi->u, 0, sizeof(wsi->u));
	wsi->mode = mode;
}

LWS_VISIBLE struct lws_plat_file_ops *
lws_get_fops(struct lws_context *context)
{
	return &context->fops;
}

LWS_VISIBLE LWS_EXTERN struct lws_context *
lws_get_context(const struct lws *wsi)
{
	return wsi->context;
}

LWS_VISIBLE LWS_EXTERN void *
lws_wsi_user(struct lws *wsi)
{
	return wsi->user_space;
}
