/*
 * Copyright (C) 2013 Andreas Steffen
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
 * @defgroup libtncif libtncif
 *
 * @addtogroup libtncif
 * TNC interface definitions
 *
 * @defgroup tnc_identities tnc_identities
 * @{ @ingroup libtncif
 */

#ifndef TNCIF_IDENTITY_H_
#define TNCIF_IDENTITY_H_

#include <library.h>

#include <pen/pen.h>
#include <bio/bio_reader.h>
#include <bio/bio_writer.h>

#define TNCIF_IDENTITY_MIN_SIZE			28

typedef struct tncif_identity_t tncif_identity_t;

/**
 * Public interface of a TNC Identity object
 */
struct tncif_identity_t {

	/**
	 * Get the TNC Identity Type
	 *
	 * @return					TNC Identity Type
	 */
	pen_type_t (*get_identity_type)(tncif_identity_t *this);

	/**
	 * Get the TNC Identity Value
	 *
	 * @return					TNC Identity Value
	 */
	chunk_t (*get_identity_value)(tncif_identity_t *this);

	/**
	 * Get the TNC Subject Type
	 *
	 * @return					TNC Subject Type
	 */
	pen_type_t (*get_subject_type)(tncif_identity_t *this);

	/**
	 * Get the TNC Authentication Type
	 *
	 * @return					TNC Authentication Type
	 */
	pen_type_t (*get_auth_type)(tncif_identity_t *this);

	/**
	 * Build the IF-IMV TNC Identity attribute encoding
	 *
	 * @param writer			writer to write encoded data to
	 */
	void (*build)(tncif_identity_t *this, bio_writer_t *writer);

	/**
	 * Process the IF-IMV TNC Identity attribute encoding
	 *
	 * @param reader			reader to read encoded data from
	 * @return					TRUE if successful
	 */
	bool (*process)(tncif_identity_t *this, bio_reader_t *reader);

	/**
	 * Destroys a tncif_identity_t object.
	 */
	void (*destroy)(tncif_identity_t *this);

};

/**
 * Create an empty TNC Identity object
 */
tncif_identity_t* tncif_identity_create_empty(void);

/**
 * Create an TNC Identity object from its components
 *
 * @param identity_type			TNC Identity Type
 * @param identity_value		TNC Identity Value (not cloned by constructor)
 * @param subject_type			TNC Subject Type
 * @param auth_type				TNC Authentication Type
 */
tncif_identity_t* tncif_identity_create(pen_type_t identity_type,
										chunk_t identity_value,
										pen_type_t subject_type,
										pen_type_t auth_type);

#endif /** TNCIF_IDENTITY_H_ @}*/
