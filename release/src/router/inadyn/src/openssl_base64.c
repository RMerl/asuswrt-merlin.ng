/* base64 encoding/decoding functions for OpenSSL
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
#include <openssl/evp.h>

/*
 * Encode a buffer into base64 format
 */
int base64_encode(unsigned char *dst, size_t *dlen, const unsigned char *src, size_t slen)
{
	size_t needed = (((4 * slen / 3) + 3) & ~3) + 1;  // +1 for NUL

	if (slen && (*dlen < needed)) {
		*dlen = needed;
		return ERR_BASE64_BUFFER_TOO_SMALL;
	}

	*dlen = EVP_EncodeBlock(dst, src, slen);

	return 0;
}

/*
 * Decode a base64-formatted buffer
 */
int base64_decode(unsigned char *dst, size_t *dlen, const unsigned char *src, size_t slen)
{
	size_t needed = (3 * slen) / 4;
	int rc;

	if (*dlen < needed) {
		*dlen = needed;
		return ERR_BASE64_BUFFER_TOO_SMALL;
	}

	rc = EVP_DecodeBlock(dst, src, slen);
	if (rc < 0)
		return ERR_BASE64_INVALID_CHARACTER;
	*dlen = (size_t) rc;

	return 0;
}
