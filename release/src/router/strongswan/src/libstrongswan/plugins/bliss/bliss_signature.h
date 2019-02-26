/*
 * Copyright (C) 2014 Andreas Steffen
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
 * @defgroup bliss_signature bliss_signature
 * @{ @ingroup bliss_p
 */

#ifndef BLISS_SIGNATURE_H_
#define BLISS_SIGNATURE_H_

typedef struct bliss_signature_t bliss_signature_t;

#include "bliss_param_set.h"

#include <library.h>

/**
 * Public interface of BLISS signature object
 */
struct bliss_signature_t {

	/**
	 * Get compressed binary encoding of BLISS signature
	 *
	 * @result			binary encoding of BLISS signature
	 */
	chunk_t (*get_encoding)(bliss_signature_t *this);

	/**
	 * Get signature parameters extracted from compressd binary encoding
	 *
	 * @param z1		signature vector z1 of size n
	 * @param z2d		signature vector z2d of size n
	 * @param c_indices	indices of sparse binary challenge vector of size kappa
	 */
	void (*get_parameters)(bliss_signature_t *this, int32_t **z1, int16_t **z2d,
						   uint16_t **c_indices);

	/**
	 * Destroy bliss_signature_t object
	 */
	void (*destroy)(bliss_signature_t *this);

};

/**
 * Create a BLISS signature object.
 *
 * @param set			BLISS parameter set
 */
bliss_signature_t *bliss_signature_create(const bliss_param_set_t *set);

/**
 * Create a BLISS signature object from encoding.
 *
 * @param set			BLISS parameter set
 * @param encoding		binary signature encoding
 */
bliss_signature_t *bliss_signature_create_from_data(const bliss_param_set_t *set,
													chunk_t encoding);

#endif /** BLISS_SIGNATURE_H_ @}*/
