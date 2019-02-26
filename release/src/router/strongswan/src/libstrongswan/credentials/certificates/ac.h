/*
 * Copyright (C) 2002-2009 Andreas Steffen
 *
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
 * @defgroup ac ac
 * @{ @ingroup certificates
 */

#ifndef AC_H_
#define AC_H_

#include <library.h>
#include <credentials/certificates/certificate.h>

typedef struct ac_t ac_t;
typedef enum ac_group_type_t ac_group_type_t;

/**
 * Common group types, from IETF Attributes Syntax
 */
enum ac_group_type_t {
	AC_GROUP_TYPE_OCTETS,
	AC_GROUP_TYPE_STRING,
	AC_GROUP_TYPE_OID,
};

/**
 * X.509 attribute certificate interface.
 *
 * This interface adds additional methods to the certificate_t type to
 * allow further operations on these certificates.
 */
struct ac_t {

	/**
	 * Implements the certificate_t interface
	 */
	certificate_t certificate;

	/**
	 * Get the attribute certificate's serial number.
	 *
	 * @return			chunk pointing to serialNumber
	 */
	chunk_t (*get_serial)(ac_t *this);

	/**
	 * Get the serial number of the holder certificate.
	 *
	 * @return			chunk pointing to serialNumber
	 */
	chunk_t (*get_holderSerial)(ac_t *this);

	/**
	 * Get the issuer of the holder certificate.
	 *
	 * @return			holderIssuer as identification_t*
	 */
	identification_t* (*get_holderIssuer)(ac_t *this);

	/**
	 * Get the authorityKeyIdentifier.
	 *
	 * @return			authKeyIdentifier as chunk_t, to internal data
	 */
	chunk_t (*get_authKeyIdentifier)(ac_t *this);

	/**
	 * Create an enumerator of contained Group memberships.
	 *
	 * @return			enumerator over (ac_group_type_t, chunk_t)
	 */
	enumerator_t* (*create_group_enumerator)(ac_t *this);
};

#endif /** AC_H_ @}*/
