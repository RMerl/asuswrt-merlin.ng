#include "first.h"

#include "fdevent.h"
#include "buffer.h"
#include "log.h"

#include <assert.h>

#ifdef USE_LIBEV

# include <ev.h>

static void io_watcher_cb(struct ev_loop *loop, ev_io *w, int revents) {
	fdevents *ev = w->data;
	fdevent_handler handler = fdevent_get_handler(ev, w->fd);
	void *context = fdevent_get_context(ev, w->fd);
	int r = 0;
	UNUSED(loop);
	if (NULL == handler) return;

	if (revents & EV_READ) r |= FDEVENT_IN;
	if (revents & EV_WRITE) r |= FDEVENT_OUT;
	if (revents & EV_ERROR) r |= FDEVENT_ERR;

	switch (r = (*handler)(ev->srv, context, r)) {
	case HANDLER_FINISHED:
	case HANDLER_GO_ON:
	case HANDLER_WAIT_FOR_EVENT:
	case HANDLER_WAIT_FOR_FD:
		break;
	case HANDLER_ERROR:
		/* should never happen */
		SEGFAULT();
		break;
	default:
		log_error_write(ev->srv, __FILE__, __LINE__, "d", r);
		break;
	}
}

static void fdevent_libev_free(fdevents *ev) {
	UNUSED(ev);
}

static int fdevent_libev_event_del(fdevents *ev, int fde_ndx, int fd) {
	fdnode *fdn;
	ev_io *watcher;

	if (-1 == fde_ndx) return -1;

	fdn = ev->fdarray[fd];
	watcher = fdn->handler_ctx;

	if (!watcher) return -1;

	ev_io_stop(ev->libev_loop, watcher);
	free(watcher);
	fdn->handler_ctx = NULL;

	return -1;
}

static int fdevent_libev_event_set(fdevents *ev, int fde_ndx, int fd, int events) {
	fdnode *fdn = ev->fdarray[fd];
	ev_io *watcher = fdn->handler_ctx;
	int ev_events = 0;
	UNUSED(fde_ndx);

	if (events & FDEVENT_IN)  ev_events |= EV_READ;
	if (events & FDEVENT_OUT) ev_events |= EV_WRITE;

	if (!watcher) {
		fdn->handler_ctx = watcher = calloc(1, sizeof(ev_io));
		force_assert(watcher);

		ev_io_init(watcher, io_watcher_cb, fd, ev_events);
		watcher->data = ev;
		ev_io_start(ev->libev_loop, watcher);
	} else {
		if ((watcher->events & (EV_READ | EV_WRITE)) != ev_events) {
			ev_io_stop(ev->libev_loop, watcher);
			ev_io_set(watcher, watcher->fd, ev_events);
			ev_io_start(ev->libev_loop, watcher);
		}
	}

	return fd;
}

static void timeout_watcher_cb(struct ev_loop *loop, ev_timer *w, int revents) {
	UNUSED(loop);
	UNUSED(w);
	UNUSED(revents);
}

static ev_timer timeout_watcher;

static int fdevent_libev_poll(fdevents *ev, int timeout_ms) {
	timeout_watcher.repeat = (timeout_ms > 0) ? timeout_ms/1000.0 : 0.001;

	ev_timer_again(ev->libev_loop, &timeout_watcher);
	ev_run(ev->libev_loop, EVRUN_ONCE);
	fdevent_sched_run(ev->srv, ev);

	return 0;
}

static int fdevent_libev_event_get_revent(fdevents *ev, size_t ndx) {
	UNUSED(ev);
	UNUSED(ndx);

	return 0;
}

static int fdevent_libev_event_get_fd(fdevents *ev, size_t ndx) {
	UNUSED(ev);
	UNUSED(ndx);

	return -1;
}

static int fdevent_libev_event_next_fdndx(fdevents *ev, int ndx) {
	UNUSED(ev);
	UNUSED(ndx);

	return -1;
}

static int fdevent_libev_reset(fdevents *ev) {
	UNUSED(ev);

	ev_default_fork();

	return 0;
}

int fdevent_libev_init(fdevents *ev) {
	struct ev_timer * const timer = &timeout_watcher;

	ev->type = FDEVENT_HANDLER_LIBEV;
#define SET(x) \
	ev->x = fdevent_libev_##x;

	SET(free);
	SET(poll);
	SET(reset);

	SET(event_del);
	SET(event_set);

	SET(event_next_fdndx);
	SET(event_get_fd);
	SET(event_get_revent);

	if (NULL == (ev->libev_loop = ev_default_loop(0))) {
		log_error_write(ev->srv, __FILE__, __LINE__, "S",
			"ev_default_loop failed , try to set server.event-handler = \"poll\" or \"select\"");

		return -1;
	}

	ev_timer_init(timer, timeout_watcher_cb, 0.0, 1.0);

	return 0;
}

#else
int fdevent_libev_init(fdevents *ev) {
	UNUSED(ev);

	log_error_write(ev->srv, __FILE__, __LINE__, "S",
		"libev not supported, try to set server.event-handler = \"poll\" or \"select\"");

	return -1;
}
#endif
