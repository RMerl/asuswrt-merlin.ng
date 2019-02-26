/*
 * Copyright (C) 2016 Andreas Steffen
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
 * @defgroup mgf1 mgf1
 * @{ @ingroup crypto
 */

#ifndef MGF1_H_
#define MGF1_H_

typedef struct mgf1_t mgf1_t;

#include "xof.h"

/**
 * Implements the PKCS#1 MGF1 Mask Generation Function based on a hash function
 * defined in section 10.2.1 of RFC 2437
 */
struct mgf1_t {

	/**
	 * Generic xof_t interface for this Extended Output Function (XOF).
	 */
	xof_t xof_interface;

	/**
	 * Hash the seed before using it as a seed for MGF1
	 * 
	 * @param yes		TRUE if seed has to be hashed first
	 */
	void (*set_hash_seed)(mgf1_t *this, bool yes);
};

#endif /** MGF1_H_ @}*/
