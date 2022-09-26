/*
 * Copyright (C) 2020 Andreas Steffen
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
 * @defgroup ita_attr_sysmlinks ita_attr_sysmlinks
 * @{ @ingroup ita_attr
 */

#ifndef ITA_ATTR_SYMLINKS_H_
#define ITA_ATTR_SYMLINKS_H_

typedef struct ita_attr_symlinks_t ita_attr_symlinks_t;

#include "ita/ita_attr.h"
#include "pa_tnc/pa_tnc_attr.h"
#include "pts/pts_symlinks.h"

/**
 * Class implementing the ITA Symlinks attribute
 *
 */
struct ita_attr_symlinks_t {

	/**
	 * Public PA-TNC attribute interface
	 */
	pa_tnc_attr_t pa_tnc_attribute;

	/**
	 * Get list of symbolic links
	 *
	 * @return				List of symbolic links pointing to directories
	 */
	pts_symlinks_t* (*get_symlinks)(ita_attr_symlinks_t *this);

};

/**
 * Creates an ita_attr_sysmlinks_t object
 */
pa_tnc_attr_t* ita_attr_symlinks_create(pts_symlinks_t *symlinks);

/**
 * Creates an ita_attr_sysmlinks_t object from received data
 *
 * @param length			Total length of attribute value
 * @param value				Unparsed attribute value (might be a segment)
 */
pa_tnc_attr_t* ita_attr_symlinks_create_from_data(size_t length, chunk_t value);

#endif /** ITA_ATTR_SYMLINKS_H_ @}*/
