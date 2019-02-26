/*
 * Copyright (C) 2013 Reto Buerki
 * Copyright (C) 2013 Adrian-Ken Rueegsegger
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
 * @defgroup tkm-credential-enc credential encoder
 * @{ @ingroup tkm
 */

#ifndef TKM_ENCODER_H_
#define TKM_ENCODER_H_

#include <credentials/cred_encoding.h>

/**
 * Encoding function for TKM key fingerprints.
 */
bool tkm_encoder_encode(cred_encoding_type_t type, chunk_t *encoding,
						va_list args);

#endif /** TKM_ENCODER_H_ @}*/
