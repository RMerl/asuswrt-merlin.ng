/*
 * Copyright (C) 2012 Tobias Brunner
 * Copyright (C) 2012 Giuliano Grassi
 * Copyright (C) 2012 Ralf Sager
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
 * @defgroup esp_packet esp_packet
 * @{ @ingroup libipsec
 */

#ifndef ESP_PACKET_H_
#define ESP_PACKET_H_

#include "ip_packet.h"
#include "esp_context.h"

#include <library.h>
#include <networking/host.h>
#include <networking/packet.h>

typedef struct esp_packet_t esp_packet_t;

/**
 *  ESP packet
 */
struct esp_packet_t {

	/**
	 * Implements packet_t interface to access the raw ESP packet
	 */
	packet_t packet;

	/**
	 * Get the source address of this packet
	 *
	 * @return				source host
	 */
	host_t *(*get_source)(esp_packet_t *this);

	/**
	 * Get the destination address of this packet
	 *
	 * @return				destination host
	 */
	host_t *(*get_destination)(esp_packet_t *this);

	/**
	 * Parse the packet header before decryption. Tries to read the SPI
	 * from the packet to find a corresponding SA.
	 *
	 * @param spi			parsed SPI, in network byte order
	 * @return				TRUE when successful, FALSE otherwise (e.g. when the
	 *						length of the packet is invalid)
	 */
	bool (*parse_header)(esp_packet_t *this, u_int32_t *spi);

	/**
	 * Authenticate and decrypt the packet. Also verifies the sequence number
	 * using the supplied ESP context and updates the anti-replay window.
	 *
	 * @param esp_context		ESP context of corresponding inbound IPsec SA
	 * @return					- SUCCESS if successfully authenticated,
	 *							  decrypted and parsed
	 *							- PARSE_ERROR if the length of the packet or the
	 *							  padding is invalid
	 *							- VERIFY_ERROR if the sequence number
	 *							  verification failed
	 *							- FAILED if the ICV (MAC) check or the actual
	 *							  decryption failed
	 */
	status_t (*decrypt)(esp_packet_t *this, esp_context_t *esp_context);

	/**
	 * Encapsulate and encrypt the packet. The sequence number will be generated
	 * using the supplied ESP context.
	 *
	 * @param esp_context		ESP context of corresponding outbound IPsec SA
	 * @param spi				SPI value to use, in network byte order
	 * @return					- SUCCESS if encrypted
	 *							- FAILED if sequence number cycled or any of the
	 *							  cryptographic functions failed
	 *							- NOT_FOUND if no suitable IV generator provided
	 */
	status_t (*encrypt)(esp_packet_t *this, esp_context_t *esp_context,
						u_int32_t spi);

	/**
	 * Get the next header field of a packet.
	 *
	 * @note Packet has to be in the decrypted state.
	 *
	 * @return					next header field
	 */
	u_int8_t (*get_next_header)(esp_packet_t *this);

	/**
	 * Get the plaintext payload of this packet.
	 *
	 * @return					plaintext payload (internal data),
	 *							NULL if not decrypted
	 */
	ip_packet_t *(*get_payload)(esp_packet_t *this);

	/**
	 * Extract the plaintext payload from this packet.
	 *
	 * @return					plaintext payload (has to be destroyed),
	 *							NULL if not decrypted
	 */
	ip_packet_t *(*extract_payload)(esp_packet_t *this);

	/**
	 * Destroy an esp_packet_t
	 */
	void (*destroy)(esp_packet_t *this);

};

/**
 * Create an ESP packet out of data from the wire.
 *
 * @param packet		the packet data as received, gets owned
 * @return				esp_packet_t instance
 */
esp_packet_t *esp_packet_create_from_packet(packet_t *packet);

/**
 * Create an ESP packet from a plaintext payload
 *
 * @param src			source address
 * @param dst			destination address
 * @param payload		plaintext payload, gets owned
 * @return				esp_packet_t instance
 */
esp_packet_t *esp_packet_create_from_payload(host_t *src, host_t *dst,
											 ip_packet_t *payload);

#endif /** ESP_PACKET_H_ @}*/

