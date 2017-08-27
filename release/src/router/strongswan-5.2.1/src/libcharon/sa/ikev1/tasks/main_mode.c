/*
 * Copyright (C) 2011-2012 Tobias Brunner
 * Hochschule fuer Technik Rapperswil
 *
 * Copyright (C) 2011 Martin Willi
 * Copyright (C) 2011 revosec AG
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

#include "main_mode.h"

#include <string.h>

#include <daemon.h>
#include <sa/ikev1/phase1.h>
#include <encoding/payloads/sa_payload.h>
#include <encoding/payloads/id_payload.h>
#include <encoding/payloads/hash_payload.h>
#include <sa/ikev1/tasks/xauth.h>
#include <sa/ikev1/tasks/mode_config.h>
#include <sa/ikev1/tasks/informational.h>
#include <sa/ikev1/tasks/isakmp_delete.h>
#include <processing/jobs/adopt_children_job.h>
#include <processing/jobs/delete_ike_sa_job.h>

typedef struct private_main_mode_t private_main_mode_t;

/**
 * Private members of a main_mode_t task.
 */
struct private_main_mode_t {

	/**
	 * Public methods and task_t interface.
	 */
	main_mode_t public;

	/**
	 * Assigned IKE_SA.
	 */
	ike_sa_t *ike_sa;

	/**
	 * Are we the initiator?
	 */
	bool initiator;

	/**
	 * Common phase 1 helper class
	 */
	phase1_t *ph1;

	/**
	 * IKE config to establish
	 */
	ike_cfg_t *ike_cfg;

	/**
	 * Peer config to use
	 */
	peer_cfg_t *peer_cfg;

	/**
	 * selected IKE proposal
	 */
	proposal_t *proposal;

	/**
	 * Negotiated SA lifetime
	 */
	u_int32_t lifetime;

	/**
	 * Negotiated authentication method
	 */
	auth_method_t method;

	/** states of main mode */
	enum {
		MM_INIT,
		MM_SA,
		MM_KE,
		MM_AUTH,
	} state;
};

/**
 * Set IKE_SA to established state
 */
static bool establish(private_main_mode_t *this)
{
	if (!charon->bus->authorize(charon->bus, TRUE))
	{
		DBG1(DBG_IKE, "final authorization hook forbids IKE_SA, cancelling");
		return FALSE;
	}

	DBG0(DBG_IKE, "IKE_SA %s[%d] established between %H[%Y]...%H[%Y]",
		 this->ike_sa->get_name(this->ike_sa),
		 this->ike_sa->get_unique_id(this->ike_sa),
		 this->ike_sa->get_my_host(this->ike_sa),
		 this->ike_sa->get_my_id(this->ike_sa),
		 this->ike_sa->get_other_host(this->ike_sa),
		 this->ike_sa->get_other_id(this->ike_sa));

	this->ike_sa->set_state(this->ike_sa, IKE_ESTABLISHED);
	charon->bus->ike_updown(charon->bus, this->ike_sa, TRUE);

	return TRUE;
}

/**
 * Check for notify errors, return TRUE if error found
 */
static bool has_notify_errors(private_main_mode_t *this, message_t *message)
{
	enumerator_t *enumerator;
	payload_t *payload;
	bool err = FALSE;

	enumerator = message->create_payload_enumerator(message);
	while (enumerator->enumerate(enumerator, &payload))
	{
		if (payload->get_type(payload) == PLV1_NOTIFY)
		{
			notify_payload_t *notify;
			notify_type_t type;

			notify = (notify_payload_t*)payload;
			type = notify->get_notify_type(notify);
			if (type < 16384)
			{
				DBG1(DBG_IKE, "received %N error notify",
					 notify_type_names, type);
				err = TRUE;
			}
			else if (type == INITIAL_CONTACT_IKEV1)
			{
				if (!this->initiator && this->state == MM_AUTH)
				{
					/* If authenticated and received INITIAL_CONTACT,
					 * delete any existing IKE_SAs with that peer.
					 * The delete takes place when the SA is checked in due
					 * to other id not known until the 3rd message.*/
					this->ike_sa->set_condition(this->ike_sa,
												COND_INIT_CONTACT_SEEN, TRUE);
				}
			}
			else
			{
				DBG1(DBG_IKE, "received %N notify", notify_type_names, type);
			}
		}
	}
	enumerator->destroy(enumerator);

	return err;
}

/**
 * Queue a task sending a notify in an INFORMATIONAL exchange
 */
static status_t send_notify(private_main_mode_t *this, notify_type_t type)
{
	notify_payload_t *notify;
	ike_sa_id_t *ike_sa_id;
	u_int64_t spi_i, spi_r;
	chunk_t spi;

	notify = notify_payload_create_from_protocol_and_type(PLV1_NOTIFY,
														  PROTO_IKE, type);
	ike_sa_id = this->ike_sa->get_id(this->ike_sa);
	spi_i = ike_sa_id->get_initiator_spi(ike_sa_id);
	spi_r = ike_sa_id->get_responder_spi(ike_sa_id);
	spi = chunk_cata("cc", chunk_from_thing(spi_i), chunk_from_thing(spi_r));
	notify->set_spi_data(notify, spi);

	this->ike_sa->queue_task(this->ike_sa,
						(task_t*)informational_create(this->ike_sa, notify));
	/* cancel all active/passive tasks in favour of informational */
	this->ike_sa->flush_queue(this->ike_sa,
					this->initiator ? TASK_QUEUE_ACTIVE : TASK_QUEUE_PASSIVE);
	return ALREADY_DONE;
}

/**
 * Queue a delete task if authentication failed as initiator
 */
static status_t send_delete(private_main_mode_t *this)
{
	this->ike_sa->queue_task(this->ike_sa,
						(task_t*)isakmp_delete_create(this->ike_sa, TRUE));
	/* cancel all active tasks in favour of informational */
	this->ike_sa->flush_queue(this->ike_sa,
					this->initiator ? TASK_QUEUE_ACTIVE : TASK_QUEUE_PASSIVE);
	return ALREADY_DONE;
}

METHOD(task_t, build_i, status_t,
	private_main_mode_t *this, message_t *message)
{
	switch (this->state)
	{
		case MM_INIT:
		{
			sa_payload_t *sa_payload;
			linked_list_t *proposals;
			packet_t *packet;

			DBG0(DBG_IKE, "initiating Main Mode IKE_SA %s[%d] to %H",
				 this->ike_sa->get_name(this->ike_sa),
				 this->ike_sa->get_unique_id(this->ike_sa),
				 this->ike_sa->get_other_host(this->ike_sa));
			this->ike_sa->set_state(this->ike_sa, IKE_CONNECTING);

			this->ike_cfg = this->ike_sa->get_ike_cfg(this->ike_sa);
			this->peer_cfg = this->ike_sa->get_peer_cfg(this->ike_sa);
			this->peer_cfg->get_ref(this->peer_cfg);

			this->method = this->ph1->get_auth_method(this->ph1, this->peer_cfg);
			if (this->method == AUTH_NONE)
			{
				DBG1(DBG_CFG, "configuration uses unsupported authentication");
				return FAILED;
			}
			this->lifetime = this->peer_cfg->get_reauth_time(this->peer_cfg,
															 FALSE);
			if (!this->lifetime)
			{	/* fall back to rekey time of no rekey time configured */
				this->lifetime = this->peer_cfg->get_rekey_time(this->peer_cfg,
																 FALSE);
			}
			this->lifetime += this->peer_cfg->get_over_time(this->peer_cfg);
			proposals = this->ike_cfg->get_proposals(this->ike_cfg);
			sa_payload = sa_payload_create_from_proposals_v1(proposals,
									this->lifetime, 0, this->method, MODE_NONE,
									ENCAP_NONE, 0);
			proposals->destroy_offset(proposals, offsetof(proposal_t, destroy));

			message->add_payload(message, &sa_payload->payload_interface);

			/* pregenerate message to store SA payload */
			if (this->ike_sa->generate_message(this->ike_sa, message,
											   &packet) != SUCCESS)
			{
				DBG1(DBG_IKE, "pregenerating SA payload failed");
				return FAILED;
			}
			packet->destroy(packet);
			if (!this->ph1->save_sa_payload(this->ph1, message))
			{
				return FAILED;
			}

			this->state = MM_SA;
			return NEED_MORE;
		}
		case MM_SA:
		{
			u_int16_t group;

			if (!this->ph1->create_hasher(this->ph1))
			{
				return send_notify(this, NO_PROPOSAL_CHOSEN);
			}
			if (!this->proposal->get_algorithm(this->proposal,
										DIFFIE_HELLMAN_GROUP, &group, NULL))
			{
				DBG1(DBG_IKE, "DH group selection failed");
				return send_notify(this, NO_PROPOSAL_CHOSEN);
			}
			if (!this->ph1->create_dh(this->ph1, group))
			{
				DBG1(DBG_IKE, "negotiated DH group not supported");
				return send_notify(this, INVALID_KEY_INFORMATION);
			}
			if (!this->ph1->add_nonce_ke(this->ph1, message))
			{
				return send_notify(this, INVALID_KEY_INFORMATION);
			}
			this->state = MM_KE;
			return NEED_MORE;
		}
		case MM_KE:
		{
			id_payload_t *id_payload;
			identification_t *id;

			id = this->ph1->get_id(this->ph1, this->peer_cfg, TRUE);
			if (!id)
			{
				DBG1(DBG_CFG, "own identity not known");
				return send_notify(this, INVALID_ID_INFORMATION);
			}
			this->ike_sa->set_my_id(this->ike_sa, id->clone(id));
			id_payload = id_payload_create_from_identification(PLV1_ID, id);
			message->add_payload(message, &id_payload->payload_interface);

			if (!this->ph1->build_auth(this->ph1, this->method, message,
									   id_payload->get_encoded(id_payload)))
			{
				return send_notify(this, AUTHENTICATION_FAILED);
			}

			this->state = MM_AUTH;
			return NEED_MORE;
		}
		default:
			return FAILED;
	}
}

METHOD(task_t, process_r, status_t,
	private_main_mode_t *this, message_t *message)
{
	switch (this->state)
	{
		case MM_INIT:
		{
			linked_list_t *list;
			sa_payload_t *sa_payload;
			bool private;

			this->ike_cfg = this->ike_sa->get_ike_cfg(this->ike_sa);
			DBG0(DBG_IKE, "%H is initiating a Main Mode IKE_SA",
				 message->get_source(message));
			this->ike_sa->set_state(this->ike_sa, IKE_CONNECTING);

			this->ike_sa->update_hosts(this->ike_sa,
									   message->get_destination(message),
									   message->get_source(message), TRUE);

			sa_payload = (sa_payload_t*)message->get_payload(message,
													PLV1_SECURITY_ASSOCIATION);
			if (!sa_payload)
			{
				DBG1(DBG_IKE, "SA payload missing");
				return send_notify(this, INVALID_PAYLOAD_TYPE);
			}
			if (!this->ph1->save_sa_payload(this->ph1, message))
			{
				return send_notify(this, INVALID_PAYLOAD_TYPE);
			}

			list = sa_payload->get_proposals(sa_payload);
			private = this->ike_sa->supports_extension(this->ike_sa,
														   EXT_STRONGSWAN);
			this->proposal = this->ike_cfg->select_proposal(this->ike_cfg,
															list, private);
			list->destroy_offset(list, offsetof(proposal_t, destroy));
			if (!this->proposal)
			{
				DBG1(DBG_IKE, "no proposal found");
				return send_notify(this, NO_PROPOSAL_CHOSEN);
			}
			this->ike_sa->set_proposal(this->ike_sa, this->proposal);

			this->method = sa_payload->get_auth_method(sa_payload);
			this->lifetime = sa_payload->get_lifetime(sa_payload);

			this->state = MM_SA;
			return NEED_MORE;
		}
		case MM_SA:
		{
			u_int16_t group;

			if (!this->ph1->create_hasher(this->ph1))
			{
				return send_notify(this, INVALID_KEY_INFORMATION);
			}
			if (!this->proposal->get_algorithm(this->proposal,
										DIFFIE_HELLMAN_GROUP, &group, NULL))
			{
				DBG1(DBG_IKE, "DH group selection failed");
				return send_notify(this, INVALID_KEY_INFORMATION);
			}
			if (!this->ph1->create_dh(this->ph1, group))
			{
				DBG1(DBG_IKE, "negotiated DH group not supported");
				return send_notify(this, INVALID_KEY_INFORMATION);
			}
			if (!this->ph1->get_nonce_ke(this->ph1, message))
			{
				return send_notify(this, INVALID_PAYLOAD_TYPE);
			}
			this->state = MM_KE;
			return NEED_MORE;
		}
		case MM_KE:
		{
			id_payload_t *id_payload;
			identification_t *id;

			id_payload = (id_payload_t*)message->get_payload(message, PLV1_ID);
			if (!id_payload)
			{
				DBG1(DBG_IKE, "IDii payload missing");
				return send_notify(this, INVALID_PAYLOAD_TYPE);
			}
			id = id_payload->get_identification(id_payload);
			this->ike_sa->set_other_id(this->ike_sa, id);

			while (TRUE)
			{
				DESTROY_IF(this->peer_cfg);
				this->peer_cfg = this->ph1->select_config(this->ph1,
													this->method, FALSE, id);
				if (!this->peer_cfg)
				{
					return send_notify(this, AUTHENTICATION_FAILED);
				}
				this->ike_sa->set_peer_cfg(this->ike_sa, this->peer_cfg);

				if (this->ph1->verify_auth(this->ph1, this->method, message,
										   id_payload->get_encoded(id_payload)))
				{
					break;
				}
			}

			if (!charon->bus->authorize(charon->bus, FALSE))
			{
				DBG1(DBG_IKE, "Main Mode authorization hook forbids IKE_SA, "
					 "cancelling");
				return send_notify(this, AUTHENTICATION_FAILED);
			}

			this->state = MM_AUTH;
			if (has_notify_errors(this, message))
			{
				return FAILED;
			}
			return NEED_MORE;
		}
		default:
			return FAILED;
	}
}

METHOD(task_t, build_r, status_t,
	private_main_mode_t *this, message_t *message)
{
	switch (this->state)
	{
		case MM_SA:
		{
			sa_payload_t *sa_payload;

			sa_payload = sa_payload_create_from_proposal_v1(this->proposal,
									this->lifetime, 0, this->method, MODE_NONE,
									ENCAP_NONE, 0);
			message->add_payload(message, &sa_payload->payload_interface);

			return NEED_MORE;
		}
		case MM_KE:
		{
			if (!this->ph1->add_nonce_ke(this->ph1, message))
			{
				return send_notify(this, INVALID_KEY_INFORMATION);
			}
			if (!this->ph1->derive_keys(this->ph1, this->peer_cfg, this->method))
			{
				return send_notify(this, INVALID_KEY_INFORMATION);
			}
			return NEED_MORE;
		}
		case MM_AUTH:
		{
			id_payload_t *id_payload;
			identification_t *id;
			adopt_children_job_t *job = NULL;
			xauth_t *xauth = NULL;

			id = this->ph1->get_id(this->ph1, this->peer_cfg, TRUE);
			if (!id)
			{
				DBG1(DBG_CFG, "own identity not known");
				return send_notify(this, INVALID_ID_INFORMATION);
			}
			this->ike_sa->set_my_id(this->ike_sa, id->clone(id));

			id_payload = id_payload_create_from_identification(PLV1_ID, id);
			message->add_payload(message, &id_payload->payload_interface);

			if (!this->ph1->build_auth(this->ph1, this->method, message,
									   id_payload->get_encoded(id_payload)))
			{
				return send_notify(this, AUTHENTICATION_FAILED);
			}

			switch (this->method)
			{
				case AUTH_XAUTH_INIT_PSK:
				case AUTH_XAUTH_INIT_RSA:
				case AUTH_HYBRID_INIT_RSA:
					xauth = xauth_create(this->ike_sa, TRUE);
					this->ike_sa->queue_task(this->ike_sa, (task_t*)xauth);
					break;
				case AUTH_XAUTH_RESP_PSK:
				case AUTH_XAUTH_RESP_RSA:
				case AUTH_HYBRID_RESP_RSA:
					/* wait for XAUTH request */
					break;
				default:
					if (charon->ike_sa_manager->check_uniqueness(
								charon->ike_sa_manager, this->ike_sa, FALSE))
					{
						DBG1(DBG_IKE, "cancelling Main Mode due to uniqueness "
							 "policy");
						return send_notify(this, AUTHENTICATION_FAILED);
					}
					if (!establish(this))
					{
						return send_notify(this, AUTHENTICATION_FAILED);
					}
					job = adopt_children_job_create(
											this->ike_sa->get_id(this->ike_sa));
					break;
			}
			if (this->ph1->has_virtual_ip(this->ph1, this->peer_cfg))
			{
				if (this->peer_cfg->use_pull_mode(this->peer_cfg))
				{
					this->ike_sa->queue_task(this->ike_sa,
						(task_t*)mode_config_create(this->ike_sa, TRUE, TRUE));
				}
			}
			else if (this->ph1->has_pool(this->ph1, this->peer_cfg))
			{
				if (!this->peer_cfg->use_pull_mode(this->peer_cfg))
				{
					if (job)
					{
						job->queue_task(job, (task_t*)
								mode_config_create(this->ike_sa, TRUE, FALSE));
					}
					else if (xauth)
					{
						xauth->queue_mode_config_push(xauth);
					}
					else
					{
						this->ike_sa->queue_task(this->ike_sa, (task_t*)
								mode_config_create(this->ike_sa, TRUE, FALSE));
					}
				}
			}
			if (job)
			{
				lib->processor->queue_job(lib->processor, (job_t*)job);
			}
			return SUCCESS;
		}
		default:
			return FAILED;
	}
}

/**
 * Schedule a timeout for the IKE_SA should it not establish
 */
static void schedule_timeout(ike_sa_t *ike_sa)
{
	job_t *job;

	job = (job_t*)delete_ike_sa_job_create(ike_sa->get_id(ike_sa), FALSE);
	lib->scheduler->schedule_job(lib->scheduler, job, HALF_OPEN_IKE_SA_TIMEOUT);
}

METHOD(task_t, process_i, status_t,
	private_main_mode_t *this, message_t *message)
{
	switch (this->state)
	{
		case MM_SA:
		{
			linked_list_t *list;
			sa_payload_t *sa_payload;
			auth_method_t method;
			u_int32_t lifetime;
			bool private;

			sa_payload = (sa_payload_t*)message->get_payload(message,
													PLV1_SECURITY_ASSOCIATION);
			if (!sa_payload)
			{
				DBG1(DBG_IKE, "SA payload missing");
				return send_notify(this, INVALID_PAYLOAD_TYPE);
			}
			list = sa_payload->get_proposals(sa_payload);
			private = this->ike_sa->supports_extension(this->ike_sa,
														   EXT_STRONGSWAN);
			this->proposal = this->ike_cfg->select_proposal(this->ike_cfg,
															list, private);
			list->destroy_offset(list, offsetof(proposal_t, destroy));
			if (!this->proposal)
			{
				DBG1(DBG_IKE, "no proposal found");
				return send_notify(this, NO_PROPOSAL_CHOSEN);
			}
			this->ike_sa->set_proposal(this->ike_sa, this->proposal);

			lifetime = sa_payload->get_lifetime(sa_payload);
			if (lifetime != this->lifetime)
			{
				DBG1(DBG_IKE, "received lifetime %us does not match configured "
					 "lifetime %us", lifetime, this->lifetime);
			}
			this->lifetime = lifetime;
			method = sa_payload->get_auth_method(sa_payload);
			if (method != this->method)
			{
				DBG1(DBG_IKE, "received %N authentication, but configured %N, "
					 "continue with configured", auth_method_names, method,
					 auth_method_names, this->method);
			}
			return NEED_MORE;
		}
		case MM_KE:
		{
			if (!this->ph1->get_nonce_ke(this->ph1, message))
			{
				return send_notify(this, INVALID_PAYLOAD_TYPE);
			}
			if (!this->ph1->derive_keys(this->ph1, this->peer_cfg, this->method))
			{
				return send_notify(this, INVALID_KEY_INFORMATION);
			}
			return NEED_MORE;
		}
		case MM_AUTH:
		{
			id_payload_t *id_payload;
			identification_t *id, *cid;

			id_payload = (id_payload_t*)message->get_payload(message, PLV1_ID);
			if (!id_payload)
			{
				DBG1(DBG_IKE, "IDir payload missing");
				return send_delete(this);
			}
			id = id_payload->get_identification(id_payload);
			cid = this->ph1->get_id(this->ph1, this->peer_cfg, FALSE);
			if (cid && !id->matches(id, cid))
			{
				DBG1(DBG_IKE, "IDir '%Y' does not match to '%Y'", id, cid);
				id->destroy(id);
				return send_delete(this);
			}
			this->ike_sa->set_other_id(this->ike_sa, id);

			if (!this->ph1->verify_auth(this->ph1, this->method, message,
										id_payload->get_encoded(id_payload)))
			{
				return send_delete(this);
			}
			if (!charon->bus->authorize(charon->bus, FALSE))
			{
				DBG1(DBG_IKE, "Main Mode authorization hook forbids IKE_SA, "
					 "cancelling");
				return send_delete(this);
			}

			switch (this->method)
			{
				case AUTH_XAUTH_INIT_PSK:
				case AUTH_XAUTH_INIT_RSA:
				case AUTH_HYBRID_INIT_RSA:
					/* wait for XAUTH request */
					schedule_timeout(this->ike_sa);
					break;
				case AUTH_XAUTH_RESP_PSK:
				case AUTH_XAUTH_RESP_RSA:
				case AUTH_HYBRID_RESP_RSA:
					this->ike_sa->queue_task(this->ike_sa,
									(task_t*)xauth_create(this->ike_sa, TRUE));
					break;
				default:
					if (charon->ike_sa_manager->check_uniqueness(
								charon->ike_sa_manager, this->ike_sa, FALSE))
					{
						DBG1(DBG_IKE, "cancelling Main Mode due to uniqueness "
							 "policy");
						return send_delete(this);
					}
					if (!establish(this))
					{
						return send_delete(this);
					}
					break;
			}
			/* check for and prepare mode config push/pull */
			if (this->ph1->has_virtual_ip(this->ph1, this->peer_cfg))
			{
				if (this->peer_cfg->use_pull_mode(this->peer_cfg))
				{
					this->ike_sa->queue_task(this->ike_sa,
						(task_t*)mode_config_create(this->ike_sa, TRUE, TRUE));
				}
				else
				{
					schedule_timeout(this->ike_sa);
				}
			}
			else if (this->ph1->has_pool(this->ph1, this->peer_cfg))
			{
				if (this->peer_cfg->use_pull_mode(this->peer_cfg))
				{
					schedule_timeout(this->ike_sa);
				}
				else
				{
					this->ike_sa->queue_task(this->ike_sa,
						(task_t*)mode_config_create(this->ike_sa, TRUE, FALSE));
				}
			}
			return SUCCESS;
		}
		default:
			return FAILED;
	}
}

METHOD(task_t, get_type, task_type_t,
	private_main_mode_t *this)
{
	return TASK_MAIN_MODE;
}

METHOD(task_t, migrate, void,
	private_main_mode_t *this, ike_sa_t *ike_sa)
{
	DESTROY_IF(this->peer_cfg);
	DESTROY_IF(this->proposal);
	this->ph1->destroy(this->ph1);

	this->ike_sa = ike_sa;
	this->state = MM_INIT;
	this->peer_cfg = NULL;
	this->proposal = NULL;
	this->ph1 = phase1_create(ike_sa, this->initiator);
}

METHOD(task_t, destroy, void,
	private_main_mode_t *this)
{
	DESTROY_IF(this->peer_cfg);
	DESTROY_IF(this->proposal);
	this->ph1->destroy(this->ph1);
	free(this);
}

/*
 * Described in header.
 */
main_mode_t *main_mode_create(ike_sa_t *ike_sa, bool initiator)
{
	private_main_mode_t *this;

	INIT(this,
		.public = {
			.task = {
				.get_type = _get_type,
				.migrate = _migrate,
				.destroy = _destroy,
			},
		},
		.ike_sa = ike_sa,
		.ph1 = phase1_create(ike_sa, initiator),
		.initiator = initiator,
		.state = MM_INIT,
	);

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

	return &this->public;
}
