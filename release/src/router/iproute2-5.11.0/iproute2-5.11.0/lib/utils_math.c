// SPDX-License-Identifier: GPL-2.0+

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <asm/types.h>

#include "utils.h"

/* See http://physics.nist.gov/cuu/Units/binary.html */
static const struct rate_suffix {
	const char *name;
	double scale;
} suffixes[] = {
	{ "bit",	1. },
	{ "Kibit",	1024. },
	{ "kbit",	1000. },
	{ "mibit",	1024.*1024. },
	{ "mbit",	1000000. },
	{ "gibit",	1024.*1024.*1024. },
	{ "gbit",	1000000000. },
	{ "tibit",	1024.*1024.*1024.*1024. },
	{ "tbit",	1000000000000. },
	{ "Bps",	8. },
	{ "KiBps",	8.*1024. },
	{ "KBps",	8000. },
	{ "MiBps",	8.*1024*1024. },
	{ "MBps",	8000000. },
	{ "GiBps",	8.*1024.*1024.*1024. },
	{ "GBps",	8000000000. },
	{ "TiBps",	8.*1024.*1024.*1024.*1024. },
	{ "TBps",	8000000000000. },
	{ NULL }
};

int get_rate(unsigned int *rate, const char *str)
{
	char *p;
	double bps = strtod(str, &p);
	const struct rate_suffix *s;

	if (p == str)
		return -1;

	for (s = suffixes; s->name; ++s) {
		if (strcasecmp(s->name, p) == 0) {
			bps *= s->scale;
			p += strlen(p);
			break;
		}
	}

	if (*p)
		return -1; /* unknown suffix */

	bps /= 8; /* -> bytes per second */
	*rate = bps;
	/* detect if an overflow happened */
	if (*rate != floor(bps))
		return -1;
	return 0;
}

int get_rate64(__u64 *rate, const char *str)
{
	char *p;
	double bps = strtod(str, &p);
	const struct rate_suffix *s;

	if (p == str)
		return -1;

	for (s = suffixes; s->name; ++s) {
		if (strcasecmp(s->name, p) == 0) {
			bps *= s->scale;
			p += strlen(p);
			break;
		}
	}

	if (*p)
		return -1; /* unknown suffix */

	bps /= 8; /* -> bytes per second */
	*rate = bps;
	return 0;
}

int get_size(unsigned int *size, const char *str)
{
	double sz;
	char *p;

	sz = strtod(str, &p);
	if (p == str)
		return -1;

	if (*p) {
		if (strcasecmp(p, "kb") == 0 || strcasecmp(p, "k") == 0)
			sz *= 1024;
		else if (strcasecmp(p, "gb") == 0 || strcasecmp(p, "g") == 0)
			sz *= 1024*1024*1024;
		else if (strcasecmp(p, "gbit") == 0)
			sz *= 1024*1024*1024/8;
		else if (strcasecmp(p, "mb") == 0 || strcasecmp(p, "m") == 0)
			sz *= 1024*1024;
		else if (strcasecmp(p, "mbit") == 0)
			sz *= 1024*1024/8;
		else if (strcasecmp(p, "kbit") == 0)
			sz *= 1024/8;
		else if (strcasecmp(p, "b") != 0)
			return -1;
	}

	*size = sz;

	/* detect if an overflow happened */
	if (*size != floor(sz))
		return -1;

	return 0;
}
