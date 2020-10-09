/*
 * crypto_mode.c --- convert between encryption modes and strings
 *
 * Copyright (C) 1999  Theodore Ts'o <tytso@mit.edu>
 *
 * %Begin-Header%
 * This file may be redistributed under the terms of the GNU Library
 * General Public License, version 2.
 * %End-Header%
 */

#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include <errno.h>

#include "e2p.h"

struct mode {
	int		num;
	const char	*string;
};

static struct mode mode_list[] = {
	{	EXT4_ENCRYPTION_MODE_INVALID,		"Invalid"},
	{	EXT4_ENCRYPTION_MODE_AES_256_XTS,	"AES-256-XTS"},
	{	EXT4_ENCRYPTION_MODE_AES_256_GCM,	"AES-256-GCM"},
	{	EXT4_ENCRYPTION_MODE_AES_256_CBC,	"AES-256-CBC"},
	{	0, 0 },
};

const char *e2p_encmode2string(int num)
{
	struct mode  *p;
	static char buf[20];

	for (p = mode_list; p->string; p++) {
		if (num == p->num)
			return p->string;
	}
	sprintf(buf, "ENC_MODE_%d", num);
	return buf;
}

/*
 * Returns the hash algorithm, or -1 on error
 */
int e2p_string2encmode(char *string)
{
	struct mode	*p;
	char		*eptr;
	int		num;

	for (p = mode_list; p->string; p++) {
		if (!strcasecmp(string, p->string)) {
			return p->num;
		}
	}
	if (strncasecmp(string, "ENC_MODE_", 9))
		return -1;

	if (string[9] == 0)
		return -1;
	num = strtol(string+9, &eptr, 10);
	if (num > 255 || num < 0)
		return -1;
	if (*eptr)
		return -1;
	return num;
}

