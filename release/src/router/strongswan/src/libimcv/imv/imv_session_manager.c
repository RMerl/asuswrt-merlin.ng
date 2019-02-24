/*
 * Copyright (C) 2014-2015 Andreas Steffen
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

#include <tncif_names.h>
#include <tncif_identity.h>

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
	linked_list_t *ar_identities)
{
	enumerator_t *enumerator;
	tncif_identity_t *tnc_id;
	imv_session_t *current, *session = NULL;

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
		ar_identities->destroy_offset(ar_identities,
							   offsetof(tncif_identity_t, destroy));
		this->mutex->unlock(this->mutex);
		return session->get_ref(session);
	}

	/* Output list of Access Requestor identities */
	enumerator = ar_identities->create_enumerator(ar_identities);
	while (enumerator->enumerate(enumerator, &tnc_id))
	{
		pen_type_t id_type, subject_type, auth_type;
		uint32_t tcg_id_type, tcg_subject_type, tcg_auth_type;
		chunk_t id_value;

		id_type = tnc_id->get_identity_type(tnc_id);
		id_value = tnc_id->get_identity_value(tnc_id);
		subject_type = tnc_id->get_subject_type(tnc_id);
		auth_type = tnc_id->get_auth_type(tnc_id);

		tcg_id_type = (subject_type.vendor_id == PEN_TCG) ?
							id_type.type : TNC_SUBJECT_UNKNOWN;
		tcg_subject_type = (subject_type.vendor_id == PEN_TCG) ?
							subject_type.type : TNC_SUBJECT_UNKNOWN;
		tcg_auth_type =    (auth_type.vendor_id == PEN_TCG) ?
							auth_type.type : TNC_AUTH_UNKNOWN;

		DBG2(DBG_IMV, "  %N AR identity '%.*s' of type %N authenticated by %N",
			 TNC_Subject_names, tcg_subject_type,
			 id_value.len, id_value.ptr,
			 TNC_Identity_names, tcg_id_type,
			 TNC_Authentication_names, tcg_auth_type);
	}
	enumerator->destroy(enumerator);

	/* create a new session entry */
	session = imv_session_create(conn_id, ar_identities);
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

