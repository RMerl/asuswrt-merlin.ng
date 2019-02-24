/*
 * Copyright (C) 2017 Andreas Steffen
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
 * @defgroup swid_gen swid_gen
 * @{ @ingroup libimcv
 */

#ifndef SWID_GEN_H_
#define SWID_GEN_H_

#include <library.h>

typedef struct swid_gen_t swid_gen_t;

/**
 * Class generating a either a full or a minimalistic ISO 19770-2:2015 SWID tag
 */
struct swid_gen_t {

	/**
	 * Generate a SWID tag
	 *
	 * @param sw_id 		Software identifier
	 * @param package		Package name (can be NULL)
	 * @param version		Package version (can be NULL)
	 * @param full			Generate full SWID tags with file information
	 * @param pretty		Generate SWID tags with pretty formatting
	 * @return				SWID tag
	 */
	char* (*generate_tag)(swid_gen_t *this, char *sw_id, char *package,
						  char *version, bool full, bool pretty);

	/**
	 * Generate SWID tags or software identifiers for all installed packages
	 *
	 * @param sw_id_only 	Return software identifier only
	 * @param full			Generate full SWID tags with file information
	 * @param pretty		Generate SWID tags with pretty formatting
	 * @return				Tag enumerator (sw_id, tag)
	 */
	enumerator_t* (*create_tag_enumerator)(swid_gen_t *this, bool sw_id_only,
										   bool full, bool pretty);

	/**
	 * Destroys a swid_gen_t object.
	 */
	void (*destroy)(swid_gen_t *this);

};

/**
 * Creates a swid_gen_t object
 */
swid_gen_t* swid_gen_create(void);

#endif /** SWID_GEN_H_ @}*/
