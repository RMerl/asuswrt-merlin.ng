/* base64 encoding/decoding functions for GnuTLS
*
* Copyright (C) 2021  Dan Fandrich <dan@coneharvesters.com>
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 2
* of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, visit the Free Software Foundation
* website at http://www.gnu.org/licenses/gpl-2.0.html or write to the
* Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
* Boston, MA 02110-1301, USA.
*/

#include "base64.h"
#include <gnutls/gnutls.h>

/*
 * Encode a buffer into base64 format
 */
int base64_encode(unsigned char *dst, size_t *dlen, const unsigned char *src, size_t slen)
{
	gnutls_datum_t in = {(unsigned char *)src, slen}, out;

	int rc = gnutls_base64_encode2(&in, &out);
	if (rc != GNUTLS_E_SUCCESS) {
		/* This is probably an OOM error, so try to return something semi-sane */
		*dlen = slen * 2 + 1;
		return ERR_BASE64_BUFFER_TOO_SMALL;
	}
	if ((out.size + 1) > *dlen) {
		*dlen = out.size + 1;
		gnutls_free(out.data);
		return ERR_BASE64_BUFFER_TOO_SMALL;
	}
	memcpy(dst, out.data, out.size);
	dst[out.size] = 0;
	*dlen = out.size;

	gnutls_free(out.data);

	return 0;
}

/*
 * Decode a base64-formatted buffer
 */
int base64_decode(unsigned char *dst, size_t *dlen, const unsigned char *src, size_t slen)
{
	gnutls_datum_t in = {(unsigned char *)src, slen}, out;

	int rc = gnutls_base64_decode2(&in, &out);
	if (rc == GNUTLS_E_BASE64_DECODING_ERROR)
		return ERR_BASE64_INVALID_CHARACTER;
	if (rc != GNUTLS_E_SUCCESS) {
		/* This is probably an OOM error, so try to return something semi-sane */
		*dlen = slen;
		return ERR_BASE64_BUFFER_TOO_SMALL;
	}
	if (out.size > *dlen) {
		*dlen = out.size;
		gnutls_free(out.data);
		return ERR_BASE64_BUFFER_TOO_SMALL;
	}
	memcpy(dst, out.data, out.size);
	*dlen = out.size;

	gnutls_free(out.data);

	return 0;
}
