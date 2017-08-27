/*
 * Copyright (C) 2013 Andreas Steffen
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

#include "imv_session.h"

#include <utils/debug.h>

typedef struct private_imv_session_t private_imv_session_t;

/**
 * Private data of a imv_session_t object.
 */
struct private_imv_session_t {

	/**
	 * Public imv_session_t interface.
	 */
	imv_session_t public;

	/**
	 * Unique Session ID
	 */
	int session_id;

	/**
	 * Unique Product ID
	 */
	int pid;

	/**
	 * Unique Device ID
	 */
	int did;

	/**
	 * TNCCS connection ID
	 */
	TNC_ConnectionID conn_id;

	/**
	 * Session creation time
	 */
	time_t created;

	/**
	 * Access Requestor ID type
	 */
	uint32_t ar_id_type;

	/**
	 * Access Requestor ID value
	 */
	chunk_t ar_id_value;

	/**
	 * OS information
	 */
	imv_os_info_t *os_info;

	/**
	 * Device ID
	 */
	chunk_t device_id;

	/**
	 * Is Device ID trusted?
	 */
	bool trusted;

	/**
	 * Have the workitems been generated?
	 */
	bool policy_started;

	/**
	 * List of worklist items
	 */
	linked_list_t *workitems;

	/**
	 * Reference count
	 */
	refcount_t ref;

};

METHOD(imv_session_t, set_session_id, void,
	private_imv_session_t *this, int session_id, int pid, int did)
{
	this->session_id = session_id;
	this->pid = pid;
	this->did = did;
}

METHOD(imv_session_t, get_session_id, int,
	private_imv_session_t *this, int *pid, int *did)
{
	if (pid)
	{
		*pid = this->pid;
	}
	if (did)
	{
		*did = this->did;
	}
	return this->session_id;
}

METHOD(imv_session_t, get_connection_id, TNC_ConnectionID,
	private_imv_session_t *this)
{
	return this->conn_id;
}

METHOD(imv_session_t, get_creation_time, time_t,
	private_imv_session_t *this)
{
	return this->created;
}

METHOD(imv_session_t, get_ar_id, chunk_t,
	private_imv_session_t *this, uint32_t *ar_id_type)
{
	if (ar_id_type)
	{
		*ar_id_type = this->ar_id_type;
	}
	return this->ar_id_value;
}

METHOD(imv_session_t, get_os_info, imv_os_info_t*,
	private_imv_session_t *this)
{
	return this->os_info;
}

METHOD(imv_session_t, set_device_id, void,
	private_imv_session_t *this, chunk_t device_id)
{
	if (device_id.len == 0)
	{
		device_id = chunk_from_str("unknown");
	}
	if (this->device_id.len)
	{
		if (chunk_equals(device_id, this->device_id))
		{
			return;
		}
		free(this->device_id.ptr);
	}
	this->device_id = chunk_clone(device_id);
}

METHOD(imv_session_t, get_device_id, bool,
	private_imv_session_t *this, chunk_t *device_id)
{
	if (this->device_id.len == 0)
	{
		return FALSE;
	}
	if (device_id)
	{
		*device_id = this->device_id;
	}
	return TRUE;
}

METHOD(imv_session_t, set_device_trust, void,
	private_imv_session_t *this, bool trusted)
{
	this->trusted = trusted;
}

METHOD(imv_session_t, get_device_trust, bool,
	private_imv_session_t *this)
{
	return this->trusted;
}

METHOD(imv_session_t, set_policy_started, void,
	private_imv_session_t *this, bool start)
{
	this->policy_started = start;
}

METHOD(imv_session_t, get_policy_started, bool,
	private_imv_session_t *this)
{
	return this->policy_started;
}

METHOD(imv_session_t, insert_workitem, void,
	private_imv_session_t *this, imv_workitem_t *workitem)
{
	this->workitems->insert_last(this->workitems, workitem);
}

METHOD(imv_session_t, remove_workitem, void,
	private_imv_session_t *this, enumerator_t *enumerator)
{
	this->workitems->remove_at(this->workitems, enumerator);
}

METHOD(imv_session_t, create_workitem_enumerator, enumerator_t*,
	private_imv_session_t *this)
{
	return this->workitems->create_enumerator(this->workitems);
}

METHOD(imv_session_t, get_workitem_count, int,
	private_imv_session_t *this, TNC_IMVID imv_id)
{
	enumerator_t *enumerator;
	imv_workitem_t *workitem;
	int count = 0;

	enumerator = this->workitems->create_enumerator(this->workitems);
	while (enumerator->enumerate(enumerator, &workitem))
	{
		if (workitem->get_imv_id(workitem) == imv_id)
		{
			count++;
		}
	}
	enumerator->destroy(enumerator);

	return count;
}

METHOD(imv_session_t, get_ref, imv_session_t*,
	private_imv_session_t *this)
{
	ref_get(&this->ref);

	return &this->public;
}

METHOD(imv_session_t, destroy, void,
	private_imv_session_t *this)
{
	if (ref_put(&this->ref))
	{
		this->workitems->destroy_offset(this->workitems,
								 offsetof(imv_workitem_t, destroy));
		this->os_info->destroy(this->os_info);
		free(this->ar_id_value.ptr);
		free(this->device_id.ptr);
		free(this);
	}
}

/**
 * See header
 */
imv_session_t *imv_session_create(TNC_ConnectionID conn_id, time_t created,
								  uint32_t ar_id_type, chunk_t ar_id_value)
{
	private_imv_session_t *this;

	INIT(this,
		.public = {
			.set_session_id = _set_session_id,
			.get_session_id = _get_session_id,
			.get_connection_id = _get_connection_id,
			.get_creation_time = _get_creation_time,
			.get_ar_id = _get_ar_id,
			.get_os_info = _get_os_info,
			.set_device_id = _set_device_id,
			.get_device_id = _get_device_id,
			.set_device_trust = _set_device_trust,
			.get_device_trust = _get_device_trust,
			.set_policy_started = _set_policy_started,
			.get_policy_started = _get_policy_started,
			.insert_workitem = _insert_workitem,
			.remove_workitem = _remove_workitem,
			.create_workitem_enumerator = _create_workitem_enumerator,
			.get_workitem_count = _get_workitem_count,
			.get_ref = _get_ref,
			.destroy = _destroy,
		},
		.conn_id = conn_id,
		.created = created,
		.ar_id_type = ar_id_type,
		.ar_id_value = chunk_clone(ar_id_value),
		.os_info = imv_os_info_create(),
		.workitems = linked_list_create(),
		.ref = 1,
	);

	return &this->public;
}
