/*
 * Copyright (C) 2014 Andreas Steffen
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

#include "imv_session_manager.h"

#include <threading/mutex.h>

typedef struct private_imv_session_manager_t private_imv_session_manager_t;

/**
 * Private data of a imv_session_manager_t object.
 */
struct private_imv_session_manager_t {

	/**
	 * Public imv_session_manager_t interface.
	 */
	imv_session_manager_t public;

	/**
	 * Session list
	 */
	linked_list_t *sessions;

	/**
	 * mutex used to lock session list
	 */
	mutex_t *mutex;

};

METHOD(imv_session_manager_t, add_session, imv_session_t*,
	private_imv_session_manager_t *this, TNC_ConnectionID conn_id,
	uint32_t ar_id_type, chunk_t ar_id_value)
{
	enumerator_t *enumerator;
	imv_session_t *current, *session = NULL;
	time_t created;

	this->mutex->lock(this->mutex);

	/* check if a session has already been assigned */
	enumerator = this->sessions->create_enumerator(this->sessions);
	while (enumerator->enumerate(enumerator, &current))
	{
		if (conn_id == current->get_connection_id(current))
		{
			session = current;
			break;
		}
	}
	enumerator->destroy(enumerator);

	/* session already exists */
	if (session)
	{
		this->mutex->unlock(this->mutex);
		return session->get_ref(session);
	}

	/* create a new session entry */
	created = time(NULL);
	session = imv_session_create(conn_id, created, ar_id_type, ar_id_value);
	this->sessions->insert_last(this->sessions, session);

	this->mutex->unlock(this->mutex);

	return session;
}

METHOD(imv_session_manager_t, remove_session, void,
	private_imv_session_manager_t *this, imv_session_t *session)
{
	enumerator_t *enumerator;
	imv_session_t *current;

	this->mutex->lock(this->mutex);
	enumerator = this->sessions->create_enumerator(this->sessions);
	while (enumerator->enumerate(enumerator, &current))
	{
		if (current == session)
		{
			this->sessions->remove_at(this->sessions, enumerator);
			break;
		}
	}
	enumerator->destroy(enumerator);
	this->mutex->unlock(this->mutex);
}

METHOD(imv_session_manager_t, destroy, void,
	private_imv_session_manager_t *this)
{
	this->sessions->destroy_offset(this->sessions,
							offsetof(imv_session_t, destroy));
	this->mutex->destroy(this->mutex);
	free(this);
}

/**
 * See header
 */
imv_session_manager_t *imv_session_manager_create(void)
{
	private_imv_session_manager_t *this;

	INIT(this,
		.public = {
			.add_session = _add_session,
			.remove_session = _remove_session,
			.destroy = _destroy,
		},
		.sessions = linked_list_create(),
		.mutex = mutex_create(MUTEX_TYPE_DEFAULT),
	);

	return &this->public;
}

