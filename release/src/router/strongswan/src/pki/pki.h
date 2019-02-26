/*
 * Copyright (C) 2015-2017 Tobias Brunner
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
 * @defgroup pki pki
 *
 * @addtogroup pki
 * @{
 */

#ifndef PKI_H_
#define PKI_H_

#include "command.h"

#include <library.h>
#include <selectors/traffic_selector.h>
#include <credentials/keys/private_key.h>

/**
 * Convert a form string to a encoding type
 */
bool get_form(char *form, cred_encoding_type_t *enc, credential_type_t type);

/**
 * Calculate start/end lifetime for certificates.
 *
 * If both nbstr and nastr are given, span is ignored. Otherwise missing
 * arguments are calculated, or assumed to be now.
 *
 * @param format	strptime() format, NULL for default: %d.%m.%y %T
 * @param nbstr		string describing notBefore datetime, or NULL
 * @param nastr		string describing notAfter datetime, or NULL
 * @param span		lifetime span, from notBefore to notAfter
 * @param nb		calculated notBefore time
 * @param na		calculated notAfter time
 * @return			TRUE of nb/na calculated successfully
 */
bool calculate_lifetime(char *format, char *nbstr, char *nastr, time_t span,
						time_t *nb, time_t *na);

/**
 * Set output file mode appropriate for credential encoding form on Windows
 */
void set_file_mode(FILE *stream, cred_encoding_type_t enc);

/**
 * Determine the signature scheme and parameters for the given private key and
 * hash algorithm and whether to use PSS padding for RSA.
 *
 * @param private	private key
 * @param digest	hash algorithm (if HASH_UNKNOWN a default is determined
 *					based on the key)
 * @param pss		use PSS padding for RSA keys
 * @return			allocated signature scheme and parameters (NULL if none
 *					found)
 */
signature_params_t *get_signature_scheme(private_key_t *private,
										 hash_algorithm_t digest, bool pss);

/**
 * Create a traffic selector from a CIDR or range string.
 *
 * @param str		input string, either a.b.c.d/e or a.b.c.d-e.f.g.h
 * @return			traffic selector, NULL on error
 */
traffic_selector_t* parse_ts(char *str);

#endif /** PKI_H_ @}*/
