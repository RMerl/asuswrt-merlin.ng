/*
 * Copyright (C) 2012 Martin Willi
 * Copyright (C) 2012 revosec AG
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

#include "load_tester_control.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <errno.h>

#include <daemon.h>
#include <collections/hashtable.h>
#include <threading/thread.h>
#include <threading/mutex.h>
#include <threading/condvar.h>
#include <processing/jobs/callback_job.h>

typedef struct private_load_tester_control_t private_load_tester_control_t;
typedef struct init_listener_t init_listener_t;

/**
 * Private data of an load_tester_control_t object.
 */
struct private_load_tester_control_t {

	/**
	 * Public load_tester_control_t interface.
	 */
	load_tester_control_t public;

	/**
	 * Load tester control stream service
	 */
	stream_service_t *service;
};

/**
 * Listener to follow initiation progress
 */
struct init_listener_t {

	/**
	 * implements listener_t
	 */
	listener_t listener;

	/**
	 * Output stream to log to
	 */
	FILE *stream;

	/**
	 * IKE_SAs we have started to initiate
	 */
	hashtable_t *initiated;

	/**
	 * IKE_SAs we have completed to initate (success or failure)
	 */
	hashtable_t *completed;

	/**
	 * Mutex to lock IKE_SA tables
	 */
	mutex_t *mutex;

	/**
	 * Condvar to wait for completion
	 */
	condvar_t *condvar;
};

/**
 * Hashtable hash function
 */
static u_int hash(uintptr_t id)
{
	return id;
}

/**
 * Hashtable hash function
 */
static bool equals(uintptr_t a, uintptr_t b)
{
	return a == b;
}

METHOD(listener_t, alert, bool,
	init_listener_t *this, ike_sa_t *ike_sa, alert_t alert, va_list args)
{
	if (alert == ALERT_RETRANSMIT_SEND)
	{
		uintptr_t id;
		bool match = FALSE;

		id = ike_sa->get_unique_id(ike_sa);
		this->mutex->lock(this->mutex);
		if (this->initiated->get(this->initiated, (void*)id))
		{
			match = TRUE;
		}
		this->mutex->unlock(this->mutex);

		if (match)
		{
			fprintf(this->stream, "*");
			fflush(this->stream);
		}
	}
	return TRUE;
}

METHOD(listener_t, ike_state_change, bool,
	init_listener_t *this, ike_sa_t *ike_sa, ike_sa_state_t state)
{
	if (state == IKE_ESTABLISHED || state == IKE_DESTROYING)
	{
		uintptr_t id;
		bool match = FALSE;

		id = ike_sa->get_unique_id(ike_sa);
		this->mutex->lock(this->mutex);
		if (this->initiated->get(this->initiated, (void*)id))
		{
			match = !this->completed->put(this->completed, (void*)id, (void*)id);
		}
		this->mutex->unlock(this->mutex);

		if (match)
		{
			this->condvar->signal(this->condvar);
			fprintf(this->stream, state == IKE_ESTABLISHED ? "+" : "-");
			fflush(this->stream);
		}
	}
	return TRUE;
}

/**
 * Logging callback function used during initiate
 */
static bool initiate_cb(init_listener_t *this, debug_t group, level_t level,
						ike_sa_t *ike_sa, const char *message)
{
	uintptr_t id;

	if (ike_sa)
	{
		id = ike_sa->get_unique_id(ike_sa);
		this->mutex->lock(this->mutex);
		this->initiated->put(this->initiated, (void*)id, (void*)id);
		this->mutex->unlock(this->mutex);

		return FALSE;
	}

	return TRUE;
}

/**
 * Accept connections, initiate load-test, write progress to stream
 */
static bool on_accept(private_load_tester_control_t *this, stream_t *io)
{
	init_listener_t *listener;
	enumerator_t *enumerator;
	peer_cfg_t *peer_cfg;
	child_cfg_t *child_cfg;
	u_int i, count, failed = 0, delay = 0;
	char buf[16] = "";
	FILE *stream;

	stream = io->get_file(io);
	if (!stream)
	{
		return FALSE;
	}
	fflush(stream);
	if (fgets(buf, sizeof(buf), stream) == NULL)
	{
		fclose(stream);
		return FALSE;
	}
	if (sscanf(buf, "%u %u", &count, &delay) < 1)
	{
		fclose(stream);
		return FALSE;
	}

	INIT(listener,
		.listener = {
			.ike_state_change = _ike_state_change,
			.alert = _alert,
		},
		.stream = stream,
		.initiated = hashtable_create((void*)hash, (void*)equals, count),
		.completed = hashtable_create((void*)hash, (void*)equals, count),
		.mutex = mutex_create(MUTEX_TYPE_DEFAULT),
		.condvar = condvar_create(CONDVAR_TYPE_DEFAULT),
	);

	charon->bus->add_listener(charon->bus, &listener->listener);

	for (i = 0; i < count; i++)
	{
		peer_cfg = charon->backends->get_peer_cfg_by_name(charon->backends,
														  "load-test");
		if (!peer_cfg)
		{
			failed++;
			fprintf(stream, "!");
			continue;
		}
		enumerator = peer_cfg->create_child_cfg_enumerator(peer_cfg);
		if (!enumerator->enumerate(enumerator, &child_cfg))
		{
			enumerator->destroy(enumerator);
			peer_cfg->destroy(peer_cfg);
			failed++;
			fprintf(stream, "!");
			continue;
		}
		enumerator->destroy(enumerator);

		switch (charon->controller->initiate(charon->controller,
										peer_cfg, child_cfg->get_ref(child_cfg),
										(void*)initiate_cb, listener, 0))
		{
			case NEED_MORE:
				/* Callback returns FALSE once it got track of this IKE_SA.
				 * FALL */
			case SUCCESS:
				fprintf(stream, ".");
				break;
			default:
				fprintf(stream, "!");
				break;
		}
		if (delay)
		{
			usleep(delay * 1000);
		}
		fflush(stream);
	}

	listener->mutex->lock(listener->mutex);
	while (listener->completed->get_count(listener->completed) < count - failed)
	{
		listener->condvar->wait(listener->condvar, listener->mutex);
	}
	listener->mutex->unlock(listener->mutex);

	charon->bus->remove_listener(charon->bus, &listener->listener);

	listener->initiated->destroy(listener->initiated);
	listener->completed->destroy(listener->completed);
	listener->mutex->destroy(listener->mutex);
	listener->condvar->destroy(listener->condvar);
	free(listener);

	fprintf(stream, "\n");
	fclose(stream);

	return FALSE;
}

METHOD(load_tester_control_t, destroy, void,
	private_load_tester_control_t *this)
{
	DESTROY_IF(this->service);
	free(this);
}

/**
 * See header
 */
load_tester_control_t *load_tester_control_create()
{
	private_load_tester_control_t *this;
	char *uri;

	INIT(this,
		.public = {
			.destroy = _destroy,
		},
	);

	uri = lib->settings->get_str(lib->settings,
				"%s.plugins.load-tester.socket", "unix://" LOAD_TESTER_SOCKET,
				lib->ns);
	this->service = lib->streams->create_service(lib->streams, uri, 10);
	if (this->service)
	{
		this->service->on_accept(this->service, (stream_service_cb_t)on_accept,
								 this, JOB_PRIO_CRITICAL, 0);
	}
	else
	{
		DBG1(DBG_CFG, "creating load-tester control socket failed");
	}
	return &this->public;
}
