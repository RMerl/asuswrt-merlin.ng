/*
 * Copyright (C) 2009 Martin Willi
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
 * @defgroup pkcs1_encoder pkcs1_encoder
 * @{ @ingroup pkcs1
 */

#ifndef PKCS1_ENCODER_H_
#define PKCS1_ENCODER_H_

#include <credentials/cred_encoding.h>

/**
 * Encoding function for PKCS#1/ASN.1 fingerprints/key formats.
 */
bool pkcs1_encoder_encode(cred_encoding_type_t type, chunk_t *encoding,
						  va_list args);

#endif /** PKCS1_ENCODER_H_ @}*/
