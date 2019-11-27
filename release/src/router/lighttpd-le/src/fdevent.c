#include "first.h"

#include "base.h"
#include "log.h"

#include <sys/types.h>

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <fcntl.h>
#include <assert.h>


fdevents *fdevent_init(server *srv, size_t maxfds, fdevent_handler_t type) {
	fdevents *ev;

	ev = calloc(1, sizeof(*ev));
	force_assert(NULL != ev);
	ev->srv = srv;
	ev->fdarray = calloc(maxfds, sizeof(*ev->fdarray));
	force_assert(NULL != ev->fdarray);
	ev->maxfds = maxfds;
	ev->highfd = -1;

	switch(type) {
	case FDEVENT_HANDLER_POLL:
		if (0 != fdevent_poll_init(ev)) {
			log_error_write(srv, __FILE__, __LINE__, "S",
				"event-handler poll failed");
			goto error;
		}
		return ev;
	case FDEVENT_HANDLER_SELECT:
		if (0 != fdevent_select_init(ev)) {
			log_error_write(srv, __FILE__, __LINE__, "S",
				"event-handler select failed");
			goto error;
		}
		return ev;
	case FDEVENT_HANDLER_LINUX_SYSEPOLL:
		if (0 != fdevent_linux_sysepoll_init(ev)) {
			log_error_write(srv, __FILE__, __LINE__, "S",
				"event-handler linux-sysepoll failed, try to set server.event-handler = \"poll\" or \"select\"");
			goto error;
		}
		return ev;
	case FDEVENT_HANDLER_SOLARIS_DEVPOLL:
		if (0 != fdevent_solaris_devpoll_init(ev)) {
			log_error_write(srv, __FILE__, __LINE__, "S",
				"event-handler solaris-devpoll failed, try to set server.event-handler = \"poll\" or \"select\"");
			goto error;
		}
		return ev;
	case FDEVENT_HANDLER_SOLARIS_PORT:
		if (0 != fdevent_solaris_port_init(ev)) {
			log_error_write(srv, __FILE__, __LINE__, "S",
				"event-handler solaris-eventports failed, try to set server.event-handler = \"poll\" or \"select\"");
			goto error;
		}
		return ev;
	case FDEVENT_HANDLER_FREEBSD_KQUEUE:
		if (0 != fdevent_freebsd_kqueue_init(ev)) {
			log_error_write(srv, __FILE__, __LINE__, "S",
				"event-handler freebsd-kqueue failed, try to set server.event-handler = \"poll\" or \"select\"");
			goto error;
		}
		return ev;
	case FDEVENT_HANDLER_LIBEV:
		if (0 != fdevent_libev_init(ev)) {
			log_error_write(srv, __FILE__, __LINE__, "S",
				"event-handler libev failed, try to set server.event-handler = \"poll\" or \"select\"");
			goto error;
		}
		return ev;
	case FDEVENT_HANDLER_UNSET:
		break;
	}

error:
	free(ev->fdarray);
	free(ev);

	log_error_write(srv, __FILE__, __LINE__, "S",
		"event-handler is unknown, try to set server.event-handler = \"poll\" or \"select\"");
	return NULL;
}

void fdevent_free(fdevents *ev) {
	size_t i;
	if (!ev) return;

	if (ev->free) ev->free(ev);

	for (i = 0; i < ev->maxfds; i++) {
		if (ev->fdarray[i] > (fdnode *)0x2) free(ev->fdarray[i]);
	}

	free(ev->fdarray);
	free(ev);
}

int fdevent_reset(fdevents *ev) {
	if (ev->reset) return ev->reset(ev);

	return 0;
}

static fdnode *fdnode_init(void) {
	fdnode *fdn;

	fdn = calloc(1, sizeof(*fdn));
	force_assert(NULL != fdn);
	fdn->fd = -1;
	return fdn;
}

static void fdnode_free(fdnode *fdn) {
	free(fdn);
}

int fdevent_register(fdevents *ev, int fd, fdevent_handler handler, void *ctx) {
	fdnode *fdn;

	fdn = fdnode_init();
	fdn->handler = handler;
	fdn->fd      = fd;
	fdn->ctx     = ctx;
	fdn->handler_ctx = NULL;
	fdn->events  = 0;

	ev->fdarray[fd] = fdn;

	return 0;
}

int fdevent_unregister(fdevents *ev, int fd) {
	fdnode *fdn;

	if (!ev) return 0;
	fdn = ev->fdarray[fd];

	fdnode_free(fdn);

	ev->fdarray[fd] = NULL;

	return 0;
}

void fdevent_sched_close(fdevents *ev, int fd, int issock) {
	if (!ev) return;
	ev->fdarray[fd] = (issock ? (fdnode *)0x1 : (fdnode *)0x2);
	if (ev->highfd < fd) ev->highfd = fd;
}

void fdevent_sched_run(server *srv, fdevents *ev) {
	const int highfd = ev->highfd;
	for (int fd = 0; fd <= highfd; ++fd) {
		fdnode * const fdn = ev->fdarray[fd];
		int rc;
		if (!((uintptr_t)fdn & 0x3)) continue;
	      #ifdef _WIN32
		if (fdn == (fdnode *)0x1) {
			rc = closesocket(fd);
		}
		else if (fdn == (fdnode *)0x2) {
			rc = close(fd);
		}
	      #else
		rc = close(fd);
	      #endif

		if (0 != rc) {
			log_error_write(srv, __FILE__, __LINE__, "sds", "close failed ", fd, strerror(errno));
		}

		ev->fdarray[fd] = NULL;
		--srv->cur_fds;
	}
	ev->highfd = -1;
}

void fdevent_event_del(fdevents *ev, int *fde_ndx, int fd) {
	if (-1 == fd) return;
	if (ev->fdarray[fd] <= (fdnode *)0x2) return;

	if (ev->event_del) *fde_ndx = ev->event_del(ev, *fde_ndx, fd);
	ev->fdarray[fd]->events = 0;
}

void fdevent_event_set(fdevents *ev, int *fde_ndx, int fd, int events) {
	if (-1 == fd) return;

	/*(Note: skips registering with kernel if initial events is 0,
         * so caller should pass non-zero events for initial registration.
         * If never registered due to never being called with non-zero events,
         * then FDEVENT_HUP or FDEVENT_ERR will never be returned.) */
	if (ev->fdarray[fd]->events == events) return;/*(no change; nothing to do)*/

	if (ev->event_set) *fde_ndx = ev->event_set(ev, *fde_ndx, fd, events);
	ev->fdarray[fd]->events = events;
}

void fdevent_event_add(fdevents *ev, int *fde_ndx, int fd, int event) {
	int events;
	if (-1 == fd) return;

	events = ev->fdarray[fd]->events;
	if ((events & event) || 0 == event) return; /*(no change; nothing to do)*/

	events |= event;
	if (ev->event_set) *fde_ndx = ev->event_set(ev, *fde_ndx, fd, events);
	ev->fdarray[fd]->events = events;
}

void fdevent_event_clr(fdevents *ev, int *fde_ndx, int fd, int event) {
	int events;
	if (-1 == fd) return;

	events = ev->fdarray[fd]->events;
	if (!(events & event)) return; /*(no change; nothing to do)*/

	events &= ~event;
	if (ev->event_set) *fde_ndx = ev->event_set(ev, *fde_ndx, fd, events);
	ev->fdarray[fd]->events = events;
}

int fdevent_poll(fdevents *ev, int timeout_ms) {
	if (ev->poll == NULL) SEGFAULT();
	return ev->poll(ev, timeout_ms);
}

int fdevent_event_get_revent(fdevents *ev, size_t ndx) {
	if (ev->event_get_revent == NULL) SEGFAULT();

	return ev->event_get_revent(ev, ndx);
}

int fdevent_event_get_fd(fdevents *ev, size_t ndx) {
	if (ev->event_get_fd == NULL) SEGFAULT();

	return ev->event_get_fd(ev, ndx);
}

fdevent_handler fdevent_get_handler(fdevents *ev, int fd) {
	if (ev->fdarray[fd] == NULL) SEGFAULT();
	if ((uintptr_t)ev->fdarray[fd] & 0x3) return NULL;
	if (ev->fdarray[fd]->fd != fd) SEGFAULT();

	return ev->fdarray[fd]->handler;
}

void * fdevent_get_context(fdevents *ev, int fd) {
	if (ev->fdarray[fd] == NULL) SEGFAULT();
	if ((uintptr_t)ev->fdarray[fd] & 0x3) return NULL;
	if (ev->fdarray[fd]->fd != fd) SEGFAULT();

	return ev->fdarray[fd]->ctx;
}

void fd_close_on_exec(int fd) {
#ifdef FD_CLOEXEC
	if (fd < 0) return;
	force_assert(-1 != fcntl(fd, F_SETFD, FD_CLOEXEC));
#else
	UNUSED(fd);
#endif
}

int fdevent_fcntl_set(fdevents *ev, int fd) {
	return ((ev) && (ev->fcntl_set)) ? ev->fcntl_set(ev, fd) : 0;
}

int fdevent_fcntl_set_nb(fdevents *ev, int fd) {
	if ((ev) && (ev->fcntl_set)) return ev->fcntl_set(ev, fd);
#ifdef O_NONBLOCK
	return fcntl(fd, F_SETFL, O_NONBLOCK | O_RDWR);
#else
	return 0;
#endif
}

int fdevent_fcntl_set_nb_cloexec(fdevents *ev, int fd) {
	fd_close_on_exec(fd);
	return fdevent_fcntl_set_nb(ev, fd);
}

int fdevent_fcntl_set_nb_cloexec_sock(fdevents *ev, int fd) {
#if defined(SOCK_CLOEXEC) && defined(SOCK_NONBLOCK)
	return ((ev) && (ev->fcntl_set)) ? ev->fcntl_set(ev, fd) : 0;
#else
	return fdevent_fcntl_set_nb_cloexec(ev, fd);
#endif
}

int fdevent_socket_cloexec(int domain, int type, int protocol) {
#ifdef SOCK_CLOEXEC
	return socket(domain, type | SOCK_CLOEXEC, protocol);
#else
	int fd;
	if (-1 != (fd = socket(domain, type, protocol))) {
#ifdef FD_CLOEXEC
		fcntl(fd, F_SETFD, FD_CLOEXEC);
#endif
	}
	return fd;
#endif
}

int fdevent_socket_nb_cloexec(int domain, int type, int protocol) {
#ifdef SOCK_CLOEXEC
	return socket(domain, type | SOCK_CLOEXEC | SOCK_NONBLOCK, protocol);
#else
	int fd;
	if (-1 != (fd = socket(domain, type, protocol))) {
#ifdef FD_CLOEXEC
		fcntl(fd, F_SETFD, FD_CLOEXEC);
#endif
#ifdef O_NONBLOCK
		fcntl(fd, F_SETFL, O_NONBLOCK | O_RDWR);
#endif
	}
	return fd;
#endif
}

#ifndef O_NOCTTY
#define O_NOCTTY 0
#endif

int fdevent_open_cloexec(const char *pathname, int flags, mode_t mode) {
#ifdef O_CLOEXEC
	return open(pathname, flags | O_CLOEXEC | O_NOCTTY, mode);
#else
	int fd = open(pathname, flags | O_NOCTTY, mode);
#ifdef FD_CLOEXEC
	if (fd != -1)
		fcntl(fd, F_SETFD, FD_CLOEXEC);
#endif
	return fd;
#endif
}


int fdevent_event_next_fdndx(fdevents *ev, int ndx) {
	if (ev->event_next_fdndx) return ev->event_next_fdndx(ev, ndx);

	return -1;
}


#include <netinet/tcp.h>
#if (defined(__APPLE__) && defined(__MACH__)) \
  || defined(__FreeBSD__) || defined(__NetBSD__) \
  || defined(__OpenBSD__) || defined(__DragonFly__)
#include <netinet/tcp_fsm.h>
#endif

/* fd must be TCP socket (AF_INET, AF_INET6), end-of-stream recv() 0 bytes */
int fdevent_is_tcp_half_closed(int fd) {
  #ifdef TCP_CONNECTION_INFO     /* Darwin */
    struct tcp_connection_info tcpi;
    socklen_t tlen = sizeof(tcpi);
    return (0 == getsockopt(fd, IPPROTO_TCP, TCP_CONNECTION_INFO, &tcpi, &tlen)
            && tcpi.tcpi_state == TCPS_CLOSE_WAIT);
  #elif defined(TCP_INFO) && defined(TCPS_CLOSE_WAIT)
    /* FreeBSD, NetBSD (not present in OpenBSD or DragonFlyBSD) */
    struct tcp_info tcpi;
    socklen_t tlen = sizeof(tcpi);
    return (0 == getsockopt(fd, IPPROTO_TCP, TCP_INFO, &tcpi, &tlen)
            && tcpi.tcpi_state == TCPS_CLOSE_WAIT);
  #elif defined(TCP_INFO) && defined(__linux__)
    /* Linux (TCP_CLOSE_WAIT is enum, so can not #ifdef TCP_CLOSE_WAIT) */
    struct tcp_info tcpi;
    socklen_t tlen = sizeof(tcpi);/*SOL_TCP == IPPROTO_TCP*/
    return (0 == getsockopt(fd,     SOL_TCP, TCP_INFO, &tcpi, &tlen)
            && tcpi.tcpi_state == TCP_CLOSE_WAIT);
  #else
    UNUSED(fd);
    /*(0 != getpeername() error might indicate TCP RST, but success
     * would not differentiate between half-close and full-close)*/
    return 0; /* false (not half-closed) or TCP state unknown */
  #endif
}
