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

#include "eap_radius_dae.h"

#include <radius_message.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>

#include <daemon.h>
#include <threading/thread.h>
#include <processing/jobs/callback_job.h>
#include <processing/jobs/delete_ike_sa_job.h>

#define RADIUS_DAE_PORT 3799

typedef struct private_eap_radius_dae_t private_eap_radius_dae_t;

/**
 * Private data of an eap_radius_dae_t object.
 */
struct private_eap_radius_dae_t {

	/**
	 * Public eap_radius_dae_t interface.
	 */
	eap_radius_dae_t public;

	/**
	 * RADIUS session state
	 */
	eap_radius_accounting_t *accounting;

	/**
	 * Socket to listen on authorization extension port
	 */
	int fd;

	/**
	 * RADIUS shared secret for DAE exchanges
	 */
	chunk_t secret;

	/**
	 * MD5 hasher
	 */
	hasher_t *hasher;

	/**
	 * HMAC MD5 signer, with secret set
	 */
	signer_t *signer;

	/**
	 * List of responses for retransmission, as entry_t
	 */
	linked_list_t *responses;
};

/**
 * Entry to store responses for retransmit
 */
typedef struct {
	/** stored response */
	radius_message_t *response;
	/** client that sent the request */
	host_t *client;
} entry_t;

/**
 * Clean up an entry
 */
static void entry_destroy(entry_t *entry)
{
	entry->response->destroy(entry->response);
	entry->client->destroy(entry->client);
	free(entry);
}

/**
 * Save/Replace response for retransmission
 */
static void save_retransmit(private_eap_radius_dae_t *this,
							radius_message_t *response, host_t *client)
{
	enumerator_t *enumerator;
	entry_t *entry;
	bool found = FALSE;

	enumerator = this->responses->create_enumerator(this->responses);
	while (enumerator->enumerate(enumerator, &entry))
	{
		if (client->equals(client, entry->client))
		{
			entry->response->destroy(entry->response);
			entry->response = response;
			found = TRUE;
			break;
		}
	}
	enumerator->destroy(enumerator);

	if (!found)
	{
		INIT(entry,
			.response = response,
			.client = client->clone(client),
		);
		this->responses->insert_first(this->responses, entry);
	}
}

/**
 * Send a RADIUS message to client
 */
static void send_message(private_eap_radius_dae_t *this,
						 radius_message_t *message, host_t *client)
{
	chunk_t data;

	data = message->get_encoding(message);
	if (sendto(this->fd, data.ptr, data.len, 0, client->get_sockaddr(client),
			   *client->get_sockaddr_len(client)) != data.len)
	{
		DBG1(DBG_CFG, "sending RADIUS DAE response failed: %s", strerror(errno));
	}
}

/**
 * Check if we request is a retransmit, retransmit stored response
 */
static bool send_retransmit(private_eap_radius_dae_t *this,
							radius_message_t *request, host_t *client)
{
	enumerator_t *enumerator;
	entry_t *entry;
	bool found = FALSE;

	enumerator = this->responses->create_enumerator(this->responses);
	while (enumerator->enumerate(enumerator, &entry))
	{
		if (client->equals(client, entry->client) &&
			request->get_identifier(request) ==
							entry->response->get_identifier(entry->response))
		{
			DBG1(DBG_CFG, "received retransmit of RADIUS %N, retransmitting %N "
				 "to %H", radius_message_code_names, request->get_code(request),
				 radius_message_code_names,
				 entry->response->get_code(entry->response), client);
			send_message(this, entry->response, client);
			found = TRUE;
			break;
		}
	}
	enumerator->destroy(enumerator);

	return found;
}

/**
 * Send an ACK/NAK response for a request
 */
static void send_response(private_eap_radius_dae_t *this,
						  radius_message_t *request, radius_message_code_t code,
						  host_t *client)
{
	radius_message_t *response;

	response = radius_message_create(code);
	response->set_identifier(response, request->get_identifier(request));
	if (response->sign(response, request->get_authenticator(request),
					   this->secret, this->hasher, this->signer, NULL, FALSE))
	{
		send_message(this, response, client);
		save_retransmit(this, response, client);
	}
	else
	{
		response->destroy(response);
	}
}

/**
 * Add all IKE_SAs matching to user to a list
 */
static void add_matching_ike_sas(linked_list_t *list, identification_t *user)
{
	enumerator_t *enumerator;
	ike_sa_t *ike_sa;
	ike_sa_id_t *id;

	enumerator = charon->ike_sa_manager->create_enumerator(
												charon->ike_sa_manager, FALSE);
	while (enumerator->enumerate(enumerator, &ike_sa))
	{
		if (user->matches(user, ike_sa->get_other_eap_id(ike_sa)))
		{
			id = ike_sa->get_id(ike_sa);
			list->insert_last(list, id->clone(id));
		}
	}
	enumerator->destroy(enumerator);
}

/**
 * Get list of IKE_SAs matching a Disconnect/CoA request
 */
static linked_list_t *get_matching_ike_sas(private_eap_radius_dae_t *this,
									radius_message_t *request, host_t *client)
{
	enumerator_t *enumerator;
	identification_t *user;
	linked_list_t *ids;
	chunk_t data;
	int type;

	ids = linked_list_create();

	enumerator = request->create_enumerator(request);
	while (enumerator->enumerate(enumerator, &type, &data))
	{
		if (type == RAT_USER_NAME && data.len)
		{
			user = identification_create_from_data(data);
			DBG1(DBG_CFG, "received RADIUS DAE %N for %Y from %H",
				 radius_message_code_names, request->get_code(request),
				 user, client);
			add_matching_ike_sas(ids, user);
			user->destroy(user);
		}
	}
	enumerator->destroy(enumerator);

	return ids;
}

/**
 * Process a DAE disconnect request, send response
 */
static void process_disconnect(private_eap_radius_dae_t *this,
							   radius_message_t *request, host_t *client)
{
	enumerator_t *enumerator;
	linked_list_t *ids;
	ike_sa_id_t *id;

	ids = get_matching_ike_sas(this, request, client);

	if (ids->get_count(ids))
	{
		DBG1(DBG_CFG, "closing %d IKE_SA%s matching %N, sending %N",
			 ids->get_count(ids), ids->get_count(ids) > 1 ? "s" : "",
			 radius_message_code_names, RMC_DISCONNECT_REQUEST,
			 radius_message_code_names, RMC_DISCONNECT_ACK);

		enumerator = ids->create_enumerator(ids);
		while (enumerator->enumerate(enumerator, &id))
		{
			lib->processor->queue_job(lib->processor, (job_t*)
									  delete_ike_sa_job_create(id, TRUE));
		}
		enumerator->destroy(enumerator);

		send_response(this, request, RMC_DISCONNECT_ACK, client);
	}
	else
	{
		DBG1(DBG_CFG, "no IKE_SA matches %N, sending %N",
			 radius_message_code_names, RMC_DISCONNECT_REQUEST,
			 radius_message_code_names, RMC_DISCONNECT_NAK);
		send_response(this, request, RMC_DISCONNECT_NAK, client);
	}
	ids->destroy_offset(ids, offsetof(ike_sa_id_t, destroy));
}

/**
 * Apply a new lifetime to an IKE_SA
 */
static void apply_lifetime(private_eap_radius_dae_t *this, ike_sa_id_t *id,
						   uint32_t lifetime)
{
	ike_sa_t *ike_sa;

	ike_sa = charon->ike_sa_manager->checkout(charon->ike_sa_manager, id);
	if (ike_sa)
	{
		if (ike_sa->set_auth_lifetime(ike_sa, lifetime) == DESTROY_ME)
		{
			charon->ike_sa_manager->checkin_and_destroy(charon->ike_sa_manager,
														ike_sa);
		}
		else
		{
			charon->ike_sa_manager->checkin(charon->ike_sa_manager, ike_sa);
		}
	}
}

/**
 * Process a DAE CoA request, send response
 */
static void process_coa(private_eap_radius_dae_t *this,
						radius_message_t *request, host_t *client)
{
	enumerator_t *enumerator;
	linked_list_t *ids;
	ike_sa_id_t *id;
	chunk_t data;
	int type;
	uint32_t lifetime = 0;
	bool lifetime_seen = FALSE;

	ids = get_matching_ike_sas(this, request, client);

	if (ids->get_count(ids))
	{
		enumerator = request->create_enumerator(request);
		while (enumerator->enumerate(enumerator, &type, &data))
		{
			if (type == RAT_SESSION_TIMEOUT && data.len == 4)
			{
				lifetime = untoh32(data.ptr);
				lifetime_seen = TRUE;
				break;
			}
		}
		enumerator->destroy(enumerator);

		if (lifetime_seen)
		{
			DBG1(DBG_CFG, "applying %us lifetime to %d IKE_SA%s matching %N, "
				 "sending %N", lifetime, ids->get_count(ids),
				 ids->get_count(ids) > 1 ? "s" : "",
				 radius_message_code_names, RMC_COA_REQUEST,
				 radius_message_code_names, RMC_COA_ACK);

			enumerator = ids->create_enumerator(ids);
			while (enumerator->enumerate(enumerator, &id))
			{
				apply_lifetime(this, id, lifetime);
			}
			enumerator->destroy(enumerator);
			send_response(this, request, RMC_COA_ACK, client);
		}
		else
		{
			DBG1(DBG_CFG, "no Session-Timeout attribute found in %N, sending %N",
				 radius_message_code_names, RMC_COA_REQUEST,
				 radius_message_code_names, RMC_COA_NAK);
			send_response(this, request, RMC_COA_NAK, client);
		}
	}
	else
	{
		DBG1(DBG_CFG, "no IKE_SA matches %N, sending %N",
			 radius_message_code_names, RMC_COA_REQUEST,
			 radius_message_code_names, RMC_COA_NAK);
		send_response(this, request, RMC_COA_NAK, client);
	}
	ids->destroy_offset(ids, offsetof(ike_sa_id_t, destroy));
}

/**
 * Receive RADIUS DAE requests
 */
static bool receive(private_eap_radius_dae_t *this)
{
	struct sockaddr_storage addr;
	socklen_t addr_len = sizeof(addr);
	radius_message_t *request;
	char buf[2048];
	ssize_t len;
	host_t *client;

	len = recvfrom(this->fd, buf, sizeof(buf), MSG_DONTWAIT,
				   (struct sockaddr*)&addr, &addr_len);
	if (len > 0)
	{
		request = radius_message_parse(chunk_create(buf, len));
		if (request)
		{
			client = host_create_from_sockaddr((struct sockaddr*)&addr);
			if (client)
			{
				if (!send_retransmit(this, request, client))
				{
					if (request->verify(request, NULL, this->secret,
										this->hasher, this->signer))
					{
						switch (request->get_code(request))
						{
							case RMC_DISCONNECT_REQUEST:
								process_disconnect(this, request, client);
								break;
							case RMC_COA_REQUEST:
								process_coa(this, request, client);
								break;
							default:
								DBG1(DBG_CFG, "ignoring unsupported RADIUS DAE "
									 "%N message from %H",
									 radius_message_code_names,
									 request->get_code(request), client);
							break;
						}
					}
				}
				client->destroy(client);
			}
			request->destroy(request);
		}
		else
		{
			DBG1(DBG_NET, "ignoring invalid RADIUS DAE request");
		}
	}
	else if (errno != EWOULDBLOCK)
	{
		DBG1(DBG_NET, "receiving RADIUS DAE request failed: %s", strerror(errno));
	}
	return TRUE;
}

/**
 * Open DAE socket
 */
static bool open_socket(private_eap_radius_dae_t *this)
{
	host_t *host;

	this->fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (this->fd == -1)
	{
		DBG1(DBG_CFG, "unable to open RADIUS DAE socket: %s", strerror(errno));
		return FALSE;
	}

	host = host_create_from_string(
				lib->settings->get_str(lib->settings,
							"%s.plugins.eap-radius.dae.listen", "0.0.0.0",
							lib->ns),
				lib->settings->get_int(lib->settings,
							"%s.plugins.eap-radius.dae.port", RADIUS_DAE_PORT,
							lib->ns));
	if (!host)
	{
		DBG1(DBG_CFG, "invalid RADIUS DAE listen address");
		return FALSE;
	}

	if (bind(this->fd, host->get_sockaddr(host),
			 *host->get_sockaddr_len(host)) == -1)
	{
		DBG1(DBG_CFG, "unable to bind RADIUS DAE socket: %s", strerror(errno));
		host->destroy(host);
		return FALSE;
	}
	host->destroy(host);
	return TRUE;
}

METHOD(eap_radius_dae_t, destroy, void,
	private_eap_radius_dae_t *this)
{
	if (this->fd != -1)
	{
		lib->watcher->remove(lib->watcher, this->fd);
		close(this->fd);
	}
	DESTROY_IF(this->signer);
	DESTROY_IF(this->hasher);
	this->responses->destroy_function(this->responses, (void*)entry_destroy);
	free(this);
}

/**
 * See header
 */
eap_radius_dae_t *eap_radius_dae_create(eap_radius_accounting_t *accounting)
{
	private_eap_radius_dae_t *this;

	INIT(this,
		.public = {
			.destroy = _destroy,
		},
		.accounting = accounting,
		.fd = -1,
		.secret = {
			.ptr = lib->settings->get_str(lib->settings,
									"%s.plugins.eap-radius.dae.secret", NULL,
									lib->ns),
		},
		.hasher = lib->crypto->create_hasher(lib->crypto, HASH_MD5),
		.signer = lib->crypto->create_signer(lib->crypto, AUTH_HMAC_MD5_128),
		.responses = linked_list_create(),
	);

	if (!this->hasher || !this->signer)
	{
		destroy(this);
		return NULL;
	}
	if (!this->secret.ptr)
	{
		DBG1(DBG_CFG, "missing RADIUS DAE secret, disabled");
		destroy(this);
		return NULL;
	}
	this->secret.len = strlen(this->secret.ptr);
	if (!this->signer->set_key(this->signer, this->secret) ||
		!open_socket(this))
	{
		destroy(this);
		return NULL;
	}

	lib->watcher->add(lib->watcher, this->fd, WATCHER_READ,
					  (watcher_cb_t)receive, this);

	return &this->public;
}
