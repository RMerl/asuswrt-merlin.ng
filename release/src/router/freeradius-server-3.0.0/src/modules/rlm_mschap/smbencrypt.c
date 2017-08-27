/*
 * smbencrypt.c	Produces LM-Password and NT-Password from
 *		cleartext password
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 *
 * Copyright 2002  3APA3A for FreeRADIUS project
   Copyright 2006  The FreeRADIUS server project
 */

RCSID("$Id$")

#include	<freeradius-devel/libradius.h>
#include	<freeradius-devel/md4.h>
#include	<freeradius-devel/md5.h>
#include	<freeradius-devel/sha1.h>
#include	<ctype.h>


#include	"smbdes.h"

static char const * hex = "0123456789ABCDEF";

/*
 *	FIXME: use functions in freeradius
 */
static void tohex (unsigned char const  *src, size_t len, char *dst)
{
	size_t i;
	for (i=0; i<len; i++) {
		dst[(i*2)] = hex[(src[i] >> 4)];
		dst[(i*2) + 1] = hex[(src[i]&0x0F)];
	}
	dst[(i*2)] = 0;
}

static void ntpwdhash(uint8_t *out, char const *password)
{
	ssize_t len;
	uint8_t ucs2_password[512];

	len = fr_utf8_to_ucs2(ucs2_password, sizeof(ucs2_password), password, strlen(password));
	if (len < 0) {
		*out = '\0';
		return;
	}
	fr_md4_calc(out, (uint8_t *) ucs2_password, len);
}

int main (int argc, char *argv[])
{
	int i, l;
	char password[1024];
	uint8_t hash[16];
	char ntpass[33];
	char lmpass[33];

	fprintf(stderr, "LM Hash			 \tNT Hash\n");
	fprintf(stderr, "--------------------------------\t--------------------------------\n");
	fflush(stderr);
	for (i = 1; i < argc; i++ ) {
		strlcpy(password, argv[i], sizeof(password));
		l = strlen(password);
		if (l && password[l-1] == '\n') password [l-1] = 0;
		smbdes_lmpwdhash(password, hash);
		tohex (hash, 16, lmpass);
		ntpwdhash (hash, password);
		tohex (hash, 16, ntpass);
		printf("%s\t%s\n", lmpass, ntpass);
	}
	return 0;
}
