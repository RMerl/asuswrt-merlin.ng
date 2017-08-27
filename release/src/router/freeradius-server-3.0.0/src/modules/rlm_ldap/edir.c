/*
 *   This program is is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License, version 2 if the
 *   License as published by the Free Software Foundation.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 */

/**
 * $Id$
 * @file edir.c
 * @brief LDAP extension for reading eDirectory universal password.
 *
 * To contact Novell about this file by physical or electronic mail, you may
 * find current contact  information at www.novell.com.
 *
 * @copyright 2012 Olivier Beytrison <olivier@heliosnet.org>
 * @copyright 2012 Alan DeKok <aland@freeradius.org>
 * @copyright 2002-2004 Novell, Inc.
 */

RCSID("$Id$")

#include	<freeradius-devel/radiusd.h>
#include	<freeradius-devel/rad_assert.h>

#include <ldap.h>
#include <lber.h>
#include "ldap.h"

/* NMAS error codes */
#define NMAS_E_BASE	(-1600)

#define NMAS_SUCCESS	0

#define NMAS_E_FRAG_FAILURE		(NMAS_E_BASE-31)     /* -1631 0xFFFFF9A1 */
#define NMAS_E_BUFFER_OVERFLOW		(NMAS_E_BASE-33)     /* -1633 0xFFFFF99F */
#define NMAS_E_SYSTEM_RESOURCES		(NMAS_E_BASE-34)     /* -1634 0xFFFFF99E */
#define NMAS_E_INSUFFICIENT_MEMORY	(NMAS_E_BASE-35)     /* -1635 0xFFFFF99D */
#define NMAS_E_NOT_SUPPORTED		(NMAS_E_BASE-36)     /* -1636 0xFFFFF99C */
#define NMAS_E_INVALID_PARAMETER	(NMAS_E_BASE-43)     /* -1643 0xFFFFF995 */
#define NMAS_E_INVALID_VERSION		(NMAS_E_BASE-52)     /* -1652 0xFFFFF98C */

/* OID of LDAP extenstion calls to read Universal Password */
#define NMASLDAP_GET_PASSWORD_REQUEST     "2.16.840.1.113719.1.39.42.100.13"
#define NMASLDAP_GET_PASSWORD_RESPONSE    "2.16.840.1.113719.1.39.42.100.14"

#define NMAS_LDAP_EXT_VERSION 1

/** Takes the object DN and BER encodes the data into the BER value which is used as part of the request
 *
 @verbatim
	RequestBer contents:
		clientVersion		INTEGER
		targetObjectDN		OCTET STRING
 @endverbatim
 *
 * @param[out] request_bv where to write the request BER value (must be freed with ber_bvfree).
 * @param[in] dn to query for.
 * @return 0 on success, and < 0 on error.
 */
static int ber_encode_request_data(char const *dn, struct berval **request_bv)
{
	int err = 0;
	int rc = 0;
	BerElement *request_ber = NULL;

	if (!dn || !*dn) {
		err = NMAS_E_INVALID_PARAMETER;
		goto finish;
	}

	/* Allocate a BerElement for the request parameters.*/
	if ((request_ber = ber_alloc()) == NULL) {
		err = NMAS_E_FRAG_FAILURE;
		goto finish;
	}

	rc = ber_printf(request_ber, "{io}", NMAS_LDAP_EXT_VERSION, dn, strlen(dn) + 1);
	if (rc < 0) {
		err = NMAS_E_FRAG_FAILURE;
		goto finish;
	}

	/*
	 *	Convert the BER we just built to a berval that we'll
	 *	send with the extended request.
	 */
	if (ber_flatten(request_ber, request_bv) < 0) {
		err = NMAS_E_FRAG_FAILURE;
		goto finish;
	}

finish:
	if (request_ber) ber_free(request_ber, 1);

	return err;
}

/** Converts the reply into server version and a return code
 *
 * This function takes the reply BER Value and decodes the NMAS server version and return code and if a non
 * null retData buffer was supplied, tries to decode the the return data and length.
 *
 @verbatim
	ResponseBer contents:
		server_version		INTEGER
		error       		INTEGER
		data			OCTET STRING
 @endverbatim
 *
 * @param[in] reply_bv reply data from extended request.
 * @param[out] server_version that responded.
 * @param[out] out data.
 * @param[out] outlen Length of data written to out.
 * @return 0 on success, and < 0 on error.
 */
static int ber_decode_login_data(struct berval *reply_bv, int *server_version, void *out, size_t *outlen)
{
	int rc = 0;
	int err = 0;
	BerElement *reply_ber = NULL;

	rad_assert(out != NULL);
	rad_assert(outlen != NULL);

	if ((reply_ber = ber_init(reply_bv)) == NULL) {
		err = NMAS_E_SYSTEM_RESOURCES;
		goto finish;
	}

	rc = ber_scanf(reply_ber, "{iis}", server_version, &err, out, outlen);
	if (rc == -1) {
		err = NMAS_E_FRAG_FAILURE;
		goto finish;
	}

finish:

	if(reply_ber) ber_free(reply_ber, 1);

	return err;
}

/** Attempt to retrieve the universal password from Novell eDirectory
 *
 * @param[in] ld LDAP handle.
 * @param[in] dn of user we want to retrieve the password for.
 * @param[out] password Where to write the retrieved password.
 * @param[out] passlen Length of data written to the password buffer.
 * @return 0 on success and < 0 on failure.
 */
int nmasldap_get_password(LDAP *ld, char const *dn, char *password, size_t *passlen)
{
	int err = 0;
	struct berval *request_bv = NULL;
	char *reply_oid = NULL;
	struct berval *reply_bv = NULL;
	int server_version;
	size_t bufsize;
	char buffer[256];

	/* Validate  parameters. */
	if (!dn || !*dn || !passlen || !ld) {
		return NMAS_E_INVALID_PARAMETER;
	}

	err = ber_encode_request_data(dn, &request_bv);
	if (err) goto finish;

	/* Call the ldap_extended_operation (synchronously) */
	err = ldap_extended_operation_s(ld, NMASLDAP_GET_PASSWORD_REQUEST, request_bv, NULL, NULL, &reply_oid, &reply_bv);
	if (err) goto finish;

	/* Make sure there is a return OID */
	if (!reply_oid) {
		err = NMAS_E_NOT_SUPPORTED;
		goto finish;
	}

	/* Is this what we were expecting to get back. */
	if (strcmp(reply_oid, NMASLDAP_GET_PASSWORD_RESPONSE) != 0) {
		err = NMAS_E_NOT_SUPPORTED;
		goto finish;
	}

	/* Do we have a good returned berval? */
	if (!reply_bv) {
		/*
		 *	No; returned berval means we experienced a rather
		 *	drastic error.  Return operations error.
		 */
		err = NMAS_E_SYSTEM_RESOURCES;
		goto finish;
	}

	bufsize = sizeof(buffer);
	err = ber_decode_login_data(reply_bv, &server_version, buffer, &bufsize);
	if (err) goto finish;

	if (server_version != NMAS_LDAP_EXT_VERSION) {
		err = NMAS_E_INVALID_VERSION;
		goto finish;
	}

	if (bufsize > *passlen) {
		err = NMAS_E_BUFFER_OVERFLOW;
		goto finish;
	}

	memcpy(password, buffer, bufsize);
	password[bufsize] = '\0';
	*passlen = bufsize;

finish:
	if (reply_bv) {
		ber_bvfree(reply_bv);
	}

	/* Free the return OID string if one was returned. */
	if (reply_oid) {
		ldap_memfree(reply_oid);
	}

	/* Free memory allocated while building the request ber and berval. */
	if (request_bv) {
		ber_bvfree(request_bv);
	}

	return err;
}
