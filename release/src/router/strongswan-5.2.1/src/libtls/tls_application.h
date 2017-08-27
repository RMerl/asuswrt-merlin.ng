/*
 * Copyright (C) 2010 Andreas Steffen
 * Copyright (C) 2010 HSR Hochschule fuer Technik Rapperswil
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
 * @defgroup tls_handshake tls_handshake
 * @{ @ingroup libtls
 */

#ifndef TLS_APPLICATION_H_
#define TLS_APPLICATION_H_

typedef struct tls_application_t tls_application_t;

#include <bio/bio_reader.h>
#include <bio/bio_writer.h>

/**
 * TLS application data interface.
 */
struct tls_application_t {

	/**
	 * Process received TLS application data.
	 *
	 * @param reader	TLS data buffer
	 * @return
	 *					- SUCCESS if application completed
	 *					- FAILED if application data processing failed
	 *					- NEED_MORE if another invocation of process/build needed
	 */
	status_t (*process)(tls_application_t *this, bio_reader_t *reader);

	/**
	 * Build TLS application data to send out.
	 *
	 * @param writer	TLS data buffer to write to
	 * @return
	 *					- SUCCESS if application completed
	 *					- FAILED if application data build failed
	 *					- NEED_MORE if more data ready for delivery
	 *					- INVALID_STATE if more input to process() required
	 */
	status_t (*build)(tls_application_t *this, bio_writer_t *writer);

	/**
	 * Destroy a tls_application_t.
	 */
	void (*destroy)(tls_application_t *this);
};

#endif /** TLS_APPLICATION_H_ @}*/
