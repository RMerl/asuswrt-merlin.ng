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
 * @defgroup pem_encoder pem_encoder
 * @{ @ingroup pem_p
 */

#ifndef PEM_ENCODER_H_
#define PEM_ENCODER_H_

#include <credentials/cred_encoding.h>

/**
 * Encoding from ASN.1 to PEM format.
 */
bool pem_encoder_encode(cred_encoding_type_t type, chunk_t *encoding,
						va_list args);

#endif /** PEM_ENCODER_H_ @}*/

