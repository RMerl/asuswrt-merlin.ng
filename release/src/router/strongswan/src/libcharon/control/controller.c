/*
 * Copyright (C) 2011-2015 Tobias Brunner
 * Copyright (C) 2007-2011 Martin Willi
 * Copyright (C) 2011 revosec AG
 * HSR Hochschule fuer Technik Rapperswil
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

#include "controller.h"

#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>

#include <daemon.h>
#include <library.h>
#include <threading/thread.h>
#include <threading/spinlock.h>
#include <threading/semaphore.h>

typedef struct private_controller_t private_controller_t;
typedef struct interface_listener_t interface_listener_t;
typedef struct interface_logger_t interface_logger_t;

/**
 * Private data of an stroke_t object.
 */
struct private_controller_t {

	/**
	 * Public part of stroke_t object.
	 */
	controller_t public;
};

/**
 * helper struct for the logger interface
 */
struct interface_logger_t {
	/**
	 * public logger interface
	 */
	logger_t public;

	/**
	 * reference to the listener
	 */
	interface_listener_t *listener;

	/**
	 *  interface callback (listener gets redirected to here)
	 */
	controller_cb_t callback;

	/**
	 * user parameter to pass to callback
	 */
	void *param;
};

/**
 * helper struct to map listener callbacks to interface callbacks
 */
struct interface_listener_t {

	/**
	 * public bus listener interface
	 */
	listener_t public;

	/**
	 * logger interface
	 */
	interface_logger_t logger;

	/**
	 * status of the operation, return to method callers
	 */
	status_t status;

	/**
	 * child configuration, used for initiate
	 */
	child_cfg_t *child_cfg;

	/**
	 * peer configuration, used for initiate
	 */
	peer_cfg_t *peer_cfg;

	/**
	 * IKE_SA to handle
	 */
	ike_sa_t *ike_sa;

	/**
	 * unique ID, used for various methods
	 */
	uint32_t id;

	/**
	 * semaphore to implement wait_for_listener()
	 */
	semaphore_t *done;

	/**
	 * spinlock to update the IKE_SA handle properly
	 */
	spinlock_t *lock;

	union {
		/**
		 * whether to check limits during initiation
		 */
		bool limits;

		/**
		 * whether to force termination
		 */
		bool force;
	} options;
};


typedef struct interface_job_t interface_job_t;

/**
 * job for asynchronous listen operations
 */
struct interface_job_t {

	/**
	 * job interface
	 */
	job_t public;

	/**
	 * associated listener
	 */
	interface_listener_t listener;

	/**
	 * the job is reference counted as the thread executing a job as well as
	 * the thread waiting in wait_for_listener() require it but either of them
	 * could be done first
	 */
	refcount_t refcount;
};

/**
 * This function wakes a thread that is waiting in wait_for_listener(),
 * either from a listener or from a job.
 */
static inline bool listener_done(interface_listener_t *listener)
{
	if (listener->done)
	{
		listener->done->post(listener->done);
	}
	return FALSE;
}

/**
 * thread_cleanup_t handler to unregister a listener.
 */
static void listener_unregister(interface_listener_t *listener)
{
	charon->bus->remove_listener(charon->bus, &listener->public);
	charon->bus->remove_logger(charon->bus, &listener->logger.public);
}

/**
 * Registers the listener, executes the job and then waits synchronously until
 * the listener is done or the timeout occurred.
 *
 * @note Use 'return listener_done(listener)' to properly unregister a listener
 *
 * @param listener  listener to register
 * @param job       job to execute asynchronously when registered, or NULL
 * @param timeout   max timeout in ms to listen for events, 0 to disable
 * @return          TRUE if timed out
 */
static bool wait_for_listener(interface_job_t *job, u_int timeout)
{
	interface_listener_t *listener = &job->listener;
	bool old, timed_out = FALSE;

	/* avoid that the job is destroyed too early */
	ref_get(&job->refcount);

	listener->done = semaphore_create(0);

	charon->bus->add_logger(charon->bus, &listener->logger.public);
	charon->bus->add_listener(charon->bus, &listener->public);
	lib->processor->queue_job(lib->processor, &job->public);

	thread_cleanup_push((thread_cleanup_t)listener_unregister, listener);
	old = thread_cancelability(TRUE);
	if (timeout)
	{
		timed_out = listener->done->timed_wait(listener->done, timeout);
	}
	else
	{
		listener->done->wait(listener->done);
	}
	thread_cancelability(old);
	thread_cleanup_pop(TRUE);
	return timed_out;
}

METHOD(logger_t, listener_log, void,
	interface_logger_t *this, debug_t group, level_t level, int thread,
	ike_sa_t *ike_sa, const char *message)
{
	ike_sa_t *target;

	this->listener->lock->lock(this->listener->lock);
	target = this->listener->ike_sa;
	this->listener->lock->unlock(this->listener->lock);

	if (target == ike_sa)
	{
		if (!this->callback(this->param, group, level, ike_sa, message))
		{
			this->listener->status = NEED_MORE;
			listener_done(this->listener);
		}
	}
}

METHOD(logger_t, listener_get_level, level_t,
	interface_logger_t *this, debug_t group)
{
	/* in order to allow callback listeners to decide what they want to log
	 * we request any log message, but only if we actually want logging */
	return this->callback == controller_cb_empty ? LEVEL_SILENT : LEVEL_PRIVATE;
}

METHOD(job_t, get_priority_medium, job_priority_t,
	job_t *this)
{
	return JOB_PRIO_MEDIUM;
}

METHOD(listener_t, ike_state_change, bool,
	interface_listener_t *this, ike_sa_t *ike_sa, ike_sa_state_t state)
{
	ike_sa_t *target;

	this->lock->lock(this->lock);
	target = this->ike_sa;
	this->lock->unlock(this->lock);

	if (target == ike_sa)
	{
		switch (state)
		{
#ifdef ME
			case IKE_ESTABLISHED:
			{	/* mediation connections are complete without CHILD_SA */
				peer_cfg_t *peer_cfg = ike_sa->get_peer_cfg(ike_sa);

				if (peer_cfg->is_mediation(peer_cfg))
				{
					this->status = SUCCESS;
					return listener_done(this);
				}
				break;
			}
#endif /* ME */
			case IKE_DESTROYING:
				return listener_done(this);
			default:
				break;
		}
	}
	return TRUE;
}

METHOD(listener_t, ike_state_change_terminate, bool,
	interface_listener_t *this, ike_sa_t *ike_sa, ike_sa_state_t state)
{
	ike_sa_t *target;

	this->lock->lock(this->lock);
	target = this->ike_sa;
	this->lock->unlock(this->lock);

	if (target == ike_sa)
	{
		switch (state)
		{
			case IKE_DESTROYING:
				this->status = SUCCESS;
				return listener_done(this);
			default:
				break;
		}
	}
	return TRUE;
}

METHOD(listener_t, child_state_change, bool,
	interface_listener_t *this, ike_sa_t *ike_sa, child_sa_t *child_sa,
	child_sa_state_t state)
{
	ike_sa_t *target;

	this->lock->lock(this->lock);
	target = this->ike_sa;
	this->lock->unlock(this->lock);

	if (target == ike_sa)
	{
		switch (state)
		{
			case CHILD_INSTALLED:
				this->status = SUCCESS;
				return listener_done(this);
			case CHILD_DESTROYING:
				switch (child_sa->get_state(child_sa))
				{
					case CHILD_RETRYING:
						/* retrying with a different DH group; survive another
						 * initiation round */
						this->status = NEED_MORE;
						return TRUE;
					case CHILD_CREATED:
						if (this->status == NEED_MORE)
						{
							this->status = FAILED;
							return TRUE;
						}
						break;
					default:
						break;
				}
				return listener_done(this);
			default:
				break;
		}
	}
	return TRUE;
}

METHOD(listener_t, child_state_change_terminate, bool,
	interface_listener_t *this, ike_sa_t *ike_sa, child_sa_t *child_sa,
	child_sa_state_t state)
{
	ike_sa_t *target;

	this->lock->lock(this->lock);
	target = this->ike_sa;
	this->lock->unlock(this->lock);

	if (target == ike_sa)
	{
		switch (state)
		{
			case CHILD_DESTROYING:
				switch (child_sa->get_state(child_sa))
				{
					case CHILD_DELETED:
						/* proper delete */
						this->status = SUCCESS;
						break;
					default:
						break;
				}
				return listener_done(this);
			default:
				break;
		}
	}
	return TRUE;
}

METHOD(job_t, destroy_job, void,
	interface_job_t *this)
{
	if (ref_put(&this->refcount))
	{
		this->listener.lock->destroy(this->listener.lock);
		DESTROY_IF(this->listener.done);
		free(this);
	}
}

METHOD(controller_t, create_ike_sa_enumerator, enumerator_t*,
	private_controller_t *this, bool wait)
{
	return charon->ike_sa_manager->create_enumerator(charon->ike_sa_manager,
													 wait);
}

METHOD(job_t, initiate_execute, job_requeue_t,
	interface_job_t *job)
{
	ike_sa_t *ike_sa;
	interface_listener_t *listener = &job->listener;
	peer_cfg_t *peer_cfg = listener->peer_cfg;

	ike_sa = charon->ike_sa_manager->checkout_by_config(charon->ike_sa_manager,
														peer_cfg);
	if (!ike_sa)
	{
		listener->child_cfg->destroy(listener->child_cfg);
		peer_cfg->destroy(peer_cfg);
		listener->status = FAILED;
		listener_done(listener);
		return JOB_REQUEUE_NONE;
	}
	listener->lock->lock(listener->lock);
	listener->ike_sa = ike_sa;
	listener->lock->unlock(listener->lock);

	if (ike_sa->get_peer_cfg(ike_sa) == NULL)
	{
		ike_sa->set_peer_cfg(ike_sa, peer_cfg);
	}
	peer_cfg->destroy(peer_cfg);

	if (listener->options.limits && ike_sa->get_state(ike_sa) == IKE_CREATED)
	{	/* only check if we are not reusing an IKE_SA */
		u_int half_open, limit_half_open, limit_job_load;

		half_open = charon->ike_sa_manager->get_half_open_count(
										charon->ike_sa_manager, NULL, FALSE);
		limit_half_open = lib->settings->get_int(lib->settings,
										"%s.init_limit_half_open", 0, lib->ns);
		limit_job_load = lib->settings->get_int(lib->settings,
										"%s.init_limit_job_load", 0, lib->ns);
		if (limit_half_open && half_open >= limit_half_open)
		{
			DBG1(DBG_IKE, "abort IKE_SA initiation, half open IKE_SA count of "
				 "%d exceeds limit of %d", half_open, limit_half_open);
			charon->ike_sa_manager->checkin_and_destroy(charon->ike_sa_manager,
														ike_sa);
			listener->child_cfg->destroy(listener->child_cfg);
			listener->status = INVALID_STATE;
			listener_done(listener);
			return JOB_REQUEUE_NONE;
		}
		if (limit_job_load)
		{
			u_int jobs = 0, i;

			for (i = 0; i < JOB_PRIO_MAX; i++)
			{
				jobs += lib->processor->get_job_load(lib->processor, i);
			}
			if (jobs > limit_job_load)
			{
				DBG1(DBG_IKE, "abort IKE_SA initiation, job load of %d exceeds "
					 "limit of %d", jobs, limit_job_load);
				charon->ike_sa_manager->checkin_and_destroy(
												charon->ike_sa_manager, ike_sa);
				listener->child_cfg->destroy(listener->child_cfg);
				listener->status = INVALID_STATE;
				listener_done(listener);
				return JOB_REQUEUE_NONE;
			}
		}
	}

	if (ike_sa->initiate(ike_sa, listener->child_cfg, 0, NULL, NULL) == SUCCESS)
	{
		if (!listener->logger.callback)
		{
			listener->status = SUCCESS;
		}
		charon->ike_sa_manager->checkin(charon->ike_sa_manager, ike_sa);
	}
	else
	{
		listener->status = FAILED;
		charon->ike_sa_manager->checkin_and_destroy(charon->ike_sa_manager,
													ike_sa);
	}
	return JOB_REQUEUE_NONE;
}

METHOD(controller_t, initiate, status_t,
	private_controller_t *this, peer_cfg_t *peer_cfg, child_cfg_t *child_cfg,
	controller_cb_t callback, void *param, u_int timeout, bool limits)
{
	interface_job_t *job;
	status_t status;

	INIT(job,
		.listener = {
			.public = {
				.ike_state_change = _ike_state_change,
				.child_state_change = _child_state_change,
			},
			.logger = {
				.public = {
					.log = _listener_log,
					.get_level = _listener_get_level,
				},
				.callback = callback,
				.param = param,
			},
			.status = FAILED,
			.child_cfg = child_cfg,
			.peer_cfg = peer_cfg,
			.lock = spinlock_create(),
			.options.limits = limits,
		},
		.public = {
			.execute = _initiate_execute,
			.get_priority = _get_priority_medium,
			.destroy = _destroy_job,
		},
		.refcount = 1,
	);
	job->listener.logger.listener = &job->listener;
	thread_cleanup_push((void*)destroy_job, job);

	if (callback == NULL)
	{
		initiate_execute(job);
	}
	else
	{
		if (wait_for_listener(job, timeout))
		{
			job->listener.status = OUT_OF_RES;
		}
	}
	status = job->listener.status;
	thread_cleanup_pop(TRUE);
	return status;
}

METHOD(job_t, terminate_ike_execute, job_requeue_t,
	interface_job_t *job)
{
	interface_listener_t *listener = &job->listener;
	uint32_t unique_id = listener->id;
	ike_sa_t *ike_sa;

	ike_sa = charon->ike_sa_manager->checkout_by_id(charon->ike_sa_manager,
													unique_id);
	if (!ike_sa)
	{
		DBG1(DBG_IKE, "unable to terminate IKE_SA: ID %d not found", unique_id);
		listener->status = NOT_FOUND;
		/* release listener */
		listener_done(listener);
		return JOB_REQUEUE_NONE;
	}
	listener->lock->lock(listener->lock);
	listener->ike_sa = ike_sa;
	listener->lock->unlock(listener->lock);

	if (ike_sa->delete(ike_sa, listener->options.force) != DESTROY_ME)
	{	/* delete queued */
		listener->status = FAILED;
		charon->ike_sa_manager->checkin(charon->ike_sa_manager, ike_sa);
	}
	else
	{
		if (!listener->logger.callback)
		{
			listener->status = SUCCESS;
		}
		charon->ike_sa_manager->checkin_and_destroy(charon->ike_sa_manager,
													ike_sa);
	}
	return JOB_REQUEUE_NONE;
}

METHOD(controller_t, terminate_ike, status_t,
	controller_t *this, uint32_t unique_id, bool force,
	controller_cb_t callback, void *param, u_int timeout)
{
	interface_job_t *job;
	status_t status;

	INIT(job,
		.listener = {
			.public = {
				.ike_state_change = _ike_state_change_terminate,
			},
			.logger = {
				.public = {
					.log = _listener_log,
					.get_level = _listener_get_level,
				},
				.callback = callback,
				.param = param,
			},
			.status = FAILED,
			.id = unique_id,
			.lock = spinlock_create(),
		},
		.public = {
			.execute = _terminate_ike_execute,
			.get_priority = _get_priority_medium,
			.destroy = _destroy_job,
		},
		.refcount = 1,
	);
	job->listener.logger.listener = &job->listener;
	thread_cleanup_push((void*)destroy_job, job);

	if (callback == NULL)
	{
		job->listener.options.force = force;
		terminate_ike_execute(job);
	}
	else
	{
		if (!timeout)
		{
			job->listener.options.force = force;
		}
		if (wait_for_listener(job, timeout))
		{
			job->listener.status = OUT_OF_RES;

			if (force)
			{	/* force termination once timeout is reached */
				job->listener.options.force = TRUE;
				terminate_ike_execute(job);
			}
		}
	}
	status = job->listener.status;
	thread_cleanup_pop(TRUE);
	return status;
}

METHOD(job_t, terminate_child_execute, job_requeue_t,
	interface_job_t *job)
{
	interface_listener_t *listener = &job->listener;
	uint32_t id = listener->id;
	child_sa_t *child_sa;
	ike_sa_t *ike_sa;

	ike_sa = charon->child_sa_manager->checkout_by_id(charon->child_sa_manager,
													  id, &child_sa);
	if (!ike_sa)
	{
		DBG1(DBG_IKE, "unable to terminate, CHILD_SA with ID %d not found", id);
		listener->status = NOT_FOUND;
		/* release listener */
		listener_done(listener);
		return JOB_REQUEUE_NONE;
	}
	listener->lock->lock(listener->lock);
	listener->ike_sa = ike_sa;
	listener->lock->unlock(listener->lock);

	if (ike_sa->delete_child_sa(ike_sa, child_sa->get_protocol(child_sa),
					child_sa->get_spi(child_sa, TRUE), FALSE) != DESTROY_ME)
	{
		if (!listener->logger.callback)
		{
			listener->status = SUCCESS;
		}
		charon->ike_sa_manager->checkin(charon->ike_sa_manager, ike_sa);
	}
	else
	{
		listener->status = FAILED;
		charon->ike_sa_manager->checkin_and_destroy(charon->ike_sa_manager,
													ike_sa);
	}
	return JOB_REQUEUE_NONE;
}

METHOD(controller_t, terminate_child, status_t,
	controller_t *this, uint32_t unique_id,
	controller_cb_t callback, void *param, u_int timeout)
{
	interface_job_t *job;
	status_t status;

	INIT(job,
		.listener = {
			.public = {
				.ike_state_change = _ike_state_change_terminate,
				.child_state_change = _child_state_change_terminate,
			},
			.logger = {
				.public = {
					.log = _listener_log,
					.get_level = _listener_get_level,
				},
				.callback = callback,
				.param = param,
			},
			.status = FAILED,
			.id = unique_id,
			.lock = spinlock_create(),
		},
		.public = {
			.execute = _terminate_child_execute,
			.get_priority = _get_priority_medium,
			.destroy = _destroy_job,
		},
		.refcount = 1,
	);
	job->listener.logger.listener = &job->listener;
	thread_cleanup_push((void*)destroy_job, job);

	if (callback == NULL)
	{
		terminate_child_execute(job);
	}
	else
	{
		if (wait_for_listener(job, timeout))
		{
			job->listener.status = OUT_OF_RES;
		}
	}
	status = job->listener.status;
	thread_cleanup_pop(TRUE);
	return status;
}

/**
 * See header
 */
bool controller_cb_empty(void *param, debug_t group, level_t level,
						 ike_sa_t *ike_sa, const char *message)
{
	return TRUE;
}

METHOD(controller_t, destroy, void,
	private_controller_t *this)
{
	free(this);
}

/*
 * Described in header-file
 */
controller_t *controller_create(void)
{
	private_controller_t *this;

	INIT(this,
		.public = {
			.create_ike_sa_enumerator = _create_ike_sa_enumerator,
			.initiate = _initiate,
			.terminate_ike = _terminate_ike,
			.terminate_child = _terminate_child,
			.destroy = _destroy,
		},
	);

	return &this->public;
}
