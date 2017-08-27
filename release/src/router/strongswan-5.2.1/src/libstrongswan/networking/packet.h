/*
 * Copyright (C) 2012 Tobias Brunner
 * Copyright (C) 2005-2006 Martin Willi
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
 * @defgroup packet packet
 * @{ @ingroup networking
 */

#ifndef PACKET_H_
#define PACKET_H_

typedef struct packet_t packet_t;

#include <library.h>
#include <networking/host.h>

/**
 * Maximum packet size we handle by default
 */
#define PACKET_MAX_DEFAULT 10000

/**
 * Abstraction of an IP/UDP-Packet, contains data, sender and receiver.
 */
struct packet_t {

	/**
	 * Set the source address.
	 *
	 * @param source	address to set as source (gets owned)
	 */
	void (*set_source)(packet_t *packet, host_t *source);

	/**
	 * Set the destination address.
	 *
	 * @param source	address to set as destination (gets owned)
	 */
	void (*set_destination)(packet_t *packet, host_t *destination);

	/**
	 * Get the source address.
	 *
	 * @return			source address (internal data)
	 */
	host_t *(*get_source)(packet_t *packet);

	/**
	 * Get the destination address.
	 *
	 * @return			destination address (internal data)
	 */
	host_t *(*get_destination)(packet_t *packet);

	/**
	 * Get the data from the packet.
	 *
	 * @return			chunk containing the data (internal data)
	 */
	chunk_t (*get_data)(packet_t *packet);

	/**
	 * Set the data in the packet.
	 *
	 * @param data		chunk with data to set (gets owned)
	 */
	void (*set_data)(packet_t *packet, chunk_t data);

	/**
	 * Get the DiffServ Code Point set on this packet.
	 *
	 * @return			DSCP value
	 */
	u_int8_t (*get_dscp)(packet_t *this);

	/**
	 * Set the DiffServ Code Point to use on this packet.
	 *
	 * @param value		DSCP value
	 */
	void (*set_dscp)(packet_t *this, u_int8_t value);

	/**
	 * Increase the offset where the actual packet data starts.
	 *
	 * The total offset applies to future calls of get_data() and clone().
	 *
	 * @note The offset is reset to 0 when set_data() is called.
	 *
	 * @param bytes		the number of additional bytes to skip
	 */
	void (*skip_bytes)(packet_t *packet, size_t bytes);

	/**
	 * Clones a packet_t object.
	 *
	 * @note Data is cloned without skipped bytes.
	 *
	 * @param clone		clone of the packet
	 */
	packet_t* (*clone)(packet_t *packet);

	/**
	 * Destroy the packet, freeing contained data.
	 */
	void (*destroy)(packet_t *packet);
};

/**
 * Create an empty packet
 *
 * @return packet_t object
 */
packet_t *packet_create();

/**
 * Create a packet from the supplied data
 *
 * @param src			source address (gets owned)
 * @param dst			destination address (gets owned)
 * @param data			packet data (gets owned)
 * @return packet_t object
 */
packet_t *packet_create_from_data(host_t *src, host_t *dst, chunk_t data);

#endif /** PACKET_H_ @}*/
