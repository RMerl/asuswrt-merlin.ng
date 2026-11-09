/*
 * Copyright (C) 2008-2025 Tobias Brunner
 * Copyright (C) 2005-2008 Martin Willi
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

#include <utils/utils.h>

#include <unistd.h>
#include <stdio.h>

ENUM(tty_color_names, TTY_RESET, TTY_BG_DEF,
	"\e[0m",
	"\e[1m",
	"\e[4m",
	"\e[5m",
	"\e[30m",
	"\e[31m",
	"\e[32m",
	"\e[33m",
	"\e[34m",
	"\e[35m",
	"\e[36m",
	"\e[37m",
	"\e[39m",
	"\e[40m",
	"\e[41m",
	"\e[42m",
	"\e[43m",
	"\e[44m",
	"\e[45m",
	"\e[46m",
	"\e[47m",
	"\e[49m",
);

/**
 * Check if the output goes to stdout/stderr in a CI environment, where colored
 * output is usually supported.
 */
static bool is_ci(int fd)
{
	static bool ci_checked = FALSE, ci_found = FALSE;

	if (!ci_checked)
	{
		ci_checked = TRUE;
		ci_found = getenv("CI") && (getenv("GITHUB_ACTIONS") ||
									getenv("CIRRUS_CI") ||
									getenv("APPVEYOR"));
	}
	return ci_found && (fd == fileno(stdout) || fd == fileno(stderr));
}

/**
 * Get the escape string for a given TTY color, empty string on non-tty FILE
 */
char* tty_escape_get(int fd, tty_escape_t escape)
{
	if (!isatty(fd) && !is_ci(fd))
	{
		return "";
	}
	switch (escape)
	{
		case TTY_RESET:
		case TTY_BOLD:
		case TTY_UNDERLINE:
		case TTY_BLINKING:
#ifdef WIN32
			return "";
#endif
		case TTY_FG_BLACK:
		case TTY_FG_RED:
		case TTY_FG_GREEN:
		case TTY_FG_YELLOW:
		case TTY_FG_BLUE:
		case TTY_FG_MAGENTA:
		case TTY_FG_CYAN:
		case TTY_FG_WHITE:
		case TTY_FG_DEF:
		case TTY_BG_BLACK:
		case TTY_BG_RED:
		case TTY_BG_GREEN:
		case TTY_BG_YELLOW:
		case TTY_BG_BLUE:
		case TTY_BG_MAGENTA:
		case TTY_BG_CYAN:
		case TTY_BG_WHITE:
		case TTY_BG_DEF:
			return enum_to_name(tty_color_names, escape);
		/* warn if a escape code is missing */
	}
	return "";
}
