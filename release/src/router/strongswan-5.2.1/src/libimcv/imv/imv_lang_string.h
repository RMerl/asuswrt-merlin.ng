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
 * @defgroup imv_lang_string_t imv_lang_string
 * @{ @ingroup libimcv_imv
 */

#ifndef IMV_LANG_STRING_H_
#define IMV_LANG_STRING_H_

#include <library.h>
#include <collections/enumerator.h>

typedef struct imv_lang_string_t imv_lang_string_t;

/**
 * Define a language string entry
 */
struct imv_lang_string_t {

	/**
	 * language code
	 */
	char *lang;

	/**
	 * UTF-8 string in the corresponding language
	 */
	char *string;

};

/**
 * Select the preferred language
 *
 * @param language_enumerator	enumerator over user preferred languages
 * @param languages				string array of available languages
 * @param lang_count			number of available languages
 * @return						selected language as a language code
 */
char* imv_lang_string_select_lang(enumerator_t *language_enumerator,
								  char* languages[], int lang_count);

/**
 * Select the preferred language string
 *
 * @param lang_string			multi-lingual array of strings
 * @param lang					language code of preferred language
 * @return						selected string
 */
char* imv_lang_string_select_string(imv_lang_string_t lang_string[], char *lang);

#endif /** IMV_LANG_STRING_H_ @}*/
