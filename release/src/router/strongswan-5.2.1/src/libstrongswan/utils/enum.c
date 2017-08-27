/*
 * Copyright (C) 2006 Martin Willi
 * Hochschule fuer Technik Rapperswil
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

#include <stddef.h>
#include <stdio.h>

#include <library.h>
#include <utils/utils.h>

#include "enum.h"

/**
 * See header.
 */
char *enum_to_name(enum_name_t *e, int val)
{
	do
	{
		if (val >= e->first && val <= e->last)
		{
			return e->names[val - e->first];
		}
	}
	while ((e = e->next));
	return NULL;
}

/**
 * See header.
 */
bool enum_from_name_as_int(enum_name_t *e, const char *name, int *val)
{
	do
	{
		int i, count = e->last - e->first + 1;

		for (i = 0; i < count; i++)
		{
			if (name && strcaseeq(name, e->names[i]))
			{
				*val = e->first + i;
				return TRUE;
			}
		}
	}
	while ((e = e->next));
	return FALSE;
}

/**
 * Described in header.
 */
int enum_printf_hook(printf_hook_data_t *data, printf_hook_spec_t *spec,
					 const void *const *args)
{
	enum_name_t *ed = *((enum_name_t**)(args[0]));
	int val = *((int*)(args[1]));
	char *name, buf[32];

	name = enum_to_name(ed, val);
	if (name == NULL)
	{
		snprintf(buf, sizeof(buf), "(%d)", val);
		name = buf;
	}
	if (spec->minus)
	{
		return print_in_hook(data, "%-*s", spec->width, name);
	}
	return print_in_hook(data, "%*s", spec->width, name);
}
