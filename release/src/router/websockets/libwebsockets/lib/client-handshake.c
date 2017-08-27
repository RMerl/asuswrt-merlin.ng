#include "private-libwebsockets.h"

struct lws *
lws_client_connect_2(struct lws *wsi)
{
#ifdef LWS_USE_IPV6
	struct sockaddr_in6 server_addr6;
	struct sockaddr_in6 client_addr6;
	struct addrinfo hints, *result;
#endif
	struct lws_context *context = wsi->context;
	struct sockaddr_in server_addr4;
	struct sockaddr_in client_addr4;
	struct lws_pollfd pfd;
	struct sockaddr *v;
	int n, plen = 0;
	const char *ads;

	lwsl_client("%s\n", __func__);

	/* proxy? */

	if (context->http_proxy_port) {
		plen = sprintf((char *)context->serv_buf,
			"CONNECT %s:%u HTTP/1.0\x0d\x0a"
			"User-agent: libwebsockets\x0d\x0a",
			lws_hdr_simple_ptr(wsi, _WSI_TOKEN_CLIENT_PEER_ADDRESS),
			wsi->u.hdr.ah->c_port);

		if (context->proxy_basic_auth_token[0])
			plen += sprintf((char *)context->serv_buf + plen,
					"Proxy-authorization: basic %s\x0d\x0a",
					context->proxy_basic_auth_token);

		plen += sprintf((char *)context->serv_buf + plen,
				"\x0d\x0a");

		ads = context->http_proxy_address;

#ifdef LWS_USE_IPV6
		if (LWS_IPV6_ENABLED(context)) {
			memset(&server_addr6, 0, sizeof(struct sockaddr_in6));
			server_addr6.sin6_port = htons(context->http_proxy_port);
		} else
#endif
			server_addr4.sin_port = htons(context->http_proxy_port);

	} else {
		ads = lws_hdr_simple_ptr(wsi, _WSI_TOKEN_CLIENT_PEER_ADDRESS);
#ifdef LWS_USE_IPV6
		if (LWS_IPV6_ENABLED(context)) {
			memset(&server_addr6, 0, sizeof(struct sockaddr_in6));
			server_addr6.sin6_port = htons(wsi->u.hdr.ah->c_port);
		} else
#endif
			server_addr4.sin_port = htons(wsi->u.hdr.ah->c_port);
	}

	/*
	 * prepare the actual connection (to the proxy, if any)
	 */
       lwsl_client("%s: address %s\n", __func__, ads);

#ifdef LWS_USE_IPV6
	if (LWS_IPV6_ENABLED(context)) {
		memset(&hints, 0, sizeof(struct addrinfo));
#if !defined(__ANDROID__)
		hints.ai_family = AF_INET6;
		hints.ai_flags = AI_V4MAPPED;
#endif
		n = getaddrinfo(ads, NULL, &hints, &result);
		if (n) {
#ifdef _WIN32
			lwsl_err("getaddrinfo: %ls\n", gai_strerrorW(n));
#else
			lwsl_err("getaddrinfo: %s\n", gai_strerror(n));
#endif
			goto oom4;
		}

		server_addr6.sin6_family = AF_INET6;
		switch (result->ai_family) {
#if defined(__ANDROID__)
		case AF_INET:
			/* map IPv4 to IPv6 */
			bzero((char *)&server_addr6.sin6_addr,
						sizeof(struct in6_addr));
			server_addr6.sin6_addr.s6_addr[10] = 0xff;
			server_addr6.sin6_addr.s6_addr[11] = 0xff;
			memcpy(&server_addr6.sin6_addr.s6_addr[12],
				&((struct sockaddr_in *)result->ai_addr)->sin_addr,
							sizeof(struct in_addr));
			break;
#endif
		case AF_INET6:
			memcpy(&server_addr6.sin6_addr,
			  &((struct sockaddr_in6 *)result->ai_addr)->sin6_addr,
						sizeof(struct in6_addr));
			break;
		default:
			lwsl_err("Unknown address family\n");
			freeaddrinfo(result);
			goto oom4;
		}

		freeaddrinfo(result);
	} else
#endif
	{
		struct addrinfo ai, *res, *result;
		void *p = NULL;

		memset (&ai, 0, sizeof ai);
		ai.ai_family = PF_UNSPEC;
		ai.ai_socktype = SOCK_STREAM;
		ai.ai_flags = AI_CANONNAME;

		if (getaddrinfo(ads, NULL, &ai, &result))
			goto oom4;

		res = result;
		while (!p && res) {
			switch (res->ai_family) {
			case AF_INET:
				p = &((struct sockaddr_in *)res->ai_addr)->sin_addr;
				break;
			}

			res = res->ai_next;
		}

		if (!p) {
			freeaddrinfo(result);
			goto oom4;
		}

		server_addr4.sin_family = AF_INET;
		server_addr4.sin_addr = *((struct in_addr *)p);
		bzero(&server_addr4.sin_zero, 8);
		freeaddrinfo(result);
	}

	if (!lws_socket_is_valid(wsi->sock)) {

#ifdef LWS_USE_IPV6
		if (LWS_IPV6_ENABLED(context))
			wsi->sock = socket(AF_INET6, SOCK_STREAM, 0);
		else
#endif
			wsi->sock = socket(AF_INET, SOCK_STREAM, 0);

		if (!lws_socket_is_valid(wsi->sock)) {
			lwsl_warn("Unable to open socket\n");
			goto oom4;
		}

		if (lws_plat_set_socket_options(context, wsi->sock)) {
			lwsl_err("Failed to set wsi socket options\n");
			compatible_close(wsi->sock);
			goto oom4;
		}

		wsi->mode = LWSCM_WSCL_WAITING_CONNECT;

		lws_libev_accept(wsi, wsi->sock);
		if (insert_wsi_socket_into_fds(context, wsi)) {
			compatible_close(wsi->sock);
			goto oom4;
		}

		/*
		 * past here, we can't simply free the structs as error
		 * handling as oom4 does.  We have to run the whole close flow.
		 */

		lws_set_timeout(wsi,
			PENDING_TIMEOUT_AWAITING_CONNECT_RESPONSE,
							      AWAITING_TIMEOUT);
#ifdef LWS_USE_IPV6
		if (LWS_IPV6_ENABLED(context)) {
			v = (struct sockaddr *)&client_addr6;
			n = sizeof(client_addr6);
			bzero((char *)v, n);
			client_addr6.sin6_family = AF_INET6;
		} else
#endif
		{
			v = (struct sockaddr *)&client_addr4;
			n = sizeof(client_addr4);
			bzero((char *)v, n);
			client_addr4.sin_family = AF_INET;
		}

		if (context->iface) {
			if (interface_to_sa(context, context->iface,
					(struct sockaddr_in *)v, n) < 0) {
				lwsl_err("Unable to find interface %s\n",
								context->iface);
				goto failed;
			}

			if (bind(wsi->sock, v, n) < 0) {
				lwsl_err("Error binding to interface %s",
								context->iface);
				goto failed;
			}
		}
	}

#ifdef LWS_USE_IPV6
	if (LWS_IPV6_ENABLED(context)) {
		v = (struct sockaddr *)&server_addr6;
		n = sizeof(struct sockaddr_in6);
	} else
#endif
	{
		v = (struct sockaddr *)&server_addr4;
		n = sizeof(struct sockaddr);
	}

	if (connect(wsi->sock, v, n) == -1 || LWS_ERRNO == LWS_EISCONN) {
		if (LWS_ERRNO == LWS_EALREADY ||
		    LWS_ERRNO == LWS_EINPROGRESS ||
		    LWS_ERRNO == LWS_EWOULDBLOCK
#ifdef _WIN32
			|| LWS_ERRNO == WSAEINVAL
#endif
		) {
			lwsl_client("nonblocking connect retry\n");

			/*
			 * must do specifically a POLLOUT poll to hear
			 * about the connect completion
			 */
			if (lws_change_pollfd(wsi, 0, LWS_POLLOUT))
				goto failed;
			lws_libev_io(wsi, LWS_EV_START | LWS_EV_WRITE);

			return wsi;
		}

		if (LWS_ERRNO != LWS_EISCONN) {
			lwsl_debug("Connect failed errno=%d\n", LWS_ERRNO);
			goto failed;
		}
	}

	lwsl_client("connected\n");

	/* we are connected to server, or proxy */

	if (context->http_proxy_port) {

		/*
		 * OK from now on we talk via the proxy, so connect to that
		 *
		 * (will overwrite existing pointer,
		 * leaving old string/frag there but unreferenced)
		 */
		if (lws_hdr_simple_create(wsi, _WSI_TOKEN_CLIENT_PEER_ADDRESS,
					  context->http_proxy_address))
			goto failed;
		wsi->u.hdr.ah->c_port = context->http_proxy_port;

		n = send(wsi->sock, (char *)context->serv_buf, plen,
			 MSG_NOSIGNAL);
		if (n < 0) {
			lwsl_debug("ERROR writing to proxy socket\n");
			goto failed;
		}

		lws_set_timeout(wsi, PENDING_TIMEOUT_AWAITING_PROXY_RESPONSE,
				AWAITING_TIMEOUT);

		wsi->mode = LWSCM_WSCL_WAITING_PROXY_REPLY;

		return wsi;
	}

	/*
	 * provoke service to issue the handshake directly
	 * we need to do it this way because in the proxy case, this is the
	 * next state and executed only if and when we get a good proxy
	 * response inside the state machine... but notice in SSL case this
	 * may not have sent anything yet with 0 return, and won't until some
	 * many retries from main loop.  To stop that becoming endless,
	 * cover with a timeout.
	 */

	lws_set_timeout(wsi, PENDING_TIMEOUT_SENT_CLIENT_HANDSHAKE,
			AWAITING_TIMEOUT);

	wsi->mode = LWSCM_WSCL_ISSUE_HANDSHAKE;
	pfd.fd = wsi->sock;
	pfd.revents = LWS_POLLIN;

	n = lws_service_fd(context, &pfd);
	if (n < 0)
		goto failed;
	if (n) /* returns 1 on failure after closing wsi */
		return NULL;

	return wsi;

oom4:
	lws_free(wsi->u.hdr.ah);
	lws_free(wsi);

	return NULL;

failed:
	lws_close_free_wsi(wsi, LWS_CLOSE_STATUS_NOSTATUS);

	return NULL;
}

/**
 * lws_client_connect() - Connect to another websocket server
 * @context:	Websocket context
 * @address:	Remote server address, eg, "myserver.com"
 * @port:	Port to connect to on the remote server, eg, 80
 * @ssl_connection:	0 = ws://, 1 = wss:// encrypted, 2 = wss:// allow self
 *			signed certs
 * @path:	Websocket path on server
 * @host:	Hostname on server
 * @origin:	Socket origin name
 * @protocol:	Comma-separated list of protocols being asked for from
 *		the server, or just one.  The server will pick the one it
 *		likes best.  If you don't want to specify a protocol, which is
 *		legal, use NULL here.
 * @ietf_version_or_minus_one: -1 to ask to connect using the default, latest
 *		protocol supported, or the specific protocol ordinal
 *
 *	This function creates a connection to a remote server
 */

LWS_VISIBLE struct lws *
lws_client_connect(struct lws_context *context, const char *address,
		   int port, int ssl_connection, const char *path,
		   const char *host, const char *origin,
		   const char *protocol, int ietf_version_or_minus_one)
{
	struct lws *wsi;

	wsi = lws_zalloc(sizeof(struct lws));
	if (wsi == NULL)
		goto bail;

	wsi->context = context;
	wsi->sock = LWS_SOCK_INVALID;

	/* -1 means just use latest supported */

	if (ietf_version_or_minus_one == -1)
		ietf_version_or_minus_one = SPEC_LATEST_SUPPORTED;

	wsi->ietf_spec_revision = ietf_version_or_minus_one;
	wsi->user_space = NULL;
	wsi->state = LWSS_CLIENT_UNCONNECTED;
	wsi->protocol = NULL;
	wsi->pending_timeout = NO_PENDING_TIMEOUT;

#ifdef LWS_OPENSSL_SUPPORT
	wsi->use_ssl = ssl_connection;
#else
	if (ssl_connection) {
		lwsl_err("libwebsockets not configured for ssl\n");
		goto bail;
	}
#endif

	if (lws_allocate_header_table(wsi))
		goto bail;

	/*
	 * we're not necessarily in a position to action these right away,
	 * stash them... we only need during connect phase so u.hdr is fine
	 */
	wsi->u.hdr.ah->c_port = port;
	if (lws_hdr_simple_create(wsi, _WSI_TOKEN_CLIENT_PEER_ADDRESS, address))
		goto bail1;

	/* these only need u.hdr lifetime as well */

	if (lws_hdr_simple_create(wsi, _WSI_TOKEN_CLIENT_URI, path))
		goto bail1;

	if (lws_hdr_simple_create(wsi, _WSI_TOKEN_CLIENT_HOST, host))
		goto bail1;

	if (origin)
		if (lws_hdr_simple_create(wsi, _WSI_TOKEN_CLIENT_ORIGIN, origin))
			goto bail1;
	/*
	 * this is a list of protocols we tell the server we're okay with
	 * stash it for later when we compare server response with it
	 */
	if (protocol)
		if (lws_hdr_simple_create(wsi, _WSI_TOKEN_CLIENT_SENT_PROTOCOLS,
					  protocol))
			goto bail1;

	wsi->protocol = &context->protocols[0];

	/*
	 * Check with each extension if it is able to route and proxy this
	 * connection for us.  For example, an extension like x-google-mux
	 * can handle this and then we don't need an actual socket for this
	 * connection.
	 */

	if (lws_ext_cb_all_exts(context, wsi,
			LWS_EXT_CALLBACK_CAN_PROXY_CLIENT_CONNECTION,
						     (void *)address, port) > 0) {
		lwsl_client("lws_client_connect: ext handling conn\n");

		lws_set_timeout(wsi,
			PENDING_TIMEOUT_AWAITING_EXTENSION_CONNECT_RESPONSE,
			        AWAITING_TIMEOUT);

		wsi->mode = LWSCM_WSCL_WAITING_EXTENSION_CONNECT;
		return wsi;
	}
	lwsl_client("lws_client_connect: direct conn\n");

       return lws_client_connect_2(wsi);

bail1:
	lws_free(wsi->u.hdr.ah);
bail:
	lws_free(wsi);

	return NULL;
}


/**
 * lws_client_connect_extended() - Connect to another websocket server
 * @context:	Websocket context
 * @address:	Remote server address, eg, "myserver.com"
 * @port:	Port to connect to on the remote server, eg, 80
 * @ssl_connection:	0 = ws://, 1 = wss:// encrypted, 2 = wss:// allow self
 *			signed certs
 * @path:	Websocket path on server
 * @host:	Hostname on server
 * @origin:	Socket origin name
 * @protocol:	Comma-separated list of protocols being asked for from
 *		the server, or just one.  The server will pick the one it
 *		likes best.
 * @ietf_version_or_minus_one: -1 to ask to connect using the default, latest
 *		protocol supported, or the specific protocol ordinal
 * @userdata: Pre-allocated user data
 *
 *	This function creates a connection to a remote server
 */

LWS_VISIBLE struct lws *
lws_client_connect_extended(struct lws_context *context, const char *address,
			    int port, int ssl_connection, const char *path,
			    const char *host, const char *origin,
			    const char *protocol, int ietf_version_or_minus_one,
			    void *userdata)
{
	struct lws *wsi;

	wsi = lws_client_connect(context, address, port, ssl_connection, path,
				 host, origin, protocol,
				 ietf_version_or_minus_one);

	if (wsi && !wsi->user_space && userdata) {
		wsi->user_space_externally_allocated = 1;
		wsi->user_space = userdata ;
	}

	return wsi;
}
