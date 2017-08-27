/*
 * libwebsockets - small server side websockets and web server implementation
 *
 * Copyright (C) 2010-2015 Andy Green <andy@warmcat.com>
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

#ifndef LWS_BUILD_HASH
#define LWS_BUILD_HASH "unknown-build-hash"
#endif

static const char *library_version = LWS_LIBRARY_VERSION " " LWS_BUILD_HASH;

/**
 * lws_get_library_version: get version and git hash library built from
 *
 *	returns a const char * to a string like "1.1 178d78c"
 *	representing the library version followed by the git head hash it
 *	was built from
 */

LWS_VISIBLE const char *
lws_get_library_version(void)
{
	return library_version;
}

/**
 * lws_create_context() - Create the websocket handler
 * @info:	pointer to struct with parameters
 *
 *	This function creates the listening socket (if serving) and takes care
 *	of all initialization in one step.
 *
 *	After initialization, it returns a struct lws_context * that
 *	represents this server.  After calling, user code needs to take care
 *	of calling lws_service() with the context pointer to get the
 *	server's sockets serviced.  This must be done in the same process
 *	context as the initialization call.
 *
 *	The protocol callback functions are called for a handful of events
 *	including http requests coming in, websocket connections becoming
 *	established, and data arriving; it's also called periodically to allow
 *	async transmission.
 *
 *	HTTP requests are sent always to the FIRST protocol in @protocol, since
 *	at that time websocket protocol has not been negotiated.  Other
 *	protocols after the first one never see any HTTP callack activity.
 *
 *	The server created is a simple http server by default; part of the
 *	websocket standard is upgrading this http connection to a websocket one.
 *
 *	This allows the same server to provide files like scripts and favicon /
 *	images or whatever over http and dynamic data over websockets all in
 *	one place; they're all handled in the user callback.
 */

LWS_VISIBLE struct lws_context *
lws_create_context(struct lws_context_creation_info *info)
{
	struct lws_context *context = NULL;
	struct lws wsi;
#ifndef LWS_NO_DAEMONIZE
	int pid_daemon = get_daemonize_pid();
#endif
	char *p;

	lwsl_notice("Initial logging level %d\n", log_level);

	lwsl_notice("Libwebsockets version: %s\n", library_version);
#if LWS_POSIX
#ifdef LWS_USE_IPV6
	if (!(info->options & LWS_SERVER_OPTION_DISABLE_IPV6))
		lwsl_notice("IPV6 compiled in and enabled\n");
	else
		lwsl_notice("IPV6 compiled in but disabled\n");
#else
	lwsl_notice("IPV6 not compiled in\n");
#endif
	lws_feature_status_libev(info);
#endif
	lwsl_info(" LWS_MAX_HEADER_LEN: %u\n", LWS_MAX_HEADER_LEN);
	lwsl_info(" LWS_MAX_PROTOCOLS: %u\n", LWS_MAX_PROTOCOLS);

	lwsl_info(" SPEC_LATEST_SUPPORTED: %u\n", SPEC_LATEST_SUPPORTED);
	lwsl_info(" AWAITING_TIMEOUT: %u\n", AWAITING_TIMEOUT);
#if LWS_POSIX
	lwsl_info(" SYSTEM_RANDOM_FILEPATH: '%s'\n", SYSTEM_RANDOM_FILEPATH);
	lwsl_info(" LWS_MAX_ZLIB_CONN_BUFFER: %u\n", LWS_MAX_ZLIB_CONN_BUFFER);
#endif
	if (lws_plat_context_early_init())
		return NULL;

	context = lws_zalloc(sizeof(struct lws_context));
	if (!context) {
		lwsl_err("No memory for websocket context\n");
		return NULL;
	}
#ifndef LWS_NO_DAEMONIZE
	if (pid_daemon) {
		context->started_with_parent = pid_daemon;
		lwsl_notice(" Started with daemon pid %d\n", pid_daemon);
	}
#endif
	context->lserv_seen = 0;
	context->protocols = info->protocols;
	context->token_limits = info->token_limits;
	context->listen_port = info->port;
	context->http_proxy_port = 0;
	context->http_proxy_address[0] = '\0';
	context->options = info->options;
	context->iface = info->iface;
	context->ka_time = info->ka_time;
	context->ka_interval = info->ka_interval;
	context->ka_probes = info->ka_probes;

	memset(&wsi, 0, sizeof(wsi));
	wsi.context = context;

	if (!info->ka_interval && info->ka_time > 0) {
		lwsl_err("info->ka_interval can't be 0 if ka_time used\n");
		return NULL;
	}

#ifdef LWS_USE_LIBEV
	/* (Issue #264) In order to *avoid breaking backwards compatibility*, we
	 * enable libev mediated SIGINT handling with a default handler of
	 * lws_sigint_cb. The handler can be overridden or disabled
	 * by invoking lws_sigint_cfg after creating the context, but
	 * before invoking lws_initloop:
	 */
	context->use_ev_sigint = 1;
	context->lws_ev_sigint_cb = &lws_sigint_cb;
#endif /* LWS_USE_LIBEV */

	/* to reduce this allocation, */
	context->max_fds = getdtablesize();
	lwsl_notice(" ctx mem: %u bytes\n", sizeof(struct lws_context) +
		    ((sizeof(struct lws_pollfd) + sizeof(struct lws *)) *
		    context->max_fds));

	context->fds = lws_zalloc(sizeof(struct lws_pollfd) * context->max_fds);
	if (context->fds == NULL) {
		lwsl_err("OOM allocating %d fds\n", context->max_fds);
		goto bail;
	}

	if (lws_plat_init(context, info))
		goto bail;

	lws_context_init_extensions(info, context);

	context->user_space = info->user;

	strcpy(context->canonical_hostname, "unknown");

	lws_server_get_canonical_hostname(context, info);

	/* either use proxy from info, or try get it from env var */

	if (info->http_proxy_address) {
		/* override for backwards compatibility */
		if (info->http_proxy_port)
			context->http_proxy_port = info->http_proxy_port;
		lws_set_proxy(context, info->http_proxy_address);
	} else {
#ifdef LWS_HAVE_GETENV
		p = getenv("http_proxy");
		if (p)
			lws_set_proxy(context, p);
#endif
	}

	lwsl_notice(" per-conn mem: %u + %u headers + protocol rx buf\n",
		    sizeof(struct lws), sizeof(struct allocated_headers));

	if (lws_context_init_server_ssl(info, context))
		goto bail;

	if (lws_context_init_client_ssl(info, context))
		goto bail;

	if (lws_context_init_server(info, context))
		goto bail;

	/*
	 * drop any root privs for this process
	 * to listen on port < 1023 we would have needed root, but now we are
	 * listening, we don't want the power for anything else
	 */
	lws_plat_drop_app_privileges(info);

	/* initialize supported protocols */

	for (context->count_protocols = 0;
	     info->protocols[context->count_protocols].callback;
	     context->count_protocols++)
		/*
		 * inform all the protocols that they are doing their one-time
		 * initialization if they want to.
		 *
		 * NOTE the wsi is all zeros except for the context pointer
		 * so lws_get_context(wsi) can work in the callback.
		 */
		info->protocols[context->count_protocols].callback(&wsi,
				LWS_CALLBACK_PROTOCOL_INIT, NULL, NULL, 0);

	/*
	 * give all extensions a chance to create any per-context
	 * allocations they need
	 */
	if (info->port != CONTEXT_PORT_NO_LISTEN) {
		if (lws_ext_cb_all_exts(context, NULL,
			LWS_EXT_CALLBACK_SERVER_CONTEXT_CONSTRUCT, NULL, 0) < 0)
			goto bail;
	} else
		if (lws_ext_cb_all_exts(context, NULL,
			LWS_EXT_CALLBACK_CLIENT_CONTEXT_CONSTRUCT, NULL, 0) < 0)
			goto bail;

	return context;

bail:
	lws_context_destroy(context);
	return NULL;
}

/**
 * lws_context_destroy() - Destroy the websocket context
 * @context:	Websocket context
 *
 *	This function closes any active connections and then frees the
 *	context.  After calling this, any further use of the context is
 *	undefined.
 */
LWS_VISIBLE void
lws_context_destroy(struct lws_context *context)
{
	const struct lws_protocols *protocol = NULL;
	struct lws wsi;
	int n;

	lwsl_notice("%s\n", __func__);

	if (!context)
		return;

	context->being_destroyed = 1;

	memset(&wsi, 0, sizeof(wsi));
	wsi.context = context;

#ifdef LWS_LATENCY
	if (context->worst_latency_info[0])
		lwsl_notice("Worst latency: %s\n", context->worst_latency_info);
#endif

	for (n = 0; n < context->fds_count; n++) {
		struct lws *wsi = wsi_from_fd(context, context->fds[n].fd);
		if (!wsi)
			continue;
		lws_close_free_wsi(wsi, LWS_CLOSE_STATUS_NOSTATUS_CONTEXT_DESTROY
				/* no protocol close */);
		n--;
	}

	/*
	 * give all extensions a chance to clean up any per-context
	 * allocations they might have made
	 */

	n = lws_ext_cb_all_exts(context, NULL,
			LWS_EXT_CALLBACK_SERVER_CONTEXT_DESTRUCT, NULL, 0);

	n = lws_ext_cb_all_exts(context, NULL,
			LWS_EXT_CALLBACK_CLIENT_CONTEXT_DESTRUCT, NULL, 0);

	/*
	 * inform all the protocols that they are done and will have no more
	 * callbacks
	 */
	protocol = context->protocols;
	if (protocol) {
		while (protocol->callback) {
			protocol->callback(&wsi,
					   LWS_CALLBACK_PROTOCOL_DESTROY,
					   NULL, NULL, 0);
			protocol++;
		}
	}

	lws_plat_context_early_destroy(context);
	lws_ssl_context_destroy(context);

	if (context->fds)
		lws_free(context->fds);

	lws_plat_context_late_destroy(context);

	lws_free(context);
}
