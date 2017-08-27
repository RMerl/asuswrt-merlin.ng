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

/**
 * @file connection.c
 * @brief Handle pools of connections (threads, sockets, etc.)
 * @note This API must be used by all modules in the public distribution that
 * maintain pools of connections.
 *
 * @copyright 2012  The FreeRADIUS server project
 * @copyright 2012  Alan DeKok <aland@deployingradius.com>
 */
RCSID("$Id$")

#include <freeradius-devel/radiusd.h>
#include <freeradius-devel/rad_assert.h>

typedef struct fr_connection fr_connection_t;

static int fr_connection_pool_check(fr_connection_pool_t *pool);

/** An individual connection within the connection pool
 *
 * Defines connection counters, timestamps, and holds a pointer to the
 * connection handle itself.
 *
 * @see fr_connection_pool_t
 */
struct fr_connection {
	fr_connection_t	*prev;		//!< Previous connection in list.
	fr_connection_t	*next;		//!< Next connection in list.

	time_t		created;	//!< Time connection was created.
	time_t		last_used;	//!< Last time the connection was
					//!< reserved.

	uint64_t	num_uses;	//!< Number of times the connection
					//!< has been reserved.
	int		in_use;		//!< Whether the connection is currently
					//!< reserved.
	uint64_t	number;		//!< Unique ID assigned when the
					//!< connection is created, these will
					//!< monotonically increase over the
					//!< lifetime of the connection pool.
	void		*connection;	//!< Pointer to whatever the module
					//!< uses for a connection handle.
};

/** A connection pool
 *
 * Defines the configuration of the connection pool, all the counters and
 * timestamps related to the connection pool, the mutex that stops multiple
 * threads leaving the pool in an inconsistent state, and the callbacks
 * required to open, close and check the status of connections within the pool.
 *
 * @see fr_connection
 */
struct fr_connection_pool_t {
	int		start;		//!< Number of initial connections
	int		min;		//!< Minimum number of concurrent
					//!< connections to keep open.
	int		max;		//!< Maximum number of concurrent
					//!< connections to allow.
	int		spare;		//!< Number of spare connections to try
					//!< and maintain.
	int		cleanup_delay;	//!< How long a connection can go unused
					//!< for before it's closed
					//!< (0 is infinite).
	uint64_t	max_uses;	//!< Maximum number of times a
					//!< connection can be used before being
					//!< closed.
	int		lifetime;	//!< How long a connection can be open
					//!< before being closed (irrespective
					//!< of whether it's idle or not).
	int		idle_timeout;	//!< How long a connection can be idle
					//!< before being closed.

	int		trigger;	//!< If true execute connection triggers
					//!< associated with the connection
					//!< pool.

	int		spread;		//!< If true requests will be spread
					//!< across all connections, instead of
					//!< re-using the most recently used
					//!< connections first.

	time_t		last_checked;	//!< Last time we pruned the connection
					//!< pool.
	time_t		last_spawned;	//!< Last time we spawned a connection.
	time_t		last_failed;	//!< Last time we tried to spawn a
					//!< a connection but failed.
	time_t		last_complained;//!< Last time we complained about
					//!< configuration parameters.
	time_t		last_throttled; //!< Last time we refused to spawn a
					//!< connection because the last
					//!< connection failed, or we were
					//!< already spawning a connection.
	time_t		last_at_max;	//!< Last time we hit the maximum number
					//!< of allowed connections.

	uint64_t	count;		//!< Number of connections spawned over
					//!< the lifetime of the pool.
	int		num;		//!< Number of connections in the pool.
	int		active;	 	//!< Number of currently reserved
					//!< connections.

	fr_connection_t	*head;		//!< Start of the connection list.
	fr_connection_t *tail;		//!< End of the connection list.

	bool		spawning;	//!< Whether we are currently attempting
					//!< to spawn a new connection.

#ifdef HAVE_PTHREAD_H
	pthread_mutex_t	mutex;		//!< Mutex used to keep consistent state
					//!< when making modifications in
					//!< threaded mode.
#endif

	CONF_SECTION	*cs;		//!< Configuration section holding
					//!< the section of parsed config file
					//!< that relates to this pool.
	void		*ctx;		//!< Pointer to context data that will
					//!< be passed to callbacks.

	char  		*log_prefix;	//!< Log prefix to prepend to all log
					//!< messages created by the connection
					//!< pool code.

	fr_connection_create_t	create;	//!< Function used to create new
					//!< connections.
	fr_connection_alive_t	alive;	//!< Function used to check status
					//!< of connections.
	fr_connection_delete_t	delete;	//!< Function used to close existing
					//!< connections.
};

#define LOG_PREFIX "rlm_%s (%s)"
#ifndef HAVE_PTHREAD_H
#define pthread_mutex_lock(_x)
#define pthread_mutex_unlock(_x)
#endif

static const CONF_PARSER connection_config[] = {
	{ "start",    PW_TYPE_INTEGER, offsetof(fr_connection_pool_t, start),
	  0, "5" },
	{ "min",      PW_TYPE_INTEGER, offsetof(fr_connection_pool_t, min),
	  0, "5" },
	{ "max",      PW_TYPE_INTEGER, offsetof(fr_connection_pool_t, max),
	  0, "10" },
	{ "spare",    PW_TYPE_INTEGER, offsetof(fr_connection_pool_t, spare),
	  0, "3" },
	{ "uses",     PW_TYPE_INTEGER, offsetof(fr_connection_pool_t, max_uses),
	  0, "0" },
	{ "lifetime", PW_TYPE_INTEGER, offsetof(fr_connection_pool_t, lifetime),
	  0, "0" },
	{ "cleanup_delay", PW_TYPE_INTEGER, offsetof(fr_connection_pool_t, cleanup_delay),
	  0, "5" },
	{ "idle_timeout",  PW_TYPE_INTEGER, offsetof(fr_connection_pool_t, idle_timeout),
	  0, "60" },
	{ "spread", PW_TYPE_BOOLEAN, offsetof(fr_connection_pool_t, spread),
	  0, "no" },
	{ NULL, -1, 0, NULL, NULL }
};

/** Removes a connection from the connection list
 *
 * @note Must be called with the mutex held.
 *
 * @param[in,out] pool to modify.
 * @param[in] this Connection to delete.
 */
static void fr_connection_unlink(fr_connection_pool_t *pool,
				 fr_connection_t *this)
{
	if (this->prev) {
		rad_assert(pool->head != this);
		this->prev->next = this->next;
	} else {
		rad_assert(pool->head == this);
		pool->head = this->next;
	}
	if (this->next) {
		rad_assert(pool->tail != this);
		this->next->prev = this->prev;
	} else {
		rad_assert(pool->tail == this);
		pool->tail = this->prev;
	}

	this->prev = this->next = NULL;
}

/** Adds a connection to the head of the connection list
 *
 * @note Must be called with the mutex held.
 *
 * @param[in,out] pool to modify.
 * @param[in] this Connection to add.
 */
static void fr_connection_link_head(fr_connection_pool_t *pool,
			            fr_connection_t *this)
{
	rad_assert(pool != NULL);
	rad_assert(this != NULL);
	rad_assert(pool->head != this);
	rad_assert(pool->tail != this);

	if (pool->head) {
		pool->head->prev = this;
	}

	this->next = pool->head;
	this->prev = NULL;
	pool->head = this;
	if (!pool->tail) {
		rad_assert(this->next == NULL);
		pool->tail = this;
	} else {
		rad_assert(this->next != NULL);
	}
}

/** Adds a connection to the tail of the connection list
 *
 * @note Must be called with the mutex held.
 *
 * @param[in,out] pool to modify.
 * @param[in] this Connection to add.
 */
static void fr_connection_link_tail(fr_connection_pool_t *pool,
			            fr_connection_t *this)
{
	rad_assert(pool != NULL);
	rad_assert(this != NULL);
	rad_assert(pool->head != this);
	rad_assert(pool->tail != this);

	if (pool->tail) {
		pool->tail->next = this;
	}
	this->prev = pool->tail;
	this->next = NULL;
	pool->tail = this;
	if (!pool->head) {
		rad_assert(this->prev == NULL);
		pool->head = this;
	} else {
		rad_assert(this->prev != NULL);
	}
}


/** Spawns a new connection
 *
 * Spawns a new connection using the create callback, and returns it for
 * adding to the connection list.
 *
 * @note Will call the 'open' trigger.
 * @note Must be called with the mutex free.
 *
 * @param[in] pool to modify.
 * @param[in] now Current time.
 * @return the new connection struct or NULL on error.
 */
static fr_connection_t *fr_connection_spawn(fr_connection_pool_t *pool,
					    time_t now)
{
	fr_connection_t *this;
	void *conn;

	rad_assert(pool != NULL);

	/*
	 *	Prevent all threads from blocking if the resource
	 *	were managing connections for appears to be unavailable.
	 */
	if ((pool->num == 0) && pool->spawning) {
		return NULL;
	}

	pthread_mutex_lock(&pool->mutex);
	rad_assert(pool->num <= pool->max);

	if ((pool->last_failed == now) || pool->spawning) {
		int complain = false;

		if (pool->last_throttled != now) {
			complain = true;

			pool->last_throttled = now;
		}

		pthread_mutex_unlock(&pool->mutex);

		if (complain) {
			if (pool->spawning) {
				ERROR("%s: Cannot open new connection, "
			       	       "connection spawning already in "
			       	       "progress", pool->log_prefix);
			} else {
				ERROR("%s: Last connection failed, "
				       "throttling connection spawn",
				       pool->log_prefix);
			}
		}

		return NULL;
	}

	pool->spawning = true;

	/*
	 *	Unlock the mutex while we try to open a new
	 *	connection.  If there are issues with the back-end,
	 *	opening a new connection may take a LONG time.  In
	 *	that case, we want the other connections to continue
	 *	to be used.
	 */
	pthread_mutex_unlock(&pool->mutex);

	INFO("%s: Opening additional connection (%" PRIu64 ")", pool->log_prefix, pool->count);

	this = rad_malloc(sizeof(*this));
	memset(this, 0, sizeof(*this));

	/*
	 *	This may take a long time, which prevents other
	 *	threads from releasing connections.  We don't care
	 *	about other threads opening new connections, as we
	 *	already have no free connections.
	 */
	conn = pool->create(pool->ctx);
	if (!conn) {
		ERROR("%s: Opening connection failed (%" PRIu64 ")", pool->log_prefix, pool->count);

		pool->last_failed = now;
		free(this);
		pool->spawning = false; /* atomic, so no lock is needed */
		return NULL;
	}

	this->created = now;
	this->connection = conn;

	/*
	 *	And lock the mutex again while we link the new
	 *	connection back into the pool.
	 */
	pthread_mutex_lock(&pool->mutex);

	this->number = pool->count++;
	this->last_used = now;
	fr_connection_link_head(pool, this);
	pool->num++;
	pool->spawning = false;
	pool->last_spawned = time(NULL);

	pthread_mutex_unlock(&pool->mutex);

	if (pool->trigger) exec_trigger(NULL, pool->cs, "open", true);

	return this;
}

/** Add a new connection to the pool
 *
 * If conn is not NULL will attempt to add that connection handle to the pool.
 * If conn is NULL will attempt to spawn a new connection using the create
 * callback.
 *
 * @note Will call the 'open' trigger.
 *
 * @param[in,out] pool to add connection to.
 * @param[in] conn to add.
 * @return 0 if the connection wasn't added else 1.
 */
int fr_connection_add(fr_connection_pool_t *pool, void *conn)
{
	fr_connection_t *this;

	if (!pool) return 0;

	pthread_mutex_lock(&pool->mutex);

	if (!conn) {
		conn = pool->create(pool->ctx);
		if (!conn) {
			pthread_mutex_unlock(&pool->mutex);
			return 0;
		}

		INFO("%s: Opening connection successful (%" PRIu64 ")", pool->log_prefix, pool->count);
	}

	/*
	 *	Too many connections: can't add it.
	 */
	if (pool->num >= pool->max) {
		pthread_mutex_unlock(&pool->mutex);
		return 0;
	}

	this = rad_malloc(sizeof(*this));
	memset(this, 0, sizeof(*this));

	this->created = time(NULL);
	this->connection = conn;

	this->number = pool->count++;
	this->last_used = time(NULL);
	fr_connection_link_head(pool, this);
	pool->num++;

	pthread_mutex_unlock(&pool->mutex);

	if (pool->trigger) exec_trigger(NULL, pool->cs, "open", true);

	return 1;
}

/** Close an existing connection.
 *
 * Removes the connection from the list, calls the delete callback to close
 * the connection, then frees memory allocated to the connection.
 *
 * @note Will call the 'close' trigger.
 * @note Must be called with the mutex held.
 *
 * @param[in,out] pool to modify.
 * @param[in,out] this Connection to delete.

 */
static void fr_connection_close(fr_connection_pool_t *pool,
				fr_connection_t *this)
{
	if (pool->trigger) exec_trigger(NULL, pool->cs, "close", true);

	rad_assert(this->in_use == false);

	fr_connection_unlink(pool, this);
	pool->delete(pool->ctx, this->connection);
	rad_assert(pool->num > 0);
	pool->num--;
	free(this);
}

/** Find a connection handle in the connection list
 *
 * Walks over the list of connections searching for a specified connection
 * handle and returns the first connection that contains that pointer.
 *
 * @note Will lock mutex and only release mutex if connection handle
 * is not found, so will usually return will mutex held.
 * @note Must be called with the mutex free.
 *
 * @param[in] pool to search in.
 * @param[in] conn handle to search for.
 * @return the connection containing the specified handle, or NULL if non is
 * found.
 */
static fr_connection_t *fr_connection_find(fr_connection_pool_t *pool, void *conn)
{
	fr_connection_t *this;

	if (!pool || !conn) return NULL;

	pthread_mutex_lock(&pool->mutex);

	/*
	 *	FIXME: This loop could be avoided if we passed a 'void
	 *	**connection' instead.  We could use "offsetof" in
	 *	order to find top of the parent structure.
	 */
	for (this = pool->head; this != NULL; this = this->next) {
		if (this->connection == conn) return this;
	}

	pthread_mutex_unlock(&pool->mutex);
	return NULL;
}

/** Delete a connection from the connection pool.
 *
 * Resolves the connection handle to a connection, then (if found)
 * closes, unlinks and frees that connection.
 *
 * @note Must be called with the mutex free.
 *
 * @param[in,out] pool Connection pool to modify.
 * @param[in] conn to delete.
 * @return 0 if the connection could not be found, else 1.
 */
int fr_connection_del(fr_connection_pool_t *pool, void *conn)
{
	fr_connection_t *this;

	this = fr_connection_find(pool, conn);
	if (!this) return 0;

	/*
	 *	If it's in use, release it.
	 */
	if (this->in_use) {
		rad_assert(this->in_use == true);
		this->in_use = false;

		rad_assert(pool->active > 0);
		pool->active--;
	}

	INFO("%s: Deleting connection (%" PRIu64 ")", pool->log_prefix, this->number);

	fr_connection_close(pool, this);
	fr_connection_pool_check(pool);
	return 1;
}

/** Delete a connection pool
 *
 * Closes, unlinks and frees all connections in the connection pool, then frees
 * all memory used by the connection pool.
 *
 * @note Will call the 'stop' trigger.
 * @note Must be called with the mutex free.
 *
 * @param[in,out] pool to delete.
 */
void fr_connection_pool_delete(fr_connection_pool_t *pool)
{
	fr_connection_t *this, *next;

	if (!pool) return;

	DEBUG("%s: Removing connection pool", pool->log_prefix);

	pthread_mutex_lock(&pool->mutex);

	for (this = pool->head; this != NULL; this = next) {
		next = this->next;

		INFO("%s: Closing connection (%" PRIu64 ")", pool->log_prefix, this->number);

		fr_connection_close(pool, this);
	}

	if (pool->trigger) exec_trigger(NULL, pool->cs, "stop", true);

	rad_assert(pool->head == NULL);
	rad_assert(pool->tail == NULL);
	rad_assert(pool->num == 0);

	free(pool->log_prefix);
	free(pool);
}

/** Create a new connection pool
 *
 * Allocates structures used by the connection pool, initialises the various
 * configuration options and counters, and sets the callback functions.
 *
 * Will also spawn the number of connections specified by the 'start'
 * configuration options.
 *
 * @note Will call the 'start' trigger.
 *
 * @param[in] parent configuration section containing a 'pool' subsection.
 * @param[in] ctx pointer to pass to callbacks.
 * @param[in] c Callback to create new connections.
 * @param[in] a Callback to check the status of connections.
 * @param[in] d Callback to delete connections.
 * @param[in] prefix to prepend to all log message, if NULL will create prefix
 *	from parent conf section names.
 * @return A new connection pool or NULL on error.
 */
fr_connection_pool_t *fr_connection_pool_init(CONF_SECTION *parent,
					      void *ctx,
					      fr_connection_create_t c,
					      fr_connection_alive_t a,
					      fr_connection_delete_t d,
					      char *prefix)
{
	int i, lp_len;
	fr_connection_pool_t *pool;
	fr_connection_t *this;
	CONF_SECTION *modules;
	CONF_SECTION *cs;
	char const *cs_name1, *cs_name2;
	time_t now = time(NULL);

	if (!parent || !ctx || !c || !d) return NULL;

	cs = cf_section_sub_find(parent, "pool");
	if (!cs) cs = cf_section_sub_find(parent, "limit");

	pool = rad_malloc(sizeof(*pool));
	memset(pool, 0, sizeof(*pool));

	pool->cs = cs;
	pool->ctx = ctx;
	pool->create = c;
	pool->alive = a;
	pool->delete = d;

	pool->head = pool->tail = NULL;

#ifdef HAVE_PTHREAD_H
	pthread_mutex_init(&pool->mutex, NULL);
#endif

	if (!prefix) {
		modules = cf_item_parent(cf_sectiontoitem(parent));
		if (modules) {
			cs_name1 = cf_section_name1(modules);
			if (cs_name1 && (strcmp(cs_name1, "modules") == 0)) {
				cs_name1 = cf_section_name1(parent);
				cs_name2 = cf_section_name2(parent);
				if (!cs_name2) {
					cs_name2 = cs_name1;
				}

				lp_len = (sizeof(LOG_PREFIX) - 4) + strlen(cs_name1) + strlen(cs_name2);
				pool->log_prefix = rad_malloc(lp_len);
				snprintf(pool->log_prefix, lp_len, LOG_PREFIX, cs_name1,
					 cs_name2);
			}
		} else {		/* not a module configuration */
			cs_name1 = cf_section_name1(parent);

			pool->log_prefix = strdup(cs_name1);
		}
	} else {
		pool->log_prefix = strdup(prefix);
	}

	DEBUG("%s: Initialising connection pool", pool->log_prefix);

	if (cs) {
		if (cf_section_parse(cs, pool, connection_config) < 0) {
			goto error;
		}

		if (cf_section_sub_find(cs, "trigger")) pool->trigger = true;
	} else {
		pool->start = 5;
		pool->min = 5;
		pool->max = 10;
		pool->spare = 3;
		pool->cleanup_delay = 5;
		pool->idle_timeout = 60;
	}

	/*
	 *	Some simple limits
	 */
	if (pool->max > 1024) pool->max = 1024;
	if (pool->start > pool->max) pool->start = pool->max;
	if (pool->spare > (pool->max - pool->min)) {
		pool->spare = pool->max - pool->min;
	}
	if ((pool->lifetime > 0) && (pool->idle_timeout > pool->lifetime)) {
		pool->idle_timeout = 0;
	}

	/*
	 *	Create all of the connections, unless the admin says
	 *	not to.
	 */
	for (i = 0; i < pool->start; i++) {
		this = fr_connection_spawn(pool, now);
		if (!this) {
		error:
			fr_connection_pool_delete(pool);
			return NULL;
		}
	}

	if (pool->trigger) exec_trigger(NULL, pool->cs, "start", true);

	return pool;
}


/** Check whether a connection needs to be removed from the pool
 *
 * Will verify that the connection is within idle_timeout, max_uses, and
 * lifetime values. If it is not, the connection will be closed.
 *
 * @note Will only close connections not in use.
 * @note Must be called with the mutex held.
 *
 * @param[in,out] pool to modify.
 * @param[in,out] this Connection to manage.
 * @param[in] now Current time.
 * @return 0 if the connection was closed, otherwise 1.
 */
static int fr_connection_manage(fr_connection_pool_t *pool,
				fr_connection_t *this,
				time_t now)
{
	rad_assert(pool != NULL);
	rad_assert(this != NULL);

	/*
	 *	Don't terminated in-use connections
	 */
	if (this->in_use) return 1;

	if ((pool->max_uses > 0) &&
	    (this->num_uses >= pool->max_uses)) {
		DEBUG("%s: Closing expired connection (%" PRIu64 "): Hit max_uses limit", pool->log_prefix,
		      this->number);
	do_delete:
		if ((pool->num <= pool->min) &&
		    (pool->last_complained < now)) {
			WARN("%s: You probably need to lower \"min\"", pool->log_prefix);

			pool->last_complained = now;
		}
		fr_connection_close(pool, this);
		return 0;
	}

	if ((pool->lifetime > 0) &&
	    ((this->created + pool->lifetime) < now)) {
		DEBUG("%s: Closing expired connection (%" PRIu64 ")", pool->log_prefix, this->number);
		goto do_delete;
	}

	if ((pool->idle_timeout > 0) &&
	    ((this->last_used + pool->idle_timeout) < now)) {
		INFO("%s: Closing connection (%" PRIu64 "): Hit idle_timeout, was idle for %u seconds",
		     pool->log_prefix, this->number, (int) (now - this->last_used));
		goto do_delete;
	}

	return 1;
}


/** Check whether any connections needs to be removed from the pool
 *
 * Maintains the number of connections in the pool as per the configuration
 * parameters for the connection pool.
 *
 * @note Will only run checks the first time it's called in a given second,
 * to throttle connection spawning/closing.
 * @note Will only close connections not in use.
 * @note Must be called with the mutex held, will release mutex before
 * returning.
 *
 * @param[in,out] pool to manage.
 * @return 1
 */
static int fr_connection_pool_check(fr_connection_pool_t *pool)
{
	int spare, spawn;
	time_t now = time(NULL);
	fr_connection_t *this, *next;

	if (pool->last_checked == now) {
		pthread_mutex_unlock(&pool->mutex);
		return 1;
	}

	spare = pool->num - pool->active;

	spawn = 0;
	if ((pool->num < pool->max) && (spare < pool->spare)) {
		spawn = pool->spare - spare;
		if ((spawn + pool->num) > pool->max) {
			spawn = pool->max - pool->num;
		}
		if (pool->spawning) spawn = 0;

		if (spawn) {
			pthread_mutex_unlock(&pool->mutex);
			fr_connection_spawn(pool, now); /* ignore return code */
			pthread_mutex_lock(&pool->mutex);
		}
	}

	/*
	 *	We haven't spawned connections in a while, and there
	 *	are too many spare ones.  Close the one which has been
	 *	idle for the longest.
	 */
	if ((now >= (pool->last_spawned + pool->cleanup_delay)) &&
	    (spare > pool->spare)) {
		fr_connection_t *idle;

		idle = NULL;
		for (this = pool->tail; this != NULL; this = this->prev) {
			if (this->in_use) continue;

			if (!idle ||
			   (this->last_used < idle->last_used)) {
				idle = this;
			}
		}

		rad_assert(idle != NULL);

		INFO("%s: Closing connection (%" PRIu64 "): Too many free connections (%d > %d)", pool->log_prefix,
		     idle->number, spare, pool->spare);
		fr_connection_close(pool, idle);
	}

	/*
	 *	Pass over all of the connections in the pool, limiting
	 *	lifetime, idle time, max requests, etc.
	 */
	for (this = pool->head; this != NULL; this = next) {
		next = this->next;
		fr_connection_manage(pool, this, now);
	}

	pool->last_checked = now;
	pthread_mutex_unlock(&pool->mutex);

	return 1;
}

/** Trigger connection check for a given connection or all connections
 *
 * If conn is not NULL then we call fr_connection_manage on the connection.
 * If conn is NULL we call fr_connection_pool_check on the pool.
 *
 * @note Only connections that are not in use will be closed.
 *
 * @see fr_connection_manage
 * @see fr_connection_pool_check
 * @param[in,out] pool to manage.
 * @param[in,out] conn to check.
 * @return 0 if the connection was closed, else 1.
 */
int fr_connection_check(fr_connection_pool_t *pool, void *conn)
{
	fr_connection_t *this;
	time_t now;
	int ret = 1;

	if (!pool) return 1;

	now = time(NULL);
	pthread_mutex_lock(&pool->mutex);

	if (!conn) return fr_connection_pool_check(pool);

	for (this = pool->head; this != NULL; this = this->next) {
		if (this->connection == conn) {
			ret = fr_connection_manage(pool, conn, now);
			break;
		}
	}

	pthread_mutex_unlock(&pool->mutex);

	return ret;
}

/** Reserve a connection in the connection pool
 *
 * Will attempt to find an unused connection in the connection pool, if one is
 * found, will mark it as in in use increment the number of active connections
 * and return the connection handle.
 *
 * If no free connections are found will attempt to spawn a new one, conditional
 * on a connection spawning not already being in progress, and not being at the
 * 'max' connection limit.
 *
 * @note fr_connection_release must be called once the caller has finished
 * using the connection.
 *
 * @see fr_connection_release
 * @param[in,out] pool to reserve the connection from.
 * @return a pointer to the connection handle, or NULL on error.
 */
void *fr_connection_get(fr_connection_pool_t *pool)
{
	time_t now;
	fr_connection_t *this, *next;

	if (!pool) return NULL;

	pthread_mutex_lock(&pool->mutex);

	now = time(NULL);
	for (this = pool->head; this != NULL; this = next) {
		next = this->next;

		if (!this->in_use) goto do_return;
	}

	if (pool->num == pool->max) {
		int complain = false;

		/*
		 *	Rate-limit complaints.
		 */
		if (pool->last_at_max != now) {
			complain = true;
			pool->last_at_max = now;
		}

		pthread_mutex_unlock(&pool->mutex);

		if (complain) {
			ERROR("%s: No connections available and at max connection limit", pool->log_prefix);
		}

		return NULL;
	}

	pthread_mutex_unlock(&pool->mutex);
	this = fr_connection_spawn(pool, now);
	if (!this) return NULL;
	pthread_mutex_lock(&pool->mutex);

do_return:
	pool->active++;
	this->num_uses++;
	this->last_used = now;
	this->in_use = true;

	pthread_mutex_unlock(&pool->mutex);

	DEBUG("%s: Reserved connection (%" PRIu64 ")", pool->log_prefix, this->number);

	return this->connection;
}

/** Release a connection
 *
 * Will mark a connection as unused and decrement the number of active
 * connections.
 *
 * @see fr_connection_get
 * @param[in,out] pool to release the connection in.
 * @param[in,out] conn to release.
 */
void fr_connection_release(fr_connection_pool_t *pool, void *conn)
{
	fr_connection_t *this;

	this = fr_connection_find(pool, conn);
	if (!this) return;

	rad_assert(this->in_use == true);
	this->in_use = false;

	/*
	 *	Determines whether the last used connection gets
	 *	re-used first.
	 */
	if (pool->spread) {
		/*
		 *	Put it at the tail of the list, so
		 *	that it will get re-used last.
		 */
		if (this != pool->tail) {
			fr_connection_unlink(pool, this);
			fr_connection_link_tail(pool, this);
		}
	} else {
		/*
		 *	Put it at the head of the list, so
		 *	that it will get re-used quickly.
		 */
		if (this != pool->head) {
			fr_connection_unlink(pool, this);
			fr_connection_link_head(pool, this);
		}
	}

	rad_assert(pool->active > 0);
	pool->active--;

	DEBUG("%s: Released connection (%" PRIu64 ")", pool->log_prefix, this->number);

	/*
	 *	We mirror the "spawn on get" functionality by having
	 *	"delete on release".  If there are too many spare
	 *	connections, go manage the pool && clean some up.
	 */
	fr_connection_pool_check(pool);
}

/** Reconnect a suspected inviable connection
 *
 * This should be called by the module if it suspects that a connection is
 * not viable (e.g. the server has closed it).
 *
 * Will attempt to create a new connection handle using the create callback,
 * and if this is successful the new handle will be assigned to the existing
 * pool connection.
 *
 * If this is not successful, the connection will be removed from the pool.
 *
 * When implementing a module that uses the connection pool API, it is advisable
 * to pass a pointer to the pointer to the handle (void **conn)
 * to all functions which may call reconnect. This is so that if a new handle
 * is created and returned, the handle pointer can be updated up the callstack,
 * and a function higher up the stack doesn't attempt to use a now invalid
 * connection handle.
 *
 * @warning After calling reconnect the caller *MUST NOT* attempt to use
 * the old handle in any other operations, as its memory will have been freed.
 *
 * @see fr_connection_get
 * @param[in,out] pool to reconnect the connection in.
 * @param[in,out] conn to reconnect.
 * @return ew connection handle if successful else NULL.
 */
void *fr_connection_reconnect(fr_connection_pool_t *pool, void *conn)
{
	void *new_conn;
	fr_connection_t *this;
	uint64_t conn_number;

	if (!pool || !conn) return NULL;

	this = fr_connection_find(pool, conn);
	if (!this) return NULL;

	conn_number = this->number;

	rad_assert(this->in_use == true);

	DEBUG("%s: Reconnecting (%" PRIu64 ")", pool->log_prefix, conn_number);

	new_conn = pool->create(pool->ctx);
	if (!new_conn) {
		time_t now = time(NULL);

		if (pool->last_complained == now) {
			now = 0;
		} else {
			pool->last_complained = now;
		}

		this->in_use = false;

		rad_assert(pool->active > 0);
		pool->active--;

		fr_connection_close(pool, this);
		pthread_mutex_unlock(&pool->mutex);

		/*
		 *	Can't create a new socket.
		 *	Try grabbing a pre-existing one.
		 */
		new_conn = fr_connection_get(pool);
		if (new_conn) return new_conn;

		if (!now) return NULL;

		ERROR("%s: Failed to reconnect (%" PRIu64 "), and no other connections available", pool->log_prefix,
		      conn_number);

		return NULL;
	}

	pool->delete(pool->ctx, conn);
	this->connection = new_conn;
	pthread_mutex_unlock(&pool->mutex);
	return new_conn;
}
