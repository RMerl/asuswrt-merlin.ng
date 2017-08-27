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

#include <stdarg.h>

#include "debug.h"

ENUM(debug_names, DBG_DMN, DBG_LIB,
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
);

ENUM(debug_lower_names, DBG_DMN, DBG_LIB,
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
);

/**
 * level logged by the default logger
 */
static level_t default_level = 1;

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
	if (level <= default_level)
	{
		va_list args;

		va_start(args, fmt);
		vfprintf(default_stream, fmt, args);
		fprintf(default_stream, "\n");
		va_end(args);
	}
}

/**
 * set the level logged by the default stderr logger
 */
void dbg_default_set_level(level_t level)
{
	default_level = level;
}

/**
 * set the stream logged by dbg_default() to
 */
void dbg_default_set_stream(FILE *stream)
{
	default_stream = stream;
}

/**
 * The registered debug hook.
 */
void (*dbg) (debug_t group, level_t level, char *fmt, ...) = dbg_default;

