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
 * @defgroup ietf_attr_installed_packagest ietf_attr_installed_packages
 * @{ @ingroup ietf_attr
 */

#ifndef IETF_ATTR_INSTALLED_PACKAGES_H_
#define IETF_ATTR_INSTALLED_PACKAGES_H_

typedef struct ietf_attr_installed_packages_t ietf_attr_installed_packages_t;

#include "ietf_attr.h"
#include "pa_tnc/pa_tnc_attr.h"

#define IETF_INSTALLED_PACKAGES_MIN_SIZE	4

/**
 * Class implementing the IETF PA-TNC Installed Packages attribute.
 *
 */
struct ietf_attr_installed_packages_t {

	/**
	 * Public PA-TNC attribute interface
	 */
	pa_tnc_attr_t pa_tnc_attribute;

	/**
	 * Add a package entry
	 *
	 * @param name			package name
	 * @param version		package version number
	 */
	void (*add)(ietf_attr_installed_packages_t *this, chunk_t name,
													  chunk_t version);

	/**
	 * Enumerates over all packages
	 * Format:  chunk_t name, chunk_t version
	 *
	 * @return				enumerator
	 */
	enumerator_t* (*create_enumerator)(ietf_attr_installed_packages_t *this);

	/**
	 * Number of Installed Packages still missing
	 *
	 * @return				Number of missing installed packages
	 */
	uint16_t (*get_count)(ietf_attr_installed_packages_t *this);

	/**
	 * Remove all Installed Packages from list
	 */
	void (*clear_packages)(ietf_attr_installed_packages_t *this);

};

/**
 * Creates an ietf_attr_installed_packages_t object
 *
 */
pa_tnc_attr_t* ietf_attr_installed_packages_create(void);

/**
 * Creates an ietf_attr_installed_packages_t object from received data
 *
 * @param length			Total length of attribute value
 * @param value				Unparsed attribute value (might be a segment)
 */
pa_tnc_attr_t* ietf_attr_installed_packages_create_from_data(size_t length,
															 chunk_t value);

#endif /** IETF_ATTR_INSTALLED_PACKAGES_H_ @}*/
