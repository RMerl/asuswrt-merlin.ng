/*
 * Copyright (C) 2010 Andreas Steffen
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
 * @defgroup eap_ttls_avp eap_ttls_avp
 * @{ @ingroup eap_ttls
 */

#ifndef EAP_TTLS_AVP_H_
#define EAP_TTLS_AVP_H_

typedef struct eap_ttls_avp_t eap_ttls_avp_t;

#include <library.h>

#include <bio/bio_reader.h>
#include <bio/bio_writer.h>

/**
 * EAP-TTLS Attribute-Value Pair (AVP) handler.
 */
struct eap_ttls_avp_t {

	/**
	 * Process received EAP-TTLS EAP Message AVP.
	 *
	 * @param reader	TLS data buffer
	 * @param data		received EAP Message
	 * @return
	 *					- SUCCESS if AVP processing succeeded
	 *					- FAILED if AVP processing failed
	 *					- NEED_MORE if another invocation of process/build needed
	 */
	status_t (*process)(eap_ttls_avp_t *this, bio_reader_t *reader,
						chunk_t *data);

	/**
	 * Build EAP-TTLS EAP Message AVP to send out.
	 *
	 * @param writer	TLS data buffer to write to
	 * @param data		EAP Message to send
	 */
	void (*build)(eap_ttls_avp_t *this, bio_writer_t *writer, chunk_t data);

	/**
	 * Destroy a eap_ttls_application_t.
	 */
	void (*destroy)(eap_ttls_avp_t *this);
};

/**
 * Create an eap_ttls_avp instance.
 */
eap_ttls_avp_t *eap_ttls_avp_create(void);

#endif /** EAP_TTLS_AVP_H_ @}*/
