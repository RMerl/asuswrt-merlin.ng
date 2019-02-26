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

#include "pt_tls_dispatcher.h"
#include "pt_tls_server.h"

#include <threading/thread.h>
#include <utils/debug.h>
#include <processing/jobs/callback_job.h>

#include <errno.h>
#include <string.h>
#include <unistd.h>

typedef struct private_pt_tls_dispatcher_t private_pt_tls_dispatcher_t;

/**
 * Private data of an pt_tls_dispatcher_t object.
 */
struct private_pt_tls_dispatcher_t {

	/**
	 * Public pt_tls_dispatcher_t interface.
	 */
	pt_tls_dispatcher_t public;

	/**
	 * Listening socket
	 */
	int fd;

	/**
	 * Client authentication requirements
	 */
	pt_tls_auth_t auth;

	/**
	 * Server identity
	 */
	identification_t *server;

	/**
	 * Peer identity
	 */
	identification_t *peer;

	/**
	 * TNCCS protocol handler constructor
	 */
	pt_tls_tnccs_constructor_t *create;
};

/**
 * Open listening server socket
 */
static bool open_socket(private_pt_tls_dispatcher_t *this, host_t *host)
{
	this->fd = socket(AF_INET, SOCK_STREAM, 0);
	if (this->fd == -1)
	{
		DBG1(DBG_TNC, "opening PT-TLS socket failed: %s", strerror(errno));
		return FALSE;
	}
	if (bind(this->fd, host->get_sockaddr(host),
			 *host->get_sockaddr_len(host)) == -1)
	{
		DBG1(DBG_TNC, "binding to PT-TLS socket failed: %s", strerror(errno));
		return FALSE;
	}
	if (listen(this->fd, 5) == -1)
	{
		DBG1(DBG_TNC, "listen on PT-TLS socket failed: %s", strerror(errno));
		return FALSE;
	}
	return TRUE;
}

/**
 * Handle a single PT-TLS client connection
 */
static job_requeue_t handle(pt_tls_server_t *connection)
{
	while (TRUE)
	{
		switch (connection->handle(connection))
		{
			case NEED_MORE:
				continue;
			case FAILED:
			case SUCCESS:
			default:
				break;
		}
		break;
	}
	return JOB_REQUEUE_NONE;
}

/**
 * Clean up connection state
 */
static void cleanup(pt_tls_server_t *connection)
{
	int fd;

	fd = connection->get_fd(connection);
	connection->destroy(connection);
	close(fd);
}

METHOD(pt_tls_dispatcher_t, dispatch, void,
	private_pt_tls_dispatcher_t *this,
	pt_tls_tnccs_constructor_t *create)
{
	while (TRUE)
	{
		pt_tls_server_t *connection;
		tnccs_t *tnccs;
		bool old;
		int fd;

		old = thread_cancelability(TRUE);
		fd = accept(this->fd, NULL, NULL);
		thread_cancelability(old);
		if (fd == -1)
		{
			DBG1(DBG_TNC, "accepting PT-TLS failed: %s", strerror(errno));
			continue;
		}

		tnccs = create(this->server, this->peer);
		if (!tnccs)
		{
			close(fd);
			continue;
		}
		connection = pt_tls_server_create(this->server, fd, this->auth, tnccs);
		if (!connection)
		{
			close(fd);
			continue;
		}
		lib->processor->queue_job(lib->processor,
				(job_t*)callback_job_create_with_prio((callback_job_cb_t)handle,
										connection, (void*)cleanup,
										(callback_job_cancel_t)return_false,
										JOB_PRIO_CRITICAL));
	}
}

METHOD(pt_tls_dispatcher_t, destroy, void,
	private_pt_tls_dispatcher_t *this)
{
	if (this->fd != -1)
	{
		close(this->fd);
	}
	this->server->destroy(this->server);
	this->peer->destroy(this->peer);
	free(this);
}

/**
 * See header
 */
pt_tls_dispatcher_t *pt_tls_dispatcher_create(host_t *address,
									identification_t *id, pt_tls_auth_t auth)
{
	private_pt_tls_dispatcher_t *this;

	INIT(this,
		.public = {
			.dispatch = _dispatch,
			.destroy = _destroy,
		},
		.server = id->clone(id),
		/* we currently don't authenticate the peer, use %any identity */
		.peer = identification_create_from_encoding(ID_ANY, chunk_empty),
		.fd = -1,
		.auth = auth,
	);

	if (!open_socket(this, address))
	{
		destroy(this);
		return NULL;
	}

	return &this->public;
}
