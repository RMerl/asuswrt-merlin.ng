/*
 * Copyright (C) 2005-2007 Martin Willi
 * Copyright (C) 2005 Jan Hutter
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

#include "process_message_job.h"

#include <daemon.h>

typedef struct private_process_message_job_t private_process_message_job_t;

/**
 * Private data of an process_message_job_t Object
 */
struct private_process_message_job_t {
	/**
	 * public process_message_job_t interface
	 */
	process_message_job_t public;

	/**
	 * Message associated with this job
	 */
	message_t *message;
};

METHOD(job_t, destroy, void,
	private_process_message_job_t *this)
{
	this->message->destroy(this->message);
	free(this);
}

METHOD(job_t, execute, job_requeue_t,
	private_process_message_job_t *this)
{
	ike_sa_t *ike_sa;

#ifdef ME
	/* if this is an unencrypted INFORMATIONAL exchange it is likely a
	 * connectivity check. */
	if (this->message->get_exchange_type(this->message) == INFORMATIONAL &&
		this->message->get_first_payload_type(this->message) != PLV2_ENCRYPTED)
	{
		/* theoretically this could also be an error message
		 * see RFC 4306, section 1.5. */
		DBG1(DBG_NET, "received unencrypted informational: from %#H to %#H",
			 this->message->get_source(this->message),
			 this->message->get_destination(this->message));
		charon->connect_manager->process_check(charon->connect_manager, this->message);
		return JOB_REQUEUE_NONE;
	}
#endif /* ME */

	ike_sa = charon->ike_sa_manager->checkout_by_message(charon->ike_sa_manager,
														 this->message);
	if (ike_sa)
	{
		DBG1(DBG_NET, "received packet: from %#H to %#H (%zu bytes)",
			 this->message->get_source(this->message),
			 this->message->get_destination(this->message),
			 this->message->get_packet_data(this->message).len);
		if (ike_sa->process_message(ike_sa, this->message) == DESTROY_ME)
		{
			charon->ike_sa_manager->checkin_and_destroy(charon->ike_sa_manager,
														ike_sa);
		}
		else
		{
			charon->ike_sa_manager->checkin(charon->ike_sa_manager, ike_sa);
		}
	}
	return JOB_REQUEUE_NONE;
}

METHOD(job_t, get_priority, job_priority_t,
	private_process_message_job_t *this)
{
	switch (this->message->get_exchange_type(this->message))
	{
		case IKE_AUTH:
			/* IKE auth is rather expensive and often blocking, low priority */
		case AGGRESSIVE:
		case ID_PROT:
			/* AM is basically IKE_SA_INIT/IKE_AUTH combined (without EAP/XAuth)
			 * MM is similar, but stretched out more */
			return JOB_PRIO_LOW;
		case INFORMATIONAL:
		case INFORMATIONAL_V1:
			/* INFORMATIONALs are inexpensive, for DPD we should have low
			 * reaction times */
			return JOB_PRIO_HIGH;
		case IKE_SA_INIT:
			/* IKE_SA_INIT is expensive, but we will drop them in the receiver
			 * if we are overloaded */
		case CREATE_CHILD_SA:
		case QUICK_MODE:
			/* these may require DH, but if not they are relatively cheap */
		case TRANSACTION:
			/* these are mostly cheap, however, if XAuth via RADIUS is used
			 * they may block */
		default:
			return JOB_PRIO_MEDIUM;
	}
}

/*
 * Described in header
 */
process_message_job_t *process_message_job_create(message_t *message)
{
	private_process_message_job_t *this;

	INIT(this,
		.public = {
			.job_interface = {
				.execute = _execute,
				.get_priority = _get_priority,
				.destroy = _destroy,
			},
		},
		.message = message,
	);

	return &(this->public);
}
