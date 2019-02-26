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
 * @defgroup imv_reason_string_t imv_reason_string
 * @{ @ingroup libimcv_imv
 */

#ifndef IMV_REASON_STRING_H_
#define IMV_REASON_STRING_H_

#include "imv_lang_string.h"

#include <library.h>
#include <collections/linked_list.h>

typedef struct imv_reason_string_t imv_reason_string_t;

/**
 * Defines and builds a TNC Reason String
 */
struct imv_reason_string_t {

	/**
	 * Add an individual remediation instruction to the string
	 *
	 * @param reason		Multi-lingual reason string
	 */
	 void (*add_reason)(imv_reason_string_t *this, imv_lang_string_t reason[]);

	/**
	 * Gets  encoding of the reason string
	 *
	 * @return				TNC reason string
	 */
	chunk_t (*get_encoding)(imv_reason_string_t *this);

	/**
	 * Destroys an imv_reason_string_t object
	 */
	void (*destroy)(imv_reason_string_t *this);
};

/**
 * Creates an Reason String object
 *
 * @param lang				Preferred language
 * @param separator			String separating multiple reasons
 */
 imv_reason_string_t* imv_reason_string_create(char *lang, char *separator);

#endif /** IMV_REASON_STRING_H_ @}*/
