/*
 * Copyright (C) 2008 Martin Willi
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

#include "ha_segments.h"

#include <threading/mutex.h>
#include <threading/condvar.h>
#include <collections/linked_list.h>
#include <threading/thread.h>
#include <processing/jobs/callback_job.h>

#define DEFAULT_HEARTBEAT_DELAY 1000
#define DEFAULT_HEARTBEAT_TIMEOUT 2100

typedef struct private_ha_segments_t private_ha_segments_t;

/**
 * Private data of an ha_segments_t object.
 */
struct private_ha_segments_t {

	/**
	 * Public ha_segments_t interface.
	 */
	ha_segments_t public;

	/**
	 * communication socket
	 */
	ha_socket_t *socket;

	/**
	 * Sync tunnel, if any
	 */
	ha_tunnel_t *tunnel;

	/**
	 * Interface to control segments at kernel level
	 */
	ha_kernel_t *kernel;

	/**
	 * Mutex to lock segment manipulation
	 */
	mutex_t *mutex;

	/**
	 * Condvar to wait for heartbeats
	 */
	condvar_t *condvar;

	/**
	 * Total number of ClusterIP segments
	 */
	u_int count;

	/**
	 * mask of active segments
	 */
	segment_mask_t active;

	/**
	 * Node number
	 */
	u_int node;

	/**
	 * Are we checking for heartbeats?
	 */
	bool heartbeat_active;

	/**
	 * Interval we send hearbeats
	 */
	int heartbeat_delay;

	/**
	 * Timeout for heartbeats received from other node
	 */
	int heartbeat_timeout;

	/**
	 * Interval to check for autobalance, 0 to disable
	 */
	int autobalance;
};

/**
 * Log currently active segments
 */
static void log_segments(private_ha_segments_t *this, bool activated,
						 u_int segment)
{
	char buf[64] = "none", *pos = buf;
	int i;
	bool first = TRUE;

	for (i = 1; i <= this->count; i++)
	{
		if (this->active & SEGMENTS_BIT(i))
		{
			if (first)
			{
				first = FALSE;
			}
			else
			{
				pos += snprintf(pos, buf + sizeof(buf) - pos, ",");
			}
			pos += snprintf(pos, buf + sizeof(buf) - pos, "%d", i);
		}
	}
	DBG1(DBG_CFG, "HA segment %d %sactivated, now active: %s",
		 segment, activated ? "" : "de", buf);
}

/**
 * Enable/Disable a specific segment
 */
static void enable_disable(private_ha_segments_t *this, u_int segment,
						   bool enable, bool notify)
{
	ike_sa_t *ike_sa;
	enumerator_t *enumerator;
	ike_sa_state_t old, new;
	ha_message_t *message = NULL;
	ha_message_type_t type;
	bool changes = FALSE;

	if (segment > this->count)
	{
		return;
	}

	if (enable)
	{
		old = IKE_PASSIVE;
		new = IKE_ESTABLISHED;
		type = HA_SEGMENT_TAKE;
		if (!(this->active & SEGMENTS_BIT(segment)))
		{
			this->active |= SEGMENTS_BIT(segment);
			this->kernel->activate(this->kernel, segment);
			changes = TRUE;
		}
	}
	else
	{
		old = IKE_ESTABLISHED;
		new = IKE_PASSIVE;
		type = HA_SEGMENT_DROP;
		if (this->active & SEGMENTS_BIT(segment))
		{
			this->active &= ~SEGMENTS_BIT(segment);
			this->kernel->deactivate(this->kernel, segment);
			changes = TRUE;
		}
	}

	if (changes)
	{
		enumerator = charon->ike_sa_manager->create_enumerator(
												charon->ike_sa_manager, TRUE);
		while (enumerator->enumerate(enumerator, &ike_sa))
		{
			if (ike_sa->get_state(ike_sa) != old)
			{
				continue;
			}
			if (this->tunnel && this->tunnel->is_sa(this->tunnel, ike_sa))
			{
				continue;
			}
			if (this->kernel->get_segment(this->kernel,
									ike_sa->get_other_host(ike_sa)) == segment)
			{
				ike_sa->set_state(ike_sa, new);
			}
		}
		enumerator->destroy(enumerator);
		log_segments(this, enable, segment);
	}

	if (notify)
	{
		message = ha_message_create(type);
		message->add_attribute(message, HA_SEGMENT, segment);
		this->socket->push(this->socket, message);
		message->destroy(message);
	}
}

/**
 * Enable/Disable all or a specific segment, do locking
 */
static void enable_disable_all(private_ha_segments_t *this, u_int segment,
							   bool enable, bool notify)
{
	int i;

	this->mutex->lock(this->mutex);
	if (segment == 0)
	{
		for (i = 1; i <= this->count; i++)
		{
			enable_disable(this, i, enable, notify);
		}
	}
	else
	{
		enable_disable(this, segment, enable, notify);
	}
	this->mutex->unlock(this->mutex);
}

METHOD(ha_segments_t, activate, void,
	private_ha_segments_t *this, u_int segment, bool notify)
{
	enable_disable_all(this, segment, TRUE, notify);
}

METHOD(ha_segments_t, deactivate, void,
	private_ha_segments_t *this, u_int segment, bool notify)
{
	enable_disable_all(this, segment, FALSE, notify);
}

METHOD(listener_t, alert_hook, bool,
	private_ha_segments_t *this, ike_sa_t *ike_sa, alert_t alert, va_list args)
{
	if (alert == ALERT_SHUTDOWN_SIGNAL)
	{
		if (this->heartbeat_active)
		{
			DBG1(DBG_CFG, "HA heartbeat active, dropping all segments");
			deactivate(this, 0, TRUE);
		}
		else
		{
			DBG1(DBG_CFG, "no HA heartbeat active, closing IKE_SAs");
		}
	}
	return TRUE;
}

/**
 * Monitor heartbeat activity of remote node
 */
static job_requeue_t watchdog(private_ha_segments_t *this)
{
	bool timeout, oldstate;

	this->mutex->lock(this->mutex);
	thread_cleanup_push((void*)this->mutex->unlock, this->mutex);
	oldstate = thread_cancelability(TRUE);
	timeout = this->condvar->timed_wait(this->condvar, this->mutex,
										this->heartbeat_timeout);
	thread_cancelability(oldstate);
	thread_cleanup_pop(TRUE);
	if (timeout)
	{
		DBG1(DBG_CFG, "no heartbeat received, taking all segments");
		activate(this, 0, TRUE);
		/* disable heartbeat detection util we get one */
		this->heartbeat_active = FALSE;
		return JOB_REQUEUE_NONE;
	}
	return JOB_REQUEUE_DIRECT;
}

/**
 * Start the heartbeat detection thread
 */
static void start_watchdog(private_ha_segments_t *this)
{
	this->heartbeat_active = TRUE;
	lib->processor->queue_job(lib->processor,
		(job_t*)callback_job_create_with_prio((callback_job_cb_t)watchdog, this,
				NULL, (callback_job_cancel_t)return_false, JOB_PRIO_CRITICAL));
}

METHOD(ha_segments_t, handle_status, void,
	private_ha_segments_t *this, segment_mask_t mask)
{
	segment_mask_t missing, twice;
	int i;

	this->mutex->lock(this->mutex);

	missing = ~(this->active | mask);
	twice = this->active & mask;

	for (i = 1; i <= this->count; i++)
	{
		if (missing & SEGMENTS_BIT(i))
		{
			if (this->node == i % 2)
			{
				DBG1(DBG_CFG, "HA segment %d was not handled, taking", i);
				enable_disable(this, i, TRUE, TRUE);
			}
			else
			{
				DBG1(DBG_CFG, "HA segment %d was not handled, dropping", i);
				enable_disable(this, i, FALSE, TRUE);
			}
		}
		if (twice & SEGMENTS_BIT(i))
		{
			if (this->node == i % 2)
			{
				DBG1(DBG_CFG, "HA segment %d was handled twice, taking", i);
				enable_disable(this, i, TRUE, TRUE);
			}
			else
			{
				DBG1(DBG_CFG, "HA segment %d was handled twice, dropping", i);
				enable_disable(this, i, FALSE, TRUE);
			}
		}
	}

	this->condvar->signal(this->condvar);
	this->mutex->unlock(this->mutex);

	if (!this->heartbeat_active)
	{
		DBG1(DBG_CFG, "received heartbeat, reenabling watchdog");
		start_watchdog(this);
	}
}

/**
 * Send a status message with our active segments
 */
static job_requeue_t send_status(private_ha_segments_t *this)
{
	ha_message_t *message;
	int i;

	message = ha_message_create(HA_STATUS);

	this->mutex->lock(this->mutex);
	for (i = 1; i <= this->count; i++)
	{
		if (this->active & SEGMENTS_BIT(i))
		{
			message->add_attribute(message, HA_SEGMENT, i);
		}
	}
	this->mutex->unlock(this->mutex);

	this->socket->push(this->socket, message);
	message->destroy(message);

	/* schedule next invocation */
	return JOB_RESCHEDULE_MS(this->heartbeat_delay);
}

/**
 * Start the heartbeat sending task
 */
static void start_heartbeat(private_ha_segments_t *this)
{
	lib->processor->queue_job(lib->processor,
		(job_t*)callback_job_create_with_prio((callback_job_cb_t)send_status,
			this, NULL, (callback_job_cancel_t)return_false, JOB_PRIO_CRITICAL));
}

/**
 * Take a segment if we are handling less than half of segments
 */
static job_requeue_t autobalance(private_ha_segments_t *this)
{
	int i, active = 0;

	this->mutex->lock(this->mutex);

	for (i = 1; i <= this->count; i++)
	{
		if (this->active & SEGMENTS_BIT(i))
		{
			active++;
		}
	}
	if (active < this->count / 2)
	{
		for (i = 1; i <= this->count; i++)
		{
			if (!(this->active & SEGMENTS_BIT(i)))
			{
				DBG1(DBG_CFG, "autobalancing HA (%d/%d active), taking %d",
					 active, this->count, i);
				enable_disable(this, i, TRUE, TRUE);
				/* we claim only one in each interval */
				break;
			}
		}
	}

	this->mutex->unlock(this->mutex);

	return JOB_RESCHEDULE(this->autobalance);
}

/**
 * Schedule autobalancing
 */
static void start_autobalance(private_ha_segments_t *this)
{
	DBG1(DBG_CFG, "scheduling HA autobalance every %ds", this->autobalance);
	lib->scheduler->schedule_job(lib->scheduler,
		(job_t*)callback_job_create_with_prio((callback_job_cb_t)autobalance,
			this, NULL, (callback_job_cancel_t)return_false, JOB_PRIO_CRITICAL),
		this->autobalance);
}

METHOD(ha_segments_t, is_active, bool,
	private_ha_segments_t *this, u_int segment)
{
	return (this->active & SEGMENTS_BIT(segment)) != 0;
}

METHOD(ha_segments_t, count, u_int,
	private_ha_segments_t *this)
{
	return this->count;
}

METHOD(ha_segments_t, destroy, void,
	private_ha_segments_t *this)
{
	this->mutex->destroy(this->mutex);
	this->condvar->destroy(this->condvar);
	free(this);
}

/**
 * See header
 */
ha_segments_t *ha_segments_create(ha_socket_t *socket, ha_kernel_t *kernel,
								  ha_tunnel_t *tunnel, u_int count, u_int node,
								  bool monitor)
{
	private_ha_segments_t *this;

	INIT(this,
		.public = {
			.listener = {
				.alert = _alert_hook,
			},
			.activate = _activate,
			.deactivate = _deactivate,
			.handle_status = _handle_status,
			.is_active = _is_active,
			.count = _count,
			.destroy = _destroy,
		},
		.socket = socket,
		.tunnel = tunnel,
		.kernel = kernel,
		.count = count,
		.node = node,
		.mutex = mutex_create(MUTEX_TYPE_DEFAULT),
		.condvar = condvar_create(CONDVAR_TYPE_DEFAULT),
		.heartbeat_delay = lib->settings->get_int(lib->settings,
				"%s.plugins.ha.heartbeat_delay", DEFAULT_HEARTBEAT_DELAY,
				lib->ns),
		.heartbeat_timeout = lib->settings->get_int(lib->settings,
				"%s.plugins.ha.heartbeat_timeout", DEFAULT_HEARTBEAT_TIMEOUT,
				lib->ns),
		.autobalance = lib->settings->get_int(lib->settings,
				"%s.plugins.ha.autobalance", 0, lib->ns),
	);

	if (monitor)
	{
		DBG1(DBG_CFG, "starting HA heartbeat, delay %dms, timeout %dms",
			 this->heartbeat_delay, this->heartbeat_timeout);
		start_heartbeat(this);
		start_watchdog(this);
	}
	if (this->autobalance)
	{
		start_autobalance(this);
	}

	return &this->public;
}
