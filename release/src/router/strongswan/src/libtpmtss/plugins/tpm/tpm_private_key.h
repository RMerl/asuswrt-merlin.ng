/*
 * Copyright (C) 2017 Andreas Steffen
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
 * @defgroup tpm_private_key tpm_private_key
 * @{ @ingroup tpm
 */

#ifndef TPM_PRIVATE_KEY_H_
#define TPM_PRIVATE_KEY_H_

typedef struct tpm_private_key_t tpm_private_key_t;

#include <credentials/builder.h>
#include <credentials/keys/private_key.h>

/**
 * Private Key implementation for the TPM 2.0 Trusted Platform Module
 */
struct tpm_private_key_t {

	/**
	 * Implements private_key_t interface.
	 */
	private_key_t key;
};

/**
 * Connect to a private key bound to the TPM
 *
 * @param type		type of the key
 * @param args		builder_part_t argument list
 * @return			loaded key, NULL on failure
 */
tpm_private_key_t *tpm_private_key_connect(key_type_t type, va_list args);

#endif /** tpm_PRIVATE_KEY_H_ @}*/
