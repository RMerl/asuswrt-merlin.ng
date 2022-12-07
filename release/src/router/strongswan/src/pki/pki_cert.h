/*
 * Copyright (C) 2022 Andreas Steffen, strongSec GmbH
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
 * @defgroup pki_cert pki_cert
 * @{ @ingroup pki
 */

#ifndef _PKI_CERT
#define _PKI_CERT

#include <library.h>
#include <credentials/sets/mem_cred.h>

/**
 * Extract X.509 CA [and SCEP RA] certificates from PKCS#7 container
 * check trust as well as validity and write to files
 */
bool pki_cert_extract_cacerts(chunk_t data, char *caout, char *raout,
                              bool is_scep, cred_encoding_type_t form,
                              bool force);

/**
 * Extract an X.509 client certificates from PKCS#7 container
 * check trust as well as validity and write to stdout
 */
bool pki_cert_extract_cert(chunk_t data, cred_encoding_type_t form);

#endif /** PKI_CERT_H_ @}*/
