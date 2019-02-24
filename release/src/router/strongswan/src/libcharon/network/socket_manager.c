/*
 * Copyright (C) 2010-2012 Tobias Brunner
 * HSR Hochschule fuer Technik Rapperswil
 * Copyright (C) 2010 Martin Willi
 * Copyright (C) 2010 revosec AG
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

#include "socket_manager.h"

#include <daemon.h>
#include <threading/thread.h>
#include <threading/rwlock.h>
#include <collections/linked_list.h>

typedef struct private_socket_manager_t private_socket_manager_t;

/**
 * Private data of an socket_manager_t object.
 */
struct private_socket_manager_t {

	/**
	 * Public socket_manager_t interface.
	 */
	socket_manager_t public;

	/**
	 * List of registered socket constructors
	 */
	linked_list_t *sockets;

	/**
	 * Instantiated socket implementation
	 */
	socket_t *socket;

	/**
	 * The constructor used to create the current socket
	 */
	socket_constructor_t create;

	/**
	 * Lock for sockets list
	 */
	rwlock_t *lock;
};

METHOD(socket_manager_t, receiver, status_t,
	private_socket_manager_t *this, packet_t **packet)
{
	status_t status;
	this->lock->read_lock(this->lock);
	if (!this->socket)
	{
		DBG1(DBG_NET, "no socket implementation registered, receiving failed");
		this->lock->unlock(this->lock);
		return NOT_SUPPORTED;
	}
	/* receive is blocking and the thread can be cancelled */
	thread_cleanup_push((thread_cleanup_t)this->lock->unlock, this->lock);
	status = this->socket->receive(this->socket, packet);
	thread_cleanup_pop(TRUE);
	return status;
}

METHOD(socket_manager_t, sender, status_t,
	private_socket_manager_t *this, packet_t *packet)
{
	status_t status;
	this->lock->read_lock(this->lock);
	if (!this->socket)
	{
		DBG1(DBG_NET, "no socket implementation registered, sending failed");
		this->lock->unlock(this->lock);
		return NOT_SUPPORTED;
	}
	status = this->socket->send(this->socket, packet);
	this->lock->unlock(this->lock);
	return status;
}

METHOD(socket_manager_t, get_port, uint16_t,
	private_socket_manager_t *this, bool nat_t)
{
	uint16_t port = 0;
	this->lock->read_lock(this->lock);
	if (this->socket)
	{
		port = this->socket->get_port(this->socket, nat_t);
	}
	this->lock->unlock(this->lock);
	return port;
}

METHOD(socket_manager_t, supported_families, socket_family_t,
	private_socket_manager_t *this)
{
	socket_family_t families = SOCKET_FAMILY_NONE;
	this->lock->read_lock(this->lock);
	if (this->socket)
	{
		families = this->socket->supported_families(this->socket);
	}
	this->lock->unlock(this->lock);
	return families;
}

static void create_socket(private_socket_manager_t *this)
{
	socket_constructor_t create;
	/* remove constructors in order to avoid trying to create broken ones
	 * multiple times */
	while (this->sockets->remove_first(this->sockets,
									   (void**)&create) == SUCCESS)
	{
		this->socket = create();
		if (this->socket)
		{
			this->create = create;
			break;
		}
	}
}

METHOD(socket_manager_t, add_socket, void,
	private_socket_manager_t *this, socket_constructor_t create)
{
	this->lock->write_lock(this->lock);
	this->sockets->insert_last(this->sockets, create);
	if (!this->socket)
	{
		create_socket(this);
	}
	this->lock->unlock(this->lock);
}

METHOD(socket_manager_t, remove_socket, void,
	private_socket_manager_t *this, socket_constructor_t create)
{
	this->lock->write_lock(this->lock);
	this->sockets->remove(this->sockets, create, NULL);
	if (this->create == create)
	{
		this->socket->destroy(this->socket);
		this->socket = NULL;
		this->create = NULL;
		create_socket(this);
	}
	this->lock->unlock(this->lock);
}

METHOD(socket_manager_t, destroy, void,
	private_socket_manager_t *this)
{
	DESTROY_IF(this->socket);
	this->sockets->destroy(this->sockets);
	this->lock->destroy(this->lock);
	free(this);
}

/**
 * See header
 */
socket_manager_t *socket_manager_create()
{
	private_socket_manager_t *this;

	INIT(this,
		.public = {
			.send = _sender,
			.receive = _receiver,
			.get_port = _get_port,
			.supported_families = _supported_families,
			.add_socket = _add_socket,
			.remove_socket = _remove_socket,
			.destroy = _destroy,
		},
		.sockets = linked_list_create(),
		.lock = rwlock_create(RWLOCK_TYPE_DEFAULT),
	);

	return &this->public;
}

