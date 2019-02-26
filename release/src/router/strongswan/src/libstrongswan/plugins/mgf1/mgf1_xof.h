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
 * @defgroup mgf1_xof mgf1_xof
 * @{ @ingroup crypto
 */

#ifndef MGF1_XOF_H_
#define MGF1_XOF_H_

typedef struct mgf1_xof_t mgf1_xof_t;

#include <crypto/xofs/mgf1.h>

/**
 * Implements the PKCS#1 MGF1_XOF Mask Generation Function based on a hash
 * function defined in section 10.2.1 of RFC 2437
 */
struct mgf1_xof_t {

	/**
	 * mgf1_t interface for this Extended Output Function (XOF).
	 */
	mgf1_t mgf1_interface;
};

/**
 * Create an mgf1_xof_t object
 *
 * @param algorithm		XOF_MGF1_SHA1, XOF_MGF1_SHA256 or XOF_MGF1_SHA512
 * @return				mgf1_xof_t object, NULL if not supported
 */
mgf1_xof_t *mgf1_xof_create(ext_out_function_t algorithm);

#endif /** MGF1_XOF_H_ @}*/

