/*
 * encoding.c --- convert between encoding magic numbers and strings
 *
 * Copyright (C) 2018  Collabora Ltd.
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
#include <ctype.h>
#include <errno.h>
#include <stdio.h>

#include "e2p.h"

#define ARRAY_SIZE(array)			\
        (sizeof(array) / sizeof(array[0]))

static const struct {
	const char *name;
	__u16 encoding_magic;
	__u16 default_flags;

} ext4_encoding_map[] = {
	{
		.encoding_magic = EXT4_ENC_UTF8_12_1,
		.name = "utf8-12.1",
		.default_flags = 0,
	},
	{
		.encoding_magic = EXT4_ENC_UTF8_12_1,
		.name = "utf8",
		.default_flags = 0,
	},
};

static const struct enc_flags {
	__u16 flag;
	const char *param;
} encoding_flags[] = {
	{ EXT4_ENC_STRICT_MODE_FL, "strict" },
};

/* Return a positive number < 0xff indicating the encoding magic number
 * or a negative value indicating error. */
int e2p_str2encoding(const char *string)
{
	unsigned int i;

	for (i = 0 ; i < ARRAY_SIZE(ext4_encoding_map); i++)
		if (!strcmp(string, ext4_encoding_map[i].name))
			return ext4_encoding_map[i].encoding_magic;

	return -EINVAL;
}

/* Return the name of an encoding or NULL */
const char *e2p_encoding2str(int encoding)
{
	unsigned int i;
	static char buf[32];

	for (i = 0 ; i < ARRAY_SIZE(ext4_encoding_map); i++)
		if (ext4_encoding_map[i].encoding_magic == encoding)
			return ext4_encoding_map[i].name;
	sprintf(buf, "UNKNOWN_ENCODING_%d", encoding);
	return buf;
}

int e2p_get_encoding_flags(int encoding)
{
	unsigned int i;

	for (i = 0 ; i < ARRAY_SIZE(ext4_encoding_map); i++)
		if (ext4_encoding_map[i].encoding_magic == encoding)
			return ext4_encoding_map[i].default_flags;

	return 0;
}

int e2p_str2encoding_flags(int encoding, char *param, __u16 *flags)
{
	char *f = strtok(param, "-");
	const struct enc_flags *fl;
	unsigned int i, neg = 0;

	if (encoding != EXT4_ENC_UTF8_12_1)
		return -EINVAL;
	while (f) {
		neg = 0;
		if (!strncmp("no", f, 2)) {
			neg = 1;
			f += 2;
		}

		for (i = 0; i < ARRAY_SIZE(encoding_flags); i++) {
			fl = &encoding_flags[i];
			if (!strcmp(fl->param, f)) {
				if (neg)
					*flags &= ~fl->flag;
				else
					*flags |= fl->flag;

				goto next_flag;
			}
		}
		return -EINVAL;
	next_flag:
		f = strtok(NULL, "-");
	}
	return 0;
}
