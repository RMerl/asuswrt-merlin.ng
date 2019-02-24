/*
 * Copyright (C) 2012 Tobias Brunner
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

/**
 * @defgroup receiver receiver
 * @{ @ingroup network
 */

#ifndef RECEIVER_H_
#define RECEIVER_H_

typedef struct receiver_t receiver_t;

#include <library.h>
#include <networking/host.h>
#include <networking/packet.h>

/**
 * Callback called for any received UDP encapsulated ESP packet.
 *
 * Implementation should be quick as the receiver doesn't receive any packets
 * while calling this function.
 *
 * @param data			data supplied during registration of the callback
 * @param packet		decapsulated ESP packet
 */
typedef void (*receiver_esp_cb_t)(void *data, packet_t *packet);

/**
 * Receives packets from the socket and adds them to the job queue.
 *
 * The receiver uses a callback job, which reads on the blocking socket.
 * A received packet is preparsed and a process_message_job is queued in the
 * job queue.
 *
 * To endure DoS attacks, cookies are enabled when too many IKE_SAs are half
 * open. The calculation of cookies is slightly different from the proposed
 * method in RFC4306. We do not include a nonce, because we think the advantage
 * we gain does not justify the overhead to parse the whole message.
 * Instead of VersionIdOfSecret, we include a timestamp. This allows us to
 * find out which key was used for cookie creation. Further, we can set a
 * lifetime for the cookie, which allows us to reuse the secret for a longer
 * time.
 *		 COOKIE = time | sha1( IPi | SPIi | time | secret )
 *
 * The secret is changed after a certain amount of cookies sent. The old
 * secret is stored to allow a clean migration between secret changes.
 *
 * Further, the number of half-initiated IKE_SAs is limited per peer. This
 * makes it impossible for a peer to flood the server with its real IP address.
 */
struct receiver_t {

	/**
	 * Register a callback which is called for any incoming ESP packets.
	 *
	 * @note Only the last callback registered will receive any packets.
	 *
	 * @param callback		callback to register
	 * @param data			data provided to callback
	 */
	void (*add_esp_cb)(receiver_t *this, receiver_esp_cb_t callback,
					   void *data);

	/**
	 * Unregister a previously registered callback for ESP packets.
	 *
	 * @param callback		previously registered callback
	 */
	void (*del_esp_cb)(receiver_t *this, receiver_esp_cb_t callback);

	/**
	 * Destroys a receiver_t object.
	 */
	void (*destroy)(receiver_t *this);
};

/**
 * Create a receiver_t object.
 *
 * The receiver thread will start working, get data
 * from the socket and add those packets to the job queue.
 *
 * @return	receiver_t object, NULL if initialization fails
 */
receiver_t * receiver_create(void);

#endif /** RECEIVER_H_ @}*/
