/*
 * Copyright (C) 2007-2008 Tobias Brunner
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

#include "ike_me.h"

#include <string.h>

#include <daemon.h>
#include <config/peer_cfg.h>
#include <encoding/payloads/id_payload.h>
#include <encoding/payloads/notify_payload.h>
#include <encoding/payloads/endpoint_notify.h>
#include <processing/jobs/mediation_job.h>

#define ME_CONNECTID_LEN 4
#define ME_CONNECTKEY_LEN 16

typedef struct private_ike_me_t private_ike_me_t;

/**
 * Private members of a ike_me_t task.
 */
struct private_ike_me_t {

	/**
	 * Public methods and task_t interface.
	 */
	ike_me_t public;

	/**
	 * Assigned IKE_SA.
	 */
	ike_sa_t *ike_sa;

	/**
	 * Are we the initiator?
	 */
	bool initiator;

	/**
	 * Is this a mediation connection?
	 */
	bool mediation;

	/**
	 * Is this the response from another peer?
	 */
	bool response;

	/**
	 * Gathered endpoints
	 */
	linked_list_t *local_endpoints;

	/**
	 * Parsed endpoints
	 */
	linked_list_t *remote_endpoints;

	/**
	 * Did the peer request a callback?
	 */
	bool callback;

	/**
	 * Did the connect fail?
	 */
	bool failed;

	/**
	 * Was there anything wrong with the payloads?
	 */
	bool invalid_syntax;

	/**
	 * The requested peer
	 */
	identification_t *peer_id;
	/**
	 * Received ID used for connectivity checks
	 */
	chunk_t connect_id;

	/**
	 * Received key used for connectivity checks
	 */
	chunk_t connect_key;

	/**
	 * Peer config of the mediated connection
	 */
	peer_cfg_t *mediated_cfg;

};

/**
 * Adds a list of endpoints as notifies to a given message
 */
static void add_endpoints_to_message(message_t *message, linked_list_t *endpoints)
{
	enumerator_t *enumerator;
	endpoint_notify_t *endpoint;

	enumerator = endpoints->create_enumerator(endpoints);
	while (enumerator->enumerate(enumerator, (void**)&endpoint))
	{
		message->add_payload(message, (payload_t*)endpoint->build_notify(endpoint));
	}
	enumerator->destroy(enumerator);
}

/**
 * Gathers endpoints and adds them to the current message
 */
static void gather_and_add_endpoints(private_ike_me_t *this, message_t *message)
{
	enumerator_t *enumerator;
	host_t *addr, *host;
	uint16_t port;

	/* get the port that is used to communicate with the ms */
	host = this->ike_sa->get_my_host(this->ike_sa);
	port = host->get_port(host);

	enumerator = charon->kernel->create_address_enumerator(charon->kernel,
														   ADDR_TYPE_REGULAR);
	while (enumerator->enumerate(enumerator, (void**)&addr))
	{
		host = addr->clone(addr);
		host->set_port(host, port);

		this->local_endpoints->insert_last(this->local_endpoints,
				endpoint_notify_create_from_host(HOST, host, NULL));

		host->destroy(host);
	}
	enumerator->destroy(enumerator);

	host = this->ike_sa->get_server_reflexive_host(this->ike_sa);
	if (host)
	{
		this->local_endpoints->insert_last(this->local_endpoints,
				endpoint_notify_create_from_host(SERVER_REFLEXIVE, host,
						this->ike_sa->get_my_host(this->ike_sa)));
	}

	add_endpoints_to_message(message, this->local_endpoints);
}

/**
 * read notifys from message and evaluate them
 */
static void process_payloads(private_ike_me_t *this, message_t *message)
{
	enumerator_t *enumerator;
	payload_t *payload;

	enumerator = message->create_payload_enumerator(message);
	while (enumerator->enumerate(enumerator, &payload))
	{
		if (payload->get_type(payload) != PLV2_NOTIFY)
		{
			continue;
		}

		notify_payload_t *notify = (notify_payload_t*)payload;

		switch (notify->get_notify_type(notify))
		{
			case ME_CONNECT_FAILED:
			{
				DBG2(DBG_IKE, "received ME_CONNECT_FAILED notify");
				this->failed = TRUE;
				break;
			}
			case ME_MEDIATION:
			{
				DBG2(DBG_IKE, "received ME_MEDIATION notify");
				this->mediation = TRUE;
				break;
			}
			case ME_ENDPOINT:
			{
				endpoint_notify_t *endpoint;
				endpoint = endpoint_notify_create_from_payload(notify);
				if (!endpoint)
				{
					DBG1(DBG_IKE, "received invalid ME_ENDPOINT notify");
					break;
				}
				DBG1(DBG_IKE, "received %N ME_ENDPOINT %#H",
					 me_endpoint_type_names, endpoint->get_type(endpoint),
					 endpoint->get_host(endpoint));

				this->remote_endpoints->insert_last(this->remote_endpoints,
													endpoint);
				break;
			}
			case ME_CALLBACK:
			{
				DBG2(DBG_IKE, "received ME_CALLBACK notify");
				this->callback = TRUE;
				break;
			}
			case ME_CONNECTID:
			{
				chunk_free(&this->connect_id);
				this->connect_id = chunk_clone(notify->get_notification_data(notify));
				DBG2(DBG_IKE, "received ME_CONNECTID %#B", &this->connect_id);
				break;
			}
			case ME_CONNECTKEY:
			{
				chunk_free(&this->connect_key);
				this->connect_key = chunk_clone(notify->get_notification_data(notify));
				DBG4(DBG_IKE, "received ME_CONNECTKEY %#B", &this->connect_key);
				break;
			}
			case ME_RESPONSE:
			{
				DBG2(DBG_IKE, "received ME_RESPONSE notify");
				this->response = TRUE;
				break;
			}
			default:
				break;
		}
	}
	enumerator->destroy(enumerator);
}

METHOD(task_t, build_i, status_t,
	private_ike_me_t *this, message_t *message)
{
	switch(message->get_exchange_type(message))
	{
		case IKE_SA_INIT:
		{
			peer_cfg_t *peer_cfg = this->ike_sa->get_peer_cfg(this->ike_sa);
			if (peer_cfg->is_mediation(peer_cfg))
			{
				DBG2(DBG_IKE, "adding ME_MEDIATION");
				message->add_notify(message, FALSE, ME_MEDIATION, chunk_empty);
			}
			else
			{
				return SUCCESS;
			}
			break;
		}
		case IKE_AUTH:
		{
			if (this->ike_sa->has_condition(this->ike_sa, COND_NAT_HERE))
			{
				endpoint_notify_t *endpoint;
				endpoint = endpoint_notify_create_from_host(SERVER_REFLEXIVE,
															NULL, NULL);
				message->add_payload(message, (payload_t*)endpoint->build_notify(endpoint));
				endpoint->destroy(endpoint);
			}
			break;
		}
		case ME_CONNECT:
		{
			rng_t *rng;
			id_payload_t *id_payload;
			id_payload = id_payload_create_from_identification(PLV2_ID_PEER,
															   this->peer_id);
			message->add_payload(message, (payload_t*)id_payload);

			rng = lib->crypto->create_rng(lib->crypto, RNG_STRONG);
			if (!rng)
			{
				DBG1(DBG_IKE, "unable to generate connect ID for ME_CONNECT");
				return FAILED;
			}
			if (!this->response)
			{
				/* only the initiator creates a connect ID. the responder
				 * returns the connect ID that it received from the initiator */
				if (!rng->allocate_bytes(rng, ME_CONNECTID_LEN,
										 &this->connect_id))
				{
					DBG1(DBG_IKE, "unable to generate ID for ME_CONNECT");
					rng->destroy(rng);
					return FAILED;
				}
			}
			if (!rng->allocate_bytes(rng, ME_CONNECTKEY_LEN,
									 &this->connect_key))
			{
				DBG1(DBG_IKE, "unable to generate connect key for ME_CONNECT");
				rng->destroy(rng);
				return FAILED;
			}
			rng->destroy(rng);

			message->add_notify(message, FALSE, ME_CONNECTID, this->connect_id);
			message->add_notify(message, FALSE, ME_CONNECTKEY, this->connect_key);

			if (this->response)
			{
				message->add_notify(message, FALSE, ME_RESPONSE, chunk_empty);
			}
			else
			{
				/* FIXME: should we make this configurable? */
				message->add_notify(message, FALSE, ME_CALLBACK, chunk_empty);
			}

			gather_and_add_endpoints(this, message);

			break;
		}
		default:
			break;
	}
	return NEED_MORE;
}

METHOD(task_t, process_r, status_t,
	private_ike_me_t *this, message_t *message)
{
	switch(message->get_exchange_type(message))
	{
		case ME_CONNECT:
		{
			id_payload_t *id_payload;
			id_payload = (id_payload_t*)message->get_payload(message, PLV2_ID_PEER);
			if (!id_payload)
			{
				DBG1(DBG_IKE, "received ME_CONNECT without ID_PEER payload"
					 ", aborting");
				break;
			}
			this->peer_id = id_payload->get_identification(id_payload);

			process_payloads(this, message);

			if (this->callback)
			{
				DBG1(DBG_IKE, "received ME_CALLBACK for '%Y'", this->peer_id);
				break;
			}

			if (!this->connect_id.ptr)
			{
				DBG1(DBG_IKE, "received ME_CONNECT without ME_CONNECTID notify"
					 ", aborting");
				this->invalid_syntax = TRUE;
				break;
			}

			if (!this->connect_key.ptr)
			{
				DBG1(DBG_IKE, "received ME_CONNECT without ME_CONNECTKEY "
					 "notify, aborting");
				this->invalid_syntax = TRUE;
				break;
			}

			if (!this->remote_endpoints->get_count(this->remote_endpoints))
			{
				DBG1(DBG_IKE, "received ME_CONNECT without any ME_ENDPOINT "
					 "payloads, aborting");
				this->invalid_syntax = TRUE;
				break;
			}

			DBG1(DBG_IKE, "received ME_CONNECT");
			break;
		}
		default:
			break;
	}
	return NEED_MORE;
}

METHOD(task_t, build_r, status_t,
	private_ike_me_t *this, message_t *message)
{
	switch(message->get_exchange_type(message))
	{
		case ME_CONNECT:
		{
			if (this->invalid_syntax)
			{
				message->add_notify(message, TRUE, INVALID_SYNTAX, chunk_empty);
				break;
			}

			if (this->callback)
			{
				/* we got a callback from the mediation server, initiate the
				 * queued mediated connecction */
				charon->connect_manager->check_and_initiate(
						charon->connect_manager,
						this->ike_sa->get_id(this->ike_sa),
						this->ike_sa->get_my_id(this->ike_sa), this->peer_id);
				return SUCCESS;
			}

			if (this->response)
			{
				/* FIXME: handle result of set_responder_data
				 * as initiator, upon receiving a response from another peer,
				 * update the checklist and start sending checks */
				charon->connect_manager->set_responder_data(
						charon->connect_manager,
						this->connect_id, this->connect_key,
						this->remote_endpoints);
			}
			else
			{
				/* FIXME: handle result of set_initiator_data
				 * as responder, create a checklist with the initiator's data */
				charon->connect_manager->set_initiator_data(
						charon->connect_manager,
						this->peer_id, this->ike_sa->get_my_id(this->ike_sa),
						this->connect_id, this->connect_key,
						this->remote_endpoints, FALSE);
				if (this->ike_sa->respond(this->ike_sa, this->peer_id,
										  this->connect_id) != SUCCESS)
				{
					return FAILED;
				}
			}
			break;
		}
		default:
			break;
	}
	return SUCCESS;
}

METHOD(task_t, process_i, status_t,
	private_ike_me_t *this, message_t *message)
{
	switch(message->get_exchange_type(message))
	{
		case IKE_SA_INIT:
		{
			process_payloads(this, message);
			if (!this->mediation)
			{
				DBG1(DBG_IKE, "server did not return a ME_MEDIATION, aborting");
				return FAILED;
			}
			/* if we are on a mediation connection we switch to port 4500 even
			 * if no NAT is detected. */
			this->ike_sa->float_ports(this->ike_sa);
			return NEED_MORE;
		}
		case IKE_AUTH:
		{
			process_payloads(this, message);
			/* FIXME: we should update the server reflexive endpoint somehow,
			 * if mobike notices a change */
			endpoint_notify_t *reflexive;
			if (this->remote_endpoints->get_first(this->remote_endpoints,
											(void**)&reflexive) == SUCCESS &&
				reflexive->get_type(reflexive) == SERVER_REFLEXIVE)
			{	/* FIXME: should we accept this endpoint even if we did not send
				 * a request? */
				host_t *endpoint = reflexive->get_host(reflexive);
				endpoint = endpoint->clone(endpoint);
				this->ike_sa->set_server_reflexive_host(this->ike_sa, endpoint);
			}
			break;
		}
		case ME_CONNECT:
		{
			process_payloads(this, message);

			if (this->failed)
			{
				DBG1(DBG_IKE, "peer '%Y' is not online", this->peer_id);
				/* FIXME: notify the mediated connection (job?) */
			}
			else
			{
				if (this->response)
				{
					/* FIXME: handle result of set_responder_data. */
					/* as responder, we update the checklist and start sending
					 * checks */
					charon->connect_manager->set_responder_data(
							charon->connect_manager, this->connect_id,
							this->connect_key, this->local_endpoints);
				}
				else
				{
					/* FIXME: handle result of set_initiator_data */
					/* as initiator, we create a checklist and set the
					 * initiator's data */
					charon->connect_manager->set_initiator_data(
							charon->connect_manager,
							this->ike_sa->get_my_id(this->ike_sa),
							this->peer_id, this->connect_id, this->connect_key,
							this->local_endpoints, TRUE);
					/* FIXME: also start a timer for the whole transaction
					 * (maybe within the connect_manager?) */
				}
			}
			break;
		}
		default:
			break;
	}
	return SUCCESS;
}

/**
 *  For mediation server
 */
METHOD(task_t, build_i_ms, status_t,
	private_ike_me_t *this, message_t *message)
{
	switch(message->get_exchange_type(message))
	{
		case ME_CONNECT:
		{
			id_payload_t *id_payload;
			id_payload = id_payload_create_from_identification(PLV2_ID_PEER,
															   this->peer_id);
			message->add_payload(message, (payload_t*)id_payload);

			if (this->callback)
			{
				message->add_notify(message, FALSE, ME_CALLBACK, chunk_empty);
			}
			else
			{
				if (this->response)
				{
					message->add_notify(message, FALSE, ME_RESPONSE,
										chunk_empty);
				}
				message->add_notify(message, FALSE, ME_CONNECTID,
									this->connect_id);
				message->add_notify(message, FALSE, ME_CONNECTKEY,
									this->connect_key);
				add_endpoints_to_message(message, this->remote_endpoints);
			}
			break;
		}
		default:
			break;
	}
	return NEED_MORE;
}

/**
 * For mediation server
 */
METHOD(task_t, process_r_ms, status_t,
	private_ike_me_t *this, message_t *message)
{
	switch(message->get_exchange_type(message))
	{
		case IKE_SA_INIT:
		{
			/* FIXME: we should check for SA* and TS* payloads. if there are
			 * any, send NO_ADDITIONAL_SAS back and delete this SA */
			process_payloads(this, message);
			return this->mediation ? NEED_MORE : SUCCESS;
		}
		case IKE_AUTH:
		{
			/* FIXME: we should check whether the current peer_config is
			 * configured as mediation connection */
			process_payloads(this, message);
			break;
		}
		case CREATE_CHILD_SA:
		{
			/* FIXME: if this is not to rekey the IKE SA we have to return a
			 * NO_ADDITIONAL_SAS and then delete the SA */
			break;
		}
		case ME_CONNECT:
		{
			id_payload_t *id_payload;
			id_payload = (id_payload_t*)message->get_payload(message, PLV2_ID_PEER);
			if (!id_payload)
			{
				DBG1(DBG_IKE, "received ME_CONNECT without ID_PEER payload"
					 ", aborting");
				this->invalid_syntax = TRUE;
				break;
			}
			this->peer_id = id_payload->get_identification(id_payload);

			process_payloads(this, message);

			if (!this->connect_id.ptr)
			{
				DBG1(DBG_IKE, "received ME_CONNECT without ME_CONNECTID notify"
					 ", aborting");
				this->invalid_syntax = TRUE;
				break;
			}

			if (!this->connect_key.ptr)
			{
				DBG1(DBG_IKE, "received ME_CONNECT without ME_CONNECTKEY notify"
					 ", aborting");
				this->invalid_syntax = TRUE;
				break;
			}

			if (!this->remote_endpoints->get_count(this->remote_endpoints))
			{
				DBG1(DBG_IKE, "received ME_CONNECT without any ME_ENDPOINT "
					 "payloads, aborting");
				this->invalid_syntax = TRUE;
				break;
			}
			break;
		}
		default:
			break;
	}
	return NEED_MORE;
}

/**
 * For mediation server
 */
METHOD(task_t, build_r_ms, status_t,
	private_ike_me_t *this, message_t *message)
{
	switch(message->get_exchange_type(message))
	{
		case IKE_SA_INIT:
		{
			message->add_notify(message, FALSE, ME_MEDIATION, chunk_empty);
			return NEED_MORE;
		}
		case IKE_AUTH:
		{
			endpoint_notify_t *endpoint;
			if (this->remote_endpoints->get_first(this->remote_endpoints,
											(void**)&endpoint) == SUCCESS &&
				endpoint->get_type(endpoint) == SERVER_REFLEXIVE)
			{
				host_t *host = this->ike_sa->get_other_host(this->ike_sa);
				DBG2(DBG_IKE, "received request for a server reflexive "
					 "endpoint sending: %#H", host);
				endpoint = endpoint_notify_create_from_host(SERVER_REFLEXIVE,
															host, NULL);
				message->add_payload(message, (payload_t*)endpoint->build_notify(endpoint));
				endpoint->destroy(endpoint);
			}
			this->ike_sa->act_as_mediation_server(this->ike_sa);
			break;
		}
		case ME_CONNECT:
		{
			if (this->invalid_syntax)
			{
				message->add_notify(message, TRUE, INVALID_SYNTAX, chunk_empty);
				break;
			}

			ike_sa_id_t *peer_sa;
			if (this->callback)
			{
				peer_sa = charon->mediation_manager->check_and_register(
									charon->mediation_manager, this->peer_id,
									this->ike_sa->get_other_id(this->ike_sa));
			}
			else
			{
				peer_sa = charon->mediation_manager->check(
									charon->mediation_manager, this->peer_id);
			}

			if (!peer_sa)
			{
				/* the peer is not online */
				message->add_notify(message, TRUE, ME_CONNECT_FAILED,
									chunk_empty);
				break;
			}

			job_t *job = (job_t*)mediation_job_create(this->peer_id,
					this->ike_sa->get_other_id(this->ike_sa), this->connect_id,
					this->connect_key, this->remote_endpoints, this->response);
			lib->processor->queue_job(lib->processor, job);
			break;
		}
		default:
			break;
	}
	return SUCCESS;
}

/**
 * For mediation server
 */
METHOD(task_t, process_i_ms, status_t,
	private_ike_me_t *this, message_t *message)
{
	/* FIXME: theoretically we should be prepared to receive a ME_CONNECT_FAILED
	 * here if the responding peer is not able to proceed. in this case we shall
	 * notify the initiating peer with a ME_CONNECT request containing only a
	 * ME_CONNECT_FAILED */
	return SUCCESS;
}

METHOD(ike_me_t, me_connect, void,
	private_ike_me_t *this, identification_t *peer_id)
{
	this->peer_id = peer_id->clone(peer_id);
}

METHOD(ike_me_t, me_respond, void,
	private_ike_me_t *this, identification_t *peer_id, chunk_t connect_id)
{
	this->peer_id = peer_id->clone(peer_id);
	this->connect_id = chunk_clone(connect_id);
	this->response = TRUE;
}

METHOD(ike_me_t, me_callback, void,
	private_ike_me_t *this, identification_t *peer_id)
{
	this->peer_id = peer_id->clone(peer_id);
	this->callback = TRUE;
}

METHOD(ike_me_t, relay, void,
	private_ike_me_t *this, identification_t *requester, chunk_t connect_id,
	chunk_t connect_key, linked_list_t *endpoints, bool response)
{
	this->peer_id = requester->clone(requester);
	this->connect_id = chunk_clone(connect_id);
	this->connect_key = chunk_clone(connect_key);

	this->remote_endpoints->destroy_offset(this->remote_endpoints,
										offsetof(endpoint_notify_t, destroy));
	this->remote_endpoints = endpoints->clone_offset(endpoints,
										offsetof(endpoint_notify_t, clone));

	this->response = response;
}

METHOD(task_t, get_type, task_type_t,
	private_ike_me_t *this)
{
	return TASK_IKE_ME;
}

METHOD(task_t, migrate, void,
	private_ike_me_t *this, ike_sa_t *ike_sa)
{
	this->ike_sa = ike_sa;
}

METHOD(task_t, destroy, void,
	private_ike_me_t *this)
{
	DESTROY_IF(this->peer_id);

	chunk_free(&this->connect_id);
	chunk_free(&this->connect_key);

	this->local_endpoints->destroy_offset(this->local_endpoints,
										offsetof(endpoint_notify_t, destroy));
	this->remote_endpoints->destroy_offset(this->remote_endpoints,
										offsetof(endpoint_notify_t, destroy));

	DESTROY_IF(this->mediated_cfg);
	free(this);
}

/*
 * Described in header.
 */
ike_me_t *ike_me_create(ike_sa_t *ike_sa, bool initiator)
{
	private_ike_me_t *this;

	INIT(this,
		.public = {
			.task = {
				.get_type = _get_type,
				.migrate = _migrate,
				.destroy = _destroy,
			},
			.connect = _me_connect,
			.respond = _me_respond,
			.callback = _me_callback,
			.relay = _relay,
		},
		.ike_sa = ike_sa,
		.initiator = initiator,
		.local_endpoints = linked_list_create(),
		.remote_endpoints = linked_list_create(),
	);

	if (ike_sa->has_condition(ike_sa, COND_ORIGINAL_INITIATOR))
	{
		if (initiator)
		{
			this->public.task.build = _build_i;
			this->public.task.process = _process_i;
		}
		else
		{
			this->public.task.build = _build_r;
			this->public.task.process = _process_r;
		}
	}
	else
	{
		/* mediation server */
		if (initiator)
		{
			this->public.task.build = _build_i_ms;
			this->public.task.process = _process_i_ms;
		}
		else
		{
			this->public.task.build = _build_r_ms;
			this->public.task.process = _process_r_ms;
		}
	}

	return &this->public;
}
