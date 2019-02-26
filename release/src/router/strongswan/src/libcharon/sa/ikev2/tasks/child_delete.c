/*
 * Copyright (C) 2009-2016 Tobias Brunner
 * Copyright (C) 2006-2007 Martin Willi
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

#include "child_delete.h"

#include <daemon.h>
#include <encoding/payloads/delete_payload.h>
#include <processing/jobs/delete_child_sa_job.h>
#include <sa/ikev2/tasks/child_create.h>
#include <sa/ikev2/tasks/child_rekey.h>

#ifndef DELETE_REKEYED_DELAY
#define DELETE_REKEYED_DELAY 5
#endif

typedef struct private_child_delete_t private_child_delete_t;

/**
 * Private members of a child_delete_t task.
 */
struct private_child_delete_t {

	/**
	 * Public methods and task_t interface.
	 */
	child_delete_t public;

	/**
	 * Assigned IKE_SA.
	 */
	ike_sa_t *ike_sa;

	/**
	 * Whether we are the initiator of the exchange
	 */
	bool initiator;

	/**
	 * Protocol of CHILD_SA to delete (as initiator)
	 */
	protocol_id_t protocol;

	/**
	 * Inbound SPI of CHILD_SA to delete (as initiator)
	 */
	uint32_t spi;

	/**
	 * CHILD_SA already expired (as initiator)
	 */
	bool expired;

	/**
	 * CHILD_SAs which get deleted, entry_t*
	 */
	linked_list_t *child_sas;
};

/**
 * Information about a deleted CHILD_SA
 */
typedef struct {
	/** Deleted CHILD_SA */
	child_sa_t *child_sa;
	/** Whether the CHILD_SA was rekeyed */
	bool rekeyed;
	/** Whether to enforce any delete action policy */
	bool check_delete_action;
} entry_t;

CALLBACK(match_child, bool,
	entry_t *entry, va_list args)
{
	child_sa_t *child_sa;

	VA_ARGS_VGET(args, child_sa);
	return entry->child_sa == child_sa;
}

/**
 * build the delete payloads from the listed child_sas
 */
static void build_payloads(private_child_delete_t *this, message_t *message)
{
	delete_payload_t *ah = NULL, *esp = NULL;
	enumerator_t *enumerator;
	entry_t *entry;
	protocol_id_t protocol;
	uint32_t spi;

	enumerator = this->child_sas->create_enumerator(this->child_sas);
	while (enumerator->enumerate(enumerator, (void**)&entry))
	{
		protocol = entry->child_sa->get_protocol(entry->child_sa);
		spi = entry->child_sa->get_spi(entry->child_sa, TRUE);

		switch (protocol)
		{
			case PROTO_ESP:
				if (!esp)
				{
					esp = delete_payload_create(PLV2_DELETE, PROTO_ESP);
					message->add_payload(message, (payload_t*)esp);
				}
				esp->add_spi(esp, spi);
				DBG1(DBG_IKE, "sending DELETE for %N CHILD_SA with SPI %.8x",
					 protocol_id_names, protocol, ntohl(spi));
				break;
			case PROTO_AH:
				if (ah == NULL)
				{
					ah = delete_payload_create(PLV2_DELETE, PROTO_AH);
					message->add_payload(message, (payload_t*)ah);
				}
				ah->add_spi(ah, spi);
				DBG1(DBG_IKE, "sending DELETE for %N CHILD_SA with SPI %.8x",
					 protocol_id_names, protocol, ntohl(spi));
				break;
			default:
				break;
		}
		entry->child_sa->set_state(entry->child_sa, CHILD_DELETING);
	}
	enumerator->destroy(enumerator);
}

/**
 * Check if the given CHILD_SA is the redundant SA created in a rekey collision.
 */
static bool is_redundant(private_child_delete_t *this, child_sa_t *child)
{
	enumerator_t *tasks;
	task_t *task;

	tasks = this->ike_sa->create_task_enumerator(this->ike_sa,
												 TASK_QUEUE_ACTIVE);
	while (tasks->enumerate(tasks, &task))
	{
		if (task->get_type(task) == TASK_CHILD_REKEY)
		{
			child_rekey_t *rekey = (child_rekey_t*)task;

			if (rekey->is_redundant(rekey, child))
			{
				tasks->destroy(tasks);
				return TRUE;
			}
		}
	}
	tasks->destroy(tasks);
	return FALSE;
}

/**
 * Install the outbound CHILD_SA with the given SPI
 */
static void install_outbound(private_child_delete_t *this,
							 protocol_id_t protocol, uint32_t spi)
{
	child_sa_t *child_sa;
	linked_list_t *my_ts, *other_ts;
	status_t status;

	if (!spi)
	{
		return;
	}

	child_sa = this->ike_sa->get_child_sa(this->ike_sa, protocol,
										  spi, FALSE);
	if (!child_sa)
	{
		DBG1(DBG_IKE, "CHILD_SA not found after rekeying");
		return;
	}
	if (this->initiator && is_redundant(this, child_sa))
	{	/* if we won the rekey collision we don't want to install the
		 * redundant SA created by the peer */
		return;
	}

	status = child_sa->install_outbound(child_sa);
	if (status != SUCCESS)
	{
		DBG1(DBG_IKE, "unable to install outbound IPsec SA (SAD) in kernel");
		charon->bus->alert(charon->bus, ALERT_INSTALL_CHILD_SA_FAILED,
						   child_sa);
		/* FIXME: delete the new child_sa? */
		return;
	}

	my_ts = linked_list_create_from_enumerator(
							child_sa->create_ts_enumerator(child_sa, TRUE));
	other_ts = linked_list_create_from_enumerator(
							child_sa->create_ts_enumerator(child_sa, FALSE));

	DBG0(DBG_IKE, "outbound CHILD_SA %s{%d} established "
		 "with SPIs %.8x_i %.8x_o and TS %#R === %#R",
		 child_sa->get_name(child_sa),
		 child_sa->get_unique_id(child_sa),
		 ntohl(child_sa->get_spi(child_sa, TRUE)),
		 ntohl(child_sa->get_spi(child_sa, FALSE)),
		 my_ts, other_ts);

	my_ts->destroy(my_ts);
	other_ts->destroy(other_ts);
}

/**
 * read in payloads and find the children to delete
 */
static void process_payloads(private_child_delete_t *this, message_t *message)
{
	enumerator_t *payloads, *spis;
	payload_t *payload;
	delete_payload_t *delete_payload;
	uint32_t spi;
	protocol_id_t protocol;
	child_sa_t *child_sa;
	entry_t *entry;

	payloads = message->create_payload_enumerator(message);
	while (payloads->enumerate(payloads, &payload))
	{
		if (payload->get_type(payload) == PLV2_DELETE)
		{
			delete_payload = (delete_payload_t*)payload;
			protocol = delete_payload->get_protocol_id(delete_payload);
			if (protocol != PROTO_ESP && protocol != PROTO_AH)
			{
				continue;
			}
			spis = delete_payload->create_spi_enumerator(delete_payload);
			while (spis->enumerate(spis, &spi))
			{
				child_sa = this->ike_sa->get_child_sa(this->ike_sa, protocol,
													  spi, FALSE);
				if (!child_sa)
				{
					DBG1(DBG_IKE, "received DELETE for unknown %N CHILD_SA with"
						 " SPI %.8x", protocol_id_names, protocol, ntohl(spi));
					continue;
				}
				DBG1(DBG_IKE, "received DELETE for %N CHILD_SA with SPI %.8x",
					 protocol_id_names, protocol, ntohl(spi));

				if (this->child_sas->find_first(this->child_sas, match_child,
												NULL, child_sa))
				{
					continue;
				}
				INIT(entry,
					.child_sa = child_sa
				);
				switch (child_sa->get_state(child_sa))
				{
					case CHILD_REKEYED:
						entry->rekeyed = TRUE;
						break;
					case CHILD_DELETED:
						/* already deleted but not yet destroyed, ignore */
					case CHILD_DELETING:
						/* we don't send back a delete if we already initiated
						 * a delete ourself */
						if (!this->initiator)
						{
							free(entry);
							continue;
						}
						break;
					case CHILD_REKEYING:
						/* we reply as usual, rekeying will fail */
					case CHILD_INSTALLED:
						if (!this->initiator)
						{
							if (is_redundant(this, child_sa))
							{
								entry->rekeyed = TRUE;
							}
							else
							{
								entry->check_delete_action = TRUE;
							}
						}
						break;
					default:
						break;
				}
				this->child_sas->insert_last(this->child_sas, entry);
			}
			spis->destroy(spis);
		}
	}
	payloads->destroy(payloads);
}

/**
 * destroy the children listed in this->child_sas, reestablish by policy
 */
static status_t destroy_and_reestablish(private_child_delete_t *this)
{
	enumerator_t *enumerator;
	entry_t *entry;
	child_sa_t *child_sa;
	child_cfg_t *child_cfg;
	protocol_id_t protocol;
	uint32_t spi, reqid;
	action_t action;
	status_t status = SUCCESS;
	time_t now, expire;
	u_int delay;

	now = time_monotonic(NULL);
	delay = lib->settings->get_int(lib->settings, "%s.delete_rekeyed_delay",
								   DELETE_REKEYED_DELAY, lib->ns);

	enumerator = this->child_sas->create_enumerator(this->child_sas);
	while (enumerator->enumerate(enumerator, (void**)&entry))
	{
		child_sa = entry->child_sa;
		child_sa->set_state(child_sa, CHILD_DELETED);
		/* signal child down event if we weren't rekeying */
		protocol = child_sa->get_protocol(child_sa);
		if (!entry->rekeyed)
		{
			charon->bus->child_updown(charon->bus, child_sa, FALSE);
		}
		else
		{
			install_outbound(this, protocol, child_sa->get_rekey_spi(child_sa));
			/* for rekeyed CHILD_SAs we uninstall the outbound SA but don't
			 * immediately destroy it, by default, so we can process delayed
			 * packets */
			child_sa->remove_outbound(child_sa);
			expire = child_sa->get_lifetime(child_sa, TRUE);
			if (delay && (!expire || ((now + delay) < expire)))
			{
				lib->scheduler->schedule_job(lib->scheduler,
					(job_t*)delete_child_sa_job_create_id(
									child_sa->get_unique_id(child_sa)), delay);
				continue;
			}
			else if (now < expire)
			{	/* let it expire naturally */
				continue;
			}
			/* no delay and no lifetime, destroy it immediately */
		}
		spi = child_sa->get_spi(child_sa, TRUE);
		reqid = child_sa->get_reqid(child_sa);
		child_cfg = child_sa->get_config(child_sa);
		child_cfg->get_ref(child_cfg);
		action = child_sa->get_close_action(child_sa);

		this->ike_sa->destroy_child_sa(this->ike_sa, protocol, spi);

		if (entry->check_delete_action)
		{	/* enforce child_cfg policy if deleted passively */
			switch (action)
			{
				case ACTION_RESTART:
					child_cfg->get_ref(child_cfg);
					status = this->ike_sa->initiate(this->ike_sa, child_cfg,
													reqid, NULL, NULL);
					break;
				case ACTION_ROUTE:
					charon->traps->install(charon->traps,
									this->ike_sa->get_peer_cfg(this->ike_sa),
									child_cfg);
					break;
				default:
					break;
			}
		}
		child_cfg->destroy(child_cfg);
		if (status != SUCCESS)
		{
			break;
		}
	}
	enumerator->destroy(enumerator);
	return status;
}

/**
 * send closing signals for all CHILD_SAs over the bus
 */
static void log_children(private_child_delete_t *this)
{
	linked_list_t *my_ts, *other_ts;
	enumerator_t *enumerator;
	entry_t *entry;
	child_sa_t *child_sa;
	uint64_t bytes_in, bytes_out;

	enumerator = this->child_sas->create_enumerator(this->child_sas);
	while (enumerator->enumerate(enumerator, (void**)&entry))
	{
		child_sa = entry->child_sa;
		my_ts = linked_list_create_from_enumerator(
							child_sa->create_ts_enumerator(child_sa, TRUE));
		other_ts = linked_list_create_from_enumerator(
							child_sa->create_ts_enumerator(child_sa, FALSE));
		if (this->expired)
		{
			DBG0(DBG_IKE, "closing expired CHILD_SA %s{%d} "
				 "with SPIs %.8x_i %.8x_o and TS %#R === %#R",
				 child_sa->get_name(child_sa), child_sa->get_unique_id(child_sa),
				 ntohl(child_sa->get_spi(child_sa, TRUE)),
				 ntohl(child_sa->get_spi(child_sa, FALSE)), my_ts, other_ts);
		}
		else
		{
			child_sa->get_usestats(child_sa, TRUE, NULL, &bytes_in, NULL);
			child_sa->get_usestats(child_sa, FALSE, NULL, &bytes_out, NULL);

			DBG0(DBG_IKE, "closing CHILD_SA %s{%d} with SPIs %.8x_i "
				 "(%llu bytes) %.8x_o (%llu bytes) and TS %#R === %#R",
				 child_sa->get_name(child_sa), child_sa->get_unique_id(child_sa),
				 ntohl(child_sa->get_spi(child_sa, TRUE)), bytes_in,
				 ntohl(child_sa->get_spi(child_sa, FALSE)), bytes_out,
				 my_ts, other_ts);
		}
		my_ts->destroy(my_ts);
		other_ts->destroy(other_ts);
	}
	enumerator->destroy(enumerator);
}

METHOD(task_t, build_i, status_t,
	private_child_delete_t *this, message_t *message)
{
	child_sa_t *child_sa;
	entry_t *entry;

	child_sa = this->ike_sa->get_child_sa(this->ike_sa, this->protocol,
										  this->spi, TRUE);
	if (!child_sa)
	{	/* check if it is an outbound sa */
		child_sa = this->ike_sa->get_child_sa(this->ike_sa, this->protocol,
											  this->spi, FALSE);
		if (!child_sa)
		{	/* child does not exist anymore */
			return SUCCESS;
		}
		/* we work only with the inbound SPI */
		this->spi = child_sa->get_spi(child_sa, TRUE);
	}

	if (this->expired && child_sa->get_state(child_sa) == CHILD_REKEYED)
	{	/* the peer was expected to delete this SA, but if we send a DELETE
		 * we might cause a collision there if the CREATE_CHILD_SA response
		 * is delayed (the peer wouldn't know if we deleted this SA due to an
		 * expire or because of a forced delete by the user and might then
		 * ignore the CREATE_CHILD_SA response once it arrives) */
		child_sa->set_state(child_sa, CHILD_DELETED);
		install_outbound(this, this->protocol,
						 child_sa->get_rekey_spi(child_sa));
	}

	if (child_sa->get_state(child_sa) == CHILD_DELETED)
	{	/* DELETEs for this CHILD_SA were already exchanged, but it was not yet
		 * destroyed to allow delayed packets to get processed */
		this->ike_sa->destroy_child_sa(this->ike_sa, this->protocol, this->spi);
		message->set_exchange_type(message, EXCHANGE_TYPE_UNDEFINED);
		return SUCCESS;
	}

	INIT(entry,
		.child_sa = child_sa,
		.rekeyed = child_sa->get_state(child_sa) == CHILD_REKEYED,
	);
	this->child_sas->insert_last(this->child_sas, entry);
	log_children(this);
	build_payloads(this, message);

	if (!entry->rekeyed && this->expired)
	{
		child_cfg_t *child_cfg;

		DBG1(DBG_IKE, "scheduling CHILD_SA recreate after hard expire");
		child_cfg = child_sa->get_config(child_sa);
		this->ike_sa->queue_task(this->ike_sa, (task_t*)
				child_create_create(this->ike_sa, child_cfg->get_ref(child_cfg),
									FALSE, NULL, NULL));
	}
	return NEED_MORE;
}

METHOD(task_t, process_i, status_t,
	private_child_delete_t *this, message_t *message)
{
	process_payloads(this, message);
	DBG1(DBG_IKE, "CHILD_SA closed");
	return destroy_and_reestablish(this);
}

METHOD(task_t, process_r, status_t,
	private_child_delete_t *this, message_t *message)
{
	process_payloads(this, message);
	log_children(this);
	return NEED_MORE;
}

METHOD(task_t, build_r, status_t,
	private_child_delete_t *this, message_t *message)
{
	build_payloads(this, message);
	DBG1(DBG_IKE, "CHILD_SA closed");
	return destroy_and_reestablish(this);
}

METHOD(task_t, get_type, task_type_t,
	private_child_delete_t *this)
{
	return TASK_CHILD_DELETE;
}

METHOD(child_delete_t , get_child, child_sa_t*,
	private_child_delete_t *this)
{
	child_sa_t *child_sa = NULL;
	entry_t *entry;

	if (this->child_sas->get_first(this->child_sas, (void**)&entry) == SUCCESS)
	{
		child_sa = entry->child_sa;
	}
	return child_sa;
}

METHOD(task_t, migrate, void,
	private_child_delete_t *this, ike_sa_t *ike_sa)
{
	this->ike_sa = ike_sa;

	this->child_sas->destroy_function(this->child_sas, free);
	this->child_sas = linked_list_create();
}

METHOD(task_t, destroy, void,
	private_child_delete_t *this)
{
	this->child_sas->destroy_function(this->child_sas, free);
	free(this);
}

/*
 * Described in header.
 */
child_delete_t *child_delete_create(ike_sa_t *ike_sa, protocol_id_t protocol,
									uint32_t spi, bool expired)
{
	private_child_delete_t *this;

	INIT(this,
		.public = {
			.task = {
				.get_type = _get_type,
				.migrate = _migrate,
				.destroy = _destroy,
			},
			.get_child = _get_child,
		},
		.ike_sa = ike_sa,
		.child_sas = linked_list_create(),
		.protocol = protocol,
		.spi = spi,
		.expired = expired,
	);

	if (protocol != PROTO_NONE)
	{
		this->public.task.build = _build_i;
		this->public.task.process = _process_i;
		this->initiator = TRUE;
	}
	else
	{
		this->public.task.build = _build_r;
		this->public.task.process = _process_r;
		this->initiator = FALSE;
	}
	return &this->public;
}
