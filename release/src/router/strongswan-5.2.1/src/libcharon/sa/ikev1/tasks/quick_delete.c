/*
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
/*
 * Copyright (C) 2013 Oliver Smith
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "quick_delete.h"

#include <daemon.h>
#include <encoding/payloads/delete_payload.h>

typedef struct private_quick_delete_t private_quick_delete_t;

/**
 * Private members of a quick_delete_t task.
 */
struct private_quick_delete_t {

	/**
	 * Public methods and task_t interface.
	 */
	quick_delete_t public;

	/**
	 * Assigned IKE_SA.
	 */
	ike_sa_t *ike_sa;

	/**
	 * Are we the initiator?
	 */
	bool initiator;

	/**
	 * Protocol of CHILD_SA to delete
	 */
	protocol_id_t protocol;

	/**
	 * Inbound SPI of CHILD_SA to delete
	 */
	u_int32_t spi;

	/**
	 * Send delete even if SA does not exist
	 */
	bool force;

	/**
	 * SA already expired?
	 */
	bool expired;
};

/**
 * Delete the specified CHILD_SA, if found
 */
static bool delete_child(private_quick_delete_t *this, protocol_id_t protocol,
						 u_int32_t spi, bool remote_close)
{
	u_int64_t bytes_in, bytes_out;
	child_sa_t *child_sa;
	linked_list_t *my_ts, *other_ts;
	child_cfg_t *child_cfg;
	bool rekeyed;

	child_sa = this->ike_sa->get_child_sa(this->ike_sa, protocol, spi, TRUE);
	if (!child_sa)
	{	/* fallback and check for outbound SA */
		child_sa = this->ike_sa->get_child_sa(this->ike_sa, protocol, spi, FALSE);
		if (!child_sa)
		{
			return FALSE;
		}
		this->spi = spi = child_sa->get_spi(child_sa, TRUE);
	}

	rekeyed = child_sa->get_state(child_sa) == CHILD_REKEYING;
	child_sa->set_state(child_sa, CHILD_DELETING);

	my_ts = linked_list_create_from_enumerator(
							child_sa->create_ts_enumerator(child_sa, TRUE));
	other_ts = linked_list_create_from_enumerator(
							child_sa->create_ts_enumerator(child_sa, FALSE));
	if (this->expired)
	{
		DBG0(DBG_IKE, "closing expired CHILD_SA %s{%d} "
			 "with SPIs %.8x_i %.8x_o and TS %#R=== %#R",
			 child_sa->get_name(child_sa), child_sa->get_reqid(child_sa),
			 ntohl(child_sa->get_spi(child_sa, TRUE)),
			 ntohl(child_sa->get_spi(child_sa, FALSE)), my_ts, other_ts);
	}
	else
	{
		child_sa->get_usestats(child_sa, TRUE, NULL, &bytes_in, NULL);
		child_sa->get_usestats(child_sa, FALSE, NULL, &bytes_out, NULL);

		DBG0(DBG_IKE, "closing CHILD_SA %s{%d} with SPIs "
			 "%.8x_i (%llu bytes) %.8x_o (%llu bytes) and TS %#R=== %#R",
			 child_sa->get_name(child_sa), child_sa->get_reqid(child_sa),
			 ntohl(child_sa->get_spi(child_sa, TRUE)), bytes_in,
			 ntohl(child_sa->get_spi(child_sa, FALSE)), bytes_out,
			 my_ts, other_ts);
	}
	my_ts->destroy(my_ts);
	other_ts->destroy(other_ts);

	if (!rekeyed)
	{
		charon->bus->child_updown(charon->bus, child_sa, FALSE);

		if (remote_close)
		{
			child_cfg = child_sa->get_config(child_sa);
			child_cfg->get_ref(child_cfg);

			switch (child_sa->get_close_action(child_sa))
			{
				case ACTION_RESTART:
					child_cfg->get_ref(child_cfg);
					this->ike_sa->initiate(this->ike_sa, child_cfg,
									child_sa->get_reqid(child_sa), NULL, NULL);
					break;
				case ACTION_ROUTE:
					charon->traps->install(charon->traps,
									this->ike_sa->get_peer_cfg(this->ike_sa),
									child_cfg, child_sa->get_reqid(child_sa));
					break;
				default:
					break;
			}
			child_cfg->destroy(child_cfg);
		}
	}
	this->ike_sa->destroy_child_sa(this->ike_sa, protocol, spi);

	return TRUE;
}

METHOD(task_t, build_i, status_t,
	private_quick_delete_t *this, message_t *message)
{
	if (delete_child(this, this->protocol, this->spi, FALSE) || this->force)
	{
		delete_payload_t *delete_payload;

		DBG1(DBG_IKE, "sending DELETE for %N CHILD_SA with SPI %.8x",
			 protocol_id_names, this->protocol, ntohl(this->spi));

		delete_payload = delete_payload_create(PLV1_DELETE, this->protocol);
		delete_payload->add_spi(delete_payload, this->spi);
		message->add_payload(message, &delete_payload->payload_interface);

		return SUCCESS;
	}
	this->ike_sa->flush_queue(this->ike_sa, TASK_QUEUE_ACTIVE);
	return ALREADY_DONE;
}

METHOD(task_t, process_i, status_t,
	private_quick_delete_t *this, message_t *message)
{
	return FAILED;
}

METHOD(task_t, process_r, status_t,
	private_quick_delete_t *this, message_t *message)
{
	enumerator_t *payloads, *spis;
	payload_t *payload;
	delete_payload_t *delete_payload;
	protocol_id_t protocol;
	u_int32_t spi;

	payloads = message->create_payload_enumerator(message);
	while (payloads->enumerate(payloads, &payload))
	{
		if (payload->get_type(payload) == PLV1_DELETE)
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
				DBG1(DBG_IKE, "received DELETE for %N CHILD_SA with SPI %.8x",
					 protocol_id_names, protocol, ntohl(spi));
				if (!delete_child(this, protocol, spi, TRUE))
				{
					DBG1(DBG_IKE, "CHILD_SA not found, ignored");
					continue;
				}
			}
			spis->destroy(spis);
		}
	}
	payloads->destroy(payloads);

	return SUCCESS;
}

METHOD(task_t, build_r, status_t,
	private_quick_delete_t *this, message_t *message)
{
	return FAILED;
}

METHOD(task_t, get_type, task_type_t,
	private_quick_delete_t *this)
{
	return TASK_QUICK_DELETE;
}

METHOD(task_t, migrate, void,
	private_quick_delete_t *this, ike_sa_t *ike_sa)
{
	this->ike_sa = ike_sa;
}

METHOD(task_t, destroy, void,
	private_quick_delete_t *this)
{
	free(this);
}

/*
 * Described in header.
 */
quick_delete_t *quick_delete_create(ike_sa_t *ike_sa, protocol_id_t protocol,
									u_int32_t spi, bool force, bool expired)
{
	private_quick_delete_t *this;

	INIT(this,
		.public = {
			.task = {
				.get_type = _get_type,
				.migrate = _migrate,
				.destroy = _destroy,
			},
		},
		.ike_sa = ike_sa,
		.protocol = protocol,
		.spi = spi,
		.force = force,
		.expired = expired,
	);

	if (protocol != PROTO_NONE)
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
