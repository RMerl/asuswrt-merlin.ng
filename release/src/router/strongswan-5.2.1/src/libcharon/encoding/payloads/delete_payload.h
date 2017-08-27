/*
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
 * @defgroup delete_payload delete_payload
 * @{ @ingroup payloads
 */

#ifndef DELETE_PAYLOAD_H_
#define DELETE_PAYLOAD_H_

typedef struct delete_payload_t delete_payload_t;

#include <library.h>
#include <encoding/payloads/payload.h>
#include <encoding/payloads/proposal_substructure.h>

/**
 * Class representing an IKEv1 or a IKEv2 DELETE payload.
 */
struct delete_payload_t {

	/**
	 * The payload_t interface.
	 */
	payload_t payload_interface;

	/**
	 * Get the protocol ID.
	 *
	 * @return				protocol ID
	 */
	protocol_id_t (*get_protocol_id) (delete_payload_t *this);

	/**
	 * Add an SPI to the list of deleted SAs.
	 *
	 * @param spi			spi to add
	 */
	void (*add_spi) (delete_payload_t *this, u_int32_t spi);

	/**
	 * Set the IKE SPIs for an IKEv1 delete.
	 *
	 * @param spi_i			initiator SPI
	 * @param spi_r			responder SPI
	 */
	void (*set_ike_spi)(delete_payload_t *this, u_int64_t spi_i, u_int64_t spi_r);

	/**
	 * Get an enumerator over the SPIs in network order.
	 *
	 * @return				enumerator over SPIs, u_int32_t
	 */
	enumerator_t *(*create_spi_enumerator) (delete_payload_t *this);

	/**
	 * Destroys an delete_payload_t object.
	 */
	void (*destroy) (delete_payload_t *this);
};

/**
 * Creates an empty delete_payload_t object.
 *
 * @param type			PLV2_DELETE or PLV1_DELETE
 * @param protocol_id	protocol, such as AH|ESP
 * @return 				delete_payload_t object
 */
delete_payload_t *delete_payload_create(payload_type_t type,
										protocol_id_t protocol_id);

#endif /** DELETE_PAYLOAD_H_ @}*/
