/*
 * Copyright (C) 2012-2014 Tobias Brunner
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
 * @defgroup ip_packet ip_packet
 * @{ @ingroup libipsec
 */

#ifndef IP_PACKET_H_
#define IP_PACKET_H_

#include <library.h>
#include <networking/host.h>
#include <networking/packet.h>

typedef struct ip_packet_t ip_packet_t;

/**
 *  IP packet
 */
struct ip_packet_t {

	/**
	 * IP version of this packet
	 *
	 * @return				ip version
	 */
	uint8_t (*get_version)(ip_packet_t *this);

	/**
	 * Get the source address of this packet
	 *
	 * @return				source host
	 */
	host_t *(*get_source)(ip_packet_t *this);

	/**
	 * Get the destination address of this packet
	 *
	 * @return				destination host
	 */
	host_t *(*get_destination)(ip_packet_t *this);

	/**
	 * Get the protocol (IPv4) or next header (IPv6) field of this packet.
	 *
	 * @return				protocol|next header field
	 */
	uint8_t (*get_next_header)(ip_packet_t *this);

	/**
	 * Get the complete IP packet (including the header)
	 *
	 * @return				IP packet (internal data)
	 */
	chunk_t (*get_encoding)(ip_packet_t *this);

	/**
	 * Get only the payload
	 *
	 * @return				IP payload (internal data)
	 */
	chunk_t (*get_payload)(ip_packet_t *this);

	/**
	 * Clone the IP packet
	 *
	 * @return				clone of the packet
	 */
	ip_packet_t *(*clone)(ip_packet_t *this);

	/**
	 * Destroy an ip_packet_t
	 */
	void (*destroy)(ip_packet_t *this);

};

/**
 * Create an IP packet out of data from the wire (or decapsulated from another
 * packet).
 *
 * @note The raw IP packet gets either owned by the new object, or destroyed,
 * if the data is invalid.
 *
 * @param packet		the IP packet (including header), gets owned
 * @return				ip_packet_t instance, or NULL if invalid
 */
ip_packet_t *ip_packet_create(chunk_t packet);

/**
 * Encode an IP packet from the given data.
 *
 * If src and/or dst have ports set they are applied to UDP/TCP headers found
 * in the packet.
 *
 * @param src			source address and optional port (cloned)
 * @param dst			destination address and optional port (cloned)
 * @param next_header	the protocol (IPv4) or next header (IPv6)
 * @param data			complete data after basic IP header (cloned)
 * @return				ip_packet_t instance, or NULL if invalid
 */
ip_packet_t *ip_packet_create_from_data(host_t *src, host_t *dst,
										uint8_t next_header, chunk_t data);

/**
 * Encode a UDP packet from the given data.
 *
 * @param src			source address and port (cloned)
 * @param dst			destination address and port (cloned)
 * @param data			UDP data (cloned)
 * @return				ip_packet_t instance, or NULL if invalid
 */
ip_packet_t *ip_packet_create_udp_from_data(host_t *src, host_t *dst,
											chunk_t data);

#endif /** IP_PACKET_H_ @}*/
