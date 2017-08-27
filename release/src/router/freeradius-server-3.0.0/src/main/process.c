/*
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 */

/*
 * $Id$
 *
 * @file process.c
 * @brief Defines the state machines that control how requests are processed.
 *
 * @copyright 2012  The FreeRADIUS server project
 * @copyright 2012  Alan DeKok <aland@deployingradius.com>
 */

RCSID("$Id$")

#include <freeradius-devel/radiusd.h>
#include <freeradius-devel/process.h>
#include <freeradius-devel/modules.h>

#include <freeradius-devel/rad_assert.h>

#ifdef WITH_DETAIL
#include <freeradius-devel/detail.h>
#endif

#include <signal.h>
#include <fcntl.h>

#ifdef HAVE_SYS_WAIT_H
#	include <sys/wait.h>
#endif

extern pid_t radius_pid;
extern int check_config;
extern fr_cond_t *debug_condition;

static int spawn_flag = 0;
static int just_started = true;
time_t				fr_start_time;
static fr_packet_list_t *pl = NULL;
static fr_event_list_t *el = NULL;

static char const *action_codes[] = {
	"INVALID",
	"run",
	"done",
	"dup",
	"conflicting",
	"timer",
#ifdef WITH_PROXY
	"proxy-reply"
#endif
};

#ifdef DEBUG_STATE_MACHINE
#define TRACE_STATE_MACHINE if (debug_flag) printf("(%u) ********\tSTATE %s action %s live M%u C%u\t********\n", request->number, __FUNCTION__, action_codes[action], request->master_state, request->child_state)
#else
#define TRACE_STATE_MACHINE {}
#endif

/*
 *	Declare a state in the state machine.
 *
 */
#define STATE_MACHINE_DECL(_x) static void _x(REQUEST *request, int action)

#define STATE_MACHINE_TIMER(_x) request->timer_action = _x; \
		fr_event_insert(el, request_timer, request, \
				&when, &request->ev);



/**
 * @section request_timeline
 *
 *	Time sequence of a request
 * @code
 *
 *	RQ-----------------P=============================Y-J-C
 *	 ::::::::::::::::::::::::::::::::::::::::::::::::::::::::M
 * @endcode
 *
 * -	R: received.  Duplicate detection is done, and request is
 * 	   cached.
 *
 * -	Q: Request is placed onto a queue for child threads to pick up.
 *	   If there are no child threads, the request goes immediately
 *	   to P.
 *
 * -	P: Processing the request through the modules.
 *
 * -	Y: Reply is ready.  Rejects MAY be delayed here.  All other
 *	   replies are sent immediately.
 *
 * -	J: Reject is sent "reject_delay" after the reply is ready.
 *
 * -	C: For Access-Requests, After "cleanup_delay", the request is
 *	   deleted.  Accounting-Request packets go directly from Y to C.
 *
 * -	M: Max request time.  If the request hits this timer, it is
 *	   forcibly stopped.
 *
 *	Other considerations include duplicate and conflicting
 *	packets.  When a dupicate packet is received, it is ignored
 *	until we've reached Y, as no response is ready.  If the reply
 *	is a reject, duplicates are ignored until J, when we're ready
 *	to send the reply.  In between the reply being sent (Y or J),
 *	and C, the server responds to duplicates by sending the cached
 *	reply.
 *
 *	Conflicting packets are sent in 2 situations.
 *
 *	The first is in between R and Y.  In that case, we consider
 *	it as a hint that we're taking too long, and the NAS has given
 *	up on the request.  We then behave just as if the M timer was
 *	reached, and we discard the current request.  This allows us
 *	to process the new one.
 *
 *	The second case is when we're at Y, but we haven't yet
 *	finished processing the request.  This is a race condition in
 *	the threading code (avoiding locks is faster).  It means that
 *	a thread has actually encoded and sent the reply, and that the
 *	NAS has responded with a new packet.  The server can then
 *	safely mark the current request as "OK to delete", and behaves
 *	just as if the M timer was reached.  This usually happens only
 *	in high-load situations.
 *
 *	Duplicate packets are sent when the NAS thinks we're taking
 *	too long, and wants a reply.  From R-Y, duplicates are
 *	ignored.  From Y-J (for Access-Rejects), duplicates are also
 *	ignored.  From Y-C, duplicates get a duplicate reply.  *And*,
 *	they cause the "cleanup_delay" time to be extended.  This
 *	extension means that we're more likely to send a duplicate
 *	reply (if we have one), or to suppress processing the packet
 *	twice if we didn't reply to it.
 *
 *	All functions in this file should be thread-safe, and should
 *	assume thet the REQUEST structure is being accessed
 *	simultaneously by the main thread, and by the child worker
 *	threads.  This means that timers, etc. cannot be updated in
 *	the child thread.
 *
 *	Instead, the master thread periodically calls request->process
 *	with action TIMER.  It's up to the individual functions to
 *	determine how to handle that.  They need to check if they're
 *	being called from a child thread or the master, and then do
 *	different things based on that.
 */


#ifdef WITH_PROXY
static fr_packet_list_t *proxy_list = NULL;
#endif

#ifdef HAVE_PTHREAD_H
#ifdef WITH_PROXY
static pthread_mutex_t	proxy_mutex;
static rad_listen_t *proxy_listener_list = NULL;
static int proxy_no_new_sockets = false;
#endif

#define PTHREAD_MUTEX_LOCK if (spawn_flag) pthread_mutex_lock
#define PTHREAD_MUTEX_UNLOCK if (spawn_flag) pthread_mutex_unlock

static pthread_t NO_SUCH_CHILD_PID;
#else
/*
 *	This is easier than ifdef's throughout the code.
 */
#define PTHREAD_MUTEX_LOCK(_x)
#define PTHREAD_MUTEX_UNLOCK(_x)
#endif

/*
 *	We need mutexes around the event FD list *only* in certain
 *	cases.
 */
#if defined (HAVE_PTHREAD_H) && (defined(WITH_PROXY) || defined(WITH_TCP))
static pthread_mutex_t	fd_mutex;
#define FD_MUTEX_LOCK if (spawn_flag) pthread_mutex_lock
#define FD_MUTEX_UNLOCK if (spawn_flag) pthread_mutex_unlock
#else
/*
 *	This is easier than ifdef's throughout the code.
 */
#define FD_MUTEX_LOCK(_x)
#define FD_MUTEX_UNLOCK(_x)
#endif

static int request_num_counter = 0;
#ifdef WITH_PROXY
static int request_will_proxy(REQUEST *request);
static int request_proxy(REQUEST *request, int retransmit);
STATE_MACHINE_DECL(proxy_wait_for_reply);
STATE_MACHINE_DECL(proxy_running);
static int process_proxy_reply(REQUEST *request);
static void remove_from_proxy_hash(REQUEST *request);
static void remove_from_proxy_hash_nl(REQUEST *request, bool yank);
static int insert_into_proxy_hash(REQUEST *request);
#endif

static REQUEST *request_setup(rad_listen_t *listener, RADIUS_PACKET *packet,
			      RADCLIENT *client, RAD_REQUEST_FUNP fun);

STATE_MACHINE_DECL(request_common);

#if  defined(HAVE_PTHREAD_H) && !defined (NDEBUG)
static int we_are_master(void)
{
	if (spawn_flag &&
	    (pthread_equal(pthread_self(), NO_SUCH_CHILD_PID) == 0)) {
		return 0;
	}

	return 1;
}
#define ASSERT_MASTER 	if (!we_are_master()) rad_panic("We are not master")

#else
#define we_are_master(_x) (1)
#define ASSERT_MASTER
#endif

STATE_MACHINE_DECL(request_reject_delay);
STATE_MACHINE_DECL(request_cleanup_delay);
STATE_MACHINE_DECL(request_running);
#ifdef WITH_COA
static void request_coa_timer(REQUEST *request);
static void request_coa_originate(REQUEST *request);
STATE_MACHINE_DECL(request_coa_process);
static void request_coa_separate(REQUEST *coa);
#endif

#undef USEC
#define USEC (1000000)

#define INSERT_EVENT(_function, _ctx) if (!fr_event_insert(el, _function, _ctx, &((_ctx)->when), &((_ctx)->ev))) { _rad_panic(__FILE__, __LINE__, "Failed to insert event"); }

static void _rad_panic(char const *file, unsigned int line, char const *msg)
{
	ERROR("[%s:%d] %s", file, line, msg);
#ifndef NDEBUG
	rad_assert(0 == 1);
#endif
	fr_exit(1);
}

#define rad_panic(x) _rad_panic(__FILE__, __LINE__, x)

static void tv_add(struct timeval *tv, int usec_delay)
{
	if (usec_delay >= USEC) {
		tv->tv_sec += usec_delay / USEC;
		usec_delay %= USEC;
	}
	tv->tv_usec += usec_delay;

	if (tv->tv_usec >= USEC) {
		tv->tv_sec += tv->tv_usec / USEC;
		tv->tv_usec %= USEC;
	}
}

/*
 *	In daemon mode, AND this request has debug flags set.
 */
#define DEBUG_PACKET if (!debug_flag && request->options && request->radlog) debug_packet

static void debug_packet(REQUEST *request, RADIUS_PACKET *packet, int direction)
{
	vp_cursor_t cursor;
	VALUE_PAIR *vp;
	char buffer[1024];
	char const *received, *from;
	fr_ipaddr_t const *ip;
	int port;

	if (!packet) return;

	rad_assert(request->radlog != NULL);

	if (direction == 0) {
		received = "Received";
		from = "from";	/* what else? */
		ip = &packet->src_ipaddr;
		port = packet->src_port;

	} else {
		received = "Sending";
		from = "to";	/* hah! */
		ip = &packet->dst_ipaddr;
		port = packet->dst_port;
	}

	/*
	 *	Client-specific debugging re-prints the input
	 *	packet into the client log.
	 *
	 *	This really belongs in a utility library
	 */
	if ((packet->code > 0) && (packet->code < FR_MAX_PACKET_CODE)) {
		RDEBUG("%s %s packet %s host %s port %d, id=%d, length=%d",
		       received, fr_packet_codes[packet->code], from,
		       inet_ntop(ip->af, &ip->ipaddr, buffer, sizeof(buffer)),
		       port, packet->id, packet->data_len);
	} else {
		RDEBUG("%s packet %s host %s port %d code=%d, id=%d, length=%d",
		       received, from,
		       inet_ntop(ip->af, &ip->ipaddr, buffer, sizeof(buffer)),
		       port,
		       packet->code, packet->id, packet->data_len);
	}

	for (vp = paircursor(&cursor, &packet->vps);
	     vp;
	     vp = pairnext(&cursor)) {
		vp_prints(buffer, sizeof(buffer), vp);
		RDEBUG("\t%s", buffer);
	}
}


/***********************************************************************
 *
 *	Start of RADIUS server state machine.
 *
 ***********************************************************************/

/*
 *	Callback for ALL timer events related to the request.
 */
static void request_timer(void *ctx)
{
	REQUEST *request = ctx;
	int action = request->timer_action;

	TRACE_STATE_MACHINE;

	request->process(request, action);
}

#define USEC (1000000)

/*
 *	Only ever called from the master thread.
 */
STATE_MACHINE_DECL(request_done)
{
	struct timeval now, when;

	TRACE_STATE_MACHINE;

#ifdef WITH_COA
	/*
	 *	CoA requests can be cleaned up in the child thread,
	 *	but ONLY if they aren't tied into anything.
	 */
	if (request->parent && (request->parent->coa == request)) {
		rad_assert(!request->in_request_hash);
		rad_assert(!request->in_proxy_hash);
		rad_assert(action == FR_ACTION_DONE);
		rad_assert(request->ev == NULL);
	}
#endif

#ifdef HAVE_PTHREAD_H
	/*
	 *	If called from a child thread, mark ourselves as done,
	 *	and wait for the master thread timer to clean us up.
	 */
	if (!we_are_master()) {
		request->child_state = REQUEST_DONE;
		request->child_pid = NO_SUCH_CHILD_PID;;
		return;
	}
#endif

#ifdef WITH_COA
	/*
	 *	Move the CoA request to its own handler.
	 */
	if (request->coa) request_coa_separate(request->coa);

	/*
	 *	If we're the CoA request, make the parent forget about
	 *	us.
	 */
	if (request->parent && (request->parent->coa == request)) {
		request->parent->coa = NULL;
	}

#endif

	/*
	 *	It doesn't hurt to send duplicate replies.  All other
	 *	signals are ignored, as the request will be cleaned up
	 *	soon anyways.
	 */
	switch (action) {
	case FR_ACTION_DUP:
		if (request->reply->code != 0) {
			request->listener->send(request->listener, request);
			return;
		}
		break;

		/*
		 *	This is only called from the master thread
		 *	when there is a child thread processing the
		 *	request.
		 */
	case FR_ACTION_CONFLICTING:
		if (request->child_state == REQUEST_DONE) break;

		/*
		 *	If there's a reply packet, then we presume
		 *	that the child has sent the reply, and we get
		 *	pinged here before the child has a chance to
		 *	say "I'm done!"
		 */
		if (request->reply->data) break;

		RERROR("Received conflicting packet from "
			       "client %s port %d - ID: %d due to "
			       "unfinished request.  Giving up on old request.",
			       request->client->shortname,
			       request->packet->src_port, request->packet->id);
		break;

		/*
		 *	Called only when there's an error remembering
		 *	the packet, or when the socket gets closed from
		 *	under us.
		 */
	case FR_ACTION_DONE:
#ifdef HAVE_PTHREAD_H
		/*
		 *	Do NOT set child_state to DONE if it's still in the queue.
		 */
		if (we_are_master() && (request->child_state == REQUEST_QUEUED)) {
			break;
		}

		/*
		 *	If we have child threads and we're NOT the
		 *	thread handling the request, don't do anything.
		 */
		if (spawn_flag &&
		    !pthread_equal(pthread_self(), request->child_pid)) {
			break;
		}
#endif
#ifdef DEBUG_STATE_MACHINE
		if (debug_flag) printf("(%u) ********\tSTATE %s C%u -> C%u\t********\n", request->number, __FUNCTION__, request->child_state, REQUEST_DONE);
#endif
		request->child_state = REQUEST_DONE;
		break;

		/*
		 *	Called when the child is taking too long to
		 *	finish.  We've already marked it "please
		 *	stop", so we don't complain any more.
		 */
	case FR_ACTION_TIMER:
		break;

#ifdef WITH_PROXY
		/*
		 *	Child is still alive, and we're receiving more
		 *	packets from the home server.
		 */
	case FR_ACTION_PROXY_REPLY:
		request_common(request, action);
		break;
#endif

	default:
		RDEBUG3("%s: Ignoring action %s", __FUNCTION__, action_codes[action]);
		break;
	}

	/*
	 *	Remove it from the request hash.
	 */
	if (request->in_request_hash) {
		ASSERT_MASTER;
		if (!fr_packet_list_yank(pl, request->packet)) {
			rad_assert(0 == 1);
		}
		request->in_request_hash = false;

		/*
		 *	@todo: do final states for TCP sockets, too?
		 */
		request_stats_final(request);

#ifdef WITH_TCP
		request->listener->count--;
#endif
	}

#ifdef WITH_PROXY
	/*
	 *	Wait for the proxy ID to expire.  This allows us to
	 *	avoid re-use of proxy IDs for a while.
	 */
	if (request->in_proxy_hash) {
		rad_assert(request->proxy != NULL);

		fr_event_now(el, &now);
		when = request->proxy->timestamp;

#ifdef WITH_COA
		if (((request->proxy->code == PW_COA_REQUEST) ||
		     (request->proxy->code == PW_DISCONNECT_REQUEST)) &&
		    (request->packet->code != request->proxy->code)) {
			when.tv_sec += request->home_server->coa_mrd;
		} else
#endif
		when.tv_sec += request->home_server->response_window;

		/*
		 *	We haven't received all responses, AND there's still
		 *	time to wait.  Do so.
		 */
		if ((request->num_proxied_requests > request->num_proxied_responses) &&
#ifdef WITH_TCP
		    (request->home_server->proto != IPPROTO_TCP) &&
#endif
		    timercmp(&now, &when, <)) {
			RDEBUG("Waiting for more responses from the home server");
			goto wait_some_more;
		}

		/*
		 *	Time to remove it.
		 */
		remove_from_proxy_hash(request);
	}
#endif

	if (request->child_state != REQUEST_DONE) {

#ifdef HAVE_PTHREAD_H
		if (!spawn_flag)
#endif
		{
			rad_panic("Request should have been marked done");
		}

		gettimeofday(&now, NULL);
#ifdef WITH_PROXY
	wait_some_more:
#endif

#ifdef HAVE_PTHREAD_H
		if (spawn_flag &&
		    (pthread_equal(request->child_pid, NO_SUCH_CHILD_PID) == 0)) {
			RDEBUG("Waiting for child thread to stop");
		}
#endif

		when = now;
		tv_add(&when, request->delay);
		request->delay += request->delay >> 1;
		if (request->delay > (10 * USEC)) request->delay = 10 * USEC;

		STATE_MACHINE_TIMER(FR_ACTION_TIMER);
		return;
	}

#ifdef HAVE_PTHREAD_H
	rad_assert(request->child_pid == NO_SUCH_CHILD_PID);
#endif

	if (request->packet) {
		RDEBUG2("Cleaning up request packet ID %d with timestamp +%d",
			request->packet->id,
			(unsigned int) (request->timestamp - fr_start_time));
	} /* else don't print anything */

	if (request->ev) fr_event_delete(el, &request->ev);

	request_free(&request);
}


static void request_cleanup_delay_init(REQUEST *request, struct timeval const *pnow)
{
	struct timeval now, when;

	if (request->packet->code == PW_ACCOUNTING_REQUEST) goto done;
	if (!request->root->cleanup_delay) goto done;

	if (pnow) {
		now = *pnow;
	} else {
		gettimeofday(&now, NULL);
	}

	rad_assert(request->reply->timestamp.tv_sec != 0);
	when = request->reply->timestamp;

	request->delay = request->root->cleanup_delay;
	when.tv_sec += request->delay;

	/*
	 *	Set timer for when we need to clean it up.
	 */
	if (timercmp(&when, &now, >)) {
#ifdef DEBUG_STATE_MACHINE
		if (debug_flag) printf("(%u) ********\tNEXT-STATE %s -> %s\n", request->number, __FUNCTION__, "request_cleanup_delay");
#endif
		request->process = request_cleanup_delay;
		request->child_state = REQUEST_DONE;
		STATE_MACHINE_TIMER(FR_ACTION_TIMER);
		return;
	}

	/*
	 *	Otherwise just clean it up.
	 */
done:
	request_done(request, FR_ACTION_DONE);
}


/*
 *	Function to do all time-related events.
 */
static void request_process_timer(REQUEST *request)
{
	struct timeval now, when;
	rad_assert(request->magic == REQUEST_MAGIC);
#ifdef DEBUG_STATE_MACHINE
	int action = FR_ACTION_TIMER;
#endif

	TRACE_STATE_MACHINE;
	ASSERT_MASTER;

#ifdef WITH_COA
	/*
	 *	If we originated a CoA request, divorce it from the
	 *	parent.  Then, set up the timers so that we can clean
	 *	it up as appropriate.
	 */
	if (request->coa) request_coa_separate(request->coa);

	/*
	 *	Check request stuff ONLY if we're running the request.
	 */
	if (!request->proxy || (request->packet->code == request->proxy->code))
#endif
	{
		rad_assert(request->listener != NULL);

		/*
		 *	The socket was closed.  Tell the request that
		 *	there is no point in continuing.
		 */
		if (request->listener->status != RAD_LISTEN_STATUS_KNOWN) {
			WDEBUG("Socket was closed while processing request %u: Stopping it.", request->number);
			goto done;
		}
	}

	gettimeofday(&now, NULL);

	/*
	 *	A child thread is still working on the request,
	 *	OR it was proxied, and there was no response,
	 *	OR it was sitting in the queue for too long.
	 */
	if ((request->child_state != REQUEST_DONE) &&
	    (request->master_state != REQUEST_STOP_PROCESSING)) {
		when = request->packet->timestamp;
		when.tv_sec += request->root->max_request_time;

		/*
		 *	Taking too long: tell it to die.
		 */
		if (timercmp(&now, &when, >=)) {
#ifdef HAVE_PTHREAD_H
			/*
			 *	If there's a child thread processing it,
			 *	complain.
			 */
			if (spawn_flag &&
			    (pthread_equal(request->child_pid, NO_SUCH_CHILD_PID) == 0)) {
				ERROR("Unresponsive child for request %u, in component %s module %s",
				       request->number,
				       request->component ? request->component : "<core>",
			       request->module ? request->module : "<core>");
				exec_trigger(request, NULL, "server.thread.unresponsive", true);
			}
#endif

			/*
			 *	Tell the request to stop it.
			 */
			goto done;
		} /* else we're not at max_request_time */

#ifdef WITH_PROXY
		if ((request->master_state != REQUEST_STOP_PROCESSING) &&
		    request->proxy &&
		    (request->process == request_running)) {
#ifdef DEBUG_STATE_MACHINE
			if (debug_flag) printf("(%u) ********\tNEXT-STATE %s -> %s\n", request->number, __FUNCTION__, "request_proxied");
#endif
			request->process = proxy_wait_for_reply;
		}
#endif

		/*
		 *	Wake up again in the future, to check for
		 *	more things to do.
		 */
		when = now;
		tv_add(&when, request->delay);
		request->delay += request->delay >> 1;

		STATE_MACHINE_TIMER(FR_ACTION_TIMER);
		return;
	}

#ifdef WITH_ACCOUNTING
	if (request->reply->code == PW_ACCOUNTING_RESPONSE) {
	done:
		request_done(request, FR_ACTION_DONE);
		return;
	}
#endif

#ifdef WITH_COA
	if (!request->proxy || (request->packet->code == request->proxy->code))
#endif

	if ((request->reply->code == PW_AUTHENTICATION_REJECT) &&
	    (request->root->reject_delay)) {
		rad_assert(request->reply->timestamp.tv_sec != 0);

		when = request->reply->timestamp;
		when.tv_sec += request->root->reject_delay;

		/*
		 *	Set timer for when we need to send it.
		 */
		if (timercmp(&when, &now, >)) {
#ifdef DEBUG_STATE_MACHINE
			if (debug_flag) printf("(%u) ********\tNEXT-STATE %s -> %s\n", request->number, __FUNCTION__, "request_reject_delay");
#endif
			request->process = request_reject_delay;

			STATE_MACHINE_TIMER(FR_ACTION_TIMER);
			return;
		}

		if (request->process == request_reject_delay) {
			/*
			 *	Assume we're at (or near) the reject
			 *	delay time.
			 */
			request->reply->timestamp = now;

			RDEBUG2("Sending delayed reject");
			DEBUG_PACKET(request, request->reply, 1);
			request->process = request_cleanup_delay;
			request->listener->send(request->listener, request);
		}
	}

	/*
	 *	The cleanup_delay is zero for accounting packets, and
	 *	enforced for all other packets.  We do the
	 *	cleanup_delay even if we don't respond to the NAS, so
	 *	that any retransmit is *not* processed as a new packet.
	 */
	request_cleanup_delay_init(request, &now);
	return;
}

static void request_queue_or_run(UNUSED REQUEST *request,
				 fr_request_process_t process)
{
	struct timeval when;
#ifdef DEBUG_STATE_MACHINE
	int action = FR_ACTION_TIMER;
#endif

	TRACE_STATE_MACHINE;
	ASSERT_MASTER;

	/*
	 *	(re) set the initial delay.
	 */
	request->delay = USEC / 3;
	gettimeofday(&when, NULL);
	tv_add(&when, request->delay);
	request->delay += request->delay >> 1;

	STATE_MACHINE_TIMER(FR_ACTION_TIMER);

	/*
	 *	Do this here so that fewer other functions need to do
	 *	it.
	 */
	if (request->master_state == REQUEST_STOP_PROCESSING) {
#ifdef DEBUG_STATE_MACHINE
		if (debug_flag) printf("(%u) ********\tSTATE %s C%u -> C%u\t********\n", request->number, __FUNCTION__, request->child_state, REQUEST_DONE);
#endif
		request_done(request, FR_ACTION_DONE);
		return;
	}

	request->process = process;

#ifdef HAVE_PTHREAD_H
	if (spawn_flag) {
		/*
		 *	A child thread will eventually pick it up.
		 */
		if (request_enqueue(request)) return;

		/*
		 *	Otherwise we're not going to do anything with
		 *	it...
		 */
		request_done(request, FR_ACTION_DONE);
		return;

	} else
#endif
	{
		request->process(request, FR_ACTION_RUN);

#ifdef WNOHANG
		/*
		 *	Requests that care about child process exit
		 *	codes have already either called
		 *	rad_waitpid(), or they've given up.
		 */
		while (waitpid(-1, NULL, WNOHANG) > 0);
#endif
	}
}

STATE_MACHINE_DECL(request_common)
{
#ifdef WITH_PROXY
	char buffer[128];
#endif

	TRACE_STATE_MACHINE;

	switch (action) {
	case FR_ACTION_DUP:
#ifdef WITH_PROXY
		if ((request->master_state != REQUEST_STOP_PROCESSING) &&
		     request->proxy && !request->proxy_reply) {
			/*
			 *	TODO: deal with this in a better way?
			 */
			proxy_wait_for_reply(request, action);
			return;
		}
#endif
		ERROR("(%u) Discarding duplicate request from "
		       "client %s port %d - ID: %u due to unfinished request",
		       request->number, request->client->shortname,
		       request->packet->src_port,request->packet->id);
		break;

	case FR_ACTION_CONFLICTING:
		/*
		 *	We're in the master thread, ask the child to
		 *	stop processing the request.
		 */
		request_done(request, action);
		return;

	case FR_ACTION_TIMER:
		request_process_timer(request);
		return;

#ifdef WITH_PROXY
	case FR_ACTION_PROXY_REPLY:
		DEBUG2("Reply from home server %s port %d  - ID: %d arrived too late for request %u. Try increasing 'retry_delay' or 'max_request_time'",
		       inet_ntop(request->proxy->src_ipaddr.af,
				 &request->proxy->src_ipaddr.ipaddr,
				 buffer, sizeof(buffer)),
		       request->proxy->dst_port, request->proxy->id,
		       request->number);
		return;
#endif

	default:
		RDEBUG3("%s: Ignoring action %s", __FUNCTION__, action_codes[action]);
		break;
	}
}

STATE_MACHINE_DECL(request_cleanup_delay)
{
	struct timeval when;

	TRACE_STATE_MACHINE;
	ASSERT_MASTER;

	switch (action) {
	case FR_ACTION_DUP:
		if (request->reply->code != 0) {
			request->listener->send(request->listener, request);
		} else {
			RDEBUG("No reply.  Ignoring retransmit.");
		}

		/*
		 *	Double the cleanup_delay to catch retransmits.
		 */
		when = request->reply->timestamp;
		request->delay += request->delay ;
		when.tv_sec += request->delay;

		STATE_MACHINE_TIMER(FR_ACTION_TIMER);
		return;

#ifdef WITH_PROXY
	case FR_ACTION_PROXY_REPLY:
#endif
	case FR_ACTION_CONFLICTING:
	case FR_ACTION_TIMER:
		request_common(request, action);
		return;

	default:
		RDEBUG3("%s: Ignoring action %s", __FUNCTION__, action_codes[action]);
		break;
	}
}

STATE_MACHINE_DECL(request_reject_delay)
{
	TRACE_STATE_MACHINE;
	ASSERT_MASTER;

	switch (action) {
	case FR_ACTION_DUP:
		ERROR("(%u) Discarding duplicate request from "
		       "client %s port %d - ID: %u due to delayed reject",
		       request->number, request->client->shortname,
		       request->packet->src_port,request->packet->id);
		return;

#ifdef WITH_PROXY
	case FR_ACTION_PROXY_REPLY:
#endif
	case FR_ACTION_CONFLICTING:
	case FR_ACTION_TIMER:
		request_common(request, action);
		break;

	default:
		RDEBUG3("%s: Ignoring action %s", __FUNCTION__, action_codes[action]);
		break;
	}
}


static int request_pre_handler(REQUEST *request, UNUSED int action)
{
	TRACE_STATE_MACHINE;

	int rcode;

	if (request->master_state == REQUEST_STOP_PROCESSING) return 0;

	/*
	 *	Don't decode the packet if it's an internal "fake"
	 *	request.  Instead, just return so that the caller can
	 *	process it.
	 */
	if (request->packet->dst_port == 0) {
		request->username = pairfind(request->packet->vps, PW_USER_NAME, 0, TAG_ANY);
		request->password = pairfind(request->packet->vps, PW_USER_PASSWORD, 0, TAG_ANY);
		return 1;
	}

#ifdef WITH_PROXY
	/*
	 *	Put the decoded packet into it's proper place.
	 */
	if (request->proxy_reply != NULL) {
		/*
		 *	There may be a proxy reply, but it may be too late.
		 */
		if (!request->proxy_listener) return 0;

		rcode = request->proxy_listener->decode(request->proxy_listener, request);
		DEBUG_PACKET(request, request->proxy_reply, 0);

		/*
		 *	Pro-actively remove it from the proxy hash.
		 *	This is later than in 2.1.x, but it means that
		 *	the replies are authenticated before being
		 *	removed from the hash.
		 */
		if ((rcode == 0) &&
		    (request->num_proxied_requests <= request->num_proxied_responses)) {
			remove_from_proxy_hash(request);
		}

	} else
#endif
	if (request->packet->vps == NULL) {
		rcode = request->listener->decode(request->listener, request);

#ifdef WITH_UNLANG
		if (debug_condition) {
			/*
			 *	Ignore parse errors.
			 */
			if (radius_evaluate_cond(request, RLM_MODULE_OK, 0, debug_condition)) {
				request->options = 2;
				request->radlog = radlog_request;
			}
		}
#endif

		DEBUG_PACKET(request, request->packet, 0);
	} else {
		rcode = 0;
	}

	if (rcode < 0) {
		RDEBUG("Dropping packet without response because of error: %s", fr_strerror());
		request->reply->offset = -2; /* bad authenticator */
		return 0;
	}

	if (!request->username) {
		request->username = pairfind(request->packet->vps, PW_USER_NAME, 0, TAG_ANY);
	}

#ifdef WITH_PROXY
	if (action == FR_ACTION_PROXY_REPLY) {
		return process_proxy_reply(request);
	}
#endif

	return 1;
}

STATE_MACHINE_DECL(request_finish)
{
	VALUE_PAIR *vp;

	TRACE_STATE_MACHINE;

	(void) action;	/* -Wunused */

	if (request->master_state == REQUEST_STOP_PROCESSING) return;

	/*
	 *	Don't send replies if there are none to send.
	 */
	if (!request->in_request_hash) return;

	/*
	 *	Catch Auth-Type := Reject BEFORE proxying the packet.
	 */
	if (request->packet->code == PW_AUTHENTICATION_REQUEST) {
		/*
		 *	Override the response code if a
		 *	control:Response-Packet-Type attribute is present.
		 */
		vp = pairfind(request->config_items, PW_RESPONSE_PACKET_TYPE, 0, TAG_ANY);
		if (vp) {
			if (vp->vp_integer == 256) {
				RDEBUG2("Not responding to request");

				request->reply->code = 0;
			} else {
				request->reply->code = vp->vp_integer;
			}
		} else if (request->reply->code == 0) {
			vp = pairfind(request->config_items, PW_AUTH_TYPE, 0, TAG_ANY);

			if (!vp || (vp->vp_integer != PW_AUTHENTICATION_REJECT)) {
				RDEBUG2("There was no response configured: "
					"rejecting request");
			}

			request->reply->code = PW_AUTHENTICATION_REJECT;
		}
	}

	/*
	 *	Copy Proxy-State from the request to the reply.
	 */
	vp = paircopy2(request->reply, request->packet->vps,
		       PW_PROXY_STATE, 0, TAG_ANY);
	if (vp) pairadd(&request->reply->vps, vp);

	switch (request->reply->code) {
	case PW_AUTHENTICATION_ACK:
		rad_postauth(request);
		break;
	case PW_ACCESS_CHALLENGE:
		pairdelete(&request->config_items, PW_POST_AUTH_TYPE, 0,
			   TAG_ANY);
		vp = pairmake_config("Post-Auth-Type", "Challenge", T_OP_SET);
		if (vp) rad_postauth(request);
		break;
	default:
		break;
	}

	/*
	 *	Run rejected packets through
	 *
	 *	Post-Auth-Type = Reject
	 *
	 *	We do this separately so ACK and challenge can change the code
	 *	to reject if a module returns reject.
	 */
	if (request->reply->code == PW_AUTHENTICATION_REJECT) {
		pairdelete(&request->config_items, PW_POST_AUTH_TYPE, 0, TAG_ANY);
		vp = pairmake_config("Post-Auth-Type", "Reject", T_OP_SET);
		if (vp) rad_postauth(request);
	}

	/*
	 *	Send the reply here.
	 */
	if ((request->reply->code != PW_AUTHENTICATION_REJECT) ||
	    (request->root->reject_delay == 0)) {
		DEBUG_PACKET(request, request->reply, 1);
		request->listener->send(request->listener,
					request);
		pairfree(&request->reply->vps);
	}

	/*
	 *	Clean up.  These are no longer needed.
	 */
	pairfree(&request->config_items);

	pairfree(&request->packet->vps);
	request->username = NULL;
	request->password = NULL;

#ifdef WITH_PROXY
	if (request->proxy) {
		pairfree(&request->proxy->vps);
	}
	if (request->proxy_reply) {
		pairfree(&request->proxy_reply->vps);
	}
#endif

	RDEBUG2("Finished request %u.", request->number);
}

STATE_MACHINE_DECL(request_running)
{
	TRACE_STATE_MACHINE;

	switch (action) {
	case FR_ACTION_CONFLICTING:
	case FR_ACTION_DUP:
	case FR_ACTION_TIMER:
		request_common(request, action);
		return;

#ifdef WITH_PROXY
	case FR_ACTION_PROXY_REPLY:
#ifdef HAVE_PTHREAD_H
		/*
		 *	Catch the case of a proxy reply when called
		 *	from the main worker thread.
		 */
		if (we_are_master() &&
		    (request->process != proxy_running)) {
			request_queue_or_run(request, proxy_running);
			return;
		}
		/* FALL-THROUGH */
#endif
#endif

	case FR_ACTION_RUN:
		if (!request_pre_handler(request, action)) goto done;

		rad_assert(request->handle != NULL);
		request->handle(request);

#ifdef WITH_PROXY
		/*
		 *	We may need to send a proxied request.
		 */
		if ((action == FR_ACTION_RUN) &&
		    request_will_proxy(request)) {
#ifdef DEBUG_STATE_MACHINE
			if (debug_flag) printf("(%u) ********\tWill Proxy\t********\n", request->number);
#endif
			/*
			 *	If this fails, it
			 *	takes care of setting
			 *	up the post proxy fail
			 *	handler.
			 */
			if (request_proxy(request, 0) < 0) goto finished;
		} else
#endif
		{
#ifdef DEBUG_STATE_MACHINE
			if (debug_flag) printf("(%u) ********\tFinished\t********\n", request->number);
#endif

#ifdef WITH_COA
			/*
			 *	Maybe originate a CoA request.
			 */
			if ((action == FR_ACTION_RUN) && request->coa) {
				request_coa_originate(request);
			}
#endif

		finished:
			request_finish(request, action);

		done:
			/*
			 *	Get the time of the reply, which is
			 *	when we're done.
			 */
			gettimeofday(&request->reply->timestamp, NULL);

#ifdef DEBUG_STATE_MACHINE
			if (debug_flag) printf("(%u) ********\tSTATE %s C%u -> C%u\t********\n", request->number, __FUNCTION__, request->child_state, REQUEST_DONE);
#endif

#ifdef HAVE_PTHREAD_H
			request->child_pid = NO_SUCH_CHILD_PID;
#endif
			request->child_state = REQUEST_DONE;
		}
		break;

	default:
		RDEBUG3("%s: Ignoring action %s", __FUNCTION__, action_codes[action]);
		break;
	}
}

int request_receive(rad_listen_t *listener, RADIUS_PACKET *packet,
		    RADCLIENT *client, RAD_REQUEST_FUNP fun)
{
	int count;
	RADIUS_PACKET **packet_p;
	REQUEST *request = NULL;
	struct timeval now;
	listen_socket_t *sock = listener->data;

	/*
	 *	Set the last packet received.
	 */
	gettimeofday(&now, NULL);
	sock->last_packet = now.tv_sec;
	packet->timestamp = now;

	/*
	 *	Skip everything if required.
	 */
	if (listener->nodup) goto skip_dup;

	packet_p = fr_packet_list_find(pl, packet);
	if (packet_p) {
		request = fr_packet2myptr(REQUEST, packet, packet_p);
		rad_assert(request->in_request_hash);

		/*
		 *	Same src/dst ip/port, length, and
		 *	authentication vector: must be a duplicate.
		 */
		if ((request->packet->data_len == packet->data_len) &&
		    (memcmp(request->packet->vector, packet->vector,
			    sizeof(packet->vector)) == 0)) {

#ifdef WITH_STATS
			switch (packet->code) {
			case PW_AUTHENTICATION_REQUEST:
				FR_STATS_INC(auth, total_dup_requests);
				break;

#ifdef WITH_ACCOUNTING
			case PW_ACCOUNTING_REQUEST:
				FR_STATS_INC(acct, total_dup_requests);
				break;
#endif
#ifdef WITH_COA
			case PW_COA_REQUEST:
				FR_STATS_INC(coa, total_dup_requests);
				break;

			case PW_DISCONNECT_REQUEST:
				FR_STATS_INC(dsc, total_dup_requests);
				break;
#endif

			default:
			  break;
			}
#endif	/* WITH_STATS */

			request->process(request, FR_ACTION_DUP);
			return 0;
		}

		/*
		 *	Say we're ignoring the old one, and continue
		 *	to process the new one.
		 */
		request->process(request, FR_ACTION_CONFLICTING);
		request = NULL;
	}

	/*
	 *	Quench maximum number of outstanding requests.
	 */
	if (mainconfig.max_requests &&
	    ((count = fr_packet_list_num_elements(pl)) > mainconfig.max_requests)) {
		ERROR("Dropping request (%d is too many): from client %s port %d - ID: %d", count,
		       client->shortname,
		       packet->src_port, packet->id);
		WARN("Please check the configuration file.\n"
		     "\tThe value for 'max_requests' is probably set too low.\n");

		exec_trigger(NULL, NULL, "server.max_requests", true);
		return 0;
	}

skip_dup:
	/*
	 *	Rate-limit the incoming packets
	 */
	if (sock->max_rate) {
		int pps;

		pps = rad_pps(&sock->rate_pps_old, &sock->rate_pps_now,
			      &sock->rate_time, &now);

		if (pps > sock->max_rate) {
			DEBUG("Dropping request due to rate limiting");
			return 0;
		}
		sock->rate_pps_now++;
	}

	request = request_setup(listener, packet, client, fun);
	if (!request) return 1;

	/*
	 *	Remember the request in the list.
	 */
	if (!listener->nodup) {
		if (!fr_packet_list_insert(pl, &request->packet)) {
			RERROR("Failed to insert request in the list of live requests: discarding it");
			request_done(request, FR_ACTION_DONE);
			return 1;
		}

		request->in_request_hash = true;
	}

	/*
	 *	Process it.  Send a response, and free it.
	 */
	if (listener->synchronous) {
		request->listener->decode(request->listener, request);
		request->username = pairfind(request->packet->vps, PW_USER_NAME, 0, TAG_ANY);
		request->password = pairfind(request->packet->vps, PW_USER_PASSWORD, 0, TAG_ANY);

		fun(request);
		request->listener->send(request->listener, request);
		request_free(&request);
		return 1;
	}

	/*
	 *	Otherwise, insert it into the state machine.
	 *	The child threads will take care of processing it.
	 */
	request_queue_or_run(request, request_running);

	return 1;
}


static REQUEST *request_setup(rad_listen_t *listener, RADIUS_PACKET *packet,
			      RADCLIENT *client, RAD_REQUEST_FUNP fun)
{
	REQUEST *request;

	/*
	 *	Create and initialize the new request.
	 */
	request = request_alloc(listener);
	request->reply = rad_alloc(request, 0);
	if (!request->reply) {
		ERROR("No memory");
		request_free(&request);
		return NULL;
	}

	request->listener = listener;
	request->client = client;
	request->packet = talloc_steal(request, packet);
	request->number = request_num_counter++;
	request->priority = listener->type;
	request->master_state = REQUEST_ACTIVE;
#ifdef DEBUG_STATE_MACHINE
	if (debug_flag) printf("(%u) ********\tSTATE %s C%u -> C%u\t********\n", request->number, __FUNCTION__, request->child_state, REQUEST_ACTIVE);
#endif
	request->child_state = REQUEST_ACTIVE;
	request->handle = fun;
#ifdef HAVE_PTHREAD_H
	request->child_pid = NO_SUCH_CHILD_PID;
#endif

#ifdef WITH_STATS
	request->listener->stats.last_packet = request->packet->timestamp.tv_sec;
	if (packet->code == PW_AUTHENTICATION_REQUEST) {
		request->client->auth.last_packet = request->packet->timestamp.tv_sec;
		radius_auth_stats.last_packet = request->packet->timestamp.tv_sec;
#ifdef WITH_ACCOUNTING
	} else if (packet->code == PW_ACCOUNTING_REQUEST) {
		request->client->acct.last_packet = request->packet->timestamp.tv_sec;
		radius_acct_stats.last_packet = request->packet->timestamp.tv_sec;
#endif
	}
#endif	/* WITH_STATS */

	/*
	 *	Status-Server packets go to the head of the queue.
	 */
	if (request->packet->code == PW_STATUS_SERVER) request->priority = 0;

	/*
	 *	Set virtual server identity
	 */
	if (client->server) {
		request->server = client->server;
	} else if (listener->server) {
		request->server = listener->server;
	} else {
		request->server = NULL;
	}

	request->root = &mainconfig;
#ifdef WITH_TCP
	request->listener->count++;
#endif

	/*
	 *	The request passes many of our sanity checks.
	 *	From here on in, if anything goes wrong, we
	 *	send a reject message, instead of dropping the
	 *	packet.
	 */

	/*
	 *	Build the reply template from the request.
	 */

	request->reply->sockfd = request->packet->sockfd;
	request->reply->dst_ipaddr = request->packet->src_ipaddr;
	request->reply->src_ipaddr = request->packet->dst_ipaddr;
	request->reply->dst_port = request->packet->src_port;
	request->reply->src_port = request->packet->dst_port;
	request->reply->id = request->packet->id;
	request->reply->code = 0; /* UNKNOWN code */
	memcpy(request->reply->vector, request->packet->vector,
	       sizeof(request->reply->vector));
	request->reply->vps = NULL;
	request->reply->data = NULL;
	request->reply->data_len = 0;

	return request;
}

#ifdef WITH_TCP
#ifdef WITH_PROXY
/***********************************************************************
 *
 *	TCP Handlers.
 *
 ***********************************************************************/

/*
 *	Timer function for all TCP sockets.
 */
static void tcp_socket_timer(void *ctx)
{
	rad_listen_t *listener = ctx;
	listen_socket_t *sock = listener->data;
	struct timeval end, now;
	char buffer[256];
	fr_socket_limit_t *limit;

	fr_event_now(el, &now);

	switch (listener->type) {
	case RAD_LISTEN_PROXY:
		limit = &sock->home->limit;
		break;

	case RAD_LISTEN_AUTH:
	case RAD_LISTEN_ACCT:
		limit = &sock->limit;
		break;

	default:
		return;
	}

	/*
	 *	If we enforce a lifetime, do it now.
	 */
	if (limit->lifetime > 0) {
		end.tv_sec = sock->opened + limit->lifetime;
		end.tv_usec = 0;

		if (timercmp(&end, &now, <=)) {
			listener->print(listener, buffer, sizeof(buffer));
			DEBUG("Reached maximum lifetime on socket %s", buffer);

		do_close:

			listener->status = RAD_LISTEN_STATUS_EOL;
			event_new_fd(listener);
			return;
		}
	} else {
		end = now;
		end.tv_sec += 3600;
	}

	/*
	 *	Enforce an idle timeout.
	 */
	if (limit->idle_timeout > 0) {
		struct timeval idle;

		rad_assert(sock->last_packet != 0);
		idle.tv_sec = sock->last_packet + limit->idle_timeout;
		idle.tv_usec = 0;

		if (timercmp(&idle, &now, <=)) {
			listener->print(listener, buffer, sizeof(buffer));
			DEBUG("Reached idle timeout on socket %s", buffer);
			goto do_close;
		}

		/*
		 *	Enforce the minimum of idle timeout or lifetime.
		 */
		if (timercmp(&idle, &end, <)) {
			end = idle;
		}
	}

	/*
	 *	Wake up at t + 0.5s.  The code above checks if the timers
	 *	are <= t.  This addition gives us a bit of leeway.
	 */
	end.tv_usec = USEC / 2;

	if (!fr_event_insert(el, tcp_socket_timer, listener, &end, &sock->ev)) {
		rad_panic("Failed to insert event");
	}
}


/*
 *	Add +/- 2s of jitter, as suggested in RFC 3539
 *	and in RFC 5080.
 */
static void add_jitter(struct timeval *when)
{
	uint32_t jitter;

	when->tv_sec -= 2;

	jitter = fr_rand();
	jitter ^= (jitter >> 10);
	jitter &= ((1 << 22) - 1); /* 22 bits of 1 */

	/*
	 *	Add in ~ (4 * USEC) of jitter.
	 */
	tv_add(when, jitter);
}

/*
 *	Called by socket_del to remove requests with this socket
 */
static int eol_proxy_listener(void *ctx, void *data)
{
	rad_listen_t *this = ctx;
	RADIUS_PACKET **proxy_p = data;
	REQUEST *request;

	request = fr_packet2myptr(REQUEST, proxy, proxy_p);
	if (request->proxy_listener != this) return 0;

	/*
	 *	The normal "remove_from_proxy_hash" tries to grab the
	 *	proxy mutex.  We already have it held, so grabbing it
	 *	again will cause a deadlock.  Instead, call the "no
	 *	lock" version of the function.
	 */
	rad_assert(request->in_proxy_hash == true);
	remove_from_proxy_hash_nl(request, false);

	/*
	 *	Don't mark it as DONE.  The client can retransmit, and
	 *	the packet SHOULD be re-proxied somewhere else.
	 *
	 *	Return "2" means that the rbtree code will remove it
	 *	from the tree, and we don't need to do it ourselves.
	 */
	return 2;
}
#endif	/* WITH_PROXY */

static int eol_listener(void *ctx, void *data)
{
	rad_listen_t *this = ctx;
	RADIUS_PACKET **packet_p = data;
	REQUEST *request;

	request = fr_packet2myptr(REQUEST, packet, packet_p);
	if (request->listener != this) return 0;

	request->master_state = REQUEST_STOP_PROCESSING;

	return 0;
}
#endif	/* WITH_TCP */

#ifdef WITH_PROXY
/***********************************************************************
 *
 *	Proxy handlers for the state machine.
 *
 ***********************************************************************/

static void remove_from_proxy_hash_nl(REQUEST *request, bool yank)
{
	if (!request->in_proxy_hash) return;

	fr_packet_list_id_free(proxy_list, request->proxy, yank);
	request->in_proxy_hash = false;

	/*
	 *	On the FIRST reply, decrement the count of outstanding
	 *	requests.  Note that this is NOT the count of sent
	 *	packets, but whether or not the home server has
	 *	responded at all.
	 */
	if (request->home_server &&
	    request->home_server->currently_outstanding) {
		request->home_server->currently_outstanding--;

		/*
		 *	If we're NOT sending it packets, then we don't know
		 *	if it's alive or dead.
		 */
		if ((request->home_server->currently_outstanding == 0) &&
		    (request->home_server->state == HOME_STATE_ALIVE)) {
			request->home_server->state = HOME_STATE_UNKNOWN;
			request->home_server->last_packet_sent = 0;
			request->home_server->last_packet_recv = 0;
		}
	}

#ifdef WITH_TCP
	request->proxy_listener->count--;
#endif
	request->proxy_listener = NULL;

	/*
	 *	Got from YES in hash, to NO, not in hash while we hold
	 *	the mutex.  This guarantees that when another thread
	 *	grabs the mutex, the "not in hash" flag is correct.
	 */
	RDEBUG3("proxy: request is no longer in proxy hash");
}

static void remove_from_proxy_hash(REQUEST *request)
{
	/*
	 *	Check this without grabbing the mutex because it's a
	 *	lot faster that way.
	 */
	if (!request->in_proxy_hash) return;

	/*
	 *	The "not in hash" flag is definitive.  However, if the
	 *	flag says that it IS in the hash, there might still be
	 *	a race condition where it isn't.
	 */
	PTHREAD_MUTEX_LOCK(&proxy_mutex);

	if (!request->in_proxy_hash) {
		PTHREAD_MUTEX_UNLOCK(&proxy_mutex);
		return;
	}

	remove_from_proxy_hash_nl(request, true);

  	PTHREAD_MUTEX_UNLOCK(&proxy_mutex);
}

static int insert_into_proxy_hash(REQUEST *request)
{
	char buf[128];
	int rcode, tries;
	void *proxy_listener;

	rad_assert(request->proxy != NULL);
	rad_assert(request->home_server != NULL);
	rad_assert(proxy_list != NULL);


	PTHREAD_MUTEX_LOCK(&proxy_mutex);
	proxy_listener = NULL;
	request->num_proxied_requests = 1;
	request->num_proxied_responses = 0;

	for (tries = 0; tries < 2; tries++) {
		rad_listen_t *this;

		RDEBUG3("proxy: Trying to allocate ID (%d/2)", tries);
		rcode = fr_packet_list_id_alloc(proxy_list,
						request->home_server->proto,
						&request->proxy, &proxy_listener);
		if ((debug_flag > 2) && (rcode == 0)) {
			RDEBUG("proxy: Failed allocating ID: %s", fr_strerror());
		}
		if (rcode > 0) break;
		if (tries > 0) continue; /* try opening new socket only once */

#ifdef HAVE_PTHREAD_H
		if (proxy_no_new_sockets) break;
#endif

		RDEBUG3("proxy: Trying to open a new listener to the home server");
		this = proxy_new_listener(request->home_server, 0);
		if (!this) {
			PTHREAD_MUTEX_UNLOCK(&proxy_mutex);
			ERROR("proxy: Failed to create a new outbound socket");
			goto fail;
		}

		request->proxy->src_port = 0; /* Use any new socket */
		proxy_listener = this;

		/*
		 *	Add it to the event loop (and to the packet list)
		 *	before we try to grab another Id.
		 */
		PTHREAD_MUTEX_UNLOCK(&proxy_mutex);
		if (!event_new_fd(this)) {
			RDEBUG3("proxy: Failed inserting new socket into event loop");
			listen_free(&this);
			goto fail;
		}
		PTHREAD_MUTEX_LOCK(&proxy_mutex);
	}

	if (!proxy_listener || (rcode == 0)) {
		PTHREAD_MUTEX_UNLOCK(&proxy_mutex);
		REDEBUG2("proxy: Failed allocating Id for proxied request");
	fail:
		request->proxy_listener = NULL;
		request->in_proxy_hash = false;
		return 0;
	}

	rad_assert(request->proxy->id >= 0);

	request->proxy_listener = proxy_listener;
	request->in_proxy_hash = true;
	RDEBUG3("proxy: request is now in proxy hash");

	/*
	 *	Keep track of maximum outstanding requests to a
	 *	particular home server.  'max_outstanding' is
	 *	enforced in home_server_ldb(), in realms.c.
	 */
	request->home_server->currently_outstanding++;

#ifdef WITH_TCP
	request->proxy_listener->count++;
#endif

	PTHREAD_MUTEX_UNLOCK(&proxy_mutex);

	RDEBUG3(" proxy: allocating destination %s port %d - Id %d",
	       inet_ntop(request->proxy->dst_ipaddr.af,
			 &request->proxy->dst_ipaddr.ipaddr, buf, sizeof(buf)),
	       request->proxy->dst_port,
	       request->proxy->id);

	return 1;
}

static int process_proxy_reply(REQUEST *request)
{
	int rcode;
	int post_proxy_type = 0;
	VALUE_PAIR *vp;

	/*
	 *	Delete any reply we had accumulated until now.
	 */
	pairfree(&request->reply->vps);

	/*
	 *	Run the packet through the post-proxy stage,
	 *	BEFORE playing games with the attributes.
	 */
	vp = pairfind(request->config_items, PW_POST_PROXY_TYPE, 0, TAG_ANY);

	/*
	 *	If we have a proxy_reply, and it was a reject, setup
	 *	post-proxy-type Reject
	 */
	if (!vp && request->proxy_reply &&
	    request->proxy_reply->code == PW_AUTHENTICATION_REJECT) {
	    	DICT_VALUE	*dval;

		dval = dict_valbyname(PW_POST_PROXY_TYPE, 0, "Reject");
		if (dval) {
			vp = radius_paircreate(request, &request->config_items,
					       PW_POST_PROXY_TYPE, 0);

			vp->vp_integer = dval->value;
		}
	}

	if (vp) {
		post_proxy_type = vp->vp_integer;

		RDEBUG2("  Found Post-Proxy-Type %s",
			dict_valnamebyattr(PW_POST_PROXY_TYPE, 0,
					   post_proxy_type));
	}

	if (request->home_pool && request->home_pool->virtual_server) {
		char const *old_server = request->server;

		request->server = request->home_pool->virtual_server;
		RDEBUG2(" server %s {", request->server);
		rcode = process_post_proxy(post_proxy_type, request);
		RDEBUG2(" }");
		request->server = old_server;
	} else {
		rcode = process_post_proxy(post_proxy_type, request);
	}

#ifdef WITH_COA
	if (request->packet->code == request->proxy->code)
	  /*
	   *	Don't run the next bit if we originated a CoA
	   *	packet, after receiving an Access-Request or
	   *	Accounting-Request.
	   */
#endif

	/*
	 *	There may NOT be a proxy reply, as we may be
	 *	running Post-Proxy-Type = Fail.
	 */
	if (request->proxy_reply) {
		/*
		 *	Delete the Proxy-State Attributes from
		 *	the reply.  These include Proxy-State
		 *	attributes from us and remote server.
		 */
		pairdelete(&request->proxy_reply->vps, PW_PROXY_STATE, 0, TAG_ANY);

		/*
		 *	Add the attributes left in the proxy
		 *	reply to the reply list.
		 */
		pairfilter(request->reply, &request->reply->vps,
			  &request->proxy_reply->vps, 0, 0, TAG_ANY);

		/*
		 *	Free proxy request pairs.
		 */
		pairfree(&request->proxy->vps);
	}

	switch (rcode) {
	default:  /* Don't do anything */
		break;
	case RLM_MODULE_FAIL:
		return 0;

	case RLM_MODULE_HANDLED:
		return 0;
	}

	return 1;
}

int request_proxy_reply(RADIUS_PACKET *packet)
{
	RADIUS_PACKET **proxy_p;
	REQUEST *request;
	struct timeval now;
	char buffer[128];

	PTHREAD_MUTEX_LOCK(&proxy_mutex);
	proxy_p = fr_packet_list_find_byreply(proxy_list, packet);

	if (!proxy_p) {
		PTHREAD_MUTEX_UNLOCK(&proxy_mutex);
		PROXY( "No outstanding request was found for reply from host %s port %d - ID %d",
		       inet_ntop(packet->src_ipaddr.af,
				 &packet->src_ipaddr.ipaddr,
				 buffer, sizeof(buffer)),
		       packet->src_port, packet->id);
		return 0;
	}

	request = fr_packet2myptr(REQUEST, proxy, proxy_p);
	request->num_proxied_responses++; /* needs to be protected by lock */

	PTHREAD_MUTEX_UNLOCK(&proxy_mutex);

	/*
	 *	No reply, BUT the current packet fails verification:
	 *	ignore it.  This does the MD5 calculations in the
	 *	server core, but I guess we can fix that later.
	 */
	if (!request->proxy_reply &&
	    (rad_verify(packet, request->proxy,
			request->home_server->secret) != 0)) {
		DEBUG("Ignoring spoofed proxy reply.  Signature is invalid");
		return 0;
	}

	/*
	 *	The home server sent us a packet which doesn't match
	 *	something we have: ignore it.  This is done only to
	 *	catch the case of broken systems.
	 */
	if (request->proxy_reply &&
	    (memcmp(request->proxy_reply->vector,
		    packet->vector,
		    sizeof(request->proxy_reply->vector)) != 0)) {
		RDEBUG2("Ignoring conflicting proxy reply");
		return 0;
	}

	gettimeofday(&now, NULL);

	/*
	 *	Status-Server packets don't count as real packets.
	 */
	if (request->proxy->code != PW_STATUS_SERVER) {
		listen_socket_t *sock = request->proxy_listener->data;

		request->home_server->last_packet_recv = now.tv_sec;
		sock->last_packet = now.tv_sec;
	}

	/*
	 *	If we have previously seen a reply, ignore the
	 *	duplicate.
	 */
	if (request->proxy_reply) {
		RDEBUG2("Discarding duplicate reply from host %s port %d  - ID: %d",
			inet_ntop(packet->src_ipaddr.af,
				  &packet->src_ipaddr.ipaddr,
				  buffer, sizeof(buffer)),
			packet->src_port, packet->id);
		return 0;
	}

	/*
	 *	Call the state machine to do something useful with the
	 *	request.
	 */
	request->proxy_reply = packet;
	packet->timestamp = now;
	request->priority = RAD_LISTEN_PROXY;

	/*
	 *	We've received a reply.  If we hadn't been sending it
	 *	packets for a while, just mark it alive.
	 */
	if (request->home_server->state == HOME_STATE_UNKNOWN) {
		request->home_server->state = HOME_STATE_ALIVE;
	}

#ifdef WITH_STATS
	request->home_server->stats.last_packet = packet->timestamp.tv_sec;
	request->proxy_listener->stats.last_packet = packet->timestamp.tv_sec;

	if (request->proxy->code == PW_AUTHENTICATION_REQUEST) {
		proxy_auth_stats.last_packet = packet->timestamp.tv_sec;
#ifdef WITH_ACCOUNTING
	} else if (request->proxy->code == PW_ACCOUNTING_REQUEST) {
		proxy_acct_stats.last_packet = packet->timestamp.tv_sec;
#endif
	}
#endif	/* WITH_STATS */

#ifdef WITH_COA
	/*
	 *	When we originate CoA requests, we patch them in here
	 *	so that they don't affect the rest of the state
	 *	machine.
	 */
	if (request->parent) {
		rad_assert(request->parent->coa == request);
		rad_assert((request->proxy->code == PW_COA_REQUEST) ||
			   (request->proxy->code == PW_DISCONNECT_REQUEST));
		rad_assert(request->process != NULL);
		request_coa_separate(request);
	}
#endif

	request->process(request, FR_ACTION_PROXY_REPLY);

	return 1;
}


static int setup_post_proxy_fail(REQUEST *request)
{
	DICT_VALUE const *dval = NULL;
	VALUE_PAIR *vp;

	if (request->proxy->code == PW_AUTHENTICATION_REQUEST) {
		dval = dict_valbyname(PW_POST_PROXY_TYPE, 0,
				      "Fail-Authentication");

	} else if (request->proxy->code == PW_ACCOUNTING_REQUEST) {
		dval = dict_valbyname(PW_POST_PROXY_TYPE, 0,
				      "Fail-Accounting");
#ifdef WITH_COA
	} else if (request->proxy->code == PW_COA_REQUEST) {
		dval = dict_valbyname(PW_POST_PROXY_TYPE, 0, "Fail-CoA");

	} else if (request->proxy->code == PW_DISCONNECT_REQUEST) {
		dval = dict_valbyname(PW_POST_PROXY_TYPE, 0, "Fail-Disconnect");
#endif
	} else {
		WDEBUG("Unknown packet type in Post-Proxy-Type Fail: ignoring");
		return 0;
	}

	if (!dval) dval = dict_valbyname(PW_POST_PROXY_TYPE, 0, "Fail");

	if (!dval) {
		DEBUG("No Post-Proxy-Type Fail: ignoring");
		pairdelete(&request->config_items, PW_POST_PROXY_TYPE, 0, TAG_ANY);
		return 0;
	}

	vp = pairfind(request->config_items, PW_POST_PROXY_TYPE, 0, TAG_ANY);
	if (!vp) vp = radius_paircreate(request, &request->config_items,
					PW_POST_PROXY_TYPE, 0);
	vp->vp_integer = dval->value;

	return 1;
}

STATE_MACHINE_DECL(proxy_running)
{
	TRACE_STATE_MACHINE;

	switch (action) {
	case FR_ACTION_CONFLICTING:
	case FR_ACTION_DUP:
	case FR_ACTION_TIMER:
	case FR_ACTION_PROXY_REPLY:
		request_common(request, action);
		break;

	case FR_ACTION_RUN:
		request_running(request, FR_ACTION_PROXY_REPLY);
		break;

	default:
		RDEBUG3("%s: Ignoring action %s", __FUNCTION__, action_codes[action]);
		break;
	}
}

STATE_MACHINE_DECL(request_virtual_server)
{
	char const *old;

	TRACE_STATE_MACHINE;

	switch (action) {
	case FR_ACTION_CONFLICTING:
	case FR_ACTION_DUP:
	case FR_ACTION_TIMER:
	case FR_ACTION_PROXY_REPLY:
		request_common(request, action);
		break;

	case FR_ACTION_RUN:
		old = request->server;
		request->server = request->home_server->server;
		request_running(request, action);
		request->server = old;
		break;

	default:
		RDEBUG3("%s: Ignoring action %s", __FUNCTION__, action_codes[action]);
		break;
	}
}


static int request_will_proxy(REQUEST *request)
{
	int rcode, pre_proxy_type = 0;
	char const *realmname = NULL;
	VALUE_PAIR *vp, *strippedname;
	home_server *home;
	REALM *realm = NULL;
	home_pool_t *pool = NULL;

	if (!request->root->proxy_requests) return 0;
	if (request->packet->dst_port == 0) return 0;
	if (request->packet->code == PW_STATUS_SERVER) return 0;
	if (request->in_proxy_hash) return 0;

	/*
	 *	FIXME: for 3.0, allow this only for rejects?
	 */
	if (request->reply->code != 0) return 0;

	vp = pairfind(request->config_items, PW_PROXY_TO_REALM, 0, TAG_ANY);
	if (vp) {
		realm = realm_find2(vp->vp_strvalue);
		if (!realm) {
			REDEBUG2("Cannot proxy to unknown realm %s",
				vp->vp_strvalue);
			return 0;
		}

		realmname = vp->vp_strvalue;

		/*
		 *	Figure out which pool to use.
		 */
		if (request->packet->code == PW_AUTHENTICATION_REQUEST) {
			pool = realm->auth_pool;

#ifdef WITH_ACCOUNTING
		} else if (request->packet->code == PW_ACCOUNTING_REQUEST) {
			pool = realm->acct_pool;
#endif

#ifdef WITH_COA
		} else if ((request->packet->code == PW_COA_REQUEST) ||
			   (request->packet->code == PW_DISCONNECT_REQUEST)) {
			pool = realm->coa_pool;
#endif

		} else {
			return 0;
		}

	} else {
		int pool_type;

		vp = pairfind(request->config_items, PW_HOME_SERVER_POOL, 0, TAG_ANY);
		if (!vp) return 0;

		switch (request->packet->code) {
		case PW_AUTHENTICATION_REQUEST:
			pool_type = HOME_TYPE_AUTH;
			break;

#ifdef WITH_ACCOUNTING
		case PW_ACCOUNTING_REQUEST:
			pool_type = HOME_TYPE_ACCT;
			break;
#endif

#ifdef WITH_COA
		case PW_COA_REQUEST:
		case PW_DISCONNECT_REQUEST:
			pool_type = HOME_TYPE_COA;
			break;
#endif

		default:
			return 0;
		}

		pool = home_pool_byname(vp->vp_strvalue, pool_type);
	}

	if (!pool) {
		RWDEBUG2("Cancelling proxy as no home pool exists");
		return 0;
	}

	if (request->listener->nodup) {
		WARN("Cannot proxy a request which is from a 'nodup' socket");
		return 0;
	}

	request->home_pool = pool;

	home = home_server_ldb(realmname, pool, request);
	if (!home) {
		REDEBUG2("Failed to find live home server: Cancelling proxy");
		return 0;
	}
	home_server_update_request(home, request);

#ifdef WITH_COA
	/*
	 *	Once we've decided to proxy a request, we cannot send
	 *	a CoA packet.  So we free up any CoA packet here.
	 */
	if (request->coa) request_done(request->coa, FR_ACTION_DONE);
#endif

	/*
	 *	Remember that we sent the request to a Realm.
	 */
	if (realmname) pairmake_packet("Realm", realmname, T_OP_EQ);

	/*
	 *	Strip the name, if told to.
	 *
	 *	Doing it here catches the case of proxied tunneled
	 *	requests.
	 */
	if (realm && (realm->striprealm == true) &&
	   (strippedname = pairfind(request->proxy->vps, PW_STRIPPED_USER_NAME, 0, TAG_ANY)) != NULL) {
		/*
		 *	If there's a Stripped-User-Name attribute in
		 *	the request, then use THAT as the User-Name
		 *	for the proxied request, instead of the
		 *	original name.
		 *
		 *	This is done by making a copy of the
		 *	Stripped-User-Name attribute, turning it into
		 *	a User-Name attribute, deleting the
		 *	Stripped-User-Name and User-Name attributes
		 *	from the vps list, and making the new
		 *	User-Name the head of the vps list.
		 */
		vp = pairfind(request->proxy->vps, PW_USER_NAME, 0, TAG_ANY);
		if (!vp) {
			vp_cursor_t cursor;
			vp = radius_paircreate(request, NULL,
					       PW_USER_NAME, 0);
			rad_assert(vp != NULL);	/* handled by above function */
			/* Insert at the START of the list */
			/* FIXME: Can't make assumptions about ordering */
			paircursor(&cursor, &vp);
			pairinsert(&cursor, request->proxy->vps);
			request->proxy->vps = vp;
		}
		pairstrcpy(vp, strippedname->vp_strvalue);

		/*
		 *	Do NOT delete Stripped-User-Name.
		 */
	}

	/*
	 *	If there is no PW_CHAP_CHALLENGE attribute but
	 *	there is a PW_CHAP_PASSWORD we need to add it
	 *	since we can't use the request authenticator
	 *	anymore - we changed it.
	 */
	if ((request->packet->code == PW_AUTHENTICATION_REQUEST) &&
	    pairfind(request->proxy->vps, PW_CHAP_PASSWORD, 0, TAG_ANY) &&
	    pairfind(request->proxy->vps, PW_CHAP_CHALLENGE, 0, TAG_ANY) == NULL) {
		uint8_t *p;
		vp = radius_paircreate(request, &request->proxy->vps,
				       PW_CHAP_CHALLENGE, 0);
		vp->length = sizeof(request->packet->vector);
		vp->vp_octets = p = talloc_array(vp, uint8_t, vp->length);

		memcpy(p, request->packet->vector,
		       sizeof(request->packet->vector));
	}

	/*
	 *	The RFC's say we have to do this, but FreeRADIUS
	 *	doesn't need it.
	 */
	vp = radius_paircreate(request, &request->proxy->vps,
			       PW_PROXY_STATE, 0);
	pairsprintf(vp, "%d", request->packet->id);

	/*
	 *	Should be done BEFORE inserting into proxy hash, as
	 *	pre-proxy may use this information, or change it.
	 */
	request->proxy->code = request->packet->code;

	/*
	 *	Call the pre-proxy routines.
	 */
	vp = pairfind(request->config_items, PW_PRE_PROXY_TYPE, 0, TAG_ANY);
	if (vp) {
		RDEBUG2("  Found Pre-Proxy-Type %s", vp->vp_strvalue);
		pre_proxy_type = vp->vp_integer;
	}

	rad_assert(request->home_pool != NULL);

	if (request->home_pool->virtual_server) {
		char const *old_server = request->server;

		request->server = request->home_pool->virtual_server;
		RDEBUG2(" server %s {", request->server);
		rcode = process_pre_proxy(pre_proxy_type, request);
		RDEBUG2(" }");
			request->server = old_server;
	} else {
		rcode = process_pre_proxy(pre_proxy_type, request);
	}
	switch (rcode) {
	case RLM_MODULE_FAIL:
	case RLM_MODULE_INVALID:
	case RLM_MODULE_NOTFOUND:
	case RLM_MODULE_USERLOCK:
	default:
		/* FIXME: debug print failed stuff */
		return -1;

	case RLM_MODULE_REJECT:
	case RLM_MODULE_HANDLED:
		return 0;

	/*
	 *	Only proxy the packet if the pre-proxy code succeeded.
	 */
	case RLM_MODULE_NOOP:
	case RLM_MODULE_OK:
	case RLM_MODULE_UPDATED:
		break;
	}

	return 1;
}

static int request_proxy(REQUEST *request, int retransmit)
{
	char buffer[128];

	rad_assert(request->parent == NULL);
	rad_assert(request->home_server != NULL);

	if (request->master_state == REQUEST_STOP_PROCESSING) return 0;

#ifdef WITH_COA
	if (request->coa) {
		RWDEBUG("Cannot proxy and originate CoA packets at the same time.  Cancelling CoA request");
		request_done(request->coa, FR_ACTION_DONE);
	}
#endif

	/*
	 *	The request may be sent to a virtual server.  If we're
	 *	in a child thread, just process it here. If we're the
	 *	master, push it back onto the queue for later
	 *	processing.
	 */
	if (request->home_server->server) {
		DEBUG("Proxying to virtual server %s",
		      request->home_server->server);

		if (!we_are_master()) {
			request_virtual_server(request, FR_ACTION_RUN);
#ifdef HAVE_PTHREAD_H
			request->child_pid = NO_SUCH_CHILD_PID;
#endif
			return 1;
		}

		request_queue_or_run(request, request_virtual_server);
		return 1;
	}

	/*
	 *	We're actually sending a proxied packet.  Do that now.
	 */
	if (!request->in_proxy_hash && !insert_into_proxy_hash(request)) {
		RPROXY("Failed to insert initial packet into the proxy list.");
		return -1;
	}

	rad_assert(request->proxy->id >= 0);

	RDEBUG2("Proxying request to home server %s port %d",
	       inet_ntop(request->proxy->dst_ipaddr.af,
			 &request->proxy->dst_ipaddr.ipaddr,
			 buffer, sizeof(buffer)),
		request->proxy->dst_port);

	DEBUG_PACKET(request, request->proxy, 1);

	gettimeofday(&request->proxy_retransmit, NULL);
	if (!retransmit) {
		request->proxy->timestamp = request->proxy_retransmit;
		request->home_server->last_packet_sent = request->proxy_retransmit.tv_sec;
	}

#ifdef HAVE_PTHREAD_H
	request->child_pid = NO_SUCH_CHILD_PID;
#endif
	FR_STATS_TYPE_INC(request->home_server->stats.total_requests);
	request->proxy_listener->send(request->proxy_listener,
				      request);
	return 1;
}

/*
 *	Proxy the packet as if it was new.
 */
static int request_proxy_anew(REQUEST *request)
{
	home_server *home;

	/*
	 *	Delete the request from the proxy list.
	 *
	 *	The packet list code takes care of ensuring that IDs
	 *	aren't reused until all 256 IDs have been used.  So
	 *	there's a 1/256 chance of re-using the same ID when
	 *	we're sending to the same home server.  Which is
	 *	acceptable.
	 */
	remove_from_proxy_hash(request);

	/*
	 *	Find a live home server for the request.
	 */
	home = home_server_ldb(NULL, request->home_pool, request);
	if (!home) {
		REDEBUG2("Failed to find live home server for request");
	post_proxy_fail:
		if (setup_post_proxy_fail(request)) {
			request_queue_or_run(request, proxy_running);
		} else {
			gettimeofday(&request->reply->timestamp, NULL);
			request_cleanup_delay_init(request, NULL);
		}
		return 0;
	}
	home_server_update_request(home, request);

	if (!insert_into_proxy_hash(request)) {
		RPROXY("Failed to insert retransmission into the proxy list.");
		goto post_proxy_fail;
	}

	/*
	 *	Free the old packet, to force re-encoding
	 */
	talloc_free(request->proxy->data);
	request->proxy->data = NULL;
	request->proxy->data_len = 0;

#ifdef WITH_ACCOUNTING
	/*
	 *	Update the Acct-Delay-Time attribute.
	 */
	if (request->packet->code == PW_ACCOUNTING_REQUEST) {
		VALUE_PAIR *vp;

		vp = pairfind(request->proxy->vps, PW_ACCT_DELAY_TIME, 0, TAG_ANY);
		if (!vp) vp = radius_paircreate(request,
						&request->proxy->vps,
						PW_ACCT_DELAY_TIME, 0);
		if (vp) {
			struct timeval now;

			gettimeofday(&now, NULL);
			vp->vp_integer += now.tv_sec - request->proxy_retransmit.tv_sec;
		}
	}
#endif

	if (request_proxy(request, 1) != 1) goto post_proxy_fail;

	return 1;
}

STATE_MACHINE_DECL(request_ping)
{
	home_server *home = request->home_server;
	char buffer[128];

	TRACE_STATE_MACHINE;
	ASSERT_MASTER;

	switch (action) {
	case FR_ACTION_TIMER:
		ERROR("No response to status check %d for home server %s port %d",
		       request->number,
		       inet_ntop(request->proxy->dst_ipaddr.af,
				 &request->proxy->dst_ipaddr.ipaddr,
				 buffer, sizeof(buffer)),
		       request->proxy->dst_port);
		break;

	case FR_ACTION_PROXY_REPLY:
		rad_assert(request->in_proxy_hash);

		request->home_server->num_received_pings++;
		RPROXY("Received response to status check %d (%d in current sequence)",
		       request->number, home->num_received_pings);

		/*
		 *	Remove the request from any hashes
		 */
		fr_event_delete(el, &request->ev);
		remove_from_proxy_hash(request);

		/*
		 *	The control socket may have marked the home server as
		 *	alive.  OR, it may have suddenly started responding to
		 *	requests again.  If so, don't re-do the "make alive"
		 *	work.
		 */
		if (home->state == HOME_STATE_ALIVE) break;

		/*
		 *	We haven't received enough ping responses to mark it
		 *	"alive".  Wait a bit.
		 */
		if (home->num_received_pings < home->num_pings_to_alive) {
			break;
		}

		/*
		 *	Mark it alive and delete any outstanding
		 *	pings.
		 */
		home->state = HOME_STATE_ALIVE;
		exec_trigger(request, request->home_server->cs, "home_server.alive", false);
		home->currently_outstanding = 0;
		home->num_sent_pings = 0;
		home->num_received_pings = 0;
		gettimeofday(&home->revive_time, NULL);

		fr_event_delete(el, &home->ev);

		RPROXY("Marking home server %s port %d alive",
		       inet_ntop(request->proxy->dst_ipaddr.af,
				 &request->proxy->dst_ipaddr.ipaddr,
				 buffer, sizeof(buffer)),
		       request->proxy->dst_port);
		break;

	default:
		RDEBUG3("%s: Ignoring action %s", __FUNCTION__, action_codes[action]);
		break;
	}

	rad_assert(!request->in_request_hash);
	rad_assert(request->ev == NULL);
	request_done(request, FR_ACTION_DONE);
}

/*
 *	Called from start of zombie period, OR after control socket
 *	marks the home server dead.
 */
static void ping_home_server(void *ctx)
{
	home_server *home = ctx;
	REQUEST *request;
	VALUE_PAIR *vp;
	struct timeval when, now;

	if ((home->state == HOME_STATE_ALIVE) ||
	    (home->ping_check == HOME_PING_CHECK_NONE) ||
#ifdef WITH_TCP
	    (home->proto == IPPROTO_TCP) ||
#endif
	    (home->ev != NULL)) {
		return;
	}

	gettimeofday(&now, NULL);

	if (home->state == HOME_STATE_ZOMBIE) {
		when = home->zombie_period_start;
		when.tv_sec += home->zombie_period;

		if (timercmp(&when, &now, <)) {
			DEBUG("PING: Zombie period is over for home server %s",
				home->name);
			mark_home_server_dead(home, &now);
		}
	}

	request = request_alloc(NULL);
	request->number = request_num_counter++;
#ifdef HAVE_PTHREAD_H
	request->child_pid = NO_SUCH_CHILD_PID;
#endif

	request->proxy = rad_alloc(request, 1);
	rad_assert(request->proxy != NULL);

	if (home->ping_check == HOME_PING_CHECK_STATUS_SERVER) {
		request->proxy->code = PW_STATUS_SERVER;

		pairmake(request->proxy, &request->proxy->vps,
			 "Message-Authenticator", "0x00", T_OP_SET);

	} else if (home->type == HOME_TYPE_AUTH) {
		request->proxy->code = PW_AUTHENTICATION_REQUEST;

		pairmake(request->proxy, &request->proxy->vps,
			 "User-Name", home->ping_user_name, T_OP_SET);
		pairmake(request->proxy, &request->proxy->vps,
			 "User-Password", home->ping_user_password, T_OP_SET);
		pairmake(request->proxy, &request->proxy->vps,
			 "Service-Type", "Authenticate-Only", T_OP_SET);
		pairmake(request->proxy, &request->proxy->vps,
			 "Message-Authenticator", "0x00", T_OP_SET);

	} else {
#ifdef WITH_ACCOUNTING
		request->proxy->code = PW_ACCOUNTING_REQUEST;

		pairmake(request->proxy, &request->proxy->vps,
			 "User-Name", home->ping_user_name, T_OP_SET);
		pairmake(request->proxy, &request->proxy->vps,
			 "Acct-Status-Type", "Stop", T_OP_SET);
		pairmake(request->proxy, &request->proxy->vps,
			 "Acct-Session-Id", "00000000", T_OP_SET);
		vp = pairmake(request->proxy, &request->proxy->vps,
			      "Event-Timestamp", "0", T_OP_SET);
		vp->vp_date = now.tv_sec;
#else
		rad_assert("Internal sanity check failed");
#endif
	}

	vp = pairmake(request->proxy, &request->proxy->vps,
		      "NAS-Identifier", "", T_OP_SET);
	if (vp) {
		pairsprintf(vp, "Status Check %u. Are you alive?",
			    home->num_sent_pings);
	}

	request->proxy->src_ipaddr = home->src_ipaddr;
	request->proxy->dst_ipaddr = home->ipaddr;
	request->proxy->dst_port = home->port;
	request->home_server = home;
#ifdef DEBUG_STATE_MACHINE
	if (debug_flag) printf("(%u) ********\tSTATE %s C%u -> C%u\t********\n", request->number, __FUNCTION__, request->child_state, REQUEST_DONE);
	if (debug_flag) printf("(%u) ********\tNEXT-STATE %s -> %s\n", request->number, __FUNCTION__, "request_ping");
#endif
#ifdef HAVE_PTHREAD_H
	rad_assert(request->child_pid == NO_SUCH_CHILD_PID);
#endif
	request->child_state = REQUEST_DONE;
	request->process = request_ping;

	rad_assert(request->proxy_listener == NULL);

	if (!insert_into_proxy_hash(request)) {
		RPROXY("Failed to insert status check %d into proxy list.  Discarding it.",
		       request->number);

		rad_assert(!request->in_request_hash);
		rad_assert(!request->in_proxy_hash);
		rad_assert(request->ev == NULL);
		request_free(&request);
		return;
	}

	/*
	 *	Set up the timer callback.
	 */
	when = now;
	when.tv_sec += home->ping_timeout;

	DEBUG("PING: Waiting %u seconds for response to ping",
	      home->ping_timeout);

	STATE_MACHINE_TIMER(FR_ACTION_TIMER);
	home->num_sent_pings++;

	rad_assert(request->proxy_listener != NULL);
	request->proxy_listener->send(request->proxy_listener,
				      request);

	/*
	 *	Add +/- 2s of jitter, as suggested in RFC 3539
	 *	and in the Issues and Fixes draft.
	 */
	home->when = now;
	home->when.tv_sec += home->ping_interval;

	add_jitter(&home->when);

	DEBUG("PING: Next status packet in %u seconds", home->ping_interval);
	INSERT_EVENT(ping_home_server, home);
}

static void home_trigger(home_server *home, char const *trigger)
{
	REQUEST my_request;
	RADIUS_PACKET my_packet;

	memset(&my_request, 0, sizeof(my_request));
	memset(&my_packet, 0, sizeof(my_packet));
	my_request.proxy = &my_packet;
	my_packet.dst_ipaddr = home->ipaddr;
	my_packet.src_ipaddr = home->src_ipaddr;

	exec_trigger(&my_request, home->cs, trigger, false);
}

static void mark_home_server_zombie(home_server *home)
{
	char buffer[128];

	ASSERT_MASTER;

	rad_assert((home->state == HOME_STATE_ALIVE) ||
		   (home->state == HOME_STATE_UNKNOWN));

#ifdef WITH_TCP
	if (home->proto == IPPROTO_TCP) {
		WDEBUG("Not marking TCP server %s zombie", home->name);
		return;
	}
#endif

	home->state = HOME_STATE_ZOMBIE;
	home_trigger(home, "home_server.zombie");

	/*
	 *	Back-date the zombie period to when we last expected
	 *	to see a response.  i.e. when we last sent a request.
	 */
	if (home->last_packet_sent == 0) {
		gettimeofday(&home->zombie_period_start, NULL);
	} else {
		home->zombie_period_start.tv_sec = home->last_packet_sent;
		home->zombie_period_start.tv_usec = 0;
	}

	fr_event_delete(el, &home->ev);
	home->num_sent_pings = 0;
	home->num_received_pings = 0;

	PROXY( "Marking home server %s port %d as zombie (it has not responded in %d seconds).",
	       inet_ntop(home->ipaddr.af, &home->ipaddr.ipaddr,
			 buffer, sizeof(buffer)),
	       home->port, home->response_window);

	ping_home_server(home);
}


void revive_home_server(void *ctx)
{
	home_server *home = ctx;
	char buffer[128];

#ifdef WITH_TCP
	rad_assert(home->proto != IPPROTO_TCP);
#endif

	home->state = HOME_STATE_ALIVE;
	home_trigger(home, "home_server.alive");
	home->currently_outstanding = 0;
	gettimeofday(&home->revive_time, NULL);

	/*
	 *	Delete any outstanding events.
	 */
	if (home->ev) fr_event_delete(el, &home->ev);

	PROXY( "Marking home server %s port %d alive again... we have no idea if it really is alive or not.",
	       inet_ntop(home->ipaddr.af, &home->ipaddr.ipaddr,
			 buffer, sizeof(buffer)),
	       home->port);
}

void mark_home_server_dead(home_server *home, struct timeval *when)
{
	int previous_state = home->state;
	char buffer[128];

#ifdef WITH_TCP
	if (home->proto == IPPROTO_TCP) {
		WDEBUG("Not marking TCP server dead");
		return;
	}
#endif

	PROXY( "Marking home server %s port %d as dead.",
	       inet_ntop(home->ipaddr.af, &home->ipaddr.ipaddr,
			 buffer, sizeof(buffer)),
	       home->port);

	home->state = HOME_STATE_IS_DEAD;
	home_trigger(home, "home_server.dead");

	if (home->ping_check != HOME_PING_CHECK_NONE) {
		/*
		 *	If the control socket marks us dead, start
		 *	pinging.  Otherwise, we already started
		 *	pinging when it was marked "zombie".
		 */
		if (previous_state == HOME_STATE_ALIVE) {
			ping_home_server(home);
		} else {
			DEBUG("PING: Already pinging home server %s",
			      home->name);
		}

	} else {
		/*
		 *	Revive it after a fixed period of time.  This
		 *	is very, very, bad.
		 */
		home->when = *when;
		home->when.tv_sec += home->revive_interval;

		DEBUG("PING: Reviving home server %s in %u seconds",
		      home->name, home->revive_interval);
		INSERT_EVENT(revive_home_server, home);
	}
}

STATE_MACHINE_DECL(proxy_wait_for_reply)
{
	struct timeval now, when;
	home_server *home = request->home_server;
	char buffer[128];

	TRACE_STATE_MACHINE;

	rad_assert(request->packet->code != PW_STATUS_SERVER);
	rad_assert(request->home_server != NULL);

	if (request->master_state == REQUEST_STOP_PROCESSING) {
		request->child_state = REQUEST_DONE;
		return;
	}

	gettimeofday(&now, NULL);

	switch (action) {
	case FR_ACTION_DUP:
		if (request->proxy_reply) return;

		if ((home->state == HOME_STATE_IS_DEAD) ||
		    !request->proxy_listener ||
		    (request->proxy_listener->status != RAD_LISTEN_STATUS_KNOWN)) {
			request_proxy_anew(request);
			return;
		}

#ifdef WITH_TCP
		if (home->proto == IPPROTO_TCP) {
			DEBUG2("Suppressing duplicate proxied request to home server %s port %d proto TCP - ID: %d",
			       inet_ntop(request->proxy->dst_ipaddr.af,
					 &request->proxy->dst_ipaddr.ipaddr,
					 buffer, sizeof(buffer)),
			       request->proxy->dst_port,
			       request->proxy->id);
			return;
		}
#endif

#ifdef WITH_ACCOUNTING
		/*
		 *	If we update the Acct-Delay-Time, we need to
		 *	get a new ID.
		 */
		if ((request->packet->code == PW_ACCOUNTING_REQUEST) &&
		    pairfind(request->proxy->vps, PW_ACCT_DELAY_TIME, 0, TAG_ANY)) {
			request_proxy_anew(request);
			return;
		}
#endif

		RDEBUG2("Sending duplicate proxied request to home server %s port %d - ID: %d",
			inet_ntop(request->proxy->dst_ipaddr.af,
				  &request->proxy->dst_ipaddr.ipaddr,
				  buffer, sizeof(buffer)),
			request->proxy->dst_port,
			request->proxy->id);
		request->num_proxied_requests++;

		rad_assert(request->proxy_listener != NULL);;
		DEBUG_PACKET(request, request->proxy, 1);
		FR_STATS_TYPE_INC(home->stats.total_requests);
		home->last_packet_sent = now.tv_sec;
		request->proxy_listener->send(request->proxy_listener,
					      request);
		break;

	case FR_ACTION_TIMER:
		/*
		 *	Wake up "response_window" time in the future.
		 *	i.e. when MY packet hasn't received a response.
		 *
		 *	Note that we DO NOT mark the home server as
		 *	zombie if it doesn't respond to us.  It may be
		 *	responding to other (better looking) packets.
		 */
		when = request->proxy->timestamp;
		when.tv_sec += home->response_window;

		/*
		 *	Not at the response window.  Set the timer for
		 *	that.
		 */
		if (timercmp(&when, &now, >)) {
			RDEBUG("Expecting proxy response no later than %d seconds from now", home->response_window);
			STATE_MACHINE_TIMER(FR_ACTION_TIMER);
			return;
		}

		RDEBUG("No proxy response, giving up on request and marking it done");

		/*
		 *	If we haven't received any packets for
		 *	"response_window", then mark the home server
		 *	as zombie.
		 *
		 *	If the connection is TCP, then another
		 *	"watchdog timer" function takes care of pings,
		 *	etc.  So we don't need to do it here.
		 *
		 *	This check should really be part of a home
		 *	server state machine.
		 */
		if (((home->state == HOME_STATE_ALIVE) ||
		     (home->state == HOME_STATE_UNKNOWN)) &&
#ifdef WITH_TCP
		    (home->proto != IPPROTO_TCP) &&
#endif
		    ((home->last_packet_recv + home->response_window) <= now.tv_sec)) {
			mark_home_server_zombie(home);
		}

		FR_STATS_TYPE_INC(home->stats.total_timeouts);
		if (home->type == HOME_TYPE_AUTH) {
			if (request->proxy_listener) FR_STATS_TYPE_INC(request->proxy_listener->stats.total_timeouts);
			FR_STATS_TYPE_INC(proxy_auth_stats.total_timeouts);
		}
#ifdef WITH_ACCT
		else if (home->type == HOME_TYPE_ACCT) {
			if (request->proxy_listener) FR_STATS_TYPE_INC(request->proxy_listener->stats.total_timeouts);
			FR_STATS_TYPE_INC(proxy_acct_stats.total_timeouts);
		}
#endif

		/*
		 *	There was no response within the window.  Stop
		 *	the request.  If the client retransmitted, it
		 *	may have failed over to another home server.
		 *	But that one may be dead, too.
		 */
		RERROR("Failing request due to lack of any response from home server %s port %d",
			       inet_ntop(request->proxy->dst_ipaddr.af,
					 &request->proxy->dst_ipaddr.ipaddr,
					 buffer, sizeof(buffer)),
			       request->proxy->dst_port);

		if (!setup_post_proxy_fail(request)) {
			gettimeofday(&request->reply->timestamp, NULL);
			request_cleanup_delay_init(request, NULL);
			return;
		}
		/* FALL-THROUGH */

		/*
		 *	Duplicate proxy replies have been quenched by
		 *	now.  This state is only called ONCE, when we
		 *	receive a new reply from the home server.
		 */
	case FR_ACTION_PROXY_REPLY:
		request_queue_or_run(request, proxy_running);
		break;

	case FR_ACTION_CONFLICTING:
		request_done(request, action);
		return;

	default:
		RDEBUG3("%s: Ignoring action %s", __FUNCTION__, action_codes[action]);
		break;
	}
}
#endif	/* WITH_PROXY */

/***********************************************************************
 *
 *  CoA code
 *
 ***********************************************************************/
#ifdef WITH_COA
static int null_handler(UNUSED REQUEST *request)
{
	return 0;
}

/*
 *	See if we need to originate a CoA request.
 */
static void request_coa_originate(REQUEST *request)
{
	int rcode, pre_proxy_type = 0;
	VALUE_PAIR *vp;
	REQUEST *coa;
	fr_ipaddr_t ipaddr;
	char buffer[256];

	rad_assert(request != NULL);
	rad_assert(request->coa != NULL);
	rad_assert(request->proxy == NULL);
	rad_assert(!request->in_proxy_hash);
	rad_assert(request->proxy_reply == NULL);

	/*
	 *	Check whether we want to originate one, or cancel one.
	 */
	vp = pairfind(request->config_items, PW_SEND_COA_REQUEST, 0, TAG_ANY);
	if (!vp) {
		vp = pairfind(request->coa->proxy->vps, PW_SEND_COA_REQUEST, 0, TAG_ANY);
	}

	if (vp) {
		if (vp->vp_integer == 0) {
		fail:
			request_done(request->coa, FR_ACTION_DONE);
			return;
		}
	}

	coa = request->coa;

	/*
	 *	src_ipaddr will be set up in proxy_encode.
	 */
	memset(&ipaddr, 0, sizeof(ipaddr));
	vp = pairfind(coa->proxy->vps, PW_PACKET_DST_IP_ADDRESS, 0, TAG_ANY);
	if (vp) {
		ipaddr.af = AF_INET;
		ipaddr.ipaddr.ip4addr.s_addr = vp->vp_ipaddr;

	} else if ((vp = pairfind(coa->proxy->vps, PW_PACKET_DST_IPV6_ADDRESS, 0, TAG_ANY)) != NULL) {
		ipaddr.af = AF_INET6;
		ipaddr.ipaddr.ip6addr = vp->vp_ipv6addr;

	} else if ((vp = pairfind(coa->proxy->vps, PW_HOME_SERVER_POOL, 0, TAG_ANY)) != NULL) {
		coa->home_pool = home_pool_byname(vp->vp_strvalue,
						  HOME_TYPE_COA);
		if (!coa->home_pool) {
			RWDEBUG2("No such home_server_pool %s",
			       vp->vp_strvalue);
			goto fail;
		}

		/*
		 *	Prefer the pool to one server
		 */
	} else if (request->client->coa_pool) {
		coa->home_pool = request->client->coa_pool;

	} else if (request->client->coa_server) {
		coa->home_server = request->client->coa_server;

	} else {
		/*
		 *	If all else fails, send it to the client that
		 *	originated this request.
		 */
		memcpy(&ipaddr, &request->packet->src_ipaddr, sizeof(ipaddr));
	}

	/*
	 *	Use the pool, if it exists.
	 */
	if (coa->home_pool) {
		coa->home_server = home_server_ldb(NULL, coa->home_pool, coa);
		if (!coa->home_server) {
			RWDEBUG("No live home server for home_server_pool %s", coa->home_pool->name);
			goto fail;
		}
		home_server_update_request(coa->home_server, coa);

	} else if (!coa->home_server) {
		int port = PW_COA_UDP_PORT;

		vp = pairfind(coa->proxy->vps, PW_PACKET_DST_PORT, 0, TAG_ANY);
		if (vp) port = vp->vp_integer;

		coa->home_server = home_server_find(&ipaddr, port, IPPROTO_UDP);
		if (!coa->home_server) {
			RWDEBUG2("Unknown destination %s:%d for CoA request.",
			       inet_ntop(ipaddr.af, &ipaddr.ipaddr,
					 buffer, sizeof(buffer)), port);
			goto fail;
		}
	}

	vp = pairfind(coa->proxy->vps, PW_PACKET_TYPE, 0, TAG_ANY);
	if (vp) {
		switch (vp->vp_integer) {
		case PW_COA_REQUEST:
		case PW_DISCONNECT_REQUEST:
			coa->proxy->code = vp->vp_integer;
			break;

		default:
			DEBUG("Cannot set CoA Packet-Type to code %d",
			      vp->vp_integer);
			goto fail;
		}
	}

	if (!coa->proxy->code) coa->proxy->code = PW_COA_REQUEST;

	/*
	 *	The rest of the server code assumes that
	 *	request->packet && request->reply exist.  Copy them
	 *	from the original request.
	 */
	rad_assert(coa->packet != NULL);
	rad_assert(coa->packet->vps == NULL);
	memcpy(coa->packet, request->packet, sizeof(*request->packet));
	coa->packet->vps = paircopy(coa->packet, request->packet->vps);
	coa->packet->data = NULL;
	rad_assert(coa->reply != NULL);
	rad_assert(coa->reply->vps == NULL);
	memcpy(coa->reply, request->reply, sizeof(*request->reply));
	coa->reply->vps = paircopy(coa->reply, request->reply->vps);
	coa->reply->data = NULL;
	coa->config_items = paircopy(coa, request->config_items);
	coa->num_coa_requests = 0;
	coa->handle = null_handler;
	coa->number = request->number ^ (1 << 24);

	/*
	 *	Call the pre-proxy routines.
	 */
	vp = pairfind(request->config_items, PW_PRE_PROXY_TYPE, 0, TAG_ANY);
	if (vp) {
		RDEBUG2("  Found Pre-Proxy-Type %s", vp->vp_strvalue);
		pre_proxy_type = vp->vp_integer;
	}

	if (coa->home_pool && coa->home_pool->virtual_server) {
		char const *old_server = coa->server;

		coa->server = coa->home_pool->virtual_server;
		RDEBUG2(" server %s {", coa->server);
		rcode = process_pre_proxy(pre_proxy_type, coa);
		RDEBUG2(" }");
		coa->server = old_server;
	} else {
		rcode = process_pre_proxy(pre_proxy_type, coa);
	}
	switch (rcode) {
	default:
		goto fail;

	/*
	 *	Only send the CoA packet if the pre-proxy code succeeded.
	 */
	case RLM_MODULE_NOOP:
	case RLM_MODULE_OK:
	case RLM_MODULE_UPDATED:
		break;
	}

	/*
	 *	Source IP / port is set when the proxy socket
	 *	is chosen.
	 */
	coa->proxy->dst_ipaddr = coa->home_server->ipaddr;
	coa->proxy->dst_port = coa->home_server->port;

	if (!insert_into_proxy_hash(coa)) {
		radlog_request(L_PROXY, 0, coa, "Failed to insert CoA request into proxy list.");
		goto fail;
	}

	/*
	 *	We CANNOT divorce the CoA request from the parent
	 *	request.  This function is running in a child thread,
	 *	and we need access to the main event loop in order to
	 *	to add the timers for the CoA packet.
	 *
	 *	Instead, we wait for the timer on the parent request
	 *	to fire.
	 */
	gettimeofday(&coa->proxy->timestamp, NULL);
	coa->packet->timestamp = coa->proxy->timestamp; /* for max_request_time */
	coa->delay = 0;		/* need to calculate a new delay */

	DEBUG_PACKET(coa, coa->proxy, 1);

	coa->process = request_coa_process;
#ifdef DEBUG_STATE_MACHINE
	if (debug_flag) printf("(%u) ********\tSTATE %s C%u -> C%u\t********\n", request->number, __FUNCTION__, request->child_state, REQUEST_ACTIVE);
#endif
	coa->child_state = REQUEST_ACTIVE;
	rad_assert(coa->proxy_reply == NULL);
	FR_STATS_TYPE_INC(coa->home_server->stats.total_requests);
	coa->home_server->last_packet_sent = coa->proxy->timestamp.tv_sec;
	coa->proxy_listener->send(coa->proxy_listener, coa);
}


static void request_coa_separate(REQUEST *request)
{
#ifdef DEBUG_STATE_MACHINE
	int action = FR_ACTION_TIMER;
#endif
	TRACE_STATE_MACHINE;

	rad_assert(request->parent != NULL);
	rad_assert(request->parent->coa == request);
	rad_assert(request->ev == NULL);
	rad_assert(!request->in_request_hash);

	rad_assert(request->proxy_listener != NULL);
	request = talloc_steal(request->proxy_listener, request);
	request->parent->coa = NULL;
	request->parent = NULL;

	/*
	 *	Set up timers for the CoA request.  These do all kinds
	 *	of different things....
	 */
	request_coa_timer(request);
}

static void request_coa_timer(REQUEST *request)
{
	int delay, frac;
	struct timeval now, when, mrd;

	rad_assert(request->parent == NULL);

	if (request->proxy_reply) return request_process_timer(request);

	gettimeofday(&now, NULL);

	if (request->delay == 0) {
		/*
		 *	Implement re-transmit algorithm as per RFC 5080
		 *	Section 2.2.1.
		 *
		 *	We want IRT + RAND*IRT
		 *	or 0.9 IRT + rand(0,.2) IRT
		 *
		 *	2^20 ~ USEC, and we want 2.
		 *	rand(0,0.2) USEC ~ (rand(0,2^21) / 10)
		 */
		delay = (fr_rand() & ((1 << 22) - 1)) / 10;
		request->delay = delay * request->home_server->coa_irt;
		delay = request->home_server->coa_irt * USEC;
		delay -= delay / 10;
		delay += request->delay;
		request->delay = delay;

		when = request->proxy->timestamp;
		tv_add(&when, delay);

		if (timercmp(&when, &now, >)) {
			STATE_MACHINE_TIMER(FR_ACTION_TIMER);
			return;
		}
	}

	/*
	 *	Retransmit CoA request.
	 */

	/*
	 *	Cap count at MRC, if it is non-zero.
	 */
	if (request->home_server->coa_mrc &&
	    (request->num_coa_requests >= request->home_server->coa_mrc)) {
		if (!setup_post_proxy_fail(request)) {
			return;
		}

		request_queue_or_run(request, proxy_running);
		return;
	}

	/*
	 *	RFC 5080 Section 2.2.1
	 *
	 *	RT = 2*RTprev + RAND*RTprev
	 *	   = 1.9 * RTprev + rand(0,.2) * RTprev
	 *	   = 1.9 * RTprev + rand(0,1) * (RTprev / 5)
	 */
	delay = fr_rand();
	delay ^= (delay >> 16);
	delay &= 0xffff;
	frac = request->delay / 5;
	delay = ((frac >> 16) * delay) + (((frac & 0xffff) * delay) >> 16);

	delay += (2 * request->delay) - (request->delay / 10);

	/*
	 *	Cap delay at MRT, if MRT is non-zero.
	 */
	if (request->home_server->coa_mrt &&
	    (delay > (request->home_server->coa_mrt * USEC))) {
		int mrt_usec = request->home_server->coa_mrt * USEC;

		/*
		 *	delay = MRT + RAND * MRT
		 *	      = 0.9 MRT + rand(0,.2)  * MRT
		 */
		delay = fr_rand();
		delay ^= (delay >> 15);
		delay &= 0x1ffff;
		delay = ((mrt_usec >> 16) * delay) + (((mrt_usec & 0xffff) * delay) >> 16);
		delay += mrt_usec - (mrt_usec / 10);
	}

	request->delay = delay;
	when = now;
	tv_add(&when, request->delay);
	mrd = request->proxy->timestamp;
	mrd.tv_sec += request->home_server->coa_mrd;

	/*
	 *	Cap duration at MRD.
	 */
	if (timercmp(&mrd, &when, <)) {
		when = mrd;
	}
	STATE_MACHINE_TIMER(FR_ACTION_TIMER);

	request->num_coa_requests++; /* is NOT reset by code 3 lines above! */

	FR_STATS_TYPE_INC(request->home_server->stats.total_requests);

	/*
	 *	Status servers don't count as real packets sent.
	 */
	request->proxy_listener->send(request->proxy_listener,
				      request);
}


#ifdef HAVE_PTHREAD_H
STATE_MACHINE_DECL(coa_running)
{
	TRACE_STATE_MACHINE;

	switch (action) {
	case FR_ACTION_TIMER:
		request_coa_timer(request);
		break;

	case FR_ACTION_PROXY_REPLY:
		request_common(request, action);
		break;

	case FR_ACTION_RUN:
		request_running(request, FR_ACTION_PROXY_REPLY);
		break;

	default:
		RDEBUG3("%s: Ignoring action %s", __FUNCTION__, action_codes[action]);
		break;
	}
}
#endif	/* HAVE_PTHREAD_H */


/*
 *	Process CoA requests that we originated.
 */
STATE_MACHINE_DECL(request_coa_process)
{
	TRACE_STATE_MACHINE;

	switch (action) {
	case FR_ACTION_TIMER:
		request_coa_timer(request);
		break;

	case FR_ACTION_PROXY_REPLY:
		rad_assert(request->parent == NULL);
#ifdef HAVE_PTHREAD_H
		/*
		 *	Catch the case of a proxy reply when called
		 *	from the main worker thread.
		 */
		if (we_are_master() &&
		    (request->process != coa_running)) {
			request_queue_or_run(request, coa_running);
			return;
		}
		/* FALL-THROUGH */
#endif
	case FR_ACTION_RUN:
		request_running(request, action);
		break;

	default:
		RDEBUG3("%s: Ignoring action %s", __FUNCTION__, action_codes[action]);
		break;
	}
}

#endif	/* WITH_COA */

/***********************************************************************
 *
 *  End of the State machine.  Start of additional helper code.
 *
 ***********************************************************************/

/***********************************************************************
 *
 *	Event handlers.
 *
 ***********************************************************************/
static void event_socket_handler(UNUSED fr_event_list_t *xel, UNUSED int fd, void *ctx)
{
	rad_listen_t *listener = ctx;

	rad_assert(xel == el);

	if (
#ifdef WITH_DETAIL
	    (listener->type != RAD_LISTEN_DETAIL) &&
#endif
	    (listener->fd < 0)) {
		char buffer[256];

		listener->print(listener, buffer, sizeof(buffer));
		ERROR("FATAL: Asked to read from closed socket: %s",
		       buffer);

		rad_panic("Socket was closed on us!");
		fr_exit_now(1);
	}

	listener->recv(listener);
}

#ifdef WITH_DETAIL
/*
 *	This function is called periodically to see if this detail
 *	file is available for reading.
 */
static void event_poll_detail(void *ctx)
{
	int delay;
	rad_listen_t *this = ctx;
	struct timeval when, now;
	listen_detail_t *detail = this->data;

	rad_assert(this->type == RAD_LISTEN_DETAIL);

 redo:
	event_socket_handler(el, this->fd, this);

	fr_event_now(el, &now);
	when = now;

	/*
	 *	Backdoor API to get the delay until the next poll
	 *	time.
	 */
	delay = this->encode(this, NULL);
	if (delay == 0) goto redo;

	tv_add(&when, delay);

	if (!fr_event_insert(el, event_poll_detail, this,
			     &when, &detail->ev)) {
		ERROR("Failed creating handler");
		fr_exit(1);
	}
}
#endif

static void event_status(struct timeval *wake)
{
#if !defined(HAVE_PTHREAD_H) && defined(WNOHANG)
	int argval;
#endif

	if (debug_flag == 0) {
		if (just_started) {
			INFO("Ready to process requests.");
			just_started = false;
		}
		return;
	}

	if (!wake) {
		INFO("Ready to process requests.");

	} else if ((wake->tv_sec != 0) ||
		   (wake->tv_usec >= 100000)) {
		DEBUG("Waking up in %d.%01u seconds.",
		      (int) wake->tv_sec, (unsigned int) wake->tv_usec / 100000);
	}


	/*
	 *	FIXME: Put this somewhere else, where it isn't called
	 *	all of the time...
	 */

#if !defined(HAVE_PTHREAD_H) && defined(WNOHANG)
	/*
	 *	If there are no child threads, then there may
	 *	be child processes.  In that case, wait for
	 *	their exit status, and throw that exit status
	 *	away.  This helps get rid of zxombie children.
	 */
	while (waitpid(-1, &argval, WNOHANG) > 0) {
		/* do nothing */
	}
#endif

}


int event_new_fd(rad_listen_t *this)
{
	char buffer[1024];

	if (this->status == RAD_LISTEN_STATUS_KNOWN) return 1;

	this->print(this, buffer, sizeof(buffer));

	if (this->status == RAD_LISTEN_STATUS_INIT) {
		listen_socket_t *sock = this->data;

		if (just_started) {
			DEBUG("Listening on %s", buffer);
		} else {
			INFO(" ... adding new socket %s", buffer);
		}

#ifdef WITH_PROXY
		/*
		 *	Add it to the list of sockets we can use.
		 *	Server sockets (i.e. auth/acct) are never
		 *	added to the packet list.
		 */
		if (this->type == RAD_LISTEN_PROXY) {
			PTHREAD_MUTEX_LOCK(&proxy_mutex);
			if (!fr_packet_list_socket_add(proxy_list, this->fd,
						       sock->proto,
						       &sock->other_ipaddr, sock->other_port,
						       this)) {

#ifdef HAVE_PTHREAD_H
				proxy_no_new_sockets = true;
#endif
				PTHREAD_MUTEX_UNLOCK(&proxy_mutex);

				/*
				 *	This is bad.  However, the
				 *	packet list now supports 256
				 *	open sockets, which should
				 *	minimize this problem.
				 */
				ERROR("Failed adding proxy socket: %s",
				       fr_strerror());
				return 0;
			}

			if (sock->home) {
				sock->home->limit.num_connections++;

#ifdef HAVE_PTHREAD_H
				/*
				 *	If necessary, add it to the list of
				 *	new proxy listeners.
				 */
				if (sock->home->limit.lifetime || sock->home->limit.idle_timeout) {
					this->next = proxy_listener_list;
					proxy_listener_list = this;
				}
#endif
			}
			PTHREAD_MUTEX_UNLOCK(&proxy_mutex);

			/*
			 *	Tell the main thread that we've added
			 *	a proxy listener, but only if we need
			 *	to update the event list.  Do this
			 *	with the mutex unlocked, to reduce
			 *	contention.
			 */
			if (sock->home) {
				if (sock->home->limit.lifetime || sock->home->limit.idle_timeout) {
					radius_signal_self(RADIUS_SIGNAL_SELF_NEW_FD);
				}
			}
		}
#endif

#ifdef WITH_DETAIL
		/*
		 *	Detail files are always known, and aren't
		 *	put into the socket event loop.
		 */
		if (this->type == RAD_LISTEN_DETAIL) {
			this->status = RAD_LISTEN_STATUS_KNOWN;

			/*
			 *	Set up the first poll interval.
			 */
			event_poll_detail(this);
			return 1;
		}
#endif

#ifdef WITH_TCP
		/*
		 *	Add timers to child sockets, if necessary.
		 */
		if (sock->proto == IPPROTO_TCP && sock->opened &&
		    (sock->limit.lifetime || sock->limit.idle_timeout)) {
			struct timeval when;

			ASSERT_MASTER;

			when.tv_sec = sock->opened + 1;
			when.tv_usec = 0;

			if (!fr_event_insert(el, tcp_socket_timer, this, &when,
					     &(sock->ev))) {
				rad_panic("Failed to insert event");
			}
		}
#endif

		FD_MUTEX_LOCK(&fd_mutex);
		if (!fr_event_fd_insert(el, 0, this->fd,
					event_socket_handler, this)) {
			ERROR("Failed adding event handler for socket!");
			fr_exit(1);
		}
		FD_MUTEX_UNLOCK(&fd_mutex);

		this->status = RAD_LISTEN_STATUS_KNOWN;
		return 1;
	} /* end of INIT */

#ifdef WITH_TCP
	/*
	 *	Stop using this socket, if at all possible.
	 */
	if (this->status == RAD_LISTEN_STATUS_EOL) {
#ifdef WITH_PROXY
		/*
		 *	Proxy sockets get frozen, so that we don't use
		 *	them for new requests.  But we do keep them
		 *	open to listen for replies to requests we had
		 *	previously sent.
		 */
		if (this->type == RAD_LISTEN_PROXY) {
			PTHREAD_MUTEX_LOCK(&proxy_mutex);
			if (!fr_packet_list_socket_freeze(proxy_list,
							  this->fd)) {
				radlog(L_ERR, "Fatal error freezing socket: %s",
				       fr_strerror());
				fr_exit(1);
			}
			PTHREAD_MUTEX_UNLOCK(&proxy_mutex);
		}
#endif

		/*
		 *	Requests are still using the socket.  Wait for
		 *	them to finish.
		 */
		if (this->count > 0) {
			struct timeval when;
			listen_socket_t *sock = this->data;

			/*
			 *	Try again to clean up the socket in 30
			 *	seconds.
			 */
			gettimeofday(&when, NULL);
			when.tv_sec += 30;

			if (!fr_event_insert(el,
					     (fr_event_callback_t) event_new_fd,
					     this, &when, &sock->ev)) {
				rad_panic("Failed to insert event");
			}

			return 1;
		}

		/*
		 *	No one is using the socket.  We can remove it now.
		 */
		this->status = RAD_LISTEN_STATUS_REMOVE_NOW;
	} /* socket is at EOL */
#endif

	/*
	 *	Nuke the socket.
	 */
	if (this->status == RAD_LISTEN_STATUS_REMOVE_NOW) {
		int devnull;
#ifdef WITH_TCP
		listen_socket_t *sock = this->data;
#endif

		/*
		 *	Remove it from the list of live FD's.
		 */
		FD_MUTEX_LOCK(&fd_mutex);
		fr_event_fd_delete(el, 0, this->fd);
		FD_MUTEX_UNLOCK(&fd_mutex);

		/*
		 *      Re-open the socket, pointing it to /dev/null.
		 *      This means that all writes proceed without
		 *      blocking, and all reads return "no data".
		 *
		 *      This leaves the socket active, so any child
		 *      threads won't go insane.  But it means that
		 *      they cannot send or receive any packets.
		 *
		 *	This is EXTRA work in the normal case, when
		 *	sockets are closed without error.  But it lets
		 *	us have one simple processing method for all
		 *	sockets.
		 */
		devnull = open("/dev/null", O_RDWR);
		if (devnull < 0) {
			ERROR("FATAL failure opening /dev/null: %s",
			       strerror(errno));
			fr_exit(1);
		}
		if (dup2(devnull, this->fd) < 0) {
			ERROR("FATAL failure closing socket: %s",
			       strerror(errno));
			fr_exit(1);
		}
		close(devnull);

#ifdef WITH_DETAIL
		rad_assert(this->type != RAD_LISTEN_DETAIL);
#endif

#ifdef WITH_TCP
		INFO(" ... closing socket %s", buffer);

#ifdef WITH_PROXY
		/*
		 *	The socket is dead.  Force all proxied packets
		 *	to stop using it.  And then remove it from the
		 *	list of outgoing sockets.
		 */
		if (this->type == RAD_LISTEN_PROXY) {
			PTHREAD_MUTEX_LOCK(&proxy_mutex);
			fr_packet_list_walk(proxy_list, this,
					    eol_proxy_listener);

			if (!fr_packet_list_socket_del(proxy_list, this->fd)) {
				ERROR("Fatal error removing socket: %s",
				      fr_strerror());
				fr_exit(1);
			}
			if (sock->home &&  sock->home->limit.num_connections) {
				sock->home->limit.num_connections--;
			}
			PTHREAD_MUTEX_UNLOCK(&proxy_mutex);
		} else
#endif
		{
			/*
			 *	EOL all requests using this socket.
			 */
			fr_packet_list_walk(proxy_list, this,
					    eol_listener);
		}

		/*
		 *	Remove any pending cleanups.
		 */
		if (sock->ev) fr_event_delete(el, &sock->ev);

		/*
		 *	And finally, close the socket.
		 */
		listen_free(&this);
	}
#endif	/* WITH_TCP */

	return 1;
}

/***********************************************************************
 *
 *	Signal handlers.
 *
 ***********************************************************************/

static void handle_signal_self(int flag)
{
	if ((flag & (RADIUS_SIGNAL_SELF_EXIT | RADIUS_SIGNAL_SELF_TERM)) != 0) {
		if ((flag & RADIUS_SIGNAL_SELF_EXIT) != 0) {
			INFO("Signalled to exit");
			fr_event_loop_exit(el, 1);
		} else {
			INFO("Signalled to terminate");
			exec_trigger(NULL, NULL, "server.signal.term", true);
			fr_event_loop_exit(el, 2);
		}

		return;
	} /* else exit/term flags weren't set */

	/*
	 *	Tell the even loop to stop processing.
	 */
	if ((flag & RADIUS_SIGNAL_SELF_HUP) != 0) {
		time_t when;
		static time_t last_hup = 0;

		when = time(NULL);
		if ((int) (when - last_hup) < 5) {
			INFO("Ignoring HUP (less than 5s since last one)");
			return;
		}

		INFO("Received HUP signal.");

		last_hup = when;

		exec_trigger(NULL, NULL, "server.signal.hup", true);
		fr_event_loop_exit(el, 0x80);
	}

#ifdef WITH_DETAIL
	if ((flag & RADIUS_SIGNAL_SELF_DETAIL) != 0) {
		rad_listen_t *this;

		/*
		 *	FIXME: O(N) loops suck.
		 */
		for (this = mainconfig.listen;
		     this != NULL;
		     this = this->next) {
			if (this->type != RAD_LISTEN_DETAIL) continue;

			/*
			 *	This one didn't send the signal, skip
			 *	it.
			 */
			if (!this->decode(this, NULL)) continue;

			/*
			 *	Go service the interrupt.
			 */
			event_poll_detail(this);
		}
	}
#endif

#ifdef WITH_TCP
#ifdef WITH_PROXY
#ifdef HAVE_PTHREAD_H
	/*
	 *	Add event handlers for idle timeouts && maximum lifetime.
	 */
	if ((flag & RADIUS_SIGNAL_SELF_NEW_FD) != 0) {
		struct timeval when, now;

		fr_event_now(el, &now);

		PTHREAD_MUTEX_LOCK(&proxy_mutex);

		while (proxy_listener_list) {
			rad_listen_t *this = proxy_listener_list;
			listen_socket_t *sock = this->data;

			rad_assert(sock->proto == IPPROTO_TCP);
			proxy_listener_list = this->next;
			this->next = NULL;

			if (!sock->home) continue; /* skip UDP sockets */

			when = now;

			/*
			 *	Sockets should only be added to the
			 *	proxy_listener_list if they have limits.
			 *
			 */
			rad_assert(sock->home->limit.lifetime || sock->home->limit.idle_timeout);

			if (!fr_event_insert(el, tcp_socket_timer, this, &when,
					     &(sock->ev))) {
				rad_panic("Failed to insert event");
			}
		}

		PTHREAD_MUTEX_UNLOCK(&proxy_mutex);
	}
#endif	/* HAVE_PTHREAD_H */
#endif	/* WITH_PROXY */
#endif	/* WITH_TCP */
}

#ifndef WITH_SELF_PIPE
void radius_signal_self(int flag)
{
	handle_signal_self(flag);
}
#else
/*
 *	Inform ourselves that we received a signal.
 */
void radius_signal_self(int flag)
{
	ssize_t rcode;
	uint8_t buffer[16];

	/*
	 *	The read MUST be non-blocking for this to work.
	 */
	rcode = read(self_pipe[0], buffer, sizeof(buffer));
	if (rcode > 0) {
		ssize_t i;

		for (i = 0; i < rcode; i++) {
			buffer[0] |= buffer[i];
		}
	} else {
		buffer[0] = 0;
	}

	buffer[0] |= flag;

	write(self_pipe[1], buffer, 1);
}


static void event_signal_handler(UNUSED fr_event_list_t *xel,
				 UNUSED int fd, UNUSED void *ctx)
{
	ssize_t i, rcode;
	uint8_t buffer[32];

	rcode = read(self_pipe[0], buffer, sizeof(buffer));
	if (rcode <= 0) return;

	/*
	 *	Merge pending signals.
	 */
	for (i = 0; i < rcode; i++) {
		buffer[0] |= buffer[i];
	}

	handle_signal_self(buffer[0]);
}
#endif

/***********************************************************************
 *
 *	Bootstrapping code.
 *
 ***********************************************************************/

/*
 *	Externally-visibly functions.
 */
int radius_event_init(CONF_SECTION *cs, int have_children)
{
	rad_listen_t *head = NULL;

	if (el) return 0;

	time(&fr_start_time);

	el = fr_event_list_create(event_status);
	if (!el) return 0;

	pl = fr_packet_list_create(0);
	if (!pl) return 0;	/* leak el */

	request_num_counter = 0;

#ifdef WITH_PROXY
	if (mainconfig.proxy_requests) {
		/*
		 *	Create the tree for managing proxied requests and
		 *	responses.
		 */
		proxy_list = fr_packet_list_create(1);
		if (!proxy_list) return 0;

#ifdef HAVE_PTHREAD_H
		if (pthread_mutex_init(&proxy_mutex, NULL) != 0) {
			ERROR("FATAL: Failed to initialize proxy mutex: %s",
			       strerror(errno));
			fr_exit(1);
		}
#endif
	}
#endif

#ifdef HAVE_PTHREAD_H
	NO_SUCH_CHILD_PID = pthread_self(); /* not a child thread */

	/*
	 *	Initialize the threads ONLY if we're spawning, AND
	 *	we're running normally.
	 */
	if (have_children && !check_config &&
	    (thread_pool_init(cs, &have_children) < 0)) {
		fr_exit(1);
	}
#endif

	/*
	 *	Move all of the thread calls to this file?
	 *
	 *	It may be best for the mutexes to be in this file...
	 */
	spawn_flag = have_children;

	if (check_config) {
		DEBUG("%s: #### Skipping IP addresses and Ports ####",
		       mainconfig.name);
		if (listen_init(cs, &head, spawn_flag) < 0) {
			fflush(NULL);
			fr_exit(1);
		}
		return 1;
	}

#ifdef WITH_SELF_PIPE
	/*
	 *	Child threads need a pipe to signal us, as do the
	 *	signal handlers.
	 */
	if (pipe(self_pipe) < 0) {
		ERROR("radiusd: Error opening internal pipe: %s",
		       strerror(errno));
		fr_exit(1);
	}
	if ((fcntl(self_pipe[0], F_SETFL, O_NONBLOCK) < 0) ||
	    (fcntl(self_pipe[0], F_SETFD, FD_CLOEXEC) < 0)) {
		ERROR("radiusd: Error setting internal flags: %s",
		       strerror(errno));
		fr_exit(1);
	}
	if ((fcntl(self_pipe[1], F_SETFL, O_NONBLOCK) < 0) ||
	    (fcntl(self_pipe[1], F_SETFD, FD_CLOEXEC) < 0)) {
		ERROR("radiusd: Error setting internal flags: %s",
		       strerror(errno));
		fr_exit(1);
	}

	if (!fr_event_fd_insert(el, 0, self_pipe[0],
				  event_signal_handler, el)) {
		ERROR("Failed creating handler for signals");
		fr_exit(1);
	}
#endif	/* WITH_SELF_PIPE */

       DEBUG("%s: #### Opening IP addresses and Ports ####",
	       mainconfig.name);

       /*
	*	The server temporarily switches to an unprivileged
	*	user very early in the bootstrapping process.
	*	However, some sockets MAY require privileged access
	*	(bind to device, or to port < 1024, or to raw
	*	sockets).  Those sockets need to call suid up/down
	*	themselves around the functions that need a privileged
	*	uid.
	*/
       if (listen_init(cs, &head, spawn_flag) < 0) {
		fr_exit_now(1);
	}

	mainconfig.listen = head;

	/*
	 *	At this point, no one has any business *ever* going
	 *	back to root uid.
	 */
	fr_suid_down_permanent();

	return 1;
}


static int request_hash_cb(UNUSED void *ctx, void *data)
{
	REQUEST *request = fr_packet2myptr(REQUEST, packet, data);

#ifdef WITH_PROXY
	rad_assert(request->in_proxy_hash == false);
#endif

	request_done(request, FR_ACTION_DONE);

	return 0;
}


#ifdef WITH_PROXY
static int proxy_hash_cb(UNUSED void *ctx, void *data)
{
	REQUEST *request = fr_packet2myptr(REQUEST, proxy, data);

	request_done(request, FR_ACTION_DONE);

	return 0;
}
#endif

void radius_event_free(void)
{
	/*
	 *	Stop and join all threads.
	 */
#ifdef HAVE_PTHREAD_H
	thread_pool_stop();
	ASSERT_MASTER;
#endif

#ifdef WITH_PROXY
	/*
	 *	There are requests in the proxy hash that aren't
	 *	referenced from anywhere else.  Remove them first.
	 */
	if (proxy_list) {
		fr_packet_list_walk(proxy_list, NULL, proxy_hash_cb);
		fr_packet_list_free(proxy_list);
		proxy_list = NULL;
	}
#endif

	fr_packet_list_walk(pl, NULL, request_hash_cb);

	fr_packet_list_free(pl);
	pl = NULL;

	fr_event_list_free(el);
}

int radius_event_process(void)
{
	if (!el) return 0;

	return fr_event_loop(el);
}
