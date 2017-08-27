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

#include "imv_lang_string.h"

#include <utils/debug.h>

/**
 * Described in header.
 */
char* imv_lang_string_select_lang(enumerator_t *language_enumerator,
								  char* languages[], int lang_count)
{
	bool match = FALSE;
	char *lang;
	int i, i_chosen = 0;

	while (language_enumerator->enumerate(language_enumerator, &lang))
	{
		for (i = 0; i < lang_count; i++)
		{
			if (streq(lang, languages[i]))
			{
				match = TRUE;
				i_chosen = i;
				break;
			}
		}
		if (match)
		{
			break;
		}
	}
	return languages[i_chosen];
}

/**
 * Described in header.
 */
char* imv_lang_string_select_string(imv_lang_string_t lang_string[], char *lang)
{
	char *string;
	int i = 0;

	if (!lang_string)
	{
		return NULL;
	}

	string = lang_string[0].string;
	while (lang_string[i].lang)
	{
		if (streq(lang, lang_string[i].lang))
		{
			string = lang_string[i].string;
			break;
		}
		i++;
	}
	return string;
}
