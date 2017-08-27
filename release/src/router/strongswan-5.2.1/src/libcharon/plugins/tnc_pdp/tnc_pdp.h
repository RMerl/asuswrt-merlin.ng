/*
 * Copyright (C) 2011 Andreas Steffen
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
 * @defgroup tnc_pdp_t tnc_pdp
 * @{ @ingroup tnc_pdp
 */

#ifndef TNC_PDP_H_
#define TNC_PDP_H_

typedef struct tnc_pdp_t tnc_pdp_t;

#include <library.h>

/**
 * Public interface of a TNC Policy Decision Point object
 */
struct tnc_pdp_t {

	/**
	 * implements plugin interface
	 */
	void (*destroy)(tnc_pdp_t *this);
};

/**
 * Create a TNC PDP instance
 */
tnc_pdp_t* tnc_pdp_create(void);

#endif /** TNC_PDP_PLUGIN_H_ @}*/
