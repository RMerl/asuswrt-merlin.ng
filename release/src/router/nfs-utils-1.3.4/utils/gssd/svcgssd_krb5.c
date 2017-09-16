/*
 * COPYRIGHT (c) 2011
 * The Regents of the University of Michigan
 * ALL RIGHTS RESERVED
 *
 * Permission is granted to use, copy, create derivative works
 * and redistribute this software and such derivative works
 * for any purpose, so long as the name of The University of
 * Michigan is not used in any advertising or publicity
 * pertaining to the use of distribution of this software
 * without specific, written prior authorization.  If the
 * above copyright notice or any other identification of the
 * University of Michigan is included in any copy of any
 * portion of this software, then the disclaimer below must
 * also be included.
 *
 * THIS SOFTWARE IS PROVIDED AS IS, WITHOUT REPRESENTATION
 * FROM THE UNIVERSITY OF MICHIGAN AS TO ITS FITNESS FOR ANY
 * PURPOSE, AND WITHOUT WARRANTY BY THE UNIVERSITY OF
 * MICHIGAN OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING
 * WITHOUT LIMITATION THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. THE
 * REGENTS OF THE UNIVERSITY OF MICHIGAN SHALL NOT BE LIABLE
 * FOR ANY DAMAGES, INCLUDING SPECIAL, INDIRECT, INCIDENTAL, OR
 * CONSEQUENTIAL DAMAGES, WITH RESPECT TO ANY CLAIM ARISING
 * OUT OF OR IN CONNECTION WITH THE USE OF THE SOFTWARE, EVEN
 * IF IT HAS BEEN OR IS HEREAFTER ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGES.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif	/* HAVE_CONFIG_H */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <gssapi/gssapi.h>
#include <krb5.h>

#include "gss_util.h"
#include "gss_oids.h"
#include "err_util.h"
#include "svcgssd_krb5.h"
#include "../mount/version.h"

#define MYBUFLEN 1024

char *supported_enctypes_filename = "/proc/fs/nfsd/supported_krb5_enctypes";
int parsed_num_enctypes = 0;
krb5_enctype *parsed_enctypes = NULL;
char *cached_enctypes = NULL;

/*==========================*/
/*===  Internal routines ===*/
/*==========================*/

/*
 * Parse the supported encryption type information
 */
static int
parse_enctypes(char *enctypes)
{
	int n = 0;
	char *curr, *comma;
	int i;

	/* Don't parse the same string over and over... */
	if (cached_enctypes && strcmp(cached_enctypes, enctypes) == 0)
		return 0;

	/* Free any existing cached_enctypes */
	free(cached_enctypes);

	if (parsed_enctypes != NULL) {
		free(parsed_enctypes);
		parsed_enctypes = NULL;
		parsed_num_enctypes = 0;
	}

	/* count the number of commas */
	for (curr = enctypes; curr && *curr != '\0'; curr = ++comma) {
		comma = strchr(curr, ',');
		if (comma != NULL)
			n++;
		else
			break;
	}

	/* If no more commas and we're not at the end, there's one more value */
	if (*curr != '\0')
		n++;

	/* Empty string, return an error */
	if (n == 0)
		return ENOENT;

	/* Skip pass any non digits */
	while (*enctypes && isdigit(*enctypes) == 0)
		enctypes++;
	if (*enctypes == '\0')
		return EINVAL;

	/* Allocate space for enctypes array */
	if ((parsed_enctypes = (int *) calloc(n, sizeof(int))) == NULL) {
		return ENOMEM;
	}

	/* Now parse each value into the array */
	for (curr = enctypes, i = 0; curr && *curr != '\0'; curr = ++comma) {
		parsed_enctypes[i++] = atoi(curr);
		comma = strchr(curr, ',');
		if (comma == NULL)
			break;
	}

	parsed_num_enctypes = n;
	if ((cached_enctypes = malloc(strlen(enctypes)+1)))
		strcpy(cached_enctypes, enctypes);

	return 0;
}

static void
get_kernel_supported_enctypes(void)
{
	FILE *s_e;
	int ret;
	char buffer[MYBUFLEN + 1];

	memset(buffer, '\0', sizeof(buffer));

	s_e = fopen(supported_enctypes_filename, "r");
	if (s_e == NULL)
		goto out_clean_parsed;

	ret = fread(buffer, 1, MYBUFLEN, s_e);
	if (ret < 0) {
		fclose(s_e);
		goto out_clean_parsed;
	}
	fclose(s_e);
	if (parse_enctypes(buffer)) {
		goto out_clean_parsed;
	}
out:
	return;

out_clean_parsed:
	if (parsed_enctypes != NULL) {
		free(parsed_enctypes);
		parsed_num_enctypes = 0;
	}
	goto out;
}

/*==========================*/
/*===  External routines ===*/
/*==========================*/

/*
 * Get encryption types supported by the kernel, and then
 * call gss_krb5_set_allowable_enctypes() to limit the
 * encryption types negotiated.
 *
 * Returns:
 *	0 => all went well
 *     -1 => there was an error
 */

int
svcgssd_limit_krb5_enctypes(void)
{
#ifdef HAVE_SET_ALLOWABLE_ENCTYPES
	u_int maj_stat, min_stat;
	krb5_enctype old_kernel_enctypes[] = {
		ENCTYPE_DES_CBC_CRC,
		ENCTYPE_DES_CBC_MD5,
		ENCTYPE_DES_CBC_MD4 };
	krb5_enctype new_kernel_enctypes[] = {
		ENCTYPE_AES256_CTS_HMAC_SHA1_96,
		ENCTYPE_AES128_CTS_HMAC_SHA1_96,
		ENCTYPE_DES3_CBC_SHA1,
		ENCTYPE_ARCFOUR_HMAC,
		ENCTYPE_DES_CBC_CRC,
		ENCTYPE_DES_CBC_MD5,
		ENCTYPE_DES_CBC_MD4 };
	krb5_enctype *default_enctypes, *enctypes;
	int default_num_enctypes, num_enctypes;


	if (linux_version_code() < MAKE_VERSION(2, 6, 35)) {
		default_enctypes = old_kernel_enctypes;
		default_num_enctypes =
			sizeof(old_kernel_enctypes) / sizeof(old_kernel_enctypes[0]);
	} else {
		default_enctypes = new_kernel_enctypes;
		default_num_enctypes =
			sizeof(new_kernel_enctypes) / sizeof(new_kernel_enctypes[0]);
	}

	get_kernel_supported_enctypes();

	if (parsed_enctypes != NULL) {
		enctypes = parsed_enctypes;
		num_enctypes = parsed_num_enctypes;
		printerr(2, "%s: Calling gss_set_allowable_enctypes with %d "
			"enctypes from the kernel\n", __func__, num_enctypes);
	} else {
		enctypes = default_enctypes;
		num_enctypes = default_num_enctypes;
		printerr(2, "%s: Calling gss_set_allowable_enctypes with %d "
			"enctypes from defaults\n", __func__, num_enctypes);
	}

	maj_stat = gss_set_allowable_enctypes(&min_stat, gssd_creds,
			&krb5oid, num_enctypes, enctypes);
	if (maj_stat != GSS_S_COMPLETE) {
		printerr(1, "WARNING: gss_set_allowable_enctypes failed\n");
		pgsserr("svcgssd_limit_krb5_enctypes: gss_set_allowable_enctypes",
			maj_stat, min_stat, &krb5oid);
		return -1;
	}
#endif
	return 0;
}
