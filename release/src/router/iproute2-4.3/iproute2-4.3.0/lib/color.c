#include <stdio.h>
#include <stdarg.h>

#include "color.h"

enum color {
	C_RED,
	C_GREEN,
	C_YELLOW,
	C_BLUE,
	C_MAGENTA,
	C_CYAN,
	C_WHITE,
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
	"\e[0m",
	NULL,
};

static enum color attr_colors[] = {
	C_CYAN,
	C_YELLOW,
	C_MAGENTA,
	C_BLUE,
	C_GREEN,
	C_RED
};

static int color_is_enabled;

void enable_color(void)
{
	color_is_enabled = 1;
}

int color_fprintf(FILE *fp, enum color_attr attr, const char *fmt, ...)
{
	int ret = 0;
	va_list args;

	va_start(args, fmt);

	if (!color_is_enabled) {
		ret = vfprintf(fp, fmt, args);
		goto end;
	}

	ret += fprintf(fp, "%s", color_codes[attr_colors[attr]]);
	ret += vfprintf(fp, fmt, args);
	ret += fprintf(fp, "%s", color_codes[C_CLEAR]);

end:
	va_end(args);
	return ret;
}
