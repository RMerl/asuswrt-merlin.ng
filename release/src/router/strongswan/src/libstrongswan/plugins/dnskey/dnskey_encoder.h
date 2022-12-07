/*
 * Copyright (C) 2013 Andreas Steffen
 *
 * Copyright (C) secunet Security Networks AG
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
 * @defgroup dnskey_encoder dnskey_encoder
 * @{ @ingroup dnskey_p
 */

#ifndef DNSKEY_ENCODER_H_
#define DNSKEY_ENCODER_H_

#include <credentials/cred_encoding.h>

/**
 * Encoding function for DNSKEY (RFC 3110) public key format.
 */
bool dnskey_encoder_encode(cred_encoding_type_t type, chunk_t *encoding,
						  va_list args);

#endif /** DNSKEY_ENCODER_H_ @}*/
