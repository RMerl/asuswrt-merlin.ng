/*
 * Copyright (C) 2013 Martin Willi
 * Copyright (C) 2013 revosec AG
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

#include "xpc_channels.h"
#include "xpc_logger.h"

#include <credentials/sets/callback_cred.h>
#include <collections/hashtable.h>
#include <threading/rwlock.h>
#include <daemon.h>

typedef struct private_xpc_channels_t private_xpc_channels_t;

/**
 * Private data of an xpc_channels_t object.
 */
struct private_xpc_channels_t {

	/**
	 * Public xpc_channels_t interface.
	 */
	xpc_channels_t public;

	/**
	 * Registered channels, IKE_SA unique ID => entry_t
	 */
	hashtable_t *channels;

	/**
	 * Lock for channels list
	 */
	rwlock_t *lock;

	/**
	 * Callback credential set for passwords
	 */
	callback_cred_t *creds;
};

/**
 * Channel entry
 */
typedef struct {
	/* XPC channel to App */
	xpc_connection_t conn;
	/* associated IKE_SA unique identifier */
	uintptr_t sa;
	/* did we already ask for a password? */
	bool passworded;
	/* channel specific logger */
	xpc_logger_t *logger;
} entry_t;

/**
 * Clean up an entry, cancelling connection
 */
static void destroy_entry(entry_t *entry)
{
	charon->bus->remove_logger(charon->bus, &entry->logger->logger);
	entry->logger->destroy(entry->logger);
	xpc_connection_suspend(entry->conn);
	xpc_release(entry->conn);
	free(entry);
}

/**
 * Find an IKE_SA unique identifier by a given XPC channel
 */
static uint32_t find_ike_sa_by_conn(private_xpc_channels_t *this,
									 xpc_connection_t conn)
{
	enumerator_t *enumerator;
	entry_t *entry;
	uint32_t ike_sa = 0;

	this->lock->read_lock(this->lock);
	enumerator = this->channels->create_enumerator(this->channels);
	while (enumerator->enumerate(enumerator, NULL, &entry))
	{
		if (entry->conn == conn)
		{
			ike_sa = entry->sa;
			break;
		}
	}
	enumerator->destroy(enumerator);
	this->lock->unlock(this->lock);

	return ike_sa;
}

/**
 * Remove an entry for a given XPC connection
 */
static void remove_conn(private_xpc_channels_t *this, xpc_connection_t conn)
{
	uintptr_t ike_sa;
	entry_t *entry;

	ike_sa = find_ike_sa_by_conn(this, conn);
	if (ike_sa)
	{
		this->lock->write_lock(this->lock);
		entry = this->channels->remove(this->channels, (void*)ike_sa);
		this->lock->unlock(this->lock);

		if (entry)
		{
			destroy_entry(entry);
		}
	}
}

/**
 * Trigger termination of a connection
 */
static void stop_connection(private_xpc_channels_t *this, uint32_t ike_sa,
							xpc_object_t request, xpc_object_t reply)
{
	status_t status;

	status = charon->controller->terminate_ike(charon->controller, ike_sa, FALSE,
											   NULL, NULL, 0);
	xpc_dictionary_set_bool(reply, "success", status != NOT_FOUND);
}

/**
 * XPC RPC command dispatch table
 */
static struct {
	char *name;
	void (*handler)(private_xpc_channels_t *this, uint32_t ike_sa,
					xpc_object_t request, xpc_object_t reply);
} commands[] = {
	{ "stop_connection", stop_connection },
};

/**
 * Handle a request message from App
 */
static void handle(private_xpc_channels_t *this, xpc_connection_t conn,
				   xpc_object_t request)
{
	xpc_object_t reply;
	const char *type, *rpc;
	bool found = FALSE;
	uint32_t ike_sa;
	int i;

	type = xpc_dictionary_get_string(request, "type");
	if (type)
	{
		if (streq(type, "rpc"))
		{
			reply = xpc_dictionary_create_reply(request);
			rpc = xpc_dictionary_get_string(request, "rpc");
			ike_sa = find_ike_sa_by_conn(this, conn);
			if (reply && rpc && ike_sa)
			{
				for (i = 0; i < countof(commands); i++)
				{
					if (streq(commands[i].name, rpc))
					{
						found = TRUE;
						commands[i].handler(this, ike_sa, request, reply);
						break;
					}
				}
			}
			if (!found)
			{
				DBG1(DBG_CFG, "received invalid XPC rpc command: %s", rpc);
			}
			if (reply)
			{
				xpc_connection_send_message(conn, reply);
				xpc_release(reply);
			}
		}
		else
		{
			DBG1(DBG_CFG, "received unknown XPC message type: %s", type);
		}
	}
	else
	{
		DBG1(DBG_CFG, "received XPC message without a type");
	}
}

METHOD(xpc_channels_t, add, void,
	private_xpc_channels_t *this, xpc_connection_t conn, uint32_t ike_sa)
{
	entry_t *entry;

	INIT(entry,
		.conn = conn,
		.sa = ike_sa,
		.logger = xpc_logger_create(conn),
	);

	xpc_retain(entry->conn);
	xpc_connection_set_event_handler(entry->conn, ^(xpc_object_t event)
	{
		if (event == XPC_ERROR_CONNECTION_INVALID ||
			event == XPC_ERROR_CONNECTION_INTERRUPTED)
		{
			remove_conn(this, conn);
		}
		else if (xpc_get_type(event) == XPC_TYPE_DICTIONARY)
		{
			handle(this, conn, event);
		}
	});

	entry->logger->set_ike_sa(entry->logger, entry->sa);
	charon->bus->add_logger(charon->bus, &entry->logger->logger);

	this->lock->write_lock(this->lock);
	this->channels->put(this->channels, (void*)entry->sa, entry);
	this->lock->unlock(this->lock);

	xpc_connection_resume(conn);
}

METHOD(listener_t, alert, bool,
	private_xpc_channels_t *this, ike_sa_t *ike_sa, alert_t alert, va_list args)
{
	const char *desc;

	switch (alert)
	{
		case ALERT_LOCAL_AUTH_FAILED:
			desc = "local-auth";
			break;
		case ALERT_PEER_AUTH_FAILED:
			desc = "remote-auth";
			break;
		case ALERT_PEER_ADDR_FAILED:
			desc = "dns";
			break;
		case ALERT_PEER_INIT_UNREACHABLE:
			desc = "unreachable";
			break;
		case ALERT_RETRANSMIT_SEND_TIMEOUT:
			desc = "timeout";
			break;
		case ALERT_PROPOSAL_MISMATCH_IKE:
		case ALERT_PROPOSAL_MISMATCH_CHILD:
			desc = "proposal-mismatch";
			break;
		case ALERT_TS_MISMATCH:
			desc = "ts-mismatch";
			break;
		default:
			return TRUE;
	}
	if (ike_sa)
	{
		entry_t *entry;
		uintptr_t sa;

		sa = ike_sa->get_unique_id(ike_sa);
		this->lock->read_lock(this->lock);
		entry = this->channels->get(this->channels, (void*)sa);
		if (entry)
		{
			xpc_object_t msg;

			msg = xpc_dictionary_create(NULL, NULL, 0);
			xpc_dictionary_set_string(msg, "type", "event");
			xpc_dictionary_set_string(msg, "event", "alert");
			xpc_dictionary_set_string(msg, "alert", desc);
			xpc_connection_send_message(entry->conn, msg);
			xpc_release(msg);
		}
		this->lock->unlock(this->lock);
	}
	return TRUE;
}

METHOD(listener_t, ike_rekey, bool,
	private_xpc_channels_t *this, ike_sa_t *old, ike_sa_t *new)
{
	entry_t *entry;
	uintptr_t sa;

	sa = old->get_unique_id(old);
	this->lock->write_lock(this->lock);
	entry = this->channels->remove(this->channels, (void*)sa);
	if (entry)
	{
		entry->sa = new->get_unique_id(new);
		entry->logger->set_ike_sa(entry->logger, entry->sa);
		this->channels->put(this->channels, (void*)entry->sa, entry);
	}
	this->lock->unlock(this->lock);

	return TRUE;
}

METHOD(listener_t, ike_state_change, bool,
	private_xpc_channels_t *this, ike_sa_t *ike_sa, ike_sa_state_t state)
{
	if (state == IKE_CONNECTING)
	{
		entry_t *entry;
		uintptr_t sa;

		sa = ike_sa->get_unique_id(ike_sa);
		this->lock->read_lock(this->lock);
		entry = this->channels->get(this->channels, (void*)sa);
		if (entry)
		{
			xpc_object_t msg;

			msg = xpc_dictionary_create(NULL, NULL, 0);
			xpc_dictionary_set_string(msg, "type", "event");
			xpc_dictionary_set_string(msg, "event", "connecting");
			xpc_connection_send_message(entry->conn, msg);
			xpc_release(msg);
		}
		this->lock->unlock(this->lock);
	}
	return TRUE;
}

METHOD(listener_t, child_updown, bool,
	private_xpc_channels_t *this, ike_sa_t *ike_sa,
	child_sa_t *child_sa, bool up)
{
	entry_t *entry;
	uintptr_t sa;

	sa = ike_sa->get_unique_id(ike_sa);
	this->lock->read_lock(this->lock);
	entry = this->channels->get(this->channels, (void*)sa);
	if (entry)
	{
		xpc_object_t msg;
		linked_list_t *list;
		char buf[256];

		msg = xpc_dictionary_create(NULL, NULL, 0);
		xpc_dictionary_set_string(msg, "type", "event");
		if (up)
		{
			xpc_dictionary_set_string(msg, "event", "child_up");
		}
		else
		{
			xpc_dictionary_set_string(msg, "event", "child_down");
		}

		list = linked_list_create_from_enumerator(
					child_sa->create_ts_enumerator(child_sa, TRUE));
		snprintf(buf, sizeof(buf), "%#R", list);
		list->destroy(list);
		xpc_dictionary_set_string(msg, "ts_local", buf);

		list = linked_list_create_from_enumerator(
					child_sa->create_ts_enumerator(child_sa, FALSE));
		snprintf(buf, sizeof(buf), "%#R", list);
		list->destroy(list);
		xpc_dictionary_set_string(msg, "ts_remote", buf);

		xpc_connection_send_message(entry->conn, msg);
		xpc_release(msg);
	}
	this->lock->unlock(this->lock);
	return TRUE;
}

METHOD(listener_t, ike_updown, bool,
	private_xpc_channels_t *this, ike_sa_t *ike_sa, bool up)
{
	xpc_object_t msg;
	entry_t *entry;
	uintptr_t sa;

	sa = ike_sa->get_unique_id(ike_sa);
	if (up)
	{
		this->lock->read_lock(this->lock);
		entry = this->channels->get(this->channels, (void*)sa);
		if (entry)
		{
			msg = xpc_dictionary_create(NULL, NULL, 0);
			xpc_dictionary_set_string(msg, "type", "event");
			xpc_dictionary_set_string(msg, "event", "up");
			xpc_connection_send_message(entry->conn, msg);
			xpc_release(msg);
		}
		this->lock->unlock(this->lock);
	}
	else
	{
		this->lock->write_lock(this->lock);
		entry = this->channels->remove(this->channels, (void*)sa);
		this->lock->unlock(this->lock);
		if (entry)
		{
			msg = xpc_dictionary_create(NULL, NULL, 0);
			xpc_dictionary_set_string(msg, "type", "event");
			xpc_dictionary_set_string(msg, "event", "down");
			xpc_connection_send_message(entry->conn, msg);
			xpc_release(msg);
			xpc_connection_send_barrier(entry->conn, ^() {
				destroy_entry(entry);
			});
		}
	}
	return TRUE;
}

/**
 * Query password from App using XPC channel
 */
static shared_key_t *query_password(xpc_connection_t conn, identification_t *id)
{
	char buf[128], *password;
	xpc_object_t request, response;
	shared_key_t *shared = NULL;

	request = xpc_dictionary_create(NULL, NULL, 0);
	xpc_dictionary_set_string(request, "type", "rpc");
	xpc_dictionary_set_string(request, "rpc", "get_password");
	snprintf(buf, sizeof(buf), "%Y", id);
	xpc_dictionary_set_string(request, "username", buf);

	response = xpc_connection_send_message_with_reply_sync(conn, request);
	xpc_release(request);
	if (xpc_get_type(response) == XPC_TYPE_DICTIONARY)
	{
		password = (char*)xpc_dictionary_get_string(response, "password");
		if (password)
		{
			shared = shared_key_create(SHARED_EAP,
									   chunk_clone(chunk_from_str(password)));
		}
	}
	xpc_release(response);
	return shared;
}

/**
 * Password query callback
 */
static shared_key_t* password_cb(private_xpc_channels_t *this,
								 shared_key_type_t type,
								 identification_t *me, identification_t *other,
								 id_match_t *match_me, id_match_t *match_other)
{
	shared_key_t *shared = NULL;
	ike_sa_t *ike_sa;
	entry_t *entry;
	uint32_t sa;

	switch (type)
	{
		case SHARED_EAP:
			break;
		default:
			return NULL;
	}
	ike_sa = charon->bus->get_sa(charon->bus);
	if (ike_sa)
	{
		sa = ike_sa->get_unique_id(ike_sa);
		this->lock->read_lock(this->lock);
		entry = this->channels->get(this->channels, (void*)(uintptr_t)sa);
		if (entry && !entry->passworded)
		{
			entry->passworded = TRUE;

			shared = query_password(entry->conn, me);
			if (shared)
			{
				if (match_me)
				{
					*match_me = ID_MATCH_PERFECT;
				}
				if (match_other)
				{
					*match_other = ID_MATCH_PERFECT;
				}
			}
		}
		this->lock->unlock(this->lock);
	}
	return shared;
}

METHOD(xpc_channels_t, destroy, void,
	private_xpc_channels_t *this)
{
	lib->credmgr->remove_set(lib->credmgr, &this->creds->set);
	this->creds->destroy(this->creds);
	this->channels->destroy(this->channels);
	this->lock->destroy(this->lock);
	free(this);
}

/**
 * See header
 */
xpc_channels_t *xpc_channels_create()
{
	private_xpc_channels_t *this;

	INIT(this,
		.public = {
			.listener = {
				.alert = _alert,
				.ike_updown = _ike_updown,
				.ike_rekey = _ike_rekey,
				.ike_state_change = _ike_state_change,
				.child_updown = _child_updown,
			},
			.add = _add,
			.destroy = _destroy,
		},
		.channels = hashtable_create(hashtable_hash_ptr,
									 hashtable_equals_ptr, 4),
		.lock = rwlock_create(RWLOCK_TYPE_DEFAULT),
	);

	this->creds = callback_cred_create_shared(
								(callback_cred_shared_cb_t)password_cb, this);
	lib->credmgr->add_set(lib->credmgr, &this->creds->set);

	return &this->public;
}
