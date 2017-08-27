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

int
insert_wsi_socket_into_fds(struct lws_context *context, struct lws *wsi)
{
	struct lws_pollargs pa = { wsi->sock, LWS_POLLIN, 0 };

	if (context->fds_count >= context->max_fds) {
		lwsl_err("Too many fds (%d)\n", context->max_fds);
		return 1;
	}

#if !defined(_WIN32) && !defined(MBED_OPERATORS)
	if (wsi->sock >= context->max_fds) {
		lwsl_err("Socket fd %d is too high (%d)\n",
			 wsi->sock, context->max_fds);
		return 1;
	}
#endif

	assert(wsi);
	assert(lws_socket_is_valid(wsi->sock));

	if (context->protocols[0].callback(wsi, LWS_CALLBACK_LOCK_POLL,
					   wsi->user_space, (void *) &pa, 1))
		return -1;

	insert_wsi(context, wsi);
	wsi->position_in_fds_table = context->fds_count;
	context->fds[context->fds_count].fd = wsi->sock;
	context->fds[context->fds_count].events = LWS_POLLIN;

	lws_plat_insert_socket_into_fds(context, wsi);

	/* external POLL support via protocol 0 */
	if (context->protocols[0].callback(wsi, LWS_CALLBACK_ADD_POLL_FD,
					   wsi->user_space, (void *) &pa, 0))
		return -1;

	if (context->protocols[0].callback(wsi, LWS_CALLBACK_UNLOCK_POLL,
					   wsi->user_space, (void *)&pa, 1))
		return -1;

	return 0;
}

int
remove_wsi_socket_from_fds(struct lws *wsi)
{
	int m;
	struct lws *end_wsi;
	struct lws_pollargs pa = { wsi->sock, 0, 0 };
	struct lws_context *context = wsi->context;

	lws_libev_io(wsi, LWS_EV_STOP | LWS_EV_READ | LWS_EV_WRITE);

	--context->fds_count;

#if !defined(_WIN32) && !defined(MBED_OPERATORS)
	if (wsi->sock > context->max_fds) {
		lwsl_err("Socket fd %d too high (%d)\n",
			 wsi->sock, context->max_fds);
		return 1;
	}
#endif

	lwsl_info("%s: wsi=%p, sock=%d, fds pos=%d\n", __func__,
		  wsi, wsi->sock, wsi->position_in_fds_table);

	if (context->protocols[0].callback(wsi, LWS_CALLBACK_LOCK_POLL,
					   wsi->user_space, (void *)&pa, 1))
		return -1;

	m = wsi->position_in_fds_table; /* replace the contents for this */

	/* have the last guy take up the vacant slot */
	context->fds[m] = context->fds[context->fds_count];

	lws_plat_delete_socket_from_fds(context, wsi, m);

	/*
	 * end guy's fds_lookup entry remains unchanged
	 * (still same fd pointing to same wsi)
	 */
	/* end guy's "position in fds table" changed */
	end_wsi = wsi_from_fd(context, context->fds[context->fds_count].fd);
	end_wsi->position_in_fds_table = m;
	/* deletion guy's lws_lookup entry needs nuking */
	delete_from_fd(context, wsi->sock);
	/* removed wsi has no position any more */
	wsi->position_in_fds_table = -1;

	/* remove also from external POLL support via protocol 0 */
	if (lws_socket_is_valid(wsi->sock)) {
		if (context->protocols[0].callback(wsi, LWS_CALLBACK_DEL_POLL_FD,
		    wsi->user_space, (void *) &pa, 0))
			return -1;
	}
	if (context->protocols[0].callback(wsi, LWS_CALLBACK_UNLOCK_POLL,
					   wsi->user_space, (void *) &pa, 1))
		return -1;

	return 0;
}

int
lws_change_pollfd(struct lws *wsi, int _and, int _or)
{
	struct lws_context *context;
	int tid;
	int sampled_tid;
	struct lws_pollfd *pfd;
	struct lws_pollargs pa;
	int pa_events = 1;

	if (!wsi || !wsi->protocol || wsi->position_in_fds_table < 0)
		return 1;

	context = lws_get_context(wsi);
	if (!context)
		return 1;

	pfd = &context->fds[wsi->position_in_fds_table];
	pa.fd = wsi->sock;

	if (context->protocols[0].callback(wsi, LWS_CALLBACK_LOCK_POLL,
					   wsi->user_space,  (void *) &pa, 0))
		return -1;

	pa.prev_events = pfd->events;
	pa.events = pfd->events = (pfd->events & ~_and) | _or;

	if (context->protocols[0].callback(wsi, LWS_CALLBACK_CHANGE_MODE_POLL_FD,
					   wsi->user_space, (void *) &pa, 0))
		return -1;

	/*
	 * if we changed something in this pollfd...
	 *   ... and we're running in a different thread context
	 *     than the service thread...
	 *       ... and the service thread is waiting ...
	 *         then cancel it to force a restart with our changed events
	 */
#if LWS_POSIX
	pa_events = pa.prev_events != pa.events;
#endif
	if (pa_events) {

		if (lws_plat_change_pollfd(context, wsi, pfd)) {
			lwsl_info("%s failed\n", __func__);
			return 1;
		}

		sampled_tid = context->service_tid;
		if (sampled_tid) {
			tid = context->protocols[0].callback(NULL,
				     LWS_CALLBACK_GET_THREAD_ID, NULL, NULL, 0);
			if (tid == -1)
				return -1;
			if (tid != sampled_tid)
				lws_cancel_service(context);
		}
	}

	if (context->protocols[0].callback(wsi, LWS_CALLBACK_UNLOCK_POLL,
					   wsi->user_space, (void *) &pa, 0))
		return -1;

	return 0;
}


/**
 * lws_callback_on_writable() - Request a callback when this socket
 *					 becomes able to be written to without
 *					 blocking
 *
 * @wsi:	Websocket connection instance to get callback for
 */

LWS_VISIBLE int
lws_callback_on_writable(struct lws *wsi)
{
#ifdef LWS_USE_HTTP2
	struct lws *network_wsi, *wsi2;
	int already;

	lwsl_info("%s: %p\n", __func__, wsi);

	if (wsi->mode != LWSCM_HTTP2_SERVING)
		goto network_sock;

	if (wsi->u.http2.requested_POLLOUT) {
		lwsl_info("already pending writable\n");
		return 1;
	}

	if (wsi->u.http2.tx_credit <= 0) {
		/*
		 * other side is not able to cope with us sending
		 * anything so no matter if we have POLLOUT on our side.
		 *
		 * Delay waiting for our POLLOUT until peer indicates he has
		 * space for more using tx window command in http2 layer
		 */
		lwsl_info("%s: %p: waiting_tx_credit (%d)\n", __func__, wsi,
			  wsi->u.http2.tx_credit);
		wsi->u.http2.waiting_tx_credit = 1;
		return 0;
	}

	network_wsi = lws_http2_get_network_wsi(wsi);
	already = network_wsi->u.http2.requested_POLLOUT;

	/* mark everybody above him as requesting pollout */

	wsi2 = wsi;
	while (wsi2) {
		wsi2->u.http2.requested_POLLOUT = 1;
		lwsl_info("mark %p pending writable\n", wsi2);
		wsi2 = wsi2->u.http2.parent_wsi;
	}

	/* for network action, act only on the network wsi */

	wsi = network_wsi;
	if (already)
		return 1;
network_sock:
#endif

	if (lws_ext_cb_wsi_active_exts(wsi,
				LWS_EXT_CALLBACK_REQUEST_ON_WRITEABLE, NULL, 0))
		return 1;

	if (wsi->position_in_fds_table < 0) {
		lwsl_err("%s: failed to find socket %d\n", __func__, wsi->sock);
		return -1;
	}

	if (lws_change_pollfd(wsi, 0, LWS_POLLOUT))
		return -1;

	lws_libev_io(wsi, LWS_EV_START | LWS_EV_WRITE);

	return 1;
}

/**
 * lws_callback_on_writable_all_protocol() - Request a callback for
 *			all connections using the given protocol when it
 *			becomes possible to write to each socket without
 *			blocking in turn.
 *
 * @context:	lws_context
 * @protocol:	Protocol whose connections will get callbacks
 */

LWS_VISIBLE int
lws_callback_on_writable_all_protocol(const struct lws_context *context,
				      const struct lws_protocols *protocol)
{
	struct lws *wsi;
	int n;

	for (n = 0; n < context->fds_count; n++) {
		wsi = wsi_from_fd(context,context->fds[n].fd);
		if (!wsi)
			continue;
		if (wsi->protocol == protocol)
			lws_callback_on_writable(wsi);
	}

	return 0;
}
