/* -*- mode: c; c-file-style: "openbsd" -*- */
/*
 * Copyright (c) 2012 Vincent Bernat <bernat@luffy.cx>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include "lldpd.h"
#include "trace.h"

#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <time.h>
#include <fcntl.h>
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdocumentation"
#endif
#include <event2/event.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#if defined(__clang__)
#pragma clang diagnostic pop
#endif

#define EVENT_BUFFER 1024

static void
levent_log_cb(int severity, const char *msg)
{
	switch (severity) {
	case _EVENT_LOG_DEBUG: log_debug("libevent", "%s", msg); break;
	case _EVENT_LOG_MSG:   log_info ("libevent", "%s", msg);  break;
	case _EVENT_LOG_WARN:  log_warnx("libevent", "%s", msg);  break;
	case _EVENT_LOG_ERR:   log_warnx("libevent", "%s", msg); break;
	}
}

struct lldpd_events {
	TAILQ_ENTRY(lldpd_events) next;
	struct event *ev;
};
TAILQ_HEAD(ev_l, lldpd_events);

#define levent_snmp_fds(cfg)   ((struct ev_l*)(cfg)->g_snmp_fds)
#define levent_hardware_fds(hardware) ((struct ev_l*)(hardware)->h_recv)

#ifdef USE_SNMP
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include <net-snmp/agent/snmp_vars.h>

/* Compatibility with older versions of NetSNMP */
#ifndef HAVE_SNMP_SELECT_INFO2
# define netsnmp_large_fd_set fd_set
# define snmp_read2 snmp_read
# define snmp_select_info2 snmp_select_info
# define netsnmp_large_fd_set_init(...)
# define netsnmp_large_fd_set_cleanup(...)
# define NETSNMP_LARGE_FD_SET FD_SET
# define NETSNMP_LARGE_FD_CLR FD_CLR
# define NETSNMP_LARGE_FD_ZERO FD_ZERO
# define NETSNMP_LARGE_FD_ISSET FD_ISSET
#else
# include <net-snmp/library/large_fd_set.h>
#endif

static void levent_snmp_update(struct lldpd *);

/*
 * Callback function when we have something to read from SNMP.
 *
 * This function is called because we have a read event on one SNMP
 * file descriptor. When need to call snmp_read() on it.
 */
static void
levent_snmp_read(evutil_socket_t fd, short what, void *arg)
{
	struct lldpd *cfg = arg;
	netsnmp_large_fd_set fdset;
	(void)what;
	netsnmp_large_fd_set_init(&fdset, FD_SETSIZE);
	NETSNMP_LARGE_FD_ZERO(&fdset);
	NETSNMP_LARGE_FD_SET(fd, &fdset);
	snmp_read2(&fdset);
	levent_snmp_update(cfg);
}

/*
 * Callback function for a SNMP timeout.
 *
 * A SNMP timeout has occurred. Call `snmp_timeout()` to handle it.
 */
static void
levent_snmp_timeout(evutil_socket_t fd, short what, void *arg)
{
	struct lldpd *cfg = arg;
	(void)what; (void)fd;
	snmp_timeout();
	run_alarms();
	levent_snmp_update(cfg);
}

/*
 * Watch a new SNMP FD.
 *
 * @param base The libevent base we are working on.
 * @param fd The file descriptor we want to watch.
 *
 * The file descriptor is appended to the list of file descriptors we
 * want to watch.
 */
static void
levent_snmp_add_fd(struct lldpd *cfg, int fd)
{
	struct event_base *base = cfg->g_base;
	struct lldpd_events *snmpfd = calloc(1, sizeof(struct lldpd_events));
	if (!snmpfd) {
		log_warn("event", "unable to allocate memory for new SNMP event");
		return;
	}
	levent_make_socket_nonblocking(fd);
	if ((snmpfd->ev = event_new(base, fd,
				    EV_READ | EV_PERSIST,
				    levent_snmp_read,
				    cfg)) == NULL) {
		log_warnx("event", "unable to allocate a new SNMP event for FD %d", fd);
		free(snmpfd);
		return;
	}
	if (event_add(snmpfd->ev, NULL) == -1) {
		log_warnx("event", "unable to schedule new SNMP event for FD %d", fd);
		event_free(snmpfd->ev);
		free(snmpfd);
		return;
	}
	TAILQ_INSERT_TAIL(levent_snmp_fds(cfg), snmpfd, next);
}

/*
 * Update SNMP event loop.
 *
 * New events are added and some other are removed. This function
 * should be called every time a SNMP event happens: either when
 * handling a SNMP packet, a SNMP timeout or when sending a SNMP
 * packet. This function will keep libevent in sync with NetSNMP.
 *
 * @param base The libevent base we are working on.
 */
static void
levent_snmp_update(struct lldpd *cfg)
{
	int maxfd = 0;
	int block = 1;
	struct timeval timeout;
	static int howmany = 0;
	int added = 0, removed = 0, current = 0;
	struct lldpd_events *snmpfd, *snmpfd_next;

	/* snmp_select_info() can be tricky to understand. We set `block` to
	   1 to means that we don't request a timeout. snmp_select_info()
	   will reset `block` to 0 if it wants us to setup a timeout. In
	   this timeout, `snmp_timeout()` should be invoked.

	   Each FD in `fdset` will need to be watched for reading. If one of
	   them become active, `snmp_read()` should be called on it.
	*/

	netsnmp_large_fd_set fdset;
	netsnmp_large_fd_set_init(&fdset, FD_SETSIZE);
        NETSNMP_LARGE_FD_ZERO(&fdset);
	snmp_select_info2(&maxfd, &fdset, &timeout, &block);

	/* We need to untrack any event whose FD is not in `fdset`
	   anymore */
	for (snmpfd = TAILQ_FIRST(levent_snmp_fds(cfg));
	     snmpfd;
	     snmpfd = snmpfd_next) {
		snmpfd_next = TAILQ_NEXT(snmpfd, next);
		if (event_get_fd(snmpfd->ev) >= maxfd ||
		    (!NETSNMP_LARGE_FD_ISSET(event_get_fd(snmpfd->ev), &fdset))) {
			event_free(snmpfd->ev);
			TAILQ_REMOVE(levent_snmp_fds(cfg), snmpfd, next);
			free(snmpfd);
			removed++;
		} else {
			NETSNMP_LARGE_FD_CLR(event_get_fd(snmpfd->ev), &fdset);
			current++;
		}
	}

	/* Invariant: FD in `fdset` are not in list of FD */
	for (int fd = 0; fd < maxfd; fd++) {
		if (NETSNMP_LARGE_FD_ISSET(fd, &fdset)) {
			levent_snmp_add_fd(cfg, fd);
			added++;
		}
	}
	current += added;
	if (howmany != current) {
		log_debug("event", "added %d events, removed %d events, total of %d events",
			   added, removed, current);
		howmany = current;
	}

	/* If needed, handle timeout */
	if (evtimer_add(cfg->g_snmp_timeout, block?NULL:&timeout) == -1)
		log_warnx("event", "unable to schedule timeout function for SNMP");

	netsnmp_large_fd_set_cleanup(&fdset);
}
#endif /* USE_SNMP */

struct lldpd_one_client {
	TAILQ_ENTRY(lldpd_one_client) next;
	struct lldpd *cfg;
	struct bufferevent *bev;
	int    subscribed;	/* Is this client subscribed to changes? */
};
TAILQ_HEAD(, lldpd_one_client) lldpd_clients;

static void
levent_ctl_free_client(struct lldpd_one_client *client)
{
	if (client && client->bev) bufferevent_free(client->bev);
	if (client) {
		TAILQ_REMOVE(&lldpd_clients, client, next);
		free(client);
	}
}

static void
levent_ctl_close_clients()
{
	struct lldpd_one_client *client, *client_next;
	for (client = TAILQ_FIRST(&lldpd_clients);
	     client;
	     client = client_next) {
		client_next = TAILQ_NEXT(client, next);
		levent_ctl_free_client(client);
	}
}

static ssize_t
levent_ctl_send(struct lldpd_one_client *client, int type, void *data, size_t len)
{
	struct bufferevent *bev = client->bev;
	struct hmsg_header hdr = { .len = len, .type = type };
	bufferevent_disable(bev, EV_WRITE);
	if (bufferevent_write(bev, &hdr, sizeof(struct hmsg_header)) == -1 ||
	    (len > 0 && bufferevent_write(bev, data, len) == -1)) {
		log_warnx("event", "unable to create answer to client");
		levent_ctl_free_client(client);
		return -1;
	}
	bufferevent_enable(bev, EV_WRITE);
	return len;
}

void
levent_ctl_notify(char *ifname, int state, struct lldpd_port *neighbor)
{
	struct lldpd_one_client *client, *client_next;
	struct lldpd_neighbor_change neigh = {
		.ifname = ifname,
		.state  = state,
		.neighbor = neighbor
	};
	void *output = NULL;
	ssize_t output_len = 0;

	/* Don't use TAILQ_FOREACH, the client may be deleted in case of errors. */
	log_debug("control", "notify clients of neighbor changes");
	for (client = TAILQ_FIRST(&lldpd_clients);
	     client;
	     client = client_next) {
		client_next = TAILQ_NEXT(client, next);
		if (!client->subscribed) continue;

		if (output == NULL) {
			/* Ugly hack: we don't want to transmit a list of
			 * ports. We patch the port to avoid this. */
			TAILQ_ENTRY(lldpd_port) backup_p_entries;
			memcpy(&backup_p_entries, &neighbor->p_entries,
			    sizeof(backup_p_entries));
			memset(&neighbor->p_entries, 0,
			    sizeof(backup_p_entries));
			output_len = lldpd_neighbor_change_serialize(&neigh, &output);
			memcpy(&neighbor->p_entries, &backup_p_entries,
			    sizeof(backup_p_entries));

			if (output_len <= 0) {
				log_warnx("event", "unable to serialize changed neighbor");
				return;
			}
		}

		levent_ctl_send(client, NOTIFICATION, output, output_len);
	}

	free(output);
}

static ssize_t
levent_ctl_send_cb(void *out, int type, void *data, size_t len)
{
	struct lldpd_one_client *client = out;
	return levent_ctl_send(client, type, data, len);
}

static void
levent_ctl_recv(struct bufferevent *bev, void *ptr)
{
	struct lldpd_one_client *client = ptr;
	struct evbuffer *buffer = bufferevent_get_input(bev);
	size_t buffer_len       = evbuffer_get_length(buffer);
	struct hmsg_header hdr;
	void *data = NULL;

	log_debug("control", "receive data on Unix socket");
	if (buffer_len < sizeof(struct hmsg_header))
		return;		/* Not enough data yet */
	if (evbuffer_copyout(buffer, &hdr,
		sizeof(struct hmsg_header)) != sizeof(struct hmsg_header)) {
		log_warnx("event", "not able to read header");
		return;
	}
	if (hdr.len > HMSG_MAX_SIZE) {
		log_warnx("event", "message received is too large");
		goto recv_error;
	}

	if (buffer_len < hdr.len + sizeof(struct hmsg_header))
		return;		/* Not enough data yet */
	if (hdr.len > 0 && (data = malloc(hdr.len)) == NULL) {
		log_warnx("event", "not enough memory");
		goto recv_error;
	}
	evbuffer_drain(buffer, sizeof(struct hmsg_header));
	if (hdr.len > 0) evbuffer_remove(buffer, data, hdr.len);

	/* Currently, we should not receive notification acknowledgment. But if
	 * we receive one, we can discard it. */
	if (hdr.len == 0 && hdr.type == NOTIFICATION) return;
	if (client_handle_client(client->cfg,
		levent_ctl_send_cb, client,
		hdr.type, data, hdr.len,
		&client->subscribed) == -1) goto recv_error;
	free(data);
	return;

recv_error:
	free(data);
	levent_ctl_free_client(client);
}

static void
levent_ctl_event(struct bufferevent *bev, short events, void *ptr)
{
	struct lldpd_one_client *client = ptr;
	if (events & BEV_EVENT_ERROR) {
		log_warnx("event", "an error occurred with client: %s",
		    evutil_socket_error_to_string(EVUTIL_SOCKET_ERROR()));
		levent_ctl_free_client(client);
	} else if (events & BEV_EVENT_EOF) {
		log_debug("event", "client has been disconnected");
		levent_ctl_free_client(client);
	}
}

static void
levent_ctl_accept(evutil_socket_t fd, short what, void *arg)
{
	struct lldpd *cfg = arg;
	struct lldpd_one_client *client = NULL;
	int s;
	(void)what;

	log_debug("control", "accept a new connection");
	if ((s = accept(fd, NULL, NULL)) == -1) {
		log_warn("event", "unable to accept connection from socket");
		return;
	}
	client = calloc(1, sizeof(struct lldpd_one_client));
	if (!client) {
		log_warnx("event", "unable to allocate memory for new client");
		close(s);
		goto accept_failed;
	}
	client->cfg = cfg;
	levent_make_socket_nonblocking(s);
	TAILQ_INSERT_TAIL(&lldpd_clients, client, next);
	if ((client->bev = bufferevent_socket_new(cfg->g_base, s,
		    BEV_OPT_CLOSE_ON_FREE)) == NULL) {
		log_warnx("event", "unable to allocate a new buffer event for new client");
		close(s);
		goto accept_failed;
	}
	bufferevent_setcb(client->bev,
	    levent_ctl_recv, NULL, levent_ctl_event,
	    client);
	bufferevent_enable(client->bev, EV_READ | EV_WRITE);
	log_debug("event", "new client accepted");
	/* coverity[leaked_handle]
	   s has been saved by bufferevent_socket_new */
	return;
accept_failed:
	levent_ctl_free_client(client);
}

static void
levent_priv(evutil_socket_t fd, short what, void *arg)
{
	struct event_base *base = arg;
	ssize_t n;
	int err;
	char one;
	(void)what;
	/* Check if we have some data available. We need to pass the socket in
	 * non-blocking mode to be able to run the check without disruption. */
	levent_make_socket_nonblocking(fd);
	n = read(fd, &one, 0); err = errno;
	levent_make_socket_blocking(fd);

	switch (n) {
	case -1:
		if (err == EAGAIN || err == EWOULDBLOCK)
			/* No data, all good */
			return;
		log_warnx("event", "unable to poll monitor process, exit");
		break;
	case 0:
		log_warnx("event", "monitor process has terminated, exit");
		break;
	default:
		/* Unfortunately, dead code, if we have data, we have requested
		 * 0 byte, so we will fall in the previous case. It seems safer
		 * to ask for 0 byte than asking for 1 byte. In the later case,
		 * if we have to speak with the monitor again before exiting, we
		 * would be out of sync. */
		log_warnx("event", "received unexpected data from monitor process, exit");
		break;
	}
	event_base_loopbreak(base);
}

static void
levent_dump(evutil_socket_t fd, short what, void *arg)
{
	struct event_base *base = arg;
	(void)fd; (void)what;
	log_debug("event", "dumping all events");
	event_base_dump_events(base, stderr);
}
static void
levent_stop(evutil_socket_t fd, short what, void *arg)
{
	struct event_base *base = arg;
	(void)fd; (void)what;
	event_base_loopbreak(base);
}

static void
levent_update_and_send(evutil_socket_t fd, short what, void *arg)
{
	struct lldpd *cfg = arg;
	struct timeval tv;
	long interval_ms = cfg->g_config.c_tx_interval;

	(void)fd; (void)what;
	lldpd_loop(cfg);
	if (cfg->g_iface_event != NULL)
		interval_ms *= 20;
	if (interval_ms < 30000)
		interval_ms = 30000;
	tv.tv_sec = interval_ms / 1000;
	tv.tv_usec = (interval_ms % 1000) * 1000;
	event_add(cfg->g_main_loop, &tv);
}

void
levent_update_now(struct lldpd *cfg)
{
	if (cfg->g_main_loop)
		event_active(cfg->g_main_loop, EV_TIMEOUT, 1);
}

void
levent_send_now(struct lldpd *cfg)
{
	struct lldpd_hardware *hardware;
	TAILQ_FOREACH(hardware, &cfg->g_hardware, h_entries) {
		if (hardware->h_timer)
			event_active(hardware->h_timer, EV_TIMEOUT, 1);
		else
			log_warnx("event", "BUG: no timer present for interface %s",
			    hardware->h_ifname);
	}
}

static void
levent_init(struct lldpd *cfg)
{
	/* Setup libevent */
	log_debug("event", "initialize libevent");
	event_set_log_callback(levent_log_cb);
	if (!(cfg->g_base = event_base_new()))
		fatalx("event", "unable to create a new libevent base");
	log_info("event", "libevent %s initialized with %s method",
		  event_get_version(),
		  event_base_get_method(cfg->g_base));

	/* Setup SNMP */
#ifdef USE_SNMP
	if (cfg->g_snmp) {
		agent_init(cfg, cfg->g_snmp_agentx);
		cfg->g_snmp_timeout = evtimer_new(cfg->g_base,
		    levent_snmp_timeout,
		    cfg);
		if (!cfg->g_snmp_timeout)
			fatalx("event", "unable to setup timeout function for SNMP");
		if ((cfg->g_snmp_fds =
			malloc(sizeof(struct ev_l))) == NULL)
			fatalx("event", "unable to allocate memory for SNMP events");
		TAILQ_INIT(levent_snmp_fds(cfg));
	}
#endif

	/* Setup loop that will run every X seconds. */
	log_debug("event", "register loop timer");
	if (!(cfg->g_main_loop = event_new(cfg->g_base, -1, 0,
					   levent_update_and_send,
					   cfg)))
		fatalx("event", "unable to setup main timer");
	event_active(cfg->g_main_loop, EV_TIMEOUT, 1);

	/* Setup unix socket */
	struct event *ctl_event;
	log_debug("event", "register Unix socket");
	TAILQ_INIT(&lldpd_clients);
	levent_make_socket_nonblocking(cfg->g_ctl);
	if ((ctl_event = event_new(cfg->g_base, cfg->g_ctl,
		    EV_READ|EV_PERSIST, levent_ctl_accept, cfg)) == NULL)
		fatalx("event", "unable to setup control socket event");
	event_add(ctl_event, NULL);

	/* Somehow monitor the monitor process */
	struct event *monitor_event;
	log_debug("event", "monitor the monitor process");
	if ((monitor_event = event_new(cfg->g_base, priv_fd(PRIV_UNPRIVILEGED),
		    EV_READ|EV_PERSIST, levent_priv, cfg->g_base)) == NULL)
		fatalx("event", "unable to monitor monitor process");
	event_add(monitor_event, NULL);

	/* Signals */
	log_debug("event", "register signals");
	evsignal_add(evsignal_new(cfg->g_base, SIGUSR1,
		levent_dump, cfg->g_base),
	    NULL);
	evsignal_add(evsignal_new(cfg->g_base, SIGINT,
		levent_stop, cfg->g_base),
	    NULL);
	evsignal_add(evsignal_new(cfg->g_base, SIGTERM,
		levent_stop, cfg->g_base),
	    NULL);
}

/* Initialize libevent and start the event loop */
void
levent_loop(struct lldpd *cfg)
{
	levent_init(cfg);
	lldpd_loop(cfg);
#ifdef USE_SNMP
	if (cfg->g_snmp) levent_snmp_update(cfg);
#endif

	/* libevent loop */
	do {
		TRACE(LLDPD_EVENT_LOOP());
		if (event_base_got_break(cfg->g_base) ||
		    event_base_got_exit(cfg->g_base))
			break;
	} while (event_base_loop(cfg->g_base, EVLOOP_ONCE) == 0);

	if (cfg->g_iface_timer_event != NULL)
		event_free(cfg->g_iface_timer_event);

#ifdef USE_SNMP
	if (cfg->g_snmp)
		agent_shutdown();
#endif /* USE_SNMP */

	levent_ctl_close_clients();
}

/* Release libevent resources */
void
levent_shutdown(struct lldpd *cfg)
{
	if (cfg->g_iface_event)
		event_free(cfg->g_iface_event);
	if (cfg->g_cleanup_timer)
		event_free(cfg->g_cleanup_timer);
	event_base_free(cfg->g_base);
}

static void
levent_hardware_recv(evutil_socket_t fd, short what, void *arg)
{
	struct lldpd_hardware *hardware = arg;
	struct lldpd *cfg = hardware->h_cfg;
	(void)what;
	log_debug("event", "received something for %s",
	    hardware->h_ifname);
	lldpd_recv(cfg, hardware, fd);
	levent_schedule_cleanup(cfg);
}

void
levent_hardware_init(struct lldpd_hardware *hardware)
{
	log_debug("event", "initialize events for %s", hardware->h_ifname);
	if ((hardware->h_recv =
		malloc(sizeof(struct ev_l))) == NULL) {
		log_warnx("event", "unable to allocate memory for %s",
		    hardware->h_ifname);
		return;
	}
	TAILQ_INIT(levent_hardware_fds(hardware));
}

void
levent_hardware_add_fd(struct lldpd_hardware *hardware, int fd)
{
	struct lldpd_events *hfd = NULL;
	if (!hardware->h_recv) return;

	hfd = calloc(1, sizeof(struct lldpd_events));
	if (!hfd) {
		log_warnx("event", "unable to allocate new event for %s",
		    hardware->h_ifname);
		return;
	}
	levent_make_socket_nonblocking(fd);
	if ((hfd->ev = event_new(hardware->h_cfg->g_base, fd,
		    EV_READ | EV_PERSIST,
		    levent_hardware_recv,
		    hardware)) == NULL) {
		log_warnx("event", "unable to allocate a new event for %s",
			hardware->h_ifname);
		free(hfd);
		return;
	}
	if (event_add(hfd->ev, NULL) == -1) {
		log_warnx("event", "unable to schedule new event for %s",
			hardware->h_ifname);
		event_free(hfd->ev);
		free(hfd);
		return;
	}
	TAILQ_INSERT_TAIL(levent_hardware_fds(hardware), hfd, next);
}

void
levent_hardware_release(struct lldpd_hardware *hardware)
{
	struct lldpd_events *ev, *ev_next;
	if (hardware->h_timer) {
		event_free(hardware->h_timer);
		hardware->h_timer = NULL;
	}
	if (!hardware->h_recv) return;

	log_debug("event", "release events for %s", hardware->h_ifname);
	for (ev = TAILQ_FIRST(levent_hardware_fds(hardware));
	     ev;
	     ev = ev_next) {
		ev_next = TAILQ_NEXT(ev, next);
		/* We may close several time the same FD. This is harmless. */
		close(event_get_fd(ev->ev));
		event_free(ev->ev);
		TAILQ_REMOVE(levent_hardware_fds(hardware), ev, next);
		free(ev);
	}
	free(levent_hardware_fds(hardware));
}

static void
levent_iface_trigger(evutil_socket_t fd, short what, void *arg)
{
	struct lldpd *cfg = arg;
	log_debug("event",
	    "triggering update of all interfaces");
	lldpd_update_localports(cfg);
}

static void
levent_iface_recv(evutil_socket_t fd, short what, void *arg)
{
	struct lldpd *cfg = arg;
	char buffer[EVENT_BUFFER];
	int n;

	if (cfg->g_iface_cb == NULL) {
		/* Discard the message */
		while (1) {
			n = read(fd, buffer, sizeof(buffer));
			if (n == -1 &&
			    (errno == EWOULDBLOCK ||
				errno == EAGAIN)) break;
			if (n == -1) {
				log_warn("event",
				    "unable to receive interface change notification message");
				return;
			}
			if (n == 0) {
				log_warnx("event",
				    "end of file reached while getting interface change notification message");
				return;
			}
		}
	} else {
		cfg->g_iface_cb(cfg);
	}

	/* Schedule local port update. We don't run it right away because we may
	 * receive a batch of events like this. */
	struct timeval one_sec = {1, 0};
	TRACE(LLDPD_INTERFACES_NOTIFICATION());
	log_debug("event",
	    "received notification change, schedule an update of all interfaces in one second");
	if (cfg->g_iface_timer_event == NULL) {
		if ((cfg->g_iface_timer_event = evtimer_new(cfg->g_base,
			    levent_iface_trigger, cfg)) == NULL) {
			log_warnx("event",
			    "unable to create a new event to trigger interface update");
			return;
		}
	}
	if (evtimer_add(cfg->g_iface_timer_event, &one_sec) == -1) {
		log_warnx("event",
		    "unable to schedule interface updates");
		return;
	}
}

int
levent_iface_subscribe(struct lldpd *cfg, int socket)
{
	log_debug("event", "subscribe to interface changes from socket %d",
	    socket);
	levent_make_socket_nonblocking(socket);
	cfg->g_iface_event = event_new(cfg->g_base, socket,
	    EV_READ | EV_PERSIST, levent_iface_recv, cfg);
	if (cfg->g_iface_event == NULL) {
		log_warnx("event",
		    "unable to allocate a new event for interface changes");
		return -1;
	}
	if (event_add(cfg->g_iface_event, NULL) == -1) {
		log_warnx("event",
		    "unable to schedule new interface changes event");
		event_free(cfg->g_iface_event);
		cfg->g_iface_event = NULL;
		return -1;
	}
	return 0;
}

static void
levent_trigger_cleanup(evutil_socket_t fd, short what, void *arg)
{
	struct lldpd *cfg = arg;
	lldpd_cleanup(cfg);
}

void
levent_schedule_cleanup(struct lldpd *cfg)
{
	log_debug("event", "schedule next cleanup");
	if (cfg->g_cleanup_timer != NULL) {
		event_free(cfg->g_cleanup_timer);
	}
	cfg->g_cleanup_timer = evtimer_new(cfg->g_base, levent_trigger_cleanup, cfg);
	if (cfg->g_cleanup_timer == NULL) {
		log_warnx("event",
		    "unable to allocate a new event for cleanup tasks");
		return;
	}

	/* Compute the next TTL event */
	struct timeval tv = { cfg->g_config.c_ttl, 0 };
	time_t now = time(NULL);
	time_t next;
	struct lldpd_hardware *hardware;
	struct lldpd_port *port;
	TAILQ_FOREACH(hardware, &cfg->g_hardware, h_entries) {
		TAILQ_FOREACH(port, &hardware->h_rports, p_entries) {
			if (now >= port->p_lastupdate + port->p_ttl) {
				tv.tv_sec = 0;
				log_debug("event", "immediate cleanup on port %s (%lld, %d, %lld)",
				    hardware->h_ifname,
				    (long long)now,
				    port->p_ttl,
				    (long long)port->p_lastupdate);
				break;
			}
			next = port->p_ttl - (now - port->p_lastupdate);
			if (next < tv.tv_sec)
				tv.tv_sec = next;
		}
	}

	log_debug("event", "next cleanup in %ld seconds",
	    (long)tv.tv_sec);
	if (event_add(cfg->g_cleanup_timer, &tv) == -1) {
		log_warnx("event",
		    "unable to schedule cleanup task");
		event_free(cfg->g_cleanup_timer);
		cfg->g_cleanup_timer = NULL;
		return;
	}
}

static void
levent_send_pdu(evutil_socket_t fd, short what, void *arg)
{
	struct lldpd_hardware *hardware = arg;
	int tx_interval = hardware->h_cfg->g_config.c_tx_interval;

	log_debug("event", "trigger sending PDU for port %s",
	    hardware->h_ifname);
	lldpd_send(hardware);

#ifdef ENABLE_LLDPMED
	if (hardware->h_tx_fast > 0)
		hardware->h_tx_fast--;

	if (hardware->h_tx_fast > 0)
		tx_interval = hardware->h_cfg->g_config.c_tx_fast_interval * 1000;
#endif

	struct timeval tv;
	tv.tv_sec = tx_interval / 1000;
	tv.tv_usec = (tx_interval % 1000) * 1000;
	if (event_add(hardware->h_timer, &tv) == -1) {
		log_warnx("event", "unable to re-register timer event for port %s",
		    hardware->h_ifname);
		event_free(hardware->h_timer);
		hardware->h_timer = NULL;
		return;
	}
}

void
levent_schedule_pdu(struct lldpd_hardware *hardware)
{
	log_debug("event", "schedule sending PDU on %s",
	    hardware->h_ifname);
	if (hardware->h_timer == NULL) {
		hardware->h_timer = evtimer_new(hardware->h_cfg->g_base,
		    levent_send_pdu, hardware);
		if (hardware->h_timer == NULL) {
			log_warnx("event", "unable to schedule PDU sending for port %s",
			    hardware->h_ifname);
			return;
		}
	}

	struct timeval tv = { 0, 0 };
	if (event_add(hardware->h_timer, &tv) == -1) {
		log_warnx("event", "unable to register timer event for port %s",
		    hardware->h_ifname);
		event_free(hardware->h_timer);
		hardware->h_timer = NULL;
		return;
	}
}

int
levent_make_socket_nonblocking(int fd)
{
	int flags;
	if ((flags = fcntl(fd, F_GETFL, NULL)) < 0) {
		log_warn("event", "fcntl(%d, F_GETFL)", fd);
		return -1;
	}
	if (flags & O_NONBLOCK) return 0;
	if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
		log_warn("event", "fcntl(%d, F_SETFL)", fd);
		return -1;
	}
	return 0;
}

int
levent_make_socket_blocking(int fd)
{
	int flags;
	if ((flags = fcntl(fd, F_GETFL, NULL)) < 0) {
		log_warn("event", "fcntl(%d, F_GETFL)", fd);
		return -1;
	}
	if (!(flags & O_NONBLOCK)) return 0;
	if (fcntl(fd, F_SETFL, flags & ~O_NONBLOCK) == -1) {
		log_warn("event", "fcntl(%d, F_SETFL)", fd);
		return -1;
	}
	return 0;
}

#ifdef HOST_OS_LINUX
/* Receive and log error from a socket when there is suspicion of an error. */
void
levent_recv_error(int fd, const char *source)
{
	do {
		ssize_t n;
		char buf[1024] = {};
		struct msghdr msg = {
			.msg_control = buf,
			.msg_controllen = sizeof(buf)
		};
		if ((n = recvmsg(fd, &msg, MSG_ERRQUEUE | MSG_DONTWAIT)) <= 0) {
			return;
		}
		struct cmsghdr *cmsg = CMSG_FIRSTHDR(&msg);
		if (cmsg == NULL)
			log_warnx("event", "received unknown error on %s",
			    source);
		else
			log_warnx("event", "received error (level=%d/type=%d) on %s",
			    cmsg->cmsg_level, cmsg->cmsg_type, source);
	} while (1);
}
#endif
