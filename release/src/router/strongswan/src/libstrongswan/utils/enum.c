/*
 * Copyright (C) 2006 Martin Willi
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
	if (!e)
	{
		return NULL;
	}
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
 * Get the position of a flag name using offset calculation
 */
static int find_flag_pos(u_int val, u_int first)
{
	int offset = 0;

	while (val != 0x01)
	{
		val = val >> 1;
		offset++;
	}
	return first - offset;
}

/**
 * Described in header.
 */
char *enum_flags_to_string(enum_name_t *e, u_int val, char *buf, size_t len)
{
	char *pos = buf, *delim = "";
	int i, wr;

	if (e->next != ENUM_FLAG_MAGIC)
	{
		if (snprintf(buf, len, "(%d)", (int)val) >= len)
		{
			return NULL;
		}
		return buf;
	}

	if (snprintf(buf, len, "(unset)") >= len)
	{
		return NULL;
	}

	for (i = 0; val; i++)
	{
		u_int flag = 1 << i;

		if (val & flag)
		{
			char *name = NULL, hex[32];

			if (flag >= (u_int)e->first && flag <= (u_int)e->last)
			{
				name = e->names[find_flag_pos(e->first, i)];
			}
			else
			{
				snprintf(hex, sizeof(hex), "(0x%X)", flag);
				name = hex;
			}
			if (name)
			{
				wr = snprintf(pos, len, "%s%s", delim, name);
				if (wr >= len)
				{
					return NULL;
				}
				len -= wr;
				pos += wr;
				delim = " | ";
			}
			val &= ~flag;
		}
	}
	return buf;
}

/**
 * See header.
 */
int enum_printf_hook(printf_hook_data_t *data, printf_hook_spec_t *spec,
					 const void *const *args)
{
	enum_name_t *ed = *((enum_name_t**)(args[0]));
	int val = *((int*)(args[1]));
	char *name, buf[512];

	if (ed && ed->next == ENUM_FLAG_MAGIC)
	{
		name = enum_flags_to_string(ed, val, buf, sizeof(buf));
		if (name == NULL)
		{
			snprintf(buf, sizeof(buf), "(0x%X)", val);
			name = buf;
		}
	}
	else
	{
		name = enum_to_name(ed, val);
		if (name == NULL)
		{
			snprintf(buf, sizeof(buf), "(%d)", val);
			name = buf;
		}
	}
	if (spec->minus)
	{
		return print_in_hook(data, "%-*s", spec->width, name);
	}
	return print_in_hook(data, "%*s", spec->width, name);
}
