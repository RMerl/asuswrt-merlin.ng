/*
 * Copyright (C) 2011 Andreas Steffen
 * Copyright (C) 2011 HSR Hochschule fuer Technik Rapperswil
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
 * @defgroup eap_peap_avp eap_peap_avp
 * @{ @ingroup eap_peap
 */

#ifndef EAP_PEAP_AVP_H_
#define EAP_PEAP_AVP_H_

typedef struct eap_peap_avp_t eap_peap_avp_t;

#include <library.h>

#include <bio/bio_reader.h>
#include <bio/bio_writer.h>

/**
 * EAP-PEAP Attribute-Value Pair (AVP) handler.
 */
struct eap_peap_avp_t {

	/**
	 * Process received EAP-PEAP Message AVP.
	 *
	 * @param reader		TLS data buffer
	 * @param data			received EAP Message
	 * @param identifier	EAP-PEAP message identifier
	 * @return
	 *					- SUCCESS if AVP processing succeeded
	 *					- FAILED if AVP processing failed
	 *					- NEED_MORE if another invocation of process/build needed
	 */
	status_t (*process)(eap_peap_avp_t *this, bio_reader_t *reader,
						chunk_t *data, u_int8_t identifier);

	/**
	 * Build EAP-PEAP Message AVP to send out.
	 *
	 * @param writer		TLS data buffer to write to
	 * @param data			EAP Message to send
	 */
	void (*build)(eap_peap_avp_t *this, bio_writer_t *writer, chunk_t data);

	/**
	 * Destroy a eap_peap_application_t.
	 */
	void (*destroy)(eap_peap_avp_t *this);
};

/**
 * Create an eap_peap_avp instance.
 *
 * @param is_server		TRUE iv eap server, FALSE if eap peer
 */
eap_peap_avp_t *eap_peap_avp_create(bool is_server);

#endif /** EAP_PEAP_AVP_H_ @}*/
