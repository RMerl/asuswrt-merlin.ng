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

/**
 * sender_t implementation that does not pass the sent packet to a socket but
 * instead provides it for immediate delivery to an ike_sa_t object.
 *
 * @defgroup mock_sender mock_sender
 * @{ @ingroup test_utils_c
 */

#ifndef MOCK_SENDER_H_
#define MOCK_SENDER_H_

#include <encoding/message.h>
#include <network/sender.h>

typedef struct mock_sender_t mock_sender_t;

struct mock_sender_t {

	/**
	 * Implemented interface
	 */
	sender_t interface;

	/**
	 * Remove the next packet in the send queue as message_t object.  The IKE
	 * header is already parsed (which is assumed does not fail) so it can
	 * directly be passed to ike_sa_t::process_message().
	 *
	 * @return		message or NULL if none is queued
	 */
	message_t *(*dequeue)(mock_sender_t *this);
};

/**
 * Creates a mock_sender_t instance.
 *
 * @return			created object
 */
mock_sender_t *mock_sender_create();

#endif /** MOCK_SENDER_H_ @} */
