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
 * @defgroup ita_attr_angel ita_attr_angel
 * @{ @ingroup ita_attr
 */

#ifndef ITA_ATTR_ANGEL_H_
#define ITA_ATTR_ANGEL_H_

typedef struct ita_attr_angel_t ita_attr_angel_t;

#include "pa_tnc/pa_tnc_attr.h"

/**
 * Class implementing the ITA Start/Stop Angel PA-TNC attribute.
 *
 */
struct ita_attr_angel_t {

	/**
	 * Public PA-TNC attribute interface
	 */
	pa_tnc_attr_t pa_tnc_attribute;

};

/**
 * Creates an ita_attr_angel_t object with an empty settings list
 *
 * @param start				TRUE for Start, FALSE for Stop Angel attribute
 */
pa_tnc_attr_t* ita_attr_angel_create(bool start);

/**
 * Creates an ita_attr_angel_t object from received data
 *
 * @param start				TRUE for Start, FALSE for Stop Angel attribute
 */
pa_tnc_attr_t* ita_attr_angel_create_from_data(bool start);

#endif /** ITA_ATTR_ANGEL_H_ @}*/
