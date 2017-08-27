/*
 * Copyright (C) 2011-2014 Tobias Brunner
 * Copyright (C) 2006 Martin Willi
 * Hochschule fuer Technik Rapperswil
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.  See <http://www.fsf.org/copyleft/gpl.txt>.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 */

#include "bus.h"

#include <stdint.h>

#include <threading/thread.h>
#include <threading/thread_value.h>
#include <threading/mutex.h>
#include <threading/rwlock.h>

/**
 * These operations allow us to speed up the log level checks on some platforms.
 * In particular if acquiring the read lock is expensive even in the absence of
 * any writers.
 *
 * Note that while holding the read/write lock the read does not have to be
 * atomic as the write lock must be held to set the level.
 */
#ifdef HAVE_GCC_ATOMIC_OPERATIONS

#define skip_level(ptr, level) (__atomic_load_n(ptr, __ATOMIC_RELAXED) < level)
#define set_level(ptr, val) __atomic_store_n(ptr, val, __ATOMIC_RELAXED)

#elif defined(HAVE_GCC_SYNC_OPERATIONS)

#define skip_level(ptr, level) (__sync_fetch_and_add(ptr, 0) < level)
#define set_level(ptr, val) __sync_bool_compare_and_swap(ptr, *ptr, val)

#else

#define skip_level(ptr, level) FALSE
#define set_level(ptr, val) ({ *ptr = val; })

#endif

typedef struct private_bus_t private_bus_t;

/**
 * Private data of a bus_t object.
 */
struct private_bus_t {
	/**
	 * Public part of a bus_t object.
	 */
	bus_t public;

	/**
	 * List of registered listeners as entry_t.
	 */
	linked_list_t *listeners;

	/**
	 * List of registered loggers for each log group as log_entry_t.
	 * Loggers are ordered by descending log level.
	 * The extra list stores all loggers so we can properly unregister them.
	 */
	linked_list_t *loggers[DBG_MAX + 1];

	/**
	 * Maximum log level of any registered logger for each log group.
	 * This allows to check quickly if a log message has to be logged at all.
	 */
	level_t max_level[DBG_MAX + 1];

	/**
	 * Same as max level, but for loggers using the vlog() method.
	 */
	level_t max_vlevel[DBG_MAX + 1];

	/**
	 * Mutex for the list of listeners, recursively.
	 */
	mutex_t *mutex;

	/**
	 * Read-write lock for the list of loggers.
	 */
	rwlock_t *log_lock;

	/**
	 * Thread local storage the threads IKE_SA
	 */
	thread_value_t *thread_sa;
};

typedef struct entry_t entry_t;

/**
 * a listener entry
 */
struct entry_t {

	/**
	 * registered listener interface
	 */
	listener_t *listener;

	/**
	 * are we currently calling this listener
	 */
	int calling;

};

typedef struct log_entry_t log_entry_t;

/**
 * a logger entry
 */
struct log_entry_t {

	/**
	 * registered logger interface
	 */
	logger_t *logger;

	/**
	 * registered log levels per group
	 */
	level_t levels[DBG_MAX];

};

METHOD(bus_t, add_listener, void,
	private_bus_t *this, listener_t *listener)
{
	entry_t *entry;

	INIT(entry,
		.listener = listener,
	);

	this->mutex->lock(this->mutex);
	this->listeners->insert_last(this->listeners, entry);
	this->mutex->unlock(this->mutex);
}

METHOD(bus_t, remove_listener, void,
	private_bus_t *this, listener_t *listener)
{
	enumerator_t *enumerator;
	entry_t *entry;

	this->mutex->lock(this->mutex);
	enumerator = this->listeners->create_enumerator(this->listeners);
	while (enumerator->enumerate(enumerator, &entry))
	{
		if (entry->listener == listener)
		{
			this->listeners->remove_at(this->listeners, enumerator);
			free(entry);
			break;
		}
	}
	enumerator->destroy(enumerator);
	this->mutex->unlock(this->mutex);
}

/**
 * Register a logger on the given log group according to the requested level
 */
static inline void register_logger(private_bus_t *this, debug_t group,
								   log_entry_t *entry)
{
	enumerator_t *enumerator;
	linked_list_t *loggers;
	log_entry_t *current;
	level_t level;

	loggers = this->loggers[group];
	level = entry->levels[group];

	enumerator = loggers->create_enumerator(loggers);
	while (enumerator->enumerate(enumerator, (void**)&current))
	{
		if (current->levels[group] <= level)
		{
			break;
		}
	}
	loggers->insert_before(loggers, enumerator, entry);
	enumerator->destroy(enumerator);

	if (entry->logger->log)
	{
		set_level(&this->max_level[group], max(this->max_level[group], level));
	}
	if (entry->logger->vlog)
	{
		set_level(&this->max_vlevel[group],
				  max(this->max_vlevel[group], level));
	}
}

/**
 * Unregister a logger from all log groups (destroys the log_entry_t)
 */
static inline void unregister_logger(private_bus_t *this, logger_t *logger)
{
	enumerator_t *enumerator;
	linked_list_t *loggers;
	log_entry_t *entry, *found = NULL;

	loggers = this->loggers[DBG_MAX];
	enumerator = loggers->create_enumerator(loggers);
	while (enumerator->enumerate(enumerator, &entry))
	{
		if (entry->logger == logger)
		{
			loggers->remove_at(loggers, enumerator);
			found = entry;
			break;
		}
	}
	enumerator->destroy(enumerator);

	if (found)
	{
		level_t level = LEVEL_SILENT, vlevel = LEVEL_SILENT;
		debug_t group;

		for (group = 0; group < DBG_MAX; group++)
		{
			if (found->levels[group] > LEVEL_SILENT)
			{
				loggers = this->loggers[group];
				loggers->remove(loggers, found, NULL);

				if (loggers->get_first(loggers, (void**)&entry) == SUCCESS)
				{
					if (entry->logger->log)
					{
						level = entry->levels[group];
					}
					if (entry->logger->vlog)
					{
						vlevel = entry->levels[group];
					}
				}
				set_level(&this->max_level[group], level);
				set_level(&this->max_vlevel[group], vlevel);
			}
		}
		free(found);
	}
}

METHOD(bus_t, add_logger, void,
	private_bus_t *this, logger_t *logger)
{
	log_entry_t *entry;
	debug_t group;

	INIT(entry,
		.logger = logger,
	);

	this->log_lock->write_lock(this->log_lock);
	unregister_logger(this, logger);
	for (group = 0; group < DBG_MAX; group++)
	{
		entry->levels[group] = logger->get_level(logger, group);
		if (entry->levels[group] > LEVEL_SILENT)
		{
			register_logger(this, group, entry);
		}
	}
	this->loggers[DBG_MAX]->insert_last(this->loggers[DBG_MAX], entry);
	this->log_lock->unlock(this->log_lock);
}

METHOD(bus_t, remove_logger, void,
	private_bus_t *this, logger_t *logger)
{
	this->log_lock->write_lock(this->log_lock);
	unregister_logger(this, logger);
	this->log_lock->unlock(this->log_lock);
}

METHOD(bus_t, set_sa, void,
	private_bus_t *this, ike_sa_t *ike_sa)
{
	this->thread_sa->set(this->thread_sa, ike_sa);
}

METHOD(bus_t, get_sa, ike_sa_t*,
	private_bus_t *this)
{
	return this->thread_sa->get(this->thread_sa);
}

/**
 * data associated to a signal, passed to callback
 */
typedef struct {
	/** associated IKE_SA */
	ike_sa_t *ike_sa;
	/** invoking thread */
	long thread;
	/** debug group */
	debug_t group;
	/** debug level */
	level_t level;
	/** message/fmt */
	char *message;
	/** argument list if message is a format string for vlog() */
	va_list args;
} log_data_t;

/**
 * logger->log() invocation as a invoke_function callback
 */
static void log_cb(log_entry_t *entry, log_data_t *data)
{
	if (entry->logger->log && entry->levels[data->group] >= data->level)
	{
		entry->logger->log(entry->logger, data->group, data->level,
						   data->thread, data->ike_sa, data->message);
	}
}

/**
 * logger->vlog() invocation as a invoke_function callback
 */
static void vlog_cb(log_entry_t *entry, log_data_t *data)
{
	if (entry->logger->vlog && entry->levels[data->group] >= data->level)
	{
		va_list copy;

		va_copy(copy, data->args);
		entry->logger->vlog(entry->logger, data->group, data->level,
							data->thread, data->ike_sa, data->message, copy);
		va_end(copy);
	}
}

METHOD(bus_t, vlog, void,
	private_bus_t *this, debug_t group, level_t level,
	char* format, va_list args)
{
	linked_list_t *loggers;
	log_data_t data;

	/* NOTE: This is not 100% thread-safe and done here only because it is
	 * performance critical.  We therefore ignore the following two issues for
	 * this particular case:  1) We might miss some log messages if another
	 * thread concurrently increases the log level or registers a new logger.
	 * 2) We might have to acquire the read lock below even if it wouldn't be
	 * necessary anymore due to another thread concurrently unregistering a
	 * logger or reducing the level. */
	if (skip_level(&this->max_level[group], level) &&
		skip_level(&this->max_vlevel[group], level))
	{
		return;
	}

	this->log_lock->read_lock(this->log_lock);
	loggers = this->loggers[group];

	if (this->max_level[group] >= level)
	{
		char buf[1024];
		ssize_t len;

		data.ike_sa = this->thread_sa->get(this->thread_sa);
		data.thread = thread_current_id();
		data.group = group;
		data.level = level;
		data.message = buf;

		va_copy(data.args, args);
		len = vsnprintf(data.message, sizeof(buf), format, data.args);
		va_end(data.args);
		if (len >= sizeof(buf))
		{
			len++;
			data.message = malloc(len);
			va_copy(data.args, args);
			len = vsnprintf(data.message, len, format, data.args);
			va_end(data.args);
		}
		if (len > 0)
		{
			loggers->invoke_function(loggers, (linked_list_invoke_t)log_cb,
									 &data);
		}
		if (data.message != buf)
		{
			free(data.message);
		}
	}
	if (this->max_vlevel[group] >= level)
	{
		data.ike_sa = this->thread_sa->get(this->thread_sa);
		data.thread = thread_current_id();
		data.group = group;
		data.level = level;
		data.message = format;

		va_copy(data.args, args);
		loggers->invoke_function(loggers, (linked_list_invoke_t)vlog_cb, &data);
		va_end(data.args);
	}

	this->log_lock->unlock(this->log_lock);
}

METHOD(bus_t, log_, void,
	private_bus_t *this, debug_t group, level_t level, char* format, ...)
{
	va_list args;

	va_start(args, format);
	vlog(this, group, level, format, args);
	va_end(args);
}

/**
 * unregister a listener
 */
static inline void unregister_listener(private_bus_t *this, entry_t *entry,
									   enumerator_t *enumerator)
{
	this->listeners->remove_at(this->listeners, enumerator);
	free(entry);
}

METHOD(bus_t, alert, void,
	private_bus_t *this, alert_t alert, ...)
{
	enumerator_t *enumerator;
	ike_sa_t *ike_sa;
	entry_t *entry;
	va_list args;
	bool keep;

	ike_sa = this->thread_sa->get(this->thread_sa);

	this->mutex->lock(this->mutex);
	enumerator = this->listeners->create_enumerator(this->listeners);
	while (enumerator->enumerate(enumerator, &entry))
	{
		if (entry->calling || !entry->listener->alert)
		{
			continue;
		}
		entry->calling++;
		va_start(args, alert);
		keep = entry->listener->alert(entry->listener, ike_sa, alert, args);
		va_end(args);
		entry->calling--;
		if (!keep)
		{
			unregister_listener(this, entry, enumerator);
		}
	}
	enumerator->destroy(enumerator);
	this->mutex->unlock(this->mutex);
}

METHOD(bus_t, ike_state_change, void,
	private_bus_t *this, ike_sa_t *ike_sa, ike_sa_state_t state)
{
	enumerator_t *enumerator;
	entry_t *entry;
	bool keep;

	this->mutex->lock(this->mutex);
	enumerator = this->listeners->create_enumerator(this->listeners);
	while (enumerator->enumerate(enumerator, &entry))
	{
		if (entry->calling || !entry->listener->ike_state_change)
		{
			continue;
		}
		entry->calling++;
		keep = entry->listener->ike_state_change(entry->listener, ike_sa, state);
		entry->calling--;
		if (!keep)
		{
			unregister_listener(this, entry, enumerator);
		}
	}
	enumerator->destroy(enumerator);
	this->mutex->unlock(this->mutex);
}

METHOD(bus_t, child_state_change, void,
	private_bus_t *this, child_sa_t *child_sa, child_sa_state_t state)
{
	enumerator_t *enumerator;
	ike_sa_t *ike_sa;
	entry_t *entry;
	bool keep;

	ike_sa = this->thread_sa->get(this->thread_sa);

	this->mutex->lock(this->mutex);
	enumerator = this->listeners->create_enumerator(this->listeners);
	while (enumerator->enumerate(enumerator, &entry))
	{
		if (entry->calling || !entry->listener->child_state_change)
		{
			continue;
		}
		entry->calling++;
		keep = entry->listener->child_state_change(entry->listener, ike_sa,
												   child_sa, state);
		entry->calling--;
		if (!keep)
		{
			unregister_listener(this, entry, enumerator);
		}
	}
	enumerator->destroy(enumerator);
	this->mutex->unlock(this->mutex);
}

METHOD(bus_t, message, void,
	private_bus_t *this, message_t *message, bool incoming, bool plain)
{
	enumerator_t *enumerator;
	ike_sa_t *ike_sa;
	entry_t *entry;
	bool keep;

	ike_sa = this->thread_sa->get(this->thread_sa);

	this->mutex->lock(this->mutex);
	enumerator = this->listeners->create_enumerator(this->listeners);
	while (enumerator->enumerate(enumerator, &entry))
	{
		if (entry->calling || !entry->listener->message)
		{
			continue;
		}
		entry->calling++;
		keep = entry->listener->message(entry->listener, ike_sa,
										message, incoming, plain);
		entry->calling--;
		if (!keep)
		{
			unregister_listener(this, entry, enumerator);
		}
	}
	enumerator->destroy(enumerator);
	this->mutex->unlock(this->mutex);
}

METHOD(bus_t, ike_keys, void,
	private_bus_t *this, ike_sa_t *ike_sa, diffie_hellman_t *dh,
	chunk_t dh_other, chunk_t nonce_i, chunk_t nonce_r,
	ike_sa_t *rekey, shared_key_t *shared)
{
	enumerator_t *enumerator;
	entry_t *entry;
	bool keep;

	this->mutex->lock(this->mutex);
	enumerator = this->listeners->create_enumerator(this->listeners);
	while (enumerator->enumerate(enumerator, &entry))
	{
		if (entry->calling || !entry->listener->ike_keys)
		{
			continue;
		}
		entry->calling++;
		keep = entry->listener->ike_keys(entry->listener, ike_sa, dh, dh_other,
										 nonce_i, nonce_r, rekey, shared);
		entry->calling--;
		if (!keep)
		{
			unregister_listener(this, entry, enumerator);
		}
	}
	enumerator->destroy(enumerator);
	this->mutex->unlock(this->mutex);
}

METHOD(bus_t, child_keys, void,
	private_bus_t *this, child_sa_t *child_sa, bool initiator,
	diffie_hellman_t *dh, chunk_t nonce_i, chunk_t nonce_r)
{
	enumerator_t *enumerator;
	ike_sa_t *ike_sa;
	entry_t *entry;
	bool keep;

	ike_sa = this->thread_sa->get(this->thread_sa);

	this->mutex->lock(this->mutex);
	enumerator = this->listeners->create_enumerator(this->listeners);
	while (enumerator->enumerate(enumerator, &entry))
	{
		if (entry->calling || !entry->listener->child_keys)
		{
			continue;
		}
		entry->calling++;
		keep = entry->listener->child_keys(entry->listener, ike_sa,
								child_sa, initiator, dh, nonce_i, nonce_r);
		entry->calling--;
		if (!keep)
		{
			unregister_listener(this, entry, enumerator);
		}
	}
	enumerator->destroy(enumerator);
	this->mutex->unlock(this->mutex);
}

METHOD(bus_t, child_updown, void,
	private_bus_t *this, child_sa_t *child_sa, bool up)
{
	enumerator_t *enumerator;
	ike_sa_t *ike_sa;
	entry_t *entry;
	bool keep;

	ike_sa = this->thread_sa->get(this->thread_sa);

	this->mutex->lock(this->mutex);
	enumerator = this->listeners->create_enumerator(this->listeners);
	while (enumerator->enumerate(enumerator, &entry))
	{
		if (entry->calling || !entry->listener->child_updown)
		{
			continue;
		}
		entry->calling++;
		keep = entry->listener->child_updown(entry->listener,
											 ike_sa, child_sa, up);
		entry->calling--;
		if (!keep)
		{
			unregister_listener(this, entry, enumerator);
		}
	}
	enumerator->destroy(enumerator);
	this->mutex->unlock(this->mutex);
}

METHOD(bus_t, child_rekey, void,
	private_bus_t *this, child_sa_t *old, child_sa_t *new)
{
	enumerator_t *enumerator;
	ike_sa_t *ike_sa;
	entry_t *entry;
	bool keep;

	ike_sa = this->thread_sa->get(this->thread_sa);

	this->mutex->lock(this->mutex);
	enumerator = this->listeners->create_enumerator(this->listeners);
	while (enumerator->enumerate(enumerator, &entry))
	{
		if (entry->calling || !entry->listener->child_rekey)
		{
			continue;
		}
		entry->calling++;
		keep = entry->listener->child_rekey(entry->listener, ike_sa,
											old, new);
		entry->calling--;
		if (!keep)
		{
			unregister_listener(this, entry, enumerator);
		}
	}
	enumerator->destroy(enumerator);
	this->mutex->unlock(this->mutex);
}

METHOD(bus_t, ike_updown, void,
	private_bus_t *this, ike_sa_t *ike_sa, bool up)
{
	enumerator_t *enumerator;
	entry_t *entry;
	bool keep;

	this->mutex->lock(this->mutex);
	enumerator = this->listeners->create_enumerator(this->listeners);
	while (enumerator->enumerate(enumerator, &entry))
	{
		if (entry->calling || !entry->listener->ike_updown)
		{
			continue;
		}
		entry->calling++;
		keep = entry->listener->ike_updown(entry->listener, ike_sa, up);
		entry->calling--;
		if (!keep)
		{
			unregister_listener(this, entry, enumerator);
		}
	}
	enumerator->destroy(enumerator);
	this->mutex->unlock(this->mutex);

	/* a down event for IKE_SA implicitly downs all CHILD_SAs */
	if (!up)
	{
		enumerator_t *enumerator;
		child_sa_t *child_sa;

		enumerator = ike_sa->create_child_sa_enumerator(ike_sa);
		while (enumerator->enumerate(enumerator, (void**)&child_sa))
		{
			child_updown(this, child_sa, FALSE);
		}
		enumerator->destroy(enumerator);
	}
}

METHOD(bus_t, ike_rekey, void,
	private_bus_t *this, ike_sa_t *old, ike_sa_t *new)
{
	enumerator_t *enumerator;
	entry_t *entry;
	bool keep;

	this->mutex->lock(this->mutex);
	enumerator = this->listeners->create_enumerator(this->listeners);
	while (enumerator->enumerate(enumerator, &entry))
	{
		if (entry->calling || !entry->listener->ike_rekey)
		{
			continue;
		}
		entry->calling++;
		keep = entry->listener->ike_rekey(entry->listener, old, new);
		entry->calling--;
		if (!keep)
		{
			unregister_listener(this, entry, enumerator);
		}
	}
	enumerator->destroy(enumerator);
	this->mutex->unlock(this->mutex);
}

METHOD(bus_t, ike_reestablish_pre, void,
	private_bus_t *this, ike_sa_t *old, ike_sa_t *new)
{
	enumerator_t *enumerator;
	entry_t *entry;
	bool keep;

	this->mutex->lock(this->mutex);
	enumerator = this->listeners->create_enumerator(this->listeners);
	while (enumerator->enumerate(enumerator, &entry))
	{
		if (entry->calling || !entry->listener->ike_reestablish_pre)
		{
			continue;
		}
		entry->calling++;
		keep = entry->listener->ike_reestablish_pre(entry->listener, old, new);
		entry->calling--;
		if (!keep)
		{
			unregister_listener(this, entry, enumerator);
		}
	}
	enumerator->destroy(enumerator);
	this->mutex->unlock(this->mutex);
}

METHOD(bus_t, ike_reestablish_post, void,
	private_bus_t *this, ike_sa_t *old, ike_sa_t *new, bool initiated)
{
	enumerator_t *enumerator;
	entry_t *entry;
	bool keep;

	this->mutex->lock(this->mutex);
	enumerator = this->listeners->create_enumerator(this->listeners);
	while (enumerator->enumerate(enumerator, &entry))
	{
		if (entry->calling || !entry->listener->ike_reestablish_post)
		{
			continue;
		}
		entry->calling++;
		keep = entry->listener->ike_reestablish_post(entry->listener, old, new,
													 initiated);
		entry->calling--;
		if (!keep)
		{
			unregister_listener(this, entry, enumerator);
		}
	}
	enumerator->destroy(enumerator);
	this->mutex->unlock(this->mutex);
}

METHOD(bus_t, authorize, bool,
	private_bus_t *this, bool final)
{
	enumerator_t *enumerator;
	ike_sa_t *ike_sa;
	entry_t *entry;
	bool keep, success = TRUE;

	ike_sa = this->thread_sa->get(this->thread_sa);

	this->mutex->lock(this->mutex);
	enumerator = this->listeners->create_enumerator(this->listeners);
	while (enumerator->enumerate(enumerator, &entry))
	{
		if (entry->calling || !entry->listener->authorize)
		{
			continue;
		}
		entry->calling++;
		keep = entry->listener->authorize(entry->listener, ike_sa,
										  final, &success);
		entry->calling--;
		if (!keep)
		{
			unregister_listener(this, entry, enumerator);
		}
		if (!success)
		{
			break;
		}
	}
	enumerator->destroy(enumerator);
	this->mutex->unlock(this->mutex);
	if (!success)
	{
		alert(this, ALERT_AUTHORIZATION_FAILED);
	}
	return success;
}

METHOD(bus_t, narrow, void,
	private_bus_t *this, child_sa_t *child_sa, narrow_hook_t type,
	linked_list_t *local, linked_list_t *remote)
{
	enumerator_t *enumerator;
	ike_sa_t *ike_sa;
	entry_t *entry;
	bool keep;

	ike_sa = this->thread_sa->get(this->thread_sa);

	this->mutex->lock(this->mutex);
	enumerator = this->listeners->create_enumerator(this->listeners);
	while (enumerator->enumerate(enumerator, &entry))
	{
		if (entry->calling || !entry->listener->narrow)
		{
			continue;
		}
		entry->calling++;
		keep = entry->listener->narrow(entry->listener, ike_sa, child_sa,
									   type, local, remote);
		entry->calling--;
		if (!keep)
		{
			unregister_listener(this, entry, enumerator);
		}
	}
	enumerator->destroy(enumerator);
	this->mutex->unlock(this->mutex);
}

METHOD(bus_t, assign_vips, void,
	private_bus_t *this, ike_sa_t *ike_sa, bool assign)
{
	enumerator_t *enumerator;
	entry_t *entry;
	bool keep;

	this->mutex->lock(this->mutex);
	enumerator = this->listeners->create_enumerator(this->listeners);
	while (enumerator->enumerate(enumerator, &entry))
	{
		if (entry->calling || !entry->listener->assign_vips)
		{
			continue;
		}
		entry->calling++;
		keep = entry->listener->assign_vips(entry->listener, ike_sa, assign);
		entry->calling--;
		if (!keep)
		{
			unregister_listener(this, entry, enumerator);
		}
	}
	enumerator->destroy(enumerator);
	this->mutex->unlock(this->mutex);
}

METHOD(bus_t, handle_vips, void,
	private_bus_t *this, ike_sa_t *ike_sa, bool handle)
{
	enumerator_t *enumerator;
	entry_t *entry;
	bool keep;

	this->mutex->lock(this->mutex);
	enumerator = this->listeners->create_enumerator(this->listeners);
	while (enumerator->enumerate(enumerator, &entry))
	{
		if (entry->calling || !entry->listener->handle_vips)
		{
			continue;
		}
		entry->calling++;
		keep = entry->listener->handle_vips(entry->listener, ike_sa, handle);
		entry->calling--;
		if (!keep)
		{
			unregister_listener(this, entry, enumerator);
		}
	}
	enumerator->destroy(enumerator);
	this->mutex->unlock(this->mutex);
}

/**
 * Credential manager hook function to forward bus alerts
 */
static void hook_creds(private_bus_t *this, credential_hook_type_t type,
					   certificate_t *cert)
{
	switch (type)
	{
		case CRED_HOOK_EXPIRED:
			return alert(this, ALERT_CERT_EXPIRED, cert);
		case CRED_HOOK_REVOKED:
			return alert(this, ALERT_CERT_REVOKED, cert);
		case CRED_HOOK_VALIDATION_FAILED:
			return alert(this, ALERT_CERT_VALIDATION_FAILED, cert);
		case CRED_HOOK_NO_ISSUER:
			return alert(this, ALERT_CERT_NO_ISSUER, cert);
		case CRED_HOOK_UNTRUSTED_ROOT:
			return alert(this, ALERT_CERT_UNTRUSTED_ROOT, cert);
		case CRED_HOOK_EXCEEDED_PATH_LEN:
			return alert(this, ALERT_CERT_EXCEEDED_PATH_LEN, cert);
		case CRED_HOOK_POLICY_VIOLATION:
			return alert(this, ALERT_CERT_POLICY_VIOLATION, cert);
	}
}

METHOD(bus_t, destroy, void,
	private_bus_t *this)
{
	debug_t group;

	lib->credmgr->set_hook(lib->credmgr, NULL, NULL);
	for (group = 0; group < DBG_MAX; group++)
	{
		this->loggers[group]->destroy(this->loggers[group]);
	}
	this->loggers[DBG_MAX]->destroy_function(this->loggers[DBG_MAX],
											 (void*)free);
	this->listeners->destroy_function(this->listeners, (void*)free);
	this->thread_sa->destroy(this->thread_sa);
	this->log_lock->destroy(this->log_lock);
	this->mutex->destroy(this->mutex);
	free(this);
}

/*
 * Described in header.
 */
bus_t *bus_create()
{
	private_bus_t *this;
	debug_t group;

	INIT(this,
		.public = {
			.add_listener = _add_listener,
			.remove_listener = _remove_listener,
			.add_logger = _add_logger,
			.remove_logger = _remove_logger,
			.set_sa = _set_sa,
			.get_sa = _get_sa,
			.log = _log_,
			.vlog = _vlog,
			.alert = _alert,
			.ike_state_change = _ike_state_change,
			.child_state_change = _child_state_change,
			.message = _message,
			.ike_keys = _ike_keys,
			.child_keys = _child_keys,
			.ike_updown = _ike_updown,
			.ike_rekey = _ike_rekey,
			.ike_reestablish_pre = _ike_reestablish_pre,
			.ike_reestablish_post = _ike_reestablish_post,
			.child_updown = _child_updown,
			.child_rekey = _child_rekey,
			.authorize = _authorize,
			.narrow = _narrow,
			.assign_vips = _assign_vips,
			.handle_vips = _handle_vips,
			.destroy = _destroy,
		},
		.listeners = linked_list_create(),
		.mutex = mutex_create(MUTEX_TYPE_RECURSIVE),
		.log_lock = rwlock_create(RWLOCK_TYPE_DEFAULT),
		.thread_sa = thread_value_create(NULL),
	);

	for (group = 0; group <= DBG_MAX; group++)
	{
		this->loggers[group] = linked_list_create();
		this->max_level[group] = LEVEL_SILENT;
		this->max_vlevel[group] = LEVEL_SILENT;
	}

	lib->credmgr->set_hook(lib->credmgr, (credential_hook_t)hook_creds, this);

	return &this->public;
}
