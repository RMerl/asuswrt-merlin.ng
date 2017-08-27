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

#include "isakmp_dpd.h"

#include <daemon.h>
#include <encoding/payloads/notify_payload.h>

typedef struct private_isakmp_dpd_t private_isakmp_dpd_t;

/**
 * Private members of a isakmp_dpd_t task.
 */
struct private_isakmp_dpd_t {

	/**
	 * Public methods and task_t interface.
	 */
	isakmp_dpd_t public;

	/**
	 * Sequence number.
	 */
	u_int32_t seqnr;

	/**
	 * DPD notify type
	 */
	notify_type_t type;

	/**
	 * IKE SA we are serving.
	 */
	ike_sa_t *ike_sa;
};

METHOD(task_t, build, status_t,
	private_isakmp_dpd_t *this, message_t *message)
{
	notify_payload_t *notify;
	ike_sa_id_t *ike_sa_id;
	u_int64_t spi_i, spi_r;
	u_int32_t seqnr;
	chunk_t spi;

	notify = notify_payload_create_from_protocol_and_type(PLV1_NOTIFY,
														  PROTO_IKE, this->type);
	seqnr = htonl(this->seqnr);
	ike_sa_id = this->ike_sa->get_id(this->ike_sa);
	spi_i = ike_sa_id->get_initiator_spi(ike_sa_id);
	spi_r = ike_sa_id->get_responder_spi(ike_sa_id);
	spi = chunk_cata("cc", chunk_from_thing(spi_i), chunk_from_thing(spi_r));

	notify->set_spi_data(notify, spi);
	notify->set_notification_data(notify, chunk_from_thing(seqnr));

	message->add_payload(message, (payload_t*)notify);

	return SUCCESS;
}

METHOD(task_t, process, status_t,
	private_isakmp_dpd_t *this, message_t *message)
{
	/* done in task manager */
	return FAILED;
}

METHOD(task_t, get_type, task_type_t,
	private_isakmp_dpd_t *this)
{
	return TASK_ISAKMP_DPD;
}

METHOD(task_t, migrate, void,
	private_isakmp_dpd_t *this, ike_sa_t *ike_sa)
{
	this->ike_sa = ike_sa;
}

METHOD(task_t, destroy, void,
	private_isakmp_dpd_t *this)
{
	free(this);
}

/*
 * Described in header.
 */
isakmp_dpd_t *isakmp_dpd_create(ike_sa_t *ike_sa, notify_type_t type,
								u_int32_t seqnr)
{
	private_isakmp_dpd_t *this;

	INIT(this,
		.public = {
			.task = {
				.get_type = _get_type,
				.build = _build,
				.process = _process,
				.migrate = _migrate,
				.destroy = _destroy,
			},
		},
		.ike_sa = ike_sa,
		.seqnr = seqnr,
		.type = type,
	);

	return &this->public;
}
