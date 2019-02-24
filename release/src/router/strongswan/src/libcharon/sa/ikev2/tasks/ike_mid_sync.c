/*
 * Copyright (C) 2016 Tobias Brunner
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
/*
 * Copyright (C) 2016 Stephen J. Bevan
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

#include "ike_mid_sync.h"

#include <daemon.h>
#include <bio/bio_reader.h>
#include <bio/bio_writer.h>
#include <encoding/payloads/notify_payload.h>

typedef struct private_ike_mid_sync_t private_ike_mid_sync_t;

/**
 * Private members
 */
struct private_ike_mid_sync_t {

	/**
	 * Public methods and task_t interface.
	 */
	ike_mid_sync_t public;

	/**
	 * Assigned IKE_SA.
	 */
	ike_sa_t *ike_sa;

	/**
	 * Nonce sent by the peer and expected to be returned
	 */
	chunk_t nonce;

	/**
	 * Expected next sender message ID
	 */
	uint32_t send;

	/**
	 * Expected received message ID
	 */
	uint32_t recv;
};

/*
 * Encoding of IKEV2_MESSAGE_SYNC_ID notify, RFC 6311
 *
 *  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * | Next Payload  |C|  RESERVED   |         Payload Length        |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |Protocol ID(=0)| SPI Size (=0) |      Notify Message Type      |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |             Nonce Data                                        |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |             EXPECTED_SEND_REQ_MESSAGE_ID                      |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |             EXPECTED_RECV_REQ_MESSAGE_ID                      |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */

/*
 * RFC 6311 section 5.1
 *
 *  o  The peer MUST silently drop any received synchronization message
 *     if M1 is lower than or equal to the highest value it has seen from
 *     the cluster.  This includes any previous received synchronization
 *     messages.
 */
METHOD(task_t, pre_process, status_t,
	private_ike_mid_sync_t *this, message_t *message)
{
	notify_payload_t *notify;
	bio_reader_t *reader;
	chunk_t nonce;
	uint32_t resp;

	if (message->get_message_id(message) != 0)
	{	/* ignore the notify if it was contained in an INFORMATIONAL with
		 * unexpected message ID */
		return SUCCESS;
	}
	if (!this->ike_sa->supports_extension(this->ike_sa,
										  EXT_IKE_MESSAGE_ID_SYNC))
	{
		DBG1(DBG_ENC, "unexpected %N notify, ignored", notify_type_names,
			 IKEV2_MESSAGE_ID_SYNC);
		return FAILED;
	}
	notify = message->get_notify(message, IKEV2_MESSAGE_ID_SYNC);

	reader = bio_reader_create(notify->get_notification_data(notify));
	if (!reader->read_data(reader, 4, &nonce) ||
		!reader->read_uint32(reader, &this->send) ||
		!reader->read_uint32(reader, &this->recv))
	{
		reader->destroy(reader);
		DBG1(DBG_ENC, "received invalid %N notify",
			 notify_type_names, IKEV2_MESSAGE_ID_SYNC);
		return FAILED;
	}
	reader->destroy(reader);
	resp = this->ike_sa->get_message_id(this->ike_sa, FALSE);
	if (this->send < resp)
	{
		DBG1(DBG_ENC, "ignore %N notify with lower (%d) than expected (%d) "
			 "sender MID", notify_type_names, IKEV2_MESSAGE_ID_SYNC, this->send,
			 resp);
		return FAILED;
	}
	this->nonce = chunk_clone(nonce);
	return SUCCESS;
}

/**
 * Check if there are any active tasks, indicating that we already
 * used the currents message ID and are waiting for a response.
 */
static bool has_active_tasks(private_ike_mid_sync_t *this)
{
	enumerator_t *enumerator;
	task_t *task;
	bool active;

	enumerator = this->ike_sa->create_task_enumerator(this->ike_sa,
													  TASK_QUEUE_ACTIVE);
	active = enumerator->enumerate(enumerator, &task);
	enumerator->destroy(enumerator);
	return active;
}

/*
 * RFC 6311 section 5.1
 *
 *  o  M2 MUST be at least the higher of the received M1, and one more
 *     than the highest sender value received from the cluster.  This
 *     includes any previous received synchronization messages.
 *
 *  o  P2 MUST be the higher of the received P1 value, and one more than
 *     the highest sender value used by the peer.
 *
 * M1 is this->send, P1 is this->recv
 */
METHOD(task_t, process, status_t,
	private_ike_mid_sync_t *this, message_t *message)
{
	uint32_t resp, init, m2, p2;

	if (message->get_message_id(message) != 0)
	{	/* ignore the notify if it was contained in an INFORMATIONAL with
		 * unexpected message id */
		return SUCCESS;
	}
	resp = this->ike_sa->get_message_id(this->ike_sa, FALSE);
	m2 = max(this->send, resp);
	if (resp != m2)
	{
		this->ike_sa->set_message_id(this->ike_sa, FALSE, m2);
	}
	init = this->ike_sa->get_message_id(this->ike_sa, TRUE);
	p2 = max(this->recv, has_active_tasks(this) ? init + 1 : init);
	if (init != p2)
	{
		this->ike_sa->set_message_id(this->ike_sa, TRUE, p2);
	}
	DBG1(DBG_IKE, "responder requested MID sync: initiating %d[%d], "
		 "responding %d[%d]", p2, init, m2, resp);
	this->send = p2;
	this->recv = m2;
	return NEED_MORE;
}

METHOD(task_t, build, status_t,
	private_ike_mid_sync_t *this, message_t *message)
{
	bio_writer_t *writer;

	writer = bio_writer_create(12);
	writer->write_data(writer, this->nonce);
	writer->write_uint32(writer, this->send);
	writer->write_uint32(writer, this->recv);

	message->set_message_id(message, 0);
	message->add_notify(message, FALSE, IKEV2_MESSAGE_ID_SYNC,
						writer->get_buf(writer));

	writer->destroy(writer);
	return SUCCESS;
}

METHOD(task_t, get_type, task_type_t,
	private_ike_mid_sync_t *this)
{
	return TASK_IKE_MID_SYNC;
}

METHOD(task_t, migrate, void,
	private_ike_mid_sync_t *this, ike_sa_t *ike_sa)
{
	this->ike_sa = ike_sa;
	chunk_free(&this->nonce);
}

METHOD(task_t, destroy, void,
	private_ike_mid_sync_t *this)
{
	chunk_free(&this->nonce);
	free(this);
}

/*
 * Described in header.
 */
ike_mid_sync_t *ike_mid_sync_create(ike_sa_t *ike_sa)
{
	private_ike_mid_sync_t *this;

	INIT(this,
		.public = {
			.task = {
				.get_type = _get_type,
				.build = _build,
				.pre_process = _pre_process,
				.process = _process,
				.migrate = _migrate,
				.destroy = _destroy,
			},
		},
		.ike_sa = ike_sa,
	);
	return &this->public;
}
