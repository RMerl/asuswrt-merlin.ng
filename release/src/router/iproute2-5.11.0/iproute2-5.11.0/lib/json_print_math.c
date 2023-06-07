// SPDX-License-Identifier: GPL-2.0+

#include <stdarg.h>
#include <stdio.h>
#include <math.h>

#include "utils.h"
#include "json_print.h"

char *sprint_size(__u32 sz, char *buf)
{
	long kilo = 1024;
	long mega = kilo * kilo;
	size_t len = SPRINT_BSIZE - 1;
	double tmp = sz;

	if (sz >= mega && fabs(mega * rint(tmp / mega) - sz) < 1024)
		snprintf(buf, len, "%gMb", rint(tmp / mega));
	else if (sz >= kilo && fabs(kilo * rint(tmp / kilo) - sz) < 16)
		snprintf(buf, len, "%gKb", rint(tmp / kilo));
	else
		snprintf(buf, len, "%ub", sz);

	return buf;
}

int print_color_size(enum output_type type, enum color_attr color,
		     const char *key, const char *fmt, __u32 sz)
{
	SPRINT_BUF(buf);

	if (_IS_JSON_CONTEXT(type))
		return print_color_uint(type, color, key, "%u", sz);

	sprint_size(sz, buf);
	return print_color_string(type, color, key, fmt, buf);
}
