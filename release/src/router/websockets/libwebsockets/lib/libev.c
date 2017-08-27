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

void lws_feature_status_libev(struct lws_context_creation_info *info)
{
	if (info->options & LWS_SERVER_OPTION_LIBEV)
		lwsl_notice("libev support compiled in and enabled\n");
	else
		lwsl_notice("libev support compiled in but disabled\n");
}

static void
lws_accept_cb(struct ev_loop *loop, struct ev_io *watcher, int revents)
{
	struct lws_pollfd eventfd;
	struct lws_io_watcher *lws_io = container_of(watcher,
						struct lws_io_watcher, watcher);
	struct lws_context *context = lws_io->context;

	if (revents & EV_ERROR)
		return;

	eventfd.fd = watcher->fd;
	eventfd.revents = EV_NONE;
	if (revents & EV_READ)
		eventfd.revents |= LWS_POLLIN;

	if (revents & EV_WRITE)
		eventfd.revents |= LWS_POLLOUT;

	lws_service_fd(context, &eventfd);
}

LWS_VISIBLE void
lws_sigint_cb(struct ev_loop *loop, struct ev_signal *watcher, int revents)
{
	ev_break(loop, EVBREAK_ALL);
}

LWS_VISIBLE int lws_sigint_cfg(
	struct lws_context *context,
	int use_ev_sigint,
	lws_ev_signal_cb* cb)
{
	context->use_ev_sigint = use_ev_sigint;
	if( cb ) {
		context->lws_ev_sigint_cb = cb;
	}
	else {
		context->lws_ev_sigint_cb = &lws_sigint_cb;
	};
	return 0;
};

LWS_VISIBLE int
lws_initloop(
	struct lws_context *context,
	struct ev_loop *loop)
{
	int status = 0;
	int backend;
	const char * backend_name;
	struct ev_io *w_accept = &context->w_accept.watcher;
	struct ev_signal *w_sigint = &context->w_sigint.watcher;

	if (!loop)
		loop = ev_default_loop(0);

	context->io_loop = loop;

	/*
	 * Initialize the accept w_accept with the listening socket
	 * and register a callback for read operations:
	 */
	ev_io_init(w_accept, lws_accept_cb,
					context->lserv_fd, EV_READ);
	ev_io_start(context->io_loop,w_accept);

	/* Register the signal watcher unless the user has indicated otherwise: */
	if( context->use_ev_sigint ) {
		ev_signal_init(w_sigint, context->lws_ev_sigint_cb, SIGINT);
		ev_signal_start(context->io_loop,w_sigint);
	};
	backend = ev_backend(loop);

	switch (backend) {
	case EVBACKEND_SELECT:
		backend_name = "select";
		break;
	case EVBACKEND_POLL:
		backend_name = "poll";
		break;
	case EVBACKEND_EPOLL:
		backend_name = "epoll";
		break;
	case EVBACKEND_KQUEUE:
		backend_name = "kqueue";
		break;
	case EVBACKEND_DEVPOLL:
		backend_name = "/dev/poll";
		break;
	case EVBACKEND_PORT:
		backend_name = "Solaris 10 \"port\"";
		break;
	default:
		backend_name = "Unknown libev backend";
		break;
	};

	lwsl_notice(" libev backend: %s\n", backend_name);

	return status;
}

LWS_VISIBLE void
lws_libev_accept(struct lws *new_wsi, int accept_fd)
{
	struct lws_context *context = lws_get_context(new_wsi);
	struct ev_io *r = &new_wsi->w_read.watcher;
	struct ev_io *w = &new_wsi->w_write.watcher;

	if (!LWS_LIBEV_ENABLED(context))
		return;

	new_wsi->w_read.context = context;
	new_wsi->w_write.context = context;
	ev_io_init(r, lws_accept_cb, accept_fd, EV_READ);
	ev_io_init(w, lws_accept_cb, accept_fd, EV_WRITE);
}

LWS_VISIBLE void
lws_libev_io(struct lws *wsi, int flags)
{
	struct lws_context *context = lws_get_context(wsi);

	if (!LWS_LIBEV_ENABLED(context))
		return;

	if (!context->io_loop)
		return;

	assert((flags & (LWS_EV_START | LWS_EV_STOP)) &&
		(flags & (LWS_EV_READ | LWS_EV_WRITE)));

	if (flags & LWS_EV_START) {
		if (flags & LWS_EV_WRITE)
			ev_io_start(context->io_loop, &wsi->w_write.watcher);
		if (flags & LWS_EV_READ)
			ev_io_start(context->io_loop, &wsi->w_read.watcher);
	} else {
		if (flags & LWS_EV_WRITE)
			ev_io_stop(context->io_loop, &wsi->w_write.watcher);
		if (flags & LWS_EV_READ)
			ev_io_stop(context->io_loop, &wsi->w_read.watcher);
	}
}

LWS_VISIBLE int
lws_libev_init_fd_table(struct lws_context *context)
{
	if (!LWS_LIBEV_ENABLED(context))
		return 0;

	context->w_accept.context = context;
	context->w_sigint.context = context;

	return 1;
}

LWS_VISIBLE void
lws_libev_run(const struct lws_context *context)
{
	if (context->io_loop && LWS_LIBEV_ENABLED(context))
		ev_run(context->io_loop, 0);
}
