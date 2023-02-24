/*
 * Copyright (C) 2006 Martin Willi
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

#include <stdarg.h>

#include "debug.h"

ENUM(debug_names, DBG_DMN, DBG_ANY,
	"DMN",
	"MGR",
	"IKE",
	"CHD",
	"JOB",
	"CFG",
	"KNL",
	"NET",
	"ASN",
	"ENC",
	"TNC",
	"IMC",
	"IMV",
	"PTS",
	"TLS",
	"APP",
	"ESP",
	"LIB",
	"ANY",
);

ENUM(debug_lower_names, DBG_DMN, DBG_ANY,
	"dmn",
	"mgr",
	"ike",
	"chd",
	"job",
	"cfg",
	"knl",
	"net",
	"asn",
	"enc",
	"tnc",
	"imc",
	"imv",
	"pts",
	"tls",
	"app",
	"esp",
	"lib",
	"any",
);

/**
 * level logged by the default logger for specific groups, to simplify things
 * we store level-1, so initialization to 0 is like setting it to 1
 */
static level_t default_level[DBG_MAX];

/**
 * stream logged to by the default logger
 */
static FILE *default_stream = NULL;

/**
 * default dbg function which printf all to stderr
 */
void dbg_default(debug_t group, level_t level, char *fmt, ...)
{
	if (!default_stream)
	{
		default_stream = stderr;
	}
	/* levels are stored as level-1 */
	if (level <= default_level[group]+1)
	{
		va_list args;

		va_start(args, fmt);
		vfprintf(default_stream, fmt, args);
		fprintf(default_stream, "\n");
		va_end(args);
	}
}

/*
 * Described in header
 */
void dbg_default_set_level_group(debug_t group, level_t level)
{
	if (group < DBG_ANY)
	{
		default_level[group] = level-1;
	}
	else
	{
		for (group = 0; group < DBG_MAX; group++)
		{
			default_level[group] = level-1;
		}
	}
}

/*
 * Described in header
 */
void dbg_default_set_level(level_t level)
{
	dbg_default_set_level_group(DBG_ANY, level);
}

/*
 * Described in header
 */
void dbg_default_set_stream(FILE *stream)
{
	default_stream = stream;
}

/**
 * The registered debug hook.
 */
void (*dbg) (debug_t group, level_t level, char *fmt, ...) = dbg_default;
