/* SPDX-License-Identifier: GPL-2.0 */
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <linux/if.h>

#include "color.h"
#include "utils.h"

static void set_color_palette(void);

enum color {
	C_RED,
	C_GREEN,
	C_YELLOW,
	C_BLUE,
	C_MAGENTA,
	C_CYAN,
	C_WHITE,
	C_BOLD_RED,
	C_BOLD_GREEN,
	C_BOLD_YELLOW,
	C_BOLD_BLUE,
	C_BOLD_MAGENTA,
	C_BOLD_CYAN,
	C_BOLD_WHITE,
	C_CLEAR
};

static const char * const color_codes[] = {
	"\e[31m",
	"\e[32m",
	"\e[33m",
	"\e[34m",
	"\e[35m",
	"\e[36m",
	"\e[37m",
	"\e[1;31m",
	"\e[1;32m",
	"\e[1;33m",
	"\e[1;34m",
	"\e[1;35m",
	"\e[1;36m",
	"\e[1;37m",
	"\e[0m",
	NULL,
};

/* light background */
static enum color attr_colors_light[] = {
	C_CYAN,
	C_YELLOW,
	C_MAGENTA,
	C_BLUE,
	C_GREEN,
	C_RED,
	C_CLEAR,
};

/* dark background */
static enum color attr_colors_dark[] = {
	C_BOLD_CYAN,
	C_BOLD_YELLOW,
	C_BOLD_MAGENTA,
	C_BOLD_BLUE,
	C_BOLD_GREEN,
	C_BOLD_RED,
	C_CLEAR
};

static int is_dark_bg;
static int color_is_enabled;

static void enable_color(void)
{
	color_is_enabled = 1;
	set_color_palette();
}

bool check_enable_color(int color, int json)
{
	if (json || color == COLOR_OPT_NEVER)
		return false;

	if (color == COLOR_OPT_ALWAYS || isatty(fileno(stdout))) {
		enable_color();
		return true;
	}
	return false;
}

bool matches_color(const char *arg, int *val)
{
	char *dup, *p;

	if (!val)
		return false;

	dup = strdupa(arg);
	p = strchrnul(dup, '=');
	if (*p)
		*(p++) = '\0';

	if (matches(dup, "-color"))
		return false;

	if (*p == '\0' || !strcmp(p, "always"))
		*val = COLOR_OPT_ALWAYS;
	else if (!strcmp(p, "auto"))
		*val = COLOR_OPT_AUTO;
	else if (!strcmp(p, "never"))
		*val = COLOR_OPT_NEVER;
	else
		return false;
	return true;
}

static void set_color_palette(void)
{
	char *p = getenv("COLORFGBG");

	/*
	 * COLORFGBG environment variable usually contains either two or three
	 * values separated by semicolons; we want the last value in either case.
	 * If this value is 0-6 or 8, background is dark.
	 */
	if (p && (p = strrchr(p, ';')) != NULL
		&& ((p[1] >= '0' && p[1] <= '6') || p[1] == '8')
		&& p[2] == '\0')
		is_dark_bg = 1;
}

__attribute__((format(printf, 3, 4)))
int color_fprintf(FILE *fp, enum color_attr attr, const char *fmt, ...)
{
	int ret = 0;
	va_list args;

	va_start(args, fmt);

	if (!color_is_enabled || attr == COLOR_NONE) {
		ret = vfprintf(fp, fmt, args);
		goto end;
	}

	ret += fprintf(fp, "%s", color_codes[is_dark_bg ?
		attr_colors_dark[attr] : attr_colors_light[attr]]);

	ret += vfprintf(fp, fmt, args);
	ret += fprintf(fp, "%s", color_codes[C_CLEAR]);

end:
	va_end(args);
	return ret;
}

enum color_attr ifa_family_color(__u8 ifa_family)
{
	switch (ifa_family) {
	case AF_INET:
		return COLOR_INET;
	case AF_INET6:
		return COLOR_INET6;
	default:
		return COLOR_NONE;
	}
}

enum color_attr oper_state_color(__u8 state)
{
	switch (state) {
	case IF_OPER_UP:
		return COLOR_OPERSTATE_UP;
	case IF_OPER_DOWN:
		return COLOR_OPERSTATE_DOWN;
	default:
		return COLOR_NONE;
	}
}
