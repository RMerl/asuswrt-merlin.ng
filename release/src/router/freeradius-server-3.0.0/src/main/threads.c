/*
 * threads.c	request threading support
 *
 * Version:	$Id$
 *
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
 *
 * Copyright 2000,2006  The FreeRADIUS server project
 * Copyright 2000  Alan DeKok <aland@ox.org>
 */

RCSID("$Id$")
USES_APPLE_DEPRECATED_API	/* OpenSSL API has been deprecated by Apple */

#include <freeradius-devel/radiusd.h>
#include <freeradius-devel/process.h>
#include <freeradius-devel/rad_assert.h>

/*
 *	Other OS's have sem_init, OS X doesn't.
 */
#ifdef HAVE_SEMAPHORE_H
#include <semaphore.h>
#endif

#ifdef __APPLE__
#ifdef WITH_GCD
#include <dispatch/dispatch.h>
#endif
#include <mach/task.h>
#include <mach/mach_init.h>
#include <mach/semaphore.h>

#undef sem_t
#define sem_t semaphore_t
#undef sem_init
#define sem_init(s,p,c) semaphore_create(mach_task_self(),s,SYNC_POLICY_FIFO,c)
#undef sem_wait
#define sem_wait(s) semaphore_wait(*s)
#undef sem_post
#define sem_post(s) semaphore_signal(*s)
#endif

#ifdef HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif

#ifdef HAVE_PTHREAD_H

#ifdef HAVE_OPENSSL_CRYPTO_H
#include <openssl/crypto.h>
#endif
#ifdef HAVE_OPENSSL_ERR_H
#include <openssl/err.h>
#endif
#ifdef HAVE_OPENSSL_EVP_H
#include <openssl/evp.h>
#endif

#ifndef WITH_GCD
#define SEMAPHORE_LOCKED	(0)
#define SEMAPHORE_UNLOCKED	(1)

#define THREAD_RUNNING		(1)
#define THREAD_CANCELLED	(2)
#define THREAD_EXITED		(3)

#define NUM_FIFOS	       RAD_LISTEN_MAX

/*
 *  A data structure which contains the information about
 *  the current thread.
 *
 *  pthread_id     pthread id
 *  thread_num     server thread number, 1...number of threads
 *  semaphore     used to block the thread until a request comes in
 *  status	is the thread running or exited?
 *  request_count the number of requests that this thread has handled
 *  timestamp     when the thread started executing.
 */
typedef struct THREAD_HANDLE {
	struct THREAD_HANDLE *prev;
	struct THREAD_HANDLE *next;
	pthread_t	    pthread_id;
	int		  thread_num;
	int		  status;
	unsigned int	 request_count;
	time_t	       timestamp;
	REQUEST		     *request;
} THREAD_HANDLE;

#endif	/* WITH_GCD */

typedef struct thread_fork_t {
	pid_t		pid;
	int		status;
	int		exited;
} thread_fork_t;


#ifdef WITH_STATS
typedef struct fr_pps_t {
	int	pps_old;
	int	pps_now;
	int	pps;
	time_t	time_old;
} fr_pps_t;
#endif


/*
 *	A data structure to manage the thread pool.  There's no real
 *	need for a data structure, but it makes things conceptually
 *	easier.
 */
typedef struct THREAD_POOL {
#ifndef WITH_GCD
	THREAD_HANDLE *head;
	THREAD_HANDLE *tail;

	int active_threads;	/* protected by queue_mutex */
	int exited_threads;
	int total_threads;
	int max_thread_num;
	int start_threads;
	int max_threads;
	int min_spare_threads;
	int max_spare_threads;
	unsigned int max_requests_per_thread;
	unsigned long request_count;
	time_t time_last_spawned;
	int cleanup_delay;
	int stop_flag;
#endif	/* WITH_GCD */
	int spawn_flag;

#ifdef WNOHANG
	pthread_mutex_t	wait_mutex;
	fr_hash_table_t *waiters;
#endif

#ifdef WITH_GCD
	dispatch_queue_t	queue;
#else

#ifdef WITH_STATS
	fr_pps_t	pps_in, pps_out;
#ifdef WITH_ACCOUNTING
	int		auto_limit_acct;
#endif
#endif

	/*
	 *	All threads wait on this semaphore, for requests
	 *	to enter the queue.
	 */
	sem_t		semaphore;

	/*
	 *	To ensure only one thread at a time touches the queue.
	 */
	pthread_mutex_t	queue_mutex;

	int		max_queue_size;
	int		num_queued;
	fr_fifo_t	*fifo[NUM_FIFOS];
#endif	/* WITH_GCD */
} THREAD_POOL;

static THREAD_POOL thread_pool;
static int pool_initialized = false;

#ifndef WITH_GCD
static time_t last_cleaned = 0;

static void thread_pool_manage(time_t now);
#endif

#ifndef WITH_GCD
/*
 *	A mapping of configuration file names to internal integers
 */
static const CONF_PARSER thread_config[] = {
	{ "start_servers",	   PW_TYPE_INTEGER, 0, &thread_pool.start_threads,	   "5" },
	{ "max_servers",	     PW_TYPE_INTEGER, 0, &thread_pool.max_threads,	     "32" },
	{ "min_spare_servers",       PW_TYPE_INTEGER, 0, &thread_pool.min_spare_threads,       "3" },
	{ "max_spare_servers",       PW_TYPE_INTEGER, 0, &thread_pool.max_spare_threads,       "10" },
	{ "max_requests_per_server", PW_TYPE_INTEGER, 0, &thread_pool.max_requests_per_thread, "0" },
	{ "cleanup_delay",	   PW_TYPE_INTEGER, 0, &thread_pool.cleanup_delay,	   "5" },
	{ "max_queue_size",	  PW_TYPE_INTEGER, 0, &thread_pool.max_queue_size,	  "65536" },
#ifdef WITH_STATS
#ifdef WITH_ACCOUNTING
	{ "auto_limit_acct",	     PW_TYPE_BOOLEAN, 0, &thread_pool.auto_limit_acct, NULL },
#endif
#endif
	{ NULL, -1, 0, NULL, NULL }
};
#endif

#ifdef HAVE_OPENSSL_CRYPTO_H

/*
 *	If we're linking against OpenSSL, then it is the
 *	duty of the application, if it is multithreaded,
 *	to provide OpenSSL with appropriate thread id
 *	and mutex locking functions
 *
 *	Note: this only implements static callbacks.
 *	OpenSSL does not use dynamic locking callbacks
 *	right now, but may in the future, so we will have
 *	to add them at some point.
 */

static pthread_mutex_t *ssl_mutexes = NULL;

static unsigned long ssl_id_function(void)
{
	return (unsigned long) pthread_self();
}

static void ssl_locking_function(int mode, int n, UNUSED char const *file, UNUSED int line)
{
	if (mode & CRYPTO_LOCK) {
		pthread_mutex_lock(&(ssl_mutexes[n]));
	} else {
		pthread_mutex_unlock(&(ssl_mutexes[n]));
	}
}

static int setup_ssl_mutexes(void)
{
	int i;

#ifdef HAVE_OPENSSL_EVP_H
	/*
	 *	Enable all ciphers and digests.
	 */
	OpenSSL_add_all_algorithms();
#endif

	ssl_mutexes = rad_malloc(CRYPTO_num_locks() * sizeof(pthread_mutex_t));
	if (!ssl_mutexes) {
		ERROR("Error allocating memory for SSL mutexes!");
		return 0;
	}

	for (i = 0; i < CRYPTO_num_locks(); i++) {
		pthread_mutex_init(&(ssl_mutexes[i]), NULL);
	}

	CRYPTO_set_id_callback(ssl_id_function);
	CRYPTO_set_locking_callback(ssl_locking_function);

	return 1;
}
#endif

#ifdef WNOHANG
/*
 *	We don't want to catch SIGCHLD for a host of reasons.
 *
 *	- exec_wait means that someone, somewhere, somewhen, will
 *	call waitpid(), and catch the child.
 *
 *	- SIGCHLD is delivered to a random thread, not the one that
 *	forked.
 *
 *	- if another thread catches the child, we have to coordinate
 *	with the thread doing the waiting.
 *
 *	- if we don't waitpid() for non-wait children, they'll be zombies,
 *	and will hang around forever.
 *
 */
static void reap_children(void)
{
	pid_t pid;
	int status;
	thread_fork_t mytf, *tf;


	pthread_mutex_lock(&thread_pool.wait_mutex);

	do {
	retry:
		pid = waitpid(0, &status, WNOHANG);
		if (pid <= 0) break;

		mytf.pid = pid;
		tf = fr_hash_table_finddata(thread_pool.waiters, &mytf);
		if (!tf) goto retry;

		tf->status = status;
		tf->exited = 1;
	} while (fr_hash_table_num_elements(thread_pool.waiters) > 0);

	pthread_mutex_unlock(&thread_pool.wait_mutex);
}
#else
#define reap_children()
#endif /* WNOHANG */

#ifndef WITH_GCD
/*
 *	Add a request to the list of waiting requests.
 *	This function gets called ONLY from the main handler thread...
 *
 *	This function should never fail.
 */
int request_enqueue(REQUEST *request)
{
	/*
	 *	If we haven't checked the number of child threads
	 *	in a while, OR if the thread pool appears to be full,
	 *	go manage it.
	 */
	if ((last_cleaned < request->timestamp) ||
	    (thread_pool.active_threads == thread_pool.total_threads) ||
	    (thread_pool.exited_threads > 0)) {
		thread_pool_manage(request->timestamp);
	}


	pthread_mutex_lock(&thread_pool.queue_mutex);

#ifdef WITH_STATS
#ifdef WITH_ACCOUNTING
	if (thread_pool.auto_limit_acct) {
		struct timeval now;

		/*
		 *	Throw away accounting requests if we're too
		 *	busy.  The NAS should retransmit these, and no
		 *	one should notice.
		 *
		 *	In contrast, we always try to process
		 *	authentication requests.  Those are more time
		 *	critical, and it's harder to determine which
		 *	we can throw away, and which we can keep.
		 *
		 *	We allow the queue to get half full before we
		 *	start worrying.  Even then, we still require
		 *	that the rate of input packets is higher than
		 *	the rate of outgoing packets.  i.e. the queue
		 *	is growing.
		 *
		 *	Once that happens, we roll a dice to see where
		 *	the barrier is for "keep" versus "toss".  If
		 *	the queue is smaller than the barrier, we
		 *	allow it.  If the queue is larger than the
		 *	barrier, we throw the packet away.  Otherwise,
		 *	we keep it.
		 *
		 *	i.e. the probability of throwing the packet
		 *	away increases from 0 (queue is half full), to
		 *	100 percent (queue is completely full).
		 *
		 *	A probabilistic approach allows us to process
		 *	SOME of the new accounting packets.
		 */
		if ((request->packet->code == PW_ACCOUNTING_REQUEST) &&
		    (thread_pool.num_queued > (thread_pool.max_queue_size / 2)) &&
		    (thread_pool.pps_in.pps_now > thread_pool.pps_out.pps_now)) {
			uint32_t prob;
			int keep;

			/*
			 *	Take a random value of how full we
			 *	want the queue to be.  It's OK to be
			 *	half full, but we get excited over
			 *	anything more than that.
			 */
			keep = (thread_pool.max_queue_size / 2);
			prob = fr_rand() & ((1 << 10) - 1);
			keep *= prob;
			keep >>= 10;
			keep += (thread_pool.max_queue_size / 2);

			/*
			 *	If the queue is larger than our dice
			 *	roll, we throw the packet away.
			 */
			if (thread_pool.num_queued > keep) {
				pthread_mutex_unlock(&thread_pool.queue_mutex);
				return 0;
			}
		}

		gettimeofday(&now, NULL);

		/*
		 *	Calculate the instantaneous arrival rate into
		 *	the queue.
		 */
		thread_pool.pps_in.pps = rad_pps(&thread_pool.pps_in.pps_old,
						 &thread_pool.pps_in.pps_now,
						 &thread_pool.pps_in.time_old,
						 &now);

		thread_pool.pps_in.pps_now++;
	}
#endif	/* WITH_ACCOUNTING */
#endif

	thread_pool.request_count++;

	if (thread_pool.num_queued >= thread_pool.max_queue_size) {
		int complain = false;
		time_t now;
		static time_t last_complained = 0;

		now = time(NULL);
		if (last_complained != now) {
			last_complained = now;
			complain = true;
		}

		pthread_mutex_unlock(&thread_pool.queue_mutex);

		/*
		 *	Mark the request as done.
		 */
		if (complain) {
			ERROR("Something is blocking the server.  There are %d packets in the queue, waiting to be processed.  Ignoring the new request.", thread_pool.max_queue_size);
		}
		return 0;
	}
	request->component = "<core>";
	request->module = "<queue>";

	/*
	 *	Push the request onto the appropriate fifo for that
	 */
	if (!fr_fifo_push(thread_pool.fifo[request->priority], request)) {
		pthread_mutex_unlock(&thread_pool.queue_mutex);
		ERROR("!!! ERROR !!! Failed inserting request %d into the queue", request->number);
		return 0;
	}

	thread_pool.num_queued++;

	pthread_mutex_unlock(&thread_pool.queue_mutex);

	/*
	 *	There's one more request in the queue.
	 *
	 *	Note that we're not touching the queue any more, so
	 *	the semaphore post is outside of the mutex.  This also
	 *	means that when the thread wakes up and tries to lock
	 *	the mutex, it will be unlocked, and there won't be
	 *	contention.
	 */
	sem_post(&thread_pool.semaphore);

	return 1;
}

/*
 *	Remove a request from the queue.
 */
static int request_dequeue(REQUEST **prequest)
{
	time_t blocked;
	static time_t last_complained = 0;
	static time_t total_blocked = 0;
	int num_blocked;
	RAD_LISTEN_TYPE i, start;
	REQUEST *request;
	reap_children();

	pthread_mutex_lock(&thread_pool.queue_mutex);

#ifdef WITH_STATS
#ifdef WITH_ACCOUNTING
	if (thread_pool.auto_limit_acct) {
		struct timeval now;

		gettimeofday(&now, NULL);

		/*
		 *	Calculate the instantaneous departure rate
		 *	from the queue.
		 */
		thread_pool.pps_out.pps  = rad_pps(&thread_pool.pps_out.pps_old,
						   &thread_pool.pps_out.pps_now,
						   &thread_pool.pps_out.time_old,
						   &now);
		thread_pool.pps_out.pps_now++;
	}
#endif
#endif

	/*
	 *	Clear old requests from all queues.
	 *
	 *	We only do one pass over the queue, in order to
	 *	amortize the work across the child threads.  Since we
	 *	do N checks for one request de-queued, the old
	 *	requests will be quickly cleared.
	 */
	for (i = 0; i < RAD_LISTEN_MAX; i++) {
		request = fr_fifo_peek(thread_pool.fifo[i]);
		if (!request) continue;

		VERIFY_REQUEST(request);

		if (request->master_state != REQUEST_STOP_PROCESSING) {
			continue;
		}

		/*
		 *	This entry was marked to be stopped.  Acknowledge it.
		 */
		request = fr_fifo_pop(thread_pool.fifo[i]);
		rad_assert(request != NULL);
		VERIFY_REQUEST(request);
		request->child_state = REQUEST_DONE;
		thread_pool.num_queued--;
	}

	start = 0;
 retry:
	/*
	 *	Pop results from the top of the queue
	 */
	for (i = start; i < RAD_LISTEN_MAX; i++) {
		request = fr_fifo_pop(thread_pool.fifo[i]);
		if (request) {
			VERIFY_REQUEST(request);
			start = i;
			break;
		}
	}

	if (!request) {
		pthread_mutex_unlock(&thread_pool.queue_mutex);
		*prequest = NULL;
		return 0;
	}

	rad_assert(thread_pool.num_queued > 0);
	thread_pool.num_queued--;
	*prequest = request;

	rad_assert(*prequest != NULL);
	rad_assert(request->magic == REQUEST_MAGIC);

	request->component = "<core>";
	request->module = "";

	/*
	 *	If the request has sat in the queue for too long,
	 *	kill it.
	 *
	 *	The main clean-up code can't delete the request from
	 *	the queue, and therefore won't clean it up until we
	 *	have acknowledged it as "done".
	 */
	if (request->master_state == REQUEST_STOP_PROCESSING) {
		request->module = "<done>";
		request->child_state = REQUEST_DONE;
		goto retry;
	}

	/*
	 *	The thread is currently processing a request.
	 */
	thread_pool.active_threads++;

	blocked = time(NULL);
	if ((blocked - request->timestamp) > 5) {
		total_blocked++;
		if (last_complained < blocked) {
			last_complained = blocked;
			blocked -= request->timestamp;
			num_blocked = total_blocked;
		} else {
			blocked = 0;
		}
	} else {
		total_blocked = 0;
		blocked = 0;
	}

	pthread_mutex_unlock(&thread_pool.queue_mutex);

	if (blocked) {
		ERROR("%d requests have been waiting in the processing queue for %d seconds.  Check that all databases are running properly!",
		      num_blocked, (int) blocked);
	}

	return 1;
}


/*
 *	The main thread handler for requests.
 *
 *	Wait on the semaphore until we have it, and process the request.
 */
static void *request_handler_thread(void *arg)
{
	THREAD_HANDLE	  *self = (THREAD_HANDLE *) arg;

	/*
	 *	Loop forever, until told to exit.
	 */
	do {
		/*
		 *	Wait to be signalled.
		 */
		DEBUG2("Thread %d waiting to be assigned a request",
		       self->thread_num);
	re_wait:
		if (sem_wait(&thread_pool.semaphore) != 0) {
			/*
			 *	Interrupted system call.  Go back to
			 *	waiting, but DON'T print out any more
			 *	text.
			 */
			if (errno == EINTR) {
				DEBUG2("Re-wait %d", self->thread_num);
				goto re_wait;
			}
			ERROR("Thread %d failed waiting for semaphore: %s: Exiting\n",
			       self->thread_num, strerror(errno));
			break;
		}

		DEBUG2("Thread %d got semaphore", self->thread_num);

#ifdef HAVE_OPENSSL_ERR_H
 		/*
		 *	Clear the error queue for the current thread.
		 */
		ERR_clear_error ();
#endif

		/*
		 *	The server is exiting.  Don't dequeue any
		 *	requests.
		 */
		if (thread_pool.stop_flag) break;

		/*
		 *	Try to grab a request from the queue.
		 *
		 *	It may be empty, in which case we fail
		 *	gracefully.
		 */
		if (!request_dequeue(&self->request)) continue;

		self->request->child_pid = self->pthread_id;
		self->request_count++;

		DEBUG2("Thread %d handling request %d, (%d handled so far)",
		       self->thread_num, self->request->number,
		       self->request_count);

		if ((self->request->packet->code == PW_ACCOUNTING_REQUEST) &&
		    thread_pool.auto_limit_acct) {
			VALUE_PAIR *vp;
			REQUEST *request = self->request;

			vp = radius_paircreate(request, &request->config_items,
					       181, VENDORPEC_FREERADIUS);
			if (vp) vp->vp_integer = thread_pool.pps_in.pps;

			vp = radius_paircreate(request, &request->config_items,
					       182, VENDORPEC_FREERADIUS);
			if (vp) vp->vp_integer = thread_pool.pps_in.pps;

			vp = radius_paircreate(request, &request->config_items,
					       183, VENDORPEC_FREERADIUS);
			if (vp) {
				vp->vp_integer = thread_pool.max_queue_size - thread_pool.num_queued;
				vp->vp_integer *= 100;
				vp->vp_integer /= thread_pool.max_queue_size;
			}
		}

		self->request->process(self->request, FR_ACTION_RUN);
		self->request = NULL;

		/*
		 *	Update the active threads.
		 */
		pthread_mutex_lock(&thread_pool.queue_mutex);
		rad_assert(thread_pool.active_threads > 0);
		thread_pool.active_threads--;
		pthread_mutex_unlock(&thread_pool.queue_mutex);

		/*
		 *	If the thread has handled too many requests, then make it
		 *	exit.
		 */
		if ((thread_pool.max_requests_per_thread > 0) &&
		    (self->request_count >= thread_pool.max_requests_per_thread)) {
			DEBUG2("Thread %d handled too many requests",
			       self->thread_num);
			break;
		}
	} while (self->status != THREAD_CANCELLED);

	DEBUG2("Thread %d exiting...", self->thread_num);

#ifdef HAVE_OPENSSL_ERR_H
	/*
	 *	If we linked with OpenSSL, the application
	 *	must remove the thread's error queue before
	 *	exiting to prevent memory leaks.
	 */
	ERR_remove_state(0);
#endif

	pthread_mutex_lock(&thread_pool.queue_mutex);
	thread_pool.exited_threads++;
	pthread_mutex_unlock(&thread_pool.queue_mutex);

	/*
	 *  Do this as the LAST thing before exiting.
	 */
	self->request = NULL;
	self->status = THREAD_EXITED;
	exec_trigger(NULL, NULL, "server.thread.stop", true);

	return NULL;
}

/*
 *	Take a THREAD_HANDLE, delete it from the thread pool and
 *	free its resources.
 *
 *	This function is called ONLY from the main server thread,
 *	ONLY after the thread has exited.
 */
static void delete_thread(THREAD_HANDLE *handle)
{
	THREAD_HANDLE *prev;
	THREAD_HANDLE *next;

	rad_assert(handle->request == NULL);

	DEBUG2("Deleting thread %d", handle->thread_num);

	prev = handle->prev;
	next = handle->next;
	rad_assert(thread_pool.total_threads > 0);
	thread_pool.total_threads--;

	/*
	 *	Remove the handle from the list.
	 */
	if (prev == NULL) {
		rad_assert(thread_pool.head == handle);
		thread_pool.head = next;
	} else {
		prev->next = next;
	}

	if (next == NULL) {
		rad_assert(thread_pool.tail == handle);
		thread_pool.tail = prev;
	} else {
		next->prev = prev;
	}

	/*
	 *	Free the handle, now that it's no longer referencable.
	 */
	free(handle);
}


/*
 *	Spawn a new thread, and place it in the thread pool.
 *
 *	The thread is started initially in the blocked state, waiting
 *	for the semaphore.
 */
static THREAD_HANDLE *spawn_thread(time_t now, int do_trigger)
{
	int rcode;
	THREAD_HANDLE *handle;

	/*
	 *	Ensure that we don't spawn too many threads.
	 */
	if (thread_pool.total_threads >= thread_pool.max_threads) {
		DEBUG2("Thread spawn failed.  Maximum number of threads (%d) already running.", thread_pool.max_threads);
		exec_trigger(NULL, NULL, "server.thread.max_threads", true);
		return NULL;
	}

	/*
	 *	Allocate a new thread handle.
	 */
	handle = (THREAD_HANDLE *) rad_malloc(sizeof(THREAD_HANDLE));
	memset(handle, 0, sizeof(THREAD_HANDLE));
	handle->prev = NULL;
	handle->next = NULL;
	handle->thread_num = thread_pool.max_thread_num++;
	handle->request_count = 0;
	handle->status = THREAD_RUNNING;
	handle->timestamp = time(NULL);

	/*
	 *	Create the thread joinable, so that it can be cleaned up
	 *	using pthread_join().
	 *
	 *	Note that the function returns non-zero on error, NOT
	 *	-1.  The return code is the error, and errno isn't set.
	 */
	rcode = pthread_create(&handle->pthread_id, 0,
			request_handler_thread, handle);
	if (rcode != 0) {
		ERROR("Thread create failed: %s",
		       strerror(rcode));
		return NULL;
	}

	/*
	 *	One more thread to go into the list.
	 */
	thread_pool.total_threads++;
	DEBUG2("Thread spawned new child %d. Total threads in pool: %d",
			handle->thread_num, thread_pool.total_threads);
	if (do_trigger) exec_trigger(NULL, NULL, "server.thread.start", true);

	/*
	 *	Add the thread handle to the tail of the thread pool list.
	 */
	if (thread_pool.tail) {
		thread_pool.tail->next = handle;
		handle->prev = thread_pool.tail;
		thread_pool.tail = handle;
	} else {
		rad_assert(thread_pool.head == NULL);
		thread_pool.head = thread_pool.tail = handle;
	}

	/*
	 *	Update the time we last spawned a thread.
	 */
	thread_pool.time_last_spawned = now;

	/*
	 *	And return the new handle to the caller.
	 */
	return handle;
}
#endif	/* WITH_GCD */


#ifdef WNOHANG
static uint32_t pid_hash(void const *data)
{
	thread_fork_t const *tf = data;

	return fr_hash(&tf->pid, sizeof(tf->pid));
}

static int pid_cmp(void const *one, void const *two)
{
	thread_fork_t const *a = one;
	thread_fork_t const *b = two;

	return (a->pid - b->pid);
}
#endif

/*
 *	Allocate the thread pool, and seed it with an initial number
 *	of threads.
 *
 *	FIXME: What to do on a SIGHUP???
 */
int thread_pool_init(UNUSED CONF_SECTION *cs, int *spawn_flag)
{
#ifndef WITH_GCD
	int		i, rcode;
	CONF_SECTION	*pool_cf;
#endif
	time_t		now;

	now = time(NULL);

	rad_assert(spawn_flag != NULL);
	rad_assert(*spawn_flag == true);
	rad_assert(pool_initialized == false); /* not called on HUP */

#ifndef WITH_GCD
	pool_cf = cf_subsection_find_next(cs, NULL, "thread");
	if (!pool_cf) *spawn_flag = false;
#endif

	/*
	 *	Initialize the thread pool to some reasonable values.
	 */
	memset(&thread_pool, 0, sizeof(THREAD_POOL));
#ifndef WITH_GCD
	thread_pool.head = NULL;
	thread_pool.tail = NULL;
	thread_pool.total_threads = 0;
	thread_pool.max_thread_num = 1;
	thread_pool.cleanup_delay = 5;
	thread_pool.stop_flag = 0;
#endif
	thread_pool.spawn_flag = *spawn_flag;

	/*
	 *	Don't bother initializing the mutexes or
	 *	creating the hash tables.  They won't be used.
	 */
	if (!*spawn_flag) return 0;

#ifdef WNOHANG
	if ((pthread_mutex_init(&thread_pool.wait_mutex,NULL) != 0)) {
		ERROR("FATAL: Failed to initialize wait mutex: %s",
		       strerror(errno));
		return -1;
	}

	/*
	 *	Create the hash table of child PID's
	 */
	thread_pool.waiters = fr_hash_table_create(pid_hash,
						   pid_cmp,
						   free);
	if (!thread_pool.waiters) {
		ERROR("FATAL: Failed to set up wait hash");
		return -1;
	}
#endif

#ifndef WITH_GCD
	if (cf_section_parse(pool_cf, NULL, thread_config) < 0) {
		return -1;
	}

	/*
	 *	Catch corner cases.
	 */
	if (thread_pool.min_spare_threads < 1)
		thread_pool.min_spare_threads = 1;
	if (thread_pool.max_spare_threads < 1)
		thread_pool.max_spare_threads = 1;
	if (thread_pool.max_spare_threads < thread_pool.min_spare_threads)
		thread_pool.max_spare_threads = thread_pool.min_spare_threads;
	if (thread_pool.max_threads == 0)
		thread_pool.max_threads = 256;
	if ((thread_pool.max_queue_size < 2) || (thread_pool.max_queue_size > 1024*1024)) {
		ERROR("FATAL: max_queue_size value must be in range 2-1048576");
		return -1;
	}
#endif	/* WITH_GCD */

	/*
	 *	The pool has already been initialized.  Don't spawn
	 *	new threads, and don't forget about forked children.
	 */
	if (pool_initialized) {
		return 0;
	}

#ifndef WITH_GCD
	/*
	 *	Initialize the queue of requests.
	 */
	memset(&thread_pool.semaphore, 0, sizeof(thread_pool.semaphore));
	rcode = sem_init(&thread_pool.semaphore, 0, SEMAPHORE_LOCKED);
	if (rcode != 0) {
		ERROR("FATAL: Failed to initialize semaphore: %s",
		       strerror(errno));
		return -1;
	}

	rcode = pthread_mutex_init(&thread_pool.queue_mutex,NULL);
	if (rcode != 0) {
		ERROR("FATAL: Failed to initialize queue mutex: %s",
		       strerror(errno));
		return -1;
	}

	/*
	 *	Allocate multiple fifos.
	 */
	for (i = 0; i < RAD_LISTEN_MAX; i++) {
		thread_pool.fifo[i] = fr_fifo_create(thread_pool.max_queue_size, NULL);
		if (!thread_pool.fifo[i]) {
			ERROR("FATAL: Failed to set up request fifo");
			return -1;
		}
	}
#endif

#ifdef HAVE_OPENSSL_CRYPTO_H
	/*
	 *	If we're linking with OpenSSL too, then we need
	 *	to set up the mutexes and enable the thread callbacks.
	 */
	if (!setup_ssl_mutexes()) {
		ERROR("FATAL: Failed to set up SSL mutexes");
		return -1;
	}
#endif


#ifndef WITH_GCD
	/*
	 *	Create a number of waiting threads.
	 *
	 *	If we fail while creating them, do something intelligent.
	 */
	for (i = 0; i < thread_pool.start_threads; i++) {
		if (spawn_thread(now, 0) == NULL) {
			return -1;
		}
	}
#else
	thread_pool.queue = dispatch_queue_create("org.freeradius.threads", NULL);
	if (!thread_pool.queue) {
		ERROR("Failed creating dispatch queue: %s", strerror(errno));
		fr_exit(1);
	}
#endif

	DEBUG2("Thread pool initialized");
	pool_initialized = true;
	return 0;
}


/*
 *	Stop all threads in the pool.
 */
void thread_pool_stop(void)
{
#ifndef WITH_GCD
	int i;
	int total_threads;
	THREAD_HANDLE *handle;
	THREAD_HANDLE *next;

	/*
	 *	Set pool stop flag.
	 */
	thread_pool.stop_flag = 1;

	/*
	 *	Wakeup all threads to make them see stop flag.
	 */
	total_threads = thread_pool.total_threads;
	for (i = 0; i != total_threads; i++) {
		sem_post(&thread_pool.semaphore);
	}

	/*
	 *	Join and free all threads.
	 */
	for (handle = thread_pool.head; handle; handle = next) {
		next = handle->next;
		pthread_join(handle->pthread_id, NULL);
		delete_thread(handle);
	}
#endif
}


#ifdef WITH_GCD
int request_enqueue(REQUEST *request)
{
	dispatch_block_t block;

	block = ^{
		request->process(request, FR_ACTION_RUN);
	};

	dispatch_async(thread_pool.queue, block);

	return 1;
}
#endif

#ifndef WITH_GCD
/*
 *	Check the min_spare_threads and max_spare_threads.
 *
 *	If there are too many or too few threads waiting, then we
 *	either create some more, or delete some.
 */
static void thread_pool_manage(time_t now)
{
	int spare;
	int i, total;
	THREAD_HANDLE *handle, *next;
	int active_threads;

	/*
	 *	Loop over the thread pool, deleting exited threads.
	 */
	for (handle = thread_pool.head; handle; handle = next) {
		next = handle->next;

		/*
		 *	Maybe we've asked the thread to exit, and it
		 *	has agreed.
		 */
		if (handle->status == THREAD_EXITED) {
			pthread_join(handle->pthread_id, NULL);
			delete_thread(handle);
			pthread_mutex_lock(&thread_pool.queue_mutex);
			thread_pool.exited_threads--;
			pthread_mutex_unlock(&thread_pool.queue_mutex);
		}
	}

	/*
	 *	We don't need a mutex lock here, as we're reading
	 *	active_threads, and not modifying it.  We want a close
	 *	approximation of the number of active threads, and this
	 *	is good enough.
	 */
	active_threads = thread_pool.active_threads;
	spare = thread_pool.total_threads - active_threads;
	if (debug_flag) {
		static int old_total = -1;
		static int old_active = -1;

		if ((old_total != thread_pool.total_threads) ||
				(old_active != active_threads)) {
			DEBUG2("Threads: total/active/spare threads = %d/%d/%d",
					thread_pool.total_threads, active_threads, spare);
			old_total = thread_pool.total_threads;
			old_active = active_threads;
		}
	}

	/*
	 *	If there are too few spare threads.  Go create some more.
	 */
	if ((thread_pool.total_threads < thread_pool.max_threads) &&
	    (spare < thread_pool.min_spare_threads)) {
		total = thread_pool.min_spare_threads - spare;

		if ((total + thread_pool.total_threads) > thread_pool.max_threads) {
			total = thread_pool.max_threads - thread_pool.total_threads;
		}

		DEBUG2("Threads: Spawning %d spares", total);

		/*
		 *	Create a number of spare threads.
		 */
		for (i = 0; i < total; i++) {
			handle = spawn_thread(now, 1);
			if (handle == NULL) {
				return;
			}
		}

		return;		/* there aren't too many spare threads */
	}

	/*
	 *	Only delete spare threads if we haven't already done
	 *	so this second.
	 */
	if (now == last_cleaned) {
		return;
	}
	last_cleaned = now;

	/*
	 *	Only delete the spare threads if sufficient time has
	 *	passed since we last created one.  This helps to minimize
	 *	the amount of create/delete cycles.
	 */
	if ((now - thread_pool.time_last_spawned) < thread_pool.cleanup_delay) {
		return;
	}

	/*
	 *	If there are too many spare threads, delete one.
	 *
	 *	Note that we only delete ONE at a time, instead of
	 *	wiping out many.  This allows the excess servers to
	 *	be slowly reaped, just in case the load spike comes again.
	 */
	if (spare > thread_pool.max_spare_threads) {

		spare -= thread_pool.max_spare_threads;

		DEBUG2("Threads: deleting 1 spare out of %d spares", spare);

		/*
		 *	Walk through the thread pool, deleting the
		 *	first idle thread we come across.
		 */
		for (handle = thread_pool.head; (handle != NULL) && (spare > 0) ; handle = next) {
			next = handle->next;

			/*
			 *	If the thread is not handling a
			 *	request, but still live, then tell it
			 *	to exit.
			 *
			 *	It will eventually wake up, and realize
			 *	it's been told to commit suicide.
			 */
			if ((handle->request == NULL) &&
			    (handle->status == THREAD_RUNNING)) {
				handle->status = THREAD_CANCELLED;
				/*
				 *	Post an extra semaphore, as a
				 *	signal to wake up, and exit.
				 */
				sem_post(&thread_pool.semaphore);
				spare--;
				break;
			}
		}
	}

	/*
	 *	Otherwise everything's kosher.  There are not too few,
	 *	or too many spare threads.  Exit happily.
	 */
	return;
}
#endif	/* WITH_GCD */

#ifdef WNOHANG
/*
 *	Thread wrapper for fork().
 */
pid_t rad_fork(void)
{
	pid_t child_pid;

	if (!pool_initialized) return fork();

	reap_children();	/* be nice to non-wait thingies */

	if (fr_hash_table_num_elements(thread_pool.waiters) >= 1024) {
		return -1;
	}

	/*
	 *	Fork & save the PID for later reaping.
	 */
	child_pid = fork();
	if (child_pid > 0) {
		int rcode;
		thread_fork_t *tf;

		tf = rad_malloc(sizeof(*tf));
		memset(tf, 0, sizeof(*tf));

		tf->pid = child_pid;

		pthread_mutex_lock(&thread_pool.wait_mutex);
		rcode = fr_hash_table_insert(thread_pool.waiters, tf);
		pthread_mutex_unlock(&thread_pool.wait_mutex);

		if (!rcode) {
			ERROR("Failed to store PID, creating what will be a zombie process %d",
			       (int) child_pid);
			free(tf);
		}
	}

	/*
	 *	Return whatever we were told.
	 */
	return child_pid;
}


/*
 *	Wait 10 seconds at most for a child to exit, then give up.
 */
pid_t rad_waitpid(pid_t pid, int *status)
{
	int i;
	thread_fork_t mytf, *tf;

	if (!pool_initialized) return waitpid(pid, status, 0);

	if (pid <= 0) return -1;

	mytf.pid = pid;

	pthread_mutex_lock(&thread_pool.wait_mutex);
	tf = fr_hash_table_finddata(thread_pool.waiters, &mytf);
	pthread_mutex_unlock(&thread_pool.wait_mutex);

	if (!tf) return -1;

	for (i = 0; i < 100; i++) {
		reap_children();

		if (tf->exited) {
			*status = tf->status;

			pthread_mutex_lock(&thread_pool.wait_mutex);
			fr_hash_table_delete(thread_pool.waiters, &mytf);
			pthread_mutex_unlock(&thread_pool.wait_mutex);
			return pid;
		}
		usleep(100000);	/* sleep for 1/10 of a second */
	}

	/*
	 *	10 seconds have passed, give up on the child.
	 */
	pthread_mutex_lock(&thread_pool.wait_mutex);
	fr_hash_table_delete(thread_pool.waiters, &mytf);
	pthread_mutex_unlock(&thread_pool.wait_mutex);

	return 0;
}
#else
/*
 *	No rad_fork or rad_waitpid
 */
#endif

void thread_pool_queue_stats(int array[RAD_LISTEN_MAX], int pps[2])
{
	int i;

#ifndef WITH_GCD
	if (pool_initialized) {
		struct timeval now;

		for (i = 0; i < RAD_LISTEN_MAX; i++) {
			array[i] = fr_fifo_num_elements(thread_pool.fifo[i]);
		}

		gettimeofday(&now, NULL);

		pps[0] = rad_pps(&thread_pool.pps_in.pps_old,
				 &thread_pool.pps_in.pps_now,
				 &thread_pool.pps_in.time_old,
				 &now);
		pps[1] = rad_pps(&thread_pool.pps_out.pps_old,
				 &thread_pool.pps_out.pps_now,
				 &thread_pool.pps_out.time_old,
				 &now);

	} else
#endif	/* WITH_GCD */
	{
		for (i = 0; i < RAD_LISTEN_MAX; i++) {
			array[i] = 0;
		}

		pps[0] = pps[1] = 0;
	}
}
#endif /* HAVE_PTHREAD_H */

static void time_free(void *data)
{
	free(data);
}

void exec_trigger(REQUEST *request, CONF_SECTION *cs, char const *name, int quench)
{
	CONF_SECTION *subcs;
	CONF_ITEM *ci;
	CONF_PAIR *cp;
	char const *attr;
	char const *value;
	VALUE_PAIR *vp;

	/*
	 *	Use global "trigger" section if no local config is given.
	 */
	if (!cs) {
		cs = mainconfig.config;
		attr = name;
	} else {
		/*
		 *	Try to use pair name, rather than reference.
		 */
		attr = strrchr(name, '.');
		if (attr) {
			attr++;
		} else {
			attr = name;
		}
	}

	/*
	 *	Find local "trigger" subsection.  If it isn't found,
	 *	try using the global "trigger" section, and reset the
	 *	reference to the full path, rather than the sub-path.
	 */
	subcs = cf_section_sub_find(cs, "trigger");
	if (!subcs && (cs != mainconfig.config)) {
		subcs = cf_section_sub_find(mainconfig.config, "trigger");
		attr = name;
	}

	if (!subcs) return;

	ci = cf_reference_item(subcs, mainconfig.config, attr);
	if (!ci) {
		RDEBUG3("No such item in trigger section: %s", attr);
		return;
	}

	if (!cf_item_is_pair(ci)) {
		RDEBUG2("Trigger is not a configuration variable: %s", attr);
		return;
	}

	cp = cf_itemtopair(ci);
	if (!cp) return;

	value = cf_pair_value(cp);
	if (!value) {
		RDEBUG2("Trigger has no value: %s", name);
		return;
	}

	/*
	 *	May be called for Status-Server packets.
	 */
	vp = NULL;
	if (request && request->packet) vp = request->packet->vps;

	/*
	 *	Perform periodic quenching.
	 */
	if (quench) {
		time_t *last_time;

		last_time = cf_data_find(cs, value);
		if (!last_time) {
			last_time = rad_malloc(sizeof(*last_time));
			*last_time = 0;

			if (cf_data_add(cs, value, last_time, time_free) < 0) {
				free(last_time);
				last_time = NULL;
			}
		}

		/*
		 *	Send the quenched traps at most once per second.
		 */
		if (last_time) {
			time_t now = time(NULL);
			if (*last_time == now) return;

			*last_time = now;
		}
	}

	RDEBUG("Trigger %s -> %s", name, value);
	radius_exec_program(request, value, false, true, NULL, 0, vp, NULL);
}
