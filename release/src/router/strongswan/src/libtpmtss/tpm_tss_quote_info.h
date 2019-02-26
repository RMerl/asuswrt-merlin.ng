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
 * @defgroup tpm_tss_quote_info tpm_tss_quote_info
 * @{ @ingroup libtpmtss
 */

#ifndef TPM_TSS_QUOTE_INFO_H_
#define TPM_TSS_QUOTE_INFO_H_

#include <library.h>

#include <crypto/hashers/hasher.h>

typedef enum tpm_quote_mode_t tpm_quote_mode_t;
typedef struct tpm_tss_quote_info_t tpm_tss_quote_info_t;
typedef struct tpm_tss_pcr_composite_t tpm_tss_pcr_composite_t;

/**
 * TPM Quote Modes
 */
enum tpm_quote_mode_t {
	TPM_QUOTE_NONE,
	TPM_QUOTE,
	TPM_QUOTE2,
	TPM_QUOTE2_VERSION_INFO,
	TPM_QUOTE_TPM2
};

struct tpm_tss_pcr_composite_t {

	/**
	 * Bit map of selected PCRs
	 */
	chunk_t pcr_select;

	/**
	 * Array of selected PCRs
	 */
	chunk_t pcr_composite;

};

/**
 * TPM Quote Information needed to verify the Quote Signature
 */
struct tpm_tss_quote_info_t {

	/**
	 * Get TPM Quote Mode
	 *
	 * @return				TPM Quote Mode
	 */
	tpm_quote_mode_t (*get_quote_mode)(tpm_tss_quote_info_t *this);

	/**
	 * Get PCR Composite digest algorithm
	 *
	 * @return					PCR Composite digest algorithm
	 */
	hash_algorithm_t (*get_pcr_digest_alg)(tpm_tss_quote_info_t *this);

	/**
	 * Get PCR Composite digest
	 *
	 * @return					PCR Composite digest
	 */
	chunk_t (*get_pcr_digest)(tpm_tss_quote_info_t *this);

	/**
	 * Get TPM Quote Info digest, the basis of the TPM Quote Singature
	 *
	 * @param nonce				Derived from the Diffie-Hellman exchange
	 * @param composite			PCR Composite as computed by IMV
	 * @param quoted			Encoded TPM Quote
	 * @return					TRUE if TPM Quote was successfully constructed
	 */
	bool (*get_quote)(tpm_tss_quote_info_t *this, chunk_t nonce,
					 		 tpm_tss_pcr_composite_t *composite,
							 chunk_t *quoted);

	/**
	 * Set TPM version info (needed for TPM 1.2)
	 *
	 * @param version_info		TPM 1.2 version info
	 */
	void (*set_version_info)(tpm_tss_quote_info_t *this, chunk_t version_info);

	/**
	 * Get TPM 2.0 version info (needed for TPM 2.0)
	 *
	 * @return					TPM 2.0 firmwareVersioin
	 */
	chunk_t (*get_version_info)(tpm_tss_quote_info_t *this);

	/**
	 * Set TPM 2.0 info parameters (needed for TPM 2.0)
	 *
	 * @param qualified_signer	TPM 2.0 qualifiedSigner
	 * @param clock_info		TPM 2.0 clockInfo
	 * @param pcr_select		TPM 2.0 pcrSelect
	 */
	void (*set_tpm2_info)(tpm_tss_quote_info_t *this, chunk_t qualified_signer,
						  chunk_t clock_info, chunk_t pcr_select);


	/**
	 * Get TPM 2.0 info parameters (needed for TPM 2.0)
	 *
	 * @param qualified_signer	TPM 2.0 qualifiedSigner
	 * @param clock_info		TPM 2.0 clockInfo
	 * @param pcr_select		TPM 2.0 pcrSelect
	 */
	void (*get_tpm2_info)(tpm_tss_quote_info_t *this, chunk_t *qualified_signer,
						  chunk_t *clock_info, chunk_t *pcr_select);

	/**
	 * Get reference to Quote Info object.
	 */
	tpm_tss_quote_info_t* (*get_ref)(tpm_tss_quote_info_t *this);

	/**
	 * Destroy a tpm_tss_quote_info_t.
	 */
	void (*destroy)(tpm_tss_quote_info_t *this);
};

/**
 * Create a tpm_tss_quote_info instance.
 *
 * @param quote_mode			TPM Quote mode
 * @param pcr_digest_alg		PCR Composite digest algorithm
 * @param pcr_digest			PCR Composite digest
 */
tpm_tss_quote_info_t *tpm_tss_quote_info_create(tpm_quote_mode_t quote_mode,
						hash_algorithm_t pcr_digest_alg, chunk_t pcr_digest);

#endif /** TPM_TSS_QUOTE_INFO_H_ @}*/
