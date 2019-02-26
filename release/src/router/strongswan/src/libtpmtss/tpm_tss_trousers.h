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
 * @defgroup tpm_tss_trousers tpm_tss_trousers
 * @{ @ingroup libtpmtss
 */

#ifndef TPM_TSS_TROUSERS_H_
#define TPM_TSS_TROUSERS_H_

#include "tpm_tss.h"

typedef struct tpm_tss_trousers_t tpm_tss_trousers_t;

/**
 * TPM 1.2 access via TrouSerS public interface
 */
struct tpm_tss_trousers_t {

	tpm_tss_t public;

	/**
	 * Load AIK public and private key pair and save it under an object handle
	 *
	 * @param blob		encrypted AIK private key
	 * @param pubkey	AIK public key
	 * @param handle	object handle under which the AIK key is stored
	 */
	void (*load_aik)(tpm_tss_trousers_t *this, chunk_t blob, chunk_t pubkey,
					 uint32_t handle);

};

/**
 * Create a tpm_tss_trousers instance.
 */
tpm_tss_t *tpm_tss_trousers_create(void);

#endif /** TPM_TSS_TROUSERS_H_ @}*/
