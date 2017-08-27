/*
 * Copyright (C) 2012 Andreas Steffen
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
 * @defgroup ietf_attr_default_pwd_enabled ietf_attr_default_pwd_enabled
 * @{ @ingroup ietf_attr
 */

#ifndef IETF_ATTR_PWD_ENABLED_H_
#define IETF_ATTR_PWD_ENABLED_H_

typedef struct ietf_attr_default_pwd_enabled_t ietf_attr_default_pwd_enabled_t;

#include "ietf_attr.h"
#include "pa_tnc/pa_tnc_attr.h"

/**
 * Class implementing the IETF PA-TNC Factory Default Password Enabled attribute.
 *
 */
struct ietf_attr_default_pwd_enabled_t {

	/**
	 * Public PA-TNC attribute interface
	 */
	pa_tnc_attr_t pa_tnc_attribute;

	/**
	 * Gets the Factory Default Password Enabled status
	 *
	 * @return				Factory Default Password Enabled status
	 */
	bool (*get_status)(ietf_attr_default_pwd_enabled_t *this);

};

/**
 * Creates an ietf_attr_default_pwd_enabled_t object
 *
 * @param status			Factory Default Password Enabled status
 */
pa_tnc_attr_t* ietf_attr_default_pwd_enabled_create(bool status);

/**
 * Creates an ietf_attr_default_pwd_enabled_t object from received data
 *
 * @param length			Total length of attribute value
 * @param value				Unparsed attribute value (might be a segment)
 */
pa_tnc_attr_t* ietf_attr_default_pwd_enabled_create_from_data(size_t length,
															  chunk_t value);

#endif /** IETF_ATTR_PWD_ENABLED_H_ @}*/
