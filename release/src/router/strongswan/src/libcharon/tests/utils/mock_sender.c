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

#include "mock_sender.h"

#include <collections/linked_list.h>

typedef struct private_mock_sender_t private_mock_sender_t;

/**
 * Private data
 */
struct private_mock_sender_t {

	/**
	 * Public interface
	 */
	mock_sender_t public;

	/**
	 * Packet queue, as message_t*
	 */
	linked_list_t *queue;
};


METHOD(sender_t, send_, void,
	private_mock_sender_t *this, packet_t *packet)
{
	message_t *message;

	message = message_create_from_packet(packet);
	message->parse_header(message);
	this->queue->insert_last(this->queue, message);
}

METHOD(mock_sender_t, dequeue, message_t*,
	private_mock_sender_t *this)
{
	message_t *message = NULL;

	this->queue->remove_first(this->queue, (void**)&message);
	return message;
}

METHOD(sender_t, destroy, void,
	private_mock_sender_t *this)
{
	this->queue->destroy_offset(this->queue, offsetof(message_t, destroy));
	free(this);
}

/*
 * Described in header
 */
mock_sender_t *mock_sender_create()
{
	private_mock_sender_t *this;

	INIT(this,
		.public = {
			.interface = {
				.send = _send_,
				.send_no_marker = (void*)nop,
				.flush = (void*)nop,
				.destroy = _destroy,
			},
			.dequeue = _dequeue,
		},
		.queue = linked_list_create(),
	);
	return &this->public;
}
