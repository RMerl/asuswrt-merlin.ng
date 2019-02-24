/*
 * Copyright (C) 2006 Andreas Steffen
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

#ifndef _ARGS_H_
#define _ARGS_H_

#include "keywords.h"

bool assign_arg(kw_token_t token, kw_token_t first, char *key, char *value,
				void *base, bool *assigned);
void free_args(kw_token_t first, kw_token_t last, void *base);
bool cmp_args(kw_token_t first, kw_token_t last, void *base1, void *base2);

#endif /* _ARGS_H_ */

