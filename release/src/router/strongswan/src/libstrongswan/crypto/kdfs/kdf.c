/*
 * Copyright (C) 2022 Tobias Brunner
 *
 * Copyright (C) secunet Security Networks AG
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

#include "kdf.h"

ENUM(key_derivation_function_names, KDF_UNDEFINED, KDF_PRF_PLUS,
	"KDF_UNDEFINED",
	"KDF_PRF",
	"KDF_PRF_PLUS",
);

/*
 * Described in header
 */
bool kdf_has_fixed_output_length(key_derivation_function_t type)
{
	switch (type)
	{
		case KDF_PRF:
			return TRUE;
		case KDF_PRF_PLUS:
		case KDF_UNDEFINED:
			break;
	}
	return FALSE;
}
