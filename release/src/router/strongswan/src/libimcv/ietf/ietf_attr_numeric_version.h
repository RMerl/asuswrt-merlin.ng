/*
 * Copyright (C) 2012-2014 Andreas Steffen
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
 * @defgroup ietf_attr_numeric_versiont ietf_attr_numeric_version
 * @{ @ingroup ietf_attr
 */

#ifndef IETF_ATTR_NUMERIC_VERSION_H_
#define IETF_ATTR_NUMERIC_VERSION_H_

typedef struct ietf_attr_numeric_version_t ietf_attr_numeric_version_t;

#include "ietf_attr.h"
#include "pa_tnc/pa_tnc_attr.h"


/**
 * Class implementing the IETF PA-TNC String Version attribute.
 *
 */
struct ietf_attr_numeric_version_t {

	/**
	 * Public PA-TNC attribute interface
	 */
	pa_tnc_attr_t pa_tnc_attribute;

	/**
	 * Gets the Major and Minor Version Numbers
	 *
	 * @param major			Major Version Number
	 * @param minor			Minor Version Number
	 */
	void (*get_version)(ietf_attr_numeric_version_t *this,
						uint32_t *major, uint32_t *minor);

	/**
	 * Gets the Build Number
	 *
	 * @param major			Major Version Number
	 * @param minor			Minor Version Number
	 */
	uint32_t (*get_build)(ietf_attr_numeric_version_t *this);

	/**
	 * Gets the Major and Minor Numbers of the Service Pack
	 *
	 * @param major			Service Pack Major Number
	 * @param minor			Servcie Pack Minor Number
	 */
	void (*get_service_pack)(ietf_attr_numeric_version_t *this,
							 uint16_t *major, uint16_t *minor);
};

/**
 * Creates an ietf_attr_numeric_version_t object
 *
 */
pa_tnc_attr_t* ietf_attr_numeric_version_create(uint32_t major, uint32_t minor,
												uint32_t build,
												uint16_t service_pack_major,
												uint16_t service_pack_minor);

/**
 * Creates an ietf_attr_numeric_version_t object from received data
 *
 * @param length			Total length of attribute value
 * @param value				Unparsed attribute value (might be a segment)
 */
pa_tnc_attr_t* ietf_attr_numeric_version_create_from_data(size_t length,
														  chunk_t value);

#endif /** IETF_ATTR_NUMERIC_VERSION_H_ @}*/
