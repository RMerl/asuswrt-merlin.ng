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
 *
 * @defgroup imv_remediation_string_t imv_remediation_string
 * @{ @ingroup libimcv_imv
 */

#ifndef IMV_REMEDIATION_STRING_H_
#define IMV_REMEDIATION_STRING_H_

#include "imv_lang_string.h"

#include <library.h>
#include <collections/linked_list.h>

typedef struct imv_remediation_string_t imv_remediation_string_t;

/**
 * Defines and builds an IETF Remediation Instructions String
 */
struct imv_remediation_string_t {

	/**
	 * Add an individual remediation instruction to the string
	 *
	 * @param title			instruction title
	 * @param description	instruction description
	 * @param itemsheader	optional items header or NULL
	 * @param items			optional items list or NULL
	 */
	 void (*add_instruction)(imv_remediation_string_t *this,
							 imv_lang_string_t title[],
							 imv_lang_string_t description[],
							 imv_lang_string_t itemsheader[],
							 linked_list_t *items);

	/**
	 * Gets the plaintext or XML encoding of the remediation instructions
	 *
	 * @return				remediation instructions string
	 */
	chunk_t (*get_encoding)(imv_remediation_string_t *this);

	/**
	 * Destroys an imv_remediation_string_t object
	 */
	void (*destroy)(imv_remediation_string_t *this);
};

/**
 * Creates an IETF Remediation Instructions String object
 *
 * @param as_xml			XML encoding if TRUE, plaintext otherwise
 * @param lang				Preferred language
 */
 imv_remediation_string_t* imv_remediation_string_create(bool as_xml, char *lang);

#endif /** IMV_REMEDIATION_STRING_H_ @}*/
