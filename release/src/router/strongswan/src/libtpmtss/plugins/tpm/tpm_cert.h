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
 * @defgroup tpm_cert tpm_cert
 * @{ @ingroup tpm
 */

#ifndef TPM_CERT_H_
#define TPM_CERT_H_

#include <credentials/certificates/certificate.h>

/**
 * Load a specific certificate from a TPM
 *
 * Requires a BUILD_PKCS11_KEYID argument, and optionally a BUILD_PKCS11_SLOT
 * to designate the NV storage hierarchy.
 *
 * @param type			certificate type, must be CERT_X509
 * @param args			variable argument list, containing BUILD_PKCS11_KEYID.
 * @return				loaded certificate, or NULL on failure
 */
certificate_t *tpm_cert_load(certificate_type_t type, va_list args);

#endif /** TPM_CERT_H_ @}*/
