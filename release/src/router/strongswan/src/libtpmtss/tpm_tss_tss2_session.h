/*
 * Copyright (C) 2021 Andreas Steffen
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
 * @defgroup tpm_tss_tss2_session tpm_tss_tss2_session
 * @{ @ingroup libtpmtss
 */

#ifndef TPM_TSS_TSS2_SESSION_H_
#define TPM_TSS_TSS2_SESSION_H_

#ifdef TSS_TSS2_V2

#include <library.h>

#include <tss2/tss2_sys.h>

typedef struct tpm_tss_tss2_session_t tpm_tss_tss2_session_t;

/**
 * public interface of TPM 2.0 TSS session object
 */
struct tpm_tss_tss2_session_t {

	/**
	 * Set TPM 2.0 TSS Command Authentications
	 *
	 * @return              TRUE if successful
	 */
	bool (*set_cmd_auths)(tpm_tss_tss2_session_t *this);

	/**
	 * Get TPM 2.0 TSS Response Authentications
	 *
	 * @return              TRUE if successful
	 */
	  bool (*get_rsp_auths)(tpm_tss_tss2_session_t *this);

	/**
	 * Destroy the TPM 2.0 TSS session object
	 */
	void (*destroy)(tpm_tss_tss2_session_t *this);

};

/**
 * Create a tpm_tss_tss2_session instance.
 *
 * @param ek_handle     endorsement key handle
 * @param public        public information on endorsement key
 * @param sys_context   TSS2 system context
 */
tpm_tss_tss2_session_t* tpm_tss_tss2_session_create(uint32_t ek_handle,
							TPM2B_PUBLIC *public, TSS2_SYS_CONTEXT *sys_context);

#endif /* TSS_TSS2_V2 */

#endif /** TPM_TSS_TSS2_SESSION_H_ @}*/
