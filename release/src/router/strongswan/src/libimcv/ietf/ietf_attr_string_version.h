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
 * @defgroup ietf_attr_string_versiont ietf_attr_string_version
 * @{ @ingroup ietf_attr
 */

#ifndef IETF_ATTR_STRING_VERSION_H_
#define IETF_ATTR_STRING_VERSION_H_

typedef struct ietf_attr_string_version_t ietf_attr_string_version_t;

#include "ietf_attr.h"
#include "pa_tnc/pa_tnc_attr.h"


/**
 * Class implementing the IETF PA-TNC String Version attribute.
 *
 */
struct ietf_attr_string_version_t {

	/**
	 * Public PA-TNC attribute interface
	 */
	pa_tnc_attr_t pa_tnc_attribute;

	/**
	 * Gets the Product Version Number and optionally the Internal Build
	 * and Configuration Version Numbers
	 *
	 * @param build			Internal Build Number (if build != NULL)
	 * @param config		Configuration Version Number (if config != NULL)
	 * @return				Product Version Number
	 */
	chunk_t (*get_version)(ietf_attr_string_version_t *this, chunk_t *build,
															 chunk_t *config);
};

/**
 * Creates an ietf_attr_string_version_t object
 *
 */
pa_tnc_attr_t* ietf_attr_string_version_create(chunk_t version, chunk_t build,
											   chunk_t config);

/**
 * Creates an ietf_attr_string_version_t object from received data
 *
 * @param length			Total length of attribute value
 * @param value				Unparsed attribute value (might be a segment)
 */
pa_tnc_attr_t* ietf_attr_string_version_create_from_data(size_t length,
														 chunk_t value);

#endif /** IETF_ATTR_STRING_VERSION_H_ @}*/
