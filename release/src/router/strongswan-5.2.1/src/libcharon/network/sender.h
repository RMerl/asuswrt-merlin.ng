/*
 * Copyright (C) 2012 Tobias Brunner
 * Copyright (C) 2005-2007 Martin Willi
 * Copyright (C) 2005 Jan Hutter
 * Hochschule fuer Technik Rapperswil
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
 * @defgroup sender sender
 * @{ @ingroup network
 */

#ifndef SENDER_H_
#define SENDER_H_

typedef struct sender_t sender_t;

#include <library.h>
#include <networking/packet.h>

/**
 * Callback job responsible for sending IKE packets over the socket.
 */
struct sender_t {

	/**
	 * Send a packet over the network.
	 *
	 * This function is non blocking and adds the packet to a queue.
	 * Whenever the sender thread thinks it's good to send the packet,
	 * it'll do so.
	 *
	 * @param packet	packet to send
	 */
	void (*send) (sender_t *this, packet_t *packet);

	/**
	 * The same as send() but does not add Non-ESP markers automatically.
	 *
	 * @param packet	packet to send
	 */
	void (*send_no_marker) (sender_t *this, packet_t *packet);

	/**
	 * Enforce a flush of the send queue.
	 *
	 * This function blocks until all queued packets have been sent.
	 */
	void (*flush)(sender_t *this);

	/**
	 * Destroys a sender object.
	 */
	void (*destroy) (sender_t *this);
};

/**
 * Create the sender thread.
 *
 * The thread will start to work, getting packets
 * from its queue and sends them out.
 *
 * @return		created sender object
 */
sender_t * sender_create(void);

#endif /** SENDER_H_ @}*/
