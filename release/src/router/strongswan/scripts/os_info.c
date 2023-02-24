/*
 * Copyright (C) 2021 Andreas Steffen
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

#include <stdio.h>

#include <library.h>
#include <utils/debug.h>
#include <imc/imc_os_info.h>

/**
 * Define debug level
 */
static level_t dbg_level = 1;

static void dbg_os_info(debug_t group, level_t level, char *fmt, ...)
{
	if ((level <= dbg_level) || level <= 1)
	{
		va_list args;

		va_start(args, fmt);
		vfprintf(stderr, fmt, args);
		fprintf(stderr, "\n");
		va_end(args);
	}
}

int main(int argc, char *argv[])
{
	imc_os_info_t *os_info;
	bool all;

	all = argc > 1;

	library_init(NULL, "os_info");
	atexit(library_deinit);

	dbg = dbg_os_info;

	os_info = imc_os_info_create(all);
	os_info->destroy(os_info);

	return 0;
}
